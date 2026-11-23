# Chương 8: Phân tích log kernel USB DMA

> Khoá học USB DMA trên Linux Kernel. Baseline: **Linux 5.10.258**. Đây là chương tra cứu. Mỗi log có **câu chữ đã kiểm tra trong source** (ghi rõ file:dòng), trừ khi được đánh dấu **[minh họa]**. Ký hiệu ★ = đã thấy trong log thật của board Quartz.

## 8.0 Cách đọc chương này

### Mức độ

| Ký hiệu | Mức | Ý nghĩa |
|---|---|---|
| 🟢 | bình thường | thông tin, xuất hiện ở mọi hệ thống khỏe |
| 🟡 | cảnh báo | có vấn đề nhưng hệ thống có thể tự phục hồi, hoặc chỉ là quirk |
| 🔴 | lỗi | một lần truyền hoặc một thiết bị bị ảnh hưởng |
| ⛔ | nghiêm trọng | controller dừng hoặc chết, mọi thiết bị trên nó bị ảnh hưởng |

### Thẻ log (mẫu dùng cho các log quan trọng)

| Mục | Câu hỏi |
|---|---|
| Nghĩa | Log đang thông báo điều gì? |
| Nguồn | Thành phần nào in ra (file:dòng)? |
| Điều kiện | Điều kiện nào khiến log xuất hiện? |
| Trước | Chuyện gì vừa xảy ra ngay trước đó? |
| Sau | Chuyện gì xảy ra tiếp theo? |
| Cơ chế | Cơ chế phần mềm/phần cứng bên dưới |
| Nguyên nhân | Các nguyên nhân có thể |
| Bình thường hay lỗi | Phân biệt thế nào |
| Kiểm tra tiếp | Bước điều tra tiếp theo |

### Lưu ý về khả năng thấy log trên Quartz

- Log `dev_info`/`pr_info` có thể bị loglevel của console chặn. Xem bằng `dmesg`.
- Log `xhci_dbg`/`dev_dbg`/`ehci_dbg` **không thể in ra** trên bản build hiện tại (`CONFIG_DYNAMIC_DEBUG` tắt).
- Log của DMA API debug, SMMU, KASAN **không có** trên cấu hình hiện tại.

---

## A. Log khởi tạo DMA và controller (bình thường)

| Log | Nguồn | Mức | Quartz |
|---|---|---|---|
| `Zone ranges:` / `DMA [mem …]` / `DMA32 empty` / `Normal [mem …]` | mm, khởi tạo zone | 🟢 | ★ |
| `software IO TLB: mapped [mem %pa-%pa] (%luMB)` | `kernel/dma/swiotlb.c:172` | 🟢 | ★ |
| `DMA: preallocated %zu KiB %pGg pool for atomic allocations` | `kernel/dma/pool.c:185` | 🟢 | ★ |
| `iommu: Default domain type: %s %s` | `drivers/iommu/iommu.c:145` | 🟢 | ★ |
| `<product_desc>` (ví dụ `xHCI Host Controller`) | `core/hcd.c:2695` | 🟢 | |
| `new USB bus registered, assigned bus number %d` | `core/hcd.c:937` | 🟢 | |
| `irq %d, %s 0x%08llx` (`io mem`/`io port`) | `core/hcd.c:2626` | 🟢 | |
| `hcc params 0x%08x hci version 0x%x quirks 0x%016llx` | `host/xhci.c:5390` | 🟢 | |
| `Enabling 64-bit DMA addresses.` / `Enabling 32-bit DMA addresses.` | `host/xhci.c:5369/5379` (debug) | 🟢 | không in được |
| `Host supports USB 3.%x %sSuperSpeed` | `host/xhci.c:5303` | 🟢 | |
| `USB %x.%x started, EHCI %x.%02x%s` | `host/ehci-hcd.c:659` | 🟢 | |
| `USB hub found` / `%d port%s detected` | `core/hub.c:1943` / `1507` | 🟢 | |
| `%s %s USB device number %d using %s` | `core/hub.c:4994` | 🟢 (`new`) / 🟡 (`reset`) | |
| `OP_BTS-69673: usb fix version = V10 …` | patch KM (không có ở upstream) | 🟢 | ★ |

### Thẻ: `hcc params 0x… hci version 0x… quirks 0x…`

