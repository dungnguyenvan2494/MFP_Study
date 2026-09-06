# MODULE 4 — `fork()` end-to-end trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/fork-end-to-end-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/fork-end-to-end-arm64.md) (4 sơ đồ: sequence đầy đủ, parent vs child, trạng thái COW, wiring `ret_from_fork`).

**File nguồn**: `kernel/fork.c` (`_do_fork`, `copy_process`, `dup_task_struct`, `dup_mm`, `dup_mmap`), `mm/memory.c` (`copy_page_range`, `copy_one_pte`, `do_wp_page`, `wp_page_copy`), `arch/arm64/kernel/process.c` (`copy_thread`), `arch/arm64/kernel/entry.S` (`ret_from_fork`), `kernel/sched/core.c` (`sched_fork`, `wake_up_new_task`, `context_switch`), `arch/arm64/kernel/entry.S` (`cpu_switch_to`).

---

## 0. Chỉnh lại roadmap: **arm64 native không có `sys_fork`**

`arch/arm64/include/asm/unistd.h`: `__ARCH_WANT_SYS_FORK` / `__ARCH_WANT_SYS_VFORK` nằm **trong `#ifdef CONFIG_COMPAT`** → chỉ wiring cho bảng syscall **AArch32** (`__NR_fork = 2`). Bảng **AArch64** chỉ có `__ARCH_WANT_SYS_CLONE`.

→ glibc `fork()` trên arm64 = `clone(SIGCHLD, 0, NULL, NULL, 0)` qua `__NR_clone` (220). Chuỗi thật: `fork()` (libc) → `svc #0` (w8=220, x0=`SIGCHLD`) → `__arm64_sys_clone` → `_do_fork(&args)`. (`vfork()` = `clone(CLONE_VM|CLONE_VFORK|SIGCHLD, 0, …)` — Module 23.)

---

## 1. Từ EL0 tới `_do_fork`

1. libc: `mov x0, #17` (SIGCHLD) ; `mov x1,#0` (child_stack=0 → dùng chung SP user, COW) ; `x2=x3=x4=0` ; `mov w8, #220` ; `svc #0`.
2. HW exception → `kernel_entry 0` → `pt_regs` của **parent** trên đỉnh kernel stack của parent (Module 3).
3. `el0_svc_handler` → `sys_call_table[220]` = `__arm64_sys_clone`.
4. `SYSCALL_DEFINE5(clone, ...)` (kernel/fork.c) đọc `regs->regs[0..4]`, dựng:
    
    ```c
    struct kernel_clone_args args = {
        .flags       = (clone_flags & ~CSIGNAL),
        .pidfd       = parent_tidptr,
        .child_tid   = child_tidptr,
        .parent_tid  = parent_tidptr,
        .exit_signal = (clone_flags & CSIGNAL),   // = SIGCHLD
        .stack       = newsp,                      // = 0
        .tls         = tls,
    };
    return _do_fork(&args);
    ```
    

---

## 2. `_do_fork()` (kernel/fork.c) — khung

```c
long _do_fork(struct kernel_clone_args *args)
{
    ...
    p = copy_process(NULL, trace, NUMA_NO_NODE, args);   // ◀── tạo task con
    if (IS_ERR(p)) return PTR_ERR(p);

    pid = get_task_pid(p, PIDTYPE_PID);
    nr  = pid_vnr(pid);                                   // PID con trong pid-ns của parent

    if (clone_flags & CLONE_PARENT_SETTID)
        put_user(nr, args->parent_tid);

    if (clone_flags & CLONE_VFORK) {                      // fork() thường: KHÔNG
        p->vfork_done = &vfork;
        init_completion(&vfork);
    }

    wake_up_new_task(p);                                  // ◀── đưa con vào runqueue

    if (clone_flags & CLONE_VFORK) {                      // fork() thường: KHÔNG
        if (!wait_for_vfork_done(p, &vfork))
            ptrace_event_pid(PTRACE_EVENT_VFORK_DONE, pid);
    }
    put_pid(pid);
    return nr;                                            // ◀── PARENT trả về PID con
}
```

Sau `_do_fork` return `nr`, giá trị này đi ngược: `__arm64_sys_clone` return `nr` → `invoke_syscall` ghi `regs->regs[0] = nr` → parent đi `ret_to_user` → `kernel_exit 0` → `eret` → libc parent thấy **`x0 = PID con`**.

