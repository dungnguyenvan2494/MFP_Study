# Chương 6: Cache coherency và đồng bộ bộ nhớ

> Khoá học USB DMA trên Linux Kernel. Baseline: **Linux 5.10.258**, arm64 (`arch/arm64/mm/cache.S`, `arch/arm64/include/asm/barrier.h`, `io.h`, `pgtable.h`), board Quartz **non-coherent** (DT không có `dma-coherent`).

## 6.0 Vì sao chương này quan trọng với Quartz

- Trên Quartz, controller xHCI **không nhìn thấy cache của CPU**. Mọi dữ liệu dùng chung với controller chỉ đúng khi phần mềm dọn cache đúng lúc, hoặc dùng bộ nhớ không cache.
- Lỗi loại này **hầu như không có log**. Triệu chứng chỉ là **dữ liệu sai**: file copy bị hỏng, gói FAX lỗi CRC, lệnh sai. Đôi khi chúng kéo theo các log gián tiếp của xHCI.
- Hiểu chương này giúp bạn nhận ra khi nào một lỗi "khó hiểu" thực chất là lỗi đồng bộ cache, và khi nào **không thể** là lỗi cache.

---

## 6.1 Khái niệm

| Thuật ngữ | Tiếng Việt | Nghĩa |
|---|---|---|
| **CPU cache** | bộ đệm của CPU | Bản sao nhanh của RAM, gần CPU (L1/L2/L3). Đơn vị là **cache line**: **64 byte** trên arm64 (`L1_CACHE_BYTES`) |
| **Write-back** | ghi trễ | CPU ghi vào cache trước, đánh dấu dòng **bẩn** (dirty). RAM chỉ được cập nhật khi dòng bị **đẩy ra** (evict) hoặc được **clean** |
| **Speculative prefetch** | nạp trước suy đoán | CPU có thể tự nạp một dòng vào cache bất kỳ lúc nào nếu vùng đó được ánh xạ có cache, kể cả khi code chưa đọc |
| **Point of Coherency (PoC)** | điểm nhất quán | Mức bộ nhớ mà mọi bên (CPU và thiết bị) đều thấy cùng dữ liệu. Với thiết bị non-coherent, đó là RAM |
| **Cache coherency** | nhất quán cache | Mọi bên quan sát cùng một giá trị cho cùng một địa chỉ. Giữa các nhân CPU, phần cứng tự đảm bảo. **Giữa CPU và xHCI trên Quartz thì không** |
| **Clean** (`dc cvac`) | làm sạch | Ghi dòng bẩn xuống PoC (RAM), **giữ** dòng trong cache |
| **Invalidate** (`dc ivac`) | vô hiệu | **Bỏ** dòng khỏi cache. Dữ liệu bẩn chưa ghi xuống sẽ **mất** |
| **Clean + invalidate** (`dc civac`) | làm sạch rồi bỏ | Ghi dòng bẩn xuống rồi bỏ khỏi cache |
| **Coherent DMA** (theo phần cứng) | DMA nhất quán | Interconnect **snoop** cache CPU (cổng ACE-Lite, CCI). DT khai báo bằng `dma-coherent`. Không cần dọn cache |
| **Non-coherent DMA** | DMA không nhất quán | Thiết bị đi thẳng tới RAM, bỏ qua cache. Phần mềm phải dọn cache. **Quartz thuộc loại này** |
| **DMA synchronization** | đồng bộ DMA | Map/unmap và sync for cpu/device: các điểm phần mềm dọn cache khi chuyển quyền sở hữu |
| **Memory barrier** | rào bộ nhớ | Lệnh buộc các lần đọc/ghi xảy ra **đúng thứ tự** mà các bên khác quan sát được |

```
                 ┌──────── CPU core ────────┐
                 │  L1 (64 B/dòng)          │   ← dữ liệu mới nhất có thể CHỈ nằm ở đây (dòng bẩn)
                 │  L2 / L3                 │
                 └───────────┬──────────────┘
                             │  clean (cvac) đẩy xuống   │ invalidate (ivac) bỏ đi
                             ▼                           │
 ┌──────────── Interconnect ───────────────┐             │
 │ Coherent: snoop cache CPU  (KHÔNG có trên Quartz)     │
 └───────────┬─────────────────────────────┘             │
             ▼                                           │
          RAM (PoC)  ◄────────── xHCI đọc/ghi trực tiếp, KHÔNG thấy dòng bẩn trong cache
```

---

## 6.2 Linux giữ nhất quán trên Quartz bằng hai chiến lược

### Chiến lược 1: bộ nhớ coherent = ánh xạ không cache

