# MODULE 10 — `execve()` trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/execve-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/execve-arm64.md) (4 sơ đồ: chuỗi gọi + "điểm không quay lại", bảng thay thế/giữ nguyên, `bprm` người vận chuyển, kernel stack sống qua exec).

**File nguồn**: `fs/exec.c` (`do_execve`, `do_execveat_common`, `__do_execve_file`, `prepare_bprm_creds`, `bprm_mm_init`, `open_exec`, `prepare_binprm`, `search_binary_handler`, `flush_old_exec`, `exec_mmap`, `de_thread`, `setup_new_exec`, `install_exec_creds`, `setup_arg_pages`), `fs/binfmt_elf.c` (`load_elf_binary`, `load_elf_interp`, `elf_map`, `create_elf_tables`, `set_brk`), `arch/arm64/kernel/process.c` (`flush_thread`), `arch/arm64/include/asm/processor.h` (`start_thread`).

---

## 0. Ý chính

`execve` **không tạo process mới**. Nó giữ nguyên vỏ (`task_struct`, PID, kernel stack, quan hệ cha/con, file descriptor không CLOEXEC, cwd) và **thay toàn bộ ruột**: `mm_struct` mới hoàn toàn, `pt_regs` bị ghi đè để `eret` nhảy vào chương trình mới, signal handler reset về mặc định.

Ranh giới quyết định là **`flush_old_exec()`** — trước nó lỗi thì trả `-errno` và chương trình cũ chạy tiếp; sau nó thì `mm` cũ đã hủy, không lui được, lỗi ⇒ `SIGSEGV`.

---

## 1. Chuỗi gọi đầy đủ

### 1.1. Vào syscall

```c
// fs/exec.c
SYSCALL_DEFINE3(execve, const char __user *, filename,
                const char __user *const __user *, argv,
                const char __user *const __user *, envp)
{
    return do_execve(getname(filename), argv, envp);
}

do_execve(filename, argv, envp)
  → do_execveat_common(AT_FDCWD, filename, argv, envp, 0)
```

`do_execveat_common` (trong 5.9+ tách phần lõi ra `bprm_execve` / `__do_execve_file`) là worker chính. Nó:

### 1.2. Dựng `struct linux_binprm` (bprm) — người vận chuyển tạm

```c
bprm = kzalloc(sizeof(*bprm), GFP_KERNEL);
```

`bprm` gom mọi thứ "mới" ở dạng **tạm**, chưa gắn vào task:

|Trường bprm|Nội dung|
|---|---|
|`mm`|`mm_struct` **mới** (từ `mm_alloc()`)|
|`cred`|**bản sao** cred hiện tại (từ `prepare_exec_creds()`)|
|`file`|`struct file` của `/bin/program`|
|`buf[256]`|256 byte đầu file (đủ chứa ELF header)|
|`p`|con trỏ đỉnh stack mới; dịch xuống mỗi lần `copy_strings`|
|`argc`, `envc`, `filename`, `interp`|metadata|

### 1.3. `prepare_bprm_creds(bprm)`

```c
bprm->cred = prepare_exec_creds();   // = bản sao struct cred của current
```

Đây là **bản nháp** credential. Nếu file có bit `setuid`/`setgid` hoặc LSM can thiệp, các trường `euid`/`egid`/`fsuid`... của `bprm->cred` sẽ bị chỉnh (ở bước `prepare_binprm`). Nếu không, `bprm->cred` cuối cùng bị bỏ đi.

### 1.4. `bprm_mm_init(bprm)` → `__bprm_mm_init`

```c
bprm->mm = mm_alloc();                 // mm_struct MỚI, rỗng, pgd mới
__bprm_mm_init(bprm):
    vma = vm_area_alloc(mm);
    vma->vm_end   = STACK_TOP_MAX;      // đỉnh không gian user
    vma->vm_start = vma->vm_end - PAGE_SIZE;
    vma->vm_flags = VM_SOFTDIRTY | VM_STACK_FLAGS | VM_STACK_INCOMPLETE_SETUP;
    insert_vm_struct(mm, vma);
    bprm->p = vma->vm_end - sizeof(void *);   // con trỏ stack mới
```

→ Có một **VMA stack tạm** ở đỉnh của `bprm->mm`. Task vẫn đang chạy trên `mm` **cũ** — `bprm->mm` là "mm song song".

### 1.5. `open_exec(filename)` → `bprm->file`

