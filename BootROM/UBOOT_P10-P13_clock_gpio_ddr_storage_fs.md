# U-Boot (BootROM/Emu800/Src) — Phần 10→13: Clock/Reset/Pinctrl/GPIO · DDR · Storage · Filesystem

> Tiếp theo `UBOOT_P05-P09_board_driver_dt_memory_uart.md` (đọc §0 ở đó trước: danh tính repo, bảng build/không-build, giới hạn phân tích).
> Nhãn: **[UPSTREAM] [VENDOR] [BOARD] [DRIVER] [CONFIG]** · **FACT** = thấy trong source · **INFERRED** = suy từ Kconfig/Makefile · **UNVERIFIED** = chưa đọc tới.

---

# PHẦN 10 — CLOCK / RESET / PINCTRL / GPIO

## 10.0 Sự thật chung
Vì **Driver Model tắt** (P06 §6.1), U-Boot ở đây **không có framework clock / reset / pinctrl / gpio kiểu `uclass`**.
Không có chuỗi `consumer → uclass → provider`. Thay vào đó là **hàm C gọi thẳng thanh ghi**. Bảng dưới trả lời "subsystem này thật ra là gì":

| Subsystem | Có framework? | Thực tế trong build này |
|---|---|---|
| Clock | ❌ | vài hằng số + 1 hàm `soc_tclk_get()`; DT `tclk` gần như không dùng |
| Reset | ❌ | `reset_cpu()` (SoC) + reset cục bộ của từng IP (SDHCI, PCIe LTSSM…) |
| Pinctrl | ❌ (`MVEBU_PINCTL`, `MVEBU_MPP_BUS` **tắt**) | chỉ có "func-select" của pad ở PADWRAP, gọi qua driver GPIO |
| GPIO | ⚠ có API **legacy riêng của Quartz** (`mvqz_gpio.c`), không phải `gpiolib`/`DM_GPIO` | 272 GPIO (A–I) ở PADWRAP AON |

## 10.1 Clock
### Nguồn tần số thật sự dùng
| Đại lượng | Giá trị | Định nghĩa | Ai dùng |
|---|---|---|---|
| Generic-timer counter (`CNTFRQ`) | **25 MHz** | `COUNTER_FREQUENCY (25*1000000)` `include/configs/armada8k.h:56` | `get_tbclk()` đọc `cntfrq_el0` (`arch/arm/cpu/armv8/generic_timer.c`) ⇒ `udelay/get_timer`. (U-Boot chỉ *ghi* `cntfrq_el0` khi chạy EL3 hoặc SPL — `start.S:77`, `lowlevel_init_spl`; BL33 dưới EL2 thì **ATF đã ghi**.) |
| "TCLK/MSS" | **200 MHz** | `CONFIG_MSS_FREQUENCY (200*1000000)` `armada8k.h:57` | `soc_tclk_get()` → `soc_mss_clk_get()` (`arch/arm/cpu/armv8/armada8k/clock.c`) ⇒ chia baud cho **AP-UART** |
| Clock UART Quartz (DW-UART) | **7 380 000 Hz** | hằng cứng trong `ns16550_calc_divisor` (`ns16550.c:137`), comment: `25 MHz /(0x271/0x171)/2` (datasheet 88PAQZ01 §6.1.2.1/6.1.2.63) | UART panel + CSRC |
| Fabric/ring clock | đọc từ SAR | `soc_ring_clk_get()` → `mvebu_sar_value_get(SAR_AP_FABRIC_FREQ)` | **chỉ** dùng khi `AP806_Z_SUPPORT` (mặc định `n`) ⇒ không dùng |
| SDHCI clock | chia trong IP | `xenon_mmc_set_clk` (`xenon_mmc.c:732`) — chia clock base của SDHCI, không hỏi "clock provider" | Xenon MMC |

### DT có nói clock, nhưng gần như bị bỏ qua
```
consumer (DT)   spi0 { clock = <&tclk>; }            ← khai báo trong apn-806-z1.dtsi
      │
      ▼
soc_clock_get(blob,node)  arch/arm/cpu/mvebu-common/clock.c
      ├─ fdtdec_lookup_phandle(blob,node,"clock")   → node tclk
      └─ get_fdt_tclk() → fdtdec_get_int(node,"clock-frequency")
```
* Consumer duy nhất là `marvell,orion-spi` (`MVEBU_SPI`) — **không build**. Vậy `soc_clock_get` là code chết trong bản này.
* ⚠ DT `armada-quartz-emu800.dts` **ghi đè** `tclk` = `125000` Hz và `spi-max-frequency = 51200` — đây là **tàn dư của Palladium (bộ mô phỏng)**, không ảnh hưởng vì không ai đọc.
* Không có `clk_enable/clk_get_rate` nào; cấp/ngắt clock ngoại vi do **ATF/BLE/R4 (SCP)** thực hiện trước đó.

