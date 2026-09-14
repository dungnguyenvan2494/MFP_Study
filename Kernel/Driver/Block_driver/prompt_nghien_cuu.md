Tôi đang nghiên cứu Chapter 14 – “Block Device Drivers” của tài liệu Linux Kernel.

Tôi muốn bạn đóng vai một Senior Linux Kernel Engineer, chuyên về Block Layer, Storage Driver, VFS, DMA, Interrupt và Device Driver Model.

Mục tiêu của tôi KHÔNG phải chỉ hiểu lý thuyết trong sách, mà phải hiểu toàn bộ chapter này ở mức SOURCE CODE Linux Kernel.

Hãy lấy nội dung của tài liệu tôi cung cấp làm syllabus chính, sau đó mapping từng concept trong tài liệu sang Linux Kernel source code thực tế.

==================================================
I. KERNEL VERSION
==================================================

Ưu tiên phân tích trên:

Source Linux Kernel ở thư mục C:\Users\PC\Documents\IT6_Kernel\Kernel\K-S800\Src

Ví dụ:

generic_make_request()
→ submit_bio()
→ blk_mq_submit_bio()

hoặc nếu một cơ chế cũ không còn tồn tại thì phải ghi rõ:

REMOVED / REPLACED / REFACTORED

Không được cố gán function cũ vào source mới nếu chúng không còn tương đương.

==================================================
II. MỤC TIÊU NGHIÊN CỨU
==================================================

Sau khi đọc toàn bộ chapter, hãy xây dựng một study map:

Chapter 14
│
├── Block Device Handling
│
├── Sector
│
├── Block
│
├── Segment
│
├── Generic Block Layer
│   ├── bio
│   ├── bio_vec
│   ├── gendisk
│   ├── block_device
│   └── partition
│
├── Submitting a Request
│
├── Request Queue
│   ├── request_queue
│   └── request
│
├── I/O Scheduler
│
├── Block Device Driver
│
├── Driver Registration
│
├── Strategy Routine
│
├── Interrupt Handler
│
└── Opening Block Device File

Sau đó nghiên cứu từng node theo source code.

==================================================
III. KIẾN TRÚC TỔNG THỂ BLOCK I/O
==================================================

Trước tiên hãy giải thích kiến trúc tổng thể:

Userspace
    |
    | read()/write()/pread()/pwrite()
    v
System Call
    |
    v
VFS
    |
    v
Filesystem
    |
    v
Page Cache
    |
    v
Filesystem mapping
    |
    v
BIO
    |
    v
Generic Block Layer
    |
    v
Request Queue
    |
    v
I/O Scheduler
    |
    v
Block Driver
    |
    v
DMA
    |
    v
Disk Controller
    |
    v
Storage Device

Sau đó tìm source code của từng layer.

Với mỗi transition:

A → B

hãy giải thích:

- function nào thực hiện transition
- source file
- data structure truyền qua
- synchronous hay asynchronous
- process context hay interrupt context
- có lock không
- có sleep được không
- ownership của object thuộc về ai

==================================================
IV. TRACE MỘT READ() HOÀN CHỈNH
==================================================

Hãy trace chi tiết:

read(fd, buf, 4096)

trên một file nằm trên ext4 và storage là block device.

Đi từ:

userspace

→ ARM64 syscall entry

→ sys_read / ksys_read / vfs_read hoặc equivalent Linux 5.4

→ file_operations

→ ext4

→ page cache

→ address_space_operations

→ block mapping

→ bio creation

→ submit_bio

→ generic block layer

→ request queue

→ blk-mq

→ block driver

→ DMA

→ interrupt

→ request completion

→ bio completion

→ page unlocked

→ process wakeup

→ copy/result trở lại userspace

Tạo call graph cụ thể.

Mỗi function ghi theo format:

function()
source/path/file.c

Purpose:
Caller:
Callee:
Input:
Output:
Important structures:
Context:
Locking:
Can sleep:
Kernel version notes:

==================================================
V. SECTOR / BLOCK / SEGMENT
==================================================

Tài liệu phân biệt:

Sector
Block
Segment

Hãy giải thích cực kỳ kỹ.

Phân biệt:

hardware sector
logical sector
physical sector
filesystem block
page
bio segment
physical segment
DMA segment

