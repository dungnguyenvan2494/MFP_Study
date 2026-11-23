# Bảng thuật ngữ

| Thuật ngữ | Tiếng Việt | Nghĩa ngắn gọn | Chương |
|---|---|---|---|
| DMA (Direct Memory Access) | truy cập bộ nhớ trực tiếp | phần cứng không phải CPU tự đọc/ghi RAM | 1, 2 |
| Bus mastering | làm chủ bus | thiết bị tự phát giao dịch bộ nhớ (first-party DMA); xHCI, EHCI | 1, 2 |
| DMA Controller | bộ điều khiển DMA | khối chép hộ cho thiết bị đơn giản (third-party DMA), dùng dmaengine | 2 |
| PIO (Programmed I/O) | CPU tự chép | CPU đọc/ghi từng word qua thanh ghi | 1, 2 |
| CPU virtual address | địa chỉ ảo CPU | địa chỉ code C dùng, dịch qua MMU | 1, 2 |
| Physical address | địa chỉ vật lý | địa chỉ RAM nhìn từ CPU | 1, 2 |
| DMA address (`dma_addr_t`) | địa chỉ DMA | địa chỉ thiết bị phát lên bus; giá trị ghi vào TRB | 1, 2 |
| IOVA | địa chỉ ảo I/O | địa chỉ DMA khi có IOMMU | 2, 7 |
| IOMMU / SMMU | bộ quản lý bộ nhớ cho I/O | dịch IOVA → vật lý, chặn truy cập sai | 7 |
| DMA mapping / unmapping | ánh xạ / gỡ ánh xạ DMA | chuyển quyền sở hữu buffer CPU ↔ thiết bị, kèm đổi địa chỉ và dọn cache | 2 |
| Coherent DMA memory | bộ nhớ DMA nhất quán | cấp một lần, hai bên dùng song song; trên Quartz là không cache | 2, 6 |
| Streaming DMA | DMA dạng luồng | buffer có cache, map/unmap mỗi lần truyền | 2, 6 |
| DMA direction | chiều DMA | TO_DEVICE, FROM_DEVICE, BIDIRECTIONAL; quyết định dọn cache/bounce | 2 |
| DMA mask | mặt nạ DMA | số bit địa chỉ thiết bị phát được (`dma_mask`, `coherent_dma_mask`) | 2, 7 |
| `bus_dma_limit` | giới hạn của bus | giới hạn địa chỉ do đường bus (từ `dma-ranges`) | 7 |
| SWIOTLB | bộ đệm bounce phần mềm | kho 64 MiB dưới 4 GiB để chép qua lại cho thiết bị không với tới RAM cao | 1, 2, 7 |
| Bounce buffer | bộ đệm trung chuyển | ô SWIOTLB thay cho buffer gốc | 2, 7 |
| DMA pool | vùng nhớ DMA nhỏ | cắt trang coherent thành đối tượng nhỏ cùng kích thước | 2 |
| Cache line | dòng cache | đơn vị cache, 64 byte trên arm64 | 6 |
| Clean / Invalidate | làm sạch / vô hiệu | đẩy dòng bẩn xuống RAM / bỏ dòng khỏi cache | 6 |
| Coherent vs non-coherent | nhất quán / không nhất quán | interconnect có snoop cache CPU hay không; Quartz là non-coherent | 6 |
| Memory barrier | rào bộ nhớ | buộc thứ tự đọc/ghi (`wmb`, `rmb`, `dma_wmb`) | 6 |
| False sharing | chia sẻ dòng cache giả | buffer DMA chung dòng cache với biến khác | 6 |
| URB (USB Request Block) | khối yêu cầu USB | đơn vị yêu cầu truyền ở USB core | 3 |
| HCD | driver host controller | xHCI/EHCI driver | 3 |
| Giveback | trả URB | HCD trả URB cho USB core, kèm unmap và gọi callback | 3 |
| HCD_BH | nửa dưới của HCD | giveback chạy trong tasklet | 3 |
| xHCI | — | host controller USB thế hệ mới, mọi tốc độ | 5 |
| EHCI | — | host controller USB 2.0 High-Speed | 4 |
| TRB | khối yêu cầu truyền | đơn vị 16 byte trên ring của xHCI | 5 |
| TD (Transfer Descriptor) | mô tả lần truyền | chuỗi TRB của một URB | 3, 5 |
| Transfer / Command / Event Ring | các ring | vòng tròn TRB cho dữ liệu / lệnh / sự kiện | 5 |
| Segment | đoạn ring | 256 TRB = 4 KiB | 5 |
| Link TRB | TRB nối | trỏ sang segment kế tiếp; Toggle Cycle khép vòng | 5 |
| Cycle bit (PCS/CCS) | bit chu kỳ | quyết định TRB thuộc producer hay consumer | 5 |
| Doorbell | chuông | thanh ghi đánh thức endpoint/Command Ring | 5 |
| DCBAA | mảng địa chỉ context | DCBAA[slot] → Output Device Context; [0] → scratchpad | 5 |
| Input / Output Context | context vào / ra | SW ghi để ra lệnh / HW ghi trạng thái thiết bị | 5 |
| Endpoint Context | context endpoint | trạng thái endpoint, TR Dequeue Pointer | 5 |
| Interrupter (IMAN, IMOD, ERDP) | bộ ngắt | bật ngắt, gộp ngắt, vị trí đã đọc Event | 5 |
| ERST | bảng segment Event Ring | địa chỉ và kích thước segment của Event Ring | 5 |
| Scratchpad | bộ nhớ nháp | RAM của host dành riêng cho controller | 5 |
| Stop Endpoint / Set TR Dequeue | dừng endpoint / đặt lại dequeue | cặp lệnh để hủy TD an toàn | 3, 5 |
| QH (Queue Head) | đầu hàng | EHCI: đại diện một endpoint, có overlay | 4 |
| qTD | mô tả phần tử hàng | EHCI: một đoạn dữ liệu, tới 5 trang 4 KiB | 4 |
| iTD / siTD | mô tả isochronous | EHCI: isoc High-Speed / Full-Speed qua TT | 4 |
| Periodic / Async Schedule | lịch định kỳ / không định kỳ | EHCI: interrupt+isoc / control+bulk | 4 |
| IAA (Interrupt on Async Advance) | ngắt khi vòng Async tiến | EHCI: chờ controller bỏ cache QH trước khi giải phóng | 4 |
| HSE (Host System Error) | lỗi hệ thống của host | controller truy cập bộ nhớ thất bại | 4, 5, 8, 9 |
| HCE (Host Controller Error) | lỗi nội bộ controller | xHCI cần reset | 5, 8 |
| Data Buffer Error | lỗi bộ đệm dữ liệu | controller không kịp truy cập RAM | 4, 5, 8 |
| Babble | thiết bị "nói quá" | thiết bị gửi nhiều hơn mong đợi | 3, 4 |
| STALL | dừng endpoint | thiết bị từ chối yêu cầu, endpoint Halted | 3, 4 |
| Short packet | gói ngắn | gói IN nhỏ hơn max packet, kết thúc TD sớm | 3 |
| ZLP | gói rỗng | gói dài 0 byte báo hết dữ liệu | 3 |
| IDT (Immediate Data) | dữ liệu tức thời | ≤ 8 byte OUT nằm ngay trong TRB, không DMA | 2, 5 |
| debugfs | hệ thống file debug | `/sys/kernel/debug/usb/xhci/…`: ring, context, thanh ghi | 11 |
| usbmon | theo dõi USB | ghi URB submit/complete | 11 |
| DMA API debug | kiểm tra DMA API | bắt lỗi dùng sai mapping | 2, 6, 11 |
