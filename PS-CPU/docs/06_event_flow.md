 # PS-CPU (pscpu_s800) — 06. Luồng Sự Kiện (Event Flow)

**Phạm vi tài liệu này:** mọi sự kiện đi vào firmware này từ thế giới bên
ngoài (hoặc từ chính nó, thông qua timer/watchdog) và cách nó lan truyền
đến một phản hồi có thể quan sát được. Tài liệu đồng hành với
`03_execution_model.md` (các context), `04_call_graph.md` (luồng điều
khiển) và `05_data_flow.md` (trạng thái) — tài liệu này tái sử dụng thuật
ngữ và trích dẫn của chúng thay vì suy diễn lại, và bổ sung một vài sự
thật chưa cần đến trước đây: các vị trí gọi chính xác của `Error_Handler()`
và tương tác của nó với thời gian watchdog, cùng một kết quả phủ định
(nguyên nhân reset không bao giờ được kiểm tra ở bất cứ đâu trong firmware
này) đã được xác nhận bằng grep triệt để trong lượt này. Các nhãn bằng
chứng tuân theo `SESSION_CONTEXT.md`.

**Hình dạng chuỗi sự kiện đã được điều chỉnh:** chuỗi mẫu
"hardware → interrupt/callback → driver → sync primitive → task →
application → state change → response" ở đây không có giai đoạn "task"
(không có RTOS) và "sync primitive" hầu như luôn là "một cờ đơn giản, được
polling ở vòng lặp kế tiếp" thay vì một semaphore/queue
(`03_execution_model.md` §8). Mọi trace bên dưới sử dụng:
```
EVENT → hardware → ISR/callback → state written → [next super-loop pass] → application logic → state change → response
```
với khoảng cách ISR→main-loop được thu gọn về 0 đối với một số ít sự kiện
được xử lý hoàn toàn bên trong ngữ cảnh ngắt (được đánh dấu rõ ràng ở
những chỗ điều đó xảy ra).

---

## 1. Danh mục sự kiện (tra cứu nhanh)

