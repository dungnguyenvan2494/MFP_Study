# PS-CPU (pscpu_s800) — Firmware Architecture

**Status:** Source-grounded architecture reconstruction. **Every non-trivial statement below cites `file:line` (or a line range) against the exact source this session decoded byte-for-byte from Google Drive.** Where a claim goes beyond what a cited line proves, it is tagged `[INFERRED]`; where the source could not settle a question, it is tagged `[UNKNOWN]`. This document builds on `01_repository_map.md` (module inventory) and `00_project_scope.md` (source-location/provenance) but goes one level deeper: it answers, for every dependency edge, **who calls whom, who owns the resource, who initializes it, who services it at runtime, and who consumes its output.**

**Provenance of line numbers:** every file cited here was downloaded this session via the Google Drive API, base64-decoded programmatically (not retyped by hand), and diffed by byte-count against the Drive file-size metadata before use — e.g. `main/App/km_extend_io.c` = 65,626 bytes / 1,540 lines, `main/App/km_i2c.c` = 30,609 bytes / 851 lines, `main/App/km_it.c` = 17,741 bytes / 502 lines, `iap/App/km_i2c_iap.c` = 21,039 bytes / 543 lines — all confirmed to match exactly. Line numbers are therefore exact, not estimated.

**This is a two-image system.** Everything below is described once for the **Main** application image (ROM `0x08000000`, 20 KB — confirmed via `main/MDK-ARM/PowerSubCPU.uvprojx`, per `01_repository_map.md` §3.6) and once for the **IAP** field-update image (ROM `0x08005000`, 12 KB), because they are separately-linked binaries that share source patterns but are not the same running program.

---

## 0. Scope correction to `01_repository_map.md`

Full-text reading of `main/App/km_i2c.c` this session (not done in the prior pass) finds the caller that `01_repository_map.md` §3.12 flagged as missing:

```c
// main/App/km_i2c.c:686-693
case IAP_PG_COMMAND:
    /* マジックナンバチェック */
    if(w_data == IAP_PG_COMMAND_MAGICNUM){
        /* 時刻退避 */
        /* save_rtc_reg(); ... 不要 */
        DeInit();                          // km_i2c.c:691 -> entry.c:186
        jump2iap();                        // km_i2c.c:692 -> entry.c:159
    }
    break;
```

So **`jump2iap()` is not dead code** — it is reached from `i2c_recv_wait()`'s dispatcher (`main/App/km_i2c.c:485`, the `case IAP_PG_COMMAND:` arm) when the main SoC writes the magic number `IAP_PG_COMMAND_MAGICNUM = 0x11223344` (`main/Inc/km_i2c.h:141` per prior read) to I2C register `0x90`. The call chain is: **main SoC (I2C master, off-chip) → `i2c_recv_wait()` (`km_i2c.c:485`) → `DeInit()` (`entry.c:186-192`) → `jump2iap()` (`entry.c:159-172`) → CPU jumps into the IAP image at `0x08005000` without a hardware reset.** This resolves the open finding in `01_repository_map.md` §3.12/§7.

---

## 1. Layer model (as this firmware actually implements it, not a generic MCU stack)

| # | Layer | What occupies it here | Present? |
|---|---|---|---|
| 1 | **Hardware** | STM32F031C6 (Cortex-M0/ARMv6-M), I2C1, RTC+backup domain, TIM3, IWDG, GPIO ports A/B/C/F, PWR (STOP/SLEEP), internal Flash, NVIC/EXTI. Confirmed device string `STM32F031C6` in both `.uvprojx` files (`01_repository_map.md` §3.6). | Yes |
| 2 | **MCU Peripheral (register/CMSIS level)** | Direct register pokes that bypass HAL: `RTC->TR`/`RTC->DR` (`main/App/km_i2c.c:74-76`, dead `save_rtc_reg()`), `RTC_BASE + offset` casts (`km_i2c.c:552-553`, `658-659`, `666-671`), `RTC->ISR`/`WPR` direct writes in `KM_RTC_Restore()` (`main/Src/mx_init.c:310-351`), raw `NVIC_DisableIRQ()` (`km_extend_io.c:538-540`), `__HAL_RCC_I2C1_FORCE_RESET()`/`RELEASE_RESET()` (`km_i2c.c:358-359`, `km_extend_io.c:1536-1537`). | Yes — used as an escape hatch *underneath* HAL, not instead of it |
| 3 | **ISR** | `main/Src/stm32f0xx_it.c` (Main, 8 handlers) / `iap/Src/stm32f0xx_it.c` (IAP, 3 handlers) — see §5 "Interrupt handlers" below | Yes |
| 4 | **BSP** | `common/Drivers/BSP/Inc/BSP_STM32F03x_Nucleo.h` + 6 `.c` files — `BSP_GPIO_*`, `BSP_TIM_*`, `BSP_WDT_*`, `BSP_PWR_*`, `BSP_Reg_*`, `BSP_ADC_*` (unused) | Yes |
| 5 | **HAL** | ST's `STM32F0xx_HAL_Driver` — consumed by `mx_init.c`, `stm32f0xx_hal_msp.c`, and by name in `km_i2c.c`/`km_extend_io.c`/`km_alarm_wake.c`/`km_it.c` (`HAL_NVIC_*`, `HAL_Delay`, `HAL_RTC_*`, `__HAL_RCC_*`) | Yes |
| 6 | **Driver** | CMSIS-Driver `Driver_I2C0` (`ARM_DRIVER_I2C`, declared `extern` at `main/App/km_i2c.c:28`, `main/App/entry.c:22`, `iap/App/km_i2c_iap.c:23`) and `Driver_Flash0` (`ARM_DRIVER_FLASH`, `iap/App/km_i2c_iap.c:24`, IAP-only) | Yes |
| 7 | **RTOS** | **None.** No CMSIS-RTOS2/FreeRTOS/RTX component in either `.uvprojx` RTE list; no `osKernelStart`/`osThreadNew`/`xTaskCreate` anywhere in the 24 files read this session. Both images are single-threaded bare-metal superloops (§4). | **Absent — confirmed** |
| 8 | **Middleware** | **None found.** No filesystem, no USB stack, no additional protocol middleware in any `#include` list read (`entry.c:9-18`, `km_i2c.c:13-17`, `km_i2c_iap.c:9-13`). | **Absent — no positive evidence found** |
| 9 | **Protocol** | I2C-slave register-map protocol: `main/App/km_i2c.c` (Main, `i2c_recv_first()`@317, `i2c_recv_wait()`@485), `iap/App/km_i2c_iap.c` (IAP, `i2c_recv_first()`@193, `i2c_recv_wait()`@240) | Yes |
| 10 | **Service** | `main/App/km_ca72_status.c` (SoC-status cache), the event-record ring buffer inside `km_i2c.c` (`setEventRecord()`@838), power-off-reason logging (`poweroff_factor_save()`@144) | Yes |
| 11 | **Application** | `main/App/entry.c` (`main()`@45), `main/App/km_extend_io.c` (1,540 lines — the power-sequencing state machine), `main/App/km_it.c` (timers/debounce, arguably Driver-flavored but application-owned state), `iap/App/entry_iap.c`, `iap/App/km_extend_io_iap.c` | Yes |

