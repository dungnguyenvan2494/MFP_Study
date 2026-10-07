# 03 — LRU, Page Scanning, Page Reclaim, Dirty/Clean, Reclaim Progress

> Phủ các mục: **09** LRU · **10** Page Scanning · **11** Page Reclaim · **12** Dirty/Clean pages · **13** Reclaim Progress
> Flow 4 (LRU scanning), Flow 5 (page reclaim), Flow 6 (dirty page), Flow 7 (no progress).
> Mọi dòng `vmscan.c:N` là của source local 5.10.241.

---

# 09. LRU — bắt đầu từ vấn đề, không phải từ `struct lruvec`

## 9.1. Vấn đề

> Kernel có hàng trăm nghìn page. Khi cần 32 page, **lấy page nào trước** mà không phải nhìn từng page một?

Ý tưởng (theo "locality"): *page vừa dùng sớm có khả năng được dùng lại; page lâu chưa đụng có khả năng không dùng nữa.* ⇒ giữ các page theo **thứ tự thời gian truy cập gần nhất**; chọn nạn nhân ở **đầu "cũ nhất"**.

```
LRU concept
    │
Page được dùng gần đây?
    ├── CÓ  ─► ACTIVE  (đang nóng, đừng đụng)
    └── KHÔNG ─► INACTIVE (nguội, ứng viên reclaim)
```

Nhưng một danh sách duy nhất bị **phá bởi dữ liệu dùng-một-lần** (copy file lớn đọc mỗi page đúng 1 lần). ⇒ chia hai: page mới vào ở **inactive**; chỉ khi bị dùng *lần thứ hai* mới lên **active**. Reclaim chỉ ăn từ **đuôi inactive**.

## 9.2. Có bao nhiêu list? Ở đâu?

```
enum lru_list { LRU_INACTIVE_ANON, LRU_ACTIVE_ANON,
                LRU_INACTIVE_FILE, LRU_ACTIVE_FILE, LRU_UNEVICTABLE }   [include/linux/mmzone.h]
```

| List | Chứa | Reclaim quét? |
|---|---|---|
| `INACTIVE_FILE` | page cache chưa chứng tỏ hot | **Có — nguồn reclaim chính** |
| `ACTIVE_FILE` | page cache hot (kể cả code chương trình) | Chỉ *hạ cấp*, không free trực tiếp |
| `INACTIVE_ANON` | anon (và **shmem/tmpfs**) chưa hot | Chỉ nếu có swap |
| `ACTIVE_ANON` | anon hot | Chỉ hạ cấp, và chỉ nếu có swap |
| `UNEVICTABLE` | mlock, ramfs… | **Không bao giờ** |

- Một bộ 5 list ấy gọi là **`lruvec`**; có **một lruvec cho mỗi (node, memcg)** `[GENERAL]`. Board: 1 node, memcg bật ⇒ mỗi memcg có lruvec riêng + lruvec gốc (root).
- **Khoá:** ở 5.10 là **`pgdat->lru_lock`** — *một spinlock cho cả node* (không phải per-lruvec như kernel mới) `[SOURCE vmscan.c:1794, 1965, 2048, 2105, 2710]`. Comment gốc: "*pgdat->lru_lock is heavily contended*" `[1652]`. ⇒ trên máy 4 core, các CPU cùng reclaim/`lru_add_drain` sẽ tranh lock này.
- Trang **file vs anon:** `page_is_file_lru(page) = !PageSwapBacked(page)` `[SOURCE mm_inline.h:22-25]` ⇒ **tmpfs/shmem đi vào LRU *anon***, dù trông như file.

## 9.3. Vòng đời một page trên LRU

```
        ┌─────────────── sinh ra ────────────────┐
        │ file: đọc/ghi file → add_to_page_cache_lru → INACTIVE_FILE
        │ anon: page fault lần đầu → lru_cache_add_inactive_or_unevictable
        │         → INACTIVE_ANON         [SOURCE mm/memory.c:3687 (do_anonymous_page); swap.c lru_cache_add]
        └───────────────────────┬─────────────────┘
                                ▼
                        ┌───────────────┐   truy cập lần 1
                        │  INACTIVE     │ ──────────────► đặt PG_referenced
                        │  (unreferenced)│
                        └───────┬───────┘
                                │ truy cập lần 2 (mark_page_accessed)  [SOURCE swap.c mark_page_accessed]
                                │   hoặc: page_check_references thấy PTE referenced lần 2
                                ▼
                        ┌───────────────┐
                        │   ACTIVE      │
                        └───────┬───────┘
                                │ shrink_active_list: xoá bit accessed (page_referenced),
                                │ ClearPageActive + SetPageWorkingset  [vmscan.c:2097-2099]
                                ▼
                        ┌───────────────┐   shrink_page_list
                        │  INACTIVE     │ ───────────────────► FREE (tháo khỏi page cache/swap)
                        └───────────────┘
                                │
                  refault (đọc lại page vừa bị evict)
                                ▼
                  workingset.c: nếu refault_distance nhỏ → vào thẳng ACTIVE
                  (dựa trên "shadow entry" do __remove_mapping để lại)
```

Hai cách một page tạo "second chance":
- **Page không bị map** (đọc bằng `read()`): `mark_page_accessed()` — lần chạm đầu đặt `PG_referenced`, lần hai mới `activate_page` `[SOURCE swap.c mark_page_accessed]`.
- **Page đang được map** (mmap/exec/anon): **phần cứng chỉ cho bit Accessed trong PTE**. Kernel xoá bit rồi nhìn lại ở lần quét sau, qua **rmap**: `page_referenced()` duyệt mọi PTE trỏ tới page và trả *số PTE đã referenced* (xoá luôn bit) `[SOURCE rmap.c page_referenced]`.

