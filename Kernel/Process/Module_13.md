# MODULE 13 — Context Switch ARM64 (Linux 5.4)

Sơ đồ đã lưu: [docs/diagrams/context-switch-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/context-switch-arm64.md) (4 sơ đồ: ba lớp, bảng lưu/không-lưu, `struct cpu_context` + asm, Task A→save→Task B→restore).

**File nguồn**: `kernel/sched/core.c` (`__schedule`, `context_switch`, `finish_task_switch`), `arch/arm64/include/asm/mmu_context.h` (`switch_mm`, `__switch_mm`), `arch/arm64/mm/context.c` (`check_and_switch_context`), `arch/arm64/mm/proc.S` (`cpu_do_switch_mm`), `arch/arm64/kernel/process.c` (`__switch_to` + các `*_thread_switch`), `arch/arm64/kernel/entry.S` (`cpu_switch_to`), `arch/arm64/include/asm/processor.h` (`struct cpu_context`, `struct thread_struct`).

---

## 0. Ý chính

Một context switch = **hai việc tách biệt**:

1. **Đổi không gian địa chỉ** — `switch_mm_irqs_off()` → `cpu_do_switch_mm` ghi `TTBR0_EL1` + ASID. Bỏ qua nếu `next` là kernel thread.
2. **Đổi thanh ghi + stack** — `switch_to()` → `__switch_to()` → `cpu_switch_to()`.

`cpu_switch_to` là **một lời gọi hàm C bình thường** (`bl cpu_switch_to` từ `__switch_to`). Nó chỉ tuân **AAPCS64** (chuẩn gọi hàm AArch64): lưu đúng những gì một hàm C phải giữ lại cho caller — **13 giá trị**: `x19`–`x28`, `x29` (fp), `sp`, `x30` (lr). Mọi thứ khác hoặc là caller-saved (C ABI cho phép call phá), hoặc do helper riêng trong `__switch_to` xử lý (FP, TLS, debug), hoặc là việc của subsystem khác (`switch_mm`).

---

## 1. Lớp GENERIC — `context_switch()` (`kernel/sched/core.c`)

```c
static __always_inline struct rq *
context_switch(struct rq *rq, struct task_struct *prev,
               struct task_struct *next, struct rq_flags *rf)
{
    prepare_task_switch(rq, prev, next);
        // sched_info_switch(), perf_event_task_sched_out(prev, next),
        // rseq_preempt(prev), fire_sched_out_preempt_notifiers(prev, next),
        // prepare_task(next)  → next->on_cpu = 1  (SMP)
        // prepare_lock_switch(rq, next, rf)

    arch_start_context_switch(prev);   // paravirt hook — nop trên arm64 bare metal

    /* ── QUYẾT ĐỊNH đổi mm ── */
    if (!next->mm) {                                   // next là KERNEL THREAD
        enter_lazy_tlb(prev->active_mm, next);
        next->active_mm = prev->active_mm;             // "mượn" mm của prev
        if (prev->mm)                                  // prev là user task
            mmgrab(prev->active_mm);                   // giữ mm không bị free
        else
            prev->active_mm = NULL;
    } else {                                           // next là USER TASK
        membarrier_switch_mm(rq, prev->active_mm, next->mm);
        switch_mm_irqs_off(prev->active_mm, next->mm, next);   // ◀── Lớp 2
        if (!prev->mm) {                               // prev là kernel thread rời đi
            rq->prev_mm = prev->active_mm;             // sẽ mmdrop trong finish_task_switch
            prev->active_mm = NULL;
        }
    }

    prepare_lock_switch(rq, next, rf);   // giữ rq->lock XUYÊN QUA switch

    switch_to(prev, next, prev);         // ◀── Lớp 3 (macro → __switch_to)
    barrier();                            // compiler barrier

    return finish_task_switch(prev);     // ◀── chạy trong ngữ cảnh NEXT (Module 9)
}
```

**Generic scheduler làm gì:**

