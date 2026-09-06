# MODULE 12 — Linux Scheduler (CFS) trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/scheduler-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/scheduler-arm64.md) (4 sơ đồ: máy trạng thái task, cấu trúc `rq`/`cfs_rq`/`sched_entity`, rbtree theo `vruntime`, trace `read()` pipe rỗng đầy đủ).

**File nguồn**: `kernel/sched/core.c` (`schedule`, `__schedule`, `pick_next_task`, `try_to_wake_up`, `wake_up_process`, `ttwu_do_activate`, `activate_task`/`deactivate_task`, `check_preempt_curr`, `scheduler_tick`), `kernel/sched/fair.c` (`enqueue_task_fair`, `pick_next_task_fair`, `update_curr`, `place_entity`, `__enqueue_entity`, `check_preempt_wakeup`, `check_preempt_tick`, `calc_delta_fair`, `sched_slice`), `kernel/sched/sched.h` (`struct rq`, `struct cfs_rq`), `include/linux/sched.h` (`struct sched_entity`). ARM64: `arch/arm64/kernel/entry.S` (`ret_to_user` kiểm `TIF_NEED_RESCHED`), `arch/arm64/kernel/smp.c` (`smp_send_reschedule` → GIC SGI), arch generic timer → `scheduler_tick`.

---

## 0. Ý chính

CFS ("Completely Fair Scheduler") có một bất biến duy nhất: **luôn chạy task có `vruntime` nhỏ nhất**. `vruntime` = thời gian CPU đã tiêu, **chia tỉ lệ theo weight** (từ `nice`). Task chạy → `vruntime` tăng → task khác thành "nhỏ nhất" → preempt. Theo thời gian mọi `vruntime` hội tụ → công bằng.

Task được giữ trong một **cây đỏ-đen** (rbtree) sắp theo `vruntime`; nút trái nhất = task chạy tiếp, lấy trong O(1) nhờ con trỏ `rb_leftmost` đã cache.

CFS **gần như không có phần ARM64-riêng**. Phần arch chỉ là: `context_switch` (`switch_mm` + `cpu_switch_to`), timer tick (arch generic timer đẩy `scheduler_tick`), IPI reschedule (GIC SGI), và chỗ kiểm `TIF_NEED_RESCHED` trong `entry.S`.

---

## 1. Ba trạng thái đầu tiên

|State|Giá trị|Ý nghĩa|Thức bởi|
|---|---|---|---|
|**`TASK_RUNNING`**|0|"Runnable" — **đang chạy trên CPU** (là `rq->curr`) **hoặc chờ trong runqueue** (trong rbtree). Cùng một state, khác chỗ ở.|—|
|**`TASK_INTERRUPTIBLE`**|1|Ngủ, chờ một điều kiện. Thức khi **sự kiện xảy ra HOẶC có tín hiệu**. Dùng cho hầu hết chờ có thể huỷ (`read()` block, `wait_event_interruptible`, `mutex_lock_interruptible`).|`wake_up_*` hoặc signal|
|**`TASK_UNINTERRUPTIBLE`**|2|Ngủ, **chỉ** thức khi sự kiện xảy ra — tín hiệu (kể cả `SIGKILL`) **không** cắt được. Dùng cho chờ ngắn/nguyên tử không được để dở (một số I/O đĩa, `mutex_lock`). Task loại này xuất hiện là `D` trong `ps` và tính vào load average.|chỉ `wake_up_*`|

Biến thể: **`TASK_KILLABLE`** = `TASK_UNINTERRUPTIBLE` nhưng thức nếu có **fatal signal** (`SIGKILL`) — thoả hiệp giữa "an toàn không bị cắt bậy" và "vẫn giết được".

Còn: `__TASK_STOPPED` (4, `SIGSTOP`), `__TASK_TRACED` (8, ptrace), và trong `exit_state`: `EXIT_ZOMBIE` / `EXIT_DEAD` (Module 16–18).

---

## 2. Cấu trúc dữ liệu

### `struct rq` — runqueue per-CPU

