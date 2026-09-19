# Phần 4: TPL / SPL

## Kết luận ngắn

| Câu hỏi                      | Trả lời (từ code)                                                                                                                                                                                                                                                                                       |
| ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Có TPL không?                | **Không, và không thể bật.** `CONFIG_TPL` yêu cầu `SUPPORT_TPL` ([Kconfig:95-97](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/Kconfig)), nhưng chỉ `mpc85xx` mới `select SUPPORT_TPL`. `TARGET_ARMADA_8K` không chọn nó [V]                         |
| Có SPL không?                | **Có mã, nhưng target thật không dùng.** `TARGET_ARMADA_8K` có `select SUPPORT_SPL` ([arch/arm/Kconfig:106-109](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/Kconfig)). Defconfig của `./build_uboot.sh 800` ghi `# CONFIG_SPL is not set` |
| SPL này có nạp U-Boot không? | **Không.** SPL của Armada 8K ở repo này là một **tiền khởi tạo trả quyền về BootROM**, không phải loader. Nó không có DDR init, storage, pinmux, PMIC (chi tiết mục 4)                                                                                                                                  |
| DDR init nằm đâu?            | **Ngoài U-Boot** (ATF/BootROM). Xem mục 5                                                                                                                                                                                                                                                               |

Nhãn: **[V]** đã đọc trong source, **[T]** tự tính/suy từ code, **[S]** suy luận, **[?]** chưa xác minh. Đường dẫn tính từ `BootROM/BootROM/Emu800/Src/`.

## 1. Defconfig nào bật SPL

|Defconfig|`CONFIG_SPL`|Ghi chú|
|---|---|---|
|`km_mvebu_quartz_toc` (target `800`), toàn bộ `egl*`, `emu800_*`, `spam*`, `spaas*`, `dnb*`, `hemlk`, `mssb`|**tắt**|Toàn bộ sản phẩm MFP [V]|
|`mvebu_quartz_toc`, `mvebu_quartz_pd`, `km_mvebu_quartz_pd`|**`=y`**|Cấu hình Marvell gốc và Palladium (giả lập). `build_uboot.sh` không gọi chúng [V]|

Vậy toàn bộ mã SPL bên dưới là mã **kế thừa từ Marvell**, không nằm trong firmware bạn giao.

## 2. Tại sao SPL tồn tại và giới hạn bộ nhớ

Lý do kiến trúc, đọc từ code (không phải lý thuyết chung):

- Ở giai đoạn đầu **DDR chưa sẵn sàng**, nên mã phải chạy từ **SRAM nội**. SPL có linker script riêng cố định vào SRAM.
- Vùng SRAM cấp cho SPL là cứng ([armada8k.h:66-77](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/configs/armada8k.h), [u-boot-armv8-spl.lds:19-27](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/mvebu-common/u-boot-armv8-spl.lds)):

|Hằng|Giá trị|
|---|---|
|`CONFIG_SPL_TEXT_BASE`|`0xFFE1C048`|
|`CONFIG_SPL_MAX_SIZE`|`0x27000` (156 KB)|
|Vùng `.sram`|`0xFFE1C048` – `0xFFE43048`|
|`SYS_SPL_MALLOC_START/SIZE`|`__end_of_spl` / `0x4000` (chỉ dùng bởi framework SPL chung, không được gọi ở đây)|

- Phần bù `0x48` sau `0xFFE1C000` là một header đứng trước code [S]. Tôi không thấy định nghĩa nào trong repo cho 0x48 byte này [?].
- **Giảm kích thước:** `Makefile.spl` thêm `-ffunction-sections -fdata-sections` và `--gc-sections` ([Makefile.spl:39-41](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/scripts/Makefile.spl)) để vừa 156 KB.
- **Không relocation:** SPL link đúng địa chỉ chạy (`. = CONFIG_SPL_TEXT_BASE`), không có `.rela.dyn`, không `-pie` cho SPL riêng. Comment trong README: "no relocation is done".
- Thư viện đưa vào SPL chỉ gồm những `CONFIG_SPL_*_SUPPORT` được bật. Header của A8K chỉ bật 5 mục ([mvebu-common.h:96-100](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/configs/mvebu-common.h)): `SPL_FRAMEWORK`, `LIBCOMMON`, `LIBGENERIC`, `SERIAL`, `DRIVERS_MISC`. **Không có** `SPL_MMC/SPI/NAND/USB/SATA/POWER/I2C` nào.

## 3. Cách SPL được build

