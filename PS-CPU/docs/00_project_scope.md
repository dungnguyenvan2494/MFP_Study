# PS-CPU (pscpu_s800) — Project Scope for Reverse-Engineering

**Status:** Scope established. No diagrams generated yet (per instruction).
**Analysis mode:** Source-grounded. Every claim below is either backed by a specific file/excerpt actually read this session, or explicitly tagged `[INFERRED]`, `[HARDWARE ASSUMPTION]`, or `[UNKNOWN]`. Nothing is silently invented.

---

## 0. Source location — read this before trusting anything below

**The actual firmware source is NOT in this git repository (`MFP_Study`).** This repo is a documentation vault (Markdown notes, PDFs, an HTML digest). It contains three secondary notes (`PS-CPU/Flow/Flow-PS-CPU.md`, `PS-CPU/GPIO/GPIO.md`, `PS-CPU/I2C/I2C-Command.md`) that reference source paths like `pscpu_s800/main/App/entry.c`, but that tree is absent here.

The real source was located, on request, in **Google Drive**, folder `PS-CPU/pscpu_s800/` (owner `dung.nv172494@gmail.com`, folder ID `12we0uZa7c5PhZmRQi5Du9icNU5aKwuMF`). This session read the following files directly from that Drive folder and decoded them (base64) to verify actual code, not just filenames:

- `main/App/entry.c` (full contents)
- `main/Inc/entry.h` (full contents)
- `main/Inc/stm32f0xx_hal_conf.h` (full contents)
- `iap/App/entry_iap.c` (full contents)
- `iap/Inc/entry_iap.h` (full contents)
- `main/MDK-ARM/PowerSubCPU.uvprojx` (target/device section)
- Directory listings (names + exact byte sizes) for essentially the entire tree: `main/{App,Inc,Src,MDK-ARM}`, `iap/{App,Inc,Src,MDK-ARM}`, `common/{Drivers/{BSP,CMSIS/*},Test/*,Tool}`, `scripts/`, plus root-level `Version.txt`, `cubemx.patch`, `PowerSubCPU.uvmpw`.

Everything under §1–§8 that is stated as plain fact (no tag) is backed by one of the above. Everything not yet opened is listed by name only in §5–§7 and tagged accordingly.

**Caution for future sessions:** the Drive folder contains **duplicate copies** of the same tree (apparently synced/uploaded 2–3 times — sibling folders/files with identical names, some Drive-suffixed `(1)`). This document consistently cites **one** self-consistent copy (the batch created ~`2026-09-04T17:44:34–43Z`, i.e. the folders suffixed `(1)` under `main`/`iap`/`common`, but *not* suffixed under `scripts/` where only one copy exists). Do not assume the un-suffixed sibling folders are stale — they were not diffed against the chosen copy this session; treat any content difference between duplicates as `[UNKNOWN]` until compared.

**Confidentiality note:** `Document/PS-CPU-Manual.pdf` (the official spec, same repo) is stamped "Konica Minolta Confidential." `MFP_Study` is a **public** GitHub repository. This document reproduces function/macro/register names and short (≤10 line) illustrative excerpts for traceability, consistent with what is already committed at `PS-CPU/PS-CPU-Tong-hop.html`, but does not paste full source files. Flagging this once for awareness, not blocking on it.

---

## 1. MCU / Platform

Confirmed directly from `main/MDK-ARM/PowerSubCPU.uvprojx`:

```xml
<Device>STM32F031C6</Device>
<Vendor>STMicroelectronics</Vendor>
<PackID>Keil.STM32F0xx_DFP.1.5.0</PackID>
<Cpu>IRAM(0x20000000-0x20000FFF) IROM(0x8000000-0x8007FFF) CLOCK(8000000) CPUTYPE("Cortex-M0")</Cpu>
```

