# 08 — Kiểm tra kiến thức (§21) và Tổng hợp (§22)

---

# §21. Câu hỏi tự kiểm tra (25 câu, dễ → khó)

> Hãy tự trả lời trước. **Đáp án giải thích nằm ở cuối file** (mục "Đáp án §21").

### Nhóm 1 — Khái niệm
1. Page và page frame khác nhau thế nào? Khi page A đã swap out xong, `struct page` nào mô tả A?
2. Swap có làm tăng RAM vật lý không? Vì sao page của `libc.so` không bao giờ đi qua swap?
3. Một swap entry gồm những gì? `swp_type()` và `swp_offset()` dùng để chọn những cấu trúc nào? Vì sao `entry.val == 0` an toàn để biểu diễn "không có entry"?
4. Swap cache nằm trong RAM hay trên đĩa? Nó lưu gì cho mỗi phần tử?
5. `swap_cgroup` lưu gì? Nếu tắt `CONFIG_MEMCG_SWAP`, swap còn hoạt động không?

### Nhóm 2 — Luồng hoạt động
6. Trong `shrink_page_list()`, thứ tự của `add_to_swap`, `try_to_unmap`, `pageout`, `__remove_mapping` là gì? Vì sao gỡ PTE *trước* khi ghi mà vẫn an toàn?
7. Vì sao frame của page không được free trong cùng lượt quét đã gọi `pageout()`? Ai đưa nó quay lại để được free?
8. Trong `do_swap_page()`, task chờ I/O đọc ở đâu và bằng cơ chế gì?
9. Khi nào `do_swap_page()` bỏ qua swap cache? Trên board S800 đường này có khả năng được dùng không?
10. Vì sao slot thường vẫn bị giữ sau khi swap in? Liệt kê các trường hợp slot được trả sớm.

### Nhóm 3 — Phân biệt cấu trúc dữ liệu
11. Giải mã các byte `swap_map`: `0x00`, `0x01`, `0x03`, `0x40`, `0x41`, `0x3f`, `0xbf`.
12. Slot đang nằm trong `slots[]` của một CPU được tính thế nào trong `nr_swap_pages` và `si->inuse_pages`? Còn slot trong `slots_ret[]`?
13. `percpu_cluster` (swapfile.c) và `swap_slots_cache` (swap_slots.c) khác nhau thế nào?

### Nhóm 4 — Truy vết function
14. Truy vết từ `add_to_swap()` đến dòng code ghi byte `swap_map` từ 0 thành `0x40`.
15. Ai gọi `swap_duplicate()`? Ai gọi `swap_free()`? Ai gọi `free_swap_and_cache()`?
16. Truy vết từ `swap_free(entry)` đến lúc byte `swap_map` thật sự về `0x00`.

### Nhóm 5 — Tình huống lỗi
17. Ghi swap bị lỗi I/O: page, PTE và dữ liệu ở trạng thái nào? Log gì xuất hiện?
18. Đọc swap bị lỗi I/O: process nhận được gì?
19. `add_to_swap()` thất bại: page đi đâu? Reclaim có thành công không?

### Nhóm 6 — Concurrency
20. Vì sao `cache->alloc_lock` là mutex còn `cache->free_lock` là spinlock?
21. Hai task cùng fault vào cùng một swap entry chưa có trong swap cache: ai đọc đĩa, task kia làm gì?
22. Vì sao `swapoff` phải drain slot cache? Vì sao cần `synchronize_rcu()` sau khi xoá `SWP_VALID`?

### Nhóm 7 — Phân tích source
23. Trong `__swap_entry_free_locked()`, vì sao khi kết quả bằng 0 lại ghi `SWAP_HAS_CACHE` vào `swap_map` thay vì 0?
24. Hunk vendor `CONFIG_PM_WARP` trong `scan_swap_map_slots()` có tác dụng gì? Vì sao đặt nó *trước* `si->flags += SWP_SCANNING` là an toàn với `swapoff`?
25. Trên kernel 5.10 này, page vừa swap in bằng một lần **đọc**, sau đó process **ghi** vào nó: kernel có copy page không? Vì sao?

---

## §21b. Tình huống dự đoán (tự dự đoán trạng thái)

> Theo yêu cầu học tập đã thống nhất từ trước, **các tình huống này KHÔNG kèm đáp án** — hãy viết dự đoán của bạn (trạng thái page, `swap_map`, swap cache, PTE sau mỗi sự kiện), gửi lại, rồi mình sẽ chấm và giải thích theo source. Có thể kiểm tra một phần bằng cách sửa `tools/swap_map_sim.py`.