---

## 3. `copy_process()` — cái gì được tạo mới, cái gì share (tóm tắt; chi tiết Module 5)

|Bước|Hàm|fork() (không cờ)|Ảnh hưởng bởi|
|---|---|---|---|
|Bộ mô tả + kernel stack|`dup_task_struct(current)`|**task_struct mới** (memcpy từ parent) + **kernel stack 16KB mới** + `thread_info` mới. `p->stack` mới. (Module 6)|luôn|
|Credentials|`copy_creds`|`p->cred = p->real_cred = get_cred(current->cred)` (chia sẻ, refcount++; COW khi `setuid`)|`CLONE_NEWUSER`|
|Lập lịch|`sched_fork`|`p->state = TASK_NEW`; `__sched_fork` zero `se`; `p->prio = current->normal_prio`; `p->sched_class`; `p->se.vruntime` sẽ đặt ở `wake_up_new_task`; `preempt_count = FORK_PREEMPT_COUNT` (khoá preempt tới `schedule_tail`)|—|
|SysV sem|`copy_semundo`|dup|`CLONE_SYSVSEM`|
|File descriptors|`copy_files`|`dup_fd()` — **bảng fd mới**, nhưng mỗi `struct file*` **share** (refcount++). Offset file dùng chung. (Module 25)|`CLONE_FILES`|
|fs context|`copy_fs`|`copy_fs_struct()` — root/pwd/umask mới (copy)|`CLONE_FS`|
|Signal handlers|`copy_sighand`|`kmem_cache_alloc` bảng `action[64]` mới, memcpy|`CLONE_SIGHAND`|
|Signal state nhóm|`copy_signal`|`signal_struct` mới (nr_threads=1, shared_pending rỗng, copy `rlim[]`)|`CLONE_THREAD`|
|**Address space**|`copy_mm`|**`dup_mm()`** — mm mới, PGD mới, mọi VMA copy, PTE nhân đôi + write-protect vùng COW (mục 5)|`CLONE_VM`|
|Namespaces|`copy_namespaces`|share `nsproxy` (refcount++)|`CLONE_NEW*`|
|I/O context|`copy_io`|share/clone theo `CLONE_IO`|`CLONE_IO`|
|**Execution context**|`copy_thread`|dựng `pt_regs` + `cpu_context` cho con (mục 4)|luôn|
|PID|`alloc_pid`|**`struct pid` mới**, `pid` mới trong `pidmap`|`CLONE_NEWPID` (cấp trong ns con)|
|Liên kết|`attach_pid`, `list_add_tail_rcu(&p->tasks, ...)`, `list_add_tail(&p->sibling, &p->real_parent->children)`|thêm con vào process list + cây cha/con + pid hash|—|

---

## 4. `copy_thread()` trên ARM64 (chi tiết Module 8; ở đây phần cốt lõi)

`arch/arm64/kernel/process.c` (5.4 tên `copy_thread_tls`; 5.10 gộp lại `copy_thread`):

```c
int copy_thread(unsigned long clone_flags, unsigned long stack_start,
                unsigned long stk_sz, struct task_struct *p, unsigned long tls)
{
    struct pt_regs *childregs = task_pt_regs(p);   // = (pt_regs*)(p->stack + THREAD_SIZE) - 1
    memset(&p->thread.cpu_context, 0, sizeof(struct cpu_context));   // x19..x28, fp, sp, pc = 0
    fpsimd_flush_task_state(p);                    // con phải reload FP/SIMD lần đầu

    if (!(p->flags & PF_KTHREAD)) {                // ← fork(): user task
        *childregs = *current_pt_regs();           // COPY NỘI DUNG pt_regs của parent
        childregs->regs[0] = 0;                     // ◀── child fork() trả về 0
        *task_user_tls(p) = read_sysreg(tpidr_el0); // giữ TLS hiện tại của parent
        if (stack_start)                            // clone() có stack riêng (pthread) → set; fork(): stack_start==0 → bỏ
            childregs->sp = stack_start;
        if (clone_flags & CLONE_SETTLS)
            p->thread.uw.tp_value = tls;
    } else {                                        // kernel thread (Module 22)
        memset(childregs, 0, sizeof(struct pt_regs));
        childregs->pstate = PSR_MODE_EL1h;
        p->thread.cpu_context.x19 = stack_start;    // fn
        p->thread.cpu_context.x20 = stk_sz;         // arg
    }
    p->thread.cpu_context.pc = (unsigned long)ret_from_fork;   // ◀── con "tỉnh dậy" ở đây
    p->thread.cpu_context.sp = (unsigned long)childregs;       // ◀── kernel sp con = ngay tại frame pt_regs
    ptrace_hw_copy_thread(p);
    return 0;
}
```

