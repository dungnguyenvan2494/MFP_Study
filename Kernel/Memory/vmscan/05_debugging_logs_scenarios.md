# 05 — Debugging: log, counter, tracepoint, scenario

> Phủ các mục: **15** Debugging · **16** Log → Function → Flow mapping · **17** Debug Scenarios · Flow 8 · Sequence diagram "Debug".
> Đây là file quan trọng nhất cho việc **debug ngoài thực tế**. Mọi `file:line` là source local 5.10.241.

---

# 15. Debugging

## 15.0. Sự thật cần biết trước: `vmscan.c` **gần như im lặng**

Tôi đã `grep` toàn bộ file cho `pr_*`, `printk`, `WARN*`, `BUG*`, `VM_BUG*`, `trace_*`, `count_vm_event*`. Kết luận:

| Loại | Số lượng | Ghi chú |
|---|---|---|
| Câu **log hiển thị cho người dùng** (`pr_err/pr_info`) | **3** | Đều là tình huống *hiếm / bất thường*, không phải "reclaim đang vất vả" |
| `WARN_*` | 5 vị trí | Đều là **lỗi lập trình/cấu trúc**, không phải áp lực bộ nhớ |
| `BUG*` | 6 vị trí (+ `BUILD_BUG_ON`) | Hỏng bất biến |
| `VM_BUG_ON_PAGE` | 7 vị trí | **Compile-out** trên board (`# CONFIG_DEBUG_VM is not set`) |
| `trace_*` (tracepoint) | ~16 điểm | Nhiều khả năng **không tồn tại** trên board (`FTRACE` off) |
| `count_vm_event(s)`/`mod_node_page_state` | ~30 điểm | **Đây mới là nguồn thông tin chính** (`/proc/vmstat`) |

> ⇒ **Bài học:** "Reclaim đang kẹt" **không** sinh ra log trong `vmscan.c`. Bạn phát hiện nó bằng **counter**, **stack của task**, và các log ở *tầng allocator/OOM* (`page allocation failure`, `invoked oom-killer`).

## 15.1. Bạn thực sự quan sát được gì trên board này (ma trận khả năng)

| Công cụ | Có? | Bằng chứng | Hệ quả |
|---|---|---|---|
| `dmesg`/printk | ✅ | `PRINTK=y`, `LOG_BUF_SHIFT=19`, `PRINTK_TIME=y`, `CONSOLE_LOGLEVEL_DEFAULT=7` `[Eagle_defconfig:196,130,4667,4669]` | Log kernel có timestamp |
| `/proc/vmstat`, `/proc/meminfo`, `/proc/zoneinfo`, `/proc/buddyinfo`, `/proc/pagetypeinfo` | ✅ | `PROC_FS=y`, `VM_EVENT_COUNTERS=y` `[233,4232]` | Nguồn chính |
| `/proc/PID/stack` | ✅ (cần root) | `CONFIG_STACKTRACE=y` `[4792]` | Xem task đang kẹt ở đâu |
| `/proc/PID/wchan` | ✅ | `KALLSYMS=y` `[213]` | Hàm đang ngủ |
| `/proc/PID/smaps`, `pagemap` | ✅ | `PROC_PAGE_MONITOR=y` `[4235]` | Ai chiếm RAM |
| `debugfs` | ✅ | `DEBUG_FS=y` `[4700]` | `/sys/kernel/debug/…` (mount nếu cần) |
| **Tracepoint/ftrace** (`trace_mm_vmscan_*`) | ❌ (nhiều khả năng) | `# CONFIG_FTRACE is not set`, không có `TRACING`/`TRACEPOINTS` ở **mọi** defconfig `[grep]` | Kiểm tra: `ls /sys/kernel/debug/tracing/events/vmscan` |
| **PSI** (`/proc/pressure/memory`) | ❌ | `# CONFIG_PSI is not set` `[102]` | Không có chỉ số "stall time" |
| **Hung-task / soft-lockup detector** | ❌ | `# CONFIG_DETECT_HUNG_TASK`, `# CONFIG_SOFTLOCKUP_DETECTOR` `[4756-4757]` | Task kẹt reclaim **không tự báo** |
| **SysRq** (`echo w > /proc/sysrq-trigger`) | ❌ (15/15 defconfig board) | `# CONFIG_MAGIC_SYSRQ is not set` `[4699]` | Không dump được task D-state hàng loạt |
| `perf` | ❌ | `# CONFIG_PERF_EVENTS is not set` `[230]` | Không profile được CPU của kswapd |
| Delay accounting (`freepages delay`) | ❌ | `# CONFIG_TASKSTATS is not set` `[101]` | Không đọc được "thời gian chờ reclaim" |
| `dynamic_debug` (`pr_debug`) | ❌ | `# CONFIG_DYNAMIC_DEBUG is not set` `[4673]` | `vmpressure_calc_level` `pr_debug` bị compile-out |
| `DEBUG_VM` | ❌ | `# CONFIG_DEBUG_VM is not set` `[4734]` | `VM_BUG_ON_PAGE` vô hiệu |
| memcg stat | ✅ (nếu mount cgroup) | `MEMCG=y` `[145]` | `memory.stat` có `pgscan/pgsteal/pgrefill…` theo memcg |

> **Khuyến nghị cho bản debug (đề xuất, chưa áp dụng):** nếu được phép build riêng, bật `FTRACE/EVENT_TRACING`, `PSI`, `DETECT_HUNG_TASK`, `MAGIC_SYSRQ`, `TASKSTATS`+`TASK_DELAY_ACCT`. Đây là 5 thứ biến "mù" thành "thấy".

## 15.2. **Toàn bộ** log/warning/bug trong `vmscan.c` (inventory đầy đủ)

> `pr_fmt` của file là `KBUILD_MODNAME ": " fmt` `[vmscan.c:15]`. Vì `vmscan.o` built-in nên `KBUILD_MODNAME = "vmscan"` ⇒ mọi `pr_*` có tiền tố **`vmscan: `**.

