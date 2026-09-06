# MODULE 22 — Kernel Thread trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/kernel-thread-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/kernel-thread-arm64.md) (4 sơ đồ: dây chuyền tạo, `copy_thread` user vs kthread, `ret_from_fork` ngã rẽ, vì sao không có user AS).

**File nguồn**: `kernel/kthread.c` (`kthread_create_on_node`, `kthreadd`, `create_kthread`, `kthread` wrapper, `kthread_should_stop`, `kthread_stop`), `kernel/fork.c` (`kernel_thread`, `copy_mm`), `arch/arm64/kernel/process.c` (`copy_thread` — nhánh `PF_KTHREAD`), `arch/arm64/kernel/entry.S` (`ret_from_fork`), `kernel/sched/core.c` (`context_switch` — nhánh `!next->mm`), `init/main.c` (`rest_init` — tạo PID 1 và PID 2).

---

## 0. Ba câu trả lời

**`ret_from_fork` phân biệt user task vs kernel thread thế nào?** Bằng `cbz x19` — `copy_thread` để `cpu_context.x19 = 0` cho user task (do `memset`), `= fn` (khác 0) cho kernel thread. `x19 == 0` → `b ret_to_user` → `eret` về EL0. `x19 != 0` → `mov x0, x20; blr x19` → gọi `fn(arg)` ở EL1.

**User process vs kernel thread ở `ret_from_fork`:**

- User: `ret_from_fork` → `schedule_tail` → `ret_to_user` → `kernel_exit 0` khôi phục `pt_regs` (bản sao của cha) → `eret` → **EL0**, tiếp tục lệnh sau `clone()`, `x0 = 0`.
- Kernel thread: `ret_from_fork` → `schedule_tail` → `blr x19` → chạy `fn(arg)` **ở EL1**, không `eret`, không bao giờ về EL0.

**Vì sao kernel thread không có user address space?** Vì (a) nó chỉ chạy code nhân — không có user code/data/stack để cần; (b) `copy_mm` để `mm == NULL` vì người tạo (`kthreadd`) cũng có `mm == NULL`; (c) khi lập lịch, `context_switch` cho kthread **mượn `active_mm`** của task trước mà **không** đổi TTBR0 (lazy-TLB) — kthread không bao giờ deref địa chỉ user nên ánh xạ mượn không quan trọng, chỉ để MMU có gì đó trong TTBR0.

---

## 1. Nguồn gốc: PID 1 và PID 2 sinh ra lúc boot

`init/main.c` → `start_kernel` → `arch_call_rest_init` → `rest_init`:

```c
static noinline void __ref rest_init(void)
{
    ...
    pid = kernel_thread(kernel_init, NULL, CLONE_FS);        // → PID 1  (init)
    ...
    pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);  // → PID 2  (kthreadd)
    rcu_read_lock();
    kthreadd_task = find_task_by_pid_ns(pid, &init_pid_ns);
    rcu_read_unlock();
    ...
    cpu_startup_entry(CPUHP_ONLINE);   // PID 0 (swapper/init_task) trở thành idle task
}
```

`current` ở đây = **`init_task` (PID 0, swapper)** — nó có `flags = PF_KTHREAD` và `mm == NULL` (`active_mm = &init_mm`). Nên cả hai `kernel_thread` tạo ra task với `PF_KTHREAD` + `mm == NULL`.

- **PID 1 (`kernel_init`)**: chạy `kernel_init()` → hoàn tất init nhân → `kernel_execve("/sbin/init")` → **`execve` cấp cho nó một `mm_struct`** → từ đó là process thường (có user AS). Không còn là kernel thread.
- **PID 2 (`kthreadd`)**: chạy `kthreadd()` vòng lặp vĩnh viễn ở EL1, `mm == NULL` mãi. Nó là **cha của mọi kernel thread khác**.

---

## 2. `kthread_create()` — driver xếp yêu cầu, `kthreadd` tạo hộ

Driver (chạy trong ngữ cảnh process của **nó**) gọi:

```c
struct task_struct *k = kthread_create(threadfn, data, "myworker/%d", cpu);
// hoặc kthread_run(threadfn, data, "...") = kthread_create + wake_up_process
wake_up_process(k);   // bắt đầu chạy threadfn
```

