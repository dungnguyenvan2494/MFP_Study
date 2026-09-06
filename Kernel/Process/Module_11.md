# MODULE 11 — ELF Loader trên Linux 5.4 / ARM64 (ví dụ PIE)

Sơ đồ đã lưu: [docs/diagrams/elf-loader-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/elf-loader-arm64.md) (4 sơ đồ: file→bộ nhớ qua `load_bias`, ba lần ASLR độc lập, layout stack ban đầu, `start_thread`→`eret`→EL0).

**File nguồn**: `fs/binfmt_elf.c` (`load_elf_binary`, `load_elf_phdrs`, `elf_map`, `load_elf_interp`, `total_mapping_size`, `set_brk`, `create_elf_tables`), `arch/arm64/include/asm/elf.h` (`ELF_ET_DYN_BASE`, `ELF_HWCAP`, `SET_PERSONALITY`, `ELF_PLAT_INIT`), `arch/arm64/kernel/vdso.c` (`arch_setup_additional_pages`), `arch/arm64/include/asm/processor.h` (`start_thread`), `arch/arm64/kernel/process.c` (`arch_randomize_brk`), `mm/util.c` (`arch_mmap_rnd`).

**Ví dụ**: `/bin/program` là **PIE** — `e_type = ET_DYN`, có `PT_INTERP` = `/lib/ld-linux-aarch64.so.1`, linked-động (`libc.so.6`). Gọi `execve("/bin/program", {"program","arg1"}, {"PATH=/usr/bin"})`.

---

## 0. Ba loại ELF, ba cách nạp

|`e_type`|Là gì|`load_bias`|`PT_INTERP`?|
|---|---|---|---|
|`ET_EXEC`|Non-PIE (địa chỉ tuyệt đối, cũ)|**0** (`MAP_FIXED` tại `p_vaddr` gốc)|có (nếu linked-động)|
|`ET_DYN` **có** `PT_INTERP`|**PIE** — chương trình bình thường ngày nay|`ELF_ET_DYN_BASE + arch_mmap_rnd()` (`MAP_FIXED`)|**có** → nạp `ld.so`|
|`ET_DYN` **không** `PT_INTERP`|Chính là `ld.so` / static-PIE|**0** → kernel chọn địa chỉ trong vùng mmap|không|

Ví dụ của ta là dòng giữa. `ELF_ET_DYN_BASE = 2 * TASK_SIZE_64 / 3`. Với VA 48-bit, `TASK_SIZE_64 = 1 << 48` → `ELF_ET_DYN_BASE ≈ 0x0000_AAAA_AAAA_A000`. Đây là lý do binary PIE hay xuất hiện quanh `0xaaaa…` trong `/proc/pid/maps`.

---

## 1. Đọc ELF header (chưa đụng process)

```c
// load_elf_binary(), fs/binfmt_elf.c — bprm->buf đã có 256 byte đầu từ prepare_binprm()
loc->elf_ex = *((struct elfhdr *)bprm->buf);

if (memcmp(elf_ex->e_ident, ELFMAG, SELFMAG) != 0)      goto out;   // "\x7fELF"
if (elf_ex->e_type != ET_EXEC && elf_ex->e_type != ET_DYN) goto out;
if (!elf_check_arch(elf_ex))                            goto out;   // e_machine == EM_AARCH64
if (elf_ex->e_ident[EI_CLASS] != ELFCLASS64)           goto out;   // 64-bit
if (!bprm->file->f_op->mmap)                            goto out;

elf_phdata = load_elf_phdrs(elf_ex, bprm->file);   // đọc e_phnum × e_phentsize byte program headers
```

`struct elfhdr` (`Elf64_Ehdr`) — trường quan trọng:

|Trường|Ví dụ|Ý nghĩa|
|---|---|---|
|`e_entry`|`0x640`|Entry point, **tính như thể nạp ở địa chỉ 0**|
|`e_phoff`|`0x40`|Offset của program header table trong file|
|`e_phnum`|`9`|Số program header|
|`e_phentsize`|`56`|`sizeof(Elf64_Phdr)`|

Tới đây mọi lỗi vẫn `return -ENOEXEC` an toàn — chương trình cũ nguyên vẹn.

---

## 2. `PT_INTERP` → mở interpreter