**P1.** Process P có anon page A (chưa từng swap). P `fork()` ra C khi A còn trong RAM (A được chia sẻ COW, 2 PTE). Sau đó reclaim chọn A. Dự đoán `swap_map[e]` và mapcount của A: (a) sau `add_to_swap`, (b) sau `try_to_unmap`, (c) sau `__remove_mapping`.

**P2.** A đã swap out (`swap_map = 0x01`). Swap đang dùng 60% dung lượng. P **đọc** A. Dự đoán: PTE, swap cache, `swap_map` ngay sau khi `do_swap_page()` trả về; slot đi đâu tiếp theo và khi nào thành `0x00`.

**P3.** A đang ở trạng thái `0x41`, đang writeback (giữa bước 4 và 5 của Ví dụ B, `07` §15). P gọi `munmap()` vùng chứa A. Dự đoán trạng thái sau `zap_pte_range()`; sau khi I/O ghi xong; và sau lượt reclaim kế tiếp.

**P4.** Slot cache đang bật. CPU0 lần lượt gọi `free_swap_slot()` cho 65 entry (của cùng một area). Dự đoán `n_ret`, byte `swap_map` của 65 entry và giá trị `nr_swap_pages` thay đổi bao nhiêu sau lời gọi thứ 64 và thứ 65.

**P5.** Page A ở trạng thái "chỉ còn cache" (`0x40`, A đang được P map bằng PTE present). Quản trị viên chạy `swapoff`. Dự đoán `try_to_unuse()` xử lý A ở vòng nào (mmlist hay `find_next_to_unuse`), và trạng thái A, PTE, `swap_map` sau đó.

**P6.** Hai swap area được bật bằng `swapon -p 5 /dev/a` và `swapon -p 5 /dev/b` (cùng priority). Trên một CPU, slot cache rỗng, có 3 lần refill liên tiếp. Dự đoán mỗi lần refill lấy slot từ area nào và vì sao (gợi ý: `plist_requeue`).

**P7.** Cùng một page anon thuộc memcg M bị swap out rồi swap in, trên (a) cgroup v1 và (b) cgroup v2. Dự đoán ở mỗi bước (`get_swap_page`, `__remove_mapping`, `mem_cgroup_charge` lúc swap in, `swap_entry_free`), `swap_cgroup` ID của slot là gì.

---

# §22. Tổng hợp kiến thức

## 22.1. Swap là gì và giải quyết vấn đề nào?
Swap là vùng lưu trữ cất nội dung **anonymous page** (và shmem) để kernel có thể thu hồi page frame của chúng. Nó không thêm RAM; nó cho phép đổi dữ liệu lạnh lấy RAM trống. Không có swap, anon page không bao giờ rời RAM và vmscan bỏ qua anon LRU.

## 22.2. Khái niệm nền tảng cần nhớ
- Page (dữ liệu/địa chỉ ảo) ≠ page frame (ô RAM); `struct page` là metadata của frame.
- PTE present → frame; swap PTE (không present) → (type, offset).
- Swap entry = (area, slot); slot = ô 4 KiB trên thiết bị; trạng thái slot = 1 byte `swap_map`.
- Swap cache = chỉ mục RAM (type, offset) → page, dùng `PG_swapcache` + `page->private`.

## 22.3. Bức tranh kiến trúc
vmscan (chọn) → swap_state (cache) → swap_slots (túi per-CPU) → swapfile (area, `swap_map`, cluster) → page_io (bio) → thiết bị. Ngược lại: fault (memory.c) → swap_state (tìm/đọc) → page_io → swapfile (`swap_free`). Bên cạnh: rmap (gỡ PTE), swap_cgroup + memcontrol (tính tiền), swapon/swapoff (dựng/huỷ). Sơ đồ 1 ở `02`.

## 22.4. Vai trò từng file
- `swap.c`: LRU/pagevec, `release_pages`, `rotate_reclaimable_page`, `swap_setup` — *không* phải lõi swap.
- `swapfile.c`: `swap_info_struct`, `swap_map`, cluster, cấp/giảm/free slot, extent, swapon/swapoff, `try_to_unuse`.
- `swap_state.c`: swap cache (`add_to_swap*`, `delete_from_swap_cache`, `lookup_swap_cache`, `__read_swap_cache_async`), readahead.
- `swap_slots.c`: per-CPU cache, `get_swap_page`, `free_swap_slot`, bật/tắt/drain.
- `swap_cgroup.c`: 2 byte/slot memcg ID; record/lookup/cmpxchg; swapon/swapoff.

