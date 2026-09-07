# LPPP — Master Index & Learning Roadmap

> Bản tổng hợp toàn bộ ghi chú hiện có trong `MFP_Study/LPPP/` + bộ prompt học theo tầng
> để phủ kín phần chưa nghiên cứu. Source: `C:\Users\PC\Documents\IT6_Kernel\lppp_s800\`
> Các repo liên quan: `atf/atf_s800`, `Kernel/K-S800`, `PS-CPU/pscpu_s800`, `BootROM/`

---

## PHẦN A — TỔNG HỢP KIẾN THỨC ĐÃ CÓ

### A.0 · LPPP là gì (mental model gốc)

**LPPP = "Proxy Firmware on R4 LPP" của Marvell** — chạy trên chip **88PA6220 (mã Quartz)**,
lõi **ARM Cortex-R4**, dùng **FreeRTOS + lwIPv6**.

Nhiệm vụ: khi Linux host ngủ (low power), R4 **tiếp quản Ethernet**, trả lời thay các gói
mạng tầm thường (ARP / mDNS / SNMP / LLMNR / NBNS…) và **chỉ đánh thức host khi thật cần**.
Khách hàng: **Konica Minolta** (thư mục `ApplicationKM`, comment tiếng Nhật) — máy in/MFP.

**SoC có 3 lõi CPU:**
| Lõi | OS | Vai trò |
|---|---|---|
| Cụm 4× Cortex-A72 (AP806, "North Bridge") | ATF + Linux | Ứng dụng chính (AP) |
| Cortex-R4 (Quartz die, "South Bridge") | FreeRTOS | **LPPP** — proxy nguồn + mạng |
| Cortex-M3 (Machine Control) | bare-metal | Điều khiển engine cơ khí máy in |

**Phân tầng từ dưới lên:** `Hal → Os → Netstack → Sys → Application`
- `Hal/quartz/` (~1000 file): cpu, clocks, i2c, uart, timer, interrupt (GIC), MHU, cdma, linker
- `Os/freertos/` + `portable/` cho Cortex-R4
- `Netstack/lwipv6/` (IPv4/IPv6), `none/`
- `Sys/`: middleware — `sdk/`, `filter/`, `fifo/`+`hostcmd/` (IPC qua SRAM FIFO), `wake_PCIE`, `wake_USBD`, `memory`, `thread`, `timer`, `MHU`, `lan`, `socket`, `jtag`, `debug`
- `Application/`: các proxy nghiệp vụ — `llmnr_proxy`, `nbns_proxy`, `simple_mdns_proxy`, `snmp_proxy`, `wake_service_proxy`, `hostcmd`, `thread`
- `ApplicationMrvl/`: code test/bring-up của Marvell — `Test/` (app_MHU, app_scpi, app_cmdproc, app_gpio), `init/`
- `ApplicationKM/`: lớp tuỳ biến khách hàng — `LPPP_Task`, `LPPP_Matrix`, `LPPP_SCPI`, `LPPP_Flash`, `LPPP_Gpio`, `LPPP_I2C`, `LPPP_QoS`, `LPPP_McChangeFreq`, `LPPP_McWDT`, `LPPP_Statlib`, `LPPP_WakeInfo`
- `lib/libpmu_r4.a` (binary) — PMU state machine
- **Build:** `./build_lppp.sh <board>` → `mvri.bin` (≤ 2 MB). 12 board: `es, emu, egl, eglz, dnbmlk, spa, eglb, eglbz, eglzp, eglbzp, hemlk, mssb`

### A.1 · Boot flow (ĐÃ CÓ — `Flow LPPP.md`, `hal_main_initialize_sections.md`, `copy_run_atf.md`)

**`main()` là luồng boot bare-metal của R4** — 7 giai đoạn:
1. **Dựng runtime C** — `hal_main_initialize_sections()`: chép `.data` từ flash (LMA) → LCM (VMA)
   bằng `fast_copy_section`, xoá `.bss` bằng `fast_fill_section`. Gác sau **cờ hằng số vá được
   bằng hex editor** (`initialize_sections[] = "5=..."`, bit0=chép .data, bit2=xoá .bss).
   `.data.nv` (`.nvramsec`) được miễn trừ nhờ vị trí trong linker script → sống qua reset mềm.
2. **Đưa lõi vào trạng thái làm việc** — `hwConfigInit()`, `hwSetProcSpeed(12MHz)` (chỉ ghi sổ
   cho `cpu_spin_delay`, KHÔNG đụng PLL), `cpu_init()` (MPU 5 vùng, D-cache), `pad_config_init()`
3. **Có tiếng nói** — `setup_debug_print_logic()` (UART tối giản, driver đầy đủ bị loại vì tốn 64KB/256KB RAM)
4. **Lên nguồn & clock** — `pmu_phase(pmu_baseline)`: SYS_PLL/MC_PLL/ND_PLL, `hwSetProcSpeed(200MHz)`,
   `avs_set_from_efuse(1097)` (hiệu chỉnh điện áp theo lô chip)
5. **Bật ngoại vi cốt lõi** — `intInit()` (GIC), `LPPP_McChangeFreq_init()`, `LPPP_McWDT_init()`,
   `init_MHU()` (RỖNG), `gpio_init()`, `AppPrintInit()`
6. **★ Đánh thức AP806** — `early_board_init(false)`: `turn_on_qspi`, `mci_init`+`iris_init`,
   `init_ddr(false)`, `copy_run_atf(0xF4000000)` ══► cụm A72 bắt đầu chạy ATF
7. **Trao quyền RTOS** — `AppInit_Initialize()` → `SysApiInit_Initialize()` → `SysApi_InitApplModules()`
   → `vTaskStartScheduler()` (KHÔNG return) → `while(1)` lưới an toàn

**`AppInit_Initialize`:** chọn use-case SDK (thực chất `cfg = NULL`, tham số không dùng),
công tắc `set_speed[]` (chết 2 lần: byte đầu `\x00` + `HalApiChip_set_speed()` rỗng trên Quartz).

**`SysApi_InitApplModules`:** bản kê khai module tầng System — `SysMemory_Init` (2 heap: 23KB
SYS cố định + phần dư APP; module đầu nuốt sạch, chữ ký `(&mem,&memsize)` là di sản ThreadX chết),
`SysSystem_Init` (mutex, event group, task `SysSystem_ThreadMain` prio 1), `power_up_system()`
(tạo `init_thread` prio 3), `SysFilter/SysSocket/SysLan/SysTime_Init`, `lpp_usbd_init`,
`wake_pcie_init`, kiểm `FW_MEMTABLE_ID`, `HalApiIsr_Unmask(TIM)` (bật tick — việc cuối).
9 lần `while(1)` không lối thoát nếu lỗi.

**`init_thread` (prio 3, cao nhất — chạy một mạch, không block):** pha 2 dựng nền tảng —
2 mutex (`cpu_power_mutex`, `pmu_transition_mutex`), `LPPP_KM_Init()` (3 queue + 6 IRQ + flash +
GPIO + I2C + `event_handler(PWR_SW_ON)` → NORMAL), `get_ddr_memory_size()`/`write_ddr_memory_size()`
(đọc strap GPIO bank H → cất vào thanh ghi timer2), `pmu_phase(pmu_ap_link_up)` (bật nguồn AP806,
KHÔNG bật core), `pmu_phase(pmu_system_up)` (bảng `power_up_dev[]`), `board_init(false)`
(`iris_detection`, `init_ddr`, **`copy_run_atf`**), `LPPP_QoS_Init/MoChi/UL/AP`, `vTaskDelete(NULL)`.

**`copy_run_atf(header_loc)`:** điểm bàn giao sang A72 — đọc `code_header_t` (magic `0xB105B002`),
`cpu_disable_dcache()`, `start_ap806(false)` (PRCR_0=0 giữ reset, có WDT 500ms làm lưới an toàn
vì reset AP806 có thể treo bus MoChi), `memcpy(DDR + load_addr, header + prolog_size, boot_image_size)`
(**KHÔNG kiểm checksum**), `RVBAR_0 = load_addr >> 16` (đơn vị 64KB), `WARMBOOT_FLAG = 0` (cold boot),
`start_ap806(true)` (PRCR_0 = 0x10001 — **A72 chạy**), `cpu_enable_dcache()`. Return ngay, không handshake.
Thất bại → in 1 dòng rồi return, AP806 nằm reset mãi mãi, R4 vẫn chạy bình thường.

### A.2 · Task model (ĐÃ CÓ — `Phân tích task.md`, `AppThread_Main.md`)

Sau scheduler: **6 task đồng thời** (`AppThread_Main` prio 1 tạo 5 task con ở `tskIDLE_PRIORITY=0`):

| Task | Nguồn | Vai trò | Giao tiếp |
|---|---|---|---|
| `AppMvl` (`AppThread_Main`) | ApplicationMrvl/init | Xử lý mạng — proxy trả lời thay host | **event-group** (không queue) |
| `AppMhuTest` ("MHU") | Test/app_MHU.c | **Cổng SCPI duy nhất** CA72↔LPPP | `got_msg_qhandle` → `gst_MsgQHandle_MHU` |
| `cmd_thread` ("cmd thread") | Test/app_cmdproc.c | Console debug UART | không queue |
| `PowerControlMHUEvent` ("PwMHUEv") | ApplicationKM | Dịch SCPI → internal event | `MHU` → `PowerCtrl` |
| `PowerControlEventManager` ("PwEvtMgr") | ApplicationKM | **State machine nguồn** — trái tim KM | consumer `gst_MsgQHandle_PowerCtrl` |
| `ClearWatchDogCounter` ("ClrWDT", stack 200) | ApplicationKM | 2 watchdog (HW R4 + SW giám sát CA72 90s) | mutex `ca72wdt_sem` |
| ~~`PowerControlIRQEvent`~~ | — | `#if 0` — không tạo | — |

