# Mục 0 · Phạm vi — LPPP có **hai** hệ timer riêng biệt

|            | **  APB TIMERS**                        | **CPU cycle counter**              |
| ---------- | --------------------------------------- | ---------------------------------- |
| Vị trí     | Peripheral `0xE8000800`                 | Bên trong lõi ARM                  |
| Driver     | `Hal/quartz/timer/apbtimer/src/timer.c` | `Hal/quartz/src/hal_cputimer.c`    |
| API        | `TimerOpen/SetConfig/On/Off/Close`      | `HalApiCpuTimer_UpdateTickCount()` |
| Đếm bằng   | Pulse train từ khối TIMEBASE2           | `cpu_get_ccount()` (CCNT)          |
| Ngắt riêng | ✅ 4 IRQ (GIC 38–41)                     | ❌ chỉ được gọi từ ISR khác         |
| Người dùng | `wake_timer` (SCPI `0x06`)              | FreeRTOS tick, `LPPP_LVDS_tick()`  |

Bài này phân tích **APB TIMERS** — đường mà `wake_timer` đi qua, nối tiếp mạch phân tích SCPI `0x06` ở các lượt trước.

> ⚠️ Một điểm quan trọng cần nói ngay: `Hal/inc/osm.h:142` có `#define ASSERT(...)` — **rỗng**. Toàn bộ `ASSERT()` trong `timer.c` và `arm_gic.c` bị biên dịch thành không có gì. Mọi bound-check trong phần dưới là **giả tưởng**. _(Đã xác nhận từ source.)_

---

# Mục 1 · Call flow

## 1.1 Nhánh khởi tạo (boot, một lần)

```
sys_init.c: SysApi_InitApplModules()
   └─ vTaskStartScheduler()
        └─ AppThread_Main (prio 1)              app_init.c:141
             ├─ MHU_init()
             ├─ ApiAppInit()
             └─ TimerInit()                     app_init.c:149  ★ ĐIỂM VÀO
                  ├─ timer_platform_get_config(&platform_config)
                  ├─ vòng lặp count 0..3:
                  │    ├─ count%4==0 → timer_init_block_of_4(0)
                  │    │     └─ Timer_get_hw_regs_base_address(0) → 0xE8000800
                  │    │        gán con trỏ register cho mytimers[0..3]
                  │    │        ghi TIAR để xoá ack cũ
                  │    ├─ timerMutex[count] = SysApiThread_MutexCreate()
                  │    └─ mytimers[count].{RepCount,FuncPtr,userData} = 0
                  └─ TimerInitialized = true
```

**Lưu ý:** `timer_start_time_usec()` đã bị comment với ghi chú _"**REVISIT** removed this timer due to interrupt complexity from APB block"_ → Timer1 (`USEC_TIMER`) được đặt gạch nhưng **không dùng**. Timer0 (`WATCHDOG`) có `WATCHDOG_NOT_AVAILABLE` → cũng không dùng. **Chỉ Timer2 và Timer3 thực sự khả dụng** (`AVAIL_TIMER = 2`).

## 1.2 Nhánh runtime (mỗi lần Linux hẹn giờ)

```
CA72/Linux gửi SCPI 0x06
   └─ mhu_handler [ISR] → got_msg_qhandle
        └─ AppMhuTest [task prio 0]
             └─ process_scpi_cmd() → case 0x6           app_scpi.c:722
                  ├─ TimerOpen(-1)                       ★ 0xFF = RANDOM_AO_TIMER
                  │    └─ TimerOpenRange(2, 4)
                  │         ├─ quét NGƯỢC: inUse[3] → inUse[2]
                  │         ├─ MutexGet(timerMutex[id], 1)
                  │         ├─ inUse[id] = BUSY
                  │         ├─ intDisable(TimerIDtoIntNum(id))
                  │         └─ intAttach(intNum, 0, timerISR, id)
                  ├─ TimerGetConfig()  → SysMemory_Alloc(sizeof(TIMER_CONFIG))
                  ├─ điền Count / eTimebase / RepCount / FuncPtr / userData
                  ├─ TimerSetConfig()  → copy vào mytimers[id]
                  ├─ SysMemory_Free(TimerConfig)
                  └─ TimerOn(timerPtr)
                       ├─ timerMode = (RepCount > 1) ? 1 : 0
                       └─ initHwTimer(id, mode, Count, eTimebase)   ★ CHẠM PHẦN CỨNG
```

## 1.3 Nhánh interrupt (khi timer nổ)

```
[HW] TTCR match → TISR bit set → TimerIRQ[n] level → GIC SPI 38+n
   └─ CPU nhận IRQ → vector → irq_handler()            arm_gic.c:231
        ├─ int_id = ICC_IAR & 0x3ff
        ├─ Handlers[int_id].Handler(handler_input)
        │    └─ timerISR(timerID)                       timer.c:712
        │         ├─ *int_ack_reg |= int_ack_bit        ★ xoá flag
        │         ├─ RepCount == 0 → tắt timer
        │         ├─ FuncPtr(userData) → my_callback()  app_scpi.c:159
        │         │     ├─ PLAT_WARMBOOT_FLAG_BASE = 0
        │         │     ├─ TimerOff() / TimerClose()    ⚠ mutex từ ISR!
        │         │     └─ wake_system() → SYS_EVENT_WAKE_SYSTEM
        │         └─ khôi phục TTCR nếu cần
        └─ ICC_EOIR = int_id
```

**Sau khi return:** CPU khôi phục context. Cờ `SYS_EVENT_WAKE_SYSTEM` được timer daemon set → `SysSystem_ThreadMain` (prio 1) → `power_cpu(0,0)` → CA72 warm boot.

---

# Mục 2 · Phân tích từng dòng — hai hàm cốt lõi

## 2.1 `initHwTimer()` — nơi phần cứng thực sự được lập trình

