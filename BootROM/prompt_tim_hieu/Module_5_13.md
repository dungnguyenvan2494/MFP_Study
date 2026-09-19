# U-Boot (BootROM/Emu800/Src) — Phần 5→9: Board init · Driver Model · Device Tree · Memory · UART

> Nguồn: `BootROM/BootROM/Emu800/Src` (gọi tắt `Src/`). Mọi kết luận dưới đây được đọc từ source thật.
> Nhãn: **[UPSTREAM]** hành vi chuẩn U-Boot · **[VENDOR]** code Konica Minolta (KM) / Marvell tùy biến ·
> **[BOARD]** riêng board · **[DRIVER]** driver phần cứng · **[CONFIG]** bật/tắt bởi cấu hình.
> Mức chắc chắn: **FACT** = đã thấy trong source · **INFERRED** = suy ra từ luật Kconfig/Makefile (xem §0.4).

---

## 0. Danh tính của repository (đọc trước)

### 0.1 Đây là gì
| Mục | Giá trị | Bằng chứng |
|---|---|---|
| Nền | **U-Boot 2015.01** (`VERSION=2015 PATCHLEVEL=01`) | `Src/Makefile:1-2` |
| Bản Marvell | `-devel-16.07.2` | `Src/localversion` |
| Vendor | **Konica Minolta** (bizhub), cờ `CONFIG_KM_BIZHUB` → `-DKM_BIZHUB` | `Src/config.mk:67-68`, `Src/Kconfig` menu "KonicaMinolta customize" |
| Kiến trúc / SoC | ARMv8 (AArch64), **Marvell APN806/AP806 + "Quartz" south-bridge** (`SYS_CPU=armv8`, `SYS_SOC=armada8k`, `SYS_VENDOR=mvebu`, `SYS_BOARD=armada8k`) | `arch/arm/cpu/armv8/armada8k/Kconfig` |
| Board (config) | `CONFIG_QUARTZ=y`, `TARGET_ARMADA_8K`, `DEVEL_BOARD=y` | `configs/*_defconfig` |
| Vai trò trong chuỗi boot | **BL33** của ATF: `BootROM → BLE → BL1 → BL2 → BL31 → U-Boot → Linux` | `KERNEL_STUDY_CONTEXT.md` §2 (ATF `BL33=../uboot_s800/output/u-boot.bin`) |
| Compiler | `CROSS_COMPILE=s800-linux-` | `Src/build_uboot.sh` |

### 0.2 Bản build nào là "thật"?
`build_uboot.sh <opt>` chọn defconfig (`Src/build_uboot.sh`):

| opt | defconfig | Ghi chú |
|---|---|---|
| `800` | `km_mvebu_quartz_toc_defconfig` | "Quartz configuration" — **không** có `KM_MACHINE_*` |
| `egl`, `eglz`, `dnbmlk`, `spam`, `spaas`, `eglb`… | `egl_defconfig`, `spam_defconfig`… | **máy thật** (Eagle/Sparrow/Deneb/Helios/Minerva) |
| `emu*` | `emu800_*_defconfig` | thêm `CONFIG_KM_BOARD_EMU800` (bo mạch giả lập Emu800) |

`diff configs/egl_defconfig configs/spam_defconfig` → **chỉ khác** `CONFIG_KM_MACHINE_*` và `CONFIG_HWCHECK_MEMSIZE*`.
Nghĩa là mọi máy dùng **cùng một cây code**; khác nhau ở vài `#if` chọn máy (RAM size check, thư mục FW…).
`CONFIG_KM_BIZHUB` mặc định `y` (`Src/Kconfig`: `config KM_BIZHUB … default y`) nên **mọi** defconfig đều bật code KM.

> Trong tài liệu này "cấu hình tham chiếu" = `egl_defconfig` ≈ `km_mvebu_quartz_toc_defconfig` (giống nhau ngoài `KM_MACHINE_EGL`, `SPL`, `MPP_BUS`, `PINCTL`…).

### 0.3 Bảng "cái gì được build / KHÔNG được build" (quyết định mọi phần sau)
| Thành phần | Trạng thái | Căn cứ |
|---|---|---|
| **SPL** | ❌ tắt (`# CONFIG_SPL is not set` trong `egl_defconfig`, `km_mvebu_quartz_toc_defconfig`) | defconfig. (`km_mvebu_quartz_pd_defconfig` thì `CONFIG_SPL=y` — build khác) |
| **Driver Model (`CONFIG_DM`)** | ❌ **tắt** | `drivers/core/Kconfig: config DM bool default n`; không defconfig Quartz nào bật; `grep "select DM"` chỉ có `DM_GPIO` (không liên quan) |
| OF_CONTROL + OF_EMBED | ✅ | defconfig `+S:CONFIG_OF_CONTROL=y`, `+S:CONFIG_OF_EMBED=y`, `DEFAULT_DEVICE_TREE="armada-quartz-emu800"` |
| Generic board (`board_f.c`/`board_r.c`) | ✅ (`arch/arm/lib/board.c` **không** build) | `include/configs/mvebu-common.h:42` + `arch/arm/lib/Makefile:23` (`ifndef CONFIG_SYS_GENERIC_BOARD obj-y += board.o`) |
| `CONFIG_SYS_NS16550` (UART) | ✅ | defconfig |
| Xenon MMC, MRVL_QSPI (BSPI), MVQZ_GPIO, USB xHCI, DW_PCIE, PCIE_MV_MSB0125, KM_NVME, KM_I2S_CDMA | ✅ | defconfig |
| `MVEBU_PINCTL`, `MVEBU_MPP_BUS` | ❌ (`# CONFIG_… =y` bị comment) | defconfig |
| `MVEBU_I2C` | ❌ (`# CONFIG_MVEBU_I2C is not set`) — I2C dùng **DesignWare** qua header | `include/configs/quartz-sb.h` |
| `MVEBU_MBUS`, `MVEBU_COMPHY_SUPPORT`, `MVEBU_SPI`, NAND, NOR, SATA, PXA NAND | ❌ (default `n`, defconfig không bật) | `drivers/misc/Kconfig`, `drivers/phy/Kconfig`, `drivers/spi/Kconfig` |
| `MVEBU_CCU/RFU` | được compile nhưng **U-Boot chỉ có hàm `dump_*`**; không có nơi nào gọi init cửa sổ | `arch/arm/cpu/armv8/armada8k/mvebu_ccu.c:85 dump_ccu`; grep không thấy caller init |
| `MULTI_DT_FILE`, `BOARD_CONFIG_EEPROM` | ❌ | defconfig `+S:# CONFIG_MULTI_DT_FILE is not set` |

### 0.4 Giới hạn của phân tích (nói thẳng)
* Repo **không có `.config`**, không có toolchain trên máy này → **không build được**. Cấu hình hiệu lực được suy ra bằng tay từ `defconfig + Kconfig default + #define trong include/configs/*.h`.
* Điểm **INFERRED** quan trọng nhất: `CONFIG_MVEBU_MMC_BOOT=y` (⇒ env nằm ở eMMC). Lý do: `choice "Flash for image"` mặc định là `MVEBU_SPI_BOOT` nhưng nó `depends on MVEBU_SPI` (=n) nên không hiển thị; Kconfig chọn mục **hiển thị đầu tiên**, tức `MVEBU_MMC_BOOT` (`depends on MVEBU_MMC || XENON_MMC`, mà `XENON_MMC=y`). Nhánh `#else` của `mvebu-common.h:198-209` (định nghĩa `CONFIG_ENV_OFFSET`, `CONFIG_SYS_MONITOR_BASE`) cũng khớp với việc `TOTAL_MALLOC_LEN` tính được (`include/common.h:174-181`).
* **Cách xác nhận trên máy có toolchain:** `make km_mvebu_quartz_toc_defconfig && grep -E "MVEBU_.*_BOOT|ENV_IS|CONFIG_DM|CONFIG_SPL" .config include/autoconf.mk`.
* Không có `u-boot.map` ⇒ kích thước thật của image (`mon_len`) **không biết**; dưới đây dùng ký hiệu `mon_len`.

---

# PHẦN 5 — BOARD INITIALIZATION

## 5.1 Bức tranh tổng: ai chạy trước ai
```
ATF BL31 (EL3)  ── eret ──►  U-Boot @ 0x1000  (BL33, non-secure, thường EL2)
                              │  (CONFIG_SYS_TEXT_BASE = 0x1000, mvebu-common.h:55)
                              ▼
 arch/arm/cpu/armv8/start.S : _start → reset          (chưa có C runtime, chưa có stack)
                              │  set VBAR theo EL, apply_core_errata, flush cache/TLB,
                              │  lowlevel_init, chọn master/slave
                              ▼  (master)
 arch/arm/lib/crt0_64.S     : _main                    (dựng SP + gd, gọi C)
                              │
                              ▼
 common/board_f.c           : board_init_f()  →  initcall_run_list(init_sequence_f)   [chạy trong DRAM ở 0x1000, chưa relocate]
                              │   …dram_init, reserve_*, setup_reloc…
                              ▼
 crt0_64.S                  : relocate_code() → nhảy về relocation_return (đã ở địa chỉ mới)
                              │   c_runtime_cpu_setup, xóa BSS
                              ▼
 common/board_r.c           : board_init_r()  →  initcall_run_list(init_sequence_r)   [chạy từ RAM đã relocate]
                              │   …initr_caches, board_init, initr_mmc, initr_bspi, init_km, initr_env,
                              │     stdio, console_init_r, misc_init_r, board_late_init, last_stage_init…
                              ▼
                            run_main_loop() → main_loop()   (KM tùy biến rất nhiều, common/main.c 2788 dòng)
```
Khác biệt lớn so với U-Boot điển hình: **DDR đã được ATF/BLE khởi tạo từ trước**, nên `board_init_f` của U-Boot **không** phải khởi tạo DRAM — xem Phần 11.

