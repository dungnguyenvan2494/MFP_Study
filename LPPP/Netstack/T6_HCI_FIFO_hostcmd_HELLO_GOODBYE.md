# T6 — HCI FIFO (net-proxy-hci) + Host-command channel + DRIVER_HELLO / GOODBYE

Gộp T6.1 + T6.2 + T6.3 (chúng khớp chặt vào nhau).

Nguồn:
`lppp_s800/Sys/memory/fw_fifo.h` (cấu trúc), `lppp_s800/Sys/fifo/sys_fifo.c` (962→674 dòng),
`lppp_s800/Hal/quartz/src/hal_fifo.c` + `hal_hostcmd.c`,
`lppp_s800/Sys/hostcmd/sys_hostcmd.{c,h}`,
`lppp_s800/Sys/system/sys_system_thread.c` (vòng lặp task `SysT`, `SysSystem_HcuIsr`, `SysSystem_handle_link_events`),
`lppp_s800/Hal/quartz/asic/linker_common/memory_sections.ld`,
`lppp_s800/Hal/quartz/asic/a0/include/{regAddrs.h:102-103, intnums.h:224}`.

---

## 1. Bản đồ bộ nhớ HCI FIFO

**FACT** (`memory_sections.ld:55-77`). Vùng `.data.nocache` neo tại `__start_ram__` = **LCM SRAM base `0xE8200000`** (MPU region NOCACHE 80KB, T1.6):

| offset (từ `0xE8200000`) | vùng | kích thước cố định | section | khớp DTS Linux |
|--------------------------|------|--------------------|---------|----------------|
| `0x0000` | USBD2 Q-HEAD | `0x0800` (2KB) | `.usbd2_qh_sec` | `usbd_qhead_lcm` |
| **`0x0800`** | **HCI FIFO (`ComFifo`)** | **`0x1880` (6272B)** | `.fifosec` | **`net-proxy-hci` lcm** |
| `0x2080` | GMAC RX/TX descriptor + pbuf | phần còn lại của 80KB | `.netbufsec` + `.bufpools` | — |

→ **HCI FIFO tại `0xE8200800`, dài `0x1880`.** Linux driver mmap đúng node DTS `net-proxy-hci` này.

---

## 2. Cấu trúc `struct communication_fifo ComFifo` (`fw_fifo.h:206-213`)

```c
struct communication_fifo {
    uint32_t status;        // FIFO_STATUS_OFF=0 / ON=1  — cờ báo driver "FIFO đã init"
    uint32_t status_ext;    // = 0xFEDCCDEF  — magic để driver nhận diện FIFO hợp lệ
    struct fifo_upload_data    up_data;     // R4 → host: result / event / status / wake
    struct fifo_upload_packets up_packets;  // R4 → host: gói Ethernet nhận được (forward)
    struct fifo_down           down_data;   // host → R4: command + app-request
};
```

Mỗi ring = `struct fifo_hdr` + `data[]`:
```c
struct fifo_hdr {
    uint32_t size_total;      // sizeof(ring) gồm header
    uint32_t read_ptr;        // ĐỊA CHỈ TUYỆT ĐỐI byte kế tiếp sẽ đọc
    uint32_t size_fifo_only;  // = FIFO_*_SIZE (chỉ vùng data[])
    uint32_t write_ptr;       // ĐỊA CHỈ TUYỆT ĐỐI byte kế tiếp sẽ ghi
};
```
- Ring buffer địa chỉ tuyệt đối; wrap bằng `ptr -= FSize` khi `ptr >= &data[FSIZE]` (`SysFifo_CopyTo` / `SysFifo_CopyFrom`, byte-by-byte).
- `write_ptr == read_ptr` ⇒ rỗng. Số byte chờ đọc / chỗ trống tính trong `SysFifo_DownDataNumberBytesPending` / `SysFifo_UpDataSpaceAvailable` / `SysFifo_UpPacketSpaceAvailable`.
- Kích thước `FIFO_UP_DATA_SIZE` / `FIFO_UP_PACKETS_SIZE` / `FIFO_DOWN_SIZE` truyền qua `-D` lúc build (phải bội số 8; tổng ≤ `0x1880 - 8 - 3×16`). **UNKNOWN**: giá trị cụ thể (trong Makefile chưa mở).

