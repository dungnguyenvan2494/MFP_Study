# 01 — Khái niệm nền, kiến trúc và BIG FLOW của `vmscan.c`

> Phủ các mục: **02** Memory reclaim là gì · **03** Tại sao cần `vmscan.c` · **04** Overall Architecture · **05** BIG FLOW · **06** Reclaim Concepts.
> Quy ước thẻ: xem [`00_index_executive_summary_version.md`](00_index_executive_summary_version.md).

---

# 02. Memory reclaim là gì? (cho người chưa biết Linux kernel)

## 2.1. Bài toán thực tế

```
RAM có giới hạn (vd. 1 GiB)
        │
        ▼
Process/driver/kernel liên tục xin thêm RAM
        │
        ▼
Số page TRỐNG (free) giảm dần
        │
        ▼
Đến một lúc: không còn page trống để cấp
        │
        ▼
Kernel phải "lấy lại" những page đang được giữ nhưng giá trị thấp   ← đây là RECLAIM
        │
        ▼
Có thêm page trống → allocation tiếp tục
```

**Ẩn dụ (và buộc nó về cơ chế thật):** RAM là *mặt bàn làm việc*, đĩa/flash là *kho*. Mặt bàn nhỏ. Khi hết chỗ:

| Trên bàn | Tương ứng trong kernel | Cách dọn |
|---|---|---|
| Bản photo của tài liệu có sẵn trong kho | **page cache sạch** (clean file page) | Vứt đi, cần thì photo lại (đọc lại từ đĩa) — **rẻ nhất** |
| Ghi chú bạn mới sửa tay lên bản photo | **page cache bẩn** (dirty file page) | Phải *cất vào kho trước* (writeback) rồi mới vứt |
| Bản nháp chỉ tồn tại trên bàn, không có bản gốc trong kho | **anonymous memory** (heap, stack, `malloc`) | Chỉ dọn được nếu có *chỗ cất* (swap). **Không có swap = không dọn được** |
| Đồ đang cầm trên tay / dán chặt vào bàn | page **pinned / mlocked / đang bị DMA** | Không đụng vào |

Ẩn dụ chỉ đúng tới đây. Phần "ai đi tìm đồ để dọn, dọn theo thứ tự nào, khi nào dừng" là phần cơ chế thật mà các mục sau mô tả.

## 2.2. Các thuật ngữ nền (mỗi thuật ngữ giải thích ngay lần đầu)

| Thuật ngữ | Giải thích đơn giản | Trên board này |
|---|---|---|
| **Physical memory (RAM)** | Chip nhớ thật. Kernel chia nó thành các khối bằng nhau gọi là *page*. | LPDDR4 |
| **Page** | Đơn vị quản lý RAM, **4 KiB** (`ARM64_PAGE_SHIFT=12`). Mỗi page vật lý có một `struct page` mô tả trạng thái. | `[SOURCE Eagle_defconfig:252]` |
| **Free page** | Page chưa ai dùng, nằm trong *buddy allocator* chờ được cấp. | |
| **Anonymous memory** ("anon") | Bộ nhớ **không có file đứng sau**: `malloc`, stack, heap. Nội dung chỉ tồn tại trong RAM. | |
| **Page cache** ("file page") | RAM dùng để **giữ bản sao dữ liệu file** (đọc/ghi file, `mmap` file, chạy chương trình). | |
| **Clean page** | Nội dung **giống hệt** bản trên đĩa → bỏ đi không mất dữ liệu. | |
| **Dirty page** | Đã bị sửa, **chưa ghi** xuống đĩa → bỏ đi là mất dữ liệu. | |
| **Writeback** | Page đang được ghi xuống đĩa (I/O đang bay). Không thể free lúc này. | |
| **Mapped page** | Page đang được **một hoặc nhiều process map** vào không gian địa chỉ của nó (qua page table). Muốn free phải gỡ mọi PTE (`try_to_unmap`). | |
| **Unmapped page** | Chỉ có page cache giữ, không process nào map → free dễ hơn. | |
| **Referenced** | CPU đã *truy cập* page gần đây (bit Accessed trong PTE / cờ `PG_referenced`). Dấu hiệu "page đang được dùng". | |
| **Reclaimable page** | Page mà kernel có thể giải phóng được (sạch, hoặc bẩn nhưng ghi được, hoặc anon có swap). | |
| **Unevictable** | `mlock`, ramfs, shm bị lock… — không bao giờ reclaim (nằm ở list riêng). | |
| **Pinned** | Có ref ngoài (`get_user_pages`, DMA) ⇒ không free được; vmscan thấy bằng `page_maybe_dma_pinned`/refcount thừa. | |
| **Slab** | Cache các *object nhỏ của kernel* (dentry, inode…). Không nằm trong LRU; thu hồi qua *shrinker*. | |
| **shmem / tmpfs** | Trông như file nhưng **nội dung chỉ ở RAM** ⇒ kernel xếp vào LRU *anon* (vì `PageSwapBacked`). `page_is_file_lru()` = `!PageSwapBacked` `[SOURCE include/linux/mm_inline.h:22-25]`. | tmpfs bật (`CONFIG_TMPFS=y`) |

