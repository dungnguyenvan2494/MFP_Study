## MODULE 24 — Credentials

Sơ đồ: [docs/diagrams/credentials-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/credentials-arm64.md) — 5 sơ đồ: nội dung `struct cred`, `real_cred` vs `cred`, macro truy cập + RCU, `prepare_creds`→`commit_creds`, vòng đời qua fork/execve/setuid.

**File nguồn**: `include/linux/cred.h` (`struct cred`, macro `current_cred`/`current_euid`/`__task_cred`), `kernel/cred.c` (`prepare_creds`, `commit_creds`, `copy_creds`, `override_creds`/`revert_creds`, `abort_creds`), `kernel/sys.c` (`__sys_setuid`, `__sys_setresuid`, `__sys_setfsuid`), `fs/exec.c` (`prepare_bprm_creds`, `bprm_fill_uid`, `install_exec_creds`), `security/commoncap.c` (`cap_bprm_set_creds` / `cap_bprm_creds_from_file`).

### 1. `struct cred` là gì

Một struct gom **toàn bộ danh tính bảo mật** của tiến trình:

- **4 UID + 4 GID**: `uid/euid/suid/fsuid` và `gid/egid/sgid/fsgid`.
- **5 tập capability**: `cap_permitted`, `cap_effective`, `cap_inheritable`, `cap_bset` (bounding set), `cap_ambient`.
- **keyring**: `session_keyring`, `process_keyring`, `thread_keyring`.
- **con trỏ phụ**: `user` (`struct user_struct` — đếm số process của user, giữ rlimit), `user_ns` (user namespace mà uid/cap tương đối vào), `group_info` (nhóm phụ), `security` (nhãn LSM: SELinux context, AppArmor profile).
- `usage` — refcount; `rcu` — hook để giải phóng trễ.

**Tính chất cốt lõi: `struct cred` bất biến sau khi commit.** Một khi object đã gắn vào `task_struct`, không dòng code nào được sửa field của nó nữa. Muốn đổi quyền → tạo object mới. Nhờ vậy mọi reader (kể cả tiến trình khác đọc qua `/proc`, `kill`, `ptrace`) chạy **không khoá** bằng RCU: con trỏ mà nó đọc được hoặc trỏ object cũ hoàn chỉnh, hoặc object mới hoàn chỉnh — không bao giờ "nửa nọ nửa kia".

### 2. Bốn UID — tại sao cần nhiều thế

- **`uid` (real)**: user thực sự. Dùng cho kế toán, `getuid()`, `RLIMIT_NPROC`, và ai được phép gửi tín hiệu cho ai.
- **`euid` (effective)**: danh tính **mọi kiểm tra quyền dùng**. `inode_permission`, mở file, `kill`, `ptrace`, IPC — tất cả so với `euid`. Đây là "tôi đang là ai" về mặt quyền lực.
- **`suid` (saved)**: bản cất của `euid` **trước khi hạ quyền**. Chương trình set-uid-root khởi động với `euid = 0`, muốn làm việc thường thì `seteuid(real_uid)` để bỏ quyền tạm, khi cần lại `seteuid(0)` — nhân cho phép vì `0 == suid`. Không có `suid` thì "bỏ tạm rồi lấy lại" bất khả.
- **`fsuid`**: danh tính cho thao tác **file system** (VFS đọc field này chứ không phải `euid`). Gần như luôn bằng `euid`; `setfsuid()` tách riêng để NFS server ngày xưa phục vụ file thay client mà không mở cửa cho client gửi tín hiệu giết nó.

`kuid_t` vs `uid_t`: `uid_t` là con số user thấy **trong một user namespace**. `kuid_t` là "khoá" toàn cục sau khi ánh xạ về `init_user_ns` (`make_kuid(ns, uid)`). Nhân **luôn** so sánh bằng `kuid_t` (`uid_eq()`), không so số thô — nếu không, uid 0 trong container sẽ bị nhầm là root thật.

### 3. Hai con trỏ: `real_cred` và `cred`

`task_struct` giữ **hai**:

```c
const struct cred __rcu  *real_cred;  /* objective: task này THUỘC VỀ ai */
const struct cred __rcu  *cred;       /* subjective: task đang HÀNH ĐỘNG với tư cách ai */
```

