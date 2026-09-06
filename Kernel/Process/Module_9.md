# MODULE 9 — `ret_from_fork` trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/ret-from-fork-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/ret-from-fork-arm64.md) (4 sơ đồ: bất đối xứng "một task bắt đầu, task khác kết thúc", hai lối hạ cánh sau `cpu_switch_to`, cách `prev` chuyền vào `schedule_tail`, cách con về EL0).

**File nguồn**: `kernel/sched/core.c` (`__schedule`, `context_switch`, `finish_task_switch`, `schedule_tail`), `arch/arm64/kernel/entry.S` (`cpu_switch_to`, `ret_from_fork`, `ret_to_user`, `kernel_exit`), `arch/arm64/kernel/process.c` (`__switch_to`, `copy_thread`).

---

## 0. Câu trả lời ngắn cho hai câu hỏi

**Vì sao con bắt đầu ở `ret_from_fork` chứ không phải lệnh sau `fork()`?** Vì `context_switch()` không phải lời gọi hàm bình thường — nó "trả về trong ngữ cảnh task khác". Task đã chạy trước đó có sẵn một địa chỉ trở về (nằm giữa `__switch_to()` của chính nó, lưu lần bị switch away). Con **chưa bao giờ chạy** → không có địa chỉ đó → `copy_thread()` phải **cấy tay** một điểm hạ cánh: `cpu_context.pc = ret_from_fork`.

**Kernel làm sao đưa con về đúng lệnh user?** `copy_thread()` đã copy `pt_regs` của cha vào đỉnh kernel stack con, trong đó `childregs->pc = ELR_EL1 của cha = &(lệnh sau svc)`. `ret_from_fork` → `ret_to_user` → `kernel_exit` nạp `elr_el1 ← childregs->pc`, `spsr_el1 ← childregs->pstate`, rồi `eret`. Phần cứng nhảy về đúng đó. Con thấy `x0 = 0` (do `childregs->regs[0] = 0`).

---

## 1. Chuỗi đầy đủ — từ scheduler tới `eret`

### Bước 1 — Scheduler chọn con

Con được đặt vào runqueue bởi `wake_up_new_task()` ngay sau `copy_process()`:

```c
p->state = TASK_RUNNING;
__set_task_cpu(p, select_task_rq(p, task_cpu(p), SD_BALANCE_FORK, 0));  // CÓ THỂ là CPU khác
activate_task(rq, p, ENQUEUE_NOCLOCK);   // enqueue_task_fair → vào cây đỏ-đen CFS
check_preempt_curr(rq, p, WF_FORK);
```

Con nằm `TASK_RUNNING` trong runqueue của **CPU nào đó** (không nhất thiết CPU của cha). Nó chưa chạy một lệnh nào. Một `__schedule()` nào đó, trên CPU đó, sẽ chọn nó.

### Bước 2 — `__schedule()` → `context_switch()`

```c
// kernel/sched/core.c, task đang chạy (gọi là A) thực thi:
next = pick_next_task(rq, prev, &rf);   // = con
if (likely(prev != next)) {
    rq->curr = next;
    ...
    rq = context_switch(rq, prev, next, &rf);   // ◀── prev = A, next = con
}
```

### Bước 3 — `context_switch()` (`kernel/sched/core.c`)

```c
context_switch(struct rq *rq, struct task_struct *prev, struct task_struct *next, struct rq_flags *rf)
{
    prepare_task_switch(rq, prev, next);
    arch_start_context_switch(prev);

    if (!next->mm) {                        // to kernel thread
        enter_lazy_tlb(prev->active_mm, next);
        next->active_mm = prev->active_mm;
        if (prev->mm) mmgrab(prev->active_mm);
        else          prev->active_mm = NULL;
    } else {                               // to user task (con của fork là user task)
        membarrier_switch_mm(rq, prev->active_mm, next->mm);
        switch_mm_irqs_off(prev->active_mm, next->mm, next);   // ◀── TTBR0 = con->pgd, cấp ASID mới
        if (!prev->mm) {                    // from kernel thread
            rq->prev_mm = prev->active_mm;
            prev->active_mm = NULL;
        }
    }

    prepare_lock_switch(rq, next, rf);

    switch_to(prev, next, prev);            // ◀── ĐỔI THANH GHI + STACK. "Trả về" trong ngữ cảnh next.
    barrier();

    return finish_task_switch(prev);        // ◀── chỉ chạy khi TASK CŨ được switch-to trở lại
}
```

