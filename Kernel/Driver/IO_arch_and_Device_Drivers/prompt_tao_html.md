============================================================
MASTER PROMPT — BUILD LINUX KERNEL CHAPTER 13 HTML
============================================================

CONTEXT
=======

Tôi đã hoàn thành quá trình nghiên cứu Linux Kernel Chapter 13
trong conversation hiện tại.

Trong conversation đã có rất nhiều:

- kiến thức lý thuyết
- giải thích concept
- source code analysis
- function analysis
- call chain
- data structure
- runtime flow
- kernel architecture
- debugging information
- ARM64 information
- các diagram/text diagram
- các ví dụ
- các ghi chú và kết luận

NHIỆM VỤ CỦA BẠN:

Hãy lấy TOÀN BỘ nội dung nghiên cứu hiện có trong conversation
và xây dựng thành MỘT FILE HTML DUY NHẤT, hoàn chỉnh, trực quan,
có thể dùng lâu dài như một tài liệu học Linux Kernel Source Code.

KHÔNG tạo summary ngắn.

KHÔNG cắt giảm kiến thức chỉ vì HTML dài.

Hãy coi đây là:

    PERSONAL LINUX KERNEL KNOWLEDGE BASE

Mục tiêu:

    READ
      ↓
    UNDERSTAND
      ↓
    SEE THE BIG PICTURE
      ↓
    FOLLOW FUNCTION RELATIONSHIPS
      ↓
    TRACE CALL CHAIN
      ↓
    TRACE DATA FLOW
      ↓
    TRACE RUNTIME
      ↓
    OPEN SOURCE CODE
      ↓
    DEBUG

============================================================
1. OUTPUT
============================================================

Tạo file:

Linux_Kernel_Chapter_13_Deep_Dive.html

Nếu môi trường hỗ trợ tạo file:

HÃY TẠO FILE THỰC TẾ.

Không chỉ paste HTML vào chat.

HTML phải:

- chạy trực tiếp bằng Chrome
- chạy trực tiếp bằng Edge
- chạy trực tiếp bằng Firefox
- hoạt động offline
- không cần server
- không cần build
- không cần npm
- không cần framework
- không cần CDN
- không cần internet

Tất cả:

HTML
CSS
JavaScript
SVG
diagram

phải nằm trong một file duy nhất.

============================================================
2. MỤC TIÊU THIẾT KẾ
============================================================

Đây phải là:

    INTERACTIVE LINUX KERNEL STUDY DOCUMENTATION

Không phải blog.

Không phải landing page.

Không phải slide.

Không phải summary.

Phong cách:

- technical documentation
- kernel source code documentation
- engineering notebook
- architecture reference
- visual study guide

Ưu tiên:

1. Technical accuracy
2. Completeness
3. Global understanding
4. Architecture
5. Function relationships
6. Call chains
7. Runtime understanding
8. Source-code traceability
9. Readability
10. Visual quality
11. Animation

============================================================
3. QUY TẮC QUAN TRỌNG NHẤT
============================================================

KHÔNG TRÌNH BÀY KIẾN THỨC NHƯ MỘT DANH SÁCH FUNCTION.

Ví dụ KHÔNG làm:

function A()
function B()
function C()
function D()

mà không cho người đọc thấy chúng liên quan với nhau.

Thay vào đó:

Architecture
      ↓
Subsystem
      ↓
Component
      ↓
Data Structure
      ↓
Function
      ↓
Call Chain
      ↓
Runtime
      ↓
Source Code

Mọi major concept phải được đặt vào context.

============================================================
4. MASTER PAGE — GLOBAL OVERVIEW
============================================================

Trang đầu tiên phải là:

# Linux Kernel Chapter 13 — Deep Dive

Có:

- Chapter title
- One sentence summary
- What problem does this chapter solve?
- Why does Linux need this subsystem?
- Where does this subsystem live?
- What are the major components?
- What are the important data structures?
- What are the important functions?
- What are the important runtime flows?

Sau đó hiển thị:

# MASTER ARCHITECTURE

Đây là diagram lớn nhất trong document.

Nó phải cho thấy:

Linux Kernel
    ↓
Chapter 13
    ↓
Major Subsystems
    ↓