`kthread_create` → `kthread_create_on_node` → `__kthread_create_on_node`:

```c
struct kthread_create_info *create = kmalloc(sizeof(*create), GFP_KERNEL);
create->threadfn = threadfn;         // hàm THẬT của driver
create->data     = data;
create->node     = node;
create->done     = &done;

spin_lock(&kthread_create_lock);
list_add_tail(&create->list, &kthread_create_list);   // ◀── xếp vào hàng đợi
spin_unlock(&kthread_create_lock);

wake_up_process(kthreadd_task);       // ◀── đánh thức kthreadd
wait_for_completion_killable(&done->completion);   // ◀── BLOCK tới khi kthreadd báo về
task = create->result;
return task;
```

**Vì sao đi vòng qua `kthreadd`, không tự `kernel_thread`?**

1. `real_parent` của kthread mới = **`kthreadd`**, không phải driver. Khi driver (process) exit, kthread **không** bị reparent/ảnh hưởng.
2. Thừa `PF_KTHREAD` + `mm == NULL` từ `kthreadd` (nếu tạo từ driver có `mm`, `copy_mm` với `CLONE_VM` sẽ _chia sẻ_ user AS của driver — sai).
3. `kthreadd` chạy với đầy đủ quyền nhân, không có ràng buộc credential của driver.

---

## 3. `kthreadd()` daemon (`kernel/kthread.c`)

```c
int kthreadd(void *unused)
{
    struct task_struct *tsk = current;
    set_task_comm(tsk, "kthreadd");
    ignore_signals(tsk);
    set_cpus_allowed_ptr(tsk, housekeeping_cpumask(HK_FLAG_KTHREAD));
    set_mems_allowed(node_states[N_MEMORY]);

    current->flags |= PF_NOFREEZE;
    cgroup_init_kthreadd();

    for (;;) {
        set_current_state(TASK_INTERRUPTIBLE);
        if (list_empty(&kthread_create_list))
            schedule();                          // ◀── NGỦ khi không có việc
        __set_current_state(TASK_RUNNING);

        spin_lock(&kthread_create_lock);
        while (!list_empty(&kthread_create_list)) {
            struct kthread_create_info *create;
            create = list_entry(kthread_create_list.next, struct kthread_create_info, list);
            list_del_init(&create->list);
            spin_unlock(&kthread_create_lock);

            create_kthread(create);              // ◀── tạo TỪNG kthread, trong ngữ cảnh kthreadd

            spin_lock(&kthread_create_lock);
        }
        spin_unlock(&kthread_create_lock);
    }
    return 0;
}
```

`create_kthread(create)`:

```c
static void create_kthread(struct kthread_create_info *create)
{
    int pid;
    ...
    pid = kernel_thread(kthread, create, CLONE_FS | CLONE_FILES | SIGCHLD);
    //                   ^^^^^^  ^^^^^^
    //                   fn = kthread (WRAPPER), KHÔNG phải threadfn
    //                           arg = create
    if (pid < 0) {
        create->result = ERR_PTR(pid);
        complete(&create->done->completion);
    }
}
```

---

## 4. `kernel_thread()` (`kernel/fork.c`)

```c
pid_t kernel_thread(int (*fn)(void *), void *arg, unsigned long flags)
{
    struct kernel_clone_args args = {
        .flags       = (lower_32_bits(flags) | CLONE_VM | CLONE_UNTRACED) & ~CSIGNAL,
        .exit_signal = lower_32_bits(flags) & CSIGNAL,
        .stack       = (unsigned long)fn,      // ◀── OVERLOAD: 'stack' chở fn
        .stack_size  = (unsigned long)arg,     // ◀── OVERLOAD: 'stack_size' chở arg
    };
    return kernel_clone(&args);   // 5.4: _do_fork(&args)
}
```

- **`CLONE_VM`** — không `dup_mm`. (Với `kthreadd` thì `current->mm == NULL` nên `dup_mm` không xảy ra dù sao, nhưng ngữ nghĩa là "không có user AS".)
- **`CLONE_UNTRACED`** — dù người tạo bị `ptrace` (không bao giờ), không ép `CLONE_PTRACE` lên con.
- **`stack`/`stack_size` bị mượn nghĩa** — với kernel thread, chúng **không** là "user stack + kích thước" mà là `fn` + `arg`. `copy_thread` biết điều này nhờ `PF_KTHREAD`.

