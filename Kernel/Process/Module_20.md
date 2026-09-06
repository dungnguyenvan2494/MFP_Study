# MODULE 20 — PID / TGID trên Linux 5.4

Sơ đồ đã lưu: [docs/diagrams/pid-tgid-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/pid-tgid-arm64.md) (4 sơ đồ: `struct pid` ↔ `task_struct`, bốn `PIDTYPE_*`, `upid` + pid namespace, `getpid()` vs `gettid()`).

**File nguồn**: `include/linux/pid.h` (`struct pid`, `struct upid`, `enum pid_type`, `pid_nr`/`pid_vnr`/`pid_nr_ns`), `include/linux/pid_namespace.h` (`struct pid_namespace`), `include/linux/sched.h` (`task_pid`/`task_tgid`/`task_pid_ptr`, `task_*_nr`/`task_*_vnr`), `kernel/pid.c` (`alloc_pid`, `__task_pid_nr_ns`, `attach_pid`), `kernel/sys.c` (`getpid`/`gettid`/`getppid`), `kernel/fork.c` (`copy_process` PID setup, `init_task_pid`).

---

## 0. Ba con số — thuật ngữ nhân vs userspace

|Userspace gọi|Nhân gọi|Là gì|Lấy bằng|
|---|---|---|---|
|**TID** (thread ID)|`task->pid`|Số định danh **một task** (một thread). Mỗi thread một số.|`gettid()`|
|**PID** (process ID)|`task->tgid`|Số của **thread-group leader** = "danh tính process". Mọi thread cùng process có cùng số.|`getpid()`|
|**PGID** (process group ID)|`PIDTYPE_PGID`|Nhóm process cho **job control** (một pipeline shell = một process group).|`getpgid()` / `setpgid()`|
|**SID** (session ID)|`PIDTYPE_SID`|**Session** — một lần đăng nhập terminal; có controlling tty.|`getsid()` / `setsid()`|

Điểm dễ nhầm: **"PID" trong tài liệu nhân Linux thường nghĩa là `task->pid` = TID**, không phải "process ID" của POSIX. `task->tgid` mới là "process ID" mà `getpid()` trả về.

---

## 1. `struct pid` — đối tượng định danh (không phải con số)

`include/linux/pid.h`:

```c
struct pid {
    refcount_t         count;               // đếm tham chiếu — sống LÂU HƠN task_struct
    unsigned int       level;               // độ sâu pid-namespace (init = 0)
    spinlock_t         lock;                // (5.9+; 5.4 chưa có)
    struct hlist_head  tasks[PIDTYPE_MAX];  // 4 danh sách task "dùng" pid này theo từng type
    struct hlist_head  inodes;              // (5.9+ pidfd) — 5.4 chưa có
    wait_queue_head_t  wait_pidfd;          // waitqueue cho pidfd poll (5.3+)
    struct rcu_head    rcu;
    struct upid        numbers[1];          // MẢNG level+1 phần tử — 1 con số / 1 namespace
};
```

- **`count`**: `struct pid` có refcount riêng. Nó tồn tại chừng nào còn ai giữ: task đang sống, một fd `/proc/<pid>`, một `pidfd`, một mục trong `pidmap`, một `wait_queue`. Nên `struct pid` có thể **sống sau khi `task_struct` đã bị `free`** — đó là cách `pidfd` biết process đã chết mà không bị race với PID tái dùng.
- **`tasks[PIDTYPE_MAX]`**: 4 hlist. `tasks[PIDTYPE_PID]` = task có TID này (đúng 1). `tasks[PIDTYPE_TGID]` = **mọi thread** của thread group có leader sở hữu pid này. `tasks[PIDTYPE_PGID]` = mọi task trong process group. `tasks[PIDTYPE_SID]` = mọi task trong session.
- **`numbers[]`**: mảng `struct upid`, một phần tử cho mỗi cấp namespace từ 0 tới `level`.

`task_struct` phía nó có:

```c
pid_t                 pid;                  // cache số TID trong init_pid_ns (= thread_pid->numbers[0].nr)
pid_t                 tgid;                 // cache số TGID trong init_pid_ns (= group_leader->pid)
struct pid           *thread_pid;           // struct pid cho PIDTYPE_PID của task này (TID)
struct hlist_node     pid_links[PIDTYPE_MAX]; // móc task vào pid->tasks[type]
// và trong signal_struct (CHUNG cả nhóm):
struct pid           *pids[PIDTYPE_MAX];    // struct pid cho TGID / PGID / SID
```

**`task_pid_ptr(task, type)`** — hàm bản lề:

