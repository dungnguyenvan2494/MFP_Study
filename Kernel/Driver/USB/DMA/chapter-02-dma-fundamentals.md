# Chương 2: Nền tảng DMA (DMA Fundamentals)

> Khoá học USB DMA trên Linux Kernel. Baseline: **Linux 5.10.258** (`Kernel/Src`), board Marvell Armada Quartz EMU 800 (arm64). Cơ chế và log trong chương này đã được đối chiếu với source; chỗ nào là ví dụ minh họa thì có ghi rõ.

## 2.0 Trước khi bắt đầu

### Đính chính so với Chương 1

Khi kiểm tra kỹ cây kernel 5.10.258 của board, có hai chi tiết khác với cách mô tả "giáo khoa". File Chương 1 đã được sửa theo:

| Chi tiết | Mô tả phổ biến (kernel cũ) | Thực tế trên 5.10.258 này | Nguồn |
|---|---|---|---|
| Thao tác cache khi map `DMA_FROM_DEVICE` trên arm64 | invalidate | **clean** (mọi direction đều clean khi map; chỉ invalidate khi unmap) | `arch/arm64/mm/cache.S` (`__dma_map_area` luôn nhảy sang `__dma_clean_area`) |
| SWIOTLB khi map buffer `DMA_FROM_DEVICE` | không chép gì | **chép buffer gốc sang ô bounce** (chống lộ dữ liệu cũ trong ô bounce ra user space) | `kernel/dma/swiotlb.c:603-610` |

Cả hai là bản backport các bản vá upstream về an toàn và đúng đắn. Lý do của từng thay đổi được giải thích ở mục 2.6.

### Cấu hình build liên quan tới DMA

Cấu hình khả năng cao là `km_mvebu_v8_lsp_defconfig`. Căn cứ: CMA tắt khớp với log `0K cma-reserved`, và xHCI là module khớp với việc driver được nạp ở giây thứ 62.

| Tùy chọn | Giá trị | Ý nghĩa cho USB DMA |
|---|---|---|
| `CONFIG_ZONE_DMA` / `ZONE_DMA32` | y / y | Có zone riêng cho thiết bị bị giới hạn địa chỉ (log: DMA = 4 GiB đầu, DMA32 rỗng) |
| `CONFIG_SWIOTLB` | y | Có bounce buffer cho thiết bị không với tới RAM cao |
| `CONFIG_DMA_DIRECT_REMAP` | y | Bộ nhớ coherent được ánh xạ lại thành **không cache** trên thiết bị non-coherent |
| `CONFIG_CMA` | **không bật** | Không có vùng CMA. Cấp phát coherent lớn phải dựa vào trang liên tục từ buddy allocator |
| `CONFIG_IOMMU_SUPPORT` / `CONFIG_ARM_SMMU` | y / **không bật** | Có khung IOMMU nhưng không có driver SMMU → không thiết bị nào dùng IOVA |
| `CONFIG_DMA_API_DEBUG` | **không bật** | Không có log kiểm tra lạm dụng DMA API |
| `CONFIG_DYNAMIC_DEBUG` | **không bật** | `dev_dbg`/`xhci_dbg` không in được |
| `CONFIG_VMAP_STACK` | y | Stack của kernel nằm trong vùng vmalloc → **không được dùng làm buffer DMA** |
| `CONFIG_USB_XHCI_HCD`, `USB_XHCI_PLATFORM`, `USB_EHCI_HCD` | m | Driver host USB là module |

---

## 2.1 Bản đồ khái niệm

```
                         DMA (Direct Memory Access – truy cập bộ nhớ trực tiếp)
                                          │
              ┌───────────────────────────┴────────────────────────────┐
     Third-party DMA                                          First-party DMA
     (DMA Controller riêng chép hộ)                           (Bus Mastering: thiết bị tự làm)
     UART, audio… trên Quartz: mv61x0-vdma                    xHCI, EHCI, NVMe, mvri
                                          │
                     Thiết bị cần 2 thứ: ĐỊA CHỈ ĐÚNG + DỮ LIỆU NHẤT QUÁN
                ┌─────────────────────────┴──────────────────────────┐
         KHÔNG GIAN ĐỊA CHỈ                                    KIỂU BỘ NHỚ DMA
   CPU Virtual → Physical → DMA address                Coherent            Streaming
                         └─(IOMMU)→ IOVA              (cấp phát 1 lần,     (DMA Mapping / Unmapping
                │                                     dùng lâu dài)        cho từng lần truyền,
           DMA MASK (thiết bị phát được bao nhiêu bit)                     kèm DMA DIRECTION)
                │ vượt mask
        SWIOTLB bounce buffer ─── hoặc ─── lỗi map
```

Mỗi mục dưới đây trình bày một nhóm khái niệm theo cùng khung: **là gì**, **vì sao tồn tại**, **hoạt động thế nào**, **liên quan USB**, **ví dụ**, **sơ đồ**.

---

## 2.2 DMA, DMA Controller, Bus Mastering, CPU-driven và DMA-driven

### DMA (Direct Memory Access – truy cập bộ nhớ trực tiếp)

