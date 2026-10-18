# 07 — Các ví dụ thực tế (Phần V: §14–§18)

> Mọi địa chỉ ảo, PFN, nội dung page dưới đây là **giá trị giả định để minh họa**, không lấy từ board hay từ source.
> Mã hoá swap PTE và byte `swap_map` thì đúng theo source (sinh bởi `tools/`).

---

## §14. Ví dụ A: RAM còn đủ

**Tình huống.** Board vừa khởi động, free = 600 MiB. Process P gọi `malloc(64 MiB)` rồi ghi vào từng page.

**Diễn biến khái niệm:**
1. `malloc` → `mmap`/`brk` → kernel chỉ tạo/mở rộng VMA. **Chưa cấp frame, chưa đụng swap.**
2. Lần ghi đầu vào mỗi page → page fault → `handle_pte_fault()`: PTE **none** → `do_anonymous_page()` `[SOURCE mm/memory.c:4499-4507]`.
3. `do_anonymous_page()` xin 1 frame (zero-filled). Free vẫn trên watermark → allocator trả frame ngay từ free list, **không** đánh thức kswapd, **không** reclaim.
4. Page được map (`page_add_new_anon_rmap`) và vào LRU inactive anon qua pagevec (`lru_cache_add_inactive_or_unevictable()` `[SOURCE mm/memory.c:3686-3687; mm/swap.c:486-505]`).
5. 16 384 page sau, P có 64 MiB anon trong RAM.

**Swap metadata thay đổi gì?** Không gì cả: `swap_map`, `nr_swap_pages`, swap cache, slot cache, `swap_cgroup` đều nguyên vẹn. `get_swap_page()` không bao giờ được gọi.

**Vì sao không phải mọi page đều đi qua swap:** swap chỉ được dùng **khi reclaim chọn một anon page để thu hồi**. Có swap ≠ dùng swap. Hệ thống có thể chạy nhiều ngày với swap bật mà `/proc/swaps` "Used" = 0. Ngược lại, page file (thư viện, file đọc) **không bao giờ** đi qua swap dù RAM có thiếu hay không.

---

## §15. Ví dụ B: RAM thiếu, anonymous page reclaim được — swap out rồi swap in

**Giả định:** page A của process X; VA `0x0000007f12345000`; frame ban đầu PFN `0x2A000`; nội dung 4 KiB bắt đầu bằng `DE AD BE EF`; swap area type 0 trên block device; slot được cấp: offset 1. Slot cache của CPU đang rỗng.

| Bước | Sự kiện (function) | VA → PTE | Page frame | Nội dung | Swap entry | Slot 1 trên đĩa | `swap_map[1]` | Swap cache |
|---|---|---|---|---|---|---|---|---|
| 0 | Bình thường | present → PFN `0x2A000` | `0x2A000` dùng bởi A | trong RAM | — | rác | `00` | — |
| 1 | `get_swap_page()`: refill 64 slot, A nhận offset 1 | present | như trên | RAM | (0,1) | rác | `40` | — |
| 2 | `add_to_swap_cache()`, `set_page_dirty()` | present | +`PG_swapcache`, `private=0x1`, dirty | RAM | (0,1) | rác | `40` | `[0][0]`: idx 1 → A |
| 3 | `try_to_unmap()` → `swap_duplicate()` | **swap PTE `0x100`** | mapcount 0 | RAM | (0,1) | rác | `41` | → A |
| 4 | `pageout()` → `swap_writepage()` → bio | swap PTE | `PG_writeback` | RAM, đang chép | (0,1) | đang ghi | `41` | → A |
| 5 | `end_swap_bio_write()` | swap PTE | sạch, đuôi inactive | RAM **và** đĩa | (0,1) | `DE AD BE EF…` | `41` | → A |
| 6 | `__remove_mapping()` + `put_swap_page()`; frame free | swap PTE | **`0x2A000` trống** → cấp cho Y | **chỉ trên đĩa** | (0,1) chỉ còn trong PTE | `DE AD BE EF…` | `01` | shadow |
| 7 | X đọc VA → fault → `lookup_swap_cache()` miss | swap PTE | — | đĩa | (0,1) | dữ liệu | `01` | shadow |
| 8 | `__read_swap_cache_async()`: `alloc_page_vma()` → PFN `0x31F00`; `swapcache_prepare()`; `add_to_swap_cache()`; `swap_readpage()` | swap PTE | `0x31F00` locked, `PG_swapcache` | đang đọc | (0,1) | dữ liệu | `41` | → page mới |
| 9 | `end_swap_bio_read()`: `SetPageUptodate`, `unlock_page` | swap PTE | `0x31F00` uptodate | RAM **và** đĩa | (0,1) | dữ liệu | `41` | → page mới |
| 10 | `do_swap_page()`: `set_pte_at()`, rmap, `swap_free()` | **present → PFN `0x31F00`** | `0x31F00` mapcount 1 | RAM (và bản trên đĩa vẫn đúng) | (0,1) vẫn gắn với page qua `private` | dữ liệu | `40` | → page |

