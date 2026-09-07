# LPPP — Luồng hoạt động (runtime flow) end-to-end

> Dựng lại từ 19 note deep-dive + source `lppp_s800/`. Đây là "câu chuyện vận hành":
> máy in chạy firmware này hành xử thế nào qua toàn bộ vòng đời nguồn.
> Xem chi tiết từng hàm ở `_LPPP_Master_Index.md`.

---

## 0 · NGUYÊN TẮC NỀN — 3 tầng điều khiển, ai làm chủ ai

```
┌──────────────────────────────────────────────────────────────────┐
│ PS-CPU  — MCU nhỏ trên bo nguồn, SỐNG NGAY KHI CÓ ĐIỆN AC         │
│   • nắm phần cứng mà R4 không với tới: relay, nguồn động cơ,       │
│     SLEEP_STATUS_REM, bộ ghi log sự kiện                          │
│   • kênh duy nhất với R4 = I2C0, PS-CPU là slave 0x3E             │
└────────────────────────────┬─────────────────────────────────────┘
                             │ I2C0  (R4 là master)
┌────────────────────────────▼─────────────────────────────────────┐
│ R4 / LPPP  — Cortex-R4 + FreeRTOS   ★ "BỘ NÃO ALWAYS-ON" ★        │
│   • chạy LIÊN TỤC từ lúc BootROM nạp tới lúc rút điện AC          │
│   • giữ máy trạng thái 5 nguồn, 2 watchdog, stack mạng proxy      │
│   • điều khiển A72: SCPI/MHU + ghi thẳng thanh ghi PMU của AP806  │
└────────────────────────────┬─────────────────────────────────────┘
       SCPI/MHU (điều khiển)  │  HCI FIFO ở SRAM (dữ liệu mạng + IOCTL)
┌────────────────────────────▼─────────────────────────────────────┐
│ A72 / AP806  — 4× Cortex-A72, ATF + Linux    "KHÁCH TRỌ"          │
│   • R4 bật/tắt nó — NÓ KHÔNG TỰ BẬT ĐƯỢC                          │
│   • khi máy ngủ: A72 tắt HOÀN TOÀN, R4 một mình giữ Ethernet      │
└──────────────────────────────────────────────────────────────────┘

Bên cạnh:
  • M3 (Machine Control) — lõi thứ 3, điều khiển engine cơ khí; R4 nói chuyện
    qua MC_LPP_IRQ0/1 + đổi tần số MC để tiết kiệm điện
  • Iris — die I/O RỜI, nối qua MoChi, CHỨA GMAC VẬT LÝ. ICU của Iris quyết
    định ngắt card mạng đi về R4 (khi ngủ) hay về A72 (khi thức)
```

**Câu một dòng:** R4 là chủ, A72 là khách. Mọi "bật/tắt/ngủ/thức" của A72 đều đi qua R4;
mọi phần cứng nguồn thật nằm dưới R4 ở PS-CPU.

---

## 1 · VÒNG ĐỜI NGUỒN — 5 trạng thái

```
        ┌──────────┐
        │ POWEROFF │  chỉ là giá trị khởi tạo — không bao giờ quay lại được
        └────┬─────┘
     PWR_SW_ON │  LPPP_KM_Init() gọi THẲNG event_handler(), 1 lần, KHÔNG qua queue
        ┌────▼─────┐
   ┌───►│  NORMAL  │◄─────────────────────────────┐
   │    └────┬─────┘                              │ WAKEUP_NORMAL (SCPI 0x88 payload 0)
   │ GO_TO_S1│ (SCPI 0x88 payload 1)              │ hoặc PWR_FLICKER (TR2b)
   │    ┌────▼─────┐   WAKEUP_S1                   │
   │    │  SLEEP1  │──────────────────────────────┤  TR4: cắt-lại-I2S, QoS UL
   │    └────┬─────┘   (từ power_cpu core đầu bật) │
   │ GO_TO_S2 / GO_TO_ERP                          │
   │  (power_cpu khi core A72 CUỐI CÙNG tắt)       │
   │    ┌────▼──────┐                              │
   │    │ SLEEP2/ErP│──────────────────────────────┘
   │    └───────────┘   GO_TO_S1 → TR8/10 → lên SLEEP1 ("thức dần")
   │
   └── (TR1 / TR7-9 → OFF: CÓ trong ma trận nhưng KHÔNG BAO GIỜ CHẠY —
        PWR_SW_OFF không có producer. Tắt máy thật = ISR nút nguồn kéo HRESET_REQN=0)

SLEEP2 vs ErP do g_wolMode quyết (Wake-on-LAN config, đến từ host-command MSG_CMD_SYSTEM_SET_WOL):
   g_wolMode = 0  → SLEEP2  (còn nhiều domain, thức nhanh hơn)
   g_wolMode = 1  → ErP     (deep, cắt tối đa, tiết kiệm điện nhất)
```

