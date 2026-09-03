# ROADMAP TỔNG THỂ

```
MODULE 0
Linux Process Mental Model
        │
        ▼
MODULE 1
task_struct + thread_info
        │
        ▼
MODULE 2
Process Memory Layout
        │
        ▼
MODULE 3
System Call Entry ARM64
        │
        ▼
MODULE 4
fork()
        │
        ▼
MODULE 5
copy_process()
        │
        ▼
MODULE 6
dup_task_struct()
        │
        ▼
MODULE 7
copy_mm() + Copy-On-Write
        │
        ▼
MODULE 8
copy_thread() ARM64
        │
        ▼
MODULE 9
ret_from_fork
        │
        ▼
MODULE 10
execve()
        │
        ▼
MODULE 11
ELF Loader
        │
        ▼
MODULE 12
Scheduler
        │
        ▼
MODULE 13
Context Switch ARM64
        │
        ▼
MODULE 14
User ↔ Kernel Transition
        │
        ▼
MODULE 15
Signal
        │
        ▼
MODULE 16
exit()
        │
        ▼
MODULE 17
wait()
        │
        ▼
MODULE 18
PID / Process Tree
        │
        ▼
MODULE 19
Threads / clone()
        │
        ▼
MODULE 20
Namespaces
        │
        ▼
MODULE 21
Debug thực tế
```

---

# MODULE 0 — PROCESS MENTAL MODEL

### Prompt

```
Bạn là chuyên gia Linux Kernel 5.4 ARM64.

Hãy dạy tôi kiến trúc tổng thể của một process trong Linux kernel 5.4 trên ARM64.

Tôi muốn hiểu process từ thời điểm:
ELF executable → execve() → user mode → syscall/interrupt → kernel mode
→ scheduler → context switch → signal → exit() → wait().

Hãy vẽ luồng tổng thể trước, sau đó giải thích từng subsystem.

Yêu cầu:
- Phân biệt process và thread trong Linux
- Giải thích vì sao kernel dùng task_struct cho cả process và thread
- Chỉ rõ các file source kernel 5.4 liên quan
- Chỉ rõ phần generic kernel và phần ARM64-specific
- Không bỏ qua stack, mm_struct, pt_regs và thread_struct
```

---

# MODULE 1 — `task_struct`

Đây là module quan trọng nhất.

### Prompt

```
Hãy dạy tôi struct task_struct trong Linux kernel 5.4 từ mức beginner đến kernel developer.

Tôi muốn phân tích task_struct theo từng subsystem:

1. Scheduling
2. Process state
3. PID
4. Parent/child relationship
5. Memory management
6. File descriptors
7. Credentials
8. Signals
9. Thread architecture-specific state
10. Kernel stack

Với mỗi field quan trọng:
- Nó dùng để làm gì
- Ai ghi
- Ai đọc
- Nó thay đổi khi nào
- Liên hệ với subsystem nào

Cuối cùng hãy vẽ sơ đồ:

task_struct
 ├── sched_entity
 ├── mm_struct
 ├── files_struct
 ├── fs_struct
 ├── signal_struct
 ├── sighand_struct
 ├── cred
 ├── thread_struct
 ├── pid
 └── kernel stack

Dùng Linux kernel 5.4 ARM64.
```

---

# MODULE 2 — PROCESS MEMORY LAYOUT ARM64

### Prompt

```
Hãy giải thích memory layout của một user process trên Linux kernel 5.4 ARM64.

Bắt đầu từ virtual address space:

User space
    ELF
    .text
    .rodata
    .data
    .bss
    heap
    mmap area
    shared libraries
    vDSO
    stack

Sau đó giải thích kernel space.

Tôi muốn hiểu:
- mm_struct
- vm_area_struct
- pgd/pud/pmd/pte
- ARM64 page table
- TTBR0_EL1
- TTBR1_EL1
- ASID
- TLB

Hãy liên hệ toàn bộ với một process thực tế.
```

---

# MODULE 3 — USER MODE → KERNEL MODE

