## MODULE 25 — File Descriptor Subsystem

Sơ đồ: [docs/diagrams/file-descriptor-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/file-descriptor-arm64.md) — 6 sơ đồ: chuỗi `task_struct`→đĩa, `open`→`fd_install`, `copy_files`/`dup_fd`, ví dụ ghi chung `f_pos`, close-on-exec, fast-path tra cứu fd.

**File nguồn**: `include/linux/fdtable.h` (`struct files_struct`, `struct fdtable`), `include/linux/fs.h` (`struct file`, `struct inode`, `struct super_block`), `kernel/fork.c` (`copy_files`), `fs/file.c` (`dup_fd`, `get_unused_fd_flags`, `__alloc_fd`, `fd_install`, `__close_fd`, `do_close_on_exec`, `__fget`/`__fget_light`), `fs/open.c` (`do_sys_open`), `fs/read_write.c` (`vfs_write` — `f_pos`).

### 1. Ba tầng

```
fd (số int)  →  struct file  →  struct inode  →  struct super_block
per-task        "open file       "file thật"      "một filesystem
                 description"      trong inode      mount"
                 có f_pos          cache
```

- **`fd`** chỉ là **chỉ số vào một mảng con trỏ**. `fd 3` nghĩa là `current->files->fdt->fd[3]`.
- **`struct file`** = "một lần mở". Giữ `f_pos` (con trỏ đọc/ghi), `f_flags` (`O_APPEND`, `O_NONBLOCK`…), `f_mode`, `f_cred` (cred lúc mở), `f_op` (bảng hàm), `f_count` (refcount). **Hai `open()` cùng file → hai `struct file`**, `f_pos` độc lập.
- **`struct inode`** = file trên đĩa. Một object trong inode cache cho mỗi file thật, dù có 100 `struct file` mở nó. Giữ `i_size`, `i_uid`, `i_mode`, `i_rwsem`, `i_mapping` (page cache của file).

### 2. `struct files_struct` và `struct fdtable`

`task_struct.files` → `struct files_struct`:

- `count` (atomic) — số task **chia sẻ** bảng này. `fork` → tạo bản mới (`count=1`); `CLONE_FILES` → `count++`.
- `fdt` (RCU) — trỏ tới `fdtab` (nhúng sẵn, 64 fd) hoặc một `fdtable` lớn hơn cấp động khi vượt 64.
- `fd_array[64]`, `open_fds_init[1]`, `close_on_exec_init[1]` — lưu trữ nhúng để tiến trình nhỏ không cần cấp phát riêng.
- `next_fd` — gợi ý bắt đầu quét tìm fd trống.

`struct fdtable`:

- `max_fds` — kích thước mảng hiện tại.
- `fd` — con trỏ tới **mảng `struct file *`**. `fd[i]` là `struct file` gắn với fd `i`, hoặc `NULL`.
- `open_fds` — bitmap: fd nào đang chiếm.
- `close_on_exec` — bitmap: fd nào có `O_CLOEXEC`.
- `full_fds_bits` — bitmap tăng tốc (mỗi bit tóm tắt "64 fd này đầy chưa").

### 3. `open()` cấp fd thế nào

1. `get_unused_fd_flags(flags)` → `__alloc_fd`: quét `open_fds` từ `next_fd`, lấy **fd nhỏ nhất còn trống**, `set_bit(fd, open_fds)`. Nếu `flags & O_CLOEXEC` → `set_bit(fd, close_on_exec)`. Lúc này fd đã "được đặt chỗ" nhưng `fd[fd]` vẫn `NULL`.
2. `path_openat` → giải path → `dentry` → `inode` → cấp `struct file`: `f_pos = 0`, `f_flags = flags`, `f_mode`, `f_cred = get_current_cred()`, `f_op = inode->i_fop`, `f_mapping = inode->i_mapping`.
3. `fd_install(fd, file)` → `rcu_assign_pointer(fdt->fd[fd], file)`. Giờ fd "trỏ" file. Trả `fd` về user.

`close(fd)` → `__close_fd`: `fdt->fd[fd] = NULL`, `clear_bit(open_fds)`, `filp_close` → **`fput(file)`** (`f_count--`). Khi `f_count` về 0 → `__fput`: gọi `f_op->release`, `dput`, `mntput`, giải phóng `struct file`. `inode` **không** bị giải phóng — nó nằm trong cache đến khi bị thu hồi vì thiếu RAM.

### 4. `fork()` — `copy_files()` / `dup_fd()`

```c
static int copy_files(unsigned long clone_flags, struct task_struct *tsk)
{
    oldf = current->files;
    if (clone_flags & CLONE_FILES) {
        atomic_inc(&oldf->count);      // THREAD: dùng chung nguyên bảng
        goto out;
    }
    newf = dup_fd(oldf, NR_OPEN_MAX, &error);   // PROCESS: bản mới
    tsk->files = newf;
}
```

`dup_fd()`:

- Cấp `files_struct` mới, `count = 1`.
- Nếu số fd đang mở > 64 → `alloc_fdtable()` cấp bảng lớn hơn.
- `copy_fd_bitmaps` — sao chép `open_fds`, `close_on_exec`, `full_fds_bits`.
- **Vòng lặp qua mọi fd đang mở**: `f = old->fd[i]; if (f) get_file(f);` (tức `f_count++`), rồi `rcu_assign_pointer(new->fd[i], f)`.
- Phần mảng còn lại `memset` = `NULL`.

**Kết quả**: con có **mảng fd riêng** (đóng/mở fd của con không ảnh hưởng cha), nhưng mỗi ô trỏ **cùng `struct file`** với cha → chung `f_pos`, chung `f_flags`. `f_count` của mỗi file mở tăng thêm 1.