| | |
|---|---|
| **Là gì** | Cơ chế để một khối phần cứng không phải CPU tự đọc/ghi RAM |
| **Vì sao** | Giải phóng CPU khỏi việc chép dữ liệu, đạt băng thông cao, giảm độ trễ (không chờ ngắt cho từng khối nhỏ), tiết kiệm điện |
| **Hoạt động** | Khối phần cứng phát giao dịch đọc/ghi lên interconnect (AXI, PCIe) giống như CPU, với địa chỉ do phần mềm đã cấp trước |
| **Liên quan USB** | Toàn bộ việc lấy TRB, lấy dữ liệu, ghi Event, ghi dữ liệu IN đều là DMA của xHCI |
| **Ví dụ** | Đọc 1 MiB từ USB disk: CPU chỉ chuẩn bị vài chục TRB và xử lý vài ngắt. Hơn 1 triệu byte được chuyển mà CPU không chạm vào |

### DMA Controller (bộ điều khiển DMA dùng chung) – "third-party DMA"

```
 ┌──────┐  cấu hình kênh  ┌──────────────┐  đọc RAM / ghi FIFO  ┌──────────┐
 │ CPU  │────────────────►│ DMA          │◄────────────────────►│ UART FIFO│
 └──────┘                 │ Controller   │                      └──────────┘
                          │ (mv61x0-vdma)│◄────────► RAM
                          └──────────────┘
   Thiết bị (UART) KHÔNG tự truy cập RAM; DMA Controller làm hộ.
```

| | |
|---|---|
| **Là gì** | Một khối riêng có nhiều kênh, chuyên chép dữ liệu giữa RAM và FIFO của thiết bị khác |
| **Vì sao** | Để các thiết bị đơn giản (UART, SPI, audio) cũng hưởng lợi DMA mà không cần tự mang DMA engine |
| **Trong Linux** | Dùng **dmaengine API** (xin kênh, chuẩn bị descriptor, submit). Đây là API khác với DMA mapping API |
| **Liên quan USB** | **Host controller xHCI/EHCI không dùng.** Một số IP USB device/OTG đời cũ dùng (ví dụ MUSB kèm CPPI 4.1 trên TI AM335x) |
| **Trên board** | Log `mv61x0-vdma … Virtual DMA Controller …`, `dw-apb-uart … failed to request DMA` là của loại này, không liên quan USB |

### Bus Mastering (làm chủ bus) – "first-party DMA"

```
                 ┌────────── Interconnect (AXI) ──────────┐
   Master ports: │  CPU cluster   xHCI   EHCI   mvri     │  ← đều có thể KHỞI TẠO giao dịch
   Slave ports:  │  DDR controller   thanh ghi MMIO       │  ← chỉ TRẢ LỜI giao dịch
                 └─────────────────────────────────────────┘
```

| | |
|---|---|
| **Là gì** | Thiết bị có cổng master trên bus, tự phát giao dịch đọc/ghi bộ nhớ |
| **Vì sao** | Thiết bị phức tạp (USB host, NIC, NVMe) cần tự duyệt cấu trúc dữ liệu trong RAM (ring, descriptor), không chỉ chép một khối |
| **Hoạt động** | PCI cần bật bit Bus Master Enable (`pci_set_master`). Thiết bị platform như xHCI trên Quartz luôn là master. **Có coherent với cache CPU hay không** phụ thuộc interconnect (cổng ACE-Lite/CCI có snoop cache hay không). DT khai báo điều này bằng `dma-coherent`, và Quartz **không** có |
| **Liên quan USB** | xHCI là master: tự đọc DCBAA → Device Context → Transfer Ring → TRB → buffer |

### CPU-driven transfer và DMA-driven transfer

| Tiêu chí | CPU-driven (PIO) | DMA-driven |
|---|---|---|
| Ai chép từng byte | CPU | Thiết bị |
| Tải CPU | tỉ lệ với lượng dữ liệu | gần như cố định cho mỗi lần truyền |
| Ảnh hưởng tới cache | dữ liệu đi qua cache CPU | phải đồng bộ cache (non-coherent) |
| Địa chỉ | CPU dùng virtual address | thiết bị cần DMA address |
| Độ phức tạp phần mềm | đơn giản | cần mapping, direction, mask, đồng bộ |
| Lỗi điển hình | chậm, mất dữ liệu khi trễ ngắt | dữ liệu cũ do cache, sai địa chỉ, use-after-unmap |
| Trong USB | HCD dùng bộ nhớ riêng trên chip, CPU chép vào (cờ `URB_MAP_LOCAL` trong USB core, chip ISP1760 ở chế độ PIO) | xHCI, EHCI, OHCI chuẩn |

---

## 2.3 Không gian địa chỉ: CPU Virtual, Physical, DMA Address, IOVA

(Chương 1, mục 1.3 đã giải thích nền. Mục này bổ sung các điểm nâng cao.)

### CPU Virtual Address: không phải virtual address nào cũng dùng được cho DMA

```
 Không gian ảo của kernel (arm64)
 ┌───────────────────────────┐
 │ Linear map                │  kmalloc, __get_free_pages, alloc_pages
 │ virt = phys + hằng số     │  → liên tục về vật lý → DÙNG ĐƯỢC cho dma_map_single
 ├───────────────────────────┤
 │ vmalloc area              │  vmalloc, STACK (vì CONFIG_VMAP_STACK=y),
 │ ánh xạ từng trang rời rạc │  bộ nhớ coherent đã remap
 │                           │  → KHÔNG liên tục vật lý → KHÔNG dùng cho dma_map_single
 ├───────────────────────────┤
 │ module, fixmap, I/O       │  thanh ghi MMIO (ioremap) → không phải RAM
 └───────────────────────────┘
```