```c
static int initHwTimer(uint8_t TimerID, uint8_t TimerMode, uint32_t Count, TIME_BASE eTimebase)
{
    uint32_t intNum;

    intNum = TimerIDtoIntNum(TimerID);                       // ①
    intDisable(intNum);                                       // ②

    *mytimers[TimerID].int_ack_reg = mytimers[TimerID].int_ack_bit;   // ③

    *mytimers[TimerID].control = eTimebase | (TimerMode << 1);        // ④

    *mytimers[TimerID].terminal_count = Count;                        // ⑤
    intEnable(intNum);                                                // ⑥

    *mytimers[TimerID].control |= TIMER_TCR_EN;                       // ⑦

    return RC_OK;
}
```

**①** `TimerIDtoIntNum(id) = list_of_banks[id/4].base_int_num + id%4` = `INTNUM_TIMER0 + id` = `38 + id`. Với Timer3 → **41**.

**②** Tắt line ở GIC trước khi động vào cấu hình. Ngăn ngắt "nửa vời" khi TCR đã có timebase mới nhưng TTCR còn giá trị cũ.

**③** Ghi **thẳng** (`=`, không `|=`) giá trị `int_ack_bit` (`1<<TimerID`) vào **TIAR**. Xoá level interrupt còn tồn đọng từ lần chạy trước. Dùng `=` là **đúng** vì TIAR là write-only.

**④** Đây là dòng quan trọng nhất. `eTimebase` **đã được mã hoá sẵn ở đúng vị trí bit** — không cần shift:

```
e_TIMEBASE_10_MS = 0x40 = 0b0100_0000
                            ├─┘
                          bits[6:4] = 0b100 = 4  → TimebaseSel = 4 = 10 ms
```

`TimerMode << 1` đặt bit1 (ContMode). Vì `TIMER_TCR_EN` (bit0) **không** được OR vào, timer vẫn đứng yên sau lệnh này. Đây là ghi **toàn bộ thanh ghi** (`=`, không `|=`) → mọi field khác (Polarity bit3, TerminalMode bit2) bị xoá về 0.

**⑤** Nạp giá trị so sánh vào **TTCR** — 32 bit đầy đủ.

**⑥** Bật line ở GIC **trước** khi bật timer. Thứ tự này đúng: nếu bật timer trước, có một khe hẹp mà timer đã đếm nhưng GIC còn mask.

**⑦** `|=` để tạo **cạnh lên 0→1** trên bit Enable. Theo datasheet: _"The timer count is reset when the counter is enabled (Enable bit changes from 0 to 1)"_ — chính cạnh này reset counter về 0. Nếu dùng `=` với giá trị đầy đủ thì cũng được, nhưng `|=` (read-modify-write) diễn đạt rõ ý "chỉ bật enable, giữ nguyên cấu hình vừa ghi".

## 2.2 `timerISR()` — chạy trong ngắt

```c
static void timerISR(uint32_t timerID)
{
    TimerDevice_t *TimerIsr = &mytimers[timerID];              // ①

    *TimerIsr->int_ack_reg |= TimerIsr->int_ack_bit;           // ② ⚠ RMW trên reg write-only

    if ((TimerIsr->RepCount) == 0) {                           // ③ one-shot
        *(TimerIsr->control)        = TIMER_TCR_R;             //   = 0  → tắt hẳn
        *(TimerIsr->terminal_count) = TIMER_TTCR_R;            //   = 0x3FF ⚠ giá trị legacy
    }
    else if (TimerIsr->RepCount != TIMER_CONTINUOUS_MODE) {    // ④ N lần
        TimerIsr->RepCount--;
    }
    // RepCount == 0xFFFFFFFF → không đụng gì, chạy mãi

    if (TimerIsr->FuncPtr) {
        TimerIsr->FuncPtr(TimerIsr->userData);                 // ⑤ callback người dùng
    }

    if ((*(TimerIsr->control)) && (*(TimerIsr->terminal_count) != TimerIsr->Count)) {
        *(TimerIsr->terminal_count) = TimerIsr->Count;         // ⑥ vá sau sleep/resume
    }
}
```

**①** `timerID` đến từ `handler_input` đã đăng ký trong `intAttach(intNum, 0, timerISR, TimerID)` — đây là cơ chế cho phép **một hàm ISR duy nhất phục vụ cả 4 timer**.

**② Đây là một lỗi thật.** TIAR được datasheet đánh dấu `[W]` (write-only). Phép `|=` bắt buộc **đọc** TIAR trước. Giá trị đọc từ một thanh ghi write-only là không xác định. Nếu bus trả về giá trị có bit khác đang set, phép ghi sẽ **ack luôn interrupt của timer khác** → mất event. So sánh: `initHwTimer` (dòng 765) và `TimerReset` (dòng 491) đều dùng `=` — chỉ riêng ISR dùng `|=`.

**③** `RepCount == 0` là "one-shot": ghi `TCR = 0` xoá sạch cả Enable lẫn TimebaseSel. `TTCR = 0x3FF` — hằng `TIMER_TTCR_R` được chú thích _"Timer Terminal Count register reset"_, giá trị 10-bit của một phiên bản IP cũ. Trên chip này TTCR là 32-bit, nên đây là ghi một giá trị vô nghĩa vào thanh ghi đã bị vô hiệu hoá. Vô hại nhưng là dấu vết di sản.

**⑤ Callback chạy TRONG ngắt.** Với `wake_timer`, `FuncPtr = my_callback` — và `my_callback` gọi `TimerClose()`, hàm này lại gọi `SysApiThread_MutexGet()` → `xSemaphoreTake()`. **Gọi API task-only từ ISR** — xem Mục 11.

**⑥** Chỉ chạy khi timer còn enable (`*control != 0`). Comment giải thích: _"This can happen on sleep/wake resume of timer. Timer counts up, so 'resume' needed to lower the terminal count."_ Với one-shot thì `*control` vừa bị ghi 0 ở ③ → điều kiện false → không ghi đè. Đúng.

---

# Mục 3 · Register mapping

## 3.1 Truy ngược macro → địa chỉ thật

```
timer_config.h:38   TIMERS_0_REG = ((TIMERS_REGS_t*) IPS_IPS_APB_TMR_BASE)
regAddrs.h:72       IPS_IPS_APB_TMR_BASE = 0xE8000800     // apb_top::apb_timers0
```

`TIMERS_REGS_t` → `TIMERS_IPS_REGS_t` (`TIMERS_IPS_regheaders.h:58`). Peripheral: **APB TIMERS block trong apb_top**, 4 timer đồng nhất.

