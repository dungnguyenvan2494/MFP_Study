Tôi muốn nghiên cứu cực kỳ chuyên sâu về Linux Process Scheduler trên kiến trúc ARM64 bằng cách đọc trực tiếp Linux kernel source code.

Nguồn tài liệu nền tảng là file "Linux_kernel_chap_7.pdf" – Chapter 7: Process Scheduling.

QUAN TRỌNG:

File PDF này mô tả scheduler Linux phiên bản cũ, chủ yếu dựa trên Linux 2.6/O(1) scheduler và giải thích bằng kiến trúc x86.

Tôi KHÔNG muốn bạn đơn giản dịch các function trong tài liệu sang ARM64.

Thay vào đó:

1. Dùng PDF làm tài liệu nền tảng để hiểu CONCEPT.
2. Sau đó mapping từng concept sang Linux kernel source code hiện đại chạy trên ARM64.
3. Nếu concept trong PDF đã lỗi thời, phải chỉ rõ:
   - PDF mô tả gì
   - Linux kernel hiện đại làm gì
   - Function/structure nào đã thay đổi
   - Vì sao Linux thay đổi
4. Tập trung vào ARM64-specific implementation khi CPU context switch, interrupt, exception, timer và return-from-exception liên quan đến scheduler.

==================================================
PHẦN 1 – XÁC ĐỊNH KERNEL VERSION
==================================================

Trước tiên hãy kiểm tra source tree hiện tại.

Cho tôi biết:

- Kernel version
- ARM64 architecture version/features nếu có
- Scheduler implementation hiện tại
- CONFIG_PREEMPT
- CONFIG_PREEMPT_VOLUNTARY
- CONFIG_PREEMPT_DYNAMIC
- CONFIG_SMP
- CONFIG_HZ
- CONFIG_NO_HZ
- CONFIG_HIGH_RES_TIMERS
- CONFIG_PREEMPT_RCU
- CONFIG_FAIR_GROUP_SCHED
- CONFIG_RT_GROUP_SCHED nếu có

Nếu source tree là Linux 5.4.x thì ưu tiên phân tích đúng source Linux 5.4.x.

KHÔNG được lấy function của kernel version khác rồi giả vờ rằng nó tồn tại trong version đang nghiên cứu.

==================================================
PHẦN 2 – SO SÁNH PDF VỚI KERNEL HIỆN ĐẠI
==================================================

Đọc Chapter 7 trong PDF và lập bảng mapping:

PDF concept
    ↓
Linux kernel hiện đại
    ↓
ARM64-specific code

Ví dụ:

Old O(1) scheduler
    → CFS / RT / Deadline scheduling classes
    → ARM64 context switching

runqueue
    → struct rq
    → ARM64 CPU-local execution

task_struct
    → current
    → ARM64 access_current_task()

scheduler_tick()
    → scheduler_tick()
    → tick handling
    → ARM64 timer interrupt

schedule()
    → __schedule()
    → pick_next_task()
    → context_switch()

context_switch()
    → switch_mm_irqs_off()
    → switch_to()
    → ARM64 switch_to / cpu_switch_to

hãy tự tìm mapping chính xác trong source.

Không được giả định mapping nếu chưa kiểm tra source.

==================================================
PHẦN 3 – HIỂU TOÀN BỘ CALL FLOW
==================================================

Tôi muốn hiểu scheduler từ phần cứng ARM64 cho tới task được chạy.

Hãy phân tích các flow sau:

FLOW A:
ARM64 timer interrupt
    ↓
exception entry
    ↓
timer interrupt handler
    ↓
scheduler tick
    ↓
TIF_NEED_RESCHED
    ↓
return from exception
    ↓
schedule
    ↓
pick next task
    ↓
context switch
    ↓
new task executes

FLOW B:
Task A đang RUNNING
    ↓
Task A gọi:
sleep / wait / mutex / semaphore / read / poll / etc.
    ↓
task chuyển sang sleeping state
    ↓
schedule()
    ↓
Task B được chọn
    ↓
context switch

FLOW C:
Task B đang sleeping
    ↓
