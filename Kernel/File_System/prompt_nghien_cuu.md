Tôi đang nghiên cứu:

Chapter 16 – "Accessing Files"

Tôi đã có một Linux Kernel source tree LOCAL.

Hãy sử dụng SOURCE CODE MÀ TÔI CUNG CẤP làm nguồn tham chiếu chính.

TUYỆT ĐỐI KHÔNG được tự ý thay source của tôi bằng Linux Kernel source trên internet hoặc một version khác.

============================================================
0. SOURCE OF TRUTH
============================================================

SOURCE CODE CỦA TÔI là SOURCE OF TRUTH.

Hãy trước tiên xác định:

- kernel version
- architecture
- source tree layout
- config nếu có
- commit/version nếu có

Ví dụ:

Linux source:
C:\Users\PC\Documents\IT6_Kernel\Kernel\K-S800\Src

Architecture:
arm64

Kernel:
5.10.x

Nếu không xác định được version:

hãy ghi:

KERNEL VERSION: UNKNOWN

Không được tự đoán.

Nếu tôi cung cấp:

CONFIG
defconfig
Makefile
Makefile kernel
.git metadata

hãy dùng chúng để xác định version.

============================================================
1. NGUYÊN TẮC QUAN TRỌNG NHẤT
============================================================

Mọi function / struct / macro / source path phải được lấy từ source tree của tôi.

Không được:

- tự bịa function
- tự bịa source path
- lấy implementation từ kernel khác
- giả định API giống Linux mainline hiện tại
- sửa textbook bằng kiến thức ngoài mà không nói rõ

Nếu textbook nói:

generic_file_read()

nhưng source của tôi không có function đó:

ghi:

TEXTBOOK:
generic_file_read()

MY KERNEL:
NOT PRESENT

Sau đó tìm implementation thực tế tương ứng trong source tree của tôi.

============================================================
2. MỤC TIÊU
============================================================

Tôi muốn hiểu Chapter 16 ở mức:

USERSPACE
→ SYSCALL
→ VFS
→ FILESYSTEM
→ PAGE CACHE
→ MM
→ DIRECT I/O / BUFFERED I/O
→ BIO
→ BLOCK LAYER
→ DRIVER
→ HARDWARE

Đặc biệt phải trả lời:

read() thực sự làm gì?

write() thực sự làm gì?

mmap() thực sự làm gì?

O_DIRECT hoạt động thế nào?

Asynchronous I/O hoạt động thế nào?

============================================================
3. CHAPTER MAP
============================================================

Đọc Chapter 16 và tạo study map:

Chapter 16
│
├── Accessing Files
│
├── Reading and Writing a File
│   ├── read()
│   ├── write()
│   ├── generic file read
│   ├── generic file write
│   ├── read-ahead
│   └── writeback
│
├── Memory Mapping
│   ├── mmap()
│   ├── shared mapping
│   ├── private mapping
│   ├── VMA
│   ├── demand paging
│   └── dirty mapped pages
│
├── Direct I/O
│   ├── O_DIRECT
│   ├── generic direct I/O
│   └── page cache bypass
│
└── Asynchronous I/O
    ├── aio_read()
    ├── aio_write()
    ├── io_submit()
    ├── completion
    └── AIO context

============================================================
4. FIRST TASK: BUILD SOURCE MAP
============================================================

Trước khi giải thích:

hãy tìm trong source tree của tôi:

- syscall implementation
- VFS
- file operations
- read/write
- mmap
- page cache
- read-ahead
- direct I/O
- async I/O
- filesystem example
- block layer

Tạo bảng:

| Concept | Source file | Function | Struct | Status |
|---------|-------------|----------|--------|--------|

Không được tiếp tục phân tích sâu nếu chưa lập source map.

============================================================
5. READ() END-TO-END
============================================================

Trace:

read(fd, buf, 4096)

từ userspace cho tới kernel.

Phải xác định chính xác trong source của tôi:

ARM64 syscall entry
        ↓
syscall dispatcher
        ↓
__arm64_sys_read()
        ↓
VFS
        ↓
file operation
        ↓
filesystem
        ↓
page cache
        ↓
block I/O
        ↓
completion
        ↓
userspace

Nếu source của tôi dùng function khác:

dùng function thực tế.

Với MỖI function:

Function:
Source:
Line:
Caller:
Callee:
Arguments:
Return:
Data structures:
Context:
Lock:
Can sleep:
Error path:

============================================================
6. ARM64 SYSCALL ENTRY
============================================================

Nếu source của tôi là ARM64:

trace:

userspace
 ↓
svc #0
 ↓
exception vector
 ↓
el0_sync
 ↓
syscall entry
 ↓
syscall dispatch
 ↓
__arm64_sys_read

Xác định function thật trong source tree.

Tìm:

arch/arm64/kernel/entry.S

hoặc implementation tương ứng.

Giải thích:

x0
x1
x2
x8

cách syscall arguments đi vào kernel.

Sau đó nối:

ARM64 syscall
→ VFS

============================================================
7. VFS READ
============================================================

Phân tích:

read()

→ vfs_read()

→ file->f_op->read

hoặc implementation thực tế trong source.

Không được ép source của tôi theo textbook.

Giải thích:

struct file

struct inode

struct file_operations

Quan hệ:

fd
 ↓
files_struct
 ↓
struct file
 ↓
f_op
 ↓
filesystem

============================================================
8. GENERIC FILE READ
============================================================

Chapter nói về:

generic_file_read()

Hãy tìm implementation tương ứng trong source tree của tôi.

Có thể là:

generic_file_read_iter()
generic_file_buffered_read()
do_generic_file_read()

hoặc function khác.

Phải chứng minh bằng source.

Giải thích:

file
 ↓
mapping
 ↓
page cache
 ↓
page
 ↓
copy to userspace

============================================================
9. READ REQUEST DESCRIPTOR
============================================================

Nếu chapter có cấu trúc cũ như:

kiocb
read descriptor
iov

hãy mapping với source của tôi.

Phân tích:

struct kiocb
struct iov_iter
struct file

hoặc structures thực tế.

Giải thích:

kiocb

dùng cho cái gì?

iov_iter

dùng để làm gì?

============================================================
10. PAGE CACHE READ
============================================================

Trace page-cache read.

Flow:

read()
 ↓
mapping
 ↓
page cache lookup
 ↓
HIT / MISS

HIT:

page cache
 ↓
page
 ↓
copy_to_user()

MISS:

lookup
 ↓
allocate page
 ↓
readpage / readahead
 ↓
BIO
 ↓
block layer
 ↓
device
 ↓
completion
 ↓
PageUptodate
 ↓
copy_to_user()

Phải dùng source của tôi.

============================================================
11. PAGE CACHE CONNECTION
============================================================

Chapter 16 phải được nối với Chapter 15.

Xây dựng:

Chapter 16
READ
 ↓
address_space
 ↓
Page Cache
 ↓
XArray / actual cache structure
 ↓
page
 ↓
BIO

Giải thích source-level.

============================================================
12. READ-AHEAD
============================================================

Chapter có phần:

readpage
readahead
page_cache_readahead

Hãy tìm implementation thật trong source của tôi.

Giải thích:

sequential access

→ kernel phát hiện thế nào?

→ readahead window được tạo như thế nào?

→ bao nhiêu page?

→ khi nào readahead tăng?

→ khi nào giảm?

→ cache hit/miss ảnh hưởng thế nào?

Nếu source của tôi đã chuyển sang:

ondemand readahead
file_ra_state
page_cache_async_readahead

hoặc implementation khác:

dùng source thật.

============================================================
13. TRACE REAL SEQUENTIAL READ
============================================================

Ví dụ:

read(file, 4KB)
read(file, 4KB)
read(file, 4KB)
read(file, 4KB)

Trace:

read 1
 ↓
cache miss
 ↓
readahead
 ↓
multiple pages loaded

read 2
 ↓
cache hit

read 3
 ↓
cache hit

Giải thích source code.

============================================================
14. RANDOM ACCESS
============================================================

So sánh:

Sequential:

0
1
2
3
4

Random:

100
5
900
12
400

Giải thích read-ahead algorithm phản ứng như thế nào.

Tìm source condition/heuristic thực tế.

============================================================
15. HANDLE_RA_MISS
============================================================

Chapter có:

handle_ra_miss()

