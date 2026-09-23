# PROMPT — LINUX KERNEL SOFTIRQ DEEP DIVE

Bạn là một **Linux Kernel Engineer + Kernel Scheduler Expert + Interrupt/Concurrency Expert**, có khả năng đọc và phân tích trực tiếp Linux kernel source code.

Tôi muốn bạn giúp tôi **nghiên cứu SoftIRQ trong Linux Kernel cực kỳ chi tiết và có hệ thống**, từ khái niệm cơ bản cho tới implementation ở source code, runtime behavior, CPU scheduling, SMP/per-CPU, networking, timer, tasklet, RCU, ksoftirqd, PREEMPT_RT, debugging và performance.

## MỤC TIÊU

Sau khi hoàn thành, tôi phải hiểu được toàn bộ chuỗi:

```text
Hardware Interrupt
        ↓
Hard IRQ handler
        ↓
raise_softirq()
        ↓
set pending bit
        ↓
return from IRQ
        ↓
softirq processing
        ↓
do_softirq()
        ↓
__do_softirq()
        ↓
softirq vector
        ↓
specific softirq handler
```

và trường hợp:

```text
SoftIRQ không được xử lý ngay
        ↓
ksoftirqd/N thread
        ↓
scheduler
        ↓
CPU chạy ksoftirqd
        ↓
do_softirq()
```

Tôi không chỉ muốn biết:

> "SoftIRQ dùng để defer work từ interrupt context."

Mà phải hiểu:

> **CPU, kernel, scheduler, interrupt subsystem và per-CPU data thực sự phối hợp với nhau như thế nào để SoftIRQ hoạt động?**

---

# 0. NGUYÊN TẮC NGHIÊN CỨU

Không được giải thích hời hợt.

Mỗi chủ đề phải phân tích theo nhiều tầng:

```text
1. CONCEPT
      ↓
2. WHY
      ↓
3. KERNEL DESIGN
      ↓
4. SOURCE CODE
      ↓
5. EXECUTION FLOW
      ↓
6. CPU / CONTEXT
      ↓
7. PER-CPU DATA
      ↓
8. SMP
      ↓
9. MEMORY / SYNCHRONIZATION
      ↓
10. PERFORMANCE
      ↓
11. DEBUGGING
```

Mỗi khi có thể, phải trả lời:

* Ai gọi?
* Gọi khi nào?
* Chạy trong context nào?
* CPU nào chạy?
* Có thể sleep không?
* Interrupt có bị disable không?
* Preemption có bị disable không?
* Data nằm per-CPU hay global?
* SoftIRQ được raise bởi ai?
* SoftIRQ được xử lý bởi ai?
* Điều kiện nào khiến nó bị defer?
* Điều gì xảy ra nếu SoftIRQ chạy quá lâu?

---

# 1. VERSION / SOURCE BASELINE

Ưu tiên sử dụng:

```text
Linux Kernel 5.10.x
```

Nếu tôi cung cấp một repository:

> PHẢI ưu tiên repository đó làm nguồn chính.

Không được tự động thay thế source trong repository bằng kernel version khác.

Khi sử dụng Linux upstream để đối chiếu:

* chỉ rõ version
* chỉ rõ file
* chỉ rõ function
* chỉ rõ sự khác biệt nếu có

Phân biệt rõ:

```text
Repository-specific code
vs
Upstream Linux code
```

---

# PHẦN I — SOFTIRQ LÀ GÌ?

## 2. Định nghĩa SoftIRQ

Giải thích:

* SoftIRQ là gì?
* Tại sao Linux cần SoftIRQ?
* Vấn đề của việc xử lý toàn bộ work trong hard IRQ?
* Deferred interrupt processing là gì?
* SoftIRQ khác với hard IRQ như thế nào?
* SoftIRQ khác process context như thế nào?

Phải có bảng:

| Context         | Sleep? | Preempt? | Typical Use |
| --------------- | ------ | -------- | ----------- |
| Hard IRQ        | ?      | ?        | ?           |
| SoftIRQ         | ?      | ?        | ?           |
| ksoftirqd       | ?      | ?        | ?           |
| Process context | ?      | ?        | ?           |

---

# PHẦN II — IRQ CONTEXT

## 3. Interrupt Context

Giải thích thật kỹ:

* interrupt context
* hardirq context
* softirq context
* process context
* atomic context

Phải phân biệt:

```text
Hard IRQ
SoftIRQ
Tasklet
ksoftirqd
Workqueue
Kernel Thread
```

Phải giải thích:

> Context nào được phép sleep?

> Context nào được phép schedule?

> Context nào được phép mutex_lock()?

> Context nào phải sử dụng spinlock?

---

# 4. IN_INTERRUPT() / CONTEXT DETECTION

Phân tích các macro/function liên quan:

* `in_interrupt()`
* `in_irq()`
* `in_softirq()`
* `in_serving_softirq()`
* `irqs_disabled()`
* `preempt_count()`
* hardirq bits
* softirq bits

Giải thích `preempt_count` dùng như thế nào để biểu diễn execution context.

Phải minh họa bit layout conceptually.

---

# PHẦN III — SOFTIRQ TYPES

## 5. SoftIRQ Vectors