### `PG_workingset` và refault (vì sao reclaim "có trí nhớ")

Khi một page bị evict khỏi page cache/swap cache, `__remove_mapping()` để lại **shadow entry** trong xarray (ghi lại "thời điểm" bị evict) `[SOURCE vmscan.c:914-950]` (`workingset_eviction`). Khi page được đọc lại (*refault*), `workingset.c` tính khoảng cách refault: nếu nhỏ, nghĩa là *page này đáng ra phải còn trong RAM* ⇒ cho vào **active** ngay và tăng `workingset_activate_*`/`workingset_refault_*`. Trong `shrink_node`, các counter này quyết định *có cần hạ cấp active không* `[2719-2743]` và chi phí anon/file `[get_scan_count]`.

## 9.4. Tỉ lệ active : inactive — `inactive_is_low()`

> **Vì sao có tỉ lệ?** Nếu inactive quá nhỏ, page mới vào bị đẩy ra trước khi kịp được dùng lần hai ⇒ cache **thrash**. Nếu inactive quá lớn thì lãng phí. Kernel giữ `inactive × ratio ≥ active`.

`ratio = int_sqrt(10 × GiB)` (tối thiểu 1) `[SOURCE vmscan.c:2220-2237]`. Kết quả từ `tools/vmscan_numbers.py`:

| Tổng list (GiB) | inactive_ratio | inactive tối thiểu giữ |
|---|---|---|
| < 1 | 1 | 50% |
| 1 | 3 | 25% |
| 2 | 4 | 20% |
| 4 | 6 | 14.3% |
| 10 | 10 | 9.1% |
| 100 | 31 | 3.1% |

Dùng ở: `shrink_node` (quyết định `may_deactivate`), `shrink_lruvec` cuối hàm (cân anon), `age_active_anon` (kswapd) `[2724, 2738, 2558, 3399]`.

## 9.5. Anon hay File? — `get_scan_count()` (Stage 4)

Hàm này trả `nr[4]` = số page cần quét cho mỗi list. Nó là **cây quyết định theo thứ tự ưu tiên** `[SOURCE vmscan.c:2255-2449]`:

```
get_scan_count()
 ① !may_swap  HOẶC  không còn chỗ swap  ───────────────► SCAN_FILE   (chỉ quét file)
 ② memcg reclaim && swappiness == 0 ────────────────────► SCAN_FILE
 ③ priority == 0 (sắp OOM) && swappiness ≠ 0 ───────────► SCAN_EQUAL  (anon & file theo kích thước, bỏ mọi khôn khéo)
 ④ file_is_tiny (file gần hết) ─────────────────────────► SCAN_ANON   (tránh "cache trap")
 ⑤ cache_trim_mode (inactive file dồi dào, không thrash)► SCAN_FILE
 ⑥ còn lại ─────────────────────────────────────────────► SCAN_FRACT  (chia theo swappiness & chi phí refault)
        ap = swappiness × (T+1)/(anon_cost'+1) ;  fp = (200−swappiness) × (T+1)/(file_cost'+1)
        (anon_cost' = total_cost + anon_cost ; file_cost' = total_cost + file_cost ; T = anon_cost'+file_cost')
 Sau đó với mỗi list: scan = lruvec_size × (bảo vệ memcg) >> priority ; rồi nhân theo ap/fp.
```

Kết quả của SCAN_FRACT (`swappiness = 60`, từ script):

| Tình huống | anon % | file % |
|---|---|---|
| Mới bắt đầu (cost = 0) | 30.0 | **70.0** |
| anon refault nhiều (anon_cost = 1000) | 17.5 | 82.5 |
| file refault nhiều (file_cost = 1000) | 46.1 | 53.9 |
| `swappiness = 100`, cost bằng nhau | 50.0 | 50.0 |
| `swappiness = 0` (global, SCAN_FRACT) | 0 | 100 |
| `swappiness = 200` | 100 | 0 |

**Đọc kết quả:** chi phí = *tần suất refault* (đã đọc lại page vừa evict) × *chi phí I/O tương đối*. List nào "evict xong lại bị đòi lại nhiều" thì **bị quét ít đi**, vì reclaim list đó vô ích. (`anon_cost/file_cost` được ghi bởi `lru_note_cost()` khi **ghi page ra** `[vmscan.c:1989]` và khi **refault** `[swap.c:281-320]`.)

> **Trên board (nếu *không có swap*):** điều kiện ① luôn đúng ⇒ `SCAN_FILE` ⇒ **anon list không bao giờ bị quét**. Mọi áp lực dồn vào page cache sạch + slab. Khi page cache sạch hết mà RAM vẫn thiếu ⇒ không còn gì để reclaim ⇒ `kswapd_failures` tăng ⇒ **OOM**. `[INFERENCE — xác nhận `cat /proc/swaps`]`

## 9.6. Ví dụ của bạn: A, B, C, D

```
RAM:  A = file cache, ít dùng          B = anon, đang dùng
      C = file cache, dùng thường xuyên D = anon, lâu không dùng
```

