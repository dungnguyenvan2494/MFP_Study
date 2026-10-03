 # PS-CPU (pscpu_s800) — 03. Mô hình thực thi (Execution Model)

**Phạm vi tài liệu này:** firmware thực sự chạy trên CPU như thế nào — các
execution context, toàn bộ chuỗi interrupt (peripheral → vector → ISR → HAL callback →
BSP → application), và mô hình concurrency/đồng bộ hoá. Tài liệu đồng hành với
`00_project_scope.md` (cái gì/tại sao), `01_repository_map.md` (bảng kê module) và
`02_architecture.md` (phân lớp). Xem `SESSION_CONTEXT.md` để biết quy ước gắn nhãn
bằng chứng (`[INFERRED]` / `[HARDWARE ASSUMPTION]` / `[UNKNOWN]`) được tái sử dụng
bên dưới.

**Ghi chú phương pháp:** khác với phiên trước (vốn phải tái dựng source từ một
bản export Google Drive qua trích xuất JSONL, xem `SESSION_CONTEXT.md` §4), lượt
làm việc này được thực hiện trực tiếp trên cây source `PS-CPU/pscpu_s800/` thực
tế có sẵn trên đĩa trong workspace này — mọi trích dẫn bên dưới đều là một lần
đọc `file:line` trực tiếp, bao gồm cả sáu thân file BSP `.c` mà phiên trước chỉ
suy luận được từ các header.

---

## 1. Mô hình tổng quan

- **Bare-metal, đơn lõi (Cortex-M0, STM32F031C6), một stack duy nhất, không có RTOS.**
  Được xác nhận lại lần này: không có symbol `osKernelStart`/`xTaskCreate`/CMSIS-RTOS
  nào ở bất kỳ đâu trong `main/` hay `iap/`; chỉ có `ARM::CMSIS:CORE` được sử dụng (xem
  `02_architecture.md`).
- **Hai image superloop độc lập dùng chung một flash**, chỉ được vào qua
  `Reset_Handler` của chính mình (Main) hoặc qua một cú nhảy phần mềm (IAP) —
  không bao giờ qua một scheduler dùng chung:
  - Main: `0x08000000`, kích thước `0x5000` (20 KB) — `main/pscpu_s800/iap/Inc/km_i2c_iap.h:118-119`
  - IAP: `0x08005000` (`= MAIN_APPLICATION_ADDRESS + MAIN_APPLICATION_SIZE`) —
    `km_i2c_iap.h:120`, và độc lập là hằng số literal `IAP_DEFAULT_ADD` trong
    `main/Inc/entry.h:26`.
- Trong số 11 nhóm execution-context nằm trong phạm vi của giai đoạn này, chỉ
  những nhóm sau thực sự xuất hiện trong firmware này: **Reset**, **Startup
  (khởi tạo C runtime)**, **superloop trong main()**, và **ISR**. Không có
  RTOS, nên "task/thread", "DMA interrupt" (DMA không bao giờ được bật), và
  "notification/queue/semaphore" không áp dụng được. "Timer callback",
  "peripheral callback", "deferred work" và "event handler" đều tồn tại,
  nhưng dưới dạng *mẫu hình bên trong* các ngữ cảnh ISR/main-loop chứ không
  phải là các execution context có thể lập lịch riêng biệt — xem §4 và §5.

---

## 2. Các execution context

### 2.1 Ngữ cảnh Reset
- Điểm vào: `Reset_Handler` — giống hệt nhau ở cả hai image,
  `main/MDK-ARM/startup_stm32f031x6.s:131-139` (và file
  `iap/MDK-ARM/startup_stm32f031x6.s` giống hệt về byte).
- Trigger: POR, chân NRST, IWDG timeout (§7), các lệnh gọi
  `HAL_NVIC_SystemReset()` (§5), hoặc một lần flash mới. **Không** được kích
  hoạt bởi chuyển tiếp Main→IAP (§5).
- Thân hàm: gọi `SystemInit()` (bring-up clock mặc định của CMSIS) rồi nhảy
  vào `__main` (thư viện C của ARM: xoá `.bss`, sao chép `.data`, sau đó gọi
  `main()`).