Tìm hiểu toàn bộ các loại SoftIRQ của Linux kernel version đang nghiên cứu.

Phân tích:

```text
HI_SOFTIRQ
TIMER_SOFTIRQ
NET_TX_SOFTIRQ
NET_RX_SOFTIRQ
BLOCK_SOFTIRQ
IRQ_POLL_SOFTIRQ
TASKLET_SOFTIRQ
SCHED_SOFTIRQ
HRTIMER_SOFTIRQ
RCU_SOFTIRQ
```

Nếu version không có một vector hoặc thứ tự khác:

> ghi rõ version-specific difference.

Với mỗi loại:

* purpose
* raise bởi ai?
* handler ở đâu?
* chạy khi nào?
* workload thực tế
* có thể chạy lâu không?
* performance implications

---

# 6. SOFTIRQ VECTOR TABLE

Phân tích kiến trúc:

```text
softirq_vec[]
```

Giải thích:

* vector index
* handler function
* registration
* initialization
* static/global storage
* per-CPU pending state

---

# PHẦN IV — RAISE SOFTIRQ

## 7. `raise_softirq()`

Theo dõi toàn bộ call chain:

```text
caller
  ↓
raise_softirq()
  ↓
raise_softirq_irqoff() / related
  ↓
open_softirq / or pending bit operation
```

Phải kiểm tra chính xác source code version đang nghiên cứu.

Giải thích:

* pending bit ở đâu?
* CPU nào sở hữu bit đó?
* atomicity như thế nào?
* local CPU hay remote CPU?
* interrupt state ảnh hưởng ra sao?

---

# 8. PENDING BITMAP

Phân tích:

```text
local_softirq_pending()
```

và các primitive liên quan.

Giải thích:

* per-CPU pending state
* bitmap
* set bit
* read pending
* clear pending
* race conditions

Minh họa:

```text
CPU0
 └── pending = 00000110

CPU1
 └── pending = 00000001
```

Giải thích vì sao SoftIRQ pending state là **per-CPU**.

---

# PHẦN V — OPEN_SOFTIRQ

## 9. `open_softirq()`

Tìm hiểu:

* ai gọi?
* khi boot hay runtime?
* handler registration
* softirq_vec initialization
* locking / initialization constraints

Ví dụ với:

```c
open_softirq(NET_RX_SOFTIRQ, net_rx_action);
```

giải thích hoàn chỉnh.

---

# PHẦN VI — PROCESSING SOFTIRQ

## 10. `do_softirq()`

Phân tích call chain đầy đủ.

Tìm tất cả caller quan trọng:

* interrupt return path
* kernel paths
* exception paths nếu phù hợp
* explicit `do_softirq()`

Không đoán caller.

Phải truy source.

---

# 11. `__do_softirq()`

Đây là phần CORE của toàn bộ nghiên cứu.

Phải phân tích từng bước.

Ví dụ conceptual:

```text
__do_softirq()
    │
    ├── save state
    ├── disable local IRQ
    ├── read pending
    ├── clear pending
    ├── invoke handlers
    ├── check new pending
    ├── time / restart limit
    └── exit or wake ksoftirqd
```

Phải map từng bước với source code thực tế.

---

# 12. SOFTIRQ TIME LIMIT

Nghiên cứu các cơ chế giới hạn:

* `MAX_SOFTIRQ_TIME`
* `MAX_SOFTIRQ_RESTART`
* jiffies limit
* restart count

Giải thích:

> Vì sao kernel không cho SoftIRQ chạy vô hạn?

Điều gì xảy ra nếu SoftIRQ luôn tự raise chính nó?

Ví dụ:

```text
SoftIRQ
   ↓
handler
   ↓
raise_softirq()
   ↓
handler again
   ↓
raise_softirq()
   ↓
...
```

Phân tích cơ chế chống starvation.

---

# 13. SOFTIRQ RESTART

Tìm hiểu logic:

```text
pending
   ↓
handler
   ↓
new pending
   ↓
restart
```

và khi nào:

```text
restart
```

bị dừng.

---

# PHẦN VII — KSOFTIRQD

## 14. ksoftirqd

Đây là phần bắt buộc phải nghiên cứu sâu.

Giải thích:

* `ksoftirqd/N` là gì?
* một thread cho mỗi CPU?
* thread được tạo khi nào?
* affinity
* scheduling priority
* lifecycle
* wakeup mechanism

Phân tích source:

```text
ksoftirqd_should_run()
run_ksoftirqd()
wakeup_softirqd()
```

hoặc function tương ứng trong version cụ thể.

Không được giả định tên function nếu source khác.

---

# 15. KHI NÀO SOFTIRQ CHUYỂN SANG KSOFTIRQD?

Phân tích chính xác:

```text
__do_softirq()
       │
       ├── time exceeded?
       │
       ├── restart exceeded?
       │
       └── cannot continue?
                ↓
         wakeup_softirqd()
                ↓
         ksoftirqd/N
```

Phải trả lời:

* tại sao không tiếp tục trong IRQ return path?
* vì sao kernel dùng thread?
* CPU nào chạy thread?
* pending state được giữ ở đâu?

---

# 16. ksoftirqd VÀ SCHEDULER

Tìm hiểu:

```text
ksoftirqd
   ↓
schedule
   ↓
run queue
   ↓
scheduler
   ↓
CPU executes ksoftirqd
```

Phải giải thích:

* task_struct
* kernel thread
* scheduling class
* priority
* preemption
* CPU affinity
* runnable state

---

# PHẦN VIII — INTERRUPT RETURN PATH

## 17. IRQ RETURN → SOFTIRQ

Đây là flow quan trọng nhất.

Truy source code thực tế của architecture.

Mô tả:

```text
Hardware interrupt
       ↓
Entry
       ↓
hard IRQ handler
       ↓
IRQ exit
       ↓
irq_exit()
       ↓
invoke_softirq()
       ↓
do_softirq()
```

Phải xác định đúng function trong architecture/version.

Nếu ARM64:

tập trung vào:

```text
arch/arm64/kernel/
```

và generic IRQ code:

```text
kernel/softirq.c
kernel/irq/
```

Phải phân biệt:

```text
Architecture-specific
vs
Generic kernel
```

---

# 18. `irq_exit()`

Phân tích:

* caller
* entry/exit nesting
* preempt_count
* softirq check
* RCU interaction
* scheduler interaction

Đặc biệt trace logic:

```text
irq_exit()
    ↓
is softirq pending?
    ↓
invoke_softirq()
```

---

# PHẦN IX — SCHEDULER TICK VÀ SOFTIRQ

## 19. Scheduler Tick

Giải thích SoftIRQ liên hệ với scheduler tick như thế nào.

Không được nhầm:

```text
scheduler_tick()
```

với:

```text
softirq processing
```

Phải trace source.

Tập trung vào:

* `update_process_times()`
* `scheduler_tick()`
* timer related softirq
* RCU softirq
* softirq pending checks

Phân biệt:

```text
Scheduler tick
vs
Timer interrupt
vs
TIMER_SOFTIRQ
vs
SoftIRQ processing
```

---

# PHẦN X — TASKLET

## 20. Tasklet

Tìm hiểu:

* tasklet là gì?
* implementation bằng SoftIRQ như thế nào?
* `TASKLET_SOFTIRQ`
* `tasklet_schedule()`
* `tasklet_hi_schedule()`
* tasklet callback
* serialization

Phải trace:

```text
tasklet_schedule()
      ↓
raise tasklet softirq
      ↓
TASKLET_SOFTIRQ
      ↓
tasklet_action()
      ↓
tasklet callback
```

---

# 21. TASKLET VS SOFTIRQ

So sánh:

| Property           | SoftIRQ | Tasklet |
| ------------------ | ------- | ------- |
| Parallel execution | ?       | ?       |
| Per-CPU            | ?       | ?       |
| API                | ?       | ?       |
| Serialization      | ?       | ?       |
| Typical use        | ?       | ?       |

Giải thích tại sao Tasklet thực chất dựa trên SoftIRQ.

---

# PHẦN XI — TIMER

## 22. TIMER_SOFTIRQ

Phân tích:

```text
timer interrupt
      ↓
timer softirq
      ↓
timer processing
```

Tìm function chính xác trong source.

Giải thích:

* timer wheel
* timer base
* expiration
* softirq handler

Nếu version đang nghiên cứu dùng implementation khác:

> phân tích đúng implementation thực tế.

---

# PHẦN XII — NETWORKING

## 23. NET_RX_SOFTIRQ

Đây phải là một trong những phần sâu nhất.

Trace:

```text
NIC interrupt
    ↓
driver ISR
    ↓
NAPI scheduling
    ↓
NET_RX_SOFTIRQ
    ↓
net_rx_action()
    ↓
poll()
    ↓
packet processing
```

Phân tích source code:

* NAPI
* `napi_schedule()`
* `____napi_schedule()`
* `net_rx_action()`
* poll budget
* packet backlog
* GRO nếu phù hợp

---

# 24. NETWORK SOFTIRQ BUDGET

Giải thích:

* NAPI budget
* why budget exists
* CPU starvation
* packet flood
* ksoftirqd
* interrupt mitigation

Phải giải thích:

> Vì sao Linux không xử lý vô hạn packet trong một lần SoftIRQ?

---

# PHẦN XIII — RCU + SOFTIRQ

## 25. RCU_SOFTIRQ

Phân tích mối liên hệ:

```text
RCU
 ↓
RCU_SOFTIRQ
 ↓
callback processing
```

Tập trung vào Linux version đang nghiên cứu.

Trace:

* raise RCU softirq
* `rcu_core()`
* `RCU_SOFTIRQ`
* callback processing

Liên hệ với:

```text
call_rcu()
Grace Period
RCU callback
RCU_SOFTIRQ
```

Phải phân biệt:

```text
Grace Period progress
vs
RCU callback execution
```

---

# PHẦN XIV — SMP / MULTI-CORE

## 26. SoftIRQ trong SMP

Phân tích:

```text
CPU0 raises SoftIRQ
CPU0 handles it
```

và:

```text
CPU0
  ↓
remote raise
  ↓
CPU1
  ↓
SoftIRQ processing
```

Tìm hiểu:

* `raise_softirq_on()`
* remote CPU triggering
* IPI nếu được sử dụng
* per-CPU pending state
* CPU hotplug
* migration

---

# 27. PER-CPU DATA

