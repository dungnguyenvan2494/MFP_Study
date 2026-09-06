## MODULE 28 — Return to User Mode

Sơ đồ: [docs/diagrams/return-to-user-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/return-to-user-arm64.md) — 4 sơ đồ: mọi đường ra hội tụ `ret_to_user`, `do_notify_resume` vòng lặp, `kernel_exit` + `eret`, khi nào không chạy `ret_to_user`.

**File nguồn**: `arch/arm64/kernel/entry.S` (`ret_to_user`, `work_pending`, `finish_ret_to_user`, macro `kernel_exit`), `arch/arm64/kernel/signal.c` (`do_notify_resume`), `arch/arm64/include/asm/thread_info.h` (`_TIF_WORK_MASK`), `kernel/sched/core.c` (`schedule`), `kernel/task_work.c` (`task_work_run`).

### 1. Mọi đường ra EL0 chụm về `ret_to_user`

`el0_svc` (syscall xong), `el0_irq` (IRQ từ EL0), `el0_sync` (page fault / undef / FP trap từ EL0), và `ret_from_fork` (task user mới) — tất cả kết thúc bằng `b ret_to_user`.

```asm
SYM_CODE_START_LOCAL(ret_to_user)
    disable_daif                          // chặn D,A,I,F — kiểm tra cờ nguyên tử
    ldr  x19, [tsk, #TSK_TI_FLAGS]        // x19 = thread_info.flags
    and  x2, x19, #_TIF_WORK_MASK
    cbnz x2, work_pending                 // có việc treo → chậm
finish_ret_to_user:
    user_enter_irqoff                     // context tracking (RCU biết đang rời kernel)
    enable_step_tsk x19, x2               // bật lại single-step nếu ptrace yêu cầu
    // stackleak_erase (nếu bật)
    kernel_exit 0                         // khôi phục pt_regs → eret
work_pending:
    mov  x0, sp                           // 'regs' = pt_regs
    mov  x1, x19                          // 'thread_flags'
    bl   do_notify_resume
    ldr  x19, [tsk, #TSK_TI_FLAGS]        // đọc lại (single-step, cờ mới)
    b    finish_ret_to_user
SYM_CODE_END(ret_to_user)
```

`_TIF_WORK_MASK` = `_TIF_NEED_RESCHED | _TIF_SIGPENDING | _TIF_NOTIFY_RESUME | _TIF_FOREIGN_FPSTATE | _TIF_UPROBE | _TIF_FSCHECK | _TIF_MTE_ASYNC_FAULT | _TIF_NOTIFY_SIGNAL`.

### 2. `do_notify_resume()` — vòng xử lý việc treo

```c
asmlinkage void do_notify_resume(struct pt_regs *regs, unsigned long thread_flags)
{
    do {
        addr_limit_user_check();                       // addr_limit == USER_DS?

        if (thread_flags & _TIF_NEED_RESCHED) {
            local_daif_restore(DAIF_PROCCTX_NOIRQ);
            schedule();                                // NHƯỜNG CPU
        } else {
            local_daif_restore(DAIF_PROCCTX);         // mở cả IRQ

            if (thread_flags & _TIF_UPROBE)
                uprobe_notify_resume(regs);

            if (thread_flags & _TIF_MTE_ASYNC_FAULT)
                send_sig_fault(SIGSEGV, SEGV_MTEAERR, NULL, current);

            if (thread_flags & (_TIF_SIGPENDING | _TIF_NOTIFY_SIGNAL))
                do_signal(regs);                       // giao signal

            if (thread_flags & _TIF_NOTIFY_RESUME) {
                tracehook_notify_resume(regs);         // ptrace stop, task_work_run, __fput trễ
                rseq_handle_notify_resume(NULL, regs);
            }

            if (thread_flags & _TIF_FOREIGN_FPSTATE)
                fpsimd_restore_current_state();        // nạp lại V0–V31/FPCR/FPSR
        }

        local_daif_mask();
        thread_flags = READ_ONCE(current_thread_info()->flags);
    } while (thread_flags & _TIF_WORK_MASK);
}
```