| State | A72 | Domain còn điện | DDR | Chi phí thức |
|---|---|---|---|---|
| NORMAL | chạy Linux | tất cả | active | — |
| SLEEP1 | cluster CPU tắt | MCix4/PIDI/CCU CÒN | active | rẻ (TR4: ~2 reg) |
| SLEEP2 | tắt hẳn | cắt MCix4/PIDI/IRIS/UL/Mech | self-refresh | đắt (TR8/10: I2C + flash + ~25 reg QoS) |
| ErP | tắt hẳn | cắt sâu hơn SLEEP2 | self-refresh | đắt |

---

## 2 · COLD BOOT — cắm điện AC / bật từ trạng thái rút nguồn

```
① AC ON
    └─ PS-CPU sống dậy trước, cấp nguồn cơ bản cho khối R4

② BootROM (lõi R4)
    └─ đọc QSPI CS1 @ 0xF8000000, kiểm negative-checksum của mvri.bin
    └─ R4 KHÔNG chép firmware vào RAM — nó XIP THẲNG TỪ QSPI
    └─ nhảy vào section .reset (offset 0): dựng stack 5 chế độ ARM, SCTLR, MPU tạm
    └─ gọi main()

③ main() — 7 giai đoạn bare-metal (chưa có RTOS, chưa có task)
    ┌─────────────────────────────────────────────────────────────────┐
    │ (1) hal_main_initialize_sections()                              │
    │       chép .data từ flash(LMA) → LCM(VMA) · xoá .bss            │
    │       ⇒ TỪ ĐÂY biến toàn cục mới hợp lệ                         │
    │ (2) hwConfigInit · cpu_init (MPU 5 vùng, D-cache) · pad_config  │
    │ (3) setup_debug_print_logic()  ⇒ TỪ ĐÂY mới in UART được       │
    │ (4) pmu_phase(pmu_baseline)  SYS/MC/ND PLL lên · 12→200 MHz    │
    │       avs_set_from_efuse()   hiệu chỉnh điện áp theo lô chip    │
    │ (5) intInit (GIC) · McChangeFreq/McWDT init · gpio_init         │
    │ (6) ★ early_board_init(false):                                  │
    │       turn_on_qspi · mci_init+iris_init · init_ddr(false)       │
    │       copy_run_atf(0xF4000000)  ══► CỤM A72 BẮT ĐẦU CHẠY ATF   │
    │ (7) AppInit_Initialize() → ... → vTaskStartScheduler()          │
    │       (không return) · while(1) lưới an toàn                    │
    └─────────────────────────────────────────────────────────────────┘

④ copy_run_atf() — điểm bàn giao sang A72 (chi tiết)
    • đọc code_header_t (magic 0xB105B002) — KHÔNG kiểm checksum thân ảnh
    • cpu_disable_dcache()
    • start_ap806(false): PRCR_0=0 GIỮ A72 trong reset (kèm WDT 500ms — reset
      AP806 có thể treo bus MoChi, watchdog là lưới an toàn duy nhất)
    • memcpy(DDR + load_addr, header + prolog_size, boot_image_size)  ← ATF vào DDR
    • RVBAR_0/1 = load_addr >> 16     ← điểm A72 nhảy vào khi thoát reset
    • WARMBOOT_FLAG (0x7FFFFFF0) = 0  ← báo ATF: "đây là COLD boot"
    • start_ap806(true): PRCR_0 = 0x10001  ★ A72 CHẠY ★
    • cpu_enable_dcache() · return NGAY (không handshake, không timeout)
    ⇒ nếu magic sai → in 1 dòng, return. A72 nằm reset MÃI MÃI, R4 vẫn chạy bình thường.

⑤ Scheduler chạy — thứ tự theo priority:
    prio 3  init_thread     — pha 2 nền tảng (xem ⑥), rồi tự vTaskDelete
    prio 1  SysSystem_ThreadMain → AppInit_StartThreads → tạo AppThread_Main
    prio 1  AppThread_Main  → tạo 5 task con (AppMhuTest, cmd, PwMHUEv, PwEvtMgr, ClrWDT)
    prio 0  các task KM     — phục vụ SCPI + máy trạng thái nguồn

⑥ init_thread (prio 3, CHẠY MỘT MẠCH, không nhường CPU):
    • tạo cpu_power_mutex, pmu_transition_mutex
    • LPPP_KM_Init():
        - tạo 3 queue (got_msg_qhandle, gst_MsgQHandle_MHU, gst_MsgQHandle_PowerCtrl)
        - attach 6 IRQ nội bộ, init Flash/GPIO/Statlib/I2C
        - ap_sd = NO_BOOT · pwr_sd = POWEROFF
        - event_handler(PWR_SW_ON)  ⇒ chạy TR0 → pwr_sd = NORMAL  ★ ngay tại đây, prio 3
          (TR0: busy-wait ≤100ms chờ panel µC, rồi bật MOTION_EN_MC/IR nếu Stop không nhấn)
    • get_ddr_memory_size() — đọc strap GPIO bank H, cất vào thanh ghi timer2
    • pmu_phase(pmu_ap_link_up)  — bật NGUỒN AP806 qua I2C→PS-CPU, CHƯA bật core A72
    • pmu_phase(pmu_system_up)   — bật mọi power domain còn lại (bảng power_up_dev[])
    • board_init(false): iris_detection, init_ddr training, copy_run_atf (đã ở ④ nếu emu800)
    • LPPP_QoS_Init/MoChi/UL/AP
    • vTaskDelete(NULL) — trả 1.2KB stack về heap SYSTEM 23KB

⑦ A72 side (song song, R4 không chờ):
    ATF BL1/BL2/BL31 → U-Boot → Linux kernel → driver load
    → Linux gửi SCPI 0x02 (get capability, bắt tay) qua MHU
    → driver net-proxy báo DRIVER_HELLO qua HCI FIFO
    ⇒ Từ đây R4 và A72 chạy SONG SONG, hai hệ điều hành khác nhau trên cùng SoC.
```

