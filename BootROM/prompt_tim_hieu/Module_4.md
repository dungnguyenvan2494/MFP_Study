# Phần 3: Reset, startup và mức thấp

Tôi đã đọc `start.S`, `exceptions.S`, `transition.S`, `macro.h`, `crt0_64.S`, `relocate_64.S`, `sections.c`, `u-boot.lds`, các header `global_data.h`, `u-boot.h`, `system.h`, và các makefile liên quan. Đường dẫn tính từ `BootROM/BootROM/Emu800/Src/`. Nhãn: **[V]** đã đọc trong source, **[T]** tôi tự tính từ định nghĩa struct/macro (chưa có build để đối chiếu), **[S]** suy luận, **[?]** chưa xác minh.

## 0. Đính chính Phần 2

|Điểm|Đã nói ở Phần 2|Thực tế [V]|
|---|---|---|
|IRQ|`board_init_r` "bật IRQ"|Sai. [interrupts_64.c:12-25](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/lib/interrupts_64.c): `interrupt_init`, `enable_interrupts`, `disable_interrupts` đều là **hàm rỗng** (trả 0). U-Boot ARM64 **không bao giờ mở IRQ**, dù `armada8k.h` có `CONFIG_USE_IRQ`. Hệ quả: `iflag` trong `bootm_disable_interrupts` luôn 0|
|`armv8_switch_to_el2`|"Chưa rõ khi đã ở EL2"|[transition.S:18](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/transition.S): `switch_el x0, 1f, 0f, 0f` nghĩa là **chỉ EL3 mới chuyển**; EL2/EL1 thì `ret` ngay (no-op). Dưới ATF, bước này không làm gì|
|`SYSTEM_INFO_ADDRESS`|Chưa xác minh|`0x4000000` [V] ([system_info.h:22](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/include/asm/arch-mvebu/system_info.h))|
|`PGTABLE_SIZE`|Chưa xác minh|`0x10000` (64 KB) cho ARM64 [V] ([system.h:17](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/include/asm/system.h))|

## 1. Từ source tới địa chỉ: build quyết định mọi thứ

|Yếu tố|Giá trị|Nguồn|
|---|---|---|
|Link address|**`-Ttext 0x1000`** (`CONFIG_SYS_TEXT_BASE`)|[Makefile:755](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/Makefile), [mvebu-common.h:55](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/configs/mvebu-common.h)|
|Linker script|`. = 0x00000000`, thứ tự `.text` (`start.o` đứng đầu) → `.rodata` → `.data` → `.u_boot_list` → `.image_copy_end` → `.rela.dyn` → `_end` → `.bss`|[u-boot.lds](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/u-boot.lds)|
|Kiểu link|**PIE** (`LDFLAGS_u-boot += -pie`), để có `.rela.dyn` phục vụ relocation|[arch/arm/config.mk:85](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/config.mk)|
|Compile|`-fno-common -ffixed-x18 -mstrict-align -march=armv8-a`|[armv8/config.mk](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/config.mk)|
|`u-boot.bin`|`objcopy -O binary` (không chứa `.bss`); `DO_STATIC_RELA` **rỗng** vì `CONFIG_STATIC_RELA` không được đặt|[Makefile:702-709, 822-824](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/Makefile)|
|Các mốc|`__image_copy_start/end`, `__rel_dyn_start/end`, `__bss_start/end`, `_end` được **định nghĩa trong C** (`sections.c`) để linker sinh `R_AARCH64_RELATIVE` thay vì địa chỉ tuyệt đối trơ|[sections.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/lib/sections.c)|

**Hệ quả quan trọng [S, từ code]:** vì `u-boot.bin` không được vá tĩnh và `.data`/literal pool giữ địa chỉ tuyệt đối theo link `0x1000`, trước relocation **binary phải đang nằm đúng ở 0x1000**. `relocate_code` cũng nạp `__image_copy_start` bằng `ldr =`, tức lấy giá trị link-time. Đây là lý do `TEXT_BASE = 0x1000` phải khớp với địa chỉ ATF nạp BL33.

## 2. Reset vector và `start.S`

- **Entry:** `_start` (`ENTRY(_start)`). Với target `800` (không SPL) chỉ có nhánh `#else`: `b reset` ([start.S:41-42](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/start.S)). Nhánh `#if MVEBU && SPL_BUILD` (`lowlevel_init_spl` rồi `board_init_f` rồi `ret`) **không được biên dịch**.
- **Ngay sau `b reset`:** ba hằng `.quad` (không có lệnh):
    - `_TEXT_BASE` = 0x1000.
    - `_end_ofs`, `_bss_start_ofs`, `_bss_end_ofs` (dạng offset so với `_start`).
    - Tôi không thấy nơi nào dùng chúng trong nhánh ARM64 [?].
