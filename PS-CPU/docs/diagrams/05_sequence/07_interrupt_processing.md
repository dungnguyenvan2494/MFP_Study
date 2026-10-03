# Sequence Diagram — Interrupt Processing (GPIO/EXTI)

Các participant: **External Device / Peripheral** (chân vật lý), **NVIC**,
**ISR**, **Driver** (lớp BSP GPIO), **Application**.

```mermaid
sequenceDiagram
    participant PER as Peripheral (GPIOA pin)
    participant NVIC as NVIC
    participant ISR as ISR
    participant BSP as Driver (BSP_GPIO)
    participant APP as Application

    PER->>NVIC: edge on PA1/PA2/PA4/PA5/PA8<br/>(HRESET_REQ/AP_PWR_EN/POWER_MONITOR/MSW_ON/MONI_24V11)
    NVIC->>ISR: vector to EXTIx_y_IRQHandler()
    Note right of ISR: main/Src/stm32f0xx_it.c:145-184<br/>priority (0,0) — same as every other IRQ used here
    ISR->>ISR: HAL_GPIO_EXTI_IRQHandler(pin)
    Note right of ISR: HAL clears the EXTI pending bit
    ISR->>BSP: HAL_GPIO_EXTI_Callback(pin)
    Note right of BSP: BSP_GPIO_STM32F03x_Nucleo.c:70 [weak override]
    BSP->>APP: signal_event(pin, EXT_INTR)
    Note right of BSP: ⇢ function pointer, registered by<br/>BSP_GPIO_Initialize(km_exti_callback) at boot
    APP->>APP: km_exti_callback(pin, event)
    Note right of APP: km_it.c:247 [still ISR context]

    alt pin == HRESET_REQ or AP_PWR_EN
        APP->>APP: set_pending_factor_bit(...)
        Note right of APP: km_it.c:254,257 — DEFERRED, protected by<br/>the reference-counted km_disable_irq/enable_irq wrapper
    else pin == POWER_MONITOR, MSW_ON, or MONI_24V11
        APP->>APP: start_anti_chattering(idx, level)
        Note right of APP: km_it.c:489-501 — ARMED NOW, in ISR
        opt pin == MSW_ON and stop_mode == ON
            APP->>PER: SystemClock_Config2()
            APP->>PER: HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A)
            Note right of APP: km_it.c:270-271 — PLL relock inside this ISR
        end
    end
    ISR-->>NVIC: return from interrupt

    Note over APP: [wait] next super-loop pass(es), [TIMING UNKNOWN]
    par pending_factor path
        APP->>APP: intr_pending_proc()
        APP->>APP: hreset_req_proc() / ap_power_en_proc(STATE_INT)
        Note right of APP: km_extend_io.c:1416-1426
    and debounce path
        APP->>APP: anti_chattering_proc()
        opt debounce timer state == End and level == start_level
            APP->>APP: info->intr_proc(STATE_INT)
            Note right of APP: km_extend_io.c:1460 — indirect call →<br/>msw_on_proc / power_monitor_proc / (none, 24V11 has intr_proc==NULL)
        end
    end
```

## Traceability

| Message | Function | File:Line |
|---|---|---|
| EXTIx_y_IRQHandler | `EXTI0_1_IRQHandler`/`EXTI2_3_IRQHandler`/`EXTI4_15_IRQHandler` | `main/Src/stm32f0xx_it.c:145-184` |
| HAL_GPIO_EXTI_Callback | `HAL_GPIO_EXTI_Callback` | `BSP_GPIO_STM32F03x_Nucleo.c:70` |
| km_exti_callback | `km_exti_callback` | `main/App/km_it.c:247` |
| set_pending_factor_bit | `set_pending_factor_bit` | `main/App/km_it.c:186` |
| start_anti_chattering | `start_anti_chattering` | `main/App/km_it.c:489` |
| intr_pending_proc | `intr_pending_proc` | `main/App/km_extend_io.c:1416` |
| anti_chattering_proc | `anti_chattering_proc` | `main/App/km_extend_io.c:1438` |
