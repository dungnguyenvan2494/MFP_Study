# MODULE 23 — `vfork()` trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/vfork-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/vfork-arm64.md) — 5 sơ đồ: (1) `fork` vs `vfork`, (2) đường đi `kernel_clone` với `CLONE_VFORK`, (3) con đánh thức cha qua `mm_release`, (4) timeline cha/con, (5) vì sao con chỉ được `_exit`/`execve`.

**File nguồn**: `kernel/fork.c` (`kernel_clone` / `_do_fork`, `copy_mm`, `wait_for_vfork_done`, `complete_vfork_done`, `mm_release` / `exit_mm_release` / `exec_mm_release`, `SYSCALL_DEFINE0(vfork)`), `fs/exec.c` (`exec_mmap` → `exec_mm_release`), `kernel/exit.c` (`exit_mm` → `exit_mm_release`), `arch/arm64/include/asm/mmu_context.h` (`deactivate_mm` = no-op), `arch/arm64/include/asm/unistd.h` (`__ARCH_WANT_SYS_VFORK`), `include/uapi/linux/sched.h` (`CLONE_VFORK 0x00004000`).

---

## 0. Ba câu trả lời ngắn

**`vfork()` là gì?** Đúng một dòng: `clone(CLONE_VM | CLONE_VFORK | SIGCHLD)`. Nó tạo một tiến trình con **dùng chung** `mm_struct` với cha (không copy page table, không COW), và **treo cha** cho tới khi con `execve()` hoặc `_exit()`.

**Vì sao cha bị block?** Vì con đang chạy trên **chính address space và chính khung stack của cha**. Nếu cha chạy song song, hai bên sẽ giẫm lên stack/heap của nhau. Nhân ép tuần tự bằng `wait_for_completion_killable()`: cha ngủ, con chạy, con gọi `execve`/`_exit` → `mm_release()` → `complete_vfork_done()` đánh thức cha.

**Con được làm gì?** Theo POSIX: **chỉ** (a) gán kết quả cho `pid_t`, (b) `_exit()`, (c) `execve()`. Mọi thứ khác — `return`, ghi biến, `malloc`, `printf` — là **undefined behavior** vì con mượn nguyên khung stack của cha.

---

## 1. `CLONE_VM` — chia sẻ address space

`copy_mm()` trong `kernel/fork.c` là nơi `CLONE_VM` có tác dụng:

```c
static int copy_mm(unsigned long clone_flags, struct task_struct *tsk)
{
    tsk->mm = NULL;
    tsk->active_mm = NULL;

    oldmm = current->mm;
    if (!oldmm)
        return 0;                       // (kernel thread — Module 22)

    if (clone_flags & CLONE_VM) {
        mmget(oldmm);                   // ◀── chỉ mm_users++
        mm = oldmm;                     // ◀── DÙNG LẠI mm của cha
        goto good_mm;
    }

    mm = dup_mm(tsk, current->mm);      // fork thường: copy mm_struct + toàn bộ page table
    ...
good_mm:
    tsk->mm = mm;
    tsk->active_mm = mm;
    return 0;
}
```

||`fork()` (không `CLONE_VM`)|`vfork()` (`CLONE_VM`)|
|---|---|---|
|`copy_mm` làm gì|`dup_mm()`: `kmem_cache_alloc` mm mới, `dup_mmap()` sao chép mọi `vm_area_struct`, `copy_page_range()` nhân đôi PGD→PTE|`mmget(oldmm); mm = oldmm`|
|Page table ARM64|PGD mới → 4 mức bảng trang mới; mọi PTE ghi được bị `ptep_set_wrprotect` cả hai phía (COW)|**không đụng gì**|
|`TTBR0_EL1` của con|trỏ PGD mới, ASID mới (`check_and_switch_context` cấp)|**giống hệt cha** — cùng PGD, cùng ASID|
|Con ghi vào biến|page fault → `do_wp_page` → `wp_page_copy` tách trang → cha không thấy|ghi thẳng vào RAM chung → **cha thấy**|
|Chi phý khởi tạo|~vài chục µs + N page fault sau đó|~0|

`CLONE_VM` **một mình** (không `CLONE_THREAD`) = "chia sẻ bộ nhớ nhưng vẫn là tiến trình riêng" — `tgid` khác, `signal_struct` khác, `files_struct` khác. `vfork` chính là trường hợp này cộng thêm `CLONE_VFORK`.

