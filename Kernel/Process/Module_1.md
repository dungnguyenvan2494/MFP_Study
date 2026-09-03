# 0. Bức tranh một dòng

Trong Linux **không có "đối tượng process" và "đối tượng thread" riêng**. Chỉ có **`task_struct`** (gọi là _task_). Cái mà scheduler xếp lịch luôn là một `task_struct`. "Process" của POSIX = một **thread group** (tập `task_struct` cùng `tgid`). "Thread" của POSIX = một `task_struct` chia sẻ `mm/files/fs/sighand/signal` với các task cùng nhóm.

Vòng đời:

```
ELF file ──execve()──▶ load_elf_binary ──start_thread()──▶ [EL0] user code
   [EL0] ──svc/IRQ/abort──▶ vector table (entry.S) ──kernel_entry──▶ pt_regs @ đỉnh kernel stack
   ──▶ el0_svc_handler ──▶ sys_call_table[nr](regs) ──▶ ret_to_user
        │                                                    │
        ├─ TIF_NEED_RESCHED ─▶ schedule() ─▶ __schedule ─▶ context_switch
        │        ─▶ switch_mm (TTBR0/ASID) + __switch_to ─▶ cpu_switch_to (cpu_context)
        └─ TIF_SIGPENDING ──▶ do_notify_resume ─▶ do_signal ─▶ setup_rt_frame (user stack)
   [EL0] ──exit()/exit_group()──▶ do_exit ─▶ exit_notify ─▶ EXIT_ZOMBIE
   cha: ──wait4()/waitid()──▶ do_wait ─▶ wait_task_zombie ─▶ release_task ─▶ free task_struct + kernel stack
```

---

# 1. Process vs Thread trong Linux

## 1.1. Đơn vị cơ bản: task

|Khái niệm POSIX/userspace|Trong kernel|
|---|---|
|Thread|1 `task_struct`. `task->pid` là **TID** thật.|
|Process|1 **thread group**: mọi `task_struct` có cùng `task->tgid`. Task có `pid == tgid` là **`group_leader`**.|
|`getpid()`|trả `current->tgid`|
|`gettid()`|trả `current->pid`|
|PID trong `/proc`|`/proc/<tgid>/`, các thread ở `/proc/<tgid>/task/<pid>/`|

## 1.2. Cả hai đều tạo bằng `clone()` — chỉ khác cờ

`glibc` gọi lời gọi hệ thống `clone` với các cờ khác nhau (`kernel/fork.c` → `_do_fork()` → `copy_process()`):

|Cách tạo|Cờ `clone` chính|Kết quả|
|---|---|---|
|`fork()`|`SIGCHLD` (không CLONE_*)|Process mới. `mm`, `files`, `fs`, `sighand`, `signal` đều là **bản sao riêng** (mm dùng COW). `tgid` mới.|
|`vfork()`|`CLONE_VM \| CLONE_VFORK \| SIGCHLD`|Chia sẻ `mm`, cha bị treo tới khi con `execve`/`exit`.|
|`pthread_create()`|`CLONE_VM \| CLONE_FS \| CLONE_FILES \| CLONE_SIGHAND \| CLONE_THREAD \| CLONE_SYSVSEM \| CLONE_SETTLS \| CLONE_PARENT_SETTID \| CLONE_CHILD_CLEARTID`|Thread mới **chia sẻ con trỏ** tới cùng `mm_struct`, `files_struct`, `fs_struct`, `sighand_struct`, `signal_struct`. Cùng `tgid`.|

Ràng buộc kiểm tra trong `copy_process()`: `CLONE_THREAD` ⇒ phải có `CLONE_SIGHAND` ⇒ phải có `CLONE_VM`.

## 1.3. "Chia sẻ" cụ thể ở đâu

Trong `copy_process()` (kernel/fork.c) có chuỗi `copy_*`:

|Hàm|Không có cờ →|Có cờ →|
|---|---|---|
|`copy_mm()`|`dup_mm()`: tạo `mm_struct` mới, sao chép VMA, đánh dấu COW|`CLONE_VM`: chỉ `mmget(oldmm)` — **cùng một `mm_struct`**|
|`copy_files()`|`dup_fd()`: bảng fd mới|`CLONE_FILES`: `atomic_inc(&files->count)`|
|`copy_fs()`|`copy_fs_struct()`|`CLONE_FS`: `fs->users++`|
|`copy_sighand()`|`kmem_cache_alloc` bảng `action[]` mới|`CLONE_SIGHAND`: `refcount_inc`|
|`copy_signal()`|`signal_struct` mới|`CLONE_THREAD`: dùng lại `signal_struct` của leader; `list_add_tail(&p->thread_group, ...)`|

## 1.4. Ngữ nghĩa tín hiệu (hệ quả của thiết kế trên)

- `kill(tgid, sig)` (gửi tới _process_): vào `signal->shared_pending`. **Bất kỳ** thread nào không block `sig` sẽ nhận.
- `tgkill(tgid, tid, sig)` (gửi tới _thread_): vào `task->pending` của đúng task đó.
- Tín hiệu chí mạng / `exit_group()`: `zap_other_threads()` gửi `SIGKILL` cho **toàn bộ** nhóm → cả process chết.
- `sighand_struct` (bảng `sigaction`) và `signal_struct` (thống kê, rlimit, group state) **dùng chung cả nhóm**; `task->pending` (hàng đợi tín hiệu riêng) là **per-thread**.

---

# 2. Vì sao kernel dùng chung `task_struct` cho cả process và thread

