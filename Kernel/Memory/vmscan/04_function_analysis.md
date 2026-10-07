# 04 — Function-by-Function: phân tích mức khái niệm

> Phủ mục **14**. Thứ tự giải thích mỗi function (theo quy tắc của bạn):
> **Vấn đề → Vì sao tồn tại → Quyết định nó đưa ra → Flow → Ý nghĩa từng action → Kết quả → Ai dùng kết quả → Ảnh hưởng → Dấu vết debug → (cuối) map về code.**
> Mọi `file:line` là source local 5.10.241. Hàm gạch chân `VENDOR` có hunk của Konica Minolta.

---

## 14.0. Phân loại — "hàm nào làm vai gì"

| Vai trò | Hàm |
|---|---|
| **Entry / trigger** | `wakeup_kswapd`, `kswapd`, `try_to_free_pages`, `try_to_free_mem_cgroup_pages`, `shrink_all_memory`, `reclaim_pages` |
| **Tạo ra *quyết định* reclaim** | `wakeup_kswapd`, `pgdat_balanced`, `prepare_kswapd_sleep`, `allow_direct_reclaim`, `get_scan_count`, `inactive_is_low`, `page_check_references`, `should_continue_reclaim`, `compaction_ready`, `too_many_isolated` |
| **Điều phối (orchestration)** | `balance_pgdat`, `kswapd_shrink_node`, `do_try_to_free_pages`, `shrink_zones`, `shrink_node`, `shrink_node_memcgs` |
| **Thực hiện *scanning*** | `shrink_lruvec`, `shrink_list`, `shrink_inactive_list`, `shrink_active_list`, `isolate_lru_pages`, `__isolate_lru_page`, `move_pages_to_lru` |
| **Thực hiện *reclaim*** | `shrink_page_list`, `pageout`, `__remove_mapping`, `do_shrink_slab`/`shrink_slab` |
| **Throttle / chống bão** | `throttle_direct_reclaim`, `too_many_isolated`, `kswapd_try_to_sleep`, `current_may_throttle` |
| **Hàm liên quan *debug / thống kê*** | `snapshot_refaults`, `set_task_reclaim_state`, `handle_write_error`, các lời gọi `vmpressure`, `count_vm_event(s)`, `trace_mm_vmscan_*` |
| **Critical (hỏng ⇒ treo/OOM/mất dữ liệu)** | `shrink_page_list`, `__remove_mapping`, `pageout`, `isolate_lru_pages`, `balance_pgdat`, `do_try_to_free_pages` |

> **Quy ước template (rút gọn có chủ đích cho hàm phụ):** 1 Mục đích · 2 Vị trí flow · 3 Input · 4 Flow · 5 Ý nghĩa action · 6 Kết quả & ai dùng · 7 Ảnh hưởng · 8 Failure/edge · 🔎 Debug · 📍 Code.

---

# Nhóm A — Entry & Trigger

## FUNCTION: `wakeup_kswapd` — *quyết định có đánh thức kswapd không*

**1. Mục đích.** Allocator chỉ biết "zone này sắp thiếu"; nó cần một cách *rẻ* để nhờ kswapd làm nền. Hàm này là **cầu nối allocator → kswapd** và là **người gác cổng**: không đánh thức nếu vô ích.
**2. Vị trí.** `__alloc_pages_slowpath → wake_all_kswapds → wakeup_kswapd → (wake_up) → kswapd`.
**3. Input.** Zone nào thiếu; gfp của người xin (có `DIRECT_RECLAIM` không); order; zone cao nhất được dùng.
**4. Flow.**
```
validate zone/cpuset → ghi yêu cầu (max order, max zoneidx) → kswapd bận? thoát
 → node hopeless hoặc đã balanced&&!boost? (thì chỉ wake kcompactd nếu không có DIRECT_RECLAIM) thoát
 → trace → wake_up_interruptible
```
**5. Ý nghĩa action.**
| Action | Tại sao | Nếu bỏ |
|---|---|---|
| Ghi order/zoneidx *trước* khi kiểm tra "ngủ chưa" | Không mất yêu cầu khi kswapd đang bận | kswapd chạy với zoneidx cũ, reclaim sai zone |
| Chặn khi `kswapd_failures ≥ 16` | Node vô vọng; đánh thức chỉ tốn CPU | Vòng lặp "wake→fail→sleep→wake" |
| Chặn khi node đã balanced | Tránh reclaim thừa | kswapd chạy vô ích |
| Gọi kcompactd khi chỉ cần compaction | Free đủ nhưng phân mảnh | high-order mãi fail |
**6. Kết quả.** `void`; thay đổi `pgdat->kswapd_order`, `kswapd_highest_zoneidx` và có thể làm kswapd chạy.
**7. Ảnh hưởng.** kswapd, kcompactd; không đụng LRU trực tiếp.
**8. Edge.** kswapd đang chạy ⇒ không wake (chỉ cập nhật tham số); node hopeless + allocation order-0 ⇒ **không ai reclaim nền**, chỉ còn direct reclaim.
🔎 `trace_mm_vmscan_wakeup_kswapd` (nid, order, gfp_flags) — chỉ khi tracepoint có mặt. 📍 `vmscan.c:3983-4026`.