Nghiên cứu:

* `DEFINE_PER_CPU`
* `this_cpu_*`
* `__this_cpu_*`
* per-CPU softirq pending state
* why per-CPU

Giải thích:

> SoftIRQ state được thiết kế per-CPU mang lại lợi ích gì?

Và:

> Nó tránh loại lock nào?

---

# 28. CPU HOTPLUG

Phân tích SoftIRQ khi:

```text
CPU online
CPU offline
CPU dying
CPU migration
```

Các pending SoftIRQ xử lý ra sao?

Tìm source code liên quan.

---

# PHẦN XV — LOCKING

## 29. SoftIRQ Synchronization

Nghiên cứu interaction với:

* spinlock
* raw_spinlock
* local_irq_disable
* local_bh_disable
* `local_bh_enable()`
* preempt disable

Phải giải thích:

```text
local_irq_disable()
vs
local_bh_disable()
vs
preempt_disable()
```

đặc biệt:

> Vì sao network code thường sử dụng `local_bh_disable()`?

---

# 30. BOTTOM HALF

Giải thích khái niệm lịch sử:

```text
Bottom Half
     ↓
SoftIRQ
Tasklet
Workqueue
```

Lịch sử tiến hóa của Linux bottom-half mechanism.

---

# PHẦN XVI — SOFTIRQ VS WORKQUEUE

## 31. SoftIRQ vs Workqueue

Bảng so sánh:

| Feature          | SoftIRQ | Workqueue |
| ---------------- | ------- | --------- |
| Context          | ?       | ?         |
| Can sleep        | ?       | ?         |
| Dedicated thread | ?       | ?         |
| CPU affinity     | ?       | ?         |
| Latency          | ?       | ?         |
| Throughput       | ?       | ?         |
| Typical usage    | ?       | ?         |

Giải thích khi nào nên defer work bằng Workqueue thay vì SoftIRQ.

---

# PHẦN XVII — SOFTIRQ VS TASKLET VS WORKQUEUE

## 32. Deferred Work Mechanisms

Làm bảng:

```text
Hard IRQ
   ↓
SoftIRQ
   ↓
Tasklet
   ↓
Workqueue
   ↓
Kernel Thread
```

Giải thích vị trí và use case của từng cơ chế.

---

# PHẦN XVIII — PREEMPTION

## 33. Preemption + SoftIRQ

Phân tích:

* softirq context preemption
* `preempt_count`
* softirq nesting
* interrupt nesting
* `local_bh_disable`
* scheduler interaction

Giải thích:

> Vì sao SoftIRQ cần cẩn thận với preemption?

---

# PHẦN XIX — PREEMPT_RT

## 34. PREEMPT_RT

Nghiên cứu cách PREEMPT_RT thay đổi semantics của:

* SoftIRQ
* threading
* interrupt
* spinlock
* local_bh_disable

So sánh:

```text
Mainline Linux
vs
PREEMPT_RT
```

Nếu khác version:

> ghi rõ phiên bản.

---

# PHẦN XX — NESTING

## 35. SoftIRQ Nesting

Phân tích:

```text
Hard IRQ
   ↓
SoftIRQ
   ↓
Hard IRQ interrupts CPU
   ↓
returns
   ↓
SoftIRQ continues
```

Tìm hiểu:

* nested interrupt
* nested softirq
* pending bit
* re-entry prevention

---

# PHẦN XXI — STARVATION

## 36. SoftIRQ Starvation

Phân tích các tình huống:

* network flood
* timer flood
* RCU callback flood
* tasklet flood
* softirq raises itself repeatedly

Điều gì xảy ra với:

* user tasks?
* kernel threads?
* scheduler latency?
* ksoftirqd?
* CPU utilization?

---

# 37. SOFTIRQ CPU STARVATION

Giải thích:

```text
CPU
│
├── 90% SoftIRQ
├── 5% kernel
└── 5% user
```

Tại sao có thể xảy ra?

Cách phát hiện?

Cách debugging?

---

# PHẦN XXII — LATENCY

## 38. SoftIRQ Latency

Nghiên cứu:

* interrupt latency
* softirq latency
* scheduling latency
* ksoftirqd latency

Phân biệt:

```text
IRQ latency
SoftIRQ latency
Thread latency
Application latency
```

---

# PHẦN XXIII — SOURCE CODE MAP

## 39. SOURCE TREE

Tạo source map.

Ít nhất tìm hiểu:

```text
kernel/softirq.c
kernel/irq/
kernel/sched/
include/linux/interrupt.h
include/linux/irq.h
include/linux/percpu.h
net/core/dev.c
kernel/time/
kernel/rcu/
```

Nếu version-specific path khác:

> sử dụng path thực tế.

Với mỗi file:

* role
* important functions
* call relationships

---

# 40. FUNCTION INDEX

Tạo bảng:

| Function             | File | Purpose | Called By |
| -------------------- | ---- | ------- | --------- |
| raise_softirq        |      |         |           |
| raise_softirq_irqoff |      |         |           |
| do_softirq           |      |         |           |
| __do_softirq         |      |         |           |
| open_softirq         |      |         |           |
| invoke_softirq       |      |         |           |
| irq_exit             |      |         |           |
| wakeup_softirqd      |      |         |           |
| run_ksoftirqd        |      |         |           |
| local_bh_disable     |      |         |           |
| local_bh_enable      |      |         |           |