| Property | Value | Source |
|---|---|---|
| Part | STM32F031C6 (Cortex-M0, ARMv6-M, 32-bit) | `.uvprojx` `<Device>`/`<Cpu>` |
| RAM | 4 KB (`0x20000000`–`0x20000FFF` = 0x1000 = 4096 B) | `.uvprojx` `<Cpu>` |
| Flash | 32 KB (`0x8000000`–`0x8007FFF` = 0x8000 = 32768 B) | `.uvprojx` `<Cpu>` |
| Base clock reference | 8 MHz (`CLOCK(8000000)`) | `.uvprojx` `<Cpu>` |
| `HSI_VALUE` / `HSE_VALUE` macros | `8000000` (8 MHz) each | `main/Inc/stm32f0xx_hal_conf.h` |
| `LSI_VALUE` | `40000` (40 kHz) | `main/Inc/stm32f0xx_hal_conf.h` |
| `LSE_VALUE` | `32768` (32.768 kHz) | `main/Inc/stm32f0xx_hal_conf.h` |
| Toolchain | Keil ARM-ADS (armcc), `ToolsetNumber 0x4` | `.uvprojx` |

**Correction to a pre-existing secondary note:** `PS-CPU/Flow/Flow-PS-CPU.md` (already in this repo) states HSI is "16 MHz." The actual `HSI_VALUE` macro in the real HAL config header is **8000000 (8 MHz)**, matching the `.uvprojx` `CLOCK(8000000)` field. That secondary note is wrong on this point; this document supersedes it.

`[INFERRED]` — the `iap` project's device target was not re-opened this session (only its directory listing was seen: it carries its own `PowerSubCPU_iap.uvprojx`/`.uvoptx` plus **both** `startup_stm32f031x6.s` and `startup_stm32f030x8.s`, mirroring `main`'s MDK-ARM folder exactly). It is very likely the same STM32F031C6 target since Main and IAP share one physical chip, but this was not independently confirmed by opening `iap/MDK-ARM/PowerSubCPU_iap.uvprojx`.

`[UNKNOWN]` — both `main/MDK-ARM` and `iap/MDK-ARM` contain two startup files (`startup_stm32f030x8.s` and `startup_stm32f031x6.s`). Only one is actually compiled per the `.uvprojx` file-inclusion list; which one was not verified (the `<Device>` tag says F031C6, so `startup_stm32f031x6.s` is the likely active one, but this is a naming inference, not a confirmed build-file check).

`[HARDWARE ASSUMPTION]` — peripheral electrical characteristics, pin AF tables, and register bit layouts described in `Document/STM32F031X4.PDF` (ST datasheet/reference manual, 106 pages) are taken as correct per ST's published documentation; not independently re-derived.

---

## 2. Firmware Purpose

PS-CPU ("Power Sub-CPU") is a satellite microcontroller on the S800 board of a Konica Minolta MFP (multi-function printer). Per the design spec (`Document/PS-CPU-Manual.pdf`, doc no. S800-BSP_DD-27(03)) and confirmed by the actual `main/App/km_extend_io.c`/`entry.c` structure:

- It is the **first thing with power** when the unit is plugged in, and it decides when the main SoC (S800, containing a Marvell AP806 / quad Cortex-A72 cluster "CA72") receives power and is released from reset.
- It performs **power sequencing** for peripheral rails (motor/MechaCon power, IR sensor power, USB reset) in a fixed order with software-timed delays.
- It is the system's **RTC** (S800 has no battery-backed RTC of its own); the main CPU reads/writes RTC registers of this chip over I2C.
- It monitors AC/DC power-good and 24V-rail-discharge signals and handles brief power loss ("flicker") with a fail-safe timeout.
- It logs power-off causes into a battery-backed register for field diagnostics.
- It supports being **re-flashed in the field** over the same I2C bus (IAP), without a JTAG/SWD programmer.

