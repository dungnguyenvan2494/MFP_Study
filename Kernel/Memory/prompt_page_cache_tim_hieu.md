Tôi đang nghiên cứu Chapter 15 – "The Page Cache" của tài liệu Linux Kernel được cung cấp.

Hãy đóng vai:

Senior Linux Kernel Engineer
+
Linux MM/VFS/Filesystem Engineer
+
Kernel Source Code Instructor

Mục tiêu của tôi KHÔNG phải chỉ hiểu nội dung textbook.

Tôi muốn hiểu Chapter 15 bằng cách đọc trực tiếp Linux Kernel source code.

Hãy lấy Chapter 15 làm syllabus chính và mapping từng concept trong chapter sang Linux Kernel source code thực tế.


Kết quả tìm hiểu phải là tiếng việt.

============================================================
0. KERNEL VERSION
============================================================

Ưu tiên:

Source Linux Kernel  trong C:\Users\PC\Documents\IT6_Kernel\Kernel\K-S800\Srcpath

============================================================
1. CHAPTER MAP
============================================================

Đầu tiên hãy đọc toàn bộ Chapter 15.

Tạo study map:

Chapter 15
│
├── Page Cache
│
├── address_space
│
├── radix tree
│
├── Page Cache Handling Functions
│   ├── Finding a page
│   ├── Adding a page
│   ├── Removing a page
│   └── Updating a page
│
├── Radix Tree Tags
│
├── Buffer Cache / Buffer Heads
│   ├── buffer_head
│   ├── buffer pages
│   ├── allocating buffer pages
│   ├── releasing buffer pages
│   └── searching blocks
│
├── Submitting Buffer Heads
│   ├── submit_bh()
│   └── ll_rw_block()
│
├── Dirty Page Writeback
│
├── pdflush
│
├── Looking for Dirty Pages
│
├── Retrieving Old Dirty Pages
│
└── sync()
    ├── fsync()
    └── fdatasync()

Sau đó mapping từng phần sang Linux 5.4.

============================================================
2. PAGE CACHE – CORE CONCEPT
============================================================

Giải thích:

Page Cache là gì?

Tại sao Linux cần Page Cache?

Page Cache nằm giữa:

Userspace
    ↓
VFS
    ↓
Filesystem
    ↓
Page Cache
    ↓
Block Layer
    ↓
Block Device

Giải thích:

read()

lần đầu:

userspace
    ↓
VFS
    ↓
filesystem
    ↓
page cache miss
    ↓
disk I/O
    ↓
page cache
    ↓
userspace

Lần thứ hai:

userspace
    ↓
VFS
    ↓
filesystem
    ↓
page cache HIT
    ↓
userspace

Phân tích source code thực tế.

============================================================
3. PAGE CACHE KEY
============================================================

Giải thích kernel xác định một page thuộc file nào bằng cách nào.

Phân tích:

inode
address_space
page index

Quan hệ:

inode
  │
  ▼
address_space
  │
  ▼
page index
  │
  ▼
cached page

Giải thích:

mapping + index

và tại sao:

page index

quan trọng.

============================================================
4. STRUCT ADDRESS_SPACE
============================================================

Tìm Linux 5.4:

struct address_space

Source:

include/linux/fs.h

hoặc source location chính xác trong Linux 5.4.

Giải thích các field quan trọng.

Đặc biệt:

host
i_pages
tree_lock / equivalent
nrpages
a_ops
flags
backing_dev_info
private_list
private_lock
assoc_mapping

Nếu field trong sách khác Linux 5.4:

BOOK
vs
LINUX 5.4

Tạo table:

| Field | Book | Linux 5.4 | Purpose |

============================================================
5. ADDRESS_SPACE_OPERATIONS
============================================================

Phân tích:

struct address_space_operations

Tập trung vào các operation liên quan Page Cache:

readpage
readpages
writepage
writepages
set_page_dirty
releasepage
invalidatepage
write_begin
write_end
direct_IO
bmap

Nhưng phải sử dụng đúng API Linux 5.4.

Giải thích:

filesystem cung cấp callback như thế nào?

VFS/Page Cache gọi callback như thế nào?

Ví dụ:

Page Cache
    ↓
a_ops
    ↓
filesystem implementation
    ↓
ext4

Trace thực tế với ext4.

============================================================
6. PAGE CACHE READ FLOW
============================================================

Trace:

read(fd, buffer, size)

cho regular file.