Không được điền nếu chưa kiểm tra source.

---

# PHẦN XXIV — FULL CALL TRACE

## 41. Trace toàn bộ lifecycle

Hãy tạo ít nhất các flow sau.

### Flow 1 — Generic SoftIRQ

```text
raise
 ↓
pending
 ↓
irq exit
 ↓
do_softirq
 ↓
__do_softirq
 ↓
handler
```

### Flow 2 — ksoftirqd

```text
raise
 ↓
pending
 ↓
softirq execution deferred
 ↓
wakeup_softirqd
 ↓
ksoftirqd
 ↓
do_softirq
```

### Flow 3 — Network RX

```text
NIC
 ↓
IRQ
 ↓
driver
 ↓
NAPI
 ↓
NET_RX_SOFTIRQ
 ↓
net_rx_action()
 ↓
poll()
 ↓
packet
```

### Flow 4 — Tasklet

```text
tasklet_schedule()
 ↓
TASKLET_SOFTIRQ
 ↓
tasklet_action()
 ↓
callback
```

### Flow 5 — RCU

```text
call_rcu()
 ↓
grace period
 ↓
callback ready
 ↓
RCU softirq
 ↓
rcu_core()
 ↓
callback invocation
```

---

# PHẦN XXV — MEMORY MODEL

## 42. Memory ordering

Phân tích:

* pending bit update
* interrupt disable
* atomicity
* barriers
* `smp_*` barriers
* per-CPU accesses

Phải trả lời:

> Có cần memory barrier giữa producer và SoftIRQ consumer không?

Không đoán.

Phải dựa source code.

---

# PHẦN XXVI — DEBUGGING

## 43. Debug SoftIRQ

Hướng dẫn cách debug bằng:

```bash
cat /proc/softirqs
cat /proc/interrupts
top
htop
ps
```

và:

```bash
trace-cmd
ftrace
perf
```

Nếu có thể:

```text
/proc/softirqs
```

phải giải thích từng counter.

---

# 44. `/proc/softirqs`

Phân tích output ví dụ:

```text
                    CPU0       CPU1
HI                  ...
TIMER               ...
NET_TX              ...
NET_RX              ...
TASKLET             ...
RCU                 ...
```

Giải thích:

* counter là gì?
* counter per-CPU?
* increment ở đâu?
* nhìn counter tăng thì suy luận được gì?
* không thể suy luận điều gì?

---

# 45. DEBUG KSOFTIRQD

Hướng dẫn:

```bash
ps -ef | grep ksoftirqd
top -H
cat /proc/<pid>/status
```

Theo dõi:

```text
ksoftirqd/0
ksoftirqd/1
...
```

Giải thích:

* CPU affinity
* CPU utilization
* scheduling state

---

# PHẦN XXVII — TRACEPOINTS

## 46. SoftIRQ tracing

Tìm các tracepoint liên quan đến:

* irq
* softirq
* sched
* napi
* net

Ví dụ:

```text
irq:softirq_entry
irq:softirq_exit
```

Nếu tracepoint khác version:

> kiểm tra source thật.

Hướng dẫn sử dụng:

```bash
trace-cmd
perf
ftrace
```

để quan sát:

```text
softirq_entry
softirq_exit
```

---

# PHẦN XXVIII — PERF

## 47. Perf analysis

Hướng dẫn phân tích:

```bash
perf top
perf record
perf report
```

để tìm:

* `__do_softirq`
* `net_rx_action`
* `run_ksoftirqd`
* `rcu_core`
* timer handlers

---

# PHẦN XXIX — FTRACE

## 48. Function Graph Trace

Dùng function graph tracer để trace:

```text
irq_exit
 ↓
invoke_softirq
 ↓
do_softirq
 ↓
__do_softirq
```

Nếu function path khác version:

> sửa theo source thực tế.

---

# PHẦN XXX — CODE READING METHOD

## 49. Cách đọc SoftIRQ source

Không được đọc file từ trên xuống một cách máy móc.

Hãy hướng dẫn theo:

```text
Entry Point
    ↓
State
    ↓
Raise
    ↓
Pending
    ↓
Dispatch
    ↓
Handler
    ↓
Defer
    ↓
Threaded execution
```

Tạo source navigation map.

---

# PHẦN XXXI — REAL SCENARIO

## 50. Scenario: Network Flood

Giả sử NIC nhận packet cực lớn.

Trace:

```text
NIC
 ↓
IRQ
 ↓
NAPI
 ↓
NET_RX_SOFTIRQ
 ↓
net_rx_action
 ↓
budget exhausted
 ↓
pending remains
 ↓
ksoftirqd
```

Giải thích chính xác từng bước.

---

# 51. Scenario: RCU Callback Flood

Trace callback backlog.

Giải thích:

```text
callback queue
 ↓
RCU softirq
 ↓
callback execution
 ↓
CPU consumed
```

Liên hệ với RCU stall nếu phù hợp.

---

# 52. Scenario: Timer Flood

Phân tích:

* timer interrupt
* timer softirq
* timer callback
* CPU load

---

# PHẦN XXXII — BUG PATTERNS

