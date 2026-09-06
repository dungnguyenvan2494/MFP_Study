## MODULE 29 — Shell chạy lệnh: `fork()` + `execve()`

Sơ đồ: [docs/diagrams/shell-fork-exec-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/shell-fork-exec-arm64.md) — 4 sơ đồ: ba mốc bash→child→ls, bảng thay/giữ, page table qua 4 thời điểm, `pt_regs` qua các mốc.

**File nguồn**: `kernel/fork.c` (`_do_fork`/`kernel_clone`, `copy_process`), `fs/exec.c` (`__do_execve_file`, `bprm_mm_init`, `flush_old_exec`/`begin_new_exec`, `de_thread`, `exec_mmap`, `flush_signal_handlers`, `do_close_on_exec`, `setup_new_exec`), `fs/binfmt_elf.c` (`load_elf_binary`, `create_elf_tables`), `arch/arm64/kernel/process.c` (`copy_thread`), `arch/arm64/include/asm/processor.h` (`start_thread`).

### 1. Vì sao `fork` + `exec` chứ không một syscall

Unix tách làm hai để con có **cửa sổ giữa fork và exec** làm việc dưới danh tính đã thừa kế: `dup2()` redirect stdin/stdout, `close()` fd không cần, `setpgid()` job control, `setuid()` hạ quyền, `chdir()`… rồi mới `execve()`. Toàn bộ chuẩn bị này chạy trong tiến trình con, với `mm`/fd/cwd của bash, trước khi ELF mới đè lên.

### 2. `fork()` — `copy_process` (bash → child PID 101)

|Thành phần|`fork` làm gì|Kết quả|
|---|---|---|
|`task_struct`|`dup_task_struct`|struct **mới**, nội dung copy; kernel stack 16 KB **mới**|
|PID/TGID|`alloc_pid`|**mới** (101); `real_parent = bash`; vào `bash->children`|
|`cred`|`copy_creds` (không `CLONE_THREAD`)|`prepare_creds` — **copy**; uid/gid/caps = bash|
|`files_struct`|`copy_files` → `dup_fd`|mảng fd **mới**, mỗi ô `get_file` → **chung `struct file`** (chung `f_pos`)|
|`fs_struct`|`copy_fs`|`CLONE_FS` tắt → **copy**; cwd/root = bash|
|`sighand_struct`|`copy_sighand`|**copy** bảng `action[64]` y hệt bash|
|`signal_struct`|`copy_signal`|**mới**; `shared_pending` rỗng, rlimit copy, `nr_threads=1`|
|`mm_struct`|`copy_mm` → `dup_mm` → `copy_page_range`|mm **mới**; page table = bản sao; **mọi PTE ghi-được → read-only cả hai phía (COW)**|
|`pt_regs`|`copy_thread`|`*childregs = *bash_pt_regs`; chỉ `regs[0] = 0`|
|khởi động|`cpu_context.pc = ret_from_fork`, `sp = childregs`|scheduler chọn → `cpu_switch_to` → `ret_from_fork` → `schedule_tail` → `ret_to_user` → `eret`|

Child `eret` về EL0 tại **cùng lệnh** sau `svc` của `fork()` (cùng `pc`, cùng `sp` — trang stack là COW dùng chung), khác duy nhất `x0 = 0`. Code bash: `if (pid == 0) { ... execve("/bin/ls", argv, envp); }`.

### 3. `execve("/bin/ls")` — cùng `task_struct`, thay `mm`

1. `bprm_mm_init()` — cấp `mm_struct` **mới hoàn toàn** (rỗng), dựng VMA stack tạm.
2. `open_exec("/bin/ls")`, `prepare_binprm()` đọc 128 byte đầu (magic ELF), `bprm_fill_uid()` (suid — `/bin/ls` thường không).
3. `search_binary_handler()` → `load_elf_binary()`.
4. **`begin_new_exec()` / (5.4: `flush_old_exec` + `setup_new_exec`) — POINT OF NO RETURN:**
    - `de_thread()` — giết mọi thread khác trong nhóm (bash-con chỉ có 1 thread → không làm gì).
    - `unshare_files()` — nếu `files_struct` đang chia sẻ thì tách ra.
    - `exec_mmap(bprm->mm)` — `current->mm = mm mới`; `mmput(mm cũ)` → **mm bản-sao-bash của child bị huỷ**; các trang COW mà nó giữ được buông (`_mapcount--`).
    - `flush_thread()` — xoá FPSIMD/TLS/debug registers.
    - `do_close_on_exec()` — đóng fd có `O_CLOEXEC` (stdin/stdout/stderr **không** có → ở lại → `ls` in ra terminal của bash).
    - `flush_signal_handlers()` — mọi handler tuỳ chỉnh → `SIG_DFL` (`SIG_IGN` giữ nguyên).
5. `setup_new_exec()`, `setup_arg_pages()` (VMA stack thật), `elf_map()` PT_LOAD của `/bin/ls` + của `ld.so` (PT_INTERP), tính `load_bias` (ASLR).
6. `install_exec_creds()` — `commit_creds(bprm->cred)` (chỉ đổi danh tính nếu suid).
7. `create_elf_tables()` — đẩy `argc`, `argv[]`, `envp[]`, `auxv[]` (AT_PHDR, AT_ENTRY, AT_BASE, AT_RANDOM, AT_SYSINFO_EHDR…) lên stack user mới.
8. `start_thread(regs, elf_entry, sp)` — `memset(regs, 0)`, `regs->pc = entry` (của `ld.so`), `regs->sp = stack mới`, `regs->pstate = PSR_MODE_EL0t`.
9. `ret_to_user` → `eret` → EL0 tại `_start` của `ld.so` → nạp libc → `_start` của `ls` → `main(argc, argv, envp)`.

