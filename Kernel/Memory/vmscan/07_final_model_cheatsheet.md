# 07 — Final Mental Model & Debug Cheat Sheet

> Phủ các mục: **21** Final Mental Model · **22** Debug Cheat Sheet · danh sách "chưa verify".
> Toàn bộ tài liệu: [`00`](00_index_executive_summary_version.md) · [`01`](01_concepts_architecture_big_flow.md) · [`02`](02_kswapd_and_direct_reclaim.md) · [`03`](03_lru_scanning_reclaim_dirty_progress.md) · [`04`](04_function_analysis.md) · [`05`](05_debugging_logs_scenarios.md) · [`06`](06_subsystems_confusions_story.md)

---

# 21. Final Mental Model

## 21.1. Sơ đồ lớn (đúng với Linux 5.10.241, có tên hàm thật)

```
                                MEMORY PRESSURE
        (free < watermark · memcg vượt limit · fragmentation boost · drop_caches/WARP/MADV_PAGEOUT)
                                       │
                                       ▼
                               RECLAIM TRIGGER   (page_alloc.c / memcontrol.c)
                                       │
              ┌────────────────────────┴─────────────────────────┐
              │ free<LOW: wakeup_kswapd()                         │ free<MIN: __alloc_pages_direct_reclaim()
              ▼                                                   ▼
        ┌───────────────┐                                  ┌───────────────────────┐
        │    KSWAPD     │  nền · RT(200) · ghi được file    │    DIRECT RECLAIM     │  đồng bộ · task chờ
        │ kswapd()      │                                  │ try_to_free_pages()   │  (throttle → loop)
        │ balance_pgdat │                                  │ do_try_to_free_pages  │
        └───────┬───────┘                                  └───────────┬───────────┘
                │ kswapd_shrink_node                                   │ shrink_zones
                └───────────────────────┬──────────────────────────────┘
                                        ▼
                                RECLAIM CONTROL
                       scan_control{ nr_to_reclaim · priority 12→0/1 · reclaim_idx
                                      may_writepage/unmap/swap · gfp_mask · order }
                                        │
                                        ▼
                                  SELECT TARGET
              node → (memcg tree) → lruvec   +   slab shrinkers          shrink_node → shrink_node_memcgs
                                        │
                                        ▼
                                   SELECT LRU
              get_scan_count():  file | anon | fraction ─ swappiness · swap? · refault cost
                                 · file_is_tiny · cache_trim_mode · priority
                                        │
                                        ▼ lô 32 page (SWAP_CLUSTER_MAX)
                                      SCAN
         ACTIVE list  → shrink_active_list : xoá Accessed bit, HẠ cấp (không free)
         INACTIVE list→ shrink_inactive_list : isolate_lru_pages (đuôi, dưới pgdat->lru_lock)
                                        │
                                        ▼
                               EVALUATE PAGE   (shrink_page_list)
   trylock → evictable → may_unmap → writeback? → referenced? → (anon: swap) → mapped: unmap → dirty? → buffers → remove_mapping
                                        │
             ┌──────────────────────────┼──────────────────────────┬───────────────────────┐
             ▼                          ▼                          ▼                       ▼
          RECLAIM                      KEEP                  SPECIAL CASE              ACTIVATE
     __remove_mapping            (xoay về inactive)      · dirty file → PG_reclaim     (sang active)
     free_unref_page_list                                · writeback → PG_reclaim
                                                         · THP split · lazyfree
                                                         · pageout() (kswapd, PGDAT_DIRTY)
             │                          │                          │                       │
             └──────────────────────────┴────────────┬─────────────┴───────────────────────┘
                                                     ▼
                                          RECLAIM PROGRESS?
                    sc->nr_reclaimed ≥ nr_to_reclaim ? · should_continue_reclaim() (order>0)
                    · kswapd: pgdat_balanced?  · vmpressure() · kswapd_failures
                                      /                         \
                                   YES                          NO
                                    │                            │
                                    ▼                            ▼
                               Dừng / ngủ                 Escalate / Retry / Exit
                      kswapd: nap 100ms → ngủ dài      priority-- (quét nhiều hơn)
                      direct: trả N cho allocator      kswapd: ≥16 lần 0 page → "hopeless"
                                    │                  direct : trả 0 → compaction → retry×16 → OOM
                                    ▼                            │
                             MEMORY RECOVERED  ◄──────────────────┘ (nếu leo thang thành công)
                       (allocator cấp lại page; drain per-CPU)
```