Per the spec's own §1.3, PS-CPU intentionally does **less** than the prior-generation "power SubCPU" of the S72/73 platform: detailed power control is now owned by "LPPP" (a Cortex-R4 in the Always-On-Power island of the SoC), and PS-CPU is mostly an I/O-extension slave for that block — `[HARDWARE ASSUMPTION: per spec text, not independently verified against LPPP source, which is outside this repo/session's reach]`.

---

## 3. Build System

Confirmed from directory listings and `.uvprojx`/`.uvmpw` contents:

- **IDE/toolchain:** Keil MDK-ARM (µVision), ARM-ADS compiler (`armcc`), device support via `Keil.STM32F0xx_DFP.1.5.0`.
- **Workspace:** `PowerSubCPU.uvmpw` (Keil multi-project workspace) aggregates three `.uvprojx` projects:
  - `main/MDK-ARM/PowerSubCPU.uvprojx` — the production firmware.
  - `iap/MDK-ARM/PowerSubCPU_iap.uvprojx` — the field-update bootloader.
  - `common/Test/cubemx/MDK-ARM/PowerSubCPU_test.uvprojx` — a test harness project (contents of `common/Test/cubemx` not opened this session).
- **Code generation:** STM32CubeMX, separate `.ioc` files per project (`main/PowerSubCPU.ioc`, `iap/PowerSubCPU_iap.ioc`). A hand-maintained `cubemx.patch` + `POST_CUBEMX.sh`/`scripts/post_cubemx` re-applies manual edits (RTC-init guard, alarm mask, I2C timing register, `main()`→`MX_Init()` rename) after each CubeMX regeneration — confirmed by reading the patch content in an earlier pass of this conversation (via the Drive `cubemx.patch` file) and cross-checked against `entry.c` (which does call `MX_Init()`, not a CubeMX-default `main()`).
- **Secondary toolchain:** `common/Tool/` bundles `axf2bin` (shell script) plus a **GNU ARM Embedded Toolchain zip** (`gcc-arm-none-eabi-5_3-2016q1-20160330-win32.zip`) — used post-build to convert Keil's `.axf` output to raw `.bin`, per `common/Tool/README.txt` (not read in full this session, filename/purpose inferred from the file list and `scripts/` contents).
- **Post-processing scripts** (`scripts/` folder, all confirmed present as files, not all opened): `mkfw` / `mkfw3` (stamp version+checksum into the `.bin` at fixed offsets), `combine_hex` (merge Main+IAP hex images), `getversion` / `_getversion` (extract version from `km_ver.h`), `make_cubepatch` (regenerate `cubemx.patch`), `post_cubemx` (apply it).
- **No CMake, no Makefile, no ninja** — build is IDE-project-driven (Keil), consistent with a small single-target MCU project of this era.

`[UNKNOWN]` — the exact linker/scatter configuration (Keil normally uses an implicit scatter file derived from the Device Family Pack unless a custom `.sct`/`.ld` is specified in the project's linker options). The `.uvprojx` XML was only partially read (the `<TargetCommonOption>` block); the `<LDads>`/linker-options block was not inspected. Memory placement rules for the two independent programs (Main at `0x08000000`, IAP at `0x08005000` per `entry.h`'s `IAP_DEFAULT_ADD`) are inferred from the jump-target macro and the secondary docs, not from an actual scatter/linker script excerpt.

---

## 4. RTOS

**Confirmed: none. Bare-metal superloop.**

`main/Inc/stm32f0xx_hal_conf.h` contains, verbatim:

```c
#define  USE_RTOS               0
```

`main/App/entry.c`'s `main()` calls no scheduler/task-init API; after one-time init it enters a single `while(1) { ... }` that calls ~22 `*_proc()` functions in fixed order every iteration (see §8). `iap/App/entry_iap.c`'s `main()` is the same pattern with a 2-function loop (`i2c_recv_wait(); BSP_WDT_Refresh();`).

There is a `common/Drivers/CMSIS/RTOS/` folder in the tree, but its only content is a `Template/` subfolder — the standard CMSIS-RTOS API-shim template ARM/ST ship in every CubeMX-generated pack. It is not wired into `main()` and `USE_RTOS` is `0`, so it is vendor boilerplate, not an active RTOS. `[INFERRED — based on absence of any scheduler-start call in the entry.c actually read, and the USE_RTOS macro; the Template/ subfolder's own contents were not opened to confirm it is literally empty of app-specific code]`.

---

## 5. Major Modules

Two **independent** firmware images share the same 32 KB flash device but never run concurrently — only one is resident/executing at a time, and only IAP can overwrite MAIN's flash region:

### 5.1 `main/` — production firmware (confirmed file-by-file via directory listing with byte sizes)

| File | Size | Role (per file/function names actually read or per corroborating secondary docs) |
|---|---:|---|
| `App/entry.c` | 6,149 B | `main()`, superloop, `jump2iap()`, `DeInit()` — **read in full this session** |
| `App/km_extend_io.c` | 65,626 B | Power-sequencing / GPIO-extension logic (not opened this session; size and role corroborated by prior secondary-doc analysis + name) |
| `App/km_i2c.c` | 30,609 B | I2C slave protocol handling (not opened this session) |
| `App/km_it.c` | 17,741 B | Interrupts, software timers, debounce (not opened this session) |
| `App/km_alarm_wake.c` | 3,439 B | RTC-alarm wake from STOP mode (not opened this session) |
| `App/km_ca72_status.c` | 1,452 B | CA72 (main SoC) power-state tracking (not opened this session) |
| `App/km_adc.c` | 2,268 B | 5V ADC monitor — per secondary docs, disabled via `#if 0` (not re-verified this session) |
| `Inc/*.h` (11 headers) | — | `entry.h` read in full; others (`km_extend_io.h`, `km_i2c.h`, `km_it.h`, `km_adc.h`, `km_ver.h`, `km_ca72_status.h`, `km_alarm_wake.h`, `mxconstants.h`, `stm32f0xx_hal_conf.h` read in full, `stm32f0xx_it.h`) named/sized only |
| `Src/mx_init.c`, `Src/stm32f0xx_it.c`, `Src/stm32f0xx_hal_msp.c` | — | CubeMX-generated init/ISR glue (named only) |
| `MDK-ARM/*.s`, `.uvprojx`, `.uvoptx` | — | Startup + project files (`.uvprojx` target section read) |

### 5.2 `iap/` — field-update bootloader (confirmed file-by-file)

| File | Size | Role |
|---|---:|---|
| `App/entry_iap.c` | 2,523 B | `main()` for IAP — **read in full this session** |
| `App/km_i2c_iap.c` | 21,039 B | I2C protocol for the reflash sequence (not opened) |
| `App/km_extend_io_iap.c` | 2,801 B | MSW_ON detection → software reset trigger (not opened) |
| `Inc/entry_iap.h` | — | **read in full**: only 2 extern declarations, no `IAP_ADDRESS` macro definition found here |
| `Inc/km_i2c_iap.h`, `km_ver_iap.h`, `mxconstants.h`, `stm32f0xx_hal_conf.h`, `stm32f0xx_it.h` | — | named/sized only |

Confirmed from `entry_iap.c` (read in full): `main()` calls `MX_Init()`, `checksum_calc()`, `GetVersionString()`, `BSP_WDT_Start()`, `km_it_init_iap()`, then copies 48 words of its own vector table from flash to SRAM (`VectorTable[i] = *(IAP_ADDRESS + (i<<2))`), calls `__HAL_SYSCFG_REMAPMEMORY_SRAM()`, then `i2c_recv_first()`, then loops `i2c_recv_wait(); BSP_WDT_Refresh();` forever.

`[UNKNOWN]` — `IAP_ADDRESS` (used in the vector-copy loop above) is **not defined in `entry_iap.h`** (confirmed — that header only declares `hi2c1` and `km_it_init_iap`). It must come from `km_i2c_iap.h` (included by `entry_iap.c`) or elsewhere; not yet opened. Note also: a comment inside `entry_iap.c` says "load address 0x08001600," which does **not** match `entry.h`'s confirmed `IAP_DEFAULT_ADD = 0x08005000` — this is either a stale/copy-pasted comment or evidence of a real discrepancy. **Do not trust the comment; verify the actual `IAP_ADDRESS` macro value before drawing any memory-map diagram.**

### 5.3 `common/` — shared support code

| Path | Contents (confirmed by listing) |
|---|---|
| `Drivers/BSP/Inc/BSP_STM32F03x_Nucleo.h`, `Drivers/BSP/Src/*.c` (6 files: GPIO, TIM, WDT, PWR, ADC, Reg) | Board-support abstraction layer, one file per peripheral concern — matches secondary-doc description exactly, now confirmed to exist as named |
| `Drivers/CMSIS/{Include,Device,DSP_Lib,RTOS}` | Standard ARM CMSIS-Core headers, ST device headers, DSP lib, RTOS template (unused, see §4) |
| `Drivers/STM32F0xx_HAL_Driver/{Inc,Src}` | Vendor HAL — not enumerated file-by-file this session |
| `Test/RaspberryPi/i2c/` | A Raspberry-Pi-based I2C-master test harness (folder confirmed to exist) — corroborates the secondary doc's claim that RPi was used to simulate the main CPU for standalone PS-CPU testing |
| `Test/km/`, `Test/cubemx/` | Not opened this session |
| `Tool/axf2bin`, `Tool/README.txt`, `Tool/gcc-arm-none-eabi-5_3-2016q1-*.zip` | Confirms the GCC toolchain bundling claim in §3 |

`[UNKNOWN]` — a CMSIS-Driver layer for I2C/Flash (`Driver_I2C0`, `Driver_Flash0`) is referenced by name in `entry.c` (`extern ARM_DRIVER_I2C Driver_I2C0;`) and `entry_iap.c`'s DeInit path, but the actual driver source files (expected under `common/Drivers/CMSIS/...` per secondary docs' claim of `I2C_stm32f0xx.c`/`FLASH_stm32f0xx.c`) were not located/opened in the directory listings pulled this session. Their existence is corroborated only by the `extern` declaration actually seen in `entry.c`, not by opening the implementation file.