Lưu ý: với con của `fork()`, `next->mm != NULL` (nó có `mm` riêng từ `dup_mm`) → đi nhánh `switch_mm_irqs_off` → `check_and_switch_context()` cấp ASID mới cho `con->mm`, `cpu_do_switch_mm` ghi `TTBR0_EL1` + `TTBR1_EL1(ASID)`.

### Bước 4 — `switch_to()` macro → `__switch_to()`

```c
// arch/arm64/include/asm/switch_to.h
#define switch_to(prev, next, last)                    \
    do {                                              \
        ((last) = __switch_to((prev), (next)));        \
    } while (0)
```

```c
// arch/arm64/kernel/process.c
__notrace_funcgraph struct task_struct *__switch_to(struct task_struct *prev, struct task_struct *next)
{
    struct task_struct *last;
    fpsimd_thread_switch(next);         // lưu FP/SIMD prev nếu dirty; set TIF_FOREIGN_FPSTATE cho next
    tls_thread_switch(next);            // lưu TPIDR_EL0 của prev; nạp next->thread.uw.tp_value
    hw_breakpoint_thread_switch(next);
    contextidr_thread_switch(next);
    entry_task_switch(next);            // __this_cpu_write(__entry_task, next) — dùng khi vào từ EL0
    uao_thread_switch(next);
    ssbs_thread_switch(next);
    dsb(ish);                           // rào: hoàn tất TLB/cache maintenance trước khi có thể migrate CPU

    last = cpu_switch_to(prev, next);   // ◀── asm
    return last;
}
```

### Bước 5 — `cpu_switch_to(prev, next)` (`arch/arm64/kernel/entry.S`)

`x0 = prev`, `x1 = next`:

```asm
mov  x10, #THREAD_CPU_CONTEXT     // offset thread.cpu_context trong task_struct
add  x8, x0, x10                  // &prev->thread.cpu_context
mov  x9, sp
stp  x19, x20, [x8], #16          // ── LƯU ngữ cảnh KERNEL của prev ──
stp  x21, x22, [x8], #16
stp  x23, x24, [x8], #16
stp  x25, x26, [x8], #16
stp  x27, x28, [x8], #16
stp  x29, x9,  [x8], #16          // fp, sp
str  lr,       [x8]               // pc = "lệnh ngay sau `bl cpu_switch_to` trong __switch_to"

add  x8, x1, x10                  // &next->thread.cpu_context
ldp  x19, x20, [x8], #16          // ── NẠP ngữ cảnh KERNEL của next ──
ldp  x21, x22, [x8], #16
ldp  x23, x24, [x8], #16
ldp  x25, x26, [x8], #16
ldp  x27, x28, [x8], #16
ldp  x29, x9,  [x8], #16          // fp, sp
ldr  lr,       [x8]               // lr = next->cpu_context.pc

mov  sp, x9                       // ◀── ĐỔI KERNEL STACK
msr  sp_el0, x1                   // ◀── current = next
ret                               // ◀── nhảy tới lr = next->cpu_context.pc
```

**`x0` không bị đụng** trong toàn bộ `cpu_switch_to` → sau `ret`, `x0` vẫn = `prev`. Đây là cách `prev` "đi lậu" từ `__switch_to` sang bên kia của switch.

### Bước 6 — `ret` "hạ cánh" ở đâu?

