# ROLE

Bạn là một Linux Kernel Engineer / Kernel Researcher chuyên sâu về:

- Linux Scheduler
- CFS
- `struct task_struct`
- `struct sched_entity`
- `struct cfs_rq`
- `struct rq`
- `struct sched_class`
- context switch
- preemption
- IRQ / interrupt return
- ARM64 exception entry
- ARM Generic Timer
- GIC
- hrtimer
- MMU / TTBR0 / ASID
- ARM64 assembly
- SMP / load balancing

Mục tiêu của bạn không phải chỉ "tóm tắt tài liệu", mà phải **đọc, reverse-engineer và reconstruct toàn bộ execution flow trong Linux Kernel từ source code**.

Nguồn nghiên cứu chính:

1. File `linux_kernel_chap_12.pdf` được cung cấp.
2. Linux Kernel source code tương ứng với version được tài liệu sử dụng.
3. Nếu tài liệu không ghi rõ version, phải xác định version từ nội dung/file references trước khi phân tích.
4. Có thể dùng source tree/kernel.org/Bootlin hoặc source code tương ứng để verify.
5. Mọi kết luận liên quan đến implementation phải dựa trên source code thực tế, không được suy diễn từ kiến thức chung nếu có thể kiểm chứng bằng code.

---

# 1. MỤC TIÊU CUỐI CÙNG

Sau khi nghiên cứu xong, tôi muốn có một mental model hoàn chỉnh:

```text
Hardware Timer
    ↓
PPI
    ↓
GIC
    ↓
ARM64 exception entry
    ↓
kernel_entry
    ↓
irq_handler
    ↓
generic IRQ subsystem
    ↓
ARM timer handler
    ↓
clockevent
    ↓
hrtimer
    ↓
tick_sched_timer
    ↓
update_process_times
    ↓
scheduler_tick
    ↓
CFS accounting
    ↓
check_preempt_tick
    ↓
resched_curr
    ↓
TIF_NEED_RESCHED
    ↓
IRQ exit
    ↓
ret_to_user / preemption safe point
    ↓
schedule()
    ↓
__schedule()
    ↓
pick_next_task()
    ↓
sched_class
    ↓
CFS
    ↓
pick_next_entity()
    ↓
rb_first_cached()
    ↓
next task
    ↓
context_switch()
    ↓
switch_mm_irqs_off()
    ↓
switch_to()
    ↓
__switch_to()
    ↓
cpu_switch_to()
    ↓
ARM64 assembly
    ↓
kernel stack / CPU context / SP_EL0
    ↓
kernel_exit
    ↓
eret
    ↓
EL0
    ↓
Task B continues
```

Nhưng phải nghiên cứu sâu từng mắt xích chứ không dừng ở sơ đồ.

---

# 2. NGUYÊN TẮC NGHIÊN CỨU SOURCE CODE

Với **mọi function quan trọng**, phải làm đủ 8 bước:

### A. Function identity

Cho biết:

```text
Function:
File:
Line:
Static / global / inline / macro:
Subsystem:
```

Ví dụ:

```text
scheduler_tick()
kernel/sched/core.c
...
generic scheduler
```

### B. Prototype

Hiển thị prototype thực tế.

### C. Source code

Trích phần source code quan trọng nhất.

Không cần copy toàn bộ function nếu function rất dài, nhưng phải lấy đủ code để hiểu control flow.

### D. Line-by-line explanation

Giải thích từng đoạn:

```c
if (...)
    ...
```

Phải nói:

- biến nào thay đổi
- struct nào bị truy cập
- lock nào đang được giữ
- CPU context đang ở đâu
- task hiện tại là ai
- trạng thái scheduler thay đổi như thế nào
- function nào được gọi tiếp

### E. Caller / Callee

Phải chỉ ra:

```text
caller
  ↓
current_function
  ↓
callee
```

Đặc biệt phải phân biệt:

- direct call
- function pointer
- macro
- inline
- indirect callback
- architecture hook

### F. Data structures

Mỗi function phải chỉ rõ nó đọc/ghi struct nào.

Ví dụ:

```text
scheduler_tick()
    ↓
rq
    ↓
rq->curr
    ↓
task_struct
    ↓
se
    ↓
cfs_rq
```

### G. Lock / atomic / preemption state

Nếu liên quan:

- spinlock
- rq lock
- RCU
- atomic
- preempt_count
- IRQ state
- memory barrier

phải phân tích.

### H. High-level purpose

Cuối function phải có:

```text
"Function này tồn tại để làm gì?"
```

---

# 3. BA LỚP CODE — LUÔN PHẢI TÁCH RÕ

Trong toàn bộ nghiên cứu, luôn phân biệt:

```text
GENERIC KERNEL
    ↓
ARCH-INDEPENDENT SCHEDULER
    ↓
ARCH/ARM64
    ↓
CPU / REGISTER / MMU
```

Ví dụ:

```text
kernel/sched/*.c
        ↓
scheduler logic
        ↓
arch/arm64/kernel/*.c
        ↓
entry.S / process.c / irq.c
        ↓
Cortex-A72
```

Không được trộn implementation generic và ARM64 thành một lớp.

Mỗi flow phải đánh dấu:

```text
[GENERIC]
[ARCH-INDEPENDENT]
[ARM64]
[ASSEMBLY]
[HARDWARE]
```

---

# 4. CORE DATA STRUCTURES

Phải reconstruct toàn bộ relationship này:

```text
CPU
 │
 ▼
current
 │
 ▼
task_struct
 │
 ├── files
 │
 ├── sched_entity se
 │
 ├── mm
 │
 └── thread_info
```

Đồng thời:

```text
task_struct
    ↓
sched_entity
    ↓
cfs_rq
    ↓
rq
```

và:

```text
task_struct->mm
    ↓
mm_struct
    ↓
pgd
    ↓
TTBR0_EL1
```

Phải giải thích:

- `task_struct`
- `sched_entity`
- `cfs_rq`
- `rq`
- `sched_class`
- `mm_struct`
- `thread_struct`
- `thread_info`
- `cpu_context`

---

# 5. CURRENT TASK TRÊN ARM64

Phân tích cực sâu:

```text
CPU
 ↓
SP_EL0
 ↓
current
 ↓
task_struct
```

Phải tìm source code thực tế của mechanism lấy `current`.

Giải thích:

```text
mrs sp_el0
```

và relationship giữa:

```text
SP_EL0
SP_EL1
current
task_struct
kernel stack
```

Phân tích chính xác:

- CPU register nào chứa thông tin liên quan
- vì sao Linux ARM64 có thể xác định `current`
- khi context switch, register nào thay đổi
- `SP_EL0` có ý nghĩa gì trước/sau switch

---

# 6. CFS RUNQUEUE

Phân tích:

```text
cfs_rq->tasks_timeline
```

và:

```text
struct rb_root_cached
```

Giải thích:

- tại sao dùng red-black tree
- key là gì
- `vruntime`
- `min_vruntime`
- leftmost node
- `rb_leftmost`
- `rb_first_cached()`
- complexity
- tại sao scheduler không cần walk toàn bộ tree để lấy task đầu tiên

Dùng ví dụ:

```text
A vruntime = 1000
B vruntime = 1030
C vruntime = 1050
D vruntime = 1100
E vruntime = 1200
```

Nếu:

```text
curr = B
```

thì giải thích tại sao `B` không nhất thiết nằm trong cây và tại sao `A` được chọn.

---

# 7. pick_next_task() — PHÂN TÍCH CHUYÊN SÂU

Trace chính xác:

```text
__schedule()
    ↓
pick_next_task()
    ↓
for_each_class()
    ↓
stop
    ↓
dl
    ↓
rt
    ↓
fair
    ↓
idle
```

Phải giải thích:

```c
class->pick_next_task(rq)
```

là function pointer như thế nào.

Phân tích:

```text
struct sched_class
```

và các implementation.

Sau đó đi sâu CFS:

```text
pick_next_task_fair()
    ↓
pick_next_entity()
    ↓
__pick_first_entity()
    ↓
rb_first_cached()
```

Phải phân biệt rõ:

```text
scheduler class selection
```

với:

```text
CFS entity selection
```

---

# 8. TIF_NEED_RESCHED

Phải nghiên cứu cực sâu:

```text
set_tsk_need_resched()
    ↓
TIF_NEED_RESCHED
```

Phải giải thích chính xác:

> `set_tsk_need_resched()` không gọi `schedule()`.

