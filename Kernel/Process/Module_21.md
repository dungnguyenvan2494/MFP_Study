# MODULE 21 — Process Tree trên Linux 5.4

Sơ đồ đã lưu: [docs/diagrams/process-tree-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/process-tree-arm64.md) (4 sơ đồ: 5 trường quan hệ, cây `init→bash→program→child1/child2`, `forget_original_parent`, timeline bash-exit).

**File nguồn**: `include/linux/sched.h` (`real_parent`, `parent`, `children`, `sibling`, `ptraced`, `ptrace_entry`), `kernel/fork.c` (`copy_process` — dựng liên kết cây), `kernel/exit.c` (`forget_original_parent`, `find_child_reaper`, `find_new_reaper`, `reparent_leader`), `kernel/ptrace.c` (`__ptrace_link`, `__ptrace_unlink`, `exit_ptrace`), `kernel/sys.c` (`getppid`), `kernel/pid_namespace.c` (`zap_pid_ns_processes`).

---

## 0. Cây process là cây của các thread-group leader

`copy_process` chỉ móc vào cây khi task là **leader**:

```c
if (thread_group_leader(p)) {
    list_add_tail(&p->sibling, &p->real_parent->children);   // vào cây cha/con
    list_add_tail_rcu(&p->tasks, &init_task.tasks);          // vào danh sách toàn cục
    ...
} else {
    // thread: chỉ vào signal->thread_head / thread_group — KHÔNG vào children
    list_add_tail_rcu(&p->thread_group, &p->group_leader->thread_group);
}
```

→ **Cây process** (qua `children`/`sibling`) = tree các thread-group leader. Thread không nằm trong cây; chúng nằm trong `signal->thread_head`. → **Danh sách toàn cục** `init_task.tasks` là thứ khác — `for_each_process(p)` duyệt nó (mọi leader), `for_each_process_thread(p, t)` duyệt cả thread.

Toàn bộ cây được bảo vệ bởi **`tasklist_lock`** (rwlock). Ghi (`copy_process`, `exit_notify`, `ptrace_link`, `de_thread`): `write_lock_irq(&tasklist_lock)`. Đọc: `read_lock` hoặc `rcu_read_lock()` — nên `real_parent`/`parent` có annotation `__rcu`.

---

## 1. Năm trường

### `real_parent` (`struct task_struct __rcu *`)

Task đã **`fork()`** ra nó. Với `clone(CLONE_PARENT)` → `real_parent` = `real_parent` của task gọi clone (con "anh em" với cloner, không phải con của cloner). **Chỉ đổi** khi reparent (cha chết). `getppid()` dùng `task_tgid_vnr(real_parent)` (khi không bị trace).

### `parent` (`struct task_struct __rcu *`)

Cha **hiệu lực** — nơi nhận `SIGCHLD` và làm `wait()`. Bằng `real_parent` **trừ khi** task đang bị `ptrace`:

```c
// kernel/ptrace.c
__ptrace_link(child, tracer):  child->parent = tracer;
                               list_add(&child->ptrace_entry, &tracer->ptraced);
                               child->ptrace = PT_PTRACED | ...
__ptrace_unlink(child):        child->parent = child->real_parent;   // khôi phục
                               list_del_init(&child->ptrace_entry);
                               child->ptrace = 0;
```

Khi gdb `PTRACE_ATTACH` một process: `process->parent = gdb`, `process->real_parent` **giữ nguyên**. Tín hiệu stop/trap và `wait()` cho process đó đi tới gdb; cha thật `wait()` bị block (trừ `__WALL`). `PTRACE_DETACH` / gdb chết → `parent` khôi phục về `real_parent`.

### `children` (`struct list_head`)

Đầu danh sách con của task này. Mỗi con móc vào qua `child->sibling`.

### `sibling` (`struct list_head`)

Nút của task này **trong `parent->children`**. Mọi task cùng cha tạo thành danh sách anh em. `list_for_each_entry(c, &parent->children, sibling)` duyệt hết con.

