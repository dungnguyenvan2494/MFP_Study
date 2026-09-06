# MODULE 18 — `wait()` và Zombie trên Linux 5.4

Sơ đồ đã lưu: [docs/diagrams/wait-and-zombie-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/wait-and-zombie-arm64.md) (5 sơ đồ: `do_wait` quét/ngủ, `wait_consider_task` cây quyết định, ba trạng thái, `wait_task_zombie` reap, vòng đời process đầy đủ).

**File nguồn**: `kernel/exit.c` (`kernel_wait4`, `kernel_waitid`, `do_wait`, `do_wait_thread`, `ptrace_do_wait`, `wait_consider_task`, `wait_task_zombie`, `wait_task_stopped`, `wait_task_continued`, `child_wait_callback`, `release_task`), `kernel/signal.c` (`__wake_up_parent`, `do_notify_parent`).

---

## 0. Bối cảnh: cha–con và `wait_chldexit`

- Mỗi process có `parent->signal->wait_chldexit` — một wait queue. Cha ngủ ở đây trong `wait4()`.
- Khi con chết, `do_notify_parent` (Module 17) làm **hai việc**: (1) `__send_signal(SIGCHLD, ..., parent)` → `parent->signal->shared_pending`; (2) `__wake_up_parent(child, parent)` → đánh thức thread cha đang ngủ trên `wait_chldexit`.
- Con zombie giữ: `task_struct` + `struct pid` + `exit_code` + thống kê CPU. Nó biến mất khi `release_task()` chạy.

---

## 1. `wait4()` → `kernel_wait4` → `do_wait`

```c
long kernel_wait4(pid_t upid, int __user *stat_addr, int options, struct rusage *ru)
{
    struct wait_opts wo;
    struct pid *pid = NULL;
    enum pid_type type;

    if (upid == -1)       type = PIDTYPE_MAX;                        // bất kỳ con nào
    else if (upid < 0)  { type = PIDTYPE_PGID; pid = find_get_pid(-upid); }   // con trong process group -upid
    else if (upid == 0) { type = PIDTYPE_PGID; pid = get_task_pid(current, PIDTYPE_PGID); }  // cùng pgrp
    else /* upid > 0 */ { type = PIDTYPE_PID;  pid = find_get_pid(upid); }    // đúng con pid

    wo.wo_type   = type;
    wo.wo_pid    = pid;
    wo.wo_flags  = options | WEXITED;      // wait4 luôn muốn con đã exit
    wo.wo_rusage = ru;
    ret = do_wait(&wo);
    put_pid(pid);
    if (ret > 0 && stat_addr && put_user(wo.wo_stat, stat_addr))   // ◀── copy `status` cho userspace
        ret = -EFAULT;
    return ret;                            // = PID con đã reap, hoặc 0 (WNOHANG), hoặc -ECHILD / -ERESTARTSYS
}
```

Quan hệ API:

- `wait(&status)` = `waitpid(-1, &status, 0)` = `wait4(-1, &status, 0, NULL)`.
- `waitpid(pid, &status, opts)` = `wait4(pid, &status, opts, NULL)`.
- `waitid(idtype, id, &siginfo, opts)` → `kernel_waitid` → `do_wait` (đặt `wo_info`; hỗ trợ `WEXITED`/`WSTOPPED`/`WCONTINUED`/`WNOWAIT` tách biệt — không tự thêm `WEXITED`).
- `WNOHANG`: không block, trả 0 nếu chưa có con nào reap được.
- `WUNTRACED`/`WSTOPPED`: cũng báo con bị stop (`SIGSTOP`). `WCONTINUED`: cũng báo con vừa `SIGCONT`. `WNOWAIT`: xem status nhưng **không** reap (con vẫn zombie).

---

## 2. `do_wait(&wo)` — quét hoặc ngủ

