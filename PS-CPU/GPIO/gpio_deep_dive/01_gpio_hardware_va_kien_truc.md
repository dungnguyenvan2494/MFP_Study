# Phần 1-2: GPIO là gì từ góc nhìn phần cứng & vị trí trong kiến trúc MCU

## LEVEL 1 — GPIO là gì?

**Định nghĩa đơn giản**: GPIO (General Purpose Input/Output) là một **chân vật lý có thể lập trình**
để đọc điện áp vào (input) hoặc xuất điện áp ra (output), dưới sự điều khiển của phần mềm.
"General Purpose" nghĩa là chân này **không có chức năng cố định** — nó có thể là input, output,
hoặc nhường quyền điều khiển cho một peripheral khác (UART, I2C, Timer...) tùy cấu hình.

**Ví dụ đời thường**: hãy nghĩ GPIO như một **ổ điện đa năng trong nhà** có thể chuyển giữa 2 chế độ
bằng một công tắc gạt: chế độ "cấp điện ra" (output — bạn cắm đèn vào, đèn sáng theo ý bạn) hoặc chế độ
"đo điện áp vào" (input — bạn cắm một cảm biến vào, đọc xem nó đang báo cao hay thấp).

**Ví dụ phần cứng thật trong project**: `MC_P_ON_Pin` (PB5) được cấu hình là **output** — PS-CPU dùng
nó để "bật/tắt" nguồn khối Main Controller. `POWER_MONITOR_Pin` (PA4) được cấu hình là **input có ngắt**
— PS-CPU dùng nó để "nghe" xem nguồn chính có bị mất điện không.
`[SOURCE]` [mxconstants.h:54,92](../../pscpu_s800/main/Inc/mxconstants.h)

## LEVEL 2 — Một chân GPIO vật lý gồm những khối nào?

`[GENERAL MCU KNOWLEDGE]` Về bản chất silicon, mỗi chân GPIO là một mạch logic nhỏ gồm:

```
                     ┌──────────────────────┐
  Physical Pin ──────┤  ESD protection diode  │
       │             └──────────────────────┘
       │
       ├──────► Input buffer (Schmitt trigger) ──► bit trong IDR (đọc được bằng CPU)
       │
       ├──────► Output driver (2 transistor: HIGH-side + LOW-side) ◄── bit trong ODR/BSRR (CPU ghi)
       │
       └──────► Pull-up / Pull-down resistor nội (có thể bật/tắt bằng PUPDR)
```

- **Input buffer**: mạch khuếch đại/so sánh, chuyển điện áp analog liên tục trên chân thành
  1 bit số (0 hoặc 1) để CPU đọc được.
- **Output driver**: 2 transistor (hoặc tương đương) — một kéo chân lên VDD (HIGH), một kéo chân
  xuống GND (LOW). Chi tiết ở [05_output_pushpull_opendrain_speed.md](05_output_pushpull_opendrain_speed.md).
- **Pull resistor nội**: một điện trở rất nhỏ (thường vài chục kΩ) có thể bật ngầm bên trong chip,
  không cần linh kiện ngoài.

## LEVEL 3 — GPIO Input hoạt động thế nào?

Khi bạn cấu hình pin là Input: Output driver bị tắt hoàn toàn (không ảnh hưởng điện áp pin), chỉ còn
Input buffer hoạt động — nó liên tục đọc điện áp vật lý trên chân và cập nhật 1 bit tương ứng trong
thanh ghi `IDR` (Input Data Register). CPU đọc thanh ghi này bất cứ lúc nào muốn biết trạng thái pin.

## LEVEL 4 — GPIO Output hoạt động thế nào?

Khi bạn cấu hình pin là Output: CPU ghi 1 bit vào thanh ghi `ODR` (hoặc dùng `BSRR` — xem phần 7) →
Output driver đọc bit đó và **chủ động** kéo chân lên VDD hoặc xuống GND. Lúc này Input buffer vẫn có
thể đọc lại được giá trị đó qua `IDR` (self-read-back), nhưng ý nghĩa thực sự đến từ việc CPU đang
**ép** điện áp ra ngoài, không phải đo từ bên ngoài.

## LEVEL 5 — GPIO peripheral khác CPU như thế nào?

`[GENERAL MCU KNOWLEDGE]` CPU (lõi Cortex-M0) chỉ biết tính toán và thực thi lệnh — nó **không có khả
năng vật lý** để tạo ra điện áp trên một chân chip. GPIO là một **peripheral riêng** (một mạch cứng
độc lập, nằm ngoài lõi CPU, kết nối qua bus nội bộ) — CPU "nói chuyện" với GPIO bằng cách đọc/ghi vào
các thanh ghi (MODER, ODR, IDR...) được "map" vào không gian địa chỉ bộ nhớ (xem Level 2 ở file
[02_rcc_clock_enable.md](02_rcc_clock_enable.md) về memory-mapped I/O). Nói cách khác:
**CPU điều khiển GPIO gián tiếp qua bus, không trực tiếp chạm vào chân vật lý.**

## LEVEL 6 — GPIO Port A/B/C/F nghĩa là gì?

`[GENERAL MCU KNOWLEDGE]` Một chip STM32 có **hàng chục chân vật lý**, nhưng để quản lý dễ, nhà sản
xuất nhóm chúng thành các **Port**, mỗi Port có tối đa 16 chân (Pin 0 đến Pin 15), mỗi Port là **một
peripheral GPIO độc lập** với bộ thanh ghi riêng (`GPIOA`, `GPIOB`, `GPIOC`, `GPIOF`...).