## 53. Các bug liên quan SoftIRQ

Phân tích:

* deadlock
* lock inversion
* improper sleeping
* use-after-free
* race condition
* missing `local_bh_disable`
* wrong CPU assumption
* tasklet reentrancy assumptions
* callback lifetime
* softirq starvation

Mỗi case:

```text
Bug
 ↓
Root cause
 ↓
Why it happens
 ↓
How to reproduce
 ↓
How to debug
 ↓
Fix
```

---

# PHẦN XXXIII — SOFTIRQ + DRIVER

## 54. Driver Design

Dùng ví dụ driver:

```text
UART
Network
USB
Storage
```

Phân tích:

```text
ISR
 ↓
deferred work
 ↓
SoftIRQ / tasklet / workqueue
```

Khi nào driver nên chọn cơ chế nào?

---

# PHẦN XXXIV — SOFTIRQ + USB

## 55. USB

Vì tôi đang nghiên cứu USB/EHCI, dành riêng một phần:

* USB interrupt handling
* URB completion
* callback context
* SoftIRQ involvement
* tasklet nếu có
* workqueue nếu có
* bottom-half processing

Không được mặc định rằng mọi USB completion đều chạy trực tiếp trong SoftIRQ.

Phải kiểm tra source path cụ thể.

---

# PHẦN XXXV — SOFTIRQ + NETWORK DRIVER

## 56. NIC Driver

Phân tích lifecycle:

```text
hardware
 ↓
IRQ
 ↓
NAPI schedule
 ↓
NET_RX_SOFTIRQ
 ↓
driver poll
 ↓
skb
 ↓
network stack
```

Theo source code thực tế.

---

# PHẦN XXXVI — SOFTIRQ + RCU

## 57. Quan hệ với RCU

Phân tích:

```text
RCU read-side
RCU update-side
Grace Period
Callback
SoftIRQ
```

Giải thích rõ:

> SoftIRQ không phải là Grace Period.

> RCU callback execution không phải là Grace Period completion.

---

# PHẦN XXXVII — SOFTIRQ VS RCU STALL

## 58. RCU Stall và SoftIRQ

Tìm hiểu:

* softirq starvation
* RCU_SOFTIRQ starvation
* ksoftirqd starvation
* CPU không thực hiện QS
* callback backlog

Liên hệ với:

```text
RCU CPU stall warning
```

Nếu có log thực tế, phân tích log.

---

# PHẦN XXXVIII — ARCHITECTURE

## 59. ARM64

Nếu nghiên cứu ARM64, trace:

```text
exception entry
 ↓
IRQ handler
 ↓
irq_exit
 ↓
softirq
```

Xác định chính xác source:

```text
arch/arm64/
kernel/irq/
kernel/softirq.c
```

---

# 60. POWERPC / OTHER ARCH

Nếu source của tôi là PowerPC hoặc architecture khác:

> phải tracing theo architecture thực tế.

So sánh:

```text
ARM64
vs
PowerPC
```

ở mức:

* interrupt entry
* irq_exit
* softirq invocation
* CPU context

---

# PHẦN XXXIX — MACHINE LEVEL

## 61. CPU-Level View

Giải thích conceptual flow:

```text
CPU enters IRQ
        ↓
register context saved
        ↓
IRQ handler
        ↓
softirq processing
        ↓
restore context
        ↓
return
```

Nếu có kernel assembly:

> chỉ ra nơi architecture chuyển từ exception/IRQ context về kernel C code.

---

# PHẦN XL — PERFORMANCE

## 62. Performance Cost

Phân tích:

* interrupt overhead
* softirq overhead
* indirect dispatch
* cache locality
* per-CPU benefits
* ksoftirqd wakeup
* context switch
* scheduler interaction

---

# 63. TUNING

Nghiên cứu các tuning parameter liên quan.

Ví dụ:

```text
netdev_budget
netdev_budget_usecs
```

và các parameter version-specific.

Không đưa parameter nếu version đang dùng không có.

---

# PHẦN XLI — SOURCE CODE LAB

## 64. Tạo Code Reading Exercises

Tạo các exercise:

### Exercise 1

Tìm `raise_softirq()`.

### Exercise 2

Tìm pending bit storage.

### Exercise 3

Tìm `do_softirq()` caller.

### Exercise 4

Tìm `__do_softirq()`.

### Exercise 5

Tìm `wakeup_softirqd()`.

### Exercise 6

Trace `NET_RX_SOFTIRQ`.

### Exercise 7

Trace `TASKLET_SOFTIRQ`.

### Exercise 8

Trace `RCU_SOFTIRQ`.

### Exercise 9

Trace timer softirq.

### Exercise 10

Trace irq_exit → softirq.

Mỗi exercise phải cho:

* file
* function
* code region
* câu hỏi
* đáp án
* explanation

---

# PHẦN XLII — INTERVIEW

## 65. SoftIRQ Interview Questions

Tạo câu hỏi từ Junior → Senior → Expert.

Ví dụ:

### Junior

* SoftIRQ là gì?
* Tại sao cần SoftIRQ?
* SoftIRQ khác hard IRQ thế nào?

### Intermediate

* SoftIRQ chạy ở context nào?
* SoftIRQ có sleep được không?
* ksoftirqd là gì?
* Tasklet dựa trên gì?