| Page | Kernel "suy nghĩ" | Kết cục |
|---|---|---|
| **A** (file, nguội) | Nằm ở `INACTIVE_FILE`, không PTE referenced. Sạch? Nếu sạch: `PAGEREF_RECLAIM` → tháo khỏi cache (**free**). Nếu bẩn: không bỏ ngay → `SetPageReclaim` + chờ writeback | **Bị lấy đầu tiên** |
| **C** (file, nóng) | Đã lên `ACTIVE_FILE`. Reclaim *không free* active. Chỉ khi `shrink_active_list` hạ cấp nó (hết referenced) mới thành ứng viên. Nếu là *code chương trình* (`VM_EXEC`) được thêm một vòng ở active | **Được bảo vệ** |
| **B** (anon, đang dùng) | `ACTIVE_ANON` (hoặc inactive nhưng PTE referenced → `PAGEREF_ACTIVATE`/`KEEP`). Nếu **không có swap**: anon không bị quét | **Giữ** |
| **D** (anon, nguội) | `INACTIVE_ANON`, không referenced. **Có swap:** cấp slot swap → ghi ra → gỡ PTE → free (đắt: có I/O). **Không swap:** `get_scan_count` trả `SCAN_FILE`, D **không bao giờ bị động tới** ⇒ D vẫn nằm đó chiếm RAM | **Có swap: bị lấy sau A. Không swap: không lấy được** |

Thứ tự "rẻ → đắt" mà kernel ưu tiên: **A (clean file) → D (anon, cần swap) → C/B (chỉ khi gần OOM, `priority` thấp)**. Riêng việc nghiêng về file được thiết kế sẵn (swappiness 60 → 70/30) `[GENERAL + SOURCE tool output]`.

---

# 10. Page Scanning — một *quyết định*, không phải "xoá ngẫu nhiên"

## 10.1. "Scan page" nghĩa là gì?

Cụ thể trong source: **lấy tối đa N page từ đuôi một list, gỡ chúng ra khỏi LRU, rồi đánh giá từng page**. Mục tiêu của scan *không phải* "xoá" mà là **tìm các page có thể giải phóng với chi phí thấp nhất**. Một page được đánh giá trên các thuộc tính:

| Thuộc tính | Hỏi | Vì sao quan trọng |
|---|---|---|
| Zone | Có thuộc zone ≤ `reclaim_idx`? | Free page ở zone không dùng được là vô ích |
| Evictable | Có `mlock`/unevictable? | Không bao giờ reclaim |
| Locked | `trylock_page` được không? | Đang bị ai đó dùng ⇒ bỏ qua |
| Writeback | Đang ghi? | Không free được; cần throttle |
| Referenced | PTE/`PG_referenced` bật? | Còn dùng ⇒ giữ/activate |
| Mapped | Có PTE trỏ tới? | Phải `try_to_unmap` trước |
| Dirty | Bẩn? | Phải ghi trước (hoặc bỏ qua) |
| Anon/swap-backed | Cần slot swap? | Không swap thì kẹt |
| Pinned / refcount thừa | Có ai giữ ref? | `__remove_mapping` sẽ thất bại |

## 10.2. Flow 4 — LRU scanning

```
shrink_node(pgdat, sc)                                                  [2691]
  └ shrink_node_memcgs()                                                [2634]
      │  với mỗi memcg:
      │    below_min → bỏ qua cứng ; below_low → bỏ qua mềm (trừ khi memcg_low_reclaim)
      ├─ shrink_lruvec(lruvec, sc)                                      [2451]
      │     ├ get_scan_count(lruvec, sc, nr[])                          ← Stage 4
      │     ├ blk_start_plug()      (gom I/O)
      │     ├ while nr[INACT_ANON] || nr[ACT_FILE] || nr[INACT_FILE]:   [2482]
      │     │     với mỗi lru (inact_anon, act_anon, inact_file, act_file):
      │     │         nr_to_scan = min(nr[lru], 32) ; nr[lru] -= nr_to_scan
      │     │         nr_reclaimed += shrink_list(lru, nr_to_scan)      ← Stage 5
      │     │     cond_resched()
      │     │     nr_reclaimed ≥ nr_to_reclaim (và không proportional) ?
      │     │         → cân bằng tỉ lệ anon:file rồi có thể dừng        [2499-2549]
      │     ├ blk_finish_plug()
      │     └ rebalance: total_swap_pages && inactive_is_low(anon) → shrink_active_list(anon,32)
      └─ shrink_slab(gfp, node, memcg, priority)                        ← slab, song song
  └ vmpressure(...)  ghi "hiệu suất": scanned vs reclaimed
```

Ví dụ một vòng (script): `nr = {anon_inact:12, anon_act:10, file_inact:64, file_act:40}` ⇒
```
round 1: shrink_list(anon_inact,12) (anon_act,10) (file_inact,32) (file_act,32)
round 2: shrink_list(file_inact,32) (file_act, 8)
```
(Chú ý: điều kiện `while` **bỏ qua** `nr[LRU_ACTIVE_ANON]` — active anon luôn được xử lý nhờ rebalance riêng ở cuối `[2482-2483, 2558-2560]`.)

### Phần "khó hiểu" trong `shrink_lruvec`: dừng sớm và *proportional reclaim*

- **Direct reclaim ở `DEF_PRIORITY` (12):** `proportional_reclaim = true` ⇒ **không dừng** khi đủ `nr_to_reclaim`; làm hết mục tiêu quét. Lý do (comment `[2467-2479]`): direct reclaim xuất hiện nghĩa là kswapd không theo kịp ⇒ nên làm *một mẻ lớn*.
- **kswapd / memcg:** khi đã đủ `nr_to_reclaim`, thay vì dừng ngay, kernel *giữ tỉ lệ anon:file đã định* bằng cách dừng list nhỏ hơn và co phần còn lại của list lớn tương ứng `[2509-2549]` ("*It's just vindictive to attack the larger once the smaller has gone to zero*").

## 10.3. `isolate_lru_pages()` — "lấy hàng ra khỏi kệ"

