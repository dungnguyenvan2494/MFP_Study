# Sequence diagram: mdels800_pmu.c (PMU driver)

Tài liệu mô tả các luồng xử lý chính của driver PMU phía kernel (`mdels800_pmu.c`) và cách nó tương tác với `THRC_PowerManager.cpp`.

## 0. Các thành phần tham gia

| Participant | Vai trò |
|---|---|
| **APP** (`THRC_PowerManager`) | Userland gọi `/dev/PMU0` (ioctl) và sysfs `/sys/.../PMU.0/devices/*` |
| **PMU** (`mdels800_pmu.c`) | Driver kernel |
| **WQ** | Workqueue (`BspPmuIsrWorkCb`, `BspPmuIntp4WorkCb`, `BspPmuMainSwWorkCb`) |
| **PSCPU** | Power Sub-CPU, giao tiếp qua I2C (`SET/CLR/GET_PSCPU`) |
| **LPPP** | Core quản lý nguồn, giao tiếp qua SCPI (`get_km_scpi_ops()`) |
| **GPIO/HW** | Các chân wake (INTP4–7, KR0–6, MAINSW, POWER_MONITER) |

## 1. Khởi tạo driver

```mermaid
sequenceDiagram
    autonumber
    participant K as Kernel init
    participant PMU as mdels800_pmu
    participant DTS as Device Tree
    participant HW as GPIO / IRQ
    participant PSCPU as PS-CPU (I2C)

    K->>PMU: arch_initcall(PMU_install)
    PMU->>PMU: alloc_chrdev_region, cdev_add (/dev/PMU0)
    PMU->>PMU: platform_driver_register (pm_ops: suspend/resume/freeze/thaw/restore)
    PMU->>PMU: PMU_sysfs_add (group "devices", request_reboot, powerdown_debug)
    PMU->>PMU: Khởi tạo spinlock, semaphore (timePowerDown, wlan_reset, qspi_write)

    K->>PMU: module_init(PMU_install_int)
    PMU->>DTS: of_find_node_by_type("power management unit")
    DTS-->>PMU: Số GPIO (sub_power, intp4..7, kr0..6, msw_on, power_monitor...)
    PMU->>HW: ioremap GPIOB_ISR, WLAN_SUSPEND_FLAG, GPIOB_ITER2
    PMU->>PMU: BspPmuWorkInit (gắn work callback cho từng IRQ)
    PMU->>HW: BspPmuIsrInit: request_irq SUBSW, INTP4..7, KR1..3, KR5, KR6, MAINSW
    PMU->>HW: BspPmuSetLpppMainSwIrqEnable(false) (OP_BTS-65250)
    PMU->>HW: request_irq POWER_MONITER → BspPowerDownIsr (không qua WQ)
    PMU->>HW: BspPmuDisableIrq (SUB_POWER, INTP4, INTP7), tắt KR6
    PMU->>HW: request_irq MC_WDT → Bsp_MCWDT_Isr
    PMU->>PSCPU: i2c_add_driver → BspI2cPmuProbe (đọc version, checksum)
    PMU->>K: pm_notifier(BspPmuPmCallback)
```

## 2. Đăng ký và truyền event: interrupt → signal → APP

```mermaid
sequenceDiagram
    autonumber
    participant APP as THRC_PowerManager
    participant PMU as PMU_ioctl / pmu_tx_sig
    participant HW as GPIO wake pin
    participant ISR as BspPmuIsr
    participant WQ as BspPmuIsrWorkCb

    APP->>PMU: ioctl PMU_IOCSPIDREGIST(pid, SIGRTMIN+6)
    PMU->>PMU: Lưu reginfo (pid, signo)

    HW->>ISR: Interrupt (INTP5..7, KR1..6)
    ISR->>WQ: schedule_work
    opt mask = INTRTC
        WQ->>WQ: pscpu_read_alarm (clear RTC alarm)
    end
    opt mask = KR6 và đang suspend
        WQ->>WQ: gp_vbus_callback (báo USB driver)
    end
    alt gl_ResumeFlag=1 và gul_PwrsubInt=0 (wake interrupt đang bị chặn)
        WQ->>WQ: gst_PmuData.interrupts |= mask (giữ lại)
    else
        WQ->>PMU: pmu_tx_sig(info, STATE_CHANGED, type)
        PMU->>PMU: Lọc lặp event 0x1267 (Fiery)
        PMU->>APP: send_sig_info(si_code)
        APP->>APP: sig_handler → pmuEvent.add → SendPmuEvent
    end

    APP->>PMU: ioctl PMU_IOCSPWRSUBINT(1)
    PMU->>PMU: gul_PwrsubInt=1, BspPmuSendInterrupt(MAINSW) gửi event đã giữ
```

