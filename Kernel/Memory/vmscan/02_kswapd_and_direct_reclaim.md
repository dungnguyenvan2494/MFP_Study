# 02 — `kswapd` (background) và Direct Reclaim (synchronous)

> Phủ các mục: **07** kswapd · **08** Direct Reclaim · Flow 2, Flow 3 · Sequence diagram kswapd & direct reclaim.
> Thẻ nguồn xem file 00. Số dòng `vmscan.c` / `page_alloc.c` là của source local 5.10.241.

---

# 07. `kswapd`

## 7.1. kswapd là ai?

| Hỏi | Đáp | Nguồn |
|---|---|---|
| User process? | **Không.** | |
| Interrupt? | **Không.** (không chạy trong IRQ context) | |
| Workqueue? | **Không.** Không phải work item. | |
| **Kernel thread?** | **Đúng.** Tạo bằng `kthread_run(kswapd, pgdat, "kswapd%d", nid)`. | `[SOURCE vmscan.c:4079]` |
| Có mấy con? | **Một con mỗi node.** Board NUMA off ⇒ chỉ **`kswapd0`**. | `[SOURCE vmscan.c:4104-4112, Eagle_defconfig:402]` |
| Tạo khi nào? | Lúc boot: `module_init(kswapd_init)` → `kswapd_run(nid)` cho mọi node có memory. | `[SOURCE vmscan.c:4104-4114]` |
| Cờ đặc biệt | `PF_MEMALLOC \| PF_SWAPWRITE \| PF_KSWAPD` — "tôi là *memory allocator*, được lấy RAM từ vùng dự trữ, đừng bắt tôi reclaim đệ quy; tôi được phép ghi swap". | `[SOURCE vmscan.c:3921]` |
| Freezable | `set_freezable()` — bị đóng băng khi suspend/hibernate. | `[SOURCE vmscan.c:3922, 3718, 3944]` |
| CPU affinity | `cpumask_of_node(nid)` | `[SOURCE vmscan.c:3900, 3906-3907]` |
| **VENDOR** | `SCHED_RR`, priority **200** (nếu `CONFIG_KM_BIZHUB`) | `[SOURCE vmscan.c:3901-3904]` |

### Tại sao kernel cần một thread nền riêng cho reclaim?

1. **Có ngữ cảnh không được ngủ.** Packet nhận trong softirq, driver trong IRQ… xin page bằng `GFP_ATOMIC` (không có `__GFP_DIRECT_RECLAIM`) ⇒ **không thể** tự reclaim. Phải có ai đó *đã dọn sẵn trước*. Comment trong source nói thẳng: "*This is needed for things like routing etc, where we otherwise might have all activity going on in asynchronous contexts that cannot page things out*" `[SOURCE vmscan.c:3885-3889]`.
2. **Giảm độ trễ của người khác.** Nếu kswapd giữ free ≥ HIGH, đa số allocation không bao giờ thấy reclaim.
3. **Ghi dirty file page an toàn.** Chỉ kswapd được ghi file page từ reclaim để tránh tràn stack: "*Only kswapd can writeback filesystem pages to avoid risk of stack overflow*" `[SOURCE vmscan.c:1336-1338]`.
4. **Gom việc theo lô, có thể ngủ chờ I/O** mà không giữ ai.
5. **Cân bằng cả node**, không chỉ theo nhu cầu một allocation.

## 7.2. Khi nào kswapd thức dậy?

### Ai gọi `wakeup_kswapd()`?

| # | Người đánh thức | Khi nào | Nguồn |
|---|---|---|---|
| 1 | **`__alloc_pages_slowpath` → `wake_all_kswapds`** | Fast path (dùng `WMARK_LOW`) thất bại và gfp có `__GFP_KSWAPD_RECLAIM` (`ALLOC_KSWAPD`). Gọi **2 lần**: trước lần thử đầu và trong nhãn `retry:` ("*Ensure kswapd doesn't accidentally go to sleep as long as we loop*") | `[SOURCE page_alloc.c:4753-4754, 4829-4830, 4454-4468]` |
| 2 | **`rmqueue()` — watermark boost** | Sau khi lấy page, nếu cờ `ZONE_BOOSTED_WATERMARK` đã được bật (do fallback fragmentation), gọi `wakeup_kswapd(zone, 0, 0, zone_idx)` | `[SOURCE page_alloc.c:2575-2576, 3510-3513]` |
| 3 | **`allow_direct_reclaim()`** | Task bị throttle vì reserve `PFMEMALLOC` cạn ⇒ đánh thức kswapd (và đặt `kswapd_highest_zoneidx ≤ ZONE_NORMAL`) | `[SOURCE vmscan.c:3161-3167]` |
| 4 | kswapd tự "đánh thức" | Khi `kswapd_try_to_sleep` bị đánh thức giữa chừng, hoặc `balance_pgdat` kết thúc mà node chưa cân bằng → vòng `for(;;)` chạy tiếp | `[SOURCE vmscan.c:3926-3969]` |