```c
static inline struct pid **task_pid_ptr(struct task_struct *task, enum pid_type type)
{
    return (type == PIDTYPE_PID)
           ? &task->thread_pid              // RIÊNG mỗi task
           : &task->signal->pids[type];     // CHUNG cả thread group (signal_struct chung)
}
```

---

## 2. `struct upid` — gắn `struct pid` với một con số trong một namespace

```c
struct upid {
    int                    nr;              // con số trong namespace này
    struct pid_namespace  *ns;              // namespace đó
};
```

Một `struct pid` được cấp trong pid-ns cấp `level` có **`level + 1`** phần tử `numbers[]`:

- `numbers[0]` = `{nr, ns = init_pid_ns}` — số nhìn từ init ns (global).
- `numbers[k]` = `{nr, ns = ns cấp k}`.
- `numbers[level]` = `{nr, ns = ns cấp cuối}` — số nhìn từ chính ns tạo ra nó.

Mỗi `upid` được đăng ký vào `upid->ns->idr` (một **IDR** = radix-tree ánh xạ `id → con trỏ`). `find_pid_ns(nr, ns)` = tra `ns->idr` tại vị trí `nr`.

**Ví dụ container**: process "init" của container (pid-ns cấp 1):

```
struct pid X:  level = 1
  numbers[0] = {nr = 5000, ns = init_pid_ns}       ← host thấy "PID 5000"
  numbers[1] = {nr = 1,    ns = container_pid_ns}  ← trong container thấy "PID 1"
```

Cùng **một** `struct pid X`, hai con số.

Các hàm lấy số:

|Hàm|Trả về|
|---|---|
|`pid_nr(pid)`|`pid->numbers[0].nr` — số **global** (từ init_pid_ns)|
|`pid_vnr(pid)`|`pid_nr_ns(pid, task_active_pid_ns(current))` — số **virtual** (từ pid-ns của tiến trình hiện tại)|
|`pid_nr_ns(pid, ns)`|`ns->level <= pid->level` → `pid->numbers[ns->level].nr`; ngược lại → **0**|

Quy tắc `pid_nr_ns` trả 0 khi `ns->level > pid->level`: một tiến trình trong container **không thấy** được số của tiến trình chỉ tồn tại ở host. Ngược lại, host (ns cấp 0) luôn thấy `numbers[0].nr` của mọi `struct pid`.

`task_pid_nr(tsk)` = `tsk->pid` (số TID global). `task_pid_vnr(tsk)` = `__task_pid_nr_ns(tsk, PIDTYPE_PID, NULL)`. Tương tự `task_tgid_nr` / `task_tgid_vnr`.

`__task_pid_nr_ns(task, type, ns)`:

```c
pid_t __task_pid_nr_ns(struct task_struct *task, enum pid_type type, struct pid_namespace *ns)
{
    rcu_read_lock();
    if (!ns) ns = task_active_pid_ns(current);
    pid_t nr = pid_nr_ns(rcu_dereference(*task_pid_ptr(task, type)), ns);   // ◀── task_pid_ptr
    rcu_read_unlock();
    return nr;
}
```

---

## 3. `struct pid_namespace`

`include/linux/pid_namespace.h`:

```c
struct pid_namespace {
    struct kref            kref;
    struct idr             idr;             // id → struct pid, cho ns này
    unsigned int           pid_allocated;
    struct task_struct    *child_reaper;    // "init" của ns này (PID 1 trong ns) — reap orphan trong ns
    struct kmem_cache     *pid_cachep;      // slab cho struct pid cỡ (level+1) upid
    unsigned int           level;           // độ sâu (init = 0)
    struct pid_namespace  *parent;          // ns bao ngoài
    struct user_namespace *user_ns;
    struct ucounts        *ucounts;
    int                    reboot;
    struct ns_common       ns;
};
```

- **`child_reaper`**: khi một process trong ns này mồ côi (cha chết), nó được reparent về `child_reaper` (không phải init toàn cục). Khi `child_reaper` chết → `zap_pid_ns_processes()` giết **mọi** process trong ns (giống PID 1 chết trên hệ thống thật → panic; trong ns → cả ns bị dọn).
- **`level`** / **`parent`**: cây lồng nhau. `init_pid_ns.level = 0`.
- Tạo bằng `clone(CLONE_NEWPID)` / `unshare(CLONE_NEWPID)` → `copy_pid_ns()` tạo ns con, gán vào `current->nsproxy->pid_ns_for_children`. **Chỉ ảnh hưởng con của caller** — bản thân caller vẫn ở ns cũ (một process không đổi được pid-ns của chính mình).
- `task_active_pid_ns(tsk)` = `ns_of_pid(task_pid(tsk))` = `task->thread_pid->numbers[level].ns`.

