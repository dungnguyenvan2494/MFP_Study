# 06 — Quan hệ với các MM subsystem · Các cặp khái niệm dễ nhầm · Câu chuyện end-to-end

> Phủ các mục: **18** Relationship với các MM subsystem · **19** Common Confusions · **20** End-to-End Example.
> Mọi `file:line` đã verify bằng `grep`/đọc trong source local 5.10.241. Hàm/biến chỉ nhắc tên (không đọc hết file đó) được ghi rõ.

---

# 18. `vmscan.c` không hoạt động một mình — ranh giới giữa các subsystem

## 18.0. Nguyên tắc đọc bảng

Mỗi dòng trả lời: **`vmscan.c ↔ file X` — hai bên trao đổi cái gì, ai gọi ai.** Chỉ liệt kê file *thực sự cần* ở 5.10.241 / board này. Ba ranh giới quan trọng nhất có sơ đồ riêng ngay sau bảng.

## 18.1. Bảng ranh giới

| File / subsystem | Hướng | Giao tiếp để làm gì | Bằng chứng (đã verify) |
|---|---|---|---|
| **`mm/page_alloc.c`** | ↔ (quan trọng nhất) | *Trigger & hậu xử lý.* Allocator: tính watermark (MIN/LOW/HIGH), phát hiện thiếu RAM, gọi `wakeup_kswapd`, gọi `try_to_free_pages` (direct reclaim), sau đó `drain_all_pages`/retry/compaction/OOM. `vmscan.c` cung cấp `zone_reclaimable_pages()` để allocator đánh giá "còn cứu được không". | `wakeup_kswapd`: `page_alloc.c:3512, 4465`; `try_to_free_pages`: `4411`; `zone_reclaimable_pages`: `4605`; `MAX_RECLAIM_RETRIES`: `mm/internal.h:125`; watermark: `page_alloc.c:8039-8045` |
| **`mm/memory.c`** | → vmscan (gián tiếp) | Page fault xin page; page *anon* mới sinh vào **inactive anon** qua `lru_cache_add_inactive_or_unevictable`; swap-in đưa lại page. | `memory.c:3687` (`do_anonymous_page`), `3540` (`do_swap_page`) |
| **`mm/swap.c`** | ↔ | *Quản lý LRU nền:* per-CPU pagevec (`lru_cache_add`, `lru_add_drain`), `mark_page_accessed` (second chance), `activate_page`, `rotate_reclaimable_page` (page vừa ghi xong về đuôi inactive), **`lru_note_cost`** (feed `anon_cost/file_cost` cho `get_scan_count`). `vmscan.c` gọi `lru_add_drain()` trước khi isolate. | `vmscan.c:1963, 2046` (drain); `swap.c:260-277, 281-320`; `mark_page_accessed` ở `swap.c` |
| **`mm/swap_state.c`, `mm/swapfile.c`, `mm/page_io.c`** | vmscan → | Swap-out anon: `add_to_swap`, `__delete_from_swap_cache`, `put_swap_page`, `try_to_free_swap`, `swap_writepage`. | `vmscan.c:1279, 919-921, 1489` |
| **`mm/rmap.c`** | vmscan → | *Reverse mapping:* hỏi "page này có PTE nào referenced không" (`page_referenced`) và **gỡ PTE** (`try_to_unmap`). Không có rmap thì kernel không thể biết page nóng hay lạnh khi nó đang được map. | `vmscan.c:1007, 2079, 1327`; `rmap.c: page_referenced` |
| **`mm/workingset.c`** | ↔ | *Trí nhớ refault:* `workingset_eviction()` do `__remove_mapping` gọi để cắm **shadow entry**; phía refault tính khoảng cách và kích hoạt page; `workingset_age_nonresident()` được gọi khi page active quay lại LRU. | `vmscan.c:918, 944, 1907`; `workingset.c:227-282` |
| **`mm/filemap.c`** | ↔ | Page cache: `__delete_from_page_cache` (vmscan tháo page); `end_page_writeback` thấy `PageReclaim` ⇒ gọi `rotate_reclaimable_page`. | `vmscan.c:945`; `filemap.c:1486-1488` |
| **`mm/page-writeback.c`, `fs/fs-writeback.c`, `mm/backing-dev.c`** | vmscan → | *Dirty page.* `wakeup_flusher_threads(WB_REASON_VMSCAN)` khi toàn dirty chưa xếp hàng; `inode_write_congested`, `wait_iff_congested`, `congestion_wait` để throttle; cơ chế **chuẩn** `balance_dirty_pages` làm throttle ở phía *người ghi* (reclaim chỉ là phương án dự phòng). | `vmscan.c:2013, 751, 2842, 2867`; `fs-writeback.c:2143` |
| **`mm/compaction.c` / `kcompactd`** | ↔ | *Order cao.* `compaction_suitable()` quyết định reclaim tiếp hay nhường; `wakeup_kcompactd()` được gọi khi kswapd ngủ/thắng boost; chia sẻ `__isolate_lru_page`/`isolate_lru_page` & `reset_isolation_suitable`. compaction dùng `zone_reclaimable_pages()` của vmscan. | `vmscan.c:2612, 2896, 3765, 3822-3828, 4019`; `compaction.c:2197` |
| **`mm/oom_kill.c`** | page_alloc → | OOM chỉ được gọi **bởi allocator** khi reclaim = 0 (`__alloc_pages_may_oom`). `vmscan.c` **không bao giờ** giết process. | `page_alloc.c:4910`; `oom_kill.c:462,927` |
| **`mm/memcontrol.c`** | ↔ | *Reclaim theo cgroup:* `try_charge`/`reclaim_high`… gọi `try_to_free_mem_cgroup_pages`; vmscan dùng `mem_cgroup_lruvec`, `mem_cgroup_calculate_protection`, `below_min/low`, soft-limit reclaim, `mem_cgroup_swapout`, `mem_cgroup_uncharge_list`. | `memcontrol.c:2443, 2744, 3381, 3514, 6378, 6427, 1782`; `vmscan.c:2641, 2653-2672, 916, 1506` |
| **`mm/vmpressure.c`** | vmscan → | Báo "hiệu suất reclaim" (scanned vs reclaimed) cho listener cgroup v1 (`low/medium/critical`). | `vmscan.c:2684, 2803, 3053`; `vmpressure.c:38-47, 121-148` |
| **`mm/vmstat.c`** | ↔ | Định nghĩa **tên counter** `/proc/vmstat` mà `vmscan.c` tăng; per-CPU threshold mà kswapd đổi khi ngủ/thức. | `vmstat.c:1245-1263`; `vmscan.c:3866, 3871` |
| **`mm/mlock.c`** | ↔ | Page `mlock` đi list **unevictable**; vmscan: `page_evictable()` loại; `PAGEREF_RECLAIM` + `try_to_unmap` để chuyển page "thua cuộc đua" sang unevictable. | `vmscan.c:1015-1016, 1128` |
| **`mm/madvise.c`** | → vmscan | `MADV_PAGEOUT` → `reclaim_pages()` (reclaim chủ động). | `madvise.c:385, 476` |
| **`mm/shmem.c`** | ↔ | tmpfs: `shmem_writepage` (swap hoặc `AOP_WRITEPAGE_ACTIVATE` nếu không có swap), `check_move_unevictable_pages`. | `shmem.c:1380, 1465, 868` |
| **`mm/huge_memory.c`** | vmscan → | `split_huge_page_to_list`, `can_split_huge_page` trước khi swap/reclaim THP. | `vmscan.c:1267-1285, 1300` |
| **`mm/truncate.c`, `fs/drop_caches.c`, `fs/splice.c`** | → vmscan | `remove_mapping()`; `drop_slab()` (`echo 2/3 > drop_caches`). | `truncate.c:212`; `drop_caches.c:65`; `splice.c:79` |
| **Shrinker owner** (fs/dcache, fs/inode, fs/super, `list_lru`…) | ↔ | Đăng ký `struct shrinker` (`count_objects/scan_objects`); vmscan điều phối. | `vmscan.c:352-425, 429-557` `[tên file owner: GENERAL, không verify]` |
| **`kernel/power/snapshot.c`, `kernel/power/warp.c`** *(VENDOR/hibernate)* | → vmscan | `shrink_all_memory()` thu nhỏ image; WARP có biến `pm_device_down/warp_canceled` mà vmscan đọc. | `snapshot.c:1786` (`hibernate_preallocate_memory`), `warp.c:1760`; vendor hunk V2/V3 |
| **`kernel/sched/*`, `include/linux/sched/prio.h`** *(VENDOR)* | vmscan → | kswapd gọi `sched_setscheduler(SCHED_RR, 200)`; ảnh hưởng nhờ `MAX_USER_RT_PRIO=512` và `sched_rt_runtime=1000000`. | `vmscan.c:3901-3904`; `prio.h:26`; `core.c:82-87` |
| **Header** | | `mmzone.h` (`pg_data_t`, `lruvec`, `enum lru_list`, `PGDAT_*`, `DEF_PRIORITY=12`), `swap.h` (`SWAP_CLUSTER_MAX=32`), `mm_inline.h` (`page_is_file_lru`, `page_lru`), `shrinker.h` (`DEFAULT_SEEKS=2`), `trace/events/vmscan.h`, `mm/internal.h` | `mmzone.h:641`, `swap.h:183`, `mm_inline.h:22`, `shrinker.h:79`, `internal.h:125` |

