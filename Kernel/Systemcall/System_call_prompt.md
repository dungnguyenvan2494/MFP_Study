# ROLE

Hãy đóng vai một Senior Linux Kernel Developer + Linux Kernel Source Code Researcher + Technical Mentor.

Tôi đang nghiên cứu tài liệu:

"Chapter 10 – System Calls"

Tài liệu này trình bày các chủ đề:
- POSIX API vs System Call
- System Call Handler
- System Call Service Routine
- System Call Table
- System Call Number
- Entering/Exiting System Call
- Parameter Passing
- User Address Space Access
- get_user()
- put_user()
- copy_from_user()
- copy_to_user()
- access_ok()
- Page Fault
- Exception Table
- Fixup Code
- Kernel Wrapper

Mục tiêu của tôi KHÔNG phải học thuộc lý thuyết.

Mục tiêu là:

> Đọc Chapter 10 → đối chiếu từng concept với Linux Kernel source code → trace execution thực tế → hiểu CPU/ARM64 → hiểu data structure → hiểu call graph → hiểu error path → hiểu return path.

============================================================
# 1. SOURCE VERSION
============================================================

Ưu tiên:

Linux Kernel v5.4.x

Architecture:

ARM64

Nếu source tree tôi cung cấp có version khác:

- sử dụng ĐÚNG version của source tree
- không được lấy code từ version khác rồi trình bày như code hiện tại
- nếu implementation thay đổi giữa các version, phải nói rõ

Nếu Chapter 10 sử dụng x86 code cũ như:

    int $0x80
    sysenter
    eax
    ebx
    ecx
    edx
    system_call

hãy giải thích đó là implementation/architecture của tài liệu cũ.

Sau đó mapping sang ARM64 Linux kernel hiện đại:

    svc #0
    x0-x5
    x8
    EL0
    EL1
    ELR_EL1
    SPSR_EL1
    ESR_EL1
    VBAR_EL1
    pt_regs
    eret

PHẢI phân biệt:

[BOOK]
    Nội dung được trình bày trong Chapter 10.

[KERNEL SOURCE]
    Implementation thực tế trong Linux kernel source.

[ARCH-SPECIFIC]
    Code phụ thuộc ARM64/x86.

[GENERIC KERNEL]
    Code dùng chung.

[INFERENCE]
    Suy luận của bạn, nếu có.

Không được trộn các category này.

============================================================
# 2. NGUYÊN TẮC NGHIÊN CỨU
============================================================

Đây là yêu cầu QUAN TRỌNG NHẤT.

KHÔNG giải thích Linux Kernel theo kiểu textbook chung chung.

Mỗi khi nói:

"Kernel kiểm tra pointer."

phải trả lời:

    Function nào?
    File nào?
    Caller nào?
    Callee nào?
    Source code nào?
    Data structure nào?
    CPU state nào?
    Context nào?

Ví dụ KHÔNG chấp nhận:

    "The kernel checks whether the pointer is valid."

Phải đi đến mức:

    access_ok()
        ↓
    function thực tế
        ↓
    architecture implementation
        ↓
    actual memory access
        ↓
    possible exception
        ↓
    recovery/fixup

Nếu source code có thể kiểm chứng thì KHÔNG được suy diễn.

Nếu không tìm thấy symbol/function:

    NOT FOUND IN THIS VERSION

Nếu concept đã thay đổi:

    CONCEPT CHANGED

Không được tự tạo function.

============================================================
# 3. OBJECTIVE
============================================================

Hãy xây dựng một nghiên cứu hoàn chỉnh:

# "Linux System Call — From Userspace to Kernel Source Code"

Tôi muốn sau khi đọc tài liệu này có thể tự mở Linux kernel source và trace:

    read()
    write()
    openat()
    close()

từ userspace đến kernel implementation và quay trở lại userspace.

============================================================
# 4. PHẦN A — CHAPTER 10 → MODERN LINUX
============================================================

Đầu tiên hãy phân tích toàn bộ Chapter 10.

Tạo bảng:

| Chapter 10 Concept |
| Modern Linux Concept |
| Linux v5.4 ARM64 Symbol |
| Source File |
| Status |
| Notes |

Mapping:

POSIX API
System Call
System Call Handler
System Call Service Routine
System Call Number
System Call Table
Parameter Passing
User Address Space
get_user()
put_user()
copy_from_user()
copy_to_user()
access_ok()
Page Fault
Exception Table
Fixup Code
Kernel Wrapper
System Call Return