Chú ý `copy_thread()` (Module 8): với `vfork`, `newsp == 0` → nhánh `if (stack_start)` không chạy → `childregs->sp` **giữ nguyên** giá trị `sp` của cha. Con thật sự dùng chung con trỏ stack.

---

## 2. `CLONE_VFORK` — treo cha

Trong `kernel_clone()` (5.4 tên là `_do_fork()`):

```c
if (clone_flags & CLONE_VFORK) {
    p->vfork_done = &vfork;            // vfork = struct completion trên KERNEL STACK của cha
    init_completion(&vfork);
    get_task_struct(p);               // giữ task_struct con sống để cha còn deref được
}

wake_up_new_task(p);                  // con vào runqueue

if (clone_flags & CLONE_VFORK) {
    if (!wait_for_vfork_done(p, &vfork))
        ptrace_event_pid(PTRACE_EVENT_VFORK_DONE, pid);
}
```

`wait_for_vfork_done()`:

```c
static int wait_for_vfork_done(struct task_struct *child, struct completion *vfork)
{
    int killed;
    freezer_do_not_count();
    cgroup_enter_frozen();
    killed = wait_for_completion_killable(vfork);   // ◀── CHA NGỦ Ở ĐÂY
    cgroup_leave_frozen(false);
    freezer_count();

    if (killed) {                     // cha bị SIGKILL trong lúc chờ
        task_lock(child);
        child->vfork_done = NULL;     // ngắt con trỏ tới stack sắp biến mất
        task_unlock(child);
    }
    put_task_struct(child);           // nhả tham chiếu lấy ở trên
    return killed;
}
```

Điểm quan trọng:

- **`wait_for_completion_killable`**, không phải `_interruptible`: chỉ tín hiệu **chết** (SIGKILL) mới cắt được. Signal thường (`SIGINT`, `SIGTERM` có handler) **không** đánh thức cha — cha thật sự bị đóng băng.
- **`vfork_done` nằm trên kernel stack của cha.** Hợp lệ vì cha đang kẹt trong `kernel_clone()` — stack frame đó không biến mất. Con giữ con trỏ qua `task->vfork_done`.
- **`get_task_struct(p)` / `put_task_struct(child)`**: nếu con chết cực nhanh, `task_struct` của nó có thể bị free trước khi cha xử lý xong. Cặp get/put giữ nó sống qua đoạn `if (killed) child->vfork_done = NULL`.
- **`freezer_*` / `cgroup_*_frozen`**: đánh dấu để hệ thống suspend / cgroup-freezer không đếm cha là "task chưa đóng băng" (nó đang ngủ hợp pháp).

---

## 3. "Guarantee con chạy trước"

Với `vfork`, việc con chạy trước cha gần như **bắt buộc về mặt logic** (cha đang ngủ trên completion). Ngoài ra scheduler còn có `sysctl_sched_child_runs_first` và `place_entity()` (Module 12) cho con `vruntime` nhỏ hơn để nó được `pick_next_task` trước. Nhưng kể cả nếu cha được đánh thức giả (spurious), nó sẽ kiểm `vfork.done` thấy vẫn 0 và ngủ lại — `wait_for_completion` là vòng lặp chờ điều kiện, không phải "ngủ một lần".

---

## 4. Con đánh thức cha: `mm_release()`

Cả **hai** lối thoát của con đều đi qua `mm_release()`:

|Lối thoát|Đường trong nhân|
|---|---|
|`execve()` thành công|`fs/exec.c: exec_mmap(new_mm)` → `exec_mm_release(tsk, old_mm)` → `mm_release()`|
|`_exit()` / `exit_group()` / bị giết|`kernel/exit.c: do_exit()` → `exit_mm()` → `exit_mm_release(tsk, mm)` → `mm_release()`|

```c
static void mm_release(struct task_struct *tsk, struct mm_struct *mm)
{
    uprobe_free_utask(tsk);
    deactivate_mm(tsk, mm);            // ARM64: #define deactivate_mm(tsk,mm) do{}while(0)

    if (tsk->clear_child_tid) {        // CLONE_CHILD_CLEARTID (pthread_join) — không liên quan vfork
        if (... mm_users > 1) {
            put_user(0, tsk->clear_child_tid);
            do_futex(tsk->clear_child_tid, FUTEX_WAKE, 1, ...);
        }
        tsk->clear_child_tid = NULL;
    }

    if (tsk->vfork_done)              // ◀── CHỈ set khi cha gọi vfork
        complete_vfork_done(tsk);
}

static void complete_vfork_done(struct task_struct *tsk)
{
    struct completion *vfork;
    rcu_read_lock();
    vfork = tsk->vfork_done;
    if (likely(vfork)) {
        tsk->vfork_done = NULL;
        complete(vfork);              // ◀── ĐÁNH THỨC CHA
    }
    rcu_read_unlock();
}
```