**Không liên quan trên board này (đừng mất thời gian):** `node_reclaim`/`zone_reclaim_mode` (NUMA off), CMA/`alloc_contig_range` (CMA off), memory hotplug (off), `CGROUP_WRITEBACK`/`BLK_CGROUP` (off).

## 18.2. Ba ranh giới có sơ đồ

### (1) `vmscan.c ↔ page_alloc.c` — "ai quyết định gì"

```
page_alloc.c                                         vmscan.c
────────────                                         ────────
watermark MIN/LOW/HIGH           ── tính ──►         (đọc: pgdat_balanced, zone_watermark_ok_safe)
fast path fail ─ wake_all_kswapds ──────────────►    wakeup_kswapd()  → kswapd → balance_pgdat
retry MIN fail ─ __perform_reclaim ─────────────►    try_to_free_pages()
                                                       ... thu N page ...
              ◄──────── N (hoặc 0, hoặc 1 nếu bị kill) ─
drain_all_pages / unreserve_highatomic  (page vừa free nằm trong per-CPU list)
should_reclaim_retry()  ─ dùng ──► zone_reclaimable_pages()   ("free hết reclaimable có đủ MIN không?")
no_progress_loops > 16  ─► OOM (oom_kill.c)
```
Quy tắc phân vai: **allocator quyết định *khi nào* và *làm gì tiếp*; vmscan chỉ quyết định *lấy ở đâu & bao nhiêu*.**

