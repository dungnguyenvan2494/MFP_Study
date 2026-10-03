# GPIO Deep Dive — PS-CPU (STM32F031C6Tx)

Bộ tài liệu học sâu về GPIO, từ bản chất phần cứng đến source code thật của project
PS-CPU (`pscpu_s800/main/Src/mx_init.c`, hàm `MX_GPIO_Init()`, dòng 419-518).

**Quy tắc tag nguồn dùng trong toàn bộ bộ tài liệu này:**
- `[SOURCE]` — lấy trực tiếp từ code thật trong repo, có file:line.
- `[GENERAL MCU KNOWLEDGE]` — kiến thức điện tử/MCU phổ quát, không riêng project này (ví dụ: push-pull là gì, pull-up resistor hoạt động thế nào). Đúng cho hầu hết MCU nhưng không được "bịa" số liệu riêng của project nếu không có trong code.
- `[INFERENCE]` — suy luận hợp lý từ tên signal/code nhưng **chưa xác nhận bằng schematic**. Luôn được đánh dấu rõ, không bao giờ biến thành fact.

**MCU xác nhận**: `STM32F031C6Tx` — xem `PS-CPU/pscpu_s800/main/PowerSubCPU.ioc:21,60,225`. Đây là chip dòng
Cortex-M0, HSI nội = 8MHz (không phải 16MHz), không có HSI48.

## Danh sách phần (đọc theo thứ tự)

| # | File | Nội dung | Tương ứng yêu cầu gốc |
|---|------|----------|------------------------|
| 01 | `01_gpio_hardware_va_kien_truc.md` | GPIO là gì, kiến trúc Port/Pin/Peripheral, vị trí trong MCU | Phần 1-2 |
| 02 | `02_rcc_clock_enable.md` | RCC, GPIO clock, macro → register → bit | Phần 3 |
| 03 | `03_gpio_inittypedef.md` | `GPIO_InitTypeDef`, bitmask `Pin` | Phần 4 |
| 04 | `04_input_floating_pullup_pulldown.md` | `GPIO_MODE_INPUT`, pull-up/pull-down, floating | Phần 5-6 |
| 05 | `05_output_pushpull_opendrain_speed.md` | Push-pull, Open-drain, GPIO Speed | Phần 7-9 |
| 06 | `06_exti_nvic_interrupt_chain.md` | EXTI, Rising/Falling, NVIC, trace tới ISR thật | Phần 10-13 |
| 07 | `07_writepin_bsrr_output_ordering.md` | `HAL_GPIO_WritePin`, BSRR vs ODR, thứ tự init→set level | Phần 14-15 |
| 08 | `08_toan_bo_source_phan_tich.md` | Nhóm toàn bộ pin theo chức năng, bảng GPIO Map, phân tích đoạn code trùng, `#if 0` | Phần 16-19 |
| 09 | `09_hal_to_register_trace.md` | Trace đầy đủ 1 ví dụ: code → HAL → CMSIS register → hardware; sơ đồ kiến trúc tổng thể | Phần 20-21 |
| 10 | `10_debug_va_loi_thuong_gap.md` | Cách debug (code/register/oscilloscope/debugger), 18 lỗi GPIO thường gặp | Phần 22-23 |
| 11 | `11_schematic_va_tai_lieu_khac.md` | Giới hạn của source code, cần đọc thêm gì | Phần 24 |
| 12 | `12_bai_tap.md` | Bài tập tự kiểm tra — **không có đáp án trong file này**, theo đúng yêu cầu học tập đã thống nhất trước đó | Phần 25 |

## Tiến độ

| File | Trạng thái |
|------|-----------|
| 00-12 | Đã viết lần đầu, dựa trên source đọc thật ngày hôm nay |

## Ghi chú quan trọng về Phần 25 (bài tập)

Theo preference học tập đã thiết lập trước đó của bạn (áp dụng cho mọi bài tập dự đoán/tự kiểm tra,
không riêng FreeRTOS): **câu hỏi được đưa ra nhưng KHÔNG kèm đáp án**. Hãy tự trả lời trước, sau đó
nhắn lại (ví dụ "chấm bài câu 1-5") thì tôi sẽ chấm và giải thích — việc này giúp bạn xây dựng mô hình
tư duy thật thay vì học vẹt đáp án.
