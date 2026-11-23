# panelcomdrv.c – Sequence Diagrams các luồng chức năng chính

> Tài liệu phân tích `panelcomdrv.c` – Linux kernel driver giao tiếp giữa hệ thống MFP và Panel MCU qua UART.
> Mỗi luồng có sequence diagram (Mermaid) và bảng giải thích từng action: **cơ chế / mục đích / output**.

## Mục lục

1. [Tổng quan kiến trúc](#tổng-quan-kiến-trúc)
2. [Luồng 1: Khởi tạo và hủy driver](#luồng-1-khởi-tạo-và-hủy-driver)
3. [Luồng 2: Gửi lệnh xuống Panel MCU (TX)](#luồng-2-gửi-lệnh-xuống-panel-mcu-tx)
4. [Luồng 3: Nhận và phân tích dữ liệu từ Panel MCU (RX)](#luồng-3-nhận-và-phân-tích-dữ-liệu-từ-panel-mcu-rx)
5. [Luồng 4: Xử lý input (touch và hard key)](#luồng-4-xử-lý-input-touch-và-hard-key)
6. [Luồng 5: Điều khiển thiết bị panel](#luồng-5-điều-khiển-thiết-bị-panel)
7. [Luồng 6: Suspend và resume](#luồng-6-suspend-và-resume)
8. [Luồng 7: Truyền dữ liệu nhiều bước](#luồng-7-truyền-dữ-liệu-nhiều-bước)
9. [Luồng 8: Logging, xử lý lỗi và test](#luồng-8-logging-xử-lý-lỗi-và-test)
10. [Tổng kết](#tổng-kết)

---

## Tổng quan kiến trúc

```text
Ứng dụng/User space
        ↕ read / poll / ioctl
panelcomdrv.c
        ↕ UART serial protocol
Panel MCU
        ↕
Touch panel, hard key, LED, backlight, buzzer, NFC...
```

**Cách đọc nhanh:** gần như mọi thao tác đều đi qua hai "đường ống" chung:

- **Gửi (TX):** `RequestXxx` → `SetTxRingBuffer` → `AnalyzeControlTask` → UART
- **Nhận (RX):** UART → analyze ring buffer → `AnalyzeControlTask` → `AnalyzeSerialCommand`

Các luồng còn lại chủ yếu tái sử dụng hai đường ống này.

**Frame giao thức UART:**

```text
| Header | Length | Code | Data ... | Checksum |
Checksum = (~sum(Code + Data) + 1) & 0x7F   (7-bit two's complement)
```

---

## Luồng 1: Khởi tạo và hủy driver

```mermaid
sequenceDiagram
    participant K as Linux Kernel
    participant D as PanelComDrv_Install
    participant P as ProcFS
    participant C as CharDevice
    participant I as Linux Input
    participant T as AnalyzeControlTask
    participant U as Panel UART Driver
    participant M as Panel MCU

    K->>D: 1. module_init goi PanelComDrv_Install
    D->>D: 2. memset pPnlComManage, init spinlock, set flag
    D->>P: 3. Tao /proc/.../panelcomdrv va info, test_flag, key_touch, panel_touch
    D->>C: 4. alloc_chrdev_region, cdev_init, cdev_add
    D->>C: 5. device_create keypanelcom, outpanelcom, keypanelcom_logp
    D->>I: 6. CreateTouchpanelDevice, dang ky input device 2 slot
    D->>D: 7. tasklet_init PanelCom_tasklet
    D->>T: 8. kthread_create va sched_setscheduler SCHED_RR
    D->>D: 9. pnlLogStart khoi tao log buffer
    D->>D: 10. PanelComDeviceInitialize reset buffer
    D->>T: 11. wake_up_process, thread vao trang thai cho
    D->>U: 12. sysBspPanelUartSetCallback THRF_SerialCommunication
    D->>D: 13. PanelPowerOn, return ngay
    D->>D: 14. PanelComDriverInitialize, SuspendPrintk 300ms
    opt ENV0208
        D->>D: 15. Cho InitReqFlag toi da 5 giay
    end
    D->>M: 16. Gui SoftInfo x3, KMSoftInfo x2, BackLightInfo, OutState
    M-->>D: 17. Response duoc xu ly sau qua luong RX
    D-->>K: 18. Return 0, driver san sang

    Note over K,D: Khi rmmod
    K->>D: 19. module_exit goi PanelComDrv_UnInstall
    D->>C: 20. device_destroy, cdev_del, unregister_chrdev_region
    D->>I: 21. DestroyTouchpanelDevice
```

| # | Action | Cơ chế / Mục đích | Output |
|---|---|---|---|
| 1 | `module_init` | Kernel gọi hàm khởi tạo khi load module. | Driver bắt đầu setup. |
| 2 | Reset `pPnlComManage` | Xóa sạch trạng thái trung tâm và khởi tạo spinlock. Các cờ ban đầu: `InitFlag=WAIT`, `AnalyzeFlag=WAIT`, `bl_LoggingFlag=True`. `InitReqFlag` phụ thuộc việc U-Boot đã khởi tạo panel hay chưa (dựa trên `ul_VramAddr`). | Trạng thái driver sạch, các khóa sẵn sàng. |
| 3 | Tạo procfs | Tạo các entry dùng để debug và test. | Có các file trong `/proc/km/panelcomdrv/`. |
| 4 | Đăng ký char device | Xin major number và gắn bảng `file_operations` gồm open, read, poll, ioctl, close. | Kernel biết hàm nào xử lý file của driver. |
| 5 | `device_create` | Tạo device node để application mở được. | Có 3 file `/dev/...`. |
| 6 | Tạo input device | Đăng ký touch panel với Linux Input Subsystem, gồm trục X/Y và 2 slot multi-touch. | Touch event được phát theo chuẩn Linux. |
| 7 | `tasklet_init` | Chuẩn bị cơ chế chuyển phần việc từ context ngắt sang kernel thread. | Tasklet sẵn sàng. |
| 8 | Tạo `AnalyzeControlTask` | Tạo thread xử lý trung tâm, đặt lịch real-time để phản hồi nhanh. | Thread được tạo nhưng chưa chạy. |
| 9 | `pnlLogStart` | Khởi tạo ring buffer log TX/RX 64KB. | Có thể ghi log UART. |
| 10–11 | `PanelComDeviceInitialize` | Reset TX ring, analyze ring, key ring, trạng thái buzzer, sau đó đánh thức thread. | Thread chạy và chờ ở `wait_event`. |
| 12 | Đăng ký callback UART | Khi UART nhận dữ liệu, BSP sẽ gọi `THRF_SerialCommunication`. | Luồng RX được kết nối. |
| 13 | `PanelPowerOn` | Hàm có `return` ngay đầu nên phần GPIO phía sau không chạy. | Không thay đổi GPIO. |
| 14 | `SuspendPrintk` | Tạm tắt console 300ms vì `printk` có thể gây overrun UART của panel. | Console tự bật lại sau 300ms. |
| 15 | Chờ initial request | Chỉ với `ENV0208`: chờ Panel MCU sẵn sàng, tối đa 5 giây. | Tránh gửi lệnh khi MCU chưa sẵn sàng. |
| 16–17 | Gửi lệnh truy vấn | Hỏi version, KM software info, độ sáng backlight và trạng thái output. | Response cập nhật `strPnlSoftInfo`, `strKMSoftInfo`, `strBackLightInfo`... |
| 18 | Return | Load module thành công. | Driver hoạt động. |
| 19–21 | UnInstall | Hủy device, `cdev`, major number và input device. | Tài nguyên chính được giải phóng. |

> **Lưu ý:** `PanelComDrv_UnInstall` không dừng `AnalyzeControlTask`, không xóa procfs entry và không hủy timer. Cần chú ý nếu module bị unload rồi load lại.

---

## Luồng 2: Gửi lệnh xuống Panel MCU (TX)

```mermaid
sequenceDiagram
    participant A as Application
    participant IO as PanelComDrv_Ioctl
    participant R as RequestXxx
    participant CM as CreateCmd_Xxx
    participant TR as TxBuffer ring
    participant T as AnalyzeControlTask
    participant TC as TxCommndBuffer ring
    participant U as Panel UART
    participant M as Panel MCU

    A->>IO: 1. ioctl /dev/outpanelcom
    IO->>IO: 2. Kiem tra minor va co download_panelfw
    IO->>R: 3. Goi RequestXxx theo command
    R->>R: 4. copy_from_user va kiem tra tham so
    R->>R: 5. spin_lock TxComSpinLock
    R->>CM: 6. Tao frame Header Len Code Data
    CM->>CM: 7. CreateCheckSum
    R->>TR: 8. SetTxRingBuffer
    alt Dang suspend
        TR-->>R: 9a. Bo lenh, van tra ve OK
    else Slot FREE
        TR->>T: 9b. Copy frame, AnalyzeFlag ACTIVE, wake_up
    else Buffer day
        TR-->>R: 9c. Tra ve ENFILE, ep BuzzerOnOffState OFF
    end
    R->>R: 10. spin_unlock va cap nhat trang thai noi bo
    R-->>A: 11. ioctl return
    T->>T: 12. Thread thuc day
    T->>TC: 13. ForwardSerialCommand chuyen frame thanh byte stream
    opt Lenh Buzzer ON
        T->>T: 14. Dang ky timer tat buzzer
    end
    T->>U: 15. SerialStartTx, day toi da FIFO size byte
    U->>M: 16. Truyen UART
    T->>T: 17. Giam i32_AnalyzeCount, het viec thi ngu
```

| # | Action | Cơ chế / Mục đích | Output |
|---|---|---|---|
| 1 | `ioctl` | Application yêu cầu điều khiển panel, ví dụ bật LED. | Vào driver. |
| 2 | Kiểm tra | Chỉ `/dev/outpanelcom` được nhận `ioctl`. Khi `download_panelfw=True`, các lệnh có gửi UART bị bỏ qua để không chen vào firmware download. | Lệnh bị chặn hoặc tiếp tục. |
| 3 | Dispatch | `switch(aioCmd)` chọn hàm `RequestXxx` tương ứng. | Chạy hàm xử lý đúng chức năng. |
| 4 | Validate | Copy dữ liệu từ user space vào kernel và kiểm tra giá trị hợp lệ (LED number, blink time...). | Trả `-EINVAL` / `-EFAULT` nếu lỗi. |
| 5 | Lock | Không cho nhiều luồng ghi TX ring cùng lúc. | Có quyền ghi độc quyền. |
| 6–7 | Tạo frame | Đóng gói theo giao thức `Header, Length, Code, Data, Checksum`. | Một frame hoàn chỉnh. |
| 8 | Đưa vào TX ring | TX ring là hàng đợi nhiều ô, mỗi ô chứa một frame. | Frame được xếp hàng. |
| 9a | Suspend | Khi `iIsSuspend=1`, lệnh bị bỏ nhưng vẫn trả thành công. | Lệnh mất mà caller không biết. |
| 9b | Bình thường | Đánh dấu ô `WAIT`, tăng con trỏ ghi, tăng `i32_AnalyzeCount`, đánh thức thread. | Thread được báo có việc. |
| 9c | Overflow | Hàng đợi đầy, trả `-ENFILE`. Buzzer state bị ép về `OFF` vì có thể không còn chỗ gửi lệnh tắt. | Báo lỗi lên caller. |
| 10–11 | Unlock và return | Cập nhật trạng thái driver (ví dụ `LedDutyState`), trả kết quả. | Application nhận mã kết quả. **Thành công chỉ có nghĩa lệnh đã vào hàng đợi, chưa phải đã gửi UART.** |
| 12–13 | Forward | Thread lấy frame khỏi TX ring, chuyển sang byte ring để UART đọc theo từng byte. | Dữ liệu sẵn sàng truyền. |
| 14 | Buzzer timer | Nếu là lệnh `Buzzer ON`, hẹn giờ kiểm tra để tắt buzzer (xem luồng 5b). | Timer được đăng ký. |
| 15–16 | Gửi UART | Đẩy byte vào FIFO, ghi log TX, kích hoạt UART. Phần còn lại UART driver lấy tiếp qua `PanelCom_getTxCommand`. | Panel MCU nhận lệnh. |
| 17 | Counter | Còn yêu cầu thì lặp lại, không thì thread ngủ. | Không bỏ sót yêu cầu. |

---

## Luồng 3: Nhận và phân tích dữ liệu từ Panel MCU (RX)

```mermaid
sequenceDiagram
    participant M as Panel MCU
    participant U as Panel UART ISR BSP
    participant RX as RxSerialCommunication
    participant AR as AnalyzeBuffer ring
    participant TL as PanelCom_tasklet
    participant T as AnalyzeControlTask
    participant AN as AnalyzeReceiveData
    participant SC as AnalyzeSerialCommand

    M->>U: 1. Gui byte qua UART
    U->>RX: 2. THRF_SerialCommunication va SerialComProsess
    loop Con du lieu trong UART
        RX->>RX: 3. sysBspPanelUartRxChar, luu ban sao UartRxBuffer
    end
    RX->>AR: 4. SetAnalyzeRingBuffer
    alt Ring day
        AR-->>RX: 5a. ThAssert va bo du lieu
    end
    RX->>TL: 5b. Co du lieu thi tasklet_schedule
    TL->>T: 6. AnalyzeFlag ACTIVE va wake_up
    T->>AN: 7. Co du lieu thi goi AnalyzeReceiveData
    loop Tung frame
        AN->>AN: 8. Lock va tim header, bo byte rac phia truoc
        AN->>AN: 9. Doc length, kiem tra min max
        alt Chua du byte
            AN->>AN: 10a. Unlock va cho lan nhan sau
        else Du frame
            AN->>AN: 10b. Copy frame, ghi log RX, unlock
            AN->>AN: 11. AnalyzeCheckSum
            alt Checksum sai
                AN->>AN: 12a. Luu snapshot lan dau, bo qua 1 byte
            else Checksum dung
                AN->>AR: 12b. Lock va xoa frame khoi ring
                AN->>SC: 13. Phan tich command
            end
        end
    end
    SC->>SC: 14. Loc Android, Lock panel, Display lock
    SC->>SC: 15. Switch theo command code va chuyen sang nhanh xu ly
```

| # | Action | Cơ chế / Mục đích | Output |
|---|---|---|---|
| 1–2 | UART ngắt | MCU gửi dữ liệu, ISR của BSP gọi callback đã đăng ký ở luồng 1. | Driver được thông báo. |
| 3 | Đọc byte | Đọc hết FIFO. Mỗi byte được sao vào `UartRxBuffer` (vòng 2048 byte) để điều tra lỗi. | Có dữ liệu thô. |
| 4–5a | Vào analyze ring | Ghi vào ring 2048 byte. Ring đầy → assert và bỏ dữ liệu. | Dữ liệu chờ phân tích. |
| 5b–6 | Tasklet | **Không phân tích trong ngắt** vì ngắt phải kết thúc nhanh. Tasklet chỉ đánh thức thread. | Thread được báo có việc. |
| 7 | Bắt đầu phân tích | Thread kiểm tra ring còn dữ liệu. | Vào vòng xử lý frame. |
| 8 | Tìm header | Dữ liệu có thể nhiễu nên bỏ mọi byte trước header. Dùng `sysBspPanelSpinLock` vì ISR cũng ghi vào cùng ring. | Con trỏ ring nằm tại đầu frame. |
| 9 | Kiểm tra length | Độ dài phải trong khoảng hợp lệ. Sai → nhảy 1 byte rồi tìm header tiếp. | Loại frame hỏng. |
| 10a | Thiếu byte | Frame chưa đủ thì giữ nguyên, chờ lần nhận sau. | Không mất dữ liệu. |
| 10b | Copy frame | Copy frame ra buffer cục bộ, ghi log RX. | Có một frame đầy đủ. |
| 11–12a | Checksum sai | Lần lỗi đầu tiên chụp snapshot (UART buffer, ring, command) phục vụ điều tra D403. Sau đó nhảy 1 byte. | Frame lỗi bị loại, bằng chứng được giữ. |
| 12b | Checksum đúng | Xóa frame khỏi ring. | Ring giảm số byte chiếm. |
| 13–14 | Lọc | Ngoài ACK, suspend, NFC, các lệnh khác bị bỏ khi panel/tablet bị lock hoặc màn hình lock. Khi Android đang kết nối, chỉ một số key được xử lý. | Input bị chặn đúng điều kiện. |
| 15 | Dispatch | Chuyển sang nhánh touch, key, ACK/NACK, soft info, firmware response, NFC, suspend... | Sự kiện được xử lý (luồng 4–8). |

---

## Luồng 4: Xử lý input (touch và hard key)

### 4a. Touch panel

```mermaid
sequenceDiagram
    participant SC as AnalyzeSerialCommand
    participant TC as AnalyzeTouchCommand
    participant TM as PollingTimer
    participant IN as InputMultitouchB
    participant LI as Linux Input
    participant APP as Application UI
    participant M as Panel MCU

    SC->>TC: 1. Lenh TOUCHPANEL ON, RESPONSE, DRAG
    loop 2 diem cham
        TC->>TC: 2. Giai ma X Y va state
        opt Dang cham va panel SResistance
            TC->>TM: 3. StartTouchpanelPollingTimer 100ms
        end
    end
    opt Dang calibration
        TC->>TC: 4. Luu khi cham, khoi phuc khi nha
    end
    TC->>IN: 5. Goi InputMultitouchB
    IN->>IN: 6. IsInputSourceAvailable kiem tra autoplay
    IN->>LI: 7. Bao slot 0 va slot 1
    IN->>LI: 8. input_mt_sync_frame va input_sync
    LI-->>APP: 9. Ung dung nhan touch event
    TM->>TM: 10. Het 100ms, slot 0 van dang cham
    TM->>M: 11. RequestTouchPanelState
    M-->>SC: 12. Tra ve toa do moi
```

| # | Action | Cơ chế / Mục đích | Output |
|---|---|---|---|
| 1 | Nhận lệnh touch | MCU báo ngón tay chạm, kéo hoặc phản hồi tọa độ. | Vào hàm giải mã. |
| 2 | Giải mã | X/Y nằm trong 2 byte, mỗi byte 7 bit. Bit `0x40` cho biết đang chạm hay nhả. `0xFF` là không có dữ liệu. | Có `TOUCHPANEL_INFO[2]`. |
| 3 | Polling timer | Panel resistive single-touch không tự báo liên tục khi giữ tay, nên driver chủ động hỏi lại. | Timer chạy. |
| 4 | Calibration | Tọa độ lúc nhả có thể sai, nên dùng tọa độ lúc chạm và chỉ đổi state sang nhả. | Calibration chính xác hơn. |
| 5–6 | Kiểm tra nguồn | Khi autoplay đang chạy, chỉ input từ autoplay được chấp nhận. | Chặn tay người dùng chen vào test. |
| 7 | Báo slot | Mỗi ngón tay là một slot. Báo `ON/HOLD` kèm X/Y hoặc `OFF` khi nhả. Slot 2 chỉ dùng cho panel không phải resistive single-touch. | Event multi-touch type B. |
| 8–9 | Sync | Đánh dấu kết thúc một khung sự kiện để userspace nhận trọn vẹn. | Ứng dụng nhận tap, kéo, thả. |
| 10–12 | Polling | Hết 100ms mà vẫn chạm thì hỏi tọa độ mới; response quay lại bước 1. | Kéo trên resistive panel liên tục hơn. |

### 4b. Hard key

```mermaid
sequenceDiagram
    participant SC as AnalyzeSerialCommand
    participant HK as InputHardkey
    participant KB as KeyPanelBuffer
    participant LP as KeyPanelBuffer_LogP
    participant APP as Application
    participant DRV as Read Select

    SC->>SC: 1. Lenh NONLOCKKEY ON OFF
    alt keypanelcom chua open
        SC-->>SC: 2a. Bo qua
    else Da open
        SC->>HK: 2b. InputHardkey
        HK->>HK: 3. Kiem tra autoplay va lock KeyComSpinLock
        HK->>KB: 4. SetKeyPanelBuffer, ON luu keycode, OFF luu 0x00
        KB->>DRV: 5. wake_up SelectKeyWaitq
        opt keypanelcom_logp da open
            SC->>LP: 6. Ghi ban sao cho LogProcess
        end
    end
    APP->>DRV: 7. poll /dev/keypanelcom
    DRV-->>APP: 8. POLLIN khi co du lieu
    APP->>DRV: 9. read
    DRV->>KB: 10. Lock va lay byte, cap nhat con tro
    DRV-->>APP: 11. copy_to_user keycode
```

| # | Action | Cơ chế / Mục đích | Output |
|---|---|---|---|
| 1 | Key event | MCU báo phím nhấn kèm keycode, hoặc phím nhả. | Vào nhánh key. |
| 2a/2b | Kiểm tra open | Chưa có application mở `/dev/keypanelcom` thì không lưu event. | Tránh đầy buffer vô ích. |
| 3–4 | Lưu event | Ghi vào ring buffer dạng byte: khác 0 là keycode khi nhấn, `0x00` là nhả. | Event được xếp hàng. |
| 5 | Đánh thức | Báo process đang chờ `poll`. | Application được đánh thức. |
| 6 | Bản sao log | LogProcess dùng ring riêng nên không lấy mất event của application chính. | Hai consumer độc lập. |
| 7–8 | `poll` | Application ngủ đến khi có dữ liệu, không tốn CPU. | Nhận `POLLIN`. |
| 9–11 | `read` | Lấy tối đa số byte yêu cầu. Buffer rỗng → `-EAGAIN`. | Application nhận keycode. |

---

## Luồng 5: Điều khiển thiết bị panel

LED, brightness, CTSC, vibrator, sound, EEPROM write... đều theo mẫu của **luồng 2**. Hai ví dụ có logic riêng là backlight và buzzer.

### 5a. Backlight (đồng bộ với LCD/LVDS)

```mermaid
sequenceDiagram
    participant A as PowerManager App
    participant R as RequestBackLightSetting
    participant FB as LCD Framebuffer LVDS
    participant TX as TX Pipeline
    participant M as Panel MCU
    participant SC as AnalyzeSerialCommand

    A->>R: 1. ioctl BACKLIGHTONOFF_SET
    R->>R: 2. copy_from_user, tach co MainSwOFF, validate
    opt Bat backlight
        R->>FB: 3. LCD power up, fb resume, LVDS ON
    end
    R->>R: 4. reinit ACK completion va set ack_result_command_code
    R->>TX: 5. Tao va gui lenh BACKLIGHT ON OFF
    R->>R: 6. Cap nhat BackLightOnOffState
    TX->>M: 7. UART truyen lenh
    M-->>SC: 8. ACK
    SC->>R: 9. Code khop, complete
    R->>R: 10. Thoat cho, timeout chi in log
    opt Tat backlight
        alt Tat nguon chinh
            R->>FB: 11a. Bat LVDS truoc, sau do suspend va power down LCD
        else Tat thong thuong
            R->>FB: 11b. LVDS OFF, fb suspend, LCD power down
        end
    end
    R-->>A: 12. Return
```

| # | Action | Cơ chế / Mục đích | Output |
|---|---|---|---|
| 1–2 | Nhận yêu cầu | Bit `MAINSWOFF` cho biết thao tác tắt đến từ tắt nguồn chính; driver tách bit này trước khi kiểm tra ON/OFF. | Có giá trị ON/OFF hợp lệ. |
| 3 | Bật LCD trước | Cần nguồn LCD, clock, tín hiệu hình trước khi bật đèn, nếu không có thể thấy màn đen/nhiễu. | LCD sẵn sàng hiển thị. |
| 4 | Chuẩn bị chờ ACK | `ack_result_command_code` cho biết ACK đang chờ thuộc lệnh nào. | Sẵn sàng chờ phản hồi. |
| 5–7 | Gửi lệnh | Lệnh đi qua TX pipeline (luồng 2). | MCU bật/tắt đèn. |
| 8–10 | Nhận ACK | ACK đúng code → `complete` đánh thức caller. Timeout chỉ log rồi tiếp tục. | Đèn đổi trạng thái hoặc có log timeout. |
| 11a | Tắt nguồn chính | Giữ LVDS ON để SuperWarp chụp snapshot không bị khung đen. | Tránh lỗi màn đen sau SuperWarp. |
| 11b | Tắt thường | Tắt ngược thứ tự bật: tín hiệu → clock → nguồn. | LCD ngừng hoạt động sạch. |
| 12 | Return | Trả kết quả cho application. | Kết thúc. |

### 5b. Buzzer (ưu tiên âm thanh và tự tắt)

```mermaid
sequenceDiagram
    participant A as Application
    participant R as RequestBuzzerSetting
    participant TX as TX Pipeline
    participant TM as BuzzerTimer
    participant M as Panel MCU
    participant SC as AnalyzeSerialCommand

    A->>R: 1. ioctl BUZZER_SET type freq volume
    R->>R: 2. Validate
    alt Am dang keu co uu tien cao hon hoac bang
        R-->>A: 3a. Bo qua, return OK
    else Duoc phep keu
        R->>R: 3b. Tra bang iBuzzerControl lay pattern va thoi gian
        R->>TX: 4. Neu can thi gui BuzzerOff truoc
        R->>TX: 5. Neu tan so hoac am luong doi thi gui BuzzerSet
        R->>TX: 6. Gui BuzzerOn1 hoac BuzzerOn2
        R->>R: 7. Luu BuzzerType, Frequency, Volume
    end
    TX->>TM: 8. Khi forward lenh ON, dang ky timer theo BuzzerTimeOut
    TM->>M: 9. Het gio, RequestPanelOutState
    M-->>SC: 10. OUTSTATE_RESPONSE
    SC->>SC: 11. UpdateBuzzerState
    TM->>TM: 12. Neu van ON thi dang ky lai timer
```

| # | Action | Cơ chế / Mục đích | Output |
|---|---|---|---|
| 1–2 | Nhận yêu cầu | Kiểm tra loại âm, tần số, âm lượng. | Tham số hợp lệ. |
| 3a | Ưu tiên | Âm quan trọng (cảnh báo lỗi) không bị âm kém quan trọng cắt ngang. Riêng âm input confirm được phép lặp. | Âm hiện tại tiếp tục. |
| 3b | Tra bảng | Mỗi loại âm có pattern bật/tắt và thời lượng riêng (ví dụ âm lỗi 8,5 giây). | Có thông số phát. |
| 4–6 | Gửi lệnh | Tắt âm cũ, đổi cấu hình nếu cần, phát âm mới. | MCU phát âm. |
| 7 | Lưu trạng thái | Dùng để so ưu tiên ở lần gọi sau. | Trạng thái cập nhật. |
| 8–12 | Theo dõi kết thúc | Driver không tự biết khi nào MCU phát xong nên hỏi trạng thái theo chu kỳ. MCU báo OFF thì timer dừng. | Trạng thái buzzer trong driver khớp phần cứng. |

---

## Luồng 6: Suspend và resume

### 6a. Do PowerManager điều khiển qua ioctl

```mermaid
sequenceDiagram
    participant PM as PowerManager
    participant S as SerialCommuniCationSuspend
    participant TX as TX Pipeline
    participant M as Panel MCU
    participant SC as AnalyzeSerialCommand
    participant PMU as PMU Driver

    PM->>S: 1. ioctl COMMUNITATION_SUSPEND SUSPEND
    S->>S: 2. Luu iComSuspend_tmp, LCD signal OFF neu co config
    S->>TX: 3. RequestInputDeviceSetting False
    M-->>SC: 4. ACK, complete
    S->>S: 5. iIsSuspend 1, SoundPlayNum 0, luu ActiveDriver
    Note over PM,M: He thong ngu, TX bi bo qua

    PM->>S: 6. ioctl RESUME1
    S->>S: 7. iIsSuspend 0, reinit completion_recv_suspend
    S->>S: 8. PanelComDeviceInitialize reset buffer va state

    PM->>S: 9. ioctl RESUME2
    S->>S: 10. download_panelfw False, khoi phuc ActiveDriver
    M-->>SC: 11. SUSPEND_RESUME kem nguyen nhan wake
    SC->>PMU: 12. pmu_tx_sig hardkey touch NFC BLE
    SC->>S: 13. complete completion_recv_suspend
    S->>S: 14. LCD signal ON neu co config
    S->>TX: 15. RequestInputDeviceSetting True
    M-->>SC: 16. ACK
    S-->>PM: 17. Return
```

| # | Action | Cơ chế / Mục đích | Output |
|---|---|---|---|
| 1–2 | Bắt đầu suspend | Ghi nhớ pha hiện tại vào `iComSuspend_tmp` để luồng 6b sử dụng. | Pha được lưu. |
| 3–4 | Tắt input | Không nhận touch/key khi hệ thống chuyển sang ngủ; chờ ACK. | Panel ngừng gửi input. |
| 5 | Chặn TX | `iIsSuspend=1` khiến `SetTxRingBuffer` bỏ mọi lệnh. Lưu trạng thái open của các device. | Không gửi UART khi panel sắp tắt. |
| 6–8 | RESUME1 | Mở lại TX, reset toàn bộ buffer vì dữ liệu cũ không còn hợp lệ sau khi panel được cấp nguồn lại. | Driver ở trạng thái sạch. |
| 9–10 | RESUME2 | Khôi phục cờ `ActiveDriver` đã lưu (vì `PanelComDeviceInitialize` đã reset). | Application tiếp tục đọc key được. |
| 11–13 | MCU báo đã thức | Gửi lý do đánh thức cho PMU rồi `complete` để caller tiếp tục. | PMU biết nguồn đánh thức. |
| 14–16 | Bật input lại | Bật tín hiệu LCD nếu config hỗ trợ, enable touch/key, chờ ACK. | Panel hoạt động trở lại. |
| 17 | Return | Kết thúc resume. | Dùng bình thường. |

### 6b. Panel MCU tự khởi động lại (initial request/response)

```mermaid
sequenceDiagram
    participant M as Panel MCU
    participant SC as AnalyzeSerialCommand
    participant RC as SuspendRecover
    participant BL as BackLightRecover
    participant TX as TX Pipeline

    M-->>SC: 1. INITIAL_REQUEST, chi ENV0208
    SC->>SC: 2. InitReqFlag ACTIVE va wake_up
    SC->>TX: 3. RequestPanelInitialize
    M-->>SC: 4. INITIAL_RESPONSE
    SC->>RC: 5. SerialCommuniCationSuspendRecover
    opt Pha truoc la RESUME2
        RC->>TX: 6. Enable input va cho ACK
    end
    SC->>BL: 7. RequestBackLightSettingRecover theo state da luu
    opt State dang ON
        BL->>TX: 8. Bat lai LCD, LVDS va backlight, cho ACK
    end
```

| # | Action | Cơ chế / Mục đích | Output |
|---|---|---|---|
| 1–3 | MCU xin khởi tạo | Mở khóa chỗ chờ ở luồng 1 bước 15, rồi gửi lệnh initialize (phần sửa OP_BTS-20828). | MCU được khởi tạo lại. |
| 4–6 | Khôi phục input | MCU reset sau resume làm mất cấu hình touch/key nên driver bật lại. | Input hoạt động lại. |
| 7–8 | Khôi phục đèn | Bật lại backlight theo state đã lưu. State là OFF thì hàm trả `-EINVAL`, không làm gì. | Panel sáng lại. |

---

## Luồng 7: Truyền dữ liệu nhiều bước

### 7a. Download firmware cho Panel MCU

```mermaid
sequenceDiagram
    participant A as Updater App
    participant IO as PanelComDrv_Ioctl
    participant DL as downloadPanelFW
    participant TX as TX Pipeline
    participant M as Panel MCU
    participant SC as AnalyzeSerialCommand

    A->>IO: 1. ioctl PANEL_FW_DOWNLOAD
    IO->>IO: 2. download_panelfw True
    IO->>DL: 3. Goi downloadPanelFW
    DL->>DL: 4. Kiem tra size 0 hoac lon hon 256KB
    DL->>DL: 5. ssleep 10 giay
    DL->>TX: 6. Gui Start kem size
    M-->>SC: 7. ACK hoac NACK
    M-->>SC: 8. Ready
    DL->>DL: 9. kmalloc 4KB
    loop Den het file
        DL->>DL: 10. Moi 4KB copy_from_user
        DL->>TX: 11. Gui 1 block ASCII hex, cong checksum
        M-->>SC: 12. Data response, complete
        alt Response khac 0 hoac timeout
            DL-->>IO: 13a. WRITE_ERROR hoac SEQUENCE_ERROR
        end
    end
    DL->>TX: 14. Gui End kem checksum 16 bit
    M-->>SC: 15. Complete response
    DL-->>IO: 16. result va sum
    IO->>IO: 17. download_panelfw False, copy_to_user
    IO-->>A: 18. Return
```

| # | Action | Cơ chế / Mục đích | Output |
|---|---|---|---|
| 1–3 | Bắt đầu | Bật cờ để chặn các `ioctl` khác gửi UART chen vào. | Chế độ download độc quyền. |
| 4 | Kiểm tra size | Không cho phép file rỗng hoặc vượt dung lượng ROM. | Có thể trả `DO_NOT_UPDATE`. |
| 5 | Chờ 10 giây | Cho hệ thống ổn định trước khi ghi ROM. | Mất cố định 10 giây. |
| 6–8 | Bắt tay | Gửi `Start`, chờ ACK rồi chờ Ready. NACK/timeout dừng ngay. | MCU vào chế độ ghi. |
| 9–10 | Lấy dữ liệu | Copy từng khối 4KB để không cấp phát bộ nhớ lớn trong kernel. | Có dữ liệu trong kernel. |
| 11 | Gửi block | Mỗi byte đổi thành 2 ký tự ASCII hex (`0x3A` → `'3''A'`) để tránh xung đột với byte header. | Một block được gửi. |
| 12–13a | Chờ từng block | **Stop-and-wait**: block trước chưa xác nhận thì chưa gửi block sau. | Phát hiện lỗi ngay tại block hỏng. |
| 14–15 | Kết thúc | MCU so checksum tổng để xác nhận toàn bộ firmware. | Có thể trả `CHECKSUM_ERROR`. |
| 16–18 | Trả kết quả | Mở lại các `ioctl` khác, báo kết quả cho application. | `result=0` nếu thành công. |

### 7b. NFC authentication (MCU chủ động gửi dữ liệu lên)

```mermaid
sequenceDiagram
    participant M as Panel MCU
    participant SC as AnalyzeSerialCommand
    participant N as NFC Auth Handler
    participant TX as TX Pipeline
    participant A as Auth App

    A->>A: 1. ioctl NFC_AUTH_DATA_TRANS, cho completion
    M-->>SC: 2. AUTH_REQUEST type va size
    SC->>N: 3. SetNFCAuthDataStartRequest
    opt type random
        N->>A: 4. result 2, complete bao bat dau
    end
    N->>TX: 5. Gui READY
    loop Tung block
        M-->>SC: 6. AUTH_DATA
        SC->>N: 7. Giai ma hex, cong sum
        N->>TX: 8. Gui DATA_RESPONSE
    end
    M-->>SC: 9. AUTH_COMPLETE kem sum
    SC->>N: 10. So sanh sum
    N->>TX: 11. Gui COMPLETE_RESPONSE
    opt type auth
        N->>A: 12. result 1, complete
    end
    A->>A: 13. copy_to_user random va auth data, reset result
```

| # | Action | Cơ chế / Mục đích | Output |
|---|---|---|---|
| 1 | Application chờ | Ngược với FW download: MCU là bên gửi. Application chờ dữ liệu đến. | Thread application ngủ. |
| 2–4 | Bắt đầu | Phân biệt random data / auth data, đọc size. Nhận random data thì báo application quá trình đã bắt đầu. | Buffer sẵn sàng. |
| 5 | READY | Báo MCU được phép gửi dữ liệu. | MCU bắt đầu truyền. |
| 6–8 | Nhận block | Đổi ASCII hex về byte, cộng checksum, xác nhận từng block. | Buffer được lấp dần. |
| 9–11 | Kiểm tra tổng | Checksum sai thì trả `0x7F`. | Xác nhận toàn vẹn dữ liệu. |
| 12–13 | Hoàn tất | Đánh thức application để copy dữ liệu ra user space. | Application có dữ liệu xác thực. |

**Các luồng nhiều bước khác cùng mô hình:**

- **NFC pairing:** giống firmware download nhưng dùng completion NFC riêng, tự retry thêm một lần nếu thất bại.
- **EEPROM read:** gửi lệnh đọc rồi chờ completion tối đa 1 giây.
- **Calibration:** application chờ từng điểm calibration qua `completion_recv_CalibrationPoint`.

---

## Luồng 8: Logging, xử lý lỗi và test

```mermaid
sequenceDiagram
    participant TX as TX RX Path
    participant LG as pnlLogRecord
    participant RB as Log Ring 64KB
    participant SC as AnalyzeSerialCommand
    participant DBG as Engineer
    participant PR as ProcFS
    participant IO as Ioctl PANEL_LOG
    participant IN as Input Path

    TX->>LG: 1. Moi byte TX hoac RX
    LG->>RB: 2. Tao header moi khi doi chieu, du 31 byte, hoac cach qua 200 tick
    LG->>RB: 3. Ghi byte, day kieu ghi de log cu
    SC->>SC: 4. NACK, dem loi theo tung bit
    SC->>SC: 5. Checksum sai, luu snapshot lan dau
    DBG->>IO: 6. ioctl PANEL_LOG
    IO-->>DBG: 7. Log touch IC, log TX RX, UART interrupt, wait status
    DBG->>PR: 8. cat info
    PR-->>DBG: 9. Snapshot checksum, so loi NACK, lock status
    DBG->>PR: 10. echo 1 vao test_flag bang root
    DBG->>PR: 11. echo keycode vao key_touch
    PR->>IN: 12. test_key_touch ghi thang vao KeyPanelBuffer
    DBG->>PR: 13. echo x y state vao panel_touch
    PR->>IN: 14. test_panel_touch goi InputMultitouchB
```

| # | Action | Cơ chế / Mục đích | Output |
|---|---|---|---|
| 1–3 | Ghi log | Gom byte thành bản ghi `TX/RX` kèm tick. Đầy 64KB thì ghi đè log cũ. | Có lịch sử giao tiếp gần nhất. |
| 4 | Đếm NACK | Mỗi bit trong mã lỗi ứng với một loại: overrun, overflow, parity, initial, checksum, length. | Thống kê lỗi theo loại. |
| 5 | Snapshot | Chỉ chụp lần checksum error đầu tiên để giữ đúng bằng chứng gốc. | Có dữ liệu điều tra D403. |
| 6–7 | `PANEL_LOG` | Dump tất cả log ra console/syslog. | Đối chiếu được TX/RX. |
| 8–9 | `/proc/.../info` | Xem snapshot, bộ đếm lỗi, trạng thái lock. | Báo cáo dạng text. |
| 10 | Bật test mode | Chỉ root được bật; điều kiện để dùng `key_touch`, `panel_touch`. | Test mode ON. |
| 11–12 | Giả lập phím | Ghi thẳng vào key buffer, **bỏ qua** kiểm tra autoplay và spinlock. | Application nhận phím giả lập. |
| 13–14 | Giả lập touch | Đi qua `InputMultitouchB` như touch thật. | Application nhận touch giả lập. |

> **Lưu ý:** Ngoài procfs còn có các `ioctl` `PSEUDO_INPUT_TOUCH`, `PSEUDO_INPUT_HARDKEY`, `SET_AUTOPLAY_STATE` cho test tự động. Trong code test qua procfs có hai điểm nên xem lại:
> - Khi test mode tắt, `proc_key_touch_write` trả `EINVAL` dương thay vì `-EINVAL`.
> - `sscanf(buf, "%d", &KeyCode)` ghi một `int` vào biến `unsigned char`, có thể ghi đè vùng nhớ stack lân cận.

---

## Tổng kết

| Luồng | Hàm trọng tâm | Cơ chế đồng bộ chính |
|---|---|---|
| 1. Install/Uninstall | `PanelComDrv_Install`, `PanelComDriverInitialize` | kthread, tasklet, wait queue |
| 2. TX | `RequestXxx`, `SetTxRingBuffer`, `ForwardSerialCommand` | `TxComSpinLock`, wake_up |
| 3. RX | `RxSerialCommunication`, `AnalyzeReceiveData`, `AnalyzeSerialCommand` | `sysBspPanelSpinLock`, tasklet |
| 4. Input | `AnalyzeTouchCommand`, `InputMultitouchB`, `InputHardkey` | `KeyComSpinLock`, poll wait queue, timer |
| 5. Device control | `RequestBackLightSetting`, `RequestBuzzerSetting` | completion ACK, buzzer timer |
| 6. Suspend/Resume | `SerialCommuniCationSuspend`, `SerialCommuniCationSuspendRecover` | `iIsSuspend`, completion suspend/ACK |
| 7. Multi-step | `downloadPanelFW`, `transNFCPairingData`, NFC auth handler | nhiều completion, stop-and-wait |
| 8. Log/Debug/Test | `pnlLogRecord`, `show_info`, `test_panel_touch` | ring buffer log, procfs |

**Hai đường ống cốt lõi cần nhớ:**

```text
TX: PanelComDrv_Ioctl → RequestXxx → SetTxRingBuffer → AnalyzeControlTask → UART
RX: UART → AnalyzeReceiveData → AnalyzeSerialCommand → xử lý sự kiện
```
