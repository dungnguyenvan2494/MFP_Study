# Linux Swap từ con số 0 — bám source thật (Linux 5.10.241, board S800)

> **Đối tượng đọc:** người mới hoàn toàn với Memory Management, muốn tự giải thích được *vòng đời của một page* khi
> Linux đưa nó ra swap, giữ nó, lấy nó về và giải phóng swap slot.
> **Nguyên tắc trình bày:** Vấn đề → Thiết kế → Khái niệm → Luồng → Dữ liệu nằm ở đâu → Trạng thái đổi thế nào → (cuối cùng) Code.
> **Ngôn ngữ:** tiếng Việt; thuật ngữ và tên function giữ nguyên tiếng Anh như trong source.

---

## 0.1. Chú giải thẻ (tag) dùng xuyên suốt

| Thẻ | Nghĩa |
|---|---|
| `[SOURCE file:line]` | Đã đọc trực tiếp trong `Kernel/K-S800/Src`, số dòng đã kiểm tra. |
| `[DIFF]` | Kết quả `diff` giữa file local và upstream `v5.10.241` (tải từ git.kernel.org). |
| `[CONFIG]` | Đọc từ `arch/arm64/configs/*_defconfig` hoặc `Kconfig` trong cây này. |
| `[KIẾN THỨC NỀN]` | Kiến thức Linux MM chung, **không** chứng minh bằng một dòng cụ thể trong 5 file chính. |
| `[SUY LUẬN]` | Suy ra từ source + config; hợp lý nhưng chưa đo trên board. |
| `[KIỂM TRA TRÊN BOARD]` | Phụ thuộc runtime/userspace, tài liệu không thể biết; phải đo. |
| `[MÔ HÌNH GIẢI THÍCH]` | Mô hình do tài liệu tự tổng hợp để dễ hiểu (ví dụ "trạng thái slot"), **không** phải enum/struct thật của kernel. |
| `[SIM]` | Con số/bảng sinh ra bởi script trong `tools/` (không tính tay). |

---

## 0.2. Bước 1 — Kiểm tra source và xác định phạm vi

### 0.2.1. Phiên bản

- `Makefile`: `VERSION = 5`, `PATCHLEVEL = 10`, `SUBLEVEL = 241` → **Linux 5.10.241** `[SOURCE Makefile:2-4]`.
- Kiến trúc: **arm64**, page 4 KiB (`CONFIG_ARM64_4K_PAGES=y`), `CONFIG_ARM64_VA_BITS=39`, `CONFIG_NR_CPUS=4`, NUMA tắt `[CONFIG km_mvebu_v8_lsp_defconfig]`.

### 0.2.2. So sánh với upstream (quan trọng: không tự giả định "giống mainline mới nhất")

Đã tải 6 file từ `git.kernel.org/.../stable/linux.git/plain/mm/<file>?h=v5.10.241` và `diff` với bản local `[DIFF]`:

| File | Kết quả diff | Ghi chú |
|---|---|---|
| `mm/swap.c` | **giống hệt** | |
| `mm/swap_state.c` | **giống hệt** | |
| `mm/swap_slots.c` | **giống hệt** | |
| `mm/swap_cgroup.c` | **giống hệt** | |
| `mm/page_io.c` | **giống hệt** | (file I/O của swap, đọc thêm) |
| `mm/swapfile.c` | **khác đúng 1 hunk** | thêm `#ifdef CONFIG_PM_WARP ... if (warp_swapout_disable) return 0;` ở đầu `scan_swap_map_slots()` `[SOURCE mm/swapfile.c:794-797]` — **VENDOR** |

Hunk vendor đó nghĩa là: khi cờ toàn cục `warp_swapout_disable` = 1 (đặt trong `kernel/power/warp.c:1972-1985`, `:2302-2304`, reset ở `:2979`), **mọi lần tìm swap slot đều trả về 0 slot** → không cấp phát được swap entry → anonymous page không thể bị swap out trong lúc đó. WARP là cơ chế hibernation/fast-boot của vendor; `warp_shrink_memory()` đọc cờ này để chọn chiến lược shrink `[SOURCE kernel/power/warp.c:1732-1741]`. Mục đích thiết kế chi tiết của WARP **không** được mô tả trong source đã đọc → `[SUY LUẬN]`: chặn ghi swap trong một pha của quá trình tạo snapshot.

