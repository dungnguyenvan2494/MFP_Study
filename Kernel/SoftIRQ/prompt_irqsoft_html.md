# PROMPT — TẠO FILE HTML TỪ TOÀN BỘ DỮ LIỆU NGHIÊN CỨU LINUX KERNEL SOFTIRQ

Tôi đã có **TOÀN BỘ dữ liệu nghiên cứu về Linux Kernel SoftIRQ** được tạo từ một prompt Deep Dive trước đó.

Nhiệm vụ của bạn:

> Đọc toàn bộ dữ liệu nghiên cứu tôi cung cấp và chuyển nó thành **MỘT FILE HTML duy nhất**, đóng vai trò như một **Linux Kernel SoftIRQ Personal Reference Book**.

Đây là nhiệm vụ:

> **STRUCTURE + FORMAT + VISUALIZE + ORGANIZE**

Không phải nhiệm vụ:

> **SUMMARY / SHORTEN / SIMPLIFY**

---

# 1. MỤC TIÊU

HTML cuối cùng phải cho phép tôi:

* học SoftIRQ từ đầu
* tra cứu một function
* trace một call chain
* hiểu execution context
* xem CPU/per-CPU state
* xem relationship giữa IRQ / SoftIRQ / ksoftirqd
* nghiên cứu NAPI / NET_RX_SOFTIRQ
* nghiên cứu Tasklet
* nghiên cứu Timer SoftIRQ
* nghiên cứu RCU_SOFTIRQ
* nghiên cứu SMP
* nghiên cứu `local_bh_disable()`
* debug SoftIRQ
* phân tích `/proc/softirqs`
* nghiên cứu SoftIRQ starvation
* liên hệ SoftIRQ với RCU stall
* đọc source Linux Kernel
* ôn interview

File phải giống một:

> **Technical Engineering Documentation / Kernel Internals Book**

chứ không phải landing page.

---

# 2. QUY TẮC QUAN TRỌNG NHẤT — KHÔNG ĐƯỢC MẤT NỘI DUNG

Toàn bộ research data là **source of truth**.

Không được:

* tự ý tóm tắt
* xóa explanation
* bỏ source code
* bỏ call flow
* bỏ memory diagram
* bỏ CPU/context analysis
* bỏ version-specific details
* bỏ architecture-specific details
* bỏ performance analysis
* bỏ debugging procedure
* bỏ examples
* bỏ interview questions
* bỏ caveats

Nếu nội dung quá dài:

> phải giữ nguyên và tổ chức thành nhiều subsection.

Không được giải quyết vấn đề độ dài bằng cách cắt nội dung.

---

# 3. NGUYÊN TẮC BIÊN TẬP

Hãy thực hiện:

```text
RAW RESEARCH
      ↓
CLASSIFY
      ↓
DE-DUPLICATE
      ↓
STRUCTURE
      ↓
CROSS-REFERENCE
      ↓
VISUALIZE
      ↓
HTML
```

Nếu hai đoạn trùng nhau:

* merge chúng nếu thực sự cùng một nội dung
* giữ lại các technical details khác nhau
* không làm mất thông tin version-specific

Nếu phát hiện contradiction:

> KHÔNG tự ý chọn một bên và xóa bên còn lại.

Hãy trình bày:

```text
Version A
Version B
Reason for difference
Relevant kernel version
```

---

# 4. OUTPUT

Chỉ tạo **một file**:

```text
linux-kernel-softirq-deep-dive.html
```

HTML phải tự chạy bằng:

* Chrome
* Edge
* Firefox

Không cần:

* backend
* server
* build
* npm
* framework

Tất cả nằm trong:

```html
<!DOCTYPE html>
<html>
<head>
    <style>
        ...
    </style>
</head>
<body>
    ...
    <script>
        ...
    </script>
</body>
</html>
```

---

# 5. OFFLINE-FIRST

File phải hoạt động offline.

Không phụ thuộc:

* CDN
* external CSS
* external JavaScript
* external fonts
* external image host

Ưu tiên:

* CSS thuần
* Vanilla JavaScript
* SVG inline
* HTML/CSS diagrams

---

# 6. VISUAL STYLE

Phong cách:

> Linux Kernel Documentation + Modern Engineering Knowledge Base

Yêu cầu:

* tối giản
* technical
* sạch
* mật độ thông tin cao nhưng dễ đọc
* ít màu
* không gradient sặc sỡ
* không animation thừa
* không card quá bo tròn
* không “dashboard hóa” tài liệu

Ưu tiên:

```text
Typography
Hierarchy
Spacing
Code readability
Technical diagrams
Navigation
```

hơn decoration.

---

# 7. FONT

Dùng system fonts để tránh lỗi.

