# Các kịch bản gọi `i2c_dw_isr()` (thread `irq/56-8e800600`, I2C1 DesignWare)

> Bám code thật trong `Kernel/Src`. File chính: `drivers/i2c/busses/i2c-designware-master.c`.
> Thiết bị: `i2c1@8e8006000` (`quartz-sb.dtsi:340`), `GIC_SPI 45`, 100 kHz, slave: `pscpu_rtc@3d`, `pscpu@3e`.
> Client chính trên bus này: `drivers/rtc/rtc-pscpu.c` (gọi `i2c_master_send` rồi `i2c_master_recv`).
> Các nhánh `CONFIG_KM_BIZHUB` được đánh dấu rõ. Phần suy luận được ghi chú là "giả thuyết".

## 0. Cây quyết định bên trong `i2c_dw_isr()` (dòng 680)

```mermaid
graph TD
    A["ISR duoc goi (thread irq/56)"] --> B["doc DW_IC_ENABLE va DW_IC_RAW_INTR_STAT"]
    B --> C{"enabled = 0 hoac raw stat chi co ACTIVITY?"}
    C -->|"Khong"| D["i2c_dw_irq_handler_master()"]
    C -->|"Co, ban upstream"| E["return IRQ_NONE"]
    C -->|"Co, ban KM_BIZHUB (OP_BTS-65237)"| F["CLR_INTR, INTR_MASK = 0, return IRQ_HANDLED"]
    D --> G["i2c_dw_read_clear_intrbits(): doc INTR_STAT, xoa tung bit CLR_*"]
    G --> H{"TX_ABRT?"}
    H -->|"Co"| I["cmd_err = TX_ABRT, status = IDLE, INTR_MASK = 0"]
    H -->|"Khong"| J["RX_FULL: i2c_dw_read, TX_EMPTY: i2c_dw_xfer_msg"]
    I --> K["complete(cmd_complete)"]
    J --> L{"STOP_DET hoac msg_err?"}
    L -->|"Co"| K
    L -->|"Khong"| M["thoat, cho ngat ke tiep"]
```

## 1. Boi canh: ISR duoc dang ky va vi sao chay trong thread `irq/56`

```mermaid
sequenceDiagram
    participant P as dw_i2c_probe (platdrv)
    participant M as i2c_dw_probe_master (master.c:843)
    participant IRQ as kernel/irq/manage.c
    participant GIC as GIC SPI 45
    participant T as thread irq/56-8e800600

    P->>M: probe thiet bi snps,designware-i2c
    M->>M: i2c_dw_disable_int(dev)
    M->>IRQ: devm_request_irq(dev->irq, i2c_dw_isr, IRQF_SHARED hoac NO_SUSPEND)
    Note over IRQ: CONFIG_IRQ_FORCED_THREADING=y nen handler thuong bi ep chay trong thread
    IRQ->>T: kthread_create irq_thread, ten irq/56-<dev_name> (manage.c:1364)
    Note over T: Ten bi cat 15 ky tu thanh irq/56-8e800600
    GIC-->>T: ngat IRQ 56 (level high)
    T->>T: goi i2c_dw_isr(irq, dev)
```

Lưu ý: forced threading chỉ chạy thật nếu cmdline có `threadirqs`. Tôi chưa kiểm tra bootargs.

## 2. Kịch bản A: ghi bình thường (`i2c_master_send`)

Ví dụ: RTC driver ghi địa chỉ thanh ghi (`rtc-pscpu.c:116`).

```mermaid
sequenceDiagram
    participant C as rtc-pscpu (i2c_master_send)
    participant X as i2c_dw_xfer (master.c:481)
    participant HW as DesignWare I2C
    participant I as irq/56: i2c_dw_isr
    participant H as i2c_dw_irq_handler_master
    participant S as Slave 0x3d

    C->>X: master_xfer(msgs, num)
    X->>X: reinit_completion, dev->status = IDLE, msg_write_idx = 0
    X->>X: i2c_dw_wait_bus_not_busy
    X->>HW: i2c_dw_xfer_init: disable, ghi TAR, enable, CLR_INTR, INTR_MASK = MASTER_MASK
    X->>X: wait_for_completion_timeout(adap->timeout)
    HW-->>I: ngat TX_EMPTY (FIFO TX trong)
    I->>I: enabled != 0 va co bit khac ACTIVITY
    I->>H: i2c_dw_irq_handler_master
    H->>H: read_clear_intrbits
    H->>HW: i2c_dw_xfer_msg: do byte vao FIFO, byte cuoi gan STOP (BIT 9)
    H->>HW: INTR_MASK bo TX_EMPTY khi het du lieu
    HW->>S: START, dia chi, du lieu, STOP
    S-->>HW: ACK
    HW-->>I: ngat STOP_DET
    I->>H: i2c_dw_irq_handler_master
    H->>X: complete(cmd_complete)
    X->>HW: __i2c_dw_disable_nowait
    X-->>C: return num
```

