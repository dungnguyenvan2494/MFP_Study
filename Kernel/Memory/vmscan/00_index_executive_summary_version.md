# `mm/vmscan.c` — Mental Model của Memory Reclaim (Linux 5.10.241, board S800)

> **Đối tượng đọc:** người mới học Linux kernel, muốn *hiểu thiết kế* và *debug được* memory reclaim trên board thật.
> **Cách đọc:** đi theo thứ tự file `00 → 07`. Mỗi file ghi rõ nó phủ các mục nào trong cấu trúc 22 mục.
> **Nguyên tắc:** *Problem → Design → Concept → Flow → Decision → Result → Debug → (cuối cùng mới) Code mapping.*

---

## Chú giải thẻ (tag) dùng xuyên suốt

| Thẻ | Nghĩa |
|---|---|
| `[SOURCE file:line]` | Đã đọc trực tiếp trong source local `Kernel/K-S800/Src` (số dòng đã verify). |
| `[DIFF]` | Kết quả `diff` giữa file local và upstream `v5.10.241` (tải từ git.kernel.org). |
| `[GENERAL KERNEL KNOWLEDGE]` | Kiến thức MM chung, **không** được chứng minh bằng một dòng cụ thể trong cây này. |
| `[INFERENCE]` | Suy luận từ source + config; cần kiểm chứng trên board. |
| `[KIỂM TRA TRÊN BOARD]` | Tài liệu không thể biết (phụ thuộc runtime/userspace); phải đo. |
| `UPSTREAM` | Hành vi giống kernel.org 5.10.241. |
| `VENDOR / CUSTOM` | Do Konica Minolta (`CONFIG_KM_BIZHUB`, `CONFIG_PM_WARP`…) thêm vào. |

---

## Bản đồ tài liệu ↔ cấu trúc 22 mục

| File | Mục phủ |
|---|---|
| **`00_index_executive_summary_version.md`** (file này) | 01 Executive Summary · 20 (Version accuracy) · cấu hình board |
| `01_concepts_architecture_big_flow.md` | 02 Memory reclaim là gì · 03 Tại sao cần vmscan.c · 04 Overall Architecture · 05 BIG FLOW · 06 Reclaim Concepts |
| `02_kswapd_and_direct_reclaim.md` | 07 kswapd · 08 Direct Reclaim |
| `03_lru_scanning_reclaim_dirty_progress.md` | 09 LRU · 10 Page Scanning · 11 Page Reclaim · 12 Dirty/Clean · 13 Reclaim Progress |
| `04_function_analysis.md` | 14 Function-by-Function |
| `05_debugging_logs_scenarios.md` | 15 Debugging · 16 Log→Function→Flow · 17 Debug Scenarios |
| `06_subsystems_confusions_story.md` | 18 Quan hệ với MM subsystem khác · 19 Common Confusions · 20 End-to-End Example |
| `07_final_model_cheatsheet.md` | 21 Final Mental Model · 22 Debug Cheat Sheet |
| `tools/vmscan_numbers.py` (+ `.out.txt`) | Mô phỏng mọi con số ví dụ (watermark, priority, scan balance…) |
| `tools/vmstat_delta.sh` | Script đo delta `/proc/vmstat` trên board |

---

# 01. Executive Summary

## 1.1. `vmscan.c` là gì trong một câu

> `vmscan.c` là **động cơ thu hồi RAM** của kernel: khi RAM trống xuống thấp, nó chọn những page ít giá trị nhất
> (cache ít dùng, memory không ai chạm tới), "dọn" chúng đi một cách an toàn (xoá nếu sạch, ghi ra đĩa nếu bẩn,
> gỡ khỏi process nếu đang map) rồi trả page về cho allocator.

Nó **không** quyết định *khi nào RAM là thấp* (đó là việc của `mm/page_alloc.c` – watermark) và **không** quyết định *giết process* (đó là `mm/oom_kill.c`).
Nó trả lời đúng một câu hỏi: **"Cho tôi N page, lấy ở đâu, lấy theo thứ tự nào, lấy xong thì ghi nhận thế nào?"**

## 1.2. Bức tranh 30 giây