|Bước|Nguồn|
|---|---|
|`ALL-$(CONFIG_SPL) += spl/u-boot-spl.bin`|[Makefile:721](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/Makefile)|
|`spl/u-boot-spl` chạy `make -f scripts/Makefile.spl`|[Makefile:1206-1209](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/Makefile)|
|Linker script|`CONFIG_SPL_LDSCRIPT = arch/arm/cpu/mvebu-common/u-boot-armv8-spl.lds`|
|Điểm vào|`head-y = arch/arm/cpu/armv8/start.o` (cùng `start.S`, nhánh `#if MVEBU && SPL_BUILD`)|
|`libs-y`|`board/mvebu/armada8k/`, `board/mvebu/common/`, `common/spl/` (nhờ `SPL_FRAMEWORK`), `common/`, `lib/`, `drivers/serial`, `drivers/misc`, `dts/` (`OF_EMBED`), thêm `drivers/phy` nếu `MVEBU_COMPHY_SUPPORT`, `drivers/pci` nếu DDR-over-PCI|

**Không có bước đóng gói** trong repo: `tools/doimage` **không tồn tại**, `CONFIG_MVEBU_DOIMAGE`/`CONFIG_DOIMAGE_TYPE` chỉ là macro trong header ([mvebu-common.h:183-241](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/configs/mvebu-common.h)) và `CONFIG_MVEBU_UBOOT_DFLT_NAME "flash-image.bin"` chỉ tên file. Việc ghép SPL/ATF/U-Boot vào `flash-image.bin` do công cụ ngoài làm [S].

## 4. Trace SPL: chỉ có gì, và không có gì

### 4.1 Điểm vào (`start.S`, nhánh SPL)

```
_start   (SPL)                                   start.S:23-40
  stp x19..x30 → stack        ← 6 cặp, dùng stack CỦA NGƯỜI GỌI (BootROM)
  bl lowlevel_init_spl        ← cntfrq_el0 = COUNTER_FREQUENCY (25 MHz)
  bl board_init_f             ← arch/arm/cpu/armv8/armada8k/spl.c
  ldp x29..x19 ; ret          ← TRẢ VỀ người gọi (BootROM), không nhảy sang U-Boot
```

|Yêu cầu bạn hỏi|Thực tế trong SPL này [V]|
|---|---|
|**Stack**|**Không thiết lập.** Không có `SP` mới, dùng nguyên stack của BootROM và chỉ push 12 thanh ghi|
|**`gd`**|`gd = &gdata` với `gdata` là biến `.data` tĩnh ([arch/arm/lib/spl.c:18](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/lib/spl.c)), **không** đặt bằng `_main`|
|**BSS**|**Không bị xoá** (`board_init_f` của mvebu ghi đè hàm mặc định vốn có `memset(__bss_start...)`). Biến `.bss` là giá trị của SRAM lúc vào|
|**Cache/MMU**|Không đụng tới (không có `dcache_enable`)|
|**Clock**|Chỉ `cntfrq_el0`. Không có cấu hình PLL/clock|
|**Pinmux**|**Không có** trong SPL. `mvebu_pinctl_probe`/`mpp_bus_probe` chỉ ở U-Boot proper (và đều tắt ở defconfig `800`)|
|**PMIC**|**Không có** (`SPL_POWER_SUPPORT` không bật)|
|**Storage**|**Không có** trình đọc storage nào|
|**DDR init**|**Không có** (xem mục 5)|
|**Nạp U-Boot**|**Không.** Không thấy `jump_to_image_no_args` được gọi|

### 4.2 `board_init_f` của A8K ([spl.c:31-81](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/armada8k/spl.c)), theo thứ tự

1. `gd = &gdata; gd->baudrate = CONFIG_BAUDRATE`; nếu tham số `silent` khác 0 thì `GD_FLG_SILENT`.
2. **`setup_fdt()`** ([mvebu-common/fdt.c:24](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/mvebu-common/fdt.c)): `gd->fdt_blob = __dtb_dt_begin`. **Chỉ hỗ trợ `OF_EMBED`** (`#error` nếu khác).
3. `cfg_eeprom_init()` chỉ với `BOARD_CONFIG_EEPROM`; `mvebu_setup_fdt()` chỉ với `MULTI_DT_FILE` (cả hai tắt ở defconfig quartz).
4. **`preloader_console_init()`** ([common/spl/spl.c:264](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/spl/spl.c)): `gd->bd = &bdata`, `serial_init()`, `have_console=1`, in banner `U-Boot SPL ...`.
5. `mvebu_sar_init(fdt)` (khi `MVEBU_CHIP_SAR`, **`default y`**): đọc thanh ghi sample-at-reset qua DT (`reg = <0x6F8200 0x8>`), có các field như `CLOCK_FREQ_MODE`, `BOOT_SOURCE`, `PIDI_HW_TRAINING_DIS`, `XTAL_BYPASS`... ([ap806_sar.c:36-65](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/drivers/misc/mvebu_sar/ap806_sar.c)). Chỉ **đọc** để sau hỏi lại, không lập trình clock.
6. `dram_over_pci_init(fdt)` chỉ khi `MVEBU_SPL_DDR_OVER_PCI_SUPPORT`.
7. `init_flc()` chỉ khi `MVEBU_FLC`.
8. `run_memory_test()` chỉ khi `MVEBU_SPL_MEMORY_TEST`.

