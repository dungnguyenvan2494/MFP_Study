# Chương 12: So sánh DMA của EHCI và xHCI

> Khoá học USB DMA trên Linux Kernel. Baseline: **Linux 5.10.258**. Chương tổng kết: đặt hai kiến trúc cạnh nhau để thấy **khác biệt nền tảng**, không phải khác biệt trong code.

## 12.1 Kiến trúc bộ nhớ

| Khía cạnh | EHCI | xHCI |
|---|---|---|
| Cấu trúc trong RAM | Periodic Frame List + cây iTD/siTD/QH; vòng Async QH; chuỗi qTD sau mỗi QH | DCBAA → Device Context; mỗi endpoint một Transfer Ring; một Command Ring; Event Ring + ERST; scratchpad |
| Cấp phát | DMA pool `ehci_qh`, `ehci_qtd`, `ehci_itd`, `ehci_sitd`; frame list coherent | DMA pool `xHCI ring segments`, `xHCI input/output contexts`; DCBAA/ERST/scratchpad coherent |
| Descriptor sống bao lâu | qTD cấp mỗi lần truyền, trả về pool khi xong | segment ring cấp sẵn, **tái sử dụng vòng tròn** |
| Trạng thái thiết bị | phần mềm giữ (trong QH) | **controller giữ** (Device Context), phần mềm đổi qua lệnh |
| Độ rộng địa chỉ trên Linux 5.10 | **luôn 32-bit** (bật 64-bit bị `#if 0`) | 32 hoặc 64-bit theo AC64 (Quartz: 32-bit do quirk) |

## 12.2 Kiến trúc descriptor

```
 EHCI                                         xHCI
 QH (endpoint) ─► qTD ─► qTD ─► T             Endpoint Context ─► TR Dequeue ─► [TRB][TRB][Link]…
   overlay = bản sao qTD đang chạy            TRB 16 byte, loại TRB quy định ý nghĩa
 qTD 32 byte: token + 5 con trỏ trang         Normal TRB: 1 vùng liên tục ≤ 64 KiB, không vắt mốc 64 KiB
 iTD/siTD cho isochronous                      Isoch TRB cho isochronous
```

| | EHCI | xHCI |
|---|---|---|
| Đơn vị | qTD (một đoạn dữ liệu tới 5 trang) | TRB (một vùng liên tục) |
| Một URB lớn | chuỗi qTD | một TD = chuỗi TRB nối CHAIN |
| Scatter-gather | sẵn trong qTD (5 trang không cần liên tục) | mỗi đoạn một TRB |
| Gói ngắn | Alternate Next qTD | cờ ISP, Event báo số byte còn lại |
| Dữ liệu nhỏ | luôn qua DMA | **IDT**: ≤ 8 byte OUT nằm ngay trong TRB |

## 12.3 Truy cập buffer dữ liệu

| | EHCI | xHCI |
|---|---|---|
| Cách tính vị trí | con trỏ trang hiện tại + offset, tự sang trang kế tiếp | địa chỉ đầu + đã truyền, trong một vùng liên tục |
| Giới hạn | 20 KiB/qTD (16 KiB nếu không căn trang) | 64 KiB/TRB, không vắt mốc 64 KiB |
| Lỗi không kịp RAM | **Data Buffer Error** (qTD halt) | Data Buffer Error (`HC couldn't access mem fast enough`) |

## 12.4 Cách controller duyệt descriptor

| | EHCI | xHCI |
|---|---|---|
| Kích hoạt | controller **tự đi vòng** schedule mỗi microframe | **doorbell** cho từng endpoint |
| Không có việc | vẫn đọc QH trong vòng Async (tốn băng thông bộ nhớ, đặc biệt khi vòng dài) | endpoint không có doorbell thì không đọc gì |
| Lập lịch periodic | phần mềm đặt S-mask/C-mask, chọn nhánh cây | controller tự lập lịch theo Endpoint Context |
| Cache nội bộ | overlay QH; cần IAA khi gỡ QH | Endpoint Context và TRB; cần Stop Endpoint/Set TR Dequeue |

## 12.5 Đồng bộ phần mềm và phần cứng

| | EHCI | xHCI |
|---|---|---|
| Trao quyền cho phần cứng | bật bit **ACTIVE** trong token qTD (sau barrier) | lật **cycle bit** của TRB đầu (sau `wmb`) + doorbell |
| Phần cứng trả quyền | xóa ACTIVE | ghi Event; TRB đã qua coi như của phần mềm |
| Gỡ an toàn | **IAA** (Async), chờ một frame (Periodic) | **Stop Endpoint** → **Set TR Dequeue** |
| Barrier cần thiết | trước khi bật ACTIVE; sau khi đọc token | trước khi lật cycle; sau khi đọc cycle của Event (`rmb`) |

## 12.6 Cơ chế báo hoàn tất

```
 EHCI:  controller xóa ACTIVE trong qTD ─► USBINT ─► phần mềm QUÉT mọi QH đang hoạt động ─► tìm qTD xong
 xHCI:  controller ghi Transfer Event (TRB pointer, mã, còn lại) ─► ngắt ─► phần mềm đọc Event TUẦN TỰ
```