## 3. Kịch bản B: đọc bình thường (`i2c_master_recv`)

```mermaid
sequenceDiagram
    participant C as rtc-pscpu (i2c_master_recv)
    participant X as i2c_dw_xfer
    participant HW as DesignWare I2C
    participant I as irq/56: i2c_dw_isr
    participant H as i2c_dw_irq_handler_master
    participant S as Slave 0x3d

    C->>X: master_xfer(msg doc, 1)
    X->>HW: i2c_dw_xfer_init, INTR_MASK = MASTER_MASK
    X->>X: wait_for_completion_timeout
    HW-->>I: TX_EMPTY
    I->>H: xu ly TX_EMPTY
    H->>HW: i2c_dw_xfer_msg: ghi DATA_CMD = 0x100 (lenh doc), byte cuoi them STOP
    Note over H,HW: gioi han theo rx_fifo_depth, rx_outstanding tang moi lenh doc
    HW->>S: START, dia chi doc
    S-->>HW: du lieu (nhieu byte)
    HW-->>I: RX_FULL (FIFO nhan dat nguong)
    I->>H: xu ly RX_FULL
    H->>HW: i2c_dw_read: doc RXFLR roi doc DATA_CMD tung byte, rx_outstanding giam
    HW-->>I: STOP_DET
    I->>H: xu ly STOP_DET
    H->>X: complete
    X-->>C: return num
```

Nếu message có `I2C_M_RECV_LEN` (SMBus block read) thì ISR tắt `TX_EMPTY` trong lúc chờ byte độ dài rồi `i2c_dw_recv_len` bật lại (dòng 396).
Tôi chưa thấy driver trên bus này dùng kiểu đó.

## 4. Kịch bản C: slave NACK hoặc mất trọng tài (TX_ABRT)

```mermaid
sequenceDiagram
    participant C as rtc-pscpu
    participant X as i2c_dw_xfer
    participant HW as DesignWare I2C
    participant I as irq/56: i2c_dw_isr
    participant H as i2c_dw_irq_handler_master
    participant A as i2c_dw_handle_tx_abort (common.c:578)

    C->>X: master_xfer
    X->>X: wait_for_completion_timeout
    HW-->>I: ngat TX_ABRT (NACK, hoac arbitration lost, ...)
    I->>H: i2c_dw_irq_handler_master
    H->>H: read_clear_intrbits: luu TX_ABRT_SOURCE vao dev->abort_source truoc khi doc CLR_TX_ABRT
    H->>H: cmd_err = TX_ABRT, status = IDLE
    H->>HW: INTR_MASK = 0 (FIFO da bi xoa, bo qua phan con lai)
    H->>X: complete(cmd_complete)
    X->>HW: __i2c_dw_disable_nowait
    X->>A: cmd_err == TX_ABRT
    alt NOACK
        A-->>X: -EREMOTEIO
    else arbitration lost
        A-->>X: -EAGAIN
    else khac
        A-->>X: -EIO (va dev_err nguyen nhan)
    end
    X-->>C: ma loi
    Note over X,C: i2c core chi thu lai (adap->retries = 3) khi ret == -EAGAIN (i2c-core-base.c:2104-2111)
    C->>C: rtc-pscpu: sc_retry_cnt++, msleep(500), goto retry
    Note over C: rtc-pscpu.h: PSCPU_I2C_RETRY_MAX = 10, RETRY_RESET = 5 (lan thu 5 gui lenh reset PS-CPU)
    C->>C: qua 10 lan: set_PmuRtcNewdetailCode (NewDC 2201E...), goto retry_over
```

Điểm cần nhớ: với `-EREMOTEIO` và `-EIO`, driver I2C không tự thử lại; việc thử lại 10 lần là của `rtc-pscpu.c`.

## 5. Kịch bản D: ISR không đến, hoặc đến mà không ra `complete()` (timeout và recovery)

Khi bus bị kẹt (SDA/SCL giữ thấp), hoặc ngắt không đến, hoặc ISR đã mask hết.

```mermaid
sequenceDiagram
    participant C as rtc-pscpu
    participant X as i2c_dw_xfer
    participant HW as DesignWare I2C
    participant I as irq/56: i2c_dw_isr
    participant R as i2c_recover_bus (km_i2c_*)

    C->>X: master_xfer
    X->>HW: i2c_dw_xfer_init
    X->>X: wait_for_completion_timeout(adap->timeout)
    Note over HW,I: khong co STOP_DET hoac TX_ABRT, hoac ISR da bi mask
    X->>X: het gio: dev_err controller timed out
    X->>R: i2c_recover_bus
    R->>R: km_i2c_prepare_recovery: doi chan scl va sda sang GPIOA 0 va 1 (pad 0xe8308000)
    R->>R: i2c_generic_scl_recovery: dap xung SCL cho toi khi SDA nha
    R->>R: km_i2c_unprepare_recovery: tra chan ve I2C
    X->>HW: i2c_dw_init_master
    X-->>C: -ETIMEDOUT
    C->>C: retry nhu kich ban C (msleep 500, toi da 10 lan)
```