Components
    ↓
Objects / Structures
    ↓
Functions
    ↓
Runtime Flow
    ↓
Hardware / CPU / MMU / Device / Memory / FS
tùy theo nội dung thực tế.

Mục đích:

Chỉ cần nhìn diagram này, tôi phải biết Chapter 13
nằm ở đâu trong toàn bộ Linux Kernel.

============================================================
5. HIGH-LEVEL ARCHITECTURE
============================================================

Tạo nhiều mức architecture.

### LEVEL 0 — SYSTEM VIEW

User Space
    ↓
Kernel Entry
    ↓
Linux Kernel
    ↓
Chapter 13
    ↓
Hardware

### LEVEL 1 — KERNEL VIEW

Kernel
 ├── Subsystem A
 ├── Subsystem B
 ├── Subsystem C
 └── Chapter 13

### LEVEL 2 — CHAPTER VIEW

Chapter 13
 ├── Component A
 ├── Component B
 ├── Component C
 ├── Component D

### LEVEL 3 — COMPONENT VIEW

Component A
 ├── object
 ├── data structure
 ├── function
 └── callback

### LEVEL 4 — FUNCTION VIEW

Function A
    ↓
Function B
    ↓
Function C
    ↓
Function D

### LEVEL 5 — SOURCE VIEW

file.c
    ↓
function()
    ↓
specific code path
    ↓
specific data structure

Không chỉ tạo một architecture diagram.

Hãy tạo nhiều diagram ở nhiều abstraction level.

============================================================
6. "WHERE DOES THIS FIT?"
============================================================

Mỗi major section phải có một box:

WHERE DOES THIS FIT?

Ví dụ:

Linux
  ↓
Kernel
  ↓
Memory Management
  ↓
Chapter 13
  ↓
Component X
  ↓
Function Y

Hoặc topology tương ứng với Chapter thực tế.

Mục tiêu:

Không bao giờ để người đọc mất phương hướng.

============================================================
7. BIG PICTURE OF FUNCTIONS
============================================================

Tạo riêng một section rất lớn:

# Big Picture — How The Functions Work Together

Phải thể hiện:

CALLER
   ↓
ENTRY FUNCTION
   ↓
VALIDATION
   ↓
CORE FUNCTION
   ↓
HELPER
   ↓
DATA STRUCTURE UPDATE
   ↓
SUBSYSTEM INTERACTION
   ↓
RETURN

Tạo call graph lớn.

Ví dụ:

entry()
 ├── validate()
 ├── prepare()
 │    ├── helper_a()
 │    └── helper_b()
 │
 ├── core()
 │    ├── subsystem_a()
 │    └── subsystem_b()
 │
 └── cleanup()

Tuyệt đối tránh việc người đọc phải tự ghép các function lại.

============================================================
8. FUNCTION CALL GRAPH
============================================================

Tạo:

# Function Call Graph

Visualize:

A()
 ├── B()
 │   ├── D()
 │   └── E()
 │
 ├── C()
 │   └── F()
 │
 └── G()

Nếu graph lớn:

chia thành:

- Initialization call graph
- Normal runtime call graph
- Fast path
- Slow path
- Error path
- Cleanup path
- Architecture-specific path

Mỗi graph phải có title rõ ràng.

============================================================
9. FUNCTION RELATIONSHIP MAP
============================================================

Tạo:

# Function Relationship Map

Không chỉ "calls".

Hãy phân loại relationship:

A → calls → B
A → modifies → C
A → creates → D
B → releases → D
C → protected-by → lock
E → callback-of → F
G → invoked-by → IRQ
H → deferred-to → workqueue

Dùng các loại arrow khác nhau.

============================================================
10. END-TO-END FLOW
============================================================

Với MỖI major operation:

Tạo:

# End-to-End Execution Flow

Format:

Trigger
 ↓
Entry
 ↓
Function A
 ↓
Function B
 ↓
Function C
 ↓
Data structure
 ↓
Subsystem
 ↓
Hardware / MMU / Memory / FS
 ↓
Return

Bên cạnh mỗi bước phải có:

WHY?
WHAT?
WHERE?
WHO?
WHAT CHANGES?

============================================================
11. SEQUENCE DIAGRAM
============================================================

