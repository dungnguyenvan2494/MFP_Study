# Chương 1: Hiểu DMA trong kiến trúc USB

> Khoá học USB DMA trên Linux Kernel. Baseline: **Linux 5.10.258** (`Kernel/Src` của repo `Source_MFP`), board Marvell Armada Quartz EMU 800 (arm64). Mọi log trong chương đều được đối chiếu với source hoặc trích từ log thật của board.

## 1.0 Mốc tham chiếu (baseline)

**Kernel:** khoá học dùng **Linux 5.10.258**, là bản trong `Kernel/Src` và khớp với log thực tế (`Linux version 5.10.258`). Chỗ nào khác giữa các phiên bản (ví dụ 5.4.x) sẽ được ghi chú riêng.

Bốn đặc điểm DMA của board (đã kiểm tra từ log và source) sẽ được dùng xuyên suốt khoá học:

| Đặc điểm | Giá trị trên Quartz EMU800 | Nguồn kiểm chứng |
|---|---|---|
| RAM | Dưới 4 GiB (0x0 – 0xBFFF_FFFF, có lỗ) **và trên 4 GiB** (0x1_0000_0000 – 0x1_3FFF_FFFF, 0x2_0000_0000 – 0x2_7FFF_FFFF) | log boot `Early memory node ranges` |
| DMA mask của xHCI | **32-bit**: KM ép quirk `XHCI_NO_64BIT_SUPPORT`, xóa bit AC64 nên driver chọn mask 32-bit | `xhci-plat.c:99-105`, `xhci.c:5362-5380` |
| Cache coherency | **Không coherent**: DT không có `dma-coherent`, nên CPU phải tự dọn cache | `armada-quartz-*.dts`, `quartz-*.dtsi` |
| IOMMU | **Không có cho xHCI**: DT không có `iommus` → DMA address = physical address | như trên |
| SWIOTLB | Vùng bounce **64 MiB** tại 0x877B5000 – 0x8B7B5000 (dưới 4 GiB) | log boot `software IO TLB: mapped` |

Hệ quả trực tiếp: **xHCI trên board này không với tới được nửa RAM phía trên 4 GiB.** Khoảng một nửa nội dung của các chương sau xoay quanh câu này.

---

## 1.1 Vì sao USB Host Controller cần DMA

### Vấn đề: CPU tự chép từng byte thì quá đắt

| Chuẩn | Tốc độ bus | Dữ liệu thực tế mỗi giây | Đơn vị lập lịch |
|---|---|---|---|
| USB 2.0 High-Speed | 480 Mb/s | khoảng 40–50 MB/s | microframe 125 µs |
| USB 3.x SuperSpeed | 5 Gb/s | khoảng 400–450 MB/s | bus interval 125 µs |

Nếu dùng **PIO** (Programmed I/O, CPU tự chép), mỗi gói bulk HS dài 512 byte buộc CPU phải:
1. bị ngắt;
2. đọc hoặc ghi 128 lần vào thanh ghi FIFO 32-bit (truy cập MMIO không qua cache, mỗi lần tốn hàng trăm ns);
3. làm như vậy hàng chục nghìn lần mỗi giây, cho từng endpoint.

CPU sẽ bận gần hết thời gian, và chỉ cần trễ ngắt vài chục µs là trượt microframe, mất băng thông.

### Giải pháp: controller tự làm chủ bus (bus mastering)

```
           PIO (CPU chép từng word)                      DMA bus master (xHCI)
 ┌─────┐  đọc/ghi FIFO  ┌──────────┐        ┌─────┐ 1.viết "phiếu việc" ┌───────────┐
 │ CPU │◄──────────────►│Controller│        │ CPU │────────────────────►│   RAM     │
 └──┬──┘  × hàng triệu  └────┬─────┘        └──┬──┘                     │ TRB, data │
    │ ngắt mỗi gói          USB               │ 2.bấm doorbell (1 lần)   └─────▲─────┘
    └─────────────────────────┘                ▼                               │ 3.tự đọc TRB,
                                         ┌──────────┐                          │   tự đọc/ghi data
                                         │   xHCI   │◄─────────────────────────┘
                                         └────┬─────┘ 4.tự retry khi device NAK
                                              │       5.ghi Event + ngắt 1 lần khi xong
                                             USB
```