ARM64-specific.

### Prompt

```
Hãy trace chi tiết một syscall trên Linux kernel 5.4 ARM64.

Ví dụ user gọi:

getpid()

Trace từ:

EL0 user mode
    ↓
SVC instruction
    ↓
exception vector
    ↓
arch/arm64/kernel/entry.S
    ↓
save registers vào pt_regs
    ↓
syscall dispatch
    ↓
sys_getpid()
    ↓
return to user mode

Tôi muốn:
- Register x0-x30
- SP_EL0
- SP_EL1
- ELR_EL1
- SPSR_EL1
- ESR_EL1
- pt_regs
- exception vector table

Giải thích từng bước và vẽ stack/register state trước và sau syscall.
```

---

# MODULE 4 — `fork()` END-TO-END

Đây nên là một module lớn riêng.

### Prompt

```
Hãy trace toàn bộ fork() trong Linux kernel 5.4 ARM64.

Bắt đầu từ user program:

pid = fork();

Trace chính xác:

user mode
 → syscall entry ARM64
 → sys_fork
 → _do_fork
 → copy_process
 → copy_thread
 → wake_up_new_task
 → scheduler
 → context switch
 → ret_from_fork
 → child return về user mode

Hãy giải thích:
- Parent thấy return value gì
- Child thấy return value gì
- Register state của parent/child khác nhau thế nào
- Stack khác nhau thế nào
- mm_struct được copy thế nào
- COW được thiết lập thế nào

Dùng source Linux kernel 5.4 ARM64.
```

`kernel/fork.c` là trung tâm của việc tạo task trong nhánh 5.4, bao gồm `_do_fork()` và các syscall wrapper như `fork`, `vfork`, `clone`.

---

# MODULE 5 — `copy_process()`

### Prompt

```
Hãy phân tích copy_process() trong kernel/fork.c của Linux 5.4.

Đi từng bước source code.

Tôi muốn hiểu:

copy_process()
 ├── dup_task_struct()
 ├── copy_creds()
 ├── copy_semundo()
 ├── copy_files()
 ├── copy_fs()
 ├── copy_sighand()
 ├── copy_signal()
 ├── copy_mm()
 ├── copy_namespaces()
 ├── copy_io()
 ├── copy_thread()
 └── pid allocation

Với mỗi bước hãy giải thích:
- clone flag nào ảnh hưởng
- object nào được copy
- object nào được share
- reference count nào tăng
- object nào chỉ được tạo lazy

Sau đó so sánh:
fork()
vfork()
clone(CLONE_VM)
clone(CLONE_THREAD)
```

---

# MODULE 6 — KERNEL STACK VÀ `dup_task_struct()`

### Prompt

```
Hãy dạy tôi cơ chế tạo task_struct và kernel stack trong Linux kernel 5.4 ARM64.

Trace:

dup_task_struct()
    ↓
alloc_thread_stack_node()
    ↓
thread_info
    ↓
kernel stack

Giải thích:

task_struct
thread_info
kernel stack

chúng nằm ở đâu trong memory.

Sau đó giải thích khi process:
- đang chạy user mode
- syscall vào kernel
- interrupt xảy ra
- context switch
- process sleep

SP thay đổi như thế nào.

Vẽ memory layout chính xác.
```

---

# MODULE 7 — `fork()` VÀ COPY-ON-WRITE

### Prompt

```
Hãy giải thích Copy-On-Write trong fork() của Linux kernel 5.4 ARM64 từ source code.

Trace:

fork()
 → copy_mm()
 → dup_mm()
 → dup_mmap()
 → copy_page_range()
 → page table duplication
 → write-protect parent
 → write-protect child
 → COW page fault
 → do_wp_page()
 → copy page
 → update PTE
 → TLB maintenance

Tôi muốn hiểu bằng một ví dụ:

Parent:
virtual address 0x...

Child:
virtual address 0x...

Trước fork:
Sau fork:
Sau parent write:
Sau child write:

Vẽ page table và physical page ở từng bước.
```