```c
struct rq {
    raw_spinlock_t       __lock;
    unsigned int         nr_running;     // tổng task runnable trên CPU này (mọi class)
    struct task_struct  *curr;           // task ĐANG chạy trên CPU này
    struct task_struct  *idle;           // task idle (chạy khi nr_running == 0)
    struct task_struct  *stop;
    u64                  clock;           // đồng hồ runqueue (ns), cập nhật mỗi lần vào scheduler
    struct cfs_rq        cfs;             // ◀── phần CFS (nhúng)
    struct rt_rq         rt;              // phần realtime
    struct dl_rq         dl;              // phần deadline
    struct mm_struct    *prev_mm;
    ...
};
```

`this_rq()` = runqueue của CPU hiện tại. `cpu_rq(cpu)` = của CPU bất kỳ. `task_rq(p)` = `cpu_rq(task_cpu(p))`.

### `struct cfs_rq` — phần CFS

```c
struct cfs_rq {
    struct load_weight    load;           // TỔNG se->load.weight của mọi entity trong hàng đợi
    unsigned int          nr_running;
    u64                   min_vruntime;   // sàn đơn điệu tăng — "điểm 0" của cây
    struct rb_root_cached  tasks_timeline; // rbtree + con trỏ leftmost đã cache
    struct sched_entity   *curr;          // entity ĐANG chạy (đã RÚT khỏi cây)
    struct rq             *rq;            // con trỏ ngược
    ...
};
```

`min_vruntime` quan trọng: nó là "mốc" để (a) làm khoá tương đối trong cây (`se->vruntime - min_vruntime`, tránh tràn `u64`), (b) làm điểm xuất phát cho task mới / task vừa ngủ dậy. Nó **chỉ tăng**, không bao giờ lùi.

### `struct sched_entity` — nhúng trong `task_struct` (`p->se`)

```c
struct sched_entity {
    struct load_weight    load;            // weight từ nice: sched_prio_to_weight[nice+20]
                                           //   nice -20 → 88761 ; nice 0 → 1024 ; nice +19 → 15
    struct rb_node        run_node;         // nút trong cfs_rq->tasks_timeline
    unsigned int          on_rq;
    u64                   exec_start;       // mốc bắt đầu lượt chạy hiện tại
    u64                   sum_exec_runtime; // tổng thời gian CPU THỰC (ns)
    u64                   vruntime;         // ◀── thời gian ẢO — KHOÁ SẮP XẾP
    u64                   prev_sum_exec_runtime;
    struct cfs_rq        *cfs_rq;           // đang nằm trên cfs_rq nào
    struct sched_entity  *parent;           // cho group scheduling (cgroup cpu)
};
```

### `vruntime` — "virtual runtime"

Đơn vị nanosecond, nhưng **được co giãn theo weight**:

```c
// calc_delta_fair(delta_exec, se): thời gian THỰC → thời gian ẢO
vruntime_delta = delta_exec * NICE_0_LOAD / se->load.weight;    // NICE_0_LOAD = 1024
```

- Task **nice 0** (weight 1024): `vruntime` tăng đúng bằng wall-clock.
- Task **nice −5** (weight ~3121): `vruntime` tăng **chậm ~3×** → ở lâu bên trái cây → được chọn nhiều hơn → nhiều CPU hơn.
- Task **nice +10** (weight ~110): `vruntime` tăng **nhanh ~9×** → nhanh bị đẩy sang phải → ít CPU.

Mỗi nấc `nice` ≈ chênh 1.25× weight ≈ **±10% CPU**.

### rbtree

`cfs_rq->tasks_timeline` là `struct rb_root_cached`: cây đỏ-đen + con trỏ `rb_leftmost`. Khoá so sánh = `entity_key(cfs_rq, se) = se->vruntime - cfs_rq->min_vruntime`. Nút **trái nhất** = `vruntime` nhỏ nhất = task "đói CPU nhất" = chạy tiếp. `__pick_first_entity()` = đọc `rb_leftmost` → **O(1)**.

**Task đang chạy KHÔNG nằm trong cây** — nó bị rút ra làm `cfs_rq->curr` khi được chọn (`set_next_entity` → `__dequeue_entity`), và chèn lại (`put_prev_entity` → `__enqueue_entity`) khi bị switch away.

---

## 3. `wake_up_process()` → `try_to_wake_up()`

