## 1. PS-CPU có phải là bộ phận khởi động đầu tiên không?
**Đúng.** Nhìn vào kiến trúc mã nguồn thì PS-CPU (STM32F0xx, thư mục [PS-CPU/pscpu_s800](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800)) là một vi điều khiển **giám sát nguồn (Power Sub-CPU)** độc lập, luôn có điện trước tiên và tự chạy firmware của chính nó ngay khi có điện — nó **không phụ thuộc** vào SoC chính S800 (Marvell AP806, chứa cụm ARM Cortex‑A72 "CA72").

Bằng chứng rõ nhất là trong vòng lặp chính, chính PS-CPU là bên **quyết định khi nào S800 được cấp nguồn và được thoát reset**:
- [entry.c:74-75](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/entry.c#L74-L75): PS-CPU chủ động giữ chân `_RESET` ở mức thấp (giữ S800 trong reset) rồi gọi `s800_power_on()`.

```c
BSP_GPIO_WritePin(_RESET_GPIO_Port,_RESET_Pin,GPIO_PIN_RESET);      /* S800リセット */
s800_power_on();                                                /* S800電源ON */
```

- [km_extend_io.c:618-620](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L618-L620): PS-CPU bật `SB_PWR_EN` (nguồn cấp cho board S800) rồi mới khởi động timer 100 ms để **sau đó** mới nhả `_RESET`.

```c
/* S800 power on */
BSP_GPIO_WritePin(SB_PWR_EN_GPIO_Port,SB_PWR_EN_Pin,GPIO_PIN_SET);
/* S800 reset release*/
km_timer_set(TYPE_Km_Timer_SB_RESET_RELEASE,100,TYPE_Km_Timer_Start);   /* 100ms wait */
```


- [km_extend_io.c:1238](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1238) (`sb_reset_proc`): chỉ khi nguồn SB tốt (`SB_PG`), 24V ổn định, công tắc `MSW_ON` bật, và timer 100 ms hết hạn thì PS-CPU mới nhả `_RESET` — lúc này AP806/S800 mới thực sự bắt đầu chạy BootROM của chính nó.

```c
BSP_GPIO_WritePin(_RESET_GPIO_Port,_RESET_Pin,GPIO_PIN_SET);
km_timer_set(TYPE_Km_Timer_SB_RESET_RELEASE,0,TYPE_Km_Timer_Normal);	/* フラグ初期化 */
//2019/10/08 Sky 【自動測定】起動イベントの通知 start.
/* LPPP起動 開始を通知 */
setEventRecord(THRD_EventRecord_LPPPStart);
```

Nói cách khác: S800/AP806 (và sau đó là Linux kernel trong [Kernel/K-S800](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800)) **không có điện, không thoát reset** cho tới khi PS-CPU tự khởi động xong và chủ động cấp nguồn/nhả reset cho nó. File [ap806.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/ap806.c) bạn đang mở là bootloader chạy **trên/cho AP806** ở giai đoạn sau (nạp DDR + ATF), tức là một tầng khởi động hoàn toàn khác, xảy ra **sau** khi PS-CPU đã cấp nguồn.

## 2. Luồng khởi động của PS-CPU

**Sơ đồ tổng quát:**

```
Cấp điện phần cứng
   │
   ▼
Reset vector (startup_stm32f0xx.s) → SystemInit → main() [entry.c]
   │
   ▼
MX_Init(): Clock / GPIO / I2C1 / RTC / TIM3 / IWDG
   │
   ▼
Đọc model board, tính checksum FW, lấy version string
   │
   ▼
Start Watchdog (IWDG) + init trạng thái nội bộ + timer 1ms
   │
   ▼
S800 power-on sequence (giữ RESET, bật SB_PWR_EN)
   │
   ▼
Vòng lặp vô hạn (state machine, chạy theo tick 1ms + ngắt EXTI)
   │
   ├─ Nhả /RESET cho S800 khi điều kiện an toàn thỏa (sb_reset_proc)
   ├─ Bật MC_P_ON / IR_P_ON khi AP_PWR_EN từ CA72 xác nhận (mc_p_on_proc)
   ├─ Theo dõi công tắc nguồn, mất điện thoáng qua, 24V xả
   ├─ Đồng bộ trạng thái Sleep/ErP với host qua SLEEP_STATUS_REM
   ├─ Xử lý I2C (giao tiếp với S800/Linux sau khi nó đã boot)
   └─ Sleep mode giữa các chu kỳ để tiết kiệm điện

```

### Chi tiết từng bước (theo [entry.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/entry.c))

**a) Khởi tạo phần cứng MCU** — [mx_init.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Src/mx_init.c) (`MX_Init`, dòng 81):

- `HAL_Init()` + `SystemClock_Config()`: dùng HSI nội (16 MHz) qua PLL x12 → 48 MHz làm SYSCLK; I2C1 clock lấy từ HSI; RTC clock lấy từ LSE (thạch anh 32.768 kHz — để giữ giờ ngay cả khi mất nguồn chính, có pin backup).
- `MX_GPIO_Init()`: cấu hình toàn bộ chân điều khiển nguồn (`SB_PWR_EN`, `MC_P_ON`, `IR_P_ON`, `_RESET`, `MC_PWR_EN`, `ERP_SENSOR_ON`, `_RST_SLP2`, `_USB2_OE`...) là output và **mặc định RESET (0)**; cấu hình các chân ngắt vào (`_HRESET_REQ`, `AP_PWR_EN`, `POWER_MONITOR`, `MSW_ON`, `MONI_24V11`) là input EXTI.
- `MX_I2C1_Init()`: PS-CPU đóng vai trò **I2C slave** (địa chỉ 122/124) — đây là kênh giao tiếp với board chính (LPPP/host) để đọc/ghi trạng thái I/O mở rộng, checksum, version, RTC... (phía Linux có driver tương ứng ở [rtc-pscpu.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/drivers/rtc/rtc-pscpu.c)).
- RTC init + `KM_RTC_Restore()`: khôi phục giờ đã lưu backup register nếu có.
- `MX_TIM3_Init()`: tạo tick 1ms dùng cho toàn bộ software timer nội bộ (`km_timer_*`).
- `MX_IWDG_Init()`: cấu hình Independent Watchdog (prescaler 256) — sẽ được arm ở bước sau.