**Vấn đề:** xử lý page (hỏi rmap, unmap, ghi đĩa) rất chậm; không thể giữ `lru_lock` suốt thời gian đó. **Giải pháp:** trong lúc giữ lock chỉ *lấy một lô ≤32 page ra* khỏi LRU (xoá `PG_lru`, tăng refcount) rồi nhả lock và làm việc trên danh sách riêng `[SOURCE vmscan.c:1651-1755]`. Comment: "*For pagecache intensive workloads, this function is the hottest spot in the kernel (apart from copy_*_user functions)*" `[1656-1657]`.

```
isolate_lru_pages(nr_to_scan, lruvec, dst, &nr_scanned, sc, lru)
  while scan < nr_to_scan && list không rỗng:
      page = ĐUÔI list                                     (lru_to_page = list->prev)
      page ở zone > reclaim_idx ?  → chuyển sang pages_skipped (không tính vào 'scan')  → PGSCAN_SKIP
      __isolate_lru_page(page, mode):
           không phải PageLRU            → lỗi
           unevictable                   → lỗi
           (mode UNMAPPED && page mapped)→ -EBUSY     (khi !may_unmap)
           get_page_unless_zero(page) ?  → ClearPageLRU, OK
                  không (đang bị free ở chỗ khác) → -EBUSY → trả về list, bỏ qua
      OK → chuyển sang 'dst', cộng vào nr_taken, nr_zone_taken[zone]
  splice pages_skipped về ĐẦU list  (phá thứ tự LRU một chút nhưng tránh quét lại mãi → OOM sớm)
  *nr_scanned = total_scan (gồm cả skipped, THP tính đủ 512 page)
  update_lru_sizes()
  return nr_taken
```

> **Điểm tinh tế hay gây hiểu nhầm khi đọc số liệu:** `nr_scanned` (→ `pgscan_*`) **bao gồm cả page bị skip do zone** (`total_scan`), còn vòng lặp dừng theo `scan` (không tính skipped). Nếu LRU đầy page ở zone cao hơn `reclaim_idx`, `pgscan` tăng mà `nr_taken` ≈ 0 ⇒ trông như "quét nhiều, reclaim ít" nhưng **nguyên nhân là zone-skip** (xem `pgscan_skip_*`). `[SOURCE 1697-1750]`

## 10.4. `shrink_inactive_list()` — quét inactive và thật sự reclaim

```
shrink_inactive_list(nr_to_scan, lruvec, sc, lru)                          [1937]
  ① while too_many_isolated(): msleep(100) 1 lần; vẫn quá → return 0        [1950-1961]
  ② lru_add_drain()        ← đẩy page đang chờ trong per-CPU pagevec vào LRU thật
  ③ [lock] isolate_lru_pages → page_list ; NR_ISOLATED += nr_taken ;
           PGSCAN_{KSWAPD|DIRECT}, PGSCAN_{ANON|FILE} += nr_scanned [unlock]   [1965-1977]
  ④ nr_taken == 0 → return 0
  ⑤ nr_reclaimed = shrink_page_list(page_list, …, &stat, false)  ← QUYẾT ĐỊNH TỪNG PAGE
  ⑥ [lock] move_pages_to_lru(page_list)    ← trả page CHƯA free về LRU
           NR_ISOLATED -= nr_taken ; lru_note_cost(file, stat.nr_pageout)
           PGSTEAL_{KSWAPD|DIRECT}, PGSTEAL_{ANON|FILE} += nr_reclaimed [unlock]
  ⑦ free_unref_page_list(page_list)       ← giải phóng page đã reclaim
  ⑧ stat.nr_unqueued_dirty == nr_taken → wakeup_flusher_threads(WB_REASON_VMSCAN)
  ⑨ sc->nr.{dirty,congested,unqueued_dirty,writeback,immediate,taken,file_taken} += stat…
  ⑩ trace_mm_vmscan_lru_shrink_inactive(...) ; return nr_reclaimed
```

| Bước | Vì sao | Nếu bỏ |
|---|---|---|
| ① `too_many_isolated` | Nhiều direct reclaimer cùng "rút" page ra, LRU còn lại nhỏ ⇒ quét quá nhanh ⇒ swap thrash, OOM sớm `[1808-1814]` | Bão reclaim |
| ② `lru_add_drain` | Page mới nằm trong per-CPU pagevec chưa trên LRU; không drain thì ứng viên thiếu | Bỏ sót page |
| ③ isolate | Làm việc ngoài lock | Giữ lock quá lâu |
| ⑥ `lru_note_cost` | Ghi "chi phí ghi page ra" để `get_scan_count` cân anon/file | Mất thông tin cân bằng |
| ⑧ wake flusher | Toàn bộ page lấy ra là dirty chưa vào hàng đợi I/O ⇒ flusher đang ngủ không làm việc ⇒ "đẩy" nó `[2001-2013]` | Reclaim chờ mãi dirty page |

Output quan trọng cho `shrink_node`: `sc->nr.*` — "đo triệu chứng ở đuôi LRU" (nhiều dirty? writeback?) để quyết throttle (mục 12).

## 10.5. `shrink_active_list()` — **không bao giờ free page**

Công việc: **hạ cấp**. Lấy page từ đuôi active, rồi với mỗi page `[2029-2122]`:

```
page ──► không evictable?            → putback (unevictable list)
     ──► buffer_heads_over_limit?    → thử gỡ buffer (try_to_release_page(…,0))
     ──► page_referenced(page,0,…)   → ★ XOÁ bit Accessed trong mọi PTE
              ├ referenced && VM_EXEC && file page → ở lại ACTIVE (1 vòng nữa: bảo vệ code)  [2090-2094]
              └ còn lại: ClearPageActive + SetPageWorkingset → chuyển sang INACTIVE
```