```c
int wake_up_process(struct task_struct *p)
{
    return try_to_wake_up(p, TASK_NORMAL, 0);   // TASK_NORMAL = INTERRUPTIBLE | UNINTERRUPTIBLE
}
```

`try_to_wake_up(p, state, wake_flags)` (`kernel/sched/core.c`):

```c
raw_spin_lock_irqsave(&p->pi_lock, flags);
if (!(p->state & state))          goto out;      // p không ở state ta quan tâm → thôi
success = 1;
if (p->on_rq && ttwu_runnable(p, wake_flags)) goto unlock;  // đã trong runqueue (race) → chỉ resched

smp_rmb();
smp_cond_load_acquire(&p->on_cpu, !VAL);         // ĐỢI p rời hẳn CPU cũ (nếu đang switch out)

cpu = select_task_rq(p, p->wake_cpu, SD_BALANCE_WAKE, wake_flags);   // chọn CPU đích (có thể khác!)

ttwu_queue(p, cpu, wake_flags);
  → ttwu_do_activate(rq, p, wake_flags, rf):
        activate_task(rq, p, ENQUEUE_WAKEUP | ENQUEUE_NOCLOCK);
          → enqueue_task_fair → enqueue_entity:
                place_entity(cfs_rq, &p->se, initial=0);   // ◀── điều chỉnh vruntime khi thức
                update_curr(cfs_rq);
                __enqueue_entity(cfs_rq, &p->se);          // ◀── chèn vào rbtree
                cfs_rq->nr_running++; cfs_rq->load += p->se.load.weight;
        p->on_rq = TASK_ON_RQ_QUEUED;
        ttwu_do_wakeup(rq, p, wake_flags, rf):
              check_preempt_curr(rq, p, wake_flags);        // → check_preempt_wakeup → có thể resched_curr
              p->state = TASK_RUNNING;                      // ◀── giờ mới "runnable"
```

**`place_entity(cfs_rq, se, initial=0)`** cho task vừa ngủ dậy (`initial=0` = wakeup, không phải fork):

```c
u64 vruntime = cfs_rq->min_vruntime;
if (initial && sched_feat(START_DEBIT))
    vruntime += sched_vslice(cfs_rq, se);        // fork: phạt ~1 slice (Module 4)
else {                                            // wakeup:
    unsigned long thresh = sysctl_sched_latency;  // 6 ms
    if (sched_feat(GENTLE_FAIR_SLEEPERS)) thresh >>= 1;   // → 3 ms
    vruntime -= thresh;
}
se->vruntime = max_vruntime(se->vruntime, vruntime);
```

→ Task ngủ lâu có `se->vruntime` << `min_vruntime`. Nếu cứ dùng giá trị cũ, nó sẽ "tích luỹ tín dụng vô hạn" và độc chiếm CPU khi thức. CFS **kẹp** nó về `min_vruntime - 6ms`: đủ nhỏ để **được chạy rất sớm** (đây là "sleeper boost" — tốt cho interactive: bàn phím, chuột, GUI phản hồi nhanh), nhưng không quá nhỏ để độc chiếm.

**`check_preempt_wakeup(rq, p)`**: nếu `p->se.vruntime` vượt trước `curr->se.vruntime` một lượng > `wakeup_gran(se)` (dẫn xuất từ `sysctl_sched_wakeup_granularity = 1ms`) → **`resched_curr(rq)`**:

- set `TIF_NEED_RESCHED` trên `rq->curr`.
- nếu `rq` là **CPU khác** → `smp_send_reschedule(cpu)` → **ARM64**: `arch/arm64/kernel/smp.c` gửi **GIC SGI** (IPI) → CPU đó nhận `IPI_RESCHEDULE`, chạy `scheduler_ipi()` → sẽ `schedule()` khi ra khỏi IRQ.

**Wakeup KHÔNG chạy `p` ngay.** Nó chỉ: (1) chèn `p` vào rbtree, (2) `p->state = TASK_RUNNING`, (3) có thể set `TIF_NEED_RESCHED`. Việc đổi sang `p` xảy ra ở `schedule()` kế tiếp trên CPU đích.

---

## 4. `schedule()` → `__schedule()`

