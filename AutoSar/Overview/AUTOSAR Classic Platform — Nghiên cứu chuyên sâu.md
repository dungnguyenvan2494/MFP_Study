Mục tiêu tài liệu: xây dựng mental model đầy đủ, có hệ thống về AUTOSAR Classic Platform cho một kỹ sư đã quen C/C++, Linux kernel, BSP, driver, interrupt, scheduler nhưng chưa từng làm AUTOSAR — dựa trên trang chính thức autosar.org/standards/classic-platform và các specification/explanation liên quan, có trích nguồn rõ ràng, phân biệt specification vs giải thích vs vendor implementation.

Tài liệu gồm 4 tab: (00) Tổng quan, Kiến trúc, SWC & RTE — tab hiện tại; (01) BSW, MCAL & AUTOSAR OS; (02) Communication, Diagnostics, Memory & Startup; (03) ARXML, Methodology, Use Case, So sánh & Cheat Sheet.

## 1. AUTOSAR là gì & Classic Platform là gì

**AUTOSAR** = _AUTomotive Open System ARchitecture_ — một liên minh (OEM, Tier-1, nhà sản xuất vi điều khiển, nhà cung cấp tool) thành lập năm 2003, đưa ra một **kiến trúc phần mềm + methodology chuẩn hóa** cho phần mềm ô tô, không phải một sản phẩm hay RTOS cụ thể.

**Vấn đề trước AUTOSAR:** mỗi OEM/Tier-1 tự viết phần mềm ECU riêng, gắn chặt với vi điều khiển cụ thể (không có lớp abstraction chuẩn) → không thể tái sử dụng SWC giữa các dự án/ECU/nhà cung cấp khác nhau, chi phí tích hợp cao, khó chuyển đổi vi điều khiển hay nhà cung cấp mà không viết lại toàn bộ.

**AUTOSAR giải quyết:** tách **application software** (SWC) khỏi **basic software** (BSW, gắn hạt nhân/phần cứng) thông qua một lớp trung gian chuẩn hóa là **RTE**; chuẩn hóa API của BSW/MCAL; chuẩn hóa định dạng trao đổi (ARXML) và methodology cấu hình/tool chain — nhờ đó SWC có thể viết độc lập với phần cứng, di chuyển giữa các ECU, và BSW/MCAL của nhiều nhà cung cấp (Vector, EB, ETAS, Infineon...) có thể thay thế nhau qua cùng một interface chuẩn.

**AUTOSAR KHÔNG giải quyết:** không cung cấp RTOS/compiler/silicon cụ thể (đó là việc của vendor); không tự động đảm bảo an toàn chức năng (ASIL) — chỉ cung cấp cơ chế (Det, memory protection, timing protection...) để xây dựng hệ thống an toàn; không chuẩn hóa logic ứng dụng (thuật toán điều khiển động cơ, phanh... vẫn là IP của OEM/Tier-1).

### Classic Platform là gì

Theo trang chính thức ([autosar.org/standards/classic-platform](https://www.autosar.org/standards/classic-platform)), kiến trúc Classic Platform phân biệt 3 lớp phần mềm chạy trên **một vi điều khiển (µC)**: Application, RTE, BSW — đây là điểm khác biệt cốt lõi so với Adaptive Platform. Một khái niệm nền tảng là **Virtual Functional Bus (VFB)**: bộ SWC giao tiếp qua các _port_ mà không cần biết chi tiết hạ tầng bên dưới (cùng ECU hay khác ECU) — VFB là khung niệm trỪu tượng, khi triển khai thực tế nó trở thành RTE (nội bộ ECU) + communication stack (liên ECU).


|                   | **Classic Platform**                                            | **Adaptive Platform**                             |
| ----------------- | --------------------------------------------------------------- | ------------------------------------------------- |
| Mục tiêu ECU      | Vi điều khiển tài nguyên hạn chế, real-time cứng                | ECU hiệu năng cao (SoC đa nhân), cần POSIX OS     |
| Ví dụ ECU         | Engine control, brake, body control, powertrain                 | ADAS/AD compute, infotainment, gateway trung tâm  |
| OS                | AUTOSAR OS (OSEK-based, static, khong MMU bắt buộc)             | POSIX OS (Linux, QNX...), cấp phát động           |
| Mô hình giao tiếp | RTE + Signal-based (CAN/LIN/FlexRay)                            | Service-Oriented (SOME/IP, DDS)                   |
| Cấu hình          | Chủ yếu build-time/static (ARXML → code generation)             | Có thể cập nhật/deploy động hơn                   |
| Khi nào dùng      | Cần ASIL cao, xung nhịp/tiền định nghiêm ngặt, chi phí BOM thấp | Cần xử lý dữ liệu lớn, ML, cập nhật OTA linh hoạt |
