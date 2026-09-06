## MODULE 27 — `struct pt_regs` trên ARM64

Sơ đồ: [docs/diagrams/pt-regs-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/pt-regs-arm64.md) — 5 sơ đồ: bố cục, vị trí trên stack, `kernel_entry` tạo `pt_regs` (5 tình huống), `fork` con, `execve`.

**File nguồn**: `arch/arm64/include/asm/ptrace.h` (`struct pt_regs`), `arch/arm64/include/uapi/asm/ptrace.h` (`struct user_pt_regs`), `arch/arm64/include/asm/processor.h` (`task_pt_regs`, `start_thread`), `arch/arm64/kernel/entry.S` (`kernel_entry`/`kernel_exit`), `arch/arm64/kernel/asm-offsets.c` (`S_FRAME_SIZE`, `S_PC`, `S_LR`, `S_SYSCALLNO`), `arch/arm64/kernel/process.c` (`copy_thread`), `arch/arm64/kernel/syscall.c` (`el0_svc_common`).

### 1. Bố cục

```c
struct pt_regs {
    union {
        struct user_pt_regs user_regs;          // phần ptrace/coredump thấy
        struct {
            u64 regs[31];                       // x0..x30
            u64 sp;                             // SP_EL0 của user
            u64 pc;                             // địa chỉ lệnh user tiếp theo
            u64 pstate;                         // NZCV/DAIF/EL/... của user
        };
    };
    u64 orig_x0;                                // x0 gốc — để restart syscall sau signal
    s32 syscallno;                              // số syscall, hoặc NO_SYSCALL (-1)
    u32 unused2;
    u64 orig_addr_limit;                        // addr_limit trước exception (5.4)
    u64 pmr_save;                               // IRQ priority mask (pseudo-NMI)
    u64 stackframe[2];                          // {fp,pc} giả cho unwinder
    /* 5.10+ thêm: */ u64 lockdep_hardirqs; u64 exit_rcu;
};
```

- **`regs[31]` + `sp` + `pc` + `pstate`** = đúng `struct user_pt_regs` (uapi) — phần `PTRACE_GETREGSET` / coredump `NT_PRSTATUS` nhìn thấy. Layout này **cố định vĩnh viễn**.
- Phần sau `orig_x0` là **nội bộ kernel**, không lộ ra userspace.

**Các thanh ghi và vai trò ABI:**

- `x0`–`x7`: tham số syscall / giá trị trả (kết quả nằm ở `x0`).
- `x8`: số syscall (theo ABI); nhưng kernel đọc số thật từ field `syscallno` sau khi `el0_svc` ghi vào.
- `x19`–`x28`: callee-saved của user.
- `x29` (fp), `x30` (lr): frame pointer & link register của user.
- `sp`: `SP_EL0` = con trỏ stack user.
- `pc`: nạp từ `ELR_EL1` — địa chỉ user sẽ chạy tiếp sau khi `eret`.
- `pstate`: nạp từ `SPSR_EL1` — cờ NZCV, mặt nạ ngắt DAIF, exception level nguồn, bit single-step SS…

### 2. `pt_regs` nằm ở đâu

```c
#define task_pt_regs(p) \
    ((struct pt_regs *)(THREAD_SIZE + task_stack_page(p)) - 1)
```

`THREAD_SIZE` = 16 KB. `pt_regs` chiếm `S_FRAME_SIZE` byte ở **đỉnh** kernel stack; khung hàm kernel (`el0_sync` → `do_el0_svc` → `sys_xxx` → …) mọc xuống dưới nó.

Mỗi lần vào kernel từ EL0, `SP_EL1` bắt đầu ở đỉnh stack → `kernel_entry` trừ `SP` đi `S_FRAME_SIZE` rồi đổ 31 GPR + `sp`/`pc`/`pstate` vào. Nên `pt_regs` của một syscall **luôn ở cùng địa chỉ** (đỉnh stack). Nếu một exception lồng vào giữa kernel (IRQ khi đang chạy syscall), `pt_regs` thứ hai nằm thấp hơn trên stack.

`task_pt_regs(p)` dùng được cho task **đang ngủ** — đó là cách `ptrace(PTRACE_GETREGSET)`, `/proc/<pid>/syscall`, coredump, `KSTK_EIP`/`KSTK_ESP` lấy trạng thái user của task khác.