| Mục | Nội dung |
|---|---|
| Nghĩa | Controller đã được khởi tạo: mask DMA đã chọn, DCBAA, Command Ring, Event Ring, ERST, scratchpad đã cấp và ghi vào thanh ghi |
| Nguồn | `host/xhci.c:5390`, cuối hàm setup chung |
| Điều kiện | Mỗi lần probe controller |
| Trước | Đọc HCCPARAMS, áp quirk (KM xóa AC64), đặt mask, cấp bộ nhớ |
| Sau | Đăng ký root hub (`new USB bus registered …`), bật Run |
| Cơ chế | `hcc params` in ra là giá trị **sau khi quirk đã sửa**. Trên Quartz, bit 0 (AC64) đã bị xóa |
| Bình thường hay lỗi | Bình thường. Thiếu dòng này sau `irq …, io mem …` nghĩa là khởi tạo bộ nhớ hoặc mask thất bại |
| Kiểm tra tiếp | Đọc bit AC64 trong `hcc params` để biết mask thực tế; đối chiếu `quirks` với danh sách quirk trong `xhci.h` |

### Thẻ: `%s %s USB device number %d using %s`

| Mục | Nội dung |
|---|---|
| Nghĩa | `new …`: thiết bị mới được gán địa chỉ. `reset …`: thiết bị đã có được reset và gán lại địa chỉ |
| Nguồn | `core/hub.c:4994` |
| Trước | Lệnh Enable Slot và Address Device qua Command Ring thành công: controller đã **DMA đọc Input Context** và **DMA ghi Output Context** |
| Sau | Đọc descriptor (control transfer qua EP0 ring) |
| Bình thường hay lỗi | `new` là bình thường. `reset` liên tục là dấu hiệu driver lớp trên (usb-storage…) phải reset vì truyền thất bại, hoặc là reset-resume sau system resume |
| Kiểm tra tiếp | Log ngay trước `reset`: timeout SCSI, lỗi transfer, resume |

---

## B. Log về DMA mapping

| Log | Nguồn | Mức |
|---|---|---|
| `swiotlb buffer is full (sz: %zd bytes), total %lu (slots), used %lu (slots)` | `kernel/dma/swiotlb.c:589` | 🔴 |
| `swiotlb addr %pad+%zu overflow (mask %llx, bus limit %llx).` | `kernel/dma/swiotlb.c:725` | 🔴 |
| `DMA addr %pad+%zu overflow (mask %llx, bus limit %llx).` (kèm WARNING) | `kernel/dma/direct.h` | 🔴 |
| `transfer buffer is on stack` (WARN_ONCE) | `core/hcd.c:1501` | 🔴 |
| `setup packet is on stack` (WARN_ONCE) | `core/hcd.c:1436` | 🔴 |
| WARNING tại `dma_alloc_attrs` (thiếu coherent mask) | `kernel/dma/mapping.c:427` | 🔴 |
| `Could not allocate xHCI USB device data structures` | `host/xhci.c:4113` | 🔴 |
| `Ring expansion failed` | `host/xhci-ring.c:3193` | 🔴 |
| DMA API debug: `… failed to check map error`, `… has not allocated …`, `… wrong function …`, `… different direction …` | `kernel/dma/debug.c` | 🔴 (cần build lại) |

### Thẻ: `swiotlb buffer is full …`

| Mục | Nội dung |
|---|---|
| Nghĩa | Không đủ ô bounce **liên tiếp** cho một lần map |
| Nguồn | `kernel/dma/swiotlb.c:589` (ratelimited) |
| Điều kiện | Map một buffer vượt mask (trên Quartz: buffer ở trên 4 GiB) khi kho không còn đủ ô |
| Trước | Driver submit URB → USB core map → SWIOTLB tìm ô |
| Sau | Map trả lỗi → `usb_submit_urb` trả `-EAGAIN` → driver xử lý lỗi |
| Cơ chế | Ô 2 KiB; tối đa 128 ô (256 KiB) cho một lần map; kho 64 MiB dùng chung |
| Nguyên nhân | **Rò mapping** (URB không được trả về, ví dụ do Event bị bỏ); tải đồng thời lớn; phân mảnh; map khối > 256 KiB |
| Bình thường hay lỗi | Luôn là lỗi. `used` tăng dần theo thời gian thì gần như chắc chắn là rò |
| Kiểm tra tiếp | Thiết bị nào in log; có URB treo không; thử `swiotlb=<lớn hơn>` chỉ để phân biệt thiếu kho và rò |