---

# MODULE 8 — `copy_thread()` TRÊN ARM64

Đây là một trong những phần thú vị nhất.

### Prompt

```
Hãy phân tích copy_thread() của Linux kernel 5.4 ARM64.

Tôi muốn hiểu cách kernel tạo execution context cho child.

Giải thích:

struct pt_regs
struct cpu_context
struct thread_struct

Parent register state
        ↓
copy_thread()
        ↓
Child register state

Hãy chỉ rõ:
- x0
- x19-x30
- SP
- PC
- PSTATE
- TLS
- stack

Vì sao child sau fork() có thể bắt đầu từ ret_from_fork?

Hãy trace bằng register diagram.
```

Ở ARM64, `ret_from_fork` là entry point cho task mới sau khi context switch tới nó; sau `schedule_tail`, kernel thread có thể gọi hàm kernel riêng, còn user task đi theo đường return-to-user.

---

# MODULE 9 — `ret_from_fork`

### Prompt

```
Hãy phân tích ret_from_fork trong Linux kernel 5.4 ARM64.

Trace:

new child task
   ↓
scheduler chọn child
   ↓
context_switch()
   ↓
switch_to()
   ↓
cpu_switch_to()
   ↓
PC = ret_from_fork
   ↓
schedule_tail()
   ↓
ret_to_user
   ↓
eret

Giải thích vì sao child process bắt đầu chạy từ ret_from_fork thay vì chạy trực tiếp từ instruction sau fork().

Sau đó giải thích kernel làm thế nào để cuối cùng child quay lại đúng instruction trong user space.
```

---

# MODULE 10 — `execve()`

### Prompt

```
Hãy trace execve() trong Linux kernel 5.4 ARM64 từ đầu đến cuối.

Bắt đầu:

execve("/bin/program", argv, envp)

Trace:

sys_execve
 → do_execve
 → do_execveat_common
 → prepare_bprm_creds
 → open_exec
 → search_binary_handler
 → load_elf_binary
 → setup_new_exec
 → flush_old_exec
 → setup_arg_pages
 → ELF PT_LOAD mapping
 → start_thread
 → return to user

Giải thích chính xác object nào của process bị thay thế và object nào giữ nguyên.

Đặc biệt giải thích:
- PID
- task_struct
- mm_struct
- files
- cwd
- credentials
- signals
- kernel stack
```

---

# MODULE 11 — ELF LOADER ARM64

### Prompt

```
Hãy giải thích cách Linux kernel 5.4 ARM64 load ELF executable.

Dùng một ELF PIE ví dụ.

Trace:

execve()
 → read ELF header
 → PT_INTERP
 → ELF interpreter
 → PT_LOAD
 → mmap
 → load_bias
 → ASLR
 → setup stack
 → argc
 → argv
 → envp
 → auxv
 → AT_PHDR
 → AT_ENTRY
 → AT_RANDOM
 → AT_SYSINFO_EHDR

Sau đó giải thích:

start_thread()

trên ARM64 set:
- PC
- SP
- PSTATE

như thế nào để process bắt đầu chạy ở EL0.
```

---

# MODULE 12 — SCHEDULER

### Prompt

```
Hãy dạy Linux scheduler trong kernel 5.4 với focus là process ARM64.

Bắt đầu từ:

TASK_RUNNING
TASK_INTERRUPTIBLE
TASK_UNINTERRUPTIBLE

Sau đó:

wake_up_process()
schedule()
__schedule()
pick_next_task()
context_switch()

Giải thích:

task_struct
sched_entity
cfs_rq
rq
vruntime
rb-tree

Và trace một process:

Running
 → sleep()
 → wakeup
 → runnable
 → scheduler select
 → context switch
 → running
```

---

# MODULE 13 — CONTEXT SWITCH ARM64

Đây là module rất quan trọng.

### Prompt

