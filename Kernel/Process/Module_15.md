# MODULE 15 — Interrupt và Process trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/interrupt-and-process-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/interrupt-and-process-arm64.md) (4 sơ đồ: đường đi đầy đủ A@EL0→IRQ→B@EL0, hai kernel stack A đóng băng/B hoạt động, "return về đâu", IRQ từ EL0 vs EL1).

**File nguồn**: `arch/arm64/kernel/entry.S` (`el0_irq`, `el1_irq`, `irq_handler` macro, `irq_stack_entry/exit`, `ret_to_user`, `work_pending`), `arch/arm64/kernel/signal.c` (`do_notify_resume`), `arch/arm64/kernel/irq.c` (`irq_stack_ptr`), `drivers/irqchip/irq-gic-v3.c` (`gic_handle_irq`), `kernel/irq/irqdesc.c` (`handle_domain_irq`, `generic_handle_irq`), `kernel/softirq.c` (`irq_enter`, `irq_exit`), `kernel/sched/core.c` (`scheduler_tick`, `resched_curr`, `schedule`, `__schedule`).

---

## 0. Câu trả lời ngắn

**Có** — CPU có thể `eret` về user space của process **B**, không phải process **A** đã bị ngắt.

- IRQ ngắt A tại `PC_A`. Handler chạy, timer tick **bật cờ `TIF_NEED_RESCHED`** trên A (không đổi task ngay).
- Sau handler, tại `ret_to_user`, kernel thấy cờ → `do_notify_resume` → **`schedule()`** → chọn B → `context_switch` → `cpu_switch_to(A, B)`.
- Giờ `current == B`. Kernel unwind call chain của B, tới `ret_to_user` của B, `kernel_exit` đọc **`pt_regs_B`** (nằm ở đỉnh kernel stack của B, lưu từ lần B vào kernel _trước đó_), `eret` → CPU về EL0 tại **`PC_B`** — chỗ B rời user space lần trước.
- `pt_regs_A` (chứa `PC_A`, lệnh bị ngắt) nằm **nguyên vẹn** ở đỉnh kernel stack của A. Khi scheduler chọn A lần sau → A `eret` về đúng `PC_A`. **Lệnh bị ngắt không mất.**

---

## 1. Phần cứng: IRQ khi A đang ở EL0

A chạy user space với IRQ **không bị mask** (`PSTATE.I = 0` — EL0 luôn chạy với IRQ mở). Một thiết bị assert IRQ line → GIC → CPU nhận exception:

|Phần cứng tự làm (1 chu kỳ)|
|---|
|`SPSR_EL1 ← PSTATE` của EL0 (`M = EL0t`, DAIF, NZCV)|
|`ELR_EL1 ← PC_A` — địa chỉ lệnh A **đang/sắp** thực thi|
|`PSTATE`: `M ← EL1h`, `DAIF ← 1111` (mask hết), `SP ← SP_EL1`|
|`PC ← VBAR_EL1 + 0x480` — vector "IRQ từ Lower EL, AArch64"|
|`SP_EL0` **không đổi** (vẫn = SP user của A); `x0`–`x30` **không đổi**|

(So với syscall dùng offset `0x400` — IRQ dùng `0x480`.)

---

## 2. `el0_irq` → `kernel_entry 0` — lưu `pt_regs_A`

`arch/arm64/kernel/entry.S`:

```asm
SYM_CODE_START_LOCAL_NOALIGN(el0_irq)
    kernel_entry 0
el0_irq_naked:
    gic_prio_irq_setup pmr=x20, tmp=x1
    enable_da_f                        // bật D/A/F, GIỮ I mask — handler chạy với IRQ off
    ct_user_exit                       // context tracking: rời userspace
    irq_handler
    b    ret_to_user
SYM_CODE_END(el0_irq)
```

`kernel_entry 0` (Module 3): `stp x0..x30` vào `pt_regs`; `mrs x21, sp_el0` → `pt_regs.sp` (SP user của A); `mrs x22, elr_el1` → `pt_regs.pc` (= `PC_A`); `mrs x23, spsr_el1` → `pt_regs.pstate`; `msr sp_el0, tsk` (`current = A`).

→ **`pt_regs_A`** nằm ở **đỉnh kernel stack của A**, chứa toàn bộ ngữ cảnh EL0 của A tại thời điểm bị ngắt.

**`enable_da_f`**: bật lại Debug/SError/FIQ nhưng **giữ IRQ mask** — trên ARM64, hardirq handler chạy với IRQ **tắt** (không có nested IRQ cùng priority; pseudo-NMI qua priority masking là ngoại lệ).

---

## 3. `irq_handler` — chạy trên IRQ stack per-CPU