- **`deactivate_mm` trên ARM64 là no-op.** Trên x86 nó phải xử lý LDT/TLS; ARM64 không cần → macro rỗng. Đây là chỗ "ARM64-specific" duy nhất đáng nói trong module này.
- **`complete()` an toàn với race.** Nếu con `complete()` **trước** khi cha kịp vào `wait_for_completion` (con chạy siêu nhanh), `completion.done` đã = 1, cha vào `wait_for_completion` thấy `done > 0` → trả về ngay, không ngủ. `struct completion` sinh ra chính để xử lý kiểu "sự kiện một lần" này — dùng waitqueue thô sẽ lỡ wakeup.

**Khi con `execve`:** ngay sau khi `bprm_mm_init()` cấp `mm` MỚI, `exec_mmap()` gọi `exec_mm_release()` trên `mm` CŨ (đang chia sẻ với cha). Cha thức. Từ giờ con có address space riêng — cha và con hoàn toàn độc lập. `mm_users` của mm cũ giảm về 1 (chỉ còn cha).

**Khi con `_exit`:** `exit_mm()` → `exit_mm_release()` → `complete_vfork_done()` → `mmput()` giảm `mm_users` về 1. Con thành zombie (`EXIT_ZOMBIE`), chờ cha `wait()`.

---

## 5. Timeline (đọc kèm sơ đồ 4)

```
mm_users(mm) = 1
  │
cha: vfork() ──► kernel_clone(CLONE_VM|CLONE_VFORK|SIGCHLD)
  │                 copy_mm: mmget(mm) ⇒ mm_users = 2
  │                 p->vfork_done = &vfork (kernel stack cha); get_task_struct(child)
  │                 wake_up_new_task(child)
  │
cha ──► wait_for_completion_killable(&vfork)   �midspan╗
  ▓▓▓▓▓▓▓▓ CHA ĐÓNG BĂNG ▓▓▓▓▓▓▓▓                     ║
                                     con chạy trên CHUNG mm + CHUNG sp
                                       │
                                       ├─ execve("/bin/x")  ──► exec_mmap: mm MỚI
                                       │                        exec_mm_release(mm cũ) ──► complete(&vfork)
                                       │                        mm_users(mm cũ) = 1
                                       └─ hoặc _exit(0) ──► exit_mm_release(mm) ──► complete(&vfork)
                                                            mm_users(mm) = 1 ; con → EXIT_ZOMBIE
  ▓▓▓▓▓▓▓▓ CHA THỨC ▓▓▓▓▓▓▓▓  ◄══════════════════════════════╝
  │
cha: ptrace_event(VFORK_DONE); put_task_struct(child); return PID_con
  │  (address space của cha nguyên vẹn vì con đã execve/exit, không sửa gì)
  │
cha: (nếu con chỉ _exit) waitpid(pid) ──► release_task(child)
```

---

## 6. Vì sao con "chỉ được `_exit()` hoặc `execve()`"

Con được trao **chính khung stack (SP) mà cha đang đứng** khi gọi `vfork()`. Hệ quả:

- **Con `return` khỏi hàm gọi `vfork()`** → nó pop/ghi đè khung stack đó → khi cha thức, địa chỉ trả về và biến cục bộ của cha đã hỏng → cha "return" vào rác → crash. Đây là lỗi kinh điển khiến `vfork` bị coi là nguy hiểm.
- **Con ghi biến** (kể cả `errno`, biến toàn cục, con trỏ) → cha thấy giá trị bị đổi (chung RAM).
- **Con `malloc` / `printf` (stdio có buffer) / mở `FILE*`** → làm bẩn heap và cấu trúc libc mà cha sẽ tiếp tục dùng.
- **`exit()` (không phải `_exit()`)** cũng cấm: `exit()` chạy các handler `atexit()` và flush buffer stdio → đụng bộ nhớ chung.

