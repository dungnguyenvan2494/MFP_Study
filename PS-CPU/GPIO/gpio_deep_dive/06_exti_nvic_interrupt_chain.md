# Phần 10-13: EXTI, NVIC, và trace đầy đủ từ GPIO edge đến callback thật trong code

Đây là phần bạn muốn hiểu sâu nhất. Toàn bộ nội dung dưới đây được trace bằng **source code thật**
trong chính project PS-CPU — không dùng ví dụ generic.

## LEVEL 1 — Sơ đồ tổng quan chuỗi sự kiện

```text
GPIO pin (điện áp đổi mức)
 ↓
Edge detection (mạch cứng trong EXTI, so sánh mức hiện tại với mức trước đó)
 ↓
EXTI (External Interrupt/Event Controller — peripheral riêng, không nằm trong GPIO)
 ↓
NVIC (Nested Vectored Interrupt Controller — nằm trong lõi Cortex-M0)
 ↓
CPU dừng chương trình đang chạy, nhảy vào Vector Table
 ↓
ISR (Interrupt Service Routine — hàm `xxx_IRQHandler`)
 ↓
HAL_GPIO_EXTI_IRQHandler() → HAL_GPIO_EXTI_Callback() → (code ứng dụng thật của project)
```

## EXTI là gì?

`[GENERAL MCU KNOWLEDGE]` EXTI (External Interrupt/Event Controller) là **một peripheral độc lập**,
**không phải một phần của GPIO peripheral** — nó chỉ "theo dõi" tín hiệu được GPIO chuyển tới thông
qua một bộ chọn kênh gọi là `SYSCFG->EXTICR`. GPIO chịu trách nhiệm đọc mức điện áp thô; EXTI chịu
trách nhiệm **phát hiện sự thay đổi mức (edge)** và tạo ra yêu cầu ngắt.

### EXTI line là gì?

Có **16 "line" EXTI dành cho GPIO** (EXTI0 đến EXTI15), và mỗi line **chỉ nhận tín hiệu từ 1 số chân
cùng vị trí (position) trên các Port khác nhau tại một thời điểm**. Ví dụ EXTI line 1 có thể nhận từ
PA1 **hoặc** PB1 **hoặc** PC1... (không bao giờ đồng thời), do một bộ mux (`SYSCFG->EXTICR`) chọn.

### Falling edge / Rising edge là gì?

```text
HIGH ────────┐
             │
             └──────── LOW
             ↑
        Falling Edge   (chuyển từ HIGH xuống LOW)

LOW  ────────┐
             │
             └──────── HIGH
             ↑
        Rising Edge    (chuyển từ LOW lên HIGH)
```
`[GENERAL MCU KNOWLEDGE]` EXTI có 2 thanh ghi riêng để chọn loại cạnh cần bắt: `RTSR` (Rising Trigger
Selection Register) và `FTSR` (Falling Trigger Selection Register) — có thể bật **1 trong 2, hoặc cả
2** cho cùng 1 line (= bắt cả 2 cạnh).

## GPIO peripheral và EXTI khác nhau như thế nào?

| | GPIO peripheral | EXTI peripheral |
|---|---|---|
| Vai trò | Đọc mức điện áp liên tục (`IDR`), hoặc xuất mức (`ODR`) | Chỉ phát hiện **thay đổi** mức (edge), không quan tâm mức tĩnh |
| Có clock riêng (RCC)? | Có (`RCC_AHBENR_GPIOxEN`) | Có cơ chế riêng, không cần enable clock kiểu AHBENR như GPIO |
| Có tạo ngắt không? | Không tự tạo ngắt | Có — đây là peripheral chịu trách nhiệm tạo pending bit cho NVIC |
| Map vào NVIC IRQ nào? | Không trực tiếp | Có — mỗi line/nhóm line map tới 1 `IRQn` cụ thể |

## Trace thật: `GPIO_MODE_IT_FALLING` ghi vào register nào trong `HAL_GPIO_Init()`