- `current_cred()` đọc `cred` (chủ quan).
- Tiến trình khác nhìn vào task này (`kill`, `/proc/<pid>/status`, `ptrace`) đọc `real_cred` (khách quan) qua `__task_cred(task)`.
- **Bình thường hai con trỏ trỏ cùng một object.** Chỉ tách khi nhân "mượn danh tính": `override_creds(new)` đặt `current->cred = new` nhưng giữ nguyên `real_cred`; `revert_creds(old)` trả lại. Dùng bởi: nfsd (phục vụ theo danh tính client), overlayfs (truy cập lớp dưới bằng mounter's creds), io_uring (worker chạy với creds của submitter), `access(2)` (kiểm tra bằng **real** uid, không phải effective).

### 4. Truy cập — macro và RCU

```c
current_cred()          // = rcu_dereference_protected(current->cred, 1)
                        //   AN TOÀN không cần rcu_read_lock — chỉ current tự sửa cred mình
current_euid()          // = current_cred()->euid   (đọc thẳng)
current_cap()           // = current_cred()->cap_effective

__task_cred(task)       // = rcu_dereference(task->real_cred)
                        //   đọc cred TASK KHÁC → PHẢI trong rcu_read_lock()
task_euid(task)         // = task_cred_xxx(task, euid)
                        //   = { rcu_read_lock(); v = __task_cred(task)->euid; rcu_read_unlock(); v; }
get_task_cred(task)     // tăng refcount → giữ cred task khác sống ra ngoài vùng RCU
```

Vì `current` là tác giả duy nhất được sửa `current->cred`, đọc cred **của chính mình** không cần đồng bộ. Đọc cred **của task khác** phải rào RCU vì task đó có thể `commit_creds()` bất cứ lúc nào.

### 5. Mẫu đổi quyền: `prepare_creds` → sửa → `commit_creds`

```c
struct cred *new = prepare_creds();        // kmem_cache_alloc + memcpy từ current->cred;
                                           // usage=1, get_group_info/get_uid/get_user_ns/key_get,
                                           // security_prepare_creds() (LSM clone nhãn)
if (!new) return -ENOMEM;

new->euid = new->fsuid = kuid;             // sửa BẢN COPY tuỳ ý
... điều chỉnh cap_* nếu cần ...

if (security_task_fix_setuid(new, old, ...) < 0) {   // LSM veto
    abort_creds(new);                      // put_cred(new) — task KHÔNG bị đụng
    return -EPERM;
}
return commit_creds(new);                  // rcu_assign_pointer(task->real_cred, new);
                                           // rcu_assign_pointer(task->cred, new);
                                           // put_cred(old); put_cred(old);
```

`commit_creds()` ngoài đổi con trỏ còn: nếu `euid/egid/fsuid` hoặc capability đổi → `set_dumpable(mm, suid_dumpable)` (tắt core dump / chặn ptrace bởi user cũ), xoá `pdeath_signal`, đặt `smp_wmb()` (rào để "không dumpable" thấy trước "đổi cred", tránh race với `__ptrace_may_access`). Nó **chỉ đổi được cred của `current`** — không có API đổi cred tiến trình khác. Đó là lý do mọi thay đổi quyền đều tự nguyện, phát từ syscall của chính task.

### 6. Vòng đời qua `fork`

`copy_creds(p, clone_flags)`:

- **`CLONE_THREAD` (tạo thread)**: `p->real_cred = p->cred = get_cred(current->cred)` — **chia sẻ** đúng object với các thread anh em. `user->processes++`.
- **Không `CLONE_THREAD` (tạo process)**: `new = prepare_creds()` — **copy** cred của cha; nếu `CLONE_NEWUSER` thì `create_user_ns(new)`; `process_keyring` **không** thừa kế (chỉ chung trong một process). Con có `struct cred` riêng nhưng **giá trị y hệt cha** (cùng uid/gid/caps).

Hệ quả: `fork()` không đổi quyền gì cả — con và cha cùng uid. Thread thì dùng chung object nên (ở tầng nhân) `setuid` của một thread chỉ đổi `task->cred` của thread đó; glibc phải broadcast để POSIX-hoá.

### 7. Vòng đời qua `execve` — set-uid binary

1. `prepare_bprm_creds()` → `bprm->cred = prepare_exec_creds()` (copy từ `current`, reset `thread_keyring`).
2. `prepare_binprm()` → `bprm_fill_uid(bprm, file)`:
    - Bỏ qua nếu: `no_new_privs` bật, mount có `nosuid`, hoặc uid chủ file không map trong user_ns.
    - File có bit `S_ISUID` → `bprm->cred->euid = inode->i_uid` (ví dụ `/usr/bin/passwd` chủ là root → `euid` mới = 0).
    - File có `S_ISGID | S_IXGRP` → `bprm->cred->egid = inode->i_gid`.
    - `bprm->per_clear |= PER_CLEAR_ON_SETID` (xoá vài bit personality).
3. `cap_bprm_set_creds()` (LSM commoncap) — tính `cap_permitted/effective/ambient` mới từ **file capabilities** (xattr `security.capability`), giao với `cap_bset`, cộng `cap_ambient`. Đây là cách `ping` không cần suid-root mà vẫn mở raw socket (`cap_net_raw` gắn trên file).
4. `security_bprm_check` — SELinux domain transition, v.v.
5. `install_exec_creds(bprm)` (5.4) / trong `begin_new_exec()` (mới hơn):
    - Tính `bprm->secureexec`: nếu `euid != uid` hoặc caps tăng → đặt `AT_SECURE=1` trong auxv → glibc bỏ qua `LD_PRELOAD`, `LD_LIBRARY_PATH`… (chống leo thang qua biến môi trường).
    - `commit_creds(bprm->cred)` — **danh tính mới có hiệu lực từ đây**. `bprm->cred = NULL`.

`task_struct`, PID, `files_struct` (trừ FD có `O_CLOEXEC`), cwd — **giữ nguyên** qua execve. Chỉ `cred` (và `mm`, `signal handlers`) bị thay.

### 8. Vòng đời qua `setuid(uid)` — `__sys_setuid`

```c
new = prepare_creds();
if (ns_capable_setid(old->user_ns, CAP_SETUID)) {          // đang là root (hoặc có CAP_SETUID)
    new->suid = new->uid = kuid;
    if (!uid_eq(kuid, old->uid))
        set_user(new);                                     // đổi user_struct, kiểm RLIMIT_NPROC
}                                                          // → HẠ QUYỀN VĨNH VIỄN: ghi cả suid
else if (uid_eq(kuid, old->uid) || uid_eq(kuid, old->suid)) {
    /* không root: chỉ được hoán euid về real hoặc saved */
}
else
    return -EPERM;

new->fsuid = new->euid = kuid;
return commit_creds(new);                                  // hoặc abort_creds nếu LSM từ chối
```

- **Là root** gọi `setuid(1000)` → cả `uid`, `euid`, `suid`, `fsuid` = 1000. **Không lấy lại được root** (vì `suid` cũng bị ghi). Đây là cách daemon "vứt quyền dứt khoát".
- **Không phải root** gọi `setuid(x)` → chỉ được nếu `x == uid` hoặc `x == suid`; chỉ đổi `euid`/`fsuid`. Dùng để nhảy qua lại giữa real và saved.
- Muốn hạ tạm rồi lấy lại → dùng `seteuid()` (chỉ đụng `euid`, để `suid` nguyên).

### Điểm dễ nhầm

|Nhầm|Thực tế|
|---|---|
|"Sửa `current->cred->euid` trực tiếp là được"|Cấm tuyệt đối — cred bất biến. Phải `prepare_creds`/`commit_creds`.|
|"`fork` có thể đổi quyền con"|Không. `fork` chỉ copy/chia sẻ cred. Đổi quyền là việc của `execve` (suid) hoặc `set*id()`.|
|"kiểm tra quyền dùng `uid`"|Dùng `euid` (và `fsuid` cho VFS). `uid` chỉ để lấy lại quyền và kế toán.|
|"`setuid` non-root luôn `-EPERM`"|Không — cho phép nếu đích là `uid` hoặc `suid` hiện tại.|
|"thread có cred riêng"|`CLONE_THREAD` → chia sẻ **cùng object** `struct cred`.|
|"`real_cred` đổi khi ptrace"|Không — đó là `task->parent`. `real_cred` chỉ đổi khi chính task `commit_creds`.|
|"suid binary luôn chạy euid 0"|Chỉ nếu chủ file là root **và** mount không `nosuid` **và** không `no_new_privs`.|

### Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản mới|
|---|---|---|
|Lắp cred vào exec|`install_exec_creds(bprm)`|gộp vào `begin_new_exec()` (5.9)|
|Tính cap từ file|`cap_bprm_set_creds()`|`cap_bprm_creds_from_file()` (5.8)|
|`cap_effective` khi exec|field rời trên `linux_binprm`|dời hẳn vào `bprm->cred`|
|`override_creds` họ hàng|như 5.4|thêm `override_creds_light`/`revert_creds_light` cho io_uring (6.x)|
|ngữ nghĩa `struct cred` + 4 UID|như mô tả|**không đổi** — API POSIX ổn định lâu năm|

> Tree local = **5.10.241**: xác nhận `bprm_fill_uid` (`fs/exec.c`), `commit_creds(bprm->cred)` tại `fs/exec.c:1368`, `__sys_setuid`. Cơ chế `prepare_creds`/`commit_creds`/`copy_creds` + 2 con trỏ giống hệt 5.4.