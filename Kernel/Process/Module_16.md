# MODULE 16 — Signal Delivery trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/signal-delivery-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/signal-delivery-arm64.md) (5 sơ đồ: bên gửi, bên nhận, signal frame trên user stack, `setup_return` trỏ `pt_regs`, vòng khứ hồi handler→trampoline→sigreturn).

**File nguồn**: `kernel/signal.c` (`__send_signal`, `complete_signal`, `signal_wake_up`, `get_signal`, `dequeue_signal`), `arch/arm64/kernel/signal.c` (`do_signal`, `handle_signal`, `setup_rt_frame`, `get_sigframe`, `setup_sigframe`, `setup_return`, `restore_sigframe`, `SYSCALL_DEFINE0(rt_sigreturn)`), `arch/arm64/include/uapi/asm/sigcontext.h` (`struct sigcontext`), `arch/arm64/kernel/vdso/sigreturn.S` (`__kernel_rt_sigreturn`), `arch/arm64/kernel/entry.S` (`ret_to_user`).

---

## 0. Hai câu hỏi trọng tâm

**Signal frame nằm ở đâu?** Trên **USER STACK**, ngay dưới SP user hiện tại (hoặc trên _alternate signal stack_ nếu handler đăng ký `SA_ONSTACK` + `sigaltstack()`). Kernel không cấp bộ nhớ riêng — nó ghi thẳng vào stack của process.

**Register ARM64 được restore thế nào?** `rt_sigreturn` (syscall 139) → `restore_sigframe()` **đọc lại** `x0..x30`, `sp`, `pc`, `pstate`, FP/SIMD từ `uc_mcontext` trên user stack và ghi đè toàn bộ `pt_regs`. Rồi `eret` bình thường dùng `pt_regs` đã khôi phục → process tiếp tục đúng lệnh bị ngắt, đúng thanh ghi.

---

## 1. Bên GỬI — `kill()` chỉ đặt hàng, không chạy handler

`kill(pid, SIGUSR1)` → `sys_kill` → `kill_pid_info` → `group_send_sig_info` → `do_send_sig_info` → `send_signal(sig, info, t, PIDTYPE_TGID)` → **`__send_signal()`** (`kernel/signal.c`), giữ `t->sighand->siglock`:

```c
static int __send_signal(int sig, struct kernel_siginfo *info, struct task_struct *t,
                         enum pid_type type, bool force)
{
    struct sigpending *pending;
    struct sigqueue *q;

    pending = (type != PIDTYPE_PID) ? &t->signal->shared_pending   // kill → cả process
                                    : &t->pending;                  // tgkill → 1 thread

    if (legacy_queue(pending, sig))              // sig ≤ 32 và bit đã bật → tín hiệu chuẩn KHÔNG xếp hàng
        return 0;                                //   (chỉ giữ 1 instance; RT signal 33-64 mới xếp hàng)

    q = __sigqueue_alloc(sig, t, GFP_ATOMIC, ...);
    if (q) {
        list_add_tail(&q->list, &pending->list);
        copy_siginfo(&q->info, info);            // si_code, si_pid, si_uid, ...
    }
    sigaddset(&pending->signal, sig);            // ◀── BẬT BIT trong mặt nạ pending
    complete_signal(sig, t, type);
    return 0;
}
```

`complete_signal(sig, t, type)`:

- Nếu tín hiệu **fatal** (disposition `SIG_DFL` mà giết, không handler, không phải `SIGSTOP`): `signal->flags |= SIGNAL_GROUP_EXIT`; `signal->group_exit_code = sig`; `zap_other_threads(t)` gửi `SIGKILL` cho mọi thread; `signal_wake_up(t, 1)` từng thread.
- Ngược lại: chọn một thread `t` thoả `wants_signal(sig, t)` (không block `sig`, không `PF_EXITING`, `TASK_RUNNING`/`INTERRUPTIBLE`) → `signal_wake_up(t, sig == SIGKILL)`.

`signal_wake_up(t, resume)` → `signal_wake_up_state(t, resume ? TASK_WAKEKILL : 0)`:

```c
set_tsk_thread_flag(t, TIF_SIGPENDING);                       // ◀── cờ mà ret_to_user KIỂM
if (!wake_up_state(t, state | TASK_INTERRUPTIBLE))
    kick_process(t);                                          // t đang chạy CPU khác → IPI reschedule
```

→ `kill()` chỉ: (a) thêm `sigqueue` vào hàng đợi, (b) bật bit trong mặt nạ pending, (c) bật `TIF_SIGPENDING` trên thread đích, (d) đánh thức nó. **Handler không chạy trong ngữ cảnh người gửi.** Nó chạy khi _thread đích_ trở về EL0.

### `signal_struct` vs `sighand_struct`

||`sighand_struct`|`signal_struct`|`task->pending` / `task->blocked`|
|---|---|---|---|
|Giữ|`k_sigaction action[64]` (bảng disposition) + `siglock` (khoá **cả** signal lẫn sighand)|`shared_pending` (tín hiệu gửi tới _process_), `nr_threads`, `group_exit_code`, `flags`, `rlim[]`|`pending`: hàng đợi RIÊNG thread (tgkill, fault); `blocked`: mặt nạ chặn RIÊNG thread|
|Chia sẻ khi|`CLONE_SIGHAND`|`CLONE_THREAD` (= "cùng process")|luôn per-thread|
|Vai trò|"gửi `sig` này thì làm gì"|"trạng thái tín hiệu của cả nhóm"|"tín hiệu chờ riêng thread này" / "thread này chặn gì"|

---

## 2. Bên NHẬN — tại `ret_to_user`

`ret_to_user` (`entry.S`) thấy `_TIF_SIGPENDING` trong `thread_info.flags` → `work_pending` → `do_notify_resume` → **`do_signal(regs)`** (`arch/arm64/kernel/signal.c`).

```c
static void do_signal(struct pt_regs *regs)
{
    unsigned long continue_addr = 0, restart_addr = 0;
    int retval = 0;
    struct ksignal ksig;
    bool syscall = in_syscall(regs);

    if (syscall) {                                    // process bị ngắt GIỮA một syscall
        continue_addr = regs->pc;
        restart_addr  = continue_addr - 4;            // địa chỉ lệnh `svc`
        retval = regs->regs[0];                       // giá trị syscall trả về (-ERESTARTSYS...)
        // nếu syscall trả -ERESTARTSYS/-ERESTARTNOINTR/-ERESTART_RESTARTBLOCK:
        if (retval == -ERESTARTNOINTR ...) {
            regs->regs[0] = regs->orig_x0;
            regs->pc = restart_addr;                  // sẽ chạy lại `svc`
        }
    }

    if (get_signal(&ksig)) {                          // ◀── generic: dequeue + phân giải disposition
        if (regs->pc == restart_addr &&              // syscall bị cắt, có handler:
            (retval == -ERESTARTNOHAND ||
             retval == -ERESTART_RESTARTBLOCK ||
             (retval == -ERESTARTSYS && !(ksig.ka.sa.sa_flags & SA_RESTART)))) {
            regs->regs[0] = -EINTR;                   //   → syscall thất bại với -EINTR
            regs->pc = continue_addr;                 //   → KHÔNG chạy lại svc
        }
        handle_signal(&ksig, regs);                   // ◀── dựng frame + trỏ pt_regs vào handler
        return;
    }

    // KHÔNG có tín hiệu nào có handler:
    if (syscall && regs->pc == restart_addr) {
        if (retval == -ERESTARTNOHAND || retval == -ERESTARTSYS || ...)
            regs->pc = continue_addr;                 // (đã lùi pc, không handler → chạy lại svc là đúng)
    }
    restore_saved_sigmask();                          // khôi phục mặt nạ nếu đang trong pselect/ppoll/sigsuspend
}
```

`get_signal(&ksig)` (generic `kernel/signal.c`):

