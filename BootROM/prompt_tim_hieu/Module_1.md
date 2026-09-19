# Phần 0: Nhận diện repository

Tôi chưa build được repo này (không có `.config`, `autoconf.mk`, toolchain `s800-linux-`). Cấu hình hiệu lực dưới đây là suy ra từ defconfig, Kconfig và header. Mỗi mục có nhãn: **[Đã xác minh]** (thấy trực tiếp trong source), **[Suy luận]** (từ bằng chứng gián tiếp) hoặc **[Chưa xác minh]**.

Đường dẫn dưới đây tính từ `BootROM/BootROM/Emu800/Src/`.

## 1. Repository profile

|Mục|Kết luận|Bằng chứng|
|---|---|---|
|Phiên bản|**U-Boot 2015.01**, hậu tố `-devel-16.07.2` [Đã xác minh]|[Makefile](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/Makefile) (`VERSION=2015`, `PATCHLEVEL=01`), [localversion](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/localversion)|
|Commit|**Không xác định được**|[snapshot.commit](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/snapshot.commit) còn nguyên placeholder `$Format:%H %cD$` (bản `git archive`), và thư mục không phải git repo|
|Bản chất|Fork nhiều lớp: U-Boot 2015.01 → lớp **Marvell MVEBU** → lớp **Konica Minolta (bizhub MFP)**|Copyright Marvell 2015–2017 ở `board/mvebu`, `arch/arm/cpu/armv8/armada8k`; menu "KonicaMinolta customize" trong [Kconfig](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/Kconfig)|
|Quy mô|11.015 file, gồm 3.341 `.c`, 3.202 `.h`, 88 `.dts`|Đếm bằng script|
|Phần thực sự chạy|Rất nhỏ. Chỉ nhánh `arm64 → armada8k → mvebu → km` là build. Gần 300 board và 14 kiến trúc còn lại là mã chết của upstream|Xem mục 4|
|Phần vendor|Khoảng **64 file** mang dấu KM (`KM_BIZHUB`, "KonicaMinolta Modificate History")|Grep. Tập trung ở `board/mvebu/armada8k/km/`, `common/{board_f,board_r,autoboot,bootm,...}.c`, `drivers/{block/nvme,mmc,pci,video,scpi,...}`|
|Công tắc vendor|`CONFIG_KM_BIZHUB` và `CONFIG_KM_EXTENDED` đều `default y`|[Kconfig](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/Kconfig) dòng 166–172. Khi bật, [config.mk](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/config.mk) thêm `-DKM_BIZHUB` và `-I board/mvebu/armada8k/km`; [Makefile](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/Makefile) dòng 650 thêm thư mục `km/` vào `libs-y`|
|File sửa muộn nhất|[mfp_panel.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/board/mvebu/armada8k/km/mfp_panel.c), mtime 2026-08-23, muộn hơn phần còn lại|Có thể đã bị sửa cục bộ sau bản snapshot. Không có diff nên tôi không kết luận được|

**Các biến thể sản phẩm.** [build_uboot.sh](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/build_uboot.sh) chọn defconfig theo tham số. Mỗi biến thể chỉ khác nhau ở cờ `KM_MACHINE_*`, `KM_BOARD_EMU800` và `HWCHECK_MEMSIZE`.

|Tham số|defconfig|Máy|
|---|---|---|
|`800`|`km_mvebu_quartz_toc_defconfig`|"Quartz" (S800)|
|`emuegl`, `emueglz`, `emudnbmlk`, `emuspam`, `emuspaas`|`emu800_*`|Máy chạy trên nền Emu800|
|`egl`, `eglz`, `dnbmlk`, `spam`, `spaas`, `eglb`, `eglbz`, `eglzp`, `eglbzp`, `spamb`, `spaasb`, `hemlk`, `mssb`|tương ứng|Eagle, Deneb, Sparrow, Helios, Minerva và các bản BK|