## 21.2. Giải thích từng khối bằng ngôn ngữ người mới

| Khối | Nói đơn giản | Hàm | Nếu không có thì |
|---|---|---|---|
| **MEMORY PRESSURE** | RAM trống tụt dưới ngưỡng an toàn | watermark (`page_alloc.c`) | Không ai biết cần dọn |
| **RECLAIM TRIGGER** | Ai bấm chuông: allocator khi sắp/đã hết | `wakeup_kswapd`, `__perform_reclaim` | Hết RAM mới phát hiện → OOM |
| **KSWAPD** | Người dọn nền, làm *trước khi ai bị chặn* | `kswapd`, `balance_pgdat` | Mọi người tự dọn → chậm; atomic không có đường |
| **DIRECT RECLAIM** | Phao cứu sinh: chính người xin tự dọn | `try_to_free_pages` | Allocation fail khi kswapd chậm |
| **RECLAIM CONTROL** | "Phiếu yêu cầu": dọn bao nhiêu, mạnh cỡ nào, được làm gì | `scan_control`, `priority` | Không có kỷ luật → dọn thừa/thiếu |
| **SELECT TARGET** | Chọn vùng (node/zone/memcg) và cache kernel (slab) | `shrink_node`, `shrink_node_memcgs`, `shrink_slab` | Dọn nhầm vùng không giúp được allocation |
| **SELECT LRU** | Chọn lấy từ cache (file) hay bộ nhớ app (anon), tỉ lệ bao nhiêu | `get_scan_count` | Swap thừa hoặc đập mãi cache đang thrash |
| **SCAN** | Lấy lô 32 page từ *đuôi* list (lâu chưa dùng) | `isolate_lru_pages`, `shrink_*_list` | Không có ứng viên |
| **EVALUATE PAGE** | Hỏi từng page: còn dùng? bẩn? đang map? có ai pin? | `shrink_page_list` | Xoá nhầm page đang dùng/dirty → hỏng dữ liệu |
| **RECLAIM / KEEP / ACTIVATE / SPECIAL** | 4 kết cục: free · giữ · nâng cấp · xử lý đặc biệt | `__remove_mapping`, `pageout` | — |
| **RECLAIM PROGRESS?** | Có thu đủ không, có tiến triển không | `should_continue_reclaim`, `kswapd_failures` | Lặp vô hạn hoặc bỏ cuộc quá sớm |
| **Escalate / Retry / Exit** | Quét mạnh hơn → nếu vẫn không → bỏ cuộc cho allocator | `--priority`, `should_reclaim_retry` | Treo hoặc OOM sớm |
| **MEMORY RECOVERED** | Có page trống để cấp | | |

## 21.3. Năm câu cần nhớ

1. **Reclaim chỉ lấy từ đuôi inactive** — active chỉ bị hạ cấp, không bị free trực tiếp.
2. **Mỗi page có nghi thức riêng** (clean → vứt; dirty → ghi/chờ; mapped → gỡ PTE; anon → swap). Không có nghi thức khả thi ⇒ page *không reclaim được*.
3. **`priority` là núm vặn độ gấp** (quét `size >> priority`); direct xuống tới 0, kswapd tới 1.
4. **Chỉ kswapd ghi dirty file page**, và chỉ khi đuôi LRU toàn dirty chưa xếp hàng (`PGDAT_DIRTY`).
5. **Reclaim thất bại ≠ OOM**: OOM do *allocator* quyết sau khi reclaim = 0 và retry hết; vmscan không giết ai.

---

# 22. Debug Cheat Sheet

## 22.1. Triệu chứng → counter đầu tiên → ý nghĩa → làm gì tiếp

