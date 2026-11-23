# Chương 11: Công cụ debug USB DMA

> Khoá học USB DMA trên Linux Kernel. Baseline: **Linux 5.10.258**. Tình trạng sẵn có trên Quartz được kiểm tra theo `arch/arm64/configs/km_mvebu_v8_lsp_defconfig`. Đường dẫn debugfs và tên tracepoint đã đối chiếu với source.

## 11.0 Công cụ nào dùng được ngay trên Quartz

| Công cụ | Cần gì | Quartz hiện tại |
|---|---|---|
| `dmesg` | luôn có | ✅ |
| `journalctl -k` | systemd-journald | ❓ tùy userspace |
| debugfs USB / xHCI | `CONFIG_DEBUG_FS` | ✅ (`=y`) |
| `lsusb` | usbutils | ✅ (FaxP gọi `system("lsusb")`) |
| `lspci` | pciutils + PCI | ⚠ xHCI là thiết bị platform, không phải PCI |
| usbmon | `CONFIG_USB_MON` | ❌ (không bật) |
| ftrace / trace-cmd | `CONFIG_FTRACE` | ❌ (không bật) |
| dynamic debug | `CONFIG_DYNAMIC_DEBUG` | ❌ (không bật) |
| DMA API debug | `CONFIG_DMA_API_DEBUG` | ❌ (không bật) |
| IOMMU fault reporting | driver SMMU | ❌ (không có SMMU) |
| KASAN | `CONFIG_KASAN` | ❌ (không bật) |

**Khuyến nghị:** chuẩn bị sẵn một **bản build debug** bật `USB_MON`, `FTRACE` (+ `TRACING`), `DYNAMIC_DEBUG`, `DMA_API_DEBUG`, và nếu chịu được tốc độ thì cả `KASAN`. Dùng nó để tái hiện các ticket USB khó.

---

## 11.1 dmesg

| | |
|---|---|
| Cung cấp | Bộ đệm log kernel, kể cả mức thấp bị console ẩn |
| Dùng khi | Luôn là bước đầu |
| Lệnh | `dmesg -T` (giờ thực), `dmesg -l err,warn`, `dmesg -w` (theo dõi), `dmesg -n 8` (cho mọi mức ra console) |
| Đọc kết quả | Tìm **log bất thường đầu tiên**; ghép thời gian giữa log kernel `[giây.µs]` và log ứng dụng |

```bash
dmesg -T | grep -iE "xhci|ehci|usb [0-9]|swiotlb|DMA|HC died|Host System Error"
```

Trên Quartz, log console được ghi kèm giờ của PC thu log và tick của ứng dụng (`(30311533)`). Quy đổi: tick ≈ 200/s, kernel time tính bằng giây (Chương 1).

## 11.2 journalctl -k

| | |
|---|---|
| Cung cấp | Log kernel lưu bền qua các lần khởi động (nếu journald có lưu) |
| Dùng khi | Lỗi đã gây reboot và cần xem log của lần chạy trước |
| Lệnh | `journalctl -k -b -1` (lần boot trước), `journalctl -k --since "10 min ago"` |
| Đọc kết quả | Như dmesg. Lưu ý có thể mất các dòng cuối cùng trước một reboot đột ngột |

## 11.3 usbmon (cần build lại)

| | |
|---|---|
| Cung cấp | Mọi URB đi qua USB core: lúc submit (S), lúc hoàn tất (C), lỗi (E); địa chỉ, endpoint, trạng thái, độ dài, dữ liệu đầu |
| Dùng khi | Muốn biết **driver đã gửi gì, controller đã trả gì**, ở mức URB (không thấy TRB) |
| Vị trí | `/sys/kernel/debug/usb/usbmon/<bus>u` (định dạng chi tiết), `<bus>t` (định dạng cũ), `0u` (mọi bus) |
| Lệnh | `cat /sys/kernel/debug/usb/usbmon/3u > /tmp/bus3.txt` |