## 3.2 Bản đồ thanh ghi đầy đủ

|Offset|Địa chỉ|Tên|R/W|Chức năng|
|---|---|---|---|---|
|`+0x00`|`0xE8000800`|`TWR0`|R/W|Timer0 Watchdog|
|`+0x04`|`0xE8000804`|`TTCR0`|R/W|Timer0 Terminal Count|
|`+0x08`|`0xE8000808`|`TCR0`|R/W|Timer0 Control|
|`+0x0C`|`0xE800080C`|`TSR0`|R|Timer0 Status (count hiện tại)|
|`+0x10`|`0xE8000810`|`TISR`|R|**Raw interrupt status, bit[3:0]**|
|`+0x14`|`0xE8000814`|`TTCR1`|R/W||
|`+0x18`|`0xE8000818`|`TCR1`|R/W||
|`+0x1C`|`0xE800081C`|`TSR1`|R||
|`+0x20`|`0xE8000820`|`TIAR`|**W**|**Interrupt Acknowledge, bit[3:0]**|
|`+0x24`|`0xE8000824`|`TTCR2`|R/W||
|`+0x28`|`0xE8000828`|`TCR2`|R/W||
|`+0x2C`|`0xE800082C`|`TSR2`|R||
|`+0x30`|—|`reserved0`|||
|`+0x34`|`0xE8000834`|`TTCR3`|R/W|← `wake_timer` dùng cái này|
|`+0x38`|`0xE8000838`|`TCR3`|R/W||
|`+0x3C`|`0xE800083C`|`TSR3`|R||
|`+0x40/44`||`REV0/REV1`|R|IP revision tag|

## 3.3 Chi tiết bit — `TCRn` (Timer Control Register)

```
 31                          7  6   4  3      2       1        0
┌──────────────────────────┬──────┬────┬──────────┬────────┬────────┐
│        reserved          │ Time │Pol │ Terminal │ Cont   │ Enable │
│         (0)              │baseSel│arity│  Mode   │ Mode   │        │
└──────────────────────────┴──────┴────┴──────────┴────────┴────────┘
                mask 0xFFFFFF80  0x70  0x8    0x4      0x2      0x1
```

|Field|Giá trị|Ý nghĩa|
|---|---|---|
|`Enable` bit0|1|Chạy. Cạnh 0→1 **reset counter về 0**|
|`ContMode` bit1|0 / 1|0 = dừng khi đạt terminal; 1 = tự reset và chạy tiếp|
|`TerminalMode` bit2|0 / 1|0 = so với TTCR; 1 = so với xung ngoài|
|`Polarity` bit3|—|cực tính xung ngoài|
|`TimebaseSel` bits6:4|0..7|0=1µs, 1=10µs, 2=100µs, 3=1ms, 4=10ms, 5=100ms, 6=bus clk, 7=xung ngoài|

## 3.4 Giá trị trước/sau cho `wake_timer` (Timer3, timebase 10 ms, one-shot, Count = 6000)

|Bước|Địa chỉ|Trước|Sau|Bit thay đổi|
|---|---|---|---|---|
|③ TIAR|`0xE8000820`|(write-only)|`0x00000008`|ACK3 = 1 → xoá IRQ3|
|④ TCR3|`0xE8000838`|`0x00000000`|`0x00000040`|TimebaseSel 0→4; ContMode giữ 0; Enable giữ 0|
|⑤ TTCR3|`0xE8000834`|`0x000003FF`|`0x00001770`|toàn bộ 32 bit ← 6000|
|⑦ TCR3|`0xE8000838`|`0x00000040`|`0x00000041`|**bit0 = 1** → cạnh lên khởi động|

Trong ISR:

|Bước|Địa chỉ|Trước|Sau|
|---|---|---|---|
|② TIAR|`0xE8000820`|(RMW ⚠)|`|
|③ TCR3|`0xE8000838`|`0x00000041`|`0x00000000`|
|③ TTCR3|`0xE8000834`|`0x00001770`|`0x000003FF`|

## 3.5 GIC

```
regAddrs.h:98   AP_GIC1_GICD_BASE = 0xE8260000 + 0x1000 = 0xE8261000   (Distributor)
regAddrs.h:99   AP_GIC1_GICC_BASE = 0xE8260000 + 0x2000 = 0xE8262000   (CPU Interface)
```

Với Timer3 → int 41:

|Thao tác|Register|Index|Bit|
|---|---|---|---|
|`intEnable`|`ICD_ISER[41/32]` = `ICD_ISER[1]`|`0xE8261104`|`1 << (41%32)` = bit 9|
|`intDisable`|`ICD_ICER[1]`|`0xE8261184`|bit 9|
|Priority|`ICD_IPR[41/4]` = `ICD_IPR[10]`|`0xE8261428`|byte `41%4` = 1 → bits 15:8|
|Đọc ID|`ICC_IAR`|`0xE826200C`|`& 0x3FF`|
|Kết thúc|`ICC_EOIR`|`0xE8262010`|ghi `int_id`|

_(Địa chỉ GICD offset ICD_ISER/ICD_ICER/ICD_IPR là suy luận theo chuẩn GICv1/PL390 — **CẦN XEM TIẾP** `arm_gic` regheaders để xác nhận offset chính xác.)_

---

# Mục 4 · Hardware mechanism

```
C:   *control |= TIMER_TCR_EN
       ↓ compiler → STR r1,[r0]   (volatile → không bị tối ưu bỏ)
       ↓ AXI/APB bridge → APB write cycle @ 0xE8000838
BIT: TCR3.Enable 0 → 1
       ↓
LOGIC: bộ phát hiện cạnh lên trong khối timer → reset_counter pulse
       ↓
COUNTER: TSR3 (32-bit up-counter) ← 0
       ↓
GATE:  mỗi khi TIMEBASE2 phát xung 10 ms VÀ TCR3.Enable == 1 → counter++
       ↓
COMPARE: comparator so TSR3 với TTCR3 mỗi chu kỳ
       ↓  khi TSR3 == TTCR3 (== 6000)