- `dma_alloc_coherent` trên thiết bị non-coherent ánh xạ vùng nhớ với thuộc tính **Normal Non-Cacheable** (`pgprot_dmacoherent` → `MT_NORMAL_NC`).
- Mọi lần đọc/ghi của CPU đi thẳng tới RAM, nên **không bao giờ có dòng cache cũ**.
- Dùng cho: DCBAA, context, mọi ring, ERST, scratchpad, và buffer của `usb_alloc_coherent` (ví dụ `usbfax`).

**Lưu ý quan trọng:** Normal Non-Cacheable **vẫn cho phép gộp và đổi thứ tự** các lần ghi (write-combining). Không cache **không** đồng nghĩa với có thứ tự. Vì vậy vẫn cần barrier (mục 6.5).

### Chiến lược 2: buffer streaming = dọn cache tại điểm chuyển quyền

| Thao tác | Lệnh cache trên arm64 (5.10.258) | Mục đích |
|---|---|---|
| map (mọi direction) | **clean** cả vùng | Đẩy dữ liệu CPU xuống RAM, và bảo đảm không còn dòng bẩn nào bị đẩy xuống đè lên dữ liệu DMA về sau |
| unmap (FROM_DEVICE, BIDIRECTIONAL) | **invalidate** cả vùng; **dòng lẻ ở hai đầu** (không căn 64 byte) thì **clean + invalidate** | Bỏ mọi dòng cũ hoặc dòng nạp trước suy đoán, để CPU đọc lại dữ liệu thiết bị đã ghi |
| unmap (TO_DEVICE) | không làm gì | |
| sync for device | clean | như map, khi dùng lại mapping |
| sync for cpu (≠ TO_DEVICE) | invalidate | như unmap, khi dùng lại mapping |

Chi tiết dòng lẻ ở hai đầu (`__dma_inv_area`, `cache.S:150-163`): nếu đầu hoặc cuối buffer không căn theo dòng cache, kernel dùng **clean + invalidate** cho dòng đó thay vì chỉ invalidate. Mục đích là không làm mất dữ liệu của **biến khác** nằm chung dòng. Cái giá: nếu dòng đó đang bẩn, nó sẽ **ghi đè** phần dữ liệu thiết bị vừa ghi ở mép buffer (xem 6.5, kịch bản 3c).

---

## 6.3 Kịch bản 1: CPU ghi buffer, xHCI đọc (Bulk OUT)

```
 ĐÚNG                                          SAI (quên clean / CPU ghi sau khi map)
 t0  CPU ghi "HELLO" ──► L1 (dòng bẩn)          t0  CPU ghi "HELLO" ──► L1 (bẩn)
     RAM = "?????"                                  RAM = "?????"
 t1  map: CLEAN ──► RAM = "HELLO"               t1  (không clean)
 t2  TRB + doorbell                             t2  TRB + doorbell
 t3  xHCI đọc RAM = "HELLO" ✔                   t3  xHCI đọc RAM = "?????" ✘  thiết bị nhận dữ liệu cũ
```

**Biến thể sai thường gặp:** CPU ghi tiếp vào buffer **sau khi map**, ví dụ sửa header sau khi đã submit. Dữ liệu sửa nằm trong cache, thiết bị không thấy, và nó chỉ xuống RAM vào một lúc không đoán trước.

**Với buffer coherent** (như `usbfax`): CPU ghi đi thẳng tới RAM nhưng có thể đang nằm trong **bộ đệm ghi**. Thứ tự "ghi dữ liệu xong rồi mới bấm doorbell" được bảo đảm vì:
- `writel` trên arm64 thực hiện `dma_wmb()` (`dmb oshst`) **trước** khi ghi MMIO (`io.h:142`);
- xHCI driver còn gọi `wmb()` (`dsb st`) trước khi lật cycle bit.

---

## 6.4 Kịch bản 2: xHCI ghi RAM, CPU đọc (Bulk IN)

```
 Thời gian ──────────────────────────────────────────────────────────────────────────►
 CPU:   [ghi linh tinh vào buffer]  map:CLEAN │      (không chạm buffer)       │ unmap:INVALIDATE │ đọc ✔
 Cache: dòng bẩn ──► RAM; dòng sạch còn lại   │  CPU có thể NẠP TRƯỚC dòng cũ   │  bỏ mọi dòng     │ nạp mới từ RAM
 xHCI:                                         │  ghi dữ liệu mới vào RAM        │                  │
```