interrupt/event xảy ra
    ↓
wake_up()
    ↓
try_to_wake_up()
    ↓
enqueue task
    ↓
check_preempt_curr()
    ↓
TIF_NEED_RESCHED
    ↓
schedule
    ↓
Task B được chạy

FLOW D:
Task A đang chạy
    ↓
Task B có priority/scheduling entity phù hợp hơn
    ↓
preemption
    ↓
scheduler
    ↓
Task B chạy

Với mỗi flow, hãy chỉ rõ:

- CPU mode
- EL0 / EL1
- exception level nếu liên quan
- IRQ state
- current task
- runqueue
- scheduler state
- relevant flags
- relevant struct
- function
- source file
- source line/function definition
- ARM64-specific function

==================================================
PHẦN 4 – ARM64 TIMER → SCHEDULER
==================================================

Đây là phần tôi muốn hiểu cực kỳ sâu.

Hãy trace:

ARM Generic Timer
    ↓
ARM64 interrupt
    ↓
GIC
    ↓
IRQ entry
    ↓
generic IRQ subsystem
    ↓
timer interrupt handler
    ↓
tick subsystem
    ↓
scheduler_tick()
    ↓
TIF_NEED_RESCHED

Tôi muốn source-level explanation.

Tìm chính xác:

arch/arm64/kernel/
drivers/clocksource/
kernel/time/
kernel/sched/
include/linux/

và các file liên quan.

Giải thích:

- ARM Generic Timer tạo interrupt như thế nào?
- GIC chuyển interrupt tới CPU như thế nào?
- ARM64 exception vector hoạt động thế nào?
- Linux vào IRQ handler bằng code nào?
- interrupt context khác process context thế nào?
- scheduler_tick() được gọi từ đâu?
- scheduler xác định task đã hết time slice như thế nào?
- TIF_NEED_RESCHED được set ở đâu?
- Tại sao scheduler thường không gọi schedule() trực tiếp từ IRQ handler?
- schedule() được gọi lúc nào sau interrupt?
- ARM64 exception return liên quan thế nào tới rescheduling?

Vẽ sequence diagram.

==================================================
PHẦN 5 – current TASK TRÊN ARM64
==================================================

Tìm hiểu cực sâu:

current
current_thread_info()
thread_info
task_struct
sp_el0
TPIDR_EL1 / TPIDRRO_EL0 nếu liên quan
per-CPU data
this_cpu
get_current()

Giải thích chính xác Linux ARM64 lấy:

current task

bằng cách nào.

Trace từ CPU register → kernel → current task.

Nếu implementation thay đổi theo kernel version, chỉ rõ.

Đặc biệt giải thích:

"current" có phải global variable không?

Làm thế nào CPU0 và CPU1 có current khác nhau?

Context switch làm current thay đổi ở đâu?

==================================================
PHẦN 6 – struct task_struct
==================================================

Phân tích các scheduler-related fields trong:

struct task_struct

Không cần dump toàn bộ struct.

Chỉ tập trung:

- state
- __state
- on_rq
- prio
- static_prio
- normal_prio
- rt_priority
- policy
- sched_class
- se
- rt
- dl
- cpu
- wake_cpu
- cpus_ptr
- nr_cpus_allowed
- on_cpu
- preempt_count
- flags
- thread
- mm
- active_mm

Với mỗi field:

1. Ý nghĩa
2. Ai update
3. Ai đọc
4. Khi nào thay đổi
5. Liên quan scheduler như thế nào
6. ARM64 có gì đặc biệt không

==================================================
PHẦN 7 – struct rq
==================================================

Phân tích cực sâu:

struct rq

Giải thích:

- rq là gì?
- mỗi CPU có một rq riêng hay không?
- rq được lưu ở đâu?
- per-CPU như thế nào?
- CPU0 và CPU1 có rq khác nhau như thế nào?

Phân tích các field quan trọng:

- curr
- idle
- nr_running
- load
- cfs
- rt
- dl
- cpu
- clock
- clock_task
- idle_stamp
- avg_rt
- avg_dl
- nohz
- core nếu có