### Thẻ: `transfer buffer is on stack`

| Mục | Nội dung |
|---|---|
| Nghĩa | Driver USB dùng biến cục bộ làm buffer cho URB |
| Điều kiện | Kernel có `CONFIG_VMAP_STACK=y` (Quartz có): stack nằm trong vmalloc, không map được |
| Sau | Chỉ in **lần đầu**. Mọi lần sau âm thầm trả `-EAGAIN` |
| Nguyên nhân | `usb_control_msg`/`usb_bulk_msg` với mảng cục bộ |
| Kiểm tra tiếp | Stack trace kèm theo chỉ ra driver. Sửa bằng `kmalloc` |

### Thẻ: `Ring expansion failed`

| Mục | Nội dung |
|---|---|
| Nghĩa | Transfer Ring hết chỗ, không cấp được segment mới |
| Cơ chế | Cấp segment coherent trong ngữ cảnh atomic, lấy từ atomic pool 512 KiB |
| Nguyên nhân | TD không bao giờ hoàn tất (Event bị bỏ, endpoint treo) nên ring không được giải phóng; driver submit dồn dập; hết bộ nhớ |
| Kiểm tra tiếp | debugfs `…/devices/NN/epXX/enqueue` và `dequeue`: dequeue có đứng yên không? |

---

## C. Log trên đường truyền USB

| Log | Nguồn | Mức | Quartz |
|---|---|---|---|
| `URB %pK submitted while active` (WARN_ONCE) | `core/urb.c:378` | 🔴 | ★ |
| `BOGUS urb xfer, pipe %x != type %x` | `core/urb.c:493` | 🟡 | |
| `BOGUS urb flags, %x --> %x` | `core/urb.c:517` | 🔴 | |
| `WARN urb submitted to disabled ep` | `host/xhci-ring.c:3158` | 🔴 | |
| `WARN waiting for error on ep to be cleared` | `host/xhci-ring.c:3161` | 🟡 | |
| `WARN Event TRB for slot %d ep %d with no TDs queued?` | `host/xhci-ring.c:2682` | 🔴 | |
| `ERROR Transfer event TRB DMA ptr not part of current TD ep_index %d comp_code %u` | `host/xhci-ring.c:2788` | ⛔ | ★ |
| `WARN Successful completion on short TX` | `host/xhci-ring.c:2391` | 🟡 | |
| `WARN Successful completion on short TX for slot %u ep %u: needs XHCI_TRUST_TX_LENGTH quirk?` | `host/xhci-ring.c:2542` | 🟡 | |
| `URB req %u and actual %u transfer length mismatch` | `host/xhci-ring.c:805` | 🔴 | |
| `WARN Set TR Deq Ptr cmd failed due to incorrect slot or ep state.` | `host/xhci-ring.c:1281` | 🔴 | |
| `Canceled URB td not found on endpoint ring` | `host/xhci.c:1708` | 🔴 | |
| `can't set config #%d, error %d` | `core/hub.c:2792` | 🔴 | ★ (`3-1.2 … error -71`) |
| `device descriptor read/64, error %d` / `read/8` / `read/all` | `core/hub.c:5074/5139/5202` | 🔴 | |
| `device not accepting address %d, error %d` | `core/hub.c:5095` | 🔴 | |
| `unable to enumerate USB device` | `core/hub.c:5634` | 🔴 | |
| `Timeout while waiting for setup device command` | `host/xhci.c:4259` | 🔴 | |
| `Setup ERROR: setup %s command for slot %d.` | `host/xhci.c:4264` | 🔴 | |

### Ý nghĩa mã lỗi URB thường gặp (đã đối chiếu xHCI/EHCI)