Chỉ đánh dấu rằng scheduler cần chạy lại.

Phân tích:

```text
local CPU
remote CPU
```

Nếu task đang chạy trên local CPU:

```text
set bit
```

Nếu task ở remote CPU:

```text
smp_send_reschedule()
    ↓
IPI
    ↓
GIC SGI
    ↓
target CPU
```

Phải giải thích:

- PPI
- SGI
- IPI_RESCHEDULE
- GIC routing
- CPU nào nhận interrupt

---

# 9. SAFE POINT — PREEMPTION

Phải phân tích toàn bộ nơi scheduler thực sự được gọi.

### Case 1: EL0 → EL1 IRQ

```text
IRQ
 ↓
irq_exit()
 ↓
ret_to_user
 ↓
_TIF_WORK_MASK
 ↓
do_notify_resume()
 ↓
schedule()
```

### Case 2: IRQ khi CPU đang ở EL1

Phải giải thích:

```text
CONFIG_PREEMPT
preempt_count()
```

và điều kiện kernel có thể preempt hay không.

### Case 3: syscall return

```text
syscall
 ↓
kernel
 ↓
ret_to_user
 ↓
_TIF_WORK_MASK
 ↓
schedule()
```

### Case 4: preempt_enable()

Phân tích:

```text
preempt_count() → 0
    ↓
TIF_NEED_RESCHED
    ↓
preempt_schedule()
```

Phải tạo bảng:

| Safe point | Có thể schedule? | Điều kiện |
|---|---|---|
| EL0 → EL1 IRQ return | ... | ... |
| EL1 IRQ return | ... | ... |
| syscall return | ... | ... |
| preempt_enable | ... | ... |

---

# 10. ARM GENERIC TIMER → SCHEDULER

Phải trace cực kỳ chi tiết:

```text
CNTV
 ↓
PPI
 ↓
GIC
 ↓
ARM64 exception
 ↓
el0_irq / el1_irq
 ↓
kernel_entry
 ↓
irq_handler
 ↓
gic_handle_irq()
 ↓
ICC_IAR1_EL1
 ↓
handle_domain_irq()
 ↓
irq_enter()
 ↓
generic_handle_irq()
 ↓
flow handler
 ↓
irqaction->handler()
 ↓
arch_timer_handler_virt()
 ↓
clockevent
 ↓
event_handler()
 ↓
hrtimer_interrupt()
 ↓
tick_sched_timer()
 ↓
tick_sched_handle()
 ↓
update_process_times()
 ↓
scheduler_tick()
```

Phải đặc biệt giải thích:

```text
CONFIG_HIGH_RES_TIMERS=y
```

và vì sao flow đi theo:

```text
hrtimer
```

thay vì mô hình legacy:

```text
tick_handle_periodic()
```

Không được tự khẳng định điều này nếu kernel version thực tế cho thấy khác; phải verify source.

---

# 11. ARM64 EXCEPTION ENTRY

Phân tích source:

```text
arch/arm64/kernel/entry.S
```

Phải trace:

```text
VBAR_EL1
```

và offsets:

```text
+0x480 → el0_irq
+0x280 → el1_irq
```

Verify từ source đúng version.

Giải thích:

```text
EL0
 ↓
exception
 ↓
SPSR_EL1
ELR_EL1
ESR_EL1
 ↓
PSTATE / DAIF
 ↓
kernel stack
 ↓
pt_regs
```

Phải giải thích chính xác:

- CPU tự save cái gì
- Linux save cái gì
- `kernel_entry`
- `pt_regs`
- register frame
- `kernel_exit`
- `eret`

---

# 12. IRQ SUBSYSTEM

Trace:

```text
gic_handle_irq()
 ↓
handle_domain_irq()
 ↓
irq_enter()
 ↓
generic_handle_irq()
 ↓
flow handler
 ↓
irqaction->handler()
 ↓
irq_exit()
```

Phải giải thích:

```text
preempt_count += HARDIRQ_OFFSET
```

và khi:

```text
irq_exit()
```

thì điều gì xảy ra.

Phải mô tả chính xác CPU state trong interrupt:

```text
current = A
EL = 1
IRQ = disabled
preempt_count > 0
```

---

# 13. scheduler_tick()

Phân tích:

```text
scheduler_tick()
```

Phải trace:

```text
rq->curr
 ↓
task_tick_fair()
 ↓
entity_tick()
 / hoặc implementation tương ứng
 ↓
check_preempt_tick()
 ↓
resched_curr()
```

và đặc biệt:

```text
update_curr()
```

Giải thích:

```text
sum_exec_runtime
vruntime
ideal_runtime
calc_delta_fair()
```

Phải chỉ rõ:

```text
vruntime += calc_delta_fair(...)
```

được tính như thế nào ở version thực tế.

---

# 14. TIMER PREEMPT TASK A → TASK B

Reconstruct đầy đủ scenario:

```text
CPU0
Task A đang chạy tại EL0
```

Sau đó:

```text
1. Timer expires
2. PPI pending
3. GIC route
4. CPU exception EL0 → EL1
5. hardware saves exception state
6. kernel_entry
7. x0-x30 → pt_regs
8. IRQ handler
9. timer handler
10. scheduler_tick
11. update_curr(A)
12. check_preempt_tick
13. resched_curr
14. TIF_NEED_RESCHED
15. irq_exit
16. ret_to_user
17. _TIF_WORK_MASK
18. do_notify_resume
19. schedule
20. __schedule
21. pick_next_task
22. CFS
23. pick_next_entity
24. Task B selected
25. context_switch
26. switch_mm_irqs_off
27. switch_to
28. __switch_to
29. cpu_switch_to
30. restore B CPU context
31. kernel_exit
32. eret
33. EL0
34. B continues
```

Mỗi bước phải ghi:

```text
CPU:
EL:
IRQ state:
current:
rq->curr:
preempt_count:
task state:
kernel/user:
stack:
important registers:
```

Tạo một bảng timeline đầy đủ.

---

# 15. __schedule() — PHÂN TÍCH TOÀN BỘ

Phải trace:

```text
schedule()
 ↓
__schedule()
```

Giải thích:

- rq lock
- prev
- next
- scheduler state
- rq current
- deactivate task
- pick next task
- context switch
- finish task switch

Phải giải thích chính xác semantics của:

```text
prev
next
```

và đặc biệt:

```c
finish_task_switch(prev);
```

Tại sao `prev` ở thời điểm này là task cũ nhưng code đang chạy trong context của `next`.

Đây là một điểm rất quan trọng, phải giải thích bằng timeline:

```text
Task A
 ↓
switch_to(A,B,A)
 ↓
CPU thực sự chạy B
 ↓
B tiếp tục sau điểm switch
 ↓
finish_task_switch(A)
```

---

# 16. context_switch()

Phân tích chính xác:

```text
context_switch(rq, prev, next)
```

theo thứ tự:

```text
prepare_task_switch()
 ↓
MM switch
 ↓
switch_to()
 ↓
barrier()
 ↓
finish_task_switch()
```

Phải phân biệt:

```text
generic
arch-independent
ARM64
assembly
```

---

# 17. MM SWITCH

Phân tích:

```text
switch_mm_irqs_off()
```

và:

```text
switch_mm()
```

Giải thích:

```text
prev->active_mm
next->mm
pgd
TTBR0_EL1
ASID
TLB
```

Case:

```text
next->mm != NULL
```

và:

```text
next->mm == NULL
```

tức kernel thread.

Phải giải thích:

```text
enter_lazy_tlb()
next->active_mm = prev->active_mm
mmgrab()
```

và:

```text
if (!prev->mm)
    rq->prev_mm = prev->active_mm;
```

Tạo sơ đồ:

```text
User task A
    ↓
A->mm
    ↓
pgd_A
    ↓
TTBR0_EL1

switch

User task B
    ↓
B->mm
    ↓
pgd_B
    ↓
TTBR0_EL1
```

---

# 18. switch_to() — RANH GIỚI ARCH

Phân tích:

```text
switch_to(prev,next,prev)
```

và xác định đây là:

```text
MACRO
```

Sau đó trace:

```text
switch_to()
 ↓
__switch_to()
 ↓
cpu_switch_to()
```

Phải chỉ rõ source file thật.

---

# 19. __switch_to()

Phân tích file:

```text
arch/arm64/kernel/process.c
```

và tất cả helper liên quan.

Tìm và giải thích:

- TLS
- FPSIMD
- contextidr
- CPU state
- thread context
- stack
- processor state

