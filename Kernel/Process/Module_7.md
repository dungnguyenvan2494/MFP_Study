# MODULE 7 — `fork()` & Copy-On-Write trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/fork-cow-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/fork-cow-arm64.md) (4 sơ đồ: cây gọi, `copy_one_pte`, ví dụ 4 bước qua page table + trang vật lý, cây quyết định `do_wp_page`).

**File nguồn**: `kernel/fork.c` (`copy_mm`, `dup_mm`, `dup_mmap`, `vm_area_dup`), `mm/memory.c` (`copy_page_range`, `copy_p4d/pud/pmd/pte_range`, `copy_one_pte` [5.4] / `copy_present_pte` [5.10], `do_wp_page`, `wp_page_copy`, `wp_page_reuse`, `cow_user_page`), `arch/arm64/mm/fault.c` (`do_page_fault`), `arch/arm64/include/asm/pgtable.h` (`pte_wrprotect`, `ptep_set_wrprotect`, `pte_mkwrite`), `arch/arm64/include/asm/tlbflush.h`.

---

## 0. Ý tưởng COW trong một câu

`fork()` **không copy trang dữ liệu**. Nó copy **bảng trang** rồi đặt PTE của **cả cha lẫn con** thành **read-only** (dù VMA vẫn cho phép ghi). Trang vật lý chỉ thật sự được nhân bản khi một trong hai bên **ghi** → page fault → `do_wp_page`. Bên ghi trước phải copy; bên ghi sau (còn một mình map trang) được tái dùng tại chỗ.

`is_cow_mapping(vm_flags)` = `(vm_flags & (VM_SHARED | VM_MAYWRITE)) == VM_MAYWRITE` → **private + có thể ghi**. Đúng cho `.data`, `.bss`, heap, stack, và cả `.text` (private, `mprotect` được → `VM_MAYWRITE` set) — nhưng `.text` có `pte_write == 0` sẵn nên write-protect là no-op.

---

## 1. `copy_mm(clone_flags, p)` — `kernel/fork.c`

```c
p->mm = NULL; p->active_mm = NULL;
oldmm = current->mm;
if (!oldmm) return 0;                       // kernel thread
vmacache_flush(p);
if (clone_flags & CLONE_VM) {               // thread → CHIA SẺ, không COW
    mmget(oldmm);                           // oldmm->mm_users++
    p->mm = p->active_mm = oldmm;
    return 0;
}
mm = dup_mm(p, current->mm);                // fork → nhân bản
p->mm = p->active_mm = mm;
```

---

## 2. `dup_mm()` → `dup_mmap()` — `kernel/fork.c`

```c
mm = allocate_mm();
memcpy(mm, oldmm, sizeof(*mm));             // copy field thô (start_code, brk, ...)
mm_init(mm, tsk, mm->user_ns):
    mm->mm_users = 1; mm->mm_count = 1;
    mm->pgd = pgd_alloc(mm);                // ◀── PGD MỚI (arm64: 1 trang zero từ pgd_cache)
    mm->map_count = 0; ...
dup_mmap(mm, oldmm):
    down_write(&oldmm->mmap_sem);
    down_write_nested(&mm->mmap_sem, SINGLE_DEPTH_NESTING);
    ...
    for (mpnt = oldmm->mmap; mpnt; mpnt = mpnt->vm_next) {
        if (mpnt->vm_flags & VM_DONTCOPY) { ...; continue; }   // vd vùng driver không được kế thừa
        tmp = vm_area_dup(mpnt);                               // copy struct vm_area_struct
        if (tmp->vm_flags & VM_WIPEONFORK)
            tmp->anon_vma = NULL;                              // con thấy zero, KHÔNG copy_page_range
        else if (anon_vma_fork(tmp, mpnt))                     // rmap ngược cho trang ẩn danh
            goto fail;
        tmp->vm_flags &= ~(VM_LOCKED | VM_LOCKONFAULT);        // con không kế thừa mlock
        tmp->vm_mm = mm;
        get_file(tmp->vm_file);                                // file->f_count++
        if (tmp->vm_flags & VM_DENYWRITE) deny_write_access(file);
        __vma_link_rb(mm, tmp, prev, rb_link, rb_parent);      // vào rbtree + list của mm con
        mm->map_count++;
        if (!(tmp->vm_flags & VM_WIPEONFORK))
            retval = copy_page_range(tmp, mpnt);               // ◀── nhân đôi PAGE TABLE
    }
    arch_dup_mmap(oldmm, mm);
    flush_tlb_mm(oldmm);                                       // ◀── TLB maintenance #1
    up_write(&mm->mmap_sem);
    up_write(&oldmm->mmap_sem);
```