### 2.1 Datagram framing (`struct FifoDgram`, `sys_fifo.c:77-81`)
```c
struct FifoDgram {
    uint16_t DgramLen;    // = sizeof(FifoDgram)=8 + PayloadLen + padlen  (bội số 8)
    uint16_t PayloadLen;  // độ dài payload thực
    uint32_t Id;          // enum fifo_msg
};
```
Ghi 3 mảnh liên tiếp: `[FifoDgram 8B][payload][pad 0..7B]`.

### 2.2 `enum fifo_msg` (`fw_fifo.h:95-107`)

| Id | tên | hướng | ring | ý nghĩa |
|----|-----|-------|------|---------|
| 1 | `fifo_msg_cmd` | host→R4 | down | lệnh cho **base layer** (Sys) — HELLO/GOODBYE/RESET |
| 2 | `fifo_msg_result` | R4→host | up_data | kết quả của `fifo_msg_cmd` (`struct FwCmdResult`) |
| 3 | `fifo_msg_packet` | R4→host | **up_packets** | gói Ethernet forward lên driver |
| 4 | `fifo_msg_host2fwapp_req` | host→R4 | down | request cho **app layer** (`MSG_CMD_*`: pattern, proxy config, mcip…) |
| 5 | `fifo_msg_fwapp2host_resp` | R4→host | up_data | phản hồi app-layer (`SysSystem_SendResponseToHost`) |
| 6 | `fifo_msg_fwapp2host_event` | R4→host | up_data | event app-layer (`SysSystem_SendEventToHost`) |
| 7 | `fifo_msg_fwapp2host_status` | R4→host | up_data | status app→drv |
| 8 | `fifo_msg_status` | R4→host | up_data | status fw→drv (`sys_hostcmd.c:247`) |
| 9 | `fifo_msg_wake` | R4→host | up_data | chuỗi `"wake"` + `reason` + `ts` (12B) — báo "R4 vừa đánh thức host" |
| 10 | `fifo_msg_device_context` | R4→host | up_data | dump ngữ cảnh thiết bị khi RESET (`sys_hostcmd.c:273`) |

---

## 3. Đường ghi R4 → host: `SysFifo_Write(Id, Data, Len)` (`sys_fifo.c:165`)

1. Dựng `FifoDgram`, tính `padlen` để `DgramLen % 8 == 0`.
2. `Id == fifo_msg_packet` → ring **up_packets**; ngược lại → ring **up_data**.
3. Kiểm `SysFifo_Up*SpaceAvailable() >= DgramLen`; không đủ → trả `RC_ERROR` (drop).
4. Ghi 3 mảnh bằng `SysApiFifo_Write{Data,Packet}Partial(&ptr, src, len, IsFirst, IsLast)`:
   - `SysFifo_CopyTo` (byte-copy + wrap),
   - `cpu_dcache_writeback_region(...)` mỗi mảnh (dù vùng NOCACHE — **INFERENCE: thừa/an toàn kép**),
   - mảnh cuối: `HalApiFifo_dsb()` (`osm_memory_write_barrier`) → cập nhật `hdr->write_ptr` → `HalApiFifo_dsb()` → **`HalApiFifo_RaiseIrq()`**.
5. `SysApiFifo_NotifyDriver()` — nếu `write_ptr != read_ptr` ở up_data hoặc up_packets → `HalApiFifo_RaiseIrq()` lần nữa.

