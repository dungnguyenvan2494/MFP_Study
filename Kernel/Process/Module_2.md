# `struct task_struct` — Linux 5.4 / ARM64

## 0. Định vị trước khi vào chi tiết

| Câu hỏi            | Trả lời                                                                                                                                                                                                                                                |
| ------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Định nghĩa ở đâu   | `include/linux/sched.h` (thân struct ~ dòng 640–1300). Kiểu `thread_struct` ở `arch/arm64/include/asm/processor.h`, `thread_info` ở `arch/arm64/include/asm/thread_info.h`.                                                                            |
| Là gì              | Cấu trúc mô tả **một task** = đơn vị lập lịch. "Process" và "thread" POSIX đều là task_struct.                                                                                                                                                         |
| Kích thước         | ~7–9 KB (tùy config). Cấp phát từ slab riêng `task_struct_cachep` (`kmem_cache`), kiến trúc-riêng qua `alloc_task_struct_node()`.                                                                                                                      |
| Cấp phát           | `dup_task_struct()` trong `kernel/fork.c` (gọi từ `copy_process()`): `alloc_task_struct_node()` + `arch_dup_task_struct()` (memcpy từ cha) + `alloc_thread_stack_node()`.                                                                              |
| Giải phóng         | `free_task()` (`kernel/fork.c`) khi `refcount` (`usage`) về 0: `free_thread_stack()` + `free_task_struct()`. Trì hoãn qua RCU (`put_task_struct_rcu_user`).                                                                                            |
| Truy cập           | Macro `current` → ARM64: `read_sysreg(sp_el0)`. `current` luôn trỏ task đang chạy trên CPU này.                                                                                                                                                        |
| Ràng buộc bố cục   | `struct thread_info thread_info` phải là **field đầu tiên** (arm64 `CONFIG_THREAD_INFO_IN_TASK`). `struct thread_struct thread` phải là **field cuối cùng** (vì `thread` có phần variable-size như FPSIMD/SVE và các macro offset giả định nó ở cuối). |
| Danh sách toàn cục | Mọi task nằm trong danh sách liên kết đôi qua `task->tasks`, đầu là `init_task`. Duyệt bằng `for_each_process()`, `for_each_process_thread()`.                                                                                                         |

Ba mức đọc struct này:

- **Beginner**: biết `state`, `pid`, `mm`, `files`, `comm`, `parent` — đủ đọc `ps`, `/proc`.
- **Intermediate**: hiểu `se.vruntime`, `sighand` vs `signal`, `real_cred` vs `cred`, `mm` vs `active_mm`.
- **Kernel dev**: biết **ai** cầm lock nào khi ghi field, thứ tự khóa (`tasklist_lock` → `siglock` → `rq->lock`), vòng đời refcount, và vì sao field ở đúng cacheline đó.

---

## 1. Scheduling

|Field|Công dụng|Ghi bởi|Đọc bởi|Đổi khi nào|Liên hệ|
|---|---|---|---|---|---|
|`struct sched_entity se`|Nút của task trong CFS: `vruntime`, `load`, `run_node` (nút cây đỏ-đen của `cfs_rq`), `sum_exec_runtime`|`kernel/sched/fair.c` (`update_curr`, `enqueue_entity`, `__enqueue_entity`) dưới `rq->lock`|`pick_next_task_fair()`, `task_tick_fair()`, `/proc/<pid>/sched`|Mỗi tick, mỗi lần enqueue/dequeue, mỗi lần task ngủ/dậy|**Scheduler (CFS)**|
|`int prio`|Độ ưu tiên **động hiệu lực** (0–139; <100 = RT). Scheduler chọn theo cái này|`sched/core.c` (`set_user_nice`, `rt_mutex_setprio` — PI boost), `effective_prio()`|`__schedule`, `check_preempt_curr`, `plist` của RT rq|Khi `nice()`/`setpriority()`, khi priority-inheritance boost/deboost, khi đổi policy|Scheduler, **rt_mutex (PI)**|
|`int static_prio`|Từ `nice` value: `static_prio = MAX_RT_PRIO + nice + 20`. Không đổi vì PI|`set_user_nice()` (giữ `rq->lock` + `p->pi_lock`)|`set_load_weight()`, `effective_prio()`|Chỉ khi `nice()`/`sched_setattr()`|Scheduler|
|`int normal_prio`|Prio "đáng ra" theo policy+static_prio, bỏ qua PI|`normal_prio()` khi `__setscheduler`|`effective_prio()`|Khi đổi policy/nice|Scheduler|
|`unsigned int policy`|`SCHED_NORMAL/BATCH/IDLE/FIFO/RR/DEADLINE`|`__setscheduler_params()` (`sched_setscheduler`)|`__schedule`, chọn `sched_class`|`sched_setscheduler()`, `sched_setattr()`|Scheduler|
|`const struct sched_class *sched_class`|Con trỏ ops: `fair_sched_class` / `rt_sched_class` / `dl_sched_class` / `idle_sched_class`|`__setscheduler()`|`__schedule` (`for_each_class`), `enqueue_task`, `put_prev_task`|Đổi policy|Scheduler (class dispatch)|
|`struct sched_rt_entity rt` / `struct sched_dl_entity dl`|Trạng thái cho RT (timeslice RR, throttling) / DEADLINE (runtime, deadline, period)|`sched/rt.c`, `sched/deadline.c` dưới `rq->lock`|pick_next của class tương ứng|Task chạy policy RT/DL|Scheduler|
|`const cpumask_t *cpus_ptr` + `cpumask_t cpus_mask`|Tập CPU được phép chạy. `cpus_ptr` thường trỏ `&cpus_mask`; trỏ chỗ khác khi bị "migrate disable" tạm|`set_cpus_allowed_ptr()` (`sched_setaffinity`, cpuset, hotplug)|`select_task_rq`, `can_migrate_task`, load balancer|`sched_setaffinity()`, cpuset thay đổi, CPU hotplug|Scheduler, **cpuset**, hotplug|
|`unsigned int on_rq`|0 / `TASK_ON_RQ_QUEUED` / `TASK_ON_RQ_MIGRATING`|`activate_task`/`deactivate_task`|`__schedule`, `ttwu`|Enqueue/dequeue khỏi runqueue|Scheduler|
|`int on_cpu` (SMP)|Task đang thực sự chạy trên CPU nào đó (dùng cho `smp_cond_load_acquire` khi wakeup)|`__schedule` (`prepare_task`/`finish_task`)|`try_to_wake_up()` (spin chờ task rời CPU)|Vào/ra CPU|Scheduler SMP|
|`unsigned int cpu` (trong/near `thread_info`)|CPU logic hiện gắn task|`set_task_cpu()`|`task_cpu()`, per-cpu access|Migrate|Scheduler|
|`unsigned long nvcsw, nivcsw`|Đếm context switch tự nguyện / bị ép|`__schedule()`|`/proc/<pid>/status`, `getrusage`|Mỗi lần switch away|Thống kê|
|`u64 utime, stime, gtime` + `struct prev_cputime prev_cputime`|Thời gian CPU user/kernel/guest tích lũy|`account_user_time()`, `account_system_time()` (từ tick / vtime)|`getrusage`, `times()`, `/proc/<pid>/stat`, `taskstats`|Mỗi tick / mỗi lần kế toán cputime|**Timekeeping**, accounting|
|`struct sched_info sched_info`|`pcount`, `run_delay` (thời gian chờ runqueue), `last_arrival`|`sched/stats.h` (`sched_info_arrive/depart/queued`)|`/proc/<pid>/schedstat`, `delayacct`|Enqueue/dequeue/switch|schedstats, delay accounting|
|`struct uclamp_se uclamp_req[2]`, `uclamp[2]`|Ràng buộc util-clamp (min/max) cho schedutil/EAS|`sched_setattr()` (UCLAMP), `uclamp_rq_inc`|`cpu_util`, chọn tần số CPU|`sched_setattr` với `SCHED_FLAG_UTIL_CLAMP`|Scheduler + cpufreq (EAS)|

