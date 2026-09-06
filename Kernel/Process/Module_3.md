# MODULE 3 — User Mode → Kernel Mode: trace `getpid()` trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/syscall-entry-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/syscall-entry-arm64.md) (4 sơ đồ: bảng vector, sequence đầy đủ, kernel stack trước/trong/sau, ai đặt thanh ghi nào).

**File nguồn**: `arch/arm64/kernel/entry.S` (vectors, `kernel_ventry`, `kernel_entry`, `el0_sync`, `el0_svc`, `ret_to_user`, `kernel_exit`), `arch/arm64/kernel/syscall.c` (`el0_svc_handler`, `el0_svc_common`, `invoke_syscall`), `arch/arm64/kernel/asm-offsets.c` (`S_*`), `kernel/sys.c` (`SYSCALL_DEFINE0(getpid)`), `arch/arm64/include/asm/{ptrace.h,esr.h,thread_info.h}`.

---

## 0. Chọn ví dụ: vì sao `getpid()`

- Trên aarch64 **không có** `getpid` trong vDSO → luôn là **syscall thật** (khác `gettimeofday`). Đường đi "sạch": không có tham số con trỏ, không copy_from_user, không blocking, không signal.
- `__NR_getpid = 172` (bảng generic `include/uapi/asm-generic/unistd.h`).
- glibc `getpid()` → `svc #0` với `w8 = 172` (số syscall trong **`x8`**, không dùng `imm16` của lệnh `svc`).
- Kết quả: `SYSCALL_DEFINE0(getpid)` trong `kernel/sys.c`:
    
    ```c
    SYSCALL_DEFINE0(getpid)
    {
        return task_tgid_vnr(current);   // TGID trong pid-namespace của caller
    }
    ```
    
    → trả **TGID** (nên tiến trình đa luồng thấy cùng số — Module 20).

---

## 1. Các thanh ghi hệ thống liên quan

|Thanh ghi|Vai trò trong đường syscall|
|---|---|
|**`VBAR_EL1`**|Base bảng vector exception EL1. HW cộng offset để ra handler.|
|**`ESR_EL1`**|_Exception Syndrome Register_. Sau SVC: `EC (bit[31:26]) = 0x15` (SVC từ AArch64), `IL (bit[25]) = 1`, `ISS (bit[15:0]) = imm16` của `svc` (Linux **bỏ qua** ISS). `el0_sync` đọc `EC` để rẽ nhánh.|
|**`ELR_EL1`**|_Exception Link Register_. HW nạp = địa chỉ **lệnh ngay sau `svc`**. `eret` sẽ nhảy về đây.|
|**`SPSR_EL1`**|_Saved Program Status_. HW nạp = `PSTATE` của EL0 lúc bị exception (N/Z/C/V, DAIF, `M[3:0]` = `0b0000` = EL0t). `eret` phục hồi PSTATE từ đây → quyết định quay về **EL0**.|
|**`SP_EL0`**|Khi ở **EL0**: là SP user. Khi ở **EL1** (kernel): ARM64 Linux dùng nó để giữ con trỏ **`current`** (`task_struct*`). HW **không** đụng `SP_EL0` khi có exception.|
|**`SP_EL1`**|SP dùng khi `PSTATE.SP = 1` ở EL1 (`EL1h`). Luôn trỏ **đỉnh kernel stack** của task đang chạy (`task->stack + THREAD_SIZE`). HW đặt `SP ← SP_EL1` khi vào exception.|
|**`TPIDR_EL1`**|Con trỏ per-CPU (`__per_cpu_offset` của CPU hiện tại). Không phải `current` (đó là việc của `SP_EL0` trên arm64 5.4).|
|**`x0`–`x7`**|Tham số syscall (getpid: không dùng). Sau syscall: **`x0`** = giá trị trả về.|
|**`x8`**|Số syscall (172).|
|**`x30` (LR)**|Địa chỉ trở về trong libc (sau lệnh gọi `getpid`). `svc` **không** đụng `x30`.|
|**`x19`–`x28`**|Callee-saved của user — `svc` không đụng; `kernel_entry` vẫn lưu hết vào `pt_regs` để `ptrace`/coredump/unwind thấy đủ.|