Không được chỉ giải thích lý thuyết.

Hãy tìm definition thật trong source.

==================================================
PHẦN 8 – SCHEDULING CLASSES
==================================================

Giải thích kiến trúc:

sched_class

và thứ tự ưu tiên giữa:

- stop
- deadline
- RT
- fair
- idle

Trace:

pick_next_task()

hoặc implementation tương ứng trong kernel version hiện tại.

Tôi muốn biết chính xác:

"Scheduler quyết định task tiếp theo chạy bằng cách nào?"

Hãy trace source code.

Ví dụ:

__schedule()
    ↓
pick_next_task()
    ↓
pick_next_task_fair()
    ↓
pick_next_entity()
    ↓
...

Nhưng KHÔNG được tự giả định call chain.

Hãy lấy call chain thật từ source.

==================================================
PHẦN 9 – CFS
==================================================

Phân tích CFS cực sâu.

Giải thích:

- sched_entity
- vruntime
- cfs_rq
- rb_tree
- rb_node
- load
- weight
- nice
- load_weight

Trace:

enqueue_task_fair()
    ↓
enqueue_entity()
    ↓
update_curr()
    ↓
vruntime

và:

pick_next_task_fair()
    ↓
pick_next_entity()

Giải thích tại sao task có vruntime nhỏ thường được chọn.

Vẽ:

CPU
 |
 +-- rq
      |
      +-- cfs_rq
             |
             +-- RB tree
                    |
                    +-- task A
                    +-- task B
                    +-- task C

==================================================
PHẦN 10 – PREEMPTION
==================================================

Phân tích cực sâu:

preemption

Phân biệt:

1. voluntary preemption
2. involuntary preemption
3. preemption from interrupt
4. preemption from wakeup

Trace:

check_preempt_curr()

wakeup_preempt()

TIF_NEED_RESCHED

preempt_count

need_resched()

resched_curr()

Giải thích:

Tại sao IRQ handler không đơn giản gọi schedule()?

Tại sao kernel phải đợi tới safe point?

ARM64 safe point nằm ở đâu?

==================================================
PHẦN 11 – TIF_NEED_RESCHED TRÊN ARM64
==================================================

Đây là một topic bắt buộc.

Trace chính xác:

set_tsk_need_resched()
    ↓
TIF_NEED_RESCHED
    ↓
need_resched()
    ↓
exception return / preempt path
    ↓
schedule()

Tìm ARM64 code xử lý:

TIF_NEED_RESCHED

đặc biệt trong:

arch/arm64/kernel/entry.S
arch/arm64/kernel/
include/linux/

Giải thích assembly nếu có.

Tôi muốn biết chính xác:

CPU đang chạy task A.
Timer interrupt xảy ra.
Scheduler quyết định A phải bị preempt.

Từ thời điểm đó cho tới khi CPU chạy task B:

CPU register thay đổi như thế nào?

==================================================
PHẦN 12 – schedule() SOURCE CODE
==================================================

Trace:

schedule()
    ↓
__schedule()
    ↓
preempt_disable
    ↓
rq lock
    ↓
prev = current
    ↓
put_prev_task()
    ↓
pick_next_task()
    ↓
next
    ↓
context_switch()
    ↓
switch_to()

Hãy kiểm tra source thực tế và điều chỉnh call flow nếu kernel version hiện tại khác.

Giải thích từng dòng quan trọng.

Đặc biệt:

- rq lock
- IRQ disable
- preempt_disable
- prev
- next
- deactivate_task
- activate_task
- context_switch

==================================================
PHẦN 13 – CONTEXT SWITCH TRÊN ARM64
==================================================

Đây là phần quan trọng nhất.

Trace:

context_switch()
    ↓
switch_mm_irqs_off()
    ↓
switch_to()
    ↓
__switch_to()
    ↓
cpu_switch_to()

hoặc call chain thực tế của kernel version đang nghiên cứu.

Phân tích ARM64 register context:

- x0-x30
- SP
- PC
- PSTATE
- SP_EL0
- TTBR0_EL1
- TTBR1_EL1
- TPIDR_EL0
- TPIDR_EL1
- FP/SIMD registers nếu có
- debug registers nếu liên quan