Mã PTE `0x100` = type 0, offset 1 trên arm64 `[SIM tools/swap_numbers.out.txt §3]`. Byte `swap_map` theo `[SIM tools/swap_map_sim.out.txt kịch bản 1 bước 1-7]`.

**Sau bước 10, ba tương lai có thể (đã kiểm chứng trong source):**
- (a) X **chỉ đọc**, rồi A bị reclaim lại: A đã `PageSwapCache` → **không cấp slot mới**; `try_to_unmap` → `41`; A sạch → không `pageout`; `__remove_mapping` → `01`; frame free. **Không có I/O ghi lần hai** `[SIM kịch bản 1 bước 8-9]`.
- (b) X **ghi** vào A: vì swap cache giữ thêm 1 reference, `do_wp_page()` thấy `page_count != 1` → `wp_page_copy()` tạo page mới cho X `[SOURCE mm/memory.c:3198-3229]`; A cũ không còn ai map, vẫn ở swap cache (`40`) → bị dọn sau (reclaim hoặc khi swap đầy).
- (c) Fault ở bước 7 là **fault ghi**: `reuse_swap_page()` thành công (count 1) → A rời swap cache, PTE ghi được + dirty; `swap_free` đưa byte về 0 → `free_swap_slot()` → slot qua `slots_ret` rồi về pool `[SOURCE mm/memory.c:3520-3525, :3545]`.
- Và nếu swap đã > 50% đầy: `try_to_free_swap()` ngay ở bước 10 → slot được trả sớm `[SOURCE mm/memory.c:3546-3548]`.

---

## §16. Ví dụ C: Swap cache hit

**Tình huống 1 — chia sẻ sau fork.** Page B đã swap out (slot 1, `01`). X `fork()` ra X'. `copy_nonpresent_pte()` → `swap_duplicate()` → `02` `[SOURCE mm/memory.c:715-727]`. Cả X và X' đều có swap PTE `0x100`.

| Bước | Sự kiện | `swap_map[1]` | Swap cache | I/O |
|---|---|---|---|---|
| 1 | X' đọc B → miss → `swapcache_prepare` → đọc | `42` | → B | **1 lần đọc** |
| 2 | X' map xong, `swap_free` | `41` | → B | — |
| 3 | X đọc B → `lookup_swap_cache()` **hit** → `lock_page`, map, `swap_free` | `40` | → B | **0** |

(Mô phỏng tương ứng: `[SIM kịch bản 2]`.)

**Vì sao không cần đọc lại:** page trong swap cache **chính là** bản đúng của slot (nó được tạo ra từ việc đọc slot, hoặc là page gốc trước khi swap out). Nếu X đọc lại đĩa ra page B'' riêng, X và X' sẽ có hai bản RAM của cùng dữ liệu — tốn RAM và phá vỡ ngữ nghĩa COW (sau fork, page chỉ nên tách khi có người *ghi*). Fault của X là **minor fault** (không tăng `PGMAJFAULT`) `[SOURCE mm/memory.c:3453-3456 chỉ ở nhánh miss]`.

**Tình huống 2 — readahead.** X fault vào page tại slot 100; `swap_cluster_readahead()` (hoặc VMA readahead) đọc luôn slot 96–103 vào swap cache, đánh `PG_readahead` cho các page không phải page đang fault `[SOURCE mm/swap_state.c:684-699]`. Khi X chạm slot 101 → hit; `TestClearPageReadahead` → đếm `SWAP_RA_HIT`, tăng `swapin_readahead_hits` → lần readahead sau cửa sổ lớn hơn `[SOURCE mm/swap_state.c:394-412]`. Nếu page 101 vẫn đang được đọc, `lock_page_or_retry()` chờ — hit không có nghĩa là không chờ.