> `GFP_KERNEL` = `__GFP_RECLAIM|IO|FS` (gồm cả kswapd & direct). `GFP_ATOMIC` có `__GFP_KSWAPD_RECLAIM` nhưng **không** `__GFP_DIRECT_RECLAIM`. `GFP_NOWAIT = __GFP_KSWAPD_RECLAIM` `[SOURCE include/linux/gfp.h:298-303]`. Nghĩa là **cả atomic cũng đánh thức được kswapd** — nhưng không bao giờ tự reclaim.

### `wakeup_kswapd()` làm gì? (và khi nào *không* làm)

```
wakeup_kswapd(zone, gfp, order, highest_zoneidx)
   │
   ├─ zone không "managed"? ───────────────► return (zone rỗng)
   ├─ cpuset không cho phép zone? ─────────► return
   │
   ├─ GHI "yêu cầu" cho kswapd đọc sau:
   │     pgdat->kswapd_highest_zoneidx = max(hiện tại, highest_zoneidx)
   │     pgdat->kswapd_order           = max(hiện tại, order)
   │
   ├─ kswapd đang BẬN (không nằm trong waitqueue)? ──► return
   │      (thông số đã ghi; kswapd sẽ đọc ở vòng sau)
   │
   ├─ "Hopeless node" (kswapd_failures ≥ 16)  HOẶC
   │  (node đã balanced VÀ không bị boost)? ──┬─► nếu KHÔNG có __GFP_DIRECT_RECLAIM:
   │                                           │      wakeup_kcompactd()   (chỉ để compaction xử lý phân mảnh)
   │                                           └─► return   (KHÔNG đánh thức kswapd)
   │
   └─ trace_mm_vmscan_wakeup_kswapd(); wake_up_interruptible(&kswapd_wait)   ◄── đánh thức thật
```
`[SOURCE vmscan.c:3983-4026]`

**Vì sao ghi `order`/`highest_zoneidx` rồi mới kiểm tra "kswapd có đang ngủ không"?** Để nếu kswapd *đang bận chạy*, yêu cầu mới vẫn không mất: kswapd đọc lại hai giá trị này ở đầu mỗi vòng lặp ngoài (`kswapd_highest_zoneidx()`, dòng 3789-3795, 3938-3942).

## 7.3. Flow 2 — vòng đời kswapd

```
                        ┌─────────────────────────────────────────────┐
  boot ──► kswapd() ───►│ vòng for(;;)  [vmscan.c:3926]               │
                        │                                             │
                        │  đọc order, highest_zoneidx từ pgdat        │
                        │            │                                │
                        │            ▼                                │
                        │  kswapd_try_to_sleep()  ◄─────────┐         │
                        │   ├ nap ngắn 100ms (HZ/10)        │         │
                        │   ├ còn balanced? → ngủ dài       │         │
                        │   └ không → return ngay           │         │
                        │            │  (bị đánh thức)      │         │
                        │            ▼                      │         │
                        │  đọc lại order/zone, xoá yêu cầu  │         │
                        │            │                      │         │
                        │  try_to_freeze()? ── có ──► continue        │
                        │            │                                │
                        │            ▼                                │
                        │  reclaim_order = balance_pgdat(order, zone) │
                        │            │                                │
                        │  reclaim_order < alloc_order ? ──── có ─────┘ (goto kswapd_try_sleep)
                        │            │ không                          │
                        └────────────┴──── quay lại đầu vòng ─────────┘
```
`[SOURCE vmscan.c:3894-3974]`

## 7.4. `balance_pgdat()` — "một đợt làm việc" của kswapd

**Câu hỏi nó trả lời:** "Làm sao để *ít nhất một zone hợp lệ* của node này đạt ≥ HIGH watermark, với chi phí thấp nhất?"