Mỗi máy ánh xạ tới một thư mục firmware (`FW_DIRECTRY`, ví dụ `FW0022` cho Sparrow MFP, `FW9022` cho bản Emu800) trong [armada8k.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/configs/armada8k.h) dòng 160–208.

## 2. Hardware profile

|Mục|Kết luận|
|---|---|
|Kiến trúc|**ARM64 / AArch64**. `CONFIG_SYS_EXTRA_OPTIONS="ARM64"`, `CONFIG_ARM=y`, dùng `arch/arm/cpu/armv8` [Đã xác minh]|
|SoC|**Marvell Armada 8K, AP806 ("APN-806")**. `CONFIG_TARGET_ARMADA_8K=y`, DT `apn-806-z1.dtsi`, `CONFIG_ARMADA_8K_SOC_ID 8022` [Đã xác minh]|
|CPU|**Không nhất quán trong repo.** DT ghi `arm,cortex-a57` ([apn-806-z1.dtsi:30](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/dts/apn-806-z1.dtsi)). Mã vendor lại gọi là CA72 (`ca72wdt.c`, `km_scpi_set_wdt_ca72_ext`). Tôi chưa xác định được lõi thật. Kiến thức ngoài repo nghiêng về A72.|
|Southbridge|**"Quartz"** (`quartz-sb.dtsi`) gồm SDHCI Xenon `e8278000`, UART DW `e8000000`, I2C DW `e8006000/6800/7000/7800`, QSPI `e8273000`. **"Iris"** (`quartz-iris-sb-0.dtsi`) gồm USB3 xHCI `d00d0000` và PCIe [Đã xác minh]|
|Board|Family `CONFIG_QUARTZ`. Runtime tự nhận **TOC / ESEVAL / EMU800** trong `get_quartz_board()` ([quartz.c:118](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/board/mvebu/armada8k/quartz.c)): có EEPROM ở I2C bus 3 addr `0x50` thì là TOC, ngược lại đọc GPIO 271 (FW_CONFIG[5]) [Đã xác minh]|
|Vendor|Marvell (SoC) + Konica Minolta (bizhub MFP: Eagle, Sparrow, Deneb, Helios, Minerva)|
|GIC / Timer|GICv2. Generic timer, `COUNTER_FREQUENCY` 25 MHz ([armada8k.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/configs/armada8k.h))|
|RAM|Bản đồ địa chỉ (`memory-map.h`): DRAM từ `0x0`, IO `0x40000000`, PEX `0x60000000`. `NR_DRAM_BANKS=3`, U-Boot giới hạn dùng tối đa **3 GB**. Dung lượng kiểm tra lúc chạy (`HWCHECK_MEMSIZE`): mặc định 8 GB, Sparrow MFP 5 GB (bản 6 GB dùng `HWCHECK_MEMSIZE_6GB`). Comment trong `mvebu-common.h` nhắc **LPDDR4** [Đã xác minh]|
|Khởi tạo DDR|**Không nằm trong U-Boot này** [Suy luận]. Target chính không có SPL, còn vùng "DDR training" `0x10400000` được định nghĩa cạnh ATF|
|Serial|NS16550 memory-mapped 32-bit, 115200 baud. Ba cổng: COM1 = UART AP `0xf0512000` (kernel `earlycon`), COM2 = **UART panel** `0xe8002800` (57600 baud), COM3 = CSRC `0xe8003000`. Chọn cổng debug bằng GPIO FW_CONFIG4 trong `init_baud_rate()` ([board_f.c:141](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/board_f.c)) [Đã xác minh]|
|Lưu trữ|eMMC/SD (`XENON_MMC`), NOR QSPI (`MRVL_QSPI`, driver `qspi.c` và `qspi_winbond.c`), **NVMe qua PCIe** (`KM_NVME`, `PCIE_MV_MSB0125`, `DW_PCIE`), USB mass storage (xHCI)|
|Mạng|Defconfig không bật `CMD_NET`. Có E1000 trên PCIe chỉ khi `DEVEL_BOARD`. Tôi chưa kiểm tra default trong Kconfig, nên coi là **không có mạng** [Chưa xác minh hoàn toàn]|
|Màn hình / âm thanh|LCD/LVDS (`AP_LCD2_BASE 0xC057E000`, tối đa 1280×768, 16 bpp), UART panel, animation boot, I2S phát âm khi bật nguồn (`KM_I2S_CDMA`, gọi trong `board_init()`)|
|Watchdog|Watchdog CA72 điều khiển qua **SCPI** tới ATF (`km_scpi_set_wdt_ca72_ext` trong `autoboot.c`)|
|Secure boot|Xem mục dưới|

