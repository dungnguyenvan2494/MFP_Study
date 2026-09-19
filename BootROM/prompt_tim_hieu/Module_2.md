# Phần 1: Repository map

Đây là U-Boot **2015.01**, trước lần tái cấu trúc thư mục của upstream, nên **không có** `cmd/`, `env/`, `boot/`, `image/`. `spl/` và `tpl/` cũng không có (chúng chỉ là thư mục output khi build, và target chính không có SPL). Các thành phần đó nằm ở chỗ khác:

|Thư mục bạn hỏi|Nằm ở đâu trong repo này|
|---|---|
|`cmd/`|`common/cmd_*.c` (~100 file, mỗi file một lệnh). Lệnh Marvell nằm ở `common/mvebu/cmd_*.c`|
|`env/`|`common/env_*.c` và `common/mvebu/env_bootdev.c`|
|`boot/`|`common/main.c`, `autoboot.c`, `cmd_bootm.c`, `bootm.c`, `bootm_os.c`, cộng `arch/arm/lib/bootm*.c`|
|`image/`|`common/image.c`, `image-fdt.c`, `image-fit.c`|

Kiểm chứng thêm: `CONFIG_CMD_NET` **không** được bật. `armada8k.h` không include `config_cmd_default.h`, và Kconfig `CMD_NET` không có default (`common/Kconfig:233`). Vì vậy `net/` **không sinh ra object nào**, và điểm "chưa xác minh" về mạng ở Phần 0 đã được chốt là **không có network stack**.

## 1. Flow thực tế của repo này

Flow này khác mẫu Boot ROM → TPL → SPL → U-Boot proper. Target chính (`./build_uboot.sh 800`) **không có SPL/TPL**. Các bước từ ATF trở lên là suy luận (mã ATF nằm ngoài repo). Mọi hàm từ `_start` trở xuống được đọc trực tiếp từ source.

```
Boot ROM (silicon, ngoài repo)
   ↓
ATF (BL1/BL2/BL31 @0x10000000, ngoài repo)  ── DDR training, PSCI, SCPI
   ↓  nhảy vào U-Boot ở EL2/EL1
_start → reset                  [arch/arm/cpu/armv8/start.S]
   ├─ đặt VBAR, bật FP/SIMD, apply_core_errata
   ├─ flush cache/TLB, lowlevel_init
   │    (bỏ qua nếu CurrentEL ≠ EL3: "ATF đã làm hết")
   ├─ CPU phụ: wfe, chờ CPU_RELEASE_ADDR (spin-table)
   └─ CPU chính: bl _main
   ↓
_main                           [arch/arm/lib/crt0_64.S]
   ├─ SP = SYS_INIT_SP_ADDR (0x1000+0xFF0000), GD ngay dưới SP → x18
   └─ bl board_init_f
   ↓
board_init_f → init_sequence_f  [common/board_f.c]  (còn ở địa chỉ gốc 0x1000)
   setup_fdt → initf_malloc → arch_cpu_init → board_early_init_f → timer_init
   → env_init → init_baud_rate → serial_init → console_init_f
   → display_options → print_cpuinfo → dram_init → reserve_* → setup_reloc
   ↓
relocate_code                   [arch/arm/lib/relocate_64.S]
   ↓ (quay về "cùng chỗ nhưng đã relocate")
c_runtime_cpu_setup → xoá BSS → board_init_r
   ↓
board_init_r → init_sequence_r  [common/board_r.c]
   initr_caches → malloc → board_init → serial/stdio → initr_mmc
   → initr_bspi → init_km → initr_env → initr_pci → console_init_r
   → misc_init_r → interrupt → board_late_init → last_stage_init
   → run_main_loop
   ↓
main_loop → bootdelay_process   [common/main.c, autoboot.c]
   ↓
run_command_list(bootcmd)       [common/cli_hush.c → cmd_bootm.c: do_booti]
   ↓
Linux Image + DTB (+ initrd)
```

Ba điểm khác flow upstream mà bạn cần biết trước khi đọc code:

1. `start.S` có **hai `_start` hoàn toàn khác nhau**, chọn bằng `#if defined(CONFIG_MVEBU) && defined(CONFIG_SPL_BUILD)` ([start.S:23](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/start.S)). Nhánh SPL chỉ chạy `lowlevel_init_spl` rồi `board_init_f`, rồi **return** về BootROM/ATF. Nhánh U-Boot proper mới có `reset` và `_main`. Target `800` chỉ dùng nhánh thứ hai.
2. **Hai điểm chèn của Konica Minolta** trong `init_sequence_r` mà upstream không có: `initr_bspi` (khi `MRVL_QSPI`) và `init_km` (khi `KM_BIZHUB`). `init_km` là **chẩn đoán phần cứng lúc boot** (UART panel, nút nguồn, kiểm tra RAM). Nó chạy **trước** `initr_env` và trước console đầy đủ ([board_r.c:435-1017](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/board_r.c)).
3. `misc_init_r` **không phải** hàm rỗng trong `board/mvebu/common/init.c` (bị `#if !defined(CONFIG_KM_BIZHUB)` loại). Bản thật nằm trong [km/machine_setup.c:674](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/board/mvebu/armada8k/km/machine_setup.c).

## 2. Bản đồ thư mục

Cột "Trạng thái" cho biết thư mục đó có thực sự vào `u-boot.bin` của target `800` hay không. Con số là số file trong thư mục.

### Nhóm A: chỉ dùng lúc build (không có trong binary)

|Thư mục|Vai trò|Vị trí trong quá trình|Ghi chú thực tế|
|---|---|---|---|
|`configs/` (1172)|`*_defconfig`, chọn cấu hình bằng `make <x>_defconfig`|Trước build|Chỉ **21** file là của dự án này (`km_mvebu_quartz_*`, `emu800_*`, `egl*`, `spa*`, `dnb*`, `hemlk`, `mssb`, `mvebu_quartz_*`). Còn lại là board upstream không liên quan|
|`scripts/` (72)|Kconfig, `Makefile.build/lib/autoconf/spl`, `setlocalversion`|Build|Sinh `include/config.h`, `autoconf.mk`. `Makefile.spl` chỉ dùng nếu bật SPL|
|`tools/` (196)|Công cụ chạy trên host: `mkimage`, `envcrc`, `dtc` wrapper...|Build và hậu build|`tools/secure/` là cấu hình ký Marvell (đã nêu ở Phần 0). `arch/arm/cpu/mvebu-common/tools/` là công cụ mvebu riêng|
|`dts/` (2)|**Không chứa DTS.** Chỉ có Makefile nhúng DTB vào binary|Build|`CONFIG_OF_EMBED=y` nên `dts/dt.dtb` được biến thành `dt.dtb.o` link vào `u-boot`. DTS thật nằm ở `arch/arm/dts/`. Dòng 21–36 có nhánh `MULTI_DT_FILE` do Marvell thêm|
|`doc/`, `Licenses/`, `MAINTAINERS`, `MAKEALL`, `README`|Tài liệu, giấy phép|Không liên quan|`doc/README.kconfig` giải thích tiền tố `+S:` trong defconfig|
|`build_uboot.sh`|Wrapper của vendor|Build|Chọn defconfig, `mrproper`, `make`, copy `u-boot.bin`|

### Nhóm B: header và cấu hình (chi phối mọi thứ nhưng không chạy)

|Thư mục|Vai trò|Ghi chú thực tế|
|---|---|---|
|`include/` (1167)|Header. `include/configs/<board>.h` là nơi quyết định hầu hết hành vi|Chuỗi include: `armada8k.h` → `mvebu-common.h` → `quartz-sb.h`. Header vendor: `quartz.h`, `warp.h`, `lxk_panel.h`, `mfp_panel.h`, `nvme.h`... Địa chỉ phần cứng ở `arch/arm/include/asm/arch-armada8k/{memory-map,regs-base}.h`|
|Các file `Kconfig`|Khai báo tuỳ chọn|Menu vendor "KonicaMinolta customize" ở `Kconfig` gốc (dòng 164). `board/mvebu/Kconfig` là menu Marvell|

### Nhóm C: mã chạy trên thiết bị, xếp theo pha boot

**Pha 1: từ reset đến `_main` (assembly, địa chỉ 0x1000, chưa có stack C)**