Từng loại việc:

- **`_TIF_NEED_RESCHED`**: `schedule()` — task này nhường CPU. Khi được chọn lại, vòng `do…while` tiếp tục. Đây là điểm reschedule "chủ động" của task sắp về user (ví dụ IRQ timer trước đó đã `set_tsk_need_resched`).
- **`_TIF_SIGPENDING` / `_TIF_NOTIFY_SIGNAL`**: `do_signal(regs)` → `get_signal()`. Nếu có handler: `setup_rt_frame()` đẩy `struct rt_sigframe` (siginfo + ucontext chứa bản sao `pt_regs`) lên **stack user**, rồi `setup_return()` sửa `pt_regs`: `pc = sa_handler`, `sp = frame`, `regs[0] = signo`, `regs[30] (lr) = __kernel_rt_sigreturn` (trampoline vDSO). Nếu `SIG_DFL` + fatal → `do_group_exit()` (không quay lại).
- **`_TIF_NOTIFY_RESUME`**: `tracehook_notify_resume` → `task_work_run()` — nơi công việc hoãn lại chạy: `fput` trễ (đóng file cuối cùng), ptrace-stop, keyring GC, `io_uring` cleanup. Cũng xử lý `rseq` (restartable sequences).
- **`_TIF_FOREIGN_FPSTATE`**: FPSIMD state của task bị đánh dấu "không nằm trên CPU này" (sau context switch, hoặc kernel vừa dùng FPU) → `fpsimd_restore_current_state()` nạp lại V-registers từ `thread.uw.fpsimd_state`.

Vòng `do…while` cần thiết vì xử lý một việc có thể sinh việc khác: `schedule()` quay về có thể có signal mới; `do_signal` có thể lại raise `NEED_RESCHED`. Chỉ thoát khi `flags & _TIF_WORK_MASK == 0`.

### 3. `kernel_exit 0` — khôi phục và `eret`

```asm
kernel_exit 0:
    disable_daif
    ldp x21, x22, [sp, #S_PC]        // x21 = pt_regs->pc, x22 = pt_regs->pstate
    ldr x23, [sp, #S_SP]
    msr sp_el0, x23                  // SP_EL0 = pt_regs->sp (stack user)
    ptrauth_keys_install_user tsk    // đổi PAC key về của user
    msr elr_el1, x21                // ← địa chỉ CPU sẽ nhảy tới
    msr spsr_el1, x22              // ← PSTATE (gồm 'target EL')
    ldp x0, x1, [sp, #16*0]          // khôi phục x0..x29
    ... ldp x28, x29 ...
    ldr lr, [sp, #S_LR]             // x30
    add sp, sp, #S_FRAME_SIZE        // nhả pt_regs; SP_EL1 về đỉnh kernel stack
    eret
```

`eret` làm (phần cứng):

1. `PSTATE ← SPSR_EL1` — field `M[3:0]` chọn EL đích và SP: `0b0000` = **EL0t** (về user, dùng SP_EL0). Đồng thời khôi phục NZCV, DAIF, bit SS (single-step), nRW (AArch64 vs AArch32).
2. `PC ← ELR_EL1`.
3. CPU chạy tiếp tại `PC` mới, ở EL mới. `x0` = giá trị trả syscall (hoặc `signo` nếu vừa vào signal handler).

|Thanh ghi|Nạp từ|Vai trò trong `eret`|
|---|---|---|
|`ELR_EL1`|`pt_regs->pc`|**PC mới** — lệnh đầu tiên chạy sau `eret`|
|`SPSR_EL1`|`pt_regs->pstate`|**PSTATE mới** — `M[3:0]` chọn EL đích + SP; khôi phục cờ NZCV/DAIF/SS|

