# Phần 3: `__HAL_RCC_GPIOx_CLK_ENABLE()` — RCC, memory-mapped I/O, và tại sao phải bật clock trước

## LEVEL 1-2 — RCC là gì? Vì sao CPU "nói chuyện" với GPIO qua bus?

**Định nghĩa đơn giản**: RCC (Reset and Clock Control) là **peripheral trung tâm quản lý toàn bộ
xung clock** của chip — bật/tắt dao động (HSI/LSI/LSE/PLL — đã phân tích ở câu hỏi trước về
`SystemClock_Config()`), và quan trọng với GPIO: **bật/tắt clock cấp cho từng peripheral khác**
(GPIOA, GPIOB, I2C1, TIM3...).

**Ví dụ đời thường**: Hãy tưởng tượng tòa nhà có **một phòng cầu dao tổng (RCC)**. Mỗi phòng/khu vực
trong tòa nhà (mỗi peripheral: GPIOA, GPIOB, I2C1...) có **cầu dao riêng** trong phòng cầu dao tổng
đó. Dù dây điện đã kéo tới tận phòng, nếu cầu dao riêng của phòng đó chưa gạt lên, phòng đó **vẫn
tối đen, không hoạt động** — dù hệ thống điện tổng (nguồn VDD chip) vẫn đang có điện.

## Memory-mapped I/O là gì?

`[GENERAL MCU KNOWLEDGE]` Trên ARM Cortex-M, **không có lệnh CPU riêng để "nói chuyện với phần cứng"**
(khác với một số CPU x86 có lệnh `IN`/`OUT` riêng). Toàn bộ các peripheral (GPIO, RCC, I2C, Timer...)
được **gán một địa chỉ trong không gian bộ nhớ** — CPU chỉ cần dùng lệnh đọc/ghi bộ nhớ bình thường
(load/store) để điều khiển phần cứng, y như đọc/ghi một biến trong RAM. Đây gọi là
**memory-mapped I/O (MMIO)**.

```
Không gian địa chỉ 32-bit của CPU:
0x0000_0000 ─ Flash (code chương trình)
0x2000_0000 ─ SRAM (RAM)
0x4000_0000 ─ Peripheral (RCC, I2C, TIM...)   ← RCC nằm ở đây, ví dụ RCC_BASE
0x4800_0000 ─ Peripheral AHB2 (GPIO)           ← GPIOA, GPIOB... nằm ở đây
```

`[SOURCE]` Xác nhận bằng chính CMSIS header của chip này — struct `GPIO_TypeDef` định nghĩa đúng các
thanh ghi theo offset byte, và macro `GPIOA`/`GPIOB` là một con trỏ ép kiểu tới địa chỉ cố định:
```c
#define GPIOA  ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB  ((GPIO_TypeDef *) GPIOB_BASE)
```
[stm32f031x6.h:502-503,544-545](../../common/Drivers/CMSIS/Device/ST/STM32F0xx/Include/stm32f031x6.h)

→ Khi code viết `GPIOx->MODER = ...`, thực chất CPU thực hiện **một lệnh ghi bộ nhớ bình thường**
vào một địa chỉ cố định — phần cứng GPIO "rình" địa chỉ đó và phản ứng lại khi bị ghi.

## GPIO peripheral clock là gì? Vì sao phải enable trước khi cấu hình?

`[GENERAL MCU KNOWLEDGE]` Mỗi mạch logic số (bao gồm GPIO peripheral) cần một **xung clock** để các
flip-flop/latch bên trong hoạt động (lưu trạng thái bit, phản ứng với lệnh ghi). Để tiết kiệm điện,
STM32 **mặc định tắt clock** cho mọi peripheral chưa dùng — chip reset xong, hầu hết peripheral ở
trạng thái "ngủ", chưa có clock.

**Nếu không enable clock mà vẫn cố ghi vào thanh ghi GPIO** → việc ghi **không có tác dụng gì**
(hoặc trên một số chip, hành vi không xác định) vì mạch logic bên trong GPIO peripheral chưa được cấp
xung để "chạy" — giống như gạt công tắc đèn khi cầu dao phòng đó chưa lên, đèn vẫn không sáng dù công
tắc đã gạt.

## Macro này cuối cùng làm gì — đi từ Macro xuống Hardware

```c
__HAL_RCC_GPIOA_CLK_ENABLE();
```