**Ghi chú lock**: mọi field lập lịch "nóng" (`se`, `prio`, `on_rq`) đổi dưới `rq->lock` (spinlock của runqueue). Đổi `prio`/affinity còn cần `p->pi_lock` (bảo vệ chuỗi thao tác wakeup + PI).

---

## 2. Process state

|Field|Công dụng|Ghi bởi|Đọc bởi|Đổi khi nào|Liên hệ|
|---|---|---|---|---|---|
|`volatile long state`|Trạng thái chạy: `TASK_RUNNING(0)`, `TASK_INTERRUPTIBLE(1)`, `TASK_UNINTERRUPTIBLE(2)`, `__TASK_STOPPED(4)`, `__TASK_TRACED(8)`|`set_current_state()` / `__set_current_state()` (task tự ngủ); `try_to_wake_up()` (→ RUNNING); `ptrace_stop`, `do_signal_stop`|`__schedule` (quyết định dequeue), `ttwu`, `/proc`, `ps`|Task chuẩn bị ngủ (đặt trước khi `schedule()`), khi bị đánh thức, khi SIGSTOP/ptrace|**Scheduler**, wait queue, signal, ptrace|
|`unsigned int flags` (PF_*)|Cờ trạng thái vòng đời/vai trò: `PF_KTHREAD`, `PF_IDLE`, `PF_EXITING`, `PF_FORKNOEXEC`, `PF_SIGNALED`, `PF_DUMPCORE`, `PF_WQ_WORKER`, `PF_MEMALLOC`, `PF_NResource`...|Nhiều nơi: `copy_process` (set `PF_FORKNOEXEC`), `do_exit` (`PF_EXITING`), `exec` (clear `PF_FORKNOEXEC`), `kswapd`/reclaim (`PF_MEMALLOC`)|`current->flags & PF_*` khắp kernel (mm reclaim, workqueue, oom, signal)|Fork, exec, exit, vào/ra reclaim, khi tạo kthread|Gần như mọi subsystem|
|`int exit_state`|`0` / `EXIT_ZOMBIE` / `EXIT_DEAD`|`exit_notify()` (`do_exit`), `release_task()`, `wait_task_zombie()` — dưới `tasklist_lock` (write)|`do_wait()`, `__schedule` (bỏ qua zombie), `/proc`|Task gọi `do_exit`; cha `wait()` xong|**exit/wait**, ptrace|
|`int exit_code`|Mã trả về (`_exit(status)` đã shift, hoặc mã tín hiệu giết)|`do_exit()`, `do_group_exit()`, `ptrace_stop` (tái dùng cho stop code)|`wait_task_zombie()` → copy `status` cho userspace; `wait_task_stopped()`|`exit()`, bị signal giết, ptrace stop|exit/wait, ptrace|
|`int exit_signal`|Tín hiệu gửi cho cha khi task chết (thường `SIGCHLD`; `-1` nếu là thread không phải leader → không báo)|`copy_process()` (từ `clone_flags & CSIGNAL`; `-1` nếu `CLONE_THREAD`)|`do_notify_parent()`|Lúc tạo; `exec` bởi thread non-leader có thể chỉnh|exit, signal|
|`int pdeath_signal`|Tín hiệu gửi cho **task này** khi cha nó chết|`prctl(PR_SET_PDEATHSIG)`|`forget_original_parent()` → `reparent_leader()`|`prctl`; **bị xóa khi `exec`** (theo credential-change)|parent/child, signal|
|`unsigned int personality`|Bit tương thích ABI (ADDR_NO_RANDOMIZE, READ_IMPLIES_EXEC, kiểu UNAME…)|`sys_personality()`, `load_elf_binary` (theo `PT_GNU_STACK`, ELF flags)|`arch_align_stack`, ASLR (`randomize_va_space`), `sys_uname`|`personality()` syscall, `exec`|mm layout, exec|
|`unsigned long atomic_flags` (PFA_*)|Cờ cần truy cập nguyên tử: `PFA_NO_NEW_PRIVS`, `PFA_SPEC_SSB_DISABLE`, `PFA_LMK_WAITING`|`set_task_syscall_work`/`task_set_no_new_privs()`, `prctl(PR_SET_SPECULATION_CTRL)`|`exec` (LSM cap calc), `__seccomp_filter`, spec-store-bypass mitigation|`prctl`, seccomp setup|**seccomp**, security, spec mitigation|
|`unsigned sched_reset_on_fork:1` (+ nhiều bitfield 1-bit)|`SCHED_RESET_ON_FORK`: con reset về SCHED_NORMAL/nice 0|`__sched_setscheduler()`|`sched_fork()`|`sched_setscheduler` flag|Scheduler|
|`struct restart_block restart_block`|Lưu cách "khởi động lại" syscall bị ngắt bởi signal (`ERESTART_RESTARTBLOCK`) — vd `nanosleep`, `futex`, `poll`|Các syscall trước khi ngủ (`set_restart_fn`)|`sys_restart_syscall()` sau khi signal handler trả về|Mỗi syscall có thể restart|signal, timers, futex|