Tạo sequence diagram cho các flow quan trọng.

Ví dụ:

User
   │
   │ operation
   ▼
Kernel Entry
   │
   ▼
Function A
   │
   ▼
Function B
   │
   ▼
Subsystem
   │
   ▼
Hardware
   │
   ▼
Subsystem
   │
   ▼
Function C
   │
   ▼
User

Nếu nhiều context:

CPU
Process
IRQ
SoftIRQ
Worker
Kernel Thread
Hardware

thì phải hiển thị riêng.

============================================================
12. DATA FLOW
============================================================

Tạo:

# Data Flow

Ví dụ:

INPUT
 ↓
VALIDATE
 ↓
TRANSFORM
 ↓
STORE
 ↓
PASS
 ↓
PROCESS
 ↓
OUTPUT

Chỉ rõ data nằm ở đâu:

- userspace
- kernel stack
- heap
- slab
- global
- shared memory
- device memory
- register
- page table
- physical memory

tùy nội dung thực tế.

============================================================
13. CONTROL FLOW
============================================================

Tạo:

# Control Flow

Hiển thị:

entry
 ↓
condition
 ├── true
 │    ↓
 │   path A
 │
 └── false
      ↓
     path B

Đặc biệt visualize:

- if/else
- switch
- loop
- retry
- timeout
- wakeup
- callback
- failure
- rollback
- cleanup

============================================================
14. OBJECT LIFECYCLE
============================================================

Với các object quan trọng:

CREATE
  ↓
INITIALIZE
  ↓
REGISTER
  ↓
ACTIVE
  ↓
MODIFY
  ↓
REFERENCE
  ↓
RELEASE
  ↓
DESTROY

Tạo lifecycle diagram.

Giải thích:

Who creates?
Who owns?
Who references?
Who modifies?
Who releases?
Who destroys?

============================================================
15. DATA STRUCTURE ARCHITECTURE
============================================================

Tạo:

# Data Structure Map

Ví dụ:

struct A
   │
   ├── B
   │
   ├── C
   │
   └── D
        ↓
      struct D

Hiển thị relationship thực tế giữa các structure.

Sau đó:

# Data Structure Usage Map

| Structure | Created By | Used By | Modified By | Protected By | Lifetime |
|-----------|------------|---------|-------------|--------------|----------|

============================================================
16. FUNCTION CARD
============================================================

Mỗi important function có card:

--------------------------------
FUNCTION
--------------------------------

Name:
File:
Line:
Kernel Version:

Role:

Purpose:

Why does this function exist?

Called By:

Calls:

Execution Context:

Input:

Output:

Side Effects:

Data Structures:

Memory Effects:

Locking:

Can Sleep:

IRQ Safety:

Reference Counting:

Error Paths:

Fast Path / Slow Path:

Architecture-specific:

ARM64 relevance:

Important invariant:

Debugging:

Related Functions:

Source Code:

--------------------------------

============================================================
17. "WHY DOES THIS FUNCTION EXIST?"
============================================================

Đây là bắt buộc.

Mỗi important function phải giải thích:

- Nó giải quyết vấn đề gì?
- Vì sao kernel cần function này?
- Nếu bỏ function này thì điều gì xảy ra?
- Tại sao không gọi function khác trực tiếp?
- Function này là abstraction layer nào?
- Nó làm cầu nối giữa những thành phần nào?

============================================================
18. "WHY DOES THIS DATA STRUCTURE EXIST?"
============================================================

Với mỗi major structure:

- Problem it solves
- Ownership
- Lifetime
- Relationships
- Who reads it?
- Who writes it?
- Why this design?

============================================================
19. EXECUTION CONTEXT MAP
============================================================

Tạo:

# Execution Context Architecture

Mapping:

Process Context
IRQ Context
SoftIRQ
Workqueue
Kernel Thread
Timer
Callback
NMI nếu có

Sau đó:

Function A → Process Context
Function B → IRQ Context
Function C → Workqueue

Cho mỗi function:

CAN SLEEP?
CAN BLOCK?
CAN TAKE MUTEX?
CAN TAKE SPINLOCK?

============================================================
20. LOCK / CONCURRENCY ARCHITECTURE
============================================================

