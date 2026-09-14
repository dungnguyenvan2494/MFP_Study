Tôi cung cấp cho bạn:

1. PDF: Chapter 19 của tài liệu Linux Kernel. File tài liệu: linux_kernel_chap_19.pdf
2. Linux Kernel source code LOCAL của tôi: C:\Users\PC\Documents\IT6_Kernel\Kernel\K-S800\Src

Nhiệm vụ của bạn là:

ĐỌC TOÀN BỘ CHAPTER 19 VÀ GIẢI THÍCH TOÀN BỘ NỘI DUNG CHO TÔI, KHÔNG ĐƯỢC BỎ SÓT BẤT KỲ Ý NÀO.

Đây không phải yêu cầu summary.

Đây là yêu cầu:

FULL CONTENT WALKTHROUGH
+
SOURCE CODE DEEP DIVE
+
PAGE-BY-PAGE AUDIT
+
SENTENCE-BY-SENTENCE EXPLANATION

============================================================
0. NGUYÊN TẮC TUYỆT ĐỐI
============================================================

SOURCE CODE CỦA TÔI là source of truth.

PDF là source of truth về:

- nội dung
- thứ tự
- terminology
- figures
- tables
- examples
- assumptions
- explanations

KHÔNG được:

- bỏ qua đoạn văn
- bỏ qua câu
- bỏ qua footnote
- bỏ qua figure
- bỏ qua table
- bỏ qua source snippet
- bỏ qua ví dụ
- bỏ qua cross-reference
- tự rút gọn một paragraph thành một câu
- nói "phần này tương tự"
- nói "các chi tiết khác không quan trọng"
- bỏ qua một subsection vì cho rằng nó đơn giản

Nếu tài liệu có 10 câu thì phải xử lý cả 10 câu.

Nếu một câu có 3 ý thì phải giải thích cả 3 ý.

============================================================
1. ĐỌC TOÀN BỘ PDF TRƯỚC
============================================================

TRƯỚC KHI GIẢI THÍCH BẤT CỨ THỨ GÌ:

hãy đọc toàn bộ Chapter 19 từ trang đầu đến trang cuối.

Không được đọc vài trang đầu rồi bắt đầu trả lời.

Phải kiểm kê:

- chapter title
- section
- subsection
- paragraph
- figure
- table
- code snippet
- function
- struct
- macro
- system call
- API
- filesystem object
- data structure
- cross-reference
- example
- footnote

============================================================
2. TẠO CHAPTER INVENTORY
============================================================

Đầu tiên tạo:

# CHAPTER 19 INVENTORY

Liệt kê chính xác tất cả section/subsection theo đúng tài liệu.

Ví dụ những phần cần kiểm tra bao gồm:

19.1 Pipes
    19.1.1 Using a Pipe
    19.1.2 Pipe Data Structures
    19.1.2.1 The Pipes Special Filesystem
    19.1.3 Creating and Destroying a Pipe
    19.1.4 Reading from a Pipe
    19.1.5 Writing into a Pipe

19.2 FIFOs
    19.2.1 Creating and Opening a FIFO

19.3 System V IPC
    19.3.1 Using an IPC Resource
    19.3.2 The ipc() System Call
    19.3.3 IPC Semaphores
        19.3.3.1 Undoable semaphore operations
        19.3.3.2 The queue of pending requests
    19.3.4 IPC Messages
    19.3.5 IPC Shared Memory
        19.3.5.1 Swapping out pages of IPC shared memory regions
        19.3.5.2 Demand paging for IPC shared memory regions

19.4 POSIX Message Queues

NHƯNG:

Không được mặc định danh sách trên là đầy đủ.

Hãy kiểm tra PDF và sửa lại theo đúng nội dung thực tế.

============================================================
3. PAGE-BY-PAGE AUDIT
============================================================

Đây là yêu cầu BẮT BUỘC.

Hãy tạo:

# PAGE-BY-PAGE COVERAGE MATRIX

Cho TỪNG TRANG trong PDF:

Page N

- Section:
- Subsection:
- Paragraphs:
- Concepts:
- Functions:
- Structs:
- Macros:
- Tables:
- Figures:
- Examples:
- Cross references:
- Footnotes:
- Code snippets:
- Status: COVERED / NOT COVERED

Ví dụ:

Page 5

Paragraph 1:
    Concept A
    Concept B

Paragraph 2:
    Concept C

Table 19-x:
    struct xxx
    field1
    field2

Figure:
    pipe data structure