**Bus mastering (làm chủ bus):** chính thiết bị phát lệnh đọc/ghi lên bus bộ nhớ (AXI/PCIe), không cần CPU trung gian. xHCI, EHCI, NVMe, card mạng (`mvri`) đều làm như vậy.

**DMA Controller (bộ điều khiển DMA riêng):** là một khối DMA dùng chung, chép dữ liệu hộ cho các thiết bị *không* tự làm chủ bus (UART, audio…). Log boot của board có cả loại này:
```
[    0.428810] mv61x0-vdma quartz-sb:vdma-owned: Virtual DMA Controller type 0:owned, 3 virt channels, 3 phys channels
[18.725196] dw-apb-uart 8e8005800.uart: failed to request DMA
```
→ **USB không dùng khối này.** DMA engine nằm sẵn bên trong xHCI. Đây là chỗ người mới hay nhầm khi đọc log.

---

## 1.2 CPU, RAM, DMA subsystem và xHCI tương tác thế nào

### Kiến trúc phân lớp (tách rõ software và hardware)

```
╔══════════════════════════ SOFTWARE (CPU thực thi) ═══════════════════════════╗
║  Driver lớp trên (usbfax, usb-storage, cdc…)   tạo dữ liệu, tạo URB          ║
║            │ submit URB                                                       ║
║  USB Core (usbcore)                            kiểm tra URB, nhờ map buffer   ║
║            │                                                                  ║
║  HCD: xHCI driver                              viết TRB, bấm doorbell,        ║
║            │                                   xử lý Event, trả URB           ║
║  Linux DMA API (dma-direct / swiotlb / iommu)  ĐỔI địa chỉ + DỌN cache        ║
║                                                (KHÔNG chép dữ liệu qua USB)   ║
╚════════════╤═════════════════════════════════════════════════════╤═══════════╝
   load/store│ qua MMU + cache                            MMIO write│(doorbell, thanh ghi)
╔════════════▼═════════════════ HARDWARE ═════════════════════════▼════════════╗
║ ┌──────────┐  ┌─────────────┐                              ┌──────────────┐  ║
║ │ CPU core │──│ Cache L1/L2 │──┐                           │ xHCI (bus    │  ║
║ └──────────┘  └─────────────┘  │                           │ master, DMA  │  ║
║                                ▼                           │ engine riêng)│  ║
║                     ┌─────────────────────┐  DMA read/write│              │  ║
║                     │ Interconnect (AXI)  │◄───────────────┤              │  ║
║                     └──────────┬──────────┘   (không có    └──────┬───────┘  ║
║                                ▼               IOMMU trên Quartz) │ IRQ      ║
║                     ┌─────────────────────┐                       │ (SPI 272)║
║                     │ RAM                 │                       ▼          ║
║                     │  • rings, contexts  │                 tới CPU (GIC)    ║
║                     │  • data buffers     │                                  ║
║                     │  • swiotlb 64 MiB   │                                  ║
║                     └─────────────────────┘                                  ║
╚══════════════════════════════════════════════════════════════════╤═══════════╝
                                                        USB packets │ (OUT/IN/DATA/ACK/NAK)
                                                         ┌──────────▼─────────┐
                                                         │ Hub 3-1 → FK-513   │
                                                         └────────────────────┘
```

### Bốn kênh giao tiếp

| Kênh | Ai → ai | Mang gì | Đặc điểm |
|---|---|---|---|
| **MMIO** (memory-mapped I/O) | CPU → thanh ghi xHCI | doorbell, Run/Stop, địa chỉ ring | nhỏ, không cache, ít lần |
| **DMA** | xHCI ↔ RAM | TRB, context, dữ liệu, Event | lớn, liên tục, CPU không tham gia |
| **Interrupt** | xHCI → CPU | "có Event mới trong Event Ring" | một lần cho mỗi đợt hoàn thành |
| **USB bus** | xHCI ↔ thiết bị | token, gói DATA, ACK/NAK/STALL | do xHCI tự lập lịch |

### Hai loại bộ nhớ mà xHCI truy cập

| Loại | Ví dụ | Sống bao lâu | Cách cấp phát | Trên Quartz |
|---|---|---|---|---|
| **Cấu trúc điều khiển** | DCBAA, Device Context, Command/Transfer/Event Ring, ERST, scratchpad | suốt đời controller (tới lần resume kế tiếp) | **coherent** | luôn dưới 4 GiB, CPU truy cập không qua cache |
| **Bộ đệm dữ liệu** | buffer của mỗi URB | từng lần truyền | thường **streaming** (map/unmap mỗi lần) | có thể ở trên 4 GiB → cần bounce |