**Tình huống 3 — fault trong lúc đang swap out.** Ở bước 3–5 của Ví dụ B, X chạm A → `lookup_swap_cache()` hit → map lại PTE present (count `41` → `40`). Reclaim lượt sau thấy A mapped/referenced → không free.

---

## §17. Ví dụ D: Không cấp được swap slot

**Các nguyên nhân có cơ sở trong source:**

| Nguyên nhân | Nơi phát hiện | Kết quả trả về |
|---|---|---|
| Hết slot: `nr_swap_pages <= 0` | `get_swap_pages()` `[SOURCE mm/swapfile.c:1069-1073]` | `n_ret = 0` |
| Mọi area không còn trong avail list / không `SWP_WRITEOK` (đầy, đang swapoff) | `[:1081-1101]` | 0 |
| Quét không thấy byte 0 (phân mảnh, đua) | `scan_swap_map_slots()` → `no_page` `[:865-868, :988-990]` | 0 |
| **VENDOR**: `warp_swapout_disable = 1` | `[SOURCE mm/swapfile.c:794-797]` | 0 |
| memcg v2 vượt `memory.swap.max` | `mem_cgroup_try_charge_swap()` → `-ENOMEM` → `get_swap_page` trả slot, entry 0 `[SOURCE mm/memcontrol.c:7320-7326; mm/swap_slots.c:349-352]` | entry 0 |
| Thêm vào xarray thất bại | `add_to_swap_cache()` lỗi → `put_swap_page()` `[SOURCE mm/swap_state.c:239-263]` | `add_to_swap` = 0 |

**Mô phỏng chuỗi xử lý** (page anon C đang ở đuôi inactive, swap đã đầy):
1. Thường thì `get_scan_count()` đã thấy `mem_cgroup_get_nr_swap_pages() <= 0` và **không quét anon** (`SCAN_FILE`) `[SOURCE mm/vmscan.c:2267-2269]` → C thậm chí không được xem xét.
2. Nếu C vẫn tới `shrink_page_list()` (ví dụ nr_swap_pages vừa về 0 giữa chừng, hoặc slot bị giữ trong cache CPU khác): `add_to_swap(C)` → `get_swap_page()` → entry 0 → `add_to_swap` trả 0 `[SOURCE mm/swap_state.c:224-226]`.
3. `goto activate_locked_split` → `activate_locked`: C được `SetPageActive`, trả về LRU `[SOURCE mm/vmscan.c:1279-1281, :1485-1496]`. **Không có dữ liệu bị mất, chỉ là không thu hồi được C.**
4. Reclaim chuyển sang page khác (file). Nếu không đủ, allocator thử lại/compaction và cuối cùng có thể gọi **OOM killer** `[KIẾN THỨC NỀN; docs/vmscan/05]`.

**Reclaim có chắc thành công không?** **Không.** Không có swap trống ⇒ anon không thể thu hồi ⇒ reclaim chỉ còn page file + slab. Ngoài ra, ngay cả khi swap "đầy", một số slot chỉ bị giữ bởi swap cache (`0x40`) — kernel giải phóng chúng ở vài chỗ khi `vm_swap_full()`/`mem_cgroup_swap_full()`: trong `activate_locked` `[SOURCE mm/vmscan.c:1487-1489]`, sau swap-in `[SOURCE mm/memory.c:3546-3548]`, trong `free_swap_and_cache()` `[SOURCE mm/swapfile.c:1818-1821]`, và trong `scan_swap_map_slots()` — nhưng nhánh cuối **chỉ chạy nếu `get_swap_pages()` còn `nr_swap_pages > 0`** (nó thoát sớm khi về 0 `[:1069-1073]`). Vì vậy không có bảo đảm.

**Trường hợp WARP (board-specific):** khi `warp_swapout_disable = 1`, `scan_swap_map_slots()` trả 0 cho **mọi** area → `get_swap_pages()` lần lượt thử từng area (log `pr_debug("scan_swap_map of si %d failed to find offset")` — không in ra vì không bật dynamic debug) rồi trả 0 và **hoàn lại** `nr_swap_pages` `[SOURCE mm/swapfile.c:1106-1137]`. Kết quả quan sát được: swap còn trống nhưng không có page anon nào được swap out trong pha đó `[SUY LUẬN, KIỂM TRA TRÊN BOARD: /proc/vmstat pswpout không tăng trong lúc WARP shrink]`.