**Secure boot [Đã xác minh, kết luận thận trọng]:**

- Trong U-Boot chỉ có **kiểm tra toàn vẹn SHA-256**. `fsload_hash_check()` ([machine_setup.c:568](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/board/mvebu/armada8k/km/machine_setup.c)) băm image nạp từ FAT rồi so với file INDEX. Không thấy xác thực chữ ký.
- `CONFIG_FIT` và `FIT_SIGNATURE` không nằm trong defconfig nào.
- [tools/secure/](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/tools/secure) chứa cấu hình ký Marvell (`sec_img.cfg`: KAK/CSK, AES, `jtag.enable=true`, `box_id=0xdeadbeef`). `build_uboot.sh` chỉ `cp u-boot.bin`, nên bước ký **không nằm trong build này**.
- Thư mục đó có **5 file `.key` private** nằm trong source tree (`kak_priv_pem.key`, `csk_priv_pem0..3.key`). Tôi không mở chúng. Nếu là khoá thật thì đây là rủi ro bảo mật, đáng xử lý riêng.

## 3. Software architecture profile

**Chuỗi boot [Suy luận, chưa có nguồn ATF trong repo]:**

```
BootROM → ATF (BL1/BL2/BL31 @0x10000000, 4MB, PSCI) → U-Boot (BL33 @0x1000)
        → relocate → autoboot → booti Image + DTB (+ initrd) → Linux
```

Bằng chứng gián tiếp:

- `armada8k.h` có "Keep this in sync with ATF" cùng `PLAT_MARVELL_ATF_BASE=0x10000000` và `TRUSTED_ROM_SIZE=4M`.
- `ARMV8_PSCI=y`.
- `arm_scpi.c` ghi rõ các define phải khớp với ATF `plat/marvell/a8k/quartz`.
- `SCPI_MAILBOX_BASE=0x7ff00000`.

|Thành phần|Trạng thái|Chi tiết|
|---|---|---|
|**SPL / TPL**|**Không có** ở target `800`|`# CONFIG_SPL is not set` trong `km_mvebu_quartz_toc_defconfig`. Chỉ các defconfig phụ (`km_mvebu_quartz_pd`, `mvebu_quartz_toc`) đặt `CONFIG_SPL=y`, và `build_uboot.sh` không dùng chúng. TPL không có ở đâu.|
|**Device tree**|Dùng, nhúng vào binary|`OF_CONTROL=y` + `OF_EMBED=y`. `DEFAULT_DEVICE_TREE="armada-quartz-emu800"`. `MULTI_DT_FILE` tắt. `dtb-$(CONFIG_QUARTZ)` build cả `emu800.dtb` và `toc.dtb`.|
|**Driver model**|Chủ yếu **legacy**|`CONFIG_DM` không thấy trong defconfig. I2C dùng `CONFIG_SYS_I2C`. Chưa kiểm tra sâu.|
|**FIT**|Không dùng|Kernel là ARM64 `Image` nạp bằng `booti`.|
|**Environment**|**Không persistent, chỉ nằm trong RAM** [Suy luận từ header]|`mvebu-common.h:190` chọn `CONFIG_ENV_IS_NOWHERE` khi không có `*_BOOT`. Các lựa chọn `MVEBU_SPI_BOOT`... phụ thuộc `MVEBU_SPI` (đang tắt, chỉ bật `MRVL_QSPI`) nên không hiện ra. Vì vậy env = default compile-time (`CONFIG_BOOTCOMMAND`, `EXTRA_ENV_SETTINGS`) + biến set lúc chạy (`dtb_name` trong `quartz_late_init()`). `saveenv` được bật nhưng không có backend, cần kiểm tra hành vi.|
|**Trạng thái lưu lâu dài**|Cơ chế riêng của KM|`userdata.c` có bản đồ SPI-flash cho backup NVRAM, SoftDipSW, dữ liệu BSP, vùng HW check. `km_linecard_boot_setting_read/write` trong `machine_setup.c`.|
|**Filesystem**|FAT (đường KM chính, `fatload`), ext2/ext4 (đường mặc định `ext2load`)|Partition DOS. EFI partition chỉ nằm trong nhánh SCSI/SATA đang không bật.|
|**Layout partition** (suy ra từ macro bootcmd)|`0:5` boot thường, `0:2` IISW/ERR, `0:A` IISW update, `0:3` và `0:10` chứa bootflag Warp!!, USB dùng `FW00xx/`|`armada8k.h:267-273` và `WARP_SAVEAREA`. Bảng partition thật của thiết bị chưa xác minh.|