### Ai làm việc gì

| Việc | CPU (software) | Linux DMA API | xHCI (hardware) | USB bus |
|---|---|---|---|---|
| Chuẩn bị dữ liệu | ✔ | | | |
| Đổi sang DMA address, dọn cache, bounce | | ✔ (CPU thực thi) | | |
| Viết TRB, bấm doorbell | ✔ | | | |
| Đọc TRB, đọc/ghi dữ liệu trong RAM | | | ✔ | |
| Gửi gói, retry khi NAK, đổi data toggle | | | ✔ | ✔ |
| Ghi Event, phát ngắt | | | ✔ | |
| Đọc Event, trả URB | ✔ | | | |

> **"DMA subsystem" trong Linux là phần mềm.** Nó không chuyển dữ liệu. Nó chỉ trả lời hai câu hỏi: *thiết bị phải dùng địa chỉ nào?* và *cache phải dọn thế nào để hai bên thấy cùng dữ liệu?* Người chuyển dữ liệu thật là xHCI.

---

## 1.3 Ba loại địa chỉ: virtual, physical, DMA

| Thuật ngữ | Tiếng Việt | Ai dùng | Ai dịch |
|---|---|---|---|
| **CPU Virtual Address** | địa chỉ ảo của CPU | code C (con trỏ) | MMU của CPU, qua bảng trang |
| **Physical Address** | địa chỉ vật lý | bus bộ nhớ, nhìn từ phía CPU | (là đích cuối cùng) |
| **DMA Address** (`dma_addr_t`), còn gọi bus address | địa chỉ DMA | **thiết bị** phát lên bus khi làm chủ bus; là giá trị ghi vào TRB | IOMMU hoặc bộ dịch trên bus (nếu có) |
| **IOVA** (I/O Virtual Address) | địa chỉ ảo của I/O | thiết bị, khi có IOMMU | IOMMU, qua bảng trang riêng của thiết bị |

### Vì sao DMA address không nhất thiết bằng physical address

```
 Con trỏ trong driver
 buf = 0xffffff81_2345_6000        (CPU virtual)
        │  MMU CPU
        ▼
 Physical 0x2_1234_5000            (RAM thật, vùng TRÊN 4 GiB)
        │
        │  Thiết bị phát địa chỉ gì để chạm tới chỗ này?
        ├──(1) Có IOMMU ─────────► IOVA, ví dụ 0x0000_F000 (IOMMU dịch về 0x2_1234_5000)
        ├──(2) Bus có offset ────► DMA = phys + offset   (dma-ranges trong DT, cửa sổ inbound PCIe)
        ├──(3) Thiết bị không với tới (mask 32-bit < 0x2_1234_5000)
        │        └► SWIOTLB: mượn ô bounce 0x8800_0000 dưới 4 GiB, chép dữ liệu qua lại
        │           DMA = 0x8800_0000  ← là physical của Ô BOUNCE, không phải buffer của bạn
        └──(4) Không có các trường hợp trên ─► DMA = physical
```

Trên Quartz: (1) không, (2) không, **(3) có** với buffer nằm trên 4 GiB, (4) có với buffer dưới 4 GiB.

Ví dụ cụ thể (các địa chỉ chỉ để minh họa):

| | Buffer A (kmalloc, rơi vào RAM thấp) | Buffer B (kmalloc, rơi vào RAM cao) |
|---|---|---|
| Virtual | 0xffffff80_0812_3000 | 0xffffff81_2345_6000 |
| Physical | 0x0812_3000 | 0x2_1234_5000 |
| So với mask 32-bit | vừa | **vượt** |
| DMA address ghi vào TRB | 0x0812_3000 | **0x8800_0000** (ô bounce trong 0x877B5000–0x8B7B5000) |
| Chi phí phụ | dọn cache | dọn cache **và chép 2 lần** (memcpy) |