Dòng mẫu **[minh họa định dạng]**:
```
ffffff80c1a2b000 3562913458 S Bo:3:093:2 -115 16384 = 1b2a0000 …
ffffff80c1a2b000 3562916512 C Bo:3:093:2 0 16384 >
```
`S`/`C` là submit/complete. `Bo:3:093:2` = Bulk OUT, bus 3, thiết bị 93, endpoint 2. `-115` (`-EINPROGRESS`) là trạng thái lúc submit. `0` là thành công lúc hoàn tất.

**Đọc kết quả:** một `S` không bao giờ có `C` tương ứng là URB treo. `C` với trạng thái âm cho biết mã lỗi (bảng ở Chương 8, mục C).

## 11.4 ftrace và trace-cmd (cần build lại)

| | |
|---|---|
| Cung cấp | Tracepoint của xHCI (`TRACE_SYSTEM xhci-hcd`), ghi **từng TRB, từng Event, từng lệnh, từng lần bấm doorbell**, kèm DMA address |
| Dùng khi | Lỗi đồng bộ ring (OP_BTS-71095), hủy URB, enumeration |
| Tracepoint hữu ích (đã đối chiếu `xhci-trace.h`) | `xhci_urb_enqueue`, `xhci_urb_giveback`, `xhci_urb_dequeue`, `xhci_queue_trb`, `xhci_handle_event`, `xhci_handle_transfer`, `xhci_handle_command`, `xhci_ring_ep_doorbell`, `xhci_inc_enq`, `xhci_inc_deq`, `xhci_handle_cmd_stop_ep`, `xhci_handle_cmd_set_deq`, `xhci_handle_cmd_set_deq_ep`, `xhci_ring_alloc`, `xhci_ring_expansion`, `xhci_handle_port_status` |

```bash
echo 1 > /sys/kernel/debug/tracing/events/xhci-hcd/enable
```
```bash
cat /sys/kernel/debug/tracing/trace_pipe > /tmp/xhci-trace.txt
```
```bash
trace-cmd record -e xhci-hcd -e dma -o /tmp/xhci.dat
```

**Đọc kết quả:** đi theo một URB:
1. `xhci_urb_enqueue` → các `xhci_queue_trb` (địa chỉ TRB) → `xhci_ring_ep_doorbell`;
2. `xhci_handle_event`/`xhci_handle_transfer` (TRB pointer trong Event) → `xhci_urb_giveback`.

Với OP_BTS-71095: so TRB pointer trong `xhci_handle_transfer` với các địa chỉ trong `xhci_queue_trb` trước đó.

**Lưu ý:** trace ghi rất nhiều. Hãy dùng bộ đệm vòng (`buffer_size_kb`) và dừng ngay khi thấy lỗi.

## 11.5 Dynamic debug (cần build lại)

| | |
|---|---|
| Cung cấp | Bật các dòng `dev_dbg`/`xhci_dbg`/`ehci_dbg` theo file, hàm, dòng |
| Dùng khi | Muốn thấy log chi tiết (Stall, Transaction Error, `Command timeout`, `Cancel URB`, `Enabling 32-bit DMA addresses.`) |

```bash
echo 'module xhci_hcd +p' > /sys/kernel/debug/dynamic_debug/control
```
```bash
echo 'file drivers/usb/host/xhci-ring.c +p' > /sys/kernel/debug/dynamic_debug/control
```
```bash
modprobe xhci_hcd dyndbg=+p
```

Lệnh cuối dùng để thấy log lúc probe. Trên Quartz, xHCI là module nên cách này khả thi sau khi build lại.

## 11.6 DMA API debug (cần build lại)

| | |
|---|---|
| Cung cấp | Theo dõi mọi mapping, báo lỗi dùng sai API (Chương 2, 6, 8) |
| Dùng khi | Nghi rò mapping, unmap sai, sai direction, trùng dòng cache, quên kiểm tra lỗi map |
| Cấu hình | `CONFIG_DMA_API_DEBUG=y`; tham số boot `dma_debug_driver=<tên driver>` để lọc |
| Vị trí (đã đối chiếu `kernel/dma/debug.c`) | `/sys/kernel/debug/dma-api/`: `disabled`, `error_count`, `all_errors`, `num_errors`, `num_free_entries`, `min_free_entries`, `nr_total_entries`, `driver_filter`, `dump` |