## 10.2 Reset
| Loại reset | Hàm / nơi | Việc làm thật |
|---|---|---|
| **Reset toàn hệ thống** | `reset_cpu()` **strong** ở `arch/arm/cpu/armv8/armada8k/soc.c:285` | ① `RFU_GLOBAL_SW_RST (0xF06F_0084)` xóa bit0 ② **[KM]** `gpio_set_value(GPIO_PORT_HRESET_REQ=66, 0)` = "Request power restart" (GPIOC[2]) — MCU PS-CPU cấp lại nguồn. *(Cùng ý với ATF `plat_pm.c` dùng `HRESET_REQn` — xem `KERNEL_STUDY_CONTEXT.md`.)* |
| (bị thay thế) | `__reset_cpu` weak ở `arch/arm/cpu/mvebu-common/misc.c:88` (ghi `MVEBU_SOFT_RESET_REG`) | **không dùng**: `reset_cpu` trong soc.c là strong, đè bản `weak alias` |
| Reset IP: SDHCI | `xenon_mmc_reset(mmc_cfg, SDHCI_RESET_ALL / CMD / DATA)` | ghi thanh ghi `SDHCI_SOFTWARE_RESET`, poll bit tự xóa |
| Reset IP: PCIe | `msb0125_pcie_host_init`: bật `APP_CTRL.LTSSM_EN` | đoạn cấu hình lane/RC-mode đã bị `#if 0` (ATF/BLE làm) |
| Reset thiết bị ngoài | `GPIO_PORT_IRIS1_RESET(74)` được **định nghĩa** (`arch-mvebu/gpio.h`) nhưng **không thấy** call-site trong `.c` của U-Boot | ⇒ reset chip Iris do phần mềm khác/HW làm |
| Reset CPU phụ | không — core phụ bật qua **PSCI ở ATF** (`armada8k/psci.S`, `CONFIG_ARMV8_PSCI=y`) | |
| Watchdog | `CONFIG_WATCHDOG` **undef** (`mvebu-common.h:91`); nhưng KM tự có `ca72wdt.c`, `initr_wd_refresh()`, `km_scpi_set_wdt_ca72_ext(...)` (WDT do R4/SCPI cấp) | `board/mvebu/armada8k/km/ca72wdt.c`, `machine_setup.c` |

Không có `reset_ctl_*`/`resets=<&…>` — DT không có thuộc tính `resets` nào được đọc.

## 10.3 Pinctrl
**Không có driver pinctrl chạy.** `soc_early_init_f()` chứa `#ifdef CONFIG_MVEBU_PINCTL → mvebu_pinctl_probe()`; defconfig để `# CONFIG_MVEBU_PINCTL=y` (comment) ⇒ khối bị loại. Nút DT `pinctl@6F008C` (`compatible="marvell,mvebu-pinctl"`, `pin-count=16`) vì vậy **vô dụng** ở U-Boot.
Cơ chế mux thật của Quartz (mỗi pad có thanh ghi `PIOCFG`):
```
consumer:   gpio_set_func_sel(pin, 0)     ← ví dụ common/board_r.c:569,573 (init_km, chân quạt)  ; 0 = "GPIO function"
   ▼
mvqz_gpio.c: gpio_set_func_sel()
   ├─ gpio_padconfig_lookup(pin) → reg = PADWRAP_AON_PIOCFG(0xE830_8000) + 4*pin      (nhóm 0..271 dùng 1 mảng pad liên tục)
   ├─ kiểm  func & 0x7 == func      (FUNC_SEL rộng 3 bit: PADWRAP_..._FUNC_SEL_MASK 0x7)
   └─ *reg = REPLACE_VAL(*reg, func)      (hoặc gpio_set_iopad(...) nếu IOPAD_WORKAROUND: ghi 2 lần + đọc thừa để tránh lỗi timing pad)
```
* Pinmux UART0/AP806 (MPP) không do U-Boot làm: `ble_main.c` (ATF) ghi `0xF06F4004/0xF06F4008 = 0x3000`; `NS16550_init` có nhánh `MVEBU_AP806_16750` để làm hộ nhưng **tắt**.
* Chỉ 2 chỗ KM gọi `gpio_set_func_sel`: hai chân quạt (`board_r.c:569,573`). Các chân khác dùng mặc định.

## 10.4 GPIO — driver `drivers/gpio/mvqz_gpio.c` (**[DRIVER] [BOARD]**)
Kconfig `MVQZ_GPIO` (depends `QUARTZ||PALLADIUM`) ⇒ `obj-$(CONFIG_MVQZ_GPIO) += mvqz_gpio_cmds.o mvqz_gpio.o`.

### Bản đồ
| GPIO # | Bank (thanh ghi) | Địa chỉ | Ghi chú |
|---|---|---|---|
| 0–31 | A | `0xE830_A000` | `HW_VER0..3` = 4..7 |
| 32–63 | B | `0xE830_A100` | phím/wake: `UTILITY_KEY 49`, `MAINSW_ON 62`… |
| 64–79 | C | `0xE830_A200` | output: `AP_PWR_EN 65`, **`HRESET_REQ 66`**, `IRIS1_RESET 74`, `POWER_OFF_PNL 79` |
| 80–111 | D | `0xE830_A300` | |
| 112–143 | E | `0xE830_A400` | quạt: `BOXFAN_HALF_REM 116`, `BOX_FAN_REM 117` |
| 144–175 | F | `0xE830_A500` | |
| 176–207 | G | `0xE830_A600` | `DDR_CONFIG 201` |
| 208–239 | H | `0xE830_A700` | `WARP_SW 228`, **`LOG_SW 233`** |
| 240–271 | I | `0xE830_A800` | `CH0/1_DDRV 262–265` (nhà sản xuất DDR), **`FW_CONFIG0..5 266–271`** |
(Tên `GPIO_PORT_*` ở `arch/arm/include/asm/arch-mvebu/gpio.h:31-91` — bọc `#if defined(KM_BIZHUB)`.)

### Thanh ghi mỗi bank (`mvqz_gpio_regs.h`, struct `PADWRAP_MAIN_GPIOF_REGS_t`)
`PLR 0x00` (mức chân, đọc) · `PDR 0x04` (hướng, đọc) · `PSR 0x08` (set output) · `PCR 0x0C` (clear output) · `SDR 0x1C` (set **direction=out**) · `CDR 0x20` (clear direction ⇒ **in**) · `HRIPR/LFIPR/ISR/…` (ngắt — **không dùng**).

