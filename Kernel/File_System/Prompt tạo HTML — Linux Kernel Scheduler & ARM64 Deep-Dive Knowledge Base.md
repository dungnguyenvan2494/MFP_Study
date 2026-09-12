# ROLE

Bạn là một **Senior Frontend Engineer + Technical Documentation Designer + Linux Kernel Engineer**.

Hãy chuyển **toàn bộ nội dung nghiên cứu Linux Kernel Scheduler / CFS / ARM64 / Timer / IRQ / Preemption / Context Switch / Load Balancing ở phía trên** thành một **interactive HTML technical knowledge base** cực kỳ đẹp, dễ đọc, dễ tra cứu và phù hợp để học source code Linux Kernel chuyên sâu.

Đây không phải một landing page marketing.

Đây là một **Linux Kernel Architecture Explorer**.

Mục tiêu:

> Người đọc có thể dùng HTML này như một "bản đồ source code Linux Kernel", từ Hardware → ARM64 → IRQ → Scheduler → CFS → Context Switch → Task B.

---

# 1. NGUYÊN TẮC QUAN TRỌNG NHẤT

## KHÔNG ĐƯỢC LÀM MẤT NỘI DUNG

Phải giữ lại **toàn bộ ý, function, flow, data structure, source reference, giải thích và relationship** có trong nội dung nghiên cứu.

Không được:

- rút gọn quá mức
- biến thành summary vài trang
- bỏ các function nhỏ
- bỏ các source reference
- bỏ các bước CPU/EL0/EL1
- bỏ các register
- bỏ các relationship giữa generic kernel và ARM64
- bỏ load balancing
- bỏ local/remote reschedule
- bỏ safe point
- bỏ context switch details

Có thể cải thiện wording và layout nhưng:

> **Content completeness > visual simplicity**

---

# 2. OUTPUT

Tạo:

```text
index.html
```

Chỉ cần một file HTML duy nhất nếu có thể.

Ưu tiên:

```text
HTML
+ CSS
+ Vanilla JavaScript
```

nhúng trực tiếp trong file.

Không yêu cầu build system.

Không dùng framework nếu không thực sự cần.

Nếu dùng thư viện ngoài thì ưu tiên CDN nhẹ và phổ biến.

---

# 3. DESIGN STYLE

Thiết kế theo phong cách:

```text
Linux Kernel Source Code Explorer
+
Developer Documentation
+
Interactive Architecture Map
```

Cảm giác giống sự kết hợp của:

- Linux kernel documentation
- VS Code
- GitHub code viewer
- technical architecture diagram
- modern developer portal

Không được quá màu mè.

Ưu tiên:

- sạch
- technical
- chuyên nghiệp
- dễ đọc nhiều giờ
- hierarchy rõ ràng
- typography tốt
- code dễ đọc
- diagram nổi bật
- navigation cực nhanh

---

# 4. COLOR SYSTEM

Sử dụng hệ màu nhất quán cho các layer:

```text
GENERIC KERNEL
→ xanh dương

SCHEDULER / CFS
→ xanh lá

ARM64
→ cam

HARDWARE
→ đỏ / hồng

MMU / MEMORY
→ tím

IRQ / TIMER
→ vàng

IMPORTANT / WARNING
→ đỏ

DATA STRUCTURE
→ cyan

ASSEMBLY
→ dark purple
```

Không dùng quá nhiều gradient.

Bảo đảm contrast tốt.

---

# 5. LAYOUT CHÍNH

Thiết kế dạng 3 vùng.

```text
┌───────────────────────────────────────────────────────────────┐
│ HEADER                                                        │
│ Linux Kernel Scheduler & ARM64 Deep-Dive                     │
├───────────────┬───────────────────────────────────┬───────────┤
│ LEFT SIDEBAR  │ MAIN CONTENT                      │ RIGHT     │
│               │                                   │ PANEL     │
│ Chapters      │ Architecture                     │ Search    │
│ Functions     │ Call Flow                        │ Details   │
│ Structures    │ Source Code                      │ Key terms │
│ Hardware      │ Diagrams                         │           │
│ Registers     │ Tables                           │           │
└───────────────┴───────────────────────────────────┴───────────┘
```

Desktop:

```text
Left sidebar: ~260px
Main: flexible
Right panel: ~320px
```

Mobile:

- sidebar collapsible
- right panel chuyển thành drawer
- content 1 column

---

# 6. HEADER

Header phải có:

```text
Linux Kernel Scheduler
ARM64 Deep-Dive
```

Subtitle:

```text
From Timer Interrupt → Scheduler → CFS → Context Switch → Task B
```

Thêm:

```text
[GENERIC]
[ARM64]
[CFS]
[IRQ]
[MMU]
[ASSEMBLY]
[HARDWARE]
```

Hiển thị như badges.

Có nút:

```text
⌕ Search
☰ Navigation
☀ / 🌙 Theme
```

---

# 7. LEFT SIDEBAR — TABLE OF CONTENTS

Tạo navigation dạng tree.

Ví dụ:

```text
01. Architecture Overview
02. Core Data Structures
03. current / task_struct
04. CFS Runqueue
05. pick_next_task()
06. TIF_NEED_RESCHED
07. Safe Points
08. Timer → IRQ
09. scheduler_tick()
10. __schedule()
11. context_switch()
12. switch_mm()
13. ARM64 switch_to()
14. cpu_switch_to()
15. Task A → Task B
16. Load Balancing
17. Local vs Remote CPU
18. State Machines
19. Full Architecture
20. Source Map
21. Common Misconceptions
22. Final Mental Model
```

Sidebar phải:

- sticky
- active section highlight
- click → smooth scroll
- tự động highlight section hiện tại khi scroll

---

# 8. SEARCH / KEYWORD LOOKUP

Đây là chức năng cực kỳ quan trọng.

Tạo một **global search**.

Người dùng gõ:

```text
schedule
```

thì phải tìm ra:

```text
schedule()
__schedule()
context_switch()
preempt_schedule()
TIF_NEED_RESCHED
```

Gõ:

```text
vruntime
```

thì hiện:

```text
vruntime
min_vruntime
update_curr()
calc_delta_fair()
pick_next_entity()
```

Gõ:

```text
cpu_switch_to
```

thì hiện:

```text
cpu_switch_to()
entry.S
x19-x28
sp
sp_el0
lr
```

Gõ:

```text
TTBR0
```

thì hiện:

```text
switch_mm_irqs_off()
mm_struct
pgd
ASID
TTBR0_EL1
```

Search phải hỗ trợ:

- function
- struct
- macro
- register
- source file
- concept
- chapter
- keyword

---

# 9. SEARCH RESULT UI

Kết quả search hiển thị:

```text
┌─────────────────────────────────────┐
│ cpu_switch_to                       │
│                                     │
│ TYPE       Function                 │
│ LAYER      ARM64 / Assembly         │
│ FILE       arch/arm64/kernel/...    │
│                                     │
│ Related:                            │
│ switch_to                           │
│ __switch_to                         │
│ context_switch                      │
└─────────────────────────────────────┘
```

Click result:

→ tự scroll đến section.

Highlight keyword vừa tìm.

---

# 10. FUNCTION INDEX

Tạo một trang / panel riêng:

```text
FUNCTION INDEX
```

Có filter:

```text
[All]
[Scheduler]
[CFS]
[IRQ]
[Timer]
[ARM64]
[MM]
[Load Balancing]
[Assembly]
```

Ví dụ:

```text
scheduler_tick()
check_preempt_tick()
resched_curr()
set_tsk_need_resched()
schedule()
__schedule()
pick_next_task()
pick_next_task_fair()
pick_next_entity()
__pick_first_entity()
context_switch()
switch_mm_irqs_off()
switch_to()
__switch_to()
cpu_switch_to()
load_balance()
find_busiest_group()
detach_tasks()
attach_tasks()
```

Click function → mở function detail.

---

# 11. FUNCTION DETAIL CARD

Mỗi function quan trọng phải có component thống nhất:

```text
┌─────────────────────────────────────────────┐
│ scheduler_tick()                            │
├─────────────────────────────────────────────┤
│ Layer: GENERIC                              │
│ File: kernel/sched/core.c                   │
│                                             │
│ PURPOSE                                     │
│ ...                                         │
│                                             │
│ CALLER                                      │
│ ...                                         │
│                                             │
│ CALLEE                                      │
│ task_tick_fair()                            │
│                                             │
│ DATA STRUCTURES                             │
│ rq                                          │
│ task_struct                                 │
│ sched_entity                                │
│                                             │
│ CPU STATE                                    │
│ EL1 / IRQ context                            │
│                                             │
│ RELATED                                      │
│ check_preempt_tick()                        │
│ resched_curr()                              │
└─────────────────────────────────────────────┘
```

Có button:

```text
[Show Source]
[Show Callers]
[Show Callees]
[Show Related Structures]
[Trace Flow]
```

---

# 12. SOURCE CODE VIEWER

Tạo code blocks đẹp như VS Code.

Ví dụ:

```c
scheduler_tick();
```

Code viewer phải hỗ trợ:

- line numbers
- syntax highlighting
- copy button
- source file badge
- highlight dòng quan trọng
- collapse / expand

Ví dụ header:

```text
kernel/sched/core.c
Line: XXXX
Language: C
Layer: GENERIC
```

---

# 13. SOURCE REFERENCE

Mỗi source reference phải được hiển thị rõ:

```text
📄 kernel/sched/core.c
📍 scheduler_tick()
```

hoặc:

```text
📄 arch/arm64/kernel/entry.S
📍 cpu_switch_to()
```

Khi click:

- scroll tới source block tương ứng
- hoặc mở code detail panel

---

# 14. HIGH-LEVEL ARCHITECTURE

Đặt đầu tài liệu một diagram lớn:

```text
HARDWARE
   │
   ▼
ARM Generic Timer
   │
   ▼
GIC
   │
   ▼
ARM64 Exception Entry
   │
   ▼
GENERIC IRQ
   │
   ▼
Timer / HRTimer
   │
   ▼
scheduler_tick()
   │
   ▼
CFS
   │
   ▼
pick_next_task()
   │
   ▼
context_switch()
   │
   ├── switch_mm()
   │
   └── cpu_switch_to()
           │
           ▼
       TASK B
```

Diagram phải interactive.

Hover node:

→ tooltip.

Click node:

→ scroll tới section tương ứng.

---

# 15. 3-LAYER CODE ARCHITECTURE

Tạo section cực kỳ nổi bật:

```text
GENERIC KERNEL
       ↓
ARCH-INDEPENDENT SCHEDULER
       ↓
ARCH/ARM64
       ↓
CPU / REGISTER / MMU
```

Mỗi layer có màu riêng.

Ví dụ:

```text
GENERIC
kernel/sched/*.c

SCHEDULER
CFS / sched_class / rq / vruntime

ARM64
arch/arm64/kernel/*.c
entry.S

HARDWARE
Cortex-A72
GIC
CNTV
MMU
```

Có thể click từng layer để filter nội dung.

---

# 16. CORE DATA STRUCTURE VIEW

Phải tạo interactive object graph:

```text
CPU
 ↓
current
 ↓
task_struct
 ├── se
 │    ↓
 │  cfs_rq
 │    ↓
 │   rq
 │
 ├── mm
 │    ↓
 │  pgd
 │    ↓
 │ TTBR0_EL1
 │
 └── thread
      ↓
   cpu_context
```

Click struct:

→ mở detail panel.

---

# 17. CFS TREE VISUALIZATION

Tạo diagram:

```text
          C vr=1050
         /          \
 A vr=1000          E vr=1200
                    /
                D vr=1100
```

Highlight:

```text
LEFTMOST = A
```

Show:

```text
rb_leftmost
rb_first_cached()
```

Bên cạnh:

```text
curr = B
vruntime = 1030
```

và giải thích:

```text
B đang chạy
→ không nằm trong cây CFS
```

Có animation khi click:

```text
pick_next_entity()
```

---

# 18. TIMER → IRQ TIMELINE

Tạo timeline visual:

```text
CNTV
 ↓
PPI
 ↓
GIC
 ↓
VBAR_EL1 + offset
 ↓
el0_irq
 ↓
kernel_entry
 ↓
irq_handler
 ↓
gic_handle_irq
 ↓
handle_domain_irq
 ↓
irq_enter
 ↓
arch_timer_handler_virt
 ↓
hrtimer_interrupt
 ↓
tick_sched_timer
 ↓
update_process_times
 ↓
scheduler_tick
```

