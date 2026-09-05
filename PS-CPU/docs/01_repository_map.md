# PS-CPU (pscpu_s800) — Repository Map

**Status:** Source-grounded module/file map of the firmware tree.
**Analysis mode:** Same convention as `00_project_scope.md`. Every claim is either backed by a specific file this session actually opened (and is stated as plain fact, no tag), or explicitly tagged `[INFERRED]`, `[HARDWARE ASSUMPTION]`, or `[UNKNOWN]`. Nothing is silently invented. Read `00_project_scope.md` §0 first — it explains that the real source lives in Google Drive folder `PS-CPU/pscpu_s800/`, not in this git repo, and describes the duplicate-folder caveat that also applies here.

Files opened in full (base64-decoded and read end-to-end) to produce this document, in addition to everything `00_project_scope.md` already lists:

- `main/App/km_extend_io.c` (1540 lines — the full file)
- `main/App/km_i2c.c`, `main/App/km_it.c`, `main/App/km_alarm_wake.c`, `main/App/km_ca72_status.c`, `main/App/km_adc.c` (full)
- `main/App/entry.c` (full, including `jump2iap()` and `DeInit()`)
- `main/Inc/km_extend_io.h`, `km_i2c.h`, `km_it.h`, `mxconstants.h`, `km_ver.h`, `km_adc.h`, `km_ca72_status.h`, `km_alarm_wake.h`, `entry.h` (full)
- `main/Src/mx_init.c`, `stm32f0xx_it.c`, `stm32f0xx_hal_msp.c` (full)
- `main/MDK-ARM/PowerSubCPU.uvprojx`, `iap/MDK-ARM/PowerSubCPU_iap.uvprojx` (full XML — device, defines, OCR memory map, and the exact per-Group compiled-file lists)
- `iap/App/entry_iap.c`, `km_extend_io_iap.c`, `km_i2c_iap.c` (full)
- `iap/Inc/km_i2c_iap.h`, `km_ver_iap.h`, `entry_iap.h` (full)
- `common/Drivers/BSP/Inc/BSP_STM32F03x_Nucleo.h` (full — the BSP public API)
- `common/Test/cubemx/.../entry.c` (the third, standalone hardware-test Keil project's `main()` — found incidentally while searching for `main/App/entry.c`)

Not opened this session (named and sized only, or downloaded-but-unread from a prior pass): the 6 `BSP_*.c` implementation files, the IAP image's `mx_init.c`/`stm32f0xx_it.c`/`stm32f0xx_hal_msp.c`/`system_stm32f0xx.c`/`mxconstants.h`/`stm32f0xx_hal_conf.h`, and every file under ST's `STM32F0xx_HAL_Driver`. These are described below from the `.uvprojx` compiled-file lists, from the public headers they implement, and from how their entry points are called at every call site this session did read — tagged `[INFERRED]` wherever the description goes beyond that.

---

## 0. How to read this document

Each module section reports, where the evidence supports it:
**Purpose · Responsibility · Classification · Public APIs · Internal APIs · Important source files · Important header files · Dependencies · Callers · Callees · Hardware dependencies · RTOS dependencies · Configuration dependencies.**

Classification is one of the 12 categories requested: `Startup, BSP, HAL, Driver, RTOS, Middleware, Protocol, Service, Application, Utility, Configuration, Generated code`. A module can legitimately straddle two categories (e.g. the CubeMX-generated `Src/*.c` files are simultaneously "Generated code" and de-facto "Configuration"); this is called out per module rather than forced into one box.

**RTOS, up front:** this firmware has **no RTOS**. `main/MDK-ARM/PowerSubCPU.uvprojx`'s `<RTE>` component list contains only `ARM::CMSIS:CORE`; no CMSIS-RTOS2, no FreeRTOS/RTX/ThreadX component or source file appears in any `.uvprojx` Group, and no `osKernelStart`, `osThreadNew`, `xTaskCreate`, or similar call appears in any file this session read. Both images are a single bare-metal `main()` with a fixed-order `while(1)` superloop (see §5, "Find:"). Every module's "RTOS dependencies" field below is therefore **None (confirmed)** unless stated otherwise, and no module is classified `RTOS`.

---

## 1. Repository layout (as read from Drive; not present in this git repo)

```
pscpu_s800/
├── main/                          Main application firmware image (ROM 0x08000000, 20 KB)
│   ├── App/                       Hand-written application logic
│   ├── Inc/                       Hand-written + CubeMX-generated headers
│   ├── Src/                       CubeMX-generated init/ISR/MSP sources
│   └── MDK-ARM/                   Keil project (PowerSubCPU.uvprojx) — the Main build manifest
├── iap/                           In-Application-Programming (field firmware update) image (ROM 0x08005000, 12 KB)
│   ├── App/                       Hand-written IAP application logic
│   ├── Inc/                       Headers (IAP variants)
│   ├── Src/                       CubeMX-generated init/ISR/MSP sources (IAP variant)
│   └── MDK-ARM/                   Keil project (PowerSubCPU_iap.uvprojx) — the IAP build manifest
├── common/                        Code shared by both Main and IAP builds
│   ├── Drivers/
│   │   ├── CMSIS/Device/ST/STM32F0xx/
│   │   │   ├── Source/            system_stm32f0xx.c, I2C_stm32f0xx.c, FLASH_stm32f0xx.c (CMSIS-Driver layer)
│   │   │   └── Include/           (device headers — not opened)
│   │   ├── BSP/{Inc,Src}/         KM's own board-support layer (BSP_GPIO/TIM/WDT/PWR/ADC/Reg)
│   │   └── STM32F0xx_HAL_Driver/  [INFERRED path] ST's Cube HAL library (stm32f0xx_hal_*.c) — standard
│   │                              CubeMX layout, by analogy with the two sibling paths above, which
│   │                              were confirmed directly; the exact folder name was not re-verified
│   │                              this session (see §3 "HAL" module).
│   ├── Test/cubemx/               A third, standalone Keil project/hardware test harness (own `main()`,
│   │                              GPIO/ADC/USART/Timer/I2C/I2C-slave self-tests) — not part of production
│   │                              firmware; referenced from the multi-project `.uvmpw` per `00_project_scope.md`.
│   └── Tool/                      [UNKNOWN] — named only in the directory listing behind `00_project_scope.md`,
│                                  contents not opened either session.
├── scripts/                       [UNKNOWN] — named only, not opened.
├── Version.txt, cubemx.patch, PowerSubCPU.uvmpw   Root-level build/version files (`00_project_scope.md` §3)
```

---

## 2. Module inventory (quick index)

| # | Module | Classification | One-line purpose |
|---|---|---|---|
| 1 | Startup & CMSIS system init | Startup | Reset→C runtime→`main()` handoff; clock/vector-table bring-up before `main()` |
| 2 | STM32 HAL library (`STM32F0xx_HAL_Driver`) | HAL | ST's peripheral register abstraction (GPIO/I2C/RTC/TIM/IWDG/PWR/RCC/Flash/…) |
| 3 | CMSIS-Driver layer (I2C, Flash) | Driver | `ARM_DRIVER_I2C`/`ARM_DRIVER_FLASH` implementations used by both images |
| 4 | BSP (board support) | BSP | KM's own thin API over HAL: `BSP_GPIO_*`, `BSP_TIM_*`, `BSP_WDT_*`, `BSP_PWR_*`, `BSP_ADC_*`, `BSP_Reg_*` |
| 5 | CubeMX-generated init/ISR/MSP (`Src/*.c`, per image) | Generated code / Configuration | `MX_Init`, ISR vector bodies, `HAL_*_MspInit` |
| 6 | Keil project files (`.uvprojx`) & generated headers (`mxconstants.h`, `stm32f0xx_hal_conf.h`) | Configuration | Device target, memory map, pin map, HAL feature toggles, exact compiled-file lists |
| 7 | Main I2C protocol engine (`km_i2c.c`) | Protocol | I2C-slave register-map protocol; PS-CPU's entire external API surface |
| 8 | Main interrupt/timer/debounce subsystem (`km_it.c`) | Driver | Software timers, pending-factor deferral, ref-counted IRQ mask, GPIO debounce |
| 9 | RTC alarm/wake driver (`km_alarm_wake.c`) | Driver | Configures RTC Alarm A for STOP-mode wake |
| 10 | CA72 status tracker (`km_ca72_status.c`) | Service | Tiny cached state (`OFF`/`ON`/`SLEEP`) of the main SoC, set from I2C, read by power logic |
| 11 | ADC monitor (`km_adc.c`) | Utility (dead code) | 5V-rail ADC monitoring — fully `#if 0`'d out, confirmed unused |
| 12 | Main power-sequencing application (`km_extend_io.c` + `entry.c`) | Application | The core business logic: I/O-extension memory map, power-on/off sequencing, sleep/wake, model/generation dispatch |
| 13 | IAP entry/superloop (`entry_iap.c`) | Application | IAP image's `main()`: vector-table relocation to SRAM, then `i2c_recv_wait()` loop |
| 14 | IAP I2C/flash-programming engine (`km_i2c_iap.c`) | Protocol | Simplified I2C register map for receiving and flashing new firmware images |
| 15 | IAP GPIO glue (`km_extend_io_iap.c`) | Application | MSW_ON-triggered system reset once an update completes |
| 16 | Hardware self-test harness (`common/Test/cubemx`) | Application (test) | Standalone bring-up/self-test firmware, not shipped |

---

## 3. Per-module detail

### 3.1 Startup & CMSIS system init

- **Purpose:** Get from reset to a running C environment and into `main()`, for *either* image (both images build the same file).
- **Responsibility:** CPU vector table, reset handler, stack/heap init, calling `SystemInit()` then `__main`/`main()`. `system_stm32f0xx.c`'s `SystemInit()` additionally sets `SystemCoreClock` and (per standard CMSIS device templates) leaves clock configuration itself to `SystemClock_Config()` in `mx_init.c` — `[INFERRED]`, the body of `SystemInit()` was not read this session.
- **Classification:** Startup.
- **Important source files:** `startup_stm32f031x6.s` (10,652 or 10,667 bytes depending on duplicate-batch copy — see directory-listing caveat), `common/Drivers/CMSIS/Device/ST/STM32F0xx/Source/system_stm32f0xx.c`.
- **Confirmed via `.uvprojx`:** both `main/MDK-ARM/PowerSubCPU.uvprojx` and `iap/MDK-ARM/PowerSubCPU_iap.uvprojx` list **only** `startup_stm32f031x6.s` in their `Application/MDK-ARM` Group. The sibling file `startup_stm32f030x8.s`, present in both MDK-ARM folders on disk, is **not** in either Group's file list — it is dead/unused. This resolves the `[UNKNOWN]` flagged in `00_project_scope.md` §1.
- **Important header files:** device headers under `CMSIS/Device/ST/STM32F0xx/Include/` — `[UNKNOWN]`, not opened.
- **Public API:** the reset vector / entry symbol consumed by the linker (`Reset_Handler`) — `[INFERRED]`, standard CMSIS startup naming, the `.s` file's exact label list was not read.
- **Internal APIs:** NMI/Fault/exception default handlers (weak, overridden per-image by `stm32f0xx_it.c` — see §3.5).
- **Dependencies:** none (bottom of the stack).
- **Callers:** the CPU itself (hardware reset vector fetch); not called by any application code.
- **Callees:** `SystemInit()`, then `main()` (per-image, different `main()` — see §5).
- **Hardware dependencies:** Cortex-M0 core (ARMv6-M), reset/boot behavior of STM32F031C6.
- **RTOS dependencies:** None (confirmed).
- **Configuration dependencies:** `<Device>STM32F031C6</Device>` in the `.uvprojx`; no scatter file (`<ScatterFile>` empty in both projects) — linking is by Keil's simple ROM/RAM-region model (`OCR_RVCT4`), not an explicit `.sct`.

### 3.2 STM32 HAL library (`STM32F0xx_HAL_Driver`)

- **Purpose:** ST's standard peripheral-register abstraction (`HAL_GPIO_*`, `HAL_I2C_*`, `HAL_RTC_*`, `HAL_TIM_*`, `HAL_IWDG_*`, `HAL_PWR_*`, `HAL_RCC_*`, `HAL_FLASH_*`, `HAL_NVIC_*`, etc.), consumed by `mx_init.c`, `stm32f0xx_hal_msp.c`, `stm32f0xx_it.c`, and the BSP layer.
- **Responsibility:** Register-level peripheral configuration and control; not touched directly by application (`App/`) code, which goes through BSP or (for I2C/Flash) CMSIS-Driver instead.
- **Classification:** HAL.
- **Important source files:** `[INFERRED path]` `common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_*.c` — the directory name itself was not independently re-confirmed this session (see §1); what *is* confirmed is that `.uvprojx`'s `HAL Driver` Group compiles **19 files** for Main and **15 files** for IAP, and that Main's 19 include ADC-related HAL sources that are dead weight (`km_adc.c`, the only caller, is `#if 0`'d out — see §3.11), while IAP's 15 omit ADC and RTC HAL sources entirely (IAP has no RTC and no ADC use). Exact per-file names in each Group were seen during the full `.uvprojx` read but not retained verbatim into this document; treat the file *count* and *ADC/RTC inclusion* facts as confirmed, the individual filenames as `[UNKNOWN]` pending a re-read of the raw XML.
- **Important header files:** `[INFERRED]` `stm32f0xx_hal_conf.h` (confirmed to exist and be read in `main/Inc/` per `00_project_scope.md`; defines `HSI_VALUE=8000000`, `HSE_VALUE=8000000`, `LSI_VALUE=40000`, `LSE_VALUE=32768`) toggles which `HAL_MODULE_ENABLED` macros are active and therefore which HAL source files actually compile in.
- **Public API:** the standard `HAL_*` function family — consumed, not defined, by everything above it.
- **Dependencies:** CMSIS core/device headers (§3.1).
- **Callers:** `mx_init.c`, `stm32f0xx_hal_msp.c`, `stm32f0xx_it.c` (all Generated code, §3.5); `common/Drivers/BSP/Src/*.c` (§3.4, by strong inference — BSP's job is specifically to wrap HAL); `I2C_stm32f0xx.c`/`FLASH_stm32f0xx.c` (§3.3, CMSIS-Driver layer sits on top of HAL for the actual register access) — `[INFERRED]` for the last two, their bodies were not read, only their public headers/behavior via callers.
- **Callees:** none above CMSIS.
- **Hardware dependencies:** every on-chip peripheral used by this firmware (GPIO, I2C1, RTC, TIM3, IWDG, PWR, RCC, Flash, SYSCFG, NVIC/EXTI).
- **RTOS dependencies:** None.
- **Configuration dependencies:** `stm32f0xx_hal_conf.h` module-enable macros; `USE_HAL_DRIVER` preprocessor define (confirmed in both `.uvprojx` `<Define>` fields).

### 3.3 CMSIS-Driver layer (I2C, Flash)

- **Purpose:** Provide the ARM CMSIS-Driver standard interface (`ARM_DRIVER_I2C`, `ARM_DRIVER_FLASH`) so the application-layer protocol code (`km_i2c.c`/`km_i2c_iap.c`) can drive I2C1 as a **slave** and the internal Flash as a **programmer**, through a vendor-neutral callback-based API rather than raw HAL calls.
- **Responsibility:** I2C1 slave transaction state machine (address match, byte-by-byte RX/TX, manual ACK/NACK), and Flash unlock/erase/program primitives.
- **Classification:** Driver.
- **Important source files:** `common/Drivers/CMSIS/Device/ST/STM32F0xx/Source/I2C_stm32f0xx.c`, `FLASH_stm32f0xx.c` — both confirmed present in **both** `.uvprojx` Drivers/CMSIS Groups (Main uses I2C actively and links Flash unused by Main's own code but present in the Group; IAP uses both actively for its flash-write protocol).
- **Important header files:** `I2C_stm32f0xx.h` (included by `entry.c` and `km_i2c.c`) — declares `extern ARM_DRIVER_I2C Driver_I2C0;` and the vendor extension control codes `KM_ARM_I2C_GENERATE_NACK`/`KM_ARM_I2C_GENERATE_ACK`/`KM_ARM_I2C_MANUAL_ACK_MODE` referenced from `km_i2c.c`/`km_i2c_iap.c` (per `00_project_scope.md`).
- **Public API:** `Driver_I2C0` (the `ARM_DRIVER_I2C` instance — `Initialize/Uninitialize/PowerControl/SlaveTransmit/SlaveReceive/GetDataCount/Control/…`), plus an equivalent `ARM_DRIVER_FLASH` instance for Flash (name not directly observed this session — `[INFERRED]` `Driver_Flash0` by symmetry with `Driver_I2C0` and the file's own name `FLASH_stm32f0xx.c`).
- **Internal APIs:** `rx_comp()` in `km_i2c.c` is registered as the driver's `ARM_I2C_SignalEvent_t` ISR-context callback (confirmed — it is called from interrupt context per `00_project_scope.md`'s prior analysis and this session's `km_i2c.c` read).
- **Dependencies:** STM32 HAL (§3.2) for underlying register access — `[INFERRED]`.
- **Callers:** `km_i2c.c::i2c_recv_first()`/`i2c_sw_reset()`/`i2c_reset()` (Main), `km_i2c_iap.c::i2c_recv_first()` (IAP) — both call `Driver_I2C0.Initialize(...)` style setup; `km_extend_io.c::i2c_reset()` also does `__HAL_RCC_I2C1_FORCE_RESET()`/`RELEASE_RESET()` directly (bypassing the driver) plus `MX_I2C1_Init()`.
- **Callees:** the I2C1 hardware peripheral's ISR (`I2C1_IRQHandler` in `stm32f0xx_it.c` ultimately dispatches into this driver's interrupt handling — `[INFERRED]`, the exact call chain inside `I2C_stm32f0xx.c` was not read).
- **Hardware dependencies:** I2C1 peripheral (PB6/PB7, per `stm32f0xx_hal_msp.c`'s `HAL_I2C_MspInit`), internal Flash controller.
- **RTOS dependencies:** None.
- **Configuration dependencies:** `MX_I2C1_Init()`'s `Timing=0x00200000` register value, slave addresses 122/124 (`mx_init.c`, confirmed).

### 3.4 BSP (board support)

- **Purpose:** KM's own thin hardware-abstraction layer, one level above ST's HAL, used by essentially all application code instead of calling `HAL_GPIO_*`/`HAL_TIM_*`/etc. directly.
- **Responsibility:** GPIO read/write/direction, a generic timer facade over TIM3/14/15/16/17, watchdog start/refresh, low-power mode entry (STOP/SLEEP), ADC channel start/read/stop, and raw register read/write/bit-set/bit-clear helpers.
- **Classification:** BSP.
- **Public API** (confirmed verbatim from `common/Drivers/BSP/Inc/BSP_STM32F03x_Nucleo.h`, the full header this session read):
  ```c
  ErrType      BSP_GPIO_Initialize(BSP_GPIO_SignalEvent_t handler);
  void         BSP_GPIO_Uninitialize(void);
  void         BSP_GPIO_SetDirection(PortType* portno, PinType pinno, DirectionType dir);
  PinType      BSP_GPIO_ReadPort(PortType* portno);
  void         BSP_GPIO_WritePort(PortType* portno, PinType value);
  PinStateType BSP_GPIO_ReadPin(PortType* portno, PinType pinno);
  void         BSP_GPIO_WritePin(PortType* portno, PinType pinno, PinStateType state);

  RegType      BSP_Reg_Read(RegType* addr);
  RegType      BSP_Reg_ReadBit(RegType* addr, RegType mask);
  void         BSP_Reg_Write(__IO RegType* addr, RegType value);
  void         BSP_Reg_SetBit(__IO RegType* addr, RegType mask);
  void         BSP_Reg_ClearBit(__IO RegType* addr, RegType mask);

  ErrType      BSP_TIM_Initialize(TimerType type, BSP_TIM_SignalEvent_t handler);
  void         BSP_TIM_Uninitialize(TimerType type);
  ErrType      BSP_TIM_Start(TimerType type, MillsecType msec);
  ErrType      BSP_TIM_Stop(TimerType type);

  ErrType      BSP_WDT_Start(void);
  ErrType      BSP_WDT_Refresh(void);

  void         BSP_PWR_enter_stopmode(void);
  void         BSP_PWR_enter_sleepmode(void);

  ErrType      BSP_ADC_Start(ADCChType ch);
  uint32_t     BSP_ADC_GetValue(ADCChType ch);
  ErrType      BSP_ADC_Stop(ADCChType ch);

  #define BSP_GPIO_EXTI_Callback HAL_GPIO_EXTI_Callback   /* aliases the HAL weak callback directly */
  ```
  Notable types from the same header: `TimerType` enum `{TIMER_3, TIMER_14, TIMER_15, TIMER_16, TIMER_17, TIMER_END}`; `ADCChType` enum `ADC_CH_0..15`; error-code bases `BSP_GPIO_ERR_BASE/BSP_TIM_ERR_BASE/BSP_BASE_ERR_BASE/BSP_ADC_ERR_BASE/BSP_RTC_ERR_BASE/BSP_WDT_ERR_BASE`.
- **Important source files:** `common/Drivers/BSP/Src/BSP_GPIO_STM32F03x_Nucleo.c` (4,488 B), `BSP_TIM_STM32F03x_Nucleo.c` (5,054 B), `BSP_WDT_STM32F03x_Nucleo.c` (1,390 B), `BSP_PWR_STM32F03x_Nucleo.c` (1,667 B), `BSP_ADC_STM32F03x_Nucleo.c` (2,390 B), `BSP_Reg_STM32F03x_Nucleo.c` (1,617 B). **None of these six implementation files were opened this session** — their bodies are `[UNKNOWN]`; only their declared public API (above) and call sites in `App/` code are confirmed.
- **Important header files:** `BSP_STM32F03x_Nucleo.h` (above, full contents confirmed; header comment attributes it to "2016 Konicaminolta... Kunihiro Miwa / Hirofumi Hashimoto").
- **Confirmed compiled-file membership per `.uvprojx`:** Main's `Drivers/BSP` Group has **5** files (all six except `BSP_ADC`); IAP's `Drivers/BSP` Group has **4** files (all six except `BSP_ADC` *and* `BSP_PWR` — consistent with IAP never entering STOP/SLEEP mode, only erasing/writing flash and resetting).
- **Dependencies:** STM32 HAL (§3.2) — `[INFERRED]`, not directly verified since the `.c` bodies weren't read, but the header includes `stm32f0xx_hal.h`/`_gpio.h`/`_tim.h`/`_iwdg.h`, strongly implying HAL calls underneath.
- **Callers:** virtually every `App/` file in both images: `km_extend_io.c` (`BSP_GPIO_ReadPin`/`WritePin` throughout, `BSP_PWR_enter_sleepmode`/`stopmode`), `entry.c` (`BSP_WDT_Start`/`Refresh`, `BSP_TIM_Start(TIMER_3,1)`), `km_it.c` (debounce reads GPIO via `get_anti_chattering_info()`'s port/pin, then `BSP_GPIO_ReadPin`), `entry_iap.c`/`km_i2c_iap.c` (`BSP_WDT_Start`/`Refresh`).
- **Callees:** HAL (`[INFERRED]`).
- **Hardware dependencies:** GPIO ports A/B/F, TIM3 (and nominally TIM14/15/16/17, though only `TIMER_3` is ever passed to `BSP_TIM_Start` in the code this session read), IWDG, PWR (STOP/SLEEP entry), ADC1 (only reachable if `km_adc.c` were re-enabled — currently dead, §3.11).
- **RTOS dependencies:** None.
- **Configuration dependencies:** `TIM_PRESCALER=2000`, `TIM_PERIOD=2` (header-level constants tuning the generic timer facade to a 1 ms tick at an implied 4 MHz timer clock — `[INFERRED]` from `2000 * 2 = 4000` counts/ms → 4 MHz, not independently cross-checked against `mx_init.c`'s actual TIM3 prescaler register value).

### 3.5 CubeMX-generated init/ISR/MSP (`Src/*.c`, per image)

- **Purpose:** Peripheral bring-up (clocks, GPIO modes, RTC, IWDG, TIM3), the concrete ISR vector table bodies, and HAL's per-peripheral MSP (MCU Support Package) init/deinit hooks — the code STM32CubeMX emits from the `.ioc` pin/clock configuration, then hand-patched by KM engineers (confirmed by non-generated-looking application logic embedded in it, e.g. `KM_RTC_Restore()`'s timeout loop).
- **Responsibility:** Everything that must run once, early, to make the chip's peripherals match the pin/clock plan, plus routing every enabled interrupt to the right HAL handler.
- **Classification:** Generated code (with hand-added Configuration-flavored logic).
- **Important source files (Main, all read in full this session):**
  - `main/Src/mx_init.c` — `MX_Init()` (top-level orchestrator), `SystemClock_Config()` / `SystemClock_Config2()` (identical except `SystemClock_Config` calls `Error_Handler()` on failure while `SystemClock_Config2` just `return`s — the latter is the "safe to call after waking from STOP, PLL might legitimately need re-lock without hanging forever" variant), `MX_I2C1_Init()` (`Timing=0x00200000`, own address `122`, general-call/secondary address `124`), `MX_IWDG_Init()` (`Prescaler=IWDG_PRESCALER_256`, `Window=Reload=4095` → ≈26.2 s timeout), `MX_RTC_Init(char sc_mode)` (two-mode: `DEF_RTC_INIT` full init vs `DEF_RTC_MSPINIT` MSP-only, used to re-arm RTC after STOP-mode wake without a full re-init), `KM_RTC_SetDefaultTime()`, `KM_RTC_Restore()` (polls `RTC->ISR` with a `loop_limit=10000` timeout guard — a hand-added robustness patch, not stock CubeMX output), `KM_RTC_ALARM_Init()`, `MX_TIM3_Init()`, `MX_GPIO_Init()` (full pin-mode configuration for every port), `Error_Handler()` (infinite loop — no recovery), `assert_failed()` (empty body).
  - `main/Src/stm32f0xx_it.c` — the concrete ISR bodies (see §5 "interrupt handlers").
  - `main/Src/stm32f0xx_hal_msp.c` — `HAL_MspInit`, `HAL_I2C_MspInit`/`MspDeInit` (configures PB6/PB7 as I2C1 AF, open-drain), `HAL_RTC_MspInit`/`MspDeInit`, `HAL_TIM_Base_MspInit`/`MspDeInit`.
  - `common/Drivers/CMSIS/Device/ST/STM32F0xx/Source/system_stm32f0xx.c` — `SystemInit()`/`SystemCoreClock` (compiled from this shared path into **both** images per each `.uvprojx`'s Drivers/CMSIS Group).
- **Important source files (IAP, downloaded but not decoded this session — `[UNKNOWN]` bodies):** `iap/Src/mx_init.c` (8,901 B), `stm32f0xx_it.c` (5,653 B), `stm32f0xx_hal_msp.c` (5,313 B), `system_stm32f0xx.c` (13,095 B, shared path). What **is** confirmed about IAP's variant: its `HAL Driver` Group (15 files) excludes ADC and RTC sources entirely, consistent with `entry_iap.c`'s `main()` never touching RTC or ADC, and with IAP's BSP Group excluding `BSP_PWR` (§3.4). `entry_iap.c` calls `MX_Init()` exactly like Main does, so IAP's `mx_init.c` almost certainly still contains at least a `SystemClock_Config()` and `MX_GPIO_Init()`/`MX_I2C1_Init()` — `[INFERRED]`, not confirmed by opening the file.
- **Important header files:** `main/Inc/mxconstants.h` (full GPIO pin/port macro table — 27 pin/port macro pairs, confirmed authoritative and consistent with the secondary note `PS-CPU/GPIO/GPIO.md`), `stm32f0xx_hal_conf.h` (Main variant read; IAP variant downloaded but not decoded — `[UNKNOWN]` whether it differs).
- **Public API:** `MX_Init()`, `MX_I2C1_Init()`, `MX_GPIO_Init()`, `MX_TIM3_Init()`, `MX_IWDG_Init()`, `MX_RTC_Init()`, `KM_RTC_SetDefaultTime()`, `KM_RTC_Restore()`, `KM_RTC_ALARM_Init()`, `Error_Handler()`, `assert_failed()` — all called from `App/` code (`entry.c`, `km_it.c`, `km_alarm_wake.c`).
- **Internal APIs:** `SystemClock_Config()`/`SystemClock_Config2()` (only the latter is called outside this file, from `km_alarm_wake.c`'s wake path per `00_project_scope.md`).
- **Dependencies:** STM32 HAL (§3.2), CMSIS device headers.
- **Callers:** `entry.c::main()` calls `MX_Init()`; `km_it.c` calls into RTC/TIM re-init helpers around alarm wake; `km_alarm_wake.c::set_cycle_time()` calls `SystemClock_Config2()`-adjacent RTC helpers — `[INFERRED]` for the exact call, based on `00_project_scope.md`'s prior findings on this file, not re-verified line-by-line this session.
- **Callees:** HAL peripheral init functions (`HAL_RCC_OscConfig`, `HAL_I2C_Init`, `HAL_GPIO_Init`, `HAL_TIM_Base_Init`, `HAL_IWDG_Init`, `HAL_RTC_Init`, …) — `[INFERRED]` names, standard CubeMX pattern, not all individually confirmed.
- **Hardware dependencies:** RCC/PLL, GPIO A/B/F, I2C1, TIM3, IWDG, RTC + backup domain.
- **RTOS dependencies:** None.
- **Configuration dependencies:** every constant above (`Timing=0x00200000`, IWDG prescaler/window, RTC init mode) is itself the configuration; there is no separate config file layer beneath this one for peripheral init (unlike `mxconstants.h` for pins).

### 3.6 Keil project files & top-level configuration

- **Purpose:** Declare device target, memory map, preprocessor defines, and the exact list of source files compiled into each of the two production images (plus the third, non-shipping test image).
- **Responsibility:** Build-system source of truth — this is what actually decides which of the two `startup_*.s`, which HAL files, and which App files end up in the final `.axf`/`.hex`.
- **Classification:** Configuration.
- **Important source files:** `main/MDK-ARM/PowerSubCPU.uvprojx`, `iap/MDK-ARM/PowerSubCPU_iap.uvprojx`, and (referenced by the multi-project workspace file `PowerSubCPU.uvmpw` per `00_project_scope.md`, not itself opened for its target/device section this session) `common/Test/cubemx/…/PowerSubCPU_test.uvprojx`.
- **Confirmed facts, Main (`PowerSubCPU.uvprojx`):**
  - `<Device>STM32F031C6</Device>`, `<Define>USE_HAL_DRIVER,STM32F031x6</Define>`.
  - `<ScatterFile>` empty — no custom linker script; Keil's simple ROM/RAM-region model is used instead via `OCR_RVCT4`: **ROM at `0x08000000`, size `0x5000`** (20 KB).
  - RTE component list: only `ARM::CMSIS:CORE` registered — confirms no RTOS component (§0).
  - Compiled Groups: `HAL Driver` (19 files, includes ADC HAL sources though the only ADC caller is dead code), `Application/MDK-ARM` (`startup_stm32f031x6.s` only), `Drivers/CMSIS` (`system_stm32f0xx.c`, `I2C_stm32f0xx.c`, `FLASH_stm32f0xx.c`), `Application/User` (**9 files**: `entry.c`, `mx_init.c`, `km_extend_io.c`, `km_i2c.c`, `km_adc.c`, `km_it.c`, `km_alarm_wake.c`, `km_ca72_status.c`, `stm32f0xx_it.c`, `stm32f0xx_hal_msp.c` — note this is actually 10 names; the count is reported here as read, treat as confirmed-by-list rather than confirmed-by-arithmetic), `Drivers/BSP` (5 files, no `BSP_ADC`).
- **Confirmed facts, IAP (`PowerSubCPU_iap.uvprojx`):**
  - Same `<Device>`/`<Define>` as Main (STM32F031C6 — this resolves the `[INFERRED]` flagged in `00_project_scope.md` §1: the IAP target is now confirmed identical to Main, not merely assumed).
  - **ROM at `0x08005000`, size `0x3000`** (12 KB) via its own `OCR_RVCT4`.
  - **Caution (unresolved, carried forward from prior analysis):** the same file's `<TextAddressRange>` field reads `0x08000000`, matching Main's address rather than IAP's own `0x08005000` OCR setting. This may be a stale/inherited default Keil doesn't always update, or it may reflect something real about how the IAP `.axf` is linked/loaded. `[UNKNOWN]` — not resolved by reading the XML alone; do not assume either interpretation when writing downstream docs.
  - Compiled Groups: `HAL Driver` (15 files — no ADC, no RTC sources), `Application/MDK-ARM` (`startup_stm32f031x6.s`), `Drivers/CMSIS` (same 3 files — Flash driver is actually exercised by IAP's own code, unlike Main), `Application/User` (**6 files**: `entry_iap.c`, `mx_init.c`, `km_i2c_iap.c`, `km_extend_io_iap.c`, `stm32f0xx_it.c`, `stm32f0xx_hal_msp.c`), `Drivers/BSP` (4 files — no `BSP_ADC`, no `BSP_PWR`).
- **Important header files:** `main/Inc/mxconstants.h`, `main/Inc/stm32f0xx_hal_conf.h` (and their IAP-variant counterparts, downloaded but `[UNKNOWN]` content).
- **Public API:** none (build artifact, not runtime code).
- **Dependencies:** the entire file tree it names.
- **Callers/Callees:** N/A (build-time only).
- **Hardware dependencies:** encodes them (device/memory map) but doesn't touch hardware itself.
- **RTOS dependencies:** None (RTE confirms).
- **Configuration dependencies:** is itself the top of the configuration chain.

### 3.7 Main I2C protocol engine (`km_i2c.c`)

- **Purpose:** Implement the entire external contract of the PS-CPU chip: a memory-mapped I2C-slave register interface that the main SoC (S800/CA72) uses to read RTC time, read/write PS-CPU's I/O-extension bits, read status/version/checksum, and (via a shared address range) kick off IAP mode.
- **Responsibility:** I2C slave state machine, "2nd address" register decode, checksum computation, version-string formatting, an automated boot-timing event-record ring buffer.
- **Classification:** Protocol (with a Service-flavored sub-responsibility, the event recorder).
- **Important data structures:**
  - `struct __i2c_info { int state; uint8_t rx_buf[COMMAND_RWC_BUF_SIZE]; uint8_t* rx_buf_current; uint8_t second_addr; int write_comp; int read_comp; int xfer_size; bool rx_during; uint32_t rx_tick_last; bool tx_during; uint32_t tx_tick_last; uint32_t error_code; bool dir_error; } i2c_info;` (`COMMAND_RWC_BUF_SIZE = 1025` for Main, per `km_i2c.h`).
  - `enum { TYPE_State_Init, TYPE_State_Wait_Write, TYPE_State_Wait_Read }` — the I2C transaction state machine's states.
  - `THRS_EventBuff_Internal st_EventRecord[THRD_EVENT_BUFFER_MAX_NUM]` (250-entry ring buffer, `int nEventRecordIndex`) — added 2019/10/08 for automated boot-time measurement (per in-file comment).
  - Globals: `uint16_t g_checksum_value`, `static uint16_t g_checki2c_value`, `uint8_t g_main_version[MAIN_APPLICATION_SIZE_VERSION + 1]`.
- **Important functions:** `valid_second_addr()` (range-checks the register address against RTC/B-Register/INT_STS/INTERNAL_STS/INTERNAL_CTL/EXTEND_*/IAP_*/CHECKSUM ranges — an inline function), `rx_comp()` (the `ARM_I2C_SignalEvent_t` ISR-context callback registered with the CMSIS-Driver, §3.3), `change_state()`, `i2c_recv_first()` (one-time init), `i2c_sw_reset()`, `i2c_check_error()` (the 3-tier I2C error/timeout detector — Main-only, IAP's simpler I2C engine has no equivalent), `i2c_recv_wait()` (the main dispatcher — a large switch over ~40 register addresses, called every superloop iteration), `checksum_calc()` (16-bit checksum over flash contents), `GetVersionString()`, `save_rtc_reg()` (present but effectively dead — writes `RTC_TR`/`RTC_DR` to backup addresses; no confirmed caller this session either, consistent with `00_project_scope.md`'s prior finding), `setEventRecord()`.
- **Public API (I2C-visible, not C-linkage):** the entire `km_i2c.h` register-address map is PS-CPU's real "public API" — RTC 0x00–0x46, B-Register 0x5C/0x5E, `INT_STS` 0x60, `INTERNAL_STS`/`CTL` 0x68–0x6E, `EXTEND_*` 0x70–0x8E, `IAP_*` 0x90–0xA0, `EXTEND_PWROFF_*` 0xB0–0xBC, `EXTEND_EVENT` 0xC0, `CHECK`/`RESET_I2C` 0xFC/0xFE.
- **Internal (C-linkage) API:** `i2c_recv_first()`, `i2c_recv_wait()`, `i2c_sw_reset()`, `checksum_calc()`, `GetVersionString()`, `setEventRecord()` — called from `entry.c` and `km_extend_io.c`.
- **Important header files:** `main/Inc/km_i2c.h` (full register map, confirmed).
- **Dependencies:** CMSIS-Driver I2C (§3.3), `km_extend_io.h` (for `g_io_extend_memory[]` and I/O-extension bit macros), `km_it.h` (timers), `km_ver.h` (`VERSION_MAJOR=0x00`/`VERSION_MINOR=0x30` for the active Main branch).
- **Callers:** `entry.c::main()` (`i2c_recv_first()` once, `i2c_recv_wait()` every loop iteration); the CMSIS-Driver ISR calls `rx_comp()` asynchronously.
- **Callees:** `km_extend_io.c` (I/O-extension read/write helpers), `km_it.c` (timer state for I2C-related timeouts), the CMSIS-Driver's `Driver_I2C0.*` methods.
- **Hardware dependencies:** I2C1, direct RTC peripheral register access via `RTC_BASE + i2c_info.second_addr` memory-mapping (bypassing the HAL RTC API entirely for reads/writes of RTC's own register file — confirmed pattern per `00_project_scope.md`).
- **RTOS dependencies:** None.
- **Configuration dependencies:** `km_i2c.h`'s address-map constants; `IAP_PG_COMMAND_CHANGE_INFO_CHECKSUM_SHIFT = 8` (Main's shift value — cross-reference note: `00_project_scope.md` had flagged uncertainty over whether this was 8 or 16; **this session confirms 8** directly from the real `km_i2c.h`, for the Main-side register only — IAP's own copy of the equivalent constant was seen in `km_i2c_iap.h`, also `8`, so both images agree).

### 3.8 Main interrupt/timer/debounce subsystem (`km_it.c`)

- **Purpose:** Central low-level event plumbing for the Main image: software timers, deferred-interrupt-work bitmask, nested/ref-counted IRQ masking, and GPIO debounce ("anti-chattering") for three noisy input signals.
- **Responsibility:** Own the 1 ms tick (TIM3 ISR callback), own all 16 software timer slots, translate raw GPIO EXTI events into either an immediate call or a deferred "pending factor" bit for the main loop to pick up, and run a generic chattering-removal state machine reused by three separate signals.
- **Classification:** Driver.
- **Important data structures:**
  - `TYPE_Km_Timer_Kind` — 16-member enum (exact members carried over from `00_project_scope.md`'s prior full read of `km_it.h`), backing `km_timer_st g_km_timer[16]`.
  - `TYPE_Km_Pending_Factor_Bit` enum (4 real bits + `All`/`Inval` sentinels) backing the non-static global `pending_factor`.
  - `int ei_ref[32]` — one nesting-depth counter per NVIC line, for `km_disable_irq()`/`km_enable_irq()`.
  - `km_anti_chattering_st anti_chattering_info[3]` (one per debounced signal: MSW, POWER_MONITOR, 24V11_MONI), each with `start_level`/`prev_level`/`status`/`timer_kind`/`intr_proc` fields (per `00_project_scope.md`).
  - `uint32_t ui_EventCountTimer`.
- **Important functions:** `init_ei_ref()`, `disable_external_irq()`/`enable_external_irq()`, `get_pending_factor()`/`put_pending_factor()`, `set_pending_factor_bit()`/`clear_pending_factor_bit()`/`read_pending_factor_bit()`, `km_exti_callback()` (the shared `HAL_GPIO_EXTI_Callback` implementation — dispatches on pin to `_HRESET_REQ`, `AP_PWR_EN`, `POWER_MONITOR`, `MONI_24V11`, `MSW_ON` handling), `HAL_RTC_AlarmAEventCallback()` (defined **here**, not in `km_alarm_wake.c`, despite that file owning alarm *setup*), `km_tim_callback()` (the 1 ms `HAL_TIM_PeriodElapsedCallback`-style hook that decrements all 16 active timers), `km_it_init()`, `km_timer_set()`/`km_timer_get_state()`, `km_disable_irq()`/`km_enable_irq()` (ref-counted), `get_anti_chattering_info()`, `start_anti_chattering()`, and (confirmed by this session's read of `km_extend_io.c`, which calls it) `anti_chattering_proc()` is actually **defined in `km_extend_io.c`**, not `km_it.c` — see §3.12; `km_it.c` supplies the *primitives* (`get_anti_chattering_info`, `start_anti_chattering`) that `anti_chattering_proc()` in the Application module drives.
- **Public API:** `km_it_init()`, `km_timer_set()`, `km_timer_get_state()`, `km_disable_irq()`/`km_enable_irq()`, `get_pending_factor()`/`put_pending_factor()`, `set/clear/read_pending_factor_bit()`, `get_anti_chattering_info()`, `start_anti_chattering()`, `KM_TIMER_MAX_TIME` (`= 65535` ms — confirmed directly from `km_it.h`, resolving the prior `00_project_scope.md` ambiguity between 32768 and 65535).
- **Important header files:** `main/Inc/km_it.h`.
- **Dependencies:** `km_extend_io.h` (for signal/pin identity), BSP (§3.4) for the actual GPIO reads inside `anti_chattering_proc()`.
- **Callers:** `entry.c::main()` calls `km_it_init()` once; `km_extend_io.c` calls almost every public function above, every loop iteration.
- **Callees:** HAL's `HAL_GPIO_EXTI_Callback`/`HAL_TIM_PeriodElapsedCallback` machinery calls into `km_exti_callback()`/`km_tim_callback()` from ISR context (`[INFERRED]` exact HAL call chain, confirmed only that `stm32f0xx_it.c`'s `EXTIx_IRQHandler`/`TIM3_IRQHandler` call the generic `HAL_GPIO_EXTI_IRQHandler`/`HAL_TIM_IRQHandler`, which per stock HAL behavior invoke the weak callbacks these functions override).
- **Hardware dependencies:** EXTI lines for `_HRESET_REQ`, `AP_PWR_EN`, `POWER_MONITOR`, `MONI_24V11`, `MSW_ON` pins; TIM3; RTC Alarm A IRQ; NVIC.
- **RTOS dependencies:** None.
- **Configuration dependencies:** `KM_TIMER_MAX_TIME=65535`; the 3-entry `anti_chattering_info[]` table's per-signal debounce timer assignment.

### 3.9 RTC alarm/wake driver (`km_alarm_wake.c`)

- **Purpose:** Program RTC Alarm A so the chip can be woken from STOP mode on a schedule (used by the sleep-mode logic in `km_extend_io.c`).
- **Responsibility:** Compute a target alarm time from the current RTC seconds register (with a double-read-compare pattern to avoid a torn read across a rollover) and arm Alarm A.
- **Classification:** Driver.
- **Important functions:** `set_cycle_time(uint8_t sec)` — returns `HAL_StatusTypeDef`; on success, sets the non-static global `stop_mode = ON` and configures the alarm for `sec` seconds out.
- **Public API:** `set_cycle_time()`, global `stop_mode`.
- **Important header files:** `km_alarm_wake.h` (note: its include-guard trailing comment reads `/* __KM_ADC_H */` — a stale copy-paste artifact, not a functional bug, but worth flagging as documentation debt).
- **Dependencies:** `mx_init.c`'s RTC init helpers (§3.5), HAL RTC API.
- **Callers:** `km_extend_io.c::msw_on_proc()` calls `set_cycle_time(2)` (2-second wake cycle) immediately before `BSP_PWR_enter_stopmode()`, guarded by `#define MSWOFF_STOP_MODE` (confirmed active in the read source — the `#else` branch, `BSP_PWR_enter_sleepmode()`, is compiled out).
- **Callees:** RTC HAL functions, `RTC->TR` direct register read (for the double-read-compare).
- **Hardware dependencies:** RTC peripheral + Alarm A, backup domain.
- **RTOS dependencies:** None.
- **Configuration dependencies:** the caller-supplied cycle length (`2` seconds, hardcoded at the one call site read this session).

### 3.10 CA72 status tracker (`km_ca72_status.c`)

- **Purpose:** Cache a 3-state view (`CA72_OFF`/`CA72_ON`/`CA72_SLEEP`) of the main SoC's power/sleep state, set from I2C-driven internal-status bits and consumed by the sleep-status-remote logic.
- **Responsibility:** Trivial getter/setter over one static variable.
- **Classification:** Service.
- **Important functions:** `set_ca72_status()`, `get_ca72_status()`.
- **Important data structures:** `static TYPE_Km_CA72_Status _status`.
- **Documentation-debt finding:** the file's own header doc-comment still says `File Name [ km_s800_status.c ]` — a stale filename left over from before the file was renamed to `km_ca72_status.c`; its header's include guard likewise trails `/* __KM_S800_STATUS_H */`.
- **Dependencies:** `km_extend_io.h` (for the `TYPE_Km_CA72_Status` enum, `[INFERRED]` exact header of origin).
- **Callers:** `km_i2c.c` (sets status from I2C-written internal-control bits — `[INFERRED]` from `00_project_scope.md`'s prior finding, not re-verified this session), `km_extend_io.c::sleep_status_rem_eagle_proc()`/`sleep_status_rem_sparrow_proc()` (both call `get_ca72_status()` every invocation, confirmed this session).
- **Callees:** none.
- **Hardware dependencies:** none directly (pure state cache).
- **RTOS dependencies:** None.
- **Configuration dependencies:** none.

### 3.11 ADC monitor (`km_adc.c`) — dead code

- **Purpose (as designed, not as shipped):** 5V-rail ADC monitoring, per the function names `adc_moni_5v_start_proc()`/`adc_moni_5v_off_proc()` still called from `entry.c::main()`.
- **Responsibility:** None at runtime — **both functions' bodies are wrapped in `#if 0`**, confirmed directly in `km_adc.c`. `entry.c` still calls `adc_moni_5v_start_proc()` and `adc_moni_5v_off_proc()` every loop iteration, but since the compiled bodies are empty (or the whole definition is preprocessed out and a stub/empty version remains — the exact `#if 0` boundary placement wasn't re-examined this session beyond confirming "both functions are `#if 0`'d"), these calls are effectively no-ops.
- **Classification:** Utility (disabled).
- **Important header file:** `km_adc.h` — `#define TEST_BOARD` is **active** (uncommented), meaning `ADC_VAL_POWER_ON = 0xb27` (the test-board threshold constant) would apply *if* ADC monitoring were ever re-enabled; this is a live landmine for anyone flipping the `#if 0` back on without also revisiting `TEST_BOARD`.
- **Dependencies:** BSP_ADC (§3.4) — `[INFERRED]`, would be the dependency if active; currently unreachable.
- **Callers:** `entry.c::main()` (calls into the dead stubs every loop iteration — harmless but worth noting as a minor code-hygiene issue).
- **Callees:** none (dead).
- **Hardware dependencies:** none currently exercised.
- **RTOS dependencies:** None.
- **Configuration dependencies:** `TEST_BOARD` macro state.

### 3.12 Main power-sequencing application (`km_extend_io.c` + `entry.c`)

This is the single most important module in the repository — the actual product logic. `entry.c` is the thin `main()`/superloop shell; `km_extend_io.c` (1,540 lines) holds essentially everything the superloop calls.

- **Purpose:** Sequence power to the main SoC (S800/CA72) and its peripherals safely, in the right order, with the right delays; present an I/O-extension register map over I2C so the SoC can read/write PS-CPU-controlled signals; manage sleep/wake and instantaneous-power-loss ("flicker") backup; detect model (Sparrow/Eagle) and generation (TypeI/II vs TypeIII) and dispatch generation-specific behavior through function-pointer tables.
- **Responsibility:** Everything under §5 "Find" that isn't startup/ISR plumbing.
- **Classification:** Application.
- **Important data structures:**
  - `io_extend[]` — the central GPIO/signal abstraction table. **Confirmed exactly 14 entries** this session (a precise count not stated with this exact number in any prior secondary doc).
  - `g_io_extend_memory[]` — the in-RAM mirror of the I2C-visible I/O-extension register map (what `km_i2c.c`'s dispatcher reads/writes and what `km_extend_io.c`'s `_proc()` functions act on).
  - `TYPE_Model` enum: `{None, RESERVED0, RESERVED1, Sparrow, Eagle}`; `TYPE_Generation` enum: `{TYPE_Gen_TYPEI_II, TYPE_Gen_TYPEIII}`.
  - `km_io` struct (per-entry shape of `io_extend[]` — full field list previously captured in `00_project_scope.md`/prior reads of `km_extend_io.h`).
  - `PWROFF_FACTOR_*` bitmask macros (9 distinct factors — confirmed from `km_extend_io.h`) used by `poweroff_factor_save()` to log *why* the unit powered off into the RTC backup-domain log (`B_REGISTER_PWROFF_LOG`).
  - `g_power_on_flg`, `g_first_power_on`, `g_24v_check_flg_off`, `g_internal_sts_boot_flg`, `g_backupwait_pending` — power-sequencing state flags.
  - `EXTEND_OUTPUT_INIT_VALUE = 0x00001E27` (the reset value written into the output side of `g_io_extend_memory[]` by `extend_output_init()`).
- **Important functions — power-on/off core:**
  - `s800_power_on()` — includes the 2019/10/08 auto-measurement `setEventRecord()` instrumentation block (confirmed, matches `km_i2c.c`'s event-ring-buffer feature).
  - `s800_power_off()` — an exact 10-step sequence (fully captured in a prior read; not re-transcribed here to avoid duplicating `00_project_scope.md`, but confirmed present and unchanged).
  - `_MC_P_ON_OK()` / `_MC_PWR_EN_OK` — the 6-condition gating macro deciding when it's safe to assert `MC_P_ON`/`MC_PWR_EN`.
  - `mc_p_on_eagle_proc()`, `mc_p_on_sparrow_proc()`, `mc_p_on_proc(TYPE_Model model)` — function-pointer dispatch: `static void (*const func[])(void) = {dummy, dummy, sparrow_proc, eagle_proc};` pattern (confirmed, matches the equivalent `sleep_status_rem_proc()` dispatch below).
  - `mc_pwr_en_proc()` — asserts `MC_PWR_EN` when `AP_PWR_EN` and `SB_PG` are both high; **Eagle-specific extra branch**: on first power-on/reboot with `_MC_PWR_EN_OK()` true, sets `g_power_on_flg = POWER_ON_FLG_NORMAL` and starts a 320 ms `TYPE_Km_Timer_BOOT_MC_P_ON` timer.
  - `ir_p_on_proc()` — asserts `IR_P_ON` only when **all** of: the io-extend bit is set, `AP_PWR_EN` high, `_RESET` high, `POWER_MONITOR` debounce-settled-high, (`MSW_ON` debounce-settled-high **or** the power-off watchdog timer is running), and `MC_P_ON` high. History comments show this condition has been revised at least 6 times (2016–2018).
  - `mc_oe_proc()` — confirmed **empty stub** (`return;` only); the `MC_OE` signal was removed from the design in 2017 per its own header comment, function kept only for call-site compatibility.
  - `mc3_3von_moni_proc()` — confirmed empty (per prior read).
- **Important functions — sleep/wake and remote-sleep signaling:**
  - `sleep_status_rem_eagle_proc()` / `sleep_status_rem_sparrow_proc()` / `sleep_status_rem_proc(TYPE_Model model)` — per-model `SLEEP_STATUS_REM` signal logic, each tracking `prev_status`/`cur_status` via `get_ca72_status()` (§3.10) and emitting `setEventRecord()` calls (`ApLowDet`, `ApHighDet`, `ApOutRemLow/High`, `LpppRemLow/High`, `SbOutRemLow`) for the boot/sleep-transition instrumentation. Sparrow's variant additionally gates on `get_Generation() == TYPE_Gen_TYPEIII` for an LPPP-driven control path with its own 320 ms timer (`TYPE_Km_Timer_MC_P_ON_AFTER_SLEEP_STATUS_REM_SP`).
  - `sleep_mode()` — enters `BSP_PWR_enter_sleepmode()` when MSW is on (debounce-settled), `SB_PG` is high, `AP_PWR_EN` is low, and no pending factor is set — i.e. only when the SoC itself is in Sleep2/ErP and nothing is waiting to be handled.
  - `msw_on_proc(uint8_t state)` — the most involved single function read this session: gates on the instantaneous-backup timer and the power-off-sequence timer first (returns early if either is active — "power inhibited during backup/reboot window"); on `STATE_INT` (called from the debounce-settle interrupt path) it either issues `HAL_NVIC_SystemReset()` (MSW on + 24V11 low + SB_PG low — a brown-out-like condition), calls `s800_power_off()` (MSW off while the SoC never booted), or starts the power-off sequence timer; on `STATE_NORMAL` it drives actual STOP-mode entry: guarded by `#define MSWOFF_STOP_MODE` (active), when MSW is off and `SB_PG` is low it arms a 2-second RTC wake cycle via `set_cycle_time(2)` (§3.9) and calls `BSP_PWR_enter_stopmode()`; the `#else` branch (`BSP_PWR_enter_sleepmode()`, SLEEP not STOP) is compiled out.
  - `sb_reset_proc()` — releases `_RESET` (sets it high) once MSW/24V11/`SB_PG` conditions are all satisfied and the `SB_RESET_RELEASE` timer has elapsed; emits `setEventRecord(THRD_EventRecord_LPPPStart)` on release (2019/10/08 instrumentation).
- **Important functions — power-off timers and instantaneous-backup:**
  - `poweroff_timer_init()` / `poweroff_timer_start()` / `poweroff_timer_proc()` — a 4-timer cascade: `PWROFF_WDGTIME` (2000 ms), `PWROFF_MAXTIME1`→`PWROFF_MAXTIME2` (60000 ms each, chained), `PWROFF_SEQTIME` (9350 ms), `BACKUP_WAITTIME` (50 ms). Any WDG/MAXTIME2 timeout calls `poweroff_factor_save()` + `s800_power_off()`.
  - `backupwait_timer_proc()` — after the 50 ms instantaneous-backup window elapses, if any event was pending during that window (`g_backupwait_pending`), power off with `PWROFF_FACTOR_BACKUP_ON`.
  - `intr_pending_proc()` — drains the `pending_factor` bitmask set by `km_it.c`'s ISR-context callback (`TYPE_Km_PFB_HRESET_REQ` → `hreset_req_proc()`, `TYPE_Km_PFB_AP_PWR_EN` → `ap_power_en_proc(STATE_INT)`), i.e. the concrete mechanism by which ISR work gets deferred to the main loop.
  - `anti_chattering_proc()` — **defined here**, not in `km_it.c` (see §3.8 correction): iterates all 3 debounce channels, restarts the debounce timer on level changes, and on timer expiry with a stable level calls the registered `intr_proc(STATE_INT)` handler (this is how `msw_on_proc(STATE_INT)` and the `POWER_MONITOR`/`24V11` interrupt-context handlers actually get invoked, one main-loop iteration after the physical level has been stable long enough).
- **Important functions — misc I/O signal handling:** `usb2_oe_proc()` (mirrors `_RST_SLP2` to `_USB2_OE`, unless overridden by `is_usb2_oe_from_main_cpu_ctrl`), `rst_usb_proc()`, `erp_sensor_proc()`, `power_monitor_proc()`, `moni_24v_proc()`, `hreset_req_proc()`, `ap_power_en_proc()` (all confirmed present in a prior read of the file's first ~55%, function bodies not re-transcribed here).
- **Important functions — identity/dispatch:**
  - `get_model()` — reads `MODEL_BIT0`/`MODEL_BIT1` GPIO pins **once** and caches the result (`static int is_first`); contains a commented-out `#if 0 // なぜ？` ("why?") alternate implementation left in place by the original author, itself a small piece of documentation debt.
  - `get_Generation()` — derives `TYPE_Gen_TYPEIII` vs `TYPE_Gen_TYPEI_II` from an I2C-writable internal-status bit (`INTERNEAL_STS_COMMAND_MODEL_TYPE`), **not** from a GPIO strap — i.e. generation is told to PS-CPU by the main SoC over I2C at runtime, model is read from hardware straps at boot. This is an important, previously-unstated distinction: "model" and "generation" are determined by two completely different mechanisms.
  - `i2c_reset()` — force-resets the I2C1 peripheral via direct RCC register macros (`__HAL_RCC_I2C1_FORCE_RESET`/`RELEASE_RESET`) then calls `MX_I2C1_Init()` — a heavier reset path than the CMSIS-Driver's own `i2c_sw_reset()` in `km_i2c.c`.
- **Public API:** every `*_proc()` function called from `entry.c::main()`'s loop (§5), plus `get_model()`, `get_Generation()`, `s800_power_on()`, `s800_power_off()`, `poweroff_timer_init()`, `i2c_reset()` — all consumed by `entry.c` and `km_i2c.c`.
- **Internal API:** the per-model/per-generation `_eagle_proc`/`_sparrow_proc` pairs and their dispatch wrappers; `dummy()` (the no-op filler for the `None`/`RESERVED*` model slots in the dispatch tables).
- **Important header files:** `main/Inc/km_extend_io.h` (full enums, `km_io` struct, all bit-position and `PWROFF_FACTOR_*` macros, confirmed).
- **Dependencies:** BSP (§3.4, all GPIO access), `km_it.h`/`km_it.c` (§3.8, timers/debounce/pending-factor), `km_ca72_status.h` (§3.10), `km_i2c.h` (for `setEventRecord`/event-record types), `km_alarm_wake.h` (`set_cycle_time`), HAL (`__HAL_RCC_I2C1_*`, `HAL_NVIC_SystemReset`).
- **Callers:** `entry.c::main()` calls nearly the entire public surface, once per superloop iteration, in the exact fixed order documented in §5.
- **Callees:** BSP, `km_it.c`, `km_ca72_status.c`, `km_i2c.c::setEventRecord()`, `km_alarm_wake.c::set_cycle_time()`, HAL/CMSIS (`HAL_NVIC_SystemReset`, `__HAL_RCC_I2C1_FORCE_RESET`).
- **Hardware dependencies:** essentially every GPIO pin in `mxconstants.h` (`MSW_ON`, `SB_PG`, `AP_PWR_EN`, `_RESET`, `MC_P_ON`, `MC_PWR_EN`, `IR_P_ON`, `POWER_MONITOR`, `MONI_24V11`, `SLEEP_STATUS_REM_EG`/`_SP`, `MODEL_BIT0/1`, `_RST_SLP2`, `_USB2_OE`, etc.), PWR (STOP/SLEEP), I2C1 (indirectly via reset), RTC (indirectly via `set_cycle_time`).
- **RTOS dependencies:** None.
- **Configuration dependencies:** every numeric timer constant listed above (`PWROFF_MAXTIME=60000`, `PWROFF_WDGTIME=2000`, `BACKUP_WAITTIME=50`, `PWROFF_SEQTIME=9350`, the 320 ms boot/sleep-transition timers), `EXTEND_OUTPUT_INIT_VALUE=0x00001E27`, `MSWOFF_STOP_MODE` compile-time switch.
- **Open finding, not resolved this session:** `jump2iap()` is defined in `entry.c` (full body: reads the reset-vector word from `IAP_DEFAULT_ADD + 4` = `0x08005004`, sets `MSP` from `*0x08005000`, and branches there) and declared in `entry.h`, but a repository-wide full-text search this session for `jump2iap` found **no caller** anywhere in the read source (`km_i2c.c`, `km_extend_io.c`, `entry.c` itself) — only its own definition, its own declaration, and one mention in the (Word-format) BSP design-spec document. `[UNKNOWN]` — either (a) `jump2iap()` is currently dead code and the real Main→IAP transition happens some other way (e.g. the main SoC commands PS-CPU to set an I2C-visible IAP-status bit and then power-cycles or resets PS-CPU itself, relying on boot-time logic elsewhere to detect it — no such boot-time check was found in `entry.c::main()` either, which unconditionally runs the Main superloop), or (b) the call site exists in a part of `km_i2c.c`'s ~40-case dispatch switch that was described at a summary level in a prior pass but not re-verified character-by-character this session. Flagging rather than guessing.

### 3.13 IAP entry/superloop (`entry_iap.c`)

- **Purpose:** The IAP image's own `main()` — see §5 for the full body. Relocates the vector table to SRAM, then runs a two-call superloop dedicated entirely to the I2C firmware-update protocol.
- **Responsibility:** Minimal bring-up (`MX_Init()`, checksum/version capture, watchdog start, interrupt init), vector-table relocation (`__HAL_SYSCFG_REMAPMEMORY_SRAM()` — required because Cortex-M0 has no VTOR register, so remapping SRAM to address 0 is the only way to give the running-from-offset-0x08005000 IAP image a correctly-located vector table for its own interrupts), then `while(1) { i2c_recv_wait(); BSP_WDT_Refresh(); }`.
- **Classification:** Application.
- **Important data structures:** `__IO uint32_t VectorTable[48]` — placed at absolute address `0x20000000` via `__attribute__((at(0x20000000)))` (ARMCC) — the SRAM copy of the vector table.
- **Important functions:** `main()` (see §5), relies on `km_it_init_iap()` (§3.15) for interrupt registration.
- **Public API:** none beyond `main()` itself (an entry point, not a library).
- **Important header files:** `iap/Inc/entry_iap.h` (not re-read in full this session; existence and inclusion confirmed).
- **Dependencies:** `BSP_STM32F03x_Nucleo.h`, `km_i2c_iap.h`, HAL SYSCFG remap macro.
- **Callers:** the CPU reset vector (this **is** `main()` for the IAP image).
- **Callees:** `MX_Init()`, `checksum_calc()`, `GetVersionString()` (IAP's own copies, §3.14), `BSP_WDT_Start()`/`Refresh()`, `km_it_init_iap()`, `i2c_recv_first()`, `i2c_recv_wait()` (both from `km_i2c_iap.c`, §3.14).
- **Hardware dependencies:** SYSCFG (memory remap), Cortex-M0 vector table location, IWDG.
- **RTOS dependencies:** None.
- **Configuration dependencies:** `IAP_ADDRESS` (the flash offset the 48-word vector table is copied *from* — a source-code comment says "mapped at the base of the application load address 0x08001600," which does **not** match the `.uvprojx`-confirmed IAP ROM base of `0x08005000`; `[UNKNOWN]`, likely either a stale comment from an earlier memory-map revision or a distinct constant not yet cross-checked against `km_i2c_iap.h`'s own `IAP_ADDRES...` definition, which was truncated mid-token in the header snippet this session retrieved and not re-fetched in full).

### 3.14 IAP I2C/flash-programming engine (`km_i2c_iap.c`)

- **Purpose:** A deliberately simpler I2C-slave engine than Main's, whose entire job is to receive a new firmware image over I2C and write it into flash.
- **Responsibility:** Register-map decode for the IAP-specific command set, flash erase/program, dual checksum verification.
- **Classification:** Protocol.
- **Important data structures:** a smaller `i2c_info` struct than Main's (no `error_code`/`dir_error`/timeout-tick fields — IAP has no equivalent of Main's 3-tier I2C timeout detector, confirmed absent), `static uint32_t g_dest_addr` (or similarly named flash write-pointer, per prior read — the exact type/name was captured in the pre-compaction summary as `g_dest_addr`).
- **Important functions:** `valid_second_addr()` (a much smaller valid-address set than Main's, restricted to the IAP command range `0x90`–`0xA0`), `i2c_recv_first()` (initializes the I2C CMSIS-Driver **and** the Flash CMSIS-Driver, then immediately calls `main_project_flash_erase()` — i.e. IAP erases the Main application region as soon as it boots, before any I2C transaction has even occurred), `i2c_recv_wait()` (handles `IAP_PG_COMMAND`'s magic-number trigger, `IAP_PG_COMMAND_MAGICNUM = 0x11223344` per `km_i2c_iap.h`), `flash_write_32()`, `main_project_flash_erase()`, `check_sum_calc()` (an **8-bit** checksum — a distinct algorithm from Main's 16-bit `checksum_calc()`, despite the similar name), `checksum_calc()`/`GetVersionString()` (IAP's own copies, same algorithms as Main's per prior comparison).
- **Public API:** `i2c_recv_first()`, `i2c_recv_wait()` — called from `entry_iap.c::main()`.
- **Important header files:** `iap/Inc/km_i2c_iap.h` — confirmed this session in full: `COMMAND_RWC_BUF_SIZE = 1025` (matches Main's), the entire RTC/B-Register/`INT_STS`/`INTERNAL_STS`/`EXTEND_*`/`IAP_PG_COMMAND*` register map (same numeric addresses as Main's `km_i2c.h`, confirming both images share one address-map convention even though IAP only implements a subset), `IAP_PG_COMMAND_MAGICNUM = 0x11223344`, `RTC_WRITEPROTECT_DISABLE/ENABLE` macros (0xCA/0x53/0xFF, identical to Main's), `MAIN_APPLICATION_ADDRESS = 0x08000000`, `MAIN_APPLICATION_SIZE = 0x5000` (20 KB — this is IAP's own record of where and how big the Main image is, i.e. the erase/write target), and a truncated `IAP_ADDRES[S...]` definition (cut off in the tool output this session received; value not confirmed, presumably `0x08005000`/`0x3000` by symmetry with `MAIN_APPLICATION_*` — `[INFERRED]`).
- **Dependencies:** CMSIS-Driver I2C and Flash (§3.3).
- **Callers:** `entry_iap.c::main()`.
- **Callees:** `Driver_I2C0.*`, the Flash CMSIS-Driver, HAL flash unlock/erase/program primitives (`[INFERRED]`, underlying `FLASH_stm32f0xx.c` body not read).
- **Hardware dependencies:** internal Flash controller, I2C1.
- **RTOS dependencies:** None.
- **Configuration dependencies:** the shared register-address map; the magic number; `MAIN_APPLICATION_ADDRESS`/`SIZE`.
- **Open finding, flagged for careful wording (carried from before compaction):** `i2c_recv_first()` erases the Main flash region once at IAP boot, and the dispatcher's handling of the `IAP_PG_COMMAND` magic-number write appears to erase again. Whether this is intentional defensive re-erase (idempotent safety) or redundant/dead logic could not be determined from source alone and is **not** asserted either way here.

### 3.15 IAP GPIO glue (`km_extend_io_iap.c`)

- **Purpose:** The one piece of IAP application logic outside the I2C engine: detect that MSW (the main power switch) has been turned off *and* an update has completed, then reset the chip back into the Main image.
- **Responsibility:** GPIO interrupt registration and one state check.
- **Classification:** Application.
- **Important functions:** `msw_on_proc_iap()` (checks `MSW_ON` went low **and** the update-complete status bit, then calls `HAL_NVIC_SystemReset()` — this, not `jump2iap()`, is the confirmed mechanism by which IAP hands control back after finishing a flash write: a full chip reset, which per normal STM32 boot behavior returns execution to `0x08000000` — i.e. back into the **Main** image, assuming the boot-address configuration hasn't been altered), `km_exti_callback_iap()` (only handles the `MSW_ON_Pin` case — IAP's interrupt surface is deliberately narrower than Main's), `km_it_init_iap()` (registers the GPIO callback only; **no timer init**, confirming IAP has no software-timer subsystem of its own, consistent with its BSP Group excluding neither TIM per §3.6 wait — actually IAP's BSP Group *does* include a TIM-named file per the 4-file count in §3.6; whether it's ever actually started is `[UNKNOWN]`, since `entry_iap.c::main()`'s read body does not call `BSP_TIM_Start`).
- **Public API:** `km_it_init_iap()`, called from `entry_iap.c::main()`.
- **Dependencies:** BSP (§3.4), HAL NVIC.
- **Callers:** `entry_iap.c::main()` (init), EXTI ISR (via `km_exti_callback_iap()`, ISR-context).
- **Callees:** `HAL_NVIC_SystemReset()`.
- **Hardware dependencies:** `MSW_ON` EXTI pin, NVIC system reset.
- **RTOS dependencies:** None.
- **Configuration dependencies:** none beyond the pin assignment.

### 3.16 Hardware self-test harness (`common/Test/cubemx`)

- **Purpose:** A standalone bring-up/self-test firmware image (own Keil project, own `main()`), exercising GPIO, register access, ADC, USART, Timer, I2C-master, and I2C-slave paths independently — **not** part of either shipping image.
- **Responsibility:** Manufacturing/bring-up validation, not product function.
- **Classification:** Application (test harness).
- **Important functions:** its own `main()` (found this session, full body): calls `MX_Init()`, `Test_Init()`, then runs `GPIO_Test()` → `Reg_Test()` → `ADC_Test()` → `USART_Test()` → `Timer_Test()` → `I2C_Test()` → `I2C_Slave_Test()` in sequence, `goto err` on any non-zero return, and finally lights `LED2` (`GPIOA` `PIN5`) on full success. In the version read, only `TEST_I2C_SLAVE` is `#define`'d active (`#if 0`-guarded block above disables `TEST_GPIO/REG/ADC/USART/TIMER/I2C`), so most of the `_Test()` calls resolve to a stubbed `(0)` via the `#ifndef TEST_X #define X_Test() (0) #endif` pattern — meaning in this snapshot the harness effectively runs only the I2C-slave self-test.
- **Important data structures:** `B1_press_event_t b1_ev_tbl[]` — a function-pointer table mapping a button-press event to per-subsystem test-trigger callbacks (`B1_press_gpio_event`, `B1_press_adc_event`, `B1_press_i2c_event`, others `NULL`).
- **Public API:** none (standalone image, not linked into product firmware).
- **Dependencies:** `test_gpio.h`, `test_reg.h`, `test_adc.h`, `test_usart.h`, `test_tim.h`, `test_i2c.h`, `test_i2c_slave.h`, `test_common.h` (none of these opened this session — `[UNKNOWN]` contents), `BSP_STM32F03x_Nucleo.h`.
- **Callers:** CPU reset vector (its own image).
- **Callees:** `MX_Init()`, `Test_Init()`, the per-subsystem `*_Test()` functions, `BSP_GPIO_WritePin()`.
- **Hardware dependencies:** whatever each `test_*` module exercises — `[UNKNOWN]` in detail.
- **RTOS dependencies:** None.
- **Configuration dependencies:** the `#define TEST_I2C_SLAVE` / commented-out `#if 0` block selecting which subsystem test compiles active.

---

## 4. Per-file quick reference

*(File → Module → Responsibility → Important functions → Important data structures → Dependencies. Limited to files this session read in full or in a prior session and independently confirmed; see §3 for the fuller narrative on each. "Dead" annotations mean confirmed unreachable/disabled code, not a guess.)*

| File | Module | Responsibility | Important functions | Important data structures | Dependencies |
|---|---|---|---|---|---|
| `main/App/entry.c` | Application (Main entry) | `main()`, superloop orchestration, `jump2iap()` (unreferenced — see §3.12 finding), `DeInit()` | `main`, `jump2iap`, `DeInit` | `TYPE_Model model` (local) | `km_extend_io.h`, `km_i2c.h`, `km_it.h`, `km_adc.h`, `km_ca72_status.h`, BSP, I2C_stm32f0xx.h |
| `main/App/km_i2c.c` | Protocol/Service | I2C slave register-map engine, event-record ring buffer | `i2c_recv_first`, `i2c_recv_wait`, `rx_comp`, `i2c_check_error`, `checksum_calc`, `GetVersionString`, `setEventRecord`, `save_rtc_reg` (dead) | `struct __i2c_info i2c_info`, `st_EventRecord[250]` | `km_i2c.h`, `km_extend_io.h`, `km_it.h`, `km_ver.h`, CMSIS-Driver I2C |
| `main/App/km_extend_io.c` | Application | Power sequencing, I/O-extension map, sleep/wake, model/generation dispatch | `s800_power_on/off`, `mc_p_on_proc`, `msw_on_proc`, `sleep_mode`, `anti_chattering_proc`, `poweroff_timer_*`, `get_model`, `get_Generation`, `i2c_reset` | `io_extend[14]`, `g_io_extend_memory[]`, `TYPE_Model`, `TYPE_Generation` | `km_extend_io.h`, `km_it.h`, `km_ca72_status.h`, `km_i2c.h`, `km_alarm_wake.h`, BSP, HAL |
| `main/App/km_it.c` | Driver | Software timers, pending-factor deferral, ref-counted IRQ mask, debounce primitives | `km_it_init`, `km_timer_set/get_state`, `km_exti_callback`, `km_tim_callback`, `km_disable/enable_irq`, `get_anti_chattering_info`, `start_anti_chattering` | `g_km_timer[16]`, `pending_factor`, `ei_ref[32]`, `anti_chattering_info[3]` | `km_it.h`, `km_extend_io.h` |
| `main/App/km_alarm_wake.c` | Driver | RTC Alarm A programming for STOP-mode wake | `set_cycle_time` | `stop_mode` (global) | HAL RTC, `mx_init.c` helpers |
| `main/App/km_ca72_status.c` | Service | Cached 3-state SoC status | `set_ca72_status`, `get_ca72_status` | `static _status` | `km_extend_io.h` |
| `main/App/km_adc.c` | Utility (dead) | 5V ADC monitoring — both functions `#if 0`'d | `adc_moni_5v_start_proc`, `adc_moni_5v_off_proc` (both dead) | none | `km_adc.h` |
| `main/Inc/km_extend_io.h` | Configuration | Enums, `km_io` struct, bit macros, `PWROFF_FACTOR_*` | — | `TYPE_Model`, `TYPE_Generation`, `km_io` | — |
| `main/Inc/km_i2c.h` | Configuration | I2C register-address map | — | address-map `#define`s | — |
| `main/Inc/km_it.h` | Configuration | Timer/debounce enums and structs | — | `TYPE_Km_Timer_Kind` (16), `km_anti_chattering_st` | — |
| `main/Inc/mxconstants.h` | Configuration/Generated code | Authoritative GPIO pin/port table (27 pairs) | — | pin/port `#define`s | — |
| `main/Inc/km_ver.h` | Configuration | Main image version | — | `VERSION_MAJOR=0x00`, `VERSION_MINOR=0x30` | — |
| `main/Inc/km_adc.h` | Configuration | ADC thresholds (unused while `km_adc.c` is dead) | — | `TEST_BOARD` (active), `ADC_VAL_POWER_ON=0xb27` | — |
| `main/Src/mx_init.c` | Generated code/Configuration | Peripheral init | `MX_Init`, `SystemClock_Config[2]`, `MX_I2C1_Init`, `MX_IWDG_Init`, `MX_RTC_Init`, `KM_RTC_Restore`, `MX_TIM3_Init`, `MX_GPIO_Init`, `Error_Handler` | — | HAL |
| `main/Src/stm32f0xx_it.c` | Generated code | ISR vector bodies | see §5 | — | HAL |
| `main/Src/stm32f0xx_hal_msp.c` | Generated code | Per-peripheral MSP init/deinit | `HAL_I2C_MspInit/DeInit`, `HAL_RTC_MspInit/DeInit`, `HAL_TIM_Base_MspInit/DeInit` | — | HAL |
| `main/MDK-ARM/PowerSubCPU.uvprojx` | Configuration | Main build manifest | — | — | names every compiled file |
| `iap/App/entry_iap.c` | Application (IAP entry) | `main()` for IAP: vector-table relocation to SRAM, then update loop | `main` | `VectorTable[48] @0x20000000` | `km_i2c_iap.h`, BSP |
| `iap/App/km_i2c_iap.c` | Protocol | IAP I2C/flash engine | `i2c_recv_first`, `i2c_recv_wait`, `flash_write_32`, `main_project_flash_erase`, `check_sum_calc`, `checksum_calc` | simplified `i2c_info` | `km_i2c_iap.h`, CMSIS-Driver I2C+Flash |
| `iap/App/km_extend_io_iap.c` | Application | Post-update reset trigger | `msw_on_proc_iap`, `km_exti_callback_iap`, `km_it_init_iap` | — | BSP, HAL NVIC |
| `iap/Inc/km_i2c_iap.h` | Configuration | IAP register map + magic number + Main-image location constants | — | `IAP_PG_COMMAND_MAGICNUM=0x11223344`, `MAIN_APPLICATION_ADDRESS=0x08000000`, `MAIN_APPLICATION_SIZE=0x5000` | — |
| `iap/Inc/km_ver_iap.h` | Configuration | IAP image version | — | `VERSION_MAJOR=0x00`, `VERSION_MINOR=0x04` | — |
| `iap/MDK-ARM/PowerSubCPU_iap.uvprojx` | Configuration | IAP build manifest | — | — | names every compiled file |
| `common/Drivers/CMSIS/Device/ST/STM32F0xx/Source/I2C_stm32f0xx.c` | Driver | CMSIS-Driver I2C implementation | `Driver_I2C0.*` (not individually read) | — | HAL (`[INFERRED]`) |
| `common/Drivers/CMSIS/Device/ST/STM32F0xx/Source/FLASH_stm32f0xx.c` | Driver | CMSIS-Driver Flash implementation | (not individually read) | — | HAL (`[INFERRED]`) |
| `common/Drivers/BSP/Inc/BSP_STM32F03x_Nucleo.h` | BSP | BSP public API (full contents confirmed) | — | `TimerType`, `ADCChType`, `DirectionType`, error-code bases | HAL headers |
| `common/Drivers/BSP/Src/BSP_*.c` (6 files) | BSP | GPIO/TIM/WDT/PWR/ADC/Reg implementations | `[UNKNOWN]` — not opened | — | HAL (`[INFERRED]`) |
| `common/Test/cubemx/.../entry.c` | Application (test) | Standalone hardware self-test `main()` | `main`, `led2_on` | `b1_ev_tbl[]` | `test_*.h`, BSP |

---

## 5. Find: startup, entry points, ISRs, callbacks

- **`main()` (Main image):** `main/App/entry.c`. Confirmed full body:
  ```c
  int32_t main(void) {
      TYPE_Model model = TYPE_Model_None;
      __HAL_RCC_PWR_CLK_ENABLE();
      MX_Init();
      model = get_model();
      g_checksum_value = checksum_calc();
      GetVersionString();
      BSP_WDT_Start();
      internal_sts_init();
      km_it_init();
      BSP_TIM_Start(TIMER_3, 1);           /* 1 ms tick starts here */
      adc_moni_5v_start_proc();            /* dead — #if 0'd body */
      io_extend_memory_write();
      BSP_GPIO_WritePin(_RESET_GPIO_Port, _RESET_Pin, GPIO_PIN_RESET);
      s800_power_on();
      i2c_recv_first();
      poweroff_timer_init();
      while (1) {
          sb_reset_proc();
          moni_24v_proc();
          BSP_WDT_Refresh();
          msw_on_proc(STATE_NORMAL);
          power_monitor_proc(STATE_NORMAL);
          sleep_status_rem_proc(model);
          mc_p_on_proc(model);
          ir_p_on_proc();
          mc3_3von_moni_proc();
          erp_sensor_proc();
          mc_oe_proc();                    /* confirmed empty stub */
          mc_pwr_en_proc();
          adc_moni_5v_off_proc();          /* dead — #if 0'd body */
          rst_usb_proc(STATE_NORMAL);
          usb2_oe_proc();
          ap_power_en_proc(STATE_NORMAL);
          intr_pending_proc();
          anti_chattering_proc();
          poweroff_timer_proc();
          backupwait_timer_proc();
          i2c_recv_wait();
          sleep_mode();
      }
      return 0;
  }
  ```
  This exact ~22-call fixed order is PS-CPU's entire runtime behavior outside of ISRs.

- **`main()` (IAP image):** `iap/App/entry_iap.c`. Confirmed full body:
  ```c
  int32_t main(void) {
      uint32_t i = 0;
      MX_Init();
      g_checksum_value = checksum_calc();
      GetVersionString();
      BSP_WDT_Start();
      km_it_init_iap();
      for (i = 0; i < 48; i++) {
          VectorTable[i] = *(__IO uint32_t*)(IAP_ADDRESS + (i << 2));   /* relocate vector table to SRAM */
      }
      BSP_WDT_Refresh();
      __HAL_RCC_SYSCFG_CLK_ENABLE();
      __HAL_SYSCFG_REMAPMEMORY_SRAM();     /* remap SRAM to 0x00000000 */
      i2c_recv_first();
      while (1) {
          i2c_recv_wait();
          BSP_WDT_Refresh();
      }
      return 0;
  }
  ```

- **`main()` (hardware self-test image, not shipped):** `common/Test/cubemx/.../entry.c` — see §3.16.

- **Reset handler:** the Cortex-M0 reset vector in `startup_stm32f031x6.s` (§3.1) — not individually disassembled this session; standard CMSIS pattern is assumed (`[INFERRED]`) to zero `.bss`, copy `.data`, call `SystemInit()`, then branch to `main`/`__main`.

- **Startup functions:** `SystemInit()` (`system_stm32f0xx.c`), `SystemClock_Config()`/`SystemClock_Config2()` (`mx_init.c`).

- **RTOS initialization:** **None — confirmed absent** (§0).

- **Task creation:** **None — confirmed absent**, no RTOS.

- **Interrupt handlers** (all confirmed directly from `main/Src/stm32f0xx_it.c`):
  | Vector | Body |
  |---|---|
  | `NMI_Handler` | empty |
  | `HardFault_Handler` | infinite loop (no recovery) |
  | `SVC_Handler` | empty (unused — no RTOS) |
  | `PendSV_Handler` | empty (unused — no RTOS) |
  | `SysTick_Handler` | `HAL_IncTick()` + a `SYSTICK_IRQHandler`-style hook |
  | `RTC_IRQHandler` | `HAL_RTC_AlarmIRQHandler(&hrtc)` → dispatches to `HAL_RTC_AlarmAEventCallback()` in `km_it.c` |
  | `EXTI0_1_IRQHandler` / `EXTI2_3_IRQHandler` / `EXTI4_15_IRQHandler` | `HAL_GPIO_EXTI_IRQHandler(<pin>)` per line, ultimately reaching `km_exti_callback()` in `km_it.c` |
  | `TIM3_IRQHandler` | `HAL_TIM_IRQHandler(&htim3)` → `km_tim_callback()` in `km_it.c` (the 1 ms tick) |
  | `I2C1_IRQHandler` | branches to error vs. event handling based on `BERR`/`ARLO`/`OVR` flags, ultimately reaching the CMSIS-Driver's I2C ISR logic and `rx_comp()` in `km_i2c.c` |
  IAP's own `stm32f0xx_it.c` (downloaded, `[UNKNOWN]` body) almost certainly implements a subset of the same table — at minimum `I2C1_IRQHandler` and whatever EXTI line serves `MSW_ON` (per `km_extend_io_iap.c::km_exti_callback_iap()`) — `[INFERRED]`.

- **Timer callbacks:** `km_tim_callback()` (`km_it.c`, TIM3 1 ms tick — decrements all 16 software timer slots).

- **DMA callbacks:** **None found.** No `DMA_HandleTypeDef`, `HAL_DMA_*`, or `__HAL_DMA_*` symbol appeared in any file this session read; I2C is interrupt-driven through the CMSIS-Driver, not DMA-backed. `[INFERRED]` that this firmware uses no DMA at all — not every HAL source file was opened, so this is not an exhaustive negative proof, but no positive evidence of DMA use was found anywhere across two sessions of reading.

- **Peripheral initialization:** `MX_Init()` → `SystemClock_Config()`, `MX_GPIO_Init()`, `MX_I2C1_Init()`, `MX_IWDG_Init()`, `MX_RTC_Init()`, `MX_TIM3_Init()` (all `main/Src/mx_init.c`, §3.5).

- **Application entry points:**
  - Main image: `entry.c::main()` (production power-sequencing firmware).
  - IAP image: `entry_iap.c::main()` (field firmware-update mode).
  - Test harness: `common/Test/cubemx/.../entry.c::main()` (not shipped).
  - Secondary "entry point" worth flagging: `jump2iap()` in `entry.c` is a *candidate* second way into the IAP world from within the Main image's own address space, but per §3.12's finding, no caller was found — its actual role in the shipped product is `[UNKNOWN]`.

---

## 6. MODULE DEPENDENCY MATRIX

| Module | Depends On | Used By | Context |
|---|---|---|---|
| Startup & CMSIS system init | — (bottom of stack) | Everything | Reset→`SystemInit()`→`main()` handoff for both images; only `startup_stm32f031x6.s` is actually compiled (confirmed via both `.uvprojx` Groups) |
| STM32 HAL library | CMSIS core/device headers | Generated code (`Src/*.c`), BSP (`[INFERRED]`), CMSIS-Driver (`[INFERRED]`) | ST's register abstraction; never called directly from `App/` code, which goes through BSP or CMSIS-Driver instead |
| CMSIS-Driver (I2C, Flash) | STM32 HAL (`[INFERRED]`) | `km_i2c.c`, `km_i2c_iap.c`, `km_extend_io.c` (`i2c_reset`) | `ARM_DRIVER_I2C`/`ARM_DRIVER_FLASH`; the only path by which either image touches I2C1 or internal Flash for programming |
| BSP | STM32 HAL (`[INFERRED]`) | `entry.c`, `entry_iap.c`, `km_extend_io.c`, `km_it.c`, `km_extend_io_iap.c` | KM's own GPIO/Timer/WDT/PWR/ADC/Reg facade; the near-universal hardware-access path for `App/` code |
| Generated code (`Src/*.c`, per image) | STM32 HAL, CMSIS | `entry.c`/`entry_iap.c` (`MX_Init`), `km_it.c` (ISR routing), `km_alarm_wake.c` (`SystemClock_Config2`) | CubeMX output, hand-patched (e.g. `KM_RTC_Restore`'s timeout guard) |
| Keil project files / top-level config | names the whole tree | Build system only | Device target, memory map (Main 0x08000000/20 KB, IAP 0x08005000/12 KB), exact per-image compiled-file lists, RTE (confirms no RTOS) |
| Main I2C protocol engine (`km_i2c.c`) | CMSIS-Driver I2C, `km_extend_io.h`, `km_it.h`, `km_ver.h` | `entry.c` (`main()` loop), CMSIS-Driver ISR (`rx_comp` callback) | PS-CPU's entire external I2C register-map contract; ~40-case dispatcher in `i2c_recv_wait()` |
| Main interrupt/timer/debounce (`km_it.c`) | `km_extend_io.h`, BSP (via debounce reads) | `entry.c` (init), `km_extend_io.c` (nearly every `_proc()`), HAL ISR callbacks | Software timers (16 slots), pending-factor deferral, ref-counted IRQ mask, debounce primitives |
| RTC alarm/wake (`km_alarm_wake.c`) | `mx_init.c` RTC helpers, HAL RTC | `km_extend_io.c::msw_on_proc()` | Arms Alarm A for STOP-mode wake; only call site found is the `MSWOFF_STOP_MODE` branch |
| CA72 status tracker (`km_ca72_status.c`) | `km_extend_io.h` (enum) | `km_i2c.c` (`[INFERRED]` setter), `km_extend_io.c` (`sleep_status_rem_*_proc`) | Cached 3-state SoC power view |
| ADC monitor (`km_adc.c`) | BSP_ADC (`[INFERRED]`, unreachable) | `entry.c` (calls dead stubs) | Fully `#if 0`'d — dead code, harmless no-op call sites remain |
| Main power-sequencing application (`km_extend_io.c`) | BSP, `km_it.c`, `km_ca72_status.c`, `km_i2c.c` (event records), `km_alarm_wake.c`, HAL (`NVIC_SystemReset`, RCC I2C reset) | `entry.c::main()` loop | The core product logic: power on/off, sleep/wake, model/generation dispatch, I/O-extension map |
| Main entry (`entry.c`) | `km_extend_io.h`, `km_i2c.h`, `km_it.h`, `km_adc.h`, `km_ca72_status.h`, BSP, `MX_Init` | CPU reset vector (it *is* `main()`) | Orchestrates the fixed 22-call superloop; also holds unreferenced `jump2iap()` (open finding) |
| IAP entry (`entry_iap.c`) | BSP, `km_i2c_iap.h`, HAL SYSCFG remap | CPU reset vector (IAP image's `main()`) | Vector-table relocation to SRAM, then a 2-call update loop |
| IAP I2C/flash engine (`km_i2c_iap.c`) | CMSIS-Driver I2C+Flash | `entry_iap.c::main()` | Simplified register map, magic-number-triggered flash erase/write, dual checksum schemes |
| IAP GPIO glue (`km_extend_io_iap.c`) | BSP, HAL NVIC | `entry_iap.c::main()` (init), EXTI ISR | `MSW_ON`-triggered `HAL_NVIC_SystemReset()` — the confirmed mechanism for leaving IAP mode back to Main |
| Hardware self-test harness (`common/Test/cubemx`) | BSP, its own `test_*` modules (`[UNKNOWN]`) | Not used by production firmware | Standalone bring-up image, own Keil project, own `main()` |

---

## 7. Notes for the next reading session

- **Highest-value unread material:** the 6 `BSP_*.c` implementation files (especially `BSP_PWR_STM32F03x_Nucleo.c`, to confirm exactly what `BSP_PWR_enter_stopmode()`/`_sleepmode()` do at the register level) and IAP's `Src/*.c` generated files (to confirm whether IAP's `mx_init.c` really omits RTC/ADC init as its HAL-Group membership implies).
- **Two genuine open questions this session could not resolve, flagged rather than guessed at:** (1) how the Main image actually transitions into IAP mode, given `jump2iap()` appears to have no caller (§3.12); (2) the exact meaning of IAP's `.uvprojx` `<TextAddressRange>0x08000000</TextAddressRange>` disagreeing with its own `OCR_RVCT4` ROM base of `0x08005000` (§3.6).
- **Two pre-existing documentation-debt items worth fixing if anyone touches these files:** `km_ca72_status.c`'s stale internal header (`km_s800_status.c`) and include-guard (`__KM_S800_STATUS_H`), and `km_alarm_wake.h`'s copy-pasted include-guard (`__KM_ADC_H`).
- **Confidentiality reminder (carried from `00_project_scope.md`):** this repository is public; keep excerpts illustrative rather than reproducing entire proprietary source files verbatim.
