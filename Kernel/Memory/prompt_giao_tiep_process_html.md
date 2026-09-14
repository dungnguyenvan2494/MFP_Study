Tôi có toàn bộ nội dung nghiên cứu của Chapter 19 – Linux Kernel IPC.

Hãy chuyển TOÀN BỘ nội dung đó thành MỘT FILE HTML DUY NHẤT.

Tên file:

linux_kernel_chapter_19_ipc.html

============================================================
1. MỤC TIÊU
============================================================

Tạo một tài liệu HTML có phong cách:

Linux Kernel Documentation
+
Technical Study Book
+
Interactive Source Code Reference

Mục tiêu của file HTML:

- đẹp
- dễ đọc
- dễ tìm kiếm
- dễ hiểu
- dễ học
- dễ tra cứu
- có cấu trúc rõ ràng
- giữ được TOÀN BỘ nội dung nghiên cứu

Đây không phải landing page.

Đây là tài liệu kỹ thuật chuyên sâu để tôi dùng học Linux Kernel.

============================================================
2. QUY TẮC QUAN TRỌNG NHẤT
============================================================

KHÔNG ĐƯỢC CẮT BỎ NỘI DUNG.

KHÔNG được:

- summarize quá mức
- bỏ paragraph
- bỏ câu
- bỏ function
- bỏ struct
- bỏ field
- bỏ bảng
- bỏ figure
- bỏ code
- bỏ ví dụ
- bỏ call flow
- bỏ error path
- bỏ locking
- bỏ phần source-code analysis
- bỏ phần legacy/modern
- bỏ checklist
- bỏ glossary

Tôi đã yêu cầu nghiên cứu không sót nội dung.

HTML phải chứa:

100% nội dung nghiên cứu đã tạo.

Chỉ được thay đổi:

- cách trình bày
- typography
- layout
- grouping
- navigation
- visual hierarchy

KHÔNG được thay đổi nội dung kỹ thuật.

============================================================
3. MỘT FILE DUY NHẤT
============================================================

Chỉ tạo:

linux_kernel_chapter_19_ipc.html

Toàn bộ:

HTML
CSS
JavaScript

nằm trong file.

Không tạo:

style.css
script.js
data.json

Không cần build.

File phải mở trực tiếp bằng:

double-click index/html

và hoạt động.

============================================================
4. DESIGN
============================================================

Phong cách:

Dark Developer Documentation

Giống:

Linux Kernel source documentation
+
VS Code
+
technical reference manual

Không dùng:

- gradient quá mạnh
- neon
- animation nhiều
- card quá bo tròn
- emoji
- màu sắc sặc sỡ
- marketing style

Ưu tiên:

- dark background
- border mảnh
- typography rõ
- monospace cho code
- spacing tốt
- hierarchy mạnh
- nhiều khoảng trắng hợp lý

============================================================
5. LAYOUT
============================================================

Thiết kế:

┌─────────────────────────────────────────────────────┐
│ HEADER                                              │
│ Linux Kernel · Chapter 19 · IPC                    │
├────────────────┬────────────────────────────────────┤
│                │                                    │
│ SIDEBAR        │ MAIN CONTENT                       │
│                │                                    │
│ Overview       │                                    │
│ Pipes          │                                    │
│ FIFOs          │                                    │
│ SysV IPC       │                                    │
│ Semaphores     │                                    │
│ Messages       │                                    │
│ Shared Memory  │                                    │
│ POSIX MQ       │                                    │
│ Source Map     │                                    │
│ Call Graph     │                                    │
│ Debugging      │                                    │
│ Labs           │                                    │
│ Glossary       │                                    │
│ Checklist      │                                    │
│                │                                    │
└────────────────┴────────────────────────────────────┘

Sidebar sticky.

Header sticky.

Main content có max-width hợp lý.

Responsive mobile.

============================================================
6. HEADER
============================================================

Header:

Linux Kernel Study

Chapter 19 — Interprocess Communication

Thêm:

Search
Dark/Light
Contents

Hiển thị badge:

Pipes
FIFO
System V IPC
Semaphore
Message Queue
Shared Memory
POSIX MQ

============================================================
7. HERO
============================================================

Trang đầu:

# Linux Kernel IPC

Subtitle:

A source-code-oriented study of pipes, FIFOs, System V IPC and POSIX message queues.

Hiển thị:

Chapter 19

và:

Source Code Deep Dive

============================================================
8. TABLE OF CONTENTS
============================================================

Sidebar phải theo đúng thứ tự nội dung.

Ít nhất:

01 Chapter Overview
02 Pipes
03 Pipe Data Structures
04 Pipe Special Filesystem
05 Creating / Destroying Pipe
06 Reading from Pipe
07 Writing into Pipe
08 FIFOs
09 Creating / Opening FIFO
10 System V IPC
11 IPC Resources
12 IPC Identifiers
13 IPC Semaphores
14 SEM_UNDO
15 Pending Semaphore Requests
16 IPC Message Queues
17 Message Structures
18 IPC Shared Memory
19 Shared Memory Swapping
20 Shared Memory Demand Paging
21 POSIX Message Queues
22 Source Code Map
23 Data Structure Map
24 Call Graphs
25 Locking
26 Lifecycle
27 Error Paths
28 Debugging
29 Practical Labs
30 Glossary
31 Learning Checklist
32 Final Mental Model

Nếu research content có thêm section thì PHẢI đưa vào.

============================================================
9. SECTION DESIGN
============================================================

Mỗi section lớn có format:

┌──────────────────────────────────┐
│ 19.x SECTION TITLE               │
│ Concept / Source / Runtime       │
└──────────────────────────────────┘

Sau đó:

WHAT

WHY

HOW

SOURCE CODE

DATA STRUCTURES

CALL FLOW

RUNTIME

LOCKING

ERROR

DEBUGGING

RELATED CONCEPTS

Không thay đổi nội dung research.

Chỉ tổ chức lại cho dễ đọc.

============================================================
10. ORIGINAL CONTENT PRESERVATION
============================================================

Giữ nguyên:

- terminology
- function name
- struct name
- source file
- source path
- code
- API
- tables
- examples
- reasoning
- conclusions

Không tự sửa technical content.

Nếu research có:

UNVERIFIED

LEGACY

REMOVED

REPLACED

FOUND

hãy giữ nguyên badge đó.

============================================================
11. CODE BLOCKS
============================================================

Mọi source code phải được trình bày trong code block chuyên nghiệp.

Format:

┌───────────────────────────────────────────────┐
│ fs/pipe.c                        COPY         │
├───────────────────────────────────────────────┤
│ 001  ...                                      │
│ 002  ...                                      │
│ 003  ...                                      │
└───────────────────────────────────────────────┘

Có:

Copy button

Line numbers

Syntax highlighting

Filename header

Function name

Nếu source location có line:

hiển thị line range.

============================================================
12. FUNCTION CARDS
============================================================

Mỗi function quan trọng:

┌──────────────────────────────────────────────┐
│ pipe_read()                                  │
│ fs/pipe.c                                    │
├──────────────────────────────────────────────┤
│ Purpose                                      │
│ ...                                          │
│                                              │
│ Caller                                       │
│ ...                                          │
│                                              │
│ Callee                                       │
│ ...                                          │
│                                              │
│ Context                                      │
│ ...                                          │
│                                              │
│ Locking                                      │
│ ...                                          │
└──────────────────────────────────────────────┘

Có thể click để expand.

============================================================
13. DATA STRUCTURE VISUALIZATION
============================================================

Đối với mỗi struct quan trọng:

Ví dụ:

struct pipe_inode_info

hiển thị dạng:

pipe_inode_info
│
├── wait
├── nrbufs
├── curbuf
├── buffers[]
├── tmp_page
├── readers
├── writers
└── waiting_writers

Click field → detail.

Tương tự:

pipe_buffer

kern_ipc_perm

sem_array

sem_queue

sem_undo

msg_queue

msg_msg

shmid_kernel

POSIX MQ structures

============================================================
14. RELATIONSHIP DIAGRAM
============================================================

Tạo diagram:

PIPE

inode
 ↓
pipe_inode_info
 ↓
pipe_buffer[]
 ↓
page
 ↓
data

SYSTEM V IPC

ipc_ids
 ↓
kern_ipc_perm
 ↓
IPC object
 ├── sem_array
 ├── msg_queue
 └── shmid_kernel

SEMAPHORE

sem_array
 ├── sem
 ├── sem_queue
 └── sem_undo

MESSAGE QUEUE

msg_queue
 ↓
msg_msg
 ↓
message data

SHARED MEMORY

shmid_kernel
 ↓
file
 ↓
inode
 ↓
address_space
 ↓
page

============================================================
15. PIPE FLOW
============================================================

Tạo visual:

pipe()
 ↓
create inode
 ↓
pipe_inode_info
 ↓
read fd
 +
write fd
 ↓
userspace processes

READ:

read()
 ↓
pipe read implementation
 ↓
check buffer
 ↓
copy data
 ↓
wake writer
 ↓
return

WRITE:

write()
 ↓
pipe write implementation
 ↓
check readers
 ↓
reserve buffer
 ↓
copy data
 ↓
wake reader
 ↓
return

Click từng bước để mở explanation.

============================================================
16. BLOCKING FLOW
============================================================

Visual:

Process A
   │
 read()
   │
   ▼
pipe empty
   │
   ▼
sleep
   │
   │
   │       Process B
   │          │
   │        write()
   │          │
   │          ▼
   │       data available
   │          │
   └──────────┤
              ▼
            wake A