### Chuỗi gọi (ví dụ thật)
```
consumer: getTermState()  (km/dipvalue.c)  → isForceLogOutput()
      │      gpio_get_value(GPIO_PORT_LOG_SW = 233)
      ▼
mvqz_gpio.c gpio_get_value(233)
      ├─ gpio_bank_lookup(233): duyệt aon_gpio_bank_config_groups[] → nhóm {208,239, PADWRAP_AON_GPIOH}
      │      regs = 0xE830_A700 ; mask = 1 << (233−208) = bit 25
      └─ return (regs->PLR & mask) ? 1 : 0                  ← đọc thanh ghi PLR
                                                            (không cần bật clock/pinmux ở U-Boot; pad đã cấu hình từ trước)
```
```
consumer: reset_cpu() → gpio_set_value(66, 0)     → bank C: regs->PCR = bit2      (kéo HRESET_REQ xuống thấp ⇒ MCU tắt/bật nguồn)
consumer: init_km     → gpio_set_direction(117, OUT) → regs->SDR = mask ; gpio_set_value(117,1) → regs->PSR = mask
consumer: get_quartz_board → gpio_set_direction(271, IN) → regs->CDR = mask ; gpio_get_value(271)
```
* Không có `gpio_request`, không có phiếu "owner"; bất cứ code nào cũng chạm được chân. Không có `gpio_to_irq`.
* Lệnh shell **`qzgpio`** (`mvqz_gpio_cmds.c:145,160`) cho phép thao tác từ prompt.
* `CONFIG_MVEBU_GPIO` (driver GPIO Marvell chuẩn `arch/arm/cpu/armv8/armada8k/gpio.c`; Kconfig `default n`, `select DM_GPIO` — và không defconfig Quartz nào bật) **tắt**, nên các đoạn `fdtdec_decode_gpio(... "sdio-vcc-gpio"/"gpio-vbus")` trong Xenon/xHCI **bị loại** (`#ifdef CONFIG_MVEBU_GPIO`).
* I2C IO-expander: các hàm `board_usb_*_init` cho IO-expander `0x21` là của DB-7040, trả sớm trên Quartz.

## 10.5 Tóm tắt "consumer → provider" thực tế
```
Clock  : ns16550_calc_divisor → soc_tclk_get() → CONFIG_MSS_FREQUENCY (hằng)                      [không có provider nào]
Reset  : reset_cpu → RFU_GLOBAL_SW_RST + GPIO HRESET_REQ → (PS-CPU M0 STM32F031 cắt/cấp nguồn)   [xem MCU_STUDY_CONTEXT.md]
Pinctrl: init_km → gpio_set_func_sel → PADWRAP PIOCFG[pin].FUNC_SEL
GPIO   : (dipvalue/quartz/soc/board_r/mfp_panel) → gpio_* → PADWRAP AON GPIO bank regs (PLR/PSR/PCR/SDR/CDR)
```

---

# PHẦN 11 — DDR / DRAM

## 11.1 Ranh giới (điều quan trọng nhất)
> **U-Boot KHÔNG khởi tạo, KHÔNG huấn luyện DDR.** DDR (LPDDR4) đã chạy xong trước khi U-Boot có mặt: nó được cấu hình + huấn luyện bởi **BLE** (Binary Extension, chạy đầu chuỗi ATF, ở SRAM on-chip `0xCCF0_0000`).

Bằng chứng phía U-Boot:
* SPL tắt: `# CONFIG_SPL is not set` (`egl_defconfig`, `km_mvebu_quartz_toc_defconfig`) ⇒ không có `arch/arm/cpu/armv8/armada8k/spl.c` (đường DDR-init kiểu Marvell chuẩn) trong build.
* `dram_init()` (`soc.c:138`) chọn **hằng**: `gd->ram_size = CONFIG_QUARTZ_RAM_SIZE (0x8000_0000)`; các nhánh `get_info(DRAM_CSx…)` (đọc `sys_info` từ BLE) nằm sau `#elif defined(CONFIG_QUARTZ_RAM_SIZE)` nên **không chạy**.
* U-Boot bắt đầu ở `0x1000` — tức đã dùng DRAM ngay ở lệnh đầu (`start.S`, SP ở `0xFF1000`).
* Comment `mvebu-common.h:71`: *"End of 16M scrubbed by training in bootrom"* — DDR training có "quét" 16 MB đầu.