Không được giả định rằng tất cả đều được switch trực tiếp nếu source thực tế tổ chức khác.

---

# 20. cpu_switch_to() — ARM64 ASSEMBLY

Phân tích cực kỳ sâu:

```text
cpu_switch_to()
```

trong:

```text
arch/arm64/kernel/entry.S
```

Phải giải thích từng instruction, ví dụ:

```asm
stp x19, x20, [x8]
...
ldp x19, x20, [x9]
...
mov sp, x9
msr sp_el0, x8
ret
```

Nhưng phải lấy instruction từ **source code thực tế của kernel version nghiên cứu**, không dùng ví dụ nếu source khác.

Phải lập bảng:

| Register | Saved? | Restored? | Ý nghĩa |
|---|---|---|---|
| x19-x28 | | | callee saved |
| x29 | | | frame pointer |
| x30 | | | LR |
| SP | | | kernel stack |
| SP_EL0 | | | current/user stack relation |
| PC | | | resume point |

Phải giải thích:

```text
cpu_context.pc
```

hoặc field tương ứng trong version đó.

---

# 21. "CPU SWITCH SANG NEXT Ở ĐÂU?"

Đây là câu hỏi trọng tâm.

Phải trả lời thật chính xác:

> Instruction nào làm CPU bắt đầu thực thi context của task B?

Phân biệt:

```text
logical scheduler switch
```

và:

```text
actual CPU register switch
```

và:

```text
actual instruction retirement after switch
```

Tạo sơ đồ:

```text
generic decision
        ↓
context_switch
        ↓
ARM64 switch_to
        ↓
cpu_switch_to
        ↓
restore registers
        ↓
SP changes
        ↓
PC/LR changes
        ↓
CPU continues in B context
```

---

# 22. RETURN PATH CỦA TASK B

Phân tích:

```text
Task B previously called schedule()
```

Sau context switch:

```text
Task B resumes
 ↓
unwinds frozen call chain
 ↓
finish_task_switch()
 ↓
return to previous scheduler frame
 ↓
...
 ↓
ret_to_user
 ↓
kernel_exit
 ↓
restore pt_regs_B
 ↓
eret
 ↓
EL0
 ↓
PC_B
```

Phải giải thích tại sao Task B có thể "tiếp tục từ giữa function" thay vì chạy lại từ đầu.

Đây là một điểm phải dùng stack diagram.

---

# 23. LOAD BALANCING

Phân tích toàn bộ:

```text
scheduler_tick()
 ↓
trigger_load_balance()
 ↓
rebalance_domains()
 ↓
load_balance()
```

Các function phải nghiên cứu:

```text
find_busiest_group()
find_busiest_queue()
detach_tasks()
attach_tasks()
can_migrate_task()
move_task()
update_sg_lb_stats()
```

Nếu source version khác line number thì phải tìm line chính xác và ghi:

```text
actual source location
```

---

# 24. sched_domain / sched_group / rq

Phân tích quan hệ:

```text
sched_domain
    ↓
sched_group
    ↓
rq
```

Giải thích:

- hierarchy
- CPU topology
- local CPU
- busiest group
- busiest rq
- capacity
- load
- imbalance

Tạo diagram cho nhiều CPU:

```text
          sched_domain
        /              \
    group0            group1
    CPU0 CPU1         CPU2 CPU3
```

---

# 25. load_balance()

Reconstruct:

```text
load_balance()
 ├── find_busiest_group()
 ├── find_busiest_queue()
 ├── lock
 ├── detach_tasks()
 ├── attach_tasks()
 └── active balance
```

Phải giải thích tại sao lock order quan trọng.

Đặc biệt:

```text
this_rq->lock
busiest->lock
```

và cách tránh deadlock.

---

# 26. detach_tasks() / attach_tasks()

Phải phân tích:

```text
detach_tasks()
```

là:

> detach trước, migrate sau

hay nói cách khác:

```text
bỏ task khỏi busiest rq
        ↓
đưa vào temporary list
        ↓
attach sang local rq
```

Phải phân tích:

```text
can_migrate_task()
```

và:

```text
p->cpus_ptr
task running?
cache hot?
local CPU idle?
```

---

# 27. ACTIVE BALANCE

Phân tích:

```text
busiest->active_balance = 1
 ↓
wakeup busiest->stop
 ↓
stop_sched_class
 ↓
forced migration
```

