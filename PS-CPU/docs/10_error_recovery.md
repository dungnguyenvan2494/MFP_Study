 # PS-CPU (pscpu_s800) — 10. Lỗi & Khôi phục

**Phạm vi của tài liệu này:** mọi đường lỗi được phát hiện (detected-error
path) trong firmware, từ lúc phát hiện, qua xử lý (handling), cho đến bất
kỳ sự khôi phục nào theo sau (hoặc không có), cùng với cơ chế ghi log chẩn
đoán bền vững (`poweroff_factor_save()`) mà phần lớn các đường này đổ vào.
Tài liệu đồng hành với `06_event_flow.md` (nơi đã giới thiệu một số đường
này dưới dạng các event E26-E30) và `09_timing.md` (các giá trị timeout
kích hoạt một số trong đó) — tài liệu này tổ chức lại nội dung đó theo
đúng khuôn dạng NORMAL→ERROR→DETECTION→HANDLING→RECOVERY→NEXT-STATE mà
Phase 10 yêu cầu, và bổ sung một phát hiện chưa từng được nêu ra trước
đây: một lệnh đọc lại chẩn đoán (diagnostic-readback) thực chất là một
stub chết (dead stub), không hề kết nối với log mà nó có vẻ như đang đọc.
Các nhãn bằng chứng theo `SESSION_CONTEXT.md`.

---

## 1. Log chẩn đoán bền vững — `poweroff_factor_save()`

Trước khi truy vết từng đường lỗi riêng lẻ, đây là cơ chế mà gần như tất
cả chúng đều đổ vào: `poweroff_factor_save(factor_bit)`
(`km_extend_io.c:144-154`) tích luỹ theo kiểu OR một bit vào một thanh ghi
RTC được nuôi bằng pin (battery-backed), `B_REGISTER_PWROFF_LOG`
(`RTC_BKP3R_CMD_2`, `km_i2c.h:60`), dưới `RTC_WRITEPROTECT_DISABLE/ENABLE`.
**Cả 9 bit factor được khai báo đều thực sự được sử dụng** (xác nhận bằng
grep — không giống hai loại software-timer chết được tìm thấy ở
`09_timing.md` §3, không có gì ở đây là tàn dư (vestigial)):

| Bit | Value | Meaning | Set by | File:line |
|---|---|---|---|---|
| `PWROFF_FACTOR_TMOUT_LIMIT` | 0x0001 | vượt quá thời gian tối đa của trình tự power-off | `poweroff_timer_proc()` | `km_extend_io.c:1322,1334` |
| `PWROFF_FACTOR_TMOUT_WDG` | 0x0002 | vượt quá WDG-time của power-off | `poweroff_timer_proc()` | `km_extend_io.c:1329,1334` |
| `PWROFF_FACTOR_HRESET_REQ` | 0x0004 | `/HRESET_REQ` được assert | `hreset_req_proc()` | `km_extend_io.c:494` |
| `PWROFF_FACTOR_OFFSEQ_ON` | 0x0008 | nguồn chính được assert lại trong khi off-sequence vẫn đang chạy | `msw_on_proc()` | `km_extend_io.c:1165` |
| `PWROFF_FACTOR_BOOTING_OFF` | 0x0010 | nguồn chính bị ngắt trong khi S800 vẫn đang boot | `msw_on_proc()` | `km_extend_io.c:1186` |
| `PWROFF_FACTOR_BACKUP_ON` | 0x0020 | nguồn chính được assert lại trong cửa sổ chờ flicker-backup | `backupwait_timer_proc()` | `km_extend_io.c:1357` |
| `PWROFF_FACTOR_FLICKER` | 0x0040 | một sụt áp power-monitor ngắn (flicker) đã kích hoạt reboot | `power_monitor_proc()` / `hreset_req_proc()` | `km_extend_io.c:455,492` |
| `PWROFF_FACTOR_FLICKER_FAILSAFE` | 0x0080 | chính timer failsafe của flicker đã hết hạn | `power_flicker_failsafe()` | `km_extend_io.c:177` |
| `PWROFF_FACTOR_BOOT_SEQ_ERROR` | 0x0100 | boot-notify nhận được hai lần (E29, `06_event_flow.md`) | `io_extend_write()` | `km_extend_io.c:320` |