### 🔴 `HalApiFifo_RaiseIrq()` là **rỗng** trên quartz
`hal_fifo.c:49-54`:
```c
void HalApiFifo_RaiseIrq(void) {
// signal via ipc?
//#warning "signal to driver unfinished"
}
```
⇒ **R4 KHÔNG ngắt Linux khi có dữ liệu up**. Linux phải **poll** `ComFifo.up_data.hdr.write_ptr` / `up_packets...write_ptr` (hoặc chờ doorbell khác — ví dụ IPC0/HCU khi R4 gọi `HalApiHostCmd_set`, hoặc gói wake qua SCPI). **INFERENCE**: với S800, Linux driver poll ring pointer (hoặc dựa vào HCU doorbell riêng của command-channel). Chưa xác nhận từ phía driver.

---

## 4. Đường đọc host → R4: `SysFifo_HandlePendingRequests()` (`sys_fifo.c:112`)

Gọi **mỗi vòng** trong task `SysT` (`sys_system_thread.c:183`), không điều kiện.

```
SysFifo_Read(&Id,&Data,&Len)          // đọc 1 datagram từ down_data ring
├─ SysApiFifo_ReadDataIsPending()      // write_ptr != read_ptr ?
├─ SysApiFifo_ReadData(&dgh, 8)        // header
├─ ptr = SysApiMemory_Alloc(dgh.PayloadLen)
├─ SysApiFifo_ReadData(ptr, PayloadLen) + đọc bỏ padlen
│   (cpu_invalidate_dcache trước mỗi lần đọc)
└─ trả Id / Len / Data

switch (Id):
  fifo_msg_host2fwapp_req (4) → SysSystem_HostMsgPut(Data,Len)
        → nếu OK: KHÔNG free (đã chuyển sở hữu cho app queue)
        → phát APP_EVENT_RECEIVE_MESSAGE_FROM_SYSTEM → AppHostcmd_HandleMessageFromSystem (T5.6)
  fifo_msg_cmd (1)           → SysFifo_CmdHandler(Data,Len)      // xem §5
  Id == 0xffff              → in "drv: <chuỗi>"  (debug driver)
  khác                     → bỏ qua
free(Data)   // trừ nhánh (4) thành công
```

### 5. `SysFifo_CmdHandler` — lệnh base-layer qua FIFO (`sys_fifo.c:334`)

`Data` bắt đầu bằng `struct fw_cmd_hdr {u16 cmd; u16 len;}`. Chỉ 3 lệnh:

| `hdr->cmd` | hành động | ghi chú |
|-----------|-----------|---------|
| `HOSTCMD_HELLO` (13) | `SysHostcmd_Hello()` | → §6 |
| `HOSTCMD_GOODBYE` (14) | `SysHostcmd_Goodbye()` + `wake_message_sent = false` | → §6 |
| `HOSTCMD_RESET_COLD` (18) / `HOSTCMD_RESET` (11) | `SysHostcmd_Reset()` | `SysLan_interface_down()` + `device_context_to_host()`; nhánh `HalApiReset_Cpu(1)` bị `#if __WE_DO_NOT_RETURN` (tắt) |
| default | log "unknown msg" | |

Luôn trả `fifo_msg_result` chứa `struct FwCmdResult {fw_cmd_hdr hdr (cmd|0x8000); u32 Result}` — `Result = HalApiCpuTimer_os_tick_count()` (timestamp; comment: *"result is currently not verified in driver"*).

---

## 6. Kênh Host-command "cũ" qua HCU/EPU_DATA — **ĐÃ CHẾT trên quartz**

