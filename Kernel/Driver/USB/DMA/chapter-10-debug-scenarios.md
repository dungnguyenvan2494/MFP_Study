# Chương 10: Tình huống debug thực tế

> Khoá học USB DMA trên Linux Kernel. Baseline: **Linux 5.10.258**. Nhãn log: **[đã xác minh]** = câu chữ đã kiểm tra trong source 5.10.258 (tham số là ví dụ). **[minh họa]** = định dạng đại diện, câu chữ có thể khác giữa các phiên bản. **★** = log thật trên board Quartz.

## Danh sách tình huống

| # | Tình huống | Tầng chính |
|---|---|---|
| 1 | Thiết bị được nhận nhưng truyền dữ liệu thất bại | ring/endpoint |
| 2 | Bulk IN trả dữ liệu hỏng | cache / mask |
| 3 | Bulk OUT treo | thiết bị / ring |
| 4 | Map DMA thất bại trên thiết bị 32-bit | mask / SWIOTLB |
| 5 | xHCI trả Transfer Event có mã lỗi | đường truyền / thiết bị |
| 6 | IOMMU báo DMA page fault | IOMMU |
| 7 | xHCI đột ngột halt | bộ nhớ controller |
| 8 | Chạy trên nền tảng này nhưng lỗi trên nền tảng khác | mask / coherency |
| 9 | Chạy trên kiến trúc coherent, lỗi trên non-coherent | cache |
| 10 | Buffer DMA bị dùng sau khi giải phóng | vòng đời URB |
| 11 | Quartz: URB treo sau khi thoát ngủ (OP_BTS-71095) | đồng bộ ring |
| 12 | Quartz: ổ USB chậm hơn kỳ vọng | mask / SWIOTLB |
| 13 | Rò mapping SWIOTLB dần dần | vòng đời URB |

---

## 1. Thiết bị được nhận nhưng truyền dữ liệu thất bại

| Mục | Nội dung |
|---|---|
| Triệu chứng | `lsusb` thấy thiết bị, driver bind được, nhưng mọi lần đọc/ghi đều lỗi hoặc timeout |
| Log mẫu | `new high-speed USB device number 9 using xhci-hcd` [đã xác minh định dạng]; sau đó `WARN urb submitted to disabled ep` [đã xác minh] hoặc chỉ là timeout của driver |
| Luồng | Enumeration xong (EP0 chạy) → Configure Endpoint cho endpoint bulk thất bại hoặc chưa chạy → URB tới endpoint Disabled/Halted |
| Nguyên nhân | Configure Endpoint lỗi (`Not enough bandwidth`, `Timeout while waiting for configure endpoint command`); driver dùng sai endpoint/alternate setting; endpoint Halted do STALL chưa được clear |
| Cách điều tra | debugfs `devices/NN/ep-context`: trạng thái endpoint; `devices/NN/epXX/` có tồn tại không; log khi bind |
| Kết quả kỳ vọng | Endpoint ở Disabled (chưa cấu hình) hoặc Halted (STALL) |
| Bước debug | 1) xem `ep-context` 2) kiểm tra `usb_set_interface` trong driver 3) với Halted: driver phải gọi `usb_clear_halt` |

## 2. Bulk IN trả dữ liệu hỏng

| Mục | Nội dung |
|---|---|
| Triệu chứng | Dữ liệu sai ở vài vị trí; checksum file lỗi; không có log nào |
| Log mẫu | thường **không có**. Đôi khi `exceeded N overlapping mappings of cacheline …` [đã xác minh, cần DMA API debug] |
| Luồng | Map FROM_DEVICE → xHCI ghi RAM → unmap invalidate → CPU đọc. Hỏng ở đâu đó trong giao nhận cache |
| Nguyên nhân | Buffer chung dòng cache với biến khác (false sharing); CPU đọc buffer trước khi callback; tự dùng `virt_to_phys` bỏ qua DMA API; mask khai báo rộng hơn khả năng thật (dữ liệu được ghi sang nơi khác) |
| Cách điều tra | Điền mẫu đã biết; xem vị trí sai có căn 64 byte không; in địa chỉ buffer (căn 128? nằm trong struct?); thử `swiotlb=force` |
| Kết quả kỳ vọng | Sai đúng ở đầu/cuối buffer → false sharing. Sai theo khối 64 byte ngẫu nhiên → thiếu đồng bộ. Buffer "không đổi" → DMA ghi nhầm chỗ |
| Bước debug | `kmalloc` buffer riêng; rà code chạm buffer khi URB đang chạy; bật DMA API debug |

## 3. Bulk OUT treo