### 4. Thay / giữ

**GIỮ qua `execve`** (cùng con trỏ, chỉ vài field đổi):

- `task_struct`, **PID/TGID 101**, `real_parent = bash`, vị trí trong cây process
- `files_struct` (trừ fd `O_CLOEXEC`) — nên `ls` thừa hưởng stdin/stdout/stderr, pipe của bash
- `fs_struct` — cwd không đổi → `ls` liệt kê đúng thư mục bash đang đứng
- `signal_struct` — nhóm tiến trình, session, rlimit, `shared_pending`
- `cred` (trừ khi file suid)
- vùng kernel stack 16 KB (chỉ `pt_regs` ở đỉnh bị ghi đè)

**THAY qua `execve`:**

- `mm_struct` — mm mới, VMA mới (.text/.data/.bss/heap/stack + `ld.so`), page table mới, `brk` reset
- `pt_regs` — memset 0, `pc`/`sp`/`pstate` đặt mới
- `sighand_struct` — handler tuỳ chỉnh → `SIG_DFL`
- fd `O_CLOEXEC` — đóng
- `cred` — nếu suid/sgid/file-caps

### 5. Bộ nhớ qua 4 thời điểm

1. **Trước fork**: bash có trang heap `H` → PFN `0x1000` (RW). `TTBR0 = PGD_bash`, `ASID = 10`.
2. **Sau fork, chưa ai ghi**: `bash pte(H)` và `child pte(H)` đều trỏ PFN `0x1000`, **read-only, COW**; `page->_mapcount = 2`. `child TTBR0 = PGD_child`, `ASID = 11`.
3. **child `execve` → `exec_mmap`**: `current->mm = mm mới`; `mmput(mm_child cũ)` → mọi VMA gỡ, `pte(H)` buông → `page 0x1000 _mapcount = 1` (chỉ còn bash). Child không còn ánh xạ nào của bash.
4. **ls chạy**: mm mới có VMA `.text` của `ld.so` @ `load_bias` (ASLR), `.text`/`.data` của `/bin/ls`, heap rỗng, stack mới với argv/envp/auxv. Trang `.text` là ánh xạ file → dùng chung page cache toàn hệ thống (nhiều `ls` chạy song song chung các trang này). `ASID = 11` giữ nguyên (chỉ nội dung bảng đổi) → `local_flush_tlb_asid(11)`.

**Bash không tốn một lần copy trang nào cho lần fork này** — child `execve` trước khi ghi vào bất kỳ trang COW nào. Đó là lý do "fork + exec" rẻ dù `fork()` trên lý thuyết "nhân đôi cả tiến trình".

### 6. `pt_regs` qua các mốc

|Mốc|`pt_regs`|
|---|---|
|bash tại `svc` của `fork()`|`x8=220`, `x0..x5`= tham số clone, `pc`=&lệnh sau svc, `sp`=stack bash|
|child sau `copy_thread`|= **copy** của bash, `regs[0]=0` → child `eret`: cùng `pc`/`sp` (trang COW), `x0=0`|
|child tại `svc` của `execve()`|`x0`=path, `x1`=argv, `x2`=envp, `x8=221`|
|sau `start_thread`|memset 0; `pc`=`e_entry` ld.so + `load_bias`; `sp`=đỉnh stack user mới (argc/argv/envp/auxv); `pstate`=`PSR_MODE_EL0t`; `x0..x30`=0|
|sau `eret`|CPU @ EL0, `PC`=`_start` ld.so, `SP`=stack mới → ld.so → libc → `_start` ls → `main`|

### Điểm dễ nhầm

|Nhầm|Thực tế|
|---|---|
|"`execve` tạo tiến trình mới"|Không — cùng `task_struct`, cùng PID. Chỉ nội dung bị thay.|
|"`execve` thành công thì trả về"|Không bao giờ trả về (nếu thành công). `pt_regs` cũ bị xoá.|
|"fork copy toàn bộ RAM tiến trình"|Chỉ copy page table + đặt COW. Trang thật copy lazy, khi ghi.|
|"child thấy `fork()` trả PID"|Child thấy **0**; bash thấy **101**.|
|"stdout của `ls` là mới"|fd 1 (không `O_CLOEXEC`) giữ nguyên → `ls` ghi vào terminal/pipe của bash.|
|"handler `SIGINT` của bash còn trong `ls`"|Không — `flush_signal_handlers` reset về `SIG_DFL`. (Nhưng `SIG_IGN` thì giữ.)|
|"`begin_new_exec` lỗi thì child quay lại là bash"|Không — sau `exec_mmap`, `mm` cũ đã mất. Lỗi ở đây → child nhận `SIGSEGV`/chết.|

### Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản mới|
|---|---|---|
|Ranh giới no-return|`flush_old_exec()` + `setup_new_exec()`|gộp `begin_new_exec()` (5.9)|
|fork lõi|`_do_fork()`|`kernel_clone()` (5.10)|
|`unshare_files` khi exec|`unshare_files(&displaced)` + `put_files_struct`|`unshare_files()` không tham số (5.10)|
|COW copy|`copy_one_pte`|`copy_present_pte` batch (5.19); ngữ nghĩa như cũ|
|ELF loader / `start_thread` / auxv|như mô tả|không đổi|

> Tree local = **5.10.241**: `begin_new_exec` (`fs/exec.c:1243`) gọi `de_thread`→`unshare_files`→`exec_mmap`→`flush_thread`→`do_close_on_exec`→`flush_signal_handlers`. Ở 5.4 các bước này nằm trong `flush_old_exec`+`setup_new_exec`; luồng và hiệu ứng giống nhau.