**Điểm cần nhớ về cold boot:**
- Có một **cửa sổ mù**: sau khi A72 chạy (bước ⑥ board_init) nhưng trước khi `init_thread`
  xong, `AppMhuTest` chưa được tạo → ngắt MHU chưa bật → LPPP chưa nhận được lệnh SCPI nào.
  Ngắn nhưng có thật (~40 lần ghi reg QoS + ~25 dòng debug print).
- Thứ tự bị ràng buộc chặt: mọi biến toàn cục cần (1); UART cần (3); `intAttach` cần (5);
  `copy_run_atf` cần `init_ddr`; scheduler cần phần cứng ổn định.

---

## 3 · TRẠNG THÁI NORMAL — cái gì đang chạy

**Hai thế giới song song trên R4, gần như không biết nhau:**

```
╔═══════════════ THẾ GIỚI MẠNG ═══════════════╗   ╔══════════ THẾ GIỚI NGUỒN (KM) ══════════╗
║ AppThread_Main ("AppMvl", prio 1)           ║   ║ AppMhuTest ("MHU", prio 0)              ║
║   vòng lặp: ApiSysGetEventsBLock()          ║   ║   xQueueReceive(got_msg_qhandle)        ║
║   ── dùng EVENT-GROUP của Sys, KHÔNG queue  ║   ║   process_scpi_cmd() → ~28 lệnh SCPI    ║
║   xử lý:                                    ║   ║   ── dùng QUEUE FreeRTOS                ║
║   • RECEIVE_PM_PACKET → proxy trả lời       ║   ║ PwMHUEv (prio 0): SCPI → internal event ║
║   • RECEIVE_MESSAGE_FROM_SYSTEM → IOCTL     ║   ║ PwEvtMgr (prio 0): STATE MACHINE 5×10   ║
║   • DRIVER_HELLO/GOODBYE → bàn giao GMAC    ║   ║ ClrWDT (prio 0): 2 watchdog, chu kỳ 5s  ║
║   • LINK_UP/DOWN, SELF_EVENT (1000ms)       ║   ║ cmd_thread: console debug UART          ║
╚═════════════════════════════════════════════╝   ╚═════════════════════════════════════════╝
        ▲                                                     ▲
        │ gặp nhau DUY NHẤT ở lớp HAL/MHU + Iris ICU (sở hữu GMAC)
```