Mỗi node click được.

---

# 19. PREEMPTION FLOW

Tạo visual flow đặc biệt:

```text
scheduler_tick()
       ↓
check_preempt_tick()
       ↓
resched_curr()
       ↓
set_tsk_need_resched()
       ↓
TIF_NEED_RESCHED
       │
       ├──── local CPU
       │
       └──── remote CPU
                ↓
           IPI / SGI
```

Sau đó:

```text
SAFE POINT
```

với 4 nhánh:

```text
EL0 IRQ return
EL1 IRQ return
syscall return
preempt_enable()
```

---

# 20. CONTEXT SWITCH FLOW

Tạo diagram lớn:

```text
context_switch()
        │
        ├── prepare_task_switch()
        │
        ├── switch_mm_irqs_off()
        │       │
        │       └── TTBR0 / ASID
        │
        ├── switch_to()
        │       │
        │       └── __switch_to()
        │               │
        │               └── cpu_switch_to()
        │                       │
        │                       ├── x19-x28
        │                       ├── fp
        │                       ├── sp
        │                       ├── lr
        │                       └── sp_el0
        │
        ├── barrier()
        │
        └── finish_task_switch()
```

Color-code:

```text
GENERIC
ARCH-INDEPENDENT
ARM64
ASSEMBLY
```

---

# 21. ASSEMBLY EXPLORER

Tạo riêng một section:

```text
ARM64 cpu_switch_to()
```

Hiển thị:

```text
SAVE PREV
      ↓
RESTORE NEXT
      ↓
SP
      ↓
SP_EL0
      ↓
LR / PC
```

Có register panel:

```text
x19
x20
...
x28
x29 (FP)
x30 (LR)
SP
SP_EL0
```

Khi click register:

→ giải thích register.

---

# 22. TASK A → TASK B COMPLETE TRACE

Tạo timeline/animation:

```text
Task A EL0
     ↓
Timer interrupt
     ↓
EL1
     ↓
IRQ
     ↓
scheduler_tick
     ↓
TIF_NEED_RESCHED
     ↓
ret_to_user
     ↓
schedule
     ↓
pick B
     ↓
context_switch
     ↓
cpu_switch_to
     ↓
restore B
     ↓
kernel_exit
     ↓
eret
     ↓
Task B EL0
```

Hiển thị ở mỗi bước:

```text
CPU
EL
IRQ state
current
rq->curr
preempt_count
stack
important registers
```

---

# 23. LOAD BALANCING

Tạo flow:

```text
scheduler_tick()
 ↓
trigger_load_balance()
 ↓
rebalance_domains()
 ↓
load_balance()
 ├── find_busiest_group()
 ├── find_busiest_queue()
 ├── detach_tasks()
 ├── attach_tasks()
 └── active_balance
```

Tạo CPU visualization:

```text
CPU0 ████████
CPU1 ██████
CPU2 ██
CPU3
```

Sau balance:

```text
CPU0 █████
CPU1 █████
CPU2 █████
CPU3 ████
```

Highlight task migration.

---

# 24. LOCAL VS REMOTE CPU

Tạo side-by-side:

```text
LOCAL CPU
resched_curr()
 ↓
TIF_NEED_RESCHED
 ↓
safe point
 ↓
schedule()
```

và:

```text
REMOTE CPU
resched_curr()
 ↓
smp_send_reschedule()
 ↓
IPI
 ↓
GIC SGI
 ↓
remote CPU
 ↓
safe point
 ↓
schedule()
```

---

# 25. KEY CONCEPT CARDS

Tạo glossary cards cho:

```text
current
task_struct
sched_entity
cfs_rq
rq
sched_class
vruntime
min_vruntime
TIF_NEED_RESCHED
preempt_count
PPI
SGI
IPI
GIC
CNTV
VBAR_EL1
pt_regs
SP_EL0
SP_EL1
TTBR0_EL1
ASID
MMU
context_switch
switch_to
cpu_switch_to
```

Mỗi card:

```text
TERM
TYPE
LAYER
MEANING
RELATED
SOURCE
```

---

