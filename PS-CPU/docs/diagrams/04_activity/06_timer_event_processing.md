# Activity Diagram — Timer Event Processing (tick TIM3 1ms, Main image)

Swimlanes: **Hardware**, **ISR**, **Application**.

```mermaid
flowchart TD
    START(["START — TIM3 counter reaches Period (every 1ms)"])

    subgraph HW["Hardware"]
        UPD["TIM3 update event (UIF flag)"]
    end

    subgraph ISR["ISR — TIM3_IRQn, priority (0,0)"]
        VEC["TIM3_IRQHandler()<br/>main/Src/stm32f0xx_it.c:189"]
        HALTIM["HAL_TIM_IRQHandler(&htim3)"]
        WEAKCB["HAL_TIM_PeriodElapsedCallback(htim)<br/>BSP_TIM_STM32F03x_Nucleo.c:72<br/>[weak override, scans Resource[] for handle match]"]
        SIGEVT["Resource[type].SignalHandler(type, PERIOD_ELAPSED)<br/>⇢ function pointer, registered by BSP_TIM_Initialize"]
        KMTIM["km_tim_callback(TIMER_3, event)<br/>km_it.c:312"]
        SELFDIS["HAL_NVIC_DisableIRQ(TIM3_IRQn)<br/>km_it.c:333<br/>[self-bracket — functionally inert,\nsee 08_concurrency.md §3 item 3]"]
        INCCOUNT["ui_EventCountTimer++<br/>km_it.c:337"]
        LOOPTIMERS["for each g_km_timer[i]:<br/>if state==Start: ms_time -= 1<br/>km_it.c:340-348"]
        CHECKZERO{"ms_time == 0?"}
        MARKEND["g_km_timer[i].state = End"]
        SELFEN["HAL_NVIC_EnableIRQ(TIM3_IRQn)<br/>km_it.c:350"]
        ISREND(["ISR returns"])
    end

    subgraph APPL["Application — main-loop, next pass(es)"]
        WAITNODE["[wait] super-loop polls\nkm_timer_get_state(kind)"]
        CONSUMER{"which timer kind\nreached End?"}
        DEBOUNCE["anti_chattering_proc()<br/>→ see 05_interrupt_processing_exti.md"]
        PWROFF["poweroff_timer_proc()<br/>→ see 08_recovery diagram"]
        BACKUP["backupwait_timer_proc()<br/>km_extend_io.c:1350"]
        MCPON["mc_p_on_eagle_proc() /\nsleep_status_rem_sparrow_proc()<br/>(BOOT_MC_P_ON / MC_P_ON_AFTER_SLEEP_STATUS_REM_SP)"]
        CLEARSTATE["km_timer_set(kind, 0, Normal)<br/>[not all consumers do this —\nsee 07_state_machines.md §1.3 note]"]
        END(["END"])
    end

    START --> UPD --> VEC --> HALTIM --> WEAKCB --> SIGEVT --> KMTIM
    KMTIM --> SELFDIS --> INCCOUNT --> LOOPTIMERS --> CHECKZERO
    CHECKZERO -- yes --> MARKEND --> SELFEN
    CHECKZERO -- no --> SELFEN
    SELFEN --> ISREND --> WAITNODE --> CONSUMER
    CONSUMER -- "debounce timers" --> DEBOUNCE --> CLEARSTATE --> END
    CONSUMER -- "PWROFF_MAXTIME1/2, WDGTIME" --> PWROFF --> END
    CONSUMER -- "BACKUP_WAITTIME" --> BACKUP --> CLEARSTATE
    CONSUMER -- "BOOT_MC_P_ON, etc." --> MCPON --> CLEARSTATE
    CLEARSTATE --> END
```

## Traceability

| Node | Function | File:Line |
|---|---|---|
| TIM3_IRQHandler | `TIM3_IRQHandler` | `main/Src/stm32f0xx_it.c:189` |
| HAL_TIM_PeriodElapsedCallback | `HAL_TIM_PeriodElapsedCallback` | `common/Drivers/BSP/Src/BSP_TIM_STM32F03x_Nucleo.c:72` |
| km_tim_callback | `km_tim_callback` | `main/App/km_it.c:312` |
| km_timer_get_state | `km_timer_get_state` | `main/App/km_it.c:417` |
| km_timer_set | `km_timer_set` | `main/App/km_it.c:384` |
| poweroff_timer_proc | `poweroff_timer_proc` | `main/App/km_extend_io.c:1305` |
| backupwait_timer_proc | `backupwait_timer_proc` | `main/App/km_extend_io.c:1350` |