Giải thích:

- tại sao cần active balance
- stop task là gì
- stop scheduling class có priority cao hơn mọi class khác như thế nào
- khi nào active balance được dùng

---

# 28. LOCAL VS REMOTE PREEMPTION

So sánh đầy đủ:

### LOCAL

```text
resched_curr(rq)
 ↓
TIF_NEED_RESCHED
```

### REMOTE

```text
resched_curr(rq)
 ↓
set flag
 ↓
smp_send_reschedule()
 ↓
IPI
 ↓
GIC
 ↓
remote CPU IRQ
 ↓
safe point
 ↓
schedule()
```

Phải tạo sequence diagram CPU0/CPU1.

---

# 29. CPU / TASK / RQ STATE MACHINE

Tạo state machine:

```text
TASK_RUNNING
    ↓
running on CPU
    ↓
TIF_NEED_RESCHED
    ↓
schedule requested
    ↓
selected as prev
    ↓
context switch
    ↓
not running
```

Đồng thời:

```text
rq->curr
```

phải được trace ở từng điểm.

---

# 30. FULL OBJECT GRAPH

Tạo một high-level object graph:

```text
CPU
 │
 ├── current
 │     ↓
 │   task_struct
 │     ├── se
 │     ├── mm
 │     ├── thread
 │     └── thread_info
 │
 └── rq
       ├── curr
       ├── cfs
       │    ├── tasks_timeline
       │    └── min_vruntime
       └── sched_class
```

Sau đó nối với:

```text
MMU
TTBR0_EL1
ASID
kernel stack
SP
SP_EL0
cpu_context
```

---

# 31. FULL HARDWARE → SOFTWARE → HARDWARE MODEL

Cuối nghiên cứu phải tạo một high-level architecture map:

```text
                    HARDWARE
                       │
               ARM Generic Timer
                       │
                       ▼
                      GIC
                       │
                       ▼
              ARM64 Exception Entry
                       │
                       ▼
                GENERIC IRQ CORE
                       │
                       ▼
               CLOCKEVENT / HRTIMER
                       │
                       ▼
                 SCHEDULER TICK
                       │
                       ▼
                     CFS
                       │
                       ▼
               TASK SELECTION
                       │
                       ▼
                 CONTEXT SWITCH
                       │
          ┌────────────┴────────────┐
          ▼                         ▼
       MMU/MM                  CPU CONTEXT
          │                         │
          ▼                         ▼
    TTBR0 / ASID              registers/stack/PC
          │                         │
          └────────────┬────────────┘
                       ▼
                   ARM64 CPU
                       │
                       ▼
                   Task B
```

---

# 32. SOURCE CODE MAP

Cuối tài liệu phải có bảng source:

| Subsystem | File | Important functions |
|---|---|---|
| Scheduler core | `kernel/sched/core.c` | ... |
| CFS | `kernel/sched/fair.c` | ... |
| Timer | `kernel/time/timer.c` | ... |
| hrtimer | `kernel/time/hrtimer.c` | ... |
| Tick | `kernel/time/tick-sched.c` | ... |
| ARM timer | `drivers/clocksource/arm_arch_timer.c` | ... |
| ARM64 entry | `arch/arm64/kernel/entry.S` | ... |
| ARM64 process | `arch/arm64/kernel/process.c` | ... |
| ARM64 IRQ | `arch/arm64/kernel/irq.c` | ... |
| MM | `arch/arm64/mm/*` / relevant files | ... |
| GIC | relevant `drivers/irqchip/*` | ... |

Line number phải là line number của source version thực tế.

---

# 33. SOURCE VERIFICATION

Không được chỉ nói:

```text
fair.c:xxxxx
```

mà không kiểm tra.

Phải verify bằng source.

Có thể sử dụng:

```bash
grep -R "function_name" kernel/sched/
grep -R "function_name" arch/arm64/
grep -R "function_name" drivers/
```

Hoặc:

```bash
git grep "function_name"
```

Nếu có thể:

```bash
git blame
git log -L
```

để hiểu lịch sử implementation.

Khi line number tài liệu khác source:

```text
Document:
fair.c:XXXX

Actual source:
fair.c:YYYY
```

và giải thích vì sao khác.

---