| # | Sự kiện | Phân loại | Nguồn phần cứng | Handler đầu tiên | Context |
|---|---|---|---|---|---|
| E1 | Power-on reset (POR) | Power/Reset | VDD tăng | `Reset_Handler` | Reset |
| E2 | Chân reset ngoài (NRST) | Power/Reset | Chân NRST | `Reset_Handler` | Reset |
| E3 | Reset bằng phần mềm (`HAL_NVIC_SystemReset()`) | Power/Reset | AIRCR.SYSRESETREQ | `Reset_Handler` | Reset (được kích hoạt từ main-loop hoặc ISR, §5) |
| E4 | IWDG timeout | Power/Reset | IWDG counter underflow | `Reset_Handler` | Reset (âm thầm — không phần mềm nào "nhìn thấy" điều này xảy ra, §4.6) |
| E5 | Cạnh xung chân HRESET_REQ | GPIO/EXTI | PA1 | `EXTI0_1_IRQHandler` | ISR |
| E6 | Cạnh xung chân AP_PWR_EN | GPIO/EXTI | PA2 | `EXTI2_3_IRQHandler` | ISR |
| E7 | Cạnh xung chân POWER_MONITOR | GPIO/EXTI | PA4 | `EXTI4_15_IRQHandler` | ISR |
| E8 | Cạnh xung chân MSW_ON | GPIO/EXTI | PA5 | `EXTI4_15_IRQHandler` | ISR |
| E9 | Cạnh xung chân MONI_24V11 | GPIO/EXTI | PA8 | `EXTI4_15_IRQHandler` | ISR |
| E10 | Khớp địa chỉ I2C | I2C | Cờ I2C1 ADDR | `I2C1_IRQHandler` | ISR |
| E11 | Nhận byte I2C | I2C | Cờ I2C1 RXNE | `I2C1_IRQHandler` (nội bộ HAL) | ISR |
| E12 | Truyền byte I2C | I2C | Cờ I2C1 TXIS | `I2C1_IRQHandler` (nội bộ HAL) | ISR |
| E13 | Hoàn tất truyền I2C | I2C | Cờ I2C1 STOPF/TC | `I2C1_IRQHandler` | ISR |
| E14 | Lỗi bus I2C (BERR/ARLO/OVR) | I2C / Error | Các bit lỗi ISR của I2C1 | `I2C1_IRQHandler` | ISR |
| E15 | Nhận NACK I2C (master hủy giao dịch) | I2C / Error | Cờ AF của I2C1 | Nội bộ HAL | ISR |
| E16 | Lệnh giao thức: đọc thanh ghi | Protocol | (mang bên trong E10-E13) | `i2c_recv_wait()` | main-loop |
| E17 | Lệnh giao thức: ghi thanh ghi | Protocol | (mang bên trong E10-E13) | `i2c_recv_wait()` | main-loop |
| E18 | Lệnh giao thức: vào IAP (magic number) | Protocol | (mang bên trong E17) | `i2c_recv_wait()` | main-loop |
| E19 | Lệnh giao thức: yêu cầu reset I2C bằng phần mềm | Protocol | (mang bên trong E17) | `i2c_recv_wait()` | main-loop |
| E20 | Lệnh giao thức: ghi dữ liệu firmware (chỉ trong IAP) | Protocol | (mang bên trong E17) | `i2c_recv_wait()` (IAP) | main-loop |
| E21 | Tick 1 ms của TIM3 | Timer | Sự kiện update của TIM3 | `TIM3_IRQHandler` | ISR |
| E22 | Hết thời gian debounce | Timer (phần mềm) | (suy ra từ E21) | `anti_chattering_proc()` | main-loop |
| E23 | Hết thời gian trình tự tắt nguồn | Timer (phần mềm) | (suy ra từ E21) | `poweroff_timer_proc()` | main-loop |
| E24 | Hết thời gian chờ backup | Timer (phần mềm) | (suy ra từ E21) | `backupwait_timer_proc()` | main-loop |
| E25 | Báo thức RTC (thức dậy định kỳ ở chế độ STOP) | Timer | Cờ RTC ALRAF | `RTC_IRQHandler` | ISR |
| E26 | Phát hiện timeout giao tiếp | Error | (suy ra từ E21/HAL_GetTick, không có sự kiện HW mới) | `i2c_check_error()` | main-loop |
| E27 | Sai lệch trạng thái giao thức giao tiếp | Error | (suy ra từ trình tự E13) | `i2c_recv_wait()` | main-loop |
| E28 | Lỗi khởi tạo HAL lúc boot | Error | (suy ra từ E1/E2/E3/E4, trong lúc `MX_Init()`) | `Error_Handler()` | startup |
| E29 | Phát hiện "bão" boot-notify | Error | (suy ra từ E17) | `io_extend_write()` | main-loop |
| E30 | Sai lệch checksum IAP sau khi cập nhật | Error | (suy ra từ E17) | `check_sum_calc()` | main-loop (IAP) |
| E31 | Chuyển từ Main sang IAP | Cross-image | (suy ra từ E18) | `jump2iap()` | main-loop |
| E32 | Chuyển từ IAP sang Main | Cross-image | (suy ra từ E8 trong IAP) | `msw_on_proc_iap()` | ISR |

---

## 2. Các sự kiện Power / Reset (E1-E4)

Cả bốn đều hội tụ về đúng một điểm vào — firmware này **không bao giờ
phân biệt lý do nó reset**. Đã xác nhận trong lượt này bằng cách grep
toàn bộ các cây thư mục `main/`, `iap/`, và `common/Drivers` để tìm
`RCC_FLAG_IWDGRST` / `RCC_FLAG_PORRST` / `RCC_FLAG_SFTRST` / `RCC_CSR`:
mọi kết quả khớp đều là sử dụng nội bộ của thư viện HAL để polling
oscillator-ready (ví dụ `stm32f0xx_hal_rcc.c:289` kiểm tra
`RCC_FLAG_HSERDY`); **không có đoạn code ứng dụng nào ở bất cứ đâu đọc
các bit nguyên nhân reset trong `RCC->CSR`.**

```
EVENT (bất kỳ trong E1-E4)
  → Reset_Handler                      startup_stm32f031x6.s:131
  → SystemInit() → __main → main()     (đường dẫn cold-start giống hệt nhau bất kể nguyên nhân)
STATE CHANGE: mọi biến static ở mức module trở về giá trị BSS/initializer của nó; g_km_timer[], i2c_info, anti_chattering_info[], pending_factor, v.v. — tất cả đều mất.
RESPONSE: khởi tạo lại toàn bộ (03_execution_model.md §2.1); thời gian timeout IWDG ~26.2s ở `03_execution_model.md` §7 chính là điều làm cho E4 có thể xảy ra ngay từ đầu.
```