| # | Vị trí | Cấp | Chuỗi (sau khi ghép `pr_fmt`) |
|---|---|---|---|
| L1 | `vmscan.c:473` (`do_shrink_slab`) | `pr_err` | `vmscan: shrink_slab: <symbol scan_objects> negative objects to delete nr=<n>` |
| L2 | `vmscan.c:823` (`pageout`) | `pr_info` | `vmscan: pageout: orphaned page` |
| L3 | `vmscan.c:4083` (`kswapd_run`) | `pr_err` | `vmscan: Failed to start kswapd on node <nid>` |
| W1 | `vmscan.c:186` | `WARN_ON_ONCE` | `rs && task->reclaim_state` (ghi đè reclaim_state) |
| W2 | `vmscan.c:189` | `WARN_ON_ONCE` | `!rs && !task->reclaim_state` (xoá khi đã null) |
| W3 | `vmscan.c:3326` (`mem_cgroup_shrink_node`) | `WARN_ON_ONCE` | `!current->reclaim_state` |
| W4 | `vmscan.c:1788` (`isolate_lru_page`) | `WARN_RATELIMIT` | `"trying to isolate tail page"` |
| B1 | `vmscan.c:243` | `BUG_ON` | `unregister_memcg_shrinker: id < 0` |
| B2 | `vmscan.c:876-877` (`__remove_mapping`) | `BUG_ON` | page không locked / mapping không khớp |
| B3 | `vmscan.c:1727` (`isolate_lru_pages`) | `BUG()` | `__isolate_lru_page` trả mã lạ |
| B4 | `vmscan.c:2444` (`get_scan_count`) | `BUG()` | `scan_balance` ngoài enum |
| B5 | `vmscan.c:4082` (`kswapd_run`) | `BUG_ON` | tạo kswapd thất bại **lúc boot** (`system_state < SYSTEM_RUNNING`) |
| B6 | `vmscan.c:3285-3287` | `BUILD_BUG_ON` | `order/priority/reclaim_idx` phải vừa `s8` (compile-time) |
| V* | `1121,1490,1501,1692,1787,1876,4337` | `VM_BUG_ON_PAGE` | **compile-out trên board** |

## 15.3. Counter `/proc/vmstat` do `vmscan.c` cập nhật (nguồn debug chính)

| Counter | Tăng ở đâu (`vmscan.c`) | Nghĩa chính xác | Nghi ngờ gì khi bất thường |
|---|---|---|---|
| `pageoutrun` | 3594 (`balance_pgdat`) | Số lần kswapd bắt đầu một đợt cân bằng | Tăng nhanh liên tục ⇒ kswapd luôn bận |
| `pgscan_kswapd` / `pgscan_direct` | 1971-1973 | Số page **đã quét** (gồm cả page bị skip do zone) bởi kswapd / direct. *Chỉ đếm global reclaim* (`!cgroup_reclaim`) | direct tăng ⇒ task tự reclaim |
| `pgsteal_kswapd` / `pgsteal_direct` | 1990-1992 | Số page **đã giải phóng** | `steal/scan` thấp ⇒ reclaim kém hiệu quả |
| `pgscan_anon` / `pgscan_file` | 1975 | Quét theo loại LRU | `anon = 0` ⇒ không swap |
| `pgsteal_anon` / `pgsteal_file` | 1994 | Thu theo loại | |
| `pgscan_skip_<zone>` | 1746 | Page bị bỏ qua vì zone > `reclaim_idx` | Cao ⇒ LRU đầy page ở zone không giúp được |
| `pgrefill` | 2056 | Số page active được xem bởi `shrink_active_list` | |
| `pgdeactivate` | 2112 | Số page hạ cấp active→inactive | |
| `pgactivate` | 1511 | Số page từ reclaim bị **đưa lên active** (gồm: referenced, *dirty file bị đánh dấu*, unmap fail, swap fail, writeback…) | Cao so với `pgsteal` ⇒ page bị "cứu" thay vì free |
| `pglazyfreed` | 1452 | Anon lazyfree bị vứt | |
| `allocstall_<zone>` | 3050 | Mỗi lần **vào `do_try_to_free_pages`** (global); cộng lại ở `retry:` | `> 0` ⇒ có task bị direct reclaim |
| `pgscan_direct_throttle` | 3236 | Số lần task bị throttle chờ kswapd | `> 0` ⇒ reserve PFMEMALLOC cạn |
| `kswapd_low_wmark_hit_quickly` | 3874 | kswapd bị đánh thức giữa nap | Áp lực kéo dài |
| `kswapd_high_wmark_hit_quickly` | 3876 | kswapd hết nap mà đã mất cân bằng | Áp lực kéo dài |
| `slabs_scanned` | 533 | Object slab đã quét | |
| `nr_vmscan_write` | 858 | Số page reclaim **tự ghi** | Cao ⇒ phụ thuộc ghi từ reclaim (đắt) |
| `nr_vmscan_immediate_reclaim` | 1355 | Dirty file page bị "đánh dấu để reclaim ngay khi ghi xong" | Cao ⇒ **dirty ở đuôi LRU** (flusher chậm) |
| `nr_isolated_anon/file` | 1970, 1988, 2053, 2115 | Page đang bị "rút" khỏi LRU | Cao ⇒ nhiều reclaimer song song |
| `unevictable_pgs_scanned/rescued` | 4346-4347 | `check_move_unevictable_pages` | |
| `thp_swpout_fallback` | 1287 | THP phải swap từng page nhỏ | THP không swap được nguyên |
| `zone_reclaim_failed` | 4289 | `node_reclaim` thất bại | **NUMA only — không có trên board** |
| *(ngoài vmscan.c)* `workingset_refault_*/activate_*`, `pswpin/pswpout`, `pgmajfault`, `compact_stall/fail`, `oom_kill`, `drop_pagecache/drop_slab` | workingset.c, page_io.c, compaction.c, oom_kill.c, drop_caches.c | Triệu chứng đi kèm | |

> **Tên counter theo zone** (`allocstall_*`, `pgscan_skip_*`) sinh ra từ macro `TEXTS_FOR_ZONES` ⇒ board có `_dma`, `_dma32`, `_normal` `[GENERAL]`.

## 15.4. Tracepoint (chỉ dùng được nếu build có `TRACEPOINTS`)