```c
asmlinkage __visible void __sched schedule(void)
{
    struct task_struct *tsk = current;
    sched_submit_work(tsk);            // flush plug I/O trước khi ngủ (tránh deadlock)
    do {
        preempt_disable();
        __schedule(false);            // false = ngủ tự nguyện (không phải preempt)
        sched_preempt_enable_no_resched();
    } while (need_resched());
    sched_update_worker(tsk);
}
```

`__schedule(bool preempt)`:

```c
static void __sched notrace __schedule(bool preempt)
{
    struct task_struct *prev, *next;
    struct rq *rq = cpu_rq(smp_processor_id());

    prev = rq->curr;
    schedule_debug(prev, preempt);     // kiểm STACK_END_MAGIC, atomic-sleep...
    rq_lock(rq, &rf);
    update_rq_clock(rq);

    prev_state = prev->state;
    if (!preempt && prev_state) {                      // prev sắp NGỦ (state != TASK_RUNNING)
        if (signal_pending_state(prev_state, prev)) {
            prev->state = TASK_RUNNING;                // có signal → KHÔNG ngủ, ở lại runnable
        } else {
            deactivate_task(rq, prev, DEQUEUE_SLEEP | DEQUEUE_NOCLOCK);  // ◀── GỠ khỏi runqueue
            prev->on_rq = 0;
        }
    }

    next = pick_next_task(rq, prev, &rf);
    clear_tsk_need_resched(prev);
    clear_preempt_need_resched();

    if (likely(prev != next)) {
        rq->curr = next;
        rq = context_switch(rq, prev, next, &rf);      // ◀── Module 9/13
    } else {
        rq_unlock_irq(rq, &rf);
    }
}
```

**Điểm then chốt**: task ngủ tự nguyện bị `deactivate_task()` gỡ khỏi runqueue **ngay trong `__schedule`**, không phải trong code chờ. Code chờ (`wait_event`, `pipe_read`…) chỉ `set_current_state(TASK_INTERRUPTIBLE)`. `__schedule` đọc `prev->state`, thấy khác 0, rồi gỡ. Đây là "control dependency" mà comment trong source nhấn mạnh — và là lý do `set_current_state()` **phải** đặt trước khi kiểm điều kiện (nếu không, waker chen vào giữa → task ngủ vĩnh viễn).

`signal_pending_state(prev_state, prev)`: nếu `prev_state == TASK_INTERRUPTIBLE` (hoặc `TASK_KILLABLE` + fatal) và có signal chờ → **không ngủ**, đặt lại `TASK_RUNNING`. Đây là cách một `read()` block bị `Ctrl+C` cắt trả `-EINTR`/`-ERESTARTSYS`.

### `pick_next_task()`

```c
static inline struct task_struct *
pick_next_task(struct rq *rq, struct task_struct *prev, struct rq_flags *rf)
{
    // fast path: nếu mọi task đều là fair và có task fair → gọi thẳng CFS
    if (likely(prev->sched_class <= &fair_sched_class &&
               rq->nr_running == rq->cfs.h_nr_running))
        return pick_next_task_fair(rq, prev, rf);

    // slow path: duyệt class theo thứ tự ưu tiên
    for_each_class(class) {
        p = class->pick_next_task(rq);   // stop → dl → rt → fair → idle
        if (p) return p;
    }
}
```

Thứ tự class: `stop_sched_class` > `dl_sched_class` (SCHED_DEADLINE) > `rt_sched_class` (SCHED_FIFO/RR) > `fair_sched_class` (SCHED_NORMAL/BATCH) > `idle_sched_class`. RT task luôn thắng CFS task.

### `pick_next_task_fair()`

```c
do {
    se = pick_next_entity(cfs_rq, curr);   // thường = __pick_first_entity (leftmost)
                                           //   trừ khi có next-buddy / skip-buddy / hoặc
                                           //   check hệ số công bằng (leftmost quá xa curr?)
    set_next_entity(cfs_rq, se);            // rút se khỏi cây, se->exec_start = now
    cfs_rq = group_cfs_rq(se);             // nếu là group entity → xuống cfs_rq con (cgroup)
} while (cfs_rq);
p = task_of(se);
```

---

## 5. `scheduler_tick()` — cưỡng chế công bằng theo thời gian