**Ý nghĩa:** đây là một đặc tính thiết kế thực sự đáng nêu rõ — dù MCU
thức dậy từ cold power-up, chân reset ngoài bị đảo trạng thái, một lệnh
`HAL_NVIC_SystemReset()` tường minh, hay watchdog timeout vì có gì đó bị
treo, mã boot của firmware hoạt động giống hệt nhau. Không có bất kỳ
telemetry nào phân biệt "khởi động lại sạch" với "watchdog vừa cứu một
lần treo" — nơi duy nhất bất kỳ điều nào trong số này được ghi lại dù chỉ
gián tiếp là `poweroff_factor_save()` (`km_extend_io.c:144-154`, một
lệnh ghi thanh ghi được nuôi bằng pin dự phòng), thứ ghi lại *lý do
firmware chọn tắt nguồn SoC S800*, chứ không phải lý do bản thân MCU
PS-CPU vừa mới reset.

### E4 chi tiết — IWDG timeout là một sự kiện âm thầm
Không có đoạn code nào ở bất cứ đâu đọc trạng thái `IWDG` hoặc một cờ
"watchdog vừa cứu tôi". Dấu vết quan sát được duy nhất là gián tiếp: nếu
có một lần treo xảy ra trong `HAL_Delay()` 500ms của `s800_power_off()`
(`04_call_graph.md` §2.9) hoặc bên trong vòng lặp vô hạn của
`Default_Handler`/`Error_Handler()` (E28, §5.5), thì timeout IWDG ~26.2s
(`03_execution_model.md` §7) là thứ duy nhất từng phục hồi hệ thống, và
nó làm điều đó bằng cách trông, ở lần boot kế tiếp, hệt như E1/E2/E3.

---

## 3. Các sự kiện tín hiệu GPIO / EXTI (E5-E9)

Đã được trace đầy đủ từ đầu đến cuối trong `03_execution_model.md` §4.2
và `04_call_graph.md` §2.3; được trình bày lại ở đây theo khuôn
Trigger/Handler/Context/Data/State/Action/Next-Event mà pha này yêu cầu,
cộng với điều thực sự mới: phân biệt cái nào trong số năm sự kiện được
xử lý *ngay lập tức* (trong ISR) so với *hoãn lại* (đến lượt super-loop
kế tiếp).

| Sự kiện | Trigger | Handler | Context | Data | State bị chạm | Action | Sự kiện kế tiếp |
|---|---|---|---|---|---|---|---|
| E5 cạnh HRESET_REQ | Cạnh PA1 | `km_exti_callback()` `km_it.c:254` | ISR | mức chân (không đọc) | bit `pending_factor` được set | không có gì trong ISR — **hoãn lại** | E-vòng-kế-tiếp: `intr_pending_proc()` → `hreset_req_proc()` |
| E6 cạnh AP_PWR_EN | Cạnh PA2 | `km_exti_callback()` `km_it.c:257` | ISR | mức chân (không đọc) | bit `pending_factor` được set | không có gì trong ISR — **hoãn lại** | E-vòng-kế-tiếp: `intr_pending_proc()` → `ap_power_en_proc(STATE_INT)` → ghi `_status` (lan tỏa vào lần đọc kế tiếp của `sleep_status_rem_*_proc`, một vòng lặp sau — trường hợp dữ liệu cũ trong `05_data_flow.md` §5.1) |
| E7 cạnh POWER_MONITOR | Cạnh PA4 | `km_exti_callback()` `km_it.c:261` | ISR | mức chân trực tiếp, đọc ngay | `anti_chattering_info[POWER_MONI]` được arm **ngay lúc này** | `start_anti_chattering()` arm một timer debounce — **ngay lập tức**, không hoãn | E22 (hết debounce, sau đó) |
| E8 cạnh MSW_ON | Cạnh PA5 | `km_exti_callback()` `km_it.c:267-274` | ISR | mức chân trực tiếp; cũng đọc `stop_mode` | `anti_chattering_info[MSW]` được arm; có thể `stop_mode` bị xóa, PLL được khóa lại | `start_anti_chattering()`; **nếu `stop_mode` đang được set**, còn gọi `SystemClock_Config2()` + `HAL_RTC_DeactivateAlarm()` **bên trong ISR** | E22, hoặc tiếp tục hoạt động tốc độ đầy đủ ngay lập tức |
| E9 cạnh MONI_24V11 | Cạnh PA8 | `km_exti_callback()` `km_it.c:264-265` | ISR | mức chân trực tiếp | `anti_chattering_info[24V11]` được arm | `start_anti_chattering()` — ngay lập tức; `intr_proc` là `NULL` cho mục này (`km_it.c:48`), nên khi E22 cuối cùng xảy ra cho tín hiệu này sẽ không tạo ra callback nào, chỉ có state đã debounce (`04_call_graph.md` §3.7) | E22 (chỉ state, không callback) |