```asm
.macro irq_handler, handler:req
    ldr_l  x1, \handler         // = handle_arch_irq (GIC driver set qua set_handle_irq())
    mov    x0, sp               // x0 = con trỏ pt_regs_A
    irq_stack_entry             // lưu sp hiện tại vào callee-saved; sp ← this_cpu(irq_stack_ptr) + IRQ_STACK_SIZE
    blr    x1                   // gọi gic_handle_irq(pt_regs)
    irq_stack_exit             // sp ← về lại kernel stack của A
.endm
```

**Vì sao IRQ stack riêng**: handler thiết bị có thể sâu; nếu chạy trên kernel stack của A (16 KB, đã có `pt_regs_A` + các frame) thì dễ tràn, nhất là khi IRQ lồng exception. IRQ stack per-CPU (`IRQ_STACK_SIZE = 16 KB`) tách riêng. Chỉ `pt_regs_A` được khắc trên kernel stack của A; phần còn lại của việc xử lý IRQ ở trên IRQ stack.

### Bên trong `gic_handle_irq` (GICv3)

```
gic_handle_irq(pt_regs):
  irqnr = read_sysreg(ICC_IAR1_EL1)          // acknowledge — đọc INTID
  handle_domain_irq(gic_domain, irqnr, regs):
      irq_enter()                            // preempt_count += HARDIRQ_OFFSET → in_irq() = true
                                             //   ⇒ KHÔNG schedule() được từ giờ tới irq_exit()
      generic_handle_irq(irq):
          desc->handle_irq(desc)             // flow handler (handle_fasteoi_irq / handle_edge_irq)
              handle_irq_event(desc):
                  action->handler(irq, dev_id)   // ◀── ISR của device driver
      irq_exit()                             // preempt_count -= HARDIRQ_OFFSET
                                             //   nếu có softirq pending && !in_interrupt():
                                             //     __do_softirq()  (net rx, timers, RCU, tasklet, sched)
  write_sysreg(irqnr, ICC_EOIR1_EL1)         // end of interrupt
```

### Nếu IRQ này là timer tick

```
timer ISR → tick_handler → tick_sched_timer / tick_periodic
  → update_process_times(user_mode(regs)):
      → scheduler_tick():
          curr->sched_class->task_tick(rq, curr, 0)   // = task_tick_fair
              entity_tick(cfs_rq, &A->se, 0):
                  update_curr(cfs_rq)                  // A.se.vruntime += calc_delta_fair(...)
                  check_preempt_tick(cfs_rq, curr):
                      nếu A chạy quá sched_slice(A)  → resched_curr(rq)
```

`resched_curr(rq)`:

```c
void resched_curr(struct rq *rq)
{
    struct task_struct *curr = rq->curr;   // = A
    if (test_tsk_need_resched(curr)) return;
    if (cpu == smp_processor_id()) {
        set_tsk_need_resched(curr);        // ◀── BẬT TIF_NEED_RESCHED trong A->thread_info.flags
        set_preempt_need_resched();
    } else {
        // CPU khác → smp_send_reschedule(cpu) → GIC SGI
    }
}
```

**Chỉ bật một bit cờ.** Không `schedule()`, không đổi task. Handler tiếp tục, `irq_exit()`, `EOIR`, `irq_stack_exit`, `b ret_to_user`.

---

## 4. `ret_to_user` — điểm quyết định

```asm
SYM_CODE_START_LOCAL(ret_to_user)
    disable_daif
    ldr   x19, [tsk, #TSK_TI_FLAGS]          // A->thread_info.flags
    and   x2,  x19, #_TIF_WORK_MASK          // NEED_RESCHED | SIGPENDING | NOTIFY_RESUME | FOREIGN_FPSTATE | ...
    cbnz  x2, work_pending
finish_ret_to_user:
    ...
    kernel_exit 0                            // eret về EL0 của A tại PC_A  (nếu KHÔNG có cờ)

work_pending:
    mov   x0, sp                             // regs (= pt_regs_A)
    mov   x1, x19                            // thread_flags
    bl    do_notify_resume
    ldr   x19, [tsk, #TSK_TI_FLAGS]
    b     finish_ret_to_user
SYM_CODE_END(ret_to_user)
```

- **Không cờ nào bật** → `kernel_exit 0` ngay → `eret` → A về EL0 tại `PC_A`. **Không đổi task.** (IRQ chỉ "ghé qua".)
- **`TIF_NEED_RESCHED` bật** (do timer) → `work_pending` → `do_notify_resume`.

---

## 5. `do_notify_resume` → `schedule()`

`arch/arm64/kernel/signal.c`:

```c
asmlinkage void do_notify_resume(struct pt_regs *regs, unsigned long thread_flags)
{
    do {
        if (thread_flags & _TIF_NEED_RESCHED) {
            schedule();                              // ◀── CONTEXT SWITCH XẢY RA Ở ĐÂY
        } else {
            local_daif_restore(DAIF_PROCCTX);
            if (thread_flags & _TIF_SIGPENDING)
                do_signal(regs);
            if (thread_flags & _TIF_NOTIFY_RESUME)
                tracehook_notify_resume(regs);       // task_work, v.v.
            if (thread_flags & _TIF_FOREIGN_FPSTATE)
                fpsimd_restore_current_state();
        }
        local_daif_mask();
        thread_flags = READ_ONCE(current_thread_info()->flags);
    } while (thread_flags & _TIF_WORK_MASK);
}
```

`schedule()` → `__schedule(false)`:

- `prev = A`, `prev->state == TASK_RUNNING` (A không ngủ, chỉ bị preempt) → **không** `deactivate_task` → A vẫn ở trong runqueue (rbtree).
- `next = pick_next_task(rq, A)` → `__pick_first_entity` = **B** (vruntime nhỏ nhất).
- `rq->curr = B`; `context_switch(rq, A, B, &rf)`.

---

## 6. `context_switch(A, B)` — hai kernel stack

```c
switch_mm_irqs_off(A->active_mm, B->mm, B);   // MM_A → MM_B: TTBR0 = PGD_B, cấp ASID_B (Module 14)
switch_to(A, B, A);                            // → __switch_to(A, B) → cpu_switch_to(A, B)
```

`cpu_switch_to(A, B)`:

- **Lưu `cpu_context_A`**: `x19..x28`, `fp`, `sp` (đang trỏ vào call chain `do_notify_resume → schedule → __schedule → __switch_to` trên kernel stack A), `pc` (địa chỉ trong `__switch_to`).
- **Kernel stack của A giờ ĐÓNG BĂNG** với layout (từ đỉnh xuống):
    
    ```
    pt_regs_A          ← ngữ cảnh EL0 của A: PC_A, SP_A user, x0..x30, pstate
    kernel_entry frame (từ el0_irq)
    ret_to_user frame
    do_notify_resume frame
    schedule → __schedule frame
    __switch_to frame  ← sp lưu trong cpu_context_A
    ```
    
- **Nạp `cpu_context_B`** → `sp` = kernel stack B, `pc` = điểm B bị switch away lần trước.
- `msr sp_el0, B` → `current = B`.
- `ret` → chạy tiếp trên kernel stack B.

---

## 7. B chạy tiếp → `eret` về EL0 của B

Call chain của B unwind (B resume ở đâu tuỳ lần B bị switch away trước đó — có thể cũng là `do_notify_resume`, có thể là `pipe_read`, v.v.):

```
__switch_to return → context_switch: finish_task_switch(A)   // nhả rq->lock, A->on_cpu = 0
  → __schedule return → schedule return → về do_notify_resume loop CỦA B (nếu B lần trước cũng bị preempt)
  → do_notify_resume return → work_pending return → b finish_ret_to_user
  → ret_to_user CỦA B: re-check _TIF_WORK_MASK (cờ CỦA B, thường đã clear)
  → kernel_exit 0
```

`kernel_exit 0` cho B:

```asm
ldp   x21, x22, [sp, #S_PC]     // x21 = pt_regs_B.pc = PC_B ; x22 = pt_regs_B.pstate
ldr   x23, [sp, #S_SP]         // x23 = pt_regs_B.sp = SP user của B
msr   sp_el0, x23
msr   elr_el1, x21
msr   spsr_el1, x22
ldp   x0..x30 từ pt_regs_B     // thanh ghi user của B
add   sp, sp, #S_FRAME_SIZE
eret
```

`pt_regs_B` nằm ở **đỉnh kernel stack của B** — được lưu **lần B vào kernel trước đó** (có thể là một syscall, một IRQ khác, một page fault — cách đây nhiều lần lập lịch).

`eret`:

- `PSTATE ← SPSR_EL1` = `pt_regs_B.pstate` (EL0t của B).
- `PC ← ELR_EL1` = `pt_regs_B.pc` = **`PC_B`** — chỗ B rời EL0 lần trước.
- `SP` (EL0) ← `SP_EL0` = SP user của B.
- MMU: `TTBR0_EL1` = `PGD_B`, ASID = `ASID_B` (đã đổi ở bước 6).

→ **CPU ở EL0 của B, tại `PC_B`.** Không phải `PC_A`.

---

## 8. A quay lại khi nào — và về đâu

`pt_regs_A` vẫn nằm nguyên ở đỉnh kernel stack của A, chứa `PC_A` (lệnh bị IRQ ngắt). Khi scheduler chọn A lần sau (`cpu_switch_to(X, A)`):