Recovery GPIO chỉ có cho bus có `scl-gpios`/`sda-gpios`. Trong dtsi chỉ `i2c1` có, và địa chỉ pad/gpio ghi cứng trong code (OP_BTS-45386).

## 6. Kịch bản E: ngắt "thừa" (IRQ storm) và nhánh `KM_BIZHUB`

### E1. Ngắt đến khi controller đang tắt (`enabled == 0`)

Controller bị tắt trong `i2c_dw_xfer_init` (`__i2c_dw_disable`) và sau mỗi transfer (`__i2c_dw_disable_nowait`).
Nếu đường ngắt vẫn còn báo lúc đó, upstream trả `IRQ_NONE` và kernel có thể tắt IRQ ("nobody cared"); bản BIZHUB xử lý thế này:

```mermaid
sequenceDiagram
    participant GIC as GIC SPI 45
    participant I as irq/56: i2c_dw_isr
    participant HW as DesignWare I2C
    participant X as i2c_dw_xfer

    X->>HW: __i2c_dw_disable_nowait (ket thuc transfer truoc)
    GIC-->>I: ngat con ton dong (dong bo tre hoac nguon ngat chua duoc xoa)
    I->>HW: doc ENABLE = 0
    I->>HW: doc RAW_INTR_STAT
    Note over I: !enabled nen vao nhanh BIZHUB (master.c:688-696)
    I->>HW: doc CLR_INTR (xoa tat ca nguon ngat)
    I->>HW: ghi INTR_MASK = 0
    I-->>GIC: return IRQ_HANDLED
    Note over I: Thread irq/56 khong bi tat, khong ngon CPU nua
    X->>HW: transfer ke tiep: i2c_dw_xfer_init ghi lai INTR_MASK = MASTER_MASK
```

### E2. Giả thuyết: raw stat chỉ có ACTIVITY giữa lúc đang transfer

Chưa chứng minh được. Nếu xảy ra, bản BIZHUB mask hết ngắt, transfer đang chờ sẽ treo tới timeout:

```mermaid
sequenceDiagram
    participant X as i2c_dw_xfer (dang cho)
    participant I as irq/56: i2c_dw_isr
    participant HW as DesignWare I2C

    X->>X: wait_for_completion_timeout
    HW-->>I: ngat, RAW_INTR_STAT chi con bit ACTIVITY (gia thuyet)
    I->>HW: CLR_INTR, INTR_MASK = 0, return IRQ_HANDLED
    Note over X,I: Khong con ngat TX_EMPTY hay STOP_DET nao duoc phep, complete() khong bao gio duoc goi
    X->>X: het gio: controller timed out
    X->>X: i2c_recover_bus roi return -ETIMEDOUT (xem kich ban D)
```

Cơ sở để nghi: `INTR_MASK` còn được ghi từ `i2c_dw_xfer_init` và `i2c_dw_xfer_msg`, không có lock chung với ISR.
Ngược lại, `TX_EMPTY` thô thường đã bật khi FIFO trống, nên khả năng bit duy nhất là `ACTIVITY` giữa lúc chờ thấp.

## 7. Bảng tóm tắt kịch bản

| Kịch bản | Cách ISR được gọi | Kết thúc | Kết quả `i2c_dw_xfer` |
|---|---|---|---|
| A. Ghi OK | `TX_EMPTY` (nhiều lần), rồi `STOP_DET` | `complete()` | `num` |
| B. Đọc OK | `TX_EMPTY`, `RX_FULL` (nhiều lần), `STOP_DET` | `complete()` | `num` |
| C. NACK/mất trọng tài | `TX_ABRT` | `complete()` | `-EREMOTEIO`/`-EAGAIN`/`-EIO` |
| D. Kẹt bus | không đến, hoặc không ra `complete()` | timeout | `-ETIMEDOUT` sau recovery |
| E1. Ngắt thừa lúc tắt | `enabled == 0` | không `complete()`, chỉ mask | không ảnh hưởng transfer sau |
| E2. Ngắt chỉ ACTIVITY giữa transfer | giả thuyết | timeout | `-ETIMEDOUT` |

## 8. Chưa xác minh

- Thật sự có `threadirqs` trong bootargs hay không (cơ sở để `irq/56` tồn tại).
- Có xảy ra E2 trên máy thật hay không. Cần log `controller timed out` kèm `RAW_INTR_STAT` mới kết luận.
- `INTR_MASK` có race giữa ISR và `i2c_dw_xfer_msg` hay không (không có lock chung trong code đã đọc).
- Cơ chế `i2c_dw_acquire_lock`/`wait_bus_not_busy` chi tiết (chưa đọc).