**Hai thế giới song song:** luồng mạng (`AppMvl`, event-group của Sys) ≈ tách rời hoàn toàn với
cụm task KM (queue FreeRTOS); chỉ gặp nhau ở lớp HAL/MHU.

**`AppThread_Main`:** `MHU_init()` (RỖNG), `ApiAppInit()` (IPv4 mặc định 192.168.1.1/24,
2 periodic timer 1000ms + 6000ms), `TimerInit()`, tạo 5 task, rồi vòng lặp `ApiAppMainLoop()` →
block `ApiSysGetEventsBLock()` → dispatch: `RECEIVE_PM_PACKET` (proxy trả lời), `DRIVER_HELLO/GOODBYE`
(bàn giao Ethernet), `RECEIVE_MESSAGE_FROM_SYSTEM` (IOCTL qua ComFifo SRAM), `LINK_UP/DOWN`, `SELF_EVENT`.

### A.3 · SCPI & MHU — trục CA72↔LPPP (ĐÃ CÓ — `AppMhuTest.md`, đầy đủ nhất)

**MHU = Message Handling Unit** tại `0xE8009800` — chỉ truyền "tiếng chuông" (1 bit/kênh),
**KHÔNG truyền dữ liệu**. Dữ liệu nằm trong **shared memory DDR** `0x7FFF0000` (`PLAT_CSS_SCP_COM_SHARED_MEM_BASE`).
- Layout: 5 kênh, mỗi kênh 256B = 2 buffer 128B xen kẽ (SCP→AP tại `2n×128`, AP→SCP tại `(2n+1)×128`)
- `scpi_header_t` = 8B header (`command_id, sender_if, payload_size, status`) + payload
- 4 macro shared-memory **phải khớp** `plat/marvell/a8k/quartz` trong ATF và `scp-shmem` trong Linux DTB — không có kiểm tra runtime
- Ranh giới thật giữa 2 nhân = **D-cache của R4** (không coherent với AP) → `mhu_handler` phải
  `cpu_dcache_invalidate_region` (2 bước: 32B header trước, rồi phần payload), `process_scpi_cmd`
  phải `cpu_dcache_writeback_region(header_dest, 256)` khi có response

**Luồng 1 lệnh SCPI qua 3 tầng queue:**
```
mhu_handler [ISR]  → invalidate cache, memcpy, clear_MHU, xQueueSendFromISR → got_msg_qhandle (16 slot)
AppMhuTest [task·0] → xQueueReceive → process_scpi_cmd(chan, &header)
   process_scpi_cmd  → send_km_queue(s_header)  ← VÔ ĐIỀU KIỆN, dòng 698, TRƯỚC switch
                        → xQueueSend → gst_MsgQHandle_MHU (10 × 12B — chỉ header + payload[0])
                     → switch (~28 case)
                     → nếu send_response: writeback cache, return true
AppMhuTest  → write_MHU(OUTPUT_PORT, 1<<chan) → ⚠ BUSY-WAIT KHÔNG TIMEOUT chờ AP clear bit
PwMHUEv [task·0]  → xQueueReceive(MHU) → lọc command_id → xQueueSend → gst_MsgQHandle_PowerCtrl (20 slot)
PwEvtMgr [task·0] → xQueueReceive(PowerCtrl) → event_handler()
```

**~28 lệnh SCPI** (2 bộ ghép): ARM chuẩn `0x02`–`0x80` + KM mở rộng `0x81`–`0x8C`.
Chỉ **3 lệnh chạm state machine nguồn:**
- `0x03` Set CSS power state → `scpi_set_power_state` → `power_cpu` → `GO_TO_S2`/`GO_TO_ERP`/`WAKEUP_S1`
  (do `g_wolMode` quyết định, không phải Linux)
- `0x88` AP event → `send_km_queue`→`PwMHUEv` → chỉ payload 0 (`WAKEUP_NORMAL`) và 1 (`GO_TO_S1`) hoạt động;
  payload 2/3 (S2/ERP) **vô tác dụng** (biến `intr_event` static bị `power_cpu` ghi đè + `#if 0` ở PwMHUEv)
- `0x85` Set power status → `LPPP_SetStatus(pwr_sd)` — **cửa hậu**, ép thẳng state, bỏ qua ma trận

**Lệnh có side effect ẩn sau tên:** `0x8A` Get PMU event → **gỡ vĩnh viễn ISR nút nguồn chính**
(bàn giao nút nguồn từ LPPP sang AP); `0x87` Get wake info → **xoá cờ sau khi đọc**; `0x0a` DVFS set
domain=0 op=1 → **`pause_cpu()` đưa R4 vào `wfi`** (dùng khi Linux ghi QSPI).

**Rủi ro lớn nhất:** busy-wait không timeout ở `AppMhuTest` chờ AP ACK — CA72 treo → queue đầy →
mất lệnh im lặng → chỉ thoát nhờ CA72WDT kéo `HRESET_REQN=0` sau 90s. `SCPI_BLOCK_INTR` (chế độ
backpressure) đang TẮT; nhóm phát triển chọn tăng queue 4→16 (bug `AR.1067928`).

### A.4 · State machine nguồn KM (ĐÃ CÓ — `PowerControlEventManager.md`, đầy đủ nhất)

**`PowerControlEventManager` = single consumer** của `gst_MsgQHandle_PowerCtrl`.
`configTICK_RATE_HZ=100` → 1 tick = 10ms; `configMAX_PRIORITIES=4`; stack 300 words = 1200B.

**5 state × 10 event → ma trận `fpLPPP_ScenarioFunc[state][event]`** (`LPPP_Matrix.h`):