**b) Nhận diện board và firmware** — [entry.c:53-60](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/entry.c#L53-L60):
- `get_model()`: đọc chân `MODEL_BIT0/MODEL_BIT1` để biết đây là dòng máy nào (Sparrow / Eagle...).
- `checksum_calc()` + `GetVersionString()`: tính checksum và chuẩn bị chuỗi version để host đọc qua I2C, dùng để xác minh tính toàn vẹn firmware.

```c
/* 機種コード取得 */
model = get_model();
/* チェックサム値取得 */
g_checksum_value = checksum_calc();

/*	2018/05/15 edit by Sky-A.Yamaguchi	[Eagle]制御構成変更No.33関連 PS-CPUバージョン長拡張	*/
/*	バージョン文字列取得	*/
```
**c) Bật watchdog & các module nội bộ** — [entry.c:63-70](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/entry.c#L63-L70):

- `BSP_WDT_Start()`: bắt đầu watchdog — nếu vòng lặp chính treo, PS-CPU tự reset.
- `internal_sts_init()`, `km_it_init()`: khởi tạo trạng thái nội bộ và cơ chế xử lý ngắt.
- `BSP_TIM_Start()`: bật timer 1ms.
- `adc_moni_5v_start_proc()`: bắt đầu giám sát điện áp 5V qua ADC.

**d) Chuỗi cấp nguồn cho S800** — [entry.c:74-77](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/entry.c#L74-L77) & [s800_power_on()](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L607):

- Giữ `_RESET` = 0 (S800 vẫn trong reset).
- `s800_power_on()`: kiểm tra `MSW_ON` (công tắc nguồn cơ), `POWER_MONITOR` (nguồn AC/PSU tốt), `MONI_24V11` (24V đã xả hết từ lần trước) — nếu OK thì bật `SB_PWR_EN` (cấp nguồn DC cho board S800) và khởi động timer 100ms để chuẩn bị nhả reset.
- `i2c_recv_first()`: sẵn sàng nhận lệnh I2C.

**e) Vòng lặp vô hạn (state machine chính)** — [entry.c:83-131](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/entry.c#L83-L131), chạy lặp liên tục, các bước đáng chú ý:

| Hàm                                                                                                                         | Vai trò                                                                                                                                                                                                                                                       |
| --------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `sb_reset_proc()`                                                                                                           | Khi SB power tốt + 24V ổn + MSW ON + timer 100ms hết → **nhả `_RESET`**, cho phép AP806 bắt đầu chạy BootROM riêng của nó                                                                                                                                     |
| `moni_24v_proc()`                                                                                                           | Theo dõi xả điện 24V để đảm bảo an toàn khi bật/tắt                                                                                                                                                                                                           |
| `msw_on_proc` / `power_monitor_proc`                                                                                        | Xử lý công tắc nguồn vật lý và tín hiệu "power monitor" từ nguồn AC; có cơ chế fail-safe cho mất điện thoáng qua (瞬断)                                                                                                                                         |
| `sleep_status_rem_proc(model)`                                                                                              | Đồng bộ tín hiệu Sleep/ErP với S800 (khác nhau giữa dòng Eagle/Sparrow)                                                                                                                                                                                       |
| `mc_p_on_proc(model)`                                                                                                       | Khi `AP_PWR_EN` (do CA72/Linux đã boot xong xác nhận), `_RESET` đã nhả, nguồn SB/24V/MSW đều OK và timer boot ban đầu hết hạn → PS-CPU bật `MC_P_ON` và `IR_P_ON` (nguồn cho các khối "Main Controller"/"Image Reading" — tức các module ngoại vi in ấn/quét) |
| `ap_power_en_proc(STATE_NORMAL)`                                                                                            | Cập nhật trạng thái CA72 (`CA72_ON`/`CA72_SLEEP`/`CA72_OFF`) dựa trên chân `AP_PWR_EN` do phía Linux điều khiển                                                                                                                                               |
| `mc3_3von_moni_proc`, <br>`erp_sensor_proc`, <br>`mc_oe_proc`, <br>`mc_pwr_en_proc`, <br>`rst_usb_proc`, <br>`usb2_oe_proc` | Tuần tự bật/giám sát các rail và tín hiệu ngoại vi còn lại (3.3V, ERP sensor, USB reset/OE...)                                                                                                                                                                |
| `poweroff_timer_proc`, `backupwait_timer_proc`                                                                              | Quản lý timer tắt nguồn có trật tự và cơ chế "backup" cho mất điện ngắn                                                                                                                                                                                       |
| `i2c_recv_wait()`                                                                                                           | Xử lý lệnh I2C đến từ host (sau khi Linux trên S800 đã boot xong và bắt đầu giao tiếp)                                                                                                                                                                        |
| `sleep_mode()`                                                                                                              | Đưa MCU vào chế độ ngủ giữa các chu kỳ để tiết kiệm điện                                                                                                                                                                                                      |
