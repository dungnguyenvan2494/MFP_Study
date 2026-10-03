# PS-CPU (pscpu_s800) — 09. Phân tích Timing

**Phạm vi của tài liệu này:** toàn bộ nguồn timing thực tế và giá trị timeout
trong firmware, với giá trị số chính xác được trích dẫn đến nguồn (không bao
giờ bịa đặt hay ước lượng), được phân loại là timing HARDWARE / SOFTWARE /
RTOS / UNKNOWN. Tài liệu đồng hành với `03_execution_model.md` §7 (nơi đầu
tiên xác lập quan sát về hai time-base) và `07_state_machines.md` §1.3 (state
machine của software timer mà tài liệu này cung cấp các con số cụ thể). Các
nhãn bằng chứng theo `SESSION_CONTEXT.md`. Firmware này không có RTOS
(`03_execution_model.md` §8), do đó hạng mục RTOS-timing được xác nhận là
rỗng trong suốt tài liệu chứ không phải bị bỏ sót.

---

## 1. Các nguồn timing — phân loại

| Source | Class | Value | Basis |
|---|---|---|---|
| SysTick | HARDWARE | 1 kHz (`HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000)`, `mx_init.c:153`) | điều khiển `HAL_GetTick()`, được dùng bởi `HAL_Delay()` và logic timeout I2C (§4) |
| TIM3 | HARDWARE | chu kỳ 1 ms, tự tính toán từ `SystemCoreClock` (`BSP_TIM_STM32F03x_Nucleo.c:122-169`, `TIM_PRESCALER=2000`→tốc độ đếm 2kHz, `TIM_PERIOD=2`→ `Period=(2×msec)-1`, `BSP_STM32F03x_Nucleo.h:40-41`), khởi động với `msec=1` (`entry.c:66`) | điều khiển `ui_EventCountTimer` và mọi slot `g_km_timer[]` (§3) — **chỉ trong Main image**, IAP có cấu hình nhưng không bao giờ khởi động nó (`03_execution_model.md` §4.3) |
| IWDG | HARDWARE | prescaler 256, reload 4095 (`mx_init.c:233-235`) → timeout ≈ 4095×256/40000 ≈ **26.2 s** `[HARDWARE ASSUMPTION: LSI≈40kHz]` | khôi phục super-loop bị treo (`06_event_flow.md` E4) |
| RTC (LSE, 32.768kHz `[HARDWARE ASSUMPTION]`) | HARDWARE | điều khiển lịch (calendar) và alarm đánh thức ở STOP-mode (§5) | `RTC_RTCCLKSOURCE_LSE`, `mx_init.c:147` |
| `g_km_timer[]` (16 loại) | SOFTWARE | giảm 1/ms bởi TIM3 ISR (`km_it.c:340-348`) | mọi timeout debounce/sequencing/failsafe trong Main image (§3) |
| Các kiểm tra thời gian trôi qua dựa trên `HAL_GetTick()` | SOFTWARE (nhưng lấy nguồn từ bộ đếm SysTick phần HARDWARE) | chỉ được dùng bởi `i2c_check_error()` (§4) — một **đường tính toán tách biệt khỏi `g_km_timer[]`**, về nguyên tắc cùng chung nguồn 1kHz với TIM3 nhưng không hề được đối chiếu chéo với nó ở bất kỳ đâu | `km_i2c.c:379-461` |
| RTOS tick / `vTaskDelay` | RTOS | **N/A — không có RTOS** | đã xác nhận tại `03_execution_model.md` §8 |
| Timing hoàn thành DMA | — | **N/A — không có kênh DMA nào từng được bật** | đã xác nhận tại `03_execution_model.md` §6 |

**Quan sát về hai time-base** (được nêu lần đầu ở
`03_execution_model.md` §7): SysTick và TIM3 đều chạy ở 1kHz nhưng **không
bao giờ được đối chiếu chéo hay điều hoà với nhau** — một timeout đo bằng
`HAL_GetTick()` (phát hiện lỗi I2C) và một timeout đo bằng `g_km_timer[]`
(mọi thứ còn lại) là hai bộ đếm chạy độc lập tình cờ có cùng tốc độ danh
định, chứ không phải chung một đồng hồ.

---

## 2. Timing anti-chattering / debounce (SOFTWARE, qua `g_km_timer[]`)