| | EHCI | xHCI |
|---|---|---|
| Thông tin hoàn tất nằm ở | chính descriptor (token) | Event riêng |
| Chi phí tìm | tỉ lệ số QH đang hoạt động | tỉ lệ số Event |
| Rủi ro đặc trưng | quét bỏ sót, overlay lệch (`qh … should be inactive!`) | Event không khớp TD (`not part of current TD`), Event Ring đầy |
| Gộp ngắt | ngưỡng ngắt theo microframe (ITC) | IMOD theo ns (Quartz: 40 µs) |

## 12.7 Báo lỗi

| Loại lỗi | EHCI | xHCI |
|---|---|---|
| Lỗi giao dịch | qTD halt + XactErr, phần mềm tự thử lại tới 32 lần | Event *USB Transaction Error*, endpoint Halted |
| STALL | qTD halt, CERR ≠ 0 → `-EPIPE` | Event *Stall Error* → `-EPIPE` |
| Babble | qTD halt + Babble → `-EOVERFLOW` | Event *Babble* → `-EOVERFLOW` |
| Không kịp RAM | Data Buffer Error → `-ENOSR`/`-ECOMM` | Event *Data Buffer Error* |
| TRB/descriptor sai | ít có cơ chế riêng | Event **TRB Error** |
| Lỗi bộ nhớ hệ thống | USBSTS **Host System Error** → `fatal error` | USBSTS **HSE** → `WARNING: Host System Error` |
| Lỗi nội bộ controller | (không có bit riêng) | USBSTS **HCE** |

## 12.8 Các kiểu hỏng liên quan DMA

| Kiểu hỏng | EHCI | xHCI |
|---|---|---|
| Giải phóng descriptor khi phần cứng còn cache | bỏ qua IAA → controller đọc QH đã giải phóng | bỏ qua Stop Endpoint → controller đọc TRB/segment đã giải phóng |
| Trao descriptor chưa ghi xong | bật ACTIVE trước khi ghi xong buffer pointer | lật cycle trước khi ghi xong TRB → TRB Error/HSE |
| Lệch đồng bộ | overlay QH khác phần mềm nghĩ | dequeue/cycle state khác phần mềm nghĩ |
| RAM cao | **luôn** bounce (32-bit) | bounce khi mask 32-bit (Quartz), đi thẳng khi 64-bit |
| Băng thông bộ nhớ | đi vòng Async liên tục tốn băng thông, dễ Data Buffer Error khi bus bận | ít truy cập thừa hơn |
| Treo ẩn | qTD ACTIVE mãi, không ai báo | TRB trên ring mãi, hoặc Event bị bỏ |

## 12.9 Vì sao những khác biệt này quan trọng khi debug

| Khi bạn thấy… | Trên EHCI hãy nghĩ | Trên xHCI hãy nghĩ |
|---|---|---|
| URB không bao giờ xong | qTD còn ACTIVE? QH có trong schedule? | ep-context Running? dequeue tiến? Event bị bỏ? |
| Controller chết sau khi hủy URB | gỡ QH không chờ IAA | Stop Endpoint/Set TR Dequeue lỗi, giải phóng quá sớm |
| Dữ liệu hỏng ở RAM cao | bounce có chạy không (luôn 32-bit) | mask có đúng không (AC64, quirk) |
| CPU bận khi không có truyền | (bình thường với EHCI: controller đi vòng không tốn CPU, nhưng quét QH sau ngắt thì tốn) | ít gặp |

## 12.10 Tóm tắt Chương 12

1. EHCI: **danh sách liên kết + controller tự đi vòng + phần mềm tự quét**. xHCI: **ring + doorbell + Event + context do controller giữ**.
2. Cả hai đều cần một cơ chế **chắc chắn phần cứng đã thôi chạm** trước khi phần mềm lấy lại descriptor: IAA (EHCI) và Stop Endpoint/Set TR Dequeue (xHCI).
3. Quyền sở hữu: bit ACTIVE (EHCI) và cycle bit (xHCI). Cả hai cần barrier.
4. EHCI trên Linux 5.10 luôn 32-bit. xHCI tùy AC64, trên Quartz là 32-bit do quirk.
5. Lỗi bộ nhớ hệ thống được báo giống nhau (Host System Error). xHCI có thêm TRB Error và HCE.

## Câu hỏi kiểm tra

1. Vì sao EHCI không cần doorbell còn xHCI thì cần? Cái giá của cách làm EHCI là gì?
2. So sánh IAA và Stop Endpoint về mục đích, chi phí thời gian và điều xảy ra khi phần cứng không phản hồi.
3. Một driver từng chạy tốt trên EHCI, chuyển sang xHCI thì xuất hiện `TRB error`. Giả thuyết đầu tiên của bạn là gì?
4. Với cùng một URB IN 1 MiB, đếm số descriptor cần trên EHCI (buffer căn trang) và trên xHCI (buffer liên tục, căn 64 KiB).
5. Vì sao xHCI phát hiện hoàn tất hiệu quả hơn EHCI khi có nhiều endpoint mở?

---

**Kết thúc khoá học.** Toàn bộ 12 chương được tổng hợp thành tài liệu HTML tra cứu, có sidebar, mục lục, tìm kiếm và bảng thuật ngữ.
