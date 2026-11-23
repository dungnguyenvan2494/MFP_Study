# Chương 7: IOMMU, DMA mask và bounce buffer

> Khoá học USB DMA trên Linux Kernel. Baseline: **Linux 5.10.258** (`kernel/dma/`, `drivers/iommu/`, `drivers/usb/storage/scsiglue.c`). Board Quartz: xHCI mask 32-bit, **không có SMMU** (`CONFIG_ARM_SMMU` không bật), SWIOTLB 64 MiB, RAM vật lý tới 0x2_7FFF_FFFF. Các log SMMU trong chương được đối chiếu với source, nhưng **không áp dụng cho Quartz**; chúng có để bạn nhận ra khi làm trên SoC khác.

## 7.0 Bài toán: chuỗi độ rộng địa chỉ

```
 CPU nhìn thấy RAM tới 0x2_7FFF_FFFF (34 bit)
        │
 Thiết bị phát được N bit địa chỉ  ──► DMA mask (dma_mask, coherent_dma_mask)
        │
 Interconnect / bus giữa thiết bị và RAM có thể hẹp hơn ──► bus_dma_limit (từ dma-ranges trong DT)
        │
 Bộ dịch địa chỉ trên đường đi (nếu có): offset của bus, IOMMU
        │
 RAM
```

Linux phải trả lời một câu cho **mỗi lần map**: *thiết bị có chạm tới được địa chỉ này không?* Phép so sánh dùng `min(dma_mask, bus_dma_limit)` với **địa chỉ cuối** của vùng cần map. Không chạm được thì có ba lối ra: **IOMMU** (dịch), **SWIOTLB** (chép qua vùng thấp), hoặc **lỗi**.

---

## 7.1 Giới hạn địa chỉ DMA đến từ đâu

| Nguồn giới hạn | Ví dụ | Linux biết qua |
|---|---|---|
| Thanh ghi địa chỉ của thiết bị | controller chỉ có con trỏ 32-bit | driver đặt mask (xHCI đọc bit AC64 trong HCCPARAMS1) |
| Lỗi phần cứng (erratum) | controller báo hỗ trợ 64-bit nhưng thực tế 32-bit (R-Car; **Quartz**: quirk KM `XHCI_NO_64BIT_SUPPORT`) | quirk trong driver |
| Interconnect hẹp | cổng AXI của thiết bị chỉ nối 32 dây địa chỉ | `dma-ranges` trong DT → `bus_dma_limit` |
| Cửa sổ inbound của PCIe | root complex chỉ ánh xạ 4 GiB đầu | `dma-ranges` của host bridge |
| Offset của bus | thiết bị thấy RAM ở địa chỉ khác (ví dụ RAM 0x0 nhìn từ thiết bị là 0xC000_0000) | `dma-ranges` (offset) → `phys_to_dma` cộng/trừ |

Trên Quartz: chỉ có hai nguồn đầu (mask 32-bit do quirk). DT không có `dma-ranges` nên không có `bus_dma_limit` và không có offset.

---

## 7.2 DMA 32-bit và 64-bit

| | DMA 32-bit | DMA 64-bit |
|---|---|---|
| Chạm tới | 4 GiB đầu | toàn bộ RAM |
| RAM cao (trên 4 GiB) | phải bounce, hoặc dùng IOMMU | đi thẳng |
| Bộ nhớ coherent | cấp từ `ZONE_DMA`/`ZONE_DMA32` | bất kỳ đâu |
| Ví dụ | xHCI trên Quartz (do quirk), **mọi EHCI trên Linux 5.10** (64-bit bị tắt trong code) | xHCI trên đa số PC |

**Kernel chấp nhận mask nào?** `dma_direct_supported` luôn chấp nhận mask **≥ 32 bit**. Kernel kỳ vọng kiến trúc có `ZONE_DMA32`/`ZONE_DMA` hoặc SWIOTLB để phục vụ (`kernel/dma/direct.c:485-496`). Mask hẹp hơn 32 bit chỉ được chấp nhận nếu có đủ RAM thấp. Không được thì `dma_set_mask` trả `-EIO`.

