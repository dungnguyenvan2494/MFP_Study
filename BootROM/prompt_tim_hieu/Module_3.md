# Phần 2: Boot flow đầy đủ, từ reset đến bàn giao kernel

Tôi đã đọc thân từng hàm trong chuỗi `start.S` → `crt0_64.S` → `board_init_f` → `relocate_code` → `board_init_r` → `main_loop` → `booti`. Đường dẫn dưới đây tính từ `BootROM/BootROM/Emu800/Src/`. Nhãn: **[V]** đã đọc trong source, **[S]** suy luận từ bằng chứng gián tiếp, **[?]** chưa xác minh.

## 0. Đính chính các phần trước

Khi đọc sâu tôi thấy bốn điểm trong Phần 0 và Phần 1 chưa chính xác:

|Điểm|Đã nói|Thực tế [V]|
|---|---|---|
|`saveenv`|"Bật nhưng không có backend"|**Lệnh không tồn tại.** [cmd_nvedit.c:688](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/cmd_nvedit.c) chỉ biên dịch `saveenv` khi `!CONFIG_ENV_IS_NOWHERE`. Trên board này gõ `saveenv` ra "Unknown command"|
|DRAM|"Chưa biết size từ đâu"|`dram_init()` **gán cứng** `gd->ram_size = CONFIG_QUARTZ_RAM_SIZE` = **2 GB** ([soc.c:145](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/armada8k/soc.c)). Kích thước thật đọc từ thanh ghi memory controller trong `dram_init_banksize()` (xem 5.6)|
|USB|"Không tự khởi tạo, cần `usb reset`"|**Sai.** `misc_init_r()` → `setup_MotionEnable_Power()` gọi `usb_init()` và `usb_stor_scan()` để tìm USB memory chứa firmware update (xem 7.5)|
|Secure boot|"Chỉ có SHA-256 index"|Còn thêm **`chk_sboot()`**: chốt "đã từng gắn secure chip" lưu trong SPI-flash, nếu tháo chip thì máy từ chối boot. Và `chkGmUpdate()` cập nhật Golden Master. Vẫn không thấy xác thực chữ ký trong U-Boot|

## 1. Sơ đồ tổng

```
[1] Reset ─ Boot ROM (silicon)                         ngoài repo
[2] ATF BL1/BL2/BL31 (DDR init, PSCI, SCPI)  @0x10000000  ngoài repo [S]
        │  nhảy vào U-Boot (BL33) tại 0x1000, EL2/EL1
[3] _start → reset            start.S       khởi tạo EL, cache; CPU phụ chờ spin-table
[4] _main                     crt0_64.S     dựng SP/GD (x18) → gọi board_init_f
[5] board_init_f              board_f.c     early init, console, I2C, DRAM, bố trí bộ nhớ
[6] relocate_code             relocate_64.S chép U-Boot lên đỉnh RAM, vá .rela.dyn
[7] board_init_r              board_r.c     cache/MMU, malloc, drivers, env, PCI, misc_init_r
[8] run_main_loop → main_loop main.c        quyết định kiểu boot, bootdelay
[9] bootcmd = fatload ×3 + booti   (kernel, dtb, initrd từ mmc/nvme 0:5)
[10] do_booti → do_bootm_states → boot_prep_linux → boot_jump_linux
[11] kernel_entry(fdt, 0, 0, 0)  ── Linux
```

Ba bước bạn liệt kê nhưng repo này **không có**:

- **TPL và SPL:** target `800` có `# CONFIG_SPL is not set`, nên nhánh SPL trong `start.S` không được biên dịch.
- **DDR init trong U-Boot:** không có mã training DDR nào được build. `dram_init()` chỉ gán một hằng số. DDR được khởi tạo trước đó bởi ATF/BootROM [S]. Bằng chứng: `armada8k.h` định nghĩa `PLAT_MARVELL_DDR_TRAINING_BASE 0x10400000` cạnh vùng ATF.
- **FIT:** không dùng. Kernel là `Image` ARM64 nạp bằng `booti`.

## 2. Các bước trước U-Boot (ngoài repo)

|||
|---|---|
|Entry|Boot ROM (silicon) → ATF → nhảy tới `_start`|
|Caller|ATF BL31 nhảy vào BL33 [S]|
|Bằng chứng|`start.S:173-176`: "U-boot is running in EL3 as standalone application, but ATF activates it at lower EL"; `armada8k.h:106-115` (ATF ở 0x10000000, 4 MB); `arm_scpi.c` đồng bộ define với ATF `plat/marvell/a8k/quartz`|
|Trạng thái khi vào|MMU tắt, cache tắt, little-endian (comment `start.S:67-68`). DDR đã sẵn sàng (vì `_main` dùng SP trong DRAM ở 0x00FF1000)|
|Địa chỉ nạp|`SYS_TEXT_BASE = 0x1000` ([mvebu-common.h:55](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/configs/mvebu-common.h)); linker script đặt `. = 0`|
|Lỗi|Ngoài phạm vi repo|

## 3. `_start` → `reset` (`arch/arm/cpu/armv8/start.S`)

- **Entry / caller:** `_start` là entry của ELF (`ENTRY(_start)` trong `u-boot.lds`). Được ATF gọi. [V]
- **Trình tự:** `b reset` → `adr vbar` → `switch_el` chọn EL3/2/1:
    - **EL3:** ghi `vbar_el3`, `SCR_EL3 |= 0xf`, `cptr_el3=0`, `cntfrq_el0 = COUNTER_FREQUENCY` (25 MHz).
    - **EL2:** ghi `vbar_el2` và `cptr_el2`.
    - **EL1:** ghi `vbar_el1` và `cpacr_el1`.