### (2) `vmscan.c ↔ swap.c / workingset.c / rmap.c` — "trí nhớ và đồng hồ"

```
rmap.c ───────── page_referenced / try_to_unmap ─────────► (vmscan hỏi "còn dùng không", "gỡ PTE")
swap.c ───────── mark_page_accessed, lru pagevec ────────► (nuôi active/inactive; vmscan drain trước khi isolate)
workingset.c ◄─── workingset_eviction (shadow) ──────────  (vmscan để lại dấu khi evict)
workingset.c ───── refault → activate → lru_note_cost ───► (feed anon_cost/file_cost → get_scan_count)
```

### (3) `vmscan.c ↔ compaction.c` — "reclaim cho số lượng, compaction cho độ liền kề"

```
order>0 request ──► page_alloc: direct compact (costly) hoặc reclaim trước
vmscan.should_continue_reclaim ── compaction_suitable()? ── "đủ free để compaction làm" → DỪNG reclaim
kswapd ngủ ─────────────► wakeup_kcompactd(alloc_order)   (reclaim xong, giờ gom liền kề)
compaction dùng ◄── zone_reclaimable_pages()  để ước lượng khả thi
```

---

# 19. Common Confusions — từng cặp dễ nhầm

### 19.1. kswapd vs direct reclaim
| | kswapd | Direct reclaim |
|---|---|---|
| Ai | kernel thread nền | chính task xin RAM |
| Khi | free < LOW | free < MIN (sau retry) |
| Hậu quả | gián tiếp | **task đứng chờ** |
**Phân biệt khi debug:** `pgscan_kswapd/pgsteal_kswapd` vs `pgscan_direct/pgsteal_direct`, và `allocstall_*` (chỉ direct).

### 19.2. Reclaim vs Compaction
| | Reclaim | Compaction |
|---|---|---|
| Mục tiêu | **tăng số lượng** page trống | **tạo khối liền kề** (order cao) bằng cách *di chuyển* page |
| Làm gì với page | gỡ khỏi RAM (free/swap/ghi) | *migrate* sang chỗ khác, không bỏ dữ liệu |
| Cần gì | page reclaimable | **free page làm đích** + page movable |
| Ở file | `vmscan.c` | `compaction.c` (+ kcompactd) |
Chúng **phối hợp**: reclaim tạo free làm đích; `should_continue_reclaim` & `compaction_ready` quyết định "đủ chưa để nhường".