```
 Allocator thấy free < watermark ──► gọi vmscan.c theo 2 đường:

   (A) kswapd0 (kernel thread, nền)          (B) Direct reclaim (chính task đang xin RAM)
        balance_pgdat()                           try_to_free_pages()
              └───────────────┬────────────────────────┘
                              ▼
                   shrink_node()   ← "quản đốc" của 1 node
                              │
              ┌───────────────┴────────────────┐
              ▼                                ▼
      shrink_lruvec()                      shrink_slab()
   (page cache + anon qua LRU)       (dentry/inode/… qua shrinker)
              │
              ▼
   isolate_lru_pages() → shrink_page_list()  ← quyết định số phận TỪNG page
              │
   ┌──────────┼───────────────┬──────────────────┐
   ▼          ▼               ▼                  ▼
 FREE      KEEP (xoay)     ACTIVATE        WRITE (dirty) rồi chờ
```

## 1.3. Bốn con số/khái niệm đáng nhớ nhất

1. **Hai watermark quan trọng:** `LOW` (thấp hơn → đánh thức kswapd) và `HIGH` (cao hơn → kswapd ngủ). `MIN` là đáy: dưới `MIN` task thường phải tự reclaim. `[SOURCE mm/page_alloc.c:8039-8045]`
2. **`priority` 12 → 0:** mỗi lần quét, kernel chỉ quét `list_size >> priority` page. Bắt đầu nhẹ (1/4096), không đủ thì tăng dần áp lực. `[SOURCE vmscan.c:2410, 3075]` (số liệu ví dụ: `tools/vmscan_numbers.py`).
3. **Batch = 32 page** (`SWAP_CLUSTER_MAX`). Mọi thứ trong vmscan đi từng lô 32. `[SOURCE include/linux/swap.h:183]`
4. **Reclaim ≠ scan.** *Scan* = số page đã xem; *reclaim* = số page thật sự giải phóng. Tỉ lệ `reclaim/scan` là chỉ số sức khoẻ số 1 để debug. `[GENERAL + SOURCE vmscan.c:1973-1994]`

## 1.4. Điều quan trọng nhất cho **board này**

| Phát hiện | Hệ quả khi debug |
|---|---|
| `vmscan.c` gần như **im lặng**: chỉ 3 câu `pr_*` + vài `WARN`/`BUG` `[SOURCE bảng ở file 05]` | Thông tin debug nằm ở **counter `/proc/vmstat`**, không phải ở log. |
| `# CONFIG_FTRACE is not set`; **không** defconfig nào (kể cả 15 file board) có `CONFIG_TRACING=y` hay `CONFIG_TRACEPOINTS=y` `[SOURCE grep configs/*]` | Các `trace_mm_vmscan_*` **nhiều khả năng là no-op / không tồn tại** lúc chạy `[INFERENCE]` — xác nhận bằng `ls /sys/kernel/debug/tracing/events/vmscan`. |
| `# CONFIG_PSI is not set` | Không có `/proc/pressure/memory`. |
| `# CONFIG_DETECT_HUNG_TASK`, `# CONFIG_SOFTLOCKUP_DETECTOR` **not set** | Task kẹt trong reclaim **không tự sinh log** "blocked for more than 120 seconds". |
| `CONFIG_NUMA` không set | Chỉ có **1 node** → **1 kswapd** (`kswapd0`); vùng `node_reclaim` (dòng 4116-4293) **không được biên dịch**. |
| `CONFIG_SWAP=y` nhưng **không có** `ZRAM`/`ZSWAP` | Nếu không có swap device, **anon page không thể reclaim** `[KIỂM TRA TRÊN BOARD: cat /proc/swaps]`. |
| `CONFIG_TRANSPARENT_HUGEPAGE_ALWAYS=y` | Path "split THP" trong `shrink_page_list` có thật; allocation order 9 là *costly*. |
| **VENDOR:** `kswapd` chạy `SCHED_RR` priority **200** + `sched_rt_runtime = 1000000` (**tắt RT throttling**) | kswapd quay vòng có thể **chiếm trọn 1 CPU** mà không bị throttle (xem 1.6). |
| **VENDOR:** `CONFIG_PM_WARP` thêm 2 điểm `break` | Reclaim bị **huỷ nhanh** khi warp (hibernate) bị cancel. |

## 1.5. Kernel & môi trường (đã verify)