Mở file, kiểm `S_ISREG`, quyền thực thi (`MAY_EXEC`), `deny_write_access` (không cho ai ghi vào file đang exec). Trả `struct file *`.

### 1.6. `sched_exec()`

Cân tải: **trước khi** phá address space, có thể migrate task sang CPU tốt hơn (giờ task "nhẹ" nhất vì sắp bỏ hết bộ nhớ cũ).

### 1.7. `prepare_binprm(bprm)`

```c
retval = kernel_read(bprm->file, bprm->buf, BINPRM_BUF_SIZE, &pos);  // 256 byte đầu
bprm_fill_uid(bprm);          // tính euid/egid từ bit setuid/setgid của file → bprm->cred
security_bprm_set_creds(bprm); // LSM (SELinux/AppArmor) có thể chỉnh tiếp bprm->cred
```

### 1.8. `copy_strings_kernel` + `copy_strings`

Chép `filename`, `argv[]`, `envp[]` (chuỗi C) từ user space cũ **lên stack của `bprm->mm`** (fault từng trang vào qua `get_arg_page`). `bprm->p` giảm dần. Giới hạn `_STK_LIM / 4 * 3` (1/4 của `RLIMIT_STACK`, tối đa) cho argv+envp.

### 1.9. `exec_binprm` → `search_binary_handler(bprm)`

```c
list_for_each_entry(fmt, &formats, lh) {
    retval = fmt->load_binary(bprm);   // ELF → load_elf_binary
    if (retval != -ENOEXEC) break;     // format khớp → dừng
}
```

Format khác: `binfmt_script` (shebang `#!`), `binfmt_misc` (wine, qemu-user…). Với ELF → **`load_elf_binary()`**.

---

## 2. `load_elf_binary()` (`fs/binfmt_elf.c`)

### 2.1. Kiểm tra & đọc header (chưa đụng gì tới process)

```c
loc->elf_ex = *((struct elfhdr *)bprm->buf);
if (memcmp(elf_ex->e_ident, ELFMAG, SELFMAG) != 0) return -ENOEXEC;
if (elf_ex->e_type != ET_EXEC && elf_ex->e_type != ET_DYN) return -ENOEXEC;
if (!elf_check_arch(elf_ex)) return -ENOEXEC;   // EM_AARCH64
elf_phdata = load_elf_phdrs(elf_ex, bprm->file); // đọc program headers
```

Quét program headers tìm `PT_INTERP` → nếu có (PIE hoặc linked-động), đọc đường dẫn interpreter (`/lib/ld-linux-aarch64.so.1`), `open_exec(elf_interpreter)` → `interpreter` file.

Tới đây vẫn có thể `return -ENOEXEC/-errno` an toàn — **chương trình cũ nguyên vẹn**.

### 2.2. `flush_old_exec(bprm)` — ĐIỂM KHÔNG QUAY LẠI

```c
// fs/exec.c (5.4). 5.9+: gộp vào begin_new_exec()
int flush_old_exec(struct linux_binprm *bprm)
{
    retval = de_thread(current);              // (a) giết mọi thread khác trong nhóm
    ...
    retval = exec_mmap(bprm->mm);             // (b) LẮP mm mới, hủy mm cũ
    ...
    set_fs(USER_DS);
    current->flags &= ~(PF_RANDOMIZE | PF_FORKNOEXEC | PF_KTHREAD | PF_NOFREEZE | PF_NO_SETAFFINITY);
    flush_thread();                           // (c) xoá trạng thái kiến trúc (arch)
    current->personality &= ~bprm->per_clear;
    ...
    do_close_on_exec(current->files);         // (d) đóng fd O_CLOEXEC
    return 0;
}
```

**(a) `de_thread(current)`** — nếu process đa luồng: gửi tín hiệu nội bộ giết mọi thread khác, chờ chúng vào zombie. Thread đang gọi `execve` **sống sót**. Nếu nó không phải group leader, `de_thread` **hoán đổi `struct pid`**: thread gọi execve chiếm lấy PID của leader cũ → từ userspace, PID của process **không đổi**, chỉ có `task_struct` sống sót thay đổi `pid` nội bộ. `nr_threads` về 1.

**(b) `exec_mmap(bprm->mm)`** — lõi của việc thay bộ nhớ:

```c
static int exec_mmap(struct mm_struct *mm)
{
    struct task_struct *tsk = current;
    struct mm_struct *old_mm = current->mm;
    ...
    task_lock(tsk);
    membarrier_exec_mmap(mm);
    local_irq_disable();
    active_mm = tsk->active_mm;
    tsk->active_mm = mm;
    tsk->mm = mm;                    // ◀── task->mm = mm MỚI
    activate_mm(active_mm, mm);      // ◀── đổi TTBR0_EL1 sang pgd mới, cấp ASID mới
    local_irq_enable();
    task_unlock(tsk);
    if (old_mm) {
        mmput(old_mm);              // ◀── mm_users cũ về 0 → exit_mmap():
                                    //     unmap mọi VMA, free page tables, thả trang
                                    //     → mmdrop → free pgd + mm_struct cũ
    }
}
```

Sau dòng này: mọi `.text/.data/.bss/heap/stack` cũ **biến mất**. ASID cũ được trả lại. `mm->context.id` = ASID mới cho `bprm->mm`.

**(c) `flush_thread()`** (arm64, `arch/arm64/kernel/process.c`):

```c
void flush_thread(void)
{
    fpsimd_flush_thread();               // xoá FP/SIMD, SVE state
    flush_ptrace_hw_breakpoint(current); // xoá hw breakpoint/watchpoint
    flush_tagged_addr_state();
    current->thread.uw.tp_value = 0;     // TLS về 0 (TPIDR_EL0 nạp 0 khi về EL0)
}
```

**(d) `do_close_on_exec(current->files)`** — duyệt bitmap `close_on_exec`, đóng các fd có cờ `O_CLOEXEC`. fd còn lại **giữ nguyên** (cùng `struct file`, cùng offset).

### 2.3. `setup_new_exec(bprm)`

```c
void setup_new_exec(struct linux_binprm *bprm)
{
    arch_pick_mmap_layout(current->mm, ...);   // mm->mmap_base + ASLR
    current->mm->task_size = STACK_TOP;         // giới hạn user (32 vs 64-bit)
    ...
    current->sas_ss_sp = current->sas_ss_size = 0;   // xoá alternate signal stack
    arch_setup_new_exec();
    __set_task_comm(current, kbasename(bprm->filename), true);  // comm[16] = "program"
    do_undo_group_signal(...);   // dọn group-stop state
    ...
}
```

Cũng đặt `mm->exe_file = bprm->file` (`set_mm_exe_file`) → `/proc/pid/exe` trỏ binary mới.

### 2.4. `install_exec_creds(bprm)`

```c
void install_exec_creds(struct linux_binprm *bprm)
{
    security_bprm_committing_creds(bprm);
    commit_creds(bprm->cred);        // ◀── setuid/setgid CÓ HIỆU LỰC TẠI ĐÂY
    bprm->cred = NULL;
    ...
    security_bprm_committed_creds(bprm);
}
```

Nếu file **không** setuid và LSM không đổi gì, `bprm->cred` là bản sao y hệt `current->cred` → `commit_creds` gần như no-op (thực tế vẫn thay object nhưng nội dung giống). Nếu file setuid: `euid`/`fsuid` mới có hiệu lực. **`real_uid` luôn giữ nguyên.**

### 2.5. `setup_arg_pages(bprm, randomize_stack_top(STACK_TOP), executable_stack)`

Dời VMA stack **tạm** (đang ở `STACK_TOP_MAX`) xuống vị trí **cuối cùng** = `STACK_TOP - random(ASLR)`. Đặt `vm_flags = VM_STACK_FLAGS | VM_GROWSDOWN`, gỡ `VM_STACK_INCOMPLETE_SETUP`. Cập nhật `mm->start_stack`, `mm->arg_start`, `bprm->p`.

### 2.6. Map các `PT_LOAD`

```c
for (i = 0; i < loc->elf_ex.e_phnum; i++, elf_ppnt++) {
    if (elf_ppnt->p_type != PT_LOAD) continue;
    ...
    if (elf_ex->e_type == ET_DYN) {          // PIE
        load_bias = ELF_ET_DYN_BASE;
        load_bias += arch_mmap_rnd();        // ASLR
        load_bias = ELF_PAGESTART(load_bias - vaddr);
    } // ET_EXEC: load_bias = 0

    error = elf_map(bprm->file, load_bias + vaddr, elf_ppnt, elf_prot, elf_flags, ...);
    // → vm_mmap_pgoff(): tạo VMA file-backed, MAP_PRIVATE, prot theo p_flags (R/W/X)
}
set_brk(elf_bss, elf_brk, bss_prot);        // zero phần đuôi .bss, tạo VMA heap
current->mm->start_code = start_code;
current->mm->end_code   = end_code;
current->mm->start_data = start_data;
current->mm->end_data   = end_data;
current->mm->brk = current->mm->start_brk = arch_randomize_brk(current->mm);
```