- **Tiếp theo:** `apply_core_errata` (chỉ áp cho **Cortex-A57**, `start.S:131`; nếu lõi thật là A72 thì hàm này không làm gì, xem Phần 0) → `__asm_flush_dcache_all`, `__asm_invalidate_icache_all`, `__asm_invalidate_tlb_all` → `lowlevel_init`.
- **`lowlevel_init`:** `cmp CurrentEL, EL3; b.ne 2f`. Nếu không ở EL3 (tức chạy dưới ATF) thì **bỏ qua** khởi tạo GIC.
- **Phân nhánh CPU:** `branch_if_master`: CPU chính → `bl _main`. CPU phụ → `wfe` lặp, đọc `CPU_RELEASE_ADDR`; khác 0 thì `br x0`.
- **Bộ nhớ:** vector tại `vectors` (exceptions.S). `CPU_RELEASE_ADDR = SDRAM_BASE + 0x2000000 = 0x02000000`.
- **Phần cứng:** thanh ghi hệ thống ARM (VBAR, SCR, CPTR, CNTFRQ), cache và TLB.
- **Lỗi:** không có kiểm tra. `relocate_code` sau này có `bl hang` nếu EL không phải 1/2/3.

## 4. `_main` (`arch/arm/lib/crt0_64.S`)

- **Caller:** `start.S` (`master_cpu: bl _main`). [V]
- **Trình tự:**
    1. `x0 = CONFIG_SYS_INIT_SP_ADDR` = `0x1000 + 0xFF0000` = **0x00FF1000**.
    2. `x18 = x0 - GD_SIZE`, tức **`gd` nằm trong thanh ghi x18** (build dùng `-ffixed-x18`).
    3. Zero vùng GD, đặt `GD_MALLOC_BASE` = `gd - 0x5000` (pre-reloc malloc `CONFIG_SYS_MALLOC_F_LEN`).
    4. `sp` = dưới vùng malloc, căn 16. `bl board_init_f(0)`.
    5. Khi `board_init_f` return: nạp `gd->start_addr_sp`, `gd->bd`, `gd->reloc_off`, `gd->relocaddr`, `adr lr, relocation_return; add lr, lr, x9`, rồi `b relocate_code`.
    6. Sau relocate: `c_runtime_cpu_setup` (đặt lại VBAR trỏ vào bản đã relocate), xoá BSS, `b board_init_r(x18, gd->relocaddr)`.
- **`[VENDOR]`:** `#if defined(KM_BIZHUB)` chuyển `GD_SIZE` sang `ldr x1, =GD_SIZE` để tránh lỗi build.
- **State:** `gd` (x18), `gd->malloc_base`.
- **Lỗi:** `board_init_f` không bao giờ return nếu hang. `board_init_r` không return.

## 5. `board_init_f` (`common/board_f.c`, chạy trước relocation)

- **Entry:** `board_init_f(boot_flags)` ở dòng 1012, được `_main` gọi. Đặt `gd->flags`, `have_console = 0`. **`[VENDOR]`** đặt `gd->arch.termFlg=0`, `printStopFlg=1`, `printBufferNum=0` (dòng 1036-1041).
- **Engine:** `initcall_run_list(init_sequence_f)`. Nếu một hàm trả khác 0 thì **`hang()`** (dòng 1043).

### 5.1 Các hàm trong `init_sequence_f` (theo thứ tự thực tế của target này)

|#|Hàm|File|Việc làm / state thay đổi|Phần cứng / bộ nhớ|
|---|---|---|---|---|
|1|`setup_mon_len`|board_f.c:281|`gd->mon_len = __bss_end - _start`||
|2|`setup_fdt`|board_f.c:356|`gd->fdt_blob = __dtb_dt_begin` (DTB nhúng, `OF_EMBED`). Env `fdtcontroladdr` có thể ghi đè|DTB nằm trong image|
|3|`initf_malloc`|:804|`gd->malloc_limit = base + 0x5000`, `malloc_ptr = 0`|Vùng ngay dưới GD|
|4|`arch_cpu_init`|weak|**Không làm gì** (không ai override)||
|5|`mark_bootstage`|:797|Ghi mốc `board_init_f`||
|6|`fdtdec_check_fdt`|lib/fdtdec.c|Kiểm tra header DTB. `[VENDOR]` fdtdec.c có sửa||
|7|`initf_dm`|:815|**Bỏ qua** (`CONFIG_DM` tắt)||
|8|`board_early_init_f`|[board/mvebu/common/init.c:69](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/board/mvebu/common/init.c)|`soc_early_init_f()` (chỉ SAR/PINCTL nếu bật, **không bật** ở defconfig này); `sys_info_init()`: chép dữ liệu ATF/SPL để lại tại `SYSTEM_INFO_ADDRESS` (comment ghi `0x4000000`) vào `gd->arch.local_sys_info[]`|Đọc RAM|
|9|`timer_init`|[mvebu-common/generic_timer.c:26](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/mvebu-common/generic_timer.c)|Đọc `GTC_CNTCR` tại `MVEBU_GENERIC_TIMER_BASE`; nếu bit 0 chưa bật thì ghi bật|**Generic Timer Controller (MMIO)**|
|10|`env_init`|[common/env_nowhere.c:29](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/env_nowhere.c)|`gd->env_addr = &default_environment[0]; gd->env_valid = 0`|Env chỉ là mảng compile-time|
|11|`init_baud_rate`|board_f.c:141|**`[VENDOR]`** đặt GPIO `FW_CONFIG4/5` làm input; nếu **bất kỳ chân nào = LOW** thì `gd->arch.term_sel = 1` (AP-UART), ngược lại `0` (CSRC-UART). Rồi `gd->baudrate = getenv("baudrate") hoặc 115200`|**GPIO** (`mvqz_gpio`)|
|12|`serial_init`|drivers/serial/serial.c|Khởi động cổng mặc định: `default_serial_console()` chọn **`eserial1` (AP-UART, `0xf0512000`) nếu `term_sel`, ngược lại `eserial3` (CSRC, `0xe8003000`)**|UART NS16550|
|13|`console_init_f`|common/console.c|`gd->have_console = 1`; xả pre-console buffer||
|14|`fdtdec_prepare_fdt`|lib/fdtdec.c|Kiểm tra DTB dùng được||
|15|`display_options`||In banner phiên bản (**bị đệm**, xem 9)||
|16|`print_cpuinfo`|[mvebu-common/misc.c:83](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/mvebu-common/misc.c)|**Trả về 0 không in gì** (in muộn ở `board_init`)||
|17|`init_func_i2c`|board_f.c:250|`i2c_init_all()`: khởi tạo 4 bus DW-I2C (`0xE8006000/6800/7000/7800`), in "I2C: ready"|**I2C DW ×4**|
|18|`announce_dram_init`|:188|In `DRAM:`||
|19|`dram_init`|[soc.c:138](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/armada8k/soc.c)|`gd->ram_size = 0x80000000` (**hằng số**)|Không chạm HW|
|20|`setup_dest_addr`|:384|`gd->ram_top = SDRAM_BASE + get_effective_memsize()` = **min(ram_size, 1 GB) = 0x40000000**; `relocaddr = ram_top`||
|21|`reserve_round_4k`|:450|Căn 4 KB||
|22|`reserve_mmu`|:458|`tlb_addr` = đáy vùng vừa trừ `PGTABLE_SIZE` (căn 64 KB), lưu `gd->arch.tlb_addr`|Bảng trang MMU|
|23|`reserve_lcd`|:475|**`[VENDOR]`** `CONFIG_LCD` bật: `lcd_setmem()` dành **framebuffer** tại đỉnh RAM; `gd->fb_base`|Framebuffer LCD|
|24|`reserve_trace`|:488|Không làm gì (`TRACE` tắt)||
|25|`reserve_uboot`|:513|`relocaddr -= mon_len` (căn 4 KB); `start_addr_sp = relocaddr`||
|26|`reserve_malloc`|:536|Trừ `TOTAL_MALLOC_LEN` (~5 MB)||
|27|`reserve_board`|:545|Cấp `bd_t` (`gd->bd`), zero||
|28|`reserve_global_data`|:566|Cấp vùng `new_gd`||
|29|`reserve_fdt`|:575|Cấp vùng chép DTB (`fdt_size = totalsize+0x1000`)||
|30|`reserve_stacks`|:594|`irq_sp = start_addr_sp`||
|31|`setup_dram_config`|:734|Gọi `dram_init_banksize()` (5.6)|**Đọc thanh ghi MC**|
|32|`show_dram_config`|:213|In tổng DRAM||
|33|`reloc_fdt`|:742|Chép DTB vào `new_fdt`, cập nhật `gd->fdt_blob`||
|34|`setup_reloc`|:752|`gd->reloc_off = relocaddr - 0x1000`; chép `gd` vào `new_gd`||