### `ptraced` (`struct list_head`)

Đầu danh sách các task mà task này **đang trace**. Mỗi tracee móc vào qua `tracee->ptrace_entry`.

### `ptrace_entry` (`struct list_head`)

Nút của task này khi nó là tracee → trong `tracer->ptraced`. Cũng được **tái dùng** làm nút của danh sách `dead` (zombie chờ `release_task`) trong `forget_original_parent`.

Kèm: `group_leader` — leader của thread group; process đơn luồng thì `group_leader == self`.

---

## 2. Cây ví dụ

```
init (PID 1)
  real_parent = kernel_init's parent (≈ swapper/PID 0)
  children: bash
     │
bash (PID 100)
  real_parent = init ;  parent = init
  sibling: nút trong init->children
  children: program
     │
program (PID 200)
  real_parent = bash ;  parent = bash
  sibling: nút trong bash->children
  children: child1, child2
     ├─ child1 (PID 201)  real_parent = program ; parent = program ; sibling ⇄ child2
     └─ child2 (PID 202)  real_parent = program ; parent = program ; sibling ⇄ child1
```

Truy vấn:

|Câu hỏi|Cách trả lời|
|---|---|
|Cha của `program`?|`program->real_parent` = bash|
|Con của `program`?|`list_for_each_entry(c, &program->children, sibling)` → child1, child2|
|Anh em của `child1`?|duyệt `program->children` → child1, child2|
|`getppid()` trong `program`?|`task_tgid_vnr(program->real_parent)` = 100|
|Mọi process trên hệ thống?|`for_each_process(p)` — duyệt `init_task.tasks` (danh sách toàn cục, **không** phải cây)|

Lớp phủ `ptrace`: nếu `gdb` (PID 300) `PTRACE_ATTACH` `child1`:

- `child1->parent = gdb`, `child1->real_parent = program` (không đổi), `child1->ptrace != 0`.
- `gdb->ptraced` chứa `child1` (qua `child1->ptrace_entry`).
- Stop của `child1` báo `SIGCHLD` cho `gdb`; `program`'s `wait(child1)` block.

---

## 3. Orphan process là gì

**Orphan** = process mà cha đã exit nhưng **nó vẫn đang chạy**. `real_parent`/`parent` của nó không còn trỏ vào task sống.

Linux **không bao giờ** để dangling. Khi cha exit, `exit_notify(cha)` → `forget_original_parent(cha, &dead)` reparent mọi con (đang chạy hoặc zombie) sang một reaper.

**Vì sao không free orphan luôn?** Vì nó là process hợp lệ, còn chạy. Nhưng khi nó chết, _ai đó_ phải `wait()` nó — nếu không nó thành zombie vĩnh viễn. **init (PID 1)** chạy vòng lặp `wait()` vô hạn chính để reap orphan zombie. Reparent về init đảm bảo **mọi cái chết đều được thu**.

---

## 4. `forget_original_parent(father, &dead)` — reparent thế nào

Gọi từ `exit_notify(father)`, giữ `write_lock_irq(&tasklist_lock)`.

```c
static void forget_original_parent(struct task_struct *father, struct list_head *dead)
{
    if (!list_empty(&father->ptraced))
        exit_ptrace(father, dead);                    // (0) father đang trace ai → detach hết

    reaper = find_child_reaper(father, dead);         // (1)
    if (list_empty(&father->children))
        return;

    reaper = find_new_reaper(father, reaper);         // (2)

    list_for_each_entry(p, &father->children, sibling) {
        for_each_thread(p, t) {                       // (3) reparent MỌI thread của mỗi con
            RCU_INIT_POINTER(t->real_parent, reaper);
            if (likely(!t->ptrace))
                t->parent = t->real_parent;
            if (t->pdeath_signal)
                group_send_sig_info(t->pdeath_signal, SEND_SIG_NOINFO, t, PIDTYPE_TGID);  // (4)
        }
        if (!same_thread_group(reaper, father))
            reparent_leader(father, p, dead);         // (5)
    }
    list_splice_tail_init(&father->children, &reaper->children);   // (6)
}
```