Bitmask này được tích luỹ theo kiểu OR, không bao giờ bị clear bởi
application code ở bất kỳ file nào đã đọc trong lượt này —
`[INFERRED]` nó chỉ reset khi chính domain backup của RTC mất nguồn (một
sự mất nguồn sâu hơn bất kỳ nguyên nhân nào trong 9 nguyên nhân này biểu
thị riêng lẻ), nghĩa là nó có thể mang theo các nguyên nhân từ nhiều chu
kỳ nguồn trước đó cho đến khi có thứ gì đó/ai đó clear nó một cách tường
minh (không tìm thấy lời gọi clear nào).

### 1.1 Cái bẫy đọc-lại — hai lệnh "đọc log power-off", chỉ một trong hai hoạt động
Hai lệnh I2C, xét theo tên, trông như đều truy xuất log này:
- **`B_REGISTER_PWROFF_LOG`** (một địa chỉ "B-register" tổng quát của RTC,
  `km_i2c.h:60`) — được định tuyến qua đường đọc *tổng quát* passthrough
  của RTC (`km_i2c.c:550-554`), đường này thực sự trả về
  `*(RTC_BASE + B_REGISTER_PWROFF_LOG)`, tức là bitmask tích luỹ thực tế.
  **Đây là lệnh duy nhất trả về dữ liệu thật.**
- **`EXTEND_PWROFF_TIMEOUTLOG_GET_CMD`** — một lệnh có tên khác, trông như
  được xây dựng riêng cho mục đích này, nhưng handler của nó lại là một
  **stub được hardcode**:
  ```c
  // km_i2c.c:605-609
  case EXTEND_PWROFF_TIMEOUTLOG_GET_CMD:
      /* 電源OFFタイムアウトログを取得処理 */
      tx_data = 0;  /* エラーなしを返却する。古いバージョンのPMUドライバの可能性がある為、2ndアドレスを残す*/
      change_state(TYPE_State_Wait_Read, ...);
      break;
  ```
  Nó **luôn luôn trả về 0** ("không có lỗi"), bất kể `poweroff_factor_save()`
  thực sự đã tích luỹ những gì. Chú thích trong code giải thích lý do: nó
  được giữ lại chỉ để một phiên bản PMU-driver bên ngoài cũ hơn, vốn kỳ
  vọng *một phản hồi nào đó* tại địa chỉ này, không bị NACK — nó chưa bao
  giờ được nối vào log thật. Lệnh ghi tương ứng
  `EXTEND_PWROFF_TIMEOUTLOG_SET_CMD` cũng là cùng loại stub
  (`km_i2c.c:706-708`, "không xử lý gì, giữ địa chỉ lại để tương thích").