- Vòng lặp: `dequeue_signal(&current->blocked, &ksig->info, &sig)` — lấy từ `task->pending` **trước**, rồi `signal->shared_pending`; **xoá bit** khỏi mặt nạ pending.
- `ka = &current->sighand->action[sig - 1]` (giữ `siglock`).
    - **`SIG_IGN`** → `continue` (bỏ qua, lấy tín hiệu kế).
    - **`SIG_DFL`**:
        - tín hiệu stop (`SIGSTOP`/`SIGTSTP`/`SIGTTIN`/`SIGTTOU`) → `do_signal_stop()` → job-control stop.
        - kernel-mặc-định-bỏ-qua (`SIGCHLD`, `SIGWINCH`, `SIGURG`) → `continue`.
        - còn lại = **fatal** → `do_coredump()` nếu là tín hiệu coredump (`SIGSEGV`, `SIGABRT`, `SIGQUIT`...), rồi **`do_group_exit(sig)` — KHÔNG TRẢ VỀ**. Đây là chỗ `SIGSEGV`/`SIGKILL`/`SIGTERM` không có handler giết process (Module 17).
    - **có handler** → `ksig->ka = *ka`, `ksig->sig = sig`; nếu `SA_ONESHOT`/`SA_RESETHAND` → `ka->sa.sa_handler = SIG_DFL` (một phát rồi thôi). Trả `true`.

`handle_signal(&ksig, regs)` (ARM64):

```c
sigset_t *oldset = sigmask_to_save();          // = &current->blocked (mặt nạ TRƯỚC khi thêm mặt nạ handler)
...
ret = setup_rt_frame(usig, ksig, oldset, regs);
signal_setup_done(ret, ksig, 0);              // thành công → current->blocked |= ka.sa_mask | sigmask(sig)
                                              //             (trừ khi SA_NODEFER)
```

---

## 3. `setup_rt_frame()` — dựng frame trên user stack

```c
static int setup_rt_frame(int usig, struct ksignal *ksig, sigset_t *set, struct pt_regs *regs)
{
    struct rt_sigframe_user_layout user;
    struct rt_sigframe __user *frame;
    int err = 0;

    get_sigframe(&user, ksig, regs);                    // (a) TÍNH địa chỉ frame trên user stack
    frame = user.sigframe;

    __put_user_error(0, &frame->uc.uc_flags, err);
    __put_user_error(NULL, &frame->uc.uc_link, err);
    err |= __save_altstack(&frame->uc.uc_stack, regs->sp);
    err |= setup_sigframe(&user, regs, set);            // (b) GHI regs/pstate/FPSIMD/mask vào frame

    if (err == 0) {
        setup_return(regs, &ksig->ka, &user, usig);     // (c) TRỎ pt_regs vào handler
        if (ksig->ka.sa.sa_flags & SA_SIGINFO) {
            err |= copy_siginfo_to_user(&frame->info, &ksig->info);
            regs->regs[1] = (unsigned long)&frame->info;   // arg2 = siginfo *
            regs->regs[2] = (unsigned long)&frame->uc;     // arg3 = ucontext *
        }
    }
    return err;
}
```

### (a) `get_sigframe()` — frame nằm ở đâu

```c
sp = sp_top = sigsp(regs->sp, ksig);                    // = regs->sp (SP user),
                                                       //   HOẶC sas_ss_sp + sas_ss_size nếu SA_ONSTACK && altstack đã đặt
sp = round_down(sp - sizeof(struct frame_record), 16);  // 16 byte cho {fp, lr} — bản ghi unwind
user->next_frame = (struct frame_record __user *)sp;
sp = round_down(sp, 16) - sigframe_size(&user);         // sigframe_size ≈ sizeof(rt_sigframe) + vùng FP/SIMD/SVE
user->sigframe = (struct rt_sigframe __user *)sp;
access_ok(user->sigframe, sp_top - sp);                 // kiểm ghi được (nếu không → SIGSEGV)
```

**Layout trên user stack** (địa chỉ cao → thấp):

