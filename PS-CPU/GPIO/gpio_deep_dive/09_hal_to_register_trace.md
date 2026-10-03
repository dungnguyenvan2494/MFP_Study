# Phần 20-21: Trace đầy đủ HAL → Register → Hardware, và sơ đồ kiến trúc tổng thể

Đây là phần quan trọng nhất theo yêu cầu của bạn. Lấy ví dụ cụ thể từ chính project:

```c
GPIO_InitStruct.Pin = IR_P_ON_Pin|ERP_SENSOR_ON_Pin|_RESET_Pin|_USB2_OE_Pin
                     |SLEEP_STATUS_REM_EG_Pin|MC_PWR_EN_Pin|SB_PWR_EN_Pin|MC_P_ON_Pin
                     |_RST_SLP2_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
```
`[SOURCE]` [mx_init.c:455-465](../../pscpu_s800/main/Src/mx_init.c#L455-L465). Để cụ thể, tôi trace
riêng cho **`MC_P_ON_Pin` = `GPIO_PIN_5` = bit position 5 trên GPIOB**.

## Chuỗi trace tổng quan

```text
Application code (mx_init.c)
       ↓
GPIO_InitTypeDef (struct trong RAM, tạm thời — không phải hardware)
       ↓
HAL_GPIO_Init()  [stm32f0xx_hal_gpio.c:188]
       ↓
GPIO register (MODER, OSPEEDR, OTYPER, PUPDR — hardware thật, memory-mapped)
       ↓
Hardware (mạch logic bên trong GPIOB peripheral đọc các bit này, điều khiển transistor)
       ↓
Physical pin (chân PB5 thật trên package chip)
```

## Bước 1 — `HAL_GPIO_Init()` tính `position` cho `MC_P_ON_Pin`

`[SOURCE]` [stm32f0xx_hal_gpio.c:201-204](../../common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c#L201-L204):
```c
while (((GPIO_Init->Pin) >> position) != RESET) {
    iocurrent = (GPIO_Init->Pin) & (1U << position);
    if(iocurrent) { /* cấu hình cho "position" này */ }
    position++;
}
```
Với `Pin` chứa bit 5 (`MC_P_ON_Pin = 0x0020`), vòng lặp sẽ tới `position = 5`, `iocurrent = 0x0020`
khác 0 → nhánh `if` chạy.

## Bước 2 — Ghi `MODER` (GPIO port mode register)

`[SOURCE]` [stm32f0xx_hal_gpio.c:224-227](../../common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c#L224-L227):
```c
temp = GPIOx->MODER;
CLEAR_BIT(temp, GPIO_MODER_MODER0 << (position * 2));     // xóa 2 bit tại vị trí 5*2=10,11
SET_BIT(temp, (GPIO_Init->Mode & GPIO_MODE) << (position * 2));  // ghi 01 (OUTPUT_PP & 0x3 = 0x1)
GPIOx->MODER = temp;
```
`[GENERAL MCU KNOWLEDGE]` `MODER` dùng **2 bit cho mỗi pin** (vì có 4 giá trị: Input=00, Output=01,
AF=10, Analog=11) → với 16 pin cần 32 bit, khớp đúng kích thước 1 thanh ghi 32-bit.

```text
MODER (32-bit), vị trí bit cho PB5 = bit 11:10 (vì position*2 = 5*2 = 10)

Bit:  31..24  23..22(PB11) ... 13..12(PB6)  11:10(PB5)  09:08(PB4) ... 01:00(PB0)
                                              ↑
                                    ghi "01" = GPIO_MODE_OUTPUT_PP (0x1 & GPIO_MODE(0x3))
```
→ Sau dòng này, phần cứng GPIOB **biết PB5 giờ là Output**, output driver được "mở khóa".

## Bước 3 — Ghi `OSPEEDR` và `OTYPER` (chỉ với Output/AF)

`[SOURCE]` [stm32f0xx_hal_gpio.c:230-245](../../common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c#L230-L245):
```c
if ((Mode==OUTPUT_PP)||(Mode==AF_PP)||(Mode==OUTPUT_OD)||(Mode==AF_OD)) {
  temp = GPIOx->OSPEEDR;
  CLEAR_BIT(temp, GPIO_OSPEEDER_OSPEEDR0 << (position * 2));
  SET_BIT(temp, GPIO_Init->Speed << (position * 2));        // Speed = GPIO_SPEED_FREQ_LOW = 0x0
  GPIOx->OSPEEDR = temp;

  temp = GPIOx->OTYPER;
  CLEAR_BIT(temp, GPIO_OTYPER_OT_0 << position);             // 1 bit mỗi pin (vì OTYPER chỉ có 2 giá trị: PP/OD)
  SET_BIT(temp, ((GPIO_Init->Mode & GPIO_OUTPUT_TYPE) >> 4) << position);  // OUTPUT_PP: bit OUTPUT_TYPE(0x10)=0 → set 0 = Push-Pull
  GPIOx->OTYPER = temp;
}
```
- `OSPEEDR`: cũng 2 bit/pin → bit 11:10 của `OSPEEDR` được ghi `00` (Speed Low).
- `OTYPER`: chỉ **1 bit/pin** (vì chỉ cần phân biệt Push-Pull=0 / Open-Drain=1) → bit 5 của `OTYPER`
  được ghi `0` (Push-Pull), khớp với `GPIO_MODE_OUTPUT_PP = 0x1` không có bit `GPIO_OUTPUT_TYPE(0x10)`
  set (còn `GPIO_MODE_OUTPUT_OD = 0x11` có bit đó set — đây chính là cách 1 giá trị `Mode` 32-bit mã
  hóa **cả MODER lẫn OTYPER** cùng lúc, như đã thấy ở `GPIO_MODE_IT_FALLING` mã hóa nhiều thông tin ở
  file 06).

## Bước 4 — Ghi `PUPDR` (luôn chạy, không điều kiện)

`[SOURCE]` [stm32f0xx_hal_gpio.c:248-252](../../common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c#L248-L252):
```c
temp = GPIOx->PUPDR;
CLEAR_BIT(temp, GPIO_PUPDR_PUPDR0 << (position * 2));
SET_BIT(temp, (GPIO_Init->Pull) << (position * 2));   // Pull = GPIO_NOPULL = 0x0
GPIOx->PUPDR = temp;
```
Bit 11:10 của `PUPDR` = `00` (No pull) — vì với output push-pull, pull resistor nội thường không cần
thiết (driver luôn chủ động ép mức), nhưng vẫn ghi để đảm bảo giá trị xác định, không để sót giá trị
cũ từ lần cấu hình trước.

## Vị trí vật lý thật của các thanh ghi này trên chip STM32F031C6

`[SOURCE]` [stm32f031x6.h:250-262](../../common/Drivers/CMSIS/Device/ST/STM32F0xx/Include/stm32f031x6.h#L250-L262) —
copy nguyên văn từ file thật, **kể cả 1 điểm không khớp giữa comment và thực tế mà tôi phát hiện khi
đọc**:
```c
typedef struct {
  __IO uint32_t MODER;    /* Address offset: 0x00 */
  __IO uint32_t OTYPER;   /* Address offset: 0x04 */
  __IO uint32_t OSPEEDR;  /* Address offset: 0x08 */
  __IO uint32_t PUPDR;    /* Address offset: 0x0C */
  __IO uint32_t IDR;      /* Address offset: 0x10 */
  __IO uint32_t ODR;      /* Address offset: 0x14 */
  __IO uint32_t BSRR;     /* comment ghi "Address offset: 0x1A" — NHƯNG đây là lỗi trong comment của ST */
  __IO uint32_t LCKR;     /* Address offset: 0x1C */
  __IO uint32_t AFR[2];   /* Address offset: 0x20-0x24 */
  __IO uint32_t BRR;      /* Address offset: 0x28 — thanh ghi RIÊNG, không phải nửa cao của BSRR trên dòng này */
} GPIO_TypeDef;
```
**Vì sao comment "0x1A" sai?** `[GENERAL MCU KNOWLEDGE]` Compiler C **không đọc comment** — nó tự tính
offset của mỗi field dựa trên **kích thước và thứ tự khai báo** của các field đứng trước. 6 field đầu
(`MODER`...`ODR`) mỗi field 4 byte (`uint32_t`) → `0x00,0x04,0x08,0x0C,0x10,0x14`, vậy field tiếp theo
(`BSRR`) **chắc chắn** nằm ở `0x14 + 4 = 0x18`, khớp với `LCKR` ngay sau nó ở `0x1C` (`0x18+4=0x1C`).
Con số "0x1A" trong comment là **lỗi đánh máy/copy trong chính CMSIS header do ST phát hành**, không
ảnh hưởng gì đến hành vi thực tế của chip hay code (comment chỉ là văn bản cho người đọc, compiler bỏ
qua hoàn toàn) — nhưng là một ví dụ tốt cho thấy **phải tin vào cách compiler tính offset (thứ tự +
kích thước field), không phải tin mù quáng vào mọi dòng comment**, kể cả comment từ chính nhà sản xuất
chip.

**Về `BRR` (offset `0x28`)**: đây xác nhận lại chi tiết ở file 07 — trên dòng **STM32F0**, `BRR` là
**một thanh ghi phần cứng độc lập, riêng biệt** (không phải cách gọi khác của nửa cao `BSRR`), khớp
chính xác với việc `HAL_GPIO_WritePin()` dùng `GPIOx->BRR` như một thanh ghi riêng khi muốn Reset.
`[SOURCE]` [stm32f031x6.h:502-503,544-545](../../common/Drivers/CMSIS/Device/ST/STM32F0xx/Include/stm32f031x6.h#L502-L545):
```c
#define GPIOA_BASE  (AHB2PERIPH_BASE + 0x00000000)
#define GPIOB_BASE  (AHB2PERIPH_BASE + 0x00000400)
#define GPIOA  ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB  ((GPIO_TypeDef *) GPIOB_BASE)
```
→ Khi code viết `GPIOB->MODER = ...`, địa chỉ ghi thật là `AHB2PERIPH_BASE + 0x400 + 0x00`
— một con số cố định, cứng trong silicon, **không đổi** giữa các lần build hay các project khác dùng
cùng chip.

## Sơ đồ kiến trúc tổng thể (đầy đủ, từ RCC tới CPU IRQ)

```text
                               CPU (Cortex-M0)
                                    |
                            Bus (AHB, memory-mapped)
                                    |
             +----------------------+----------------------+
             |                      |                      |
            RCC                   GPIO                    NVIC
             |                      |                      |
      AHBENR.GPIOxEN          +-----+-----+          IRQn table
      (bật clock từng Port)   |           |          (EXTI0_1, EXTI2_3,
             |               GPIOA       GPIOB         EXTI4_15, TIM3,
       (cũng điều khiển             |           |         I2C1, RTC...)
        PLL/HSI/LSE/LSI —      MODER/OTYPER/  MODER/OTYPER/      |
        xem file clock ở       OSPEEDR/PUPDR  OSPEEDR/PUPDR      |
        câu hỏi trước)         IDR/ODR/BSRR   IDR/ODR/BSRR       |
             |                      |           |                |
             |                   PA0..15      PB0..15             |
             |                      |                              |
             |                 (các pin cấu hình                   |
             |                  EXTI_MODE: PA1,2,4,5,8)             |
             |                      |                              |
             |                SYSCFG->EXTICR (chọn port cho line)   |
             |                      |                              |
             |                   EXTI (IMR/EMR/RTSR/FTSR/PR)        |
             |                      |                              |
             |                 pending bit lên 1 ──────────────────┘
             |                                                      |
             |                                              CPU bị ngắt, nhảy
             |                                              vào Vector Table
             |                                                      |
             |                                              xxx_IRQHandler()
             |                                                      |
             |                                        HAL_GPIO_EXTI_IRQHandler()
             |                                                      |
             |                                         HAL_GPIO_EXTI_Callback()
             |                                                      |
             |                                      BSP_GPIO_EXTI_Callback() → signal_event()
             |                                                      |
             |                                              km_exti_callback()  [App layer thật]
```

Tiếp theo: [10_debug_va_loi_thuong_gap.md](10_debug_va_loi_thuong_gap.md).