## 22.5. Luồng swap out
`shrink_page_list` → `add_to_swap` (`get_swap_page` + `add_to_swap_cache` + dirty) → `try_to_unmap` (swap PTE, `swap_duplicate`) → flush TLB → `pageout` → `swap_writepage` (bio, bất đồng bộ) → I/O xong, `rotate_reclaimable_page` → lượt sau `__remove_mapping` (`__delete_from_swap_cache` + `put_swap_page`) → frame free. (`03`)

## 22.6. Luồng swap in
fault → `handle_pte_fault` → `do_swap_page` → `lookup_swap_cache` (hit) hoặc `swapin_readahead` → `__read_swap_cache_async` (`alloc_page_vma`, `swapcache_prepare`, `add_to_swap_cache`, memcg, LRU) + `swap_readpage` → `lock_page_or_retry` (chờ) → kiểm tra đua → `set_pte_at`, rmap → `swap_free` → (`try_to_free_swap` nếu swap đầy/mlock). (`04`)

## 22.7. Vòng đời swap slot
`00` → (cấp, có thể qua túi CPU) `40` → (gỡ N PTE) `4N` → (rời cache) `0N` → (fault) `4N` → (map lại) … → count 0 + rời cache → tạm `40` → `slots_ret` → `swap_entry_free` → `00`. (`05` §9)

## 22.8. Swap cache: vai trò và vòng đời
Vào: `add_to_swap` (out) hoặc `__read_swap_cache_async` (in). Ra: `__remove_mapping` (reclaim), `try_to_free_swap`, `reuse_swap_page`, `free_swap_and_cache`, swap đầy, swapoff. Giải quyết chia sẻ, đua I/O, tránh ghi lại. Ra khỏi cache ≠ free frame. (`04` §10)

## 22.9. swapon / swapoff
swapon: header → `swap_map` → cluster → `swap_cgroup` → extent → swap cache → `SWP_WRITEOK|SWP_VALID` → slot cache. swapoff: xoá `SWP_WRITEOK` → drain slot cache → `try_to_unuse` (đọc hết về, có thể ENOMEM/EINTR → hoàn tác) → xoá `SWP_VALID` + RCU → chờ `SWP_SCANNING` → huỷ. (`05` §11)

## 22.10. Concurrency
Byte `swap_map` ghi dưới `si->lock`/`ci->lock` với kiểm tra lại; đường cấp có thể ngủ → mutex; đường trả không được ngủ → spinlock; `SWAP_HAS_CACHE` loại trừ thêm-vào-cache; page lock, PTE lock + `pte_same`; RCU + `SWP_VALID` chống swapoff; `SWP_SCANNING` đếm người quét. (`06` §13)

## 22.11. Bảng tra cứu nhanh

> Chỉ gồm function/cấu trúc đã đọc trong cây 5.10.241. Số dòng theo `Kernel/K-S800/Src`.