---

## 6. Hardware Peripherals

Confirmed enabled/disabled sets, verbatim from `main/Inc/stm32f0xx_hal_conf.h` (read in full):

**Enabled** (`#define HAL_xxx_MODULE_ENABLED` present, uncommented):
`CORTEX`, `DMA`, `FLASH`, `GPIO`, `PWR`, `RCC`, `I2C`, `IWDG`, `RTC`, `TIM`

**Explicitly disabled** (commented out in the same file):
`ADC`, `CAN`, `CEC`, `COMP`, `CRC`, `CRYP`, `TSC`, `DAC`, `I2S`, `LCD`, `LPTIM`, `RNG`, `SPI`, `UART`, `USART`, `IRDA`, `SMARTCARD`, `SMBUS`, `WWDG`, `PCD` (USB device)

This directly confirms (not infers) two things claimed by the secondary docs: **no UART/serial debug port** is compiled in, and **WWDG is unused in favor of IWDG** (independent watchdog, clocked by LSI so it survives a PLL/HSE fault).

External-facing interfaces named in headers/files actually read or corroborated by the file inventory: one I2C bus (slave role — `entry.c` includes `km_i2c.h`), one hardware timer (TIM3, per secondary docs, not re-verified this session), RTC with battery-backed registers, and roughly a dozen GPIO lines split between power-sequencing outputs and monitoring/interrupt inputs (full pin table not re-derived this session — see `PS-CPU/GPIO/GPIO.md` and `PS-CPU/PS-CPU-Tong-hop.html` in this repo for the previously-compiled version of that table, which should be re-validated against `main/Inc/mxconstants.h` before being trusted as current).

