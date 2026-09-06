# MODULE 14 — Context Switch & MMU trên Linux 5.4 / ARM64

Sơ đồ đã lưu: [docs/diagrams/context-switch-mmu-arm64.md](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/docs/diagrams/context-switch-mmu-arm64.md) (5 sơ đồ: quyết định `switch_mm`, fast/slow path của ASID allocator, `cpu_do_switch_mm` ghi TTBR, vì sao ASID, so sánh chi phí thread vs process).

**File nguồn**: `arch/arm64/include/asm/mmu_context.h` (`switch_mm`, `__switch_mm`, `cpu_switch_mm`), `arch/arm64/mm/context.c` (`check_and_switch_context`, `new_context`, `flush_context`, `cpu_do_switch_mm`), `arch/arm64/include/asm/pgtable-hwdef.h` (`TCR_A1`, `TCR_ASID16`), `arch/arm64/include/asm/mmu.h` (`TTBR_ASID_MASK`), `kernel/sched/core.c` (`context_switch`).

---

## 0. Bối cảnh: Task A và Task B

```
Task A:  A->mm = MM_A   MM_A->pgd = PGD_A   MM_A->context.id = [gen_X | ASID_A]
Task B:  B->mm = MM_B   MM_B->pgd = PGD_B   MM_B->context.id = [gen_X | ASID_B]
```