| Mục | Giá trị | Nguồn |
|---|---|---|
| Kernel | **5.10.241** (`VERSION=5, PATCHLEVEL=10, SUBLEVEL=241`) | `[SOURCE Src/Makefile:2-4]` |
| Kiến trúc | arm64, **4 KB page**, `NR_CPUS=4`, `SMP`, `PREEMPT`, `HZ=200` | `[SOURCE configs/Eagle_defconfig:252,272,399,85,410]` |
| SoC / máy | Marvell Armada 8K/AP806 trong máy in Konica Minolta bizhub S800 | `[SOURCE CONTEXT_INDEX.md, KERNEL_STUDY_CONTEXT.md]` |
| Zone | `ZONE_DMA`, `ZONE_DMA32` (+Normal); không highmem trên arm64 | `[SOURCE Eagle_defconfig:268-269]` |
| NUMA / CMA / hotplug | **tắt** | `[SOURCE Eagle_defconfig:402,828,812]` |
| COMPACTION / MIGRATION / THP | **bật** (`THP=always`) | `[SOURCE Eagle_defconfig:814,816,823-824]` |
| MEMCG / SWAP | **bật** (`MEMCG_SWAP`, `MEMCG_KMEM`); **không** ZSWAP/ZRAM | `[SOURCE Eagle_defconfig:33,145-147]` |
| `CGROUP_WRITEBACK`, `BLK_CGROUP` | **không** | `[SOURCE grep 15 defconfig]` |
| Debug | `DEBUG_FS=y`, `DEBUG_KERNEL=y`, `PROC_PAGE_MONITOR=y`, `DEBUG_VM` **off**, `FTRACE` **off**, `PSI` **off**, `MAGIC_SYSRQ` off (15/15 board), `TASKSTATS` off | `[SOURCE Eagle_defconfig:4700,4711,4235,4734,4834,102,101]` |
| Vendor | `KM_BIZHUB=y`, `HIGH_PRIORITY=y`, `PM_WARP=y` (`PM_WARP_SHRINK=1`) trong **15** defconfig board | `[SOURCE grep configs/*]` |

> **Phạm vi kiểm tra config:** các giá trị trên đọc từ `Eagle_defconfig` rồi **đối chiếu lại với cả 15 defconfig có `CONFIG_KM_BIZHUB=y`** (`DenebMLK, EagleBK, EagleZBKp, EagleZBK, EagleZp, EagleZ, Eagle, HeliosMLK, km_mvebu_v8_lsp{,_dnb,_eglz,_spa}, MinervaSSBK, SparrowBK, Sparrow`): tất cả đều khớp (kể cả `MAGIC_SYSRQ` off ở **15/15** board, `HIGH_PRIORITY=y`, `PM_WARP=y`, `THP=always`, `FTRACE` off, `DETECT_HUNG_TASK` off, `PERF_EVENTS` off, `TASKSTATS` off, `DYNAMIC_DEBUG` off, `CMEMDRV=y`, `NR_CPUS=4`). Ngoại lệ về *hình thức*: `km_mvebu_v8_lsp_eglz_defconfig` (3624 dòng, ngắn hơn) **không có dòng** `CONFIG_SWAP`, `# CONFIG_NUMA`, `# CONFIG_PSI`, `# CONFIG_SOFTLOCKUP_DETECTOR` — vì file ngắn hơn nên symbol vắng sẽ lấy **mặc định Kconfig** (`SWAP` default `y` `[init/Kconfig:350-353]`; NUMA/PSI/SOFTLOCKUP default `n`), tức là hiệu lực vẫn như các board còn lại `[INFERENCE]`. Cấu hình hiệu lực cuối cùng của ảnh đang chạy: `zcat /proc/config.gz` nếu có.
>
> **RAM size của board:** tài liệu này **không biết** (không đọc DTS board S800). Mọi ví dụ "1 GiB RAM" là **giả định minh hoạ** do bạn đặt ra trong đề bài, không phải số đo.

## 1.6. Vì sao kswapd `SCHED_RR 200` + "RT throttling tắt" là chi tiết nguy hiểm