| Loại buffer | Dùng cho USB URB được không | Lý do |
|---|---|---|
| `kmalloc` (≤ vài trang) | **được** | liên tục vật lý; căn lề tối thiểu 128 byte trên arm64 (`ARCH_DMA_MINALIGN`) |
| `alloc_pages` / `__get_free_pages` | được | liên tục vật lý |
| Biến trên stack | **không** | stack là vmalloc. USB core từ chối kèm cảnh báo `transfer buffer is on stack` |
| `vmalloc` | **không** dùng trực tiếp | không liên tục; phải tự tách trang thành scatter-gather |
| Bộ nhớ coherent (`usb_alloc_coherent`) | được, kèm `URB_NO_TRANSFER_DMA_MAP` | đã có sẵn DMA address, không cần map |

### Physical Address

Địa chỉ trên bus bộ nhớ nhìn từ CPU. Trên Quartz, RAM vật lý nằm ở ba dải, trong đó 3 GiB ở trên 4 GiB (xem Chương 1).

### DMA Address (`dma_addr_t`)

Địa chỉ **thiết bị** phát ra. Trên Quartz, `phys_to_dma` không cộng offset (không có `dma-ranges`), nên DMA address bằng physical address, **trừ khi** buffer bị bounce.

### IOVA (I/O Virtual Address)

```
          Có IOMMU (ví dụ SMMU trên SoC khác)               Quartz (không có SMMU driver)
 xHCI phát IOVA 0x0010_0000                         xHCI phát 0x0812_3000
        │                                                  │
 ┌──────▼──────────────┐                                   │ đi thẳng
 │ IOMMU: bảng trang    │ 0x0010_0000 → 0x2_1234_5000      │
 │ RIÊNG cho thiết bị   │ (thiết bị 32-bit vẫn với tới     │
 └──────┬──────────────┘  RAM cao, KHÔNG cần bounce)       │
        ▼                                                  ▼
       RAM                                                RAM
```

| | |
|---|---|
| **Là gì** | Không gian địa chỉ ảo dành riêng cho thiết bị, do IOMMU dịch |
| **Vì sao** | (1) Cho thiết bị 32-bit với tới RAM cao mà không cần bounce. (2) Ghép nhiều trang rời thành một khối liên tục trong mắt thiết bị. (3) **Cách ly**: thiết bị chỉ chạm được vùng đã map; truy cập sai sẽ gây IOMMU fault thay vì âm thầm ghi đè RAM |
| **Trên Quartz** | Không áp dụng (`CONFIG_ARM_SMMU` không bật, DT không có `iommus`). Vì vậy **một DMA address sai sẽ âm thầm đọc/ghi nhầm RAM**, không có fault nào báo lại. Đây là điểm quan trọng khi debug hỏng dữ liệu |

---

## 2.4 DMA Mapping và DMA Unmapping

### Là gì: hợp đồng chuyển quyền sở hữu

**Mapping** là thao tác "giao buffer cho thiết bị": lấy DMA address hợp lệ và làm cho dữ liệu nhất quán. **Unmapping** là "nhận lại buffer": làm cho CPU thấy đúng dữ liệu thiết bị đã ghi và giải phóng tài nguyên (ô bounce, IOVA).

```
 thời gian ───────────────────────────────────────────────────────────────────►
 │ CPU SỞ HỮU      │ map  │      THIẾT BỊ SỞ HỮU              │unmap │ CPU SỞ HỮU
 │ CPU ghi/đọc     │      │ CPU KHÔNG được đọc/ghi buffer     │      │ CPU đọc kết quả
 │ tự do           │clean │ xHCI đọc/ghi qua DMA              │ inv  │
 │                 │(+copy│                                   │(+copy│
 │                 │bounce│                                   │back) │
```

### Vì sao tồn tại

Driver không biết, và không nên phải biết, board có IOMMU không, coherent không, có cần bounce không. Mapping API gói toàn bộ khác biệt nền tảng vào một lời gọi.

### Hoạt động thế nào trên Quartz (đường dma-direct)

```
 map(dev, buffer, size, dir)
  1. buffer → trang → physical address
  2. physical → DMA address (Quartz: không đổi)
  3. Thiết bị với tới không?  (địa chỉ cuối ≤ min(dma_mask, bus_dma_limit))
       ├─ Có ──► 4. non-coherent: CLEAN cache vùng buffer ─► trả DMA address
       └─ Không ─► SWIOTLB:
              a. tìm ô liên tiếp (mỗi ô 2 KiB, tối đa 128 ô = 256 KiB một lần map)
                 └─ không tìm được ─► "swiotlb buffer is full …" ─► DMA_MAPPING_ERROR
              b. chép buffer gốc → ô bounce (MỌI direction trên 5.10.258)
              c. kiểm tra ô bounce có nằm trong mask không
                 └─ không ─► "swiotlb addr … overflow …" ─► DMA_MAPPING_ERROR
              d. non-coherent: CLEAN cache vùng ô bounce
              e. trả DMA address = địa chỉ ô bounce
       (nếu SWIOTLB bị tắt bằng swiotlb=noforce) ─► WARNING "DMA addr … overflow …" ─► DMA_MAPPING_ERROR

 unmap(dev, dma_addr, size, dir)
  1. DMA address → physical
  2. non-coherent và dir ≠ TO_DEVICE: INVALIDATE cache vùng đó
  3. nếu là ô bounce: dir là FROM_DEVICE/BIDIRECTIONAL thì chép ô bounce → buffer gốc; trả ô
```