**Điểm mấu chốt state**: barrier. `set_current_state(TASK_INTERRUPTIBLE)` là `smp_store_mb()` — phải đặt state **trước** khi kiểm tra điều kiện ngủ, nếu không có race với waker. Đây là bug kinh điển kernel dev phải nắm.

---

## 3. PID / định danh nhóm

|Field|Công dụng|Ghi bởi|Đọc bởi|Đổi khi nào|Liên hệ|
|---|---|---|---|---|---|
|`pid_t pid`|Số hiệu task **trong pid namespace gốc** = TID theo nghĩa POSIX|`copy_process()` → `alloc_pid()` rồi `task->pid = pid_nr(pid)`|`task_pid_nr()`, `gettid()`, scheduler tracepoints|Chỉ lúc tạo; **không đổi** suốt đời task|**PID subsystem**, namespace|
|`pid_t tgid`|PID của **thread group** = số `getpid()` trả về. `= group_leader->pid`|`copy_process()`: nếu `CLONE_THREAD` thì `tgid = current->tgid`, ngược lại `tgid = pid`|`getpid()`, `task_tgid_nr()`, `/proc/<tgid>`|Lúc tạo; đổi khi thread non-leader gọi `exec` (nó "chiếm" vai leader, `de_thread()`)|PID, exec, thread group|
|`struct pid *thread_pid`|Con trỏ tới object `struct pid` cho **cấp TID** của task này (thay cho mảng `pids[]` cũ)|`copy_process()` (`init_task_pid`), `__change_pid`|`pid_task()`, `find_task_by_vpid()` ngược lại|Lúc tạo; `exec` (de_thread hoán đổi pid leader↔caller); exit (`detach_pid`)|PID hash, `/proc`|
|`struct hlist_node pid_links[PIDTYPE_MAX]`|Móc task vào danh sách của `struct pid` theo từng type: `PIDTYPE_PID` (TID), `PIDTYPE_TGID`, `PIDTYPE_PGID`, `PIDTYPE_SID`|`attach_pid()` / `detach_pid()` / `change_pid()` — giữ `tasklist_lock` (write)|`do_each_pid_task()`, gửi signal theo nhóm (`kill(-pgrp)`), `pid_task()`|`fork` (attach PID+TGID), `setpgid`/`setsid` (đổi PGID/SID), `exec`, `exit`|PID, **job control**, signal delivery|
|`struct list_head thread_group`|Vòng liên kết mọi thread cùng `tgid`|`copy_process()` (`list_add_tail_rcu(&p->thread_group, &p->group_leader->thread_group)`), `__unhash_process` (exit)|`while_each_thread()`, `for_each_thread()`, `zap_other_threads()`, `do_group_exit`|Tạo/hủy thread|thread group, signal, exit|
|`struct list_head thread_node`|Móc task vào `signal->thread_head` (cách duyệt thread hiện đại, an toàn RCU)|`copy_process()`, `__exit_signal()`|`for_each_thread()`|Tạo/hủy thread|signal_struct|
|`struct task_struct *group_leader`|Trỏ tới task leader của nhóm (task có `pid==tgid`)|`copy_process()` (`p->group_leader = CLONE_THREAD ? current->group_leader : p`)|`thread_group_leader()`, `has_group_leader_pid()`, hầu hết code "process-level"|Lúc tạo; `de_thread()` khi non-leader `exec`|thread group, exec, exit|
|`struct nsproxy *nsproxy`|Chứa con trỏ tới `pid_namespace` (và mnt/net/uts/ipc/cgroup ns)|`copy_process()` → `copy_namespaces()`; `setns()`, `unshare()`|`task_active_pid_ns()`, `pid_nr_ns()` để dịch PID theo ns của người xem|`clone(CLONE_NEW*)`, `setns`, `unshare`|**Namespaces**, container|

**Vì sao tách `struct pid` object**: một PID có thể được nhiều nơi tham chiếu (fd của `pidfd`, `struct file` của `/proc/<pid>`, waitqueue) _sau khi_ task đã chết. `struct pid` có refcount riêng, sống lâu hơn `task_struct`. `pid_task(pid, type)` trả `NULL` nếu task đã đi.

---

## 4. Quan hệ cha/con