```bash
echo xhci_hcd > /sys/kernel/debug/dma-api/driver_filter
```
```bash
cat /sys/kernel/debug/dma-api/dump
```

**Đọc kết quả:** `num_free_entries` giảm dần không hồi phục nghĩa là rò mapping. `dump` liệt kê các mapping còn sống, gồm thiết bị, địa chỉ, kích thước, direction.

## 11.7 IOMMU fault reporting (SoC có SMMU)

| | |
|---|---|
| Cung cấp | Log fault kèm IOVA, loại fault, context bank/stream |
| Dùng khi | Controller halt (HSE) trên SoC có SMMU |
| Lệnh | `dmesg | grep -iE "smmu|iommu|context fault"`; `ls /sys/kernel/iommu_groups/*/devices/` để biết thiết bị nào sau IOMMU |
| Đọc kết quả | `iova` → tra mapping vừa unmap; `fsr` → loại fault |
| Quartz | Không áp dụng. `ls /sys/kernel/iommu_groups/` sẽ rỗng (cần xác nhận trên máy) |

## 11.8 debugfs USB và xHCI (dùng được ngay trên Quartz)

```
 /sys/kernel/debug/usb/
 ├── devices                          ← danh sách thiết bị, cấu hình, endpoint (USB core)
 ├── usbmon/                          ← chỉ có khi bật CONFIG_USB_MON
 └── xhci/
     └── 8d00d0000.usb3/              ← tên thiết bị platform của controller
         ├── reg-cap  reg-op  reg-runtime  reg-ext-legsup  reg-ext-protocol…
         ├── command-ring/  { enqueue  dequeue  cycle  trbs }
         ├── event-ring/    { enqueue  dequeue  cycle  trbs }
         ├── ports/portNN/portsc
         └── devices/NN/                ← NN = slot ID
             ├── slot-context
             ├── ep-context
             └── epXX/      { enqueue  dequeue  cycle  trbs }   ← XX = ep_index
```

| File | Cho biết | Dùng cho |
|---|---|---|
| `reg-op` | USBCMD, USBSTS, CRCR, DCBAAP… | controller có Run không, có HSE/HCE/HCH không |
| `reg-runtime` | IMAN, IMOD, ERSTSZ, ERSTBA, ERDP | ngắt có bật không, ERDP có tiến không |
| `event-ring/trbs` | toàn bộ Event (loại, TRB pointer, completion code, cycle) | thấy Event mà phần mềm đã bỏ |
| `devices/NN/ep-context` | trạng thái endpoint, TR Dequeue Pointer + DCS | Running/Stopped/Halted, phần cứng đang ở đâu |
| `devices/NN/epXX/trbs` | các TRB trên Transfer Ring của endpoint, kèm DMA address | so với TRB pointer trong Event |
| `devices/NN/epXX/enqueue`, `dequeue` | vị trí phần mềm đang ghi và đang chờ | dequeue có tiến không |
| `ports/portNN/portsc` | trạng thái cổng (kết nối, enable, link state) | cổng còn ở U0 không, có lỗi không |

**Quy trình chụp nhanh khi xảy ra lỗi** (ví dụ OP_BTS-71095, EP1 OUT có `ep_index 1` ⇒ thư mục `ep01`):

```bash
D=/sys/kernel/debug/usb/xhci/8d00d0000.usb3; for f in reg-op reg-runtime event-ring/trbs event-ring/dequeue; do echo "== $f"; cat $D/$f; done > /tmp/xhci-snap.txt
```
```bash
for s in /sys/kernel/debug/usb/xhci/8d00d0000.usb3/devices/*; do echo "== $s"; cat $s/ep-context; cat $s/ep01/trbs $s/ep01/dequeue 2>/dev/null; done >> /tmp/xhci-snap.txt
```

