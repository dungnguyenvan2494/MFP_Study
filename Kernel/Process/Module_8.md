# MODULE 8 — `copy_thread()` trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/copy-thread-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/copy-thread-arm64.md) (3 sơ đồ: giải phẫu 3 struct, register diagram cha→con, sequence "vì sao `ret_from_fork`").

**File nguồn**: `arch/arm64/kernel/process.c` (`copy_thread`, `__switch_to`), `arch/arm64/kernel/entry.S` (`cpu_switch_to`, `ret_from_fork`), `arch/arm64/include/asm/processor.h` (`thread_struct`, `cpu_context`, `start_thread`, `task_pt_regs`), `arch/arm64/include/asm/ptrace.h` (`pt_regs`), `arch/arm64/include/asm/switch_to.h` (`switch_to` macro), `kernel/sched/core.c` (`schedule_tail`, `finish_task_switch`).

---

## 0. Ý chính

`copy_thread()` không "chạy" gì cả — nó chỉ **dựng sẵn hai ảnh chụp thanh ghi** cho con:

1. **`pt_regs`** ở đỉnh kernel stack mới của con = bản sao `pt_regs` của cha, sửa đúng một chỗ: `regs[0] = 0`. Đây là thứ con sẽ khôi phục khi `eret` về EL0 → con thấy `fork()` trả về 0, ở đúng lệnh sau `svc`, với mọi thanh ghi khác giống hệt cha.
    
2. **`cpu_context`** trong `thread_struct` của con, dựng tay: `pc = ret_from_fork`, `sp = &pt_regs`. Đây là thứ `cpu_switch_to` sẽ nạp lần đầu bộ lập lịch chọn con → CPU rơi vào `ret_from_fork` với `sp` trỏ sẵn vào frame `pt_regs`.
    

---

## 1. Ba cấu trúc

### `struct pt_regs` — ảnh chụp CPU tại ranh giới EL0↔EL1

```c
struct pt_regs {
    union {
        struct user_pt_regs user_regs;
        struct {
            u64 regs[31];   // x0 .. x30  của USER
            u64 sp;          // SP_EL0 của user
            u64 pc;          // = ELR_EL1 — địa chỉ user quay lại
            u64 pstate;      // = SPSR_EL1 — PSTATE của user (EL0t, DAIF, NZCV)
        };
    };
    u64 orig_x0;             // x0 gốc, để restart syscall khi bị signal cắt
    s32 syscallno;           // số syscall (-1 nếu không phải)
    u32 unused2;
    u64 orig_addr_limit;
    u64 pmr_save;            // ICC_PMR_EL1 (pseudo-NMI)
    u64 stackframe[2];       // frame record giả để unwinder dừng
};
```

- **Nằm ở đỉnh kernel stack**: `task_pt_regs(p) = (struct pt_regs *)(THREAD_SIZE + task_stack_page(p)) - 1`.
- Ai ghi: macro `kernel_entry` trong `entry.S`, **mỗi lần** vào EL1 từ EL0.
- Dùng để: `eret` về đúng chỗ user · ABI syscall (`regs[0..7]` = tham số, `regs[8]` = số syscall, `regs[0]` = kết quả) · `ptrace` đọc/ghi thanh ghi user · signal frame lưu/khôi phục từ đây · `copy_thread` khởi tạo cho con.

### `struct cpu_context` — ảnh chụp khi task KHÔNG chạy

```c
struct cpu_context {
    unsigned long x19, x20, x21, x22, x23, x24, x25, x26, x27, x28;  // callee-saved
    unsigned long fp;   // x29
    unsigned long sp;   // SP_EL1 tại điểm task gọi schedule()
    unsigned long pc;   // điểm task tỉnh dậy (cpu_switch_to nạp vào lr rồi ret)
};
```

- Chỉ **13 giá trị**. Đủ để đóng băng một task tại chỗ nó gọi `schedule()` (một lời gọi hàm C bình thường → chỉ callee-saved + `sp` + địa chỉ trở về là cần lưu; caller-saved đã nằm trên stack theo AAPCS64).
- Ai ghi: `cpu_switch_to` (asm) mỗi context switch.
- Nằm trong `task_struct.thread.cpu_context` — offset `THREAD_CPU_CONTEXT` do `asm-offsets.c` sinh ra.