- Chọn `next` (`pick_next_task` — Module 12).
- Cập nhật thống kê (perf, sched_info, rseq).
- **Quyết định** có đổi `mm` không: kernel thread → lazy-TLB (mượn `active_mm`, không đổi TTBR0); user task → gọi `switch_mm_irqs_off`.
- Giữ `rq->lock` **xuyên qua** switch (nhả bởi `finish_task_switch` **trong ngữ cảnh `next`** — đây là lý do `ret_from_fork` phải gọi `schedule_tail`, Module 9).
- Gọi `switch_to()` (bàn giao cho arch).
- Sau khi "trở về" (thực chất là được task khác switch-to lại), gọi `finish_task_switch(prev)`.

`switch_to` là **macro** (`arch/arm64/include/asm/mmu_context.h`... thực ra `asm/switch_to.h`):

```c
#define switch_to(prev, next, last)                    \
    do {                                              \
        ((last) = __switch_to((prev), (next)));        \
    } while (0)
```

---

## 2. Lớp `switch_mm_irqs_off()` — đổi không gian địa chỉ

`arch/arm64/include/asm/mmu_context.h`:

```c
#define switch_mm_irqs_off switch_mm    // arm64 không có biến thể irqs-off riêng

static inline void
switch_mm(struct mm_struct *prev, struct mm_struct *next, struct task_struct *tsk)
{
    if (prev != next)
        __switch_mm(next);
    update_saved_ttbr0(tsk, next);       // SW-PAN: lưu TTBR0 của next vào thread_info
}

static inline void __switch_mm(struct mm_struct *next)
{
    if (next == &init_mm) {               // mm của kernel — không có ánh xạ user
        cpu_set_reserved_ttbr0();         // TTBR0 = trang rỗng, ASID 0
        return;
    }
    check_and_switch_context(next);       // arch/arm64/mm/context.c (Module 2)
}
```

`check_and_switch_context(mm)`:

- Đọc `mm->context.id` (ASID + generation).
- **Fast path**: cùng generation → chỉ `atomic64_set(active_asids[cpu], asid)`. **Không lock, không flush TLB.**
- **Slow path** (ASID rollover): lấy `cpu_asid_lock`, `new_context(mm)`, nếu cạn ASID → `flush_context()` + `local_flush_tlb_all()` + đánh dấu mọi CPU.
- `cpu_switch_mm(mm->pgd, mm)` → `cpu_do_switch_mm` (`proc.S`):
    
    ```
    msr ttbr0_el1, <phys(pgd)>                  ; đổi bảng trang user
    msr ttbr1_el1, <swapper_pgd | ASID << 48>   ; đổi ASID (TCR_EL1.A1 = 1)
    isb
    ```
    
    (kèm bước trung gian "reserved TTBR0" để không có cửa sổ "TTBR0 mới + ASID cũ".)

**Điểm quan trọng**: `switch_mm` chạy **TRƯỚC** `switch_to`, và với **process khác** thì **không flush TLB** (mục TLB gắn ASID khác nên vô hại). Kernel thread thì `switch_mm` bị bỏ qua hoàn toàn.

---

## 3. Lớp ARCH C — `__switch_to()` (`arch/arm64/kernel/process.c`)

```c
__notrace_funcgraph struct task_struct *
__switch_to(struct task_struct *prev, struct task_struct *next)
{
    struct task_struct *last;

    fpsimd_thread_switch(next);         // (1)
    tls_thread_switch(next);            // (2)
    hw_breakpoint_thread_switch(next);  // (3)
    contextidr_thread_switch(next);     // (4)
    entry_task_switch(next);            // (5)
    uao_thread_switch(next);            // (6)
    ssbs_thread_switch(next);           // (7)

    dsb(ish);                           // (8) rào đầy đủ

    last = cpu_switch_to(prev, next);   // (9) ◀── Lớp 4 (asm)
    return last;
}
```

`__notrace_funcgraph` — ftrace function-graph không được trace hàm này (return-address stack của nó sẽ hỏng khi vắt qua switch).