Tương tự cho:

full pipe
writer blocked
reader blocked
FIFO open blocking

============================================================
17. FIFO
============================================================

Visual:

Filesystem namespace

/path/my_fifo
      │
      ▼
    inode
      │
      ▼
FIFO / pipe infrastructure

So sánh:

anonymous pipe
vs
named FIFO

============================================================
18. SYSTEM V IPC
============================================================

Tạo architecture:

                 System V IPC
                       │
          ┌────────────┼────────────┐
          ▼            ▼            ▼
       Semaphore     Message      Shared
                      Queue        Memory

Click từng thành phần.

============================================================
19. IPC IDENTIFIER
============================================================

Tạo visual:

IPC key
   ↓
slot
   ↓
sequence
   ↓
IPC identifier
   ↓
kernel object

Giải thích:

ID reuse protection.

============================================================
20. SEMAPHORE
============================================================

Visual:

semget()
   ↓
sem_array
   ↓
sem[]
   ↓
semop()
   ↓
check operation
   ↓
execute OR sleep
   ↓
wake
   ↓
complete

============================================================
21. SEM_UNDO
============================================================

Visual:

Process
 ↓
semop(SEM_UNDO)
 ↓
semaphore changed
 ↓
sem_undo record
 ↓
process exits
 ↓
kernel rollback

============================================================
22. SEMAPHORE WAIT QUEUE
============================================================

Visual:

sem_array
   │
   ▼
sem_pending
   │
   ├── request A
   ├── request B
   └── request C

Mỗi request:

process
operation array
status
undo
wait queue

============================================================
23. MESSAGE QUEUE
============================================================

Visual:

msgget()
 ↓
msg_queue
 ↓
┌───────────────┐
│ msg            │
│ msg            │
│ msg            │
└───────────────┘

msgsnd():
userspace
 ↓
kernel
 ↓
allocate message
 ↓
copy data
 ↓
enqueue
 ↓
wake receiver

msgrcv():

queue
 ↓
find message
 ↓
copy
 ↓
remove
 ↓
userspace

============================================================
24. MESSAGE TYPE
============================================================

Tạo interactive explanation:

msgtyp > 0
msgtyp = 0
msgtyp < 0

Giải thích đúng theo research.

============================================================
25. SHARED MEMORY
============================================================

Visual:

Process A
   │
   ▼
shmat()
   │
   ▼
VMA
   │
   ▼
shared memory object
   │
   ▼
pages
   ▲
   │
   ▼
Process B

============================================================
26. DEMAND PAGING
============================================================

Visual:

shmat()
 ↓
VMA created
 ↓
NO physical page
 ↓
CPU access
 ↓
page fault
 ↓
fault handler
 ↓
find/allocate page
 ↓
map page
 ↓
resume process

============================================================
27. SWAPPING
============================================================

Visual:

Shared Memory Page
        ↓
     reclaim
        ↓
      swap
        ↓
page not resident
        ↓
fault
        ↓
swap-in
        ↓
map again

============================================================
28. POSIX MESSAGE QUEUE
============================================================

Architecture:

mq_open()
 ↓
mqueue filesystem/object
 ↓
queue
 ↓
mq_send()
 ↓
message
 ↓
mq_receive()

============================================================
29. POSIX VS SYSV
============================================================

Create comparison table:

| Feature | SysV | POSIX |

Keep every point from research.

Không thêm kiến thức ngoài nếu research không có.

============================================================
30. LEGACY VS MODERN
============================================================

Nếu research đã phân tích:

legacy kernel
vs
modern kernel

hãy tạo dedicated section.

Badges:

LEGACY

MODERN

REPLACED

REMOVED

UNVERIFIED

============================================================
31. SOURCE MAP
============================================================

Tạo Source Explorer:

Kernel Source
│
├── fs/
│   ├── ...
│
├── ipc/
│   ├── ...
│
├── mm/
│   ├── ...
│
├── include/
│   └── ...
│
└── arch/
    └── ...

Mỗi file:

Purpose

Important functions

Related section

============================================================
32. CALL GRAPH
============================================================

Tạo interactive call graph:

pipe()
 ↓
...
 ↓
...

semop()
 ↓
...
 ↓
...

msgsnd()
 ↓
...
 ↓
...

msgrcv()
 ↓
...
 ↓
...

shmat()
 ↓
...
 ↓
...

mq_send()
 ↓
...
 ↓
...

Mỗi node clickable.

============================================================
33. LOCKING MATRIX
============================================================

Table:

| Function | Lock | Context | Can Sleep |

Dùng source/research hiện có.

============================================================
34. OBJECT LIFECYCLE
============================================================

Tạo lifecycle cho:

Pipe
FIFO
Semaphore
Message Queue
Shared Memory
POSIX MQ

