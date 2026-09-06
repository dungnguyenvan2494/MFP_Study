# MODULE 17 — Process Exit trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/exit-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/exit-arm64.md) (4 sơ đồ: dây chuyền `do_exit`, `exit_notify` zombie/autoreap, zombie giữ gì/free gì, đường `SIGCHLD`).

**File nguồn**: `kernel/exit.c` (`do_exit`, `exit_notify`, `forget_original_parent`, `release_task`, `__exit_signal`, `__unhash_process`, `exit_mm`, `exit_files`, `exit_fs`), `kernel/signal.c` (`do_notify_parent`, `exit_signals`, `retarget_shared_pending`), `kernel/sched/core.c` (`do_task_dead`, `finish_task_switch`), `arch/arm64/kernel/process.c` (`exit_thread`).

---

## 0. Ai gọi `do_exit`

|Nguồn|Đường|
|---|---|
|`exit(status)` (libc — dọn atexit, flush stdio)|→ `exit_group(status)` syscall → `do_group_exit((status & 0xff) << 8)` → `do_exit()` từng thread|
|`_exit(status)` / `syscall(SYS_exit)`|→ `sys_exit` → `do_exit((status & 0xff) << 8)` (chỉ thread gọi)|
|Tín hiệu chí mạng không handler (`SIGSEGV`, `SIGKILL`, `SIGTERM`...)|`get_signal` → `do_coredump` (nếu cần) → `do_group_exit(sig)` → `do_exit(sig \| 0x80 nếu core)`|
|Kernel oops trong ngữ cảnh process|`die()` → `do_exit(SIGSEGV)`|

**`exit_code` mã hoá** (trong `task_struct.exit_code`):

- Bit `& 0x7f` khác 0 → bị **tín hiệu** đó giết. Bit `& 0x80` → có **core dump**.
- Bit thấp = 0 → thoát bình thường, **status** = `exit_code >> 8`.

---

## 1. `do_exit(code)` — từng bước

```c
void __noreturn do_exit(long code)
{
    struct task_struct *tsk = current;
    int group_dead;

    if (in_interrupt()) panic("Aiee, killing interrupt handler!");   // KHÔNG exit được từ ngữ cảnh IRQ
    if (!tsk->pid)      panic("Attempted to kill the idle task!");    // pid 0 (swapper) không chết
    set_fs(USER_DS);                                                  // (5.4) reset addr_limit — chống clear_child_tid ghi bậy
    if (in_atomic()) preempt_count_set(PREEMPT_ENABLED);             // oops có thể để lại preempt_count

    if (tsk->flags & PF_EXITING) {                                  // fault đệ quy TRONG do_exit
        set_current_state(TASK_UNINTERRUPTIBLE); schedule();          // → treo vĩnh viễn, chờ reboot
    }

    exit_signals(tsk);                        // ① đặt PF_EXITING
    if (tsk->mm) sync_mm_rss(tsk->mm);

    group_dead = atomic_dec_and_test(&tsk->signal->live);           // thread CUỐI của thread group?
    if (group_dead) {
        if (is_global_init(tsk)) panic("Attempted to kill init!");   // giết PID 1 → panic
        hrtimer_cancel(&tsk->signal->real_timer);
        exit_itimers(tsk);                                           // hủy POSIX interval timers
    }

    tsk->exit_code = code;                    // ◀── LƯU cho cha đọc bằng wait()

    exit_mm();                                // ② nhả address space
    if (group_dead) exit_shm(tsk);
    exit_sem(tsk);
    exit_files(tsk);                          // ③ đóng mọi fd
    exit_fs(tsk);                             // ④ nhả root/pwd
    if (group_dead) disassociate_ctty(1);
    exit_task_namespaces(tsk);
    exit_task_work(tsk);                      // chạy task_work còn treo (fput trì hoãn, ...)
    exit_thread(tsk);                         // ⑤ arch: nhả SVE state

    perf_event_exit_task(tsk);
    sched_autogroup_exit_task(tsk);
    cgroup_exit(tsk);
    if (group_dead) kill_orphaned_pgrp(tsk->group_leader, NULL);

    tsk->exit_state = EXIT_ZOMBIE;            // đặt lần đầu (exit_notify có thể ghi đè)
    exit_notify(tsk, group_dead);             // ⑥ reparent con, báo cha, trở thành zombie

    futex_exit_release(tsk);
    if (tsk->io_context) exit_io_context(tsk);
    check_stack_usage();                      // cảnh báo nếu stack đã dùng quá sâu

    preempt_disable();
    tsk->state = TASK_DEAD;                   // ⑦
    tsk->flags |= PF_NOFREEZE;
    do_task_dead();                           // → __schedule() — KHÔNG BAO GIỜ TRẢ VỀ
}
```