Bằng chứng phía ATF (`atf/atf_s800/ble/`):
```
ble_initial_boot.S → ble_main(bootrom_flags)              ble/ble_main.c
   ├─ xóa 2 KB SRAM 0xC020_0000 (workaround OP_BTS-51229 "SuperWarp")
   ├─ get_it(); console_init(UART, …); getLogOutput()     (KM: đọc LOG_SW GPIOH[25] quyết định in log)
   ├─ plat_delay_timer_init()
   └─ lpddr4_config()                                     ble/mv_lpddr4_apn806.c:8751   ← ĐÂY là "DDR init"
```
## 11.2 Những gì BLE làm (để hiểu boundary; file `mv_lpddr4_apn806.c` ~13 778 dòng)
```
lpddr4_config()                                     :8751
  ├─ warmboot? (BOOT_SYNC_DDR_WARMBOOT)  · revA/revB (apn806_rev_id_get) · adjust_top_freq_for_pll(ddr_top_freq)
  ├─ cold boot: lặp tối đa 6 lần { lpddr4_training_Error=0; lpddr4_dynamic_config(DYNAMIC,warmboot); nếu OK break }  (ghi dấu vào SRAM 0xC0200000[0..7])
  └─ warm boot: lpddr4_dynamic_config(…, warmboot=true)         (khôi phục từ kết quả đã lưu, không train lại)

lpddr4_dynamic_config()                              :7866
  ├─ pad_cal()                              :1065   hiệu chuẩn pad (ZQ/impedance) của PHY
  ├─ power_adll(), wait_for_dll_lock()      :1246,1278
  ├─ MCConfig_ap806_dual_chan_…_x32_dpi()   :1384   nạp cấu hình Memory Controller (MC6): 2 kênh × x32, timing, địa chỉ CS
  ├─ lpddr4_ca_training()                   :6751   Command/Address training (CBT: enter_cbt/exit_cbt)
  ├─ lpddr4_write_leveling()                :7366   (wl_enable/wl_disable, applyWlDlys)
  ├─ lpddr4_dqs_gate_training()             :6011   DQS gate / preamble
  ├─ lpddr4_read_centering(FIFO_TEST, ADD_PBS…)   :3683  đọc: deskew + centering + Vref
  ├─ lpddr4_write_centering(FIFO_TEST…)     :4551   ghi: deskew + centering + Vref
  ├─ (lặp lại với DMA_TEST — huấn luyện lại bằng truy cập DMA)  DMA_start/DMA_stop, read_data_test/write_data_test
  ├─ get_ddr_density() (MR8: 4 GB hay 8 GB)  :7793 — hàm tồn tại nhưng **không thấy nơi gọi** trong file (grep chỉ có định nghĩa)
  └─ KM_CHIP_DETECT: `chip_detect_base_addr` (thanh ghi "chip detect", TRAIN_RETRY_NUM, KM_MEASUREMENT_TIME…),
                     tham số huấn luyện lưu trong QSPI (checksum/revision/backup: `calculate_checksum_qspi`, `judge_qspi_param`,
                     `backup_train_area`/`restore_train_area`) — dùng để **bỏ qua train lại** khi tham số còn hợp lệ
                     (chân `GPIO_PORT_CH0/1_DDRV` "nhà sản xuất DDR" chỉ thấy định nghĩa phía U-Boot `gpio.h`, BLE không dùng theo grep)
```
Bảng thuật ngữ:
| Từ | Ở đâu trong code |
|---|---|
| **Controller** | "MC6" (`MC6_REGS_t`, `MC_regstructs.h`), 2 channel × tối đa 4 CS |
| **PHY** | "nova" (`nova_ddrphy_write/read`, `DDR_PHY_REGS_t`, `ddr_phy_regstructs.h`) |
| **Timing** | `MCConfig_*`, `set_fsp1_parameters` (FSP), `KM_LPDDR4_config.h`/`mv_lpddr4_apn806_static.h` |
| **Training** | CA → WL → DQS gate → read/write centering → DMA re-centering |
| **Size detect** | `get_ddr_density()` (MR8) trong BLE; **U-Boot** đọc lại kết quả từ thanh ghi MC (§11.3) |
| **Memory test** | BLE: `memFill/common_memfill`, `lpddr4_memTester_shmoo`; U-Boot: lệnh `mtest` (§11.5) |
| **Frequency** | `change_clk_freq`, `change_MC_to_high_freq`, `adjust_top_freq_for_pll` |

## 11.3 Việc U-Boot **có** làm liên quan tới DDR
### (a) Đọc cấu hình MC để dựng `bd->bi_dram[]` — `dram_init_banksize()` (`soc.c:173`, nhánh `CONFIG_KM_BIZHUB`)
```
for ch in 0..1:                                   (MVEBU_MC_MAX_CH = 2)
    bi_dram[ch].start = CONFIG_SYS_DRAM_BASE(ch) = 0x0 + 0x2_0000_0000*ch   (ch1 bắt đầu ở 8 GB)
    for cs in 0..3:
        val = readl(MVEBU_MMAP_L(ch,cs))         = 0xF002_0200 + ch*0x200 + cs*8     (thanh ghi "memory map low")
        if (val & 1) {                            ← CS này có bật
            n = (val & 0x001F_0000) >> 16
            size = (n < 7) ? 0x1800_0000 << n     (0:384MB, 1:768MB, 2:1.5GB, 3:3GB, 4:6GB)
                            : 0x1_0000 << n       (7:8MB … 26:4TB)
            bi_dram[ch].size += size
        }
    gd->chk_ram_Size += bi_dram[ch].size          ← tổng RAM thật, dùng cho HW-check
[LIMIT_MEMORY_4GB (chỉ Emu800+DenebMLK) / MEMORY_6GB_TO_5GB (Sparrow MFP): ép mỗi ch ≤ 2 GB / hạ Bank0 còn 2 GB khi tổng > 5 GB]
if (readl(0xF000_1700 /*MC_RCR*/) & 1):          ← có "remap window"
    bi_dram[2].start = (readl(MC_RSBR 0xF000_1704) & 0x3FFF_FC00) << 10
    bi_dram[2].size  = (RCR & 0xFFF0_0000) + 1MB
    bi_dram[0].size -= bi_dram[2].size            (trừ đi phần bị remap; Sparrow-5GB thì bỏ qua trừ khi SparrowH)
```
⇒ 3 bank: **Bank0** (ch0, từ 0), **Bank1** (ch1, từ `0x2_0000_0000`), **Bank2** (cửa sổ remap). `DEBUG()` trong file bị định nghĩa lại thành `printf` (`soc.c:71-75 #if 1`) nên các dòng "DRAM CH%d CS%d : …" **luôn được printf** (nhưng đi qua lớp log-gating KM, P09 §9.5).

