# 06 — Các vấn đề thiết kế (Phần IV: §12 Vì sao nhiều lớp · §13 Concurrency & synchronization)

---

# §12. Vì sao Swap subsystem cần nhiều lớp quản lý?

Một hàm "ghi page ra đĩa / đọc page về" là không đủ, vì mỗi việc dưới đây có **dữ liệu riêng, thời điểm riêng, người gọi riêng và quy tắc khoá riêng**. Gộp lại sẽ buộc mọi đường (reclaim, fault, fork, exit, swapoff, memcg) phải giữ cùng một khoá lớn và chạy cùng một tốc độ.

| Nhiệm vụ | 1. Giải quyết vấn đề gì | 2. Vì sao không gộp | 3. Dữ liệu quản lý | 4. Phối hợp với | 5. File |
|---|---|---|---|---|---|
| **Chọn page cần reclaim** | Thu hồi page ít khả năng được dùng sớm | Áp dụng cho *mọi* loại page (file, anon, shmem); swap chỉ là một nhánh. Logic chọn (LRU, references, swappiness, memcg) độc lập với cách cất dữ liệu | LRU list, cờ `PG_active/referenced`, `lruvec->anon_cost/file_cost` | swap cache (`add_to_swap`), rmap, I/O | `mm/vmscan.c`, LRU ở `mm/swap.c` |
| **Cấp swap entry** | Chọn một ô trống trên đúng area, nhanh, ít phân mảnh | Cấp xảy ra cực nhiều lần và trên mọi CPU; cần cache per-CPU và thuật toán cluster riêng; người gọi không cần biết area nào | `swap_avail_heads`, `nr_swap_pages`, per-CPU `slots[]`, `percpu_cluster` | memcg (charge), swap cache | `mm/swap_slots.c`, `mm/swapfile.c` (`get_swap_pages`, `scan_swap_map_slots`) |
| **Quản lý slot đã dùng** | Biết chính xác bao nhiêu tham chiếu tới mỗi slot để free đúng lúc | Tham chiếu đến từ nhiều nguồn ở các thời điểm khác nhau: unmap (rmap), fork, fault, exit, shmem, swap cache | `swap_map[]` (count + `SWAP_HAS_CACHE`), continuation page, `cluster_info` | rmap, memory.c, swap_state.c | `mm/swapfile.c` |
| **Giữ page trong swap cache** | Một nguồn duy nhất cho page trong RAM của một slot; che khoảng I/O; tránh ghi lại | Phải tồn tại *song song* với mapping của process và kéo dài qua nhiều lượt reclaim/fault; cần tra cứu theo (type, offset) | `swapper_spaces[][]` (xarray), `page->private`, `PG_swapcache`, shadow entry | vmscan, fault, readahead, swapoff | `mm/swap_state.c` |
| **Thực hiện I/O** | Chuyển 4 KiB giữa frame và vị trí đúng trên thiết bị | Phụ thuộc loại backend (block device, file qua FS, `rw_page` đồng bộ, frontswap); bất đồng bộ, completion ở ngữ cảnh khác | swap extent (offset → sector), `bio` | block layer; swap cache (page lock, `PG_writeback`) | `mm/page_io.c`, extent ở `mm/swapfile.c` |
| **Quản lý thông tin cgroup** | Tính swap đúng nhóm, kể cả khi page không còn trong RAM | Là chính sách tuỳ chọn (có thể tắt); vòng đời theo slot, không theo page; v1/v2 khác thời điểm | `swap_cgroup_ctrl[type].map` (2 byte/slot) | memcontrol.c, swap_slots (charge), swapfile (uncharge), swap-in (lookup) | `mm/swap_cgroup.c` |
| **Giải phóng slot** | Trả slot về trạng thái trống an toàn, theo lô | Free phải chờ *cả* count về 0 *và* page rời cache; được gọi trong ngữ cảnh giữ spinlock (PTE lock) nên không được ngủ; trả theo lô để giảm khoá và phân mảnh | `slots_ret[]`, `swap_map`, `inuse_pages`, `lowest/highest_bit` | memcg (uncharge), discard, swap cache (xoá shadow) | `mm/swap_slots.c` (`free_swap_slot`), `mm/swapfile.c` (`swapcache_free_entries`, `swap_entry_free`) |