---

## 5. `copy_process` → `copy_mm` để `mm == NULL`

```c
static int copy_mm(unsigned long clone_flags, struct task_struct *tsk)
{
    tsk->mm = NULL;
    tsk->active_mm = NULL;

    oldmm = current->mm;         // current = kthreadd
    if (!oldmm)
        return 0;                // ◀── kthreadd->mm == NULL → RETURN NGAY
                                 //     → tsk->mm = tsk->active_mm = NULL

    if (clone_flags & CLONE_VM) { mmget(oldmm); mm = oldmm; goto good_mm; }
    mm = dup_mm(tsk, current->mm);
    ...
}
```

Kiểm `if (!oldmm) return 0` **trước** kiểm `CLONE_VM` — nên kernel thread luôn kết thúc với `mm == NULL, active_mm == NULL`.

`PF_KTHREAD` được **thừa kế** từ `kthreadd` qua `arch_dup_task_struct` (`*dst = *src` copy cả `flags`). `copy_process` không xóa cờ này (nó chỉ xóa `PF_SUPERPRIV | PF_WQ_WORKER | PF_IDLE`). (5.10+ đặt tường minh `p->flags |= PF_KTHREAD` khi `args->kthread`.)

---

## 6. `copy_thread()` — nhánh `PF_KTHREAD` (`arch/arm64/kernel/process.c`)

```c
int copy_thread(unsigned long clone_flags, unsigned long stack_start,
                unsigned long stk_sz, struct task_struct *p, unsigned long tls)
{
    struct pt_regs *childregs = task_pt_regs(p);
    memset(&p->thread.cpu_context, 0, sizeof(struct cpu_context));

    if (likely(!(p->flags & PF_KTHREAD))) {
        // ── user task ── (Module 8)
        *childregs = *current_pt_regs();
        childregs->regs[0] = 0;
        ...
    } else {
        // ── KERNEL THREAD ──
        memset(childregs, 0, sizeof(struct pt_regs));   // ◀── pt_regs RỖNG (không có EL0 để về)
        childregs->pstate = PSR_MODE_EL1h;              // (nếu lỡ có eret → ở EL1)
        spectre_v4_enable_task_mitigation(p);
        if (system_uses_irq_prio_masking())
            childregs->pmr_save = GIC_PRIO_IRQON;
        p->thread.cpu_context.x19 = stack_start;        // ◀── = fn
        p->thread.cpu_context.x20 = stk_sz;             // ◀── = arg
    }
    p->thread.cpu_context.pc = (unsigned long)ret_from_fork;   // giống user
    p->thread.cpu_context.sp = (unsigned long)childregs;
    return 0;
}
```

|Trường|User task|Kernel thread|
|---|---|---|
|`pt_regs` (đỉnh kernel stack)|bản sao `pt_regs` của cha|**memset 0**|
|`cpu_context.x19`|0 (memset)|**`fn`**|
|`cpu_context.x20`|0|**`arg`**|
|`cpu_context.pc`|`ret_from_fork`|`ret_from_fork`|
|`cpu_context.sp`|`childregs`|`childregs`|

---

## 7. Lần đầu scheduler chọn kthread → `ret_from_fork`

`context_switch(rq, prev, next = kthread)` (`kernel/sched/core.c`):

```c
if (!next->mm) {                              // ◀── kthread: mm == NULL
    enter_lazy_tlb(prev->active_mm, next);
    next->active_mm = prev->active_mm;        // ◀── MƯỢN active_mm của prev
    if (prev->mm)                             // prev là user task
        mmgrab(prev->active_mm);              //   giữ mm khỏi bị free
    else
        prev->active_mm = NULL;
    // → switch_mm() KHÔNG được gọi → TTBR0_EL1 + ASID GIỮ NGUYÊN (lazy TLB) → 0 chi phí MMU
}
...
switch_to(prev, next, prev);                  // → __switch_to → cpu_switch_to
```

`cpu_switch_to(prev, kthread)`:

- lưu `cpu_context` của prev
- nạp của kthread: `x19 = fn`, `x20 = arg`, `x21..x28 = 0`, `fp = 0`, `sp = childregs` (đỉnh kernel stack kthread), `lr = ret_from_fork`
- `mov sp, x9` → sp = kernel stack kthread
- `msr sp_el0, x1` → `current = kthread`
- `ret` → nhảy tới `lr = ret_from_fork`

`ret_from_fork` (`arch/arm64/kernel/entry.S`):

```asm
SYM_CODE_START(ret_from_fork)
    bl   schedule_tail          // finish_task_switch(prev): nhả rq->lock, preempt_count → 0
    cbz  x19, 1f                // x19 = fn ≠ 0 → KHÔNG rẽ
    mov  x0, x20               // x0 = arg
    blr  x19                   // ◀── gọi fn(arg) = kthread(create)  — Ở EL1
1:  get_current_task tsk       // (chỉ user task tới đây)
    b    ret_to_user
SYM_CODE_END(ret_from_fork)
```

`schedule_tail` (`kernel/sched/core.c`) chạy **trước** cho mọi task mới — hoàn tất context switch mà task khác đã bắt đầu (Module 9). Rồi `cbz x19` rẽ.

---

## 8. `kthread()` wrapper — sleep, report, chạy `threadfn`

```c
static int kthread(void *_create)          // gọi bởi ret_from_fork's blr x19, x0 = create
{
    struct kthread_create_info *create = _create;
    int (*threadfn)(void *) = create->threadfn;   // hàm THẬT của driver
    void *data = create->data;
    struct kthread *self;

    self = kzalloc(sizeof(*self), GFP_KERNEL);
    set_kthread_struct(self);                // to_kthread(current) từ giờ trả 'self'
    self->threadfn = threadfn;
    self->data     = data;
    init_completion(&self->exited);
    current->vfork_done = &self->exited;     // kthread_stop() chờ ở đây

    __set_current_state(TASK_UNINTERRUPTIBLE);
    create->result = current;                // ◀── báo task_struct về cho kthread_create()
    preempt_disable();
    complete(done);                          // ◀── ĐÁNH THỨC wait_for_completion() của driver
    schedule_preempt_disabled();             // ◀── NGỦ — driver phải wake_up_process() để bắt đầu
    preempt_enable();

    ret = -EINTR;
    if (!test_bit(KTHREAD_SHOULD_STOP, &self->flags)) {
        cgroup_kthread_ready();
        __kthread_parkme(self);
        ret = threadfn(data);               // ◀── CUỐI CÙNG chạy hàm của driver
    }
    do_exit(ret);                           // ◀── threadfn return → do_exit
}
```

Nên:

- `kthread_create()` trả về `task_struct` nhưng `threadfn` **chưa** chạy — thread đang ngủ `TASK_UNINTERRUPTIBLE`.
- Driver gọi `wake_up_process(k)` (hoặc dùng `kthread_run` = tạo + wake) → `schedule_preempt_disabled()` trả về → chạy `threadfn(data)`.

### `threadfn` điển hình

```c
static int my_worker(void *data)
{
    while (!kthread_should_stop()) {
        // ... làm việc ...
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(HZ);               // ngủ 1 giây
    }
    return 0;
}
```

Chạy **hoàn toàn ở EL1**, trên kernel stack 16 KB của kthread, `mm == NULL`, dùng `active_mm` mượn. `kthread_should_stop()` = `test_bit(KTHREAD_SHOULD_STOP, &to_kthread(current)->flags)`.

### Kết thúc

- `threadfn` return → `kthread()` → `do_exit(ret)`.
- `kthread_stop(k)`: `set_bit(KTHREAD_SHOULD_STOP, ...)`, `wake_up_process(k)`, `wait_for_completion(&kthread->exited)`, trả `ret` của threadfn.

---

## 9. So sánh EL0-return vs kernel-function