Hãy tìm modern equivalent trong source tree của tôi.

Giải thích:

read-ahead miss

được phát hiện thế nào?

kernel làm gì tiếp theo?

============================================================
16. WRITE()
============================================================

Trace:

write(fd, buf, 4096)

Flow:

userspace
 ↓
syscall
 ↓
VFS
 ↓
file->f_op
 ↓
filesystem
 ↓
page cache
 ↓
copy_from_user()
 ↓
mark page dirty
 ↓
writeback
 ↓
BIO
 ↓
block layer
 ↓
device

Phải phân biệt:

write()

KHÔNG nhất thiết đồng nghĩa:

disk write ngay lập tức.

Giải thích chính xác bằng source.

============================================================
17. FILE WRITE ITER
============================================================

Tìm implementation thật:

generic_file_write_iter()
new_sync_write()
generic_perform_write()

hoặc equivalent trong source của tôi.

Trace call graph.

============================================================
18. PREPARE_WRITE / COMMIT_WRITE
============================================================

Chapter dùng:

prepare_write()
commit_write()

Đây là legacy API.

Tìm modern equivalent.

Ví dụ có thể liên quan:

write_begin()
write_end()

Nhưng KHÔNG được tự giả định.

Phải tìm source thật.

Tạo bảng:

| Textbook | My Kernel | Change |

============================================================
19. WRITE PAGE DIRTY
============================================================

Trace:

userspace write
 ↓
page cache
 ↓
modify page
 ↓
set dirty
 ↓
writeback

Tìm:

set_page_dirty()
set_page_dirty_lock()
folio_mark_dirty()

nếu có.

Dùng đúng function trong source.

============================================================
20. WRITEBACK
============================================================

Chapter 16 liên kết trực tiếp Chapter 15.

Trace:

write()
 ↓
dirty page
 ↓
writeback
 ↓
filesystem ->writepage/writepages
 ↓
BIO
 ↓
block layer
 ↓
device

Giải thích:

tại sao write() có thể return trước disk I/O completion?

============================================================
21. MEMORY MAPPING – mmap()
============================================================

Trace:

mmap(addr, length, prot, flags, fd, offset)

từ:

userspace

→ syscall

→ do_mmap_pgoff / modern equivalent

→ VMA creation

→ file mapping

→ filesystem mmap

→ address_space

Không được chỉ giải thích:

mmap "map file vào memory."

Phải giải thích source.

============================================================
22. VM_AREA_STRUCT
============================================================

Phân tích:

struct vm_area_struct

Fields quan trọng:

vm_start
vm_end
vm_flags
vm_file
vm_pgoff
vm_ops

Giải thích:

VMA

khác:

page

khác:

address_space

như thế nào.

============================================================
23. FILE MAPPING DATA STRUCTURE
============================================================

Trace:

inode
 ↓
address_space
 ↓
mapping
 ↓
vm_area_struct
 ↓
vm_file
 ↓
page cache

Giải thích relationship.

============================================================
24. SHARED VS PRIVATE MMAP
============================================================

So sánh:

MAP_SHARED

vs

MAP_PRIVATE

Giải thích:

MAP_SHARED:
write memory
→ file/page cache dirty

MAP_PRIVATE:
write memory
→ Copy-on-Write

Nhưng phải trace source thật của tôi.

============================================================
25. DEMAND PAGING
============================================================

Cực kỳ quan trọng.

mmap()

KHÔNG nhất thiết đọc toàn bộ file vào RAM ngay.

Trace:

mmap()
 ↓
VMA created
 ↓
NO physical page yet
 ↓
CPU accesses address
 ↓
page fault
 ↓
ARM64 exception
 ↓
page fault handler
 ↓
filemap_fault()
 ↓
page cache lookup
 ↓
load page
 ↓
map page
 ↓
resume instruction

Phải trace source tree của tôi.

============================================================
26. ARM64 PAGE FAULT
============================================================

Nếu source ARM64:

CPU
 ↓
EL0 memory access
 ↓
Data Abort
 ↓
vector
 ↓
do_mem_abort()
 ↓
do_page_fault()
 ↓
handle_mm_fault()
 ↓
filemap_fault()
 ↓
page cache

Dùng đúng flow source.