```
[SP user cũ]  ← regs->sp lúc bị ngắt
   ...
struct frame_record { u64 fp; u64 lr; }     ← user->next_frame   (fp = x29 cũ, lr = x30 cũ → backtrace xuyên qua handler)
   (căn 16)
struct rt_sigframe {
    struct siginfo   info;                   ← (nếu SA_SIGINFO) bản sao siginfo
    struct ucontext  uc {
        u64        uc_flags;
        ucontext  *uc_link;
        stack_t    uc_stack;                 ← altstack cũ
        sigset_t   uc_sigmask;               ← MẶT NẠ CHẶN để khôi phục ở sigreturn (= blocked TRƯỚC handler)
        u8         __unused[...];
        struct sigcontext uc_mcontext {      ← NGỮ CẢNH CPU ĐƯỢC LƯU
            u64 fault_address;
            u64 regs[31];                    ← x0..x30 của process lúc bị ngắt
            u64 sp;                          ← SP user lúc bị ngắt
            u64 pc;                          ← PC user lúc bị ngắt  ← chỗ sẽ quay lại
            u64 pstate;                      ← PSTATE lúc bị ngắt
            u8  __reserved[4096];            ← các khối magic-tag nối tiếp:
                                            //   fpsimd_context (FPSIMD_MAGIC): V0..V31, FPSR, FPCR
                                            //   esr_context (ESR_MAGIC): thread.fault_code  (nếu do fault)
                                            //   sve_context (SVE_MAGIC): nếu có SVE
                                            //   _aarch64_ctx {magic=0, size=0}: kết thúc
        };
    };
}   ← user->sigframe  ← regs->sp MỚI sẽ trỏ vào đây
```

### (b) `setup_sigframe()` — ghi ngữ cảnh vào frame

```c
__put_user_error(regs->regs[29], &user->next_frame->fp, err);   // bản ghi unwind
__put_user_error(regs->regs[30], &user->next_frame->lr, err);

for (i = 0; i < 31; i++)
    __put_user_error(regs->regs[i], &sf->uc.uc_mcontext.regs[i], err);   // x0..x30
__put_user_error(regs->sp,     &sf->uc.uc_mcontext.sp,     err);
__put_user_error(regs->pc,     &sf->uc.uc_mcontext.pc,     err);
__put_user_error(regs->pstate, &sf->uc.uc_mcontext.pstate, err);
__put_user_error(current->thread.fault_address, &sf->uc.uc_mcontext.fault_address, err);
__copy_to_user(&sf->uc.uc_sigmask, set, sizeof(*set));         // set = oldset = blocked TRƯỚC handler

preserve_fpsimd_context(fpsimd_ctx);                          // V0..V31, FPSR, FPCR → khối FPSIMD_MAGIC
// nếu do fault: esr_context (ESR_MAGIC) với thread.fault_code
// nếu SVE: preserve_sve_context()
```

### (c) `setup_return()` — trỏ `pt_regs` vào handler

```c
static void setup_return(struct pt_regs *regs, struct k_sigaction *ka,
                         struct rt_sigframe_user_layout *user, int usig)
{
    __sigrestore_t sigtramp;

    regs->regs[0]  = usig;                              // arg1 = số tín hiệu
    regs->sp       = (unsigned long)user->sigframe;     // ◀── SP user trỏ frame vừa dựng
    regs->regs[29] = (unsigned long)&user->next_frame->fp;   // x29 = &bản ghi unwind
    regs->pc       = (unsigned long)ka->sa.sa_handler;  // ◀── PC = entry của handler

    if (ka->sa.sa_flags & SA_RESTORER)
        sigtramp = ka->sa.sa_restorer;                  // libc cung cấp
    else
        sigtramp = VDSO_SYMBOL(current->mm->context.vdso, sigtramp);  // = __kernel_rt_sigreturn

    regs->regs[30] = (unsigned long)sigtramp;           // ◀── x30 (LR) = trampoline
}
```

`signal_setup_done` sau đó: `current->blocked |= ka->sa.sa_mask | sigmask(sig)` (trừ `SA_NODEFER`) — nên trong lúc handler chạy, `sig` và các tín hiệu trong `sa_mask` bị chặn.

---

## 4. `eret` → handler chạy ở EL0

`do_signal` return → `do_notify_resume` loop re-check flags → `ret_to_user` → `kernel_exit 0` nạp từ `pt_regs` (giờ đã trỏ handler) → `eret`:

```
PC     ← regs->pc      = handler entry
SP     ← regs->sp      = user->sigframe  (frame nằm ngay dưới, có thể đọc uc_mcontext)
x0     = usig          (số tín hiệu)
x1     = &frame->info  (siginfo *,  nếu SA_SIGINFO)
x2     = &frame->uc    (ucontext *, nếu SA_SIGINFO)
x29    = &frame_record
x30    = __kernel_rt_sigreturn
PSTATE ← regs->pstate  (EL0t)
```

Handler chạy như một hàm C bình thường: `void handler(int signo, siginfo_t *info, void *ucontext)`. Nó **có thể sửa `ucontext->uc_mcontext`** (ví dụ đổi `pc` để "nhảy chỗ khác" khi quay lại — kỹ thuật dùng trong một số runtime/JIT/GC).

---

## 5. Handler `ret` → trampoline → `rt_sigreturn` → restore

### `__kernel_rt_sigreturn` (`arch/arm64/kernel/vdso/sigreturn.S`)

Khi handler thực thi `ret`, `PC ← x30 = __kernel_rt_sigreturn`:

```asm
__kernel_rt_sigreturn:
    mov  x8, #__NR_rt_sigreturn        // 139
    svc  #0
```

Chỉ hai lệnh. `svc` → EL1.

### `SYSCALL_DEFINE0(rt_sigreturn)` (`arch/arm64/kernel/signal.c`)

```c
SYSCALL_DEFINE0(rt_sigreturn)
{
    struct pt_regs *regs = current_pt_regs();
    struct rt_sigframe __user *frame;

    current->restart_block.fn = do_no_restart_syscall;

    if (regs->sp & 15)                              // frame phải căn 16 byte
        goto badframe;
    frame = (struct rt_sigframe __user *)regs->sp;  // SP vẫn trỏ frame (trampoline vào với SP = frame)
    if (!access_ok(frame, sizeof(*frame)))
        goto badframe;
    if (restore_sigframe(regs, frame))             // ◀── KHÔI PHỤC pt_regs
        goto badframe;
    if (restore_altstack(&frame->uc.uc_stack))
        goto badframe;
    return regs->regs[0];                          // trả về = x0 vừa khôi phục từ frame

badframe:
    arm64_notify_segfault(regs->sp);              // frame hỏng → SIGSEGV
    return 0;
}
```

### `restore_sigframe()` — register được restore thế nào

```c
static int restore_sigframe(struct pt_regs *regs, struct rt_sigframe __user *sf)
{
    sigset_t set;
    int i, err;
    struct user_ctxs user;

    __copy_from_user(&set, &sf->uc.uc_sigmask, sizeof(set));
    set_current_blocked(&set);                              // ◀ KHÔI PHỤC mặt nạ chặn (bỏ mặt nạ handler đã thêm)

    for (i = 0; i < 31; i++)
        __get_user_error(regs->regs[i], &sf->uc.uc_mcontext.regs[i], err);   // ◀ x0..x30
    __get_user_error(regs->sp,     &sf->uc.uc_mcontext.sp,     err);         // ◀ SP user
    __get_user_error(regs->pc,     &sf->uc.uc_mcontext.pc,     err);         // ◀ PC user (chỗ bị ngắt)
    __get_user_error(regs->pstate, &sf->uc.uc_mcontext.pstate, err);         // ◀ PSTATE

    forget_syscall(regs);                                   // regs->syscallno = -1 → rt_sigreturn KHÔNG bị restart

    err |= !valid_user_regs(&regs->user_regs, current);     // ◀ LỌC: pstate không được yêu cầu EL1, không leo thang

    parse_user_sigframe(&user, sf);                         // duyệt __reserved[] tìm khối magic
    if (system_supports_fpsimd()) {
        if (user.sve) restore_sve_fpsimd_context(&user);
        else          restore_fpsimd_context(user.fpsimd);  // ◀ V0..V31, FPSR, FPCR
    }
    return err;
}
```

**`valid_user_regs()`** là chốt bảo mật: `pstate` đọc từ frame do userspace ghi → phải lọc. Nó ép `M[3:0] = EL0t`, chỉ cho phép NZCV + vài bit an toàn, cấm bit `M[4]` (nRW) sai, cấm DAIF sai. Không có nó, một handler độc hại có thể craft `uc_mcontext.pstate = EL1h` và `rt_sigreturn` sẽ `eret` vào EL1 với PC do nó chọn → toàn quyền kernel.