Nếu Chapter có concurrency:

Tạo:

# Concurrency Map

CPU0
CPU1
IRQ
Worker
Process

và:

mutex
spinlock
rwlock
atomic
RCU
waitqueue

Cho mỗi lock:

- protects what?
- acquired where?
- released where?
- lock owner?
- lock ordering?
- IRQ safe?
- sleep allowed?
- possible deadlock?

============================================================
21. NORMAL PATH / ERROR PATH
============================================================

Mỗi major operation phải có hai diagram.

NORMAL:

entry
 ↓
check
 ↓
allocate
 ↓
initialize
 ↓
execute
 ↓
success

ERROR:

entry
 ↓
check
 ↓
failure
 ↓
cleanup
 ↓
rollback
 ↓
return error

Phải map resource ownership.

============================================================
22. FAST PATH / SLOW PATH
============================================================

Nếu subsystem có:

fast path

và

slow path

hãy tạo diagram riêng:

FAST PATH
entry → lookup → common operation → return

SLOW PATH
entry → validation → allocation → processing → cleanup → return

Giải thích tại sao hai path tồn tại.

============================================================
23. INITIALIZATION FLOW
============================================================

Tạo riêng:

# Initialization Architecture

Boot
 ↓
Early Init
 ↓
Subsystem Init
 ↓
Object Creation
 ↓
Registration
 ↓
Ready

Call graph:

init()
 ├── init_A()
 ├── init_B()
 └── init_C()

============================================================
24. CLEANUP / EXIT FLOW
============================================================

Tạo:

# Cleanup Architecture

Use
 ↓
Stop
 ↓
Unregister
 ↓
Release
 ↓
Destroy

Nếu có reference counting / RCU / deferred free:

phải visualize.

============================================================
25. SOURCE TREE MAP
============================================================

Tạo:

# Linux Kernel Source Tree

linux/
├── arch/
├── include/
├── kernel/
├── mm/
├── fs/
├── drivers/
└── ...

Highlight các file liên quan thực tế tới Chapter 13.

Sau đó:

file A
   ↓
function A
   ↓
file B
   ↓
function B

============================================================
26. SOURCE CODE NAVIGATION
============================================================

Tạo:

# How To Read The Source

START
 ↓
file A
 ↓
function A
 ↓
function B
 ↓
structure C
 ↓
function D
 ↓
architecture-specific implementation

Mục tiêu:

Người đọc có thể mở source code và biết nên đi đâu tiếp theo.

============================================================
27. FROM ARCHITECTURE TO CODE
============================================================

Tạo một visual bridge:

Architecture
     ↓
Subsystem
     ↓
Component
     ↓
Object
     ↓
Data Structure
     ↓
Function
     ↓
Call Chain
     ↓
Source File
     ↓
Line
     ↓
Assembly
     ↓
CPU / Hardware

Đây phải là một trong các diagram nổi bật nhất của document.

============================================================
28. RUNTIME STORY
============================================================

Tạo:

# Runtime Story

Kể lại operation dưới góc nhìn:

"What is the kernel doing?"

Ví dụ:

1. Event xảy ra.
2. CPU nhận event.
3. Kernel entry.
4. Kernel xác định object.
5. Validation.
6. Core operation.
7. Data structure update.
8. Subsystem interaction.
9. Resource handling.
10. Return.

Không dùng quá nhiều jargon mà không giải thích.

============================================================
29. MENTAL MODEL
============================================================

Mỗi major section phải có:

# Mental Model

Ví dụ:

Think of X as ...

Think of Y as ...

The important relationship is ...

The key invariant is ...

Nếu phải nhớ phần này bằng trí nhớ:

hãy nhớ...

============================================================
30. "IF YOU REMEMBER ONLY 5 THINGS"
============================================================

Mỗi major section:

1.
2.
3.
4.
5.

============================================================
31. COMMON MISCONCEPTIONS
============================================================

Tạo:

# Common Misconceptions

Những điểm mà người mới đọc Linux kernel thường hiểu sai.

Mỗi misconception:

WRONG IDEA
   ↓
CORRECT MODEL
   ↓
WHY

============================================================
32. DEBUGGING ARCHITECTURE
============================================================

Tạo:

# Debugging Map

Mapping:

Symptom
 ↓
Where to look
 ↓
Function
 ↓
Data structure
 ↓
Trace point
 ↓
Lock
 ↓
Stack
 ↓
Root cause

Dựa trên tools phù hợp với nội dung:

- printk
- dynamic_debug
- ftrace
- trace-cmd
- perf
- kgdb
- crash
- gdb
- objdump
- addr2line
- dump_stack()

Chỉ dùng tool thực sự liên quan.

============================================================
33. DEBUG LABS
============================================================

Tạo:

LAB 01 — Trace Entry
LAB 02 — Trace Call Chain
LAB 03 — Inspect Data Structure
LAB 04 — Trace Runtime
LAB 05 — Trace Error Path
LAB 06 — Inspect Locking
LAB 07 — Source Code Navigation

Mỗi lab có:

Goal
Environment
Steps
Expected Result
What To Observe
Why It Matters

============================================================
34. HIGH-LEVEL VISUALIZATION INDEX
============================================================

Tạo:

# Diagram Index

D01 — Master Architecture
D02 — Kernel Layer Architecture
D03 — Chapter Architecture
D04 — Component Interaction
D05 — Data Structure Map
D06 — Function Graph
D07 — Call Chain
D08 — Data Flow
D09 — Control Flow
D10 — Runtime Sequence
D11 — Object Lifecycle
D12 — Initialization
D13 — Cleanup
D14 — Error Path
D15 — Concurrency
D16 — Source Tree
D17 — Source Navigation
D18 — Architecture → Source Code
...

Tự động đánh index cho toàn bộ diagram.

============================================================
35. FUNCTION INDEX
============================================================

Tạo searchable table:

| Function | File | Layer | Purpose | Caller | Callee |
|----------|------|-------|---------|--------|--------|

============================================================
36. DATA STRUCTURE INDEX
============================================================

Tạo:

| Structure | Purpose | Owner | Used By | Related Functions |
|-----------|---------|-------|---------|-------------------|

============================================================
37. DEPENDENCY GRAPH
============================================================

Tạo:

# Concept Dependency Graph

Prerequisite
    ↓
Concept A
    ↓
Concept B
    ↓
Function
    ↓
Runtime

Cho tôi biết:

"What must I understand before understanding X?"

============================================================
38. KNOWLEDGE MAP
============================================================

Tạo:

# Chapter 13 Knowledge Graph

Architecture
   │
   ├── Concepts
   ├── Components
   ├── Structures
   ├── Functions
   ├── Runtime
   ├── Debugging
   └── Source Code

Các node phải link được với nhau.

============================================================
39. LEARNING PATH
============================================================

Tạo:

LEVEL 1
Understand the problem

LEVEL 2
Understand architecture

LEVEL 3
Understand data structures

LEVEL 4
Understand functions

LEVEL 5
Understand call chains

LEVEL 6
Read source code

LEVEL 7
Trace runtime

LEVEL 8
Debug

LEVEL 9
Modify kernel

============================================================
40. QUICK REVIEW
============================================================

Tạo:

5 MINUTES
   ↓
15 MINUTES
   ↓
30 MINUTES
   ↓
DEEP STUDY

5-Minute Review:

chỉ architecture + key concepts.

15-Minute:

architecture + call graph.

30-Minute:

architecture + structures + functions + runtime.

Deep Study:

full document.

============================================================
41. CHEAT SHEET
============================================================

Cuối document:

# Chapter 13 Cheat Sheet

Bao gồm:

- architecture
- key components
- key structures
- key functions
- call chain
- runtime flow
- locks
- execution contexts
- lifecycle
- error path
- important source files
- debugging entry points

============================================================
42. QUIZ
============================================================

Tạo quiz:

Concept
Architecture
Data Structure
Function
Call Chain
Runtime
Debugging

Đặc biệt có câu:

"If function X calls function Y, why?"

"What happens if X fails?"

"Who owns structure Y?"

"Which context executes X?"

"Can X sleep?"

"What is the next function?"

Mỗi câu có:

Answer
Explanation

và answer phải collapsible.

============================================================
43. INTERACTIVE UI
============================================================

HTML phải có:

- sticky sidebar
- table of contents
- current section highlight
- search
- search highlight
- dark mode
- light mode
- collapse / expand
- copy code
- copy section link
- diagram zoom
- back to top
- reading progress
- study mode
- quick review mode

Keyboard shortcuts:

/
    Search

Esc
    Close search

Home
    Top

End
    Bottom

============================================================
44. STUDY MODE
============================================================

STUDY MODE:

hiển thị:

- full explanation
- diagrams
- source code
- call graphs
- deep dive
- why explanation
- runtime

============================================================
45. QUICK REVIEW MODE
============================================================

QUICK REVIEW:

chỉ hiển thị:

- architecture
- important concepts
- important functions
- major call chains
- key diagrams
- cheat sheet

============================================================
46. SEARCH
============================================================

Search phải tìm được:

- function
- structure
- file
- concept
- section
- diagram title

Khi tìm:

highlight kết quả.

============================================================
47. DIAGRAM DESIGN
============================================================

Dùng consistent visual language:

BLUE
= userspace

PURPLE
= kernel

ORANGE
= CPU / MMU / architecture

GREEN
= memory / data / resource

YELLOW
= synchronization

RED
= error / failure

Mỗi diagram có legend.

============================================================
48. DIAGRAM RULE
============================================================

ĐỪNG VẼ DIAGRAM ĐỂ TRANG TRÍ.

Mỗi diagram phải trả lời MỘT câu hỏi:

"Diagram này giúp tôi hiểu điều gì?"

Ví dụ:

Where does this component live?

Who calls this function?

Where does this data go?

What happens next?

What happens on failure?

Who owns this object?

What runs in IRQ context?

============================================================
49. LARGE GRAPH RULE
============================================================

Nếu graph quá lớn:

KHÔNG nhồi vào một hình.

Chia thành:

Overview Graph
     ↓
Component Graph
     ↓
Function Graph
     ↓
Detailed Call Graph

Có zoom.

Có expand/collapse nếu phù hợp.

============================================================
50. SOURCE ACCURACY
============================================================

TUYỆT ĐỐI KHÔNG BỊA.

Không tự bịa:

- function
- file
- line
- structure
- call relationship
- kernel behavior
- architecture behavior

Nếu không xác minh được:

ghi:

[NEEDS VERIFICATION]

Phân biệt:

[SOURCE]

[RESEARCH]

[INFERENCE]

[MODERN KERNEL CONTEXT]

============================================================
51. VERSION AWARENESS
============================================================

Nếu nghiên cứu dựa trên kernel version cụ thể:

ghi rõ version.

Nếu behavior khác giữa versions:

tạo:

| Topic | Reference Version | Modern Version | Difference |
|------|-------------------|-----------------|------------|

Không tự ý thay thế version trong tài liệu.

============================================================
52. CONTENT ORGANIZATION
============================================================

Mỗi major section nên có cấu trúc:

1. Overview
2. Why it exists
3. Where it fits
4. Architecture
5. Data structures
6. Important functions
7. Function relationships
8. Call chain
9. Runtime flow
10. Error path
11. Source code
12. Debugging
13. Mental model
14. If you remember only 5 things

============================================================
53. NAVIGATION STRUCTURE
============================================================

Sidebar:

01 Overview
02 Global Architecture
03 Chapter Architecture
04 Components
05 Data Structures
06 Functions
07 Function Relationships
08 Call Graph
09 Runtime Flow
10 Data Flow
11 Control Flow
12 Lifecycle
13 Concurrency
14 Error Paths
15 Source Tree
16 Source Navigation
17 Debugging
18 Labs
19 Knowledge Map
20 Learning Path
21 Quiz
22 Cheat Sheet

Điều chỉnh theo nội dung thực tế Chapter 13.

============================================================
54. VISUAL HIERARCHY
============================================================

Mỗi section phải nhìn vào là biết:

LEVEL 0
Chapter

LEVEL 1
Major subsystem

LEVEL 2
Component

LEVEL 3
Concept

LEVEL 4
Function

LEVEL 5
Implementation

============================================================
55. CODE DISPLAY
============================================================

Code block:

- dark background
- readable font
- line number
- copy button
- filename header
- function name
- language label

Ví dụ:

----------------------------------------
mm/example.c
function: example()
----------------------------------------

code...

============================================================
56. RESPONSIVE DESIGN
============================================================

Desktop:

Sidebar + Main Content + optional Right Panel

Tablet:

Sidebar collapsible

Mobile:

Sidebar drawer

Diagram:

horizontal scrolling khi cần.

Code:

horizontal scrolling.

============================================================
57. PRINT MODE
============================================================

Ctrl + P:

- hide sidebar
- hide buttons
- hide interactive controls
- keep diagrams
- keep source code
- preserve heading hierarchy
- avoid awkward page breaks

============================================================
58. PERFORMANCE
============================================================

Document có thể rất dài.

Không dùng:

- heavy animation
- huge libraries
- remote JS
- remote CSS
- CDN

Use:

- vanilla JavaScript
- CSS
- inline SVG

============================================================
59. SELF VALIDATION
============================================================

TRƯỚC KHI XUẤT FILE:

Kiểm tra:

[ ] Master Architecture
[ ] Global Overview
[ ] Layer Architecture
[ ] Component Architecture
[ ] Function Graph
[ ] Function Relationship
[ ] Call Chain
[ ] Data Flow
[ ] Control Flow
[ ] Runtime Sequence
[ ] Lifecycle
[ ] Error Path
[ ] Concurrency
[ ] Source Tree
[ ] Source Navigation
[ ] Architecture → Source Code bridge
[ ] Function Index
[ ] Structure Index
[ ] Dependency Graph
[ ] Knowledge Map
[ ] Learning Path
[ ] Debugging
[ ] Labs
[ ] Quiz
[ ] Cheat Sheet
[ ] Search
[ ] Dark Mode
[ ] Quick Review
[ ] Study Mode
[ ] Print Mode

============================================================
60. FINAL QUALITY TEST
============================================================

Hãy tự kiểm tra document bằng cách đọc theo 5 câu hỏi:

1.

"Nếu tôi chưa biết Chapter 13,
tôi có hiểu Chapter này giải quyết vấn đề gì không?"

2.

"Nếu tôi biết architecture,
tôi có biết các component liên quan với nhau như thế nào không?"

3.

"Nếu tôi biết function A,
tôi có biết ai gọi A và A gọi ai không?"

4.

"Nếu tôi đứng tại function A trong source code,
tôi có biết đi đâu tiếp để hiểu toàn bộ flow không?"

5.

"Nếu runtime xảy ra,
tôi có thể trace từ event → function → data structure → subsystem
→ hardware → return không?"

Nếu câu trả lời là KHÔNG:

BỔ SUNG DIAGRAM / GRAPH / EXPLANATION.

Đừng chỉ thêm text.

============================================================
61. FINAL INSTRUCTION
============================================================

Điều quan trọng nhất:

ĐỪNG biến kết quả thành một "collection of notes".

Hãy biến nó thành:

    A COHERENT SYSTEM

Tôi muốn khi nhìn vào HTML:

Architecture
    ↓
Component
    ↓
Data Structure
    ↓
Function
    ↓
Caller / Callee
    ↓
Call Chain
    ↓
Runtime
    ↓
Source Code
    ↓
Hardware

tất cả đều được liên kết.

Tài liệu phải giúp tôi hình thành:

"MENTAL MODEL OF THE SUBSYSTEM"

chứ không chỉ nhớ từng function riêng lẻ.

ƯU TIÊN:

VISUALIZE RELATIONSHIPS
OVER
LISTING INFORMATION.

Nếu cần thêm diagram để làm rõ:

HÃY THÊM DIAGRAM.

Nếu cần thêm call graph:

HÃY THÊM CALL GRAPH.

Nếu cần thêm sequence diagram:

HÃY THÊM SEQUENCE DIAGRAM.

Nếu cần chia một diagram thành 3-5 diagram:

HÃY CHIA.

Nếu nội dung dài:

HÃY ĐỂ HTML DÀI.

KHÔNG CẮT KIẾN THỨC CHỈ ĐỂ HTML NGẮN.

Cuối cùng:

HÃY TẠO FILE

Linux_Kernel_Chapter_13_Deep_Dive.html

và kiểm tra nó trước khi hoàn thành.