**ARM64 `process.c` làm gì** — swap các trạng thái per-thread mà `cpu_switch_to` **không** đụng tới:

|#|Helper|Việc|
|---|---|---|
|1|`fpsimd_thread_switch(next)`|Nếu `!TIF_FOREIGN_FPSTATE` và prev đã "bẩn" FP → `fpsimd_save()` lưu `V0`–`V31` + `FPSR`/`FPCR` vào `prev->thread.uw.fpsimd_state`. Set `TIF_FOREIGN_FPSTATE` cho `next` → next sẽ **trap** khi chạm lệnh FP đầu tiên và nạp lại (`fpsimd_restore_current_state`). **Lazy** — không lưu/nạp mỗi switch.|
|2|`tls_thread_switch(next)`|`prev->thread.uw.tp_value = read_sysreg(tpidr_el0)`; `write_sysreg(next->thread.uw.tp_value, tpidr_el0)`; `write_sysreg(next->thread.uw.tp2_value, tpidrro_el0)` (compat). TLS per-thread.|
|3|`hw_breakpoint_thread_switch(next)`|Nếu prev hoặc next dùng hw breakpoint/watchpoint → nạp `DBGBVR<n>_EL1`/`DBGBCR<n>_EL1`/`DBGWVR`/`DBGWCR`. Chỉ khi cần.|
|4|`contextidr_thread_switch(next)`|`write_sysreg(task_pid_nr(next), contextidr_el1)` — để debug ngoài / CoreSight ETM gán trace theo PID. Chỉ khi `CONFIG_PID_IN_CONTEXTIDR`.|
|5|`entry_task_switch(next)`|`__this_cpu_write(__entry_task, next)` — per-CPU pointer mà `kernel_entry` (khi vào từ EL0) dùng để nạp `current` vào `SP_EL0` (Module 3/6). Khác với `msr sp_el0` trong `cpu_switch_to`: cái đó set `current` cho **kernel đang chạy NGAY**; `__entry_task` là nguồn cho **lần vào EL0→EL1 KẾ TIẾP**.|
|6|`uao_thread_switch(next)`|Set/clear `PSTATE.UAO` theo `addr_limit` của next (USER_DS vs KERNEL_DS) — cho lệnh `LDTR`/`STTR` mà `copy_*_user` dùng.|
|7|`ssbs_thread_switch(next)`|Set `PSTATE.SSBS` theo lựa chọn mitigation spec-store-bypass của next (`prctl(PR_SPEC_STORE_BYPASS)`).|
|8|`dsb(ish)`|Rào bộ nhớ đầy đủ: hoàn tất mọi TLB/cache maintenance đang chờ trước khi task có thể migrate sang CPU khác. `sys_membarrier()` cũng yêu cầu rào ở đây.|

---

## 4. Lớp ARCH ASM — `cpu_switch_to(prev, next)` (`arch/arm64/kernel/entry.S`)

```asm
SYM_FUNC_START(cpu_switch_to)
    mov     x10, #THREAD_CPU_CONTEXT     // = offsetof(task_struct, thread.cpu_context)
    add     x8, x0, x10                  // x8 = &prev->thread.cpu_context   (x0 = prev)
    mov     x9, sp                       // x9 = SP hiện tại (SP_EL1)

    stp     x19, x20, [x8], #16          // ┐
    stp     x21, x22, [x8], #16          // │ LƯU 10 callee-saved của prev
    stp     x23, x24, [x8], #16          // │
    stp     x25, x26, [x8], #16          // │
    stp     x27, x28, [x8], #16          // ┘
    stp     x29, x9,  [x8], #16          // fp (x29), sp (x9)
    str     lr,       [x8]               // pc ← lr (x30)  = địa chỉ sau "bl cpu_switch_to"

    add     x8, x1, x10                  // x8 = &next->thread.cpu_context    (x1 = next)
    ldp     x19, x20, [x8], #16          // ┐
    ldp     x21, x22, [x8], #16          // │ KHÔI PHỤC 10 callee-saved của next
    ldp     x23, x24, [x8], #16          // │
    ldp     x25, x26, [x8], #16          // │
    ldp     x27, x28, [x8], #16          // ┘
    ldp     x29, x9,  [x8], #16          // fp, sp
    ldr     lr,       [x8]               // lr ← next->cpu_context.pc

    mov     sp, x9                       // ◀── ĐỔI KERNEL STACK
    msr     sp_el0, x1                   // ◀── current = next
    ret                                  // ◀── nhảy tới lr = next->cpu_context.pc
SYM_FUNC_END(cpu_switch_to)
```