Nếu `SPSR.M == EL1h` (như IRQ lồng trong kernel, `kernel_exit 1`) → `eret` quay lại **kernel**, tiếp lệnh bị ngắt. Cùng một lệnh `eret`, đích do `SPSR_EL1` quyết định.

### 4. Khi nào KHÔNG chạy `ret_to_user`

- **Vào kernel từ EL0**: luôn kết thúc bằng `ret_to_user` → kiểm `_TIF_WORK_MASK` → `kernel_exit 0` → `eret` EL0. Signal & reschedule (từ EL0) được xử lý **ở đây**, đúng ranh giới user.
- **Vào kernel từ EL1** (IRQ / fault khi kernel đang chạy): `el1_irq`/`el1_sync` → xử lý → `kernel_exit 1`. **Không** kiểm signal/notify-resume (task chưa sắp về user). Chỉ preempt nếu `CONFIG_PREEMPT` && `preempt_count == 0` && `_TIF_NEED_RESCHED` (`el1_preempt` → `preempt_schedule_irq`).

Quy tắc: cờ (`SIGPENDING`, `NEED_RESCHED`, `NOTIFY_RESUME`) được **đặt** bất cứ lúc nào (từ IRQ handler, từ syscall khác, từ `wake_up`), nhưng chỉ được **xử lý** khi kernel chuẩn bị trả điều khiển cho userspace — tại `ret_to_user`.

### Điểm dễ nhầm

|Nhầm|Thực tế|
|---|---|
|"signal được giao ngay khi `kill()` chạy"|`kill()` chỉ đặt `sigqueue` + `TIF_SIGPENDING`. Giao xảy ra ở `do_signal` trong `ret_to_user` của task đích.|
|"`eret` luôn về EL0"|Đích do `SPSR_EL1.M` quyết định — có thể EL0t (user) hoặc EL1h (kernel).|
|"reschedule xảy ra ngay khi timer tick"|Tick chỉ `set_tsk_need_resched`. `schedule()` chạy ở `ret_to_user` (EL0) hoặc `el1_preempt` (EL1, nếu CONFIG_PREEMPT).|
|"`do_notify_resume` chạy 1 lần"|Vòng `do…while(flags & _TIF_WORK_MASK)` — lặp tới khi sạch.|
|"`__fput` (đóng file) chạy ngay trong `close()`"|Phần cuối có thể hoãn qua `task_work`, chạy ở `_TIF_NOTIFY_RESUME`.|
|"IRQ từ EL1 luôn preempt"|Chỉ `CONFIG_PREEMPT` + `preempt_count == 0`.|
|"kernel_exit khôi phục cả callee-saved của kernel"|`kernel_exit` khôi phục `pt_regs` (trạng thái **user**). Callee-saved kernel do `cpu_context`/trình biên dịch lo.|

### Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản mới|
|---|---|---|
|`ret_to_user` / `work_pending`|asm trong `entry.S`, gọi `do_notify_resume`|5.12+: chuyển sang C `exit_to_user_mode()` (`entry-common.c`); asm chỉ khôi phục regs + `eret`|
|`addr_limit_user_check()`|có (`set_fs` còn)|bỏ khi `set_fs()` bị xoá (5.18)|
|`_TIF_NOTIFY_SIGNAL`|5.4 gốc chưa có|thêm 5.11 (5.10 local đã có)|
|`_TIF_MTE_ASYNC_FAULT`|chưa (MTE chưa merge)|thêm 5.10|
|KPTI trampoline `tramp_exit_*`|có nếu `CONFIG_UNMAP_KERNEL_AT_EL0`|không đổi|
|`eret` + `ELR_EL1`/`SPSR_EL1`|kiến trúc ARMv8|**không bao giờ đổi**|

> Tree local = **5.10.241**: `ret_to_user`/`work_pending`/`finish_ret_to_user` vẫn asm; `do_notify_resume` đã có `_TIF_NOTIFY_SIGNAL`, `_TIF_MTE_ASYNC_FAULT`. Vòng lặp + `kernel_exit` + `eret` giống 5.4.