### 6.1 Đường HCU
- Doorbell IPC0 (T2.3): `APB_IPC0_B_BASE = 0xE8008000` (**AP sở hữu**, AP→R4), `APB_IPC0_G_BASE = 0xE8008100` (**R4 sở hữu**, R4→AP). `IPC0_REGS_t` (`IPC_ISRW +0xC`, `IPC_ICR +0x10`, `IPC_DUMMY +0x28`).
- `HAL_HOSTCMD_TRIGGER = 0x100` (bit 8). `INTNUM_HCU_WAKEUP = INTNUM_APB_IPC_B2G_1` (SPI 152).
- `HalApiIsr_Register(true, false, HAL_ISR_TYPE_HCU, SysSystem_HcuIsr)` (`sys_system_thread.c:137`).
- `SysSystem_HcuIsr()`: chỉ `SysApiThread_EventFlagsSet(SysSystem_SysEventGroup, OR, SYS_EVENT_HCU)` + `HalApiHostCmd_clear_irq_hcu()` (`Hal_clear_ipc_irq` ghi `IPC_ICR = 0x100`, kẹp 2 `IPC_DUMMY = 0`).
- `HalApiHostCmd_set()` (R4→AP): `((IPC0_REGS_t*)0xE8008100)->IPC_ISRW = 0x100`.

### 6.2 `SysHostcmd_HandlePendingCmds()` (`sys_hostcmd.c:85`) — lệnh HCU 128B
`switch(CmdCode)`: `HELLO`/`GOODBYE`/`RESET[_COLD]`(→`HalApiReset_Cpu(0)`)/`BREAKPOINT`(→`HalApiReset_Cpu(1)`)/`IDLE_LOOP`(2)/`LOW/NORMAL/HIGH_CLOCK`(→`HalApiChip_set_speed(0/1/2)`)/`SA_UPDATE_COMPLETE`·`SYS_WILL_SLEEP`·`CHECK_ALIVE`·`STANDBY`(→`Ack(OK)`)/`DUMP_RAM`.

### 🔴 Nhưng lớp HAL rỗng
`hal_hostcmd.c`:
```c
MV_RC HalApiHostcmd_Receive(uint8_t* Cmd) { return RC_ERROR; }   // luôn "không có lệnh"
void  HalApiHostCmd_Ack(HAL_HOSTCMD_ACK Ack) { }                 // no-op
void  HalApiHostCmd_Finish(void) { }                             // no-op
```
⇒ `SysHostcmd_HandlePendingCmds()` **không bao giờ thấy lệnh** (Receive luôn RC_ERROR). Toàn bộ HELLO/GOODBYE/RESET **thực tế đến qua HCI FIFO** (`fifo_msg_cmd`). HCU chỉ còn là **tín hiệu "có gì đó trong FIFO, dậy mà đọc"** (`SYS_EVENT_HCU`) + đường **wake R4 khỏi `pause_cpu`** (`PAUSE_R4_INT`, T2.5).

### 6.3 Fallback poll
`SysSystem_fool_firmware()` (cuối `vApplicationIdleHook`, `sys_system_thread.c:266`):
```c
if (SysApiFifo_ReadDataIsPending())   SysSystem_HcuIsr();
```
→ dù không có ngắt HCU, R4 vẫn tự phát hiện dữ liệu down_data mỗi lần idle.

---

## 7. `SysHostcmd_Hello` / `SysHostcmd_Goodbye` (`sys_hostcmd.c:160-208`) — bản lề chuyển thế giới

### 7.1 `SysHostcmd_Hello()` — **Linux driver lên / host thức**
1. `++hello_count`.
2. `g_wolMode = 0` (tắt WoL HW — driver tự lo).
3. `SysSystem_Notify_hello()` → phát `SYS_EVENT_DRIVER_HELLO` cho task `SysT`.
4. `SysTime_NotifySystemWake()`.
5. `print_app_layer_version()` → gửi `FW_STATUS_VERSION` (chuỗi version app+base) qua `fifo_msg_status`.
6. `HalApiHostCmd_Ack(HAL_HOSTCMD_ACK_OK)` (no-op trên quartz).

### 7.2 `SysHostcmd_Goodbye()` — **Linux driver rút / host ngủ**
1. `#ifdef MRVL_FILTER_ENABLED` → **`RedirectTraffic = false`** ⇒ từ giờ SysFilter/proxy tự xử lý gói, KHÔNG forward hết lên Linux (xem T5.1/T5.6).
2. `SysSystem_Notify_goodbye()` → phát `SYS_EVENT_DRIVER_GOODBYE`.
3. `SysTime_NotifySystemSleep()`.
4. `HalApiHostCmd_Ack(OK)`.