Hàm nằm ngoài danh sách vì điều kiện `#if`: `init_func_watchdog`, `misc_init_f`, `checkboard`, `testdram`, `init_post`, `jump_to_copy` (ARM tự gọi `relocate_code` từ `crt0_64.S`).

### 5.2 Bộ nhớ sau `board_init_f` [S, số tính từ code]

Xếp từ đỉnh xuống theo thứ tự các hàm `reserve_*`:

```
0x40000000  ← ram_top (min(2GB,1GB))
   TLB (PGTABLE_SIZE, căn 64K)
   Framebuffer LCD (lcd_setmem)
   U-Boot image (mon_len, căn 4K)  ← gd->relocaddr
   malloc (~5MB)
   bd_t
   gd (new_gd)
   FDT copy
   stack (irq_sp, start_addr_sp)
```

`reloc_off = relocaddr - 0x1000`.

### 5.3 Vì sao log không hiện khi khởi động (điểm rất khác upstream)

Xem mục 9. Toàn bộ `printf`/`puts` của các bước 15, 17, 18, 32 chạy khi `termFlg = 0`, nên **bị đệm, không ra UART**.

### 5.4 `board_early_init_f` chi tiết

`init_mbus()` chỉ chạy nếu `CONFIG_MVEBU_MBUS` (không bật). `cfg_eeprom_init()` chỉ nếu `CONFIG_BOARD_CONFIG_EEPROM` (không bật). `mvebu_setup_fdt()` chỉ với `MULTI_DT_FILE` (tắt).

### 5.5 Failure của giai đoạn này

- Hàm nào trả khác 0 → `hang()`, board treo, chỉ ATF watchdog (nếu đã bật) mới reset. Watchdog **chưa** được bật ở giai đoạn này (`initr_wd_start` nằm trong `init_sequence_r`).
- Kiểu hỏng thực tế: DTB hỏng (`fdtdec_check_fdt`); `serial_init` lỗi thì treo không có log.

### 5.6 `dram_init_banksize` (`[VENDOR]`, soc.c:173)

Đây là nơi đọc DRAM thật. Với mỗi channel `ch` 0..1 và chip-select `cs` 0..3:

- Đọc `MVEBU_MMAP_L(ch,cs) = 0xF0020200 + ch*0x200 + cs*8`. Bit 0 = valid.
- Trường `[20:16]`: nếu `< 7` thì kích thước `0x18000000 << val` (384 MB, 768 MB, 1.5 GB...), ngược lại `0x10000 << val`.
- Cộng vào `bi_dram[ch].size` và vào **`gd->chk_ram_Size`** (dùng cho HW check ở 7.4). Bank `ch` bắt đầu tại `0x200000000 * ch`.
- Nếu `MVEBU_MC_RCR & 1` (có vùng remap): `bi_dram[2].start = (RSBR & 0x3FFFFC00) << 10`, `bi_dram[2].size = (RCR & 0xFFF00000) + 1MB`.
- **`[CONFIG]`** Với `KM_MACHINE_SPAM/SPABKM`, `CONFIG_MEMORY_6GB_TO_5GB` giới hạn 6 GB thành 5 GB, trừ máy SparrowH/Sparrow III MLK (nhận ra bằng byte machineInfo đọc từ flash).
- **Hệ quả:** RAM Linux thấy được là số này, không phải 2 GB của `dram_init`. Giá trị này được ghi vào node `/memory` của DTB (xem 12).

## 6. Relocation (`arch/arm/lib/relocate_64.S`)

|||
|---|---|
|Entry / caller|`relocate_code(x0 = gd->relocaddr)`; `crt0_64.S` nhảy tới bằng `b` sau khi đã đổi `lr`|
|Trình tự|`x9 = relocaddr - __image_copy_start`; nếu 0 thì bỏ qua. Chép từng 16 byte từ `__image_copy_start` đến `__image_copy_end`. Duyệt `.rela.dyn` từ `__rel_dyn_start` đến `__rel_dyn_end`; với entry loại `R_AARCH64_RELATIVE` (1027) ghi `addend + offset` vào `dest + offset`|
|Sau cùng|Nếu cache đang bật thì `ic iallu` rồi `__asm_flush_dcache_range` cho vùng đích (`cache.S`, `[VENDOR]`). `ret` về địa chỉ đã relocate|
|State|`gd->relocaddr`, `gd->reloc_off`, `x18` trỏ `new_gd`|
|Phần cứng|Bộ nhớ DRAM; cache|
|Lỗi|Nếu EL không hợp lệ thì `bl hang`. Không có kiểm tra checksum|