## 2.3. Tại sao kernel không "xoá bừa một page"?

Mỗi loại page **bắt buộc** có một nghi thức trước khi giải phóng:

| Loại page | Nghi thức bắt buộc | Nếu bỏ qua |
|---|---|---|
| Clean, unmapped file page | Tháo khỏi page cache (kèm *shadow entry* để biết sau này có refault) | — (trường hợp dễ nhất) |
| Clean, **mapped** file page | **Gỡ PTE** của mọi process (`try_to_unmap`) rồi mới tháo | Process truy cập → dùng page đã bị cấp cho người khác → **hỏng dữ liệu** |
| **Dirty** file page | Ghi xuống đĩa (hoặc chờ flusher), đợi hoàn tất, rồi mới free | **Mất dữ liệu người dùng** |
| **Anon** page | Cấp slot swap → ghi page ra swap → gỡ PTE (thay bằng swap entry) → free | Mất dữ liệu process → process crash/sai |
| Page đang **writeback** | Chờ (hoặc bỏ qua, quay lại sau) | Free khi DMA đang đọc → corrupt |
| Page **pinned** | Không được free | use-after-free |

Vì thế `shrink_page_list()` (hàm quyết định số phận từng page) là một cây quyết định dài, không phải một lệnh `free()`.

## 2.4. Tại sao phải *scan* (quét)?

Kernel **không có sẵn danh sách "page nào lạnh nhất"**. Phần cứng CPU chỉ cho một thông tin rất thô: bit *Accessed* trong PTE ("page này đã bị chạm kể từ lần xoá bit trước"). Muốn biết page nào ít dùng, kernel phải **nhìn vào page rồi hỏi** (qua *rmap* — reverse mapping — hỏi mọi PTE trỏ tới page đó). Đó là *scan*.

Quét mọi page mỗi lần thì quá đắt (hàng trăm nghìn page). Giải pháp: **chỉ quét một phần nhỏ, phần ở "đuôi" danh sách** — đó là lý do có LRU và `priority`.

## 2.5. Tại sao có LRU và tại sao chia **active / inactive**?

- **LRU = Least Recently Used.** Kernel giữ các page trên danh sách sắp theo *độ mới* truy cập. Đầu list = mới dùng; **đuôi list = lâu chưa dùng ⇒ ứng viên reclaim**. Việc chọn ứng viên chỉ là "lấy từ đuôi" — O(1), không cần tìm.
- Chỉ một list thì **dễ bị phá bởi dữ liệu dùng một lần** (ví dụ copy một file 2 GiB: mỗi page chỉ đọc đúng một lần nhưng sẽ đẩy hết page "nóng" ra khỏi RAM).
- Nên chia **hai list**: *inactive* (vừa vào / chưa chứng minh được là hot) và *active* (đã được dùng lặp lại). Page phải được dùng **hai lần** mới lên active; reclaim chỉ ăn từ **đuôi inactive**. Đây là *second chance* — chi tiết ở file 03.

---

# 03. Tại sao Linux cần `vmscan.c`?

## 3.1. Ba nhiệm vụ

| # | Nhiệm vụ | Hàm tiêu biểu |
|---|---|---|
| 1 | **Chọn và giải phóng** page (kể cả slab) một cách an toàn | `shrink_page_list`, `shrink_lruvec`, `shrink_slab` |
| 2 | **Điều phối**: bao nhiêu, ở node/zone/memcg nào, mạnh cỡ nào, khi nào dừng, khi nào leo thang | `shrink_node`, `do_try_to_free_pages`, `balance_pgdat`, `get_scan_count` |
| 3 | **Giữ cho hệ thống không sụp**: ngăn bão reclaim, throttle, đảm bảo có tiến triển | `throttle_direct_reclaim`, `too_many_isolated`, `kswapd_failures` |

## 3.2. Ranh giới trách nhiệm