### 3. `kernel_entry` tạo `pt_regs` — 5 tình huống

`kernel_entry` (macro trong `entry.S`) trước hết luôn lưu 31 GPR:

```asm
stp x0, x1, [sp, #16*0]
stp x2, x3, [sp, #16*1]
... (đến x28, x29)
```

Rồi rẽ theo `\el`:

**Từ EL0** (`.if \el == 0`):

```asm
mrs x21, sp_el0                  // x21 = SP user
ldr_this_cpu tsk, __entry_task   // nạp current
msr sp_el0, tsk
mrs x22, elr_el1                 // x22 = PC user
mrs x23, spsr_el1               // x23 = PSTATE user
stp lr, x21, [sp, #S_LR]         // regs[30]=lr, sp=x21
stp x22, x23, [sp, #S_PC]        // pc=x22, pstate=x23
stp xzr, xzr, [sp, #S_STACKFRAME]// chặn unwinder vượt biên EL0
mov w21, #NO_SYSCALL
str w21, [sp, #S_SYSCALLNO]      // mặc định KHÔNG phải syscall
```

**Từ EL1** (`.else` — exception lồng khi kernel đang chạy):

```asm
add x21, sp, #S_FRAME_SIZE       // sp lưu = SP_EL1 TRƯỚC exception (không phải SP user)
get_current_task tsk             // SP_EL0 đã là current, không nạp lại
// lưu & set addr_limit = USER_DS
stp x29, x22, [sp, #S_STACKFRAME]// unwinder ĐƯỢC nối tiếp qua khung kernel này
```

|Tình huống|EL|`pc`/`pstate` chụp|`sp` chụp|`syscallno`|Context-switch được?|
|---|---|---|---|---|---|
|`svc #0` (syscall) từ EL0|0→1|lệnh user sau `svc`|SP user|số thật (w8) — `el0_svc` ghi|có (ngủ trong syscall)|
|IRQ khi ở EL0|0→1|lệnh user bị ngắt|SP user|`NO_SYSCALL`|có (điểm preempt)|
|page fault / undef từ EL0|0→1|lệnh user gây fault|SP user|`NO_SYSCALL`|có (fault có thể ngủ)|
|lời gọi hàm nội bộ kernel (`ksys_read`…)|—|**không tạo `pt_regs`**|—|—|—|
|IRQ / fault khi ở EL1|1→1|lệnh **kernel** bị ngắt|SP_EL1 trước đó|`NO_SYSCALL`|chỉ IRQ-from-EL1 nếu `CONFIG_PREEMPT` & `preempt_count==0`|

"Syscall từ kernel" không tồn tại theo nghĩa `svc`: kernel gọi thẳng hàm C, không tạo `pt_regs` mới.

### 4. `fork` con — `copy_thread` đặt `pt_regs`

```c
struct pt_regs *childregs = task_pt_regs(p);   // đỉnh kernel stack CON

if (!(p->flags & PF_KTHREAD)) {
    *childregs = *current_pt_regs();            // COPY nguyên pt_regs của CHA
    childregs->regs[0] = 0;                     // con thấy fork()/clone() trả 0
    if (stack_start)                            // clone() có newsp
        childregs->sp = stack_start;            // fork()/vfork() newsp=0 → giữ SP cha
    if (clone_flags & CLONE_SETTLS)
        p->thread.uw.tp_value = tls;
} else {
    memset(childregs, 0, sizeof(*childregs));
    childregs->pstate = PSR_MODE_EL1h;          // kthread: không có EL0 để về
}
p->thread.cpu_context.pc = (unsigned long)ret_from_fork;
p->thread.cpu_context.sp = (unsigned long)childregs;
```

`pc`/`pstate` **giữ nguyên của cha** → khi con cuối cùng `eret`, nó quay lại đúng lệnh user ngay sau `svc` của `fork()`, ở EL0, chỉ khác `x0 = 0`. `cpu_context.pc = ret_from_fork` là để lần **đầu** scheduler chọn con: `cpu_switch_to` nạp `sp = childregs`, nhảy `ret_from_fork` → `schedule_tail` → `ret_to_user` → `kernel_exit` đọc `childregs` → `eret`.

