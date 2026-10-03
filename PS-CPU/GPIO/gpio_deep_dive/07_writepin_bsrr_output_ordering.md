# Phần 14-15: `HAL_GPIO_WritePin()`, BSRR vs ODR, và thứ tự Init → Set Level

## `HAL_GPIO_WritePin()` làm gì, ghi vào register nào?

```c
HAL_GPIO_WritePin(GPIOB, IR_P_ON_Pin|ERP_SENSOR_ON_Pin|...|_RST_SLP2_Pin, GPIO_PIN_RESET);
```
`[SOURCE]` [mx_init.c:504-506](../../pscpu_s800/main/Src/mx_init.c#L504-L506)

`[SOURCE]` Implementation thật:
[stm32f0xx_hal_gpio.c:427-441](../../common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c#L427-L441):
```c
void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
  if (PinState != GPIO_PIN_RESET) {
    GPIOx->BSRR = (uint32_t)GPIO_Pin;      // muốn SET (HIGH) → ghi vào nửa thấp của BSRR
  } else {
    GPIOx->BRR  = (uint32_t)GPIO_Pin;      // muốn RESET (LOW) → ghi vào BRR
  }
}
```
(Trên STM32F0, `BRR` là một thanh ghi **riêng** cho reset — tương đương ghi vào nửa cao (bit 16-31)
của `BSRR` trên các dòng STM32 khác không có `BRR` riêng; về bản chất mạch là như nhau.)

## Vì sao HAL dùng `BSRR`/`BRR` thay vì `GPIOx->ODR = ...` trực tiếp?

`[GENERAL MCU KNOWLEDGE]` `ODR` (Output Data Register) là thanh ghi **đọc-sửa-ghi được** (bạn có thể
`GPIOx->ODR |= PIN` để set 1 bit mà không đổi các bit khác) — nhưng chính vì nó đọc-sửa-ghi, nó **không
atomic**: giữa lúc CPU đọc `ODR` và lúc ghi lại, nếu một **ngắt xảy ra giữa 2 bước đó** và ISR cũng ghi
vào `ODR` (một bit khác), giá trị ISR vừa ghi **sẽ bị ghi đè mất** khi code chính hoàn thành bước ghi
lại của nó (dựa trên giá trị đọc cũ, chưa có thay đổi của ISR).

```c
GPIOB->ODR |= PIN_A;      // (1) đọc ODR hiện tại, (2) OR với PIN_A, (3) ghi lại — 3 bước, không atomic
```
```text
Thời điểm t0: ODR = 0b0000
Code chính:    đọc ODR → 0b0000                         (bước 1)
   ↓ bị ngắt ở đây! ISR chạy: ODR |= PIN_B → ODR = 0b0010
Code chính:    tiếp tục, OR với PIN_A → 0b0001, GHI LẠI ODR = 0b0001   (bước 3, dựa trên giá trị CŨ)
Kết quả: PIN_B bị ISR set nhưng lại bị code chính XÓA MẤT — race condition!
```

## BSRR giải quyết thế nào — "atomic" nghĩa là gì?

```c
GPIOB->BSRR = PIN_A;      // chỉ 1 lệnh ghi duy nhất
```
`[GENERAL MCU KNOWLEDGE]` `BSRR` (Bit Set/Reset Register) được thiết kế đặc biệt: **ghi 1 vào 1 bit
của BSRR chỉ tác động đúng 1 bit tương ứng trong ODR**, các bit khác của ODR **hoàn toàn không bị
đụng tới** — phần cứng tự làm việc "chỉ sửa đúng bit cần, giữ nguyên các bit còn lại" bằng mạch logic
tổ hợp, **không cần CPU đọc ODR trước**. Vì chỉ có **1 lệnh ghi duy nhất** (không có bước đọc), không
có "khoảng hở" giữa đọc và ghi để một ngắt có thể chen vào và làm mất dữ liệu → đây gọi là
**atomic (nguyên tử) theo nghĩa không thể bị chia cắt giữa chừng**.

| | `GPIOx->ODR |= PIN` | `GPIOx->BSRR = PIN` |
|---|---|---|
| Số bước CPU | 3 (đọc, sửa, ghi) | 1 (chỉ ghi) |
| Có an toàn khi ISR cũng ghi ODR không? | **Không** — có thể mất dữ liệu (race condition) | **Có** — mỗi bit độc lập hoàn toàn |
| Có ảnh hưởng tới các bit pin khác không liên quan? | Không (nếu code đúng logic OR) | Không |

→ Đây là lý do **mọi HAL_GPIO_WritePin() trên STM32 đều dùng BSRR/BRR**, không dùng `ODR = ...` trực
tiếp — an toàn tuyệt đối khi có ngắt chen vào, dù trong `MX_GPIO_Init()` cụ thể (code chạy tuần tự lúc
khởi động, chưa có ngắt nào khác đang chạy) sự khác biệt này **chưa thể hiện rủi ro thực tế**, nhưng
đây vẫn là lý do thiết kế chuẩn của HAL áp dụng nhất quán cho mọi lời gọi `WritePin` trong toàn bộ
project, bao gồm cả những lời gọi trong `power_monitor_proc()`, `km_exti_callback()`... (các hàm được
gọi từ context ngắt hoặc polling loop đan xen).

## Thứ tự: Init trước, rồi mới Set Output Level — tại sao?

```c
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);   // (1) cấu hình MODER/OTYPER/OSPEEDR/PUPDR — CHƯA đổi ODR
...
HAL_GPIO_WritePin(GPIOB, IR_P_ON_Pin|...|_RST_SLP2_Pin, GPIO_PIN_RESET);  // (2) set mức logic mong muốn
```
`[SOURCE]` [mx_init.c:461-465](../../pscpu_s800/main/Src/mx_init.c#L461-L465) (Init) và
[mx_init.c:504-506](../../pscpu_s800/main/Src/mx_init.c#L504-L506) (WritePin) — **đáng chú ý: toàn bộ
các lời gọi `HAL_GPIO_Init()` chạy xong hết trước, rồi mới tới 2 lời gọi `HAL_GPIO_WritePin()` ở cuối
hàm** — không đặt ngay sau từng `Init()` riêng lẻ.

### GPIO output mặc định (reset state) là gì?

`[GENERAL MCU KNOWLEDGE]` Sau khi chip power-on hoặc reset, **toàn bộ `ODR` mặc định = 0** (tất cả
pin ở mức LOW nếu/khi chúng trở thành output), và `MODER` mặc định = Input (Analog trên một số dòng
khác, nhưng STM32F0 reset về input floating) — nghĩa là: **trước khi `HAL_GPIO_Init()` chạy, mọi chân
đều đang là input**, chưa chân nào là output để "mức điện áp sai" có thể xảy ra.

### Có nguy cơ glitch không? — Phân tích thứ tự thật của code này

`[GENERAL MCU KNOWLEDGE]` Về lý thuyết, khi `HAL_GPIO_Init()` đổi `MODER` từ Input sang Output
(`GPIO_MODE_OUTPUT_PP`), chân sẽ **ngay lập tức xuất ra giá trị đang có sẵn trong `ODR` lúc đó**
(không phải giá trị bạn "định" set sau — vì `HAL_GPIO_Init()` **không đụng vào `ODR`**, chỉ đổi
`MODER`/`OTYPER`/`OSPEEDR`/`PUPDR`, xem lại file
[09_hal_to_register_trace.md](09_hal_to_register_trace.md) để thấy rõ `HAL_GPIO_Init()` không có dòng
nào ghi `ODR`/`BSRR`).

**Áp dụng vào project**: vì `ODR` reset mặc định = 0 (tất cả bit = LOW), và các pin trong nhóm
`MX_GPIO_Init()` dòng 455-460 đều được set `GPIO_PIN_RESET` ở bước (2) — **giá trị mặc định (0) và giá
trị mong muốn (RESET=LOW=0) trùng nhau**. Do đó, trong trường hợp cụ thể này, **việc gọi
`HAL_GPIO_WritePin(..., GPIO_PIN_RESET)` sau `Init()` không thực sự "sửa" gì khác so với lúc chuyển
mode** — nó chỉ là bước "nói rõ ý định" (explicit, cho dễ đọc code / phòng trường hợp `ODR` từng bị
thay đổi trước đó bởi code khác) hơn là một thao tác bắt buộc để tránh glitch trong chính đoạn code
này.

**Khi nào thứ tự này THỰC SỰ cần cẩn thận (tổng quát, không riêng đoạn code này)?**
`[GENERAL MCU KNOWLEDGE]` Nếu bạn muốn một chân output khởi động ở mức **HIGH** (không phải LOW mặc
định), và việc chân đó "thoáng qua mức LOW" dù chỉ vài chu kỳ clock có thể gây hại (ví dụ chân điều
khiển relay công suất, hoặc chân Enable của một IC nhạy cảm) — thì **nên set `ODR`/`BSRR` trước khi
đổi `MODER` thành Output**, để khi chân "bật" thành output, nó xuất ra ngay giá trị đúng, không đi
qua trạng thái LOW mặc định dù chỉ một khoảnh khắc. HAL cung cấp cách làm điều này bằng cách gọi
`HAL_GPIO_WritePin()` **trước** `HAL_GPIO_Init()` (ghi `BSRR` không yêu cầu chân đã là output — chỉ
không có tác dụng quan sát được bên ngoài cho đến khi `MODER` đổi thành output).

### Liên hệ với các signal điều khiển nguồn trong project

`[INFERENCE]` Nhìn vào nhóm pin `MC_PWR_EN`, `SB_PWR_EN`, `MC_P_ON`, `IR_P_ON`, `_RESET`,
`_RST_SLP2`, `_USB2_OE` — tất cả đều **mong muốn mặc định là LOW/RESET lúc khởi động** (tắt nguồn,
giữ reset), và điều này **khớp hoàn toàn với giá trị reset mặc định của `ODR` (=0)** — nghĩa là thiết
kế này **không cần** lo ngại thứ tự set-before-init, vì "trạng thái an toàn nhất lúc khởi động" trùng
với "trạng thái mặc định của phần cứng". Đây là một lựa chọn thiết kế hợp lý và phổ biến cho các tín
hiệu enable/reset tích cực mức thấp hoặc để nguyên tắc "fail-safe = mọi thứ tắt khi không chắc chắn"
— nhưng đây là suy luận từ việc đọc tên signal và logic xử lý ở `km_extend_io.c`, không có comment
nào trong `mx_init.c` giải thích trực tiếp lý do.

## Lỗi thường gặp

- Giả định `HAL_GPIO_Init()` sẽ tự set `ODR` theo "mức an toàn" nào đó — **sai**, nó hoàn toàn không
  đụng tới `ODR`/`BSRR`. Nếu bạn cần mức cụ thể lúc chân vừa chuyển thành output, phải tự gọi
  `WritePin` (và cân nhắc gọi **trước** `Init` nếu mức LOW mặc định có thể gây hại).
- Dùng `GPIOx->ODR |= PIN` trong code có ngắt chạy đồng thời ghi `ODR` khác → race condition hiếm gặp,
  khó debug (chỉ xảy ra khi đúng thời điểm ngắt chen giữa đọc và ghi).

Tiếp theo: [08_toan_bo_source_phan_tich.md](08_toan_bo_source_phan_tich.md) — nhóm toàn bộ pin theo
chức năng và phân tích đoạn cấu hình trùng mà bạn thắc mắc.