```
balance_pgdat(pgdat, order, highest_zoneidx)
  │
  ├─ Chuẩn bị: sc{gfp=GFP_KERNEL, order, may_unmap=1}; psi_memstall_enter; PAGEOUTRUN++
  ├─ Tính "boost": nr_boost_reclaim = Σ zone->watermark_boost (zone ≤ highest)
  │
  └─ restart:  priority = 12
     do {                                                       ← mỗi vòng = 1 mức priority
        reclaim_idx = highest_zoneidx
        balanced = pgdat_balanced(pgdat, order, highest_zoneidx)
        ├─ !balanced && có boost  → bỏ boost, restart       (ưu tiên cân bằng thật)
        ├─  balanced && không boost → GOTO out                (đã đủ, xong)
        │
        may_writepage = !laptop_mode && !boost ;  may_swap = !boost
        age_active_anon()                       ← "lão hoá" anon list nền
        priority < 10 → may_writepage = 1       ← bắt đầu ghi nếu khó khăn
        soft-limit memcg reclaim
        kswapd_shrink_node()                    ← gọi shrink_node: CÔNG VIỆC CHÍNH
        wake pfmemalloc_wait nếu allow_direct_reclaim()
        try_to_freeze() / kthread_should_stop()  → break
        nr_boost_reclaim -= nr_reclaimed ; boost mà 0 tiến triển → break
        if (raise_priority || !nr_reclaimed)  priority--      ← leo thang nếu cần
     } while (priority >= 1)                               ← KHÔNG bao giờ xuống 0
     │
     if (!sc.nr_reclaimed)  kswapd_failures++               ← chạy cả đợt mà 0 page
  out:
     nếu có boost: trả lại boost (zone->watermark_boost -= …) + wakeup_kcompactd(pageblock_order…)
     snapshot_refaults; psi_memstall_leave
     return sc.order
```
`[SOURCE vmscan.c:3574-3780]`

### Ý nghĩa từng action

| Action | Tại sao phải làm | Nếu bỏ | Trạng thái tạo ra |
|---|---|---|---|
| `pgdat_balanced()` đầu mỗi vòng | Kiểm tra "đã đủ chưa" *trước* khi tốn công | kswapd reclaim thừa, đẩy cache hữu ích ra khỏi RAM | Quyết định thoát `out` |
| `age_active_anon()` | anon active list không bao giờ được nhìn nếu chỉ quét inactive; cần "xoay" nền để page có cơ hội thể hiện nó còn dùng | active anon phình, inactive cạn → khi cần reclaim không có ứng viên đã *chín* | Hạ cấp một ít active→inactive |
| `kswapd_shrink_node()` | Thực thi reclaim cho cả node theo `nr_to_reclaim = Σ high` | — | `sc.nr_reclaimed`, `sc.nr_scanned` |
| `raise_priority = false` khi `sc->nr_scanned >= nr_to_reclaim` | Nếu đã *quét đủ nhiều* mà vẫn chưa balanced thì vấn đề không phải "quét ít" ⇒ đừng cuồng quét thêm | priority tụt nhanh ⇒ quét sâu quá mức ⇒ đẩy working set ra | priority giữ nguyên |
| `if (raise_priority \|\| !nr_reclaimed) priority--` | Leo thang khi quét nhẹ không ăn thua **hoặc** vòng này thu 0 page | kẹt ở priority 12 vĩnh viễn | priority giảm |
| `kswapd_failures++` | Cơ chế **tự bỏ cuộc**: node vô vọng thì thôi, để direct reclaim lo | kswapd quay vô hạn ăn CPU | Cờ "hopeless" |
| Boost accounting | watermark boost chỉ là "vay tạm"; phải trả lại | watermark treo cao mãi ⇒ reclaim thừa | `watermark_boost` giảm |
| `wakeup_kcompactd(pageblock_order…)` ở `out` (**chỉ khi đợt này có boost**) | Free đã tăng ⇒ lúc tốt để compaction gom page liền kề | phân mảnh tồn tại | kcompactd chạy |

### `pgdat_balanced()` thực chất kiểm tra gì?

`true` nếu **có ít nhất một zone** (từ dưới lên, trong phạm vi ≤ `highest_zoneidx`) thoả `zone_watermark_ok_safe(zone, order, HIGH, highest_zoneidx)`. Nếu *không có zone nào có page* trong phạm vi ⇒ `true` luôn (không cần cân bằng). `[SOURCE vmscan.c:3439-3469]`
⇒ "Cân bằng" ở đây = **một zone đủ dùng cho allocation đó**, chứ không phải mọi zone đều đầy.

## 7.5. Khi nào kswapd **dừng/ngủ** và khi nào **tiếp tục**?

| Điều kiện | Kết quả | Nguồn |
|---|---|---|
| `balanced && !boost` (đầu vòng priority) | `goto out` → quay về `kswapd_try_to_sleep` | `[3661-3662]` |
| priority tụt qua 1 (vòng `while (priority >= 1)` hết) | Thoát; nếu `nr_reclaimed == 0` ⇒ `kswapd_failures++` | `[3740-3743]` |
| `try_to_freeze()` / `kthread_should_stop()` | `break` (suspend/dừng thread) | `[3716-3721]` |
| Boost mà không thu được gì | `break` (tránh vòng lặp vô hạn vì không ghi/ swap được) | `[3735-3736]` |
| `kswapd_failures ≥ 16` (`MAX_RECLAIM_RETRIES`) | `prepare_kswapd_sleep` coi node "hopeless" → **cho ngủ**; `wakeup_kswapd` không đánh thức nữa | `[3507-3508, 4008]` |
| `reclaim_order < alloc_order` (high-order reclaim fail, hạ xuống order 0) | `goto kswapd_try_sleep` | `[3967-3968]` |