| Signal | Duration | Source | File:line |
|---|---|---|---|
| MSW_ON | 10 ms | `KM_ANTI_CHATTER_MSW_TIME` | `km_it.h:6` |
| POWER_MONITOR | 10 ms | `KM_ANTI_CHATTER_PMON_TIME` | `km_it.h:8` |
| MONI_24V11 | 30 ms | `KM_ANTI_CHATTER_24V11_MONI` | `km_it.h:10` |

Các chú thích trong header quy các giá trị cụ thể này về đặc tính phần cứng
của switch/sensor (`KM_ANTI_CHATTER_MSW_TIME`: "10ms - derived from the
MSW's HW spec") — `[HARDWARE ASSUMPTION]`, bản thân các giá trị được lấy
nguyên như trong source, không được suy ra lại từ datasheet trong lượt phân
tích này. Cơ chế: được arm (kích hoạt) trong ngữ cảnh ISR
(`start_anti_chattering()`, `km_it.c:489-501`), giảm dần bởi tick TIM3 dùng
chung, được tiêu thụ bởi `anti_chattering_proc()` mỗi lượt super-loop
(`07_state_machines.md` §1.2).

---

## 3. Các software timer điều phối trình tự nguồn (power-sequencing) — mọi thời lượng được arm tìm thấy trong source

Tất cả đều thông qua `km_timer_set(kind, ms, Start)`, tất cả đều được giảm
dần bởi cùng một ISR TIM3 1ms (§1). "Có thể điều chỉnh khi runtime"
(Runtime-adjustable) = I2C master bên ngoài có thể ghi đè thời lượng đã arm
thông qua một lệnh ghi thanh ghi chuyên dụng.

| Timer kind | Duration | Runtime-adjustable? | Armed by | File:line |
|---|---|---|---|---|
| `SB_RESET_RELEASE` | 100 ms | không | `s800_power_on()` | `km_extend_io.c:620` |
| `BOOT_MC_P_ON` | 320 ms | không | `mc_p_on_proc()` (ngữ cảnh dispatch) | `km_extend_io.c:1119` |
| `MC_P_ON_AFTER_SLEEP_STATUS_REM_SP` | 320 ms | không | `sleep_status_rem_sparrow_proc()` (3 vị trí) | `km_extend_io.c:990,999,1035` |
| `MONI_24V11_OFF` | 500 ms | không | `moni_24v_proc()` | `km_extend_io.c:587` |
| `POWMONI_FAILSAFE` | 2000 ms | không | `power_flicker_failsafe()` | `km_extend_io.c:172` |
| `PWROFF_MAXTIME1` | 60000 ms (`PWROFF_MAXTIME`) | không | `poweroff_timer_start()` | `km_extend_io.c:1281,1287` |
| `PWROFF_MAXTIME2` | 60000 ms (giá trị literal, được arm lại khi MAXTIME1 hết hạn) | không | `poweroff_timer_proc()` | `km_extend_io.c:1316` |
| `PWROFF_WDGTIME` | 2000 ms mặc định (macro `PWROFF_WDGTIME`) | **có** — lệnh ghi I2C `EXTEND_PWROFF_WDGTIME_SET_CMD`, giới hạn tối đa ở `KM_TIMER_MAX_TIME`=65535ms | `poweroff_timer_start()` / lệnh ghi I2C | `km_extend_io.c:1282,1288` / `km_i2c.c:696-703` |
| `BACKUP_WAITTIME` | 50 ms (macro `BACKUP_WAITTIME`) | không | `poweroff_timer_start()` | `km_extend_io.c:1283,1289` |
| `PWROFF_SEQTIME` | 9350 ms mặc định (macro `PWROFF_SEQTIME`) | **có** — lệnh ghi I2C `EXTEND_PWROFF_SEQTIME_SET_CMD`, giới hạn tối đa ở 65535ms | `poweroff_timer_start()` / lệnh ghi I2C | `km_extend_io.c:1284,1290` / `km_i2c.c:709-717` |
| `Reboot_SB_PWR_EN` | được arm qua `set_cycle_time(2)` → **2 giây** (đơn vị giây, dựa trên RTC, không dựa trên `g_km_timer` — xem §5) | n/a | đường STOP-mode của `msw_on_proc()` | `km_extend_io.c:1202` |
| `Anti_Chattering_MSW/PMON/24V` | xem §2 | không | `start_anti_chattering()` | `km_it.c:500` |
| `Rstusb` | **không bao giờ được arm ở bất kỳ đâu trong source đã đọc ở lượt này** | — | — | được khai báo tại `km_it.h:26`, không có tham chiếu nào khác — `[UNKNOWN]`/giá trị enum chết (dead) |
| `MONI_24V11_ON` | **không bao giờ được arm ở bất kỳ đâu trong source đã đọc ở lượt này** | — | — | được khai báo tại `km_it.h:29`, không có tham chiếu nào khác — `[UNKNOWN]`/giá trị enum chết (dead) |

**Timeout nối chuỗi: tổng ngân sách failsafe power-off là ~120 giây, không
phải 60 giây.** `poweroff_timer_proc()` arm lại `PWROFF_MAXTIME2` thêm trọn
60000ms nữa ngay khi `PWROFF_MAXTIME1` hết hạn (`km_extend_io.c:1314-1317`)
— thời gian worst-case thực tế từ `poweroff_timer_start()` cho đến khi
`s800_power_off()` được kích hoạt bởi `PWROFF_FACTOR_TMOUT_LIMIT` là
**~120 giây** (60s + 60s), một sự thật chỉ có thể thấy được khi đọc logic
tiêu thụ của cả hai loại timer cùng nhau, chứ không phải chỉ từ giá trị
literal 60000ms của một trong hai.

**Hai trong số mười sáu loại timer được khai báo được chứng minh là không
được dùng.** `Rstusb` và `MONI_24V11_ON` không xuất hiện ở bất kỳ đâu ngoài
khai báo enumerator của chính chúng — `rst_usb_proc()`
(`km_extend_io.c:711-729`) là một hàm GPIO tổ hợp (combinational) thuần
tuý, không có timer riêng của nó, xác nhận rằng `Rstusb` có khả năng đã
được lên kế hoạch nhưng chưa bao giờ được nối vào thực tế, `[INFERRED]`.

---

## 4. Timeout giao tiếp (SOFTWARE, dựa trên `HAL_GetTick()`, tách biệt khỏi time base của §3)

`i2c_check_error()` (`km_i2c.c:379-461`), được gọi ở đầu mỗi lượt
`i2c_recv_wait()`:

| Condition monitored | Threshold | Action on expiry | File:line |
|---|---|---|---|
| `busy_tx` giữ liên tục | 2000 ms (`I2C_ERROR_TIMEOUT_BUSY_TX`) | `i2c_sw_reset()` | `km_i2c.c:381,404-420` |
| RX đang tiến hành nhưng không hoàn tất | 1000 ms (`I2C_ERROR_TIMEOUT_RX`) | `i2c_sw_reset()` | `km_i2c.c:382,426-436` |
| TX đang tiến hành nhưng không hoàn tất | 1000 ms (`I2C_ERROR_TIMEOUT_TX`) | `i2c_sw_reset()` | `km_i2c.c:383,438-448` |
| `error_code != 0` (BERR/ARLO/OVR bị latch) | tức thì (không có thời gian chờ thêm) | `i2c_sw_reset()` | `km_i2c.c:451-453` |
| `dir_error` được set | tức thì | `i2c_sw_reset()` | `km_i2c.c:456-458` |

**Không có khoảng thời gian retry, không có bộ đếm retry, ở bất kỳ đâu
trong logic này** — timeout/lỗi đầu tiên được phát hiện sẽ kích hoạt
`i2c_sw_reset()` ngay lập tức và vô điều kiện; không tồn tại mẫu hình
"thử N lần trước khi bỏ cuộc" trong firmware này cho bất kỳ subsystem nào,
được xác nhận lại một lần nữa ở đây (`08_concurrency.md` §4 đã xác lập
cùng một sự vắng mặt này từ góc độ concurrency; đây là cùng một sự thật
nhìn từ góc độ timing: không tồn tại giá trị khoảng-thời-gian-retry nào để
ghi nhận vì không tồn tại vòng lặp retry nào).

---

## 5. Đánh thức định kỳ ở STOP-mode (HARDWARE, RTC alarm)

`set_cycle_time(sec)` (`km_alarm_wake.c:41-110`) tính toán một RTC alarm
cách thời điểm RTC hiện tại `sec` giây và arm nó thông qua
`HAL_RTC_SetAlarm_IT()`. Được gọi với **giá trị literal `2`** từ
`msw_on_proc()` (`km_extend_io.c:1202`) — một chu kỳ **2 giây**. Đại lượng
này được đo bằng giây lịch (calendar) của RTC (dùng clock LSE), không phải
bằng mili-giây của `g_km_timer[]` — một miền timing thứ ba, độc lập, song
song với SysTick và TIM3. Lối thoát: hoặc chính alarm này lại kích hoạt lần
nữa (`HAL_RTC_AlarmAEventCallback`, `km_it.c:284-300`), hoặc một cạnh
MSW_ON (`km_it.c:267-274`) — cả hai đều được tài liệu hoá ở
`07_state_machines.md` §1.5.

---

## 6. Các delay chặn (blocking) (`HAL_Delay()`, dựa trên SysTick, chặn main-loop)

| Call site | Duration | Context | Consequence |
|---|---|---|---|
| `s800_power_off()` | 500 ms | main-loop | toàn bộ super-loop bị chặn trong 500ms ngay trước `HAL_NVIC_SystemReset()` — `04_call_graph.md` §2.9 đã đánh dấu đây là điểm dừng cố ý duy nhất kéo dài hàng trăm mili-giây trong firmware; chú thích trong code nói rằng nó đảm bảo "SB_PWR_EN's L period ≥500ms" | `km_extend_io.c:558` |
| `i2c_sw_reset()` | 2 ms | main-loop | khoảng dừng ngắn giữa lúc force-reset và lúc giải phóng clock của peripheral I2C1 | `km_i2c.c:359` |

Cả hai đều là vòng lặp busy-wait dựa trên bộ đếm tick do SysTick điều khiển
bên trong `HAL_Delay()` của HAL — CPU không làm gì khác, không có interrupt
nào bị vô hiệu hoá trong suốt cả hai trường hợp (nên các ISR vẫn chạy và
`HAL_IncTick()` vẫn tiếp tục tăng), nhưng không có giai đoạn super-loop nào
chạy lại cho đến khi delay kết thúc.

### 6.1 Một pseudo-timeout không dựa trên thời gian — `KM_RTC_Restore()`
`mx_init.c:312-328`: một vòng lặp đếm-số-lần-lặp trần trụi
(`volatile int loop_limit`, giới hạn ở 10000) chờ
`RTC->ISR & RTC_ISR_INITF`, với chú thích khẳng định "approximately 2ms or
more is timeout" (`約2ms以上はタイムアウト`, `:324`). Đây **không phải**
là một timeout đo bằng thời gian thực — đó là một heuristic
đếm-vòng-lặp chỉ xấp xỉ 2ms ở bất kỳ tốc độ clock và mức tối ưu hoá trình
biên dịch nào đã tạo ra ước lượng trong chú thích đó. Được phân loại
**UNKNOWN TIMING**: thời gian trôi qua thực tế (wall-clock) mà vòng lặp
này tốn không thể suy ra chỉ từ source (phụ thuộc vào số lệnh máy được
biên dịch cho mỗi vòng lặp và `SystemCoreClock`), khác với mọi timeout
khác trong tài liệu này vốn đều là một giá trị thanh ghi phần cứng hoặc
một con số mili-giây từ `g_km_timer`/`HAL_GetTick()`.

---

## 7. Bảng đầy đủ Event/Start/Duration/Timeout/Next-Action/Context

| Event | Start | Duration | Timeout | Next action | Context |
|---|---|---|---|---|---|
| TIM3 update | boot (`BSP_TIM_Start`, `entry.c:66`) | 1 ms, lặp lại | n/a (chạy tự do) | giảm `g_km_timer[]`, `ui_EventCountTimer++` | ISR |
| SysTick | boot (`HAL_SYSTICK_Config`) | 1 ms, lặp lại | n/a | `HAL_IncTick()` | ISR |
| MSW/POWER_MONI/24V11 debounce | cạnh EXTI | 10/10/30 ms | khi hết hạn | kích hoạt `intr_proc()` nếu ổn định | ISR arm, main-loop drain |
| `SB_RESET_RELEASE` | `s800_power_on()` | 100 ms | khi hết hạn | (consumer chưa được truy vết thêm ở lượt này — `[UNKNOWN]` vị trí đọc chính xác ngoài điểm arm) | main-loop |
| `BOOT_MC_P_ON` | dispatch MC_P_ON lúc boot | 320 ms | khi hết hạn (`TYPE_Km_Timer_End`) | `mc_p_on_eagle_proc()` set các bit `MC_P_ON`/`IR_P_ON` | main-loop |
| `MC_P_ON_AFTER_SLEEP_STATUS_REM_SP` | điều khiển SLEEP_STATUS_REM của Sparrow | 320 ms | khi hết hạn | cho phép `mc_p_on_sparrow_proc()` assert MC_P_ON | main-loop |
| `MONI_24V11_OFF` | `moni_24v_proc()` | 500 ms | khi hết hạn | (xác nhận xả điện 24V — logic consumer ngoài điểm arm không được truy vết lại ở lượt này) | main-loop |
| `POWMONI_FAILSAFE` | phát hiện POWER_MONITOR sụt | 2000 ms | khi hết hạn | `poweroff_factor_save()` + `s800_power_off()` | main-loop |
| Chuỗi `PWROFF_MAXTIME1`→`MAXTIME2` | `poweroff_timer_start()` | 60000+60000 ms (~120s tổng cộng) | khi `MAXTIME2` hết hạn | `poweroff_factor_save(TMOUT_LIMIT)` + `s800_power_off()` | main-loop |
| `PWROFF_WDGTIME` | `poweroff_timer_start()` | 2000 ms (hoặc set qua I2C, ≤65535ms) | khi hết hạn | `poweroff_factor_save(TMOUT_WDG)` + `s800_power_off()` | main-loop |
| `BACKUP_WAITTIME` | `poweroff_timer_start()` | 50 ms | khi hết hạn | `backupwait_timer_proc()` tác động lên `g_backupwait_pending` | main-loop |
| `PWROFF_SEQTIME` | `poweroff_timer_start()` | 9350 ms (hoặc set qua I2C, ≤65535ms) | trong lúc đang chạy | chi phối hành vi row-2/row-3 của `msw_on_proc()` (`07_state_machines.md` §2.2) | main-loop |
| STOP-mode RTC alarm | `msw_on_proc()` vào STOP | 2 s, tự lặp lại | mỗi lần kích hoạt | xoá `stop_mode`, đánh giá lại ở lượt kế tiếp (có thể arm lại) | ISR (RTC) |
| I2C `busy_tx`/`rx`/`tx` | bắt đầu transaction (`rx_comp()` set `*_tick_last`) | n/a | 2000/1000/1000 ms | `i2c_sw_reset()` | main-loop (kiểm tra), ISR (đóng dấu thời gian) |
| IWDG | boot (`BSP_WDT_Start`) | n/a | ~26.2 s | hardware MCU reset | hardware |
| `s800_power_off()` blocking delay | vào power-off | 500 ms | n/a (vô điều kiện) | `HAL_NVIC_SystemReset()` | main-loop, chặn (blocking) |
| `i2c_sw_reset()` blocking delay | I2C reset được kích hoạt | 2 ms | n/a | giải phóng I2C1 khỏi reset, khởi tạo lại | main-loop, chặn (blocking) |
| `KM_RTC_Restore()` spin-wait | boot, khôi phục RTC backup | ~2 ms `[UNKNOWN precision]` | 10000 vòng lặp | vẫn tiếp tục bất kể (`break`, không lan truyền lỗi) | startup |

---

## 8. Quan sát

1. **Hai-rưỡi miền timing độc lập cùng tồn tại mà không có đối chiếu
   chéo**: `g_km_timer[]` do TIM3 điều khiển (ms), `HAL_GetTick()` do
   SysTick điều khiển (ms, chỉ dùng cho phát hiện lỗi I2C), và giây lịch
   RTC (chỉ dùng cho alarm STOP-mode). Một lỗi cấu hình ở bất kỳ đồng hồ
   nào trong số này sẽ chỉ bao giờ lộ ra ở các consumer thuộc chính miền
   đó.
2. **Trần failsafe power-off thực tế 120 giây không thể thấy được nếu chỉ
   nhìn riêng lẻ `PWROFF_MAXTIME1` hoặc `PWROFF_MAXTIME2`** — chỉ có thể
   thấy được khi đọc logic arm-lại của `poweroff_timer_proc()` (§3).
3. **Không tồn tại giá trị khoảng-thời-gian-retry nào ở bất kỳ đâu trong
   firmware này** vì không tồn tại vòng lặp retry nào ở bất kỳ đâu trong
   firmware này (§4) — một sự xác nhận từ góc độ phân tích timing cho
   phát hiện của phân tích concurrency trong `08_concurrency.md`.
4. **Hai trong số mười sáu loại software timer là chết (dead)**
   (`Rstusb`, `MONI_24V11_ON`) — được khai báo, không bao giờ được arm,
   không bao giờ được tra cứu.
5. **Có một timeout duy nhất trong toàn bộ firmware hoàn toàn không phải
   là một thời lượng được đo lường** (vòng lặp đếm-số-lần-lặp của
   `KM_RTC_Restore()`, §6.1) — mọi thứ khác trong tài liệu này đều là một
   giá trị mili-giây/giây thực sự, có thể truy vết về một hằng số hoặc
   một giá trị thanh ghi do I2C cung cấp.