```
Hãy trace context switch trong Linux kernel 5.4 ARM64.

Trace:

schedule()
 → __schedule()
 → context_switch()
 → switch_mm_irqs_off()
 → switch_to()
 → __switch_to()
 → cpu_switch_to()

Giải thích chính xác:

Generic scheduler làm gì?
ARM64 process.c làm gì?
ARM64 assembly làm gì?

Đặc biệt phân tích:

cpu_switch_to(prev, next)

Register nào được save?

x19
x20
...
x29
x30
SP

Register nào không được save ở đây và tại sao?

Sau đó vẽ:

Task A context
        ↓
save
        ↓
Task B context
        ↓
restore
```

Trong ARM64, `cpu_switch_to()` lưu các callee-saved registers `x19`–`x29`, stack pointer và link register vào context của task cũ, rồi khôi phục context của task mới; sau đó `sp_el0` được cập nhật để phản ánh task hiện tại.

---

# MODULE 14 — CONTEXT SWITCH VÀ MMU

### Prompt

```
Hãy giải thích context switch giữa hai process khác address space trên Linux 5.4 ARM64.

Trace:

Task A
 mm = MM_A
 TTBR0 = PGD_A
 ASID = A

Task B
 mm = MM_B
 TTBR0 = PGD_B
 ASID = B

Khi switch:

switch_mm()
 → check_and_switch_context()
 → cpu_switch_mm()
 → TTBR0_EL1
 → ASID
 → TLB

Giải thích tại sao Linux dùng ASID.

So sánh:

switch thread cùng process

vs

switch process khác mm_struct.

Cái nào tốn nhiều chi phí hơn và tại sao?
```

---

# MODULE 15 — INTERRUPT VÀ PROCESS

### Prompt

```
Hãy giải thích điều gì xảy ra khi một process đang chạy ở ARM64 EL0 và một hardware interrupt xảy ra.

Trace:

EL0
 → IRQ
 → exception vector
 → EL1
 → save pt_regs
 → IRQ handler
 → scheduler check
 → context switch có thể xảy ra
 → return

Đặc biệt giải thích:

Một process có thể quay về user space khác process đã bị interrupt hay không?

Ví dụ:

Process A running
IRQ xảy ra
scheduler chạy
Process B được chọn

CPU return về đâu?

Dùng ARM64 kernel 5.4.
```

---

# MODULE 16 — SIGNAL

### Prompt

```
Hãy dạy signal delivery trong Linux kernel 5.4 ARM64.

Trace:

kill()
 → send_signal()
 → pending signal
 → signal_struct
 → sighand_struct
 → task signal pending
 → return to user check
 → do_signal()
 → setup signal frame ARM64
 → user signal handler
 → sigreturn
 → restore pt_regs

Tôi muốn hiểu signal frame nằm ở đâu và register ARM64 được restore như thế nào.
```

---

# MODULE 17 — `exit()`

### Prompt

```
Hãy trace process exit trong Linux kernel 5.4.

Trace:

exit()
 → do_exit()
 → exit_signals()
 → exit_mm()
 → exit_files()
 → exit_fs()
 → exit_thread()
 → release_task()
 → TASK_DEAD

Giải thích:

Zombie process là gì?

Tại sao process đã exit nhưng task_struct/PID vẫn có thể tồn tại?

Parent nhận SIGCHLD như thế nào?
```

---

# MODULE 18 — `wait()` VÀ ZOMBIE

### Prompt

```
Hãy giải thích toàn bộ cơ chế parent-child và wait() trong Linux kernel 5.4.

Trace:

Child exit
 → zombie
 → parent SIGCHLD
 → wait4()
 → do_wait()
 → wait_consider_task()
 → release_task()

Giải thích:

TASK_DEAD
EXIT_ZOMBIE
EXIT_DEAD

Khác nhau như thế nào?

Vẽ lifetime của process:

fork
 → running
 → exit
 → zombie
 → wait
 → task_struct freed
```

---

# MODULE 19 — THREAD VÀ `clone()`

### Prompt