| Event | Trường | Cho biết gì | Điểm phát |
|---|---|---|---|
| `mm_vmscan_wakeup_kswapd` | nid, order, gfp_flags | **Ai/vì sao** đánh thức kswapd | `wakeup_kswapd` 4023 |
| `mm_vmscan_kswapd_wake` | nid, order | kswapd bắt đầu làm việc | `kswapd` 3963 |
| `mm_vmscan_kswapd_sleep` | nid | kswapd ngủ dài | 3856 |
| `mm_vmscan_direct_reclaim_begin/end` | order, gfp_flags / nr_reclaimed | **Thời gian một lần direct reclaim** (end−begin) và kết quả | 3298, 3302 |
| `mm_vmscan_memcg_reclaim_begin/end`, `…softlimit…` | như trên | reclaim memcg | 3331-3343, 3376-3382 |
| `mm_vmscan_lru_isolate` | classzone, order, nr_requested, **nr_scanned, nr_skipped, nr_taken**, lru | Mỗi lần "rút" page: skip bao nhiêu, lấy bao nhiêu | 1751 |
| `mm_vmscan_lru_shrink_inactive` | nr_scanned, nr_reclaimed, **nr_dirty, nr_writeback, nr_congested, nr_immediate, nr_activate_anon/file, nr_ref_keep, nr_unmap_fail**, priority, flags | **Bản "khám bệnh" tốt nhất**: vì sao page không free | 2024 |
| `mm_vmscan_lru_shrink_active` | nr_taken, nr_active, nr_deactivated, nr_referenced, priority | Hạ cấp active | 2120 |
| `mm_vmscan_writepage` | pfn, flags | reclaim đang tự ghi page nào | 857 |
| `mm_shrink_slab_start/end` | shrinker, nr_to_scan, delta, total_scan, freed | Hành vi shrinker | 503, 555 |
| `mm_vmscan_node_reclaim_begin/end` | | **NUMA only** | 4214, 4243 |
| `mm_vmscan_inactive_list_is_low` | (khai báo trong header) | **Không còn được phát ở 5.10** (không có `trace_mm_vmscan_inactive_list_is_low` trong `vmscan.c`) | — |
| *(page_alloc)* `reclaim_retry_zone` | order, reclaimable, available, min_wmark, no_progress_loops, wmark_check | Tại sao allocator còn retry/OOM | `page_alloc.c:4614` |

**Cách đọc `lru_shrink_inactive`** (nếu có): `nr_scanned=32 nr_reclaimed=2 nr_dirty=30 nr_immediate=30` ⇒ đuôi LRU toàn dirty. `nr_ref_keep` lớn ⇒ page đang hot. `nr_unmap_fail` lớn ⇒ page mapped không gỡ được PTE. `priority` thấp (≤4) với tỉ lệ thấp ⇒ khó thở.

## 15.5. Debug mindset — quy trình 9 bước

```
Triệu chứng quan sát được
        ↓
① Hệ thống có đang chịu memory pressure không?
        ↓
② kswapd có hoạt động không?
        ↓
③ Direct reclaim có xảy ra không?
        ↓
④ Quét BAO NHIÊU?
        ↓
⑤ Thu về BAO NHIÊU?
        ↓
⑥ Loại page nào (anon/file/tmpfs/slab)?
        ↓
⑦ VÌ SAO page không reclaim được?
        ↓
⑧ Thời gian đang trôi ở ĐÂU?
        ↓
⑨ Nguyên nhân gốc
```

> **Công cụ đi kèm:** `tools/vmstat_delta.sh` (đo delta 2 mẫu `/proc/vmstat`, tính hiệu suất, in chẩn đoán nhanh). Bạn có thể chạy trên board: `sh vmstat_delta.sh 10`.

### ① Có áp lực bộ nhớ không?

**Hỏi:** free có dưới watermark không? Có allocation nào phải chờ?
**Xem:**
- `/proc/meminfo`: `MemFree`, `MemAvailable`, `Active/Inactive(anon/file)`, `Dirty`, `Writeback`, `Shmem`, `Slab`, `SReclaimable/SUnreclaim`, `Unevictable/Mlocked`, `SwapTotal/SwapFree`.
- `/proc/zoneinfo`: từng zone: `pages free`, `min`, `low`, `high`, `protection:` — **so sánh `free` với `low`/`min`** (lưu ý kernel dùng `free − lowmem_reserve` cho allocation của zone thấp).
- `/proc/buddyinfo`: nếu order cao = 0 nhưng free lớn ⇒ **phân mảnh** (có thể là *boost* chứ không phải thiếu RAM).
**Đọc:** `MemAvailable` thấp & `free < low` ⇒ pressure thật. `free` lớn mà kswapd chạy ⇒ nghi **watermark boost** (file 02, 7.7).
**Chú ý board:** `CONFIG_CMEMDRV=y` `[Eagle_defconfig:4118]` và `arm64_memblock_init()` gọi `thr_cmemdrvr_reserve_memory()` `[SOURCE arch/arm64/mm/init.c:327-340]` — từ tên hàm, đây là vùng **reserve cho driver CMEM** nên nhiều khả năng bị loại khỏi RAM mà buddy/reclaim quản lý ⇒ `MemTotal` nhỏ hơn RAM vật lý `[INFERENCE — không đọc thân hàm]`.

### ② kswapd có hoạt động không?

**Xem:** `pageoutrun` (delta), `pgscan_kswapd`/`pgsteal_kswapd` (delta), CPU của `kswapd0` (`/proc/PID/stat` trường 14+15 — script đã làm), trạng thái `R/S/D`.
**Đọc:**
| Quan sát | Nghĩa |
|---|---|
| `pageoutrun` ≈ 0, free thấp, không có direct | hệ thống ổn định ở ngưỡng, hoặc kswapd *hopeless* (xem ⑦) |
| `pageoutrun` cao, `pgsteal_kswapd/pgscan_kswapd` cao | kswapd đang làm việc *hiệu quả* — khoẻ |
| `pageoutrun` cao, hiệu suất thấp, CPU cao | **kswapd quay vô ích** (Scenario 3/6) |
| `kswapd_*_wmark_hit_quickly` tăng | áp lực kéo dài |

### ③ Direct reclaim có xảy ra không?

**Xem:** `allocstall_*`, `pgscan_direct`, `pgsteal_direct`, `pgscan_direct_throttle`.
**Đọc:** `allocstall_* > 0` ⇒ có task đã tự reclaim (⇒ chậm). `pgscan_direct_throttle > 0` ⇒ có task đã ngủ chờ kswapd (reserve cạn, xem 8.5). Nhớ `allocstall` đếm cả mỗi vòng retry.

### ④ Quét bao nhiêu? ⑤ Thu bao nhiêu?

**Hiệu suất = Δpgsteal / Δpgscan** (cho kswapd và direct riêng).
| Hiệu suất | Chẩn đoán sơ bộ `[GENERAL — heuristic, không phải ngưỡng của kernel]` |
|---|---|
| ≥ 50% | Reclaim khoẻ (cache sạch dồi dào) |
| 10–50% | Có trở ngại (referenced/dirty); theo dõi |
| < 10% | **Không hiệu quả** → bước ⑦ |
| scan ≈ 0 nhưng free thấp | Reclaim *không chạy* hoặc bị chặn (throttle/hopeless/`!may_enter_fs`) |