| State \ Event | SW_OFF | SW_ON | GoS1 | GoS2 | GoErP | Wake_Norm | Wake_S1 | Flicker |
|---|---|---|---|---|---|---|---|---|
| **POWEROFF** | – | **TR0→NORMAL** | – | – | – | – | – | – |
| **NORMAL** | TR1→OFF | – | **TR3→SLEEP1** | – | – | – | – | TR2b→NORMAL |
| **SLEEP1** | TR1→OFF | – | – | TR5/6→SLEEP2 | TR5/6→ErP | **TR4→NORMAL** | – | – |
| **SLEEP2** | TR7/9→OFF | – | TR8/10→SLEEP1 | – | – | – | TR8/10→SLEEP1 | TR2b→NORMAL |
| **ErP** | TR7/9→OFF | – | TR8/10→SLEEP1 | – | – | – | TR8/10→SLEEP1 | TR2b→NORMAL |

2 cột `LVDS_INTERRUPT`/`LVDS_TIMER`: cùng handler mọi state, **trả về state không đổi** — máy trạng
thái con độc lập (power-down 8 kênh LVDS Y/M/C/K × ENG0/1, chống chattering 500ms).

**Transition đáng nhớ:**
- **TR0** (`LPPP_TR0_OFFToNormal`): busy-wait `while(tick < 10)` so với **tick tuyệt đối từ boot**
  (chặn hệ thống ≤100ms), rồi bật `MOTION_EN_MC`(71)/`MOTION_EN_IR`(72) nếu Stop không nhấn
- **TR1** / **TR7/9** (→ OFF): **KHÔNG BAO GIỜ được gọi** — `PWR_SW_OFF` không có producer nào;
  tắt máy thật đi qua `LPPP_GPIO_CallbackMainSW` kéo thẳng `HRESET_REQN=0` từ ISR
- **TR2b** (`LPPP_TR2b_Sleep2_Or_ErPToNormal`): mất điện thoáng qua — nếu `ap_sd == AP_STATUS_NO_BOOT`
  thì ghi cờ `0x10` vào 2 vùng QSPI redundant (`0xF4377007`/`0xF4378007`, U-Boot đọc) rồi reset cứng CA72
- **TR3** (→SLEEP1): `gpio_set_func_sel(33/34, 0)` cắt I2S; **TR4** (→NORMAL): khôi phục + `LPPP_QoS_SetupUL`
- **TR8/10** (Sleep2/ErP→Sleep1) — **nặng nhất:** I2C tới PS-CPU (log event 56), ghi flash B014 bit13,
  `LPPP_QoS_SetupMoChi` (10 reg) + `LPPP_QoS_SetupAP` (~15 reg) + ~25 dòng debug print. Phải nạp lại
  QoS vì domain MCix4/PIDI/CCU mất điện khi vào Sleep2/ErP.

**`ap_sd` = biến ghép nối duy nhất giữa TR5/6 (đặt NO_BOOT) và TR2b (đọc).** SCPI `0x83`
`SET_AP_STATUS` = "Linux nói tao boot xong, đừng reset tao".

**5 nguồn sinh event** (`Phần 8` trong note): GPIO ISR (`PWR_FLICKER`/`LVDS_INTERRUPT` — chỉ 2 chân
`POWER_MONITOR`+`XEN_LVDS_ENG` được attach, phần còn lại dead code), timer ISR (`LVDS_TIMER`),
`PwMHUEv` (`WAKEUP_NORMAL`/`GO_TO_S1`), `power_cpu()` (`GO_TO_S2`/`GO_TO_ERP`/`WAKEUP_S1`),
`LPPP_IntIRQ_Callback` (⚠ **producer hỏng** — `LPPP_AnalyzeIRQEvent()` không có `return`).
`PWR_SW_ON` chỉ do `LPPP_KM_Init` gọi thẳng `event_handler()` 1 lần lúc boot, không qua queue.

### A.5 · power_cpu / scpi_set_power_state (ĐÃ CÓ — 2 note riêng)

**`scpi_set_power_state`** = adapter mỏng: giải mã 1 `uint32_t` payload
(`cpuid[1:0], clusterid[7:4], cpupower[11:8]` — 3=tắt, khác=bật) → `power_cpu(cpuid | clusterid<<2, cpupower)`.
Bỏ qua `clusterpower`/`suspend` (firmware tự suy ra từ `cpu_state`). Macro `PWRC_CPUN_CR_REG`
đúng nhờ trùng hợp với 4 id hợp lệ `{0,1,4,5}`.

**`power_cpu`** = tầng thấp nhất, toàn hàm trong `cpu_power_mutex`. `cpu_state`: bit=1 nghĩa
**core ĐANG TẮT**, `ALL_CPU_OFF=0xF`, khởi tạo `0xe`.
- **Nhánh tắt** (`power==3`): chờ WFI (⚠ **sai độ ưu tiên toán tử** `!(*reg) & mask` — với cpu==1 vòng
  lặp không chạy lần nào → phải bù bằng ~14 `udelay` rải khắp, bản vá OP_BTS-20790), Isolation enable
  (bit16), PWR_DN_RQ **clear** để tắt (tên trái nghĩa), PRCR=Reset. Khi core cuối tắt (`cpu_state==0xF`):
  `WARMBOOT_FLAG` ≠ 0, `pmu_phase(pmu_suspend)`, phát `GO_TO_ERP` (nếu `g_wolMode`) hoặc `GO_TO_S2`, `CA72WDT_End()`
- **Nhánh bật** (`power!=3`): khi core đầu bật (`cpu_state==0xF`): `CA72WDT_Start()`,
  `Bregister_modify(B014_BEGIN_PHASE6)` (**ghi flash trên critical path resume**), `pmu_phase(pmu_resume)`
  (hỏng → `return false`, để lại trạng thái nửa vời — chỉ CA72WDT 90s cứu được), phát `WAKEUP_S1`,
  `RVBAR_0/1 = WARM_BOOT_ADDR` (0x1003 = 0x10030000), `intDisable(MCIX2_1)` + `clear_wake_ints()` +
  `wake_lan()` (định tuyến ngắt GMAC qua IRIS ICU về AP) + `SysApiSystem_Wake()`, rồi PWR_DN_RQ↑ → ISO↓ → PRCR=nReset

`g_wolMode` đến từ host command `MSG_CMD_SYSTEM_SET_WOL` của network stack, **không** từ SCPI.

### A.6 · Wake timer & wake_system (ĐÃ CÓ — `waker timer.md`, `wake_system.md`)

**SCPI `0x06`** = đồng hồ báo thức duy nhất để hệ thống tự hẹn đánh thức khi CA72 tắt hẳn.
`wake_timer = TimerOpen(-1)` → `0xFF` = `RANDOM_AO_TIMER` (bank **always-on**, không thuộc power
island — nếu không sẽ chết cùng hệ thống). `RepCount=0` (one-shot), `eTimebase = e_TIMEBASE_10_MS`,
`Count` = payload word 0 (bỏ 32 bit cao — với 10ms đủ 497 ngày). Không trả response.
Khi nổ → `my_callback` → `PLAT_WARMBOOT_FLAG = 0` (**ép cold boot** — khác nguồn wake khác),
`TimerOff/Close`, `wake_system()` → `SYS_EVENT_WAKE_SYSTEM`.
`case 0x07` = cancel (allocate/free pair). ⚠ Đặt 2 lần không xen `0x07` → **rò rỉ timer**, chỉ có 2 timer khả dụng.

**`wake_system()` → `SysApiThread_EventFlagsSet(SysSystem_SysEventGroup, _OR, SYS_EVENT_WAKE_SYSTEM)`**
— ⚠ **bất đồng bộ**: `xEventGroupSetBitsFromISR` chỉ đẩy yêu cầu vào **timer command queue** (30 slot);
bit thật được set sau đó bởi **Tmr Svc daemon (prio 3, cao nhất)** → `xEventGroupSetBits`.
`Option` (`_AND`/`_OR`) bị bỏ qua hoàn toàn, luôn OR. `debug_event_set_fails` đếm queue đầy
(chỉ đọc qua JTAG). Nếu `configUSE_TIMERS=0` / thiếu heap cho timer task → **cả hệ thống event chết câm lặng**.

