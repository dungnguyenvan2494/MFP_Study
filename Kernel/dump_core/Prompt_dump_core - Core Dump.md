# ROLE

Bạn là một Linux Kernel Engineer và Kernel Source Code Researcher chuyên về:
- Linux kernel internals
- Process lifecycle
- Signal subsystem
- Core dump
- ELF binary format
- MM / VM / VMA / page tables
- Filesystem and kernel I/O
- Scheduler
- Locking / synchronization
- x86_64 / ARM64 architecture
- Kernel debugging

Tôi muốn bạn giúp tôi **nghiên cứu cực kỳ chi tiết cơ chế core dump trong Linux kernel**, tập trung vào source code thực tế.

Không được chỉ giải thích ở mức conceptual.
Mục tiêu là phải giúp tôi **đọc và hiểu source code như một kernel developer**.

---

# SOURCE OF TRUTH

Tôi sẽ cung cấp Linux kernel source tree.

Ưu tiên tuyệt đối:
1. Source code thực tế của kernel tôi cung cấp.
2. Function definitions trong source.
3. Struct definitions.
4. Macro definitions.
5. Call sites.
6. Comments trong source.
7. Commit/history nếu tôi cung cấp.

Không được tự suy đoán khi source code có thể kiểm tra được.

Khi kernel version khác nhau làm implementation thay đổi, phải nói rõ:
- version nào
- function nào thay đổi
- logic nào thay đổi
- tại sao thay đổi

Không được trộn implementation của Linux 5.4, 5.10, 6.x... mà không cảnh báo.

---

# MAIN OBJECTIVE

Hãy giúp tôi hiểu toàn bộ pipeline:

    User process
        ↓
    CPU exception / signal
        ↓
    SIGSEGV / SIGABRT / SIGILL / SIGBUS / ...
        ↓
    signal generation
        ↓
    signal delivery
        ↓
    get_signal()
        ↓
    core-dump decision
        ↓
    do_coredump()
        ↓
    core dump infrastructure
        ↓
    binfmt handler
        ↓
    ELF core dump
        ↓
    ELF header
        ↓
    program headers
        ↓
    NOTE segments
        ↓
    thread/register state
        ↓
    process memory mappings
        ↓
    VMA
        ↓
    page collection / memory access
        ↓
    write to core file
        ↓
    process termination
        ↓
    GDB loads core
        ↓
    stack/register/memory reconstruction

Tôi muốn hiểu chính xác từng transition ở trên.

---

# PART 1 — CORE DUMP CONCEPT

Giải thích:

1. Core dump là gì?
2. Core dump khác crash log như thế nào?
3. Core dump khác kernel panic như thế nào?
4. Core dump khác kernel dump như vmcore như thế nào?
5. Core dump thuộc subsystem nào?
6. Tại sao core dump nằm gần signal processing?
7. Process nào có thể tạo core?
8. Kernel quyết định process có được dump hay không ở đâu?
9. Core dump có phải lúc nào cũng xảy ra khi SIGSEGV không?
10. Signal disposition ảnh hưởng như thế nào?
11. RLIMIT_CORE ảnh hưởng thế nào?
12. permissions / dumpability ảnh hưởng thế nào?
13. setuid / setgid ảnh hưởng thế nào?
14. `/proc/sys/kernel/core_pattern` ảnh hưởng thế nào?
15. `/proc/<pid>/coredump_filter` ảnh hưởng thế nào?
16. systemd-coredump liên quan ở đâu?

Sau phần conceptual phải map mỗi khái niệm vào source code.

---

# PART 2 — FIND ALL RELEVANT SOURCE FILES

Hãy tìm và liệt kê toàn bộ source file quan trọng liên quan đến core dump.

Tối thiểu phải kiểm tra:

- kernel/signal.c
- fs/coredump.c
- fs/binfmt_elf.c

Sau đó tự tìm thêm các file liên quan.

Với mỗi file hãy giải thích:

    File
    ↓
    Responsibility
    ↓
    Important structs
    ↓
    Important functions
    ↓
    Relationship to core dump

Tạo bảng:

| File | Responsibility | Important Functions | Important Structs |
|------|----------------|---------------------|-------------------|

---

# PART 3 — BUILD THE CALL GRAPH

Hãy dựng call graph chính xác từ source.

Bắt đầu từ signal gây crash.

Ví dụ:

    page fault
       ↓
    SIGSEGV
       ↓
    signal generation
       ↓
    signal pending
       ↓
    get_signal()
       ↓
    do_coredump()
       ↓
    ELF handler
       ↓
    elf_core_dump()

Nhưng không được giả định call path.

Hãy đọc source và xác định chính xác.

Với mỗi function:

- ai gọi function này?
- function gọi ai?
- input arguments là gì?
- return value có nghĩa gì?
- global state nào bị đọc?
- global state nào bị ghi?
- struct nào được truy cập?
- lock nào đang giữ?
- lock nào được acquire/release?
- process state lúc này là gì?
- context đang là process context hay special context?
- có thể sleep không?
- có thể fail không?

Output thành:

    Caller
      ↓
    Function
      ↓
    Callee

và tạo một call graph dạng ASCII.

---

# PART 4 — TRACE FROM SIGSEGV

Hãy chọn một chương trình tối thiểu:

    int *p = NULL;
    *p = 10;

Sau đó trace cực kỳ chi tiết:

    CPU
     ↓
    exception
     ↓
    page fault
     ↓
    architecture-specific fault handler
     ↓
    signal generation
     ↓
    signal pending
     ↓
    return to user
     ↓
    signal delivery
     ↓
    get_signal()
     ↓
    fatal signal path
     ↓
    core dump
     ↓
    process exit

Phân biệt:

- ARM64
- x86_64

nếu implementation khác nhau.

Chỉ ra chính xác:
- register state được lưu ở đâu
- `pt_regs` nằm ở đâu
- architecture-specific exception frame được xử lý như thế nào
- signal được tạo như thế nào
- kernel biết signal này là core-dumpable ở đâu

---

# PART 5 — UNDERSTAND SIGNAL → CORE DECISION

Phân tích chính xác logic:

    signal arrives
        ↓
    should process terminate?
        ↓
    should core dump?
        ↓
    can core dump?
        ↓
    do_coredump()

Tìm các điều kiện trong source.

Giải thích từng condition.

Không được nói chung chung kiểu:

"kernel checks permissions"

Phải chỉ ra:
- function
- source location
- variable
- field
- condition
- effect

Ví dụ format:

    condition:
        ...

    source:
        ...

    meaning:
        ...

    if TRUE:
        ...

    if FALSE:
        ...

---

# PART 6 — DEEP DIVE INTO do_coredump()

Đây là phần quan trọng nhất.

Phân tích `do_coredump()` từng statement / block logic.

Với từng block hãy trả lời:

1. Mục đích?
2. State trước block?
3. State sau block?
4. Lock nào đang giữ?
5. Lock nào được release?
6. Có thể sleep không?
7. Có thể fail không?
8. Nếu fail thì process làm gì?
9. Memory ownership thế nào?
10. Concurrency issue nào có thể xảy ra?

Tạo pseudo-code dễ đọc:

    do_coredump()
    {
        phase 1
        phase 2
        phase 3
        ...
    }

Sau đó map từng phase với source code thực tế.

---

# PART 7 — UNDERSTAND COREDUMP STATE MACHINE

Nếu implementation có state machine hoặc state transitions, hãy dựng:

    PROCESS RUNNING
         ↓
    FATAL SIGNAL
         ↓
    CORE DUMP DECISION
         ↓
    CORE DUMP START
         ↓
    OTHER THREADS STOP / PARTICIPATE
         ↓
    MEMORY SNAPSHOT
         ↓
    WRITE CORE
         ↓
    CORE COMPLETE / FAIL
         ↓
    PROCESS EXIT

Tìm source code thực tế chứng minh từng transition.

---

# PART 8 — CORE DUMP AND MULTI-THREADING

Giả sử:

    Process
       ├── Thread A
       ├── Thread B
       ├── Thread C
       └── Thread D

A bị SIGSEGV.

Phân tích:

1. Thread nào nhận signal?
2. Thread nào thực hiện core dump?
3. Thread nào bị stop?
4. Các thread khác có chạy trong khi core dump không?
5. `core_state` là gì?
6. Các thread rendezvous như thế nào?
7. Kernel tránh race với thread exit như thế nào?
8. Register của từng thread lấy ở đâu?
9. Stack của từng thread được dump thế nào?
10. Nếu một thread đang tạo hoặc destroy VMA trong lúc dump thì sao?
11. Nếu thread đang mmap/munmap thì sao?
12. Nếu another thread đang exit thì sao?

Phân tích tất cả locking liên quan.

---

# PART 9 — COREDUMP LOCKING

Tìm toàn bộ lock liên quan đến core dump.

Phân loại:

- mutex
- spinlock
- rwsem
- atomic
- completion
- wait queue
- tasklist locks
- mm locks
- signal locks

Với mỗi lock:

| Lock | Owner | Acquire | Release | Purpose | Can Sleep? |
|------|-------|---------|---------|---------|------------|

Sau đó vẽ lock dependency:

    Lock A
      ↓
    Lock B
      ↓
    Lock C

Tìm potential deadlock.

Giải thích tại sao kernel phải tuân thủ lock ordering.

---

# PART 10 — COREDUMP AND MM STRUCTURE

Đây là phần phải cực kỳ sâu.

Giải thích:

    task_struct
       ↓
    mm_struct
       ↓
    vm_area_struct
       ↓
    page tables
       ↓
    physical pages

Phân tích các field quan trọng:

- task_struct
- mm_struct
- vm_area_struct
- mmap / VMA tree structure
- pgd
- vm_start
- vm_end
- vm_flags
- vm_file
- anon_vma
- vm_ops

Giải thích core dump tìm memory regions như thế nào.

---

# PART 11 — VMA WALK

Trace chính xác source code dùng để iterate VMA.

Ví dụ conceptual:

    for each VMA
        check whether it should be dumped
        determine memory range
        create PT_LOAD
        write memory

Nhưng phải tìm implementation thật.

Giải thích:

1. VMA được enumerate như thế nào?
2. Lock nào bảo vệ VMA?
3. VMA tree được sử dụng thế nào?
4. VMA merge/split có ảnh hưởng không?
5. `munmap()` concurrent có thể xảy ra không?
6. Core dump giữ ổn định memory layout như thế nào?

---

# PART 12 — coredump_filter

Nghiên cứu sâu:

    /proc/<pid>/coredump_filter

Giải thích:

1. Bitmask format.
2. Mỗi bit có nghĩa gì.
3. Kernel đọc bitmask ở đâu.
4. VMA được quyết định dump/skip ở đâu.
5. Anonymous memory.
6. File-backed memory.
7. Shared memory.
8. Private memory.
9. Huge pages.
10. DAX nếu có.
11. Mapping đặc biệt.

Tạo bảng:

| Bit | Mapping Type | Dump? | Example |
|-----|--------------|-------|---------|

Sau đó trace từng bit vào source.

---

# PART 13 — ELF CORE FORMAT

Đây là phần cực kỳ quan trọng.

Giải thích ELF core file từ đầu.

Tạo ví dụ:

    core
    │
    ├── ELF Header
    ├── Program Header Table
    │
    ├── PT_NOTE
    │     ├── NT_PRSTATUS
    │     ├── NT_PRPSINFO
    │     ├── NT_SIGINFO
    │     ├── NT_AUXV
    │     └── architecture-specific notes
    │
    ├── PT_LOAD
    │     ├── text
    │     ├── data
    │     ├── heap
    │     └── stack
    │
    └── ...

Giải thích chính xác kernel tạo từng thành phần ở đâu.

---

# PART 14 — ELF HEADER GENERATION

Trace source:

    elf_core_dump()
       ↓
    ELF header generation

Giải thích từng field:

- e_ident
- e_type
- e_machine
- e_version
- e_phoff
- e_shoff
- e_phnum
- e_phentsize

Tại sao core dump thường dùng:

    ET_CORE

Architecture-specific differences phải được nói rõ.

---