- **Vector address:** `adr x0, vectors` (PC-relative, nên đúng dù nằm ở đâu).

### 2.1 Phát hiện exception level: macro `switch_el`

[macro.h:66](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/include/asm/macro.h): `mrs CurrentEL`; so với `0xc` (EL3), `0x8` (EL2), `0x4` (EL1). Nếu không khớp cái nào thì rơi xuống lệnh kế tiếp (không có nhánh lỗi).

|EL|Việc làm trong `reset`|
|---|---|
|EL3|`vbar_el3 = vectors`; `SCR_EL3|
|EL2|`vbar_el2 = vectors`; `cptr_el2 = 0x33ff`|
|EL1|`vbar_el1 = vectors`; `cpacr_el1 = 3<<20`|

Dưới ATF, code chạy ở EL2 hoặc EL1 [S]. Lưu ý: `cntfrq_el0` **chỉ** được ghi ở EL3, nên U-Boot dựa vào ATF đã đặt sẵn.

### 2.2 Các bước còn lại trước khi vào C

1. **`apply_core_errata`** (weak): `branch_if_a57_core` so `MIDR.PartNum` với `0xD07`. Chỉ Cortex-A57 mới được vá (và chỉ khi `CONFIG_ARM_ERRATA_*` bật). Nếu lõi là **A72 (PartNum 0xD08)** hàm này không làm gì [V]. Đây là mắc xích chưa rõ CPU thật ở Phần 0.
2. Nếu không phải `CONFIG_PALLADIUM`: `__asm_flush_dcache_all`, `__asm_invalidate_icache_all`, `__asm_invalidate_tlb_all`.
    - **`[VENDOR]`** `cache.S:159` thêm `__asm_flush_l3_cache`: đọc `0xF0008100` bit 0; nếu bật thì ghi `0xFFFFFFFF` vào `0xF00087FC`, ghi `1` vào `0xF0008700`, `dsb sy`. Đây là thanh ghi **LLC** (`MVEBU_LLC_BASE = 0xF0008000`), được `flush_dcache_all()` của KM gọi.
3. **`lowlevel_init`** (weak): `CurrentEL != EL3` thì **bỏ qua** (comment: "ATF đã làm hết"). Ở EL3 mới khởi tạo GIC (`gic_init_secure`) và, với CPU phụ, `armv8_switch_to_el2`.
4. **Phân vai CPU:** `branch_if_master` kiểm `MPIDR_EL1`: chỉ CPU có toàn bộ 4 affinity field = 0 là master (`master_cpu: bl _main`). CPU khác vào `slave_cpu`: `wfe`, đọc `[CPU_RELEASE_ADDR]` (`0x02000000`), nếu 0 thì lặp, khác 0 thì `br x0`.
    - `mvebu_soc_init()` ghi `0` vào `CPU_RELEASE_ADDR`, nên vòng chờ này chỉ thoát khi ai đó ghi địa chỉ mới. Dưới ATF+PSCI, CPU phụ thường không vào U-Boot [S].

## 3. Chế độ CPU và trạng thái hệ thống

|Thời điểm|EL|MMU|I-cache|D-cache|IRQ|
|---|---|---|---|---|---|
|Vào `_start`|EL2/EL1 [S]|tắt|tắt|tắt|masked (do ATF)|
|Sau `initr_caches`|như trên|**bật** (`SCTLR.M`)|bật (`CR_I`)|bật (`CR_C`)|vẫn masked (`enable_interrupts` là hàm rỗng)|
|Trước khi vào kernel|giữ EL, `armv8_switch_to_el2` no-op|**tắt** (`dcache_disable`)|tắt|tắt|masked|

`SCTLR` bit: `CR_M=1<<0`, `CR_C=1<<2`, `CR_I=1<<12` ([system.h:9-15](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/include/asm/system.h)). Helpers `get_sctlr/set_sctlr` tự chọn thanh ghi theo `current_el()`.

**Bảng trang** (`mmu_setup`, [cache_v8.c:26](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/cache_v8.c)): mỗi entry là **section 512 MB** (`SECTION_SHIFT 29`), VA 42-bit (`VA_BITS 42`, `TCR_EL3_IPS_BITS` = 42-bit PA). Bảng `PGTABLE_SIZE = 0x10000` = 8192 entry × 8 byte, phủ 2^42. Toàn bộ mặc định là `MT_DEVICE_NGNRNE`; bank RAM `bi_dram[i]` được đổi sang `MT_NORMAL`. Vì đơn vị 512 MB, bank không chia hết cho 512 MB bị **cắt xuống** theo `end >> 29` [T].

## 4. Vector ngoại lệ (`exceptions.S`)

- `.align 11` (2 KB, yêu cầu của VBAR). Chỉ có **8 entry** mỗi 0x80 byte, dùng cho **Current-EL** (SP0 và SPx × Sync/IRQ/FIQ/SError). Không có entry cho Lower-EL, vì U-Boot chạy ở EL cao nhất mà nó sở hữu.
- Mỗi entry nhảy vào `exception_entry`: push x0-x30 (từ x29 xuống x1), đọc `ESR_ELn` và `ELR_ELn` theo EL (`switch_el`), push `ELR`/`x0`, `x0 = sp` (con trỏ `pt_regs`), gọi `do_bad_sync/irq/fiq/error` hoặc `do_sync/irq/fiq/error`.
- Handlers ([interrupts_64.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/lib/interrupts_64.c)): in `esr`, `show_regs()` (`ELR`, `LR`, `x0-x29`) rồi **`panic("Resetting CPU ...")`**. Không có xử lý phục hồi nào.
- **Stack dùng khi có ngoại lệ:** chính `sp` hiện tại. Không có stack riêng (`irq_sp` được đặt trong `reserve_stacks` nhưng ARM64 không dùng).
- **Chuỗi kết thúc:** `panic` ([vsprintf.c:845](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/lib/vsprintf.c)): `vprintf`, `putc('\n')`, `udelay(100000)`, `do_reset` ([arch/arm/lib/reset.c:30](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/lib/reset.c)): in "resetting ...", `udelay(50000)`, `reset_misc()`, **`reset_cpu(0)`**. Hàm này ([soc.c:285](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/armada8k/soc.c)) xoá bit 0 của `RFU_GLOBAL_SW_RST` (`0xF06F0084`), rồi **`[VENDOR]`** kéo GPIO `HRESET_REQ` xuống 0 (yêu cầu tắt/mở nguồn lại).
- **Vì log KM bị đệm** (Phần 2, mục 9), thông báo panic thường **không hiện ra UART** trong máy production, chỉ thấy board tự reset.
- **VBAR sau relocation:** `crt0_64.S` gọi `c_runtime_cpu_setup` ([start.S:233](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/cpu/armv8/start.S)), đặt lại `VBAR_ELn = adr vectors` (bản đã relocate).
- **Trước relocation** VBAR trỏ tới `vectors` ở khoảng 0x1000; một ngoại lệ sớm dùng SP ở vùng 0xFAA7xx (xem mục 5).

## 5. Dựng stack và `gd` (`crt0_64.S:_main`)

### 5.1 Các bước

```
x0  = CONFIG_SYS_INIT_SP_ADDR                 = 0x1000 + 0xFF0000 = 0x00FF1000
x18 = x0 - GD_SIZE            (bic 7: căn 8)       ← gd trỏ ở đây
loop zero_gd: ghi 0 từng 8 byte từ x0 xuống x18   ← xoá toàn bộ vùng gd
x0  = x18 - CONFIG_SYS_MALLOC_F_LEN (0x5000)
gd->malloc_base = x0
sp  = x0 & ~0xf               (căn 16)
board_init_f(0)
```

`GD_SIZE` là hằng do `lib/asm-offsets.c` sinh lúc build (`DEFINE(GD_SIZE, sizeof(struct global_data))`). **`[VENDOR]`** vì `GD_SIZE` quá lớn để nhúng vào lệnh `sub x18, x0, #imm`, KM đổi sang `ldr x1,=GD_SIZE; sub x18, x0, x1` ([crt0_64.S:65-70](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/lib/crt0_64.S)).