|`next` là…|`next->cpu_context.pc` =|`ret` nhảy tới|
|---|---|---|
|Task đã chạy trước|"lệnh sau `bl cpu_switch_to`" (lưu ở lần nó bị switch away)|Trở về `__switch_to()` → `return last` → `context_switch()` → `return finish_task_switch(prev)` **tự động** → `__schedule` return → `schedule` return → task chạy tiếp chỗ nó ngủ|
|**Con mới fork**|**`ret_from_fork`** (do `copy_thread` cấy)|`ret_from_fork` (stub asm)|

Stack con lúc này: `sp = childregs` (do `cpu_context.sp = childregs`), chỉ có `pt_regs` ở đỉnh, **không có** frame `context_switch`/`__switch_to`/`__schedule` nào.

### Bước 7 — `ret_from_fork` (`arch/arm64/kernel/entry.S`)

```asm
SYM_CODE_START(ret_from_fork)
    bl   schedule_tail          // (a) x0 = prev (đã sẵn từ cpu_switch_to)
    cbz  x19, 1f                // (b) x19 == 0 (memset cpu_context) → user task
    mov  x0, x20               //     kernel thread: x0 = arg
    blr  x19                   //     kernel thread: gọi fn(arg)
1:  get_current_task tsk
    b    ret_to_user            // (c) user task: đi ra EL0
SYM_CODE_END(ret_from_fork)
```

### Bước 8 — `schedule_tail(prev)` (`kernel/sched/core.c`)

```c
asmlinkage __visible void schedule_tail(struct task_struct *prev)
{
    finish_task_switch(prev);       // ◀── việc mà task cũ được làm giùm, con phải làm tay
    preempt_enable();               // preempt_count: FORK_PREEMPT_COUNT (2) → 0
    if (current->set_child_tid)
        put_user(task_pid_vnr(current), current->set_child_tid);   // CLONE_CHILD_SETTID
    calculate_sigpending();
}
```

`finish_task_switch(prev)` làm:

- kiểm `preempt_count() == 2*PREEMPT_DISABLE_OFFSET` (task cũ để lại count = 2: `schedule()` `preempt_disable` → 1, `__schedule` `raw_spin_lock_irq(&rq->lock)` → 2; con được `copy_process` khởi tạo `FORK_PREEMPT_COUNT` = 2 để khớp).
- `finish_task(prev)` → `prev->on_cpu = 0` (giải phóng `prev` cho CPU khác wakeup).
- `finish_lock_switch(rq)` → `raw_spin_unlock_irq(&rq->lock)` — **nhả khoá runqueue** mà task cũ đã giữ xuyên suốt switch.
- nếu `prev->state == TASK_DEAD`: `put_task_stack(prev)` + `put_task_struct_rcu_user(prev)` — **con dọn xác task chết trước nó** trên CPU này.
- `mmdrop(rq->prev_mm)` nếu task cũ là kernel thread nhả mm mượn.

### Bước 9 — `ret_to_user` → `kernel_exit 0`

`b ret_to_user`:

```asm
ret_to_user:
    disable_daif
    ldr   x1, [tsk, #TSK_TI_FLAGS]
    and   x2, x1, #_TIF_WORK_MASK     // NEED_RESCHED | SIGPENDING | NOTIFY_RESUME | ...
    cbnz  x2, work_pending            // thường rỗng ngay sau fork
finish_ret_to_user:
    kernel_exit 0
```

`kernel_exit 0`:

```asm
ldp   x21, x22, [sp, #S_PC]        // x21 = childregs->pc (=&svc+4) ; x22 = childregs->pstate (=EL0t)
ldr   x23, [sp, #S_SP]            // x23 = childregs->sp = SP user
msr   sp_el0, x23                 // khôi phục SP user
msr   elr_el1, x21               // nơi eret nhảy về
msr   spsr_el1, x22             // PSTATE eret phục hồi
ldp   x0, x1,  [sp, #16*0]      // ldp toàn bộ x0..x29 từ childregs → x0 = 0
...
ldp   x28, x29,[sp, #16*14]
ldr   lr, [sp, #S_LR]
add   sp, sp, #S_FRAME_SIZE     // SP_EL1 về đỉnh kernel stack con
eret
```