- Stack: nạp `SP` từ slot 0 của vector table, `__initial_sp` = đỉnh của một
  vùng **1 KB** duy nhất (`Stack_Size EQU 0x400`, startup `.s:48`). Đây là
  stack *duy nhất* trong toàn hệ thống — `main()` và mọi ISR đều dùng chung
  nó (Cortex-M0 không có sự phân tách PSP/MSP thực dụng ở đây, và không có
  RTOS để cấp cho các task stack riêng).

### 2.2 Ngữ cảnh Startup (khởi tạo C runtime)
Khởi tạo chuẩn của thư viện C ARM giữa `Reset_Handler` và `main()`
(khởi tạo `.bss`/`.data`). Không phải code ứng dụng; không có trích dẫn
thêm ngoài phần trên — chỉ được liệt kê vì Phase 3 yêu cầu coi đây là một
context riêng biệt.

### 2.3 Superloop trong `main()` — Image Main
`main/App/entry.c:45-147`.
- **Init** (`:50-81`): `__HAL_RCC_PWR_CLK_ENABLE()` → `MX_Init()` (clock,
  GPIO, I2C1, RTC, TIM3, IWDG) → `get_model()` → checksum/version →
  `BSP_WDT_Start()` → `internal_sts_init()` → `km_it_init()` →
  `BSP_TIM_Start(TIMER_3,1)` (kích hoạt tick 1 ms, §4.3) → code ADC đã chết
  (`adc_moni_5v_start_proc()`, toàn bộ nằm trong `#if 0`, `km_adc.c:29-39`) →
  `io_extend_memory_write()` → assert `/RESET` → `s800_power_on()` →
  `i2c_recv_first()` → `poweroff_timer_init()`.
- **Thân vòng lặp** (`:83-144`): ~20 lệnh gọi polling tuần tự, không chặn
  (non-blocking) mỗi lượt (reset/MSW/power-monitor/sleep-status/MC-power/
  ERP/USB-OE/v.v.), kết thúc mỗi lượt bằng `i2c_recv_wait()` (rút cạn các
  cờ hoàn tất I2C do ISR của I2C1 thiết lập, §4.4) và `sleep_mode()` (WFI
  có điều kiện, `km_extend_io.c:1395-1404`). Không có chu kỳ cố định — thời
  gian mỗi lượt là tổng chi phí của từng giai đoạn, tất cả đều là truy cập
  GPIO/struct với độ phức tạp O(1).
- Blocking: không có trong thân vòng lặp riêng của Main, ngoại trừ WFI trong
  `sleep_mode()`, vốn quay lại ngay khi có interrupt kế tiếp (chỉ riêng
  SysTick đã đảm bảo ≤1 ms).
- Tài nguyên dùng chung cũng được ghi từ ngữ cảnh ISR (xem §4):
  `pending_factor` (`km_it.c:17`), `g_km_timer[]` (`km_it.c:18`), struct
  `i2c_info` (`km_i2c.c:33-47`), `anti_chattering_info[]` (`km_it.c:20-54`).

### 2.4 Superloop trong `main()` — Image IAP
`iap/App/entry_iap.c:39-79` — ngắn hơn nhiều: `MX_Init()`, checksum/version,
`BSP_WDT_Start()`, `km_it_init_iap()` (**chỉ** đăng ký callback GPIO/EXTI —
không có callback timer nào được đăng ký dù phần cứng TIM3 đã được khởi
tạo, xem §4.3), một bước **relocate vector table thủ công** (§2.5),
`i2c_recv_first()`, sau đó:
```c
while(1) { i2c_recv_wait(); BSP_WDT_Refresh(); }   // entry_iap.c:73-77
```
`i2c_recv_wait()` ở đây cũng là nơi diễn ra việc xoá/lập trình flash
(`km_i2c_iap.c:319, 361`) — vòng lặp này có thể bị chặn trong suốt thời
gian xoá một sector flash (§5, §7).

