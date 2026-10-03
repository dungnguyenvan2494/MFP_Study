 # PS-CPU (pscpu_s800) — 04. Call Graph

**Phạm vi của tài liệu này:** đây không phải là một bản dump call-graph máy móc —
mọi cạnh (edge) bên dưới đều được lần theo bằng tay trong source code và được
giải thích theo *lý do tại sao* nó tồn tại và *ý nghĩa* của nó đối với hệ thống
(các hazard về thứ tự, dữ liệu cũ (staleness), việc block, ai sở hữu state
nào), chứ không chỉ đơn thuần *rằng* nó tồn tại. Đây là tài liệu đồng hành với
`03_execution_model.md` (các execution context/ISR — hãy đọc file đó trước,
tài liệu này giả định người đọc đã nắm vốn từ vựng của nó: không có RTOS, flat
NVIC priority, flag-polling là cơ chế deferred-work). Các nhãn bằng chứng
(`[INFERRED]`/`[HARDWARE ASSUMPTION]`/`[UNKNOWN]`) tuân theo quy ước trong
`SESSION_CONTEXT.md`. Mọi trích dẫn đều là các lần đọc trực tiếp `file:line`
đối chiếu với `PS-CPU/pscpu_s800/`.

**Về phạm vi:** các hàm wrapper GPIO một dòng đơn giản (`BSP_GPIO_ReadPin`/
`WritePin`) được xem như các leaf và không được mở rộng ở mọi nơi chúng được
dùng — chúng xuất hiện trong danh sách "Callee" nhưng không có mục riêng của
chúng. Trọng tâm là khoảng ~30 hàm mang state, đưa ra quyết định, hoặc băng
qua ranh giới execution-context.

---

## 1. Chú giải ký hiệu (Legend)

```
A → B     direct call (A's source names B)
A ⇢ B     indirect call through a function pointer / struct-of-pointers
A ↝ f     A registers f as a callback (stores it, does not call it now)
⚡A→B     interrupt entry: hardware vectors into A, which calls B
A ⇒ B     A is one of several dispatch targets picked by a jump table / switch
```

---

## 2. Các luồng thực thi chính (Major execution paths)

### 2.1 Main image — cold boot đến vòng lặp superloop đầu tiên

```
ENTRY  Reset_Handler                    main/MDK-ARM/startup_stm32f031x6.s:131
  → SystemInit()                        (CMSIS, default clock — not app clock)
  → __main (ARM C lib: .data/.bss init)
  → main()                              entry.c:46
      → __HAL_RCC_PWR_CLK_ENABLE()
      → MX_Init()                       mx_init.c:81
          → HAL_Init()
          → SystemClock_Config()        mx_init.c:113   (real app clock: PLL from HSI)
          → MX_GPIO_Init()
          → MX_I2C1_Init()              mx_init.c:202   (also called again later by i2c_sw_reset(), §2.8)
          → MX_RTC_Init(...) / KM_RTC_Restore() / KM_RTC_ALARM_Init()
          → MX_TIM3_Init()              (configures, does NOT start — §2.5 starts it)
          → MX_IWDG_Init()
      → get_model()                     km_extend_io.c:1480  (reads+caches 2 GPIO straps once)
      → checksum_calc()                 km_i2c.c:771     (sums own flash image 0x08000000..+0x4FFE)
      → GetVersionString()              km_i2c.c:800     (copies version bytes out of own flash into RAM)
      → BSP_WDT_Start()                 → HAL_IWDG_Start()
      → internal_sts_init()             km_extend_io.c:366
      → km_it_init()                    km_it.c:364      ↝ registers km_exti_callback, km_tim_callback (§3)
      → BSP_TIM_Start(TIMER_3, 1)       → arms the 1 ms tick (see 03_execution_model.md §4.3)
      → adc_moni_5v_start_proc()        km_adc.c:27      [DEAD — body is #if 0]
      → io_extend_memory_write()        km_extend_io.c:339 (seeds g_io_extend_memory[] from live GPIO)
      → BSP_GPIO_WritePin(_RESET, RESET)
      → s800_power_on()                 km_extend_io.c:607  (§2.9)
      → i2c_recv_first()                km_i2c.c:317     ⇢ Driver_I2C0.Initialize(rx_comp) — arms I2C1 slave RX
      → poweroff_timer_init()
  → while(1) { … }                      entry.c:83        (§2.2, never returns)
RETURN  — never (superloop is terminal; only jump2iap()/reset leave it)
```

**Ý nghĩa:** toàn bộ đoạn trên chạy đơn luồng (single-threaded), interrupt chỉ
được bật toàn cục từ thời điểm `HAL_Init()`/`MX_GPIO_Init()` bắt đầu bật các
đường NVIC. Có hai thứ được tính toán **trước** khi loop bắt đầu và sau đó chỉ
mutate qua đúng call path riêng của chúng: `get_model()` cache câu trả lời của
nó vào một biến `static` mãi mãi (nên việc thay đổi strap cần một lần reset
đầy đủ mới có hiệu lực), và `checksum_calc()`/`GetVersionString()` chỉ được
chạy lại sau đó *duy nhất* bởi IAP image sau khi một lần cập nhật firmware
hoàn tất (`km_i2c_iap.c:345-346`) — bản thân Main không bao giờ tính lại chúng
sau khi boot.

### 2.2 Main image — một vòng lặp superloop (steady state)

```
ENTRY  while(1) body                    entry.c:83-144
  → sb_reset_proc()                     km_extend_io.c:1231
  → moni_24v_proc()                     km_extend_io.c:575
  → BSP_WDT_Refresh()                   → HAL_IWDG_Refresh()   [feeds the ~26.2s watchdog]
  → msw_on_proc(STATE_NORMAL)           km_extend_io.c:1149    (§2.9 — may call s800_power_off/on, BSP_PWR_enter_stopmode)
  → power_monitor_proc(STATE_NORMAL)    km_extend_io.c:445
  → sleep_status_rem_proc(model)        km_extend_io.c:1055  ⇒ jump table (§3.5) — READS get_ca72_status()
  → mc_p_on_proc(model)                 km_extend_io.c:839   ⇒ jump table (§3.5)
  → ir_p_on_proc()                      km_extend_io.c:862
  → mc3_3von_moni_proc()                km_extend_io.c:425
  → erp_sensor_proc()                   km_extend_io.c:687    (also reachable re-entrantly from io_extend_write(), §2.7)
  → mc_oe_proc()                        km_extend_io.c:1072   (no-op stub, signal removed per its own comment)
  → mc_pwr_en_proc()                    km_extend_io.c:1096
  → adc_moni_5v_off_proc()              km_adc.c:54            [DEAD — body is #if 0]
  → rst_usb_proc(STATE_NORMAL)          km_extend_io.c:711
  → usb2_oe_proc()                      km_extend_io.c:1373
  → ap_power_en_proc(STATE_NORMAL)      km_extend_io.c:658    WRITES set_ca72_status()  ← see hazard note below
  → intr_pending_proc()                 km_extend_io.c:1416
      → hreset_req_proc()               km_extend_io.c:489       [if HRESET_REQ pending bit set]
      → ap_power_en_proc(STATE_INT)     km_extend_io.c:658       [if AP_PWR_EN pending bit set] → set_ca72_status(CA72_ON)
  → anti_chattering_proc()              km_extend_io.c:1438
      ⇢ info->intr_proc(STATE_INT)      km_extend_io.c:1460      indirect call (§3.6) → msw_on_proc / power_monitor_proc / (none)
  → poweroff_timer_proc()               km_extend_io.c:1305
  → backupwait_timer_proc()             km_extend_io.c:1350
  → i2c_recv_wait()                     km_i2c.c:485           (§2.6/§2.7 — the I2C command dispatcher)
  → sleep_mode()                        km_extend_io.c:1395    → BSP_PWR_enter_sleepmode() (conditional WFI)
RETURN  — loops back to sb_reset_proc()
```