|Field|Công dụng|Ghi bởi|Đọc bởi|Đổi khi nào|Liên hệ|
|---|---|---|---|---|---|
|`struct task_struct __rcu *real_parent`|Cha **thật** (task đã `fork` ra nó). Không đổi vì ptrace|`copy_process()` (`= CLONE_PARENT ? current->real_parent : current`); `forget_original_parent()` khi cha chết → reparent|`getppid()` (khi không bị trace), `do_wait`, `/proc/<pid>/status`|Tạo; **reparent** khi cha chết (giao cho subreaper/init)|parent/child, exit|
|`struct task_struct __rcu *parent`|Cha **hiệu lực** = nơi nhận `SIGCHLD` và `wait()`. Bằng `real_parent` trừ khi đang bị `ptrace` (khi đó = tracer)|`copy_process()`; `__ptrace_link()` / `__ptrace_unlink()`|`do_notify_parent()`, `do_wait`, `ptrace`|`PTRACE_ATTACH`/`PTRACE_TRACEME` → parent := tracer; `PTRACE_DETACH` → khôi phục|**ptrace**, exit/wait, signal|
|`struct list_head children`|Đầu danh sách con của task này (liên kết qua `child->sibling`)|`copy_process()` (`list_add_tail(&p->sibling, &p->real_parent->children)`), `exit`|`do_wait()` duyệt con; `forget_original_parent()`|Mỗi lần fork/exit con|parent/child, wait|
|`struct list_head sibling`|Móc task vào `parent->children`|như trên|như trên|Fork; reparent (chuyển sang list con của cha mới)|parent/child|
|`struct list_head ptraced`|Đầu list các task mà task này đang **trace**|`__ptrace_link()`|`exit_ptrace()` (khi tracer chết, thả hết tracee), `ptrace_check_attach`|`PTRACE_ATTACH`|ptrace|
|`struct list_head ptrace_entry`|Móc task (với vai trò tracee) vào `tracer->ptraced`|`__ptrace_link/unlink`|`exit_ptrace`, reparent|ATTACH/DETACH/exit|ptrace|
|`unsigned int ptrace`|Bitmask: `PT_PTRACED`, `PT_SEIZED`, `PT_TRACE_*` (fork/vfork/clone/exec/exit), `PT_TRACESYSGOOD`|`ptrace_setoptions()`, `ptrace_attach()`|`ptrace_event()`, `syscall_trace_enter/exit`, `tracehook_*`|`PTRACE_SETOPTIONS`, ATTACH, DETACH|ptrace, syscall entry (`arch/arm64/kernel/ptrace.c`)|
|`int __user *set_child_tid`|Địa chỉ user để kernel ghi TID con sau clone (`CLONE_CHILD_SETTID`)|`copy_process()` (lưu con trỏ); `schedule_tail()` ghi TID vào đó khi con chạy lần đầu|glibc pthread (đọc để biết tid)|`clone(CLONE_CHILD_SETTID)`|pthread, futex|
|`int __user *clear_child_tid`|Khi thread chết, kernel ghi 0 vào địa chỉ này và `futex_wake` (`CLONE_CHILD_CLEARTID`) — nền tảng `pthread_join`|`copy_process()`, `sys_set_tid_address()`|`mm_release()` trong `do_exit` → `put_user(0, tidptr); futex_wake()`|`clone(CLONE_CHILD_CLEARTID)`, `set_tid_address()`|**pthread join**, futex|
|`struct completion *vfork_done`|Cha `vfork()` chờ trên completion này; con signal khi `exec`/`exit`|`copy_process()` (cha set trước khi wake con); `mm_release()` (con `complete()`)|`wait_for_vfork_done()` (cha)|`vfork()`|fork/exec|
|`u64 parent_exec_id` / `self_exec_id`|Chống race "cha exec/exit rồi PID bị tái dùng" khi gửi SIGCHLD|`copy_process()`, `de_thread`, `setup_new_exec`|`do_notify_parent()` so khớp|Mỗi lần `exec`|exit/signal đúng đắn|

**Lock**: chỉnh cây cha/con luôn dưới `write_lock_irq(&tasklist_lock)`. Đọc thì `rcu_read_lock()` + `rcu_dereference()` (chú ý `__rcu` trên `real_parent`/`parent`).

---

## 5. Quản lý bộ nhớ

|Field|Công dụng|Ghi bởi|Đọc bởi|Đổi khi nào|Liên hệ|
|---|---|---|---|---|---|
|`struct mm_struct *mm`|Không gian địa chỉ **do task sở hữu**. `NULL` với kernel thread|`copy_mm()` (fork), `exec_mmap()` (exec), `exit_mm()` (→ `NULL`), `kthread_use_mm()`|Page fault handler, `get_user_pages`, `access_process_vm`, OOM killer, `/proc/<pid>/maps`|fork (share nếu `CLONE_VM`, hoặc `dup_mm`), exec (mm mới), exit (bỏ)|**mm subsystem**, page fault, OOM|
|`struct mm_struct *active_mm`|mm **đang được cài trong `TTBR0_EL1`**. Với kernel thread = mm "mượn" của task trước (lazy TLB)|`context_switch()`: nếu `!next->mm` thì `next->active_mm = prev->active_mm; mmgrab()`|`switch_mm()`, `check_and_switch_context()` (ARM64), `mmdrop()`|Mỗi context switch liên quan kernel thread|Scheduler ↔ mm, ARM64 ASID|
|`struct vmacache vmacache`|Cache 4 phần tử `vm_area_struct*` tra cứu VMA gần nhất theo địa chỉ (per-thread!) + `seqnum` để invalidate|`vmacache_update()` sau mỗi `find_vma()` thành công|`vmacache_find()` đầu `find_vma()`|Mỗi `find_vma`; invalidate khi `mm->vmacache_seqnum++` (VMA thêm/xóa)|Page fault hot path|
|`struct task_rss_stat rss_stat`|Bộ đệm per-task cộng dồn thay đổi RSS (giảm atomic lên `mm->rss_stat`)|`add_mm_counter_fast()` / `inc_mm_counter_fast()` khi map/unmap page|Flush vào `mm` ở `check_sync_rss_stat()` (mỗi 64 sự kiện hoặc khi switch)|Mỗi lần page vào/ra address space|mm accounting, `/proc/<pid>/statm`|
|`struct page_frag task_frag`|Bộ cấp phát mảnh trang per-task (chủ yếu cho networking `sk_page_frag`)|`page_frag_alloc()`|`skb_page_frag_refill()`|Khi gửi dữ liệu mạng cần buffer|net|
|`int pagefault_disabled`|Đếm lồng `pagefault_disable()` — trong vùng này page fault từ kernel không được ngủ (vd `copy_*_user` trong atomic, kmap_atomic)|`pagefault_disable/enable()`|`faulthandler_disabled()`, `do_page_fault` (ARM64)|Vào/ra vùng atomic-copy|page fault, preempt|
|`unsigned long ~ (trong` thread`) fault_address, fault_code`|(Thực ra ở `thread_struct`) FAR_EL1 / ESR_EL1 của fault gần nhất|`do_page_fault()` (`arch/arm64/mm/fault.c`)|`__do_kernel_fault`, coredump, `show_regs`|Mỗi fault|ARM64 fault, ptrace `PTRACE_GETSIGINFO`|
|`unsigned int (bitfield) brk_randomized:1`, `in_user_fault:1` ...|Cờ trạng thái mm nhỏ|`setup_new_exec`, `do_page_fault`|ASLR heap, memcg oom|exec, khi đang xử lý user fault|mm layout, memcg|