Không được dùng flow khác version nếu source của tôi khác.

============================================================
27. FILEMAP_FAULT
============================================================

Tìm:

filemap_fault()

hoặc actual equivalent.

Giải thích:

VMA
+
address
+
mapping

→ tìm page

→ page cache

→ page fault resolution.

============================================================
28. DIRECT I/O
============================================================

Chapter nói:

O_DIRECT

Phân tích:

open(... O_DIRECT ...)

hoặc:

pread()/pwrite() với O_DIRECT.

Trace:

application
 ↓
VFS
 ↓
filesystem
 ↓
direct I/O
 ↓
BIO
 ↓
block layer
 ↓
device

Giải thích:

Page Cache bypass

thực tế có nghĩa gì?

Không nói tuyệt đối rằng "O_DIRECT hoàn toàn bypass mọi cache".

Phải dựa trên source.

============================================================
29. GENERIC_FILE_DIRECT_IO
============================================================

Tìm function tương ứng:

generic_file_direct_IO()

hoặc modern equivalent.

Giải thích:

kiocb
iov_iter
direct_IO
BIO
filesystem

============================================================
30. BUFFERED I/O VS DIRECT I/O
============================================================

Tạo comparison:

BUFFERED:

Userspace
 ↓
Page Cache
 ↓
BIO
 ↓
Disk

DIRECT:

Userspace
 ↓
filesystem direct I/O
 ↓
BIO
 ↓
Disk

So sánh:

Page Cache
copy
alignment
DMA
performance
consistency
filesystem requirements

============================================================
31. DIRECT I/O ALIGNMENT
============================================================

Tìm source kiểm tra:

alignment

length

offset

buffer address

block size

Nếu source có:

O_DIRECT alignment restrictions

hãy giải thích.

============================================================
32. ASYNCHRONOUS I/O
============================================================

Chapter mô tả:

aio_read()
aio_write()
aio_fsync()
aio_error()
aio_return()
aio_cancel()
aio_suspend()

và:

io_setup()
io_submit()
io_getevents()
io_cancel()
io_destroy()

Phân tích implementation thực tế trong source tree của tôi.

============================================================
33. AIO CONTEXT
============================================================

Trace:

io_setup()

→ AIO context

Phân tích:

struct kioctx

hoặc modern equivalent.

Giải thích:

AIO context

ring

events

completion queue

============================================================
34. IO_SUBMIT
============================================================

Trace:

io_submit()

→ syscall

→ aio context

→ IOCB validation

→ operation dispatch

→ kiocb

→ read/write

→ filesystem

→ block layer

============================================================
35. COMPLETION
============================================================

Trace:

AIO operation
 ↓
I/O submitted
 ↓
process continues
 ↓
I/O completes
 ↓
completion event
 ↓
io_getevents()
 ↓
userspace

Giải thích callback/event mechanism bằng source code.

============================================================
36. MODERN ASYNC I/O NOTE
============================================================

Nếu source kernel của tôi chứa:

AIO

hãy phân tích nó.

Không tự thay bằng io_uring.

Chỉ thêm:

"Modern alternative: io_uring"

nếu tôi yêu cầu hoặc nếu cần để định hướng.

============================================================
37. FILE OPERATIONS
============================================================

Phân tích:

struct file_operations

Đặc biệt:

read
read_iter
write
write_iter
mmap
fsync
unlocked_ioctl

Giải thích:

VFS

→ file_operations

→ filesystem

Tạo diagram:

struct file
    |
    v
f_op
    |
    +── read_iter
    +── write_iter
    +── mmap
    +── fsync

============================================================
38. ITERATORS
============================================================

Nếu source của tôi sử dụng:

struct iov_iter

hãy giải thích rất kỹ.

Phân tích:

ITER_UBUF
ITER_IOVEC
ITER_KVEC
ITER_BVEC
ITER_XARRAY

nếu tồn tại.

Giải thích:

Userspace buffer
vs
kernel buffer
vs
BIO vector

============================================================
39. COPY TO/FROM USER
============================================================

Trace:

read:

kernel page
 ↓
copy_to_user()

write:

copy_from_user()

Nếu ARM64:

giải thích implementation tương ứng.

============================================================
40. LOCKING
============================================================

