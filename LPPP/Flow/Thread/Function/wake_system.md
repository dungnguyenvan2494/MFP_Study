Hàm này call đến hàm `SysApiThread_EventFlagsSet`

```c
void wake_system()
{
	SysApiThread_EventFlagsSet(SysSystem_SysEventGroup, SYS_THREAD_EVENTSETFLAG_OR, SYS_EVENT_WAKE_SYSTEM);
}
```

`SysApiThread_EventFlagsSet` là **lớp trừu tượng hoá OS** của Marvell SDK — nó che giấu sự khác biệt giữa ThreadX và FreeRTOS đằng sau một API chung. Nhưng bên trong nó có một hành vi rất dễ gây bất ngờ: **nó không set cờ ngay**.

## Mục đích: một cái chuông, không phải một cái hộp thư

`SysApiThread_EventFlagsSet` tồn tại để **một nơi bất kỳ trong firmware đánh thức `SysSystem_ThreadMain`** mà không cần biết task đó là ai, đang ở đâu, hay đang làm gì.

Nó cố tình **không mang dữ liệu** — chỉ mang một bitmask nói _"loại sự kiện gì đã xảy ra"_. Toàn bộ giá trị của nó nằm ở chỗ đó:

- **Người gửi** (ISR timer, ISR USB, ISR link) không cần con trỏ, không cần khoá, không cần biết ai đang chờ — chỉ bật một bit rồi đi tiếp.
- **Người nhận** (`SysSystem_ThreadMain`) ngủ 0% CPU trên `xEventGroupWaitBits`, thức dậy khi _bất kỳ_ bit nào bật, đọc cả cụm bit một lần rồi xử lý.
- Nhiều nguồn có thể bật **cùng một bit** trước khi người nhận kịp đọc — chúng gộp lại bằng OR chứ không xếp hàng. Đây là khác biệt cốt lõi với queue: queue đếm số lần, event flag chỉ nhớ "đã từng xảy ra".

Trong LPPP, công dụng thực tế lớn nhất là biến năm nguồn đánh thức khác nhau (wake timer, wake interrupt, USB, WoL, host command) thành **một điểm hội tụ duy nhất** dẫn tới `power_cpu(0, 0)` — bật lại CA72.

---

# Phần A — Chữ ký và bối cảnh

```c
MV_RC SysApiThread_EventFlagsSet(MV_HANDLE Handle,
                                 SYS_THREAD_EVENTSETFLAG Option,
                                 uint32_t RequestedFlags);
```

|Tham số|Ý nghĩa|
|---|---|
|`Handle`|Nhóm cờ, tạo bởi `SysApiThread_EventFlagsCreate`|
|`Option`|`_AND` hoặc `_OR` — **bị bỏ qua hoàn toàn**, xem §D.1|
|`RequestedFlags`|Bitmask cờ muốn bật|