Timer ARM64 (arch generic timer, `CNTV_TVAL_EL0`) mỗi `1/HZ` giây → `tick_handle_periodic` / hrtimer tick → `update_process_times` → **`scheduler_tick()`** (`kernel/sched/core.c`):

```c
void scheduler_tick(void)
{
    struct rq *rq = this_rq();
    rq_lock(rq, &rf);
    update_rq_clock(rq);
    curr->sched_class->task_tick(rq, curr, 0);   // fair → task_tick_fair
    rq_unlock(rq, &rf);
    ...
}
```

`task_tick_fair` → `entity_tick`:

```c
update_curr(cfs_rq);        // curr->se.vruntime += calc_delta_fair(now - exec_start, curr)
                            // curr->se.sum_exec_runtime += (now - exec_start)
                            // update_min_vruntime(cfs_rq)
if (cfs_rq->nr_running > 1)
    check_preempt_tick(cfs_rq, curr);
```

`check_preempt_tick`:

```c
ideal_runtime = sched_slice(cfs_rq, curr);   // slice wall-clock của curr trong chu kỳ hiện tại
                                             //   = __sched_period(nr) * curr.weight / cfs_rq.load.weight
delta_exec = curr->sum_exec_runtime - curr->prev_sum_exec_runtime;
if (delta_exec > ideal_runtime) {
    resched_curr(rq);                        // curr chạy quá phần của nó → set TIF_NEED_RESCHED
    return;
}
if (delta_exec < sysctl_sched_min_granularity) return;   // chưa chạy đủ 0.75ms → để yên
// nếu leftmost đang bị "đói" hơn curr một khoảng > ideal_runtime → resched
se = __pick_first_entity(cfs_rq);
if ((s64)(curr->vruntime - se->vruntime) > ideal_runtime)
    resched_curr(rq);
```

`__sched_period(nr_running)`:

```c
if (nr_running > sched_nr_latency /* = 8 */)
    return nr_running * sysctl_sched_min_granularity;   // chu kỳ giãn ra: mỗi task tối thiểu 0.75ms
else
    return sysctl_sched_latency;                        // 6ms — mọi task chạy 1 lượt trong 6ms
```

→ ≤ 8 task runnable: mỗi task chạy 1 lượt mỗi 6ms. > 8 task: mỗi task tối thiểu 0.75ms/lượt.

**`resched_curr` không tự đổi task.** Nó chỉ set `TIF_NEED_RESCHED`. Việc `schedule()` xảy ra khi:

- `ret_to_user` (về EL0) thấy `TIF_NEED_RESCHED` → gọi `schedule()` (Module 3).
- `preempt_enable()` với `CONFIG_PREEMPT` và `TIF_NEED_RESCHED` set → `preempt_schedule()`.
- ARM64: ra khỏi IRQ ở EL1 với `CONFIG_PREEMPT` → `arm64_preempt_schedule_irq()`.
- Task tự gọi `schedule()` (block).

---

## 6. Trace đầy đủ: `read()` trên pipe rỗng

### ① Running

P là `rq0->curr` trên CPU 0. `p->state = TASK_RUNNING`, `p->on_rq = 1`, `p->on_cpu = 1`. `p->se` **không** trong rbtree — nó là `cfs_rq->curr`. Mỗi tick: `update_curr(cfs_rq)` → `P.se.vruntime += calc_delta_fair(delta_exec, &P.se)`.

### ② Sleep

```c
// pipe_read() → wait_event_interruptible(pipe->rd_wait, pipe_readable(pipe))
// bung ra thành:
for (;;) {
    prepare_to_wait(&pipe->rd_wait, &wait, TASK_INTERRUPTIBLE);   // set_current_state + add_wait_queue
    if (pipe_readable(pipe)) break;                                // vẫn rỗng
    if (signal_pending(current)) { ret = -ERESTARTSYS; break; }
    schedule();                                                    // ◀── nhường CPU
}
finish_wait(&pipe->rd_wait, &wait);
```

`schedule()` → `__schedule(false)`:

- `prev = P`, `prev_state = TASK_INTERRUPTIBLE` (≠ 0), không có signal.
- `deactivate_task(rq0, P, DEQUEUE_SLEEP | DEQUEUE_NOCLOCK)` → `dequeue_task_fair` → `dequeue_entity` → `__dequeue_entity(cfs_rq, &P.se)`: **gỡ `P.se` khỏi rbtree**. `cfs_rq->nr_running--`. `cfs_rq->load.weight -= P.se.load.weight`. `P->on_rq = 0`.
- `next = pick_next_task(rq0, P)` → `__pick_first_entity` = Q.
- `rq0->curr = Q`; `context_switch(rq0, P, Q)` → `switch_mm(Q->mm, ASID_Q)` + `cpu_switch_to(P, Q)`.

P giờ: off-CPU, `state = TASK_INTERRUPTIBLE`, **không nằm trong runqueue nào**. Kernel stack của P đóng băng giữa `pipe_read` (call chain: `pipe_read` → `schedule` → `__schedule` → `context_switch` → `cpu_switch_to` đã lưu `sp`/`pc`).

### ③ Wakeup

Task W ghi vào pipe → `pipe_write` → dữ liệu có → `wake_up_interruptible(&pipe->rd_wait)` → `__wake_up_common` → duyệt wait queue → gọi hàm đánh thức của phần tử `wait` → `try_to_wake_up(P, TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE, 0)`:

- `p->pi_lock` giữ. `P->state & TASK_INTERRUPTIBLE` ✓ → `success = 1`.
- `smp_cond_load_acquire(&P->on_cpu, !VAL)` — P đã rời CPU 0 nên qua ngay.
- `cpu = select_task_rq(P, P->wake_cpu, SD_BALANCE_WAKE, 0)` — wake-affine: thường chọn CPU 0 (cache của P còn nóng) hoặc một CPU đang idle.
- `ttwu_queue(P, cpu)` → `ttwu_do_activate`:
    - `activate_task` → `enqueue_task_fair` → `enqueue_entity`:
        - `place_entity(cfs_rq, &P.se, 0)`: `thresh = 6ms` → `P.se.vruntime = max(P.se.vruntime, cfs_rq->min_vruntime - 6ms)`. P ngủ lâu ⇒ bị kẹp về `min_vruntime - 6ms` ⇒ **sẽ là leftmost**.
        - `update_curr(cfs_rq)`.
        - `__enqueue_entity(cfs_rq, &P.se)` → chèn vào rbtree (thành nút trái nhất).
        - `nr_running++`, `load.weight += P.se.load.weight`.
    - `P->on_rq = TASK_ON_RQ_QUEUED`.
    - `ttwu_do_wakeup`:
        - `check_preempt_curr(rq, P, 0)` → `check_preempt_wakeup(rq, P)`: `P.se.vruntime` vượt trước `curr.se.vruntime` > `wakeup_gran` → **`resched_curr(rq)`** → set `TIF_NEED_RESCHED` trên `rq->curr`. Nếu `rq` là CPU khác → `smp_send_reschedule(cpu)` → **GIC SGI (IPI)**.
        - `P->state = TASK_RUNNING`.

### ④ Runnable

P: `TASK_RUNNING`, trong rbtree của CPU `cpu`, là nút trái nhất. **Chưa chạy.**

### ⑤ Scheduler select

Trên CPU `cpu`, task hiện tại (gọi là Q) tới một điểm `schedule()`:

- `ret_to_user` thấy `TIF_NEED_RESCHED` → `schedule()`, hoặc
- `preempt_enable()` (nếu `CONFIG_PREEMPT`) với `TIF_NEED_RESCHED`, hoặc
- Q tự block.

`__schedule()` → `pick_next_task_fair` → `__pick_first_entity(cfs_rq)` = **P** → `next = P`. `set_next_entity` rút `P.se` khỏi cây, `P.se.exec_start = now`.

### ⑥ Context switch

`rq->curr = P`; `context_switch(rq, Q, P, &rf)` (Module 9/13):

- `switch_mm_irqs_off(Q->active_mm, P->mm, P)` → `check_and_switch_context(P->mm)`:
    - ASID của `P->mm` còn hạn (chưa rollover) → fast path, chỉ set `active_asids[cpu]`.
    - `cpu_do_switch_mm(P->mm->pgd, P->mm)` → `msr ttbr0_el1, phys(P->pgd)` ; `msr ttbr1_el1, swapper | ASID_P << 48`.