`struct cpu_context` (`arch/arm64/include/asm/processor.h`):

```c
struct cpu_context {
    unsigned long x19, x20, x21, x22, x23, x24, x25, x26, x27, x28;  // 10 callee-saved
    unsigned long fp;   // x29
    unsigned long sp;   // SP_EL1
    unsigned long pc;   // = lr lúc gọi cpu_switch_to
};  // 13 × 8 = 104 byte
```

**ARM64 assembly làm gì**: lưu 13 giá trị của `prev` vào `prev->thread.cpu_context`, nạp 13 giá trị của `next` từ `next->thread.cpu_context`, đổi `sp`, đổi `sp_el0` (`current`), rồi `ret` — nhảy tới `pc` đã khôi phục của `next`.

---

## 5. Register nào được SAVE, register nào KHÔNG — và tại sao

### ĐƯỢC lưu vào `cpu_context` (13 giá trị)

|Register|Vai trò AAPCS64|Tại sao PHẢI lưu|
|---|---|---|
|`x19`–`x28`|**Callee-saved**|Một hàm C phải giữ lại cho caller. `__switch_to`, `__schedule`, và mọi hàm phía trên trong call chain của `next` có thể đang giữ giá trị sống ở đây. Khi `next` chạy lại, `x19`–`x28` từ lần `next` bị switch-out phải được khôi phục để call chain của `next` nhất quán.|
|`x29` (FP)|**Callee-saved** (frame pointer)|Chuỗi frame pointer phải nhất quán cho `next` (unwinding, backtrace).|
|`x30` (LR)|Link register|**Đây chính là "next chạy tiếp ở đâu"** — địa chỉ trở về vào `__switch_to` của `next`. `ret` cuối cùng nhảy tới LR đã khôi phục.|
|`SP` (SP_EL1)|Stack pointer|Kernel stack **chính là danh tính "task đang làm gì dở"**. Đổi `sp` = đổi call chain đang chạy = đổi task.|

### KHÔNG lưu ở đây — và tại sao không cần