# 26. KEYWORD RELATIONSHIP MAP

Tạo một graph:

```text
scheduler_tick
      │
      ├── update_curr
      │
      ├── check_preempt_tick
      │
      └── resched_curr
                │
                └── TIF_NEED_RESCHED
```

Ví dụ:

```text
context_switch
 ├── switch_mm_irqs_off
 ├── switch_to
 ├── __switch_to
 └── cpu_switch_to
```

User có thể click node.

---

# 27. "WHY?" PANELS

Ở những điểm quan trọng phải có box:

```text
💡 WHY?
```

Ví dụ:

### Why does timer not directly call schedule()?

Giải thích:

```text
interrupt context
→ preempt_count
→ scheduler marks need_resched
→ scheduling happens at safe point
```

### Why is current not immediately B?

```text
TIF_NEED_RESCHED chỉ là request.
CPU chưa context switch.
```

### Why can curr be outside CFS tree?

```text
running entity được quản lý riêng qua cfs_rq->curr
```

---

# 28. IMPORTANT INVARIANTS

Tạo box màu nổi bật:

```text
TIF_NEED_RESCHED
≠
schedule immediately
```

```text
Timer IRQ
≠
context switch immediately
```

```text
pick_next_task()
≠
cpu register switch
```

```text
switch_mm()
≠
switch CPU context
```

```text
context_switch()
=
MM switch + task context switch
```

Các statement phải được giải thích dựa trên nội dung nghiên cứu.

---

# 29. COMMON MISCONCEPTIONS

Tạo accordion:

```text
▶ Timer interrupt trực tiếp gọi schedule()
▶ current đổi ngay khi set TIF_NEED_RESCHED
▶ Task curr luôn nằm trong rb-tree
▶ CFS là toàn bộ scheduler
▶ switch_mm() là context switch hoàn chỉnh
▶ finish_task_switch() chạy trong prev context
▶ remote reschedule không cần IPI
```

Click → hiện explanation.

---

# 30. SOURCE TREE EXPLORER

Hiển thị:

```text
linux/
├── kernel/
│   ├── sched/
│   │   ├── core.c
│   │   └── fair.c
│   │
│   └── time/
│       ├── timer.c
│       ├── hrtimer.c
│       └── tick-sched.c
│
├── drivers/
│   └── clocksource/
│       └── arm_arch_timer.c
│
└── arch/
    └── arm64/
        └── kernel/
            ├── entry.S
            ├── process.c
            └── irq.c
```

Click file:

→ hiển thị các function quan trọng trong file.

---

# 31. CROSS REFERENCE

Mỗi function phải có:

```text
Called by
Calls
Uses
Related structures
Related registers
Related concepts
```

Ví dụ:

```text
cpu_switch_to()

Called by:
__switch_to()

Calls:
none / assembly

Uses:
cpu_context
SP
SP_EL0
x19-x28
FP
LR

Related:
switch_to()
context_switch()
task_struct
```

---

# 32. INTERACTIVE FILTERS

Tạo các filter chips:

```text
[All]

[Scheduler]
[CFS]
[Timer]
[IRQ]
[Preemption]
[Context Switch]
[MMU]
[ARM64]
[Assembly]
[Hardware]
[Load Balancing]
[Data Structures]
```

Click filter:

→ chỉ highlight nội dung tương ứng.

---

# 33. BREADCRUMB

Ví dụ:

```text
Scheduler
 / CFS
 / pick_next_task
 / pick_next_entity
```

Sticky ở đầu content.

---

# 34. PROGRESS INDICATOR

Có progress bar:

```text
Reading progress
████████████░░░░ 72%
```

Dựa trên scroll.

---

# 35. BACK TO TOP

Có floating button.

---

# 36. DARK MODE

Dark mode phải cực đẹp và phù hợp code reading.

Light:

```text
white / gray / pastel
```

Dark:

```text
#0d1117
#161b22
#21262d
```

Không đổi layout khi chuyển theme.

---

# 37. TYPOGRAPHY

Ưu tiên font:

```text
Inter
JetBrains Mono
IBM Plex Mono
```

Nếu không có internet thì fallback:

```text
system-ui
monospace
```

Code dùng monospace.