### 7.3 `SysSystem_handle_link_events()` xử lý tiếp (`sys_system_thread.c:329-360`)

| event | tầng Sys làm gì | rồi phát cho App |
|-------|-----------------|------------------|
| `SYS_EVENT_DRIVER_HELLO` | `hello=true; goodbye=false`; `HalApiEth_disable_irqs()` (ngừng xử lý gói ở R4) | `SysSystem_NotifyDriverSentHello()` → `APP_EVENT_DRIVER_HELLO` |
| `SYS_EVENT_DRIVER_GOODBYE` | `goodbye=true; goodbye_processing=true`; `HalApiLan_wol_detected=false`; **`SysLan_setup_gigabit_core()`** (re-init GMAC — T5.1); `SetWakeReason(Unspecified)`; `app_connect_lan_isr()` | `SysSystem_NotifyDriverSentGoodbye()` → `APP_EVENT_DRIVER_GOODBYE` |
| `SYS_EVENT_WAKE_SYSTEM` | nếu `!goodbye_processing` → **`power_cpu(0,0)`** (đánh thức A72); ngược lại đặt `wake_request=true` (trễ tới `SYS_EVENT_CAN_WAKE`) — AR.1066939 | — |
| `SYS_EVENT_CAN_WAKE` | `goodbye_processing=false`; nếu `wake_request` → `power_cpu(0,0)` | — |

`goodbye_processing` chặn wake sớm: nếu gói đánh thức đến ngay lúc đang xử lý GOODBYE, hoãn `power_cpu` tới khi ổn định.

---

## 8. Vòng lặp task `SysT` (`sys_system_thread.c:150-190`)

```c
SysApiFifo_Init();                                    // :127 — status_ext=0xFEDCCDEF, init 3 ring
HalApiIsr_Register(..., HAL_ISR_TYPE_HCU, SysSystem_HcuIsr);   // :137
while (1) {
    if (HalApiInit_DriverIsPresent())  SysSystem_System_EventsGet(&Events);   // không block
    else  SysApiThread_EventFlagsGet(SysSystem_SysEventGroup, OR_CLEAR, &Events, SYS_EVENT_ALL, FOREVER);  // block
    SysSystem_handle_link_events(Events);       // HELLO/GOODBYE/WAKE/CAN_WAKE
    SysSystem_periodic_debug_prints();
    SysHostcmd_HandlePendingCmds();             // HCU 128B — DEAD (Receive→RC_ERROR)
    SysFifo_HandlePendingRequests();            // HCI FIFO — đường lệnh THẬT
#ifndef USE_IDLE_TASK
    vApplicationIdleHook();                     // CpuSleep, clock-gating, fool_firmware
#endif
}
```
- Driver có mặt ⇒ không block (poll nhanh, xử lý gói kịp). Driver vắng ⇒ block chờ event (tiết kiệm điện).
- `vApplicationIdleHook`: khi `!DriverIsPresent` → `HalApiPowersave_disable_10tx()`, `SysSystem_CpuSleep()` (nếu cho phép), clock-gating; luôn `SysSystem_fool_firmware()` (poll FIFO fallback).

---

## 9. FACT / INFERENCE / UNKNOWN