**Ba "đồng hồ" luôn tích tắc ở NORMAL:**
| Nhịp | Ai | Làm gì |
|---|---|---|
| 10 ms | FreeRTOS tick (CPU cycle counter) | scheduler + `polling_functions[]` + `LPPP_LVDS_tick` |
| 1000 ms | `periodic_callback` | bắn `APP_EVENT_SELF_EVENT` cho AppMvl |
| 5000 ms | `ClearWatchDogCounter` | (A) kick WDT phần cứng R4 · (B) đếm CA72 software-WDT (timeout 90s) |

**A72↔LPPP giao tiếp bằng 2 kênh riêng biệt:**
- **SCPI/MHU** (`0xE8009800` + shared-mem DDR `0x7FFF0000`): lệnh điều khiển nguồn/clock/wake.
  MHU chỉ rung chuông 1 bit; data ở DDR; ranh giới thật = D-cache R4 (phải invalidate/writeback tay).
- **HCI FIFO** (SRAM LCM `0xE8200800`, 6.1KB, node `net-proxy-hci`): luồng dữ liệu mạng +
  IOCTL cấu hình (IP/filter/proxy/WoL) từ Linux driver. `SysFifo_HandlePendingRequests` +
  `AppHostcmd_HandleMessageFromSystem` poll mỗi vòng `SysSystem_ThreadMain`.

---

## 4 · LUỒNG NGỦ — NORMAL → SLEEP1 → SLEEP2/ErP

**Linux khởi xướng. R4 chỉ phản ứng.**

```
[Linux quyết định ngủ]
   │
   ├─(A) Linux gửi SCPI 0x88 {payload = GO_TO_S1}
   │       → mhu_handler(ISR) → got_msg_qhandle
   │       → AppMhuTest: process_scpi_cmd()
   │             send_km_queue()  → gst_MsgQHandle_MHU   (mọi SCPI đều nhân bản sang KM)
   │             + trả ACK qua write_MHU + busy-wait chờ AP clear bit
   │       → PwMHUEv: 0x88 payload 1 → LPPP_INTERNALEVT_GO_TO_S1 → gst_MsgQHandle_PowerCtrl
   │       → PwEvtMgr: event_handler(GO_TO_S1)
   │             pwr_sd = NORMAL, event = GoS1
   │             fpLPPP_ScenarioFunc[NORMAL][GoS1] = LPPP_TR3_NormalToSleep1
   │                   • gpio_set_func_sel(33/34, 0)  cắt I2S (audio)
   │                   • return SLEEP1
   │             LPPP_SetStatus(pwr_sd, SLEEP1)
   │   ⇒ pwr_sd = SLEEP1.  A72 vẫn còn điện cluster.
   │
   └─(B) Linux/PSCI gửi SCPI 0x03 (set power state) cho TỪNG core A72 khi suspend
           → scpi_set_power_state() giải mã payload → power_cpu(cpuid|clusterid<<2, 3)
           → power_cpu() [giữ cpu_power_mutex]:
                 • chờ core vào WFI (⚠ vòng chờ có bug độ ưu tiên — bù bằng udelay)
                 • Isolation enable (bit16) → PWR_DN_RQ clear (bit0) → CCU_B_PRCR = Reset
                 • cpu_state |= bit
                 • KHI core CUỐI CÙNG tắt (cpu_state == ALL_CPU_OFF = 0xF):
                       WARMBOOT_FLAG = non-zero        (báo ATF: warm boot khi thức)
                       pmu_phase(pmu_suspend)           cắt MCix4/PIDI/IRIS/UL/Mech
                       suspended = true
                       intr_event = g_wolMode ? GO_TO_ERP : GO_TO_S2
                       xQueueSend(gst_MsgQHandle_PowerCtrl, intr_event)
                       CA72WDT_End()                    tắt software watchdog giám sát A72
           → PwEvtMgr: event_handler(GO_TO_S2 hoặc GO_TO_ERP)
                 state = SLEEP1 → fpLPPP_ScenarioFunc[SLEEP1][GoS2] = LPPP_TR5_TR6
                       • ap_sd = AP_STATUS_NO_BOOT        ★ VŨ TRANG cho TR2b (mất điện)
                       • ghi flash B014 bit TO_SLEEP2_OR_ErP
                       • return SLEEP2 (hoặc ErP)
           ⇒ pwr_sd = SLEEP2/ErP.  DDR vào self-refresh.  AP_PWR_EN có thể hạ.
```