Flow:

userspace
 ↓
syscall
 ↓
VFS
 ↓
filesystem
 ↓
page cache lookup
 ↓
HIT / MISS

Nếu MISS:

 ↓
filesystem read operation
 ↓
page allocation
 ↓
I/O submission
 ↓
block layer
 ↓
storage
 ↓
completion
 ↓
page becomes uptodate
 ↓
userspace

Nếu HIT:

 ↓
copy/read from page cache

Tìm source code thực tế Linux 5.4.

Mỗi function phải có:

Function
Source
Caller
Callee
Input
Output
Data structure
Context
Locking

============================================================
7. PAGE CACHE LOOKUP
============================================================

Chapter sử dụng:

find_get_page()
find_get_pages()
find_get_page_tag()

Hãy mapping sang Linux 5.4.

Đặc biệt nghiên cứu:

find_get_page()
find_get_pages()
find_get_entries()
find_get_pages_contig()

hoặc equivalent thực tế trong version.

Giải thích:

Page Cache lookup

dựa trên:

mapping
+
index

Trace:

mapping
 ↓
i_pages
 ↓
XArray
 ↓
page

============================================================
8. RADIX TREE → XARRAY
============================================================

Đây là phần CỰC KỲ QUAN TRỌNG.

Chapter sử dụng:

struct radix_tree_root
struct radix_tree_node

Linux 5.4 sử dụng:

struct xarray
struct xa_node

Giải thích:

Tại sao page cache chuyển từ radix tree sang XArray?

So sánh:

RADIX TREE

vs

XARRAY

Tạo table:

| Concept | Legacy | Linux 5.4 |

Bao gồm:

root
node
slot
index
lookup
insert
delete
tag
locking
concurrency

Trace source code:

XArray
    ↓
xas_load()
    ↓
xarray node
    ↓
page

hoặc dùng đúng function thực tế Linux 5.4.

Không được tự bịa function.

============================================================
9. PAGE CACHE ADD
============================================================

Chapter mô tả:

add_to_page_cache()

add_to_page_cache_locked()

Hãy tìm Linux 5.4 equivalent.

Giải thích:

page được thêm vào Page Cache như thế nào?

Flow:

page
 ↓
mapping
 ↓
index
 ↓
XArray
 ↓
cache

Phân tích:

locking
reference counting
LRU
page flags

============================================================
10. PAGE CACHE REMOVE
============================================================

Chapter:

remove_from_page_cache()

Phân tích Linux 5.4.

Flow:

page
 ↓
XArray erase
 ↓
mapping cleared
 ↓
page cache reference
 ↓
LRU/free/reuse

Giải thích lifetime.

============================================================
11. PAGE CACHE UPDATE
============================================================

Chapter:

read_cache_page()

Tìm modern equivalent.

Giải thích:

Page Cache update có nghĩa gì?

Flow:

lookup
 ↓
if absent:
    allocate page
 ↓
insert
 ↓
filesystem read
 ↓
PageUptodate
 ↓
return

Giải thích:

PG_locked
PG_uptodate
PG_dirty
PG_writeback

============================================================
12. PAGE FLAGS
============================================================

Phân tích các flags:

PG_locked
PG_uptodate
PG_dirty
PG_writeback
PG_readahead
PG_lru
PG_private

Nếu Linux 5.4 khác:

giải thích chính xác.

Tạo state machine:

PAGE

NEW
 ↓
ALLOCATED
 ↓
LOCKED
 ↓
READING
 ↓
UPTODATE
 ↓
DIRTY
 ↓
WRITEBACK
 ↓
CLEAN
 ↓
RECLAIMED

Chỉ sử dụng state thực sự tồn tại.

============================================================
13. RADIX TREE TAGS → XARRAY MARKS
============================================================

Chapter mô tả:

PAGECACHE_TAG_DIRTY
PAGECACHE_TAG_WRITEBACK

Giải thích Linux 5.4 equivalent.

Tại sao kernel cần tag/mark?

Ví dụ:

Page 0 CLEAN
Page 1 DIRTY
Page 2 CLEAN
Page 3 DIRTY
Page 4 CLEAN

Thay vì scan toàn bộ:

XArray
   ↓
MARK_DIRTY
   ↓
find dirty pages

Giải thích:

xa_mark()
xas_mark()
xas_find_marked()

hoặc function thực tế của Linux 5.4.

============================================================
14. BUFFER CACHE
============================================================