Ở thời điểm này cache/MMU **vẫn tắt** (bật ở `initr_caches`), nên chép chậm nhưng an toàn.

## 7. `board_init_r` (`common/board_r.c`, chạy sau relocation)

- **Entry:** `board_init_r(new_gd, dest_addr)` (dòng 1128), được `crt0_64.S` `b` tới. Chạy `initcall_run_list(init_sequence_r)`; lỗi → `hang()`.

### 7.1 Thứ tự thực tế trong `init_sequence_r`

Xen kẽ giữa các bước là `INIT_FUNC_WATCHDOG_RESET`. **`[VENDOR]`** hàm này được định nghĩa lại là `initr_wd_refresh` ([board_r.c:880-900](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/board_r.c)). Nó gọi `km_scpi_set_wdt_ca72_ext(REFRESH, 0x1000|count, 5s)`, gửi lệnh `0x8C` qua SCPI tới ATF để **nạp lại watchdog 5 giây**.

|#|Hàm|Việc làm|Phần cứng / bộ nhớ|
|---|---|---|---|
|1|`initr_wd_start`|`[VENDOR]` **Bật watchdog CA72** qua SCPI: point `0x1000`, timeout 5 s|SCPI mailbox (`0x7ff00000`) tới ATF|
|2|`initr_reloc`|`gd->flags|= GD_FLG_RELOC|
|3|**`initr_caches`**|`enable_caches()` → `icache_enable`; `dcache_enable` → `mmu_setup` ([cache_v8.c:26](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/cache_v8.c)): **toàn bộ không gian địa chỉ map là Device-nGnRnE** (section 512 MB, VA 42-bit), rồi từng bank `bi_dram[i]` map thành **Normal**; ghi TTBR0/TCR/MAIR, bật `SCTLR.M` và `.C`|`gd->arch.tlb_addr`|
|4|`initr_reloc_global_data`|`monitor_flash_len = _end - __image_copy_start`||
|5|`initr_malloc`|`mem_malloc_init` tại `relocaddr - TOTAL_MALLOC_LEN`|Heap DRAM|
|6|`bootstage_relocate`|||
|7|**`board_init`**|`[VENDOR]` `sound_init()`+`sound_play()` (phát âm bật nguồn, I2S/CDMA/RT5640); `mvebu_print_info()` in `Board: <model>`, đọc **SoC rev** ở `0xE8000040` và **Board rev** ở `0xE830A000`; `mvebu_soc_init()` (comphy/thermal là weak nên gần như no-op) và **`*CPU_RELEASE_ADDR = 0`**; `mvebu_board_init()` → `mvebu_devel_board_init()`|I2S, thanh ghi ID|
|8|`stdio_init_tables`, **`initr_serial`**|`serial_initialize()` đăng ký `eserial1`, `eserial3` (**`[VENDOR]` COM2 panel bị loại khỏi console**), `mvebu_serial_initialize`; `serial_assign(default_serial_console()->name)`||
|9|`initr_announce`|In "Now running in RAM - U-Boot at: %08lx" và địa chỉ DT blob||
|10|**`initr_mmc`**|`mmc_initialize()` → driver Xenon (`drivers/mmc/xenon_mmc.c`); `mmc_soc_init()` tắt SD-PHY khỏi MPP (`EMMC_PHY_IO_CTRL`)|**eMMC/SD**|
|11|**`initr_bspi`**|`[VENDOR]` `bspi_initialize()` ([qspi.c:379](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/drivers/spi/qspi.c)): tìm node DT `marvell,bspi` (`bspi0`, `bspi1`), lấy `reg`, `memmap_base`, lệnh flash (read-id 0x9f, erase 0x20 hoặc 0xd8, sector 4 KB hoặc 64 KB, ...), lưu `devices[]`|**QSPI NOR** (`0xe8273000`; map `0xf4000000`, `0xf8000000`)|
|12|**`init_km`**|`[VENDOR]` chẩn đoán HW (7.4)|UART panel, GPIO, nút nguồn|
|13|**`initr_env`**|`env_relocate()`: với `ENV_IS_NOWHERE` thì `set_default_env(NULL)` **im lặng**; đọc env `loadaddr`|Env chỉ trong RAM|
|14|`initr_secondary_cpu`|`cpu_secondary_init_r()` weak||
|15|**`initr_pci`**|`pci_init()`: quét PCIe (NVMe). `[VENDOR]` `KM_PCIE_SET_MPS` sửa payload size; driver `pcie-mv-msb0125.c`, `pcie_dw.c`|**PCIe**|
|16|**`stdio_add_devices`**|`[VENDOR]` **`gd->arch.termFlg = 1`** (mở cổng log, xem 9)||
|17|`initr_jumptable`, `console_init_r`|||
|18|**`misc_init_r`**|`[VENDOR]` (7.5)|Panel, USB, GPIO|
|19|`interrupt_init`, `initr_enable_interrupts`|Bật IRQ. `[CONFIG]` `armada8k.h` định nghĩa `CONFIG_USE_IRQ` khi `KM_BIZHUB`|GIC|
|20|**`board_late_init`**|`quartz_late_init()` (7.6)|I2C bus 3, GPIO 271|
|21|`last_stage_init`|[soc.c:380](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/armada8k/soc.c): chỉ `MULTI_DT_FILE` mới có việc, nên **no-op**||
|22|**`run_main_loop`**|`for(;;) main_loop();`||

Không chạy do `#if`: `initr_flash` (`SYS_NO_FLASH`), `initr_dm`, `initr_ethaddr`/`initr_net` (`CMD_NET` tắt), `initr_scsi`, `initr_nand`, `initr_post`.

**Lưu ý:** `initr_pci` chạy **trước** `misc_init_r`, nên thiết bị NVMe đã được quét khi `misc_init_r` cần chọn thiết bị boot.