> `mm` vs `active_mm` là câu hỏi phỏng vấn kinh điển: **task thường**: `mm == active_mm`. **kernel thread**: `mm == NULL`, `active_mm` trỏ mm mượn → không phải reload `TTBR0` khi switch giữa kernel thread và task cùng mm.

---

## 6. File descriptors & filesystem context

|Field|Công dụng|Ghi bởi|Đọc bởi|Đổi khi nào|Liên hệ|
|---|---|---|---|---|---|
|`struct files_struct *files`|Bảng file descriptor: `fdtable` (`fd[]`, `close_on_exec`, `open_fds` bitmap) + `fd_array[]` nhỏ nhúng sẵn|`copy_files()` (fork: share nếu `CLONE_FILES`, else `dup_fd`); `exit_files()`; `unshare(CLONE_FILES)`|`fget()`/`fdget()` mọi syscall thao tác fd (`read`,`write`,`close`,`dup`,`mmap`...)|`open`/`close`/`dup` (đổi nội dung, không đổi con trỏ), fork, `unshare`, exit|**VFS**, syscall fd|
|`struct fs_struct *fs`|Ngữ cảnh filesystem: `root` (path), `pwd` (path), `umask`, `in_exec`|`copy_fs()` (share nếu `CLONE_FS`); `set_fs_root/pwd()` (`chdir`,`chroot`); `exit_fs()`|Path resolution (`path_init`, `link_path_walk`), `getcwd`, tạo file (áp `umask`)|`chdir`, `chroot`, `umask()`, `pivot_root`, fork, `unshare(CLONE_FS)`|**VFS** namei, mount namespace|
|`struct nsproxy *nsproxy`|mount ns (`mnt_ns`), net, uts, ipc, cgroup, pid-for-children, time ns|`copy_namespaces()`, `setns()`, `unshare()`|`path_init` (mnt_ns), socket create (net_ns), `uname` (uts_ns)...|`clone(CLONE_NEW*)`, `setns`, `unshare`|Namespaces|
|`struct io_context *io_context`|Ngữ cảnh I/O scheduler khối (BFQ/CFQ đời cũ): độ ưu tiên I/O, cấu trúc per-process|`get_task_io_context()`, `ioprio_set()`|Block layer I/O scheduler|`ionice`, submit I/O lần đầu|block layer|
|`char comm[TASK_COMM_LEN]` (16)|Tên lệnh ngắn (không path). Hiển thị `ps`, `top`, `/proc/<pid>/comm`|`__set_task_comm()` từ `setup_new_exec()` (exec), `prctl(PR_SET_NAME)`, `pthread_setname_np`|`get_task_comm()`, printk oops, tracepoints, audit|`exec`, `prctl(PR_SET_NAME)`|debugging, audit, tracing|

**Lưu ý bảo mật**: khi `exec` một chương trình setuid, `files` **không** bị thay (fd vẫn còn, trừ `O_CLOEXEC`), nhưng fd 0/1/2 được kiểm tra/hoán để tránh lỗ hổng (`unsafe_exec`).

---

## 7. Credentials

|Field|Công dụng|Ghi bởi|Đọc bởi|Đổi khi nào|Liên hệ|
|---|---|---|---|---|---|
|`const struct cred __rcu *real_cred`|Credential **khách quan** — cái mà _người khác_ thấy khi tác động lên task này (vd ai được `kill`, `ptrace` nó). Chứa uid/gid thật, saved, fs; capabilities; `user_struct`; keyrings|`commit_creds()` (giữ qua `prepare_creds`); `copy_creds()` (fork)|`ptrace_may_access()`, `check_kill_permission()`, `__ptrace_may_access`|`setuid/setgid/setresuid`, `exec` file setuid, `capset`, keyring ops|**LSM**, capabilities, ptrace, signal perm|
|`const struct cred __rcu *cred`|Credential **chủ quan / hiệu lực** — cái task dùng khi _nó_ truy cập tài nguyên (mở file, tạo socket). Thường trùng `real_cred`; khác tạm khi `override_creds()` (vd `open_by_handle_at`, nfsd, `access()`? không — `access` dùng cách khác)|`commit_creds()`, `override_creds()`/`revert_creds()`|`inode_permission()`, `generic_permission()`, mọi `current_cred()` / `current_fsuid()`|`setuid` v.v., `exec`, vùng `override_creds`|VFS permission, LSM|
|`const struct cred __rcu *ptracer_cred`|Cred của tracer tại thời điểm ATTACH (để check quyền khi tracee sau này đổi cred)|`ptrace_link()`|`ptrace_may_access()` (yama, `PTRACE_MODE_*`)|`PTRACE_ATTACH`|ptrace, Yama LSM|
|`struct key *cached_requested_key`|Cache key vừa `request_key()` để tránh tra lại|`request_key_and_link()`|`request_key()` fast path|Mỗi `request_key`|keyrings|
|`struct seccomp seccomp`|`mode` (DISABLED/STRICT/FILTER) + con trỏ danh sách BPF filter (`struct seccomp_filter`)|`seccomp_set_mode_filter()` (`prctl`/`seccomp()`); `copy_process` copy con trỏ (filter share, refcount)|`__secure_computing()` gọi từ `syscall_trace_enter` (`arch/arm64/kernel/syscall.c`) trên **mọi syscall**|`prctl(PR_SET_SECCOMP)`, `seccomp(2)`; con kế thừa khi fork|**seccomp**, syscall entry|
|`kernel_cap_t` (trong `cred`) `cap_permitted/effective/inheritable/bset/ambient`|Chia nhỏ quyền root thành ~40 capability (`CAP_NET_ADMIN`, `CAP_SYS_ADMIN`...)|`capset()`, tính lại khi `exec` theo file capabilities (xattr `security.capability`)|`capable()`, `ns_capable()` khắp kernel|`capset`, `exec`, `prctl(PR_CAP_AMBIENT)`|Toàn bộ kiểm tra đặc quyền|