Với mỗi concept:

1. Chapter 10 nói gì?
2. Linux v5.4 ARM64 làm gì?
3. Symbol nào?
4. Source file nào?
5. Concept còn tồn tại không?
6. Concept đã thay đổi không?
7. Nếu khác, tại sao?

============================================================
# 5. PHẦN B — BIG PICTURE
============================================================

Tạo execution flow hoàn chỉnh:

    User Application
          │
          ▼
    libc wrapper
          │
          ▼
    syscall ABI
          │
          ▼
    ARM64:
        svc #0
          │
          ▼
    CPU Exception
          │
          ▼
    EL0 → EL1
          │
          ▼
    ARM64 Exception Entry
          │
          ▼
    register save
          │
          ▼
    struct pt_regs
          │
          ▼
    syscall number
          │
          ▼
    syscall dispatch
          │
          ▼
    syscall wrapper
          │
          ▼
    syscall implementation
          │
          ▼
    VFS / subsystem
          │
          ▼
    filesystem / driver
          │
          ▼
    return value
          │
          ▼
    syscall return
          │
          ▼
    exception exit
          │
          ▼
    eret
          │
          ▼
    EL1 → EL0
          │
          ▼
    userspace

Đối với MỖI arrow:

    - symbol
    - source file
    - caller
    - callee
    - input
    - output
    - CPU register
    - execution context

============================================================
# 6. PHẦN C — ARM64 EXCEPTION ENTRY
============================================================

Nghiên cứu:

    svc #0

Từ instruction này hãy trace toàn bộ:

    userspace
       ↓
    CPU hardware
       ↓
    ELR_EL1
    SPSR_EL1
    ESR_EL1
    VBAR_EL1
       ↓
    exception vector
       ↓
    entry.S
       ↓
    kernel_entry
       ↓
    pt_regs
       ↓
    el0_svc
       ↓
    syscall dispatch

Phải phân biệt:

CPU HARDWARE BEHAVIOR

và

LINUX SOFTWARE BEHAVIOR

Giải thích:

    EL0
    EL1
    SP_EL0
    SP_EL1
    ELR_EL1
    SPSR_EL1
    ESR_EL1
    VBAR_EL1

============================================================
# 7. PHẦN D — pt_regs
============================================================

Phân tích chính xác:

    struct pt_regs

Giải thích:

    regs[]
    sp
    pc
    pstate
    orig_x0
    syscallno
    ...

Trace:

userspace:

    x0 = argument 1
    x1 = argument 2
    x2 = argument 3
    ...
    x8 = syscall number

        ↓

exception entry

        ↓

pt_regs

        ↓

syscall handler

Hãy chỉ ra:

    x0 → pt_regs field nào?
    x1 → field nào?
    x8 → field nào?

Và khi return:

    kernel return value
        ↓
    pt_regs
        ↓
    x0
        ↓
    eret
        ↓
    userspace

============================================================
# 8. PHẦN E — SYSCALL NUMBER + SYSCALL TABLE
============================================================

Trace chính xác:

    __NR_read
    __NR_write
    __NR_openat

Tìm:

    syscall number definition
    syscall table
    syscall dispatch
    generated code
    architecture-specific dispatch

Giải thích:

    x8
     ↓
    syscall number
     ↓
    dispatch
     ↓
    syscall function

Kiểm tra xem:

    sys_call_table

có thực sự được sử dụng trong ARM64 kernel v5.4 hay không.

Nếu không:

    giải thích implementation thực tế.

============================================================
# 9. PHẦN F — SYSCALL_DEFINE
============================================================

Phân tích cực kỳ sâu:

    SYSCALL_DEFINE3(read, ...)

Không chỉ nói macro dùng để định nghĩa syscall.

Hãy:

1. Tìm macro definition.
2. Expand macro.
3. Trace các macro trung gian.
4. Xác định function được compiler tạo ra.
5. Xác định wrapper.
6. Xác định actual implementation.
7. Giải thích argument type conversion.
8. Giải thích `__user`.
9. Giải thích `asmlinkage` nếu có.
10. Giải thích `pt_regs`.

Tạo:

    SYSCALL_DEFINE3()
          ↓
    macro expansion
          ↓
    generated wrapper
          ↓
    syscall implementation

Nếu flow thực tế khác:

    sửa lại theo source.

============================================================
# 10. PHẦN G — READ() END-TO-END
============================================================

Dùng:

    read(fd, buf, count)