### 7.2 Failure của `board_init_r`

Hàm trả lỗi → `hang()`. Nhưng watchdog SCPI đã chạy từ bước 1, nên treo quá 5 s không có refresh thì **ATF reset board** (phải có refresh xen kẽ; `INIT_FUNC_WATCHDOG_RESET`). Các điểm treo có chủ ý ở 7.4, 7.5 dùng `_WAIT_FOREVER()` mà **vẫn refresh watchdog** (`initr_wd_refresh` mỗi giây), để board đứng yên hiện lỗi thay vì reset lặp.

### 7.3 Memory và trạng thái sau `board_init_r` (trước main_loop)

Cache/MMU bật, heap DRAM, IRQ bật, watchdog 5 s đang chạy, console đã đăng ký nhưng log vẫn chưa xả.

### 7.4 `init_km` (`[VENDOR]`, board_r.c:447)

- Gọi `init_Panel_Uart()` (UART panel `0xe8002800`, 57600).
- `uc_AutoHWcheckMode = 0`. Nếu `is_execHwCheckMode()` thì đặt `uc_HWcheckModeBoot = 1`, tự chẩn đoán; hoặc nếu không phải restart và `checkPowerSaveKey()` (giữ phím nguồn) thì cũng vào HW-check.
- Trong HW-check: nếu `uart_error` thì in `### BOOT-DIAG ERROR/Panel ###` rồi `_WAIT_FOREVER()`. Kiểm tra **`gd->chk_ram_Size` phải bằng `CONFIG_HWCHECK_MEMSIZE`** (8 GB mặc định, 5 GB/6 GB cho Sparrow, biến thể AIO2/SFP2 có ngưỡng riêng). Sai thì báo `BOOT-DIAG ERROR/Dimm...`, `set_Panel_BootDiagErr(CPU_BOARD_ERR)`, **treo vĩnh viễn**.
- Luôn chạy (ngoài HW-check): đặt GPIO fan `BOXFAN_HALF_REM`=0, `BOX_FAN_REM`=1.
- Dùng `serial_puts()` trực tiếp (`_CONSOLE_OUT`), nên thông báo lỗi này **hiện ngay** dù console đang đệm.

### 7.5 `misc_init_r` (`[VENDOR]`, [machine_setup.c:674](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/board/mvebu/armada8k/km/machine_setup.c))

Thứ tự:

1. **`misc_init_r_env()`**: `setenv("ethaddr","00:00:00:00:00:00")`; `read_rom()` đọc vùng tham số ROM trong SPI-flash: MAC (chỉ nhận nếu tiền tố `00:20:6B`, `00:50:AA` hoặc `08:00:86`), `machineInfo`, `machineNo`, serial `Assy1` → biến toàn cục `gMachineInfo`, `gMachineNo`, `gSerialNo`. Lỗi đọc thì in `Read ERROR.(SPI FLASH Parameter)` và thoát sớm.
2. `THRF_getAutoHwDebugFlag()`; `init_Panel()` (LCD).
3. **`setup_MotionEnable_Power()`** ([machine_setup.c:149](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/board/mvebu/armada8k/km/machine_setup.c)): nếu `uc_TempUtilityKey==0` (phím utility bấm) hoặc cờ line-card: watchdog 120 s, **`usb_init()` + `usb_stor_scan(1)`** (retry 2 lần, chờ 10 s giữa các lần), rồi **`check_action()`** đọc file `INDEX` trên USB để đặt **`g_action`**. Ngược lại: đọc SoftDipSW `0x000006` và cờ flash `0x07B000` để đặt `g_action` (`DEF_IISW`, `DEF_UPDATE`, `BOOT_ERR`, `DEF_BACKUP`, `DEF_RESTORE`, `BOOT_HW_CHECKMODE`, mặc định `DEF_BOOT`). Sau đó điều khiển GPIO **MOTION_EN_MC / MOTION_EN_IR** (chỉ bật cho boot thường) và khi IISW/backup thì `mdelay(4500)`.
4. `hwcDispBootDeviceDiag()`; **`boot_diag()`**: nếu HW-check tự động, `fsload_hash_check()` (SHA-256 các image so với `EXP_INDEX`); sai thì `Hash Error!!!`, báo lỗi storage, **treo**.
5. Xử lý debug-flag HW-check (có thể treo vô thời hạn hoặc đặt `g_action = BOOT_HW_CHECKMODE`); vẽ logo boot.

### 7.6 `quartz_late_init` ([quartz.c:152](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/board/mvebu/armada8k/quartz.c))

- `get_quartz_board()`: `i2c_set_bus_num(3); i2c_probe(0x50)`. **Có EEPROM → TOC.** Không có thì đọc GPIO 271 (FW_CONFIG[5]): 1 → ESEVAL, 0 → **EMU800**.
- `setenv("dtb_name", ...)`: TOC → `armada-quartz-toc.dtb` (kèm **`bspi_boot()`**), ESEVAL → `eseval.dtb`, **EMU800 → `emu800.dtb`**.
- `bspi_boot()` đọc header ở `0xf8000000 + 0x200000` (magic `0xbaadc0de`), chép DTB vào `0x1000` và kernel vào `0x800000`, `setenv("bootcmd","booti $kernel_addr - $fdt_addr")`. Trên TOC việc này bị **ghi đè** ở bước sau (main_loop gọi `check_autoboot` đặt lại `bootcmd`).
- Không nhận ra board thì in `UNKNOWN BOARD TYPE!` rồi tiếp tục (`dtb_name` không được đặt, bootcmd sau đó sẽ hỏng).

## 8. Environment