Ví dụ:

PAGE_SIZE = 4096
sector = 512

1 page
=
8 sectors

Nhưng giải thích thêm các trường hợp:

4K native disk
512e disk
filesystem block = 4K
page size != filesystem block

Sau đó mapping với source code:

sector_t
struct bio
struct bio_vec
struct scatterlist

==================================================
VI. STRUCT BIO
==================================================

Đây là phần rất quan trọng.

Hãy tìm:

struct bio

trong Linux 5.4.

Giải thích từng field quan trọng.

Đặc biệt:

bi_iter
bi_bdev
bi_opf
bi_io_vec
bi_vcnt
bi_end_io
bi_private
bi_status

Giải thích:

bio đại diện cho cái gì?

Tại sao bio không phải là request?

Quan hệ:

bio
  |
  +-- bio_vec
  |
  +-- page
  |
  +-- sector range

Giải thích:

bio_for_each_segment()

bio_add_page()

bio_alloc()

bio_put()

bio_get()

submit_bio()

bio_endio()

Trace lifecycle đầy đủ:

allocate
   ↓
initialize
   ↓
add page
   ↓
submit
   ↓
queue
   ↓
driver
   ↓
complete
   ↓
end_io
   ↓
free

==================================================
VII. BIO_VEC
==================================================

Phân tích:

struct bio_vec

Giải thích:

bv_page
bv_len
bv_offset

Cho ví dụ:

page
+---------------------------+
|                           |
| offset = 512              |
|      +---------------+    |
|      | len = 2048    |    |
|      +---------------+    |
|                           |
+---------------------------+

Giải thích vì sao block layer dùng:

page + offset + len

thay vì virtual address.

Liên hệ:

highmem
DMA
scatter-gather

==================================================
VIII. BIO vs REQUEST
==================================================

Giải thích cực kỳ rõ:

BIO
vs
REQUEST

Tạo bảng:

| BIO | REQUEST |

Giải thích flow:

bio1
bio2
bio3
   |
   v
merge
   |
   v
request
   |
   v
hardware queue

Giải thích:

front merge
back merge
request merge

Tìm implementation Linux 5.4.

==================================================
IX. GENDISK
==================================================

Tài liệu sử dụng:

struct gendisk

Hãy tìm implementation Linux 5.4.

Giải thích:

struct gendisk

và các field quan trọng:

major
first_minor
minors
disk_name
fops
queue
private_data
part0

Nếu field đã thay đổi so với tài liệu thì nói rõ.

Giải thích quan hệ:

gendisk
    |
    +-- request_queue
    |
    +-- block_device_operations
    |
    +-- partitions
    |
    +-- device model

==================================================
X. BLOCK_DEVICE
==================================================

Phân tích:

struct block_device

Giải thích:

bdev
inode
gendisk
partition

Quan hệ:

inode
   |
   v
block_device
   |
   v
gendisk
   |
   v
request_queue

Giải thích:

whole disk

/dev/sda

vs partition

/dev/sda1

==================================================
XI. BLOCK DEVICE REGISTRATION
==================================================

Trace quá trình driver đăng ký block device.

Tài liệu cũ dùng các API kiểu:

register_blkdev()

alloc_disk()

add_disk()

blk_init_queue()

Hãy tìm equivalent Linux 5.4.

Trace:

module_init()

→ allocate driver private data

→ register major

→ allocate gendisk

→ create request queue

→ initialize queue

→ assign gendisk fields

→ set capacity

→ add_disk

→ device model

→ /sys/block

→ /dev node

Giải thích từng bước.

==================================================
XII. REQUEST_QUEUE
==================================================

Phân tích:

struct request_queue

Đây là cấu trúc cực kỳ quan trọng.

Giải thích:

queue limits

max sectors

max segments

DMA alignment

logical block size

physical block size

discard

write cache

flush

queue flags

Giải thích quan hệ:

gendisk
   |
   v
request_queue
   |
   +-- blk-mq
   |
   +-- hardware queues

==================================================
XIII. STRUCT REQUEST
==================================================

Phân tích:

struct request

Giải thích:

request đại diện cho cái gì?

request khác bio như thế nào?

Trace:

BIO
 ↓
request
 ↓
hardware command