### 19.3. Active vs inactive
Active = đã được dùng lặp lại (được bảo vệ); inactive = chưa chứng tỏ (ứng viên). **Reclaim chỉ free từ inactive**; active chỉ bị *hạ cấp* (`shrink_active_list`). Page lên active sau lần dùng thứ hai.

### 19.4. Anonymous vs file-backed
| | Anon | File-backed |
|---|---|---|
| Có bản gốc trên đĩa? | **Không** | Có |
| Reclaim cần | **swap** (ghi ra) | sạch: vứt; bẩn: ghi file |
| Không có swap | **không reclaim được** | vẫn reclaim được (nếu sạch) |
| Đặc biệt | **tmpfs/shmem** nằm LRU anon (`PageSwapBacked`) | |

### 19.5. Clean vs dirty
Clean = giống đĩa (vứt được). Dirty = đã sửa (phải ghi trước). Dirty **không** có nghĩa "reclaim sẽ ghi nó": đa số được *bỏ qua + đánh dấu `PageReclaim`* chờ flusher; chỉ kswapd ghi, khi `PGDAT_DIRTY`.

### 19.6. Page scan vs page reclaim
*Scan* = số page đã xem (`pgscan_*`, gồm cả page bị skip do zone); *reclaim* = số page thật sự giải phóng (`pgsteal_*`). **Hiệu suất = reclaim/scan.**

### 19.7. Page reclaim vs page free
*Reclaim* = quyết định + **tháo** page khỏi mapping (`__remove_mapping`) + đếm `nr_reclaimed`. *Free* = trả về buddy (qua `free_unref_page_list` → per-CPU list → buddy). Sau reclaim, allocator phải `drain_all_pages` để *thấy* chúng ngay. Lưu ý `nr_reclaimed` còn cộng **page do slab trả** (`reclaimed_slab`).

### 19.8. Memory pressure vs OOM
| | Pressure | OOM |
|---|---|---|
| Bản chất | trạng thái liên tục (free gần/dưới watermark) | quyết định **giết process** |
| Ai quyết | không ai — là hệ quả | **allocator** (`__alloc_pages_may_oom`) sau khi reclaim = 0 và retry hết |
| Reclaim có giết không | **Không bao giờ** | — |
Có thể áp lực rất cao kéo dài mà **không OOM** (reclaim vẫn thu đủ); và có thể **OOM dù còn nhiều cache** nếu cache đó *không reclaim được* (mlock, dirty không ghi được, `GFP_NOFS`, bị pin, **không có swap cho anon**).

### 19.9. Reclaim progress vs scan progress
`sc->nr_scanned` tăng ≠ có tiến triển. Allocator và `kswapd_failures` chỉ quan tâm **`nr_reclaimed`**. Ngoại lệ: `kswapd_shrink_node` trả "đã *quét* đủ" để **không** leo priority khi quét đã nhiều (vấn đề không còn là quét ít).

### 19.10. Background vs synchronous reclaim
Background (kswapd): task không đợi; được ghi file page; có thể ngủ chờ I/O. Synchronous (direct): task **đợi**; không ghi file page; bị throttle (`throttle_direct_reclaim`, `too_many_isolated`, `wait_iff_congested`).

---

# 20. End-to-End Example — câu chuyện một máy 1 GiB

> **Giả định minh hoạ (do bạn đặt, không phải đo từ board):** máy ~1 GiB, 1 zone, `managed ≈ 250 000 page`, **không có swap**, một process cấp phát dần bộ nhớ.
> **Số liệu** lấy từ `tools/vmscan_numbers.py` (watermark) và `tools/kswapd_toy_sim.py` (**mô hình đồ chơi**, không phải kernel; chỉ dùng để minh hoạ dòng chảy). Watermark: MIN = 1000, LOW = 1250, HIGH = 1500 page.

## Chương 1 — Yên bình
`free` còn nhiều, trên HIGH. kswapd ngủ sâu (`schedule()` trong `kswapd_try_to_sleep`). Không có reclaim.

## Chương 2 — Process bắt đầu xin RAM
Process `malloc` rồi chạm từng trang ⇒ mỗi lần page fault gọi `do_anonymous_page` ⇒ `__alloc_pages_nodemask`. `free` hạ dần. Giữa HIGH và LOW: vẫn cấp thẳng, kswapd chưa dậy.