Hợp lệ: đổi thanh ghi, và các syscall **không đụng bộ nhớ tiến trình** — `dup2`, `close`, `open`, `chdir`, `setuid`, `signal`, `nice`… (đúng những thứ bạn cần làm giữa fork và exec). Đây chính xác là lý do POSIX thêm **`posix_spawn()`**: một API bọc `vfork`+`exec` an toàn, khai báo trước danh sách "file actions" để nhân/libc thực hiện thay vì để bạn tự viết code chạy trong con.

---

## 7. `vfork` trên ARM64 cụ thể

- `arch/arm64/include/asm/unistd.h` có `#define __ARCH_WANT_SYS_VFORK` → `SYSCALL_DEFINE0(vfork)` được biên dịch. Nhưng bảng syscall **AArch64 native** (`include/uapi/asm-generic/unistd.h`) **không** có `__NR_vfork` — chỉ bảng **AArch32 compat** (`arch/arm64/include/asm/unistd32.h`) có `__NR_vfork 190`.
- ⇒ Chương trình 64-bit trên ARM64: glibc `vfork()` gọi thẳng `clone(CLONE_VM | CLONE_VFORK | SIGCHLD, sp=0, ...)` qua `__NR_clone` (x8 = 220). `SYSCALL_DEFINE0(vfork)` chỉ phục vụ tiến trình 32-bit.
- `deactivate_mm` = no-op (đã nói ở mục 4).
- `copy_thread` nhánh user: `*childregs = *current_pt_regs()`, `childregs->regs[0] = 0` (con thấy `vfork` trả 0), `newsp==0` nên `childregs->sp` = `sp` của cha.

---

## 8. Điểm dễ nhầm

|Nhầm|Thực tế|
|---|---|
|"`vfork` không tạo tiến trình mới"|Có — `task_struct` mới, `tgid` mới, PID mới. Chỉ `mm_struct` là chung.|
|"Cha và con cùng chạy"|Không. Cha bị `wait_for_completion_killable` treo tới khi con `execve`/`_exit`.|
|"Con `return` được nếu cẩn thận"|Không. Undefined behavior — khung stack của cha bị phá.|
|"`SIGTERM` đánh thức cha đang chờ vfork"|Không, chỉ `SIGKILL` (`_killable`).|
|"`execve` thất bại thì sao?"|Con vẫn ở address space chung. Nó **phải** `_exit()` ngay (glibc/`posix_spawn` làm vậy). Nếu con `return` để báo lỗi → hỏng stack cha.|
|"`vfork` nhanh hơn `fork` nhiều"|Trên nhân hiện đại có COW, khoảng cách nhỏ; lợi ích chính là với tiến trình có address space **rất lớn** (nhiều VMA) hoặc hệ không MMU.|
|"`clear_child_tid` trong `mm_release` liên quan vfork"|Không — đó là cho `CLONE_CHILD_CLEARTID` (đánh thức `pthread_join`). `mm_release` gộp nhiều việc.|

---

## 9. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản mới|
|---|---|---|
|Hàm fork lõi|`_do_fork(struct kernel_clone_args *)`|`kernel_clone()` (đổi tên ở 5.10)|
|`mm_release`|một hàm `mm_release(tsk, mm)` gọi trực tiếp từ exec và exit|tách vỏ `exec_mm_release()` / `exit_mm_release()` (5.9), bên trong vẫn gọi `mm_release`|
|`exit_mm()` chữ ký|`exit_mm(struct task_struct *tsk)`|`exit_mm(void)` (luôn dùng `current`)|
|Chờ vfork|`freezer_do_not_count()` / `freezer_count()`|thêm `cgroup_enter_frozen()` / `cgroup_leave_frozen()` (cgroup v2 freezer, ~5.7)|
|`PTRACE_EVENT_VFORK` / `VFORK_DONE`|đã có|không đổi|
|Ngữ nghĩa `CLONE_VM` + `CLONE_VFORK`|như trên|**không đổi** — API vfork ổn định nhiều năm|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: đã xác nhận `kernel_clone`, `exec_mm_release`/`exit_mm_release`, `exit_mm(void)`, `cgroup_enter_frozen()`, `SYSCALL_DEFINE0(vfork){ .flags = CLONE_VFORK | CLONE_VM }`. Cơ chế `vfork_done` + `struct completion` + `mmget`/`mmput` giống hệt 5.4.