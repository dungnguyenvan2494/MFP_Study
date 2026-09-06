# MODULE 19 — Thread và `clone()` trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/thread-and-clone-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/thread-and-clone-arm64.md) (4 sơ đồ: "every thread is a task_struct", ma trận so sánh 4 cách, trace `pthread_create`→`clone`, luôn-riêng vs luôn-chung).

**File nguồn**: `kernel/fork.c` (`SYSCALL_DEFINE5(clone)`, `kernel_clone`/`_do_fork`, `copy_process`, các `copy_*`), `include/uapi/linux/sched.h` (`CLONE_*`), `arch/arm64/kernel/process.c` (`copy_thread`), glibc `nptl/pthread_create.c` + `sysdeps/unix/sysv/linux/aarch64/clone.S`.

---

## 0. "Every thread is a task_struct" — nghĩa là gì

Linux **không có** kiểu dữ liệu "thread" tách khỏi "process". Chỉ có **`task_struct`** (nhân gọi là _task_). Cái mà scheduler xếp lịch **luôn** là một `task_struct`.

- "Thread" POSIX = **một `task_struct`**.
- "Process" POSIX = một **thread group**: tập các `task_struct` có cùng `task->tgid`. Cái đầu tiên (có `pid == tgid`) là **`group_leader`**.
- `getpid()` trả `current->tgid` → mọi thread trong một chương trình thấy **cùng một số**.
- `gettid()` trả `current->pid` → mỗi thread một số riêng (TID).

Đây là **mô hình 1:1 (NPTL — Native POSIX Thread Library)**: mỗi user thread ⇔ đúng một kernel task. Không có mô hình M:N (như user-level green threads) hay M:1.

**Vì sao thiết kế thế này:**

1. **Đồng nhất tuyệt đối.** `schedule()`, `__switch_to()`, `do_exit()`, `ptrace`, `/proc`, signal delivery — tất cả chỉ thao tác trên `task_struct`. Không có chỗ nào phải rẽ nhánh "đây là process hay thread".
    
2. **Chia sẻ bằng con trỏ + refcount, không bằng "chứa".** Thay vì "một process object chứa danh sách thread object", Linux cho nhiều `task_struct` **cùng trỏ** vào một `mm_struct`/`files_struct`/`fs_struct`/`sighand_struct`/`signal_struct` và tăng refcount. "Process" chỉ là _khung nhìn tổng hợp_: duyệt `signal->thread_head` / `while_each_thread()` để gom các task cùng `tgid`.
    
3. **Kernel thread cũng là `task_struct`** (`mm == NULL`, mượn `active_mm` của task trước). Cha của chúng là `kthreadd` (PID 2). Không cần loại đối tượng thứ ba.
    
4. **Lịch sử.** `task_struct` có từ đầu (Linux 0.01). Khi thêm hỗ trợ đa luồng POSIX (~2.6, NPTL 2003), nhân chỉ thêm cờ **`CLONE_THREAD`** + trường **`tgid`** + cho dùng chung **`signal_struct`** — để "một đống task" _hành xử_ như một process trước mắt userspace (chung PID qua `getpid()==tgid`, tín hiệu nhóm, `exit_group()` giết cả nhóm).
    

Hệ quả tín hiệu:

- `kill(tgid, sig)` → `signal->shared_pending` → **bất kỳ** thread nào không block `sig` sẽ nhận.
- `tgkill(tgid, tid, sig)` → `task->pending` của đúng thread đó.
- Tín hiệu chí mạng / `exit_group()` → `zap_other_threads()` gửi `SIGKILL` cho **toàn bộ** nhóm → cả process chết cùng lúc.

---

## 1. `fork()` và `pthread_create()` dùng CÙNG một syscall