Lưu ý: số slot của bo FAX thay đổi sau mỗi lần enumerate, nên lệnh thứ hai duyệt mọi slot.

## 11.9 lsusb

| | |
|---|---|
| Cung cấp | Thiết bị trên từng bus, cây kết nối, descriptor |
| Lệnh | `lsusb` (danh sách), `lsusb -t` (cây cổng và driver), `lsusb -v -d 132b:20b7` (descriptor đầy đủ của FK-513) |
| Đọc kết quả | `Device 093` tăng mỗi lần enumerate, nên số lớn là dấu hiệu thiết bị đã bị gỡ/gắn nhiều lần. `-t` xác nhận vị trí cổng (`3-1.2`) để đối chiếu với các script reset |

## 11.10 lspci

| | |
|---|---|
| Cung cấp | Thiết bị PCI, BAR, khả năng (MSI), trạng thái Bus Master |
| Dùng khi | Controller USB nằm trên PCI (đa số PC, một số SoC) |
| Lệnh | `lspci -nnk` (driver đang dùng), `lspci -vvv -s <bdf>` (BusMaster+, MSI, lỗi AER) |
| Quartz | xHCI là thiết bị platform, nên `lspci` không thấy. Nó chỉ thấy các thiết bị PCI khác như NVMe |

---

## 11.11 Chọn công cụ theo triệu chứng

| Triệu chứng | Công cụ đầu tiên | Công cụ tiếp theo |
|---|---|---|
| URB treo | debugfs `ep-context`, `epXX/dequeue` | usbmon, trace `xhci_handle_transfer` |
| `not part of current TD` | debugfs `epXX/trbs` + `event-ring/trbs` | trace `xhci_queue_trb`, `xhci_handle_transfer` |
| `swiotlb buffer is full` | dmesg (theo dõi `used`) | DMA API debug `num_free_entries`, `dump` |
| Dữ liệu hỏng | mẫu dữ liệu, kiểm tra căn lề | DMA API debug, `swiotlb=force` |
| Controller chết | dmesg (log đầu tiên), `reg-op` (USBSTS) | trace toàn bộ `xhci-hcd`, KASAN |
| Lỗi enumerate | dmesg, `lsusb -t`, `portsc` | usbmon EP0, dynamic debug `hub.c` |
| Hiệu năng thấp | `/sys/block/sdX/queue/max_hw_sectors_kb`, `top` | trace để đếm kích thước URB |

## 11.12 Tóm tắt Chương 11

1. Trên Quartz hiện tại chỉ có **dmesg, debugfs, lsusb**. Debugfs của xHCI đủ để chụp trạng thái ring và context ngay khi lỗi.
2. **usbmon** cho thấy URB, **ftrace** cho thấy TRB và Event, **DMA API debug** cho thấy lỗi dùng API, **SMMU** cho thấy địa chỉ sai. Cả bốn đều cần build lại (hoặc phần cứng khác).
3. Chuẩn bị sẵn script chụp debugfs và một bản build debug giúp rút ngắn thời gian điều tra các ticket hiếm gặp.

## Câu hỏi kiểm tra

1. Với OP_BTS-71095, viết chính xác các file debugfs bạn sẽ chụp và giải thích mỗi file trả lời câu hỏi nào.
2. Vì sao usbmon **không** thấy được lỗi "TRB pointer không thuộc TD"? Công cụ nào thấy được?
3. Trên một bản build có DMA API debug, bạn theo dõi chỉ số nào để phát hiện rò mapping?
4. `lsusb` cho thấy FK-513 là `Device 093`. Điều đó nói gì về lịch sử của thiết bị kể từ lúc boot?
5. Liệt kê các tùy chọn cấu hình bạn sẽ bật trong bản build debug cho Quartz, và chi phí (hiệu năng, bộ nhớ) của từng tùy chọn.

---

**Chương tiếp theo:** Chương 12 so sánh kiến trúc DMA của EHCI và xHCI.