```c
static long do_wait(struct wait_opts *wo)
{
    struct task_struct *tsk;
    int retval;

    init_waitqueue_func_entry(&wo->child_wait, child_wait_callback);
    wo->child_wait.private = current;
    add_wait_queue(&current->signal->wait_chldexit, &wo->child_wait);   // ◀── vào queue mà do_notify_parent đánh thức
repeat:
    wo->notask_error = -ECHILD;                        // mặc định: "bạn không có con nào như thế"
    if ((wo->wo_type < PIDTYPE_MAX) &&
        (!wo->wo_pid || !pid_has_task(wo->wo_pid, wo->wo_type)))
        goto notask;                                    // wo_pid không ứng với task nào → khỏi quét

    set_current_state(TASK_INTERRUPTIBLE);              // ◀── ARM trước khi quét (rào bộ nhớ — Module 12)
    read_lock(&tasklist_lock);
    tsk = current;
    do {
        retval = do_wait_thread(wo, tsk);              // quét tsk->children
        if (retval) goto end;                          //   reap được → thoát với PID
        retval = ptrace_do_wait(wo, tsk);             // quét tsk->ptraced
        if (retval) goto end;
        if (wo->wo_flags & __WNOTHREAD) break;
    } while_each_thread(current, tsk);                 // ◀── quét con của MỌI thread trong nhóm gọi
    read_unlock(&tasklist_lock);

notask:
    retval = wo->notask_error;                         // = 0 nếu có con khớp (dù chưa reap); = -ECHILD nếu không
    if (!retval && !(wo->wo_flags & WNOHANG)) {
        retval = -ERESTARTSYS;
        if (!signal_pending(current)) {
            schedule();                                 // ◀── NGỦ, chờ child_wait_callback
            goto repeat;                                // thức dậy → quét lại từ đầu
        }
        // signal_pending → return -ERESTARTSYS (wait bị Ctrl+C cắt)
    }
end:
    __set_current_state(TASK_RUNNING);
    remove_wait_queue(&current->signal->wait_chldexit, &wo->child_wait);
    return retval;
}
```

`do_wait_thread(wo, tsk)`:

```c
list_for_each_entry(p, &tsk->children, sibling) {
    int ret = wait_consider_task(wo, 0, p);
    if (ret) return ret;
}
```

`while_each_thread(current, tsk)`: quét `children` của **tất cả thread trong thread group của caller** — nên thread T1 fork con, thread T2 vẫn `wait()` được (trừ khi `__WNOTHREAD`).

### `child_wait_callback` — wakeup CÓ MỤC TIÊU

Khi con chết: `do_notify_parent` → `__wake_up_parent(child, parent)` → `__wake_up_sync_key(&parent->signal->wait_chldexit, TASK_INTERRUPTIBLE, child)` → duyệt queue, gọi:

```c
static int child_wait_callback(wait_queue_entry_t *wait, unsigned mode, int sync, void *key)
{
    struct wait_opts *wo = container_of(wait, struct wait_opts, child_wait);
    struct task_struct *p = key;                        // = con vừa chết

    if (!eligible_pid(wo, p))                           // waiter này KHÔNG chờ đứa con này
        return 0;                                       //   → KHÔNG đánh thức
    if ((wo->wo_flags & __WNOTHREAD) && wait->private != p->parent)
        return 0;
    return default_wake_function(wait, mode, sync, key);   // đánh thức → nó goto repeat
}
```

→ Nếu process có 5 thread cùng `wait4()` cho 5 con khác nhau, chỉ thread chờ đúng con vừa chết được đánh thức. Tránh "thundering herd".

---

## 3. `wait_consider_task(wo, ptrace, p)` — với một đứa con

```c
int exit_state = READ_ONCE(p->exit_state);

if (exit_state == EXIT_DEAD)
    return 0;                                           // thread khác đang reap p rồi → bỏ qua

ret = eligible_child(wo, ptrace, p);                    // p khớp wo_pid/wo_type + cờ __WCLONE/__WALL?
if (!ret) return ret;

if (exit_state == EXIT_TRACE) {                         // zombie bị ptrace, đang được bàn giao
    if (!ptrace) wo->notask_error = 0;
    return 0;
}

if (!ptrace && p->ptrace && !ptrace_reparented(p))     // cha thật cũng đang trace con
    ptrace = 1;                                          //   → coi caller như ptracer

if (exit_state == EXIT_ZOMBIE) {
    if (!delay_group_leader(p)) {                       // không reap leader còn sub-thread sống
        if (ptrace || !p->ptrace)
            return wait_task_zombie(wo, p);             // ◀── REAP
    }
    if (!ptrace || (wo->wo_flags & (WCONTINUED | WEXITED)))
        wo->notask_error = 0;                            // "có con khớp, chỉ chưa reap được"
} else {
    // p CÒN SỐNG — kiểm trạng thái stopped/continued:
    wo->notask_error = 0;
    ret = wait_task_stopped(wo, ptrace, p);             // WUNTRACED/WSTOPPED → báo si_code = CLD_STOPPED/CLD_TRAPPED
    if (!ret) ret = wait_task_continued(wo, p);         // WCONTINUED → báo si_code = CLD_CONTINUED
    return ret;                                          // KHÔNG release_task — con còn sống
}
return 0;
```