## Chương 3 — Chạm LOW: kswapd được đánh thức, task *không* bị chậm
`free = 1200 < LOW (1250)`. Fast path (`WMARK_LOW`) fail ⇒ slowpath `wake_all_kswapds` ⇒ `wakeup_kswapd` ghi order=0, zoneidx và `wake_up_interruptible`. Slowpath retry với `WMARK_MIN` (1000): `1200 > 1000` ⇒ **thành công**. Process không hay biết gì. *(Nguồn: `page_alloc.c:4753-4760`, `vmscan.c:3983-4026`.)*

## Chương 4 — kswapd làm việc (Kịch bản A của simulator)
`balance_pgdat`: priority 12 → xuống dần; mỗi vòng quét `inactive_file >> priority`. Giả sử inactive-file = 60 000 page, thành phần: **70% sạch & không map**, 10% referenced (đang map), 15% dirty, 5% writeback:

| vòng | priority | quét | thu | cứu (activate) | free sau |
|---|---|---|---|---|---|
| 1 | 12 | 14 | 9 | 5 | 1209 |
| 2 | 11 | 29 | 20 | 9 | 1229 |
| 3 | 10 | 58 | 40 | 18 | 1269 |
| 4 | 9 | 116 | 81 | 35 | 1350 |
| 5 | 8 | 233 | 163 | 70 | **1513** |
| 6 | 7 | — | — | — | `pgdat_balanced` (≥ HIGH 1500) ⇒ **kswapd ngủ** |

Tổng quét 450, thu 313 (**hiệu suất 69.6%**). Nhận xét: *priority leo từ 12 xuống 8* (mỗi vòng tăng gấp đôi lượng quét) mà chỉ cần quét **450/60 000 = 0.75%** list. Đây là lý do reclaim thường "nhẹ nhàng" — và vì sao `pgscan_kswapd` nhỏ khi hệ thống khoẻ.

## Chương 5 — Số phận từng page trong một lô 32 (Stage 6)
Với một lô 32 page lấy từ đuôi inactive-file (tỉ lệ ở trên: ≈ 22 / 3 / 5 / 2):

| Nhóm | Số page | `shrink_page_list` làm gì | Kết cục |
|---|---|---|---|
| Sạch, không map, không referenced | ~22 | `page_check_references` ⇒ `RECLAIM` → `__remove_mapping` (kèm shadow entry) | **FREE** |
| Đang map, referenced | ~3 | `ACTIVATE` hoặc `KEEP` (`SetPageReferenced`) | **Giữ lại** (active/inactive) |
| Dirty file | ~5 | `SetPageReclaim`, `nr_vmscan_immediate_reclaim++` → activate; **không ghi** | **Giữ, chờ flusher** |
| Đang writeback | ~2 | case 2 ⇒ `SetPageReclaim` + activate | **Giữ** |

Cuối lô: `free_unref_page_list` trả ~22 page; `move_pages_to_lru` trả phần còn lại; vì **toàn bộ lô không phải toàn dirty** nên **không** gọi flusher. *(Nguồn: `vmscan.c:1087-1514, 1979-2013`.)*

## Chương 6 — Process xin nhanh hơn kswapd dọn (Kịch bản B)
Process tiếp tục xin ~100 page/vòng trong khi phần lớn page ở đuôi LRU là **nóng/dirty** (chỉ 4% sạch & không map). Mô hình đồ chơi cho thấy:

| Giai đoạn | priority | quét/vòng | thu/vòng | free |
|---|---|---|---|---|
| đầu | 12→10 | 14→58 | 0–2 | 1100 → 903 (xuống dưới MIN từ vòng 3) |
| giữa | 8→5 | 233→1817 | 9→72 | 716 → 542 |
| cuối | 2→1 | 1400→2101 | 56→84 | 99 → 42 (gần 0); **199 page process muốn mà không được cấp** (`stalled`) |
| **Tổng** | | **58 949** | **2 343** | **hiệu suất 4.0%** (inactive-file từ 60 000 còn 1 051: page "bị cứu" rời đuôi list) |