### ① `exit_signals(tsk)`

Giữ `siglock`: `retarget_shared_pending()` — dời các tín hiệu trong `shared_pending` cho thread khác còn sống trong nhóm (để tín hiệu gửi tới process không bị mất khi thread này chết). `tsk->flags |= PF_EXITING`. `clear TIF_SIGPENDING`. → Từ giờ `wants_signal()` bỏ qua task này; nó không nhận/xử lý tín hiệu mới nữa.

### ② `exit_mm()` — **address space biến mất**

```c
mm = tsk->mm;
if (!mm) return;                              // kernel thread
mm_release(tsk, mm):
    if (tsk->clear_child_tid) {
        put_user(0, tsk->clear_child_tid);
        futex_wake(tsk->clear_child_tid, 1);  // ◀── ĐÁNH THỨC pthread_join()
    }
    if (tsk->vfork_done) complete_vfork_done(tsk);   // vfork: nhả cha đang chờ
task_lock(tsk);
tsk->mm = NULL;                               // task giờ mm == NULL, chạy trên active_mm (lazy)
enter_lazy_tlb(mm, tsk);
task_unlock(tsk);
mmput(mm):
    if (atomic_dec_and_test(&mm->mm_users)) {
        exit_mmap(mm);                        // unmap mọi VMA, free bảng trang, thả mọi trang
        mmdrop(mm) → __mmdrop → free pgd + mm_struct
    }
```

Nếu là thread cùng process với thread khác còn sống (`mm_users > 1`) thì chỉ giảm đếm — address space còn cho các thread kia.

### ③ `exit_files(tsk)`

`tsk->files = NULL`; `put_files_struct(files)`: nếu `atomic_dec_and_test(&files->count)` → `close_files()` → `filp_close()` mọi fd đang mở → `fput()` mỗi `struct file` → ref cuối → `__fput` (flush, nhả inode). Nếu `CLONE_FILES` (thread) → chỉ giảm đếm.

### ④ `exit_fs(tsk)`

`tsk->fs = NULL`; `fs->users--`; nếu 0 → `free_fs_struct` → `path_put(&fs->root)` + `path_put(&fs->pwd)` (nhả ref dentry/vfsmount của cwd và root).

### ⑤ `exit_thread(tsk)` — ARM64 (`arch/arm64/kernel/process.c`)

```c
void exit_thread(struct task_struct *tsk)
{
    fpsimd_release_task(tsk);   // kfree(tsk->thread.sve_state)
}
```

Rất nhẹ trên arm64. (Hw breakpoint được dọn ở `ptrace_release_task` / `flush_thread`.)

### ⑦ `do_task_dead()` (`kernel/sched/core.c`)

```c
void __noreturn do_task_dead(void)
{
    set_special_state(TASK_DEAD);
    smp_mb();
    raw_spin_lock_irq(&current->pi_lock);
    __schedule(false);          // chọn task khác, switch away — task này KHÔNG chạy lại
    BUG();
}
```

`__schedule` → `context_switch(rq, dead_task, next)` → `cpu_switch_to(dead_task, next)` chạy **trên kernel stack của dead_task lần cuối cùng**. Rồi trong ngữ cảnh `next`:

```c
finish_task_switch(prev = dead_task):
    prev_state = prev->state;   // == TASK_DEAD
    ...
    if (prev_state == TASK_DEAD) {
        put_task_stack(prev);            // ◀── FREE kernel stack 16 KB Ở ĐÂY (task kế tiếp làm)
        put_task_struct_rcu_user(prev);  // bỏ tham chiếu "scheduler"
    }
```

Vậy: **kernel stack được giải phóng sớm** — bởi task chạy ngay sau task chết, trong `finish_task_switch`. `task_struct` thì chưa (cha còn giữ 1 ref qua `children` list).

---

## 2. `exit_notify(tsk, group_dead)` — trở thành zombie