`CLONE_FILES` (pthread): dùng chung nguyên `files_struct` → thread A `open()` được fd 5 thì thread B thấy fd 5 ngay lập tức.

### 5. Ví dụ: `open()` rồi `fork()`, hai bên `write`

```
parent: fd = open("/log", O_WRONLY)   → struct file: f_pos=0, f_count=1
parent: fork()                         → dup_fd: get_file(file) ⇒ f_count=2
                                          child->fd[fd] = CÙNG struct file

parent: write(fd, "AAAA", 4)   → vfs_write: pos = file->f_pos (0); ghi; file->f_pos = 4
child : write(fd, "BBBB", 4)   → vfs_write: pos = file->f_pos (4); ghi; file->f_pos = 8

File trên đĩa: "AAAABBBB"   ← KHÔNG đè nhau vì f_pos DÙNG CHUNG
```

Trái lại, nếu **không fork** mà cha và con mỗi bên `open("/log")` riêng → **hai `struct file`**, `f_pos` độc lập, cả hai ghi từ offset 0 → "BBBB" đè "AAAA".

Đây là lý do `>>` trong shell (`prog >> log`) dùng `O_APPEND`: mỗi `write` khoá `inode->i_rwsem`, đặt `f_pos = i_size` ngay trước khi ghi → an toàn kể cả nhiều tiến trình độc lập.

### 6. `execve()` và close-on-exec

`begin_new_exec` (5.4: `flush_old_exec`) gọi `do_close_on_exec(current->files)`:

```c
for (i = 0; ...; i++) {
    set = fdt->close_on_exec[i] & fdt->open_fds[i];
    for (mỗi bit set) { filp_close(fd); fdt->fd[fd] = NULL; __clear_open_fd(fd); }
}
```

- fd **không** `O_CLOEXEC` → **sống qua execve**. Chương trình mới thừa hưởng: `stdin/stdout/stderr`, pipe, socket mà shell đã chuẩn bị.
- fd **có** `O_CLOEXEC` → đóng ngay trước khi ELF mới chạy → không rò rỉ fd nhạy cảm (log file, socket đặc quyền) vào chương trình con.

`files_struct` **không bị thay** khi execve (khác `mm`, khác `cred`, khác `sighand`). `dup2(oldfd, newfd)` và `fcntl(F_DUPFD)` tạo fd mới **luôn không có `O_CLOEXEC`** (kể cả khi `oldfd` có) — đó chính là cách shell làm redirect: `dup2(pipe_wr, 1)` rồi `execve` → lệnh con ghi stdout vào pipe.

### 7. Fast path tra cứu fd

```c
static unsigned long __fget_light(unsigned int fd, fmode_t mask)
{
    struct files_struct *files = current->files;
    if (atomic_read(&files->count) == 1) {          // KHÔNG ai chia sẻ bảng
        file = files_lookup_fd_raw(files, fd);       // chỉ 1 lần đọc mảng, KHÔNG get_file()
        return (unsigned long)file;
    } else {
        file = __fget(fd, mask, 1);                  // rcu_read_lock + get_file_rcu
        return FDPUT_FPUT | (unsigned long)file;
    }
}
```

Nếu `count == 1` (không `CLONE_FILES`), bảng fd không thể đổi dưới chân → `fdget` khỏi cần refcount, cực rẻ. Có thread chia sẻ → phải RCU + `get_file_rcu` + `fput` khi xong.

### Điểm dễ nhầm

|Nhầm|Thực tế|
|---|---|
|"`fork` copy `struct file`"|Không — copy **mảng fd**, `get_file` (refcount++) cùng `struct file`.|
|"cha/con có `f_pos` riêng sau fork"|Chung. Cùng `struct file` = cùng `f_pos`.|
|"hai `open()` cùng file chung offset"|Không — hai `struct file`, offset độc lập, chung `inode`.|
|"`execve` reset bảng fd"|Không — chỉ đóng fd `O_CLOEXEC`. Phần còn lại nguyên vẹn.|
|"`dup2` giữ `O_CLOEXEC` của nguồn"|Không — fd mới luôn **không** `O_CLOEXEC`.|
|"`CLONE_FILES` = `fork` với fd"|`CLONE_FILES` chung `files_struct`; `fork` tạo bản mới. Khác nhau khi một bên `open`/`close` sau đó.|
|"đóng fd cuối cùng xoá inode"|Xoá `struct file`. `inode` ở lại cache; chỉ bị free khi unlink + không còn ai mở, hoặc bị thu hồi.|

### Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản mới|
|---|---|---|
|Tra cứu fd|`__fcheck_files()` / `fcheck_files()`|đổi tên `files_lookup_fd_raw` / `_rcu` (5.11)|
|`close_range(2)`|**chưa có**|5.9 — đóng dải `[min,max]`, cờ `CLOSE_RANGE_CLOEXEC`/`UNSHARE`|
|Đóng CLOEXEC lúc exec|từ `flush_old_exec`|từ `begin_new_exec()` (5.9)|
|`f_count`|`atomic_long_t`|`file_ref` (6.7)|
|`pidfd` qua fd|sơ khai (`pidfd_open` 5.3)|mở rộng nhiều|
|`dup_fd`/`get_file`/`f_pos` chia sẻ|như mô tả|**không đổi**|

> Tree local = **5.10.241**: xác nhận `files_lookup_fd_raw`, `dup_fd(oldf, NR_OPEN_MAX, &error)`, `copy_files` nhánh `CLONE_FILES`, `fd_install`. Cơ chế giống 5.4 (5.4 dùng `__fcheck_files`).