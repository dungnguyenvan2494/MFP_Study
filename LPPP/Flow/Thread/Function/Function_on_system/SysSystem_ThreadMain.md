# Phần A — Vị trí: đây là consumer duy nhất

`SysSystem_ThreadMain` được tạo tại [sys_system_init.c:123-126](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Sys/system/sys_system_init.c#L123-L126) qua `SysApiThread_Create` → priority `TASK_DEFAULT_PRIORITY = configMAX_PRIORITIES - 3 = 1`.

Điều này quan trọng: nó **cao hơn** toàn bộ các task ứng dụng (`PwEvtMgr`, `PwMHUEv`, `AppMhuTest`, `ClrWDT` — đều prio 0). Nên khi cờ được set và task này thức dậy, nó **giành CPU ngay** khỏi các task KM.

---

# Phần B — Vòng lặp: chỉ bước 1 phụ thuộc event

```c
while (1) {
    Events = 0;
    loop_count++;

    /* ① LẤY EVENT — hai chế độ */
    if (HalApiInit_DriverIsPresent()) {
        SysSystem_System_EventsGet(&Events);          // KHÔNG block (WaitTicks = 0)
    } else {
        SysApiThread_EventFlagsGet(SysSystem_SysEventGroup,
            SYS_THREAD_EVENTGETFLAG_OR_CLEAR, &Events,
            SYS_EVENT_ALL, EVENT_GET_WAIT_FOREVER);   // block vô hạn
    }

    SysSystem_handle_link_events(Events);   /* ② DISPATCH theo cờ */

    SysSystem_periodic_debug_prints();      /* ③ vô điều kiện */
    SysHostcmd_HandlePendingCmds();         /* ④ vô điều kiện */
    SysFifo_HandlePendingRequests();        /* ⑤ vô điều kiện */

#ifndef USE_IDLE_TASK
    vApplicationIdleHook();
#endif
}
```

**Ba trong năm bước chạy vô điều kiện** mỗi vòng, không quan tâm `Events` là gì. Đây là mấu chốt để hiểu §D.1.

**Hai chế độ ở bước ①:**

|`HalApiInit_DriverIsPresent()`|Hành vi|
|---|---|
|`true` (Linux đang chạy, driver đã nạp)|Poll không block → vòng lặp quay liên tục ở prio 1|
|`false` (chưa boot / đã ngủ)|Block vô hạn → 0% CPU, chỉ thức khi có cờ|

`OR_CLEAR` nghĩa là: chờ **bất kỳ** cờ nào trong `SYS_EVENT_ALL`, đọc cả cụm rồi **xoá sạch**. Nên mỗi sự kiện được xử lý đúng một lần, và nhiều cờ đến gần nhau sẽ được gom vào **một lần dispatch duy nhất**.

---

# Phần C — Bảng dispatch trong `SysSystem_handle_link_events`

Các `if` được kiểm tra **tuần tự, không `else`** — nên một `Events` mang nhiều bit sẽ kích hoạt nhiều nhánh trong cùng một lượt, theo đúng thứ tự này:

|Thứ tự|Cờ|Action|
|---|---|---|
|1|`SYS_EVENT_HCU` (0x004)|**Chỉ log** — xem §D.1|
|2|`SYS_EVENT_WAKE_SYSTEM` (0x001)|`power_cpu(0, 0)` — **bật lại CA72**|
|3|`SYS_EVENT_CAN_WAKE` (0x800)|Gỡ chốt `goodbye_processing`, chạy wake bị hoãn|
|4|`SYS_EVENT_DRIVER_HELLO` (0x100)|`HalApiEth_disable_irqs()` + báo lên app|
|5|`SYS_EVENT_DRIVER_GOODBYE` (0x200)|Bàn giao mạng: reset WoL, `SysLan_setup_gigabit_core()`, `app_connect_lan_isr()`, `HalApiEth_unmask_irqs()`|
|6|`SYS_EVENT_LINK_UP` (0x080)|Ghi timestamp, `SysSystem_NotifyLinkUp()`|
|7|`SYS_EVENT_LINK_DOWN` (0x020)|Tắt MAC RX/TX, báo IP stack, tắt LED, bật energy-detect, `HalApiEth_start_port(true)`|
|8|`SYS_EVENT_WOL_DETECTED` (0x010)|`HalApiWol_Disable()` rồi **tự phát lại** `SYS_EVENT_WAKE_SYSTEM`|
|9|_(không cờ nào)_|Kiểm tra hội tụ trạng thái — xem §D.3|

---

# Phần D — Bốn điều đáng chú ý về cách dispatch hoạt động

## D.1 `SYS_EVENT_HCU` không làm gì cả — và đó là chủ ý

```c
if (Events & SYS_EVENT_HCU) {
    SYS_DBG_MSG(..., "HCU Events=%08x\n", Events);      // chỉ có thế
}
```

Việc xử lý host command thật nằm ở `SysHostcmd_HandlePendingCmds()` — chạy **vô điều kiện** ở cuối vòng lặp. Nên nhiệm vụ duy nhất của bit HCU là **phá vỡ trạng thái block** ở bước ①, để vòng lặp quay thêm một lượt và bước ④ có cơ hội chạy.

Đây là mẫu thiết kế nhất quán trong cả vòng lặp: **cờ là chuông đánh thức, không phải mệnh lệnh.** Công việc thật do các hàm poll ở cuối vòng đảm nhiệm.

## D.2 `WOL_DETECTED` cần **hai vòng lặp** mới đánh thức được CPU

```c
if (Events & SYS_EVENT_WOL_DETECTED) {
    HalApiWol_Disable();
    SysApiThread_EventFlagsSet(SysSystem_SysEventGroup, _OR, SYS_EVENT_WAKE_SYSTEM);
}
```

Nó không gọi `power_cpu` trực tiếp mà **tự gửi cờ cho chính mình**. Vì nhánh `WAKE_SYSTEM` (thứ tự 2) đã đi qua trước nhánh `WOL_DETECTED` (thứ tự 8) trong cùng lượt, cờ mới này chỉ được xử lý ở **vòng lặp kế tiếp**:

```
vòng n   : WOL_DETECTED → HalApiWol_Disable() + gửi WAKE_SYSTEM
              ↓ (qua timer daemon, xem lượt trước)
vòng n+1 : WAKE_SYSTEM  → power_cpu(0, 0)
```

Cách này đảm bảo WoL luôn đi qua đúng một điểm vào duy nhất (`power_cpu`), giống hệt wake timer và wake interrupt — trả giá bằng một vòng lặp phụ.

## D.3 Khối cuối cùng là **level-triggered**, không gắn với cờ nào

```c
// packet reception may only start after goodbye
if (HalApiLan_LinkIsUp() && goodbye && !enable_packet_processing) {
    enable_packet_processing = true;
    if (!HalApiInit_DriverIsPresent()) {
        HalApiLan_MacRxTxEnable();
        SysApiLan_IpStackNotifyLinkUp();
    }
    KM_HalEth_LedControl(true);
}
```

Khối này chạy **mỗi lượt**, kiểm tra một điều kiện hợp của ba trạng thái. Lý do: link-up và driver-goodbye có thể đến theo **thứ tự bất kỳ**, nên không thể gắn hành động vào một cờ cụ thể. Thay vì viết logic xử lý cả hai thứ tự, tác giả đặt một điều kiện hội tụ chạy liên tục cho tới khi cả ba điều kiện cùng đúng. `enable_packet_processing` là cờ chống lặp.

Đây là **bộ dispatch edge-triggered chứa một kiểm tra level-triggered** — kỹ thuật hợp lý nhưng dễ bị bỏ sót khi đọc code.

## D.4 Cơ chế hoãn wake khi đang bàn giao driver

Ba biến `static` giữ trạng thái giữa các lượt:

```
DRIVER_GOODBYE đến  →  goodbye_processing = true      (đang bàn giao mạng cho AP)
        │
        ├─ WAKE_SYSTEM đến trong lúc này
        │     → KHÔNG gọi power_cpu
        │     → wake_request = true;  log "wake delay"
        │
CAN_WAKE đến  →  goodbye_processing = false
                 nếu wake_request → power_cpu(0, 0);  log "wake start"
```

Đây là bản vá `AR.1066939` với ghi chú _"Timing is not good for wake"_: nếu đánh thức CA72 ngay giữa lúc `SysLan_setup_gigabit_core()` đang cấu hình lại GMAC, phần cứng mạng sẽ ở trạng thái nửa vời. Sự kiện được **giữ lại** thay vì bỏ đi.

⚠️ Nhưng có một điểm bất đối xứng do thứ tự kiểm tra: `WAKE_SYSTEM` (thứ 2) đứng **trước** `CAN_WAKE` (thứ 3). Nếu cả hai đến trong cùng một batch, `WAKE_SYSTEM` sẽ đọc giá trị `goodbye_processing` **cũ** (còn `true`) → đặt `wake_request`, rồi ngay sau đó `CAN_WAKE` gỡ chốt và chạy `power_cpu`. Kết quả vẫn đúng, nhưng là nhờ may mắn về thứ tự chứ không phải thiết kế tường minh.

---

# Phần E — Trả lời trực tiếp: chuỗi hoàn chỉnh cho `SYS_EVENT_WAKE_SYSTEM`

Đây là luồng bạn đã theo từ đầu — nay khép kín:

```
① my_callback / wake_isr / lpp_usbd_isr
      SysApiThread_EventFlagsSet(SysEventGroup, _OR, SYS_EVENT_WAKE_SYSTEM)
            ↓  (xếp hàng vào timer command queue, KHÔNG set ngay)
② Tmr Svc daemon (prio 3)
      xEventGroupSetBits() → SysSystem_SysEventGroup |= 0x001
            ↓  đánh thức task đang chờ
③ SysSystem_ThreadMain (prio 1)  ← PREEMPT các task KM ở prio 0
      xEventGroupWaitBits trả về Events = 0x001, cờ được XOÁ
            ↓
④ SysSystem_handle_link_events(0x001)
      if (goodbye_processing == false)  →  power_cpu(0, 0)
            ↓
⑤ power_cpu(0, 0)  —  cpu_state == ALL_CPU_OFF nên chạy khối "core đầu tiên":
      CA72WDT_Start()                        watchdog 90 s
      Bregister_modify(B014_BEGIN_PHASE6)    ghi breadcrumb vào SPI-Flash
      pmu_phase(pmu_resume)                  bật lại các power domain
      xQueueSend(PowerCtrl, WAKEUP_S1)  ─────────┐
      RVBAR_0/1 = WARM_BOOT_ADDR                 │
      intDisable(MCIX2_1); clear_wake_ints();    │
      wake_lan(); SysApiSystem_Wake()            │  bàn giao GMAC về AP
      PWR_DN_RQ ↑ → ISO ↓ → PRCR = nReset        │  thả core 0 khỏi reset
            ↓                                     ↓
⑥ CA72 warm boot                        PwEvtMgr (prio 0)
                                          LPPP_TR8_TR10_Sleep2_Or_ErPToSleep1
                                            I2C → PS-CPU, B014 bit13,
                                            QoS MoChi + AP (~25 thanh ghi)
                                          pwr_sd: SLEEP2/ErP → SLEEP1
```

Sau bước ④ và ⑤, vòng lặp **vẫn chạy tiếp** ba bước vô điều kiện (`periodic_debug_prints`, `HandlePendingCmds`, `HandlePendingRequests`) rồi quay lại chờ cờ tiếp theo.

---

# Phần F — Hai điểm cần dè chừng

**🟠 `SYS_EVENT_GE_INIT` (0x400) không nằm trong `SYS_EVENT_ALL`.** Nó được định nghĩa ở [sys_system.h:59](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Sys/system/sys_system.h#L59) nhưng bị bỏ khỏi mask chờ, và không có nhánh `if` nào xử lý. Nếu ai đó set cờ này: task **không thức dậy** vì nó, và cờ **không bao giờ được xoá** vì `OR_CLEAR` chỉ xoá các bit trong `RequestedFlags`. Cờ chết vĩnh viễn trong event group.

**🟡 `power_cpu()` được gọi từ prio 1, đồng thời với PwEvtMgr ở prio 0.** Sự kiện `WAKEUP_S1` được đẩy vào queue ở giữa `power_cpu`, nhưng `SysSystem_ThreadMain` có priority cao hơn nên nó **chạy hết `power_cpu` trước**, rồi `PwEvtMgr` mới được lượt. Nghĩa là `LPPP_TR8_TR10` (nạp lại QoS) luôn chạy **sau khi** core đã thoát reset. Đây có lẽ là điều tác giả trông đợi, nhưng nó phụ thuộc hoàn toàn vào chênh lệch priority chứ không có cơ chế đồng bộ nào bảo đảm.