Status:
    COVERED

MỤC TIÊU:

KHÔNG CÓ TRANG NÀO Ở TRẠNG THÁI NOT COVERED.

============================================================
4. SENTENCE-BY-SENTENCE RULE
============================================================

Đối với từng paragraph:

KHÔNG summary paragraph.

Hãy tách thành từng ý logic.

Ví dụ:

Sentence 1:
"...."

Explanation:
...

Sentence 2:
"...."

Explanation:
...

Nếu câu dài:

Clause A:
...

Clause B:
...

Clause C:
...

Explanation:
...

Không được bỏ phần nào.

============================================================
5. KHÔNG ĐƯỢC “PARAPHRASE AWAY” Ý
============================================================

Nếu PDF nói:

X xảy ra vì A, B và C.

Không được giải thích:

"X xảy ra vì một số lý do."

Phải giữ nguyên:

A
B
C

Sau đó giải thích từng lý do.

============================================================
6. TEXTBOOK VS SOURCE CODE
============================================================

Sau khi giải thích nội dung của PDF:

hãy tìm implementation trong Linux Kernel source code của tôi.

Flow:

TEXTBOOK
   ↓
CONCEPT
   ↓
MY SOURCE TREE
   ↓
STRUCT
   ↓
FUNCTION
   ↓
CALLER
   ↓
CALLEE
   ↓
RUNTIME

============================================================
7. SOURCE CODE STRICT MODE
============================================================

Mọi function/struct/API được nêu phải được kiểm tra trong source tree.

Format:

Function:
foo()

Source:
path/to/file.c

Line:
xxxx

Definition:
...

Called by:
...

Calls:
...

Purpose:
...

Nếu có thể:
git commit / version information

Nếu không tìm thấy:

NOT FOUND IN MY SOURCE

Không được thay bằng function từ kernel khác rồi giả định đó là equivalent.

============================================================
8. SOURCE VERSION
============================================================

Đầu tiên xác định kernel version của source tree:

VERSION:
...

ARCH:
...

CONFIG:
...

Nếu có:

git commit:
...

Nếu source của tôi không đủ thông tin:

UNKNOWN

Không được đoán version.

============================================================
9. PIPES
============================================================

Phân tích toàn bộ section Pipes.

Phải giải thích không thiếu:

- pipe là gì
- one-way communication
- process relationship
- shell example
- file descriptors
- close()
- dup()
- dup2()
- parent/child
- read end
- write end
- blocking behavior
- synchronization
- pipe filesystem
- anonymous pipe
- data structures
- pipe buffers
- page-backed buffers
- readers/writers counters
- waiting readers
- waiting writers
- semaphores/locking
- pipe creation
- pipe destruction
- read operation
- write operation

Mỗi ý phải có source mapping.

============================================================
10. PIPE DATA STRUCTURES
============================================================

Phân tích struct trong textbook.

Ví dụ:

pipe_inode_info

pipe_buffer

pipe_buf_operations

Nhưng phải kiểm tra source của tôi.

Với mỗi struct:

Purpose
Fields
Who allocates it
Who initializes it
Who reads it
Who modifies it
Who frees it
Locking
Reference counting
Lifetime

Tạo diagram:

inode
 ↓
pipe_inode_info
 ↓
pipe_buffer[]
 ↓
page
 ↓
data

============================================================
11. PIPE SPECIAL FILESYSTEM
============================================================

Phân tích:

pipefs

hoặc implementation tương ứng trong source.

Giải thích:

VFS object
inode
superblock
mount
pipe inode

Tìm:

register filesystem

mount

inode creation

pipe inode lifecycle

============================================================
12. CREATING A PIPE
============================================================

Trace:

pipe()

→ syscall entry

→ kernel

→ actual function trong source của tôi

→ pipe creation

→ file descriptor creation

→ read/write ends

→ return userspace

Nếu ARM64:

EL0
 ↓
svc #0
 ↓
exception entry
 ↓
syscall
 ↓
pipe implementation

Tạo call graph đầy đủ.

============================================================
13. DUP / DUP2
============================================================

Giải thích pipe example trong textbook.

Trace:

pipe()
 ↓
fd
 ↓
dup()
 ↓
file descriptor table
 ↓
same struct file
 ↓
pipe endpoint

Đặc biệt giải thích:

fd duplication

vs

file object duplication.

============================================================
14. PIPE READ
============================================================

Trace:

read(fd, buf, count)

→ pipe read implementation

