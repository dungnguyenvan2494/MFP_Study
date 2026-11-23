# Panel wake – Sequence diagrams (LPPP / R4)

Các sơ đồ dưới đây bám theo source trong `lppp_s800/`. Mermaid: mở bằng VS Code (Markdown Preview Mermaid), GitHub, hoặc mermaid.live.

| Pin | GPIO | Bảng cấu hình | ISR type | Vai trò |
|---|---|---|---|---|
| `LPPP_GPIOPIN_PANEL_STOP` | 49 | `gpio_info[]` | RISING_EDGE | Phím Stop. **Không** sinh sự kiện nguồn, chỉ được đọc (poll) |
| `LPPP_GPIOPIN_PANEL_SUBSW` | 50 | `wake_gpio_info[]` | RISING_EDGE | Sub power (comment code: 副電源OFF割り込み) |
| `LPPP_GPIOPIN_PANEL_WAKE` | 54 | `wake_gpio_info[]` | RISING_EDGE | Chạm touch panel (comment code: タッチパネル割り込み) |

Nguồn: `ApplicationKM/LPPP_Gpio.c` (bảng pin và `LPPP_GPIO_AnalyzeEventfromISR`), `ApplicationKM/LPPP_Gpio.h`.

---

## 1. Tổng quan: wake từ PANEL_SUBSW / PANEL_WAKE (đường chính)

Hai pin này nằm trong `wake_gpio_info[]`. Khi hệ thống suspend, `setup_wake_events()` được PMU gọi để arm lại ISR. Khi có cạnh lên, ISR đặt event `SYS_EVENT_WAKE_SYSTEM`. Event này được `SysSystem_handle_link_events()` xử lý bằng `power_cpu(0, 0)`.

```mermaid
sequenceDiagram
    autonumber
    actor User as Người dùng
    participant Panel as Operation Panel
    participant GPIO as GPIO block (ITER1/ITER2)
    participant PMU as PMU (pmu_pm_ops)
    participant GISR as gpio_isr (app_gpio.c)
    participant CB as LPPP_GPIO_Callback
    participant SysEvt as SysSystem_SysEventGroup
    participant SysThr as SysSystem_handle_link_events
    participant PWR as power_cpu(0,0)
    participant CA72 as CA72 (AP)

    Note over PMU,GPIO: Giai đoạn chuẩn bị suspend
    PMU->>GISR: gpio_ops.suspend = setup_wake_events()
    GISR->>GPIO: đọc ITER1 (wake events), gpio_open + gpio_isr_attach(gpio_isr) + gpio_isr_enable
    GISR->>GPIO: gpio_set_itern(ITER2 R4) |= cur_wake_events
    Note over CA72: Hệ thống ở trạng thái SLEEP (pmu_suspend_active = SLEEP)

    User->>Panel: Chạm màn hình / nhấn Sub Power
    Panel->>GPIO: PANEL_WAKE (GPIO54) / PANEL_SUBSW (GPIO50) lên cao (rising edge)
    GPIO->>GISR: ngắt tới R4 qua ITER2
    GISR->>GPIO: gỡ bit khỏi ITER2, cur_wake_events = 0
    GISR->>SysEvt: EventFlagsSet(SYS_EVENT_WAKE_SYSTEM)
    GISR->>CB: LPPP_GPIO_Callback(pin, level, &pinno)
    CB->>CB: LPPP_GPIO_AnalyzeEventfromISR() = LPPP_INTERNALEVT_WAKEUP_S1
    CB-->>GISR: return false (không đẩy vào queue PowerCtrl)
    Note right of CB: "Wake có nguồn ngoài GPIO, nên xử lý trong power_cpu()"
    SysEvt->>SysThr: Events & SYS_EVENT_WAKE_SYSTEM
    SysThr->>PWR: power_cpu(0, 0)
    PWR->>CA72: Cấp nguồn / bật lại CA72 (resume)
    Note over PMU,GPIO: PMU resume = gpio_wake_close(): đóng pin, dọn handle
```

Điểm cần lưu ý (so với bản phân tích trước):
- Callback `LPPP_GPIO_Callback` **có** chạy trên đường wake, nhưng chỉ để map ra `WAKEUP_S1` rồi `return false`. Việc wake thực sự do `SYS_EVENT_WAKE_SYSTEM` -> `power_cpu(0,0)`.
- `LPPP_GPIO_CallbackDummy` chỉ dùng ở lúc khởi tạo (`gpio_isr_attach` ở `LPPP_Gpio.c` ~dòng 603) để cấu hình phần cứng. ISR thật lúc suspend là `gpio_isr`.