Heading rõ cấp độ:

```text
H1
H2
H3
```

Không dùng chữ quá nhỏ.

---

# 38. DIAGRAM STYLE

Diagram phải đồng bộ.

Node:

```text
rounded rectangle
```

Arrow:

```text
→
↓
```

Layer color.

Có hover effect.

Có tooltip.

Nếu dùng Mermaid thì style Mermaid đồng bộ với website.

Nếu Mermaid CDN có thể gây lỗi offline, tạo fallback bằng HTML/CSS/SVG.

---

# 39. SVG ARCHITECTURE MAP

Ngoài Mermaid, nên dùng SVG cho những sơ đồ quan trọng:

```text
Timer → GIC → IRQ → Scheduler → Context Switch
```

Mục tiêu:

- responsive
- zoom
- click node
- highlight path
- tooltip

---

# 40. PERFORMANCE

HTML phải nhẹ.

Không render tất cả interactive details cùng lúc nếu không cần.

Dùng:

```text
lazy rendering
event delegation
debounced search
```

Search phải nhanh kể cả vài trăm keywords.

---

# 41. ACCESSIBILITY

Có:

```text
keyboard navigation
focus states
ARIA labels
good contrast
```

Search có:

```text
Ctrl + K
```

để focus.

ESC đóng search overlay.

---

# 42. COMMAND PALETTE

Tạo command palette:

```text
Ctrl + K
```

Có thể gõ:

```text
scheduler_tick
```

hoặc:

```text
CFS
```

hoặc:

```text
ARM64 context switch
```

Kết quả dẫn trực tiếp đến section.

---

# 43. MINI MAP

Ở desktop tạo mini-map bên phải:

```text
Architecture
████
Data structures
██
CFS
████
IRQ
████
Context switch
██████
Load balancing
███
```

Click minimap → jump section.

---

# 44. "MASTER FLOW" PANEL

Đặt một panel luôn dễ truy cập:

```text
ONE SCREEN MASTER FLOW
```

Nội dung:

```text
CNTV
 ↓
PPI
 ↓
GIC
 ↓
VBAR_EL1
 ↓
el0_irq
 ↓
kernel_entry
 ↓
irq_handler
 ↓
gic_handle_irq
 ↓
handle_domain_irq
 ↓
irq_enter
 ↓
arch_timer_handler_virt
 ↓
hrtimer_interrupt
 ↓
tick_sched_timer
 ↓
update_process_times
 ↓
scheduler_tick
 ↓
update_curr
 ↓
check_preempt_tick
 ↓
resched_curr
 ↓
TIF_NEED_RESCHED
 ↓
irq_exit
 ↓
ret_to_user
 ↓
do_notify_resume
 ↓
schedule
 ↓
__schedule
 ↓
pick_next_task
 ↓
pick_next_task_fair
 ↓
pick_next_entity
 ↓
__pick_first_entity
 ↓
rb_first_cached
 ↓
context_switch
 ↓
switch_mm_irqs_off
 ↓
switch_to
 ↓
__switch_to
 ↓
cpu_switch_to
 ↓
restore context B
 ↓
kernel_exit
 ↓
eret
 ↓
EL0
 ↓
Task B
```

Phải có nút:

```text
▶ Play flow
```

Khi click:

- từng node sáng lên tuần tự
- hiển thị function detail ở side panel
- scroll theo flow

Có:

```text
Pause
Replay
Step
```

---

# 45. STEP-BY-STEP DEBUG MODE

Thêm mode:

```text
TRACE MODE
```

Người dùng có thể bấm:

```text
Previous
Next
Play
Pause
```

Mỗi step hiển thị:

```text
Current function
File
CPU
EL
Task
rq->curr
IRQ state
preempt_count
Important register
```

Ví dụ:

```text
STEP 13 / 34

Function:
check_preempt_tick()

CPU:
CPU0

EL:
EL1

current:
Task A

rq->curr:
Task A

Decision:
Need reschedule = YES
```

---

# 46. FINAL DASHBOARD

Cuối trang tạo dashboard:

```text
┌─────────────────┬─────────────────┬─────────────────┐
│ Functions       │ Data Structures │ Registers       │
│ 40+             │ 20+             │ 15+             │
├─────────────────┼─────────────────┼─────────────────┤
│ Source files    │ Concepts        │ Major flows     │
│ ...             │ ...             │ ...             │
└─────────────────┴─────────────────┴─────────────────┘
```

Các con số phải được tính từ dữ liệu thực tế trong HTML, không hard-code nếu có thể.

---

# 47. CONTENT STRUCTURE

Nội dung chính phải được tổ chức tối thiểu:

```text
01. Executive Overview
02. Architecture — 4 Layers
03. Core Data Structures
04. current / task_struct
05. CFS Runqueue
06. sched_class
07. pick_next_task
08. CFS Entity Selection
09. Timer / GIC / IRQ
10. scheduler_tick
11. TIF_NEED_RESCHED
12. Safe Points
13. __schedule
14. context_switch
15. MM Switch
16. switch_to
17. __switch_to
18. cpu_switch_to
19. Task A → Task B
20. Load Balancing
21. Local / Remote Reschedule
22. State Machines
23. Source Tree
24. Function Index
25. Keyword Index
26. Common Misconceptions
27. Invariants
28. Final Mental Model
```

---

# 48. KEYWORD INDEX

Tạo một dictionary A-Z:

```text
A
ASID
active_balance
address space

C
CFS
cfs_rq
context_switch
cpu_switch_to

G
GIC

H
hrtimer

P
preempt_count
pick_next_entity
pick_next_task

R
rq
rb_first_cached
resched_curr

S
sched_class
scheduler_tick
switch_mm
switch_to

T
TIF_NEED_RESCHED
TTBR0_EL1
task_struct
tick_sched_timer
```

Click keyword:

→ mở definition.

---

# 49. RELATED TOPICS

Ở cuối mỗi section:

```text
RELATED TOPICS
```

Ví dụ:

```text
scheduler_tick()
Related:
→ update_curr()
→ check_preempt_tick()
→ resched_curr()
→ CFS
→ TIF_NEED_RESCHED
```

Click để navigate.

---

# 50. DATA MODEL

Không viết toàn bộ content trực tiếp thành HTML rời rạc.

Hãy tổ chức data bằng JavaScript object/JSON-like structure.

Ví dụ:

```javascript
const functions = {
    scheduler_tick: {
        name: "scheduler_tick",
        layer: "scheduler",
        file: "kernel/sched/core.c",
        description: "...",
        callers: [],
        callees: [],
        structures: [],
        registers: [],
        related: []
    }
};
```

Tương tự:

```javascript
const structures = {};
const registers = {};
const keywords = {};
const sourceFiles = {};
const flows = {};
```

Sau đó render UI từ data này.

Điều này giúp:

- search dễ
- filter dễ
- cross reference dễ
- maintain dễ

---

# 51. SEARCH ALGORITHM

Search phải match:

```text
exact
prefix
substring
related keyword
aliases
```

Ví dụ:

```text
"resched"
```

có thể tìm:

```text
resched_curr
set_tsk_need_resched
TIF_NEED_RESCHED
```

Ranking:

```text
Exact function name
>
Exact struct/register
>
Prefix
>
Substring
>
Related concept
```

Highlight matched characters.

---

# 52. RESPONSIVE

Phải test ít nhất:

```text
1920x1080
1440x900
1024x768
768x1024
390x844
```

Không để:

- overflow ngang
- diagram vỡ layout
- code tràn màn hình
- sidebar che content

---

# 53. VISUAL HIERARCHY

Ưu tiên thứ tự nhìn:

```text
1. Architecture
2. Main flow
3. Function
4. Source
5. Explanation
6. Data structure
7. Details
```

Không để tất cả box có visual weight bằng nhau.

---

# 54. CODE READING EXPERIENCE

Khi người dùng đọc source:

```text
function
```

ở bên trái.

Explanation:

```text
why
what
how
```

ở bên phải.

Ví dụ:

```text
┌──────────────────────┬─────────────────────────┐
│ SOURCE               │ EXPLANATION             │
│                      │                         │
│ code line 1          │ line 1 làm gì          │
│ code line 2          │ line 2 làm gì          │
│ code line 3          │ relationship            │
└──────────────────────┴─────────────────────────┘
```

---