Ba nhóm hàm và quy tắc kèm theo:

| Nhóm | Dùng khi | Quy tắc |
|---|---|---|
| map single / map page | một buffer liên tục | **phải kiểm tra lỗi map** (`DMA_MAPPING_ERROR` = toàn bit 1) trước khi đưa địa chỉ cho phần cứng |
| map scatter-gather | nhiều đoạn rời (usb-storage, mạng) | số đoạn trả về có thể nhỏ hơn số đoạn đưa vào nếu IOMMU ghép lại (Quartz: không ghép) |
| sync for cpu / sync for device | muốn dùng lại cùng một mapping nhiều lần mà không unmap | làm đúng các thao tác cache/bounce như unmap/map, nhưng giữ nguyên DMA address |

### Liên quan USB: USB core quyết định cách map cho từng URB

Khi URB được submit, USB core chọn một trong các cách sau. Cờ đánh dấu được lưu vào URB để lúc trả URB biết cách unmap.

| Tình huống của URB | Cách xử lý | Cờ đánh dấu | Nếu thất bại |
|---|---|---|---|
| HCD dùng bộ nhớ riêng trên chip | cấp vùng nhớ của chip và chép vào | `URB_MAP_LOCAL` | `-ENOMEM` |
| Driver bật `URB_NO_TRANSFER_DMA_MAP` (vd `usbfax`) | **không map**, dùng `urb->transfer_dma` driver cung cấp | — | — |
| Có danh sách scatter-gather (`num_sgs > 0`) | map scatter-gather | `URB_DMA_MAP_SG` | `-EAGAIN` |
| Một đoạn sg đơn (`urb->sg`, `num_sgs = 0`) | map page | `URB_DMA_MAP_PAGE` | `-EAGAIN` |
| Buffer nằm trên stack | **từ chối**, `WARN_ONCE("transfer buffer is on stack")` | — | `-EAGAIN` |
| Buffer thường (kmalloc) | map single | `URB_DMA_MAP_SINGLE` | `-EAGAIN` |
| Control transfer | gói SETUP 8 byte được map **riêng**, direction `TO_DEVICE`. Nằm trên stack thì từ chối kèm `setup packet is on stack` | `URB_SETUP_MAP_SINGLE` | `-EAGAIN` |

Hai biệt lệ riêng của xHCI:
- **Immediate Data (IDT):** URB OUT không phải isoc, **≤ 8 byte**, không có sg và không có `URB_NO_TRANSFER_DMA_MAP` thì xHCI **không map**. Nó chép thẳng 8 byte dữ liệu vào trong TRB, nên controller không cần DMA để lấy dữ liệu.
- **Bounce riêng của xHCI:** với một số TD dùng scatter-gather có đoạn không căn đúng kích thước gói, xHCI dùng một bộ đệm nhỏ gắn với ring segment (`bounce_buf`). Bộ đệm này khác SWIOTLB.

### Ví dụ: một URB đi qua map/unmap

usb-storage đọc 64 KiB vào buffer `kmalloc` có physical 0x1_0400_0000 (nằm trên 4 GiB):
1. submit → map single, `FROM_DEVICE`
2. vượt mask 32-bit → SWIOTLB cấp 32 ô (64 KiB), chép buffer gốc sang ô (chống lộ dữ liệu), clean cache ô
3. DMA address trả về (ví dụ minh họa) 0x8912_0000 → ghi vào TRB
4. xHCI ghi 64 KiB vào 0x8912_0000
5. Event → unmap: invalidate cache ô bounce, **chép 64 KiB từ ô về 0x1_0400_0000**, trả 32 ô
6. usb-storage đọc buffer gốc

Cái giá phải trả: hai lần memcpy 64 KiB. Chính tốc độ memcpy của CPU giới hạn băng thông thực tế.

---

## 2.5 Coherent DMA và Streaming DMA

### Coherent DMA (DMA nhất quán)

| | |
|---|---|
| **Là gì** | Vùng nhớ cấp phát một lần, CPU và thiết bị **cùng truy cập bất kỳ lúc nào** mà luôn thấy cùng dữ liệu, không cần map/unmap |
| **Vì sao** | Cấu trúc dùng chung liên tục (ring, context) bị cả hai bên đọc/ghi xen kẽ. Không thể map/unmap mỗi lần |
| **Hoạt động trên hệ coherent** | Phần cứng snoop cache, nên bộ nhớ coherent vẫn dùng cache bình thường |
| **Hoạt động trên Quartz (non-coherent)** | 1) Lấy trang thỏa coherent mask (32-bit → từ `ZONE_DMA`). 2) Dọn sạch cache của các trang đó. 3) **Ánh xạ lại vào vùng vmalloc với thuộc tính Normal Non-Cacheable.** CPU đọc ghi không qua cache, nên luôn khớp với thiết bị. 4) Nếu đang ở ngữ cảnh không được ngủ: lấy từ **atomic pool** đã cấp lúc boot (log `DMA: preallocated 512 KiB …`) |
| **Cái giá** | CPU truy cập chậm (mỗi lần đọc là một lần ra RAM). Không hợp cho buffer lớn mà CPU xử lý nhiều |