### (b) Kiểm tra dung lượng — `init_km` (`board_r.c:447`)
Chỉ khi HW-check mode: so `gd->chk_ram_Size` với `CONFIG_HWCHECK_MEMSIZE*`:
Eagle 8 GB (`8589934592`); Sparrow MFP/BK 5 GB hoặc 6 GB; AIO2 6 GB; SFP2 3 GB hoặc 4 GB. Lệch ⇒ `### BOOT-DIAG ERROR/Dimm ###`, báo panel, `_WAIT_FOREVER()`.

### (c) In thông tin — `print_soc_specific_info()` (`soc.c:299`)
`DDR %d Bit width = (1 << ((MC_CTRL_0[10:8]))) * 4` (`0xF002_0044`), và `LLC Enabled/Disabled`.

### (d) Lưu kết quả huấn luyện cho resume nhanh — `qspi_write_lpddr4_training_result()` (`qspi_winbond.c`, trong `bspi_initialize`)
`KM_RESUME_REFINE`: đọc vùng kết quả huấn luyện ở **`0x2_0000_0000`** (`TRAIN_RESULT_MEM_ADDR`, DRAM) và ghi vào QSPI **`0xF438_0000`** (`TRAIN_RESULT_QSPI_ADDR`, tối đa 4096 B) nếu khác cái đã lưu (cờ `THRD_SPI_OFFSET_LPDDR4_TRANING_NG 0x3d` để đánh dấu NG). Lần boot sau BLE dùng lại (warm/"SuperWarp"). ⇒ **U-Boot là bên *ghi* flash**, BLE là bên *đọc & dùng*.

### (e) Reserve vùng cho ATF/training (chỉ là hằng, để giữ đồng bộ)
`armada8k.h:106-115`: ATF `0x1000_0000` (4 MB), DDR training `0x1040_0000` (1 MB), SCPI `0x7FF0_0000` (1 MB). Comment nói rõ *"Keep this in sync with … ATF"*.

## 11.4 Số liệu DRAM đối chiếu
| Thứ | Nơi quyết định | Giá trị |
|---|---|---|
| `gd->ram_size` | hằng `CONFIG_QUARTZ_RAM_SIZE` | 2 GB (không phản ánh RAM thật) |
| RAM dùng để **relocate** | `get_effective_memsize()` | ≤ 1 GB |
| RAM **thật** để kernel/MMU | `bd->bi_dram[0..2]` (đọc MC) | 0…8 GB tùy máy |
| Nhiều CS/ch | MC | 2 ch × ≤4 CS |

## 11.5 Memory test
* Lệnh **`mtest`** (`CONFIG_CMD_MEMTEST=y`, thuật toán "alt" vì `CONFIG_SYS_ALT_MEMTEST`, `mvebu-common.h:172-175`): mặc định `START=0x0`, `END=0x1000_0000`, scratch `0x1080_0000`. `END` đúng bằng đáy ATF ⇒ không giẫm ATF.
* `quartz limit_mem`: thêm `mem=256M` vào `bootargs` để kernel chỉ dùng 256 MB (dùng khi test DDR) (`quartz.c: do_quartz`).
* Không có `testdram` khi khởi động (`CONFIG_SYS_DRAM_TEST` không bật) và không có POST.

---

# PHẦN 12 — STORAGE

## 12.1 Kho thiết bị lưu trữ thực sự dùng
| Loại | Bộ điều khiển / địa chỉ | Driver (được build?) | Interface trong U-Boot | Dùng để |
|---|---|---|---|---|
| **eMMC / SD** | Xenon SDHCI `0xE827_8000` (Quartz SB) | `drivers/mmc/xenon_mmc.c` ✔ | `mmc <dev>` (block) | rootfs/kernel/initrd/DTB (`mmc 0:2`, `0:5`, `0:7`…), **env** (INFERRED) |
| **QSPI NOR** (Winbond) | BSPI `0xE827_3000`, XIP `0xF400_0000` (CS0,7 MB), `0xF800_0000` (CS1, 6 MB) | `drivers/spi/qspi_winbond.c` ✔ (KM); MTD-lite | `bspi`, MTD `mrvl_bspi0/1` | tham số máy (MAC, machine info, DIP mềm), DTB+kernel dự phòng (`quartz bspi_boot`), kết quả LPDDR4, driver hibernate (`WARP_DRV_INFO … SPI 0x2b0000`) |
| **USB mass-storage** | xHCI `0xD00D_0000` (chip Iris) | `xhci*.c` + `xhci-mvebu.c` + `usb_storage.c` ✔ | `usb <dev>` | cập nhật FW từ USB (`FW00xx/`), boot USB (`CONFIG_USBCMD_*`) |
| **NVMe SSD** | PCIe MSB0125 (Iris, `0xD009_0000…`) | `drivers/block/nvme.c` + `common/cmd_nvme.c` ✔ (`KM_NVME`) | `nvme <dev>` | boot chính bằng SSD (`root=/dev/kmsda7`), Warp!! (SuperWarp SSD) |
| SATA/AHCI | — | ❌ (`MV_INCLUDE_SATA`, `SCSI_AHCI_PLAT` = n) | — | không |
| NAND / NOR-CFI | — | ❌ (`MVEBU_NAND`, `MV_INCLUDE_NOR` không bật; `CONFIG_SYS_NO_FLASH` định nghĩa) | — | không |
| SPI-NOR chuẩn (`sf`) | — | ❌ (`MVEBU_SPI=n`, `CMD_SF` không được `select`) | — | không (QSPI dùng driver riêng) |