### 2.5 Việc relocate vector table của IAP (bước khởi tạo một-lần-duy-nhất, đặc thù riêng)
Cortex-M0 không có thanh ghi VTOR, nên image IAP — nằm về mặt vật lý tại
`0x08005000`, không phải địa chỉ reset của CPU — không thể dựa vào cơ chế
vectoring phần cứng để tới bảng vector của chính nó. `main()` sao chép bảng
48 mục của chính nó vào `VectorTable[48]` trong SRAM tại `0x20000000`
(`entry_iap.c:16-22`, vòng lặp sao chép `:55-60`, đọc từ `IAP_ADDRESS`, tức
là vùng flash của chính nó) rồi gọi `__HAL_SYSCFG_REMAPMEMORY_SRAM()`
(`entry_iap.c:68`) để địa chỉ 0 ánh xạ tới bản sao SRAM đó. Main không bao
giờ cần bước này vì Main *chính là* thứ nằm tại địa chỉ reset vật lý của CPU.

### 2.6 Ngữ cảnh ISR (tổng quát)
Cortex-M0 chỉ có 2 bit priority (4 mức). Mọi lệnh gọi `HAL_NVIC_SetPriority`
ở cả hai image đều dùng `(0, 0)` — cùng một mức, cao nhất — xác nhận tại
`main/Src/mx_init.c:158`; `main/Src/stm32f0xx_hal_msp.c:55,57,59,90,137,178`;
và tập hợp giống hệt trong `iap/Src/stm32f0xx_hal_msp.c:55,57,59,90,137`.
**Hệ quả:** không có interrupt nào mà firmware này sử dụng có thể preempt
một interrupt khác — NVIC chỉ phân xử giữa các IRQ *đang chờ (pending)*
cùng mức priority theo số hiệu vector, nó không preempt một IRQ đang chạy.
Mọi ISR đều chạy trên cùng một Main Stack 1 KB duy nhất (§2.1).

---

## 3. Bảng kê vector interrupt

32 vector ngoại vi được định nghĩa tại
`main/MDK-ARM/startup_stm32f031x6.s:94-121` (bố cục giống hệt trong
`iap/MDK-ARM/startup_stm32f031x6.s`). "Sử dụng" nghĩa là được override
trong `stm32f0xx_it.c` của image đó **và** được NVIC-enable ở đâu đó trong
image đó.

| Vector | Image Main | Image IAP | Rơi vào `Default_Handler` nếu bị kích hoạt? |
|---|---|---|---|
| WWDG, PVD, FLASH, RCC | không dùng | không dùng | có (cả hai) |
| RTC | **có dùng** (§4.5) | không dùng | có (IAP) |
| EXTI0_1, EXTI2_3, EXTI4_15 | **có dùng** (§4.2) | chỉ EXTI4_15 (MSW_ON, `iap/Src/stm32f0xx_it.c:130-139`) | có, với EXTI0_1/2_3 trong IAP |
| DMA1_Ch1, DMA1_Ch2_3, DMA1_Ch4_5 | không dùng | không dùng | có (cả hai) |
| ADC1 | được cấu hình nhưng không thể chạm tới (§6) | không dùng | có (cả hai) |
| TIM1_BRK/CC, TIM2, TIM14, TIM16, TIM17 | không dùng | không dùng | có (cả hai) |
| TIM3 | **có dùng**, tick 1 ms (§4.3) | được NVIC-arm nhưng không bao giờ khởi động (§4.3) | n/a với Main / về bản chất là có với IAP nếu từng được khởi động mà không đổi handler — nhưng nó không bao giờ được khởi động |
| I2C1 | **có dùng** (§4.4) | **có dùng** (§4.4) | — |
| SPI1, USART1 | không dùng | không dùng | có (cả hai) |

`Default_Handler` là một vòng lặp vô hạn `B .` (`startup_stm32f031x6.s:165-216`).
Nó **không** cho watchdog ăn (feed), nên cách duy nhất để phục hồi khỏi việc
rơi vào đây là timeout của IWDG (~26 giây, §7) hoặc một reset ngoài.

---

## 4. Truy vết chuỗi interrupt