```
Hãy giải thích thread implementation của Linux kernel 5.4.

Tại sao Linux nói:

"Every thread is a task_struct"

So sánh:

fork()

clone(CLONE_VM)
clone(CLONE_THREAD)
pthread_create()

Bằng bảng:

Resource          Process A    Thread B
mm_struct
files_struct
fs_struct
signal_struct
sighand_struct
PID
TGID
kernel stack
thread_struct

Sau đó trace pthread_create() đến clone syscall trên ARM64.
```

---

# MODULE 20 — PID / TGID

### Prompt

```
Hãy giải thích PID implementation trong Linux kernel 5.4.

Tôi muốn hiểu:

struct pid
struct pid_namespace
upid
PIDTYPE_PID
PIDTYPE_TGID
PIDTYPE_PGID
PIDTYPE_SID

Phân biệt:

PID
TGID
TID

Và giải thích vì sao:

getpid()

trong multithread process trả về TGID thay vì thread ID.
```

---

# MODULE 21 — PROCESS TREE

### Prompt

```
Hãy giải thích process hierarchy trong Linux kernel 5.4.

Phân tích:

task_struct
 ├── parent
 ├── real_parent
 ├── children
 ├── sibling
 └── ptraced

Trace:

init
 └── bash
      └── program
           ├── child1
           └── child2

Sau đó giải thích orphan process.

Điều gì xảy ra khi parent exit trước child?

Linux kernel 5.4 reparent child như thế nào?
```

---

# MODULE 22 — KERNEL THREAD

### Prompt

```
Hãy giải thích kernel thread trong Linux kernel 5.4 ARM64.

Trace:

kthread_create()
 → kernel_clone
 → copy_process()
 → copy_thread()

Sau đó:

scheduler
 → cpu_switch_to
 → ret_from_fork
 → kernel thread function

So sánh kernel thread với user process:

User process:
ret_from_fork → return to EL0

Kernel thread:
ret_from_fork → gọi kernel function

Giải thích tại sao kernel thread không có user address space bình thường.
```

`ret_from_fork` trên ARM64 phân biệt kernel thread với user task dựa trên context đã được chuẩn bị; đường đi này được thiết kế để task mới hoàn tất phần scheduler trước khi chạy hàm kernel hoặc quay về user mode.

---

# MODULE 23 — `vfork()`

### Prompt

```
Hãy phân tích vfork() trong Linux kernel 5.4.

So sánh:

fork()
vfork()

Đặc biệt giải thích:

CLONE_VM
CLONE_VFORK

Tại sao parent bị block?

Child được phép làm gì?

Điều gì xảy ra khi child:
execve()
exit()

Vẽ timeline parent/child.
```

---

# MODULE 24 — CREDENTIALS

### Prompt

```
Hãy giải thích credentials của process trong Linux kernel 5.4.

Phân tích:

struct cred

UID
EUID
SUID
FSUID

GID
EGID
SGID
FSGID

capabilities

Và:

current_cred()
prepare_creds()
commit_creds()

Trace credential lifecycle qua:

fork()
execve()
setuid()
```

---

# MODULE 25 — FILE DESCRIPTOR

### Prompt

```
Hãy trace file descriptor subsystem của một process Linux kernel 5.4.

Từ:

task_struct
 → files_struct
 → fdtable
 → struct file
 → inode
 → super_block

Sau đó trace:

fork()

copy_files()

và giải thích parent/child share struct file như thế nào.

Ví dụ:

fd = open()

fork()

Parent write
Child write

File offset thay đổi như thế nào?
```

---

# MODULE 26 — CURRENT TASK ARM64

Đây là phần rất hay nếu bạn học sâu architecture.

### Prompt

```
Hãy giải thích Linux kernel 5.4 ARM64 lấy current task như thế nào.

Tôi muốn hiểu:

current
current_thread_info()

TPIDR_EL1
SP_EL0
thread_info

Vì sao ARM64 có thể truy cập current task nhanh?

Sau đó trace khi context switch:

Task A
 ↓
cpu_switch_to()
 ↓
Task B

Giá trị nào thay đổi để current trỏ tới Task B?
```