---

## §18. Ví dụ E: Hệ thống nhiều CPU

**Tình huống:** 4 CPU, cả bốn đang reclaim mạnh và cấp/trả tổng cộng 1 000 000 slot.

### 18.1. So sánh ba cách

| | Truy cập global pool trực tiếp (giả định không có slot cache) | Per-CPU cache (`swap_slots.c`) | Drain khi cần trả về global |
|---|---|---|---|
| Khoá toàn cục khi **cấp** | 1 000 000 × (`swap_avail_lock` + `si->lock`) | 15 625 lần refill × (`swap_avail_lock` + `si->lock`) | — |
| Khoá khi **trả** | 1 000 000 × `si->lock` | 15 625 lô × `si->lock` (đã sort theo area) | mỗi lần drain: 1 lô/CPU/mảng |
| Khoá per-CPU | không có | `alloc_lock` (mutex) / `free_lock` (spinlock) — gần như không tranh chấp | lấy cả hai khoá của từng CPU |
| Slot "bị giữ" không dùng được | 0 | tối đa 4 × 128 = 512 slot (2 MiB) | trả hết về pool |
| Phân mảnh | slot trả lẻ tẻ ngay | trả theo lô, slot liền kề trống cùng lúc | |
| Khi swap gần cạn | công bằng | CPU này thiếu khi CPU kia ôm thừa `[SIM kịch bản 3]` | `check_cache_active()` tắt cache dưới 512, bật lại trên 1280 `[SIM §2, kịch bản 4]` |

Số liệu `[SIM tools/swap_numbers.out.txt §2, §7]`: giảm số lần lấy khoá toàn cục **64 lần**.

### 18.2. Diễn biến (từ mô phỏng, cache thu nhỏ còn 4 slot, 2 CPU, 48 slot)

```
lần 1  CPU0: trước nr_swap_pages=47 → refill 4 slot, dùng slot 1 → nr_swap_pages=43, cpu0=[2,3,4]
lần 2  CPU1: refill 4 slot, dùng slot 5 → nr_swap_pages=39, cpu1=[6,7,8]
...
lần 27 CPU0: trước nr_swap_pages=15 (< 2 CPU × 2 × 4 = 16) cpu0=[26,27,28] cpu1=[30,31,32]
       → check_cache_active(): deactivate + drain 6 slot về pool (15+6=21)
       → cấp trực tiếp slot 33 → nr_swap_pages=20, cả hai túi rỗng
lần 36 CPU1: cấp trực tiếp, nr_swap_pages=11
```
`[SIM tools/swap_map_sim.out.txt kịch bản 4]`

### 18.3. Lợi ích và chi phí

**Lợi ích hiệu năng:** ít khoá toàn cục (64×), mảng per-CPU nóng trong cache CPU, các CPU không tranh nhau cache line của `swap_map`/`si->lock`; kết hợp `percpu_cluster` (SSD) mỗi CPU ghi vào vùng riêng, tuần tự.

**Chi phí quản lý đồng bộ:**
- Thêm 4 khoá (2 per-CPU + 2 mutex toàn cục) và trạng thái `swap_slot_cache_enabled/active/initialized`.
- Phải xử lý CPU hotplug (`cpuhp_setup_state` + `free_slot_cache`), swapoff (`disable_swap_slots_cache_lock` giữ mutex suốt `try_to_unuse`), swap thấp (hysteresis + drain).
- Slot "đã free" chưa trống ngay (nằm trong `slots_ret`, byte vẫn `0x40`) → các phép đếm như `inuse_pages`, `/proc/swaps Used` hơi trễ.
- Kiểm tra kép: `free_swap_slot()` phải kiểm tra lại `use_swap_slot_cache && cache->slots_ret` **sau** khi lấy `free_lock` vì cache có thể bị tắt giữa chừng `[SOURCE mm/swap_slots.c:279-285]`; `get_swap_page()` kiểm tra lại `cache->slots` sau khi lấy `alloc_lock` `[:330-332]`; `alloc_swap_slot_cache()` dùng `mb()` để bảo đảm khoá đã init trước khi con trỏ mảng được nhìn thấy `[:156-164]`.