- `switch_to(Q, P, Q)` → `__switch_to(Q, P)`:
    - `fpsimd_thread_switch(P)`, `tls_thread_switch(P)` (nạp `P->thread.uw.tp_value` vào `TPIDR_EL0`), `entry_task_switch(P)`...
    - `cpu_switch_to(Q, P)`: lưu `cpu_context` của Q; nạp của P → `sp` = kernel stack P (đóng băng giữa `pipe_read`), `pc` = "lệnh sau `bl cpu_switch_to` trong `__switch_to`" của lần P bị switch out; `msr sp_el0, P` (current = P); `ret`.
- `finish_task_switch(Q)` chạy trong ngữ cảnh P: `raw_spin_unlock_irq(&rq->lock)`, `mmdrop` nếu Q là kernel thread, `put_task_struct(Q)` nếu Q `TASK_DEAD`.

### ⑦ Running

`__switch_to` return → `context_switch` return → `__schedule` return → `schedule()` return → về `pipe_read` **ngay sau lệnh `schedule()`**:

- vòng lặp: `prepare_to_wait(..., TASK_INTERRUPTIBLE)` → `pipe_readable(pipe)` giờ **đúng** (có data) → `break`.
- `finish_wait()` → `set_current_state(TASK_RUNNING)`, `remove_wait_queue`.
- `pipe_read` copy dữ liệu ra buffer user, trả số byte.
- `sys_read` return count → `ret_to_user` → `kernel_exit` → `eret` → P ở EL0, `read()` trả về.

---

## 7. `nice` → weight → CPU share (bảng nhanh)

|nice|weight (`sched_prio_to_weight`)|tốc độ `vruntime` (so nice 0)|CPU share tương đối|
|---|---|---|---|
|−20|88761|×0.0115 (rất chậm)|~88× nice 0|
|−5|3121|×0.33|~3×|
|**0**|**1024**|**×1.0**|**1×**|
|+5|335|×3.06|~1/3×|
|+19|15|×68|~1/68×|

Hai task nice 0 + nice 5 cùng chạy → tỉ lệ CPU ≈ `1024 : 335` ≈ **75% : 25%**.

---

## 8. Phần ARM64-riêng (rất ít)

|Việc|Generic|ARM64|
|---|---|---|
|Chọn task, rbtree, vruntime|`kernel/sched/{core,fair}.c`|— (không đụng)|
|`context_switch`|`context_switch()`|`switch_mm` → `check_and_switch_context` (ASID), `cpu_do_switch_mm` (TTBR0/1) · `__switch_to` → `cpu_switch_to`|
|Timer tick đẩy `scheduler_tick`|`update_process_times`|arch generic timer (`CNTV`), `arch/arm64/kernel/time.c`|
|IPI reschedule|`smp_send_reschedule()`|`arch/arm64/kernel/smp.c` → GIC SGI (`IPI_RESCHEDULE`)|
|Kiểm `TIF_NEED_RESCHED`|—|`entry.S` `ret_to_user`, `arm64_preempt_schedule_irq`|
|`TIF_NEED_RESCHED` bit|`include/linux/thread_info.h` chung tên|`arch/arm64/include/asm/thread_info.h` (số bit cụ thể)|

---

## 9. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|`__schedule` đọc `prev->state`|trực tiếp `prev->state`|`prev_state = READ_ONCE(prev->__state)` (5.14, đổi tên `state`→`__state`)|
|`try_to_wake_up`|như mô tả|tách `ttwu_queue_wakelist` (đẩy wakeup qua IPI thay vì lấy rq->lock từ xa) — tinh chỉnh 5.8+|
|CFS core (vruntime, rbtree, place_entity)|như mô tả|**không đổi** tới 6.6 (EEVDF thay CFS ở 6.6)|
|`pick_next_task` fast path|có|có|
|Hằng số `sched_latency` 6ms...|như mô tả|không đổi|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `try_to_wake_up` có `ttwu_queue_wakelist`, còn lại giống. CFS (rbtree theo `vruntime`, `place_entity`, `check_preempt_tick`, `calc_delta_fair`) — **giống hệt 5.4**. (6.6+ mới thay bằng EEVDF — khác hẳn.)