1. **Mô hình luồng 1:1 (NPTL)**: từ ~2.6, mỗi user thread ⇔ đúng một kernel task. Không có M:N. Vậy "đơn vị lập lịch" và "luồng người dùng" là **cùng một thứ** → chỉ cần một loại đối tượng.
    
2. **Chia sẻ bằng con trỏ + refcount, không bằng "chứa"**. Thay vì "một process object chứa danh sách thread object", Linux làm: `task_struct` trỏ tới các cấu trúc tài nguyên (`mm_struct`, `files_struct`, `fs_struct`, `sighand_struct`, `signal_struct`); nhiều `task_struct` cùng trỏ vào một cấu trúc và tăng refcount (`mm_users`, `mm_count`, `files->count`, …). Nhờ vậy:
    
    - Mọi đường code lõi (`schedule()`, `__switch_to()`, `do_exit()`, `ptrace`, `/proc`) chỉ thao tác trên `task_struct` — **không phải rẽ nhánh** "đây là process hay thread".
    - "Process" chỉ là _khung nhìn tổng hợp_: duyệt `signal->thread_head` / `while_each_thread()` để gom các task cùng `tgid`.
3. **Kernel thread cũng là `task_struct`** (`task->mm == NULL`, chạy trên `active_mm` mượn của task trước). Cha của chúng là `kthreadd` (PID 2). Không cần loại đối tượng thứ ba.
    
4. **Lịch sử**: `task_struct` có từ đầu. Khi thêm hỗ trợ đa luồng POSIX, thay vì phát minh "thread object", kernel chỉ thêm `CLONE_THREAD` + trường `tgid` + cho dùng chung `signal_struct`, để "một đống task" _hành xử_ như một process trước mắt userspace (chung PID, chung tín hiệu, `exit_group` giết cả nhóm).
    

> Đối lập: `thread_info` là thứ **luôn per-task** (cờ lập lịch cấp thấp). `task_struct` là "task"; `signal_struct` là "process".

---

# 3. Đi qua từng subsystem theo luồng

## 3.1. ELF executable → `execve()`

**Generic** (`fs/exec.c`, `fs/binfmt_elf.c`):

```
sys_execve → do_execve → do_execveat_common → __do_execve_file
   ├─ bprm_mm_init()        : mm_alloc() — tạo mm_struct MỚI, chưa gắn vào task
   ├─ prepare_binprm()      : đọc 128 byte đầu file vào bprm->buf
   ├─ search_binary_handler : duyệt formats → load_elf_binary()
   │     ├─ đọc program headers, kiểm tra ELF magic / EI_CLASS
   │     ├─ flush_old_exec() → exec_mmap(): GẮN mm mới vào current->mm,
   │     │                     mmput() mm cũ (giải phóng address space cũ)
   │     ├─ setup_new_exec(), arch_pick_mmap_layout()
   │     ├─ setup_arg_pages(): tạo VMA stack (VM_GROWSDOWN), copy argv/envp
   │     ├─ elf_map(): mmap từng PT_LOAD (text r-x, data rw-, bss)
   │     ├─ (nếu PIE/động) map interpreter ld.so
   │     ├─ create_elf_tables(): dựng argc/argv/envp/auxv trên user stack
   │     └─ start_thread(regs, elf_entry, bprm->p)   ◀── ARM64-specific
   └─ (không có PT_INTERP → nhảy thẳng elf_entry; có → nhảy ld.so)
```

**ARM64-specific** — `start_thread()` (`arch/arm64/include/asm/processor.h`):

```c
static inline void start_thread(struct pt_regs *regs, unsigned long pc, unsigned long sp)
{
    start_thread_common(regs, pc);            // memset pt_regs; regs->pstate = PSR_MODE_EL0t
    regs->pstate |= PSR_...;                  // AArch64, IRQ bật
    regs->pc = pc;                            // = ELF e_entry (hoặc ld.so)
    regs->sp = sp;                            // = đỉnh user stack đã dựng argv/env/auxv
}
```

`execve` **không tạo task mới** — nó thay ruột task hiện tại: `mm_struct` mới, `pt_regs` (đỉnh kernel stack) được ghi đè để khi `ret_to_user` → `kernel_exit` → `ERET`, CPU nhảy vào `e_entry` ở EL0. `files_struct` giữ nguyên (trừ fd có cờ `O_CLOEXEC`), `sighand` reset về `SIG_DFL`.

## 3.2. User mode (EL0)

Chương trình chạy ở **EL0** (Exception Level 0). MMU dùng **`TTBR0_EL1`** cho ánh xạ user (nửa dưới không gian ảo), **`TTBR1_EL1`** cho kernel (nửa trên, `0xffff…`) — bảng kernel **không đổi** khi context switch. `current` được đọc từ **`SP_EL0`** (arm64 dùng `SP_EL0` để lưu con trỏ `task_struct` của task đang chạy khi ở EL1).

## 3.3. User → kernel: syscall / interrupt / exception

**Cửa vào duy nhất** là **vector table** trỏ bởi `VBAR_EL1`, định nghĩa trong **`arch/arm64/kernel/entry.S`** (ARM64-specific, assembly thuần ở 5.4). 16 entry: {EL1t, EL1h, EL0 AArch64, EL0 AArch32} × {sync, IRQ, FIQ, SError}.

Với syscall: lệnh `svc #0` ở EL0 → CPU nhảy tới vector `el0_sync`:

```
el0_sync (entry.S):
   kernel_entry 0                 ; macro: lưu x0..x30, rồi sp (SP_EL0), pc (ELR_EL1),
                                  ; pstate (SPSR_EL1) vào struct pt_regs ở ĐỈNH kernel stack;
                                  ; nạp SP_EL1 = đỉnh - sizeof(pt_regs); nạp task vào SP_EL0
   mrs x25, esr_el1              ; đọc Exception Syndrome Register
   lsr x24, x25, #ESR_ELx_EC_SHIFT
   cmp x24, #ESR_ELx_EC_SVC64   ; là syscall?
   b.eq  el0_svc
   cmp x24, #ESR_ELx_EC_DABT_LOW ; data abort từ EL0?
   b.eq  el0_da
   ...                          ; el0_ia, el0_fpsimd_acc, el0_sve_acc, el0_undef, el0_sys, el0_dbg...

el0_svc:
   mov x0, sp                   ; x0 = con trỏ pt_regs
   bl  el0_svc_handler          ; ◀── chuyển sang C
   b   ret_to_user
```

**ARM64-specific** — `el0_svc_handler()` / `el0_svc_common()` (`arch/arm64/kernel/syscall.c`):

```c
static void el0_svc_common(struct pt_regs *regs, int scno, int sc_nr, const syscall_fn_t syscall_table[])
{
    regs->orig_x0 = regs->regs[0];      // lưu để restart syscall nếu bị signal cắt
    regs->syscallno = scno;             // scno = regs->regs[8]
    ...
    if (thread_flags & _TIF_SYSCALL_WORK)   // ptrace/seccomp/audit
        scno = syscall_trace_enter(regs);
    invoke_syscall(regs, scno, sc_nr, syscall_table);
        → __invoke_syscall: regs->regs[0] = syscall_fn(regs);   // gọi sys_xxx(), kết quả về x0
    ...
    syscall_trace_exit(regs);
}
```

`sys_call_table[]` (generic, sinh từ `include/uapi/asm-generic/unistd.h` + `arch/arm64/include/asm/unistd.h`) trỏ tới các `SYSCALL_DEFINEn(...)` (**generic**, ở `kernel/`, `fs/`, `mm/`…). Tham số syscall = `regs->regs[0..5]`, số syscall = `regs->regs[8]`, giá trị trả về ghi vào `regs->regs[0]`.

**IRQ**: `el0_irq`/`el1_irq` → `kernel_entry` → `irq_handler` → chuyển sang **IRQ stack** per-CPU → `handle_arch_irq` → GIC driver → `generic_handle_irq`. Timer tick trong đó gọi `scheduler_tick()` → có thể set `TIF_NEED_RESCHED`.

**Page fault**: `el0_da`/`el1_da` → `do_mem_abort()` → `do_page_fault()` (`arch/arm64/mm/fault.c`, **ARM64**) → `handle_mm_fault()` (`mm/memory.c`, **generic**). `FAR_EL1` → `current->thread.fault_address`, `ESR_EL1` → `current->thread.fault_code`.

## 3.4. `ret_to_user` — điểm rẽ nhánh

Sau khi syscall handler xong, `el0_svc` `b ret_to_user` (entry.S):

```
ret_to_user:
   ldr  x1, [tsk, #TSK_TI_FLAGS]        ; thread_info.flags
   and  x2, x1, #_TIF_WORK_MASK         ; NEED_RESCHED | SIGPENDING | NOTIFY_RESUME | FOREIGN_FPSTATE
   cbnz x2, work_pending
finish_ret_to_user:
   kernel_exit 0                        ; khôi phục x0..x30, SP_EL0, ELR_EL1←pc, SPSR_EL1←pstate; ERET

work_pending:
   mov  x0, sp                          ; pt_regs
   mov  x1, x24                          ; thread flags
   bl   do_notify_resume                ; arch/arm64/kernel/signal.c
   b    ret_to_user                     ; lặp lại tới khi hết work
```

**ARM64** — `do_notify_resume(regs, thread_flags)`:

```c
do {
    if (thread_flags & _TIF_NEED_RESCHED)
        schedule();                                  // ◀── vào scheduler
    else {
        if (thread_flags & _TIF_SIGPENDING)
            do_signal(regs);                         // ◀── xử lý signal
        if (thread_flags & _TIF_NOTIFY_RESUME)
            tracehook_notify_resume(regs);
        if (thread_flags & _TIF_FOREIGN_FPSTATE)
            fpsimd_restore_current_state();
    }
    thread_flags = READ_ONCE(current_thread_info()->flags);
} while (thread_flags & _TIF_WORK_MASK);
```

## 3.5. Scheduler

**Generic** (`kernel/sched/core.c`, `kernel/sched/fair.c`):

```
schedule() → __schedule(preempt=false):
   prev = rq->curr;
   if (prev là voluntary sleep) deactivate_task(prev)      // gỡ khỏi runqueue
   next = pick_next_task(rq, prev, &rf);
        └─ fast path: fair_sched_class.pick_next_task → pick_next_task_fair()
              └─ chọn sched_entity trái nhất trong cây đỏ-đen theo vruntime nhỏ nhất
   if (prev != next)
       rq->curr = next;
       context_switch(rq, prev, next, &rf);               // ◀──
   else
       vẫn giữ prev
```

`sched_entity se` nhúng trong `task_struct` là "nút" của CFS. `scheduler_tick()` (từ timer IRQ) cập nhật `vruntime`, so với `sysctl_sched_min_granularity` → set `TIF_NEED_RESCHED` nếu prev đã chạy đủ lâu. Preempt kernel (nếu `CONFIG_PREEMPT`) cũng gọi `__schedule(true)` từ `preempt_enable()`.

