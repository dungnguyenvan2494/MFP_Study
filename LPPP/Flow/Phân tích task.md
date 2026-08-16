Dưới đây là phân tích chi tiết 6 task chạy đồng thời (tất cả đều ở `tskIDLE_PRIORITY` — nghĩa là round-robin cùng mức ưu tiên).

### Bức tranh tổng thể

```
[MHU IRQ từ CA72/AP]
        ↓ xQueueSendFromISR
   got_msg_qhandle ──→ AppMhuTest  ──(send_km_queue)──→ gst_MsgQHandle_MHU
        ↑                  │                                    ↓
   LPPP_SendMHUMsg         │ gọi trực tiếp                PowerControlMHUEvent
   (từ KM)                 │ CA72WDT_*()                         │ đổi SCPI→event
                           ↓                                     ↓
                    ClearWatchDogCounter ←─sem─→        gst_MsgQHandle_PowerCtrl
                                                                 ↓
[IRQ nội bộ / LVDS tick] ──xQueueSendFromISR──────→  PowerControlEventManager
                                                        ↓ event_handler()
                                                   ma trận 5 state × 10 event

[Gói mạng, lệnh IOCTL] ──event flags (không queue)──→  AppMvl (AppThread_Main)
```

---

### 1. `AppMvl` — chính `AppThread_Main`

**Làm gì:** sau khi tạo 5 task con, nó tự trở thành task xử lý mạng. Vòng lặp gọi `ApiAppMainLoop()` → block tại `ApiSysGetEventsBLock(&Events)`.

**Các action và mục đích:**

- `APP_EVENT_RECEIVE_PM_PACKET` → gói mạng đã khớp packet filter phần cứng. Đây là nơi proxy LLMNR/NBNS/mDNS/SNMP trả lời thay host, mục đích: **không đánh thức CPU chính** cho các truy vấn tầm thường (ai đang hỏi tên máy in, hỏi trạng thái SNMP...)
- `APP_EVENT_DRIVER_HELLO` / `GOODBYE` → bàn giao quyền điều khiển Ethernet giữa driver Linux và firmware
- `APP_EVENT_RECEIVE_MESSAGE_FROM_SYSTEM` → lệnh IOCTL từ driver (cấu hình IP, filter, proxy) qua ComFifo trong SRAM
- `APP_EVENT_LINK_UP/DOWN`, `CHANGED_NETWORK_CREDENTIALS`
- `APP_EVENT_SELF_EVENT` → do `periodic_callback()` bắn mỗi 1000 ms (`ApiSysSetEvent_self_event()`), dùng để làm việc định kỳ mà không cần timer riêng

**Giao tiếp:** ❗Task này **không dùng queue FreeRTOS**. Nó dùng event-group của lớp Sys. Tức là nó gần như tách rời hoàn toàn với cụm task KM — hai thế giới song song trên cùng con chip.

---

### 2. `MHU` — `AppMhuTest` (ApplicationMrvl/Test/app_MHU.c)

**Làm gì:** đây là **cổng giao tiếp duy nhất với CPU ứng dụng (CA72)**. Tên "Test" gây hiểu nhầm — nó là code sản phẩm.

**Khởi tạo:** `intAttach(get_mhu_intnum(), 0, mhu_handler, 0)` + `intEnable`.

**Luồng:**

1. ISR `mhu_handler` quét từng bit trạng thái MHU, xác định channel (0–4), đóng gói `int_msg_format_t` rồi `xQueueSendFromISR(got_msg_qhandle,...)`. Có cảnh báo khi queue ≥ 8 phần tử.
2. Task block tại `xQueueReceive(got_msg_qhandle, &msg, -1)`.
3. Gọi `process_scpi_cmd(channel, &msg.scpi_header)` — xử lý lệnh SCPI.
4. Nếu cần trả lời: `write_MHU(OUTPUT_PORT, 1<<channel)` rồi **poll đợi** AP nhận xong (`do { get_MHU_status } while (i & bit)`).
5. Đo thời gian xử lý, cảnh báo nếu > 50 ms (`scpi : very slow responce`) — vì AP có timeout.

**Nội dung trao đổi (lệnh SCPI):** `GET_PROTOCOL_VER`, `AP_OFF` (hạ AP_PWR_EN), `SET/GET_AP_STATUS`, `SET/GET_PWR_STATUS`, `GET_WAKE_INFO` (nguyên nhân đánh thức), `AP_EVENT`, `REWRITE_REQUEST` (0x89, ghi lại ROM LPPP), `GET_PMU_EVENT`, `WDT_REBOOT`, `WDT_REBOOTEXT`, lệnh 0x02 get-capability, 0x80 config wake interrupt, và các lệnh power domain của PMU.