---

## 2. Chi tiết giai đoạn arm wake khi suspend + workaround panel (OP_BTS-37711)

Nếu `PANEL_WAKE` đang ở mức cao ngay lúc chuẩn bị suspend (người dùng chạm panel giữa chừng khi chuyển DeepSleep), cạnh lên đã xảy ra trước khi ISR được arm nên sẽ bị mất. Code xử lý bằng cách pulse `POWER_OFF_PANEL` (GPIOC15) để panel tạo lại cạnh.

```mermaid
sequenceDiagram
    autonumber
    participant PMU as PMU suspend path
    participant SWE as setup_wake_events()
    participant GPIO as GPIO bank
    participant PNL as POWER_OFF_PANEL (GPIOC15)

    PMU->>SWE: suspend callback
    SWE->>SWE: LPPP_GPIO_HandleMainSW(false)
    loop mỗi GPIO register group
        SWE->>GPIO: wake_events = ITER1 (GPIOB: thêm WAKE_WLAN, WAKE_UP bit19, WAKE_FIERY bit20)
        loop mỗi bit wake_event
            SWE->>GPIO: gpio_open + gpio_isr_attach(gpio_isr, isr_type từ wake_gpio_info) + gpio_isr_enable
        end
        SWE->>GPIO: ITER2 |= cur_wake_events
    end
    SWE->>GPIO: gpio_read(temp_storage[4].pin) = mức PANEL_WAKE
    alt PANEL_WAKE == 1 (đã bị chạm trong lúc vào DeepSleep)
        SWE->>PNL: gpio_open(output, level=1)
        SWE->>SWE: cpu_spin_delay(25 ms)
        SWE->>PNL: gpio_write(0), gpio_close
        Note over PNL: Panel đánh lại cạnh, wake không bị mất
    else mức thấp
        Note over SWE: Không cần xử lý thêm
    end
    SWE-->>PMU: return have_events
```

Ghi chú: code lấy `temp_storage[4]` làm pin panel wake, tức là dựa vào **thứ tự** bit được thêm vào chứ không tra theo tên pin. Nếu thứ tự đổi (thêm/bớt wake pin) thì đọc nhầm pin. Đây là điểm dễ vỡ, nên kiểm lại nếu sửa `wake_gpio_info[]`.

---

## 3. Wake bị trì hoãn khi đang `goodbye_processing`

`SysSystem_handle_link_events()` không gọi `power_cpu` ngay nếu hệ thống đang trong quá trình goodbye (vào sleep). Wake được giữ lại (`wake_request`) và thực hiện khi có `SYS_EVENT_CAN_WAKE` (phát ra ở cuối đường suspend trong `power.c`).

```mermaid
sequenceDiagram
    autonumber
    actor User as Người dùng
    participant GISR as gpio_isr
    participant Evt as SysSystem_SysEventGroup
    participant Thr as SysSystem_handle_link_events
    participant Pwr as power.c (suspend path)
    participant PC as power_cpu(0,0)

    User->>GISR: Chạm panel (PANEL_WAKE) khi đang goodbye
    GISR->>Evt: SYS_EVENT_WAKE_SYSTEM
    Evt->>Thr: Events & WAKE_SYSTEM
    alt goodbye_processing == false
        Thr->>PC: power_cpu(0,0) ngay
    else goodbye_processing == true
        Thr->>Thr: wake_request = true, log "wake delay"
        Pwr->>Pwr: hoàn tất suspend, pmu_suspend_active = SLEEP
        Pwr->>Evt: SYS_EVENT_CAN_WAKE
        Evt->>Thr: Events & CAN_WAKE
        Thr->>Thr: goodbye_processing = false
        Thr->>PC: wake_request == true: power_cpu(0,0), log "wake start"
        Thr->>Thr: wake_request = false
    end
```

Nguồn: `Sys/system/sys_system_thread.c` (~dòng 307-327), `Hal/quartz/asic/build/power.c` (~dòng 1599).

---

## 4. PANEL_STOP: không phải nguồn wake, dùng để quyết định MOTION_EN sau khi bật nguồn

`PANEL_STOP` nằm trong `gpio_info[]` (cấu hình ngắt thường), không nằm trong `wake_gpio_info[]`. `LPPP_GPIO_AnalyzeEventfromISR()` không có `case` cho pin này, nên ngắt rơi vào `default` ("no event", `intr_event = INVALID`, callback `return false`). Pin được đọc trực tiếp ở các chỗ cần dùng.