### 4.1 SysTick (Main + IAP, giống hệt nhau)
`SysTick_Handler` (`main/Src/stm32f0xx_it.c:109-119` /
`iap/Src/stm32f0xx_it.c:108-118`) → `HAL_IncTick()` +
`HAL_SYSTICK_IRQHandler()` → `HAL_SYSTICK_Callback` yếu (weak, không bị
override, không làm gì). Tần số 1 kHz (`HAL_SYSTICK_Config(HCLK/1000)`,
`mx_init.c:153`, priority `(0,0)` tại `mx_init.c:158`). `HAL_GetTick()`
(được nuôi bởi ISR này) được dùng trực tiếp cho việc theo dõi timeout I2C
(`km_i2c.c:237,244`) — một time base 1 kHz **thứ hai**, độc lập, song song
với TIM3 (§4.3, §7).

### 4.2 EXTI0_1 / EXTI2_3 / EXTI4_15 — chỉ ở Main
Phần cứng: 5 chân input GPIOA — `_HRESET_REQ_Pin`=PA1, `AP_PWR_EN_Pin`=PA2,
`POWER_MONITOR_Pin`=PA4, `MSW_ON_Pin`=PA5, `MONI_24V11_Pin`=PA8
(`main/Inc/mxconstants.h:50,52,54,56,76`) →
các vector NVIC `EXTI0_1_IRQn`/`EXTI2_3_IRQn`/`EXTI4_15_IRQn`, priority `(0,0)`
(`mx_init.c:509-516`) →
ISR `main/Src/stm32f0xx_it.c:145-184`, mỗi ISR gọi
`HAL_GPIO_EXTI_IRQHandler(pin)` cho mỗi chân trên line đó →
HAL xoá bit pending của EXTI và gọi `HAL_GPIO_EXTI_Callback` yếu →
được override tại `common/Drivers/BSP/Src/BSP_GPIO_STM32F03x_Nucleo.c:70-76`,
nơi chuyển tiếp tới con trỏ hàm `signal_event` duy nhất đã đăng ký →
handler ứng dụng `km_exti_callback` (`km_it.c:247-282`), được đăng ký qua
`BSP_GPIO_Initialize(km_exti_callback)` tại `km_it.c:367`.

Những gì thực sự xảy ra **bên trong ISR này** (không bị trì hoãn):
- `_HRESET_REQ_Pin` / `AP_PWR_EN_Pin`: chỉ một bit pending-factor được
  thiết lập (`km_it.c:254,257`) — việc xử lý thực sự
  (`hreset_req_proc`/`ap_power_en_proc`) được trì hoãn đến
  `intr_pending_proc()` của main loop
  (`km_extend_io.c:1416-1426`).
- `POWER_MONITOR_Pin` / `MONI_24V11_Pin` / `MSW_ON_Pin`: `start_anti_chattering()`
  chạy trực tiếp trong ngữ cảnh ISR (`km_it.c:261,265,274` → `km_it.c:489-501`),
  cài đặt một bộ đếm ngược mà ISR của TIM3 sau đó sẽ giảm dần (§4.3) — trạng
  thái debounce bị thay đổi từ ngữ cảnh interrupt, không hề bị trì hoãn.
- Trường hợp đặc biệt của `MSW_ON_Pin` (`km_it.c:267-274`): nếu thiết bị
  đang ở chế độ RTC STOP, **chính ISR EXTI này** gọi `SystemClock_Config2()`
  (khoá lại PLL sau khi STOP làm lõi tụt xuống HSI, `mx_init.c:161-200`) và
  `HAL_RTC_DeactivateAlarm()` — nghĩa là việc cấu hình lại clock toàn phần
  chạy bên trong một ISR GPIO.