---

## 7.3 Mask không khớp với thực tế phần cứng

Đây là loại lỗi nguy hiểm nhất của chương: **không có log nào báo**.

### Trường hợp A: mask khai báo RỘNG hơn khả năng thật (ví dụ không có quirk KM)

```
 Driver tin AC64 = 1 ─► mask 64-bit ─► map buffer tại 0x2_1234_5000: "vừa mask" → không bounce
 TRB chứa 0x0000_0002_1234_5000
 Controller thực tế chỉ dùng 32 bit thấp ─► truy cập 0x1234_5000  ← RAM CỦA AI KHÁC
   OUT: gửi dữ liệu rác cho thiết bị
   IN : GHI ĐÈ âm thầm lên vùng nhớ 0x1234_5000 (page cache, struct kernel, stack…)
```

Triệu chứng: hỏng bộ nhớ ngẫu nhiên khắp hệ thống, oops ở những chỗ không liên quan USB, chỉ xảy ra khi buffer rơi vào RAM cao (nghĩa là phụ thuộc mức sử dụng bộ nhớ). **Đây chính là lý do quirk `XHCI_NO_64BIT_SUPPORT` của KM tồn tại.**

### Trường hợp B: mask HẸP hơn khả năng thật

An toàn nhưng chậm: bounce không cần thiết, mỗi lần truyền tốn hai memcpy, kho SWIOTLB dễ đầy.

### Trường hợp C: bus hẹp hơn thiết bị nhưng DT thiếu `dma-ranges`

Giống trường hợp A. Thiết bị phát đủ 64 bit nhưng interconnect cắt bớt.

### Trường hợp D: coherent mask khác streaming mask

Ring (coherent) đặt đúng chỗ nhưng buffer dữ liệu (streaming) thì không, hoặc ngược lại. xHCI luôn đặt hai mask bằng nhau (`xhci.c:5368-5380`).

---

## 7.4 IOMMU và IOVA

### Kiến trúc

```
 xHCI phát IOVA ──► IOMMU (SMMU trên ARM)
                    ├─ Stream ID → chọn "domain" (bảng trang riêng của thiết bị)
                    ├─ IOTLB (cache dịch địa chỉ)
                    └─ bảng trang: IOVA → địa chỉ vật lý + quyền đọc/ghi
                         │ có mục hợp lệ        │ không có / sai quyền
                         ▼                      ▼
                        RAM                   FAULT → giao dịch bị chặn → thiết bị nhận lỗi bus
```

### Map và unmap khi có IOMMU

| Bước | Map | Unmap |
|---|---|---|
| 1 | cấp một dải IOVA **trong vùng thiết bị với tới** (32-bit cho thiết bị 32-bit) | xóa mục trong bảng trang |
| 2 | ghi mục bảng trang IOVA → vật lý | xóa IOTLB (ngay lập tức nếu `strict`, hoặc gộp nhiều lần nếu lazy) |
| 3 | dọn cache (nếu thiết bị non-coherent) | dọn cache |
| 4 | trả IOVA làm DMA address | trả dải IOVA |

### Lợi ích cho USB

| Lợi ích | Ví dụ |
|---|---|
| Thiết bị 32-bit với tới RAM cao **mà không bounce** | xHCI 32-bit trên SoC 16 GiB vẫn không tốn memcpy |
| Ghép scatter-gather | 16 trang rời thành một dải IOVA liên tục, nên ít TRB hơn |
| **Cách ly** | DMA sai địa chỉ thì **bị chặn và có log fault**, thay vì âm thầm ghi đè RAM như trên Quartz |

### Các chế độ

- **Translated** (mặc định): dịch đầy đủ.
- **Passthrough / identity** (`iommu.passthrough=1`): IOVA = vật lý, tức là không còn bảo vệ, chỉ còn cấu trúc.
- **Strict / lazy** (`iommu.strict`): xóa IOTLB ngay hay trì hoãn. Lazy nhanh hơn, nhưng **sau khi unmap thiết bị vẫn có thể truy cập trong một khoảng ngắn**.