# PART 15 — PROGRAM HEADER GENERATION

Giải thích:

    PT_NOTE
    PT_LOAD

Tạo flow:

    VMA
      ↓
    dump decision
      ↓
    program header
      ↓
    file offset
      ↓
    virtual address
      ↓
    memory size
      ↓
    file size

Giải thích cực kỳ kỹ:

- p_type
- p_offset
- p_vaddr
- p_paddr
- p_filesz
- p_memsz
- p_flags
- p_align

Đặc biệt giải thích:

    p_filesz != p_memsz

và tại sao.

---

# PART 16 — NOTE SEGMENTS

Phân tích:

    NT_PRSTATUS
    NT_PRPSINFO
    NT_SIGINFO
    NT_AUXV

Nếu kernel/version có thêm notes thì tìm và giải thích.

Với mỗi note:

    who produces it
    where it is created
    struct used
    information contained
    why GDB needs it

---

# PART 17 — REGISTER DUMP

Phân tích cực sâu.

Từ:

    CPU register state
        ↓
    pt_regs
        ↓
    ELF note
        ↓
    core file
        ↓
    GDB register view

Phải chỉ ra source code thực tế.

Phân biệt:

- x86_64
- ARM64

Giải thích các register:

    PC
    SP
    FP
    general purpose registers
    flags / PSTATE
    architecture-specific registers

---

# PART 18 — MEMORY DUMP PATH

Đây là phần tôi muốn hiểu sâu nhất.

Trace:

    VMA
     ↓
    memory range
     ↓
    page lookup
     ↓
    page data
     ↓
    kernel buffer / iterator
     ↓
    file write

Tìm function thực tế thực hiện memory dump.

Không được dừng ở `elf_core_dump()`.

Hãy đi sâu đến function thực sự copy memory.

Giải thích:

- page fault có thể xảy ra không?
- page có thể không resident không?
- swapped-out page thì sao?
- anonymous page thì sao?
- file-backed page thì sao?
- huge page thì sao?
- zero page thì sao?
- inaccessible page thì sao?

---

# PART 19 — PAGE TABLE LEVEL

Giải thích khi core dump cần đọc virtual memory:

    user virtual address
        ↓
    PGD
        ↓
    P4D
        ↓
    PUD
        ↓
    PMD
        ↓
    PTE
        ↓
    physical page

Nhưng phải phân biệt:
- core dump trực tiếp page-table walk hay thông qua abstraction nào
- implementation thực tế của kernel version đang nghiên cứu

Không được giả định.

---

# PART 20 — FILE I/O PATH

Trace từ:

    core dump
       ↓
    kernel file abstraction
       ↓
    write

Xác định:

- `struct file`
- file position
- kernel_write / write_iter / similar mechanism
- page cache involvement
- filesystem
- block layer

Giải thích core dump từ kernel memory đến storage device.

Flow:

    ELF data
       ↓
    kernel
       ↓
    VFS
       ↓
    filesystem
       ↓
    page cache
       ↓
    block layer
       ↓
    block device
       ↓
    SSD / eMMC / storage

Nếu core_pattern dùng pipe hãy phân tích flow khác.

---

# PART 21 — core_pattern

Nghiên cứu rất sâu:

    /proc/sys/kernel/core_pattern

Các trường hợp:

1. Direct file
2. Pipe handler

Ví dụ:

    core.%e.%p

và:

    |/usr/lib/systemd/systemd-coredump ...

Phải trace:

    core_pattern
        ↓
    filename generation
        ↓
    open file

và:

    core_pattern starts with |
        ↓
    create pipe
        ↓
    userspace helper
        ↓
    helper receives core dump

---

# PART 22 — systemd-coredump

Không cần nghiên cứu toàn bộ systemd.

Chỉ cần giải thích:

    kernel
      ↓
    pipe
      ↓
    systemd-coredump
      ↓
    storage
      ↓
    coredumpctl
      ↓
    gdb

So sánh với direct core file.

---

# PART 23 — PROCESS EXIT

Sau khi core dump xong:

    do_coredump()
       ↓
    signal handling
       ↓
    do_exit()
       ↓
    exit_mm()
       ↓
    mm cleanup
       ↓
    task cleanup