---

## FUNCTION: `kswapd` — *vòng đời của thread nền*

**1. Mục đích.** Chạy mãi: "ngủ → thức khi có yêu cầu → cân bằng node → ngủ".
**2. Vị trí.** `kswapd_run → kthread → kswapd() → {kswapd_try_to_sleep, balance_pgdat}`.
**3. Input.** `pgdat` của node; yêu cầu `order`/`highest_zoneidx` do allocator ghi.
**4. Flow.** Xem Flow 2 (file 02, 7.3).
**5. Ý nghĩa action.**
| Action | Tại sao |
|---|---|
| `set_cpus_allowed_ptr(node cpus)` | kswapd chạy gần RAM của node |
| `PF_MEMALLOC\|PF_SWAPWRITE\|PF_KSWAPD` | Được dùng reserve; không reclaim đệ quy; được ghi swap; `current_is_kswapd()` đúng |
| `set_freezable` + `try_to_freeze` | Không reclaim khi hệ thống đóng băng (suspend/hibernate) |
| **VENDOR** `sched_setscheduler(SCHED_RR, 200)` | Làm kswapd *real-time* để reclaim không bị task thường chèn |
| Reset `kswapd_order=0`, `highest_zoneidx=MAX_NR_ZONES` sau khi đọc | "Đã nhận yêu cầu"; yêu cầu mới sẽ ghi lại |
**6. Kết quả.** Không trả (chạy tới `kthread_stop`).
**7. Ảnh hưởng.** Toàn bộ reclaim nền; CPU; scheduling.
**8. Edge.** `sched_setscheduler` **bị bỏ qua giá trị trả về**; trên board nó thành công vì `HIGH_PRIORITY=y` (xem file 00 mục 1.6). Nếu kernel khác build không có `HIGH_PRIORITY` ⇒ thất bại im lặng, kswapd là `SCHED_NORMAL`.
🔎 `ps -o pid,cls,rtprio,comm | grep kswapd` (kỳ vọng `RR 200`). 📍 `vmscan.c:3894-3974`, vendor `3901-3904`.

---

## FUNCTION: `kswapd_try_to_sleep` (+ `prepare_kswapd_sleep`, `pgdat_balanced`) — *khi nào được phép ngủ?*

**1. Mục đích.** Quyết định kswapd **ngủ thật** hay **tiếp tục làm việc**, tránh "ngủ rồi lại bị đánh thức ngay" (flapping).
**2. Vị trí.** Đầu mỗi vòng `kswapd()`.
**3. Input.** Tình trạng balanced của node (qua `pgdat_balanced`); `kswapd_failures`; có task đang chờ ở `pfmemalloc_wait`?
**4. Flow.** Xem file 02 mục 7.5 (nap 100 ms → ngủ dài).
**5. Ý nghĩa action.**
| Action | Tại sao | Nếu bỏ |
|---|---|---|
| `prepare_kswapd_sleep`: đánh thức `pfmemalloc_wait` | Không ai được ngủ chờ kswapd khi kswapd sắp ngủ | Task bị throttle mãi |
| Nap 100 ms (`HZ/10`) trước khi ngủ dài | "Ngủ ngắn xem có bị đánh thức không" → phát hiện *premature sleep* | kswapd ngủ/dậy liên tục |
| `reset_isolation_suitable` + `wakeup_kcompactd` | Reclaim xong ⇒ compaction có cơ hội thành công | phân mảnh không được xử lý |
| Hạ/nâng `per-cpu vmstat threshold` | Số liệu free chính xác khi đang áp lực | Vi phạm watermark vì số liệu trễ |
| Đếm `KSWAPD_LOW/HIGH_WMARK_HIT_QUICKLY` | **Đo "áp lực kéo dài"** | — |
**6. Kết quả.** kswapd ngủ hoặc quay lại `balance_pgdat`.
**7. Ảnh hưởng.** vmstat counter, kcompactd, task đang throttle.
**8. Edge.** `kswapd_failures ≥ 16` ⇒ `prepare_kswapd_sleep` trả `true` ngay (hopeless).
🔎 `kswapd_low_wmark_hit_quickly`, `kswapd_high_wmark_hit_quickly`, `trace_mm_vmscan_kswapd_sleep`. 📍 `3487-3516, 3797-3879, 3439-3469`.

---

## FUNCTION: `balance_pgdat` — *một đợt cân bằng node của kswapd*  ★ CRITICAL

**1. Mục đích.** "Đưa node về trạng thái có ít nhất một zone ≥ HIGH, tốn ít nhất, không reclaim thừa."
**2. Vị trí.** `kswapd → balance_pgdat → {age_active_anon, kswapd_shrink_node → shrink_node}`.
**3. Input.** `order`, `highest_zoneidx` (do allocator yêu cầu); watermark boost hiện có.
**4. Flow.** Xem file 02 mục 7.4.
**5. Ý nghĩa action.** Bảng đầy đủ ở file 02 mục 7.4.
**6. Kết quả.** Trả `sc.order` (order mà kswapd *thực sự* đã reclaim; có thể hạ về 0). Hiệu ứng phụ: `kswapd_failures`, trả boost, wake kcompactd, `snapshot_refaults`.
**7. Ảnh hưởng.** LRU, slab, watermark boost, PSI (`psi_memstall_enter/leave`), `PAGEOUTRUN`.
**8. Edge.** Boost reclaim mà 0 tiến triển ⇒ dừng (để khỏi vòng vô hạn vì không ghi/ swap được). `kthread_should_stop`/freeze ⇒ thoát giữa chừng. `buffer_heads_over_limit` ⇒ mở rộng `reclaim_idx` (không có ý nghĩa highmem trên arm64).
🔎 `pageoutrun`, `pgscan_kswapd`, `pgsteal_kswapd`, `trace_mm_vmscan_kswapd_wake`. 📍 `3574-3780`.