| Mã | Ý nghĩa | DMA liên quan thế nào |
|---|---|---|
| `0` | thành công | Event SUCCESS (hoặc SHORT_PACKET với URB chấp nhận gói ngắn) |
| `-ENOENT` | URB bị `usb_kill_urb` | đã đi hết luồng Stop Endpoint/IAA, buffer đã unmap |
| `-ECONNRESET` | URB bị `usb_unlink_urb` (bất đồng bộ) | như trên |
| `-ESHUTDOWN` | controller chết hoặc thiết bị bị gỡ | URB bị trả hàng loạt |
| `-EPIPE` | STALL | endpoint Halted, cần CLEAR_FEATURE |
| `-EPROTO` | lỗi giao dịch (CRC, timeout, 3 lần thử) | không phải DMA, là đường truyền/thiết bị |
| `-EOVERFLOW` | babble | thiết bị gửi quá dài |
| `-ENOSR` / `-ECOMM` | Data Buffer Error (EHCI IN/OUT) | **controller không theo kịp RAM** |
| `-EREMOTEIO` | gói ngắn khi URB không chấp nhận gói ngắn | |
| `-ETIMEDOUT` | do API đồng bộ (`usb_bulk_msg`) trả khi hết thời gian | URB đã bị kill bên trong |
| `-EAGAIN` (khi submit) | map DMA thất bại, hoặc buffer trên stack | map không thành |
| `-EBUSY` (khi submit) | URB đang active | `submitted while active` |

### Thẻ: `URB … submitted while active` ★

| Mục | Nội dung |
|---|---|
| Nghĩa | Submit lại một URB chưa được giveback |
| Nguồn | `core/urb.c:378`, kèm stack trace |
| Trước | Một lần submit trước không hoàn tất, và driver không hủy nó |
| Sau | `-EBUSY`. Các lần sau không in nữa (WARN_ONCE) |
| Nguyên nhân | Driver không `usb_kill_urb` khi timeout; HCD bỏ Event nên không giveback |
| Kiểm tra tiếp | Stack trace → driver. Tìm log xHCI ngay trước đó (Chương 3, mục 3.7) |

### Thẻ: `ERROR Transfer event TRB DMA ptr not part of current TD …` ★

| Mục | Nội dung |
|---|---|
| Nghĩa | Event báo hoàn tất một TRB nằm ngoài TD driver đang chờ |
| Nguồn | `host/xhci-ring.c:2788` |
| Trước | Controller ghi Transfer Event. Trạng thái ring giữa phần cứng và phần mềm đã lệch từ trước |
| Sau | **Event bị bỏ** → URB không giveback → không unmap → timeout ở driver → dây chuyền `submitted while active`, có thể `swiotlb buffer is full` |
| Cơ chế | Chương 5, mục 5.6 và 5.14 |
| Nguyên nhân | Hủy URB sai cách, khôi phục endpoint sai, trạng thái controller sau resume, lỗi phần cứng. Trên Quartz loại trừ được "Event cũ do cache" (Chương 6) |
| Kiểm tra tiếp | Bật dump Event (đoạn `#if 0` ngay sau dòng log); debugfs ring; trace `xhci-hcd` (cần build lại với FTRACE) |

### Thẻ: `device descriptor read/64, error -71`

| Mục | Nội dung |
|---|---|
| Nghĩa | Đọc 64 byte đầu của device descriptor qua EP0 thất bại; `-71` = `-EPROTO` |
| Trước | Thiết bị vừa được cấp địa chỉ (Address Device thành công) |
| Sau | Hub driver thử lại, có thể reset cổng, cuối cùng có thể `unable to enumerate USB device` |
| Cơ chế | Control transfer: TRB Setup (dữ liệu trong TRB, IDT) + TRB Data IN (DMA ghi vào buffer) + TRB Status |
| Nguyên nhân | Thường là **đường truyền/thiết bị** (cáp, nguồn, tín hiệu, thiết bị chưa sẵn sàng sau reset), hiếm khi là DMA |
| Bình thường hay lỗi | Một lần khi cắm có thể chấp nhận. Lặp lại là lỗi phần cứng hoặc nguồn |
| Kiểm tra tiếp | Thử cáp/cổng khác, đo nguồn VBUS. Trên Quartz: `can't set config … error -71` lúc PortReset trong OP_BTS-71095 cùng họ lỗi `-EPROTO` |

---

## D. Log riêng của xHCI