| Thiếu bước | Điều gì xảy ra |
|---|---|
| Thiếu **clean khi map** | Một dòng bẩn cũ có thể bị cache đẩy xuống RAM **giữa chừng**, ghi đè dữ liệu xHCI vừa ghi → hỏng ngẫu nhiên theo khối 64 byte |
| Thiếu **invalidate khi unmap** | CPU đọc dòng cũ còn trong cache (có thể do nạp trước suy đoán *trong lúc* DMA chạy) → thấy dữ liệu cũ |
| CPU **đọc buffer trước khi unmap** (ví dụ "nhìn trộm" trong lúc chờ) | Dòng cũ bị nạp vào cache. Lần này unmap vẫn cứu được, nhưng nếu driver dùng sync for cpu sai thứ tự thì không |

Đây là lý do invalidate phải làm **lúc unmap**: chỉ khi đó mới chắc chắn không còn ai nạp dòng cũ vào nữa.

---

## 6.5 Kịch bản 3: CPU và xHCI cùng truy cập một vùng nhớ

### 3a. Cấu trúc dùng chung liên tục (ring, Event Ring, context)

Trên Quartz chúng là **Normal Non-Cacheable**, nên không có chuyện cache cũ. Vấn đề còn lại là **thứ tự**:

```
 Ghi TRB (phần mềm là producer)                  Đọc Event (phần mềm là consumer)
 1. ghi buffer address, length, flags            1. đọc cycle bit của Event
 2. wmb()      ← dsb st                          2. so với CCS
 3. lật cycle bit của TRB đầu                    3. rmb()     ← chặn đọc trước
 4. writel(doorbell) ← tự có dma_wmb trước       4. đọc TRB pointer, length, completion code
```

| Thiếu barrier | Hậu quả |
|---|---|
| Thiếu bước 2 phía ghi | Controller có thể thấy cycle bit **trước** các trường khác (do gộp hoặc đổi thứ tự ghi) → thực thi TRB **nửa cũ nửa mới**: địa chỉ buffer sai → đọc/ghi nhầm RAM, *TRB Error*, hoặc **Host System Error** |
| Thiếu bước 3 phía đọc | CPU có thể đọc trường Event **trước** khi thấy cycle bit mới → xử lý nội dung cũ → khớp TD sai, **`TRB DMA ptr not part of current TD`** |

> **Áp vào OP_BTS-71095:** Event Ring trên Quartz là bộ nhớ không cache, và driver 5.10.258 có `rmb()` đúng chỗ (`xhci-ring.c`, ngay sau khi kiểm tra cycle bit). Vì vậy **"CPU đọc Event cũ do cache" bị loại**. Các nguyên nhân còn lại nằm ở phía trạng thái ring hoặc phần cứng (Chương 5, mục 5.14).

### 3b. Buffer streaming bị CPU chạm khi thiết bị đang sở hữu

Không hợp lệ. Nếu thực sự cần (ví dụ đọc header của một buffer nhận vòng tròn):

```
 sync for cpu (invalidate) ─► CPU đọc ─► sync for device (clean) ─► thiết bị tiếp tục
```

Bỏ cặp sync này thì quay lại các lỗi ở 6.3 và 6.4.

### 3c. Buffer DMA chung dòng cache với dữ liệu khác (false sharing)

```
 struct my_dev {
     int   state;         ┐ cùng một dòng 64 byte
     u8    rx_buf[60];    ┘ ← URB IN ghi vào đây
 };

 t0 map rx_buf (FROM_DEVICE): clean dòng → RAM đúng
 t1 xHCI ghi rx_buf vào RAM
 t2 CPU ghi state = 2  ──► dòng trong cache bẩn: [state=2 | rx_buf CŨ]
 t3 unmap: dòng lẻ ở đầu buffer → CLEAN + INVALIDATE
    ──► ghi cả dòng [state=2 | rx_buf CŨ] xuống RAM ──► ĐÈ MẤT dữ liệu xHCI vừa ghi ở phần đầu rx_buf
 (Kernel cũ invalidate khi map thì ngược lại: mất cập nhật của state)
```

| Ai được bảo vệ | Vì sao |
|---|---|
| `kmalloc` riêng cho buffer | Trên arm64, `kmalloc` căn tối thiểu **128 byte** (`ARCH_DMA_MINALIGN`), không chia dòng với đối tượng khác |
| Buffer là **thành viên của struct** | **Không** được bảo vệ. USB core từ chối buffer trên stack, nhưng **không** kiểm tra buffer nằm trong struct |
| Cách sửa | `kmalloc` buffer riêng, hoặc đánh dấu `____cacheline_aligned` cả trước lẫn sau buffer |

---