Phải giải thích tất cả các trường hợp trong bảng textbook.

Bao gồm:

- no writer
- writer exists
- data available
- empty pipe
- blocking
- nonblocking
- partial read
- requested size smaller than available
- requested size larger than available
- return values
- EINTR
- EAGAIN

Nhưng chỉ ghi errno nếu source của tôi xác nhận.

============================================================
15. PIPE WRITE
============================================================

Trace:

write(fd, buf, count)

Giải thích:

- reader exists
- no reader
- available buffer space
- blocking write
- nonblocking write
- count <= pipe capacity
- count > pipe capacity
- partial write
- SIGPIPE
- EPIPE
- wakeup reader
- wakeup writer

Phải trace source.

============================================================
16. PIPE WAIT QUEUES
============================================================

Tìm:

wait queue

reader waiting

writer waiting

Giải thích:

Process A:
read()

→ sleep

Process B:
write()

→ data available

→ wake A

Tạo timeline:

T0
T1
T2
T3

============================================================
17. PIPE BUFFER LIFECYCLE
============================================================

Tạo:

ALLOCATE
 ↓
INITIALIZE
 ↓
WRITE DATA
 ↓
READ DATA
 ↓
ADVANCE BUFFER
 ↓
RELEASE
 ↓
FREE

Tìm function tương ứng.

============================================================
18. FIFOs
============================================================

Phân tích toàn bộ section FIFOs.

Giải thích:

named pipe

vs

anonymous pipe

Filesystem namespace

mkfifo()

open()

read()

write()

blocking open

O_NONBLOCK

reader/writer matching

============================================================
19. FIFO CREATION
============================================================

Trace:

mkfifo()

→ syscall/libc

→ VFS

→ filesystem

→ special inode

→ FIFO

Tìm source function thật.

============================================================
20. FIFO OPEN
============================================================

Đây là phần QUAN TRỌNG.

Trace các trường hợp:

O_RDONLY
O_WRONLY
O_RDWR
O_NONBLOCK

Phân tích:

reader opens first

writer opens first

blocking

nonblocking

wake up

errors

============================================================
21. SYSTEM V IPC
============================================================

Phân tích toàn bộ section System V IPC.

Giải thích:

- IPC identifier
- IPC key
- IPC resource
- semaphore
- message queue
- shared memory
- permissions
- owner
- creator
- sequence number
- identifier reuse protection

============================================================
22. IPC KEY
============================================================

Giải thích:

key_t

IPC_PRIVATE

ftok()

key generation

collision

permissions

Phân tích source.

============================================================
23. IPC IDENTIFIER
============================================================

Textbook nói về:

IPC identifier

sequence number

slot index

Công thức identifier.

Giải thích tại sao kernel không tái sử dụng ID một cách đơn giản.

Trace:

create
 ↓
allocate slot
 ↓
sequence
 ↓
generate id
 ↓
userspace

============================================================
24. IPC BASE STRUCTURE
============================================================

Phân tích:

struct kern_ipc_perm

hoặc actual equivalent.

Fields:

key
uid
gid
cuid
cgid
mode
seq
security
deleted

Tạo table:

Field
Type
Meaning
Writer
Reader
Lock

============================================================
25. IPC ID MANAGEMENT
============================================================

Phân tích:

idr/xarray/array/namespace

hoặc actual structure trong source.

Giải thích:

ID → slot → object

và:

ID reuse

sequence protection.

============================================================
26. IPC SEMAPHORES
============================================================

Phân tích:

semget()
semctl()
semop()

hoặc actual syscall implementation.

Trace:

userspace
 ↓
semget()
 ↓
kernel
 ↓
IPC subsystem
 ↓
semaphore set

============================================================
27. SEMAPHORE ARRAY
============================================================

Phân tích:

struct sem_array
struct sem
struct sem_queue
struct sem_undo
struct kern_ipc_perm

Tạo relationship diagram.

============================================================
28. SEMOP
============================================================

Trace:

semop()

→ request validation

→ operation array

→ semaphore changes

→ blocking condition

→ wait queue

→ wakeup

→ completion

Giải thích từng step source-level.

============================================================
29. UNDOABLE SEMAPHORE OPERATIONS
============================================================

Phân tích:

SEM_UNDO

struct sem_undo

undo list

process exit

rollback

Giải thích:

Process A:
semop(SEM_UNDO)
 ↓
sem value changed
 ↓
process terminates
 ↓
kernel finds undo structure
 ↓
restore semaphore state

Trace source.