Kernel tự đo giá trị tương tự để quyết định `vmpressure` (60% = medium, 95% = critical).

### ⑥ Loại page nào?

`pgscan_anon` vs `pgscan_file`; `Shmem` (tmpfs là anon-LRU); `SReclaimable` + `slabs_scanned` (slab). **Không có swap** ⇒ `pgscan_anon = 0`, và `Active/Inactive(anon)` + `Shmem` là *vùng không thể reclaim* ⇒ chỉ page cache còn lại gánh toàn bộ.

### ⑦ Vì sao page không reclaim được?

Duyệt cây (đối chiếu counter):
```
Quét nhiều, thu ít
   ├─ Page đang NÓNG (referenced)?        → pgactivate cao, workingset_refault_file cao  (thrash)
   ├─ Page DIRTY/WRITEBACK?               → nr_vmscan_immediate_reclaim ↑, meminfo Dirty/Writeback cao
   ├─ ANON mà không có swap?              → pgscan_anon=0, SwapTotal=0, Shmem/anon chiếm RAM
   ├─ Zone sai (skip)?                    → pgscan_skip_* cao
   ├─ PIN / refcount thừa / unmap fail?   → (tracepoint nr_unmap_fail), khó thấy bằng counter
   ├─ THP không split?                    → thp_swpout_fallback
   ├─ Quá nhiều reclaimer song song?      → nr_isolated_* cao; task ngủ msleep(100)
   └─ GFP_NOFS/NOIO (không vào FS)?       → thu ít page có buffer/FS; log Mem-Info khi alloc fail
```

### ⑧ Thời gian trôi ở đâu?

Không có `perf`/PSI/delayacct ⇒ dùng **lấy mẫu stack**:
```sh
for i in 1 2 3 4 5 6 7 8 9 10; do cat /proc/<PID>/stack | head -8; echo ---; sleep 0.2; done
cat /proc/<PID>/wchan; echo
grep State /proc/<PID>/status
```
**Bảng "chữ ký stack → ý nghĩa"** (đều có thật trong source):

| Stack / trạng thái | Nghĩa | Gốc ở |
|---|---|---|
| `msleep ← shrink_inactive_list ← … ← try_to_free_pages` | `too_many_isolated` — quá nhiều reclaimer | `vmscan.c:1950-1961` |
| `congestion_wait` / `wait_iff_congested ← shrink_node` | Dirty/writeback congested | `vmscan.c:2842, 2867` |
| `wait_event_killable ← throttle_direct_reclaim` | Chờ kswapd (reserve cạn) | `vmscan.c:3254` |
| `…_timeout ← throttle_direct_reclaim` (≤1s) | Như trên nhưng caller `!__GFP_FS` | `vmscan.c:3247` |
| `wait_on_page_writeback ← shrink_page_list` | Legacy memcg chờ I/O | `vmscan.c:1233` |
| `congestion_wait ← should_reclaim_retry ← __alloc_pages_slowpath` | Allocator chờ I/O (nhiều dirty, không tiến triển) | `page_alloc.c:4630` |
| **R** (running) trong `page_referenced / rmap_walk / isolate_lru_pages / shrink_page_list` | **CPU-bound scanning** | |
| Tranh `pgdat->lru_lock` (spin) | Nhiều CPU cùng reclaim/`lru_add_drain` | `vmscan.c:1794,1965…` |
| `schedule ← kswapd_try_to_sleep` | kswapd đang **ngủ** (bình thường) | `vmscan.c:3869` |

### ⑨ Nguyên nhân gốc

Gộp ⑥+⑦: *ai chiếm RAM không thể trả* (anon/tmpfs/driver/slab không reclaim được) + *tại sao allocation rate > reclaim rate*. Xem Scenario 3 và 6.

## 15.6. Flow 8 — Debug / log flow

```
 Điều kiện (condition)
      │      vd: total_scan < 0 | dirty page mồ côi | reclaim_state lồng nhau | alloc cuối cùng fail
      ▼
 Hàm (function)         do_shrink_slab | pageout | set_task_reclaim_state | __alloc_pages_slowpath
      ▼
 Quyết định (decision)  "không thể tiếp tục an toàn" | "bỏ qua" | "bỏ cuộc"
      ▼
 Log                    pr_err / pr_info / WARN / warn_alloc
      ▼
 Triệu chứng có thể có  rò hụt slab | I/O lỗ hổng journaling | nested reclaim | process crash/alloc fail
      ▼
 Hành động debug        xem shrinker (%pS) | kiểm tra FS data=journal | xem call stack | xem Mem-Info + vmstat delta
```

## 15.7. Sequence diagram — Debug

```
 Memory      Reclaim        Điều kiện        Hàm              Kernel Log        Developer       Điều tra
 Pressure    (vmscan)       (condition)      (function)
    │           │               │               │                 │                │               │
    │──free<LOW─►kswapd/direct  │               │                 │                │               │
    │           │──shrink_node─►│               │                 │                │               │
    │           │               │ (A) counter tăng (im lặng)      │                │               │
    │           │               │───────────────────────────────────────────────►  │ vmstat_delta  │
    │           │               │ (B) alloc cuối fail             │                │──────────────►│
    │           │               │──────────────►__alloc_pages_slowpath               │ Mem-Info,     │
    │           │               │               │─warn_alloc─────►"page allocation failure"  vmstat │
    │           │               │ (C) reclaim 0 tiến triển, OOM   │                │               │
    │           │               │──────────────►__alloc_pages_may_oom               │               │
    │           │               │               │─oom_kill───────►"invoked oom-killer"+"Killed process"
    │           │               │ (D) bất biến vỡ                 │                │               │
    │           │               │──────────────►set_task_reclaim_state/do_shrink_slab               │
    │           │               │               │─WARN/pr_err────►"WARNING … vmscan.c:186" / "negative objects"
    │           │               │               │                 │───đọc log─────►│ addr2line,    │
    │           │               │               │                 │                │ xem stack ───►│
```

---

# 16. Log → Function → Flow mapping

## 16.1. Phân tích từng log theo mô hình `LOG → Function → Caller → Flow stage → Condition → Meaning → Root cause`

### L1 — `vmscan: shrink_slab: <sym> negative objects to delete nr=<n>`