Chú ý: `elf_map` chỉ **tạo VMA** — chưa có PTE nào. Trang thật được nạp qua page fault khi chương trình chạy (Module 7 / demand paging).

### 2.7. Interpreter (nếu PIE-dynamic)

```c
if (elf_interpreter) {
    elf_entry = load_elf_interp(&loc->interp_elf_ex, interpreter, &interp_map_addr, load_bias, interp_elf_phdata);
    // → map ld.so vào không gian địa chỉ
    elf_entry += loc->interp_elf_ex.e_entry;   // entry point của ld.so
} else {
    elf_entry = loc->elf_ex.e_entry + load_bias;  // entry point của chính chương trình
}
```

### 2.8. `create_elf_tables(bprm, &loc->elf_ex, load_addr, interp_load_addr)`

Dựng trên stack mới, từ `bprm->p` đi lên:

- `argc` (int)
- `argv[]` (con trỏ tới các chuỗi đã copy ở bước 1.8), kết thúc `NULL`
- `envp[]`, kết thúc `NULL`
- **auxv[]** — mảng `Elf64_auxv_t`: `AT_PHDR` (địa chỉ program headers), `AT_PHENT`, `AT_PHNUM`, `AT_BASE` (địa chỉ nạp ld.so), `AT_ENTRY` (entry của _chương trình_, để ld.so nhảy tới sau khi relocate), `AT_RANDOM` (16 byte ngẫu nhiên cho stack canary), `AT_SYSINFO_EHDR` (địa chỉ **vDSO**), `AT_HWCAP`/`AT_HWCAP2` (cờ tính năng CPU), `AT_PAGESZ`, `AT_UID`/`AT_EUID`/`AT_GID`/`AT_EGID`, `AT_SECURE` (1 nếu setuid), `AT_EXECFN` (đường dẫn), kết thúc `AT_NULL`.

`bprm->p` giờ trỏ tới `argc` — đây là `sp` ban đầu của EL0.

### 2.9. `start_thread(regs, elf_entry, bprm->p)` (arm64)

```c
static inline void start_thread(struct pt_regs *regs, unsigned long pc, unsigned long sp)
{
    start_thread_common(regs, pc);    // memset(regs, 0, sizeof(*regs)); regs->pc = pc; giữ syscallno
    regs->pstate = PSR_MODE_EL0t;     // AArch64, EL0, DAIF theo mặc định
    regs->sp = sp;                     // = bprm->p (đỉnh stack user mới, trỏ argc)
}
```

→ `pt_regs` ở đỉnh kernel stack (cùng ô nhớ cũ) bị **ghi đè**: mọi `x0..x30 = 0`, `pc = elf_entry`, `sp = đỉnh stack mới`, `pstate = EL0t`.

### 2.10. Return

`load_elf_binary` return 0 → `search_binary_handler` return 0 → `free_bprm(bprm)` (thả `bprm->mm` reference thừa, `bprm->file`…) → `__do_execve_file` return 0.

Call chain hàm C (`__do_execve_file` → `search_binary_handler` → `load_elf_binary` → …) **unwind bình thường** trên kernel stack — vẫn là kernel stack cũ, không hề bị `exec_mmap` đụng tới. Rồi:

`ret_to_user` → `kernel_exit 0` → `ldp x0..x30` từ `pt_regs` (mới, toàn 0) → `msr elr_el1, regs->pc` (= `elf_entry`) → `msr sp_el0, regs->sp` (đỉnh stack mới) → `eret`.

Phần cứng: `PC ← elf_entry`, `SP ← đỉnh stack user mới`, EL0. Thực thi bắt đầu ở `ld.so` (PIE) hoặc `_start` (static). `_start` đọc `argc` tại `[sp]`, `argv` tại `[sp+8]`, tính `envp`, `auxv`, gọi `__libc_start_main` → `main()`.

`x0 = 0` không có ý nghĩa — ELF `_start`/`ld.so` không đọc `x0` như giá trị trả về; chúng đọc stack.

---

## 3. Bảng thay thế vs giữ nguyên — chi tiết theo prompt

### PID — **GIỮ NGUYÊN**

`task->pid` không đổi. `tgid` không đổi. Từ userspace, PID hoàn toàn ổn định qua `execve`. Ngoại lệ nội bộ: nếu một **thread không phải leader** gọi `execve`, `de_thread()` hoán `struct pid` giữa nó và leader cũ → `task_struct` sống sót có `pid` nội bộ đổi thành pid của leader cũ, nhưng con số PID mà thế giới bên ngoài thấy vẫn nguyên.