### 4.3 TIM3 — Main: tick 1 ms đang hoạt động; IAP: đã cấu hình nhưng ở trạng thái ngủ
Main: được khởi động qua `BSP_TIM_Start(TIMER_3,1)` (`entry.c:66`) →
`BSP_TIM_STM32F03x_Nucleo.c:122-169` tính toán Prescaler/Period cho chu kỳ
1 ms rồi gọi `HAL_TIM_Base_Start_IT` → `TIM3_IRQn`, priority `(0,0)`
(`hal_msp.c:178`) → `main/Src/stm32f0xx_it.c:189-198` → `HAL_TIM_IRQHandler`
→ `HAL_TIM_PeriodElapsedCallback` yếu, được override tại
`BSP_TIM_STM32F03x_Nucleo.c:72-88`, nơi chuyển tiếp tới handler đã đăng ký
`km_tim_callback` (`km_it.c:312-352`, đăng ký qua
`BSP_TIM_Initialize(TIMER_3, km_tim_callback)` tại `km_it.c:368`).
Bên trong ISR 1 ms này (`km_it.c:333-350`): bọc thân hàm của chính nó
trong `HAL_NVIC_DisableIRQ`/`EnableIRQ(TIM3_IRQn)` (dư thừa vì sơ đồ
priority phẳng, §2.6), tăng bộ đếm ms chạy tự do
`ui_EventCountTimer` (`:337`), và giảm dần mỗi slot đang được cài đặt của
`g_km_timer[TYPE_Km_Timer_Kind_Max]` (`:340-348`) — **nguồn tick duy nhất
cho mọi timer phần mềm trong firmware** (debounce, trình tự tắt nguồn,
backup-wait).

IAP: clock của TIM3 được bật và `TIM3_IRQn` được NVIC-arm giống hệt như
Main (`iap/Src/stm32f0xx_hal_msp.c:134-138`), nhưng không có lệnh gọi
`HAL_TIM_Base_Start_IT`/`BSP_TIM_Start` nào tồn tại ở bất kỳ đâu trong code
App của IAP (đã xác nhận bằng tìm kiếm) — bộ đếm không bao giờ chạy, nên
`TIM3_IRQn` không bao giờ thực sự nổ ra trong image IAP. Nó ở trạng thái
*đã cấu hình nhưng ngủ (configured-but-dormant)* ở mức thanh ghi, không
đơn thuần là "không tồn tại" — `[INFERRED]` đây là boilerplate còn sót lại
từ việc sao chép mẫu `MX_Init` của Main sang IAP, vì IAP không có tính năng
nào phụ thuộc timer.

### 4.4 I2C1 — Main và IAP, cùng một sơ đồ kết nối
Phần cứng: I2C1 ở chế độ slave, hai địa chỉ riêng (dual own-address) `122`/`124`
(`main/Src/mx_init.c:207,210`), được điều khiển qua lớp bọc kiểu CMSIS
`Driver_I2C0` (`I2C_stm32f0xx.c:777-790`), có `I2C0_Resources.Handle`
là `&hi2c1` (`I2C_stm32f0xx.c:73-81`) — `Driver_I2C1` cũng tồn tại trong
cùng file nhưng handle của nó là `NULL` (`:83-95`) và không bao giờ được
dùng.
`I2C1_IRQn`, priority `(0,0)` → ISR (`main/Src/stm32f0xx_it.c:203-216`, IAP
giống hệt tại `iap/Src/stm32f0xx_it.c:158-171`): đọc trực tiếp
`hi2c1.Instance->ISR` để chọn đường lỗi hay đường sự kiện, gọi
`HAL_I2C_ER_IRQHandler` hoặc `HAL_I2C_EV_IRQHandler` → HAL gọi một trong
các callback I2C yếu, tất cả được override trong `I2C_stm32f0xx.c`:
`HAL_I2C_AddrCallback` (`:767`), `HAL_I2C_SlaveRxCpltCallback` (`:747`),
`HAL_I2C_SlaveTxCpltCallback` (`:736`), `HAL_I2C_ErrorCallback` (`:649`) —
mỗi callback đều đi qua `do_callback()` (`:692-708`) vào `Cb_event` do
ứng dụng đăng ký, chính là `rx_comp` (`km_i2c.c:171-255` với Main; tương
đương trong `km_i2c_iap.c`), được đăng ký qua `Driver_I2C0.Initialize(rx_comp)`.