**Ý nghĩa:** ba trong năm sự kiện GPIO (E7-E9) được *arm debounce phần
cứng* một cách đồng bộ trong ngữ cảnh ngắt, trong khi hai sự kiện (E5-E6)
không làm gì trong ISR ngoài việc set một bit — một cách xử lý thực sự
khác nhau cho từng tín hiệu mà không thể thấy được chỉ từ định nghĩa
chân hay chữ ký của ISR.

---

## 4. Các sự kiện phần cứng I2C (E10-E15) và các sự kiện giao thức mà chúng mang theo (E16-E20)

### 4.1 Chuỗi ở mức phần cứng (dùng chung cho mọi sự kiện I2C)
```
E10/E11/E12/E13/E14/E15 (cờ ngoại vi I2C1)
  → I2C1_IRQHandler                     main/Src/stm32f0xx_it.c:203 (iap: :158)
  → HAL_I2C_EV_IRQHandler / ER_IRQHandler → I2C_Slave_ISR_IT()   stm32f0xx_hal_i2c.c:3380 (05_data_flow.md §4.1-4.2 để xem các dòng copy thanh ghi chính xác)
  → weak HAL_I2C_AddrCallback/SlaveRxCpltCallback/SlaveTxCpltCallback/ErrorCallback → do_callback()/direct
  → rx_comp(event)                      km_i2c.c:171   — ISR, set i2c_info.{read_comp,write_comp,error_code,...}
```
Đây là một chuỗi duy nhất, nhưng nó kích hoạt cho sáu điều kiện phần
cứng khác biệt về mặt logic (E10-E15); bitmask `event` của `rx_comp()`
(`KM_I2C_EVENT_MASK`, `km_i2c.c:174-182`) chính là thứ cho phần còn lại
của pipeline biết cái nào vừa xảy ra. E14/E15 (lỗi) còn latch thêm
`i2c_info.error_code` (`:251-253`), sẽ được tiêu thụ sau bởi E26 (§5.1),
không được hành động ngay lập tức.

### 4.2 Các sự kiện lệnh giao thức — sự kiện logic cưỡi bên trong E10-E13/E17
Master I2C không gửi "sự kiện" theo nghĩa phần cứng cho mỗi lần truy cập
thanh ghi — từ góc nhìn của firmware, **mọi** giao dịch ghi đã hoàn tất
(E13 với `direction==write`) tiềm năng là một sự kiện *logic* khác nhau
tùy thuộc vào byte `second_addr` mà master đã gửi. Đây là tầng nơi
"lệnh giao thức" như một hạng mục sự kiện thực sự tồn tại trong firmware
này:

| Sự kiện logic | Điều kiện trigger (trong `i2c_recv_wait()`) | File:line | Action | Sự kiện kế tiếp |
|---|---|---|---|---|
| E16 đọc thanh ghi | `second_addr` khớp một lệnh đọc đã biết, `xfer_size==1` | `km_i2c.c:511-626` | dàn dữ liệu `tx_data`/con trỏ, `change_state(...SlaveTransmit...)` | E12 (các byte được clock ra) |
| E17 ghi thanh ghi | `second_addr` khớp một lệnh ghi đã biết, `xfer_size>1` | `km_i2c.c:637-731` | thay đổi thanh ghi RTC / `g_io_extend_memory` / timer (`05_data_flow.md` §2.1-2.3) | thay đổi theo từng lệnh, xem bên dưới |
| E18 vào IAP | `second_addr==IAP_PG_COMMAND` **và** `data==IAP_PG_COMMAND_MAGICNUM` | `km_i2c.c:686-693` | `DeInit()` + `jump2iap()` | E31 (kết thúc đối với Main) |
| E19 yêu cầu reset I2C bằng phần mềm | `second_addr==EXTEND_RESET_I2C` | `km_i2c.c:720-723` | `i2c_sw_reset()` | tương đương E1 nhưng chỉ cho ngoại vi I2C (không phải MCU) — arm lại `i2c_recv_first()` |
| E20 ghi dữ liệu firmware (IAP) | `second_addr==IAP_PG_COMMAND_DATA`, chỉ image IAP | `km_i2c_iap.c:353-375` | `flash_write_32()` mỗi từ 4 byte | không có gì cho đến khi lệnh ghi hoàn tất `IAP_PG_COMMAND_CHANGE_INFO` của E17 kích hoạt kiểm tra checksum của E30 |