**Ba quy tắc rút ra:**
1. Không bao giờ đưa **virtual address** cho phần cứng. Thiết bị không có MMU của CPU.
2. Không dùng `virt_to_phys()` để lấy địa chỉ cho thiết bị. Nó bỏ qua IOMMU, offset và bounce. Luôn đi qua DMA API.
3. DMA address chỉ có giá trị **trong khoảng thời gian đang được map**. Sau khi unmap, ô bounce hoặc IOVA có thể đã thuộc về người khác.

---

## 1.4 Ví dụ hoàn chỉnh: Bulk OUT và Bulk IN

Ký hiệu làn: **[SW]** CPU chạy driver, **[API]** DMA API (cũng do CPU chạy), **[HW]** xHCI, **[BUS]** đường USB, **[DEV]** thiết bị.

### Kịch bản A: Bulk OUT (host → thiết bị), ví dụ ghi 16 KiB tới bo FAX

```
 Driver      USB Core     DMA API          xHCI driver        xHCI (HW)              Device
   │ ghi data   │             │                  │                 │                    │
   │ vào buf    │             │                  │                 │                    │
   │ (có thể    │             │                  │                 │                    │
   │ còn trong  │             │                  │                 │                    │
   │ cache)     │             │                  │                 │                    │
   │─submit────►│             │                  │                 │                    │
   │            │─map TO_DEV─►│                  │                 │                    │
   │            │             │ dọn cache (clean)│                 │                    │
   │            │             │ [buf>4GiB: chép  │                 │                    │
   │            │             │  sang ô bounce]  │                 │                    │
   │            │◄─dma addr───│                  │                 │                    │
   │            │─enqueue────────────────────────►│                 │                    │
   │            │             │                  │ viết Normal TRB │                    │
   │            │             │                  │ (addr,len,IOC)  │                    │
   │            │             │                  │ lật cycle bit   │                    │
   │            │             │                  │ cuối cùng, wmb  │                    │
   │            │             │                  │─doorbell(MMIO)─►│                    │
   │            │             │                  │                 │─DMA đọc TRB        │
   │            │             │                  │                 │─DMA đọc data       │
   │            │             │                  │                 │═OUT+DATA 512B═════►│
   │            │             │                  │                 │◄════════NAK/ACK════│
   │            │             │                  │                 │ (tự retry, PING)   │
   │            │             │                  │                 │ … ×32 gói          │
   │            │             │                  │                 │─DMA ghi Event TRB  │
   │            │             │                  │                 │ (ptr TRB, SUCCESS, │
   │            │             │                  │                 │  còn lại 0)        │
   │            │             │                  │◄═══IRQ══════════│                    │
   │            │             │                  │ đọc Event, KHỚP │                    │
   │            │             │                  │ với TD đang chờ │                    │
   │            │◄─unmap─────────────────────────│                 │                    │
   │            │             │ trả ô bounce     │                 │                    │
   │◄─callback──│ (giveback)  │                  │                 │                    │
```

| # | Bước | Ai | Chuyện gì xảy ra bên trong |
|---|---|---|---|
| 1 | Driver ghi dữ liệu vào buffer | SW | Dữ liệu có thể **vẫn nằm trong cache** của CPU, chưa xuống RAM |
| 2 | Submit URB | SW | USB Core kiểm tra URB, chuyển cho HCD |
| 3 | Map `DMA_TO_DEVICE` | API | Non-coherent: **clean** (đẩy dòng cache bẩn xuống RAM). Buffer > 4 GiB: lấy ô swiotlb, **chép dữ liệu sang ô bounce**, clean ô đó. Trả về DMA address |
| 4 | Viết TRB | SW | Normal TRB chứa DMA address, độ dài, cờ IOC (báo ngắt khi xong). TRB không được vắt qua ranh giới 64 KiB, nếu cần thì tách thành nhiều TRB. **Lật cycle bit sau cùng**: đây là thao tác trao quyền sở hữu TRB cho phần cứng |
| 5 | Doorbell | SW→HW | Một lần ghi MMIO: "slot X, endpoint Y có việc" |
| 6 | Đọc TRB | HW | Controller đọc TRB qua DMA tại con trỏ dequeue của endpoint (lưu trong Endpoint Context) |
| 7 | Đọc dữ liệu | HW | Đọc từ DMA address trong TRB, theo từng khối |
| 8 | Truyền | BUS | Token OUT + gói DATA 512 B (HS). Thiết bị NAK thì controller tự thử lại, CPU không hề biết |
| 9 | Ghi Event | HW | Transfer Event TRB gồm: **địa chỉ TRB vừa xong**, completion code, số byte còn lại. Ghi qua DMA vào Event Ring |
| 10 | Ngắt | HW→SW | Interrupter phát IRQ |
| 11 | Xử lý Event | SW | **Khớp địa chỉ TRB trong Event với TD đang chờ.** Đây chính là bước báo lỗi `TRB DMA ptr not part of current TD` trong OP_BTS-71095 |
| 12 | Unmap | API | Với OUT: không có gì phải chép ngược. Trả ô bounce về pool |
| 13 | Trả URB | SW | `status = 0`, `actual_length = 16384`, gọi callback để đánh thức tiến trình đang ghi |