```
LOG ─ do_shrink_slab() ─ Caller: shrink_slab() / shrink_slab_memcg() ─ Stage: Slab reclaim (Stage 3-6, song song LRU)
```
- **Khi nào:** `total_scan = nr_deferred + delta < 0` (kiểu `long`) `[473]`. `delta = (freeable >> priority) × 4 / seeks`. Chỉ xảy ra nếu `nr_deferred` bị tràn/âm **hoặc** `count_objects()` trả giá trị bất hợp lý (âm khi xem như `long`).
- **Kernel làm gì tiếp:** đặt `total_scan = freeable`, giữ `next_deferred = nr` `[474-476]` — tức *tự cứu*, không crash.
- **Tại sao cần log:** Kernel muốn báo cho dev: *"shrinker này đang báo số object sai / nợ đã hỏng"* — một **bug đếm** ở subsystem sở hữu shrinker, không phải bug của reclaim. `%pS` in luôn *tên hàm `scan_objects`* ⇒ biết ngay subsystem (vd. `super_cache_scan` ⇒ dentry/inode cache; driver riêng ⇒ driver đó).
- **Nghi ngờ gì:** (1) shrinker của driver/FS custom đếm `count_objects` sai (underflow `list_lru`); (2) race giữa `count_objects`/`scan_objects`; (3) lỗi bộ nhớ làm hỏng `shrinker->nr_deferred` `[INFERENCE]`.
- **Kiểm tra:** `addr2line -e vmlinux <sym>`; đọc code `count_objects`; xem `slabs_scanned`, `SReclaimable`.

### L2 — `vmscan: pageout: orphaned page`

```
LOG ─ pageout() ─ Caller: shrink_page_list() (nhánh PageDirty) ─ Stage: ghi dirty page (Flow 6)
```
- **Khi nào:** `pageout` gọi cho page *dirty* thoả `is_page_cache_freeable`, **`mapping == NULL`**, `page_has_private`, và `try_to_free_buffers()` thành công ⇒ `ClearPageDirty` ⇒ `PAGE_CLEAN` `[813-826]`.
- **Ý nghĩa:** page đã bị *truncate khỏi file* (mapping NULL) nhưng còn buffer + cờ dirty ("*data journaling orphaned pages*" theo comment `[816-819]`). Reclaim dọn nó đi.
- **Tại sao log:** báo hiệu một **trạng thái hiếm** của FS (journal data mode) — để dev biết có page mồ côi bị dọn.
- **Nghi ngờ:** FS dùng data journaling (ext3/ext4 `data=journal`), truncate race. **Không ảnh hưởng đến memory pressure.** Nếu xuất hiện dồn dập ⇒ xem FS/driver đang truncate dưới tải. `[FS của board: KIỂM TRA TRÊN BOARD]`.

### L3 — `vmscan: Failed to start kswapd on node <nid>` (+ B5)

```
LOG ─ kswapd_run() ─ Caller: kswapd_init() (boot) / memory_hotplug ─ Stage: khởi tạo
```
- **Khi nào:** `kthread_run` trả lỗi. **Lúc boot** (`system_state < SYSTEM_RUNNING`) ⇒ `BUG_ON` ngay `[4082]` (panic). Sau boot (hotplug) ⇒ chỉ in `pr_err`.
- **Ý nghĩa:** không tạo được kswapd ⇒ node **không có reclaim nền**.
- **Board:** hotplug off ⇒ gần như chỉ có thể là BUG lúc boot (hết RAM sớm / scheduler chưa sẵn sàng).

### W1/W2/W3 — `WARNING … at mm/vmscan.c:186|189|3326`

- **W1/W2** (`set_task_reclaim_state`): một task vào reclaim khi `reclaim_state` **đã** được gắn (lồng) / gỡ khi chưa gắn. Entry points đều gọi hàm này: `try_to_free_pages`, `try_to_free_mem_cgroup_pages`, `balance_pgdat`, `shrink_all_memory`, `__node_reclaim`.
  - *Nguyên nhân thật:* **reclaim lồng nhau** — một shrinker/`writepage`/driver *đang chạy trong reclaim* lại cấp phát với gfp cho phép reclaim ⇒ vào direct reclaim lần hai (đáng lẽ phải bị chặn bởi `PF_MEMALLOC`/`memalloc_noreclaim_save`/`GFP_NOFS`).
  - *Nhìn stack của WARN* để thấy ai gọi lần hai.
- **W3** (`mem_cgroup_shrink_node`): soft-limit reclaim chạy khi `current->reclaim_state == NULL` ⇒ caller bỏ sót `set_task_reclaim_state`.
- **Tại sao có:** bảo vệ *kế toán slab* (`reclaimed_slab`) khỏi bị ghi đè. Là **lỗi cấu trúc code**, không phải lỗi tải.

### W4 — `trying to isolate tail page`

`isolate_lru_page()` nhận tail page của compound page `[1788]`. Người gọi (migration/mlock/driver) truyền sai page. Stack của WARN chỉ ra caller.

### B-series

| Mã | Điều kiện | Ý nghĩa |
|---|---|---|
| B2 `__remove_mapping` | caller quên lock page / `mapping` đổi | Bất biến locking của reclaim bị vi phạm |
| B3 `isolate_lru_pages` | `__isolate_lru_page` trả khác `0/-EBUSY` | Hàm trả mã lạ (lỗi code) |
| B4 `get_scan_count` | `scan_balance` ngoài enum | Không thể xảy ra nếu code không bị hỏng |
| B1 | `shrinker->id < 0` | Hủy đăng ký shrinker memcg-aware không hợp lệ |

## 16.2. Log **ngoài `vmscan.c`** nhưng là *dấu hiệu trực tiếp* của reclaim thất bại

> Đây mới là những dòng bạn thực sự gặp trên log khi reclaim "không đủ".

### X1 — `<comm>: page allocation failure: order:<o>, mode:<gfp>(<flags>), nodemask=…` + `dump_stack` + `Mem-Info`

```
LOG ─ warn_alloc() ─ Caller: __alloc_pages_slowpath (nhãn fail:) ─ Stage: allocator (SAU reclaim/compaction/OOM)
```
- **Khi nào:** đi hết slowpath mà không có page, **và** không `__GFP_NOFAIL`, không `__GFP_NOWARN`, không bị rate-limit (1 lần/10 s) `[page_alloc.c:3993-4016, 4975-4977]`.
- **Các đường dẫn đến đây (quan trọng, hay bị hiểu sai):**
  1. **Atomic (`GFP_ATOMIC/NOWAIT`) hết page:** `!can_direct_reclaim ⇒ nopage` ngay `[4853-4854]` — **reclaim KHÔNG hề được thử**. Đây là kịch bản phổ biến nhất (driver mạng/USB/DMA trong IRQ). Nguyên nhân: free cạn *tại thời điểm đó*, kswapd chưa kịp dọn, hoặc order cao bị phân mảnh.
  2. **`__GFP_NORETRY`** (vd. THP fault): bỏ cuộc sớm sau 1 lần reclaim/compact `[4873-4874]`.
  3. **High-order (> 3) không `__GFP_RETRY_MAYFAIL`:** không retry `[4880-4882]`.
  4. **Hết kiên nhẫn:** `no_progress_loops > 16` rồi OOM thất bại `[4587]`.