| Mục | Nội dung |
|---|---|
| Triệu chứng | `write()` không trả về, hoặc trả lỗi sau timeout |
| Log mẫu | không có log USB, hoặc sau timeout là log hủy URB; nếu controller có vấn đề: `xHCI host not responding to stop endpoint command.` [đã xác minh] |
| Luồng | TRB đã trao, doorbell đã bấm → controller gửi OUT → thiết bị NAK mãi, **hoặc** Event tới nhưng bị bỏ (`not part of current TD`), **hoặc** doorbell không được bấm (endpoint đang chờ Set TR Dequeue) |
| Nguyên nhân | Thiết bị không nhận (FIFO đầy, firmware treo); ring lệch; endpoint kẹt ở trạng thái Stopped |
| Cách điều tra | debugfs: `ep-context` (Running/Stopped/Halted), `dequeue` có tiến không; usbmon (cần build lại): có gói OUT đi ra không |
| Kết quả kỳ vọng | Running + dequeue đứng yên → thiết bị NAK. Stopped → thiếu doorbell. Dequeue đã qua TD nhưng URB không về → Event bị bỏ |
| Bước debug | Kiểm tra thiết bị; tìm log `not part of current TD`; đảm bảo driver hủy URB khi timeout |

## 4. Map DMA thất bại trên thiết bị 32-bit

| Mục | Nội dung |
|---|---|
| Triệu chứng | `usb_submit_urb` trả `-EAGAIN` lác đác hoặc liên tục |
| Log mẫu | `xhci-hcd 8d00d0000.usb3: swiotlb buffer is full (sz: 262144 bytes), total 32768 (slots), used 30112 (slots)` [đã xác minh định dạng, số liệu minh họa]; hoặc `DMA addr 0x…+… overflow (mask ffffffff, bus limit 0).` [đã xác minh] |
| Luồng | Buffer ở trên 4 GiB → vượt mask 32-bit → SWIOTLB → hết ô hoặc map quá 256 KiB |
| Nguyên nhân | Rò mapping; map khối quá lớn; SWIOTLB bị tắt (`swiotlb=noforce`); kho nhỏ |
| Cách điều tra | Xem `sz` và `used` theo thời gian; tham số boot; `dma_max_mapping_size` có được tôn trọng không |
| Kết quả kỳ vọng | `sz` > 256 KiB → chia nhỏ. `used` tăng mãi → rò |
| Bước debug | Chia nhỏ URB; sửa rò; với buffer dùng lâu dài có thể dùng `usb_alloc_coherent` (luôn trong mask) |

## 5. xHCI trả Transfer Event có mã lỗi

| Mục | Nội dung |
|---|---|
| Triệu chứng | URB về với `-EPROTO`, `-EPIPE`, `-EOVERFLOW` |
| Log mẫu | `WARN: TRB error for slot 3 ep 2 on endpoint` [đã xác minh]; `WARN: HC couldn't access mem fast enough for slot 3 ep 2` [đã xác minh]; Transaction Error/Stall/Babble thường chỉ có ở mức debug |
| Luồng | Controller thực thi TRB → lỗi trên bus hoặc ở bộ nhớ → Transfer Event với completion code → driver ánh xạ sang mã lỗi URB |
| Nguyên nhân theo mã | Transaction Error: cáp/nhiễu/thiết bị. Stall: thiết bị từ chối lệnh. Babble: thiết bị gửi quá dài. **TRB Error**: TRB có trường sai, tức lỗi phần mềm. **Data Buffer Error**: controller không kịp RAM |
| Cách điều tra | Xác định completion code; với TRB Error dump `trbs` của endpoint; với Data Buffer Error đo tải bus bộ nhớ |
| Kết quả kỳ vọng | TRB Error → độ dài/cờ TRB sai hoặc TRB trao khi chưa ghi xong. DBE → tải DMA khác cao |
| Bước debug | Trace `xhci_queue_trb` (cần FTRACE); kiểm tra barrier; giảm tải DMA đồng thời |

## 6. IOMMU báo DMA page fault (SoC có SMMU, không áp dụng Quartz)

| Mục | Nội dung |
|---|---|
| Triệu chứng | Controller halt, thiết bị biến mất |
| Log mẫu | `arm-smmu …: Unhandled context fault: fsr=0x402, iova=0xfffe1000, fsynr=0x…, cbfrsynra=0x…, cb=3` [đã xác minh định dạng, giá trị minh họa] → `WARNING: Host System Error` [đã xác minh] |
| Luồng | IOVA bị unmap (hoặc chưa từng map) → controller DMA tới đó → SMMU chặn → xHCI nhận lỗi bus → HSE |
| Nguyên nhân | Dùng sau khi unmap; `virt_to_phys` thay vì DMA API; ring/context bị ghi đè; IOVA bị cắt do mask sai |
| Cách điều tra | Tra `iova` trong trace `iommu:map`/`iommu:unmap`; DMA API debug `dump` |
| Kết quả kỳ vọng | `iova` thuộc một mapping vừa unmap → dùng sau khi unmap |
| Bước debug | Sửa thứ tự hủy URB (hủy xong rồi mới giải phóng); kiểm tra mọi chỗ ghi địa chỉ vào TRB/context |

