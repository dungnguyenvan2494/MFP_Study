# MODULE 6 — Kernel stack & `dup_task_struct()` trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/kernel-stack-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/kernel-stack-arm64.md) (4 sơ đồ: luồng `dup_task_struct`, bố trí bộ nhớ, `SP` qua 6 tình huống, phát hiện tràn VMAP_STACK).

**File nguồn**: `kernel/fork.c` (`dup_task_struct`, `alloc_thread_stack_node`, `free_thread_stack`), `arch/arm64/kernel/process.c` (`arch_dup_task_struct`), `arch/arm64/include/asm/{memory.h,thread_info.h,processor.h,stacktrace.h}`, `arch/arm64/kernel/entry.S` (`kernel_ventry`, `irq_handler`), `arch/arm64/kernel/irq.c` (`irq_stack`), `arch/arm64/kernel/traps.c` (`handle_bad_stack`).

---

## 0. Ba đối tượng, ba chỗ ở khác nhau

|Đối tượng|Kích thước|Nằm ở đâu|Cấp phát bởi|
|---|---|---|---|
|`struct task_struct`|~7–9 KB|**slab** `task_struct_cachep` (trong linear map)|`alloc_task_struct_node()` → `kmem_cache_alloc_node`|
|`struct thread_info`|~40 B|**field #1 của `task_struct`** (arm64 `CONFIG_THREAD_INFO_IN_TASK`) — **KHÔNG trên stack**|đi cùng `task_struct`|
|kernel stack|`THREAD_SIZE` = **16 KB**|**vmalloc area** (`VMAP_STACK`), 4 trang 4 KB **rời nhau** + guard page|`alloc_thread_stack_node()` → `__vmalloc_node_range`|

Liên kết: `task_struct.stack` (con trỏ) → **đáy** kernel stack. `task_thread_info(p)` = `&p->thread_info`.

> Đây là điểm khác biệt lớn so với Linux đời cũ / ARM32: xưa `thread_info` nằm ở **đáy kernel stack**, `current` suy ra bằng `sp & ~(THREAD_SIZE-1)`. Nay arm64: `thread_info` trong `task_struct`, `current = read_sysreg(SP_EL0)`. Lợi: (a) tràn stack không đè `thread_info`/`task_struct`; (b) lấy `current` chỉ 1 lệnh `mrs`.

---

## 1. `dup_task_struct(orig, node)` — từng bước

```c
static struct task_struct *dup_task_struct(struct task_struct *orig, int node)
{
    node = (node == NUMA_NO_NODE) ? tsk_fork_get_node(orig) : node;

    tsk = alloc_task_struct_node(node);          // (1) task_struct từ SLAB
    stack = alloc_thread_stack_node(tsk, node);  // (2) kernel stack 16KB
    stack_vm_area = task_stack_vm_area(tsk);     //     (vm_struct của vùng vmalloc)

    err = arch_dup_task_struct(tsk, orig);       // (3) *tsk = *orig  (memcpy toàn bộ)

    tsk->stack = stack;                          // (4) sửa lại: trỏ stack MỚI
    tsk->stack_vm_area = stack_vm_area;
    refcount_set(&tsk->stack_refcount, 1);

    setup_thread_stack(tsk, orig);               // (5) no-op khi THREAD_INFO_IN_TASK
    clear_tsk_need_resched(tsk);                 //     bỏ TIF_NEED_RESCHED kế thừa
    set_task_stack_end_magic(tsk);               // (6) STACK_END_MAGIC ở đáy stack
    tsk->stack_canary = get_random_canary();     // (7) stackprotector

    refcount_set(&tsk->usage, 2);                // (8) 1=tồn tại, 1=scheduler (5.4)
    account_kernel_stack(tsk, 1);                //     /proc/meminfo: KernelStack += 16
    return tsk;
}
```

### (1) `alloc_task_struct_node`