### A.7 · SysSystem_ThreadMain (ĐÃ CÓ — `SysSystem_ThreadMain.md`)

**Consumer duy nhất** của `SysSystem_SysEventGroup`, prio 1 (> mọi task KM). Vòng lặp: lấy event
(2 chế độ — driver present → poll không block; không → block vô hạn `OR_CLEAR`), rồi
`SysSystem_handle_link_events(Events)` (dispatch tuần tự, không `else`), rồi **3 bước vô điều kiện**
(`periodic_debug_prints`, `SysHostcmd_HandlePendingCmds`, `SysFifo_HandlePendingRequests`).

Dispatch: `SYS_EVENT_HCU` (**chỉ log** — chuông đánh thức, việc thật ở `HandlePendingCmds`),
`WAKE_SYSTEM` → `power_cpu(0,0)`, `CAN_WAKE` (gỡ chốt `goodbye_processing`), `DRIVER_HELLO/GOODBYE`
(bàn giao mạng), `LINK_UP/DOWN`, `WOL_DETECTED` → **tự gửi lại `WAKE_SYSTEM`** (cần 2 vòng lặp).
Khối cuối **level-triggered** không gắn cờ: `LinkIsUp && goodbye && !enable_packet_processing`.
⚠ `SYS_EVENT_GE_INIT (0x400)` không nằm trong `SYS_EVENT_ALL` → cờ chết vĩnh viễn nếu ai set.

### A.8 · Update FW LPPP (ĐÃ CÓ — `Update_FW_LPPP.md`)

Firmware LPPP (`mvri.bin`) ở **QSPI chip-select 1, offset 0, 2MB** (`0xF8000000`, partition
`lppp-spi-firmware`). Linux **ghi thẳng vào MTD partition**, KHÔNG "nạp vào RAM".

**Cơ chế then chốt:** R4 và CA72 dùng chung 1 bộ điều khiển BOOTSPI (`0xE8273000`). Driver MTD
hook `prepare`/`unprepare` → `disable_clk_r4()` → **SCPI `0x0a {domain=0, idx=1}`** → LPPP
`pause_cpu()` (lưu + tắt toàn bộ GIC, chừa 1 ngắt MHU, **tự trả response TRƯỚC `wfi`**) → `usleep(1ms)`
→ Linux độc chiếm controller ghi/xoá → `0x0a {0,0}` đánh thức. Loại trừ = **thoả thuận phần mềm**,
không semaphore HW (bug `OP_BTS-28170/28498` — "LPPP フリーズ").

**3 lớp chặn ghi:** `is_write_protect_lpddr4param` mở theo `c_BootType` ∈ {2,3,5,9,11,12}
(chế độ update) hoặc `echo 0 > /sys/.../write_protect_lpddr4param`. **Không A/B partition** —
mất điện giữa chừng = LPPP không boot = máy chết cứng.

**SCPI `0x89 REWRITE_REQUEST` = code chết** (từng định nghĩa `rewrite_roop()` chạy từ RAM, đã bỏ).
**LPPP tự ghi flash** (cờ mất điện, B014, log CA72WDT) qua `flash_modify_sub()`: sao `LPPP_Flash.o`
sang `.qspi_ram_code`, bộ ba cache (writeback D / invalidate I / isb), tắt ngắt toàn cục.

### A.9 · Core subsystems

**I2C (ĐÃ CÓ — `I2C.md`):** IP Synopsys DesignWare APB, 4 instance `0xE8006000 + n×0x800`.
**2 driver song song:** Marvell (`Hal/quartz/i2c`, chỉ MCP4542) + KM (`ApplicationKM/LPPP_I2C.c`,
port từ Linux `i2c-designware`, thay ISR bằng polling). FIFO + FSM phần cứng, không bit-bang
(`IC_TAR` địa chỉ, `IC_DATA_CMD` cửa sổ 2 chiều: bit8=R/W, bit9=STOP, bit10=RESTART).
**I2C0 (0x3E) = PS-CPU** (dây thần kinh với mạch nguồn: `0x68` trạng thái nguồn, `0x6C` SLEEP_STATUS_REM,
`0x74/0x78` MC_PWR_EN, `0xC0` log sự kiện). **I2C1 (0x2D) = MCP4542** (chiết áp số, chỉnh điện áp IO 1 lần).
I2C2/3 khởi tạo nhưng không thiết bị. ⚠ `KM_vTaskDelay()` là busy-wait 1ms (gốc Linux `udelay(5)` → **×200**).

**Timer (ĐÃ CÓ — `Timer-Introduction.md`):** 2 hệ — APB timers (`0xE8000800`, 4 timer, GIC SPI 38–41)
+ CPU cycle counter (FreeRTOS tick). Chỉ Timer2/3 khả dụng (`AVAIL_TIMER=2`). `wake_timer` là khách
hàng duy nhất. Đếm lên 32-bit, timebase từ khối **TIMEBASE2** (`0xE8001800`, pulse train 1µs–100ms,
ref 25MHz — ⚠ `TIMEBASE2->TCR` **không được firmware ghi**, dựa vào giá trị reset). Interrupt
level-sensitive, ack = write-1-to-`TIAR`. `ASSERT()` rỗng toàn cục (`osm.h:142`).

**Memory (ĐÃ CÓ — `Memory.md`):** R4 **XIP trực tiếp từ QSPI `0xF8000000`** + **256KB LCM `0xE8200000`**
làm toàn bộ RW. **MPU 5 vùng** (vùng số cao thắng): 4GB NOCACHE nền, 1GB DEVICE `0xC0000000`,
256KB LCM WBWA, 128MB flash ROCACHE, **80KB đầu LCM NOCACHE** (mask `0xE000` = 5/8 subregion).
80KB đó = vùng DMA GMAC + **cửa sổ chia sẻ với Linux** (USB qhead 2KB `0xE8200000` + HCI FIFO 6.1KB
`0xE8200800` — kênh IPC thứ 2). Heap: **2 heap** (`heap_mrvl.c` = `heap_4` nhân đôi), `pvPortMalloc`
FreeRTOS → SYSTEM 23KB. `sys_memtable.c` = **bảng danh bạ hằng số trong flash** (magic `'MeMt'`)
cho driver Linux/debugger tìm print buffer / version / capability mà không cần rebuild.

**CDMA (ĐÃ CÓ — `CDMA.md`):** Central DMA `0xE8271000`, 10 channel × 0x100. Descriptor 4 trường
64-bit scatter-gather, 3 mode (m2m/m2p/p2m qua `CFG.FLOWCTRL`), 35 peripheral request line.
Thủ thuật cross-boundary: channel >15 thuộc DMA của lõi M3 → descriptor sao vào LCM M3 `0xEE02FE00`.
**⇒ TOÀN BỘ FOLDER LÀ CODE CHẾT** (không trong `TL_ARFLAGS`, `cdma_config_t` không tồn tại, phụ thuộc
`UTF_*` của unit test framework). DMA thật: GMAC riêng, QSPI/I2C polling, MHU memcpy, CRC32 DDR qua
**XOR engine của AP806** (`mv_lpddr4_sleep.c` — "using XOR to bypass MPU").

**Iris (ĐÃ CÓ — `iris.md`):** **die I/O rời của Marvell** (vai trò như CP110 trong Armada 8K),
nối qua MoChi x2. Tối đa 3 con: `IRIS_SB_0` (phía Quartz/R4), `IRIS_NB_0/1` (phía AP806). Chứa
GbE/USB/PCIe/SATA + COMPHY SerDes 2 lane + PLL riêng + **ICU** (Interrupt Consolidation Unit).
Nằm ngoài dải PMU (`≥ pmu_device_end=73`) → `case 0x1b` phải dùng `pmu_iris_actions()`. `iris_detection()`
đọc PIO `0xC0042008`. **Card mạng vật lý nằm trong Iris** — ICU chuyển giao quyền sở hữu GMAC giữa R4↔CA72.