Trace chính xác.

Giải thích:
- core dump trước hay sau `do_exit()`?
- khi nào mm còn tồn tại?
- khi nào address space bị destroy?
- vì sao core dump phải diễn ra trước một số cleanup?

---

# PART 24 — FAILURE PATH

Phân tích mọi failure có thể xảy ra:

- cannot open core file
- permission failure
- file size limit
- disk full
- signal interruption
- memory allocation failure
- VMA access failure
- pipe handler failure
- ELF generation failure
- filesystem failure

Với mỗi failure:

    Detection
        ↓
    Error code
        ↓
    Recovery
        ↓
    Does process still die?
        ↓
    Is partial core generated?

---

# PART 25 — RESOURCE LIMITS

Phân tích:

    RLIMIT_CORE

và các resource limitations liên quan.

Giải thích:

- giới hạn được kiểm tra ở đâu
- file size được giới hạn thế nào
- truncated core có thể xảy ra không
- sparse output có thể xảy ra không

---

# PART 26 — SECURITY

Phân tích security model.

Tập trung:

- setuid
- setgid
- dumpable
- secureexec
- ptrace restrictions nếu liên quan
- permission
- core pattern
- privileged processes

Câu hỏi:

"Vì sao Linux không cho phép mọi process dump core tùy ý?"

Trace logic vào source.

---

# PART 27 — CONCURRENCY / RACE CONDITIONS

Phân tích các race có thể có:

### Race 1
    core dump
       vs
    another thread mmap()

### Race 2
    core dump
       vs
    munmap()

### Race 3
    core dump
       vs
    thread exit

### Race 4
    core dump
       vs
    execve()

### Race 5
    core dump
       vs
    fork()

### Race 6
    core dump
       vs
    ptrace/debugger

Tìm kernel mechanism bảo vệ mỗi case.

Đặc biệt phải liên hệ:

- mm lock
- task list
- signal lock
- reference counting
- mm_users
- mm_count
- get_task_mm()
- mmgrab()
- mmap locking

---

# PART 28 — MEMORY CONSISTENCY

Giải thích core dump có phải là snapshot "atomic" của process hay không.

Ví dụ:

    Thread A writes memory
    Thread B causes SIGSEGV
    Thread C modifies another mapping

Core dump nhìn thấy trạng thái nào?

Phải trả lời dựa trên kernel implementation.

Tôi muốn hiểu chính xác:

    Is core dump a consistent snapshot?
    If not, what consistency guarantees exist?

---

# PART 29 — GDB SIDE

Không dừng ở kernel.

Trace:

    core file
       ↓
    GDB
       ↓
    ELF parser
       ↓
    PT_NOTE
       ↓
    register recovery
       ↓
    PT_LOAD
       ↓
    virtual memory reconstruction
       ↓
    symbol resolution
       ↓
    backtrace

Giải thích tại sao GDB có thể làm:

    bt
    info registers
    info threads
    x/32gx ADDRESS

từ core file.

---

# PART 30 — HANDS-ON EXPERIMENT

Tạo một experiment nhỏ.

Program:

    #include <stdio.h>

    int global_data = 10;

    int main()
    {
        int *p = NULL;
        *p = 123;
        return 0;
    }

Build:

    gcc -g -O0 test.c -o test

Enable:

    ulimit -c unlimited

Run.

Sau đó:

    readelf -h core
    readelf -l core
    readelf -n core
    gdb ./test core

Yêu cầu giải thích output.

---

# PART 31 — EXPERIMENT WITH THREADS

Viết chương trình:

    main thread
        ↓
    pthread A
        ↓
    pthread B
        ↓
    pthread C

Một thread crash.

Quan sát:

    info threads
    thread apply all bt

Sau đó map kết quả trở lại:

    kernel thread traversal
        ↓
    NT_PRSTATUS
        ↓
    GDB threads

---

# PART 32 — EXPERIMENT WITH MEMORY MAPPINGS

Tạo process có:

- stack
- heap
- anonymous mmap
- file-backed mmap
- shared mmap