`[SOURCE]` [stm32f0xx_hal_rcc.h:640-646](../../common/Drivers/STM32F0xx_HAL_Driver/Inc/stm32f0xx_hal_rcc.h#L640-L646):
```c
#define __HAL_RCC_GPIOA_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->AHBENR, RCC_AHBENR_GPIOAEN);\
                                        tmpreg = READ_BIT(RCC->AHBENR, RCC_AHBENR_GPIOAEN);\
                                        UNUSED(tmpreg); \
                                      } while(0)
```

Trace tiếp xuống register thật (chip STM32F031C6):
`[SOURCE]` [stm32f031x6.h:3156-3167](../../common/Drivers/CMSIS/Device/ST/STM32F0xx/Include/stm32f031x6.h#L3156-L3167):
```c
#define RCC_AHBENR_GPIOAEN   (0x1U << 17)   // bit 17 của RCC->AHBENR
#define RCC_AHBENR_GPIOBEN   (0x1U << 18)   // bit 18
#define RCC_AHBENR_GPIOCEN   (0x1U << 19)   // bit 19
#define RCC_AHBENR_GPIOFEN   (0x1U << 22)   // bit 22
```

```text
Macro                              __HAL_RCC_GPIOA_CLK_ENABLE()
  ↓
HAL (SET_BIT là macro CMSIS)       SET_BIT(RCC->AHBENR, RCC_AHBENR_GPIOAEN)
  ↓                                 tương đương: RCC->AHBENR |= (1 << 17)
Register                           RCC->AHBENR  (địa chỉ cố định, peripheral RCC)
  ↓
Bit                                 bit 17 = GPIOAEN
  ↓
Hardware                            mạch chia/định tuyến clock nội bộ bật dòng clock
                                     chạy tới mạch logic của GPIOA peripheral
```

**Đọc lại ngay sau khi ghi (`tmpreg = READ_BIT(...)`) để làm gì?** — giống hệt cơ chế đã phân tích ở
macro `__HAL_RCC_PWR_CLK_ENABLE()` (câu hỏi trước): việc bật clock **không có hiệu lực tức thời** do độ
trễ lan truyền tín hiệu trong mạch clock-tree; đọc lại thanh ghi buộc CPU **chờ** cho đến khi bus
xác nhận ghi xong, đảm bảo dòng code tiếp theo (vốn sẽ ghi `GPIOx->MODER`...) chạy khi clock GPIOA
**đã chắc chắn ổn định**.

## Áp dụng vào source thật của project

```c
__HAL_RCC_GPIOC_CLK_ENABLE();
__HAL_RCC_GPIOF_CLK_ENABLE();
__HAL_RCC_GPIOA_CLK_ENABLE();
__HAL_RCC_GPIOB_CLK_ENABLE();
```
`[SOURCE]` [mx_init.c:425-428](../../pscpu_s800/main/Src/mx_init.c#L425-L428)

Bốn dòng này bật clock cho **cả 4 Port mà project dùng đến**, luôn đứng **đầu tiên** trong
`MX_GPIO_Init()` — trước bất kỳ dòng `HAL_GPIO_Init(...)` nào. Nếu đảo thứ tự (cấu hình pin trước, bật
clock sau), việc cấu hình `MODER`/`OTYPER`/`PUPDR`... cho pin đó **sẽ không có tác dụng**, vì lúc ghi,
GPIO peripheral tương ứng chưa có clock để "nhận" lệnh ghi.

## Lỗi thường gặp liên quan

- **Quên enable clock cho đúng Port**: ví dụ code dùng `PF0_IN_Pin` nhưng quên gọi
  `__HAL_RCC_GPIOF_CLK_ENABLE()` → đọc `PF0_IN_Pin` luôn trả về giá trị cố định (thường là 0), không
  phản ứng với điện áp thật trên chân — bug rất khó nhận ra vì **code compile và chạy "bình thường",
  không crash, chỉ trả sai dữ liệu**.
- Nhầm Port: gọi `__HAL_RCC_GPIOA_CLK_ENABLE()` nhưng lại cấu hình pin trên `GPIOB` → GPIOB vẫn chưa
  có clock, tương tự lỗi trên.

Tiếp theo: [03_gpio_inittypedef.md](03_gpio_inittypedef.md) — cấu trúc `GPIO_InitTypeDef` và cách
`Pin` biểu diễn nhiều chân cùng lúc bằng bitmask.