- `sp` ← `cpu_context_A.sp` → kernel stack A.
- Call chain A unwind: `__switch_to` → `context_switch` → `finish_task_switch` → `__schedule` → `schedule` → `do_notify_resume` loop **của A** → re-check `_TIF_WORK_MASK` của A (giờ `TIF_NEED_RESCHED` đã bị `__schedule` clear) → thoát loop → `ret_to_user` của A → `kernel_exit 0` đọc **`pt_regs_A`** → `eret`.
- `PC ← pt_regs_A.pc = PC_A` → **A chạy tiếp đúng lệnh mà IRQ ban đầu đã ngắt.**

IRQ được "tính" vào dòng thời gian của A (A đang chạy khi IRQ tới, `update_process_times(user_mode=1)` cộng tick cho `A->utime`). Nhưng **lần trở về EL0 ngay sau IRQ có thể là của B**. Lệnh bị ngắt của A không mất — nó ở `pt_regs_A`.

---

## 9. IRQ từ EL0 vs IRQ từ EL1

||IRQ khi A ở **EL0** (prompt)|IRQ khi A ở **EL1** (giữa syscall)|
|---|---|---|
|Vector|`0x480` `el0_irq`|`0x280` `el1_irq`|
|Sau handler|`b ret_to_user` → kiểm `_TIF_WORK_MASK` → `do_notify_resume` → `schedule()`|kiểm preempt trong `el1_irq`|
|Điểm reschedule|**luôn** (mọi `CONFIG_PREEMPT`)|chỉ nếu `CONFIG_PREEMPT` **và** `preempt_count == 0` **và** `TIF_NEED_RESCHED` → `arm64_preempt_schedule_irq()` → `preempt_schedule_irq()` → `__schedule(true)`|
|Nếu `PREEMPT_NONE` / `VOLUNTARY`|vẫn preempt (đường EL0 độc lập config)|**không** preempt; A tiếp tục syscall, reschedule hoãn tới khi syscall của A về `ret_to_user`|
|Nếu `preempt_count != 0` (A giữ spinlock/RCU)|N/A (ở EL0 `preempt_count` = 0)|**không** preempt; hoãn tới `preempt_enable()` đưa count về 0|

Trong `PREEMPT_NONE` (mặc định nhiều bản 5.4), một task đang trong syscall dài **không** bị preempt bởi IRQ — nó chỉ nhường CPU khi (a) syscall xong và về `ret_to_user`, hoặc (b) syscall tự gọi `schedule()`/`cond_resched()`, hoặc (c) syscall block. Task ở EL0 thì luôn bị preempt ở lần IRQ kế tiếp.

---

## 10. Vì sao handler không tự `schedule()`

`irq_enter()` bơm `preempt_count += HARDIRQ_OFFSET` → `in_irq()` true, `preempt_count() != 0`. Nếu handler gọi `schedule()` → `schedule_debug()` phát hiện `preempt_count != 0` → **BUG** ("scheduling while atomic"). Lý do:

- Handler có thể chạy trên IRQ stack per-CPU — không phải stack của task nào, không đổi task được.
- Handler ngắt task A tại điểm bất kỳ (kể cả khi A đang giữ spinlock) — đổi task ở đây có thể deadlock.
- `pt_regs` ở đỉnh kernel stack A là ngữ cảnh cần để A quay về; nó chỉ được "tiêu thụ" bởi `kernel_exit` ở `ret_to_user`/`kernel_exit 1`, sau khi handler xong.

Nên IRQ handler chỉ **bật cờ** (`TIF_NEED_RESCHED`, hoặc `wake_up` một task → task đó vào runqueue + bật cờ trên `rq->curr`). Việc `schedule()` xảy ra ở điểm an toàn tiếp theo: `ret_to_user` (EL0) hoặc preempt point (EL1 + `CONFIG_PREEMPT`).

---

## 11. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|`el0_irq`|asm: `kernel_entry 0; irq_handler; b ret_to_user`|C: `el0_irq()` trong `entry-common.c` (5.12), gọi `enter_from_user_mode` / `exit_to_user_mode`|
|`handle_domain_irq`|có|đổi tên `generic_handle_domain_irq` (5.18)|
|`do_notify_resume` loop|`_TIF_NEED_RESCHED` → `schedule()`|y hệt (chuyển sang `exit_to_user_mode_loop` ở 5.12)|
|IRQ stack|`irq_stack_entry`/`irq_stack_exit` macro|`call_on_irq_stack` (5.12)|
|`irq_enter`/`irq_exit`|như mô tả|tách `irq_enter_rcu`/`irq_exit_rcu` (5.11)|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `el0_irq` gần 5.4 (chưa chuyển hẳn sang C), có `el0_interrupt_handler` macro. Cơ chế "IRQ handler chỉ bật cờ, `schedule()` ở `ret_to_user`, `eret` dùng `pt_regs` của task được chọn" — **giống hệt 5.4**.