```c
for (i = 0; i < elf_ex->e_phnum; i++) {
    if (elf_ppnt->p_type == PT_INTERP) {
        elf_interpreter = kmalloc(elf_ppnt->p_filesz, GFP_KERNEL);
        // đọc chuỗi từ file tại p_offset:
        retval = elf_read(bprm->file, elf_interpreter, elf_ppnt->p_filesz, elf_ppnt->p_offset);
        // elf_interpreter = "/lib/ld-linux-aarch64.so.1"
        interpreter = open_exec(elf_interpreter);   // struct file * của ld.so
        // đọc ELF header của ld.so → loc->interp_elf_ex
        interp_elf_phdata = load_elf_phdrs(&loc->interp_elf_ex, interpreter);
    }
}
```

Kernel giờ có **hai** file: chương trình (`bprm->file`) và ld.so (`interpreter`).

Các `PT_*` khác quét được:

- `PT_GNU_STACK` — `p_flags` không có `PF_X` → stack **không thực thi** (`executable_stack = EXSTACK_DISABLE_X`).
- `PT_GNU_RELRO` — vùng sẽ được `mprotect` thành read-only sau relocation.
- `PT_DYNAMIC` — bảng `.dynamic` (ld.so đọc, kernel bỏ qua).

---

## 3. `flush_old_exec` + `setup_new_exec` (Module 10 — điểm không quay lại)

`de_thread` → `exec_mmap(bprm->mm)` (lắp `mm` mới, `mmput` mm cũ) → `flush_thread` → `flush_signal_handlers` → `setup_new_exec` (ASLR `mmap_base`, `comm`, `set_mm_exe_file`) → `install_exec_creds` (`commit_creds`).

`SET_PERSONALITY2(*elf_ex, &arch_state)` (arm64, `elf.h`): xóa `TIF_32BIT` (chạy AArch64), set `PF_RANDOMIZE` nếu `randomize_va_space != 0`.

---

## 4. Dựng stack: `setup_arg_pages`

VMA stack tạm (đang ở `STACK_TOP_MAX`, chứa các chuỗi argv/envp đã copy ở `prepare_binprm`) được **dời** về vị trí cuối:

```c
retval = setup_arg_pages(bprm, randomize_stack_top(STACK_TOP), executable_stack);
```

- `STACK_TOP` = `TASK_SIZE` (đỉnh vùng user).
- `randomize_stack_top(STACK_TOP)` = `STACK_TOP - (arch_mmap_rnd() & ~PAGE_MASK)` — **lần ASLR thứ 3**, độc lập với program và với mmap.
- Đặt `vm_flags = VM_STACK_FLAGS | VM_GROWSDOWN`, bỏ cờ tạm `VM_STACK_INCOMPLETE_SETUP`.
- Cập nhật `bprm->p` (đỉnh stack), `mm->arg_start`.

---

## 5. `PT_LOAD` → `mmap` + `load_bias`

### 5.1. Tính `load_bias` (chạy MỘT LẦN, cho `PT_LOAD` đầu tiên)

```c
// load_elf_binary(), nhánh ET_DYN + có interpreter
if (interpreter) {
    load_bias = ELF_ET_DYN_BASE;                    // ≈ 0xAAAA_AAAA_A000
    if (current->flags & PF_RANDOMIZE)
        load_bias += arch_mmap_rnd();               // + tối đa ~1 GB, page-granular  ← ASLR #1
    elf_flags |= MAP_FIXED;                         // ép nạp đúng địa chỉ này
} else
    load_bias = 0;                                  // (ld.so chạy trực tiếp)

load_bias = ELF_PAGESTART(load_bias - vaddr);       // vaddr của PT_LOAD đầu = 0 → không đổi
total_size = total_mapping_size(elf_phdata, e_phnum);  // span từ vaddr đầu tới hết PT_LOAD cuối
```

`arch_mmap_rnd()` (arm64): `(get_random_long() & ((1UL << mmap_rnd_bits) - 1)) << PAGE_SHIFT`, `mmap_rnd_bits` mặc định 18.

Giả sử `arch_mmap_rnd()` trả `0x0B3C_0000` → `load_bias = 0xAAAA_AB3C_0000`.

### 5.2. `elf_map()` từng `PT_LOAD`