Trên **AArch64 native không có `sys_fork`** (`__ARCH_WANT_SYS_FORK` nằm trong `#ifdef CONFIG_COMPAT`). glibc `fork()` trên arm64 = `clone(SIGCHLD, 0, NULL, NULL, 0)`. glibc `pthread_create()` cũng gọi `clone` — chỉ khác **bộ cờ**.

`SYSCALL_DEFINE5(clone, ...)` (ARM64 dùng thứ tự mặc định, không phải `CLONE_BACKWARDS*`):

```c
SYSCALL_DEFINE5(clone, unsigned long, clone_flags,     // x0
                       unsigned long, newsp,           // x1  — đỉnh stack cho con
                       int __user *,  parent_tidptr,   // x2
                       unsigned long, tls,             // x3
                       int __user *,  child_tidptr)    // x4
{
    struct kernel_clone_args args = {
        .flags       = lower_32_bits(clone_flags) & ~CSIGNAL,
        .pidfd       = parent_tidptr,
        .child_tid   = child_tidptr,
        .parent_tid  = parent_tidptr,
        .exit_signal = lower_32_bits(clone_flags) & CSIGNAL,   // = SIGCHLD với fork; = 0 với pthread
        .stack       = newsp,
        .tls         = tls,
    };
    return kernel_clone(&args);   // 5.4: _do_fork(&args)
}
```

Các cờ (`include/uapi/linux/sched.h`):

|Cờ|Chia sẻ gì|
|---|---|
|`CLONE_VM` (0x100)|`mm_struct` — cùng không gian địa chỉ|
|`CLONE_FS` (0x200)|`fs_struct` — cwd, root, umask|
|`CLONE_FILES` (0x400)|`files_struct` — bảng fd|
|`CLONE_SIGHAND` (0x800)|`sighand_struct` — bảng `action[64]`|
|`CLONE_SYSVSEM` (0x40000)|SysV `SEM_UNDO`|
|`CLONE_THREAD` (0x10000)|Cùng thread group (`tgid`), `exit_signal = -1`|
|`CLONE_SETTLS` (0x80000)|Đặt TLS của con từ tham số `tls`|
|`CLONE_PARENT_SETTID` (0x100000)|Ghi TID con vào `*parent_tidptr`|
|`CLONE_CHILD_CLEARTID` (0x200000)|Khi con chết: ghi 0 vào `*child_tidptr` + `futex_wake` → nền của `pthread_join`|

**Ràng buộc `copy_process` cưỡng chế**: `CLONE_THREAD` ⇒ `CLONE_SIGHAND` ⇒ `CLONE_VM`. Nên một thread **luôn** chia sẻ cả `mm` + `sighand` + `signal`.

---

## 2. So sánh 4 cách tạo

### `fork()` = `clone(SIGCHLD, 0, ...)`

|Resource||
|---|---|
|`mm_struct`|**riêng** — `dup_mm()` (COW)|
|`files_struct`|**riêng** bảng — `dup_fd()`; mỗi `struct file` share qua refcount (offset chung)|
|`fs_struct`|**riêng** — `copy_fs_struct()`|
|`signal_struct`|**riêng** — `signal_struct` mới, `nr_threads = 1`, memcpy `rlim[]`|
|`sighand_struct`|**riêng** — `action[64]` mới (memcpy của cha)|
|`pid` (TID)|**mới**|
|`tgid`|**mới** (= `pid` mới; là thread-group-leader mới)|
|`exit_signal`|`SIGCHLD` (cha nhận thông báo khi con chết)|
|kernel stack|**mới** 16 KB|
|`thread_struct`|**mới** (cpu_context, fpsimd, TLS = copy `TPIDR_EL0` của cha)|
|`nsproxy` / `cred`|chia sẻ (refcount++)|

### `clone(CLONE_VM)` một mình