## 3. Power key (INTP4): phân biệt nhấn ngắn và nhấn dài

```mermaid
sequenceDiagram
    autonumber
    participant HW as INTP4 / SUBSW
    participant WQ as BspPmuIntp4WorkCb
    participant TMR as PMU_Timer_Handler (5ms)
    participant APP as THRC_PowerManager

    HW->>WQ: Interrupt (schedule_work)
    alt Đang chạy (gl_ResumeFlag=1)
        alt gul_PwrsubInt=0
            WQ->>WQ: interrupts |= INTP4 (giữ lại)
        else level=1
            WQ->>WQ: PMU_SubpowerswOnInitialize(ON_NOISECHECK)
            WQ->>APP: pmu_tx_sig(0x03)
        end
    else Đang suspend
        WQ->>WQ: gul_SubSwInt=1, PMU_SubpowerswOnInitialize(ON)
        WQ->>APP: pmu_tx_sig(0x62) (wake bằng power key)
    end
    WQ->>TMR: PMU_Timer_Register (5ms)
    loop Mỗi 5ms khi state ≠ OFF
        TMR->>TMR: Đọc INTP4: NOISECHECK → ON → OFF_NOISECHECK
        alt Giữ ≥ 2s
            TMR->>APP: pmu_tx_sig(PSKEY_LONG) → 0x1307
        else Nhả sau khi qua lọc nhiễu 20ms
            TMR->>APP: pmu_tx_sig(PSKEY_SHORT) → 0x1306
        end
        TMR->>TMR: Vào PROHIBITION 1s rồi về OFF
    end
    Note over TMR,APP: Không gửi signal khi panel bị lock. Nếu đang nối Android thì sendMessageDrv
```

## 4. Main SW OFF và đường tắt nguồn

```mermaid
sequenceDiagram
    autonumber
    participant HW as MAINSW GPIO
    participant WQ as BspPmuMainSwWorkCb
    participant PSCPU as PS-CPU
    participant APP as THRC_PowerManager
    participant FS as Flash / NVMe

    HW->>WQ: Interrupt (MAINSW, active low)
    WQ->>WQ: mdelay 10ms, đọc lại (chống nhiễu)
    alt bl_pmu_PowerOffDelay=false hoặc MAINSW không bị mask
        WQ->>WQ: PMU_Request_PowerOff (xem bên dưới)
    else Chế độ chờ snapshot (PowerOffDelay=1)
        WQ->>PSCPU: BspPowerOffTimerReset (2000ms hoặc 5000ms)
        WQ->>WQ: BspPowerOffFailSafe_Start (timer - 200ms)
        WQ->>FS: THRF_writeFlash (PowerDown data), lưu RTC FAX backup (0x4F)
        WQ->>WQ: reset_suspend_count
        alt gul_PwrsubInt=1
            WQ->>APP: pmu_tx_sig(0x6A) → 0x126A MainSW OFF
        else
            WQ->>WQ: interrupts |= MAINSW (gửi sau khi bật lại ngắt)
        end
    end

    loop APP cập nhật timer định kỳ
        APP->>WQ: sysfs PowerOffTimer = n
        WQ->>WQ: BspPowerOffFailSafe_Stop
        WQ->>PSCPU: I2C SET 0xB0 (PWROFF_TIM_SET)
    end

    APP->>WQ: sysfs PowerOffDelay = 0 (snapshot xong hoặc lỗi)
    WQ->>PSCPU: PMU_Request_PowerOff: TimerReset 5000
    WQ->>FS: ksys_sync, km_nvme_shutdown
    WQ->>HW: /HRESET_REQ = 0 (cắt nguồn)
    Note over WQ: Nếu APP treo thì failsafe timer hết hạn và tự gọi PMU_Request_PowerOff
```

## 5. Suspend / Resume (STR) và Warp