---

## 4. Vì sao `getpid()` trong process đa luồng trả TGID

### Định nghĩa

```c
SYSCALL_DEFINE0(getpid)  { return task_tgid_vnr(current); }
SYSCALL_DEFINE0(gettid)  { return task_pid_vnr(current);  }
SYSCALL_DEFINE0(getppid) { return task_tgid_vnr(rcu_dereference(current->real_parent)); }
```

### Truy vết `getpid()`

```
getpid()
= task_tgid_vnr(current)
= __task_pid_nr_ns(current, PIDTYPE_TGID, NULL)
= pid_nr_ns( *task_pid_ptr(current, PIDTYPE_TGID), current's pid ns )
= pid_nr_ns( current->signal->pids[PIDTYPE_TGID], ns )        ← vì type != PIDTYPE_PID
```

`current->signal` là **`signal_struct` chung của cả thread group** (`CLONE_THREAD` ⇒ `CLONE_SIGHAND`; `copy_signal` `return 0` ngay khi `CLONE_THREAD` → thread dùng lại `signal_struct` của leader). Nên `current->signal->pids[PIDTYPE_TGID]` là **cùng một con trỏ `struct pid`** cho **mọi** thread — chính là `thread_pid` của group leader.

`pid_nr_ns()` của một `struct pid` cố định → một con số cố định. Con số đó = TID của leader = "process ID".

### Truy vết `gettid()`

```
gettid()
= task_pid_vnr(current)
= pid_nr_ns( *task_pid_ptr(current, PIDTYPE_PID), ns )
= pid_nr_ns( current->thread_pid, ns )                        ← vì type == PIDTYPE_PID
```

`current->thread_pid` là **`struct pid` riêng của từng task**. Nên mỗi thread ra số khác nhau.

### Cơ chế trong `copy_process` khi tạo thread

```c
// kernel/fork.c
pid = alloc_pid(p->nsproxy->pid_ns_for_children, args->set_tid, args->set_tid_size);
// → struct pid MỚI cho TID của thread mới

p->pid = pid_nr(pid);
init_task_pid(p, PIDTYPE_PID, pid);   // p->thread_pid = pid;  chuẩn bị hlist pid->tasks[PIDTYPE_PID]

if (thread_group_leader(p)) {                        // TẠO PROCESS MỚI
    init_task_pid(p, PIDTYPE_TGID, pid);             //   signal->pids[TGID] = pid (của chính nó)
    init_task_pid(p, PIDTYPE_PGID, task_pgrp(current)); // kế thừa process group của cha
    init_task_pid(p, PIDTYPE_SID,  task_session(current)); // kế thừa session của cha
    p->tgid = p->pid;
    ...
    attach_pid(p, PIDTYPE_TGID);
    attach_pid(p, PIDTYPE_PGID);
    attach_pid(p, PIDTYPE_SID);
} else {                                             // TẠO THREAD
    p->tgid = current->tgid;
    // ◀── KHÔNG init_task_pid(PIDTYPE_TGID/PGID/SID)
    //     KHÔNG attach_pid(PIDTYPE_TGID/PGID/SID)
    //     → signal->pids[TGID/PGID/SID] giữ nguyên giá trị của leader (vì signal_struct CHUNG)
    current->signal->nr_threads++;
    list_add_tail_rcu(&p->thread_group, &p->group_leader->thread_group);
}
attach_pid(p, PIDTYPE_PID);           // giờ find_task_by_vpid(TID) thấy thread mới
```

→ Thread mới **cấp `struct pid` riêng cho TID** và móc vào `pid->tasks[PIDTYPE_PID]`, nhưng **cố ý không đụng** `signal->pids[PIDTYPE_TGID]`. Cái đó vẫn trỏ vào `struct pid` của leader. Nên `getpid()` của mọi thread → cùng số.

### Vì sao POSIX yêu cầu thế

- POSIX: "một process có **một** process ID; mọi thread của process chia sẻ nó". `kill(getpid(), sig)` từ thread bất kỳ phải tác động **cả process**.
- Nếu `getpid()` trả `task->pid` (TID) → mỗi thread một số → `kill(getpid(), SIGTERM)` từ thread 2 chỉ giết thread 2, không giết process. `getpid()` dùng làm khoá "danh tính process" (file lock, IPC key, log) sẽ không nhất quán.
- **Lịch sử**: LinuxThreads (trước 2.6) thực sự trả số khác nhau per thread — một điểm **không tuân POSIX** nổi tiếng. NPTL + `CLONE_THREAD` + trường `tgid` + `getpid() = task_tgid_vnr()` đã sửa.

