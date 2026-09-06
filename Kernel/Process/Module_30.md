## MODULE 30 — Case Study: Vòng đời hoàn chỉnh một process

Sơ đồ: [docs/diagrams/process-lifecycle-casestudy-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/process-lifecycle-casestudy-arm64.md) — 6 sơ đồ + bảng 18 transition (file · function · hiệu ứng register/page-table/stack).

Chương trình:

```c
int main(void) {
    pid_t pid = fork();
    if (pid == 0) execve("/bin/program", argv, envp);
    wait(NULL);
    return 0;
}
```

---

### Giai đoạn A — Chương trình khởi động (bước 1–3)

**1. bash `execve` chương trình này.** `fs/exec.c:__do_execve_file` → `fs/binfmt_elf.c:load_elf_binary`. bash-con (đã fork từ bash) thay `mm` của nó bằng `mm` của chương trình.

**2. ELF loader dựng địa chỉ ảo.** `load_elf_binary` map các segment `PT_LOAD` (`.text` RO+X, `.data` RW, `.bss` anonymous); nếu là PIE hoặc có `PT_INTERP` → nạp `/lib/ld-linux-aarch64.so.1` tại `load_bias = ELF_ET_DYN_BASE + arch_mmap_rnd()` (ASLR). `create_elf_tables` đẩy `argc`, `argv[]`, `envp[]`, `auxv[]` (`AT_PHDR`, `AT_ENTRY`, `AT_BASE`, `AT_RANDOM`, `AT_SYSINFO_EHDR`, `AT_HWCAP`…) lên đỉnh stack user.

**3. Vào `main()`.** `start_thread` đặt `pt_regs->pc = entry của ld.so`, `sp = đỉnh stack`, `pstate = PSR_MODE_EL0t` → `eret` → EL0. `ld.so` nạp `libc`, xử lý relocation, nhảy `_start` của chương trình → `__libc_start_main` → `main(argc, argv, envp)`.

---

### Giai đoạn B — `fork()` (bước 4–9)

**4. `fork()` → syscall.** `main` gọi `fork()` (glibc) → `svc #0` với `x8 = 220` (`__NR_clone`), flags `= SIGCHLD`. `arch/arm64/kernel/entry.S:el0_svc` → `kernel_entry` lưu 31 GPR + `sp`/`pc`/`pstate` vào **`pt_regs` ở đỉnh kernel stack của CHA** → `el0_svc_common` ghi `syscallno = 220` → `__arm64_sys_clone` → `kernel/fork.c:_do_fork`.

**5. `copy_process`.** `kernel/fork.c`:

- `dup_task_struct(current)` — `alloc_task_struct` + `alloc_thread_stack` (kernel stack **16 KB mới** cho CON) + `arch_dup_task_struct` (memcpy). `thread_info` reset.
- `copy_creds` — không `CLONE_THREAD` → `prepare_creds` (CON có `struct cred` riêng, uid/gid/caps = cha).
- `copy_files` → `dup_fd` — mảng fd mới, `get_file` từng `struct file` (chung `f_pos` với cha).
- `copy_fs`, `copy_sighand` (copy `action[64]`), `copy_signal` (`signal_struct` mới, pending rỗng).

**6. `copy_mm` → `dup_mm`.** `kernel/fork.c` → `dup_mmap` chép từng `vm_area_struct` → `mm/memory.c:copy_page_range` chép cây page table (PGD→PUD→PMD→PTE) của CON.

**7. Thiết lập COW.** `mm/memory.c:copy_present_pte` (5.4: `copy_one_pte`): với mỗi PTE trỏ trang anonymous ghi-được → `ptep_set_wrprotect` **cả hai phía** (cha và con read-only), `page->_mapcount++`, `get_page`. Trang thật **chưa** copy.

**8. `copy_thread` (ARM64).** `arch/arm64/kernel/process.c`:

```c
childregs = task_pt_regs(p);              // đỉnh kernel stack CON
*childregs = *current_pt_regs();          // COPY pt_regs của CHA
childregs->regs[0] = 0;                   // CON thấy fork() trả 0
p->thread.cpu_context.pc = ret_from_fork;
p->thread.cpu_context.sp = (unsigned long)childregs;
```

`pc`/`pstate` trong `childregs` giữ nguyên của cha → CON sẽ `eret` về đúng lệnh sau `svc`.

**9. Kích hoạt + PID.** `alloc_pid` cấp `struct pid` (PID 101). `kernel/sched/core.c:wake_up_new_task` → `sched_fork` đã đặt CON `TASK_RUNNING`, `enqueue_entity` vào `cfs_rq`, `place_entity` cho `vruntime` hơi thấp. `_do_fork` **trả 101 cho CHA** → CHA chạy tiếp, vào `wait(NULL)`.