```c
for (i = 0, elf_ppnt = elf_phdata; i < e_phnum; i++, elf_ppnt++) {
    if (elf_ppnt->p_type != PT_LOAD) continue;

    elf_prot  = make_prot(elf_ppnt->p_flags);       // PF_R→PROT_READ, PF_W→PROT_WRITE, PF_X→PROT_EXEC
    elf_flags = MAP_PRIVATE | MAP_DENYWRITE | MAP_EXECUTABLE;
    if (load_addr_set) elf_flags |= MAP_FIXED;      // các segment sau: FIXED tương đối với load_bias

    error = elf_map(bprm->file, load_bias + elf_ppnt->p_vaddr, elf_ppnt,
                    elf_prot, elf_flags, total_size);
    // total_size chỉ dùng cho segment đầu: reserve cả span rồi vm_munmap phần thừa
    // → các segment không đè lên thứ khác và giữ khoảng cách tương đối đúng

    if (!load_addr_set) {
        load_addr_set = 1;
        load_addr = elf_ppnt->p_vaddr - elf_ppnt->p_offset + load_bias;
    }
}
```

`elf_map()` (`fs/binfmt_elf.c`):

```c
size = eppnt->p_filesz + ELF_PAGEOFFSET(eppnt->p_vaddr);
off  = eppnt->p_offset  - ELF_PAGEOFFSET(eppnt->p_vaddr);
addr = ELF_PAGESTART(addr);
map_addr = vm_mmap(filep, addr, size, prot, type, off);   // → do_mmap → VMA file-backed MAP_PRIVATE
```

Kết quả cho ví dụ:

|`PT_LOAD`|file|VA (= `load_bias + p_vaddr`)|prot|VMA|
|---|---|---|---|---|
|#1 (text)|`0x0..0x734`|`0xAAAA_AB3C_0000`|`R-X`|`.text` `.plt` — file-backed|
|#2 (data)|`0xdb8..0x1020`|`0xAAAA_AB3C_D000`|`RW-`|`.data` — file-backed, `MAP_PRIVATE` (ghi = COW)|

### 5.3. `.bss` — phần `p_memsz > p_filesz`

```c
// set_brk(elf_bss, elf_brk, bss_prot) trong load_elf_binary
elf_bss  = load_bias + <cuối p_filesz của PT_LOAD#2>;   // 0xAAAA_AB3C_D268
elf_brk  = load_bias + <cuối p_memsz  của PT_LOAD#2>;   // 0xAAAA_AB3C_D270

set_brk(elf_bss, elf_brk, bss_prot):
    start = ELF_PAGEALIGN(elf_bss);   // 0xAAAA_AB3C_E000
    end   = ELF_PAGEALIGN(elf_brk);
    if (end > start)
        vm_brk_flags(start, end - start, VM_...);   // ◀── VMA ẨN DANH zero-fill cho đuôi bss
padzero(elf_bss);   // zero phần lẻ trang ở ranh giới file↔bss (0xD268..0xE000)
```

### 5.4. Heap

```c
mm->brk = mm->start_brk = arch_randomize_brk(mm);   // arch/arm64/kernel/process.c
// = mm->brk_base + (get_random_long() & STACK_RND_MASK) << PAGE_SHIFT   (≤ ~32 MB, nếu randomize_va_space >= 2)
```

`mm->start_code/end_code/start_data/end_data` = các mốc `+ load_bias`.

---

## 6. Nạp interpreter (`ld.so`)

```c
if (interpreter) {
    elf_entry = load_elf_interp(&loc->interp_elf_ex, interpreter, load_bias, interp_elf_phdata);
    interp_load_addr = elf_entry;                       // = địa chỉ nền ld.so
    elf_entry += interp_elf_ex->e_entry;                // = entry tuyệt đối của ld.so
}
```

`load_elf_interp()`:

- `total_size = total_mapping_size(interp_elf_phdata, ...)`.
- Map các `PT_LOAD` của ld.so. Segment đầu: `elf_map(interpreter, load_addr + vaddr, ...)` với `load_addr` khởi đầu 0 → `get_unmapped_area()` chọn địa chỉ trong **vùng mmap**, top-down, ngẫu nhiên qua `mm->mmap_base` — **ASLR #2, độc lập với ASLR #1 của program**.
- Trả về `load_addr` = lượng relocation cho ld.so.

Giả sử `interp_load_addr = 0xFFFF_9C1A_0000`, `ld.so::e_entry = 0x1cc0` → `elf_entry = 0xFFFF_9C1A_1CC0`.

**Vì sao hai ASLR tách rời**: để lệnh `./ld.so ./program` (test ld.so mới) chạy được — ld.so lúc đó là `ET_DYN` không có `PT_INTERP`, phải nạp **xa** program để không đè nhau. Nên "loader" luôn vào vùng mmap, "program" vào vùng `ELF_ET_DYN_BASE`.