```
page_alloc.c  :  "KHI NÀO thiếu?  AI gọi?  Sau reclaim thì thử cấp lại / compaction / OOM?"
vmscan.c      :  "LẤY page ở ĐÂU?  THEO THỨ TỰ NÀO?  Lấy được bao nhiêu?"
oom_kill.c    :  "Reclaim đã thất bại hẳn → giết ai?"
```

## 3.3. Nếu **không có** `vmscan.c`

```
Free memory chạm đáy ──► allocation thất bại ──► OOM kill ngay lập tức
(trong khi hàng trăm MiB page cache sạch đang nằm đó chẳng ai dùng)
```

Kernel sẽ giết process dù có thể thu hồi cache dễ dàng. Linux **cố tình dùng RAM trống làm cache** (nhanh), và dựa vào `vmscan.c` để "trả" cache lại khi cần.

---

# 04. Overall Architecture — phân rã `vmscan.c` (đúng với 5.10.241)

> Đã kiểm tra source local và điều chỉnh taxonomy theo **đúng version** (không dùng cấu trúc của kernel mới).

```
mm/vmscan.c  (4351 dòng, 5.10.241 + 4 hunk vendor)
│
├── A. Reclaim control & hạ tầng chung                         dòng  71-297
│      struct scan_control · vm_swappiness · set_task_reclaim_state
│      cgroup_reclaim · writeback_throttling_sane
│
├── B. Slab / Shrinker framework (thu hồi cache KHÔNG qua LRU)  dòng 194-734
│      register_shrinker · do_shrink_slab · shrink_slab(_memcg) · drop_slab(_node)
│
├── C. Reclaim TỪNG page (quyết định số phận)                   dòng 736-1553
│      pageout · __remove_mapping/remove_mapping · putback_lru_page
│      page_check_references · page_check_dirty_writeback
│      ★ shrink_page_list · reclaim_clean_pages_from_list
│
├── D. LRU isolation / putback (lấy page ra khỏi / trả về LRU)  dòng 1565-1917, 4304
│      __isolate_lru_page · isolate_lru_pages · isolate_lru_page
│      too_many_isolated · move_pages_to_lru · check_move_unevictable_pages
│
├── E. LRU scanning & cân bằng anon/file                        dòng 1937-2561
│      ★ shrink_inactive_list · shrink_active_list · shrink_list
│      inactive_is_low · ★ get_scan_count · ★ shrink_lruvec · reclaim_pages
│
├── F. Điều phối theo node / memcg / zone                       dòng 2564-3021
│      in_reclaim_compaction · should_continue_reclaim · compaction_ready
│      shrink_node_memcgs · ★ shrink_node · shrink_zones · snapshot_refaults
│
├── G. DIRECT reclaim                                           dòng 3039-3387
│      ★ do_try_to_free_pages · allow_direct_reclaim · throttle_direct_reclaim
│      ★ try_to_free_pages · try_to_free_mem_cgroup_pages · mem_cgroup_shrink_node
│
├── H. kswapd / BACKGROUND reclaim                              dòng 3389-4112
│      age_active_anon · pgdat_balanced · pgdat_watermark_boosted · prepare_kswapd_sleep
│      kswapd_shrink_node · ★ balance_pgdat · kswapd_try_to_sleep · ★ kswapd
│      ★ wakeup_kswapd · kswapd_run/stop/init
│
├── I. Chế độ reclaim đặc biệt                                  dòng 4028-4293
│      shrink_all_memory (hibernation / PM_WARP)   ← ĐƯỢC build
│      node_reclaim (NUMA)                         ← KHÔNG build (NUMA off)
│
└── J. Thống kê / debug / tracing (rải khắp file)
       count_vm_event(s) · trace_mm_vmscan_* · vmpressure() · psi_memstall_* ·
       delayacct_freepages_* · pr_err/pr_info/WARN/BUG   (chi tiết ở file 05)
```
(★ = hàm cốt lõi, sẽ phân tích kỹ ở file 04.)

## 4.1. Hàm public (nối `vmscan.c` với phần còn lại của kernel)