|Thư mục / file|Vai trò|
|---|---|
|`arch/arm/cpu/armv8/`|`start.S` (reset vector), `cache.S`, `tlb.S`, `exceptions.S` (vector), `transition.S` (đổi EL), `psci.S`, `u-boot.lds` (linker script)|
|`arch/arm/lib/crt0_64.S`|`_main`, dựng stack/GD rồi gọi `board_init_f`. Vendor có sửa (`#if defined(KM_BIZHUB)` xử lý hằng `GD_SIZE` lớn)|

**Pha 2: `board_init_f`, trước relocation**

|Thư mục|Vai trò|Vendor / config|
|---|---|---|
|`common/board_f.c`|Danh sách `init_sequence_f`|`[VENDOR]` `init_baud_rate` đọc GPIO FW_CONFIG4 để chọn cổng debug, đặt cờ `printStopFlg` để đệm log|
|`arch/arm/cpu/armv8/armada8k/`|Phần SoC AP806: `soc.c`, `clock.c`, `cache_llc.c`, `mvebu_ccu.c`, `mvebu_rfu.c`|`[CONFIG]` `mci*`, `amber*`, `unas.c` **không build** (`MVEBU_A3700_*` tắt). `spl.c` chỉ trong SPL|
|`arch/arm/cpu/mvebu-common/`|Mã chung MVEBU: `soc-init.c`, `fdt.c`, `system_info.c`, `misc.c`, `generic_timer.c`|Được kéo vào qua `arch/arm/cpu/Makefile` (`obj-$(CONFIG_MVEBU)`). `mpp-bus.c` **tắt**|
|`board/mvebu/common/init.c`|`board_early_init_f`, `board_init`, `board_late_init`|`[BOARD]` `sys_info_init()` lấy dữ liệu từ ATF|
|`drivers/serial/`|NS16550 (`ns16550.c`, `serial_ns16550.c`)|`[DRIVER]` mã vendor: nhiều cổng, cổng panel 57600 baud|
|`drivers/gpio/`|`mvqz_gpio*.c`|`[DRIVER]` `[VENDOR]` phải chạy được **trước relocation** vì `init_baud_rate` gọi GPIO|
|`lib/` (103)|`fdtdec.c`, `libfdt/`, `vsprintf.c`, `string.c`, `hashtable.c`...|Thư viện lõi. `lib/rsa`, `lzma`, `bzip2`, `zlib`, `tizen` chỉ build nếu config bật|

**Pha 3: relocate**

|Thư mục|Vai trò|
|---|---|
|`arch/arm/lib/`|`relocate_64.S` áp `.rela.dyn`, `sections.c`, `gic_64.c`, `interrupts_64.c`, `reset.c` (KM có sửa)|

**Pha 4: `board_init_r`, sau relocation**

|Thư mục|Vai trò|Vendor / config|
|---|---|---|
|`common/board_r.c`|Danh sách `init_sequence_r`|`[VENDOR]` `initr_bspi`, `init_km` (chẩn đoán HW, comment lịch sử đến **2025/03**)|
|`drivers/mmc/`|`xenon_mmc.c` (SDHCI của Marvell), `mmc.c`|`[DRIVER]` `initr_mmc` gọi ở đây|
|`drivers/spi/`|`qspi.c`, `qspi_winbond.c` (bộ đọc flash NOR)|`[DRIVER]` `[VENDOR]` `MRVL_QSPI`|
|`drivers/pci/`|`pcie-mv-msb0125.c`, `pcie_dw.c`, `pci.c`|`[DRIVER]` `initr_pci` quét PCIe để thấy NVMe|
|`drivers/block/`|`nvme.o` (`KM_NVME`)|`[VENDOR]`|
|`drivers/usb/`|xHCI (`xhci*.c`), `phy-utm-mv.c`, `phy-usb3-mv.c` cùng `common/usb*.c`|`[DRIVER]` USB **không** tự khởi tạo, cần `usb reset`|
|`drivers/i2c/`|`designware_i2c.c`|`[DRIVER]` 4 bus, dùng để dò EEPROM nhận diện board|
|`drivers/scpi/`|`arm_scpi.c`|`[VENDOR]` nói chuyện với ATF (watchdog CA72, mailbox `0x7ff00000`)|
|`drivers/video/`, `common/lcd.c`|Điều khiển LCD/LVDS: `lxk_*`, `lcd_lvds_pll.c`, `km_panel.c`|`[VENDOR]`|
|`drivers/sound/`|`km-i2s.c`, `km-cdma.c`, `km-rt5640.c`, `km-sound.c`|`[VENDOR]` phát âm khi bật nguồn, gọi từ `board_init()`|
|`board/mvebu/armada8k/`|`armada8k.c`, `quartz.c`, `devel-board.c`|`[BOARD]` `quartz.c` nhận diện TOC/ESEVAL/EMU800, đặt `dtb_name`, chạy `bspi_boot`|
|`board/mvebu/armada8k/km/`|**Mã vendor chính:** `machine_setup.c`, `mfp_panel.c`, `userdata.c`, `mtd_flash_access.c`, `dipvalue.c`, `ca72wdt.c`, `picture.c`, `graphlib.c`, `pngfilter.c`, `mfp_config.c`, `init_debug.c`|`[VENDOR]` `[BOARD]` được kéo vào bằng `libs-$(CONFIG_KM_BIZHUB)` ở Makefile gốc dòng 650|
|`common/env_*.c`|Lưu và nạp biến môi trường|`[CONFIG]` chỉ `env_nowhere.o` build (xem Phần 0)|
|`disk/`, `fs/`|Bảng phân vùng (`part_dos.c`), FAT (`fs/fat/`), ext4 (`fs/ext4/`)|`[VENDOR]` `disk/part.c`, `fs/fat/fat.c` có sửa. `part_efi.c` không build vì `EFI_PARTITION` không bật|