Chapter chuyển sang:

Block Buffers
Buffer Heads
Buffer Pages

Giải thích lịch sử:

Page Cache
+
Buffer Cache

và tại sao Linux hợp nhất/đan xen các cơ chế này.

============================================================
15. STRUCT BUFFER_HEAD
============================================================

Phân tích:

struct buffer_head

Linux 5.4.

Giải thích các field quan trọng:

b_state
b_this_page
b_page
b_count
b_size
b_blocknr
b_data
b_bdev
b_end_io
b_private
b_assoc_buffers

Tạo diagram:

page
│
├── buffer_head
│
├── buffer_head
│
├── buffer_head
│
└── buffer_head

Và:

buffer_head
    ↓
block number
    ↓
block device

============================================================
16. BUFFER HEAD FLAGS
============================================================

Giải thích:

BH_Uptodate
BH_Dirty
BH_Lock
BH_Req
BH_Mapped
BH_New
BH_Async_Read
BH_Async_Write
BH_Delay
BH_Boundary
BH_Write_EIO
BH_Ordered
BH_Eopnotsupp

Nhưng:

CHỈ sử dụng những flag thực sự tồn tại trong Linux 5.4.

Tạo table:

| Flag | Meaning | Set by | Cleared by | Used by |

============================================================
17. BUFFER PAGE
============================================================

Giải thích:

buffer page

và quan hệ:

Page
 ↓
multiple buffer_head
 ↓
multiple filesystem blocks

Ví dụ:

PAGE_SIZE = 4096
block size = 1024

Một page có thể chứa:

4 buffer heads.

Vẽ diagram.

============================================================
18. BUFFER PAGE ALLOCATION
============================================================

Trace logic:

__getblk()
__bread()
bread()
sb_getblk()
sb_bread()

hoặc modern equivalents.

Giải thích:

block number

→ page offset

→ buffer head

→ block device

Trace source.

============================================================
19. SEARCHING BLOCKS IN PAGE CACHE
============================================================

Chapter có:

__find_get_block()

__getblk()

__bread()

Hãy mapping Linux 5.4.

Giải thích:

block device
+
block number
+
block size

→ locate buffer

Flow:

bdev
 ↓
block number
 ↓
buffer cache
 ↓
buffer_head
 ↓
page

============================================================
20. SUBMITTING BUFFER HEADS
============================================================

Chapter:

submit_bh()

ll_rw_block()

Hãy tìm Linux 5.4 implementation/equivalent.

Trace:

buffer_head
 ↓
BIO
 ↓
block layer
 ↓
request
 ↓
blk-mq
 ↓
driver

Đây là cầu nối quan trọng giữa:

BUFFER CACHE

và

BLOCK LAYER.

Tạo diagram:

buffer_head
      ↓
     BIO
      ↓
 request
      ↓
   blk-mq
      ↓
 block driver

============================================================
21. DIRTY PAGE
============================================================

Giải thích:

dirty page là gì?

Khi nào page trở thành dirty?

Ai set:

PG_dirty

Filesystem có vai trò gì?

Phân tích:

set_page_dirty()
set_page_dirty_lock()

hoặc Linux 5.4 equivalent.

Flow:

write()
 ↓
page cache
 ↓
modify page
 ↓
mark dirty
 ↓
writeback later

============================================================
22. DIRTY PAGE WRITEBACK
============================================================

Đây là phần quan trọng nhất của chapter.

Giải thích:

dirty page không được ghi xuống disk ngay lập tức.

Kernel trì hoãn writeback.

Tại sao?

- performance
- write combining
- sequential I/O
- reduce disk operations
- batching

Flow:

write()
 ↓
Page Cache
 ↓
PG_dirty
 ↓
dirty page accumulation
 ↓
writeback trigger
 ↓
filesystem writepages()
 ↓
BIO
 ↓
Block Layer
 ↓
Disk

============================================================
23. MODERN WRITEBACK
============================================================

Chapter dùng:

pdflush

Nhưng Linux 5.4 không dùng mô hình pdflush cũ.

Phải giải thích:

OLD:

pdflush
 ↓
global dirty page scanning

MODERN:

writeback infrastructure
 ↓
backing device
 ↓
bdi_writeback
 ↓
writeback work
 ↓
filesystem
 ↓
block layer

Tìm source Linux 5.4.

Đặc biệt nghiên cứu:

struct backing_dev_info

struct bdi_writeback