### `kswapd_try_to_sleep()` — ngủ có hai bước, và nó là **nguồn của 2 counter hữu ích**

```
kswapd_try_to_sleep()
  │
  ├─ prepare_kswapd_sleep() == true ?   (node balanced HOẶC hopeless; đồng thời đánh thức
  │                                       các task đang chờ ở pfmemalloc_wait)
  │      ├─ CÓ → reset_isolation_suitable(); wakeup_kcompactd(alloc_order…);
  │      │       remaining = schedule_timeout(HZ/10)       ← NAP 100 ms
  │      │       (remaining > 0 nghĩa là BỊ ĐÁNH THỨC giữa nap)
  │      └─ KHÔNG → bỏ qua nap
  │
  ├─ !remaining  &&  prepare_kswapd_sleep() vẫn true ?
  │      ├─ CÓ → trace_mm_vmscan_kswapd_sleep; hạ thấp độ chính xác vmstat
  │      │        (set_pgdat_percpu_threshold(normal)); schedule()  ← NGỦ DÀI tới khi có wakeup_kswapd
  │      │        ; thức dậy: set_pgdat_percpu_threshold(pressure)
  │      └─ KHÔNG → đếm:
  │              remaining ≠ 0 → KSWAPD_LOW_WMARK_HIT_QUICKLY    (bị đánh thức ngay trong nap)
  │              remaining = 0 → KSWAPD_HIGH_WMARK_HIT_QUICKLY   (hết nap mà đã mất cân bằng)
```
`[SOURCE vmscan.c:3797-3879]`

> **Đọc counter:** `kswapd_low_wmark_hit_quickly` / `kswapd_high_wmark_hit_quickly` tăng nhanh ⇒ *kswapd vừa muốn ngủ thì lại có người đẩy free xuống dưới watermark* ⇒ **áp lực kéo dài** (allocation rate ≥ reclaim rate). Đây là chỉ báo sớm tốt hơn nhìn free memory.

### Vì sao hạ/ nâng "vmstat per-cpu threshold"?

`NR_FREE_PAGES` được cộng dồn theo từng CPU nên có sai số tới `nr_cpus × threshold`. Khi kswapd *thức* (hệ thống đang gần watermark), kernel dùng threshold *nhỏ* (chính xác hơn) để không vi phạm watermark vì số liệu trễ; khi ngủ thì dùng threshold lớn (rẻ hơn). `[SOURCE vmscan.c:3858-3871 (comment)]`

## 7.6. Cơ chế "tự bỏ cuộc": `kswapd_failures` (rất quan trọng khi debug)

```
balance_pgdat chạy hết đợt mà nr_reclaimed == 0      ──► pgdat->kswapd_failures++       [3742-3743]
shrink_node có reclaimable (thu ≥1 page)             ──► pgdat->kswapd_failures = 0     [2879-2880]
kswapd_failures ≥ 16                                 ──► "hopeless":
        • wakeup_kswapd() KHÔNG đánh thức nữa                                           [4008]
        • prepare_kswapd_sleep() cho ngủ                                                [3507]
        • allow_direct_reclaim() = true (không throttle task nữa)                       [3140]
        • show_mem in:  "all_unreclaimable? yes"                                        [page_alloc.c:5630,5655]
Một lần DIRECT reclaim thành công  ──► shrink_node đặt kswapd_failures = 0 → kswapd "sống lại"
```

> **Bẫy debug trên board:** counter chỉ bị reset khi **bất kỳ** `shrink_node` nào (cả direct) thu ≥ 1 page. Nếu mỗi vòng direct reclaim chỉ cứu được vài page cache, `kswapd_failures` **không bao giờ đạt 16** ⇒ kswapd (đang `SCHED_RR 200`, RT throttling tắt) vẫn bị đánh thức và quét tiếp ⇒ **CPU cao kéo dài**. "Hopeless" chỉ xảy ra khi *thật sự* không thu được gì.

## 7.7. Watermark boost (kswapd chạy dù RAM chưa cạn)