# 55. VISUAL REPRESENTATION OF CPU STATE

Trong các flow quan trọng luôn có small panel:

```text
CPU0

EL       : EL1
IRQ      : disabled
current  : Task A
rq->curr : Task A
preempt  : HARDIRQ
stack    : kernel stack A
```

Sau switch:

```text
CPU0

EL       : EL1
IRQ      : enabled
current  : Task B
rq->curr : Task B
stack    : kernel stack B
```

---

# 56. REGISTER PANEL

Tạo fixed component:

```text
ARM64 REGISTERS
```

Ví dụ:

```text
SP_EL0
SP_EL1
ELR_EL1
SPSR_EL1
ESR_EL1
TTBR0_EL1
TTBR1_EL1
VBAR_EL1
x19-x30
```

Click register → definition.

---

# 57. INTERACTIVE LEARNING FEATURES

Thêm:

```text
[Explain this]
```

cho mỗi function.

Thêm:

```text
[Trace caller]
[Trace callee]
[Show structure]
[Show register]
```

Không cần gọi AI thật; chỉ cần navigation nội bộ.

---

# 58. VISUAL QUALITY BAR

Trang phải nhìn như một sản phẩm tài liệu kỹ thuật hoàn chỉnh.

Không được nhìn như:

- blog đơn giản
- markdown converted HTML
- collection of text boxes
- dashboard đầy màu sắc nhưng khó đọc

Phải có:

- spacing tốt
- alignment tốt
- consistent border radius
- subtle shadows
- good typography
- technical diagrams
- visual hierarchy
- clear code presentation

---

# 59. FINAL QA

Trước khi trả HTML, tự kiểm tra:

```text
[ ] Không mất nội dung nghiên cứu
[ ] Có global search
[ ] Có keyword lookup
[ ] Có function index
[ ] Có source index
[ ] Có structure index
[ ] Có register index
[ ] Có cross reference
[ ] Có architecture diagram
[ ] Có timer → IRQ flow
[ ] Có CFS flow
[ ] Có TIF_NEED_RESCHED flow
[ ] Có context switch flow
[ ] Có Task A → Task B flow
[ ] Có load balancing
[ ] Có local/remote CPU
[ ] Có assembly viewer
[ ] Có dark mode
[ ] Có responsive
[ ] Có Ctrl+K search
[ ] Có back-to-top
[ ] Có scroll progress
[ ] Có interactive diagrams
[ ] Có source references
[ ] Có common misconceptions
[ ] Có final mental model
[ ] Không có broken links
[ ] Không có console errors
```

---

# 60. FINAL DELIVERABLE

Trả về:

```text
index.html
```

và bảo đảm khi mở file trực tiếp bằng browser:

```text
Search hoạt động
Navigation hoạt động
Filter hoạt động
Dark mode hoạt động
Diagram hoạt động
Keyword lookup hoạt động
Function lookup hoạt động
Source cross-reference hoạt động
Responsive hoạt động
```

Cuối HTML có một footer:

```text
Linux Kernel Scheduler & ARM64 Deep-Dive
Source-oriented technical knowledge base
```

# MOST IMPORTANT DESIGN PRINCIPLE

Đừng thiết kế HTML như một tài liệu đọc từ trên xuống dưới.

Hãy thiết kế nó như:

> **Một bản đồ tương tác của Linux Kernel Scheduler mà người đọc có thể tra cứu theo function, struct, register, source file hoặc execution flow.**

Người dùng có thể bắt đầu từ bất kỳ điểm nào:

```text
Timer
↓
IRQ
↓
scheduler_tick
↓
CFS
↓
pick_next_task
↓
context_switch
↓
ARM64 assembly
↓
register
```

hoặc:

```text
cpu_switch_to
↓
__switch_to
↓
switch_to
↓
context_switch
↓
__schedule
```

hoặc:

```text
TIF_NEED_RESCHED
↓
safe point
↓
schedule
```

và HTML phải tự dẫn người dùng đến các phần liên quan.

**Ưu tiên số 1: source-code understanding.**

**Ưu tiên số 2: navigation / search.**

**Ưu tiên số 3: visual quality.**

**Không được hy sinh nội dung kỹ thuật để làm giao diện đẹp.**