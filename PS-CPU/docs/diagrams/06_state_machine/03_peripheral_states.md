# Sơ đồ trạng thái — State Machine ngoại vi (Peripheral)

**Danh mục: State machine Ngoại vi.** Bốn state machine nhỏ, mỗi cái được
sở hữu bởi một module thuộc lớp BSP/driver chứ không phải logic ứng dụng
cấp cao nhất. Nguồn: `07_state_machines.md` §1.2, §1.3, §1.5, §1.6.

## 3a. Anti-chattering (debounce) — 3 instance (MSW_ON, POWER_MONITOR, MONI_24V11_MONI)

```mermaid
stateDiagram-v2
    [*] --> AC_STOP_STATE : static init [km_it.c:20-54]

    AC_STOP_STATE --> AC_START_STATE : EXTI edge on the signal\n/ start_anti_chattering()\nlatch start_level=prev_level=level,\narm decision_time timer\n[km_it.c:489-501, ISR context]

    AC_START_STATE --> AC_START_STATE : main-loop poll: prev_level != level\n(signal changed mid-debounce)\n/ start_anti_chattering() again\n[km_extend_io.c:1447-1451]

    AC_START_STATE --> AC_STOP_STATE : timer reaches End,\nstart_level == level,\nintr_proc != NULL\n/ fire intr_proc(STATE_INT)\n[km_extend_io.c:1453-1462]

    AC_START_STATE --> AC_STOP_STATE : timer reaches End,\nintr_proc == NULL (24V11 entry)\n/ no callback fired\n[km_extend_io.c:1456-1462]

    AC_START_STATE --> AC_STOP_STATE : timer reaches End,\nstart_level != level (bounced back)\n/ no callback fired\n[km_extend_io.c:1453-1457]
```
Timeout (decision_time): MSW=10ms, POWER_MONITOR=10ms, MONI_24V11=30ms
(`km_it.h:6-10`). Không có retry, không có state lỗi — chỉ tồn tại 2 state.

## 3b. Mẫu software timer — 1 hình dạng, 16 instance (`g_km_timer[]`)

```mermaid
stateDiagram-v2
    [*] --> Normal : static init (BSS zero)

    Normal --> Start : km_timer_set(kind, val, Start)\n[~15 call sites, e.g. km_extend_io.c:1287-1290]

    Start --> Start : every 1ms TIM3 tick,\nms_time -= 1, ms_time > 0\n/ km_tim_callback() [ISR][km_it.c:340-348]

    Start --> End : ms_time reaches 0\n/ km_tim_callback() [ISR][km_it.c:344-346]

    End --> Normal : consumer calls\nkm_timer_set(kind, 0, Normal)\n[e.g. km_extend_io.c:1315,1454]

    End --> End : consumer does not clear it\n(e.g. PWROFF_MAXTIME2/WDGTIME\nuntil s800_power_off() re-inits)\n[UNKNOWN self-clear for all 16 kinds]
```
Hai trong số mười sáu loại (`Rstusb`, `MONI_24V11_ON`) không bao giờ rời
khỏi `Normal` — được khai báo, nhưng không bao giờ được khởi động
(`09_timing.md` §3).

## 3c. Cờ chế độ STOP (`stop_mode`)

```mermaid
stateDiagram-v2
    [*] --> STOP_MODE_OFF : static init [km_alarm_wake.c:16]

    STOP_MODE_OFF --> STOP_MODE_ON : msw_on_proc() enters STOP\n(MSW off, SB_PG off,\nMSWOFF_STOP_MODE active)\n/ set_cycle_time(2) [km_alarm_wake.c:108]\n+ BSP_PWR_enter_stopmode()\n[main-loop context]

    STOP_MODE_ON --> STOP_MODE_OFF : RTC alarm fires (2s)\n/ HAL_RTC_AlarmAEventCallback()\n[km_it.c:284-300, ISR context]

    STOP_MODE_ON --> STOP_MODE_OFF : MSW_ON EXTI edge\n/ km_exti_callback():\nSystemClock_Config2()+\nHAL_RTC_DeactivateAlarm()\n[km_it.c:267-274, ISR context]
```
Cả hai lối thoát đều xảy ra trong ngữ cảnh ISR; lối vào xảy ra trong ngữ
cảnh main-loop — một state machine 2 trạng thái mà hai cạnh của nó sống
trong hai domain thực thi khác nhau.

## 3d. Chốt (latch) driver IWDG (`WDT_STATE`, lớp BSP)

```mermaid
stateDiagram-v2
    [*] --> WDT_STATE_INIT

    WDT_STATE_INIT --> WDT_STATE_START : BSP_WDT_Start() called\n/ HAL_IWDG_Start()\n[BSP_WDT_STM32F03x_Nucleo.c:31-45]

    WDT_STATE_START --> WDT_STATE_START : BSP_WDT_Start() called again\n/ returns HAL_BUSY-derived error,\nno re-start [:34-36]

    WDT_STATE_INIT --> WDT_STATE_INIT : BSP_WDT_Refresh() called\nbefore Start() / returns error,\nno HAL_IWDG_Refresh() call [:56-58]
```
Một chốt một chiều — không có transition nào quay trở lại
`WDT_STATE_INIT` tồn tại ở bất kỳ đâu (khớp với phần cứng: IWDG không thể
bị dừng lại một khi đã được khởi động, `[HARDWARE ASSUMPTION]`).

## Truy vết

| Machine | Function | File:Line |
|---|---|---|
| Anti-chattering | `start_anti_chattering`, `anti_chattering_proc` | `km_it.c:489`, `km_extend_io.c:1438` |
| Software timer | `km_timer_set`, `km_tim_callback` | `km_it.c:384,312` |
| STOP mode | `set_cycle_time`, `HAL_RTC_AlarmAEventCallback` | `km_alarm_wake.c:41`, `km_it.c:284` |
| IWDG latch | `BSP_WDT_Start`, `BSP_WDT_Refresh` | `BSP_WDT_STM32F03x_Nucleo.c:31,53` |