**Ý nghĩa:** bất kỳ ai debug một lỗi thực địa (field failure) bằng cách
polling `EXTEND_PWROFF_TIMEOUTLOG_GET_CMD` — lệnh có tên gợi ý trực tiếp
nhất "đọc nguyên nhân power-off" — sẽ đọc được `0` (bị hiểu lầm là "không
có lỗi xảy ra") ở mọi lần truy vấn, ngay cả khi `poweroff_factor_save()`
đã thực sự ghi nhận một timeout, một flicker, hay một lỗi trình tự boot.
Dữ liệu thật chỉ có thể tiếp cận được qua địa chỉ passthrough B-register
của RTC, dưới một cái tên (`B_REGISTER_PWROFF_LOG`) không hề gợi ý rằng đó
là log chẩn đoán power-off, trừ khi người đọc đã biết trước để tìm nó.
Đây là một cái bẫy công cụ chẩn đoán cụ thể, đã được xác nhận từ source,
không phải một giả thuyết.

---

## 2. Truy vết các đường lỗi

### 2.1 Lỗi bus I2C / timeout / protocol mismatch
```
NORMAL: i2c_recv_wait() draining completion flags every super-loop pass
ERROR: BERR/ARLO/OVR latched by rx_comp() (ISR, km_i2c.c:251-253); OR busy/rx/tx held past threshold; OR read/write completion arrives in the wrong protocol state
DETECTION: i2c_check_error() (main-loop, km_i2c.c:379-461) — thresholds in 09_timing.md §4; protocol-mismatch checks directly in i2c_recv_wait() (km_i2c.c:739-742,746-748)
HANDLING: i2c_sw_reset() (km_i2c.c:356-364)
RECOVERY: force-reset I2C1 peripheral clock, HAL_Delay(2ms), MX_I2C1_Init(), i2c_recv_first() — full re-arm
NEXT STATE: i2c_info.state = TYPE_State_Wait_Write (07_state_machines.md §1.1) — no retry counter, no backoff, no distinction between error causes in the recovery action taken
```
Cũng có thể được kích hoạt từ bên ngoài theo yêu cầu: I2C master có thể
gửi `EXTEND_RESET_I2C` (`km_i2c.c:720-723`) để buộc thực hiện chính sự
khôi phục này mà không cần có lỗi thực sự xảy ra — về bản chất, master
luôn có sẵn một lệnh "reset phía protocol của tôi" bất kỳ lúc nào.

### 2.2 Lỗi khởi tạo HAL lúc boot
```
NORMAL: MX_Init() running through its ~13 HAL_*_Init calls
ERROR: any HAL_RCC_OscConfig / HAL_RCC_ClockConfig / HAL_RCCEx_PeriphCLKConfig / HAL_I2C_Init / HAL_I2CEx_ConfigAnalogFilter / RTC-init-family call returns != HAL_OK
DETECTION: the immediately-following `if (... != HAL_OK) { Error_Handler(); }` guard (13 sites, mx_init.c:132,142,150,216,223,238,259,292,301,376,394,400,407)
HANDLING: Error_Handler() (mx_init.c:529-537) — while(1){}
RECOVERY: NONE from software — and because MX_Init() runs before BSP_WDT_Start() (entry.c:52-63), the IWDG is not yet armed, so even the ~26.2s hardware watchdog cannot recover this hang (06_event_flow.md E28)
NEXT STATE: permanent hang until an external NRST/power-cycle
```
Hình dạng giống hệt trong IAP image (`iap/Src/mx_init.c:264-271`,
`entry_iap.c:44-51`).

### 2.3 Interrupt vector không sử dụng/không được xử lý
```
NORMAL: an interrupt line that has no application ISR override and is never NVIC-enabled (WWDG/PVD/FLASH/RCC/DMA×3/ADC1/TIM1×2/TIM2/TIM14/TIM16/TIM17/SPI1/USART1, 03_execution_model.md §6)
ERROR: that line fires anyway (only reachable if dead code were re-enabled without adding a handler — the one concretely identified latent path is the #if 0 ADC code, 03_execution_model.md §6)
DETECTION: none — the CPU simply vectors to Default_Handler
HANDLING: Default_Handler() (startup_stm32f031x6.s:165-216) — `B .`, infinite loop
RECOVERY: the IWDG — since this happens during normal operation (watchdog already armed), the ~26.2s timeout does eventually reset the MCU
NEXT STATE: full reset (equivalent to E1-E4, 06_event_flow.md §2) — cause is not recorded anywhere (no factor bit exists for "unexpected interrupt hang")
```

### 2.4 IWDG timeout (chính watchdog)
```
NORMAL: BSP_WDT_Refresh() called once per super-loop iteration (entry.c:89 / entry_iap.c:76)
ERROR: any hang longer than ~26.2s — a stuck Default_Handler()/Error_Handler() loop, an unexpectedly long blocking call, or a genuine infinite loop bug
DETECTION: hardware counter underflow — no software detection exists (06_event_flow.md §2, E4 is "silent")
HANDLING: hardware — MCU reset
RECOVERY: full cold-start (03_execution_model.md §2.1)
NEXT STATE: indistinguishable from POR/pin-reset/software-reset at the next boot (06_event_flow.md §2 — confirmed by exhaustive grep that no code reads RCC's reset-cause flags)
```

### 2.5 Power-monitor flicker (sụt áp ngắn)

**Correction (được phát hiện trong đợt kiểm toán pháp y Phase-18 —
Phase-18 forensic audit):** một phiên bản trước đó của phần này khẳng
định rằng `power_monitor_proc(STATE_INT)` được gọi trực tiếp từ switch
trong `km_exti_callback()`, ở ngữ cảnh ISR, bỏ qua cơ chế anti-chattering.
Điều đó là sai. Nhánh `POWER_MONITOR_Pin` trong `km_exti_callback()`
**chỉ** gọi `start_anti_chattering()` (`main/App/km_it.c:259-262`) — hoàn
toàn giống như mục E7 của chính `06_event_flow.md` và
`07_state_machines.md` §1.2 đã nêu đúng từ trước.
`power_monitor_proc(STATE_INT)` chỉ được chạm tới sau đó, trong **ngữ
cảnh main-loop**, thông qua lời gọi gián tiếp `info->intr_proc(STATE_INT)`
của `anti_chattering_proc()` (`km_extend_io.c:1460`), và chỉ sau khi cửa
sổ debounce 10ms `KM_ANTI_CHATTER_PMON_TIME` xác nhận chân tín hiệu thực
sự vẫn ở mức thấp liên tục. Đoạn trace bên dưới đã được sửa lại cho đúng.

```
NORMAL: POWER_MONITOR pin held high (power good)
ERROR: POWER_MONITOR EXTI edge to low
DETECTION (two-stage, not immediate):
  1. ISR: km_exti_callback() calls start_anti_chattering(POWER_MONI, level) — km_it.c:259-262, arms a 10ms debounce timer, no application decision made yet
  2. main-loop, ~10ms later: anti_chattering_proc() finds the debounce timer at End and the level still low → fires power_monitor_proc(STATE_INT) via the info->intr_proc indirect call — km_extend_io.c:1453-1462
HANDLING (inside power_monitor_proc(STATE_INT), main-loop context): if this is the very first power-on (`g_first_power_on==FIRST_POWER_ON_FLG_START`): poweroff_factor_save(FLICKER) + s800_power_off() immediately (km_extend_io.c:454-458). Otherwise: arm the flicker-failsafe path — `is_powmon_off=1` (:461), later drained by power_flicker_failsafe() (km_extend_io.c:166-180) every main-loop pass via power_monitor_proc(STATE_NORMAL) — which, unlike the STATE_INT path above, IS called directly from the super-loop every iteration (entry.c), not through debounce
RECOVERY: if POWER_MONITOR recovers before the 2000ms POWMONI_FAILSAFE timer expires, no action; the timer is only ever armed once flicker is detected the second way (:171-173) — power_flicker_failsafe() clears the pending flag once the timer fires
NEXT STATE: if the timer does expire → poweroff_factor_save(FLICKER_FAILSAFE) + s800_power_off() (:177-178) → full reset
```

### 2.6 Assert tín hiệu `/HRESET_REQ` (không debounce)
```
NORMAL: /HRESET_REQ pin held at its idle level
ERROR: EXTI edge on PA1 → pending_factor bit set in ISR (km_it.c:254) — this signal is NOT run through the anti-chattering debounce machine, unlike POWER_MONITOR/MSW/24V11
DETECTION: intr_pending_proc() (main-loop) polls the pending bit every iteration (km_extend_io.c:1418-1421)
HANDLING: hreset_req_proc() (km_extend_io.c:489-500) — unconditionally poweroff_factor_save(FLICKER or HRESET_REQ, depending on whether the flicker-failsafe timer happens to already be running) then s800_power_off()
RECOVERY: none within this path — s800_power_off() ends in HAL_NVIC_SystemReset()
NEXT STATE: full reset
```

### 2.7 Double-notification trong trình tự boot
```
NORMAL: S800 notifies "booted" once via an INTERNEAL_STS_COMMAND I2C write with the BOOT bit set
ERROR: a second such notification arrives while g_internal_sts_boot_flg is already 1 (io_extend_write(), km_extend_io.c:317-325) — interpreted as a possible independent LPPP reboot
DETECTION: the flag check itself, inline in io_extend_write()
HANDLING: poweroff_factor_save(BOOT_SEQ_ERROR) + s800_power_off()
RECOVERY: none — full reset
NEXT STATE: full reset; `g_internal_sts_boot_flg` resets to 0 only on the next `s800_power_off()` pass (km_extend_io.c:553)
```

### 2.8 Chuỗi timeout trình tự power-off (failsafe ~120s)
```
NORMAL: poweroff_timer_start() arms PWROFF_MAXTIME1(60s)/PWROFF_WDGTIME(2s, or I2C-set)/BACKUP_WAITTIME(50ms)/PWROFF_SEQTIME(9350ms, or I2C-set) whenever a power-down sequence begins (msw_on_proc(), km_extend_io.c:1190)
ERROR: the sequence never completes in the expected window (external SoC firmware hangs, communication with it is lost, etc. — [UNKNOWN] the exact external failure this guards against, since that logic lives outside this MCU's firmware)
DETECTION: poweroff_timer_proc() every super-loop pass (km_extend_io.c:1305-1338) — PWROFF_MAXTIME1 expiry re-arms PWROFF_MAXTIME2 for another 60s (09_timing.md §3); PWROFF_WDGTIME expiry is checked independently
HANDLING: poweroff_factor_save(TMOUT_LIMIT and/or TMOUT_WDG, OR'd together into one write if both fired, km_extend_io.c:1332-1334) + s800_power_off()
RECOVERY: none — full reset
NEXT STATE: full reset, cause recorded (readable only via the correct command, §1.1)
```

### 2.9 Checksum IAP không khớp sau khi cập nhật firmware (được phát hiện, KHÔNG được thực thi)
```
NORMAL: master streams IAP_PG_COMMAND_DATA writes, then signals completion via IAP_PG_COMMAND_CHANGE_INFO's COMP bit
ERROR: check_sum_calc()'s comparison of the master-supplied checksum against a freshly recomputed one fails (km_i2c_iap.c:334-340)
DETECTION: the comparison itself, inline in i2c_recv_wait() (IAP)
HANDLING: g_iap_reg[CHANGE_STATUS] |= IAP_PG_COMMAND_CHANGE_STATUS_CHKSUM_ERR (km_i2c_iap.c:339) — a status bit is set and is readable by the master via the normal register-read path
RECOVERY: **none coded** — no automatic erase-and-retry, no automatic refusal to proceed
NEXT STATE: **the same as the success path** — msw_on_proc_iap() (km_extend_io_iap.c:28-41) resets back to Main whenever MSW is released and the COMP bit is set, without ever inspecting the CHKSUM_ERR bit. This is the enforcement gap already flagged as E30 in `06_event_flow.md` and restated in `07_state_machines.md` §1.7 — recorded a third time here because Phase 10 is specifically about what happens after detection, and the answer for this one error is "nothing, unless the external master separately checks the status register and chooses not to proceed."
```

### 2.10 Địa chỉ ghi flash IAP nằm ngoài phạm vi
```
NORMAL: g_dest_addr advances by 4 bytes per IAP_PG_COMMAND_DATA write, starting from MAIN_APPLICATION_ADDRESS after a successful erase
ERROR: g_dest_addr falls outside [MAIN_APPLICATION_ADDRESS, +MAIN_APPLICATION_SIZE) — flash_write_32()'s own range check (km_i2c_iap.c:415-417)
DETECTION: the range check itself
HANDLING: flash_write_32() returns ARM_DRIVER_ERROR without touching flash; g_iap_reg[CHANGE_STATUS] |= IAP_PG_COMMAND_CHANGE_STATUS_PSCPU_ERR (km_i2c_iap.c:373)
RECOVERY: none coded — g_dest_addr is not reset, not clamped, no further writes are blocked
NEXT STATE: same gap as §2.9 — msw_on_proc_iap() does not check PSCPU_ERR either
```

### 2.11 Assertion — được xác nhận đã tắt hoàn toàn
`assert_param()` (macro kiểm tra tham số của thư viện HAL, được gọi khắp
source HAL của vendor) biên dịch thành no-op vì `USE_FULL_ASSERT` bị
comment out (`main/Inc/stm32f0xx_hal_conf.h:176`:
`/* #define USE_FULL_ASSERT 1 */`). Ngay cả handler dự phòng,
`assert_failed()` (`mx_init.c:539-556`, bản thân nó chỉ được biên dịch khi
`USE_FULL_ASSERT` được định nghĩa — nên nó **hoàn toàn không được biên
dịch vào firmware này**), cũng có thân hàm rỗng, không báo lỗi, không
reset, không ghi log. **Không có cơ chế assertion nào đang hoạt động ở
bất kỳ đâu trong firmware này.** Bất kỳ sự dùng sai tham số nào mà HAL của
vendor lẽ ra đã bắt được ngay tại điểm gọi thì nay lại tiếp tục trôi qua
trong im lặng, dẫn đến bất kỳ hành vi không xác định (undefined behavior)
nào mà sự dùng sai đó gây ra.

---

## 3. Bảng tổng hợp — detection → recovery cho mọi đường lỗi

| # | Error | Detected by | Recovery action | Scope of recovery | Cause recorded? |
|---|---|---|---|---|---|
| 2.1 | Lỗi bus I2C/timeout/mismatch | `i2c_check_error()` / `i2c_recv_wait()` | `i2c_sw_reset()` | chỉ peripheral I2C | không |
| 2.2 | Lỗi khởi tạo HAL lúc boot | các kiểm tra `!= HAL_OK` nội tuyến | `Error_Handler()` treo (hang) | **không có** — watchdog chưa được arm | không |
| 2.3 | Interrupt không dùng bị kích hoạt | không có (rơi xuống mặc định) | `Default_Handler()` treo → IWDG | reset toàn bộ MCU | không |
| 2.4 | IWDG timeout | chỉ phần cứng | reset phần cứng | reset toàn bộ MCU | không |
| 2.5 | Flicker power-monitor | `power_monitor_proc`/`power_flicker_failsafe` | `s800_power_off()` | reset toàn bộ MCU | **có** (FLICKER/FLICKER_FAILSAFE) |
| 2.6 | Assert `/HRESET_REQ` | `intr_pending_proc` | `s800_power_off()` | reset toàn bộ MCU | **có** (HRESET_REQ hoặc FLICKER) |
| 2.7 | Double-notification boot | `io_extend_write()` | `s800_power_off()` | reset toàn bộ MCU | **có** (BOOT_SEQ_ERROR) |
| 2.8 | Timeout trình tự power-off | `poweroff_timer_proc()` | `s800_power_off()` | reset toàn bộ MCU | **có** (TMOUT_LIMIT/TMOUT_WDG) |
| 2.9 | Checksum IAP không khớp | `check_sum_calc()` | **không có** — chỉ có status bit | n/a | có, nhưng không được hành động theo |
| 2.10 | Ghi flash IAP ngoài phạm vi | kiểm tra phạm vi của `flash_write_32()` | **không có** — chỉ có status bit | n/a | có, nhưng không được hành động theo |
| 2.11 | Dùng sai tham số (HAL `assert_param`) | **bị tắt hoàn toàn** | n/a | n/a | không |

**Mẫu hình:** mọi lỗi mà firmware này có thể *hành động* để xử lý (2.1,
2.5-2.8) đều dẫn đến đúng một trong hai kết quả — reset peripheral I2C,
hoặc reset toàn bộ MCU — không có điểm ở giữa (không có chế độ vận hành
suy giảm - degraded-mode, không có tắt máy một phần). Mọi lỗi mà firmware
này chỉ có thể *báo cáo* (2.9, 2.10) được để hoàn toàn cho I2C master bên
ngoài tự nhận biết và phản ứng, và lệnh truy xuất chẩn đoán được xây dựng
chuyên biệt cho log phong phú nhất trong số này (§1.1) thì lại không thực
sự hoạt động.

---

## 4. Quan sát

1. **Log chẩn đoán thì toàn diện và được nối đầy đủ (§1) — nhưng lệnh
   truy xuất dự định cho nó thì không (§1.1).** Đây là phát hiện có tính
   hành động cao nhất trong tài liệu này: một kỹ thuật viên thực tế debug
   một hàng trả về từ hiện trường bằng cách dùng
   `EXTEND_PWROFF_TIMEOUTLOG_GET_CMD` sẽ kết luận "không có lỗi nào được
   ghi nhận" ở mọi lần, bất kể sự thật là gì, trừ khi họ biết để truy vấn
   `B_REGISTER_PWROFF_LOG` thay vào đó.
2. **Độ chi tiết (granularity) của khôi phục là nhị phân** — chỉ reset
   I2C hoặc reset toàn bộ MCU — không có chế độ vận hành suy giảm/an toàn
   trung gian nào ở bất kỳ đâu trong firmware.
3. **Việc phát hiện (detection) và việc thực thi (enforcement) bị tách
   rời nhau đối với cả hai điều kiện lỗi IAP** (§2.9, §2.10) — firmware
   này tính toán và lưu trữ đáp án đúng nhưng không bao giờ tra cứu nó
   trước khi hành động, một mẫu hình khác biệt với các trường hợp "không
   có recovery được lập trình" (2.2-2.4), nơi thực sự không còn gì khác
   để làm.
4. **`/HRESET_REQ` là tín hiệu ngoài duy nhất được xử lý với debounce
   bằng không** — mọi input vật lý khác (`MSW_ON`, `POWER_MONITOR`,
   `MONI_24V11`) đều đi qua máy anti-chattering (`07_state_machines.md`
   §1.2) trước khi kích hoạt bất cứ điều gì; `/HRESET_REQ` đi thẳng từ
   EXTI đến `s800_power_off()` ngay ở lượt main-loop đầu tiên quan sát
   thấy pending bit.
5. **Assertion được xác nhận là đã tắt, chứ không phải chỉ đơn thuần chưa
   được cấu hình** — điều này đã được xác minh bằng cách đọc trực tiếp
   dòng `#define` thực tế, không phải suy ra từ sự vắng mặt của nó.