làm CASE STUDY chính.

Trace từ:

    userspace
       ↓
    libc
       ↓
    svc #0
       ↓
    ARM64 entry
       ↓
    pt_regs
       ↓
    syscall dispatch
       ↓
    __arm64_sys_read
       ↓
    ksys_read
       ↓
    vfs_read
       ↓
    file_operations
       ↓
    filesystem
       ↓
    page cache
       ↓
    copy_to_user
       ↓
    return

Nếu source code v5.4 có flow khác:

    sử dụng flow thực tế.

Với mỗi function:

    prototype
    file
    caller
    callee
    parameters
    return value
    important structures
    locking
    reference counting
    error path

============================================================
# 11. PHẦN H — FILE DESCRIPTOR DATA STRUCTURES
============================================================

Trace:

    current
      ↓
    task_struct
      ↓
    files
      ↓
    files_struct
      ↓
    fdt
      ↓
    fdtable
      ↓
    fd
      ↓
    struct file
      ↓
    f_path
      ↓
    struct path
      ↓
    dentry
      ↓
    inode
      ↓
    super_block

Giải thích chính xác:

    struct task_struct
    struct files_struct
    struct fdtable
    struct file
    struct path
    struct dentry
    struct inode
    struct super_block
    struct address_space

Đặc biệt:

    fd
    struct file
    struct inode
    struct dentry

KHÔNG phải cùng một object.

Giải thích object lifetime.

Trace:

    f_count
    i_count

Tìm:

    get_file()
    fput()
    igrab()
    iput()

hoặc các mechanism tương ứng trong version thực tế.

============================================================
# 12. PHẦN I — OPENAT()
============================================================

Trace:

    openat()

End-to-end:

    userspace
       ↓
    syscall
       ↓
    syscall wrapper
       ↓
    do_sys_open()
       ↓
    pathname lookup
       ↓
    dentry
       ↓
    inode
       ↓
    struct file
       ↓
    fd allocation
       ↓
    fdtable
       ↓
    userspace receives fd

Tìm source thực tế.

Giải thích:

    file allocation
    fd allocation
    dentry lookup
    inode lookup
    path resolution
    reference counting

============================================================
# 13. PHẦN J — STRUCT FILE
============================================================

Phân tích:

    struct file

Đặc biệt:

    f_path
    f_inode
    f_op
    f_pos
    f_mapping
    f_count

Giải thích:

    f_path
       ↓
    path
       ↓
    dentry + vfsmount

    f_inode
       ↓
    inode

    f_op
       ↓
    file_operations

    f_mapping
       ↓
    address_space

Giải thích:

    f_mapping
    vs
    inode->i_mapping

============================================================
# 14. PHẦN K — VFS
============================================================

Trace:

    read()
       ↓
    vfs_read()
       ↓
    file_operations
       ↓
    filesystem/device implementation

Giải thích dynamic dispatch:

    file->f_op->read_iter

Tìm:

    nơi `f_op` được khởi tạo
    nơi function pointer được gán
    nơi function pointer được gọi

Không được chỉ nói:

    "VFS calls filesystem."

Phải chỉ ra:

    struct
    field
    assignment
    caller
    callee

============================================================
# 15. PHẦN L — USER ACCESS
============================================================

Nghiên cứu:

    access_ok()
    get_user()
    put_user()
    copy_from_user()
    copy_to_user()

Trace source code thực tế.

Giải thích tại sao kernel không được:

    memcpy(kernel_buf, user_ptr, size);

Phân tích:

    __user
    access_ok
    raw_copy_from_user
    raw_copy_to_user
    architecture implementation

Nếu function đã thay đổi trong v5.4:

    dùng implementation thực tế.

============================================================
# 16. PHẦN M — PAGE FAULT
============================================================

Phân tích trường hợp:

    copy_from_user()
         ↓
    user pointer
         ↓
    unmapped page
         ↓
    memory access fault

Trace:

    CPU
      ↓
    ARM64 exception
      ↓
    page fault handling
      ↓
    memory management
      ↓
    uaccess recovery

Giải thích:

    mapped page
    unmapped page
    permission fault
    translation fault

Phân biệt:

    address validation

với:

    actual memory access

============================================================
# 17. PHẦN N — EXCEPTION TABLE + FIXUP
============================================================

Đây là phần cần nghiên cứu cực sâu.

Trace:

    user memory access
         ↓
    faulting instruction
         ↓
    exception
         ↓
    exception table
         ↓
    fixup
         ↓
    recovery
         ↓
    return error