Sau vòng lặp này, `mm` con có: **PGD mới**, **cây VMA giống hệt** cha (mỗi VMA là `struct vm_area_struct` mới), và **bảng trang đã điền**, nhưng mọi PTE vùng COW đều RO.

`anon_vma_fork`: gắn `tmp` vào chuỗi `anon_vma` để về sau khi swap/migrate một trang ẩn danh, kernel tìm được **mọi** PTE (cả cha lẫn con) trỏ tới trang đó.

---

## 3. `copy_page_range()` → `copy_one_pte()` — `mm/memory.c`

`copy_page_range(dst_vma, src_vma)` gọi lồng: `copy_p4d_range` → `copy_pud_range` → `copy_pmd_range` → `copy_pte_range`. Ở mỗi mức, nếu bảng con của cha tồn tại và bảng con của con chưa có → cấp bảng mới (`pud_alloc`, `pmd_alloc`, `pte_alloc`). Ở mức PTE, với **mỗi PTE hiện diện** của cha:

```c
// copy_one_pte() (5.4) — trọng tâm
pte_t pte = *src_pte;                         // PTE của PARENT tại addr
struct page *page = vm_normal_page(vma, addr, pte);

if (page) {
    get_page(page);                           // page->_refcount++   (2)
    page_dup_rmap(page, false);               // page->_mapcount++   (giờ 2 mm map trang này)
    rss[mm_counter(page)]++;                  // MM_ANONPAGES/MM_FILEPAGES của CHILD
}

/* --- write-protect CẢ HAI nếu là COW mapping --- */
if (is_cow_mapping(vm_flags) && pte_write(pte)) {
    ptep_set_wrprotect(src_mm, addr, src_pte);   // PARENT PTE: xoá PTE_WRITE, set PTE_RDONLY (atomic trên PTE sống)
    pte = pte_wrprotect(pte);                     // giá trị sẽ ghi cho CHILD: cũng RO
}
if (vm_flags & VM_SHARED)
    pte = pte_mkclean(pte);
pte = pte_mkold(pte);                             // xoá Access Flag → fault kế cập nhật "young"

set_pte_at(dst_mm, addr, dst_pte, pte);           // ghi PTE cho CHILD
```

**Trên ARM64**, `pte_wrprotect(pte)` = `clear PTE_WRITE (bit DBM) & set PTE_RDONLY (AP[2], bit 7)`. `ptep_set_wrprotect` làm y vậy nhưng **nguyên tử trên PTE đang sống** của cha (dùng `cmpxchg` để không mất bit dirty do phần cứng vừa set).

Sau `copy_page_range` toàn bộ `mm`, `dup_mmap` gọi `flush_tlb_mm(oldmm)`: **ARM64**: `flush_tlb_mm` → `__tlbi(aside1is, ASID(oldmm) << 48)` — xoá **mọi entry TLB gắn ASID của cha** trên **mọi CPU** (Inner Shareable, không cần IPI). Bắt buộc: nếu không, entry "ghi được" cũ của cha còn trong TLB → cha ghi mà **không** fault → hỏng COW.

---