---

## FUNCTION: `kswapd_shrink_node` — *một nhát reclaim cho cả node*

**1. Mục đích.** Chọn **mục tiêu** (`nr_to_reclaim`) rồi gọi `shrink_node` cho node.
**2. Vị trí.** `balance_pgdat → kswapd_shrink_node → shrink_node`.
**3. Input.** `sc` (đã có `reclaim_idx`, `order`, `priority`).
**4. Flow.** `nr_to_reclaim = Σ max(high_wmark, 32)` trên zone ≤ reclaim_idx → `shrink_node` → nếu `order && nr_reclaimed ≥ compact_gap(order)` thì `order = 0` → trả `nr_scanned ≥ nr_to_reclaim`.
**5. Ý nghĩa.** Mục tiêu theo **tổng HIGH** (không phải "free hiện tại + X") vì kswapd muốn đưa mọi zone vượt HIGH. Trả `true` ⇒ "đã *quét* đủ nhiều" ⇒ `balance_pgdat` **không tăng priority**.
**6. Kết quả.** Bool "quét đủ hay chưa" — **không phải** "thu đủ hay chưa".
**7. Ảnh hưởng.** `sc->nr_to_reclaim`, `sc->order`.
**8. Edge.** Fragmentation: reclaim đủ 2× gap thì thôi để tránh dọn thừa.
📍 `3526-3559`.

---

## FUNCTION: `try_to_free_pages` — *cửa vào của direct reclaim*

**1. Mục đích.** Cho allocator một API đồng bộ: "hãy giải phóng ≥ vài page ngay bây giờ".
**2. Vị trí.** `__perform_reclaim → try_to_free_pages → {throttle_direct_reclaim, do_try_to_free_pages}`.
**3. Input.** zonelist, order, gfp, nodemask.
**4. Flow.** Dựng `sc` (32 page, priority 12) → `throttle_direct_reclaim` → set reclaim_state → `do_try_to_free_pages` → trả số page.
**5. Ý nghĩa.** `set_task_reclaim_state` để slab free được cộng vào `nr_reclaimed`; `trace_..._begin/end` bao quanh để đo thời gian direct reclaim.
**6. Kết quả.** `0` = không reclaim được gì; `1` (đặc biệt) = bị kill khi đang throttle ⇒ **đừng OOM**; ngược lại = số page.
**7. Ảnh hưởng.** Task gọi (chậm), LRU, slab.
**8. Edge.** `PF_MEMALLOC` task không vào đây (allocator chặn); `BUILD_BUG_ON` đảm bảo `order/priority/reclaim_idx` vừa `s8`.
🔎 `allocstall_*`, `pgscan_direct`, `pgsteal_direct`, `trace_mm_vmscan_direct_reclaim_begin/end`. 📍 `3265-3306`.

---

## FUNCTION: `throttle_direct_reclaim` / `allow_direct_reclaim` — *đừng đổ xô vào reclaim khi kho dự trữ cạn*

**1. Mục đích.** Khi reserve `PFMEMALLOC` (≈ Σ MIN watermark các zone ≤ NORMAL) cạn, nhiều direct reclaimer cùng chạy chỉ **tranh nhau** page dự trữ mà không tiến triển. ⇒ bắt chúng **chờ kswapd**.
**2. Vị trí.** Ngay đầu `try_to_free_pages`.
**3. Input.** Node ưu tiên đầu tiên có zone ≤ NORMAL; task có phải kthread/đang bị kill không; gfp có `__GFP_FS` không.
**4. Flow.**
```
kthread / fatal_signal → khỏi throttle
chọn node đầu tiên có zone ≤ NORMAL
allow_direct_reclaim(pgdat)?  ── có → khỏi throttle
PGSCAN_DIRECT_THROTTLE++
!__GFP_FS ? chờ ≤ 1s (HZ) trên pfmemalloc_wait : chờ killable tới khi allow_direct_reclaim
bị kill? → return true (try_to_free_pages trả 1)
```
**5. Ý nghĩa.** Kthread **không** bị throttle vì có thể chính nó dọn page cho người khác (vd. kjournald). `!__GFP_FS` chỉ chờ ≤ 1s vì kswapd có thể đang bị chặn bởi cùng lock FS ⇒ chờ vô hạn sẽ deadlock.
**6. Kết quả.** `true` nếu bị kill khi chờ (⇒ không OOM); `false` ⇒ tiếp tục reclaim.
**7. Ảnh hưởng.** Đánh thức kswapd (`allow_direct_reclaim`), task bị ngủ.
**8. Edge.** `kswapd_failures ≥ 16` ⇒ `allow_direct_reclaim = true` (không bắt chờ kswapd đã bỏ cuộc). Reserve = 0 ⇒ không throttle.
🔎 `pgscan_direct_throttle` > 0 ⇒ *task đã ngủ chờ kswapd*. 📍 `3132-3263`.

