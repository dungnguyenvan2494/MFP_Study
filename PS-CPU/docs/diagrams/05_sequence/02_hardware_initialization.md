# Sequence Diagram — Hardware Initialization (chi tiết `MX_Init()`)

Các participant: **Application**, **HAL**, **Peripheral** (thanh ghi
RCC/GPIO/I2C1/RTC/TIM3/IWDG), **Hardware**. Mở rộng chi tiết khối
`MX_Init()` từ `01_boot_sequence.md`.

```mermaid
sequenceDiagram
    participant APP as Application
    participant HAL as HAL
    participant PER as Peripheral (RCC/GPIO/I2C1/RTC/TIM3/IWDG)

    APP->>HAL: HAL_Init()
    APP->>HAL: SystemClock_Config()
    Note right of APP: mx_init.c:113
    HAL->>PER: HAL_RCC_OscConfig(HSI+LSI+LSE, PLL×12)
    alt != HAL_OK
        HAL-->>APP: Error_Handler()
        Note right of APP: mx_init.c:132
    end
    HAL->>PER: HAL_RCC_ClockConfig(PLLCLK, FLASH_LATENCY_1)
    alt != HAL_OK
        HAL-->>APP: Error_Handler()
        Note right of APP: mx_init.c:142
    end
    HAL->>PER: HAL_RCCEx_PeriphCLKConfig(I2C1←HSI, RTC←LSE)
    alt != HAL_OK
        HAL-->>APP: Error_Handler()
        Note right of APP: mx_init.c:150
    end
    HAL->>PER: HAL_SYSTICK_Config(HCLK/1000)
    Note right of PER: 1kHz — 09_timing.md §1

    APP->>APP: MX_GPIO_Init()
    Note right of APP: configures all 14 io_extend[] pins + EXTI lines

    APP->>HAL: MX_I2C1_Init()
    Note right of APP: mx_init.c:202 — OwnAddress1=122, OwnAddress2=124
    HAL->>PER: HAL_I2C_Init(&hi2c1)
    alt != HAL_OK
        HAL-->>APP: Error_Handler()
        Note right of APP: mx_init.c:216
    end
    HAL->>PER: HAL_I2CEx_ConfigAnalogFilter(ENABLE)
    alt != HAL_OK
        HAL-->>APP: Error_Handler()
        Note right of APP: mx_init.c:223
    end
    Note over HAL,PER: HAL_I2C_MspInit() (weak override) runs here:<br/>configures PB6/PB7 AF_OD, HAL_NVIC_SetPriority(I2C1_IRQn,0,0),<br/>HAL_NVIC_EnableIRQ(I2C1_IRQn) — stm32f0xx_hal_msp.c:90-91

    alt RTC not yet initialized (fresh part)
        APP->>PER: MX_RTC_Init(DEF_RTC_INIT)
        APP->>PER: KM_RTC_SetDefaultTime()
    else RTC backup domain already valid
        APP->>PER: MX_RTC_Init(DEF_RTC_MSPINIT)
    end
    APP->>PER: KM_RTC_Restore()
    Note right of APP: mx_init.c:310 — [TIMING UNKNOWN precision,<br/>~2ms iteration-count spin, 09_timing.md §6.1]
    APP->>PER: KM_RTC_ALARM_Init()
    Note over HAL,PER: HAL_RTC_MspInit() runs here: HAL_NVIC_SetPriority(RTC_IRQn,0,0),<br/>HAL_NVIC_EnableIRQ(RTC_IRQn) — stm32f0xx_hal_msp.c:137-138

    APP->>PER: MX_TIM3_Init()
    Note right of APP: configures only — does NOT start<br/>(HAL_TIM_Base_MspInit sets priority+enables TIM3_IRQn,<br/>hal_msp.c:178-179, but counter isn't running yet)

    APP->>PER: MX_IWDG_Init()
    Note right of APP: prescaler 256, reload 4095 → ~26.2s
    alt != HAL_OK
        HAL-->>APP: Error_Handler()
        Note right of APP: mx_init.c:238
    end
```

## Traceability

| Message | Function | File:Line |
|---|---|---|
| SystemClock_Config | `SystemClock_Config` | `main/Src/mx_init.c:113` |
| MX_I2C1_Init | `MX_I2C1_Init` | `main/Src/mx_init.c:202` |
| HAL_I2C_MspInit (EXTI/NVIC config) | `HAL_I2C_MspInit` | `main/Src/stm32f0xx_hal_msp.c:66-97` |
| MX_RTC_Init | `MX_RTC_Init` | `main/Src/mx_init.c` (declared `:70`) |
| KM_RTC_Restore | `KM_RTC_Restore` | `main/Src/mx_init.c:310` |
| HAL_RTC_MspInit | `HAL_RTC_MspInit` | `main/Src/stm32f0xx_hal_msp.c:126-144` |
| MX_TIM3_Init | `MX_TIM3_Init` | `main/Src/mx_init.c:185` (declared `:64`) |
| MX_IWDG_Init | `MX_IWDG_Init` | `main/Src/mx_init.c:229` |