---

## 7. vDSO: `arch_setup_additional_pages()`

```c
// arch/arm64/kernel/vdso.c
retval = arch_setup_additional_pages(bprm, uses_interp);
```

- `_install_special_mapping()` cho `[vvar]` (trang dữ liệu kernel export — `vdso_data`) và `[vdso]` (ELF nhỏ chứa code) vào vùng mmap.
- `mm->context.vdso = (void *)vdso_base`.
- vDSO chứa: `__kernel_gettimeofday`, `__kernel_clock_gettime`, `__kernel_clock_getres`, `__kernel_rt_sigreturn`. glibc parse `[vdso]` để gọi các hàm giờ **không cần syscall**.

---

## 8. `create_elf_tables()` — dựng `argc/argv/envp/auxv` trên stack

```c
// từ bprm->p đi xuống
p = arch_align_stack(bprm->p);

// 1. đẩy 16 byte ngẫu nhiên
get_random_bytes(k_rand_bytes, sizeof(k_rand_bytes));
u_rand_bytes = (elf_addr_t __user *)STACK_ALLOC(p, 16);
copy_to_user(u_rand_bytes, k_rand_bytes, 16);

// 2. dựng mảng auxv trong buffer kernel
#define NEW_AUX_ENT(id, val) do { elf_info[ei_index++] = id; elf_info[ei_index++] = val; } while (0)
NEW_AUX_ENT(AT_HWCAP,   ELF_HWCAP);
NEW_AUX_ENT(AT_PAGESZ,  ELF_EXEC_PAGESIZE);        // 4096
NEW_AUX_ENT(AT_CLKTCK,  CLOCKS_PER_SEC);           // 100 (USER_HZ)
NEW_AUX_ENT(AT_PHDR,    load_addr + exec->e_phoff);// địa chỉ program headers TRONG BỘ NHỚ
NEW_AUX_ENT(AT_PHENT,   sizeof(struct elf_phdr));  // 56
NEW_AUX_ENT(AT_PHNUM,   exec->e_phnum);            // 9
NEW_AUX_ENT(AT_BASE,    interp_load_addr);         // địa chỉ nền ld.so
NEW_AUX_ENT(AT_FLAGS,   0);
NEW_AUX_ENT(AT_ENTRY,   exec->e_entry + load_bias);// entry của CHƯƠNG TRÌNH (ld.so nhảy tới sau)
NEW_AUX_ENT(AT_UID,  ...); NEW_AUX_ENT(AT_EUID, ...);
NEW_AUX_ENT(AT_GID,  ...); NEW_AUX_ENT(AT_EGID, ...);
NEW_AUX_ENT(AT_SECURE,  bprm->secureexec);         // 1 nếu setuid/setgid/caps đổi
NEW_AUX_ENT(AT_RANDOM,  (elf_addr_t)u_rand_bytes); // con trỏ tới 16 byte ở bước 1
NEW_AUX_ENT(AT_HWCAP2,  ELF_HWCAP2);
NEW_AUX_ENT(AT_EXECFN,  bprm->exec);               // con trỏ tới chuỗi đường dẫn
NEW_AUX_ENT(AT_PLATFORM,(elf_addr_t)u_platform);   // "aarch64"
NEW_AUX_ENT(AT_SYSINFO_EHDR, current->mm->context.vdso);  // địa chỉ [vdso]
NEW_AUX_ENT(AT_NULL, 0);

// 3. copy_to_user xuống stack: argc, argv[], NULL, envp[], NULL, auxv[]
```

### Layout stack cuối cùng (`_start` đọc từ `[sp]`)