---

## 2. `struct pt_regs` (ARM64, 5.4)

`arch/arm64/include/asm/ptrace.h`:

```c
struct pt_regs {
    union {
        struct user_pt_regs user_regs;
        struct {
            u64 regs[31];   // x0..x30
            u64 sp;         // SP_EL0 lúc bị exception (SP user nếu từ EL0)
            u64 pc;          // = ELR_EL1
            u64 pstate;      // = SPSR_EL1
        };
    };
    u64 orig_x0;             // x0 gốc — để restart syscall khi bị signal cắt
    s32 syscallno;           // số syscall (-1 = NO_SYSCALL nếu không phải syscall)
    u32 unused2;
    u64 orig_addr_limit;     // addr_limit (USER_DS/KERNEL_DS) trước khi vào kernel
    u64 pmr_save;            // ICC_PMR_EL1 (pseudo-NMI)
    u64 stackframe[2];       // frame record giả để unwinder dừng
};
```

- `S_FRAME_SIZE = sizeof(struct pt_regs)` (`asm-offsets.c`) ≈ **320 byte** ở 5.4 (5.10 thêm `lockdep_hardirqs`, `exit_rcu` → 336). Căn 16.
- Các offset `S_X0, S_X2, …, S_LR, S_SP, S_PC, S_PSTATE, S_SYSCALLNO, S_ORIG_ADDR_LIMIT, S_STACKFRAME` do `asm-offsets.c` sinh ra để assembly dùng.
- **`pt_regs` nằm ở đỉnh kernel stack**: `task_pt_regs(p) = (struct pt_regs *)(task_stack_page(p) + THREAD_SIZE) - 1`. Mỗi lần vào kernel từ EL0, `kernel_ventry` trừ `sp` đi `S_FRAME_SIZE` để khắc frame này.

---

## 3. Đi từng bước

### Bước 0 — EL0, ngay trước `svc`

```
PSTATE : M = 0b0000 (EL0t) ; DAIF theo user (I=0: IRQ mở) ; NZCV theo phép tính trước
PC     : &(lệnh svc) trong libc
SP     : = SP_EL0 = SP user, nằm trong VMA [stack]
x8     : 172
x0..x7 : (rác — getpid không có tham số)
x30    : địa chỉ trở về trong libc
SP_EL1 : = task->stack + THREAD_SIZE  (đỉnh kernel stack, đang rỗng)
```

libc thực thi: `mov w8, #172` ; `svc #0`.

### Bước 1 — HW xử lý exception (nguyên tử, ~1 chu kỳ, không có lệnh nào chạy)

Phần cứng tự động:

1. `SPSR_EL1 ← PSTATE` (lưu trạng thái EL0, gồm `M[3:0]=EL0t`).
2. `ELR_EL1 ← PC(svc) + 4` (lệnh kế tiếp trong libc).
3. `ESR_EL1 ← (0x15 << 26) | (1 << 25) | 0` — EC = SVC64.
4. `PSTATE`: `M ← EL1h` (chuyển EL1, dùng SP_EL1), `DAIF ← 1111` (mask hết IRQ/FIQ/SError/Debug), `PSTATE.SS ← 0`, `PAN`/`UAO` theo `SCTLR_EL1`.
5. `SP ← SP_EL1` (đỉnh kernel stack).
6. `PC ← VBAR_EL1 + 0x400` (offset của "SVC từ Lower EL, AArch64").
7. **`SP_EL0` không đổi** (vẫn = SP user). **`x0`–`x30` không đổi**.

### Bước 2 — `kernel_ventry 0, sync` (entry 0x400)