---

## PHẦN B — BỘ PROMPT HỌC THEO TẦNG

> Cách dùng: mở Claude Code trong thư mục `IT6_Kernel`, paste từng prompt. Skill
> `kernel-source-explorer` sẽ tự kích hoạt. Ưu tiên làm theo thứ tự tầng; trong mỗi tầng làm từ trên xuống.
> `[✓]` = đã có note · `[~]` = có một phần · `[ ]` = chưa có

### TIER 0 — Bức tranh tổng thể

- `[✓]` T0.1 — *"Đọc `lppp_s800/ApplicationMrvl/readme.txt`, `Application/readme.txt`, `build_lppp.sh`,
  toàn bộ `Makefile-cfg`. Vẽ sơ đồ khối 5 tầng Hal→Os→Netstack→Sys→Application, chỉ rõ module nào
  build cho board `eglz`, module nào bị loại. Liệt kê 12 board config và điểm khác biệt."*
- `[ ]` T0.2 — *"So sánh 3 lõi CPU của SoC S800 (A72/AP806, R4/Quartz, M3/MachineControl): mỗi lõi
  chạy gì, ở repo nào (`lppp_s800` / `atf_s800` / `Kernel/K-S800` / `pscpu_s800` / `Subset/S-S800`),
  giao tiếp với nhau qua kênh nào (MHU/SCPI, HCI FIFO, I2C, MoChi). Vẽ sơ đồ liên lạc liên lõi."*
- `[ ]` T0.3 — *"Lập bản đồ địa chỉ vật lý toàn SoC từ `Hal/quartz/asic/a0/include/regAddrs.h`:
  các dải `0xC0000000` (device), `0xCE000000` (AP806 config), `0xD0000000` (Iris/MoChi),
  `0xE8000000` (APB peripheral), `0xF4000000`/`0xF8000000` (QSPI CS0/CS1), `0x7FFF0000` (SCPI shmem),
  `0x00000000` (DDR nhìn từ R4). Đối chiếu với MPU config."*

### TIER 1 — Boot & bàn giao (đã phủ ~80%)

- `[✓]` T1.1 — boot flow `main()` 7 giai đoạn (có `Flow LPPP.md`)
- `[✓]` T1.2 — `hal_main_initialize_sections`, `copy_run_atf`
- `[ ]` T1.3 — *"Đọc `Hal/quartz/asic/build/hal_init_gnu.asm` (section `.reset`) và linker script
  `Hal/quartz/asic/linker_common/*.ld`. Trace từ vector reset đến lúc gọi `main()`: dựng stack 5 chế
  độ ARM, cấu hình SCTLR, MPU tạm, nhảy vào C. `DumpFaultRegisters`/`ResetEntry2` xử lý fault thế nào?"*
- `[ ]` T1.4 — *"Đọc `early_board_init()` và `board.c` trong `Hal/quartz/asic/build/`. Phân biệt
  `board_init_palladium` / `board_init_toc` / `board_init_emu800` / `board_init_eseval`. Vai trò
  `turn_on_qspi`, `set_chip_select`, `mci_init`, `pidi_init`, `setup_a2_primary_windows` /
  `setup_a2_io_windows` (cửa sổ địa chỉ A2 bus). Tham số `resume` đổi luồng ra sao."*
- `[~]` T1.5 — *"Trace toàn bộ chuỗi `AppInit_Initialize` → `SysApiInit_Initialize` →
  `SysApi_InitApplModules` → `vTaskStartScheduler` từ `Sys/init/sys_init.c` và `Application/init`.
  Thứ tự tạo task, priority, ai tạo ai. Cơ chế `SYS_INIT_USECASE` (SDK/ECMA/mDNS) — có được implement không."*
- `[ ]` T1.6 — *"Đọc `Hal/quartz/asic/hardware/devices/config/cortex_r4_mpu_config.c` đầy đủ +
  `cpu_init()` / `cpu_initialize_mpu()`. Giải thích từng vùng MPU, cơ chế subregion mask, quan hệ
  với linker `__nocache_start__/__nocache_end__`, và vì sao 80KB đầu LCM phải NOCACHE."*

### TIER 2 — SCPI & MHU (đã phủ ~85%, thiếu phía HW & phía đối tác)

- `[✓]` T2.1 — `AppMhuTest`, `process_scpi_cmd`, 28 lệnh SCPI (note đầy đủ nhất)
- `[✓]` T2.2 — `PowerControlMHUEvent` (trong `Phân tích task.md` + `PowerControlEventManager.md`)
- `[ ]` T2.3 — *"Đọc `Hal/quartz/MHU/` (`hal_MHU.c`, `mhu.c`) và `Sys/MHU/`. Bản đồ thanh ghi MHU
  `0xE8009800` đầy đủ: `SCPI_TX/RX_INT_STATUS/SET/CLEAR`. `INPUT_PORT`/`OUTPUT_PORT` map sang block
  nào. `get_mhu_intnum()` → `INTNUM_APB_SCPI_TX`. Vì sao `init_MHU()` rỗng."*
- `[ ]` T2.4 — *"Đọc `atf_s800/plat/marvell/a8k/quartz/` — phía ATF của SCPI. Đối chiếu 4 macro
  shared-memory (`PLAT_CSS_SCP_COM_SHARED_MEM_BASE`, `SCPI_CHANNEL_BUFFER_SIZE`, layout kênh) giữa
  ATF ↔ LPPP `scpi_api.h` ↔ Linux DTB `scp-shmem`. ATF gửi/nhận SCPI khi nào trong luồng PSCI."*
- `[ ]` T2.5 — *"Đọc phía Linux: `Kernel/K-S800/Src/drivers/firmware/arm_scpi.c` + `km_scpi.c` +
  `drivers/.../mdels800_pmu.c`. Map mọi lệnh SCPI Linux gửi (`scpi_ops`, `km_scpi_ops`) sang case
  trong `process_scpi_cmd`. Sysfs nào trigger lệnh nào (`LPPP_REWRITE_REQUEST`, `CA72WDT`, …)."*
- `[ ]` T2.6 — *"Đọc `ApplicationKM/LPPP_SCPI.c` + `LPPP_SCPI_Protocol.h` đầy đủ. Bộ lệnh KM
  `0x81`–`0x8C`: `SCPISetAPStatus`, `SCPISetPowerStatus`, `SCPIGetWakeInfo`, `SCPIGetPmuEvent`,
  `CA72WDT_*`. Quan hệ với `LPPP_WakeInfo.c` và `LPPP_Statlib.c`."*

### TIER 3 — State machine nguồn KM (đã phủ ~75%, thiếu từng TR chi tiết)

- `[✓]` T3.1 — `PowerControlEventManager`, ma trận 5×10, 5 nguồn event
- `[ ]` T3.2 — *"Đọc `ApplicationKM/LPPP_Matrix.c` đầy đủ. Với MỖI hàm `LPPP_TR*`: liệt kê chính
  xác GPIO/register/flash/I2C nào bị chạm, giá trị ghi, comment lịch sử (OP_BTS/AR số mấy), và điều
  kiện rẽ nhánh. Lập bảng 'transition × side effect'."*
- `[ ]` T3.3 — *"Đọc `ApplicationKM/LPPP_Statlib.c` + `LPPP_KM_Status.h`. Cơ chế status descriptor
  (`ap_sd`, `pwr_sd`), `LPPP_OpenStatus/GetStatus/SetStatus`, mutex bảo vệ. Vì sao chuỗi
  get→dispatch→set không atomic mà vẫn an toàn."*
- `[ ]` T3.4 — *"Đọc `ApplicationKM/LPPP_Task.c` phần LVDS (`LPPP_LVDS_tick`, `LPPP_LVDS_Polling`,
  `LPPP_LVDS_PowerDown`) + `LPPP_McWDT.c`. Máy trạng thái con LVDS: 8 thanh ghi PIOCFG
  (`0xE8308440`…), chống chattering 500ms, quan hệ với `hal_cputimer.c`."*
