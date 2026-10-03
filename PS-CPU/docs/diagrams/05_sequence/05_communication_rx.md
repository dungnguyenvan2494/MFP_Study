# Sequence Diagram — Communication RX (ghi thanh ghi qua I2C)

Các participant: **External Device** (I2C Master), **Peripheral** (I2C1),
**NVIC**, **ISR**, **HAL**, **Driver** (CMSIS `Driver_I2C0`), **Application**.

```mermaid
sequenceDiagram
    participant EXT as External Device (I2C Master)
    participant PER as Peripheral (I2C1)
    participant NVIC as NVIC
    participant ISR as ISR
    participant HAL as HAL
    participant DRV as Driver (Driver_I2C0)
    participant APP as Application

    EXT->>PER: START + address + data bytes
    PER->>NVIC: I2C1 IRQ (ADDR/RXNE flags)
    NVIC->>ISR: vector to I2C1_IRQHandler()
    Note right of ISR: main/Src/stm32f0xx_it.c:203
    ISR->>HAL: HAL_I2C_EV_IRQHandler(&hi2c1)
    HAL->>HAL: I2C_Slave_ISR_IT()
    Note right of HAL: stm32f0xx_hal_i2c.c:3380
    HAL->>PER: read RXDR → (*hi2c->pBuffPtr++)
    Note right of HAL: stm32f0xx_hal_i2c.c:3432 — byte copy into i2c_info.rx_buf[]

    alt TRANSFER_RELOAD event (1 byte received, manual-ACK cycle)
        HAL->>DRV: KM_HAL_I2C_ReloadCpltCallback → do_callback(RELOAD)
        DRV->>APP: rx_comp(event)
        Note right of APP: km_i2c.c:171-255 [ISR context]
        APP->>APP: valid_second_addr(rx_buf_current)
        alt address in range
            APP->>DRV: Driver_I2C0.Control(GENERATE_ACK)
        else out of range
            APP->>DRV: Driver_I2C0.Control(GENERATE_NACK)
            Note right of DRV: I2C_stm32f0xx.c:597-600 — falls through<br/>into ACK's CR2 write too, see 05_data_flow.md §5 item 5
        end
    else ADDR event
        HAL->>DRV: HAL_I2C_AddrCallback → do_callback(ADDRESS_MATCH)
        DRV->>APP: rx_comp(event)
    else STOPF/transfer-complete event
        HAL->>DRV: HAL_I2C_SlaveRxCpltCallback → do_callback(TRANSFER_DONE)
        DRV->>APP: rx_comp(event)
        APP->>APP: i2c_info.write_comp = 1
        Note right of APP: km_i2c.c:226 [ISR — sets a plain flag,<br/>NOT a queue/semaphore, 03_execution_model.md §8]
    end
    ISR-->>NVIC: return from interrupt

    Note over APP: [wait] next super-loop pass, [TIMING UNKNOWN — depends on loop position]
    APP->>APP: i2c_recv_wait()
    Note right of APP: km_i2c.c:485
    APP->>APP: check i2c_info.write_comp
    alt write_comp set, state==Wait_Write
        APP->>APP: decode second_addr, mutate target<br/>(RTC / g_io_extend_memory / timers)
        APP->>DRV: change_state(Wait_Write, ..., SlaveReceive, ...)
        Note right of APP: km_i2c.c:274 — re-arm for next command
        DRV->>PER: arm next reception
    else protocol mismatch (read_comp arrived instead)
        APP->>APP: i2c_sw_reset()
        Note right of APP: → 10_error_handling.md
    end
```

## Traceability

| Message | Function | File:Line |
|---|---|---|
| I2C1_IRQHandler | `I2C1_IRQHandler` | `main/Src/stm32f0xx_it.c:203` |
| I2C_Slave_ISR_IT | `I2C_Slave_ISR_IT` | `stm32f0xx_hal_i2c.c:3380,3432` |
| rx_comp | `rx_comp` | `main/App/km_i2c.c:171` |
| valid_second_addr | `valid_second_addr` | `main/App/km_i2c.c:93` |
| I2Cx_Control | `I2Cx_Control` | `I2C_stm32f0xx.c:552-608` |
| i2c_recv_wait | `i2c_recv_wait` | `main/App/km_i2c.c:485` |
| change_state | `change_state` | `main/App/km_i2c.c:274` |