**Sau khi ngủ:** chỉ còn R4 sống. Timer always-on vẫn đếm (nếu Linux đã đặt wake_timer).
Iris ICU đã định tuyến ngắt GMAC về R4. LPPP giờ là proxy mạng.

---

## 5 · KHI ĐANG NGỦ — LPPP làm proxy mạng

```
Gói Ethernet tới GMAC (trong Iris)
   │
   ├─ Packet filter 2 tầng:
   │     tầng phần cứng (pattern match trong GMAC/Iris) lọc thô
   │     tầng phần mềm (SysFilter) lọc tinh
   │
   ├─ Gói KHỚP filter proxy (mDNS/LLMNR/NBNS/SNMP/ARP query tầm thường)
   │     → APP_EVENT_RECEIVE_PM_PACKET → AppThread_HandlePatternMatchedPacket()
   │     → proxy tương ứng tự dựng response, gửi thẳng ra GMAC
   │     ⇒ HOST KHÔNG BỊ ĐÁNH THỨC — mục đích tồn tại của LPPP
   │
   └─ Gói cần host thật xử lý (magic packet WoL / SNMP OID sâu / kết nối mới)
         → LPPP_SetWakeInfo(bit tương ứng)
         → wake_system() → SYS_EVENT_WAKE_SYSTEM
         → chuyển sang LUỒNG ĐÁNH THỨC (mục 6)
```

**Nguồn wake khác trong lúc ngủ:** nút panel/cửa máy (GPIO), USB có hoạt động
(`lpp_usbd_isr`), PCIe (`wake_pcie`), RTC/wake_timer nổ, host command. Tất cả hội tụ về
`SYS_EVENT_WAKE_SYSTEM` → `power_cpu(0, 0)`.

---

## 6 · LUỒNG ĐÁNH THỨC — SLEEP2/ErP → SLEEP1 → NORMAL