> **Vì sao tách "hạ cấp" khỏi "xét xử"?** Hạ cấp làm hai việc: (1) **xoá bit Accessed** để lần quét inactive sau sẽ thấy bit *mới* (nếu bị bật lại nghĩa là page *thực sự* được dùng trong khoảng giữa), (2) cho page một **cơ hội thứ hai** trước khi bị judge. Active-scan = "đặt lại đồng hồ"; inactive-scan = "xét xử".

Hệ quả cho **debug**: `pgrefill` (số page active được xem) và `pgdeactivate` (số bị hạ cấp) cho biết kernel đang *chuẩn bị* ứng viên hay chưa.

## 10.6. Reclaim slab song song (không qua LRU)

`shrink_slab()` gọi **shrinker** của từng subsystem (dentry/inode/superblock cache…). Mỗi shrinker nói "tôi có `freeable` object" (`count_objects`), rồi kernel chọn quét `delta = (freeable >> priority) × 4 / seeks` object `[SOURCE vmscan.c:429-557]`:

| priority | delta (freeable = 10 000, seeks = 2) | có gọi `scan_objects`? |
|---|---|---|
| 12 | 4 | không (cộng dồn `nr_deferred`) |
| 10 | 18 | không |
| 8 | 78 | không |
| 6 | 312 | **có** |
| 4 | 1 250 | có |
| 0 | 20 000 | có (bị chặn ≤ `2 × freeable`) |

⇒ **slab chỉ bị đụng thật khi priority đã xuống ≲ 7**; ở áp lực nhẹ chỉ *dồn nợ* vào `nr_deferred` `[540-553]`. Quy tắc: `while (total_scan >= batch_size (128) || total_scan >= freeable)` `[521-522]`. Kết quả slab tính vào `sc->nr_reclaimed` qua `reclaim_state->reclaimed_slab` `[2797-2800]`.

---

# 11. Page Reclaim — `shrink_page_list()` quyết định số phận từng page

## 11.1. Flow 5 — cây quyết định (đúng 5.10.241)

```
Với MỖI page trong page_list (đã isolate):

 trylock_page() thất bại ──────────────────────────────────────────► KEEP   (về LRU)      [1118]
 sc->nr_scanned += nr_pages
 !page_evictable (mlock…) ─────────────────────────────────────────► ACTIVATE→ unevictable [1128]
 !sc->may_unmap && page_mapped ────────────────────────────────────► KEEP                 [1131]
 may_enter_fs = gfp&__GFP_FS  ||  (SwapCache && gfp&__GFP_IO)
 page_check_dirty_writeback → dirty?, writeback? ; cập nhật stat.nr_dirty/unqueued/congested  [1143-1160]

 ┌─ PageWriteback ? ───────────────────────────────────────────────────────────────────────┐ [1204]
 │   case 1: kswapd && PageReclaim && PGDAT_WRITEBACK  → nr_immediate++ → ACTIVATE          │
 │   case 2: writeback_throttling_sane || !PageReclaim || !may_enter_fs                     │
 │               → SetPageReclaim; nr_writeback++ → ACTIVATE                                │
 │   case 3: (legacy memcg, PageReclaim)  → wait_on_page_writeback() rồi XÉT LẠI page       │
 └──────────────────────────────────────────────────────────────────────────────────────────┘

 references = page_check_references()    (trừ khi ignore_references)                         [1240]
     PAGEREF_ACTIVATE   ───────────────────────────────────────────► ACTIVATE
     PAGEREF_KEEP       ───────────────────────────────────────────► KEEP (nr_ref_keep++)
     PAGEREF_RECLAIM / RECLAIM_CLEAN → đi tiếp

 anon & swap-backed & chưa trong swap cache:                                                  [1259]
     !(gfp & __GFP_IO)      → KEEP
     page_maybe_dma_pinned  → KEEP
     THP: không split được → ACTIVATE ; PMD-unmapped → split ngay
     add_to_swap() thất bại → ACTIVATE (THP: split rồi thử lại)
 file THP → split (thất bại → KEEP)                                                           [1298]

 page_mapped ?  → try_to_unmap()   thất bại ───────────────────────► ACTIVATE (nr_unmap_fail++) [1320-1333]

 PageDirty ? ────────────────────────────────────────────────────────────────────────────────┐ [1335]
   file page && (không phải kswapd || !PageReclaim || !PGDAT_DIRTY)                           │
        → NR_VMSCAN_IMMEDIATE++ ; SetPageReclaim → ACTIVATE   (không ghi!)                   │
   references == RECLAIM_CLEAN → KEEP                                                         │
   !may_enter_fs → KEEP        ;  !sc->may_writepage → KEEP                                   │
   pageout(page) :                                                                            │
        PAGE_KEEP → KEEP ; PAGE_ACTIVATE → ACTIVATE                                           │
        PAGE_SUCCESS → (đang writeback / vẫn dirty) → KEEP ; (ghi đồng bộ xong) → thử free    │
        PAGE_CLEAN → thử free                                                                 │
 ─────────────────────────────────────────────────────────────────────────────────────────────┘

 page_has_private (buffer_head): try_to_release_page() thất bại → ACTIVATE                    [1422]
     (nếu !mapping && page_count==1 → free ngay)

 anon lazyfree (!SwapBacked): page_ref_freeze(1) ; PageDirty → KEEP ; PGLAZYFREED++           [1443-1453]
 else  __remove_mapping(mapping, page, reclaimed=true, memcg)  thất bại → KEEP                [1454]

 ─────────────────────────────►  FREE:  nr_reclaimed += nr_pages ; thêm vào free_pages        [1458-1473]
```
Kết thúc danh sách: `mem_cgroup_uncharge_list(free)`, `try_to_unmap_flush()`, `free_unref_page_list(free)`, `list_splice(ret_pages → page_list)`, `PGACTIVATE += …` `[1504-1513]`.