## 7. xHCI đột ngột halt

| Mục | Nội dung |
|---|---|
| Triệu chứng | Mọi thiết bị USB trên một controller biến mất cùng lúc |
| Log mẫu | `WARNING: Host System Error` → `xHCI host not responding to stop endpoint command.` → `USBSTS: …` → `xHCI host controller not responding, assume dead` → `HC died; cleaning up` [tất cả đã xác minh] |
| Luồng | Chương 9, mục 9.1 và 9.2 |
| Nguyên nhân | DMA địa chỉ sai; controller mất clock/nguồn (USBSTS = 0xFFFFFFFF); erratum; thao tác suspend/resume sai |
| Cách điều tra | Log bất thường đầu tiên; có suspend/resume ngay trước không; USBSTS in ra |
| Kết quả kỳ vọng | HSE → lỗi bộ nhớ. Toàn bit 1 → controller mất truy cập. Không bit lỗi → ngắt/Event Ring |
| Bước debug | Tái hiện kèm dump debugfs trước và sau; bật KASAN và DMA API debug trong bản build debug |

## 8. Chạy trên nền tảng này nhưng lỗi trên nền tảng khác

| Mục | Nội dung |
|---|---|
| Triệu chứng | Driver tốt trên PC, lỗi trên SoC (hoặc ngược lại) |
| Log mẫu | không có, hoặc `swiotlb buffer is full …`, hoặc dữ liệu hỏng |
| Luồng | Khác biệt nằm ở: mask (32/64), IOMMU có/không, coherent/non-coherent, RAM trên 4 GiB có/không |
| Nguyên nhân | Driver giả định coherent; dùng `virt_to_phys`; buffer trên stack/trong struct; mask sai |
| Cách điều tra | Lập bảng bốn đặc điểm DMA của hai nền tảng (như Chương 1, mục 1.0) |
| Kết quả kỳ vọng | Lỗi trùng với đúng một khác biệt |
| Bước debug | Thử `swiotlb=force` trên nền tảng "tốt" để mô phỏng bounce; dùng DMA API debug |

## 9. Chạy trên kiến trúc coherent, lỗi trên non-coherent

| Mục | Nội dung |
|---|---|
| Triệu chứng | x86/SoC có ACE: ổn. Quartz: hỏng dữ liệu lác đác |
| Log mẫu | không có |
| Luồng | Trên coherent, mọi thao tác dọn cache là no-op, nên lỗi đồng bộ bị che giấu |
| Nguyên nhân | CPU ghi buffer sau khi map; đọc trước unmap; thiếu sync; false sharing; sửa TRB/ring không qua barrier (driver tự viết) |
| Cách điều tra | Chương 6, mục 6.8 |
| Kết quả kỳ vọng | Sai theo khối 64 byte |
| Bước debug | Rà mọi chỗ CPU chạm buffer trong khoảng map–unmap |

## 10. Buffer DMA bị dùng sau khi giải phóng

| Mục | Nội dung |
|---|---|
| Triệu chứng | Hỏng bộ nhớ ngẫu nhiên, oops ở chỗ không liên quan USB, thường sau timeout USB |
| Log mẫu | `Unable to handle kernel paging request at virtual address …` [minh họa]; với KASAN: `BUG: KASAN: use-after-free in …` [minh họa]. Lưu ý KASAN chỉ bắt **CPU** truy cập, không bắt **thiết bị** |
| Luồng | Driver timeout → `kfree(buffer)` mà không `usb_kill_urb` → TRB vẫn trên ring → controller ghi vào vùng đã cấp cho người khác |
| Nguyên nhân | Hủy URB sai cách (Chương 3, mục 3.6) |
| Cách điều tra | Rà mọi đường timeout và lỗi của driver: có `usb_kill_urb` trước khi giải phóng không; trên SoC có SMMU sẽ thấy fault |
| Kết quả kỳ vọng | Một đường lỗi giải phóng buffer khi URB còn active |
| Bước debug | `usb_kill_urb` (đồng bộ) trước mọi `kfree`; dùng `usb_anchor` để hủy hàng loạt khi disconnect |

## 11. Quartz: URB treo sau khi thoát ngủ (OP_BTS-71095)