|Resource||
|---|---|
|`mm_struct`|**CHUNG** (`mmget`)|
|`files_struct` / `fs_struct` / `signal_struct` / `sighand_struct`|**riêng** (không có cờ chia sẻ)|
|`pid`|mới|
|`tgid`|**mới** — **vẫn là process riêng!**|
|`exit_signal`|theo byte thấp của `clone_flags` (0, hoặc `SIGCHLD`...)|
|kernel stack / `thread_struct`|mới; TLS chỉ đặt nếu `CLONE_SETTLS`|

**Điểm mấu chốt**: `CLONE_VM` một mình = "chia sẻ bộ nhớ nhưng KHÔNG chia sẻ danh tính". `getpid()` khác nhau (`tgid` khác), tín hiệu riêng, `wait()` riêng. Đây là kiểu LWP thô — hầu như không dùng trực tiếp trong userspace.

### `clone(CLONE_THREAD | CLONE_SIGHAND | CLONE_VM)` (tối thiểu cho một thread)

|Resource||
|---|---|
|`mm_struct`|**CHUNG** (ép bởi `CLONE_VM`)|
|`sighand_struct`|**CHUNG** (ép bởi `CLONE_SIGHAND`)|
|`signal_struct`|**CHUNG** — `copy_signal` `return 0` ngay → dùng `signal_struct` của leader. **Đây chính là định nghĩa "cùng process".**|
|`files_struct` / `fs_struct`|**riêng** (trừ khi thêm `CLONE_FILES`/`CLONE_FS`)|
|`pid` (TID)|**mới**|
|`tgid`|**= `tgid` của leader** — cùng process|
|`exit_signal`|**−1** — thread chết không gửi `SIGCHLD` (chỉ khi thread cuối chết mới báo cha của leader)|
|kernel stack / `thread_struct`|**mới**|

### `pthread_create()` — bộ cờ đầy đủ

```c
clone_flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SYSVSEM
            | CLONE_SIGHAND | CLONE_THREAD
            | CLONE_SETTLS | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID;
```

|Resource|Process A (`fork`)|Thread B (`pthread_create`)|
|---|---|---|
|`mm_struct`|**riêng** (COW)|**chung** — `mm_users++`|
|`files_struct`|**riêng** bảng (file share qua refcount)|**chung** — `count++`|
|`fs_struct` (cwd, root, umask)|**riêng**|**chung** — `users++`|
|`signal_struct`|**riêng**|**chung** — `nr_threads++`|
|`sighand_struct` (`action[64]`)|**riêng**|**chung** — `count++`|
|`pid` — `gettid()`|**riêng**|**riêng** (khác leader)|
|`tgid` — `getpid()`|**riêng** (= pid nó)|**= tgid leader** (giống leader)|
|`exit_signal`|`SIGCHLD`|**−1**|
|kernel stack|**riêng** 16 KB|**riêng** 16 KB|
|`thread_struct` (cpu_context, FP/SIMD, TLS)|**riêng**|**riêng**|
|user stack|**riêng** (COW của cha)|**riêng** (glibc `mmap`, truyền đỉnh làm `newsp`)|
|`blocked` mask · `task->pending`|**riêng**|**riêng**|
|`real_parent` / `parent`|= task đã `fork`|= `real_parent` của leader (`CLONE_THREAD` → `p->real_parent = current->real_parent`)|
|`nsproxy` · `cred`|chia sẻ|chia sẻ|

**Luôn per-thread (kể cả pthread)**: `task_struct`, kernel stack, `pt_regs`, `thread_struct`, `pid` (TID), user stack, `blocked`, `task->pending`, `sched_entity`. **Chung hết còn lại** cho pthread.

---

## 3. Trace `pthread_create()` → `clone` syscall trên ARM64

### 3.1. glibc: `pthread_create` → `create_thread` (`nptl/pthread_create.c`)