- `[ ]` T3.5 — *"Đọc `LPPP_Task.c` phần IRQ nội bộ: `LPPP_IRQ_Setting`, `c_intirq_table`
  (NCTI/NPMU/NVAL_FIQ/NVAL_IRQ/MC_LPP_IRQ0/1), `LPPP_IntIRQ_Callback`, `LPPP_AnalyzeIRQEvent`
  (hàm không return — đánh giá mức độ nguy hiểm thật). 6 IRQ này đến từ khối phần cứng nào."*

### TIER 4 — PMU, Power domain, Clock ⚠ LỖ HỔNG LỚN

- `[ ]` T4.1 — *"Đọc `Hal/quartz/asic/build/pmu_lib.h`, `power_api.h`, `power_device_init.h`,
  `power.c` + giải mã (nếu có symbol) `lib/libpmu_r4.a`. Liệt kê đầy đủ các pha `pmu_phase()`:
  `pmu_baseline`, `pmu_ap_link_up`, `pmu_system_up`, `pmu_suspend`, `pmu_resume`. Mỗi pha bật/tắt
  power domain nào theo bảng `power_up_dev[]`/`power_down_dev[]`. Vẽ state diagram PMU."*
- `[ ]` T4.2 — *"Đọc `power.c` — `ap806_power_on/off`, `ap806_power_on_prep`, `ap806_mcix4_enable`,
  `ap806_update_voltage`, `icu_init`, `mci_init_mcix4`. Trình tự bật nguồn AP806 qua I2C tới PS-CPU,
  thứ tự bắt buộc `SLEEP_STATUS_REM` trước `AP_PWR_EN` 100ms, `ap806_power_good` polling."*
- `[ ]` T4.3 — *"Đọc `Hal/quartz/clocks/` đầy đủ. Clock tree S800: SYS_PLL / MC_PLL / ND_PLL,
  ref 25MHz, các bộ chia (QSPI, PI bus, MC). `hwSetProcSpeed` vs PLL thật. `hal_get_clk_freq` /
  `hal_set_current` (SCPI `0x0d`–`0x10`). Vẽ clock tree diagram."*
- `[ ]` T4.4 — *"Đọc `Hal/quartz/asic/build/` phần AVS: `avs_set_from_efuse`, `io_update_voltage`,
  `io_millivolts_set`. Cơ chế Adaptive Voltage Scaling: đọc efuse (ZP/ZN), tính wiper code, ghi
  MCP4542 qua I2C1. Quan hệ với `pmu_*_update_voltage`."*
- `[ ]` T4.5 — *"Đọc `ApplicationKM/LPPP_McChangeFreq.c` + `.h`. Cơ chế đổi tần số Machine Control
  để tiết kiệm điện: ngắt `MC_LPP_IRQ0` → `LPPP_McChangeFreq_handler`, ghi thanh ghi clock nào,
  quan hệ với QSPI clock của MC (`LPPP_McChangeFreq_init` trong `main()`)."*
- `[ ]` T4.6 — *"Đọc `ApplicationKM/LPPP_QoS.c` + `.h` đầy đủ. Mọi thanh ghi AXI QoS: MCix4 RD/WR
  remapping (`0xC05A00xx`), PIDI, CCU_B_LTC (`0xCE001xxx`), UL PRI0_U2A (`0xC0040040`), NB MoChix4.
  Vì sao Sleep1 chỉ cần setup UL còn Sleep2/ErP cần MoChi+AP. Ý nghĩa giá trị `[2,1,0,1,2]`."*

### TIER 5 — Netstack & Proxy (lý do LPPP tồn tại) ⚠ LỖ HỔNG LỚN

- `[ ]` T5.1 — *"Đọc `Netstack/lwipv6/` — cấu trúc, khác biệt với lwIP mainline, hỗ trợ IPv4/IPv6
  song song. `Sys/lan/` (`SysLan_Init`, `SysLan_setup_gigabit_core`), `Sys/socket/`. Buffer pool
  ở `.netbufsec`/`.bufpools` trong vùng NOCACHE. Vẽ sơ đồ đường đi 1 gói từ GMAC → lwIP → proxy."*
- `[ ]` T5.2 — *"Đọc `Sys/filter/` đầy đủ — packet filter 2 tầng. Tầng phần cứng (pattern match
  trong GMAC/Iris) vs tầng phần mềm. `SysFilter_Init`, cấu hình filter mặc định trong
  `Application/thread`. Gói nào được HW match → sinh `APP_EVENT_RECEIVE_PM_PACKET`."*
- `[ ]` T5.3 — *"Đọc `Application/simple_mdns_proxy/` + `llmnr_proxy/` + `nbns_proxy/`. Mỗi proxy:
  đăng ký nghe gì, parse gói ra sao, tự sinh response thế nào, khi nào KHÔNG trả lời mà đánh thức
  host. Đối chiếu với `AppThread_HandlePatternMatchedPacket`."*
- `[ ]` T5.4 — *"Đọc `Application/snmp_proxy/` + `wake_service_proxy/`. SNMP proxy trả lời OID nào
  offline, OID nào phải wake. `wake_service_proxy` — dịch vụ đánh thức theo cổng/giao thức."*
- `[ ]` T5.5 — *"Đọc `Sys/sdk/` — API `mvbase_api_*` (socket, network, pattern, offload, ipsec).
  ARP/NS offload (`NUM_ARP_OFFLOAD_ENTRIES`, `NUM_NS_OFFLOAD_ENTRIES`). Quan hệ với memtable
  capability (`fw_npo_capabilities`, `fw_wol_capabilities`)."*
- `[ ]` T5.6 — *"Đọc `Application/thread/app_thread.c` đầy đủ — `ApiAppInit`, `ApiAppMainLoop`,
  `periodic_callback`/`periodic_callback2`, cấu hình filter mặc định. Toàn bộ danh sách `APP_EVENT_*`
  và handler tương ứng."*

### TIER 6 — Kênh IPC thứ 2: HCI FIFO & hostcmd ⚠ LỖ HỔNG

- `[ ]` T6.1 — *"Đọc `Sys/fifo/` + `Sys/hostcmd/` + `Application/hostcmd/app_hostcmd.c`. Cơ chế
  ComFifo trong SRAM LCM `0xE8200800` (6.1KB, node `net-proxy-hci` trong DTS). Định dạng message,
  `SysFifo_HandlePendingRequests`, `AppHostcmd_HandleMessageFromSystem`. Toàn bộ `MSG_CMD_*`
  (`SYSTEM_SET_WOL`, cấu hình IP/filter/proxy)."*
- `[ ]` T6.2 — *"Đọc phía Linux `Kernel/K-S800/Src/drivers/net/` phần net-proxy / HCI driver.
  Đối chiếu định dạng FIFO 2 bên. `usbd_qhead_lcm` (`0xE8200000`, 2KB) dùng cho gì."*
- `[ ]` T6.3 — *"Trace luồng `DRIVER_HELLO` / `DRIVER_GOODBYE` end-to-end: Linux suspend →
  driver báo → LPPP nhận quyền Ethernet → ... → resume → trả quyền. 4 bước trong `readme.txt`."*

### TIER 7 — Ngoại vi lõi (đã phủ I2C/Timer/Memory; thiếu GPIO/Flash/Interrupt)

- `[✓]` T7.1 — I2C, Timer, Memory, CDMA
- `[ ]` T7.2 — *"Đọc `ApplicationKM/LPPP_Gpio.c` + `.h` đầy đủ. Toàn bộ `gpio_info[]` và
  `wake_gpio_info[]`: mỗi chân — số, hướng, logic level, chức năng, callback. `LPPP_GPIO_Setting`
  (chỉ 2 chân được attach ISR), `LPPP_GPIO_Callback`, `LPPP_GPIO_CallbackMainSW` (reset cứng),
  `LPPP_GPIO_CallbackDummy`, `LPPP_GPIO_HandleMainSW`. Bản đồ chân GPIO ↔ tín hiệu board."*