Các mục 6-8 **không được bật** trong bất kỳ defconfig quartz nào. Tôi đã grep `DDR_OVER_PCI`, `MEMORY_TEST`, `MVEBU_FLC` trong `configs/*quartz*`, không thấy [V]. Riêng `MVEBU_SPL_DDR_OVER_PCI_SUPPORT` **không có khai báo Kconfig nào**, chỉ định nghĩa được qua header/defconfig [V].

### 4.3 Những nhánh code có nhưng SPL này không chạy

- `common/spl/spl.c:board_init_r()` (framework SPL chung: `spl_boot_device`, `spl_mmc_load_image`, `jump_to_image_no_args`...) **được biên dịch** vì `SPL_FRAMEWORK` bật, nhưng **không ai gọi** nó: `start.S` gọi `board_init_f` rồi `ret`, `board_init_f` của A8K không gọi `board_init_r`. Các `case BOOT_DEVICE_*` cũng bị ngắt bởi `#ifdef CONFIG_SPL_*_SUPPORT` chưa bật, nên nếu gọi thì rơi vào `default: hang()`.
- `mvebu_is_in_recovery_mode()` (soc.c:341, nhánh `SPL_BUILD`): đọc thanh ghi MPP xem chân UART RX có bật không (chế độ **recovery boot qua UART**) rồi `set_info(RECOVERY_MODE, ...)`. Trong repo **không có caller** [V], nên hiện chỉ là hàm sẵn có cho code ngoài.

## 5. DDR init: nằm đâu

Bằng chứng, xếp theo độ chắc chắn:

|Bằng chứng|Nguồn|Nhãn|
|---|---|---|
|SPL không gọi mã DDR nào; các hàm DDR duy nhất là DDR-over-PCI (giả lập), FLC, memory test|[spl.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/armada8k/spl.c)|[V]|
|Không có thư mục driver DDR của Marvell (`mv_ddr`) trong repo; `drivers/ddr/` chỉ có `fsl/` của NXP, không liên quan|glob|[V]|
|`dram_init()` chỉ `gd->ram_size = QUARTZ_RAM_SIZE` (2 GB)|[soc.c:145](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/armada8k/soc.c)|[V]|
|`dram_init_banksize()` **đọc** thanh ghi memory controller (`0xF0020200...`) để biết DDR đã được cấu hình|[soc.c:189](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/armada8k/soc.c)|[V]|
|`armada8k.h` dành vùng "DDR training" `0x10400000` (1 MB) ngay sau ATF|armada8k.h:110|[V]|
|Comment "End of 16M scrubbed by training in bootrom"|mvebu-common.h:71|[V]|
|`dram_over_pci.c`: "disable DDR window opened by **BootROM**"|dram_over_pci.c:83|[V]|
|DDR init thực tế do ATF (BL2) làm|không có mã ATF|**[S]**|

**Kết luận:** DDR (kể cả LPDDR4 training) đã được cấu hình **trước khi** U-Boot chạy, U-Boot chỉ **đọc lại** cấu hình. Mã chịu trách nhiệm không nằm trong repo này.

**Một điểm chưa giải thích:** comment ở [mvebu-common.h:260](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/configs/mvebu-common.h) nói KM đổi `PRINT_BUFFER_NUM` từ 50 lên 255 vì "LPDDR4 パラメータログが出ない" (log tham số LPDDR4 không hiện ra). Tôi không tìm thấy code U-Boot nào in tham số LPDDR4, nên nguồn log đó [?] (có thể là chuỗi từ một tầng trước, hoặc do người viết chú thích lẫn).

### DDR-over-PCI, FLC, memory test (chỉ Marvell/giả lập, không dùng ở đây)