### `task_struct` — **GIỮ NGUYÊN (cùng object, cùng địa chỉ)**

`execve` không bao giờ `kmem_cache_alloc` một `task_struct` mới. Hầu hết trường không đụng: `sched_entity` (vị trí lập lịch, `vruntime`, `prio`), `cpus_mask`, `real_parent`/`parent`/`children`/`sibling`, `nsproxy`, `thread_info.flags` (một số cờ như `PF_FORKNOEXEC` bị xóa). Bị chỉnh: `comm[16]`, `thread.uw.tp_value` (→ 0), `thread.uw.fpsimd_state` (xóa), `thread.debug` (xóa), `sas_ss_*` (→ 0), `mm`/`active_mm` (→ mm mới).

### `mm_struct` — **THAY THẾ HOÀN TOÀN**

`mm_alloc()` tạo cái mới; `exec_mmap` gắn vào task; `mmput(old_mm)` → `exit_mmap` hủy mọi VMA, page table, thả trang; `mmdrop` → free `pgd` + `mm_struct` cũ. Hệ quả: mọi mã, dữ liệu, heap, stack, mmap của chương trình cũ **mất sạch**; `pgd` mới; **ASID mới**; `/proc/pid/maps` hoàn toàn khác; `/proc/pid/exe` trỏ binary mới.

### `files` (`files_struct`) — **GIỮ NGUYÊN (cùng object)**

fd không có `O_CLOEXEC` vẫn mở, trỏ cùng `struct file`, cùng `f_pos`. fd có `O_CLOEXEC` bị đóng (`do_close_on_exec`). **Đây là cơ chế nền của I/O redirection**: `bash` mở fd 1 tới file, rồi `execve("/bin/ls")` — `ls` kế thừa fd 1 và ghi ra file. Cũng là lý do `fexecve`, `dup2`, pipe giữa các lệnh hoạt động.

### `cwd` (trong `fs_struct`) — **GIỮ NGUYÊN (cùng object)**

`fs_struct` không thay. `cwd`, `root` (chroot), `umask` đều giữ. `chdir()` trước `execve` vẫn có hiệu lực sau đó — nền của `cd dir && ./prog`.

### `credentials` (`cred`) — **GIỮ, TRỪ setuid/LSM**

- File **không** setuid/setgid và LSM không can thiệp → `bprm->cred` là bản sao y hệt → `commit_creds` thay object nhưng nội dung không đổi. Coi như giữ.
- File **có** bit setuid (vd `/usr/bin/passwd` thuộc root, bit `s`) → `bprm->cred->euid = file owner`, `fsuid` theo. `commit_creds(bprm->cred)` cài đặt → process chạy với `euid` mới.
- **`real_uid`/`real_gid` LUÔN giữ nguyên** kể cả setuid — đó là "tôi thực sự là ai".
- `pdeath_signal` (`prctl PR_SET_PDEATHSIG`) bị **xóa** trên `execve` (chống leo thang đặc quyền qua signal khi đổi cred).
- Cấp phát keyring session mới nếu cần; capabilities tính lại theo file capabilities (xattr `security.capability`).
- `AT_SECURE` trong auxv = 1 nếu có bất kỳ thay đổi đặc quyền nào → `ld.so` bỏ qua `LD_PRELOAD`, `LD_LIBRARY_PATH` từ env.

### `signals` — **HANDLER reset, phần còn lại giữ**

- `flush_signal_handlers(current, 0)`: mọi disposition **không phải `SIG_IGN`** → `SIG_DFL`. Handler đã đặt `SIG_IGN` thì **giữ** (chương trình mới vẫn thấy tín hiệu đó bị bỏ qua — hành vi POSIX).
- `sighand_struct` là **cùng object** (chỉ nội dung `action[]` bị reset).
- `signal_struct` là **cùng object**: `rlim[]` giữ, process group / session giữ, `tty` giữ, `nr_threads` → 1.
- **Mặt nạ chặn (`current->blocked`) GIỮ NGUYÊN** — chương trình mới thừa hưởng signal mask.
- Tín hiệu đang chờ: `task->pending` (riêng thread) và `signal->shared_pending` phần lớn giữ; nhưng `de_thread` dọn state group-stop.
- **POSIX interval timers** (`timer_create`) bị **hủy** (`exit_itimers`). `setitimer(ITIMER_REAL/VIRTUAL/PROF)` — `ITIMER_REAL` giữ; các cái khác reset.
- Alternate signal stack (`sigaltstack`) bị **xóa** (`sas_ss_sp = sas_ss_size = 0`).