**Sự phân chia trách nhiệm thực tế khác "mô hình sách vở" ở vài điểm (theo code):**
- *Cấp slot* không nằm trọn trong một file: API công khai `get_swap_page()` ở `swap_slots.c`, nhưng lõi `get_swap_pages()/scan_swap_map_slots()` ở `swapfile.c`.
- *Swap cache* không chỉ là "cache": `SWAP_HAS_CACHE` trong `swap_map` (swapfile.c) là một nửa của quan hệ; nửa còn lại là xarray (swap_state.c). Hai nửa phải được cập nhật nhất quán: thêm vào cache cần cờ trước (`swapcache_prepare`), rời cache bỏ cờ sau (`put_swap_page`).
- *Giải phóng frame* không thuộc swap subsystem: đó là `vmscan.c` (`__remove_mapping` + `free_unref_page_list`) hoặc `swap.c` (`release_pages`).
- `mm/swap.c` gần như không thuộc swap subsystem (`02` §6.1).

---

# §13. Concurrency và synchronization

## 13.1. Vì sao nhiều CPU có thể đồng thời cấp/giải phóng slot?

Board có `CONFIG_NR_CPUS=4` `[CONFIG]`. Tại cùng một thời điểm có thể có:
- `kswapd0` đang reclaim trên CPU0 (gọi `get_swap_page`);
- hai task đang direct reclaim trên CPU1, CPU2 (cũng gọi `get_swap_page`);
- một process đang exit trên CPU3 (gọi `free_swap_and_cache` cho hàng nghìn swap PTE);
- một task khác đang fault swap-in (gọi `swapcache_prepare`, `swap_free`);
- và có thể ai đó chạy `swapoff`.

Tất cả cùng đọc/ghi `swap_map`, `nr_swap_pages`, xarray swap cache, page table.

## 13.2. Race condition xuất hiện thế nào? — Ví dụ hai CPU cùng lấy một slot

Mô phỏng **không có khoá** `[SIM tools/swap_map_sim.out.txt kịch bản 5]`:

```
ban đầu swap_map = [3f 40 40 00 00]
CPU0: đọc swap_map[3] == 0  → true
CPU1: đọc swap_map[3] == 0  → true        (CPU0 chưa kịp ghi)
CPU0: ghi swap_map[3] = 0x40, giao slot 3 cho page P
CPU1: ghi swap_map[3] = 0x40, giao slot 3 cho page Q
→ P và Q cùng được ghi vào slot 3; một trong hai bị ghi đè trên đĩa.
→ Về sau process của P fault, đọc slot 3 → nhận dữ liệu của Q: hỏng bộ nhớ âm thầm.
```

**Cách source ngăn chặn:** đọc và ghi byte nằm trong **cùng một vùng găng**: `scan_swap_map_slots()` được gọi với `si->lock` đã giữ (`spin_lock(&si->lock)` trong `get_swap_pages()` `[SOURCE mm/swapfile.c:1085]`), và với SSD còn khoá cluster: `ci = lock_cluster(si, offset); ... if (si->swap_map[offset]) {...} WRITE_ONCE(si->swap_map[offset], usage); ... unlock_cluster(ci);` `[SOURCE mm/swapfile.c:872-895]`. CPU thứ hai phải chờ khoá, và khi vào được thì thấy byte ≠ 0.

Ngoài ra, các vòng quét "nhìn trộm" **không khoá** (`data_race(!si->swap_map[offset])` `[:953, :970]`) chỉ để *tìm ứng viên*, sau đó **luôn lấy lại khoá và kiểm tra lại** ở `checks:` trước khi ghi `[:954-955, :854-892]` — mẫu "optimistic scan + locked re-check".

Ví dụ race khác mà code xử lý (đã trình bày ở `04` §10.3): hai task cùng đọc một entry về swap cache — giải bằng `SWAP_HAS_CACHE` (`swapcache_prepare` trả `-EEXIST` cho kẻ đến sau).

## 13.3. Vì sao dữ liệu quản lý slot cần được bảo vệ?
Vì một lỗi đồng bộ ở đây không gây crash ngay mà gây **hỏng dữ liệu người dùng** (sai slot, slot bị free khi còn PTE trỏ → slot cấp cho page khác → PTE cũ đọc nhầm dữ liệu) hoặc **rò rỉ** (count không về 0 → slot không bao giờ free, `swapoff` lặp vô hạn). Kernel có rất nhiều `VM_BUG_ON` canh các bất biến này, ví dụ `VM_BUG_ON(count != SWAP_HAS_CACHE)` trong `swap_entry_free()` `[SOURCE mm/swapfile.c:1370]` (chỉ có hiệu lực khi `CONFIG_DEBUG_VM`, **tắt** trên board `[CONFIG]`).