Ví dụ:

```css
font-family:
    Inter,
    ui-sans-serif,
    system-ui,
    -apple-system,
    BlinkMacSystemFont,
    "Segoe UI",
    sans-serif;
```

Code:

```css
font-family:
    "JetBrains Mono",
    "Cascadia Code",
    Consolas,
    monospace;
```

Không phụ thuộc font bên ngoài.

Bắt buộc:

```html
<meta charset="UTF-8">
```

để không xảy ra lỗi tiếng Việt / Unicode.

---

# 8. PAGE LAYOUT

Thiết kế:

```text
┌──────────────────────────────────────────────────────────────┐
│ HEADER                                                       │
│ Linux Kernel SoftIRQ Deep Dive                              │
├────────────────┬─────────────────────────────────────────────┤
│                │                                             │
│ SIDEBAR        │                 MAIN CONTENT                │
│                │                                             │
│ Search         │                                             │
│ Contents       │                                             │
│ Sections       │                                             │
│ Functions      │                                             │
│ Concepts       │                                             │
│                │                                             │
├────────────────┴─────────────────────────────────────────────┤
│ FOOTER                                                       │
└──────────────────────────────────────────────────────────────┘
```

Sidebar sticky.

Main content width phù hợp đọc technical documentation.

---

# 9. HEADER

Header:

```text
Linux Kernel
SoftIRQ Deep Dive
```

Subtitle:

```text
IRQ → SoftIRQ → Per-CPU Pending → do_softirq → ksoftirqd → Scheduler
```

Có thể thêm:

```text
Kernel Internals
Source Code Deep Dive
```

Nếu research xác định một kernel version cụ thể thì hiển thị:

```text
Linux 5.10.x
```

hoặc version thực tế.

---

# 10. SIDEBAR / TABLE OF CONTENTS

Tạo navigation đầy đủ.

Gợi ý structure:

```text
00. Overview
01. SoftIRQ Fundamentals
02. Interrupt Context
03. SoftIRQ Vectors
04. raise_softirq()
05. Pending State
06. open_softirq()
07. do_softirq()
08. __do_softirq()
09. SoftIRQ Restart / Time Limit
10. ksoftirqd
11. irq_exit()
12. Interrupt Return Path
13. Scheduler Interaction
14. Tasklet
15. Timer SoftIRQ
16. NET_RX_SOFTIRQ
17. NAPI
18. RCU_SOFTIRQ
19. SMP / Remote SoftIRQ
20. Per-CPU Data
21. local_bh_disable()
22. Locking / Synchronization
23. Preemption
24. PREEMPT_RT
25. SoftIRQ Starvation
26. Latency
27. Source Code Map
28. Function Index
29. Full Call Graphs
30. ARM64
31. PowerPC / Other Architectures
32. Driver Integration
33. USB
34. Network Driver
35. Debugging
36. /proc/softirqs
37. Tracepoints
38. ftrace
39. perf
40. Real Scenarios
41. Common Bugs
42. Performance
43. Tuning
44. Interview
45. Exercises
46. Cheat Sheet
47. Master Architecture Map
```

Nếu research có thêm section:

> phải giữ lại.

---

# 11. SIDEBAR FEATURES

Sidebar phải có:

### Search

```text
Search SoftIRQ...
```

Hỗ trợ:

* title
* text
* function
* keyword

### Section navigation

Click → scroll tới section.

### Active section

Highlight section hiện tại khi scroll.

### Collapse/Expand

Subsections có thể collapse nếu tài liệu quá dài.

---

# 12. KEYWORD SEARCH

Implement bằng Vanilla JS.

Search phải hỗ trợ các từ như:

```text
softirq
ksoftirqd
raise_softirq
do_softirq
__do_softirq
NET_RX_SOFTIRQ
TASKLET_SOFTIRQ
RCU_SOFTIRQ
NAPI
irq_exit
local_bh_disable
percpu
preempt_count
starvation
```

Khi search:

* highlight match
* hiển thị số kết quả
* cho phép next/previous nếu hợp lý
* clear search

Thêm:

```text
Ctrl + K
```

để focus search.

---

# 13. CORE CONCEPT BOX

Các concept quan trọng phải có box:

```text
CORE CONCEPT
```

Ví dụ:

```text
SoftIRQ is deferred execution in atomic context.
```

Nhưng:

> Nội dung trong box phải lấy từ research data.

Không được tạo claim trái với source.

---

# 14. WARNING BOX

Tạo:

```text
WARNING
```

cho:

* cannot sleep
* lock misuse
* starvation
* incorrect context assumption
* callback lifetime
* race condition

---

# 15. VERSION-SPECIFIC BOX

Tạo box:

```text
KERNEL VERSION
```

để hiển thị:

```text
Linux 5.10.x
```

hoặc version thực tế của research.

Mọi version-dependent statement nên được đánh dấu rõ.

---

# 16. SOURCE CODE BOX

Tạo style riêng:

```text
SOURCE:
kernel/softirq.c
FUNCTION:
__do_softirq()
```

Thông tin:

```text
File
Function
Approximate line
Caller
Context
```

Không được tự bịa line number.

Nếu research không có line number:

> không tạo line number giả.

---

# 17. CODE BLOCK

Ví dụ:

```c
void __do_softirq(void)
{
    ...
}
```

Code phải:

* monospace
* syntax highlighting nếu có thể
* line numbers nếu hữu ích
* copy button
* giữ indentation
* horizontal scroll

Nút:

```text
Copy
```

sau khi copy:

```text
Copied!
```

---

# 18. SOURCE CODE ANNOTATION

Nếu research có code explanation:

```text
source line
     ↓
explanation
```

hãy thể hiện bằng:

```text
Code
Explanation
```

hoặc:

```text
Annotated Code
```

Không bỏ phần phân tích từng đoạn.

---

# 19. CALL FLOW

Các call flow phải được trực quan hóa.

Ví dụ:

```text
raise_softirq()
        ↓
raise_softirq_irqoff()
        ↓
pending bit
        ↓
irq_exit()
        ↓
invoke_softirq()
        ↓
do_softirq()
        ↓
__do_softirq()
        ↓
handler
```

Dùng HTML/CSS hoặc SVG inline.

---

# 20. MASTER SOFTIRQ FLOW

Phải có một diagram cực rõ:

```text
                     HARDWARE
                         │
                         ▼
                    IRQ ENTRY
                         │
                         ▼
                  HARD IRQ HANDLER
                         │
                         ▼
                  raise_softirq()
                         │
                         ▼
                  PER-CPU PENDING
                         │
                         ▼
                     irq_exit()
                         │
              ┌──────────┴──────────┐
              │                     │
              ▼                     ▼
        do_softirq()          defer / wakeup
              │                     │
              ▼                     ▼
       __do_softirq()          ksoftirqd/N
              │                     │
              └──────────┬──────────┘
                         ▼
                   SOFTIRQ VECTOR
                         │
        ┌────────────────┼────────────────┐
        ▼                ▼                ▼
     NET_RX           TIMER             RCU
        │                │                │
        ▼                ▼                ▼
      NAPI             timers          rcu_core
```

Nếu research có flow khác:

> điều chỉnh theo source thực tế.

---

# 21. CONTEXT DIAGRAM

Tạo diagram:

```text
┌───────────────────────────────┐
│ Process Context               │
│ Can sleep                     │
└───────────────┬───────────────┘
                │
                ▼
┌───────────────────────────────┐
│ ksoftirqd/N                   │
│ Kernel Thread Context         │
└───────────────┬───────────────┘
                │
                ▼
┌───────────────────────────────┐
│ SoftIRQ Context               │
│ Atomic / Cannot Sleep         │
└───────────────┬───────────────┘
                │
                ▼
┌───────────────────────────────┐
│ Hard IRQ Context              │
│ Atomic / Interrupt Context    │
└───────────────────────────────┘
```

Nhưng phải cẩn thận:

> Không trình bày đây như một strict execution hierarchy nếu research cho thấy semantics phức tạp hơn.

Mục đích là minh họa relationship.

---

# 22. IRQ / SOFTIRQ / KSOFTIRQD COMPARISON

Tạo bảng:

| Mechanism | Context | Sleep | Typical Entry | CPU Affinity | Scheduling |
| --------- | ------- | ----- | ------------- | ------------ | ---------- |
| Hard IRQ  |         |       |               |              |            |
| SoftIRQ   |         |       |               |              |            |
| ksoftirqd |         |       |               |              |            |
| Workqueue |         |       |               |              |            |

Điền **chỉ từ research data**.

---

# 23. SOFTIRQ VECTOR TABLE

Tạo bảng:

| Vector          | Purpose | Handler | Typical Raise Source | Notes |
| --------------- | ------- | ------- | -------------------- | ----- |
| HI_SOFTIRQ      |         |         |                      |       |
| TIMER_SOFTIRQ   |         |         |                      |       |
| NET_TX_SOFTIRQ  |         |         |                      |       |
| NET_RX_SOFTIRQ  |         |         |                      |       |
| TASKLET_SOFTIRQ |         |         |                      |       |
| RCU_SOFTIRQ     |         |         |                      |       |

Nếu version có vector khác:

> thêm đúng version đó.

---

# 24. FUNCTION INDEX