---

## FUNCTION: `do_try_to_free_pages` — *vòng leo thang priority của direct reclaim*  ★ CRITICAL

**1. Mục đích.** "Giải phóng đủ nhiều bằng cách bắt đầu nhẹ, tăng dần cho tới khi đủ hoặc hết cách."
**2. Vị trí.** `try_to_free_pages | try_to_free_mem_cgroup_pages | shrink_all_memory → do_try_to_free_pages → shrink_zones`.
**3. Input.** zonelist, `sc` (mục tiêu 32 page, phạm vi).
**4. Flow.** Xem file 02 mục 8.3.
**5. Ý nghĩa action.**
| Action | Tại sao | Nếu bỏ |
|---|---|---|
| Vòng `--priority` 12→0 | Tăng áp lực **chỉ khi cần** | Luôn quét mạnh (đắt) hoặc luôn quét nhẹ (không đủ) |
| `priority < 10 ⇒ may_writepage` | Khó khăn thì cho phép ghi (kể cả laptop mode) | Kẹt vì dirty page |
| `vmpressure_prio` | Báo cho memcg listener "đang leo thang" | Mất tín hiệu pressure |
| `snapshot_refaults` | Chốt mốc refault để lần sau biết "có refault mới không" | `may_deactivate` sai |
| Retry `force_deactivate` / `memcg_low_reclaim` | Ước lượng sai → thử lại trước khi trả 0 | OOM oan |
| **VENDOR** `warp_canceled` ⇒ break | Hủy nhanh khi WARP bị cancel | Reclaim cố hoàn thành trong lúc đang hủy |
**6. Kết quả.** Số page đã reclaim; `0` = thất bại hoàn toàn (⇒ allocator vào compaction/OOM).
**7. Ảnh hưởng.** `ALLOCSTALL`, delay accounting (`delayacct_freepages_*`, nhưng `TASKSTATS` off), memcg congested flag.
**8. Edge.** Đếm `ALLOCSTALL` **mỗi lần vào `retry:`** (có thể >1 lần/1 allocation) ⇒ số `allocstall_*` ≥ số lần task thực sự stall.
🔎 `allocstall_*`. 📍 `3039-3130`, vendor `3071-3074`.

---

## FUNCTION: `shrink_zones` — *chọn node để reclaim (direct)*

**1. Mục đích.** Từ zonelist (có thứ tự ưu tiên) chọn các **node** thực sự cần shrink.
**2. Input.** zonelist; `reclaim_idx`, nodemask; cpuset; order.
**4. Flow.** Xem file 02 mục 8.4.
**5. Ý nghĩa.** *Một node một lần* (`last_pgdat`) vì LRU nằm ở node, không ở zone; *bỏ qua zone khi `compaction_ready`* để compaction làm thay vì reclaim thừa.
**6. Kết quả.** Gián tiếp (cộng vào `sc->nr_reclaimed`, `nr_scanned`).
**8. Edge.** cpuset chặn hết zone ⇒ không reclaim gì ⇒ trả 0 ⇒ OOM.
📍 `2926-3009`.

---

## FUNCTION: `try_to_free_mem_cgroup_pages` — *reclaim vì memcg chạm limit*

**1. Mục đích.** Giữ một cgroup trong `memory.max/high` mà **không cần RAM toàn máy thấp**.
**2. Vị trí.** `memcontrol.c: try_charge / reclaim_high / force_empty… → try_to_free_mem_cgroup_pages → do_try_to_free_pages`.
**3. Input.** memcg, `nr_pages` cần, gfp, `may_swap`.
**5. Ý nghĩa.** `target_mem_cgroup` ⇒ `cgroup_reclaim(sc)=true` ⇒ chỉ quét subtree; `memalloc_noreclaim_save` ⇒ không đệ quy.
**6. Kết quả.** số page reclaim.
**8. Edge.** Không tăng `ALLOCSTALL`; writeback throttling kiểu legacy nếu cgroup v1 ⇒ có thể `wait_on_page_writeback`.
📍 `3350-3386`.

---

# Nhóm B — Điều phối

## FUNCTION: `shrink_node` — *quản đốc của một node*  ★ CORE

