### Chuỗi gọi dẫn tới `AppThread_Main`

`hal_main()` (Hal/quartz/asic/build/hal_main.c) → `AppInit_Initialize()` → `SysApiInit_Initialize()` → task hệ thống `SysSystem_ThreadMain()` → `AppInit_StartThreads()` → tạo thread **"AppMvl"** với entry là `AppThread_Main` (tạo với cờ `DONT_START` rồi `SysApiThread_Resume`).

Lưu ý: trước đó `init_thread()` trong hal_main đã chạy `LPPP_KM_Init()` — tạo các message queue (`gst_MsgQHandle_MHU`, `gst_MsgQHandle_PowerCtrl`), init IRQ/Flash/GPIO/I2C, đặt trạng thái POWEROFF và bắn event `PWR_SW_ON`. Nên khi `AppThread_Main` chạy thì hạ tầng KM đã sẵn sàng.

### Bên trong `AppThread_Main` (app_init.c:141)

**Giai đoạn 1 — khởi tạo tuần tự**

1. `MHU_init()` — mở kênh IPC Mailbox Unit với CPU host (AP)
2. `ApiAppInit()` — vào `Application/thread/app_thread.c:424`:
    - đặt debug mask, set app version (`"AppLayer V1.00"`)
    - `ApiSysWakeOnLinkchangeDisable()`
    - reset heap multicast IP (v4/v6)
    - set IPv4 mặc định `192.168.1.1/24`
    - đăng ký 2 periodic timer: `periodic_callback` mỗi 1000 ms, `periodic_callback2` mỗi 6000 ms
3. `TimerInit()` — timer APB của HAL

**Giai đoạn 2 — sinh 5 task FreeRTOS** (đều `tskIDLE_PRIORITY`)

|Task|Nguồn|Vai trò|
|---|---|---|
|`AppMhuTest` ("MHU")|ApplicationMrvl/Test|test/xử lý MHU|
|`cmd_thread` ("cmd thread")|ApplicationMrvl/Test|console command processor|
|`PowerControlEventManager` ("PwEvtMgr")|ApplicationKM|máy trạng thái nguồn, chạy ma trận scenario|
|`PowerControlMHUEvent` ("PwMHUEv")|ApplicationKM|nhận SCPI qua MHU → đẩy event vào queue|
|`ClearWatchDogCounter` ("ClrWDT", stack 200)|ApplicationKM|kick watchdog|

(`PowerControlIRQEvent` bị vô hiệu bằng `#if 0`.) Mỗi lần tạo đều kiểm tra `xResult != pdPASS` và in lỗi.

**Giai đoạn 3 — vòng lặp vô tận**

c

```c
while (1) {
    ApiAppMainLoop();
    SysApiThread_Relinquish(HandleMainThread);
}
```

### `ApiAppMainLoop()` — trái tim xử lý

Block chờ event bằng `ApiSysGetEventsBLock(&Events)`, rồi dispatch theo bitmask:

- `RECEIVE_PM_PACKET` → `AppThread_HandlePatternMatchedPacket()` — gói mạng khớp filter, nơi các proxy (mDNS/SNMP/LLMNR/NBNS) thực sự trả lời thay host
- `RECEIVE_MESSAGE_FROM_SYSTEM` → `AppHostcmd_HandleMessageFromSystem()` — lệnh IOCTL từ driver Linux
- `CHANGED_NETWORK_CREDENTIALS` → cập nhật IP/MAC
- `DRIVER_HELLO` / `DRIVER_GOODBYE` → host chuẩn bị ngủ / firmware nhả phần cứng (bước 2 và 4 trong readme.txt)
- `LINK_UP` / `LINK_DOWN` → sự kiện Ethernet
- `SELF_EVENT` → event nội bộ do chính app bắn

Điểm quan trọng: **luồng nghiệp vụ mạng chạy trong `AppThread_Main`**, còn **luồng điện nguồn của KM chạy song song** trong `PwEvtMgr`/`PwMHUEv` qua queue riêng — hai bên gần như độc lập, chỉ gặp nhau ở lớp HAL/MHU.