```asm
.align 7
sub  sp, sp, #S_FRAME_SIZE        // chừa chỗ pt_regs trên kernel stack
#ifdef CONFIG_VMAP_STACK
    // dùng x0 làm tạm để test bit THREAD_SHIFT của sp:
    // nếu sub vừa rồi tràn xuống guard page → nhảy handler overflow (b el0_inv / __bad_stack)
#endif
b    el0_sync
```

Nếu `CONFIG_UNMAP_KERNEL_AT_EL0` (KPTI, cho core dính Meltdown như Cortex-A75): vector thực tế trỏ vào `tramp_vectors` — trampoline map tối thiểu, đổi `TTBR1_EL1` từ pgd trampoline sang pgd kernel đầy đủ rồi mới `b vectors`. Với đa số core ARM (A53/A55/A57/A72 không dính) thì bỏ qua.

### Bước 3 — `el0_sync` → `kernel_entry 0`

`kernel_entry 0` (macro trong `entry.S`):

```asm
stp x0, x1,  [sp, #16*0]         // lưu x0..x29 thành từng cặp
...
stp x28, x29,[sp, #16*14]
                                  // (x30 lưu sau, cùng S_SP)
// --- vì \el == 0: ---
clear_gp_regs                     // xoá x0..x29 (chống rò dữ liệu kernel ngược nếu speculation)
mrs  x21, sp_el0                  // x21 = SP user  → sẽ vào pt_regs.sp
ldr_this_cpu tsk, __entry_task, x20   // tsk = per-CPU __entry_task (được __switch_to cập nhật)
msr  sp_el0, tsk                  // TỪ ĐÂY: SP_EL0 = current (task_struct*)
ldr  x19, [tsk, #TSK_TI_FLAGS]
disable_step_tsk x19, x20         // tắt single-step trong kernel
// --- chung: ---
mrs  x22, elr_el1                 // x22 = PC trả về (svc+4)
mrs  x23, spsr_el1               // x23 = PSTATE user
stp  lr,  x21, [sp, #S_LR]       // pt_regs.regs[30]=x30 ; pt_regs.sp=x21 (SP user)
stp  xzr, xzr, [sp, #S_STACKFRAME]   // frame record = 0 (unwinder dừng ở ranh giới user)
add  x29, sp, #S_STACKFRAME
stp  x22, x23, [sp, #S_PC]       // pt_regs.pc=elr ; pt_regs.pstate=spsr
mov  w21, #NO_SYSCALL
str  w21, [sp, #S_SYSCALLNO]     // mặc định "không phải syscall" (el0_svc ghi đè)
// (nếu pseudo-NMI: lưu ICC_PMR_EL1 → S_PMR_SAVE, đặt PMR = IRQON)
```

**Sau `kernel_entry`**, `struct pt_regs` trên đỉnh kernel stack đã đầy đủ; `SP` (=`SP_EL1`) trỏ **đáy** frame `pt_regs`; `SP_EL0` = `current`. IRQ vẫn đang **mask** (từ HW).

> `addr_limit` (`USER_DS`/`KERNEL_DS`, uaccess kiểu cũ): với syscall từ EL0, `addr_limit` của task vốn đã là `USER_DS` nên không cần đụng ở đường el0. `orig_addr_limit` chỉ dùng ở đường el1 (nested). (5.10+ đã bỏ hẳn cơ chế `set_fs`.)

### Bước 4 — `el0_sync`: giải mã ESR (assembly, 5.4)

```asm
el0_sync:
    mrs   x25, esr_el1
    lsr   x24, x25, #ESR_ELx_EC_SHIFT   // x24 = EC
    cmp   x24, #ESR_ELx_EC_SVC64        // 0x15?
    b.eq  el0_svc
    cmp   x24, #ESR_ELx_EC_DABT_LOW     // data abort từ EL0?
    b.eq  el0_da
    cmp   x24, #ESR_ELx_EC_IABT_LOW
    b.eq  el0_ia
    cmp   x24, #ESR_ELx_EC_FP_ASIMD
    b.eq  el0_fpsimd_acc
    ...
    b     el0_inv
```