### 5.2 Kích thước `gd_t` và vì sao nó lớn

`arch_global_data` của KM chứa mảng đệm log (`global_data.h:72`):

```c
char printBuffer[PRINT_BUFFER_NUM][CONFIG_SYS_PBSIZE];   /* 255 × 1051 */
```

- `PRINT_BUFFER_NUM = 255` ([mvebu-common.h:261](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/configs/mvebu-common.h)).
- `CONFIG_SYS_PBSIZE = CBSIZE(1024) + sizeof("Marvell>> ")(11) + 16 = 1051`.
- Riêng mảng này = 255 × 1051 = **268.005 byte**.

Tôi cộng tay các field theo định nghĩa (giả định `phys_size_t` 8 byte, `enum` 4 byte, các `#if` bật/tắt theo config đã xác minh) [T]:

|Phần|Offset bắt đầu|Kích thước|
|---|---|---|
|Field chung (`bd` ... `cur_serial_dev`)|0|288 B|
|`arch`: timer, `tlb_addr/size`, `soc_family/board_family/reg_base`|288|80 B|
|`arch.local_sys_info[MAX_OPTION=13]` (mỗi phần tử 8 B)|368|104 B|
|`arch.printBuffer[255][1051]`|472|268.005 B|
|5 byte cờ (`printBufferNum`, `printStopFlg`, `uc_HWcheckMode_Dimm`, `termFlg`, `term_sel`) + đệm||~11 B|
|**Tổng `GD_SIZE`**||**≈ 268.488 B = 0x418C8** [T]|