|Register/state|Tại sao KHÔNG lưu trong `cpu_switch_to`|
|---|---|
|`x0`–`x7` (arg/return, caller-saved)|C ABI: một lời gọi hàm **được phép** phá chúng — caller (`__switch_to`) không kỳ vọng chúng còn nguyên sau `bl`. **`x0 = prev` được cố ý GIỮ NGUYÊN** (cpu_switch_to không ghi `x0`) để `__switch_to` `return last` = `x0` = tham số thứ ba của `switch_to` (Module 9).|
|`x8` (indirect result), `x9`–`x15` (temp)|Caller-saved. `x8`/`x9` dùng làm scratch ngay trong `cpu_switch_to`.|
|`x16` (IP0), `x17` (IP1)|Scratch nội-thủ-tục, linker veneer có thể phá — caller-saved.|
|`x18`|5.4: reserved/không dùng. 5.10+ với Shadow Call Stack: xử lý riêng bằng `scs_save`/`scs_load_current` trong `cpu_switch_to`.|
|**Giá trị caller-saved đang SỐNG** (`x0`–`x18` mà `__switch_to` còn cần)|**KHÔNG mất!** Compiler khi sinh `__switch_to` biết `bl cpu_switch_to` là một lời gọi → **spill** các giá trị caller-saved còn sống vào **stack frame của `__switch_to`** (nằm trên kernel stack, ở đỉnh). Stack đi theo `sp` — nên khi `prev` chạy lại và `sp` được khôi phục → frame → giá trị tự nạp lại. Chúng được bảo toàn **qua stack**, không qua `cpu_context`.|
|`PSTATE` (NZCV, DAIF, ...)|(a) NZCV caller-saved — một hàm được phép sửa cờ. (b) DAIF: switch xảy ra với **IRQ tắt** (`rq->lock` giữ với `_irq`), và **mọi** task vào/ra `__schedule` đều ở cùng trạng thái DAIF → ngầm nhất quán. (c) `cpu_switch_to` là `ret` của hàm, **không phải `eret`** — không có SPSR để khôi phục PSTATE.|
|FP/SIMD `V0`–`V31`, `FPSR`/`FPCR`|Lớn (>500 byte), thường không dùng. `fpsimd_thread_switch` lưu **lười** (chỉ khi prev bẩn); `next` nạp lại khi chạm lệnh FP đầu tiên qua trap. **Không nằm trong `cpu_context`.**|
|`TPIDR_EL0` (TLS)|`tls_thread_switch` swap qua đọc/ghi sysreg, lưu vào `thread.uw.tp_value`.|
|`DBGBVR`/`DBGWVR` (debug)|`hw_breakpoint_thread_switch` — chỉ nạp khi có task dùng.|
|`TTBR0_EL1` / ASID|`switch_mm_irqs_off` đã làm **trước** `switch_to`. Đổi không gian địa chỉ là mối lo tách biệt.|
|`SP_EL0` (kernel: giữ `current`)|Được **ghi** (`msr sp_el0, x1`), không "lưu". Giá trị `SP_EL0` của `prev` không cần lưu — nó suy được (`prev` chính là `x0`).|

**Tóm một câu**: `cpu_switch_to` chỉ lưu **13 giá trị đủ để một lời gọi hàm C hồi phục đúng**. Phần còn lại: hoặc caller-saved (C ABI cho phá), hoặc đã ở trên stack (spill), hoặc helper riêng lo (FP/TLS/debug), hoặc subsystem khác lo (mm).

---

## 6. Task A → save → Task B → restore (diagram)

### ① A đang chạy trong kernel, vừa gọi `schedule()`

```
CPU regs:
  x19..x28 = giá trị SỐNG của A
  sp       = kernel stack A (trỏ vào __switch_to frame)
  x0 = A (prev) , x1 = B (next)
  lr = &(lệnh sau "bl cpu_switch_to" trong __switch_to)

A's kernel stack:
  ... | pipe_read frame | __schedule frame | __switch_to frame (compiler spill x0..x18 sống) | ← sp

A.cpu_context: <giá trị cũ — của lần A bị switch-out trước, sắp bị ghi đè>
```

### ② `cpu_switch_to(A, B)` — LƯU A

```
A.cpu_context.x19..x28 ← x19..x28 hiện tại
A.cpu_context.fp       ← x29
A.cpu_context.sp       ← sp   (đang trỏ vào __switch_to frame trên stack A)
A.cpu_context.pc       ← lr   (điểm A chạy tiếp khi được chọn lại)
```

### ③ `cpu_switch_to` — KHÔI PHỤC B (từ `B.cpu_context` lưu ở lần B bị switch-out **trước đó**)

```
x19..x28 ← B.cpu_context.x19..x28
x29      ← B.cpu_context.fp
x9       ← B.cpu_context.sp  →  mov sp, x9   (giờ trên kernel stack B)
lr       ← B.cpu_context.pc
msr sp_el0, x1              →  current = B
```

### ④ `ret` → đang chạy như B