| Log | Nguồn | Mức |
|---|---|---|
| `WARNING: Host System Error` | `xhci-ring.c:3052` | ⛔ |
| `xHCI host controller not responding, assume dead` | `xhci-ring.c:1115` | ⛔ |
| `HC died; cleaning up` | `core/hcd.c:2396` | ⛔ |
| `xHCI host not responding to stop endpoint command.` + `USBSTS:%s` | `xhci-ring.c:1169-1170` | ⛔ |
| `Abort failed to stop command ring: %d` | `xhci-ring.c:380` | ⛔ |
| `ERR: No room for command on command ring` | `xhci-ring.c:4389` | 🔴 |
| `Do not support expand command ring` | `xhci-ring.c:3184` | 🔴 |
| `ERROR event ring not ready` | `xhci-ring.c:2926` | 🔴 |
| `WARN something wrong with SW event ring dequeue ptr` | `xhci-ring.c:3005` | 🔴 |
| `ERROR unknown event type %d` | `xhci-ring.c:2965` | 🔴 |
| `ERROR Unknown event condition %u for slot %u ep %u , HC probably busted` | `xhci-ring.c` (nhánh `default` của completion code) | ⛔ |
| `WARN: TRB error for slot %u ep %u on endpoint` | `xhci-ring.c` (completion code TRB Error) | 🔴 |
| `WARN: HC couldn't access mem fast enough for slot %u ep %u` | `xhci-ring.c` (Data Buffer Error) | 🔴 |
| `WARN: bandwidth overrun event for slot %u ep %u on endpoint` | `xhci-ring.c` | 🟡 |
| `Host not accessible, reset failed.` | `xhci.c:180` | ⛔ |
| `Host controller not halted, aborting reset.` | `xhci.c:185` | 🔴 |
| `Host halt failed, %d` | `xhci.c:122` | 🔴 |
| `Host took too long to start, …` | `xhci.c:151` | ⛔ |
| `Controller not ready at resume %d` | `xhci.c:1152` | ⛔ |
| `WARN: xHC restore state timeout` / `WARN: xHC save state timeout` | `xhci.c:1173` / `1078` | 🔴 |
| `xHC error in resume, USBSTS 0x%x, Reinit` | `xhci.c:1185` | 🟡 (tự khởi tạo lại) |
| `Timeout while waiting for configure endpoint command` | `xhci.c:2027` | 🔴 |
| `Error while assigning device slot ID` | `xhci.c:4086` | 🔴 |
| `Not enough bandwidth …` | `xhci.c:2514`, `2844` | 🔴 |

**Ghi chú về Event Ring Full:** completion code *Event Ring Full Error* (21) có trong bảng tên (`xhci.h`), nhưng 5.10.258 **không có nhánh xử lý riêng cho Host Controller Event**, nên Event loại này rơi vào `ERROR unknown event type %d`. Phòng ngừa bằng cách cập nhật ERDP mỗi 128 Event (Chương 5).

### Thẻ: `WARNING: Host System Error`

| Mục | Nội dung |
|---|---|
| Nghĩa | USBSTS.HSE = 1: giao dịch bộ nhớ của controller thất bại; controller tự Halt |
| Trước | Controller DMA theo một địa chỉ trong TRB/context/thanh ghi |
| Sau | Driver halt, ngừng xử lý Event → URB treo → hủy URB → Stop Endpoint không phản hồi → `assume dead` → `HC died; cleaning up` |
| Nguyên nhân | DMA address hỏng; dùng sau khi giải phóng ring/context/buffer; IOMMU fault (SoC có SMMU); cấu hình sai sau resume |
| Kiểm tra tiếp | Chương 9 có sơ đồ lần ngược |

### Thẻ: `xHCI host not responding to stop endpoint command.` + `USBSTS:…`

| Mục | Nội dung |
|---|---|
| Nghĩa | Lệnh Stop Endpoint (để hủy URB) không có phản hồi trong 5 giây |
| Sau | `assume dead` → mọi URB của controller bị trả về → thiết bị biến mất |
| Đọc USBSTS | `HCHalted` → controller đã dừng (thường do HSE trước đó); `HSE` → lỗi bộ nhớ; `HCE` → lỗi nội bộ controller; không bit nào lạ → ngắt không tới CPU hoặc Command Ring kẹt |
| Kiểm tra tiếp | Log ngay trước (HSE? resume?); `/proc/interrupts` có tăng không |

### Thẻ: `xHC error in resume, USBSTS 0x…, Reinit`