Tạo function reference table:

| Function                 | File | Role | Caller | Context | Important Detail |
| ------------------------ | ---- | ---- | ------ | ------- | ---------------- |
| `raise_softirq()`        |      |      |        |         |                  |
| `raise_softirq_irqoff()` |      |      |        |         |                  |
| `open_softirq()`         |      |      |        |         |                  |
| `do_softirq()`           |      |      |        |         |                  |
| `__do_softirq()`         |      |      |        |         |                  |
| `irq_exit()`             |      |      |        |         |                  |
| `invoke_softirq()`       |      |      |        |         |                  |
| `wakeup_softirqd()`      |      |      |        |         |                  |
| `run_ksoftirqd()`        |      |      |        |         |                  |
| `local_bh_disable()`     |      |      |        |         |                  |
| `local_bh_enable()`      |      |      |        |         |                  |

Không invent information.

---

# 25. SOURCE TREE MAP

Tạo phần:

```text
Linux Kernel Source Map
```

Ví dụ:

```text
kernel/
├── softirq.c
├── irq/
├── sched/
├── time/
└── rcu/

include/linux/
├── interrupt.h
├── irq.h
└── percpu.h

net/core/
└── dev.c

arch/arm64/
...
```

Mỗi file có:

* purpose
* important functions
* relationship với SoftIRQ

---

# 26. PER-CPU VISUALIZATION

Tạo visualization:

```text
CPU0
├── softirq pending
├── softirq counter
├── ksoftirqd/0
└── local execution state

CPU1
├── softirq pending
├── softirq counter
├── ksoftirqd/1
└── local execution state
```

Giải thích:

* cái gì là per-CPU
* cái gì global
* tại sao

---

# 27. SMP DIAGRAM

Hiển thị:

```text
CPU0
  │
  │ raise remote work
  ▼
CPU1
  │
  ▼
pending bit
  │
  ▼
SoftIRQ execution
```

Nếu có IPI hoặc cơ chế architecture-specific:

> đưa vào diagram đúng theo research.

---

# 28. `__do_softirq()` DEEP DIVE

Đây là một trong những section quan trọng nhất.

Tạo riêng:

```text
__do_softirq() Deep Dive
```

với:

```text
Entry
 ↓
Save state
 ↓
Disable local IRQ
 ↓
Read pending
 ↓
Clear pending
 ↓
Dispatch vector
 ↓
Run handler
 ↓
Check new pending
 ↓
Check restart/time limit
 ↓
Continue OR defer
```

Mỗi bước phải link tới source analysis.

---

# 29. KSOFTIRQD DEEP DIVE

Tạo flow:

```text
SoftIRQ pending
      ↓
cannot continue in current context
      ↓
wakeup_softirqd()
      ↓
ksoftirqd/N becomes runnable
      ↓
scheduler
      ↓
CPU executes ksoftirqd/N
      ↓
run_ksoftirqd()
      ↓
do_softirq()
```

Phân biệt rõ:

```text
direct SoftIRQ execution
vs
ksoftirqd-mediated execution
```

---

# 30. NETWORKING SECTION

Tạo section riêng:

```text
NET_RX_SOFTIRQ
```

Master flow:

```text
NIC
 ↓
Interrupt
 ↓
Driver ISR
 ↓
NAPI schedule
 ↓
NET_RX_SOFTIRQ
 ↓
net_rx_action()
 ↓
NAPI poll
 ↓
Packet processing
```

Nếu research có details như:

* budget
* GRO
* backlog
* skb

phải giữ lại.

---

# 31. TASKLET SECTION

Flow:

```text
tasklet_schedule()
       ↓
TASKLET_SOFTIRQ
       ↓
tasklet_action()
       ↓
callback
```

Thêm:

```text
Tasklet vs SoftIRQ
```

và bảng comparison.

---

# 32. TIMER SECTION

Flow:

```text
Timer Interrupt
       ↓
TIMER_SOFTIRQ
       ↓
Timer processing
       ↓
Callback
```

Tổ chức:

* timer source
* raise path
* handler
* callback
* CPU
* latency

---

# 33. RCU SECTION

Đặc biệt quan trọng.

Tạo:

```text
RCU_SOFTIRQ Deep Dive
```

Flow:

```text
call_rcu()
       ↓
Grace Period
       ↓
Callback becomes ready
       ↓
RCU SoftIRQ
       ↓
rcu_core()
       ↓
Callback execution
```

Nếu research phân biệt:

```text
Grace Period progress
vs
Callback invocation
```

phải giữ nguyên distinction đó.

---

# 34. RCU STALL CONNECTION

Tạo riêng:

```text
SoftIRQ ↔ RCU Stall
```