Với tất cả flow:

read
write
mmap
page fault
writeback
direct I/O
AIO

phải chỉ rõ:

mutex
spinlock
rwsem
page lock
mapping lock
XArray lock
inode lock
RCU

Context:

process
softirq
hardirq
workqueue

Can sleep:

YES/NO

============================================================
41. LIFETIME
============================================================

Trace lifetime:

fd
 ↓
struct file
 ↓
inode
 ↓
address_space
 ↓
page
 ↓
bio
 ↓
request

Giải thích reference counting.

============================================================
42. ERROR PATH
============================================================

Đối với mỗi flow phải phân tích:

- invalid fd
- invalid user buffer
- permission error
- page cache miss
- allocation failure
- filesystem error
- I/O error
- EFAULT
- EIO
- EINTR
- EINVAL
- O_DIRECT alignment failure

Chỉ đưa errno thực sự xuất hiện trong source.

============================================================
43. PERFORMANCE
============================================================

Phân tích:

Buffered I/O
Direct I/O
mmap
read-ahead
page cache
writeback
AIO

So sánh:

latency
throughput
CPU overhead
memory usage
copy overhead

============================================================
44. CHAPTER 14 + 15 + 16 CONNECTION
============================================================

Tạo mental model:

Chapter 16
ACCESSING FILES
        │
        ▼
       VFS
        │
        ▼
Chapter 15
PAGE CACHE
        │
        ▼
      BIO
        │
        ▼
Chapter 14
BLOCK DEVICE
        │
        ▼
    BLK-MQ
        │
        ▼
     DRIVER
        │
        ▼
    HARDWARE

Đây phải là một section riêng.

============================================================
45. COMPLETE READ FLOW
============================================================

Tạo một call graph thực tế từ source của tôi:

read()
 ↓
syscall
 ↓
VFS
 ↓
filesystem
 ↓
page cache
 ↓
XArray
 ↓
page
 ↓
read-ahead
 ↓
filesystem
 ↓
BIO
 ↓
block layer
 ↓
driver
 ↓
DMA
 ↓
IRQ
 ↓
completion
 ↓
page
 ↓
copy_to_user()
 ↓
userspace

Mỗi arrow phải có function thật.

============================================================
46. COMPLETE WRITE FLOW
============================================================

write()
 ↓
syscall
 ↓
VFS
 ↓
filesystem
 ↓
page cache
 ↓
copy_from_user()
 ↓
dirty page
 ↓
writeback
 ↓
filesystem
 ↓
BIO
 ↓
block layer
 ↓
driver
 ↓
DMA
 ↓
IRQ
 ↓
completion

============================================================
47. COMPLETE MMAP FLOW
============================================================

mmap()
 ↓
syscall
 ↓
do_mmap()
 ↓
VMA
 ↓
return userspace

Sau đó:

CPU memory access
 ↓
page fault
 ↓
ARM64 exception
 ↓
page fault handler
 ↓
filemap_fault()
 ↓
Page Cache
 ↓
filesystem
 ↓
BIO
 ↓
device
 ↓
page mapped
 ↓
resume userspace

============================================================
48. COMPLETE DIRECT I/O FLOW
============================================================

read(O_DIRECT)
 ↓
VFS
 ↓
filesystem
 ↓
direct I/O
 ↓
iov_iter
 ↓
BIO
 ↓
block layer
 ↓
driver
 ↓
DMA
 ↓
device

============================================================
49. SOURCE CODE WALKTHROUGH
============================================================

Với mỗi function quan trọng:

=====================================
FUNCTION
=====================================

Name:
...

Source:
...

Line:
...

Why:
...

Called by:
...

Calls:
...

Arguments:
...

Return:
...

Data structures:
...

Locks:
...

Context:
...

Can sleep:
...

Error paths:
...

Legacy:
...

Modern:
...

=====================================

Không paste toàn bộ function.

Chỉ đưa đoạn code cần thiết.

Sau code:

giải thích từng block.

============================================================
50. SOURCE SEARCH COMMANDS
============================================================

Khi cần tìm source, sử dụng logic:

rg
git grep
grep
find

Ví dụ:

rg "sys_read|__arm64_sys_read" arch/ fs/
rg "file_operations" include/ fs/
rg "read_iter" fs/ mm/
rg "mmap" mm/ fs/
rg "O_DIRECT" fs/ include/
rg "kiocb" fs/ include/
rg "iov_iter" include/ lib/ fs/
rg "filemap_fault" mm/ fs/

Nhưng source path phải lấy từ cây source thực tế của tôi.

============================================================
51. SOURCE VERIFICATION
============================================================

Mỗi claim về source phải có:

Source file
Function
Line nếu có thể

Ví dụ:

mm/filemap.c
function:
generic_file_buffered_read()

Không nói:

"Kernel làm X"

mà không chỉ ra source.

============================================================
52. PRACTICAL LABS
============================================================

Thiết kế lab dựa trên source kernel của tôi.

LAB 1
Trace read().

LAB 2
Page Cache HIT/MISS.

LAB 3
Sequential read + readahead.

LAB 4
Random read.

LAB 5
write() + dirty pages.

LAB 6
writeback.

LAB 7
mmap() + page fault.

LAB 8
MAP_SHARED vs MAP_PRIVATE.

LAB 9
O_DIRECT.

LAB 10
AIO.

LAB 11
Trace read/write xuống BIO.

LAB 12
ARM64 syscall/page fault.

Mỗi lab:

Goal
Setup
Command
Expected output
Source functions
Tracepoints
What to inspect

============================================================
53. DEBUGGING
============================================================

Nếu phù hợp với source của tôi:

ftrace
trace-cmd
perf
bpftrace
kgdb
gdb
dynamic_debug

Đưa cụ thể:

function tracing
tracepoint
kprobe

để tôi có thể quan sát:

read()
→ Page Cache
→ BIO
→ Block layer

============================================================
54. FINAL KNOWLEDGE CHECK
============================================================

Cuối cùng tạo câu hỏi kiểm tra:

1. read() bắt đầu ở đâu?
2. fd chuyển thành struct file như thế nào?
3. file->f_op hoạt động thế nào?
4. read_iter() có vai trò gì?
5. Page Cache được lookup bằng gì?
6. read cache hit khác miss thế nào?
7. read-ahead hoạt động thế nào?
8. write() có ghi disk ngay không?
9. dirty page được flush khi nào?
10. mmap() có đọc file ngay không?
11. page fault nối vào Page Cache thế nào?
12. MAP_SHARED khác MAP_PRIVATE thế nào?
13. O_DIRECT bypass gì?
14. AIO hoạt động thế nào?
15. iov_iter là gì?
16. BIO xuất hiện ở đâu?
17. Block layer kết nối với Chapter 14 như thế nào?
18. Page Cache kết nối với Chapter 15 như thế nào?
19. ARM64 syscall entry nằm ở đâu?
20. ARM64 page fault flow nằm ở đâu?

============================================================
55. FINAL OUTPUT
============================================================

Kết quả cuối cùng phải có:

A. Chapter map
B. Source tree map
C. Data structure map
D. READ call graph
E. WRITE call graph
F. mmap call graph
G. Direct I/O graph
H. AIO graph
I. Page Cache relationship
J. Block Layer relationship
K. ARM64 syscall flow
L. ARM64 page fault flow
M. Function walkthrough
N. Locking analysis
O. Error paths
P. Debugging guide
Q. Practical labs
R. Knowledge checklist

============================================================
56. ABSOLUTE RULE
============================================================

SOURCE CODE CỦA TÔI > TEXTBOOK > MODEL KNOWLEDGE

Nếu textbook và source khác nhau:

KHÔNG sửa source để giống textbook.

Hãy giải thích:

Textbook says:
...

My source does:
...

Difference:
...

Reason:
...

Nếu không biết reason:

REASON NOT VERIFIED

Không được đoán.

Mục tiêu cuối cùng là:

Tôi có thể mở source code của chính mình,

search một function,

và biết chính xác:

Nó được gọi từ đâu?
Nó gọi gì?
Nó sửa struct nào?
Nó chạy trong context nào?
Lock nào bảo vệ?
Nó tạo I/O ở đâu?
I/O đi xuống Page Cache/BIO/Block Layer thế nào?
Và kết quả quay trở lại userspace ra sao?