**DMA pool:** `dma_alloc_coherent` cấp theo trang, quá phí cho các đối tượng nhỏ. DMA pool cắt trang coherent thành đối tượng nhỏ cùng kích thước.

| Pool (tên thật trong source) | Ai tạo | Chứa gì |
|---|---|---|
| `xHCI ring segments` | xHCI | các segment của Command/Transfer/Event Ring (mỗi segment chứa nhiều TRB) |
| `xHCI input/output contexts` | xHCI | Input Context, Device Context |
| `xHCI 256 byte stream ctx arrays`, `xHCI 1KB stream ctx arrays` | xHCI | stream context (USB3 bulk streams) |
| `buffer-32`, `buffer-128`, `buffer-512`, `buffer-2048` | USB core, phục vụ `usb_alloc_coherent` | buffer dữ liệu nhỏ của driver. Trên arm64, căn lề kmalloc là 128 byte nên **pool 32 byte bị tắt** |

### Streaming DMA (DMA dạng luồng)

| | |
|---|---|
| **Là gì** | Buffer thường (có cache), được giao cho thiết bị bằng map và nhận lại bằng unmap **cho từng lần truyền** |
| **Vì sao** | Dữ liệu lớn, CPU cần đọc ghi nhanh qua cache. Chỉ phải trả giá dọn cache tại thời điểm giao nhận |
| **Hoạt động** | Như mục 2.4. Ngoài cửa sổ map–unmap, buffer thuộc về CPU |
| **Cái giá** | Thao tác cache cho mỗi lần truyền. Có thể phải bounce. **Sai thứ tự (CPU chạm buffer khi thiết bị đang sở hữu) gây hỏng dữ liệu không báo lỗi** |

### So sánh và chỗ dùng trong USB

| | Coherent | Streaming |
|---|---|---|
| Thời gian sống | dài (suốt đời driver) | một lần truyền |
| Thuộc tính cache trên Quartz | **không cache** | có cache, dọn khi map/unmap |
| Vị trí trên Quartz | luôn < 4 GiB (theo coherent mask) | bất kỳ; trên 4 GiB thì bounce |
| CPU truy cập | chậm | nhanh |
| Ai sở hữu lúc nào | cả hai, mọi lúc (cần barrier khi thứ tự quan trọng) | luân phiên rõ ràng |
| Trong xHCI/EHCI | DCBAA, context, mọi ring, ERST, scratchpad (EHCI: QH, qTD, iTD, siTD, frame list) | buffer dữ liệu của URB |
| Trong driver USB | `usb_alloc_coherent` (vd `usbfax`) | `kmalloc` + để USB core tự map (đa số driver) |

```
                    RAM vật lý Quartz (không theo tỉ lệ)
 0x0 ─────────────────────────── 4 GiB ─────────── 0x2_8000_0000
 │ ZONE_DMA                        │ ZONE_NORMAL                  │
 │ ┌──────────┐ ┌──────────────┐   │  ┌─────────────────────────┐ │
 │ │ coherent │ │ SWIOTLB 64MiB│   │  │ kmalloc buffer của URB  │ │
 │ │ rings,ctx│ │ 0x877B5000–  │◄──┼──┤ (streaming) → phải      │ │
 │ │ (uncached│ │ 0x8B7B5000   │   │  │ bounce qua SWIOTLB      │ │
 │ │  map)    │ └──────────────┘   │  └─────────────────────────┘ │
 │ └──────────┘                    │                              │
 │◄──── xHCI với tới (mask 32-bit) │──── xHCI KHÔNG với tới ─────►│
```

---

## 2.6 DMA Direction

### Là gì

Lời khai báo của driver về **chiều dữ liệu đi** trong một mapping. DMA API dựa vào đó để quyết định dọn cache và chép bounce như thế nào.

| Giá trị | Nghĩa | Trong USB |
|---|---|---|
| `DMA_TO_DEVICE` | CPU ghi, thiết bị đọc | URB OUT (endpoint hướng ra), gói SETUP |
| `DMA_FROM_DEVICE` | thiết bị ghi, CPU đọc | URB IN |
| `DMA_BIDIRECTIONAL` | cả hai | hiếm trong USB. An toàn nhưng tốn hơn |
| `DMA_NONE` | không hợp lệ cho mapping thật | chỉ dùng để gỡ lỗi |

USB core tự chọn direction từ hướng endpoint của URB (IN → `FROM_DEVICE`, OUT → `TO_DEVICE`). Driver không chọn.

### Hoạt động trên 5.10.258 (arm64, non-coherent, có SWIOTLB)

| Direction | Cache khi map | Cache khi unmap | Bounce khi map | Bounce khi unmap |
|---|---|---|---|---|
| `TO_DEVICE` | clean | (không làm gì) | chép gốc → ô | (không chép) |
| `FROM_DEVICE` | **clean** | **invalidate** | chép gốc → ô (*chống lộ dữ liệu*) | **chép ô → gốc** |
| `BIDIRECTIONAL` | clean | invalidate | chép gốc → ô | chép ô → gốc |

### Vì sao `FROM_DEVICE` lại clean khi map, không phải invalidate?