============================================================
30. PENDING SEMAPHORE REQUESTS
============================================================

Phân tích:

struct sem_queue

Fields:

next
prev
sleeper
undo
pid
status
sops
nsops

Giải thích từng field.

Tạo timeline:

Request enters queue
 ↓
sleep
 ↓
another process changes semaphore
 ↓
recheck
 ↓
execute OR continue sleeping

============================================================
31. SEMAPHORE WAITING ALGORITHM
============================================================

Giải thích:

Tại sao request có thể block?

Điều kiện wakeup là gì?

Kernel scan pending operations thế nào?

Khi nào request được:

- granted
- retried
- removed
- cancelled

============================================================
32. SYSTEM V MESSAGE QUEUE
============================================================

Phân tích:

msgget()
msgsnd()
msgrcv()
msgctl()

Trace source.

============================================================
33. MESSAGE QUEUE DATA STRUCTURES
============================================================

Phân tích:

struct msg_queue
struct msg_msg
struct kern_ipc_perm

và actual source structures.

Tạo:

IPC identifier
 ↓
msg_queue
 ↓
message list
 ↓
msg_msg
 ↓
message data

============================================================
34. MESSAGE STRUCTURE
============================================================

Giải thích:

message type
message size
message text
next
security

Nếu message text lớn hơn một page:

phân tích các linked chunks/page fragments theo source.

============================================================
35. MSGSND
============================================================

Trace:

msgsnd()

→ validate

→ lookup IPC resource

→ allocate message

→ copy from userspace

→ insert queue

→ wake receiver

Tìm source.

============================================================
36. MSGRCV
============================================================

Trace:

msgrcv()

→ lookup queue

→ find message

→ type filtering

→ copy to userspace

→ remove message

Phân tích:

msgtyp

including:

positive
zero
negative

nếu có.

============================================================
37. MESSAGE QUEUE BLOCKING
============================================================

Giải thích:

queue empty

→ receiver waits

queue full

→ sender waits

nonblocking

→ immediate return

Trace source.

============================================================
38. SYSTEM V SHARED MEMORY
============================================================

Phân tích:

shmget()
shmat()
shmdt()
shmctl()

Trace:

create
 ↓
attach
 ↓
map
 ↓
access
 ↓
detach
 ↓
destroy

============================================================
39. SHARED MEMORY DATA STRUCTURES
============================================================

Phân tích:

struct shmid_kernel

hoặc actual equivalent.

Quan hệ:

IPC id
 ↓
shmid_kernel
 ↓
file
 ↓
dentry
 ↓
inode
 ↓
address_space
 ↓
pages

Tạo diagram.

============================================================
40. SHARED MEMORY VS MMAP
============================================================

Giải thích:

System V SHM

vs

normal mmap()

Phải chỉ ra:

- data structures
- VMA
- file
- mapping
- page cache
- attach
- detach

============================================================
41. SHM ATTACH
============================================================

Trace:

shmat()

→ validate

→ find address

→ create mapping

→ VMA

→ shared memory object

→ page fault later

Tìm source thật.

============================================================
42. SHM DETACH
============================================================

Trace:

shmdt()

→ unmap

→ update references
→ cleanup

Giải thích lifecycle.

============================================================
43. SWAPPING SHARED MEMORY
============================================================

Phân tích toàn bộ subsection về swapping.

Giải thích:

shared memory page

→ inactive memory

→ swap

→ later fault

Đặc biệt:

tại sao page của System V SHM vẫn có thể swap?

Tìm source path.

============================================================
44. DEMAND PAGING SHARED MEMORY
============================================================

Trace:

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
SHM fault handling
 ↓
page lookup/allocation
 ↓
map page
 ↓
resume process

Nếu source của tôi ARM64:

trace exception:

EL0
 ↓
data abort
 ↓
ARM64 fault handler
 ↓
handle_mm_fault
 ↓
VMA fault
 ↓
SHM

============================================================
45. POSIX MESSAGE QUEUES
============================================================

Phân tích toàn bộ:

mq_open()
mq_close()
mq_unlink()
mq_send()
mq_receive()
mq_timedsend()
mq_timedreceive()
mq_notify()
mq_getattr()
mq_setattr()

Nhưng phải kiểm tra syscall/source thực tế.

============================================================
46. POSIX MQ VS SYSTEM V MQ
============================================================

Tạo bảng:

| Feature | System V MQ | POSIX MQ |

So sánh:

identifier
filesystem representation
message priority
blocking
notification
permissions
lifecycle
namespace
kernel data structure
syscall/API
implementation

============================================================
47. POSIX MQ DATA STRUCTURES
============================================================

Tìm source.

Phân tích:

mq object
inode
dentry
queue
message
waiters
notification

Tạo data structure graph.

============================================================
48. POSIX MQ FILESYSTEM
============================================================

Nếu source của tôi có:

mqueue filesystem

phân tích:

mount
inode
file_operations
file descriptor
mq_open()

Trace:

mq_open()
 ↓
mqueue filesystem
 ↓
inode
 ↓
file
 ↓
queue object

============================================================
49. NOTIFICATION
============================================================

Phân tích:

mq_notify()

Các notification mechanisms:

- signal
- thread
- event
- callback equivalent

Chỉ đưa mechanism tồn tại trong source của tôi.

============================================================
50. BLOCKING SEMANTICS
============================================================

So sánh:

blocking
nonblocking
timed

cho:

Pipe
FIFO
System V semaphore
System V message queue
POSIX message queue

============================================================
51. IPC OBJECT LIFECYCLE
============================================================

Tạo lifecycle:

CREATE
 ↓
LOOKUP
 ↓
USE
 ↓
BLOCK
 ↓
WAKE
 ↓
MODIFY
 ↓
DETACH
 ↓
REMOVE
 ↓
FREE

Cho từng IPC subsystem.

============================================================
52. LOCKING / CONCURRENCY
============================================================

Đây là phần BẮT BUỘC.

Phân tích:

Pipe:
- lock
- wait queue

FIFO:
- inode locking

SysV IPC:
- ipc lock
- semaphore lock
- queue lock

Shared memory:
- mmap/vma/page lock

POSIX MQ:
- queue lock
- wait queues

Với từng function:

Context:
...

Lock:
...

Lock order:
...

Can sleep:
YES/NO

Interrupt context:
YES/NO

============================================================
53. REFERENCE COUNT
============================================================

Trace:

file reference
inode reference
IPC object reference
page reference
VMA reference

Giải thích:

get
+
put
+
free

============================================================
54. ERROR PATH
============================================================

Với mỗi API:

hãy trace error path trong source.

Ví dụ:

pipe()
→ ENOMEM

mkfifo()
→ EEXIST

semget()
→ EEXIST / ENOMEM / EACCES

msgsnd()
→ EAGAIN / EINTR / EIDRM

shmget()
→ EEXIST / ENOSPC / ENOMEM

mq_open()
→ EACCES / EEXIST / ENOMEM

Nhưng:

CHỈ đưa errno nếu source của tôi hoặc tài liệu xác nhận.

============================================================
55. USERSPACE → KERNEL
============================================================

Đối với mỗi system call:

- arguments
- syscall number
- ABI
- entry
- dispatcher
- kernel implementation

Nếu ARM64:

EL0
 ↓
svc #0
 ↓
vector
 ↓
el0_sync
 ↓
syscall handling
 ↓
specific syscall

============================================================
56. DATA FLOW
============================================================

Với mỗi subsystem hãy chỉ ra:

Userspace data
↓
Kernel object
↓
copy_from_user / copy_to_user
↓
internal buffer
↓
IPC object
↓
other process

Ví dụ pipe:

Process A
 ↓
write()
 ↓
pipe_buffer
 ↓
page
 ↓
read()
 ↓
Process B

============================================================
57. PROCESS RELATIONSHIP
============================================================

Đặc biệt với:

pipe

hãy giải thích:

parent
child
fork()
dup()
file descriptor inheritance

Tại sao pipe thường được dùng sau fork()?

============================================================
58. NAMESPACE
============================================================

Nếu source có IPC namespaces:

phân tích:

IPC namespace

và relationship:

task
 ↓
nsproxy
 ↓
ipc namespace
 ↓
IPC objects

Chỉ đưa nếu source của tôi có.

============================================================
59. SECURITY
============================================================

Phân tích:

permission checking
credentials
uid
gid
capabilities
security hooks
SELinux hooks

Chỉ đưa những gì source xác minh.

============================================================
60. SOURCE CODE TABLE
============================================================

Cuối mỗi section tạo bảng:

| Concept | Struct | Function | Source | Line | Context |

Không được để function/path chưa kiểm tra.

============================================================
61. CALL GRAPH
============================================================

Tạo call graph cho:

pipe()
read(pipe)
write(pipe)

mkfifo()
open(FIFO)

semget()
semop()
semctl()