## 13.4. Mutex khác spinlock thế nào? Khi nào được ngủ?

| | Spinlock | Mutex |
|---|---|---|
| Khi khoá bận | **quay vòng** chờ (busy-wait) trên CPU | **ngủ**: task bị đưa ra khỏi CPU, CPU làm việc khác |
| Được ngủ khi đang giữ? | **Không** (giữ spinlock = tắt preemption; ngủ sẽ có thể deadlock) | Có |
| Dùng được trong ngữ cảnh ngắt / atomic? | Có (biến thể `_irq`, `_irqsave` tắt ngắt cục bộ) | Không |
| Chi phí | rất rẻ nếu vùng găng ngắn | đắt hơn (lập lịch) nhưng không đốt CPU khi chờ lâu |
| Phù hợp | vùng găng ngắn, không gọi hàm có thể ngủ | vùng găng có thể ngủ (cấp phát `GFP_KERNEL`, I/O, `cond_resched`) |

**Được ngủ khi:** đang ở ngữ cảnh process, không giữ spinlock, không tắt ngắt/preempt, không trong RCU read-side (trừ SRCU) `[KIẾN THỨC NỀN]`. **Không được ngủ khi:** trong interrupt/softirq, khi giữ spinlock (PTE lock, `si->lock`, xarray lock…), trong `rcu_read_lock()` thường.

Hệ quả cụ thể thấy trong code swap:
- `swap_duplicate()` được gọi khi **đang giữ PTE lock** (trong `try_to_unmap_one`, `copy_nonpresent_pte`) → nếu cần continuation page phải cấp `GFP_ATOMIC` (không ngủ) `[SOURCE mm/swapfile.c:3553-3559]`; comment `:3613-3615` nói rõ có thể gọi lại với `GFP_KERNEL` sau khi nhả khoá.
- `scan_swap_map_slots()` muốn `cond_resched()` (có thể ngủ) → phải **nhả `si->lock` trước** (`spin_unlock(&si->lock); cond_resched(); spin_lock(&si->lock);` `[SOURCE mm/swapfile.c:907-914]`). Chính vì đường này có thể ngủ, `swap_slots.c` bảo vệ đường cấp bằng **mutex** — comment *"The swap slots cache is protected by a mutex instead of a spin lock as when we search for slots with scan_swap_map, we can possibly sleep"* `[SOURCE mm/swap_slots.c:25-27]`.
- `free_swap_slot()` có thể được gọi khi người gọi giữ spinlock (ví dụ `free_swap_and_cache()` trong `zap_pte_range()` khi giữ PTE lock `[SOURCE mm/memory.c:1342]`) → đường trả dùng **spinlock** `free_lock` và không bao giờ ngủ.
- `alloc_swap_slot_cache()` cấp mảng bằng `kvcalloc(GFP_KERNEL)` **ngoài** `swap_slots_cache_mutex` vì cấp phát có thể reclaim → `get_swap_page()` → có thể cần chính mutex đó → deadlock `[SOURCE mm/swap_slots.c:119-123]`.

## 13.5. Các khoá cụ thể trong source và dữ liệu chúng bảo vệ

> Chỉ liệt kê những gì đã kiểm tra đường truy cập hoặc có comment trong source.