### `struct thread_struct` — toàn bộ trạng thái kiến trúc của task

```c
struct thread_struct {
    struct cpu_context cpu_context;   // ◀── ở trên
    struct {
        unsigned long tp_value;       // TLS → nạp vào TPIDR_EL0
        unsigned long tp2_value;
        struct user_fpsimd_state fpsimd_state;   // V0..V31 + FPSR/FPCR
    } uw;                              // "user-writable" — whitelist cho hardened usercopy / ptrace
    unsigned int fpsimd_cpu;
    void *sve_state; unsigned int sve_vl;
    unsigned long fault_address;       // FAR_EL1 của fault gần nhất
    unsigned long fault_code;          // ESR_EL1
    struct debug_info debug;           // hw breakpoint / watchpoint
};
```

- Là **trường cuối** của `task_struct` (bắt buộc — có phần variable-size cho FP/SVE, và macro offset giả định nó ở cuối).

> **Đừng lẫn hai bộ `x19..x28`**: một bộ trong `pt_regs` (giá trị _user_, khôi phục khi `eret`), một bộ trong `cpu_context` (giá trị _kernel_, dùng khi lập lịch). Hoàn toàn độc lập.

---

## 2. `copy_thread()` — mã và giải thích

```c
int copy_thread(unsigned long clone_flags, unsigned long stack_start,
                unsigned long stk_sz, struct task_struct *p, unsigned long tls)
{
    struct pt_regs *childregs = task_pt_regs(p);   // đỉnh kernel stack MỚI của con

    memset(&p->thread.cpu_context, 0, sizeof(struct cpu_context));   // (A) cpu_context = toàn 0
    fpsimd_flush_task_state(p);                                       // con phải reload FP/SIMD lần đầu

    if (!(p->flags & PF_KTHREAD)) {              // ── nhánh USER TASK (fork / pthread) ──
        *childregs = *current_pt_regs();          // (B) COPY NGUYÊN KHỐI pt_regs của cha
        childregs->regs[0] = 0;                   // (C) ◀ con thấy fork() trả về 0

        *task_user_tls(p) = read_sysreg(tpidr_el0);   // (D) con thừa TLS hiện tại của cha
        if (stack_start)                              // (E) clone() có stack riêng (pthread)?
            childregs->sp = stack_start;              //     → SP user của con = stack mới
        if (clone_flags & CLONE_SETTLS)              // (F) pthread truyền TCB mới?
            p->thread.uw.tp_value = tls;
    } else {                                     // ── nhánh KERNEL THREAD ──
        memset(childregs, 0, sizeof(struct pt_regs));
        childregs->pstate = PSR_MODE_EL1h;        // kernel thread chạy ở EL1
        p->thread.cpu_context.x19 = stack_start;  // (G) fn   — hàm kernel cần chạy
        p->thread.cpu_context.x20 = stk_sz;       //     arg  — tham số của fn
    }

    p->thread.cpu_context.pc = (unsigned long)ret_from_fork;   // (H) ◀ điểm con TỈNH DẬY
    p->thread.cpu_context.sp = (unsigned long)childregs;       // (I) ◀ SP kernel con = ngay tại frame pt_regs

    ptrace_hw_copy_thread(p);
    return 0;
}
```

|Bước|Việc|Ý nghĩa|
|---|---|---|
|**A**|`memset` cpu_context = 0|user task: `x19..x28 = 0`, `fp = 0` → `ret_from_fork` dùng `x19` (bằng 0) để biết "đây là user task"|
|**B**|`*childregs = *current_pt_regs()`|Con thừa **toàn bộ** ngữ cảnh user của cha: `x1..x30`, `sp`, `pc` (= `&svc+4`), `pstate` (= `EL0t`, DAIF user)|
|**C**|`childregs->regs[0] = 0`|**Cả sự khác biệt của con so với cha.** Cha nhận `x0 = PID con` (từ đường trả về syscall bình thường); con nhận `x0 = 0` từ đây|
|**D**|`tp_value = read_sysreg(tpidr_el0)`|Đọc TLS **đang sống** trong thanh ghi (có thể lệch giá trị đã lưu) → con dùng chung con trỏ TLS với cha (với `fork()`)|
|**E**|`stack_start` → `childregs->sp`|`fork()`: `stack_start == 0` → con dùng **cùng địa chỉ SP user** như cha (COW tách trang vật lý khi có bên ghi stack). `pthread_create()`: truyền stack mới đã `mmap` → con có stack user riêng|
|**F**|`CLONE_SETTLS` → `tp_value = tls`|`pthread` truyền địa chỉ TCB mới; `fork()` không có cờ này|
|**G**|kthread: `x19 = fn`, `x20 = arg`|Nhét vào `cpu_context` để `ret_from_fork` lấy ra gọi `fn(arg)`|
|**H**|`cpu_context.pc = ret_from_fork`|Lần đầu `cpu_switch_to` chọn con, `ret` sẽ nhảy vào đây|
|**I**|`cpu_context.sp = childregs`|`cpu_switch_to` `mov sp, x9` → SP con trỏ **thẳng vào frame `pt_regs`** ở đỉnh kernel stack mới → `ret_to_user`/`kernel_exit` chỉ việc `ldp` các thanh ghi ra rồi `eret`|