**Ý nghĩa — một hazard thứ tự có thật, không chỉ là một danh sách:**
`sleep_status_rem_proc()` chạy ở **đầu** vòng lặp và đọc `get_ca72_status()`
(`km_extend_io.c:896,971`), nhưng những nơi duy nhất ghi status đó,
`set_ca72_status(CA72_ON)`/`set_ca72_status(CA72_SLEEP)`, nằm bên trong
`ap_power_en_proc()` (`:665,670`), hàm này chạy **muộn hơn trong cùng một lần
lặp** (`entry.c:114`, và lại có điều kiện từ `intr_pending_proc()` tại
`entry.c:116`). Vì vậy `sleep_status_rem_proc()` luôn hành động dựa trên
CA72 status của lần lặp *trước đó*, không bao giờ là lần lặp hiện tại — một
độ trễ một-lần-lặp (one-iteration lag) mà nếu nhìn riêng lẻ từng hàm sẽ không
thấy được và chỉ lộ ra khi đọc thứ tự gọi của cả vòng lặp. Vì thân vòng lặp chỉ
là một vài lần đọc GPIO (microsecond), độ trễ này `[INFERRED]` là vô hại trong
thực tế, nhưng nó là một cửa sổ dữ liệu cũ (staleness window) có thật mà nếu
chỉ đọc từng hàm riêng lẻ một cách ngây thơ sẽ hoàn toàn bỏ lỡ — đây chính xác
là lý do phase này yêu cầu call graph phải được lần theo bằng tay, không phải
được sinh tự động.

### 2.3 EXTI interrupt → arm debounce (ISR context)

```
⚡ HRESET_REQ/AP_PWR_EN/POWER_MONITOR/MSW_ON/MONI_24V11 pin edge
  → EXTIx_y_IRQHandler                  main/Src/stm32f0xx_it.c:145-184
  → HAL_GPIO_EXTI_IRQHandler(pin)       (HAL: clears EXTI pending bit)
  ⇢ HAL_GPIO_EXTI_Callback(pin)         BSP_GPIO_STM32F03x_Nucleo.c:70   [weak override]
  ⇢ signal_event(pin, GPIO_EVENT_EXT_INTR)   — function pointer, registered §3.1
      = km_exti_callback(pin, event)    km_it.c:247
          ⇒ set_pending_factor_bit(...)         [_HRESET_REQ_Pin, AP_PWR_EN_Pin — deferred]
          ⇒ start_anti_chattering(idx, level)   [POWER_MONITOR/MONI_24V11/MSW_ON — armed NOW]
              → km_timer_set(timer_kind, decision_time, Start)
          ⇒ (MSW_ON only, if stop_mode) SystemClock_Config2() + HAL_RTC_DeactivateAlarm()
RETURN to interrupted context (main loop or another ISR body)
```

**Ý nghĩa:** ISR này đưa ra một *quyết định* thực sự, không chỉ đơn thuần set
một flag — hai trong số năm pin có việc xử lý được deferred sang main loop
(`intr_pending_proc()`, §2.2) trong khi ba pin còn lại có timer debounce của
chúng được arm ngay lập tức, từ interrupt context, trước khi main loop chạy
lại lần nữa. Nhánh `MSW_ON` còn có nghĩa là một lần relock PLL đầy đủ có thể
xảy ra bên trong một GPIO ISR — điều đáng biết trước khi giả định rằng các
ISR ở đây "chỉ" nhanh và đồng nhất.

### 2.4 TIM3 interrupt → software timebase (ISR context, chỉ Main)

```
⚡ TIM3 update event (1 ms)
  → TIM3_IRQHandler                     main/Src/stm32f0xx_it.c:189
  → HAL_TIM_IRQHandler(&htim3)
  ⇢ HAL_TIM_PeriodElapsedCallback(htim) BSP_TIM_STM32F03x_Nucleo.c:72   [weak override]
  ⇢ Resource[type].SignalHandler(type, TIM_EVENT_PERIOD_ELAPSED)  — function pointer, registered §3.2
      = km_tim_callback(TIMER_3, event) km_it.c:312
          → ui_EventCountTimer++
          → for each g_km_timer[i]: decrement, mark End at 0
RETURN
```

**Ý nghĩa:** đây là nơi **duy nhất** `g_km_timer[]` bị decrement. Mọi
"timer" trong firmware này — debounce, trình tự power-off, backup-wait,
stopwatch sự kiện boot — thực chất chỉ là một mảng dùng chung, được đọc bởi
nhiều hàm main-loop (`km_timer_get_state()`) và chỉ được ghi bởi đúng một ISR
1 ms này cộng với các lệnh gọi `km_timer_set()` rải rác trong code main-loop
(§3, bảng shared-state). Không có timer phần cứng độc lập nào cho từng
feature; tất cả đều multiplex lên trên đúng một mảng này.

### 2.5 I2C1 interrupt → completion flag (ISR context, Main + IAP)

```
⚡ I2C1 event/error
  → I2C1_IRQHandler                     main/Src/stm32f0xx_it.c:203  (iap/Src/stm32f0xx_it.c:158, identical shape)
  → HAL_I2C_EV_IRQHandler / HAL_I2C_ER_IRQHandler  (HAL, chosen by reading hi2c1.Instance->ISR directly)
  ⇢ HAL_I2C_AddrCallback / SlaveRxCpltCallback / SlaveTxCpltCallback / ErrorCallback
      I2C_stm32f0xx.c:767 / :747 / :736 / :649   [weak overrides]
  → do_callback(hi2c, event)             I2C_stm32f0xx.c:692
  ⇢ resource->Cb_event(event)            — function pointer, registered §3.3
      = rx_comp(event)                   km_i2c.c:171  (Main) / km_i2c_iap.c:90 (IAP)
          → Driver_I2C0.Control(ACK/NACK, ...)      ⇢ indirect, struct member (§3.4)
          → i2c_info.read_comp = 1  |  i2c_info.write_comp = 1
          → i2c_info.error_code |= ...
RETURN
```