| Mục | Nội dung |
|---|---|
| Triệu chứng | Sau khi thoát SLEEP1, bo FAX không ghi được, cuối cùng ra B197 |
| Log ★ | `xhci-hcd 8d00d0000.usb3: ERROR Transfer event TRB DMA ptr not part of current TD ep_index 1 comp_code 1` → `usbfax … write error` → `URB 00000000330743d7 submitted while active` → `can't set config #1, error -71` |
| Luồng | Chương 9, mục 9.10 |
| Nguyên nhân | Ring lệch đồng bộ sau SLEEP1 (chưa xác định gốc); BusReset gọi sai cổng; `usbfax_write` không hủy URB |
| Cách điều tra | Bật dump Event; dump debugfs ring EP1 OUT ngay khi lỗi; so địa chỉ |
| Kết quả kỳ vọng | Xác định X thuộc TD cũ, segment lạ, hay rác |
| Bước debug | Sửa `usbfax_write` (kill URB); sửa cổng BusReset; thêm bước reset cấp controller; tái hiện kèm thu thập |

## 12. Quartz: ổ USB chậm hơn kỳ vọng

| Mục | Nội dung |
|---|---|
| Triệu chứng | Copy file từ ổ USB 3.0 chậm hơn nhiều so với trên PC |
| Log mẫu | không có |
| Luồng | xHCI mask 32-bit + RAM trên 4 GiB → `dma_addressing_limited` → `dma_max_mapping_size` = 256 KiB → usb-storage giới hạn mỗi lệnh 512 sector; buffer cao phải bounce (2 lần memcpy) |
| Nguyên nhân | Giới hạn kiến trúc, không phải lỗi |
| Cách điều tra | `cat /sys/block/sdX/queue/max_hw_sectors_kb` (kỳ vọng 256); đo tải CPU khi copy |
| Kết quả kỳ vọng | 256 KiB mỗi lệnh; CPU bận memcpy |
| Bước debug | Không có cách sửa trong driver; chỉ cải thiện được ở phần cứng (64-bit thật, IOMMU) hoặc bằng cách giữ buffer trong RAM thấp |

## 13. Rò mapping SWIOTLB dần dần

| Mục | Nội dung |
|---|---|
| Triệu chứng | Sau nhiều giờ chạy, các thiết bị bắt đầu lỗi `-EAGAIN` |
| Log mẫu | `swiotlb buffer is full …` với `used` tăng dần qua các lần in [đã xác minh định dạng] |
| Luồng | URB không bao giờ giveback → không unmap → ô bounce không được trả |
| Nguyên nhân | Event bị bỏ (TD không hoàn tất); driver không hủy URB; driver tự map mà quên unmap |
| Cách điều tra | Đếm URB đang active theo endpoint; DMA API debug `num_free_entries`/`dump` (khi bật) |
| Kết quả kỳ vọng | Số mapping còn sống tăng tuyến tính theo số lần lỗi |
| Bước debug | Sửa đường hủy URB; thêm giám sát số URB active |

---

## Tóm tắt Chương 10

1. Phần lớn tình huống DMA rơi vào năm nhóm: **mask/SWIOTLB**, **cache**, **vòng đời URB (hủy sai, dùng sau giải phóng)**, **đồng bộ ring**, **truy cập bộ nhớ của controller**.
2. Lỗi cache và lỗi mask khai báo rộng **không có log**. Phải dùng mẫu dữ liệu, căn lề và so sánh nền tảng.
3. `submitted while active`, `Ring expansion failed`, `swiotlb buffer is full` thường là **hậu quả** của URB không được trả về.
4. Trên Quartz, thiếu SMMU, FTRACE, usbmon, DMA API debug. Một bản build debug riêng là khoản đầu tư xứng đáng cho các ticket kiểu OP_BTS-71095.

## Câu hỏi kiểm tra

1. Tình huống 3: làm sao phân biệt "thiết bị NAK mãi" và "Event bị bỏ" chỉ bằng debugfs?
2. Tình huống 10: vì sao KASAN không bắt được khi controller DMA vào vùng đã giải phóng?
3. Tình huống 12: nếu Quartz có SMMU, `max_hw_sectors_kb` sẽ thay đổi thế nào? Vì sao?
4. Chọn hai tình huống mà `swiotlb=force` giúp phân biệt nguyên nhân, và giải thích.
5. Với tình huống 11, liệt kê ba thay đổi code giúp hệ thống tự phục hồi mà không cần biết nguyên nhân gốc.

---

**Chương tiếp theo:** Chương 11 hướng dẫn các công cụ debug: dmesg, journalctl, usbmon, ftrace/trace-cmd, dynamic debug, DMA API debug, IOMMU fault reporting, debugfs USB, lspci, lsusb, kèm tình trạng sẵn có trên Quartz.