# 34. DISTINGUISH FACT / INFERENCE

Mọi kết luận phải phân loại:

```text
[SOURCE VERIFIED]
```

nếu thấy trực tiếp trong source.

```text
[DOCUMENT]
```

nếu lấy từ PDF.

```text
[INFERENCE]
```

nếu suy luận từ source.

```text
[HARDWARE]
```

nếu là hành vi do ARM/GIC/CPU architecture quy định.

Không được trình bày inference như fact.

---

# 35. KHÔNG ĐƯỢC BỎ QUA CÁC CHI TIẾT SAU

Đây là checklist bắt buộc:

```text
[ ] task_struct
[ ] sched_entity
[ ] cfs_rq
[ ] rq
[ ] sched_class
[ ] sched_domain
[ ] sched_group
[ ] current
[ ] SP_EL0
[ ] kernel stack
[ ] pt_regs
[ ] mm_struct
[ ] pgd
[ ] TTBR0_EL1
[ ] ASID
[ ] TIF_NEED_RESCHED
[ ] _TIF_WORK_MASK
[ ] preempt_count
[ ] HARDIRQ_OFFSET
[ ] resched_curr
[ ] set_tsk_need_resched
[ ] smp_send_reschedule
[ ] IPI
[ ] GIC SGI
[ ] GIC PPI
[ ] CNTV
[ ] CNTV_CTL_EL0
[ ] CNTV_TVAL_EL0
[ ] CNTV_CVAL_EL0
[ ] VBAR_EL1
[ ] el0_irq
[ ] el1_irq
[ ] kernel_entry
[ ] irq_handler
[ ] gic_handle_irq
[ ] handle_domain_irq
[ ] irq_enter
[ ] irq_exit
[ ] arch_timer_handler_virt
[ ] clockevent
[ ] hrtimer_interrupt
[ ] tick_sched_timer
[ ] tick_sched_handle
[ ] update_process_times
[ ] scheduler_tick
[ ] update_curr
[ ] calc_delta_fair
[ ] check_preempt_tick
[ ] ideal_runtime
[ ] vruntime
[ ] min_vruntime
[ ] pick_next_task
[ ] pick_next_task_fair
[ ] pick_next_entity
[ ] __pick_first_entity
[ ] rb_first_cached
[ ] __schedule
[ ] prepare_task_switch
[ ] context_switch
[ ] switch_mm_irqs_off
[ ] switch_to
[ ] __switch_to
[ ] cpu_switch_to
[ ] barrier
[ ] finish_task_switch
[ ] kernel_exit
[ ] eret
[ ] load_balance
[ ] find_busiest_group
[ ] find_busiest_queue
[ ] detach_tasks
[ ] attach_tasks
[ ] can_migrate_task
[ ] active_balance
[ ] stop_sched_class
```

---

# 36. OUTPUT FORMAT

Không viết kiểu textbook chung chung.

Hãy tạo tài liệu theo cấu trúc:

```text
# Chapter 12 — Deep Linux Scheduler / ARM64 Analysis

## 1. Executive Mental Model

## 2. Architecture Overview

## 3. Core Data Structures

## 4. current / task_struct / rq

## 5. CFS Internals

## 6. pick_next_task()

## 7. Timer → IRQ

## 8. scheduler_tick()

## 9. TIF_NEED_RESCHED

## 10. Safe Points / Preemption

## 11. __schedule()

## 12. context_switch()

## 13. MM Switch

## 14. ARM64 switch_to()

## 15. cpu_switch_to()

## 16. Task A → Task B Complete Trace

## 17. Load Balancing

## 18. Local vs Remote Reschedule

## 19. State Machines

## 20. Complete High-Level Architecture

## 21. Source Code Reference

## 22. Important Invariants

## 23. Common Misconceptions

## 24. Debugging / Verification Techniques

## 25. Final Mental Model
```

---

# 37. DIAGRAMS BẮT BUỘC

Tạo Mermaid diagrams cho ít nhất:

### Diagram 1
```text
Timer → GIC → IRQ → Scheduler
```

### Diagram 2
```text
pick_next_task → CFS → rb_first_cached
```

### Diagram 3
```text
TIF_NEED_RESCHED → safe point → schedule
```

### Diagram 4
```text
context_switch → switch_mm → switch_to → cpu_switch_to
```