Giải thích:

Task A:

PC = A1
SP = A2
registers = A3

sau context switch:

Task B:

PC = B1
SP = B2
registers = B3

CPU làm thế nào để "tiếp tục" Task B đúng instruction trước đó?

Tôi muốn hiểu chính xác mechanism này.

==================================================
PHẦN 14 – cpu_switch_to() ARM64
==================================================

Tìm implementation thật của:

cpu_switch_to()

Phân tích assembly instruction-by-instruction.

Ví dụ nếu source có:

stp
ldp
mov
ret

thì giải thích:

- register nào được save
- memory address nào
- stack nào
- task_struct/thread_struct nào
- LR được save ở đâu
- PC được restore bằng cách nào

Không được giải thích assembly một cách chung chung.

Hãy liên kết từng instruction với struct/thread context thật.

==================================================
PHẦN 15 – MM SWITCH
==================================================

Phân tích:

switch_mm()
switch_mm_irqs_off()
switch_mm_context()

và ARM64 MMU/TTBR.

Giải thích:

Task A address space
    ↓
TTBR0_EL1
    ↓
context switch
    ↓
Task B address space
    ↓
TTBR0_EL1

Giải thích:

- user virtual address space
- kernel address space
- TTBR0_EL1
- TTBR1_EL1
- ASID
- TLB
- context switch overhead

Phân biệt:

kernel thread switch

và

user process switch.

==================================================
PHẦN 16 – KERNEL THREAD
==================================================

So sánh:

User process
vs
Kernel thread

Ví dụ:

bash
kworker
ksoftirqd
migration thread
idle thread

Trace context switch giữa:

user process A
→
kernel thread
→
user process B

Giải thích mm = NULL / active_mm nếu kernel version còn dùng concept đó.

==================================================
PHẦN 17 – WAKEUP PATH
==================================================

Trace source thật:

wake_up()
    ↓
__wake_up()
    ↓
try_to_wake_up()
    ↓
ttwu()
    ↓
ttwu_queue()
    ↓
enqueue_task()
    ↓
check_preempt_curr()

Điều chỉnh call chain theo kernel version.

Giải thích:

Task đang:

TASK_INTERRUPTIBLE

sau đó:

IRQ/event
    ↓
wake_up()
    ↓
TASK_RUNNING
    ↓
enqueue vào rq
    ↓
scheduler

Đặc biệt giải thích wakeup trên SMP ARM64.

==================================================
PHẦN 18 – SMP / MULTI-CORE ARM64
==================================================

Phân tích:

CPU0
CPU1
CPU2
CPU3

mỗi CPU có:

per-CPU rq

Giải thích:

- task migration
- load balancing
- CPU affinity
- cpumask
- sched_domain
- sched_group
- idle_balance
- load_balance
- migration thread

Trace:

CPU0 overloaded
    ↓
load balancing
    ↓
find_busiest_group()
    ↓
detach task
    ↓
CPU1
    ↓
attach task

Tìm source thật.

==================================================
PHẦN 19 – ARM64 CPU HOTPLUG
==================================================

Tìm hiểu scheduler khi CPU online/offline.

Flow:

CPU offline
    ↓
migrate tasks
    ↓
stop scheduling tasks on CPU
    ↓
CPU shutdown

và:

CPU online
    ↓
initialize rq
    ↓
scheduler setup
    ↓
CPU becomes schedulable

Trace source.

==================================================
PHẦN 20 – SYSTEM CALLS
==================================================

PDF có phần scheduling-related system calls.

Hãy mapping chúng sang ARM64 syscall implementation hiện đại:

nice()
getpriority()
setpriority()
sched_getscheduler()
sched_setscheduler()
sched_getparam()
sched_setparam()
sched_yield()
sched_get_priority_min()
sched_get_priority_max()
sched_rr_get_interval()

Trace:

userspace
    ↓
SVC #0
    ↓
ARM64 syscall entry
    ↓
syscall table
    ↓