### 5. `execve` — `start_thread` viết đè `pt_regs`

```c
static inline void start_thread_common(struct pt_regs *regs, unsigned long pc)
{
    memset(regs, 0, sizeof(*regs));             // XOÁ SẠCH mọi GPR cũ
    forget_syscall(regs);                       // syscallno = NO_SYSCALL, orig_x0 = 0
    regs->pstate = PSR_MODE_EL0t;               // về EL0, AArch64, DAIF mở
}
static inline void start_thread(struct pt_regs *regs, unsigned long pc, unsigned long sp)
{
    start_thread_common(regs, pc);
    regs->pstate |= PSR_SSBS_BIT;
    regs->pc = pc;                              // = ELF entry (hoặc entry của ld.so)
    regs->sp = sp;                              // = đỉnh stack user vừa dựng (argc/argv/envp/auxv)
}
```

`task_struct`, PID, `files_struct` (trừ `O_CLOEXEC`) giữ; `mm` bị thay (`exec_mmap`); `cred` có thể đổi (suid). Nhưng `pt_regs` thì **bị xoá trắng**: chương trình mới theo ABI **không được** thừa kế thanh ghi cũ, và `x0` **không** phải mã lỗi — nó bắt đầu = 0.

||`fork` con|`execve`|
|---|---|---|
|`regs[1..30]`|copy từ cha|memset 0|
|`regs[0]`|= 0 (giá trị trả)|= 0 (memset)|
|`pc`|giữ của cha (lệnh sau `svc`)|ELF/ld.so entry|
|`sp`|giữ của cha (hoặc `newsp`)|đỉnh stack user mới|
|`pstate`|giữ của cha|đặt lại `PSR_MODE_EL0t`|
|`syscallno`|giữ (đang trong `clone`)|`NO_SYSCALL`|

### Điểm dễ nhầm

|Nhầm|Thực tế|
|---|---|
|"`pt_regs` là toàn bộ context CPU của task"|Chỉ trạng thái **tại ranh giới EL0↔EL1**. Callee-saved của kernel nằm trong `cpu_context`.|
|"`pt_regs` nằm trong `task_struct`"|Nằm ở **đỉnh kernel stack**. `task_struct.thread` mới là nơi giữ `cpu_context`/`thread_struct`.|
|"`x8` trong `regs[8]` là số syscall kernel dùng"|Kernel đọc từ field `syscallno` (do `el0_svc` ghi), không phải `regs[8]`.|
|"`pc` = địa chỉ lệnh `svc`"|`pc` = lệnh **ngay sau** `svc` (từ `ELR_EL1`).|
|"IRQ từ EL1 luôn tạo điểm preempt"|Chỉ khi `CONFIG_PREEMPT` và `preempt_count == 0`.|
|"`fork` con có `pt_regs` mới tinh"|Con **copy** `pt_regs` cha, chỉ sửa `regs[0]` (và `sp` nếu `clone` có `newsp`).|
|"`orig_x0` = `x0` hiện tại"|`orig_x0` là `x0` **trước** syscall — cần để restart syscall khi bị signal cắt (syscall trả `-ERESTARTSYS`).|

### Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản mới|
|---|---|---|
|`orig_addr_limit` trong `pt_regs`|có (`set_fs`/`USER_DS` còn)|**bỏ** khi `set_fs()` bị xoá khỏi arm64 (5.18)|
|`lockdep_hardirqs`, `exit_rcu`|5.4 gốc chưa có|thêm ~5.8–5.10|
|Xử lý `el0_svc`|`entry.S` + `syscall.c`|chuyển nhiều sang C `entry-common.c` (5.12+); struct không đổi|
|`start_thread`|`start_thread_common` + `PSR_MODE_EL0t`|không đổi về ngữ nghĩa|
|layout `regs[31]/sp/pc/pstate`|ABI cố định = `user_pt_regs`|**không bao giờ đổi**|

> Tree local = **5.10.241**: `struct pt_regs` đã có `lockdep_hardirqs`/`exit_rcu`, vẫn còn `orig_addr_limit`. `task_pt_regs()`, `copy_thread`, `kernel_entry` giống 5.4.