## 3.6. Context switch

**Generic** — `context_switch()` (`kernel/sched/core.c`):

```c
if (!next->mm) {                         // next là kernel thread
    next->active_mm = prev->active_mm;
    mmgrab(prev->active_mm);
    enter_lazy_tlb(prev->active_mm, next);   // KHÔNG đụng TTBR0
} else {
    switch_mm_irqs_off(prev->active_mm, next->mm, next);   // ◀── đổi address space
}
if (!prev->mm) {                         // prev là kernel thread rời đi
    prev->active_mm = NULL;
    rq->prev_mm = oldmm;                 // mmdrop sau
}
...
switch_to(prev, next, prev);             // ◀── đổi register/stack context
barrier();
return finish_task_switch(prev);         // CHẠY TRONG NGỮ CẢNH 'next'
```

### (a) `switch_mm` — ARM64 (`arch/arm64/mm/context.c`, `proc.S`)

```
switch_mm_irqs_off → switch_mm → __switch_mm → check_and_switch_context(mm, cpu):
   - Lấy ASID cho mm từ atomic64 mm->context.id
   - Nếu ASID generation cũ → __new_context(): cấp ASID mới; nếu cạn 16-bit ASID space
     → flush_context() + rollover, broadcast TLBI trên mọi CPU
   - cpu_switch_mm(mm->pgd, mm) → cpu_do_switch_mm (proc.S):
        msr ttbr0_el1, <pgd_phys | ASID<<48>       ; chỉ đổi TTBR0 (user)
        isb
```

`TTBR1_EL1` (kernel) không đổi. ASID để **không phải flush toàn bộ TLB** mỗi lần switch.

### (b) `__switch_to` — ARM64 (`arch/arm64/kernel/process.c`)

```c
__notrace_funcgraph struct task_struct *__switch_to(struct task_struct *prev,
                                                    struct task_struct *next)
{
    fpsimd_thread_switch(next);        // lưu FP/SIMD của prev nếu "dirty", set TIF_FOREIGN_FPSTATE cho next (lazy)
    tls_thread_switch(next);           // lưu TPIDR_EL0 (prev) → prev->thread.uw.tp_value; nạp cho next; TPIDRRO_EL0
    hw_breakpoint_thread_switch(next); // debug registers (DBGBVR/DBGWVR)
    contextidr_thread_switch(next);    // CONTEXTIDR_EL1 (tracing)
    entry_task_switch(next);           // __this_cpu_write(__entry_task, next)
    uao_thread_switch(next);           // PSTATE.UAO
    ssbs_thread_switch(next);          // spec-store-bypass state

    dsb(ish);                          // đảm bảo mọi lưu trữ thấy trước khi đổi stack

    last = cpu_switch_to(prev, next);  // ◀── assembly, entry.S
    return last;                       // = 'prev' của lần switch KẾ TIẾP (tham số thứ 3 của switch_to)
}
```

### (c) `cpu_switch_to` — ARM64 (`arch/arm64/kernel/entry.S`) — trái tim của switch

```
cpu_switch_to:
   mov  x10, #THREAD_CPU_CONTEXT        ; offset của thread.cpu_context trong task_struct (asm-offsets.c)
   add  x8, x0, x10                     ; &prev->thread.cpu_context
   mov  x9, sp
   stp  x19, x20, [x8], #16             ; --- LƯU callee-saved của prev ---
   stp  x21, x22, [x8], #16
   stp  x23, x24, [x8], #16
   stp  x25, x26, [x8], #16
   stp  x27, x28, [x8], #16
   stp  x29, x9,  [x8], #16             ; fp, sp
   str  lr,       [x8]                  ; pc = địa chỉ trở về (điểm prev gọi schedule)

   add  x8, x1, x10                     ; &next->thread.cpu_context
   ldp  x19, x20, [x8], #16             ; --- NẠP callee-saved của next ---
   ...
   ldp  x29, x9,  [x8], #16
   ldr  lr,       [x8]
   mov  sp, x9                          ; ◀── ĐỔI KERNEL STACK = ĐỔI TASK
   msr  sp_el0, x1                      ; ◀── current = next
   ret                                  ; nhảy về 'pc' đã nạp của next
```

Điểm cốt lõi: **cái được swap chỉ là `struct cpu_context` bé xíu** (x19–x28, fp, sp, pc). Vì `__switch_to` là hàm C bình thường, các thanh ghi _caller-saved_ (x0–x18) đã nằm trên stack theo AAPCS64 — đổi `sp` là tự động lấy lại chúng đúng của `next`. Trạng thái EL0 đầy đủ **không** ở đây — nó ở `pt_regs`.

### (d) Kết thúc — `finish_task_switch()` (generic)

Chạy **trong ngữ cảnh `next`** (vì `sp`/`pc` đã đổi):

- `mmdrop(rq->prev_mm)` nếu prev là kernel thread.
- Nếu `prev->state == TASK_DEAD` → `put_task_struct_rcu_user(prev)` (giải phóng task đã `do_exit`).
- Với task **mới fork**: `pc` của nó trỏ tới **`ret_from_fork`** (entry.S), không phải giữa `__schedule`. `ret_from_fork` → `bl schedule_tail` (gọi `finish_task_switch`) → nếu là kernel thread thì `blr x19` (gọi `fn(arg)`), nếu là user task thì restore regs từ `pt_regs` và `b ret_to_user` → `ERET` về EL0. Con của `fork()` thấy giá trị trả về `0` vì `copy_thread` đã set `childregs->regs[0] = 0`.