| Triệu chứng | Nhìn trước | Nếu thấy | Làm tiếp |
|---|---|---|---|
| App chậm, RAM gần đầy | `allocstall_*` (delta) | `> 0` ⇒ có direct reclaim | `pgscan_direct`/`pgsteal_direct`; kswapd có chạy kịp? |
| `kswapd0` CPU cao | `pageoutrun`, CPU `kswapd0`, hiệu suất `pgsteal_kswapd/pgscan_kswapd` | hiệu suất < 10% | Scenario 3: referenced/dirty/anon-no-swap/zone-skip |
| Quét nhiều, thu ít | `pgactivate` vs `pgsteal`; `nr_vmscan_immediate_reclaim`; `pgscan_anon` | `pgactivate` lớn · `immediate` lớn · `anon=0` | Tương ứng: thrash · dirty · không swap |
| Có `Dirty/Writeback` lớn | `nr_vmscan_write`, `nr_vmscan_immediate_reclaim`, meminfo | cao | Storage chậm? `vm.dirty_*_ratio`; stack `congestion_wait` |
| kswapd vừa ngủ lại dậy | `kswapd_low/high_wmark_hit_quickly` | tăng | alloc rate ≥ reclaim rate (áp lực kéo dài) |
| Task ngủ lâu trong alloc | `pgscan_direct_throttle`; stack | `> 0` / `throttle_direct_reclaim` | reserve cạn; kswapd chạy không? |
| `free` lớn nhưng kswapd chạy | `buddyinfo`, `watermark_boost_factor` | order cao = 0 | Fragmentation boost (7.7) |
| `page allocation failure` | gfp mode + `Mem-Info` | `GFP_ATOMIC` | Atomic không reclaim được; tăng `min_free_kbytes` / giảm burst |
| `all_unreclaimable? yes` | — | kswapd ≥16 lần thất bại | Node vô vọng; xem anon/tmpfs/unevictable/pinned |
| `invoked oom-killer` | bảng `Tasks state`, `Mem-Info`, `/proc/swaps` | anon/shmem lớn, không swap | Ai chiếm RAM; giới hạn tmpfs/ứng dụng |
| `vmscan: shrink_slab: … negative objects` | `%pS` | symbol shrinker | Sửa `count_objects` của subsystem đó |
| `WARNING … vmscan.c:186` | stack của WARN | reclaim lồng | Tìm allocation có `__GFP_RECLAIM` trong reclaim path |

## 22.2. Công thức

```
hiệu suất            = Δpgsteal_* / Δpgscan_*          (kswapd và direct riêng; anon và file riêng)
tốc độ direct stall  = Δallocstall_* / Δt              (đếm cả vòng retry)
vmpressure%          ≈ (1 − reclaimed/scanned) × 100   (cửa sổ 512 page; medium ≥60, critical ≥95)
scan_target(lru)     = lru_size >> priority            (priority 12 → 0)
watermark            MIN = min_free_kbytes/4 (page) ; LOW = MIN + max(MIN/4, managed×scale/10000) ; HIGH = MIN + 2×(…)
                      (scale = vm.watermark_scale_factor, mặc định 10)
kswapd mục tiêu      = Σ max(HIGH_zone, 32)
SCAN_FRACT           ap = swappiness×(T+1)/(anon_cost'+1) ; fp = (200−swappiness)×(T+1)/(file_cost'+1)
slab delta           = (freeable >> priority) × 4 / seeks   (gọi scan_objects khi ≥128 hoặc ≥ freeable)
```

## 22.3. Hằng số đáng nhớ (kèm nguồn)

| Hằng | Giá trị | Nguồn |
|---|---|---|
| `DEF_PRIORITY` | 12 | `include/linux/mmzone.h:641` |
| `SWAP_CLUSTER_MAX` | 32 | `include/linux/swap.h:183` |
| `MAX_RECLAIM_RETRIES` | 16 | `mm/internal.h:125` |
| `PAGE_ALLOC_COSTLY_ORDER` | 3 | `include/linux/mmzone.h:39` |
| kswapd nap | `HZ/10` = 100 ms | `vmscan.c:3830` |
| `wait_iff_congested`/`congestion_wait` | `HZ/10` | `vmscan.c:2842, 2867` |
| `msleep` ở `too_many_isolated` | 100 ms (một lần) | `vmscan.c:1955` |
| throttle `!__GFP_FS` | tối đa `HZ` = 1 s | `vmscan.c:3247` |
| `vmpressure_win` | 512 page | `vmpressure.c:38` |
| `vm.swappiness` | 60 | `vmscan.c:180` |
| `watermark_scale_factor` / `watermark_boost_factor` | 10 / 15000 | `page_alloc.c:369, 367` |
| `SHRINK_BATCH` | 128 | `vmscan.c:427` |
| `HZ` (board) | 200 ⇒ 100 ms = 20 tick | `Eagle_defconfig:410` |

