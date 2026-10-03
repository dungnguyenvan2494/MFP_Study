# Phần 5-6: `GPIO_MODE_INPUT`, Floating, Pull-up/Pull-down

## LEVEL 1-3 — `GPIO_MODE_INPUT` hoạt động thế nào

`[SOURCE]` [stm32f0xx_hal_gpio.h:135](../../common/Drivers/STM32F0xx_HAL_Driver/Inc/stm32f0xx_hal_gpio.h#L135):
```c
#define GPIO_MODE_INPUT  ((uint32_t)0x00000000)   /* Input Floating Mode */
```
Giá trị `0x0` này được ghi vào 2 bit tương ứng trong `MODER` (xem file 09) — nghĩa là output driver
**bị tắt hoàn toàn**, chỉ còn input buffer hoạt động.

**Luồng điện áp → bit CPU đọc được**:
```text
Physical voltage (điện áp thật trên chân, ví dụ 0V hoặc 3.3V)
 ↓
Input buffer (Schmitt trigger — so sánh với 2 ngưỡng điện áp Vth_low/Vth_high, không phải 1 ngưỡng duy
               nhất, để tránh đọc nhiễu loạn khi điện áp dao động gần ngưỡng — gọi là "hysteresis")
 ↓
GPIO input logic (chốt kết quả so sánh thành 1 bit ổn định)
 ↓
GPIO IDR register (Input Data Register — mỗi bit ứng với 1 chân trong Port)
 ↓
CPU đọc bit đó qua HAL_GPIO_ReadPin()
```

`[GENERAL MCU KNOWLEDGE]` Khi pin ≈ 3.3V (mức HIGH điện) → CPU đọc `GPIO_PIN_SET` (logic 1).
Khi pin ≈ 0V (mức LOW điện) → CPU đọc `GPIO_PIN_RESET` (logic 0). **Ngưỡng điện áp chính xác phân
biệt HIGH/LOW phụ thuộc electrical specification của từng chip** (ví dụ với STM32F0 chạy ở 3.3V, theo
tiêu chuẩn CMOS thường ngưỡng input HIGH tối thiểu khoảng 0.7×VDD, ngưỡng LOW tối đa khoảng 0.3×VDD —
`[GENERAL MCU KNOWLEDGE]`, con số chính xác **phải tra datasheet điện của STM32F031**, không được suy
diễn từ source code vì source code không chứa thông tin analog này).

Xác nhận bằng code thật — `HAL_GPIO_ReadPin()`:
`[SOURCE]` [stm32f0xx_hal_gpio.c:394-410](../../common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c#L394-L410):
```c
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  if ((GPIOx->IDR & GPIO_Pin) != (uint32_t)GPIO_PIN_RESET) return GPIO_PIN_SET;
  else return GPIO_PIN_RESET;
}
```
→ Đây chính xác là bước cuối cùng của sơ đồ trên: đọc trực tiếp bit trong `IDR`.

## Pull-up / Pull-down / Floating — bằng mạch điện

`[SOURCE]` [stm32f0xx_hal_gpio.h:166-168](../../common/Drivers/STM32F0xx_HAL_Driver/Inc/stm32f0xx_hal_gpio.h#L166-L168):
```c
#define GPIO_NOPULL    0x00000000
#define GPIO_PULLUP    0x00000001
#define GPIO_PULLDOWN  0x00000002
```

### Floating input (`GPIO_NOPULL`) — tại sao nguy hiểm?

```text
          GPIO pin ───────── (không nối gì cả nếu bên ngoài cũng không nối chắc)
```
`[GENERAL MCU KNOWLEDGE]` Nếu chân input **không được pull lên hoặc xuống**, và bên ngoài cũng không
có mạch nào ép mức điện áp rõ ràng (ví dụ cảm biến đang ở trạng thái "tri-state" hoặc dây chưa cắm),
điện áp trên chân sẽ **trôi tự do** theo nhiễu điện từ môi trường (EMI từ các mạch lân cận, tĩnh điện,
dao động nguồn...) — Input buffer có thể đọc ra 0 hoặc 1 **ngẫu nhiên, đổi liên tục**, dù không có gì
thay đổi về mặt vật lý thật. Đây gọi là **floating input** — rất nguy hiểm nếu input đó gắn với ngắt
(interrupt), vì có thể gây **ngắt giả liên tục** dù không ai bấm/đổi gì.

### Pull-up resistor

```text
        VCC (3.3V)
         |
         R  (điện trở nội, thường vài chục kΩ)
         |
        GPIO pin ──────── external signal (cảm biến/nút bấm bên ngoài)
```
Khi external signal **không kéo gì** (tri-state/hở mạch), điện trở R kéo chân về **gần VCC** → CPU đọc
ổn định là HIGH. Khi external signal **chủ động kéo xuống GND** (ví dụ nút bấm nối GND), chân xuống
LOW — vì R rất lớn so với điện trở của mạch kéo xuống, dòng điện qua R rất nhỏ, không "chống" lại được.

### Pull-down resistor

```text
        GPIO pin ──────── external signal
         |
         R
         |
        GND
```
Ngược lại: khi không có gì kéo, chân bị R kéo về **gần GND** → đọc LOW ổn định. Khi external signal
chủ động kéo lên VCC, chân lên HIGH.

### Khi nào dùng pull-up / pull-down?

`[GENERAL MCU KNOWLEDGE]`
- Dùng khi tín hiệu bên ngoài là **open-drain/open-collector** hoặc **nút bấm cơ khí** (chỉ chủ động
  kéo về 1 mức, mức còn lại phải nhờ resistor) — ví dụ I2C (xem file 05), hoặc nút nhấn nối GND khi bấm.
- Chọn **pull-up** nếu "trạng thái nghỉ/mặc định an toàn" của hệ thống là HIGH, **pull-down** nếu
  trạng thái nghỉ an toàn là LOW.
- Giá trị điện trở nội **không cố định tuyệt đối theo ý người dùng** — nó là một giá trị **cố định do
  nhà sản xuất thiết kế trên silicon** (dao động trong một khoảng theo datasheet, ví dụ thường
  30kΩ-50kΩ cho STM32, không chỉnh được chính xác). Nếu ứng dụng cần giá trị điện trở chính xác hoặc
  dòng pull mạnh hơn khả năng của điện trở nội, phải dùng **external resistor** (điện trở rời ngoài
  board) thay thế hoặc bổ sung.

### Áp dụng vào `PA9_IN_Pin | PA10_IN_Pin` với `GPIO_PULLDOWN`

```c
GPIO_InitStruct.Pin = PA9_IN_Pin|PA10_IN_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
GPIO_InitStruct.Pull = GPIO_PULLDOWN;
```
`[SOURCE]` [mx_init.c:479-483](../../pscpu_s800/main/Src/mx_init.c#L479-L483)

Comment trong tài liệu dự án (đã đọc ở câu hỏi trước) cho biết 2 chân này **về vật lý hỗ trợ
USART1_TX/RX** nhưng thiết kế **không dùng UART**, chỉ để input dự phòng.
`[INFERENCE]` Lý do hợp lý để chọn `PULLDOWN` (chưa xác nhận bằng schematic): khi không có gì nối vào
2 chân này (board không gắn thêm module UART debug), designer muốn trạng thái mặc định của 2 input dự
phòng này là **LOW ổn định** (không floating, không gây đọc nhầm/ngắt giả nếu sau này lại cấu hình
thành input có ngắt), thay vì để trôi tự do. Đây là suy luận dựa trên tên signal và comment có sẵn,
**cần schematic thật để xác nhận** có mạch ngoài nào nối vào PA9/PA10 hay không.

## Lỗi thường gặp

- Để input floating (`GPIO_NOPULL`) trên chân **không có gì nối bên ngoài** → đọc giá trị ngẫu nhiên,
  đặc biệt nguy hiểm nếu chân đó cấu hình thêm ngắt (`GPIO_MODE_IT_...`) → ngắt nổ liên tục không lý do
  (xem thêm Part 23 ở file 10).
- Chọn pull-up/pull-down **ngược** với thiết kế mạch ngoài → mạch ngoài phải "chiến đấu" liên tục với
  điện trở nội, tiêu tốn điện không cần thiết, hoặc đọc sai mức khi mạch ngoài yếu hơn điện trở nội.

Tiếp theo: [05_output_pushpull_opendrain_speed.md](05_output_pushpull_opendrain_speed.md).
