# Sequence Diagram — Timeout / Retry

## Trình bày trung thực, theo bằng chứng từ source

Firmware này **không có bất kỳ pattern retry nào** — đã được xác nhận độc
lập trong `08_concurrency.md` §4, `09_timing.md` §4, và
`10_error_recovery.md` §3 (bảng tổng hợp). Mọi timeout trong hệ thống đều
đi thẳng đến một hành động khôi phục one-shot (chỉ thực hiện một lần) ngay
khi phát hiện lần đầu: reset ngoại vi I2C, hoặc reset toàn bộ MCU. Sơ đồ
này thể hiện hai chuỗi timeout tiêu biểu thay vì bịa ra một vòng lặp retry
không hề tồn tại.

```mermaid
sequenceDiagram
    participant APP as Application
    participant PER as Peripheral (I2C1 / RTC backup / hardware IWDG)

    Note over APP: Chain 1 — I2C communication timeout (09_timing.md §4)
    Note over APP: t=0 transaction starts (rx_tick_last/tx_tick_last stamped in rx_comp, ISR)
    Note over APP: t=+1000ms (rx/tx) or t=+2000ms (busy_tx) — i2c_check_error() detects it
    APP->>APP: i2c_sw_reset()
    Note right of APP: → 10_error_handling.md — ONE-SHOT, no retry counter

    Note over APP: Chain 2 — power-off sequencing timeout (09_timing.md §3)
    Note over APP: t=0 poweroff_timer_start() arms MAXTIME1=60000ms
    Note over APP: t=+60000ms MAXTIME1 expires
    APP->>APP: km_timer_set(MAXTIME2, 60000, Start)
    Note right of APP: km_extend_io.c:1316 — NOT a retry of a failed operation;<br/>a second, independent 60s window, always the same length
    Note over APP: t=+120000ms (total) MAXTIME2 expires
    APP->>PER: poweroff_factor_save(TMOUT_LIMIT)
    APP->>APP: s800_power_off()
    Note right of APP: → 13_shutdown.md — TERMINAL, no further retry
```

## Traceability

| Message | Function | File:Line |
|---|---|---|
| i2c_check_error / i2c_sw_reset | `i2c_check_error`, `i2c_sw_reset` | `main/App/km_i2c.c:379,356` |
| poweroff_timer_start | `poweroff_timer_start` | `main/App/km_extend_io.c:1279` |
| poweroff_timer_proc (MAXTIME2 re-arm) | `poweroff_timer_proc` | `main/App/km_extend_io.c:1305,1316` |
| s800_power_off | `s800_power_off` | `main/App/km_extend_io.c:523` |