**Giao tiếp:**

- **Nhận** từ `got_msg_qhandle` (ISR MHU đẩy vào; KM cũng có thể đẩy vào qua `LPPP_SendMHUMsg()`)
- **Gửi** vào `gst_MsgQHandle_MHU` — thông qua `send_km_queue(s_header)` gọi ở **đầu** `process_scpi_cmd`, nghĩa là **mọi lệnh SCPI đều được nhân bản sang cho KM** trước khi Marvell tự xử lý
- **Gọi hàm trực tiếp** (không qua queue): `CA72WDT_Start/End/Refresh/Ext`, `SCPISetPowerStatus`... → chia sẻ state với `ClrWDT` qua semaphore

---

### 3. `cmd thread` — `cmd_thread` (app_cmdproc.c)

**Làm gì:** console debug qua UART. Vòng lặp `uartReceive(uart, 1, _buf, &num, 100)`, dựng dòng lệnh, hỗ trợ phím mũi tên + lịch sử lệnh, tách `argc/argv` bằng `gen_argv()`, rồi `runcmd()`.

**Mục đích:** bring-up và debug tại bàn — đọc/ghi thanh ghi, test GPIO, dump memory. Không tham gia luồng nghiệp vụ.

**Giao tiếp:** không dùng queue nào. Chỉ UART + gọi hàm trực tiếp.

---

### 4. `PwMHUEv` — `PowerControlMHUEvent` (LPPP_Task.c:290)

**Làm gì:** lớp **phiên dịch** giữa giao thức SCPI của Marvell và máy trạng thái nguồn của KM.

**Luồng:** block tại `xQueueReceive(gst_MsgQHandle_MHU, &scpi, -1)` → log `command_id`, `sender_if`, `payload_size`, `status` → `switch(command_id)`:

|SCPI nhận được|Event nội bộ phát ra|
|---|---|
|`AP_EVENT` + payload `WAKEUP_NORMAL`|`LPPP_INTERNALEVT_WAKEUP_NORMAL`|
|`AP_EVENT` + payload `GO_TO_S1`|`LPPP_INTERNALEVT_GO_TO_S1`|
|`GO_TO_S2` / `GO_TO_ERP`|_(đang bị `#if 0` — xử lý ở chỗ khác trong app_scpi.c)_|

Có kiểm tra `payload_size != sizeof(int)` thì bỏ qua.

**Giao tiếp:**

- **Nhận** `gst_MsgQHandle_MHU` (10 phần tử × `scpi_header_t`) từ AppMhuTest
- **Gửi** `gst_MsgQHandle_PowerCtrl` bằng `xQueueSend(..., portMAX_DELAY)`

⚠️ Điểm đáng lưu ý: nếu gửi lỗi thì `break` ra khỏi `while(1)` → **task chết vĩnh viễn**, hệ thống mất khả năng nhận lệnh nguồn từ AP.

---

### 5. `PwEvtMgr` — `PowerControlEventManager` (LPPP_Task.c:573)

**Làm gì:** đây là **trái tim của logic KM** — máy trạng thái điện nguồn của cả máy in.

**Luồng:** block `xQueueReceive(gst_MsgQHandle_PowerCtrl, &intr_event, -1)` → `event_handler(intr_event)`:

1. `LPPP_GetStatus(pwr_sd)` lấy trạng thái hiện tại
2. Tra bảng `fpLPPP_ScenarioFunc[state][event]` và gọi hàm chuyển trạng thái
3. `LPPP_SetStatus(pwr_sd, kết quả)`

**Ma trận 5 trạng thái × 10 sự kiện** (LPPP_Matrix.h):

|State \ Event|PWR_SW_OFF|PWR_SW_ON|GoS1|GoS2|GoErP|Wake Normal|Wake S1|Flicker|
|---|---|---|---|---|---|---|---|---|
|**PowerOFF**|–|TR0→Normal|–|–|–|–|–|–|
|**Normal**|TR1→OFF|–|TR3→Sleep1|–|–|–|–|TR2b→Normal|
|**Sleep1**|TR1→OFF|–|–|TR5/6|TR5/6|TR4→Normal|–|–|
|**Sleep2**|TR7/9→OFF|–|TR8/10→S1|–|–|–|TR8/10→S1|TR2b→Normal|
|**ErP**|TR7/9→OFF|–|TR8/10→S1|–|–|–|TR8/10→S1|TR2b→Normal|

