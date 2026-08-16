Trên S800, Linux không phải thứ khởi động đầu tiên — cũng không phải thứ hai. Con R4 luôn-bật phải dựng xong DDR và chép ATF vào bộ nhớ trước khi cụm A72 được phép thoát reset. Trang này theo dấu ba lần bàn giao quyền điều khiển giữa bốn tầng phần mềm.

- 4 tầng phần mềm nối tiếp
- 3 lần bàn giao quyền
- magic của ATF image: 0xB105B002
- QSPI CS0 — ATF + U-Boot: 0xF4000000
- QSPI CS1 — firmware R4: 0xF8000000

# CORTEX-R4 · LPPP (luôn bật)

![[Pasted image 20260816154006.png]]

Điểm dễ bỏ sót nhất nằm ở đường kẻ đứt: **R4 thả AP806 khỏi reset trước khi FreeRTOS của chính nó khởi động**. Trong suốt giai đoạn ATF chạy, LPPP vẫn còn ở vòng `main()` bare-metal và **chưa trả lời được SCPI** — hàng đợi MHU chỉ được tạo ở `LPPP_KM_Init()` mãi sau đó.

# Hình 2 · Bàn giao 1 mổ xẻ — copy_run_atf()

![[Pasted image 20260816135853.png]]

Ảnh ATF **không chạy XIP** như firmware R4 — nó phải được chép vào DDR trước, nên `init_ddr()` bắt buộc phải xong ở bước trước đó. Nếu magic không khớp ở cả hai chip-select, hàm in `"Unable to find ATF code"` rồi `return` — AP806 nằm im mãi mãi và chỉ có watchdog cứu được.

# Hình 3 · Cold boot so với Resume — khác nhau ở đâu

![[Pasted image 20260816135929.png]]

Ba khác biệt quyết định tốc độ: resume **không huấn luyện lại DDR**, **không chép lại ảnh ATF**, và **không chạy U-Boot**. Cờ `PLAT_WARMBOOT_FLAG_BASE` tại `0x7FFFFFF0` là thứ ATF đọc để biết mình đang ở nhánh nào — và `my_callback()` của wake timer cố tình **xoá cờ này về 0** để ép cold boot khi máy thức dậy theo lịch.

## Dòng thời gian ghi vào PS-CPU

Cả LPPP lẫn U-Boot đều ghi mốc thời gian boot vào **cùng một thanh ghi** của PS-CPU — I2C bus 0, slave `0x3E`, register `0xC0` — dựng nên một timeline thống nhất mà công cụ đo đọc lại được sau đó.

| Mã  | Sự kiện                         | Ai ghi    | Thời điểm                                                      | Nguồn                   |
| --- | ------------------------------- | --------- | -------------------------------------------------------------- | ----------------------- |
| 3   | ATFStart                        | R4 · LPPP | Ngay trước `copy_run_atf()`                                    | board.c:519             |
| 4   | WarpStart                       | U-Boot    | Trước `warp_boot()`, cả nhánh Super lẫn Normal                 | common/main.c:2267      |
| 10  | KernelStart                     | U-Boot    | Trong `announce_and_cleanup()`, ngay trước khi nhảy vào kernel | arch/arm/lib/bootm.c:86 |
| 56  | PressPanelPower<br>KeyDeepSleep | R4 · LPPP | Trong transition Sleep2/ErP → Sleep1                           | LPPP_Matrix.c:367       |
| 88  | ResumeRequest                   | R4 · LPPP | Đầu `ap806_power_on(true)` trên dòng Eagle3                    | power.c:329             |
Toàn bộ được gác sau một công tắc: LPPP kiểm `isAutomaticMeasurementToolOn()`, U-Boot kiểm soft DIP `0xA1` bit 0. Máy sản xuất không tốn thời gian ghi log này.

## Bốn điều dễ hiểu nhầm

### 1. Thứ tự

**R4 thả AP806 trước khi RTOS của nó chạy**

`copy_run_atf()` nằm trong `early_board_init()`, được gọi từ `main()` — trước `AppInit_Initialize()` và `vTaskStartScheduler()`. Suốt giai đoạn ATF khởi động, LPPP không thể trả lời SCPI vì hàng đợi MHU chưa tồn tại.

### 2. Bộ nhớ

**Hai chip-select, hai vai trò hoàn toàn khác nhau**

`CS0 @ 0xF4000000` chứa ATF + U-Boot + romfs + user data — **được chép vào DDR rồi mới chạy**. `CS1 @ 0xF8000000` chứa firmware R4 — **chạy XIP tại chỗ**. Đó là lý do khi Linux muốn ghi CS1 thì phải cho R4 ngủ trước.

### 3.Ngược chiều

**U-Boot gọi ngược về LPPP qua SCPI**

Ở `main_loop()`, U-Boot gửi `km_scpi_set_wdt_ca72_ext(REFRESH, 0x11b0, 40, …)` với chuỗi debug `"SuperWarp boot."` — refresh point này chính là giá trị mà LPPP lưu vào `ca72wdte_refreshPoint` và ghi xuống flash nếu watchdog nổ. Nhờ vậy khi máy treo, kỹ sư đọc flash ra là biết nó chết ở giai đoạn boot nào.

### 4.Rủi ro

**Không có ATF hợp lệ thì chỉ watchdog cứu được**

`copy_run_atf()` kiểm magic ở CS0, hỏng thì thử CS1, vẫn hỏng thì in một dòng rồi `return` — AP806 vĩnh viễn nằm trong reset. Watchdog 500 ms bật ở bước 2 là lớp bảo vệ duy nhất, và nó chỉ hoạt động nếu chưa có ai bật watchdog trước đó (`isWdEnable == false`).