**`notask_error`**: khởi tạo `-ECHILD` trong `do_wait`. Chỉ cần thấy **một** con khớp tiêu chí (dù chưa reap được) → set `0`. Nên `wait()` trả `-ECHILD` **chỉ khi** caller thật sự không có con nào khớp; có con còn sống → block (hoặc trả 0 với `WNOHANG`).

---

## 4. `wait_task_zombie(wo, p)` — reap

```c
static int wait_task_zombie(struct wait_opts *wo, struct task_struct *p)
{
    int state, status;
    pid_t pid = task_pid_vnr(p);                        // PID con trong pid-ns của caller

    if (!likely(wo->wo_flags & WEXITED))
        return 0;

    if (unlikely(wo->wo_flags & WNOWAIT)) {             // chỉ XEM, không reap
        status = p->exit_code;
        // fill rusage, exit_state KHÔNG đổi, return pid
        goto out_info;
    }

    // ── GIÀNH quyền reap: chỉ MỘT thread thắng ──
    state = (ptrace_reparented(p) && thread_group_leader(p)) ? EXIT_TRACE : EXIT_DEAD;
    if (cmpxchg(&p->exit_state, EXIT_ZOMBIE, state) != EXIT_ZOMBIE)
        return 0;                                        // thua race — thread wait4() khác đã lấy

    read_unlock(&tasklist_lock);                        // "ta sở hữu p, không ai reap nữa"

    if (state == EXIT_DEAD && thread_group_leader(p)) {
        // ── CỘNG DỒN cputime của con vào signal_struct CỦA CHA (trường c* = "children") ──
        thread_group_cputime_adjusted(p, &tgutime, &tgstime);
        psig->cutime   += tgutime + sig->cutime;
        psig->cstime   += tgstime + sig->cstime;
        psig->cgtime   += ...;
        psig->cmin_flt += p->min_flt + sig->min_flt + sig->cmin_flt;
        psig->cmaj_flt += ...;
        psig->cnvcsw   += p->nvcsw + sig->nvcsw + sig->cnvcsw;
        psig->cnivcsw  += ...;
        psig->cinblock += ...; psig->coublock += ...;
        // ◀── ĐÂY là nguồn số của getrusage(RUSAGE_CHILDREN) và lệnh `time`
    }

    if (wo->wo_rusage)
        getrusage(p, RUSAGE_BOTH, wo->wo_rusage);

    status = (p->signal->flags & SIGNAL_GROUP_EXIT)
             ? p->signal->group_exit_code               // exit_group() / fatal signal
             : p->exit_code;                            // _exit() của thread leader
    wo->wo_stat = status;                               // ◀── `int status` mà cha nhận (WIFEXITED/WEXITSTATUS...)

    if (state == EXIT_TRACE) {                          // con bị ai đó khác cha thật ptrace
        write_lock_irq(&tasklist_lock);
        ptrace_unlink(p);                               // gỡ khỏi tracer
        state = EXIT_ZOMBIE;
        if (do_notify_parent(p, p->exit_signal))        // báo lại CHA THẬT
            state = EXIT_DEAD;
        p->exit_state = state;
        write_unlock_irq(&tasklist_lock);
    }

    if (state == EXIT_DEAD)
        release_task(p);                                // ◀── free pid + task_struct (Module 17)

out_info:
    infop = wo->wo_info;                                // (waitid) điền struct siginfo
    if (infop) {
        if ((status & 0x7f) == 0) { infop->cause = CLD_EXITED; infop->status = status >> 8; }
        else { infop->cause = (status & 0x80) ? CLD_DUMPED : CLD_KILLED; infop->status = status & 0x7f; }
        infop->pid = pid; infop->uid = uid;
    }
    return pid;                                         // wait4() trả về PID con
}
```