---

### Giai đoạn C — CON lần đầu chạy, quay về EL0 (bước 10–12)

**10. Context switch sang CON.** `kernel/sched/core.c:__schedule` → `context_switch(rq, prev, next=CON)`:

- `switch_mm_irqs_off` → `arch/arm64/mm/context.c:check_and_switch_context(mm_B)` — cấp `ASID = 11`, `TTBR0_EL1 = PGD_B | (11 << 48)`, flush TLB cục bộ nếu cần.
- `switch_to` → `__switch_to` (C: `fpsimd_thread_switch`, `tls_thread_switch`, `entry_task_switch` ghi `__entry_task = CON`, `dsb(ish)`) → `cpu_switch_to` (asm): lưu `x19–x28/fp/sp/lr` của `prev` vào `prev->thread.cpu_context`, nạp của CON; `mov sp, x9` (SP_EL1 = kernel stack CON, trỏ `childregs`); `msr sp_el0, x1` (**`current` = CON**); `ret` → nhảy `lr = ret_from_fork`.

**11. `ret_from_fork`.** `arch/arm64/kernel/entry.S`:

```asm
bl schedule_tail          // finish_task_switch(prev): nhả rq->lock, preempt_count→0
cbz x19, 1f               // x19 = 0 (user task, không phải kthread) → nhảy
1: b ret_to_user
```

**12. Về EL0.** `ret_to_user` → `_TIF_WORK_MASK` sạch → `kernel_exit 0`: `msr elr_el1, childregs->pc`, `msr spsr_el1, childregs->pstate`, `msr sp_el0, childregs->sp`, `ldp x0..x30` (`x0 = 0`), `add sp, S_FRAME_SIZE`, `eret` → CPU ở EL0, `x0 = 0`.

---

### Giai đoạn D — CON `execve` (bước 13–15)

**13. `execve("/bin/program")`.** CON ở EL0 thấy `fork()` trả 0 → nhánh `if (pid == 0)` → `svc #0` với `x8 = 221` (`__NR_execve`), `x0 = path`. `fs/exec.c:do_execve` → `do_execveat_common` → `__do_execve_file`.

**14. Điểm không quay lại.** `bprm_mm_init` cấp `mm_C` mới (rỗng). Sau `open_exec` + `prepare_binprm` + `search_binary_handler` → `load_elf_binary` → **`begin_new_exec`** (5.4: `flush_old_exec` + `setup_new_exec`):

- `de_thread` — CON chỉ 1 thread → không làm gì.
- `exec_mmap(mm_C)` — `current->mm = mm_C`; **`mmput(mm_B)`** → `mm_B` mọi VMA gỡ, PTE buông → các trang COW mà CON giữ được thả (`_mapcount--`); `mm_B` được giải phóng.
- `flush_thread` (xoá FPSIMD/TLS), `do_close_on_exec` (đóng fd `O_CLOEXEC`), `flush_signal_handlers` (handler tuỳ chỉnh → `SIG_DFL`).

**15. Nạp ELF mới.** `elf_map` các `PT_LOAD` của `/bin/program` + `ld.so`; `create_elf_tables` (argv/envp/auxv lên stack `mm_C`); `install_exec_creds` → `commit_creds` (đổi nếu suid); `start_thread(regs, entry, sp)`: `memset(regs, 0)`, `regs->pc = entry`, `regs->sp = stack mới`, `regs->pstate = PSR_MODE_EL0t`. `ret_to_user` → `eret` → `/bin/program` chạy.

---

### Giai đoạn E — CON exit, CHA reap (bước 16–18)

**16. `/bin/program` chạy rồi `exit(status)`.** glibc `exit()` → flush stdio → `_exit` → `svc #0` `x8 = 94` (`__NR_exit_group`) → `kernel/exit.c:do_exit` (qua `do_group_exit`).

**17. `do_exit` tháo dỡ.** `kernel/exit.c`:

- `exit_signals` — đặt `PF_EXITING`.
- `exit_mm` — `mmput(mm_C)` → `mm_users` về 0 → giải phóng mọi VMA, page table, trang anonymous; `mm_C` freed. Task thành lazy-TLB tạm (`active_mm` mượn).
- `exit_files`, `exit_fs`, `exit_thread`, `exit_task_namespaces`.
- `exit_notify`:
    - `forget_original_parent` — nếu CON có con, reparent chúng cho reaper (subreaper hoặc init).
    - `exit_state = EXIT_ZOMBIE`.
    - `do_notify_parent(CON, SIGCHLD)` → gửi `SIGCHLD` cho CHA + `__wake_up_parent` (đánh thức `wait()` của CHA qua waitqueue `signal->wait_chldexit`).
