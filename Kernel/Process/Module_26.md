## MODULE 26 — `current` trên ARM64

Sơ đồ: [docs/diagrams/current-task-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/current-task-arm64.md) — 5 sơ đồ: cách đọc `current`, `SP_EL0` đổi vai theo EL, nạp lại `current` khi vào kernel, context switch đổi gì, vì sao nhanh.

**File nguồn**: `arch/arm64/include/asm/current.h` (`get_current`), `include/linux/thread_info.h` (`current_thread_info`), `arch/arm64/kernel/entry.S` (`kernel_entry` macro, `cpu_switch_to`), `arch/arm64/kernel/process.c` (`__switch_to`, `entry_task_switch`, `DEFINE_PER_CPU(__entry_task)`), `arch/arm64/kernel/asm-offsets.c` (`TSK_TI_FLAGS`).

### 1. `current` là gì trên ARM64

```c
static __always_inline struct task_struct *get_current(void)
{
    unsigned long sp_el0;
    asm ("mrs %0, sp_el0" : "=r" (sp_el0));
    return (struct task_struct *)sp_el0;
}
#define current get_current()
```

**Một lệnh `mrs`** đọc thanh ghi hệ thống `SP_EL0` và ép kiểu thành `task_struct *`. Không truy cập bộ nhớ, không phép AND, không per-CPU lookup.

Vì sao dùng `asm(...)` thô thay `read_sysreg()` (vốn có `volatile`): để trình biên dịch được phép **giữ giá trị `current` trong một thanh ghi suốt một hàm** thay vì đọc lại `mrs` mỗi lần — `current` không đổi giữa hai điểm schedule.

### 2. Vì sao `SP_EL0` chứa được `current`

ARM64 có bộ thanh ghi stack pointer theo exception level: `SP_EL0`, `SP_EL1`, `SP_EL2`, `SP_EL3`. Khi CPU chạy ở EL nào, nó **có thể** chọn dùng `SP_ELx` tương ứng hoặc `SP_EL0` (bit `SPSel`).

- **EL0 (user)**: dùng `SP_EL0` làm stack pointer thật → `SP_EL0` = con trỏ stack user.
- **EL1 (kernel)**: Linux cấu hình dùng `SP_EL1` làm stack pointer → **`SP_EL0` rảnh**. Linux nhét con trỏ `task_struct` của task hiện tại vào đó.

Kết quả: khi ở kernel, `mrs x, sp_el0` cho ra `current`; khi ở user, `SP_EL0` là stack user (và user không có khái niệm `current`). Mỗi CPU có bộ `SP_EL0` riêng ⇒ `current` **tự nhiên là per-CPU**, không cần khoá, không cần biến toàn cục.

### 3. `current_thread_info()`

```c
#define current_thread_info() ((struct thread_info *)current)
```

Trên ARM64 (`CONFIG_THREAD_INFO_IN_TASK=y`), `struct thread_info` là **field đầu tiên** của `struct task_struct`:

```c
struct task_struct {
    struct thread_info thread_info;   // offset 0
    ...
};
```

⇒ `&task->thread_info == task` → chỉ cần ép kiểu, không cộng offset. `TSK_TI_FLAGS` (offset của `thread_info.flags` trong `task_struct`) nhỏ, nên `entry.S` đọc cờ bằng `ldr x19, [tsk, #TSK_TI_FLAGS]`.

**Trước 4.14** (và các arch generic): `thread_info` nằm ở **đáy kernel stack**, và `get_current()` = `current_thread_info()->task` với `current_thread_info() = (SP & ~(THREAD_SIZE-1))`. Nhược điểm: một phép AND + một load, và **stack overflow ghi đè `thread_info` → `current` sai** → khó debug. ARM64 5.4 đã bỏ hẳn cách này.

### 4. Vào kernel từ EL0 — phải nạp lại `current`

Khi CPU ở EL0 và xảy ra exception (`svc` syscall, IRQ, page fault), phần cứng chuyển lên EL1 và nhảy tới vector. Lúc này `SP_EL0` **vẫn đang là stack pointer của user** — chưa phải `current`. Macro `kernel_entry` (nhánh `.if \el == 0`) xử lý:

```asm
mrs     x21, sp_el0                       // x21 = USER SP (cất để nhét vào pt_regs->sp)
ldr_this_cpu tsk, __entry_task, x20       // tsk = task_struct của CPU này (per-CPU var)
msr     sp_el0, tsk                       // SP_EL0 = current  ← từ đây current đúng
ldr     x19, [tsk, #TSK_TI_FLAGS]         // đọc thread_info.flags
```

`__entry_task` là biến per-CPU (`DEFINE_PER_CPU(struct task_struct *, __entry_task)`) được cập nhật mỗi lần context switch (xem mục 5). Nó luôn khớp task đang chạy trên CPU đó, nên `kernel_entry` chỉ cần đọc nó.