|||
|---|---|
|Nơi lưu|**Chỉ RAM**, `ENV_IS_NOWHERE`. Mọi thay đổi mất khi reset|
|Default|`default_environment[]` từ `CONFIG_EXTRA_ENV_SETTINGS` = `kernel_addr=0x800000`, `initrd_addr=0x3000000`, `initrd_name=irfs.ubt`, `fdt_addr=0x1000` ([armada8k.h:119-122](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/configs/armada8k.h)). Cộng `bootfile=Image` (`CONFIG_BOOTFILE`) [S], `bootdelay=3`|
|Đặt runtime|`ethaddr` (`misc_init_r_env`), `dtb_name` (`quartz_late_init`), rồi trong `main_loop`: `bootcmd`, `bootargs`, `initrd_high=ffffffff`, `device_str`|
|Lưu bền thay thế|**Không dùng env.** Trạng thái lưu ở SPI-flash (`userdata.c`, `mtd_flash_access.c`: SoftDipSW, NVRAM backup, secure info, cờ line-card) và sector cài đặt trên SSD/SD (`km_warp_setting_read/write`)|
|Lỗi|Không có cơ chế CRC. `should_load_env()` đọc `load-environment` trong DT (mặc định 1)|

## 9. Console: cơ chế đệm log của KM (điều khác upstream quan trọng nhất)

- `printf`/`puts`/`putc` được `[VENDOR]` bọc ([console.c:261-355](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/console.c)). Mỗi lệnh in gọi `BspPrintValid()`.
- **`BspPrintValid()`** gọi `getTermState()` ([dipvalue.c:48](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/board/mvebu/armada8k/km/dipvalue.c)):
    - `gd->arch.termFlg == 0` → `TERM_STATE_0` (**không in**, đệm).
    - `termFlg == 1` thì đọc **`isForceLogOutput()`** (GPIO `FW_CONFIG5` LOW hoặc `LOG_SW` HIGH → `TERM_STATE_1`), hoặc **SoftDipSW** (`0x51` bit 7 và `0x21` bit 0-1, đọc từ SPI-flash) để ra `TERM_STATE_0..4`.
- **Đệm:** `BspPrintBuffering()` chép chuỗi vào `gd->arch.printBuffer[printBufferNum++]` (**tối đa `PRINT_BUFFER_NUM` = 255 mục**; đầy thì mất, không báo).
- **Xả:** khi `getTermState() != TERM_STATE_0`, `BspPrintBufferFlush()` in toàn bộ log đã đệm **một lần duy nhất** rồi đặt `printStopFlg=0`.
- **Hệ quả thực tế:** máy production (`TERM_STATE_0`) **không bao giờ in log U-Boot ra UART**. Chỉ khi bật debug (GPIO/SoftDIP) mới thấy toàn bộ log (kể cả log của giai đoạn trước, được xả trễ). `termFlg` chỉ bằng 1 sau `stdio_add_devices` (bước 16 ở 7.1).
- **Ngoại lệ:** `init_km` dùng `serial_puts` trực tiếp nên thông báo lỗi chẩn đoán vẫn ra ngay.
- `BspPrintStopCheck()` chặn gọi đệ quy (vì `getTermState` cũng có thể in).
- Cổng vật lý: `term_sel` (`FW_CONFIG4/5`) chọn AP-UART hay CSRC-UART. Tham số kernel `console=` cũng phụ thuộc điều này (10.2).

## 10. `main_loop` (`common/main.c:2166`, `[VENDOR]` viết lại hoàn toàn)

- **Entry / caller:** `run_main_loop()` gọi trong vòng `for(;;)`. Nếu `main_loop` return thì chạy lại.

### 10.1 Trình tự

1. `THRG_getSoftDipSw(0xA1)` → cờ đo tự động (`b_AutoMeasurement`).
2. `cli_init()`; `run_preboot_environment_command()` (`PREBOOT` không định nghĩa).
3. **`g_action == BOOT_INIT`** → tắt watchdog, `initialize_card_main()` (xoá SPI-flash cho thẻ init) rồi **treo**. **`BOOT_BDERASE`** tương tự (`bderase_card_main`).
4. **`chk_sboot()`** ([autoboot.c:464](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/autoboot.c)): đọc cờ `THRD_SECAREA_SBOOT` từ SPI-flash và `sysBspSecureChip()` (đọc `0xC0042008` bit `0x10`, PIO_SYS[36]). Nếu chip chưa từng gắn: bình thường. Nếu chip **có** mà cờ chưa bật thì ghi cờ (`boot flg on`). Nếu **chip không có nhưng cờ đã bật** → `boot failed.` + tắt watchdog + `You can turn off the power.` + **`while(1){}`**.
5. **`bootdelay_process(&autoboot)`** ([autoboot.c:281](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/autoboot.c)): `bootdelay = env hoặc CONFIG_BOOTDELAY (3)`, DT có thể ghi đè. Gọi **`check_autoboot()`** (10.2) rồi `getenv("bootcmd")`.
6. **Warp!!** (nếu `autoboot == AUTOBOOT_WARP` hoặc line-card): ghi kiểu boot xuống SSD, `sound_stop()`, `nvme_opal_cmd_send(0)` (SSD), bật watchdog (SuperWarp 40 s / Warp 90 s), **`warp_boot(ssid)`** phục hồi ảnh hibernate. Thành công thì **không return** (không đi tiếp), thất bại thì `check_autoboot_DEF_BOOT_case`. Có gọi `warp_clear_bootf` để xoá snapshot.
7. `set_boot_type_to_main_routine(autoboot, ...)`: đọc-sửa-ghi **1 sector cài đặt** trên SSD/SD (`km_warp_setting_read/write`): ghi kiểu boot, thông tin PCI, panel type/option/size, DIP 177.
8. `autoboot == NOT_AUTOBOOT*` → in "Functional restriction 1/2" (không boot). Ngược lại `cli_process_fdt` rồi **`autoboot_command(s)`** (10.3).
9. Sau `autoboot_command`: tắt watchdog (`KM_SCPI_WDT_CA72_END`), vào **`cli_loop()`** (prompt `Marvell>>` ).

### 10.2 `check_autoboot()` và `make_bootcmds/make_bootargs`

`check_autoboot()` ([main.c:1870](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/main.c)) là công tắc quyết định bằng `g_action`:

|`g_action`|`bootcmd` (format)|`bootargs` root|Ghi chú|
|---|---|---|---|
|`USB_BOOT`|`CONFIG_USBCMD_ENV`|`root=/dev/sdc3 rootdelay=10`|nạp từ `usb 0`|
|`USB_UPDATE`|`CONFIG_USBCMD_SUBSET(_NORESET)_ENV`|như trên|thư mục `FW00xx/`|
|`DEF_IISW`/`DEF_BACKUP`/`DEF_RESTORE`|`CONFIG_IISWCMD_ENV` (`0:2`)|`root=/dev/kmsda7 rootdelay=1`||
|`DEF_UPDATE`|`CONFIG_IISWCMD_UPDATE_ENV` (`0:A`)|`kmsda7`||
|`BOOT_ERR`|`CONFIG_ERRCMD_ENV` (`0:2`)|||
|`BOOT_LINE`, `BOOT_HW_CHECKMODE`, `BOOT_SPEEDCHANGE`|`CONFIG_CMD_ENV` (`0:5`)|`kmsda7`||
|mặc định (`DEF_BOOT`)|**`CONFIG_CMD_ENV`** (`0:5`)|`kmsda7`|có thể chuyển `AUTOBOOT_WARP`|

Tất cả đều `setenv("initrd_high","ffffffff")` (nghĩa: **không** relocate initrd).

- **`make_bootcmds(env)`**: `sprintf(tmp, env, INIT, DEV, DEV, DEV)` với `isBootFromSD()` quyết định `("mmc rescan","mmc")` hay `("nvme init","nvme")`.
- **`make_bootargs(type, rfs)`** tạo: `boottyp=%d <rfs> [nopanel] console=... TerminalState=%d MachineInfo=%X MachineNo=%X SerialNo=%s [Capacity=1] IISW=%d FwDir=%s DevStr=%s IntegrityState=%d uio_pdrv_genirq.of_id=generic-uio`. `console=` là `tty1 loglevel=1` (không debug) hoặc `ttyS0,115200` (AP-UART) hoặc `ttyS5,115200` (CSRC-UART) theo `term_sel`.
- `isBootFromSD()` = `is_exist_mmc()`. Với `DNBMLK/HEMLK` thì ưu tiên NVMe nếu `is_exist_nvme()`.

### 10.3 `autoboot_command` (autoboot.c:343)

`stored_bootdelay != -1 && s && !abortboot(...)`. `abortboot_normal`: nếu `isWarpBootMode()` thì `bootdelay=0`; nếu không in `Hit any key to stop autoboot`. Đặt watchdog `bootdelay*2` (hoặc 5 s nếu delay=0). Trong vòng đếm, `tstc()` phát hiện phím → `sysBspSetAbort()`, in `Push Any Key`, **tắt watchdog**, không boot (rơi vào `cli_loop`). Không bị ngắt: đặt watchdog theo `g_action` (`DEF_BOOT` = **300 s**, USB/IISW 120 s; kiểu khác thì tắt) rồi **`run_command_list(bootcmd)`**. Riêng `DEF_BOOT` còn gọi `chkGmUpdate()`.

## 11. bootcmd: nạp kernel, DTB, initrd

`CONFIG_CMD_ENV` = `CMD_INITRD_ENV("%s","%s","0:5","")` ([armada8k.h:267](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/configs/armada8k.h)). Sau `sprintf`, `bootcmd` cho boot thường là:

```
mmc rescan | nvme init ;
fatload <mmc|nvme> 0:5 $kernel_addr $bootfile ;
fatload <mmc|nvme> 0:5 $fdt_addr    $dtb_name ;
fatload <mmc|nvme> 0:5 $initrd_addr $initrd_name ;
booti $kernel_addr $initrd_addr $fdt_addr
```

|Thành phần|Địa chỉ RAM|Nguồn (`0:5` = thiết bị 0, partition 5)|
|---|---|---|
|Kernel|`0x00800000` (`kernel_addr`)|`Image` (`bootfile`)|
|DTB|`0x00001000` (`fdt_addr`)|`emu800.dtb` (hoặc `eseval.dtb`, `armada-quartz-toc.dtb`)|
|Initrd|`0x03000000` (`initrd_addr`)|`irfs.ubt`|

- **Hàm:** `run_command_list` → `cli_hush.c` → `do_fat_fsload` (`cmd_fat.c`, `fs/fat/fat.c`, `[VENDOR]` có sửa). Với `mmc`: `mmc rescan` rồi `mmc_init`. Với `nvme`: `nvme init` → `cmd_nvme.c` → `nvme.c`.
- **Phần cứng:** SDHCI Xenon hoặc NVMe qua PCIe; **DMA** vào DRAM.
- **Lỗi:** `fatload` lỗi (file thiếu, đọc hỏng) thì lệnh trả lỗi và **hush dừng danh sách**; `booti` không chạy. Sẽ vào `cli_loop`. `dtb_name` rỗng (board không nhận diện) → `fatload ...` tên rỗng → lỗi.
- **Lưu ý (số 0x0200_0000):** `SYS_LOAD_ADDR`, `PRE_CON_BUF_ADDR` (1 MB) và `CPU_RELEASE_ADDR` trùng địa chỉ `0x02000000`. Tôi không xác minh được có va chạm thực sự hay không (đường console KM bỏ qua pre-console buffer). Chỉ ghi nhận.

## 12. `booti` và bàn giao cho kernel