(5.8+: toàn bộ đoạn này thành C `el0_sync_handler()` trong `arch/arm64/kernel/entry-common.c`, dùng `switch (ESR_ELx_EC(esr))`.)

### Bước 5 — `el0_svc`

```asm
el0_svc:
    gic_prio_kentry_setup tmp=x1
    mov   x0, sp                  // x0 = con trỏ pt_regs
    bl    el0_svc_handler         // → C, arch/arm64/kernel/syscall.c
    b     ret_to_user
```

### Bước 6 — `el0_svc_handler` / `el0_svc_common` (C, `syscall.c`)

```c
asmlinkage void el0_svc_handler(struct pt_regs *regs)
{
    sve_user_discard();          // bỏ trạng thái SVE (syscall làm mất)
    el0_svc_common(regs, regs->regs[8], __NR_syscalls, sys_call_table);
}

static void el0_svc_common(struct pt_regs *regs, int scno, int sc_nr,
                           const syscall_fn_t syscall_table[])
{
    regs->orig_x0   = regs->regs[0];    // lưu x0 gốc (restart syscall)
    regs->syscallno = scno;             // = 172
    cortex_a76_erratum_1463225_svc_handler();
    local_daif_restore(DAIF_PROCCTX);   // ◀── BẬT LẠI IRQ: syscall chạy với IRQ mở
    user_exit();                        // context tracking (NO_HZ_FULL)

    if (has_syscall_work(flags)) {      // _TIF_SYSCALL_TRACE/AUDIT/SECCOMP/TRACEPOINT
        scno = syscall_trace_enter(regs);   // ptrace có thể đổi/hủy syscall
        if (scno == NO_SYSCALL) goto trace_exit;
    }
    invoke_syscall(regs, scno, sc_nr, syscall_table);

    if (!has_syscall_work(flags) && !IS_ENABLED(CONFIG_DEBUG_RSEQ))
        return;
trace_exit:
    syscall_trace_exit(regs);
}
```

`invoke_syscall` → `__invoke_syscall`:

```c
static long __invoke_syscall(struct pt_regs *regs, syscall_fn_t syscall_fn)
{
    return syscall_fn(regs);            // __arm64_sys_getpid(regs)
}
// ...
regs->regs[0] = __invoke_syscall(regs, syscall_table[scno]);   // ghi kết quả vào x0
```

`sys_call_table[172]` = `__arm64_sys_getpid`. Macro `SYSCALL_DEFINE0` sinh wrapper `__arm64_sys_getpid(const struct pt_regs *regs)` → gọi `__do_sys_getpid()` → `task_tgid_vnr(current)`.

`current` lấy qua `get_current()` = `mrs x, sp_el0` → `task_struct*`. `task_tgid_vnr` = `pid_nr_ns(task_tgid(current), task_active_pid_ns(current))` → ví dụ **1234**.

→ `regs->regs[0] = 1234`.

### Bước 7 — `ret_to_user`

```asm
ret_to_user:
    disable_daif                        // mask IRQ trước khi kiểm tra/return
    gic_prio_kentry_setup tmp=x3
    ldr   x1, [tsk, #TSK_TI_FLAGS]     // thread_info.flags
    and   x2, x1, #_TIF_WORK_MASK       // NEED_RESCHED|SIGPENDING|NOTIFY_RESUME|FOREIGN_FPSTATE|...
    cbnz  x2, work_pending
finish_ret_to_user:
    enable_step_tsk x1, x2
    kernel_exit 0

work_pending:
    mov   x0, sp                        // regs
    mov   x1, x24
    bl    do_notify_resume              // schedule() / do_signal() / task_work_run() ...
    ldr   x1, [tsk, #TSK_TI_FLAGS]
    b     finish_ret_to_user
```

Với `getpid()` "sạch", thường `_TIF_WORK_MASK == 0` → đi thẳng `kernel_exit 0`. (Nếu tick vừa xảy ra và set `TIF_NEED_RESCHED`, hoặc có signal pending → rẽ `work_pending` — Module 12/15.)