| API (định nghĩa trong vmscan.c) | Ai gọi `[SOURCE grep]` | Mục đích |
|---|---|---|
| `wakeup_kswapd()` | `page_alloc.c:3512, 4465` | Đánh thức kswapd |
| `try_to_free_pages()` | `page_alloc.c:4411` (`__perform_reclaim`) | **Direct reclaim** |
| `try_to_free_mem_cgroup_pages()` | `memcontrol.c:2443, 2744, 3381, 3514, 6378, 6427` | Reclaim khi memcg chạm limit/high |
| `mem_cgroup_shrink_node()` | `memcontrol.c:1782` | Soft-limit reclaim |
| `zone_reclaimable_pages()` | `page_alloc.c:4605`, `compaction.c:2197` | Ước lượng "còn bao nhiêu page có thể reclaim" |
| `shrink_all_memory()` | `kernel/power/snapshot.c:1786`, `kernel/power/warp.c:1760` | Hibernate / **PM_WARP** |
| `reclaim_pages()` | `madvise.c:385, 476` | `MADV_PAGEOUT` (reclaim chủ động theo yêu cầu user) |
| `reclaim_clean_pages_from_list()` | `page_alloc.c:8602` | `alloc_contig_range` (CMA — **off** trên board) |
| `isolate_lru_page()`, `putback_lru_page()`, `__isolate_lru_page()` | `migrate.c`, `compaction.c`, `mlock.c`… | Migration/compaction/mlock mượn cơ chế LRU |
| `remove_mapping()` | `truncate.c:212`, `splice.c:79` | Gỡ page khỏi page cache có kiểm tra |
| `check_move_unevictable_pages()` | `shmem.c:868` (+ driver GPU) | Cứu page khỏi list unevictable |
| `drop_slab()` | `fs/drop_caches.c:65` | `echo 2/3 > /proc/sys/vm/drop_caches` |
| `register_shrinker()` | mọi subsystem có cache (fs, …) | Đăng ký shrinker |
| `kswapd_run()` | `kswapd_init()` lúc boot; `memory_hotplug.c:852` | Tạo thread |

---

# 05. BIG FLOW của toàn bộ `vmscan.c`

## 5.1. Sơ đồ duy nhất (đúng 5.10.241, có tên hàm thật)