| Khoá | Loại | Bảo vệ (bằng chứng) |
|---|---|---|
| `swap_lock` | spinlock toàn cục `[swapfile.c:52]` | `swap_info[]`/`nr_swapfiles` khi cấp (`alloc_swap_info` `:2910-2943`), `swap_active_head` (`:70-74`), `total_swap_pages` (`:61-62`), thay đổi `si->flags` (cùng `si->lock`; comment `swap.h:265-277`: *"changing flags need hold this lock and swap_lock. If both locks need hold, hold swap_lock first"*) |
| `swap_avail_lock` | spinlock `[:89]` | `swap_avail_heads[]`; tách riêng vì area chuyển đầy/không đầy khi **đang giữ `si->lock`**, mà thứ tự khoá bắt buộc `swap_lock` trước `si->lock` (comment `:76-87`) |
| `si->lock` | spinlock / area | "map scan related fields like swap_map, lowest_bit, highest_bit, inuse_pages, cluster_next, cluster_nr, ... free/discard cluster list" (comment `swap.h:265-277`) |
| `ci->lock` | spinlock / cluster 256 slot | "Protect swap_cluster_info fields and swap_info_struct->swap_map elements correspond to the swap cluster" (`swap.h:209-214`); chỉ có khi non-rotational; HDD dùng `si->lock` thay thế (`lock_cluster_or_swap_info` `:373-394`) |
| `si->cont_lock` | spinlock | danh sách continuation page (`swap.h:278-281`; `:3672, :3743`) |
| `swapon_mutex` | mutex `[:93]` | giai đoạn cuối swapon/swapoff và duyệt `/proc/swaps` (`swap_start`/`swap_stop` `:2785, :2822`) |
| `mmlist_lock` | spinlock | `init_mm.mmlist` (`rmap.c:1700-1705`, `memory.c:720-726`, `try_to_unuse` `:2216-2242`) |
| `cache->alloc_lock` | mutex / CPU | `slots`, `nr`, `cur` (`swap_slots.h:15`) |
| `cache->free_lock` | spinlock (`_irq`) / CPU | `slots_ret`, `n_ret` (`swap_slots.h:19`) |
| `swap_slots_cache_mutex` | mutex | `swap_slot_cache_active`, cấp/free mảng per-CPU (`swap_slots.c:41, :53-66, :136-165, :232-238`) |
| `swap_slots_cache_enable_mutex` | mutex | "Serialize swap slots cache enable/disable operations" (`swap_slots.c:42-43`); giữ suốt `try_to_unuse()` trong swapoff |
| xarray lock của `swapper_spaces[..]` | spinlock (`xas_lock_irq`/`xa_lock_irq`) | nội dung swap cache (`swap_state.c:147, :277, :297`; `vmscan.c:879`) |
| `ctrl->lock` | spinlock (`irqsave`) / area | đọc-sửa-ghi ID trong `swap_cgroup_record`/`cmpxchg` (`swap_cgroup.c:107, :138`) |
| `swap_cgroup_mutex` | mutex | dựng/huỷ `ctrl->map` (`swap_cgroup.c:8, :182-194, :209-215`) |
| Page lock `PG_locked` | bit-lock có thể ngủ | thêm/xoá swap cache, `try_to_free_swap`, `reuse_swap_page`; chờ I/O đọc |
| PTE lock (`pte_offset_map_lock`) | spinlock | đọc-so sánh-ghi PTE (`do_swap_page :3497`, `unuse_pte :1945`) |
| RCU + `SWP_VALID` | RCU | `get_swap_device()`/`put_swap_device()` vs `swapoff`/`swapon` (`:1278-1344, :2553, :2678`) |
| `SWP_SCANNING` | **bộ đếm trong `flags`** (không phải khoá) | swapoff chờ mọi người đang quét thoát (`:799, :947, :989, :2695-2702`) |
| `SWAP_HAS_CACHE` | **bit trong `swap_map`** (không phải khoá) | loại trừ "ai được thêm page vào swap cache" (`swapcache_prepare`) |

Thứ tự khoá tổng quát của MM (comment đầu `mm/rmap.c:20-42`) đặt PTE lock **bên ngoài** các khoá swap: `mm->page_table_lock or pte_lock → ... swap_lock (in swap_duplicate, swap_info_get) → mmlist_lock`. Lưu ý comment này có từ trước khi có khoá cluster: trong code 5.10, `swap_duplicate()` thực tế lấy `ci->lock` hoặc `si->lock` (`__swap_duplicate` `:3487`), không lấy `swap_lock` — nhưng ý chính vẫn đúng: **khoá swap luôn được lấy khi đã giữ PTE lock**, nên chúng phải là spinlock.

## 13.6. Vì sao `swap_slots.c` dùng cache riêng cho từng CPU?

1. **Giảm tranh chấp:** không cache → mỗi slot = `swap_avail_lock` + `si->lock` (toàn cục). Có cache → 1 lần khoá toàn cục cho 64 slot; giữa hai lần nạp chỉ khoá `alloc_lock` *của riêng CPU đó* (gần như không bao giờ bị tranh).
2. **Cục bộ dữ liệu:** mảng per-CPU nằm trong cache L1/L2 của CPU đó.
3. **Gom trả:** `slots_ret` gom 64 slot → 1 lần `swapcache_free_entries()` (sắp xếp theo area để mỗi `si->lock` chỉ lấy một lần `[SOURCE mm/swapfile.c:1477-1483]`).
4. **Ít phân mảnh:** slot liền kề được trả cùng lúc.