FLAG:  TISR.Status3 ← 1  (và vì ContMode == 0, counter dừng tại đây)
       ↓
IRQ:   TimerIRQ[3] chuyển sang mức tích cực (LEVEL, không phải edge)
       ↓
GIC-D: SPI 41 pending → so priority với ICC_PMR (=0xFF, thấy tất cả)
       ↓
GIC-C: assert nIRQ tới lõi ARM
       ↓
CPU:   nếu CPSR.I == 0 → vào IRQ mode, PC ← vector IRQ
```

Điểm mấu chốt: interrupt là **level-sensitive** (_"Each timer generates a level interrupt"_). Nghĩa là nó **giữ nguyên** cho tới khi TIAR được ghi. Đây là lý do `timerISR` phải ack **trước** khi làm bất cứ việc gì khác — nếu không, khi `irq_handler` ghi `ICC_EOIR`, level vẫn còn assert → GIC lập tức kích ngắt lại → **interrupt storm**.

---

# Mục 5 · Timer mechanism

## 5.1 Clock đến từ đâu

Timer **không** có prescaler riêng. Nó nhận **pulse train** từ một khối chuyên dụng:

```
apb_top_regheaders.h:178
"TIMEBASE2 block is used to generate timing periods that are useful to other blocks
 in the system... By using a common configurable timebase, each block is not burdened
 with programmable dividers.
 The reference clock can be fixed to a stable time standard such as a 25MHz crystal oscillator.
 There are six output timebases provided - 1us, 10us, 100us, 1ms, 10ms, 100ms.
 Each output timebase is a train of single clock cycle pulses."
```

```
XTAL 25 MHz ──► TIMEBASE2 @ 0xE8001800 ──┬─► 1us   pulse train
                (TCR: ref freq config)    ├─► 10us
                                          ├─► 100us
                                          ├─► 1ms
                                          ├─► 10ms  ──┐
                                          └─► 100ms   │
                                                       ▼
                                          APB TIMERS: mux TimebaseSel[2:0]
                                                       ▼
                                             TSRn counter ++
```

Tần số tham chiếu: `timer_config.h:22` → `#define SYS_BYPASS_MHZ 25`. _(Đã xác nhận từ source, nhưng đây chỉ là hằng số dùng cho `timer_get_time_usec` — hàm này đang `return 0`.)_

⚠️ **`TIMEBASE2->TCR` không được ghi ở bất kỳ đâu trong codebase này.** `timer_bank_config_t` khai báo `.timebase = 0, .timebase_reg = 0` với comment _"timebase handled by system"_, và `timer.c` bỏ qua hai field đó. Datasheet nói: _"This register should be programmed immediately following boot, and should be adjusted whenever the frequency changes."_ → Firmware **dựa vào giá trị reset mặc định 25 MHz**. _(Suy luận: hoặc BootROM/U-Boot đã cấu hình, hoặc mặc định đủ đúng.)_

## 5.2 Bảng thông số

|Câu hỏi|Trả lời|Nguồn|
|---|---|---|
|Prescaler?|Không có trong timer. Chia tần nằm ở TIMEBASE2|apb_top doc|
|Counter đếm lên hay xuống?|**Đếm lên** từ 0|_"Timer counts up"_ — timer.c:744|
|Counter width?|**32 bit**|`TIMERS_TSR0_COUNT_MASK 0xffffffff`|
|Reset counter khi nào?|Cạnh lên của `TCR.Enable`|TCR doc|
|Overflow?|**Không xảy ra** — dừng/reset tại terminal count trước|TTCR doc|
|Compare hoạt động ra sao?|`TSRn == TTCRn` → sinh terminal event|TTCR doc|
|ContMode = 1|Counter reset về 0 ở chu kỳ timebase kế|TTCR doc|
|ContMode = 0|Counter **dừng tại** terminal count|TTCR doc|
|Flag set ở đâu?|`TISR.StatusN` (bit N)|TISR doc|
|Flag clear thế nào?|Ghi **1** vào `TIAR.ACKN` — **write-1-to-clear**|TIAR doc|
|Interrupt enable ở đâu?|**Không có trong peripheral** — chỉ ở GIC `ICD_ISER`|TISR doc: _"individually maskable using the Interrupt Controller's Interrupt Enable Register"_|

Điểm đáng chú ý: **peripheral này không có thanh ghi Interrupt Enable.** TISR luôn phản ánh raw status; việc che ngắt hoàn toàn do GIC đảm nhiệm. Đó là lý do `initHwTimer` phải gọi `intDisable/intEnable` chứ không thể mask ở tầng timer.

## 5.3 Đọc counter đang chạy

`TSR` doc mô tả ba cách:

1. Ghi `Enable = 0` → counter **khoá giá trị**, đọc TSR. Nhưng khi enable lại thì **counter về 0**.
2. Đọc TSR hai lần liên tiếp; hai giá trị giống nhau → hợp lệ.
3. Dựa vào IRQ.

`timer_get_count()` (timer.c:509) dùng cách đọc **một lần**, không có bảo vệ → có thể bắt trúng giá trị đang chuyển. `TimerGetCount()` (timer.c:535) có logic phức tạp hơn nhưng **side effect là zero hoá timer** (theo doc của chính API).

---

# Mục 6 · Interrupt flow đầy đủ

|#|Tầng|Sự kiện|Source code|
|---|---|---|---|
|1|Counter|`TSR3` đạt `TTCR3`|phần cứng|
|2|Peripheral|`TISR.Status3 ← 1`|phần cứng|
|3|Wire|`TimerIRQ[3]` level assert|phần cứng|
|4|GIC-D|SPI `41` pending; đã enable từ trước|`arm_gic.c:200` `ICD_ISER[1] = 1<<9`|
|5|GIC-D|so priority với `ICC_PMR = 0xFF`|`arm_gic.c:145`|
|6|GIC-C|assert `nIRQ`|phần cứng|
|7|CPU|`CPSR.I == 0` → IRQ mode, PC ← vector|FreeRTOS ARM port|
|8|Vector|nhảy tới `irq_handler`|`arm_gic.c:231`|
|9|ISR|`int_id = ICC_IAR & 0x3ff` → 41|`arm_gic.c:237`|
|10|Dispatch|`Handlers[41].Handler(Handlers[41].handler_input)`|`arm_gic.c:262`|
|11|Driver|`timerISR(3)`|`timer.c:712`|
|12|**Clear**|`TIAR|= 0x08` → level deassert|
|13|Logic|one-shot → `TCR3 = 0`, `TTCR3 = 0x3FF`|`timer.c:729-730`|
|14|Callback|`my_callback(wake_timer)`|`app_scpi.c:159`|
|15|EOI|`ICC_EOIR = 41`|`arm_gic.c:268`|
|16|CPU|trở lại context bị ngắt|port|