```
[Nguồn wake bất kỳ]
   │  (ví dụ: wake_timer nổ)
   ▼
timerISR → my_callback():
   • PLAT_WARMBOOT_FLAG = 0        ★ wake_timer ÉP COLD BOOT (khác các nguồn wake khác)
   • TimerOff/Close
   • wake_system()
        └─ SysApiThread_EventFlagsSet(SysEventGroup, _OR, SYS_EVENT_WAKE_SYSTEM)
             ⚠ BẤT ĐỒNG BỘ: chỉ đẩy yêu cầu vào timer command queue (30 slot)
   │
   ▼  Tmr Svc daemon (prio 3, cao nhất) chạy
xEventGroupSetBits() → SysSystem_SysEventGroup |= SYS_EVENT_WAKE_SYSTEM
   │  đánh thức task đang chờ
   ▼
SysSystem_ThreadMain (prio 1) — PREEMPT mọi task KM ở prio 0
   xEventGroupWaitBits trả về, cờ được XOÁ (OR_CLEAR)
   SysSystem_handle_link_events():
      if (goodbye_processing == false)  →  power_cpu(0, 0)
   │
   ▼
power_cpu(0, 0) [giữ cpu_power_mutex] — cpu_state == ALL_CPU_OFF ⇒ khối "core đầu bật":
   • CA72WDT_Start()                      software watchdog 90s cho A72
   • Bregister_modify(B014_BEGIN_PHASE6)  ghi breadcrumb vào SPI-Flash (trên critical path!)
   • pmu_phase(pmu_resume)                bật lại power domain
        └─ nếu -1 → return false, để lại trạng thái NỬA VỜI
           (watchdog đã bật, flash đã ghi, KHÔNG có event) → chỉ CA72WDT 90s cứu được
   • xQueueSend(gst_MsgQHandle_PowerCtrl, WAKEUP_S1)  ────────────┐
   • RVBAR_0/1 = WARM_BOOT_ADDR (0x1003 = 0x10030000)             │
   • intDisable(MCIX2_1); clear_wake_ints()                       │
   • wake_lan()      Iris ICU định tuyến ngắt GMAC TRỞ LẠI về A72  │
   • SysApiSystem_Wake()                                          │
   • PWR_DN_RQ set → Isolation disable → PRCR = nReset            │
   ⇒ CORE 0 CỦA A72 THOÁT RESET, bắt đầu warm boot ATF            │
                                                                  │
   ┌──────────────────────────────────────────────────────────────┘
   ▼  (song song, PwEvtMgr prio 0 chạy sau khi ThreadMain prio 1 nhường)
PwEvtMgr: event_handler(WAKEUP_S1)
   state = SLEEP2/ErP → fpLPPP_ScenarioFunc[SLEEP2][WakeS1] = LPPP_TR8_TR10  (HÀM NẶNG NHẤT)
      • I2C → PS-CPU: log event 56 (nếu tool đo bật)
      • ghi flash B014 bit TO_SLEEP1
      • LPPP_QoS_SetupMoChi()  10 reg   ┐ nạp lại QoS vì domain MCix4/PIDI/CCU
      • LPPP_QoS_SetupAP()     ~15 reg  ┘ đã MẤT ĐIỆN khi vào Sleep2/ErP
      • return SLEEP1
   LPPP_SetStatus(pwr_sd, SLEEP1)
   ⇒ pwr_sd = SLEEP1

▼  A72 warm boot xong → Linux resume
Linux gửi SCPI 0x03 (power ON) cho các core còn lại → power_cpu(cpuid, on)
Linux gửi SCPI 0x88 {WAKEUP_NORMAL} → PwMHUEv → PwEvtMgr
   fpLPPP_ScenarioFunc[SLEEP1][WakeNorm] = LPPP_TR4_Sleep1ToNormal
      • gpio_set_func_sel(33/34, 1)  khôi phục I2S
      • LPPP_QoS_SetupUL()
      • return NORMAL
   ⇒ pwr_sd = NORMAL

▼  driver net-proxy bàn giao GMAC ngược lại
DRIVER_GOODBYE (qua HCI FIFO) → SysSystem: reset WoL, SysLan_setup_gigabit_core(),
   app_connect_lan_isr(), HalApiEth_unmask_irqs()
CAN_WAKE → gỡ chốt goodbye_processing
   ⇒ Máy trở lại NORMAL đầy đủ, engine sẵn sàng in.
```

**Cold vs warm boot khi thức:** khác nhau DUY NHẤT ở giá trị `WARMBOOT_FLAG` (`0x7FFFFFF0`):
- Nguồn wake sự kiện (panel/LAN/USB) → flag ≠ 0 → ATF **warm boot** (khôi phục nhanh từ snapshot)
- `wake_timer` nổ → `my_callback` ghi flag = 0 → ATF **cold boot** (khởi động sạch — đánh thức
  theo lịch thì làm mới hoàn toàn)

---

## 7 · CÁC LUỒNG ĐẶC BIỆT

### 7.1 · Mất điện thoáng qua (power flicker)

```
Điện AC chập chờn → chân POWER_MONITOR đổi mức → GPIO ISR
   → LPPP_GPIO_Callback → is_power_flicker() phát hiện cạnh lên (0→1)
   → xQueueSendFromISR(gst_MsgQHandle_PowerCtrl, PWR_FLICKER)
   → PwEvtMgr: event_handler(PWR_FLICKER)
       state ∈ {NORMAL, SLEEP2, ErP} → LPPP_TR2b_Sleep2_Or_ErPToNormal
           if (ap_sd == AP_STATUS_NO_BOOT)          ← A72 CHƯA boot xong
                LPPP_SpiFlashWrite_PowerFlickerFlag()  ghi 0x10 vào 2 vùng QSPI redundant
                                                       (0xF4377007 + 0xF4378007, U-Boot đọc)
                LPPP_GPIO_Write(HRESET_REQN, 0)        RESET CỨNG A72
           return NORMAL   (luôn luôn — kể cả khi không làm gì)
```

`ap_sd` là biến ghép nối: `TR5/6` đặt `NO_BOOT` khi vào Sleep2/ErP; SCPI `0x83 SET_AP_STATUS`
gỡ về `WARP_BOOT` khi Linux nói "tao boot xong rồi". Nghĩa là: flicker khi A72 chưa sẵn sàng
→ reset cứng cho chắc; flicker khi A72 đang chạy ổn → bỏ qua.