Sau đó kiểm tra:

    /proc/<pid>/maps

và core.

So sánh:

    process VMA
        vs
    PT_LOAD

Giải thích mapping nào xuất hiện và vì sao.

---

# PART 33 — EXPERIMENT WITH coredump_filter

Thử thay đổi:

    /proc/self/coredump_filter

Ví dụ các bitmask khác nhau.

So sánh:

    size of core
    PT_LOAD count
    mappings dumped

Đưa ra kết luận từ experiment.

---

# PART 34 — DEBUG THE KERNEL

Hướng dẫn cách debug core dump implementation bằng:

- printk
- dynamic_debug
- ftrace
- function_graph tracer
- kprobe
- bpftrace nếu phù hợp
- kgdb
- QEMU + GDB

Mục tiêu:

trace:

    do_coredump()
        ↓
    elf_core_dump()
        ↓
    VMA dump
        ↓
    file write

---

# PART 35 — SOURCE-CODE READING METHOD

Khi phân tích mỗi function, luôn dùng template:

## Function

`function_name()`

## Location

`path/to/file.c:line`

## Purpose

...

## Caller

...

## Callees

...

## Important arguments

...

## Important structs

...

## Locking

...

## Memory ownership

...

## Error handling

...

## Side effects

...

## Why this function exists

...

## Related functions

...

---

# PART 36 — STRUCT RELATIONSHIP GRAPH

Dựng relationship:

    task_struct
       │
       ├── signal
       ├── sighand
       ├── mm
       ├── cred
       └── thread
              │
              ▼
           pt_regs

    mm_struct
       │
       └── VMAs

    VMA
       │
       ├── vm_file
       ├── anon_vma
       ├── vm_flags
       └── page tables

    core dump
       │
       ├── task state
       ├── thread state
       ├── VMAs
       └── memory

Giải thích từng arrow.

---

# PART 37 — ARCHITECTURE COMPARISON

So sánh:

## x86_64

    exception
    pt_regs
    signal
    core dump

## ARM64

    exception
    pt_regs
    signal
    core dump

Tạo bảng:

| Area | x86_64 | ARM64 |
|------|--------|-------|
| Fault entry | | |
| Register frame | | |
| PC | | |
| SP | | |
| Signal setup | | |
| Core register dump | | |

---

# PART 38 — SOURCE CODE ANNOTATION

Khi tôi đưa một function source code, hãy annotate trực tiếp:

    original line
       ↓
    explanation
       ↓
    kernel concept
       ↓
    relationship to core dump

Không bỏ qua những dòng có vẻ "nhỏ".

Ví dụ:

    get_task_mm()
    mmgrab()
    atomic operations
    reference counting

cũng phải giải thích vì sao chúng tồn tại.

---

# PART 39 — DON'T SKIP BORING DETAILS

Tôi muốn hiểu cả những phần thường bị bỏ qua:

- reference counting
- file reference
- struct lifetime
- task lifetime
- mm lifetime
- VMA lifetime
- page lifetime
- error propagation
- offset calculation
- alignment
- padding
- ELF alignment
- page size
- architecture-specific alignment

---

# PART 40 — FINAL MASTER DIAGRAM

Sau khi nghiên cứu xong, hãy tạo một diagram cực kỳ chi tiết:

    APPLICATION
        │
        ▼
    CPU EXCEPTION
        │
        ▼
    ARCH EXCEPTION HANDLER
        │
        ▼
    SIGNAL GENERATION
        │
        ▼
    SIGNAL PENDING
        │
        ▼
    SIGNAL DELIVERY
        │
        ▼
    get_signal()
        │
        ▼
    FATAL SIGNAL
        │
        ├───────────────┐
        │               │
        ▼               ▼
    CORE CHECK       NO CORE
        │               │
        ▼               │
    do_coredump()       │
        │               │
        ▼               │
    core_pattern        │
        │               │
        ▼               │
    ELF CORE HANDLER    │
        │               │
        ├── ELF HEADER  │
        ├── PT_NOTE     │
        ├── THREADS     │
        ├── REGISTERS   │
        ├── VMAs        │
        └── MEMORY      │
               │        │
               ▼        ▼
           CORE FILE / PIPE
               │
               ▼
              GDB
               │
               ▼
         Debugging Session