**1. Mục đích.** Với một node: (a) **đặt các tham số chiến lược** (anon hay file? có hạ cấp không?), (b) **thực thi** qua từng memcg, (c) **đo triệu chứng** (dirty/writeback/congested) và đặt cờ throttle, (d) quyết định có lặp lại cho high-order không.
**2. Vị trí.** `shrink_zones | kswapd_shrink_node → shrink_node → shrink_node_memcgs`.
**3. Input.** pgdat, `sc` (priority, mục tiêu, phạm vi).
**4. Flow.**
```
again:
  reset sc->nr (bộ đếm triệu chứng)
  lấy anon_cost/file_cost (dưới lru_lock)
  tính may_deactivate (DEACTIVATE_ANON/FILE) ← refault mới? hay inactive quá nhỏ?
  tính cache_trim_mode  ← inactive file còn nhiều & không thrash
  tính file_is_tiny     ← (global) file+free ≤ Σ high_wmark & anon còn
  shrink_node_memcgs()
  cộng slab đã reclaim (reclaim_state->reclaimed_slab) vào nr_reclaimed
  vmpressure(root)
  reclaimable ⇐ thu ≥1 page
  kswapd: đặt PGDAT_WRITEBACK / PGDAT_DIRTY ; nr.immediate ⇒ congestion_wait(HZ/10)
  (kswapd | memcg sane): nr.dirty == nr.congested ⇒ LRUVEC_CONGESTED
  direct & LRUVEC_CONGESTED ⇒ wait_iff_congested(HZ/10)
  should_continue_reclaim ? goto again
  reclaimable ⇒ kswapd_failures = 0
```
**5. Ý nghĩa action.**
| Action | Tại sao | Nếu bỏ |
|---|---|---|
| `may_deactivate` | Chỉ hạ cấp active khi có bằng chứng (refault mới / inactive nhỏ) | Hạ cấp thừa → đẩy working set ra; hoặc thiếu → inactive cạn |
| `cache_trim_mode` | Có sẵn nhiều cache sạch ⇒ **đừng** động anon (đắt) | Swap thừa |
| `file_is_tiny` | Chống **cache trap**: file LRU bé xíu thrash nhưng "hấp dẫn" vô hạn | Vòng phản hồi chết: quét file bé mãi, không bao giờ đụng anon |
| Cờ `PGDAT_*`/`LRUVEC_CONGESTED` | Biến *quan sát* ở đuôi LRU thành *quyết định* (cho kswapd ghi / bắt direct ngủ) | Reclaim cứ đập vào dirty page / I/O nghẽn |
| Cộng slab vào `nr_reclaimed` | Slab cũng là "thu hồi" | Tưởng không tiến triển dù slab đã trả |
| `kswapd_failures = 0` khi reclaimable | Cho kswapd "sống lại" | kswapd bị bỏ rơi mãi sau lần fail |
**6. Kết quả.** Cập nhật `sc->nr_reclaimed/nr_scanned`, cờ node, `kswapd_failures`.
**7. Ảnh hưởng.** LRU, slab, cờ pgdat/lruvec, vmpressure, ngủ (congestion_wait/wait_iff_congested).
**8. Edge.** `reclaim_state == NULL` ⇒ slab không được tính (đã có `set_task_reclaim_state` ở mọi entry); memcg bị bảo vệ hết ⇒ `nr_reclaimed = 0` ⇒ OOM.
🔎 Gần như mọi counter `pgscan/pgsteal`, `nr_vmscan_*`. 📍 `2691-2881`.

## FUNCTION: `shrink_node_memcgs` — *đi qua cây memcg*

**1. Mục đích.** `lruvec` tồn tại **theo từng memcg**; cần duyệt cả cây (hoặc subtree khi memcg-reclaim) và **tôn trọng bảo vệ** `memory.min/low`.
**4. Flow.** `mem_cgroup_iter` → `cond_resched` → `mem_cgroup_calculate_protection` → `below_min` ⇒ `continue` (cứng) / `below_low` ⇒ `continue` + `memcg_low_skipped=1` (mềm, trừ khi `memcg_low_reclaim`) → `shrink_lruvec` → `shrink_slab(memcg)` → `vmpressure(memcg)`.
**5. Ý nghĩa.** `cond_resched` vì có thể CPU-bound khi memcg không đủ điều kiện reclaim (comment 2646-2650).
**8. Edge.** Mọi memcg được bảo vệ ⇒ không có gì để lấy ⇒ vòng retry `memcg_low_reclaim` ở `do_try_to_free_pages`.
📍 `2634-2689`.

## FUNCTION: `should_continue_reclaim` — xem file 03 mục 13.3. 📍 `2581-2632`.

---

# Nhóm C — Scanning

## FUNCTION: `shrink_lruvec` — *quét 4 list theo tỉ lệ đã định*  ★ CORE

**1. Mục đích.** Với một `lruvec`: hỏi `get_scan_count` quét mỗi list bao nhiêu rồi **thực sự quét theo lô 32**, đồng thời giữ cân bằng anon/file khi đã đủ mục tiêu.
**4. Flow.** Xem file 03 mục 10.2.
**5. Ý nghĩa action.**
| Action | Tại sao | Nếu bỏ |
|---|---|---|
| Lô 32 (`SWAP_CLUSTER_MAX`) xen kẽ 4 list | Không bị kẹt ở một list; giữ độ trễ lock thấp; có `cond_resched()` | Quét một cục lớn, giữ CPU/lock lâu |
| `blk_start_plug/finish_plug` | Gom I/O (từ `pageout`) thành lô | I/O nhỏ lẻ |
| *Proportional* khi đã đủ `nr_to_reclaim` | Giữ tỉ lệ anon:file đã định | Một loại bị bỏ đói |
| Rebalance `active anon` cuối | Anon active luôn cần xoay dù không evict anon | Inactive anon cạn khi cần swap |
**6. Kết quả.** `sc->nr_reclaimed` tăng.
**7. Ảnh hưởng.** LRU list, `pgscan/pgsteal`.
**8. Edge.** Direct reclaim ở `DEF_PRIORITY` không dừng sớm (làm trọn mẻ). Lỗi: `BUG()` ở `default:` của `switch(scan_balance)` (không thể xảy ra).
📍 `2451-2561`.