```
╔════════════════════════════════════════════════════════════════════════════════════╗
║ STAGE 1 ─ MEMORY PRESSURE                                                          ║
║   (a) free(zone) < watermark   (b) memcg vượt limit   (c) fragmentation boost      ║
║   (d) yêu cầu tường minh: drop_caches / hibernate-WARP / MADV_PAGEOUT              ║
╚══════════════════════════════════╤═════════════════════════════════════════════════╝
                                   │   (phát hiện ở mm/page_alloc.c, memcontrol.c, …)
                                   ▼
╔════════════════════════════════════════════════════════════════════════════════════╗
║ STAGE 2 ─ RECLAIM TRIGGER                                                          ║
║                                                                                    ║
║  [BẤT ĐỒNG BỘ — nền]                         [ĐỒNG BỘ — ngay trong task xin RAM]   ║
║  wakeup_kswapd()  ──► kswapd0                try_to_free_pages()                    ║
║                       kswapd()                 ├ throttle_direct_reclaim()          ║
║                        └ balance_pgdat()       └ do_try_to_free_pages()             ║
║                                              (memcg: try_to_free_mem_cgroup_pages)  ║
╚══════════════════╤═══════════════════════════════════════════╤═════════════════════╝
                   │                                           │
                   ▼                                           ▼
╔════════════════════════════════════════════════════════════════════════════════════╗
║ STAGE 3 ─ CHỌN VÙNG (node / zone / memcg)                                          ║
║   kswapd :  kswapd_shrink_node(pgdat)       direct :  shrink_zones(zonelist)       ║
║             reclaim_idx = highest_zoneidx             zone ≤ reclaim_idx, cpuset,  ║
║             nr_to_reclaim = Σ high_wmark              1 lần / node, nr_to_reclaim=32║
║                          └──────────────┬──────────────┘                           ║
║                                         ▼                                          ║
║                               shrink_node(pgdat, sc)                               ║
║                    (đặt sc: may_deactivate, cache_trim_mode, file_is_tiny, cost)   ║
║                                         ▼                                          ║
║                               shrink_node_memcgs()  (bỏ qua memcg được bảo vệ)     ║
╚══════════════════════════════════╤═════════════════════════════════════════════════╝
                      ┌────────────┴─────────────┐
                      ▼                          ▼
        ┌──────────────────────────┐   ┌──────────────────────────┐
        │ shrink_lruvec(lruvec)    │   │ shrink_slab(memcg)       │
        │  (page cache + anon)     │   │  do_shrink_slab(...)     │
        └─────────────┬────────────┘   │  dentry/inode/…          │
                      │                └──────────────────────────┘
                      ▼
╔════════════════════════════════════════════════════════════════════════════════════╗
║ STAGE 4 ─ CHỌN LRU                                                                 ║
║   get_scan_count() → nr[]: {anon-inactive, anon-active, file-inactive, file-active}║
║   (anon hay file? dựa vào: có swap?, swappiness, file_is_tiny, cache_trim_mode,    ║
║    chi phí refault anon_cost/file_cost, priority)                                  ║
╚══════════════════════════════════╤═════════════════════════════════════════════════╝
                                   ▼   lặp theo LÔ 32 page (SWAP_CLUSTER_MAX)
╔════════════════════════════════════════════════════════════════════════════════════╗
║ STAGE 5 ─ SCAN                                                                     ║
║   shrink_list(lru, 32)                                                             ║
║    ├─ lru = ACTIVE   → shrink_active_list : active → inactive (CHƯA free gì cả)    ║
║    └─ lru = INACTIVE → shrink_inactive_list                                        ║
║           isolate_lru_pages()  lấy ≤32 page từ ĐUÔI list (dưới pgdat->lru_lock)    ║
╚══════════════════════════════════╤═════════════════════════════════════════════════╝
                                   ▼
╔════════════════════════════════════════════════════════════════════════════════════╗
║ STAGE 6 ─ EVALUATE & RECLAIM  (shrink_page_list, từng page)                        ║
║                                                                                    ║
║   page ─► evictable? ─► may_unmap? ─► writeback? ─► referenced? ─► (anon: swap)    ║
║        ─► mapped: try_to_unmap ─► dirty: pageout/skip ─► buffers ─► __remove_mapping║
║                                                                                    ║
║        ┌─────────────┬──────────────────┬─────────────────┬───────────────────┐    ║
║        ▼             ▼                  ▼                 ▼                   ▼    ║
║      FREE         KEEP(xoay)         ACTIVATE        WRITE→chờ        "skip/lock"  ║
║  free_unref_    về inactive        sang active      (pageout)         trylock fail ║
║  page_list      (move_pages_to_lru)                                                 ║
╚══════════════════════════════════╤═════════════════════════════════════════════════╝
                                   ▼
╔════════════════════════════════════════════════════════════════════════════════════╗
║ STAGE 7 ─ ĐÁNH GIÁ TIẾN ĐỘ                                                         ║
║   sc->nr_reclaimed ≥ nr_to_reclaim ?   should_continue_reclaim() ?  (high-order)   ║
║   direct : --sc->priority  (12 → 0)       kswapd : pgdat_balanced() ? priority 12→1║
║   kswapd_failures++ / = 0                 vmpressure(), snapshot_refaults()        ║
╚══════════════╤══════════════════════════════════════════════════════╤══════════════╝
               │ ĐỦ                                                    │ KHÔNG ĐỦ
               ▼                                                       ▼
        MEMORY RECOVERED                         Leo thang: tăng priority → ... → trả 0
        (allocator thử cấp lại)                  → page_alloc.c: compaction / retry / OOM
```

## 5.2. Giải thích từng stage

### Stage 1 — Memory Pressure

**"Memory pressure" là gì?** Là trạng thái mà *nhu cầu xin page vượt quá lượng page trống* theo tiêu chuẩn của kernel.

**Kernel biết bằng cách nào?** So sánh `NR_FREE_PAGES` của từng **zone** với ba **watermark** (`MIN < LOW < HIGH`):

```
 free pages
   ▲
   │  ········································  HIGH   ← kswapd ngủ khi vượt mức này
   │  ·············  ·····················   
   │  ········································  LOW    ← dưới mức này: ĐÁNH THỨC kswapd
   │
   │  ········································  MIN    ← dưới mức này: allocation thường phải TỰ reclaim
   │
   └──────────────────────────────────────────────►
```

Con số cụ thể cho 1 zone ~977 MiB (giả định minh hoạ, từ `tools/vmscan_numbers.py`):

| Watermark | pages | MiB |
|---|---|---|
| MIN | 1000 | 3.91 |
| LOW | 1250 | 4.88 |
| HIGH | 1500 | 5.86 |

(công thức: `min_free_kbytes = sqrt(lowmem_kbytes × 16)`, `LOW = MIN + max(MIN/4, managed × 10/10000)`, `HIGH = MIN + 2×(…)` `[SOURCE page_alloc.c:8094-8100, 8039-8045]`; `vm.watermark_scale_factor` mặc định 10 `[SOURCE page_alloc.c:369]`.)