Ví dụ:

CREATE
 ↓
INITIALIZE
 ↓
OPEN/ATTACH
 ↓
USE
 ↓
BLOCK
 ↓
WAKE
 ↓
CLOSE/DETACH
 ↓
DESTROY
 ↓
FREE

============================================================
35. ERROR FLOW
============================================================

Tạo error cards.

Ví dụ:

pipe()
 ├── ENOMEM
 └── ...

semop()
 ├── EINTR
 ├── EIDRM
 └── ...

msgsnd()
 ├── EAGAIN
 ├── EINTR
 └── ...

shmget()
 ├── ENOMEM
 └── ...

Chỉ hiển thị errors có trong research.

============================================================
36. DEBUGGING
============================================================

Tạo command cards:

strace
ftrace
perf
trace-cmd
gdb
kgdb
bpftrace

Mỗi card:

Goal
Command
Expected
Kernel Function

============================================================
37. PRACTICAL LABS
============================================================

Tạo accordion.

LAB 01
Parent → Child Pipe

LAB 02
Blocking Pipe

LAB 03
Nonblocking Pipe

LAB 04
FIFO

LAB 05
System V Semaphore

LAB 06
SEM_UNDO

LAB 07
Message Queue

LAB 08
Shared Memory

LAB 09
Shared Memory Page Fault

LAB 10
POSIX Message Queue

Mỗi lab:

Goal
Setup
Code
Commands
Expected Result
Source Function
What to Observe

============================================================
38. GLOSSARY
============================================================

Searchable glossary.

Terms:

pipe
FIFO
pipe_inode_info
pipe_buffer
wait queue
IPC key
IPC identifier
kern_ipc_perm
sem_array
sem_queue
sem_undo
msg_queue
msg_msg
shared memory
shmid_kernel
POSIX MQ
mqueue
blocking
nonblocking
SEM_UNDO

Nếu research có thêm term thì thêm hết.

============================================================
39. SEARCH
============================================================

Implement global search.

Shortcut:

Ctrl + K

Search được:

- concept
- function
- struct
- field
- source file
- macro
- system call
- error
- glossary

Ví dụ search:

sem_undo

Kết quả:

SEM_UNDO
struct sem_undo
semaphore lifecycle
process exit
source

============================================================
40. CODE COPY
============================================================

Mỗi code block có:

Copy

Sau khi click:

Copied!

============================================================
41. DARK / LIGHT MODE
============================================================

Dark mode mặc định.

Có toggle.

Dùng localStorage.

============================================================
42. READING PROGRESS
============================================================

Hiển thị:

Reading Progress: XX%

Sidebar tự highlight section hiện tại.

============================================================
43. MOBILE
============================================================

Mobile:

sidebar → drawer

diagram → horizontal scrolling

table → horizontal scrolling

code → horizontal scrolling

============================================================
44. INTERACTIVE DIAGRAMS
============================================================

Diagram không nên là ảnh tĩnh.

Ưu tiên:

SVG inline
CSS
HTML

Các diagram phải clickable.

Node click:

scroll tới section hoặc mở detail panel.

============================================================
45. CONTENT HIERARCHY
============================================================

Dùng hierarchy:

H1:
Chapter

H2:
Section

H3:
Subsection

H4:
Concept

Code:
monospace

Function:
highlight

Struct:
highlight

Source:
code badge

Legacy:
badge

Modern:
badge

============================================================
46. “DEEP DIVE” BOX
============================================================

Những phần khó:

SEM_UNDO
IPC ID
Page Fault
Blocking
Wait Queue
Shared Memory

dùng box:

┌──────────────────────────────────────────┐
│ DEEP DIVE                                │
│                                          │
│ ...                                      │
└──────────────────────────────────────────┘

============================================================
47. “MENTAL MODEL” BOX
============================================================

Sau mỗi section quan trọng:

MENTAL MODEL

Ví dụ:

Pipe:

writer
 ↓
pipe buffer
 ↓
reader

Semaphore:

operation
 ↓
condition
 ↓
wait
 ↓
wake
 ↓
retry

Shared memory:

object
 ↓
VMA
 ↓
page
 ↓
access

============================================================
48. FINAL MASTER DIAGRAM
============================================================

Cuối tài liệu tạo:

                    LINUX KERNEL IPC
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
      PIPE/FIFO          SYSV IPC          POSIX MQ
        │                  │                  │
        │           ┌──────┼──────┐           │
        │           │      │      │           │
        │          SEM     MSG    SHM          │
        │           │      │      │           │
        └───────────┴──────┴──────┴────────────┘
                           │
                           ▼
                         VFS
                           │
                    Kernel Objects
                           │
                 ┌─────────┴─────────┐
                 ▼                   ▼
              Processes             MM
                 │                   │
                 ▼                   ▼
             Scheduler           Page Fault