## FUNCTION: `get_scan_count` — *chọn anon hay file, bao nhiêu*  ★ DECISION

Xem cây quyết định đầy đủ ở file 03 mục 9.5. **Kết quả:** `nr[lru]` ∈ page. **Ai dùng:** `shrink_lruvec`. **Vắng mặt:** không biết quét gì → hoặc quét hết anon vô ích (không swap), hoặc đập mãi vào file đang thrash. **Edge:** `memcg_low` scaling; cgroup đã xoá (`!mem_cgroup_online`) phải "cạo sạch" cache còn sót. 🔎 gián tiếp: tỉ lệ `pgscan_anon : pgscan_file`. 📍 `2255-2449`.

## FUNCTION: `shrink_inactive_list` — xem file 03 mục 10.4. ★ CORE. 📍 `1937-2027`.
## FUNCTION: `shrink_active_list` — xem file 03 mục 10.5. 📍 `2029-2122`.

## FUNCTION: `isolate_lru_pages` (+ `__isolate_lru_page`) — "lấy hàng ra khỏi kệ" ★ CRITICAL

**1. Mục đích.** Rút ≤N page từ đuôi LRU *an toàn*, để xử lý ngoài lock.
**3. Input.** Số page cần xem; lruvec/list; `sc` (zone cap `reclaim_idx`, `may_unmap`).
**4–5.** Xem file 03 mục 10.3. Điểm mấu chốt: *skip theo zone không tính vào quota*; *page đang bị free chỗ khác (`-EBUSY`) thì trả lại*.
**6. Kết quả.** `nr_taken`, `*nr_scanned` (gồm cả skip), `dst` list; cập nhật size.
**7. Ảnh hưởng.** `PG_lru` bị xoá, refcount +1, `NR_ISOLATED_*` (do caller), `PGSCAN_SKIP`.
**8. Edge.** `BUG()` nếu `__isolate_lru_page` trả mã lạ (≠0, ≠-EBUSY).
🔎 `pgscan_skip_*`, `nr_isolated_*`. 📍 `1565-1755`.

## FUNCTION: `too_many_isolated` — *van chống bão*

**1. Mục đích.** Nếu số page đang "bị rút khỏi LRU" lớn hơn inactive còn lại ⇒ LRU sắp trống giả tạo ⇒ quét càng quyết liệt ⇒ OOM sớm. ⇒ bắt direct reclaimer **chờ**.
**5.** kswapd được miễn; caller `GFP_NOIO/NOFS` được nới (`inactive >>= 3` chỉ áp cho GFP đầy đủ) để tránh deadlock vòng với caller có lock; memcg không "sane" được miễn.
**8. Edge.** `msleep(100)` một lần; nếu vẫn quá nhiều ⇒ trả 0 (task coi như không tiến triển).
📍 `1815-1843`.

## FUNCTION: `move_pages_to_lru` — *trả hàng về kệ*

**1. Mục đích.** Đưa page *chưa free* về đúng list (đầu); xử lý page bị free trong lúc ta cầm (refcount về 0) và page đã thành unevictable.
**5.** Với `put_page_testzero == true`: page chủ sở hữu khác đã bỏ ref ⇒ *ta* phải free nó (không thì rò). Page active quay lại ⇒ `workingset_age_nonresident`.
📍 `1865-1917`.

## FUNCTION: `inactive_is_low` — xem file 03 mục 9.4. 📍 `2220-2237`.

---

# Nhóm D — Reclaim từng page

## FUNCTION: `shrink_page_list` — *cây quyết định số phận page*  ★ CRITICAL

**1. Mục đích.** Với một lô page đã isolate: **free** những page free được an toàn, **giữ/activate** phần còn lại, **ghi** khi buộc.
**4. Flow.** Xem Flow 5 (file 03 mục 11.1).
**5. Ý nghĩa action (nhóm).**
| Nhóm | Tại sao | Nếu bỏ |
|---|---|---|
| `trylock_page` (không `lock_page`) | **Không bao giờ ngủ chờ lock** trong reclaim | Deadlock với người đang giữ lock page và xin RAM |
| Stat dirty/writeback/congested | Đo triệu chứng cho `shrink_node` | Không có cơ sở throttle |
| Xử lý writeback (3 case) | Tránh chờ I/O vô hạn (đĩa chết); chờ chỉ ở legacy memcg | Stall vô hạn / OOM oan |
| `page_check_references` | Bảo vệ page đang dùng | Xoá cả working set |
| swap-out anon | Anon cần chỗ cất | Mất dữ liệu / không reclaim được anon |
| `try_to_unmap` | Gỡ PTE trước khi free | Process truy cập page đã cấp cho người khác |
| Dirty: skip/ghi | Ghi từ reclaim đắt, nguy hiểm | Tràn stack / I/O lẻ |
| `__remove_mapping` | Nút "không đường lui", kiểm tra ref & dirty | Free page đang bị pin/dirty |
| `free_unref_page_list` gom cuối | Một lần free cho cả lô | Tốn atomic từng page |
**6. Kết quả.** `nr_reclaimed`; `stat` (nr_dirty, nr_unqueued_dirty, nr_congested, nr_writeback, nr_immediate, nr_activate[2], nr_ref_keep, nr_unmap_fail, nr_pageout, nr_lazyfree_fail); danh sách còn lại (keep/activate) trả về `page_list`.
**7. Ảnh hưởng.** page flags (`PG_reclaim`, `PG_active`, `PG_referenced`), swap cache, page cache, memcg charge, TLB.
**8. Edge.** **VENDOR** `warp_canceled` ⇒ `break` giữa lô (page chưa xử lý trả lại LRU). `VM_BUG_ON_PAGE(PageActive)` (compile-out trên board).
🔎 `pgactivate`, `pglazyfreed`, `nr_vmscan_immediate_reclaim`, `thp_swpout_fallback`, tracepoint `lru_shrink_inactive` (nr_dirty, nr_writeback, nr_ref_keep, nr_unmap_fail…). 📍 `1087-1514`, vendor `1110-1113`.