- **DDR-over-PCI** ([dram_over_pci.c:75-96](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/mvebu-common/dram_over_pci.c)): tắt cửa sổ CCU số 2 mà BootROM mở cho DDR, bật cửa sổ RFU-PEX với cấu hình `0x50000000`, để **coi DRAM nằm sau một thiết bị PCIe** (bo giả lập/FPGA). Kích thước cửa sổ `0x80000000`.
- **FLC** ([mvebu_flc.c:183](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/armada8k/mvebu_flc.c)): Final Level Cache của McKinley MC; đọc `flc_ext_dev_map`/`flc_nc_map` từ DT và ghi vào `MMAP_FLC*`.
- **Memory test** (`ddr_test.c`): quét `0x100000`-`0xB00000` mặc định, 2 lượt.

## 6. Định dạng image và handoff

### 6.1 Định dạng image BootROM của A8K

[cmd_bubt.c:38-61](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/mvebu/cmd_bubt.c) mô tả header chính mà BootROM đọc:

|Offset|Field|Ý nghĩa|
|---|---|---|
|0|`magic`|`0xB105B002` (`MAIN_HDR_MAGIC`)|
|4|`prolog_size`|kích thước header + phần mở rộng|
|8|`prolog_checksum`|tổng 32-bit toàn bộ prolog (trường này tính là 0)|
|12|`boot_image_size`|kích thước image kèm theo|
|16|`boot_image_checksum`|checksum image|
|24|`load_addr`|nơi BootROM nạp image|
|28|`exec_addr`|nơi BootROM nhảy tới|
|32-35|`uart_cfg`, `baudrate`, `ext_count`, `aux_flags`|cấu hình UART và mở rộng|
|36-51|`io_arg_0..3`|tham số thiết bị boot|

`check_image_header()` chỉ kiểm magic và checksum ([cmd_bubt.c:466-496](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/mvebu/cmd_bubt.c)). **Lưu ý:** `bubt` (`CONFIG_CMD_MVEBU_BUBT`, `default n`, không bật ở defconfig nào) **không được biên dịch**, nên chỉ để tham khảo định dạng. Cũng không có `SHA`/chữ ký nào trong header này, và không thấy xác thực khoá công khai phía U-Boot.

### 6.2 Handoff giữa các tầng

|Kênh|Chi tiết|Nhãn|
|---|---|---|
|**Quyền thực thi**|SPL `ret` về người gọi (BootROM) chứ không nhảy vào U-Boot|[V]|
|**Dữ liệu `sys_info`**|Bảng `{field_id, value}` tại **`0x04000000`** ([system_info.h:22](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/include/asm/arch-mvebu/system_info.h)). SPL ghi bằng `set_info()`: phần tử 0 là `ARRAY_SIZE` (chứa số phần tử), các phần tử sau là `DRAM_CS*_SIZE`, `DRAM_BUS_WIDTH`, `DRAM_ECC`, `RECOVERY_MODE`, `BOOT_MODE`... U-Boot proper chép vào `gd->arch.local_sys_info[]` ở `sys_info_init()` rồi đọc bằng `get_info()`|[V]|
|Ai ghi `sys_info`|Trong repo chỉ có `mvebu_is_in_recovery_mode` (chưa ai gọi). Các trường DRAM do tầng khác ghi cùng bố cục|**[S]**|
|**Nơi U-Boot proper nằm**|`0x1000` (`TEXT_BASE`) do BootROM/ATF nạp|[S]|

**Ở target thật (không SPL)** kênh này gần như không dùng: `dram_init` bỏ qua `get_info(DRAM_CS*)` (dùng `QUARTZ_RAM_SIZE`), và `print_soc_specific_info` chỉ dùng `DRAM_BUS_WIDTH` khi **không** phải `KM_BIZHUB` (KM đọc thanh ghi `MVEBU_MC_CTRL_0` trực tiếp).

## 7. Sơ đồ

### 7.1 Sản phẩm thật (`./build_uboot.sh 800`, không SPL)

```
Boot ROM (silicon)
   │  đọc main header (0xB105B002), nạp và chạy boot image
   ▼
ATF: BL1 → BL2 → BL31                    ← ngoài repo [S]
   │   ├─ DDR init/training (LPDDR4)     ← ngoài repo [S]
   │   └─ PSCI, SCPI, watchdog
   ▼
BL33 = u-boot.bin  @ 0x1000  (TEXT_BASE)  ← repo này bắt đầu ở đây [V]
   ▼
start.S → _main → board_init_f → relocate → board_init_r → main_loop → booti
```