## 6.6 Bảng tổng hợp: cái gì hỏng khi đồng bộ sai

| Triệu chứng | Chiều | Dấu hiệu nhận ra | Nguyên nhân hay gặp |
|---|---|---|---|
| Thiết bị nhận dữ liệu cũ hoặc ngẫu nhiên | OUT | sai theo khối 64 byte, phần còn lại đúng | CPU ghi sau khi map; tự dùng `virt_to_phys` không qua DMA API |
| CPU đọc dữ liệu cũ | IN | lặp lại đúng nội dung lần trước | đọc trước unmap; thiếu sync for cpu |
| Mép buffer bị hỏng (vài byte đầu/cuối) | IN | chỉ ở dòng cache đầu hoặc cuối | buffer chung dòng cache với biến khác (3c) |
| Biến bên cạnh buffer "tự đổi giá trị" | IN | biến cùng struct với buffer | false sharing (3c) |
| Controller chạy TRB rác, TRB Error, HSE | ring | lỗi ngẫu nhiên lúc tải cao | thiếu `wmb()` trước khi trao TRB |
| Khớp Event sai, Event lạ | Event Ring | `not part of current TD`, `unknown event type` | thiếu `rmb()`, hoặc (trên SoC có cache cho ring) không invalidate |
| Chỉ lỗi trên một nền tảng | mọi chiều | chạy tốt trên x86/SoC coherent, lỗi trên Quartz | code dựa vào phần cứng coherent, bỏ qua DMA API |
| Lỗi biến mất khi thêm printk | mọi chiều | thời điểm thay đổi làm cache bị đẩy xuống khác đi | thiếu đồng bộ, thiếu barrier |

**Một bẫy riêng của DT:** khai báo `dma-coherent` cho thiết bị mà interconnect **không** snoop thì Linux bỏ qua mọi thao tác cache → hỏng dữ liệu hàng loạt. Ngược lại, khai báo thiếu trên phần cứng coherent thì chỉ làm chậm. Trên Quartz, DT không khai báo `dma-coherent`, đúng với giả định non-coherent.

---

## 6.7 Log liên quan

Lỗi cache **hiếm khi tự sinh log**. Có hai nhóm dấu hiệu:

### Nhóm 1: log gián tiếp của xHCI (câu chữ đã kiểm tra trong `xhci-ring.c`)

| Log | Có thể liên quan đồng bộ khi |
|---|---|
| `WARN: TRB error for slot %u ep %u on endpoint` | controller đọc TRB có trường không hợp lệ (TRB trao trước khi ghi xong) |
| `WARNING: Host System Error` | TRB hoặc context mang địa chỉ rác |
| `ERROR unknown event type %d` | đọc Event chưa hoàn chỉnh (thiếu rmb) hoặc Event Ring hỏng |
| `ERROR Transfer event TRB DMA ptr not part of current TD …` | như trên, hoặc trạng thái ring lệch |
| `WARN: HC couldn't access mem fast enough for slot %u ep %u` | **không** phải lỗi cache: đây là Data Buffer Error, tức độ trễ truy cập RAM (tương tự EHCI ở Chương 4) |

### Nhóm 2: DMA API debug (cần `CONFIG_DMA_API_DEBUG`, đang tắt trên Quartz)

Câu chữ đã kiểm tra trong `kernel/dma/debug.c`:

| Cảnh báo | Bắt được |
|---|---|
| `device driver syncs DMA memory with different direction …` | sync với direction khác lúc map |
| `device driver syncs device read-only DMA memory for cpu …` | sync for cpu trên mapping TO_DEVICE (vô nghĩa, dấu hiệu nhầm chiều) |
| `device driver syncs device write-only DMA memory to device …` | sync for device trên mapping FROM_DEVICE |
| `device driver syncs DMA memory outside allocated range …` | sync vượt vùng đã map |
| `exceeded %d overlapping mappings of cacheline %pa` | nhiều mapping cùng một dòng cache: dấu hiệu **false sharing** (3c) |

### Log-to-flow: dữ liệu IN thỉnh thoảng sai ở 32 byte đầu

```
 [Triệu chứng] file đọc từ USB đôi khi sai ở đầu khối, không có log
      │ sai theo bội 64 byte? chỉ ở đầu/cuối buffer? chỉ trên Quartz?
      ▼
 [Giả thuyết] buffer chung dòng cache (3c) hoặc CPU chạm buffer khi đang DMA (3b)
      │
      ▼
 [Kiểm tra] địa chỉ buffer có căn 64/128 byte không? buffer có nằm trong struct không?
            bật DMA API debug: có "overlapping mappings of cacheline" không?
      │
      ▼
 [Thử nghiệm] kmalloc buffer riêng → lỗi biến mất? → xác nhận false sharing
```