## 3.7. Signal

**Generic** (`kernel/signal.c`): `__send_signal()` đưa `struct sigqueue` vào `task->pending` (per-thread) hoặc `signal->shared_pending` (cả nhóm), set `TIF_SIGPENDING`, `signal_wake_up()`.

Khi về `ret_to_user` với `TIF_SIGPENDING` → `do_notify_resume` → **ARM64** `do_signal(regs)` (`arch/arm64/kernel/signal.c`):

```c
while (get_signal(&ksig)) {              // generic: dequeue, xử lý SIG_DFL/SIG_IGN/stop; nếu fatal → do_group_exit (không trả về)
    ...
    handle_signal(&ksig, regs);
       ├─ nếu syscall bị cắt: xử lý -ERESTARTSYS/-ERESTART_RESTARTBLOCK (regs->pc -= 4; regs->regs[0] = regs->orig_x0)
       ├─ setup_rt_frame(usig, &ksig, oldset, regs):     ◀── ARM64
       │     - cấp chỗ trên USER stack: struct rt_sigframe (siginfo + ucontext)
       │     - setup_sigframe(): lưu regs[0..30], sp, pc, pstate + FPSIMD context + (SVE, ESR) vào sigcontext
       │     - regs->regs[0] = signo;  regs->regs[1] = &info;  regs->regs[2] = &uc
       │     - regs->sp  = (unsigned long)frame
       │     - regs->pc  = ksig->ka.sa.sa_handler
       │     - regs->regs[30] (LR) = sigtramp  (vDSO __kernel_rt_sigreturn, hoặc sa_restorer)
       └─ signal_setup_done()
}
if (!handled) restore_saved_sigmask();
```

`ERET` → handler chạy ở EL0. Khi handler `ret` → nhảy vào `__kernel_rt_sigreturn` → `svc` gọi **`sys_rt_sigreturn()`** → `restore_sigframe(regs, frame)` khôi phục `pt_regs` từ user stack → `ret_to_user` → `ERET` về đúng chỗ user bị ngắt. FP/SIMD của signal handler được cô lập rồi khôi phục.

## 3.8. `exit()`

**Generic** (`kernel/exit.c`):

```
exit_group(code)  →  do_group_exit(code):
   signal->group_exit_code = code;
   zap_other_threads(current);         // gửi SIGKILL cho mọi task cùng tgid
   do_exit(code);

_exit(code)       →  do_exit(code):
   exit_signals(tsk);                  // set PF_EXITING; nếu là thread cuối → group dead
   exit_mm();                          // mmput(mm): nếu mm_users→0 → exit_mmap() giải phóng mọi VMA + trang;
                                       //           mmdrop sau đó khi mm_count→0 → free pgd + mm_struct
   exit_sem(); exit_shm();
   exit_files(tsk);                    // put_files_struct: nếu count→0 → đóng mọi fd
   exit_fs(tsk);
   exit_task_namespaces();
   exit_thread(tsk);                   // ◀── ARM64: fpsimd_flush_task_state, giải phóng SVE state
   perf_event_exit_task(); cgroup_exit();
   exit_notify(tsk):
       forget_original_parent()        // reparent con: giao cho subreaper hoặc init (PID 1)
       tsk->exit_state = EXIT_ZOMBIE   // (hoặc EXIT_DEAD + release_task ngay nếu autoreap / không ai wait)
       do_notify_parent(tsk, tsk->exit_signal)   // gửi SIGCHLD; đánh thức parent trên signal->wait_chldexit
   tsk->state = TASK_DEAD;
   do_task_dead()  →  __schedule(false);   // KHÔNG BAO GIỜ TRỞ LẠI
```

Task lúc này ở `EXIT_ZOMBIE`: `task_struct` + kernel stack **vẫn còn**, chỉ giữ `exit_code`/rusage để cha đọc. `mm`, `files`, `fs` đã nhả.

## 3.9. `wait()`

**Generic** (`kernel/exit.c`):

```
sys_wait4 / sys_waitid  →  kernel_wait4  →  do_wait(struct wait_opts *wo):
   add_wait_queue(&current->signal->wait_chldexit, &wo->child_wait);
   repeat:
     __wo_state = TASK_INTERRUPTIBLE;
     duyệt current->children (và ->ptraced):
        wait_consider_task(wo, ptrace, p):
           if (p->exit_state == EXIT_ZOMBIE)
               wait_task_zombie(wo, p):
                   - đọc p->exit_code → status trả về userspace; cộng rusage
                   - p->exit_state = EXIT_DEAD
                   - release_task(p):
                        __exit_signal(p)          // gỡ khỏi signal_struct, cộng dồn thống kê
                        __unhash_process(p):
                           detach_pid(p, PIDTYPE_PID/TGID [+PGID/SID nếu group_leader])
                           list_del_rcu(&p->tasks)          // rời process list
                           list_del(&p->thread_group), ->sibling
                        release_thread(p)          // ◀── ARM64: no-op
                        put_task_struct_rcu_user(p)
                           → khi refcount=0: free_task(p):
                                free_thread_stack(p)   // vfree vùng vmap 16KB (VMAP_STACK)
                                free_task_struct(p)    // kmem_cache_free(task_struct_cachep)
     if (không có con nào reap được && vẫn còn con):
        schedule();  goto repeat;           // ngủ tới khi SIGCHLD đánh thức
   remove_wait_queue(...)
```