1. Khi allocation phải lấy page từ pageblock *khác loại migratetype* (fallback = nguy cơ phân mảnh), `steal_suitable_fallback()` gọi `boost_watermark(zone)`: tăng `zone->watermark_boost` (tối đa `HIGH × watermark_boost_factor/10000`, mặc định factor = **15000** ⇒ ≤ 150% HIGH; chỉ bật nếu zone ≥ 4 pageblock) `[SOURCE page_alloc.c:365-367, 2503-2530, 2575-2576]`.
2. `rmqueue` thấy cờ `ZONE_BOOSTED_WATERMARK` → `wakeup_kswapd`.
3. `balance_pgdat`: **boosted reclaim** *không ghi, không swap*, priority bị chặn ở `DEF_PRIORITY-2`, hết tiến triển thì dừng `[3664-3675, 3735-3736]`.

**Hệ quả quan sát:** thấy `kswapd0` chạy khi `free` vẫn lớn hơn LOW ⇒ nghi **fragmentation boost**, không phải thiếu RAM. Kiểm tra `/proc/buddyinfo`, `/proc/pagetypeinfo` và `/proc/sys/vm/watermark_boost_factor`.

## 7.8. High-order (order > 0)

- kswapd nhận `alloc_order` từ allocator; sau khi thu ≥ `compact_gap(order)` = `2 << order` page, nó **hạ `sc->order` về 0** để khỏi reclaim thừa (giả định task xin sẽ tự compact) `[SOURCE vmscan.c:3555-3556; compact_gap: include/linux/compaction.h:65-78]`.
- Khi kswapd ngủ, nó đánh thức **kcompactd** với `alloc_order` ban đầu `[3828]`.
- Phân biệt `alloc_order` (cái người xin cần) và `reclaim_order` (cái kswapd thực sự reclaim) `[3961-3968]`.

## 7.9. Sequence diagram — kswapd

```
 Allocator           pgdat/waitq         kswapd0            vmscan core            LRU/Pages
 (page_alloc.c)                          (kernel thread)    (shrink_node…)
    │                    │                    │                   │                    │
    │ free < LOW, fast path fail             │                   │                    │
    │──wakeup_kswapd()──►│                    │                   │                    │
    │  ghi order & zoneidx                    │                   │                    │
    │                    │──wake_up──────────►│                   │                    │
    │ (retry get_page với MIN → OK, không chờ)│                   │                    │
    │◄── trả page ───────│                    │                   │                    │
    │                    │                    │ kswapd_try_to_sleep() trả về           │
    │                    │                    │──balance_pgdat()─►│                    │
    │                    │                    │                   │ pgdat_balanced? no │
    │                    │                    │                   │──shrink_node───────►│
    │                    │                    │                   │   isolate 32 từ đuôi│
    │                    │                    │                   │   shrink_page_list  │
    │                    │                    │                   │◄── free/keep/activate
    │                    │                    │   (lặp, priority 12→…, ≥1)              │
    │                    │                    │◄──── balanced ────│                    │
    │                    │                    │ kswapd_try_to_sleep: nap 100ms → ngủ dài│
    │                    │                    │ (set_pgdat_percpu_threshold normal)     │
```

---

# 08. Direct Reclaim

## 8.1. Câu chuyện: một `malloc()` bị chậm

```
Process A:  malloc(1MB)                       ← chỉ cấp ĐỊA CHỈ ẢO (brk/mmap), chưa có RAM
   │
   ▼  A ghi vào vùng đó lần đầu
CPU page fault  ──►  do_page_fault → handle_mm_fault → do_anonymous_page
   │                       (gfp = GFP_HIGHUSER_MOVABLE|__GFP_ZERO: có DIRECT_RECLAIM, IO, FS)
   ▼
__alloc_pages_nodemask()    ← fast path, WMARK_LOW: FAIL (free thấp)         [page_alloc.c:5032-5080]
   ▼
__alloc_pages_slowpath()    ← wake kswapd, retry với MIN: VẪN FAIL (free < MIN) [page_alloc.c:4689]
   ▼
__alloc_pages_direct_reclaim() → __perform_reclaim() → try_to_free_pages()
   ▼
Process A TỰ chạy vmscan trong context của nó... A phải chờ (không chạy code app)
   ▼
reclaim thu được ≥32 page → cấp lại page → page fault trả về → A chạy tiếp
```
`[SOURCE page_alloc.c:4397-4452, 4861-4864]`; chuỗi page-fault là `[GENERAL KERNEL KNOWLEDGE]`.

## 8.2. Flow 3 — Direct reclaim từ góc nhìn allocator (đúng thứ tự trong `__alloc_pages_slowpath`)

