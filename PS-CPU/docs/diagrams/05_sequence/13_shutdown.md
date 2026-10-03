# Sequence Diagram — Shutdown (chuỗi tắt nguồn S800)

Các participant: **External Device** (User qua MSW, hoặc S800),
**Application**, **Peripheral** (GPIO/NVIC/RTC), **Hardware** (reset cuối
cùng). Đây là sequence cuối cùng (terminal) mà mọi sơ đồ error/failsafe
trong bộ tài liệu này đều đổ dồn về.

```mermaid
sequenceDiagram
    participant EXT as External Device (User / S800)
    participant APP as Application
    participant PER as Peripheral (GPIO/NVIC/RTC)
    participant HW as Hardware

    EXT->>APP: MSW_ON released (or one of 3 other trigger paths,<br/>04_call_graph.md §2.9 + 06_event_flow.md E23 amendment)
    APP->>APP: s800_power_off()
    Note right of APP: main/App/km_extend_io.c:523
    APP->>APP: set_ca72_status(CA72_OFF)
    APP->>APP: g_first_power_on = START
    APP->>APP: poweroff_timer_init()
    APP->>APP: clear_pending_factor_bit(All)
    APP->>PER: NVIC_DisableIRQ ×3 (EXTI0_1/2_3/4_15)
    Note right of APP: km_extend_io.c:538-540 — RAW BYPASS of the<br/>reference-counted wrapper, 08_concurrency.md §3 item 6.<br/>No matching re-enable — function ends in a reset.
    APP->>PER: /RESET = LOW, SB_PWR_EN = LOW
    APP->>APP: g_io_extend_memory[Internal_Sts0] |= BOOT_STATES_REBOOT
    APP->>APP: init_ei_ref()
    APP->>APP: extend_output_init()
    APP->>APP: g_24v_check_flg_off = ACTIVE; g_internal_sts_boot_flg = 0
    APP->>APP: get_model()
    APP->>APP: sleep_status_rem_proc(model)
    Note right of APP: ⇒ jump table — drives SLEEP_STATUS_REM low
    APP->>APP: HAL_Delay(500ms)
    Note right of APP: km_extend_io.c:558 — [BLOCKING, entire<br/>super-loop stalled], 09_timing.md §6
    APP->>HW: HAL_NVIC_SystemReset()
    Note right of APP: km_extend_io.c:560 — TERMINAL
    HW-->>APP: → 01_boot_sequence.md<br/>(reset cause NOT distinguishable from POR/pin/IWDG,<br/>06_event_flow.md §2)
```

## Traceability

| Message | Function | File:Line |
|---|---|---|
| s800_power_off | `s800_power_off` | `main/App/km_extend_io.c:523` |
| set_ca72_status | `set_ca72_status` | `main/App/km_ca72_status.c:22` |
| clear_pending_factor_bit | `clear_pending_factor_bit` | `main/App/km_it.c:202` |
| sleep_status_rem_proc | `sleep_status_rem_proc` | `main/App/km_extend_io.c:1055` |
| HAL_NVIC_SystemReset call site | — | `main/App/km_extend_io.c:560` |