## 22.4. Lệnh an toàn trên board (đọc-chỉ)

```sh
cat /proc/meminfo                  # MemAvailable, Active/Inactive(anon/file), Dirty, Writeback, Shmem, Slab…
cat /proc/zoneinfo | grep -E 'Node|pages free|min|low|high|protection'   # so free với min/low/high
cat /proc/buddyinfo ; cat /proc/pagetypeinfo     # phân mảnh
cat /proc/swaps                    # CÓ swap không?  (rỗng = anon không reclaim được)
grep -E 'pgscan|pgsteal|allocstall|pageoutrun|pgrefill|pgdeactivate|pgactivate|nr_vmscan|kswapd_|workingset|oom_kill|thp_swpout' /proc/vmstat
sh vmstat_delta.sh 10              # (tools/) delta 10 s + chẩn đoán nhanh
ps -o pid,cls,rtprio,pcpu,comm | grep kswapd      # kỳ vọng: RR 200 (VENDOR)
cat /proc/sys/vm/{swappiness,min_free_kbytes,watermark_scale_factor,watermark_boost_factor,dirty_ratio,dirty_background_ratio,vfs_cache_pressure,laptop_mode}
cat /proc/sys/kernel/sched_rt_runtime_us          # kỳ vọng 1000000 (VENDOR: tắt RT throttling)
for i in 1 2 3 4 5; do cat /proc/<PID>/stack | head -6; echo --; sleep 0.2; done   # stack lấy mẫu
ls /sys/kernel/debug/tracing/events/vmscan 2>&1   # tracepoint có không?
```
> **Thay đổi tuỳ chọn (cần cân nhắc):** `echo 3 > /proc/sys/vm/drop_caches` chỉ để thử nghiệm (đẩy cache đi, *làm hiệu suất xấu đi*); không dùng để "sửa" pressure.

## 22.5. Chữ ký stack nhanh

| Thấy | Nghĩa |
|---|---|
| `msleep ← shrink_inactive_list` | `too_many_isolated` (quá nhiều reclaimer) |
| `congestion_wait`/`wait_iff_congested ← shrink_node` | dirty/writeback nghẽn |
| `wait_event_killable ← throttle_direct_reclaim` | chờ kswapd (reserve cạn) |
| `wait_on_page_writeback ← shrink_page_list` | legacy memcg chờ I/O |
| `congestion_wait ← should_reclaim_retry` | allocator chờ I/O |
| **R** ở `page_referenced`/`rmap_walk`/`isolate_lru_pages`/`shrink_page_list` | CPU-bound scanning |
| `schedule ← kswapd_try_to_sleep` | kswapd ngủ (bình thường) |

## 22.6. Sự thật về **board này** (một trang)

| Mục | Giá trị |
|---|---|
| Kernel | 5.10.241 + 4 hunk vendor ở `vmscan.c` |
| Node/CPU | 1 node (`kswapd0`), 4 CPU, `PREEMPT`, `HZ=200` |
| Zone | DMA, DMA32, Normal; 4 KiB page |
| **kswapd** | **`SCHED_RR` prio 200 (`KM_BIZHUB`+`HIGH_PRIORITY`)**; RT throttling **tắt** (`sched_rt_runtime=1000000`) |
| **PM_WARP** | reclaim bị `break` khi `pm_device_down && warp_canceled`; `shrink_all_memory` dùng bởi WARP |
| Swap | `SWAP=y` nhưng **không ZRAM/ZSWAP** — `[KIỂM TRA: cat /proc/swaps]` |
| THP | `always` |
| Không có | PSI, FTRACE/tracepoint, hung-task/softlockup, SysRq (15/15 board), perf, taskstats, `DEBUG_VM`, `DYNAMIC_DEBUG`, NUMA `node_reclaim`, CMA |
| Có | `/proc/vmstat` (`VM_EVENT_COUNTERS`), `/proc/PID/stack`, `DEBUG_FS`, memcg (không `BLK_CGROUP`/`CGROUP_WRITEBACK`) |