Giải thích fields quan trọng Linux 5.4:

q
mq_ctx
mq_hctx
cmd_flags
tag
bio
biotail
__sector
__data_len

nếu tồn tại trong version đang nghiên cứu.

==================================================
XIV. LEGACY REQUEST QUEUE vs BLK-MQ
==================================================

Tài liệu sử dụng legacy request queue và elevator.

Nhưng Linux hiện đại sử dụng blk-mq.

Hãy giải thích lịch sử:

Legacy Block Layer
        ↓
single request queue
        ↓
I/O scheduler
        ↓
driver

vs

blk-mq
        |
        +-- software queues
        |
        +-- hardware contexts
        |
        +-- tag sets
        |
        +-- hardware queues

Giải thích:

struct blk_mq_tag_set
struct blk_mq_hw_ctx
struct blk_mq_ctx

và quan hệ giữa chúng.

Vẽ sơ đồ:

CPU0 ── software queue ─┐
CPU1 ── software queue ─┤
CPU2 ── software queue ─┼── hardware queue 0
CPU3 ── software queue ─┘

==================================================
XV. BLK-MQ REQUEST FLOW
==================================================

Trace:

submit_bio()

→ generic block layer

→ blk_mq_submit_bio()

→ request allocation

→ merge

→ scheduler

→ hardware context

→ driver queue_rq()

Tìm implementation Linux 5.4.

Giải thích:

blk_mq_make_request()
blk_mq_get_request()
blk_mq_insert_request()
blk_mq_run_hw_queue()
blk_mq_dispatch_rq_list()
queue_rq()

Nếu tên function khác trong Linux 5.4 thì dùng đúng source.

==================================================
XVI. I/O SCHEDULER
==================================================

Tài liệu nói về:

Noop
Deadline
CFQ
Anticipatory

Hãy giải thích lịch sử và mapping sang Linux 5.4.

Cho biết scheduler nào:

- còn tồn tại
- deprecated
- removed
- replaced

Giải thích Linux 5.4 schedulers:

mq-deadline
kyber
bfq
none

nếu relevant.

Giải thích mục tiêu:

throughput
latency
fairness
seek reduction

==================================================
XVII. REQUEST MERGING
==================================================

Tài liệu mô tả:

front merge
back merge

Hãy tìm source hiện tại.

Giải thích bằng ví dụ:

request:
sector 100 → 107

bio:
sector 108 → 115

→ back merge

và:

bio:
sector 92 → 99

→ front merge

Tìm function xử lý merge.

==================================================
XVIII. PLUGGING
==================================================

Tài liệu mô tả:

plug
unplug

Hãy giải thích cơ chế hiện đại:

blk_start_plug()
blk_finish_plug()

Giải thích tại sao plugging cải thiện performance.

Ví dụ:

write block A
write block B
write block C

thay vì dispatch từng request ngay:

collect
merge
dispatch

==================================================
XIX. DMA / SCATTER-GATHER
==================================================

Tài liệu nói nhiều về segments và scatter-gather DMA.

Trace:

bio_vec
   ↓
request
   ↓
scatterlist
   ↓
dma_map_sg()
   ↓
DMA descriptors
   ↓
device

Giải thích:

virtual address
physical page
DMA address

khác nhau như thế nào.

Phân tích:

struct scatterlist

sg_init_table()
sg_set_page()
dma_map_sg()

==================================================
XX. DRIVER STRATEGY ROUTINE
==================================================

Tài liệu dùng khái niệm:

strategy routine

Hãy mapping sang Linux 5.4.

Trong blk-mq driver:

struct blk_mq_ops

đặc biệt:

.queue_rq

Giải thích:

queue_rq()

nhận request như thế nào

driver convert request thành hardware command thế nào.

==================================================
XXI. INTERRUPT + COMPLETION
==================================================

Trace hoàn chỉnh:

storage device
   ↓
DMA complete
   ↓
IRQ
   ↓
driver interrupt handler
   ↓
ack hardware interrupt
   ↓
complete request
   ↓
blk_mq_end_request()
   ↓
bio completion
   ↓
bio_endio()
   ↓
filesystem/page cache
   ↓
wake process

Phân tích context:

Hard IRQ
SoftIRQ
Task context