## 4. Ví dụ cụ thể — `heap[0]` tại `VA = 0x0000_aaaa_1234_5000`

Giả định:

- `MM_parent`: PGD_parent, `ASID = 7`.
- PTE ban đầu: `PFN = 0x40100` (phys `0x4010_0000`), `PTE_WRITE=1`, `PTE_RDONLY=0`, `USER`, `nG=1`, `AF=1`.
- Trang `0x40100`: `_refcount = 1`, `_mapcount = 0`, `byte[0] = 0xAB`.

### (a) TRƯỚC `fork()`

```
MM_parent PTE(0xaaaa12345000) → PFN 0x40100  RW USER
Phys 0x40100: refcount=1  mapcount=0  byte[0]=0xAB
```

### (b) SAU `fork()` (`copy_one_pte` cho PTE này + `flush_tlb_mm(MM_parent)`)

```
is_cow_mapping(heap)=true,  pte_write=true  →  write-protect cả hai:
  ptep_set_wrprotect(MM_parent) : PTE parent → PFN 0x40100  RO (PTE_WRITE=0, PTE_RDONLY=1)
  pte_wrprotect(pte)            : PTE child  → PFN 0x40100  RO,  AF=0 (old)
  get_page + page_dup_rmap     : Phys 0x40100 refcount=2  mapcount=1

MM_parent PTE(0xaaaa12345000) → 0x40100  RO
MM_child  PTE(0xaaaa12345000) → 0x40100  RO
Phys 0x40100: refcount=2  mapcount=1  byte[0]=0xAB

TLB: tlbi aside1is, ASID=7   (xoá entry RW cũ của parent)
```

(Bảng trang của con: `PGD_child → PUD → PMD → PTE` — các bảng con này **là bản mới**, chỉ _nội dung PTE_ trỏ tới cùng PFN với cha.)

### (c) SAU khi **PARENT** ghi `heap[0] = 0xCD`

1. CPU thực thi `strb w1, [x0]` với `x0 = 0xaaaa_1234_5000`. TLB miss (vừa flush) → walk → PTE `PTE_RDONLY=1` → **permission fault khi ghi**.
2. `ESR_EL1`: `EC = 0x24` (DABT_LOW), `WnR = 1` (write), `DFSC = 0b001111` (permission fault, level 3). `FAR_EL1 = 0xaaaa_1234_5000`.
3. `el0_sync` → `el0_da` → `do_mem_abort` → `do_page_fault(far, esr, regs)` (`arch/arm64/mm/fault.c`):
    - `vma = find_vma(mm, far)` → heap VMA. `vm_flags & VM_WRITE` → cho phép ghi (không phải SIGSEGV).
    - `mm_flags = FAULT_FLAG_WRITE | FAULT_FLAG_USER | FAULT_FLAG_ALLOW_RETRY | FAULT_FLAG_KILLABLE`.
    - `handle_mm_fault(vma, far, mm_flags, regs)`.
4. `__handle_mm_fault` → `handle_pte_fault(vmf)`: `vmf->pte` hiện diện; `vmf->flags & FAULT_FLAG_WRITE` và `!pte_write(entry)` → **`do_wp_page(vmf)`**.
5. `do_wp_page`:
    - `page = vm_normal_page(vma, addr, orig_pte)` → `0x40100`. `PageAnon(page)` → true.
    - Kiểm tra "exclusive": `page_count(0x40100) == 2` (cha + con) → **KHÔNG** → `wp_page_copy(vmf)`.