---

# MODULE 27 — `pt_regs`

### Prompt

```
Hãy phân tích struct pt_regs trên Linux kernel 5.4 ARM64.

Giải thích:

regs[0..30]
sp
pc
pstate

Nó được tạo khi nào?

So sánh:

- syscall từ EL0
- IRQ từ EL0
- exception từ EL0
- syscall từ kernel
- IRQ khi kernel đang chạy

Sau đó giải thích:

fork child

và

execve

thay đổi pt_regs như thế nào.
```

---

# MODULE 28 — RETURN TO USER MODE

### Prompt

```
Hãy trace đường đi kernel → user mode trên Linux kernel 5.4 ARM64.

Bắt đầu từ:

ret_to_user

Sau đó giải thích các việc kernel phải kiểm tra trước khi ERET:

- pending signals
- reschedule
- tracing
- single-step
- work pending

Cuối cùng:

restore registers
 → ERET
 → EL0

Giải thích ELR_EL1 và SPSR_EL1 ảnh hưởng thế nào đến nơi CPU quay lại.
```

---

# MODULE 29 — `execve()` SAU `fork()`

### Prompt

```
Hãy trace shell chạy một command:

bash
    |
    fork()
    |
    +---- child
           |
           execve("/bin/ls")

Hãy trace:

Parent bash
Child bash copy
Child execve
ELF /bin/ls loaded

Phân tích chính xác:

task_struct
mm_struct
PID
file descriptor
COW memory
register state

thay đổi ở từng bước.
```

---

# MODULE 30 — CASE STUDY HOÀN CHỈNH

Đây là prompt mình khuyên bạn dùng cuối cùng.

```
Hãy làm một case study cực sâu về lifecycle của một process trên Linux kernel 5.4 ARM64.

Ví dụ:

int main()
{
    pid_t pid = fork();

    if (pid == 0) {
        execve("/bin/program", ...);
    }

    wait(NULL);
}

Trace toàn bộ lifecycle:

1. bash execve program
2. ELF loader
3. user main()
4. fork syscall
5. copy_process()
6. copy_mm()
7. COW
8. copy_thread()
9. scheduler
10. context switch ARM64
11. ret_from_fork
12. child return
13. execve
14. new ELF
15. child exit
16. SIGCHLD
17. parent wait
18. zombie cleanup

Yêu cầu:
- Linux kernel 5.4
- ARM64
- chỉ rõ source file và function
- vẽ ASCII diagram
- giải thích register
- giải thích page table
- giải thích kernel stack
- không bỏ qua bất kỳ transition quan trọng nào
```

---

# THỨ TỰ HỌC TỐI ƯU

Nếu bạn muốn đi đến mức **đọc source Linux Kernel**, mình đề xuất:

```
PHASE 1 — Foundation
0 → 1 → 2 → 26 → 27

PHASE 2 — ARM64 Entry/Exit
3 → 14 → 28

PHASE 3 — Process Creation
4 → 5 → 6 → 7 → 8 → 9

PHASE 4 — Scheduling
12 → 13 → 14

PHASE 5 — Program Execution
10 → 11 → 29

PHASE 6 — Process Lifecycle
15 → 16 → 17 → 18

PHASE 7 — Advanced
19 → 20 → 21 → 22 → 23 → 24 → 25

PHASE 8 — Master Level
30
```

---

## Nếu học theo kiểu đọc source trực tiếp

Các file bạn nên tập trung trước tiên:

```
kernel/
├── fork.c
├── exit.c
├── sched/
│   ├── core.c
│   └── fair.c
├── signal.c
└── pid.c

fs/
├── exec.c
└── binfmt_elf.c

mm/
├── memory.c
├── mmap.c
└── mprotect.c

arch/arm64/
├── kernel/
│   ├── entry.S
│   ├── process.c
│   └── sys.c
├── include/asm/
│   ├── processor.h
│   ├── ptrace.h
│   └── mmu_context.h
└── mm/
```