**`rx_comp()` chính là ranh giới trì hoãn công việc (deferred-work boundary)
của toàn bộ phân hệ này.** Nó chạy hoàn toàn trong ngữ cảnh ISR và chỉ làm
đúng ba việc:
1. ACK/NACK byte địa chỉ vừa nhận dựa trên việc kiểm tra phạm vi hợp lệ
   (`km_i2c.c:200-213`, `valid_second_addr()` tại `:93-156`);
2. thiết lập một trong hai cờ hoàn tất, `i2c_info.read_comp` /
   `i2c_info.write_comp` (`:217-229`);
3. ghi lại một bitmask lỗi (`:251-253`).

Nó không bao giờ chạy switch dispatch lệnh, không bao giờ chạm vào flash,
và không bao giờ tự gọi `SlaveTransmit`/`SlaveReceive` — tất cả việc đó
xảy ra sau, trong `i2c_recv_wait()` của main-loop (`km_i2c.c:485-...`, IAP:
`km_i2c_iap.c:240-399`), một khi hàm này quan sát thấy một cờ hoàn tất được
thiết lập. **Việc poll cờ này chính là toàn bộ cơ chế "deferred work" của
firmware này** — không có queue, không có semaphore, không có gì mang hình
dáng RTOS.

Lệnh nhảy sang IAP của Main được giải mã bên trong dispatch của main-loop
đó, không phải trong ISR: `km_i2c.c:686-693`, `second_addr == IAP_PG_COMMAND`
và `data == IAP_PG_COMMAND_MAGICNUM` → `jump2iap()` (§5).

### 4.5 RTC Alarm — chỉ ở Main
`HAL_RTC_SetAlarm_IT`, được cài đặt bởi `set_cycle_time()`
(`km_alarm_wake.c:41-110`, cài đặt tại `:105`), bản thân hàm này được gọi từ
`km_extend_io.c:1202` ngay trước `BSP_PWR_enter_stopmode()`
(`:1203`) khi MSW tắt và `SB_PG` tắt →
`RTC_IRQn`, priority `(0,0)` (`hal_msp.c:137`) →
`main/Src/stm32f0xx_it.c:131-140` → `HAL_RTC_AlarmIRQHandler` →
`HAL_RTC_AlarmAEventCallback` yếu, được override tại `km_it.c:284-300`: nếu
`stop_mode` đang được thiết lập, xoá cờ RTC alarm bằng một macro thanh ghi
trực tiếp và xoá `stop_mode` (`:288-289`). Đây chính là cơ chế tự đánh thức
định kỳ (chu kỳ 2 giây, thiết lập tại `km_extend_io.c:1202`) cho phép
firmware tự kiểm tra lại các điều kiện thoát khỏi chế độ STOP mà không cần
một cạnh GPIO trực tiếp.

---

## 5. Chuyển tiếp giữa các image (không qua `Reset_Handler`)

- **Main → IAP**: `jump2iap()` (`entry.c:159-172`), được gọi từ dispatch
  lệnh của `i2c_recv_wait()` trong **ngữ cảnh main-loop** (không phải ISR,
  không phải reset) — `km_i2c.c:686-693`. Trình tự: `DeInit()`
  (`entry.c:186-192` — dừng TIM3, uninitialize driver I2C0, vô hiệu hoá RTC
  alarm) → nạp `SP` từ `*(IAP_DEFAULT_ADD)` → nhảy tới
  `*(IAP_DEFAULT_ADD+4)`. Đây là một cú nhảy PC/SP **phần mềm** vào giá trị
  reset riêng của IAP, không kèm một reset lõi thực sự — các peripheral mà
  Main đã cấu hình (hướng GPIO, cây clock RCC) vẫn còn sống khi `MX_Init()`
  của IAP chạy lại đè lên trên chúng.