6. `wp_page_copy`:
    - `new_page = alloc_page_vma(GFP_HIGHUSER_MOVABLE, vma, addr)` → `PFN 0x40222`.
    - `cow_user_page(new_page, old_page, vmf)` → `memcpy` 4 KB `0x40100 → 0x40222` (nội dung `0xAB…` được copy; việc ghi `0xCD` xảy ra khi lệnh chạy lại).
    - `mmu_notifier_invalidate_range_start(range)` (báo secondary MMU: KVM/IOMMU).
    - khoá `pte_lock`, kiểm PTE chưa đổi.
    - `entry = mk_pte(new_page, vma->vm_page_prot)`; `entry = maybe_mkwrite(pte_mkdirty(entry), vma)` → **RW, dirty**.
    - **`ptep_clear_flush_notify(vma, addr, vmf->pte)`** → xoá PTE cha + `flush_tlb_page(vma, addr)` → **ARM64**: `tlbi vae1is, (addr >> 12) | (ASID=7 << 48)` (xoá đúng 1 trang, mọi CPU) + `mmu_notifier_invalidate_range`. ◀── **TLB maintenance #2**.
    - `page_add_new_anon_rmap(new_page, vma, addr, false)` → `0x40222._mapcount = 0` (1 mapping), thêm vào LRU.
    - `set_pte_at_notify(mm, addr, vmf->pte, entry)` → **PTE cha → `0x40222`, RW**.
    - `update_mmu_cache(vma, addr, pte)` → **ARM64**: nếu trang exec thì `__sync_icache_dcache`; thường no-op (không có cache TLB kiểu software).
    - `page_remove_rmap(old_page, false)` → `0x40100._mapcount--` (còn 1: chỉ con map).
    - `put_page(old_page)` → `0x40100._refcount = 1`.
    - `mmu_notifier_invalidate_range_only_end(range)`.
    - trả `VM_FAULT_WRITE`.
7. Trở về EL0, `eret`, **lệnh `strb` chạy lại** → PTE giờ RW trỏ `0x40222` → ghi `0xCD` thành công.

Trạng thái:

```
MM_parent PTE(0xaaaa12345000) → PFN 0x40222  RW dirty
MM_child  PTE(0xaaaa12345000) → PFN 0x40100  RO      (KHÔNG đổi)
Phys 0x40222: refcount=1  byte[0]=0xCD, byte[1..]=0xAB (copy)
Phys 0x40100: refcount=1  byte[0]=0xAB              (chỉ con map)
```

### (d) SAU khi **CHILD** ghi `heap[0] = 0xEF`

1. Con thực thi `strb` → PTE con `PTE_RDONLY=1` → permission fault ghi → `do_page_fault` → `handle_mm_fault` → `do_wp_page`.
2. `page = 0x40100`. `PageAnon` → true. Kiểm exclusive:
    - `page_count(0x40100) == 1` **và** `page_mapcount(0x40100) == 1` **và** `!PageKsm` → **exclusive** → **`wp_page_reuse(vmf)`**.
3. `wp_page_reuse`:
    - `entry = pte_mkyoung(orig_pte)`; `entry = maybe_mkwrite(pte_mkdirty(entry), vma)` → RW dirty.
    - `ptep_set_access_flags(vma, addr, vmf->pte, entry, 1)` → **sửa PTE con TẠI CHỖ** thành RW.
    - **ARM64**: `ptep_set_access_flags` nếu PTE đổi → `flush_tlb_page(vma, addr)` (`tlbi vae1is, addr|ASID_child`). ◀── **TLB maintenance #3** (chỉ khi cần).
    - `update_mmu_cache`.
    - **KHÔNG alloc, KHÔNG memcpy.**
4. `eret` → lệnh chạy lại → ghi `0xEF` vào `0x40100`.

Trạng thái cuối:

```
MM_parent PTE(0xaaaa12345000) → 0x40222  RW   byte[0]=0xCD
MM_child  PTE(0xaaaa12345000) → 0x40100  RW   byte[0]=0xEF
```

Hai trang **độc lập hoàn toàn**, mỗi trang `refcount=1`.

---

## 5. `do_wp_page()` — reuse vs copy

