## 1. Vị trí trong kiến trúc

`PowerControlEventManager` là **task tiêu thụ trung tâm (single consumer)** của KM app — nó là nơi _duy nhất_ chạy state machine quản lý nguồn (Power OFF / Normal / Sleep1 / Sleep2 / ErP) của con LPPP (Cortex-M "Quartz" chạy FreeRTOS), điều khiển việc bật/tắt AP (CA72 chạy Linux).

![[Pasted image 20260815174128.png]]
Cấu hình liên quan ([FreeRTOSConfig.h:123-126](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Os/freertos/portable/GCC/quartz/FreeRTOSConfig.h#L123-L126)): `configTICK_RATE_HZ=100` → **1 tick = 10ms**; `configMAX_PRIORITIES=4`; `configMINIMAL_STACK_SIZE=300 words = 1200 byte`.

---

## 2. Vòng đời khởi tạo (thứ tự rất quan trọng)

```
sys_init.c:124  SysApi_InitApplModules()
                 ├─ SysSystem_Init() → AppInit_StartThreads()
                 │    └─ SysApiThread_Create(AppThread_Main)  prio = configMAX_PRIORITIES-3 = 1
                 └─ power_up_system() → xTaskCreate(init_thread) prio = configMAX_PRIORITIES-1 = 3
sys_init.c:103  vTaskStartScheduler()   ← scheduler mới bắt đầu chạy ở đây
```

Khi scheduler chạy, **init_thread (prio 3, cao nhất) chiếm quyền trước**:

[hal_main.c:173](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/hal_main.c#L173) → [LPPP_KM_Init()](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L236-L253):

```c
LPPP_que_create();      // ← tạo gst_MsgQHandle_PowerCtrl (20 slot)  ★ phải xong TRƯỚC khi task chạy
LPPP_IRQ_Setting();     // attach 6 internal IRQ → LPPP_IntIRQ_Callback
LPPP_Flash_Setting();   // QSPI flash
LPPP_GPIO_Setting();    // GPIO + đăng ký LPPP_GPIO_Callback cho các chân wake
LPPP_InitStatus();      // tạo mutex của statlib
LPPP_i2c_init();
ap_sd  = LPPP_OpenStatus();   // descriptor 0
pwr_sd = LPPP_OpenStatus();   // descriptor 1
LPPP_SetStatus(pwr_sd, PWR_STATUS_POWEROFF);
LPPP_SetStatus(ap_sd,  AP_STATUS_NO_BOOT);
event_handler(LPPP_INTERNALEVT_PWR_SW_ON);   // ★ gọi TRỰC TIẾP, không qua queue
```

**Điểm mấu chốt #1:** transition đầu tiên `POWEROFF → NORMAL` **không chạy trong context của PowerControlEventManager**, mà chạy ngay trong `init_thread` ở priority 3. Đây là lần duy nhất `event_handler()` được gọi ngoài task manager. Vì `init_thread` có prio cao hơn hẳn `PwEvtMgr` (prio 0), nó luôn chạy xong trước → thực tế không có race, nhưng về mặt thiết kế `event_handler()` là **hàm 2-context**.

Sau đó `AppThread_Main` (prio 1) mới tạo các task ([app_init.c:175-195](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/init/app_init.c#L175-L195)):

| Task          | Hàm                      | Priority               | Stack                        |
| ------------- | ------------------------ | ---------------------- | ---------------------------- |
| `PwEvtMgr`    | PowerControlEventManager | `tskIDLE_PRIORITY` (0) | 300 words                    |
| `PwMHUEv`     | PowerControlMHUEvent     | 0                      | 300 words                    |
| `ClrWDT`      | ClearWatchDogCounter     | 0                      | 200 words                    |
| ~~`PwIRQEv`~~ | ~~PowerControlIRQEvent~~ | —                      | **`#if 0` → không được tạo** |

---

## 3. Mổ xẻ vòng lặp (dòng theo dòng)

[LPPP_Task.c:573-596](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L573-L596)

```c
LPPP_INTERNALEVT intr_event = LPPP_INTERNALEVT_MAX;   // (a)
BaseType_t ret = 0;

while( 1 ){
    ret = xQueueReceive(gst_MsgQHandle_PowerCtrl, &intr_event, -1);   // (b)
    if (ret != pdFALSE) {                                             // (c)
        if(intr_event != LPPP_INTERNALEVT_INVALID) {                  // (d)
            event_handler(intr_event);                                // (e)
        }
    }
}
out :                                                                  // (f)
    LPPP_CloseStatus(pwr_sd);
    LPPP_CloseStatus(ap_sd);
```

**(a)** Khởi tạo `= LPPP_INTERNALEVT_MAX` là thừa — bị ghi đè ngay ở lần `xQueueReceive` đầu tiên. Chỉ có ý nghĩa "giá trị an toàn" nếu queue receive fail (không xảy ra ở đây).

**(b) `-1` làm timeout.** Tham số 3 kiểu `TickType_t` (unsigned 32-bit) → `-1` chuyển thành `0xFFFFFFFF` = `portMAX_DELAY`. Vì [`INCLUDE_vTaskSuspend = 1`](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Os/freertos/portable/GCC/quartz/FreeRTOSConfig.h#L162), FreeRTOS coi đây là **block vô hạn thật sự** (task vào Suspended list, không phải Delayed list) → task ngủ 0% CPU cho tới khi có event. Cách viết `-1` thay vì `portMAX_DELAY` là _implicit conversion_, hoạt động đúng nhưng dễ gây hiểu nhầm.

**(c)** Với block vô hạn, `ret` **luôn** là `pdTRUE`. Nhánh `if` này về mặt logic là dead branch — không bao giờ false.

**(d) Bộ lọc INVALID.** Lưu ý enum ([LPPP_inc.h:44-57](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_inc.h#L44-L57)):

```
PWR_SW_OFF=0, PWR_SW_ON=1, GO_TO_S1=2, GO_TO_S2=3, GO_TO_ERP=4,
WAKEUP_NORMAL=5, WAKEUP_S1=6, PWR_FLICKER=7, LVDS_INTERRUPT=8, LVDS_TIMER=9,
INVALID=10, MAX = INVALID = 10
```

`INVALID == MAX == 10`. Ma trận là `[5][10]`, index hợp lệ 0..9. Check này chặn `10`, và `event_handler()` chặn lại lần nữa bằng `LPPP_INTERNALEVT_MAX <= e_Evt`. **Bảo vệ 2 lớp cho cùng một điều kiện** — cố ý (fix bug CT #7690 ghi ở header).

**(e)** Gọi **đồng bộ**. Toàn bộ scenario chạy trong stack của task này (1200 byte) — đây là lý do `LPPP_SpiFlashWrite_PowerFlickerFlag()` phải để buffer 4KB dạng `static` chứ không dùng stack ([LPPP_Matrix.c:157](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Matrix.c#L157): _"スタックは4KBを取れないため静的に確保"_).

**(f) `out:` là dead code.** `while(1)` không có `break` nào → label không bao giờ tới được. Đây là tàn dư của thiết kế cũ (task có thể thoát). GCC sẽ cảnh báo _"label 'out' defined but not used"_.

---

## 4. Năm nguồn sinh event

|#|Nguồn|Context|API dùng|Event tạo ra|
|---|---|---|---|---|
|1|[LPPP_GPIO_Callback](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Gpio.c#L319-L368)|**ISR**|`xQueueSendFromISR`|`PWR_SW_ON/OFF`, `PWR_FLICKER`, `LVDS_INTERRUPT`|
|2|[LPPP_IntIRQ_Callback](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L390-L431)|**ISR**|`xQueueSendFromISR`|(xem cảnh báo ở §6)|
|3|[LPPP_LVDS_tick](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L140-L175)|**ISR timer**|`xQueueSendFromISR`|`LVDS_TIMER`|
|4|[PowerControlMHUEvent](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L290-L361)|task|`xQueueSend`|`WAKEUP_NORMAL`, `GO_TO_S1`|
|5|[app_scpi.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L273-L302)|task|`xQueueSend`|`GO_TO_S2`, `GO_TO_ERP`, `WAKEUP_S1`|

**Nguồn 4 — đường từ Linux/CA72 xuống:** CA72 gửi SCPI qua MHU → `send_km_queue()` đẩy vào `gst_MsgQHandle_MHU` → `PowerControlMHUEvent` dịch `KM_SCPI_CMD_AP_EVENT` thành internal event → forward sang `gst_MsgQHandle_PowerCtrl`. Đây là **queue 2 tầng** (SCPI queue → internal event queue), tách biệt protocol khỏi state machine. Lưu ý `GO_TO_S2`/`GO_TO_ERP` trong task này đã bị `#if 0` ([LPPP_Task.c:332-343](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L332-L343)) — chúng đi qua đường app_scpi.c thay thế.

**Nguồn 1 — có filter đặc biệt:** `WAKEUP_S1` **bị chặn không gửi vào queue** tại [LPPP_Gpio.c:342-344](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Gpio.c#L342-L344) (_"起動イベントはGPIO以外もあるためpower_cpu()で処理"_) — vì wake có nhiều nguồn ngoài GPIO nên được xử lý tập trung ở `power_cpu()`.

**Nguồn 3 — vòng lặp tự duy trì (LVDS):**

```
GPIO XEN_LVDS_ENG đổi ─ISR─► LVDS_INTERRUPT ─► LPPP_LVDS_Polling(true)  → lvds_timer_active = true
                                                        ↓
timer ISR mỗi tick (10ms) ─► LPPP_LVDS_tick ─ đếm 10 tick = 100ms ─► LVDS_TIMER ─► LPPP_LVDS_Polling(false)
                                                        ↓
                                  cần 5 mẫu liên tiếp giống nhau (LPPP_LVDS_FIXED)
                                       = 500ms chống chattering  →  lvds_timer_active = false (dừng)
                                                        ↓
                                   LPPP_LVDS_PowerDown(true/false) → ghi 8 thanh ghi PIOCFG (Y/M/C/K × ENG0/1)
```

Con số khớp chính xác với comment `/* チャタリング500ms */`: 10 tick × 10ms × 5 lần = 500ms.

---

## 5. `event_handler` và ma trận trạng thái

[LPPP_Task.c:187-210](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L187-L210):

```c
if (e_Evt < PWR_SW_OFF || e_Evt >= MAX) return;   // guard cột
e_lppstat = LPPP_GetStatus(pwr_sd);               // ← mutex take/give
if (e_lppstat >= PWR_STATUS_MAX) return;          // guard hàng
e_lppstat = (*fpLPPP_ScenarioFunc[e_lppstat][e_Evt])(e_lppstat, e_Evt);  // dispatch
LPPP_SetStatus(pwr_sd, e_lppstat);                // ← mutex take/give
```

Ma trận 5×10 ([LPPP_Matrix.h:46-54](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Matrix.h#L46-L54)), ô trống = `LPPP_Dummy` (giữ nguyên state, chỉ log):

| State \ Event | PWR_SW_OFF    | PWR_SW_ON      | GO_S1             | GO_S2            | GO_ERP        | WAKE_NORM      | WAKE_S1           | FLICKER         |
| ------------- | ------------- | -------------- | ----------------- | ---------------- | ------------- | -------------- | ----------------- | --------------- |
| **POWEROFF**  | –             | **TR0→NORMAL** | –                 | –                | –             | –              | –                 | –               |
| **NORMAL**    | **TR1→OFF**   | –              | **TR3→SLEEP1**    | –                | –             | –              | –                 | **TR2b→NORMAL** |
| **SLEEP1**    | **TR1→OFF**   | –              | –                 | **TR5/6→SLEEP2** | **TR5/6→ErP** | **TR4→NORMAL** | –                 | –               |
| **SLEEP2**    | **TR7/9→OFF** | –              | **TR8/10→SLEEP1** | –                | –             | –              | **TR8/10→SLEEP1** | **TR2b→NORMAL** |
| **ErP**       | **TR7/9→OFF** | –              | **TR8/10→SLEEP1** | –                | –             | –              | **TR8/10→SLEEP1** | **TR2b→NORMAL** |

Hai cột `LVDS_INTERRUPT` / `LVDS_TIMER` giống hệt nhau ở mọi state (luôn gọi `LPPP_LVDS_Polling`, **trả về `state` không đổi**) — LVDS là "orthogonal concern", ghép vào cùng state machine chỉ để tái dùng cơ chế queue/dispatch.

**Điểm mấu chốt #2 — chuỗi get→dispatch→set KHÔNG atomic.** `LPPP_GetStatus`/`LPPP_SetStatus` mỗi cái tự lấy mutex riêng ([LPPP_Statlib.c:128,159](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Statlib.c#L128-L162)), nhưng khoảng giữa thì không được bảo vệ. An toàn _chỉ vì_ mọi event đều được serialize qua đúng một task tiêu thụ. Nếu ai đó thêm một consumer thứ hai, hoặc gọi `event_handler()` từ context khác (như `LPPP_KM_Init` đang làm), sẽ có lost-update.


## 5.1. **TR0→NORMAL** - `LPPP_TR0_OFFToNormal`
```c
const TickType_t panel_wait_tick = 100 / (1000/configTICK_RATE_HZ);   // = 10 tick = 100ms

tick = xTaskGetTickCount();
while(tick < panel_wait_tick) { tick = xTaskGetTickCount(); }    // ① busy-wait

if(LPPP_GPIO_Read(LPPP_GPIOPIN_PANEL_STOP) == 0) {               // ② GPIO 49
    LPPP_GPIO_Write(LPPP_GPIOPIN_MOTION_EN_MC, 0);               //   GPIO 71 (BANK C)
    LPPP_GPIO_Write(LPPP_GPIOPIN_MOTION_EN_IR, 0);               //   GPIO 72 (BANK C)
}
ret = PWR_STATUS_NORMAL;
```

**① Chờ panel micro-controller** — comment: _"SB_PWR_EN[H]から100ms以降"_. Đây là **so sánh với tick tuyệt đối kể từ boot**, không phải delay tương đối. Hàm chạy trong `init_thread` (prio 3, cao nhất) và **spin CPU** — chặn toàn hệ thống tối đa 100ms. Đồng thời nghĩa là: nếu hàm này chạy ở thời điểm nào khác sau boot (tick > 10), nó **không chờ gì cả**.

**② Bật động cơ (motion)** — chỉ khi phím Stop trên panel **không** bị nhấn. Cả `MOTION_EN_MC` và `MOTION_EN_IR` khai báo `logic_level = 1` trong `gpio_info[]`, nên đây là chuyển **1 → 0** (logic đã được đảo ở bản sửa _2017/10/26 K.hirota MOTION_EN_MC/IR論理変更_).

**Output:** `PWR_STATUS_NORMAL` (1).

**Ảnh hưởng:**

- `MOTION_EN_MC` → cho phép khối Machine Control chạy engine cơ khí; `MOTION_EN_IR` → khối Iris.
- Nếu người dùng **đang giữ phím Stop lúc bật máy**, hai chân này giữ nguyên mức 1 → engine bị khoá, và **không có cơ chế nào bật lại sau đó** trong state machine.
- Toàn bộ phần AP_PWR_EN / AP_PG polling / PIDI / DDR init / copy ATF / release AP reset đã bị `#if 0` với ghi chú _"Marvell対応済み"_ — đã chuyển sang `pmu_phase()` phía Marvell.

## 5.2. **TR1→OFF** - `LPPP_TR1_NormalToOFF`
```c
APPKM_INFO_MSG(...);
/* 処理なし */
return PWR_STATUS_POWEROFF;
```

**Action:** không có. **Output:** `POWEROFF` (0). **Ảnh hưởng:** chỉ đổi biến `pwr_sd`.

❌ Không bao giờ được gọi. Việc tắt máy thực tế đi qua [LPPP_GPIO_CallbackMainSW](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Gpio.c#L651-L655) — kéo thẳng `HRESET_REQN = 0` từ trong ISR, **bỏ qua hoàn toàn** state machine.

## 5.3. **TR7/9→OFF** - ## `LPPP_TR2b_Sleep2_Or_ErPToNormal` - xử lý mất điện thoáng qua

```c
if(LPPP_GetStatus(ap_sd) == AP_STATUS_NO_BOOT) {          // ① điều kiện then chốt
    (void)LPPP_SpiFlashWrite_PowerFlickerFlag();          // ② ghi cờ vào QSPI
    LPPP_GPIO_Write(LPPP_GPIOPIN_HRESET_REQN, 0);         // ③ reset cứng CA72
}
ret = PWR_STATUS_NORMAL;                                   // ④ luôn luôn
```

**① Điều kiện:** chỉ hành động khi **AP chưa boot xong**. `ap_sd` được đặt `NO_BOOT` bởi `TR5_TR6` (lúc vào Sleep2/ErP) và bởi `LPPP_KM_Init`; được đặt lại thành `WARP_BOOT_OR_GET_SNAPSHOT` khi CA72 gửi SCPI `KM_SCPI_CMD_SET_AP_STATUS` → [SCPISetAPStatus()](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_SCPI.c#L96-L100).

**② Ghi cờ vào 2 vùng flash redundant** ([LPPP_Matrix.c:180-192](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Matrix.c#L180-L192)):

```c
unsigned char data = 0x10;
flash_modify_sub(0xF4377000 + 0x07, &data, 1);   // 領域1  (BSP/Thread region 1, 4KB)
flash_modify_sub(0xF4378000 + 0x07, &data, 1);   // 領域2  (bản sao)
```

Offset `0x07` = `FLASH_BspThrRegion_OFS_PDFLAG_UBOOT` — **U-Boot đọc byte này lúc boot lại** để biết lần tắt vừa rồi là do mất điện chứ không phải tắt bình thường. Ghi 2 vùng để chống hỏng nếu mất điện tiếp ngay giữa lúc ghi.

Bản `#if 0` cũ dùng buffer `static unsigned char buf[4096]` (read-modify-write cả sector); bản hiện tại dùng `flash_modify_sub` chạy **từ RAM** (ghi chú _2018/05/10 Sky_) vì không thể vừa ghi vừa đọc lệnh từ chính QSPI.

**③ `HRESET_REQN = 0`** (GPIO 66, BANK C) → reset cứng toàn bộ AP/CA72.

**④ Output:** `PWR_STATUS_NORMAL` — **trả `NORMAL` kể cả khi không làm gì**. Tức nếu `ap_sd == WARP_BOOT` (AP đã chạy ổn), flicker bị **bỏ qua âm thầm** nhưng state vẫn bị ép về `NORMAL`. Nếu đang ở `SLEEP2`/`ErP`, đây là chuyển trạng thái "ngầm" mà không có hành động phần cứng nào đi kèm — state machine và thực tế phần cứng có thể lệch nhau ở điểm này.

## 5.4.  **TR4→NORMAL**-  `LPPP_TR3_NormalToSleep1` — cắt I2S
```c
gpio_set_func_sel( 33, 1 );   /* I2S_SCLK  (khôi phục) */
gpio_set_func_sel( 34, 1 );   /* I2S_WS    (khôi phục) */
LPPP_QoS_SetupUL();
return PWR_STATUS_NORMAL;
```

`LPPP_QoS_SetupUL()` ([LPPP_QoS.c:177-184](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_QoS.c#L177-L184)) ghi đúng **1 thanh ghi**:

```
0xC0040040 (UL SYSTEM PRI0_U2A_MAIN) ← 0x00021012
```

Comment: _"ch43210を高い順→中→[21012]へ"_ — đặt priority 5 kênh U2A (UL→AXI) thành `[2,1,0,1,2]`, tức hạ từ mức cao xuống mức trung bình.

**Output:** `PWR_STATUS_NORMAL` (1).

**Ảnh hưởng:** Lưu ý bất đối xứng — `TR4` **chỉ** setup UL, không setup MoChi/AP; còn `TR8_TR10` thì ngược lại (MoChi + AP, không UL). Lý do: Sleep1 chỉ tắt CPU cluster, các power domain chứa MCix4/PIDI/CCU vẫn giữ điện nên giá trị QoS còn nguyên; Sleep2/ErP mới cắt các domain đó.

## 5.5. **TR1→OFF** `LPPP_TR8_TR10_Sleep2_Or_ErPToSleep1` — hàm nặng nhất
```c
if( isAutomaticMeasurementToolOn() == true ) {                      // ①
    retval = LPPP_i2c_write(0, PSCPU_SLAVE_ADDR /*0x3E*/,
                            PSCPU_REG_EVENT /*0xC0*/,
                            THRD_EventRecord_PressPanelPowerKeyDeepSleep /*56*/);
    if(retval < 0) ApiSysDebug_Printf(... "LPPP_i2c_write ERROR ret=[%d]" ...);
}
Bregister_modify(B014_ADDRESS, 1<<B014_TO_SLEEP1, 1<<B014_TO_SLEEP1);   // ②
LPPP_QoS_SetupMoChi();                                                   // ③
LPPP_QoS_SetupAP();                                                      // ④
ret = PWR_STATUS_SLEEP1;
```

**① Thông báo sự kiện cho PS-CPU** (chỉ khi công cụ đo tự động đang bật): ghi giá trị `56` = _"đã nhấn nút nguồn trên panel (DeepSleep)"_ vào register `0xC0` của con vi điều khiển PS-CPU tại địa chỉ I2C `0x3E`, bus 0. Đây là để công cụ đo ghi lại timeline khởi động. Lỗi I2C chỉ được log, **không ảnh hưởng luồng**.

**② Ghi bit 13 (`B014_TO_SLEEP1`)** vào flash — cùng cơ chế RMW như `TR5_TR6`.

**③ `LPPP_QoS_SetupMoChi()`** — 10 thanh ghi về 0:

```
0xC05A0080..0x0090  MCix4 Read  QoS Remapping RD0..RD4 ← 0
0xC05A00C0..0x00D0  MCix4 Write QoS Remapping WR0..WR4 ← 0
```

**④ `LPPP_QoS_SetupAP()`** — nặng nhất, ~15 thanh ghi:

```
0xCE000F18                       ← 0x441
0xCE001B18  CCU_B_LTC_QOS_CPU01  ← 0x001    (chỉ bật share, không ưu tiên R/W)
0xCE001F18  CCU_B_LTC_QOS_CPU23  ← 0x001
0xCE001318  CCU_B_LTC_QOS_DMA    ← 0x000
0xC05A0000..0x0010  PIDI Read  RD0..RD4  ← 0
0xC05A0040..0x0050  PIDI Write WR0..WR4  ← 0
NB_MOCHIx4_CONTROLLER_ADDR (+0/+4/+4)    ← 0x090081c7 / 0x000d0120 / 0x000d0300
                                            (MCIx4 outstanding 8/8)
```

**Output:** `PWR_STATUS_SLEEP1` (2).

**Ảnh hưởng:**

- **Vì sao phải setup lại QoS?** Các thanh ghi MCix4/PIDI/CCU nằm trong power domain bị cắt điện khi vào Sleep2/ErP → mất giá trị. Nếu không nạp lại, băng thông DDR giữa CPU/PIDI/MoChi sẽ về mặc định và gây sụt hiệu năng in.
- **Chi phí:** 1 giao dịch I2C (~ms) + 1 chu trình read-modify-write SPI-Flash (~chục ms) + ~25 lần ghi register + **~25 dòng `ApiSysDebug_Printf`** (khối `#if 1` trong `LPPP_QoS.c` in ra toàn bộ giá trị đọc lại). Toàn bộ chạy đồng bộ, blocking, trong stack 1200 byte của `PwEvtMgr`. Đây là hàm dễ gây trễ nhất trong toàn ma trận.
- Ô `[SLEEP2][GO_TO_S1]` cũng trỏ vào hàm này: nếu CA72 gửi `GO_TO_S1` trong lúc đang ở Sleep2, máy sẽ **đi lên** Sleep1 chứ không xuống — đúng theo hướng "thức dần".

## 5.6. `LPPP_LVDS_Interrupt` / `LPPP_LVDS_Timer` — không đổi state
```c
stat_t LPPP_LVDS_Interrupt(stat_t state, ...) { LPPP_LVDS_Polling(true);  return state; }
stat_t LPPP_LVDS_Timer    (stat_t state, ...) { LPPP_LVDS_Polling(false); return state; }
```

**Output:** trả `state` nguyên vẹn — **hai cột này không bao giờ làm chuyển trạng thái nguồn**. Chúng chỉ mượn cơ chế dispatch của ma trận (chiếm 10/50 ô, giống hệt nhau ở cả 5 hàng) để chạy một máy trạng thái con hoàn toàn độc lập.

**Ảnh hưởng thật sự** nằm ở đích cuối `LPPP_LVDS_PowerDown()` ([LPPP_Task.c:67-79](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L67-L79)) — chỉ được gọi khi bộ chống chattering đã chốt sau 500ms:

```c
mrvl_regrdmwr32(0xE8308440, 1<<25, value);   // Y_ENG0_P_PIOCFG
mrvl_regrdmwr32(0xE8308444, 1<<25, value);   // Y_ENG1
mrvl_regrdmwr32(0xE8308448/0x444C, ...);     // M_ENG0/1
mrvl_regrdmwr32(0xE8308450/0x4454, ...);     // C_ENG0/1
mrvl_regrdmwr32(0xE8308458/0x445C, ...);     // K_ENG0/1
```

Bit 25 = PWRDN của 8 pad LVDS (4 màu Y/M/C/K × 2 engine). Tắt transceiver LVDS tới print head khi engine không dùng → **tiết kiệm điện**, độc lập hoàn toàn với Sleep1/2/ErP.

## Phần C — Bản đồ tác động tổng hợp

```
                 ┌──────────────── GPIO output ────────────┐
TR0  ────────────► MOTION_EN_MC(71)=0, MOTION_EN_IR(72)=0  |cho phép engine chạy
TR2b ────────────► HRESET_REQN(66)=0                       │ reset cứng CA72
TR7/9 ───────────► ENG_STATUS1(192)=1, ENG_STATUS2(193)=1, │ (code chết)
                   AP_PWR_EN(65)=1                         │
                 └─────────────────────────────────────────┘

                 ┌──────────── Pad function mux ──────────┐
TR3  ────────────► pad33/34 FUNC_SEL = 0  (I2S → GPIO)    │ cắt audio khi ngủ
TR4  ────────────► pad33/34 FUNC_SEL = 1  (GPIO → I2S)    │ khôi phục
                 └────────────────────────────────────────┘

                 ┌────────────── SPI-Flash (QSPI) ────────┐
TR2b ────────────► 0xF4377007 / 0xF4378007 ← 0x10         │ cờ mất điện, U-Boot đọc
TR5/6 ───────────► 0xF43FF01C bit11 (TO_SLEEP2_OR_ErP)    │ breadcrumb phase
TR8/10 ──────────► 0xF43FF01C bit13 (TO_SLEEP1)           │ breadcrumb phase
                 └────────────────────────────────────────┘

                 ┌────────────── AXI QoS register ────────┐
TR4  ────────────► UL PRI0_U2A_MAIN                (1 reg)│ Sleep1 không mất domain
TR8/10 ──────────► MCix4 RD/WR ×10 + PIDI ×10 + CCU ×4(~25)│ Sleep2/ErP mất domain
                 └─────────────────────────────────────────┘

                 ┌──────────────── I2C / khác ───────────────┐
TR8/10 ──────────► PS-CPU I2C0 addr 0x3E reg 0xC0 ← 56       │ log cho tool đo
LVDS_* ──────────► 8×PIOCFG bit25 (Y/M/C/K × ENG0/1)         │ power-down LVDS
                 └───────────────────────────────────────────┘

                 ┌──────────── Trạng thái nội bộ ───────┐
TR5/6 ───────────► ap_sd = AP_STATUS_NO_BOOT            │ ★ vũ trang cho TR2b
(SCPI SET_AP_STATUS) ► ap_sd = WARP_BOOT_OR_GET_SNAPSHOT│ giải giáp
                 └──────────────────────────────────────┘
```

**Ba quan sát cuối:**

1. **`ap_sd` là biến ghép nối duy nhất giữa 2 transition** — `TR5_TR6` đặt nó, `TR2b` đọc nó. Đây là cặp quan hệ ngầm không thể hiện trong ma trận: đọc riêng `TR2b` sẽ không hiểu vì sao có lúc reset CA72, có lúc không.
    
2. **Chi phí thực thi rất lệch nhau**: `TR3` ~2 lần ghi register (microsecond), còn `TR8_TR10` gồm I2C + flash RMW + 25 register + 25 dòng debug print (có thể hàng chục ms). Vì `PwEvtMgr` xử lý **tuần tự, một event một lúc**, một `TR8_TR10` đang chạy sẽ chặn mọi `LVDS_TIMER`/`PWR_FLICKER` phía sau trong queue.
    
3. **Hai transition duy nhất đụng tới flash đều nằm trên đường vào/ra sleep** — nghĩa là mọi chu kỳ sleep của máy đều kéo theo ghi QSPI. Đây là điểm cần chú ý cả về hiệu năng lẫn tuổi thọ flash.



---

## 6. Trace một luồng thật: Linux yêu cầu vào Sleep1

```
[CA72/Linux] gửi SCPI: KM_SCPI_CMD_AP_EVENT, payload[0]=KM_SCPI_AP_EVENT_GO_TO_S1
        ↓ MHU hardware interrupt
[LPPP] scpi handler → send_km_queue() → xQueueSend(gst_MsgQHandle_MHU)
        ↓
[task PwMHUEv] xQueueReceive(MHU) → check payload_size == sizeof(int)
               → intr_event = LPPP_INTERNALEVT_GO_TO_S1 → xQueueSend(PowerCtrl, portMAX_DELAY)
        ↓
[task PwEvtMgr] xQueueReceive(PowerCtrl) → event_handler(GO_TO_S1)
               → LPPP_GetStatus(pwr_sd) = PWR_STATUS_NORMAL (1)
               → fpLPPP_ScenarioFunc[1][2] = LPPP_TR3_NormalToSleep1
                     ├─ gpio_set_func_sel(33, 0)  // I2S_SCLK → GPIOB[1]
                     ├─ gpio_set_func_sel(34, 0)  // I2S_FSYN → GPIOB[2]
                     └─ return PWR_STATUS_SLEEP1
               → LPPP_SetStatus(pwr_sd, PWR_STATUS_SLEEP1)
```

Và luồng đi tiếp xuống Sleep2 (`LPPP_TR5_TR6`): reset `ap_sd = AP_STATUS_NO_BOOT`, ghi `B014_TO_SLEEP2_OR_ErP` vào B-register. Cờ `ap_sd` này chính là thứ `LPPP_TR2b` đọc để phát hiện **mất điện thoáng qua khi AP chưa boot** → ghi flag `0x10` vào 2 vùng QSPI-Flash rồi kéo `HRESET_REQN=0` để reset cứng ([LPPP_Matrix.c:207-222](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Matrix.c#L207-L222)).

---

## 7. Những điểm cần lưu ý / rủi ro

**🔴 `LPPP_AnalyzeIRQEvent()` không có `return`** — [LPPP_Task.c:373-378](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L373-L378) khai báo trả `LPPP_INTERNALEVT` nhưng thân hàm chỉ có 2 dòng debug print, **không có câu lệnh return nào** (T.B.D chưa implement). `LPPP_IntIRQ_Callback` dùng giá trị rác từ thanh ghi r0. Trên ARM/GCC không có `-Werror=return-type`, đây là _undefined behavior_: nếu rác rơi vào 0..9 thì một transition **sai ngẫu nhiên** sẽ được bơm vào state machine. Range check ở dòng 400 lọc phần lớn nhưng không phải tất cả. 6 IRQ nội bộ đều attach vào callback này ([LPPP_Task.c:445-452](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L445-L452)).

**🟠 Tất cả task ở `tskIDLE_PRIORITY` (= 0).** `xHigherPriorityTaskWoken` từ `xQueueSendFromISR` gần như **không bao giờ true**, vì task nhận (prio 0) không cao hơn task đang chạy. Toàn bộ khối `portYIELD_FROM_ISR()` được thêm vào bởi OP_BTS-22999 thực tế hiếm khi kích hoạt. Hệ quả: latency xử lý event phụ thuộc vào round-robin giữa các task cùng prio 0.

**🟠 Busy-wait trong `LPPP_TR0_OFFToNormal`.** [LPPP_Matrix.c:94-97](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Matrix.c#L94-L97) spin `while(tick < 10)` — so sánh với **tick tuyệt đối từ lúc boot**, không phải delay tương đối. Chạy từ `init_thread` prio 3 → chặn toàn hệ thống tối đa 100ms. Nhưng nếu `TR0` được kích hoạt lần thứ hai sau khi hệ thống chạy lâu (tick > 10), vòng lặp **không chờ gì cả** → mất luôn 100ms panel wait. Đây là hai hành vi khác nhau tùy thời điểm.

**🟡 Không có backpressure handling.** Queue 20 slot; các producer trong task dùng `portMAX_DELAY` (block chờ) còn ISR thì không thể block — nếu queue đầy, `xQueueSendFromISR` trả fail và **event bị mất im lặng** (chỉ log). Với `PWR_FLICKER` hoặc `PWR_SW_OFF` thì mất event là nghiêm trọng.

**🟡 Stack 1200 byte.** Task này gọi sâu xuống flash write (`flash_modify_sub`), I2C, và `ApiSysDebug_Printf` với format phức tạp. Đã có dấu vết tác giả phải né stack 4KB. Đáng đo `uxTaskGetStackHighWaterMark()` nếu gặp crash lạ.

**🟡 `PowerControlIRQEvent` là code chết** — bị `#if 0` ở [app_init.c:190-195](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/init/app_init.c#L190-L195). Cùng với nó, `LPPP_GPIO_AnalyzeEvent()` (bản polling, [LPPP_Gpio.c:381-393](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Gpio.c#L381-L393)) hard-code trả `PWR_SW_ON` với comment T.B.D — nếu ai bật lại task này, mọi thay đổi GPIO sẽ sinh ra `PWR_SW_ON`.

# 8. Nhận event

## 8.1. Ai gửi event
Có **4 producer thật sự** + 1 producer hỏng + 1 đường gọi thẳng không qua queue.

![[Pasted image 20260816024024.png]]

### B.1 — GPIO ISR → `PWR_FLICKER` / `LVDS_INTERRUPT`
**Ai đăng ký:** [LPPP_GPIO_Setting()](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Gpio.c#L554-L576) chạy trong `LPPP_KM_Init`. Vòng lặp duyệt `gpio_info[]` nhưng chỉ attach callback cho **đúng 2 chân**:

```c
if( GPIO_DIRECTION_INPUT == gpio_info[i].direction ){
    if( ( LPPP_GPIOPIN_POWER_MONITOR == gpio_info[i].pinno ) ||
        ( LPPP_GPIOPIN_XEN_LVDS_ENG  == gpio_info[i].pinno ) ){
        gpio_isr_attach( pin[i], LPPP_GPIO_Callback, &gpio_info[i].pinno, ... );
        gpio_isr_enable(pin[i]);
    } else {
        gpio_close(pin[i]);      // ← tất cả chân input khác bị ĐÓNG, không có ISR
    }
} else { gpio_close(pin[i]); }
```

Các chân trong `wake_gpio_info[]` (PANEL_SUBSW, WAKE_UP, DOOR_OPEN, MAINSW_ON…) được attach `LPPP_GPIO_CallbackDummy` — hàm này `return false;` và **không đụng vào queue**; chúng chỉ được cấu hình để phần cứng PMU dùng làm nguồn wake. `MAINSW_ON` được xử lý riêng bởi [LPPP_GPIO_CallbackMainSW](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Gpio.c#L641-L659) — hàm này kéo thẳng `HRESET_REQN=0`, cũng không gửi queue.

➡️ **Hệ quả:** trong [LPPP_GPIO_AnalyzeEventfromISR](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Gpio.c#L251-L305), chỉ 2 `case` là sống (`POWER_MONITOR`, `XEN_LVDS_ENG`); `case MAINSW_ON` (→ `PWR_SW_ON/OFF`) và nhóm case wake (→ `WAKEUP_S1`) là **dead code**.

**Khi nào gửi:**

|Chân|ISR type|Điều kiện gửi|Event|
|---|---|---|---|
|`POWER_MONITOR`|EITHER_EDGE|`is_power_flicker()` = true, tức phát hiện **cạnh lên** (`prev==0 && now==1`)|`PWR_FLICKER`|
|`XEN_LVDS_ENG`|EITHER_EDGE|mọi cạnh, không điều kiện|`LVDS_INTERRUPT`|

**Flow đầy đủ:**

```
Điện áp AC chập chờn → POWER_MONITOR đổi mức → HW interrupt
  └► gpio ISR dispatcher → LPPP_GPIO_Callback(ppin, ui_LogicLevel, &pinno)
       ├─ NULL check ppin / pv_PinTblNo
       ├─ LPPP_GPIO_AnalyzeEventfromISR(level, pinno)
       │    └─ case POWER_MONITOR: is_power_flicker(level)?
       │         └─ static prev_logic_level: 0→1 mới trả PWR_FLICKER,
       │            ngược lại trả INVALID(10)
       ├─ range check: INVALID(10) >= MAX(10) → return false  ← lọc cạnh xuống
       ├─ if (event == WAKEUP_S1) return false;   ← nhánh chết (xem trên)
       ├─ xQueueSendFromISR(gst_MsgQHandle_PowerCtrl, &intr_event, &woken)
       └─ if(woken) { irq/fiq mode ? portYIELD_FROM_ISR() : taskYIELD() }
```

Đích đến: `NORMAL/SLEEP2/ErP + PWR_FLICKER` → `LPPP_TR2b_Sleep2_Or_ErPToNormal` → nếu `ap_sd == AP_STATUS_NO_BOOT` thì ghi cờ `0x10` vào 2 vùng QSPI-Flash rồi kéo `HRESET_REQN=0`.

### B.2 — Timer ISR → `LVDS_TIMER`

**Ai gửi:** [LPPP_LVDS_tick()](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L140-L175), được gọi từ [hal_cputimer.c:153](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/src/hal_cputimer.c#L153) — tức **trong ISR của CPU timer, mỗi tick 10ms**.

**Khi nào gửi:** chỉ khi `lvds_timer_active == true`, và cứ mỗi `LPPP_LVDS_PERIOD = 10` tick = **100ms** mới gửi 1 event.

**Flow — đây là vòng lặp tự khởi động rồi tự tắt:**

```
① XEN_LVDS_ENG đổi cạnh (B.1) → LVDS_INTERRUPT vào queue
② PwEvtMgr → LPPP_LVDS_Interrupt() → LPPP_LVDS_Polling(reset=true)
        └─ lvds_timer_active = true;  lvds_count = 0;      ← "bật máy bơm"
③ timer ISR mỗi 10ms: lvds_tick_cnt++
      khi đủ 10 → xQueueSendFromISR(LVDS_TIMER) ; lvds_tick_cnt = 0
④ PwEvtMgr → LPPP_LVDS_Timer() → LPPP_LVDS_Polling(reset=false)
        ├─ đọc LPPP_GPIO_Read(XEN_LVDS_ENG)
        ├─ khác lần trước → lvds_count = 0   (reset chống rung)
        └─ giống lần trước → lvds_count++
⑤ lvds_count >= LPPP_LVDS_FIXED(5) → CHỐT:
        ├─ nếu giá trị đổi so với lần chốt trước:
        │     0 → "LVDS ON"  → LPPP_LVDS_PowerDown(false)
        │     1 → "LVDS OFF" → LPPP_LVDS_PowerDown(true)
        │        (ghi bit 25 vào 8 thanh ghi PIOCFG: Y/M/C/K × ENG0/ENG1)
        └─ lvds_timer_active = false;       ← "tắt máy bơm", quay lại ①
```

Tổng thời gian chống chattering: 5 lần × 100ms = **500ms**, khớp comment `/* チャタリング500ms */`.

⚠️ Lưu ý: mỗi chu kỳ LVDS bơm khoảng 5–6 event vào queue 20 slot. Nếu tín hiệu LVDS rung liên tục, đây là nguồn tải chính của queue.

### B.3 — `PowerControlMHUEvent` → `WAKEUP_NORMAL` / `GO_TO_S1`

Đây là đường **Linux/CA72 chủ động yêu cầu chuyển power state**, đi qua **3 tầng queue**.

```
[CA72 - Linux]  ghi SCPI vào shared memory + kích MHU doorbell
            command_id = 0x88 (KM_SCPI_CMD_AP_EVENT), payload[0] = 0 hoặc 1
        │
        ▼  MHU hardware interrupt
[LPPP - ISR]  mhu_handler(isr_input)                       app_MHU.c:119
        ├─ get_MHU_status(INPUT_PORT, &status)  → duyệt từng bit kênh
        ├─ cpu_dcache_invalidate_region(header_source, ...)   ← bắt buộc, AP ghi qua DDR
        ├─ memcpy(&msg.scpi_header, header_source, size)
        ├─ clear_MHU(INPUT_PORT, 1<<bit)
        └─ xQueueSendFromISR(got_msg_qhandle, &msg, &woken)   ← QUEUE #1
        │
        ▼
[task AppMhuTest, prio 0]  app_MHU.c:206
        ├─ xQueueReceive(got_msg_qhandle, &msg, ...)
        └─ process_scpi_cmd(msg.channel_number, &msg.scpi_header)   app_scpi.c:676
             └─ send_km_queue(s_header)  ← gọi VÔ ĐIỀU KIỆN ở dòng 698,
                  └─ xQueueSend(gst_MsgQHandle_MHU, scpi, portMAX_DELAY)  ← QUEUE #2
                     (mọi SCPI command đều được nhân bản sang KM, lọc sau)
        │
        ▼
[task PwMHUEv, prio 0]  LPPP_Task.c:290
        ├─ xQueueReceive(gst_MsgQHandle_MHU, &scpi, -1)
        ├─ switch(scpi.command_id):
        │    case KM_SCPI_CMD_AP_EVENT (0x88):
        │       if (scpi.payload_size != sizeof(int)) break;      ← guard kích thước
        │       payload[0]==KM_SCPI_AP_EVENT_WAKEUP_NORMAL(0) → WAKEUP_NORMAL
        │       payload[0]==KM_SCPI_AP_EVENT_GO_TO_S1(1)      → GO_TO_S1
        │       /* GO_TO_S2(2), GO_TO_ERP(3) bị #if 0 — đi đường B.4 */
        │    default: break;   ← mọi command_id khác bị bỏ, is_send_queue = 0
        └─ if(is_send_queue) xQueueSend(gst_MsgQHandle_PowerCtrl, ...)  ← QUEUE #3
        │
        ▼
[task PwEvtMgr, prio 0]  ← nhận ở đây
```

**Vì sao 3 tầng?** Tầng 1 = tách ISR khỏi task (không xử lý trong ISR). Tầng 2 = nhân bản luồng SCPI sang KM app để tách protocol Marvell khỏi state machine KM. Tầng 3 = chuyển từ "ngôn ngữ SCPI" sang "internal event".

⚠️ Cả `AppMhuTest` và `PwMHUEv` đều prio 0 → sau `send_km_queue`, `PwMHUEv` phải đợi round-robin; rồi `PwEvtMgr` lại đợi tiếp. Một lệnh từ CA72 có thể mất **2 lượt time-slice (~20ms)** mới tới được state machine.

### B.4 — `power_cpu()` → `GO_TO_S2` / `GO_TO_ERP` / `WAKEUP_S1`

Đây là đường **suy ra từ trạng thái phần cứng**, không phải lệnh tường minh. Chạy trong context task `AppMhuTest` (qua `process_scpi_cmd` → `case 0x03` → `scpi_set_power_state` → `power_cpu`).

**Nhánh tắt (`power == 3`)** — [app_scpi.c:214-280](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L214-L280):

```
CA72 gửi SCPI 0x03 (set power state) cho từng core khi Linux suspend
  └─ power_cpu(cpuid, 3)  [giữ cpu_power_mutex]
       ├─ chờ WFI status của core (poll tối đa 20 lần × 1µs)
       ├─ Isolation enable → PWR_DN_RQ → CCU_B_PRCR reset
       ├─ cpu_state |= (1<<cpu)
       └─ if (cpu_state == ALL_CPU_OFF)      ← ★ CHỈ khi core CUỐI CÙNG tắt
            ├─ đặt PLAT_WARMBOOT_FLAG_BASE
            ├─ pmu_phase(pmu_suspend);  suspended = true;
            ├─ intr_event = g_wolMode ? GO_TO_ERP : GO_TO_S2   ← WoL quyết định
            ├─ xQueueSend(gst_MsgQHandle_PowerCtrl, portMAX_DELAY)
            └─ CA72WDT_End()
```

➡️ Event chỉ được gửi **một lần duy nhất**, khi CPU cuối cùng của CA72 tắt. `g_wolMode` (Wake-on-LAN) là thứ quyết định đi vào **Sleep2** hay **ErP**.

**Nhánh bật (`power != 3`)** — [app_scpi.c:283-303](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L283-L303):

```
Có nguồn wake (RTC / panel / LAN / USB / door…) → PMU đánh thức LPPP
  └─ power_cpu(cpuid, on)
       └─ if (cpu_state == ALL_CPU_OFF)      ← ★ CHỈ ở core ĐẦU TIÊN bật
            ├─ CA72WDT_Start()
            ├─ Bregister_modify(B014_BEGIN_PHASE6)
            ├─ pmu_phase(pmu_resume)   → nếu -1 thì return false (KHÔNG gửi event)
            ├─ intr_event = LPPP_INTERNALEVT_WAKEUP_S1
            ├─ xQueueSend(gst_MsgQHandle_PowerCtrl, portMAX_DELAY)
            ├─ set RVBAR_0/1 = WARM_BOOT_ADDR   ← điểm vào warm boot của CA72
            ├─ intDisable(INTNUM_MCIX2_1); clear_wake_ints(); wake_lan();
            └─ SysApiSystem_Wake()
```

➡️ Đây chính là lý do `LPPP_GPIO_Callback` **cố tình chặn** `WAKEUP_S1` ([LPPP_Gpio.c:342](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Gpio.c#L342), comment _"起動イベントはGPIO以外もあるためpower_cpu()で処理"_): wake có thể đến từ LAN/RTC/USB chứ không chỉ GPIO, nên điểm phát `WAKEUP_S1` được gom về **một chỗ duy nhất** là `power_cpu()`, sau khi `pmu_resume` đã thành công.

### B.5 — `LPPP_IntIRQ_Callback` (⚠ producer hỏng)

Attach cho 5/6 IRQ nội bộ trong [c_intirq_table](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L445-L452) (`NCTI_IRQ`, `NPMU_IRQ`, `NVAL_FIQ`, `NVAL_IRQ`, `MC_LPP_IRQ1`; riêng `MC_LPP_IRQ0` dùng `LPPP_McChangeFreq_handler`).

```c
intr_event = LPPP_AnalyzeIRQEvent(ui_isr_input);   // ← hàm này KHÔNG có return!
if (intr_event < PWR_SW_OFF || intr_event >= MAX) return;   // lọc phần lớn rác
xQueueSendFromISR(gst_MsgQHandle_PowerCtrl, &intr_event, &woken);
```

[LPPP_AnalyzeIRQEvent](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L373-L378) khai báo trả `LPPP_INTERNALEVT` nhưng thân hàm chỉ có 2 dòng `APPKM_DBG_MSG` — không có `return` nào (T.B.D chưa implement). Giá trị nhận được là rác trong r0. Range check chặn hầu hết, nhưng nếu rác rơi vào 0..9 thì **một transition sai ngẫu nhiên** sẽ được bơm vào state machine.

### B.6 — Đường không đi qua queue

[LPPP_KM_Init()](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Task.c#L252) gọi thẳng `event_handler(LPPP_INTERNALEVT_PWR_SW_ON)`. Đây là transition `POWEROFF → NORMAL` duy nhất, chạy trong `init_thread` (prio 3), **trước khi `PwEvtMgr` từng chạy lần nào**. Nghĩa là khi `PwEvtMgr` bắt đầu vòng `while(1)` đầu tiên, state đã là `PWR_STATUS_NORMAL`.

### Phần C — Tổng kết bảng tra

|Event|Producer|Context|Trigger cụ thể|
|---|---|---|---|
|`PWR_FLICKER` (7)|`LPPP_GPIO_Callback`|ISR|`POWER_MONITOR` có cạnh lên (mất điện thoáng qua)|
|`LVDS_INTERRUPT` (8)|`LPPP_GPIO_Callback`|ISR|`XEN_LVDS_ENG` đổi cạnh bất kỳ|
|`LVDS_TIMER` (9)|`LPPP_LVDS_tick`|ISR timer|mỗi 100ms **khi** `lvds_timer_active`|
|`WAKEUP_NORMAL` (5)|`PowerControlMHUEvent`|task prio 0|CA72 gửi SCPI `0x88`, payload[0]=`0`|
|`GO_TO_S1` (2)|`PowerControlMHUEvent`|task prio 0|CA72 gửi SCPI `0x88`, payload[0]=`1`|
|`GO_TO_S2` (3)|`power_cpu()`|task prio 0|CPU CA72 cuối cùng tắt, `g_wolMode == false`|
|`GO_TO_ERP` (4)|`power_cpu()`|task prio 0|CPU CA72 cuối cùng tắt, `g_wolMode == true`|
|`WAKEUP_S1` (6)|`power_cpu()`|task prio 0|`pmu_resume` OK ở CPU đầu tiên bật lại|
|`PWR_SW_ON` (1)|⚠ chỉ `LPPP_KM_Init` gọi trực tiếp|init_thread prio 3|một lần lúc boot, **không qua queue**|
|`PWR_SW_OFF` (0)|❌ **không producer nào gửi**|—|nhánh trong `AnalyzeEventfromISR` là dead code|

Điểm đáng chú ý cuối: **`PWR_SW_OFF` không bao giờ vào được queue**, nghĩa là các transition `LPPP_TR1_NormalToOFF` và `LPPP_TR7_TR9_Sleep2_Or_ErPToOFF` trong ma trận **không bao giờ được thực thi** ở firmware hiện tại. Việc tắt máy được làm bằng cách kéo thẳng `HRESET_REQN=0` từ [LPPP_GPIO_CallbackMainSW](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Gpio.c#L651-L655), bỏ qua hoàn toàn state machine.