| Mục | Nội dung |
|---|---|
| Nghĩa | Khôi phục trạng thái khi resume thất bại (SRE hoặc HCE) → chuyển sang khởi tạo lại |
| Bình thường hay lỗi | Cảnh báo: hệ thống tự cứu. Trên Quartz không thấy dòng này vì luôn khởi tạo lại (quirk reset-on-resume) |
| Hệ quả | Mọi thiết bị được reset-resume (`reset … USB device number …`) |

---

## E. Log riêng của EHCI

| Log | Nguồn | Mức | Liên quan DMA |
|---|---|---|---|
| `fatal error` | `ehci-hcd.c:844` | ⛔ | Host System Error: controller truy cập bộ nhớ thất bại |
| `HC died; cleaning up` | `core/hcd.c:2396` | ⛔ | hệ quả |
| `Waited too long for the controller to stop, giving up` | `ehci-timer.c:197` | 🔴 | controller chết không chịu Halt |
| `USB %x.%x, controller refused to start: %d` | `ehci-hcd.c:651` | ⛔ | |
| `qh %p should be inactive!` | `ehci-q.c:129` | 🔴 | overlay QH còn ACTIVE khi phần mềm nghĩ nó đã dừng: lệch đồng bộ phần mềm/phần cứng |
| `can't reschedule qh %p, err %d` | `ehci-sched.c:723` | 🔴 | thiếu băng thông periodic |
| `port %d reset error %d` / `port %d resume error %d` | `ehci-hub.c:1088/1068` | 🔴 | lỗi cổng |
| `devpath %s ep%d%s 3strikes` | `ehci-q.c:237` | debug | Transaction Error |
| `detected XactErr len %zu/%zu retry %d` | `ehci-q.c:376` | debug | phần mềm thử lại (tối đa 32) |
| `detected DataBufferErr for urb %p ep%d%s len %d, qtd %p [qh %p]` | `ehci-q.c:355` | debug | **controller không kịp RAM** |
| `IAA watchdog: status %x cmd %x` | `ehci-timer.c:354` | debug | lỡ ngắt IAA: gỡ QH có thể chậm |

Ánh xạ trạng thái qTD sang lỗi (Babble, Missed Micro-Frame, Data Buffer Error, Transaction Error, STALL) xem Chương 4, mục 4.6.

**Liên hệ với DMA:**
- *Host System Error* là lỗi địa chỉ hoặc bus.
- *Data Buffer Error* là lỗi độ trễ bộ nhớ.
- `qh … should be inactive!` là lỗi giao nhận quyền sở hữu descriptor.
- Babble và XactErr thuộc về đường truyền, không phải DMA.

---

## F. Log về IOMMU và bộ nhớ

| Log | Nguồn | Mức | Quartz |
|---|---|---|---|
| `Unhandled context fault: fsr=0x%x, iova=0x%08lx, fsynr=0x%x, cbfrsynra=0x%x, cb=%d` | `drivers/iommu/arm/arm-smmu/arm-smmu.c:440` | ⛔ | không có SMMU |
| `Blocked unknown Stream ID 0x%hx; boot with "arm-smmu.disable_bypass=0" to allow, …` | `arm-smmu.c:466` | 🔴 | không |
| `event 0x%02x received:` | `arm-smmu-v3/arm-smmu-v3.c:1378` | ⛔ | không |
| `swiotlb buffer is full …` | `kernel/dma/swiotlb.c:589` | 🔴 | có thể |
| `DMA addr … overflow …` / `swiotlb addr … overflow …` | `kernel/dma/direct.h`, `swiotlb.c:725` | 🔴 | ít khả năng |
| DMA API debug: `… driver tries to free DMA memory it has not allocated …` | `kernel/dma/debug.c:979` | 🔴 | cần build lại |
| DMA API debug: `… device driver failed to check map error …` | `kernel/dma/debug.c:1042` | 🔴 | cần build lại |
| DMA API debug: `device driver syncs DMA memory with different direction …` | `kernel/dma/debug.c:1138` | 🔴 | cần build lại |
| DMA API debug: `exceeded %d overlapping mappings of cacheline %pa` | `kernel/dma/debug.c:502` | 🟡 | cần build lại |
| `BUG: KASAN: use-after-free in …` **[minh họa định dạng]** | KASAN | ⛔ | KASAN chưa bật |
| `Unable to handle kernel paging request at virtual address …` **[minh họa định dạng]** | arm64 fault | ⛔ | có thể |