## 11.2. `page_check_references()` — "page này còn dùng không?"

Hai nguồn thông tin: `referenced_ptes` (số PTE referenced, qua rmap; hàm này **xoá** bit) và `referenced_page` (cờ `PG_referenced` do `mark_page_accessed`). `[SOURCE vmscan.c:1001-1052]`

| `VM_LOCKED`? | `referenced_ptes` | `referenced_page` | Loại | Kết quả | Giải thích |
|---|---|---|---|---|---|
| có | — | — | — | `RECLAIM` | mlock "thua cuộc đua isolation"; `try_to_unmap` sẽ chuyển page sang unevictable |
| không | **> 0** | có **hoặc** ptes > 1 | bất kỳ | **ACTIVATE** | được dùng ≥ 2 lần |
| không | > 0 (đúng 1) | không | file-backed **và** `VM_EXEC` | **ACTIVATE** | code chương trình: kích hoạt ngay sau lần dùng đầu |
| không | > 0 (đúng 1) | không | khác | **KEEP** (và `SetPageReferenced`) | "đã dùng 1 lần" — spare, đi thêm 1 vòng inactive; lần sau thấy nữa thì lên active |
| không | 0 | có | **file** (`!SwapBacked`) | `RECLAIM_CLEAN` | reclaim **nếu sạch**; nếu bẩn thì bỏ qua cho writeback lo |
| không | 0 | không/khác | — | `RECLAIM` | không ai dùng ⇒ reclaim |

Comment gốc cho dòng "KEEP": "*All mapped pages start out with page table references from the instantiating fault, so we need to look twice if a mapped file page is used more than once*" `[1019-1032]`.

## 11.3. Bốn kết cục của một page

| Kết cục | Nhãn trong code | Chuyện gì xảy ra | Counter/stat |
|---|---|---|---|
| **FREE** | `free_it` | Đã tháo khỏi mapping; thêm vào `free_pages`; trả cho buddy qua per-CPU list | `nr_reclaimed`, `PGSTEAL_*` |
| **KEEP** | `keep_locked` / `keep` | Trả về chính list (đuôi→đầu, "xoay"), thử lại lần sau | `nr_ref_keep` |
| **ACTIVATE** | `activate_locked` | `SetPageActive` (trừ page `Mlocked`) → sang active; có thể `try_to_free_swap` | `stat.nr_activate[file]`, `PGACTIVATE` |
| **WRITE** | `pageout()` | Gửi I/O; page ở lại chờ hoàn tất | `nr_pageout`, `NR_VMSCAN_WRITE` |

### FREE thật sự làm gì: `__remove_mapping()` `[869-957]`

```
__remove_mapping(mapping, page, reclaimed, memcg)
  khoá xa_lock_irqsave(&mapping->i_pages)
  page_ref_freeze(page, 1 + compound_nr)   ← "chỉ MÌNH tôi + page cache giữ ref?"  Nếu có ai khác giữ (pin, GUP, I/O) → FAIL
  PageDirty ?  → unfreeze, FAIL            ← kiểm tra Dirty SAU refcount (tránh race GUP+dirty; comment 880-903)
  ├ PageSwapCache : mem_cgroup_swapout ; shadow = workingset_eviction ; __delete_from_swap_cache ; put_swap_page
  └ page cache    : shadow = workingset_eviction (nếu file LRU, mapping không exiting, không DAX)
                    __delete_from_page_cache(page, shadow) ; freepage()
  → return 1 (page refcount = 0, sẵn sàng free)
```
Ý nghĩa: **đây là điểm "không thể quay lại"**. Nó cũng là nơi **các page bị pin thất bại** (refcount thừa) — page đó chỉ bị `KEEP`.

## 11.4. Các trường hợp đặc biệt

| Loại page | Điều đặc biệt | Rủi ro/nút thắt |
|---|---|---|
| **Anon (không trong swap cache)** | Phải `add_to_swap()` trước (cấp slot + vào swap cache), rồi `try_to_unmap` thay PTE bằng *swap entry*, rồi `pageout` ghi ra swap | **Không có swap ⇒ `add_to_swap` fail ⇒ `ACTIVATE`** (vì thế `get_scan_count` loại anon khi hết swap để khỏi quét vô ích) |
| **THP** | Có thể phải **split** trước (`split_huge_page_to_list`); không split được (pinned) ⇒ ACTIVATE | `THP_SWPOUT_FALLBACK`; board `THP=always` nên có thật |
| **Lazyfree** (`MADV_FREE`, anon !SwapBacked) | Nếu chưa bị ghi lại thì **vứt thẳng**, không cần swap | `PGLAZYFREED` |
| **tmpfs/shmem** | Là anon-LRU nhưng `PageSwapBacked`; reclaim qua `shmem_writepage` | Không có swap: `shmem_writepage` trả `AOP_WRITEPAGE_ACTIVATE` → `pageout()` = `PAGE_ACTIVATE` `[SOURCE shmem.c:1380,1465; vmscan.c:848-851]` |
| **Page có `buffer_head`** | `try_to_release_page` gỡ buffer; page "bẩn" nhưng buffer sạch vẫn free được | fs không cho gỡ ⇒ ACTIVATE |
| **Mapped file page** | `try_to_unmap` gỡ PTE (gom TLB flush bằng `TTU_BATCH_FLUSH`, flush ở `try_to_unmap_flush`) | thất bại ⇒ ACTIVATE (`nr_unmap_fail`) |
| **Pinned (DMA/GUP)** | `page_maybe_dma_pinned` (anon) hoặc refcount thừa ở `__remove_mapping` | KEEP — *reclaim không thể thắng một page đang bị pin* |