**Quan hệ GIC ↔ peripheral trên Cortex-R4 này:** GIC là **GICv1 (PL390)** gắn ngoài lõi R4 (R4 không có NVIC). Interrupt của timer là **SPI** (Shared Peripheral Interrupt) với `INTRL_GIC_OFFSET = 32` — 32 ID đầu dành cho SGI (0-15) và PPI (16-31) theo chuẩn ARM. Nên `INTNUM_TIMER0 = 6 + 32 = 38` nghĩa là **SPI số 6** của SoC.

⚠️ `irq_handler` gọi `Handlers[int_id].Handler(...)` **không kiểm tra NULL**. Nếu một ngắt được enable mà chưa attach → nhảy vào `NULL` → Prefetch Abort. `ASSERT` trong `gic_intenable` đã bị vô hiệu hoá.

---

# Mục 7 · State machine

```
                    ┌──────────┐
                    │  RESET   │  TCR=0, TTCR=0x3FF, inUse[]=?
                    └────┬─────┘
          TimerInit()    │  timer_init_block_of_4()
          gán con trỏ,   │  đọc TCRn.Enable → BUSY hay IDLE
          tạo mutex      ▼
                    ┌──────────┐
                    │ INIT'ED  │  inUse=IDLE, FuncPtr=NULL
                    └────┬─────┘
          TimerOpen()    │  inUse=BUSY; intAttach(); intDisable()
                         ▼
                    ┌──────────┐
                    │  OPENED  │  ISR đã gắn, GIC còn tắt
                    └────┬─────┘
        TimerSetConfig() │  chỉ ghi RAM (mytimers[]), CHƯA chạm register
                         ▼
                    ┌──────────┐
                    │CONFIGURED│
                    └────┬─────┘
             TimerOn()   │  initHwTimer(): TIAR=bit; TCR=tb|mode;
             ────────────┤  TTCR=Count; intEnable; TCR|=EN  ← cạnh 0→1
                         ▼
                    ┌──────────┐◄──────────────┐
                    │ RUNNING  │  TSR đếm lên  │ ContMode=1:
                    └────┬─────┘                │ counter tự reset
         TSR == TTCR     │                      │
                         ▼                      │
                    ┌──────────┐                │
                    │ MATCHED  │  TISR.StatusN=1│
                    └────┬─────┘                │
         GIC pending     │                      │
                         ▼                      │
                    ┌──────────┐                │
                    │ INT PEND │                │
                    └────┬─────┘                │
         irq_handler     │                      │
                         ▼                      │
                    ┌──────────┐                │
                    │ SERVICED │  TIAR ghi ─────┤
                    └────┬─────┘                │
        RepCount==0      │  TCR=0, TTCR=0x3FF   │  RepCount>0
        ─────────────────┤──────────────────────┘
                         ▼
                    ┌──────────┐
                    │ STOPPED  │
                    └────┬─────┘
         TimerClose()    │  TCR=0; TTCR=0x3FF; inUse=IDLE; intDisable()
                         │  ⚠ KHÔNG intDetach → Handlers[] vẫn giữ timerISR
                         ▼
                    ┌──────────┐
                    │ INIT'ED  │
                    └──────────┘
```

**Bảng transition:**

|Từ → Đến|Trigger|Register/hành động|
|---|---|---|
|RESET → INIT'ED|`TimerInit()`|đọc `TCRn.Enable`; ghi `TIAR`|
|INIT'ED → OPENED|`TimerOpen()`|`ICD_ICER`; `Handlers[]`|
|OPENED → CONFIGURED|`TimerSetConfig()`|chỉ RAM|
|CONFIGURED → RUNNING|`TimerOn()`|`TIAR`, `TCR`, `TTCR`, `ICD_ISER`, `TCR.Enable`|
|RUNNING → MATCHED|`TSR == TTCR`|phần cứng set `TISR`|
|SERVICED → RUNNING|ContMode=1|phần cứng tự reset counter|
|SERVICED → STOPPED|`RepCount == 0`|`TCR = 0`|
|RUNNING → STOPPED|`TimerOff()`|`TCR = 0`; `ICD_ICER` ⚠ **không ghi TIAR**|
|bất kỳ → INIT'ED|`TimerClose()`|`TCR=0`, `TTCR=0x3FF`, `inUse=IDLE`|

---

# Mục 8 · Timeline một lần chạy

Giả định: Timer3, `e_TIMEBASE_10_MS`, `Count = 6000`, `RepCount = 0` (one-shot) → timeout 60 s.