### (1) `find_child_reaper(father, dead)`

```c
pid_ns = task_active_pid_ns(father);
reaper = pid_ns->child_reaper;                        // "init" của pid-namespace của father
if (reaper != father)
    return reaper;                                    // ◀── trường hợp thường: father không phải init → reaper = init

// father CHÍNH LÀ init của pid-ns:
reaper = find_alive_thread(father);                   // còn thread nào của init sống?
if (reaper) { pid_ns->child_reaper = reaper; return reaper; }   // → nó thành init mới

// cả thread group của init đang chết:
zap_pid_ns_processes(pid_ns);                         // SIGKILL MỌI process trong ns → ns bị dọn
return father;                                         // (trong container: cả container kết thúc)
```

Bình thường: reaper = `pid_ns->child_reaper` = **PID 1 của pid-namespace của process** — `init` toàn cục trên host, hoặc **init của container** bên trong container.

(Trên host, giết PID 1 bị chặn ở `do_exit`: `if (is_global_init(tsk)) panic("Attempted to kill init!")` — nên nhánh `zap_pid_ns_processes` cho `init_pid_ns` không xảy ra.)

### (2) `find_new_reaper(father, child_reaper)` — subreaper

```c
thread = find_alive_thread(father);                   // thread KHÁC của father còn sống?
if (thread) return thread;                            // → reparent NỘI BỘ (không cần báo ai)

if (father->signal->has_child_subreaper) {            // tổ tiên nào đó đã prctl(PR_SET_CHILD_SUBREAPER)
    ns_level = task_pid(father)->level;
    for (reaper = father->real_parent;
         task_pid(reaper)->level == ns_level;         // ở TRONG CÙNG pid-namespace
         reaper = reaper->real_parent) {
        if (reaper == &init_task) break;
        if (!reaper->signal->is_child_subreaper) continue;
        thread = find_alive_thread(reaper);
        if (thread) return thread;                    // ◀── subreaper tổ tiên GẦN NHẤT
    }
}
return child_reaper;                                   // fallback: init của pid-ns
```

`prctl(PR_SET_CHILD_SUBREAPER, 1)` đánh dấu process là "subreaper": con cháu mồ côi được reparent về **nó** thay vì về init. Dùng bởi service manager (systemd `--user`, `tmux`, `upstart`) để `wait()` được cháu mà cha trực tiếp của cháu đã chết. Cờ `has_child_subreaper` lan xuống cây để kiểm nhanh.

### (3) Reparent mọi thread

Vòng lặp reparent `p` (một con — luôn là leader) **và mọi thread `t` trong nhóm của `p`** — `t->real_parent = reaper` cho tất cả. `if (!t->ptrace) t->parent = t->real_parent` — nếu không bị trace, `parent` theo `real_parent`.

### (4) `pdeath_signal`

Nếu orphan đã gọi `prctl(PR_SET_PDEATHSIG, sig)` → nó nhận `sig` **ngay bây giờ** ("cha mày vừa chết"). (Bị xoá khi `execve` một binary đổi đặc quyền.)

### (5) `reparent_leader(father, p, dead)`

Nếu con `p` **đã là zombie** (`EXIT_ZOMBIE && thread_group_empty && !ptrace`):

```c
p->exit_signal = SIGCHLD;                             // ép SIGCHLD (phòng khi = -1 hoặc custom)
if (do_notify_parent(p, p->exit_signal)) {           // báo cho reaper (init) — init có thể autoreap
    p->exit_state = EXIT_DEAD;
    list_add(&p->ptrace_entry, dead);                 // xếp lịch release_task ngay
}
kill_orphaned_pgrp(p, father);
```