```mermaid
sequenceDiagram
    autonumber
    participant APP as req_DeepSleepShift
    participant PM as Linux PM core
    participant PMU as PMU_suspend / PMU_resume
    participant LPPP as LPPP (SCPI)
    participant HW as GPIOB_ISR / WLAN flag

    APP->>PMU: ioctl IOCSSUBSWINT(0), IOCSPWRSUBINT(0)
    APP->>PMU: ioctl IOCSSET_STR_STATUS(1)
    APP->>PM: exec_str()
    PM->>PMU: PMU_suspend
    PMU->>PMU: gl_ResumeFlag=0, gul_PwrsubInt=0, dừng timer power key
    opt bl_Wdt_ca72_DeepLeepFlg
        PMU->>LPPP: set_wdt_ca72(END)
    end
    PMU->>HW: Cho LPPP nhận ngắt MAINSW (ITER2 bit30 = 1)
    Note over PM,LPPP: CPU ngủ. LPPP và PS-CPU giám sát wake source

    LPPP-->>PM: Wake
    PM->>PMU: PMU_resume
    opt bl_Wdt_ca72_DeepLeepFlg
        PMU->>LPPP: set_wdt_ca72(START)
    end
    PMU->>HW: Đọc GPIOB_ISR, WLAN flag
    alt bit19 WAKE_UP (RTC)
        PMU->>APP: pmu_tx_sig(0x63)
    end
    alt bit20 WAKE_FIERY
        PMU->>APP: pmu_tx_sig(0x67)
    end
    alt bit26 WAKE_WLAN và cờ WLAN suspend
        PMU->>APP: pmu_tx_sig(0x64)
    end
    PMU->>HW: Tắt ngắt MAINSW phía LPPP (ITER2 bit30 = 0)
    PMU->>PMU: gl_ResumeFlag=1, gul_PwrsubInt=1, gul_SubSwInt=1
    PMU->>APP: BspPmuSendInterrupt(MAINSW) (nếu đã giữ)
    PM-->>APP: exec_str() trả về

    APP->>PMU: ioctl IOCSGETWAKEINFO
    PMU->>LPPP: scpi->get_wake_info
    LPPP-->>APP: flags (USB / PROXY / SNMP2 / WOL)
    APP->>PMU: ioctl IOCSREQESTFACTOR, IOCSSUBSWINT(1)

    Note over PM,PMU: Warp: freeze/thaw/restore chỉ bật hoặc tắt ngắt MAINSW phía LPPP. PM_POST_HIBERNATION reset bl_pmu_PowerOffDelay=false
```

## 6. USB resume và WLAN reset

```mermaid
sequenceDiagram
    autonumber
    participant USBD as USB driver (kernel)
    participant WLD as WLAN driver
    participant PMU as mdels800_pmu
    participant RT as TASK_WakeupUSB (resumeAfterDeepSleep)
    participant WR as TASK_WLAN_Reset (wlan_reset_Loop)

    WR->>PMU: ioctl IOCSGET_WLANRESET (block)
    PMU->>PMU: wait_for_completion(completion_recv__WlanReset)

    RT->>PMU: ioctl IOCSSET_REQ_WLANRESET(1) → down(sem_wlan_reset)
    USBD->>PMU: pmuSetUsbResumeFin / pmuSetUsbResumeError
    RT->>PMU: ioctl IOCSSET_RESET_USB_VAR
    PMU-->>RT: bit0 = resume xong, bit1 = lỗi TRB (đọc xong thì clear)
    opt Lỗi resume
        RT->>RT: rmmod/insmod USB, waitUsbResumeWlanReady
        RT->>PMU: ioctl IOCSSET_APP_REQ_WLANRESET(2)
    end

    WLD->>PMU: set_WlanReset_ErrorNotify(1|2|3)
    opt flag = 3
        PMU->>RT: pmu_tx_sig(0x05) (yêu cầu wake)
    end
    opt b_usb_sleep=true và b_str_status=false
        PMU->>PMU: complete(completion_recv__WlanReset)
    end
    RT->>PMU: ioctl IOCSSET_REQ_WLANRESET(2) → up(sem_wlan_reset)
    PMU-->>WR: wlan_reset_usbport_flg (chạy sau khi giải phóng sem)
    WR->>WR: unbind/bind USB, mioWlanReset
```

## 7. Mất điện tức thời (POWER_MONITER) và reboot