**TLS** trên đường ra: khi bộ lập lịch chọn con, `__switch_to()` gọi `tls_thread_switch(next)` → nạp `next->thread.uw.tp_value` vào `TPIDR_EL0`. Nên `fork()` con có `TPIDR_EL0` = của cha; `pthread` con có TCB riêng.

---

## 3. Register diagram: cha → con

```
CHA — tại lệnh svc (đã nằm trong pt_regs của cha, đỉnh kernel stack cha)
┌───────────────────────────────────────────────────────────────┐
│ x0     = SIGCHLD          (tham số clone)                      │
│ x1..x7 = tham số clone khác                                    │
│ x8     = 220              (__NR_clone)                         │
│ x9..x18= caller-saved (rác sau syscall)                        │
│ x19..x28 = callee-saved của user                              │
│ x29 fp / x30 lr = frame + địa chỉ trở về trong libc          │
│ sp     = SP_EL0 user                                          │
│ pc     = &svc + 4                                             │
│ pstate = EL0t, DAIF theo user, NZCV                          │
│ TPIDR_EL0 = con trỏ TLS                                       │
└───────────────────────────────────────────────────────────────┘
                         │
                         ▼  copy_thread()
        ┌────────────────┴────────────────┐
        ▼ (B)(C)(E)                       ▼ (A)(H)(I)(G)
┌──────────────────────────┐   ┌──────────────────────────────┐
│ pt_regs CỦA CON          │   │ cpu_context CỦA CON          │
│ (đỉnh kernel stack MỚI)  │   │ (task_struct.thread)         │
├──────────────────────────┤   ├──────────────────────────────┤
│ regs[0]   = 0     ◀◀◀    │   │ x19..x28 = 0  (user task)    │
│ regs[1..30] = y hệt cha  │   │           = fn,arg (kthread) │
│ sp   = sp cha  (fork)    │   │ fp  = 0                       │
│      = stack mới (pthr)  │   │ sp  = &(pt_regs của con) ◀◀  │
│ pc   = &svc+4  (của cha) │   │ pc  = ret_from_fork      ◀◀  │
│ pstate = EL0t  (của cha) │   │                              │
└──────────────────────────┘   └──────────────────────────────┘
         │                               │
         │ dùng khi eret về EL0          │ dùng khi cpu_switch_to chọn con
         ▼                               ▼
   con chạy tiếp sau svc            con "tỉnh dậy" ở ret_from_fork
   với x0 = 0                       trên kernel stack mới
```

|Thanh ghi|`pt_regs` con|`cpu_context` con|
|---|---|---|
|`x0`|**0**|—|
|`x1`–`x18`|copy từ cha|—|
|`x19`–`x28`|copy từ cha (giá trị **user**)|**0** / `fn`,`arg`|
|`x29` (fp)|fp user của cha|0|
|`x30` (lr)|lr user của cha|_(không có ô — `pc` đóng vai)_|
|`SP`|SP user cha, hoặc stack mới nếu `clone()` truyền `stack_start`|`= &pt_regs` (đỉnh kernel stack 16 KB **mới**)|
|`PC`|`&svc+4` của cha|`ret_from_fork`|
|`PSTATE`|`EL0t` của cha|_(kthread: `PSR_MODE_EL1h`)_|
|TLS|—|→ `thread.uw.tp_value` = `TPIDR_EL0` của cha (hoặc `tls`)|
|stack|frame ở đỉnh kernel stack 16 KB mới (từ `dup_task_struct`)|`cpu_context.sp` trỏ vào frame đó|