```
 Trường hợp invalidate khi map (kernel cũ):
   CPU từng ghi dở dữ liệu nhạy cảm vào buffer (dòng cache bẩn) → invalidate → dòng bẩn bị BỎ,
   RAM giữ dữ liệu CŨ hơn → thiết bị chỉ ghi một phần (short packet/lỗi) → unmap invalidate →
   CPU đọc phần còn lại = dữ liệu cũ trong RAM, KHÔNG phải thứ CPU ghi gần nhất (sai ngữ nghĩa, có thể lộ dữ liệu).

 Trường hợp clean khi map (5.10.258 này):
   Dòng bẩn được đẩy xuống RAM trước → RAM = đúng dữ liệu CPU thấy → thiết bị ghi đè một phần →
   unmap invalidate → CPU thấy: phần thiết bị ghi + phần cũ CPU đã ghi. Nhất quán.
   Không còn dòng bẩn nào có thể bị đẩy xuống đè lên dữ liệu DMA giữa chừng.
```

Lý do SWIOTLB chép cả với `FROM_DEVICE`: nếu thiết bị chỉ ghi một phần ô bounce, phần còn lại của ô có thể chứa dữ liệu của mapping trước (của thiết bị hoặc tiến trình khác). Lúc unmap, phần đó bị chép ngược vào buffer của bạn, có thể lộ ra user space. Chép buffer gốc vào ô trước sẽ xóa dấu vết đó.

### Ví dụ lỗi khi khai báo sai direction

| Sai | Hậu quả trên Quartz |
|---|---|
| URB IN nhưng map `TO_DEVICE` | Unmap không invalidate, không chép bounce về → CPU đọc dữ liệu cũ hoặc nhận buffer gốc chưa được cập nhật |
| URB OUT nhưng map `FROM_DEVICE` | Khi map vẫn clean và vẫn chép bounce (trên kernel này), nên thường "vẫn chạy". Lỗi bị che giấu cho tới khi chạy trên kernel hoặc nền tảng khác |

---

## 2.7 DMA Mask

### Là gì

Giá trị bit cho biết thiết bị **phát được địa chỉ tối đa bao nhiêu bit**. Có hai mask:
- `dma_mask` cho streaming mapping;
- `coherent_dma_mask` cho cấp phát coherent.

Ngoài ra còn `bus_dma_limit`, giới hạn của chính đường bus nối thiết bị (Quartz: không đặt).

### Vì sao tồn tại

Thiết bị có thanh ghi địa chỉ hữu hạn (32-bit, 36-bit, 64-bit), hoặc bus nối tới nó chỉ có số bit địa chỉ hữu hạn. Kernel cần biết để không đưa cho thiết bị một địa chỉ nó không phát ra được.

### Hoạt động

1. Driver đề xuất mask. Kernel kiểm tra nền tảng có hỗ trợ không. Không hỗ trợ thì trả `-EIO`, driver thường thử mask nhỏ hơn.
2. Mỗi lần map: so địa chỉ cuối của buffer với mask. Vượt thì bounce, hoặc báo lỗi.
3. Mỗi lần cấp phát coherent: chọn zone sao cho trang nằm trong coherent mask.

### Liên quan USB trên Quartz: chuỗi quyết định của xHCI

```
 Đọc HCCPARAMS1 từ controller ── bit AC64 = 1 ("tôi hỗ trợ 64-bit")
           │
 KM quirk XHCI_NO_64BIT_SUPPORT (xhci-plat.c)  ── comment KM: thực tế phải là 32-bit
           │  xóa AC64 trong bản sao hcc_params
           ▼
 AC64 = 0 ─► đặt dma_mask = coherent_dma_mask = 32-bit   ("Enabling 32-bit DMA addresses." – debug)
           │
           ├─► coherent (ring, context): luôn cấp dưới 4 GiB
           └─► streaming (dữ liệu URB):   buffer > 4 GiB → SWIOTLB
```

### Ví dụ

| Buffer physical | Kết quả map trên xHCI Quartz |
|---|---|
| 0x0812_3000, dài 4 KiB | DMA = 0x0812_3000, chỉ clean cache |
| 0xFFFF_F000, dài 8 KiB (vắt qua mốc 4 GiB) | địa chỉ cuối vượt mask → **bounce** |
| 0x2_1234_5000, dài 4 KiB | **bounce** |

Một chi tiết dễ bị bỏ qua: so sánh dùng **địa chỉ cuối**, không chỉ địa chỉ đầu.

---

## 2.8 Log của chương này

Định dạng các log dưới đây đã được **kiểm tra trong source 5.10.258**. Giá trị số trong ví dụ là **minh họa**.

### Log A: SWIOTLB hết ô

```
xhci-hcd 8d00d0000.usb3: swiotlb buffer is full (sz: 65536 bytes), total 32768 (slots), used 32700 (slots)
```
Câu chữ đã xác minh ở `kernel/dma/swiotlb.c:588-589`. Các con số là minh họa: 64 MiB / 2 KiB = 32768 ô.