```c
int __pthread_create_2_1(pthread_t *newthread, const pthread_attr_t *attr,
                         void *(*start_routine)(void *), void *arg)
{
    struct pthread *pd;
    // 1. cấp stack cho thread:
    allocate_stack(iattr, &pd, &stackaddr, &stacksize);
        // → mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_STACK, -1, 0)
        //   guard page ở đáy; struct pthread `pd` đặt Ở ĐỈNH vùng (pd chứa tid, TCB, cleanup, cancel state)
    pd->start_routine = start_routine;
    pd->arg = arg;
    pd->stackblock = stackaddr;

    // 2. tạo thread:
    create_thread(pd, iattr, &stopped_start, stackaddr, stacksize, &thread_ran);
}
```

`create_thread()`:

```c
const int clone_flags = (CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SYSVSEM
                         | CLONE_SIGHAND | CLONE_THREAD
                         | CLONE_SETTLS | CLONE_PARENT_SETTID | CLONE_CHILD_CLEARTID);

TLS_DEFINE_INIT_TP(tp, pd);   // tp = con trỏ TCB cho thread mới (aarch64: pd + TLS_TCB_OFFSET)

if (__clone_internal(&args, __clone_start_routine, pd) == -1)
    return errno;
// args: fn = start_thread, stack = stackaddr + stacksize (đỉnh),
//       flags = clone_flags, arg = pd,
//       parent_tid = &pd->tid, child_tid = &pd->tid, tls = tp
```

### 3.2. glibc: `__clone` (`sysdeps/unix/sysv/linux/aarch64/clone.S`)

Nguyên mẫu C: `int __clone(int (*fn)(void*), void *stack, int flags, void *arg, pid_t *ptid, void *tls, pid_t *ctid)` → vào với `x0=fn, x1=stack, x2=flags, x3=arg, x4=ptid, x5=tls, x6=ctid`.

```asm
__clone:
    cbz     x0, .Leinval             // fn == NULL → -EINVAL
    cbz     x1, .Leinval             // stack == NULL → -EINVAL

    stp     x0, x3, [x1, #-16]!      // đẩy {fn, arg} lên đỉnh stack con; x1 -= 16

    /* sắp xếp thanh ghi về ABI kernel: clone(flags, newsp, ptid, tls, ctid) */
    mov     x0, x2                   // x0 = flags
    /* x1 đã = stack con (đã trừ 16) */
    mov     x2, x4                   // x2 = ptid
    mov     x3, x5                   // x3 = tls
    mov     x4, x6                   // x4 = ctid

    mov     x8, #__NR_clone          // 220
    svc     #0                       // ◀── SYSCALL

    cbz     x0, .Lthread_start       // x0 == 0 → ta là CON
    ret                             // x0 != 0 → CHA: trả về TID con (hoặc -errno)

.Lthread_start:
    ldp     x1, x0, [sp], #16        // pop {fn, arg} → x1 = fn, x0 = arg
    blr     x1                       // fn(arg)   (= __clone_start_routine → start_thread → start_routine)
    mov     x8, #__NR_exit           // 93
    svc     #0                       // _exit(giá trị fn trả về)
```

**Thanh ghi tại `svc #0`**:

```
x0 = CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SYSVSEM|CLONE_SIGHAND|CLONE_THREAD
     |CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID   (= 0x3D0F00)
x1 = đỉnh stack thread mới − 16  (đã có {fn, arg})
x2 = &pd->tid                     (parent_tidptr)
x3 = tp (con trỏ TCB)             (tls)
x4 = &pd->tid                     (child_tidptr)
x8 = 220
```

### 3.3. Kernel: `svc` → `el0_svc` → `__arm64_sys_clone` → `copy_process`

`SYSCALL_DEFINE5(clone)` dựng `kernel_clone_args`:

```
flags       = 0x3D0F00 & ~CSIGNAL = 0x3D0F00   (không có SIGCHLD trong byte thấp)
exit_signal = 0x3D0F00 & CSIGNAL = 0
stack       = x1  (đỉnh stack thread mới)
tls         = x3  (TCB)
parent_tid  = x2 ; child_tid = x4
```