|t|CPU làm gì|Hardware làm gì|
|---|---|---|
|**t0**|`AppMhuTest` nhận SCPI `0x06`, gọi `TimerOpen(-1)`|—|
|**t0+**|`inUse[3]=BUSY`; `intAttach(41,0,timerISR,3)` → ghi `ICD_IPR[10]` byte1 = 0|GIC ghi nhận priority|
|**t1**|`TimerGetConfig` → malloc; điền struct; `TimerSetConfig` copy vào `mytimers[3]`|— (chưa chạm register)|
|**t2**|`TimerOn` → `initHwTimer(3, 0, 6000, 0x40)`||
|t2.1|`intDisable(41)` → `ICD_ICER[1] = 1<<9`|GIC mask SPI 41|
|t2.2|`TIAR = 0x08`|`TISR.Status3` ← 0, `TimerIRQ[3]` deassert|
|t2.3|`TCR3 = 0x40`|TimebaseSel=4 (10 ms); Enable vẫn 0 → **counter đứng yên**|
|t2.4|`TTCR3 = 6000`|comparator nạp ngưỡng|
|t2.5|`intEnable(41)` → `ICD_ISER[1] = 1<<9`|GIC unmask|
|**t3**|`TCR3|= 0x01`→`0x41`|
|t3+10ms|(CPU làm việc khác)|TIMEBASE2 phát xung 10 ms → `TSR3 = 1`|
|t3+20ms||`TSR3 = 2`|
|…||…|
|**t4 = t3+60s**||`TSR3` đạt `6000` = `TTCR3`|
|**t5**||`TISR.Status3 ← 1`; ContMode=0 → counter **dừng** ở 6000|
|**t6**||`TimerIRQ[3]` level assert → GIC-D pending → GIC-C assert `nIRQ`|
|**t7**|CPU: CPSR.I==0 → IRQ mode; PC ← vector; `irq_handler()`; `ICC_IAR` → 41|GIC chuyển SPI 41 sang trạng thái _active_|
|**t8**|`timerISR(3)`||
|**t9**|`TIAR|= 0x08`|
|t9.1|`RepCount==0` → `TCR3 = 0`; `TTCR3 = 0x3FF`|timer tắt hẳn|
|t9.2|`my_callback()`: `WARMBOOT_FLAG=0`; `TimerOff()`; `TimerClose()`; `wake_system()`|`ICD_ICER[1]=1<<9`; `inUse[3]=IDLE`|
|t9.3|kiểm tra `*control` = 0 → **không** khôi phục TTCR|—|
|**t10**|`ICC_EOIR = 41`; return từ IRQ|GIC hạ _active_; SPI 41 về _inactive_|
|t10+|Timer daemon set `SYS_EVENT_WAKE_SYSTEM` → `SysSystem_ThreadMain` → `power_cpu(0,0)`|—|

---

# Mục 9 · Data flow và công thức

```
Linux tính timeout (ms)
   │
   ▼  đóng gói vào SCPI payload word 0
scpi->payload[0]                                    [uint32]
   │
   ▼  app_scpi.c:731
count_low = ((uint32_t*)&s_header->payload)[0]      ⚠ chỉ 32 bit thấp
   │
   ▼  app_scpi.c:734  TimerConfig->Count = count_low
   ▼  app_scpi.c:736  TimerConfig->eTimebase = e_TIMEBASE_10_MS (0x40)
   │
   ▼  TimerSetConfig → mytimers[3].Count / .eTimebase
   │
   ▼  TimerOn → initHwTimer(3, 0, Count, 0x40)
   │
   ├──► TCR3 bits[6:4] = (0x40 & 0x70) >> 4 = 4  →  T_tick = 10 ms
   └──► TTCR3 = Count
```

## Công thức

Từ TIMEBASE2 (f_ref = 25 MHz, T_ref = 40 ns) và bảng TimebaseSel:

$$T_{tick}(sel) = 10^{-6} \times 10^{,sel} \ \text{[s]},\quad sel \in {0..5}$$

$$N_{ref}(sel) = \frac{T_{tick}}{T_{ref}} = \frac{10^{-6}\times 10^{sel}}{40\times10^{-9}} = 25 \times 10^{sel}$$

$$T_{timeout} = \text{TTCR} \times T_{tick} = \text{Count} \times 10^{-6}\times 10^{sel}$$

## Kiểm tra bằng số

Với `sel = 4` (10 ms), `Count = 6000`:

- `N_ref(4)` = 25 × 10⁴ = **250 000** chu kỳ 25 MHz cho mỗi tick → 250 000 × 40 ns = 10 ms ✓
- `T_timeout` = 6000 × 10 ms = **60 000 ms = 60 s** ✓
- Kiểm chứng ngược: 6000 × 250 000 = 1.5 × 10⁹ chu kỳ ref × 40 ns = 60 s ✓

## Dải giá trị

|                           | Giá trị                                  |
| ------------------------- | ---------------------------------------- |
| Độ phân giải nhỏ nhất     | `sel=0`, Count=1 → **1 µs**              |
| Timeout dài nhất (10 ms)  | `0xFFFFFFFF × 10 ms` ≈ **497,1 ngày**    |
| Timeout dài nhất (100 ms) | ≈ **4971 ngày**                          |
| Sai số lượng tử           | ± 1 tick (± 10 ms ở cấu hình wake_timer) |

Với dải 497 ngày, việc bỏ 32 bit cao của payload là vô hại trên thực tế.
Với dải 497 ngày, việc bỏ 32 bit cao của payload là vô hại trên thực tế.

---

# Mục 10 · Hidden dependencies

|Trạng thái|File|Vì sao cần|
|---|---|---|
|✅ Đã có|`timer.c`, `timer_api.h`, `timer_config.c/h`|driver + platform map|
|✅ Đã có|`TIMERS_IPS_regheaders.h`|định nghĩa bit đầy đủ|
|✅ Đã có|`regAddrs.h`, `intnums.h`|địa chỉ + số hiệu ngắt|
|✅ Đã có|`int_common.c`, `arm_gic.c`|tầng interrupt|
|✅ Đã có|`apb_top_regheaders.h`|mô tả TIMEBASE2|
|❌ **CẦN XEM TIẾP**|**`TIMEBASE2` init code** (chưa tìm thấy trong repo)|Xác nhận `TIMEBASE2->TCR` được ghi ở đâu, ref clock thật là bao nhiêu. Nếu không ai ghi → mọi timing dựa vào default. **Đây là lỗ hổng lớn nhất trong hiểu biết hiện tại.**|
|❌ **CẦN XEM TIẾP**|**`arm_gic` regheaders** (`GIC_DIST_REGS_t`)|Xác nhận offset thật của `ICD_ISER/ICD_ICER/ICD_IPR` — hiện là suy luận theo chuẩn GICv1|
|❌ **CẦN XEM TIẾP**|**`interrupt_config.c` (asic/a0)**|`NUM_GIC_SUPPORTED`, `NUM_GIC_INTS`, cấu hình level/edge (`ICD_ICFR`) cho SPI 38–41. Cần để xác nhận timer IRQ được cấu hình **level** đúng như datasheet mô tả|
|❌ **CẦN XEM TIẾP**|**FreeRTOS ARM port `port.c`/`portASM.S`**|Vector IRQ, cách `irq_handler` được gọi, `uxCriticalNesting`|
|❌ **CẦN XEM TIẾP**|**Datasheet 88PAQZ21 mục APB Timers + TIMEBASE2**|Xác nhận cực tính, timing chính xác của cạnh Enable, hành vi đọc TIAR|