## 11.5. Sau khi FREE: page đi đâu?

```
free_pages (list)
   │ mem_cgroup_uncharge_list         ← trả "tiền" memcg
   │ try_to_unmap_flush               ← flush TLB gom
   ▼ free_unref_page_list
 per-CPU free list (pcp)  ──(đầy / drain_all_pages)──►  buddy allocator (free_area)
```
Hệ quả: *đã reclaim* ≠ *allocator thấy ngay trong buddy*; sau direct reclaim, allocator `drain_all_pages()` rồi thử lại `[page_alloc.c:4444-4449]`.

---

# 12. Dirty / Clean pages

## 12.1. Vấn đề

> Page dirty **không được vứt**. Nhưng "ghi từ reclaim" lại tệ: nó chậm, dễ tràn stack, chen ngang flusher, và có thể sinh I/O nhỏ lẻ kém hiệu quả.

Cách `vmscan.c` giải: **né ghi hết mức có thể; nhường cho flusher thread; chỉ kswapd mới được "tự tay" ghi, và chỉ khi tình hình đủ xấu.** `[SOURCE comment 1336-1345]`

## 12.2. Flow 6 — Dirty page handling

```
Dirty FILE page ở đuôi inactive
        │
        ▼ shrink_page_list
 [có phải kswapd?] [PageReclaim đã đặt?] [node có cờ PGDAT_DIRTY?]    → cả ba đều CÓ ?
        │ KHÔNG (đa số lần)                                   │ CÓ (tình hình đủ xấu)
        ▼                                                     ▼
 SetPageReclaim ;  NR_VMSCAN_IMMEDIATE++               references==RECLAIM_CLEAN → keep
 → ACTIVATE (đưa ra khỏi đường đi,                     !may_enter_fs / !may_writepage → keep
   tiếp tục tìm page SẠCH)                             pageout(page):
        │                                                 may_write_to_inode ? (PF_SWAPWRITE của kswapd ⇒ luôn có)
        │ (cuối lô)                                       clear_page_dirty_for_io; SetPageReclaim
        ▼                                                 ->writepage(wbc{WB_SYNC_NONE, for_reclaim})
 stat.nr_unqueued_dirty == nr_taken ?                     NR_VMSCAN_WRITE++ → PAGE_SUCCESS
        │ CÓ                                                │
        ▼                                                   ▼
 wakeup_flusher_threads(WB_REASON_VMSCAN)           I/O hoàn tất → end_page_writeback():
        │                                              PageReclaim ? → rotate_reclaimable_page()
        ▼                                              → ClearPageActive + đưa về ĐUÔI inactive
 flusher ghi page (hiệu quả hơn)                       → lần quét sau page đã SẠCH → FREE
```
`[SOURCE vmscan.c:1335-1399, 2012-2013; swap.c:260-277, pagevec_move_tail_fn; filemap.c:1486-1488]`

### Các cờ trạng thái cấp node (do `shrink_node` đặt dựa trên `sc->nr.*`) `[2810-2867]`

| Cờ | Đặt khi (chỉ kswapd) | Nghĩa | Ảnh hưởng |
|---|---|---|---|
| `PGDAT_WRITEBACK` | `nr.writeback > 0 && nr.writeback == nr.taken` | *Toàn bộ page lấy ra đều đang writeback* ⇒ tốc độ allocation > tốc độ writeback | Lần sau kswapd gặp `PageReclaim`+writeback ⇒ `nr_immediate` ⇒ **kswapd `congestion_wait(HZ/10)`** |
| `PGDAT_DIRTY` | `nr.unqueued_dirty == nr.file_taken` | Đuôi LRU toàn dirty chưa xếp hàng I/O | **Cho phép kswapd ghi** trang (mục trên) |
| `LRUVEC_CONGESTED` | (kswapd *hoặc* memcg "sane") `nr.dirty > 0 && nr.dirty == nr.congested` | Mọi dirty page quét được đều thuộc BDI đang nghẽn | Direct reclaim sẽ `wait_iff_congested(HZ/10)` |
| *(xoá)* | `clear_pgdat_congested()` khi `prepare_kswapd_sleep` thấy balanced; memcg: cuối `do_try_to_free_pages` | | |

## 12.3. Anon "dirty"

Anon page **không có flusher**; chúng phải được ghi từ chính reclaim (`pageout` → swap). Vì thế `page_check_dirty_writeback()` cố ý báo *không dirty, không writeback* cho anon (kể cả lazyfree) — "*Anonymous pages are not handled by flushers and must be written from reclaim context. Do not stall reclaim based on them*" `[1061-1069]`. Direct reclaimer **được** ghi anon/swap (nếu `may_enter_fs`, `may_writepage`); **không** được ghi file.

## 12.4. Điều kiện để `pageout()` thực sự ghi `[795-863]`

```
!is_page_cache_freeable(page)         → PAGE_KEEP   (có ref thừa)
!mapping (orphan, có buffer): try_to_free_buffers OK → PAGE_CLEAN  + pr_info("%s: orphaned page")
mapping->a_ops->writepage == NULL     → PAGE_ACTIVATE
!may_write_to_inode(host)             → PAGE_KEEP   (bdi đang congested và task không có PF_SWAPWRITE…)
clear_page_dirty_for_io(page) ?       → ghi: SetPageReclaim → writepage → PAGE_SUCCESS
                                         (AOP_WRITEPAGE_ACTIVATE → PAGE_ACTIVATE ; res<0 → handle_write_error)
ngược lại                             → PAGE_CLEAN  (đã sạch)
```
`may_write_to_inode`: `PF_SWAPWRITE` (kswapd luôn có) **hoặc** bdi không congested **hoặc** bdi chính là `current->backing_dev_info` `[747-756]`.