- **Đọc `Mem-Info`:** các dòng `active_anon … inactive_file … unevictable … dirty … writeback … slab_reclaimable … free … free_pcp` rồi từng `Node … all_unreclaimable? yes|no` `[page_alloc.c:5579-5630]`. **`all_unreclaimable? yes` ⇔ `kswapd_failures ≥ 16`** `[5655]` — **kswapd đã bỏ cuộc**.
- **Nghi ngờ:** pressure thật (so `free` với `min`), phân mảnh (xem dòng `Node 0 Normal: 120*4kB … 0*2048kB` ở phần zone), atomic burst, hoặc RAM bị giữ bởi thứ không reclaim được (`unevictable`, `shmem`, `slab_unreclaimable`, driver).

### X2 — OOM killer: `invoked oom-killer` → `Mem-Info` → `Tasks state` → `oom-kill:constraint=…` → `Out of memory: Killed process …`

```
LOG ─ out_of_memory() (oom_kill.c) ─ Caller: __alloc_pages_may_oom ─ Stage: leo thang cuối cùng, sau khi reclaim = 0
```
- **Khi nào:** `should_reclaim_retry` trả false (hết kiên nhẫn *hoặc* ngay cả free hết reclaimable cũng không đủ watermark) và compaction retry cũng false `[page_alloc.c:4884-4910]`.
- **Chuỗi dòng thật:** `<comm> invoked oom-killer: gfp_mask=…, order=…, oom_score_adj=…` `[oom_kill.c:462]` → (`COMPACTION is disabled!!!` chỉ in khi `!CONFIG_COMPACTION && order` `[465-466]` — **board có COMPACTION**, nên không in) → `dump_stack()` → `Mem-Info:` `[lib/show_mem.c:16]` → `Tasks state (memory values in pages):` + bảng task `[428-429, 405]` → `oom-kill:constraint=…,task=…,pid=…` `[451-456]` → `Out of memory: Killed process <pid> (<name>) total-vm:… anon-rss:… file-rss:… shmem-rss:…` `[927]` → `oom_reaper: reaped process …` `[602]`.
- **Ý nghĩa:** reclaim **đã thất bại hoàn toàn**; hệ thống chấp nhận giết process. `anon-rss/shmem-rss` của nạn nhân và bảng task cho biết *ai giữ RAM*.
- **Nghi ngờ:** (xem 15.5 bước ⑦) — nhất là **không swap + anon/tmpfs chiếm hết**.

### X3 — `Shrinking memory...  done (N pages freed)` / `Drop bdev cache ... done` (từ `kernel/power/warp.c`, **VENDOR**)

```
LOG ─ warp_shrink_memory() ─ Caller: warp snapshot ─ Stage: shrink_all_memory (Flow đặc biệt)
```
- Cho biết WARP đã gọi `shrink_all_memory()` và tổng số page giải phóng `[warp.c:1748-1768]`. Số nhỏ ⇒ ít cache để thu trước khi snapshot.

### X4 — `min_free_kbytes is not updated to <a> because user defined value <b> is preferred`

`init_per_zone_wmark_min` `[page_alloc.c:8108-8111]`: user đã đặt `vm.min_free_kbytes` ⇒ watermark **không** theo công thức tự động.

## 16.3. Bảng tổng: **Log / Message → Function → Khi nào → Flow stage → Ý nghĩa → Cần kiểm tra gì**

| Log / Message | Function | Khi nào xuất hiện | Flow stage | Ý nghĩa | Cần kiểm tra gì |
|---|---|---|---|---|---|
| `vmscan: shrink_slab: <sym> negative objects to delete nr=N` | `do_shrink_slab` | `nr_deferred + delta < 0` | Slab reclaim | Shrinker đếm sai/overflow | Code `count_objects` của `<sym>`; `slabs_scanned`; driver/FS custom |
| `vmscan: pageout: orphaned page` | `pageout` | Dirty page `mapping==NULL`, có buffer, free được buffer | Ghi dirty page | Page mồ côi (journal data) bị dọn | FS mode (`data=journal`), truncate race |
| `vmscan: Failed to start kswapd on node N` | `kswapd_run` | `kthread_run` lỗi (post-boot) | Khởi tạo | Node không có reclaim nền | Hotplug; RAM sớm |
| `WARNING … vmscan.c:186/189` | `set_task_reclaim_state` | reclaim lồng nhau / gỡ sai | Entry | Allocation trong reclaim vào lại reclaim | Stack WARN: ai cấp phát (shrinker/writepage/driver) |
| `WARNING … vmscan.c:3326` | `mem_cgroup_shrink_node` | Thiếu reclaim_state | memcg soft-limit | Caller sai | Stack WARN |
| `trying to isolate tail page` | `isolate_lru_page` | Tail page | Isolation | Người gọi sai | Stack (migrate/mlock/driver) |
| `kernel BUG at mm/vmscan.c:…` | `__remove_mapping`, `isolate_lru_pages`, `kswapd_run`… | Bất biến vỡ | Reclaim page / init | Lỗi nghiêm trọng (panic) | Backtrace; `addr2line` |
| `<comm>: page allocation failure: order:…` | `warn_alloc` (page_alloc.c) | Slowpath thất bại | **Sau** reclaim (hoặc không thử vì atomic) | Allocation không thể thoả | `Mem-Info`, `all_unreclaimable?`, gfp mode, vmstat delta |
| `Node 0 … all_unreclaimable? yes` | `show_free_areas` | `kswapd_failures ≥ 16` | kswapd bỏ cuộc | Node "vô vọng" | Anon/tmpfs có swap không; unevictable; pinned |
| `<comm> invoked oom-killer: …` | `out_of_memory` | Reclaim = 0 + retry hết | Leo thang cuối | Reclaim thất bại hoàn toàn | Bảng `Tasks state`, `Mem-Info`, swap |
| `Out of memory: Killed process …` | `oom_kill_process` | như trên | OOM | Nạn nhân & RSS | Ai chiếm RAM |
| `Shrinking memory…  done (N pages freed)` | `warp_shrink_memory` (VENDOR) | WARP snapshot | `shrink_all_memory` | Kết quả shrink trước snapshot | N nhỏ ⇒ ít cache |
| `min_free_kbytes is not updated …` | `init_per_zone_wmark_min` | User set `min_free_kbytes` | Watermark | Watermark thủ công | `/proc/sys/vm/min_free_kbytes` |
| **(counter)** `allocstall_*` tăng | `do_try_to_free_pages` | Mỗi lần vào direct reclaim | Direct reclaim | Task bị chậm | Tốc độ alloc vs kswapd |
| **(counter)** `pgscan_direct_throttle` tăng | `throttle_direct_reclaim` | Reserve PFMEMALLOC ≤ ½ | Throttle | Task ngủ chờ kswapd | kswapd có chạy? CPU? |
| **(counter)** `kswapd_*_wmark_hit_quickly` tăng | `kswapd_try_to_sleep` | Không ngủ yên được | kswapd sleep | Áp lực kéo dài | Alloc rate; hiệu suất |
| **(counter)** `nr_vmscan_immediate_reclaim` tăng | `shrink_page_list` | Dirty file page bị đánh dấu | Page evaluation | Dirty ở đuôi LRU | Flusher; I/O |
| **(counter)** `pgactivate` ≫ `pgsteal` | `shrink_page_list` | Page bị cứu | Page evaluation | Referenced/dirty/unmap fail | Phân biệt bằng các counter khác |
| **(tracepoint)** `mm_vmscan_lru_shrink_inactive` | `shrink_inactive_list` | Mỗi lô 32 | Scanning | "Khám bệnh" từng lô | nr_dirty, nr_ref_keep, nr_unmap_fail |