- `[ ]` T7.3 — *"Đọc `Hal/quartz/` GPIO driver (`gpio_init`, `app_gpio_init`, `gpio_set_func_sel`,
  `gpio_isr_attach`, `pad_config_init`). Cơ chế pad mux, bank A–H, quan hệ pad ↔ GPIO number.
  Strap GPIO bank H (`0xE830A700`) mã hoá cấu hình LPDDR4 ra sao."*
- `[ ]` T7.4 — *"Đọc `ApplicationKM/LPPP_Flash.c` + `LPPP_FlashSub.c` + `LPPP_Flash.h` đầy đủ.
  Layout QSPI CS0 (`0xF4000000`): romfs, spi-user, LPDDR4 param `0x381800`, cờ mất điện
  `0x377000`/`0x378000`, Machine Type `0x379000`/`0x37A000` (+ write-counter), B014 register
  `0x3FF01C`, secure area `0x3FD000`. `flash_modify_sub` RAM execution, redundancy 2 mặt."*
- `[ ]` T7.5 — *"Đọc `Hal/quartz/interrupt/` (`arm_gic.c`, `int_common.c`) + `Hal/quartz/asic/a0`
  interrupt config. GIC v1 (PL390): SGI/PPI/SPI, `INTRL_GIC_OFFSET=32`, `intAttach/intEnable/intDisable`,
  `irq_handler`, `Handlers[]` table. `intnums.h` — bảng số hiệu ngắt đầy đủ. SIC (nếu có)."*
- `[ ]` T7.6 — *"Đọc `Hal/quartz/uart/` + `Hal/quartz/debug/` + `setup_debug_print_logic` +
  `AppPrintInit` + `Sys/debug/`. Driver UART tối giản vs đầy đủ (bị loại 64KB). `ApiSysDebug_Printf`,
  debug mask, `isForceLogOutput`, `setup_debug_read`. `Sys/jtag/`."*
- `[ ]` T7.7 — *"Đọc `Hal/quartz/cpu/` (`hal_cpu.c`, `hal_pause.c`, `hal_cputimer.c`,
  `cortex_r4_*`). `cpu_disable_interrupts`, `cpu_spin_delay`, `cpu_dcache_*`, `pause_cpu` (GIC
  save/restore + wfi), `HalApiCpuTimer_UpdateTickCount`, `polling_functions[]` trong tick ISR."*

### TIER 8 — DDR / LPDDR4 & sleep ⚠ LỖ HỔNG

- `[ ]` T8.1 — *"Đọc `Hal/quartz/asic/build/` phần DDR: `init_ddr`, `get_ddr_memory_size`/
  `write_ddr_memory_size`, `mci_init`. Quá trình training LPDDR4, tham số từ flash `0x381800`,
  strap GPIO bank H (CH0/CH1 CS0/CS1 + hãng DRAM). Vì sao resume bỏ qua training."*
- `[ ]` T8.2 — *"Đọc `Hal/quartz/asic/build/mv_lpddr4_sleep.c` đầy đủ. Self-refresh vào/ra,
  `mv_xor_4dma_crc32_list` (CRC32 vùng DDR qua XOR engine AP806, 'bypass MPU'), lưu/khôi phục
  controller state. Quan hệ với `pmu_suspend`/`pmu_resume` và cờ warmboot."*
- `[ ]` T8.3 — *"Đọc `Hal/quartz/cdma/` phần XOR engine thật của AP806 (`mv_xor*`). Đây là DMA
  đa dụng duy nhất được dùng thật. API, descriptor list, dùng ở đâu ngoài CRC32."*

### TIER 9 — Wake sources (đã phủ wake_timer; thiếu USBD/PCIE/GPIO/WoL)

- `[✓]` T9.1 — wake_timer (SCPI 0x06)
- `[ ]` T9.2 — *"Đọc `Sys/wake_USBD/` (`lpp_usbd_init`, `lpp_usbd_isr`) + `.usbd2_qh_sec`. USB
  device controller làm nguồn wake: queue head ở LCM `0xE8200000`, khi có hoạt động USB → set
  `SYS_EVENT_WAKE_SYSTEM` + `LPPP_SetWakeInfo(USB)`. Cấu trúc gói USB HS."*
- `[ ]` T9.3 — *"Đọc `Sys/wake_PCIE/` (`wake_pcie_init`). PCIe làm nguồn wake, quan hệ với Iris."*
- `[ ]` T9.4 — *"Đọc `Hal/quartz/` + `Sys/lan/` phần Wake-on-LAN: `HalApiWol_*`, `wake_lan`,
  `clear_wake_ints`, `wake_isr` (SCPI `0x80` đăng ký tối đa 10 wake int). Magic packet / ARP /
  unicast / link-act. `g_wolMode` được set từ đâu, ảnh hưởng Sleep2 vs ErP thế nào."*
- `[ ]` T9.5 — *"Tổng hợp: lập bảng đầy đủ MỌI nguồn đánh thức LPPP (GPIO panel/door/mainsw, RTC/
  wake_timer, LAN/WoL, USB, PCIe, SNMP) → cơ chế phát hiện → điểm hội tụ (`power_cpu(0,0)`) →
  `LPPP_GetWakeInfo` bit nào. Đối chiếu `LPPP_WakeInfo.c`."*

### TIER 10 — Tương tác với Cortex-M3 (Machine Control)

- `[~]` T10.1 — McChangeFreq, McWDT, CDMA cross-boundary (rải rác trong T3.4, T4.5, CDMA.md)
- `[ ]` T10.2 — *"Đọc `Subset/S-S800/` (nếu là firmware M3) hoặc phần MC trong `lppp_s800`. Xác định
  M3 chạy gì, giao tiếp với R4 qua kênh nào (`MC_LPP_IRQ0/1`, SPI0/1, QSPI riêng, LCM `0xEE02FE00`).
  `pmu_MC_M3`, `pmu_MC_SPI0/1`, `pmu_MC_QSPI` — power domain của MC."*

### TIER 11 — Build, board variants, debug

- `[ ]` T11.1 — *"Đọc toàn bộ hệ Makefile (`Makefile`, `-cfg`, `-rules`, `-os`, `-lib`, `-lib-cfg`).
  Cách chọn ARCH/ASIC, `TL_ARFLAGS`, ghép `mvri.bin` (`.reset` + phần còn lại + negative checksum),
  macro `-D` truyền vào (`SYSTEM_HEAP_SIZE`, `APP_HEAP_SIZE`, `ENABLE_SCA/NPO/WOL`). Section
  `.rewrite-lppp` (no-op)."*
- `[ ]` T11.2 — *"Đọc `getMachineType()` (`app_scpi.c:541`) + `supportTEC()` + `isMachineSupportWdtVersionCheck`
  + `isFunctionVersion_*`. Cơ chế đọc Machine Type 2 mặt QSPI, chọn mặt mới hơn theo write-counter.
  Map 12 board config ↔ MACHINEINFO_* ↔ Eagle3/DenebMLK/Sparrow. Ảnh hưởng luồng (Eagle3 chờ 100ms…)."*
- `[ ]` T11.3 — *"Đọc `ApplicationMrvl/Test/app_cmdproc.c` đầy đủ — console debug: mọi lệnh
  `runcmd()`, `mhu` (inject SCPI giả), read/write reg, dump memory, test GPIO. `TestMsgSend`/
  `TestMsgSendIsr` (bug đảo logic). `ApplicationMrvl/util/` (Perl: sniffer, sleep_test, memory_dump_analyzer)."*

### TIER 12 — Phía đối tác (cross-repo)