| Chủ đề | File | Function/cấu trúc quan trọng | Vai trò |
|---|---|---|---|
| Mô tả swap area | `include/linux/swap.h:240` | `struct swap_info_struct` | 1 area: flags, `swap_map`, cluster, extent, lock |
| Trạng thái slot | `include/linux/swap.h:186-196` | `SWAP_HAS_CACHE`, `SWAP_MAP_MAX`, `SWAP_MAP_BAD`, `COUNT_CONTINUED` | mã hoá byte `swap_map` |
| Mã hoá entry | `include/linux/swapops.h:39-94` | `swp_entry()`, `swp_type()`, `swp_offset()`, `pte_to_swp_entry()`, `swp_entry_to_pte()` | (type, offset) ↔ PTE |
| PTE arm64 | `arch/arm64/include/asm/pgtable.h:894-913` | `__swp_entry`, `__SWP_*` | bit 2–7 type, 8–57 offset |
| Cấp slot (API) | `mm/swap_slots.c:306` | `get_swap_page()` | per-CPU cache + memcg charge |
| Nạp/trả theo lô | `mm/swap_slots.c:261, :274` | `refill_swap_slots_cache()`, `free_swap_slot()` | 64 slot/lô |
| Bật/tắt cache | `mm/swap_slots.c:69-112, :240` | `disable_swap_slots_cache_lock()`, `check_cache_active()`, `enable_swap_slots_cache()` | swapoff, swap thấp, swapon |
| Cấp slot (lõi) | `mm/swapfile.c:1056, :771` | `get_swap_pages()`, `scan_swap_map_slots()` | chọn area theo priority, quét `swap_map` |
| Cluster SSD | `mm/swapfile.c:350, :611` | `lock_cluster()`, `scan_swap_map_try_ssd_cluster()`, `struct swap_cluster_info` | khoá mịn, cấp tuần tự per-CPU |
| Tăng count | `mm/swapfile.c:3473, :3553, :3570` | `__swap_duplicate()`, `swap_duplicate()`, `swapcache_prepare()` | PTE mới / giành cờ cache |
| Giảm count | `mm/swapfile.c:1238, :1383, :1395, :1802` | `__swap_entry_free_locked()`, `swap_free()`, `put_swap_page()`, `free_swap_and_cache()` | PTE mất / rời cache / unmap |
| Free thật | `mm/swapfile.c:1466, :1362, :710` | `swapcache_free_entries()`, `swap_entry_free()`, `swap_range_free()` | byte → 0, uncharge, `nr_swap_pages++` |
| Bỏ khỏi cache | `mm/swapfile.c:1763, :1723` | `try_to_free_swap()`, `reuse_swap_page()` | giải phóng slot khi không cần |
| An toàn swapoff | `mm/swapfile.c:1318` | `get_swap_device()` / `put_swap_device()` | RCU + `SWP_VALID` |
| Vị trí đĩa | `mm/swapfile.c:2324, :223, :2378` | `map_swap_entry()`, `swap_page_sector()`, `add_swap_extent()`, `struct swap_extent` | offset → sector |
| swapon/swapoff | `mm/swapfile.c:3182, :2582, :2190` | `SYSCALL swapon`, `SYSCALL swapoff`, `try_to_unuse()`, `unuse_pte()` | dựng/huỷ, đọc hết về |
| Swap cache (cấu trúc) | `mm/swap_state.c:39; include/linux/swap.h:407-412` | `swapper_spaces[]`, `swap_address_space()` | xarray theo 64 MiB |
| Swap out | `mm/swap_state.c:216, :128` | `add_to_swap()`, `add_to_swap_cache()` | slot + cache + dirty |
| Rời cache | `mm/swap_state.c:183, :272` | `__delete_from_swap_cache()`, `delete_from_swap_cache()` | gỡ xarray, bỏ cờ |
| Swap in | `mm/swap_state.c:369, :451, :557, :891` | `lookup_swap_cache()`, `__read_swap_cache_async()`, `read_swap_cache_async()`, `swapin_readahead()` | tìm/cấp/đọc |
| Readahead | `mm/swap_state.c:570, :649, :832` | `__swapin_nr_pages()`, `swap_cluster_readahead()`, `swap_vma_readahead()` | cửa sổ thích nghi |
| swap_cgroup | `mm/swap_cgroup.c:126, :162, :97, :167, :203` | `swap_cgroup_record()`, `lookup_swap_cgroup_id()`, `swap_cgroup_cmpxchg()`, `swap_cgroup_swapon/off()` | memcg ID / slot |
| LRU hỗ trợ | `mm/swap.c:462, :265, :422, :1179` | `lru_cache_add()`, `rotate_reclaimable_page()`, `mark_page_accessed()`, `swap_setup()` | LRU, page_cluster |
| I/O | `mm/page_io.c:198, :253, :324, :48, :72` | `swap_writepage()`, `__swap_writepage()`, `swap_readpage()`, `end_swap_bio_write/read()` | bio và completion |
| Reclaim | `mm/vmscan.c:1087, :1279, :1327, :795, :869` | `shrink_page_list()`, `add_to_swap` call, `try_to_unmap` call, `pageout()`, `__remove_mapping()` | quyết định + free frame |
| Gỡ PTE | `mm/rmap.c:1627-1716` | nhánh anon của `try_to_unmap_one()` | swap PTE, `swap_duplicate` |
| Fault | `mm/memory.c:4465, :3366` | `handle_pte_fault()`, `do_swap_page()` | swap in |
| fork/exit | `mm/memory.c:705, :1342` | `copy_nonpresent_pte()`, `zap_pte_range()` → `free_swap_and_cache()` | +1 / −1 count |
| memcg | `mm/memcontrol.c:7297, :7231, :7343, :6827` | `mem_cgroup_try_charge_swap()`, `mem_cgroup_swapout()`, `mem_cgroup_uncharge_swap()`, swap-in lookup | v2 / v1 charge |
| VENDOR | `mm/swapfile.c:794-797; kernel/power/warp.c:303` | `warp_swapout_disable` | chặn cấp slot khi WARP |

