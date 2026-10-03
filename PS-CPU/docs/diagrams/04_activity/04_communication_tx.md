# Activity Diagram — Communication TX (phản hồi đọc thanh ghi qua I2C)

Swimlanes: **Hardware**, **ISR**, **Application**. Là phần tiếp theo của
nhánh `XFERSIZE == 1` trong `03_communication_rx.md`.

```mermaid
flowchart TD
    START(["START — i2c_recv_wait() has decoded\na 1-byte (read-request) write"])

    subgraph APPL["Application — km_i2c.c:511-627"]
        SWADDR{"switch(second_addr)"}
        RTCPATH["RTC_*/B_REGISTER_* case<br/>stage pointer = RTC_BASE+addr<br/>km_i2c.c:553"]
        EXTPATH["EXTEND_OUTPUT/INPUT/*STS* case<br/>tx_data = io_extend_read(addr)<br/>km_i2c.c:561,570"]
        CKSPATH["EXTEND_CHECKSUM_COMMAND case<br/>tx_data = g_checksum_value<br/>km_i2c.c:574"]
        VERPATH["EXTEND_VERSION_COMMAND case<br/>buf = &g_main_version[0]<br/>km_i2c.c:580"]
        EVTPATH["EXTEND_EVENT_COMMAND case<br/>buf = &st_EventRecord[0]<br/>km_i2c.c:612"]
        DEFPATH["default case<br/>tx_data = 0xffff (sentinel)<br/>km_i2c.c:623"]
        CHANGEST["change_state(Wait_Read, ...,\nDriver_I2C0.SlaveTransmit, buf, size, NULL, 0)<br/>km_i2c.c:274"]
        NVICDIS["HAL_NVIC_DisableIRQ(I2C1_IRQn)<br/>km_i2c.c:282"]
        SLAVETX["(*transfer)(buf,size) =\nDriver_I2C0.SlaveTransmit(...)<br/>⇢ indirect call, arms HAL"]
        NVICEN["HAL_NVIC_EnableIRQ(I2C1_IRQn)<br/>km_i2c.c:302"]
        STATENOW["i2c_info.state = Wait_Read"]
        RETURNAPP(["i2c_recv_wait() returns —\nmaster now clocks bytes out"])
    end

    subgraph HW["Hardware"]
        TXIS["I2C1: master clocks bytes,\nTXIS flag sets per byte"]
    end

    subgraph ISR2["ISR — I2C1_IRQn"]
        VEC2["I2C1_IRQHandler()<br/>stm32f0xx_it.c:203"]
        SLAVEISRTX["I2C_Slave_ISR_IT()<br/>stm32f0xx_hal_i2c.c:3457<br/>TXDR = (*hi2c->pBuffPtr++)"]
        TXCPLT["HAL_I2C_SlaveTxCpltCallback()<br/>I2C_stm32f0xx.c:736"]
        DOCB2["do_callback(TRANSFER_DONE)<br/>I2C_stm32f0xx.c:692"]
        RXCOMP2["rx_comp(event)<br/>km_i2c.c:171"]
        SETFLAG2["i2c_info.read_comp = 1<br/>km_i2c.c:221"]
        ISREND2(["ISR returns"])
    end

    WAITNODE2["[wait] next super-loop pass"]
    REARMREAD["i2c_recv_wait(), state==Wait_Read case<br/>km_i2c.c:745-751<br/>change_state(Wait_Write, ..., SlaveReceive, ...)"]
    ENDF(["END — ready for next command,\nsee 03_communication_rx.md"])

    START --> SWADDR
    SWADDR --> RTCPATH --> CHANGEST
    SWADDR --> EXTPATH --> CHANGEST
    SWADDR --> CKSPATH --> CHANGEST
    SWADDR --> VERPATH --> CHANGEST
    SWADDR --> EVTPATH --> CHANGEST
    SWADDR --> DEFPATH --> CHANGEST
    CHANGEST --> NVICDIS --> SLAVETX --> NVICEN --> STATENOW --> RETURNAPP
    RETURNAPP --> TXIS --> VEC2 --> SLAVEISRTX
    SLAVEISRTX --> TXCPLT --> DOCB2 --> RXCOMP2 --> SETFLAG2 --> ISREND2
    ISREND2 --> WAITNODE2 --> REARMREAD --> ENDF
```

## Traceability

| Node | Function | File:Line |
|---|---|---|
| i2c_recv_wait (read dispatch) | `i2c_recv_wait` | `main/App/km_i2c.c:511-627` |
| change_state | `change_state` | `main/App/km_i2c.c:274` |
| Driver_I2C0.SlaveTransmit (indirect) | `I2C0_SlaveTransmit` via `ARM_DRIVER_I2C` struct | `common/Drivers/CMSIS/.../I2C_stm32f0xx.c:777-790` (`04_call_graph.md` §3.4) |
| I2C1_IRQHandler | `I2C1_IRQHandler` | `main/Src/stm32f0xx_it.c:203` |
| I2C_Slave_ISR_IT (TXDR write) | `I2C_Slave_ISR_IT` | `stm32f0xx_hal_i2c.c:3457` |
| HAL_I2C_SlaveTxCpltCallback | `HAL_I2C_SlaveTxCpltCallback` | `I2C_stm32f0xx.c:736` |
| rx_comp | `rx_comp` | `main/App/km_i2c.c:171` |