```c
GPIO_InitStruct.Pin = _HRESET_REQ_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
GPIO_InitStruct.Pull = GPIO_NOPULL;
HAL_GPIO_Init(_HRESET_REQ_GPIO_Port, &GPIO_InitStruct);
```
`[SOURCE]` [mx_init.c:436-440](../../pscpu_s800/main/Src/mx_init.c#L436-L440) —
`_HRESET_REQ_Pin` = PA1 `[SOURCE]` [mxconstants.h:50-51](../../pscpu_s800/main/Inc/mxconstants.h#L50-L51).

Giá trị `GPIO_MODE_IT_FALLING` thực chất mã hóa **4 thông tin cùng lúc** trong 1 số 32-bit:
`[SOURCE]` [stm32f0xx_hal_gpio.h:142](../../common/Drivers/STM32F0xx_HAL_Driver/Inc/stm32f0xx_hal_gpio.h#L142)
và các mask riêng trong file .c:
`[SOURCE]` [stm32f0xx_hal_gpio.c:147-153](../../common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c#L147-L153)

```text
GPIO_MODE_IT_FALLING = 0x10210000
                      = EXTI_MODE(0x10000000) | GPIO_MODE_IT(0x00010000) | FALLING_EDGE(0x00200000)
                        + 2 bit thấp = 00 (base mode = INPUT)
```

Đoạn code `HAL_GPIO_Init()` giải mã từng bit này:
`[SOURCE]` [stm32f0xx_hal_gpio.c:254-299](../../common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c#L254-L299):
```c
if((GPIO_Init->Mode & EXTI_MODE) == EXTI_MODE)        // có bit 0x10000000 → đây là chế độ EXTI
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();                      // SYSCFG cần clock để ghi EXTICR (xem bài macro PWR/RCC trước)

  temp = SYSCFG->EXTICR[position >> 2];               // chọn đúng 1 trong 4 thanh ghi EXTICR0..3
  CLEAR_BIT(temp, 0x0F << (4*(position & 0x03)));
  SET_BIT(temp, GPIO_GET_INDEX(GPIOx) << (4*(position & 0x03)));   // "line 1 → map tới GPIOA" (vì GPIOx = GPIOA)
  SYSCFG->EXTICR[position >> 2] = temp;

  temp = EXTI->IMR;                                   // Interrupt Mask Register
  CLEAR_BIT(temp, iocurrent);
  if((GPIO_Init->Mode & GPIO_MODE_IT) == GPIO_MODE_IT) SET_BIT(temp, iocurrent);  // bật IT cho line 1
  EXTI->IMR = temp;

  temp = EXTI->RTSR; CLEAR_BIT(temp, iocurrent);
  if((GPIO_Init->Mode & RISING_EDGE) == RISING_EDGE) SET_BIT(temp, iocurrent);     // không set vì IT_FALLING không có RISING_EDGE
  EXTI->RTSR = temp;

  temp = EXTI->FTSR; CLEAR_BIT(temp, iocurrent);
  if((GPIO_Init->Mode & FALLING_EDGE) == FALLING_EDGE) SET_BIT(temp, iocurrent);   // SET vì IT_FALLING có bit này
  EXTI->FTSR = temp;
}
```

**Tóm lại, với `_HRESET_REQ_Pin` (PA1, position=1)**:
1. `SYSCFG->EXTICR[0]` được ghi để "EXTI line 1 lấy tín hiệu từ GPIOA" (không phải GPIOB/C/F).
2. `EXTI->IMR` bit 1 = 1 (cho phép line 1 tạo **ngắt**, khác với `EMR` dùng cho "event" không vào ISR).
3. `EXTI->FTSR` bit 1 = 1 (bắt cạnh **falling**).
4. `EXTI->RTSR` bit 1 = 0 (không bắt rising).

## `GPIO_MODE_IT_RISING_FALLING` — vì sao 4 pin cần bắt cả 2 cạnh

```c
GPIO_InitStruct.Pin = AP_PWR_EN_Pin|POWER_MONITOR_Pin|MSW_ON_Pin|MONI_24V11_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
```
`[SOURCE]` [mx_init.c:442-445](../../pscpu_s800/main/Src/mx_init.c#L442-L445) — 4 pin này đều trên
GPIOA: `AP_PWR_EN`=PA2, `POWER_MONITOR`=PA4, `MSW_ON`=PA5, `MONI_24V11`=PA8
`[SOURCE]` [mxconstants.h:52-57,76-77](../../pscpu_s800/main/Inc/mxconstants.h#L52-L57)

`GPIO_MODE_IT_RISING_FALLING = 0x10310000` → cả `RTSR` và `FTSR` đều set bit tương ứng.

**Tại sao cần bắt cả 2 cạnh?** Đối chiếu với logic xử lý thật trong `km_exti_callback()` (xem cuối
file này) — cả 4 tín hiệu này đều là **tín hiệu trạng thái 2 chiều mà hệ thống cần biết CẢ 2 sự kiện**:

- `POWER_MONITOR`: hệ thống cần biết **cả lúc mất điện (HIGH→LOW)** để tắt khẩn cấp MC_P_ON ngay
  (`[SOURCE]` [km_extend_io.c:448-452](../../pscpu_s800/main/App/km_extend_io.c#L448-L452), đã phân
  tích ở câu hỏi trước), **và lúc có điện lại (LOW→HIGH)** để biết khi nào an toàn bật nguồn trở lại.
- `MSW_ON` (công tắc nguồn cơ khí): cần biết cả lúc bấm (1 cạnh) và lúc thả/không bấm (cạnh ngược).
- `AP_PWR_EN`, `MONI_24V11`: tương tự — trạng thái 2 chiều, hệ thống phản ứng khác nhau ở mỗi hướng
  chuyển mức.

Code thật xác nhận cơ chế xử lý **sau khi bắt được edge, vẫn đọc lại mức hiện tại** bằng
`BSP_GPIO_ReadPin()` (không chỉ dựa vào "có ngắt là đủ") — vì ngắt chỉ báo "có thay đổi", còn
**mức hiện tại cụ thể là gì vẫn phải đọc `IDR` riêng**:
`[SOURCE]` [km_it.c:259-266](../../pscpu_s800/main/App/km_it.c#L259-L266) (xem đầy đủ cuối file).

## NVIC là gì?

`[GENERAL MCU KNOWLEDGE]` NVIC (Nested Vectored Interrupt Controller) là mạch điều phối ngắt **nằm
ngay trong lõi Cortex-M**, chịu trách nhiệm: nhận "pending bit" từ các peripheral (EXTI, Timer, I2C...),
quyết định ngắt nào được xử lý trước nếu có nhiều ngắt cùng chờ, và **ép CPU nhảy** tới đúng địa chỉ
hàm xử lý (ISR) tương ứng trong Vector Table.

### Interrupt priority / Preemption priority / Subpriority

`[GENERAL MCU KNOWLEDGE]` Trên Cortex-M0 (chip này dùng), NVIC **đơn giản hơn** M3/M4 — chỉ có
**4 mức ưu tiên** (2-bit), và **không có khái niệm preemption/subpriority tách riêng** như M3/M4 (đó
là lý do hàm `HAL_NVIC_SetPriority(IRQn, 0, 0)` luôn truyền subpriority=0 — tham số đó bị bỏ qua trên
M0, chỉ tham số đầu (0) có tác dụng là mức ưu tiên chính). Mức số **nhỏ hơn = ưu tiên cao hơn** (0 là
cao nhất).

### IRQn, IRQ Handler, Pending bit, Enable bit

- **IRQn**: một mã số định danh "loại ngắt nào" (ví dụ `EXTI0_1_IRQn`) — dùng để nói với NVIC "tôi
  đang cấu hình ngắt này".
- **IRQ Handler**: hàm có tên cố định theo chuẩn CMSIS/startup file (ví dụ `EXTI0_1_IRQHandler`), CPU
  tự nhảy vào đây khi ngắt đó xảy ra — tên hàm này **không gọi trực tiếp từ code của bạn**, mà được
  đặt sẵn trong Vector Table lúc build (`startup_stm32f0xx.s`).
- **Pending bit**: 1 bit báo "ngắt này đang chờ xử lý" — EXTI set bit này khi phát hiện đúng edge.
  Phải **xóa (clear) pending bit** trong ISR, nếu không ngắt sẽ lặp lại ngay khi ISR vừa thoát.
- **Enable bit**: 1 bit riêng cho phép NVIC "lắng nghe" loại ngắt đó — nếu enable=0, pending bit có
  lên 1 cũng không bao giờ CPU nhảy vào ISR (`HAL_NVIC_EnableIRQ()` set bit này).

## Vì sao nhiều pin dùng chung 1 IRQ (`EXTI0_1_IRQn`, `EXTI2_3_IRQn`, `EXTI4_15_IRQn`)?

```c
HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);   HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);   HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
```
`[SOURCE]` [mx_init.c:508-517](../../pscpu_s800/main/Src/mx_init.c#L508-L517)

`[GENERAL MCU KNOWLEDGE]` STM32F0 (và nhiều dòng STM32 nhỏ) **không có đủ số IRQn riêng cho cả 16 EXTI
line** — để tiết kiệm số vector trong Vector Table, nhà sản xuất **gộp nhiều line vào cùng 1 IRQn**:
line 0 và 1 chung `EXTI0_1_IRQn`, line 2 và 3 chung `EXTI2_3_IRQn`, line 4 đến 15 (12 line) chung
`EXTI4_15_IRQn`. Khi 1 trong các line thuộc nhóm đó có pending bit, CPU nhảy vào **cùng 1 hàm
IRQHandler** — hàm đó phải tự kiểm tra **line nào cụ thể** đang pending để xử lý đúng, như bạn sẽ thấy
ngay dưới đây bằng code thật của project.

## Trace đầy đủ — code thật trong `stm32f0xx_it.c`

`[SOURCE]` [stm32f0xx_it.c:145-184](../../pscpu_s800/main/Src/stm32f0xx_it.c#L145-L184):
```c
void EXTI0_1_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);   // chỉ check line 1 — vì project không dùng line 0
}

void EXTI2_3_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);   // chỉ check line 2 — AP_PWR_EN (PA2), line này đã được EXTICR map vào GPIOA
}

void EXTI4_15_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);   // POWER_MONITOR (PA4)
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);   // MSW_ON (PA5)
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);   // MONI_24V11 (PA8)
}
```

Khớp chính xác với 5 pin có `GPIO_MODE_IT_*` trong `MX_GPIO_Init()`:
`_HRESET_REQ`=PA1→line1, `AP_PWR_EN`=PA2→line2, `POWER_MONITOR`=PA4→line4, `MSW_ON`=PA5→line5,
`MONI_24V11`=PA8→line8 — CubeMX **tự sinh đúng số lượng dòng gọi cần thiết** theo từng line thực sự
được cấu hình ngắt, không phải gọi dư.

`[SOURCE]` `HAL_GPIO_EXTI_IRQHandler()` tại
[stm32f0xx_hal_gpio.c:502-510](../../common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c#L502-L510):
```c
void HAL_GPIO_EXTI_IRQHandler(uint16_t GPIO_Pin)
{
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != RESET)   // đọc EXTI->PR (Pending Register) — line này có đang pending?
  {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);           // XÓA pending bit NGAY — tránh ngắt lặp lại vô hạn
    HAL_GPIO_EXTI_Callback(GPIO_Pin);             // gọi callback — hàm __weak, chờ override
  }
}
```

→ Đây là lý do `EXTI4_15_IRQHandler()` gọi `HAL_GPIO_EXTI_IRQHandler()` **3 lần với 3 pin khác nhau**:
mỗi lần hàm tự kiểm tra **đúng pending bit của line đó** — nếu chỉ line 5 (MSW_ON) đang pending, lời
gọi với `GPIO_PIN_4` và `GPIO_PIN_8` sẽ **không làm gì cả** (điều kiện `if` sai), chỉ lời gọi với
`GPIO_PIN_5` mới thực sự chạy tiếp vào callback.

## Callback thật — không dừng ở hàm `__weak` rỗng

`[SOURCE]` Hàm gốc trong HAL là `__weak` (rỗng):
[stm32f0xx_hal_gpio.c:517-525](../../common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c#L517-L525).
Nhưng project này **có override thật** ở tầng BSP:
`[SOURCE]` [BSP_GPIO_STM32F03x_Nucleo.c:71-76](../../common/Drivers/BSP/Src/BSP_GPIO_STM32F03x_Nucleo.c#L71-L76):
```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(signal_event != NULL) {
    signal_event(GPIO_Pin, GPIO_EVENT_EXT_INTR);   // gọi tiếp 1 hàm con trỏ đã đăng ký từ tầng ứng dụng
  }
}
```

`signal_event` là một **con trỏ hàm** được tầng ứng dụng đăng ký (cơ chế giống "đăng ký listener") —
nó trỏ tới `km_exti_callback()` trong App layer:
`[SOURCE]` [km_it.c:247-282](../../pscpu_s800/main/App/km_it.c#L247-L282):
```c
static void km_exti_callback(PinType GPIO_Pin, GPIOEventType event)
{
    PinStateType level;
    if(event == GPIO_EVENT_EXT_INTR){
        switch(GPIO_Pin){
            case _HRESET_REQ_Pin:
                set_pending_factor_bit(TYPE_Km_PFB_HRESET_REQ);   // chỉ "đánh dấu cờ", xử lý thật ở main loop
                break;
            case AP_PWR_EN_Pin:
                set_pending_factor_bit(TYPE_Km_PFB_AP_PWR_EN);
                break;
            case POWER_MONITOR_Pin:
                level = BSP_GPIO_ReadPin(POWER_MONITOR_GPIO_Port, POWER_MONITOR_Pin);  // đọc lại mức HIỆN TẠI
                start_anti_chattering(TYPE_Km_AC_Idx_POWER_MONI, level);               // khởi động bộ lọc chống dội (debounce)
                break;
            case MONI_24V11_Pin:
                level = BSP_GPIO_ReadPin(MONI_24V11_GPIO_Port, MONI_24V11_Pin);
                start_anti_chattering(TYPE_Km_AC_Idx_24V11_MONI, level);
                break;
            case MSW_ON_Pin:
                level = BSP_GPIO_ReadPin(MSW_ON_GPIO_Port, MSW_ON_Pin);
                /* ... xử lý thoát Stop mode nếu đang ngủ ... */
                start_anti_chattering(TYPE_Km_AC_Idx_MSW, level);
                break;
        }
    }
}
```

## Sơ đồ call flow đầy đủ — thật 100%, không phải generic

```text
MX_GPIO_Init()                                   [mx_init.c:419]
    ↓
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct)           [mx_init.c:436-440, mode=IT_FALLING cho _HRESET_REQ]
    ↓ (lúc init — không phải lúc chạy)
EXTI configuration: EXTICR, IMR, FTSR             [hal_gpio.c:261-298]
    ↓ (lúc runtime — GPIO edge thật xảy ra)
GPIO edge occurs (ví dụ host kéo _HRESET_REQ xuống LOW)
    ↓
EXTI pending (EXTI->PR bit tương ứng = 1)
    ↓
NVIC (đã enable qua HAL_NVIC_EnableIRQ, đang chờ) → CPU bị ngắt
    ↓
EXTI0_1_IRQHandler()                              [stm32f0xx_it.c:145]
    ↓
HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1)              [hal_gpio.c:502] — check pending, clear, gọi callback
    ↓
HAL_GPIO_EXTI_Callback(GPIO_PIN_1)                [override tại BSP_GPIO_STM32F03x_Nucleo.c:71]
    ↓
signal_event(GPIO_PIN_1, GPIO_EVENT_EXT_INTR)     [con trỏ hàm, trỏ tới...]
    ↓
km_exti_callback(_HRESET_REQ_Pin, GPIO_EVENT_EXT_INTR)   [km_it.c:247]
    ↓
set_pending_factor_bit(TYPE_Km_PFB_HRESET_REQ)    [km_it.c:254] — chỉ đánh cờ, KHÔNG xử lý logic nặng trong ISR
```

**Ghi chú quan trọng về thiết kế**: ISR trong project này **chỉ đánh cờ** (`set_pending_factor_bit`)
hoặc khởi động bộ chống dội (`start_anti_chattering`), **không xử lý logic nghiệp vụ nặng ngay trong
ngắt**. Logic thật (ví dụ quyết định tắt `MC_P_ON`) chạy trong vòng lặp `while(1)` chính ở `entry.c`,
đọc lại các cờ/kết quả debounce này. `[GENERAL MCU KNOWLEDGE]` Đây là thực hành chuẩn trong lập trình
nhúng: **ISR nên càng ngắn càng tốt**, để không chặn các ngắt khác hoặc làm trễ hệ thống.

Tiếp theo: [07_writepin_bsrr_output_ordering.md](07_writepin_bsrr_output_ordering.md).