**Đánh dấu độ tin cậy các kết luận chính:**

|Kết luận|Mức|
|---|---|
|Base `0xE8000800`, bản đồ offset, bit field TCR/TTCR/TSR/TISR/TIAR|✅ Đã xác nhận từ source|
|`e_TIMEBASE_*` được mã hoá sẵn ở bits[6:4]|✅ Đã xác nhận (đối chiếu enum với mask 0x70/shift 4)|
|Interrupt là level-sensitive, TIAR là write-1-to-clear|✅ Đã xác nhận từ doc trong regheader|
|`INTNUM_TIMER0..3` = 38..41 (SPI 6..9)|✅ Đã xác nhận|
|ASSERT rỗng|✅ Đã xác nhận (`osm.h:142`)|
|Ref clock 25 MHz|🟡 Suy luận (`SYS_BYPASS_MHZ 25` + doc TIMEBASE2)|
|TIMEBASE2 dùng giá trị reset|🟡 Suy luận (không tìm thấy code ghi)|
|Offset GICD cụ thể|🟠 Chưa đủ thông tin|
|Cấu hình level/edge của SPI 38–41 tại GIC|🟠 Chưa đủ thông tin|

---

# Mục 11 · Lỗi và điểm nguy hiểm

## 🔴 11.1 `ASSERT` bị vô hiệu hoá toàn cục

```c
// Hal/inc/osm.h:142
#define ASSERT(...)
```

Hệ quả trong timer path:

- `ASSERT(TimerID < MAX_NUM_TIMER)` trong `TimerIDtoIntNum` → truy cập `list_of_banks[id/4]` ngoài mảng nếu id sai.
- `ASSERT(Handlers[int_num].Handler == NULL)` trong `gic_intattach` → **cho phép ghi đè handler im lặng** (xem 11.2).
- `ASSERT(config->Count <= TIMER_MAX_COUNT)` → vô nghĩa (Count là uint32, luôn ≤ 0xFFFFFFFF).

## 🔴 11.2 Read-modify-write trên thanh ghi write-only

```c
*TimerIsr->int_ack_reg |= TimerIsr->int_ack_bit;    // timer.c:725
```

TIAR được đánh dấu `[W]`. Đọc nó trả về giá trị không xác định. Nếu bus trả về bit của timer khác đang set → phép ghi **ack nhầm interrupt của timer đó** → mất event vĩnh viễn (level bị hạ mà ISR chưa chạy). Sửa: đổi `|=` thành `=`, đồng bộ với `initHwTimer` và `TimerReset`.

## 🔴 11.3 Gọi API task-only từ ISR context

`my_callback` (chạy trong `timerISR`) gọi `TimerClose()`, hàm này gọi:

```c
SysApiThread_MutexGet(timerMutex[timerID], 1)   →   xSemaphoreTake(handle, 1)
```

`xSemaphoreTake` là **task-only API**. Với `ticks = 1` và mutex đang rảnh, nó đi đường nhanh và "chạy được". Nhưng nếu một task khác đang giữ `timerMutex[3]` đúng lúc đó, FreeRTOS sẽ gọi `xTaskPriorityInherit()` và `vTaskPlaceOnEventList()` **từ ISR** → hỏng danh sách task của scheduler. Đây là race hiếm nhưng hậu quả nghiêm trọng.

## 🟠 11.4 Con trỏ NULL nếu timer đang chạy lúc boot

```c
inUse[base+0] = TIMERS_TCR0_ENABLE_MASK_SHIFT(tmr->TCR0) ? BUSY : IDLE;
if (!inUse[base+0]) {           // ← CHỈ gán con trỏ khi IDLE
    mytimers[base+0].terminal_count = &tmr->TTCR0;
    ...
}
```

Nếu BootROM/U-Boot để lại `TCRn.Enable = 1`, timer đó bị đánh dấu BUSY và **`terminal_count`/`control`/`status`/`int_ack_reg` giữ nguyên NULL**, `int_ack_bit = 0`. `TimerOpenRange` sẽ không cấp phát nó (vì BUSY), nên hiện tại an toàn — nhưng bất kỳ đường nào truy cập `mytimers[i]` theo index sẽ dereference NULL.

## 🟠 11.5 Rò rỉ timer trong `case 0x6`

```c
wake_timer = TimerOpen(-1);       // KHÔNG đóng handle cũ
```

Gọi `0x06` hai lần liên tiếp không xen `0x07` → handle đầu mất, `inUse[id]` kẹt BUSY vĩnh viễn. Chỉ có 2 timer khả dụng (index 2 và 3), nên **sau 2 lần là hết** → `TimerOpen` trả `0` → lệnh im lặng không làm gì (bọc trong `if (wake_timer)`), Linux không được báo lỗi.

## 🟠 11.6 `TimerClose` không detach handler

```c
#ifdef __REVISIT__  // what is the equivalent of intDetach in Rialto?
    intDetach(TimerIDtoIntNum(timerID));
#endif
```

`Handlers[41].Handler` vẫn là `timerISR` sau khi đóng. Lần `TimerOpen` sau gọi `intAttach` → `gic_intattach` lẽ ra phải assert nhưng ASSERT rỗng → **ghi đè im lặng**. Vô hại vì handler giống nhau, nhưng che mất một lớp bảo vệ thật.

## 🟠 11.7 `RepCount` vs `rep_count` — hai biến, một biến chết

`timerISR` chỉ đọc/ghi `RepCount`. `rep_count` được tính trong `TimerSetConfig` và `TimerReset` rồi **chỉ được đọc lại trong `TimerGetCount`**. Toàn bộ cơ chế `rep_count` không tham gia vào logic đếm. Comment ngay tại chỗ đọc cũng thừa nhận: `// ??? different meanings to the values`.

## 🟠 11.8 Race trên `RepCount` giữa ISR và task