→ Zombie mồ côi được **báo lại cho init**; vòng `wait()` của init reap nó → **không zombie leak**. Nếu `same_thread_group(reaper, father)` (thread khác của process đang chết đứng ra reap) → không cần `do_notify_parent` (bàn giao nội bộ).

### (6) `list_splice_tail_init(&father->children, &reaper->children)`

Dời **cả** danh sách `children` sang `reaper->children` trong một thao tác O(1). Giờ mọi nút `sibling` của orphan nằm trong `reaper->children`.

Về `exit_notify`: danh sách `dead` (zombie chuyển `EXIT_DEAD` trong lúc reparent) được duyệt và `release_task`.

---

## 5. Timeline: `bash` exit trước `program`

1. `bash` → `exit_group()` → `do_exit` → `exit_notify(bash)`.
2. `forget_original_parent(bash, &dead)`:
    - `find_child_reaper(bash)`: bash ≠ init → `reaper = init`.
    - `find_new_reaper(bash, init)`: bash không còn thread; giả sử không có subreaper tổ tiên → `reaper = init`.
    - Với `program` (con duy nhất của bash): `program->real_parent = init`; `program->parent = init` (không trace). Nếu `program` có `PR_SET_PDEATHSIG` → nhận tín hiệu đó ngay.
    - `program` còn **sống** (không zombie) → `reparent_leader` không làm gì.
    - `list_splice`: `init->children` giờ chứa `program`.
3. `bash->exit_state = EXIT_ZOMBIE`; `do_notify_parent(bash, SIGCHLD)` → `SIGCHLD` tới init + `__wake_up_parent`. Vòng `wait()` của init reap bash.
4. `program` chạy tiếp, giờ là **orphan reparent về init**. `getppid()` trong `program` trả **1**.
5. Sau này `program` exit → `do_notify_parent(program, SIGCHLD)` → `SIGCHLD` tới **init** → vòng `wait()` của init reap program. Không zombie leak.
6. `child1`/`child2` là con của `program`, **không** bị ảnh hưởng bởi bash chết. Nếu `program` exit trước chúng → chúng reparent về init trong `forget_original_parent` của `program`.

---

## 6. Các quy tắc rút ra

|Tình huống|Kết quả|
|---|---|
|Cha `fork` con, cả hai sống|`child->real_parent = child->parent = forking task`|
|Con bị `ptrace_attach` bởi X|`child->parent = X`; `child->real_parent` không đổi|
|Cha (không phải init, không subreaper) exit trước con|con reparent về **init của pid-namespace**; `child->parent = child->real_parent = init`|
|Cha exit, có tổ tiên subreaper còn sống trong cùng pid-ns|con reparent về **subreaper gần nhất**|
|Cha đa luồng, một thread exit nhưng nhóm còn thread khác|con reparent về **thread còn sống của cùng nhóm** (nội bộ)|
|Con đã là **zombie** khi bị reparent|`reparent_leader` → `do_notify_parent(reaper)` → init reap → không leak|
|Con có `PR_SET_PDEATHSIG`|nhận tín hiệu đó ngay khi bị reparent|
|**init của container** (PID 1 trong pid-ns) chết|`zap_pid_ns_processes` → `SIGKILL` mọi process trong ns → container kết thúc|
|**init toàn cục** (PID 1 trên host) chết|`panic("Attempted to kill init!")` ở `do_exit`|

---

## 7. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|`forget_original_parent` / `find_new_reaper` / `reparent_leader`|như mô tả|y hệt|
|`find_child_reaper` + `zap_pid_ns_processes`|có|có|
|`PR_SET_CHILD_SUBREAPER` (`prctl`)|có (từ 3.4)|có; + `PR_GET_CHILD_SUBREAPER`|
|`real_parent`/`parent` là `__rcu`|có|có|
|`children`/`sibling` chỉ leader|có|có (maple tree không đụng phần này)|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `forget_original_parent`, `find_new_reaper` giống hệt như trên. Cơ chế `real_parent` vs `parent`, cây chỉ-leader, reparent về subreaper/init — **giống hệt 5.4**.