### Trên Quartz

Log boot `iommu: Default domain type: Translated` chỉ là **chính sách mặc định**. Không có driver SMMU, DT không có `iommus`, nên xHCI đi thẳng. Hệ quả: mọi lỗi DMA address trên Quartz là **im lặng**.

---

## 7.5 SWIOTLB và bounce buffer

| Thông số (5.10.258) | Giá trị | Nguồn |
|---|---|---|
| Kích thước mặc định | **64 MiB** (`IO_TLB_DEFAULT_SIZE`) | `swiotlb.c:153` |
| Kích thước một ô | **2 KiB** (`IO_TLB_SHIFT = 11`) | `swiotlb.h:31` |
| Số ô liên tiếp tối đa cho một lần map | **128 ô = 256 KiB** (`IO_TLB_SEGSIZE`) | `swiotlb.h:25` |
| Tham số boot | `swiotlb=<số ô>[,force]` hoặc `swiotlb=noforce` | `swiotlb.c:110-128` |
| Vị trí trên Quartz | 0x877B5000 – 0x8B7B5000 | log boot |

### Hệ quả ít ai để ý: giới hạn kích thước mỗi lệnh của usb-storage

```
 dma_addressing_limited(xHCI) = min(mask 32-bit, bus limit) < mask cần để phủ RAM (34-bit) = ĐÚNG
 is_swiotlb_active() = ĐÚNG
 ──► dma_max_mapping_size(xHCI) = swiotlb_max_mapping_size = 256 KiB     (kernel/dma/direct.c:508-515)
 ──► usb-storage: max_hw_sectors = min(…, 256 KiB / 512) = 512 sector     (drivers/usb/storage/scsiglue.c:125-131)
```

Trên Quartz, **mỗi lệnh đọc/ghi của ổ USB bị giới hạn 256 KiB**, kể cả khi buffer thực tế nằm dưới 4 GiB, chỉ vì *có thể* phải bounce. Đây là ví dụ rõ nhất cho thấy mask ảnh hưởng tới hiệu năng ở tầng rất cao.

### Chi phí và rủi ro

| Vấn đề | Mô tả |
|---|---|
| Hiệu năng | Hai lần memcpy cho mỗi byte (map và unmap). Băng thông bị giới hạn bởi tốc độ memcpy của CPU |
| Cạn kho | Kho dùng chung cho **mọi thiết bị** bị giới hạn địa chỉ. Rò mapping (URB không trả về) làm cạn dần |
| Phân mảnh | Còn nhiều ô trống nhưng không đủ 128 ô liên tiếp cho một lần map lớn |
| Ô bounce nằm ngoài mask | Thiết bị hẹp hơn 32 bit (ví dụ 31-bit) trong khi kho nằm ở khoảng 2.1 GiB → `swiotlb addr … overflow` |

---

## 7.6 Tình huống thực tế khi các cơ chế này được kích hoạt

| # | Tình huống | Cơ chế kích hoạt | Ảnh hưởng tới USB | Dấu hiệu |
|---|---|---|---|---|
| 1 | Quartz, URB với buffer `kmalloc` rơi vào RAM cao | SWIOTLB | chạy đúng nhưng tốn memcpy | không có log |
| 2 | Quartz, ổ USB đọc file lớn | `dma_max_mapping_size` | mỗi lệnh ≤ 256 KiB | throughput thấp hơn kỳ vọng |
| 3 | Driver map một khối > 256 KiB | SWIOTLB | map thất bại, submit `-EAGAIN` | `swiotlb buffer is full …` |
| 4 | URB treo không được trả về (như OP_BTS-71095) với buffer bị bounce | SWIOTLB | ô không được trả, kho cạn dần | `used` tăng trong `swiotlb buffer is full` |
| 5 | Thiết bị 31-bit, kho SWIOTLB ở trên 2 GiB | SWIOTLB | map thất bại | `swiotlb addr … overflow …` |
| 6 | Boot với `swiotlb=noforce` | không có bounce | map thất bại kèm WARNING | `DMA addr … overflow …` |
| 7 | Mask khai báo rộng hơn khả năng thật | không gì cả (vì "vừa mask") | **ghi đè RAM âm thầm** | oops ngẫu nhiên ở chỗ khác |
| 8 | SoC có SMMU, driver dùng `virt_to_phys` | SMMU | giao dịch bị chặn → xHCI báo HSE | SMMU fault + `WARNING: Host System Error` |
| 9 | SoC có SMMU, URB bị dùng sau khi unmap | SMMU (strict) | bị chặn | SMMU fault |
| 10 | SoC có SMMU chế độ lazy, truy cập ngay sau unmap | IOTLB cũ | **có thể lọt qua** trong một khoảng ngắn | không có log |