Tên phân vùng KM (từ `machine_setup.c:fsl_table` và `armada8k.h`): `0:2` = `/dev/kmsda2` (SUB.img), `0:5` = `/dev/kmsda5` (Image, irfs.ubt, dtb), `0:7` = `/dev/kmsda7` (RFS1/RFS3.img), `0:A` = IISW update; SD boot: `root=/dev/mmcblk1p3`; USB boot: `root=/dev/sdc3`.

## 12.2 Lệnh → block layer → driver → controller → media
### (A) eMMC/SD: `mmc rescan` / `fatload mmc 0:5 $kernel_addr Image`
```
lệnh "mmc rescan"  (KM: MMC_INIT_CMD, armada8k.h:220)                      common/cmd_mmc.c  (CONFIG_CMD_MMC)
   └─ mmc_init(mmc)  → mmc_start_init → mmc_getcd  (SDHCI_QUIRK_NO_CD ⇒ luôn "có thẻ")
         ├─ mmc->cfg->ops->init = xenon_mmc_init()     (P06 §6.4)   power/clock/PHY
         ├─ mmc_go_idle (CMD0) → send_if_cond → sd/mmc_send_op_cond → mmc_startup (CMD2/3/9/7, EXT_CSD, đặt bus width/timing HS/DDR52, tuning nếu cần)
         │        mỗi lệnh: mmc_send_cmd → ops->send_cmd = xenon_mmc_send_cmd() → thanh ghi SDHCI
         └─ [KM] mmc_exist=1 ; hwcSetBootDeviceNormal(EMMC | SD)
   └─ init_part(&mmc->block_dev)  (disk/part.c, đọc MBR/DOS)      → block_dev_desc_t IF_TYPE_MMC

lệnh "fatload mmc 0:5 addr file"  →  (P13) …  →  disk_read → cur_dev->block_read = mmc_bread (mmc.c:1424)
   mmc_bread → (chia theo b_max) mmc_read_blocks (mmc.c:203): CMD17/CMD18 (READ_SINGLE/MULTIPLE_BLOCK), stop CMD12
        → mmc_send_cmd → xenon_mmc_send_cmd (xenon_mmc.c:519)
              ├─ chờ SDHCI_PRESENT_STATE không bận (CMD_INHIBIT/DATA_INHIBIT)
              ├─ [CONFIG_MMC_SDMA] ghi SDHCI_DMA_ADDRESS = địa chỉ đệm (32-bit; nếu địa chỉ dest không align ⇒ dùng bounce buffer 512 KB `aligned_buffer`, rồi memcpy ra)
              │      flush_cache(...) ; ghi SDHCI_BLOCK_SIZE/COUNT, TRANSFER_MODE(TRNS_DMA…), ARGUMENT, COMMAND(=SDHCI_MAKE_CMD)
              ├─ xenon_mmc_transfer_data: poll SDHCI_INT_STATUS (DATA_END / DMA_END(bump boundary) / lỗi), timeout 1 s
              └─ đọc phản hồi (SDHCI_RESPONSE) trả cho lớp trên
   ⇒ media: chip eMMC/SD  (DMA engine là SDMA của chính SDHCI; **UNVERIFIED** chi tiết PIO vs SDMA trong `xenon_mmc_transfer_data` vì có cả nhánh `xenon_mmc_transfer_pio` — cần đọc kỹ để khẳng định đường nào thực sự chạy cho từng lần đọc)
```
### (B) QSPI: đọc/ghi tham số & DTB+kernel
```
"quartz bspi_boot"  hoặc  quartz_late_init() (TOC)            board/mvebu/armada8k/quartz.c
   └─ do_raw_boot(BSPI_CS1=0xF8000000, HEADER_OFFSET=0x200000)
        header{magic 0xBAADC0DE, dtb_off/size, kernel_off/size}  ← đọc thẳng vùng XIP (con trỏ trỏ vào 0xF800_0000+0x200000)
        setenv fdt_addr=0x1000, kernel_addr=0x800000, bootcmd="booti $kernel_addr - $fdt_addr"
        memcpy(0x1000 ← DTB) ; memcpy(0x800000 ← kernel)         (memcpy từ cửa sổ XIP: controller tự sinh lệnh SPI đọc)
tham số: read_rom()/THRG_*  → mtd_flash_access.c → mtd_read(mrvl_bspiN) → __qspi_read → qspi_read → memcpy_fromio(base+off)
ghi/xóa: mtd_write/mtd_erase → __qspi_write/__qspi_erase → execute_cmd(): BSCR/BSCMDR (write_enable 0x06, page-program 0x02, sector-erase 0x20 (4 KB), read-id 0x9F, Quad-enable Winbond), poll `check_busy()` (timeout ~1 s)
lệnh shell "bspi": do_bspi (qspi_winbond.c:1394) để readid/read/write/erase/dump thủ công
```
### (C) USB mass storage
```
"usb reset" (USB_INIT_CMD) hoặc usb_init() từ setup_MotionEnable_Power     common/usb.c
   ├─ xhci_hcd_init() (P06 §6.6) → lõi xHCI: reset HC, dựng DCBAA/command ring/event ring, run
   ├─ quét root hub → enumeration (common/usb.c, common/usb_hub.c — KM sửa `UHOST_XHCI_CUSTOMIZE`) → usb_new_device
   └─ usb_stor_scan(1) → SCSI INQUIRY / READ_CAPACITY qua Bulk-Only Transport (common/usb_storage.c) → block_dev_desc_t IF_TYPE_USB
đọc: fatload usb 0:1 … → block_read = usb_stor_read → SCSI READ(10) trong CBW/CSW → xhci bulk transfer (xhci-ring.c: TRB → doorbell → event ring) → thiết bị USB
```
(Chi tiết ring/TRB là lõi xHCI upstream, **UNVERIFIED** tới từng dòng.)