- **IAP → Main**: luôn luôn là một reset phần cứng thực sự,
  `HAL_NVIC_SystemReset()` (`iap/App/km_extend_io_iap.c:37`), được kích hoạt
  **từ bên trong ISR EXTI của MSW_ON** (`km_extend_io_iap.c:28-41`,
  `msw_on_proc_iap()`) khi `MSW_ON` được nhả ra và
  `IAP_PG_COMMAND_CHANGE_INFO_COMP` được thiết lập trong `g_iap_reg[]`.
  Khác với đường Main→IAP, đường này *có* đi qua `Reset_Handler`/`SystemInit`/
  `__main` một lần nữa, nên trạng thái peripheral được đảm bảo sạch khi
  quay về Main.

---

## 6. Các đường interrupt đã xác nhận chết / được cấu hình nhưng bất động

Không có vector nào trong số WWDG/PVD/FLASH/RCC/DMA1×3/TIM1×2/TIM2/TIM14/TIM16/TIM17/SPI1/USART1
từng được NVIC-enable ở bất kỳ image nào (không có lệnh gọi
`HAL_NVIC_EnableIRQ` nào cho bất kỳ vector nào trong số này tồn tại trong
`main/` hay `iap/`), nên trong điều kiện vận hành bình thường không vector
nào trong số đó có thể nổ ra.

ADC là một ngoại lệ một phần đáng chú ý: bản thân peripheral đã được cấu
hình (`hadc`, được tham chiếu tại `BSP_ADC_STM32F03x_Nucleo.c:11`), và
`BSP_ADC_Start()` có gọi `HAL_ADC_Start_IT()` (`:57`), điều này sẽ bật
interrupt EOC của chính ADC — nhưng cả hai điểm gọi `BSP_ADC_Start` đều
nằm bên trong khối `#if 0` (`km_adc.c:29-39` và `:56-77`, toàn bộ thân hàm
của cả hai hàm đều đã chết), nên trong firmware đã xuất xưởng đường đó
không bao giờ được chạm tới và `ADC1_IRQn` cũng không bao giờ được
NVIC-enable. **Nguy cơ tiềm ẩn:** nếu dead code này từng được bật lại mà
không đồng thời thêm một override `ADC1_IRQHandler` vào `stm32f0xx_it.c`,
interrupt EOC đầu tiên của ADC sẽ làm treo CPU trong vòng lặp vô hạn của
`Default_Handler` — chỉ có thể phục hồi qua timeout IWDG ~26 giây (§7)
hoặc một reset ngoài, vì `Default_Handler` không bao giờ cho watchdog ăn.

---

## 7. Ngân sách timing / stack / độ tin cậy

- **Watchdog:** IWDG, LSI (~40 kHz `[HARDWARE ASSUMPTION]`) / prescaler 256,
  reload 4095 (`mx_init.c:233-235`) → timeout ≈ `4095 × 256 / 40000` ≈
  **26,2 giây**; được refresh một lần mỗi lượt superloop ở cả hai image
  (`entry.c:89`, `entry_iap.c:76`). Một lượt lặp bị kẹt — kể cả bị treo
  trong `Default_Handler` — có ~26 giây trước khi một reset phần cứng phục
  hồi nó.
- **Hai time base 1 kHz độc lập, không đồng bộ** cùng tồn tại: SysTick
  (`HAL_GetTick()`, §4.1) và bộ đếm ms riêng của TIM3 / mảng `g_km_timer[]`
  (§4.3, chỉ ở Main). Không có gì gắn kết chúng lại với nhau; mỗi phân hệ
  trong code chọn dùng cái mà tác giả của nó thấy phù hợp.
- **Stack:** một vùng MSP 1 KB duy nhất (`startup...s:48`) được dùng chung
  bởi `main()` và mọi ISR — không có công cụ đo mức sử dụng stack nào tồn
  tại trong repo; với tính chất nông, không đệ quy của mọi ISR đã đọc trong
  phiên này, kết luận này là `[INFERRED]` đủ dùng nhưng chưa được xác minh
  bằng bất kỳ phân tích stack tĩnh/động nào.