### Bước 10 — `eret` (phần cứng)

- `PSTATE ← SPSR_EL1` → về **EL0t**, DAIF theo user (IRQ mở lại), NZCV như cha.
- `PC ← ELR_EL1` = **lệnh ngay sau `svc`** trong libc.
- `SP` đang dùng ← `SP_EL0` = SP user.

Con ở EL0, tại lệnh sau `svc`, `x0 = 0`. glibc fork wrapper thấy `x0 = 0`, trả 0 cho lời gọi C.

---

## 2. Vì sao con không thể "nhảy thẳng vào lệnh sau `fork()`"

### Lý do 1 — Không có điểm hạ cánh tự nhiên

`cpu_switch_to` kết thúc bằng `ret`, nhảy tới `next->cpu_context.pc`. Với task đã chạy, giá trị đó là địa chỉ trong `__switch_to()` — chỗ nó bị đóng băng lần trước. Con **chưa bao giờ vào `__switch_to()`**, nên không tồn tại giá trị nào để `ret` tới. `copy_thread()` giải quyết bằng cách gán `cpu_context.pc = ret_from_fork` — một stub được viết sẵn để đóng vai "điểm ra khỏi switch".

### Lý do 2 — `context_switch` được task khác khởi động

Switch bắt đầu trong `__schedule()` của **task A**, với `rq->lock` giữ và `preempt_count = 2`. Đường code bình thường: task được switch-to trở về `__switch_to` → `context_switch` → `return finish_task_switch(prev)` **tự động** nhả khoá và cân bằng preempt. Con **không có frame `context_switch` trên stack** (stack con mới toanh, chỉ có `pt_regs`). Nên `ret_from_fork` phải gọi `schedule_tail()` (bọc `finish_task_switch`) một cách tường minh — nếu không, `rq->lock` bị giữ mãi và preempt tắt vĩnh viễn → hệ thống treo.

### Lý do 3 — Cần chỗ rẽ nhánh user vs kernel thread

Con có thể là user task (khôi phục `pt_regs`, `eret` về EL0) hoặc kernel thread (gọi `fn(arg)`, không có EL0 để về). `ret_from_fork` dùng `cbz x19` để rẽ: `x19 == 0` (từ `memset(&cpu_context, 0, ...)`) → user; `x19 == fn` (do `copy_thread` gán cho kernel thread) → gọi `fn`.

### Đối chiếu: task bình thường bị preempt rồi chạy lại

```
Lần 1 (bị switch away):
  ... vfs_read → schedule → __schedule → context_switch
                            → switch_to → __switch_to → bl cpu_switch_to
  cpu_switch_to LƯU: cpu_context.pc = &(lệnh sau `bl cpu_switch_to`)   ← ĐIỂM HẠ CÁNH
                     cpu_context.sp = sp hiện tại (giữa __switch_to)

Lần 2 (được chọn lại):
  cpu_switch_to NẠP: sp ← cpu_context.sp, lr ← cpu_context.pc
  ret → &(lệnh sau `bl cpu_switch_to`)   ← trở về GIỮA __switch_to
      → return last → context_switch: return finish_task_switch(prev)  ← TỰ ĐỘNG
      → __schedule return → schedule return → vfs_read tiếp tục
```

Con không có "Lần 1" nên không có điểm hạ cánh — `ret_from_fork` thay thế.

---

## 3. Cách con quay lại đúng lệnh user

Không có "địa chỉ lệnh đúng" được lưu ở chỗ đặc biệt nào. Nó chỉ là một trường trong `pt_regs`:

|Nguồn|Giá trị|Được dùng bởi|
|---|---|---|
|`copy_thread`: `*childregs = *current_pt_regs()`|`childregs->pc` = `ELR_EL1` của cha lúc `svc` = `&(lệnh sau svc)`|`kernel_exit`: `msr elr_el1, childregs->pc`|
|như trên|`childregs->pstate` = `SPSR_EL1` của cha = `EL0t`, DAIF user|`kernel_exit`: `msr spsr_el1, childregs->pstate`|
|như trên|`childregs->sp` = `SP_EL0` của cha (hoặc stack `pthread`)|`kernel_exit`: `msr sp_el0, childregs->sp`|
|`copy_thread`: `childregs->regs[0] = 0`|`x0 = 0`|`kernel_exit`: `ldp x0, x1, [sp]`|
|`eret` phần cứng|`PC ← ELR_EL1`, `PSTATE ← SPSR_EL1`, `SP ← SP_EL0`|—|

**Cha** đi đường khác nhưng về **cùng địa chỉ**: `_do_fork` trả `nr` (PID con) → `invoke_syscall` ghi `regs->regs[0] = nr` vào `pt_regs` của cha → cùng `ret_to_user` → `kernel_exit` → `eret` → cha ở EL0 tại `&(lệnh sau svc)` với `x0 = PID con`.

→ Cả hai `eret` về cùng một lệnh; chỉ khác `x0` (0 vs PID) và stack/mm.

---

## 4. Timeline: cha vs con (có thể khác CPU, có thể khác thứ tự)

```
CPU 0                                CPU 1
─────                                ─────
cha: svc(clone)
  _do_fork → copy_process
    dup_task_struct (con: task + stack mới)
    copy_thread    (con: pt_regs copy, regs[0]=0;
                    cpu_context.pc=ret_from_fork, sp=childregs)
  wake_up_new_task(con)
    select_task_rq → chọn CPU 1  ───────────►  con: TASK_RUNNING trong rq của CPU 1
  _do_fork return nr
cha: regs[0]=nr → ret_to_user → eret
cha ở EL0, x0 = PID con                        (CPU 1 đang chạy task khác)
  ...chạy tiếp...                               ...
                                               __schedule() chọn con
                                               context_switch → switch_mm (ASID mới)
                                               cpu_switch_to → ret → ret_from_fork
                                               schedule_tail → finish_task_switch(prev)
                                               cbz x19 → b ret_to_user → kernel_exit → eret
                                               con ở EL0, x0 = 0
```

`sysctl kernel.sched_child_runs_first` mặc định 0 → cha thường chạy trước. Nhưng nếu `check_preempt_curr` thấy con "đáng chạy hơn" (vruntime nhỏ hơn đủ nhiều) hoặc con được đặt lên CPU rảnh, con có thể `eret` **trước** cha.

---

## 5. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|`ret_from_fork`|`bl schedule_tail; cbz x19,1f; mov x0,x20; blr x19; 1: b ret_to_user`|y hệt|
|`cpu_switch_to`|lưu/nạp `cpu_context` + `msr sp_el0`|+ `scs_save`/`scs_load_current` (Shadow Call Stack, 5.8), `ptrauth_keys_install_kernel`|
|`context_switch`|như mô tả|thêm `membarrier_switch_mm`, `prepare_lock_switch` refactor|
|`finish_task_switch`|`put_task_struct` khi `TASK_DEAD`|tách `put_task_struct_rcu_user` (5.9)|
|`schedule_tail`|`finish_task_switch` + `preempt_enable` + `set_child_tid`|y hệt|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `cpu_switch_to` có thêm `scs_save x0, x8` / `scs_load_current` / `ptrauth_keys_install_kernel`. Bất đối xứng "task A bắt đầu switch, con hoàn tất qua `schedule_tail`" và điểm hạ cánh `cpu_context.pc = ret_from_fork` — **giống hệt 5.4**.