**Subsystem tùy biến chính, đều là [VENDOR]:**

|Subsystem|Vị trí|Việc nó làm|
|---|---|---|
|Warp!! (resume kiểu hibernate)|`common/cmd_warp.c`, `include/warp.h`, `WARP_*` trong `armada8k.h`|Khôi phục ảnh hibernate từ SD/NVMe/SPI (offset `0x2b0000`), bỏ qua boot thường|
|Machine setup / HW check|`km/machine_setup.c`, `bootm.c`|Bật nguồn, `misc_init_r`, `boot_diag`, kiểm tra hash, kiểm tra dung lượng RAM|
|Panel UI|`km/mfp_panel.c`, `picture.c`, `graphlib.c`, `pngfilter.c`, `drivers/video/lxk_*`|LCD, UART panel, animation|
|NVMe|`drivers/block/nvme.c`, `common/cmd_nvme.c`|Boot từ SSD|
|Watchdog / SCPI|`km/ca72wdt.c`, `drivers/scpi/arm_scpi.c`|Refresh watchdog quanh autoboot|
|Flash / DIP|`km/mtd_flash_access.c`, `dipvalue.c`, `userdata.c`|Truy cập SPI-flash, DIP mềm, dữ liệu người dùng|
|Lệnh `quartz`|`quartz.c`|`bspi_boot`, `mmc_raw_boot`, `mmc_fs`, `limit_mem`|
|Console đệm log|`board_f.c`, `PRINT_BUFFER_NUM=255`|Giữ log đến khi cổng debug sẵn sàng (`printStopFlg`)|

## 4. Build configuration profile

**Quy trình build** ([build_uboot.sh](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/build_uboot.sh)): `CROSS_COMPILE=s800-linux-`, sau đó `make mrproper`, `make <defconfig>`, `make`, rồi `cp u-boot.bin output/`. Lưu ý `make <defconfig>` chỉ chạy nếu chưa có `.config` (`makeConfig`), nên đổi target mà không `mrproper` sẽ dùng `.config` cũ.

**Chuỗi cấu hình gộp** (thứ tự include): [armada8k.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/configs/armada8k.h) → `mvebu-common.h` → `quartz-sb.h`. Sau đó `armada8k.h` `#undef` và ghi đè một số giá trị của `mvebu-common.h` (tên file `flash-image.bin`, `BOOTDELAY`, `BOOTARGS`).

**Ba defconfig chính:**