struct wb_writeback_work

writeback_control

wb_workfn()

writeback_sb_inodes()

hoặc equivalent thực tế.

Không được bịa.

============================================================
24. WRITEBACK_CONTROL
============================================================

Phân tích:

struct writeback_control

Các field quan trọng.

Ví dụ:

nr_to_write
sync_mode
range_start
range_end
for_kupdate
for_background
for_reclaim

Giải thích từng field.

Trace:

writeback worker
 ↓
writeback_control
 ↓
filesystem
 ↓
writepages
 ↓
bio

============================================================
25. WRITEBACK TRIGGER
============================================================

Phân tích các trường hợp:

1. Dirty pages vượt threshold
2. Periodic writeback
3. Memory pressure
4. Explicit sync
5. fsync()
6. fdatasync()
7. sync()

Giải thích từng trigger.

============================================================
26. DIRTY PAGE THRESHOLDS
============================================================

Giải thích:

dirty_background_ratio
dirty_ratio

hoặc modern equivalents.

Trace:

dirty pages
 ↓
global/per-bdi accounting
 ↓
threshold
 ↓
background writeback
 ↓
writeback thread

Giải thích:

background writeback

vs

process writeback/throttling.

============================================================
27. RETRIEVING DIRTY PAGES
============================================================

Chapter mô tả việc tìm dirty pages trong radix tree.

Linux 5.4 dùng XArray/pagevec/list/writeback structures.

Giải thích:

làm thế nào kernel tìm dirty pages hiệu quả?

Không được nói đơn giản:

"Kernel scan tất cả pages."

Phải giải thích:

mark
+
lists
+
writeback structures

và source code.

============================================================
28. LRU + PAGE CACHE
============================================================

Giải thích quan hệ:

Page Cache
+
LRU

Phân tích:

active list
inactive list

và:

add_to_page_cache_lru()

pagevec
LRU isolation
reclaim

Giải thích:

Page Cache giữ page

nhưng VM subsystem quyết định khi nào page có thể reclaim.

============================================================
29. PAGE RECLAIM
============================================================

Trace:

memory pressure
 ↓
kswapd
 ↓
LRU scan
 ↓
page cache
 ↓
clean page
 ↓
reclaim

Nếu page dirty:

dirty
 ↓
writeback
 ↓
clean
 ↓
reclaim

Giải thích source code Linux 5.4.

============================================================
30. SYNC()
============================================================

Trace:

userspace:

sync()

↓

ARM64 syscall

↓

__arm64_sys_sync()

↓

sync_filesystems()

↓

writeback

↓

filesystem

↓

block layer

↓

disk

Tìm source chính xác Linux 5.4.

============================================================
31. FSYNC()
============================================================

Trace:

fsync(fd)

↓

VFS

↓

file->f_op->fsync()

↓

filesystem

↓

write dirty pages

↓

write metadata

↓

flush device nếu cần

Giải thích sự khác biệt:

sync()

vs

fsync()

============================================================
32. FDATASYNC()
============================================================

Giải thích:

fdatasync()

vs

fsync()

Tập trung vào:

data

vs

metadata.

Trace source Linux 5.4.

============================================================
33. PAGE CACHE + EXT4
============================================================

Dùng ext4 làm concrete example.

Trace:

application
 ↓
VFS
 ↓
ext4
 ↓
address_space
 ↓
page cache
 ↓
ext4 write/read
 ↓
BIO
 ↓
block layer
 ↓
device

Xác định:

ext4 file_operations

ext4 address_space_operations

Các callback quan trọng.

============================================================
34. READ VS WRITE
============================================================

Tạo bảng:

| READ | WRITE |

So sánh:

page cache lookup
page allocation
PageUptodate
PG_dirty
writeback
BIO
completion

============================================================
35. PAGE CACHE LIFECYCLE
============================================================

Tạo lifecycle:

ALLOCATE
 ↓
ADD TO CACHE
 ↓
UPTODATE
 ↓
ACCESS
 ↓
DIRTY
 ↓
WRITEBACK
 ↓
CLEAN
 ↓
RECLAIM
 ↓
FREE

Mỗi transition phải có:

function
source
condition

============================================================
36. OBJECT RELATIONSHIP
============================================================

Tạo diagram:

inode
  |
  v
address_space
  |
  v
i_pages / XArray
  |
  v
page
  |
  +── buffer_head
  |
  +── mapping
  |
  +── index
  |
  +── flags