```c
write_lock_irq(&tasklist_lock);

forget_original_parent(tsk, &dead);          // (a) REPARENT con của tsk
    // reaper = find_child_reaper(tsk):
    //   subreaper gần nhất (task tổ tiên đặt prctl(PR_SET_CHILD_SUBREAPER)),
    //   hoặc child_reaper của pid namespace = init (PID 1)
    // for each child c of tsk:
    //     c->real_parent = c->parent = reaper;
    //     list_move_tail(&c->sibling, &reaper->children);
    //     if (c->exit_state == EXIT_ZOMBIE)                       // con đã là zombie
    //         if (do_notify_parent(c, c->exit_signal)) list_add(&c->ptrace_entry, &dead);

if (group_dead)
    kill_orphaned_pgrp(tsk->group_leader, NULL);   // POSIX: process group mồ côi → SIGHUP + SIGCONT

tsk->exit_state = EXIT_ZOMBIE;                // (b) ◀── GIỜ LÀ ZOMBIE

if (tsk->ptrace) {
    int sig = (thread_group_leader(tsk) && thread_group_empty(tsk) && !ptrace_reparented(tsk))
              ? tsk->exit_signal : SIGCHLD;
    autoreap = do_notify_parent(tsk, sig);   // báo cho tracer
} else if (thread_group_leader(tsk)) {
    autoreap = thread_group_empty(tsk) &&
               do_notify_parent(tsk, tsk->exit_signal);   // (c) ◀── báo cho CHA THẬT
} else {
    autoreap = true;                         // thread non-leader → không ai wait() → tự reap
}

if (autoreap) {
    tsk->exit_state = EXIT_DEAD;              // (d) bỏ qua hẳn giai đoạn zombie
    list_add(&tsk->ptrace_entry, &dead);
}

if (tsk->signal->notify_count < 0)           // de_thread() đang chờ group leader
    wake_up_process(tsk->signal->group_exit_task);
write_unlock_irq(&tasklist_lock);

list_for_each_entry_safe(p, n, &dead, ptrace_entry)
    release_task(p);                          // reap ngay những cái autoreap-able
```

`autoreap == true` khi:

- `tsk` là thread **non-leader** (`exit_signal == -1`) — không ai `wait()` cho từng thread.
- Cha đặt `SIGCHLD` = `SIG_IGN` hoặc `SA_NOCLDWAIT` (`do_notify_parent` trả `true`).

→ Khi đó `EXIT_DEAD` + `release_task` ngay, **không có zombie**.

Ngược lại → `EXIT_ZOMBIE`, chờ cha `wait()`.

---

## 3. `do_notify_parent()` — CHA nhận SIGCHLD thế nào

`kernel/signal.c`, gọi từ `exit_notify` (giữ `tasklist_lock`):

```c
bool do_notify_parent(struct task_struct *tsk, int sig)
{
    struct kernel_siginfo info;
    struct sighand_struct *psig;
    bool autoreap = false;

    do_notify_pidfd(tsk);                     // đánh thức mọi ai poll pidfd của tsk

    // dựng siginfo — TRONG namespace của CHA:
    info.si_signo = sig;                      // = tsk->exit_signal (thường SIGCHLD)
    info.si_pid   = task_pid_nr_ns(tsk, task_active_pid_ns(tsk->parent));  // ◀ PID con trong pid-ns CỦA CHA
    info.si_uid   = from_kuid_munged(task_cred_xxx(tsk->parent, user_ns), task_uid(tsk));
    task_cputime(tsk, &utime, &stime);
    info.si_utime = nsec_to_clock_t(utime + tsk->signal->utime);
    info.si_stime = nsec_to_clock_t(stime + tsk->signal->stime);

    // giải mã exit_code:
    if (tsk->exit_code & 0x80)      { info.si_code = CLD_DUMPED; info.si_status = tsk->exit_code & 0x7f; }
    else if (tsk->exit_code & 0x7f) { info.si_code = CLD_KILLED; info.si_status = tsk->exit_code & 0x7f; }
    else                           { info.si_code = CLD_EXITED; info.si_status = tsk->exit_code >> 8; }

    psig = tsk->parent->sighand;
    spin_lock_irqsave(&psig->siglock, flags);
    if (!tsk->ptrace && sig == SIGCHLD &&
        (psig->action[SIGCHLD-1].sa.sa_handler == SIG_IGN ||
         (psig->action[SIGCHLD-1].sa.sa_flags & SA_NOCLDWAIT))) {
        autoreap = true;                                          // ◀── cha không quan tâm → reap ngay
        if (psig->action[SIGCHLD-1].sa.sa_handler == SIG_IGN)
            sig = 0;                                              //     và không gửi tín hiệu gì
    }
    if (valid_signal(sig) && sig)
        __send_signal(sig, &info, tsk->parent, PIDTYPE_TGID, false);  // ◀── SIGCHLD → shared_pending của CHA
    __wake_up_parent(tsk, tsk->parent);                          // ◀── đánh thức thread cha đang wait4()
    spin_unlock_irqrestore(&psig->siglock, flags);

    return autoreap;
}
```