### Kịch bản B: Bulk IN (thiết bị → host)

| # | Bước | Ai | Khác với OUT ở đâu |
|---|---|---|---|
| 1 | Submit URB đọc | SW | Buffer rỗng, chờ được ghi vào |
| 2 | Map `DMA_FROM_DEVICE` | API | Non-coherent: trên 5.10.258 là **clean** (đẩy mọi dòng bẩn xuống RAM trước, để sau này không có dòng nào bị đẩy xuống đè lên dữ liệu DMA). Kernel cũ hơn thì invalidate ở bước này. Có bounce: vẫn **chép buffer gốc sang ô bounce** (bản vá chống lộ dữ liệu cũ trong ô bounce) |
| 3 | Viết TRB | SW | Thêm cờ ISP (báo khi gặp short packet) |
| 4 | Doorbell | SW→HW | |
| 5 | Token IN | HW/BUS | Thiết bị chưa có dữ liệu thì NAK. **Controller cứ thế thử lại mãi**, nên timeout phải do driver tự đặt |
| 6 | Nhận dữ liệu | BUS→HW | Gói DATA từ thiết bị |
| 7 | **Ghi vào RAM** | HW | Controller ghi dữ liệu thẳng vào RAM (vào ô bounce nếu có). CPU không thấy gì |
| 8 | Kết thúc | HW | Đủ byte (SUCCESS), hoặc gói ngắn hơn max packet (**SHORT_PACKET**, kèm số byte còn lại) → ghi Event, phát ngắt |
| 9 | Xử lý Event | SW | `actual_length = yêu cầu − còn lại` |
| 10 | Unmap | API | **Invalidate** (bỏ các dòng cũ, kể cả dòng CPU đã nạp trước một cách suy đoán trong lúc DMA chạy). Có bounce: **chép ô bounce → buffer gốc** |
| 11 | CPU đọc dữ liệu | SW | Lúc này CPU nạp dữ liệu mới từ RAM vào cache |

### So sánh IN và OUT

| | Bulk OUT | Bulk IN |
|---|---|---|
| Ai ghi RAM | CPU | **xHCI** |
| Thao tác cache khi map | **clean** (đẩy xuống RAM) | **clean** trên 5.10.258 (kernel cũ: invalidate) |
| Thao tác cache khi unmap | không cần | **invalidate** |
| Chép swiotlb | lúc **map** (gốc → bounce) | lúc map (gốc → bounce, chỉ để chống lộ dữ liệu) **và** lúc **unmap** (bounce → gốc, đây mới là dữ liệu thật) |
| "Hoàn tất" nghĩa là | thiết bị đã ACK hết dữ liệu | đủ byte hoặc gặp short packet |
| Nếu xử lý cache sai | thiết bị nhận **dữ liệu cũ** | CPU đọc **dữ liệu cũ hoặc lẫn lộn**, thường hỏng theo khối 64 byte (đúng kích thước dòng cache arm64) |

### Ứng dụng vào `usbfax` trên board

`usbfax` không dùng streaming DMA. Nó cấp buffer **coherent** sẵn (`usb_alloc_coherent`, `usbfax.c:1108-1138`) và bật cờ `URB_NO_TRANSFER_DMA_MAP`. Vì vậy bước 3 và bước 12 bị bỏ qua: không dọn cache, không bounce, buffer luôn nằm dưới 4 GiB và CPU truy cập nó không qua cache.