Sau đó:

page
 ↓
BIO
 ↓
request
 ↓
blk-mq
 ↓
block driver

============================================================
37. LOCKING
============================================================

Đây là phần bắt buộc.

Phân tích locking:

page lock
XArray lock
mapping locks
buffer_head lock
inode lock
writeback locks
LRU locks

Với mỗi function:

Lock:
...

Lock acquired:
...

Lock released:
...

Can sleep:
YES/NO

Context:
process/softirq/hardirq/workqueue

============================================================
38. REFERENCE COUNTING
============================================================

Giải thích lifecycle/reference:

page reference
buffer_head reference
mapping reference
inode reference

Các function:

get_page()
put_page()

get_bh()
brelse()

và modern equivalents.

Giải thích tại sao page không được free ngay khi không còn được lookup.

============================================================
39. MEMORY ORDERING / RACE
============================================================

Nếu source code có:

atomic
memory barrier
RCU
lockless XArray lookup

hãy giải thích.

Đặc biệt:

PageUptodate

và page lock.

Giải thích race:

CPU0:
read page

CPU1:
invalidate page

CPU2:
writeback page

Kernel đảm bảo consistency như thế nào?

============================================================
40. SOURCE TREE
============================================================

Tạo source map Linux 5.4.

Tìm các file liên quan:

mm/filemap.c
mm/page-writeback.c
mm/readahead.c
mm/swap.c
mm/truncate.c

include/linux/mm.h
include/linux/fs.h
include/linux/pagemap.h
include/linux/writeback.h
include/linux/xarray.h

fs/ext4/

fs/buffer.c

block/

Nhưng phải kiểm tra source thực tế.

Không được tự bịa path/function.

============================================================
41. IMPORTANT FUNCTIONS
============================================================

Tạo bảng:

Function
Source
Purpose
Caller
Callee
Data Structure
Context

Tập trung vào:

find_get_page()
find_get_pages()
add_to_page_cache_lru()
delete_from_page_cache()
read_cache_page()
set_page_dirty()
writeback
write_cache_pages()
mapping_writepages()
__filemap_fdatawrite_range()
filemap_write_and_wait_range()
sync_filesystems()
fsync
submit_bh()
ll_rw_block()

Nếu function không tồn tại Linux 5.4:

OLD API
→
MODERN EQUIVALENT

============================================================
42. CALL GRAPH
============================================================

Tạo call graph:

READ CACHE HIT

read()
 ↓
VFS
 ↓
filesystem
 ↓
page cache lookup
 ↓
XArray
 ↓
page
 ↓
copy data
 ↓
userspace

READ CACHE MISS

read()
 ↓
filesystem
 ↓
page cache lookup
 ↓
MISS
 ↓
allocate page
 ↓
add to cache
 ↓
filesystem read
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
userspace

WRITE

write()
 ↓
page cache
 ↓
modify page
 ↓
dirty
 ↓
writeback
 ↓
filesystem
 ↓
BIO
 ↓
block layer
 ↓
device

============================================================
43. DEBUGGING / OBSERVABILITY
============================================================

Thiết kế lab để quan sát Page Cache.

Tools:

/proc/meminfo

/sys/kernel/mm/

vmstat

/proc/vmstat

ftrace

tracepoints

perf

bpftrace nếu phù hợp

Các event liên quan:

writeback
mm_filemap
mm_page_alloc
mm_page_free
block

Nếu tracepoint không tồn tại trong Linux 5.4:

ghi rõ.

============================================================
44. PRACTICAL LABS
============================================================

LAB 1:

Page Cache HIT/MISS

Tạo file lớn.

Đọc lần 1.

Đọc lần 2.

Quan sát cache.

LAB 2:

drop_caches

Quan sát sự khác biệt.

LAB 3:

Dirty Page

write file lớn.

Quan sát:

Dirty
Writeback

trong /proc/meminfo.

LAB 4:

fsync()

So sánh:

write()

write()+fsync()

LAB 5:

fdatasync()

So sánh với fsync().

LAB 6:

Trace ext4 writeback.

LAB 7:

Trace BIO generated from page cache.

LAB 8:

Trace block I/O completion.

Mỗi LAB:

Goal
Command
Expected result
Kernel source
Functions
What to observe

============================================================
45. FINAL LEGACY VS MODERN TABLE
============================================================

Tạo bảng:

| Chapter 15 | Linux 5.4 | Status |