---

# 17. Debug Scenarios

> **Các con số trong ví dụ** là *tổng hợp để minh hoạ cách đọc* (tạo bằng dữ liệu giả cho `tools/vmstat_delta.sh`), **không phải đo từ board**.

## Scenario 1 — RAM thấp → kswapd (luồng khoẻ)

```
Available memory ↓  (free vượt LOW)
        ↓
allocator: fast path (LOW) fail → wake_all_kswapds → retry (MIN) OK     [task KHÔNG bị chậm]
        ↓
kswapd0: balance_pgdat → shrink_node (priority 12, quét 1/4096) → thu page cache sạch
        ↓
free vượt HIGH → pgdat_balanced → kswapd_try_to_sleep (nap 100ms → ngủ dài)
```
**Dấu hiệu khoẻ:** `pageoutrun` tăng chút; `pgscan_kswapd`/`pgsteal_kswapd` tăng với **hiệu suất cao (≳50%)**; `allocstall_* = 0`; không log.
**Nếu `kswapd_*_wmark_hit_quickly` tăng dần:** kswapd vẫn ổn nhưng áp lực dai dẳng.

## Scenario 2 — Allocation thiếu memory → direct reclaim

```
application ──malloc/ghi lần đầu──► page fault ──► __alloc_pages_nodemask
        │ free < MIN (retry MIN fail)
        ▼
__alloc_pages_direct_reclaim → try_to_free_pages (throttle? → do_try_to_free_pages)
        │ thu ≥ 32 page → get_page_from_freelist → app tiếp tục (đã chậm)
        │ thu 0         → compaction/retry → OOM
```
**Dấu hiệu:** `allocstall_*` > 0, `pgscan_direct` tăng. App chậm đúng bằng thời gian reclaim. Không log trừ khi thất bại (X1/X2).
**Hỏi tiếp:** kswapd có đang chạy kịp không? (`pageoutrun`, CPU `kswapd0`). Direct reclaim xuất hiện *cùng* kswapd bận nghĩa là **alloc rate > reclaim rate**; xuất hiện khi kswapd *ngủ* ⇒ kswapd hopeless (`all_unreclaimable? yes`) hoặc watermark/`order` làm kswapd không thức.

## Scenario 3 — Reclaim không hiệu quả (quét nhiều, thu ít)

Ví dụ minh hoạ (kết quả của `vmstat_delta.sh` trên dữ liệu giả, 10 s):
```
pgscan_kswapd 50000  pgsteal_kswapd 500   → 1.0%      pgscan_direct 20000  pgsteal_direct 300 → 1.5%
pgscan_file 70000    pgsteal_file 800 (1.1%)           pgscan_anon 0
pgscan_skip_normal 4000   pgactivate 20000   nr_vmscan_immediate_reclaim 700   allocstall 15
kswapd0 CPU 90% của 1 CPU
```
**Đọc từng chỉ số:**
| Quan sát | Gợi ý |
|---|---|
| `pgscan_anon = 0` | **Không swap** ⇒ chỉ page cache gánh áp lực; anon/tmpfs không động được |
| `pgactivate 20000` (~29% của `pgscan_file`) | Rất nhiều page bị **cứu lên active** (referenced hoặc dirty-file bị đánh dấu) |
| `nr_vmscan_immediate_reclaim 700` | Có dirty file page ở đuôi LRU (nhưng chỉ 3.5% `pgactivate` ⇒ chưa phải thủ phạm chính) |
| `pgscan_skip_normal 4000` (5.7%) | Một phần quét lãng phí do zone-skip |
| kswapd 90% CPU + hiệu suất 1% | kswapd **quay vô ích** — vì `kswapd_failures` chỉ tăng khi *không thu nổi 1 page* nên không "tự bỏ cuộc" (xem 7.6) |

**Cây nguyên nhân (đối chiếu counter):**

| Nguyên nhân | Dấu hiệu | Cách xác nhận | Hướng xử lý |
|---|---|---|---|
| **Working set > RAM (thrash page cache)** | `pgactivate` cao, `workingset_refault_file` cao, `pgmajfault` cao | meminfo `Active(file)` lớn; refault tăng | Giảm working set; tăng RAM; kiểm tra app đọc lặp file lớn |
| **Anon/tmpfs chiếm hết, không swap** | `pgscan_anon=0`; `Shmem`, `Active(anon)` lớn | `cat /proc/swaps` rỗng; `df` tmpfs | Giới hạn tmpfs; giảm anon; (nếu hợp lý) thêm swap/zram |
| **Dirty/writeback nghẽn** | `nr_vmscan_immediate_reclaim` cao; `Dirty`/`Writeback` lớn | Sampling stack: `congestion_wait` | Tinh chỉnh `dirty_*_ratio`; kiểm tra tốc độ storage |
| **Zone-skip** | `pgscan_skip_*` lớn | `zoneinfo`: zone thấp cạn, zone cao nhiều | Xem `lowmem_reserve`, nguồn xin `GFP_DMA/DMA32` |
| **Pinned/unmap fail** | khó thấy | (tracepoint `nr_unmap_fail`); `Mlocked`, DMA pin | Driver giữ page dài hạn |
| **Phân mảnh/boost** | `kswapd` chạy dù `free` lớn | `buddyinfo`; `watermark_boost_factor` | Giảm boost; xem order xin |
| **Quá nhiều reclaimer** | `nr_isolated_*` cao; msleep(100) | stack `msleep ← shrink_inactive_list` | Giảm đồng thời; nới RAM |