Hai dòng cuối là "công tắc": lần đầu scheduler chọn con, `cpu_switch_to` nạp `cpu_context.sp` → `sp` con, `cpu_context.pc` → chỗ `ret` nhảy tới = `ret_from_fork`. `x19` = 0 (memset) → `ret_from_fork` biết đây là user task.

---

## 5. `mm_struct` được copy thế nào + thiết lập COW (chi tiết Module 7)

`copy_mm()` → (không `CLONE_VM`) → `dup_mm(tsk, current->mm)`:

```
dup_mm():
  mm = allocate_mm(); memcpy(mm, oldmm, sizeof(*mm));   // copy field thô
  mm_init(mm, tsk, mm->user_ns):
     mm->mm_users = 1; mm->mm_count = 1;
     mm->pgd = pgd_alloc(mm);        // PGD MỚI (arm64: trang zero từ pgd_cache — user pgd tách khỏi kernel)
     mm->map_count = 0; ...
  dup_mmap(mm, oldmm):
     down_write(&oldmm->mmap_sem); down_write(&mm->mmap_sem);
     for (mpnt = oldmm->mmap; mpnt; mpnt = mpnt->vm_next) {
         tmp = vm_area_dup(mpnt);           // copy struct vm_area_struct
         if (is_cow_mapping(tmp->vm_flags))  // private + có thể ghi
             tmp->vm_flags &= ~VM_LOCKED;
         tmp->vm_mm = mm;
         anon_vma_fork(tmp, mpnt);           // gắn reverse-mapping cho trang ẩn danh
         file_get(tmp->vm_file);
         __vma_link_rb(mm, tmp, ...);        // vào rbtree + list của mm con
         mm->map_count++;
         retval = copy_page_range(mm, oldmm, mpnt);   // ◀── nhân đôi PAGE TABLE
     }
     flush_tlb_mm(oldmm);                    // PTE ghi-được của parent vừa đổi → flush TLB parent
```

`copy_page_range` đi 4 mức PGD→PUD→PMD→PTE của **dải VA của VMA đó** trong `oldmm`, cấp bảng con tương ứng trong `mm` con, rồi với mỗi PTE hiện diện gọi `copy_one_pte()`:

```c
// mm/memory.c, rút gọn
if (is_cow_mapping(vm_flags) && pte_write(pte)) {
    ptep_set_wrprotect(src_mm, addr, src_pte);   // PARENT: xoá PTE_WRITE (arm64: set PTE_RDONLY)
    pte = pte_wrprotect(pte);                     // CHILD: cũng RO
}
if (!(vm_flags & VM_SHARED))
    pte = pte_mkclean(pte);
pte = pte_mkold(pte);                             // xoá bit young → ép fault kế cập nhật AF
get_page(page);                                   // refcount++
page_dup_rmap(page, false);                       // _mapcount++
set_pte_at(dst_mm, addr, dst_pte, pte);           // ghi PTE cho child
rss[mm_counter(page)]++;
```

Kết quả ngay sau `fork()`:

|Loại VMA|PTE parent|PTE child|Trang vật lý|
|---|---|---|---|
|private + writable (`.data`, `.bss`, heap, stack)|**RO** (write-protected)|**RO**|dùng chung, `refcount=2`|
|private RO (`.text`, `.rodata`)|RO (không đổi)|RO|dùng chung, `refcount=2`|
|`MAP_SHARED`|không đổi|copy y nguyên|dùng chung, `refcount=2`|

**COW page fault về sau** (parent hoặc child ghi vào trang RO trong VMA có `VM_WRITE`): `svc`… không, là **data abort (write)** → `el0_da` → `do_mem_abort` → `do_page_fault` → `handle_mm_fault` → `handle_pte_fault`: thấy `pte_present` nhưng `!pte_write` và `vma->vm_flags & VM_WRITE` và fault là ghi → **`do_wp_page()`**:

- `page_count(page) == 1` (bên kia đã COW đi rồi, hoặc là trang ẩn danh chỉ mình map) → **tái dùng**: `pte_mkwrite`, `set_pte`, `flush_tlb_page`. Không copy.
- `page_count(page) > 1` → **`wp_page_copy()`**: `alloc_page`, `cow_user_page` (copy nội dung), `page_add_new_anon_rmap(newpage)`, `set_pte` = writable trỏ newpage, `page_remove_rmap(oldpage)`, `put_page(oldpage)` (→ refcount--), `flush_tlb_page`, `mmu_notifier_invalidate_range`.

Nên "mm được copy thế nào": **cấu trúc (VMA tree + page tables) được copy ngay; nội dung trang chỉ copy khi có bên ghi.**

---

## 6. `wake_up_new_task()` — đưa con vào chạy

`kernel/sched/core.c`:

```c
void wake_up_new_task(struct task_struct *p)
{
    raw_spin_lock_irqsave(&p->pi_lock, flags);
    p->state = TASK_RUNNING;
    __set_task_cpu(p, select_task_rq(p, task_cpu(p), SD_BALANCE_FORK, 0));   // có thể chọn CPU khác parent
    rq = __task_rq_lock(p, &rf);
    activate_task(rq, p, ENQUEUE_NOCLOCK);        // → enqueue_task_fair → child vào rbtree CFS
                                                  //   place_entity(cfs_rq, se, initial=1): child nhận vruntime
                                                  //   ≈ min_vruntime + sched_vslice (một "phạt" nhỏ)
    p->on_rq = TASK_ON_RQ_QUEUED;
    check_preempt_curr(rq, p, WF_FORK);           // nếu child "đáng chạy hơn" → set TIF_NEED_RESCHED trên parent
    __task_rq_unlock(rq, &rf);
    raw_spin_unlock_irqrestore(&p->pi_lock, flags);
}
```

- `sysctl_sched_child_runs_first` mặc định **0** → parent thường chạy tiếp (về `ret_to_user`, `eret`, `x0=PID con`). Nếu `check_preempt_curr` thấy child nên chạy trước (vruntime nhỏ hơn đủ nhiều) → `TIF_NEED_RESCHED` được set → parent tới `ret_to_user` sẽ `schedule()` và có thể nhường CPU cho child ngay.
- Con vẫn **chưa chạy một lệnh nào** — chỉ nằm trong runqueue ở `TASK_RUNNING`.

---

## 7. Lần đầu con được chọn — `context_switch` → `ret_from_fork`

Ở một `__schedule()` nào đó (CPU nào đó), `pick_next_task` chọn con:

```
context_switch(rq, prev, child):
   switch_mm_irqs_off(prev->active_mm, child->mm, child):
       check_and_switch_context(child->mm)   → cấp ASID mới cho MM_child
       cpu_do_switch_mm: TTBR0_EL1 = phys(MM_child->pgd) ; TTBR1_EL1 = swapper | ASID_child
   switch_to(prev, child, prev) → __switch_to(prev, child):
       fpsimd_thread_switch(child); tls_thread_switch(child); ...
       cpu_switch_to(prev, child):
           stp x19..x28, fp, sp → prev->thread.cpu_context     // lưu prev
           str lr              → prev->thread.cpu_context.pc
           ldp x19..x28, fp, x9 ← child->thread.cpu_context     // x19..x28 = 0
           ldr lr              ← child->thread.cpu_context.pc = ret_from_fork
           mov sp, x9          ← child->thread.cpu_context.sp = childregs
           msr sp_el0, x1      // current = child
           ret                 // → nhảy vào ret_from_fork, sp trỏ đúng frame pt_regs của child
```

`ret_from_fork` (`arch/arm64/kernel/entry.S`):

```asm
SYM_CODE_START(ret_from_fork)
    bl   schedule_tail          // = finish_task_switch(prev): thả rq->lock, mmdrop nếu cần,
                                //   preempt_count về 0, xử lý prev TASK_DEAD...
    cbz  x19, 1f                // x19 == 0  → KHÔNG phải kernel thread
    mov  x0, x20               //   (kernel thread: x0 = arg)
    blr  x19                   //   (kernel thread: gọi fn(arg))
1:  get_current_task tsk
    b    ret_to_user            // ◀── user task: đi ra EL0
SYM_CODE_END(ret_from_fork)
```