→ `_do_fork(&args)` → `copy_process(NULL, trace, NUMA_NO_NODE, &args)`:

|`copy_*`|Với các cờ pthread|Kết quả|
|---|---|---|
|`dup_task_struct(current)`|—|**`task_struct` mới + kernel stack 16 KB mới** (luôn mới, kể cả thread)|
|`copy_creds`|không `CLONE_NEWUSER`|`get_cred(current->cred)` — chia sẻ|
|`copy_semundo`|`CLONE_SYSVSEM`|`undo_list->refcnt++`|
|`copy_files`|`CLONE_FILES`|`atomic_inc(&oldf->count)` — **chung**|
|`copy_fs`|`CLONE_FS`|`fs->users++` — **chung**|
|`copy_sighand`|`CLONE_SIGHAND`|`refcount_inc(&current->sighand->count)` — **chung**|
|`copy_signal`|`CLONE_THREAD`|`return 0` — dùng `signal_struct` của leader, **chung**|
|`copy_mm`|`CLONE_VM`|`mmget(oldmm)`, `mm_users++` — **chung**|
|`copy_namespaces`|không `CLONE_NEW*`|`get_nsproxy(old_ns)` — chia sẻ|
|`copy_thread(flags, args->stack, 0, p, args->tls)`|ARM64|(dưới)|
|`alloc_pid(pid_ns_for_children)`|—|**`struct pid` mới + `pid` mới (= TID)**; `attach_pid(p, PIDTYPE_PID)`|

**`copy_thread` (ARM64)** — mấu chốt của "thread có stack + TLS riêng":

```c
struct pt_regs *childregs = task_pt_regs(p);          // đỉnh kernel stack MỚI
*childregs = *current_pt_regs();                       // copy pt_regs của caller
childregs->regs[0] = 0;                                // con thấy clone() trả về 0
*task_user_tls(p) = read_sysreg(tpidr_el0);            // (mặc định thừa TLS caller)
if (stack_start)                                       // = args->stack = đỉnh stack thread mới
    childregs->sp = stack_start;                       // ◀── SP user của thread = stack MỚI
if (clone_flags & CLONE_SETTLS)
    p->thread.uw.tp_value = tls;                       // ◀── TLS của thread = TCB MỚI (args->tls)
p->thread.cpu_context.pc = (unsigned long)ret_from_fork;
p->thread.cpu_context.sp = (unsigned long)childregs;
```

**Giai đoạn cuối `copy_process` — vì `CLONE_THREAD`**:

```c
p->pid = pid_nr(pid);                                  // TID mới
p->group_leader = current->group_leader;               // ◀── cùng leader
p->tgid         = current->tgid;                       // ◀── cùng TGID → getpid() giống
p->exit_signal  = -1;                                  // ◀── chết không gửi SIGCHLD
...
current->signal->nr_threads++;
atomic_inc(&current->signal->live);
refcount_inc(&current->signal->sigcnt);
list_add_tail_rcu(&p->thread_group, &p->group_leader->thread_group);
list_add_tail_rcu(&p->thread_node,  &p->signal->thread_head);
// KHÔNG attach_pid(PIDTYPE_TGID/PGID/SID) — chỉ group_leader nằm trong các hash đó.
// Thread dùng chung TGID/PGID/SID của leader.

if (clone_flags & CLONE_PARENT_SETTID)
    put_user(nr, args->parent_tid);                    // ◀── glibc pd->tid = TID

p->clear_child_tid = (clone_flags & CLONE_CHILD_CLEARTID) ? args->child_tid : NULL;
// ◀── khi thread chết: mm_release() → put_user(0, clear_child_tid); futex_wake(clear_child_tid)
//     → đây là cái pthread_join() chờ trên
```

### 3.4. `_do_fork` return + thread chạy

`_do_fork`: `wake_up_new_task(p)` (đưa vào runqueue CFS), return `nr` (TID).