Nếu cha chết trước con → con được **reparent** cho subreaper gần nhất hoặc **init (PID 1)**; init chạy vòng `wait()` liên tục nên zombie luôn được dọn.

---

# 4. Bốn cấu trúc bắt buộc phải nắm

## 4.1. `mm_struct` — không gian địa chỉ ảo user

- **File**: `include/linux/mm_types.h` (generic); arch phần `mm_context_t` ở `arch/arm64/include/asm/mmu.h`.
- **Trường then chốt**:

|Trường|Ý nghĩa|
|---|---|
|`pgd_t *pgd`|bảng trang gốc → nạp vào **`TTBR0_EL1`**|
|`struct vm_area_struct *mmap` + `struct rb_root mm_rb`|danh sách + cây đỏ-đen các VMA|
|`mmap_base`, `task_size`|layout; `task_size ≈ TASK_SIZE` (giới hạn user, 2^39/2^48 tùy VA_BITS)|
|`start_code/end_code`, `start_data/end_data`, `start_brk/brk`, `start_stack`, `arg_start/arg_end/env_start/env_end`|các mốc do `execve` dựng|
|`atomic_t mm_users`|số **task** đang dùng mm (thread cùng process). 0 → `exit_mmap()`|
|`atomic_t mm_count`|số "tham chiếu giữ" (kernel thread mượn `active_mm`, …). 0 → `__mmdrop()` free `pgd` + `mm_struct`|
|`mm_context_t context`|**ARM64**: `atomic64_t id` (ASID), `void *vdso`, `unsigned long flags`|

- **`task->mm`** = mm sở hữu (NULL với kernel thread). **`task->active_mm`** = mm đang thực sự cài trong TTBR0 (kernel thread mượn của task trước → tránh flush).
- **Vòng đời**: `mm_alloc()` trong `bprm_mm_init` (execve) → `exec_mmap()` gắn → `copy_mm()` chia sẻ (CLONE_VM) → `exit_mm()`/`mmput()` nhả.
- **Đổi khi switch**: `switch_mm_irqs_off` → `check_and_switch_context` (ASID) → ghi `TTBR0_EL1`. Kernel thread: **không đổi TTBR0**.

## 4.2. Kernel stack (và các stack khác)

|Stack|Kích thước / nơi|Ghi chú|
|---|---|---|
|**User stack**|VMA `VM_GROWSDOWN` trong `mm`, gần `TASK_SIZE`; `mm->start_stack`|Dựng ở `execve` (`setup_arg_pages`, `randomize_stack_top`). `sp` user nằm trong `pt_regs`. Tự lớn xuống qua page-fault `expand_stack()`. Giới hạn `RLIMIT_STACK`.|
|**Kernel stack**|`THREAD_SIZE`. ARM64 4K pages: `MIN_THREAD_SHIFT=14` → **16 KB** (64K pages → 64 KB)|Cấp bởi `alloc_thread_stack_node()` (`kernel/fork.c`). Với `VMAP_STACK`: vùng `vmalloc` + **guard page** → tràn → `handle_bad_stack()`. `task->stack` trỏ đáy. `sp_el1` khi vào kernel = đỉnh − `sizeof(pt_regs)`.|
|**IRQ stack**|per-CPU, 16 KB|`arch/arm64/kernel/irq.c` (`irq_stack`), `entry.S` chuyển sang khi xử lý IRQ để không ăn task stack.|
|**Overflow / SDEI stack**|per-CPU, nhỏ|Xử lý stack overflow, SDEI, một số nested exception.|

- **`pt_regs` nằm ở ĐỈNH kernel stack**: `task_pt_regs(p) = (struct pt_regs *)(task->stack + THREAD_SIZE) - 1`.
- **`thread_info` KHÔNG ở trên stack** (arm64 5.4: `CONFIG_THREAD_INFO_IN_TASK` → nhúng đầu `task_struct`; chứa `flags` TIF_*, `preempt_count`, `cpu`, `addr_limit`).
- **`current`** = `read_sysreg(sp_el0)` (không còn suy từ `sp & ~(THREAD_SIZE-1)`).
- Khi switch: `cpu_switch_to` lưu `sp` (EL1) vào `prev->thread.cpu_context.sp`, nạp của `next` → **đổi stack = đổi task**.

## 4.3. `pt_regs` — ảnh chụp CPU tại ranh giới exception

- **File**: `arch/arm64/include/asm/ptrace.h` (ARM64). Bản 5.4:

```c
struct pt_regs {
    union {
        struct user_pt_regs user_regs;
        struct { u64 regs[31]; u64 sp; u64 pc; u64 pstate; };
    };
    u64 orig_x0;        // x0 gốc, để restart syscall
    s32 syscallno;      // số syscall (hoặc -1)
    u32 unused2;
    u64 orig_addr_limit;
    u64 pmr_save;       // GIC PMR (nếu IRQ priority masking)
    u64 stackframe[2];  // frame record giả để unwinder dừng
};
```

(5.10+ thêm `lockdep_hardirqs`, `exit_rcu`; 5.11+ thêm PAC/MTE — **không có ở 5.4**.)

