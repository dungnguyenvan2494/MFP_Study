# Sequence Diagram — Communication TX (phản hồi đọc thanh ghi qua I2C)

Các participant: **External Device** (I2C Master), **Application**,
**Driver** (`Driver_I2C0`), **HAL**, **ISR**, **Peripheral** (I2C1).

```mermaid
sequenceDiagram
    participant EXT as External Device (I2C Master)
    participant APP as Application
    participant DRV as Driver (Driver_I2C0)
    participant HAL as HAL
    participant ISR as ISR
    participant PER as Peripheral (I2C1)

    Note over APP: i2c_recv_wait() has decoded a 1-byte<br/>(read-request) write — see 05_communication_rx.md
    APP->>APP: switch(second_addr)
    alt RTC_*/B_REGISTER_* command
        APP->>APP: stage pointer = RTC_BASE + addr
        Note right of APP: km_i2c.c:553 — live register, not a copy
    else EXTEND_OUTPUT/INPUT/*STS* command
        APP->>APP: tx_data = io_extend_read(addr)
        Note right of APP: km_i2c.c:561,570
    else EXTEND_CHECKSUM_COMMAND
        APP->>APP: tx_data = g_checksum_value
    else EXTEND_VERSION_COMMAND
        APP->>APP: buf = &g_main_version[0]
    else EXTEND_EVENT_COMMAND
        APP->>APP: buf = &st_EventRecord[0], size=sizeof(st_EventRecord)
        Note right of APP: km_i2c.c:612 — entire 250-entry ring buffer in one transfer
    else unrecognized address
        APP->>APP: tx_data = 0xffff (sentinel)
    end

    APP->>DRV: change_state(Wait_Read, ..., SlaveTransmit, buf, size, NULL, 0)
    Note right of APP: km_i2c.c:274
    DRV->>PER: HAL_NVIC_DisableIRQ(I2C1_IRQn)
    DRV->>DRV: (*transfer)(buf,size) = Driver_I2C0.SlaveTransmit(...)
    Note right of DRV: indirect call through ARM_DRIVER_I2C struct, 04_call_graph.md §3.4
    DRV->>PER: HAL_NVIC_EnableIRQ(I2C1_IRQn)
    APP->>APP: i2c_info.state = Wait_Read
    APP-->>EXT: (function returns — master now clocks bytes out)

    EXT->>PER: I2C1 clocks bytes (TXIS flag per byte)
    PER->>ISR: I2C1 IRQ
    ISR->>HAL: I2C1_IRQHandler() → HAL_I2C_EV_IRQHandler()
    HAL->>HAL: I2C_Slave_ISR_IT()
    Note right of HAL: stm32f0xx_hal_i2c.c:3457
    HAL->>PER: TXDR = (*hi2c->pBuffPtr++)
    HAL->>DRV: HAL_I2C_SlaveTxCpltCallback → do_callback(TRANSFER_DONE)
    Note right of HAL: I2C_stm32f0xx.c:736
    DRV->>APP: rx_comp(event)
    APP->>APP: i2c_info.read_comp = 1
    Note right of APP: km_i2c.c:221 [ISR]
    ISR-->>PER: return from interrupt

    Note over APP: [wait] next super-loop pass
    APP->>APP: i2c_recv_wait(), state==Wait_Read case
    APP->>DRV: change_state(Wait_Write, ..., SlaveReceive, ...)
    Note right of APP: km_i2c.c:745-751 — ready for next command,<br/>→ 05_communication_rx.md
```

## Traceability

| Message | Function | File:Line |
|---|---|---|
| change_state (arm transmit) | `change_state` | `main/App/km_i2c.c:274` |
| Driver_I2C0.SlaveTransmit | `I2C0_SlaveTransmit` (via `ARM_DRIVER_I2C`) | `I2C_stm32f0xx.c:777-790` |
| I2C_Slave_ISR_IT (TXDR write) | `I2C_Slave_ISR_IT` | `stm32f0xx_hal_i2c.c:3457` |
| HAL_I2C_SlaveTxCpltCallback | `HAL_I2C_SlaveTxCpltCallback` | `I2C_stm32f0xx.c:736` |
| rx_comp | `rx_comp` | `main/App/km_i2c.c:171` |
| i2c_recv_wait (re-arm) | `i2c_recv_wait` | `main/App/km_i2c.c:745-751` |