**Pha 5: `main_loop` và lệnh**

|Thư mục|Vai trò|Vendor / config|
|---|---|---|
|`common/main.c`, `autoboot.c`, `cli_hush.c`, `cli_readline.c`, `command.c`, `console.c`|Vòng lặp lệnh, đếm ngược autoboot, parser hush, console mux|`[VENDOR]` `autoboot.c`: watchdog qua SCPI, `g_action`, kiểm tra `isWarpBootMode()`|
|`common/cmd_*.c` (≈ thư mục `cmd/` của upstream)|Mỗi file là một lệnh (`U_BOOT_CMD`)|`[CONFIG]` chỉ file có `CONFIG_CMD_*` bật mới build. Lệnh vendor: `cmd_warp.c`, `cmd_nvme.c`, `cmd_km-sound.c`|
|`common/mvebu/`|Lệnh Marvell: `cmd_misc.c` (default `y`), `cmd_mpp.c`...|`[CONFIG]` `cmd_mpp.o` phụ thuộc `MVEBU_MPP_BUS`, đang tắt ở target 800|

**Pha 6: nạp và khởi chạy kernel**

|Thư mục|Vai trò|
|---|---|
|`common/cmd_bootm.c`|`do_booti` (dòng 725). `CMD_BOOTM` default `y`, nên file này có mặt|
|`common/bootm.c`, `bootm_os.c`|Chuẩn bị boot; `[VENDOR]` `bootm.c` include `mfp_panel.h` (kiểm tra chế độ HW trước khi boot)|
|`common/image.c`, `image-fdt.c`, `fdt_support.c`|Xử lý header image và DTB, vá DTB; `image-fit.c` **không build** (`FIT` tắt)|
|`arch/arm/lib/bootm.c`, `bootm-fdt.c`|Bước cuối kiểu ARM: nhảy vào kernel, truyền FDT|
|`arch/arm/cpu/armv8/psci.S`|Vùng `.secure_text` phục vụ PSCI (`ARMV8_PSCI=y`)|
|`arch/arm/dts/`|DTB build ra: `armada-quartz-{emu800,toc,pd}.dts`, `apn-806-z1.dtsi`, `quartz-sb.dtsi`, `quartz-iris-sb-0.dtsi`. Nhúng vào u-boot **và** là DTB mà kernel có thể dùng|

### Nhóm D: thư mục có trong tree nhưng gần như không dùng