```
địa chỉ THẤP  ← sp
┌─────────────────────────────┐
│ argc = 2                    │
│ argv[0] → "/bin/program"    │
│ argv[1] → "arg1"            │
│ NULL                        │
│ envp[0] → "PATH=/usr/bin"   │
│ NULL                        │
│ AT_PHDR   , 0xAAAA_AB3C_0040 │
│ AT_PHENT  , 56              │
│ AT_PHNUM  , 9               │
│ AT_BASE   , 0xFFFF_9C1A_0000 │  ← ld.so tự relocate bằng cái này
│ AT_ENTRY  , 0xAAAA_AB3C_0640 │  ← ld.so nhảy tới sau khi setup
│ AT_RANDOM , &(16 byte dưới) │  ← glibc seed stack canary từ đây
│ AT_SYSINFO_EHDR, 0xFFFF_9C1C_0000 │ ← địa chỉ [vdso]
│ AT_HWCAP  , 0x...           │  ← glibc ifunc chọn memcpy/strlen tối ưu
│ AT_HWCAP2 , 0x...           │
│ AT_SECURE , 0               │  ← nếu 1: ld.so bỏ LD_PRELOAD/LD_LIBRARY_PATH
│ AT_PAGESZ , 4096            │
│ AT_NULL   , 0               │
│ ── dữ liệu chuỗi ──         │
│ "/bin/program\0" "arg1\0"   │
│ "PATH=/usr/bin\0" "aarch64\0"│
│ 16 byte AT_RANDOM           │
└─────────────────────────────┘
địa chỉ CAO   (đỉnh stack = STACK_TOP − rnd)
```

`mm->start_stack = bprm->p` (địa chỉ của `argc`).

### Các `AT_*` mà prompt hỏi

|Auxv|Giá trị|Ai dùng, để làm gì|
|---|---|---|
|**`AT_PHDR`**|`load_bias + e_phoff`|ld.so tìm `PT_DYNAMIC` của **chương trình** mà không cần mở lại file|
|**`AT_PHENT`** / **`AT_PHNUM`**|56 / 9|duyệt bảng program header|
|**`AT_ENTRY`**|`load_bias + e_entry`|entry của **chương trình**; ld.so `br` tới đây sau khi relocate + nạp lib + chạy `init_array`|
|**`AT_BASE`**|`interp_load_addr`|địa chỉ ld.so được nạp → ld.so tự relocate chính nó|
|**`AT_RANDOM`**|con trỏ tới 16 byte|glibc `_dl_random` → seed `__stack_chk_guard` (stack canary) và `__pointer_chk_guard`|
|**`AT_SYSINFO_EHDR`**|địa chỉ `[vdso]`|glibc parse vDSO → `clock_gettime`, `gettimeofday`, `getcpu` chạy **không syscall**|
|**`AT_HWCAP`** / **`AT_HWCAP2`**|bitmask CPU (FP, ASIMD, AES, PMULL, SHA, CRC32, ATOMICS, LSE…)|ifunc resolver trong glibc chọn `memcpy`/`strcmp`/AES tối ưu cho CPU thực|
|**`AT_SECURE`**|0 (1 nếu đổi đặc quyền)|1 ⇒ ld.so bỏ `LD_PRELOAD`, `LD_LIBRARY_PATH`, `GLIBC_TUNABLES` từ env|
|**`AT_PAGESZ`**|4096|`sysconf(_SC_PAGESIZE)` không cần syscall|

---

## 9. `start_thread()` — đặt PC / SP / PSTATE cho EL0

### Chọn `elf_entry`

```c
if (interpreter)
    elf_entry = interp_load_addr + interp_elf_ex->e_entry;   // = _start của ld.so
else
    elf_entry = elf_ex->e_entry + load_bias;                 // = _start của chương trình
```

Ví dụ PIE-dynamic → `elf_entry = 0xFFFF_9C1A_1CC0` (ld.so).

### `start_thread()` (`arch/arm64/include/asm/processor.h`)

```c
static inline void start_thread_common(struct pt_regs *regs, unsigned long pc)
{
    s32 previous_syscall = regs->syscallno;
    memset(regs, 0, sizeof(*regs));          // x0..x30 = 0 ; sp = pc = pstate = 0 ; orig_x0 = 0
    regs->syscallno = previous_syscall;      // giữ lại (để xử lý -ERESTART của chính execve nếu cần)
    regs->pc = pc;                           // ◀── PC
    if (system_uses_irq_prio_masking())
        regs->pmr_save = GIC_PRIO_IRQON;
}

static inline void start_thread(struct pt_regs *regs, unsigned long pc, unsigned long sp)
{
    start_thread_common(regs, pc);
    regs->pstate = PSR_MODE_EL0t;            // ◀── PSTATE
    regs->sp = sp;                           // ◀── SP   (= bprm->p, trỏ argc)
}
```

`ELF_PLAT_INIT(regs, load_addr)` (`elf.h`) → `regs->regs[0] = 0` (arm64 không truyền `_dl_fini` qua thanh ghi; glibc lấy qua cơ chế khác).

### Ba trường được đặt