```mermaid
sequenceDiagram
    autonumber
    participant HW as POWER_MONITER
    participant ISR as BspPowerDownIsr (hard IRQ)
    participant QSPI as QSPI Flash
    participant WQ as PMU_Reboot_Flicker_work
    participant APP as THRC_PowerManager

    HW->>ISR: Nguồn xuống (falling)
    ISR->>ISR: local_irq_disable, kiểm tra 10 lần × 10µs
    alt Phục hồi (nhiễu)
        ISR-->>HW: return
    else Mất điện thật
        ISR->>ISR: smp_send_stop_powerdown, irq_disable_flg=1
        ISR->>QSPI: qspi_powerdown_write (PowerDown data, RTC FAX backup)
        ISR->>ISR: Chờ nguồn ổn định lại
        ISR->>QSPI: Ghi cờ mất điện (BSPTHR1/2)
        ISR->>WQ: PMU_Reboot_Flicker (schedule_work)
        WQ->>WQ: km_nvme_shutdown, /HRESET_REQ=0, kernel_restart
    end

    Note over APP,WQ: Các đường reboot khác
    APP->>WQ: ioctl IOCSGET_WD_EXPIRE → set_newdetailcode, sleep 10s, PMU_Reboot
    Note over WQ: Lỗi I2C quá số lần retry thì set_PmuRtcNewdetailCode (0x2201F...) rồi PMU_Reboot
    Note over WQ: Kernel lỗi thì set_newdetailcode_sig gửi TM_NewDetailCode lên APP qua signal
```

## Ánh xạ nhanh sang `THRC_PowerManager.cpp`

| Code phía APP | Phía driver |
|---|---|
| `THRD_PMU_IOCSPIDREGIST` | Lưu `reginfo`, đích của `pmu_tx_sig` |
| `THRD_PMU_IOCSPWRSUBINT` / `IOCSSUBSWINT` | Cờ `gul_PwrsubInt` / `gul_SubSwInt`, dùng để giữ hoặc gửi event |
| `SwitchPowerState(SET_KRx/INTPx)` | sysfs `SET_KRx` → `enable_irq` / `disable_irq` |
| `SwitchPowerState(ENG_POWER, MC_PWR_EN, IR_RY...)` | `BspPsCpuStore` → I2C `SET/CLR_PSCPU` |
| `THRF_setPowerOffDelay` / `THRF_setPowerOffTimer` | sysfs `PowerOffDelay` / `PowerOffTimer` |
| `ca72wdt_ext_refresh` | `IOCSSET_WDT_CA72_EXT` → `scpi->set_wdt_ca72_ext` |
| `getNewDetailCode` | sysfs `NEW_DETAIL_CODE(_ISREBOOT)` |
| B013 / B014 | sysfs → `mtd_read` / `Bsp_mtd_write` (QSPI) |

# powerSaveKey_info

powerSaveKey_info là state machine của phím tiết kiệm điện (副電源SW, INTP4) trong mdels800_pmu. Nó dùng để chống nhiễu (debounce), phân biệt nhấn ngắn với nhấn dài, và bỏ qua các lần nhấn liên tiếp. Driver chỉ báo lên APP một event cho mỗi lần nhấn, không báo theo từng cạnh ngắt.

## Các field

| Field | Ý nghĩa | Reset khi |
|---|---|---|
| state | Trạng thái phím: OFF, ON_NOISECHECK, ON, OFF_NOISECHECK, PROHIBITION | Về OFF |
| cnt | Số tick kể từ lúc bắt đầu nhấn, dùng để phát hiện nhấn dài | Init / phát hiện xong |
| cnt_nc | Số tick trong giai đoạn lọc nhiễu | Mỗi lần đổi pha nhấn/nhả |
| cnt_irq | Số tick trong vùng cấm (PROHIBITION) | Hết vùng cấm |

Các ngưỡng tính theo tick 5ms (timeout = 5):

SUBSWITCH_NOISECHECK = 20/5: lọc nhiễu 20ms
SUBSWITCH_THRESHOLD = 2000/5: từ 2s trở lên là nhấn dài
SUBSWITCH_PROHIBITED_SECTION = 1000/5: vùng cấm 1s sau khi đã báo event

## State diagram