**Ý nghĩa:** `rx_comp()` cố tình được thiết kế "nông" — ba lần ghi
flag/field và một quyết định ACK/NACK, không gì khác. Đây là ranh giới cứng
giữa "ISR context" và "mọi thứ còn lại" cho toàn bộ subsystem I2C: không có
lệnh nào từng được diễn giải (interpret) ở đây. Ai đọc `km_i2c.c` từ trên
xuống dưới cũng có thể nhầm `rx_comp` là "trình xử lý I2C"; call graph làm rõ
rằng nó chỉ là phần ba đầu tiên của pipeline — hai phần ba còn lại chạy sau đó
0 đến ~vài ms trong main loop (§2.6).

### 2.6 I2C command dispatch — main-loop context (Main image, đọc register)

```
ENTRY  i2c_recv_wait()                  km_i2c.c:485      (called every iteration, entry.c:126)
  → i2c_check_error()                   km_i2c.c:379      [HAL_NVIC_DisableIRQ(I2C1_IRQn) critical section, §5]
      if timeout/error → i2c_sw_reset() km_i2c.c:356 → return   (§2.8)
  if (!read_comp && !write_comp) → return   [nothing to do this iteration]
  switch(i2c_info.state):
    case TYPE_State_Wait_Write, 1-byte xfer (a register READ request from the I2C master):
      switch(second_addr):
        RTC_* / B_REGISTER_*      → change_state(..., Driver_I2C0.SlaveTransmit, (uint8_t*)(RTC_BASE+addr), ...)   [km_i2c.c:553]
        EXTEND_OUTPUT/INPUT/*STS* → io_extend_read(addr)          km_extend_io.c:192  → change_state(...)          [km_i2c.c:561]
        EXTEND_CHECKSUM_COMMAND   → tx_data = g_checksum_value    → change_state(...)                              [km_i2c.c:575]
        EXTEND_VERSION_COMMAND    → change_state(..., &g_main_version[0], ...)                                     [km_i2c.c:580]
        EXTEND_EVENT_COMMAND      → change_state(..., &st_EventRecord[0], sizeof(...))                             [km_i2c.c:612]
        default                   → tx_data = 0xffff (undefined-address sentinel)
      ⇢ change_state(next_state, &comp, Driver_I2C0.SlaveTransmit, buf, size, NULL, 0)   km_i2c.c:274  (§3.4)
          → HAL_NVIC_DisableIRQ(I2C1_IRQn)
          → (*transfer)(buf, size)        = Driver_I2C0.SlaveTransmit(buf,size)   ⇢ indirect, arms next HW transfer
          → HAL_NVIC_EnableIRQ(I2C1_IRQn)
RETURN — i2c_info.state now TYPE_State_Wait_Read, waiting for the master to clock the reply out (ISR will set read_comp)
```

**Ý nghĩa:** mỗi lần "đọc register" mà I2C master thực hiện đều được trả lời
bằng cách copy **live application state** vào một vùng scratch cấp module
(`tx_data`, hoặc một con trỏ trực tiếp vào memory-mapped register của RTC,
hoặc `g_main_version`) và đưa con trỏ đó cho driver I2C của CMSIS — không có
bước snapshot/copy nào bảo vệ giá trị khỏi bị thay đổi giữa lúc
`change_state()` arm transmit và lúc master clock-out xong; với các lần đọc
RTC điều này ít quan trọng nhất (RTC hoàn toàn là passthrough, xem
`03_execution_model.md` §về quyền sở hữu RTC), nhưng với `g_checksum_value`/
`g_main_version` điều đó có nghĩa là một lần ghi đồng thời (chỉ có thể xảy ra
từ IAP image sau khi cập nhật firmware, không phải từ chính Main) về nguyên
tắc có thể race — `[INFERRED]` không phải vấn đề thực tế vì Main không bao
giờ ghi lại hai giá trị đó sau khi boot.

### 2.7 I2C command dispatch — main-loop context (Main image, ghi register / các lệnh có side-effect)

```
ENTRY  i2c_recv_wait(), case Wait_Write, xfer_size > 1 (a WRITE)   km_i2c.c:629
  switch(second_addr):
    RTC_*                        → direct pointer write to RTC_BASE+offset, RTC_WRITEPROTECT_DISABLE/ENABLE around it   [km_i2c.c:657-661]
    B_REGISTER_*                 → read-modify-write same way, 16-bit field packed into a 32-bit backup register        [km_i2c.c:663-673]
    EXTEND_OUTPUT_HIGH/LOW/*CTL* → io_extend_write(addr, data)      km_extend_io.c:249    (§2.7.1 below)
    IAP_PG_COMMAND (magic match) → DeInit() + jump2iap()            km_i2c.c:691-692      (§2.9, terminal for this image)
    EXTEND_PWROFF_WDGTIME_SET_CMD/EXTEND_PWROFF_SEQTIME_SET_CMD → km_timer_set(...)
    EXTEND_RESET_I2C             → i2c_sw_reset(); return           km_i2c.c:721          (§2.8, master-requested reset)
    EXTEND_EVENT_COMMAND         → setEventRecord(data)             km_i2c.c:726
  → change_state(Wait_Write, &write_comp, Driver_I2C0.SlaveReceive, rx_buf, ..., Driver_I2C0.Control, 1)   [re-arm for next command]
RETURN
```

#### 2.7.1 `io_extend_write()` — fan-out tái nhập (re-entrant) vào các hàm khác thuộc chính superloop
```
io_extend_write(addr, data)             km_extend_io.c:249
  → change_cmd2array(addr)              km_extend_io.c:98     [table-driven address→index mapping, not a jump table]
  → g_io_extend_memory[num] updated (OR/AND-NOT/assign depending on which of 3 command variants)
  → for each io_extend[] table row matching num (14-entry const table, km_extend_io.c:50-65):
      ⇒ erp_sensor_proc()                km_extend_io.c:687     [special-cased pin — re-enters a normal loop-body function from inside I2C dispatch]
      (MC_P_ON / IR_P_ON / MC_PWR_EN / SB_PWR_EN pins are explicitly skipped here — comment says "controlled elsewhere", by mc_p_on_proc/ir_p_on_proc/mc_pwr_en_proc in the loop body)
      else → BSP_GPIO_WritePin(...)
  if (Internal_Sts0 boot bit newly set twice) → poweroff_factor_save() + s800_power_off()   km_extend_io.c:320-321   (§2.9)
RETURN
```

