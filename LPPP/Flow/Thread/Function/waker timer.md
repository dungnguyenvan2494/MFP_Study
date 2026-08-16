## Mục đích: đây là **đồng hồ báo thức** của LPPP

Trong toàn bộ hệ thống, mọi nguồn đánh thức khác đều là **sự kiện bên ngoài** — gói Wake-on-LAN tới, người dùng bấm panel, cắm USB, mở cửa máy, ngắt RTC. Lệnh `0x06` là đường duy nhất để hệ thống **tự hẹn giờ đánh thức chính mình** mà không cần bất kỳ kích thích nào từ bên ngoài.

Bối cảnh: khi Linux ngủ (Sleep2/ErP), **CA72 đã tắt hoàn toàn** — không còn CPU nào chạy để đếm giờ. Trước khi tắt, Linux phải nhờ LPPP: _"90 phút nữa gọi tao dậy"_. Trên một máy MFP, đây là cơ sở cho các tính năng như lịch bật máy theo giờ hành chính, bảo dưỡng đầu in định kỳ, hay in báo cáo theo lịch.

## `wake_timer` là gì

```c
static TimerDevice_t *restart_timer, *wake_timer;
```

Là **handle tới một bộ đếm phần cứng APB timer**. Chi tiết quan trọng nhất nằm ở tham số truyền vào:

```c
wake_timer = TimerOpen(-1);
```

