# Sequence Diagram — Timer Event (tick 1ms của TIM3, image Main)

Các participant: **Peripheral** (TIM3), **NVIC**, **ISR**, **Driver**
(BSP_TIM), **Application**.

```mermaid
sequenceDiagram
    participant PER as Peripheral (TIM3)
    participant NVIC as NVIC
    participant ISR as ISR
    participant BSP as Driver (BSP_TIM)
    participant APP as Application

    Note over PER: t=0, then every +1ms (repeating)<br/>09_timing.md §1 — TIM_PRESCALER=2000, TIM_PERIOD=2
    PER->>NVIC: TIM3 update event (UIF)
    NVIC->>ISR: vector to TIM3_IRQHandler()
    Note right of ISR: main/Src/stm32f0xx_it.c:189, priority (0,0)
    ISR->>ISR: HAL_TIM_IRQHandler(&htim3)
    ISR->>BSP: HAL_TIM_PeriodElapsedCallback(htim)
    Note right of BSP: BSP_TIM_STM32F03x_Nucleo.c:72 [weak override,<br/>linear-scans Resource[] for handle match]
    BSP->>APP: Resource[type].SignalHandler(TIMER_3, PERIOD_ELAPSED)
    Note right of BSP: ⇢ function pointer, registered by<br/>BSP_TIM_Initialize(TIMER_3, km_tim_callback) at boot
    APP->>APP: km_tim_callback(TIMER_3, event)
    Note right of APP: km_it.c:312 [ISR context]
    APP->>NVIC: HAL_NVIC_DisableIRQ(TIM3_IRQn)
    Note right of APP: km_it.c:333 — self-bracket, functionally inert<br/>(same-priority IRQs can't preempt anyway, 08_concurrency.md §3 item 3)
    APP->>APP: ui_EventCountTimer++
    loop for i in 0..TYPE_Km_Timer_Kind_Max-1 (16 kinds)
        alt g_km_timer[i].state == Start
            APP->>APP: g_km_timer[i].ms_time -= 1
            opt ms_time == 0
                APP->>APP: g_km_timer[i].state = End
            end
        end
    end
    APP->>NVIC: HAL_NVIC_EnableIRQ(TIM3_IRQn)
    ISR-->>NVIC: return from interrupt

    Note over APP: [wait] main-loop consumers poll km_timer_get_state()<br/>on their next super-loop pass — [TIMING UNKNOWN, depends on<br/>which consumer and where it sits in loop order]
    par debounce consumer
        APP->>APP: anti_chattering_proc()
    and power-off sequencing consumer
        APP->>APP: poweroff_timer_proc()
    and backup-wait consumer
        APP->>APP: backupwait_timer_proc()
    end
```

## Traceability

| Message | Function | File:Line |
|---|---|---|
| TIM3_IRQHandler | `TIM3_IRQHandler` | `main/Src/stm32f0xx_it.c:189` |
| HAL_TIM_PeriodElapsedCallback | `HAL_TIM_PeriodElapsedCallback` | `BSP_TIM_STM32F03x_Nucleo.c:72` |
| km_tim_callback | `km_tim_callback` | `main/App/km_it.c:312` |
| km_timer_get_state (consumers) | `km_timer_get_state` | `main/App/km_it.c:417` |
| poweroff_timer_proc | `poweroff_timer_proc` | `main/App/km_extend_io.c:1305` |
| backupwait_timer_proc | `backupwait_timer_proc` | `main/App/km_extend_io.c:1350` |

**Ghi chú về image IAP:** TIM3 được bật clock và được nạp (arm) vào NVIC
giống hệt như vậy trong image IAP, nhưng `HAL_TIM_Base_Start_IT`/
`BSP_TIM_Start` không bao giờ được gọi ở đó — sequence này không bao giờ
thực sự xảy ra trong IAP
(`03_execution_model.md` §4.3).