### 7.2 Cấu hình có SPL (`mvebu_quartz_*`, `km_mvebu_quartz_pd`)

```
Boot ROM
   │  đọc main header, chép "BIN header" (SPL) vào SRAM 0xFFE1C048
   ▼
SPL @ SRAM  (156 KB, chạy trên stack của BootROM)        [V] phía SPL
   │  start.S: lưu x19-x30 → cntfrq → board_init_f:
   │    setup_fdt → console → SAR → [DDR-over-PCI | FLC | memtest]
   │  ret  ──────────────────────────────►  Boot ROM
   ▼
Boot ROM (tiếp tục)  ← [S] hành vi BootROM, không có mã trong repo
   │  nạp boot image (U-Boot) vào DRAM, nhảy tới exec_addr
   ▼
U-Boot proper @ 0x1000 → như 7.1
```

TPL không xuất hiện trong cả hai luồng.

## 8. Rủi ro và điểm bất thường (chỉ áp dụng nếu bạn thật sự build SPL)

1. **`gdata` có khả năng tràn SRAM.** `gd_t gdata` nằm ở `.data` của SPL (`arch/arm/lib/spl.c:18`), mà `gd_t` của KM chứa `printBuffer[255][1051]` ≈ **268 KB**. Vùng `.sram` chỉ **159.744 byte** (0x27000). Nếu `KM_BIZHUB` được định nghĩa khi build SPL (mặc định `y`, và `config.mk` truyền `-DKM_BIZHUB` cho cả SPL), thì SPL **sẽ không link được** (`region .sram overflowed`) [T, chưa build]. Nhiều khả năng đây là lý do mọi defconfig KM đều tắt SPL. Nhận xét thêm: `PRINT_BUFFER_NUM` từng là 50 (≈52 KB, còn vừa) cho đến khi đổi sang 255 năm 2019, nên tôi đoán SPL từng build được trước đó [S].
2. **SPL không in được log** trong build KM: `BspPrintValid()` có nhánh `#if defined(CONFIG_SPL_BUILD) return 0;` ([console.c:314-316](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/console.c)), nên mọi `printf` của SPL chỉ bị đệm, banner `U-Boot SPL` không hiện. Điều này cho thấy KM có quan tâm đến build SPL.
3. **`x18` bị ghi đè khi trả về.** `_start` (SPL) lưu `x19-x30` nhưng `board_init_f` gán `gd` (thanh ghi **x18**), và không khôi phục x18. Có gây lỗi hay không phụ thuộc quy ước gọi của BootROM [?].
4. **BSS không xoá:** biến `static` khởi tạo 0 trong SPL có giá trị rác trừ khi BootROM đã xoá SRAM [?].
5. **`SYS_SPL_MALLOC_START = __end_of_spl`** nằm sát cuối vùng `.sram`, nếu có dùng heap 16 KB thì vượt `CONFIG_SPL_MAX_SIZE`. Hiện không được dùng vì `board_init_r` không chạy.

## 9. Điều tôi chưa xác minh

- **Hành vi BootROM** (nạp BIN header vào SRAM, gọi SPL rồi tiếp tục nạp U-Boot, EL lúc gọi SPL): không có mã, suy từ code phía SPL và comment.
- **Ý nghĩa `0x48`** sau `0xFFE1C000` và vị trí SRAM thật của SoC: chỉ có hằng số.
- **Việc `gdata` tràn SRAM**: cần build `mvebu_quartz_toc_defconfig` để xác nhận (và xem `spl/include/autoconf.mk` xem `KM_BIZHUB` có bật không).
- **Nguồn log LPDDR4** nói ở mục 5.
- **Ai ghi các trường DRAM của `sys_info`** ở luồng ATF.
- Chưa đọc `mvebu-common/misc.c`, `mbus.c` và các thanh ghi `clock.c` của A8K để xem có bước clock nào chạy trong SPL không. Nhưng `board_init_f` của SPL không gọi hàm nào như vậy, nên khả năng có là rất thấp.

Bước hợp lý tiếp theo là **Phần 5: driver init trong `board_init_r`**, bắt đầu bằng trục lưu trữ (Xenon MMC, QSPI, NVMe/PCIe) vì `fatload` và `read_rom` phụ thuộc trực tiếp vào chúng. Hoặc, nếu bạn thật sự muốn dùng SPL, tôi có thể phân tích cụ thể vì sao `gdata` tràn và cách rút gọn.