- `kswapd()` gọi `sched_setscheduler(current, SCHED_RR, {.sched_priority = 200})` **VENDOR** `[SOURCE vmscan.c:3901-3904]`. Giá trị trả về **bị bỏ qua**.
- Với kernel gốc, `MAX_RT_PRIO = 100` ⇒ priority 200 sẽ bị từ chối (`-EINVAL`) và kswapd lặng lẽ ở lại `SCHED_NORMAL`. Nhưng KM nâng giới hạn: `CONFIG_HIGH_PRIORITY=y` ⇒ `MAX_USER_RT_PRIO = 512` `[SOURCE include/linux/sched/prio.h:26]` (kiểm tra ở `core.c:5280-5281`: kernel thread dùng `MAX_RT_PRIO-1`). Vì `HIGH_PRIORITY=y` ở 15 defconfig, lời gọi **thành công**: kswapd là **real-time thread**.
- Thêm nữa, KM đặt `sysctl_sched_rt_runtime = 1000000` (= `sched_rt_period`) `[SOURCE kernel/sched/core.c:82-87]`; trong `sched_rt_runtime_exceeded()` điều kiện `runtime >= period → return 0` `[SOURCE kernel/sched/rt.c]` ⇒ **RT task không bao giờ bị throttle**.
- **Hệ quả `[INFERENCE]`:** nếu kswapd rơi vào vòng "quét nhiều – thu hồi ít" nhưng **mỗi lần chạy vẫn thu được vài page** (nên `kswapd_failures` bị reset về 0, xem file 02), nó sẽ ăn CPU như một RT thread không bị giới hạn, lấn át mọi task `SCHED_NORMAL` (và RT priority thấp hơn 200) trên core đó. Đây là kịch bản "**kswapd CPU cao + latency tăng**" mà đề bài hỏi — sẽ phân tích chi tiết ở file 05.
- Cần kiểm tra trên board: `chrt -p $(pidof kswapd0)` (nếu có `chrt`) hoặc `cat /proc/$(pidof kswapd0)/sched` — **`SCHED_DEBUG` off** nên có thể không có file `sched`; dùng `ps -o pid,cls,rtprio,comm`.

---

# 20. Version Accuracy (đặt ở đây vì ảnh hưởng mọi thứ phía sau)

## 20.1. Phương pháp

1. Đọc **toàn bộ 4351 dòng** `Src/mm/vmscan.c`.
2. Tải `mm/vmscan.c` của tag **`v5.10.241`** từ `git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git` (4332 dòng) và chạy `diff -u upstream local`.
3. Đối chiếu config board (`arch/arm64/configs/*_defconfig`) để biết đoạn nào thực sự được biên dịch.

## 20.2. Kết quả `[DIFF]`: local = upstream 5.10.241 + **đúng 4 hunk vendor** (+19 dòng)

| # | Vị trí local | Nội dung | Gắn với | Ý nghĩa |
|---|---|---|---|---|
| V1 | `vmscan.c:61-64` | `#include <linux/sched.h>` + `<uapi/linux/sched/types.h>` | `CONFIG_KM_BIZHUB` | Để dùng `struct sched_param` cho V4. |
| V2 | `vmscan.c:1110-1113` (trong vòng `while` của `shrink_page_list`) | `if (pm_device_down && warp_canceled) break;` | `CONFIG_PM_WARP` | Đang xử lý list page mà warp bị huỷ ⇒ **thoát vòng lặp** sớm. Các page chưa xử lý được `list_splice` trả về LRU như bình thường (vì `break` ra khỏi `while`, đoạn dọn dẹp cuối hàm vẫn chạy). |
| V3 | `vmscan.c:3071-3074` (trong vòng priority của `do_try_to_free_pages`) | cùng điều kiện, `break` | `CONFIG_PM_WARP` | Dừng leo thang priority khi warp bị huỷ. |
| V4 | `vmscan.c:3901-3904` (đầu `kswapd()`) | `sched_setscheduler(current, SCHED_RR, {prio=200})` | `CONFIG_KM_BIZHUB` | kswapd thành RT thread (xem 1.6). |

**Bối cảnh V2/V3:** `pm_device_down` và `warp_canceled` được định nghĩa ở `kernel/power/warp.c:293,298` (`extern` ở `include/linux/warp.h:20,25`). "WARP" là cơ chế **hibernate/snapshot khởi động nhanh** của KM. `warp_shrink_memory()` gọi `shrink_all_memory(SHRINK_BITE)` **lặp** (`repeat` lần; tối đa `WARP_SHRINK_REPEAT_P1 = 10000` khi swapout bật) để giảm kích thước image trước khi snapshot `[SOURCE kernel/power/warp.c:1734-1768]`. Khi warp bị huỷ giữa chừng (`warp_canceled = 1`), V2/V3 giúp thoát khỏi reclaim ngay thay vì quét nốt. Điều này suy ra từ code; **không có tài liệu KM** xác nhận ý đồ `[INFERENCE]`.