So sánh với `percpu_cluster` (swapfile.c): đó là một cơ chế per-CPU **khác**, ở tầng thấp hơn — mỗi CPU quét trong cluster 256 slot riêng của mình để các CPU không giẫm lên cùng cluster lock và để ghi tuần tự trên SSD `[SOURCE include/linux/swap.h:222-230]`. Hai cơ chế chồng lên nhau: túi 64 slot của `swap_slots.c` thường được nạp từ cluster riêng của CPU đó.

## 13.7. Vì sao phải drain khi disable cache hoặc swapoff?

- **swapoff:** slot của area đang gỡ có thể nằm trong `slots[]` (đã đặt chỗ, `0x40`) hoặc `slots_ret[]` (đã trả, `0x40`) của *bất kỳ* CPU nào. Chúng được tính là `inuse_pages` → `try_to_unuse()` không thể kết thúc nếu không drain; và sau khi gỡ area, một CPU có thể cấp ra entry trỏ vào area không còn tồn tại. `disable_swap_slots_cache_lock()` drain dưới `get_online_cpus()` (chặn hotplug) `[SOURCE mm/swap_slots.c:69-79]`.
- **Swap thấp (deactivate):** slot bị "giam" trong túi các CPU khác không phục vụ được CPU đang thiếu `[SIM kịch bản 3]`. `deactivate_swap_slots_cache()` trả hết về pool, từ đó cấp trực tiếp `[SIM kịch bản 4]`: với ngưỡng thu nhỏ, lúc `nr_swap_pages = 15 < 16`, cache tắt, 6 slot trong túi được trả về (15 + 6 = 21), rồi cấp trực tiếp 1 slot → 20.
- **CPU offline:** túi của CPU đã chết không ai dùng → `free_slot_cache()` `[SOURCE mm/swap_slots.c:232-238]`.
- Không lấy cpu-hotplug lock trong `__drain_swap_slots_cache()` vì nó có thể được gọi *từ* đường hotplug (hotplug → cấp phát → reclaim → `get_swap_page` → deactivate → drain) — comment `[SOURCE mm/swap_slots.c:214-226]` lập luận vì sao bỏ sót CPU đang online dở là vô hại.

## 13.8. `SWAP_HAS_CACHE` giải quyết vấn đề gì (theo comment và logic)?

1. **Không cấp lại slot đã được đặt chỗ hoặc đang chờ trả:** *"The swap entry allocated is marked with SWAP_HAS_CACHE flag in map_count that prevents it from being allocated again from the global pool"* `[SOURCE mm/swap_slots.c:21-23]`; `free_swap_slot`: *"The current swap_map value is SWAP_HAS_CACHE. Set it to 0 to indicate it is available for allocation in global pool"* `[:288-292]`.
2. **Một slot ↔ tối đa một page trong RAM:** `swapcache_prepare()` là phép "test-and-set" dưới khoá cluster/area: chỉ một người đặt được cờ; người khác nhận `-EEXIST` và chờ `[SOURCE mm/swapfile.c:3504-3512; mm/swap_state.c:497-516]`.
3. **Cho phép tái chế slot "chỉ còn cache" khi swap đầy** `[SOURCE mm/swapfile.c:873-884]`.
4. **Giữ slot trong khoảng giữa:** sau cấp phát (count 0, chưa PTE) và sau khi PTE cuối cùng map lại (count 0, page còn trong cache).

## 13.9. Tóm tắt
- Cấp/giải phóng slot xảy ra song song trên mọi CPU; byte `swap_map` chỉ được ghi dưới `si->lock`/`ci->lock` với kiểm tra lại sau khi lấy khoá.
- Đường cấp có thể ngủ → mutex (`alloc_lock`); đường trả không được ngủ → spinlock (`free_lock`).
- Per-CPU cache giảm khoá toàn cục 64 lần, đổi lại phải drain khi swapoff/hotplug/swap thấp.
- `SWAP_HAS_CACHE` vừa là trạng thái, vừa là cơ chế loại trừ; RCU + `SWP_VALID` và `SWP_SCANNING` bảo vệ khỏi swapoff.
