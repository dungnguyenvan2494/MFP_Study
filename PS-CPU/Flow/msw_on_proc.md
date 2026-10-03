Đây là phân tích của `msw_on_proc(STATE_NORMAL)`: [km_extend_io.c:1149](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1149), đường gọi từ vòng lặp chính [entry.c:91](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/PS-CPU/pscpu_s800/main/App/entry.c#L91). Nhánh `STATE_INT` của cùng hàm (sau khi chống rung xong) đã phân tích ở trước; ở đây chỉ mổ xẻ nhánh `STATE_NORMAL`. Các diagram chưa được render thử.

## Cây lời gọi (nhánh NORMAL)

```
msw_on_proc(NORMAL)
├─ km_timer_get_state(BACKUP_WAITTIME / PWROFF_SEQTIME)      →  RAM g_km_timer
├─ IS_CHECKING_CHATTERING(MSW)                                →  RAM anti_chattering_info[0].status
├─ BSP_GPIO_ReadPin(MSW_ON / SB_PG)                           →  GPIOA_IDR / GPIOB_IDR
├─ km_timer_get_state(Reboot_SB_PWR_EN) / km_timer_set        →  RAM (nhánh chết, xem cuối)
├─ set_cycle_time(2)                                          (km_alarm_wake.c dòng 41)
│    ├─ HAL_RTC_GetTime ×2                                    →  RTC_SSR, RTC_PRER, RTC_TR
│    └─ HAL_RTC_SetAlarm_IT                                   →  RTC_WPR, CR, ISR, ALRMAR, ALRMASSR, EXTI_IMR/RTSR
├─ BSP_PWR_enter_stopmode
│    ├─ HAL_SuspendTick / HAL_ResumeTick                      →  SysTick_CTRL
│    └─ HAL_PWR_EnterSTOPMode                                 →  PWR_CR, SCB_SCR, lệnh WFI
└─ s800_power_on                                              (đã vẽ ở phân tích trước)
```

## Diagram A: hai "cổng chặn" đầu hàm (chỉ đọc RAM)

```mermaid
sequenceDiagram
    autonumber
    participant MW as msw_on_proc
    participant TG as km_timer_get_state
    participant IC as IS_CHECKING_CHATTERING
    participant BR as BSP_GPIO_ReadPin
    participant RAM as RAM g_km_timer
    participant REG as GPIOA_IDR

    MW->>TG: km_timer_get_state(BACKUP_WAITTIME)
    TG->>RAM: đọc state
    alt đang Start (khoảng chờ 50 ms)
        MW->>IC: IS_CHECKING_CHATTERING(MSW)
        MW->>BR: ReadPin(GPIOA, PIN_5)
        BR->>REG: đọc GPIOA_IDR (0x48000010) AND 0x20
        MW->>RAM: nếu MSW High ổn định thì g_backupwait_pending |= 1
        MW-->>MW: return (bỏ qua mọi thứ phía dưới)
    end
    MW->>TG: km_timer_get_state(PWROFF_SEQTIME)
    TG->>RAM: đọc state
    alt state == Start
        MW->>REG: đọc MSW_ON, nếu High thì lưu nguyên nhân OFFSEQ_ON và s800_power_off
        MW-->>MW: return
    else state == End
        MW-->>MW: return (bỏ qua mọi sự kiện)
    else state == Normal
        Note over MW: Đi tiếp vào logic chính (Diagram B)
    end
```

## Diagram B: logic chính của nhánh NORMAL

```mermaid
sequenceDiagram
    autonumber
    participant MW as msw_on_proc
    participant IC as IS_CHECKING_CHATTERING
    participant BR as BSP_GPIO_ReadPin
    participant SC as set_cycle_time
    participant PW as BSP_PWR_enter_stopmode
    participant SP as s800_power_on
    participant REG as GPIO IDR

    MW->>IC: IS_CHECKING_CHATTERING(MSW)
    MW->>BR: ReadPin(GPIOA, PIN_5) → GPIOA_IDR bit5
    alt MSW không đang chống rung VÀ MSW_ON Low (công tắc mở, đã ổn định)
        MW->>BR: ReadPin(GPIOB, PIN_4) → GPIOB_IDR bit4 (SB_PG)
        alt SB_PG Low (nguồn SB đã tắt)
            MW->>MW: timer Reboot_SB_PWR_EN khác Start (luôn đúng)
            MW->>SC: set_cycle_time(2) (Diagram C)
            alt trả HAL_OK
                MW->>PW: BSP_PWR_enter_stopmode() (Diagram D)
            end
        end
    else MSW_ON High, hoặc đang chống rung
        MW->>BR: ReadPin(GPIOB, PIN_4) → GPIOB_IDR bit4 (SB_PG)
        alt SB_PG Low (SB chưa bật)
            MW->>SP: s800_power_on() (tự kiểm tra lại MSW, POWER_MONITOR, 24V)
        end
    end
```

Hằng `MSWOFF_STOP_MODE` được định nghĩa ngay trước hàm ([dòng 1126](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1126)) nên `#ifdef` chọn nhánh STOP; nhánh `BSP_PWR_enter_sleepmode` (ngủ nhẹ) trong `#else` không được biên dịch.

## Diagram C: `set_cycle_time(2)` xuống thanh ghi RTC

```mermaid
sequenceDiagram
    autonumber
    participant SC as set_cycle_time (km_alarm_wake.c dòng 41)
    participant GT as HAL_RTC_GetTime (hal_rtc.c dòng 533)
    participant SA as HAL_RTC_SetAlarm_IT (hal_rtc.c dòng 890)
    participant RAM as RAM stop_mode
    participant REG as Thanh ghi RTC (0x40002800)

    SC->>SC: sec bằng 0 thì trả HAL_ERROR
    loop tối đa 5 lần
        SC->>GT: HAL_RTC_GetTime(&sTime_bfr, BIN)
        GT->>REG: đọc RTC_SSR, RTC_PRER, RTC_TR (0x40002800)
        SC->>GT: HAL_RTC_GetTime(&sTime_aft, BIN)
        GT->>REG: đọc RTC_SSR, RTC_PRER, RTC_TR
        Note over SC: hai lần đọc cùng giây thì thoát vòng
    end
    SC->>SC: nếu 5 lần vẫn khác giây thì trả HAL_ERROR (không vào STOP)
    SC->>SC: set_sec = giây hiện tại + 2, quay vòng nếu từ 60 trở lên
    SC->>SC: Alarm A, mask DATEWEEKDAY, HOURS, MINUTES (chỉ so giây)
    SC->>SA: HAL_RTC_SetAlarm_IT(&sAlarm, BIN)
    SA->>REG: RTC_WPR = 0xCA, 0x53 (mở khóa ghi)
    SA->>REG: RTC_CR bỏ ALRAE (tắt Alarm A)
    SA->>REG: RTC_ISR bỏ cờ ALRAF
    loop đến khi ALRAWF = 1 (timeout)
        SA->>REG: đọc RTC_ISR.ALRAWF
    end
    SA->>REG: RTC_ALRMAR = giây + mask, RTC_ALRMASSR
    SA->>REG: RTC_CR |= ALRAE, rồi |= ALRAIE (bật ngắt)
    SA->>REG: EXTI_IMR |= bit17, EXTI_RTSR |= bit17
    SA->>REG: RTC_WPR = 0xFF (khóa lại)
    SC->>RAM: stop_mode = STOP_MODE_ON
    SC-->>SC: return HAL_OK
```

## Diagram D: vào STOP và hai cách thức dậy

```mermaid
sequenceDiagram
    autonumber
    participant PW as BSP_PWR_enter_stopmode
    participant HP as HAL_PWR_EnterSTOPMode
    participant REG as Thanh ghi
    participant CPU as Lõi Cortex-M0
    participant RTC as RTC Alarm A (EXTI17)
    participant MS as Chân MSW_ON (EXTI5)
    participant KI as km_exti_callback (km_it.c dòng 247)
    participant AE as HAL_RTC_AlarmAEventCallback (km_it.c dòng 284)
    participant SC2 as SystemClock_Config2

    PW->>REG: SysTick_CTRL bỏ TICKINT (HAL_SuspendTick)
    PW->>HP: HAL_PWR_EnterSTOPMode(regulator=0, WFI)
    HP->>REG: PWR_CR (0x40007000): bỏ PDDS và LPDS, OR regulator
    HP->>REG: SCB_SCR (0xE000ED10) |= SLEEPDEEP
    HP->>CPU: lệnh WFI, CPU và ngoại vi tốc độ cao dừng
    Note over CPU: Ngủ STOP, PLL tắt, SYSCLK về HSI khi thức

    alt Thức do RTC Alarm A sau khoảng 2 s
        RTC->>CPU: ngắt RTC_IRQ (IRQ 2) qua EXTI17
        CPU->>AE: HAL_RTC_AlarmIRQHandler gọi callback
        AE->>REG: nếu stop_mode ON: RTC_ISR bỏ cờ ALRAF
        AE->>AE: stop_mode = STOP_MODE_OFF
    else Thức do công tắc chính MSW_ON đổi mức
        MS->>CPU: EXTI4_15_IRQ (IRQ 7)
        CPU->>KI: km_exti_callback(PIN_5)
        KI->>KI: stop_mode == ON ?
        KI->>SC2: SystemClock_Config2() (dựng lại PLL ×12, SW = PLL, như SystemClock_Config)
        KI->>REG: HAL_RTC_DeactivateAlarm (tắt Alarm A)
        KI->>KI: stop_mode = OFF, start_anti_chattering(MSW)
    end

    PW->>REG: SCB_SCR bỏ SLEEPDEEP (cuối HAL_PWR_EnterSTOPMode)
    PW->>REG: SysTick_CTRL |= TICKINT (HAL_ResumeTick)
```

## Giải thích từng bước

1. **Hai cổng chặn đầu hàm (Diagram A).** `[SOURCE]` Hai khoảng thời gian bảo vệ (backup 50 ms và chuỗi tắt nguồn 9,35 s) **ưu tiên hơn** logic bình thường. Trong lúc đó hàm chỉ ghi nhớ sự kiện hoặc bỏ qua, không bật/ngủ gì.
2. **Phân nhánh theo công tắc (Diagram B).**
    - Công tắc **mở** (`MSW_ON` Low ổn định) và `SB_PG` Low (hệ thống đã tắt hẳn): chuẩn bị ngủ.
    - Công tắc **đóng**, hoặc đang chống rung, và `SB_PG` Low (SB chưa bật): gọi `s800_power_on()` để bật.
    - `SB_PG` High (đang chạy hoặc đang tắt dở): hàm **không làm gì**, giao cho các hàm khác (`power_monitor_proc`, `poweroff_timer_proc`...).
3. **Hẹn giờ thức (Diagram C).** `[SOURCE]` `set_cycle_time` đọc giờ hiện tại 2 lần liên tiếp cho đến khi giây trùng nhau (tránh đọc đúng lúc chuyển giây), rồi đặt Alarm A tại `giây + 2`. Alarm chỉ so khớp **trường giây**, nên sẽ nổ mỗi phút ở giây đó, chứ không phải đúng "2 s sau". Hàm tính `giây_hiện_tại + 2`, nên với cách đặt này lần nổ đầu tiên cách 2 s, và vì alarm chỉ mask giây nên sẽ lặp mỗi 60 s nếu không được đặt lại. Mỗi vòng lặp `msw_on_proc` lại gọi `set_cycle_time` để đặt lại, nên thực tế chu kỳ là khoảng 2 s.
4. **Vào STOP (Diagram D).** `[SOURCE]` `HAL_SuspendTick` tắt ngắt SysTick (nếu không, SysTick 1 ms sẽ đánh thức ngay). `Regulator = 0` nên `PWR_CR.LPDS` = 0: giữ bộ ổn áp chính trong STOP (`[RM0091]`, tức tiêu thụ cao hơn nhưng thức nhanh hơn). `SLEEPDEEP` = 1 rồi `WFI` đưa chip vào STOP. Mã đặt `SLEEPDEEP` về 0 và bật lại SysTick sau khi thức.
5. **Thức bằng RTC.** `[SOURCE]` `HAL_RTC_AlarmAEventCallback` xóa cờ `ALRAF` và đặt `stop_mode = OFF`. Nó **không** gọi `SystemClock_Config2`.
6. **Thức bằng công tắc.** `[SOURCE]` `km_exti_callback` (chân `MSW_ON`) dựng lại PLL và hủy alarm, rồi bắt đầu chống rung. Chỉ làm vậy khi `stop_mode == ON`.

## Vì sao phải gọi hàm này mỗi vòng lặp (mục đích)

1. **Là trung tâm điều khiển nguồn theo công tắc ở trạng thái ổn định.** `[SOURCE]` Nhánh `STATE_INT` chỉ chạy một lần khi công tắc đổi mức. Nhánh `STATE_NORMAL` chạy liên tục để **duy trì** trạng thái đúng: nếu công tắc đóng mà nguồn chưa lên thì bật, nếu công tắc mở mà nguồn đã tắt thì ngủ. Nhờ vậy hệ thống tự phục hồi dù bỏ lỡ một sự kiện ngắt (ví dụ ngắt đến khi đang trong khoảng bảo vệ).
2. **Tiết kiệm điện khi máy tắt.** `[SOURCE]` Comment ở `km_alarm_wake.c` và `MSWOFF_STOP_MODE` cho thấy ý đồ: khi công tắc chính tắt, PS-CPU vào STOP, chỉ thức mỗi 2 s. Lịch sử sửa đổi trong code còn có mục `OP_BTS-21266` liên quan đến dòng tiêu thụ khi nguồn chính tắt, cho thấy đây là yêu cầu có thật.
3. **Vẫn phải thức theo chu kỳ.** `[INFERENCE]` Thức mỗi 2 s để vòng lặp tiếp tục refresh watchdog (timeout khoảng 26 s, xem phân tích `BSP_WDT_Refresh`) và để kiểm tra lại công tắc phòng khi ngắt bị sót.
4. **Là cầu nối giữa khởi động lạnh và khởi động lại.** `[SOURCE]` Khi `s800_power_off()` kết thúc bằng `HAL_NVIC_SystemReset()`, PS-CPU khởi động lại và chạy lại hàm này; nếu công tắc đã mở và `SB_PG` Low, nó đưa chip ngay vào STOP.

## Bảng chân tri (nhánh NORMAL, sau hai cổng chặn)

|Chống rung MSW|MSW_ON|SB_PG|Hành động|
|---|---|---|---|
|Không|Low|Low|Đặt Alarm 2 s, vào STOP|
|Không|Low|High|Không làm gì (tắt dở hoặc đang chạy)|
|Không hoặc Có|High hoặc đang chống rung|Low|`s800_power_on()`|
|Không hoặc Có|High hoặc đang chống rung|High|Không làm gì|

## Điểm đáng chú ý

- **Nhánh `Reboot_SB_PWR_EN` là mã chết.** `[SOURCE]` Timer này không bao giờ được `Start` (đã nêu), nên điều kiện `!= Start` luôn đúng.
- **Rủi ro về clock sau khi thức bằng RTC. Đây là suy luận, cần kiểm chứng trên phần cứng.** `[SOURCE]` `HAL_RTC_AlarmAEventCallback` đặt `stop_mode = OFF` mà không dựng lại PLL; chỉ ngắt `MSW_ON` mới gọi `SystemClock_Config2`. `[RM0091]` Sau STOP, SYSCLK về HSI (8 MHz) và PLL tắt. Hệ quả có thể xảy ra: sau mỗi lần thức do RTC, CPU chạy ở 8 MHz trong khi `SysTick_LOAD` và `TIM3_PSC` được tính cho 48 MHz, nên các tick chậm đi 6 lần (1 ms thành 6 ms) cho đến lần STOP kế tiếp. Nếu công tắc được đóng trong khoảng ngắn lúc đang thức giữa hai lần STOP (`stop_mode` đã là OFF), ISR sẽ **không** dựng lại PLL, và PS-CPU có thể tiếp tục chạy ở 8 MHz khi đã bật nguồn. Tôi chưa chứng minh được điều này xảy ra thật, vì chưa kiểm chứng hành vi sau STOP trên phần cứng.
- **Alarm A cũng là tín hiệu `WAKEUP` phát cho S800.** `[SOURCE]` Comment trong `stm32f0xx_hal_rtc.c` (đã bị `#if 0` ở `HAL_RTC_AlarmIRQHandler`) giải thích: PS-CPU không được xóa cờ `ALRAF` vì làm vậy sẽ làm tín hiệu `WAKEUP` mất một nhịp; chủ (S800) tự xóa. `[INFERENCE]` Đó là lý do `RTC_CR.OSEL = ALARMA` được bật ở `MX_RTC_Init`. Callback riêng của dự án chỉ xóa cờ khi `stop_mode == ON`, tức là khi alarm là do PS-CPU tự đặt.
- **Mọi chân EXTI đều đánh thức STOP**, không chỉ `MSW_ON` (`POWER_MONITOR`, `MONI_24V11`, `AP_PWR_EN`, `_HRESET_REQ`). `[INFERENCE]` Với các chân khác, callback không dựng lại clock, nên chúng được xử lý ở clock HSI cho đến lần kế tiếp.

## Chưa xác minh

- Offset thanh ghi (`IDR`, `PWR_CR`, `SCB_SCR`, RTC, EXTI) là kiến thức reference manual; base address đọc từ `stm32f031x6.h`.
- Hành vi clock sau STOP (SYSCLK về HSI, PLL tắt) là kiến thức reference manual, chưa đo trên phần cứng.
- `PWR_MAINREGULATOR_ON` bằng 0: tôi giả định theo quy ước HAL, chưa mở header.
- Tôi chưa đọc `__HAL_RTC_ALARM_EXTI_CLEAR_FLAG` và `HAL_RTC_DeactivateAlarm` ở mức thanh ghi.