Tìm:

    __ex_table
    exception table entry
    search_exception_tables()
    fixup
    uaccess exception handling

Nếu ARM64 implementation khác x86 trong Chapter 10:

    giải thích sự khác biệt.

Giải thích:

    .fixup
    __ex_table
    linker sections
    exception table generation
    lookup
    fixup address
    instruction address

============================================================
# 18. PHẦN O — SYSCALL RETURN
============================================================

Trace:

    syscall implementation
        ↓
    return value
        ↓
    pt_regs
        ↓
    x0
        ↓
    kernel exit
        ↓
    eret
        ↓
    userspace
        ↓
    libc
        ↓
    errno

Giải thích:

    negative errno

và:

    errno

Không được nói kernel trực tiếp set userspace `errno` nếu source không làm như vậy.

============================================================
# 19. PHẦN P — ERROR PATH
============================================================

Đối với:

    read()
    write()
    openat()

trace các error path quan trọng.

Ví dụ:

    invalid fd
    invalid user pointer
    permission error
    interrupted syscall
    page fault
    filesystem error

Tạo:

    SUCCESS PATH

và:

    ERROR PATH

============================================================
# 20. PHẦN Q — LOCKING
============================================================

Trong quá trình trace:

Tìm:

    mutex
    spinlock
    rwlock
    atomic
    RCU

Với mỗi lock:

    lock nào?
    protect object nào?
    acquire ở đâu?
    release ở đâu?
    tại sao cần?

Không bỏ qua concurrency.

============================================================
# 21. PHẦN R — REFERENCE COUNTING
============================================================

Trace lifetime:

    task
    files_struct
    fdtable
    file
    inode
    dentry

Đặc biệt:

    f_count
    i_count
    dentry reference

Tìm:

    get
    put
    grab
    release

Giải thích:

    object created
        ↓
    reference acquired
        ↓
    reference transferred
        ↓
    reference released
        ↓
    object freed

============================================================
# 22. PHẦN S — SOURCE CODE ARCHAEOLOGY
============================================================

Đối với mỗi symbol quan trọng, hãy tạo:

## Definition

    Symbol:
    File:
    Prototype:

## Callers

    caller
       ↓
    target

## Callees

    target
       ├── function A
       ├── function B
       └── function C

## Data Flow

    argument
       ↓
    variable
       ↓
    structure field
       ↓
    next function

## Control Flow

    condition
       ├── success
       └── error

## Context

    process context?
    interrupt context?
    softirq?
    preemptible?

## Locking

## Reference Counting

## Error Path

## Return Path

============================================================
# 23. PHẦN T — SOURCE CODE EVIDENCE
============================================================

Mọi kết luận quan trọng phải có evidence.

Format:

    Claim:
    ...

    Source:
    path/to/file.c

    Symbol:
    function_name()

    Relevant code:
    ...

    Explanation:
    ...

Không quote source code quá dài.

Chỉ quote block code cần thiết để chứng minh.

Nếu không chắc:

    SAY THAT YOU ARE NOT CERTAIN.

Không hallucinate source code.

============================================================
# 24. PHẦN U — DIAGRAMS
============================================================

Tạo Mermaid diagrams.

Bắt buộc có:

1. System Call Sequence Diagram

2. ARM64 Exception Entry Diagram

3. Syscall Dispatch Diagram

4. read() Call Graph

5. openat() Call Graph

6. File Descriptor Object Relationship

7. User Access / Page Fault Diagram

8. Exception Table / Fixup Diagram

9. Syscall Return Diagram

10. Complete End-to-End Diagram

Ví dụ:

sequenceDiagram
    participant U as Userspace
    participant CPU
    participant ENTRY as ARM64 Entry
    participant SC as Syscall Dispatcher
    participant VFS
    participant FS

    U->>CPU: svc #0
    CPU->>ENTRY: Exception Entry
    ENTRY->>SC: pt_regs
    SC->>VFS: syscall
    VFS->>FS: filesystem operation
    FS-->>VFS: result
    VFS-->>SC: return
    SC-->>ENTRY: x0
    ENTRY-->>CPU: eret
    CPU-->>U: return

NHƯNG:

Không được coi diagram trên là sự thật.

Hãy sửa nó theo source code thực tế.

============================================================
# 25. PHẦN V — REGISTER TIMELINE
============================================================

Tạo timeline cho:

    read(fd, buf, count)