**Ý nghĩa:** E18 là sự kiện logic quan trọng nhất trong toàn bộ firmware
— một giá trị 4-byte cụ thể được ghi vào một địa chỉ thanh ghi cụ thể là
toàn bộ bề mặt kích hoạt để từ bỏ image đang chạy và nhảy vào IAP
(`04_call_graph.md` §2.9). Không có bước xác nhận thứ hai nào (không "bạn
có chắc không," không có chuỗi arm-rồi-commit riêng biệt) — chỉ cần khớp
magic number là đủ.

---

## 5. Các sự kiện timer và watchdog (E21-E25)

### 5.1 E21 — Tick 1 ms của TIM3 (chỉ Main)
```
E21: sự kiện update TIM3 (phần cứng, mỗi 1ms)
  → TIM3_IRQHandler → HAL_TIM_IRQHandler → HAL_TIM_PeriodElapsedCallback → km_tim_callback()   km_it.c:312  (ISR)
STATE CHANGE: ui_EventCountTimer++; mọi slot g_km_timer[] đang được arm bị giảm, cái nào hết hạn được đánh dấu End
ACTION: không có gì ngoài việc giảm — km_tim_callback() không bao giờ tự kích hoạt phản hồi
NEXT EVENT: E22/E23/E24, bất kỳ slot nào vừa chạm End, được phát hiện ở lượt super-loop kế tiếp
```

### 5.2 E22/E23/E24 — hết hạn timer phần mềm (suy ra, main-loop)
Đây không phải các sự kiện phần cứng riêng biệt — chúng là việc main
loop *nhận ra* rằng `km_timer_get_state()` bây giờ trả về
`TYPE_Km_Timer_End` cho một slot mà E21 đã giảm về 0. Mỗi cái có hàm
polling riêng:
- E22 hết debounce → `anti_chattering_proc()` (`km_extend_io.c:1438`) →
  điều kiện, kích hoạt `info->intr_proc(STATE_INT)` (`04_call_graph.md` §3.7).
- E23 các timer trình tự tắt nguồn → `poweroff_timer_proc()`
  (`km_extend_io.c:1305-1338`) → khi `PWROFF_MAXTIME2`/`PWROFF_WDGTIME`
  hết hạn, `poweroff_factor_save()` + `s800_power_off()` (một vị trí gọi
  **thứ tư** cho `s800_power_off()`, chưa được liệt kê trước đây trong
  `04_call_graph.md` §2.9 — được ghi chú là một bổ sung bên dưới).
- E24 hết thời gian chờ backup → `backupwait_timer_proc()`
  (`km_extend_io.c:1350+`).

> **Bổ sung cho `04_call_graph.md` §2.9**: tài liệu đó đã liệt kê 3 nơi
> gọi `s800_power_off()` (`msw_on_proc`, `power_flicker_failsafe`,
> `io_extend_write`). Còn tồn tại một nơi gọi thứ 4: `poweroff_timer_proc()`
> (`km_extend_io.c:1336`), đến được thông qua E23 ở trên. Được ghi lại ở
> đây thay vì âm thầm sửa vào tài liệu trước đó, theo quy tắc "không bao
> giờ âm thầm bịa ra hoặc bỏ sót hành vi" — Phase 4 đã đếm thiếu một trường
> hợp này.

### 5.3 E25 — Báo thức RTC (thức dậy định kỳ ở chế độ STOP, chỉ Main)
```
E25: cờ RTC ALRAF được set (phần cứng, chu kỳ ~2s trong khi ở chế độ STOP, 03_execution_model.md §4.5)
  → RTC_IRQHandler → HAL_RTC_AlarmIRQHandler → HAL_RTC_AlarmAEventCallback()   km_it.c:284  (ISR)
DATA: cờ stop_mode (05_data_flow.md §2.4)
STATE CHANGE: nếu stop_mode==ON, xóa cờ báo thức RTC trực tiếp và xóa stop_mode — hoàn toàn bên trong ISR
ACTION: không có gì ngoài việc xóa cờ (hành vi "thức dậy và kiểm tra lại điều kiện" thực tế là ngầm định: xóa stop_mode cho phép logic main-loop của sleep_mode()/msw_on_proc() đánh giá lại ở lượt kế tiếp thay vì CPU ngay lập tức vào lại STOP)
NEXT EVENT: lượt super-loop kế tiếp đánh giá lại điều kiện vào STOP-mode của msw_on_proc() (km_extend_io.c:1202-1203) — nếu trạng thái tín hiệu vẫn yêu cầu điều đó, một chu kỳ set_cycle_time()/BSP_PWR_enter_stopmode() **mới** bắt đầu, thực chất là một sự kiện tự lặp lại mỗi ~2s cho đến khi trạng thái MSW_ON hoặc SB_PG thay đổi
```

### 5.4 Watchdog — làm mới IWDG (không phải một "sự kiện" theo nghĩa trigger, nhưng sự kiện *không* xảy ra mới quan trọng)
`BSP_WDT_Refresh()` được gọi một lần mỗi lượt super-loop trong cả hai
image (`entry.c:89`, `entry_iap.c:76`) — đây là firmware *ngăn chặn* E4
xảy ra, chứ không phải phản hồi một sự kiện. Cách duy nhất để suy luận
về "sự kiện watchdog" ở đây là theo hướng phủ định: bất kỳ đường code
nào không trả quyền điều khiển về vòng lặp trong vòng ~26.2s (một lần
treo trong `Error_Handler()`/`Default_Handler()`, hoặc một khoảng dừng
bất thường rất dài) sẽ âm thầm chuyển thành E4.

---

## 6. Các sự kiện lỗi giao tiếp và sai lệch giao thức (E26-E27)

### E26 — Timeout giao tiếp
```
Trigger: i2c_check_error() (main-loop, được gọi ở đầu mỗi i2c_recv_wait(), km_i2c.c:379-461) phát hiện một trong:
  - busy_tx giữ > 2000ms
  - rx_during giữ > 1000ms mà không hoàn tất
  - tx_during giữ > 1000ms mà không hoàn tất
  - i2c_info.error_code != 0 (một E14/E15 đã được latch)
  - i2c_info.dir_error (sai lệch hướng giao thức, set trong rx_comp(), km_i2c.c:234,242)
Handler: i2c_recv_wait()                km_i2c.c:493-496
Context: main-loop
Data: thời gian trôi qua dựa trên HAL_GetTick(), các trường i2c_info đã latch (đọc dưới HAL_NVIC_DisableIRQ(I2C1_IRQn), km_i2c.c:392-401)
State change: i2c_info được reset hoàn toàn thông qua i2c_recv_first() (được gọi từ i2c_sw_reset())
Action: i2c_sw_reset()                  km_i2c.c:356-364 — force-reset ngoại vi I2C1, khởi tạo lại, arm lại slave RX
Next event: tương đương E1 nhưng chỉ cho I2C — master phải bắt đầu một giao dịch mới; không có gì được báo lại cho master về những gì đã xảy ra (không có NACK-storm, không có thanh ghi lỗi được cập nhật cho nguyên nhân cụ thể này ngoài những gì nó đã đọc được qua chẩn đoán kiểu EXTEND_CHECK_I2C)
```

### E27 — Sai lệch trạng thái giao thức (master và firmware bất đồng về lượt của ai)
```
Trigger: i2c_recv_wait() quan sát thấy read_comp được set trong khi state==Wait_Write (km_i2c.c:739-742), hoặc chỉ write_comp trong khi state==Wait_Read (km_i2c.c:746-748)
Handler: i2c_recv_wait()
Context: main-loop
Action: i2c_sw_reset() — cùng hành động khôi phục như E26
Next event: giống như E26
```
**Ý nghĩa:** E26 và E27 là hai điểm phát hiện lỗi duy nhất cho toàn bộ
giao thức I2C, và cả hai đều hội tụ về đúng một hành động khôi phục.
Không có sự phân biệt nào giữa "bus bị glitch" và "phần mềm của master
có bug trong việc theo dõi trạng thái của chính nó" — cả hai đều trông
giống hệt nhau từ phía PS-CPU và nhận cùng một phản hồi kiểu one-size.

---

## 5.5 Lỗi khởi tạo lúc boot (E28)

```
Trigger: bất kỳ lệnh nào trong 13 lệnh gọi khởi tạo HAL bên trong MX_Init() trả về != HAL_OK
         (main/Src/mx_init.c:132,142,150,216,223,238,259,292,301,376,394,400,407 — các đường cấu hình RCC osc/clock, khởi tạo I2C1/cấu hình analog-filter, khởi tạo IWDG, khởi tạo/khôi phục RTC/khởi tạo alarm)
Handler: Error_Handler()                main/Src/mx_init.c:529-537 (iap: :264-271) — hình dạng giống hệt: while(1){}
Context: startup (vẫn còn trong MX_Init(), được gọi từ main() trước khi BSP_WDT_Start() chạy)
State change: không có — vòng lặp không bao giờ trả về
Action: không có
Next event: E4 (timeout IWDG) — NHƯNG có một lưu ý chưa từng được nêu trước đây: vì MX_Init() chạy TRƯỚC BSP_WDT_Start() trong cả entry.c:52-63 và entry_iap.c:44-51, một lần treo Error_Handler() xảy ra trong lúc boot xảy ra khi watchdog CHƯA được khởi động — [INFERRED] lần treo cụ thể này KHÔNG có khôi phục bằng watchdog nào cả và sẽ cần một reset bên ngoài (NRST/cắt nguồn) để giải quyết, khác với một lần treo Default_Handler() xảy ra sau đó trong hoạt động bình thường (03_execution_model.md §6), vốn đã có IWDG đang chạy và được arm.
```

**Ý nghĩa:** đây là một kiểu thất bại khác biệt về bản chất so với lần
treo `Default_Handler()` đã được tài liệu hoá trước đó
(`03_execution_model.md` §6) — cùng triệu chứng (vòng lặp vô hạn, không
nuôi watchdog) nhưng *không có* watchdog nào tồn tại lúc đó để cuối cùng
khôi phục nó, vì đúng vị trí trong trình tự boot nơi nó xảy ra so với
`BSP_WDT_Start()`.

---

## 7. Các sự kiện lỗi ở tầng ứng dụng (E29-E30)

### E29 — Bảo vệ chống "bão" boot-notify
```
Trigger: io_extend_write() quan sát thấy bit INT_STS_BOOT được set lần thứ hai trong khi g_internal_sts_boot_flg đã là 1   km_extend_io.c:317-325
Handler: chính io_extend_write() (main-loop, được gọi lại từ i2c_recv_wait())
Action: poweroff_factor_save(PWROFF_FACTOR_BOOT_SEQ_ERROR) + s800_power_off()   km_extend_io.c:320-321
Next event: liền kề E31 — s800_power_off() kết thúc bằng HAL_NVIC_SystemReset() (E3)
```

### E30 — Sai lệch checksum IAP sau khi cập nhật firmware
```
Trigger: master ghi IAP_PG_COMMAND_CHANGE_INFO với bit hoàn tất được set; check_sum_calc() so sánh giá trị master cung cấp với checksum vừa tính lại   km_i2c_iap.c:334-340
Handler: i2c_recv_wait() (IAP, main-loop)
State change: g_iap_reg[...CHANGE_STATUS] được set bit IAP_PG_COMMAND_CHANGE_STATUS_CHKSUM_ERR
Action: không có gì ngoài cờ trạng thái — firmware KHÔNG tự động thử lại, xóa, hay từ chối cho phép msw_on_proc_iap() reset trở lại Main; master được kỳ vọng đọc thanh ghi trạng thái và tự quyết định
Next event: bất cứ điều gì master bên ngoài làm tiếp theo — có thể là E32 (reset trở lại Main với một image lỗi) nếu master bỏ qua cờ lỗi; [UNKNOWN] liệu có bất kỳ công cụ bên ngoài nào thực sự kiểm tra bit này trước khi tiếp tục, vì logic đó nằm ngoài phạm vi firmware này
```

**Ý nghĩa:** E30 là một khoảng hở thực sự đáng được nêu rõ ràng —
firmware tính toán và phơi bày sự thật về sai lệch checksum, nhưng việc
thực thi "không boot một image lỗi" hoàn toàn là trách nhiệm của master
bên ngoài. Không có gì trong `msw_on_proc_iap()`
(`km_extend_io_iap.c:28-41`) kiểm tra
`IAP_PG_COMMAND_CHANGE_STATUS_CHKSUM_ERR` trước khi reset vào Main.

---

## 8. Các sự kiện chuyển đổi giữa các image (E31-E32)

Cả hai đã được trace đầy đủ trong `03_execution_model.md` §5 và
`04_call_graph.md` §2.9/§2.11; được trình bày lại ở đây thuần túy như
các sự kiện:

| | E31 Main→IAP | E32 IAP→Main |
|---|---|---|
| Trigger | E18 (lệnh giao thức vào IAP) | E8 (cạnh MSW_ON) trong khi bit hoàn tất `g_iap_reg[CHANGE_INFO]` được set |
| Handler | `jump2iap()`, `entry.c:159` | `msw_on_proc_iap()`, `km_extend_io_iap.c:28` |
| Context | main-loop | **ISR** (EXTI4_15) |
| Cơ chế | nhảy PC/SP bằng phần mềm, không reset lõi | `HAL_NVIC_SystemReset()` — reset phần cứng thực sự |
| Trạng thái sống sót | cấu hình ngoại vi của Main (GPIO/RCC) sống sót vào lần khởi tạo lại của IAP | không có — reset toàn bộ xóa mọi thứ (tương đương E1) |
| Bị chặn bởi E30? | n/a | **Không** (§7 ở trên) |

---

## 9. Các quan sát xuyên suốt

1. **Mù nguyên nhân reset (§2)** là điểm "mất thông tin sự kiện" lớn
   nhất trong firmware — bốn trigger khác biệt (E1-E4) tạo ra một kết
   quả không thể phân biệt, được xác nhận bằng grep triệt để thay vì
   giả định từ việc không thấy một lệnh gọi rõ ràng.
2. **Một sự kiện phần cứng, nhiều sự kiện logic**: một giao dịch ghi I2C
   duy nhất (E13/E17) là vật mang cho ít nhất 5 ý nghĩa khác nhau ở tầng
   ứng dụng tùy thuộc vào payload (§4.2) — "sự kiện" mà một con người sẽ
   suy luận về (ví dụ "master vừa yêu cầu cập nhật firmware") cách xa
   một tầng so với bất cứ điều gì NVIC từng vector đến.
3. **Các lỗi lúc boot (E28) về bản chất tệ hơn các lỗi runtime chạm đến
   `Default_Handler`** vì thứ tự khởi động watchdog — một sự thật chỉ có
   thể thấy được bằng cách đối chiếu thứ tự gọi của `main()`
   (`04_call_graph.md` §2.1) với nơi `Error_Handler()` có thể được chạm
   đến (`mx_init.c`, được gọi *trong cùng* trình tự đó, trước khi dòng
   watchdog được thực thi).
4. **E23 phát hiện ra nơi gọi thứ 4 của `s800_power_off()`** bị bỏ sót
   ở Phase 4 — đã được sửa chữa rõ ràng trong §5.2 thay vì âm thầm vá
   vào tài liệu trước đó, để bản thân sự khác biệt vẫn hiện rõ.
5. **Khoảng hở thực thi của E30** là một phát hiện thực sự, có nguồn
   xác nhận, không phải suy đoán: bit sai lệch checksum được tính toán
   và lưu trữ nhưng không bao giờ được kiểm tra bởi hàm duy nhất
   (`msw_on_proc_iap()`) có thể hành động dựa trên nó trước khi trả
   quyền điều khiển về một image Main có thể đã hỏng.