**Boundary consequence of "no RTOS":** there is no scheduler, no task, no mutex/semaphore, and no context switch anywhere in this firmware. Every function call in §4/§5 below executes to completion on the single Cortex-M0 thread stack, either in `Thread mode` (the superloop) or `Handler mode` (an ISR, which can only be interrupted by a higher-priority ISR — and this firmware doesn't set differentiated priorities: every `HAL_NVIC_SetPriority()` call this session found uses `(x, 0, 0)`, e.g. `main/Src/mx_init.c:509,512,515`, `stm32f0xx_hal_msp.c:91` region — i.e. **all interrupts share the same priority level**, so ISR nesting by priority does not happen; only tail-chaining does).

---

## 2. Module dependency graph

```
                      ┌─────────────────────────────────────────────┐
                      │   Hardware (STM32F031C6: I2C1, RTC, TIM3,    │
                      │   IWDG, GPIO A/B/C/F, PWR, Flash, NVIC/EXTI) │
                      └───────────────┬───────────────────────────────┘
                                      │ register I/O
                      ┌───────────────▼───────────────┐
                      │  ISR layer                     │
                      │  main/Src/stm32f0xx_it.c        │  iap/Src/stm32f0xx_it.c
                      │  (8 handlers, §5)                │  (3 handlers, §5)
                      └───────┬───────────┬─────────────┘
                              │           │
              ┌───────────────▼─┐    ┌────▼─────────────┐
              │ HAL (ST Cube)    │    │ CMSIS-Driver       │
              │ HAL_GPIO_EXTI_*  │    │ Driver_I2C0 (I2C)   │
              │ HAL_TIM_IRQHandler│   │ Driver_Flash0 (IAP) │
              │ HAL_I2C_EV/ER_*  │    │ km_i2c.c:28, entry.c:22
              │ HAL_RTC_AlarmIRQ*│    └────┬───────────────┘
              └───────┬──────────┘         │ ISR-context callback
                      │ weak-callback       │ rx_comp() km_i2c.c:171 / km_i2c_iap.c:90
        ┌─────────────▼─────────────┐       │
        │ BSP                        │       │
        │ BSP_GPIO_*, BSP_TIM_*,     │       │
        │ BSP_WDT_*, BSP_PWR_*       │       │
        │ common/Drivers/BSP/*.c     │       │
        └──┬───────────┬─────────────┘       │
           │           │                     │
  ┌────────▼───┐  ┌────▼──────────┐   ┌──────▼─────────────┐
  │ Driver      │  │ Driver         │   │ Protocol             │
  │ km_it.c     │  │ km_alarm_wake.c│   │ km_i2c.c (Main)       │
  │ (timers,    │  │ (RTC Alarm A)  │   │ km_i2c_iap.c (IAP)    │
  │ debounce,   │  │ set_cycle_time │   └──────┬───────────────┘
  │ pending-    │  │ @41            │          │
  │ factor)     │  └───────┬────────┘          │ writes/reads
  └──┬──────────┘          │                    │
     │                     │ called from        │
     │        ┌────────────▼────┐               │
     │        │ Application       │◄─────────────┘ reads g_io_extend_memory[],
     │        │ km_extend_io.c    │                 calls setEventRecord()
     │        │ (power sequencing,│
     │        │ 1540 lines)       │
     │        └─────┬─────────────┘
     │              │ Service
     │        ┌──────▼────────────┐
     │        │ km_ca72_status.c   │
     │        │ (SoC status cache) │
     │        └────────────────────┘
     │
     └──────────► Application entry: main/App/entry.c::main() (entry.c:45-147)
                  iap/App/entry_iap.c::main() (entry_iap.c:40-79)
```

Concrete edges, each with the four questions answered and a citation:

| Edge | Calls | Owns | Initializes | Services | Consumes |
|---|---|---|---|---|---|
| `entry.c::main()` → `MX_Init()` | `entry.c:52` | — | `mx_init.c::MX_Init()` (`81-107`) owns nothing itself; it *sequences* ownership handoff | once, at boot | `main()`'s own stack frame | n/a |
| `MX_Init()` → `MX_RTC_Init(DEF_RTC_INIT\|DEF_RTC_MSPINIT)` | `mx_init.c:96,100` | RTC peripheral registers | `MX_RTC_Init()` (`mx_init.c:244-273`) | once, at boot (mode chosen by `IS_BACKUP_EXIST` macro, `mx_init.c:60`, evaluated at `MX_Init()`'s `if(!(hrtc.Instance->ISR & RTC_ISR_INITS))` gate `mx_init.c:94`) | `KM_RTC_Restore()` (`102`), `km_i2c.c`'s RTC register reads/writes (§0-table below) |
| `MX_Init()` → `KM_RTC_Restore()` | `mx_init.c:102` | RTC backup registers `RTC_BKP0R/1R` (aliased `RTC_TR_BKP`/`RTC_DR_BKP` in `km_i2c.h:34-35`) | n/a (restore, not init) | once, at boot, with a `loop_limit` bounded busy-wait (`mx_init.c:328,341` — `> 10000` guard) | consumed immediately into `RTC->TR`/`RTC->DR` (`mx_init.c:325-326`) |
| `entry.c::main()` → `km_it_init()` | `entry.c:65` | `pending_factor` (`km_it.c:17`), `g_km_timer[16]` (`km_it.c:18`), `ei_ref[32]` (`km_it.c:19`), `anti_chattering_info[3]` (`km_it.c:20-46`) | `km_it_init()` (`km_it.c:364-370`): `init_ei_ref()` (`365`), `BSP_GPIO_Initialize(km_exti_callback)` (`367`), `BSP_TIM_Initialize(TIMER_3, km_tim_callback)` (`368`) | called once at boot | BSP registers the two callback function pointers that ISRs will later invoke (`TIM3_IRQHandler`→HAL→`km_tim_callback`; `EXTIx_IRQHandler`→HAL→`km_exti_callback`) |
| `entry.c::main()` → `BSP_TIM_Start(TIMER_3,1)` | `entry.c:66` | TIM3 hardware counter | starts the 1 ms tick | every ms thereafter, via ISR | `km_it.c::km_tim_callback()` (`312-360`) decrements `g_km_timer[]` |
| `entry.c::main()` → `s800_power_on()` | `entry.c:75` | `g_first_power_on`, `g_power_on_flg` (`km_extend_io.c:44,46`) | n/a | once at boot (also called from dead `km_adc.c:66`, §7) | drives `IR_P_ON`/`MC_P_ON` output pins via `io_extend_write()` |
| `entry.c::main()` → `i2c_recv_first()` | `entry.c:77` | `i2c_info` struct (`km_i2c.c:33-46`) | `Driver_I2C0.Initialize(rx_comp)` (`km_i2c.c:338`), `Driver_I2C0.Control(KM_ARM_I2C_MANUAL_ACK_MODE,1)` (`340`), `Driver_I2C0.SlaveReceive(...)` (`342`) | once at boot; re-invoked by `i2c_sw_reset()` (`km_i2c.c:356-363`) whenever `i2c_check_error()` (`379-460`) trips | first I2C transaction after boot |
| `entry.c::main()` while(1) → `i2c_recv_wait()` | `entry.c:126` | `i2c_info.state` (protocol FSM) | n/a | every superloop iteration (~hundreds of Hz, no fixed period — bare loop) | main SoC's I2C reads/writes |
| `km_i2c.c::rx_comp()` (ISR-context CMSIS-Driver callback) | registered `km_i2c.c:338`, defined `171-241` | `i2c_info.rx_during/tx_during/error_code/dir_error` | n/a | every I2C byte/transfer event, from `I2C1_IRQHandler` (`stm32f0xx_it.c:203-213`) → HAL → CMSIS-Driver → `rx_comp` | flags consumed by `i2c_recv_wait()` next superloop pass, and by `i2c_check_error()` |
| `km_i2c.c::i2c_recv_wait()` → `io_extend_read()`/`io_extend_write()` | e.g. `km_i2c.c:663-664` (read), `680` (write) | `g_io_extend_memory[TYPE_Io_Extend_Max]` (`km_extend_io.c:67`) | n/a — the array's *initial* contents are set once by `io_extend_memory_write()` (`km_extend_io.c:339-365`), called from `entry.c:71` | every I2C register access in the `0x70`-`0x82` range | main SoC (read side); `km_extend_io.c`'s `*_proc()` functions (write side, e.g. `mc_p_on_proc` reading `MC_P_ON_NUM` bit at `km_extend_io.c:766`) |
| `km_i2c.c::i2c_recv_wait()` → direct `RTC_BASE`+offset write | `km_i2c.c:658-659`, `666-671` | RTC peripheral registers **directly**, bypassing `HAL_RTC_SetTime`/`SetAlarm` | n/a | every RTC-range I2C write (`0x00`-`0x5E`) | RTC hardware register file; read back the same way at `km_i2c.c:552-553` |
| `km_extend_io.c::*_proc()` → `km_it.c` timer/debounce API | e.g. `km_timer_set()` called ~15 places, `get_anti_chattering_info()` at `km_extend_io.c:1443` | `g_km_timer[]`, `anti_chattering_info[]` (owned by `km_it.c`) | n/a (timers armed per-call) | every superloop iteration via `anti_chattering_proc()` (`km_extend_io.c:1438-1467`, itself called `entry.c:118`) | debounced GPIO levels consumed by `msw_on_proc`, `power_monitor_proc`, etc. |
| `km_it.c::km_exti_callback()` (ISR context) → `set_pending_factor_bit()` | `km_it.c:250,253` (for `_HRESET_REQ_Pin`, `AP_PWR_EN_Pin`) | `pending_factor` bitmask | n/a | every rising/falling edge on those 2 pins | drained by `km_extend_io.c::intr_pending_proc()` (`1416-1427`, called `entry.c:116`) — **this is the concrete ISR→main-loop deferral mechanism** |
| `km_it.c::km_exti_callback()` (ISR context) → `start_anti_chattering()` | `km_it.c:258,262,266,276` (for `POWER_MONITOR`, `MONI_24V11`, `MSW_ON` pins) | `anti_chattering_info[idx].status/start_level/prev_level` | arms `km_timer_set(info->timer_kind, info->decision_time, TYPE_Km_Timer_Start)` (`km_it.c:497`) | every edge on those 3 pins | consumed by `anti_chattering_proc()` (main-loop, `km_extend_io.c:1438-1467`) once the debounce timer elapses |
| `km_extend_io.c::msw_on_proc()` → `km_alarm_wake.c::set_cycle_time(2)` → `BSP_PWR_enter_stopmode()` | `km_extend_io.c:1202-1203` | RTC Alarm A registers | arms a 2 s wake alarm (`km_alarm_wake.c:41-111`) | only on the `MSWOFF_STOP_MODE` path (`#define` at `km_extend_io.c:1126`, active) when MSW is off and `SB_PG` is low | CPU enters STOP; wakes via `RTC_IRQHandler` (`main/Src/stm32f0xx_it.c:131-137`) → `HAL_RTC_AlarmIRQHandler` → `HAL_RTC_AlarmAEventCallback()` (`km_it.c:284-306`) |
| `km_i2c.c::i2c_recv_wait()` (`IAP_PG_COMMAND` case) → `DeInit()` → `jump2iap()` | `km_i2c.c:691-692` | I2C/TIM3/RTC-alarm peripheral state (torn down), then CPU control | n/a (teardown + handoff) | once, only when the main SoC writes the magic number | the IAP image, entered via a software jump (not a hardware reset) — see §0 |

---

## 3. Responsibility matrix

| Module (file) | Classification | Owns (resource) | Initializes | Services (per iteration/event) | Consumes (reads) |
|---|---|---|---|---|---|
| `startup_stm32f031x6.s` | Startup | CPU registers, initial SP/PC | Reset vector → `SystemInit()` → `main()` | once | n/a |
| `common/.../system_stm32f0xx.c` | Startup | `SystemCoreClock` | `SystemInit()` | once | n/a |
| ST `STM32F0xx_HAL_Driver` `[INFERRED path]` | HAL | peripheral register bit-fields (indirectly) | called by `mx_init.c`/`hal_msp.c` | on every `HAL_*` call site | HAL config macros (`stm32f0xx_hal_conf.h`) |
| `I2C_stm32f0xx.c` (CMSIS-Driver) | Driver | I2C1 slave-transaction state machine | `Driver_I2C0.Initialize()` — called from `km_i2c.c:338` (Main) / `km_i2c_iap.c:209` (IAP) | every I2C1 interrupt | `Driver_I2C0.Control/SlaveReceive/SlaveTransmit` calls from `km_i2c.c`/`km_i2c_iap.c` |
| `FLASH_stm32f0xx.c` (CMSIS-Driver) | Driver | internal Flash controller | `Driver_Flash0.Initialize(NULL)` — `km_i2c_iap.c:214` (**IAP only**) | on `EraseSector`/`ProgramData` calls (`km_i2c_iap.c:441`, `421`) | flash write data from I2C payload (`km_i2c_iap.c:340-352`) |
| `common/Drivers/BSP/*` | BSP | GPIO/TIM/WDT/PWR register access | `BSP_GPIO_Initialize()`/`BSP_TIM_Initialize()` called from `km_it.c:367-368` (Main) and `km_extend_io_iap.c:80` (IAP, GPIO only) | every `BSP_GPIO_ReadPin`/`WritePin`/`BSP_WDT_Refresh` call (dozens of sites) | callback function pointers registered at init |
| `main/App/km_it.c` | Driver | `pending_factor`, `g_km_timer[16]`, `ei_ref[32]`, `anti_chattering_info[3]` | `km_it_init()` — `entry.c:65` | 1 ms tick (`km_tim_callback`, ISR) + every GPIO edge (`km_exti_callback`, ISR) | pin identities from `km_extend_io.h` |
| `main/App/km_alarm_wake.c` | Driver | RTC Alarm A config, `stop_mode` global (`km_alarm_wake.c:16`) | `set_cycle_time()` called from `km_extend_io.c:1202` only | on STOP-mode entry | current `RTC->TR` (double-read pattern, `km_alarm_wake.c` — full body read in prior session) |
| `main/App/km_i2c.c` | Protocol/Service | `i2c_info` struct, `g_checksum_value`, `g_main_version[]`, `st_EventRecord[250]` | `i2c_recv_first()` — `entry.c:77` | every superloop pass (`i2c_recv_wait()`) + every I2C IRQ (`rx_comp()`) | `g_io_extend_memory[]` (via `io_extend_read/write`), RTC registers directly |
| `main/App/km_ca72_status.c` | Service | `static _status` (`km_ca72_status.c:11`) | n/a (statically initialized to `CA72_OFF`) | `set_ca72_status()` called from `km_extend_io.c:528,665,670` | `get_ca72_status()` called from `km_extend_io.c:896,971` |
| `main/App/km_extend_io.c` | Application | `g_io_extend_memory[]`, `io_extend[14]` (const), `g_power_on_flg`, `g_first_power_on`, `g_24v_check_flg_off`, `g_internal_sts_boot_flg`, `is_usb2_oe_from_main_cpu_ctrl`, `g_timer_flg` (all file-static, `km_extend_io.c:42-48`) | `internal_sts_init()` (`entry.c:64`), `extend_output_init()` (called from `s800_power_off()`@551, `s800_power_on()` per prior read) | every superloop iteration, ~15 `*_proc()` calls (`entry.c:83-144`) | `km_it.c` timers/debounce, `km_ca72_status.c` state, GPIO via BSP |
| `main/App/entry.c` | Application | superloop `model` local (`entry.c:48`) | orchestrates everyone else's init (`entry.c:50-80`) | forever, one iteration per loop pass | n/a — it is the top |
| `iap/App/entry_iap.c` | Application | `VectorTable[48]` at `0x20000000` (`entry_iap.c:16`) | copies vector table (`57-60`), remaps SRAM (`68`) | forever, 2-call loop (`73-77`) | Main image's flash region (read-only, as vector-table source) |
| `iap/App/km_i2c_iap.c` | Protocol | `i2c_info` (simpler struct, no timeout fields — `km_i2c_iap.c:33-40`), `g_iap_reg[4]`, `g_dest_addr` | `i2c_recv_first()` — `entry_iap.c:70`, which also calls `main_project_flash_erase()` (`km_i2c_iap.c:216`) | every superloop pass + every I2C IRQ | Main's flash region (erase/write target) |
| `iap/App/km_extend_io_iap.c` | Application | none (stateless) | `km_it_init_iap()` — `entry_iap.c:53` | every `MSW_ON` edge | `g_iap_reg[]` update-complete bit (`km_extend_io_iap.c:35`) |

---

## 4. Initialization ownership (exact boot sequence)

### Main image

```
Reset_Handler (startup_stm32f031x6.s)         [Startup]
  -> SystemInit()                              [Startup]
  -> main()                                     entry.c:45
       __HAL_RCC_PWR_CLK_ENABLE()                entry.c:50            [HAL macro, direct]
       MX_Init()                                 entry.c:52  -> mx_init.c:81
           HAL_Init()                             mx_init.c:87          [HAL]
           SystemClock_Config()                   mx_init.c:90 -> 113   [HAL: RCC/PLL/I2C1&RTC clock src]
           MX_GPIO_Init()                         mx_init.c:91 -> 419   [HAL: all GPIO pin modes, EXTI priorities @509-516]
           MX_I2C1_Init()                         mx_init.c:93 -> 202   [HAL: I2C1 Timing=0x00200000, OwnAddress1=122, OwnAddress2=124]
           hrtc.Instance = RTC; if(!INITS) {       mx_init.c:94-101
               MX_RTC_Init(DEF_RTC_INIT)            mx_init.c:96 -> 244  [HAL: RTC_HOURFORMAT_24, AsynchPrediv=127, SynchPrediv=255]
               KM_RTC_SetDefaultTime()              mx_init.c:97 -> 275  [direct register: 2017/12/05 08:30:00 default]
           } else {
               MX_RTC_Init(DEF_RTC_MSPINIT)          mx_init.c:100        [MSP-only re-init after STOP wake]
           }
           KM_RTC_Restore()                        mx_init.c:102 -> 310  [direct register restore from RTC_BKP0R/1R, 10000-iter timeout loop]
           KM_RTC_ALARM_Init()                     mx_init.c:104 -> 352  [HAL: HAL_RTC_SetAlarm_IT, mx_init.c:374]
           MX_TIM3_Init()                          mx_init.c:105 -> 381  [HAL: Prescaler=0, Period=0, internal clock]
           MX_IWDG_Init()                          mx_init.c:106 -> 229  [HAL: Prescaler_256, Window=Reload=4095 -> ~26.2s]
       get_model()                                entry.c:54  -> km_extend_io.c:1480  [reads MODEL_BIT0/1 GPIO once, caches]
       g_checksum_value = checksum_calc()          entry.c:56  -> km_i2c.c:771
       GetVersionString()                          entry.c:60  -> km_i2c.c:800
       BSP_WDT_Start()                             entry.c:63           [BSP -> HAL IWDG]
       internal_sts_init()                         entry.c:64  -> km_extend_io.c:366
       km_it_init()                                entry.c:65  -> km_it.c:364
           init_ei_ref()                            km_it.c:365 -> 72    [zeroes ei_ref[32]]
           BSP_GPIO_Initialize(km_exti_callback)    km_it.c:367          [registers ISR-context GPIO callback]
           BSP_TIM_Initialize(TIMER_3,km_tim_callback) km_it.c:368       [registers ISR-context 1ms callback]
       BSP_TIM_Start(TIMER_3,1)                    entry.c:66           [starts the 1ms hardware tick]
       adc_moni_5v_start_proc()                    entry.c:69           [dead — #if 0 body, km_adc.c:27-40]
       io_extend_memory_write()                    entry.c:71  -> km_extend_io.c:339  [seeds g_io_extend_memory[] from io_extend[14] initial GPIO levels]
       BSP_GPIO_WritePin(_RESET,...RESET)          entry.c:74           [holds S800 in reset]
       s800_power_on()                             entry.c:75  -> km_extend_io.c:607  [begins power-on sequence]
       i2c_recv_first()                            entry.c:77  -> km_i2c.c:317
           Driver_I2C0.Initialize(rx_comp)          km_i2c.c:338         [CMSIS-Driver: registers ISR callback rx_comp]
           Driver_I2C0.Control(MANUAL_ACK_MODE,1)   km_i2c.c:340
           Driver_I2C0.SlaveReceive(rx_buf,16)      km_i2c.c:342         [arms first I2C reception]
       poweroff_timer_init()                       entry.c:80  -> km_extend_io.c:1258 [zeroes 5 power-off-related timers]
       while(1) { ... }                             entry.c:83           [see §5 — steady state begins here]
```

**Who initializes what, in one line each:**
- **Clocks/RCC/PLL:** `mx_init.c::SystemClock_Config()` (`113-159`), called once by `MX_Init()` — owner of `SystemCoreClock` thereafter is HAL (`SystemCoreClock` global, HAL-maintained).
- **GPIO pin modes:** `mx_init.c::MX_GPIO_Init()` (`419-527`) — every pin in `mxconstants.h` gets its mode/pull/speed set here, once, at boot. No other function in the 24 files read this session reconfigures a pin's *mode* afterward (only level, via `BSP_GPIO_WritePin`).
- **I2C1 peripheral registers:** two independent owners at different times — `mx_init.c::MX_I2C1_Init()` (`202-227`) at boot, and `km_i2c.c::i2c_sw_reset()` (`356-363`, which itself calls `MX_I2C1_Init()` again at `362`) and `km_extend_io.c::i2c_reset()` (`1534-1539`, which force-resets via RCC then also calls `MX_I2C1_Init()` at `1538`) on error recovery. **Three call sites own I2C1 re-init: boot, `i2c_sw_reset()`, and `i2c_reset()`** — see §7 for the redundancy this implies.
- **RTC:** owned by `mx_init.c` at boot (`MX_RTC_Init`/`KM_RTC_SetDefaultTime`/`KM_RTC_Restore`/`KM_RTC_ALARM_Init`), but **also written directly by `km_i2c.c`** whenever the main SoC issues an RTC-range I2C write (`km_i2c.c:658-671`) — i.e. RTC register *ownership* is shared between boot-time HAL init and runtime direct-register I2C passthrough, with no locking (single-threaded, so this is safe only because there's no concurrent writer).
- **TIM3:** `mx_init.c::MX_TIM3_Init()` configures it (`381-417`, `Prescaler=0, Period=0` — i.e. HAL leaves the actual 1 ms period to the BSP layer's own prescaler math, `[INFERRED]` from `01_repository_map.md` §3.4's `TIM_PRESCALER=2000`/`TIM_PERIOD=2` constants in `BSP_STM32F03x_Nucleo.h:57-58`, not re-verified against the `.c` body this session); `km_it.c::km_it_init()` (`367-368`) registers the *callback*, and `entry.c:66` is what actually *starts* it counting.
- **IWDG:** `mx_init.c::MX_IWDG_Init()` configures it once (`229-243`); it is **never reconfigured** afterward — only serviced (§5).

### IAP image

```
Reset_Handler (startup_stm32f031x6.s)          [Startup — same file as Main, confirmed via .uvprojx]
  -> SystemInit()
  -> main()                                     entry_iap.c:40
       MX_Init()                                 entry_iap.c:44  -> iap/Src/mx_init.c:69
           HAL_Init()                             (iap mx_init.c body — `[INFERRED]` structurally identical to Main's opening, not verified line-by-line this pass beyond the prototype list at iap/Src/mx_init.c:53-58)
           SystemClock_Config()                   iap/Src/mx_init.c:97   [same RCC/PLL settings as Main per prototype match]
           MX_GPIO_Init()                         iap/Src/mx_init.c:225  [narrower pin set — no RTC/TIM3/ADC pins]
           MX_I2C1_Init()                         iap/Src/mx_init.c:143  [same Timing=0x00200000; NOTE OwnAddress1=124, not 122 — see §7]
           MX_TIM3_Init()                         iap/Src/mx_init.c:185
           MX_IWDG_Init()                         iap/Src/mx_init.c:170
       g_checksum_value = checksum_calc()         entry_iap.c:47 -> km_i2c_iap.c:490
       GetVersionString()                         entry_iap.c:48 -> km_i2c_iap.c:519
       BSP_WDT_Start()                            entry_iap.c:51
       km_it_init_iap()                           entry_iap.c:53 -> km_extend_io_iap.c:78
           BSP_GPIO_Initialize(km_exti_callback_iap) km_extend_io_iap.c:80  [only GPIO callback — no BSP_TIM_Initialize call found: IAP has no software-timer subsystem]
       for(i<48) VectorTable[i] = *(IAP_ADDRESS+i*4)  entry_iap.c:57-60      [copies Main's vector table (IAP_ADDRESS = MAIN_APPLICATION_ADDRESS+MAIN_APPLICATION_SIZE = 0x08005000, km_i2c_iap.h:120) into SRAM @0x20000000 — see §7 for the stale comment this resolves]
       BSP_WDT_Refresh()                          entry_iap.c:63
       __HAL_RCC_SYSCFG_CLK_ENABLE()               entry_iap.c:65
       __HAL_SYSCFG_REMAPMEMORY_SRAM()             entry_iap.c:68          [remaps SRAM to 0x00000000 — required because Cortex-M0 has no VTOR]
       i2c_recv_first()                            entry_iap.c:70 -> km_i2c_iap.c:193
           Driver_I2C0.Initialize(rx_comp)          km_i2c_iap.c:209
           Driver_I2C0.Control(MANUAL_ACK_MODE,1)   km_i2c_iap.c:211
           Driver_I2C0.SlaveReceive(rx_buf,1025)    km_i2c_iap.c:213
           Driver_Flash0.Initialize(NULL)           km_i2c_iap.c:214       [Flash CMSIS-Driver init — Main never does this]
           main_project_flash_erase()               km_i2c_iap.c:216 -> 435  [erases the entire 20KB Main region BEFORE any I2C transaction has occurred]
           g_iap_reg[...CHANGE_STATUS...] |= ...IAP  km_i2c_iap.c:218       [tells the main SoC "I am now in IAP mode" via the register map]
       while(1) { i2c_recv_wait(); BSP_WDT_Refresh(); }  entry_iap.c:73-77
```

**Correction to `01_repository_map.md` §3.13:** that document flagged `entry_iap.c`'s comment `"mapped at the base of the application load address 0x08001600"` (line 56 in the file this session confirms) as possibly conflicting with the confirmed IAP base `0x08005000`. This session confirms the **code**, not just the comment: `IAP_ADDRESS` is `#define`'d at `iap/Inc/km_i2c_iap.h:120` as `MAIN_APPLICATION_ADDRESS + MAIN_APPLICATION_SIZE` = `0x08000000 + 0x5000` = **`0x08005000`**, matching the confirmed IAP ROM base exactly. The `0x08001600` figure in the comment is simply stale/wrong prose left over from an earlier memory-map revision — the macro it describes is correct. This is a documentation bug in the vendor source, not a functional one.

---

## 5. Runtime ownership (steady-state superloop)

### Main image — one full pass of `entry.c:83-144`, in order, with the resource each step touches

| Order | Call (file:line) | Touches (owns/reads) | Hardware behind it |
|---|---|---|---|
| 1 | `sb_reset_proc()` — `entry.c:85` → `km_extend_io.c:1231` | reads MSW/24V11/SB_PG pins + `SB_RESET_RELEASE` timer; writes `_RESET` pin high when ready | GPIO |
| 2 | `moni_24v_proc()` — `entry.c:87` → `km_extend_io.c:575` | `g_power_on_flg` | GPIO (`_DISCHG_Pin`, `[INFERRED]` from naming) |
| 3 | `BSP_WDT_Refresh()` — `entry.c:89` | IWDG counter | IWDG |
| 4 | `msw_on_proc(STATE_NORMAL)` — `entry.c:91` → `km_extend_io.c:1149` | 4 power-off/backup timers; on the `MSWOFF_STOP_MODE` path (`km_extend_io.c:1197-1205`) calls `set_cycle_time(2)`+`BSP_PWR_enter_stopmode()` | GPIO, RTC (Alarm A), PWR |
| 5 | `power_monitor_proc(STATE_NORMAL)` — `entry.c:93` | `g_24v_check_flg_off` | GPIO |
| 6 | `sleep_status_rem_proc(model)` — `entry.c:95` → `km_extend_io.c:1055` (dispatch table `1057`) | `get_ca72_status()` (`896` or `971`), `TYPE_Internal_Ctl0` bits | GPIO (`SLEEP_STATUS_REM_EG`/`_SP`) |
| 7 | `mc_p_on_proc(model)` — `entry.c:97` → `km_extend_io.c:839` (dispatch `731,763,801`) | `_MC_P_ON_OK()` macro (`735-743`) reads 6 GPIO conditions | GPIO (`MC_P_ON`) |
| 8 | `ir_p_on_proc()` — `entry.c:99` → `km_extend_io.c:862` | same 6-condition style gate | GPIO (`IR_P_ON`) |
| 9 | `mc3_3von_moni_proc()` — `entry.c:101` → `km_extend_io.c:425` | (empty body, confirmed) | none |
| 10 | `erp_sensor_proc()` — `entry.c:103` → `km_extend_io.c:687` | `ERP_SENSOR_ON_NUM` bit, `RST_SLP2_NUM` bit | GPIO |
| 11 | `mc_oe_proc()` — `entry.c:105` → `km_extend_io.c:1072` | (empty stub, confirmed) | none |
| 12 | `mc_pwr_en_proc()` — `entry.c:107` → `km_extend_io.c:1096` | `g_power_on_flg`, Eagle-only 320 ms boot timer | GPIO (`MC_PWR_EN`) |
| 13 | `adc_moni_5v_off_proc()` — `entry.c:109` | dead — `#if 0` body, `km_adc.c:54-78` | none |
| 14 | `rst_usb_proc(STATE_NORMAL)` — `entry.c:111` → `km_extend_io.c:711` | (prior-session read, not re-verified this pass) | GPIO |
| 15 | `usb2_oe_proc()` — `entry.c:112` → `km_extend_io.c:1373` | `is_usb2_oe_from_main_cpu_ctrl` | GPIO (`_RST_SLP2`, `_USB2_OE`) |
| 16 | `ap_power_en_proc(STATE_NORMAL)` — `entry.c:114` | (prior-session read) | GPIO, `km_ca72_status.c` writer |
| 17 | `intr_pending_proc()` — `entry.c:116` → `km_extend_io.c:1416` | drains `pending_factor` (set by ISR at `km_it.c:250,253`) | n/a — pure software handoff |
| 18 | `anti_chattering_proc()` — `entry.c:118` → `km_extend_io.c:1438` | iterates `anti_chattering_info[3]` (owned by `km_it.c`), calls back into `msw_on_proc(STATE_INT)` etc. via `info->intr_proc` | GPIO reads via `BSP_GPIO_ReadPin` |
| 19 | `poweroff_timer_proc()` — `entry.c:121` → `km_extend_io.c:1305` | 3 timers (`PWROFF_MAXTIME1/2`, `PWROFF_WDGTIME`) | none directly; can trigger `s800_power_off()` → `HAL_NVIC_SystemReset()` |
| 20 | `backupwait_timer_proc()` — `entry.c:123` → `km_extend_io.c:1350` | `g_backupwait_pending` | none |
| 21 | `i2c_recv_wait()` — `entry.c:126` → `km_i2c.c:485` | `i2c_info.state`, `g_io_extend_memory[]`, RTC registers directly | I2C1 (indirectly, via CMSIS-Driver state already updated by ISR) |
| 22 | `sleep_mode()` — `entry.c:128` → `km_extend_io.c:1395` | reads `pending_factor` (must be 0) | PWR (`BSP_PWR_enter_sleepmode()` @`1402`) |

**Ownership note on the loop itself:** there is exactly one thread of execution (`entry.c::main()`'s `while(1)`); every one of the 18 active `*_proc()` calls above runs to completion before the next begins. **No step can starve another except by taking unbounded time** — and none of the functions read this session contain a loop that isn't itself timer-bounded, except `KM_RTC_Restore()` (boot-only, `10000`-iteration cap) and the busy-wait in `set_cycle_time()`'s 5-iteration `for` loop (`km_alarm_wake.c`, prior-session read).

### IAP image — `entry_iap.c:73-77`

Two calls per pass: `i2c_recv_wait()` (`km_i2c_iap.c:240-411`) and `BSP_WDT_Refresh()`. No debounce, no software timers, no power-sequencing — IAP's only job at runtime is to shuttle I2C payload bytes into flash (`flash_write_32()`, `km_i2c_iap.c:413-432`) and answer status/version reads.

**Runtime hand-back to Main:** `iap/App/km_extend_io_iap.c::msw_on_proc_iap()` (`28-51`), called from the EXTI ISR path via `km_exti_callback_iap()` (`53-72`), checks `MSW_ON` went low **and** `g_iap_reg[...CHANGE_INFO...] & IAP_PG_COMMAND_CHANGE_INFO_COMP` (`km_extend_io_iap.c:35`), then calls `HAL_NVIC_SystemReset()` (`km_extend_io_iap.c:37`) — a full chip reset, which returns execution to address `0x08000000` (Main) under normal STM32 boot-address behavior. This is architecturally distinct from the Main→IAP transition (§0), which is a software jump with no reset.

---

## 6. Hardware ownership (peripheral/pin → owning module)

| Hardware | Owning module (writes/configures it) | Read by | Notes |
|---|---|---|---|
| I2C1 peripheral registers | CMSIS-Driver `I2C_stm32f0xx.c` (via `Driver_I2C0`) | `km_i2c.c`/`km_i2c_iap.c` (indirectly, via driver API) | `mx_init.c::MX_I2C1_Init()` sets the *config* (Timing, addresses); the CMSIS-Driver owns the *runtime* transaction state |
| RTC time/date/alarm registers | Split: `mx_init.c` at boot; `km_i2c.c` directly at runtime (`658-671`) | main SoC (via I2C reads at `km_i2c.c:552-553`) | **Two owners across the firmware's lifetime — see §7** |
| RTC backup registers (`BKP0R-3R`) | `mx_init.c::KM_RTC_Restore()`/`(save_rtc_reg(), dead)`; `km_i2c.c` B-Register write path (`663-673`) | `KM_RTC_Restore()` reads them back at next boot (`mx_init.c:325-326`) | `B_REGISTER_MCIR`/`B_REGISTER_PWROFF_LOG` persistence, per `01_repository_map.md` §3.7 |
| TIM3 | `mx_init.c::MX_TIM3_Init()` (config) + BSP (`BSP_TIM_Start`, actual prescaler math per `01_repository_map.md` §3.4) | `km_it.c::km_tim_callback()` (`312-360`) | Sole 1 ms system tick source |
| IWDG | `mx_init.c::MX_IWDG_Init()` (config, once) | serviced from 3 sites: `entry.c:89` (main loop), `entry_iap.c:63,76` (IAP), never from an ISR | `~26.2s` timeout (`Window=Reload=4095`, `IWDG_PRESCALER_256`) |
| GPIO pin **modes** | `mx_init.c::MX_GPIO_Init()` — exclusive owner, set once at boot | n/a | No other function in the read set reconfigures a pin mode at runtime |
| GPIO pin **levels** | `BSP_GPIO_WritePin()` call sites throughout `km_extend_io.c` (the vast majority), plus `entry.c:74` | `BSP_GPIO_ReadPin()` call sites throughout `km_extend_io.c` and `km_it.c` | Level ownership is diffuse — this is expected for a power-sequencing state machine with dozens of signals |
| NVIC/EXTI enable state (3 lines: `EXTI0_1`, `EXTI2_3`, `EXTI4_15`) | **Two disjoint owners**: `km_it.c::disable/enable_external_irq()` (`90-127`, ref-counted via `km_disable_irq`/`km_enable_irq`, `436-462`) **and** `km_extend_io.c::s800_power_off()` (`538-540`, raw `NVIC_DisableIRQ`, unref-counted) | n/a | Architectural inconsistency — see §7 |
| PWR (STOP/SLEEP) | `km_extend_io.c::msw_on_proc()` (STOP, `1203`) and `sleep_mode()` (SLEEP, `1402`) — both Application-layer, via BSP | n/a | No other caller of `BSP_PWR_enter_*` found in the read set |
| Internal Flash (Main's own 20 KB region) | **IAP image only** — `Driver_Flash0` via `km_i2c_iap.c` (`main_project_flash_erase()`@435, `flash_write_32()`@413) | `checksum_calc()` in both images reads it (`km_i2c.c:771`, `km_i2c_iap.c:490`) | Main never writes its own flash; only IAP does |
| Cortex-M0 vector table | Fixed at `0x08000000` (Main) by hardware boot; **relocated to SRAM `0x20000000`** by IAP (`entry_iap.c:57-60,68`) | NVIC (implicitly, for every interrupt in the IAP image) | Necessary because Cortex-M0 has no VTOR register — the only way to relocate vectors is remap+copy |

---

## 7. Architectural observations (violations and unusual patterns)

1. **Layering violation — Application code bypasses the Driver-layer IRQ-disable abstraction.** `km_it.c` provides a reference-counted IRQ mask/unmask pair, `km_disable_irq()`/`km_enable_irq()` (`km_it.c:436-462`), specifically so nested disable/enable calls compose safely (`ei_ref[32]`, `km_it.c:19`). `disable_external_irq()`/`enable_external_irq()` (`km_it.c:90-127`) use this correctly for the same 3 EXTI lines. But `km_extend_io.c::s800_power_off()` (`523-563`) calls raw `NVIC_DisableIRQ()` directly on the identical 3 lines (`538-540`), **without** going through `km_it.c`'s wrapper — and two lines later calls `init_ei_ref()` (`549`) to zero the very reference-count array that its own raw call just skipped incrementing/decrementing. In practice this is harmless *only* because `s800_power_off()` ends in `HAL_NVIC_SystemReset()` (`560`) a few lines later, so the mismatched ref-count never has a chance to cause a stuck-disabled IRQ line — but it is a real inconsistency: two different idioms for disabling the same hardware exist side-by-side in the codebase, and only one of them is reflected in the reference-count state that the rest of the firmware trusts.

2. **HAL-bypass pattern is pervasive and intentional for the RTC, but not for I2C1 reset.** `km_i2c.c` reads and writes RTC registers by casting `RTC_BASE + offset` directly (`552-553` read, `658-660` and `666-671` write) instead of calling `HAL_RTC_GetTime`/`SetTime`/`SetAlarm`. This is a deliberate design choice (the I2C protocol *is* a raw register-address-to-value mapping, so this is the natural implementation), and `RTC_WRITEPROTECT_DISABLE()`/`ENABLE()` macros (`km_i2c.h`, prior-session read) correctly bracket every write. By contrast, `km_i2c.c::i2c_sw_reset()` (`356-363`) uses `__HAL_RCC_I2C1_FORCE_RESET()`/`RELEASE_RESET()` (RCC-level force-reset, bypassing the CMSIS-Driver's own `Uninitialize()`/`Initialize()` life-cycle for the *reset* half but still calling `Driver_I2C0.Uninitialize()` at `361`) — a **third, heavier** I2C1-recovery path exists at `km_extend_io.c::i2c_reset()` (`1534-1539`), which does the same RCC force-reset **and** `MX_I2C1_Init()`, but is a completely separate function from `i2c_sw_reset()` with no call relationship between them `[UNKNOWN — no caller of km_extend_io.c::i2c_reset() was found in the 24 files read this session; it may be dead code, or called from a file not yet read]`.

3. **Two initialization owners for the same peripheral, active at different times, with no synchronization primitive between them (none needed, single-threaded) — but no clear single "owner" either.** RTC is configured by `mx_init.c` at boot and then treated as a plain memory-mapped register file by `km_i2c.c` at runtime. This works only because the firmware is single-threaded and the RTC hardware itself serializes access; a reviewer looking for "who owns the RTC" cannot point to one file.

4. **Model detection and generation detection use two unrelated mechanisms, easy to conflate.** `get_model()` (`km_extend_io.c:1480-1502`) reads two GPIO straps (`MODEL_BIT0`/`MODEL_BIT1`) **once** at first call and caches the result (`static int is_first`, `1482`) — a hardware-strap, boot-time-only signal. `get_Generation()` (`km_extend_io.c:1514-1522`) instead reads an I2C-writable bit, `INTERNEAL_STS_COMMAND_MODEL_TYPE` in `g_io_extend_memory[TYPE_Internal_Sts0]` — a runtime, main-SoC-controlled signal that can change at any time. Both functions return a "what kind of hardware is this" answer, but one is immutable-after-boot and the other is live and externally writable; nothing in the naming distinguishes this, and a maintainer changing one code path for "model" reasons could easily reach for the wrong function.

5. **Redundant/apparently-defensive flash erase in the IAP image.** `km_i2c_iap.c::i2c_recv_first()` erases the entire Main application region unconditionally at IAP boot (`main_project_flash_erase()`, `216`), before any I2C transaction has occurred. The dispatcher's own handling of the magic-number write (`i2c_recv_wait()`, `case IAP_PG_COMMAND:`, confirmed structurally analogous to Main's `km_i2c.c:686-693` pattern but in the IAP file) also triggers `main_project_flash_erase()` a second time per the pattern documented in `01_repository_map.md` §3.14 — `[the exact second call site inside km_i2c_iap.c's IAP_PG_COMMAND branch was not re-read line-by-line this session; flagging per the prior finding rather than re-asserting a line number]`. Whether double-erase is intentional (defensive idempotency) or accidental could not be determined from source.

6. **Dead code with live call sites (not merely unreferenced — actively called every loop iteration).** `km_adc.c::adc_moni_5v_start_proc()`/`adc_moni_5v_off_proc()` (`27-78`) are both wrapped in `#if 0`, so their bodies compile to nothing, yet `entry.c:69` and `entry.c:109` call them unconditionally on every boot/every loop iteration respectively. This costs nothing at runtime (empty function calls optimize away or are trivially cheap) but is a code-hygiene signal: the call sites were never cleaned up when the feature was disabled, and `km_adc.h`'s `TEST_BOARD` macro (confirmed active, per `01_repository_map.md` §3.11) would silently apply test-board ADC thresholds if anyone ever flips the `#if 0` back on without revisiting that macro.

7. **All interrupts share one NVIC priority level.** Every `HAL_NVIC_SetPriority()` call found across `mx_init.c` (`509,512,515`) and `stm32f0xx_hal_msp.c` (the `(x,0,0)` pattern at each `MspInit`) uses priority `0`. On Cortex-M0 (no BASEPRI, only a single active-priority comparison), this means no interrupt can preempt another that is already running — an EXTI edge arriving during I2C1's ISR will simply wait its turn (tail-chained), never nest. This is a valid, common design for simple bare-metal firmware, but it means the `1 ms` timer tick (`TIM3_IRQHandler`) can itself be delayed by a long-running I2C ISR, which given the timers are used for both debounce (10-30 ms windows, `km_it.h` `KM_ANTI_CHATTER_*` constants) and multi-second power-off sequencing (`PWROFF_SEQTIME=9350` ms) is unlikely to matter in practice but is a real, uncosted latency source `[INFERRED — no measurement or worst-case-ISR-length analysis was performed or found in source]`.

8. **The Main→IAP transition is a software jump, not a reset; the IAP→Main transition is a hardware reset.** Confirmed asymmetry: `jump2iap()` (`entry.c:159-172`) manually sets `MSP` and branches to the IAP reset-vector address without ever going through `Reset_Handler` — meaning IAP's `.data`/`.bss` are whatever startup left them from *Main's* run, until IAP's own (separately-linked) `Reset_Handler` executes as part of that jump `[INFERRED — jump2iap() branches to the address stored at IAP_DEFAULT_ADD+4, which per standard Cortex-M vector table layout is the IAP image's own Reset_Handler, so C-runtime init (.data copy, .bss zero) does happen, just without a CPU-level reset; the SP is explicitly reloaded from IAP_DEFAULT_ADD at entry.c:169]`. The return path (`km_extend_io_iap.c:37`) uses `HAL_NVIC_SystemReset()` — a full CPU reset. So the same logical "switch firmware image" operation is implemented two different ways depending on direction, each with different guarantees about residual RAM state.

9. **`km_i2c.c::save_rtc_reg()` is dead code with a live comment referencing it.** The function (`km_i2c.c:74-77`) writes `RTC->TR`/`RTC->DR` to backup addresses but has no caller anywhere in the read set. `km_i2c.c:690`'s comment explicitly says `/* save_rtc_reg(); 2023/06/30 ... 不要 */` ("...no longer needed") — i.e. this dead function is a **known**, intentionally-retired call site, not an oversight, confirmed by the inline commit-history comment.

10. **Confidentiality reminder (carried from `00_project_scope.md`):** `MFP_Study` is a public GitHub repository. This document reproduces short illustrative excerpts (≤10 lines) with line-number citations for traceability, consistent with what is already committed; it does not paste any file in full.

---

## 8. Summary: boundary map

| Boundary | Where it sits | Enforced how |
|---|---|---|
| Hardware ↔ MCU Peripheral | Register bit-fields vs. named registers | Naming only (`RTC->TR` vs `RTC_BASE+offset` are the same memory, different access idiom) |
| MCU Peripheral/HAL ↔ BSP | `common/Drivers/BSP/*.c` wraps `HAL_GPIO_*`/`HAL_TIM_*`/`HAL_IWDG_*` `[INFERRED, bodies unread]` | Header-only (`BSP_STM32F03x_Nucleo.h`) — nothing prevents an App file from calling HAL directly, and `km_i2c.c`/`km_extend_io.c` do so for RCC/NVIC/RTC (see Observation 2) |
| BSP ↔ Driver/App | `km_it.c`, `km_alarm_wake.c` call `BSP_*` functions only, never raw HAL — **clean boundary**, confirmed by grep across both files: no `HAL_` prefix found outside `km_it.c:284` (`HAL_RTC_AlarmAEventCallback`, a HAL-defined callback signature, not a call) and `km_it.c:271` (`HAL_RTC_DeactivateAlarm`, one exception) |
| Driver (CMSIS) ↔ Protocol | `Driver_I2C0`/`Driver_Flash0` calls confined to `km_i2c.c`/`km_i2c_iap.c`/`entry.c` (the `extern` declaration and `DeInit()`'s `Uninitialize()` call) — **clean**, no other file references `Driver_I2C0` |
| Protocol ↔ Application | `km_i2c.c` calls into `km_extend_io.c` only via `io_extend_read()`/`io_extend_write()` and `setEventRecord()`'s counterpart calls **the other way** (`km_extend_io.c` calls `km_i2c.c::setEventRecord()`) — a **bidirectional** boundary, not the one-way Protocol→Application flow a layered diagram would suggest |
| Application ↔ RTOS | N/A — no RTOS exists; the Application layer *is* the scheduler (a fixed-order superloop) |
| Application ↔ Middleware | N/A — no middleware exists |