Ví dụ format:

BEFORE SVC:

    EL0
    x0 = ...
    x1 = ...
    x2 = ...
    x8 = ...
    PC = ...

AFTER EXCEPTION:

    EL1
    ELR_EL1 = ...
    SPSR_EL1 = ...
    ESR_EL1 = ...
    SP = ...

PT_REGS:

    regs[0] = ...
    regs[1] = ...
    regs[2] = ...

SYSCALL:

    syscall number = ...

RETURN:

    x0 = return value

AFTER ERET:

    EL0
    PC = ...
    x0 = ...

Mỗi giá trị phải giải thích nguồn gốc.

============================================================
# 26. PHẦN W — STRUCTURE MAP
============================================================

Tạo một bảng lớn:

| Structure | Field | Points to | Created by | Lifetime | Refcount | Used by |
|-----------|-------|-----------|------------|----------|----------|---------|

Bao gồm:

task_struct
files_struct
fdtable
file
path
dentry
inode
super_block
address_space
file_operations
inode_operations

============================================================
# 27. PHẦN X — BOOK vs SOURCE
============================================================

Cuối cùng tạo bảng:

| Chapter 10 | Linux v5.4 ARM64 | Difference |
|------------|------------------|------------|

Đặc biệt đánh dấu:

    OBSOLETE
    CHANGED
    STILL VALID
    X86 ONLY
    ARM64 EQUIVALENT

============================================================
# 28. PHẦN Y — FINAL KNOWLEDGE MAP
============================================================

Tạo một knowledge map:

                    SYSTEM CALL
                         │
          ┌──────────────┼──────────────┐
          ↓              ↓              ↓
      CPU/ARM64      SYSCALL ABI       VFS
          │              │              │
          ↓              ↓              ↓
     SVC/ERET        pt_regs          file
          │              │              │
          ↓              ↓              ↓
     EL0/EL1          x0-x8         dentry/inode
          │              │              │
          └──────────────┼──────────────┘
                         ↓
                    USER ACCESS
                         │
                         ↓
                  copy_from_user
                         │
                         ↓
                    PAGE FAULT
                         │
                         ↓
                  EXCEPTION TABLE
                         │
                         ↓
                      FIXUP

============================================================
# 29. PHẦN Z — WHAT TO READ NEXT
============================================================

Sau khi hoàn thành:

Đưa ra roadmap source code tiếp theo.

Ví dụ:

    syscall
      ↓
    VFS
      ↓
    page cache
      ↓
    MM
      ↓
    filesystem
      ↓
    block layer
      ↓
    driver

Đối với mỗi topic:

    Function cần đọc
    File cần đọc
    Struct cần đọc
    Tại sao cần đọc
    Kiến thức prerequisite

============================================================
# 30. FINAL RULE
============================================================

Tôi muốn bạn ưu tiên:

SOURCE CODE
    >
CALL GRAPH
    >
DATA STRUCTURE
    >
CPU/REGISTER
    >
EXECUTION FLOW
    >
THEORY

Không được làm ngược lại.

Nếu một câu trả lời có thể được chứng minh bằng source code,
hãy chứng minh bằng source code.

Nếu Chapter 10 nói một điều nhưng Linux v5.4 ARM64 đã thay đổi,
hãy nói rõ:

    "Chapter 10 describes X,
     but Linux v5.4 ARM64 implements this as Y."

Không được âm thầm thay thế nội dung của sách.

MỤC TIÊU CUỐI CÙNG:

Sau khi hoàn thành nghiên cứu này, tôi muốn có khả năng tự mở Linux kernel source và trả lời câu hỏi:

"Process gọi:

    read(3, buffer, 100);

CPU làm gì?

Kernel entry ở đâu?

Register nào chứa gì?

pt_regs ở đâu?

Syscall number được lấy thế nào?

Syscall được dispatch thế nào?

Function nào thực sự chạy?

fd = 3 được chuyển thành struct file* thế nào?

struct file liên kết với inode/dentry thế nào?

VFS gọi filesystem thế nào?

Kernel truy cập buffer userspace thế nào?

Nếu buffer invalid thì chuyện gì xảy ra?

Exception table/fixup hoạt động thế nào?

Return value đi từ kernel về userspace thế nào?

CPU quay về EL0 bằng cách nào?"

Hãy xây dựng toàn bộ tài liệu nghiên cứu để tôi có thể tự trace được câu trả lời cho câu hỏi này bằng Linux kernel source code.