## FUNCTION: `page_check_references` — xem bảng ở file 03 mục 11.2. ★ DECISION. 📍 `1001-1052`.

## FUNCTION: `page_check_dirty_writeback` — *page này có "bẩn thật" không?*

**1.** Cho biết dirty/writeback để đo triệu chứng. **5.** Anon (kể cả lazyfree) cố ý báo *không dirty/không writeback* vì không có flusher và phải ghi từ reclaim; với page có `private`, hỏi filesystem (`->is_dirty_writeback`) vì cờ page có thể không chính xác. 📍 `1055-1082`.

## FUNCTION: `pageout` — *thật sự ghi một page*  ★ CRITICAL

**1.** Ghi dirty page ra backing store **không chặn**. Điều kiện/kết cục: xem file 03 mục 12.4. **Kết quả:** `PAGE_KEEP / ACTIVATE / SUCCESS / CLEAN`. **Edge:** lỗi ghi đồng bộ ⇒ `handle_write_error` đánh dấu `mapping_set_error` để `fsync()` sau báo lỗi. 🔎 `pr_info("pageout: orphaned page")`, `nr_vmscan_write`, `trace_mm_vmscan_writepage`. 📍 `795-863, 770-777`.

## FUNCTION: `__remove_mapping` — xem file 03 mục 11.3. ★ CRITICAL. 📍 `869-957`.

---

# Nhóm E — Slab

## FUNCTION: `shrink_slab` / `do_shrink_slab` — *thu hồi cache không qua LRU*

**1. Mục đích.** Dentry/inode/… không nằm trên LRU. Mỗi subsystem đăng ký **shrinker** (đếm + thu). `vmscan.c` điều phối "bao nhiêu" theo `priority`.
**3. Input.** gfp; node; memcg; priority.
**4. Flow.** `freeable = count_objects()` → `delta = (freeable>>prio)*4/seeks` (hoặc `freeable/2` nếu `seeks==0`) → cộng `nr_deferred` → chặn `[≤ freeable/2 nếu delta nhỏ ; ≤ 2×freeable]` → vòng `scan_objects` theo batch (128) → trả phần chưa làm vào `nr_deferred`.
**5. Ý nghĩa.**
| Action | Tại sao |
|---|---|
| `nr_deferred` | Nhớ "nợ" quét khi áp lực nhẹ; trả khi áp lực lớn |
| Batch 128 | Tránh gọi shrinker quá lắt nhắt |
| `rwsem_is_contended` ⇒ break | Không để `register_shrinker` chết đói |
| `down_read_trylock` | Shrinker có thể gọi lại reclaim ⇒ phải *trylock* (tránh deadlock) |
**6. Kết quả.** Số object đã free (→ `reclaimed_slab` → `nr_reclaimed`).
**8. Edge.** `SHRINK_STOP` ⇒ dừng; `SHRINK_EMPTY` ⇒ memcg shrinker map xoá bit (có một lần gọi lại để chống race `list_lru_add`).
🔎 **`pr_err("shrink_slab: %pS negative objects to delete nr=%ld")`**, `slabs_scanned`, `trace_mm_shrink_slab_start/end`. 📍 `429-708`.

---

# Nhóm F — Chế độ đặc biệt

## FUNCTION: `shrink_all_memory` — *hibernate / PM_WARP*

**1.** "Cho tôi *càng nhiều càng tốt*" — dùng khi tạo image hibernate. `sc`: `hibernation_mode=1`, `may_writepage/unmap/swap = 1`, mọi zone, `GFP_HIGHUSER_MOVABLE`. Chạy `do_try_to_free_pages` (**không** throttle `wait_iff_congested` vì `hibernation_mode`). **Board:** `warp_shrink_memory()` gọi nó **lặp** (tới `repeat` lần, đến 10 000 nếu swapout bật) in `"Shrinking memory...  done (%d pages freed)"`. **VENDOR** hunk V2/V3 cho phép hủy khi `warp_canceled`. 📍 `4037-4064`; `kernel/power/warp.c:1734-1768`.