```mermaid
sequenceDiagram
    autonumber
    actor User as Người dùng
    participant Panel as Operation Panel
    participant GPIO as GPIO 49 (PANEL_STOP)
    participant CB as LPPP_GPIO_Callback
    participant Mat as LPPP_Matrix (power-on sequence)
    participant Rd as LPPP_GPIO read (flag PANEL_STOP 0x0001)
    participant MOT as MOTION_EN_MC / MOTION_EN_IR

    rect rgb(245,245,245)
    Note over User,CB: (a) Ngắt khi nhấn Stop
    User->>Panel: Nhấn Stop
    Panel->>GPIO: rising edge
    GPIO->>CB: LPPP_GPIO_Callback
    CB->>CB: AnalyzeEventfromISR(): default -> "no event"
    CB-->>GPIO: return false (không có sự kiện nguồn)
    end

    rect rgb(245,245,245)
    Note over Mat,MOT: (b) Dùng trong luồng bật nguồn sau wake
    Mat->>Mat: GPIO_Write(AP_PWR_EN, 1), chờ AP_PG
    Mat->>Mat: chờ panel microcomputer khởi động (>= 100 ms sau SB_PWR_EN H)
    Mat->>GPIO: LPPP_GPIO_Read(PANEL_STOP)
    alt == 0 (Stop không bị nhấn)
        Mat->>MOT: GPIO_Write(MOTION_EN_MC, 0), GPIO_Write(MOTION_EN_IR, 0)
    else == 1 (Stop đang nhấn)
        Note over MOT: Giữ nguyên, không đổi MOTION_EN
    end
    end

    rect rgb(245,245,245)
    Note over Rd: (c) Đọc trạng thái dạng flag
    Rd->>GPIO: LPPP_GPIO_Read(PANEL_STOP)
    Rd->>Rd: HIGH: readval |= LPPP_GPIOFLAG_PANEL_STOP, LOW: readval &= ~flag
    end
```

Ghi chú: các comment tiếng Nhật trong `LPPP_Matrix.c` bị lỗi mã hoá khi đọc (file không phải UTF-8), nên nhánh `else` ở (b) và logic mức tích cực của `MOTION_EN_*` chỉ suy ra từ mã. Cần xác nhận lại với comment gốc nếu dùng làm tài liệu chính thức.

---

## 5. Tổng hợp các nguồn wake và phân biệt với WAKEUP (RTC) của PS-CPU

```mermaid
sequenceDiagram
    autonumber
    participant Pnl as Panel (SUBSW / WAKE)
    participant RTC as RTC alarm (WAKE_UP GPIO, bit19 GPIOB)
    participant Other as FIERY / USB_VBUS / DOOR / FAX / MODEM / EDH...
    participant LAN as WoL (SYS_EVENT_WOL_DETECTED)
    participant GISR as gpio_isr
    participant Evt as SYS_EVENT_WAKE_SYSTEM
    participant PC as power_cpu(0,0)

    Pnl->>GISR: rising edge
    RTC->>GISR: level high
    Other->>GISR: edge / level
    GISR->>Evt: set
    LAN->>Evt: set (sys_system_thread.c ~dòng 419)
    Evt->>PC: SysSystem_handle_link_events()
    Note over GISR,PC: Tất cả cùng map về LPPP_INTERNALEVT_WAKEUP_S1 trong AnalyzeEventfromISR, cùng một đường wake. Panel không có đường riêng.
```

`WAKEUP` của PS-CPU (RTC alarm gửi tới AP) được code LPPP thấy ở pin `LPPP_GPIOPIN_WAKE_UP` (level high) và đi chung đường trên. Nó không phải tín hiệu chạm panel.

---

## Cách đọc

- Sơ đồ 1 là luồng chính từ lúc chạm panel đến lúc CA72 chạy lại. Đọc trước.
- Sơ đồ 2 và 3 là hai chỗ có xử lý đặc biệt (mất cạnh khi đang vào DeepSleep, và wake bị hoãn khi đang goodbye).
- Sơ đồ 4 giải thích vì sao `PANEL_STOP` không xuất hiện trong luồng wake.
- Sơ đồ 5 cho thấy mọi nguồn wake dùng chung một đường nên không cần vẽ riêng cho từng loại.
