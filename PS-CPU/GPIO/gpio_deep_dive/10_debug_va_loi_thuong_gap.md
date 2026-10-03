# Phần 22-23: Thực hành Debug GPIO & Các lỗi thường gặp

## Phần 22 — Cách debug GPIO

### Bằng source code

```c
HAL_GPIO_ReadPin(GPIOx, PIN)     // đọc 1 bit IDR — dùng để in log/breakpoint kiểm tra trạng thái input
HAL_GPIO_WritePin(GPIOx, PIN, STATE)  // tự ép mức để test phần cứng phía sau (ví dụ tự tay set MC_P_ON=1
                                       //  để kiểm tra module nguồn phản ứng đúng không, tách biệt khỏi
                                       //  toàn bộ logic điều kiện _MC_P_ON_OK())
```
Kỹ thuật thực dụng: tạm thời chèn `HAL_GPIO_WritePin(_USB2_OE_GPIO_Port, _USB2_OE_Pin, ...)` để nhấp
nháy LED debug — project này **đã có sẵn cơ chế này** dưới macro `#ifdef DEBUG_LED`
`[SOURCE]` [km_it.c:290-297](../../pscpu_s800/main/App/km_it.c#L290-L297), bạn chỉ cần định nghĩa
`DEBUG_LED` khi build để bật.

### Bằng register (qua debugger, không cần sửa code)

Hầu hết IDE nhúng (Keil, STM32CubeIDE...) có khung xem "Peripheral Register" — xem trực tiếp giá trị
hiện tại của:
```text
GPIOB->MODER     — xác nhận PB5 có đúng là Output (01) không
GPIOB->OTYPER    — xác nhận Push-Pull (0) hay vô tình thành Open-Drain (1)
GPIOB->OSPEEDR   — xác nhận Speed
GPIOB->PUPDR     — xác nhận Pull
GPIOB->IDR       — đọc mức input hiện tại
GPIOB->ODR       — đọc giá trị output đang set (lưu ý: ODR phản ánh giá trị SAU khi BSRR/BRR đã áp dụng,
                    không phải giá trị bạn "định" ghi vào BSRR)
GPIOB->BSRR      — chỉ dùng để GHI, đọc lại thường luôn = 0 (vì đây là write-only theo thiết kế atomic)
```
Nếu `MODER` hiện giá trị khác bạn mong đợi → kiểm tra lại có đoạn code nào khác (ví dụ mục Phần 18 đã
phân tích — cấu hình trùng/ghi đè) đang chạy `HAL_GPIO_Init()` lần 2 với Mode khác sau đoạn bạn tưởng là
"cuối cùng".

### Bằng Oscilloscope / Logic Analyzer

`[GENERAL MCU KNOWLEDGE]`
- **Kiểm tra GPIO output**: đặt que đo trực tiếp vào chân vật lý, quan sát mức điện áp thực tế có
  đúng 0V/3.3V và đúng thời điểm chuyển mức như code mong đợi không — phát hiện được cả **glitch** (một
  xung rất ngắn không mong muốn, mà debug bằng breakpoint/single-step **không bao giờ thấy được**, vì
  breakpoint làm chậm CPU, còn glitch chỉ xảy ra ở tốc độ chạy thật).
- **Kiểm tra GPIO interrupt / rising/falling edge**: dùng Logic Analyzer bắt đồng thời **tín hiệu vật
  lý trên chân** và **một chân debug khác** (ví dụ toggle `_USB2_OE` ngay đầu ISR) → đo được **độ trễ
  thật** từ lúc edge xảy ra tới lúc ISR chạy (gọi là "interrupt latency") — rất hữu ích khi nghi ngờ
  ngắt bị trễ do có ngắt khác ưu tiên cao hơn đang chiếm CPU lâu.

### Bằng Debugger (breakpoint, watch, memory view)

- **Breakpoint** trong `km_exti_callback()` tại đúng `case` của pin nghi vấn → xác nhận ISR **có thực
  sự được gọi** không (loại trừ khả năng quên `HAL_NVIC_EnableIRQ` hoặc EXTICR map sai port).
- **Watch register**: thêm `GPIOB->IDR`, `EXTI->PR`, `EXTI->IMR` vào watch window — theo dõi **PR**
  thay đổi theo thời gian thực giúp phân biệt "chân có đổi mức" với "NVIC có thực sự enable".
- **Memory view tại địa chỉ RCC->AHBENR** → xác nhận đúng bit `GPIOxEN` đã set, nếu nghi ngờ lỗi ở
  Phần 2 (quên enable clock).

## Phần 23 — 18 lỗi GPIO thường gặp

1. **Quên enable GPIO clock** — đã phân tích chi tiết ở file 02; hậu quả: ghi/đọc GPIO không có tác
   dụng, không crash, chỉ trả sai dữ liệu (lỗi âm thầm, khó phát hiện).
2. **Sai pin** — nhầm `GPIO_PIN_5` với số `5` (xem file 03) → cấu hình nhầm bit 0 và bit 2.
3. **Sai GPIO port** — gọi `HAL_GPIO_Init(GPIOA, ...)` cho pin thực tế nằm trên GPIOB.
4. **Pin floating** — xem file 04; đặc biệt nguy hiểm nếu floating input lại gắn ngắt.
5. **Sai pull-up/pull-down** — mạch ngoài thiết kế cho pull-down nhưng code set `GPIO_PULLUP` → đọc
   sai mức "mặc định" khi mạch ngoài đang tri-state.
6. **Output contention** (đã phân tích ở file 05) — 2 output push-pull nối chung dây, mức khác nhau →
   dòng điện lớn bất thường, có thể hỏng chip.
7. **Sai initial level** — như phân tích ở file 07: nếu mặc định `ODR=0` (LOW) không trùng với "mức an
   toàn" mong muốn, chân có thể thoáng qua mức sai ngay lúc `MODER` chuyển thành Output, trước khi code
   kịp gọi `WritePin` để sửa lại.
8. **Interrupt không vào** — nguyên nhân thường gặp: quên `HAL_NVIC_EnableIRQ()`, hoặc
   `SYSCFG->EXTICR` map sai Port cho line đó (ví dụ cấu hình ngắt cho PA1 nhưng một đoạn code khác sau
   đó lại cấu hình PB1 với cùng Mode IT — PB1 sẽ "cướp" line 1, EXTICR bị ghi đè sang GPIOB).
9. **Interrupt vào liên tục** — thường do (a) quên `__HAL_GPIO_EXTI_CLEAR_IT()` (nhưng `HAL_GPIO_EXTI_IRQHandler()` trong HAL đã tự làm điều này — xem file 06 — nên lỗi này chỉ xảy ra nếu bạn viết ISR tay không qua HAL), hoặc (b) floating input (#4) gây đổi mức giả liên tục.
10. **Bouncing (dội phím/dội cơ khí)** — `[GENERAL MCU KNOWLEDGE]` khi một tiếp điểm cơ khí (công tắc,
    relay) đóng/mở, điện áp **không chuyển mức sạch 1 lần** mà dao động rất nhanh qua lại vài lần trong
    khoảng vài ms trước khi ổn định — nếu dùng ngắt edge trực tiếp, 1 lần bấm thật có thể sinh ra
    **nhiều ngắt giả**. `[SOURCE]` Project này **đã giải quyết đúng vấn đề này một cách tường minh** —
    có cả một cơ chế tên thẳng là "anti-chattering" (チャタリング = tiếng Nhật cho "chattering/bouncing"):
    mỗi lần ngắt (`MSW_ON`, `POWER_MONITOR`, `MONI_24V11`) chỉ **khởi động một timer** (`start_anti_chattering()`,
    [km_it.c:489-501](../../pscpu_s800/main/App/km_it.c#L489-L501)), và logic thật (ví dụ `_MC_P_ON_OK()` đã
    đọc ở câu hỏi trước) chỉ tin vào mức tín hiệu **sau khi timer đó hết hạn mà không có thêm edge nào
    khác xảy ra** (`IS_CHECKING_CHATTERING()` — true nghĩa là "còn đang trong giai đoạn chờ ổn định,
    chưa được tin"). Đây là ví dụ thực hành chuẩn (debounce bằng timer) áp dụng ngay trong code bạn
    đang học.
11. **Sai EXTI mapping** — xem #8; triệu chứng: ngắt báo đúng line nhưng callback nhận nhầm
    `GPIO_Pin` tương ứng Port khác với Port thật bạn mong đợi.
12. **Sai NVIC enable** — quên gọi `HAL_NVIC_EnableIRQ(EXTI4_15_IRQn)` dù đã cấu hình `GPIO_MODE_IT_...`
    đúng → pending bit lên 1 nhưng CPU **không bao giờ** nhảy vào ISR (EXTI vẫn "biết" có sự kiện, chỉ
    là NVIC không được phép báo cho CPU).
13. **Sai IRQ Handler** — viết nhầm tên hàm (ví dụ gõ sai `EXTI0_1_IRQHandler` thành
    `EXTI01_IRQHandler`) → linker vẫn build thành công (vì đây chỉ là 1 hàm C bình thường), nhưng Vector
    Table (định nghĩa trong `startup_stm32f0xx.s`) vẫn trỏ tới **hàm handler mặc định rỗng** (weak alias),
    hàm bạn viết **không bao giờ được gọi** — lỗi rất khó phát hiện vì không có cảnh báo compile.
14. **Clear pending flag sai thời điểm/sai cách** — nếu tự viết ISR tay (không qua HAL) và quên clear
    pending bit, hoặc clear **trước khi** đọc xong dữ liệu liên quan → có thể miss 1 edge xảy ra đúng
    lúc đang xử lý (nếu line đó cho phép edge tiếp theo set lại pending ngay, dữ liệu cũ đã bị ghi đè).
15. **Race condition giữa ISR và main loop** — ví dụ nếu code chính đọc `g_io_extend_memory[...]`
    (biến được cả ISR và main loop cùng truy cập, như đã thấy ở `km_extend_io.c` các câu hỏi trước)
    **không qua cơ chế khóa/tắt ngắt tạm thời**, có thể đọc được giá trị nửa-cập-nhật. `[SOURCE]` Chính
    project này đã implement cơ chế `get_pending_factor()` / `put_pending_factor()` (xem
    [km_it.c:226-234](../../pscpu_s800/main/App/km_it.c#L226-L234)) — tên hàm gợi ý một cặp
    lock/unlock (có thể tắt/mở ngắt tạm thời bên trong) để tránh đúng loại race condition này khi đọc
    "pending factor bit".
16. **GPIO bị peripheral khác chiếm do Alternate Function** — ví dụ nếu code vô tình cấu hình lại
    `PA9`/`PA10` (đang dùng `GPIO_MODE_INPUT` làm input dự phòng) thành `GPIO_MODE_AF_PP` với
    `Alternate = GPIO_AF1_USART1` ở đâu đó khác trong code → 2 chân này sẽ bị USART1 "chiếm", input
    thường sẽ ngưng hoạt động như ý ban đầu.
17. **Glitch khi chuyển Input → Output** — đã phân tích chi tiết ở file 07 (Phần 15): nếu `ODR` đang
    giữ giá trị khác mong muốn từ trước, chân có thể xuất ra mức sai trong khoảnh khắc `MODER` vừa đổi.
18. **Điện áp thực tế không phù hợp với MCU** — `[GENERAL MCU KNOWLEDGE]` STM32F031 chạy ở 3.3V; nếu
    tín hiệu input bên ngoài dùng mức 5V (ví dụ một số IC logic cũ) mà không có mạch chia áp/level
    shifter, có thể **vượt ngưỡng chịu đựng tối đa (Absolute Maximum Rating)** của chân input, gây hỏng
    chip vĩnh viễn — đây là lý do bắt buộc phải tra datasheet điện áp max cho từng chân trước khi nối
    mạch ngoài, **source code hoàn toàn không cho biết thông tin này**.

Tiếp theo: [11_schematic_va_tai_lieu_khac.md](11_schematic_va_tai_lieu_khac.md).
