# Activity Diagram — Interrupt Processing (sự kiện tín hiệu GPIO/EXTI)

Swimlanes: **Hardware**, **ISR**, **Application**. Bao phủ cả 5 ngõ vào
vật lý có debounce/được trì hoãn xử lý (`03_execution_model.md` §4.2).

```mermaid
flowchart TD
    START(["START — physical pin edge"])

    subgraph HW["Hardware"]
        EDGE["GPIOA pin edge:<br/>PA1/_HRESET_REQ, PA2/AP_PWR_EN,<br/>PA4/POWER_MONITOR, PA5/MSW_ON, PA8/MONI_24V11"]
    end

    subgraph ISR["ISR — EXTI0_1/EXTI2_3/EXTI4_15, priority (0,0)"]
        VEC["EXTIx_y_IRQHandler()<br/>main/Src/stm32f0xx_it.c:145-184"]
        HALEXTI["HAL_GPIO_EXTI_IRQHandler(pin)<br/>(HAL clears EXTI pending bit)"]
        WEAKCB["HAL_GPIO_EXTI_Callback(pin)<br/>BSP_GPIO_STM32F03x_Nucleo.c:70<br/>[weak override]"]
        SIGEVT["signal_event(pin, EXT_INTR)<br/>⇢ function pointer, registered by\nBSP_GPIO_Initialize(km_exti_callback)"]
        KMEXTI["km_exti_callback(pin, event)<br/>km_it.c:247"]
        WHICHPIN{"which pin?"}
        DEFERBIT["set_pending_factor_bit(...)<br/>km_it.c:254,257<br/>[DEFERRED to main-loop]"]
        ARMDEBOUNCE["start_anti_chattering(idx, level)<br/>km_it.c:261,265,274<br/>[ARMED NOW, in ISR]"]
        MSWCHECK{"MSW_ON AND\nstop_mode == ON?"}
        PLLRELOCK["SystemClock_Config2()<br/>+ HAL_RTC_DeactivateAlarm()<br/>km_it.c:270-271<br/>[PLL relock inside ISR]"]
        ISREND(["ISR returns"])
    end

    subgraph APPL["Application — main-loop"]
        WAITNODE["[wait] next super-loop pass(es)"]
        BRANCH{"which mechanism<br/>armed?"}
        INTRPEND["intr_pending_proc()<br/>km_extend_io.c:1416<br/>polls pending_factor bits"]
        DISPATCH1["hreset_req_proc() /\nap_power_en_proc(STATE_INT)"]
        ANTICHAT["anti_chattering_proc()<br/>km_extend_io.c:1438<br/>polls debounce timer"]
        TIMEREXP{"decision timer\nstate == End AND\nlevel == start_level?"}
        FIREPROC["⇢ info->intr_proc(STATE_INT)<br/>km_extend_io.c:1460<br/>indirect call → msw_on_proc/\npower_monitor_proc/(none for 24V11)"]
        NOOP["no callback fired<br/>(24V11 entry has intr_proc==NULL)"]
        END(["END"])
    end

    START --> EDGE --> VEC --> HALEXTI --> WEAKCB --> SIGEVT --> KMEXTI --> WHICHPIN
    WHICHPIN -- "HRESET_REQ / AP_PWR_EN" --> DEFERBIT --> ISREND
    WHICHPIN -- "POWER_MONITOR / MSW_ON / MONI_24V11" --> ARMDEBOUNCE --> MSWCHECK
    MSWCHECK -- yes --> PLLRELOCK --> ISREND
    MSWCHECK -- no --> ISREND
    ISREND --> WAITNODE --> BRANCH
    BRANCH -- "pending_factor path" --> INTRPEND --> DISPATCH1 --> END
    BRANCH -- "debounce path" --> ANTICHAT --> TIMEREXP
    TIMEREXP -- yes --> FIREPROC --> END
    TIMEREXP -- "no / NULL callback" --> NOOP --> END
```

## Traceability

| Node | Function | File:Line |
|---|---|---|
| EXTIx_y_IRQHandler | `EXTI0_1_IRQHandler`/`EXTI2_3_IRQHandler`/`EXTI4_15_IRQHandler` | `main/Src/stm32f0xx_it.c:145-184` |
| HAL_GPIO_EXTI_Callback | `HAL_GPIO_EXTI_Callback` | `common/Drivers/BSP/Src/BSP_GPIO_STM32F03x_Nucleo.c:70` |
| km_exti_callback | `km_exti_callback` | `main/App/km_it.c:247` |
| set_pending_factor_bit | `set_pending_factor_bit` | `main/App/km_it.c:186` (calls at `:254,257`) |
| start_anti_chattering | `start_anti_chattering` | `main/App/km_it.c:489` |
| intr_pending_proc | `intr_pending_proc` | `main/App/km_extend_io.c:1416` |
| anti_chattering_proc | `anti_chattering_proc` | `main/App/km_extend_io.c:1438` |