### 7.2 · A72 / Linux treo

```
ClearWatchDogCounter (chu kỳ 5s) đếm ca72wdt_timer
   Linux phải gửi SCPI 0x8B {WD_CA72_REFRESH} định kỳ để reset bộ đếm
   nếu ca72wdt_timer >= timeout (mặc định 90s):
      CA72WDT_Expire():
         LPPP_SpiFlashWrite_Ca72wdt_Reboot()   ghi log nguyên nhân vào QSPI (để truy nguyên)
         LPPP_GPIO_Write(HRESET_REQN, 0)        CƯỠNG BỨC RESET A72
```

Đây cũng là **lối thoát duy nhất** khi `AppMhuTest` kẹt busy-wait chờ A72 ACK (A72 treo →
queue SCPI đầy → mất lệnh im lặng → 90s sau CA72WDT kéo máy ra).
⚠ Lệnh `0x8B {WD_CA72_END}` bị từ chối im lặng khi đang ở SLEEP1 (fix OP_BTS-16528 — không cho
tắt watchdog lúc ngủ, vì A72 treo trong sleep sẽ không ai cứu).

### 7.3 · Nút nguồn panel

```
Người dùng bấm MAINSW_ON → GPIO ISR → LPPP_GPIO_CallbackMainSW()
   → LPPP_GPIO_Write(HRESET_REQN, 0)  kéo thẳng, TỪ TRONG ISR
   ⇒ BỎ QUA HOÀN TOÀN state machine. Đây là lý do TR1/TR7-9 (→OFF) là dead code.

Trước khi Linux boot xong: LPPP tự xử lý nút nguồn (như trên).
Sau khi Linux sẵn sàng: Linux gửi SCPI 0x8A (Get PMU event) — hàm này CÓ SIDE EFFECT:
   LPPP_GPIO_HandleMainSW(false) → gpio_isr_detach(msw_pin) → gpio_close(msw_pin)
   ⇒ LPPP NHẢ nút nguồn cho Linux xử lý. Không thể gọi 0x8A lần nữa để hoàn tác.
```

### 7.4 · Update firmware LPPP

```
userspace: flashcp mvri.bin /dev/mtdX   (X = "lppp-spi-firmware", QSPI CS1)
   → mtd → spi_nor → với MỖI erase/write:
        mrvl_prepare() → disable_clk_r4(true) → SCPI 0x0a {domain=0, idx=1}
              → LPPP process_scpi_cmd case 0xa → pause_cpu(chan):
                    lưu + tắt TOÀN BỘ GIC, chừa 1 ngắt MHU làm đường thức
                    clear_MHU + write_MHU  ← TỰ trả response TRƯỚC KHI wfi
                    __asm("wfi")            ← R4 ĐỨNG IM
        usleep(1ms)   ← Linux chờ chắc chắn R4 đã dừng
        mrvl_qspi_erase / mrvl_qspi_write   ← Linux ĐỘC CHIẾM BOOTSPI controller
        mrvl_unprepare() → SCPI 0x0a {0,0} → ngắt MHU đánh thức R4 khỏi wfi → khôi phục GIC
```

Cơ chế loại trừ giữa R4 và A72 trên cùng 1 bộ điều khiển BOOTSPI (`0xE8273000`) chỉ là
**thoả thuận phần mềm** ("bảo R4 ngủ rồi chờ 1ms"), không có semaphore phần cứng. Bug
`OP_BTS-28170/28498` ("LPPP フリーズ") xoay quanh tranh chấp này. Không có A/B partition —
mất điện giữa lúc ghi = LPPP không boot = máy chết cứng.

### 7.5 · LVDS power-down (chạy song song, độc lập nguồn)

```
GPIO XEN_LVDS_ENG đổi cạnh → ISR → LVDS_INTERRUPT vào PowerCtrl queue
   → PwEvtMgr: LPPP_LVDS_Interrupt() → LPPP_LVDS_Polling(true) → bật "máy bơm"
timer ISR mỗi 10ms → LPPP_LVDS_tick → mỗi 100ms gửi LVDS_TIMER
   → LPPP_LVDS_Polling(false): cần 5 mẫu giống nhau liên tiếp = 500ms chống chattering
   → LPPP_LVDS_PowerDown(true/false): ghi bit 25 vào 8 thanh ghi PIOCFG
     (Y/M/C/K × ENG0/ENG1 @ 0xE8308440…) — tắt transceiver LVDS tới print head khi không dùng
```