Hai việc song song:

1. **`__send_signal(SIGCHLD, ..., parent, PIDTYPE_TGID)`** — SIGCHLD là **tín hiệu bình thường**, vào `parent->signal->shared_pending`, `sigaddset`, `complete_signal` → bật `TIF_SIGPENDING` trên một thread của cha, đánh thức nó. Handler `SIGCHLD` của cha (nếu có) chạy ở lần `ret_to_user` kế tiếp của thread đó (Module 16).
2. **`__wake_up_parent(tsk, parent)`** — đánh thức mọi thread của cha đang ngủ trong `wait4()`/`waitpid()` trên `parent->signal->wait_chldexit` (Module 18).

`si_pid`/`si_uid` tính trong pid-namespace và user-namespace **của cha** — đó là lý do dùng `__send_signal` cấp thấp chứ không phải helper cao hơn.

Nếu `tsk` là thread non-leader → `exit_signal == -1` → `exit_notify` **không gọi** `do_notify_parent` (không có `WARN_ON_ONCE(sig == -1)` bị trigger). Cha chỉ nhận SIGCHLD khi **cả process** (thread group) chết — tức khi group leader chết và mọi thread khác đã đi (`thread_group_empty`).

---

## 4. Ba câu hỏi

### Zombie process là gì?

Một task ở trạng thái **`EXIT_ZOMBIE`**: đã chạy xong hoàn toàn (`do_exit` xong, `TASK_DEAD`, kernel stack đã free, `mm`/`files`/`fs`/`sighand` đã nhả, không chiếm CPU, không chạy code), nhưng `task_struct` **được giữ lại** chỉ để mang:

- `exit_code` — chết thế nào (thoát bình thường + status, hay bị tín hiệu nào giết, hay có core dump);
- **PID** — để định danh "đứa con nào";
- thống kê CPU đã cộng dồn (`utime`, `stime`, `min_flt`, `nvcsw`...).

Trong `ps` hiện là `Z` / `<defunct>`. Tốn ~1 `task_struct` (~9 KB) + 1 `struct pid` + 1 slot PID. Biến mất khi cha `wait()` đọc status (hoặc khi reparent về init và init reap).

### Tại sao `task_struct`/PID vẫn tồn tại sau khi process đã exit?

Vì hợp đồng `wait()` của Unix: cha phải hỏi được "con tôi chết thế nào?" **sau khi** con đã đi. Câu trả lời nằm ở `task_struct.exit_code` + PID định danh con.

Nếu kernel free `task_struct` và tái dùng PID ngay tại `do_exit`:

- `wait()` của cha sau đó không có gì để trả về;
- PID có thể bị cấp cho một process mới → cha `wait(old_pid)` trúng nhầm process khác.

Nên `do_exit` → `exit_notify` **dừng ở `EXIT_ZOMBIE`**. `detach_pid(PIDTYPE_PID)` (giải phóng PID để tái dùng) và `free_task` được **hoãn tới `release_task()`**, chỉ chạy khi:

- cha gọi `wait4()`/`waitpid()` (sau khi đọc status), hoặc
- con **autoreap-able** (thread non-leader, hoặc cha đặt `SIGCHLD` = `SIG_IGN` / `SA_NOCLDWAIT`).

Nếu cha **không bao giờ** `wait()` và **không** chết → zombie ở lại vĩnh viễn ("zombie leak"). Nếu cha **chết trước** → `forget_original_parent` reparent zombie (và mọi con khác) sang **subreaper gần nhất** hoặc **init (PID 1)**; vòng lặp `wait()` của init reap nó.

### `release_task()` — dọn nốt