Mỗi node phải link tới source function thực tế.

---

# PART 41 — QUESTIONS I MUST BE ABLE TO ANSWER

Sau khi hoàn thành nghiên cứu, tôi phải có thể tự trả lời:

1. Chính xác function nào bắt đầu core dump?
2. Từ SIGSEGV đến `do_coredump()` đi qua những function nào?
3. Tại sao signal handler biết signal này cần core dump?
4. Thread nào thực hiện core dump?
5. Thread khác được xử lý thế nào?
6. Kernel lấy register ở đâu?
7. Kernel lấy VMA ở đâu?
8. Kernel quyết định VMA nào được dump ở đâu?
9. `coredump_filter` được dùng ở đâu?
10. PT_NOTE được tạo ở đâu?
11. PT_LOAD được tạo ở đâu?
12. Kernel copy memory vào core file bằng function nào?
13. File write đi qua VFS như thế nào?
14. Core dump có phải atomic snapshot không?
15. Lock nào bảo vệ memory layout?
16. Lock nào bảo vệ thread state?
17. `core_pattern` hoạt động thế nào?
18. Pipe mode khác file mode thế nào?
19. systemd-coredump nằm ở đâu trong flow?
20. Khi core dump thất bại process có chết không?
21. ELF core file liên hệ với GDB thế nào?
22. ARM64 khác x86_64 ở đâu?
23. Core dump xử lý anonymous memory thế nào?
24. Core dump xử lý mmap file-backed memory thế nào?
25. Core dump xử lý swapped memory thế nào?
26. Core dump xử lý multithread thế nào?
27. Vì sao cần reference counting?
28. Vì sao cần mm lifetime management?
29. Process exit xảy ra trước hay sau core dump?
30. Nếu process đang `mmap()` / `munmap()` trong lúc crash thì sao?

---

# OUTPUT FORMAT

Không trả lời toàn bộ như một bài viết ngắn.

Hãy chia nghiên cứu thành các chương:

    Chapter 01 — Core Dump Overview
    Chapter 02 — Signal → Core Dump Entry
    Chapter 03 — do_coredump()
    Chapter 04 — Core Dump Concurrency
    Chapter 05 — ELF Core Format
    Chapter 06 — Thread and Register Dump
    Chapter 07 — VMA and MM
    Chapter 08 — Memory Dump
    Chapter 09 — File I/O
    Chapter 10 — core_pattern
    Chapter 11 — Security
    Chapter 12 — Error Paths
    Chapter 13 — GDB
    Chapter 14 — Architecture Differences
    Chapter 15 — Hands-on Experiments
    Chapter 16 — Full Source Call Graph

Mỗi chapter phải có:

    Concept
    ↓
    Source code
    ↓
    Function-by-function analysis
    ↓
    Struct analysis
    ↓
    Locking analysis
    ↓
    Memory/lifetime analysis
    ↓
    Flow diagram
    ↓
    Practical experiment
    ↓
    Interview-level questions

---

# EXTREMELY IMPORTANT

Đừng chỉ nói:

"kernel writes process memory to the core file."

Tôi muốn biết:

    WHICH FUNCTION?
    WHICH STRUCT?
    WHICH LOCK?
    WHICH MEMORY?
    WHICH ITERATOR?
    WHICH FILE OPERATION?
    WHICH ERROR PATH?
    WHICH REFERENCE COUNT?
    WHICH ARCHITECTURE CODE?

Mọi câu trả lời quan trọng phải được nối trực tiếp về source code.

Nếu source code chưa đủ để kết luận, hãy nói:

    "Need to inspect X"

thay vì đoán.

Mục tiêu cuối cùng:

Tôi muốn có khả năng mở Linux kernel source, đi đến:

    do_coredump()

và tự mình trace toàn bộ đường đi:

    signal
      → core dump decision
      → do_coredump
      → ELF generation
      → thread state
      → register state
      → VMA
      → memory
      → file
      → process exit

mà không còn function hoặc struct quan trọng nào là "black box".