```
CPU regs:
  x19..x28 = giá trị của B (từ lần B ngủ)
  sp       = kernel stack B
  x0       = ???  ← cpu_switch_to KHÔNG ghi x0 → còn là giá trị lúc B gọi cpu_switch_to
                    = prev mà B truyền vào lần đó = task Z (task chạy trước B)
  pc       = B.cpu_context.pc = "lệnh sau bl cpu_switch_to trong __switch_to CỦA B"

B's kernel stack:
  ... | __schedule frame | __switch_to frame (spill x0..x18 CỦA B) | ← sp

→ __switch_to tiếp tục: return last;   (last = x0 = Z)
→ context_switch: return finish_task_switch(prev = Z);   ← dọn task Z (Module 9)
→ __schedule return → schedule return → B chạy tiếp chỗ nó ngủ
```

**Chú ý `x0`**: khi B "trở về" từ `cpu_switch_to`, `x0` không được khôi phục — nó giữ nguyên giá trị lúc B _gọi_ `cpu_switch_to` (lần B ngủ), tức `prev` của B lúc đó = Z. Nên `last = Z`, `finish_task_switch(Z)` dọn task Z đã bị B thay. Đây là toàn bộ ý nghĩa tham số thứ ba `last` của macro `switch_to(prev, next, last)`.

---

## 7. Ba câu hỏi trong prompt — trả lời gọn

**Generic scheduler làm gì?** (`context_switch`) Chọn `next`; cập nhật perf/sched_info/rseq; **quyết định** đổi `mm` hay không (kernel thread → lazy-TLB, không đổi TTBR0; user task → gọi `switch_mm_irqs_off`); giữ `rq->lock` xuyên qua switch; gọi `switch_to()`; sau đó gọi `finish_task_switch()` (thực ra chạy trong ngữ cảnh `next`).

**ARM64 `process.c` làm gì?** (`__switch_to`) Swap các trạng thái **per-thread** mà assembly không đụng: FP/SIMD (lười), TLS (`TPIDR_EL0`), hw breakpoint, `CONTEXTIDR_EL1`, `__entry_task` per-CPU, `PSTATE.UAO`/`SSBS`. Rồi `dsb(ish)`. Rồi gọi `cpu_switch_to`.

**ARM64 assembly làm gì?** (`cpu_switch_to`) Lưu 13 giá trị (`x19`–`x28`, `fp`, `sp`, `lr`) của `prev` vào `prev->thread.cpu_context`; nạp 13 giá trị của `next`; `mov sp, x9` (đổi kernel stack); `msr sp_el0, x1` (`current = next`); `ret` (nhảy tới `pc` đã khôi phục của `next`).

**Register nào được save?** `x19`–`x28`, `x29` (fp), `sp` (SP_EL1), `x30` (lr → `cpu_context.pc`). Tổng 13.

**Register nào KHÔNG save ở đây, tại sao?** `x0`–`x18` + NZCV (caller-saved — C ABI cho phép call phá; giá trị sống đã spill vào stack frame đi theo `sp`). FP/SIMD, TLS, debug regs (helper riêng trong `__switch_to`). TTBR0/ASID (`switch_mm` làm trước). PSTATE/DAIF (không phải `eret`; DAIF luôn nhất quán khi vào `__schedule`).

---

## 8. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|`cpu_switch_to`|13 giá trị + `msr sp_el0`|+ `ptrauth_keys_install_kernel` + `scs_save x0,x8` / `scs_load_current` (Shadow Call Stack, 5.8)|
|`__switch_to` helpers|7 helper như mô tả|+ `ptrauth_thread_switch` (5.7), `mte_thread_switch` (5.10), `erratum_1418040_thread_switch` (5.9)|
|`struct cpu_context`|`x19..x28, fp, sp, pc`|y hệt|
|`context_switch`|như mô tả|+ `membarrier_switch_mm`, tinh chỉnh `prepare_lock_switch`|
|`switch_mm`|`check_and_switch_context` + `update_saved_ttbr0`|y hệt|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `cpu_switch_to` có `scs_save`/`scs_load_current`/`ptrauth_keys_install_kernel`; `__switch_to` có `mte_thread_switch`, `erratum_1418040_thread_switch`. Cốt lõi — 13 giá trị trong `cpu_context`, lý do caller-saved không cần lưu, cơ chế `last` — **giống hệt 5.4**.