```
Process needs allocation
        │
        ▼
get_page_from_freelist (WMARK_LOW)                         ← fast path
        │ fail
        ▼
__alloc_pages_slowpath:
   ① gfp_to_alloc_flags()  (bắt đầu từ WMARK_MIN)                       [4727]
   ② ALLOC_KSWAPD ? → wake_all_kswapds()                                [4753]
   ③ get_page_from_freelist(MIN)   ── OK ──► xong                       [4760]
   ④ order cao / non-movable ? → direct COMPACT trước                   [4773-4817]
retry:
   ⑤ wake_all_kswapds() lần nữa; xét reserve (PF_MEMALLOC/OOM victim…)  [4829-4845]
   ⑥ get_page_from_freelist  ── OK ──► xong                             [4848]
   ⑦ !can_direct_reclaim  → nopage (atomic: bỏ cuộc)                    [4853]
   ⑧ PF_MEMALLOC          → nopage (tránh đệ quy)                       [4857]
   ⑨ ★ __alloc_pages_direct_reclaim()  ◄── DIRECT RECLAIM               [4861]
          └ __perform_reclaim → try_to_free_pages → do_try_to_free_pages
          └ nếu progress>0 → get_page_from_freelist; fail → unreserve_highatomic + drain_all_pages → thử lại
   ⑩ direct COMPACT                                                      [4867]
   ⑪ __GFP_NORETRY → nopage                                              [4873]
   ⑫ costly order & !RETRY_MAYFAIL → nopage                              [4880]
   ⑬ should_reclaim_retry() ── true ──► goto retry                       [4884]
   ⑭ should_compact_retry() ── true ──► goto retry                       [4894]
   ⑮ __alloc_pages_may_oom()  (OOM killer)                               [4910]
        └ OOM thành công → retry; ngược lại ↓
   nopage: __GFP_NOFAIL ? vòng lại : warn_alloc("page allocation failure")  [4926-4977]
```

## 8.3. Phần của `vmscan.c` trong direct reclaim

```
try_to_free_pages(zonelist, order, gfp, nodemask)                    [3265]
  │  sc{nr_to_reclaim=32, priority=12, may_writepage=!laptop_mode, may_unmap=1, may_swap=1}
  │
  ├─ throttle_direct_reclaim()    ← có thể NGỦ (xem 8.5); trả true nếu bị kill khi chờ → return 1
  ├─ set_task_reclaim_state()  ; trace direct_reclaim_begin
  │
  └─ do_try_to_free_pages(zonelist, sc)                              [3039]
        │  retry:
        │  delayacct_freepages_start ; ALLOCSTALL++ (zone = reclaim_idx)       [3047-3050]
        │  do {                                  ← mỗi vòng = 1 mức priority
        │      vmpressure_prio()
        │      sc->nr_scanned = 0
        │      shrink_zones(zonelist, sc)        ← duyệt zone → shrink_node (1 lần / node)
        │      nr_reclaimed ≥ nr_to_reclaim ?  → break     (ĐỦ 32 page)
        │      compaction_ready ?              → break
        │      priority < 10 → may_writepage = 1
        │      [VENDOR] pm_device_down && warp_canceled → break
        │  } while (--priority >= 0)             ← có thể xuống tận 0 (quét hết)
        │  snapshot_refaults ; clear LRUVEC_CONGESTED (memcg)
        │  nr_reclaimed > 0      → return nr_reclaimed
        │  compaction_ready      → return 1  (đừng OOM, cứ compaction)
        │  skipped_deactivate    → priority = initial ; force_deactivate=1 ; goto retry
        │  memcg_low_skipped     → priority = initial ; memcg_low_reclaim=1 ; goto retry
        └─ return 0   ← "không reclaim được gì"
```

### Ý nghĩa các nhánh "retry" cuối hàm (hay bị bỏ sót)

| Nhánh | Tình huống | Vì sao cần |
|---|---|---|
| `skipped_deactivate` → `force_deactivate` | Kernel *ước lượng* "active list không cần hạ cấp" (dựa vào `inactive_is_low` & refault) nhưng thực tế không thu được gì ⇒ ước lượng sai | Chạy lại bắt buộc hạ cấp active → tránh OOM oan `[3113-3118]` |
| `memcg_low_skipped` → `memcg_low_reclaim` | Mọi memcg có thể reclaim đều đang được `memory.low` bảo vệ | Trước khi OOM, thử phá bảo vệ mềm `[3121-3127]` |
| `compaction_ready` → `return 1` | Free đã đủ để compaction làm việc (order cao) | Đừng kích OOM khi chỉ cần compact `[3101-3102]` |

## 8.4. `shrink_zones()` — chọn node để reclaim (direct)

- Duyệt `zonelist` các zone `≤ reclaim_idx`. Nếu `buffer_heads_over_limit` thì tạm bật `__GFP_HIGHMEM` (chỉ có ý nghĩa với highmem) `[2940-2944]`.
- Bỏ zone mà cpuset không cho (`cpuset_zone_allowed`) `[2953]`.
- Order > `PAGE_ALLOC_COSTLY_ORDER(3)` & `compaction_ready` ⇒ **không reclaim thêm**, để compaction làm `[2966-2971]`.
- **Mỗi node chỉ shrink một lần** (`last_pgdat`) `[2979, 2998-3001]`.
- Trước đó: `mem_cgroup_soft_limit_reclaim` — lấy bớt từ memcg vượt soft limit `[2988-2993]`.
- Cuối cùng gọi `shrink_node()`.