`RepCount` bị `timerISR` giảm (ISR context) và bị `TimerSetConfig`/`TimerReset` ghi (task context), **không có bảo vệ nào**. `timerMutex[]` chỉ dùng cho `inUse[]` trong Open/Close. Với timer N-lần đang chạy, một `TimerReset` đồng thời có thể làm mất một lần đếm.

## 🟡 11.9 `TimerSetConfig` sửa struct của caller

```c
timerPtr->rep_count = (config->RepCount > 1) ? --(config->RepCount) : config->RepCount;
```

`--(config->RepCount)` **mutate tham số đầu vào**. Trong `case 0x6` thì `TimerConfig` được `SysMemory_Free` ngay sau nên vô hại, nhưng đây là side effect ẩn trên một hàm tên `Set*`.

## 🟡 11.10 Mutex put hai lần cho một get

```c
SysApiThread_MutexGet(timerMutex[timerID], 1);   // timer.c:302
inUse[timerID] = BUSY;
SysApiThread_MutexPut(timerMutex[timerID]);      // timer.c:309
... intAttach ...
SysApiThread_MutexPut(timerMutex[timerID]);      // timer.c:323  ← thừa
```

Mutex FreeRTOS không đệ quy → `xSemaphoreGive` lần hai trả `pdFALSE`, bị bỏ qua. Vô hại nhưng sai, và làm vùng `intAttach` **không được bảo vệ** như tác giả tưởng.

## 🟡 11.11 `TimerOff` không xoá TIAR

```c
void TimerOff(TimerDevice_t *timerPtr) {
    *timerPtr->control = TIMER_TCR_R;
    intDisable(TimerIDtoIntNum(timerPtr->timer_id));
}
```

Nếu timer vừa nổ đúng lúc gọi `TimerOff`, `TISR` còn set và GIC còn pending. Lần `intEnable` kế tiếp sẽ kích ISR ngay. May là `initHwTimer` xoá TIAR trước `intEnable` → được che. Nhưng nếu ai đó gọi `intEnable` trực tiếp thì sẽ dính.

## 🟡 11.12 Timing drift và phụ thuộc TIMEBASE2

Vì `TIMEBASE2->TCR` không được firmware ghi, mọi timing phụ thuộc giá trị reset. Datasheet cảnh báo phải chỉnh lại _"whenever the frequency changes (if using a programmable system PLL)"_. Nếu `pmu_phase()` hay `HalApiChip_set_speed()` đổi PLL hệ thống, **wake timer sẽ lệch giờ** mà không có cơ chế bù nào. **Đây là rủi ro cần kiểm chứng bằng đo thực tế.**

## 🟡 11.13 `volatile` đúng chỗ

Tích cực: `struct TimerDevice_s` khai báo `volatile uint32_t *terminal_count/control/status/watchdog/int_ack_reg`, và `TIMERS_IPS_REGS_t` cũng `volatile` từng field. Nên compiler không thể gộp/xoá các lần truy cập register. Đây là phần code làm **đúng**.

---

# Mục 12 · Tổng kết theo A–E

## `initHwTimer()`

**A. Làm gì?** Chuyển cấu hình đang nằm trong RAM (`mytimers[id]`) thành trạng thái phần cứng thật và khởi động bộ đếm.

**B. Register bị ảnh hưởng?** `TIAR` (`0xE8000820`), `TCRn`, `TTCRn`, và `ICD_ICER`/`ICD_ISER` của GIC.

**C. Hardware phản ứng?** Xoá level interrupt tồn đọng → nạp bộ chia timebase → nạp ngưỡng so sánh → mở đường ngắt ở GIC → cạnh lên Enable reset counter về 0 và bắt đầu đếm theo pulse train.

**D. Vị trí trong flow?** Là **ranh giới duy nhất giữa phần mềm và phần cứng** trong chiều đi. Mọi hàm phía trên (`TimerOpen/SetConfig/On`) chỉ thao tác RAM; chỉ hàm này chạm thanh ghi.

**E. Xem tiếp gì?** `interrupt_config.c` để xác nhận SPI 38–41 được cấu hình level-sensitive; datasheet APB Timers để xác nhận timing của cạnh Enable.

## `timerISR()`

**A. Làm gì?** Ack ngắt phần cứng, cập nhật bộ đếm số lần lặp, gọi callback người dùng, và vá lại TTCR nếu bị hỏng sau resume.

**B. Register bị ảnh hưởng?** `TIAR` (⚠ RMW sai), `TCRn`, `TTCRn`.

**C. Hardware phản ứng?** Ghi TIAR hạ level `TimerIRQ[n]` — nếu bỏ bước này, GIC sẽ kích lại ngay sau `ICC_EOIR` → interrupt storm. Với one-shot, ghi `TCR = 0` dừng hẳn bộ đếm.

**D. Vị trí trong flow?** Là **ranh giới phần cứng → phần mềm** trong chiều về, và là **điểm nối duy nhất** giữa tầng HAL timer và tầng ứng dụng (qua `FuncPtr`).

**E. Xem tiếp gì?** FreeRTOS ARM port (`portASM.S`) để hiểu context save/restore quanh `irq_handler`; và kiểm tra xem callback nào khác ngoài `my_callback` cũng gọi API task-only.

## Toàn module

**A.** Cung cấp 4 timer đếm-lên 32-bit dùng chung một pulse-train timebase, với API one-shot / N-lần / liên tục.

**B.** Toàn bộ block `0xE8000800–0xE8000844` + GIC SPI 38–41.

**C.** Peripheral không có Interrupt Enable riêng — mọi việc che ngắt do GIC đảm nhiệm; interrupt là level và phải ack bằng write-1-to-TIAR.

**D.** Trong LPPP, module này chỉ phục vụ **một khách hàng duy nhất**: `wake_timer` của SCPI `0x06` — đồng hồ báo thức để đánh thức CA72. Hai timer còn lại (0, 1) bị đặt gạch nhưng không dùng.

**E.** Ưu tiên cao nhất: tìm cho ra nơi `TIMEBASE2` được cấu hình. Nếu không ai ghi nó, toàn bộ độ chính xác timing của hệ thống đang dựa vào một giả định chưa được kiểm chứng.