---

## 7.7 So sánh đường đi của một lần map trên ba nền tảng

```
                      Quartz (của bạn)          SoC có SMMU              PC x86, xHCI 64-bit
 Mask                 32-bit (quirk)            32 hoặc 64-bit           64-bit
 Buffer ở RAM cao     SWIOTLB bounce            IOMMU cấp IOVA thấp      đi thẳng
 DMA address          phys hoặc ô bounce        IOVA                     phys
 Cache                clean/invalidate          clean/invalidate (nếu    snoop, không làm gì
                                                non-coherent)
 Địa chỉ sai          ghi đè âm thầm            FAULT + log              ghi đè âm thầm (nếu
                                                                         IOMMU tắt)
 Giới hạn mỗi lần map 256 KiB                    không                    không
```

---

## 7.8 Log của chương

| Log | Nguồn (đã kiểm tra) | Áp dụng Quartz | Nghĩa |
|---|---|---|---|
| `software IO TLB: mapped [mem …] (64MB)` | `swiotlb.c:172` | **có** (log boot) | kho bounce đã sẵn sàng |
| `swiotlb buffer is full (sz: %zd bytes), total %lu (slots), used %lu (slots)` | `swiotlb.c:589` | có thể | hết ô liên tiếp |
| `swiotlb addr %pad+%zu overflow (mask %llx, bus limit %llx).` | `swiotlb.c:725` | ít khả năng | chính ô bounce nằm ngoài mask |
| `DMA addr %pad+%zu overflow (mask %llx, bus limit %llx).` | `kernel/dma/direct.h` | chỉ khi `swiotlb=noforce` | vượt mask mà không có bounce |
| `iommu: Default domain type: Translated` | `iommu.c:145` | **có** (log boot) | chỉ là chính sách |
| `Unhandled context fault: fsr=0x%x, iova=0x%08lx, fsynr=0x%x, cbfrsynra=0x%x, cb=%d` | `arm-smmu.c:440` | **không** | SMMUv1/v2: thiết bị truy cập IOVA không hợp lệ |
| `Blocked unknown Stream ID 0x%hx; boot with "arm-smmu.disable_bypass=0" to allow, …` | `arm-smmu.c:466` | **không** | thiết bị chưa được gắn domain mà đã DMA |
| `event 0x%02x received:` | `arm-smmu-v3.c:1378` | **không** | SMMUv3 báo sự kiện (fault) |

### Phân tích: `Unhandled context fault … iova=…` (trên SoC có SMMU)

| Mục | Giải thích |
|---|---|
| Ý nghĩa | Thiết bị (theo stream ID của context bank `cb`) truy cập một IOVA không có mục hợp lệ, hoặc sai quyền |
| Trước đó | Controller DMA theo một địa chỉ lấy từ TRB/context/thanh ghi |
| Sau đó | Giao dịch bị chặn → controller nhận lỗi bus → với xHCI thường là **`WARNING: Host System Error`**, rồi controller halt |
| Nguyên nhân | Dùng sau khi unmap (URB/buffer đã trả nhưng TRB còn trên ring), DMA address lấy bằng `virt_to_phys`, ring/context bị ghi đè, mask sai làm IOVA bị cắt |
| Cần kiểm tra | Giá trị `iova` → tra xem vừa thuộc mapping nào (DMA API debug, trace `iommu:map`/`iommu:unmap`); `fsr` cho biết loại fault |