|Thư mục|Tình trạng|
|---|---|
|`net/` (25)|Nằm trong `libs-y`, nhưng mọi file gate bằng `CONFIG_CMD_NET` (tắt), nên rỗng|
|`api/` (9)|`libs-$(CONFIG_API)`, không bật, không build|
|`post/` (75)|`libs-$(CONFIG_HAS_POST)`, không bật|
|`test/` (38)|Nằm trong `libs-y`, nhưng file gate bằng `CONFIG_SANDBOX`/`CONFIG_DM`, thực tế rỗng|
|`examples/` (27)|Nằm trong `u-boot-dirs` để build ví dụ standalone, không đi vào `u-boot.bin`|
|`arch/` (2965)|14 arch không dùng (arc, avr32, blackfin, m68k, microblaze, mips, nds32, nios2, openrisc, powerpc, sandbox, sh, sparc, x86). Dùng `arm/` cho phần `armv8/`, `lib/`, `dts/`, `include/`. `armv8/fsl-lsch3/` và `armv8/armada3700/` **không build**|
|`board/` (3480)|Chỉ `board/mvebu/common/`, `board/mvebu/armada8k/` và `.../km/` (~50 file) được build. ~300 board còn lại là mã chết của upstream|
|`drivers/` (1047)|Phần lớn là driver upstream không bật. Danh sách driver thật ở nhóm C ở trên|

## 3. Vendor không nằm gọn trong một thư mục

Mã của Konica Minolta **rải trong các file upstream** bằng `#if defined(KM_BIZHUB)`, nên không thể chỉ đọc `board/.../km/` là đủ. Bản đồ các điểm chèn (từ 64 file đã đánh dấu):

|Vị trí|Điểm chèn|
|---|---|
|Khởi tạo sớm|`common/board_f.c` (chọn cổng debug, đệm log), `arch/arm/lib/crt0_64.S`, `arch/arm/lib/reset.c`|
|Khởi tạo muộn|`common/board_r.c` (`init_km`, `initr_bspi`), `board/mvebu/common/init.c`|
|Đường boot|`common/autoboot.c`, `common/bootm.c`, `common/console.c`, `common/stdio.c`, `common/command.c`|
|Driver|`mmc/mmc.c`, `pci/pci.c`, `i2c/designware_i2c.c`, `serial/ns16550.c`, `usb*.c`, `block/nvme.c`|
|Filesystem/partition|`fs/fat/fat.c`, `disk/part.c`|
|Device tree|`lib/fdtdec.c`, `common/fdt_support.c`, các `.dts` (`#define CONFIG_KM_BIZHUB` gõ cứng)|
|Header dùng chung|`include/{mmc,part,pci,usb_defs,fdtdec,autoboot}.h`, `arch/arm/include/asm/{system,global_data}.h`|

Mã mới hoàn toàn của KM (hoàn toàn không có ở upstream) gồm: Warp!!, NVMe, panel/LCD/boot animation, sound I2S, chẩn đoán HW, watchdog SCPI, và các vùng dữ liệu flash (`userdata`, `mtd_flash_access`).

## 4. Thứ tự đọc tôi đề xuất

1. `arch/arm/cpu/armv8/start.S` → `arch/arm/lib/crt0_64.S`
2. `common/board_f.c` (`init_sequence_f`, đặc biệt `init_baud_rate`)
3. `common/board_r.c` (`init_sequence_r`, `init_km`)
4. `board/mvebu/armada8k/quartz.c` → `km/machine_setup.c`
5. `common/main.c` → `autoboot.c` → `cmd_bootm.c`

Đây cũng là thứ tự tôi sẽ đi trong các phần sau.

## Điều chưa xác minh

- Hai file `board/mvebu/common/Makefile` và `arch/arm/cpu/mvebu-common/tools/Makefile` tôi chưa đọc, nên chưa liệt kê chính xác các `.o` của chúng.
- Chưa đọc thân hàm `dram_init` của armada8k, nên chưa biết DRAM size thực đến từ đâu (DT, thanh ghi hay `sys_info` do ATF truyền). Tôi sẽ mở nó ở Phần 2.
- Vì chưa có build thật, danh sách "file nào vào binary" ở trên là suy ra từ Makefile và Kconfig. Nếu bạn có `u-boot.map` hoặc `.config`, tôi có thể đối chiếu và sửa lại các chỗ sai.

Nếu ổn, tôi tiếp tục **Phần 2: `board_init_f` chi tiết**, cụ thể là `init_sequence_f` từng bước với call-flow, thay đổi `gd`, và phần cứng bị chạm tới.