## 8.5. Tại sao direct reclaim làm task **chậm / block**

Direct reclaim chạy **trong context của chính task xin RAM** nên (1) nó *tiêu CPU của task*, (2) nó có thể **ngủ**. Các điểm ngủ / chậm có thật trong source:

| # | Điểm | Ngủ bao lâu | Khi nào | Dòng |
|---|---|---|---|---|
| 1 | `throttle_direct_reclaim()` → `wait_event_killable(pfmemalloc_wait)` | **tới khi kswapd cứu được** (killable) | Reserve `PFMEMALLOC` ≤ ½ tổng MIN và `kswapd_failures < 16`; task không phải kthread | `[3181-3263]` |
| 1b | cùng, nhưng caller không có `__GFP_FS` | **tối đa 1 giây** (`HZ`) | tránh deadlock nếu kswapd bị khoá cùng lock FS | `[3246-3251]` |
| 2 | `shrink_inactive_list`: `msleep(100)` | 100 ms, **một lần** rồi `return 0` nếu vẫn quá nhiều isolated | `too_many_isolated()` — quá nhiều direct reclaimer đã lấy page ra khỏi LRU | `[1950-1961]` |
| 3 | `shrink_node`: `wait_iff_congested(BLK_RW_ASYNC, HZ/10)` | ≤ 100 ms | Direct reclaim (không kswapd) & `LRUVEC_CONGESTED` | `[2864-2867]` |
| 4 | `shrink_page_list` case 3: `wait_on_page_writeback()` | tới khi I/O xong | **chỉ legacy memcg** (không có dirty throttling) gặp page `PageReclaim` đang writeback | `[1231-1237]` |
| 5 | Mọi nơi `cond_resched()` | tự nguyện nhường CPU | khắp nơi; board `PREEMPT=y` nên còn bị preempt | |
| 6 | `should_reclaim_retry`: `congestion_wait(HZ/10)` | 100 ms | không tiến triển & nhiều dirty/writeback | `[page_alloc.c:4623-4632]` |
| 7 | `wait_on_page_writeback`/`lock` ở tầng khác (FS, swap I/O) | tuỳ I/O | `pageout()` anon/swap sync write | |

> **Không** có trong danh sách: ghi dirty *file page* từ direct reclaim. Một direct reclaimer gặp dirty file page luôn chọn `SetPageReclaim + activate` (bỏ qua) chứ không `pageout()` — chỉ **kswapd** (khi `PGDAT_DIRTY`) mới ghi `[1346-1359]`. Vì vậy task thường bị chậm do **quét + ngủ chờ**, hiếm khi do tự ghi file.

### `throttle_direct_reclaim()` / `allow_direct_reclaim()` — "đừng cạnh tranh khi kho dự trữ cạn"

```
allow_direct_reclaim(pgdat):
   kswapd_failures ≥ 16 → true  (không đợi nữa vì kswapd vô vọng)
   Σ min_wmark(zone ≤ NORMAL, có reclaimable)  = reserve
   free_pages(các zone đó)                     = free
   wmark_ok = free > reserve/2
   !wmark_ok && kswapd đang ngủ → đánh thức kswapd (zoneidx ≤ NORMAL)
   return wmark_ok
```
`[SOURCE vmscan.c:3132-3170]` — với reserve = 1000 page (ví dụ), task bị throttle khi free ≤ 500 page. Mục đích gốc: khi swap qua mạng, reclaim cần page từ reserve `PFMEMALLOC` để gửi packet; nếu nhiều direct reclaimer tranh nhau hết reserve thì kẹt. Source áp dụng cho **mọi** direct reclaimer không phải kthread. Counter: `pgscan_direct_throttle` `[3236]`.

## 8.6. Direct reclaim kết thúc thế nào → quyết định tiếp theo

```
try_to_free_pages trả N
   N > 0 → __alloc_pages_direct_reclaim: get_page_from_freelist
              ├ OK → page cho task  ✔
              └ fail → drain per-CPU lists & highatomic reserve → thử lại; vẫn fail → xuống ⑩ compaction ⑬ retry …
   N = 0 → did_some_progress = 0 → should_reclaim_retry():
              no_progress_loops++ ;  > 16 → hết kiên nhẫn → OOM path
              còn lại: "nếu reclaim HẾT mọi page reclaimable thì watermark có thoả không?"
                         có  → retry (và nếu nhiều write_pending: congestion_wait 100ms)
                         không → OOM
```
`[SOURCE page_alloc.c:4564-4653]`.