## 20.3. Những thứ trông "lạ" nhưng thực ra là **UPSTREAM 5.10.241**

| Chỗ | Vì sao dễ nhầm là vendor | Sự thật |
|---|---|---|
| `zone_reclaimable_pages()` có nhánh `if (nr == 0) nr = NR_FREE_PAGES` (`vmscan.c:313-320`) | Comment nhắc "DMA32", nghe như patch riêng | **UPSTREAM** (không xuất hiện trong diff). Đây là bản backport stable. |
| Dùng `gfp_compaction_allowed()` (`vmscan.c:2566, 2893`) | Hàm này mới hơn 5.10 gốc | **UPSTREAM 5.10.241** (định nghĩa ở `include/linux/gfp.h:630`, không phải patch KM). |
| `printk`-style comment tiếng Nhật ở `page_alloc.c:4018-4051` (`OP_BTS-18784`, `OP_BTS-7620`) | Có chữ Shift-JIS | Nằm trong `#if 0` ⇒ **không có tác dụng**; không thuộc `vmscan.c`. |

> ⚠️ **Hai thay đổi ngoài `vmscan.c` nhưng tác động mạnh tới reclaim:** (a) `sysctl_sched_rt_runtime = 1000000` (`kernel/sched/core.c:82-87`), (b) `MAX_USER_RT_PRIO = 512` khi `HIGH_PRIORITY` (`include/linux/sched/prio.h:26`). Cả hai là **VENDOR**.

## 20.4. 5.10.241 **khác** các kernel mới hơn như thế nào (đừng áp bài blog 6.x vào đây)

Đã xác nhận bằng cách đọc cả file — những thứ sau **không tồn tại** trong source local:

| Tính năng kernel mới | Có ở local? | Cái thay thế ở 5.10 |
|---|---|---|
| `struct folio` | ❌ | Mọi thứ dùng `struct page` (kể cả THP: `compound_nr`, `thp_nr_pages`) |
| MGLRU (`CONFIG_LRU_GEN`, `lru_gen_*`) | ❌ | LRU cổ điển 5 list (active/inactive × anon/file + unevictable) |
| `reclaim_throttle()` / `VMSCAN_THROTTLE_*` / tracepoint `mm_vmscan_throttled` | ❌ (xuất hiện ≥ 5.16) | `congestion_wait()` / `wait_iff_congested()` / `msleep(100)` trong `shrink_inactive_list` |
| `memory.reclaim` (proactive reclaim cgroup v2) | ❌ | Không có; *proactive reclaim* duy nhất là `MADV_PAGEOUT` → `reclaim_pages()` (`vmscan.c:2124`) |
| `lru_lock` theo từng lruvec | ❌ | **`pgdat->lru_lock` — 1 spinlock cho cả node** (`vmscan.c:1794, 1965`) |
| `wait_iff_congested` bị gỡ | ❌ (còn nguyên) | `vmscan.c:2867` |

## 20.5. Phần nào của file **được biên dịch** trên board

| Vùng trong file | Điều kiện | Có chạy trên board? |
|---|---|---|
| Infrastructure memcg (`prealloc_memcg_shrinker`, `shrink_slab_memcg`, `mem_cgroup_shrink_node`, `try_to_free_mem_cgroup_pages`) | `CONFIG_MEMCG` | **Có** (`MEMCG=y`) |
| `shrink_all_memory` (4028-4065) | `CONFIG_HIBERNATION` | **Có** (`HIBERNATION=y`; WARP dùng) |
| `node_reclaim` & sysctl `zone_reclaim_mode` (4116-4293) | `CONFIG_NUMA` | **Không** (NUMA off) |
| `kswapd_run`/`kswapd_stop` | luôn | Chỉ `kswapd_run` lúc boot |
| Tracepoint `trace_*` | `CONFIG_TRACEPOINTS` | **Nhiều khả năng không** `[INFERENCE]` |
| `VM_BUG_ON_PAGE` | `CONFIG_DEBUG_VM` | **Không** (compile-out) |
| `psi_memstall_enter/leave` | `CONFIG_PSI` | no-op |
| Hunk V1/V4 | `KM_BIZHUB` | Có |
| Hunk V2/V3 | `PM_WARP` | Có |

Tiếp theo → [`01_concepts_architecture_big_flow.md`](01_concepts_architecture_big_flow.md).