---

## 4. Vì sao con bắt đầu từ `ret_from_fork`

### Cơ chế: `switch_to` → `cpu_switch_to`

`switch_to(prev, next, last)` (macro, `arch/arm64/include/asm/switch_to.h`) → `last = __switch_to(prev, next)`.

`__switch_to()` (C, `process.c`) gọi các `*_thread_switch(next)` (FP/SIMD, TLS, hw breakpoint, `entry_task_switch` → cập nhật `__entry_task` per-CPU…), `dsb(ish)`, rồi:

```c
last = cpu_switch_to(prev, next);   // asm
return last;
```

`cpu_switch_to(prev, next)` — `x0 = prev`, `x1 = next`:

```asm
mov  x10, #THREAD_CPU_CONTEXT       // offset của thread.cpu_context
add  x8, x0, x10                    // &prev->thread.cpu_context
mov  x9, sp
stp  x19, x20, [x8], #16            // ── LƯU callee-saved của prev ──
stp  x21, x22, [x8], #16
stp  x23, x24, [x8], #16
stp  x25, x26, [x8], #16
stp  x27, x28, [x8], #16
stp  x29, x9,  [x8], #16            // fp, sp
str  lr,       [x8]                 // pc = địa chỉ trở về (trong __switch_to)

add  x8, x1, x10                    // &next->thread.cpu_context
ldp  x19, x20, [x8], #16            // ── NẠP callee-saved của next ──
ldp  x21, x22, [x8], #16
ldp  x23, x24, [x8], #16
ldp  x25, x26, [x8], #16
ldp  x27, x28, [x8], #16
ldp  x29, x9,  [x8], #16            // fp, sp
ldr  lr,       [x8]                 // lr = next->cpu_context.pc

mov  sp, x9                         // ◀── ĐỔI KERNEL STACK
msr  sp_el0, x1                     // ◀── current = next
ret                                 // ◀── nhảy vào lr = next->cpu_context.pc
```

Với **task đã chạy trước đó**: `lr` = địa chỉ ngay sau `bl cpu_switch_to` trong `__switch_to` của lần nó bị switch away → nó "trở về" `__switch_to`, rồi `__schedule` return, `schedule` return…

Với **con mới fork**: `copy_thread` đã đặt `cpu_context.pc = ret_from_fork` và `cpu_context.sp = childregs`. Nên `ret` nhảy thẳng vào `ret_from_fork` với `sp` đã trỏ vào frame `pt_regs`.

### `ret_from_fork` (`arch/arm64/kernel/entry.S`)

```asm
SYM_CODE_START(ret_from_fork)
    bl   schedule_tail          // (1) hoàn tất context switch
    cbz  x19, 1f                // (2) x19 == 0 → KHÔNG phải kernel thread
    mov  x0, x20               //     kernel thread: x0 = arg
    blr  x19                   //     kernel thread: gọi fn(arg)
1:  get_current_task tsk
    b    ret_to_user            // (3) user task: đi ra EL0
SYM_CODE_END(ret_from_fork)
```

### Ba lý do con **phải** đi qua đây

|#|Vấn đề|Cách giải quyết|
|---|---|---|
|**1**|Con **chưa từng chạy** → không có "địa chỉ trở về vào `__schedule()`" để `cpu_switch_to`'s `ret` nhảy tới. `context_switch()` đang chạy trên stack của **task khác**.|`copy_thread` cấy một điểm hạ cánh nhân tạo: `cpu_context.pc = ret_from_fork`|
|**2**|Context switch được task khác khởi động với `rq->lock` đang giữ và `preempt_count` bị tắt (`FORK_PREEMPT_COUNT`). Phải nhả chúng **trong ngữ cảnh con** (task nào giữ lock thì phải là task nhả — nhưng ở đây task bắt đầu switch và task kết thúc switch là hai task khác nhau).|`schedule_tail(prev)` → `finish_task_switch(prev)`: `raw_spin_unlock_irq(&rq->lock)`, cân bằng `preempt_count` về 0, nếu `prev` là `TASK_DEAD` thì `put_task_struct(prev)`|
|**3**|Con có thể là **user task** (phải khôi phục `pt_regs` rồi `eret` về EL0) hoặc **kernel thread** (phải gọi `fn(arg)`, không bao giờ về EL0).|`cbz x19` rẽ nhánh: `x19 == 0` (từ `memset` cpu_context) → user; `x19 != 0` (= `fn`) → kernel thread|