## 22.12. Những điểm dễ hiểu sai nhất
1. **"`mm/swap.c` là file swap."** Nó chủ yếu là LRU/pagevec.
2. **"Linux swap cả process."** Đơn vị là page; process có thể nằm rải rác RAM/swap.
3. **"RAM đầy là swap."** Phụ thuộc còn swap, swappiness, tỉ lệ file/anon, LRU, gfp, memcg, WARP; reclaim thường lấy page cache trước.
4. **"Swap cache nằm trên đĩa."** Nằm trong RAM; là chỉ mục tới page đang ở RAM.
5. **"Swap out = ghi page."** Gồm cấp slot, vào cache, gỡ mọi PTE, ghi, chờ, free frame — qua nhiều lượt quét.
6. **"Gỡ PTE sau khi ghi."** Code gỡ trước, ghi sau; swap cache che khoảng giữa.
7. **"Swap in xong là slot được free."** Thường slot được giữ (`0x40`) để tránh ghi lại.
8. **"Xoá khỏi swap cache = free frame."** Không; chỉ bỏ một reference và cờ.
9. **"`swap_map == 0` ⇔ slot đã được giải phóng."** Slot đã "free" có thể đang chờ trong `slots_ret` với byte `0x40`.
10. **"Một slot thuộc một process."** Count có thể > 1 (fork), tối đa 62 trong byte rồi dùng continuation.
11. **"swap_cgroup chứa dữ liệu page."** Chỉ chứa ID 16-bit.
12. **"Mọi PTE không-present là swap."** Còn migration/device/hwpoison entry (`non_swap_entry`).
13. **"Có swap cache hit thì không phải chờ."** Page hit có thể đang được readahead đọc → vẫn chờ page lock.
14. **"Hành vi mainline mới áp dụng cho 5.10."** Ví dụ tối ưu COW cho page trong swapcache không có ở 5.10 (câu 25).

---

# Đáp án §21

**1.** Page = khối 4 KiB của không gian địa chỉ ảo/dữ liệu; page frame = ô 4 KiB trong RAM; `struct page` = metadata của *frame*. Khi A đã swap out xong, **không có `struct page` nào mô tả A**: frame cũ đã free (struct page của nó giờ mô tả frame trống hoặc dữ liệu khác). Thông tin "A ở đâu" chỉ nằm trong swap PTE; dữ liệu nằm trên slot. Khi A được đọc về, một `struct page` *mới* (của frame mới) mô tả A. (`01` §4.1)

**2.** Không: số frame cố định; swap chỉ là chỗ gửi tạm, truy cập phải qua page fault (chậm). Page `libc.so` là page cache **file-backed**, đã có bản gốc trong file: nếu sạch thì bỏ, đọc lại từ file khi cần; `__remove_mapping()` đi nhánh `__delete_from_page_cache()` `[SOURCE mm/vmscan.c:922-950]`; điều kiện vào nhánh swap là `PageAnon && PageSwapBacked` `[:1259]`.

**3.** (type, offset): type = chỉ số swap area (0..`MAX_SWAPFILES−1` = 0..29 trên config KM), offset = chỉ số slot. `swp_type` chọn `swap_info[type]`, `swapper_spaces[type]`, `swap_cgroup_ctrl[type]`; `swp_offset` chọn byte `swap_map`, cluster, index xarray, phần tử `swap_cgroup`, và sector qua extent. `val == 0` ⇔ (type 0, offset 0); offset 0 luôn là header (`SWAP_MAP_BAD`), không bao giờ được cấp → 0 không thể là entry hợp lệ. (`01` §4.3)

**4.** Trong RAM. Mỗi phần tử xarray của `swapper_spaces[type][offset >> 14]` tại index `offset` là con trỏ tới `struct page` (hoặc một *shadow entry* — giá trị nhỏ dùng cho workingset sau khi page bị đuổi). Page có `PG_swapcache` và `page->private = entry.val`. (`02` §6.3)

**5.** 2 byte/slot: ID của memcg sở hữu slot (`struct swap_cgroup { unsigned short id; }`). Không chứa dữ liệu, không chứa trạng thái slot. Tắt `MEMCG_SWAP` → `swap_cgroup_*` là stub trả 0 (`include/linux/swap_cgroup.h:17-43`), swap vẫn chạy đúng, chỉ mất tính tiền swap theo nhóm.