Trong LPPP có hai nhóm cờ, tạo tại [sys_system_init.c:109-112](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Sys/system/sys_system_init.c#L109-L112):

```c
SysSystem_AppEventGroup = SysApiThread_EventFlagsCreate("EventGroupApp");
SysSystem_SysEventGroup = SysApiThread_EventFlagsCreate("EventGroupSys");
```

Nhóm `Sys` mang các cờ ở [sys_system.h:52-60](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Sys/system/sys_system.h#L52-L60):

```
SYS_EVENT_WAKE_SYSTEM   0x001      SYS_EVENT_LINK_UP        0x080
SYS_EVENT_HCU           0x004      SYS_EVENT_DRIVER_HELLO   0x100
SYS_EVENT_WOL_DETECTED  0x010      SYS_EVENT_DRIVER_GOODBYE 0x200
SYS_EVENT_LINK_DOWN     0x020      SYS_EVENT_GE_INIT        0x400
                                   SYS_EVENT_CAN_WAKE       0x800
```

---

# Phần B — Ba đường biên dịch, chỉ một được build

```c
#ifdef OS_threadx                    // ① ThreadX
    tx_event_flags_set(..., TX_OR);
#elif defined OS_freertos
  #ifdef USE_FREERTOS_EVENTS         // ② FreeRTOS event group   ← ĐANG DÙNG
    xEventGroupSetBitsFromISR(...);
  #else                              // ③ giả lập bằng queue
    xQueueSendFromISR(...);
  #endif
#endif
```

`USE_FREERTOS_EVENTS` được định nghĩa tại [FreeRTOSConfig.h:186](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Os/freertos/portable/GCC/quartz/FreeRTOSConfig.h#L186) → **nhánh ② là nhánh thật sự chạy**.

> Nhánh ③ (không build) còn chứa khối `xHigherPriorityTaskWoken` bị **lặp hai lần** — một lần _bên trong_ critical section (dòng 458-471) và một lần _sau khi_ thoát (dòng 475-479). Đây là dấu vết bản vá OP_BTS-22999 được chồng lên code cũ mà không xoá. Không ảnh hưởng vì không được biên dịch, nhưng cho thấy file đã mục.

---

# Phần C — Mổ xẻ nhánh đang chạy

```c
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
EventBits_t uxBits = 0;
//  Option parameter is not used here                     ← comment của chính tác giả

uint32_t previous_state;
previous_state = HalApiCpu_interrupt_control(ARM_INTERRUPT_DISABLE);   // ①

uxBits = xEventGroupSetBitsFromISR((EventGroupHandle_t)Handle,          // ②
                                   (const EventBits_t)RequestedFlags,
                                   &xHigherPriorityTaskWoken);
if (uxBits == pdFAIL) debug_event_set_fails++;                          // ③

HalApiCpu_interrupt_control(previous_state);                            // ④

if (xHigherPriorityTaskWoken) {                                         // ⑤
    if (HalApiCpu_is_irq_mode() || HalApiCpu_is_fiq_mode())
        portYIELD_FROM_ISR();
    else
        taskYIELD();
}

if (pdPASS != uxBits) return RC_ERROR;
```

## ①④ Critical section — một mẹo khá tối nghĩa

`ARM_INTERRUPT_DISABLE = 0xc1`. Hàm được biên dịch là bản FreeRTOS ([hal_cpu.c:68-81](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/src/hal_cpu.c#L68-L81), vì `USE_FREERTOS_CRITICAL_SECTIONING` được define ngay dòng 65):

```c
uint32_t HalApiCpu_interrupt_control(uint32_t state) {
    if (state & 1)    vPortEnterCritical();
    if (!(state & 1)) vPortExitCritical();
    return state & ~1;              // ← xoá bit 0
}
```

Cơ chế: `0xc1` có bit 0 = 1 → **vào** critical section, và trả về `0xc0` (bit 0 đã xoá). Lần gọi thứ hai với `0xc0` → bit 0 = 0 → **thoát** critical section.

Điểm cần lưu ý: biến tên `previous_state` nhưng **không hề chứa trạng thái cũ nào** — nó luôn bằng `0xc0` bất kể trước đó ngắt đang bật hay tắt. Cách viết này bắt chước mẫu save/restore CPSR của phiên bản ThreadX (nhánh `#else` không được build, thao tác trực tiếp CPSR bit 6/7). Với bản FreeRTOS thì `vPortEnterCritical`/`vPortExitCritical` **tự đếm nesting**, nên vẫn đúng — nhưng tên biến gây hiểu nhầm.

## ② Điểm bất ngờ lớn nhất: **hàm này là bất đồng bộ**

`xEventGroupSetBitsFromISR` **không set bit**. Nhìn vào chính bản FreeRTOS trong repo ([event_groups.c:719-727](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Os/freertos/event_groups.c#L719-L727)):

```c
BaseType_t xEventGroupSetBitsFromISR(EventGroupHandle_t xEventGroup,
                                     const EventBits_t uxBitsToSet,
                                     BaseType_t *pxHigherPriorityTaskWoken)
{
    return xTimerPendFunctionCallFromISR(vEventGroupSetBitsCallback,
                                         (void *)xEventGroup,
                                         (uint32_t)uxBitsToSet,
                                         pxHigherPriorityTaskWoken);
}
```

Nó chỉ **đẩy một yêu cầu "gọi hàm hộ tao" vào timer command queue**. Việc set bit thật sự do **timer daemon task** thực hiện sau đó, khi nó chạy và gọi `vEventGroupSetBitsCallback` → `xEventGroupSetBits`.

```
Task/ISR gọi SysApiThread_EventFlagsSet
        │
        ▼  xQueueSendFromISR vào timer command queue (30 slot)
   ┌──────────────────────────────────────────────┐
   │  Tmr Svc daemon task  ·  prio 3 (CAO NHẤT)   │
   │      vEventGroupSetBitsCallback              │
   │          → xEventGroupSetBits(group, bits)   │  ← BIT MỚI THỰC SỰ ĐƯỢC SET
   │          → đánh thức các task đang chờ       │
   └──────────────────────────────────────────────┘
        │
        ▼
   SysSystem_ThreadMain thoát khỏi xEventGroupWaitBits
```

Nghĩa là **khi hàm return, bit vẫn chưa được set**. Cấu hình liên quan ([FreeRTOSConfig.h:135-142](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Os/freertos/portable/GCC/quartz/FreeRTOSConfig.h#L135-L142)):

```c
#define configUSE_TIMERS                1
#define configTIMER_TASK_PRIORITY       (configMAX_PRIORITIES - 1)   // = 3, cao nhất
#define configTIMER_QUEUE_LENGTH        30
#define configTIMER_TASK_STACK_DEPTH    400
#define INCLUDE_xTimerPendFunctionCall  1      // ← bắt buộc, nếu thiếu sẽ không build được
```

Comment trong file ghi rõ chủ ý: _"give the timer task the highest priority"_. Nhờ prio 3 mà độ trễ rất ngắn — daemon giành CPU ngay sau `taskYIELD()`. Nhưng về nguyên tắc đây vẫn là **eventual delivery**, không phải tức thời.

## ③ Bộ đếm lỗi toàn cục

```c
uint32_t debug_event_set_fails = 0;
…
if (uxBits == pdFAIL) debug_event_set_fails++;
```

Đếm số lần **timer command queue đầy** (30 slot). Biến này không nằm trong header nào — chỉ để đọc qua debugger/JTAG khi điều tra sự cố "event bị mất". Đây là điểm hay: các queue khác trong firmware này chỉ log rồi quên, còn ở đây có bộ đếm thật.

> Chi tiết nhỏ: `uxBits` khai báo kiểu `EventBits_t` nhưng chứa giá trị `pdPASS`/`pdFAIL` kiểu `BaseType_t`. So sánh vẫn đúng vì cả hai là số nguyên, nhưng tên và kiểu đều sai ngữ nghĩa — dấu vết copy-paste từ `xEventGroupSetBits` (hàm đó mới thật sự trả về bitmask).

## ⑤ Yield ở đây khác hẳn các queue khác trong firmware

Ở các chỗ khác (`gst_MsgQHandle_PowerCtrl`, `got_msg_qhandle`), tất cả task đều ở `tskIDLE_PRIORITY = 0` nên `xHigherPriorityTaskWoken` gần như **không bao giờ** true. Ở đây thì ngược lại: task được đánh thức là **timer daemon ở prio 3**, cao hơn mọi task ứng dụng — nên cờ này **thường xuyên true** và `taskYIELD()` thật sự kích hoạt.

Thứ tự cũng đúng: yield nằm **sau** `HalApiCpu_interrupt_control(previous_state)`, tức sau khi đã thoát critical section. Nếu yield khi còn trong critical section thì FreeRTOS sẽ hoãn context switch, làm mất tác dụng.

Phân biệt `portYIELD_FROM_ISR()` với `taskYIELD()` dựa vào `HalApiCpu_is_irq_mode()` / `is_fiq_mode()` — cùng mẫu xuất hiện ở `mhu_handler`, `LPPP_GPIO_Callback`, `LPPP_LVDS_tick`. Comment giải thích lý do:

> _taskYIELD → portYIELD → SWI. Calling SWI in interrupt context results in an abort._

---

# Phần D — Vì sao thiết kế như vậy

## D.1 `Option` bị bỏ qua trên **cả hai** nền tảng

```c
//  Option parameter is not used here          ← FreeRTOS
tx_event_flags_set(..., TX_OR);                ← ThreadX: hard-code TX_OR
```

`SYS_THREAD_EVENTSETFLAG_AND` (nghĩa là "AND các cờ hiện tại với mask", dùng để **xoá** cờ) **không bao giờ được tôn trọng**. Mọi lời gọi đều là OR. Trong toàn bộ codebase, mọi chỗ đều truyền `SYS_THREAD_EVENTSETFLAG_OR`, nên không có bug thực tế — nhưng API hứa một thứ nó không làm được.

Đối chiếu: phía `EventFlagsGet` thì `Option` **có** được xử lý đầy đủ ([sys_thread_api.c:346-363](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Sys/thread/sys_thread_api.c#L346-L363)) — ánh xạ 4 tổ hợp AND/OR × CLEAR sang `xWaitForAllBits` / `xClearOnExit`.

## D.2 Vì sao dùng API `FromISR` ngay cả trong task context?

Đây là lựa chọn có chủ ý, và hệ quả nằm ngay ở hàm kế bên:

```c
MV_RC SysApiThread_EventFlagsISRSet(MV_HANDLE Handle, SYS_THREAD_EVENTSETFLAG Option, uint32_t RequestedFlags)
{
	/* SysApiThread_EventFlagsSet is already isr-safe. */
	return SysApiThread_EventFlagsSet(Handle, Option, RequestedFlags);
}
```

Bằng cách **luôn** dùng biến thể ISR-safe và bọc trong critical section, SDK có được một hàm gọi được từ **bất kỳ context nào** mà người dùng không cần biết mình đang ở đâu. Rất tiện cho một codebase mà cùng một cờ được set từ ISR (`lpp_usbd_isr`, `my_callback`) lẫn từ task (`SysSystem_handle_link_events`, `app_hostcmd`).

Cái giá phải trả chính là §C.2: **mọi** lời gọi, kể cả từ task, đều phải đi vòng qua timer daemon.

---

# Phần E — Toàn cảnh: ai set, ai chờ

## Phía chờ — chỉ một nơi

`SysSystem_ThreadMain` ([sys_system_thread.c:158-166](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Sys/system/sys_system_thread.c#L158-L166)):

```c
if (HalApiInit_DriverIsPresent()) {
    SysSystem_System_EventsGet(&Events);              // non-blocking (WaitTicks = 0)
} else {
    /* this call blocks. */
    SysApiThread_EventFlagsGet(SysSystem_SysEventGroup,
        SYS_THREAD_EVENTGETFLAG_OR_CLEAR, &Events,
        SYS_EVENT_ALL, EVENT_GET_WAIT_FOREVER);
}
SysSystem_handle_link_events(Events);
```

`OR_CLEAR` → `xClearOnExit = pdTRUE, xWaitForAllBits = pdFALSE`: chờ **bất kỳ** cờ nào trong `SYS_EVENT_ALL`, và **xoá sạch** sau khi đọc. Nên mỗi sự kiện chỉ được xử lý đúng một lần.

Hai chế độ tuỳ theo driver phía AP có mặt hay không: có driver → poll không block (vì còn việc khác phải làm trong vòng lặp); không có driver → block vô hạn, tiết kiệm điện tối đa.

## Phía set — nhiều nguồn

|Nguồn|Context|Cờ|
|---|---|---|
|`my_callback` (wake timer nổ)|ISR timer|`SYS_EVENT_WAKE_SYSTEM`|
|`wake_isr` (wake interrupt đăng ký qua SCPI `0x80`)|ISR|`SYS_EVENT_WAKE_SYSTEM`|
|`lpp_usbd_isr` (USB có hoạt động)|ISR|`SYS_EVENT_WAKE_SYSTEM` + `LPPP_SetWakeInfo(USB)`|
|`SysSystem_handle_link_events` (phát hiện WoL)|task|`SYS_EVENT_WAKE_SYSTEM` (tự phát cho chính mình)|
|Macro `SysSystem_SystemNotifyLinkUp/Down`|ISR|`SYS_EVENT_LINK_UP/DOWN`|

Và điểm đến cuối cùng của `SYS_EVENT_WAKE_SYSTEM` chính là chỗ đã phân tích ở lượt trước:

```c
if (Events & SYS_EVENT_WAKE_SYSTEM) {
    if (goodbye_processing == false)  power_cpu(0, 0);   // bật lại core 0 của CA72
    else                              wake_request = true;
}
```

---

# Phần F — Điểm cần lưu ý

**🟠 Hàm không đồng bộ nhưng tên không hề gợi ý.** `EventFlagsSet` nghe như set xong là xong. Thực tế bit chỉ được set sau khi timer daemon chạy. Nếu ai đó viết `EventFlagsSet(...)` rồi ngay dòng sau đọc lại cờ, sẽ thấy nó chưa có.

**🟠 Phụ thuộc ngầm vào timer daemon.** Nếu `configUSE_TIMERS = 0`, `INCLUDE_xTimerPendFunctionCall = 0`, hoặc timer task không tạo được vì thiếu heap (cần 400 words stack), thì **toàn bộ hệ thống sự kiện chết câm lặng** — không wake, không link event, không hostcmd. Không có kiểm tra nào cảnh báo tình huống này.

**🟡 `Option` là tham số giả.** Cả hai nền tảng đều hard-code OR. Nên xoá tham số hoặc ít nhất thêm `configASSERT(Option == ..._OR)`.

**🟡 Queue 30 slot có thể tràn.** Timer command queue dùng chung cho _mọi_ software timer và _mọi_ `xTimerPendFunctionCall` trong hệ thống. Khi tràn, cờ mất im lặng — chỉ `debug_event_set_fails` tăng lên, và biến này không được hiển thị ở bất kỳ log nào.

**🟡 `previous_state` là tên sai.** Nó luôn bằng `0xc0`, không phải trạng thái đã lưu. Đúng ngữ nghĩa thì nên đổi tên thành `exit_token` hoặc dùng thẳng `vPortEnterCritical`/`vPortExitCritical`.

## Output: cần tách làm hai thứ

### 1. Giá trị trả về — chỉ nói về việc _xếp hàng_

|Trả về|Nghĩa|
|---|---|
|`RC_OK`|Yêu cầu đã vào được timer command queue|
|`RC_ERROR`|Queue đầy (30 slot) → **sự kiện mất luôn**, `debug_event_set_fails++`|

⚠️ `RC_OK` **không** có nghĩa là cờ đã được set. Nó chỉ có nghĩa là "tao đã nhờ được người khác set hộ".

### 2. Tác dụng thật — xảy ra _sau khi_ hàm đã return

```
t0  gọi SysApiThread_EventFlagsSet(SysSystem_SysEventGroup, _OR, SYS_EVENT_WAKE_SYSTEM)
      → đẩy yêu cầu vào timer command queue
      → xHigherPriorityTaskWoken = pdTRUE  (daemon prio 3 > task hiện tại)

t1  hàm RETURN RC_OK          ← lúc này cờ VẪN CHƯA ĐƯỢC SET
      → taskYIELD()

t2  Tmr Svc daemon (prio 3) chạy
      → vEventGroupSetBitsCallback → xEventGroupSetBits()
      → SysSystem_SysEventGroup |= 0x001      ← BIT ĐƯỢC SET TẠI ĐÂY
      → SysSystem_ThreadMain chuyển sang Ready

t3  SysSystem_ThreadMain thoát khỏi xEventGroupWaitBits
      → Events = 0x001, cờ bị XOÁ (OR_CLEAR)
      → SysSystem_handle_link_events(0x001) → power_cpu(0, 0)
```

Tóm lại, ba thứ thay đổi sau khi hàm chạy xong:

|Đối tượng|Thay đổi|
|---|---|
|`SysSystem_SysEventGroup`|`|
|Task đang chờ|Được chuyển từ Blocked sang Ready ở `t2`|
|`debug_event_set_fails`|`++` nếu queue đầy (chỉ đọc được qua debugger)|

Cộng thêm một tác dụng phụ tức thời tại `t1`: `taskYIELD()` nhường CPU ngay cho daemon, nên khoảng cách `t1 → t2` thực tế rất ngắn.