**Mô hình COW**: không bao giờ sửa `cred` tại chỗ. Luôn `new = prepare_creds()` → chỉnh `new` → `commit_creds(new)` (atomic RCU thay con trỏ, `put_cred` bản cũ). Đọc trong RCU: `current_cred()` = `rcu_dereference_protected(current->cred, 1)` (an toàn vì task tự nó không đổi cred của chính mình song song).

---

## 8. Signals

|Field|Công dụng|Ghi bởi|Đọc bởi|Đổi khi nào|Liên hệ|
|---|---|---|---|---|---|
|`struct signal_struct *signal`|Trạng thái **cả thread group**: `shared_pending` (signal gửi tới process), `nr_threads`, `thread_head`, `group_exit_code`/`group_exit_task` (điều phối `exit_group`), `rlim[RLIM_NLIMITS]`, `pids[PIDTYPE_*]` (pgrp/session), `tty`, thống kê cộng dồn của thread đã chết (`utime`,`cutime`...), `wait_chldexit`|`copy_signal()` (fork: share nếu `CLONE_THREAD`); nhiều field đổi runtime dưới `siglock`|`getrlimit`, `wait4`, job control, `do_notify_parent`, `complete_signal`|Tạo thread (nr_threads++), exit, `setrlimit`, `setpgid/setsid`, signal nhóm|**Signal**, exit/wait, rlimit, job control, tty|
|`struct sighand_struct __rcu *sighand`|`action[_NSIG]` (bảng `struct k_sigaction` = disposition + mask + flags) + `siglock` (spinlock bảo vệ **cả** `sighand` lẫn `signal`) + `count`|`copy_sighand()` (share nếu `CLONE_SIGHAND`); `do_sigaction()` (`rt_sigaction`); `flush_signal_handlers()` khi `exec` (reset non-ignored về `SIG_DFL`)|`get_signal()` → `ka = &sighand->action[signr-1]`; `do_signal` (ARM64)|`sigaction()`, `exec`, tạo/hủy thread|**Signal delivery**, exec|
|`sigset_t blocked`|Mặt nạ tín hiệu bị chặn của **thread này** (per-thread!)|`sigprocmask()`, `pthread_sigmask` (→ `set_current_blocked()`); `get_signal` tạm chặn `sa_mask + signr` khi vào handler|`next_signal()`, `dequeue_signal()`, `recalc_sigpending()`|`sigprocmask`, vào/ra signal handler|Signal|
|`sigset_t real_blocked`|Lưu `blocked` thật khi đang trong `sigtimedwait()` (task tạm mở một số signal để "bắt")|`sys_rt_sigtimedwait()`|`dequeue_signal`|`sigwaitinfo/sigtimedwait`|Signal|
|`sigset_t saved_sigmask`|Lưu mask để khôi phục sau `pselect/ppoll/epoll_pwait/sigsuspend` (khôi phục ở `ret_to_user` nếu không có handler chạy)|`sigsuspend`, `pselect6` set `TIF_RESTORE_SIGMASK`|`restore_saved_sigmask()` trong `do_signal` (ARM64)|Mỗi `pselect/ppoll/sigsuspend`|Signal, syscall|
|`struct sigpending pending`|Hàng đợi tín hiệu **riêng thread** (`list` các `struct sigqueue` + `signal` bitmap) — cho `tgkill`, `SIGSEGV`/`SIGFPE` do fault của chính thread|`__send_signal()` (khi `specific` = task), `force_sig_info()` (fault)|`dequeue_signal()`, `recalc_sigpending()`|`tgkill`, synchronous fault, timer per-thread|Signal, ARM64 fault → `force_sig_fault()`|
|`unsigned long sas_ss_sp; size_t sas_ss_size; unsigned int sas_ss_flags`|Alternate signal stack (`sigaltstack()`), dùng khi `SA_ONSTACK` — vd bắt `SIGSEGV` do tràn stack|`sys_sigaltstack()`|`sigsp()` trong `setup_rt_frame()` (ARM64 `signal.c`) chọn stack dựng sigframe|`sigaltstack()`; auto-disable khi đang chạy trên nó|Signal, ARM64 sigframe|
|`struct callback_head *task_works`|Danh sách công việc chạy khi task **sắp trở về user** (`task_work_add`) — vd `fput` trì hoãn, `mntput`, khoá keyring, `io_uring`|`task_work_add()`|`task_work_run()` gọi từ `do_notify_resume` (ARM64, khi `_TIF_NOTIFY_RESUME`)|Bất cứ khi nào kernel cần "làm nốt ở ngữ cảnh task"|VFS (`fput`), keyrings, ARM64 ret-to-user|

**Thứ tự khóa signal**: `read_lock(&tasklist_lock)` (để giữ task/nhóm ổn định) → `spin_lock_irqsave(&sighand->siglock)`. `siglock` bảo vệ cả `pending`, `shared_pending`, `blocked` recalculation, `action[]`.

---

## 9. Trạng thái kiến trúc-riêng: `struct thread_struct thread` (ARM64)