### `schedule_tail()` — hàm đầu tiên MỌI task mới chạy

```c
asmlinkage __visible void schedule_tail(struct task_struct *prev)
{
    struct rq *rq;
    finish_task_switch(prev);        // dọn nốt context switch: unlock rq, mmdrop, put_task nếu DEAD
    preempt_enable();                // preempt_count: FORK_PREEMPT_COUNT → 0
    if (current->set_child_tid)
        put_user(task_pid_vnr(current), current->set_child_tid);  // CLONE_CHILD_SETTID
    calculate_sigpending();
}
```

### Sau `ret_from_fork` — user task

`b ret_to_user` → kiểm `_TIF_WORK_MASK` (thường rỗng ngay sau fork) → `kernel_exit 0`:

- `sp` đang trỏ vào `*childregs` (nhờ `cpu_context.sp = childregs`).
- `ldp x0..x30` từ `childregs` → **`x0 = 0`**, `x1..x30` = giá trị của cha.
- `msr elr_el1, childregs->pc` (= `&svc+4` của cha) · `msr spsr_el1, childregs->pstate` (= `EL0t`).
- `msr sp_el0, childregs->sp` (SP user).
- `add sp, sp, #S_FRAME_SIZE` → SP_EL1 con về đỉnh kernel stack.
- `eret` → con ở EL0, **tại lệnh ngay sau `svc`**, `x0 = 0`, trên kernel stack riêng, `mm` riêng (COW).

### Sau `ret_from_fork` — kernel thread

`mov x0, x20` (arg) ; `blr x19` (gọi `fn(arg)`). Hàm `fn` thường là vòng lặp vô hạn (`kthread` → `kthread()` wrapper → gọi threadfn). Nếu `fn` return → rơi vào `do_exit()` (kernel thread không có EL0 để về).

---

## 5. Đối chiếu với `execve()` (để hiểu `pt_regs` bị ghi lại thế nào)

`fork` chỉ **copy** `pt_regs`. `execve` **ghi đè** nó qua `start_thread()`:

```c
static inline void start_thread(struct pt_regs *regs, unsigned long pc, unsigned long sp)
{
    start_thread_common(regs, pc);   // memset(regs, 0, ...) ; regs->pc = pc ; giữ lại syscallno
    regs->pstate = PSR_MODE_EL0t;    // AArch64, EL0
    regs->sp = sp;                    // đỉnh user stack đã dựng argv/envp/auxv
}
```

→ sau `execve`, `pt_regs.pc` = entry point ELF (hoặc `ld.so`), `pt_regs.sp` = stack user mới, mọi `regs[]` = 0. `task_struct`/`pid`/`files` giữ nguyên; `mm` là mới. Con của `fork()` gọi `execve()` = kết hợp cả hai (Module 10).

---

## 6. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|Tên hàm|`copy_thread_tls(clone_flags, sp, sz, p, tls)`|`copy_thread(..., struct kernel_clone_args *)` — 5.10|
|`__switch_to`|các `*_thread_switch` + `cpu_switch_to`|thêm `ptrauth_thread_switch`, `mte_thread_switch`, `erratum_1418040` (5.8–5.11)|
|`cpu_switch_to`|lưu/nạp `cpu_context` + `msr sp_el0`|thêm `ptrauth_keys_install_kernel`, `scs_save`/`scs_load` (Shadow Call Stack, 5.8)|
|`start_thread`|`start_thread_common` + `pstate` + `sp`|thêm `spectre_v4_enable_task_mitigation` (5.10)|
|`ret_from_fork`|`bl schedule_tail; cbz x19,1f; ...; b ret_to_user`|y hệt|
|kernel thread branch|`x19`/`x20` = fn/arg trong `cpu_context`|y hệt|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `copy_thread(..., args)`, `cpu_switch_to` có `scs_save`/`ptrauth_keys_install_kernel`, `struct thread_struct` có `keys_user`/`keys_kernel`. Cơ chế `cpu_context.pc = ret_from_fork` + `sp = childregs` + `regs[0] = 0` — **giống hệt 5.4**.