## FUNCTION: `reclaim_pages` / `reclaim_clean_pages_from_list` / `isolate_lru_page` / `putback_lru_page` / `check_move_unevictable_pages` — *mượn cơ chế LRU cho người khác*

| Hàm | Cho ai | Ghi chú |
|---|---|---|
| `reclaim_pages` | `MADV_PAGEOUT` | Gom theo node → `shrink_page_list` (may_writepage/unmap/swap=1, priority 12, **ignore_references=false**) → trả page chưa free về LRU. Tác vụ **chủ động** duy nhất ở 5.10 |
| `reclaim_clean_pages_from_list` | `alloc_contig_range` (CMA) | `ignore_references=true`, chỉ page file sạch; **board CMA off** |
| `isolate_lru_page` | migration, mlock, compaction | Tách 1 page khỏi LRU (cần refcount sẵn) |
| `putback_lru_page` | như trên | Đưa lại qua `lru_cache_add` |
| `check_move_unevictable_pages` | shmem unlock, driver | Cứu page khỏi list unevictable (vd. shm hết `SHM_LOCK`) |
📍 `2124-2176, 1516-1553, 1783-1806, 988-992, 4304-4350`.

## FUNCTION: `zone_reclaimable_pages` — *"còn bao nhiêu có thể reclaim?"*

**1.** Ước lượng cho `should_reclaim_retry` & compaction: `inactive_file + active_file (+ anon nếu có swap)`; nếu **bằng 0** (upstream stable) thì dùng `NR_FREE_PAGES` để không loại zone như DMA32 khỏi cuộc chơi. **Lưu ý:** không tính page đang isolate. 📍 `304-322`.

---

# Nhóm G — hàm nhỏ còn lại

| Hàm | Làm gì | 📍 |
|---|---|---|
| `set_task_reclaim_state` | Gắn `reclaim_state` vào task để slab free được đếm. **`WARN_ON_ONCE`** nếu ghi đè / xoá khi đã null (reclaim lồng nhau) | 182-192 |
| `cgroup_reclaim`, `writeback_throttling_sane` | "Đây là reclaim memcg?" / "cơ chế dirty-throttle chuẩn có hoạt động không?" (false với cgroup v1 legacy) | 250-297 |
| `lruvec_lru_size` | Số page của một list **giới hạn theo zone ≤ idx** (dùng `mem_cgroup_get_zone_lru_size`) | 330-347 |
| `snapshot_refaults` | Chốt `WORKINGSET_ACTIVATE_*` vào `lruvec->refaults[]` để lần sau `shrink_node` biết "có refault mới không" | 3011-3021 |
| `age_active_anon` | kswapd xoay anon active nền | 3389-3409 |
| `pgdat_watermark_boosted` | Node có zone đang boost? | 3411-3433 |
| `clear_pgdat_congested` | Xoá `LRUVEC_CONGESTED/PGDAT_DIRTY/PGDAT_WRITEBACK` khi balanced | 3472-3479 |
| `compaction_ready`, `in_reclaim_compaction` | Có nên nhường cho compaction (order cao) | 2564-2572, 2888-2916 |
| `current_may_throttle` | kthread `PF_LOCAL_THROTTLE` (vd. nfsd) chỉ throttle khi BDI nó ghi bị congested | 1925-1930 |
| `is_page_cache_freeable`, `may_write_to_inode`, `handle_write_error` | Điều kiện/biến chứng của `pageout` | 736-777 |
| `drop_slab(_node)` | `drop_caches=2/3`: `shrink_slab(prio 0)` lặp tới khi `freed ≤ 10` | 710-734 |
| `register_shrinker` & họ | Đăng ký shrinker; memcg-aware dùng IDR + bitmap | 352-425 |
| `kswapd_run/stop/init` | Tạo/dừng thread; **`pr_err("Failed to start kswapd on node %d")`** | 4071-4112 |

---

## 14.99. Bảng tra nhanh: *"Nếu hàm này không chạy thì sao?"*

| Hàm | Hệ quả |
|---|---|
| `wakeup_kswapd` | Không ai reclaim nền ⇒ mọi allocation vượt MIN đều tự direct reclaim ⇒ latency tăng; atomic allocation hết đường |
| `balance_pgdat` | kswapd không làm gì |
| `shrink_page_list` | Có scan mà không free ⇒ treo/OOM |
| `shrink_active_list` | inactive cạn ⇒ không có ứng viên "chín" ⇒ reclaim kém hiệu quả / thrash |
| `get_scan_count` | Không biết quét gì |
| `throttle_direct_reclaim` | Direct reclaimer tranh reserve ⇒ có thể kẹt (đặc biệt swap qua mạng) |
| `too_many_isolated` | Bão direct reclaim làm LRU "trống giả" ⇒ swap thrash / OOM sớm |
| `__remove_mapping` | Không free được page cache/swap cache |
| `shrink_slab` | dentry/inode không bao giờ bị thu ⇒ RAM bị slab chiếm |
| `should_continue_reclaim` | high-order allocation không bao giờ gom đủ free cho compaction |

Tiếp theo → [`05_debugging_logs_scenarios.md`](05_debugging_logs_scenarios.md).