## 5.2 Giai đoạn 0 — `start.S` (assembly, không stack)
**Caller:** ATF BL31 nhảy tới BL33 entry (`0x1000`). **Callee:** `apply_core_errata`, `__asm_flush_dcache_all`, `lowlevel_init`, `_main`.
```
_start: b reset                                   (start.S:22-42, nhánh #else vì không phải SPL)
reset  (start.S:65)
  ├─ switch_el → đặt VBAR_ELn = vectors (EL3: còn set SCR_EL3, CPTR_EL3, CNTFRQ)
  ├─ apply_core_errata          [UPSTREAM, weak]  (chỉ A57 errata; core thật là A72 ⇒ gần như no-op)
  ├─ __asm_flush_dcache_all / __asm_invalidate_icache_all / __asm_invalidate_tlb_all   (trừ CONFIG_PALLADIUM)
  ├─ lowlevel_init              [UPSTREAM weak, start.S:170]
  │     └─ if CurrentEL != EL3  → return ngay          ← ATF đã lo GIC/EL, nên KHÔNG làm gì (start.S:173-176)
  └─ branch_if_master
        ├─ master → _main
        └─ slave  → wfe loop, đọc [CPU_RELEASE_ADDR = 0x0200_0000] ≠0 thì `br` tới đó  (spin-table, start.S:113-118)
```
* **Hardware bị tác động:** thanh ghi hệ thống (VBAR/SCTLR…), cache/TLB toàn cục.
* **State:** MMU **tắt**, I/D-cache tắt (comment `start.S:66-68`). Không stack ⇒ không gọi C.
* **Slave cores:** không đi qua U-Boot init; chờ spin-table ở `0x0200_0000`. `mvebu_soc_init()` ghi `0` vào địa chỉ đó (`arch/arm/cpu/mvebu-common/soc-init.c`, khối `#ifdef CPU_RELEASE_ADDR`). Kernel dùng PSCI (ATF) để bật core khác.

## 5.3 Giai đoạn 1 — `_main` (crt0_64.S:59-137)
```
x0 = CONFIG_SYS_INIT_SP_ADDR = 0x1000 + 0xFF0000 = 0x00FF_1000      (mvebu-common.h:71)
x18 (= gd) = x0 - GD_SIZE, căn 8 byte                                (crt0_64.S:64-72)
zero_gd: xóa vùng gd
gd->malloc_base = x18 - CONFIG_SYS_MALLOC_F_LEN (0x5000)             (crt0_64.S:78-81)   [pre-reloc heap]
sp = align16(malloc_base)
bl board_init_f(0)
```
**Điểm KM đáng chú ý:** `gd_t` của KM to bất thường vì `gd->arch` chứa
`char printBuffer[PRINT_BUFFER_NUM=255][CONFIG_SYS_PBSIZE]` (`arch/arm/include/asm/global_data.h:71-78`).
`CONFIG_SYS_PBSIZE = CBSIZE(1024) + sizeof("Marvell>> ")(11) + 16 = 1051` ⇒ **≈ 255 × 1051 ≈ 262 KB** trong `gd`. Đây là bộ đệm log trước khi UART được phép in (xem Phần 9). Vì vậy dòng `ldr x1,=GD_SIZE; sub` được KM sửa (`crt0_64.S:65-70`, comment: giá trị lớn không dùng làm immediate được).

## 5.4 Giai đoạn 2 — `board_init_f()` và `init_sequence_f[]` (`common/board_f.c:828`)
`board_init_f()` (khoảng `board_f.c:1010+`) đặt `gd->flags`, `have_console=0`, **[VENDOR]** khởi tạo `gd->arch.termFlg=0, printStopFlg=1, printBufferNum=0`, rồi `initcall_run_list(init_sequence_f)`; hàm nào trả ≠0 ⇒ `hang()`.

Danh sách **thực sự chạy** với cấu hình này (đã lọc `#if`), theo thứ tự:

| # | Hàm | Loại | Việc làm thật | Ghi chú |
|---|---|---|---|---|
| 1 | `setup_mon_len` | UPSTREAM | `gd->mon_len = __bss_end - _start` | dùng cho vị trí relocate |
| 2 | `setup_fdt` | UPSTREAM/CONFIG | `gd->fdt_blob = __dtb_dt_begin` (OF_EMBED); cho phép env `fdtcontroladdr` ghi đè | env lúc này chỉ là default |
| 3 | `initf_malloc` | UPSTREAM | `malloc_limit = base + 0x5000; malloc_ptr = 0` | heap trước relocate |
| 4 | `arch_cpu_init` | UPSTREAM weak | trả 0 (không ai override cho armada8k) | `board_f.c:296` |
| 5 | `mark_bootstage`, `fdtdec_check_fdt` | UPSTREAM | kiểm DTB hợp lệ | |
| 6 | `initf_dm` | UPSTREAM | **no-op** (`CONFIG_DM` tắt) | `board_f.c:817` |
| 7 | **`board_early_init_f`** | **[BOARD]** | xem §5.5 | `board/mvebu/common/init.c` |
| 8 | `timer_init` | DRIVER | bật Generic Timer nếu chưa: ghi `GTC_CNTCR |= 1` tại `0xF058_1000` | `arch/arm/cpu/mvebu-common/generic_timer.c:26` |
| 9 | `env_init` | UPSTREAM | `gd->env_addr = default_environment` (env mặc định trong image) | `common/env_mmc.c` (nếu ENV_IS_IN_MMC) |
| 10 | **`init_baud_rate`** | **[VENDOR]** | đọc GPIO `FW_CONFIG4/5` (270/271) → chọn UART console (`gd->arch.term_sel`) → `gd->baudrate = env baudrate ∥ 115200` | `board_f.c:143-158` |
| 11 | `serial_init` | UPSTREAM | `get_current()->start()` = `eserialN_init` → `NS16550_init` | Phần 9 |
| 12 | `console_init_f` | UPSTREAM | `gd->have_console=1`, in nốt pre-console buffer | `common/console.c:831` |
| 13 | `fdtdec_prepare_fdt`, `display_options`, `display_text_info` | UPSTREAM | banner "U-Boot 2015.01…" | (thực tế bị KM chặn/đệm log) |
| 14 | `print_cpuinfo` | **[VENDOR]** | **no-op** cố ý (`misc.c:83`): Marvell in board name sau, trong `board_init` | |
| 15 | `init_func_i2c` | UPSTREAM | `i2c_init_all()` (DW I2C ×4) in "I2C: ready" | cần `CONFIG_SYS_I2C` (quartz-sb.h) |
| 16 | `announce_dram_init` | UPSTREAM | `puts("DRAM:  ")` | |
| 17 | **`dram_init`** | **[BOARD/SoC]** | `gd->ram_size = CONFIG_QUARTZ_RAM_SIZE = 0x8000_0000` (hằng!) | `arch/arm/cpu/armv8/armada8k/soc.c:138-162` |
| 18 | `setup_dest_addr` | UPSTREAM | `ram_top = SDRAM_BASE + get_effective_memsize()` = **0x4000_0000** | `get_effective_memsize()` = min(ram_size, 1 GB) (`soc.c:164`) |
| 19 | `reserve_round_4k`, **`reserve_mmu`**, **`reserve_lcd`**, `reserve_trace`, **`reserve_uboot`**, `reserve_malloc`, `reserve_board`, `setup_machine`, `reserve_global_data`, `reserve_fdt`, `reserve_stacks` | UPSTREAM (+VENDOR ở lcd) | trừ dần `relocaddr`/`start_addr_sp` từ trên xuống — bảng ở Phần 8 | `reserve_lcd`: **[VENDOR]** VRAM = 1280×768×2 B (`common/lcd.c:585-587`) |
| 20 | **`setup_dram_config`** → **`dram_init_banksize`** | **[BOARD]** | đọc thanh ghi Memory Controller để điền `bd->bi_dram[0..2]` | `soc.c:173-283` |
| 21 | `show_dram_config` | UPSTREAM | in "DRAM Total: …" | |
| 22 | `display_new_sp`, `reloc_fdt`, **`setup_reloc`** | UPSTREAM | copy DTB sang vùng mới; `reloc_off = relocaddr - 0x1000`; `memcpy(new_gd, gd)` | |
| — | (`init_func_ram`, `testdram`, `post_*`, watchdog…) | | **không có** trong cấu hình này | `CONFIG_SYS_DRAM_TEST` không bật |

> ⚠ Bẫy đọc code: thư mục có cả `arch/arm/lib/board.c` (kiểu init cũ) — **không được build** vì `CONFIG_SYS_GENERIC_BOARD` (`arch/arm/lib/Makefile:23`). Đừng đọc file đó để hiểu flow.