`TimerOpen` nhận `uint8_t`, nên `-1` chuyển thành `0xFF` — và `0xFF` là hằng số có ý nghĩa ([timer_api.h:53](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/timer/include/timer_api.h#L53)):

```c
///choose from timer banks that are 'always on' not on power island
#define RANDOM_AO_TIMER 0xff
```

```c
if (TimerNum == RANDOM_AO_TIMER)
    return TimerOpenRange(platform_config->number_available,
                          platform_config->number_of_timers_ao);   // ← bank always-on
```

Đây **không phải viết tắt cẩu thả**, mà là lựa chọn có chủ ý: timer phải nằm trong bank **always-on**, không thuộc power island nào, vì `pmu_phase(pmu_suspend)` sẽ cắt điện các island khi vào Sleep2/ErP. Nếu lấy nhầm timer trên power island, nó sẽ chết cùng lúc với hệ thống và không bao giờ đánh thức được ai.

> Chi tiết dễ gây nhầm: `restart_timer` khai báo cùng dòng nhưng **không được dùng ở đâu cả** — biến chết.

## Từng dòng của `case 0x6`

```c
if (s_header->payload_size < 12) {          // ① case DUY NHẤT kiểm tra size
    header_dest->status = SCPI_E_SIZE;
    send_response = true;
    break;
}
wake_timer = TimerOpen(-1);                  // ② lấy timer always-on
if (wake_timer) {
    TimerConfig = TimerGetConfig(wake_timer);            // ③ CẤP PHÁT bộ nhớ
    count_low   = ((uint32_t *)&s_header->payload)[0];   // ④ chỉ lấy 32 bit thấp
    TimerConfig->RepCount  = 0;                          // ⑤ one-shot
    TimerConfig->Count     = count_low;
    TimerConfig->eTimebase = e_TIMEBASE_10_MS;           // ⑥ đơn vị 10 ms
    TimerConfig->userData  = wake_timer;                 // ⑦ tự truyền handle cho callback
    TimerConfig->FuncPtr   = my_callback;
    TimerSetConfig(wake_timer, TimerConfig);
    SysMemory_Free(TimerConfig);                         // ⑧ trả bộ nhớ
    TimerOn(wake_timer);                                 // ⑨ chạy
}
break;                                                    // ⑩ KHÔNG response
```

**① Kiểm tra `payload_size < 12`.** Đây là case duy nhất trong ~28 case kiểm tra kích thước payload. 12 byte = 3 word, nhưng code chỉ dùng word đầu. Tên biến `count_low` gợi ý word thứ hai đáng lẽ là `count_high` của một giá trị 64-bit — **phần cao bị bỏ hoàn toàn**.

**③⑧ `TimerGetConfig` cấp phát heap.** Header ghi rõ: _"It is responsibility of the caller api to free memory allocated for TIMER_CONFIG parameter"_ — hàm gọi `SysMemory_Alloc(sizeof(TIMER_CONFIG))`. Nên `SysMemory_Free` ở dòng ⑧ là bắt buộc, không phải thừa. Mẫu dùng ở đây là **get–modify–set–free**.

**⑤ `RepCount = 0` là one-shot.** Header nói single-shot là `RepCount = 1`, nhưng nhìn vào ISR thật ([timer.c:726](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/timer/apbtimer/src/timer.c#L726)):

```c
if ((TimerIsr->RepCount) == 0) {
    *(TimerIsr->control) = TIMER_TCR_R;          // tự tắt timer
    *(TimerIsr->terminal_count) = TIMER_TTCR_R;
}
```

`0` nghĩa là "nổ một lần rồi tự dừng". Đúng ý định, dù tài liệu header nói khác.

**⑥ Khoảng thời gian.** `Count × 10 ms`, với `TIMER_MAX_COUNT = 0xFFFFFFFF` → tối đa ~497 ngày. Thực tế không có giới hạn nào đáng lo.

**⑦ `userData = wake_timer`.** Timer tự truyền handle của chính nó cho callback, để `my_callback` biết phải đóng cái nào.

**⑩ Không trả response.** Linux gửi lệnh rồi đi ngủ luôn, không chờ.

## Chuỗi sự kiện khi timer nổ — đây mới là phần thú vị

```
Bộ đếm always-on chạy hết  →  timerISR  →  my_callback(wake_timer)
```

```c
void my_callback(void *userdata)
{
    TimerDevice_t *my_timer = (TimerDevice_t *)userdata;
    *(volatile uint32_t *)PLAT_WARMBOOT_FLAG_BASE = 0;   // ① XOÁ cờ warm boot
    TimerOff(my_timer);
    TimerClose(my_timer);                                 // ② trả timer về pool
    if (wake_timer == my_timer) wake_timer = NULL;
    wake_system();                                        // ③ phát event
}
```

**① Xoá cờ warm boot.** `power_cpu()` lúc tắt đã ghi `PLAT_WARMBOOT_FLAG_BASE = PLAT_WARMBOOT_FLAG_BASE` (một giá trị khác 0). `my_callback` **đặt lại về 0** — nghĩa là khi thức dậy bằng wake timer, ATF sẽ thấy cờ bằng 0 và thực hiện **cold boot** thay vì warm boot. Đây là khác biệt hành vi có chủ ý so với các nguồn wake khác: đánh thức theo lịch thì khởi động sạch, đánh thức do sự kiện thì khôi phục nhanh từ snapshot.

**③ `wake_system()`** chỉ set một cờ sự kiện:

```c
SysApiThread_EventFlagsSet(SysSystem_SysEventGroup, SYS_THREAD_EVENTSETFLAG_OR, SYS_EVENT_WAKE_SYSTEM);
```

Cờ này được system thread bắt ở [sys_system_thread.c:307](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Sys/system/sys_system_thread.c#L307):

```c
if (Events & SYS_EVENT_WAKE_SYSTEM) {
    if (goodbye_processing == false) {
        power_cpu(0, 0);          // ★ BẬT LẠI core 0 của CA72
    } else {
        wake_request = true;      // đang bàn giao driver → hoãn lại
        "wake delay";
    }
}
```

Và thế là **vòng tròn khép lại**: `power_cpu(0, 0)` chính là hàm đã phân tích ở lượt trước — nó thấy `cpu_state == ALL_CPU_OFF`, bèn chạy `pmu_resume`, phát `WAKEUP_S1` vào `gst_MsgQHandle_PowerCtrl`, đặt RVBAR, bàn giao GMAC, rồi thả core 0 khỏi reset.

```
SCPI 0x06 (Linux hẹn giờ)
      ↓
 wake_timer  ── ngủ Sleep2/ErP, CA72 tắt hẳn, timer always-on vẫn đếm ──
      ↓  hết giờ
 timerISR → my_callback → xoá cờ warmboot → wake_system()
      ↓  SYS_EVENT_WAKE_SYSTEM
 system thread → power_cpu(0, 0)
      ↓
 pmu_resume + WAKEUP_S1 → PwEvtMgr → LPPP_TR8_TR10 → SLEEP1
      ↓
 core 0 thoát reset → CA72 boot lại
```

Nhánh `goodbye_processing` (bản vá AR.1066939) xử lý trường hợp timer nổ đúng lúc driver mạng đang bàn giao — khi đó hoãn `power_cpu` lại tới khi nhận được `SYS_EVENT_CAN_WAKE`.

Xem [[]]
## Quan hệ với `case 7`

```c
case 7:  /* Cancel any pending wake timer */
    if (wake_timer) {
        TimerOff(wake_timer);
        TimerClose(wake_timer);
        wake_timer = NULL;
    }
    break;
```

Cặp `0x06` / `0x07` là **allocate / free**. Linux huỷ báo thức khi nó thức dậy sớm vì lý do khác (có người bấm nút, có gói mạng tới) — không huỷ thì timer vẫn nổ giữa lúc máy đang chạy bình thường, và `my_callback` sẽ gọi `power_cpu(0,0)` một cách vô nghĩa.

---

## Ba điểm đáng chú ý

**🟠 Rò rỉ timer nếu đặt hai lần liên tiếp.** `wake_timer = TimerOpen(-1)` **không đóng handle cũ**. Nếu Linux gửi `0x06` hai lần mà không xen `0x07`, handle đầu bị ghi đè và mất — timer phần cứng đó giữ `inUse[timerID] = BUSY` vĩnh viễn ([timer.c:308](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/timer/apbtimer/src/timer.c#L308)). Lặp đủ nhiều lần thì `TimerOpen` trả `0` và **lệnh im lặng không làm gì** (vì bọc trong `if (wake_timer)`), Linux không hề biết báo thức đã không được đặt.

**🟡 Không báo lỗi khi hết timer.** `if (wake_timer)` sai thì `break` luôn, không đặt `SCPI_E_*`, và vốn dĩ lệnh cũng không trả response. Linux đi ngủ với niềm tin đã hẹn giờ thành công.

**🟡 32 bit cao của count bị bỏ.** Kiểm tra `payload_size >= 12` cho thấy giao thức mang nhiều hơn một word, tên `count_low` cho thấy có phần cao — nhưng chỉ `[0]` được đọc. Với timebase 10 ms thì 32 bit đã cho ~497 ngày nên không ảnh hưởng thực tế, chỉ là chỗ dễ hiểu nhầm khi đọc code.