### Senior

* `__do_softirq()` hoạt động thế nào?
* SoftIRQ pending state nằm ở đâu?
* Vì sao pending state là per-CPU?
* SoftIRQ bị giới hạn thời gian như thế nào?
* Khi nào ksoftirqd được đánh thức?
* NAPI liên hệ với NET_RX_SOFTIRQ thế nào?

### Expert

* `preempt_count` biểu diễn softirq context thế nào?
* softirq starvation có thể dẫn tới RCU stall thế nào?
* local_bh_disable() thực sự bảo vệ điều gì?
* remote CPU softirq raise hoạt động ra sao?
* PREEMPT_RT thay đổi SoftIRQ semantics thế nào?

Mỗi câu phải có:

```text
Question
↓
Short answer
↓
Deep answer
↓
Source code
↓
Common trap
```

---

# PHẦN XLIII — CHEAT SHEET

## 66. SoftIRQ Cheat Sheet

Tạo bảng tra cứu nhanh:

```text
raise_softirq
raise_softirq_irqoff
open_softirq
do_softirq
__do_softirq
invoke_softirq
irq_exit
wakeup_softirqd
ksoftirqd
local_bh_disable
local_bh_enable
```

và:

```text
HI_SOFTIRQ
TIMER_SOFTIRQ
NET_TX_SOFTIRQ
NET_RX_SOFTIRQ
TASKLET_SOFTIRQ
RCU_SOFTIRQ
...
```

---

# PHẦN XLIV — MASTER DIAGRAM

## 67. Tạo diagram tổng thể

Phải tạo diagram:

```text
                    HARDWARE
                       │
                       ▼
                 IRQ / Exception
                       │
                       ▼
                Hard IRQ Handler
                       │
                       ▼
                  raise_softirq
                       │
                       ▼
               Per-CPU pending bit
                       │
                       ▼
                   irq_exit()
                       │
              ┌────────┴────────┐
              │                 │
              ▼                 ▼
         do_softirq()       defer / wake
              │                 │
              ▼                 ▼
       __do_softirq()      ksoftirqd/N
              │                 │
              └────────┬────────┘
                       ▼
                 softirq vector
                       │
        ┌──────────────┼──────────────┐
        ▼              ▼              ▼
     NET_RX          TIMER           RCU
        │              │              │
        ▼              ▼              ▼
     NAPI           timers         rcu_core
```

---

# PHẦN XLV — MASTER CALL GRAPH

## 68. Call Graph

Tạo call graph dạng:

```text
raise_softirq()
    │
    └── raise_softirq_irqoff()
             │
             └── __raise_softirq_irqoff()
                       │
                       └── pending bit
```

và:

```text
irq_exit()
    │
    └── invoke_softirq()
             │
             └── do_softirq()
                      │
                      └── __do_softirq()
                               │
                               ├── TIMER_SOFTIRQ
                               ├── NET_RX_SOFTIRQ
                               ├── TASKLET_SOFTIRQ
                               └── RCU_SOFTIRQ
```

Phải điều chỉnh theo source version thực tế.

---

# PHẦN XLVI — STANDARD VS IMPLEMENTATION

Mặc dù Linux kernel không phải C++:

> luôn phân biệt rõ giữa kernel API/contract và implementation detail.

Không được biến implementation hiện tại thành một quy luật tuyệt đối.

Khi có:

```text
architecture-specific
version-specific
CONFIG-specific
PREEMPT_RT-specific
```

phải đánh dấu rõ.

---

# PHẦN XLVII — YÊU CẦU SOURCE CODE

Đối với mỗi function quan trọng:

```text
Function:
File:
Line:
Caller:
Purpose:
Input:
Output:
Context:
Locking:
CPU affinity:
Can sleep?:
Side effects:
```

Nếu có thể, trích đoạn code nhỏ cần thiết và giải thích.

Không dump hàng nghìn dòng source mà không phân tích.

---

# PHẦN XLVIII — OUTPUT FORMAT

Tôi muốn kết quả như một **Linux Kernel SoftIRQ Deep Dive Book**.

Mỗi chapter:

```text
1. Concept
2. Why
3. Architecture
4. Source Code
5. Call Flow
6. CPU Context
7. Per-CPU State
8. Locking
9. SMP
10. Scheduler
11. Performance
12. Debugging
13. Common Bugs
14. Practical Example
15. Summary
```

Không bắt buộc mọi chapter phải có đủ tất cả subsection nếu không relevant.

---

# PHẦN XLIX — QUY TẮC ĐẶC BIỆT

## Rule 1

Không được nói:

> "SoftIRQ chạy sau interrupt."

mà không giải thích:

> "chạy ở đâu, ai gọi, CPU nào, context nào, qua function nào."

---

## Rule 2

Không được nói:

> "ksoftirqd xử lý SoftIRQ."

mà không giải thích:

> "trong trường hợp nào kernel chuyển sang ksoftirqd và vì sao."

---

## Rule 3

Không được nói:

> "SoftIRQ chạy trong kernel thread."

Đây là phát biểu dễ gây hiểu nhầm.

Phải phân biệt:

```text
SoftIRQ execution trực tiếp
vs
SoftIRQ execution trong ksoftirqd
```