Nếu kernel version có softirq/block completion mechanism liên quan thì giải thích.

==================================================
XXII. OPENING A BLOCK DEVICE
==================================================

Tài liệu cuối chapter mô tả mở block device file.

Trace:

open("/dev/sda", ...)

→ ARM64 syscall

→ do_sys_open()

→ path lookup

→ inode

→ file_operations

→ blkdev_open()

→ block_device

→ gendisk

→ block_device_operations->open()

Trace chính xác Linux 5.4.

Giải thích:

/dev/sda

khác:

regular file

như thế nào trong VFS.

==================================================
XXIII. BLOCK_DEVICE_OPERATIONS
==================================================

Phân tích:

struct block_device_operations

Giải thích các callback:

open
release
ioctl
getgeo
media_changed
revalidate_disk

nếu tồn tại trong Linux 5.4.

Mapping với table trong tài liệu.

==================================================
XXIV. DATA STRUCTURE RELATIONSHIP
==================================================

Vẽ sơ đồ cực kỳ rõ:

             inode
               |
               v
        struct block_device
               |
               v
          struct gendisk
               |
        +------+------+
        |             |
        v             v
block_device_ops  request_queue
                      |
                      v
                   blk-mq
                      |
                      v
                   request
                      |
                      v
                     bio
                      |
                      v
                  bio_vec
                      |
                      v
                    page

Sau đó giải thích ownership/lifetime từng object.

==================================================
XXV. CODE WALKTHROUGH
==================================================

Mỗi subsystem phải có ít nhất một source walkthrough.

Format:

------------------------------------
FUNCTION
------------------------------------

Function:
submit_bio()

Source:
block/blk-core.c

Prototype:
...

Purpose:
...

Caller:
...

Important code:
...

Step 1:
...

Step 2:
...

Step 3:
...

Next function:
...

Không paste hàng trăm dòng code.

Chỉ trích đoạn quan trọng và giải thích từng dòng.

==================================================
XXVI. CALL GRAPH
==================================================

Tạo call graph cho ít nhất các flow:

1. read regular file → disk
2. write regular file → disk
3. submit_bio()
4. blk-mq request dispatch
5. interrupt completion
6. open /dev/sda
7. block driver registration

Dùng ASCII diagram.

Ví dụ:

submit_bio()
    |
    v
submit_bio_noacct()
    |
    v
...
    |
    v
blk_mq_submit_bio()
    |
    v
...
    |
    v
driver->queue_rq()

Nhưng phải dựa trên Linux 5.4 source thật.

==================================================
XXVII. CONTEXT + LOCKING
==================================================

Với mỗi function quan trọng, cho biết:

Context:
- process
- softirq
- hardirq
- workqueue

Can sleep:
YES / NO

Lock:
spinlock
mutex
RCU
atomic
none

Giải thích vì sao.

==================================================
XXVIII. ARM64
==================================================

Nếu flow bắt đầu từ userspace syscall:

read()
write()
open()

hãy trace ARM64 syscall entry:

EL0
 ↓
svc #0
 ↓
exception vector
 ↓
el0_sync
 ↓
el0_svc
 ↓
syscall dispatcher
 ↓
__arm64_sys_xxx

Mapping với:

arch/arm64/kernel/entry.S

hoặc implementation tương ứng Linux 5.4.

Chỉ phân tích ARM64-specific khi thực sự liên quan.

Block layer bản thân phải được phân biệt với architecture-independent code.

==================================================
XXIX. SOURCE TREE MAP
==================================================

Sau khi phân tích, tạo source tree map:

block/
├── bio.c
├── blk-core.c
├── blk-mq.c
├── blk-merge.c
├── blk-settings.c
├── elevator.c
└── ...

fs/
├── block_dev.c
└── ...

include/linux/
├── bio.h
├── blkdev.h
└── blk-mq.h

drivers/
└── block/

Ghi chức năng từng file.

==================================================
XXX. LEGACY vs MODERN KERNEL TABLE
==================================================

Tạo bảng:

| Concept trong sách | Linux 2.6 | Linux 5.4 | Trạng thái |

Bao gồm ít nhất:

bio
bio_vec
request
request_queue
gendisk
hd_struct
generic_make_request
make_request_fn
request_fn
elevator
CFQ
deadline
noop
blk_init_queue
blk-mq