> ⚠️ Lưu ý tinh tế: lệnh `return 0` nằm **trước** `si->flags += SWP_SCANNING` `[SOURCE mm/swapfile.c:794-799]`, nên nó không làm `swapoff` (vốn chờ `SWP_SCANNING` về 0, `:2696`) bị treo.

Ngoài ra (đã biết từ nghiên cứu `docs/vmscan/`): `mm/vmscan.c` có thêm hunk `CONFIG_PM_WARP` `break` trong `shrink_page_list()` `[SOURCE mm/vmscan.c:1110-1113]` và kswapd chạy `SCHED_RR` prio 200 `[SOURCE mm/vmscan.c:3901]`.

### 0.2.3. Cấu hình liên quan đến swap `[CONFIG]`

| Option | Giá trị | Hệ quả cho tài liệu |
|---|---|---|
| `CONFIG_SWAP` | `=y` ở 11 defconfig sản phẩm (Eagle*, Deneb, Sparrow*, Helios, Minerva) và `km_mvebu_v8_lsp{,_dnb,_spa}`; **không bật** ở `km_mvebu_v8_lsp_eglz`, `mvebu_v8_lsp`, `mvebu_v8_pd_lsp` | Nếu build bằng config không có `CONFIG_SWAP`, toàn bộ `swapfile.c/swap_state.c/swap_slots.c` không được biên dịch, chỉ còn stub trong `include/linux/swap.h:494+`. |
| `CONFIG_MEMCG` / `CONFIG_MEMCG_SWAP` | `=y` / tự động `y` (`init/Kconfig:923-926`: `depends on MEMCG && SWAP, default y`) | `swap_cgroup.c` được biên dịch. |
| `CONFIG_THP_SWAP` | **tắt** — `mm/Kconfig:438-440` cần `ARCH_WANTS_THP_SWAP`, mà arm64 5.10 không select | `SWAPFILE_CLUSTER = 256`, `swap_entry_size()` luôn = 1 `[SOURCE mm/swapfile.c:265-277]`; THP bị **split** trước khi swap `[SOURCE mm/vmscan.c:1265-1291]`. |
| `CONFIG_FRONTSWAP` | tắt | `frontswap_store()/frontswap_load()` là stub; không có zswap. |
| `CONFIG_ZRAM`, `CONFIG_ZSWAP` | không xuất hiện trong defconfig sản phẩm | Không có swap nén trong RAM theo config. |
| `CONFIG_BLK_CGROUP` | tắt | `cgroup_throttle_swaprate()` (`swapfile.c:3831`) không biên dịch. |
| `CONFIG_HIBERNATION`, `CONFIG_PM_WARP` | `=y` | hunk vendor ở 0.2.2 có hiệu lực. |
| `CONFIG_KSM` | tắt | nhánh `ksm_might_need_to_copy()` thực tế trả lại chính page. |
| `CONFIG_ARM64_MTE` | `=y` | `arch_prepare_to_swap()` lưu MTE tag nếu CPU hỗ trợ `[SOURCE arch/arm64/include/asm/pgtable.h:928-958]`; CPU của board có MTE hay không → `[KIỂM TRA TRÊN BOARD]` (`[SUY LUẬN]`: SoC Marvell ARMv8 đời này thường không có MTE). |

### 0.2.4. Những điều tài liệu KHÔNG thể biết `[KIỂM TRA TRÊN BOARD]`

- Board có thật sự bật một swap area không, swap partition hay swap file, nằm trên eMMC/SSD/HDD (`cat /proc/swaps`, `swapon -s`).
- Cgroup v1 hay v2 (ảnh hưởng *thời điểm* charge swap cho memcg — xem `02`, mục 6.5): `mount | grep cgroup`.
- Giá trị runtime `vm.swappiness`, `vm.page-cluster`, `/sys/kernel/mm/swap/vma_ra_enabled`.
- Lượng RAM thật. Các ví dụ dùng **RAM 1 GiB + swap 2 GiB** theo đề bài, chỉ để minh họa.

---

## 0.3. Bản đồ tài liệu ↔ yêu cầu