```c
void release_task(struct task_struct *p)
{
    atomic_dec(&__task_cred(p)->user->processes);        // giảm đếm process của user
    cgroup_release(p);
    write_lock_irq(&tasklist_lock);
    ptrace_release_task(p);
    __exit_signal(p):
        // giữ sighand->siglock:
        posix_cpu_timers_exit(p);
        sig->utime += p_utime; sig->stime += ...; sig->nvcsw += ...   // CỘNG DỒN stats vào signal_struct
        sig->nr_threads--;
        __unhash_process(p, group_dead):
            nr_threads--;
            detach_pid(p, PIDTYPE_PID);                  // ◀── gỡ khỏi pid hash → PID TÁI DÙNG ĐƯỢC
            if (group_dead) {
                detach_pid(p, PIDTYPE_TGID/PGID/SID);
                list_del_rcu(&p->tasks);                 // rời process list toàn cục
                list_del_init(&p->sibling);              // rời children list của cha
            }
            list_del_rcu(&p->thread_group);
            list_del_rcu(&p->thread_node);
        flush_sigqueue(&p->pending);
        p->sighand = NULL;
        __cleanup_sighand(sighand);                      // nhả ref sighand_struct
        if (group_dead) flush_sigqueue(&sig->shared_pending);
    write_unlock_irq(&tasklist_lock);
    proc_flush_pid(thread_pid);                          // xoá cache dentry /proc/<pid>
    put_pid(thread_pid);
    release_thread(p);                                   // arch: nop trên arm64
    put_task_struct_rcu_user(p):                         // ◀── bỏ THAM CHIẾU CUỐI
        if (refcount_dec_and_test(&p->rcu_users))
            call_rcu(&p->rcu, delayed_put_task_struct);  // → put_task_struct → __put_task_struct → free_task
}
```

`free_task(p)`: `free_thread_stack(p)` (thực ra stack đã free ở `finish_task_switch`) + `free_task_struct(p)` = `kmem_cache_free(task_struct_cachep, p)`. Việc free thật đi qua một **RCU grace period** (`call_rcu`) để các reader đang duyệt `tasklist` bằng `rcu_read_lock()` không đọc trúng bộ nhớ đã free.

---

## 5. Vòng đời đầy đủ

```
fork → chạy → do_exit:
  exit_signals (PF_EXITING) → exit_mm (address space mất) → exit_files → exit_fs → exit_thread
  exit_notify:
    forget_original_parent → con của nó reparent về subreaper/init
    exit_state = EXIT_ZOMBIE
    do_notify_parent → SIGCHLD vào shared_pending của cha + __wake_up_parent
    autoreap? → EXIT_DEAD + release_task ngay
  state = TASK_DEAD → do_task_dead → __schedule (không trở lại)
                                       ↓
  task kế tiếp: finish_task_switch → free kernel stack 16 KB

  [ zombie: chỉ còn task_struct + pid + exit_code + stats ]
                                       ↓
  cha: wait4() → do_wait → wait_task_zombie:
    đọc exit_code → status cho userspace
    release_task(zombie):
      __exit_signal: cộng stats vào signal_struct, detach_pid (PID tái dùng được)
      put_task_struct_rcu_user → call_rcu → free_task_struct

  [ hoặc: cha chết trước → init reap zombie qua wait() loop của nó ]
```

---

## 6. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|`do_exit` reset addr_limit|`set_fs(USER_DS)`|`force_uaccess_begin()` (5.10, sau bỏ `set_fs`)|
|Drop ref cuối trong `release_task`|`put_task_struct(p)` + `call_rcu(delayed_put_task_struct)`|`put_task_struct_rcu_user(p)` (tách `rcu_users` — 5.9)|
|`exit_notify` / `do_notify_parent`|như mô tả|y hệt (+ `do_notify_pidfd` — 5.3)|
|`exit_thread` (arm64)|`fpsimd_release_task` (nếu có SVE)|+ PAC/MTE cleanup|
|`_exit` vs `exit_group`|2 syscall riêng|y hệt|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `force_uaccess_begin()`, `put_task_struct_rcu_user`, `do_notify_pidfd`. Cơ chế "`do_exit` nhả tài nguyên → `EXIT_ZOMBIE` → `do_notify_parent` gửi SIGCHLD → `release_task` khi cha `wait()`" — **giống hệt 5.4**.