```c
static vm_fault_t do_wp_page(struct vm_fault *vmf)
{
    struct page *page = vm_normal_page(vma, vmf->address, vmf->orig_pte);

    if (!page) {                                   // PFNMAP / trang đặc biệt
        if ((vma->vm_flags & (VM_WRITE|VM_SHARED)) == (VM_WRITE|VM_SHARED))
            return wp_pfn_shared(vmf);
        return wp_page_copy(vmf);
    }

    if (PageAnon(page)) {                          // heap/stack/bss/COW
        if (!PageKsm(page) && page_count(page) == 1 && page_mapcount(page) == 1) {
            wp_page_reuse(vmf);                    // ◀── ĐỘC QUYỀN → tái dùng
            return VM_FAULT_WRITE;
        }
        return wp_page_copy(vmf);                  // ◀── còn bên khác map → COPY
    }

    if (vma->vm_flags & VM_SHARED)                 // MAP_SHARED writable file
        return wp_page_shared(vmf);                // page_mkwrite → dirty → wp_page_reuse

    return wp_page_copy(vmf);                      // MAP_PRIVATE file (vd .text đã mprotect) → COPY
}
```

Quy tắc: **`page_count == 1` → reuse** (không copy, chỉ nới quyền); **`> 1` → copy**. Ngoài fork, `page_count > 1` cũng xảy ra với KSM (trang gộp), swap-cache, page pinning.

---

## 6. Tổng kết TLB maintenance (ARM64)

|Thời điểm|Lệnh|Phạm vi|
|---|---|---|
|Cuối `dup_mmap`|`flush_tlb_mm(oldmm)` → `tlbi aside1is, ASID_parent`|mọi entry của cha, mọi CPU|
|`wp_page_copy` → `ptep_clear_flush_notify`|`flush_tlb_page` → `tlbi vae1is, VA\|ASID` + mmu_notifier|1 trang, mm bị fault, mọi CPU|
|`wp_page_reuse` → `ptep_set_access_flags` (nếu PTE đổi)|`flush_tlb_page`|1 trang|
|Con chạy lần đầu (`switch_mm`)|(không flush) — chỉ cấp ASID mới + đổi TTBR0|—|

`tlbi ...is` (Inner Shareable) tự lan sang mọi CPU trong domain — **không cần IPI** (khác x86). Kèm `dsb(ishst)` trước, `dsb(ish); isb()` sau.

---

## 7. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|Hàm copy 1 PTE|`copy_one_pte()` (một hàm)|tách `copy_present_pte()` + `copy_present_page()` (5.10) cho page-pinning (GUP)|
|`copy_page_range` chữ ký|`(dst_mm, src_mm, vma)`|`(dst_vma, src_vma)` (5.10)|
|mmu_notifier quanh `copy_page_range`|không|có (5.8) — cho copy trang bị pin|
|`wp_page_copy` re-check|`pte_same` dưới lock|thêm xử lý `page_count` chặt hơn (5.9 "COW fix" cho GUP) + `PageAnonExclusive` (6.1)|
|`VM_WIPEONFORK` (madvise)|có (4.14)|có|
|VMA tree|`mm_rb` + `vm_next`|maple tree (6.1)|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `copy_present_pte`, `copy_page_range(dst_vma, src_vma)`, `page_copy_prealloc`. Cơ chế write-protect cả hai + `do_wp_page` reuse/copy + TLB flush — **giống 5.4**.

---

✅ **MODULE 7 hoàn tất.** Sơ đồ: [docs/diagrams/fork-cow-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/fork-cow-arm64.md).

**Kế tiếp: MODULE 8 — `copy_thread()` trên ARM64** (chi tiết `struct pt_regs` / `struct cpu_context` / `struct thread_struct`; parent register state → `copy_thread` → child register state; `x0`, `x19–x30`, `SP`, `PC`, `PSTATE`, `TLS`, stack; vì sao child bắt đầu từ `ret_from_fork`; register diagram). Nói "tiếp" để làm Module 8.