`arch/arm64/include/asm/processor.h`. Đây là field **cuối** task_struct.

|Field|Công dụng|Ghi bởi|Đọc bởi|Đổi khi nào|Liên hệ|
|---|---|---|---|---|---|
|`struct cpu_context cpu_context`|`x19..x28, fp(x29), sp, pc` — **ngữ cảnh CPU khi task KHÔNG chạy**. Đủ để đóng băng/rã đông task tại điểm nó gọi `schedule()`|`cpu_switch_to` (asm, `entry.S`) lưu của `prev`; `copy_thread_tls()` khởi tạo cho task mới (`pc = ret_from_fork`, `sp = childregs`)|`cpu_switch_to` nạp của `next`; `__switch_to()`|**Mỗi context switch**; lúc tạo task|**Scheduler ↔ ARM64**, `asm-offsets.c` (`THREAD_CPU_CONTEXT`)|
|`.uw.tp_value`|Giá trị TLS của user (`TPIDR_EL0`) — glibc dùng làm con trỏ TCB|`tls_thread_switch()` lưu `TPIDR_EL0` của prev; `sys_prctl(PR_SET_TLS)` / `clone(CLONE_SETTLS)`|`tls_thread_switch()` nạp cho next; `ptrace` `NT_ARM_TLS`|Mỗi switch; `clone(CLONE_SETTLS)`|ARM64 `__switch_to`, TLS|
|`.uw.fpsimd_state` (`struct user_fpsimd_state`: `__uint128_t vregs[32], u32 fpsr, fpcr`)|Trạng thái FP/SIMD (NEON) của task|`fpsimd_save()` (lazy — chỉ khi task "dirty" và bị switch away / vào signal / `exec`)|`fpsimd_load_state()` khi task chạy lại và chạm lệnh FP (bẫy `TIF_FOREIGN_FPSTATE`)|Lazy: switch away nếu dùng FP; `exec` (xóa); signal (lưu/khôi phục)|`arch/arm64/kernel/fpsimd.c`, `entry-fpsimd.S`|
|`void *sve_state; unsigned int sve_vl, sve_vl_onexec`|Buffer + vector length cho SVE (nếu CPU có)|`sve_alloc()`, `sve_set_vector_length()` (`prctl(PR_SVE_SET_VL)`), `fpsimd` switch|SVE load/save, `ptrace` `NT_ARM_SVE`|Lần đầu dùng SVE; `prctl`; `exec` (`sve_vl_onexec`)|fpsimd.c|
|`unsigned long fault_address`|`FAR_EL1` (địa chỉ gây fault) của lần abort gần nhất|`do_mem_abort()` / `do_page_fault()`|coredump, `show_regs()`, `PTRACE_GETSIGINFO` (`si_addr`)|Mỗi data/instr abort|ARM64 fault|
|`unsigned long fault_code`|`ESR_EL1` của lần fault|như trên|`__do_kernel_fault`, phân loại lỗi|Mỗi fault|ARM64 fault|
|`struct debug_info debug`|`hbp_break[]`, `hbp_watch[]` — hardware breakpoint/watchpoint (`DBGBVR/DBGWVR`), `bps_disabled`|`ptrace` (`PTRACE_SETHBPREGS`), `hw_breakpoint` perf|`hw_breakpoint_thread_switch()` → nạp DBG regs khi switch vào task có bp|`PTRACE_POKEUSER`/`SETHBPREGS`, perf breakpoint|ptrace, perf, `hw_breakpoint.c`|
|`.uw.tp2_value` (nếu có) / `struct ptrauth_keys_user keys_user` (5.7+, KHÔNG có ở 5.4 mainline gốc)|PAC keys — bỏ qua ở 5.4|—|—|—|Pointer Authentication|

**Whitelist hardened usercopy**: sub-struct `uw` (user-writable) được đánh dấu để `copy_to_user`/`copy_from_user` được phép chạm (qua `ptrace` regset), phần còn lại của `thread_struct` thì không.

---

## 10. Kernel stack & low-level