```
GPIO peripheral (khối tổng)
 ├─ GPIOA  (peripheral con, có bộ thanh ghi MODER/ODR/IDR... riêng)
 │    ├─ PA0, PA1, PA2, ... PA15
 ├─ GPIOB
 │    ├─ PB0, PB1, PB2, ... PB15
 ├─ GPIOC   (project này chỉ dùng 1 pin: WAKEUP_Pin = PC13)
 └─ GPIOF   (chỉ có PF0, PF1 trên chip nhỏ như STM32F031 — không đủ 16 chân vật lý)
```

**Tại sao chip này có GPIOC, GPIOF nhưng project chỉ dùng vài pin của chúng?**
`[SOURCE]` STM32F031C6Tx là chip **48-pin (LQFP48)**, không đủ chân vật lý để lộ hết 16×4 = 64 pin GPIO
ra ngoài — nhiều bit trong các thanh ghi GPIOC/GPIOF tồn tại trên silicon nhưng không nối ra chân nào
cả (gọi là "not bonded"), nên project chỉ cấu hình đúng những pin thực sự có chân vật lý
(`WAKEUP_Pin`=PC13, `PF0_IN_Pin`/`PF1_IN_Pin`=PF0/PF1).
[mxconstants.h:44-49](../../pscpu_s800/main/Inc/mxconstants.h)

## LEVEL 7 — GPIO pin number là gì?

Là **số thứ tự của chân trong 1 Port cụ thể** — một con số từ 0 đến 15, không phải số chân vật lý trên
package chip (ví dụ chân vật lý số 14 trên package LQFP48 mới là "PA2", không phải "pin 14" theo nghĩa
GPIO). Mỗi macro `GPIO_PIN_x` trong HAL thực chất là **một bitmask** (ví dụ `GPIO_PIN_5` = `0x0020` =
bit thứ 5), không phải con số thứ tự thường — chi tiết ở
[03_gpio_inittypedef.md](03_gpio_inittypedef.md).

### Ví dụ thật trong project

| Tên trong code | Port | Pin number | Ý nghĩa (theo code) |
|---|---|---|---|
| `PA0` (không dùng trực tiếp — chỉ minh họa) | GPIOA | 0 | — |
| `_HRESET_REQ_Pin` | GPIOA | 1 | Input ngắt falling-edge — yêu cầu reset từ host |
| `MC_P_ON_Pin` | GPIOB | 5 | Output — cấp nguồn khối Main Controller |
| `PF0_IN_Pin` | GPIOF | 0 | Input thường |
| `PF1_IN_Pin` | GPIOF | 1 | Input thường |

`[SOURCE]` [mxconstants.h:46-97](../../pscpu_s800/main/Inc/mxconstants.h)

## Phân biệt 5 khái niệm hay bị nhầm

| Khái niệm | Nghĩa là gì | Ví dụ |
|---|---|---|
| **GPIO Port** | Một nhóm tối đa 16 pin, có **1 bộ thanh ghi peripheral riêng** | `GPIOB` (cả khối) |
| **GPIO Pin** | Một bit cụ thể trong Port đó, đại diện 1 chân | `GPIO_PIN_5` (bit số 5 trong GPIOB) |
| **GPIO peripheral** | Mạch cứng thực thi chức năng GPIO cho 1 Port — tồn tại độc lập, có clock riêng (xem phần 3) | "GPIOB peripheral" |
| **Physical pin** | Chân kim loại thật trên package chip, có thể mang nhiều chức năng (GPIO, hoặc Alternate Function như UART/I2C) | Chân vật lý số 23 trên LQFP48 = PB5 |
| **MCU package pin** | Đồng nghĩa physical pin, nhấn mạnh góc nhìn "đóng gói" (package) của chip, có số thứ tự riêng trên datasheet (pin 1, pin 2... pin 48) | — |

**Lưu ý quan trọng**: 1 physical pin có thể **không chỉ** là GPIO — nó có thể được "multiplex" (dùng
chung) với chức năng khác (UART_TX, I2C_SCL...) qua cơ chế Alternate Function (AF). Ví dụ trong project
này, `PA9_IN_Pin`/`PA10_IN_Pin` dù về mặt vật lý hỗ trợ `USART1_TX/RX` theo datasheet, nhưng được cấu
hình là `GPIO_MODE_INPUT` thường — nghĩa là thiết kế **chủ động không dùng** UART trên 2 chân này.
`[SOURCE]` [mx_init.c:479-483](../../pscpu_s800/main/Src/mx_init.c#L479-L483)

## Sơ đồ tổng quan

```text
CPU (Cortex-M0)
 |
 | Bus (AHB — xem file 02)
 v
GPIO Peripheral (khối tổng, gồm nhiều Port)
 |
 +---- GPIOA peripheral (clock riêng: RCC_AHBENR_GPIOAEN)
 |      +--- PA0 .. PA15  (một số không nối ra chân vật lý)
 |
 +---- GPIOB peripheral (clock riêng: RCC_AHBENR_GPIOBEN)
 |      +--- PB0 .. PB15
 |
 +---- GPIOC peripheral  (chỉ PC13 được dùng: WAKEUP_Pin)
 |
 +---- GPIOF peripheral  (chỉ PF0, PF1 được dùng)
```

Tiếp theo: [02_rcc_clock_enable.md](02_rcc_clock_enable.md) — tại sao phải "cấp điện" (bật clock) cho
từng Port *trước khi* cấu hình pin.