### Bước 8 — `kernel_exit 0` + `eret`

```asm
.macro kernel_exit, el
    ...
    ldp   x21, x22, [sp, #S_PC]        // x21=pt_regs.pc(=svc+4), x22=pt_regs.pstate(=PSTATE user)
    .if \el == 0
        ldr x23, [sp, #S_SP]          // x23 = pt_regs.sp = SP user
        msr sp_el0, x23              // ◀── SP_EL0 quay lại = SP user
        // (SW-PAN / ARM64_WORKAROUND_SPECULATIVE_AT nếu có)
    .endif
    msr   elr_el1, x21               // nơi eret nhảy về
    msr   spsr_el1, x22             // PSTATE eret phục hồi (M = EL0t)
    ldp   x0, x1,  [sp, #16*0]      // khôi phục x0..x29  (x0 = 1234)
    ...
    ldp   x28, x29,[sp, #16*14]
    ldr   lr, [sp, #S_LR]           // x30
    add   sp, sp, #S_FRAME_SIZE     // pop frame → SP_EL1 = đỉnh kernel stack (rỗng lại)
    .if \el == 0
        // KPTI: b tramp_exit (đổi TTBR1 về pgd trampoline) rồi eret ở trang trampoline
    .endif
    eret
.endm
```

`eret` (HW):

- `PSTATE ← SPSR_EL1` → `M = EL0t`, DAIF theo user (IRQ mở lại), NZCV user.
- `PC ← ELR_EL1` = lệnh **ngay sau `svc`** trong libc.
- SP đang dùng ← `SP_EL0` = SP user.
- CPU trở lại **EL0**.

### Bước 9 — về libc

libc thấy `x0 = 1234`, đó là giá trị trả về của `getpid()`. glibc kiểm tra `x0` có phải mã lỗi `-4095..-1` không (không) → trả 1234 cho ứng dụng.

---

## 4. Trạng thái thanh ghi: trước vs sau

||Trước `svc` (EL0)|Ngay sau HW exception|Trong `sys_getpid`|Sau `eret` (EL0)|
|---|---|---|---|---|
|EL / `PSTATE.M`|EL0t|EL1h|EL1h|EL0t|
|PC|`&svc`|`VBAR_EL1+0x400`|trong kernel|`&svc + 4`|
|SP đang dùng|SP_EL0 (user)|SP_EL1 (kernel top)|SP_EL1 (dưới `pt_regs`)|SP_EL0 (user)|
|`SP_EL0`|SP user|SP user (chưa đổi)|**`current`**|SP user|
|`SP_EL1`|kernel stack top|kernel stack top|kernel stack top − `S_FRAME_SIZE`|kernel stack top|
|`ELR_EL1`|(giá trị cũ)|`&svc + 4`|`&svc + 4` (trừ khi kernel sửa)|(đã dùng)|
|`SPSR_EL1`|(cũ)|PSTATE user|PSTATE user|(đã dùng)|
|`ESR_EL1`|(cũ)|EC=0x15, SVC64|(còn đó)|(còn đó)|
|DAIF (IRQ)|user (mở)|**masked**|mở lại (`local_daif_restore`) → masked ở `ret_to_user`|user (mở)|
|`x0`|rác|rác|1234 (sau syscall)|**1234**|
|`x8`|172|172|172 (`regs->syscallno`)|(khôi phục = 172)|
|`x1..x7,x9..x18`|user|user (chưa đổi)|clobbered bởi kernel|**khôi phục từ `pt_regs`**|
|`x19..x30`|user|user|lưu trong `pt_regs`, kernel dùng lại|khôi phục từ `pt_regs`|