**6.** `add_to_swap` (`:1279`) → `try_to_unmap` (`:1327`) → `pageout` (`:1374`) → [lượt sau] `__remove_mapping` (`:1454`). An toàn vì sau `add_to_swap`, page nằm trong swap cache và **vẫn ở RAM**: nếu process fault trong lúc đang ghi, `do_swap_page()` → `lookup_swap_cache()` tìm thấy chính page đó và map lại; dữ liệu trong RAM luôn là bản mới nhất. TLB được flush trước khi ghi (`try_to_unmap_flush_dirty()` `:1373`) để không CPU nào còn ghi được vào page trong lúc I/O.

**7.** Vì ghi bất đồng bộ: `__swap_writepage()` gửi bio và trả về ngay với `PG_writeback`; `pageout` → `PAGE_SUCCESS` → `if (PageWriteback(page)) goto keep;` `[SOURCE mm/vmscan.c:1379-1383]`. Khi I/O xong, `end_swap_bio_write()` → `end_page_writeback()` → vì có `PG_reclaim`, gọi `rotate_reclaimable_page()` đưa page ra đuôi inactive list `[SOURCE mm/filemap.c:1488; mm/swap.c:265-279]` → lượt quét sau gặp sớm và `__remove_mapping()` free nó.

**8.** Ở `lock_page_or_retry(page, vma->vm_mm, vmf->flags)` `[SOURCE mm/memory.c:3467]`. Page được tạo ở trạng thái locked (`__SetPageLocked` `[SOURCE mm/swap_state.c:522]`); `end_swap_bio_read()` luôn `unlock_page()` khi I/O xong (thành công hay lỗi) `[SOURCE mm/page_io.c:72-95]`. Task fault ngủ trên page lock; có thể trả `VM_FAULT_RETRY` và nhả `mmap_lock` trong lúc chờ.

**9.** Khi area có `SWP_SYNCHRONOUS_IO` (block driver có `rw_page`, đặt lúc swapon `[SOURCE mm/swapfile.c:3273-3274]`) **và** `__swap_count(entry) == 1` `[SOURCE mm/memory.c:3405-3406]`: cấp page, đọc đồng bộ, không thêm vào swap cache. Trên board: không có zram/brd theo defconfig; chỉ khi driver eMMC/SSD có `rw_page` → `[SUY LUẬN]` ít khả năng; `[KIỂM TRA TRÊN BOARD]` bằng đọc driver block hoặc theo dõi `SwapCached` sau swap-in.

**10.** Vì `swap_free()` chỉ giảm count; page vẫn ở swap cache → `SWAP_HAS_CACHE` còn → byte `0x40` ≠ 0 → slot không trả. Lợi: page sạch có thể bị reclaim lại mà không cần ghi. Trả sớm khi: fault ghi + `reuse_swap_page()` thành công; `mem_cgroup_swap_full()` (swap toàn cục > 50%, hoặc memcg v2 ≥ 1/2 high/max) hoặc VMA `VM_LOCKED`/page mlocked → `try_to_free_swap()` `[SOURCE mm/memory.c:3520-3548]`; đường bypass swap cache; sau đó: unmap/exit (`free_swap_cache`), swap đầy lúc cấp (`__try_to_reclaim_swap`), reclaim đi qua `activate_locked` khi swap đầy.

**11.** `0x00` trống; `0x01` 1 tham chiếu (PTE), không có cache; `0x03` 3 tham chiếu, không cache; `0x40` count 0 + `SWAP_HAS_CACHE` (đã đặt chỗ/đang chờ trả/chỉ còn cache); `0x41` 1 tham chiếu + cache; `0x3f` `SWAP_MAP_BAD` (header, bad block, hoặc cluster đang discard); `0xbf` `SWAP_MAP_SHMEM` (slot của shmem/tmpfs). `[SIM tools/swap_numbers.out.txt §5]`

**12.** Slot trong `slots[]`: byte `0x40`, **đã bị trừ khỏi `nr_swap_pages`** (trừ trước trong `get_swap_pages()` `[:1077]`) và **đã tính vào `inuse_pages`** (`swap_range_alloc` `[:690]`) — tức hệ thống coi là "đã dùng" dù chưa gán cho page nào. Slot trong `slots_ret[]`: byte `0x40`, vẫn trong `inuse_pages`, chưa cộng lại `nr_swap_pages` — chỉ khi `swap_entry_free()` → `swap_range_free()` chạy mới được trả `[:710-741]`. `[SIM kịch bản 1 bước 1 và 10]`