**Phát hiện phụ, cần xác nhận thêm:**
- Đường gửi lại phần còn thiếu của một lần ghi (`usbfax_softint`) dời **con trỏ virtual** tới `tx_buffer + txdatasize`, nhưng vẫn đặt **DMA address** `transfer_dma = usbfax_dma1`, tức địa chỉ đầu buffer.
- Khi có `URB_NO_TRANSFER_DMA_MAP`, HCD chỉ dùng DMA address. Nếu đường này chạy, thiết bị sẽ nhận lại **dữ liệu từ đầu buffer**, không phải phần còn thiếu.
- Đây là ví dụ điển hình của việc "dời con trỏ virtual nhưng quên dời DMA address". Đường này chỉ chạy khi một lần ghi bị truyền thiếu.

---

## 1.5 Bộ log đầu tiên: bức tranh DMA lúc khởi động

### Nhóm 1: log thật trên board (trích log OP_BTS-71321, lúc khởi động lại 08:41)

**Log 1: phân vùng bộ nhớ**
```
[    0.000000] Zone ranges:
[    0.000000]   DMA      [mem 0x0000000000000000-0x00000000ffffffff]
[    0.000000]   DMA32    empty
[    0.000000]   Normal   [mem 0x0000000100000000-0x000000027fffffff]
```

| Mục | Giải thích |
|---|---|
| Ý nghĩa | Kernel chia RAM thành các **zone**. `ZONE_DMA` = 4 GiB đầu tiên, `ZONE_DMA32` rỗng, `ZONE_NORMAL` = phần trên 4 GiB |
| Nguồn | Bộ quản lý bộ nhớ (mm), lúc khởi tạo zone trên arm64 |
| Khi nào | Mỗi lần boot, rất sớm |
| Cơ chế | Cấp phát có cờ `GFP_DMA` chỉ lấy từ `ZONE_DMA`, tức luôn dưới 4 GiB. Cấp phát thông thường (`GFP_KERNEL`) **có thể rơi vào `Normal`**, tức trên 4 GiB |
| Liên quan USB | Buffer `kmalloc` thông thường có thể nằm trên 4 GiB, trong khi xHCI chỉ với tới 32-bit → cần bounce |
| Bình thường hay lỗi | Bình thường. Đây là bản đồ để suy luận, không phải lỗi |
| Cần kiểm tra tiếp | Tỉ lệ RAM cao/thấp (trên board: 3 GiB trên 4 GiB, khoảng 3 GiB dưới) → khả năng một buffer phải bounce là đáng kể |

**Log 2: vùng bounce buffer**
```
[    0.000000] software IO TLB: mapped [mem 0x00000000877b5000-0x000000008b7b5000] (64MB)
```

| Mục | Giải thích |
|---|---|
| Ý nghĩa | SWIOTLB đã giữ sẵn **64 MiB dưới 4 GiB** làm kho ô bounce |
| Nguồn | `kernel/dma/swiotlb.c` (tiền tố `software IO TLB:`) |
| Khi nào | Lúc boot. arm64 bật SWIOTLB khi RAM vượt quá vùng mà mọi thiết bị đều với tới |
| Trước / sau | Sau khi zone được dựng. Từ đây mọi lần map buffer vượt mask sẽ lấy ô từ kho này |
| Cơ chế | Map: cấp ô và chép gốc → bounce (mọi direction trên 5.10.258). Unmap: chép ngược bounce → gốc (với IN và BIDIRECTIONAL) rồi trả ô |
| Bình thường hay lỗi | Bình thường. Bất thường là khi sau này thấy log `swiotlb buffer is full` (Chương 2 và Chương 7): kho hết ô, map thất bại |
| Cần kiểm tra tiếp | Thiết bị nào đang dùng bounce (DMA API debug, `/sys/kernel/debug/swiotlb` trên kernel mới hơn) |

**Log 3: pool cấp phát coherent trong ngữ cảnh atomic**
```
[    0.156801] DMA: preallocated 512 KiB GFP_KERNEL pool for atomic allocations
[    0.156886] DMA: preallocated 512 KiB GFP_KERNEL|GFP_DMA pool for atomic allocations
[    0.156968] DMA: preallocated 512 KiB GFP_KERNEL|GFP_DMA32 pool for atomic allocations
```