### 5.5 `board_early_init_f()` (`board/mvebu/common/init.c`, bật bởi `CONFIG_BOARD_EARLY_INIT_F`, `mvebu-common.h:86`)
```
board_early_init_f
  ├─ [CONFIG_BOARD_CONFIG_EEPROM]  ✗ tắt  (setup_fdt/cfg_eeprom_init không chạy)
  ├─ [CONFIG_MULTI_DT_FILE]        ✗ tắt
  ├─ soc_early_init_f()            [BOARD/SoC]  arch/arm/cpu/armv8/armada8k/soc.c:78
  │     ├─ [MVEBU_CHIP_SAR ✔ default y] mvebu_sar_init(gd->fdt_blob)
  │     │        └─ fdtdec_find_aliases_for_id("sar-reg", COMPAT_MVEBU_SAR_REG_COMMON)  → node DT `sar-reg` @0x6F8200
  │     └─ [MVEBU_PINCTL ✗ tắt] mvebu_pinctl_probe() bị loại
  ├─ [MVEBU_SYS_INFO ✔ default y] sys_info_init()   ← chép bảng thông tin BLE→U-Boot từ 0x0400_0000 vào gd->arch.local_sys_info[]
  │        (arch/arm/cpu/mvebu-common/system_info.c; SYSTEM_INFO_ADDRESS=0x4000000)
  └─ [MVEBU_MBUS ✗ tắt] init_mbus() bị loại
```
* **Phụ thuộc:** `sar_init` cần **DT đã có** (`setup_fdt` chạy trước, #2) và **malloc_f** (#3).
* `sys_info` là **kênh giao tiếp từ ATF/BLE sang U-Boot** (BLE ghi, U-Boot đọc). Trong build này `get_info()` chỉ còn dùng ở nhánh `dram_init` không-Quartz.

## 5.6 Giai đoạn 3 — relocate rồi `board_init_r()` (`common/board_r.c:915 init_sequence_r`)
Thứ tự **thực sự chạy** (đã lọc theo cấu hình):

| # | Hàm | Loại | Việc làm |
|---|---|---|---|
| 1 | `initr_reloc` | UPSTREAM | `gd->flags |= GD_FLG_RELOC | GD_FLG_FULL_MALLOC_INIT` |
| 2 | `initr_caches` → `enable_caches()` → `dcache_enable()` → `mmu_setup()` | UPSTREAM/ARMv8 | dựng bảng trang identity, bật MMU + D-cache (Phần 8) |
| 3 | `initr_reloc_global_data` | UPSTREAM | `monitor_flash_len = _end - __image_copy_start` |
| 4 | `initr_barrier`, `initr_malloc` | UPSTREAM | heap thật = `relocaddr − TOTAL_MALLOC_LEN` (dưới ảnh U-Boot) |
| 5 | `bootstage_relocate` | UPSTREAM | |
| 6 | `initr_dm` | — | **không tồn tại** (`CONFIG_DM` tắt) |
| 7 | **`board_init`** | **[BOARD+VENDOR]** | xem §5.7 |
| 8 | `stdio_init_tables`, `initr_serial` | UPSTREAM | `serial_initialize()` đăng ký các `struct serial_device` (Phần 9) |
| 9 | `initr_announce` | UPSTREAM | in "Now running in RAM - U-Boot at: %08lx" |
| 10 | `power_init_board` | weak | no-op |
| 11 | **`initr_mmc`** | UPSTREAM+DRIVER | `mmc_initialize()` → `board_mmc_init()` (Xenon, Phần 6/12) |
| 12 | **`initr_bspi`** | **[VENDOR]** | `bspi_initialize()` (QSPI flash, `CONFIG_MRVL_QSPI`) — thêm **trong** `board_r.c:583,1010-1013` |
| 13 | **`init_km`** | **[VENDOR]** | `board_r.c:447`: `init_Panel_Uart()`, HW-check mode, so RAM, GPIO quạt (§5.8) |
| 14 | `initr_env` | UPSTREAM | `env_relocate()` → `env_relocate_spec()` (đọc env từ eMMC nếu `ENV_IS_IN_MMC`, INFERRED) rồi `load_addr` |
| 15 | `initr_secondary_cpu` | UPSTREAM | |
| 16 | `initr_pci` | UPSTREAM | `pci_init()` → gọi **`pci_init_board()`** của KM (`drivers/pci/pcie-mv-msb0125.c`) |
| 17 | `stdio_add_devices` | UPSTREAM+**[VENDOR]** | đặt `gd->arch.termFlg=1` (cho phép in log); `i2c_init_all()`; đăng ký LCD/serial |
| 18 | `initr_jumptable`, `console_init_r` | UPSTREAM | gán `stdin/stdout/stderr` = `serial` |
| 19 | **`misc_init_r`** | **[VENDOR]** | `board/mvebu/armada8k/km/machine_setup.c:674` (§5.9) |
| 20 | `interrupt_init`, `initr_enable_interrupts` | UPSTREAM | (ARM64: stub) |
| 21 | `initr_ethaddr` | UPSTREAM | chỉ nếu `CONFIG_CMD_NET` |
| 22 | **`board_late_init`** | **[BOARD]** | `quartz_late_init()` → xác định loại board (§5.10) |
| 23 | **`last_stage_init`** | UPSTREAM/BOARD | `soc.c:380`: chỉ làm gì khi `MULTI_DT_FILE` (tắt) ⇒ thực tế no-op |
| 24 | `run_main_loop` | UPSTREAM | `for(;;) main_loop();` — **KM tự viết** (`common/main.c`, có `check_action`, `check_autoboot`, `make_bootcmds`…) |

### 5.7 `board_init()` (`board/mvebu/common/init.c`) — **[BOARD+VENDOR]**
```
board_init
  ├─ [CONFIG_KM_I2S_CDMA ✔]  sound_init(); sound_play()      ← phát âm thanh khi bật nguồn panel (KM sửa 2018/05/18: chuyển từ misc_init_r sang đây)
  ├─ mvebu_print_info()      → printf("Board: %s", DT "model") ; đọc 0xE8000040 (SoC rev) & 0xE830A000 (board rev)
  │                             → mvebu_print_soc_info() → soc_print_clock_info + print_soc_specific_info (DDR width từ MC_CTRL_0, LLC)
  ├─ mvebu_soc_init()        arch/arm/cpu/mvebu-common/soc-init.c
  │      ├─ soc_init()       (comphy_init chỉ khi MVEBU_COMPHY_SUPPORT — tắt)
  │      ├─ mvebu_sar_init() (lần 2)
  │      ├─ mvebu_thermal_sensor_probe() (weak → 0)
  │      ├─ soc_late_init()  (weak → 0)
  │      └─ *(CPU_RELEASE_ADDR) = 0        ← xóa spin-table
  ├─ mvebu_board_init()
  │      ├─ [MVEBU_MPP_BUS ✗]
  │      └─ [DEVEL_BOARD ✔]  mvebu_devel_board_init()   board/mvebu/armada8k/devel-board.c:167
  │             ├─ sar_init()                       (MVEBU_SAR ✔ default y)
  │             ├─ board_usb_current_limit_init()   ┐ **no-op trên Quartz**: chúng kiểm
  │             └─ board_usb_vbus_early_init()      ┘ DT root compatible == "marvell,armada-70x0-db", còn DT của ta là "marvell,apn-806-pd" ⇒ return sớm
  └─ mvebu_io_init()         (rỗng)
```
Gotcha: chuỗi model in ra là **"Marvell Quartz Palladium"** (DT `armada-quartz-emu800.dts`, `model=`), dù máy là thiết bị thật — đó chỉ là chuỗi trong DT.

### 5.8 `init_km()` — **[VENDOR]** `common/board_r.c:447`
```
init_km
  ├─ init_Panel_Uart()                      (km/mfp_panel.c)  cấu hình UART panel COM2 (0xE800_2800) 57600 8E1
  ├─ is_execHwCheckMode()==1 ?  → uc_HWcheckModeBoot=1, uc_AutoHWcheckMode=1, THRF_initAutoHwDiagnosCSRAData()
  │   elif !is_RestartOccured() && checkPowerSaveKey() → uc_HWcheckModeBoot=1        (giữ phím nguồn tiết kiệm = chế độ chẩn đoán)
  ├─ if HWcheckMode:
  │      ├─ uart_error ≠0  → in "### BOOT-DIAG ERROR/Panel ###", _WAIT_FOREVER() (vòng lặp + refresh watchdog)
  │      ├─ set_Panel_BootDiagStatus(ST1)
  │      └─ so `gd->chk_ram_Size` (tổng bank, tính ở dram_init_banksize) với CONFIG_HWCHECK_MEMSIZE* theo máy
  │           (Eagle 8 GB; Sparrow MFP 5/6 GB; AIO2 6 GB; SFP2 3/4 GB…) ; sai ⇒ "BOOT-DIAG ERROR/Dimm", báo panel, treo
  └─ GPIO: GPIO_PORT_BOXFAN_HALF_REM(116)=0, GPIO_PORT_BOX_FAN_REM(117)=1 (output)   → điều khiển quạt hộp
```
→ Đây là chỗ mà **kiểm tra DDR của U-Boot** nằm (không phải huấn luyện DDR).

### 5.9 `misc_init_r()` — **[VENDOR]** `km/machine_setup.c:674`
(Bản Marvell trong `init.c` bị loại bởi `#if !defined(CONFIG_KM_BIZHUB)`.)
```
misc_init_r
  ├─ misc_init_r_env()          đọc "ROM" tham số từ SPI-flash (read_rom → BSPI, mtd_flash_access.c):
  │        ethaddr (chỉ nhận OUI KM 00:20:6B/00:50:AA/08:00:86), machineInfo, machineNo, serial → gMachineInfo/gMachineNo/gSerialNo, setenv("ethaddr")
  ├─ THRF_getAutoHwDebugFlag()  cờ debug chẩn đoán trong SPI-flash
  ├─ init_Panel()               khởi tạo LCD/VRAM/panel
  ├─ setup_MotionEnable_Power() đọc cờ "line-card boot" (SPI), nếu UtilityKey=0 hoặc cờ=1 → usb_init(); usb_stor_scan(1) (retry 2 lần, 10 s) — để nạp FW từ USB
  ├─ hwcDispBootDeviceDiag()    nếu không có boot device → LED nhấp nháy, dừng
  ├─ boot_diag()                nếu HW-check tự động: fsload_hash_check() (SHA-256 các file SUB.img/Image/irfs.ubt/emu800.dtb/RFS1/RFS3 từ mmc 0:2/0:5/0:7) — sai ⇒ "Hash Error", treo
  └─ nhánh debug-flag: có thể ép g_action = BOOT_HW_CHECKMODE, vẽ logo (DrawPanel_lzo)
```
### 5.10 `board_late_init()` → `quartz_late_init()` — **[BOARD]** `board/mvebu/armada8k/quartz.c`
```
board_late_init (init.c, CONFIG_BOARD_LATE_INIT ✔ mvebu-common.h:88)
  └─ quartz_late_init
        └─ get_quartz_board()      (cache kết quả)
              ├─ i2c_set_bus_num(3); i2c_probe(0x50) == 0  → BOARD_TYPE_TOC      ("có EEPROM = board khách hàng")
              └─ else đọc GPIO 271 (FW_CONFIG5): 1 → ESEVAL ; 0 → EMU800
        ├─ TOC    : setenv("dtb_name","armada-quartz-toc.dtb");  bspi_boot()    ← copy DTB+kernel từ QSPI CS1 0xF8000000 nếu header==0xbaadc0de
        ├─ ESEVAL : setenv("dtb_name","eseval.dtb")
        └─ EMU800 : setenv("dtb_name","emu800.dtb")
```
Lệnh `quartz` (`bspi_boot|mmc_raw_boot|mmc_fs|limit_mem`) cũng ở file này.

## 5.11 Sơ đồ phụ thuộc giữa các init
```
 setup_fdt ──► (DT) ──► soc_early_init_f/mvebu_sar_init ──► soc_tclk_get()  ──► ns16550_calc_divisor ──► UART baud
     │                                                              (CONFIG_MSS_FREQUENCY)
     └────────► init_baud_rate (GPIO 270/271) ──► term_sel ──► default_serial_console() ──► serial_init
 sys_info_init(0x4000000, do BLE ghi) ──(chỉ nhánh không-Quartz)──► dram_init
 dram_init(ram_size cố định) ─► setup_dest_addr ─► reserve_* ─► setup_reloc ─► relocate_code
 dram_init_banksize (đọc MC regs) ─► bd->bi_dram[] ─► mmu_setup (initr_caches) ─► toàn bộ DRAM cacheable
 gd->chk_ram_Size (từ banksize) ─► init_km (HW check RAM)
 initr_mmc ─► (eMMC/SD sẵn sàng) ─► initr_env (env từ eMMC) ─► misc_init_r (fsload_hash_check, USB) ─► board_late_init
 initr_bspi ─► read_rom()/THRG_getSoftDipSw() ─► getTermState() ─► (có được in log không)  ─► misc_init_r_env (MAC/serial)
 stdio_add_devices (termFlg=1) ─► log bắt đầu có thể in ─► console_init_r
```

---

# PHẦN 6 — DRIVER MODEL

## 6.1 Sự thật quan trọng nhất
**Driver Model của U-Boot KHÔNG được biên dịch trong firmware này.** (§0.3: `CONFIG_DM` mặc định `n`, không defconfig Quartz nào bật.)
Hệ quả có thể kiểm chứng ngay trong code đã đọc:
* `initf_dm`/`initr_dm` biến mất khỏi `init_sequence_f/r` (`board_f.c:817`, `board_r.c:933` đều `#ifdef CONFIG_DM`).
* `drivers/core/Makefile: obj-$(CONFIG_DM) += device.o lists.o root.o uclass.o util.o` ⇒ **không link**.
* `drivers/serial/Makefile`: `ifdef CONFIG_DM_SERIAL … else obj-y += serial.o` ⇒ build dùng nhánh **legacy** `serial.o`.
* `ns16550.c` bọc phần `udevice` bằng `#ifdef CONFIG_DM_SERIAL` — không biên dịch.
* Không có `struct udevice`, `struct uclass`, `U_BOOT_DRIVER`, `dev_get_priv` nào tham gia flow runtime.

Vì vậy phần "Device Tree → bind → uclass → udevice → probe()" **không tồn tại thực tế** ở đây. Tôi **không** dựng lại nó cho đẹp; thay vào đó dưới đây là (a) khái niệm DM (để bạn hiểu code upstream còn nằm trong cây) và (b) flow **thật** mà repo này dùng.

## 6.2 Khái niệm DM (upstream) — và "tương đương thật" trong repo này
| Khái niệm DM [UPSTREAM] | Nghĩa | Trong firmware này thay bằng |
|---|---|---|
| `struct driver` + `U_BOOT_DRIVER()` | mô tả driver: tên, `of_match`, `bind/probe/remove`, `priv_auto_alloc_size` | **không có**. Driver là hàm C thường (`board_mmc_init`, `bspi_initialize`, `pci_init_board`, `xhci_hcd_init`…) gọi tường minh từ init sequence |
| `struct udevice` | 1 thể hiện thiết bị, có `of_offset`, `priv`, `platdata` | biến tĩnh/`calloc` riêng từng driver (vd. `struct xenon_mmc_cfg`, `struct qspi_s devices[10]`, `struct msb0125_pcie msb0125_pcie[]`) |
| `UCLASS_DRIVER()` (serial, gpio, mmc…) | API chung cho 1 loại thiết bị | API "legacy" chung: `struct serial_device` (`serial.c`), `struct mmc`+`struct mmc_ops`, `block_dev_desc_t`, `mtd_info`, `struct stdio_dev` |
| bind (`dm_scan_fdt`) | quét DT, tạo udevice theo `compatible` | mỗi driver **tự** gọi `fdtdec_find_aliases_for_id(blob, "<alias>", COMPAT_xxx, list, max)` |
| probe | cấp phát + init phần cứng khi cần | hàm init của driver, gọi **eager** trong `init_sequence_r` |
| `ofnode`/`dev_read_*` (mới) | API đọc DT | ở 2015.01 chưa có; dùng `fdtdec_get_int/addr/bool`, `fdt_get_regs_offs` |
| sequence (`dev->seq`) | thứ tự thiết bị cùng loại (`mmc0`, `serial0`) | thứ tự phát hiện trong `node_list[]` hoặc chỉ số alias (`xenon-sdhci0`, `usb3N`) |

Code DM **vẫn có trong cây** (`drivers/core/*.c`, `include/dm/*.h`, nhánh `CONFIG_DM_SERIAL` trong `ns16550.c:65-115,287-375`) nhưng là code "chết" với build này. (Chỉ `mvebu_armada3700/70x0…_defconfig` — không phải Quartz — mới bật DM.)

## 6.3 Mẫu "driver" thật của repo (5 bước, thay cho DM)
```
Kconfig (CONFIG_XENON_MMC=y, drivers/mmc/Kconfig)
  └─ Makefile (obj-$(CONFIG_XENON_MMC) += xenon_mmc.o)            ← quyết định file có link không
       └─ include/configs/*.h  (#define CONFIG_MMC, CONFIG_GENERIC_MMC, CONFIG_MMC_SDMA)   ← bật API lõi
            └─ init_sequence_r[] → initr_mmc() gọi mmc_initialize()  ← "bind+probe" cứng
                 └─ mmc_initialize() → board_mmc_init(bis)           ← driver quét DT bằng compatible
                      └─ fdtdec_find_aliases_for_id(… "xenon-sdhci", COMPAT_MVEBU_XENON_MMC …)
                           └─ xenon_mmc_create() → mmc_create(&cfg, priv) → mmc_register (danh sách mmc_devices)
```
Ba "hợp đồng" quyết định driver đứng vững: **(1)** `CONFIG_*` bật file, **(2)** hàm được gọi ở init sequence, **(3)** node DT khớp `compatible` **và** `status="okay"` (`fdtdec_get_is_enabled`: `status` phải bằng đúng `"okay"`; các node ghi `"disable"`/`"disabled"` bị bỏ qua).

## 6.4 Trace driver thật #1 — Xenon SDHCI (eMMC/SD)
```
DT  arch/arm/dts/quartz-sb.dtsi:
    quartz-sb { mmc0: mmc@e8278000 { compatible="marvell,xenon-sdhci"; reg=<0xe8278000 0x90000>;
                                     xenon,emmc; bus-width=<4>; status="okay"; } }
 │ (lib/fdtdec.c:126  COMPAT(MVEBU_XENON_MMC,"marvell,xenon-sdhci"))
 ▼
initr_mmc  (board_r.c)  → mmc_initialize()  (drivers/mmc/mmc.c:1666)
 ▼
board_mmc_init(bis)  drivers/mmc/xenon_mmc.c:1157        [DRIVER]
   ├─ fdtdec_find_aliases_for_id(blob,"xenon-sdhci",COMPAT_MVEBU_XENON_MMC,…)   ← /aliases rỗng ⇒ tìm theo compatible (lib/fdtdec.c:292…, phần "Add any nodes not mentioned by an alias")
   ├─ mmc_soc_init()  (strong ở soc.c:317): xóa bit SDPHY_EN ở 0xF06F_4100 ⇒ PHY eMMC/SD lấy chân thay MPP
   ├─ fdtdec_get_is_enabled → bỏ node không "okay"
   ├─ fdt_get_regs_offs(blob,node,"reg")   ← đi ngược cây DT cộng `ranges` → 0xE827_8000 (node cha `quartz-sb` không có `ranges` ⇒ giữ nguyên)
   ├─ "xenon,emmc" ⇒ XENON_MMC_MODE_EMMC   … nhưng [BOARD] nếu get_quartz_board()==ESEVAL/EMU800 ⇒ ép SD_SDIO
   ├─ bus-width (DT) ⇒ dt_mmc_host_cap (4/8-bit)
   └─ xenon_mmc_create(port, reg_base, XENON_MMC_MAX_CLK, mode, cap, gpio)      xenon_mmc.c:1031
         ├─ calloc(struct xenon_mmc_cfg); quirks = NO_CD | WAIT_SEND_CMD | 32BIT_DMA_ADDR
         ├─ đọc SDHCI_HOST_VERSION, SDHCI_CAPABILITIES → f_max/f_min/voltages/host_caps (HS, 52MHz, DDR52, 4/8bit)
         ├─ xenon_mmc_reset(SDHCI_RESET_ALL)                                     [ghi register SDHCI]
         └─ mmc_create(&cfg, priv) → gán block_dev.block_read = mmc_bread (mmc.c:1424)
 ▼
(sau đó) mmc_init(mmc)/mmc_start_init(): mmc->cfg->ops->init = xenon_mmc_init (xenon_mmc.c:959)
   → set FIFO, cấp bounce buffer 512 KB, tắt ACG, enable slot, set power (eMMC 1.8V / SD 3.3V), set clock, init PHY (xenon_mmc_phy_set), bật parallel-transfer
 ▼
Hoạt động:  mmc_bread → mmc_read_blocks (mmc.c:203) → mmc_send_cmd → ops->send_cmd = xenon_mmc_send_cmd (xenon_mmc.c:519)
   → ghi SDHCI_DMA_ADDRESS / ARGUMENT / TRANSFER_MODE / COMMAND (SDMA 32-bit; bounce buffer khi địa chỉ không align)
   → đợi SDHCI_INT_STATUS (CMD_COMPLETE / DATA_END) → trả dữ liệu
```
Điểm KM thêm vào lõi `mmc.c`: cờ `mmc_exist` (phân biệt eMMC vs SD cho Sparrow), `hwcSetBootDeviceNormal(HWC_BootDevice_EMMC/SD)`, và `mmc_read()/mmc_write()` bọc `mmc_init` + `mmc_bread/bwrite` (dùng cho Warp/hibernate).

## 6.5 Trace driver thật #2 — BSPI (QSPI NOR, bản KM `qspi_winbond.c`)
**Chọn implementation** (Rule 8): `drivers/spi/Makefile:42-46`
```
ifdef CONFIG_KM_BIZHUB
obj-$(CONFIG_MRVL_QSPI) += qspi_winbond.o      ← ĐƯỢC BUILD (KM_BIZHUB=y)
else
obj-$(CONFIG_MRVL_QSPI) += qspi.o              ← bản Marvell gốc, KHÔNG build
endif
```
```
DT  armada-quartz-emu800.dts:  bspi0: bspi0@e8273000 { compatible="marvell,bspi"; reg=<0xe8273000 0x100>;
       memmap_base=<0xf4000000 0x700000>; memmap_size=<0x700000>; chip_select=<0>; erase_cmd=<0x20>; sector_size=<0x1000>; … }
    bspi1: { same reg; memmap_base=<0xf8000000 0x600000>; chip_select=<1>; … }        (KM_BIZHUB: xóa 4 KB sector, không phải 64 KB)
 ▼
initr_bspi (board_r.c:583) → puts("BSPI:   ") → bspi_initialize()         qspi_winbond.c:1095   [VENDOR+DRIVER]
   ├─ fdtdec_find_aliases_for_id(blob,"marvell,bspi",COMPAT_MVEBU_BSPI,node_list,10) → num_parts
   ├─ mỗi node: devices[i].register_base = fdt_get_regs_offs("reg")  (0xE827_3000)
   │            devices[i].base          = fdt_get_regs_offs("memmap_base") (cửa sổ XIP 0xF400_0000 / 0xF800_0000)
   │            size, chip_select, page_size, program/erase/write_en/read_id op (lệnh, số byte addr, dummy) ← đọc từ DT
   ├─ qspi_probe(i): đăng ký MTD: mtd.name="mrvl_bspiN", _read=__qspi_read, _write=__qspi_write, _erase=__qspi_erase, type NOR
   │       └─ winbond_quad_enable(); add_mtd_device(&mtd)
   └─ qspi_write_lpddr4_training_result()   ← lưu kết quả huấn luyện LPDDR4 (DDR 0x2_0000_0000) vào QSPI 0xF438_0000 (KM_RESUME_REFINE) — xem Phần 11
 ▼
Đọc:  qspi_read() = memcpy_fromio(buffer, base + addr, size)     ← chỉ là đọc bộ nhớ cửa sổ XIP (phần cứng tự phát lệnh SPI)
Ghi/xóa: qspi_write/qspi_erase → execute_cmd(): ghi BSCR (mode/địa chỉ/dummy), BSCMDR (0x200 = assert CS; 0x500+byte = gửi byte), chờ BSSR.COMMANDBUSY
```
Consumer chính: `read_rom()`/`THRG_getSoftDipSw()`/`THRG_getBspThrRegion()` (`km/mtd_flash_access.c`, 1778 dòng) — nơi KM lưu MAC, machine info, DIP mềm, cờ chẩn đoán.

## 6.6 Trace driver thật #3 — xHCI USB3 (trên chip "Iris")
```
DT  quartz-iris-sb-0.dtsi:  usb3h0: usb3@d00d0000 { compatible="marvell,mvebu-usb3"; reg=<0xd00d0000 0x8000>; status="disabled" }
    armada-quartz-emu800.dts:  quartz-iris-sb-0 { usb3h0: usb3@d00d0000 { status="okay"; } }     ← bật ở đây
 ▼
Lệnh `usb reset` hoặc usb_init() (gọi từ setup_MotionEnable_Power, machine_setup.c)  → common/usb.c usb_init
 ▼
xhci_hcd_init(index, &hccr, &hcor)     drivers/usb/host/xhci-mvebu.c            [DRIVER glue]
   ├─ [UHOST_XHCI_CUSTOMIZE = CONFIG_USB_XHCI && KM_BIZHUB, mvebu-common.h:117]  phy_utm_mv_init()  (UTMI PHY)
   │        (mv_usb3_phy_init bị vô hiệu bằng `#if 0` — in "[WP30 REQ1] Skipped mv_usb3_phy_init()")
   ├─ board_usb_get_enabled_port_count(): fdtdec_find_aliases_for_id(blob,"usb3",COMPAT_MVEBU_USB3,…)  (quét DT 1 lần)
   ├─ hccr = fdt_get_regs_offs(node,"reg") = 0xD00D_0000; hcor = hccr + HC_LENGTH(cr_capbase)
   └─ usb_vbus_init(node): chỉ chạy nếu MVEBU_GPIO (không bật) ⇒ no-op
 ▼
xhci.c/xhci-mem.c/xhci-ring.c (lõi UPSTREAM): reset controller, cấp ring/DCBAA, quét cổng → usb_hub (KM sửa nhiều, UHOST_XHCI_CUSTOMIZE)
 ▼
usb_stor_scan → usb_storage.c (BOT/SCSI) → block_dev_desc_t (IF_TYPE_USB) → fatload usb 0:1 …
```

## 6.7 Trace driver thật #4 — PCIe (MSB0125) → NVMe SSD
```
DT  quartz-iris-sb-0.dtsi:  pcie-controller { compatible="marvell,msb0125-pcie";
        pcie@2,0 { phy=<0xd00a2000 0x1000>; ctrl=<0xd0098000 0x8000>; app=<0xd00a0000 0x10000>;
                   top=<0xd0040000 0x10000>; mem=<0xd0200000 0x700000>; cfg=<0xd0090000 0x8000>; status="disabled"; } }
    emu800.dts: bật `pcie@2,0` = "okay"
 ▼
initr_pci → pci_init() → pci_init_board()        drivers/pci/pcie-mv-msb0125.c:256           [DRIVER, tên hàm do U-Boot gọi]
   ├─ fdtdec_find_aliases_for_id(blob,"pcie-controller",COMPAT_PCIE_MV_MSB0125,…)
   ├─ fdt_for_each_subnode: skip nếu !enabled
   ├─ phy/ctrl/app/top = fdt_get_regs_offs(port,"phy"|"ctrl"|"app"|"top")
   ├─ msb0125_pcie_comphyh_check_status()   đọc COMPHY_H: PLL_READY_TX & PCLK enable
   ├─ msb0125_pcie_host_init(): nếu link chưa lên → bật LTSSM (APP_CTRL |= LTSSM_EN), chờ LINKUP tối đa 1000 µs   ("Link not up after reconfiguration" nếu fail)
   ├─ [KM_PCIE_SET_MPS] đặt Max Payload Size theo capability của RC
   └─ dw_pcie_init(host_id, regs_base, &mem_win, &cfg_win, first_busno)  (drivers/pci/pcie_dw.c)  → đăng ký `pci_controller`; sau đó pci_auto_config
 ▼
`nvme init` / lúc boot:  common/cmd_nvme.c __nvme_initialize() → init_nvme(0) → scan_nvme(0)
   ├─ pci_find_devices(&vendor_table[i]) — bảng cứng (Samsung, Toshiba/KIOXIA, Micron, Phison, Intel, SanDisk…) + bảng vendor bổ sung đọc từ SPI-flash (DEF_OFS_UBOOT_SSDVENDOR_TABLE)
   ├─ map BAR0, dựng admin/IO queue, doorbell (drivers/block/nvme.c: nvme_submit_cmd, nvme_submit_sync_cmd, nvme_identify_ctrl/ns)
   └─ block_dev_desc_t { if_type=IF_TYPE_NVME, block_read=nvme_read, block_write=nvme_write } ; init_part() ; hwcSetBootDeviceNormal(NVME)
```
> Chưa trace từng dòng bên trong `nvme_io()`/queue (file 4085 dòng); các tên hàm nêu ở trên đã xác minh bằng grep, chi tiết DMA/PRP **chưa đọc**.

## 6.8 Driver thật #5 — GPIO/UART/I2C (không dùng DT)
Ba driver này **không lấy `reg` từ DT**: địa chỉ nằm cứng trong header (`mvebu-common.h:264-294`, `quartz-sb.h`, `mvqz_gpio_regs.h`). Xem Phần 9 (UART) và Phần 10 (GPIO). Đây là lý do Marvell ghi chú trong `mvebu-common.h:265-268`: *"UART driver is basic driver for loading U-Boot; if any issue in FDT… U-Boot will stuck"*.

---

# PHẦN 7 — DEVICE TREE

## 7.1 DT được nạp và dùng thế nào
| Câu hỏi | Trả lời | Bằng chứng |
|---|---|---|
| DTB lấy từ đâu? | **Nhúng trong image** (`OF_EMBED`): `dts/dt.dtb` link vào `libs`, `gd->fdt_blob = __dtb_dt_begin` | `Makefile:609 libs-$(CONFIG_OF_EMBED) += dts/`; `board_f.c:setup_fdt` |
| File nguồn | `armada-quartz-emu800.dts` (`CONFIG_DEFAULT_DEVICE_TREE`); **DTS này tự `#define CONFIG_KM_BIZHUB`** và `#include "apn-806-z1.dtsi", "quartz-sb.dtsi", "quartz-iris-sb-0.dtsi"` | `arch/arm/dts/armada-quartz-emu800.dts:15-24` |
| Có gì trùng tên gây nhầm? | `dts/Makefile:71`: `dtb-$(CONFIG_QUARTZ) += armada-quartz-emu800.dtb` khi `KM_BIZHUB`, còn `armada-quartz-toc.dtb` chỉ khi **không** `KM_BIZHUB`. Với KM, `toc.dts` **không** được build vào U-Boot. | `arch/arm/dts/Makefile` |
| Căn DTB | `DTC_FLAGS += -R 4 -S $(CONFIG_FDT_SIZE)` (Armada8k: pad cố định để chọn multi-DT) | `arch/arm/dts/Makefile` (cuối) |
| Kernel dùng DTB này? | **Không.** DTB của **Linux** nạp từ storage vào `fdt_addr=0x1000` theo env `dtb_name` (`emu800.dtb`/`eseval.dtb`/`armada-quartz-toc.dtb`) rồi `booti $kernel_addr $initrd_addr $fdt_addr`. DTB điều khiển của U-Boot ≠ DTB kernel | `armada8k.h:119-130`, `quartz.c` |
| Copy khi relocate | `reserve_fdt` chừa `ALIGN(totalsize+0x1000, 32)`, `reloc_fdt` `memcpy` sang vùng mới | `board_f.c` |

## 7.2 Cơ chế (khác DM)
```
compatible  ──(bảng enum + chuỗi)──►  lib/fdtdec.c:  COMPAT(MVEBU_XENON_MMC,"marvell,xenon-sdhci") …
node        ──fdtdec_find_aliases_for_id(blob,"<alias>",COMPAT_ID,list,max)──►  danh sách node (alias trước, rồi node không alias theo thứ tự cây)
"status"    ──fdtdec_get_is_enabled──►  phải == "okay" (hoặc vắng mặt); "disable"/"disabled" ⇒ bỏ
"reg"       ──fdt_get_regs_offs(blob,node,"reg")──►  đọc address cell, rồi ĐI NGƯỢC node cha, mỗi cấp có "ranges" thì cộng offset
                                                   (arch/arm/cpu/mvebu-common/fdt.c:46-84); cha KHÔNG có "ranges" thì bỏ qua cấp đó
tham số     ──fdtdec_get_int/bool/array──►  memmap_base, bus-width, xenon,emmc, chip_select, force_cap_speed…
phandle     ──fdtdec_lookup_phandle(blob,node,"clock")──► soc_clock_get() (arch/arm/cpu/mvebu-common/clock.c) — chỉ có consumer `orion-spi` (không build) ⇒ hầu như không dùng
```
Ví dụ đi ngược `ranges`: `ap-806/internal-regs { ranges = <0x0000 0xf0000000 0x1000000>; }` ⇒ node con `sar-reg reg=<0x6F8200>` → **0xF06F_8200**. Còn `quartz-sb`/`quartz-iris-sb-0` **không có `ranges`** ⇒ `reg` là địa chỉ tuyệt đối (`0xE827_8000`, `0xD00D_0000`).

Thuộc tính DT **không** được U-Boot dùng ở build này: `interrupts`, `resets`, `clocks` (kiểu Linux), `pinctrl-*` (không có pinctrl). Các node `i2c0`, `spi0` (Marvell orion), `comphy`, `thermal`, `pinctl`, `ccu`, `rfu`, `tclk` tồn tại trong DTS nhưng **driver tương ứng không build hoặc chỉ dump** (§0.3).

## 7.3 Bốn thiết bị đã trace đầy đủ
Chuỗi thống nhất: **DT node → compatible → hàm tìm node → "device object" (struct tĩnh) → init → thanh ghi**

| # | DT node (địa chỉ) | compatible | Hàm khớp | "Device object" | Init | Phần cứng chạm đầu tiên |
|---|---|---|---|---|---|---|
| 1 | `quartz-sb/mmc@e8278000` | `marvell,xenon-sdhci` | `board_mmc_init` (`xenon_mmc.c:1157`) | `struct xenon_mmc_cfg` (calloc) + `struct mmc` | `xenon_mmc_create` → `xenon_mmc_init` | `SDHCI_HOST_VERSION/CAPABILITIES` (đọc), `SDHCI_RESET_ALL` (ghi), rồi `SDHC_SLOT_FIFO_CTRL=0x315`, power, clock, PHY |
| 2 | `bspi0@e8273000`, `bspi1@e8273000` | `marvell,bspi` | `bspi_initialize` (`qspi_winbond.c:1095`) | `struct qspi_s devices[10]` + `struct mtd_info` | `qspi_probe`, `winbond_quad_enable` | BSCR/BSCMDR (QSPI ctrl) + cửa sổ XIP `0xF400_0000`/`0xF800_0000` |
| 3 | `quartz-iris-sb-0/usb3@d00d0000` | `marvell,mvebu-usb3` | `board_usb_get_enabled_port_count`/`xhci_hcd_init` (`xhci-mvebu.c`) | `hccr/hcor` (con trỏ), `xhci_ctrl` (lõi) | `phy_utm_mv_init` rồi lõi xHCI | UTMI PHY regs; `cr_capbase` @0xD00D_0000 |
| 4 | `quartz-iris-sb-0/pcie-controller/pcie@2,0` | `marvell,msb0125-pcie` | `pci_init_board` (`pcie-mv-msb0125.c:256`) | `struct msb0125_pcie[]` → `pci_controller` | `msb0125_pcie_host_init` → `dw_pcie_init` | COMPHY-H `phy=0xD00A2000` status; `APP_CTRL.LTSSM_EN` @0xD00A0000 |
| 5 | `ap-806/internal-regs/sar-reg` | `marvell,sample-at-reset-common`, `marvell,sample-at-reset-ap806` | `mvebu_sar_init` (`drivers/misc/mvebu_sar/chip_sar.c:97`) | `soc_sar_info[]` | `chip_cfg_ptr->sar_init_func` | đọc SAR @0xF06F_8200 (tần số fabric, boot source) |

Node thử "thiếu driver": `thermal@6f8084` (`status="okay"`) — không có code U-Boot nào tiêu thụ (`mvebu_thermal_sensor_probe` là weak → 0).

Lưu ý dữ liệu DT: `cpus/cpu@0 compatible="arm,cortex-a57"` (ghi A57 dù core thật là A72 theo ATF context); U-Boot không dùng cho quyết định phần cứng ngoài `cpu-dt.c`.

---

# PHẦN 8 — MEMORY

## 8.1 Bản đồ bộ nhớ (địa chỉ **vật lý**, cấu hình tham chiếu)
### Vùng DRAM thấp (0x0000_0000 …)
| Địa chỉ | Kích thước | Nội dung | Nguồn |
|---|---|---|---|
| `0x0000_1000` | ~`mon_len` | **U-Boot lúc chạy lần đầu** (link addr = load addr = `CONFIG_SYS_TEXT_BASE`) — *sau relocate vùng này thành trống*, nên `fdt_addr=0x1000` dùng lại được | `mvebu-common.h:55`; `armada8k.h:127` |
| `0x0000_1000` (env) | — | `fdt_addr` = nơi nạp DTB **của kernel** | `armada8k.h:127` |
| `0x0080_0000` | — | `kernel_addr` (Image, mốc 8 MB) | `armada8k.h:126`, `quartz.c KERNEL_ADDR` |
| `0x00FF_1000` | — | **SP khởi tạo** (`INIT_SP_ADDR`); gd ở ngay dưới; "16 MB đầu bị training scrub" (comment) | `mvebu-common.h:71` |
| `0x0200_0000` | 1 MB | **pre-console buffer** (`CONFIG_PRE_CON_BUF_ADDR`, 1 MB) — *trùng địa chỉ với* `CPU_RELEASE_ADDR` (spin-table, `mvebu-common.h:253`) và `CONFIG_SYS_LOAD_ADDR` (`:127`) | `mvebu-common.h:127,152-154,253` |
| `0x0300_0000` | — | `initrd_addr` (initramfs `irfs.ubt`) | `armada8k.h:120` |
| `0x0400_0000` | — | **sys_info** (ATF/BLE → U-Boot) | `system_info.h:22` |
| `0x1000_0000` | 4 MB | **ATF** (`PLAT_MARVELL_ATF_BASE`, TRUSTED_ROM_SIZE=4M) | `armada8k.h:106-107` |
| `0x1040_0000` | 1 MB | vùng huấn luyện DDR ngay sau ATF | `armada8k.h:110-111` |
| `0x1080_0000` | — | `SYS_MEMTEST_SCRATCH` (dữ liệu tạm của `mtest`) | `mvebu-common.h:174` |
| `0x0000_0000`–`0x1000_0000` | — | vùng `mtest` (`MEMTEST_START..END`): dừng đúng ở đáy ATF | `mvebu-common.h:82-83` |
| … `0x3FE1_0000`–`0x4000_0000` | xem §8.2 | vùng **U-Boot sau relocate** (từ trên xuống) | tính toán |
| `0x7FF0_0000` | 1 MB | **SCPI mailbox** (ATF ↔ kernel; ATF ghi `0x7FFF_0000` trong context — nằm trong 1 MB này) | `armada8k.h:114-115` |

### Hằng số nhưng **không tìm thấy nơi dùng**
`CONFIG_UBOOT_MAX_MEM_SIZE (3 GB)` và `MVEBU_IO_RESERVE_BASE (0xC000_0000)` (`mvebu-common.h:78-79`): grep toàn cây **không thấy call-site** ⇒ chỉ là định nghĩa, không tác dụng.

### Thanh ghi / MMIO quan trọng
| Vùng | Địa chỉ | Ghi chú |
|---|---|---|
| Internal regs AP806 | `0xF000_0000` (`MVEBU_REGS_BASE`) | `memory-map.h` |
| GIC-D / GIC-C | `0xF021_0000` / `0xF022_0000` | `regs-base.h` |
| Generic timer (CNTCR) | `0xF058_1000` | |
| UART0 (AP-UART, **COM1**) | `0xF051_2000` | `MVEBU_UART_BASE(0)` = REGS+0x512000 |
| Memory Controller (MMAP/RCR) | `0xF002_0200…`, `0xF000_1700…` | dùng ở `dram_init_banksize` |
| CP0 | `0xF200_0000` | |
| **Quartz SB**: DW-UART panel (**COM2**) / CSRC (**COM3**) | `0xE800_2800` / `0xE800_3000` | `mvebu-common.h:271-291` |
| DW-I2C ×4 | `0xE800_6000/6800/7000/7800` | `quartz-sb.h` |
| BSPI ctrl / eMMC | `0xE827_3000` / `0xE827_8000` | DT |
| PADWRAP PIOCFG / GPIO A-I | `0xE830_8000` / `0xE830_A000 + 0x100·bank` | `mvqz_gpio_regs.h` |
| BSPI XIP window CS0 / CS1 | `0xF400_0000` (7 MB) / `0xF800_0000` (6 MB) | DT `memmap_base` |
| **Iris SB**: USB3 / PCIe(app,ctrl,cfg,phy,top,mem) | `0xD00D_0000` / `0xD00A_0000`, `0xD009_8000`, `0xD009_0000`, `0xD00A_2000`, `0xD004_0000`, `0xD020_0000` | `quartz-iris-sb-0.dtsi` |
| LCD LVDS | `0xC057_E000` | `mvebu-common.h:307` |

## 8.2 Vị trí U-Boot sau relocation (tính từ code)
Điều kiện: `dram_init` đặt `ram_size = 0x8000_0000`, nhưng `get_effective_memsize()` giới hạn **1 GB** ⇒ `ram_top = 0x4000_0000` (bất kể máy có 5/6/8 GB).
Các hàm `reserve_*` trừ từ trên xuống (`board_f.c`):

```
0x4000_0000  ram_top = relocaddr ban đầu                                   setup_dest_addr
   − 0x1_0000   reserve_mmu  : bảng trang MMU  (PGTABLE_SIZE=0x10000), căn 64 KB  → tlb_addr = 0x3FFF_0000
   − 0x1E_0000  reserve_lcd  : VRAM = 1280×768×2 B = 1 966 080 B (=0x1E0000)      → fb_base ≈ 0x3FE1_0000   [VENDOR]
   − mon_len    reserve_uboot: ảnh U-Boot (text+data+bss), căn 4 KB              → **relocaddr** = (0x3FE1_0000 − mon_len)&~0xFFF ; start_addr_sp = relocaddr
   − 0x51_0000  reserve_malloc: TOTAL_MALLOC_LEN = 5 MB + ENV_SIZE 0x10000        → heap chính
   − sizeof(bd_t)         reserve_board   (bd->bi_dram[3]…)
   − sizeof(gd_t)  (~270 KB do printBuffer KM)  reserve_global_data → new_gd
   − ALIGN(fdt+0x1000,32) reserve_fdt      → new_fdt
   − 16           reserve_stacks  → start_addr_sp (stack mới, mọc xuống)
```
* `reloc_off = relocaddr − 0x1000` (`setup_reloc`).
* Giá trị `mon_len` cần `u-boot.map`; hằng số khác đều tính từ header (**[CONFIG]**, số liệu đã kiểm).
* Hệ quả: **toàn bộ U-Boot + heap + stack nằm ở 1 GB đầu của DRAM**, dù RAM lớn hơn; RAM còn lại chỉ được dùng để nạp kernel/initrd, dữ liệu.

## 8.3 gd, bd, heap, stack, env
| Đối tượng | Vị trí | Ghi chú |
|---|---|---|
| `gd` (pre-reloc) | `0x00FF_1000 − GD_SIZE` (x18) | `crt0_64.S:64-72`; thanh ghi `x18` giữ `gd` xuyên suốt (`-ffixed-x18`, `arch/arm/cpu/armv8/config.mk`) |
| pre-reloc heap | `gd − 0x5000` | `CONFIG_SYS_MALLOC_F_LEN`, `initf_malloc` |
| `gd` (post) | `new_gd` (`reserve_global_data`) | `board_init_r` nhận `x18=new_gd` (`crt0_64.S:93-131`) |
| `bd->bi_dram[0..2]` | trong `bd_t` | điền bởi `dram_init_banksize` (Phần 11) |
| heap chính | `relocaddr − TOTAL_MALLOC_LEN` (`initr_malloc`) | 5 MB + 64 KB |
| Env (RAM) | trong heap (`env_relocate` → `malloc(ENV_SIZE)`) | mặc định `default_environment` trong `.rodata` |
| Env (lưu) | **eMMC boot partition 1 (`SYS_MMC_ENV_PART=1`), offset `CONFIG_ENV_OFFSET = CONFIG_UBOOT_SIZE = 0x200000`, dài `0x10000`, dev `SYS_MMC_ENV_DEV=0`** — **INFERRED** (§0.4) | `mvebu-common.h:226-232,198-209`; `common/env_mmc.c` |
| `ENV_OVERWRITE` | cho phép đổi biến "ethaddr/serial#" | `mvebu-common.h:90` |

## 8.4 MMU/cache (thật)
`initr_caches → enable_caches → dcache_enable → mmu_setup` (`arch/arm/cpu/armv8/cache_v8.c:26`):
1. Cả không gian địa chỉ: **identity map**, thuộc tính **Device-nGnRnE** (`MT_DEVICE_NGNRNE`) — mọi vùng MMIO uncacheable.
2. Với mỗi `bd->bi_dram[i]` (i < `CONFIG_NR_DRAM_BANKS`=3): đặt **`MT_NORMAL`** (cacheable).
3. `set_ttbr_tcr_mair(el, tlb_addr, TCR_FLAGS|TCR_ELx_IPS_BITS, MEMORY_ATTRIBUTES)`; `VA_BITS=42`; bật `SCTLR.M`, rồi `SCTLR.C`.

Chi tiết dễ sai: `SECTION_SHIFT = 29` (`asm/armv8/mmu.h:35`) ⇒ **mỗi mục bảng trang phủ 512 MB**, và vòng lặp dùng `size >> 29` ⇒ nếu bank không bội số 512 MB thì phần dư bị **bỏ qua (vẫn là device)**. Với khai báo bank hiện có (bội số 512 MB), không phải lỗi.
`cleanup_before_linux()` (`cpu.c`): tắt IRQ, tắt+invalidate I-cache, `dcache_disable()` (KM: flush toàn bộ + invalidate TLB trước) — kernel nhận bộ nhớ sạch cache.
Nhánh KM: `flush_dcache_all()` gọi thêm `__asm_flush_l3_cache()` (LLC); ATF context cho biết LLC **tắt** (`LLC_DISABLE=1`), U-Boot in trạng thái bằng `print_soc_specific_info → llc_mode_get`.

## 8.5 Relocation — giải thích kỹ
**Vì sao cần:** U-Boot được ATF nạp ở `0x1000` (đáy DRAM, nơi Linux thích dùng cho DTB/kernel). Nó **tự chuyển lên đỉnh 1 GB** để nhường vùng thấp.

```
(1) LINK ADDRESS
    Makefile:755  LDFLAGS_u-boot += -Ttext $(CONFIG_SYS_TEXT_BASE)   = 0x1000
    config.mk (arch/arm)  LDFLAGS_u-boot += -pie                     ← tạo PIE, có bảng .rela.dyn
    u-boot.lds  .text → .rodata → .data → .u_boot_list → __image_copy_end → __rel_dyn_start .rela.dyn __rel_dyn_end → _end → __bss_start … __bss_end

(2) BUILD: u-boot.bin  (Makefile:822-826)
    objcopy → u-boot.bin ; DO_STATIC_RELA(u-boot, u-boot.bin, CONFIG_SYS_TEXT_BASE)
    (chỉ chạy khi CONFIG_STATIC_RELA — arch/arm/include/asm/config.h:15 tự định nghĩa cho ARM64; công cụ tools/relocate-rela)
    ⇒ áp sẵn các relocation R_AARCH64_RELATIVE với base = 0x1000 vào chính u-boot.bin, để con trỏ trong .data/.rodata đã đúng
      khi ảnh chạy tại 0x1000 (lúc chưa relocate). Bảng .rela.dyn vẫn còn nguyên để relocate_code dùng lần nữa.

(3) RUNTIME ADDRESS lần 1 = 0x1000 (ATF nhảy tới đây). Chạy _start → _main → board_init_f  (đều ở 0x1000)

(4) board_init_f tính: relocaddr (§8.2), reloc_off = relocaddr − 0x1000, new_gd, new_sp, new_fdt  (setup_reloc)

(5) crt0_64.S (86-107):
      x0 = gd->start_addr_sp → sp ; x18 = gd->bd − GD_SIZE (KM: dùng ldr x1,=GD_SIZE)  ← gd mới ngay dưới bd
      lr = &relocation_return + gd->reloc_off      ← "địa chỉ trả về" đã cộng offset ⇒ sau relocate_code nhảy vào BẢN SAO
      x0 = gd->relocaddr ; b relocate_code

(6) relocate_code (relocate_64.S:22-77)
      x1 = __image_copy_start ; x9 = x0 − x1 = relocation offset ; nếu 0 → bỏ qua
      copy_loop: ldp/stp 16 byte từ [__image_copy_start .. __image_copy_end) → [relocaddr …)
      fixloop:  duyệt .rela.dyn: mỗi mục 24 byte (offset, info, addend);
                nếu (info & 0xFFFFFFFF) == 1027 (R_AARCH64_RELATIVE):
                       *(offset + x9) = addend + x9          ← FIXUP: con trỏ trong bản sao trỏ vào bản sao
      cuối: nếu I/D-cache bật thì `ic iallu; isb` + `__asm_flush_dcache_range`; ret (về lr đã cộng offset)

(7) relocation_return (bản sao, địa chỉ mới): c_runtime_cpu_setup (đặt lại VBAR = vectors mới)
      xóa BSS [__bss_start, __bss_end)  (địa chỉ đã auto-relocate qua fixup)
      b board_init_r(gd, relocaddr)      ← từ đây chạy 100% ở địa chỉ mới; vùng 0x1000 bị bỏ trống
```
Những gì **được sao chép/ fix**: chỉ `[__image_copy_start, __image_copy_end)` (text+rodata+data+u_boot_list), **không** gồm BSS/`.rela.dyn`. Con trỏ hàm (vd. `init_sequence_r[]`, `U_BOOT_CMD` list, `serial_device` con trỏ) được sửa bởi `fixloop`. Riêng `CONFIG_NEEDS_MANUAL_RELOC` **không** dùng (ARM64 dùng PIE).
Sau relocate: `initr_reloc` set `GD_FLG_RELOC`; `serial.c:get_current()` dùng cờ này để chuyển từ `default_serial_console()` sang `serial_current` (Phần 9).

## 8.6 Kernel / initrd / DTB / reserved
* `booti $kernel_addr $initrd_addr $fdt_addr` (`CONFIG_CMD_BOOTI` khi ARM64). `CONFIG_SYS_BOOTM_LEN = 20 MB`, `SYS_BOOTMAPSZ = 16 MB`.
* Kernel Image ở `0x80_0000`, initrd ở `0x300_0000`, DTB ở `0x1000`, mọi thứ nằm **dưới 1 GB** — khớp giới hạn.
* Vùng cấm chạm: ATF `0x1000_0000–0x1040_0000`, training `0x1040_0000–0x1050_0000`, SCPI `0x7FF0_0000`. Trong phần code đã đọc, U-Boot **không** thêm `/memreserved` vào DTB kernel; việc này do ATF/DTB kernel đảm nhiệm (không xác minh phía kernel ở đây).
* `warp_prepare_boot()` (Warp!!/hibernate, `armada8k.h:349`): `smp_kick_all_cpus(); flush_dcache_all();` rồi ghi `0x100` vào timer `0xF063_0000` — nhánh khôi phục nhanh của KM (ngoài phạm vi phần này).

---

# PHẦN 9 — CONSOLE / UART

## 9.1 UART thật của board
| Cổng | Địa chỉ | Macro | Vai trò | Đăng ký làm console? |
|---|---|---|---|---|
| COM1 = "eserial0" | `0xF051_2000` (AP806 UART0, **16550 nội**) | `CONFIG_SYS_NS16550_COM1 = MVEBU_UART_BASE(0)` | **Debug terminal kiểu "AP-UART"** | ✔ (`serial_register(&eserial1_device)`) |
| COM2 = "eserial1" | `0xE800_2800` (DW-UART0 trong Quartz SB) | `CONFIG_SYS_NS16550_COM2`, `CONFIG_PANEL_PORT` | **Giao tiếp với panel LCD/nút** (57600, **8E1**) | ✘ (`&& !defined(KM_BIZHUB)` — KM cố ý không đăng ký làm console) |
| COM3 = "eserial2" | `0xE800_3000` (DW-UART, "CSRC UART") | `CONFIG_CSRC_PORT` | **Debug terminal kiểu "CSRC-UART"** | ✔ |
Thanh ghi 16550 dạng **32-bit** (`CONFIG_SYS_NS16550_MEM32`, `REG_SIZE=-4`) ⇒ mỗi thanh ghi cách 4 byte, truy cập `in_le32/out_le32`. Mặc định `CONFIG_CONS_INDEX=1`, `BAUDRATE=115200` (`mvebu-common.h:284,322`).

## 9.2 Chọn cổng debug (KM)
```
init_baud_rate (board_f.c:143)   [gọi từ init_sequence_f, TRƯỚC serial_init]
   gpio_set_direction(FW_CONFIG4=270, IN); gpio_set_direction(FW_CONFIG5=271, IN)
   if (gpio_get_value(270)==LOW || gpio_get_value(271)==LOW)  term_sel = 1  → AP-UART  (eserial1_device, 0xF051_2000)   ← "máy debug" hoặc "ES"
   else                                                        term_sel = 0  → CSRC-UART (eserial3_device, 0xE800_3000)
default_serial_console()  (drivers/serial/serial_ns16550.c:228-253):  return term_sel ? &eserial1_device : &eserial3_device
```
Lưu ý: `FW_CONFIG5` cũng dùng ở `get_quartz_board()` (giá trị 1 ⇒ ESEVAL) — cùng một chân, hai mục đích.

## 9.3 Chuỗi gọi TX (từ `printf` xuống thanh ghi)
```
printf("...")                                   lib/vsprintf.c  (vscnprintf vào buffer)
   └─ puts(buf)                                 common/console.c
        ├─ [VENDOR, KM_BIZHUB]  BspPrintStopCheck()   chống gọi đệ quy (gc_stopflg)
        ├─ [VENDOR]  BspPrintValid()  ── getTermState() ≠ TERM_STATE_0 ?  (§9.5)
        │       ├─ KHÔNG → BspPrintBuffering(s): sprintf vào gd->arch.printBuffer[n++] (tối đa 255 dòng, sau đó ném) ; return   ← KHÔNG ra UART
        │       └─ CÓ    → BspPrintBufferFlush(): lần đầu in lại toàn bộ dòng đã đệm bằng puts()
        ├─ CONFIG_SILENT_CONSOLE: nếu gd->flags&GD_FLG_SILENT (env "silent") → return
        ├─ !gd->have_console → pre_console_puts(s)   (chép vào buffer 0x0200_0000, 1 MB)
        └─ GD_FLG_DEVINIT ? fputs(stdout, s) : serial_puts(s)
              stdout → stdio_dev "serial" → serial_puts                          drivers/serial/serial.c:503
                  └─ get_current()->puts(s)                                        serial.c:388
                         (chưa relocate hoặc chưa serial_assign → default_serial_console() ; sau đó serial_current)
                       = eserialN_puts(s)  (macro DECLARE_ESERIAL_FUNCTIONS, serial_ns16550.c:80)
                           └─ serial_puts_dev(port,s) → _serial_puts → _serial_putc(c,port)
                                  '\n' → gửi thêm '\r' trước
                                  └─ NS16550_putc(PORT, c)                          drivers/serial/ns16550.c:235
                                        while(!(LSR & UART_LSR_THRE)) ;            ← chờ FIFO/THR trống (LSR offset 0x14 vì cách 4 byte)
                                        THR = c                                     ← ghi 32-bit vào thanh ghi TX
                                        if (c=='\n') WATCHDOG_RESET()
```
**Điểm mấu chốt:** `putc()/puts()/printf()` **không** đi thẳng xuống UART. KM chèn một tầng "log gating" phía trên (`BspPrint*`) — *xem §9.5*.

## 9.4 Khởi tạo UART & baud (RX cũng vậy)
```
serial_init()  (serial.c:422)      gd->flags |= GD_FLG_SERIAL_READY ; return get_current()->start()
   = eserialN_init:  clock_divisor = ns16550_calc_divisor(port, CONFIG_SYS_NS16550_CLK, gd->baudrate);  NS16550_init(port, div)

ns16550_calc_divisor (ns16550.c:117-143)  [CONFIG_MVEBU + KM]
   nếu port thuộc DW UART (địa chỉ & 0xFFFF0000 == 0xE800_0000):  div = (7 380 000/16)/baud   ← "25 MHz /(0x271/0x171)/2" (comment: 88PAQZ01 datasheet 6.1.2.1, 6.1.2.63)
   ngược lại (AP-UART):                                             div = (soc_tclk_get()/16)/baud, với soc_tclk_get()=soc_mss_clk_get()=CONFIG_MSS_FREQUENCY=200 MHz
   ⇒ 115200: DW ⇒ div 4 (thực ≈115 312 baud) ; AP ⇒ div 108

NS16550_init (ns16550.c:155):
   (MVEBU_AP806_16750 ✗ tắt: không đụng MPP — pinmux UART đã do ATF BLE đặt: ble_main.c ghi 0xF06F4004/8)
   chờ TEMT → IER=0 → setbrg(0) → MCR=DTR|RTS → FCR=FIFO_EN|RXSR|TXSR → setbrg(div)  (LCR.BKSE: DLL/DLM) → LCR=8N1
   [KM] nếu port == CONFIG_PANEL_PORT: LCR = 8N1|EPS|PEN  (parity chẵn)         ← panel UART 8E1

RX:  main_loop → readline → getc → serial_getc → get_current()->getc → eserialN_getc → NS16550_getc
        polling LSR & UART_LSR_DR ; [KM] với PANEL_PORT: timeout 5000×100 µs = 500 ms, quá hạn ⇒ uart_error=1 (init_km dùng để báo panel lỗi)
     tstc → NS16550_tstc (LSR&DR)
```
Sau relocate: `initr_serial → serial_initialize()` gọi ~50 hàm `*_serial_initialize()` (đa số rỗng theo config) — `ns16550_serial_initialize()` đăng ký COM1 & COM3 (KM bỏ COM2), rồi `serial_assign(default_serial_console()->name)`. `console_init_r` gán `gd->jt[XF_putc]=serial_putc`… và `stdout="serial"`.

## 9.5 Vì sao bạn có thể **không thấy log nào** (rất hay gặp)
`BspPrintValid()` (`common/console.c`) chỉ cho in khi `getTermState() != TERM_STATE_0`:
```
getTermState()  km/dipvalue.c:48
   if (gd->arch.termFlg == 0) return TERM_STATE_0            ← termFlg chỉ =1 sau stdio_add_devices (initr, #17)
   cache term_state (static)
   if (isForceLogOutput()) → TERM_STATE_1
        isForceLogOutput: FW_CONFIG5(271)==LOW (ES) → 1 ; else gpio_get_value(LOG_SW=233 = GPIOH[25])   [High = ép in log]
   else đọc DIP mềm trong SPI-flash: THRG_getSoftDipSw(0x51) bit7 (→bit2) và THRG_getSoftDipSw(0x21) bit0-1
        giá trị → TERM_STATE_1..4 ("MFP debug", "CSRC debug", "D103 debug", "PIC debug")  hoặc TERM_STATE_0 ("không debug")
```
* Sản phẩm bình thường ⇒ `TERM_STATE_0` ⇒ **mọi `printf` chỉ vào bộ đệm 255 dòng trong `gd`, không ra UART**; vượt 255 dòng thì mất. (Cũng vì vậy DDR training log của ATF cũng có cơ chế tương tự: `km_dipvalue.c getLogOutput()` đọc **cùng chân LOG_SW** — ATF `context` §2.)
* Muốn có log: kéo `LOG_SW` (GPIOH[25]) lên cao, hoặc board ES/debug (`FW_CONFIG5` thấp), hoặc đặt DIP mềm. Lệnh `db_sw` (`dipvalue.c`) in `Force Log Output` và `Terminal State`.
* `CONFIG_SILENT_CONSOLE` (env `silent`) là cơ chế **upstream** riêng, cộng dồn với cơ chế KM.

## 9.6 Ba mức "console" trong code (đừng lẫn)
| Mức | Hàm | Khi nào |
|---|---|---|
| Trước console | `pre_console_putc/puts` (buffer `0x0200_0000`) | `!gd->have_console` |
| Gating KM | `BspPrint*` (buffer trong `gd`) | luôn, khi `KM_BIZHUB` |
| Console thật | `serial_putc/puts` → `eserialN_*` → `NS16550_*` | sau khi vượt hai lớp trên |
| Panel/LCD | `CONFIG_LCD_STDOUT` **không** bật (`/*#define CONFIG_LCD_STDOUT */`) ⇒ printf **không** ra LCD |

Bản kế: `docs/uboot/UBOOT_P10-P13_clock_gpio_ddr_storage_fs.md`


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