Muốn có số chính xác: `grep GD_SIZE include/generated/asm-offsets.h` sau khi build.

### 5.3 Bố cục pre-relocation (số tính từ mục 5.1 và 5.2) [T]

```
0x00FF1000  CONFIG_SYS_INIT_SP_ADDR            (comment: "End of 16M scrubbed by training in bootrom")
0x00FAF738  gd  (x18)   ◄── GD_SIZE ≈ 0x418C8 bên dưới 0x00FF1000
0x00FAA738  malloc_f base  (0x5000 = 20 KB, pre-reloc malloc)
0x00FAA730  sp ban đầu, stack mọc xuống ↓
   ...      (trống, ≈ 14 MB)
0x00001000 + mon_len  ← _end của U-Boot image
0x00001000  U-Boot (TEXT_BASE, _start)
```

Vì `gd` ở thanh ghi x18, mã nào cũng thấy `gd` mà không cần biến toàn cục.

## 6. Đầu vào C sớm: `board_init_f`

- Gọi bởi `_main`. Nhận `boot_flags = 0` vào `gd->flags`.
- Code C khai báo `DECLARE_GLOBAL_DATA_PTR` = `register volatile gd_t *gd asm("x18")` ([global_data.h:108](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/include/asm/global_data.h)). Trình biên dịch không được dùng x18 vì `-ffixed-x18`.
- **Ràng buộc trước relocation** (quan trọng khi đọc code):
    - Cache và MMU tắt, mọi truy cập là bộ nhớ thường (strict-align do `-mstrict-align`).
    - **BSS chưa được xoá** và nằm ngoài `u-boot.bin` (objcopy bỏ `.bss`): biến `static`/global khởi tạo 0 có giá trị **rác** cho đến sau relocation.
    - **`.data` khởi tạo thì dùng được** (RAM, không phải flash), và các thay đổi trước relocation **được chép theo** vì việc chép xảy ra sau.
    - `malloc()` trước relocation là **bump-allocator** `malloc_simple` ([malloc_simple.c:15](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/malloc_simple.c)): dùng `gd->malloc_base + malloc_ptr`, vượt `malloc_limit` (base+0x5000) là **`panic("Out of pre-reloc memory")`**. Không có `free`.
- **Quan sát về BSS trước relocation [V, code đọc]:** `gc_stopflg` ([console.c:262](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/console.c)) nằm ở `.bss` nhưng `BspPrintStopCheck()` được gọi từ `puts/putc/printf` **trước** relocation. Nếu ô nhớ đó tình cờ bằng 1, lần in đầu bị nuốt (bị coi là gọi lồng). Sau relocation BSS được xoá nên hết. Ảnh hưởng nhỏ.

## 7. Cấu trúc `gd_t` (global_data)

Định nghĩa: [asm-generic/global_data.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/asm-generic/global_data.h) + `arch_global_data` trong [arch/arm/include/asm/global_data.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/include/asm/global_data.h). Chỉ liệt kê field **có mặt trong build này** và ai đặt chúng:

|Field|Ý nghĩa|Ai đặt / khi nào|Dùng bởi|
|---|---|---|---|
|`bd`|con trỏ `bd_t`|`reserve_board` (`board_init_f`)|mọi nơi cần bank RAM|
|`flags`|cờ `GD_FLG_*`|`board_init_f(boot_flags)`; `GD_FLG_RELOC\|FULL_MALLOC_INIT` ở `initr_reloc`|`console.c` (`SILENT`, `DEVINIT`)|
|`baudrate`|baud|`init_baud_rate`|serial|
|`cpu_clk/bus_clk/pci_clk/mem_clk`|xung|không thấy ai đặt trong nhánh này [?]||
|`fb_base`|đáy framebuffer LCD|`reserve_lcd`|`drv_lcd_init`, `fdt_fixup_memory_banks` (`vramaddr`)|
|`have_console`|serial đã init|`console_init_f`|`putc/puts`|
|`precon_buf_idx`|chỉ số pre-console buffer|console.c|pre-console (`0x2000000`, 1 MB)|
|`env_addr`, `env_valid`|env|`env_init` (env_nowhere): `default_environment`, 0|`env_relocate`, `getenv`|
|`ram_top`|đỉnh RAM U-Boot dùng|`setup_dest_addr`|tính relocaddr|
|**`relocaddr`**|**địa chỉ bắt đầu U-Boot sau khi relocate**|`setup_dest_addr`, giảm dần qua các `reserve_*`|`crt0_64.S`, `relocate_code`, `initr_malloc`|
|`ram_size`|tổng RAM|`dram_init` (0x80000000 cố định)|tính `ram_top`|
|**`chk_ram_Size`**|**`[VENDOR]`** tổng DRAM thật cộng từ MC|`dram_init_banksize`|`init_km` (HW check)|
|`mon_len`|độ dài U-Boot|`setup_mon_len`|`reserve_uboot`|
|`irq_sp`|SP cho IRQ|`reserve_stacks`|(ARM64: không dùng)|
|**`start_addr_sp`**|**SP đầu tiên sau relocation**|`reserve_uboot`... `reserve_stacks`|`crt0_64.S`|
|**`reloc_off`**|**`relocaddr - TEXT_BASE`**|`setup_reloc`|`crt0_64.S` (đổi `lr`)|
|`new_gd`|vị trí `gd` sau relocate|`reserve_global_data`|`setup_reloc`, `crt0`|
|`fdt_blob`|DT đang dùng|`setup_fdt`; đổi ở `reloc_fdt`|fdtdec, mọi driver dùng DT|
|`new_fdt`, `fdt_size`|vùng chép DTB|`reserve_fdt`|`reloc_fdt`|
|`jt`|jump table|`initr_jumptable`|standalone app|
|`env_buf[32]`|buffer `getenv` trước reloc||env|
|`cur_i2c_bus`|bus I2C hiện tại|`i2c_set_bus_num`|I2C|
|`timebase_h/l`||không dùng ARM64||
|`malloc_base/limit/ptr`|malloc pre-reloc|`_main`, `initf_malloc`|`malloc_simple`|
|`cur_serial_dev`||không dùng (không DM)||
|**`arch.tlb_addr/tlb_size`**|**bảng trang MMU**|`reserve_mmu`|`mmu_setup`|
|`arch.soc_family/board_family/reg_base`|MVEBU|[?] chưa thấy ai đặt||
|**`arch.local_sys_info[13]`**|dữ liệu từ ATF/SPL|`sys_info_init` (từ `0x4000000`)|`get_info()`|
|**`arch.printBuffer[255][1051]`**|**`[VENDOR]`** đệm log|`BspPrintBuffering`|`BspPrintBufferFlush`|
|`arch.printBufferNum/printStopFlg/termFlg/term_sel`|**`[VENDOR]`** trạng thái log/terminal|`board_init_f`, `stdio_add_devices`, `init_baud_rate`|console, serial|

Field `dm_root*`, `trace_buff`, `post_*`, `do_mdm_init`... **không có** vì `DM`, `TRACE`, `POST`, `MODEM_SUPPORT` tắt.

## 8. Cấu trúc `bd_t` (board info)

`CONFIG_SYS_GENERIC_BOARD` bật nên dùng [include/asm-generic/u-boot.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/asm-generic/u-boot.h), không phải bản trong `arch/arm/include/asm/u-boot.h`. Field có mặt (tính ≈ **184 byte** [T]):

`bi_memstart`, `bi_memsize`, `bi_flashstart`, `bi_flashsize`, `bi_flashoffset`, `bi_sramstart`, `bi_sramsize`, `bi_arm_freq`, `bi_dsp_freq`, `bi_ddr_freq`, `bi_bootflags`, `bi_ip_addr`, `bi_enetaddr[6]`, `bi_ethspeed`, `bi_intfreq`, `bi_busfreq`, `bi_arch_number`, `bi_boot_params`, và **`bi_dram[3]`** (`CONFIG_NR_DRAM_BANKS = 3`, mỗi phần tử `{start, size}`).

Thực tế trong build này:

- **Field thực sự có giá trị:** chỉ `bi_dram[0..2]`, do `dram_init_banksize()` điền (Phần 2, mục 5.6).
- `bi_memstart/bi_memsize`: chỉ PPC mới đặt (`setup_board_part1`), nên trên ARM64 chúng **bằng 0**.
- `bi_arch_number`: chỉ đặt nếu có `CONFIG_MACH_TYPE`; không có nên 0.
- `bi_enet*addr`: chỉ đặt nếu `CMD_NET`; không có.
- **Nơi đọc `bi_dram`:** `mmu_setup` (map Normal), `arch_fixup_fdt` (ghi `/memory`), `booti_setup` (`bi_dram[0].start + text_offset`), `arch_lmb_reserve`.

## 9. Bố trí bộ nhớ sau `board_init_f` và các con số

Tính theo đúng thứ tự `reserve_*` trong `init_sequence_f` (mục 5.1 của Phần 2), với `ram_size = 2 GB` nên `get_effective_memsize() = 1 GB`:

|Bước|Hàm|`relocaddr` / kết quả|Ghi chú|
|---|---|---|---|
|1|`setup_dest_addr`|`ram_top = relocaddr = 0x40000000`|`SDRAM_BASE(0) + min(2GB, 1GB)` [V]|
|2|`reserve_round_4k`|không đổi||
|3|`reserve_mmu`|trừ `0x10000`, căn 64 KB: **`tlb_addr = 0x3FFF0000`**|[T]|
|4|`reserve_lcd`|`lcd_setmem`: kích thước = 1280 × 768 × 2 = **0x1E0000** (đã căn trang); **`relocaddr = fb_base = 0x3FE10000`**|Kích thước là **tối đa** panel (`KM_PANEL_MAX_*`, [lcd.c:587](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/lcd.c)); `CONFIG_LCD_ALIGNMENT = PAGE_SIZE` giả định 4 KB [S]|
|5|`reserve_uboot`|`relocaddr = (0x3FE10000 - mon_len) & ~0xFFF`; `start_addr_sp = relocaddr`|`mon_len = __bss_end - _start`, cần build để có số [?]|
|6|`reserve_malloc`|`start_addr_sp -= TOTAL_MALLOC_LEN`|`TOTAL_MALLOC_LEN = 5 MB + ENV_SIZE = 0x510000` [S]|
|7|`reserve_board`|`start_addr_sp -= sizeof(bd_t)`; `gd->bd = start_addr_sp`; zero|≈184 B|
|8|`reserve_global_data`|`start_addr_sp -= sizeof(gd_t)`; `new_gd`|≈0x418C8|
|9|`reserve_fdt`|`start_addr_sp -= fdt_size`|`ALIGN(totalsize+0x1000, 32)`|
|10|`reserve_stacks`|`start_addr_sp -= 16; &= ~0xf`; `irq_sp = start_addr_sp`||

```
0x40000000  ram_top (1 GB, giới hạn "effective memsize")
0x3FFF0000  ── TLB (64 KB)                  gd->arch.tlb_addr
0x3FE10000  ── Framebuffer LCD (1.875 MiB)  gd->fb_base   ◄─ Linux nhận qua /memory "vramaddr"
            ── U-Boot image (mon_len)        gd->relocaddr = R
R-0x510000  ── malloc (5 MB + 64 KB)
            ── bd_t
            ── gd_t (new_gd, ~262 KB)
            ── bản sao FDT
            ── stack (start_addr_sp, mọc xuống ↓)
```

`reloc_off = relocaddr - 0x1000` (đặt ở `setup_reloc`). Đây là hằng số cộng vào mọi địa chỉ khi vá `.rela.dyn`.

Ví dụ minh hoạ: nếu `mon_len` = 1.5 MB (0x180000) thì `R = 0x3FC90000` (`0x3FE10000 - 0x180000`), `reloc_off = 0x3FC8F000`. Đây chỉ là ví dụ, không phải số đo.

## 10. Relocation, BSS và chuyển sang `board_init_r`

Trình tự trong `crt0_64.S` sau khi `board_init_f` return ([crt0_64.S:86-131](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/lib/crt0_64.S)):

1. `sp = gd->start_addr_sp & ~0xf` (SP mới, trong DRAM đã relocate).
2. `x18 = gd->bd - GD_SIZE` (**gd mới nằm ngay dưới `bd`**, khớp `reserve_global_data`). Khác với `gd->new_gd`, crt0 không đọc `new_gd` mà tính lại theo `bd`.
3. `lr = relocation_return + gd->reloc_off` (địa chỉ sau relocation), `x0 = gd->relocaddr`, rồi **`b relocate_code`**. Dùng `b` chứ không phải `bl`, nên `ret` trong `relocate_code` nhảy về **bản đã relocate**.

`relocate_code` ([relocate_64.S:22](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/lib/relocate_64.S)):

|Bước|Chi tiết|
|---|---|
|Offset|`x9 = x0 - __image_copy_start`; bằng 0 thì bỏ qua|
|Chép|`ldp/stp` từng 16 byte từ `__image_copy_start` đến `__image_copy_end` (bao gồm `.text .rodata .data .u_boot_list`; **không** gồm `.bss` và `.rela.dyn`)|
|Vá|Duyệt bản ghi `Elf64_Rela` từ `__rel_dyn_start` đến `__rel_dyn_end`. Mỗi bản ghi 24 byte (`r_offset, r_info, r_addend`). Với `(r_info & 0xffffffff) == 1027` (`R_AARCH64_RELATIVE`) ghi `*(r_offset + x9) = r_addend + x9`|
|Bản ghi khác|**Bị bỏ qua im lặng** (không cảnh báo)|
|Cache|Nếu `SCTLR.C` bật thì `ic iallu; isb`, rồi `__asm_flush_dcache_range(dst)`. Lúc này cache chưa bật nên bước này thực tế bị `tbz` bỏ qua|
|Lỗi|EL không hợp lệ (không phải 1/2/3) thì `bl hang`|