```
 Log-to-flow:
 [SW] unmap buffer (URB trả về / bị hủy sai cách) ──► IOVA không còn hợp lệ
 [HW] TRB cũ vẫn trỏ IOVA đó, controller DMA ──► SMMU tra bảng trang: không có ──► FAULT
 [Log] "Unhandled context fault: … iova=0x… cb=…"
 [HW] xHCI nhận lỗi bus ──► USBSTS.HSE ──► [Log] "WARNING: Host System Error" ──► halt
 Trên Quartz (không SMMU): cùng chuỗi sự kiện nhưng KHÔNG có fault, controller ghi thẳng vào RAM cũ
```

---

## 7.9 Tóm tắt Chương 7

1. Mỗi lần map, kernel so **địa chỉ cuối** với `min(dma_mask, bus_dma_limit)`. Không chạm tới thì có ba lối: **IOMMU**, **SWIOTLB**, hoặc **lỗi**.
2. Mask khai báo **rộng hơn** khả năng thật là lỗi nguy hiểm nhất: không bounce, địa chỉ bị cắt, ghi đè RAM âm thầm. Quirk 32-bit của KM ngăn đúng lỗi này.
3. **IOMMU** cho thiết bị 32-bit với tới RAM cao mà không cần memcpy, và biến DMA sai thành **fault có log**. Quartz không có IOMMU.
4. **SWIOTLB**: 64 MiB, ô 2 KiB, tối đa 256 KiB mỗi lần map, dùng chung cho mọi thiết bị bị giới hạn địa chỉ.
5. Trên Quartz, xHCI bị coi là giới hạn địa chỉ, nên **usb-storage giới hạn mỗi lệnh 256 KiB**.
6. Log SMMU (`Unhandled context fault`, `event … received`) thường đi trước `WARNING: Host System Error` trên SoC có SMMU. Trên Quartz, chuỗi đó im lặng.

## Câu hỏi kiểm tra

1. Nếu xóa quirk `XHCI_NO_64BIT_SUPPORT` trên Quartz, kernel sẽ chọn mask nào? Mô tả điều gì xảy ra khi buffer IN rơi vào 0x2_0000_1000.
2. Vì sao `dma_direct_supported` luôn chấp nhận mask 32-bit, nhưng có thể từ chối mask 30-bit?
3. Một ổ USB trên Quartz đạt tốc độ thấp hơn trên PC dù cùng controller USB 3.0. Nêu hai nguyên nhân liên quan chương này và cách kiểm chứng.
4. So sánh hậu quả của việc "DMA vào buffer đã unmap" trên Quartz, trên SoC có SMMU chế độ strict, và chế độ lazy.
5. `swiotlb buffer is full` xuất hiện dù `used` chỉ 60% tổng số ô. Giải thích.
6. Vì sao IOMMU có thể giảm số TRB cần cho một URB scatter-gather?
7. Bạn thấy `Unhandled context fault … iova=0xfffe1000` ngay trước `WARNING: Host System Error`. Hãy lần ngược về ba nơi có thể chứa địa chỉ đó.
8. Trên Quartz, giảm kích thước SWIOTLB xuống 16 MiB có ảnh hưởng gì tới URB nhỏ (dưới 4 KiB) và URB lớn (256 KiB)?

---

**Chương tiếp theo:** Chương 8 tổng hợp và phân tích log kernel USB DMA theo sáu nhóm A–F (khởi tạo, mapping, truyền, xHCI, EHCI, IOMMU/bộ nhớ), mỗi log theo mẫu: ý nghĩa, nguồn, điều kiện, trước/sau, cơ chế, nguyên nhân, bình thường hay lỗi, cần kiểm tra gì.
