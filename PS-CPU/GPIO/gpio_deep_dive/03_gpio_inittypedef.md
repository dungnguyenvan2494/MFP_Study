# Phần 4: `GPIO_InitTypeDef` — struct cấu hình và bitmask `Pin`

## Vì sao HAL dùng struct thay vì gọi hàm với nhiều tham số?

```c
GPIO_InitTypeDef GPIO_InitStruct;
```
`[SOURCE]` struct định nghĩa tại
[stm32f0xx_hal_gpio.h:65-82](../../common/Drivers/STM32F0xx_HAL_Driver/Inc/stm32f0xx_hal_gpio.h#L65-L82):
```c
typedef struct {
  uint32_t Pin;        /* Chân nào (bitmask, có thể nhiều chân cùng lúc) */
  uint32_t Mode;        /* Input / Output_PP / Output_OD / IT_Rising / IT_Falling / ... */
  uint32_t Pull;        /* NoPull / PullUp / PullDown */
  uint32_t Speed;       /* chỉ dùng khi Mode là Output/AF */
  uint32_t Alternate;    /* chỉ dùng khi Mode là AF (Alternate Function) */
} GPIO_InitTypeDef;
```

`[GENERAL MCU KNOWLEDGE]` Dùng struct giúp: (1) một hàm `HAL_GPIO_Init(Port, &struct)` xử lý được
**mọi tổ hợp cấu hình** mà không cần hàng chục hàm riêng; (2) code gọi có thể **tái sử dụng** struct
đã khai báo, chỉ đổi vài field rồi gọi lại — đúng như project này làm (xem cách biến `GPIO_InitStruct`
được dùng lại 7 lần liên tiếp trong `MX_GPIO_Init()`, mỗi lần đổi `Pin`/`Mode`/`Pull`/`Speed` rồi gọi
`HAL_GPIO_Init()`).

## Bảng ý nghĩa & ảnh hưởng hardware của từng field

| Field | Ý nghĩa | Ảnh hưởng hardware (ghi vào register nào) |
|---|---|---|
| `Pin` | Bitmask chọn (các) chân trong 1 Port | Không tự ghi register — chỉ dùng để tính vị trí bit khi set các field khác |
| `Mode` | Input/Output/IT/EVT/AF, và kiểu cạnh ngắt | `MODER` (hướng chân), và nếu có cờ ngắt: `SYSCFG->EXTICR`, `EXTI->IMR/EMR/RTSR/FTSR` |
| `Pull` | Pull-up/pull-down nội | `PUPDR` |
| `Speed` | Slew rate của driver output (chỉ áp dụng Output/AF) | `OSPEEDR` |
| `Alternate` | Chọn peripheral nào "mượn" chân này (UART, I2C...) | `AFR[0]`/`AFR[1]` |

(Trace chi tiết từng register này ở
[09_hal_to_register_trace.md](09_hal_to_register_trace.md).)

## `Pin` là bitmask như thế nào?

**Đây là điểm người mới dễ hiểu lầm nhất**: `Pin` **không phải** một số thứ tự (0, 1, 2, 5...) — nó là
một **bitmask 16-bit**, mỗi bit đại diện đúng 1 chân trong Port.

`[SOURCE]` [stm32f0xx_hal_gpio.h:102-118](../../common/Drivers/STM32F0xx_HAL_Driver/Inc/stm32f0xx_hal_gpio.h#L102-L118):
```c
#define GPIO_PIN_0   ((uint16_t)0x0001)   /* 0000 0000 0000 0001 — bit 0 */
#define GPIO_PIN_5   ((uint16_t)0x0020)   /* 0000 0000 0010 0000 — bit 5 */
#define GPIO_PIN_All ((uint16_t)0xFFFF)   /* tất cả 16 bit = 1 */
```

### Ví dụ thật: `GPIO_PIN_0 | GPIO_PIN_1` thực chất là gì?

```text
GPIO_PIN_0  = 0000 0000 0000 0001
GPIO_PIN_1  = 0000 0000 0000 0010
   OR (|)   -------------------------
  kết quả  = 0000 0000 0000 0011   = 0x0003
```
→ Phép `|` (OR theo bit) **gộp 2 bit riêng biệt thành 1 số duy nhất**, dùng để báo "cấu hình đồng thời
cả 2 chân này, với cùng Mode/Pull/Speed". Ví dụ thật trong project:

```c
GPIO_InitStruct.Pin = IR_P_ON_Pin|ERP_SENSOR_ON_Pin|_RESET_Pin|_USB2_OE_Pin
                     |SLEEP_STATUS_REM_EG_Pin|MC_PWR_EN_Pin|SB_PWR_EN_Pin|MC_P_ON_Pin
                     |_RST_SLP2_Pin;
```
`[SOURCE]` [mx_init.c:455-460](../../pscpu_s800/main/Src/mx_init.c#L455-L460) — **9 chân khác nhau**
trên cùng GPIOB được gộp vào **1 số nguyên 16-bit duy nhất** bằng phép OR, vì tất cả đều cần cấu hình
giống nhau: `OUTPUT_PP`, `NOPULL`, `SPEED_LOW`.

### HAL xử lý bitmask này thế nào? (preview, chi tiết ở file 09)

`[SOURCE]` [stm32f0xx_hal_gpio.c:201-204](../../common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c#L201-L204):
```c
while (((GPIO_Init->Pin) >> position) != RESET) {
    iocurrent = (GPIO_Init->Pin) & (1U << position);
    if (iocurrent) { /* ... cấu hình riêng cho bit "position" này ... */ }
    position++;
}
```
→ `HAL_GPIO_Init()` **lặp qua từng bit** (0 đến 15) của `Pin`, bit nào đang là 1 thì cấu hình chân đó.
Nghĩa là gọi 1 lần với 9 chân OR vào nhau **hoàn toàn tương đương** gọi 9 lần riêng lẻ với từng
`GPIO_PIN_x` — chỉ là viết gọn hơn, hiệu năng không đổi đáng kể (vẫn là vòng lặp 16 lần kiểm tra bit).

## Lỗi thường gặp liên quan đến `Pin`

- Dùng `=` thay vì `|=` khi muốn "thêm" 1 chân vào mask đã có → vô tình xóa các chân đã cấu hình trước
  (không áp dụng trong file này vì mỗi lần code đều gán mới `GPIO_InitStruct.Pin = ...` trước khi gọi
  `HAL_GPIO_Init()`, nhưng là lỗi runtime phổ biến nếu code logic viết tay reuse struct không cẩn thận).
- Nhầm `GPIO_PIN_5` (bitmask `0x0020`) với số `5` — nếu lỡ viết `GPIO_InitStruct.Pin = 5;` thì thực tế
  đang cấu hình **PIN 0 và PIN 2** (vì `5 = 0b0101` = bit 0 + bit 2), không phải pin số 5 như ý định.

Tiếp theo: [04_input_floating_pullup_pulldown.md](04_input_floating_pullup_pulldown.md).