**Chú ý:** vòng vá **đọc `.rela.dyn` ở bản gốc (0x1000)** nhưng **ghi vào bản đích**. Vì thế bản gốc không bị sửa. Và vì `.rela.dyn` không được chép sang đích, nó chỉ tồn tại ở vị trí cũ.

Sau đó:

- **`c_runtime_cpu_setup`**: đặt lại `VBAR` (mục 4).
- **Xoá BSS**: `x0 = __bss_start`, `x1 = __bss_end` (literal pool đã được vá nên là địa chỉ đã relocate), `str xzr` từng 8 byte tới `x1`. Linker script căn 8 nên không thừa/hụt byte. Đây là thời điểm các biến `static`/global bằng 0 trở nên hợp lệ.
- `x0 = gd`, `x1 = gd->relocaddr`, **`b board_init_r`**. Không có `bl`, không return.

Trong `board_init_f`, hàm cuối `setup_reloc()` cũng chép **toàn bộ `gd_t`** (kể cả `printBuffer` 262 KB) sang `new_gd`, nên **log đã đệm không mất** qua relocation.

## 11. Vùng malloc

|Giai đoạn|Cơ chế|Vị trí|Kích thước|Giới hạn|
|---|---|---|---|---|
|Trước relocation|`malloc_simple` (bump, không free)|`gd->malloc_base` = `gd - 0x5000`|`CONFIG_SYS_MALLOC_F_LEN = 0x5000` (20 KB)|`panic("Out of pre-reloc memory")`|
|Sau relocation|dlmalloc (`common/dlmalloc.c`) qua `mem_malloc_init`|`relocaddr - TOTAL_MALLOC_LEN`|`TOTAL_MALLOC_LEN` = `SYS_MALLOC_LEN (5<<20) + ENV_SIZE (0x10000)` = **0x510000** [S]|hết heap thì `malloc` trả NULL|

Vùng malloc **ngay dưới ảnh U-Boot** (`initr_malloc: malloc_start = relocaddr - TOTAL_MALLOC_LEN`), khớp `reserve_malloc`. Dữ liệu cấp phát trước relocation **không được chuyển** sang heap mới [S]. `GD_FLG_FULL_MALLOC_INIT` báo hiệu heap đầy đủ đã sẵn sàng.

## 12. Bản đồ vị trí toàn diện

|Đối tượng|Địa chỉ|Nguồn|
|---|---|---|
|Ảnh U-Boot ban đầu (`_start`, TEXT_BASE)|`0x00001000`|[V]|
|Initial SP addr|`0x00FF1000`|[V]|
|`gd` ban đầu (x18)|≈ `0x00FAF738`|[T]|
|malloc pre-reloc|≈ `0x00FAA738`, 20 KB|[T]|
|SP ban đầu|≈ `0x00FAA730`|[T]|
|`CPU_RELEASE_ADDR` (spin-table) / `SYS_LOAD_ADDR` / `PRE_CON_BUF_ADDR`|`0x02000000` (cả ba trùng địa chỉ)|[V]|
|`SYSTEM_INFO_ADDRESS` (dữ liệu từ ATF)|`0x04000000`|[V]|
|`INDEX_LOAD_ADDRESS` / `IMAGE_LOAD_ADDRESS` (hash check)|`0x00700000` / `0x00800000`|[V]|
|`kernel_addr` / `fdt_addr` / `initrd_addr`|`0x00800000` / `0x00001000` / `0x03000000`|[V]|
|ATF (BL1/2/31), 4 MB|`0x10000000`|[V]|
|DDR training|`0x10400000` (1 MB)|[V]|
|`CONFIG_SYS_MEMTEST_START/END`, scratch|`0x0` – `0x10000000`, scratch `0x10800000`|[V]|
|SCPI mailbox|`0x7FF00000` (1 MB)|[V]|
|Ảnh U-Boot sau relocation|ngay dưới `0x3FE10000`|[T]|
|TLB / Framebuffer|`0x3FFF0000` / `0x3FE10000`|[T]|
|Thanh ghi SoC|`MVEBU_REGS_BASE = 0xF0000000` (GICD `+0x210000`, GICC `+0x220000`, GTC `+0x581000`, UART AP `+0x512000`, LLC `+0x8000`, RFU `+0x6F0000`)|[V]|
|Memory controller|`0xF0020200 + ch*0x200 + cs*8`|[V]|
|Southbridge (UART, I2C, QSPI, SDHCI)|`0xE8xxxxxx`|[V]|
|LCD/LVDS|`0xC057E000`|[V]|

Lưu ý: `fdt_addr = 0x1000` chính là chỗ U-Boot từng chạy trước relocation. Sau relocation vùng này rảnh nên dùng cho DTB là hợp lệ.

## 13. Điểm lỗi và hành vi khi hỏng ở mức thấp