| Mục | Giải thích |
|---|---|
| Ý nghĩa | Giữ sẵn 3 pool 512 KiB để phục vụ `dma_alloc_coherent` khi **không được ngủ** (atomic context) |
| Nguồn | `kernel/dma/pool.c` |
| Vì sao cần | Trên hệ **non-coherent** (như Quartz), bộ nhớ coherent phải được ánh xạ lại thành không cache. Việc ánh xạ lại có thể phải ngủ, nên trong atomic context phải lấy từ pool đã chuẩn bị sẵn |
| Liên quan USB | xHCI nới rộng ring (ring expansion) trong lúc giữ spinlock, tức là cấp phát atomic → lấy từ pool này. Bản `GFP_DMA` đảm bảo dưới 4 GiB, khớp mask 32-bit |
| Bình thường hay lỗi | Bình thường. Lỗi về sau sẽ là cấp phát atomic thất bại, khiến xHCI không nới được ring |

**Log 4: chính sách IOMMU**
```
[    0.170475] iommu: Default domain type: Translated
```

| Mục | Giải thích |
|---|---|
| Ý nghĩa | Chính sách **mặc định** cho thiết bị nằm sau IOMMU là dịch địa chỉ (IOVA) |
| Nguồn | `drivers/iommu/iommu.c` |
| **Bẫy khi đọc log** | Dòng này **không** có nghĩa xHCI đang dùng IOMMU. Trên Quartz, node xHCI trong DT không có `iommus`, nên xHCI đi thẳng: DMA address = physical |
| Cần kiểm tra tiếp | Có thiết bị nào thực sự gắn với IOMMU không: `ls /sys/kernel/iommu_groups/`, `/sys/bus/platform/devices/8d00d0000.usb3/iommu_group` |

**Log 5: log tùy biến của KM (không có ở upstream)**
```
[   62.499269] xhci-hcd 8d00d0000.usb3: OP_BTS-69673: usb fix version = V10 (no xhci-level Address Device retry)
```
Đây là patch của KM tự in phiên bản bản vá USB, không phải log chuẩn. Nó cho biết xHCI được nạp vào khoảng giây thứ 62 sau boot, tức driver được nạp muộn (dạng module), không phải lúc kernel vừa khởi động.

### Nhóm 2: log probe chuẩn của xHCI và hub

Định dạng các log dưới đây đã được **kiểm tra trong source 5.10.258**. Chúng **không xuất hiện trên console của board**: hoặc vì mức `KERN_INFO`/debug bị loglevel chặn, hoặc vì in ra lúc console chưa được ghi lại. Muốn thấy thì dùng `dmesg`.

| Log (định dạng trong source) | Nguồn | Nói gì về DMA và controller |
|---|---|---|
| `new USB bus registered, assigned bus number N` | `core/hcd.c:937` | Root hub đã được tạo. Trên Quartz có usb1–usb4 (2 controller × 2 root hub) |
| `irq N, io mem 0x8d00d0000` | `core/hcd.c:2626` | Thanh ghi MMIO đã được ánh xạ, IRQ đã gắn. Từ lúc này CPU nói chuyện được với controller |
| `hcc params 0x… hci version 0x… quirks 0x…` | `host/xhci.c:5390` | **Quan trọng nhất cho DMA.** In ra *sau* khi đã chọn mask và cấp phát xong mọi ring/context. Trên Quartz, `hcc params` in ra đã bị **xóa bit 0 (AC64)**, và `quirks` có bit `NO_64BIT_SUPPORT` cùng `RESET_ON_RESUME` |
| `Enabling 32-bit DMA addresses.` | `host/xhci.c:5379` | Mask DMA chọn 32-bit. Đây là `xhci_dbg` (tức `dev_dbg`). Cấu hình `km_mvebu_v8_lsp_defconfig` tắt `CONFIG_DYNAMIC_DEBUG`, nên trên bản build hiện tại dòng này **không thể in ra**, trừ khi build lại xHCI với `DEBUG` |
| `USB hub found` / `N ports detected` | `core/hub.c:1943`, `1507` | Root hub hoạt động, controller đã chạy (bit Run) và Event Ring đã nhận sự kiện cổng |
| `%s %s USB device number %d using %s`, ví dụ `new high-speed USB device number 93 using xhci-hcd` | `core/hub.c:4994` | Lệnh Address Device qua **Command Ring** đã thành công, controller đã đọc được Input Context qua DMA. Chữ đầu là **`new`** khi cắm mới, **`reset`** khi reset-resume |