| Mục | Giải thích |
|---|---|
| Ý nghĩa | Không tìm được đủ ô bounce **liên tiếp** cho một lần map |
| Nguồn | SWIOTLB, gọi trong lúc map một buffer vượt mask |
| Điều kiện | Map buffer nằm trên 4 GiB, và kho 64 MiB không còn chỗ trống đủ dài |
| Trước đó | Driver submit URB → USB core map → buffer vượt mask → SWIOTLB tìm ô |
| Sau đó | Map trả `DMA_MAPPING_ERROR` → USB core trả **`-EAGAIN`** cho lần submit URB → driver tự quyết định (thử lại hoặc báo lỗi I/O) |
| Cơ chế | Ô 2 KiB. Một lần map dùng tối đa 128 ô liên tiếp (256 KiB) |
| Nguyên nhân thường gặp | (1) **Rò mapping**: driver map mà không unmap (URB không bao giờ được trả về, như URB treo trong OP_BTS-71095), nên ô bị giữ mãi. (2) Nhiều thiết bị cùng bounce khối lớn. (3) Phân mảnh. (4) Map khối quá lớn |
| Bình thường hay lỗi | Không bao giờ là bình thường. Xuất hiện một lần lúc tải cao có thể tạm chấp nhận nếu driver thử lại; lặp lại hoặc tăng dần thì gần như chắc chắn là rò |
| Cần kiểm tra tiếp | `used` tăng theo thời gian không giảm → rò. Thiết bị nào in log. Có URB nào kẹt không trả về. Thử boot với `swiotlb=<số ô>` để tăng kho, chỉ nhằm phân biệt "thiếu" hay "rò" |

### Log B: DMA address vượt mask khi không có SWIOTLB

```
WARNING: CPU: 1 PID: 1234 at kernel/dma/direct.h:NN dma_direct_map_page+...
xhci-hcd 8d00d0000.usb3: DMA addr 0x0000000212345000+4096 overflow (mask ffffffff, bus limit 0).
```
Câu chữ đã xác minh ở `kernel/dma/direct.h` (`dev_WARN_ONCE`). Dòng `WARNING` và số dòng là minh họa định dạng.

| Mục | Giải thích |
|---|---|
| Ý nghĩa | Buffer vượt mask của thiết bị và **không được phép bounce** |
| Điều kiện | SWIOTLB bị tắt (`swiotlb=noforce`) mà vẫn map buffer vượt mask. Trên cấu hình hiện tại của Quartz (SWIOTLB bật), log này **không nên xuất hiện** |
| Sau đó | In một lần kèm stack trace → map trả lỗi → USB core trả `-EAGAIN` |
| Nguyên nhân | Cấu hình boot tắt SWIOTLB; driver đặt mask sai |
| Cần kiểm tra | Stack trace cho biết driver nào map. So `mask` với địa chỉ |

Biến thể `swiotlb addr … overflow …` (`kernel/dma/swiotlb.c:725`) nghĩa là **chính ô bounce** nằm ngoài mask. Ví dụ: thiết bị có mask 31-bit, trong khi SWIOTLB trên Quartz nằm ở khoảng 2.1 GiB.

### Log C: buffer trên stack

```
WARNING: CPU: 0 PID: 567 at drivers/usb/core/hcd.c:NNNN usb_hcd_map_urb_for_dma+...
transfer buffer is on stack
```
Câu chữ đã xác minh ở `core/hcd.c:1501`. Bản dành cho gói SETUP là `setup packet is on stack` (`hcd.c:1436`).

| Mục | Giải thích |
|---|---|
| Ý nghĩa | Driver USB đưa buffer nằm trên stack vào URB |
| Vì sao bị từ chối | Với `CONFIG_VMAP_STACK=y`, stack nằm trong vmalloc: không liên tục vật lý, không chuyển được sang trang để map. Kể cả khi không có VMAP_STACK, buffer stack có thể chung dòng cache với biến khác, nên dọn cache sẽ làm hỏng biến đó |
| Sau đó | `WARN_ONCE` chỉ in **lần đầu**, sau đó `-EAGAIN` âm thầm cho mọi lần sau |
| Nguyên nhân thường gặp | Gọi `usb_control_msg`/`usb_bulk_msg` với mảng cục bộ |
| Cần kiểm tra | Stack trace chỉ ra driver. Đổi sang `kmalloc` |

### Log D: thiếu coherent mask

```
WARNING: CPU: 2 PID: 89 at kernel/dma/mapping.c:427 dma_alloc_attrs+...
```
Đây là `WARN_ON_ONCE(!dev->coherent_dma_mask)`: không có câu chữ, chỉ có dòng WARNING kèm stack trace.

| Mục | Giải thích |
|---|---|
| Ý nghĩa | Cấp phát coherent cho một thiết bị **chưa từng đặt coherent mask** |
| Nguyên nhân | Driver platform gọi cấp phát trước khi đặt mask (sai thứ tự trong probe), hoặc DT/bus không khởi tạo mask |
| Cần kiểm tra | Stack trace → tên driver → thứ tự trong hàm probe |

### Log E: DMA API debug (đã tắt trên cấu hình hiện tại)

Khi bật `CONFIG_DMA_API_DEBUG`, kernel theo dõi mọi mapping và báo khi dùng sai. Một số câu chữ đã xác minh trong `kernel/dma/debug.c`: `… memory it has not allocated …`, `… DMA memory with wrong function …`, `device driver failed to check map error`, `… to sync DMA memory it has not allocated …`.

| Cảnh báo | Bắt được lỗi gì |
|---|---|
| failed to check map error | driver không kiểm tra lỗi map |
| has not allocated | unmap/free địa chỉ chưa từng map (thường do unmap hai lần) |
| wrong function | map single nhưng unmap bằng hàm của page/sg |
| sync … not allocated | sync trên mapping đã bị unmap |