SYSCALL_DEFINE(...)
    ↓
scheduler code

Giải thích ARM64 syscall entry.

Tôi muốn thấy:

x8 = syscall number
x0-x5 = arguments
SVC #0
    ↓
EL1
    ↓
syscall handler
    ↓
return to EL0

==================================================
PHẦN 21 – ARM64 EXCEPTION ENTRY
==================================================

Tìm hiểu:

arch/arm64/kernel/entry.S

hoặc file tương ứng kernel version.

Trace:

EL0
 ↓
SVC/IRQ
 ↓
vector
 ↓
pt_regs
 ↓
EL1 handler
 ↓
return
 ↓
EL0

Giải thích:

- vector table
- exception level
- SPSR_EL1
- ELR_EL1
- ESR_EL1
- FAR_EL1
- pt_regs
- kernel stack

Sau đó liên kết với scheduler.

==================================================
PHẦN 22 – INTERRUPT PREEMPTION
==================================================

Hãy dựng một scenario cực cụ thể:

CPU0:

Task A đang chạy ở EL1.

Tại thời điểm:

T = 1000 us

ARM Generic Timer interrupt.

Trace toàn bộ:

1. CPU exception
2. save registers
3. vector entry
4. GIC
5. IRQ subsystem
6. timer handler
7. scheduler tick
8. update runtime
9. scheduler quyết định preempt
10. set TIF_NEED_RESCHED
11. interrupt return
12. preemption check
13. schedule
14. pick Task B
15. context switch
16. restore Task B context
17. CPU tiếp tục Task B

Tạo sequence diagram.

==================================================
PHẦN 23 – DEBUG SOURCE
==================================================

Không chỉ đọc code.

Hãy hướng dẫn tôi build kernel ARM64 có debug symbols.

Sau đó hướng dẫn:

QEMU ARM64

hoặc board ARM64 thật.

Dùng:

gdb
kgdb
ftrace
trace-cmd
perf
bpftrace
/proc/sched_debug
/proc/schedstat
/sys/kernel/debug/tracing/

Để quan sát:

schedule
sched_switch
sched_wakeup
sched_wakeup_new
sched_process_fork
sched_process_exit

==================================================
PHẦN 24 – TRACE THỰC TẾ
==================================================

Tạo một chương trình:

Task A
Task B

sau đó:

Task A sleep
Task B wake
Task A wake

Dùng ftrace/perf để quan sát:

sched_switch

Tôi muốn đối chiếu:

SOURCE CODE

với

RUNTIME TRACE.

Ví dụ:

Task A
 ↓
schedule()
 ↓
pick Task B
 ↓
context_switch()
 ↓
sched_switch event
 ↓
Task B

==================================================
PHẦN 25 – SOURCE CODE STUDY METHOD
==================================================

Mỗi function phải được phân tích theo format:

----------------------------------------
FUNCTION: __schedule()
----------------------------------------

FILE:
kernel/sched/core.c

PURPOSE:
...

CALLERS:
...

CALLEES:
...

INPUT:
...

OUTPUT:
...

LOCK:
...

IRQ STATE:
...

PREEMPT STATE:
...

IMPORTANT STRUCTURES:
...

STEP-BY-STEP:

1.
2.
3.
4.
5.

ARM64 IMPACT:
...

RELATED ASM:
...

RELATED DATA STRUCTURES:
...

CALL GRAPH:

A
|
v
B
|
v
C

----------------------------------------

Không được chỉ nói function "làm nhiệm vụ scheduling".

Tôi muốn biết chính xác nó làm gì ở source level.

==================================================
PHẦN 26 – DIAGRAM
==================================================

Sau mỗi topic hãy tạo:

1. Architecture diagram
2. Sequence diagram
3. Call graph
4. State machine nếu phù hợp
5. Data structure relationship

Ưu tiên Mermaid.

Ví dụ:

```mermaid
sequenceDiagram
    participant CPU
    participant ARM64
    participant IRQ
    participant TIMER
    participant SCHED
    participant TASK

    CPU->>ARM64: Timer IRQ
    ARM64->>IRQ: Exception entry
    IRQ->>TIMER: Timer handler
    TIMER->>SCHED: scheduler_tick()
    SCHED->>TASK: update runtime
    SCHED->>TASK: set need_resched
    ARM64->>SCHED: schedule()
    SCHED->>SCHED: pick_next_task()
    SCHED->>ARM64: context_switch()
    ARM64->>TASK: Resume next task

Nhưng phải sửa diagram theo source code thực tế.

==================================================
PHẦN 27 – OLD LINUX 2.6 VS MODERN LINUX

Đây là phần bắt buộc.

Tạo bảng:

PDF Linux 2.6 concept	Modern Linux	ARM64
O(1) scheduler	?	?
active/expired array	?	?
static_prio	?	?
dynamic priority	?	?
time slice	?	?
runqueue	?	?
scheduler_tick	?	?
schedule	?	?
context_switch	?	?
load_balance	?	?
sched_domain	?	?

Giải thích những concept đã bị loại bỏ.

Đặc biệt:

PDF mô tả:

active array
expired array
prio_array

Hãy chỉ ra tại sao chúng không còn là cơ chế chính của modern scheduler.

==================================================
PHẦN 28 – VERSION-AWARE ANALYSIS

Mỗi khi phát hiện:

PDF function:

foo()

nhưng source hiện tại không có foo()

KHÔNG được nói:

"foo() tương đương bar()"

ngay lập tức.

Hãy:

git log
git blame
git grep
tìm commit thay đổi
tìm function replacement
giải thích architectural reason

Nếu có thể, sử dụng git history để giải thích evolution.

==================================================
PHẦN 29 – SOURCE COMMANDS

Trong source tree hãy sử dụng:

git grep
rg
grep
cscope
ctags
git blame
git log -S
git log -G

Ví dụ:

rg "TIF_NEED_RESCHED"
rg "struct rq"
rg "struct sched_class"
rg "context_switch"
rg "cpu_switch_to"
rg "__schedule"
rg "scheduler_tick"
rg "try_to_wake_up"

Khi đưa ra kết luận, hãy chỉ rõ:

FILE
FUNCTION
STRUCT
MACRO
CALLER
CALLEE

==================================================
PHẦN 30 – FINAL GOAL

Mục tiêu cuối cùng là tôi phải có thể tự trả lời:

Scheduler nằm ở đâu trong Linux kernel?
current là gì?
task_struct là gì?
rq là gì?
mỗi CPU có rq riêng như thế nào?
scheduler chọn task tiếp theo bằng cách nào?
CFS hoạt động như thế nào?
vruntime là gì?
preemption hoạt động như thế nào?
TIF_NEED_RESCHED dùng để làm gì?
timer interrupt liên quan scheduler như thế nào?
ARM64 exception entry hoạt động như thế nào?
ARM64 context switch save/restore register như thế nào?
cpu_switch_to() làm gì?
switch_mm() làm gì?
kernel thread khác user process thế nào?
wake_up() làm task chạy lại như thế nào?
SMP load balancing hoạt động thế nào?
CPU affinity hoạt động thế nào?
sched_domain dùng làm gì?
syscall scheduler đi qua ARM64 SVC như thế nào?
từ Timer IRQ đến context switch mất những bước nào?
==================================================
YÊU CẦU QUAN TRỌNG NHẤT

Đừng dạy tôi scheduler chỉ bằng lý thuyết.

Tôi muốn học theo phương pháp:

CONCEPT
↓
SOURCE CODE
↓
CALL GRAPH
↓
DATA STRUCTURE
↓
ARM64 ASSEMBLY
↓
CPU HARDWARE
↓
RUNTIME TRACE

Mỗi kết luận phải ưu tiên source code thực tế.

Nếu PDF và source kernel hiện tại khác nhau:
→ nói rõ sự khác biệt.

Nếu ARM64 khác x86:
→ giải thích chính xác khác ở đâu.

Nếu một concept không còn tồn tại:
→ nói rõ nó đã bị thay thế bằng gì và tại sao.

Không được hallucinate function hoặc source path.

Nếu chưa chắc:
→ tìm source trước rồi mới kết luận.

Hãy bắt đầu từ:

Chapter 7 PDF
↓
Scheduling concept
↓
Modern Linux scheduler
↓
ARM64 implementation

và cuối cùng xây dựng cho tôi một "Linux ARM64 Scheduler Source Code Map" để tôi có thể dùng nó làm roadmap đọc kernel source.


---

## Tôi khuyên bạn học theo thứ tự này

File của bạn thực ra có một cấu trúc rất tốt để làm **khung lý thuyết**, dù implementation đã cũ.

### Phase 1 — Concept từ PDF

Đầu tiên đọc:

**1. Scheduling Policy**

Hiểu:

```text
process
   ↓