## 22.7. Nếu chỉ nhớ 10 điều

1. `vmscan.c` im lặng — debug bằng **counter `/proc/vmstat`**, không bằng log.
2. **Hiệu suất = Δpgsteal/Δpgscan** là chỉ số số 1.
3. `allocstall_*` > 0 ⇒ task đã bị chậm vì direct reclaim.
4. Giữa LOW và MIN: kswapd được đánh thức nhưng task *không* bị chậm. Dưới MIN: direct reclaim.
5. Không swap ⇒ anon/tmpfs **không reclaim được** ⇒ cache sạch hữu hạn.
6. Direct reclaim **không ghi** dirty file page; chỉ kswapd (khi `PGDAT_DIRTY`).
7. kswapd chỉ "bỏ cuộc" sau **16 đợt hoàn toàn 0 page**; thu vài page là đủ để nó quay mãi.
8. Trên board: kswapd RT 200 + không RT throttle ⇒ quay vô ích = **đói CPU** cho task thường.
9. Không hung-task/softlockup/SysRq ⇒ reclaim stall **không tự báo** ⇒ phải lấy mẫu stack.
10. `page allocation failure` với `GFP_ATOMIC` nghĩa là **reclaim chưa hề được thử** — không phải reclaim thất bại.

---

# Phụ lục A — Những gì **chưa** verify (trung thực về ranh giới tài liệu)

| Điều | Trạng thái | Cách xác nhận |
|---|---|---|
| Dung lượng RAM của S800 | **Không biết** (ví dụ 1 GiB là giả định của đề bài) | `/proc/meminfo`, `zoneinfo` |
| Có swap/zram lúc chạy không | **Không biết** (config không có ZRAM/ZSWAP) | `/proc/swaps` |
| Loại storage, file system, tmpfs dùng | **Không biết** | `mount`, `/proc/mounts` |
| Tracepoint thực sự có hay không | **Suy luận** từ config | `ls …/tracing/events/vmscan` |
| Giá trị sysctl runtime (`min_free_kbytes` có thể bị `khugepaged` điều chỉnh khi THP=always) | **Không biết** | `/proc/sys/vm/*` |
| Ý đồ thiết kế của V2/V3 (PM_WARP) | **Suy luận** từ code (không có tài liệu KM) | hỏi tác giả patch / ticket |
| Chủ sở hữu từng shrinker (fs/…) | Chỉ nêu kiến thức chung | `grep register_shrinker` |
| Nội bộ `compaction.c`, `workingset.c`, `oom_kill.c`, `memcontrol.c` | Chỉ đọc điểm giao tiếp | — |
| `diff` upstream | Đã làm với tag `v5.10.241` (git.kernel.org) — lệnh lặp lại được: tải `…/stable/linux.git/plain/mm/vmscan.c?h=v5.10.241` rồi `diff -u` | file tải về nằm ở thư mục tạm, không lưu kèm |

# Phụ lục B — Công cụ đi kèm

| File | Chức năng | Ghi chú |
|---|---|---|
| `tools/vmscan_numbers.py` (+`.out.txt`) | Tính watermark, scan theo priority, inactive_ratio, SCAN_FRACT, slab delta, throttle, compact_gap, vmpressure | Công thức chép từ source; giả định 1 GiB |
| `tools/kswapd_toy_sim.py` (+`.out.txt`) | **Mô hình đồ chơi** của `balance_pgdat` cho 3 kịch bản | **Không phải kernel**; chỉ để hình dung dòng chảy |
| `tools/vmstat_delta.sh` | Đo delta `/proc/vmstat`, hiệu suất, chẩn đoán nhanh | POSIX sh + awk; đã test offline (`--files A B`) bằng dữ liệu giả |