Thứ tự probe (luồng phía sau các log):
```
nạp driver → map MMIO + gắn IRQ ─────────────► "irq N, io mem …"
          → đọc HCCPARAMS (+ quirk KM xóa AC64)
          → chọn DMA mask 32-bit ────────────► "Enabling 32-bit DMA addresses." (debug)
          → cấp phát coherent: DCBAA, Command Ring, Event Ring + ERST, scratchpad
          → ghi các DMA address đó vào thanh ghi ► "hcc params … quirks …"
          → tạo root hub ────────────────────► "new USB bus registered …"
          → bật Run, bật ngắt
          → hub driver dò cổng ──────────────► "USB hub found", "N ports detected"
          → Enable Slot + Address Device ────► "new high-speed USB device number …"
```
Nếu thiếu một dòng giữa chuỗi này, bước ngay trước nó là chỗ đáng nghi nhất.

**Cách xem các log bị ẩn:**
```bash
dmesg | grep -iE "xhci|usb [0-9]|hub|swiotlb|DMA:"
```
```bash
dmesg -n 8
```
Lệnh thứ hai cho phép in mọi mức log ra console. Riêng `xhci_dbg` thì cần kernel bật `CONFIG_DYNAMIC_DEBUG` rồi nạp module kèm `dyndbg=+p`. Cấu hình `km_mvebu_v8_lsp_defconfig` đang tắt tùy chọn này, nên muốn thấy phải build lại.

---

## 1.6 Tóm tắt Chương 1

1. USB host controller là **bus master**: CPU chỉ viết "phiếu việc" (TRB) và bấm doorbell. Controller tự đọc/ghi RAM và tự retry trên bus. Khối DMA dùng chung của SoC không liên quan tới USB.
2. **Linux DMA API là phần mềm**: nó đổi địa chỉ và giữ cache nhất quán, không chép dữ liệu qua USB.
3. Ba loại địa chỉ: **virtual** (CPU dùng), **physical** (RAM thật), **DMA** (thiết bị dùng). Chúng khác nhau khi có IOMMU, offset trên bus, hoặc **bounce buffer**.
4. Trên Quartz: mask 32-bit, non-coherent, không IOMMU, 3 GiB RAM nằm trên 4 GiB. Vì vậy buffer streaming có thể phải bounce qua SWIOTLB 64 MiB, và CPU luôn phải dọn cache.
5. Trên 5.10.258: **OUT** clean khi map, chép bounce lúc map. **IN** clean khi map, invalidate khi unmap, dữ liệu thật được chép từ bounce về lúc unmap. (Chi tiết và lý do ở Chương 2.)
6. Điểm hoàn tất của mọi lần truyền là bước **khớp Event với TD**. Lỗi trong OP_BTS-71095 nằm đúng ở bước này.

## Câu hỏi kiểm tra

1. Vì sao `mv61x0-vdma` không tham gia vào việc truyền USB?
2. Một buffer `kmalloc` có physical `0x1_2000_0000`. Trên Quartz, DMA address ghi vào TRB là gì? Và lúc nào dữ liệu được chép, với OUT và với IN?
3. Với Bulk IN, CPU không đưa dữ liệu nào cho thiết bị. Vậy vì sao lúc map vẫn phải clean cache, và vì sao lúc unmap bắt buộc phải invalidate?
4. Dòng `iommu: Default domain type: Translated` có chứng tỏ xHCI đang dùng IOMMU không? Kiểm tra điều đó bằng cách nào?
5. Trong Bulk IN, nếu thiết bị không bao giờ gửi dữ liệu thì controller làm gì, và CPU có biết không?
6. Vì sao lật **cycle bit sau cùng** khi viết TRB lại quan trọng?
7. `usbfax` dùng buffer coherent kèm `URB_NO_TRANSFER_DMA_MAP`. Bước nào trong luồng OUT bị bỏ qua, và cái giá phải trả là gì?
8. Log `reset high-speed USB device number …` khác `new high-speed …` ở điểm nào, và nó liên quan gì đến hàm `xhci_resume` (xem `docs/diagrams/xhci-resume-sequence.md`)?
9. Vì sao các log probe chuẩn của xHCI không thấy trên console của board, và làm sao để thấy chúng?

---

**Chương tiếp theo:** Chương 2 đi sâu từng khái niệm nền tảng của DMA: mapping và unmapping, coherent và streaming, direction, mask, IOVA, mỗi khái niệm có sơ đồ cơ chế và ví dụ USB.