Hai cột cuối (`LVDS_INTERRUPT`, `LVDS_TIMER`) dùng chung handler ở mọi state — điều khiển power-down 8 kênh LVDS của engine in YMCK (`LPPP_LVDS_PowerDown` ghi bit 25 vào các thanh ghi `*_ENGn_P_PIOCFG`), có chống rung 500 ms.

**Giao tiếp — 4 nguồn đẩy vào `gst_MsgQHandle_PowerCtrl` (20 phần tử × `LPPP_INTERNALEVT`):**

- `PowerControlMHUEvent` — lệnh từ AP
- `LPPP_IntIRQ_Callback` — ISR nội bộ (NCTI, NPMU, NVAL_FIQ/IRQ, MC_LPP_IRQ1), `xQueueSendFromISR`
- `LPPP_LVDS_tick` — mỗi 10 tick, `xQueueSendFromISR`
- `PowerControlIRQEvent` — poll GPIO 10 ms, **đang bị `#if 0`**

Nó cũng **gửi ngược lên AP** qua `LPPP_SendMHUMsg()` → `got_msg_qhandle` → AppMhuTest.

---

### 6. `ClrWDT` — `ClearWatchDogCounter` (stack chỉ 200 words)

**Làm gì:** hai watchdog riêng biệt trong một vòng lặp `vTaskDelayUntil(..., 500)` = **chu kỳ 5 giây**.

**A. WDT phần cứng của R4:** nếu `is_wdt_clear` thì `HalService_watchdog(0)`. Nếu `enable_wdt` bị hạ (qua SCPI) thì `HalApiWatchdog_Disable(0)` — dùng khi update firmware LPPP để WDT không reset giữa chừng.

**B. Watchdog phần mềm giám sát CA72:** đếm `ca72wdt_timer`, timeout mặc định 90 s (`CA72_WDT_TIMEOUT = 90/5`), bản short 10 s. AP phải gửi `WD_CA72_REFRESH` định kỳ. Khi hết hạn → `CA72WDT_Expire()`:

1. `LPPP_SpiFlashWrite_Ca72wdt_Reboot()` ghi log nguyên nhân vào QSPI-Flash (để sau reboot còn truy nguyên)
2. `LPPP_GPIO_Write(LPPP_GPIOPIN_HRESET_REQN, 0)` → **cưỡng bức reset CPU chính**

Tức là: nếu Linux/CA72 treo, con R4 này là thứ duy nhất còn sống và nó sẽ đá CPU chính khởi động lại.

**Giao tiếp:** không dùng queue. Chia sẻ `ca72wdt_isEnable/timer/timeout` với `AppMhuTest` (đường gọi `process_scpi_cmd` → `CA72WDT_*`) qua mutex `ca72wdt_sem`, timeout lấy sem 1 s.

---

### Bảng tổng hợp queue

|Queue|Sâu × Kiểu|Producer|Consumer|Nội dung|
|---|---|---|---|---|
|`got_msg_qhandle`|`SCPI_QUEUE_MSGS` × `int_msg_format_t`|ISR `mhu_handler`; `LPPP_SendMHUMsg`|`AppMhuTest`|channel number + `scpi_header_t` thô từ AP|
|`gst_MsgQHandle_MHU`|10 × `scpi_header_t`|`send_km_queue()` trong `process_scpi_cmd`|`PowerControlMHUEvent`|bản sao mọi lệnh SCPI cho lớp KM|
|`gst_MsgQHandle_PowerCtrl`|20 × `LPPP_INTERNALEVT`|`PwMHUEv`, ISR nội bộ, LVDS tick|`PowerControlEventManager`|mã sự kiện nguồn (0–9)|

### Ví dụ luồng đầy đủ: máy in vào Sleep1

```
Linux/CA72 gửi SCPI AP_EVENT{GO_TO_S1} qua MHU channel N
  → mhu_handler (ISR) → got_msg_qhandle
  → AppMhuTest: process_scpi_cmd()
       ├─ send_km_queue()  → gst_MsgQHandle_MHU
       └─ trả ACK qua write_MHU + poll
  → PowerControlMHUEvent: đổi thành LPPP_INTERNALEVT_GO_TO_S1
  → gst_MsgQHandle_PowerCtrl
  → PowerControlEventManager: state=Normal, event=2
  → fpLPPP_ScenarioFunc[Normal][GoS1] = LPPP_TR3_NormalToSleep1()
       → cắt nguồn LVDS/engine, hạ tần số MC, chuyển GPIO, bật proxy mạng
  → LPPP_SetStatus(pwr_sd, PWR_STATUS_SLEEP1)
```

Từ lúc này `AppMvl` mới là task làm việc chính — trả lời mDNS/SNMP thay cho máy đang ngủ.