---

## Rule 4

Không được nhầm:

```text
SoftIRQ
Tasklet
Workqueue
Kernel Thread
```

Hãy phân biệt execution context của từng cơ chế.

---

## Rule 5

Không được giả định mọi deferred work đều là SoftIRQ.

Luôn trace source code.

---

## Rule 6

Nếu không chắc:

> kiểm tra source code trước khi kết luận.

Không suy đoán.

---

# PHẦN L — FINAL QUESTIONS

Sau toàn bộ nghiên cứu, tôi phải tự trả lời được:

1. SoftIRQ là gì?
2. Tại sao Linux cần SoftIRQ?
3. SoftIRQ khác HardIRQ thế nào?
4. SoftIRQ context là gì?
5. SoftIRQ có sleep được không?
6. Pending SoftIRQ nằm ở đâu?
7. Vì sao pending state là per-CPU?
8. `raise_softirq()` làm gì?
9. `do_softirq()` làm gì?
10. `__do_softirq()` hoạt động ra sao?
11. SoftIRQ handler được dispatch thế nào?
12. `softirq_vec[]` là gì?
13. `irq_exit()` liên quan gì đến SoftIRQ?
14. Khi nào SoftIRQ được xử lý ngay?
15. Khi nào ksoftirqd được wake?
16. ksoftirqd khác SoftIRQ execution trực tiếp thế nào?
17. SoftIRQ time limit là gì?
18. SoftIRQ restart limit là gì?
19. Tasklet dựa trên SoftIRQ như thế nào?
20. TIMER_SOFTIRQ làm gì?
21. NET_RX_SOFTIRQ làm gì?
22. NAPI liên hệ với NET_RX_SOFTIRQ thế nào?
23. RCU_SOFTIRQ làm gì?
24. `local_bh_disable()` bảo vệ điều gì?
25. SoftIRQ hoạt động thế nào trên SMP?
26. Remote CPU raise SoftIRQ thế nào?
27. SoftIRQ starvation là gì?
28. SoftIRQ có thể gây scheduling latency thế nào?
29. SoftIRQ có thể liên quan đến RCU stall thế nào?
30. SoftIRQ và workqueue khác nhau thế nào?
31. SoftIRQ và tasklet khác nhau thế nào?
32. PREEMPT_RT thay đổi SoftIRQ ra sao?
33. `/proc/softirqs` cho biết điều gì?
34. Làm sao debug SoftIRQ?
35. Làm sao dùng ftrace/perf để trace SoftIRQ?
36. Làm sao trace một packet từ NIC interrupt tới network stack?
37. Làm sao trace một tasklet?
38. Làm sao trace RCU callback?
39. Làm sao phát hiện CPU bị SoftIRQ hog?
40. SoftIRQ architecture được tổ chức trong source code như thế nào?

---

# PHẦN LI — FINAL DELIVERABLE

Cuối cùng hãy tạo:

## 1. SoftIRQ Concept Map

```text
IRQ
 ↓
Deferred Work
 ↓
SoftIRQ
 ↓
Tasklet / Networking / Timer / RCU
 ↓
ksoftirqd
 ↓
Scheduler
```

## 2. Source Code Map

```text
File
 ↓
Function
 ↓
Caller
 ↓
Execution Context
```

## 3. Full Lifecycle

```text
RAISE
 ↓
PENDING
 ↓
DISPATCH
 ↓
EXECUTION
 ↓
RESTART / DEFER
 ↓
KSOFTIRQD
```

## 4. Debugging Playbook

Từ:

```text
symptom
 ↓
hypothesis
 ↓
trace
 ↓
evidence
 ↓
root cause
```

## 5. Interview Cheat Sheet

Junior → Senior → Expert.

## 6. Final Summary

Tạo một bản tóm tắt cuối cùng khoảng 2–4 trang nhưng **không thay thế phần nghiên cứu chi tiết**.

---

# MỤC TIÊU CUỐI CÙNG

Tôi muốn có khả năng nhìn vào source Linux Kernel và tự trace được:

```text
Hardware Interrupt
       ↓
IRQ handler
       ↓
SoftIRQ raised
       ↓
per-CPU pending
       ↓
irq_exit
       ↓
do_softirq
       ↓
__do_softirq
       ↓
specific handler
       ↓
possible restart
       ↓
possible ksoftirqd
       ↓
scheduler
```

và với một bug thực tế, tôi phải có thể trả lời:

> **CPU nào đang xử lý SoftIRQ?**

> **SoftIRQ nào đang chiếm CPU?**

> **Ai raise nó?**

> **Handler nào chạy?**

> **Chạy trực tiếp hay qua ksoftirqd?**

> **Có starvation không?**

> **Có liên quan tới scheduler latency không?**

> **Có liên quan tới RCU stall không?**

> **Source code nào chứng minh điều đó?**

Đây phải là một **deep technical research**, ưu tiên:

```text
SOURCE CODE
>
CALL FLOW
>
CONTEXT
>
CPU
>
PER-CPU STATE
>
SCHEDULER
>
SMP
>
MEMORY / LOCKING
>
PERFORMANCE
>
DEBUGGING
```

Không được biến nó thành một bài viết lý thuyết chung chung về SoftIRQ.