### (D) NVMe
```
"nvme init" hoặc initr … → __nvme_initialize() (common/cmd_nvme.c:36)   [CONFIG_SYS_NVME_MAX_DEVICE=1]
   ├─ nvme_dev_desc[i]: if_type=IF_TYPE_NVME, blksz=512, block_read=nvme_read, block_write=nvme_write
   ├─ init_nvme(i) → tìm thiết bị qua pci_find_devices() theo bảng vendor/device (drivers/block/nvme.c:153…: Samsung 144d:a804, KIOXIA 1e0f:0001, Micron 1344:5189/5405, Phison 1987:5007/5008/5013, Intel 8086:0953/f1a5, SanDisk 15b7:5001, Transcend…)
   │       + bảng vendor MỞ RỘNG đọc từ SPI-flash (DEF_OFS_UBOOT_SSDVENDOR_TABLE, có checksum/version)   ← thêm SSD mới không cần đổi U-Boot
   ├─ scan_nvme(i): identify controller/namespace → lba, blksz
   ├─ init_part(&nvme_dev_desc[i]) ; hwcSetBootDeviceNormal(NVME)
   └─ (KM) is_exist_nvme()/check_nvme_is_exist(): phát hiện SSD có mặt hay không (cho chẩn đoán)
đọc/ghi: nvme_read/nvme_write → nvme_io(dev,start,cnt,buf,TYPE_READ|WRITE) → nvme_submit_cmd (ghi SQ entry + doorbell) → poll CQ → media (SSD qua PCIe Gen1-3)
```
Chi tiết hàng đợi/PRP/DMA **UNVERIFIED** (nvme.c 4085 dòng; chỉ xác minh tên hàm & luồng cấp cao). Có thêm tính năng bảo mật KM: `nvme_set_password/unlock/security_erase/opal` (OPAL/ATA-security qua `nvme_sec_submit`).

## 12.3 Môi trường (env) và lưu trữ
* `env_init()` luôn dùng `default_environment` (trong image).
* `initr_env()` → `env_relocate()` → `env_relocate_spec()` (`common/env_mmc.c`): **nếu** `ENV_IS_IN_MMC` (INFERRED, §0.4) thì đọc `CONFIG_ENV_SIZE=0x10000` byte tại `offset = CONFIG_ENV_OFFSET = 0x200000`, thiết bị `SYS_MMC_ENV_DEV=0`, sau khi `mmc_switch_part` sang **hw-partition `SYS_MMC_ENV_PART=1` (boot0)**; CRC sai ⇒ dùng mặc định.
* Bootcmd thực tế do KM sinh động (`make_bootcmds`, `check_autoboot` trong `common/main.c`) từ các macro `CONFIG_CMD_ENV/USBCMD_ENV…` (`armada8k.h:267-273`); mặc định Marvell `CONFIG_BOOTCOMMAND="ext2load mmc 0:1 … ; booti …"` (`armada8k.h:130`) ít khi là đường chạy chính của máy thật.

---

# PHẦN 13 — FILESYSTEM

## 13.1 Filesystem nào có trong firmware
| FS | Build? | Căn cứ |
|---|---|---|
| **FAT (FAT12/16/32, VFAT)** | ✔ | `CONFIG_CMD_FAT` (`mvebu-common.h:26`, `armada8k.h:287`) ⇒ `config_fallbacks.h:32` tự `#define CONFIG_FS_FAT` ⇒ `fs/fat/fat.o file.o`. Là FS **chính** (KM nạp bằng `fatload`, `armada8k.h:222`). |
| **ext2/ext3/ext4 (đọc)** | ✔ | `CONFIG_FS_EXT4=y`, `CONFIG_CMD_EXT2=y` (lệnh `ext2load`, `ext2ls`; `ext4load` nếu `CMD_EXT4`) ⇒ `fs/ext4/ext4fs.o ext4_common.o dev.o` |
| ext4 **ghi** | ❓ | `CONFIG_CMD_EXT4_WRITE` được define vô điều kiện (`mvebu-common.h:25`) ⇒ `fs.c` gắn `.write = ext4_write_file`; nhưng `ext4_write.o` chỉ build khi `CONFIG_EXT4_WRITE`, mà macro này chỉ define trong khối SATA/AHCI **không kích hoạt** (`mvebu-common.h:455-459`). **Mâu thuẫn — cần build để xác nhận.** |
| FAT **ghi** | ✘ | `CONFIG_FAT_WRITE` chỉ define trong khối `MVEBU_SATA_BOOT` (`:240`, không kích hoạt) ⇒ `.write = fs_write_unsupported` (`fs/fs.c:117-121`) |
| JFFS2, UBIFS, SquashFS, cramfs, reiserfs, zfs, yaffs2, CBFS | ✘ | `fs/Makefile`: đều `obj-$(CONFIG_CMD_*)`; `CMD_JFFS2` chỉ define trong khối SATA/AHCI không kích hoạt; không thấy bật `UBIFS`/`SQUASHFS` |
| ISO/EFI/DOS partition | DOS ✔ (`CONFIG_DOS_PARTITION`); ISO ✔ khi `CONFIG_USB` (`mvebu-common.h:…ISO_PARTITION`); EFI chỉ trong khối SATA (không kích hoạt) | `mvebu-common.h:28,…` |