|Tình huống|Hậu quả|Ghi chú|
|---|---|---|
|`board_init_f` có hàm trả khác 0|`hang()`: `puts("### ERROR ### Please RESET the board ###")` rồi vòng vô hạn ([hang.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/lib/hang.c))|Watchdog SCPI **chưa** bật lúc này; log bị đệm nên không thấy|
|`hang()` sau `initr_wd_start`|Vòng vô hạn; watchdog 5 s **reset board** nếu không refresh||
|Ngoại lệ đồng bộ/lỗi (`do_sync` ...)|`panic` → `do_reset` → `reset_cpu`: RFU reset + `HRESET_REQ`|Không có trap-and-continue|
|`malloc_simple` hết 20 KB|`panic("Out of pre-reloc memory")` → reset||
|Load address ≠ 0x1000|Con trỏ trong `.data`/literal pool sai, treo hoặc ngoại lệ mà không có chẩn đoán|Suy luận từ mục 1 [S]|
|Cấu trúc bảng trang|Bank RAM không chia hết 512 MB bị cắt ở `mmu_setup`|[T]|
|`reset_with_recoverycode()`|Chỉ dùng ở driver video (PLL/panel lỗi): ghi mã lỗi + bộ đếm vào flash rồi `do_reset`; **quá 5 lần thì không reset nữa** để tránh vòng lặp reboot ([reset.c:67](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/lib/reset.c))|`[VENDOR]`|

**Hai rủi ro tôi thấy khi đọc code KM (không phải lỗi đã kích hoạt được):**

1. **`BspPrintBuffering` dùng `sprintf(dest, printbuffer)` không giới hạn** ([console.c:298](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/console.c)). Mỗi ô đệm dài 1051 byte. `puts()` truyền chuỗi dài hơn 1051 byte (ví dụ help text dài) sẽ tràn sang ô kế tiếp trong `gd`. Chuỗi cũng bị coi là **format string**, nên ký tự `%` trong nội dung (`"100%"`) gây hành vi sai. Chỉ xảy ra trong giai đoạn còn đệm.
2. **Đệm đầy 255 mục thì mất log không báo** (`if printBufferNum < PRINT_BUFFER_NUM`).

## 14. Cách tự kiểm chứng trên máy thật

- **`bdinfo`** (`CONFIG_CMD_BDI=y`): in `TLB addr`, `relocaddr`, `reloc off`, `irq_sp`, `sp start`, `FB base` và các bank DRAM ([cmd_bdinfo.c:371-378](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/common/cmd_bdinfo.c)). Đối chiếu với bảng ở mục 9 để xác nhận `GD_SIZE`, `TOTAL_MALLOC_LEN`, `mon_len`.
- Sau build: `include/generated/asm-offsets.h` (`GD_SIZE`), `u-boot.map` (`__image_copy_end`, `__bss_end`) và `nm u-boot | grep -E "_end|__bss"`.
- Muốn thấy log thật trên UART cần bật debug (GPIO `FW_CONFIG5`/`LOG_SW` hoặc SoftDIP), vì log KM bị đệm.

## 15. Điều tôi chưa xác minh

- **`GD_SIZE`, `sizeof(bd_t)`, `TOTAL_MALLOC_LEN`** đều là số tôi cộng tay từ định nghĩa struct/macro (giả định căn lề 8, `phys_size_t` 8 byte). Cần đối chiếu bằng `bdinfo`/`asm-offsets.h`.
- **`mon_len`** và địa chỉ relocate thật (mục 9) phụ thuộc kích thước binary.
- **ATF nạp BL33 chính xác ở 0x1000 và ở EL nào**: suy luận từ ràng buộc link/PIE, ATF không có trong repo.
- **Lõi CPU thật** (A57 hay A72) vẫn chưa chốt. Chỉ biết errata trong `start.S` chỉ áp cho A57.
- **`arch.soc_family/board_family/reg_base`, `cpu_clk`, `bus_clk`**: tôi không thấy ai ghi vào trong nhánh này.
- **Các hằng `_TEXT_BASE`, `_end_ofs`... ở đầu `start.S`**: không thấy nơi dùng.
- **PAGE_SIZE = 4 KB** cho `CONFIG_LCD_ALIGNMENT`: suy luận.

Bước hợp lý tiếp theo: **Phần 4: driver và phần cứng khởi tạo trong `board_init_r`** (Xenon MMC, QSPI/`mtd_flash_access`, PCIe/NVMe, USB xHCI, GPIO `mvqz`, I2C DW), hoặc đi sâu **cơ chế console và log đệm** vì nó ảnh hưởng đến cách bạn debug boot. Tôi đề xuất bắt đầu từ trục lưu trữ (MMC, QSPI, NVMe) vì `fatload` và `read_rom` phụ thuộc trực tiếp.