**13.** `percpu_cluster` (chỉ khi non-rotational): mỗi CPU giữ một **cluster 256 slot hiện hành** trong *một* area để quét/cấp tuần tự và giảm tranh chấp `ci->lock`; vẫn chạy dưới `si->lock`, ở tầng `scan_swap_map_slots()`. `swap_slots_cache`: mỗi CPU giữ **một mảng entry đã cấp sẵn** (64 cái, có thể từ nhiều area) và một mảng entry chờ trả; ở tầng trên, tránh gọi `get_swap_pages()` và khoá toàn cục. Cái sau thường được nạp từ cái trước.

**14.** `add_to_swap()` `[swap_state.c:224]` → `get_swap_page(page)` `[swap_slots.c:306]` → (cache rỗng) `refill_swap_slots_cache()` `[:261]` → `get_swap_pages(64, cache->slots, 1)` `[swapfile.c:1056]` → `spin_lock(&si->lock)` `[:1085]` → `scan_swap_map_slots(si, SWAP_HAS_CACHE, n, slots)` `[:1106]` → `lock_cluster()` `[:872]` → `WRITE_ONCE(si->swap_map[offset], usage)` `[:893]` (usage = `SWAP_HAS_CACHE` = `0x40`).

**15.** `swap_duplicate()`: `try_to_unmap_one()` mỗi PTE `[SOURCE mm/rmap.c:1688]`; `copy_nonpresent_pte()` khi fork `[SOURCE mm/memory.c:716]` (và các đường shmem/migration không đọc chi tiết ở đây). `swap_free()`: `do_swap_page()` sau khi map `[SOURCE mm/memory.c:3545]`; `unuse_pte()` trong swapoff `[SOURCE mm/swapfile.c:1962]`. `free_swap_and_cache()`: `zap_pte_range()` khi munmap/exit gặp swap PTE `[SOURCE mm/memory.c:1342]`.

**16.** `swap_free()` `[:1383]` → `_swap_info_get()` → `__swap_entry_free()` `[:1346]` → `lock_cluster_or_swap_info()` → `__swap_entry_free_locked(p, off, 1)` `[:1238]` (count−1; nếu kết quả 0 ghi `SWAP_HAS_CACHE`, trả 0) → `free_swap_slot(entry)` `[swap_slots.c:274]` → (cache bật) vào `slots_ret`; khi đầy/drain → `swapcache_free_entries()` `[swapfile.c:1466]` → `swap_info_get_cont()` (lấy `si->lock`) → `swap_entry_free()` `[:1362]`: `lock_cluster`, `swap_map[offset] = 0` `[:1371]`, `dec_cluster_info_page`, `mem_cgroup_uncharge_swap`, `swap_range_free`.

**17.** `end_swap_bio_write()`: `SetPageError`, **`set_page_dirty(page)`** (để không bị reclaim), log `pr_alert("Write-error on swap-device (%u:%u:%llu)")`, `ClearPageReclaim`, `end_page_writeback` `[SOURCE mm/page_io.c:48-70]`. Page vẫn ở RAM trong swap cache; PTE vẫn là swap PTE (đã gỡ trước); dữ liệu an toàn trong RAM — fault sẽ hit swap cache. Lần reclaim sau thử ghi lại (vào cùng slot).

**18.** `end_swap_bio_read()`: `SetPageError`, `ClearPageUptodate`, log `"Read-error on swap-device"`, `unlock_page` `[SOURCE mm/page_io.c:77-88]`. `do_swap_page()` sau khi lock thấy `!PageUptodate(page)` → `VM_FAULT_SIGBUS` `[SOURCE mm/memory.c:3502-3505]` → process nhận **SIGBUS**.

**19.** `add_to_swap` = 0 → `activate_locked_split` → `activate_locked`: (nếu ở swap cache và swap đầy/mlock thì `try_to_free_swap`), `SetPageActive`, page về active LRU `[SOURCE mm/vmscan.c:1279-1281, :1476-1496]`. Reclaim **không** thu hồi được page đó; tổng thể có thể thất bại nếu không đủ page khác → có thể dẫn tới OOM. Không mất dữ liệu.

**20.** Đường cấp có thể **ngủ**: `refill_swap_slots_cache()` → `get_swap_pages()` → `scan_swap_map_slots()` gọi `cond_resched()` (sau khi nhả `si->lock`) → không thể giữ spinlock → mutex (comment `[SOURCE mm/swap_slots.c:25-27, :319-327]`). Đường trả **không được ngủ** vì được gọi khi người gọi đang giữ spinlock (ví dụ PTE lock trong `zap_pte_range` → `free_swap_and_cache` → `__swap_entry_free` → `free_swap_slot`) và chỉ làm việc ngắn → spinlock.

