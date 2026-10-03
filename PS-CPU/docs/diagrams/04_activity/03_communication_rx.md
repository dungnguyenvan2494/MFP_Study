# Activity Diagram — Communication RX (ghi thanh ghi qua I2C)

Swimlanes: **Hardware**, **ISR**, **Application**. Không tồn tại node
RTOS/queue/semaphore nào trong firmware này (`03_execution_model.md` §8) —
việc chuyển giao từ ISR sang main-loop là một cờ (flag) polling đơn giản,
được thể hiện rõ ràng như một node "wait" thay vì biểu tượng queue/semaphore.

```mermaid
flowchart TD
    START(["START — I2C master issues a WRITE transaction"])

    subgraph HW["Hardware"]
        BUS["I2C1: START + address + data bytes clocked in<br/>RXNE/ADDR flags set"]
    end

    subgraph ISR["ISR — I2C1_IRQn, priority (0,0)"]
        VEC["I2C1_IRQHandler()<br/>main/Src/stm32f0xx_it.c:203"]
        HALEV["HAL_I2C_EV_IRQHandler() / ER_IRQHandler()"]
        SLAVEISR["I2C_Slave_ISR_IT()<br/>stm32f0xx_hal_i2c.c:3380<br/>(*hi2c->pBuffPtr++) = RXDR  — line :3432"]
        ADDRCB["HAL_I2C_AddrCallback()<br/>I2C_stm32f0xx.c:767"]
        RXCPLT["HAL_I2C_SlaveRxCpltCallback()<br/>I2C_stm32f0xx.c:747"]
        DOCB["do_callback(event)<br/>I2C_stm32f0xx.c:692"]
        RXCOMP["rx_comp(event)<br/>km_i2c.c:171<br/>[ISR — Main image]"]
        DECRELOAD{"TRANSFER_RELOAD<br/>event?"}
        VALIDADDR["valid_second_addr()<br/>km_i2c.c:93<br/>range-check the address byte"]
        ACKDEC{"address<br/>in range?"}
        NACK["Driver_I2C0.Control(GENERATE_NACK)<br/>⚠ falls through into ACK's<br/>CR2 write, I2C_stm32f0xx.c:597-600"]
        ACK["Driver_I2C0.Control(GENERATE_ACK)"]
        SETFLAG["i2c_info.write_comp = 1<br/>km_i2c.c:226"]
        ISREND(["ISR returns"])
    end

    subgraph APPL["Application — main-loop, next super-loop pass"]
        WAITNODE["[wait]<br/>super-loop reaches i2c_recv_wait()<br/>km_i2c.c:485"]
        CHECKFLAG{"write_comp\nor read_comp\nset?"}
        RETURNEARLY["return — nothing to do this pass"]
        SWITCHSTATE{"i2c_info.state ==\nWait_Write?"}
        XFERSIZE{"xfer_size\n== 1 or > 1?"}
        READPATH["[register READ request]<br/>→ see 04_communication_tx.md"]
        WRITEPATH["[register WRITE]<br/>decode second_addr, mutate\nRTC / g_io_extend_memory / timers<br/>km_i2c.c:637-731"]
        REARM["change_state(Wait_Write, ..., SlaveReceive, ...)<br/>km_i2c.c:274<br/>re-arm for next command"]
        MISMATCH["[protocol mismatch]<br/>i2c_sw_reset()<br/>→ see 07_error_handling_i2c.md"]
        END(["END"])
    end

    START --> BUS --> VEC --> HALEV --> SLAVEISR
    SLAVEISR --> DECRELOAD
    DECRELOAD -- yes --> VALIDADDR --> ACKDEC
    ACKDEC -- no --> NACK --> ISREND
    ACKDEC -- yes --> ACK --> ISREND
    DECRELOAD -- no --> ADDRCB --> DOCB --> RXCOMP
    SLAVEISR --> RXCPLT --> DOCB
    RXCOMP --> SETFLAG --> ISREND
    ISREND --> WAITNODE --> CHECKFLAG
    CHECKFLAG -- no --> RETURNEARLY --> END
    CHECKFLAG -- yes --> SWITCHSTATE
    SWITCHSTATE -- "no (mismatch)" --> MISMATCH --> END
    SWITCHSTATE -- yes --> XFERSIZE
    XFERSIZE -- "==1 (read request)" --> READPATH --> END
    XFERSIZE -- ">1 (write)" --> WRITEPATH --> REARM --> END
```

## Traceability

| Node | Function | File:Line |
|---|---|---|
| I2C1_IRQHandler | `I2C1_IRQHandler` | `main/Src/stm32f0xx_it.c:203` |
| I2C_Slave_ISR_IT (byte copy) | `I2C_Slave_ISR_IT` | `common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c.c:3432` |
| HAL_I2C_AddrCallback | `HAL_I2C_AddrCallback` | `common/Drivers/CMSIS/.../I2C_stm32f0xx.c:767` |
| HAL_I2C_SlaveRxCpltCallback | `HAL_I2C_SlaveRxCpltCallback` | `I2C_stm32f0xx.c:747` |
| do_callback | `do_callback` | `I2C_stm32f0xx.c:692` |
| rx_comp | `rx_comp` | `main/App/km_i2c.c:171` |
| valid_second_addr | `valid_second_addr` | `main/App/km_i2c.c:93` |
| I2Cx_Control NACK/ACK fallthrough | `I2Cx_Control` | `I2C_stm32f0xx.c:597-600` — see `05_data_flow.md` §5 item 5 |
| i2c_recv_wait | `i2c_recv_wait` | `main/App/km_i2c.c:485` |
| change_state | `change_state` | `main/App/km_i2c.c:274` |
| i2c_sw_reset | `i2c_sw_reset` | `main/App/km_i2c.c:356` |
