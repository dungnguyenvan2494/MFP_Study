# Sequence Diagram — Boot (image Main)

Các participant được dùng: **Hardware**, **Startup**, **Application**. Các
participant `RTOS`/`Task` bị lược bỏ — xem `03_rtos_startup.md` để biết ghi
chú xác nhận-vắng-mặt rõ ràng. Timing chỉ được thể hiện khi có căn cứ từ
source (`09_timing.md`); mọi thứ khác đều là `[TIMING UNKNOWN]`.

```mermaid
sequenceDiagram
    participant HW as Hardware
    participant SU as Startup
    participant APP as Application (main())
    participant WDG as IWDG (Hardware)

    Note over HW: t=0 — POR / NRST / IWDG timeout / SW reset
    HW->>SU: vector to Reset_Handler()
    Note right of SU: startup_stm32f031x6.s:131
    SU->>SU: SystemInit()
    SU->>SU: __main (.data/.bss init)
    SU->>APP: call main()
    Note right of APP: entry.c:46

    APP->>HW: __HAL_RCC_PWR_CLK_ENABLE()
    APP->>APP: MX_Init()
    Note right of APP: mx_init.c:81 — see 02_hardware_initialization.md for detail
    alt any of 13 HAL init calls fails
        APP->>APP: Error_Handler()
        Note right of APP: mx_init.c:529 — while(1){}, WDG NOT YET ARMED<br/>[TERMINAL — no recovery, 10_error_recovery.md §2.2]
    else all HAL init calls succeed
        APP->>APP: get_model()
        Note right of APP: km_extend_io.c:1480 — reads+caches 2 GPIO straps
        APP->>APP: checksum_calc() / GetVersionString()
        Note right of APP: km_i2c.c:771,800
        APP->>WDG: BSP_WDT_Start()
        Note right of APP: BSP_WDT_STM32F03x_Nucleo.c:31<br/>[WATCHDOG NOW ARMED — ~26.2s, 09_timing.md §1]
        APP->>APP: internal_sts_init()
        APP->>APP: km_it_init()
        Note right of APP: km_it.c:364 — registers km_exti_callback,<br/>km_tim_callback (EXTI armed here)
        APP->>HW: BSP_TIM_Start(TIMER_3, 1)
        Note right of APP: entry.c:66 — TIM3 IRQ armed, 1ms period
        APP->>APP: adc_moni_5v_start_proc()
        Note right of APP: km_adc.c:27 — [DEAD, #if 0 body]
        APP->>HW: io_extend_memory_write()
        APP->>HW: assert /RESET
        APP->>APP: s800_power_on()
        Note right of APP: km_extend_io.c:607
        APP->>HW: i2c_recv_first()
        Note right of APP: km_i2c.c:317 — I2C1 slave RX armed
        APP->>APP: poweroff_timer_init()
        APP->>APP: enter while(1) super-loop
        Note right of APP: → 04_normal_application_flow.md
    end
```

## Traceability

| Message | Function | File:Line |
|---|---|---|
| vector to Reset_Handler | `Reset_Handler` | `main/MDK-ARM/startup_stm32f031x6.s:131` |
| call main() | `main` | `main/App/entry.c:46` |
| MX_Init() | `MX_Init` | `main/Src/mx_init.c:81` |
| Error_Handler() | `Error_Handler` | `main/Src/mx_init.c:529` |
| get_model() | `get_model` | `main/App/km_extend_io.c:1480` |
| BSP_WDT_Start() | `BSP_WDT_Start` | `common/Drivers/BSP/Src/BSP_WDT_STM32F03x_Nucleo.c:31` |
| km_it_init() | `km_it_init` | `main/App/km_it.c:364` |
| s800_power_on() | `s800_power_on` | `main/App/km_extend_io.c:607` |
| i2c_recv_first() | `i2c_recv_first` | `main/App/km_i2c.c:317` |