---

# 13. Reclaim Progress — scan progress ≠ reclaim progress

## 13.1. Định nghĩa

| Đại lượng | Là gì | Ở đâu |
|---|---|---|
| **Scanned** | Số page đã *xem xét* (isolate) | `sc->nr_scanned`; `pgscan_{kswapd,direct,anon,file}`; `pgscan_skip_*` |
| **Reclaimed** | Số page thật sự *giải phóng* (kể cả slab) | `sc->nr_reclaimed`; `pgsteal_{kswapd,direct,anon,file}` |
| **Efficiency** | `reclaimed / scanned` | `vmpressure()` biến thành `low/medium/critical` |
| **Progress** (theo allocator) | `try_to_free_pages` trả > 0 | `did_some_progress` |
| **No progress** | Trả 0 | → `no_progress_loops++` |

`vmpressure()` định nghĩa: `pressure ≈ (1 − reclaimed/scanned) × 100` (tính nguyên: `scale = scanned+reclaimed`; nếu `reclaimed ≥ scanned` ⇒ 0 — vì slab cộng vào reclaimed mà không cộng vào scanned), cửa sổ 512 page quét; ngưỡng medium **60**, critical **95** `[SOURCE mm/vmpressure.c:38-47, 121-148]` (ví dụ script: scanned 512, reclaimed 200 → 60% *medium*; reclaimed 20 → 96% *critical*). Board **không bật PSI**, `vmpressure` chỉ lộ qua cgroup v1 `memory.pressure_level`.

## 13.2. Ai tiêu thụ "tiến độ"

| Người dùng | Cách dùng |
|---|---|
| `do_try_to_free_pages` | `nr_reclaimed ≥ nr_to_reclaim` ⇒ dừng; không thì `--priority` |
| `balance_pgdat` | `raise_priority`/`!nr_reclaimed` ⇒ `--priority`; tổng 0 ⇒ `kswapd_failures++` |
| `should_continue_reclaim` | Quyết định reclaim thêm *cho high-order* (xem dưới) |
| `shrink_node` | `reclaimable ⇒ kswapd_failures = 0`; throttle theo `sc->nr.*` |
| `page_alloc.c should_reclaim_retry` | `did_some_progress` ⇒ reset `no_progress_loops`; không ⇒ `++`, quá 16 ⇒ OOM |

## 13.3. `should_continue_reclaim()` — *reclaim cho high-order còn đáng làm không?*

```
should_continue_reclaim(pgdat, nr_reclaimed_vòng_này, sc)               [2581-2632]
   !in_reclaim_compaction (order==0, hoặc order≤3 và priority ≥ 10) ──► false
   nr_reclaimed == 0 ──────────────────────────────────────────────► false   (bỏ cuộc, đừng cố)
   có zone nào compaction_suitable() là SUCCESS/CONTINUE ──────────► false   (đủ free để compaction làm)
   ngược lại: true  ⇔  (inactive_file + inactive_anon nếu có swap) > compact_gap(order) = 2<<order
```
Ví dụ `compact_gap`: order 3 → 16, order 4 → 32, order 9 (THP 2MiB) → 1024 page `[script]`. Nếu `true`, `shrink_node` `goto again` ⇒ **lặp lại cả node** trước khi trả về `[2869-2871]`.

## 13.4. Flow 7 — Reclaim failure / no progress

```
Một vòng shrink_node thu 0 page
        │
        ├─ kswapd:  priority-- (leo thang) ──► đến priority 1 mà vẫn 0 ──► kswapd_failures++ ──► ... ≥16 ──► HOPELESS
        │                                                                                         (kswapd ngủ, direct reclaim tự lo)
        └─ direct:  --priority ... ──► 0 mà vẫn 0 ──► do_try_to_free_pages return 0
                         │
                         ▼
            page_alloc: __alloc_pages_direct_reclaim → NULL
                         │
         ┌───────────────┴───────────────────────────┐
         ▼                                           ▼
 should_reclaim_retry():                      should_compact_retry():
   no_progress_loops++ (>16 → bỏ)               (chỉ khi did_some_progress>0)
   "nếu free HẾT reclaimable thì đủ
    watermark?" → có: retry                    cuối cùng: __alloc_pages_may_oom()
                  không: OOM                            │
                                                         ▼
                                              OOM killer → giết task → page_alloc retry
```

### Vì sao reclaim có thể *quét nhiều mà thu ít*? (tóm tắt, chi tiết ở file 05 Scenario 3)

| Nguyên nhân | Dấu hiệu trong counter |
|---|---|
| Page **referenced** (nóng) → ACTIVATE/KEEP | `pgactivate` ↑, `nr_ref_keep` (tracepoint) ↑ |
| Page **dirty/writeback** | `nr_vmscan_immediate_reclaim` ↑, meminfo `Dirty/Writeback` cao |
| **Không có swap**, anon chiếm đa số | `pgscan_anon = 0`, chỉ `pgscan_file`; thu ít vì file cache cạn |
| **Zone-skip** | `pgscan_skip_*` ↑ |
| **Pinned / refcount thừa** | `__remove_mapping` fail (không có counter riêng) |
| **try_to_unmap fail** | `nr_unmap_fail` (tracepoint) |
| **THP không split được** | `thp_swpout_fallback` |
| `too_many_isolated` | task ngủ 100 ms, trả 0 |
| `!may_enter_fs` (GFP_NOFS/NOIO) | không ghi/giải phóng page có buffer/FS |

Tiếp theo → [`04_function_analysis.md`](04_function_analysis.md).