## 13.2 Chuỗi gọi: `fatload mmc 0:5 $kernel_addr Image` (thật, từng tầng)
```
prompt / bootcmd
  └─ do_fat_fsload()                          common/cmd_fat.c:34  (lệnh "fatload")
        └─ do_load(cmdtp, flag, argc, argv, FS_TYPE_FAT)          fs/fs.c:350
              ├─ fs_set_blk_dev("mmc", "0:5", FS_TYPE_FAT)        fs/fs.c:185
              │     ├─ get_device_and_partition("mmc","0:5",&dev_desc,&part_info,1)       disk/part.c
              │     │        ├─ tách "0:5" → dev=0, part=5
              │     │        ├─ get_dev("mmc",0) → mmc_get_dev(0) → &mmc->block_dev        (IF_TYPE_MMC; mmc_create đã gán block_read=mmc_bread)
              │     │        └─ get_partition_info → bảng DOS (CONFIG_DOS_PARTITION): entry #5 (logic, EBR) → part_info{start,size,blksz}
              │     └─ vòng qua fstypes[]: info->probe = fat_set_blk_dev(dev_desc,&part)      (fs/fat/fat.c: fat_register_device(dev_desc, part_no) :90)
              │              ├─ cur_dev = dev_desc; cur_part_info = part
              │              └─ đọc boot sector (disk_read → block_read) kiểm signature FAT ⇒ nếu sai ⇒ thử FS khác (ext) hoặc lỗi "Unsupported filesystem"
              ├─ addr = simple_strtoul($kernel_addr) = 0x800000 ; filename = "Image"
              └─ fs_read(filename, addr, pos, bytes) → info->read = fat_read_file()            fs/fat/fat.c / file.c
                    └─ file_fat_read_at → do_fat_read_at → get_contents():
                          ├─ đọc FAT / thư mục: disk_read(sector, n, buf)                        fat.c:52
                          │      └─ cur_dev->block_read(cur_dev->dev, cur_part_info.start + block, nr, buf)
                          │              = mmc_bread(dev, start, cnt, dst)                        drivers/mmc/mmc.c
                          │                  └─ mmc_read_blocks → mmc_send_cmd (CMD17/18)
                          │                         └─ xenon_mmc_send_cmd → SDHCI regs + SDMA    drivers/mmc/xenon_mmc.c:519
                          │                                └─ eMMC/SD chip
                          └─ get_contents_vfatname_block…: đọc từng cluster, copy vào `addr` (0x800000)
              └─ printf("%llu bytes read in %lu ms (%s/s)") ; setenv("filesize", …)
```
Cùng cấu trúc cho `usb`/`nvme`: chỉ khác `cur_dev->block_read` (= `usb_stor_read` hoặc `nvme_read`) — **tầng FS hoàn toàn không biết** phía dưới là gì; đó là ý nghĩa của `block_dev_desc_t`.

## 13.3 `ext2load mmc 0:1 …` (đường mặc định Marvell)
```
do_ext2load (common/cmd_ext2.c:33) → do_load(..., FS_TYPE_EXT) → fs_set_blk_dev → ext4fs_probe (fs/ext4/dev.c: ext4fs_set_blk_dev + ext4fs_mount: đọc superblock, kiểm magic 0xEF53)
   → fs_read → ext4_read_file → ext4fs_open/ext4fs_read → ext4fs_read_file → ext4fs_devread() → dev_desc->block_read (=mmc_bread) → …
```
## 13.4 Cách KM dùng FS ở mức ứng dụng (ngoài `fatload`)
* `fsload_hash_check()` (`machine_setup.c:568`): với mỗi file trong `fsl_table[]` (SUB.img, Image, irfs.ubt, emu800.dtb, RFS1.img, RFS3.img) nạp vào `IMAGE_LOAD_ADDRESS=0x800000` rồi so **SHA-256** (`sha256_calculate`) với hash nằm trong file `INDEX` (nạp ở `0x700000`) — cơ chế "boot-diag" xác thực đĩa.
* Boot dùng `CMD_INITRD_ENV(INIT,DEV,PART,DIR)` = `INIT; fatload DEV PART $kernel_addr DIR$bootfile; fatload … $fdt_addr DIR$dtb_name; fatload … $initrd_addr DIR$initrd_name; booti $kernel_addr $initrd_addr $fdt_addr` (`armada8k.h:240-250`), ví dụ thực khi boot thường: `mmc rescan; fatload mmc 0:5 $kernel_addr Image; …`.

---

## Phụ lục — Danh mục kiểm chứng nhanh (chạy khi có toolchain)
```
cd BootROM/BootROM/Emu800/Src
./build_uboot.sh 800   (hoặc: make egl_defconfig)
grep -E "CONFIG_DM\b|CONFIG_SPL\b|MVEBU_.*_BOOT|ENV_IS|KM_BIZHUB|FS_(FAT|EXT4)|EXT4_WRITE|FAT_WRITE|CONFIG_CMD_(FAT|EXT2|EXT4|NVME|MEMTEST)" .config include/autoconf.mk
# xác nhận kích thước ảnh để tính relocaddr:  size/nm u-boot ;  grep -E "__bss_end|_start|__image_copy_end" u-boot.map
# xác nhận DTB nhúng:  ls dts/dt.dtb  &&  fdtdump dts/dt.dtb | head
```