### Diagram 5
```text
Task A → Task B complete timeline
```

### Diagram 6
```text
load_balance
```

### Diagram 7
```text
task_struct / se / cfs_rq / rq object graph
```

### Diagram 8
```text
CPU register / current / SP_EL0 / kernel stack
```

---

# 38. IMPORTANT INVARIANTS

Cuối mỗi major section phải ghi:

```text
### Invariants
```

Ví dụ:

```text
TIF_NEED_RESCHED != immediate schedule()

current != necessarily the same as rq->curr in every transient context

curr task is not necessarily present in CFS rb-tree

pick_next_task() ≠ CFS selection only

context_switch() ≠ CPU register switch only

MM switch != CPU context switch

timer interrupt ≠ direct call to schedule()
```

Nhưng mỗi invariant phải được source-verify.

---

# 39. COMMON MISCONCEPTIONS

Tạo section:

```text
## Common Misconceptions
```

Phải giải thích ít nhất:

1. Timer interrupt không trực tiếp gọi `schedule()`.
2. `set_tsk_need_resched()` chỉ set flag.
3. `current` không tự động đổi ngay khi set flag.
4. `pick_next_task()` không đồng nghĩa với context switch.
5. `switch_mm()` không phải switch toàn bộ CPU context.
6. `cpu_switch_to()` mới là low-level register/context switch.
7. Task đang chạy (`curr`) có thể không nằm trong CFS tree.
8. `vruntime` là key logic của CFS nhưng implementation có thể dùng offset/reference normalization.
9. `finish_task_switch(prev)` chạy sau khi CPU đã switched sang next.
10. remote reschedule cần IPI.

---

# 40. FINAL REQUIREMENT

Sau toàn bộ nghiên cứu, hãy trả lời câu hỏi:

> "Nếu tôi nhìn vào một Cortex-A72 đang chạy Task A ở EL0, timer hết hạn, tại sao CPU cuối cùng lại chạy Task B?"

Phải trả lời theo đúng 4 tầng:

```text
HARDWARE
    ↓
ARM64
    ↓
GENERIC KERNEL
    ↓
SCHEDULER / CFS
```

và cuối cùng cô đọng thành:

```text
ONE SCREEN MASTER FLOW
```

với toàn bộ flow:

```text
CNTV
→ PPI
→ GIC
→ VBAR_EL1
→ el0_irq
→ kernel_entry
→ irq_handler
→ gic_handle_irq
→ handle_domain_irq
→ irq_enter
→ arch_timer_handler_virt
→ hrtimer
→ tick_sched_timer
→ update_process_times
→ scheduler_tick
→ update_curr
→ check_preempt_tick
→ resched_curr
→ TIF_NEED_RESCHED
→ irq_exit
→ ret_to_user
→ _TIF_WORK_MASK
→ do_notify_resume
→ schedule
→ __schedule
→ pick_next_task
→ pick_next_task_fair
→ pick_next_entity
→ __pick_first_entity
→ rb_first_cached
→ context_switch
→ switch_mm_irqs_off
→ switch_to
→ __switch_to
→ cpu_switch_to
→ restore CPU context B
→ kernel_exit
→ eret
→ EL0
→ Task B
```

# CORE RULE

Đừng chỉ mô tả "Linux scheduler hoạt động như thế nào".

Hãy **reverse-engineer Linux scheduler từ source code**.

Mục tiêu là sau khi đọc tài liệu này, người đọc có thể mở:

```text
kernel/sched/core.c
kernel/sched/fair.c
kernel/time/hrtimer.c
kernel/time/tick-sched.c
kernel/time/timer.c
drivers/clocksource/arm_arch_timer.c
arch/arm64/kernel/entry.S
arch/arm64/kernel/process.c
arch/arm64/kernel/irq.c
```

và tự trace được:

```text
Timer IRQ
    ↓
scheduler decision
    ↓
CFS task selection
    ↓
context switch
    ↓
ARM64 register switch
    ↓
Task B resumes
```

Mỗi function quan trọng phải có:

```text
SOURCE
→ CALLER
→ CALLEE
→ DATA STRUCTURE
→ CPU STATE
→ LOCK/PREEMPT STATE
→ WHY
→ NEXT STEP
```

Không được bỏ qua implementation details chỉ vì chúng "quá low-level".