- `tsk->state = TASK_DEAD` → `do_task_dead` → `__schedule()` — **không bao giờ trở lại**.
- Kernel stack 16 KB của CON được giải phóng trong `finish_task_switch` của **task kế tiếp** (thấy `prev->state == TASK_DEAD` → `put_task_struct_rcu_user(prev)`). `task_struct` giữ lại "vỏ" (`EXIT_ZOMBIE`) cho CHA.

**18. CHA `wait(NULL)`.** `kernel/exit.c:kernel_wait4` → `do_wait` (ngủ trên `wait_chldexit`, `child_wait_callback` đánh thức có mục tiêu) → `wait_consider_task` → `wait_task_zombie`:

- `cmpxchg(&CON->exit_state, EXIT_ZOMBIE, EXIT_DEAD)` — chốt quyền dọn (chống hai luồng `wait` đua nhau).
- Cộng `CON->utime/stime` + con số của con-của-con vào `CHA->signal->cutime/cstime`.
- `release_task(CON)` → `__exit_signal`, `__unhash_process` (gỡ khỏi bảng PID `pid_hash`, cây process `children`/`sibling`, `for_each_process` list), `detach_pid` → `put_task_struct` lần cuối → sau RCU grace period, `task_struct` biến mất → **PID 101 tái sử dụng được**.
- `kernel_wait4` trả về 101 (và status). CHA `main` return 0.

---

### Ba trạng thái "chết"

|Trạng thái|Biến|Ý nghĩa|Ai dọn gì|
|---|---|---|---|
|`TASK_DEAD`|`task->state`|Nhất thời, chỉ trong `__schedule` cuối cùng|task kế tiếp `finish_task_switch` → giải phóng **kernel stack**|
|`EXIT_ZOMBIE`|`task->exit_state`|Kéo dài tới khi CHA `wait`|giữ **vỏ `task_struct`** (pid, exit_code, rusage) để báo cáo|
|`EXIT_DEAD`|`task->exit_state`|Chớp nhoáng, `cmpxchg` guard|`release_task` đang chạy → gỡ hash + free `task_struct`|
|`EXIT_TRACE`|`task->exit_state`|Biến thể khi zombie bị `ptrace`|tracer detach → chuyển tiếp cho parent thật|

---

### Ba chi phí — tại sao "fork nặng" là hiểu nhầm

- **`fork` KHÔNG copy RAM**: chỉ copy page table + đặt read-only. Trang thật copy lazy trong `do_wp_page` khi có ghi. Trong case study này CHA **không tốn một lần copy trang nào** vì CON `execve` trước khi ghi.
- **`mm_B` sống ~vài chục µs**: sinh ở `dup_mm` (bước 6), chết ở `exec_mmap` (bước 14).
- **`ASID 11` tái dùng ba lần**: cấp cho `mm_B`, rồi `mm_C` (bước 14 chỉ đổi nội dung bảng + `local_flush_tlb_asid`), rồi trả về pool ở `exit_mm`.

---

### Khác biệt 5.4 vs bản mới (tổng hợp toàn roadmap)

|Vùng|5.4|Bản mới|
|---|---|---|
|fork|`_do_fork`, `copy_thread_tls`|`kernel_clone`, `copy_thread` (5.10)|
|execve no-return|`flush_old_exec` + `setup_new_exec`|`begin_new_exec` (5.9)|
|syscall entry|`el0_svc_handler`/`el0_svc_common` (asm+C)|`do_el0_svc` từ `entry-common.c` (5.12+)|
|return-to-user|asm `ret_to_user`/`work_pending`|C `exit_to_user_mode()` (5.12+)|
|reap|`put_task_struct`|`put_task_struct_rcu_user` (5.9)|
|COW pte copy|`copy_one_pte`|`copy_present_pte` batch (5.19)|
|scheduler|CFS (`vruntime`, rbtree)|CFS → 6.5; **EEVDF** từ 6.6|
|`set_fs()`/`addr_limit`|còn (`USER_DS`/`KERNEL_DS`, `orig_addr_limit` trong `pt_regs`)|xoá khỏi arm64 (5.18) — dùng TTBR0 PAN|
|`_TIF_NOTIFY_SIGNAL`|chưa|thêm 5.11|

> Tree local `Kernel/K-S800/Src` = **5.10.241** — đứng giữa: đã có `kernel_clone`, `begin_new_exec`, `put_task_struct_rcu_user`, `copy_present_pte`; vẫn còn `orig_addr_limit`, `ret_to_user` asm, CFS. **Mọi ngữ nghĩa vòng đời process — fork COW → exec thay mm → exit zombie → wait reap — giống hệt 5.4**, chỉ khác tên hàm và vị trí code.