`ret_to_user` → không có work pending (thường) → `kernel_exit 0`:

- `sp` đang trỏ **ngay tại `*childregs`** (vì `cpu_context.sp = childregs`).
- Nạp `x0..x30` từ `childregs`: **`x0 = 0`**, `x1..x30` = giá trị của parent lúc `svc`.
- `elr_el1 ← childregs->pc` = `&svc + 4` (copy từ parent).
- `spsr_el1 ← childregs->pstate` = PSTATE user của parent.
- `sp_el0 ← childregs->sp` = **cùng địa chỉ ảo SP user như parent** (nhưng ánh xạ tới trang vật lý khác sau COW).
- `add sp, sp, #S_FRAME_SIZE` → SP_EL1 con = đỉnh kernel stack con.
- `eret` → con ở EL0, tại lệnh sau `svc`, **`x0 = 0`**.

---

## 8. Parent vs Child — bảng khác biệt

||Parent|Child|
|---|---|---|
|`fork()` trả về (`x0`)|**PID con** (`nr` từ `_do_fork`)|**0** (do `copy_thread`: `childregs->regs[0] = 0`)|
|Đường code ra `eret`|`invoke_syscall` → `regs[0]=nr` → `ret_to_user` → `kernel_exit`|`ret_from_fork` → `schedule_tail` → `ret_to_user` → `kernel_exit`|
|PC trả về|lệnh sau `svc`|lệnh sau `svc` (giống hệt)|
|`x1..x30`, `pstate` user|như trước `svc`|**bản sao** của parent (qua `*childregs = *current_pt_regs()`)|
|SP user (địa chỉ ảo)|như trước|**cùng địa chỉ** (COW) — trang vật lý tách khi có bên ghi stack|
|Kernel stack|của parent (không đổi)|**mới, 16 KB** (`dup_task_struct` → `alloc_thread_stack_node`, VMAP)|
|`pt_regs`|frame gốc của parent|frame mới = **copy nội dung** + `regs[0]=0`|
|`task_struct`|`current`|mới; `p->pid` mới, `p->tgid = p->pid` (fork → thread group leader mới), `real_parent = parent = current`|
|`mm_struct`|`MM_parent` (PTE vùng COW đã bị write-protect)|**`MM_child`** = `dup_mm` (PGD mới, VMA copy, PTE COW read-only)|
|`TTBR0_EL1` / ASID|không đổi|mới (cấp ở `switch_mm` lần đầu)|
|`cpu_context`|được `cpu_switch_to` lưu khi parent bị switch away|`{ x19..x28=0, pc=ret_from_fork, sp=childregs }`|
|`files_struct`|`current->files`|bảng fd mới nhưng **cùng `struct file*`** (offset chia sẻ)|
|Thứ tự chạy|thường chạy tiếp ngay|chờ scheduler chọn (có thể sau, có thể trước nếu preempt)|

---

## 9. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|Entry point|`_do_fork(struct kernel_clone_args *)`|`kernel_clone()` (5.10)|
|arch hook|`copy_thread_tls(clone_flags, sp, sz, p, tls)`|`copy_thread(..., struct kernel_clone_args *)` (5.10)|
|`ret_from_fork`|`bl schedule_tail; cbz x19,1f; mov x0,x20; blr x19; 1: b ret_to_user`|gần như y hệt; 5.8+ thêm `scs`/PAC init trong `copy_thread`|
|PTE copy|`copy_page_range` → `copy_one_pte`|6.6+ tách `copy_present_pte` / batch COW; ý nghĩa như nhau|
|VMA tree|`mm_rb` rbtree + `vm_next`|maple tree (6.1)|
|`clone3()`|mới có (5.3)|ổn định|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `kernel_clone`, `copy_thread(..., args)`. Cơ chế "một `svc`, hai return", COW, `ret_from_fork` — giống 5.4.

---

✅ **MODULE 4 hoàn tất.** Sơ đồ: [docs/diagrams/fork-end-to-end-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/fork-end-to-end-arm64.md).

**Kế tiếp: MODULE 5 — `copy_process()` chi tiết từng bước** (mọi `copy_*`, clone flag nào tác động gì, object nào share/dup/lazy, refcount nào tăng; so sánh `fork` / `vfork` / `clone(CLONE_VM)` / `clone(CLONE_THREAD)`). Nói "tiếp" để làm Module 5.