Khi exception xảy ra **lúc đã ở EL1** (IRQ lồng trong kernel): `SP_EL0` **đã** là `current` → nhánh `.else` của `kernel_entry` chỉ `get_current_task tsk` (`mrs tsk, sp_el0`), không nạp lại.

Khi trả về user, `kernel_exit` khôi phục `SP_EL0` = `pt_regs->sp` (stack user đã lưu) trước `eret`.

### 5. Context switch — giá trị nào đổi

`__schedule()` → `context_switch()` → `switch_to(prev, next, prev)` → `__switch_to(prev, next)` (C, `process.c`):

```c
struct task_struct *__switch_to(struct task_struct *prev, struct task_struct *next)
{
    fpsimd_thread_switch(next);
    tls_thread_switch(next);
    hw_breakpoint_thread_switch(next);
    contextidr_thread_switch(next);
    entry_task_switch(next);          // (1) __this_cpu_write(__entry_task, next)
    uao_thread_switch(next);
    ssbs_thread_switch(next);
    dsb(ish);                         // xả TLB/cache maintenance treo; cần cho membarrier
    last = cpu_switch_to(prev, next); // (2)(3) — asm
    return last;
}
```

`cpu_switch_to` (asm, `entry.S`):

```asm
// lưu context PREV
mov  x10, #THREAD_CPU_CONTEXT
add  x8, x0, x10                 // x0 = prev
stp  x19, x20, [x8], #16         // callee-saved x19..x28
... stp x29, x9 ...              // x9 = sp
str  lr,  [x8]
// nạp context NEXT
add  x8, x1, x10                 // x1 = next
ldp  x19, x20, [x8], #16
...
ldr  lr,  [x8]
mov  sp, x9                      // (2) SP_EL1 = kernel stack của NEXT
msr  sp_el0, x1                  // (3) SP_EL0 = next  ← current GIỜ = next
ptrauth_keys_install_kernel x1, x8, x9, x10
scs_load_current
ret
```

**Ba chỗ đổi:**

1. `__entry_task` (per-CPU) — dùng khi CPU này lần sau vào kernel từ EL0.
2. `SP_EL1` (`mov sp, x9`) — kernel stack chuyển sang của `next`.
3. `SP_EL0` (`msr sp_el0, x1`) — **con trỏ `current`** chuyển sang `next`.

Sau `msr sp_el0, x1`, mọi `current` trên CPU này = `next`. `cpu_switch_to` chỉ lưu/nạp **callee-saved** (`x19`–`x28`, `fp`, `sp`, `lr`) vì nó là một lời gọi hàm bình thường — caller-saved do trình biên dịch tự lo, và `x0` (giá trị trả `last`) cố ý **không** bị ghi để thực hiện mẹo `switch_to(prev, next, last)` (Module 13).

### Điểm dễ nhầm

|Nhầm|Thực tế|
|---|---|
|"`current` là biến toàn cục / per-CPU trong RAM"|Là **nội dung thanh ghi `SP_EL0`**. Không có biến.|
|"`SP_EL0` luôn là stack pointer"|Chỉ ở EL0. Ở EL1 nó chứa `current`; kernel dùng `SP_EL1`.|
|"`current` đọc được ở EL0"|Không — khái niệm chỉ tồn tại khi ở kernel.|
|"`current_thread_info()` phải mask SP"|Chỉ arch cũ. ARM64 5.4: `thread_info` là field đầu `task_struct`, chỉ ép kiểu.|
|"context switch chỉ đổi `SP_EL0`"|Đổi 3: `__entry_task`, `SP_EL1`, `SP_EL0`.|
|"stack overflow làm hỏng `current`"|Không còn đúng — `thread_info` tách khỏi stack từ 4.14.|
|"vào kernel từ EL1 phải nạp lại `current`"|Không — `SP_EL0` đã đúng; chỉ EL0 entry mới nạp từ `__entry_task`.|

### Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản mới|
|---|---|---|
|`current` = `mrs sp_el0`|có (từ 4.14)|**không đổi**|
|`THREAD_INFO_IN_TASK` trên arm64|bật|không đổi|
|`kernel_entry` EL0 nạp `current` từ `__entry_task`|có|không đổi; 5.12+ chuyển phần lớn low-level entry sang C (`entry-common.c`), `current` vẫn từ `SP_EL0`|
|`scs_load_current` / `ptrauth_keys_install_kernel` trong `cpu_switch_to`|có (tuỳ config)|bật rộng hơn theo mặc định|

> Tree local = **5.10.241**: xác nhận `get_current()` dùng `asm("mrs %0, sp_el0")`; `cpu_switch_to` có `msr sp_el0, x1`; `entry_task_switch` → `__this_cpu_write(__entry_task, next)`; `kernel_entry` EL0 dùng `ldr_this_cpu tsk, __entry_task`. Cơ chế giống hệt 5.4.