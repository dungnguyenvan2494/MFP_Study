## 1. Kiến trúc & vai trò tổng thể

- MHU (Message Handling Unit) trong `lppp_s800` dùng để làm gì — nó là kênh giao tiếp giữa CPU chính và bộ xử lý phụ nào trong chip Quartz?
- Vì sao thanh ghi trong `hal_MHU.h` đặt tên `SCPI_RX_INT_*` / `SCPI_TX_INT_*` — mối liên hệ với chuẩn **SCPI** (System Control and Power Interface) của ARM là gì, và nó khớp với `arm_scpi.c` + `arm_mhu*.c` bên phía Linux kernel (`Kernel/K-S800/Src/drivers/mailbox/`) như thế nào?
- Tại sao cùng một `hal_MHU.c` lại build được cho cả `Hal/quartz/asic/a0` (chip thật) lẫn `Hal/quartz/mvqsim/a0` (simulator) — khác biệt nằm ở đâu (gợi ý: `regAddrs.h`, `intnums.h` mỗi thư mục định nghĩa khác nhau, code driver dùng chung)?

## 2. Thanh ghi & vùng nhớ chia sẻ (shared memory)

- 6 thanh ghi trong `MHU_regs_t` (RX/TX × STATUS/SET/CLEAR) ánh xạ vào địa chỉ vật lý nào (`MHU_BASE`), và ý nghĩa từng thanh ghi trong giao thức bắt tay ngắt hai chiều là gì?
- `shared_memory_write` / `shared_memory_read` (hal_MHU.c dòng 52-53) được khai báo `static uint8_t* = 0` — **chúng được gán giá trị thực ở đâu?** Tôi không tìm thấy nơi nào set chúng trong file này — đây có phải phần chưa hoàn thiện không?
- Payload thực tế của một message MHU nằm ở vùng nhớ chia sẻ, còn thanh ghi interrupt chỉ mang 1 con số `value` — vậy `value` đó mã hoá "loại message" hay chỉ là bitmask kênh?

## 3. Luồng khởi tạo (init)

- `init_MHU()` trong hal_MHU.c (dòng 66-69) là **hàm rỗng** — trong khi `MHU_register` đã được gán tĩnh ngay từ khai báo. Vậy `init_MHU()` để làm gì, tại sao không có ai gọi nó trong `sys_MHU.c`?
- `MHU_init()` (sys_MHU.c dòng 40) chỉ in `"hello out there&&&&&&\n"` rồi... không `return` gì cả dù khai báo kiểu `int` — đây là debug leftover hay bug thật? Điều gì xảy ra nếu code gọi `if (MHU_init())`?

## 4. Gửi / nhận message

- Trong `MHU_send()` (sys_MHU.c:44), điều kiện `if (get_MHU_status(block, &temp))` — đọc kỹ thì `get_MHU_status()` **luôn return 0** nếu block hợp lệ (xem hal_MHU.c dòng 118-139), vậy comment `// block is active` có đúng ý code không, hay đây là logic kiểm tra busy-bit bị viết sai (lẽ ra phải kiểm tra giá trị `temp`, không phải return code)?
- `MHU_receive()` copy dữ liệu từ `shared_memory_read` — nếu con trỏ này chưa từng được gán (mục 2), lệnh `memcpy(data, shared_region, length)` sẽ crash ở đâu, và trong luồng thực tế điều gì ngăn nó bị gọi trước khi shared memory sẵn sàng?
- Tại sao `MHU_send`/`MHU_receive` (sys_MHU.c) là lớp bọc "protocol" phía trên, còn `write_MHU`/`clear_MHU`/`get_MHU_status` (hal_MHU.c) là lớp "register access" thuần — ranh giới HAL vs SYS ở đây được thiết kế theo nguyên tắc gì?

## 5. Cơ chế dùng song song: bitmask kênh vs shared-memory block

- `app_MHU.c` (dòng 128-250) dùng thẳng `write_MHU(OUTPUT_PORT, 1<<msg.channel_number)` — tức bắn 1 bit theo số kênh, **không đi qua** `MHU_send()`/shared memory. Vậy có hai giao thức MHU cùng tồn tại (bitmask thuần vs message+payload) — service nào dùng cái nào, và chọn sai có gây xung đột thanh ghi không?

## 6. Ngắt & tích hợp ICU

- `get_mhu_intnum()` trả về `INTNUM_MHU_tx0` — số ngắt này được đăng ký với interrupt controller (`hal_icu.c`) ở đâu, mức ưu tiên/edge-vs-level (`EDGE 0` ở hal_MHU.c dòng 47) được cấu hình ra sao?
- `SETSPI_NS`/`CLRSPI_NS` (0xf03f0040 / 0xf03f0048) và `PinID 157`, `IGRP 0` — đây là thanh ghi GIC (Generic Interrupt Controller) để bắn SPI (Shared Peripheral Interrupt) sang lõi khác đúng không? Luồng "CPU A ghi MHU → GIC báo ngắt cho CPU B" đi qua đúng những bước nào?

## 7. Liên hệ hai đầu: LPPP HAL vs Linux kernel driver

- Kernel có tới **4 driver MHU khác nhau** (`arm_mhu.c`, `arm_mhu_base.c`, `arm_mhu_db.c`, `mv_mhu.c`, `platform_mhu.c`) — driver nào thực sự tương ứng với phía cứng mà `lppp_s800/Hal/quartz/MHU` giao tiếp cùng? (gợi ý: tên `mv_mhu.c` = Marvell, khớp trực tiếp với "Marvell" trong license header của `hal_MHU.c`)
- `updateLpppFw()` trong `fwupdate.cpp` (đã trace ở câu hỏi trước) nạp firmware cho LPPP — firmware đó chính là binary chạy trên chip mà `hal_MHU.c` này điều khiển. Sau khi flash xong, ai là bên gửi message MHU **đầu tiên** để đánh thức/handshake với LPPP mới?