||User process|Kernel thread|
|---|---|---|
|Sau `ret_from_fork` + `schedule_tail`|`cbz x19` (x19==0) → `b ret_to_user`|`cbz x19` (x19≠0) → `mov x0,x20; blr x19`|
|`ret_to_user` → `kernel_exit 0`|khôi phục `pt_regs` (bản sao cha, `x0=0`) → `eret`|_(không tới đây)_|
|Điểm hạ cánh cuối|**EL0**, lệnh sau `clone()`, `x0 = 0`|`fn(arg)` ở **EL1**|
|`mm`|user `mm_struct`; `TTBR0 = pgd của nó`|**`NULL`**; `active_mm` mượn; `TTBR0` giữ nguyên|
|`pt_regs` đỉnh stack|ngữ cảnh EL0 hợp lệ|memset 0, không dùng|
|Về userspace|có|**không bao giờ**|
|Bị `ptrace`|được|không (`CLONE_UNTRACED`)|
|Nhận signal thường|có|thường `ignore_signals()`; chỉ xử lý cái mình chọn|
|`ps`|`program`|`[kthreadd]`, `[ksoftirqd/0]`, `[kworker/1:2-events]`|

---

## 10. Vì sao kernel thread không cần user address space

1. **Việc của nó toàn là code nhân.** `kswapd` (thu hồi bộ nhớ), `kcompactd` (nén bộ nhớ), `pdflush`/`kworker` (ghi trang bẩn), `ksoftirqd/N` (chạy softirq bị dồn), `kblockd` (I/O khối), `rcu_sched`/`rcuos/N` (RCU callback), `migration/N` (di chuyển task giữa CPU), `watchdog/N` (lockup detector). Tất cả nằm ở **kernel VA** — nửa `TTBR1_EL1` (`PAGE_OFFSET` trở lên: linear map, vmalloc, code nhân). Nửa này **chung mọi task**, luôn được map. Không có user code, user data, user stack, `argv`/`envp`.
    
2. **`mm == NULL` là dấu hiệu.** `copy_mm` để nó NULL vì `kthreadd->mm == NULL`. Khắp nhân có `if (current->mm)` để phân biệt "đang chạy thay mặt một process" với "kernel thread". `get_user`/`put_user` vô nghĩa với kthread; `copy_from_user` từ kthread tới địa chỉ TTBR0 sẽ fault hoặc đọc rác.
    
3. **Lazy-TLB — `active_mm`.** MMU luôn cần _một_ bảng trang trong `TTBR0_EL1` (dù không dùng). Khi switch sang kthread, `context_switch` đặt `next->active_mm = prev->active_mm` và `mmgrab` — kthread **mượn** user AS của task trước. `switch_mm` **không** được gọi → `TTBR0_EL1` + ASID **không đổi** → **0 chi phí MMU** cho lần switch. kthread không bao giờ deref địa chỉ user nên ánh xạ mượn không quan trọng. Khi kthread sau đó switch sang user task có `mm` khác → _lần switch đó_ mới gọi `switch_mm` và `mmdrop(rq->prev_mm)` nhả cái đã mượn.
    
4. **Chỉ nửa `TTBR1`.** Vì không đụng địa chỉ `TTBR0`, kthread thực chất sống hoàn toàn ở nửa kernel VA.
    
5. **Bonus an toàn.** Không có user AS → không bị lừa deref con trỏ do user điều khiển. `CLONE_UNTRACED` → debugger không `ptrace` được kthread.
    

---

## 11. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|Đặt `PF_KTHREAD`|thừa kế từ `kthreadd` qua `*dst=*src`|tường minh `p->flags|
|`kthread()` wrapper kết thúc|`do_exit(ret)`|`kthread_exit(ret)` (5.17)|
|`kernel_thread`|`_do_fork(&args)` với `stack=fn, stack_size=arg`|`kernel_clone` với `args.fn`/`args.fn_arg` tường minh (5.17)|
|`ret_from_fork`|`bl schedule_tail; cbz x19,1f; mov x0,x20; blr x19`|y hệt (5.4–5.17); 5.18 chuyển một phần sang C|
|kthread `mm`|`NULL`, mượn `active_mm`|y hệt|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `copy_thread` có nhánh `PF_KTHREAD | PF_IO_WORKER`; `kthread()` vẫn `do_exit`. Cơ chế `cpu_context.x19=fn` + `ret_from_fork` `cbz x19` + `mm==NULL` + lazy-TLB — **giống hệt 5.4**.