msgget()
msgsnd()
msgrcv()
msgctl()

shmget()
shmat()
shmdt()
shmctl()

mq_open()
mq_send()
mq_receive()
mq_notify()

============================================================
62. DATA STRUCTURE MAP
============================================================

Tạo master map:

PIPE
│
├── inode
├── pipe_inode_info
├── pipe_buffer
└── page

FIFO
│
├── inode
└── pipe infrastructure

SYSV IPC
│
├── kern_ipc_perm
├── sem_array
├── sem_queue
├── sem_undo
├── msg_queue
├── msg_msg
└── shmid_kernel

POSIX MQ
│
├── inode
├── queue
├── message
└── waiters

============================================================
63. FIGURE ANALYSIS
============================================================

Mọi figure trong PDF:

PHẢI được giải thích.

Format:

Figure X

What it shows:
...

Node-by-node:
...

Arrow-by-arrow:
...

Relationship:
...

Source-code equivalent:
...

Nếu figure thể hiện data structure:

mapping:

box
→ struct
→ field

============================================================
64. TABLE ANALYSIS
============================================================

Mọi table:

PHẢI được xử lý.

Ví dụ:

Table 19-x

Column:
...

Row 1:
...

Meaning:
...

Source equivalent:
...

Không được bỏ table.

============================================================
65. SOURCE SNIPPET ANALYSIS
============================================================

Nếu PDF có code:

BOOK CODE
...

Giải thích từng dòng.

Sau đó:

MY SOURCE
...

So sánh:

BOOK
vs
MY SOURCE

============================================================
66. OLD KERNEL / MODERN KERNEL
============================================================

Nếu chapter dùng API cũ:

KHÔNG tự sửa textbook.

Hãy trình bày:

TEXTBOOK IMPLEMENTATION

vs

MY KERNEL IMPLEMENTATION

Format:

Old:
...

My source:
...

Difference:
...

Reason:
...

Nếu không xác minh reason:

NOT VERIFIED.

============================================================
67. NO HALLUCINATION RULE
============================================================

Nếu không tìm thấy:

NOT FOUND

Nếu không chắc:

UNVERIFIED

Nếu function đã thay đổi:

CHANGED

Nếu đã bị remove:

REMOVED

Nếu equivalent:

REPLACED BY ...

Không được đoán.

============================================================
68. PERFORMANCE
============================================================

Nếu textbook hoặc source có performance implication:

giải thích:

throughput
latency
memory
copy overhead
blocking
wakeups
cache
page allocation
locking

============================================================
69. DEBUGGING
============================================================

Cho mỗi subsystem:

nói tôi debug bằng:

strace
ftrace
perf
trace-cmd
gdb
kgdb
bpftrace
dynamic_debug

nếu phù hợp.

Ví dụ:

trace pipe_write

trace pipe_read

trace semop

trace msgsnd

trace shmat

============================================================
70. PRACTICAL LABS
============================================================

Tạo lab để kiểm chứng source.

LAB 1
Parent-child pipe

LAB 2
Pipe blocking

LAB 3
Pipe nonblocking

LAB 4
FIFO reader/writer

LAB 5
System V semaphore

LAB 6
SEM_UNDO

LAB 7
System V message queue

LAB 8
System V shared memory

LAB 9
Shared memory page fault

LAB 10
POSIX message queue

Mỗi lab:

Goal
Code
Command
Expected behavior
Kernel function
Source file
What to observe

============================================================
71. END-TO-END SCENARIOS
============================================================

Tạo các scenario hoàn chỉnh.

SCENARIO 1:

shell:
ls | more

Trace:

shell
 ↓
pipe()
 ↓
fork()
 ↓
dup2()
 ↓
execve()
 ↓
write()
 ↓
pipe
 ↓
read()
 ↓
terminal

SCENARIO 2:

FIFO:

process A
 ↓
open FIFO
 ↓
block
 ↓
process B
 ↓
open FIFO
 ↓
wake
 ↓
write
 ↓
read

SCENARIO 3:

SysV semaphore:

Process A
 ↓
semop()
 ↓
block
 ↓
Process B
 ↓
semop()
 ↓
wake A

SCENARIO 4:

SysV shared memory:

shmget()
 ↓
shmat()
 ↓
VMA
 ↓
page fault
 ↓
page
 ↓
shared memory access

SCENARIO 5:

POSIX MQ:

mq_open()
 ↓
mq_send()
 ↓
message queue
 ↓
mq_receive()