# `MX_Init()`: sequence diagram đệ quy tới tận thanh ghi

Nguồn: `pscpu_s800/main/Src/mx_init.c` ([mx_init.c:81](../pscpu_s800/main/Src/mx_init.c#L81)).
Chip: **STM32F031C6Tx** (`PowerSubCPU.ioc`: `Mcu.UserName=STM32F031C6Tx`), HCLK = 48 MHz.

Quy ước tag:

| Tag           | Nghĩa                                                                                                           |     |
| ------------- | --------------------------------------------------------------------------------------------------------------- | --- |
| `[SOURCE]`    | Đã đọc trong code của repo (HAL, CMSIS, mx_init.c, hal_msp.c)                                                   |     |
| `[RM0091]`    | Kiến thức chung về STM32F0 (reference manual), **chưa** đọc trong repo. Dùng cho vị trí bit và offset thanh ghi |     |
| `[INFERENCE]` | Suy luận, code không nói thẳng                                                                                  |     |

---

## 0. Bản đồ địa chỉ thanh ghi

Base address đọc từ `stm32f031x6.h` `[SOURCE]`. Offset thanh ghi lấy từ `[RM0091]`.

| Khối | Base | Thanh ghi dùng trong `MX_Init` (offset) |
|---|---|---|
| RCC | `0x40021000` | CR `+00`, CFGR `+04`, AHBENR `+14`, APB2ENR `+18`, APB1ENR `+1C`, BDCR `+20`, CSR `+24`, CFGR2 `+2C`, CFGR3 `+30` |
| FLASH (interface) | `0x40022000` | ACR `+00` |
| PWR | `0x40007000` | CR `+00` |
| GPIOA / B / C / F | `0x48000000` / `0x48000400` / `0x48000800` / `0x48001400` | MODER `+00`, OTYPER `+04`, OSPEEDR `+08`, PUPDR `+0C`, BSRR `+18`, AFRL `+20`, AFRH `+24`, BRR `+28` |
| SYSCFG | `0x40010000` | EXTICR1..4 `+08..+14` |
| EXTI | `0x40010400` | IMR `+00`, EMR `+04`, RTSR `+08`, FTSR `+0C` |
| I2C1 | `0x40005400` | CR1 `+00`, CR2 `+04`, OAR1 `+08`, OAR2 `+0C`, TIMINGR `+10` |
| RTC | `0x40002800` | TR `+00`, DR `+04`, CR `+08`, ISR `+0C`, PRER `+10`, ALRMAR `+1C`, WPR `+24`, TAFCR `+40`, ALRMASSR `+44`, BKP0R `+50`, BKP1R `+54` |
| TIM3 | `0x40000400` | CR1 `+00`, CR2 `+04`, SMCR `+08`, DIER `+0C`, SR `+10`, EGR `+14`, PSC `+28`, ARR `+2C` |
| IWDG | `0x40003000` | KR `+00`, PR `+04`, RLR `+08`, SR `+0C`, WINR `+10` |
| SysTick (core) | `0xE000E010` | CTRL `+00`, LOAD `+04`, VAL `+08` |
| NVIC (core) | `0xE000E100` | ISER `+00`, IPR0..7 tại `0xE000E400` |
| SCB (core) | `0xE000ED00` | SHPR2 `0xE000ED1C` (SVC), SHPR3 `0xE000ED20` (PendSV, SysTick) |

Số ngắt `[SOURCE]` (`stm32f031x6.h`): RTC=2, EXTI0_1=5, EXTI2_3=6, EXTI4_15=7, TIM3=16, I2C1=23; SVC=-5, PendSV=-2, SysTick=-1.

---

## 1. Tổng quan: thứ tự `MX_Init()`

```mermaid
sequenceDiagram
    autonumber
    participant Main as main (entry.c)
    participant MX as MX_Init
    participant HAL as HAL_Init
    participant CLK as SystemClock_Config
    participant GPIO as MX_GPIO_Init
    participant I2C as MX_I2C1_Init
    participant RTC as khối RTC (4 hàm)
    participant TIM as MX_TIM3_Init
    participant WDG as MX_IWDG_Init

    Main->>MX: MX_Init()
    MX->>HAL: HAL_Init()
    Note right of HAL: SysTick tạm theo HSI 8 MHz, bật SYSCFG, đặt ưu tiên SVC/PendSV/SysTick
    MX->>CLK: SystemClock_Config()
    Note right of CLK: HSI trim, LSI on, LSE on, PLL x12 thành 48 MHz, Flash 1 WS, nguồn RTC = LSE, I2C1 = HSI, SysTick lại theo 48 MHz
    MX->>GPIO: MX_GPIO_Init()
    Note right of GPIO: bật clock GPIOA/B/C/F, cấu hình toàn bộ chân, kéo output về Low, bật 3 ngắt EXTI trong NVIC
    MX->>I2C: MX_I2C1_Init()
    Note right of I2C: chân PB6/PB7 AF1 open-drain, địa chỉ slave, bật ngắt I2C1
    MX->>RTC: đọc RTC_ISR.INITS rồi rẽ nhánh
    Note right of RTC: INITS=0 thì khởi tạo đầy đủ và đặt giờ mặc định. INITS=1 thì chỉ bật clock và NVIC. Sau đó Restore và đặt Alarm A
    MX->>TIM: MX_TIM3_Init()
    Note right of TIM: chỉ cấu hình, chưa chạy (CEN chưa bật)
    MX->>WDG: MX_IWDG_Init()
    Note right of WDG: nạp PR và RLR, chưa chạy (chưa ghi 0xCCCC)
    MX-->>Main: return
    Main->>Main: get_model, checksum, BSP_WDT_Start, km_it_init, BSP_TIM_Start
```

**Giải thích bước.**

| # | Bước | Vì sao ở vị trí này |
|---|---|---|
| 1 | `HAL_Init` | Dựng nền tảng thời gian (SysTick) và ưu tiên ngắt hệ thống. Mọi hàm HAL sau đó dùng `HAL_GetTick()` để chờ timeout |
| 2 | `SystemClock_Config` | Đưa CPU lên 48 MHz và bật LSE/LSI. Phải có trước khi cấu hình ngoại vi vì các ngoại vi (RTC, I2C) cần đúng nguồn clock |
| 3 | `MX_GPIO_Init` | Đặt trạng thái an toàn cho các chân điều khiển nguồn (tất cả Low) trước khi các khối khác chạy |
| 4 | `MX_I2C1_Init` | Mở cổng giao tiếp slave để S800 có thể nói chuyện |
| 5 | Khối RTC | Cần LSE (bước 2) và PWR/backup domain |
| 6 | `MX_TIM3_Init` | Chuẩn bị timer 1 ms. Việc chạy do `BSP_TIM_Start` làm sau |
| 7 | `MX_IWDG_Init` | Chuẩn bị watchdog. Việc chạy do `BSP_WDT_Start` làm sau |

---

## 2. `HAL_Init()`

```mermaid
sequenceDiagram
    autonumber
    participant HI as HAL_Init (stm32f0xx_hal.c dòng 157)
    participant IT as HAL_InitTick (dòng 238)
    participant ST as HAL_SYSTICK_Config
    participant CM as SysTick_Config (core_cm0.h dòng 769)
    participant NV as HAL_NVIC_SetPriority
    participant MSP as HAL_MspInit (main/Src/stm32f0xx_hal_msp.c dòng 45)
    participant REG as Thanh ghi

    HI->>REG: nếu PREFETCH_ENABLE khác 0: FLASH_ACR bit PRFTBE = 1
    HI->>IT: HAL_InitTick(TICK_INT_PRIORITY)
    IT->>ST: HAL_SYSTICK_Config(HCLK / 1000)
    ST->>CM: SysTick_Config(ticks)
    CM->>REG: SysTick_LOAD (0xE000E014) = ticks - 1
    CM->>NV: NVIC_SetPriority(SysTick, mức thấp nhất)
    NV->>REG: SCB_SHPR3 (0xE000ED20) byte 3 = 0xC0
    CM->>REG: SysTick_VAL (0xE000E018) = 0
    CM->>REG: SysTick_CTRL (0xE000E010) = CLKSOURCE, TICKINT, ENABLE
    IT->>NV: HAL_NVIC_SetPriority(SysTick, TickPriority, 0)
    NV->>REG: SCB_SHPR3 byte 3 = TickPriority << 6
    HI->>MSP: HAL_MspInit()
    MSP->>REG: RCC_APB2ENR (0x40021018) bit SYSCFGEN = 1, đọc lại để trễ
    MSP->>NV: SetPriority(SVC, 0, 0)
    NV->>REG: SCB_SHPR2 (0xE000ED1C) byte 3 = 0
    MSP->>NV: SetPriority(PendSV, 0, 0)
    NV->>REG: SCB_SHPR3 byte 2 = 0
    MSP->>NV: SetPriority(SysTick, 0, 0)
    NV->>REG: SCB_SHPR3 byte 3 = 0
```

**Giải thích.**

1. `[SOURCE]` `HAL_Init` chỉ làm 3 việc: bật prefetch (nếu cấu hình bật), `HAL_InitTick`, `HAL_MspInit`. Tôi chưa kiểm tra giá trị `PREFETCH_ENABLE`.
2. `[SOURCE]` `HAL_InitTick` lấy `HAL_RCC_GetHCLKFreq()/1000`. Ở lúc này CPU còn chạy HSI (mặc định sau reset). `[RM0091]` HSI = 8 MHz, nên `LOAD = 7999` cho nhịp 1 ms. Tôi chưa đọc thân `HAL_RCC_GetHCLKFreq` nên giá trị cụ thể là suy ra.
3. `[SOURCE]` `SysTick_Config` ghi 3 thanh ghi SysTick: LOAD, VAL, CTRL. CTRL bật nguồn clock = HCLK, bật ngắt, bật đếm. Bit nào ở CTRL: `[RM0091]` bit0 ENABLE, bit1 TICKINT, bit2 CLKSOURCE.
4. `[SOURCE]` `NVIC_SetPriority` cho ngắt lõi (số âm) ghi vào `SCB->SHP[]`. Với số dương ghi vào `NVIC->IP[]`. Cortex-M0 có 2 bit ưu tiên (`__NVIC_PRIO_BITS = 2`), nên giá trị được dịch trái `8 - 2 = 6` bit.
5. `[SOURCE]` `HAL_MspInit` do dự án định nghĩa lại ở `main/Src/stm32f0xx_hal_msp.c:45`: bật clock SYSCFG (cần cho EXTI sau này) và đặt SVC, PendSV, SysTick đều ưu tiên 0 (cao nhất).

---

## 3. `SystemClock_Config()`

### 3a. `HAL_RCC_OscConfig` (osc: HSI + LSI + LSE + PLL)

```mermaid
sequenceDiagram
    autonumber
    participant SC as SystemClock_Config (mx_init.c dòng 113)
    participant OC as HAL_RCC_OscConfig (hal_rcc.c dòng 271)
    participant REG as Thanh ghi

    SC->>OC: OscillatorType = HSI, LSI, LSE. LSE=ON, HSI=ON (trim 16), LSI=ON, PLL=ON (nguồn HSI, x12, PREDIV 1)

    Note over OC: Khối HSI
    OC->>REG: đọc RCC_CFGR.SWS: HSI đang là SYSCLK
    OC->>REG: RCC_CR.HSITRIM[7:3] = 16 (MODIFY_REG, chỉ chỉnh trim)

    Note over OC: Khối LSI
    OC->>REG: RCC_CSR.LSION = 1
    loop đến khi LSIRDY = 1 (timeout)
        OC->>REG: đọc RCC_CSR.LSIRDY
    end

    Note over OC: Khối LSE
    OC->>REG: nếu RCC_APB1ENR.PWREN = 0 thì đặt 1 (nhớ để tắt lại)
    OC->>REG: nếu PWR_CR.DBP = 0 thì đặt 1, chờ DBP = 1
    OC->>REG: RCC_BDCR.LSEON = 1
    loop đến khi LSERDY = 1 (timeout)
        OC->>REG: đọc RCC_BDCR.LSERDY
    end
    OC->>REG: nếu đã bật PWREN ở trên thì RCC_APB1ENR.PWREN = 0 (DBP vẫn = 1)

    Note over OC: Khối PLL (SYSCLK chưa phải PLL)
    OC->>REG: RCC_CR.PLLON = 0, chờ PLLRDY = 0
    OC->>REG: RCC_CFGR2.PREDIV = DIV1
    OC->>REG: RCC_CFGR.PLLMUL = MUL12 và PLLSRC = HSI
    OC->>REG: RCC_CR.PLLON = 1, chờ PLLRDY = 1
    OC-->>SC: HAL_OK
```

**Giải thích.**

1. `[SOURCE]` HSI: vì lúc gọi SYSCLK đang là HSI (`SWS == HSI`), HAL đi vào nhánh "HSI đang làm clock hệ thống: chỉ cho phép chỉnh calibration". Nó không tắt/bật HSI mà chỉ ghi `HSITRIM`. Giá trị 16 là mặc định của chip nên thực chất không đổi.
2. `[SOURCE]` LSI: `CSR.LSION`, rồi poll `LSIRDY`. LSI là clock 40 kHz nội dùng cho watchdog IWDG.
3. `[SOURCE]` LSE: ghi vào `BDCR` cần được phép ghi vào backup domain, nên HAL bật clock PWR rồi đặt `PWR_CR.DBP`. Sau khi bật LSE, HAL tắt lại clock PWR nếu chính nó bật, **nhưng `DBP` vẫn giữ 1**, nên các ghi RTC về sau không bị chặn.
4. `[SOURCE]` PLL: không được sửa khi PLL đang chạy, nên HAL tắt trước, cấu hình, bật, chờ khóa. Hệ số `PLLMUL = ×12`.
5. `[INFERENCE]` Tôi **chưa giải mã** bit `PLLSRC` thực tế (`HSI/2` hay `HSI/PREDIV`) vì macro `RCC_PLLSOURCE_HSI` có nhiều định nghĩa tùy dòng chip. Kết quả cuối được `.ioc` ghi là 48 MHz (`RCC.SYSCLKFreq_VALUE=48000000`), khớp với 8 MHz / 2 × 12.

### 3b. `HAL_RCC_ClockConfig` (SYSCLK = PLL, 1 wait state)

```mermaid
sequenceDiagram
    autonumber
    participant SC as SystemClock_Config
    participant CC as HAL_RCC_ClockConfig (hal_rcc.c dòng 727)
    participant REG as Thanh ghi
    participant IT as HAL_InitTick

    SC->>CC: ClockType = HCLK, SYSCLK, PCLK1. SYSCLK = PLL. AHB /1, APB1 /1. FLatency = 1
    CC->>REG: FLASH_ACR.LATENCY = 1 (vì 1 lớn hơn mức hiện tại 0)
    CC->>REG: đọc lại FLASH_ACR, xác nhận LATENCY = 1
    CC->>REG: RCC_CFGR.HPRE = DIV1
    CC->>REG: đọc RCC_CR.PLLRDY, phải = 1
    CC->>REG: RCC_CFGR.SW = PLL
    loop đến khi SWS = PLL (timeout)
        CC->>REG: đọc RCC_CFGR.SWS
    end
    CC->>REG: RCC_CFGR.PPRE = DIV1
    CC->>CC: SystemCoreClock = GetSysClockFreq() >> AHBPrescTable[HPRE]
    CC->>IT: HAL_InitTick(TICK_INT_PRIORITY)
    IT->>REG: SysTick_LOAD = 48000 - 1, VAL = 0, CTRL = ENABLE | TICKINT | CLKSOURCE
    CC-->>SC: HAL_OK
```

**Giải thích.**

1. `[SOURCE]` Thứ tự an toàn: **tăng wait state của Flash trước khi tăng tần số**. 48 MHz cần 1 wait state (`FLASH_LATENCY_1`). Nếu chuyển clock trước, CPU đọc Flash sai.
2. `[SOURCE]` Chỉ khi `PLLRDY = 1` HAL mới cho đổi `SW = PLL`, sau đó poll `SWS` để chắc chắn chuyển xong.
3. `[SOURCE]` Cuối hàm cập nhật biến `SystemCoreClock` và gọi lại `HAL_InitTick`. Từ giờ `SysTick_LOAD` được nạp theo 48 MHz, nên SysTick vẫn ngắt mỗi 1 ms dù clock đã nhanh gấp 6.
4. `[SOURCE]` `HPRE`/`PPRE` đều /1, vậy PCLK1 = HCLK = 48 MHz.

### 3c. `HAL_RCCEx_PeriphCLKConfig` (RTC = LSE, I2C1 = HSI)

```mermaid
sequenceDiagram
    autonumber
    participant SC as SystemClock_Config
    participant PC as HAL_RCCEx_PeriphCLKConfig (hal_rcc_ex.c dòng 122)
    participant REG as Thanh ghi

    SC->>PC: PeriphClockSelection = I2C1, RTC. RTC = LSE. I2C1 = HSI
    Note over PC: Khối RTC
    PC->>REG: nếu PWREN = 0 thì RCC_APB1ENR.PWREN = 1
    PC->>REG: nếu PWR_CR.DBP = 0 thì DBP = 1, chờ
    PC->>REG: đọc RCC_BDCR.RTCSEL
    alt RTCSEL khác 0 và khác LSE
        PC->>REG: BDCR.BDRST = 1 rồi 0 (reset backup domain), khôi phục BDCR, chờ LSERDY
    end
    PC->>REG: RCC_BDCR.RTCSEL[9:8] = LSE (01)
    PC->>REG: nếu đã bật PWREN ở trên thì tắt lại
    Note over PC: Khối I2C1
    PC->>REG: RCC_CFGR3.I2C1SW (bit 4) = 0 (nguồn HSI)
    PC-->>SC: HAL_OK
```

**Giải thích.**

1. `[SOURCE]` Chỉ được đổi nguồn clock RTC khi RTC chưa chọn nguồn hoặc đã reset backup domain. Nếu lần boot trước RTC đã chọn LSE (trường hợp thường gặp khi nguồn được cắm lại), `RTCSEL` đã bằng LSE nên HAL **không reset**, và ghi lại cùng giá trị (an toàn).
2. `[SOURCE]` Chỉ khi `RTCSEL` khác 0 và khác LSE mới xóa backup domain (điều này sẽ xóa toàn bộ trạng thái RTC).
3. `[SOURCE]` I2C1 lấy clock từ HSI (không phải SYSCLK). `[INFERENCE]` Lựa chọn này giúp I2C có clock độc lập với PLL (HSI vẫn hoạt động khi chip thức dậy từ STOP, xem `SystemClock_Config2` trong cùng file). Chưa tìm thấy comment xác nhận.

### 3d. SysTick (lần cuối)

```mermaid
sequenceDiagram
    autonumber
    participant SC as SystemClock_Config
    participant ST as HAL_SYSTICK_Config
    participant CS as HAL_SYSTICK_CLKSourceConfig
    participant NV as HAL_NVIC_SetPriority
    participant REG as Thanh ghi

    SC->>ST: HAL_SYSTICK_Config(HCLK / 1000)
    ST->>REG: SysTick_LOAD = 47999, VAL = 0, CTRL = 0b111
    SC->>CS: SYSTICK_CLKSOURCE_HCLK
    CS->>REG: SysTick_CTRL |= CLKSOURCE (bit 2)
    SC->>NV: SetPriority(SysTick, 0, 0)
    NV->>REG: SCB_SHPR3 byte 3 = 0
```

`[SOURCE]` Đây là bước lặp lại cố ý do code CubeMX sinh ra: giá trị LOAD cuối cùng bằng `HCLK/1000 - 1 = 47999` và ưu tiên SysTick đặt về 0.

---

## 4. `MX_GPIO_Init()` (và `HAL_GPIO_Init` ở mức thanh ghi)

### 4a. Thuật toán chung của `HAL_GPIO_Init` `[SOURCE]` (hal_gpio.c:188)

```mermaid
sequenceDiagram
    autonumber
    participant HG as HAL_GPIO_Init
    participant REG as Thanh ghi GPIOx / SYSCFG / EXTI

    loop với mỗi bit pin được chọn trong mask
        alt chế độ AF_PP hoặc AF_OD
            HG->>REG: GPIOx_AFR[pos >> 3]: xóa 4 bit rồi ghi số AF
        end
        HG->>REG: GPIOx_MODER[2 bit]: 00 vào, 01 ra, 10 AF, 11 analog
        alt chế độ output hoặc AF
            HG->>REG: GPIOx_OSPEEDR[2 bit] = Speed
            HG->>REG: GPIOx_OTYPER[1 bit] = PP(0) hoặc OD(1)
        end
        HG->>REG: GPIOx_PUPDR[2 bit] = Pull
        alt chế độ có EXTI (IT hoặc EVT)
            HG->>REG: RCC_APB2ENR.SYSCFGEN = 1
            HG->>REG: SYSCFG_EXTICR[pos >> 2]: chọn cổng (A=0, B=1, C=2, F=5) cho line pos
            HG->>REG: EXTI_IMR bit pos = 1 (nếu IT)
            HG->>REG: EXTI_EMR bit pos = 0 (nếu không EVT)
            HG->>REG: EXTI_RTSR bit pos = 1 nếu có cạnh lên, ngược lại 0
            HG->>REG: EXTI_FTSR bit pos = 1 nếu có cạnh xuống, ngược lại 0
        end
    end
```

Giá trị cấu hình `[SOURCE]` (`stm32f0xx_hal_gpio.h`): `INPUT=0x0`, `OUTPUT_PP=0x1`, `AF_OD=0x12`, `IT_FALLING=0x10210000`, `IT_RISING_FALLING=0x10310000`.

### 4b. Trình tự `MX_GPIO_Init` `[SOURCE]` (mx_init.c:419)

```mermaid
sequenceDiagram
    autonumber
    participant GI as MX_GPIO_Init
    participant HG as HAL_GPIO_Init
    participant WP as HAL_GPIO_WritePin
    participant NV as HAL_NVIC_SetPriority / EnableIRQ
    participant REG as Thanh ghi

    GI->>REG: RCC_AHBENR (0x40021014): IOPCEN, IOPFEN, IOPAEN, IOPBEN = 1
    GI->>HG: GPIOF: PF0, PF1 = INPUT, không pull
    GI->>HG: GPIOA: PA1 (_HRESET_REQ) = IT_FALLING
    HG->>REG: EXTICR1[7:4] = 0 (port A), EXTI IMR bit1, FTSR bit1
    GI->>HG: GPIOA: PA2, PA4, PA5, PA8 = IT_RISING_FALLING
    HG->>REG: EXTICR1[11:8], EXTICR2[3:0], EXTICR2[7:4], EXTICR3[3:0] = 0, IMR + RTSR + FTSR bit 2/4/5/8
    GI->>HG: GPIOA: PA6 (SLEEP_STATUS_REM_SP) = OUTPUT_PP, speed thấp
    GI->>HG: GPIOB: PB0 PB1 PB2 PB3 PB5 PB9 PB10 PB12 PB13 = OUTPUT_PP
    GI->>HG: GPIOB: PB4 PB8 PB14 PB15 = INPUT, không pull
    GI->>HG: GPIOA: PA11, PA15 = INPUT, không pull
    GI->>HG: GPIOA: PA9, PA10 = INPUT, pull-down
    GI->>HG: GPIOB: PB4, PB8 = INPUT (lặp lại)
    GI->>WP: GPIOA PA6 = RESET
    WP->>REG: GPIOA_BRR (0x48000028) = 1 << 6
    GI->>WP: GPIOB nhóm 9 chân output = RESET
    WP->>REG: GPIOB_BRR (0x48000428) = các bit 0,1,2,3,5,9,10,12,13
    GI->>NV: EXTI0_1: ưu tiên 0, bật
    NV->>REG: NVIC_IPR1 (0xE000E404) byte 1 = 0, NVIC_ISER (0xE000E100) bit 5
    GI->>NV: EXTI2_3: ưu tiên 0, bật
    NV->>REG: NVIC_ISER bit 6
    GI->>NV: EXTI4_15: ưu tiên 0, bật
    NV->>REG: NVIC_ISER bit 7
```

**Bảng chân sau `MX_GPIO_Init`** `[SOURCE]` (`mxconstants.h`):

| Chân | Tên | Chế độ | Ghi chú |
|---|---|---|---|
| PA1 | `_HRESET_REQ` | EXTI cạnh xuống | S800 yêu cầu reset |
| PA2 | `AP_PWR_EN` | EXTI hai cạnh | S800 báo nguồn AP |
| PA4 | `POWER_MONITOR` | EXTI hai cạnh | Giám sát nguồn |
| PA5 | `MSW_ON` | EXTI hai cạnh | Công tắc nguồn chính |
| PA8 | `MONI_24V11` | EXTI hai cạnh | Giám sát 24V |
| PA6 | `SLEEP_STATUS_REM_SP` | Output | Mặc định Low |
| PB0 | `IR_P_ON` | Output | Mặc định Low |
| PB1 | `ERP_SENSOR_ON` | Output | Mặc định Low |
| PB2 | `_RESET` | Output | Mặc định Low, tức S800 đang bị reset |
| PB3 | `SB_PWR_EN` | Output | Mặc định Low, nguồn SB tắt |
| PB5 | `MC_P_ON` | Output | Mặc định Low |
| PB9 | `_RST_SLP2` | Output | Mặc định Low |
| PB10 | `_USB2_OE` | Output | Mặc định Low |
| PB12 | `SLEEP_STATUS_REM_EG` | Output | Mặc định Low |
| PB13 | `MC_PWR_EN` | Output | Mặc định Low |
| PB4 | `SB_PG` | Input | Báo nguồn SB tốt |
| PB8 | `_DISCHG` | Input | |
| PB14, PB15 | `MODEL_BIT0/1` | Input | Đọc mã model |
| PA11 | `MC_PG` | Input | |
| PA15 | `MC3_3VON_MONI` | Input | |
| PA9, PA10 | `PA9_IN`, `PA10_IN` | Input pull-down | |
| PF0, PF1 | `PF0_IN`, `PF1_IN` | Input | |

**Giải thích.**

1. `[SOURCE]` Bật clock cổng trước: nếu clock cổng tắt, ghi vào thanh ghi `GPIOx` không có hiệu lực.
2. `[SOURCE]` Mỗi nhóm chân gọi `HAL_GPIO_Init` một lần với `Pin` là mask OR nhiều chân. HAL lặp qua từng bit.
3. `[SOURCE]` Với chân EXTI: `SYSCFG_EXTICR` chọn cổng nào nối vào đường EXTI tương ứng; sau đó `IMR` mở mask ngắt, `RTSR`/`FTSR` chọn cạnh.
4. `[RM0091]` Thanh ghi `ODR` có giá trị reset là 0, nên chân output đã ở Low ngay khi `MODER` chuyển sang output, và `BRR` sau đó chỉ khẳng định lại. Nhờ vậy `_RESET`, `SB_PWR_EN`, `MC_P_ON`... không bị xung nhiễu lúc khởi động. Đây là **nền tảng cho chuỗi bật nguồn**: mọi nguồn đều tắt, S800 đang bị giữ reset, cho tới khi firmware chủ động bật.
5. `[SOURCE]` Cả 3 ngắt EXTI được bật ngay (ưu tiên 0), **trước khi** `km_it_init()` đăng ký callback. `[INFERENCE]` Nếu có cạnh xảy ra trong khoảng này thì `signal_event` còn `NULL` và sự kiện bị bỏ. Đoạn đó rất ngắn, nhưng đáng biết.
6. `[SOURCE]` Tất cả ngắt đều ưu tiên 0, nên không ngắt nào chen ngang được ngắt khác.

---

## 5. `MX_I2C1_Init()`

```mermaid
sequenceDiagram
    autonumber
    participant MI as MX_I2C1_Init (mx_init.c dòng 202)
    participant HI as HAL_I2C_Init (hal_i2c.c dòng 403)
    participant MSP as HAL_I2C_MspInit (hal_msp.c dòng 66)
    participant HG as HAL_GPIO_Init
    participant NV as NVIC
    participant AF as HAL_I2CEx_ConfigAnalogFilter
    participant REG as Thanh ghi

    MI->>MI: hi2c1: Timing 0x00200000, Own1 122, 7 bit, dual ON, Own2 124, no general call, stretch ON
    MI->>HI: HAL_I2C_Init(hi2c1)
    HI->>MSP: State == RESET nên gọi MspInit
    MSP->>HG: PB6, PB7 = AF_OD, no pull, speed HIGH, AF1
    HG->>REG: GPIOB_AFRL (0x48000420): nibble 6 và 7 = 0x1
    HG->>REG: GPIOB_MODER bit 13:12 và 15:14 = 10
    HG->>REG: GPIOB_OSPEEDR = 11, GPIOB_OTYPER bit 6 và 7 = 1, GPIOB_PUPDR = 00
    MSP->>REG: RCC_APB1ENR (0x4002101C) bit I2C1EN = 1
    MSP->>NV: SetPriority(I2C1, 0) và EnableIRQ(I2C1)
    NV->>REG: NVIC_IPR5 (0xE000E414) byte 3 = 0, NVIC_ISER bit 23
    HI->>REG: I2C1_CR1.PE = 0 (tắt ngoại vi)
    HI->>REG: I2C1_TIMINGR = 0x00200000 AND 0xF0FFFFFF
    HI->>REG: I2C1_OAR1 bỏ OA1EN, rồi = OA1EN | 122
    HI->>REG: I2C1_CR2 = CR2 | AUTOEND | NACK
    HI->>REG: I2C1_OAR2 = OA2EN | 124 | (mask 0 << 8)
    HI->>REG: I2C1_CR1 = 0 (không general call, không tắt stretch)
    HI->>REG: I2C1_CR1.PE = 1 (bật)
    HI-->>MI: HAL_OK
    MI->>AF: ConfigAnalogFilter(ENABLE)
    AF->>REG: CR1.PE = 0, CR1.ANFOFF = 0, CR1 |= 0, CR1.PE = 1
    AF-->>MI: HAL_OK
```

**Giải thích.**

1. `[SOURCE]` PB6 = SCL, PB7 = SDA (comment trong `MspInit`), chọn AF1, open-drain. Open-drain là bắt buộc cho I2C: chỉ kéo xuống, điện trở kéo lên bên ngoài kéo lên.
2. `[SOURCE]` `TIMINGR = 0x00200000` giá trị này không tính lại tốc độ. `[RM0091]` bit 23:20 là `SCLDEL = 2`, các trường còn lại (PRESC, SCLH, SCLL, SDADEL) = 0. Đây là slave nên SCL do S800 tạo; `SCLDEL` chỉ ảnh hưởng thời gian setup dữ liệu.
3. `[SOURCE]` Hai địa chỉ slave: `OwnAddress1 = 122 (0x7A)`, `OwnAddress2 = 124 (0x7C)`. HAL ghi thẳng vào `OAR1`/`OAR2`. `[RM0091]` trường địa chỉ 7-bit nằm ở bit [7:1], nên 7-bit address thực là **0x3D** và **0x3E** (tức byte ghi 0x7A và 0x7C). `[INFERENCE]` Tôi đoán đây là hai địa chỉ mà S800 dùng, chưa đối chiếu với phía S800.
4. `[SOURCE]` `CR2 |= AUTOEND | NACK`. Comment HAL: "NACK chỉ được tắt trong quá trình slave" — chính là ngầm giải thích vì sao `i2c_recv_first` dùng **manual ACK**.
5. `[SOURCE]` Mỗi lần đổi cấu hình, HAL tắt `PE` rồi bật lại (thay đổi `TIMINGR`, `OAR`, `ANFOFF` chỉ được phép khi `PE = 0`, `[RM0091]`).
6. `[SOURCE]` Bộ lọc nhiễu analog bật (`ANFOFF = 0`).
7. `[SOURCE]` Ngắt I2C1 (IRQ 23) được bật trong `MspInit` **trước** khi ngoại vi cấu hình xong. `[INFERENCE]` Không sao vì các bit ngắt trong `CR1` (ADDRIE, RXIE...) chưa được bật ở thời điểm này; chúng được bật sau trong `Driver_I2C0.Initialize`.

---

## 6. Khối RTC (4 hàm)

Mã trong `MX_Init`:

```c
hrtc.Instance = RTC;
if(!(hrtc.Instance->ISR & RTC_ISR_INITS)) {   // INITS: lịch chưa được khởi tạo
    MX_RTC_Init(DEF_RTC_INIT);   KM_RTC_SetDefaultTime();
} else {
    MX_RTC_Init(DEF_RTC_MSPINIT);
}
KM_RTC_Restore();
KM_RTC_ALARM_Init();
```

### 6a. Quyết định nhánh

```mermaid
sequenceDiagram
    autonumber
    participant MX as MX_Init
    participant REG as RTC_ISR (0x4002280C)
    participant A as Nhánh A: MX_RTC_Init(0) + SetDefaultTime
    participant B as Nhánh B: MX_RTC_Init(1)

    MX->>REG: đọc ISR.INITS (bit 4)
    alt INITS = 0 (lịch chưa từng được đặt, ví dụ cắm nguồn lần đầu hoặc backup domain bị reset)
        MX->>A: khởi tạo đầy đủ, đặt giờ mặc định
    else INITS = 1 (RTC đã chạy)
        MX->>B: chỉ bật clock RTC và NVIC, không chạm vào cấu hình RTC
    end
    MX->>MX: KM_RTC_Restore()
    MX->>MX: KM_RTC_ALARM_Init()
```

`[SOURCE]` Comment ở `mx_init.c:67` và `:93` (`OP_BTS-21266`, "主電源OFF時消費電流増加問題対策"): nhánh B tồn tại để **tránh vào init mode RTC mỗi lần khởi động**, vì việc đó từng làm tăng dòng tiêu thụ khi công tắc nguồn chính tắt. `[INFERENCE]` Cơ chế vật lý chính xác của hiện tượng này tôi không kiểm chứng.

### 6b. Nhánh A: `MX_RTC_Init(0)` → `HAL_RTC_Init`

```mermaid
sequenceDiagram
    autonumber
    participant MR as MX_RTC_Init (mx_init.c dòng 244)
    participant HR as HAL_RTC_Init (hal_rtc.c dòng 152)
    participant MSP as HAL_RTC_MspInit (hal_msp.c dòng 126)
    participant EI as RTC_EnterInitMode (dòng 1314)
    participant BY as HAL_RTCEx_EnableBypassShadow
    participant REG as Thanh ghi RTC

    MR->>HR: Init: 24h, Async 127, Sync 255, Output ALARMA, Pol LOW, Type PUSHPULL
    HR->>MSP: State == RESET nên gọi MspInit
    MSP->>REG: RCC_BDCR (0x40021020).RTCEN = 1
    MSP->>REG: NVIC: SetPriority(RTC, 0), ISER bit 2
    HR->>REG: RTC_WPR (0x40002824) = 0xCA rồi 0x53 (mở khóa ghi)
    HR->>EI: RTC_EnterInitMode
    EI->>REG: RTC_ISR = 0xFFFFFFFF (đặt INIT)
    loop đến khi INITF = 1 (timeout)
        EI->>REG: đọc RTC_ISR.INITF
    end
    HR->>REG: RTC_CR bỏ FMT, OSEL, POL rồi OR (24h | OSEL=ALARMA 0x00200000 | POL=LOW 0x00100000)
    HR->>REG: RTC_PRER = 255 | (127 << 16)
    HR->>REG: RTC_ISR bỏ INIT (thoát init mode)
    HR->>REG: RTC_TAFCR bỏ ALARMOUTTYPE rồi OR 0x00040000 (push-pull)
    HR->>REG: RTC_WPR = 0xFF (khóa lại)
    HR-->>MR: HAL_OK
    MR->>BY: HAL_RTCEx_EnableBypassShadow
    BY->>REG: WPR mở khóa, RTC_CR.BYPSHAD = 1, WPR = 0xFF
```

**Giải thích.**

1. `[SOURCE]` Bảo vệ ghi RTC: phải ghi tuần tự `0xCA` rồi `0x53` vào `WPR` mới ghi được các thanh ghi RTC. Ghi `0xFF` (HAL) hoặc `0xFE` rồi `0x64` (code KM) để khóa lại.
2. `[SOURCE]` `PRER`: prescaler bất đồng bộ 127, đồng bộ 255. `[RM0091]` Với LSE 32,768 kHz: 32768 / (127+1) / (255+1) = 1 Hz, đúng 1 giây.
3. `[SOURCE]` `OSEL = ALARMA` và `POL = LOW` nghĩa là chân đầu ra RTC phát tín hiệu Alarm A, tích cực Low. `[INFERENCE]` Tôi không thấy chân RTC output nào trong bảng GPIO, nên chưa rõ nó nối đi đâu hoặc có dùng không.
4. `[SOURCE]` `BYPSHAD = 1`: đọc `TR`/`DR` trực tiếp từ bộ đếm, không qua thanh ghi bóng. Hệ quả: `HAL_RTC_SetTime`/`SetDate` **bỏ qua** bước `WaitForSynchro` (nhánh `if CR_BYPSHAD == RESET`).
5. `[SOURCE]` Comment `/* !!! CubeMXでコード生成後以下をマージする事 !!! */` cảnh báo: dòng `HAL_RTCEx_EnableBypassShadow` là sửa tay, cần **giữ lại** khi sinh lại code bằng CubeMX.

### 6c. Nhánh A (tiếp): `KM_RTC_SetDefaultTime`

```mermaid
sequenceDiagram
    autonumber
    participant KS as KM_RTC_SetDefaultTime (mx_init.c dòng 275)
    participant ST as HAL_RTC_SetTime (dòng 398)
    participant SD as HAL_RTC_SetDate (dòng 577)
    participant EI as RTC_EnterInitMode
    participant REG as Thanh ghi RTC

    KS->>REG: đọc RTC_BKP1R (0x40002854), kiểm byte [23:16] khác 0?
    alt không có backup (IS_BACKUP_EXIST sai)
        KS->>REG: RTC_WPR = 0xCA, 0x53
        KS->>ST: SetTime 08:30:00 (BCD)
        ST->>REG: WPR mở khóa, EnterInitMode (ISR = 0xFFFFFFFF, chờ INITF)
        ST->>REG: RTC_TR = 0x00083000, CR bỏ BCK, CR OR (DayLight|StoreOp), ISR bỏ INIT
        ST->>REG: WPR = 0xFF
        KS->>SD: SetDate 2017-12-05 (thứ Ba)
        SD->>REG: WPR mở khóa, EnterInitMode
        SD->>REG: RTC_DR = 0x00175205 (năm 0x17, tháng 0x12, ngày 0x05, thứ 2 << 13)
        SD->>REG: ISR bỏ INIT, WPR = 0xFF
        KS->>REG: RTC_WPR = 0xFE, 0x64
    else đã có backup
        KS->>KS: không làm gì
    end
```

`[SOURCE]` Giá trị mặc định `2017/12/05 08:30:00` (comment trong code). `TR = 0x08<<16 | 0x30<<8 = 0x00083000`. `DR = 0x17<<16 | 0x12<<8 | 0x05 | 2<<13 = 0x00175205` theo công thức ở `hal_rtc.c:615-618`; riêng giá trị hằng `RTC_MONTH_DECEMBER = 0x12` và `RTC_WEEKDAY_TUESDAY = 2` tôi lấy từ quy ước của HAL, chưa mở header để xác nhận.

### 6d. `KM_RTC_Restore`

```mermaid
sequenceDiagram
    autonumber
    participant KR as KM_RTC_Restore (mx_init.c dòng 310)
    participant REG as Thanh ghi RTC

    KR->>REG: đọc BKP1R (0x40002854) byte [23:16]
    alt có backup
        KR->>REG: WPR = 0xCA, 0x53
        KR->>REG: RTC_ISR |= INIT
        loop tối đa 10000 vòng
            KR->>REG: chờ ISR.INITF = 1
        end
        KR->>REG: RTC_TR = BKP0R (0x40002850)
        KR->>REG: RTC_DR = BKP1R (0x40002854)
        KR->>REG: RTC_ISR &= ~INIT
        loop tối đa 10000 vòng
            KR->>REG: chờ ISR.INITS = 1
        end
        KR->>REG: BKP0R = 0, BKP1R = 0 (xóa bản backup)
        KR->>REG: WPR = 0xFE, 0x64
    else không có backup
        KR->>KR: không làm gì
    end
```

`[SOURCE]` Comment `2023/06/30 河村 OP_BTS-44141`: phía **lưu** (`save_rtc_reg`) đã bị vô hiệu hóa, nhưng phía **khôi phục** vẫn giữ để tương thích với phiên bản firmware cũ (bản cũ lưu giờ vào BKP0R/BKP1R trước khi nhảy sang IAP). Với firmware hiện tại thanh ghi này thường bằng 0 nên hàm không làm gì.

### 6e. `KM_RTC_ALARM_Init` → `HAL_RTC_SetAlarm_IT`

```mermaid
sequenceDiagram
    autonumber
    participant KA as KM_RTC_ALARM_Init (mx_init.c dòng 352)
    participant SA as HAL_RTC_SetAlarm_IT (dòng 890)
    participant REG as Thanh ghi

    KA->>SA: Alarm A, 00:00:00, mask SECONDS, SubSecond mask ALL, chọn WEEKDAY, WeekDay 0, BCD
    SA->>SA: tmpreg = giờ, phút, giây, weekday, WDSEL, mask
    SA->>REG: RTC_WPR = 0xCA, 0x53
    SA->>REG: RTC_CR bỏ ALRAE (tắt Alarm A)
    SA->>REG: RTC_ISR bỏ cờ ALRAF
    loop đến khi ALRAWF = 1 (timeout)
        SA->>REG: đọc RTC_ISR.ALRAWF
    end
    SA->>REG: RTC_ALRMAR (0x4000281C) = tmpreg
    SA->>REG: RTC_ALRMASSR (0x40002844) = SubSeconds | mask
    SA->>REG: RTC_CR |= ALRAE (bật Alarm A)
    SA->>REG: RTC_CR |= ALRAIE (0x1000, bật ngắt Alarm A)
    SA->>REG: EXTI_IMR (0x40010400) |= bit 17
    SA->>REG: EXTI_RTSR (0x40010408) |= bit 17
    SA->>REG: RTC_WPR = 0xFF
```

**Giải thích.**

1. `[SOURCE]` Quy trình chuẩn của RTC: phải **tắt** Alarm A rồi chờ `ALRAWF = 1` mới được ghi `ALRMAR`; xong mới bật lại.
2. `[SOURCE]` Alarm A của RTC đi qua **EXTI line 17**; HAL phải mở `IMR` và `RTSR` ở EXTI mới có ngắt tới NVIC (IRQ RTC = 2, đã bật trong `HAL_RTC_MspInit`).
3. `[SOURCE]` Comment `RTC_ALARMMASK_SECONDS` ghi rõ: đây là giá trị **sửa tay** sau khi CubeMX sinh code. Giá trị là cấu hình ban đầu. Thời điểm báo thức thực tế dùng cho chế độ STOP được nạp lại bởi `set_cycle_time()` (gọi tại `km_extend_io.c:1202`). `[INFERENCE]` Tôi chưa đọc `set_cycle_time` nên chưa xác nhận được thời gian cụ thể; và tôi chưa đánh giá liệu báo thức khởi tạo này (đặt `weekday = 0`) có bao giờ khớp hay không.

---

## 7. `MX_TIM3_Init()`

```mermaid
sequenceDiagram
    autonumber
    participant MT as MX_TIM3_Init (mx_init.c dòng 381)
    participant HT as HAL_TIM_Base_Init (hal_tim.c dòng 205)
    participant MSP as HAL_TIM_Base_MspInit (hal_msp.c dòng 167)
    participant SC as TIM_Base_SetConfig (dòng 4573)
    participant CS as HAL_TIM_ConfigClockSource (dòng 3919)
    participant MS as HAL_TIMEx_MasterConfigSynchronization
    participant REG as Thanh ghi

    MT->>HT: TIM3, Prescaler 0, UP, Period 0, DIV1
    HT->>MSP: State == RESET nên gọi MspInit
    MSP->>REG: RCC_APB1ENR (0x4002101C) bit TIM3EN = 1
    MSP->>REG: NVIC: SetPriority(TIM3, 0), ISER bit 16
    HT->>SC: TIM_Base_SetConfig
    SC->>REG: TIM3_CR1: bỏ DIR, CMS, CKD, rồi OR (UP | DIV1)
    SC->>REG: TIM3_ARR (0x4000042C) = 0
    SC->>REG: TIM3_PSC (0x40000428) = 0
    SC->>REG: TIM3_EGR (0x40000414) = UG (nạp ngay PSC và ARR)
    HT-->>MT: HAL_OK
    MT->>CS: ClockSource = INTERNAL
    CS->>REG: TIM3_SMCR: xóa SMS, TS, ETF, ETPS, ECE, ETP
    CS->>REG: TIM3_SMCR &= ~SMS (clock nội trực tiếp)
    MT->>MS: TRGO = RESET, Master/Slave = DISABLE
    MS->>REG: TIM3_CR2 &= ~MMS rồi |= 0
    MS->>REG: TIM3_SMCR &= ~MSM rồi |= 0
```

**Giải thích.**

1. `[SOURCE]` Hàm này chỉ **dựng khung** timer: bật clock, bật ngắt NVIC, ghi giá trị placeholder `PSC = 0`, `ARR = 0`.
2. `[SOURCE]` **Bộ đếm chưa chạy**: `CR1.CEN` không được set, và `DIER.UIE` không được set. Việc đặt `PSC`/`ARR` thật (tạo 1 ms) và bật chạy là của `BSP_TIM_Start(TIMER_3, 1)` (đã phân tích ở câu trả lời trước), bằng cách `DeInit` rồi `Init` lại.
3. `[SOURCE]` `TIM3_IRQn` đã được bật ở NVIC từ đây nhưng timer chưa phát ngắt vì `UIE = 0`.
4. `[SOURCE]` Clock nguồn là nội (APB1 timer clock), không dùng chế độ master/slave hay trigger ngoài.

---

## 8. `MX_IWDG_Init()`

```mermaid
sequenceDiagram
    autonumber
    participant MW as MX_IWDG_Init (mx_init.c dòng 229)
    participant HW as HAL_IWDG_Init (hal_iwdg.c dòng 166)
    participant REG as Thanh ghi IWDG

    MW->>HW: Prescaler /256, Window 4095, Reload 4095
    HW->>REG: đọc IWDG_SR (0x4000300C): PVU, RVU, WVU không cùng đặt
    HW->>HW: State == RESET nên gọi HAL_IWDG_MspInit (hàm weak, rỗng)
    HW->>REG: IWDG_KR (0x40003000) = 0x5555 (mở khóa PR, RLR, WINR)
    HW->>REG: IWDG_PR (0x40003004).PR = 6 (chia 256)
    HW->>REG: IWDG_RLR (0x40003008).RL = 4095
    alt Window khác 4095 hoặc WINR khác 4095
        HW->>REG: chờ SR = 0 rồi ghi WINR
    else Window = 4095 và WINR = 4095 (trường hợp này)
        HW->>HW: bỏ qua, cửa sổ vô hiệu
    end
    HW-->>MW: HAL_OK
```

**Giải thích.**

1. `[SOURCE]` `HAL_IWDG_Init` chỉ **cấu hình** watchdog, không chạy nó. Việc chạy (ghi `KR = 0xCCCC`) nằm trong `HAL_IWDG_Start`, được gọi từ `BSP_WDT_Start()` ([entry.c:63](../pscpu_s800/main/App/entry.c#L63)). Tôi chưa đọc thân `HAL_IWDG_Start`, nên "ghi `0xCCCC`" là kiến thức `[RM0091]`.
2. `[SOURCE]` `IWDG_PRESCALER_256 = IWDG_PR_PR_2 | IWDG_PR_PR_1` tức `PR = 0b110`.
3. `[SOURCE]` `Window = 4095` bằng giá trị mặc định của `WINR`, nên nhánh ghi `WINR` bị bỏ qua: watchdog hoạt động ở chế độ thường, không có cửa sổ.
4. `[RM0091]` Với LSI danh định khoảng 40 kHz: mỗi nhịp = 256 / 40 000 ≈ 6,4 ms, nên `RL = 4095` cho timeout ≈ 26 giây. Con số này chỉ là ước tính, vì tần số LSI thực thay đổi theo chip và nhiệt độ.
5. `[SOURCE]` Vòng lặp chính gọi `BSP_WDT_Refresh()` mỗi lần lặp ([entry.c:89](../pscpu_s800/main/App/entry.c#L89)); nếu vòng lặp bị treo hơn timeout này thì chip tự reset.

---

## 9. Những gì tôi chưa xác minh

| Mục | Lý do |
|---|---|
| Giá trị `PREFETCH_ENABLE` | Chưa đọc `stm32f0xx_hal_conf.h` |
| Thân `HAL_RCC_GetHCLKFreq`, `HAL_RCC_GetSysClockFreq` | Chưa đọc. Giá trị HCLK trước và sau PLL là suy ra từ `.ioc` |
| Bit `PLLSRC` thực tế | Macro `RCC_PLLSOURCE_HSI` có nhiều định nghĩa theo dòng chip |
| Vị trí bit và offset thanh ghi | Lấy từ `[RM0091]` (kiến thức chung), chỉ **base address** và **số IRQ** là đọc từ `stm32f031x6.h` |
| `HAL_IWDG_Start` | Chưa đọc thân hàm |
| `set_cycle_time` | Chưa đọc, nên chưa kết luận về chu kỳ Alarm A thực dụng |
| Ý nghĩa của hai địa chỉ I2C (0x3D, 0x3E) | Cần đối chiếu với phía S800 |