Điểm cần thấy:
1. Khi `free < MIN` (từ vòng 3), process **tự vào direct reclaim** ⇒ **bị chậm**, `allocstall_*` tăng. (Mô hình đồ chơi không mô phỏng direct reclaim; nó chỉ đánh dấu.)
2. kswapd **quét rất nhiều (58 949) nhưng thu rất ít (2 343)**; `scan ≥ nr_to_reclaim` nên `raise_priority=false` — kswapd *không* leo priority thêm nhưng vẫn quay.
3. **kswapd không "hopeless"**: mỗi đợt vẫn thu > 0 page ⇒ `kswapd_failures` không tăng ⇒ kswapd tiếp tục quay. Trên board này kswapd là **SCHED_RR 200, không bị RT throttle** ⇒ nó có thể ăn trọn một CPU (xem file 00 mục 1.6).

## Chương 7 — Không có swap, anon chiếm RAM (Kịch bản C)
Nếu inactive-file chỉ còn 3 000 page (anon/tmpfs chiếm phần còn lại; `get_scan_count` loại anon vì không swap), kswapd chỉ quét được rất ít ở priority cao (vòng 1: 0 page, vòng 2: 1 page…) và phải leo tới priority 1 mới quét được ~868 page. Kết quả sau 12 vòng: quét 2 131, thu 1 273 (hiệu suất 59.7%), nhưng inactive-file bị **rút cạn từ 3 000 xuống 869** trong khi `free` mới chỉ lên 1 073 — đã qua MIN (1000) nhưng **vẫn dưới HIGH (1500)** — và đã đi qua đáy 304 page ở giữa chừng. Lần sau gần như không còn gì để thu ⇒ kswapd bắt đầu không thu nổi page nào ⇒ `kswapd_failures++` … ⇒ OOM nếu process vẫn xin tiếp. *(Mô hình đồ chơi; thông điệp: **kho page cache sạch là hữu hạn**, hết là hết — và không swap thì anon không thể bù vào.)*

## Chương 8 — Dirty page và flusher
Khi lô toàn dirty chưa xếp hàng, `wakeup_flusher_threads(WB_REASON_VMSCAN)` đánh thức flusher; flusher ghi; `end_page_writeback` thấy `PageReclaim` ⇒ `rotate_reclaimable_page` đưa page về **đuôi inactive** ⇒ lần quét sau page đã sạch ⇒ **FREE**. Nếu storage chậm, direct reclaim ngủ ở `congestion_wait`/`wait_iff_congested` (file 02, 8.5).

## Chương 9 — Kết thúc
- **Nhánh tốt:** thu đủ ⇒ `free ≥ HIGH` ⇒ `pgdat_balanced` ⇒ kswapd nap 100 ms rồi ngủ dài; process tiếp tục; `kswapd_failures` về 0.
- **Nhánh xấu:** `try_to_free_pages` trả 0 liên tiếp ⇒ `should_reclaim_retry` hết 16 lần ⇒ `__alloc_pages_may_oom` ⇒ `invoked oom-killer` ⇒ `Out of memory: Killed process …`. Nếu allocation là atomic: `page allocation failure` ngay (không reclaim).

## Chương 10 — Bản đồ câu chuyện → hàm → thứ bạn sẽ thấy

| Chương | Hàm chính | Counter/Log bạn thấy |
|---|---|---|
| 2 | `do_anonymous_page` → `__alloc_pages_nodemask` | — |
| 3 | `wake_all_kswapds` → `wakeup_kswapd` | `trace_mm_vmscan_wakeup_kswapd` (nếu có) |
| 4 | `balance_pgdat` → `kswapd_shrink_node` → `shrink_node` → `shrink_lruvec` | `pageoutrun`, `pgscan_kswapd`, `pgsteal_kswapd` |
| 5 | `shrink_inactive_list` → `shrink_page_list` | `pgactivate`, `nr_vmscan_immediate_reclaim` |
| 6 | `__alloc_pages_direct_reclaim` → `try_to_free_pages` → `do_try_to_free_pages` | `allocstall_*`, `pgscan_direct`, `pgsteal_direct` |
| 7 | `get_scan_count` (SCAN_FILE) | `pgscan_anon = 0` |
| 8 | `wakeup_flusher_threads`, `rotate_reclaimable_page` | `nr_vmscan_write` |
| 9 | `pgdat_balanced`/`kswapd_try_to_sleep` / `__alloc_pages_may_oom` | `kswapd_*_wmark_hit_quickly` / OOM log |

Tiếp theo → [`07_final_model_cheatsheet.md`](07_final_model_cheatsheet.md).