Giải mã `status` ở userspace (`<sys/wait.h>`):

- `WIFEXITED(status)` = `(status & 0x7f) == 0` → `WEXITSTATUS(status)` = `status >> 8`
- `WIFSIGNALED(status)` = `((signed char)((status & 0x7f) + 1) >> 1) > 0` → `WTERMSIG(status)` = `status & 0x7f`
- `WCOREDUMP(status)` = `status & 0x80`
- `WIFSTOPPED` / `WSTOPSIG` (khi `WUNTRACED`), `WIFCONTINUED` (khi `WCONTINUED`)

---

## 5. `TASK_DEAD` vs `EXIT_ZOMBIE` vs `EXIT_DEAD` — khác nhau thế nào

Chúng ở **hai trường khác nhau** trong `task_struct`:

- `task->state` — trạng thái **lập lịch** ("task này chạy được không?").
- `task->exit_state` — trạng thái **reap** ("`task_struct` còn sống bao lâu nữa?").

||`TASK_DEAD`|`EXIT_ZOMBIE`|`EXIT_DEAD`|
|---|---|---|---|
|Trường|`task->state`|`task->exit_state`|`task->exit_state`|
|Nghĩa|"Sẽ **không bao giờ chạy lại**. Kernel stack free được."|"Đã xong, **giữ lại** để cha đọc `exit_code`/PID/stats bằng `wait()`."|"Đang được **reap ngay bây giờ**." Chuyển tiếp, rất ngắn.|
|Đặt bởi|`do_task_dead()` — cuối `do_exit`, qua `set_special_state`|`exit_notify()` — trong `do_exit`; hoặc `wait_task_zombie` đặt lại sau `EXIT_TRACE`|`wait_task_zombie` (`cmpxchg`); hoặc `exit_notify` khi `autoreap` (bỏ qua zombie)|
|Rời khỏi bởi|`finish_task_switch(prev)` của task kế: `prev_state == TASK_DEAD` → `put_task_stack(prev)` + bỏ ref scheduler|`wait_task_zombie` `cmpxchg` sang `EXIT_DEAD` (hoặc `EXIT_TRACE`)|`release_task` → `put_task_struct_rcu_user` → `call_rcu` → `free_task` sau grace period|
|Hệ quả|**Kernel stack 16 KB được free**|`task_struct` + `pid` + `exit_code` vẫn tồn tại; hiện `Z`/`<defunct>` trong `ps`|Chốt `cmpxchg` — chặn **hai** thread `wait4()` cùng reap một zombie|

**Đồng thời tồn tại**: ngay sau `exit_notify`, task là `state = TASK_RUNNING` nhưng `exit_state = EXIT_ZOMBIE` (nó vẫn đang chạy `do_exit`). Rồi cuối `do_exit`: `state = TASK_DEAD` **và** `exit_state = EXIT_ZOMBIE` cùng lúc — task off-CPU vĩnh viễn, chờ cha `wait()`.

`EXIT_TRACE` (biến thể của `EXIT_DEAD`): zombie bị process khác (không phải cha thật) `ptrace`. `wait_task_zombie` giành nó bằng `cmpxchg(EXIT_ZOMBIE → EXIT_TRACE)`, rồi `ptrace_unlink` + `do_notify_parent(cha thật)` + đặt `EXIT_ZOMBIE` (nếu cha còn muốn) hoặc `EXIT_DEAD` (nếu autoreap).

---

## 6. Vòng đời process đầy đủ