### Quay về

`rt_sigreturn` return `regs->regs[0]` → `el0_svc` → `ret_to_user` → `kernel_exit 0` nạp từ `pt_regs` (giờ đã là ngữ cảnh **trước signal**) → `eret`:

```
PC     ← uc_mcontext.pc      = lệnh bị ngắt (hoặc `svc` nếu syscall cần restart)
SP     ← uc_mcontext.sp      = SP user trước signal
x0..x30 ← uc_mcontext.regs[] = thanh ghi trước signal
PSTATE ← uc_mcontext.pstate  (đã lọc)
```

→ Process tiếp tục **như thể signal chưa từng xảy ra** (trừ side-effect của handler và mọi thay đổi handler cố ý làm lên `ucontext`).

---

## 6. Timeline tổng hợp

```
Task W                          Task A
──────                          ──────
kill(A, SIGUSR1)
  __send_signal:
    sigqueue → A->signal->shared_pending
    sigaddset(pending, SIGUSR1)
    complete_signal → signal_wake_up(A):
      set TIF_SIGPENDING trên A
      wake_up_state(A) / kick_process(A)
                                A: đang trong syscall / vừa xử lý IRQ → tới ret_to_user
                                  _TIF_SIGPENDING → do_notify_resume → do_signal
                                  get_signal: dequeue SIGUSR1, action có handler
                                  (syscall restart check: -ERESTARTSYS + SA_RESTART? → pc -= 4)
                                  handle_signal → setup_rt_frame:
                                    get_sigframe: frame = round_down(user_sp - ..., 16)
                                    setup_sigframe: ghi x0..x30/sp/pc/pstate/FPSIMD/mask vào frame
                                    setup_return: regs->pc=handler, regs->sp=frame,
                                                  regs->regs[0]=SIGUSR1, regs->regs[30]=trampoline
                                    current->blocked |= sa_mask | SIGUSR1
                                  ret_to_user → kernel_exit → eret
                                A @ EL0: handler(SIGUSR1, ...) chạy trên frame
                                  handler ret → pc = x30 = __kernel_rt_sigreturn
                                  mov x8,#139 ; svc #0
                                el0_svc → sys_rt_sigreturn:
                                  restore_sigframe: set_current_blocked(uc_sigmask),
                                    regs->regs[0..30] ← uc_mcontext.regs[],
                                    regs->sp/pc/pstate ← uc_mcontext, restore_fpsimd_context
                                  valid_user_regs (lọc pstate)
                                  return regs->regs[0]
                                ret_to_user → kernel_exit → eret
                                A @ EL0: tiếp tục ĐÚNG lệnh bị ngắt, ĐÚNG thanh ghi
```

---

## 7. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|`setup_return`|đặt `regs[0]`, `sp`, `pc`, `regs[29]`, `regs[30]`|+ `pstate.BTYPE = PSR_BTYPE_C` (BTI, 5.8), `pstate.TCO = 0` (MTE, 5.10)|
|`restore_sigframe`|FPSIMD + (SVE nếu có)|+ xử lý TPIDR2/ZA (SME, 6.3)|
|`rt_sigframe_user_layout` (frame kích thước động)|có (từ 4.19)|có|
|trampoline|`VDSO_SYMBOL(vdso, sigtramp)` = `__kernel_rt_sigreturn`|y hệt|
|`do_signal` syscall-restart|như mô tả|y hệt|
|`get_signal` / `dequeue_signal` (generic)|như mô tả|+ `_TIF_NOTIFY_SIGNAL` (task_work-driven, 5.11)|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `setup_return` có BTI/TCO, `struct sigcontext` giống. Cơ chế "frame trên user stack, `setup_return` trỏ `pt_regs` vào handler + `x30 = trampoline`, `rt_sigreturn` → `restore_sigframe` ghi đè `pt_regs`, `valid_user_regs` lọc pstate" — **giống hệt 5.4**.