**21.** Cả hai `lookup_swap_cache` miss → cả hai vào `__read_swap_cache_async()`, cả hai cấp page, nhưng chỉ một `swapcache_prepare()` thành công (đặt `SWAP_HAS_CACHE` dưới khoá cluster/area). Task thắng thêm page vào cache và gửi I/O. Task thua nhận `-EEXIST`, `put_page()` page của mình, `schedule_timeout_uninterruptible(1)`, lặp lại: `find_get_page()` thấy page của task thắng → trả về; cả hai sau đó chờ trên page lock. Ở `do_swap_page`, `pte_same()` dưới PTE lock bảo đảm chỉ một bên map nếu cùng một PTE `[SOURCE mm/swap_state.c:461-516; mm/memory.c:3497-3500]`.

**22.** Drain: slot của area (đã đặt chỗ hoặc chờ trả, byte `0x40`) có thể nằm trong túi của bất kỳ CPU nào → được tính `inuse_pages` → `try_to_unuse()` không thể đưa `inuse_pages` về 0, và sau khi gỡ area một CPU có thể cấp entry trỏ vào area đã huỷ `[SOURCE mm/swap_slots.c:205-212; mm/swapfile.c:2654]`. `synchronize_rcu()`: các đường như `lookup_swap_cache()`, `__swap_duplicate()` truy cập `si->swap_map` bên trong `get_swap_device()` (RCU read-side, kiểm tra `SWP_VALID`). Sau khi xoá `SWP_VALID`, `synchronize_rcu()` chờ mọi đoạn đang chạy kết thúc → sau đó free `swap_map`/`cluster_info` không còn ai đọc `[SOURCE mm/swapfile.c:1278-1317, :2669-2678]`.

**23.** Vì slot sắp được đưa qua `free_swap_slot()` và có thể nằm một thời gian trong `slots_ret[]` của CPU. Nếu ghi 0 ngay, bộ cấp phát toàn cục có thể cấp slot đó cho người khác *trong khi* nó còn nằm trong `slots_ret` → khi lô được trả, `swap_entry_free()` sẽ free slot đang được dùng (và `VM_BUG_ON(count != SWAP_HAS_CACHE)` `[:1370]` sẽ vấp). Ghi `SWAP_HAS_CACHE` giữ slot ở trạng thái "đã dùng" cho tới khi `swap_entry_free()` đặt 0 dưới khoá — đúng như comment `[SOURCE mm/swap_slots.c:21-23, :288-292]`. `[SIM kịch bản 1 bước 10]`

**24.** Khi `warp_swapout_disable` khác 0, `scan_swap_map_slots()` trả 0 slot ngay → `get_swap_pages()` không cấp được gì từ mọi area → `get_swap_page()` trả entry 0 → không anon page nào bị swap out (các nhánh `add_to_swap` thất bại → `activate_locked`). An toàn với `swapoff` vì `swapoff` chờ `p->flags >= SWP_SCANNING` về dưới ngưỡng `[SOURCE mm/swapfile.c:2696]`; return đặt *trước* `si->flags += SWP_SCANNING` `[:799]` nên không để lại bộ đếm "đang quét" bị lệch (nếu đặt sau mà quên `-= SWP_SCANNING`, swapoff sẽ chờ mãi). Hunk cũng không giữ/nhả khoá nào khác — `si->lock` vẫn do người gọi giữ và nhả bình thường.

**25.** Có, thường là **có copy**. Sau fault đọc, page ở lại swap cache, nên refcount ≥ 2 (1 của PTE/mapping + 1 của swap cache). Fault ghi sau đó vào `do_wp_page()`; nhánh anon chỉ reuse khi `page_count(page) == 1` (và `page_mapcount == 1`) `[SOURCE mm/memory.c:3198-3217]` → điều kiện sai → `wp_page_copy()` cấp page mới. Ngoại lệ: nếu ngay lúc swap-in, swap đã đầy (`try_to_free_swap` đã gỡ page khỏi cache) thì refcount có thể là 1 → reuse. Ngược lại, nếu fault đầu tiên là **ghi**, `do_swap_page()` gọi `reuse_swap_page()` và không copy `[SOURCE mm/memory.c:3520-3525]`. (Pagevec giữ tạm thêm reference cũng có thể làm `page_count` > 1 — `[SUY LUẬN]`, phụ thuộc thời điểm drain.)