|Bước|Hàm (file)|Việc làm|
|---|---|---|
|1|`do_booti` ([cmd_bootm.c:725](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/cmd_bootm.c))|Bỏ `argv[0]`, gọi `booti_start`|
|2|`booti_start` → `do_bootm_states(START)`|`images->ep = 0x800000` (`argv[0]`)|
|3|`booti_setup`|Kiểm tra magic `0x644d5241`, không khớp: **"Bad Linux ARM64 Image magic!"**, return 1. `image_size == 0` → giả 16 MiB. **`dst = bi_dram[0].start + text_offset`**; nếu `ep != dst` thì **`memmove` kernel** xuống `dst` (thường `0x80000`, phụ thuộc `text_offset` trong header Image [?])|
|4|`lmb_reserve` + `bootm_find_ramdisk_fdt`|Lấy initrd (`$initrd_addr`) và DTB (`$fdt_addr`)|
|5|`bootm_disable_interrupts`|Tắt IRQ, **`usb_stop()`** (tránh USB DMA khi Linux chạy)|
|6|`do_bootm_states(OS_PREP|FAKE_GO|
|7|`boot_prep_linux` → **`image_setup_linux`** ([image.c:1241](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/image.c))|Đặt vùng DTB: `boot_relocate_fdt` (chỉ dời nếu ngoài bootmap 16 MB [S]); rồi **`image_setup_libfdt`** ([image-fdt.c:465](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/image-fdt.c)): `fdt_chosen()` (ghi `bootargs` vào `/chosen`), **`arch_fixup_fdt()`**, `fdt_fixup_ethernet`, `fdt_shrink_to_minimum`, `fdt_initrd`|
|8|`arch_fixup_fdt` → `fdt_fixup_memory_banks` ([fdt_support.c:442](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/fdt_support.c))|Ghi `/memory` `reg` từ **`bi_dram[0..2]`** (số đã đọc ở 5.6). **`[VENDOR]`** thêm thuộc tính: `hddinfo`, `boottype` (=`g_action`), `hddexistence`, `sd_existence`, `paneltype`, `paneltypecode`, `paneloption`, `panelsize`, `utilitykey`, **`vramaddr` (=`gd->fb_base`) và `vramsize`**. Kernel dùng để giữ nguyên framebuffer boot animation|
|9|`boot_jump_linux` ([arch/arm/lib/bootm.c:284](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/lib/bootm.c))|`kernel_entry = images->ep`|
|10|`announce_and_cleanup`|In "Starting kernel ...", **`[VENDOR]`** ghi sự kiện `KernelStart` qua I2C bus 0 tới PS-CPU (`0x3E`, reg `0xC0`) nếu SoftDIP `0xA1` bit 0; `bootstage_report`; **`cleanup_before_linux`** ([armv8/cpu.c:19](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/cpu.c)): tắt IRQ, `icache_disable`, `invalidate_icache_all`, `dcache_disable` (KM: flush trước rồi mới tắt MMU), `invalidate_dcache_all`|
|11|PSCI|`is_psci_enabled()` = `current_el() == 3`. **Chạy dưới ATF thì `false`**, U-Boot **không** dựng PSCI; kernel gọi PSCI của ATF qua SMC|
|12|`do_nonsec_virt_switch`|`smp_kick_all_cpus()` (SGI 0 qua GICD), `flush_dcache_all()`, `armv8_switch_to_el2()` (`ARMV8_SWITCH_TO_EL1` bị comment)|
|13|**`kernel_entry(images->ft_addr, NULL, NULL, NULL)`**|**Nhảy vào kernel**|

**Trước bước 13, `do_bootm_states` cho `[VENDOR]`:** nếu `uc_HWcheckModeBoot == 1` thì `set_Panel_BootDiagStatus(ST5)` (đèn LED). Nếu lỗi bất kỳ thì `hwcDispLoadDiag()` báo lỗi boot lên panel.

**Trạng thái bàn giao cho Linux:**

- `x0` = địa chỉ DTB (đã được sửa), `x1..x3` = 0.
- MMU tắt, D-cache/I-cache tắt, IRQ tắt.
- EL: EL2 (hoặc EL mà ATF giao) [S].
- DTB chứa `/chosen/bootargs`, `/memory`, `linux,initrd-start/end`, thuộc tính vendor.
- Watchdog CA72 đang chạy 300 s (`DEF_BOOT`), kernel/userland phải refresh qua kênh SCPI.
- CPU phụ được đánh thức bằng SGI 0 (nếu ATF/kernel dùng PSCI thì không phụ thuộc spin-table).

**Failure của bước này:**

- `Bad Linux ARM64 Image magic!` → return 1 + `hwcDispLoadDiag()` → về `main_loop` → `cli_loop`.
- `image_setup_libfdt` lỗi → `FDT creation failed! hanging...` → **`hang()`** (watchdog 300 s sẽ reset).
- Kernel không boot → không có đường quay lại (không `return`), chỉ watchdog.

## 13. Bảng watchdog theo giai đoạn (`km_scpi_set_wdt_ca72_ext`)

|Point|Timeout|Nơi|Ý nghĩa|
|---|---|---|---|
|`0x1000`|5 s|`initr_wd_start`/`initr_wd_refresh`|Suốt `board_init_r`|
|`0x1120`/`0x1121`|120 s / 5 s|`setup_MotionEnable_Power`|Quét USB|
|`0x1130`|10 s|IISW/backup||
|`0x11D0`/`0x11D1`|`bootdelay*2` / 5 s|`abortboot_normal`|Đếm ngược|
|`0x11D2`|**300 s**|`autoboot_command` (DEF_BOOT)|Từ lúc boot đến khi userland lo watchdog|
|`0x11E0-2`|120 s|USB update/IISW||
|`0x11b0`/`0x11a0`|40 s / 90 s|SuperWarp / Warp||
|END||Sau `autoboot_command`, vào `cli_loop`, hoặc khi abort|Tắt watchdog|

## 14. Điều tôi chưa xác minh

- **`SYSTEM_INFO_ADDRESS`** (giá trị macro), `TOTAL_MALLOC_LEN`, `PGTABLE_SIZE`: chỉ suy từ comment/định nghĩa, chưa đọc số cuối.
- **`text_offset`** trong header `Image` của kernel: quyết định `booti_setup` có `memmove` hay không (phụ thuộc kernel, không nằm trong repo).
- **Nội dung `init_Panel`, `hwcDispBootDeviceDiag`, `warp_boot`, `pci_init` chi tiết** và `armv8_switch_to_el2` khi đã ở EL2: chưa mở thân hàm.
- **Bảng partition thực của SSD/SD** (`0:5`, `0:2`, `0:7`, `0:A`) chỉ suy từ macro; không có bảng thật.
- **ATF là BL33 handoff** ở EL nào: suy luận, ATF không có trong repo.
- **`bootfile=Image`** trong default env: suy từ `CONFIG_BOOTFILE`, chưa mở `env_common.c` để xác nhận.
- Chưa build nên các số bộ nhớ ở mục 5.2 chưa đối chiếu với `u-boot.map`.

Bước tiếp theo hợp lý là **Phần 3: sơ đồ bộ nhớ và các cấu trúc dữ liệu** (`gd_t`, `bd_t`, `gd->arch`, vùng flash của `userdata.c`), hoặc đi sâu một trục cụ thể. Tôi đề xuất trục `misc_init_r`, `check_action`, `g_action` vì đó là phần vendor phức tạp nhất và quyết định hành vi boot.