- **Ai lưu**: macro `kernel_entry` trong `entry.S`, **mỗi lần** vào EL1 từ EL0 (hoặc nested từ EL1). Ai khôi phục: `kernel_exit` → `ERET` (`ELR_EL1 ← pc`, `SPSR_EL1 ← pstate`).
- **Dùng để**:
    1. Khôi phục nguyên vẹn ngữ cảnh EL0 khi trở về user.
    2. Syscall: `regs->regs[0..5]` = tham số, `regs->regs[8]` = số syscall, kết quả ghi `regs->regs[0]`.
    3. `ptrace` (`PTRACE_GETREGSET`) cho debugger đọc/ghi thanh ghi user của tracee.
    4. `copy_thread()` khởi tạo `pt_regs` cho task con.
    5. Signal: `setup_sigframe`/`restore_sigframe` lưu/nạp từ đây.
    6. `do_page_fault` nhận `regs` để biết fault ở user hay kernel, in oops.
- **Số lượng**: một `pt_regs` "sống" mỗi lần vào kernel; nested exception → nhiều frame chồng trên kernel stack.

## 4.4. `thread_struct` — ngữ cảnh CPU của task khi KHÔNG chạy

- **File**: `arch/arm64/include/asm/processor.h` (ARM64). Nhúng trong `task_struct` ở trường `thread`. Bản 5.4:

```c
struct thread_struct {
    struct cpu_context cpu_context;   // ◀── x19..x28, fp, sp, pc — cái cpu_switch_to swap
    struct {
        unsigned long tp_value;       // TLS → TPIDR_EL0
        unsigned long tp2_value;
        struct user_fpsimd_state fpsimd_state;   // 32× 128-bit V-regs + FPSR/FPCR
    } uw;                              // "user-writable", whitelist cho hardened usercopy
    unsigned int fpsimd_cpu;
    void *sve_state;  unsigned int sve_vl, sve_vl_onexec;   // SVE (nếu có)
    unsigned long fault_address;       // FAR_EL1 của fault gần nhất
    unsigned long fault_code;          // ESR_EL1
    struct debug_info debug;           // hw breakpoint/watchpoint
    // (5.7+ thêm ptrauth keys; 5.10+ MTE — KHÔNG có ở 5.4)
};

struct cpu_context {
    unsigned long x19, x20, x21, x22, x23, x24, x25, x26, x27, x28;
    unsigned long fp;   // x29
    unsigned long sp;   // kernel sp
    unsigned long pc;   // điểm trở về (nơi task gọi schedule, hoặc ret_from_fork)
};
```

- **Chỉ chứa callee-saved + sp + pc** — vừa đủ cho một _lời gọi hàm C_ (`__switch_to`) "đóng băng" và "rã đông" task. Caller-saved (x0–x18) không ở đây vì chúng đã trên stack theo AAPCS64.
- **`pt_regs` vs `thread_struct.cpu_context`**:

||`pt_regs`|`thread_struct.cpu_context`|
|---|---|---|
|Chụp ở đâu|ranh giới **EL0 ↔ EL1**|điểm task **gọi `schedule()`** (EL1)|
|Nội dung|**toàn bộ** x0–x30, sp, pc, pstate của user|**callee-saved** x19–x28, fp, sp, pc của kernel|
|Nằm ở|đỉnh kernel stack|trong `task_struct`|
|Ai ghi|`kernel_entry` (asm)|`cpu_switch_to` (asm)|
|Mục đích|quay lại user đúng chỗ; syscall ABI; ptrace|context switch tự nguyện/bị ép|

- **`copy_thread()`** (`arch/arm64/kernel/process.c`, 5.4 tên `copy_thread_tls`) thiết lập cả hai cho task con:

```c
int copy_thread_tls(unsigned long clone_flags, unsigned long stack_start,
                    unsigned long stk_sz, struct task_struct *p, unsigned long tls)
{
    struct pt_regs *childregs = task_pt_regs(p);          // đỉnh kernel stack con
    memset(&p->thread.cpu_context, 0, sizeof(struct cpu_context));
    if (likely(!(p->flags & PF_KTHREAD))) {              // task user
        *childregs = *current_pt_regs();                 // sao pt_regs của cha
        childregs->regs[0] = 0;                          // con của fork() trả về 0
        if (stack_start) childregs->sp = stack_start;    // (pthread: stack riêng)
        if (clone_flags & CLONE_SETTLS) p->thread.uw.tp_value = tls;
    } else {                                             // kernel thread
        memset(childregs, 0, sizeof(struct pt_regs));
        childregs->pstate = PSR_MODE_EL1h | ...;
        p->thread.cpu_context.x19 = stack_start;         // fn
        p->thread.cpu_context.x20 = stk_sz;             // arg
    }
    p->thread.cpu_context.pc = (unsigned long)ret_from_fork;   // ◀── task mới "tỉnh dậy" ở đây
    p->thread.cpu_context.sp = (unsigned long)childregs;       // kernel sp bắt đầu ngay dưới pt_regs
}
```

---

# 5. Bảng file nguồn: generic vs ARM64-specific