Đây là công cụ mạnh nhất để bắt lỗi dùng sai DMA API. Bật nó cần build lại kernel (Chương 11).

### Log-to-flow: từ "swiotlb buffer is full" lần ngược về nguyên nhân

```
 [Log] swiotlb buffer is full (… used 32700 …)
   │  Ai in?            SWIOTLB, khi map, cho thiết bị xhci-hcd 8d00d0000.usb3
   ▼
 [Hàm] map single/page/sg trong lúc USB core submit URB
   │  Vì sao lại bounce? buffer physical > 4 GiB, mask xHCI = 32-bit
   ▼
 [Sự kiện] không có đủ ô liên tiếp
   │  Vì sao hết ô?    "used" có giảm theo thời gian không?
   ├─ Có giảm, chỉ lúc tải cao ──► thiếu kho: tăng swiotlb, giảm kích thước map, dùng coherent buffer
   └─ Không giảm, tăng dần ──► RÒ: có mapping không bao giờ được unmap
          ▼
 [Trạng thái phần cứng] có URB nào đang treo trong xHCI không được trả về?
          ▼                 (Event không khớp TD → URB không giveback → không unmap)
 [Nguyên nhân gốc] driver không hủy URB khi timeout / HCD không trả URB khi lỗi
```

---

## 2.9 Tóm tắt Chương 2

1. **Third-party DMA** (DMA Controller, dmaengine) và **first-party DMA** (bus master, DMA mapping API) là hai thế giới khác nhau. USB host thuộc loại sau.
2. Chỉ bộ nhớ **liên tục vật lý trong linear map** (kmalloc, page) mới map được. Stack và vmalloc thì không. Trên Quartz, stack là vmalloc.
3. Mapping là **hợp đồng chuyển quyền sở hữu**. Trên 5.10.258/arm64: map luôn **clean**, unmap **invalidate** (trừ TO_DEVICE). SWIOTLB chép gốc → ô khi map **với mọi direction**, và chép ô → gốc khi unmap với FROM_DEVICE/BIDIRECTIONAL.
4. **Coherent** = cấp phát một lần. Trên Quartz là bộ nhớ **không cache**, dùng cho ring và context. **Streaming** = buffer có cache, map/unmap mỗi lần, dùng cho dữ liệu.
5. **Direction** do USB core suy ra từ endpoint. Nó quyết định thao tác cache và hướng chép bounce.
6. **Mask** 32-bit của xHCI trên Quartz do quirk KM đặt. Kiểm tra mask dựa trên **địa chỉ cuối** của buffer. Vượt mask thì bounce. SWIOTLB mỗi lần map tối đa 256 KiB, cả kho 64 MiB.
7. Không có IOMMU nên **DMA address sai sẽ âm thầm ghi nhầm RAM**, không có fault.
8. Log quan trọng nhất của chương: `swiotlb buffer is full` thường là dấu hiệu **rò mapping** do URB không được trả về.

## Câu hỏi kiểm tra

1. Vì sao `mv61x0-vdma` dùng dmaengine API, còn xHCI dùng DMA mapping API? Hai API này giải quyết hai vấn đề khác nhau thế nào?
2. Một driver gọi `usb_bulk_msg` với `u8 buf[64]` khai báo cục bộ. Trên Quartz chuyện gì xảy ra, ở lần gọi đầu và ở các lần sau?
3. Vì sao trên 5.10.258 việc map `DMA_FROM_DEVICE` lại **clean** chứ không invalidate? Nêu một tình huống mà invalidate khi map gây kết quả sai.
4. SWIOTLB chép buffer gốc vào ô bounce ngay cả với `DMA_FROM_DEVICE`. Nếu không chép thì điều gì có thể bị lộ, và lộ cho ai?
5. Buffer dài 8 KiB bắt đầu ở physical 0xFFFF_F000. Map cho xHCI trên Quartz có bị bounce không? Vì sao?
6. Vì sao ring của xHCI phải là bộ nhớ coherent, còn buffer dữ liệu thì thường là streaming? Cái giá của việc dùng coherent cho buffer dữ liệu lớn là gì?
7. `usbfax` dùng `usb_alloc_coherent`. Trên Quartz, buffer đó nằm ở zone nào, có cache không, và có bao giờ cần bounce không?
8. Bạn thấy `swiotlb buffer is full` với `used` tăng dần qua nhiều giờ. Hãy lập luận ngược (log → hàm → sự kiện → trạng thái phần cứng → nguyên nhân) và nói bước kiểm tra đầu tiên.
9. URB OUT 6 byte gửi tới thiết bị HS bulk (max packet 512) trên xHCI: dữ liệu có được map DMA không? Controller lấy dữ liệu từ đâu?
10. Vì sao thiếu IOMMU khiến lỗi DMA address sai trên Quartz khó phát hiện hơn trên một SoC có SMMU?

---

**Chương tiếp theo:** Chương 3 dựng lại toàn bộ vòng đời một lần truyền USB (Bulk OUT và Bulk IN), tách rõ bốn làn: CPU software, DMA subsystem, phần cứng host controller và giao dịch trên bus USB, kèm sequence diagram chi tiết cho xHCI.