Giải thích dựa trên research:

```text
SoftIRQ starvation
      ↓
RCU processing delayed
      ↓
callbacks / progress delayed
      ↓
potential RCU-related symptoms
```

Không được tự suy diễn causal chain nếu research chưa chứng minh.

Mọi relationship phải được trình bày rõ:

```text
Confirmed
Possible
Version-specific
```

nếu cần.

---

# 35. SCHEDULER SECTION

Tạo:

```text
SoftIRQ ↔ Scheduler
```

Flow:

```text
SoftIRQ
   ↓
time/restart limit
   ↓
ksoftirqd
   ↓
task becomes runnable
   ↓
scheduler
   ↓
CPU executes ksoftirqd
```

Giải thích:

* `task_struct`
* runnable state
* CPU affinity
* scheduling
* latency

---

# 36. LOCKING SECTION

Tạo bảng:

| Primitive             | Purpose | Context | Relation to SoftIRQ |
| --------------------- | ------- | ------- | ------------------- |
| `local_irq_disable()` |         |         |                     |
| `local_bh_disable()`  |         |         |                     |
| `preempt_disable()`   |         |         |                     |
| spinlock              |         |         |                     |
| mutex                 |         |         |                     |

Phải đặc biệt nhấn mạnh:

```text
local_irq_disable()
vs
local_bh_disable()
vs
preempt_disable()
```

---

# 37. PREEMPT_COUNT VISUALIZATION

Nếu research có phân tích:

```text
preempt_count
```

tạo visual:

```text
preempt_count
┌───────────────┬───────────────┬───────────────┐
│ PREEMPT bits  │ SOFTIRQ bits  │ HARDIRQ bits  │
└───────────────┴───────────────┴───────────────┘
```

Chỉ hiển thị bit layout đúng theo source/version.

---

# 38. LATENCY / STARVATION

Tạo:

```text
SoftIRQ Starvation
```

với scenario:

```text
CPU
│
├── SoftIRQ workload
├── SoftIRQ workload
├── SoftIRQ workload
├── ksoftirqd
└── user/kernel tasks
```

Phân tích:

* starvation
* latency
* CPU saturation
* scheduling effect
* symptoms
* debugging

---

# 39. REAL SCENARIOS

Tạo section:

```text
Real-world Scenarios
```

ít nhất:

### Scenario 1

Network flood.

### Scenario 2

RCU callback flood.

### Scenario 3

Timer flood.

### Scenario 4

Tasklet flood.

### Scenario 5

SoftIRQ CPU hog.

Mỗi scenario:

```text
Symptom
↓
Expected flow
↓
What happens
↓
What to inspect
↓
How to verify
```

---

# 40. DEBUGGING PLAYBOOK

Tạo section cực rõ:

```text
SoftIRQ Debugging Playbook
```

Flow:

```text
Symptom
    ↓
Check /proc/softirqs
    ↓
Check /proc/interrupts
    ↓
Check CPU usage
    ↓
Check ksoftirqd
    ↓
Trace softirq entry/exit
    ↓
Trace specific subsystem
    ↓
Inspect source
    ↓
Root cause
```

---

# 41. `/proc/softirqs`

Giữ output example nếu research có.

Format:

```text
Command
↓
Output
↓
Interpretation
```

Ví dụ:

```bash
cat /proc/softirqs
```

Sau đó:

* giải thích từng counter
* per-CPU nature
* useful signals
* limitations

---

# 42. TRACEPOINTS

Tạo section:

```text
Tracing SoftIRQ
```

Bao gồm nếu research có:

```text
irq:softirq_entry
irq:softirq_exit
```

và hướng dẫn:

```bash
trace-cmd
perf
ftrace
```

Không tự tạo command chưa được research xác nhận.

---

# 43. FTRACE

Tạo flow:

```text
irq_exit
   ↓
invoke_softirq
   ↓
do_softirq
   ↓
__do_softirq
```

Hiển thị:

```text
Command
Expected Output
Interpretation
```

---

# 44. PERF

Tạo section:

```text
perf Analysis
```

Giữ lại command:

```bash
perf top
perf record
perf report
```

nếu research đã có.

Giải thích cách dùng để tìm:

* `__do_softirq`
* `net_rx_action`
* `run_ksoftirqd`
* `rcu_core`
* timer handlers

---

# 45. ARCHITECTURE

Tạo subsection:

```text
Architecture-specific Behavior
```

Ví dụ:

```text
ARM64
PowerPC
Other architectures
```

Phải phân biệt:

```text
Generic kernel
vs
Architecture-specific entry/exit path
```

---

# 46. DRIVER SECTION

Tạo:

```text
SoftIRQ in Drivers
```

Phân tích:

```text
ISR
 ↓
Deferred processing
 ↓
SoftIRQ / Tasklet / Workqueue
```

Ví dụ:

* UART
* network
* storage
* USB

---

# 47. USB SECTION

Nếu research có nội dung:

```text
USB interrupt
URB completion
callback context
softirq
tasklet
workqueue
```

hãy tạo flow riêng.

Đặc biệt:

> Không được khẳng định mọi USB completion đều SoftIRQ nếu source không chứng minh điều đó.

---

# 48. PERFORMANCE SECTION

Tạo:

```text
SoftIRQ Performance
```

Phân tích:

* interrupt overhead
* softirq overhead
* CPU time
* cache effects
* scheduler effects
* ksoftirqd wakeup
* context switching
* packet processing
* budget

Tách:

```text
Measured
vs
Conceptual
```

nếu research có benchmark.

---

# 49. TUNING

Tạo table:

| Parameter | Meaning | Subsystem | Version | Effect |
| --------- | ------- | --------- | ------- | ------ |

Ví dụ:

```text
netdev_budget
netdev_budget_usecs
```

Chỉ đưa parameter thực sự xuất hiện trong research.

---

# 50. COMMON BUGS

Tạo bảng:

| Bug | Root Cause | Symptom | Detection | Fix |
| --- | ---------- | ------- | --------- | --- |

Bao gồm nếu research có:

* sleeping in atomic context
* wrong locking
* deadlock
* race condition
* callback lifetime
* starvation
* incorrect CPU assumption
* misuse of `local_bh_disable()`

---

# 51. INTERVIEW SECTION

Tạo:

```text
SoftIRQ Interview
```

chia:

```text
Junior
Intermediate
Senior
Expert
```

Mỗi câu:

```text
Question
↓
Short Answer
↓
Deep Explanation
↓
Source
↓
Common Trap
```

Không rút gọn câu trả lời gốc.

---

# 52. EXERCISES

Tạo:

```text
Source Code Exercises
```

Ví dụ:

```text
Exercise 1
Find raise_softirq()

Exercise 2
Find pending state

Exercise 3
Trace irq_exit()

Exercise 4
Trace __do_softirq()

Exercise 5
Trace ksoftirqd

Exercise 6
Trace NET_RX_SOFTIRQ

Exercise 7
Trace TASKLET_SOFTIRQ

Exercise 8
Trace RCU_SOFTIRQ

Exercise 9
Trace remote CPU SoftIRQ

Exercise 10
Trace a real packet path
```

Nếu source code line/function đã có trong research:

> link trực tiếp.

---

# 53. CROSS REFERENCES

Mỗi khi một concept liên quan đến concept khác, tạo internal link.

Ví dụ:

```text
NET_RX_SOFTIRQ
   ↓
NAPI
```

```text
RCU_SOFTIRQ
   ↓
RCU
```

```text
ksoftirqd
   ↓
Scheduler
```

```text
SoftIRQ
   ↓
RCU Stall
```

Dùng:

```html
<a href="#napi">NAPI</a>
```

Không để broken anchor.

---

# 54. MASTER CONCEPT MAP

Đầu hoặc cuối document phải có:

```text
                    SOFTIRQ
                       │
       ┌───────────────┼────────────────┐
       │               │                │
     RAISE          PROCESS          DEFER
       │               │                │
       ▼               ▼                ▼
 pending          __do_softirq       ksoftirqd
       │               │                │
       ▼               ▼                ▼
   per-CPU          vectors          scheduler
                       │
        ┌──────────────┼──────────────┐
        ▼              ▼              ▼
      TIMER          NET_RX          RCU
        │              │              │
        ▼              ▼              ▼
      timer           NAPI         rcu_core
```

---

# 55. MASTER SOURCE GRAPH

Tạo:

```text
kernel/softirq.c
      │
      ├── raise_softirq
      ├── open_softirq
      ├── do_softirq
      └── __do_softirq
              │
              ├── TIMER
              ├── NET_RX
              ├── TASKLET
              └── RCU
```

Thêm:

```text
kernel/irq/
kernel/sched/
net/core/dev.c
kernel/rcu/
kernel/time/
arch/<arch>/
```

theo research data.

---

# 56. CHEAT SHEET

Cuối file:

```text
SoftIRQ Cheat Sheet
```

Bao gồm:

### Core Functions

```text
raise_softirq
raise_softirq_irqoff
open_softirq
do_softirq
__do_softirq
invoke_softirq
irq_exit
wakeup_softirqd
run_ksoftirqd
```

### Important vectors

```text
HI_SOFTIRQ
TIMER_SOFTIRQ
NET_TX_SOFTIRQ
NET_RX_SOFTIRQ
TASKLET_SOFTIRQ
RCU_SOFTIRQ
```

