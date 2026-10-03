# Phần 25: Bài tập tự kiểm tra

> **Không có đáp án trong file này.** Hãy tự trả lời trước (viết ra, không chỉ nghĩ trong đầu), rồi
> nhắn lại cho tôi (ví dụ: "chấm bài câu 1-10") — tôi sẽ chấm và giải thích từng câu. Mục tiêu là xây
> dựng mô hình tư duy thật, không phải học vẹt đáp án có sẵn.

## Dễ (kiểm tra khái niệm cơ bản)

1. GPIO clock dùng để làm gì? Nếu quên enable, hiện tượng quan sát được là gì (crash, hay im lặng sai)?
2. `GPIO_NOPULL` nghĩa là gì? Khi nào dùng nó là an toàn, khi nào là nguy hiểm?
3. Input floating là gì? Vì sao nó đặc biệt nguy hiểm khi kết hợp với `GPIO_MODE_IT_...`?
4. Push-pull là gì? Vẽ lại sơ đồ 2 transistor và giải thích trạng thái khi xuất HIGH.
5. Falling edge khác Rising edge ở điểm nào? Cho 1 ví dụ tín hiệu trong project này dùng falling-only.
6. EXTI là gì? Nó có nằm "trong" GPIO peripheral không?
7. NVIC làm gì? Nó có tự biết ngắt nào quan trọng hơn ngắt nào không, hay phải được code cấu hình?

## Trung bình (kiểm tra hiểu luồng + đọc code thật)

8. `BSRR` khác `ODR` thế nào về mặt "atomic"? Viết 1 kịch bản cụ thể (có ISR chen vào) cho thấy
   `ODR |= PIN` có thể gây mất dữ liệu còn `BSRR = PIN` thì không.
9. Vì sao GPIO interrupt cần NVIC — nếu EXTI đã phát hiện được edge, tại sao không thể "tự" chạy ISR mà
   phải qua NVIC?
10. Tại sao một IRQn (ví dụ `EXTI4_15_IRQn`) có thể phục vụ nhiều GPIO pin khác nhau? Trong
    `stm32f0xx_it.c` thật của project, `EXTI4_15_IRQHandler()` gọi `HAL_GPIO_EXTI_IRQHandler()` mấy
    lần, với pin nào? Vì sao đúng số lần đó mà không phải nhiều/ít hơn?
11. Tại sao `GPIO_SPEED_FREQ_LOW` **không** có nghĩa là "GPIO chỉ chạy được ở tần số thấp"? Giải thích
    đúng ý nghĩa thật của "Speed" trong ngữ cảnh này.
12. Trong `MX_GPIO_Init()`, `SB_PG_Pin` và `_DISCHG_Pin` bị gọi `HAL_GPIO_Init()` 2 lần với cấu hình
    giống nhau. Việc này có gây lỗi hành vi không? Vì sao? Nếu lần 2 có `Pull` khác lần 1, kết quả thực
    tế sẽ là gì (lần 1 hay lần 2 "thắng")?
13. `HAL_GPIO_Init()` có đụng vào `ODR`/`BSRR` không? Vì sao điều này quan trọng khi xét thứ tự gọi
    `HAL_GPIO_Init()` rồi `HAL_GPIO_WritePin()`?
14. Giải thích tại sao đoạn code reset Backup Domain trong `HAL_RCCEx_PeriphCLKConfig()` (đã học ở câu
    hỏi `SystemClock_Config()` trước đây) **không** chạy lại mỗi lần MCU reset, dù hàm này được gọi
    lại mỗi lần boot.

## Khó (kiểm tra khả năng tự trace/suy luận trên source thật, kể cả phần chưa được giải thích trực tiếp)

15. Tìm trong `mxconstants.h`: có pin nào được định nghĩa nhưng **không** xuất hiện trong bất kỳ lời
    gọi `HAL_GPIO_Init()` nào trong `MX_GPIO_Init()`? Với mỗi pin đó, đề xuất 1 giả thuyết hợp lý về lý
    do (và nói rõ giả thuyết đó cần xác minh thêm bằng gì).
16. `_DISCHG_Pin` được cấu hình là `GPIO_MODE_INPUT` trong `MX_GPIO_Init()`, nhưng ở một file khác
    (`km_extend_io.c`) có dòng `BSP_GPIO_WritePin(_DISCHG_GPIO_Port, _DISCHG_Pin, GPIO_PIN_RESET)`.
    Dòng `WritePin` đó có tác dụng vật lý gì không trong trạng thái hiện tại? Nếu bạn nghi ngờ đây là
    bug, bạn sẽ tìm thêm ở đâu trong codebase để xác nhận trước khi báo cáo?
17. Giả sử bạn muốn thêm 1 pin mới, ví dụ `TEST_OUT_Pin` trên `PF1`, cấu hình Output Push-Pull, mặc
    định HIGH ngay khi vừa chuyển sang output (không được "chớp" qua LOW dù chỉ 1 chu kỳ). Viết (bằng
    lời, không cần code thật) đúng thứ tự các bước cần làm, dựa trên những gì bạn đã học ở file 07.
18. Tính thử: nếu muốn `GPIO_MODE_AF_PP` cho 1 pin để dùng USART1, cần thêm field nào trong
    `GPIO_InitTypeDef` so với khi dùng `GPIO_MODE_OUTPUT_PP`? Dựa vào đoạn code `HAL_GPIO_Init()` đã
    đọc ở file 09, field đó ảnh hưởng tới thanh ghi nào mà `OUTPUT_PP` không chạm tới?
19. Trong bảng GPIO Map (file 08), vì sao `PA9_IN_Pin`/`PA10_IN_Pin` chọn `GPIO_PULLDOWN` thay vì
    `GPIO_NOPULL` dù chúng chỉ là "input dự phòng chưa dùng"? Đưa ra lập luận kỹ thuật (không chỉ nhắc
    lại suy luận `[INFERENCE]` đã nêu — hãy tự phản biện: lập luận đó có lỗ hổng nào không?).
20. (Mở, không có đáp án "đúng" cố định — dùng để tập suy luận kỹ thuật) Nếu schematic thật tiết lộ
    rằng `MC_P_ON_Pin` nối tới 1 MOSFET high-side điều khiển một đường nguồn 24V công suất lớn, bạn
    nghĩ lựa chọn `GPIO_SPEED_FREQ_LOW` (thay vì HIGH) có còn hợp lý không? Giải thích bằng kiến thức
    về slew rate/EMI đã học ở file 05, liên hệ thêm với rủi ro điện áp cảm ứng (inductive kickback) khi
    điều khiển tải công suất lớn tần số chuyển mạch thấp.