Điểm cần nhớ: kernel **khôi phục đầy đủ x0..x30** từ `pt_regs` khi thoát (khác quy ước hàm C — vì đây là ranh giới exception, không phải lời gọi hàm). Chỉ `x0` bị "cố ý" đổi (giá trị trả về). Nếu syscall bị signal cắt, `pt_regs.pc` bị lùi 4 và `pt_regs.regs[0] = pt_regs.orig_x0` để `svc` chạy lại (Module 15).

---

## 5. So sánh nhanh các loại exception từ EL0

|Nguồn|Vector offset|`ESR_EL1.EC`|Handler 5.4 (asm)|Có tạo `pt_regs`?|
|---|---|---|---|---|
|`svc #0` (syscall)|0x400|`0x15` SVC64|`el0_sync` → `el0_svc` → `el0_svc_handler`|Có|
|Data abort (page fault đọc/ghi)|0x400|`0x24` DABT_LOW|`el0_sync` → `el0_da` → `do_mem_abort`→`do_page_fault`|Có|
|Instruction abort (fetch lỗi)|0x400|`0x20` IABT_LOW|`el0_sync` → `el0_ia`|Có|
|Undefined instruction|0x400|`0x00` UNKNOWN|`el0_sync` → `el0_undef` → `do_undefinstr`|Có|
|FP/SIMD access khi lazy-off|0x400|`0x07` FP_ASIMD|`el0_sync` → `el0_fpsimd_acc` → `do_fpsimd_acc`|Có|
|**IRQ** (timer, thiết bị)|**0x480**|(n/a — dùng `el0_irq`)|`el0_irq` → `handle_arch_irq` → chuyển IRQ stack|Có|
|SError|0x580|—|`el0_error` → `do_serror`|Có|
|Syscall/IRQ **từ EL1** (nested)|0x200 / 0x280|—|`el1_sync` / `el1_irq` (`kernel_entry 1`) — không đổi `SP_EL0`, không set USER_DS|Có (frame trên cùng kernel stack)|

Mọi đường đều đi qua `kernel_entry`/`kernel_exit` và đều tạo một `pt_regs` — chỉ khác chỗ rẽ nhánh và việc có `ret_to_user` (chỉ đường từ EL0) hay `kernel_exit 1` thẳng (đường từ EL1).

---

## 6. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|Giải mã `el0_sync`|assembly trong `entry.S` (chuỗi `cmp EC / b.eq`)|C: `el0_sync_handler()` (`entry-common.c`), `switch(ESR_ELx_EC(esr))` — **5.8**|
|Handler syscall|`el0_svc_handler()`|`do_el0_svc()` — 5.8|
|`pt_regs`|kết thúc ở `stackframe[2]`|+`lockdep_hardirqs`, `exit_rcu` (5.10); +PAC/MTE (5.11+)|
|`current`|`SP_EL0` (đã vậy từ ~5.1); `__entry_task` per-CPU làm nguồn khi vào từ EL0|giữ nguyên|
|uaccess|còn `addr_limit`/`set_fs`/`USER_DS` (`orig_addr_limit` trong pt_regs)|bỏ hẳn `set_fs` (5.10), dùng `access_ok` thuần|
|Shadow Call Stack (`x18`), PAC key install ở `kernel_entry`|không có|5.8+|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: bạn sẽ thấy `el0_sync_handler`, `do_el0_svc`, `scs_load_current`, `ptrauth_keys_install_kernel` trong `kernel_entry`. Cơ chế `SP_EL0=current`, `pt_regs` ở đỉnh stack, `ELR/SPSR/ESR`, `eret` — giống hệt 5.4.

---

✅ **MODULE 3 hoàn tất.** Sơ đồ: [docs/diagrams/syscall-entry-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/syscall-entry-arm64.md).

**Kế tiếp: MODULE 4 — `fork()` end-to-end** (`sys_clone`/`sys_fork` → `_do_fork` → `copy_process` → `copy_thread` → `wake_up_new_task` → scheduler → `ret_from_fork` → con về EL0; parent/child return value, khác biệt register/stack, `mm_struct` copy, thiết lập COW). Nói "tiếp" để tôi làm Module 4.