---

## 5. `attach_pid` / `detach_pid` / `de_thread` / `pidfd`

**`attach_pid(task, type)`** (giữ `write_lock(&tasklist_lock)`): `hlist_add_head_rcu(&task->pid_links[type], &(*task_pid_ptr(task, type))->tasks[type])`. Từ đây `pid_task()`, `find_task_by_vpid()`, gửi tín hiệu theo nhóm (`kill(-pgid)`) thấy task.

**`detach_pid(task, type)`** (ở `__unhash_process` trong `release_task` — Module 17/18): gỡ khỏi hlist; nếu `pid->tasks[type]` rỗng hết → thả ref `struct pid` cho type đó. Khi `PIDTYPE_PID` cũng gỡ và không ai giữ nữa → `free_pid()` → xoá khỏi mọi `idr`, `call_rcu(pid, delayed_put_pid)` → số PID **tái dùng được**.

**`de_thread()`** (khi thread non-leader gọi `execve`): thread gọi `exec` "trở thành" leader. `exchange_tids(new_leader, old_leader)` **hoán đổi `thread_pid`** giữa hai task → task sống sót giữ `struct pid` của process (số TGID/PID mà userspace thấy **không đổi**), còn leader cũ (giờ là zombie) mang TID cũ của thread gọi exec.

**`pidfd`** (5.3+, có trong 5.4): `pidfd_open(pid, 0)` hoặc `clone(CLONE_PIDFD)` → một `struct file` tham chiếu `struct pid` (giữ `count`). `poll()` nó → `POLLIN` khi process chết (`wait_pidfd` được đánh thức ở `do_notify_pidfd`). `pidfd_send_signal(pidfd, sig, info, 0)` → gửi tín hiệu **không race** với PID tái dùng (vì tham chiếu `struct pid`, không phải con số). `struct pid` sống nhờ ref của fd ngay cả sau khi task bị reap.

---

## 6. Bảng phân biệt cuối

||Đối tượng|Con trỏ trong nhân|Per-task hay per-group|Con số lấy bằng|Ví dụ dùng|
|---|---|---|---|---|---|
|**TID**|`struct pid` (PIDTYPE_PID)|`task->thread_pid`|**per-task**|`gettid()`, `task_pid_vnr`|`tgkill(tgid, tid, sig)`, `/proc/<tgid>/task/<tid>`|
|**PID** (process)|`struct pid` (PIDTYPE_TGID)|`task->signal->pids[TGID]`|**per-group** (= leader's `thread_pid`)|`getpid()`, `task_tgid_vnr`|`kill(pid, sig)`, `waitpid`, file lock|
|**PGID**|`struct pid` (PIDTYPE_PGID)|`task->signal->pids[PGID]`|per-group|`getpgid()`|`kill(-pgid, sig)`, job control (`Ctrl+C` → SIGINT tới foreground pgroup)|
|**SID**|`struct pid` (PIDTYPE_SID)|`task->signal->pids[SID]`|per-group|`getsid()`|controlling tty, `SIGHUP` khi session leader chết|

Một `struct pid` có thể là TGID của process này **đồng thời** PGID của process group nó dẫn **đồng thời** SID của session (nếu nó là session leader) — nó nằm trong nhiều `tasks[type]` cùng lúc.

---

## 7. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|`struct pid`|`count`, `level`, `tasks[]`, `wait_pidfd`, `rcu`, `numbers[]`|+ `spinlock_t lock`, `struct hlist_head inodes` (5.9 pidfd rework)|
|Lookup|`pid_hash` bảng băm|đổi sang `idr` per-namespace (đã có từ 4.15)|
|`alloc_pid`|1 tham số `ns`|+ `set_tid`/`set_tid_size` (5.5, cho CRIU)|
|`task_struct` PID|`struct pid *thread_pid` + `pid_links[PIDTYPE_MAX]`|y hệt (đổi từ `struct pid_link pids[]` ở 4.19)|
|pidfd|`pidfd_open` (5.3), `clone(CLONE_PIDFD)` (5.2), `pidfd_send_signal` (5.1)|+ `pidfd` cho `waitid(P_PIDFD)` (5.4), `setns(pidfd)` (5.8)|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `struct pid` có thêm `lock` và `inodes`. Cơ chế `task_pid_ptr` rẽ `thread_pid` vs `signal->pids[]`, `upid` per-namespace, `getpid()==task_tgid_vnr` — **giống hệt 5.4**.