- **Kiểm soát concurrency hoàn toàn theo kiểu ad hoc** (sơ đồ priority
  phẳng, §2.6 nghĩa là không có preemption để cần bảo vệ chống lại ngay từ
  đầu — nguy cơ thực sự là một truy cập main-loop đụng độ (race) với một
  lần ghi từ ISR). Ba cơ chế khác nhau cùng tồn tại cho việc đó:
  1. cặp disable/enable cả 3 line EXTI dùng quanh việc truy cập
     `pending_factor` (`disable_external_irq`/`enable_external_irq`,
     `km_it.c:90-127`, bọc quanh `get_pending_factor`/`put_pending_factor`
     tại `:141-163`);
  2. wrapper `km_disable_irq`/`km_enable_irq` có đếm tham chiếu
     (`km_it.c:436-460`), primitive "đúng chuẩn" theo `02_architecture.md`;
  3. hai vùng critical-section dùng `NVIC_DisableIRQ`/`HAL_NVIC_xxxIRQ`
     **thô**, bỏ qua hoàn toàn wrapper (2): `s800_power_off()`
     (`km_extend_io.c:538-540`, disable trực tiếp cả 3 line EXTI — đã được
     đánh dấu là vi phạm phân lớp trong `02_architecture.md`) và
     `s800_power_on()` (`km_extend_io.c:627,637`, bọc quanh một đoạn cập
     nhật 3 dòng cho `ui_EventCountTimer`/`setEventRecord()` bằng một cặp
     `HAL_NVIC_DisableIRQ`/`EnableIRQ(TIM3_IRQn)` thô). Cả hai chỉ vô hại vì
     *thời điểm* chúng chạy (chuyển tiếp nguồn, không phải trạng thái ổn
     định), không phải vì chúng tuân theo cùng một kỷ luật với phần còn lại
     của code.

---

## 8. Phân tích RTOS

**N/A.** Không có RTOS nào được link hay sử dụng — được xác nhận lại lần
này trên mọi file đã đọc (xem §1). Do đó không có task, không có lập lịch
theo priority, không có queue, semaphore, mutex, event group, software
timer kiểu RTOS, task notification, hay critical section ở mức OS ở bất kỳ
đâu trong firmware này. Mỗi khái niệm trong số đó đều được tái triển khai
theo kiểu ad hoc thay thế:

| Khái niệm RTOS | Thứ mà firmware này dùng thay thế |
|---|---|
| Task / scheduler | Một superloop `while(1)` cho mỗi image; thứ tự là thứ tự nguồn tĩnh (§2.3/§2.4) |
| Queue / semaphore (bàn giao ISR→task) | Các cờ hoàn tất kiểu `int`/`bool` thuần tuý được vòng lặp poll (`i2c_info.read_comp`/`write_comp`, §4.4) |
| Software timer | Mảng `g_km_timer[]` tự viết tay, được ISR của TIM3 giảm dần (§4.3) |
| Mutex / critical section | Enable/disable NVIC thô hoặc có đếm tham chiếu (§7) — không có priority-inheritance, không có timeout |
| Event group | Bitmask `pending_factor` + `set_pending_factor_bit`/`read_pending_factor_bit` (`km_it.c:186-234`) |

---

## 9. Các mục còn bỏ ngỏ (chưa giải quyết trong phiên này)

- `km_ca72_status.c` và phần còn lại của `km_alarm_wake.c` ngoài
  `set_cycle_time()` chưa được truy vết từng dòng để tìm thêm tương tác
  interrupt trong lượt này; dựa trên kích thước file và phân loại của phiên
  trước, chúng chỉ mang tính polling (đọc GPIO trạng thái / cài đặt alarm)
  không có thêm đăng ký ISR nào — `[INFERRED]`, chưa được xác minh ở đây.
- Các giá trị dual own-address của I2C1 (`122`/`124`, `mx_init.c:207,210`)
  đã được đọc nhưng chưa được đối chiếu chéo với một tài liệu giao
  thức/register-map bên ngoài — nằm ngoài phạm vi của một lượt phân tích
  execution-model.
- Chưa có phân tích độ sâu stack tĩnh nào được chạy trên cả hai image; kết
  luận "1 KB có lẽ là đủ" trong §7 là một suy luận từ hình dạng code, không
  phải một phép đo.