Trạng thái:

UNCHANGED
CHANGED
REPLACED
REMOVED
REFACTORED

==================================================
XXXI. DEBUG / OBSERVABILITY
==================================================

Cho tôi cách quan sát block layer trên Linux thật.

Bao gồm:

lsblk
cat /proc/partitions
/sys/block
/sys/class/block

block statistics

iostat

blktrace

ftrace

trace-cmd

perf

BPF nếu phù hợp.

Đưa ví dụ tracepoints:

block:block_rq_insert
block:block_rq_issue
block:block_rq_complete

nếu tồn tại trong Linux 5.4.

Giải thích cách dùng chúng để trace:

process
→ bio
→ request
→ driver
→ completion

==================================================
XXXII. LAB THỰC HÀNH
==================================================

Thiết kế lab để tôi tự kiểm chứng.

LAB 1:
Quan sát /sys/block

LAB 2:
Trace open("/dev/sda")

LAB 3:
Trace submit_bio

LAB 4:
Trace request insert/issue/complete

LAB 5:
So sánh sequential vs random I/O

LAB 6:
Quan sát I/O scheduler

LAB 7:
Viết một minimal RAM block driver hoặc blk-mq block driver

Mỗi lab phải có:

Goal
Commands
Expected result
Kernel functions cần đặt breakpoint/trace
Điều cần quan sát

==================================================
XXXIII. MINIMAL BLOCK DRIVER
==================================================

Sau khi hiểu kiến trúc, hãy thiết kế một minimal RAM block device driver cho Linux 5.4.

Ví dụ:

/dev/myblock

Capacity:
16 MB

sector size:
512

Dữ liệu lưu trong RAM.

Driver phải minh họa:

gendisk
request_queue / blk-mq
blk_mq_tag_set
blk_mq_ops
queue_rq
request processing
blk_mq_end_request

Giải thích từng dòng.

Không cần production quality.

Mục tiêu là để hiểu block layer.

==================================================
XXXIV. PHƯƠNG PHÁP GIẢI THÍCH
==================================================

Tôi là Embedded Software Engineer và đang học Linux Kernel source code.

Do đó hãy giải thích theo hướng:

hardware
→ driver
→ kernel subsystem
→ data structure
→ call flow

Ưu tiên source code hơn lý thuyết.

Mỗi concept phải trả lời 5 câu:

1. Nó là gì?
2. Tại sao kernel cần nó?
3. Nó được biểu diễn bằng struct nào?
4. Function nào thao tác trên nó?
5. Nó nằm ở đâu trong end-to-end flow?

==================================================
XXXV. QUY TẮC QUAN TRỌNG
==================================================

Không được:

- chỉ giải thích textbook
- tự bịa function
- tự bịa source path
- trộn Linux 2.6 và Linux 5.4 mà không cảnh báo
- dùng legacy API rồi giả vờ nó vẫn còn trong kernel mới

Nếu chưa xác minh:

ghi rõ:

UNVERIFIED

Nếu API khác version:

ghi:

Linux 2.6:
...

Linux 5.4:
...

==================================================
XXXVI. OUTPUT CUỐI CÙNG
==================================================

Sau khi hoàn thành chapter, tạo một MASTER FLOW:

Userspace
   ↓
ARM64 syscall
   ↓
VFS
   ↓
Filesystem
   ↓
Page Cache
   ↓
BIO
   ↓
Generic Block Layer
   ↓
BIO merge
   ↓
Request
   ↓
blk-mq
   ↓
I/O Scheduler
   ↓
Hardware Queue
   ↓
Driver queue_rq()
   ↓
DMA mapping
   ↓
Controller
   ↓
Storage
   ↓
IRQ
   ↓
blk_mq_end_request()
   ↓
bio_endio()
   ↓
Page completion
   ↓
Wake process
   ↓
Userspace

Đối với từng arrow:

ghi function thực hiện transition.

Mục tiêu cuối cùng:

Sau khi học xong, tôi phải có khả năng nhìn thấy một:

read()
write()
submit_bio()
struct bio
struct request
request_queue
blk-mq
queue_rq()
interrupt

và biết chính xác nó nằm ở đâu trong toàn bộ Linux block I/O architecture.