|Field|Công dụng|Ghi bởi|Đọc bởi|Đổi khi nào|Liên hệ|
|---|---|---|---|---|---|
|`struct thread_info thread_info` (**field #1**)|ARM64 (`CONFIG_THREAD_INFO_IN_TASK`): `unsigned long flags` (TIF_*), `mm_segment_t addr_limit` (USER_DS/KERNEL_DS), `union { u64 preempt_count }`|`set_ti_thread_flag()` / `set_tsk_thread_flag()`; `preempt_count_add/sub()`; `set_fs()` (deprecated)|`entry.S` (`ret_to_user` đọc `TSK_TI_FLAGS`), `preempt_count()`, `test_thread_flag()`, `uaccess` (`addr_limit`)|Set `TIF_NEED_RESCHED` (tick/wakeup), `TIF_SIGPENDING` (signal), vào/ra preempt-disable, vào/ra syscall|**Scheduler**, signal, preempt, uaccess, ARM64 entry|
|`void *stack`|Trỏ tới **đáy** vùng kernel stack (`THREAD_SIZE` = 16 KB, 4 trang). `pt_regs` nằm ở **đỉnh**: `task_pt_regs(p) = (struct pt_regs *)(stack + THREAD_SIZE) - 1`|`dup_task_struct()` → `alloc_thread_stack_node()` (với `VMAP_STACK`: vùng vmalloc + guard page)|`entry.S` (nạp `sp_el1`), unwinder (`arch_stack_walk`), `task_pt_regs()`, `end_of_stack()` (stack canary end)|Lúc tạo task; free ở `free_thread_stack()` (exit)|**ARM64 entry**, stack overflow detect (`handle_bad_stack`), unwind|
|`refcount_t usage`|Refcount `task_struct` — số nơi đang giữ con trỏ tới task|`get_task_struct()` / `put_task_struct()`|`put_task_struct()` (khi về 0 → `__put_task_struct` → `free_task`)|Mỗi khi lấy/thả tham chiếu (waitqueue, timer, `find_get_task`, ptrace list...)|Vòng đời task, RCU|
|`atomic_t stack_refcount` (khi `THREAD_INFO_IN_TASK && VMAP_STACK`)|Refcount **riêng cho vùng stack** — cho phép free stack sớm (khi task chết) trong khi `task_struct` còn sống chờ RCU/wait|`try_get_task_stack()` / `put_task_stack()`|`free_thread_stack` khi về 0|Unwinder/NMI giữ stack tạm; `do_exit`→`release_task`|VMAP_STACK, `/proc/<pid>/stack`|
|`unsigned long stack_canary` (nếu `CONFIG_STACKPROTECTOR`)|Giá trị canary cho `-fstack-protector`; ARM64 lấy canary từ per-cpu chứ không phải field này (field vẫn tồn tại cho arch dùng `TSK_STACK_CANARY`)|`copy_process()` (`p->stack_canary = get_random_canary()`); `boot_init_stack_canary()`|prologue/epilogue hàm (so canary), `__stack_chk_fail`|Lúc tạo task|Stack protector|
|`struct task_struct *cpu` (thực ra `unsigned int cpu` gần `thread_info`)|CPU logic đang gắn (dùng `task_cpu()`, `set_task_cpu()`)|`set_task_cpu()` (scheduler)|per-cpu access, tracepoint, `smp_processor_id()` sanity|Migrate|Scheduler SMP|
|`int (bitfield) in_execve:1, in_iowait:1, ...`|Cờ vi mô: `in_iowait` (task ngủ vì I/O → tính vào `iowait` load)|`io_schedule_prepare()`, `setup_new_exec`|load average, `/proc/stat` iowait|Vào `io_schedule()`|scheduler stats, block|

**Stack overflow ARM64**: với `VMAP_STACK`, chạm guard page → data abort → `entry.S` phát hiện `sp` ngoài vùng stack hợp lệ → chuyển sang **overflow stack** per-cpu → `handle_bad_stack()` in `"Insufficient stack space"` rồi panic. Không có `VMAP_STACK` thì overflow âm thầm đè `thread_info`/`task_struct` lân cận (rất khó debug).

---

## 11. Sơ đồ (đã lưu ra file)

Xem [docs/diagrams/task-struct-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/task-struct-arm64.md):

- **Sơ đồ 1** — `classDiagram`: `task_struct` với đầy đủ field theo nhóm + 12 object vệ tinh (`sched_entity`, `mm_struct`, `files_struct`, `fs_struct`, `signal_struct`, `sighand_struct`, `cred`, `thread_struct`, `pid`, `thread_info`) và field then chốt của từng cái.
- **Sơ đồ 2** — `graph`: **cái gì per-thread, cái gì shared** khi các task cùng thread group. Đây là hình cho câu "vì sao 1 struct cho cả process lẫn thread".

Rút gọn quan hệ sở hữu:

```
task_struct
 ├─ thread_info      (NHÚNG, field #1)      → TIF_*, preempt_count, addr_limit
 ├─ sched_entity se  (NHÚNG)                → vruntime, cây đỏ-đen cfs_rq
 ├─ mm_struct*       (con trỏ, refcnt)      → pgd→TTBR0, VMA, ASID     [share: CLONE_VM]
 ├─ files_struct*    (con trỏ, refcnt)      → fdtable                  [share: CLONE_FILES]
 ├─ fs_struct*       (con trỏ, refcnt)      → root, pwd, umask         [share: CLONE_FS]
 ├─ signal_struct*   (con trỏ, refcnt)      → shared_pending, rlim[], pgrp/sid  [share: CLONE_THREAD]
 ├─ sighand_struct*  (con trỏ, refcnt)      → action[_NSIG], siglock   [share: CLONE_SIGHAND]
 ├─ cred* (real_cred/cred, RCU+refcnt)      → uid/gid/caps/keyrings    [COW per-task]
 ├─ thread_struct thread (NHÚNG, field CUỐI)→ cpu_context, fpsimd, TLS, hw bp   [per-thread]
 ├─ pid* (thread_pid) + pid_links[]         → PID/TGID/PGID/SID hash   [per-thread cho PID]
 └─ void* stack (16KB) + pt_regs @ đỉnh     → kernel stack             [per-thread]
```

---

## 12. Khác biệt 5.4 so với bản mới (đọc kernel khác khỏi lạc)

|Field / điểm|5.4|Bản sau|
|---|---|---|
|`state`|`volatile long state`|`unsigned int __state` + `READ_ONCE`/`WRITE_ONCE` (5.14)|
|Cấu trúc PID|`struct pid *thread_pid` + `struct hlist_node pid_links[PIDTYPE_MAX]`|giữ nguyên (đổi từ `struct pid_link pids[]` từ 4.19)|
|Affinity|`const cpumask_t *cpus_ptr` + `cpumask_t cpus_mask`|+ cơ chế `migrate_disable()` dùng `cpus_ptr` nhiều hơn (5.11)|
|`io_uring`|**không có**|`struct io_uring_task *io_uring` (5.6+)|
|`posix_cputimers`|`struct posix_cputimers posix_cputimers` (đã thay `cputime_expires` từ 5.3)|+ `posix_cputimers_work` (5.11)|
|RCU-tasks-trace|không có `trc_holdout_list`|thêm 5.7 (BPF sleepable)|
|`wake_entry`|`struct llist_node wake_entry`|`struct __call_single_node wake_entry` (5.10)|
|PAC keys trong `thread_struct`|5.4 mainline: chưa; **BSP có thể đã backport**|`ptrauth_keys_user/kernel` (5.7 / 5.8)|
|arch entry|`el0_svc_handler()` asm-dispatch trong `entry.S`|`do_el0_svc()` + `entry-common.c` C-dispatch (5.8)|

> Cây nguồn local của bạn (`Kernel/K-S800/Src`) là **5.10.241** — nên khi đối chiếu bạn sẽ thấy `io_uring`, `__call_single_node`, `do_el0_svc`. Bố cục theo subsystem ở trên không đổi.