- `[ ]` T12.1 — *"Đọc `atf_s800/` — kiến trúc ATF cho S800: BL1/BL2/BL31, `plat/marvell/a8k/quartz`.
  Luồng PSCI CPU_ON/CPU_OFF/SYSTEM_SUSPEND ↔ SCPI gửi xuống R4. Warm boot entry, RVBAR, cờ warmboot
  `0x7FFFFFF0`. BLE (DDR init — bỏ qua vì R4 tự làm)."*
- `[ ]` T12.2 — *"Đọc `BootROM/` — BootROM đọc CS1 `0xF8000000`, kiểm checksum `mvri.bin`, nạp/khởi
  động R4. Định dạng `code_header_t` phía loader. Boot type (`c_BootType`) 0–12 xác định thế nào."*
- `[ ]` T12.3 — *"Đọc `PS-CPU/pscpu_s800/` — firmware PS-CPU (MCU trên bo nguồn). Bản đồ thanh ghi
  slave I2C `0x3E` phía PS-CPU (`0x68`/`0x6C`/`0x70`/`0x74`/`0x78`/`0xC0`). Bộ ghi log sự kiện
  (mã 3/56/88…), điều khiển MC_PWR_EN, SLEEP_STATUS_REM. `isAutomaticMeasurementToolOn` (SoftSW 0xA4 bit8)."*
- `[ ]` T12.4 — *"Đọc `Kernel/K-S800/Src/arch/arm64/boot/dts/marvell/quartz*.dtsi` đầy đủ. Mọi
  'hợp đồng địa chỉ bằng comment' giữa DTB ↔ LPPP linker ↔ ATF: `scp-shmem`, `usbd_qhead_lcm`,
  `net-proxy-hci`, `lppp-spi-firmware`, `bspi0/bspi1`. Driver Linux tương ứng mỗi node."*

### TIER 13 — Sequence tổng hợp end-to-end (synthesis — làm cuối)

- `[ ]` T13.1 — *"Vẽ sequence diagram ĐẦY ĐỦ COLD BOOT: nguồn AC → BootROM → R4 `main()` →
  `copy_run_atf` → ATF → U-Boot → Linux → driver load → `DRIVER_HELLO`. Đánh dấu mọi lần R4↔A72
  bắt tay (SCPI/MHU), mọi lần R4↔PS-CPU (I2C), mọi lần ghi flash, timeline có đo (PS-CPU log)."*
- `[ ]` T13.2 — *"Vẽ sequence diagram ĐẦY ĐỦ NORMAL → SLEEP2/ErP: Linux quyết định ngủ →
  4× SCPI `0x03` → `power_cpu` core cuối → `pmu_suspend` → `GO_TO_S2/ERP` → `PwEvtMgr` chạy
  transition → DDR self-refresh → AP_PWR_EN=0. Cái gì còn sống trên R4."*
- `[ ]` T13.3 — *"Vẽ sequence diagram ĐẦY ĐỦ RESUME (wake): nguồn wake (chọn 3 loại: WoL, panel,
  wake_timer) → `power_cpu(0,0)` → `pmu_resume` → `WAKEUP_S1` → `TR8/10` → QoS reload → core 0
  thoát reset → ATF warm boot → Linux resume → `DRIVER_GOODBYE`/`CAN_WAKE`. Đối chiếu cold vs warm."*
- `[ ]` T13.4 — *"Vẽ sequence diagram ĐẦY ĐỦ UPDATE FW LPPP: userspace `flashcp` → mtd → spi_nor →
  `mrvl_prepare` → SCPI `0x0a{0,1}` → `pause_cpu` → erase/write từng page → `0x0a{0,0}` → wake.
  Chỉ rõ chỗ tranh chấp BOOTSPI controller và cơ chế chống treo."*
- `[ ]` T13.5 — *"Tổng hợp 'danh sách bug & code chết' toàn firmware LPPP từ tất cả note: phân loại
  🔴/🟠/🟡, kèm điều kiện kích hoạt và hậu quả thực tế. Đánh dấu cái nào đã có bản vá (OP_BTS/AR số)."*

---

## PHỤ LỤC — Bản đồ note hiện có → prompt

| Note đã có | Phủ prompt |
|---|---|
| `Folder_Struct/Cấu trúc folder.md` | T0.1 |
| `Flow LPPP.md` | T1.1, T1.5 (một phần) |
| `Function/hal_main_initialize_sections.md` | T1.2 |
| `Function/copy_run_atf.md` | T1.2 |
| `Flow/Phân tích task.md` | T2.1, T2.2 (một phần) |
| `Flow/Thread/AppThread_Main.md` | T5.6 (một phần) |
| `Flow/Thread/AppMhuTest.md` | T2.1 (đầy đủ nhất) |
| `Flow/Thread/PowerControlEventManager.md` | T3.1 (đầy đủ nhất) |
| `Flow/Thread/Function/power_cpu.md` | T2.1 (nhánh 0x03) |
| `Flow/Thread/Function/scpi_set_power_state.md` | T2.1 |
| `Flow/Thread/Function/waker timer.md` | T9.1 |
| `Flow/Thread/Function/wake_system.md` | T6/T9 (event plumbing) |
| `Flow/Thread/Function/iris.md` | T4.2 / T9.3 (một phần) |
| `Flow/Thread/Function/Function_on_system/SysSystem_ThreadMain.md` | T2/T6 (dispatch) |
| `Update_FW_LPPP/Update_FW_LPPP.md` | T13.4 (một phần) |
| `Core/I2C.md` | T7.1 / T12.3 (một phần) |
| `Core/timer/Timer-Introduction.md` | T7.1 |
| `Core/Memory/Memory.md` | T1.6 / T6 (một phần) |
| `Core/CDMA/CDMA.md` | T8.3 (một phần) |

**Thứ tự đề xuất:** T0 → T4 (PMU là lỗ hổng lớn nhất, khoá hiểu toàn bộ luồng nguồn) →
T5 (lý do LPPP tồn tại) → T6 → T7.2/7.4/7.5 → T8 → T9 → T2.4/2.5 (phía đối tác) → T12 → T13 (synthesis).

---

## ✅ TRẠNG THÁI HOÀN THÀNH (cập nhật cuối phiên)

Toàn bộ roadmap T0–T13 đã có note trong `Overview/`, `Function/`, `Netstack/`. 51 file `T*.md`.

| Tier | Note |
|------|------|
| T0 | `Overview/T0.2`, `T0.3` |
| T1 | `Function/T1.3`–`T1.6` (+ `copy_run_atf.md`, `hal_main_initialize_sections.md`) |
| T2 | `Function/T2.3`–`T2.6` |
| T3 | `Function/T3.2`–`T3.5` |
| T4 | `Function/T4.1`–`T4.6` |
| T5 | `Netstack/T5.1`–`T5.6` |
| T6 | `Netstack/T6_HCI_FIFO_hostcmd_HELLO_GOODBYE.md` (gộp 6.1+6.2+6.3) |
| T7 | `Function/T7.2`–`T7.7` |
| T8 | `Function/T8.1_DDR_LPDDR4_init_sleep_XOR.md` (gộp 8.1+8.2+8.3) |
| T9 | `Function/T9.2`–`T9.4` + `Overview/T9.5_Tong_hop_nguon_danh_thuc.md` |
| T10 | `Overview/T10.2_M3_Machine_Control_va_giao_tiep_R4.md` |
| T11 | `Overview/T11.1`–`T11.3` |
| T12 | `Overview/T12.1`–`T12.4` |
| T13 | `Overview/T13.1`–`T13.3`, `T13.5` + `Netstack/T13.4` |

**Đọc theo trình tự hiểu nhanh nhất:** T13.1 (cold boot) → T13.2 (sleep) → T13.3 (wake) → T13.4 (proxy) → T13.5 (bug list) — 5 note synthesis này tóm cả hệ thống, mỗi dòng trỏ note chi tiết.