- **Cha** (`x0 = nr` sau syscall): `__clone.S` thấy `x0 != 0` → `ret` → glibc `pthread_create` trả về 0, `*newthread = pd`.
- **Thread mới**: scheduler chọn nó → `cpu_switch_to` nạp `cpu_context` (`sp = childregs`, `pc = ret_from_fork`) → `ret_from_fork` → `schedule_tail` → `ret_to_user` → `kernel_exit` nạp `childregs` (`x0 = 0`, `sp = stack thread mới`) → `eret` → EL0.
    - `TLS`: `__switch_to` gọi `tls_thread_switch` → nạp `p->thread.uw.tp_value` (= TCB mới) vào `TPIDR_EL0`.
    - EL0: PC = `&svc + 4` trong `__clone.S`, `x0 = 0` → nhánh `.Lthread_start`: `ldp {fn, arg}` từ stack thread → `blr fn` → `__clone_start_routine` → `start_thread(pd)` → `pd->start_routine(pd->arg)`.
    - Khi `start_routine` return → `__clone.S`: `svc __NR_exit` → `do_exit` → `mm_release` ghi 0 vào `pd->tid` + `futex_wake` → `pthread_join` thức dậy.

---

## 4. `clone3()` (bổ sung — có ở 5.4)

5.3+ có `clone3(struct clone_args *, size_t)` — struct mở rộng thay vì 5 tham số cứng:

```c
struct clone_args {
    __aligned_u64 flags;         // gồm cả CLONE_INTO_CGROUP, CLONE_CLEAR_SIGHAND (5.5)
    __aligned_u64 pidfd;
    __aligned_u64 child_tid;
    __aligned_u64 parent_tid;
    __aligned_u64 exit_signal;   // tách khỏi flags (không còn nhồi vào byte thấp)
    __aligned_u64 stack;
    __aligned_u64 stack_size;    // ◀── truyền kích thước, kernel tự tính đỉnh
    __aligned_u64 tls;
    __aligned_u64 set_tid;       // (5.5) chọn PID cho con
    __aligned_u64 set_tid_size;
    __aligned_u64 cgroup;
};
```

glibc từ 2.34 dùng `clone3` cho `pthread_create` khi kernel hỗ trợ (fallback `clone`). Nội bộ vẫn qua `kernel_clone(&args)` chung.

---

## 5. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|Entry|`_do_fork(struct kernel_clone_args *)`|`kernel_clone()` — 5.10|
|`clone3`|có (5.3), `set_tid` chưa|`set_tid`/`set_tid_size` (5.5), `CLONE_INTO_CGROUP` (5.7)|
|`copy_thread` arm64|`copy_thread_tls(flags, sp, sz, p, tls)`|`copy_thread(..., args)` — 5.10|
|glibc `pthread_create` syscall|`clone`|`clone3` (glibc 2.34+, fallback `clone`)|
|Cờ pthread|`CLONE_VM\|FS\|FILES\|SYSVSEM\|SIGHAND\|THREAD\|SETTLS\|PARENT_SETTID\|CHILD_CLEARTID`|y hệt|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `kernel_clone`, `copy_thread(..., args)`. Cơ chế "mọi thread là `task_struct`", ràng buộc `CLONE_THREAD⇒CLONE_SIGHAND⇒CLONE_VM`, bảng chia sẻ, đường `pthread_create`→`clone`→`copy_process` — **giống hệt 5.4**.

---

✅ **MODULE 19 hoàn tất.** Sơ đồ: [docs/diagrams/thread-and-clone-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/thread-and-clone-arm64.md).

**Kế tiếp: MODULE 20 — PID / TGID** (`struct pid`, `struct pid_namespace`, `upid`, `PIDTYPE_PID/TGID/PGID/SID`; phân biệt PID / TGID / TID; vì sao `getpid()` trong process đa luồng trả `tgid` chứ không phải thread id). Nói "tiếp" để làm Module 20.