Bao gồm:

Page Cache
address_space
radix tree
XArray
radix tree tags
XArray marks
buffer_head
buffer cache
submit_bh
ll_rw_block
pdflush
writeback
bdi
bdi_writeback
writeback_control
sync
fsync
fdatasync

Phân loại:

UNCHANGED
CHANGED
REPLACED
REMOVED
REFACTORED

============================================================
46. SOURCE CODE READING RULE
============================================================

Mỗi khi giải thích function:

-------------------------------------
FUNCTION
-------------------------------------

Function:
...

Source:
...

Kernel version:
Linux 5.4.x

Purpose:
...

Called by:
...

Calls:
...

Input:
...

Output:
...

Data structures:
...

Locking:
...

Context:
...

Can sleep:
...

Important flags:
...

Legacy equivalent:
...

Modern implementation:
...

-------------------------------------

Không paste quá nhiều source code.

Chỉ đưa đoạn source code quan trọng.

Sau code:

giải thích từng đoạn.

============================================================
47. SOURCE CODE VERIFICATION
============================================================

CỰC KỲ QUAN TRỌNG:

Không được hallucinate.

Không được tự bịa:

function
struct
field
source path
call graph

Nếu chưa xác minh:

UNVERIFIED

Nếu function thuộc Linux cũ:

LEGACY

Nếu function đã bị remove:

REMOVED

Nếu function được thay thế:

REPLACED BY ...

============================================================
48. FINAL MASTER MODEL
============================================================

Cuối cùng phải giúp tôi xây dựng mental model:

                    USERSPACE
                        │
                        ▼
                     read()
                        │
                        ▼
                       VFS
                        │
                        ▼
                    FILESYSTEM
                        │
                        ▼
                   address_space
                        │
                        ▼
                   Page Cache
                        │
                 ┌──────┴──────┐
                 │             │
               HIT           MISS
                 │             │
                 │             ▼
                 │        filesystem
                 │             │
                 │             ▼
                 │            BIO
                 │             │
                 │             ▼
                 │        Block Layer
                 │             │
                 │             ▼
                 │           Disk
                 │             │
                 │             ▼
                 │        completion
                 │             │
                 └───────► Page
                              │
                              ▼
                           Userspace


WRITE:

Userspace
   ↓
write()
   ↓
Page Cache
   ↓
modify page
   ↓
PG_dirty
   ↓
writeback
   ↓
filesystem
   ↓
BIO
   ↓
Block Layer
   ↓
Disk


SYNC:

sync()
   ↓
writeback
   ↓
dirty pages
   ↓
filesystem
   ↓
block layer
   ↓
device


FSYNC:

fsync(fd)
   ↓
file-specific writeback
   ↓
filesystem
   ↓
metadata/data synchronization
   ↓
device flush if required

============================================================
49. FINAL GOAL
============================================================

Sau khi nghiên cứu xong, tôi phải có khả năng trả lời:

1. Page Cache là gì?
2. Một page được xác định trong Page Cache bằng gì?
3. address_space làm nhiệm vụ gì?
4. i_pages là gì?
5. Radix Tree trong textbook tương ứng với gì trong Linux 5.4?
6. XArray hoạt động như thế nào?
7. Page được add vào Page Cache như thế nào?
8. Page bị remove như thế nào?
9. PageUptodate nghĩa là gì?
10. PG_dirty được set như thế nào?
11. Dirty page được tìm như thế nào?
12. Writeback được trigger như thế nào?
13. pdflush trong sách tương ứng với kiến trúc nào hiện đại?
14. buffer_head là gì?
15. Một page có thể chứa nhiều buffer_head như thế nào?
16. buffer_head liên kết với block device như thế nào?
17. submit_bh() tạo BIO như thế nào?
18. BIO đi xuống block layer như thế nào?
19. write() có thực sự ghi disk ngay không?
20. fsync() khác sync() như thế nào?
21. fdatasync() khác fsync() như thế nào?
22. Khi nào Page Cache page được reclaim?
23. LRU liên quan Page Cache như thế nào?
24. ext4 tương tác với Page Cache như thế nào?
25. Cuối cùng làm sao:

read()
→ Page Cache
→ BIO
→ Block Layer
→ Disk

và:

write()
→ Page Cache
→ Dirty
→ Writeback
→ BIO
→ Block Layer
→ Disk

Hãy coi đây là mục tiêu cuối cùng của toàn bộ Chapter 15.