### Context rules

```text
Hard IRQ
SoftIRQ
ksoftirqd
Process context
```

---

# 57. QUICK REFERENCE TABLE

Tạo một bảng lớn:

| Concept          | Meaning | Source | Important Function | Context |
| ---------------- | ------- | ------ | ------------------ | ------- |
| SoftIRQ          |         |        |                    |         |
| Pending          |         |        |                    |         |
| ksoftirqd        |         |        |                    |         |
| Tasklet          |         |        |                    |         |
| NAPI             |         |        |                    |         |
| RCU_SOFTIRQ      |         |        |                    |         |
| local_bh_disable |         |        |                    |         |

---

# 58. PROGRESS TRACKING

Mỗi chapter có:

```text
☐ Not Started
☐ Learning
☑ Completed
```

Implement bằng checkbox.

Lưu bằng:

```javascript
localStorage
```

Có progress bar:

```text
SoftIRQ Study Progress: 38%
```

Tính dựa trên chapter đã hoàn thành.

---

# 59. DARK / LIGHT MODE

Có toggle:

```text
☾ / ☀
```

Lưu preference bằng:

```javascript
localStorage
```

Dark mode phải phù hợp đọc code lâu.

---

# 60. BACK TO TOP

Hiển thị khi scroll xuống:

```text
↑
```

---

# 61. RESPONSIVE

Desktop:

```text
Sidebar + Main Content
```

Tablet:

```text
smaller sidebar
```

Mobile:

```text
☰ Contents
```

Không để code bị cắt.

---

# 62. PRINT / PDF

Phải có:

```css
@media print
```

Khi print:

* hide sidebar
* hide buttons
* hide search
* giữ heading
* giữ code
* giữ diagram
* giữ table
* tránh break section nếu có thể

---

# 63. DIAGRAM STYLE

Diagram phải:

* đơn giản
* ít màu
* border rõ
* arrows rõ
* monospace nếu cần
* không 3D
* không quá decorative

Ưu tiên:

```text
SVG
CSS
HTML
```

---

# 64. SEMANTIC HTML

Dùng:

```html
<header>
<nav>
<main>
<section>
<article>
<aside>
<footer>
```

Heading hierarchy hợp lệ:

```text
h1
 ├── h2
 │    ├── h3
 │    └── h3
 └── h2
```

Không nhảy lung tung giữa heading levels.

---

# 65. ACCESSIBILITY

Có:

* sufficient contrast
* readable font size
* keyboard navigation
* focus state
* semantic elements
* alt text nếu có images
* buttons có aria-label nếu chỉ có icon

---

# 66. CONTENT TAGS

Có thể gắn tag cho section:

```text
[SOURCE]
[CONCEPT]
[CALL FLOW]
[CONCURRENCY]
[SMP]
[PERFORMANCE]
[DEBUG]
[NETWORK]
[RCU]
[ARCH]
```

Click tag có thể filter nếu implementation đơn giản.

Không cần làm phức tạp nếu ảnh hưởng readability.

---

# 67. STANDARD / IMPLEMENTATION SEPARATION

Tạo riêng các badge:

```text
GENERIC KERNEL
ARCH SPECIFIC
VERSION SPECIFIC
CONFIG SPECIFIC
PREEMPT_RT
DRIVER SPECIFIC
```

Mục tiêu:

> Người đọc biết statement nào áp dụng ở đâu.

---

# 68. SOURCE-FIRST PRESENTATION

Với kiến thức source-code-based, ưu tiên thứ tự:

```text
Concept
 ↓
Function
 ↓
Source Location
 ↓
Call Flow
 ↓
Context
 ↓
Behavior
 ↓
Why
```

Không chỉ nói:

> "Linux làm như vậy."

Phải nói:

> Function nào làm?

> File nào?

> Được gọi từ đâu?

> Trong context nào?

---

# 69. NO INVENTED INFORMATION

Nếu research data không có thông tin:

> Không được tự bịa.

Ví dụ không có line number:

```text
Line: Not provided in source data
```

Không được tự nghĩ ra.

Nếu có uncertainty:

```text
UNCERTAIN
```

Nếu implementation-dependent:

```text
IMPLEMENTATION-SPECIFIC
```

---

# 70. NO EXTERNAL KNOWLEDGE OVERRIDE

Nếu kiến thức tôi cung cấp khác với kiến thức bạn biết:

> ưu tiên dữ liệu research của tôi về implementation cụ thể.

Chỉ bổ sung external knowledge khi cần để nối logic.

Nếu bổ sung:

```text
ADDITIONAL CONTEXT
```