```
fork()
  copy_process → task_struct + kernel stack (16 KB) được cấp phát
  p->state = TASK_NEW → wake_up_new_task → p->state = TASK_RUNNING
        │
        ▼
RUNNING  (state = TASK_RUNNING; exit_state = 0)
  ⇄ TASK_INTERRUPTIBLE / TASK_UNINTERRUPTIBLE   (ngủ/thức — Module 12)
        │
        │  exit(status)  |  tín hiệu chí mạng  |  exit_group()
        ▼
do_exit(code):
  exit_signals (PF_EXITING) · exit_mm (address space mất) · exit_files · exit_fs · exit_thread
  exit_notify:
     forget_original_parent → CON của nó reparent về subreaper / init
     exit_state = EXIT_ZOMBIE                                   ◀── ZOMBIE (state vẫn RUNNING)
     do_notify_parent(SIGCHLD) → parent->signal->shared_pending
                              → __wake_up_parent → parent->signal->wait_chldexit
     [ autoreap? (non-leader thread / cha SIG_IGN SIGCHLD) → exit_state = EXIT_DEAD, release_task NGAY ]
  state = TASK_DEAD                                             ◀── không bao giờ chạy lại
  do_task_dead() → __schedule()  (KHÔNG trả về)
        │
        ▼  (task kế tiếp chạy)
finish_task_switch(dead_task):
  put_task_stack() → KERNEL STACK 16 KB được FREE              ◀── STACK GONE
  (task_struct vẫn sống: cha giữ 1 ref qua children list)
        │
        ▼
ZOMBIE  (exit_state = EXIT_ZOMBIE, state = TASK_DEAD, off CPU)
  giữ: task_struct (~9 KB) + struct pid + exit_code + thống kê CPU
  hiện 'Z' / '<defunct>' trong ps ; chiếm 1 slot PID
        │
        │  cha: wait4() → do_wait → do_wait_thread → wait_consider_task
        ▼
wait_task_zombie(p):
  WEXITED? có
  cmpxchg(p->exit_state, EXIT_ZOMBIE → EXIT_DEAD)              ◀── EXIT_DEAD (giành được)
  cộng dồn cputime của p vào parent->signal->c{utime,stime,min_flt,nvcsw,...}
  wo_stat = exit_code  → kernel_wait4: put_user(status, stat_addr)   ◀── cha nhận status
  release_task(p):
     __exit_signal → cộng dồn stats vào signal_struct → __unhash_process → detach_pid(PIDTYPE_PID)   ◀── PID TÁI DÙNG ĐƯỢC
     put_task_struct_rcu_user → call_rcu(delayed_put_task_struct)
        │
        ▼  (sau một RCU grace period)
free_task(p):  free_thread_stack (đã gone) + kmem_cache_free(task_struct_cachep, p)   ◀── TASK_STRUCT FREED
        │
        ▼
[ hết ]  — wait4() đã trả PID con cho cha
```

**Nếu cha chết trước con**: `forget_original_parent` reparent con (zombie hoặc còn sống) về subreaper gần nhất hoặc **init (PID 1)**. Vòng lặp `wait()` của init reap zombie → không leak.

**Nếu cha sống nhưng không bao giờ `wait()`**: zombie ở lại **vĩnh viễn** cho tới khi cha chết (rồi init reap). Đây là "zombie leak" — mỗi zombie tốn 1 `task_struct` + 1 `struct pid` + 1 slot PID.

**Kernel stack free sớm, `task_struct` free muộn**: stack đi ở `finish_task_switch` (ngay sau `do_task_dead`); `task_struct` đi ở `release_task` (khi cha `wait()`), qua RCU. Zombie chính là khoảng giữa hai mốc đó.

---

## 7. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|`do_wait` / `wait_consider_task` / `wait_task_zombie`|như mô tả|y hệt|
|`child_wait_callback` targeted wakeup|có|có|
|`release_task` drop ref|`put_task_struct` + `call_rcu`|`put_task_struct_rcu_user` (5.9)|
|`EXIT_TRACE`|có (từ 3.19)|có|
|`pidfd` + `waitid(P_PIDFD)`|`pidfd_open` (5.3), `waitid(P_PIDFD)` (5.4)|ổn định|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `wait_task_zombie` có `put_task_struct_rcu_user` gián tiếp qua `release_task`. Cơ chế "quét `children` → `cmpxchg EXIT_ZOMBIE→EXIT_DEAD` → cộng cputime vào `c*` → `release_task`" và ba trạng thái — **giống hệt 5.4**.