### `kernel stack` — **GIỮ NGUYÊN (cùng 16 KB, cùng trang vật lý)**

`execve` không gọi `alloc_thread_stack_node`. Toàn bộ syscall `execve` chạy **trên chính kernel stack đó**. `exec_mmap` hủy `mm` cũ nhưng kernel stack là vùng vmalloc riêng biệt (Module 6) — không liên quan. `flush_thread()` xóa trạng thái _kiến trúc_ (thanh ghi FP, debug), không đụng bộ nhớ stack. Chỉ **`pt_regs` ở đỉnh** bị `start_thread` ghi đè nội dung — ô nhớ vẫn là ô cũ. Các frame hàm C của `execve` tự gỡ khi return như mọi syscall.

---

## 4. Tóm tắt hai cột

||Đối tượng|Chi tiết|
|---|---|---|
|**THAY THẾ**|`mm_struct`|mới hoàn toàn: pgd, ASID, mọi VMA, page table, `.text/.data/.bss/heap/stack`|
||`pt_regs` (nội dung)|`start_thread`: `pc=elf_entry`, `sp=stack mới`, `pstate=EL0t`, `x0..x30=0`|
||signal handlers|≠ `SIG_IGN` → `SIG_DFL`|
||FP/SIMD, SVE, TLS, hw breakpoint|`flush_thread()` xóa|
||fd có `O_CLOEXEC`|đóng|
||`comm[16]`, `/proc/pid/exe`, altstack|mới / xóa|
||thread khác trong nhóm|`de_thread()` giết|
||POSIX timers, `pdeath_signal`|hủy / xóa|
||credentials (`euid`…)|**chỉ nếu** file setuid/setgid hoặc LSM|
|**GIỮ NGUYÊN**|**PID**, **TGID**|userspace thấy ổn định|
||**`task_struct`**|cùng object, cùng địa chỉ|
||**kernel stack**|cùng 16 KB, cùng trang vật lý|
||**`files_struct`**|cùng object; fd không CLOEXEC + offset giữ|
||**`fs_struct`** (cwd, root, umask)|cùng object|
||**`signal_struct`**|cùng object; `rlim[]`, pgrp, session, tty, `nr_threads→1`|
||**mặt nạ `blocked`**|thừa hưởng|
||quan hệ cha/con, `nsproxy`|không đổi|
||`real_uid`/`real_gid`|không đổi kể cả setuid|
||`sched_entity`, prio, vruntime, cpus_mask|không đổi|

---

## 5. "Điểm không quay lại"

```
execve("/bin/x")
  ... kiểm tra, mở file, đọc ELF header ...      ← lỗi ở đây → return -errno,
                                                    chương trình CŨ chạy tiếp bình thường
  ─── flush_old_exec() ───────────────────────────  ← BẢN LỀ
      exec_mmap: mmput(old_mm)   ← mm cũ CHẾT
  ... map PT_LOAD, dựng stack, start_thread ...    ← lỗi ở đây → không lui được
                                                    → force_sigsegv(SIGSEGV) giết process
  return 0  →  eret vào elf_entry
```

Đây là lý do POSIX nói: `execve` **thành công thì không trả về**; **thất bại thì trả `-1`** và `errno`. Không có trạng thái "trả về 0 sau execve".

---

## 6. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|Worker|`__do_execve_file()`|`bprm_execve()` (5.9)|
|Hủy mm cũ + lắp mm mới|`flush_old_exec()` rồi `setup_new_exec()`, gọi từ `load_elf_binary`|gộp thành `begin_new_exec()` (5.9)|
|Commit creds|`install_exec_creds()` sau `setup_new_exec`|`commit_creds` gọi bên trong `begin_new_exec` (thứ tự chặt hơn)|
|`bprm->cred`|`prepare_exec_creds()`|y hệt|
|`de_thread`, `exec_mmap`, `create_elf_tables`|như mô tả|y hệt về ngữ nghĩa|
|`time namespace`|chưa có|`timens` reset trên exec (5.6)|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: bạn sẽ thấy `bprm_execve`, `begin_new_exec` (dòng 1243), `setup_new_exec` (dòng 1428). Ngữ nghĩa "thay `mm`, giữ PID/task_struct/files/fs/kernel stack" — **giống hệt 5.4**.