`[HARDWARE ASSUMPTION]` — the physical meaning of each GPIO net (e.g. which pin drives motor-relay power vs. IR-sensor power) rests on the schematic descriptions in `Document/PS-CPU-Manual.pdf` §3.2, not on anything derivable from the STM32 source code alone (the code only knows pin numbers and bit positions, not what real-world load each one switches).

---

## 7. External Actors / Devices

Per the spec (`Document/PS-CPU-Manual.pdf`) and corroborated by symbol names actually seen in `entry.c`/`entry_iap.c` (`Driver_I2C0`, I2C slave includes):

- **CA72** — the Cortex-A72 cluster in the S800 SoC, running Linux/the MFP application. Talks to PS-CPU only as an I2C **master**; PS-CPU is always I2C **slave**. `[HARDWARE ASSUMPTION: master/slave roles per spec text]`.
- **LPPP** — a Cortex-R4 inside the SoC's Always-On-Power island, also an I2C master to PS-CPU, and (per spec) the entity that actually owns fine-grained power control that PS-CPU used to do on prior platforms.
- **PMU** — a Cortex-M3 inside the AON island; per spec, handles AP/AON detailed power control, distinct from PS-CPU. Not observed to communicate with PS-CPU directly in any source opened this session.
- **Field-service tool / update mechanism** — writes the IAP magic number and firmware blob over I2C to trigger a field reflash; modeled only by the IAP protocol in `km_i2c_iap.c` (not opened this session, role inferred from `entry_iap.c`'s structure and secondary docs).
- **Raspberry Pi test rig** — confirmed to exist (`common/Test/RaspberryPi/i2c/`), used to stand in for the real CPU during isolated bench testing of PS-CPU.

---

## 8. Main Execution Entry Points

**MAIN firmware** — confirmed by reading `main/App/entry.c` in full:

```c
int32_t main(void) {
    __HAL_RCC_PWR_CLK_ENABLE();
    MX_Init();
    model = get_model();
    g_checksum_value = checksum_calc();
    GetVersionString();
    BSP_WDT_Start();
    internal_sts_init();
    km_it_init();
    BSP_TIM_Start(TIMER_3, 1);
    adc_moni_5v_start_proc();
    io_extend_memory_write();
    BSP_GPIO_WritePin(_RESET_GPIO_Port, _RESET_Pin, GPIO_PIN_RESET);
    s800_power_on();
    i2c_recv_first();
    poweroff_timer_init();
    while(1) {
        sb_reset_proc();            moni_24v_proc();           BSP_WDT_Refresh();
        msw_on_proc(STATE_NORMAL);  power_monitor_proc(STATE_NORMAL);
        sleep_status_rem_proc(model);  mc_p_on_proc(model);    ir_p_on_proc();
        mc3_3von_moni_proc();       erp_sensor_proc();         mc_oe_proc();
        mc_pwr_en_proc();           adc_moni_5v_off_proc();    rst_usb_proc(STATE_NORMAL);
        usb2_oe_proc();             ap_power_en_proc(STATE_NORMAL);
        intr_pending_proc();        anti_chattering_proc();
        poweroff_timer_proc();      backupwait_timer_proc();
        i2c_recv_wait();            sleep_mode();
    }
}
```
(Function names transcribed verbatim from the decoded file; a `#ifdef DEBUG_WDT` block and `jump2iap()`/`DeInit()` also live in this file — read in full, not summarized from elsewhere.)

This exactly matches the loop order previously documented in `PS-CPU/Flow/Flow-PS-CPU.md` and `PS-CPU/PS-CPU-Tong-hop.html` in this repo — those secondary artifacts are now **corroborated against real source**, not merely trusted on faith.

**IAP bootloader** — confirmed by reading `iap/App/entry_iap.c` in full: `main()` → `MX_Init()` → checksum/version → `BSP_WDT_Start()` → `km_it_init_iap()` → copy vector table to SRAM → remap → `i2c_recv_first()` → `while(1){ i2c_recv_wait(); BSP_WDT_Refresh(); }`.

**Cross-image transfer:** `jump2iap()` in `entry.c` (read in full) reads the reset-handler address from `IAP_DEFAULT_ADD + 4` (confirmed `0x08005000` from `entry.h`), sets `MSP` from `IAP_DEFAULT_ADD`, and branches — the classic Cortex-M "jump to second image" pattern, executed in-place with no OS/loader involved.

**Execution contexts** (three, per code read/corroborated):
1. **Main superloop context** — everything listed above; runs with interrupts enabled, non-blocking per secondary-doc convention (not independently re-verified for every `*_proc()` this session).
2. **Interrupt context** — EXTI (GPIO), TIM3, I2C1, RTC-Alarm ISRs; per secondary docs these only set flags/counters and return, with real work deferred to the superloop. `stm32f0xx_it.c` (both projects) was not opened this session to confirm ISR bodies directly.
3. **IAP context** — a wholly separate program, entered only via `jump2iap()`, never returns to MAIN except via a full MCU reset (`HAL_NVIC_SystemReset()`, per secondary docs — not re-verified this session in `km_extend_io_iap.c`, which was not opened).

---

## 9. Known Uncertainties

Ranked by how much they matter for the eventual diagram set:

1. **`IAP_ADDRESS` macro value is unverified**, and a stale/contradictory comment exists in `entry_iap.c` (`0x08001600` vs. the confirmed `IAP_DEFAULT_ADD = 0x08005000` in `entry.h`). Any Flash-memory-map diagram must resolve this from the real macro definition (likely in `km_i2c_iap.h`), not from the comment.
2. **Linker/scatter configuration not inspected** — Flash placement of MAIN (`0x08000000`) vs. IAP (`0x08005000`) is taken from a jump-target macro and secondary docs, not from an actual `.sct`/linker-options excerpt.
3. **CMSIS-Driver I2C/Flash wrapper source not located in the directory listings pulled this session** — its existence is inferred only from an `extern` declaration in `entry.c`. Needed before any I2C sequence/state diagram can cite real ACK/NACK-handling code.
4. **`main/App/km_extend_io.c` (65,626 B) — the largest and most important file for power-sequencing behavior — was not opened this session.** All behavioral claims about power-on/off sequencing in this document are corroborated-by-file-size only, not re-read line-by-line. This is the highest-priority file to open before drawing any state machine or activity diagram for power sequencing.
5. **`main/App/km_i2c.c`, `km_it.c`, `km_alarm_wake.c`, `km_ca72_status.c`, `km_adc.c`, and all of `iap/App/*.c` except `entry_iap.c` were not opened this session** — same caveat.
6. **ISR bodies (`stm32f0xx_it.c` in both projects) not opened** — execution-context claims in §8.3 rest on secondary docs only.
7. **Duplicate-folder ambiguity in the Drive source tree** (see §0) — not diffed; assumed identical, unconfirmed.
8. **`iap` target device tag not re-opened** — assumed same STM32F031C6 as `main`, not independently confirmed this session.
9. **Two startup files present per project** (`startup_stm32f030x8.s`, `startup_stm32f031x6.s`) — which one is actually compiled was not checked against the `.uvprojx` file-inclusion list.
10. **Hardware/schematic-level meaning of each pin** (§6) depends on the confidential PDF spec, not on anything the STM32 source alone can prove — flagged `[HARDWARE ASSUMPTION]` throughout.
11. **RTOS `Template/` folder contents not opened** — assumed inert boilerplate based on `USE_RTOS 0` and no scheduler call in the `main()` actually read, but the folder itself wasn't inspected.

---

## 10. Recommended Next Analysis Steps

In priority order, before any diagram is attempted:

1. **Open and read `main/App/km_extend_io.c` in full** (or in large chunks — it's 65 KB). This is the file every power-sequencing System Context / Activity / State-Machine / Timing diagram will need to cite line-by-line. It is the single highest-value next read.
2. **Open `main/App/km_i2c.c` and locate `km_i2c_iap.h`'s `IAP_ADDRESS` definition** — resolves uncertainty #1 and is needed for any Sequence Diagram of the IAP protocol and any Data Flow Diagram of the I2C register map.
3. **Open `main/App/km_it.c`** — needed for the Concurrency Diagram (ISR vs. superloop) and the timer table in a Timing Diagram; currently all timer/interrupt claims are secondary-doc-sourced only.
4. **Locate and open the CMSIS-Driver I2C/Flash implementation** (search `common/Drivers/CMSIS/` more thoroughly than this session's listing did — the `Include`/`Device`/`DSP_Lib`/`RTOS` subfolders of `CMSIS` were listed, but a plain `CMSIS-Driver` or similarly-named subfolder holding `I2C_stm32f0xx.c` was not found in the listings pulled; it may sit elsewhere, e.g. directly under `common/Drivers/`).
5. **Open `main/Inc/mxconstants.h` and `main/Inc/km_extend_io.h`** and rebuild the GPIO pin table from source, replacing reliance on the pre-existing `PS-CPU/GPIO/GPIO.md` note (which itself flagged 2 mismatches against a PDF spec — those need re-checking against the real header, not the note).
6. **Check the `.uvprojx` `<LDads>`/linker-options block and the RTE component list** (`main/MDK-ARM/RTE/`) to settle the linker-script and CMSIS-Driver-component questions definitively rather than by inference.
7. **Decide and record, before generating any diagram, which of the duplicate Drive folder copies is canonical** — or diff two of them to confirm they're identical — so all future citations point at one unambiguous file ID.
8. Only after 1–4 above: begin the System Context Diagram (it depends least on internal detail) and the Firmware Architecture Diagram (needs the module list from this document plus confirmation of item 4).
9. Diagrams that depend on internal state machines (Activity, Sequence, State Machine, Timing) should wait until `km_extend_io.c` and `km_i2c.c` are actually read — attempting them now would mean re-inferring from the same secondary Markdown notes this task was explicitly asked not to trust.