**Điểm tinh tế:** page vừa được reclaim đi vào **per-CPU list** (`free_unref_page_list`), chưa chắc đã "thấy" ngay trong buddy ⇒ sau reclaim allocator **drain** (`drain_all_pages`) rồi thử lại `[page_alloc.c:4444-4449]`.

## 8.7. memcg direct reclaim (biến thể)

`try_charge()` ⇒ `try_to_free_mem_cgroup_pages(memcg, nr_pages, gfp, may_swap)` ⇒ cùng `do_try_to_free_pages` nhưng `sc->target_mem_cgroup = memcg` (→ `cgroup_reclaim(sc) = true`): chỉ reclaim trong subtree; **không** dùng zonelist toàn cục mà `node_zonelist(numa_node_id())` để *"put equal pressure on all the nodes"* `[3369-3373]`; chạy dưới `memalloc_noreclaim_save()`. Counter `ALLOCSTALL` **không** tăng cho memcg reclaim `[3049]`. Board bật `MEMCG` nhưng `BLK_CGROUP`/`CGROUP_WRITEBACK` tắt ⇒ memcg **legacy-style writeback throttling** (`writeback_throttling_sane = false` nếu là cgroup v1) ⇒ có thể gặp case 3 (`wait_on_page_writeback`) `[SOURCE vmscan.c:268-277]`.

## 8.8. Sequence diagram — direct reclaim

```
Application   Allocator (page_alloc.c)        vmscan.c              LRU/pages           kswapd0
    │                  │                          │                      │                 │
    │ page fault       │                          │                      │                 │
    │─alloc_pages()───►│ fast path (LOW) FAIL     │                      │                 │
    │                  │──wake_all_kswapds()──────┼──────────────────────┼────────────────►│
    │                  │ retry(MIN) FAIL          │                      │                 │
    │                  │──try_to_free_pages()────►│                      │                 │
    │                  │                          │ throttle?            │                 │
    │                  │                          │  (có thể ngủ chờ)◄───┼─────────────────│ wake khi OK
    │                  │                          │ do_try_to_free_pages │                 │
    │                  │                          │  loop priority 12→0  │                 │
    │                  │                          │──shrink_node────────►│ isolate 32 đuôi │
    │                  │                          │                      │ shrink_page_list│
    │                  │                          │◄──free/keep/activate─│                 │
    │                  │                          │ nr_reclaimed ≥ 32 ?  │                 │
    │                  │◄──── trả nr_reclaimed ───│                      │                 │
    │                  │ get_page_from_freelist   │                      │                 │
    │◄─── page ────────│ (hoặc compaction/retry/OOM nếu 0)               │                 │
    │ App chạy tiếp (đã chậm đúng bằng thời gian reclaim)               │                 │
```

## 8.9. So sánh kswapd vs direct reclaim

| Tiêu chí | kswapd | Direct reclaim |
|---|---|---|
| Ai chạy | thread `kswapd0` | **chính task xin RAM** |
| Đồng bộ? | Bất đồng bộ (nền) | **Đồng bộ** (task chờ) |
| Kích hoạt | free < **LOW** (hoặc boost) | free < **MIN** (sau khi retry fail) |
| Mục tiêu | đưa một zone lên ≥ **HIGH** (`nr_to_reclaim = Σ high`) | thu ≥ **32** page (`SWAP_CLUSTER_MAX`) |
| priority | 12 → **1** | 12 → **0** |
| Ghi dirty file page | **Có** (khi `PGDAT_DIRTY` & `PageReclaim`) | **Không** (đánh dấu rồi bỏ qua) |
| Throttle | không bị `too_many_isolated`; nap/ngủ riêng | `throttle_direct_reclaim`, `too_many_isolated`, `wait_iff_congested` |
| Tự bỏ cuộc | `kswapd_failures ≥ 16` | thất bại ⇒ allocator → compaction/retry/OOM |
| Ảnh hưởng latency app | Gián tiếp (ăn CPU, tranh lock) | **Trực tiếp** (app đứng chờ) |
| Cờ `PF_*` | `PF_KSWAPD` | `PF_MEMALLOC` (set tạm trong `__perform_reclaim`) |
| Counter | `pgscan_kswapd`, `pgsteal_kswapd`, `pageoutrun` | `pgscan_direct`, `pgsteal_direct`, `allocstall_*`, `pgscan_direct_throttle` |
| Board | **SCHED_RR 200, không bị RT throttle** (VENDOR) | chạy theo scheduling class của task |

Tiếp theo → [`03_lru_scanning_reclaim_dirty_progress.md`](03_lru_scanning_reclaim_dirty_progress.md).