**Ý nghĩa:** đây là điểm tái nhập (re-entrancy) thứ hai trong codebase (điểm
đầu tiên là TIM3 ISR chạm vào `g_km_timer[]`, §2.4): một hàm được gọi từ *bên
trong* I2C main-loop dispatch (`io_extend_write`) có thể gọi
`erp_sensor_proc()` hoặc `s800_power_off()` — chính những hàm mà thân
superloop bình thường cũng gọi trực tiếp mỗi lần lặp (§2.2). Không có gì ngăn
cả hai call path chạy trong cùng một lần lặp (thân loop gọi
`erp_sensor_proc()`, rồi sau đó, trong cùng lượt đó, một lần ghi I2C lại
trigger nó lần nữa) — `[INFERRED]` idempotent theo thiết kế (cả hai chỉ đơn
thuần resolve GPIO state từ `g_io_extend_memory`), nhưng điều đó có nghĩa là
câu hỏi "ai gọi `erp_sensor_proc()`" có hai câu trả lời, không phải một.

### 2.8 I2C software reset — một recovery path dùng chung với 4 call site, và một "người anh em" đã xác nhận là dead

```
i2c_sw_reset()                          km_i2c.c:356
  → __HAL_RCC_I2C1_FORCE_RESET() / HAL_Delay(2) / __HAL_RCC_I2C1_RELEASE_RESET()
  → Driver_I2C0.Uninitialize()          ⇢ indirect, struct member
  → MX_I2C1_Init()                      mx_init.c:202   (re-run, second time post-boot)
  → i2c_recv_first()                    km_i2c.c:317    (re-arms slave RX, §2.1)
```
Các caller (đều ở main-loop context): `i2c_recv_wait()` khi phát hiện
error/timeout (`km_i2c.c:494`), khi có mismatch giao thức write/read
(`:741,748`), và khi master chủ động gửi `EXTEND_RESET_I2C` (`:721`).

Có một hàm reset **thứ hai, tên riêng biệt** làm gần như cùng một việc —
`i2c_reset()` (`km_extend_io.c:1534-1539`: force-reset clock của peripheral
I2C1 rồi `MX_I2C1_Init()`, nhưng *không* có `HAL_Delay(2)` và *không* re-arm
`i2c_recv_first()`). Grep toàn bộ cây `main/` và `iap/` để tìm `i2c_reset()`
chỉ cho ra **đúng định nghĩa của chính nó** — điều này giải quyết dứt điểm
mục `[UNKNOWN]` còn để ngỏ từ `SESSION_CONTEXT.md`/`02_architecture.md`:
**`i2c_reset()` không có bất kỳ caller nào trong toàn bộ firmware này; nó đã
được xác nhận là dead code**, chứ không chỉ là "chưa tìm thấy caller."

### 2.9 Trình tự nguồn điện (Power sequencing) — ba hàm gọi lẫn nhau và mỗi hàm kết thúc theo một cách khác nhau

```
s800_power_on()                         km_extend_io.c:607
  called from: main() init (entry.c:75), msw_on_proc() (km_extend_io.c:1213, MSW re-asserted while SB_PG is down)
  → set/check GPIO (SB_PWR_EN, timers) → km_timer_set(SB_RESET_RELEASE, 100ms, Start)
  → [if g_timer_flg == NORMAL] HAL_NVIC_DisableIRQ(TIM3_IRQn) → setEventRecord(SwitchOn) → HAL_NVIC_EnableIRQ(TIM3_IRQn)
      ← 3rd raw NVIC critical-section instance in this codebase (besides s800_power_off and the reference-counted wrapper — see 03_execution_model.md §7)
RETURN normally (this is the one non-terminal function of the three)

s800_power_off()                        km_extend_io.c:523
  called from: msw_on_proc() (km_extend_io.c:1166, off-sequence timer elapsed), power_flicker_failsafe() (:178), io_extend_write() boot-notify-storm guard (:321)
  → set_ca72_status(CA72_OFF) → poweroff_timer_init() → clear_pending_factor_bit(All)
  → NVIC_DisableIRQ × 3 (raw, bypasses km_disable_irq — flagged in 02_architecture.md and 03_execution_model.md §7)
  → extend_output_init() → get_model() → sleep_status_rem_proc(model)  ⇒ jump table (§3.5)
  → HAL_Delay(500)                      [BLOCKING — 500ms busy-wait inside what may be an I2C-write-triggered call, §2.7.1]
  → HAL_NVIC_SystemReset()              — TERMINAL, never returns

jump2iap()                              entry.c:159   — TERMINAL, never returns (software jump, see 03_execution_model.md §5)
  called from exactly one site: km_i2c.c:692, guarded by DeInit() at :691
```

**Ý nghĩa:** hai trong ba hàm "power" này là **terminal** — call graph phải
đánh dấu rõ ràng rằng `s800_power_off()` và `jump2iap()` không quay trở lại
caller của chúng, điều này quan trọng với bất kỳ ai đang suy luận về "cái gì
chạy sau `io_extend_write()`": đôi khi câu trả lời là "không gì cả, mãi mãi,
cho lần boot này" nếu lần ghi I2C cụ thể đó vô tình là lần trigger vào
boot-notify-storm guard dẫn tới `s800_power_off()`.

### 2.10 IAP image — đường flash reprogramming (main-loop context)

```
ENTRY  i2c_recv_wait()                  km_i2c_iap.c:240   (IAP's own copy, same rx_comp/change_state shape as Main, §2.5)
  case IAP_PG_COMMAND write, magic match:
    → main_project_flash_erase()        km_i2c_iap.c:435
        for each sector in MAIN_APPLICATION_SIZE/sector_size:
          ⇢ Driver_Flash0.EraseSector(addr)   — indirect, ARM_DRIVER_FLASH struct member (§3.4)
  case IAP_PG_COMMAND_DATA write (repeated, 4 bytes at a time):
    → flash_write_32(g_dest_addr, &w_data)   km_i2c_iap.c:413
        range-checks against [MAIN_APPLICATION_ADDRESS, +MAIN_APPLICATION_SIZE)
        ⇢ Driver_Flash0.ProgramData(addr, data, 2)   — indirect
  case IAP_PG_COMMAND_CHANGE_INFO write, completion bit set:
    → check_sum_calc(data)              km_i2c_iap.c:456   (compares master's checksum against a freshly recomputed one)
    → g_checksum_value = checksum_calc()          km_i2c.c:771 (shared with Main — same function, now checksumming the freshly-written image)
    → GetVersionString()                km_i2c.c:800        (re-reads the version string out of the just-flashed image)
RETURN
```

**Ý nghĩa:** đây là call path duy nhất trong toàn bộ firmware nơi các thao
tác erase/program flash chạy trong khi interrupt vẫn đang bật và CPU sắp
fetch instruction *tiếp theo* của chính nó từ đúng mảng flash đang bị
erase/program — trên chip single-bank này, flash controller stall việc CPU
fetch trong suốt thời gian đó (một đặc tính phần cứng, không phải bảo đảm của
phần mềm), nên trong thực tế điều này hoạt động giống như một lệnh gọi
blocking dù không có critical section phần mềm nào bọc quanh nó.
`check_sum_calc`/`checksum_calc`/`GetVersionString` được dùng chung nguyên
văn giữa Main và IAP (cùng một hàm, `km_i2c.c`, được link vào cả hai image)
có nghĩa là một bug được sửa ở bản này phải được nhớ để kiểm tra lại ở bản
build kia.