`mm->context.id` là một `atomic64_t` đóng gói **[generation (48 bit cao) | ASID (16 bit thấp)]`. ASID 16-bit vì` TCR_EL1.ASID16 = 1`(CPU báo`ID_AA64MMFR0_EL1.ASIDBits == 2`); nếu không thì 8-bit.

Khi switch A → B, MMU phải:

1. Trỏ `TTBR0_EL1` sang `PGD_B` (bảng trang user của B).
2. Đặt ASID hiện hành = `ASID_B`.
3. **Không** flush TLB (đó chính là điểm của ASID).

`TTBR1_EL1` (bảng trang kernel = `swapper_pg_dir`) **không đổi bao giờ** — chỉ trường ASID trong nó thay.

---

## 1. `context_switch()` → `switch_mm()` — quyết định có đổi mm không

`kernel/sched/core.c`:

```c
if (!next->mm) {                                   // next = KERNEL THREAD
    enter_lazy_tlb(prev->active_mm, next);
    next->active_mm = prev->active_mm;             // mượn mm của prev
    if (prev->mm) mmgrab(prev->active_mm);
    // → KHÔNG gọi switch_mm. TTBR0 giữ nguyên của prev. Chi phí MMU = 0.
} else {                                           // next = USER TASK
    switch_mm_irqs_off(prev->active_mm, next->mm, next);
}
```

`switch_mm` (`arch/arm64/include/asm/mmu_context.h`, `switch_mm_irqs_off` là alias):

```c
static inline void
switch_mm(struct mm_struct *prev, struct mm_struct *next, struct task_struct *tsk)
{
    if (prev != next)                 // ◀── cửa quyết định
        __switch_mm(next);

    update_saved_ttbr0(tsk, next);    // chỉ bookkeeping cho SW-PAN (thường nop)
}
```

**Điểm mấu chốt: `if (prev != next)`** — tham số là `mm_struct *`, không phải task. Nếu A và B **cùng process** (`A->mm == B->mm == MM_A`), thì `prev == next` → `__switch_mm` **không được gọi**. Không đụng TTBR0, không ASID, không TLB. Đây là gốc rễ vì sao thread-switch rẻ.

Nếu **khác process** (`MM_A != MM_B`) → `__switch_mm(MM_B)`:

```c
static inline void __switch_mm(struct mm_struct *next)
{
    if (next == &init_mm) {           // mm của kernel (không có ánh xạ user)
        cpu_set_reserved_ttbr0();     // TTBR0 = trang rỗng, ASID 0
        return;
    }
    check_and_switch_context(next);   // ◀── xử lý ASID + ghi TTBR0
}
```

---

## 2. `check_and_switch_context()` — cấp/kiểm ASID

`arch/arm64/mm/context.c`:

```c
void check_and_switch_context(struct mm_struct *mm)
{
    asid = atomic64_read(&mm->context.id);

    /* ── FAST PATH ── */
    old_active_asid = atomic64_read(this_cpu_ptr(&active_asids));
    if (old_active_asid &&
        asid_gen_match(asid) &&                                   // generation của mm khớp asid_generation toàn cục?
        atomic64_cmpxchg_relaxed(this_cpu_ptr(&active_asids),
                                 old_active_asid, asid))
        goto switch_mm_fastpath;                                  // ◀── KHÔNG lock, KHÔNG flush

    /* ── SLOW PATH ── */
    raw_spin_lock_irqsave(&cpu_asid_lock, flags);
    asid = atomic64_read(&mm->context.id);
    if (!asid_gen_match(asid)) {                                  // ASID của mm quá cũ (rollover đã xảy ra)
        asid = new_context(mm);                                   // cấp ASID mới cho generation hiện tại
        atomic64_set(&mm->context.id, asid);
    }
    cpu = smp_processor_id();
    if (cpumask_test_and_clear_cpu(cpu, &tlb_flush_pending))      // CPU này nợ 1 lần flush từ rollover?
        local_flush_tlb_all();                                    // ◀── tlbi vmalle1 — ĐẮT, nhưng hiếm
    atomic64_set(this_cpu_ptr(&active_asids), asid);
    raw_spin_unlock_irqrestore(&cpu_asid_lock, flags);

switch_mm_fastpath:
    arm64_apply_bp_hardening();                                   // Spectre-BHB / branch predictor
    if (!system_uses_ttbr0_pan())
        cpu_switch_mm(mm->pgd, mm);                               // ◀── ghi TTBR (Mục 3)
}
```

- **`active_asids[cpu]`** (per-CPU): ASID mà CPU này đang dùng.
- **`asid_generation`** (toàn cục): tăng `+= (1<<16)` mỗi lần cạn ASID.
- **`asid_gen_match(asid)`**: bit generation trong `mm->context.id` có bằng `asid_generation` không.
- **Fast path** (đa số các switch): `cmpxchg` cập nhật `active_asids[cpu]` — không lock, không flush. Chỉ 1 phép nguyên tử.
- **Slow path**: chỉ khi ASID của `mm` thuộc generation cũ. `new_context(mm)` cố tái dùng ASID cũ (nếu bit trong `asid_map` còn trống), nếu không thì `find_next_zero_bit`, nếu hết thì **rollover**.

`new_context()` khi hết ASID (`find_next_zero_bit` trả về `NUM_USER_ASIDS`):

```c
generation = atomic64_add_return_relaxed(ASID_FIRST_VERSION, &asid_generation);  // gen++
flush_context();                                                                  // Mục 2b
asid = find_next_zero_bit(asid_map, NUM_USER_ASIDS, 1);                            // luôn thành công
```

### 2b. `flush_context()` — rollover

```c
static void flush_context(void)
{
    set_reserved_asid_bits();                              // xoá asid_map (hoặc copy pinned/KPTI bits)
    for_each_possible_cpu(i) {
        asid = atomic64_xchg_relaxed(&per_cpu(active_asids, i), 0);
        if (asid == 0) asid = per_cpu(reserved_asids, i);  // CPU chưa chạy task nào từ lần rollover trước
        __set_bit(ctxid2asid(asid), asid_map);             // giữ ASID đang active — không cấp lại cho ai
        per_cpu(reserved_asids, i) = asid;
    }
    cpumask_setall(&tlb_flush_pending);                    // ◀── MỌI CPU sẽ local_flush_tlb_all
}                                                          //     ở check_and_switch_context kế tiếp
```

Rollover xảy ra **~1 lần mỗi 65536 `mm_struct` được tạo** (65536 process/thread-group mới). Phí `local_flush_tlb_all` trên mỗi CPU trải mỏng qua 65536 lần → gần như 0 trung bình.

---

## 3. `cpu_do_switch_mm()` — ghi `TTBR0_EL1` + ASID vào `TTBR1_EL1`

5.4: assembly trong `proc.S`. 5.5+: C trong `context.c`:

```c
void cpu_do_switch_mm(phys_addr_t pgd_phys, struct mm_struct *mm)
{
    unsigned long ttbr1 = read_sysreg(ttbr1_el1);          // = swapper_pgd | (ASID_cũ << 48)
    unsigned long asid  = ASID(mm);                        // ASID 16-bit của next
    unsigned long ttbr0 = phys_to_ttbr(pgd_phys);          // = phys(next->pgd)

    if (IS_ENABLED(CONFIG_ARM64_SW_TTBR0_PAN))
        ttbr0 |= FIELD_PREP(TTBR_ASID_MASK, asid);         // SW-PAN: cần bản sao ASID ở TTBR0

    ttbr1 &= ~TTBR_ASID_MASK;                              // xoá ASID cũ trong TTBR1
    ttbr1 |= FIELD_PREP(TTBR_ASID_MASK, asid);             // gắn ASID MỚI

    write_sysreg(ttbr1, ttbr1_el1);  isb();                // ① ĐỔI ASID trước (kernel pgd KHÔNG đổi)
    write_sysreg(ttbr0, ttbr0_el1);  isb();                // ② rồi mới ĐỔI bảng trang user
    post_ttbr_update_workaround();                         // errata Cortex-A76 speculative-AT
}
```

Chi phí: **2 lệnh `msr` + 2 `isb`** (mỗi `isb` xả pipeline, ~vài chục chu kỳ) + workaround. **TLB không bị flush.**

### Vì sao ASID nằm ở `TTBR1_EL1`, không phải `TTBR0` (`TCR_EL1.A1 = 1`)

1. **Không có cửa sổ "pgd mới + ASID cũ".** Giữa bước ① và ②: `TTBR0` vẫn trỏ `PGD_A` (cũ) nhưng ASID đã là `ASID_B`. Mọi mục TLB sinh từ `PGD_A` đều gắn `ASID_A` → MMU **không dùng** (lệch ASID). Nên trong khoảng ①→② không thể xảy ra ánh xạ bẩn. Nếu ASID ở TTBR0, ghi TTBR0 một lần vừa đổi pgd vừa đổi ASID — cũng nguyên tử, nhưng KPTI phá thế này:
2. **KPTI trampoline** (`CONFIG_UNMAP_KERNEL_AT_EL0`) lật `TTBR1` giữa "kernel pgd đầy đủ" và "trampoline pgd tối thiểu" **mỗi lần vào/ra EL1**. Nếu ASID ở TTBR1, việc lật này không đụng ASID. Nếu ASID ở TTBR0... trampoline phải xử lý phức tạp hơn nhiều.

`TTBR1_EL1` (kernel) về mặt _bảng trang_ không đổi khi switch process — chỉ trường ASID `[63:48]` thay.

---

## 4. Vì sao Linux dùng ASID

### Không có ASID: bắt buộc flush toàn bộ TLB mỗi switch

Mục TLB ánh xạ VA → PA. Không có thẻ ASID, một mục cho VA `X` là "toàn cục". Khi switch A → B:

- Nếu **không flush**: B chạm VA `X` → trúng mục TLB **cũ của A** tại VA `X` → B đọc/ghi **trang vật lý của A**. Đây vừa là hỏng dữ liệu, vừa là lỗ hổng bảo mật (B đọc bộ nhớ A).
- Nên bắt buộc `tlbi vmalle1` (xoá mọi mục EL1) mỗi lần `switch_mm`.
- Hậu quả: sau flush, **mọi truy cập của B đều TLB miss** → mỗi miss kích hoạt **page-table walk 4 tầng** (PGD→PUD→PMD→PTE), tới 4 lần truy cập bộ nhớ, thường L1/L2 cache miss → 4 × ~10–100 ns. Kéo dài cho tới khi TLB "ấm" lại.
- Tệ hơn: khi **quay lại A**, mục TLB của A cũng đã bị xoá → A cũng chịu bão TLB miss.

Trên hệ thống switch thường xuyên (hàng nghìn lần/giây), chi phí này áp đảo.

### Có ASID: 0 flush

- `TCR_EL1.A1 = 1` → MMU đọc ASID hiện hành từ `TTBR1_EL1[63:48]`.
- Mọi mục TLB cho ánh xạ **user** có bit `nG` (non-Global) = 1 → được **gắn thẻ ASID**.
- MMU **chỉ dùng** một mục nếu ASID của nó khớp ASID hiện hành.
- Switch A → B: chỉ đổi ASID hiện hành (ghi TTBR1). Mục của A (`ASID_A`) **vẫn nằm trong TLB** — MMU bỏ qua vì lệch ASID, **vô hại**.
- **Không lệnh `tlbi` nào chạy.**
- Quay lại A: mục `ASID_A` **vẫn còn** (nếu chưa bị evict do B cạnh tranh) → tái dùng ngay, không refill.

Mục kernel (`nG = 0`, Global) không gắn ASID → dùng chung mọi process → syscall không phải refill TLB kernel.

---

## 5. So sánh chi phí

### Thread cùng process: A1 → A2 (đều `MM_A`, `ASID_A`)

|Bước|Có làm?|
|---|---|
|`switch_mm`: `if (prev != next)` = `if (MM_A != MM_A)`|**SAI** → `__switch_mm` **không gọi**|
|`check_and_switch_context`|✗|
|`cpu_do_switch_mm` / `msr ttbr0_el1` / `msr ttbr1_el1`|✗|
|Cấp/kiểm ASID|✗|
|TLB flush|✗|
|`cpu_switch_to`: lưu/nạp 13 giá trị|✓|
|`__switch_to`: fpsimd (lười), tls swap, ~7 `msr` sysreg, `dsb(ish)`|✓|
|**TLB**|**Dùng chung, còn nóng nguyên.** A2 trúng ngay mục A1 vừa dùng — cùng VA, cùng ASID, cùng bảng trang.|

**Chi phí ≈ vài trăm ns** — chỉ thanh ghi + helper `__switch_to`. Không có phần MMU.

### Process khác mm: A → B (`MM_A` → `MM_B`, `ASID_A` → `ASID_B`)

|Bước|Chi phí|
|---|---|
|`switch_mm`: `MM_A != MM_B` → `__switch_mm(MM_B)` → `check_and_switch_context(MM_B)`|—|
|Fast path: `cmpxchg active_asids[cpu]`|1 phép nguyên tử, không lock|
|`cpu_do_switch_mm`: `msr ttbr1_el1`; `isb`; `msr ttbr0_el1`; `isb`; workaround|~50–100 chu kỳ. **Không flush TLB.**|
|`cpu_switch_to` 13 giá trị + `__switch_to` helpers|giống thread-switch|
|**Bão TLB refill** (chi phí chủ đạo)|Mục TLB của B có thể đã bị A đẩy ra khi A chạy → B TLB miss hàng loạt → mỗi cái page-table walk 4 tầng (4 × truy cập bộ nhớ, cache miss) → cho tới khi tập làm việc B nằm lại TLB.|
|Cache L1i/L1d/branch-predictor nguội cho code path của B|Không phải MMU nhưng cùng góp vào "process-switch đắt"|
|Slow path (rollover, ~1/65536 lần tạo mm)|`+ cpu_asid_lock` `+ local_flush_tlb_all()` → sau đó **mọi** truy cập của B refill → tệ hơn nhiều, nhưng cực hiếm|

**Chi phí ≈ vài µs (fast path)** — chủ yếu do **bão TLB refill**, không phải hai lệnh `msr`.

### Kết luận

**Process-switch đắt hơn.** Nhưng phần đắt **không phải** hai lệnh `msr ttbr` (chỉ ~50–100 chu kỳ). Phần đắt là **mất tập làm việc TLB**: sau khi chuyển sang B, B chịu một loạt TLB miss, mỗi miss là một walk 4 tầng, kéo dài cho tới khi TLB của B ấm lại.

Thread-switch **không** chịu phần này vì TLB dùng chung — A2 kế thừa nguyên tập làm việc TLB nóng của A1.

**Vai trò của ASID**: nó không làm process-switch "miễn phí". Nó loại bỏ **cú flush toàn bộ bắt buộc** — mà cú flush đó còn tệ hơn nhiều: flush xoá _tất cả_ mục của B (kể cả mục chưa bị evict), buộc B refill từ đầu; ASID giữ lại những mục của B chưa bị A đẩy ra, và giữ nguyên mục của A cho lần quay lại.

---

## 6. Bảng tổng hợp

||Thread cùng process|Process khác `mm` (fast path)|Process khác `mm` (rollover)|
|---|---|---|---|
|`__switch_mm` gọi?|Không|Có|Có|
|Ghi `TTBR0_EL1`|Không|Có (`msr` + `isb`)|Có|
|Đổi ASID|Không|Có (ghi `TTBR1`)|Có (cấp ASID mới)|
|`cpu_asid_lock`|Không|Không|**Có**|
|`tlbi` (flush TLB)|Không|**Không**|**`local_flush_tlb_all`** trên mỗi CPU|
|Bão TLB refill sau switch|Không (TLB nóng dùng chung)|Có (mục B bị A evict)|**Nặng** (mọi mục B mất)|
|Chi phí điển hình|~trăm ns|~vài µs|~chục µs (hiếm, ~1/65536)|

---

## 7. Khác biệt 5.4 vs bản mới

|Điểm|5.4|Bản sau|
|---|---|---|
|`cpu_do_switch_mm`|assembly trong `proc.S`|C trong `context.c` — 5.5|
|ASID pinning (`mm->context.pinned`)|chưa có|5.12 (cho SVA / iommu)|
|`new_context`|như mô tả (không có nhánh `pinned`)|+ nhánh `refcount_read(&mm->context.pinned)`|
|KPTI ASID (`set_kpti_asid_bits`)|có (dùng nửa ASID space)|có|
|`TCR_EL1.A1` (ASID ở TTBR1)|có (từ 4.20)|có|
|`check_and_switch_context` fast path|như mô tả|y hệt|

> Tree local `Kernel/K-S800/Src` = **5.10.241**: `cpu_do_switch_mm` là C trong `context.c`, `new_context` có nhánh `pinned`. Cơ chế fast/slow path, `asid_generation`, `flush_context` + `tlb_flush_pending`, ASID ở TTBR1 — **giống 5.4**.