```mermaid
stateDiagram-v2
    [*] --> OFF
    OFF --> ON_NOISECHECK: INTP4=1 lúc đang chạy<br/>(gửi 0x03)
    OFF --> ON: INTP4=1 lúc suspend<br/>(gửi 0x62, bỏ qua lọc nhiễu)
    ON_NOISECHECK --> OFF: sau 20ms mà INTP4=0 (nhiễu)
    ON_NOISECHECK --> ON: sau 20ms mà INTP4 vẫn =1
    ON --> OFF_NOISECHECK: INTP4=0 (nhả)
    ON --> PROHIBITION: cnt >= 2s → PSKEY_LONG
    OFF_NOISECHECK --> ON: nhấn lại sau 20ms (dội phím)
    OFF_NOISECHECK --> PROHIBITION: cnt >= 2s và đang nhấn → PSKEY_LONG
    OFF_NOISECHECK --> PROHIBITION: nhả ổn định 20ms → PSKEY_SHORT
    PROHIBITION --> OFF: sau 1s
```

## Sequence
```mermaid
sequenceDiagram
    autonumber
    actor U as Người dùng
    participant HW as INTP4 (GPIO)
    participant WQ as BspPmuIntp4WorkCb
    participant INI as PMU_SubpowerswOnInitialize
    participant TMR as PMU_Timer_Handler (5ms)
    participant APP as THRC_PowerManager

    U->>HW: Nhấn phím
    HW->>WQ: IRQ, BspPmuIsr gọi schedule_work

    alt Đang chạy (gl_ResumeFlag=1)
        alt gul_PwrsubInt=0 (APP đang chặn ngắt)
            WQ->>WQ: interrupts |= INTP4, return<br/>(không chạy state machine)
        else INTP4=1
            WQ->>INI: goState = ON_NOISECHECK
        end
    else Đang suspend (gl_ResumeFlag=0)
        WQ->>WQ: gul_SubSwInt = 1
        WQ->>INI: goState = ON
    end

    alt state != OFF (đang xử lý lần nhấn trước hoặc trong vùng cấm)
        INI-->>WQ: Không làm gì (bỏ qua lần nhấn này)
    else state = OFF
        INI->>APP: pmu_tx_sig 0x03 (GPIO) khi chạy, hoặc 0x62 (IRQ) khi suspend
        INI->>INI: state = goState, cnt = 0, cnt_nc = 0
        INI->>TMR: PMU_Timer_Register (5ms)
    end

    loop Mỗi tick khi state != OFF và gul_SubSwInt = 1
        TMR->>TMR: cnt++, cnt_nc++, cnt_irq++, đọc INTP4
        alt ON_NOISECHECK và cnt_nc >= 20ms
            TMR->>TMR: INTP4=0 thì về OFF (nhiễu), =1 thì sang ON
        else ON
            TMR->>TMR: INTP4=0 thì sang OFF_NOISECHECK (cnt_nc=0)
            opt cnt >= 2s
                TMR->>TMR: notifyEvent = PSKEY_LONG, sang PROHIBITION
            end
        else OFF_NOISECHECK
            alt INTP4=1 và cnt >= 2s
                TMR->>TMR: notifyEvent = PSKEY_LONG
            else INTP4=1 và cnt_nc >= 20ms
                TMR->>TMR: Quay lại ON (dội phím)
            else INTP4=0 ổn định 20ms
                TMR->>TMR: notifyEvent = PSKEY_SHORT, sang PROHIBITION
            end
        else PROHIBITION và cnt_irq >= 1s
            TMR->>TMR: state = OFF, reset các counter
        end

        opt Có notifyEvent
            alt Đang nối Android (PanelCom_GetAndroidStatus)
                TMR->>APP: sendMessageDrv(TID_AndroidControl, TabletMainPanelTouchNotice)
            else Panel không bị lock
                TMR->>APP: pmu_tx_sig(PSKEY_SHORT hoặc PSKEY_LONG, ETC)
            else Display hoặc panel đang lock
                TMR->>TMR: Không báo
            end
        end
        opt state != OFF
            TMR->>TMR: PMU_Timer_Register (tick tiếp theo)
        end
    end

    Note over WQ,TMR: Reset state machine về OFF khi:<br/>ioctl IOCSSUBSWINT(0 hoặc 1), PMU_suspend (thêm PMU_Timer_Del), PMU_uninstall
```