và phải phân biệt với research gốc.

---

# 71. FINAL VALIDATION

Trước khi xuất file, tự kiểm tra:

```text
[ ] UTF-8
[ ] HTML valid
[ ] No broken links
[ ] No duplicate IDs
[ ] Search works
[ ] Sidebar works
[ ] Scroll highlighting works
[ ] Copy buttons work
[ ] Dark mode works
[ ] Light mode works
[ ] localStorage works
[ ] Progress bar works
[ ] Responsive works
[ ] Print works
[ ] All source sections preserved
[ ] All code blocks preserved
[ ] All diagrams preserved
[ ] All interview questions preserved
[ ] No invented line numbers
[ ] No missing version information
[ ] No missing architecture distinction
```

---

# 72. CONTENT COMPLETENESS CHECK

Trước khi hoàn tất, đối chiếu với research input.

Ít nhất phải kiểm tra:

```text
[ ] SoftIRQ fundamentals
[ ] Interrupt context
[ ] Atomic context
[ ] SoftIRQ vectors
[ ] raise_softirq
[ ] pending bitmap
[ ] open_softirq
[ ] do_softirq
[ ] __do_softirq
[ ] restart/time limits
[ ] ksoftirqd
[ ] irq_exit
[ ] interrupt return path
[ ] scheduler
[ ] Tasklet
[ ] Timer
[ ] NET_RX
[ ] NAPI
[ ] RCU_SOFTIRQ
[ ] SMP
[ ] per-CPU
[ ] remote raise
[ ] locking
[ ] local_bh_disable
[ ] preemption
[ ] PREEMPT_RT
[ ] starvation
[ ] latency
[ ] source map
[ ] function index
[ ] ARM64
[ ] PowerPC/other arch
[ ] driver
[ ] USB
[ ] network driver
[ ] debugging
[ ] /proc/softirqs
[ ] tracepoints
[ ] ftrace
[ ] perf
[ ] real scenarios
[ ] bugs
[ ] performance
[ ] tuning
[ ] interview
[ ] exercises
[ ] cheat sheet
```

Nếu research input có thêm topic:

> phải giữ lại.

---

# 73. FINAL MASTER FLOW

Cuối tài liệu phải có một diagram lớn thể hiện toàn bộ:

```text
                    HARDWARE
                        │
                        ▼
                   IRQ ENTRY
                        │
                        ▼
                HARD IRQ HANDLER
                        │
                        ▼
                 RAISE SOFTIRQ
                        │
                        ▼
                PER-CPU PENDING
                        │
                        ▼
                    IRQ EXIT
                        │
             ┌──────────┴──────────┐
             │                     │
             ▼                     ▼
       DIRECT EXECUTION         DEFER
             │                     │
             ▼                     ▼
        do_softirq()          wakeup ksoftirqd
             │                     │
             ▼                     ▼
       __do_softirq()         scheduler
             │                     │
             └──────────┬──────────┘
                        ▼
                  SOFTIRQ VECTOR
                        │
       ┌────────────────┼────────────────┐
       │                │                │
       ▼                ▼                ▼
    TIMER            NET_RX             RCU
       │                │                │
       ▼                ▼                ▼
    Timer             NAPI           rcu_core
                        │
                        ▼
                   Network Stack
```

---

# 74. FINAL DELIVERABLE

Output cuối cùng:

```text
linux-kernel-softirq-deep-dive.html
```

Không tạo nhiều HTML nhỏ.

Không tạo summary thay thế research.

Không tạo landing page.

Không làm UI lấn át technical content.

Ưu tiên tuyệt đối:

```text
1. Technical correctness
2. Content completeness
3. Source-code traceability
4. Clear hierarchy
5. Call-flow visualization
6. Search / navigation
7. Readability
8. Visual polish
```

---

# 75. TRIẾT LÝ CỦA FILE

Hãy coi file này là:

> **Linux Kernel SoftIRQ Personal Reference Book**

Tôi phải có thể mở file sau vài tháng và dùng nó để trả lời:

```text
SoftIRQ là gì?
        ↓
Ai raise?
        ↓
Pending nằm đâu?
        ↓
Ai execute?
        ↓
CPU nào?
        ↓
Context nào?
        ↓
Function nào?
        ↓
Khi nào ksoftirqd chạy?
        ↓
Scheduler tham gia ở đâu?
        ↓
NET_RX/TIMER/RCU chạy thế nào?
        ↓
Debug ở đâu?
        ↓
Source code chứng minh ở đâu?
```

Mục tiêu cuối cùng:

> Tôi phải nhìn một SoftIRQ-related problem trong Linux Kernel và có thể dùng HTML này như một **source-oriented troubleshooting and learning reference**.