### 2.11 IAP image — MSW_ON interrupt → quay lại Main (ISR context)

```
⚡ MSW_ON pin edge
  → EXTI4_15_IRQHandler                 iap/Src/stm32f0xx_it.c:130
  → HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5)
  ⇢ HAL_GPIO_EXTI_Callback → signal_event(pin, EXT_INTR)   = km_exti_callback_iap()   km_extend_io_iap.c:53
      → msw_on_proc_iap()                km_extend_io_iap.c:28
          if (!MSW_ON) && (g_iap_reg[...CHANGE_INFO] has COMP bit):
              → HAL_NVIC_SystemReset()   — TERMINAL, fired from ISR context, not main-loop
```

**Ý nghĩa:** không giống mọi lệnh gọi "terminal" khác trong firmware này
(`s800_power_off`, `jump2iap`), lệnh này được bắn ra **từ bên trong một ISR**,
không phải từ main loop — super-loop của IAP có thể đơn giản là ngừng tồn
tại giữa chừng một lệnh `WDT_Refresh()` mà không có unwind, ngay khi EXTI này
fire. `03_execution_model.md` §5 đã đề cập tính bất đối xứng này (nhảy bằng
phần mềm theo một chiều, reset bằng phần cứng theo chiều kia); call graph
này bổ sung thêm rằng chiều reset cụ thể bắt nguồn từ interrupt context.

---

## 3. Function pointer, đăng ký callback, và indirect dispatch — danh mục đầy đủ

Firmware này không có virtual dispatch (C, không phải C++) và không có RTOS
callback, nhưng nó có một số lượng đáng ngạc nhiên các cơ chế indirect-call
ở cấp C. Mọi cơ chế được tìm thấy trong `main/` và `iap/` đều được liệt kê ở
đây; §2 đã cho thấy một vài trong số chúng trong bối cảnh thực tế.

### 3.1 `BSP_GPIO_SignalEvent_t` — một slot toàn cục, EXTI fan-in
- Type: `void (*)(uint16_t pin, GPIOEventType event)` (implied by usage; declared in `BSP_STM32F03x_Nucleo.h`).
- Storage: single static `signal_event` in `BSP_GPIO_STM32F03x_Nucleo.c:10`.
- Registered by: `BSP_GPIO_Initialize(handler)` (`:87-91`) — called once by `km_it_init()` (Main, `km_it.c:367`) or `km_it_init_iap()` (IAP, `km_extend_io_iap.c:80`).
- Called from: `HAL_GPIO_EXTI_Callback()` (`:71`), itself invoked from every EXTI ISR that fires (§2.3).
- Bound value: `km_exti_callback` (Main) or `km_exti_callback_iap` (IAP) — **hai image bind hai hàm khác nhau vào cùng một slot**; chỉ có đúng một mức độ gián tiếp (indirection), không phải dispatch theo từng pin riêng — chính hàm được bind mới làm việc `switch` theo pin.