---

## 6.8 Kỹ thuật điều tra lỗi đồng bộ

| Kỹ thuật | Làm gì | Kỳ vọng |
|---|---|---|
| Mẫu dữ liệu | Điền buffer bằng mẫu đã biết (`0xA5`, bộ đếm tăng dần) rồi so kết quả | Vị trí sai căn theo **64 byte** → nghi cache |
| Kiểm tra căn lề | In địa chỉ buffer (`%p` với `no_hash_pointers` hoặc `%px` khi debug) | Không căn 64/128 hoặc nằm trong struct → nghi 3c |
| `swiotlb=force` (tham số boot) | Ép mọi mapping đi qua bounce | Lỗi đổi hành vi → liên quan buffer gốc (căn lề, chạm buffer). Đây là thử nghiệm, không phải cách sửa |
| DMA API debug | Bật `CONFIG_DMA_API_DEBUG` | Bắt sai direction, sai sync, trùng dòng cache |
| So nền tảng | Chạy cùng driver trên SoC coherent | Chỉ lỗi trên Quartz → nghi đồng bộ cache |
| Đọc code driver | Tìm chỗ CPU ghi buffer sau khi submit, đọc buffer trước callback | |

---

## 6.9 Tóm tắt Chương 6

1. Quartz **non-coherent**: xHCI không thấy cache CPU. Dòng cache 64 byte, `kmalloc` căn 128 byte.
2. **Bộ nhớ coherent** trên Quartz = Normal Non-Cacheable: không có cache cũ nhưng **vẫn cần barrier**, vì các lần ghi có thể bị gộp hoặc đổi thứ tự.
3. **Buffer streaming**: map thì **clean**, unmap (FROM_DEVICE) thì **invalidate**. Dòng lẻ ở mép thì clean + invalidate.
4. OUT sai → thiết bị nhận dữ liệu cũ. IN sai → CPU đọc dữ liệu cũ, hoặc dòng bẩn đè dữ liệu DMA.
5. Ring: **`wmb()` trước khi trao TRB**, **`rmb()` sau khi thấy cycle bit**. `writel` tự có `dma_wmb()` trước khi ghi MMIO.
6. **False sharing** (buffer chung dòng cache với biến khác) làm hỏng mép buffer hoặc biến bên cạnh. USB core **không** phát hiện được buffer nằm trong struct.
7. Lỗi đồng bộ cache **gần như không có log**. Dấu hiệu là sai theo 64 byte và chỉ lỗi trên nền tảng non-coherent.
8. Với OP_BTS-71095: Event Ring không cache và có `rmb()` đúng chỗ, nên **cache cũ không phải nguyên nhân**.

## Câu hỏi kiểm tra

1. Vì sao "không cache" (Normal Non-Cacheable) vẫn chưa đủ để CPU và xHCI phối hợp đúng trên ring?
2. Trong Bulk IN, vì sao clean lúc map không thay được invalidate lúc unmap?
3. Vẽ diễn biến khi một dòng bẩn bị cache tự đẩy xuống RAM **giữa** một lần DMA IN, nếu kernel không clean lúc map.
4. Một driver USB khai báo `struct { u16 status; u8 buf[62]; }` rồi dùng `buf` cho URB IN. Lỗi gì có thể xảy ra trên Quartz, và vì sao USB core không cảnh báo?
5. Vì sao `writel(doorbell)` trên arm64 tự bảo đảm các lần ghi bộ nhớ trước đó (TRB, buffer coherent) đã được thiết bị thấy?
6. Nếu thiếu `rmb()` sau khi kiểm tra cycle bit của Event, log nào có thể xuất hiện, và vì sao trên Quartz lại khó xảy ra với Event Ring?
7. DT khai báo nhầm `dma-coherent` cho xHCI trên một SoC không snoop. Mô tả triệu chứng cho cả ring lẫn buffer dữ liệu.
8. Dữ liệu sai đúng ở các vị trí 0x40, 0x80, 0xC0 của buffer, mỗi chỗ dài 64 byte. Giả thuyết đầu tiên của bạn là gì, và kiểm chứng ra sao?

---

**Chương tiếp theo:** Chương 7 đi sâu IOMMU, DMA mask và bounce buffer: giới hạn địa chỉ, 32/64-bit, mask không khớp, IOVA, SWIOTLB, các tình huống thực tế khi chúng được kích hoạt và ảnh hưởng tới USB DMA.