| File | Nội dung (theo đánh số của đề bài) |
|---|---|
| **`00_index_scope_version.md`** (file này) | Bước 1: phạm vi, phiên bản, diff, config, bảng tag |
| `01_foundations.md` | Phần I: §3 Swap là gì (RAM, VM, page, page table, mô phỏng 1 GiB/2 GiB) · §4 Page/frame, anon/file, PTE/swap entry, swap slot · **Sơ đồ 8** |
| `02_architecture_and_files.md` | Phần II: §5 Kiến trúc tổng thể (**Sơ đồ 1**) · §6 Phân tích 5 file |
| `03_flow_swap_out.md` | §7 Luồng swap out (**Sơ đồ 2, 3**) + trạng thái trước/sau của page A |
| `04_flow_swap_in_and_swap_cache.md` | §8 Swap in + page fault (**Sơ đồ 4**) · §10 Swap cache (**Sơ đồ 6**) |
| `05_slot_lifecycle_swapon_swapoff.md` | §9 Vòng đời swap slot (**Sơ đồ 5**) · §11 swapon/swapoff (**Sơ đồ 7**) |
| `06_design_and_concurrency.md` | Phần IV: §12 Vì sao nhiều lớp · §13 Concurrency & synchronization |
| `07_examples.md` | Phần V: §14–18 Ví dụ A–E |
| `08_quiz_and_summary.md` | Phần VIII–IX: §21 câu hỏi + tình huống dự đoán · §22 tổng kết, bảng tra cứu, hiểu lầm |
| `tools/swap_numbers.py` (+`.out.txt`) | Tính kích thước metadata, mã hoá swap entry/PTE, ngưỡng slot cache, readahead |
| `tools/swap_map_sim.py` (+`.out.txt`) | Mô hình đồ chơi của `swap_map` + per-CPU slot cache: 5 kịch bản |
| `tools/mermaid_lint.py` | Kiểm tra cú pháp Mermaid kiểu heuristic cho tất cả file `.md` |

**Thứ tự đọc khuyến nghị:** `01 → 02 → 03 → 04 → 05 → 06 → 07 → 08`. Nếu chỉ có 30 phút: đọc `01` mục 3.10, `03` mục 7.0, `04` mục 8.0, `05` mục 9.4 và bảng tra cứu trong `08`.

---

## 0.4. Các file source thực sự tham gia (đã đọc)

| File | Vai trò trong swap | Đọc ở |
|---|---|---|
| `mm/swap.c` | **Không phải** trung tâm của swap: quản lý LRU/pagevec, `mark_page_accessed`, `lru_cache_add`, `rotate_reclaimable_page`, `swap_setup()` (đặt `page_cluster`) | `02` §6.1 |
| `mm/swapfile.c` | Swap area (`swap_info_struct`), `swap_map`, cấp phát/giải phóng slot, cluster, `swapon`/`swapoff`, `try_to_unuse` | `02` §6.2, `05` |
| `mm/swap_state.c` | Swap cache (`add_to_swap*`, `delete_from_swap_cache`, `lookup_swap_cache`, `read_swap_cache_async`), readahead | `02` §6.3, `04` |
| `mm/swap_slots.c` | Per-CPU cache swap slot (`get_swap_page`, `free_swap_slot`) | `02` §6.4, `06` |
| `mm/swap_cgroup.c` | Mảng `swap_cgroup` ghi memcg ID cho từng slot | `02` §6.5 |
| `mm/page_io.c` | Ghi/đọc thật xuống thiết bị (`swap_writepage`, `swap_readpage`, bio completion) | `03`, `04` |
| `mm/vmscan.c` | Quyết định reclaim, gọi `add_to_swap`, `try_to_unmap`, `pageout`, `__remove_mapping` | `03` |
| `mm/rmap.c` | `try_to_unmap_one()`: thay PTE bằng swap PTE | `03` |
| `mm/memory.c` | `handle_pte_fault()` → `do_swap_page()`; fork (`copy_nonpresent_pte`); munmap/exit (`zap_pte_range`) | `04`, `05` |
| `mm/memcontrol.c` | charge/uncharge swap cho memcg (gọi vào `swap_cgroup.c`) | `02` §6.5 |
| `include/linux/swap.h`, `swapops.h`, `swap_slots.h`, `swap_cgroup.h`, `arch/arm64/include/asm/pgtable.h` | Struct, hằng số, mã hoá swap entry | khắp nơi |

> Đề bài có nhắc `get_swap_cache_page()`: **không có function nào tên như vậy** trong 5 file và trong MM core của cây 5.10.241 (đã grep). Chỉ có
> `zswap_get_swap_cache_page()` — một hàm `static` riêng của `mm/zswap.c:845`, mà zswap **không bật** trong config KM. Các function đọc swap thật là
> `lookup_swap_cache()`, `__read_swap_cache_async()`, `read_swap_cache_async()`, `swapin_readahead()` `[SOURCE mm/swap_state.c:369, :451, :557, :891]`.