### 3.2 `BSP_TIM_SignalEvent_t` — mảng slot cho từng timer instance
- Type: `void (*)(TimerType type, TimerEventType event)`.
- Storage: `Resource[].SignalHandler` array in `BSP_TIM_STM32F03x_Nucleo.c:34-62`, one slot per compiled-in timer instance (only TIM3's slot is ever populated in this hardware config — the `STM32F030x8` `#ifdef` block for TIM14/15/16/17 is inactive on the actual F031C6 part).
- Registered by: `BSP_TIM_Initialize(TIMER_3, handler)` (`:98-101`) — called once by `km_it_init()` (Main only; IAP never calls this, §4.3 of `03_execution_model.md`).
- Called from: `HAL_TIM_PeriodElapsedCallback()` (`:72-88`), which linearly scans `Resource[]` for a `Handle` match before dereferencing `SignalHandler` — so this is a *searched* indirect call, not a direct array index.
- Bound value: `km_tim_callback` (Main only).

### 3.3 `ARM_I2C_SignalEvent_t` — callback hoàn tất của I2C
- Type: `void (*)(uint32_t event)`.
- Storage: `I2C_Resources.Cb_event`, one per CMSIS resource struct (`I2C_stm32f0xx.c:59`); only `I2C0_Resources` is ever wired to a real handle (`&hi2c1`), `I2C1_Resources.Handle` is `NULL` (`:83-95`) — nửa cơ chế này thuộc về `Driver_I2C1` hoàn toàn "chết" (inert).
- Registered by: `Driver_I2C0.Initialize(rx_comp)` — called from `i2c_recv_first()` (§2.1, both images) and again from `i2c_sw_reset()` (§2.8).
- Called from: `do_callback()` (`:692-708`) and directly from `HAL_I2C_ErrorCallback()` (`:678-680`) — both HAL weak-callback overrides reached via the I2C1 ISR (§2.5).
- Bound value: `rx_comp` — **một hàm khác nhau cho mỗi image** (`km_i2c.c:171` for Main, `km_i2c_iap.c:90` for IAP) mặc dù call site đăng ký (`i2c_recv_first()`) trông giống hệt nhau ở cả hai image — hàm `rx_comp` riêng của mỗi image mới thực sự được bind, vì mỗi image link translation unit của riêng nó.

### 3.4 `ARM_DRIVER_I2C` / `ARM_DRIVER_FLASH` — struct-of-function-pointers ("C vtable")
- `Driver_I2C0` (`I2C_stm32f0xx.c:777-790`) và `Driver_I2C1` (`:792-805`, unused) là các struct dạng `const` với cả 12 thành viên đều là function pointer (`Initialize`, `SlaveTransmit`, `SlaveReceive`, `Control`, `GetStatus`, …) — đây là pattern CMSIS-Driver, về cấu trúc giống hệt một vtable của C++ nhưng được xây bằng tay dưới dạng static initializer list.
- Mỗi lệnh gọi kiểu `Driver_I2C0.SlaveTransmit(...)` trong `km_i2c.c`/`km_i2c_iap.c` do đó là một lệnh gọi gián tiếp qua struct này, chứ không phải gọi trực tiếp đến `I2C0_SlaveTransmit` mặc dù đó là hàm duy nhất có thể nằm sau nó trong binary đã đóng gói (không có việc re-wiring động nào của struct xảy ra ở bất cứ đâu).
- `change_state()` (§2.6/§3.6 dưới đây) đi thêm một bước nữa và nhận **các thành viên của struct này làm chính tham số con trỏ hàm của nó** (`slave_transfer_t transfer`, `control_t control`), nên tính gián tiếp này được truyền đi, chứ không chỉ được gọi tại chỗ.
- `Driver_Flash0` (declared `extern ARM_DRIVER_FLASH Driver_Flash0`, `km_i2c_iap.c:24`, defined in `common/Drivers/CMSIS/.../FLASH_stm32f0xx.c` — chưa được đọc lại trong lần phase này) là cùng pattern đó cho erase/program flash, chỉ được IAP dùng (§2.10).

### 3.5 Jump table chỉ mục theo model — `mc_p_on_proc()` / `sleep_status_rem_proc()`
```c
// km_extend_io.c:841 and :1057, same shape both places
static void (*const func[])(void) = { dummy, dummy, mc_p_on_sparrow_proc, mc_p_on_eagle_proc };
func[(int)model]();
```
- Đây là các jump table function-pointer đúng nghĩa, không phải câu lệnh
  `switch` — `(int)model` index thẳng vào một mảng `static const` cục bộ
  gồm 4 con trỏ hàm và gọi qua nó.
- Ánh xạ index đến từ `TYPE_Model` (`km_extend_io.h:6-12`):
  `RESERVED0=0`, `RESERVED1=1` (cả hai đều → `dummy()`, một no-op đúng
  nghĩa, `km_extend_io.c:731-733`), `Sparrow=2`, `Eagle=3`.
- Cả hai bảng đều được điền và index giống hệt nhau, nhưng là **hai mảng
  cục bộ riêng biệt** (không phải một bảng dùng chung) — một hazard bảo trì:
  thêm model thứ 5 đòi hỏi cập nhật cả hai literal `func[]` đồng bộ với
  nhau, và không có gì trong hệ thống kiểu (type system) buộc điều đó.
- Được gọi từ main loop mỗi lần lặp với `model` cố định ở giá trị
  `get_model()` đã cache (§2.1) — nên trong thực tế chỉ một trong bốn
  target từng được chọn trong suốt vòng đời của một lần boot.

### 3.6 `change_state()` — function pointer được truyền như tham số gọi hàm
Đã được trình bày chi tiết ở §2.6; bản thân cơ chế: `km_i2c.c:274`
```c
static void inline change_state(int next_state, int* comp,
    slave_transfer_t transfer,   // called as (*transfer)(buf, size) at :301
    uint8_t* buf, uint16_t size,
    control_t control, int is_manual_ack)   // called as control(...) at :290/:298
```
Mỗi call site trong số ~15 call site của `i2c_recv_wait()` (Main và IAP)
truyền một tổ hợp *khác nhau* của `Driver_I2C0.SlaveTransmit`/
`SlaveReceive` và `Driver_I2C0.Control`/`NULL` — cùng một thân hàm cuối cùng
lại thực hiện một lệnh gọi gián tiếp khác nhau tùy thuộc hoàn toàn vào tổ
hợp tham số caller chọn, đây là thứ gần nhất với dispatch kiểu
strategy-pattern trong codebase này.

### 3.7 `anti_chattering_info[].intr_proc` — callback tùy chọn theo từng entry
```c
// km_it.c:20-54
{ ..., msw_on_proc,          ... },   /* MSW entry           */
{ ..., power_monitor_proc,   ... },   /* POWER_MONITOR entry */
{ ..., NULL,                 ... },   /* 24V11_MONI entry — deliberately no callback */
```
Được gọi tại `km_extend_io.c:1460`: `info->intr_proc(STATE_INT)`, được bảo
vệ bởi một NULL check trên cùng dòng — entry bảng debounce 24V11 tồn tại
đơn thuần để drive timer `TYPE_Km_Timer_Anti_Chattering_24V` và phơi bày
state `IS_CHECKING_CHATTERING()` cho code khác (ví dụ, điều kiện macro của
`_MC_P_ON_OK()`, `km_extend_io.c:739`); nó không bao giờ tự mình trigger
một callback. Đây là một bảng function-pointer *có một lỗ hổng cố ý*, không
phải một sơ suất — đáng để phân biệt với `i2c_reset()` (§2.8) vốn thực sự
chết (dead).

### 3.8 Weak-symbol override — indirect dispatch tại link-time
Không phải là một function pointer runtime, nhưng là một dạng indirection
thực sự đáng nêu tên: mọi hàm `HAL_*_Callback`/`HAL_*_MspInit` được khai báo
`__weak` trong HAL của ST và được thay thế một cách âm thầm tại **link
time** bởi bất kỳ translation unit nào định nghĩa một strong symbol cùng
tên. Firmware này dựa nhiều vào cơ chế đó — `HAL_GPIO_EXTI_Callback`,
`HAL_TIM_PeriodElapsedCallback`, `HAL_I2C_*Callback`,
`HAL_RTC_AlarmAEventCallback`, `HAL_*_MspInit/DeInit` đều được override theo
cách này (§2.3-§2.5, `03_execution_model.md` §4). "Call graph" ở đây không
có call site tường minh nào để grep — linker giải quyết nó — đây chính xác
là kiểu cạnh (edge) mà một text search ngây thơ sẽ bỏ lỡ và một call graph
sinh tự động bằng công cụ thường sẽ hiểu sai (nó có thể hiển thị *stub*
weak rỗng như là target thay vì override thực sự, tùy vào thứ tự link mà
công cụ giả định).

---

## 4. Bản ghi theo từng hàm (chỉ các node then chốt)

### `main()` (Main)
- File: `main/App/entry.c:46`
- Caller: `Reset_Handler` (via `__main`)
- Callee: `MX_Init`, `get_model`, `checksum_calc`, `GetVersionString`, `BSP_WDT_Start`, `internal_sts_init`, `km_it_init`, `BSP_TIM_Start`, `adc_moni_5v_start_proc`(dead), `io_extend_memory_write`, `s800_power_on`, `i2c_recv_first`, `poweroff_timer_init`, then the full loop-body list (§2.2)
- Execution context: startup → super-loop (never returns)
- Purpose: khởi động hệ thống rồi dispatcher polling vĩnh viễn
- Input: none (no argv-equivalent)
- Output: none (return value unused, `int32_t` is vestigial)
- Side effects: configure mọi peripheral; assert `/RESET` cho SoC S800; bắt đầu I2C slave listening
- Shared state written: hầu như mọi biến static cấp module đều nhận lần ghi đầu tiên của chúng ở đây (initialization)

### `km_exti_callback()` (Main)
- File: `main/App/km_it.c:247`
- Caller: `HAL_GPIO_EXTI_Callback` (weak override, §3.8) ⇐ any of 3 EXTI ISRs
- Callee: `set_pending_factor_bit`, `start_anti_chattering`, `SystemClock_Config2`, `HAL_RTC_DeactivateAlarm`
- Execution context: **ISR** (EXTI0_1/EXTI2_3/EXTI4_15, priority 0,0)
- Purpose: triage cấp một cho 5 tín hiệu đầu vào vật lý
- Input: `PinType GPIO_Pin`, `GPIOEventType event`
- Output: none
- Side effects: mutate `pending_factor` (via helper) hoặc `anti_chattering_info[]`; có thể cấu hình lại PLL
- Shared state: `pending_factor` (km_it.c:17), `anti_chattering_info[]` (km_it.c:20-54), `stop_mode` (km_alarm_wake.c:16) — đều cũng được chạm tới từ main-loop context

### `km_tim_callback()` (Main)
- File: `main/App/km_it.c:312`
- Caller: `HAL_TIM_PeriodElapsedCallback` (weak override, §3.2) ⇐ TIM3 ISR
- Callee: none (pure arithmetic on module state)
- Execution context: **ISR** (TIM3, priority 0,0), 1 ms period
- Purpose: nguồn tick software-timer duy nhất của hệ thống
- Input: `TimerType type`, `TimerEventType event` (only `TIMER_3`/`PERIOD_ELAPSED` handled)
- Output: none
- Side effects: `ui_EventCountTimer++`; decrement mỗi slot `g_km_timer[]`, chuyển các slot hết hạn sang `TYPE_Km_Timer_End`
- Shared state: `g_km_timer[TYPE_Km_Timer_Kind_Max]` (km_it.c:18) — được đọc bởi ~10 hàm main-loop qua `km_timer_get_state()`

### `rx_comp()` (Main)
- File: `main/App/km_i2c.c:171`
- Caller: `do_callback()`/`HAL_I2C_ErrorCallback()` (weak overrides, §3.3) ⇐ I2C1 ISR
- Callee: `Driver_I2C0.Control` (indirect), `Driver_I2C0.GetDataCount`, `Driver_I2C0.GetStatus`
- Execution context: **ISR** (I2C1, priority 0,0)
- Purpose: ACK/NACK byte địa chỉ đến, latch các flag transfer-complete/error — không gì hơn
- Input: `uint32_t event` (ARM_I2C_EVENT_* bitmask)
- Output: none
- Side effects: `i2c_info.{read_comp,write_comp,error_code,rx_during,tx_during,rx_tick_last,tx_tick_last}`
- Shared state: `i2c_info` struct (km_i2c.c:33-47) — toàn bộ bề mặt handoff Main↔ISR cho I2C

### `i2c_recv_wait()` (Main)
- File: `main/App/km_i2c.c:485`
- Caller: `main()` loop body, every iteration (`entry.c:126`)
- Callee: `i2c_check_error`, `i2c_sw_reset`, `io_extend_read/write`, `change_state` (⇢ `Driver_I2C0.*`), `DeInit`+`jump2iap`, `km_timer_set/get_state`, `setEventRecord`
- Execution context: main-loop (super-loop body)
- Purpose: toàn bộ giao thức register-map của I2C — decode `second_addr`, đọc hoặc ghi tài nguyên được địa chỉ hóa, re-arm driver cho byte tiếp theo
- Input: none directly — reads `i2c_info` (set by `rx_comp` in ISR context)
- Output: none directly — writes into `tx_data`/RTC memory/`g_io_extend_memory` depending on command
- Side effects: có thể trigger `jump2iap()` (terminal), `i2c_sw_reset()`, ghi GPIO tùy ý qua `io_extend_write`
- Shared state: `i2c_info`, `g_io_extend_memory[]`, `g_checksum_value`, `g_checki2c_value`, các register memory-mapped của RTC

### `change_state()` (Main + IAP, cùng cấu trúc)
- File: `main/App/km_i2c.c:274` (IAP: `km_i2c_iap.c`, không được đọc lại riêng — cùng pattern source theo ghi chú Phase 3)
- Caller: `i2c_recv_wait()` — ~15 call sites
- Callee: `(*transfer)(buf,size)`, `control(...)` — **cả hai đều là tham số con trỏ hàm** (§3.6)
- Execution context: main-loop (called only from `i2c_recv_wait`, never from ISR)
- Purpose: điểm nghẽn (choke point) duy nhất cho việc "tiến state machine I2C và re-arm driver"
- Input: next state, con trỏ completion-flag, các con trỏ hàm transfer/control, buffer, size, cờ manual-ack
- Output: none
- Side effects: cặp `HAL_NVIC_DisableIRQ/EnableIRQ(I2C1_IRQn)` bao quanh việc mutate state + lệnh gọi gián tiếp; `i2c_info.state`, `*comp`, `i2c_info.rx_buf_current`
- Shared state: `i2c_info` (same struct as `rx_comp`)

### `io_extend_read()` / `io_extend_write()` (Main)
- File: `main/App/km_extend_io.c:192` / `:249`
- Caller: `i2c_recv_wait()` (I2C-driven), plus `io_extend_memory_write()` calls neither (it writes `g_io_extend_memory` directly from live pins instead)
- Callee: `change_cmd2array` (table lookup, not a jump table — a `switch`), `BSP_GPIO_ReadPin/WritePin`, and (write only) `erp_sensor_proc`, `poweroff_factor_save`, `s800_power_off`
- Execution context: chỉ main-loop (never ISR)
- Purpose: toàn bộ abstraction GPIO register-mapped mà I2C master bên ngoài nhìn thấy — 14 pin vật lý gói gọn vào một vài "extend" register 32-bit
- Input: `uint8_t addr` (I2C second-address byte), and for write, `uint32_t data`
- Output (read only): `uint32_t` register value
- Side effects (write only): `g_io_extend_memory[]`, các pin GPIO vật lý (một số pin bị loại trừ tường minh khỏi việc ghi trực tiếp — xem §2.7.1), có thể dẫn đến `s800_power_off()`
- Shared state: `g_io_extend_memory[TYPE_Io_Extend_Max]` (km_extend_io.c:67) — được đọc bởi gần như mọi hàm `*_proc()` trong file

### `anti_chattering_proc()` (Main)
- File: `main/App/km_extend_io.c:1438`
- Caller: `main()` loop body (`entry.c:118`)
- Callee: `get_anti_chattering_info`, `BSP_GPIO_ReadPin`, `start_anti_chattering`, `km_timer_get_state/set`, `info->intr_proc(...)` (indirect, §3.7)
- Execution context: main-loop
- Purpose: giải phóng (drain) state debounce đã được arm bởi `km_exti_callback()` trong ISR context; fire callback deferred một khi tín hiệu đã giữ ổn định đủ thời gian cấu hình
- Input: none (iterates the fixed 3-entry `anti_chattering_info[]`)
- Output: none
- Side effects: mutate `anti_chattering_info[].{status,prev_level}`; có điều kiện gọi `msw_on_proc`/`power_monitor_proc`
- Shared state: `anti_chattering_info[]` (km_it.c:20-54) — được ghi từ cả hàm này lẫn `km_exti_callback()`/`start_anti_chattering()` trong ISR context

### `s800_power_off()` (Main)
- File: `main/App/km_extend_io.c:523`
- Caller: `msw_on_proc()`, `power_flicker_failsafe()`, `io_extend_write()` (3 distinct call sites, §2.9)
- Callee: `set_ca72_status`, `poweroff_timer_init`, `clear_pending_factor_bit`, raw `NVIC_DisableIRQ`×3, `extend_output_init`, `get_model`, `sleep_status_rem_proc` (⇒ jump table), `HAL_Delay`, `HAL_NVIC_SystemReset`
- Execution context: main-loop (reachable transitively from an I2C write, §2.7.1, but never from an ISR directly)
- Purpose: tắt nguồn SoC S800 một cách trật tự, theo sau bởi một lần reset toàn bộ PS-CPU
- Input: none
- Output: none — **terminal**, không trả về
- Side effects: disable trực tiếp 3 đường NVIC (bỏ qua wrapper reference-counted — đã được nêu trong `02_architecture.md` và `03_execution_model.md`); delay blocking 500 ms; reset phần cứng
- Shared state: `g_first_power_on`, `g_io_extend_memory[TYPE_Internal_Sts0]`, `_status` (via `set_ca72_status`)

### `jump2iap()` (Main)
- File: `main/App/entry.c:159`
- Caller: exactly one site, `km_i2c.c:692`, gated by magic-number check
- Callee: none (raw pointer load + branch)
- Execution context: main-loop
- Purpose: chuyển giao thực thi cho IAP image mà không cần reset phần cứng
- Input: none (reads `IAP_DEFAULT_ADD` = `0x08005000`, a compile-time constant)
- Output: none — **terminal**
- Side effects: ghi đè `MSP`; bỏ lại C stack/global của Main tại chỗ (không bị zero, chỉ là không còn được tham chiếu tới nữa)
- Shared state: none after the jump — đây chính xác là ranh giới nơi "shared state" ngừng còn là một khái niệm có ý nghĩa cho đến lần reset tiếp theo

### `main()` (IAP) / `i2c_recv_wait()` (IAP) / `flash_write_32()` / `main_project_flash_erase()`
- Files: `iap/App/entry_iap.c:39`, `iap/App/km_i2c_iap.c:240`, `:413`, `:435`
- Caller chain: `Reset_Handler`/software-jump-in → `main()` → loop → `i2c_recv_wait()` → (on `IAP_PG_COMMAND`/`IAP_PG_COMMAND_DATA`) → `flash_write_32`/`main_project_flash_erase`
- Callee: `Driver_Flash0.EraseSector`/`ProgramData` (indirect, §3.4)
- Execution context: main-loop (IAP image only)
- Purpose: toàn bộ cơ chế cập nhật firmware cho Main image
- Input: I2C-supplied address/data bytes, buffered in `i2c_info.rx_buf`
- Output: `int32_t` success/`ARM_DRIVER_ERROR` (propagated into `g_iap_reg[]` status bits, read back by the external master over I2C)
- Side effects: erase và reprogram `0x08000000`-`0x08004FFF` — toàn bộ Main image — trong khi bản thân IAP vẫn tiếp tục chạy từ `0x08005000`+
- Shared state: `g_iap_reg[]`, `g_dest_addr`, `g_checksum_value`/`g_main_version` (cả hai đều dùng chung nguyên văn với `km_i2c.c` của Main)

### `msw_on_proc_iap()` (IAP)
- File: `iap/App/km_extend_io_iap.c:28`
- Caller: `km_exti_callback_iap()` ⇐ EXTI4_15 ISR (MSW_ON pin)
- Callee: `BSP_GPIO_ReadPin`, `HAL_NVIC_SystemReset`
- Execution context: **ISR** (EXTI4_15, priority 0,0) — lưu ý đây là logic tương đương main-loop chạy trực tiếp trong interrupt context, khác với `msw_on_proc()` của Main vốn là một hàm thân loop
- Purpose: phát hiện "MSW đã nhả + cập nhật firmware hoàn tất" và trả quyền điều khiển lại cho Main thông qua reset
- Input: none (reads live pin + `g_iap_reg[]`)
- Output: none — có điều kiện là **terminal**
- Side effects: reset phần cứng toàn bộ MCU
- Shared state: `g_iap_reg[IAP_REG_INDEX_CALC(IAP_PG_COMMAND_CHANGE_INFO)]` — được ghi bởi `i2c_recv_wait()` trong main-loop context, được đọc ở đây trong ISR context mà không có đồng bộ hóa (một lần đọc 32-bit aligned trên Cortex-M0 là atomic theo cấu tạo, nên `[INFERRED]` an toàn trong thực tế, nhưng đây là một lần đọc cross-context không đồng bộ hóa thực sự)

---

## 5. Các quan sát xuyên suốt (chỉ nhìn thấy được từ đồ thị, không từ bất kỳ hàm đơn lẻ nào)

1. **Ba idiom "critical section" độc lập cùng tồn tại chỉ riêng cho I2C1**:
   `i2c_check_error()` (`km_i2c.c:392,401`), `change_state()`
   (`km_i2c.c:282,302`), và các lệnh gọi HAL bên dưới của `i2c_sw_reset()`
   đều bọc trực tiếp việc disable/enable `I2C1_IRQn` thay vì dùng chung một
   hàm wrapper — về mặt chức năng thì nhất quán (đều nhắm vào cùng một IRQ)
   nhưng là ba bản sao viết tay riêng biệt của cùng một idiom 2 dòng.
2. **`msw_on_proc()` tồn tại hai lần, với các đảm bảo context khác nhau**:
   Phiên bản của Main (`km_extend_io.c:1149`) là một hàm thân loop được gọi
   với `STATE_NORMAL` (mỗi lần lặp) hoặc `STATE_INT` (qua bảng function-pointer
   `intr_proc`, §3.7, tự nó lại từ một hàm main-loop). Phiên bản của IAP,
   `msw_on_proc_iap()`, chạy **trực tiếp trong ISR context** mà không có giai
   đoạn debounce nào cả. Cùng tên, cùng khái niệm tín hiệu, nhưng đảm bảo
   execution-context lại khác nhau về cấu trúc giữa hai image — một cái bẫy
   cho bất kỳ ai port logic từ image này sang image kia chỉ dựa vào việc
   khớp tên.
3. **`checksum_calc`/`GetVersionString` là source dùng chung, đa mục đích ở
   runtime**: các hàm giống hệt nhau (`km_i2c.c:771,800`) được link vào cả
   hai image, nhưng Main gọi chúng một lần lúc boot để mô tả *chính nó*,
   trong khi IAP gọi chúng sau một lần ghi flash để xác minh *image mà nó
   vừa ghi* — bản thân code không hề biết ý nghĩa nào trong hai ý nghĩa đó
   áp dụng tại một call site cho trước; chỉ có context của caller (§2.1 so
   với §2.10) mới cung cấp ý nghĩa đó.
4. **Mảng debounce-timer và mảng software-timer là cùng một mảng**
   (`g_km_timer[]`): `start_anti_chattering()` (ISR, §2.3) và
   `poweroff_timer_start()`/`km_timer_set()` (main-loop, nhiều call site)
   đều ghi vào đúng một cấu trúc mà `km_tim_callback()` (ISR, §2.4)
   decrement. Có đúng một mảnh shared mutable state nằm bên dưới mọi quyết
   định dựa-trên-thời-gian trong Main image.