|Giai đoạn|Generic (`kernel/`, `fs/`, `mm/`, `include/linux/`)|ARM64 (`arch/arm64/…`)|
|---|---|---|
|**Cấu trúc**|`include/linux/sched.h` (`task_struct`), `include/linux/mm_types.h` (`mm_struct`, `vm_area_struct`), `include/linux/sched/signal.h` (`signal_struct`, `sighand_struct`)|`include/asm/processor.h` (`thread_struct`, `cpu_context`, `start_thread`), `include/asm/ptrace.h` (`pt_regs`), `include/asm/thread_info.h` (`thread_info`, `TIF_*`), `include/asm/current.h` (`current` ← `sp_el0`)|
|**Tạo** (`fork`/`clone`)|`kernel/fork.c`: `_do_fork` (5.4), `copy_process`, `dup_task_struct`, `alloc_thread_stack_node`, `copy_mm/files/fs/sighand/signal`, `sched_fork`|`kernel/process.c`: `copy_thread_tls`, `flush_thread`; `kernel/entry.S`: `ret_from_fork`; `kernel/asm-offsets.c`: `THREAD_CPU_CONTEXT`, `TSK_STACK`|
|**execve / ELF**|`fs/exec.c`: `do_execve`, `__do_execve_file`, `search_binary_handler`, `exec_mmap`, `setup_arg_pages`; `fs/binfmt_elf.c`: `load_elf_binary`|`start_thread()` (`include/asm/processor.h`); `flush_thread()` (`kernel/process.c`)|
|**User→kernel** (syscall/IRQ/abort)|`sys_call_table` sinh từ `include/uapi/asm-generic/unistd.h`; các `SYSCALL_DEFINEn`; `kernel/entry/` (chưa dùng cho arm64 ở 5.4)|`kernel/entry.S` (vector table, `kernel_entry`/`kernel_exit`, `el0_sync`, `el0_svc`, `el0_irq`, `ret_to_user`, `work_pending`); `kernel/syscall.c` (`el0_svc_handler`, `el0_svc_common`, `invoke_syscall`); `mm/fault.c` (`do_mem_abort`, `do_page_fault`); `kernel/irq.c`|
|**Scheduler**|`kernel/sched/core.c` (`schedule`, `__schedule`, `context_switch`, `pick_next_task`, `finish_task_switch`, `schedule_tail`, `wake_up_new_task`); `kernel/sched/fair.c` (CFS); `kernel/sched/idle.c`|`switch_to` macro (`include/asm/…`); phần lớn switch nằm ở `__switch_to` + `cpu_switch_to`|
|**Context switch**|`context_switch()`, `switch_mm_irqs_off()` (`include/linux/mmu_context.h`)|`kernel/process.c`: `__switch_to`, `fpsimd_thread_switch`, `tls_thread_switch`; `kernel/entry.S`: `cpu_switch_to`; `mm/context.c`: `check_and_switch_context` (ASID); `mm/proc.S`: `cpu_do_switch_mm` (ghi `TTBR0_EL1`); `kernel/fpsimd.c`|
|**Signal**|`kernel/signal.c`: `get_signal`, `dequeue_signal`, `__send_signal`, `do_group_exit`, `zap_other_threads`|`kernel/signal.c`: `do_notify_resume`, `do_signal`, `handle_signal`, `setup_rt_frame`, `setup_sigframe`, `restore_sigframe`, `sys_rt_sigreturn`; trampoline trong vDSO `kernel/vdso/sigreturn.S`|
|**exit**|`kernel/exit.c`: `do_exit`, `do_group_exit`, `exit_notify`, `forget_original_parent`, `do_notify_parent`, `release_task`, `__unhash_process`|`kernel/process.c`: `exit_thread`, `release_thread` (no-op); `kernel/fpsimd.c`: `fpsimd_flush_task_state`|
|**wait**|`kernel/exit.c`: `kernel_wait4`, `kernel_waitid`, `do_wait`, `wait_consider_task`, `wait_task_zombie`, `wait_task_stopped`|— (thuần generic)|
|**Giải phóng**|`kernel/fork.c`: `free_task`, `free_thread_stack`, `put_task_struct`, `__put_task_struct`; `mm/mmap.c`: `exit_mmap`; `kernel/fork.c`: `__mmdrop`|`free_thread_stack` gọi `vfree` (VMAP_STACK)|

---

# 6. Khác biệt 5.4 so với bản mới (để khỏi lạc khi đọc kernel khác)

|Chủ đề|5.4|Bản sau|
|---|---|---|
|Entry point fork|`_do_fork(struct kernel_clone_args *)`|`kernel_clone()` (5.10)|
|arch copy_thread|`copy_thread_tls(clone_flags, sp, sz, p, tls)`|`copy_thread(clone_flags, sp, sz, p, tls)` gộp lại (5.10)|
|Dispatch `el0_sync`|**Assembly** trong `entry.S`, rẽ theo `ESR_ELx_EC_*`|**C**: `el0_sync_handler()` trong `arch/arm64/kernel/entry-common.c` (5.8)|
|Handler syscall|`el0_svc_handler()`|`do_el0_svc()` (5.8)|
|`struct pt_regs`|kết thúc ở `stackframe[2]`|+ `lockdep_hardirqs`, `exit_rcu` (5.10); + PAC/MTE fields|
|`struct thread_struct`|chưa có `keys_kernel` (in-kernel PAC), `sctlr_tcf0`/`gcr_user_incl` (MTE)|thêm dần 5.7 / 5.10+|
|Shadow Call Stack (x18)|không có|`CONFIG_SHADOW_CALL_STACK` (5.8)|
|`current` qua `SP_EL0`|**đã có** (từ 5.1)|giữ nguyên|
|`THREAD_INFO_IN_TASK`|**đã bật** cho arm64|giữ nguyên|

> Cây nguồn local của bạn (`Kernel/K-S800/Src`) là **5.10.241**, không phải 5.4 — nên nếu đối chiếu, bạn sẽ thấy `kernel_clone`, `do_el0_svc`, `entry-common.c`. Bản chất luồng không đổi, chỉ tên hàm và ranh giới asm/C dịch chuyển như bảng trên.