2 cột `LVDS_INTERRUPT`/`LVDS_TIMER` trong ma trận 5×10 **luôn trả về state không đổi** —
máy trạng thái con hoàn toàn tách biệt, chỉ mượn cơ chế queue/dispatch.

---

## 8 · BẢN ĐỒ ĐƯỜNG DỮ LIỆU — nhìn một lần cho nhớ

```
                          ┌─────────────┐
        SCPI (điều khiển) │             │  HCI FIFO @ SRAM 0xE8200800 (dữ liệu mạng + IOCTL)
   ┌──────────────────────┤   A72       ├──────────────────────┐
   │  MHU doorbell +       │  Linux      │                      │
   │  shared-mem DDR       └─────────────┘                      │
   │  0x7FFF0000                                                │
   ▼                                                            ▼
┌────────────────────────────────────────────────────────────────────┐
│                          R4 / LPPP                                  │
│  mhu_handler → got_msg_qhandle → AppMhuTest → process_scpi_cmd      │
│       │ send_km_queue (mọi lệnh, vô điều kiện)                      │
│       ▼                                                             │
│  gst_MsgQHandle_MHU → PwMHUEv → gst_MsgQHandle_PowerCtrl → PwEvtMgr │
│                                        ▲         ▲                  │
│  ISR nội bộ (NCTI/NPMU/…) ─────────────┘         │                  │
│  GPIO ISR (POWER_MONITOR, XEN_LVDS_ENG) ─────────┤                  │
│  timer ISR (LVDS_TIMER) ─────────────────────────┤                  │
│  power_cpu() (GO_TO_S2/ERP, WAKEUP_S1) ──────────┘                  │
│                                                                    │
│  wake_system() → timer daemon → SysSystem_SysEventGroup             │
│       → SysSystem_ThreadMain → power_cpu(0,0)                       │
│                                                                    │
│  AppThread_Main ── event-group ── proxy mạng (lwIPv6 + filter)      │
└───────────────┬──────────────────────────────┬─────────────────────┘
      I2C0      │                              │  MoChi
      ▼         ▼                              ▼
┌───────────┐ ┌──────────┐              ┌──────────────┐
│  PS-CPU   │ │ MCP4542  │              │  Iris (GMAC) │
│  0x3E     │ │  0x2D    │              │  ICU routing │
│ nguồn     │ │ điện áp  │              │  R4 ↔ A72    │
└───────────┘ └──────────┘              └──────────────┘
```

---

## 9 · MƯỜI CÂU CHỐT ĐỂ NHỚ LUỒNG

1. **R4 chạy liên tục; A72 là khách R4 bật/tắt.** Mọi ngủ/thức đi qua R4.
2. **5 trạng thái nguồn**, chuyển bằng ma trận `fpLPPP_ScenarioFunc[state][event]` trong `PwEvtMgr`.
3. **Linux khởi xướng ngủ** (SCPI `0x88 GO_TO_S1` + `0x03` cho từng core); R4 chỉ phản ứng.
4. **`power_cpu()` phát event nguồn ĐÚNG 1 LẦN** — khi core A72 cuối tắt / core đầu bật.
5. **`g_wolMode`** (không phải Linux nói) quyết SLEEP2 hay ErP.
6. **Khi ngủ, R4 giữ Ethernet** qua Iris ICU; proxy trả lời thay host để không đánh thức A72.
7. **Đánh thức** = mọi nguồn hội tụ về `SYS_EVENT_WAKE_SYSTEM` → `power_cpu(0,0)` → `pmu_resume`
   → `WAKEUP_S1` → `TR8/10` reload QoS → A72 warm boot.
8. **`WARMBOOT_FLAG` phân biệt cold/warm boot** khi thức; `wake_timer` cố tình ép cold.
9. **Tắt máy thật + reset A72 khi treo/flicker** = kéo `HRESET_REQN=0` từ ISR, **bỏ qua state machine**.
10. **2 kênh IPC:** SCPI/MHU (điều khiển, qua DDR shared-mem) + HCI FIFO (dữ liệu mạng + IOCTL, qua SRAM).
    **Tầng dưới R4:** PS-CPU qua I2C0 nắm phần cứng nguồn thật.