|Trường `pt_regs`|Giá trị|Sau `eret` trở thành|
|---|---|---|
|**`pc`**|`elf_entry` (ld.so hoặc `_start`)|`ELR_EL1` → `PC` của EL0|
|**`sp`**|`bprm->p` = đỉnh stack user mới (trỏ `argc`)|`SP_EL0`|
|**`pstate`**|`PSR_MODE_EL0t`|`SPSR_EL1` → `PSTATE` của EL0|

### `PSR_MODE_EL0t` là gì

`PSR_MODE_EL0t = 0x00000000`. Sau `memset`, `pstate = 0`, rồi `|= PSR_MODE_EL0t` (vẫn 0). Nghĩa là:

|Trường PSTATE|Giá trị|Ý nghĩa|
|---|---|---|
|`M[3:0]`|`0b0000`|**EL0**, chọn `SP_EL0` ('t' = thread SP; EL0 luôn dùng SP_EL0)|
|`M[4]` (nRW)|`0`|**AArch64** (không phải AArch32)|
|`DAIF`|`0000`|**Debug/SError/IRQ/FIQ đều BẬT** — chương trình user nhận ngắt bình thường|
|`NZCV`|`0000`|cờ điều kiện sạch|
|`SS`, `IL`|`0`|không single-step|

→ Một trạng thái EL0 "sạch": AArch64, mọi ngắt bật, chưa có cờ nào set.

---

## 10. `eret` → chạy ở EL0

`load_elf_binary` return 0 → call chain hàm C unwind trên kernel stack (không bị `exec_mmap` đụng — Module 6/10). `ret_to_user` → `kernel_exit 0`:

```asm
ldr   x21, [sp, #S_PC]       // = regs->pc = elf_entry
ldr   x22, [sp, #S_PSTATE]  // = regs->pstate = PSR_MODE_EL0t
ldr   x23, [sp, #S_SP]      // = regs->sp = đỉnh stack user
msr   sp_el0, x23
msr   elr_el1, x21
msr   spsr_el1, x22
ldp   x0, x1, [sp, #16*0]   // ldp toàn bộ x0..x30 = 0
...
add   sp, sp, #S_FRAME_SIZE
eret
```

Phần cứng `eret`:

- `PSTATE ← SPSR_EL1` (`PSR_MODE_EL0t`) → CPU chuyển **EL0**, AArch64, IRQ bật.
- `PC ← ELR_EL1` = `elf_entry` = `_start` của ld.so.
- `SP` đang dùng ← `SP_EL0` = đỉnh stack user, trỏ `argc`.

**Ở EL0:**

1. `ld.so::_start` (assembly nhỏ): đọc `argc`/`argv`/`envp`/`auxv` từ `[sp]`, gọi `_dl_start`.
2. ld.so tự relocate (dùng `AT_BASE`), map các thư viện `DT_NEEDED` (`libc.so.6` → `mmap`), xử lý `PT_GNU_RELRO` (`mprotect` read-only), chạy `init_array` của các lib.
3. ld.so `br` tới `AT_ENTRY` = `_start` của `/bin/program`.
4. `program::_start` gọi `__libc_start_main(main, argc, argv, init, fini, rtld_fini, sp)`.
5. `__libc_start_main` chạy `__libc_csu_init` (constructors), rồi gọi `main(argc, argv, envp)`.

---

## 11. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|`create_elf_tables` chữ ký|`(bprm, &elf_ex, load_addr, interp_load_addr)`; `AT_PHDR` tính inline|thêm tham số `e_entry`, `phdr_addr` (5.10)|
|Segment `PT_LOAD` đầu|`MAP_FIXED`|`MAP_FIXED_NOREPLACE` (an toàn hơn, 5.4-ish backport)|
|`maximum_alignment()` cho `load_bias`|không có|5.10 (căn theo `p_align` lớn nhất)|
|`load_elf_interp` chữ ký|có `*interp_map_addr` out-param|bỏ, tính lại (5.10)|
|`arch_setup_additional_pages`|vdso + vvar (+ sigpage cho compat)|y hệt|
|`start_thread`|`start_thread_common` + `pstate` + `sp`|+ `spectre_v4_enable_task_mitigation` (5.10)|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `create_elf_tables(bprm, elf_ex, interp_load_addr, e_entry, phdr_addr)`, có `maximum_alignment`, `MAP_FIXED_NOREPLACE`. Cơ chế `load_bias`, hai ASLR độc lập, layout `argc/argv/envp/auxv`, `start_thread` đặt `pc/sp/pstate` — **giống 5.4**.