## Scenario 4 — Dirty pages

```
dirty page ở đuôi LRU
     ↓
KHÔNG thể vứt (mất dữ liệu)
     ↓
reclaim: SetPageReclaim + (activate)  — không tự ghi (trừ kswapd khi PGDAT_DIRTY)
     ↓
cuối lô: toàn dirty chưa xếp hàng? → wakeup_flusher_threads
     ↓
flusher ghi → end_page_writeback → PageReclaim → rotate về đuôi inactive → lần sau FREE được
```
**Triệu chứng:** `nr_vmscan_immediate_reclaim` ↑; `Dirty`/`Writeback` cao; direct reclaim ngủ ở `congestion_wait`/`wait_iff_congested`; `kswapd` có `congestion_wait` (`nr_immediate`).
**Hiểu sai thường gặp:** "reclaim ghi hết dirty page" — **sai**; direct reclaim *không* ghi file page; kswapd chỉ ghi khi `PGDAT_DIRTY` (đuôi LRU toàn dirty chưa xếp hàng).
**Board:** storage chậm (flash/eMMC/NAND) ⇒ writeback chậm ⇒ nhánh này dễ lộ. `[INFERENCE — kiểu storage KIỂM TRA TRÊN BOARD]`

## Scenario 5 — Reclaim stall

```
Reclaim chậm / không tiến triển
   ├─ Task nào bị block?   → mọi task xin RAM (direct reclaim) + task chờ lock của chúng
   ├─ kswapd thế nào?      → R (CPU-bound, SCHED_RR 200!) | D/S ở congestion_wait | ngủ vì hopeless
   ├─ Log nào có thể có?   → (mặc định board: KHÔNG hung-task) → chỉ page allocation failure / OOM
   └─ Xem gì?              → vmstat delta + stack sampling + load average (D-state tăng load)
```
**Bước điều tra:**
1. `cat /proc/loadavg` — load cao với CPU idle ⇒ nhiều task D-state (đang chờ).
2. `grep -l 'D' /proc/[0-9]*/status`… rồi `cat /proc/PID/stack` cho vài task: dùng **bảng chữ ký stack ở 15.5 ⑧**.
3. `vmstat_delta.sh`: `allocstall_*`, `pgscan_direct_throttle`, hiệu suất.
4. `ps -o pid,cls,rtprio,pcpu,comm | grep kswapd`: kswapd `RR 200` và CPU cao?
5. `Mem-Info` (nếu có alloc failure) — `all_unreclaimable?`.

**Đặc thù board (VENDOR + config):** vì kswapd là RT (prio 200) **và RT throttling tắt**, một kswapd quay vô ích có thể **chiếm hoàn toàn 1 CPU** ⇒ các task `SCHED_NORMAL` (và RT thấp hơn 200) trên core đó đói CPU ⇒ trông như "hệ thống đơ" dù không ai deadlock. Vì **không có hung-task/softlockup/SysRq**, hiện tượng này **không tự báo log**.

## Scenario 6 — Bài toán đích của bạn

> **RAM gần đầy · kswapd CPU cao · direct reclaim xuất hiện · scan nhiều · reclaim ít · latency tăng**

```
Memory pressure (free gần MIN)
      ↓
Reclaim activated ─ Which path? ─► CẢ HAI: kswapd (CPU cao) + direct (allocstall>0)
      ↓
Which LRU?  ─ pgscan_anon vs pgscan_file ─► nếu anon=0 → không swap
      ↓
How much scanning?  Δpgscan_*  (lớn)
      ↓
How many reclaimed? Δpgsteal_* (nhỏ) → hiệu suất < 10%
      ↓
Why pages can't be reclaimed?  → Scenario 3 (referenced/dirty/anon-no-swap/zone-skip/pinned)
      ↓
Where is reclaim spending time? → stack sampling: R trong isolate/page_referenced/shrink_page_list
                                    hay ngủ ở congestion_wait/msleep/throttle
      ↓
Potential root cause
```

**Phân tích theo từng khối (người mới):**
1. *Cả kswapd và direct cùng chạy* ⇒ **alloc rate > reclaim rate** kéo dài (nếu kswapd còn thức) — kswapd không "hopeless" vì vẫn thu được vài page (nên `kswapd_failures` không lên 16).
2. *Hiệu suất thấp* ⇒ phần lớn page quét **được cứu** hoặc **không free được**.
3. *Latency tăng* có hai nguồn: (a) task **tự reclaim** (direct) + ngủ ở `msleep(100)`/`congestion_wait`/`throttle`; (b) **kswapd RT chiếm CPU** (vendor) làm task thường đói CPU.
4. *Nguyên nhân gốc điển hình* (giả thuyết cho dòng máy MFP — phải xác nhận bằng số đo):
   - **Bộ nhớ không thể reclaim chiếm phần lớn RAM:** anon của ứng dụng xử lý ảnh, **tmpfs** (spool/tmp trong RAM), vùng driver (CMEM) `[INFERENCE]`.
   - **Không có swap** ⇒ chúng bất khả xâm phạm ⇒ page cache còn lại bị cày liên tục (thrash) ⇒ hiệu suất thấp + `pgactivate` cao.
   - **THP=always** có thể làm phân mảnh/ tăng order xin ⇒ boost/compaction `[INFERENCE]`.
5. **Hướng hành động (theo độ rủi ro tăng dần):** (i) đo bằng `vmstat_delta.sh` 3–5 mẫu; (ii) xác nhận swap/tmpfs/`Shmem`; (iii) xem `smaps`/`pagemap` ai giữ anon lớn; (iv) *nếu cần can thiệp kernel*: dùng build debug có ftrace+PSI (đề xuất 15.1); (v) cân nhắc hạ ưu tiên RT của kswapd (đây là thay đổi vendor — **cần xin ý kiến người sở hữu code**, tài liệu này không sửa source).

Tiếp theo → [`06_subsystems_confusions_story.md`](06_subsystems_confusions_story.md).
