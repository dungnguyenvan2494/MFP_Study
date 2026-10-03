# Activity Diagram — Error Handling (lỗi giao tiếp I2C)

Swimlanes: **Hardware**, **ISR**, **Application**. Tương ứng với
`10_error_recovery.md` §2.1.

```mermaid
flowchart TD
    START(["START — i2c_recv_wait() called this super-loop pass"])

    subgraph ISRlane["ISR context (already occurred, latched state)"]
        ERRLATCH["rx_comp() latched error_code\n(BERR/ARLO/OVR) or dir_error<br/>km_i2c.c:251-253,234,242<br/>[may or may not have happened]"]
    end

    subgraph APPL["Application — km_i2c.c:379-461"]
        ENTRY["i2c_check_error()<br/>km_i2c.c:379"]
        NVICDIS["HAL_NVIC_DisableIRQ(I2C1_IRQn)<br/>km_i2c.c:392<br/>[critical section: latch a snapshot]"]
        SNAPSHOT["snapshot: nowtick, rx/tx_tick_last,\nrx/tx_during, error_code, dir_error,\nDriver_I2C0.GetStatus()"]
        NVICEN["HAL_NVIC_EnableIRQ(I2C1_IRQn)<br/>km_i2c.c:401"]
        CHECKBUSY{"busy_tx held\n> 2000ms?"}
        CHECKRX{"rx_during AND\nheld > 1000ms?"}
        CHECKTX{"tx_during AND\nheld > 1000ms?"}
        CHECKERR{"error_code != 0?"}
        CHECKDIR{"dir_error?"}
        ANYERROR{"any check\ntrue?"}
        NOERROR["no error — proceed to\nnormal dispatch, 03/04 diagrams"]
        RESETCALL["i2c_sw_reset()<br/>km_i2c.c:356"]
        FORCERST["__HAL_RCC_I2C1_FORCE_RESET()"]
        DELAY["HAL_Delay(2ms)<br/>[BLOCKING]"]
        RELEASERST["__HAL_RCC_I2C1_RELEASE_RESET()"]
        UNINIT["Driver_I2C0.Uninitialize()"]
        REINIT["MX_I2C1_Init()<br/>mx_init.c:202"]
        REARM["i2c_recv_first()<br/>km_i2c.c:317<br/>state = Wait_Write"]
        END(["END — I2C1 fully re-armed,\nno retry counter, no cause logged"])
    end

    START --> ENTRY --> NVICDIS --> SNAPSHOT --> NVICEN
    NVICEN --> CHECKBUSY
    CHECKBUSY -- yes --> ANYERROR
    CHECKBUSY -- no --> CHECKRX
    CHECKRX -- yes --> ANYERROR
    CHECKRX -- no --> CHECKTX
    CHECKTX -- yes --> ANYERROR
    CHECKTX -- no --> CHECKERR
    CHECKERR -- yes --> ANYERROR
    CHECKERR -- no --> CHECKDIR
    CHECKDIR -- yes --> ANYERROR
    CHECKDIR -- no --> ANYERROR
    ANYERROR -- yes --> RESETCALL
    ANYERROR -- no --> NOERROR --> END
    RESETCALL --> FORCERST --> DELAY --> RELEASERST --> UNINIT --> REINIT --> REARM --> END
    ERRLATCH -.contributes to.-> SNAPSHOT
```

## Alternative trigger — reset do master yêu cầu (không có lỗi nào cả)

I2C master có thể đi thẳng tới cùng node `i2c_sw_reset()` này, mà không
cần đi qua bất kỳ bước kiểm tra timeout/lỗi nào, bằng cách ghi
`EXTEND_RESET_I2C` (`km_i2c.c:720-723`) — được thể hiện như một điểm vào
riêng biệt tại node `WRITEPATH` trong `03_communication_rx.md`.

## Traceability

| Node | Function | File:Line |
|---|---|---|
| i2c_check_error | `i2c_check_error` | `main/App/km_i2c.c:379` |
| i2c_sw_reset | `i2c_sw_reset` | `main/App/km_i2c.c:356` |
| i2c_recv_first | `i2c_recv_first` | `main/App/km_i2c.c:317` |
| MX_I2C1_Init | `MX_I2C1_Init` | `main/Src/mx_init.c:202` |