priority
   ↓
time slice
   ↓
preemption

Phần này nằm khoảng trang 3–6 của chapter. PDF giải thích process preemption và vấn đề lựa chọn quantum khá rõ.

Sau đó đọc phần algorithm ở trang 7–12 để hiểu tư duy scheduler cũ:

RUNNING
   ↓
scheduler
   ↓
priority
   ↓
time slice
   ↓
next process

Phase 2 — Data structure

Sau đó chuyển sang phần rất quan trọng:

struct task_struct
        │
        ▼
struct rq
        │
        ├── CFS
        ├── RT
        └── DL

PDF dành trang 13–18 cho runqueue và process descriptor. Đây là phần cực hữu ích để hình thành mental model, mặc dù cấu trúc scheduler hiện đại đã khác đáng kể.

Phase 3 — Scheduler core

Tiếp theo tập trung vào:

scheduler_tick()
        │
        ▼
try_to_wake_up()
        │
        ▼
recalc_task_prio()
        │
        ▼
schedule()
        │
        ▼
context_switch()

PDF dành một đoạn rất lớn từ trang 19 đến 34 để giải thích các operation này.

Đây chính là chỗ bạn nên chuyển từ PDF sang source Linux 5.4 ARM64.

Phase 4 — SMP

Sau khi hiểu single CPU:

CPU0
 └── rq0

CPU1
 └── rq1

CPU2
 └── rq2

CPU3
 └── rq3

mới học:

        Scheduler
            │
     ┌──────┴──────┐
     ▼             ▼
   CPU0           CPU1
    rq             rq
     │             │
     └──── load ───┘
          balance

PDF có riêng phần Runqueue Balancing in Multiprocessor Systems, gồm scheduling domains, rebalance_tick(), load_balance() và move_tasks().

Và đặc biệt: đừng bắt đầu bằng schedule()

Nếu mục tiêu của bạn là thật sự hiểu ARM64 scheduler, tôi khuyên roadmap source code là:

                    ARM64 CPU
                       │
                       ▼
              Generic Timer / GIC
                       │
                       ▼
              ARM64 Exception Entry
                       │
                       ▼
                  IRQ Handler
                       │
                       ▼
               scheduler_tick()
                       │
                       ▼
                need_resched
                       │
                       ▼
                __schedule()
                       │
              ┌────────┴────────┐
              ▼                 ▼
        current task       next task
              │                 │
              └────────┬────────┘
                       ▼
               context_switch()
                       │
             ┌─────────┴─────────┐
             ▼                   ▼
        switch_mm()        cpu_switch_to()
             │                   │
             ▼                   ▼
       ARM64 MMU/ASID       ARM64 registers
                                 │
                                 ▼
                              Task B

Nếu bạn hiểu được flow này từ source code đến ARM64 assembly, thì lúc đó bạn không chỉ "biết Linux scheduler", mà đã hiểu được CPU thực sự chuyển từ task A sang task B như thế nào.

Một điểm rất đáng chú ý: phần cuối PDF cũng đi xuống tận scheduling-related system calls như nice(), getpriority(), setpriority(), sched_setscheduler(), sched_yield() và sched_rr_get_interval(). Vì vậy sau scheduler core, bạn có thể nối tiếp sang ARM64 syscall entry → scheduler syscall → kernel scheduler → return EL0 để có một bức tranh hoàn chỉnh.