- **FACT**: địa chỉ `0xE8200800`/size `0x1880`, layout `communication_fifo`, `status_ext=0xFEDCCDEF`, 3 ring + `fifo_hdr` con trỏ tuyệt đối, framing `FifoDgram`, bảng `enum fifo_msg`, `SysFifo_Write`/`Read`/`CmdHandler`, `HalApiFifo_RaiseIrq` rỗng, `HalApiHostcmd_Receive` trả `RC_ERROR`, `HalApiHostCmd_Ack` rỗng, IPC0 base `0xE8008000`/`0xE8008100`, `HAL_HOSTCMD_TRIGGER=0x100`, SPI 152, hostcmd id list, thân `Hello`/`Goodbye`/`handle_link_events`, `RedirectTraffic=false` trong Goodbye, `power_cpu(0,0)` trong WAKE_SYSTEM.
- **INFERENCE**: Linux driver poll ring pointer để nhận up-data (vì `RaiseIrq` rỗng) — hoặc dùng doorbell khác. Chưa đọc phía driver (`Kernel/.../net-proxy` / `mvebu` fifo driver).
- **INFERENCE**: `cpu_dcache_writeback/invalidate` trong `sys_fifo.c` là thừa vì `.fifosec` nằm trong MPU NOCACHE region (T1.6) — trừ khi có cấu hình khác cho vùng LCM. Chưa đối chiếu MPU encoding vs `0xE8200800`.
- **INFERENCE**: HELLO/GOODBYE luôn tới qua `fifo_msg_cmd` chứ không qua HCU — suy từ HAL HCU stub rỗng. Hợp lý nhưng chưa thấy log/driver xác nhận.
- **UNKNOWN**: `FIFO_UP_DATA_SIZE` / `FIFO_UP_PACKETS_SIZE` / `FIFO_DOWN_SIZE` (truyền `-D`, Makefile chưa mở) — chỉ biết tổng ≤ `0x1880`.
- **UNKNOWN**: `SysSystem_HostMsgPut` chi tiết queue (app request), `AppHostcmd_HandleMessageFromSystem` xử lý `MSG_CMD_*` nào (để dành khi đọc `Application/hostcmd/app_hostcmd.c`).
- **UNKNOWN**: `HalApiInit_DriverIsPresent()` dựa vào gì (cờ do HELLO/GOODBYE set? hay đọc register?).

---

## Tóm tắt (theo format skill)

**HCI FIFO để làm gì:**
Là kênh IPC dùng chung một struct `communication_fifo` đặt tại LCM SRAM `0xE8200800` (node DTS `net-proxy-hci`), gồm 3 ring buffer: `down_data` (Linux→R4: lệnh + request app-layer), `up_data` (R4→Linux: result/event/status/wake), `up_packets` (R4→Linux: gói Ethernet forward). Mỗi bản tin bọc trong `FifoDgram{DgramLen,PayloadLen,Id}` căn 8 byte, `Id` là `enum fifo_msg`. R4 đọc `down_data` mỗi vòng task `SysT` qua `SysFifo_HandlePendingRequests` và phân loại: `fifo_msg_cmd`→HELLO/GOODBYE/RESET (base layer), `fifo_msg_host2fwapp_req`→app layer. Kênh HCU/EPU_DATA "cũ" trên quartz đã bị vô hiệu (HAL stub rỗng); HCU giờ chỉ còn là tín hiệu "dậy đọc FIFO" (`SYS_EVENT_HCU`) và đường wake R4 khỏi `pause_cpu`. `HalApiFifo_RaiseIrq` cũng rỗng ⇒ chiều R4→Linux dựa vào Linux poll con trỏ ring. HELLO = host thức (R4 lùi lại, `HalApiEth_disable_irqs`), GOODBYE = host ngủ (`RedirectTraffic=false`, `SysLan_setup_gigabit_core`, R4 tiếp quản).