|CONFIG|`800` (toc)|`emu800_spam` / `spam`|`km_..._pd`|
|---|---|---|---|
|`SPL`|tắt|tắt|**bật**|
|`QUARTZ`|y|y|(không đặt)|
|Device tree|`armada-quartz-emu800`|`armada-quartz-emu800`|`armada-quartz-pd`|
|`KM_MACHINE_*`|(không có)|`SPAM`|(không có)|
|`KM_BOARD_EMU800`|(không có)|chỉ bản `emu800_spam`|(không có)|
|`HWCHECK_MEMSIZE`|mặc định 8 GB|5 GB|8 GB|
|Tính năng chung|`XENON_MMC`, `MRVL_QSPI`, `MVQZ_GPIO`, xHCI USB, `KM_NVME`, `DW_PCIE`, `KM_I2S_CDMA`|cùng bộ|thiếu NVMe, PCIe|

**Cờ biên dịch và link:**

- Cờ compiler: `-march=armv8-a -mstrict-align -fno-common -ffixed-x18` ([arch/arm/cpu/armv8/config.mk](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/config.mk)). Thanh ghi x18 bị chiếm làm con trỏ `gd`.
- Linker script: [u-boot.lds](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/u-boot.lds), `. = 0`. `SYS_TEXT_BASE` là `0x1000` (`mvebu-common.h:55`). Có `.rela.dyn` để relocate lúc chạy. Có vùng `.secure_text` cho PSCI (`ARMV8_PSCI=y`).
- Bộ nhớ: `MALLOC_LEN` 5 MB, `SYS_INIT_SP_ADDR = TEXT_BASE + 0xFF0000`, `SYS_LOAD_ADDR 0x02000000`, `BOOTM_LEN` 20 MB.
- `+S:` trong defconfig nghĩa là dòng đó áp dụng cho cả image thường và SPL (`doc/README.kconfig`).

**Bootcmd mặc định** (đường không-KM, `armada8k.h:130`): `ext2load mmc 0:1 $fdt_addr $dtb_name; ext2load mmc 0:1 $kernel_addr Image; booti $kernel_addr - $fdt_addr` với `kernel_addr=0x800000`, `fdt_addr=0x1000`. Với `KM_BIZHUB`, lệnh boot thật được ghép từ macro `CMD_INITRD_ENV(...)` (fatload trên `0:5`, có initrd). `BOOTDELAY=3`.

## Điểm bất thường cần lưu ý

1. **Tên gây nhầm.** Target `800` dùng `km_mvebu_quartz_toc_defconfig` nhưng đặt `DEFAULT_DEVICE_TREE="armada-quartz-emu800"`. DT đó có `model = "Marvell Quartz Palladium"`, và `#define CONFIG_KM_BIZHUB` được **gõ cứng** trong file `.dts`, không đến từ Kconfig.
2. **DT nói A57, mã vendor nói CA72.** Cần xác định trước khi phân tích cache và watchdog.
3. **`saveenv` không có backend** (ENV_IS_NOWHERE). Cần đọc `env_nowhere.c` để biết lệnh báo lỗi hay im lặng.
4. **Comment tiếng Nhật** trong các file KM dùng mã CP932/Shift-JIS nên hiện thành ký tự rác. Nội dung mã không bị ảnh hưởng, nhưng cần mở bằng encoding đúng.
5. **Khoá ký private nằm trong tree** (đã nêu ở mục 2).

## Bước tiếp theo

Phần 0 đã đủ nền để đi tiếp. Tôi đề xuất **Phần 1: luồng boot đầy đủ** theo trình tự sau, mỗi bước có call-flow và nhãn `[UPSTREAM]/[VENDOR]/[BOARD]/[DRIVER]/[CONFIG]`:

1. `start.S` (armv8)
2. `crt0_64.S`
3. `board_init_f` (`init_sequence_f`, gồm phần KM chèn vào)
4. relocate
5. `board_init_r`
6. `main_loop`
7. `bootdelay_process` / autoboot với watchdog và `g_action`
8. `bootm` / `booti`

Muốn đi tiếp thì bảo tôi "tiếp tục Phần 1". Nếu bạn có thể cung cấp `.config` hoặc `autoconf.mk` đã build, hoặc xác nhận lõi CPU thật, các mục **[Suy luận]** ở trên sẽ được kiểm chứng.