### Diễn giải nhóm F

| Dấu hiệu | Nói gì về truy cập bộ nhớ |
|---|---|
| IOMMU/SMMU fault | **Thiết bị** truy cập địa chỉ không được phép. Đây là dấu hiệu tốt nhất để biết DMA sai, nhưng **Quartz không có** |
| SWIOTLB full/overflow | Mask quá hẹp, kho cạn, hoặc rò mapping |
| DMA API debug | **Phần mềm** dùng sai API: thiếu kiểm tra lỗi, unmap sai, direction sai, trùng dòng cache |
| KASAN use-after-free | **CPU** truy cập vùng đã giải phóng. KASAN **không bắt được** khi thiết bị DMA vào vùng đã giải phóng |
| Paging request oops ở chỗ không liên quan USB | Có thể là hậu quả của DMA ghi đè (mask sai, dùng buffer sau khi giải phóng) trên SoC **không có IOMMU** |

---

## 8.7 Bảng phân loại nhanh: log nói về tầng nào

| Tầng | Log tiêu biểu |
|---|---|
| Driver lớp trên dùng sai API | `transfer buffer is on stack`, `submitted while active`, `BOGUS urb …`, DMA API debug |
| Ánh xạ DMA và giới hạn địa chỉ | `swiotlb …`, `DMA addr … overflow`, `Ring expansion failed` |
| Đồng bộ ring giữa phần mềm và phần cứng | `TRB DMA ptr not part of current TD`, `no TDs queued`, `qh … should be inactive!`, `Set TR Deq … failed` |
| Truy cập bộ nhớ của controller | `Host System Error`, `fatal error`, SMMU fault, Data Buffer Error |
| Controller không còn phản hồi | `not responding to stop endpoint`, `Abort failed to stop command ring`, `assume dead`, `HC died` |
| Đường truyền/thiết bị (không phải DMA) | `-EPROTO`, `-71`, `3strikes`, `device descriptor read`, babble |
| Suspend/resume | `Controller not ready at resume`, `restore state timeout`, `xHC error in resume …` |

## 8.8 Tóm tắt Chương 8

1. Phân biệt bốn mức (bình thường, cảnh báo, lỗi, nghiêm trọng) và **tầng** mà log thuộc về.
2. Nhóm A cho biết khởi tạo DMA đã xong tới đâu: thiếu dòng nào thì bước trước nó hỏng.
3. Nhóm B và F nói về địa chỉ và mapping. Nhóm C và D nói về đường đi của TRB/Event. Nhóm E là EHCI.
4. Trên Quartz, nhiều nhóm log **không thể xuất hiện** (debug, SMMU, KASAN, DMA API debug). Thiếu log không có nghĩa là không có lỗi.
5. Chuỗi nghiêm trọng điển hình: `Host System Error` → (URB treo) → `not responding to stop endpoint` → `assume dead` → `HC died; cleaning up`.

## Câu hỏi kiểm tra

1. `reset high-speed USB device number 5 using xhci-hcd` xuất hiện mỗi 30 giây. Bạn nghĩ tới những nguyên nhân nào và xem log nào trước?
2. Vì sao `-EPROTO` gần như không bao giờ là lỗi DMA, còn `-ENOSR` (EHCI) thì có?
3. Bạn thấy `ERROR unknown event type 37`. Loại Event 37 là gì theo đặc tả xHCI, và vì sao 5.10.258 in log này thay vì xử lý?
4. Trên Quartz, hai log nào trong nhóm F có thể xuất hiện, và hai log nào chắc chắn không?
5. Sắp xếp theo thứ tự thời gian và giải thích quan hệ nhân quả: `HC died; cleaning up`, `WARNING: Host System Error`, `xHCI host controller not responding, assume dead`, `xHCI host not responding to stop endpoint command.`
6. `qh … should be inactive!` (EHCI) và `TRB DMA ptr not part of current TD` (xHCI) giống nhau ở bản chất nào?

---

**Chương tiếp theo:** Chương 9 dựng sơ đồ **log → luồng thực thi** cho các log quan trọng, và phương pháp lần ngược *Log → Hàm → Sự kiện → Trạng thái phần cứng → Nguyên nhân gốc*.