**Call flow (Linux → R4, một lệnh GOODBYE):**
```
Linux driver: ghi FifoDgram(id=fifo_msg_cmd, payload=fw_cmd_hdr{cmd=HOSTCMD_GOODBYE}) vào ComFifo.down_data
              + (doorbell IPC0 0xE8008000 bit8 → SPI152)  hoặc để R4 tự poll
R4 SPI152 → SysSystem_HcuIsr → set SYS_EVENT_HCU, clear IPC_ICR=0x100
task SysT vòng lặp:
  SysFifo_HandlePendingRequests
   └─ SysFifo_Read(down_data) → Id=1 → SysFifo_CmdHandler
        └─ case HOSTCMD_GOODBYE → SysHostcmd_Goodbye()
             ├─ RedirectTraffic = false
             ├─ SysSystem_Notify_goodbye() → SYS_EVENT_DRIVER_GOODBYE
             └─ HalApiHostCmd_Ack(OK)   [no-op]
        └─ return_result → SysFifo_Write(fifo_msg_result, FwCmdResult) vào up_data
  SysSystem_handle_link_events(SYS_EVENT_DRIVER_GOODBYE)
   ├─ goodbye=true; goodbye_processing=true
   ├─ HalApiLan_wol_detected=false; SysLan_setup_gigabit_core()   [re-init GMAC]
   ├─ SysApiSystem_SetWakeReason(Unspecified); app_connect_lan_isr()
   └─ SysSystem_NotifyDriverSentGoodbye() → APP_EVENT_DRIVER_GOODBYE
        → AppThread_HandleDriverGoodBye() (T5.6): join lại multicast, SystemIsSuspended=TRUE
```

**Call flow (R4 → Linux, forward gói + wake):**
```
low_level_input → SysFilter STAGE_2 → proxy quyết "cần host"
 └─ ApiSysNetworkWakupAndForwardPacketToDriver → SysFilter_Wakeup_and_Forward
     ├─ SysApiLan_PacketForwardToDriver → SysFifo_Write(fifo_msg_packet) → ComFifo.up_packets
     │     └─ HalApiFifo_RaiseIrq()   [RỖNG → Linux phải poll write_ptr]
     └─ set SYS_EVENT_WAKE_SYSTEM
task SysT: SysSystem_handle_link_events(SYS_EVENT_WAKE_SYSTEM)
     └─ !goodbye_processing → power_cpu(0,0)   [đánh thức A72]
(+ SysFifo_send_msg_wake once → fifo_msg_wake "wake"+reason+ts vào up_data)
```

**File:line đã tham chiếu:**
- `Sys/memory/fw_fifo.h`:95-107 (`enum fifo_msg`), :128-213 (structs)
- `Sys/fifo/sys_fifo.c`:53 (`ComFifo .fifosec`), :112-159 (`HandlePendingRequests`), :165-254 (`SysFifo_Write`), :262-302 (`SysFifo_Read`), :306-324 (`send_msg_wake`), :326-382 (`return_result`, `CmdHandler`), :390-425 (`SysApiFifo_Init`), :605-667 (space calc + copy wrap)
- `Hal/quartz/src/hal_fifo.c`:49-59 (`RaiseIrq` rỗng, `dsb`)
- `Hal/quartz/src/hal_hostcmd.c`:72-105 (`Receive` RC_ERROR, `Ack` rỗng, `Hal_clear_ipc_irq`, `HalApiHostCmd_set` `IPC_ISRW=0x100`)
- `Sys/hostcmd/sys_hostcmd.{h:46-60 (id list), c:85-155 (HandlePendingCmds), c:160-215 (Hello/Goodbye/Reset)}`
- `Sys/system/sys_system_thread.c`:127-190 (vòng lặp `SysT`), :198-240 (`vApplicationIdleHook`, `fool_firmware`), :272-280 (`SysSystem_HcuIsr`), :300-360 (`handle_link_events`)
- `Hal/quartz/asic/linker_common/memory_sections.ld`:55-77 (bố trí LCM)
- `Hal/quartz/asic/a0/include/regAddrs.h`:102-103, `intnums.h`:224

Gợi ý: tiếp theo `kernel-arch-diagram` cho sơ đồ "communication_fifo 3 ring + doorbell IPC0 + task SysT".

---

## ✅ T6 (6.1+6.2+6.3) XONG. Tự động sang **T7.2** — `ApplicationKM/LPPP_Gpio.c` (bảng GPIO KM, `LPPP_GPIO_Handle*`, ánh xạ tên → pin).