**Chi tiết quan trọng — vì sao allocation giữa LOW và MIN không bị chậm:**
Đường nhanh (`__alloc_pages_nodemask`) dùng `ALLOC_WMARK_LOW` `[SOURCE page_alloc.c:5036]`. Nếu fail, vào `__alloc_pages_slowpath`, ở đó `alloc_flags` bắt đầu bằng `ALLOC_WMARK_MIN` `[SOURCE page_alloc.c:4473]` và **đánh thức kswapd trước** (`wake_all_kswapds`, dòng 4753-4754), rồi **thử cấp lại** với watermark MIN (dòng 4760). Vì vậy:

```
free ∈ (MIN, LOW)  : fast path FAIL → wake kswapd → retry với MIN → THÀNH CÔNG (không direct reclaim)
free < MIN         : retry cũng FAIL → vào __alloc_pages_direct_reclaim (task tự reclaim)
```

**Ngoài watermark, "pressure" còn đến từ:**
1. **memcg chạm giới hạn** → `try_charge()` gọi `try_to_free_mem_cgroup_pages()` (memcontrol.c).
2. **Fragmentation** (order cao): `boost_watermark()` nâng watermark tạm → kswapd chạy dù RAM chưa cạn `[SOURCE page_alloc.c:2503-2576, 3510-3513]`.
3. **Yêu cầu tường minh:** `drop_caches`, hibernate/**WARP** (`shrink_all_memory`), `MADV_PAGEOUT`.

### Stage 2 — Reclaim được kích hoạt

| Loại | Ai kích hoạt | Chạy ở đâu | Hàm vào | Mục đích thiết kế |
|---|---|---|---|---|
| **Background (kswapd)** | Allocator khi free < LOW (hoặc boost) | Kernel thread `kswapd0` | `wakeup_kswapd → kswapd → balance_pgdat` | Giữ free ≥ HIGH **trước khi** ai đó bị chặn; phục vụ cả context không ngủ được (IRQ/softirq) |
| **Direct reclaim** | Chính task xin page, khi free < MIN | **Context của task đó** (đồng bộ) | `try_to_free_pages → do_try_to_free_pages` | Phao cứu sinh: kswapd không theo kịp thì người xin tự dọn |
| **memcg reclaim** | `try_charge` khi memcg vượt limit | Context task | `try_to_free_mem_cgroup_pages` | Giữ memcg trong giới hạn của nó (không cần RAM toàn máy thấp) |
| **memcg soft-limit reclaim** | `balance_pgdat`/`shrink_zones` | kswapd/task | `mem_cgroup_soft_limit_reclaim → mem_cgroup_shrink_node` | Lấy bớt từ memcg vượt soft limit trước |
| **Hibernate / PM_WARP** | `hibernate_preallocate_memory`, `warp_shrink_memory` | Task suspend | `shrink_all_memory` | Thu nhỏ image snapshot |
| **MADV_PAGEOUT** (≈ *proactive*) | Process (`madvise`) | Task đó | `reclaim_pages` | App chủ động bảo kernel dọn vùng của nó |
| **node_reclaim** | `get_page_from_freelist` khi `zone_reclaim_mode` | Task | `node_reclaim` | **Không build** (NUMA off) |

> **"Proactive reclaim" ở 5.10:** không có `memory.reclaim`; chỉ `MADV_PAGEOUT` và hiệu ứng "boost" của kswapd. `[SOURCE: không có trong vmscan.c]`

**Khác nhau về mục đích thiết kế:**
- *kswapd* tối ưu **throughput & độ trễ của người khác**: dọn nền, không ai phải chờ. Nó được phép làm việc nặng (ghi dirty file page).
- *Direct reclaim* tối ưu **tính đúng đắn / tiến triển**: bảo đảm allocation *cuối cùng* có page, đổi lại **task xin RAM bị chậm**.

### Stage 3 — Chọn vùng memory cần reclaim

| Khái niệm | Ý nghĩa | Trên board |
|---|---|---|
| **Node** | Một cụm RAM gắn với một nhóm CPU (NUMA). Mỗi node có `pg_data_t` (`pgdat`), **LRU nằm ở pgdat** (từ 4.8; không còn per-zone). | **1 node** (NUMA off) |
| **Zone** | Dải địa chỉ vật lý cùng "tính chất": `DMA`, `DMA32`, `Normal` (arm64 không highmem). Mỗi zone có watermark riêng. | DMA/DMA32/Normal |
| **Watermark** | Ngưỡng free MIN/LOW/HIGH của zone. | xem Stage 1 |
| **Zonelist** | Danh sách zone theo thứ tự ưu tiên để thử khi cấp page. Direct reclaim đi qua danh sách này. | |
| **reclaim_idx / highest_zoneidx** | "Zone cao nhất mà allocation này được dùng". Chỉ page ở zone ≤ mức này mới có ích → isolate bỏ qua page ở zone cao hơn (`PGSCAN_SKIP`). `[SOURCE vmscan.c:1697-1701]` | `gfp_zone(GFP_KERNEL)` |
| **Reclaim target (`nr_to_reclaim`)** | Mục tiêu *số page* cần thu về trong một lần chạy. | direct: **32**; kswapd: **Σ high wmark**; memcg: `max(nr_pages,32)` |

Vì sao cần `reclaim_idx`? Ví dụ: allocation `GFP_KERNEL` chỉ dùng được `DMA/DMA32/Normal`. Nếu kernel đi giải phóng page ở zone khác (không dùng được) thì RAM trống tăng nhưng allocation vẫn thất bại → **reclaim vô ích**. `[GENERAL + SOURCE vmscan.c:1703-1712]`

### Stage 4 — Chọn LRU

`shrink_lruvec()` chia ra **4 list** cần quét (anon/file × active/inactive; unevictable không bao giờ quét) và hỏi `get_scan_count()` *quét mỗi list bao nhiêu page*. Quyết định dựa trên: có swap không, `swappiness`, file có quá ít không (`file_is_tiny`), cache sạch dư không (`cache_trim_mode`), chi phí refault, và `priority`. Chi tiết ở **file 03**.

### Stage 5 — Scanning

"Scan" = lấy tối đa 32 page từ **đuôi** list (`isolate_lru_pages`) *gỡ chúng khỏi LRU* (để làm việc ngoài lock) rồi chuyển cho `shrink_page_list`. Scan page *active* khác scan page *inactive*: active chỉ **hạ cấp** (xem lại bit referenced), **không** free. Chi tiết ở **file 03**.

### Stage 6 — Evaluate & Reclaim

Mỗi page trải qua cây quyết định (referenced? mapped? dirty? writeback? pinned?). Kết quả chỉ có 4 loại: **free**, **keep** (xoay về inactive), **activate** (sang active), **write** (ghi rồi giữ lại chờ I/O). Chi tiết ở **file 03**.

### Stage 7 — Đánh giá tiến độ (thêm vào so với sơ đồ gốc của bạn)

Sau mỗi vòng, kernel hỏi *"đã đủ chưa? có tiến triển không?"* — nếu không thì **tăng priority** (quét nhiều hơn) hoặc bỏ cuộc để allocator leo thang (compaction/OOM). Chi tiết ở file 03 (mục 13) và file 02.

---

# 06. Reclaim Concepts — các "biến quyết định" của một lần reclaim

Mỗi lần reclaim mang theo một **`struct scan_control sc`** — "phiếu yêu cầu" mô tả *được làm gì, phải đạt gì*. Đọc `sc` là hiểu quyết định. `[SOURCE vmscan.c:71-161]`

## 6.1. `scan_control` — ý nghĩa từng trường (theo nhóm)

| Nhóm | Trường | Ý nghĩa thực tế |
|---|---|---|
| **Mục tiêu** | `nr_to_reclaim` | Cần thu bao nhiêu page thì dừng. |
| | `priority` (s8) | **Độ mạnh**: quét `size >> priority`. 12 = nhẹ nhất, 0 = quét hết. |
| | `order` | Order của allocation đang xin (0 = 1 page; >0 cần khối liền kề → liên quan compaction). |
| **Phạm vi** | `reclaim_idx` | Zone cao nhất được phép lấy page. |
| | `nodemask`, `target_mem_cgroup` | Giới hạn node / reclaim *chỉ trong subtree memcg này* (`cgroup_reclaim(sc)`). |
| | `gfp_mask` | Ngữ cảnh gọi. `__GFP_FS`/`__GFP_IO` quyết định **có được động vào filesystem/I/O không** (`may_enter_fs`). |
| **Được phép làm gì** | `may_writepage` | Có được ghi dirty page ra đĩa từ reclaim không. |
| | `may_unmap` | Có được reclaim page đang mapped không. |
| | `may_swap` | Có được swap anon không. |
| | `may_deactivate` (bit `DEACTIVATE_ANON/FILE`), `force_deactivate` | Có được hạ cấp active→inactive không. |
| **Cân bằng anon/file** | `anon_cost`, `file_cost` | Chi phí refault gần đây (floating average) → ai bị quét nhiều hơn. |
| | `cache_trim_mode` | Còn nhiều inactive file sạch, không thrash → ăn cache trước, đừng đụng anon. |
| | `file_is_tiny` | File quá ít → bắt buộc quét anon (tránh "cache trap"). |
| **memcg protection** | `memcg_low_reclaim`, `memcg_low_skipped` | Vòng retry để phá bảo vệ `memory.low` trước khi OOM. |
| **Kết quả** | `nr_scanned`, `nr_reclaimed` | Số page đã **quét** / đã **giải phóng**. |
| | `nr.{dirty,unqueued_dirty,congested,writeback,immediate,file_taken,taken}` | "Đo triệu chứng" ở đuôi LRU: nhiều dirty? nhiều writeback? → quyết định throttle. |
| **Cờ đặc biệt** | `compaction_ready`, `hibernation_mode`, `skipped_deactivate` | Cho nhánh compaction / hibernate / retry. |

## 6.2. Giá trị khởi tạo `sc` theo từng đường vào

| Đường vào | `nr_to_reclaim` | `priority` | `may_writepage` | `may_swap` | `reclaim_idx` | `gfp_mask` |
|---|---|---|---|---|---|---|
| `try_to_free_pages` (**direct**) `[3269-3279]` | 32 | 12 → 0 | `!laptop_mode` | 1 | `gfp_zone(gfp)` | gfp của caller |
| `balance_pgdat` (**kswapd**) `[3584-3588, 3612-3675]` | `Σ max(high_wmark,32)` (đặt trong `kswapd_shrink_node`) | 12 → 1 | `!laptop_mode && !boost` | `!boost` | `highest_zoneidx` | `GFP_KERNEL` |
| `try_to_free_mem_cgroup_pages` `[3357-3367]` | `max(nr_pages,32)` | 12 → 0 | `!laptop_mode` | tham số | `MAX_NR_ZONES-1` | caller + `HIGHUSER_MOVABLE` |
| `shrink_all_memory` (hibernate/WARP) `[4039-4048]` | tham số | 12 → 0 | **1** | 1 | max | `HIGHUSER_MOVABLE`, `hibernation_mode=1` |
| `reclaim_pages` (`MADV_PAGEOUT`) `[2131-2137]` | — (không loop) | 12 | **1** | 1 | — | `GFP_KERNEL` |

`laptop_mode` (sysctl `vm.laptop_mode`, mặc định 0) ⇒ `may_writepage = 1` bình thường. `[GENERAL; SOURCE sysctl.c:3070]`

## 6.3. "Priority" thực sự điều khiển điều gì

`priority` là **núm vặn duy nhất** của "mức độ gấp". Nó tham gia vào:

| Chỗ | Tác động | Dòng |
|---|---|---|
| `get_scan_count` | `scan = lruvec_size >> priority` | 2410 |
| `do_shrink_slab` | `delta = (freeable >> priority) * 4 / seeks` | 459-461 |
| `do_try_to_free_pages` | Vòng `--sc->priority` | 3075 |
| `shrink_lruvec` | `proportional_reclaim` chỉ ở `DEF_PRIORITY` | 2478-2479 |
| `in_reclaim_compaction` | `priority < DEF_PRIORITY - 2` ⇒ coi như cần compaction | 2566-2569 |
| `do_try_to_free_pages` | `priority < DEF_PRIORITY - 2` ⇒ bật `may_writepage` | 3068-3069 |
| `get_scan_count` | `!priority` ⇒ `SCAN_EQUAL` (quét đều anon & file, bỏ mọi khôn khéo) | 2290-2293 |
| `balance_pgdat` | priority của kswapd dừng ở 1 (không xuống 0) | 3740 |

Ví dụ với list 80 000 page `[tools/vmscan_numbers.py]`:

| priority | quét bao nhiêu | số lô 32 |
|---|---|---|
| 12 | 19 | 1 |
| 10 | 78 | 3 |
| 8 | 312 | 10 |
| 6 | 1 250 | 40 |
| 4 | 5 000 | 157 |
| 2 | 20 000 | 625 |
| 0 | 80 000 | 2 500 |

→ Đọc: *nếu thấy `priority` xuống thấp (qua tracepoint `nr_scanned`, hoặc tổng `pgscan` vọt lên), nghĩa là các vòng nhẹ đã không thu đủ — hệ thống đang khó thở.*

Tiếp theo → [`02_kswapd_and_direct_reclaim.md`](02_kswapd_and_direct_reclaim.md).