`kmem_cache_alloc_node(task_struct_cachep, GFP_KERNEL, node)`. `task_struct_cachep` tạo lúc `fork_init()` với size = `arch_task_struct_size` (gồm cả `thread_struct` variable-size cho FPSIMD). Không dính gì tới stack.

### (2) `alloc_thread_stack_node` (chi tiết mục 2)

### (3) `arch_dup_task_struct(tsk, orig)` — `arch/arm64/kernel/process.c`

```c
int arch_dup_task_struct(struct task_struct *dst, struct task_struct *src)
{
    if (current->mm)
        fpsimd_preserve_current_state();   // lưu FP/SIMD "sống" của parent vào task->thread.uw.fpsimd_state
    *dst = *src;                           // ◀── COPY NGUYÊN KHỐI task_struct
    dst->thread.sve_state = NULL;          // con phải tự cấp buffer SVE
    clear_tsk_thread_flag(dst, TIF_SVE);
    return 0;
}
```

`*dst = *src` copy **luôn cả `thread_info`** (field #1) và **`thread`** (field cuối, gồm `cpu_context`, `uw.fpsimd_state`). Nghĩa là ngay sau dòng này, con thừa hưởng `flags`, `addr_limit`, `preempt_count`, FP state của parent. `copy_thread` (Module 8) sẽ ghi đè `cpu_context` và `pt_regs`.

### (4) Sửa các field bị `*dst=*src` đè bậy

`*dst=*src` vừa copy `src->stack` (con trỏ tới stack của **parent**) sang `dst->stack` — **sai**. Phải gán lại `tsk->stack = stack` (vùng 16 KB mới cấp ở bước 2), `tsk->stack_vm_area`, `tsk->stack_refcount = 1`. Comment trong source ghi rõ: _"arch_dup_task_struct() clobbers the stack-related fields"_.

### (5) `setup_thread_stack(tsk, orig)`

Khi `CONFIG_THREAD_INFO_IN_TASK`: **rỗng** (`thread_info` đã được copy trong bước 3 vì nó là field của `task_struct`). Khi `thread_info` nằm trên stack (arch cũ): hàm này copy `*task_thread_info(tsk) = *task_thread_info(orig)` rồi `->task = tsk`.

### (6) `set_task_stack_end_magic(tsk)`

```c
void set_task_stack_end_magic(struct task_struct *tsk)
{
    unsigned long *stackend = end_of_stack(tsk);   // = tsk->stack (ĐÁY, địa chỉ THẤP nhất)
    *stackend = STACK_END_MAGIC;                    // 0x57AC6E9D
}
```

Ô 8 byte đầu tiên của stack (đáy) giữ magic. `schedule_debug()` mỗi lần `__schedule` kiểm `*end_of_stack(prev) == STACK_END_MAGIC` — nếu bị đè → `panic("Thread overran stack")`. (Với VMAP_STACK thì guard page bắt tràn sớm hơn; magic là lưới an toàn thứ hai / cho non-VMAP.)

### (7) `stack_canary`

`get_random_canary()` — giá trị `-fstack-protector` cho task. (Trên arm64 canary thực dùng khi biên dịch lấy từ `TPIDR_EL0`? Không — arm64 dùng `__stack_chk_guard` toàn cục / per-task field `stack_canary` qua `TSK_STACK_CANARY` offset khi `CONFIG_STACKPROTECTOR_PER_TASK`.)

### (8) refcount

5.4: `refcount_set(&tsk->usage, 2)` — comment: _"One for the user space visible state that goes away when reaped. One for the scheduler."_ (5.10 tách thành `rcu_users = 2` + `usage = 1`.) `put_task_struct()` khi về 0 → `free_task()` → `free_thread_stack()` + `free_task_struct()`.

---

## 2. `alloc_thread_stack_node(tsk, node)` — cấp kernel stack

### Với `CONFIG_VMAP_STACK` (mặc định arm64)

```c
// 1) thử per-CPU cache trước (tránh vmalloc/vfree đắt)
for (i = 0; i < NR_CACHED_STACKS /* = 2 */; i++) {
    s = this_cpu_xchg(cached_stacks[i], NULL);
    if (!s) continue;
    kasan_unpoison_shadow(s->addr, THREAD_SIZE);
    memset(s->addr, 0, THREAD_SIZE);      // stack của task vừa chết → xoá sạch
    tsk->stack_vm_area = s;
    tsk->stack = s->addr;
    return s->addr;
}
// 2) cache rỗng → vmalloc
stack = __vmalloc_node_range(THREAD_SIZE /* 16K */, THREAD_ALIGN /* 32K */,
                             VMALLOC_START, VMALLOC_END,
                             THREADINFO_GFP & ~__GFP_ACCOUNT, PAGE_KERNEL,
                             0, node, __builtin_return_address(0));
tsk->stack_vm_area = find_vm_area(stack);
tsk->stack = stack;
```

- Kết quả: **16 KB địa chỉ ảo liên tục trong vmalloc area**, ánh xạ tới **4 trang vật lý 4 KB rời rạc**. `vmalloc` luôn để **1 guard page** (không map) ngay sau mỗi vùng → chạm vào = translation fault.
- `THREAD_ALIGN = 2 * THREAD_SIZE = 32 KB`: căn địa chỉ để `sp & (1 << THREAD_SHIFT)` (bit 14) = 0 khi `sp` còn trong stack, và lật thành 1 ngay khi `sp` tụt xuống guard page → `kernel_ventry` phát hiện tràn chỉ bằng `tbnz` (mục 5).
- `cached_stacks`: khi task chết, `free_thread_stack()` thử cất `vm_struct` vào `cached_stacks[cpu]` thay vì `vfree` ngay → fork kế tiếp trên CPU đó tái dùng, nhanh hơn nhiều.

### Không `VMAP_STACK`

```c
page = alloc_pages_node(node, THREADINFO_GFP, THREAD_SIZE_ORDER /* = 2 */);
tsk->stack = page_address(page);   // 4 trang LIÊN TỤC vật lý, KHÔNG guard page
```

→ tràn stack **âm thầm** đè `struct` lân cận trong linear map (rất khó debug). Đây là lý do `VMAP_STACK` là mặc định.

### `THREAD_SIZE` được tính (`arch/arm64/include/asm/memory.h`)

```
KASAN_THREAD_SHIFT = 0                       (1 nếu bật KASAN)
MIN_THREAD_SHIFT   = 14 + KASAN_THREAD_SHIFT = 14
// VMAP_STACK && MIN_THREAD_SHIFT(14) < PAGE_SHIFT(12)? → false
THREAD_SHIFT       = MIN_THREAD_SHIFT = 14
THREAD_SIZE        = 1 << 14 = 16384 (16 KB)
THREAD_SIZE_ORDER  = 14 - 12 = 2   (4 trang)
```

(64K pages: `PAGE_SHIFT=16 > 14` → `THREAD_SHIFT = 16` → 64 KB. KASAN 4K pages → 32 KB.)

---

## 3. `struct thread_info` (arm64 5.4)

```c
struct thread_info {
    unsigned long   flags;        // TIF_NEED_RESCHED, TIF_SIGPENDING, TIF_SVE,
                                  // TIF_FOREIGN_FPSTATE, TIF_SYSCALL_TRACE, TIF_32BIT...
    mm_segment_t    addr_limit;   // USER_DS / KERNEL_DS (giới hạn uaccess kiểu cũ)
#ifdef CONFIG_ARM64_SW_TTBR0_PAN
    u64             ttbr0;        // TTBR0 lưu tạm cho SW-PAN
#endif
    union {
        u64 preempt_count;        // 0 = preemptible ; >0 = đang khoá preempt ; <0 = BUG
        struct { u32 count; u32 need_resched; } preempt;
    };
};
```

- Nhúng ở **đầu `task_struct`** (offset 0). `TSK_TI_FLAGS`, `TSK_TI_ADDR_LIMIT`, `TSK_TI_PREEMPT` = offset do `asm-offsets.c` sinh, dùng trong `entry.S` (`ldr x1, [tsk, #TSK_TI_FLAGS]` ở `ret_to_user`).
- `flags` là "cửa" báo hiệu cấp thấp: scheduler set `TIF_NEED_RESCHED`, signal set `TIF_SIGPENDING`, FP switch set `TIF_FOREIGN_FPSTATE`. `ret_to_user` kiểm `_TIF_WORK_MASK` = OR các bit này.
- `preempt_count`: `preempt_disable()` tăng, `preempt_enable()` giảm; = 0 mới cho preempt kernel. `copy_process` khởi tạo con với `FORK_PREEMPT_COUNT` (khoá tới `schedule_tail`).

---

## 4. Bố trí bộ nhớ chính xác (một task con vừa fork)

```
KERNEL VA
│
├── SLAB (linear map)  ── struct task_struct  @ 0xffff_0000_1234_5000  (~9 KB)
│     ├─ +0x000  thread_info { flags, addr_limit, preempt_count }
│     ├─ +0x028  state, stack(ptr), usage, flags
│     │           stack ─────────────────────────────────┐
│     ├─ ...      se, mm, active_mm, files, fs, signal,   │
│     │           sighand, cred, pid, real_parent...      │
│     └─ +0x8xx  thread_struct thread {                   │
│                  cpu_context { x19..x28, fp, sp, pc }   │   (pc=ret_from_fork, sp=childregs)
│                  uw { tp_value, fpsimd_state }          │
│                  fault_address, fault_code, debug }     │
│                                                          │
└── VMALLOC area  ── kernel stack 16 KB                   │
      0xffff_8000_aabb_0000  guard page (KHÔNG map)  ◀────┼── chạm = handle_bad_stack()
      0xffff_8000_aabb_4000  ĐÁY  = tsk->stack  ◀─────────┘
                             [0] = STACK_END_MAGIC 0x57AC6E9D
                             ↑ sp chạy XUỐNG khi gọi hàm sâu
      ...........            (vùng dùng dần)
      0xffff_8000_aabb_7ec0  struct pt_regs  (S_FRAME_SIZE ≈ 320 B)
                             task_pt_regs(tsk) = (pt_regs*)(tsk->stack + 0x4000) - 1
      0xffff_8000_aabb_8000  ĐỈNH = tsk->stack + THREAD_SIZE   ◀── SP_EL1 khi task ở EL0
```

- `task_struct` và stack **cách xa nhau** (slab vs vmalloc). Không có quan hệ "cùng 2 trang" như sách cũ.
- `pt_regs` **luôn ở đỉnh**; `STACK_END_MAGIC` **luôn ở đáy**.
- `end_of_stack(p)` = `p->stack` (arm64, stack mọc xuống, đáy = địa chỉ thấp).

---

## 5. `SP` thay đổi theo tình huống

### (a) Task chạy **user mode (EL0)**

- CPU ở EL0 → chỉ có `SP_EL0`. `SP` đang dùng = `SP_EL0` = **SP user** (trong VMA `[stack]` của process, dịch qua `TTBR0_EL1`).
- `SP_EL1` được giữ sẵn = **đỉnh kernel stack task** (`tsk->stack + THREAD_SIZE`), để dùng ngay khi có exception.
- Kernel stack: **rỗng**, không ai đụng.
- `current` = `read_sysreg(SP_EL0)`? Không — lúc ở EL0, `SP_EL0` = SP user. `current` chỉ hợp lệ khi ở EL1. (Trong khi ở EL0, kernel không chạy nên không cần `current`.)

### (b) **Syscall** EL0→EL1 (`svc`)

1. HW: `SP ← SP_EL1` = đỉnh kernel stack.
2. `kernel_ventry`: `sub sp, sp, #S_FRAME_SIZE` → chừa chỗ `pt_regs` ở đỉnh.
3. `kernel_entry`: `mrs x21, sp_el0` (lưu SP user → `pt_regs.sp`); `ldr_this_cpu tsk, __entry_task`; `msr sp_el0, tsk` → **từ giờ `SP_EL0` = `current`**.
4. Kernel chạy: `sp` **giảm dần** khi `el0_svc_common` gọi `sys_read` gọi `vfs_read` … Mỗi frame vài chục–vài trăm byte. Tổng thường < 4 KB.
5. `kernel_exit`: khôi phục `SP_EL0` = SP user; `add sp, sp, #S_FRAME_SIZE` → `SP_EL1` về đỉnh; `eret`.

### (c) **Interrupt (IRQ)** khi task đang ở EL0

1. HW: `SP ← SP_EL1` = đỉnh kernel stack task; `kernel_entry` khắc `pt_regs`.
2. `irq_handler` (`entry.S`): `mov x29, sp` (lưu task sp); `ldr_this_cpu x1, irq_stack_ptr`; `mov sp, x1` → **chuyển sang IRQ stack per-CPU** (16 KB, `DEFINE_PER_CPU irq_stack`).
3. `handle_arch_irq` → GIC driver → `generic_handle_irq` → handler thiết bị — **chạy trên IRQ stack**, không ăn vào task kernel stack (quan trọng khi handler sâu / IRQ lồng nhau).
4. Xong: `mov sp, x29` → về task kernel stack; `b ret_to_user` (nếu về EL0) — ở đây có thể `schedule()` nếu tick vừa set `TIF_NEED_RESCHED`.

### (d) **Interrupt** khi kernel đang chạy (từ EL1)

1. `SP` đang ở **giữa chừng** task kernel stack (task đang trong syscall chẳng hạn).
2. `kernel_ventry 1`: `sub sp, sp, #S_FRAME_SIZE` → `pt_regs` **nối tiếp** (nested frame) ngay trên chỗ `sp` hiện tại — cùng task kernel stack.
3. Chuyển sang IRQ stack (nếu chưa ở trên đó) để xử lý; `kernel_exit 1` pop nested frame, trả `sp` về đúng chỗ syscall đang dở.
4. Đây là lý do cần 16 KB (không phải 8): IRQ nested + exception có thể chồng vài frame `pt_regs` + call chain handler.

### (e) **Context switch** A → B

`cpu_switch_to(prev=A, next=B)` (`entry.S`), gọi từ `__switch_to` ← `switch_to` ← `context_switch` ← `__schedule` (đang chạy trên **kernel stack của A**):

```asm
mov  x9, sp
stp  x29, x9, [x8, #CPU_CONTEXT_FP]   // A->thread.cpu_context.fp = fp ; .sp = sp (SP_EL1 của A, tại điểm này)
str  lr,      [x8, #CPU_CONTEXT_PC]   // A->thread.cpu_context.pc = địa chỉ trở về trong __switch_to
...
ldp  x29, x9, [x8_B, #CPU_CONTEXT_FP] // fp, sp ← B->thread.cpu_context
mov  sp, x9                            // ◀── SP = kernel stack của B (điểm B từng bị switch-away)
msr  sp_el0, x1                        // current = B
ret                                    // nhảy về B->cpu_context.pc → chạy tiếp trên stack B
```

→ **đổi giá trị `sp` = đổi task**. Stack của A đóng băng nguyên trạng, chờ lần sau A được chọn lại.

### (f) **Process sleep** (tự nguyện `schedule()`)

Task gọi `read()` trên pipe rỗng: `sys_read` → `pipe_read` → `wait_event_interruptible` → `set_current_state(TASK_INTERRUPTIBLE)` → `schedule()` → `__schedule` → `deactivate_task` (gỡ khỏi runqueue) → `context_switch` sang task khác.

- `sp` của task ngủ được lưu trong `thread.cpu_context.sp`.
- Kernel stack của nó **giữ nguyên toàn bộ call chain**:
    
    ```
    [đỉnh] pt_regs (ngữ cảnh EL0 lúc gọi read)
           el0_svc_common
           invoke_syscall
           __arm64_sys_read
           vfs_read
           pipe_read
           schedule           ← sp lưu quanh đây
           __schedule
           (cpu_switch_to đã lưu sp)
    ```
    
- Khi có dữ liệu → `wake_up` → `try_to_wake_up` → `activate_task` (vào runqueue). Lần `__schedule` nào đó chọn lại → `cpu_switch_to` khôi phục `sp` → `ret` → tiếp tục ngay sau `cpu_switch_to` trong `__schedule` **của chính task này** → `__schedule` return → `schedule` return → `pipe_read` tiếp tục vòng lặp, copy dữ liệu → `sys_read` return → `ret_to_user` → `eret` về EL0.

Điểm mấu chốt: **kernel stack là nơi lưu "task đang làm gì dở"**. Sleep = đóng băng stack + cất `sp`. Wake = khôi phục `sp` + chạy tiếp. `pt_regs` ở đỉnh giữ ngữ cảnh user để lúc cùng về được EL0.

---

## 6. Phát hiện tràn kernel stack (VMAP_STACK)

`kernel_ventry` (mỗi lần vào exception) chèn kiểm tra rẻ tiền:

```asm
sub  sp, sp, #S_FRAME_SIZE
add  sp, sp, x0              // sp' = sp + x0   (x0 = giá trị tạm, sẽ khôi phục)
sub  x0, sp, x0             // x0  = sp (giá trị thật sau khi trừ S_FRAME_SIZE)
tbnz x0, #THREAD_SHIFT, 0f  // bit 14 của sp lật? → nhảy handler tràn
...
0:  b   __bad_stack
```

Vì stack căn theo `THREAD_ALIGN = 2*THREAD_SIZE = 32 KB`, khi `sp` còn trong vùng 16 KB hợp lệ thì bit 14 = 0; khi `sp` tụt xuống guard page (16 KB kế) thì bit 14 = 1 → `tbnz` bắt được. `__bad_stack` → chuyển sang **overflow stack per-CPU** (`OVERFLOW_STACK_SIZE = 4 KB`) → `handle_bad_stack()` in `"Insufficient stack space"`, dump `pt_regs`, `panic`.

---

## 7. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|refcount trong `dup_task_struct`|`refcount_set(&tsk->usage, 2)`|tách `rcu_users = 2` + `usage = 1` (5.9)|
|`thread_info`|`flags`, `addr_limit`, `preempt_count`|5.10 vẫn vậy; +`scs_base/scs_sp` nếu Shadow Call Stack (5.8); `addr_limit` bỏ khi bỏ `set_fs` (5.18)|
|`scs_prepare` (shadow call stack cho `x18`)|**không có**|5.8|
|`arch_dup_task_struct`|`fpsimd_preserve_current_state`, `*dst=*src`, `sve_state=NULL`|+PAC key init, +MTE|
|cached stacks|`NR_CACHED_STACKS = 2`|như cũ; 5.19 thêm `DYNAMIC_STACK`? (thử nghiệm)|
|`set_task_stack_end_magic`|có|có|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `dup_task_struct` set `rcu_users=2, usage=1`, có `scs_prepare`, `pf_io_worker`. `alloc_thread_stack_node`, `THREAD_SIZE=16K`, `struct thread_info`, IRQ/overflow stack — giống 5.4.

---

✅ **MODULE 6 hoàn tất.** Sơ đồ: [docs/diagrams/kernel-stack-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/kernel-stack-arm64.md).

**Kế tiếp: MODULE 7 — `fork()` & Copy-On-Write** (trace `copy_mm` → `dup_mm` → `dup_mmap` → `copy_page_range` → write-protect cha+con → COW fault → `do_wp_page` → copy trang → cập nhật PTE → TLB; ví dụ cụ thể một địa chỉ ảo qua từng bước: trước fork / sau fork / sau parent write / sau child write, vẽ page table + trang vật lý). Nói "tiếp" để làm Module 7.