# PHẦN 14 (PART 15): Environment

## 0. Kết luận trước

1. **Environment chỉ nằm trong RAM và luôn khởi động từ mặc định biên dịch sẵn.** Không có CRC, không có bản dự phòng, không có nơi lưu. Mỗi lần reset là mất mọi thay đổi.
2. **`saveenv` và `env save` không tồn tại** (`cmd_nvedit.c:688, 1128, 1200` bị loại bởi `CONFIG_ENV_IS_NOWHERE`).
3. Trạng thái bền của KM **không dùng env**. Nó dùng SPI-flash (vùng BSP/SoftDIP/Secure), sector cài đặt trên SSD/SD, và cờ ở phần cứng (Phần 12).
4. Biến `bootcmd`/`bootargs` được **ghi lại ở `main_loop`** mỗi lần boot, nên giá trị mặc định biên dịch sẵn của chúng gần như không còn tác dụng ở đường KM.

## 1. Vòng đời

```
[compile]  env_default.h  ghép  CONFIG_*  →  const uchar default_environment[]  (chuỗi "k=v\0k=v\0…\0\0")
   │
[board_init_f, trước relocation]                                      env_init  [env_nowhere.c:29]
   gd->env_addr  = &default_environment[0] ;  gd->env_valid = 0
   getenv() lúc này: hàm bị hạn chế  getenv_f() quét TUYẾN TÍNH default_environment, chép vào gd->env_buf[32]
        (chỉ giá trị ≤31 ký tự; dùng bởi init_baud_rate("baudrate"), setup_fdt("fdtcontroladdr"), console_init_f("silent"))
   setenv() lúc này: return 1 (bỏ qua), vì GD_FLG_ENV_READY chưa có
   │
[relocate]
   │
[board_init_r → initr_env]                                            [board_r.c:621]
   should_load_env(): fdtdec_get_config_int(blob,"load-environment",1)  (DT không có /config ⇒ 1)
   env_relocate()      [env_common.c:263]
        gd->env_valid == 0 && ENV_IS_NOWHERE ⇒ set_default_env(NULL)
              in "Using default environment"   (đi qua lớp đệm log KM ⇒ thường không thấy)
              himport_r(&env_htab, default_environment, sizeof, '\0', flags, …)   ← nạp vào BẢNG BĂM (malloc)
              gd->flags |= GD_FLG_ENV_READY
   load_addr = getenv_ulong("loadaddr", 16, load_addr)
   │
[runtime]   getenv(name)  → hsearch_r(FIND, &env_htab)         setenv(name,val) → _do_env_set → hsearch_r(ENTER) ; env_id++
            các nơi KM ghi: misc_init_r_env (ethaddr), quartz_late_init (dtb_name), check_autoboot (bootcmd, bootargs, initrd_high, device_str)
   │
[boot / reset]  môi trường biến mất (chỉ bootargs được truyền cho kernel qua /chosen)
```

## 2. Nội dung mặc định của board

`env_default.h` ghép theo `CONFIG_*` (đã kiểm từng macro trong `armada8k.h`/`mvebu-common.h`):

|Biến|Giá trị|Nguồn|
|---|---|---|
|`bootargs`|`uio_pdrv_genirq.of_id=generic-uio` (`KM_BIZHUB`)|armada8k.h:118|
|`bootcmd`|`ext2load mmc 0:1 $fdt_addr $dtb_name; ext2load mmc 0:1 $kernel_addr Image; booti $kernel_addr - $fdt_addr`|armada8k.h:130 (bị KM ghi đè lúc boot)|
|`bootdelay`|`3`|armada8k.h:132 (`#undef` rồi `#define 3`)|
|`baudrate`|`115200`|mvebu-common.h:322|
|`loads_echo`|`0`|mvebu-common.h:146|
|`rootpath`|`/srv/nfs/`|mvebu-common.h:160|
|`bootfile`|`Image` (KM)|armada8k.h:158|
|`kernel_addr`|`0x800000`|`CONFIG_EXTRA_ENV_SETTINGS` (KM)|
|`initrd_addr`|`0x3000000`|như trên|
|`initrd_name`|`irfs.ubt`|như trên|
|`fdt_addr`|`0x1000`|như trên|
|`ethaddr`, `ipaddr`…|**không có** (chỉ định nghĩa khi `CONFIG_CMD_NET`, đang tắt)|mvebu-common.h:433|

Xác nhận điều tôi từng đánh dấu [S] ở Phần 2: `bootfile=Image` là đúng.

**Biến thêm khi chạy** (ghi từ code KM): `ethaddr` (đặt `00:00:00:00:00:00` rồi thay bằng MAC trong ROM nếu OUI hợp lệ), `dtb_name`, `bootcmd`, `bootargs`, `initrd_high=ffffffff`, `device_str`; `filesize` (mỗi lần `fatload`), `initrd_start/initrd_end` (khi có `BOOTM_STATE_RAMDISK`, không dùng ở `booti`).

Giới hạn: `ENV_SIZE = CONFIG_ENV_SIZE = 0x10000`; `set_default_env` từ chối nếu `sizeof(default_environment) > ENV_SIZE` (in `*** Error - default environment is too large`). Heap cấp thêm `ENV_SIZE` vào `TOTAL_MALLOC_LEN`.

## 3. Cấu trúc dữ liệu

|Cấu trúc|Vai trò|
|---|---|
|`default_environment[]`|Mảng `const` chuỗi `k=v\0…` (nằm trong `.rodata`; **được relocate** cùng ảnh)|
|`env_htab` (`struct hsearch_data`)|Bảng băm `lib/hashtable.c`, `change_ok = env_flags_validate` (kiểm kiểu/quyền biến)|
|`gd->env_addr`, `gd->env_valid`|Con trỏ + cờ hợp lệ (chỉ dùng trước import)|
|`gd->env_buf[32]`|Bộ đệm `getenv` trước khi nhập vào bảng băm|
|`env_id`|Đếm mỗi lần `setenv/delete` để phát hiện thay đổi|
|`GD_FLG_ENV_READY`|Đã nhập vào bảng băm|

Hàm API: `getenv`, `getenv_f`, `getenv_ulong`, `getenv_yesno`, `setenv`, `setenv_ulong`, `setenv_hex`, `setenv_addr`, `getenv_default`, `set_default_vars`, `set_default_env`. (Tên mới `env_get/env_set` **chưa có** ở 2015.01.)

## 4. Lệnh liên quan (đã build)

`printenv`, `setenv`, `run`, `editenv` (`CMD_EDITENV=y`), `env` với các sub-command `print, set, run, default, delete, edit, export, import, grep, callbacks, flags, ask?`; **không có** `saveenv`/`env save`. Chi tiết các lệnh ở Phần 15.

`env import/export` cho phép sao chép môi trường vào/ra bộ nhớ (ví dụ nạp một file cấu hình từ FAT rồi `env import`), nhưng không có nơi lưu bền.

## 5. Cơ chế phụ

- **Callbacks** (`env_callback.c`, `U_BOOT_ENV_CALLBACK(loadaddr, on_loadaddr)` ở image.c:429): khi `loadaddr` đổi thì cập nhật `load_addr`.
- **Flags** (`env_flags.c`): ràng buộc kiểu (decimal/hex/bool/IP/MAC) và quyền ghi (`H_FORCE`, `CONFIG_ENV_OVERWRITE` bật ở mvebu-common.h:90 nên cho đổi các tham số vốn bị khoá).
- **`setenv` trả 1 im lặng khi chưa `ENV_READY`**: các `setenv()` sớm trong `board_init_f` bị bỏ (không có ở KM, vì mọi `setenv` KM nằm sau `initr_env`).

## 6. Failure

|Tình huống|Hành vi|
|---|---|
|Default quá lớn|`set_default_env` in lỗi và **không nhập**: env rỗng, `bootcmd` không có|
|`himport_r` lỗi|`error("Environment import failed…")`|
|Thiếu heap (`_do_env_set`)|`## Can't malloc %d bytes`|
|Tên chứa `=`|`## Error: illegal character '='`|
|`getenv` trước ENV_READY với giá trị > 31 ký tự|`env_buf [%d bytes] too small…` và cắt|

Không có "env corrupt" theo nghĩa CRC (vì không có lưu trữ).

## 7. COMMON CONFUSION

1. **"`setenv` rồi `saveenv` để giữ"** không dùng được: không có `saveenv`.
2. **`printenv` đầu tiên có thể thiếu `bootargs` mặc định:** `check_autoboot` đã ghi đè.
3. **`bootcmd` trong `armada8k.h` không phải cái chạy thật.**
4. **`CONFIG_BOOTARGS`** được dùng **trực tiếp bằng macro** trong `make_bootargs` (nối vào cuối), không đọc từ env.
5. **Thay đổi env trong `cli_loop` không ảnh hưởng `bootcmd` đã chạy** (đã đọc lúc `bootdelay_process`).
6. **`gd->env_buf` 32 byte:** mọi `getenv` **trước** relocation phải giá trị ngắn.

## 8. MENTAL MODEL

```
 default_environment[] (compile-time)  ──(initr_env: himport_r)──►  env_htab (RAM)  ──setenv/getenv──►  code
        ▲                                                            │
   CONFIG_* headers                                                  └─ mất khi reset ; KM ghi bootcmd/bootargs mỗi lần boot
 Trạng thái bền của máy KHÔNG ở đây: SPI-flash + sector cài đặt SSD/SD
```

## 9. Chưa xác minh

- Danh sách `ENV_CALLBACK_LIST_DEFAULT`/`ENV_FLAGS_LIST_DEFAULT` cụ thể.
- Việc `setenv_addr` và `setenv_hex` được dùng ở đâu ngoài `filesize`.

---

Xong Phần 14. Tiếp theo **Phần 15: Command System**: `U_BOOT_CMD`, `.u_boot_list`, `find_cmd`, hush parser, và liệt kê lệnh thật được build.

Tiếp tục Phần 15: đọc `common/command.c` (cmd_process), rồi xem cách lệnh được build và chạy.

Đang xác định lệnh nào thực sự được build: có `common/Kconfig` với các CMD_* mặc định, đọc để biết default y/n.

# PHẦN 15 (PART 16): Command System

## 0. Kết luận trước

1. **Bảng lệnh là một mảng do linker dựng, không có bảng nào được viết tay.** Mỗi `U_BOOT_CMD` đặt một `cmd_tbl_t` vào section `.u_boot_list_2_cmd_2_<tên>`. `find_cmd` quét mảng đó.
2. **Shell là hush**, không phải bộ phân tích đơn giản. `CONFIG_SYS_HUSH_PARSER` được định nghĩa trong `mvebu-common.h:139` và `armada8k.h:289`. Vì vậy `if/then/fi`, `for`, `while`, `;`, `&&`, `||`, `$var`, `$(...)` đều dùng được.
3. **Tập lệnh thật được build khác hẳn `config_cmd_default.h`.** Header này không được include (kiểm tra bằng grep). Chỉ có: các dòng trong defconfig, các `default y` của Kconfig, và các `#define CONFIG_CMD_*` trong `mvebu-common.h`/`armada8k.h`.
4. Lệnh `load/ls/size/save` (cmd_fs.c) và `saveenv` **không** được build. Xem mục 4.
5. KM thêm khoảng 15 lệnh riêng (`doboot`, `quartz`, `qzgpio`, `bspi`, `warp`, `C`, `CB`, `db_sw`, `draw`, `sound`, `initdebugdata`, `MACHINE`, `setInitJpeg`, `clear_bootf`…).

Bốn lệnh KM `C`, `MACHINE`, `db_sw`, `EAD` chỉ tồn tại ở một số biến thể máy (`CONFIG_KM_MACHINE_*`). Tôi chưa dựng được cấu hình đầy đủ của biến thể `EMU800`, nên tôi không khẳng định chúng có mặt trong ảnh đã build. Nhãn [S].

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Cơ chế đăng ký lệnh, phân tích dòng nhập, tra lệnh, gọi hàm `do_xxx`, trả mã lỗi|
|WHY|U-Boot là chương trình tương tác. Mọi thao tác (boot, đọc đĩa, ghi bộ nhớ) đều đi qua "lệnh"|
|WHERE|`include/command.h`, `include/linker_lists.h`, `common/command.c`, `common/cli.c`, `common/cli_hush.c`, `common/cli_readline.c`, `common/main.c`, `common/autoboot.c`|

## 2. HOW: mô hình đăng ký lệnh

```c
struct cmd_tbl_s {                    // command.h:30
    char *name;  int maxargs;  int repeatable;
    int (*cmd)(struct cmd_tbl_s *, int flag, int argc, char * const argv[]);
    char *usage;
    char *help;                       // vì CONFIG_SYS_LONGHELP có định nghĩa
    int (*complete)(...);             // vì CONFIG_AUTO_COMPLETE có định nghĩa
};

#define U_BOOT_CMD(name, maxargs, rep, cmd, usage, help)
   ll_entry_declare(cmd_tbl_t, name, cmd) = {...}
   // → cmd_tbl_t _u_boot_list_2_cmd_2_<name> __aligned(4)
   //   __attribute__((unused, section(".u_boot_list_2_cmd_2_<name>")))
```

Linker script gom `.u_boot_list*` theo thứ tự tên section. `ll_entry_start(cmd_tbl_t, cmd)` và `ll_entry_count` cho địa chỉ đầu và số phần tử của mảng.

Tác dụng phụ quan trọng: **thứ tự lệnh là thứ tự bảng chữ cái theo tên section**, và không có phụ thuộc thứ tự nào giữa các file.

Ngoại lệ: lệnh `?` không dùng macro vì `?` không hợp lệ trong tên ký hiệu. `cmd_help.c:34` khai báo trực tiếp bằng `ll_entry_declare(..., question_mark, cmd)`.

Vì ảnh là PIE (`CONFIG_NEEDS_MANUAL_RELOC` không cần), con trỏ `name`, `cmd`, `usage`, `help` được sửa bởi bước `R_AARCH64_RELATIVE` của `relocate_code` (Phần 3). Không cần `fixup_cmdtable`.

## 3. CALL FLOW

```
Người dùng gõ / bootcmd / env "run"
  │
  ├─ (tương tác)  main_loop → cli_loop()                       cli.c:201
  │      └ parse_file_outer()                                  cli_hush.c:3263   (không bao giờ return)
  │            └ parse_stream_outer(FLAG_PARSE_SEMICOLON)
  │                  └ b_getch → cli_readline(CONFIG_SYS_PROMPT "Marvell>> ")   cli_hush.c:1006
  │                        └ readline_into_buffer (CMDLINE_EDITING: lịch sử, mũi tên, Tab)
  │
  ├─ (tự động)   autoboot_command(s) → run_command_list(s,-1,0) → parse_string_outer(buff, FLAG_PARSE_SEMICOLON)
  │
  └─ (lệnh 'run', 'doboot', script)   run_command(cmd, flag) → parse_string_outer(cmd, SEMICOLON|EXIT_FROM_LOOP [|CONT_ON_NEWLINE])
                                                                                 cli.c:27

hush: parse_stream_outer → parse_stream (tách token, mở rộng $var, `cmd`, $(cmd))
        → run_list_real → run_pipe_real                        cli_hush.c:1513
              └ #ifdef __U_BOOT__: cmd_process(flag, argc, argv, &flag_repeat, NULL)     cli_hush.c:1656
                    ├ find_cmd(argv[0])                                         command.c:517
                    │     └ find_cmd_tbl: so khớp tiền tố tới dấu '.' đầu tiên  command.c:96
                    ├ argc > cmdtp->maxargs  → CMD_RET_USAGE
                    ├ (CMD_BOOTD) chống đệ quy bootd bằng CMD_FLAG_BOOTD
                    ├ cmd_call → (cmdtp->cmd)(cmdtp, flag, argc, argv)          command.c:500
                    ├ *repeatable &= cmdtp->repeatable
                    └ CMD_RET_USAGE → cmd_usage(cmdtp) in "name - usage\n\nhelp"
```

Điểm đáng nhớ:

- **Tra lệnh cho phép viết tắt.** `find_cmd_tbl` duyệt toàn bảng. Khớp đủ tên thì trả về ngay. Nếu chỉ có đúng một lệnh khớp tiền tố thì dùng lệnh đó. Nếu có nhiều lệnh khớp thì trả NULL và in `Unknown command`. Ví dụ `fatl` khớp `fatload` (và `fatls` nếu có, nên `fatl` sẽ mơ hồ).
- **Hậu tố `.b/.w/.l/.q`** (như `md.b`) bị bỏ khi so tên (`strchr(cmd,'.')`), phần sau dấu chấm do chính lệnh xử lý.
- **Số đối số tối đa** là `maxargs`. `CONFIG_SYS_MAXARGS = 32` (`mvebu-common.h:73`). Lệnh có `maxargs` nhỏ hơn (`fatload` = 7, `mmc` = 3 hoặc 7) sẽ in usage nếu thừa đối số.
- **Mã trả về:** `CMD_RET_SUCCESS=0`, `CMD_RET_FAILURE=1`, `CMD_RET_USAGE=-1`. Hush dùng kết quả cho `$?` và cho `&&`/`||`/`if`.
- **Nhận diện `;`:** khi hush chạy ở `FLAG_PARSE_SEMICOLON`, `;` là dấu tách lệnh. Ngoài chế độ đó, `run_pipe_real` in `Unknown command '%s' - try 'help' or use 'run' command` (cli_hush.c:1650).
- **Lặp lệnh (repeatable):** trường `repeatable` cho phép nhấn Enter để lặp (như `md`). Với hush, `parse_stream_outer` xử lý cờ `do_repeat`.
- `run_command_list(cmd, -1, 0)` (dùng bởi `autoboot_command`) không cắt chuỗi, vì hush không sửa chuỗi (`need_buff=0`).

### Ba cách chạy

|Đường|Hàm|Cờ hush|
|---|---|---|
|Dòng lệnh tương tác|`parse_file_outer`|`FLAG_PARSE_SEMICOLON`|
|Chuỗi tự động (bootcmd)|`run_command_list` → `parse_string_outer`|`FLAG_PARSE_SEMICOLON`|
|Lệnh `run VAR`, `doboot`|`run_command(…, flag|CMD_FLAG_ENV)`|

`cli_process_fdt` (cli.c:139) cho phép DT ghi đè `bootcmd` qua node `/config`, và `bootsecure` để chạy lệnh boot không qua hush (`cli_secure_boot_cmd`). DT của board không có `/config`, nên cả hai không tác dụng.

## 4. Lệnh thực sự được build

### 4.1 Nguồn quyết định

|Nguồn|Nội dung|
|---|---|
|defconfig `km_mvebu_quartz_toc_defconfig:11-37`|`CMD_BOOTD, RUN, EDITENV, SAVEENV, MEMORY, MEMTEST, PCI, TIME, MISC, EXT2, LOADS, LOADB, TIMER, MVEBU_MPP, BDI`|
|`common/Kconfig` (`default y`)|`CMD_BOOTM`, `CMD_GO`, `CMD_EXPORTENV`, `CMD_IMPORTENV`, `CMD_CRC32`|
|`mvebu-common.h`|`CMD_BOOTI` (:168, ARM64), `CMD_ELF` (:165), `CMD_USB` (:358, khi `CONFIG_USB`), `CMD_MMC` (:392, khi `XENON_MMC`), `CMD_NVME` (:484, khi `KM_NVME`), `CMD_FAT`/`CMD_EXT2`/`CMD_EXT4_WRITE` (:25-31), `OF_LIBFDT` (:250)|
|`common/mvebu/Kconfig`|`CMD_MVEBU_MISC` (default y), `CMD_MVEBU_MSS_BOOT` (default y)|
|KM|`CONFIG_WARP` (`cmd_warp.c`), `CONFIG_KM_I2S_CDMA` (`cmd_km-sound.c`), `CONFIG_MVQZ_GPIO` (`mvqz_gpio_cmds.c`), file KM luôn `obj-y`|

### 4.2 Danh sách theo nhóm

Lệnh có [V] nếu tôi thấy cả `U_BOOT_CMD` lẫn điều kiện build. Lệnh có [S] nếu điều kiện suy ra từ Kconfig/header nhưng chưa thấy toàn bộ chuỗi phụ thuộc.

|Nhóm|Lệnh|File|
|---|---|---|
|Shell/hush|`help`, `?`, `version`, `exit`, `test` (`[`), `true`, `false`, `showvar`|cmd_help.c, cmd_version.c, cmd_exit.c, cmd_test.c, cli_hush.c:3680 [V]|
|Env|`env` (print/set/run/default/delete/edit/export/import/grep/callbacks/flags/ask), `printenv`, `setenv`, `editenv`, `grepenv`, `askenv`, `run`|cmd_nvedit.c [V]|
|Boot|`bootm`, `booti`, `boot`/`bootd`, `go`, `bootelf`, `bootvx`, `reset`|cmd_bootm.c, cmd_boot.c, cmd_elf.c [V] (`bootz` **không**: CMD_BOOTZ không định nghĩa)|
|Bộ nhớ|`md`, `mm`, `nm`, `mw`, `cp`, `cmp`, `crc32`, `base`, `loop`, `mtest`|cmd_mem.c (`mtest` do CMD_MEMTEST=y) [V]|
|Nạp nối tiếp|`loads`, `loadb`, `loadx`, `loady`, `saves`, `hwflow`|cmd_load.c (obj-y) [V]|
|Thông tin|`bdinfo`|cmd_bdinfo.c [V]|
|Thời gian|`sleep`, `timer`|cmd_misc.c; `time`|
|Đĩa/FS|`fatload`, `fatsize`, `fatls`, `fatinfo`, `ext2load`, `ext2ls`, `mmc`, `mmcinfo`|cmd_fat.c, cmd_ext2.c, cmd_mmc.c [V/S]|
|PCI/NVMe/USB|`pci`, `nvme`, `usb`, `usbboot`|cmd_pci.c, cmd_nvme.c, cmd_usb.c [V]|
|FDT|`fdt`|cmd_fdt.c [S] (OF_LIBFDT)|
|MVEBU|`map`, `mss_boot`|common/mvebu/cmd_misc.c, cmd_mss.c [S]|
|KM (luôn `obj-y`)|`doboot` (main.c:2082), `quartz` (quartz.c:107), `bspi` (qspi_winbond.c:1581), `qzgpio` (mvqz_gpio_cmds.c:160), `warp`, `clear_bootf` (cmd_warp.c), `sound` (cmd_km-sound.c), `initdebugdata` (init_debug.c:217), `draw` (mfp_panel.c:1058), `setInitJpeg` (userdata.c:380), `C`, `CB`, `db_sw`, `MACHINE`|các file KM [V]; riêng `C/CB/db_sw/MACHINE` xem chú ý ở mục 0 [S]|

### 4.3 Những gì KHÔNG có

|Lệnh|Vì sao không|
|---|---|
|`saveenv`, `env save`|cmd_nvedit.c:688/1128/1200: `!defined(CONFIG_ENV_IS_NOWHERE)`|
|`load`, `ls`, `size`, `save` (chung)|cmd_fs.c cần `CONFIG_CMD_FS_GENERIC`. Nó chỉ được định nghĩa bởi `config_distro_defaults.h` và vài board khác, không ở đây|
|`ext4load/ext4ls/ext4size/ext4write`, `fatwrite`, `fatinfo`|CMD_EXT4 và CMD_FAT_WRITE không định nghĩa. `mvebu-common.h:454` nằm trong khối chỉ bật khi có SATA/`MVEBU_PCI` (không đúng cho `PCIE_MV_MSB0125`). Chỉ `CONFIG_CMD_EXT4_WRITE` được định nghĩa ở :25, nhưng không có `CMD_EXT4` thì `cmd_ext4.o` không được biên dịch|
|`echo`, `itest`, `source`|CMD_ECHO, CMD_ITEST, CMD_SOURCE không có trong defconfig; hush có `echo` riêng không (chỉ có qua `cmd_echo.c`, không build)|
|`iminfo`, `imls`, `imxtract`|CMD_IMI/CMD_IMLS/CMD_XIMG không bật|
|mạng (`tftpboot`, `ping`, `dhcp`…)|CMD_NET không bật, `net/` không build|
|`gpio` (chung), `sf`, `i2c`, `spi`|CMD_GPIO, CMD_SF, CMD_I2C, CMD_SPI không bật. KM dùng `qzgpio`/`bspi`. Lưu ý `quartz-sb.h:43` có `CONFIG_CMD_I2C`, nhưng `cmd_i2c.c` chỉ build khi `CONFIG_CMD_I2C` và có driver i2c; `CONFIG_MVEBU_I2C` không được bật ([?] chưa kiểm)|
|`mpp`|`CMD_MVEBU_MPP` yêu cầu `MVEBU_MPP_BUS`, mà defconfig để `# CONFIG_MVEBU_MPP_BUS=y` (comment) nên Kconfig bỏ dòng `CONFIG_CMD_MVEBU_MPP=y`|
|`resetenv`, `remap`, `tsen`|`resetenv` cần `MVEBU_SPI_BOOT/NAND_BOOT/NOR` (không); `remap` cần `MVEBU_MBUS`; `tsen` cần `CMD_MVEBU_TSEN`|

Đính chính Phần 12: tôi đã nhắc `ext2ls/ext2load` là đường đọc ext (đúng), và `ext4load` không có (đúng).

Đính chính Phần 13/14: `fatinfo` **có** trong bảng (cmd_fat.c:92, chỉ cần CMD_FAT). Ở Phần 13 tôi ghi nó là có, nhất quán. `fatwrite` (cmd_fat.c:144) nằm trong `#ifdef CONFIG_FAT_WRITE`, không định nghĩa, nên không có, đúng như đã nói.

## 5. Thông tin nhanh về các lệnh quan trọng cho boot

|Lệnh|`maxargs`|`repeatable`|Ý nghĩa với KM|
|---|---|---|---|
|`fatload`|7|0|Được `bootcmd` sinh bởi `make_bootcmds`|
|`ext2load`|6|0|Đường mặc định trong `CONFIG_BOOTCOMMAND` (chỉ dùng khi `check_autoboot` không ghi đè)|
|`booti`|32|1|`do_booti` → `booti_setup` → `do_bootm_states`|
|`doboot [type] [preview]`|3|1|Đặt `g_action=type`, gọi `check_autoboot()`, chạy `bootcmd`. Với đối số thứ 3, chỉ in `bootargs`/`bootcmd` (xem thử mà không boot)|
|`reset`|1|0|`do_reset` → `reset_cpu` (Phần 11)|
|`warp` / `clear_bootf`|2|0|Resume hibernate / xoá cờ snapshot|

`doboot` rất hữu ích để gỡ lỗi: `doboot 0 0` in `bootargs`/`bootcmd` cho từng kiểu boot mà không chạy.

## 6. Autoboot và cửa sổ ngắt

`main_loop` (KM) → `bootdelay_process(&autoboot)` → `autoboot_command(s)`:

```
bootdelay_process:  s = getenv("bootdelay") ? atoi : CONFIG_BOOTDELAY (=3, armada8k.h:131-132)
                    (OF_CONTROL: fdtdec_get_config_int("bootdelay") → DT không có /config)
                    *autoboot = check_autoboot()     ← KM: quyết định g_action, ghi bootcmd/bootargs
                    s = getenv("bootcmd")
autoboot_command:   if (stored_bootdelay != -1 && s && !abortboot(stored_bootdelay))
                         watchdog theo g_action:  DEF_BOOT → 300s; USB_UPDATE/IISW → 120s; khác → tắt
                         chkGmUpdate()  (chỉ khi DEF_BOOT)
                         run_command_list(s, -1, 0)
abortboot_normal:   (không AUTOBOOT_KEYED) in "Hit any key to stop autoboot: N"
                    isWarpBootMode() → bootdelay = 0
                    watchdog REFRESH 0x11D0 với timeout = bootdelay==0 ? 5 : bootdelay*2
                    CONFIG_ZERO_BOOTDELAY_CHECK: nếu đã có phím trong bộ đệm → abort
                    vòng đếm: mỗi 1s, tstc() → abort
                    abort → sysBspSetAbort(), in "Push Any Key", tắt watchdog
```

Điểm cần biết:

- **Bất kỳ phím nào trên console cũng dừng autoboot** (không có `bootstopkey`). Bộ đệm phím có thể đã chứa ký tự nhiễu trước đó nên hhành vi `ZERO_BOOTDELAY_CHECK` khiến một byte rác trên UART đủ để dừng boot.
- Khi bị ngắt (`abort`), `main_loop` chạy `km_scpi_set_wdt_ca72_ext(END)` rồi `cli_loop()`. Tức là **watchdog tắt** và bạn đang ở prompt `Marvell>>` .
- Nếu `autoboot_command` chạy `bootcmd` và lệnh trả về (lỗi), luồng cũng rơi xuống `cli_loop()` sau khi tắt watchdog.
- `chk_sboot()` chạy **trước** `bootdelay_process` (main.c:2232), nên kiểm tra chip bảo mật xảy ra kể cả khi người dùng ngắt.

## 7. Giao diện dòng lệnh (cli_readline)

- `CONFIG_CMDLINE_EDITING=1` (mvebu-common.h:121): mũi tên, Home/End, lịch sử.
- `CONFIG_AUTO_COMPLETE` (:140): Tab hoàn thành tên lệnh (`cmd_auto_complete`) và biến (`var_complete` cho `setenv/printenv`).
- Prompt `Marvell>>` , prompt tiếp dòng `>` (hush PS2). Bộ đệm `CONFIG_SYS_CBSIZE=1024`.
- `CONFIG_SYS_LONGHELP` bật, nên `help <lệnh>` in đầy đủ.
- Ctrl-C: `ctrlc()` được các lệnh dài (mtest, showvar, `loop`) kiểm tra. `disable_ctrlc()` chỉ dùng trong đường `cli_secure_boot_cmd`.

## 8. COMMON CONFUSION

1. **"Lệnh có trong source thì có trên máy"**: sai. Có 135 file `.c` chứa `U_BOOT_CMD` (281 điểm) nhưng chỉ khoảng 90 lệnh được build. Điều kiện nằm ở `common/Makefile` + `CONFIG_CMD_*`.
2. **`config_cmd_default.h`** là nguồn tập lệnh mặc định ở nhiều board nhưng **không** được include ở đây. Đừng suy luận "board này chắc có `echo`, `saveenv`, `tftp`".
3. **`booti` ≠ `bootm`.** Cả hai có mặt; KM dùng `booti` (Image thô ARM64). `bootz` không có.
4. **`boot` và `bootd` là cùng một hàm** (`do_bootd`, cmd_bootm.c:205,212). Cả hai chạy `bootcmd` từ env qua `run_command`, **không** qua `check_autoboot`. Chỉ `doboot` (KM) mới gọi `check_autoboot`. Do đó lệnh `boot` gõ tay sau khi bị ngắt sẽ chạy `bootcmd` **đã được ghi lúc `bootdelay_process`**, nhưng nếu bạn đổi `g_action` bằng cách khác thì cần `doboot`.
5. **`qzgpio` xuất hiện hai lần trong source** (mvqz_gpio_cmds.c:160 và qspi_winbond.c:1602). Bản thứ hai nằm trong `#ifndef CONFIG_KM_BIZHUB` nên không được biên dịch cùng KM. Chỉ có một lệnh `qzgpio`.
6. **`ext2load` có `repeatable=0`** nhưng `ext2ls` có `1`: Enter lặp `ext2ls`, không lặp `ext2load`. Đây là an toàn (tránh nạp lặp vào RAM).
7. **Chuỗi `bootcmd` chứa `;`** chỉ chạy đúng nhờ hush (`FLAG_PARSE_SEMICOLON`). Với bộ phân tích đơn giản thì cần `run`.

## 9. MENTAL MODEL

```
 gõ phím / bootcmd
     │
     ▼
 [hush] tách token, mở rộng $biến, xử lý if/for/;/&&
     │  argv[]
     ▼
 [find_cmd] quét mảng .u_boot_list_2_cmd_* (linker dựng), khớp tiền tố
     │  cmd_tbl_t*
     ▼
 [cmd_process] kiểm maxargs → cmd_call → do_xxx(cmdtp, flag, argc, argv)
     │  CMD_RET_*
     ▼
 hush: $?, &&, ||, if
```

Mỗi lệnh chỉ là một hàm C có chữ ký cố định, và "thêm lệnh" chỉ là thêm một `U_BOOT_CMD` vào file có trong `obj-y`.

## 10. Chưa xác minh

- Chuỗi phụ thuộc Kconfig đầy đủ cho `CMD_MVEBU_MISC`/`MSS_BOOT` (cần `CONFIG_MVEBU=y` do `TARGET_ARMADA_8K` chọn; chưa mở `arch/arm/Kconfig` để xác nhận `select MVEBU`).
- `CONFIG_KM_MACHINE_*` cụ thể của biến thể EMU800 (quyết định có `C`/`MACHINE`/`db_sw` hay không).
- Tình trạng `cmd_i2c.o` khi `CONFIG_CMD_I2C` được `quartz-sb.h:43` định nghĩa nhưng `CONFIG_MVEBU_I2C` tắt.
- Nội dung `cmd_pcmcia.c` (`obj-y` nhưng `pinit` thường nằm sau `#ifdef`).

---

Xong Phần 15. Tiếp theo **Phần 16: Boot command**: `check_autoboot`/`make_bootcmds`/`make_bootargs`, các `g_action`, `do_bootm_states` và `do_booti`.

# PHẦN 16 (PART 17): Boot Command

## 0. Kết luận và các đính chính

Đường boot thật: `check_autoboot` ghi `bootcmd` và `bootargs` rồi `autoboot_command` chạy `bootcmd`. `bootcmd` là chuỗi `nvme init; fatload ×3; booti`. Trong lúc đọc code tôi phát hiện bốn điểm sai ở các phần trước:

|#|Nội dung cũ|Sự thật (bằng chứng)|
|---|---|---|
|C1|Phần 8: "`CONFIG_SYS_BOOT_RAMDISK_HIGH` không định nghĩa, initrd ở nguyên 0x3000000"|**Có định nghĩa** ở `arch/arm/include/asm/config.h:11` (cùng `CONFIG_LMB`). initrd **bị sao chép** lên vùng cao. Xem mục 5|
|C2|Phần 0/12: coi `fsload_hash_check` (SHA-256 vs EXP_INDEX) như kiểm tra secure-boot|Chỉ được gọi từ `boot_diag()` (machine_setup.c:714) khi ở **chế độ tự chẩn đoán HW** (`uc_HWcheckModeBoot==1 && uc_AutoHWcheckMode==1`). Không nằm trên đường boot thường|
|C3|Phần 12: "D-cache tắt từ `init_nvme` đến `stop_nvme`"|Đúng hơn: mỗi hàm NVMe tắt D-cache lúc vào và **chỉ bật lại khi lỗi hoặc trong `stop_nvme`**. `stop_nvme` chỉ được gọi từ lệnh `nvme stop` (cmd_nvme.c:175). `bootcmd` không gọi nó, nên **D-cache ở trạng thái tắt suốt quá trình nạp và cho đến lúc `cleanup_before_linux`**, và NVMe controller **không bị shutdown** trước khi giao cho kernel [S]|
|C4|Phần 2: `CONFIG_STATIC_RELA` "không có"|Macro **có được định nghĩa** (`asm/config.h:15`, cho ARM64) nhưng không nơi nào kiểm tra `#ifdef CONFIG_STATIC_RELA`, nên **không có tác dụng** [V bằng grep]|

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Chuỗi lệnh đưa kernel Linux ARM64 + DTB + initrd vào RAM rồi nhảy vào kernel|
|WHY|Đây là mục đích cuối của U-Boot (BL33)|
|WHERE|`common/main.c` (`check_autoboot`, `make_bootcmds`, `make_bootargs`, `check_action`, `main_loop`, `do_doboot`), `common/autoboot.c`, `include/configs/armada8k.h:150-284` (mẫu lệnh), `common/cmd_bootm.c` (`do_booti`), `common/bootm.c`, `common/image.c`, `common/image-fdt.c`, `arch/arm/lib/bootm.c`|

## 2. Bộ sinh `bootcmd`: từ macro tới chuỗi lệnh

Mẫu trong `armada8k.h` ghép chuỗi lúc biên dịch:

```
CMD_INITRD_ENV(INIT, DEV, PART, DIR) =
    INIT ";"
    "fatload " DEV " " PART " $kernel_addr " DIR "$bootfile" ";"
    "fatload " DEV " " PART " $fdt_addr "    DIR "$dtb_name" ";"
    "fatload " DEV " " PART " $initrd_addr " DIR "$initrd_name" ";"
    "booti $kernel_addr $initrd_addr $fdt_addr"
```

Trong `CONFIG_CMD_ENV` các tham số `INIT` và `DEV` là `"%s"`. Do đó chuỗi có bốn `%s` (một cho lệnh khởi tạo và ba cho thiết bị). `make_bootcmds` (main.c:1560) điền bằng `sprintf` vào `static char tmp[768]`:

```
isBootFromSD() ? sprintf(tmp, env, "mmc rescan", "mmc","mmc","mmc")
               : sprintf(tmp, env, "nvme init",  "nvme","nvme","nvme")
```

Kết quả của boot thường (SSD):

```
nvme init;
fatload nvme 0:5 $kernel_addr $bootfile;          → 0x800000, Image
fatload nvme 0:5 $fdt_addr    $dtb_name;          → 0x1000,   emu800.dtb
fatload nvme 0:5 $initrd_addr $initrd_name;       → 0x3000000, irfs.ubt
booti $kernel_addr $initrd_addr $fdt_addr
```

`$bootfile=Image`, `$kernel_addr`, `$fdt_addr`, `$initrd_addr`, `$initrd_name` đến từ env mặc định (Phần 14), còn `$dtb_name` do `quartz_late_init` đặt. Các `$var` được hush mở rộng **khi chạy**, không phải khi ghép.

`0:5` là thiết bị 0, phân vùng 5. Vì KM còn dùng `0:A` cho phân vùng thứ 10, số phân vùng phải được đọc theo hệ 16 [S]. Phân vùng theo `fsl_table` (machine_setup.c:452): `0:2` = `/dev/kmsda2` (SUB.img), `0:5` = `kmsda5` (Image, irfs.ubt, emu800.dtb), `0:7` = `kmsda7` (RFS, root của kernel).

## 3. `g_action` → bootcmd / bootargs / autoboot

`g_action` (int, mặc định `DEF_BOOT=0`) được `check_action()` đặt (gọi từ `machine_setup.c:205`, qua `setup_MotionEnable_Power`) hoặc `doboot N`. `check_autoboot()` (main.c:1870) đọc nó:

|g_action|Tên (action.h)|`bootcmd`|root trong bootargs|Trả về `autoboot`|
|---|---|---|---|---|
|0|DEF_BOOT|`CMD_ENV` (0:5, SSD/SD)|`/dev/kmsda7 rootdelay=1`|1 (AUTOBOOT), hoặc 4 (WARP) nếu `checkWarpBootMode()==1`, hoặc 6 (SSD_SUBSET) nếu đang IISW/tải MSC|
|1|USB_BOOT|`usb reset; fatload usb 0 …`|`/dev/sdc3 rootdelay=10`|2|
|2|USB_UPDATE|như trên, thư mục `FWxxxx/`, có/không `reset` tuỳ `sysBspGetAbort()`|`/dev/sdc3`|5|
|3|DEF_IISW|`IISWCMD` (0:2)|`kmsda7`|6|
|4|BOOT_LINE|`CMD_ENV`|`kmsda7`; `term_state` bị ép = 1|7|
|5|BOOT_ERR|`ERRCMD` (0:2)|`kmsda7`|6|
|7|HW_CHECKMODE|`CMD_ENV`|`kmsda7`|9|
|8|SPEEDCHANGE|`CMD_ENV`|`kmsda7`|10|
|9|DEF_UPDATE|`IISWCMD_UPDATE` (0:A)|`kmsda7`|6|
|11, 12|BACKUP/RESTORE|`IISWCMD` (0:2)|`kmsda7`|6|
|6, 13|BOOT_INIT/BDERASE|**không xử lý ở đây**: `main_loop` gọi `initialize_card_main()`/`bderase_card_main()` (không quay lại), trước đó|||

Với `DEF_UPDATE` (9), `check_autoboot` còn nạp `INDEX` từ `0:A` bằng `do_fat_fsload` để xác nhận kiểu máy. Sai kiểu, thiếu INDEX hoặc lỗi thì `g_action = BOOT_ERR`.

Mọi nhánh đều `setenv("initrd_high","ffffffff")`.

### `make_bootargs` (main.c:1591)

```
bootargs = "boottyp=%d <root=… rootdelay=…> [nopanel ]<console=…> \
            TerminalState=%d MachineInfo=%X MachineNo=%X SerialNo=%s [Capacity=1] \
            IISW=%d FwDir=%s DevStr=%s IntegrityState=%d uio_pdrv_genirq.of_id=generic-uio"
```

- `console`: `TERM_STATE_0` → `console=tty1 loglevel=1`. Ngược lại `term_sel ? console=ttyS0,115200 : console=ttyS5,115200`.
- `nopanel`: khi `getPanelConnect() != ON` hoặc env `nopanel=1`.
- `IntegrityState`: DIP mềm `0x413` (`getInergrity()`); bit này chỉ **truyền** cho Linux, U-Boot không tự kiểm.
- `FwDir` là chuỗi `FW9020`/`FW9022`/`FW9023` tuỳ macro máy [S].
- Đệm tĩnh `chBtEv[768]` với `sprintf` không giới hạn kích thước, nên có rủi ro tràn [?].
- Khối `#ifndef CONFIG_OF_LIBFDT` (nhét hddinfo/paneltype vào bootargs) **không** biên dịch. Do có `OF_LIBFDT`, các tham số này đi qua DTB (Phần 8: `fdt_fixup_memory_banks` bản KM).

## 4. Vòng đời trong `main_loop` (KM)

```
main_loop
 ├ cli_init (hush start), run_preboot_environment_command
 ├ g_action == BOOT_INIT/BOOT_BDERASE → tắt WDT, initialize/bderase_card_main (vô hạn)
 ├ chk_sboot()                              chip bảo mật: sai → "boot failed." + treo (Phần 21)
 ├ s = bootdelay_process(&autoboot)         → check_autoboot() → getenv("bootcmd")
 ├ autoboot == WARP hoặc (LINE && checkWarpBootMode):
 │     set_boot_type_to_main_routine; sound_stop; nvme_opal_cmd_send(0) (nếu SSD)
 │     panelKey!=0x16 && !isDisableSuperWarp_ErrorOccurred → warp_boot(SuperWarp SSID)  ┐ thành công thì
 │     ngược lại warp_clear_bootf                                                        │ không return
 │     setStartupBypeNomalWarp; warp_boot(NormalSSID)                                    ┘
 │     nếu return → autoboot = check_autoboot_DEF_BOOT_case(autoboot)
 ├ warp_clear_bootf(Normal); (g_action != DEF_UPDATE) warp_clear_bootf(SuperWarp)
 ├ set_boot_type_to_main_routine(autoboot)  ghi loại boot vào sector Warp (SSD, sector 65)
 ├ autoboot == NOT_AUTOBOOT (0)               → in "Functional restriction 1"
 │ autoboot == NOT_AUTOBOOT_FOR_BOOTDEV (3)   → "Functional restriction 2"
 │ ngược lại → cli_process_fdt (không tác dụng) → autoboot_command(s)
 ├ km_scpi_set_wdt_ca72_ext(END)              (chỉ tới đây khi bootcmd trả về)
 └ cli_loop()                                 prompt "Marvell>> "
```

Điểm lưu ý:

- Giá trị `autoboot = 8` (`NOT_AUTOBOOT_NOTHING_BOOT_IMAGE`) **không** bị chặn: mã chỉ so với 0 và 3, nên 8 vẫn chạy `autoboot_command` với `bootcmd` hiện có [V bằng đọc code].
- `set_boot_type_to_main_routine` chỉ thực hiện đọc–sửa–ghi 1 sector qua `km_warp_setting_read/write` (mã cứng `DEF_KM_WARP_SETTINGINFO_ADR = 65`). Nó cần NVMe/SD đã sẵn sàng. Nếu `km_warp_setting_read` không trả 1 thì bỏ qua. Ở đường này nó cũng ghi PCI info, panel type, DIP 177, v.v.
- Watchdog 300s bật ở `autoboot_command` khi `g_action == DEF_BOOT`. Chỉ tắt nếu `bootcmd` trả về. Nếu kernel sống thì phần mềm trên kernel phải tự xử lý tiếp (Phần 22).

## 5. `booti` từng bước

`do_booti` (cmd_bootm.c:725):

```
do_booti(argv = booti $kernel_addr $initrd_addr $fdt_addr)
 ├ booti_start
 │   ├ do_bootm_states(BOOTM_STATE_START)  → bootm_start:
 │   │     memset(&images,0); images.verify = getenv_yesno("verify")   → env chưa có → -1 (khác 0)
 │   │     boot_start_lmb: lmb_add(bootm_low, bi_dram[0].size), arch_lmb_reserve
 │   │       (giữ [sp-4KB .. hết bank 0], tức là toàn bộ vùng trên stack: U-Boot + heap + TLB + VRAM…)
 │   ├ images->ep = strtoul(argv[0], 16) = 0x800000
 │   ├ booti_setup:  kiểm magic "ARM\x64" (0x644d5241); image_size==0 → giả 16MiB
 │   │     dst = bi_dram[0].start + text_offset; nếu ep != dst → memmove(dst, ep, image_size), ep = dst
 │   ├ lmb_reserve(ep, image_size)
 │   └ bootm_find_ramdisk_fdt:
 │        boot_get_ramdisk  (image.c:872)  → xem 5.1
 │        boot_get_fdt      (image-fdt.c)  → kiểm header FDT tại 0x1000, set_working_fdt_addr
 ├ bootm_disable_interrupts:  disable_interrupts() (stub) ; usb_stop()  (vì CMD_USB)
 └ do_bootm_states(OS_PREP | OS_FAKE_GO | OS_GO)     (images.os.os = IH_OS_LINUX)
      ├ boot_fn = do_bootm_linux (bootm_os_get_boot_func)
      ├ OS_PREP → boot_prep_linux                                   arch/arm/lib/bootm.c:226
      │     ft_len != 0 → image_setup_linux                          image.c:1241
      │        ├ boot_fdt_add_mem_rsv_regions (từ /memreserve/ của DTB)
      │        ├ boot_ramdisk_high   ← sao chép initrd (mục 5.2)
      │        ├ boot_relocate_fdt   ← chuyển DTB lên đỉnh cửa sổ 16MB
      │        └ image_setup_libfdt  ← /chosen (bootargs, initrd-start/end), fdt_fixup_memory_banks, KM props
      ├ OS_FAKE_GO: chỉ khi CONFIG_TRACE (không có)
      └ OS_GO → do_bootm_linux → boot_jump_linux                     :284
             announce_and_cleanup: in "Starting kernel ...", I2C event nếu DIP 144, bootstage_report
                                   → cleanup_before_linux (tắt I-cache, tắt D-cache/MMU, invalidate)
             (CONFIG_ARMV8_PSCI: is_psci_enabled() = EL3 only → false)
             do_nonsec_virt_switch → armv8_switch_to_el2 (no-op ngoài EL3)
             kernel_entry(images->ft_addr, 0, 0, 0)          ← không quay lại
```

`do_bootm_states` lần hai **không** có `BOOTM_STATE_RAMDISK/FDT`, nên việc sao chép initrd và chuyển DTB xảy ra trong `image_setup_linux`, không phải trong `do_bootm_states` (khối tại bootm.c:599-617 không dùng ở `booti`).

### 5.1 initrd phải là uImage kiểu ramdisk

`booti … $initrd_addr …` truyền địa chỉ **không có `:size`**. `boot_get_ramdisk`:

- `CONFIG_SUPPORT_RAW_INITRD` không được định nghĩa cho board này. Nó chỉ nằm trong `config_distro_defaults.h` và vài board khác. Vì vậy nhánh raw bị loại.
- `CONFIG_IMAGE_FORMAT_LEGACY` được bật qua `config_fallbacks.h:87` (khi không có FIT signature bắt buộc). `CONFIG_FIT` không được định nghĩa cho board này [S, chưa đọc lại chuỗi điều kiện].
- Do đó `irfs.ubt` **phải là legacy uImage** (header 64 byte, magic `0x27051956`, type `IH_TYPE_RAMDISK`, os Linux, arch `IH_ARCH_DEFAULT`). Tên `.ubt` khớp cách nghĩ này. Sai định dạng thì in `Wrong Ramdisk Image Format` và `booti` trả 1.
- `image_get_ramdisk` luôn kiểm magic và **CRC header**. Vì `images.verify = -1` (khác 0, do env `verify` không tồn tại và `getenv_yesno` trả -1) nên còn chạy **CRC32 dữ liệu** của toàn bộ initrd mỗi lần boot (in `Verifying Checksum ... OK`). Đây là một khoản thời gian đáng kể nếu initrd lớn, và chạy khi D-cache đang tắt (C3).
- `rd_start` = địa chỉ dữ liệu (sau header 64 byte), `rd_end = rd_start + size`.

### 5.2 `boot_ramdisk_high` và giá trị `ffffffff` (C1)

```c
initrd_high = simple_strtoul("ffffffff", NULL, 16);   // ulong 64-bit ⇒ 0x00000000FFFFFFFF
if (initrd_high == ~0)  initrd_copy_to_ram = 0;        // ~0 (int) mở rộng thành 0xFFFF…FFFF ⇒ KHÔNG bằng
```

Trên ARM64, `ulong` rộng 64 bit, nên `"ffffffff"` **không** bằng `~0`. Ý định của KM ("không sao chép initrd") không đạt được. Thay vào đó:

```
*initrd_start = lmb_alloc_base(lmb, rd_len, 0x1000, 0xFFFFFFFF);   // vùng trống cao nhất dưới 4GiB
memmove_wd(initrd_start, rd_data, rd_len, CHUNKSZ);
```

Vùng trống cao nhất là ngay dưới `sp - 4KB` (vì `arch_lmb_reserve` đã giữ mọi thứ từ stack tới hết bank 0). Vậy initrd bị chép từ 0x3000000 tới một địa chỉ ngay dưới vùng của U-Boot (khoảng dưới 0x3FE0_0000; con số chính xác phụ thuộc `sp` lúc chạy [S]). Kernel nhận địa chỉ mới qua `/chosen`.

Hệ quả:

- Thêm một lần `memmove` cỡ initrd (D-cache tắt nên chậm).
- Nếu initrd lớn hơn khoảng trống thì `ramdisk - allocation error`.

### 5.3 DTB

`boot_relocate_fdt` (Phần 8): luôn chuyển tới đỉnh cửa sổ `CONFIG_SYS_BOOTMAPSZ` (16MB, tính từ `bootm_low`), cộng `CONFIG_SYS_FDT_PAD`, trừ khi `fdt_high` đặt. Vì `Image` đã bị chép xuống `bi_dram[0].start + text_offset` và bị `lmb_reserve` trước, nên nếu `Image` chiếm gần hết 16MB thì DTB không còn chỗ (`device tree - allocation error`, rồi `image_setup_linux` lỗi và `boot_prep_linux` **gọi `hang()`**: `FDT creation failed! hanging...`). Mục [?] cũ vẫn còn: chưa biết kích thước `Image` thật.

## 6. Bản đồ bộ nhớ khi boot

```
0x00001000  DTB nạp (fdt_addr)              ← vùng TEXT_BASE cũ, đã trống sau relocation
0x00700000  EXP_INDEX   (chỉ chế độ tự chẩn đoán)
0x00800000  Image nạp (kernel_addr)         ← booti_setup memmove → bi_dram[0].start + text_offset
0x02000000  CPU_RELEASE_ADDR / SYS_LOAD_ADDR / PRE_CON_BUF_ADDR (chồng nhau, Phần 2)
0x03000000  irfs.ubt nạp (initrd_addr)      ← boot_ramdisk_high chép lên vùng cao dưới stack
…<16MB      DTB sau relocation (đỉnh cửa sổ)
~0x3FExxxxx initrd sau relocation
0x3FE10000  VRAM;  0x3FFF0000 TLB;  U-Boot/heap/gd/stack ngay dưới
```

Lưu ý `IMAGE_LOAD_ADDRESS 0x800000` của `boot_diag` trùng `kernel_addr`. Chế độ chẩn đoán nạp từng ảnh đè lên vùng này, điều đó an toàn vì nó chạy trong `misc_init_r`, trước `bootcmd`.

## 7. `doboot` và `boot`

- `doboot N` (main.c:2043): `g_action = N` (1→USB_BOOT, 2→USB_UPDATE, khác thì lấy N, in `boottype N booting !!!`), gọi `check_autoboot()`, rồi `run_command(getenv("bootcmd"), 0)`. Không qua `abortboot`, **không bật watchdog**, **không chạy `warp_boot`**, **không cập nhật sector Warp**. Với tham số thứ ba thì chỉ in `bootargs`/`bootcmd` (xem thử).
- `boot`/`bootd` chạy `bootcmd` hiện hành, không gọi lại `check_autoboot`.

## 8. Failure và rẽ nhánh

|Tình huống|Hành vi|
|---|---|
|`nvme init` lỗi|hush `;` vẫn chạy lệnh kế tiếp. Các `fatload` lỗi (in `** Bad device`…). `booti` kiểm magic tại RAM 0x800000|
|`fatload Image` lỗi nhưng RAM còn dữ liệu cũ|`booti` có thể khởi động ảnh cũ nếu RAM còn giữ (không có `&&` nối) [S]|
|Không có magic|`Bad Linux ARM64 Image magic!` → `booti` trả 1 → `bootcmd` xong → WDT END → `cli_loop`|
|`initrd` sai định dạng / CRC|`Wrong Ramdisk Image Format` / `Bad Data CRC`, `booti` trả 1|
|DTB sai|`Could not find a valid device tree`|
|Lỗi từ `image_setup_linux`|`hang()` (không có prompt)|
|`booti` lỗi trong `do_bootm_states`|`hwcDispLoadDiag()` vẽ màn hình chẩn đoán (KM) rồi trả về|

## 9. COMMON CONFUSION

1. **`CONFIG_BOOTCOMMAND` ≠ bootcmd thực.** Giá trị `ext2load mmc 0:1 …` bị `check_autoboot` ghi đè trước khi chạy (Phần 14 đã nói).
2. **`initrd_high=ffffffff` ≠ "không chép".** Trên ARM64 nó khiến initrd bị chép (C1).
3. **`booti` không dùng `bootargs` từ đối số**. Nó lấy env `bootargs` qua `image_setup_libfdt` → `fdt_chosen`.
4. **`fsload_hash_check` không phải cổng bảo mật boot thường** (C2).
5. **USB bị dừng trước kernel (`usb_stop`)** nhưng **NVMe thì không** (C3).
6. **Warp**: `warp_boot` trả về khi thất bại. Khi đó luồng rơi xuống boot thường, không dừng lại.

## 10. MENTAL MODEL

```
check_action → g_action ──► check_autoboot ──► bootcmd, bootargs (chuỗi ghép sẵn theo bảng)
                                     │
        main_loop ── bootdelay(3s, phím bất kỳ ngắt) ──► run_command_list(bootcmd)
                                     │
      nvme init → fatload Image/DTB/initrd → booti
                                     │ magic ✓; memmove Image; initrd (uImage, CRC) → chép cao; DTB → ≤16MB
                                     ▼
                       cleanup_before_linux → kernel_entry(fdt, 0,0,0)
```

## 11. Chưa xác minh

- Địa chỉ thực của initrd/DTB sau relocation (cần `sp` runtime), kích thước `Image`, `irfs.ubt`.
- `text_offset` của Image (mặc định thường 0x80000; chưa thấy Image).
- `CONFIG_SYS_SDRAM_BASE` (nếu không định nghĩa thì dùng `bi_dram[0].start`).
- Có `CONFIG_FIT` hay không (ảnh hưởng nhánh FIT trong `boot_get_ramdisk`). Cần kiểm chứng lại chuỗi `config_fallbacks.h:80-91`.
- `sprintf` không giới hạn trong `make_bootargs`/`make_bootcmds` (`SerialNo` tới từ ROM).

---

Xong Phần 16. Tiếp theo **Phần 17: Image format**: legacy uImage, FIT (có hay không), raw arm64 Image, DTB, tool `mkimage`.

# PHẦN 17 (PART 18): Image Format

## 0. Kết luận

Ba loại "ảnh" đi qua đường boot và mỗi loại có bộ kiểm tra khác nhau:

|Ảnh|Định dạng|Kiểm tra ở U-Boot|Nơi kiểm|
|---|---|---|---|
|`Image` (kernel)|**ARM64 Image thô** (header 64 byte của Linux)|Chỉ magic `"ARM\x64"`|`booti_setup` (cmd_bootm.c:656)|
|`irfs.ubt` (initrd)|**Legacy uImage, type RAMDISK, arch ARM64**|magic, CRC header, CRC dữ liệu (vì `verify=-1`), os/arch/type|`image_get_ramdisk` (image.c:361)|
|`emu800.dtb` (kernel DTB)|**FDT thô** (`0xd00dfeed`)|`fdt_check_header`|`boot_get_fdt` / `boot_relocate_fdt`|

Và có hai điều "không có":

- **FIT không được build** (`CONFIG_FIT` không định nghĩa ở defconfig, header hay Kconfig default). Không có `image-fit.o`, không có `FIT_SIGNATURE`, tức là **không có chữ ký ảnh** ở U-Boot.
- **Không có kiểm tra tính toàn vẹn/chữ ký của `Image` và DTB** trên đường boot thường. Bảo vệ nằm ở chỗ khác (Phần 21).

## 1. Bản đồ định dạng

### 1.1 ARM64 Image (`Image`)

`struct Image_header` (cmd_bootm.c:634), theo `Documentation/arm64/booting.txt`:

```
+0   u32 code0       lệnh thực thi (thường "b _head" hoặc MZ)
+4   u32 code1
+8   u64 text_offset offset nạp so với đầu RAM (LE)
+16  u64 image_size  kích thước hiệu dụng (LE)
+24  u64 res1..res4  (flags, reserved)
+56  u32 magic       0x644d5241 ("ARM\x64")
+60  u32 res5
```

`booti_setup`:

1. `ih->magic != le32_to_cpu(0x644d5241)` → `Bad Linux ARM64 Image magic!`, trả 1.
2. `image_size == 0` → giả định 16MiB và **ghi vào chính header trong RAM** (`ih->image_size = 16<<20`).
3. `dst = bi_dram[0].start + text_offset`; nếu `ep != dst` thì `memmove(dst, ep, image_size)`, `ep = dst`.

Ghi chú:

- **Không kiểm tra alignment 2MB, không kiểm tra `flags` (endianness/page size), không kiểm tra `image_size` so với RAM**, không giới hạn theo `CONFIG_SYS_BOOTM_LEN`. Giá trị `image_size` từ header do bên tạo ảnh quyết định, và `memmove` tin nó hoàn toàn.
- Vì `memmove` dùng kích thước từ header, nếu header bị hỏng (nhưng magic đúng) thì có thể chép quá mức. Kích thước file thật (`filesize`) không được đối chiếu với `image_size`.
- `Image` được `fatload` vào 0x800000, còn kernel chạy ở `text_offset` (thường 0x80000), tức là thấp hơn. `memmove` (chồng lấn OK) chuyển xuống.
- **Không hỗ trợ nén**. Chỉ `Image` thô. `Image.gz` sẽ bị từ chối ở magic.

### 1.2 Legacy uImage (initrd `irfs.ubt`)

`image_header_t` (image.h:264), 64 byte, **big-endian**:

```
+0   be32 ih_magic  0x27051956
+4   be32 ih_hcrc   CRC32 của header (trường này = 0 khi tính)
+8   be32 ih_time
+12  be32 ih_size    kích thước dữ liệu
+16  be32 ih_load    địa chỉ nạp
+20  be32 ih_ep      entry point
+24  be32 ih_dcrc    CRC32 của dữ liệu
+28  u8 ih_os        5 = Linux
+29  u8 ih_arch      22 = ARM64   (IH_ARCH_DEFAULT vì CONFIG_ARM64)
+30  u8 ih_type      3 = RAMDISK
+31  u8 ih_comp      0 none / 1 gzip / 2 bzip2 / 3 lzma / 4 lzo
+32  u8 ih_name[32]
```

`image_get_ramdisk` (image.c:361):

1. `image_check_magic` → `Bad Magic Number`.
2. `image_check_hcrc` → `Bad Header Checksum`.
3. `image_print_contents` (in "Image Name / Created / Image Type / Data Size / Load Address / Entry Point").
4. `if (verify)`: `image_check_dcrc` (`crc32_wd`, `CHUNKSZ_CRC32`), lỗi → `Bad Data CRC`. `verify = getenv_yesno("verify")` = **-1** khi env chưa có, và -1 là "đúng", nên CRC dữ liệu **luôn chạy** (Phần 16).
5. `image_check_os(LINUX) && image_check_arch(IH_INITRD_ARCH) && image_check_type(RAMDISK)`, sai thì `No Linux <arch> Ramdisk Image`.

`IH_INITRD_ARCH = IH_ARCH_DEFAULT = IH_ARCH_ARM64` (bootm.c:42, `asm/u-boot.h:49`). Do đó initrd phải tạo bằng dạng `mkimage -A arm64 -O linux -T ramdisk -C none …`. Nếu tạo với `-A arm` (mặc định thường gặp) sẽ bị từ chối. `ih_comp` không được dùng vì initrd được kernel giải nén, U-Boot chỉ chép `ih_size` byte.

Không kiểm `ih_load`/`ih_ep` cho ramdisk. `rd_start = ảnh + 64`.

### 1.3 Device tree (`emu800.dtb`)

FDT chuẩn (`fdt_header`: magic `0xd00dfeed`, `totalsize`, offsets, `version`, big-endian). Có hai DTB khác nhau, dễ nhầm:

||DTB **điều khiển** của U-Boot|DTB **của kernel**|
|---|---|---|
|Nguồn|`arch/arm/dts/armada-quartz-emu800.dts` (Phần 7), `dtc` biên dịch trong build|`emu800.dtb` trên phân vùng 5 của SSD/SD, **không nằm trong repo này** [S]|
|Vào đâu|Nhúng vào ảnh U-Boot (`.dtb.init.rodata`, `OF_EMBED`), `gd->fdt_blob`|`fatload … $fdt_addr $dtb_name` → 0x1000|
|Dùng để|Cấu hình driver U-Boot (MMC, QSPI, PCIe, xHCI…)|Truyền cho kernel qua x0|

`boot_get_fdt` (image-fdt.c:222) với `argv[2]=$fdt_addr`:

- `genimg_get_format` (image.c:734): thử legacy trước (magic `0x27051956`), rồi `fdt_check_header` (kích hoạt vì `CONFIG_OF_LIBFDT`, dòng 743). **DTB thô trả về `IMAGE_FORMAT_FIT`** dù FIT không build (tên dễ gây hiểu nhầm, đúng như comment ở image.c:728). Nhánh `case IMAGE_FORMAT_FIT` khi `CONFIG_FIT` không định nghĩa chỉ in `## Flattened Device Tree blob at …`.
- Cũng chấp nhận DTB đóng gói trong legacy uImage (type FLATDT), bằng `image_get_fdt`, nhưng KM dùng DTB thô.
- Trả `of_size = fdt_totalsize(blob)`.

Sau đó (Phần 16): `boot_relocate_fdt` → `fdt_open_into(blob, dst, of_len)` mở rộng thêm `CONFIG_SYS_FDT_PAD`, `image_setup_libfdt` (image-fdt.c:465):

```
fdt_chosen(blob)            tạo /chosen, chép bootargs từ env "bootargs" (+ stdout-path nếu có)
arch_fixup_fdt(blob)        fdt_fixup_memory_banks + thuộc tính KM (Phần 8)
[ft_board_setup]            KHÔNG (CONFIG_OF_BOARD_SETUP không định nghĩa)
fdt_fixup_ethernet(blob)    dùng env ethaddr/eth1addr → /aliases ethernetN (nếu có)
lmb_free/lmb_reserve blob   (ép blob về u32: chỉ đúng khi blob < 4GiB, ở đây <16MB nên ổn)
fdt_shrink_to_minimum
fdt_initrd(blob, start, end) ghi linux,initrd-start/end vào /chosen
ft_verify_fdt (weak → 1)
```

Nếu bất kỳ bước nào lỗi: in `- must RESET the board to recover.` và `boot_prep_linux` gọi `hang()`.

## 2. Bộ nhận diện định dạng và mã liên quan

```
genimg_get_format(buf):   legacy (magic 0x27051956)?  →  FDT header?  →  [Android nếu bật]  →  INVALID
```

|Hằng|Giá trị|
|---|---|
|`IH_MAGIC`|0x27051956|
|`IH_OS_LINUX`|5|
|`IH_ARCH_ARM64`|22|
|`IH_TYPE_KERNEL/RAMDISK/FLATDT`|2 / 3 / 8|
|`IH_COMP_NONE/GZIP/BZIP2/LZMA/LZO`|0/1/2/3/4|

`bootm_os.c:407` `boot_os[]`: `IH_OS_LINUX → do_bootm_linux` (vì `CONFIG_BOOTM_LINUX=1` ở `config_defaults.h:13`), `IH_OS_U_BOOT → do_bootm_standalone`, `IH_OS_QNX → do_bootm_qnxelf` (vì `CMD_ELF`). Không có NetBSD, RTEMS, OSE, Plan9, VxWorks (cần thêm `CONFIG_BOOTM_*`) hay INTEGRITY.

## 3. Lệnh `bootm` (có nhưng không dùng ở đây)

`bootm` có trong ảnh (Kconfig `CMD_BOOTM default y`), nhưng bootcmd dùng `booti`. Với legacy uImage kernel, `bootm_load_os` giải nén `ih_comp`:

|Nén|Hỗ trợ|Ghi chú|
|---|---|---|
|gzip|**Có** (`CONFIG_GZIP=1`, `config_defaults.h:19`)|`gunzip`, giới hạn `unc_len = CONFIG_SYS_BOOTM_LEN = 20MiB` (mvebu-common.h:118)|
|bzip2, lzma, lzo|Không (không định nghĩa)||

`CONFIG_SYS_BOOTM_LEN` chỉ giới hạn `unc_len` ở bootm. Nó **không** ràng buộc `booti` dù comment ghi "max Image size" (mvebu-common.h:117).

`CONFIG_GZIP_COMPRESSED` (armada8k.h:46) chỉ bật `lib/gzip.o` (bên nén, cho `gzwrite`/`zip`), không phải bên giải nén [V bằng lib/Makefile:25-26].

Với ARM64, kernel legacy uImage sẽ cần `ih_load`/`ih_ep` đúng và `IH_ARCH_ARM64`. Không có ảnh nào như vậy trong đường boot hiện tại.

## 4. Công cụ tạo ảnh

`tools/Makefile:55`: `hostprogs-y += dumpimage mkimage` (luôn build) và `mkenvimage` (dòng 52). `fit_info`, `fit_check_sign` chỉ khi `FIT_SIGNATURE`, do đó không có. Repo **không chứa** script đóng gói firmware (tar, INDEX, `irfs.ubt`); phần đó nằm ngoài (`Subset\...` gợi ý ở thư mục cạnh, chưa đọc).

Cách tạo initrd cho chương trình này (suy ra từ các kiểm tra):

```
mkimage -A arm64 -O linux -T ramdisk -C none -n <name> -d <cpio.gz> irfs.ubt
```

Tôi ghi [S] vì không thấy script thật của KM.

## 5. Dữ liệu chảy qua các định dạng

```
SSD p5:  Image (raw)  ─fatload→ 0x800000 ─booti_setup memmove→ bi_dram[0].start+text_offset  → lmb_reserve
         irfs.ubt(uImage) ─fatload→ 0x3000000 ─CRC/hdr─→ rd_start=+64 ─memmove→ cao (lmb_alloc_base) → /chosen initrd-*
         emu800.dtb(FDT)  ─fatload→ 0x1000    ─fdt_check_header→ boot_relocate_fdt → ≤16MB → fdt_chosen/fixup → x0
```

## 6. COMMON CONFUSION

1. **`IMAGE_FORMAT_FIT` ≠ FIT.** Với `CONFIG_OF_LIBFDT`, DTB thô cũng mang nhãn này.
2. **"`booti` verify ảnh"**: không. Chỉ magic. CRC chỉ áp cho initrd.
3. **`.ubt`** không phải một định dạng riêng: nó là uImage kiểu ramdisk (suy ra từ mã, [S]).
4. **Arch phải là `arm64`**: `-A arm` cho initrd thất bại với `No Linux ARM64 Ramdisk Image`.
5. **Hai file DTB cùng tên `emu800`** nhưng khác vai trò (U-Boot điều khiển và kernel).
6. **`image_size` trong header Linux** là độ dài `memmove`, không phải kích thước file.

## 7. MENTAL MODEL

```
       ┌─────────── định dạng ───────────┐
Image  │ ARM64 header 64B → magic only   │→ chép về text_offset → CHẠY
initrd │ uImage 64B(BE) → magic+CRC+arch │→ chép cao            → đưa địa chỉ vào /chosen
DTB    │ FDT (BE)      → header hợp lệ   │→ chép ≤16MB + vá     → x0
                 FIT / chữ ký / nén: KHÔNG có
```

## 8. Chưa xác minh

- Hình thức tạo `irfs.ubt` (ngoài repo).
- Nội dung kernel DTB (không có trong repo), gồm `/aliases`, `/reserved-memory` của ATF.
- `image_size` và `text_offset` thực của `Image`.
- Có `CONFIG_ANDROID_BOOT_IMAGE` hay không (Makefile: `obj-$(CONFIG_ANDROID_BOOT_IMAGE)`; không thấy định nghĩa, coi như không).

---

Xong Phần 17. Tiếp theo **Phần 18: Kernel handoff**: trạng thái CPU/MMU/cache/EL khi nhảy, `cleanup_before_linux`, PSCI/spin-table, vùng nhớ bảo lưu.

Grep "smp_kick_all_cpus" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src)

11 lines of output

Grep "armv8_switch_to_el2|switch_el|current_el|el2|EL2|CurrentEL" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8, glob: *.S)

36 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\start.S](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 56-270)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\include\configs\armada8k.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 313-382)

# PHẦN 18 (PART 19): Kernel Handoff

## 0. Kết luận

U-Boot giao cho kernel đúng theo giao thức khởi động ARM64 của Linux: **x0 = địa chỉ vật lý DTB, x1–x3 = 0, MMU tắt, D-cache tắt, ngắt che, nhảy tới `Image` tại `bi_dram[0].start + text_offset`**. Mọi bước dọn dẹp chỉ gồm tắt cache và (khi cần) chuyển EL. **U-Boot không dừng NVMe, MMC, PCIe hay watchdog của KM** trước khi nhảy.

Có một đính chính nhỏ cho Phần 17:

|#|Cũ|Đúng|
|---|---|---|
|C5|"bootm chỉ giải nén gzip; lzo không hỗ trợ"|`CONFIG_LZO` **được định nghĩa** ở `armada8k.h:358` (trong khối `KM_BIZHUB`), nên `lib/lzo/` được build (`lib/Makefile:12`) và nhánh `IH_COMP_LZO` trong `bootm.c:354` hoạt động. KM bật nó cho ảnh màn hình khởi động của Warp. bzip2/lzma vẫn không có. Không ảnh hưởng `booti`|

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Khoảnh khắc chuyển quyền điều khiển từ U-Boot sang Linux|
|WHY|Linux giả định CPU ở trạng thái "sạch" theo `Documentation/arm64/booting.txt`|
|WHERE|`arch/arm/lib/bootm.c` (`boot_jump_linux`, `announce_and_cleanup`, `do_nonsec_virt_switch`), `arch/arm/cpu/armv8/cpu.c` (`cleanup_before_linux`), `cache_v8.c`, `transition.S`, `start.S`|

## 2. Trình tự thật ngay trước khi nhảy

```
do_bootm_linux(OS_GO)  ──► boot_jump_linux(images, flag)          arch/arm/lib/bootm.c:284
   kernel_entry = images->ep                                       (đã là bi_dram[0].start + text_offset)
   announce_and_cleanup(fake=0):
        printf("\nStarting kernel ...\n\n")                        (đi qua bộ đệm log KM → thường không hiện)
        THRG_getSoftDipSw(0xA1): bit0 → i2c_write(PSCPU_SLAVE_ADDR, PSCPU_REG_EVENT, KernelStart)   (KM)
        bootstage_mark_name / bootstage_report()                   (CONFIG_BOOTSTAGE_REPORT có)
        cleanup_before_linux():
             disable_interrupts()             (stub, Phần 3)
             icache_disable(); invalidate_icache_all()
             dcache_disable()                 flush_dcache_all + __asm_flush_l3_cache (KM) + invalidate TLB,
                                              rồi xoá SCTLR.C và SCTLR.M
             invalidate_dcache_all()
   (CONFIG_ARMV8_PSCI: is_psci_enabled() == (current_el()==3) → false dưới ATF ⇒ setup_psci() bị bỏ)
   do_nonsec_virt_switch():
        smp_kick_all_cpus()     weak trong start.S:220 (SGI 0 qua GICD nếu GICv2/v3 định nghĩa)
        flush_dcache_all()
        armv8_switch_to_el2()   transition.S: chỉ làm việc khi đang ở EL3, còn lại là no-op
        (CONFIG_ARMV8_SWITCH_TO_EL1: không định nghĩa)
   kernel_entry(images->ft_addr, NULL, NULL, NULL)                 ← không quay lại
```

Đối chiếu với giao thức Linux:

|Yêu cầu|Trạng thái|Nguồn|
|---|---|---|
|x0 = PA của DTB, 8-byte aligned|`ft_addr` do `lmb_alloc_base(…, 0x1000, …)` nên căn 4KB|image-fdt.c:156|
|x1, x2, x3 = 0|NULL|bootm.c:306|
|MMU tắt|Có (`SCTLR.M` bị xoá)|cache_v8.c:140|
|D-cache tắt, đã đồng bộ về RAM|Có. Đã flush trước khi tắt|cache_v8.c:136-139|
|I-cache: tuỳ|Tắt|cpu.c:32|
|Ngắt DAIF che|Không do U-Boot chủ động mà kế thừa từ ATF; U-Boot không bao giờ bật ngắt (Phần 3)|[S]|
|Kernel ở EL2 hoặc EL1|**Cùng EL với U-Boot** (không đổi). Mức EL do ATF chọn cho BL33, ngoài repo|[S]|
|Image tại `base + text_offset`, base căn 2MB|`bi_dram[0].start = 0`; `text_offset` từ header|cmd_bootm.c:670|

### Về việc tắt cache có an toàn không

- **Nhánh MMC/SD** (D-cache đang bật): `dcache_disable` (bản KM) chạy `flush_dcache_all()` **trước**, rồi mới xoá `C|M`. Dữ liệu `Image`, initrd, DTB đã ra RAM trước khi tắt.
- **Nhánh NVMe** (D-cache đã tắt từ lúc `nvme init`, C3 ở Phần 16): `dcache_disable` thoát sớm vì `SCTLR.C` đã 0. Toàn bộ dữ liệu nạp và `memmove` đã đi thẳng ra RAM (MMU tắt nên mọi truy cập là Device-nGnRnE), nên `invalidate_dcache_all()` sau đó chỉ vứt các dòng không còn dirty. An toàn nhưng chậm.

Điều này dẫn tới một hệ quả đáng ghi nhận: `dcache_disable()` xoá **cả `SCTLR.M`**, nên mỗi hàm NVMe gọi `nvme_dcache_disable()` sẽ tắt MMU của U-Boot. **Sau `nvme init` U-Boot chạy với MMU tắt** cho tới khi nhảy vào kernel. Khi MMU tắt, mọi truy cập dữ liệu là Device, nên truy cập không căn hàng sẽ sinh alignment fault. Code C của U-Boot (memmove, CRC32, `fatload`) chạy được trong thực tế, nhưng không có `-mstrict-align` trong `arch/arm/Makefile`/`config.mk` (grep rỗng) nên đây là một điểm dễ vỡ nếu compiler đổi [?].

## 3. Những gì kernel nhận qua DTB

Từ `image_setup_libfdt` (Phần 17) + `arch_fixup_fdt`:

|Nút / thuộc tính|Giá trị|Nguồn|
|---|---|---|
|`/chosen/bootargs`|Env `bootargs` (chuỗi từ `make_bootargs`)|`fdt_chosen`|
|`/chosen/linux,initrd-start/end`|Địa chỉ **sau khi chép cao** (Phần 16 mục 5.2)|`fdt_initrd`|
|`/memory/reg`|`bi_dram[]`: bank 0 (0…), bank 1 tại 0x2_0000_0000, bank 2 (remap)|`fdt_fixup_memory_banks`|
|`/memory` thuộc tính KM|hddinfo, boottype, hddexistence, sd_existence, paneltype/code/option/size, utilitykey, **vramaddr, vramsize**|bản KM của `arch_fixup_fdt` (Phần 8)|
|`/aliases` ethernet|theo env `ethaddr` (`fdt_fixup_ethernet`)|image-fdt.c:496|

Không có: `ft_board_setup` (không `CONFIG_OF_BOARD_SETUP`), `/reserved-memory` cho ATF hay cho U-Boot (do U-Boot không thêm), PSCI node (dựa vào DTB kernel).

`vramaddr`/`vramsize` (VRAM 0x1E0000 ở 0x3FE10000) là cách U-Boot báo cho kernel về framebuffer đã dựng sẵn (Phần 9). Kernel nên giữ nguyên vùng này để màn hình khởi động không bị xoá.

**Vùng nhớ mà U-Boot không bảo vệ**: chính vùng chứa U-Boot (đỉnh RAM), ATF (0x10000000, 4MB), DDR training (0x10400000) và mailbox SCPI (0x7ff00000, `CONFIG_SCPI_MAILBOX_BASE`). Việc kernel tránh chúng phụ thuộc hoàn toàn vào DTB kernel trên SSD (`/reserved-memory`), không nằm trong repo [?]. Đây là điểm cần kiểm khi debug lỗi kernel ngẫu nhiên.

## 4. CPU phụ (secondary cores)

- Dưới ATF, các lõi phụ do ATF quản lý (PSCI). U-Boot chỉ chạy trên lõi chính.
- `start.S:113-118` vẫn có vòng `slave_cpu: wfe; ldr x0,[CPU_RELEASE_ADDR]; cbz; br x0`, với `CPU_RELEASE_ADDR = SDRAM_BASE + 0x2000000` (mvebu-common.h:253). Đây là cơ chế spin-table và chỉ chạy nếu một lõi phụ nhảy vào `_start` của U-Boot, điều không xảy ra theo luồng ATF. Không có DTB `enable-method = "spin-table"` do U-Boot cấp.
- `lowlevel_init` (start.S:170) thoát ngay vì `CurrentEL != EL3` ("ATF has done all low level tasks").
- `smp_kick_all_cpus()` gửi SGI 0 nếu `CONFIG_GICV2/V3` định nghĩa. Nếu không có định nghĩa thì hàm là no-op. Chưa xác minh macro GIC nào bật (`include/configs/`, `asm/arch-mvebu/`) [?].

## 5. Trạng thái phần cứng bị bỏ lại cho kernel

|Khối|U-Boot làm gì trước khi nhảy|Hệ quả cho kernel|
|---|---|---|
|USB xHCI|`usb_stop()` (`bootm_disable_interrupts`, CMD_USB)|Sạch|
|NVMe|**Không** shutdown (`stop_nvme` chỉ có ở lệnh `nvme stop`). Hàng đợi I/O còn trong RAM của U-Boot|Linux `nvme` driver sẽ reset controller lúc probe (CC.EN=0) [S]. Nếu kernel DTB dùng lại vùng RAM đó trước khi reset, controller có thể DMA vào đó|
|PCIe RC|Giữ nguyên, đã enumerate|Kernel enumerate lại|
|Xenon SDHCI (SD/MMC)|Không dừng|Driver kernel init lại|
|UART|Giữ cấu hình; `console=ttyS0/ttyS5,115200`|Kernel lấy đúng baud|
|GPIO, pinmux|Giữ trạng thái (kể cả HRESET_REQ, POWER_OFF_PNL…)|Có thể ảnh hưởng nguồn nếu kernel mong đợi mặc định|
|Watchdog SCPI (CA72)|Còn chạy với hạn 300s cho boot thường|Phải có phần mềm phía kernel/ứng dụng cấp lại hoặc tắt. Xem Phần 22|
|Timer, GIC|ATF quản lý|Không đổi|
|VBAR_ELx|Còn trỏ vào vector của U-Boot|Kernel tự đặt VBAR ngay ở đầu|
|Sound (I2S/CDMA)|`sound_stop()` đã gọi trong `main_loop` trước bootcmd|Sạch|

## 6. Đường Warp (hibernate resume): không đi qua `booti`

`warp_boot()` không dùng `booti`. Nó chạy `WARP` driver, nạp ảnh hibernate từ SPI/SD/NVMe rồi dùng `warp_prepare_boot()` (armada8k.h:349):

```
smp_kick_all_cpus(); flush_dcache_all();
*(volatile u32 *)(0xF0630000 + 0) = 0x100;      // Timer 1: WARP_TIMER_BASE
```

rồi nhảy vào kernel đã ngủ đông. Thanh ghi `0xF0630000` là Timer 1 của SoC; giá trị 0x100 được ghi trước khi nhảy (ý nghĩa bit chính xác chưa tra [?]). Chi tiết ở Phần 31/34.

## 7. Failure

|Điều kiện|Kết quả|
|---|---|
|`image_setup_linux` lỗi|`FDT creation failed! hanging...` và `hang()` (vòng vô hạn, WDT 300s sẽ reset máy)|
|`ep`/DTB sai|Kernel không boot, không phản hồi: U-Boot không có cách biết. WDT 300s reset|
|Kernel treo sớm|Vì `console` chọn theo `term_state`, có thể không thấy gì (Phần 9)|
|Kernel không nhận được `bootargs`|do `/chosen` lỗi; kernel dùng cmdline mặc định trong DTB|

Vì watchdog 300s vẫn chạy trong `DEF_BOOT` (`autoboot_command` đặt `REFRESH 0x11D2, 300`), toàn bộ đoạn "U-Boot → kernel → ứng dụng cấp lại watchdog" phải hoàn tất trong 300s, nếu không SoC reset. Đó là cơ chế phục hồi lỗi boot chính của máy [S, cần xác nhận ở Phần 22/40].

## 8. COMMON CONFUSION

1. **"U-Boot chuyển kernel sang EL1"**: không. Nó giữ nguyên EL; chỉ khi ở EL3 mới đổi sang EL2.
2. **"`Starting kernel ...` luôn hiện"**: bị bộ đệm log KM nuốt trên máy sản xuất (Phần 9).
3. **"PSCI do U-Boot cài"**: dưới ATF thì không (`is_psci_enabled()` false).
4. **"initrd ở 0x3000000 khi vào kernel"**: đã bị chép cao (Phần 16).
5. **"U-Boot reserve bộ nhớ cho ATF trong DTB"**: không, việc này nằm ở DTB của kernel.
6. **"`dcache_disable` chỉ tắt cache"**: nó cũng tắt MMU; điều này giải thích vì sao NVMe chạy với MMU tắt.

## 9. MENTAL MODEL

```
booti ─ chuẩn bị (Image, initrd, DTB) ─►  announce_and_cleanup
                                              ├ flush D-cache → RAM
                                              ├ tắt I-cache, D-cache, MMU
                                              └ (EL3 → EL2 nếu cần)
                                          ─►  br Image_entry   ; x0 = DTB
[Không dừng: NVMe, MMC, PCIe, GPIO; WDT 300s vẫn chạy]
```

## 10. Chưa xác minh

- EL thực khi vào U-Boot (ATF cấu hình).
- `CONFIG_GICV2/V3` có hay không (quyết định `smp_kick_all_cpus`).
- DTB kernel thực (reserved-memory, PSCI, chosen, aliases).
- Ý nghĩa ghi `0x100` vào `0xF0630000` trước khi resume Warp.
- Nhánh MMU-off + code C không căn hàng trong đường NVMe.

---

Xong Phần 18. Tiếp theo **Phần 19: Network**: `net/`, driver `e1000`, `CMD_NET`, MAC (`ethaddr`, `BspBootMacAddrReadWrite`).

Grep "e1000|CONFIG_E1000|CONFIG_CMD_NET|CONFIG_NET|CONFIG_NETDEVICES|eth_register|CONFIG_PHYLIB|CONFIG_MII|CONFIG_USB_HOST_ETHER|CONFIG_USB_ETHER" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\net\Makefile)

3 lines of output

Grep "eth_register|CONFIG_CMD_NET|CONFIG_NET\b|eth_initialize|ethaddr|eth_getenv_enetaddr|eth_setenv_enetaddr|NETDEVICES" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\net\e1000.c)

1 line of output

Grep "ethaddr|eth_initialize|CONFIG_CMD_NET|BspBootMacAddr|MAC" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\machine_setup.c)

10 lines of output

Grep "board_eth_init|cpu_eth_init|eth_initialize|net_init|CONFIG_CMD_NET" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common, glob: board_r.c)

6 lines of output

Grep "^#\s*(if|ifdef|ifndef|endif)\b.*CMD_NET|^#include <net.h>|^#include <malloc.h>|^#include <pci.h>|^#include "e1000.h"|CONFIG_E1000" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\net\e1000.c)

16 lines of output

Grep "drivers/net|net/|libs-y|libs-\$\(CONFIG_(NET|CMD_NET)" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\Makefile)

35 lines of output

Grep "\+S:|S:" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\Makefile)

1 line of output

Grep "\+S:" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\doc, glob: README.kconfig)

15 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\net\Makefile](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Grep "eth_register|CONFIG_CMD_NET|CONFIG_NET|eth_initialize|^#else|^#endif" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\include\net.h)

24 lines of output

Grep "e1000_initialize|board_eth_init|eth_initialize|eth_register|CONFIG_CMD_NET|CONFIG_E1000" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu)

4 lines of output

Grep "e1000_initialize" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.[ch])

6 lines of output

Grep "DEVEL_BOARD|config QUARTZ|PCIE_MV_MSB0125" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: Kconfig*)

61 lines of output

Grep "DEVEL_BOARD|E1000|QUARTZ" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\include, glob: configs/{armada8k.h,quartz-sb.h,mvebu-common.h})

7 lines of output

Grep "obj-|ifdef|ifndef|endif" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\Makefile)

3 lines of output

Grep "int eth_register|eth_register\s*\(" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.c)

21 lines of output

Grep "config MVEBU$|select MVEBU|imply|config TARGET_ARMADA_8K" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\Kconfig)

13 lines of output

Grep "e1000|E1000|eth_|net" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\net, glob: Kconfig)

11 lines of output

Grep "config MVEBU\b|select MVEBU\b|default y if.*MVEBU|config ARCH_MVEBU|CONFIG_MVEBU\b" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {Kconfig,arch/arm/Kconfig,arch/arm/cpu/armv8/Kconfig,arch/arm/cpu/armv8/armada8k/Kconfig,board/mvebu/Kconfig,configs/km_mvebu_quartz_toc_defconfig})

9 lines of output

Grep "MVEBU\b" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\Kconfig)

1 line of output

Grep "define CONFIG_MVEBU\b|CONFIG_MVEBU$|CONFIG_MVEBU\s" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\include\configs, glob: {armada8k.h,mvebu-common.h,quartz-sb.h})

1 line of output

Grep "gc-sections|ffunction-sections|fdata-sections" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {Makefile,config.mk,arch/arm/config.mk,arch/arm/Makefile})

2 lines of output

Grep "KEEP|u_boot_list|gc" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\u-boot.lds)

4 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\machine_setup.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 356-425)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\command.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 564-683)

Grep "void fdt_fixup_ethernet|fdt_get_alias|ethaddr|eth%daddr|ethernet|local-mac-address|mac-address" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\fdt_support.c)

11 lines of output

Grep "CONFIG_USB_XHCI\b" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.{h,c,mk})

11 lines of output

Grep "U_BOOT_CMD|^int sysBsp|^static int|^int do_|sysBspBootDo|cmd_tbl_t\s+\w+\[|Terminal|CONFIG_CMD_BSP" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\command.c)

81 lines of output

Grep "sysBspBootDo_X|sysBspBootDo_" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.[ch])

18 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\command.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 701-800)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\command.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 896-1065)

# PHẦN 19 (PART 20): Network

## 0. Kết luận

**Ảnh U-Boot này không có mạng.** Không có TFTP, BOOTP/DHCP, NFS, ping, `eth_initialize`, không có driver Ethernet nào chạy, và không có `netconsole`. Toàn bộ liên quan đến "mạng" chỉ còn là **địa chỉ MAC** mà U-Boot đọc từ SPI-flash và ghi vào DTB cho Linux. Máy nạp firmware từ SSD/SD/USB, không từ mạng.

Ghi nhận thêm hai đính chính (không thuộc riêng Phần 19):

|#|Nội dung|Sự thật|
|---|---|---|
|C6|Phần 15 bỏ sót lệnh sản xuất của KM|`common/command.c:1014-1053` đăng ký thêm **`X`, `x`, `Y`, `y`, `Z`, `z`** (lệnh một chữ cái). Chúng nằm trong `#if defined(KM_BIZHUB) && !defined(CONFIG_SPL_BUILD)` nên luôn có trong ảnh này|
|C7|Phần 15 để mở "`map`/`mss_boot` có build không?"|**Có.** `CONFIG_MVEBU` là `default y` khi `SYS_VENDOR="mvebu"` (`board/mvebu/Kconfig:5-8`); `CMD_MVEBU_MISC` và `CMD_MVEBU_MSS_BOOT` `default y`. `DEVEL_BOARD` cũng hiệu lực (defconfig `+S:` là "Normal và SPL" theo `doc/README.kconfig:80`), nên `devel-board.o` được build **cùng** `quartz.o` (`board/mvebu/armada8k/Makefile:20-21`)|

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Ngăn xếp mạng của U-Boot (`net/`, driver NIC, lệnh mạng)|
|WHY|Ở board thường dùng để tải kernel/khởi tạo qua mạng. Máy in KM không cần|
|WHERE|`net/Makefile`, `drivers/net/`, `board/mvebu/common/eth_init.c`, `include/configs/mvebu-common.h:432-446,398-430`, `common/board_r.c:703-727,779-783`|

## 2. Vì sao "không có mạng"

|Lớp|Điều kiện|Trạng thái|
|---|---|---|
|Lệnh (`tftpboot`, `bootp`, `dhcp`, `nfs`, `ping`…)|`CONFIG_CMD_NET` và các CMD_* phụ|**Không định nghĩa** (không có trong defconfig; `config_cmd_default.h` không được include, Phần 15)|
|Core `net/`|`obj-$(CONFIG_CMD_NET)` cho `arp.o bootp.o eth.o net.o tftp.o` (net/Makefile:10-21)|Không build|
|`eth_initialize(gd->bd)`|`board_r.c:779-783` trong `#ifdef CONFIG_CMD_NET`|Không được gọi|
|`board_eth_init`|`board/mvebu/common/Makefile:21`: `obj-$(CONFIG_CMD_NET) += eth_init.o`|Không build|
|Biến môi trường mạng mặc định (`ipaddr`, `serverip`, `netmask`, `gatewayip`, `ethaddr` mặc định `00:00:00:00:51:81`, `ethprime`)|`mvebu-common.h:433-446` trong `#ifdef CONFIG_CMD_NET`|Không có (khớp Phần 14)|
|`netconsole`|`CONFIG_NETCONSOLE`|Không định nghĩa|

### Code mạng có được biên dịch nhưng không chạy

Do các header vẫn định nghĩa vài macro, một số file `.c` **được biên dịch** nhưng bị loại ở bước liên kết:

- `mvebu-common.h:408-409` (và 427-428): `#ifdef CONFIG_DEVEL_BOARD → #define CONFIG_E1000` (trong khối PCIe, vì `CONFIG_PCIE_MV_MSB0125` có). Khi đó `drivers/net/Makefile:22` biên dịch `e1000.o`, có tham chiếu `eth_register` (net/eth.c, **không build**).
- `mvebu-common.h:351-357`: khi `CONFIG_USB` mà `UHOST_XHCI_CUSTOMIZE` không định nghĩa thì `CONFIG_USB_HOST_ETHER` và `CONFIG_USB_ETHER_ASIX` được bật (xem Phần 20 về điều kiện này).
- Vì sao không lỗi liên kết: `arch/arm/config.mk:16-17` đặt **`--gc-sections`** và `-ffunction-sections -fdata-sections`. Các hàm không có đường gọi từ `_start` bị loại, và `ld` không báo "undefined reference" cho chúng. Riêng `.u_boot_list*` được giữ bằng `KEEP(*(SORT(.u_boot_list*)))` (`u-boot.lds:71`), nên lệnh trong mọi file đã biên dịch **vẫn sống** dù hàm ở nơi khác không được tham chiếu. Đó là lý do "có `U_BOOT_CMD` trong file nào đã biên dịch thì lệnh đó có mặt" (đúng cho Phần 15).

Kết luận: `e1000.o` và `usb/eth` là mã chết. Không có NIC được khởi tạo, dù board có thể có card PCIe.

## 3. MAC address: đường duy nhất còn liên quan

```
SPI-flash (vùng "machineinfo", 2 bản A/B, Phần 12)
     │  read_rom()  →  pRom->mac[0][0..5]
     ▼
misc_init_r_env()   (machine_setup.c:365, gọi từ misc_init_r)
     ├ setenv("ethaddr","00:00:00:00:00:00")        ← mặc định
     ├ in "MAC address(%s) is invalid" nếu OUI không hợp lệ
     └ nếu 3 byte đầu ∈ { 00:20:6B , 00:50:AA , 08:00:86 } → setenv("ethaddr", "XX:XX:XX:XX:XX:XX")
     ▼
boot: image_setup_libfdt → fdt_fixup_ethernet(blob)        image-fdt.c:496 → fdt_support.c:584
     ├ dùng "ethaddr", rồi "eth1addr", "eth2addr"… (sprintf "eth%daddr")
     ├ tìm alias "ethernet%d" trong DTB kernel
     └ do_fixup_by_path(path, "mac-address" và "local-mac-address", 6 byte)
```

Ghi chú:

- `read_rom()` thất bại → in `Read ERROR.(SPI FLASH Parameter)`, thoát và **bỏ qua luôn việc đọc machineInfo/machineNo/serial** (`return` ở dòng 385). Các biến `gMachineInfo`, `gMachineNo`, `gSerialNo` giữ giá trị 0 và bị đưa vào `bootargs` (Phần 16) như `MachineInfo=0 MachineNo=0 SerialNo=`.
- Kiểm tra OUI chỉ áp cho `mac[0]` (MAC "MFP controller"). Các giá trị OUI này là ba tiền tố mà KM chấp nhận [S: tôi tin chúng thuộc Konica Minolta nhưng không xác nhận trong repo].
- Nếu DTB kernel **không có** alias `ethernetN`, việc vá không có tác dụng. Kernel dùng nguồn MAC khác. Không thể xác nhận vì DTB kernel không có trong repo [?].
- Bộ nhớ `ethaddr` chỉ ở RAM (env NOWHERE). Nguồn thật là SPI-flash.

## 4. Các lệnh sản xuất về MAC/serial (C6)

Đăng ký trực tiếp bằng `U_BOOT_CMD` (command.c:1014-1053), tất cả `repeatable = 0`:

|Lệnh|maxargs|Hàm|Tác dụng|
|---|---|---|---|
|`Y`|1|`sysBspBootDo_MacAddrRead`|In `MFP Controller MAC Address:XXXXXXXXXXXX` từ vùng machineinfo|
|`y <12 hex>`|2|`sysBspBootDo_MacAddrWrite`|Ghi MAC (kiểm độ dài = 12, mỗi ký tự là hex) bằng `THRG_setMachineInfo`, chờ `BOOT_UDELAY_TIME`, in `MAC Address setting : OK`|
|`X`|1|`sysBspBootDo_X`|Xoá MAC: ghi 0x00 sau khi in "`OverWrite in ROM? Y(y) or N ==>`" và đợi phím (`getc()`)|
|`Z`|1|`sysBspBootDo_Z`|In serial 16 ký tự; ký tự ngoài 0x20–0x7E → `No serial numberd` và trả -1|
|`z <16 ký tự>`|2|`sysBspBootDo_z`|Ghi serial (kiểm độ dài = 16 và ASCII in được) và **xoá luôn snapshot Warp** (SuperWarp + Normal, cả SD và SSD, qua `km_warp_init_device` + `warp_clear_bootf`)|
|`x`|1|`sysBspBootDo_Delete_Serial`|Ghi lại serial `"0000000000000000"`|

Điểm cần lưu ý (đối với an toàn vận hành, dùng cho Phần 21, 29):

- **Không có xác thực.** Bất kỳ ai có console (UART) và bấm phím ngắt `bootdelay` đều có thể chạy `y`/`z`/`X`/`x` và ghi đè MAC/serial trong SPI-flash. Trên máy sản xuất, việc nhấn phím và thấy prompt phụ thuộc bộ đệm log KM (Phần 9) và GPIO/SoftDIP quyết định `termFlg`.
- **`y` và `z` không kiểm `argc`.** `maxargs` chỉ chặn thừa đối số. Gõ `y` hoặc `z` một mình khiến `argv[1] == NULL`, và `strlen(argv[1])` sẽ truy cập địa chỉ 0 [V bằng đọc code: command.c:718, 905; hậu quả là data abort → handler ngoại lệ, Phần 28].
- `z` xoá snapshot Warp có chủ đích (đổi serial thì ảnh ngủ đông cũ không còn hợp lệ).
- `X` cần xác nhận, `x` không cần.

## 5. Mạng ở tầng khác (ngoài U-Boot)

|Thành phần|Vai trò|
|---|---|
|Linux (kernel trên SSD/SD)|Toàn bộ mạng thực nằm ở đây: dùng `ethaddr` đã đưa qua DTB, tải firmware qua mạng (nếu có) là việc của phần mềm máy in|
|Cập nhật firmware|Không qua mạng ở U-Boot; qua USB memory (`USB_UPDATE`/`USB_BOOT`) hoặc IISW/SSD (Phần 31)|
|Card PCIe Ethernet (E1000 trong DTB điều khiển của devel board)|Chỉ ảnh hưởng tài liệu cấu hình; không có driver hoạt động|

## 6. Failure

|Tình huống|Hành vi|
|---|---|
|MAC trong ROM sai OUI hoặc chưa ghi|In `MAC address(...) is invalid.`, `ethaddr = 00:00:00:00:00:00`; kernel nhận MAC 0 hoặc tự sinh MAC ngẫu nhiên (tuỳ driver Linux)|
|`read_rom()` NULL|Toàn bộ `MachineInfo/No/SerialNo` = 0|
|Gõ lệnh mạng (`tftp`, `ping`…)|`Unknown command 'tftp' - try 'help'`|
|Nhấn `y`/`z` không đối số|Truy cập NULL (xem trên)|

## 7. COMMON CONFUSION

1. **"Có `e1000.c` nghĩa là có mạng"**: không. Nó bị `--gc-sections` loại.
2. **"`ethaddr` cho U-Boot dùng"**: không có ai dùng trong U-Boot. Nó chỉ để **vá DTB**.
3. **`CONFIG_ETHADDR 00:00:00:00:51:81`** trong `mvebu-common.h:439` là mặc định devel board, **không** có hiệu lực vì khối bị `CMD_NET` loại.
4. **`x` và `X` khác nhau** (serial và MAC); **`y`/`Y`** cũng vậy. Tên chỉ khác chữ hoa/thường và phép so khớp lệnh nhạy chữ hoa/thường (`strncmp`).
5. **Lệnh `Z` vs `z`** dễ gõ nhầm: `z` **ghi** (và xoá Warp), `Z` chỉ đọc.

## 8. MENTAL MODEL

```
Không có: NIC driver ─ eth core ─ tftp/bootp ─ lệnh mạng
Có:       SPI-flash(MAC) ─ misc_init_r_env ─ env "ethaddr" ─ fdt_fixup_ethernet ─ DTB kernel ─ Linux
Nhà máy:  console ─ y/z/X/x ─ THRG_setMachineInfo ─ SPI-flash
```

## 9. Chưa xác minh

- DTB kernel có alias `ethernet0/1` hay không.
- Chủ sở hữu của các OUI (ghi [S]).
- Kích thước/bố cục chính xác `BOOT_ROM_*` (offset MAC/serial trong machineinfo).
- `CONFIG_USB_XHCI` (khác `CONFIG_USB_XHCI_HCD`) có được Kconfig định nghĩa không. Nó quyết định `UHOST_XHCI_CUSTOMIZE`, tức số cổng root (3 hay 4) và có bật `USB_HOST_ETHER` không (Phần 20).

---

Xong Phần 19. Tiếp theo **Phần 20: USB**: `xhci-mvebu.c`, PHY, `usb_storage`, quét USB trong `misc_init_r`, `check_usb_port`, `USB_BOOT`/`USB_UPDATE`.

Grep "USB_XHCI|USB_MV|USB_EHCI|config USB" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\usb, glob: Kconfig)

81 lines of output

Grep "obj-|ifdef|endif" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\usb\host\Makefile)

38 lines of output

Grep "obj-|MV_USB|USB_MV" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\usb\phy\Makefile)

4 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\usb\host\xhci-mvebu.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\usb_storage.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 1481-1570)

Grep "usb_init|usb_stor_scan|usb_stop|check_usb_port|USB_MAX|isBootFromSD|usb_reset|mmc_exist|usb_dev_index|usb_started" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\machine_setup.c)

44 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\machine_setup.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 151-270)

Globpattern: "drivers/usb/phy/phy-*"

Found 4 files

Grep "usb3|usb@|xhci|mvebu,usb3|gpio-vbus|utmi|phy" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\dts, glob: {armada-quartz-emu800.dts,apn-806-z1.dtsi,quartz-sb.dtsi,quartz-iris-sb-0.dtsi})

8 lines of output

Grep "uc_TempUtilityKey|uc_HWcheckModeBoot\s*=|uc_HWcheckModeBoot;|GPIO_PORT_UTILITY|UTILITY_KEY" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.[ch])

26 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\machine_setup.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 270-329)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\machine_setup.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 697-766)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\usb\phy\phy-utm-mv.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\dts\armada-quartz-emu800.dts](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 51-90)

Grep "^int |^void |^static int |^static void |define|writel|readl|mdelay|udelay" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\usb\phy\phy-usb3-mv.c)

41 lines of output

Grep "board_usb_vbus_init|board_usb_get_enabled_port_count|usb_lowlevel_init|xhci_hcd_init" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.c)

21 lines of output

Grep "^int usb_init|usb_lowlevel_init|CONFIG_USB_MAX_CONTROLLER_COUNT|usb_started|usb_hub_reset|usb_scan_devices|usb_stop|dev_index|usb_max_devs|USB_MAX_DEVICE|mdelay|CONFIG_SYS_USB_EVENT_POLL|usb_reset_root_port|USB_STOR_MAX|usb_alloc_new_device|USB_MAX_HUB|usb_new_device|KM_BIZHUB|maxchild|timeout|usb_disable_asynch" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\usb.c)

61 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\usb.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 39-198)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\usb\host\xhci.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 971-1060)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\include\configs\quartz-sb.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 15-74)

Grep "mv_i2c|mvtwsi|I2C_MV|MVEBU_I2C|quartz|km_i2c|SYS_I2C|obj-y|obj-\$\(CONFIG_(SYS_I2C|HARD_I2C|I2C_MV|MVEBU_I2C|I2C_MV)" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\i2c\Makefile)

23 lines of output

Grep "i2c_set_bus_num|i2c_write\s*\(|CONFIG_I2C_MULTI_BUS|CONFIG_SYS_I2C|CONFIG_HARD_I2C|CONFIG_I2C_MV|CONFIG_SYS_MAX_I2C_BUS|CONFIG_MVEBU_I2C|CONFIG_SYS_I2C_SLAVE|CONFIG_SYS_I2C_SPEED" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\include\configs, glob: {armada8k.h,quartz-sb.h,mvebu-common.h})

21 lines of output

# PHẦN 20 (PART 21): USB

## 0. Kết luận và đính chính

USB trong U-Boot này là **bộ điều khiển xHCI duy nhất** (Marvell, `usb3@d00d0000`) chạy ở chế độ **USB 2.0** (PHY UTM), dùng cho **USB memory** để cập nhật/khởi động firmware (`USB_BOOT`, `USB_UPDATE`, line card, initialize card, board-erase). **Bình thường nó không được bật.** USB chỉ khởi tạo khi giữ **Utility Key** lúc bật nguồn hoặc khi cờ line-card đang đặt.

|#|Đính chính|Bằng chứng|
|---|---|---|
|C8|Phần 1/2 nói "USB được auto-init trong `misc_init_r`"|Chỉ đúng khi `0 == uc_TempUtilityKey \| 1 == uc_CheckUSBFlg` (machine_setup.c:190). Nếu không, đường "NON_USB_MEMORY" quyết định `g_action` từ SoftDIP/cờ MSC, **không đụng USB**|
|C9|Phần 19: "`CONFIG_USB_HOST_ETHER` bật khi `UHOST_XHCI_CUSTOMIZE` không định nghĩa (chưa rõ)"|`USB_XHCI` là Kconfig `default USB_XHCI_HCD` (drivers/usb/host/Kconfig:12-14) nên `CONFIG_USB_XHCI` **có**, `UHOST_XHCI_CUSTOMIZE` **có** (mvebu-common.h:347-349), do đó `USB_HOST_ETHER`/`USB_ETHER_ASIX` **không** bật và số cổng root là **4** (mvebu-common.h:366)|

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Ngăn xếp USB host (core `common/usb*.c`, xHCI, PHY, mass-storage)|
|WHY|Cập nhật firmware/khởi động từ USB memory tại xưởng và tại hiện trường; thẻ đặc biệt (Initialize card, Board-Erase)|
|WHERE|`drivers/usb/host/{xhci.c,xhci-mem.c,xhci-ring.c,xhci-mvebu.c}`, `drivers/usb/phy/phy-utm-mv.c`, `phy-usb3-mv.c`, `common/{usb.c,usb_hub.c,usb_storage.c,cmd_usb.c}`, `board/mvebu/armada8k/km/machine_setup.c`, `common/main.c:568`|

## 2. Cấu hình thực

|Mục|Giá trị|Nguồn|
|---|---|---|
|Bật|`CONFIG_USB=y`, `CONFIG_USB_XHCI_HCD=y`, `USB_MV_UTM_PHY=y`, `USB_MV_USB3_PHY=y`|defconfig:38-41|
|Suy ra|`CONFIG_USB_XHCI` (default), `CONFIG_USB_XHCI_MVEBU` (default y, phụ thuộc MVEBU), `CONFIG_USB_STORAGE`, `CONFIG_CMD_USB`, `DOS_PARTITION`, `ISO_PARTITION`|mvebu-common.h:351-361|
|EHCI (USB 2.0 riêng)|**Không** (`USB_EHCI_HCD` không bật)||
|Số cổng root cho phép|`CONFIG_SYS_USB_XHCI_MAX_ROOT_PORTS = 4`, `USB_MAX_CONTROLLER_COUNT = 4`|mvebu-common.h:366-370|
|Controller thực sự bật|1: node `usb3@d00d0000` `status="okay"` trong `quartz-iris-sb-0` (chỉ khi `CONFIG_KM_BIZHUB`)|emu800.dts:58-62|
|Thanh ghi xHCI|`reg` của node (0xD00D0000), tra bằng `fdt_get_regs_offs`|xhci-mvebu.c:106|
|PHY UTM (USB2)|Cố định `0xD00DC000` trong code (không lấy từ DT)|phy-utm-mv.c:68|
|PHY USB3 (SuperSpeed)|`phy = <0xd00a2000 0x1000>` trong DT, nhưng **không được khởi tạo** (xem 4)|quartz-iris-sb-0.dtsi:16|

Danh sách controller đến từ `board_usb_get_enabled_port_count()` (xhci-mvebu.c:60): `fdtdec_find_aliases_for_id(blob, "usb3", COMPAT_MVEBU_USB3, node_list, 4)`. DT không có `/aliases`, nhưng hàm này cũng gom các node có `compatible` khớp và đang `okay` nên vẫn tìm ra 1 node [S: theo hành vi upstream của `fdtdec_add_aliases_for_id`, chưa đọc lại `lib/fdtdec.c` cho việc này]. Kết quả: `enable_port_count = 1`.

## 3. Khi nào USB chạy: cổng điều kiện

```
misc_init_r
  ├ init_Panel()   → mfp_panel.c:196-197
  │     gpio_set_direction(GPIO_PORT_UTILITY_KEY = 49, IN)
  │     uc_TempUtilityKey = gpio_get_value(49) ? 0 : 1        ("HIGH Active": chân cao ⇒ 0 = phím ĐANG NHẤN)
  └ setup_MotionEnable_Power()                                  machine_setup.c:149
        uc_CheckUSBFlg = cờ line-card (km_linecard_boot_setting_read, sector SSD/SD)
        if ( uc_TempUtilityKey == 0  ||  uc_CheckUSBFlg == 1 ) {          ← nhấn Utility Key HOẶC cờ line-card
              WDT refresh 0x1120, 120 s
              do { usb_init(); usb_stor_curr_dev = usb_stor_scan(1); retry++;
                   if (dev < 0 && retry < 2) { usb_stop(); mdelay(10000); } } while (dev < 0 && retry < 2);
              WDT refresh 0x1121, 5 s
              check_action();  printf("action = [%d]")
        } else {
              in "#######NON_USB_MEMORY#######"
              g_action từ SoftDIP 0x06 (IISW 4–11), cờ MSC 0x7B000, HWcheckMode; mặc định DEF_BOOT ("Normal boot.")
        }
        (rồi: dựa vào g_action/Utility Key để đặt GPIO MOTION_EN_MC/IR, xem mục 7)
```

Vì `uc_TempUtilityKey` khởi tạo `= 1` ("không nhấn", machine_setup.c:40), khởi động bình thường **không** chạm USB và **không** ghi PS-CPU. Cũng vì thế `usb_stop()` trong `bootm_disable_interrupts` thường là no-op (`usb_started == 0`).

Thử lại: nếu lần quét đầu không thấy thiết bị lưu trữ, U-Boot **dừng USB, ngủ 10 giây, rồi thử thêm một lần** (tối đa 2 lần). Tổng thời gian tối thiểu khi không có USB (nhưng vẫn giữ phím) là ~10 s cộng thời gian init. WDT được nới 120 s để chịu được.

## 4. `usb_init()` (bản KM) và xHCI

```
usb_init()                                             common/usb.c:75
 ├ (KM) i2c_set_bus_num(0); i2c_write(0x3E, 0x74, 1, &0x0200, 4)      ← PS-CPU: đặt bit 9 = /RST_SLP2n
 │        (nhả reset cho phần cứng phụ trợ; nếu không thì "USBHub không nhận diện", theo comment KM)
 │        lỗi đổi bus ⇒ in "USB error: Can't change I2C BusNo to PS-CPU." và return -1
 │        KẾT QUẢ i2c_write KHÔNG được kiểm ⇒ nếu PS-CPU không ACK vẫn đi tiếp
 ├ dev_index = 0; usb_hub_reset(); xoá usb_dev[]
 ├ enable_port_count = board_usb_get_enabled_port_count()               = 1
 └ với mỗi i: printf("USB%d:   ")
        usb_lowlevel_init(i, USB_INIT_HOST, &ctrl)                      xhci.c:977
            ├ xhci_hcd_init(i, &hccr, &hcor)                            xhci-mvebu.c:80
            │     ├ (UHOST_XHCI_CUSTOMIZE) mv_usb3_phy_init() ĐÃ TẮT ("#if 0 … Unable to boot from USB3.0")
            │     │                        in "[WP30 REQ1] Skipped mv_usb3_phy_init()"
            │     ├ phy_utm_mv_init()  → start_pll(0xD00DC000)         PHY USB2 UTM
            │     ├ (lần đầu) board_usb_vbus_init() (weak → in cảnh báo; không có bản override) 
            │     ├ node = node_list[index]; hccr = reg base; hcor = hccr + HC_LENGTH
            │     └ usb_vbus_init(node)  chỉ chạy nếu CONFIG_MVEBU_GPIO (không có) ⇒ bỏ qua
            ├ xhci_reset, or_config (max slots), xhci_mem_init (malloc)
            ├ in "Register %x NbrPorts %d", "USB XHCI %x.%02x"
            └ xhci_start
        usb_alloc_new_device(ctrl)  →  (KM) dev->level=0, devpath="0", route=0  →  usb_new_device(dev)  (root hub, quét cây)
        in "%d USB Device(s) found"; usb_started = 1
 nếu chưa có bus nào: "USB error: all controllers failed lowlevel init" (return -1)
```

- **VBUS**: U-Boot không điều khiển nguồn VBUS qua GPIO. `usb_vbus_init` bị vô hiệu (`CONFIG_MVEBU_GPIO` không định nghĩa), và `board_usb_vbus_init` chỉ là hàm weak in cảnh báo. Việc cấp nguồn cổng USB do phần cứng/PS-CPU quyết định [S].
- **PHY USB3 bị bỏ qua có chủ đích**: KM thêm `#if 0 mv_usb3_phy_init();` với chú thích "Comment to fix 'Unable to boot from USB3.0'" và in dòng `Skipped`. Hậu quả: liên kết SuperSpeed không được cấu hình, USB memory hoạt động qua **USB 2.0** (UTM) [S].
- **`phy_utm_mv_init` cấu hình PLL UTM** qua các thanh ghi tại `0xD00DC000 + {0x0,0x4,0x8,0xC,0x14,0x18,0x1C}`:
    - TX_CCR0 (+0xC): `PU_ANA|PU_BY_REG`, `drv_en_ls=3`, `imp_sel_ls=3`.
    - RX_CC0 (+0x14): `host_dis_thres=0`, `ANA_SQ_DET_EN`.
    - RX_CC1 (+0x18): `ext_sq_amp_cal=1`, enable bit.
    - PHY_PLL0 (+0x0): `fbdiv=0x60`, `refdiv=5`, `sel_lpfr=0`.
    - DIG_CR_0 (+0x1C): `PLL_LOCK_BYPASS`.
    - PHY_PLL1 (+0x4): `PU_PLL_BY_REG`, rồi `PU_PLL`.
    - CAL_CNTL (+0x8): bắt đầu hiệu chuẩn PLL/impedance (`IMPCAL_VTH=7`), rồi SQ cal ở RX_CC1.
    - **Ba vòng `while` chờ không có timeout** (phy-utm-mv.c:164-168): `PLLCAL_DONE|IMPCAL_DONE`, `REG_SQCAL_DONE`, `PLL_READY`. Nếu PHY không đáp, U-Boot treo tại đây cho tới khi WDT SCPI (120 s, đã nới ở 0x1120) reset máy [V bằng đọc code]. Hàm này chạy mỗi lần `usb_init` (kể cả lần thử lại).

## 5. Mass storage và các hàm KM

`common/usb_storage.c` (bản KM, `KM_BIZHUB`):

```
check_usb_port()                                       usb_storage.c:1506
   dev = usb_get_dev_index(0)                          root hub của controller 0
   nếu chưa usb_started → do_usb("usb start")
   dev == NULL → "hub not found!" → return 0
   với i < usb_max_devs:  BspUhostCheckPortDevice(dev, usb_dev_desc[i].target)   (duyệt cây hub tìm devnum)
        thấy ⇒ setenv("bootusb", "<i>") ; return 1
   không thấy ⇒ return 0
```

Sau đó `check_action()` (main.c:568) dùng `bootusb` để đọc các file "nhận dạng" trên USB memory:

|Tệp trên USB (partition 1)|Điều kiện|`g_action`|
|---|---|---|
|`FW9020/INDEX` (thư mục theo `FW_DIRECTRY`) đọc được, loại "USB_DOWN"|firmware dùng để cập nhật|`USB_UPDATE` (2)|
|INDEX loại "USB_ZZZ"|USB memory boot|`USB_BOOT` (1)|
|INDEX loại USB_SPD (speed-change) hoặc kiểu máy sai|sai `TYP_NAME` ⇒ in "This usb memory is not for <machine>"|`BOOT_ERR` (5)|
|Không có INDEX, có `TAR_FILE_NAME` loại LINECARD (`sysBspGetUsbMemType == USB_XYZ`)|line card|`BOOT_LINE` (4)|
|`TAR_FILE_NAME_INIT` loại USB_WVU + chuỗi `DEF_INITIALIZE_CARD_IDENFITIFER`|Initialize card|`BOOT_INIT` (6)|
|`TAR_FILE_NAME_BDERASE` loại USB_BDE + `DEF_BDERASE_CARD_IDENFITIFER`|Board-erase card|`BOOT_BDERASE` (13)|
|Không USB storage, nhưng cờ mạng-DL trong SoftDIP 0x06 = 5/6||`DEF_IISW` (3)|
|tương tự, 4/7/8/9||`DEF_UPDATE` (9)|
|10 / 11||`DEF_BACKUP` (11) / `DEF_RESTORE` (12)|
|`uc_HWcheckModeBoot == 1`||`BOOT_HW_CHECKMODE` (7)|
|còn lại||`DEF_BOOT` (0)|

Đọc file bằng `do_fat_fsload(NULL, 0, 6, args)` với `args = { "fatload","usb","<bootusb>:1", addr, "FW…/INDEX", size }`. Buffer `cindex` cấp bằng `ALLOC_CACHE_ALIGN_BUFFER` (tránh "FAT: Misaligned buffer").

Với `USB_BOOT`, `bootcmd` là `usb reset; fatload usb 0 $kernel_addr $bootfile; fatload usb 0 $fdt_addr $dtb_name; fatload usb 0 $initrd_addr $initrd_name; booti …` và `root=/dev/sdc3 rootdelay=10` (Phần 16). Riêng USB_UPDATE dùng thư mục con `FWxxxx/`.

`BOOT_INIT`/`BOOT_BDERASE`: `main_loop` gọi `initialize_card_main()` / `bderase_card_main()` (xoá vùng SPI-flash, in "You can turn off the power." rồi vòng lặp vô hạn), chạy sau khi tắt WDT (`KM_SCPI_WDT_CA72_END`). Đây là **các thẻ có sức tàn phá** (xoá SPI-flash), và việc nhận diện chỉ dựa vào tên file/định danh trong INDEX, không có chữ ký (xem Phần 21).

## 6. `usb` lệnh và `usbboot`

`cmd_usb.c`: `usb` (start/reset/stop/tree/info/storage/dev/part/read/write) và `usbboot` (`do_usbboot`). `usb reset` (trong bootcmd USB) gọi lại `usb_stop()` + `usb_init()`, tức **chạy lại phần ghi PS-CPU và `phy_utm_mv_init`**.

## 7. Liên hệ với GPIO MOTION_EN

Ngay sau nhánh USB/không USB, `setup_MotionEnable_Power` đặt hai GPIO `GPIO_PORT_MOTION_EN_MC` và `_IR` (cấp phép chuyển động cơ cấu máy in):

- Utility Key không nhấn (`uc_TempUtilityKey != 0`): `MotionEnable = 1` trừ khi `g_action ∈ {USB_UPDATE, DEF_IISW, DEF_UPDATE, BOOT_ERR, DEF_BACKUP, DEF_RESTORE, BOOT_SPEEDCHANGE}`.
- Nhấn: chỉ bật cho `DEF_BOOT` (panelKey != 0x16), `USB_BOOT`, `BOOT_LINE` (panelKey != 0x16).
- GPIO chỉ được ghi khi `g_action != DEF_BOOT || uc_TempUtilityKey == 0` (còn khi DEF_BOOT bình thường, PS-CPU đã tự đặt).
- Trong các chế độ IISW/UPDATE/BACKUP/RESTORE có bước "nhả rồi cấp lại" nguồn qua `mdelay(4500)` (relay M/C, phần gọi PS-CPU đang bị comment).

## 8. Failure

|Tình huống|Hành vi|
|---|---|
|Không đổi được I2C bus 0|`usb_init` in `USB error: Can't change I2C BusNo to PS-CPU.` và `return -1`|
|PS-CPU không ACK|Bỏ qua im lặng. Hub có thể không thấy|
|`usb_lowlevel_init` = -ENODEV|`Port not available.`|
|xHCI reset/start lỗi|`lowlevel init failed` hoặc `-ENODEV`|
|PHY UTM không sẵn sàng|Treo vô hạn, cho tới khi WDT reset|
|Không thấy thiết bị lưu trữ|`usb_stor_curr_dev < 0` → thử lại một lần sau 10 s. Sau đó `check_action` → `DEF_BOOT`|
|Sai kiểu máy|`Error. This usb memory is not for <name>` → `BOOT_ERR`|
|INDEX không có|`Can not found INDEX file.` rồi thử tar line-card / init / bderase|

## 9. COMMON CONFUSION

1. **"U-Boot luôn quét USB"**: sai (C8). Cần Utility Key hoặc cờ line-card.
2. **"USB 3.0 SuperSpeed"**: PHY USB3 bị bỏ qua có chủ đích. Hoạt động ở USB 2.0.
3. **`usb_stop()` trước kernel** thường không làm gì vì USB chưa bao giờ start.
4. **`USB_BOOT` khác `USB_UPDATE`**: cái đầu chạy Linux từ USB (rootfs `/dev/sdc3`), cái sau chạy Subset firmware để cập nhật SSD.
5. **PS-CPU `0x3E`/reg `0x74`**: đây là I2C tới một vi điều khiển nguồn phụ, không phải chip USB.
6. **`usb start` do người dùng gõ** cũng chạy đúng `usb_init` bản KM nên cũng ghi PS-CPU.

## 10. MENTAL MODEL

```
GPIO49 (Utility Key) ─┐
cờ line-card ─────────┴► usb_init ─► (PS-CPU: nhả /RST_SLP2n) ─► PHY UTM PLL ─► xHCI ─► hub ─► USB memory
                                                                         │
                                          check_usb_port ─► env bootusb ─► fatload INDEX/tar ─► g_action
                                                                         │
            USB_BOOT / USB_UPDATE / BOOT_LINE / BOOT_INIT / BOOT_BDERASE / BOOT_ERR
Không nhấn phím & không cờ: bỏ qua USB, quyết định g_action từ SoftDIP/MSC → DEF_BOOT
```

## 11. Chưa xác minh

- `lib/fdtdec.c` (`fdtdec_add_aliases_for_id`) tìm node theo `compatible` khi không có `/aliases`.
- Nguồn VBUS thật của cổng USB.
- Nội dung `usb_stor_scan`/`BspUhostCheckPortDevice` ngoài phần đã đọc, và `sysBspGetUsbMemType` (phân loại INDEX).
- Ý nghĩa `USB_SPD`, `USB_DOWN`, `USB_ZZZ` (giữ nguyên tên trong code).
- Giá trị `FW_DIRECTRY` cuối cùng cho biến thể EMU800 (`FW9020`, `FW9022`, `FW9023`).

---

Xong Phần 20. Tiếp theo **Phần 21: Security**: `chk_sboot`, `sysBspSecureChip`, GM boot update, OPAL, cờ Integrity, cơ chế xác thực (hoặc không có), phạm vi bảo vệ của lệnh và cổng UART.

Grep "sysBspSecureChip|get_secureinfo|set_secureinfo_to_spi|THRG_updateGmBoot|THRG_updateGmLPPP|THRD_SECAREA_|THRD_SbootExist|THRD_GmUpdateReq|nvme_opal_cmd_send|OPAL|opal" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.[ch])

10 lines of output

Grep "sysBspSecureChip|get_secureinfo|set_secureinfo_to_spi|THRG_updateGmBoot|THRG_updateGmLPPP|THRD_SECAREA_|THRD_SbootExist|THRD_GmUpdateReq" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km)

33 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\machine_setup.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 771-840)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\mtd_flash_access.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 21-110)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\mtd_flash_access.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 1441-1700)

Grep "get_sig_from_spi|THRE_SIG_|THRD_SECAREA_RECOVERY|THRD_SECAREA_VALIDERR|THRD_SecureInfoOffset|THRD_SigOffset|THRD_SigSize|THRD_GmBootOffset|THRD_PrBootOffset|THRD_BOOT_ROM_SIZE|THRD_FLASH_CHIP_SIZE|mtd_write_protection" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.[ch])

41 lines of output

Grep "opal|OPAL" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\block\nvme.c)

31 lines of output

Grep "nvme_opal_cmd_send|NVME_OPAL|CONFIG_NVME_SECURITY|opal_.*unlock|opal_lock|opal_discovery|LOCKINGRANGE|OPAL_LOCKING|nvme_security_|sec_send|sec_recv|security_send|security_recv|nvme_admin_security" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {drivers/block/nvme.c,include/linux/nvme.h,common/cmd_nvme.c})

41 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\block\nvme.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 3921-4050)

# PHẦN 21 (PART 22): Security

## 0. Kết luận

Trong repo này **U-Boot không tự thực hiện xác thực mật mã nào** (không RSA/ECDSA, không FIT signature, không kiểm chữ ký `Image`/DTB/initrd). Các cơ chế bảo vệ tồn tại nhưng thuộc loại **toàn vẹn/chống tháo gỡ/khoá lưu trữ**, và phần lớn chuỗi tin cậy nằm ngoài repo (BootROM, ATF, phần mềm Linux). Bề mặt tấn công qua console UART không có bất kỳ cổng xác thực nào.

Phân loại theo mức bằng chứng:

|Cơ chế|Có trong repo?|Loại|Ghi chú|
|---|---|---|---|
|Xác thực chữ ký BL33/kernel/DTB/initrd|**Không**||`get_sig_from_spi` **không có người gọi** (grep)|
|FIT + `FIT_SIGNATURE`|Không build||Phần 17|
|CRC32 initrd (uImage)|Có|Toàn vẹn (ngẫu nhiên)|`image_check_dcrc`, luôn chạy vì `verify=-1`|
|SHA-256 với `EXP_INDEX`|Có, chỉ trong chế độ tự chẩn đoán HW|Toàn vẹn|Không nằm trên đường boot thường (Phần 16, C2)|
|`chk_sboot()` chốt chip bảo mật|Có|Chống tháo gỡ|Mục 2|
|GoldenMaster (GM) cập nhật + write-protect|Có|Phục hồi|Mục 3|
|Whitelist SSD NVMe|Có|Chống linh kiện lạ|Phần 12|
|OPAL (SED)|Có, không mở khoá||Mục 4|
|Mật khẩu/khoá dừng autoboot|**Không**||Mục 5|
|eFuse/OTP|Không build||`CMD_MVEBU_EFUSE` không có|

## 1. Chuỗi tin cậy và phần thuộc/không thuộc repo

```
BootROM(Marvell) ──► ATF BL1/BL2 ──► BL31 (EL3 @0x10000000, 4MB) ──► BL33 = U-Boot (EL2/EL1)
   [ngoài repo: xác thực BL2/BL31/BL33? không thể xác nhận từ repo]
U-Boot ── không xác thực ──► Image / irfs.ubt / emu800.dtb  (SSD/SD phân vùng 5)
             │  CRC32 initrd (toàn vẹn)
             ▼
        Linux (bootargs: IntegrityState=<DIP 0x413>; TerminalState, MachineInfo, SerialNo…)
```

U-Boot chỉ **chuyển** cờ `IntegrityState` (`getInergrity()` đọc SoftDIP `0x413`) cho kernel. Việc có kiểm chữ ký `THRE_SIG_KERNEL/DTB/RFS1/RFS3/IRFS` (5 khe 256 byte tại SPI `0x3FD000`) là của phần mềm khác. Kết luận "U-Boot có secure boot đầy đủ" **không** được hỗ trợ bởi mã này [V bằng grep: không ai gọi `get_sig_from_spi`, không có hàm hash/RSA nối với nó].

Điều chỉnh Phần 0: tôi từng ghi "SHA-256 index check + `chk_sboot` + `chkGmUpdate`" như thể là secure boot. Đúng hơn: SHA-256 chỉ ở tự chẩn đoán; `chk_sboot` là chống tháo chip; `chkGmUpdate` là sao lưu firmware.

## 2. `chk_sboot()`: khoá chống tháo chip bảo mật

```
main_loop → chk_sboot()   (main_loop gọi TRƯỚC bootdelay; autoboot.c:464)
   bSboot  = sysBspSecureChip()          machine_setup.c:795
               đọc *(volatile u32*)0xC0042008 & 0x10   (PIO_SYS[36]: L = không chip, H = có chip)
               kết quả cache (chỉ đọc lần đầu), in "piosys36[%d]"
   spiflg  = get_secureinfo(THRD_SECAREA_SBOOT = 0x28, 8 byte)     ← SPI 0x3FE000 + 0x28
   nếu đọc lỗi                                          → "flash read err", bResult = false
   nếu !bSboot && spiflg == THRD_SbootExist (1)          → "boot failed."       bResult = false   (chip đã từng có, nay mất)
   nếu  bSboot && spiflg != 1  → ghi spiflg = 1 (set_secureinfo_to_spi) "boot flg on"
   nếu bResult == false:
        WDT: km_scpi_set_wdt_ca72_ext(END)          (tắt watchdog để khỏi reset)
        "You can turn off the power."  →  while(1){}                 ← KHÓA VĨNH VIỄN cho đến khi cắt nguồn
```

Ý nghĩa: sau khi máy từng được lắp chip bảo mật (dùng cho chế độ bảo mật), việc tháo chip khiến máy **từ chối khởi động** ở mọi lần sau. Cờ được lưu **trong SPI-flash**, mỗi cờ dài 8 byte (offset cố định trong sector `0x3FE000`, gồm `RECOVERY 0x00`, `VALIDERR 0x10`, `GMBOOT 0x18`, `GMLPPP 0x20`, `SBOOT 0x28`).

Hạn chế đã thấy:

- Cờ nằm trong SPI-flash mà console có `bspi` (erase/write) và `set_secureinfo_to_spi` không có xác thực, nên **không đủ để chống kẻ có console** [S: phụ thuộc vào việc vùng đó có write-protect ở cấp phần cứng hay không].
- `set_secureinfo_to_spi` thực hiện **đọc cả sector 4KB → xoá → ghi lại**. Mất điện giữa "erase" và "write" làm mất toàn bộ sector (gồm cả các cờ `GMBOOT`, `GMLPPP`, `SBOOT`) mà không có bản sao [V bằng đọc code: một sector duy nhất, không có 2 bản như vùng SoftDIP].
- `sysBspSecureChip()` tin một chân PIO đọc qua địa chỉ vật lý `0xC0042008`.

## 3. GoldenMaster (GM): phục hồi firmware Boot/LPPP

`chkGmUpdate()` (autoboot.c:399) chạy trong `autoboot_command` **chỉ khi `g_action == DEF_BOOT`**, ngay trước `run_command_list(bootcmd)`:

```
u64_update[2] = get_secureinfo(THRD_SECAREA_GMBOOT (0x18), 16 byte)      // [0]=Boot, [1]=LPPP
if u64_update[0] == THRD_GmUpdateReq(1):   "update GM Boot"
      THRG_updateGmBoot():        mtd_flash_access.c:1600
         flash = get_mtd_device(NULL,0); nếu chip nhỏ hơn 8MB (THRD_FLASH_CHIP_SIZE) → "No Area for GM", return OK
         đọc Primary Boot (offset 0, 3MiB = THRD_BOOT_ROM_SIZE) vào bộ đệm stack uc_buf[3MiB]
         mtd_write_protection(flash, false)                 ← bỏ khoá ghi
         erase  0x400000 (GM), write, đọc lại, memcmp
         mtd_write_protection(flash, true)                  ← khoá lại
      thành công → ghi lại cờ = 0 ("complete")
tương tự cho LPPP (u64_update[1]) qua THRG_updateGmLPPP()
```

- `Primary Boot` = 3 MiB tại SPI offset 0 (chứa chuỗi khởi động: ATF + U-Boot). `GoldenMaster Boot` = bản sao ở `0x400000`. Nghĩa là khi Primary lỗi thì BootROM/ATF (ngoài repo) có thể rơi về GM [S].
- Yêu cầu cập nhật `GmUpdateReq` do phía Linux đặt (sau khi cập nhật firmware và xác nhận chạy tốt), U-Boot chỉ thực hiện sao chép. Nếu thất bại, in `GM update failed.` nhưng **vẫn tiếp tục boot**.
- **Cảnh báo bộ nhớ**: `unsigned char uc_buf[THRD_BOOT_ROM_SIZE]` và `uc_cmp[...]` là **mảng cục bộ 3 MiB mỗi cái** (tổng 6 MiB) nằm trên **stack** của U-Boot. Stack ban đầu trong `board_init_r` nằm dưới đáy `gd`/`bd` gần đỉnh RAM và không có kích thước dành riêng cho 6 MiB [V bằng đọc code; hậu quả thực phụ thuộc vào bố cục bộ nhớ]. Chưa xác định vùng nào bị đè, cần kiểm chứng khi chạy. Đánh dấu [?] nhưng nên coi là điểm rủi ro cao ở đường "cập nhật GM".
- Cả hai hàm (Boot, LPPP) không kiểm `mtd_read_id_with_size` khi `flash` lỗi (`IS_ERR` chỉ được kiểm ngay sau `get_mtd_device` ở Boot).

## 4. OPAL/SED và whitelist SSD

- **Whitelist SSD** (`st_nvme_supported[]` + tối đa 30 mục nạp từ SPI 0x200, Phần 12): chỉ SSD có PCI ID đã biết mới được dùng. Đây là kiểm soát chống linh kiện không được phê duyệt, không phải xác thực mật mã.
- **OPAL** (`nvme_opal_cmd_send`, nvme.c:3938): thực hiện Level 0 Discovery rồi mở phiên `ANYBODY → LOCKING SP` (không mật khẩu) và đóng lại (`end_opal_session`, thêm cho HeliosMLK 2020/03/02). **Không mở khoá locking range**, không nhận mật khẩu. Chỉ chạy ở nhánh Warp (main.c:2255) và chỉ khi SSD hỗ trợ (`pdev->b_security`). Nếu SSD bị khoá bởi SED, U-Boot không thể mở nó; mục đích chính xác của lệnh này không được ghi trong mã [?].
- `CONFIG_NVME_SECURITY_SUPPORT` bị comment (mvebu-common.h:487), nên `nvme_security_erase` (Sanitize/Secure Erase từ console) **không** được build.

## 5. Bề mặt tấn công qua console

|Yếu tố|Trạng thái|Nguồn|
|---|---|---|
|Ngắt autoboot|**Bất kỳ phím nào**; không `CONFIG_AUTOBOOT_KEYED`, không `bootstopkey/bootdelaykey`, không mật khẩu|autoboot.c:167-241|
|`ZERO_BOOTDELAY_CHECK`|Một byte đã có trong bộ đệm UART cũng đủ ngắt|mvebu-common.h:126|
|Prompt sau khi ngắt|`Marvell>>` với hush đầy đủ|Phần 15|
|Khoá lệnh / danh sách trắng|Không||
|Console ẩn|`CONFIG_SILENT_CONSOLE` có, nhưng `silent` không nằm trong env mặc định; KM dùng bộ đệm log + `termFlg` (Phần 9). **Đầu vào UART vẫn xử lý** (`tstc()`)||
|Ẩn UART panel|COM2 (panel) không đăng ký console|Phần 9|

Lệnh có sẵn có năng lực nguy hiểm (không cần xác thực), đã xác minh trong Phần 15/19/20:

- Bộ nhớ/thanh ghi vật lý: `md`, `mm`, `nm`, `mw`, `cp`, `cmp`, `mtest`, `loop`.
- Thực thi: `go`, `bootelf`, `bootvx`, `mss_boot` (nạp mã vào CM3 và thả reset), `bootm/booti`, `loads/loadb/loadx/loady` (nạp qua UART).
- Flash/lưu trữ: `bspi` (đọc/xoá/ghi/`quartz_writemem` SPI-flash, kể cả vùng secure và GM), `mmc`, `nvme`, `usb`, `fatload/ext2load`.
- Dữ liệu nhà máy: `y`, `z`, `X`, `x` (MAC, serial; `z` còn xoá snapshot Warp).
- GPIO: `qzgpio` (đặt hướng, giá trị, pinmux của 272 chân, gồm `HRESET_REQ`, `POWER_OFF_PNL`, `MOTION_EN_*`).
- Điều khiển boot: `doboot N` (ép `g_action`, gồm `BOOT_INIT`/`BOOT_BDERASE`? Xem ghi chú), `warp`, `clear_bootf`, `C` (xoá NVRAM), `CB`, `setenv/env import`.

Ghi chú `doboot`: `g_action` được gán trực tiếp từ đối số (`g_action = mode`), nhưng `check_autoboot` chỉ có nhánh cho các giá trị đã liệt kê; `BOOT_INIT (6)`/`BOOT_BDERASE (13)` không được xử lý trong `check_autoboot` (rơi vào `default`), việc xoá thẻ chỉ xảy ra khi `main_loop` thấy `g_action` này **trước** bước ngắt (`initialize_card_main`). `doboot` chạy sau khi đã vào prompt nên không kích hoạt được (Phần 16) [V].

Đường "thẻ USB đặc biệt" (Phần 20): `BOOT_INIT` (xoá SPI-flash vùng người dùng) và `BOOT_BDERASE` được nhận diện **chỉ bằng tên file và định danh chuỗi** trong INDEX trên USB (`DEF_INITIALIZE_CARD_IDENFITIFER`, `DEF_BDERASE_CARD_IDENFITIFER`), không có chữ ký. Ai tạo được USB memory đúng định dạng và giữ Utility Key lúc bật nguồn có thể kích hoạt. Rào cản thực tế là quyền vật lý (giữ phím) và kiến thức định dạng [S].

## 6. Lỗi bộ nhớ đã xác nhận có thể khai thác từ console hoặc dữ liệu ngoài

|#|Vị trí|Sự cố|Điều kiện|
|---|---|---|---|
|S1|`command.c:718,905` (`y`, `z`)|`strlen(argv[1])` với `argv[1]==NULL` khi gõ lệnh không đối số|console; kết quả: data abort|
|S2|`machine_setup.c:593-595` (`fsload_hash_check`)|`p_str_env = getenv("filesize")` là **con trỏ vào chuỗi giá trị của biến môi trường**; code ghi `p_str_env[ui32_length] = '\0'` với `ui32_length` = giá trị filesize (parse hệ **10** dù `filesize` là hex). Nghĩa là ghi 1 byte 0 tại `địa chỉ chuỗi + kích thước tệp`, ngoài chuỗi (heap). Ý định là ghi vào bộ đệm INDEX|chế độ tự chẩn đoán HW|
|S3|`main.c:1656-1669` (`make_bootargs`)|`sprintf` vào `chBtEv[768]` không giới hạn; `SerialNo` lấy từ ROM. `BspPrintBuffering` dùng `sprintf(dest, printbuffer)` (định dạng do dữ liệu) [từ Phần 4/9]|dữ liệu SPI-flash hoặc chuỗi in|
|S4|`mtd_flash_access.c:1604-1605`|2 mảng 3 MiB trên stack (mục 3)|`GmUpdateReq` được đặt|
|S5|`phy-utm-mv.c:164-168`|Vòng chờ phần cứng vô hạn (treo cho tới khi WDT)|phần cứng USB không đáp|
|S6|`image.c:1241` (kernel Image)|`memmove(dst, src, ih->image_size)` tin trường `image_size` trong header Linux, `lmb_reserve` cũng vậy|Image do bên ghi lên SSD kiểm soát|
|S7|`image-fdt.c:499`|`(u32)(uintptr_t)blob`|Chỉ đúng khi DTB dưới 4GiB (16MB window nên an toàn)|

Không có bảo vệ chống tràn (stack protector, NX, ASLR) trong cấu hình này [?: chưa kiểm cờ `-fstack-protector`; 2015.01 không bật mặc định].

## 7. Phần cứng và quyền thực thi

- U-Boot chạy ở **EL2 hoặc EL1** (không phải EL3): không sửa được cấu hình EL3/TrustZone của ATF; PSCI do ATF. `CONFIG_ARMV8_PSCI` chỉ kích hoạt ở EL3 (`is_psci_enabled`).
- Bảng trang: `mmu_setup` dựng các "section" 512MB; vùng RAM là Normal, vùng khác là Device-nGnRnE (Phần 3/23). Không có phân quyền XN/PXN theo từng vùng được xác nhận [?].
- Không có lệnh ghi OTP/eFuse trong ảnh (`CMD_MVEBU_EFUSE` không có).
- Không có lệnh tự cập nhật U-Boot (`bubt` không build). Cập nhật U-Boot qua đường Linux/GM (Phần 31).

## 8. Failure và hành vi khi bị từ chối

|Điều kiện|Hành vi|
|---|---|
|Chip bảo mật bị tháo|`boot failed.` rồi treo vĩnh viễn, WDT bị tắt trước để tránh reset lặp|
|Đọc secure info lỗi|`flash read err (%d)` và cũng treo (vì `bResult=false`)|
|GM update lỗi|`GM update failed.` nhưng vẫn boot|
|CRC initrd sai|`Bad Data CRC`, `booti` thất bại, rơi vào prompt|
|Hash tự chẩn đoán sai|`Hash Error!!!`, panel báo `STORAGE_ERR`, `_WAIT_FOREVER()`|

Lưu ý: khi `chk_sboot` treo, **không có kênh phục hồi bằng phần mềm** (WDT đã tắt); chỉ cắt nguồn. Nếu đọc SPI-flash lỗi tạm thời cũng dẫn tới treo, đó là trường hợp fail-closed khắc nghiệt (một lỗi đọc tạm thời có thể làm máy không boot cho tới khi cắt nguồn) [V bằng đọc code].

## 9. COMMON CONFUSION

1. **"Có `THRE_SIG_*` nghĩa là U-Boot kiểm chữ ký"**: không, `get_sig_from_spi` không có người gọi.
2. **"SHA-256 với `EXP_INDEX` là secure boot"**: nó chỉ ở tự chẩn đoán và không xác thực nguồn.
3. **"`IntegrityState=1` nghĩa là U-Boot đã kiểm tra"**: chỉ là cờ truyền cho kernel.
4. **"`nvme_opal_cmd_send` mở khoá SSD"**: không có mật khẩu, không mở khoá.
5. **"Không thấy log nên không ai dùng được console"**: đầu vào UART vẫn hoạt động, chỉ đầu ra bị đệm.
6. **"GM là bản chép của toàn bộ SPI"**: chỉ 3 MiB Boot (offset 0 → 0x400000) và LPPP.

## 10. MENTAL MODEL

```
Bảo vệ thật ở đây:   [chip PIO] ─ chk_sboot ─ cờ SPI ─► từ chối boot nếu chip bị tháo
                     [GM]  Primary ─copy─► Golden Master (có write-protect)
                     [CRC32 initrd], [SHA-256 chỉ tự chẩn đoán], [whitelist SSD]
Không có ở đây:      chữ ký, FIT, mật khẩu console, eFuse, xác thực USB/INDEX
Kiểm chữ ký (nếu có) → phần mềm khác (Linux/ATF)
```

## 11. Chưa xác minh

- Có xác thực BL33 ở ATF/BootROM không (ngoài repo).
- Bố cục stack khi `THRG_updateGmBoot` chạy (6 MiB cục bộ).
- SPI-flash vùng `0x3FE000` có write-protect phần cứng không.
- Có cờ `-fstack-protector` hay không (kiểm `arch/arm/config.mk` và `Makefile`).
- Ý nghĩa thực của phiên OPAL `ANYBODY`.
- UART RX có thực sự truy cập được trên máy sản xuất.

---

Xong Phần 21. Tiếp theo **Phần 22: Watchdog / Reset / Power**: SCPI watchdog CA72 (`ca72wdt.c`), mailbox, PS-CPU qua I2C (DW-I2C bus 0), `reset_cpu`, `POWER_OFF_PNL`.

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\ca72wdt.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Grep "km_scpi_set_wdt_ca72_ext|KM_SCPI_WDT_CA72_|SCPI_MAILBOX|scpi" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km, glob: ca72wdt.h)

4 lines of output

Grep "bool send_cmd\(|send_cmd\(|SCPI_MAILBOX|scpi_|MHU|mhu" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.[ch])

21 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\scpi\arm_scpi.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Grep "scpi|SCPI" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {drivers/Makefile,drivers/scpi/Makefile,drivers/scpi/Kconfig,include/configs/armada8k.h,include/configs/mvebu-common.h,Makefile})

7 lines of output

Grep "PSCPU_SLAVE_ADDR|PSCPU_REG_[A-Z_]+|THRD_EventRecord_[A-Za-z]+" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.[ch])

16 lines of output

Grep "pause_r4\(|_WAIT_FOREVER|define _WAIT_FOREVER|GPIO_PORT_POWER_OFF_PNL|POWER_OFF_PNL|reset_with_recoverycode|reset_cpu\(" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.[ch])

41 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\i2c\designware_i2c.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 326-435)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\board_r.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 85-102)

Grep "^void reset_cpu|^void reset_with_recoverycode|^int do_reset|recovery|RFU_GLOBAL_SW_RST|HRESET|gpio_set_value|hang\(|wdt|WDT|watchdog" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\armada8k\soc.c)

16 lines of output

Grep "initr_wd_refresh|WATCHDOG_RESET|hw_watchdog_reset|CONFIG_HW_WATCHDOG|CONFIG_WATCHDOG" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {common/board_r.c,common/board_f.c,common/main.c,include/configs/armada8k.h,include/configs/mvebu-common.h,include/watchdog.h,board/mvebu/armada8k/km/*.c,arch/arm/cpu/armv8/armada8k/*.c,arch/arm/cpu/mvebu-common/*.c})

21 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\armada8k\soc.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 276-335)

Grep "initr_wd_refresh" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src)

6 lines of output

Grep "ca72|CA72|wdt|wd_refresh|km_scpi_set_wdt_ca72_ext" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\board_r.c)

7 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\board_r.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 873-912)

Grep "initr_wd_start|initr_wd_refresh|INIT_FUNC_WATCHDOG_RESET|INIT_FUNC_WATCHDOG_INIT|init_sequence_r\[\]" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\board_r.c)

26 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\include\watchdog.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 21-35)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\board_r.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 914-1128)

Grep "km_scpi_set_wdt_ca72_ext\(" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.c)

22 lines of output

Grep "void reset_with_recoverycode" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: *.c)

61 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\cmd_boot.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 11-60)

Grep "reset_with_recoverycode" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {board/**/*.c,arch/**/*.c,lib/*.c,common/*.c})

8 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\lib\reset.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

# PHẦN 22 (PART 23): Watchdog / Reset / Power

## 0. Kết luận

U-Boot **không dùng watchdog phần cứng của ARM/SoC**. `CONFIG_WATCHDOG` và `CONFIG_HW_WATCHDOG` đều không định nghĩa, nên `WATCHDOG_RESET()` rỗng. Toàn bộ "chó canh" là một **watchdog ngoài do bộ vi xử lý phụ R4 ("LPPP/SCP") trong Quartz đảm nhiệm**. U-Boot (lõi CA72) ra lệnh cho R4 qua **hộp thư SCPI/MHU**. Nếu R4 không nhận được "refresh" trong thời hạn thì R4 khởi động lại máy. Nguồn/khởi động lại toàn máy còn qua **PS-CPU** (I2C 0x3E) và chân **HRESET_REQ**.

Đính chính nhỏ ở Phần 18: chỗ "WDT 300 s chạy tiếp vào kernel" đúng. Nhưng ở Phần 21 tôi viết "chk_sboot treo vĩnh viễn kể cả khi đọc lỗi tạm thời". Chi tiết đúng: `chk_sboot` **tắt WDT trước rồi mới treo** nên chỉ cắt nguồn mới thoát (không có reset tự động). Xem mục 4.

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Cơ chế phát hiện treo (watchdog), khởi động lại và điều khiển nguồn|
|WHY|Máy in phải tự phục hồi khi boot treo; phần mềm phụ (R4, PS-CPU) chủ động cấp nguồn/reset|
|WHERE|`board/mvebu/armada8k/km/ca72wdt.c`, `drivers/scpi/arm_scpi.c`, `common/board_r.c:880-900,88-93`, `common/autoboot.c`, `common/main.c`, `arch/arm/lib/reset.c`, `arch/arm/cpu/armv8/armada8k/soc.c:285`, `drivers/i2c/designware_i2c.c`, `common/usb.c`, `arch/arm/lib/bootm.c`|

## 2. Watchdog SCPI (CA72 → R4)

### 2.1 API

```c
km_scpi_set_wdt_ca72_ext(int event, u16 point, u16 timeout, u16 debugInfo1, char *debugInfo2)   // ca72wdt.c:7
   event : START(0) / END(1) / REFRESH(2)                    ca72wdt.h:4-6
   struct { u32 event, point, timeout, debugInfo1; char debugInfo2[240]; }   // 256 byte
   send_cmd(0x8C /*KM_SCPI_CMD_WDT_REBOOT_EXT*/, 256, &sendData)
```

- `point` là mã "điểm kiểm" (checkpoint) để R4 ghi lại vị trí treo khi phải reset. `timeout` tính bằng **giây** (ví dụ 300 cho boot thường). `debugInfo2` là chuỗi tối đa 239 ký tự (ví dụ `"Normal Boot.\n"`).
- Ý nghĩa `timeout` và hành động của R4 chỉ suy ra từ cách dùng (R4 firmware ngoài repo) [S].
- Bật chế độ gỡ lỗi: START với `debugInfo1 & 1` → in `## wd debug mode.` và về sau in mỗi lệnh WDT.

### 2.2 Đường truyền `send_cmd` (arm_scpi.c:204)

```
mhu_regs = 0xE8009800 (MHU)
sheader  = 0x7FFF0000 + (2*1+1)*128 = 0x7FFF0180     ← kênh 1, hướng AP→SCP  (SCP_COM_SHARED_MEM_BASE 0x7FFF0000, "khớp scp-shmem trong DTB Linux")
sheader->status=0; command_id=cmd; sender_if=2; payload_size=256; chép payload
flush_dcache_range(sheader, sheader+64)              ← chỉ 64 byte
write_MHU(mhu, block=1, 2)                           ← TX_INT_SET bit1: báo R4
chờ R4 xoá bit1 (TX_INT_STATUS): tối đa 1000 × mdelay(1)   → "[SCPI] Send timeout"  return false
chờ R4 đặt bit1 (RX_INT_STATUS) = phản hồi: tối đa 1000 ms  → "[SCPI] Response timeout" return false
clear_MHU(mhu, 0, 2); return true
```

Điểm rút ra:

- **Bộ đệm dùng chung**: mỗi ô kênh `SCPI_CHANNEL_BUFFER_SIZE = 128` byte, nhưng `payload_size = 256` và vòng lặp chép vào `sheader->payload` (offset 8) nên ghi tới `0x7FFF0180 + 8 + 256 = 0x7FFF0288`, tức **tràn sang ô kế tiếp** (kênh 2, 0x200–0x2FF) [V bằng số học]. Chỉ vô hại nếu R4 và ATF/Linux không dùng kênh 2. (Phần cứng Marvell/R4 firmware của KM có thể đã thống nhất kích thước này; không xác minh được.)
- **Chỉ flush 64 byte** đầu tiên. Bốn trường điều khiển (`event, point, timeout, debugInfo1` = 16 byte, cộng header 8 byte = 24 byte) nằm gọn trong 64 byte, còn `debugInfo2` (từ byte thứ 24 tới 264) chỉ đến R4 đầy đủ khi D-cache đang tắt (đường NVMe) hoặc theo may rủi. Hậu quả chỉ là chuỗi gỡ lỗi bị cắt/nhiễu.
- **Kết quả bị bỏ qua**: `km_scpi_set_wdt_ca72_ext` không kiểm giá trị trả về của `send_cmd`. Nếu R4 không đáp thì **mỗi lệnh WDT tốn tới ~1–2 giây** rồi im lặng thất bại. Tổng chi phí trong một lần boot (khoảng 25–35 lệnh, xem 2.3) có thể lên hàng chục giây, và tệ hơn là WDT thực sự không được nạp.
- Bản KM ép `send_cmd` trả `bool` (`CONFIG_KM_BIZHUB`); bản gốc Marvell chờ vô hạn (`do { … } while(val)`).

### 2.3 Dòng thời gian WDT trong một lần boot

|Điểm (`point`)|Sự kiện|Timeout (s)|Nơi|Ghi chú|
|---|---|---|---|---|
|0x1000|START|5|`initr_wd_start`, mục **đầu tiên** của `init_sequence_r` (board_r.c:916)|Ngay sau relocation|
|0x1000\|n|REFRESH|5|`initr_wd_refresh` chèn giữa **16 bước** của `init_sequence_r` (`INIT_FUNC_WATCHDOG_RESET`)|`n` là bộ đếm `static int count` tăng mỗi lần; cũng chạy trong `_WAIT_FOREVER`|
|0x1120|REFRESH|**120**|`setup_MotionEnable_Power`, trước quét USB|Chỉ khi Utility Key/line-card (Phần 20)|
|0x1121|REFRESH|5|sau quét USB||
|0x1130|REFRESH|10|IISW/UPDATE/BACKUP/RESTORE trước khi ngắt/cấp lại nguồn M/C|rồi `mdelay(4500)`|
|0x11B0|REFRESH|40|Trước `warp_boot(SuperWarp)` "SuperWarp boot."|main.c:2260|
|0x11A0|REFRESH|90|Trước `warp_boot(Normal)` "Warp boot."|main.c:2280|
|0x11D0|REFRESH|`bootdelay==0 ? 5 : bootdelay×2`|`abortboot_normal` (autoboot.c:181)|3 s → 6 s|
|0x11D1|REFRESH|5|không bị ngắt, tiếp tục (autoboot.c:237)||
|END|||phím ngắt (autoboot.c:235)||
|0x11D2|REFRESH|**300**|`DEF_BOOT`: "Normal Boot." (autoboot.c:353)|chạy xuyên `bootcmd` → kernel → ứng dụng|
|0x11E0/1/2|REFRESH|120|`USB_UPDATE` / `DEF_IISW` / `DEF_UPDATE`||
|END|||mọi `g_action` khác (autoboot.c:368)||
|END|||`BOOT_INIT`, `BOOT_BDERASE` (main.c:2220,2226), sau `bootcmd` (2345), `chk_sboot` thất bại (autoboot.c:504), một nhánh trong `setup_MotionEnable_Power` (machine_setup.c:338)||

Vì vậy: trong **boot thường** WDT vẫn **đang mở với hạn 300 s khi kernel khởi chạy**. Phần mềm ở kernel/ứng dụng phải tiếp tục nạp hoặc tắt nó qua chính giao thức SCPI (chung vùng nhớ `0x7FFF0000`, "khớp scp-shmem trong Linux DTB"). Nếu kernel không bao giờ tới được bước đó, R4 sẽ reset máy sau tối đa 300 s. Đây là cơ chế phục hồi chính của đường boot hỏng, **không có trong U-Boot**.

### 2.4 Các vòng treo có chủ đích

|Vòng|WDT|Khi nào|
|---|---|---|
|`_WAIT_FOREVER()` (board_r.c:88)|**Vẫn nạp** mỗi giây (`udelay(1s); initr_wd_refresh()`)|Lỗi chẩn đoán HW đã hiển thị (LED/panel), giữ máy ở trạng thái lỗi để nhân viên đọc mã, không reset lặp. Dùng ở init_km và `boot_diag`, `hwcDispLoadDiag`|
|`chk_sboot` thất bại (autoboot.c:503-507)|**Tắt (END)** rồi `while(1){}`|Chip bảo mật bị tháo. Chỉ cắt nguồn mới thoát|
|`initialize_card_main` / `bderase_card_main`|Tắt, `while(1)`|"You can turn off the power."|
|`hang()` (`FDT creation failed! hanging...`)|Không đụng WDT (nếu đang mở thì R4 sẽ reset sau timeout)|Lỗi nghiêm trọng khi chuẩn bị kernel|
|`phy_utm_mv_init` vòng chờ PHY|Không nạp; nới 120 s trước đó|Xem Phần 20|

## 3. Reset

```
reset  ─►  do_reset()                 arch/arm/lib/reset.c:30
             puts("resetting ...")     (bộ đệm log KM)
             udelay(50 ms); disable_interrupts() (stub); reset_misc() (weak, rỗng)
             reset_cpu(0)              arch/arm/cpu/armv8/armada8k/soc.c:285
                 RFU_GLOBAL_SW_RST (0xF06F0084) &= ~(1 << RFU_SW_RESET_OFFSET)     ← reset mềm SoC
                 (KM) gpio_set_value(GPIO_PORT_HRESET_REQ = 66, 0)                 ← "Request power restart" → PS-CPU
             return 0                  ("NOTREACHED" nhưng vẫn có thể quay về nếu reset chưa xảy ra ngay)
```

- Hai cơ chế song song: SW reset của RFU và **yêu cầu khởi động lại nguồn** tới PS-CPU bằng chân 66 (kéo xuống 0). Chỉ có GPIO khi `KM_BIZHUB`. Hàm không đặt hướng chân (giả định đã là output do `init_km`/panel).
- `do_reset` **có thể trả về** (dòng 42): trong `reset_with_recoverycode`, sau `do_reset`, hàm cũng return và luồng của caller (ví dụ khởi tạo LVDS) tiếp tục cho tới khi reset thật sự xảy ra.

### `reset_with_recoverycode(code)` (reset.c:67, chỉ `KM_BIZHUB`)

```
rec = BSP/Thread region offset 0x2E, 2 byte  = (code<<8)|counter        (SPI-flash 0x77000/0x78000, Phần 12)
counter++ ; rec = code<<8 | counter
nếu counter > 5 (DEF_UBOOT_REBOOT_COUNT_OVER) hoặc đọc SPI lỗi (NG_PARAM/ALL_AREA/READ_ERR):
       in "## reset_with_recoverycode : code:0x%x ret:%d"; return   ← KHÔNG reset (chống vòng reset vô hạn)
ghi rec
ghi 0x1010 vào offset 0x20 (2 byte)  → "SuperWarp disable"          (cấm SuperWarp sau lỗi)
do_reset()
```

Firmware chính (Linux) đọc `0x2201C0000000xxyy` (xx = recovery code, yy = counter) và tự xoá về 0. Các mã đã thấy trong source:

|Mã|Nơi|Lỗi|
|---|---|---|
|0x01|lcd_lvds_pll.c:387|LVDS PLL không khoá (timeout)|
|0x02|lcd_lvds_pll.c:620|Tuỳ chọn LVDS không nhận ra|
|0x03|lcd_lvds_pll.c:614|LVDS không dùng|
|0x04|lcd_lvds_pll.c:639|`pixelclk` = 0|
|0x05|lxk_armada_dumb.c:102|Kiểm gamma LUT sai|
|0x06|lxk_armada_dumb.c:195|Không đủ frame buffer|
|0x07|graphlib.c:62|Định dạng màu sai (`g_press`)|
|0x08|graphlib.c:88|`Inflate` khởi tạo lỗi|
|0x09|graphlib.c:107|`Inflate` lỗi|
|0x0a|graphlib.c:179|LZO giải nén lỗi|
|0x0b|lxk_armada_dumb.c:168|Không có dữ liệu panel|
|0x0c|lxk_armada_dumb.c:178|Độ sâu màu không hỗ trợ|

Nói cách khác, **lỗi đồ hoạ khi khởi động** (panel/LVDS/ảnh nén) làm U-Boot tự reset tối đa 5 lần rồi thôi. Ở lần cuối không reset nữa và tiếp tục chạy với đồ hoạ hỏng, để máy vẫn còn khả năng boot.

## 4. Nguồn: PS-CPU, GPIO, I2C

### 4.1 I2C (DW-I2C) và PS-CPU

- Bus I2C: **DesignWare** `CONFIG_SYS_I2C_DW`, 4 bus tại `0xE8006000`, `0xE8006800`, `0xE8007000`, `0xE8007800`; tốc độ 400k/400k/100k/100k; địa chỉ slave của chính mình 0x02 (`quartz-sb.h:23-41`). Đính chính Phần 11: `CONFIG_MVEBU_I2C` tắt nhưng I2C **không vắng**: driver DW dùng thay thế (`designware_i2c.o` + `i2c_core.o`).
- **PS-CPU** ở bus 0, địa chỉ `0x3E`; RTC ở `0x3D`. Trong `dw_i2c_read` KM thêm chuỗi đặc biệt cho hai slave này (designware_i2c.c:380-388): sau khi gửi địa chỉ + 2nd address, gửi `IC_STOP`, chờ 25 µs (~10 bit), rồi đặt lại địa chỉ slave và đọc (không dùng repeated-start).
- Thanh ghi PS-CPU đã thấy:

|Thanh ghi|Nội dung|Nơi dùng|
|---|---|---|
|`0x74` ("bit set")|ghi `0x0200`: bit9 = `/RST_SLP2n` (nhả reset phụ trợ AP)|`usb_init` (usb.c:88)|
|`0xC0` (event)|ghi số sự kiện: `4 = WarpStart`, `10 = KernelStart`|main.c:2267/2287 (Warp), bootm.c:93 (kernel)|

Sự kiện KernelStart chỉ ghi khi DIP mềm 144 (`0xA1` bit0) bật. `i2c_write` không được kiểm kết quả ở hầu hết chỗ gọi.

- Việc đo bằng Warp: `b_AutoMeasurement` bật thì có sự kiện Warp. Các lệnh `pwsub_relay_port_ctrl` (điều khiển rơ-le M/C qua PS-CPU) đã bị **comment** ("cần hỗ trợ giao tiếp PS-CPU", machine_setup.c:319-328), nên U-Boot chỉ chờ 4,5 s.

### 4.2 GPIO nguồn/chuyển động (đã biết)

|GPIO|Vai trò|Dùng ở|
|---|---|---|
|66 `HRESET_REQ`|Yêu cầu khởi động lại nguồn|`reset_cpu`|
|79 `POWER_OFF_PNL`|Tắt nguồn panel (dùng chung IO pad với UTILITY_KEY=49 theo comment mfp_panel.c:191)|panel|
|49 `UTILITY_KEY`|Phím tiện ích|Phần 20|
|`MOTION_EN_MC`, `MOTION_EN_IR`|Cho phép cơ cấu chuyển động|`setup_MotionEnable_Power` (Phần 20)|
|`PANEL_SUBSW`|Phím tiết kiệm điện (`checkPowerSaveKey`)|main.c:1077|

Cách `MotionEnable` được quyết định: xem Phần 20 mục 7. Nguyên tắc chung: **chỉ bật cơ cấu khi boot bình thường**, và tắt khi đang cập nhật/lỗi/chẩn đoán.

### 4.3 Bảo vệ R4 khi ghi SPI-flash: `pause_r4`

`qspi_winbond.c` bọc mọi thao tác ghi/xoá/đọc ID (các cặp tại dòng 262/280, 468/541, 581/638, 704/769, 1227/1387) bằng `pause_r4(true)` … `pause_r4(false)`:

```
pause_r4(true):   (đếm lồng: chỉ lần đầu làm việc)
     send_cmd(0x0A, 2, 0x0100)                       ← lệnh "pause" cho R4
     wait_for_LPPPintoWFI()                          ← đọc 0xE8246260 & 0x10 (sau khi bật clock ở 0xE8240080), 100µs × 1000 = tối đa 100 ms
pause_r4(false):  (khi bộ đếm về 0)
     *(u32*)0xE800800C = 1                           ← cho R4 chạy lại
```

Lý do: R4 dùng chung QSPI-flash; nếu R4 truy cập flash lúc U-Boot ghi thì lỗi. Trong khoảng R4 bị dừng, **WDT do R4 phụ trách cũng đứng**; U-Boot không nạp WDT trong lúc đó (xoá/ghi vùng lớn như GM Boot 3 MiB mất nhiều giây, nhưng WDT không chạy vì R4 đang WFI) [S].

Nhận xét: `pause_r4(true)` không kiểm kết quả `send_cmd`; timeout "wfi timeout !!!" chỉ in. Bộ đếm `current_state` không bảo vệ đa luồng (không cần vì đơn luồng).

## 5. Interrupt/ngắt và chờ

U-Boot không bật ngắt (Phần 3): mọi chờ là **polling** (`mdelay`/`udelay`/`get_timer`). Không có timer ngắt để nạp WDT; do đó WDT chỉ được nạp tại các điểm gọi tường minh (mục 2.3). Bất kỳ đoạn nào chạy lâu hơn `timeout` giữa hai điểm gọi sẽ khiến R4 reset máy, ví dụ:

- Chờ 5 s giữa hai `initr_wd_refresh` (mục "0x1000|n") mà có bước tốn hơn 5 s. `init_km` (kiểm DIMM, panel UART) và `initr_bspi` nằm giữa hai lần nạp; đây là lý do KM chèn thêm WDT quanh chúng.
- Lệnh người dùng sau khi ngắt autoboot: WDT đã END nên không có giới hạn.

## 6. Failure

|Tình huống|Hành vi|
|---|---|
|R4 không phản hồi lệnh WDT|`[SCPI] Send/Response timeout`, mất ≤2 s mỗi lệnh, WDT không được nạp, boot vẫn tiếp tục|
|Timer 5 s giữa hai refresh hết|R4 reset máy (không thấy trong log U-Boot)|
|`hang()` khi chuẩn bị kernel|Reset sau `timeout` hiện hành (300 s nếu DEF_BOOT)|
|Reset đồ hoạ vượt 5 lần|`reset_with_recoverycode` ngừng reset; U-Boot tiếp tục chạy|
|Đọc BSP/Thread lỗi trong `reset_with_recoverycode`|Cũng bỏ qua reset|
|`chk_sboot` thất bại|Treo vĩnh viễn, WDT đã tắt|
|Kernel không nạp/tắt WDT|R4 reset tối đa sau 300 s|

## 7. COMMON CONFUSION

1. **"Watchdog của U-Boot"**: không tồn tại. Đó là dịch vụ của R4, U-Boot chỉ gửi lệnh.
2. **`CONFIG_WATCHDOG`/`HW_WATCHDOG`** không được định nghĩa; `INIT_FUNC_WATCHDOG_RESET` được KM ghi đè trong `board_r.c:896-900`.
3. **`END` không có nghĩa là an toàn**: sau END không còn lưới bảo vệ nào cho tới khi phần mềm kế tiếp tự bật lại.
4. **Thời gian trong `timeout`** là giây (theo cách dùng: `bootdelay*2`, `300`, `120`, `5`).
5. **`reset` không chắc kết thúc chương trình**: hàm có thể quay về.
6. **`_WAIT_FOREVER` khác `hang`**: cái đầu nạp WDT, cái sau thì không.

## 8. MENTAL MODEL

```
U-Boot(CA72) ──SCPI lệnh 0x8C (START/REFRESH/END, point, timeout)──► MHU 0xE8009800 + shmem 0x7FFF0000 (kênh 1) ──► R4 (LPPP/SCP)
                                                                                  │ hết hạn?  → khởi động lại máy
U-Boot ──I2C0 @0x3E──► PS-CPU (nguồn phụ: /RST_SLP2n, sự kiện Warp/Kernel)
U-Boot ──reset_cpu──► RFU_GLOBAL_SW_RST + HRESET_REQ(66)
Ghi SPI-flash: pause_r4 (R4 vào WFI) … resume
Lỗi đồ hoạ: reset_with_recoverycode (≤5 lần, ghi mã vào 0x2E, tắt SuperWarp)
Boot thường: WDT 300 s vẫn mở khi vào kernel → Linux tiếp nhận
```

## 9. Chưa xác minh

- Hành vi thực của R4 (hạn, reset kiểu gì, ý nghĩa `point`), vì firmware R4 ngoài repo.
- Kích thước kênh SCPI thực (128 hay 256).
- Ý nghĩa thanh ghi `0xE800800C` (resume R4).
- Việc kernel có tiếp tục nạp/tắt WDT như giả định hay không.
- Hướng của GPIO 66 và trạng thái ban đầu (do `init_km`/panel đặt).

---

Xong Phần 22. Tiếp theo **Phần 23: Cache / MMU / CPU**: `mmu_setup`, bảng trang, `dcache_*`, `arch/arm/cpu/armv8/{cache_v8.c,cache.S,tlb.S}`, LLC (`cache_llc.c`), FLC (`mvebu_flc.c`).

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\cache_v8.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\cache_v8.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 187-266)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\cache.S](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Grep "SECTION_SHIFT|SECTION_SIZE|PGTABLE_SIZE|PGTABLE_ALIGN|TCR_FLAGS|TCR_EL[123]_IPS_BITS|MEMORY_ATTRIBUTES|MT_NORMAL|MT_DEVICE_NGNRNE|MT_DEVICE_NGNRE|MT_DEVICE_GRE|MT_NORMAL_NC|PMD_SECT_|PMD_TYPE_SECT|VA_BITS|TCR_T0SZ|TCR_TG0|TCR_SHARED|TCR_ORGN|TCR_IRGN|TCR_IPS|MAIR|UL\(0x[0-9a-f]+\)|define\s+(PMD_ATTRINDX)" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\include\asm\armv8\mmu.h)

47 lines of output

Grep "enable_caches|CONFIG_SYS_DCACHE_OFF|CONFIG_SYS_ICACHE_OFF|initr_caches|dcache_enable|icache_enable|CONFIG_SYS_CACHELINE_SIZE|ARCH_DMA_MINALIGN|CONFIG_SYS_ARM_CACHE|CONFIG_SYS_NONCACHED" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {common/board_r.c,common/board_f.c,arch/arm/cpu/armv8/armada8k/*.c,arch/arm/cpu/mvebu-common/*.c,board/mvebu/armada8k/*.c,board/mvebu/common/*.c,include/configs/armada8k.h,include/configs/mvebu-common.h,arch/arm/include/asm/cache.h,arch/arm/include/asm/config.h})

17 lines of output

Grep "nvme_dcache_(disable|enable)\(\)|^ulong nvme_bread|^static ulong nvme_bread|^ulong nvme_|^int (init_nvme|scan_nvme|stop_nvme|nvme_initialize)|^static int nvme_(read|write)|^unsigned long nvme_" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\block\nvme.c)

9 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\board_r.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 149-166), [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\board_f.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 453-482)

Grep "^(int|void|static|unsigned|bool)\b.*\(|#define\s+\w+\s+(0x|\()|LLC_|FLC_|CONFIG_MVEBU_(LLC|FLC)" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\armada8k\cache_llc.c)

13 lines of output

Grep "^(int|void|static|unsigned|bool)\b.*\(|#define\s+\w+\s+(0x|\()|CONFIG_MVEBU_FLC" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\armada8k\mvebu_flc.c)

13 lines of output

Grep "cache_llc|mvebu_flc|llc_|flc" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\armada8k\Makefile)

2 lines of output

Grep "llc_|LLC|flush_l3|mvebu_soc_init|llc_enable|llc_cache_sync" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\armada8k\soc.c)

4 lines of output

Grep "flush_(dcache_range|cache)|invalidate_dcache_range|flush_dcache_all|invalidate_dcache_all|dcache_(enable|disable)" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {drivers/mmc/xenon_mmc.c,drivers/mmc/mmc.c,drivers/usb/host/xhci*.c,common/usb*.c,board/mvebu/armada8k/km/*.c,drivers/video/*.c,drivers/spi/qspi_winbond.c,drivers/sound/km-sound.c,common/lcd.c,drivers/pci/pcie-mv-msb0125.c,common/cmd_warp.c,drivers/warp/*.c})

9 lines of output

Grep "MVEBU_LLC_BASE|MVEBU_RFU_BASE|MVEBU_REGS_BASE|define\s+MVEBU_.*BASE" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\include\asm\arch-armada8k)

26 lines of output

# PHẦN 23 (PART 24): MMU / Cache / CPU

## 0. Kết luận

- **Bảng trang cực đơn giản**: một bảng "section" duy nhất, ánh xạ đồng nhất (identity), khối 512 MB, hạt trang 64 KB, VA 42 bit. Toàn bộ không gian là **Device-nGnRnE**, riêng các bank DRAM được ghi đè thành **Normal WB**. Không có XN, không có bảo vệ ghi, không có vùng non-cacheable riêng cho DMA.
- I-cache và D-cache+MMU được bật **một lần** ở `initr_caches`. Từ đó, **`nvme init` lại tắt D-cache và MMU** (vì `dcache_disable` xoá cả `SCTLR.M`), và không bật lại cho tới khi vào kernel (Phần 16/18, C3).
- KM chèn bước **clean+invalidate LLC (L3)** vào `flush_dcache_all()`.
- Không có FLC (`mvebu_flc.o` không build), không có bảo vệ bộ nhớ theo vùng.

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Cách CPU AArch64 của U-Boot dùng MMU, cache, TLB, LLC|
|WHY|Tốc độ (nạp/đọc dữ liệu lớn) và tính đúng đắn khi DMA|
|WHERE|`arch/arm/cpu/armv8/{cache_v8.c,cache.S,tlb.S,start.S,cpu.c}`, `arch/arm/include/asm/armv8/mmu.h`, `arch/arm/cpu/armv8/armada8k/cache_llc.c`, `common/board_f.c:456`, `common/board_r.c:153`|

## 2. Trình tự bật cache

```
_start/reset (start.S:65)   MMU tắt, I/D-cache tắt (trạng thái ATF bàn giao)
   apply_core_errata         (chỉ Cortex-A57, MIDR 0xD07; CA72 không có xử lý riêng — Phần 3)
   __asm_flush_dcache_all;  __asm_invalidate_icache_all;  __asm_invalidate_tlb_all
   lowlevel_init             no-op khi CurrentEL != EL3
   _main → board_init_f      (chạy KHÔNG cache)
        reserve_mmu:  tlb_size = PGTABLE_SIZE (0x10000); tlb_addr = relocaddr - 64KB, căn 64KB  → 0x3FFF0000 (Phần 2)
   relocate_code
   board_init_r → init_sequence_r:
        WDT start → initr_trace → initr_reloc → initr_caches
                                     └ enable_caches() (weak, cache_v8.c:234) = icache_enable() + dcache_enable()
```

`enable_caches` chỉ bị `misc.c:101` thay thế khi `CONFIG_SYS_DCACHE_OFF`, và macro này **đang bị comment** (mvebu-common.h:103), nên dùng bản mặc định.

`dcache_enable()` (cache_v8.c:114):

```
nếu SCTLR.M == 0:  invalidate_dcache_all(); __asm_invalidate_tlb_all(); mmu_setup();
SCTLR |= C
```

`icache_enable()`: `ic ialluis; isb` rồi `SCTLR |= I`.

## 3. `mmu_setup()` (cache_v8.c:26)

```c
page_table = gd->arch.tlb_addr;                       // 64 KB, 8192 mục × 8 byte
for (i = 0; i < PGTABLE_SIZE>>3; i++)                 // toàn bộ 4 TB (2^42)
    set_pgtable_section(pt, i, i<<29, MT_DEVICE_NGNRNE);
for (mỗi bank DRAM i < CONFIG_NR_DRAM_BANKS)          // bi_dram[].start .. start+size
    for (j = start>>29; j < end>>29; j++)
        set_pgtable_section(pt, j, j<<29, MT_NORMAL);
set_ttbr_tcr_mair(el, tlb_addr, TCR_FLAGS | TCR_ELx_IPS_BITS, MEMORY_ATTRIBUTES);   // el = EL1/EL2/EL3
SCTLR |= M
```

`set_pgtable_section`: `descriptor = section | PMD_TYPE_SECT | PMD_SECT_AF | PMD_SECT_S | ATTRINDX(type)`.

|Tham số|Giá trị|Nguồn (mmu.h)|
|---|---|---|
|Kích thước khối|`SECTION_SHIFT = 29` → **512 MB** (khối cấp 2 với granule 64KB)|:35|
|VA|`VA_BITS = 42`, `T0SZ = 22`|:24, :83|
|Granule|**64 KB** (`TCR_TG0_64K`)|:105|
|Cache-ability bảng trang|`ORGN/IRGN = WBWA`; `TCR_SHARED_NON` (non-shareable cho **table walk**)|:105-108|
|IPS|3 → **42 bit PA**|:100-102|
|MAIR|0: Device-nGnRnE `0x00`; 1: Device-nGnRE `0x04`; 2: Device-GRE `0x0c`; 3: Normal-NC `0x44`; 4: Normal WB `0xff`|:49-53|
|Shareability trong mục|`PMD_SECT_S = 3<<8` (inner-shareable)|:68|
|AP|mặc định 00 → RW||
|XN/PXN/UXN|**không đặt** (macro có, không dùng)|:71-72 vs cache_v8.c:20|

Hệ quả:

1. **Mọi ánh xạ đều thực thi được** (không XN), kể cả vùng thanh ghi Device. Đây là rủi ro theo phỏng đoán nhánh (speculative fetch vào Device); không thấy phòng chống [V bằng đọc code].
2. **Mọi ánh xạ đều RW**: không có vùng chỉ đọc (ví dụ `.text` của U-Boot), nên lỗi ghi nhầm vào mã không bị bắt.
3. **Độ phân giải 512 MB**: DRAM bank có kích thước không phải bội 512 MB sẽ bị cắt (`end >> 29` làm tròn xuống) [V: cần bank thật để kiểm]. Với board này bank0 = 0x80000000 = 4 khối nên khớp.
4. **RAM Normal-cacheable bao gồm cả vùng của ATF (0x10000000–0x105FFFFF), mailbox SCPI (0x7FFF0000) và VRAM (0x3FE10000)**: mọi thứ trong bank đều cacheable. Với các vùng chia sẻ với thiết bị khác (SCPI/R4, LCD DMA), việc đồng bộ phải bằng flush tường minh. `send_cmd` flush 64 byte (Phần 22). Với VRAM, **không thấy** lệnh flush trong `board/mvebu/armada8k/km/*.c` (grep 0 kết quả); chỉ `common/lcd.c` (2) và `cmd_warp.c` (6). Việc màn hình cập nhật đúng phụ thuộc bộ điều khiển LCD có xem cache hay không [?].
5. **Bank 1 tại 0x2_0000_0000** (8 GB): `0x2_0000_0000 >> 29 = 16 < 8192` nên nằm trong bảng.
6. Nếu chạy ở EL2, TCR dùng dạng `TCR_EL2` (một dải TTBR0_EL2), thiếu bit `RES1` nếu có; code upstream đã xử lý.

## 4. API duy trì cache

|Hàm|Thực hiện|Ghi chú|
|---|---|---|
|`flush_dcache_all()` (KM)|`__asm_flush_dcache_all` (clean+invalidate theo set/way, mọi cấp tới LoC) **+ `__asm_flush_l3_cache`**|cache_v8.c:85-89|
|`invalidate_dcache_all()`|`__asm_dcache_all(0xffff)` (chỉ invalidate, **không** clean)|dùng ở `cleanup_before_linux`|
|`flush_dcache_range`|`DC CIVAC` từng dòng (dòng nhỏ nhất theo `CTR_EL0.DminLine`)|cache.S:130|
|`invalidate_dcache_range`|**cũng là `__asm_flush_dcache_range`** (clean+invalidate)|cache_v8.c:101-104: bản này "invalidate" thực chất là clean+inv|
|`flush_cache(start,size)`|`flush_dcache_range`|cache_v8.c:243|
|`dcache_disable()` (KM)|`flush_dcache_all` → `invalidate TLB` → xoá `SCTLR.C|M` (KM đảo thứ tự so với upstream: flush trước khi tắt)|
|`icache_enable/disable`, `invalidate_icache_all`|`ic ialluis` …||

Ghi chú:

- **`invalidate_dcache_range` = clean+invalidate**: an toàn vì không mất dữ liệu của CPU, nhưng khi đệm RX vừa được thiết bị ghi và còn dòng dirty cũ trong cache thì bước clean có thể ghi đè dữ liệu DMA. Chỉ an toàn nếu bộ đệm đã được `invalidate` trước khi DMA (đúng quy trình thông thường).
- `__asm_dcache_all` duyệt `CLIDR_EL1` từ cấp 0 tới LoC; bỏ qua cấp không có D/unified (`cache.S:88-89`).
- `ARCH_DMA_MINALIGN = 64` (`CONFIG_SYS_CACHELINE_SIZE 64`, mvebu-common.h:104); trùng độ dài dòng của A72.
- `CONFIG_SYS_NONCACHED_MEMORY` **không** định nghĩa: không có vùng DMA non-cacheable, mọi DMA dùng bộ đệm cache-aligned + flush/inv.

### 4.1 LLC (L3) và bước KM

`MVEBU_LLC_BASE = MVEBU_REGS_BASE + 0x8000` (regs-base.h:56); với `MVEBU_REGS_BASE = 0xF0000000` (theo các địa chỉ 0xF06F0000 RFU, 0xF0512000 UART đã gặp) tương ứng **0xF0008000**.

|Thanh ghi|Offset|Ý nghĩa|
|---|---|---|
|`LLC_CTRL`|0x100|bit0 = EN; bit `LLC_EXCLUSIVE_EN` (0x100)|
|`LLC_SYNC`|0x700|đồng bộ|
|`LLC_MNT_CIW`|0x7FC|clean-invalidate theo way|
|`LLC_MNT_CW`|0x7BC|clean theo way|

- Bản C (`cache_llc.c:62`, `flush_l3_cache`): nếu LLC bật thì ghi `0xFFFFFFFF` vào `0x7FC` rồi `1` vào `0x700`.
- Bản KM `__asm_flush_l3_cache` (cache.S:160): làm y hệt bằng asm (đọc `0xF0008100`, nếu bit0 thì ghi `0xF00087FC = 0xFFFFFFFF`, `0xF0008700 = 1`, `dsb sy`). `flush_dcache_all` của KM gọi thẳng bản asm, bỏ bản `weak flush_l3_cache`.
- Vì `flush_dcache_all` được gọi trong `dcache_disable`, `warp_prepare_boot` và các đường `nvme_dcache_disable`, **mỗi lần tắt D-cache đều tốn một lần clean+invalidate toàn bộ LLC** (thời gian tỉ lệ dung lượng LLC).
- `print_soc_specific_info` chỉ in trạng thái `LLC Enabled/Disabled (Exclusive Mode)`. Không có mã bật/tắt LLC trong U-Boot (do ATF cấu hình).

### 4.2 FLC (Marvell "Fabric/Flow Level Cache"?)

`mvebu_flc.c` (registers `flc_base+0x0…0xc`, mask `0xFF800000`) chỉ được build khi `CONFIG_MVEBU_FLC` (`arch/.../armada8k/Makefile:28`), không có trong defconfig, nên **không** hiện diện. `CONFIG_MVEBU_FLC` cũng không xuất hiện trong header board.

## 5. Ba trạng thái cache/MMU trong đời sống U-Boot

|Giai đoạn|I-cache|D-cache|MMU|Ghi chú|
|---|---|---|---|---|
|`_start` … `board_init_f`|Tắt|Tắt|Tắt|Truy cập RAM là Device (chậm, cần căn hàng)|
|Sau `initr_caches`|Bật|Bật|Bật|Trạng thái chuẩn|
|Sau `nvme init` (SSD)|Bật|**Tắt**|**Tắt**|Chưa bật lại vì `nvme_read/write` không đụng cache và `stop_nvme` không được gọi|
|`cleanup_before_linux`|Tắt|Tắt|Tắt|Trạng thái bàn giao cho kernel|

Khi MMU tắt: mọi truy cập dữ liệu là Device-nGnRnE (không cache, không gộp, không reorder, **chống truy cập không căn hàng**). Mọi thứ U-Boot làm sau `nvme init`: nạp `Image` (hàng chục MB), `memmove`, CRC32 initrd, `sha256` (tự chẩn đoán), giải mã ảnh panel... đều chạy ở chế độ này. Hiệu năng suy giảm nhiều (chưa đo được, Phần 38) [S]. Nhánh SD/MMC (Xenon) không có hiện tượng này (D-cache bật xuyên suốt).

## 6. Đồng bộ với DMA theo từng thiết bị (đã kiểm)

|Thiết bị|Cách|Mức xác minh|
|---|---|---|
|NVMe|Tắt D-cache toàn cục (`NVME_DCACHE_OFF`), tránh bảo trì cache|[V]|
|xHCI|`flush_dcache_range`/`invalidate_dcache_range` trong `xhci-mem.c` (8 điểm), `xhci-ring.c` (6), `xhci.c` (1)|[V số lượng, chi tiết chưa đọc]|
|Xenon SDHCI|1 điểm `flush_*` trong `xenon_mmc.c` (Phần 12 đã thấy flush trước lệnh; còn thiếu invalidate sau DMA đọc)|[?]|
|SCPI/MHU|`flush_dcache_range` 64B khi gửi, `invalidate_dcache_range` khi chờ trạng thái MHU|[V]|
|Warp (`cmd_warp.c`)|6 điểm|[chưa đọc]|
|LCD/VRAM|`common/lcd.c` 2 điểm; KM panel code không flush|[?]|
|QSPI|`memcpy_fromio` từ cửa sổ 0xF4000000 (Device) nên không cần cache|Phần 12|

## 7. TLB và EL

- `__asm_invalidate_tlb_all` (tlb.S): chọn theo EL (`switch_el` với `tlbi alle3/alle2/alle1`).
- `current_el()` chỉ để quyết định thanh ghi TTBR/TCR/SCTLR tương ứng. Thực tế (dưới ATF) là EL2 hoặc EL1 (Phần 18).
- Không có thay đổi TTBR sau khi bật (không bảng trang động, không `mmap` runtime).
- Không dùng SMP: chỉ lõi chính. Bảng trang không được chia sẻ với lõi khác.

## 8. CPU / lõi

- Lõi thực là **Cortex-A72** (mã KM `ca72wdt.c`; DTB điều khiển ghi `arm,cortex-a57` nên có sai khác, Phần 0).
- Errata chỉ cho A57 (MIDR `0xD07`); `start.S:134` `branch_if_a57_core`. **Không có errata cho A72** trong ảnh này.
- `COUNTER_FREQUENCY` chỉ đặt `CNTFRQ_EL0` khi ở EL3 (`start.S:77`), dưới ATF do ATF làm.
- `mrs midr_el1`... không có `cpu_info` in trong log (Phần 39).
- PSCI/`spin-table`: Phần 18.

## 9. Failure và cạm bẫy

|Điều kiện|Hệ quả|
|---|---|
|Truy cập không căn hàng khi MMU tắt (đường NVMe)|Alignment fault (data abort), vào handler ngoại lệ (Phần 28). Không có `-mstrict-align` [?]|
|DMA vào vùng cacheable mà không invalidate|Dữ liệu cũ (không thấy trường hợp cụ thể, nhưng xenon còn [?])|
|Vùng RAM cuối bank không chia hết 512 MB|Phần dư không được map Normal|
|Ghi nhầm vào `.text`|Không bị chặn (không có W^X)|
|`dcache_disable` khi `SCTLR.C==0`|Thoát sớm và **không** clean; `invalidate_dcache_all` sau đó vẫn xoá cache (an toàn vì cache đã sạch)|

## 10. COMMON CONFUSION

1. **"Bật D-cache là bật MMU"**: đúng ở ARMv8 U-Boot: `dcache_enable` bật cả MMU, `dcache_disable` tắt cả hai.
2. **"Tắt D-cache chỉ làm chậm một chút"**: MMU tắt biến RAM thành Device, có thể chậm hàng chục lần và cần căn hàng.
3. **"`invalidate_dcache_range` chỉ invalidate"**: ở đây là clean+invalidate.
4. **"LLC do U-Boot cấu hình"**: chỉ được flush, không bật/tắt.
5. **"SECTION = 2 MB"**: ở đây là 512 MB (granule 64 KB).
6. **"TLB ở gần RAM top nên bảo vệ được"**: không có bảo vệ; `mmu_setup` cho ghi vào mọi nơi.

## 11. MENTAL MODEL

```
Bảng trang: [Device 4TB] + [đè: các bank DRAM → Normal WB], 512MB/khối, 64KB granule, RWX
bật: initr_caches → I+D+MMU on
     nvme init → D+MMU off (giữ đến khi vào kernel)
flush_dcache_all = set/way (L1,L2) + LLC clean-inv (0xF00087FC/0xF0008700)
bàn giao: cleanup_before_linux → I,D,MMU off
```

## 12. Chưa xác minh

- Kích thước/tên của cache thực (CLIDR/CCSIDR) và tổng thời gian `__asm_flush_l3_cache`.
- Việc phần cứng LCD có coherent với cache CPU không (VRAM).
- Đọc chi tiết cách `xhci-*.c`, `xenon_mmc.c` và `cmd_warp.c` bảo trì cache.
- Có `-mstrict-align` không (grep `arch/arm/config.mk`/`Makefile` trống, nhưng có thể nằm ở `Makefile` gốc).
- Bố cục bank DRAM thực để xét việc cắt theo 512 MB.

---

Xong Phần 23. Tiếp theo **Phần 24: DMA / Interrupt**: các thiết bị DMA (Xenon SDMA, NVMe, xHCI, I2S CDMA, LCD), stub ngắt, GIC, `CONFIG_USE_IRQ`.

Grep "CDMA|cdma|dma_|DMA" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\sound\km-sound.c)

18 lines of output

Grep "interrupt|irq|IRQ|GIC|enable_interrupts|disable_interrupts" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\lib\interrupts_64.c)

7 lines of output

Grep "CONFIG_GICV[23]|GICD_BASE|GICC_BASE|GICR_BASE|CONFIG_USE_IRQ|CONFIG_STACKSIZE_IRQ" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {include/configs/armada8k.h,include/configs/mvebu-common.h,arch/arm/include/asm/arch-armada8k/*.h,arch/arm/include/asm/arch-mvebu/*.h,configs/km_mvebu_quartz_toc_defconfig,arch/arm/cpu/armv8/armada8k/Kconfig})

10 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\lib\interrupts_64.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Globpattern: "drivers/sound/*"

Found 19 files

Grep "gic_kick_secondary_cpus|gic_init_secure|gic_wait_for_interrupt" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8, glob: *.S)

11 lines of output

Grep "gic_kick_secondary_cpus|GICD_SGIR|GICD_CTLR|ENTRY\(" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\lib, glob: gic*)

10 lines of output

Grep "^(int|void|static|unsigned|long|bool|u32)\b.*\(|#define\s+(CDMA|DEF_CDMA|.*BASE|.*REG)|ioremap|flush_|invalidate_|dcache|desc|descriptor|timeout|mdelay|udelay" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\sound\km-cdma.c)

16 lines of output

Grep "CONFIG_PANIC_HANG" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {include/configs/armada8k.h,include/configs/mvebu-common.h,include/configs/quartz-sb.h,include/config_defaults.h,include/config_fallbacks.h,lib/panic.c,common/panic.c,configs/km_mvebu_quartz_toc_defconfig})

1 line of output

Grep "panic|hang|do_reset|udelay|CONFIG_PANIC" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\lib\panic.c)

1 line of output

Grep "udelay|get_timer|timer_get_us|__udelay|timer_init|get_tbclk|CNTPCT|cntpct|MVEBU_GENERIC_TIMER|read_cntpct|get_ticks" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\mvebu-common\generic_timer.c)

3 lines of output

Grep "void panic\(|PANIC_HANG" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {common/*.c,lib/*.c})

51 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\mvebu-common\generic_timer.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

# PHẦN 24 (PART 25): DMA / Interrupt

## 0. Kết luận

- **Không có ngắt nào được dùng.** Mọi thiết bị đều **polling**. Cờ `CONFIG_USE_IRQ` được định nghĩa (armada8k.h:295, ghi đè `#undef` ở mvebu-common.h:75) nhưng `interrupt_init`, `enable_interrupts`, `disable_interrupts` đều là hàm rỗng (`arch/arm/lib/interrupts_64.c:12-25`). Nếu có IRQ/FIQ thực sự xảy ra, handler in thanh ghi rồi **reset máy**.
- **DMA thì có, ở 5 nơi**: NVMe (PCIe bus master), Xenon SDHCI (SDMA), xHCI, CDMA âm thanh (I2S), bộ điều khiển LCD (quét VRAM). Không có bộ điều khiển DMA đa dụng nào được dùng, không IOMMU/SMMU, không vùng non-cacheable riêng.
- Panic **reset máy** (không treo), nên một lỗi ngoại lệ lặp lại có thể tạo vòng khởi động lại.

Cập nhật cho Phần 18: `CONFIG_GICV2` **có** (armada8k.h:49, `#undef CONFIG_GICV3`), với `GICD_BASE = 0xF0210000`, `GICC_BASE = 0xF0220000`. Vì vậy `smp_kick_all_cpus()` (start.S:220) thực sự gọi `gic_kick_secondary_cpus(GICD_BASE)` (gic_64.S:162), ghi vào `GICD_SGIR` để gửi SGI 0 tới các lõi khác. Dưới ATF, các lõi phụ đang do ATF/PSCI quản lý, nên tác dụng thực tế của SGI này không chắc chắn [?].

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Cách thiết bị chuyển dữ liệu (DMA) và cách CPU biết thiết bị xong (ngắt hay polling)|
|WHY|Hiểu vì sao có các thao tác cache, cấu trúc `struct` căn 64 byte, vòng chờ có timeout|
|WHERE|`arch/arm/lib/interrupts_64.c`, `arch/arm/lib/gic_64.S`, `arch/arm/cpu/armv8/{start.S,exceptions.S}`, `arch/arm/cpu/mvebu-common/generic_timer.c`, `drivers/block/nvme.c`, `drivers/mmc/xenon_mmc.c`, `drivers/usb/host/xhci*.c`, `drivers/sound/km-cdma.c`, `lib/vsprintf.c:845`|

## 2. Ngắt: cấu hình thực

|Mục|Trạng thái|Nguồn|
|---|---|---|
|`interrupt_init`|trả 0, không làm gì|interrupts_64.c:12|
|`enable_interrupts`|rỗng|:17|
|`disable_interrupts`|trả 0 (không thay đổi DAIF)|:22|
|Cờ `DAIF`|Không do U-Boot chủ động; giữ theo trạng thái ATF bàn giao (thường che)|[S]|
|GIC|`CONFIG_GICV2` bật; `gic_init_secure*` chỉ chạy khi `CurrentEL == EL3` (start.S:170-191), dưới ATF thì bỏ qua|start.S|
|`CONFIG_STACKSIZE_IRQ`|4 KB được khai báo nhưng không có vùng ngăn xếp IRQ riêng được dùng|armada8k.h:296|
|Timer ngắt|Không dùng. `timer_init` chỉ bật **system counter** (`GTC_CNTCR bit0` tại `MVEBU_GENERIC_TIMER_BASE = 0xF0581000`), nhận xét "timer is not needed for ARMv8"|generic_timer.c:26|

Hệ quả: các thiết bị **không thể** báo ngắt tới U-Boot. Mọi chờ đợi là vòng lặp có `mdelay/udelay/get_timer` (ví dụ chờ hoàn tất NVMe, chờ MHU, chờ PHY USB).

### Khi có ngoại lệ hoặc IRQ

`exceptions.S` gom trạng thái rồi gọi các hàm trong `interrupts_64.c`:

|Hàm|Kết quả|
|---|---|
|`do_sync`, `do_irq`, `do_fiq`, `do_error` (weak)|in `"<loại> handler, esr 0x…"`, `show_regs` (ELR, LR, x0–x29), rồi `panic("Resetting CPU ...")`|
|`do_bad_sync/irq/fiq/error`|tương tự với chuỗi "Bad mode in …"|

`panic()` (`lib/vsprintf.c:845`): in thông báo; **`CONFIG_PANIC_HANG` không định nghĩa** nên chạy nhánh `udelay(100000); do_reset(NULL,0,0,NULL);` rồi `while(1)`. `do_reset` gọi `reset_cpu`: `RFU_GLOBAL_SW_RST` + `HRESET_REQ` (Phần 22). Nghĩa là **mọi ngoại lệ trong U-Boot dẫn tới khởi động lại toàn máy** sau ~150 ms. Khác với `reset_with_recoverycode`, **không có bộ đếm giới hạn**: nếu ngoại lệ xảy ra ổn định ở một giai đoạn boot, máy sẽ lặp khởi động lại vô hạn (có thể gọn trong vài giây mỗi vòng). Chi tiết ngoại lệ ở Phần 28.

## 4. DMA: bảng thiết bị

|Thiết bị|Dạng DMA|Nơi|Đồng bộ cache|Ghi chú|
|---|---|---|---|---|
|**NVMe SSD**|PCIe bus master, PRP, hàng đợi admin + 1 I/O, độ sâu ≤128|`drivers/block/nvme.c`|**Tắt D-cache toàn cục** sau `init_nvme` (`NVME_DCACHE_OFF`), không có flush/inv theo vùng|Bộ đệm phải **căn 64 byte**, nếu không hàm trả 0 (Phần 12)|
|**Xenon SDHCI (SD/MMC)**|SDMA 32-bit hoặc PIO (khi `has_init==0`), có bộ đệm bounce|`xenon_mmc.c`|1 lệnh `flush_*` trước lệnh; **chưa thấy invalidate sau khi đọc**|[?] Phần 12|
|**xHCI (USB)**|Bus master, ring/context trong bộ nhớ heap căn hàng|`xhci-mem.c` (8 điểm), `xhci-ring.c` (6), `xhci.c` (1)|flush trước ghi, invalidate trước đọc|Chi tiết chưa đọc|
|**CDMA (âm thanh I2S)**|Bộ điều khiển DMA AXI riêng, mô tả 1 phần tử, `CDMA_REG7_CLDSR/CUDSR` nhận địa chỉ mô tả 64 bit|`drivers/sound/km-cdma.c`|Chỉ flush **mô tả**|Xem 4.1|
|**LCD (panel)**|Bộ điều khiển quét VRAM (0x3FE10000, 0x1E0000 byte)|`drivers/video/*`, `common/lcd.c`|Không thấy flush trong code panel của KM|[?]|
|QSPI|Không DMA (đọc qua cửa sổ MMIO 0xF4000000, ghi qua thanh ghi)|`qspi_winbond.c`|||
|MHU/SCPI|Không phải DMA (hộp thư + ngắt phần cứng, nhưng U-Boot poll)|`arm_scpi.c`|flush 64B/invalidate|Phần 22|

### 4.1 CDMA (km-cdma.c)

```
cdma_start(src, len, dest=SSPA_TXD, pid=SSPA_CDMA_PID)
   static CDMA_AXI_DESC4_REGS_t st_cdma_desc __aligned(ARCH_DMA_MINALIGN)    // 64B
   dma_length = len & mask | OWN;  source = src;  dest = SSPA_TXD;  next = STOP
   flush_dcache_range((ulong)&st_cdma_desc, (ulong)sizeof(st_cdma_desc));   ← ĐỐI SỐ THỨ HAI LÀ KÍCH THƯỚC, KHÔNG PHẢI ĐỊA CHỈ CUỐI
   writel(virt_to_phys(&desc),      CDMA_REG7_CLDSR)
   writel(virt_to_phys(&desc)>>32,  CDMA_REG7_CUDSR)
   ... bật kênh
cdma_is_busy() poll; cdma_stop()  chờ có udelay(1), timeout "cdma busy check timeout"
```

- **Lỗi API**: `flush_dcache_range(start, stop)` nhận địa chỉ cuối, nhưng code truyền `sizeof(desc)`. `__asm_flush_dcache_range` là vòng `do { dc civac,x0; x0 += line } while (x0 < x1)`, nên với `x1` nhỏ vẫn thực hiện **đúng một** dòng cache (do-while). Mô tả 64 byte căn 64 nằm gọn một dòng nên vẫn chạy đúng, nhưng chỉ đúng do tình cờ [V bằng đọc `cache.S:130-145`].
- Dữ liệu nguồn `gus_km_boot_sound[]` không được flush trước DMA. Nó là dữ liệu hằng nằm trong ảnh U-Boot, được chép ra RAM bởi `relocate_code` khi cache còn tắt, nên nằm sẵn trong RAM và không dirty trong cache [S]. Nếu dữ liệu bị ghi lại về sau thì sẽ sai.
- Việc bật/tắt clock của khối âm thanh (`sound_clock_gate` cho `CDMA`/I2S) được làm thủ công quanh mỗi lần phát.
- `sound_stop()` được gọi nhiều chỗ ở `main_loop` (Phần 16) để dừng DMA trước khi boot tiếp.

## 5. Địa chỉ DMA và không gian địa chỉ

- **Không có IOMMU**: `dma_addr_t = virt_to_phys(...)` và ánh xạ đồng nhất (identity), nên địa chỉ CPU = địa chỉ bus tới RAM (Phần 23). Hàm `virt_to_phys` trong `nvme.c`, `km-cdma.c` chỉ ép kiểu.
- **Xenon SDMA 32-bit**: buffer phải dưới 4 GiB. U-Boot cấp phát dưới 1 GiB (`relocaddr` top 0x40000000) nên đúng; buffer người dùng (ví dụ `fatload` tới 0x800000/0x3000000) cũng thấp. Bank 1 ở 0x2_0000_0000 sẽ vượt giới hạn nếu ai chỉ tới đó (không có ở đường boot) [S].
- **PCIe (NVMe)**: cần cửa sổ inbound của RC bao phủ RAM (thiết lập trong `pcie-mv-msb0125.c`, Phần 12). Chưa xác minh dải bao phủ so với bank thứ hai [?].
- **xHCI 64-bit** cho phép đặt ring ở bất kỳ đâu; dùng heap dưới 1 GiB nên an toàn.
- **MMU tắt sau `nvme init`**: mọi DMA đọc/ghi dữ liệu do CPU chuẩn bị sau thời điểm này đều đi thẳng RAM (không cache), nên đồng bộ với DMA "tự nhiên" đúng, và đó là lý do thiết kế `NVME_DCACHE_OFF` tồn tại.

## 6. Timer và chờ

- Nguồn thời gian là **ARM generic system counter** (bật bởi `timer_init`; `get_timer`/`udelay` dựa vào `CNTPCT`/`CNTFRQ`, `COUNTER_FREQUENCY` chỉ đặt tại EL3). Đây là cơ sở của mọi timeout (`ADMIN_TIMEOUT`, `SENDCMD_TIMEOUT_MS = 1000`, `I2C_BYTE_TO`, `mdelay(10000)`).
- Không có timer ngắt: `get_timer(0)` đọc bộ đếm rồi so sánh (`nvme_wait_completion_queue`, `start_time_rx` trong `dw_i2c_read`).
- Có **vòng chờ không timeout** ở `phy_utm_mv_init` (Phần 20) và `mtest`, `cdma` không.

## 7. Failure

|Điều kiện|Hệ quả|
|---|---|
|IRQ/FIQ/SError xuất hiện|In thanh ghi, `panic` → reset (Phần 28)|
|DMA vào buffer không căn 64B (NVMe)|Hàm trả 0 khối; `fatload` báo lỗi|
|DMA với cache bẩn (Xenon nếu thiếu invalidate)|Dữ liệu cũ (nghi vấn, chưa xác nhận)|
|`flush_dcache_range` sai đối số (CDMA)|Tình cờ đúng (1 dòng); sai nếu mô tả > 64B hoặc không căn 64B|
|Thiết bị không hoàn tất|Vòng chờ hết timeout hoặc treo (`phy_utm_mv_init`)|
|Panic lặp|Vòng khởi động lại vô hạn cho tới khi lỗi biến mất|

## 8. COMMON CONFUSION

1. **`CONFIG_USE_IRQ` bật = có ngắt**: không. Hàm ngắt là stub.
2. **GICv2 khởi tạo bởi U-Boot**: chỉ ở EL3; dưới ATF không.
3. **"DMA cần `dma_alloc_coherent`"**: không có, dùng `memalign` + cache maintenance (hoặc tắt D-cache).
4. **`flush_dcache_range(start, size)`**: chữ ký thật là `(start, stop)`.
5. **`panic` treo máy**: ở đây panic **reset máy**.
6. **Bộ đếm hệ thống ≠ timer ngắt**: chỉ để đo thời gian.

## 9. MENTAL MODEL

```
CPU ──ghi hàng đợi/mô tả (RAM)──► thiết bị (NVMe/xHCI/SD/CDMA/LCD) ──DMA──► RAM
CPU ──polling (get_timer)──► chờ hoàn tất        [không có ngắt]
Cache: NVMe = tắt D-cache; xHCI/SD = flush/inv theo vùng; CDMA = flush mô tả
IRQ/FIQ/SError/Sync abort ──► in regs ──► panic ──► do_reset (không có bộ đếm)
```

## 10. Chưa xác minh

- Ngoại lệ đồng bộ bên trong `exceptions.S` (Phần 28).
- Chi tiết bảo trì cache của `xhci-*.c`, `xenon_mmc.c` (đọc sau).
- Cửa sổ inbound PCIe có bao phủ toàn bộ RAM (kể cả bank ≥ 4 GiB) không.
- Ý nghĩa `CONFIG_STACKSIZE_IRQ` (không có vùng IRQ stack riêng trong `crt0_64.S`).
- Coherency của LCD với cache CPU.

---

Xong Phần 24. Tiếp theo **Phần 25: Build system**: Makefile gốc, Kbuild, `config.mk`, `scripts/Makefile.*`, `build_uboot.sh`, quá trình sinh `u-boot.bin`, DTB nhúng, cờ biên dịch, thư viện, `autoconf.mk`.

Bash List repo root and scripts directory

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && ls -la | head -60 && ls scripts | head -50

OUT

total 1218
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 .
drwxr-xr-x 1 PC 197121      0 Aug 14 02:19 ..
-rw-r--r-- 1 PC 197121   2279 Aug 14 02:19 Kbuild
-rw-r--r-- 1 PC 197121   6607 Aug 14 02:19 Kconfig
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 Licenses
-rw-r--r-- 1 PC 197121  10962 Aug 14 02:19 MAINTAINERS
-rwxr-xr-x 1 PC 197121  23420 Aug 14 02:19 MAKEALL
-rw-r--r-- 1 PC 197121  48890 Aug 14 02:19 Makefile
-rw-r--r-- 1 PC 197121 236824 Aug 14 02:19 README
drwxr-xr-x 1 PC 197121      0 Aug 14 02:19 api
drwxr-xr-x 1 PC 197121      0 Aug 14 02:19 arch
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 board
-rwxr-xr-x 1 PC 197121   2931 Aug 14 02:19 build_uboot.sh
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 common
-rw-r--r-- 1 PC 197121   2421 Aug 14 02:19 config.mk
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 configs
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 disk
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 doc
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 drivers
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 dts
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 examples
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 fs
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 include
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 lib
-rw-r--r-- 1 PC 197121     15 Aug 14 02:19 localversion
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 net
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 post
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 scripts
-rw-r--r-- 1 PC 197121     17 Aug 14 02:19 snapshot.commit
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 test
drwxr-xr-x 1 PC 197121      0 Aug 14 02:20 tools
Kbuild.include
Lindent
Makefile
Makefile.autoconf
Makefile.build
Makefile.clean
Makefile.extrawarn
Makefile.host
Makefile.lib
Makefile.spl
basic
bin2phex.pl
binutils-version.sh
checkpatch.pl
checkstack.pl
cleanpatch
docproc.c
dtc-version.sh
gcc-stack-usage.sh
gcc-version.sh
get_maintainer.pl
kconfig
kernel-doc
ld-version.sh
mailmapper
mkmakefile
multiconfig.sh
objdiff
setlocalversion
show-gnu-make
tim2img.pl
tim2phex.pl

Grep "CROSS_COMPILE|ARCH|KBUILD_CFLAGS|KBUILD_CPPFLAGS|-O2|-Os|-Wall|-fno-|-ffreestanding|-g\b|CONFIG_CC_OPTIMIZE|LDFLAGS|-Ttext|u-boot\.(bin|srec|img|dtb|lds)|OBJCOPYFLAGS|ALL-y|ALL-\$|_all:|all:|MKIMAGE|KM_BIZHUB|-I board" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\Makefile)

91 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\build_uboot.sh](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\config.mk](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\config.mk](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#), [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\Makefile](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 341-370)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\config.mk](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\Makefile](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 496-570)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\Makefile](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 601-675), [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\Makefile](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 706-840)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\Makefile](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 673-712)

Bash List tools dir, marvell subdir, relocate-rela

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && ls tools | head -80; echo ---; ls tools/marvell 2>/dev/null | head; echo ---; ls tools/relocate-rela* 2>/dev/null

OUT

Makefile
aisimage.c
aisimage.h
atmel_pmecc_params.c
atmelimage.c
bin2header.c
bmp_logo.c
buildman
default_image.c
dumpimage.c
dumpimage.h
easylogo
env
envcrc.c
fdt_host.h
fit_check_sign.c
fit_common.c
fit_common.h
fit_image.c
fit_info.c
gdb
gen_eth_addr.c
genboardscfg.py
getline.c
getline.h
gpheader.h
gpimage-common.c
gpimage.c
ifdtool.c
ifdtool.h
image-host.c
imagetool.c
imagetool.h
img2brec.sh
img2srec.c
imximage.c
imximage.h
jtagconsole
kermit
kwbimage.c
kwbimage.h
kwboot.c
logos
marvell
microcode-tool
microcode-tool.py
mingw_support.c
mingw_support.h
mkenvimage.c
mkexynosspl.c
mkimage.c
mkimage.h
mksunxiboot.c
mpc86x_clk.c
mxsboot.c
mxsimage.c
mxsimage.h
ncb.c
netconsole
omap
omapimage.c
omapimage.h
os_support.c
os_support.h
palmtreo680
patman
pbl_crc32.c
pbl_crc32.h
pblimage.c
pblimage.h
proftool.c
relocate-rela.c
scripts
secure
socfpgaimage.c
ublimage.c
ublimage.h
ubsha1.c
xway-swap-bytes.c
---
BCH
bin_hdr
bin_hdr.uart
debugger_scripts
doimage
dtc
params.raw
xmodem
---
tools/relocate-rela.c

Grep "relocate-rela|STATIC_RELA" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {Makefile,tools/Makefile,scripts/*,arch/arm/Makefile,arch/arm/cpu/armv8/*,arch/arm/lib/Makefile})

6 lines of output

Bash Show define2mk.sed and relocate-rela.c

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && cat tools/scripts/define2mk.sed; echo ----; sed -n 1,200p tools/relocate-rela.c

OUT

#
# Sed script to parse CPP macros and generate output usable by make
#
# It is expected that this script is fed the output of 'gpp -dM'
# which preprocesses the common.h header files and outputs the final
# list of CPP macros (and whitespace is sanitized)
#

# Only process values prefixed with #define CONFIG_
/^#define CONFIG_[A-Za-z0-9_][A-Za-z0-9_]*/ {
	# Strip the #define prefix
	s/#define *//;
	# Change to form CONFIG_*=VALUE
	s/  */=/;
	# Drop trailing spaces
	s/ *$//;
	# drop quotes around string values
	s/="\(.*\)"$/=\1/;
	# Concatenate string values
	s/" *"//g;
	# Assume strings as default - add quotes around values
	s/=\(..*\)/="\1"/;
	# but remove again from decimal numbers
	s/="\([0-9][0-9]*\)"/=\1/;
	# ... and from hex numbers
	s/="\(0[Xx][0-9a-fA-F][0-9a-fA-F]*\)"/=\1/;
	# ... and from configs defined from other configs
	s/="\(CONFIG_[A-Za-z0-9_][A-Za-z0-9_]*\)"/=$(\1)/;
	# Change '1' and empty values to "y" (not perfect, but
	# supports conditional compilation in the makefiles
	s/=$/=y/;
	s/=1$/=y/;
	# print the line
	p
}
----
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+ BSD-2-Clause
 *
 * 64-bit and little-endian target only until we need to support a different
 * arch that needs this.
 */

#include <elf.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef R_AARCH64_RELATIVE
#define R_AARCH64_RELATIVE	1027
#endif

static const bool debug_en;

static void debug(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	if (debug_en)
		vprintf(fmt, args);
}

static bool supported_rela(Elf64_Rela *rela)
{
	uint64_t mask = 0xffffffffULL; /* would be different on 32-bit */
	uint32_t type = rela->r_info & mask;

	switch (type) {
#ifdef R_AARCH64_RELATIVE
	case R_AARCH64_RELATIVE:
		return true;
#endif
	default:
		fprintf(stderr, "warning: unsupported relocation type %"
				PRIu32 " at %" PRIx64 "\n",
			type, rela->r_offset);

		return false;
	}
}

static inline uint64_t swap64(uint64_t val)
{
	return ((val >> 56) & 0x00000000000000ffULL) |
	       ((val >> 40) & 0x000000000000ff00ULL) |
	       ((val >> 24) & 0x0000000000ff0000ULL) |
	       ((val >>  8) & 0x00000000ff000000ULL) |
	       ((val <<  8) & 0x000000ff00000000ULL) |
	       ((val << 24) & 0x0000ff0000000000ULL) |
	       ((val << 40) & 0x00ff000000000000ULL) |
	       ((val << 56) & 0xff00000000000000ULL);
}

#if __BYTE_ORDER == __LITTLE_ENDIAN
static inline uint64_t be64(uint64_t val)
{
	return swap64(val);
}

static inline uint64_t le64(uint64_t val)
{
	return val;
}
#else
static inline uint64_t le64(uint64_t val)
{
	return swap64(val);
}

static inline uint64_t be64(uint64_t val)
{
	return val;
}
#endif

static bool read_num(const char *str, uint64_t *num)
{
	char *endptr;
	*num = strtoull(str, &endptr, 16);
	return str[0] && !endptr[0];
}

int main(int argc, char **argv)
{
	FILE *f;
	int i, num;
	uint64_t rela_start, rela_end, text_base;

	if (argc != 5) {
		fprintf(stderr, "Statically apply ELF rela relocations\n");
		fprintf(stderr, "Usage: %s <bin file> <text base> " \
				"<rela start> <rela end>\n", argv[0]);
		fprintf(stderr, "All numbers in hex.\n");
		return 1;
	}

	f = fopen(argv[1], "r+b");
	if (!f) {
		fprintf(stderr, "%s: Cannot open %s: %s\n",
			argv[0], argv[1], strerror(errno));
		return 2;
	}

	if (!read_num(argv[2], &text_base) ||
	    !read_num(argv[3], &rela_start) ||
	    !read_num(argv[4], &rela_end)) {
		fprintf(stderr, "%s: bad number\n", argv[0]);
		return 3;
	}

	if (rela_start > rela_end || rela_start < text_base ||
	    (rela_end - rela_start) % sizeof(Elf64_Rela)) {
		fprintf(stderr, "%s: bad rela bounds\n", argv[0]);
		return 3;
	}

	rela_start -= text_base;
	rela_end -= text_base;

	num = (rela_end - rela_start) / sizeof(Elf64_Rela);

	for (i = 0; i < num; i++) {
		Elf64_Rela rela, swrela;
		uint64_t pos = rela_start + sizeof(Elf64_Rela) * i;
		uint64_t addr;

		if (fseek(f, pos, SEEK_SET) < 0) {
			fprintf(stderr, "%s: %s: seek to %" PRIx64
					" failed: %s\n",
				argv[0], argv[1], pos, strerror(errno));
		}

		if (fread(&rela, sizeof(rela), 1, f) != 1) {
			fprintf(stderr, "%s: %s: read rela failed at %"
					PRIx64 "\n",
				argv[0], argv[1], pos);
			return 4;
		}

		swrela.r_offset = le64(rela.r_offset);
		swrela.r_info = le64(rela.r_info);
		swrela.r_addend = le64(rela.r_addend);

		if (!supported_rela(&swrela))
			continue;

		debug("Rela %" PRIx64 " %" PRIu64 " %" PRIx64 "\n",
		      swrela.r_offset, swrela.r_info, swrela.r_addend);

		if (swrela.r_offset < text_base) {
			fprintf(stderr, "%s: %s: bad rela at %" PRIx64 "\n",
				argv[0], argv[1], pos);
			return 4;
		}

		addr = swrela.r_offset - text_base;

		if (fseek(f, addr, SEEK_SET) < 0) {
			fprintf(stderr, "%s: %s: seek to %"
					PRIx64 " failed: %s\n",
				argv[0], argv[1], addr, strerror(errno));
		}

		if (fwrite(&rela.r_addend, sizeof(rela.r_addend), 1, f) != 1) {
			fprintf(stderr, "%s: %s: write failed at %" PRIx64 "\n",
				argv[0], argv[1], addr);
			return 4;
		}
	}

	if (fclose(f) < 0) {
		fprintf(stderr, "%s: %s: close failed: %s\n",
			argv[0], argv[1], strerror(errno));
		return 4;
	}

	return 0;
}

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\lib\relocate_64.S](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\Makefile](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 837-966)

Grep "checkarmreloc|checkdtc|SYS_TEXT_BASE" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {Makefile,arch/arm/Makefile,arch/arm/Kconfig,board/mvebu/Kconfig,board/mvebu/armada8k/Kconfig,arch/arm/cpu/armv8/armada8k/Kconfig,include/configs/armada8k.h,include/configs/mvebu-common.h})

21 lines of output

Grep "^u-boot:|cmd_u-boot__|cmd_smap|System.map|-Map|u-boot.map|LDFLAGS_u-boot|^mrproper|^clean:|^%_defconfig|defconfig|sub-make|^config:|^silentoldconfig|scripts/kconfig|^prepare|^archprepare|^include/config.h|^include/generated|asm-offsets|timestamp|version|include/autoconf.mk" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\Makefile)

69 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\Makefile](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 1237-1276), [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\include\configs\mvebu-common.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 45-84)

# PHẦN 25 (PART 26): Build System

## 0. Kết luận và hai đính chính quan trọng

Hệ thống build là **Kbuild của U-Boot 2015.01** (đệ quy theo thư mục, `obj-y`/`libs-y`), thêm một lớp KM mỏng. Đầu ra dùng thực tế duy nhất là **`output/u-boot.bin`**, được `build_uboot.sh` sao chép ra. Không có SPL/TPL, không có `u-boot.img`.

|#|Đính chính|Bằng chứng|
|---|---|---|
|C10|Phần 18 (C4) viết "`CONFIG_STATIC_RELA` được định nghĩa nhưng không nơi nào kiểm tra, không có tác dụng"|**Sai.** Nó được kiểm trong **Makefile** (mà lần grep trước tôi giới hạn theo đuôi file và bỏ sót). `Makefile:702`: `ifneq ($(CONFIG_STATIC_RELA),)`; `tools/Makefile:148` build `relocate-rela`. Vì `asm/config.h:15` định nghĩa macro rỗng (ARM64) và `tools/scripts/define2mk.sed` đổi "giá trị rỗng" thành `=y`, biến này **có giá trị `y`** trong `autoconf.mk`, nên `u-boot.bin` **được xử lý tĩnh sau khi tạo** (mục 5)|
|C11|Phần 18/23: "không có `-mstrict-align` [?]"|**Có.** `arch/arm/cpu/armv8/config.mk:10-12`: `PF_NO_UNALIGNED := $(call cc-option,-mstrict-align)` rồi thêm vào `PLATFORM_CPPFLAGS`. Trình biên dịch không phát sinh truy cập không căn hàng, nên việc chạy khi MMU tắt (đường NVMe) an toàn về mặt mã do compiler sinh ra. Ngoài ra `-fno-stack-protector` **có** (Makefile:549), trả lời câu hỏi mở ở Phần 21|

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Chuỗi biến `defconfig` thành `u-boot.bin`|
|WHY|Tái tạo firmware chính xác; hiểu cờ biên dịch nào quyết định hành vi|
|WHERE|`Makefile`, `config.mk`, `arch/arm/{Makefile,config.mk}`, `arch/arm/cpu/armv8/{config.mk,u-boot.lds}`, `scripts/Makefile.*`, `scripts/kconfig/`, `tools/`, `build_uboot.sh`, `include/configs/*.h`|

## 2. Điểm vào: `build_uboot.sh`

```
./build_uboot.sh 800 [clean|update]
  CONFIG=km_mvebu_quartz_toc_defconfig           (đối số "800")
  setEnv: export CROSS_COMPILE=s800-linux-       (toolchain tuỳ biến của KM)
  (không đối số 2)  make mrproper ; makeUboot
  update            makeUboot (KHÔNG mrproper)
  clean             make mrproper; rm -rf output
makeUboot: rm -rf output; mkdir output
           makeConfig: nếu KHÔNG có .config thì  make ${CONFIG}
           make                                   ← xây toàn bộ
           cp -a u-boot.bin output/
```

Các biến thể có sẵn: `emuegl`, `emueglz`, `emudnbmlk`, `egl`, `eglz`, `dnbmlk`, `spam`, `spaas`, `emuspam`, `emuspaas`, `eglb`, `eglbz`, `eglzp`, `eglbzp`, `spamb`, `spaasb`, `hemlk`, `mssb`. Repo này chỉ chứa defconfig `km_mvebu_quartz_toc_defconfig` cho đối số `800` (các biến thể kia trỏ tới defconfig không có trong `configs/` của bản này [?]: chưa liệt kê hết `configs/`).

Bẫy: với `update`, nếu `.config` đã tồn tại thì **không** chạy lại defconfig. Đổi defconfig hoặc header cấu hình rồi chỉ `update` có thể dùng cấu hình cũ. Cách an toàn là chạy không đối số (có `mrproper`).

## 3. Từ defconfig tới cấu hình (Kconfig lai)

```
make km_mvebu_quartz_toc_defconfig
   scripts/kconfig  → .config
   include/config/auto.conf , include/config/auto.conf.cmd , include/generated/autoconf.h   (từ Kconfig)
   scripts/Makefile.autoconf:
        include/config.h   (echo #include) : config_defaults.h → configs/armada8k.h → asm/config.h → config_fallbacks.h → config_uncmd_spl.h
        include/autoconf.mk  = CPP -dM common.h | sed define2mk.sed | (loại dòng trùng auto.conf)
             ◄── Makefile gốc: -include include/autoconf.mk    (Makefile:495)
```

- Hai nguồn cấu hình: (1) **Kconfig** (`auto.conf`), (2) **header** (`configs/*.h`), được đưa vào make bằng `autoconf.mk` (`define2mk.sed`: `#define CONFIG_X` rỗng hoặc `1` thành `CONFIG_X=y`; chuỗi đưa vào ngoặc kép; số giữ nguyên).
- Hệ quả: một macro trong header (ví dụ `CONFIG_CMD_USB`, `CONFIG_WARP`, `CONFIG_STATIC_RELA`, `CONFIG_E1000`) **vẫn điều khiển `obj-$(CONFIG_…)`** trong Makefile. Vì thế, để biết "file nào được biên dịch", phải xét cả header lẫn Kconfig (bài học ở Phần 15 và 19).
- Quy ước defconfig: `+S:` = áp cho ảnh thường và SPL (`doc/README.kconfig:80`); không tiền tố = chỉ ảnh thường.
- `CONFIG_SYS_TEXT_BASE = 0x00001000` (mvebu-common.h:53/55, cả hai nhánh giống nhau); `CONFIG_SYS_INIT_SP_ADDR = TEXT_BASE + 0xFF0000 = 0x00FF1000`; `CONFIG_SYS_MALLOC_LEN = 5 MiB`; `CONFIG_SYS_MALLOC_F_LEN = 0x5000` (malloc sớm); `CONFIG_NR_DRAM_BANKS = 3`; `CONFIG_UBOOT_SIZE = 2 MiB`.

## 4. Cờ biên dịch/liên kết

|Cờ|Nguồn|Ghi chú|
|---|---|---|
|`-D__KERNEL__ -D__UBOOT__ -D__ARM__`|Makefile:350, arch/arm/config.mk:25||
|`-Wall -Werror -Wstrict-prototypes -Wno-format-security -fno-builtin -ffreestanding`|Makefile:352-354|**`-Werror`**: mọi cảnh báo mới làm hỏng build|
|`-O2` (hoặc `-Os` nếu `CC_OPTIMIZE_FOR_SIZE`)|Makefile:539-543||
|`-g`|:551|có debug info trong ELF|
|`-fno-stack-protector`|:549|không có canary|
|`-Wno-format-nonliteral`, `-Werror=date-time`|:561, :571|cấm `__DATE__/__TIME__` trực tiếp|
|`-fstack-usage`|:557 nếu compiler hỗ trợ|sinh `.su`|
|`-pipe`|config.mk:83||
|`-DKM_BIZHUB`, `-I board/mvebu/armada8k/km`|config.mk:67-70 (khi `CONFIG_KM_BIZHUB=y`)|Đây là công tắc KM|
|`-fno-common -ffixed-x18`|armv8/config.mk:7|`x18` dành cho `gd`|
|`-march=armv8-a`, `-mstrict-align`|armv8/config.mk:9-12|C11|
|`-ffunction-sections -fdata-sections` + `--gc-sections`|arch/arm/config.mk:16-17|Loại mã chết (Phần 19)|
|`-Bstatic`, `-pie`, `-Ttext 0x1000`|config.mk:86, arch/arm/config.mk:85, Makefile:755|Nhị phân "PIE" liên kết ở 0x1000|
|`-DCONFIG_SYS_TEXT_BASE=0x1000`|Makefile:574||
|Thư viện|`-L… -lgcc` (`PLATFORM_LIBGCC` từ `$(CC) -print-libgcc-file-name`) hoặc `arch/arm/lib/lib.a` nếu `USE_PRIVATE_LIBGCC`||
|`DTC = ./tools/marvell/dtc`|Makefile:344|dtc đi kèm repo (tệp nhị phân Marvell, `tools/marvell/dtc`). `checkdtc` yêu cầu ≥ 1.4|

`arch/arm/config.mk` cũng chứa dòng `-ffixed-r9` và các cờ ARM32 (Thumb/EABI); chúng được đưa qua `cc-option` hoặc chỉ tác dụng cho ARM32 nên không cản build ARM64 (build thực tế đã thành công ở sản phẩm) [S].

## 5. Liên kết và tạo `u-boot.bin`

```
libs-y (thứ tự): lib/  board/mvebu/common/  dts/ (OF_EMBED)  fs/  net/  disk/  drivers/  drivers/{dma,gpio,i2c,mmc,mtd,mtd/spi,net,pci,power,spi,serial,usb/*,phy}/
                 common/  test/  board/mvebu/armada8k/  board/mvebu/armada8k/km/ (KM_BIZHUB)     (Makefile:607-650)
head-y = arch/arm/cpu/armv8/start.o
u-boot (ELF) = $(LD) $(LDFLAGS) $(LDFLAGS_u-boot) -o u-boot  head-y --start-group libs --end-group PLATFORM_LIBS -Map u-boot.map
               linker script: arch/arm/cpu/armv8/u-boot.lds  (LDSCRIPT tìm: board → CPUDIR → arch)
u-boot.bin  = objcopy -O binary
                 -j .text -j .secure_text -j .rodata -j .data -j .u_boot_list -j .rela.dyn   (ARM64, arch/arm/config.mk:116)
                 -j .dtb.init.rodata                                                       (OF_EMBED, :122)
                 --gap-fill=0xff                                                            (config.mk:80)
              rồi:  tools/relocate-rela u-boot.bin 0x1000 <__rel_dyn_start> <__rel_dyn_end>        (Makefile:824, do C10)
binary_size_check:  so kích thước file với (_image_binary_end - _image_copy_start) trong u-boot.map (phải bằng nhau)
checkarmreloc:      readelf -r phải chỉ có R_AARCH64_RELATIVE
ALL-y (mục tiêu thực): u-boot.srec u-boot.bin System.map binary_size_check checkarmreloc
```

Ý nghĩa của bước `relocate-rela` (`tools/relocate-rela.c`): với mỗi mục `.rela.dyn` loại `R_AARCH64_RELATIVE`, ghi **giá trị addend** vào đúng vị trí trong tệp `.bin` (`fseek(addr - text_base)`, `fwrite(&r_addend)`). Nghĩa là bản nhị phân **đã được "liên kết trước" cho địa chỉ 0x1000**: các con trỏ trong `.data/.rodata` (bảng `init_sequence_f[]`, `.u_boot_list`, chuỗi…) đã là địa chỉ tuyệt đối đúng tại 0x1000 _trước khi_ `relocate_code` chạy. Sau đó `relocate_64.S` (dòng 44-59) vẫn xử lý `.rela.dyn` lần nữa khi chuyển ảnh lên RAM cao: với mỗi RELATIVE, `*(r_offset + x9) = r_addend + x9` (`x9` = độ dời), tức tính từ **addend** chứ không cộng vào giá trị đang có, nên hai bước không chồng nhau (idempotent theo thiết kế).

Hệ quả: **ATF phải nạp `u-boot.bin` đúng địa chỉ 0x1000** (BL33_BASE). Ở địa chỉ khác, các con trỏ tuyệt đối tĩnh sẽ sai trước khi relocation chạy (đúng như tôi ghi ở Phần 2 "phải bằng địa chỉ liên kết"). Lý do cần: `board_init_f` chạy **trước** relocation và dùng các bảng con trỏ này.

`u-boot.bin` không có header (không `mkimage`), không có `.rela.dyn` thừa? Có: `.rela.dyn` được giữ (`-j .rela.dyn`) vì `relocate_code` cần nó khi chạy. `.u_boot_list` (bảng lệnh/driver) và DTB điều khiển (`.dtb.init.rodata`, đệm `-S 0x4000`) nằm trong `.bin`.

## 6. DTB nhúng

`libs-$(CONFIG_OF_EMBED) += dts/` (Makefile:609): `dts/Makefile` chọn DTB theo `CONFIG_DEFAULT_DEVICE_TREE = armada-quartz-emu800`, biên dịch bằng `tools/marvell/dtc` qua tiền xử lý C (`#include`, `#ifdef CONFIG_KM_BIZHUB` trong `.dts`), rồi gắn vào `.dtb.init.rodata` (`__dtb_dt_begin`). Chi tiết: Phần 7. Nhắc lại: với `KM_BIZHUB`, chỉ `emu800.dtb` được build; `#define CONFIG_KM_BIZHUB` được **cứng hoá ở đầu `emu800.dts`** (toc.dts thì không có).

## 7. Sinh phiên bản và thời gian

- `include/generated/version_autogenerated.h` (Makefile:1171-1180): `U_BOOT_VERSION`, `CC_VERSION_STRING`, `LD_VERSION_STRING`.
- `include/generated/timestamp_autogenerated.h`: thời điểm build.
- Chuỗi phiên bản = `2015.01` + `localversion` (`-devel-16.07.2`) + kết quả `scripts/setlocalversion` (**không có git** ở bản này; `snapshot.commit` giữ nguyên `$Format:%H %cD$`, chưa được mở rộng) (Phần 0).
- `asm-offsets.h`: sinh từ `arch/arm/lib/asm-offsets.c` (vd. `GD_SIZE`, `GD_RELOCADDR`) cho mã assembly (`crt0_64.S`). Đây là nguồn của kích thước `gd` khổng lồ (≈0x418C8) khi KM thêm bộ đệm log (Phần 3).

## 8. Chỗ KM can thiệp vào build

|Chỗ|Nội dung|
|---|---|
|`Kconfig` gốc|Menu KonicaMinolta: `KM_BIZHUB` (default y), `KM_MACHINE_*`, `KM_BOARD_*`, `KM_NVME`, `KM_I2S_CDMA`, `KM_PCIE_SET_MPS`…|
|`config.mk`|`-DKM_BIZHUB`, `-I board/.../km`|
|`Makefile:650`|`libs-$(CONFIG_KM_BIZHUB) += board/mvebu/armada8k/km/`|
|`board/mvebu/armada8k/km/Makefile`|11 đối tượng: `machine_setup mfp_panel dipvalue graphlib pngfilter picture userdata mfp_config mtd_flash_access init_debug ca72wdt`|
|`drivers/*/Makefile`|Chọn driver KM (`qspi_winbond.o`, `mvqz_gpio*.o`, `km-*.o`, `phy-utm-mv.o`…)|
|`include/configs/*.h`|Toàn bộ khối `#if defined(KM_BIZHUB)` (Phần 26)|
|`common/Makefile`|`cmd_warp.o` (CONFIG_WARP), `cmd_km-sound.o` (KM_I2S_CDMA)|

## 9. Kiểm tra và lỗi thường gặp

|Tình huống|Hiện tượng|
|---|---|
|Cảnh báo mới do toolchain mới|Lỗi vì `-Werror`|
|Định nghĩa `CONFIG_X` ở header mà không phản ánh Kconfig|Vẫn ảnh hưởng Makefile qua `autoconf.mk` (dễ lẫn khi đọc chỉ `.config`)|
|`update` sau khi đổi defconfig|Dùng `.config` cũ|
|`u-boot.bin` khác `u-boot.map`|`binary_size_check` báo lỗi|
|Có relocation ngoài RELATIVE|`checkarmreloc` báo lỗi|
|Thiếu `tools/marvell/dtc` chạy được (host khác kiến trúc)|Không dựng được DTB (bản `dtc` là nhị phân dựng sẵn cho Linux x86 [?])|
|Không phải Linux (workspace Windows)|Không thể build tại đây; chỉ đọc mã|

Tôi **không** xác minh được kết quả build thực (không có `u-boot.map`, `.config`, `u-boot`, `output/`); mọi kết luận về "file nào được biên dịch" dựa trên phân tích Makefile + header + defconfig.

## 10. COMMON CONFUSION

1. **"Chỉ Kconfig quyết định file biên dịch"**: không, còn header + `autoconf.mk`.
2. **`u-boot.bin` = `objcopy` thô**: đã có thêm bước `relocate-rela`.
3. **PIE nghĩa là chạy ở địa chỉ tuỳ ý**: ở đây không; nó phải nằm ở 0x1000 khi khởi chạy.
4. **`-ffixed-x18` chỉ là tối ưu**: nó là hợp đồng ABI: `x18` = `gd`, và mọi code (kể cả asm) phải tôn trọng.
5. **`DTC` hệ thống**: dùng dtc riêng trong repo.
6. **SPL build**: `scripts/Makefile.spl` tồn tại nhưng `# CONFIG_SPL is not set` nên không chạy (Phần 4).

## 11. MENTAL MODEL

```
defconfig ─(Kconfig)→ .config, auto.conf
config.h ─(cpp -dM | define2mk.sed)→ autoconf.mk         ← header thành biến make
        │
        ▼
Makefile: libs-y (đệ quy) ─► u-boot (ELF, -pie, -Ttext 0x1000, --gc-sections)
        ▼ objcopy (-j .text .rodata .data .u_boot_list .rela.dyn .dtb.init.rodata, gap 0xff)
        ▼ tools/relocate-rela  (ghi addend → bản "liên kết trước" tại 0x1000)
     output/u-boot.bin  ◄─ chép bởi build_uboot.sh
```

## 12. Chưa xác minh

- Danh sách đầy đủ defconfig trong `configs/` so với script.
- `asm-offsets` và các tệp sinh (không có bản build).
- Việc `-ffixed-r9` có hại/no-op khi dựng ARM64.
- Giá trị `CONFIG_SYS_UBOOT_START`, `CONFIG_SPL_*` (không dùng).
- `tools/marvell/dtc` là Linux x86-64 hay không.

---

Xong Phần 25. Tiếp theo **Phần 26: Configuration**: Kconfig, header cấu hình theo lớp (`armada8k.h`, `mvebu-common.h`, `quartz-sb.h`), cờ `CONFIG_KM_*`, `KM_MACHINE_*`, bảng ảnh hưởng.

Bash List configs dir and show top-level Kconfig

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && ls configs | head -80; echo ---; sed -n 1,140p Kconfig | sed -n 1,140p

OUT

A10-OLinuXino-Lime_defconfig
A10s-OLinuXino-M_defconfig
A13-OLinuXinoM_defconfig
A13-OLinuXino_defconfig
A20-OLinuXino-Lime2_defconfig
A20-OLinuXino-Lime_defconfig
A20-OLinuXino_MICRO_defconfig
APC405_defconfig
AR405_defconfig
ASH405_defconfig
Auxtek-T004_defconfig
B4420QDS_NAND_defconfig
B4420QDS_SPIFLASH_defconfig
B4420QDS_defconfig
B4860QDS_NAND_defconfig
B4860QDS_SECURE_BOOT_defconfig
B4860QDS_SPIFLASH_defconfig
B4860QDS_SRIO_PCIE_BOOT_defconfig
B4860QDS_defconfig
BC3450_defconfig
BSC9131RDB_NAND_SYSCLK100_defconfig
BSC9131RDB_NAND_defconfig
BSC9131RDB_SPIFLASH_SYSCLK100_defconfig
BSC9131RDB_SPIFLASH_defconfig
BSC9132QDS_NAND_DDRCLK100_SECURE_defconfig
BSC9132QDS_NAND_DDRCLK100_defconfig
BSC9132QDS_NAND_DDRCLK133_SECURE_defconfig
BSC9132QDS_NAND_DDRCLK133_defconfig
BSC9132QDS_NOR_DDRCLK100_SECURE_defconfig
BSC9132QDS_NOR_DDRCLK100_defconfig
BSC9132QDS_NOR_DDRCLK133_SECURE_defconfig
BSC9132QDS_NOR_DDRCLK133_defconfig
BSC9132QDS_SDCARD_DDRCLK100_SECURE_defconfig
BSC9132QDS_SDCARD_DDRCLK100_defconfig
BSC9132QDS_SDCARD_DDRCLK133_SECURE_defconfig
BSC9132QDS_SDCARD_DDRCLK133_defconfig
BSC9132QDS_SPIFLASH_DDRCLK100_SECURE_defconfig
BSC9132QDS_SPIFLASH_DDRCLK100_defconfig
BSC9132QDS_SPIFLASH_DDRCLK133_SECURE_defconfig
BSC9132QDS_SPIFLASH_DDRCLK133_defconfig
Bananapi_defconfig
C29XPCIE_NAND_defconfig
C29XPCIE_SPIFLASH_defconfig
C29XPCIE_defconfig
CATcenter_25_defconfig
CATcenter_33_defconfig
CATcenter_defconfig
CMS700_defconfig
CPCI2DP_defconfig
CPCI4052_defconfig
CPCI405AB_defconfig
CPCI405DT_defconfig
CPCI405_defconfig
CPCIISER4_defconfig
Colombus_defconfig
Cubieboard2_defconfig
Cubieboard_defconfig
Cubietruck_defconfig
DP405_defconfig
DU405_defconfig
DU440_defconfig
ELPPC_defconfig
G2000_defconfig
HH405_defconfig
HUB405_defconfig
Ippo_q8h_v5_defconfig
JSE_defconfig
Linksprite_pcDuino3_defconfig
Linksprite_pcDuino3_fdt_defconfig
Lite5200_LOWBOOT08_defconfig
Lite5200_LOWBOOT_defconfig
Lite5200_defconfig
M5208EVBE_defconfig
M52277EVB_defconfig
M52277EVB_stmicro_defconfig
M5235EVB_Flash32_defconfig
M5235EVB_defconfig
M5249EVB_defconfig
M5253DEMO_defconfig
M5253EVBE_defconfig
---
#
# For a description of the syntax of this configuration file,
# see Documentation/kbuild/kconfig-language.txt.
#
mainmenu "U-Boot $UBOOTVERSION Configuration"

config UBOOTVERSION
	string
	option env="UBOOTVERSION"

config KCONFIG_OBJDIR
	string
	option env="KCONFIG_OBJDIR"

menu "General setup"

config LOCALVERSION
	string "Local version - append to U-Boot release"
	depends on !SPL_BUILD
	help
	  Append an extra string to the end of your U-Boot version.
	  This will show up on your boot log, for example.
	  The string you set here will be appended after the contents of
	  any files with a filename matching localversion* in your
	  object and source tree, in that order.  Your total string can
	  be a maximum of 64 characters.

config LOCALVERSION_AUTO
	bool "Automatically append version information to the version string"
	depends on !SPL_BUILD
	default y
	help
	  This will try to automatically determine if the current tree is a
	  release tree by looking for git tags that belong to the current
	  top of tree revision.

	  A string of the format -gxxxxxxxx will be added to the localversion
	  if a git-based tree is found.  The string generated by this will be
	  appended after any matching localversion* files, and after the value
	  set in CONFIG_LOCALVERSION.

	  (The actual string used here is the first eight characters produced
	  by running the command:

	    $ git rev-parse --verify HEAD

	  which is done within the script "scripts/setlocalversion".)

config CC_OPTIMIZE_FOR_SIZE
	bool "Optimize for size"
	depends on !SPL_BUILD
	default y
	help
	  Enabling this option will pass "-Os" instead of "-O2" to gcc
	  resulting in a smaller U-Boot image.

	  This option is enabled by default for U-Boot.

menuconfig EXPERT
        bool "Configure standard U-Boot features (expert users)"
        help
          This option allows certain base U-Boot options and settings
          to be disabled or tweaked. This is for specialized
          environments which can tolerate a "non-standard" U-Boot.
          Only use this if you really know what you are doing.

endmenu		# General setup

menu "Boot images"

config SPL_BUILD
	bool
	depends on $KCONFIG_OBJDIR="spl" || $KCONFIG_OBJDIR="tpl"
	default y

config TPL_BUILD
	bool
	depends on $KCONFIG_OBJDIR="tpl"
	default y

config SUPPORT_SPL
	bool

config SUPPORT_TPL
	bool

config SPL
	bool
	depends on SUPPORT_SPL
	prompt "Enable SPL" if !SPL_BUILD
	default y if SPL_BUILD
	help
	  If you want to build SPL as well as the normal image, say Y.

config TPL
	bool
	depends on SPL && SUPPORT_TPL
	prompt "Enable TPL" if !SPL_BUILD
	default y if TPL_BUILD
	default n
	help
	  If you want to build TPL as well as the normal image and SPL, say Y.

config FIT
	bool "Support Flattened Image Tree"
	depends on !SPL_BUILD
	help
	  This option allows to boot the new uImage structrure,
	  Flattened Image Tree.  FIT is formally a FDT, which can include
	  images of various types (kernel, FDT blob, ramdisk, etc.)
	  in a single blob.  To boot this new uImage structure,
	  pass the the address of the blob to the "bootm" command.

config FIT_VERBOSE
	bool "Display verbose messages on FIT boot"
	depends on FIT

config FIT_SIGNATURE
	bool "Enabel signature verification of FIT uImages"
	depends on FIT
	help
	  This option enables signature verification of FIT uImages,
	  using a hash signed and verified using RSA.
	  See doc/uImage.FIT/signature.txt for more details.

config SYS_EXTRA_OPTIONS
	string "Extra Options (DEPRECATED)"
	depends on !SPL_BUILD
	help
	  The old configuration infrastructure (= mkconfig + boards.cfg)
	  provided the extra options field. If you have something like
	  "HAS_BAR,BAZ=64", the optional options
	    #define CONFIG_HAS
	    #define CONFIG_BAZ	64
	  will be defined in include/config.h.
	  This option was prepared for the smooth migration from the old
	  configuration to Kconfig. Since this option will be removed sometime,
	  new boards should not use this option.

config SYS_TEXT_BASE

Grep "KM_|KonicaMinolta|Konica" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {Kconfig,board/mvebu/Kconfig,drivers/*/Kconfig,common/Kconfig,arch/arm/Kconfig})

43 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\Kconfig](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 127-286)

Bash List KM-related defconfigs

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src/configs" && ls | grep -i -E "emu800|^egl|spam|spaas|hemlk|dnbmlk|mssb|km_|mvebu|armada|quartz" ; echo; ls | wc -l

OUT

dnbmlk_defconfig
egl_defconfig
eglb_defconfig
eglbz_defconfig
eglbzp_defconfig
eglz_defconfig
eglzp_defconfig
emu800_dnbmlk_defconfig
emu800_egl_defconfig
emu800_eglz_defconfig
emu800_spaas_defconfig
emu800_spam_defconfig
hemlk_defconfig
km_kirkwood_128m16_defconfig
km_kirkwood_defconfig
km_kirkwood_pci_defconfig
km_mvebu_quartz_pd_defconfig
km_mvebu_quartz_toc_defconfig
mssb_defconfig
mvebu_apn806_defconfig
mvebu_apn806_dop_defconfig
mvebu_apn806_pd_defconfig
mvebu_armada3700_defconfig
mvebu_armada38x_defconfig
mvebu_armada70x0_defconfig
mvebu_armada70x0_dop_defconfig
mvebu_armada70x0_pd_defconfig
mvebu_armada70x0_rz_defconfig
mvebu_armada70x0amber_defconfig
mvebu_armada70x0uNAS_defconfig
mvebu_quartz_pd_defconfig
mvebu_quartz_toc_defconfig
spaas_defconfig
spaasb_defconfig
spam_defconfig
spamb_defconfig

1172

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\board_r.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 377-591)

Grep "TYP_NAME|DEF_MACHINE_NAME|define\s+TYP_NAME|CONFIG_KM_MACHINE|CONFIG_KM_BOARD" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\main.c)

41 lines of output

Bash Show KM-related defconfigs for comparison

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src/configs" && for f in km_mvebu_quartz_toc_defconfig km_mvebu_quartz_pd_defconfig emu800_egl_defconfig egl_defconfig spam_defconfig hemlk_defconfig mvebu_quartz_toc_defconfig; do echo "=== $f"; cat $f; echo; done

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\main.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 311-370)

Remote Control is active · Continue here, on your phone, or at [claude.ai/code](https://claude.ai/code/session_014NZ7TYd5k76Y3adxRrisNE)

Bash Check KM_ flags in the actual defconfig used for this board

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && grep -n "KM_" configs/km_mvebu_quartz_toc_defconfig; echo ---; cat configs/km_mvebu_quartz_toc_defconfig

OUT

33:CONFIG_KM_I2S_CDMA=y
44:CONFIG_KM_NVME=y
45:CONFIG_KM_PCIE_SET_MPS=y
---
# CONFIG_SPL is not set
CONFIG_QUARTZ=y
CONFIG_SYS_EXTRA_OPTIONS="ARM64"
+S:CONFIG_ARM=y
+S:CONFIG_TARGET_ARMADA_8K=y
CONFIG_MVEBU_CCU=y
CONFIG_MVEBU_RFU=y
# CONFIG_MVEBU_MPP_BUS=y
+S:CONFIG_DEVEL_BOARD=y
+S:# CONFIG_MULTI_DT_FILE is not set
CONFIG_CMD_BOOTD=y
CONFIG_CMD_RUN=y
CONFIG_CMD_EDITENV=y
CONFIG_CMD_SAVEENV=y
CONFIG_CMD_MEMORY=y
CONFIG_CMD_MEMTEST=y
CONFIG_CMD_PCI=y
CONFIG_CMD_TIME=y
CONFIG_CMD_MISC=y
CONFIG_FS_EXT4=y
CONFIG_CMD_EXT2=y
CONFIG_CMD_LOADS=y
CONFIG_CMD_LOADB=y
CONFIG_CMD_TIMER=y
CONFIG_CMD_MVEBU_MPP=y
+S:CONFIG_OF_CONTROL=y
+S:CONFIG_OF_EMBED=y
+S:CONFIG_DEFAULT_DEVICE_TREE="armada-quartz-emu800"
+S:CONFIG_SYS_NS16550=y
+S:# CONFIG_MVEBU_AP806_16750=y
# CONFIG_MVEBU_PINCTL=y
CONFIG_XENON_MMC=y
CONFIG_KM_I2S_CDMA=y
# CONFIG_MVEBU_I2C is not set
CONFIG_MRVL_QSPI=y
CONFIG_MVQZ_GPIO=y
CONFIG_CMD_BDI=y
CONFIG_USB=y
CONFIG_USB_XHCI_HCD=y
CONFIG_USB_MV_UTM_PHY=y
CONFIG_USB_MV_USB3_PHY=y
CONFIG_DW_PCIE=y
CONFIG_PCIE_MV_MSB0125=y
CONFIG_KM_NVME=y
CONFIG_KM_PCIE_SET_MPS=y

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\main.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 317-360)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\include\configs\armada8k.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 2-101)

# PHẦN 26 (PART 27): Configuration

## 0. Kết luận và đính chính

Cấu hình gồm **ba lớp** chồng nhau: (1) defconfig + Kconfig, (2) chuỗi header `include/configs/*.h`, (3) macro do compiler `-D` đưa vào (`KM_BIZHUB`, `CONFIG_SYS_TEXT_BASE`). Để biết một tính năng có bật hay không phải xét cả ba (Phần 25).

Điều quan trọng nhất của phần này: **defconfig `km_mvebu_quartz_toc_defconfig` không chọn máy cụ thể nào.** Nó không đặt `CONFIG_KM_BOARD_EMU800` và không đặt bất kỳ `CONFIG_KM_MACHINE_*` nào (grep `KM_` trong defconfig chỉ ra `KM_I2S_CDMA`, `KM_NVME`, `KM_PCIE_SET_MPS`). Mọi cờ đó `default n` trong `Kconfig:183-276`. Đây là "bản build 800" dùng chung, và loại máy được xác định **lúc chạy** từ dữ liệu trong SPI-flash (`MachineInfo`, `MachineNo`).

|#|Đính chính|Bằng chứng|
|---|---|---|
|C12|Phần 16/20 ghi `FW_DIRECTRY = FW9020/FW9022/FW9023 [S]`|Chuỗi `#if/#elif` ở `armada8k.h:160-208` không nhánh nào khớp (không `KM_MACHINE_*`, không `KM_BOARD_EMU800`), rơi vào `#else` **`"FW0000"`**. Vậy bootargs có `FwDir=FW0000` và USB update dùng thư mục `FW0000/`|
|C13|Phần 20 nhắc `TYP_NAME`/`DEF_MACHINE_NAME` chưa rõ|Nhánh `#else /* 800ビルド用パターン */` (main.c:353-355): `TYP_NAME = "@TYP=ZS_ZX0_PF"`, `DEF_MACHINE_NAME = "ZeusS_ZX0"`. INDEX trên USB phải mang `@TYP=ZS_ZX0_PF` thì mới qua kiểm tra "This usb memory is not for ZeusS_ZX0"|
|C14|Phần 25: "`-O2` (hoặc `-Os`...)"|`CC_OPTIMIZE_FOR_SIZE` là Kconfig `default y` (Kconfig:47-56) và defconfig không tắt, nên thực tế là **`-Os`**|

Hệ quả cho các đoạn `#ifdef CONFIG_KM_MACHINE_*` đã đọc ở các phần trước: nhánh dành cho Sparrow/Eagle/Deneb/Helios/Minerva đều **không được biên dịch**; các lệnh `EAD` (chỉ HEMLK) không có; `do_help` không bị vô hiệu (chỉ HEMLK). Lệnh `C`, `CB` (userdata.c) thì không bị điều kiện máy chặn nên **có** (giải quyết điểm [S] ở Phần 15). Còn `MACHINE`, `db_sw` chưa xác định điều kiện; giữ [S].

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Toàn bộ "công tắc" quyết định ảnh U-Boot chứa gì và hành xử ra sao|
|WHY|Một mã nguồn phục vụ hơn 15 loại máy KM + board Marvell|
|WHERE|`configs/km_mvebu_quartz_toc_defconfig`, `Kconfig` (KM menu), `board/mvebu/Kconfig`, `common/Kconfig`, `drivers/*/Kconfig`, `include/config_defaults.h`, `include/configs/{armada8k,mvebu-common,quartz-sb}.h`, `arch/arm/include/asm/config.h`, `include/config_fallbacks.h`|

## 2. Lớp 1: defconfig và Kconfig

### 2.1 Nội dung defconfig `800`

|Nhóm|Dòng|Ý nghĩa|
|---|---|---|
|Nền tảng|`# CONFIG_SPL is not set`, `QUARTZ=y`, `SYS_EXTRA_OPTIONS="ARM64"`, `+S:ARM`, `+S:TARGET_ARMADA_8K`|Không SPL; ARM64; Armada 8K|
|Cửa sổ địa chỉ|`MVEBU_CCU`, `MVEBU_RFU` (`# MVEBU_MPP_BUS` tắt)|Phần 6/8|
|Board|`+S:DEVEL_BOARD=y`|Kéo theo `devel-board.o` và `CONFIG_E1000` (Phần 19)|
|Lệnh|`BOOTD RUN EDITENV SAVEENV MEMORY MEMTEST PCI TIME MISC EXT2 LOADS LOADB TIMER MVEBU_MPP BDI` + `FS_EXT4`|Phần 15. `CMD_SAVEENV=y` nhưng lệnh bị loại bởi `ENV_IS_NOWHERE`|
|DT|`+S:OF_CONTROL`, `+S:OF_EMBED`, `+S:DEFAULT_DEVICE_TREE="armada-quartz-emu800"`, `# MULTI_DT_FILE` tắt|Phần 7|
|UART|`+S:SYS_NS16550`, `# MVEBU_AP806_16750` tắt|Phần 9|
|Thiết bị|`XENON_MMC`, `MRVL_QSPI`, `MVQZ_GPIO`, `USB`, `USB_XHCI_HCD`, `USB_MV_UTM_PHY`, `USB_MV_USB3_PHY`, `DW_PCIE`, `PCIE_MV_MSB0125`|Phần 12, 20|
|Tắt|`# MVEBU_PINCTL`, `# MVEBU_I2C`|I2C dùng DW qua header (Phần 22)|
|KM|`KM_I2S_CDMA`, `KM_NVME`, `KM_PCIE_SET_MPS`|Âm thanh, NVMe, MPS cho PCIe|

Quy ước `+S:` = áp cho ảnh thường **và** SPL (`doc/README.kconfig:80`).

### 2.2 Cờ KM trong Kconfig gốc (`Kconfig:164-278`)

|Cờ|Mặc định|Vai trò|
|---|---|---|
|`KM_EXTENDED`|y|Mã mở rộng KM (`-DKM_EXTENDED`)|
|`KM_BIZHUB`|**y**|Công tắc chính: `-DKM_BIZHUB` + thư mục `km/`|
|`HWCHECK_MEMSIZE`|8589934592 (8 GB)|Dung lượng DRAM mong đợi khi tự chẩn đoán HW ("Eagle spec")|
|`HWCHECK_MEMSIZE_SFP2` / `_SFP2_4GB` / `_AIO2_6GB` / `_5GB` / `_6GB`|3 / 4 / 6 / 5 / 6 GB|Ngưỡng cho từng máy Sparrow (chỉ dùng trong nhánh `KM_MACHINE_SPA*`)|
|`KM_BOARD_EMU800`|n|Bộ mô phỏng Emu800|
|`KM_MACHINE_EGL/EGLZ/DNBMLK/SPAM/SPAAS/EGLBK/EGLBKZ/EGLZP/EGLBKZP/SPABKM/SPABKAS/HEMLK/MSSB`|n|13 loại máy|
|`KM_NVME`|(drivers/block/Kconfig)|Driver NVMe KM; header đặt `CMD_NVME`, `NVME_MAX_DEVICE=1`, `NAMESPACE_ID=1`|
|`KM_I2S_CDMA`|(drivers/sound/Kconfig)|Âm thanh I2S+CDMA, kéo `cmd_km-sound.o`|
|`KM_PCIE_SET_MPS`|(drivers/pci/Kconfig)|`CONFIG_PCI_FIXUP_DEV` để chỉnh MPS|

Với `800`, `init_km` dùng nhánh mặc định `#else` (board_r.c:555-565): so `gd->chk_ram_Size` với **`CONFIG_HWCHECK_MEMSIZE` = 8 GB**; chỉ chạy trong chế độ HW-check (`uc_HWcheckModeBoot == 1`). Nếu máy chỉ có DIMM nhỏ hơn 8 GB mà vào chế độ chẩn đoán, sẽ báo `### BOOT-DIAG ERROR/Dimm ###` rồi `_WAIT_FOREVER` [V bằng đọc code; chưa xác nhận dung lượng DIMM thực].

### 2.3 Kconfig của Marvell/mvebu

- `board/mvebu/Kconfig`: `MVEBU` (default y khi `SYS_VENDOR="mvebu"`), `DEVEL_BOARD` (depends MVEBU), `QUARTZ` (depends `TARGET_ARMADA_8K`), … → điều kiện của `common/mvebu/*`.
- `common/mvebu/Kconfig`: `CMD_MVEBU_MISC` (y), `CMD_MVEBU_MSS_BOOT` (y), còn lại n (Phần 15).
- `drivers/usb/host/Kconfig`: `USB_XHCI` = `default USB_XHCI_HCD`, `USB_XHCI_MVEBU` default y.
- `common/Kconfig`: `CMD_BOOTM`, `CMD_GO`, `CMD_EXPORTENV`, `CMD_IMPORTENV`, `CMD_CRC32` default y; `HUSH_PARSER` chọn `SYS_HUSH_PARSER`.
- `Kconfig` gốc: `FIT` (n), `SPL` chỉ khi `SUPPORT_SPL` (TARGET_ARMADA_8K có `select SUPPORT_SPL` nhưng defconfig tắt).

## 3. Lớp 2: chuỗi header

Thứ tự include (`include/config.h` do `Makefile.autoconf` sinh):

```
config_defaults.h            giá trị mặc định chung (CONFIG_BOOTM_LINUX=1, CONFIG_GZIP=1, …)
configs/armada8k.h           (CONFIG_SYS_CONFIG_NAME=armada8k)
   ├ #include "mvebu-common.h"   chung MVEBU
   ├ #include "quartz-sb.h"      southbridge Quartz (I2C DW…)
   └ ghi đè + khối KM
asm/config.h                 CONFIG_LMB, CONFIG_SYS_BOOT_RAMDISK_HIGH, ARM64: CONFIG_PHYS_64BIT, CONFIG_STATIC_RELA
config_fallbacks.h           suy diễn: CMD_FAT⇒FS_FAT, CMD_EXT2⇒FS_EXT4, IMAGE_FORMAT_LEGACY, …
config_uncmd_spl.h           xoá cờ cho SPL
```

Lưu ý thứ tự: `armada8k.h` **include `mvebu-common.h` trước** rồi mới `#undef`/`#define` lại (ví dụ `CONFIG_BOOTDELAY` 2 → 3, `CONFIG_MVEBU_UBOOT_DFLT_NAME` → `flash-image.bin`), nên giá trị cuối cùng luôn là của `armada8k.h`.

### 3.1 Bảng hằng số quan trọng và nguồn thắng cuối cùng

|Hằng|Giá trị|Nguồn|
|---|---|---|
|`CONFIG_SYS_TEXT_BASE`|`0x00001000`|mvebu-common.h:53-55|
|`CONFIG_SYS_SDRAM_BASE`|0|:59|
|`CONFIG_SYS_INIT_SP_ADDR`|`TEXT_BASE + 0xFF0000` = 0xFF1000|:71|
|`CONFIG_SYS_MALLOC_LEN` / `_F_LEN`|5 MiB / 0x5000|:65, :47|
|`CONFIG_NR_DRAM_BANKS`|3|:67|
|`CONFIG_SYS_MAXARGS`|32|:73|
|`CONFIG_UBOOT_MAX_MEM_SIZE`|3 GiB|:78|
|`CONFIG_SYS_BOOTMAPSZ`|16 MiB|:111|
|`CONFIG_SYS_BOOTM_LEN`|20 MiB|:118|
|`CONFIG_SYS_LOAD_ADDR`|0x02000000|:127|
|`CONFIG_PRE_CON_BUF_ADDR/SZ`|0x02000000 / 1 MiB|:153-154|
|`CONFIG_SYS_CBSIZE` / `PBSIZE`|1024 / +prompt|:144-145|
|`CONFIG_SYS_PROMPT`|`"Marvell>> "`|:143|
|`CONFIG_BOOTDELAY`|**3** (thắng `2` của mvebu-common)|armada8k.h:131-132|
|`COUNTER_FREQUENCY`|25 MHz|armada8k.h:56|
|`CONFIG_MSS_FREQUENCY`|200 MHz|:57|
|`CONFIG_QUARTZ_RAM_SIZE`|0x80000000 (2 GiB, cứng)|:108|
|ATF|base `0x10000000`, size 4 MiB; DDR training `0x10400000` 1 MiB; SCPI mailbox `0x7ff00000` 1 MiB|:106-115|
|`CONFIG_BOOTFILE` / `KERNEL_ADDR` / `FDT_ADDR` / `INITRD_ADDR`|`Image` / 0x800000 / 0x1000 / 0x3000000|:158, :119-122|
|`CONFIG_GICV2`|có|:49|
|`CONFIG_WARP*`|`WARP`, `WARP_SD`, `WARP_SPI`, `WARP_SPI_MTD`, `WARP_NVME`, `WARP_LOAD_ADDR = 0x800000`, `SAVENO=0` (armada8k.h) **và** `WARP_LOAD_ADDR = SDRAM_BASE` trong mvebu-common.h:568|armada8k.h:305-311 thắng|

Ghi chú lệch: `mvebu-common.h:565-569` cũng định nghĩa `CONFIG_WARP*` với `WARP_LOAD_ADDR = SDRAM_BASE` (0). Vì `armada8k.h` include `mvebu-common.h` **trước** rồi định nghĩa lại `WARP_LOAD_ADDR = SDRAM_BASE + 0x800000` không `#undef`, compiler sẽ báo **cảnh báo "redefined"**; với `-Werror` một số compiler coi là lỗi. Sản phẩm vẫn build được, nên hoặc `mvebu-common.h:565` nằm sau một điều kiện chưa thỏa, hoặc redefinition giống hệt không cảnh báo. Chưa xác minh được [?].

### 3.2 Khối KM trong header

- `armada8k.h:117-128`: `CONFIG_BOOTARGS="uio_pdrv_genirq.of_id=generic-uio"`, `EXTRA_ENV_SETTINGS`.
- `armada8k.h:150-284`: `FW_DIRECTRY`, mẫu `CMD_INITRD_ENV`, mã hoá `bootcmd` cho mỗi kiểu boot (Phần 16).
- `armada8k.h:287-311`: `CMD_FAT`, `CMD_MMC`, `SYS_HUSH_PARSER`, `USE_IRQ`, `MISC_INIT_R`, `WARP_*`.
- `armada8k.h:345-354`: `warp_prepare_boot()` (Phần 18), `CONFIG_LZO` (:358), `CONFIG_SYS_WHITE_ON_BLACK` (:361).
- `mvebu-common.h:158-159`: `CONFIG_BOOTFILE`, `CONFIG_BOOTDELAY 2` (bị ghi đè).
- `main.c:80-357`: `TYP_NAME`, `DEF_MACHINE_NAME`, `TYP_NAME_LINECARD`… theo `KM_MACHINE_*`.

## 4. Lớp 3: cờ dòng lệnh

`-DKM_BIZHUB`, `-DKM_EXTENDED` (nếu bật), `-DCONFIG_SYS_TEXT_BASE=0x1000`, `-D__KERNEL__ -D__UBOOT__ -D__ARM__`, `-DBUILD_TAG` (nếu có). Chú ý: trong code, `KM_BIZHUB` (từ `-D`) và `CONFIG_KM_BIZHUB` (từ Kconfig) **cùng tồn tại**; hai chuỗi kiểm tra `#if defined(KM_BIZHUB)` / `#ifdef CONFIG_KM_BIZHUB` lẫn lộn trong mã. Với defconfig hiện tại cả hai cùng đúng.

DTS thì dùng tiền xử lý C, và `#define CONFIG_KM_BIZHUB` được cứng hoá ở đầu `emu800.dts` (Phần 7).

## 5. Nhận biết "cái gì thực sự bật"

Thứ tự tra cứu đáng tin:

1. `.config`/`auto.conf` (không có trong repo, sinh khi build).
2. `include/configs/armada8k.h → mvebu-common.h → quartz-sb.h`.
3. `asm/config.h`, `config_defaults.h`, `config_fallbacks.h`.
4. `Kconfig` `default` của từng ký hiệu.
5. Điều kiện `obj-$(CONFIG_…)` trong Makefile cục bộ.

Một số cái bẫy đã gặp trong quá trình phân tích:

|Bẫy|Ví dụ|
|---|---|
|Macro header vẫn điều khiển Makefile|`CONFIG_E1000`, `CONFIG_STATIC_RELA`|
|Macro có nhưng mã bị `--gc-sections` loại|`e1000.o`, `usb/eth`|
|Cùng tên có ở Kconfig và header|`CONFIG_USB_XHCI` (Kconfig, default theo `USB_XHCI_HCD`) vs `CONFIG_USB_XHCI_HCD`|
|Defconfig đặt nhưng phụ thuộc không thoả|`CONFIG_CMD_MVEBU_MPP=y` bị bỏ vì `MVEBU_MPP_BUS` tắt|
|`#undef` rồi `#define` lại|`CONFIG_BOOTDELAY`, `CONFIG_USE_IRQ` (mvebu-common `#undef`, armada8k `#define`)|
|Khối chỉ bật khi macro khác|`mvebu-common.h:449-481` (SCSI/EXT4/JFFS2) chỉ khi có SATA/`MVEBU_PCI`, không áp cho board này|
|Macro cho mã đã tắt|`CONFIG_NVME_SECURITY_SUPPORT` (comment), `CONFIG_CMD_NET` (không)|

## 6. Failure

|Điều kiện|Hậu quả|
|---|---|
|Đổi header cấu hình mà build bằng `update`|Có thể dùng `.config` cũ; `autoconf.mk` phụ thuộc `common.h` nên thường tự cập nhật, nhưng không đảm bảo|
|Bật `KM_MACHINE_*` nhưng không đổi `HWCHECK_MEMSIZE*`|So sánh DIMM sai → `BOOT-DIAG ERROR/Dimm`|
|Hai `KM_MACHINE_*` cùng bật|Chuỗi `#elif` chọn cái đầu; không có lỗi biên dịch, dễ ra cấu hình sai|
|`-Werror` + cảnh báo redefine|Có thể chặn build|

## 7. COMMON CONFUSION

1. **"Build 800 = Emu800"**: sai. Cấu hình `800` là bản Quartz dùng chung (`ZeusS_ZX0`, `FW0000`); `Emu800` là tên DT (`armada-quartz-emu800`) và cách đặt tên thư mục/repo.
2. **"Nhánh `KM_BOARD_EMU800` đang chạy"**: không; nhánh `#else` của `CONFIG_KM_BOARD_EMU800` mới chạy.
3. **`.config` là toàn bộ cấu hình**: chỉ là Kconfig; header còn quyết định nhiều.
4. **`bootdelay = 2`**: giá trị cuối là 3.
5. **`CONFIG_KM_BIZHUB` = `KM_BIZHUB`**: hai macro riêng, cùng đúng ở đây.
6. **Machine được chọn khi biên dịch**: ở bản `800`, được suy ra khi chạy từ ROM (`MachineInfo`/`MachineNo`), chỉ chuỗi tên/`FwDir` cố định.

## 8. MENTAL MODEL

```
defconfig ─► Kconfig(.config) ┐
armada8k.h ─► mvebu-common.h ─► quartz-sb.h ─► asm/config.h ─► config_fallbacks.h ┼─► macro CONFIG_* cuối cùng
-D KM_BIZHUB, TEXT_BASE                                                          ┘        │
                                                             Makefile (autoconf.mk) ──► obj-y
Bản 800:  không KM_MACHINE_*  ⇒  FW0000 / ZS_ZX0_PF / ZeusS_ZX0 ; loại máy thật đọc từ SPI-flash lúc chạy
```

## 9. Chưa xác minh

- Chi tiết `MACHINE`, `db_sw` (điều kiện biên dịch).
- Dung lượng DIMM thực và ý nghĩa `HWCHECK_MEMSIZE = 8 GB` với `QUARTZ_RAM_SIZE = 2 GB`.
- `CONFIG_WARP_LOAD_ADDR` bị định nghĩa lại hai lần.
- `.config` thực từ một lần build.
- Nội dung `KM_NVME`/`KM_PCIE_SET_MPS` Kconfig (help/phụ thuộc).

---

Xong Phần 26. Tiếp theo **Phần 27: Linker script**: `arch/arm/cpu/armv8/u-boot.lds`, các section, ký hiệu `__image_copy_*`, `__rel_dyn_*`, `_end`, `.u_boot_list`, bss.

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\u-boot.lds](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\lib\sections.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\start.S](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 2-65)

Grep "ARMV8_PSCI|ARMV8_SECURE_BASE" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {arch/arm/cpu/armv8/Makefile,arch/arm/cpu/armv8/armada8k/Makefile,arch/arm/cpu/armv8/armada8k/Kconfig,arch/arm/Kconfig,include/configs/armada8k.h,include/configs/mvebu-common.h,arch/arm/include/asm/config.h,configs/km_mvebu_quartz_toc_defconfig,board/mvebu/Kconfig})

3 lines of output

Grep "__image_copy|_bss_start_ofs|_bss_end_ofs|_end_ofs|_TEXT_BASE|__bss_start|__bss_end|mon_len" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {arch/arm/lib/crt0_64.S,common/board_f.c,common/board_r.c})

21 lines of output

# PHẦN 27 (PART 28): Linker Script

## 0. Kết luận

Chỉ có **một** linker script: `arch/arm/cpu/armv8/u-boot.lds` (LDSCRIPT được chọn theo thứ tự board → CPUDIR → arch, Phần 25; không có `board/mvebu/armada8k/u-boot.lds`). Nó rất ngắn. Địa chỉ 0x1000 **không** nằm trong script mà đến từ `-Ttext 0x1000` ở dòng lệnh. Bố cục quan trọng: **mọi thứ cần chép khi relocation nằm giữa `__image_copy_start` và `__image_copy_end`; `.rela.dyn` và `.bss` nằm ngoài vùng chép.** `.secure_text`/PSCI **không** có (defconfig không bật `ARMV8_PSCI`; Kconfig tại `armada8k/Kconfig:23`).

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Bản đồ bộ nhớ của ảnh U-Boot: thứ tự section, ký hiệu ranh giới|
|WHY|`crt0_64.S`, `relocate_64.S`, `board_f.c` dùng ký hiệu này để chép, vá con trỏ, xoá BSS, tính `mon_len`|
|WHERE|`arch/arm/cpu/armv8/u-boot.lds`, `arch/arm/lib/sections.c`, `arch/arm/cpu/armv8/start.S`, `arch/arm/lib/{crt0_64.S,relocate_64.S}`, `common/board_f.c`|

## 2. Bố cục section (theo thứ tự trong file)

```
OUTPUT_FORMAT elf64-littleaarch64 ; ENTRY(_start)
. = 0                                    (bị -Ttext 0x1000 ghi đè cho .text)
.text        : { *(.__image_copy_start)     ← nhãn 0 byte đánh dấu đầu ảnh
                 CPUDIR/start.o (.text*)     ← ÉP start.S lên đầu ⇒ _start = 0x1000
                 *(.text*) }
[ .__secure_start / .secure_text / .__secure_end ]   chỉ khi CONFIG_ARMV8_PSCI (KHÔNG có ở bản này)
.rodata      : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }
.data        : { *(.data*) }
.u_boot_list : { KEEP(*(SORT(.u_boot_list*))) }        ← bảng lệnh/driver, giữ dù không có tham chiếu
.image_copy_end : { *(.__image_copy_end) }             ← hết vùng CHÉP
.rel_dyn_start / .rela.dyn { *(.rela*) } / .rel_dyn_end  ← bảng relocation
_end = .
.bss_start { KEEP(*(.__bss_start)) } ; .bss { *(.bss*) ; ALIGN(8) } ; .bss_end { KEEP(*(.__bss_end)) }
/DISCARD/ : .dynsym .dynstr* .dynamic* .plt* .interp* .gnu*
```

Ghi chú:

- **`.text` được đặt bởi `-Ttext`**: script chỉ có `. = 0`. Với `-Ttext 0x1000` (Makefile:755) phần `.text` bắt đầu tại 0x1000, các section sau nối tiếp. Nếu quên `-Ttext` thì ảnh liên kết ở 0 và `relocate-rela`/ATF sẽ sai (Phần 25).
- **`start.o` bị ép lên đầu `.text`** để lệnh `b reset` ở `_start` nằm đúng địa chỉ nhập của BL33. Các vùng đầu: `_start` (`b reset`), rồi bốn hằng 8 byte: `_TEXT_BASE`, `_end_ofs`, `_bss_start_ofs`, `_bss_end_ofs` (start.S:44-63), rồi `reset:`.
- **`.dtb.init.rodata` (DTB nhúng) không xuất hiện trong script.** Mẫu `.rodata*` không khớp tên này, nên nó là **section mồ côi** (orphan) do `ld` tự xếp; `objcopy -j .dtb.init.rodata` (Phần 25) mới đưa nó vào `u-boot.bin`. Vị trí chính xác của nó so với `__image_copy_end` **không xác minh được nếu không có `u-boot.map`** [?]. Nếu nó bị xếp sau `.image_copy_end` thì DTB sẽ **không** được chép lên RAM cao và `gd->fdt_blob` phải trỏ vào bản gốc ở 0x1000-based; do khai báo DTB nhúng được các phần trước xác nhận là chạy tốt sau relocation (`fdt_blob` đã relocate trong `reloc_fdt`?), nên nhiều khả năng nó nằm trong vùng chép [S].
- `.u_boot_list` được `KEEP`, sắp theo tên (`SORT`): đảm bảo thứ tự bảng lệnh và tránh `--gc-sections` loại bỏ (Phần 15, 19).
- `.rela.dyn` **nằm sau** `.image_copy_end`: `relocate_code` không chép nó lên RAM cao, mà **đọc thẳng từ vị trí gốc** (`__rel_dyn_start/end` tính theo địa chỉ nguồn, relocate_64.S:44-45). Vì vậy vùng gốc ở 0x1000.. **phải còn nguyên** đến khi relocation xong.
- `_end` đặt sau `.rela.dyn` chứ không sau `.bss`; nhưng `setup_mon_len` (board_f.c:284) dùng `&__bss_end - _start` cho `mon_len` (kích thước vùng dành cho U-Boot trên RAM cao, gồm cả BSS).
- `/DISCARD/` loại `.dynsym/.dynstr/.dynamic/.plt/.interp/.gnu*`: ảnh không cần động; chỉ giữ `.rela.dyn` cho relocation kiểu RELATIVE.

## 3. Ký hiệu ranh giới và cách chúng được khai báo

`arch/arm/lib/sections.c` khai báo bằng **mảng 0 phần tử trong C**, đặt vào section riêng:

```c
char __bss_start[0]        __attribute__((section(".__bss_start")));
char __bss_end[0]          __attribute__((section(".__bss_end")));
char __image_copy_start[0] __attribute__((section(".__image_copy_start")));
char __image_copy_end[0]   __attribute__((section(".__image_copy_end")));
char __rel_dyn_start[0]    ... ; char __rel_dyn_end[0] ...
char __secure_start[0] __secure_end[0] _end[0] __end_of_spl[0]
```

Lý do (comment sections.c:7-20): nếu định nghĩa ký hiệu trong linker script thì `ld` dùng relocation tuyệt đối (ABS) cho tham chiếu tới chúng, còn định nghĩa trong C thì dùng **RELATIVE**, đảm bảo giá trị đúng cả **trước lẫn sau relocation** (và `checkarmreloc` chỉ chấp nhận RELATIVE, Phần 25). Đây là lý do `crt0_64.S:119-120` có ghi chú "this is auto-relocated!".

Lưu ý: `_end` vừa có `char _end[0]` (section `.__end`) vừa `_end = .;` trong script. Build sản phẩm vẫn qua nên ld chấp nhận; giá trị `_end` được dùng chỉ ở `board_r.c:169` (`monitor_flash_len = _end - __image_copy_start`) và `start.S:55` (`_end_ofs`). Chưa xác minh giá trị thực do định nghĩa nào thắng [?].

## 4. Ai dùng ký hiệu nào

|Ký hiệu|Người dùng|Mục đích|
|---|---|---|
|`_start`|ATF (nhảy vào BL33)|điểm vào tại 0x1000|
|`__image_copy_start`/`_end`|`relocate_64.S:29,32`|phạm vi chép ảnh lên RAM cao|
|`__rel_dyn_start`/`_end`|`relocate_64.S:44-45`, Makefile (`relocate-rela`)|danh sách relocation|
|`__bss_start`/`_end`|`crt0_64.S:119-120` (xoá BSS sau relocation), `board_f.c:164` (in bố cục), `setup_mon_len`||
|`_TEXT_BASE` (`.quad CONFIG_SYS_TEXT_BASE`)|crt0/start|biết địa chỉ liên kết để tính `reloc_off`|
|`_end_ofs`, `_bss_start_ofs`, `_bss_end_ofs`|mã asm cần độ lệch tương đối||
|`.u_boot_list` (`ll_entry_start`)|`find_cmd`, `help` (Phần 15)|bảng lệnh|
|`gd->reloc_off = relocaddr - CONFIG_SYS_TEXT_BASE`|board_f.c:754-755|độ dời cho toàn bộ vá con trỏ|

## 5. Vòng đời bộ nhớ theo bố cục

```
0x1000                                 __image_copy_start (= _start)
   │ .text (start.S trước) │ .rodata │ [.dtb.init.rodata ?] │ .data │ .u_boot_list │
   │                                                                  __image_copy_end
   │ .rela.dyn (chỉ đọc, KHÔNG chép)                                   _end
   │ .bss (trước relocation chưa dùng; xoá sau relocation ở bản đã chép)
   ▼ (ATF chỉ nạp .bin: text … rela.dyn; BSS không có trong .bin)
relocate_code: copy [image_copy_start, image_copy_end) → relocaddr; vá con trỏ RELATIVE bằng .rela.dyn gốc
crt0_64.S: clear BSS [__bss_start, __bss_end) (địa chỉ đã relocate)
```

Ràng buộc kích thước: `CONFIG_UBOOT_SIZE = 2 MiB` (mvebu-common.h:74) là hằng danh nghĩa; không có kiểm tra `ASSERT` trong script. Vùng dưới `CONFIG_SYS_INIT_SP_ADDR = 0xFF1000` (Phần 3): U-Boot tại 0x1000 + BSS + `gd` phải nhỏ hơn ~16 MiB, nếu không sẽ đè lên stack ban đầu. `gd` cực lớn của KM (~0x418C8 do `printBuffer[255][1051]`) nằm trong stack vùng đó.

Bộ ATF ở `0x10000000` và training ở `0x10400000` nằm ngoài dải 0x1000–0xFF1000, nên không xung đột (Phần 2).

## 6. Failure

|Điều kiện|Hậu quả|
|---|---|
|ATF nạp ảnh không phải 0x1000|`relocate-rela` đã ghi tuyệt đối cho 0x1000: mọi con trỏ tĩnh sai; `board_init_f` nhảy sai hoặc crash rất sớm (không có log)|
|Ảnh + BSS vượt 0xFF1000|Đè stack ban đầu; treo/ngẫu nhiên|
|Thêm section mới không có trong script|Thành section mồ côi, có thể rơi ngoài vùng chép ⇒ mất dữ liệu sau relocation|
|`.u_boot_list` không KEEP|`gc-sections` loại lệnh không tham chiếu|
|Loại `.rela.dyn` khỏi `.bin`|`relocate_code` không có bảng, mọi con trỏ sau relocation sai|

## 7. COMMON CONFUSION

1. **"Địa chỉ nạp nằm trong `.lds`"**: nó ở `-Ttext`.
2. **"`_end` = hết BSS"**: ở đây `_end` là hết `.rela.dyn`; hết BSS là `__bss_end`.
3. **".bin có BSS"**: không, BSS chỉ có sau khi xoá ở crt0.
4. **"Các ký hiệu là hằng do linker"**: chúng là biến C 0 byte để có relocation RELATIVE.
5. **`.rela.dyn` chép lên RAM cao**: không, đọc tại chỗ gốc.

## 8. MENTAL MODEL

```
[start.o][text][rodata][dtb?][data][u_boot_list] | copy_end | [rela.dyn] | _end | [bss…]
 └──────────── chép lên RAM cao ─────────────┘     └ chỉ đọc ┘            └ xoá về 0 sau chép
```

## 9. Chưa xác minh

- Vị trí `.dtb.init.rodata` (cần `u-boot.map`).
- Định nghĩa `_end` thắng.
- Kích thước ảnh thực.
- `crt0_64.S` chi tiết xử lý `x18/gd` (đã tóm tắt ở Phần 3).

---

Xong Phần 27. Tiếp theo **Phần 28: Interrupt/Exception**: bảng vector `exceptions.S`, xử lý sync abort, ESR/ELR, `show_regs`, `panic`.

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\cpu\armv8\exceptions.S](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

Bash Inspect hang(), panel boot-diag codes and functions

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && cat common/hang.c | sed -n 1,40p; echo ----; grep -n "PANEL_BOOTDIAG" board/mvebu/armada8k/km/*.h | head -30; echo ----; grep -n "hwcDispLoadDiag\|hwcDispBootDeviceDiag\|set_Panel_BootDiagErr\|set_Panel_BootDiagResult\|set_Panel_BootDiagStatus" board/mvebu/armada8k/km/mfp_panel.c | head -20

OUT

cat: common/hang.c: No such file or directory
----
board/mvebu/armada8k/km/mfp_panel.h:56:#define PANEL_BOOTDIAG_ST0				0
board/mvebu/armada8k/km/mfp_panel.h:57:#define PANEL_BOOTDIAG_ST1				1
board/mvebu/armada8k/km/mfp_panel.h:58:#define PANEL_BOOTDIAG_ST2				2
board/mvebu/armada8k/km/mfp_panel.h:59:#define PANEL_BOOTDIAG_ST3				3
board/mvebu/armada8k/km/mfp_panel.h:60:#define PANEL_BOOTDIAG_ST4				4
board/mvebu/armada8k/km/mfp_panel.h:61:#define PANEL_BOOTDIAG_ST5				5
board/mvebu/armada8k/km/mfp_panel.h:63:#define PANEL_BOOTDIAG_CPU_BORAD_ERR	0x01	// CPU��s��.
board/mvebu/armada8k/km/mfp_panel.h:64:#define PANEL_BOOTDIAG_BASE_BORAD_ERR	0x02	// �x�[�X��s��.
board/mvebu/armada8k/km/mfp_panel.h:65:#define PANEL_BOOTDIAG_STORAGE_ERR		0x04	// �X�g���[�W�s��.
----
210:void set_Panel_BootDiagStatus(unsigned char uc_DiagSts)
1140:void hwcDispBootDeviceDiag(void)
1145:		set_Panel_BootDiagErr(PANEL_BOOTDIAG_STORAGE_ERR);
1146:		set_Panel_BootDiagResult();
1152: * Function Name [ set_Panel_BootDiagErr ]
1161:void set_Panel_BootDiagErr(unsigned int ui_DiagErr)
1167: * Function Name [ set_Panel_BootDiagResult ]
1177:void set_Panel_BootDiagResult(void)
1221: * Function Name [ hwcDispLoadDiag ]
1230:void hwcDispLoadDiag(void)
1233:		set_Panel_BootDiagErr(PANEL_BOOTDIAG_STORAGE_ERR);	// �X�g���[�W�s�Ǖۑ�.
1234:		set_Panel_BootDiagResult();							// LED�_��.

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\mfp_panel.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 201-260)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\mfp_panel.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 1131-1260)

Grep "void hang\(" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {common/*.c,lib/*.c,arch/arm/lib/*.c})

9 lines of output

# PHẦN 28 (PART 29): Interrupt / Exception

## 0. Kết luận

U-Boot chỉ có **một bảng vector 8 mục**, dành cho ngoại lệ **cùng EL** (EL hiện tại), mọi ngoại lệ đều dẫn tới `panic` rồi **reset máy** (Phần 24). Không có xử lý hồi phục, không có ngăn xếp ngoại lệ riêng, không in `FAR_ELx`, và thông báo thường **không hiển thị** trên máy sản xuất vì bộ đệm log của KM. Vì DAIF do ATF bàn giao thường che mọi ngoại lệ bất đồng bộ, chỉ **ngoại lệ đồng bộ** (lỗi truy cập, lệnh sai) thực sự xảy ra trong thực tế [S].

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Cách CPU chuyển điều khiển khi có ngoại lệ (đồng bộ, IRQ, FIQ, SError)|
|WHY|Chẩn đoán lỗi nghiêm trọng; ngăn treo ngầm|
|WHERE|`arch/arm/cpu/armv8/exceptions.S`, `arch/arm/lib/interrupts_64.c`, `arch/arm/cpu/armv8/start.S:65-90,233-245`, `lib/vsprintf.c:845` (`panic`), `lib/hang.c`|

## 2. Bảng vector (exceptions.S:55-80)

`vectors` căn **2 KB** (`.align 11`), mỗi mục cách 128 byte (`.align 7`), chỉ **8 mục**:

|Offset|Loại|Nhãn|Hàm C|
|---|---|---|---|
|0x000|Sync, SP_EL0 (Current EL, Thread)|`_do_bad_sync`|`do_bad_sync`|
|0x080|IRQ, SP_EL0|`_do_bad_irq`|`do_bad_irq`|
|0x100|FIQ, SP_EL0|`_do_bad_fiq`|`do_bad_fiq`|
|0x180|SError, SP_EL0|`_do_bad_error`|`do_bad_error`|
|**0x200**|**Sync, SP_ELx (Handler)**|`_do_sync`|`do_sync`|
|**0x280**|**IRQ, SP_ELx**|`_do_irq`|`do_irq`|
|**0x300**|**FIQ, SP_ELx**|`_do_fiq`|`do_fiq`|
|**0x380**|**SError, SP_ELx**|`_do_error`|`do_error` (weak)|

- U-Boot chạy với `SPSel=1` (dùng `SP_ELx`), nên ngoại lệ thường rơi vào bốn mục đậm (0x200–0x380). Bốn mục "bad" (SP_EL0) chỉ xảy ra khi SP bị chuyển sai.
- **Không có mục cho "Lower EL"** (offset 0x400–0x7FF). Mã `_do_bad_sync…_do_error` nằm ngay sau `b _do_error` ở 0x380, tức đè lên vùng lẽ ra dành cho Lower EL. Ngoại lệ từ EL thấp hơn (không xảy ra vì U-Boot không hạ EL) sẽ chạy vào mã ngẫu nhiên.
- Mỗi mục là một lệnh `b` tới trình bao ngoài: `exception_entry` rồi `bl do_xxx`. Sau `bl` **không có `eret`**: các hàm C không quay về (đều `panic`).

## 3. `exception_entry` (exceptions.S:20-50)

```
push x0..x30 theo cặp lên stack HIỆN HÀNH (dùng SP hiện tại, không đổi stack)
switch_el:  EL3→ esr_el3/elr_el3 ; EL2→ esr_el2/elr_el2 ; EL1→ esr_el1/elr_el1
x1 = ESR ; x2 = ELR ; push (x2, x0) ; x0 = sp  (con trỏ struct pt_regs)
→ do_xxx(struct pt_regs *regs, unsigned int esr)
```

- Xử lý ở **ngăn xếp đang dùng**: nếu ngoại lệ do tràn stack hoặc SP hỏng thì lần lưu thanh ghi tiếp theo lại sinh ngoại lệ lồng nhau (vòng lặp fault). Không có ngăn xếp ngoại lệ riêng (`CONFIG_STACKSIZE_IRQ` không được dùng, Phần 24).
- **Không lưu SPSR** và **không đọc `FAR_ELx`** (địa chỉ gây lỗi). Chỉ có ESR và ELR.

## 4. Xử lý trong C (interrupts_64.c)

```c
do_sync / do_irq / do_fiq / do_error:
     printf("\"Synchronous Abort\" handler, esr 0x%08x\n", esr);     // hoặc Irq/Fiq/Error
     show_regs(pt_regs);       // ELR, LR, x0..x29 (2 thanh ghi/dòng)
     panic("Resetting CPU ...\n");
do_bad_*: giống, với tiêu đề "Bad mode in ... handler"
panic():  vprintf; putc('\n'); udelay(100 ms); do_reset(NULL,0,0,NULL); while(1);
```

- `panic` **không** đi qua `hang()` (vì `CONFIG_PANIC_HANG` không định nghĩa). `hang()` (`lib/hang.c:22`) chỉ in `### ERROR ### Please RESET the board ###` và vòng lặp vô hạn; nó được dùng cho lỗi chuẩn bị kernel (Phần 16) chứ không phải ngoại lệ.
- **Giải mã ESR** phải làm bằng tay: `EC = ESR[31:26]` (0x25 = data abort cùng EL; 0x21 = instruction abort cùng EL; 0x00 = unknown; 0x15 = SVC; 0x3C = BRK), `ISS = ESR[24:0]` (DFSC: 0x04–0x07 lỗi dịch địa chỉ, 0x21 alignment, 0x10 lỗi ngoại vi đồng bộ…). Mã U-Boot không giải mã.

## 5. Ai thực sự gây ngoại lệ trong thực tế

|Nguồn|Điều kiện|Ghi chú|
|---|---|---|
|NULL/địa chỉ xấu|`y`/`z` không đối số (Phần 19), lệnh `md`/`mm` tới địa chỉ không ánh xạ hoặc thiết bị không phản hồi|Data abort; vùng ngoài RAM là Device nên truy cập vẫn "hợp lệ" ở MMU, nhưng thiết bị/ATF có thể trả bus error|
|Bus error từ thiết bị|Đọc thanh ghi NVMe/PCIe khi liên kết mất, đọc IOB/CCU cửa sổ chưa cấu hình|Có thể thành SError (không đồng bộ) hoặc sync abort|
|Alignment|`SCTLR.A`/Device-memory với MMU tắt (đường NVMe)|Được giảm nhờ `-mstrict-align` (Phần 25)|
|Lệnh sai|Nhảy vào dữ liệu (`go` sai địa chỉ), con trỏ hỏng|Không XN nên nhảy được vào bất kỳ đâu (Phần 23)|
|FP/SIMD|Được bật ở `reset:` (`cptr_el2 = 0x33ff`, `cpacr_el1`)|Không trap|
|SError/IRQ/FIQ|DAIF do ATF che|Không thực sự vào (xem 6)|

## 6. Trạng thái che ngắt (DAIF)

- U-Boot **không bao giờ** ghi DAIF (`enable_interrupts`/`disable_interrupts` là hàm rỗng, Phần 24).
- ATF chuyển vào BL33 với D,A,I,F **được che** (mặc định của ATF cho non-secure entry) [S, ngoài repo].
- Hệ quả: IRQ/FIQ **không bao giờ** được giao; SError (bit A) cũng bị che, nên lỗi bus bất đồng bộ (ví dụ từ ghi đệm) **không** kích hoạt `do_error`; chúng có thể trở thành lỗi im lặng hoặc bị kernel thấy sau này. Còn ngoại lệ đồng bộ và debug (D) thì: sync luôn giao được; debug bị che.

## 7. Thiết lập VBAR

- `reset:` (start.S:70-86): `adr x0, vectors; msr vbar_elX, x0` (theo `switch_el`), địa chỉ **trước relocation** (ở 0x1000-based).
- `c_runtime_cpu_setup` (start.S:233): sau relocation, nạp lại `vbar_elX` với bản `vectors` đã chép (`adr x0, vectors`). Nếu ngoại lệ xảy ra trong khoảng giữa (relocation chưa xong) thì vẫn vào bản gốc, vẫn còn nguyên (vì vùng nguồn không bị ghi đè).
- Trước khi vào kernel VBAR vẫn trỏ vào U-Boot (Phần 18).

## 8. Log và hiển thị (điểm dễ bỏ sót)

`printf` trong handler đi qua đường console của KM: khi `termFlg` ẩn console (máy sản xuất, Phần 9), chuỗi chỉ vào bộ đệm `printBuffer` và **không bao giờ in ra** trước khi `do_reset` xoá máy. Vì bộ đệm nằm trong `gd` (RAM), nó **mất** khi reset. Vậy trên máy thật thường **không có dấu vết nào** của ngoại lệ; chỉ khi bật đầu ra console (GPIO/SoftDIP cho phép) mới thấy ESR/ELR/thanh ghi.

## 9. Failure và vòng khởi động lại

- Mọi ngoại lệ → reset sau ~150 ms (50 ms `do_reset` + 100 ms `panic`) → khởi động lại → nếu lỗi ổn định thì **lặp** (không có bộ đếm như `reset_with_recoverycode`).
- WDT (R4) không có vai trò vì máy tự reset trước.
- Ngoại lệ lồng nhau (do stack hỏng) dẫn tới treo trong handler, không tới `panic`; khi đó WDT R4 (nếu đang mở) là lưới bảo vệ duy nhất.

## 10. COMMON CONFUSION

1. **"Có xử lý ngoại lệ hồi phục"**: không; chỉ báo rồi reset.
2. **"Handler in FAR"**: không; chỉ ESR, ELR, x0–x30.
3. **"Có vector Lower EL"**: không.
4. **"IRQ được bật để chạy timer"**: không, polling.
5. **"Panic treo"**: reset.
6. **"Log ngoại lệ luôn thấy trên UART"**: không, phụ thuộc console bị ẩn.

## 11. MENTAL MODEL

```
sync fault ─► vectors+0x200 ─► exception_entry (lưu x0–x30 trên stack hiện tại; ESR, ELR)
                              ─► do_sync ─► printf ESR ─► show_regs ─► panic ─► udelay ─► do_reset ─► (khởi động lại)
IRQ/FIQ/SError: bị che bởi DAIF nên thực tế không tới
```

## 12. Chưa xác minh

- Giá trị DAIF khi vào BL33.
- Bố cục `struct pt_regs` (`asm/ptrace.h`) khớp thứ tự lưu.
- Có nơi nào đặt `SCTLR.A` hay không.

---

# PHẦN 29 (PART 30): Error Handling

## 0. Kết luận

Xử lý lỗi trong U-Boot này là **năm cơ chế xếp lớp**, khác nhau tuỳ giai đoạn. Không có mã lỗi thống nhất: mỗi hàm trả `int`/`bool` theo cách riêng, người gọi thường **không kiểm tra**.

|Lớp|Cơ chế|Kết cục|
|---|---|---|
|L1|Mã trả về (`CMD_RET_*`, `0/1/-1`, `bool`)|Lệnh hush chuyển tiếp; `;` **không dừng** khi lỗi|
|L2|`printf` báo lỗi rồi tiếp tục|Boot vẫn chạy (ví dụ `GM update failed.`, MAC không hợp lệ)|
|L3|`hang()` (vô hạn, không WDT refresh)|Chờ WDT R4 hoặc cắt nguồn|
|L4|`panic()` → `do_reset()`|Khởi động lại (không bộ đếm)|
|L5|`_WAIT_FOREVER()` (vô hạn, **có** refresh WDT) + LED panel|Giữ trạng thái lỗi cho kỹ thuật viên|
|L6|`reset_with_recoverycode()` (≤5 lần)|Reset có ghi mã lỗi cho firmware chính|
|L7|`chk_sboot` (WDT tắt, `while(1)`)|Khoá vĩnh viễn|

## 1. Mã trả về và lan truyền

- Lệnh: `CMD_RET_SUCCESS=0`, `FAILURE=1`, `USAGE=-1` (`cmd_process`, Phần 15). Hush đặt `$?`; **dấu `;` không phụ thuộc kết quả**, nên `bootcmd` không dừng khi `fatload` lỗi (Phần 16).
- Hàm KM dùng bảng mã riêng, ví dụ SPI-flash: `THRD_SET_SPI_OK 0`, `NG_PARAM 10`, `AREA1_ERR 21`, `AREA2_ERR 22`, `READ_ERR 30`, `ERASE_ERR 31`, `WRITE_ERR 32`, `RE_READ_COUNT 40`, `COUNT_NOT_EQUAL 41`, `W_PROTECT_ERR 50`, `W_UNPROTECT_ERR 51`; đọc: `GET_SPI_OK 0`, `OK_AREA1_ERR 1`, `OK_AREA2_ERR 2` (dữ liệu vẫn dùng được nhờ bản sao), `NG_PARAM 10`, `NG_ALL_AREA 20`, `NG_READ_ERR 30` (mtd_flash_access.h:21-39). Vùng SoftDIP/BSP/machineinfo có **hai bản (A/B) + bộ đếm**, nên mất một bản vẫn khôi phục (`OK_AREAx_ERR`).
- Nhiều nơi bỏ qua lỗi: kết quả `send_cmd`/`km_scpi_set_wdt_ca72_ext` (Phần 22), `i2c_write` tới PS-CPU, `THRG_getSoftDipSw` thất bại thì mặc định 0 ("Read Error → flag = 0"), `read_rom()` NULL thì thoát sớm.

## 2. Chỉ báo lỗi cho người dùng: LED panel

Kênh báo lỗi chính khi không có console là **đèn LED trên panel**, điều khiển bằng khung UART đến vi điều khiển panel (COM2, 57600 8E1):

```
khung = { 0xFE, 0x06, 0x30, LED1, LED2, 0, 0, 0xFF } + checksum (make_CheckSum), gửi SendData, nhận RecvData
LED1 = Power(xanh/cam: bit0, bit2) + Warning (bit4);  LED2 = Data (bit2)     trạng thái: OFF / ON / FLASH
```

|Hàm|Ý nghĩa|
|---|---|
|`set_Panel_BootDiagStatus(ST0..ST5)` (mfp_panel.c:210)|Tiến trình boot: ST1 = Power(cả hai) ON, Data OFF, Warning OFF; ST2 = Data nhấp nháy; ST3 = Data ON; ST4 = Data ON + Warning nhấp nháy; ST5 = tất cả ON (ngay trước `OS_GO` ở chế độ HW-check)|
|`set_Panel_BootDiagErr(mask)`|Tích luỹ lỗi vào `g_PanelBootDiagErr`: `CPU_BORAD_ERR 0x01`, `BASE_BORAD_ERR 0x02`, `STORAGE_ERR 0x04`|
|`set_Panel_BootDiagResult()` (:1177)|Hiển thị: CPU lỗi = **Data LED nhấp nháy**, các LED khác tắt; Base lỗi = **Power LED nhấp nháy**; Storage lỗi = **Warning LED nhấp nháy**|
|`hwcDispBootDeviceDiag()` (:1140)|Chế độ HW-check mà không có thiết bị boot → mã `0x41`, Storage lỗi, `_WAIT_FOREVER`|
|`hwcDispLoadDiag()` (:1230)|Khi nạp/`booti` lỗi (gọi từ `do_bootm_states`) ở chế độ HW-check → Storage lỗi + `_WAIT_FOREVER`|

Lưu ý: `set_Panel_BootDiagResult` **cộng dồn (`+=`)** vào `static uc_setLed1/2` mỗi lần gọi; gọi nhiều lần (ví dụ hai lỗi) làm các giá trị LED cộng chồng lên nhau và có thể tràn `unsigned char` hoặc tạo tổ hợp ngoài ý muốn [V bằng đọc code: các biến `static` không được xoá giữa các lần gọi].

Mã tự chẩn đoán lưu bộ nhớ CSRA: `THRF_setAutoHwDiagnosCUP(0x02)` (DIMM), `THRF_setAutoHwDiagnosStorage(0x41/0x46)` (không có thiết bị boot / hash sai), phục vụ tra cứu sau.

## 3. Bảng kịch bản lỗi thường gặp

|Sự kiện|Phát hiện ở|Phản ứng|
|---|---|---|
|DIMM sai kích thước (HW-check)|`init_km`|`### BOOT-DIAG ERROR/Dimm ###`, `CPU_BORAD_ERR`, `_WAIT_FOREVER`|
|UART panel lỗi (HW-check)|`init_km`|`### BOOT-DIAG ERROR/Panel ###`, `_WAIT_FOREVER`|
|Không thấy SSD/SD (HW-check)|`hwcDispBootDeviceDiag`|Storage lỗi, `_WAIT_FOREVER`|
|Hash sai (tự chẩn đoán)|`boot_diag`|`Hash Error!!!`, Storage lỗi, `_WAIT_FOREVER`|
|Nạp kernel lỗi (HW-check)|`do_bootm_states`|`hwcDispLoadDiag`|
|Nạp kernel lỗi (thường)|`booti`|Trả 1, `cli_loop`, WDT đã END|
|Đồ hoạ lỗi (LVDS/PNG/LZO)|drivers video, `graphlib`|`reset_with_recoverycode(0x01–0x0c)`|
|Chip bảo mật bị tháo|`chk_sboot`|Khoá vĩnh viễn|
|Ngoại lệ CPU|handler|`panic` → reset|
|Không tạo được FDT|`boot_prep_linux`|`hang()`|
|R4 không phản hồi|`send_cmd`|Bỏ qua, chậm boot|

## 4. Thứ tự ưu tiên khi nhiều lỗi

Các kiểm tra được chạy theo dòng chảy của `init_sequence_r`: `init_km` (DIMM, UART panel) → `misc_init_r` (ROM, panel, USB, `boot_diag`, cờ tự chẩn đoán) → `main_loop` (`chk_sboot`, boot). Lỗi đầu tiên gặp là lỗi được báo; các lỗi sau không bao giờ được kiểm nếu `_WAIT_FOREVER` đã chạy.

## 5. Điểm yếu đã xác nhận trong xử lý lỗi

1. Kết quả nhiều lệnh KM bị bỏ qua (SCPI, I2C).
2. `set_Panel_BootDiagResult` cộng dồn không xoá (mục 2).
3. `reset_with_recoverycode` **không reset** sau lần thứ sáu, và lỗi đồ hoạ khi đó chỉ được in; tiếp tục với đồ hoạ hỏng.
4. `panic` không có bộ đếm (Phần 28).
5. Không có phân biệt "lỗi tạm thời" và "vĩnh viễn" ở `chk_sboot` (Phần 21).
6. `;` trong `bootcmd` không dừng khi lỗi (Phần 16).
7. Log lỗi phụ thuộc console bị ẩn (Phần 28).

## 6. COMMON CONFUSION

1. **"Lỗi nào cũng dẫn tới treo"**: nhiều lỗi chỉ in và chạy tiếp.
2. **"`_WAIT_FOREVER` giống `hang`"**: cái đầu nạp WDT.
3. **"LED nhấp nháy = một mã duy nhất"**: có ba loại (CPU/Base/Storage) và có thể cộng dồn.
4. **"Mã recovery là mã lỗi U-Boot chung"**: chỉ là mã 0x01–0x0c của đồ hoạ.

## 7. MENTAL MODEL

```
lỗi nhẹ ─► printf + tiếp tục
lỗi chẩn đoán ─► LED panel + _WAIT_FOREVER (WDT vẫn nạp)
lỗi đồ hoạ ─► reset_with_recoverycode (≤5) ─► firmware chính đọc mã 0x2201C0000000xxyy
lỗi nặng ─► hang (chờ WDT R4)  |  ngoại lệ ─► panic ─► reset
bảo mật ─► chk_sboot ─► khoá vĩnh viễn
```

## 8. Chưa xác minh

- Ý nghĩa cụ thể mã LED trên tài liệu dịch vụ của máy.
- Cách firmware chính xử lý `0x2201C0000000xxyy`.
- `PANEL_LED_*` giá trị số (không đọc header).

---

Xong Phần 28 và 29. Tiếp theo **Phần 30: Custom/Vendor code**: bảng File / Function / Upstream / Vendor modification / Reason / Runtime effect.

Bash List all files carrying KM customization markers

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && grep -rlE "KM_BIZHUB|KonicaMinolta" --include=*.c --include=*.h --include=*.S --include=Makefile --include=Kconfig --include=*.dts --include=*.dtsi . 2>/dev/null | sort | head -120; echo; grep -rlE "KM_BIZHUB" --include=*.c --include=*.h --include=*.S . | wc -l

OUT

./Kconfig
./Makefile
./arch/arm/cpu/armv8/armada8k/clock.c
./arch/arm/cpu/armv8/armada8k/soc.c
./arch/arm/cpu/armv8/cache.S
./arch/arm/cpu/armv8/cache_v8.c
./arch/arm/dts/Makefile
./arch/arm/dts/apn-806-z1.dtsi
./arch/arm/dts/armada-quartz-emu800.dts
./arch/arm/dts/armada-quartz-pd.dts
./arch/arm/dts/armada-quartz-toc.dts
./arch/arm/include/asm/arch-mvebu/gpio.h
./arch/arm/include/asm/global_data.h
./arch/arm/include/asm/system.h
./arch/arm/lib/board.c
./arch/arm/lib/crt0_64.S
./arch/arm/lib/reset.c
./board/mvebu/armada8k/km/machine_setup.c
./board/mvebu/armada8k/km/mtd_flash_access.c
./board/mvebu/armada8k/km/mtd_flash_access.h
./board/mvebu/armada8k/km/userdata.c
./board/mvebu/common/init.c
./common/autoboot.c
./common/board_f.c
./common/board_r.c
./common/bootm.c
./common/cmd_nvme.c
./common/cmd_usb.c
./common/cmd_warp.c
./common/command.c
./common/console.c
./common/fdt_support.c
./common/lcd.c
./common/main.c
./common/stdio.c
./common/usb.c
./common/usb_hub.c
./common/usb_storage.c
./disk/part.c
./drivers/block/nvme.c
./drivers/gpio/mvqz_gpio_cmds.c
./drivers/i2c/designware_i2c.c
./drivers/mmc/mmc.c
./drivers/pci/pci.c
./drivers/pci/pcie-mv-msb0125.c
./drivers/pci/pcie_dw.c
./drivers/scpi/arm_scpi.c
./drivers/serial/ns16550.c
./drivers/serial/serial_ns16550.c
./drivers/spi/Makefile
./drivers/spi/qspi_winbond.c
./drivers/video/Makefile
./drivers/video/lcd_lvds_pll.c
./drivers/video/lxk_armada_dumb.c
./drivers/video/lxk_vga_armada.c
./fs/fat/fat.c
./include/autoboot.h
./include/configs/armada8k.h
./include/configs/mvebu-common.h
./include/fdtdec.h
./include/linux/nvme.h
./include/lxk_panel.h
./include/mmc.h
./include/part.h
./include/pci.h
./include/usb_defs.h
./include/warp.h
./lib/fdtdec.c

51

Bash Find warp driver location and size of main KM files

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && grep -rl "warp_boot" --include=*.c --include=*.h . | head; echo ---; grep -rlE "mtd_write_protection|mtd_read_id_with_size" --include=*.c --include=*.h . | head; echo ---; ls drivers | tr '\n' ' ' | head -c 1500; echo; ls drivers/warp 2>/dev/null; echo ---; grep -rlE "Sky|ADS|Ads���|K\.Morikawa|T\.Watanabe" --include=*.c --include=*.h --include=*.S . 2>/dev/null | wc -l; wc -l board/mvebu/armada8k/km/*.c common/main.c common/cmd_warp.c drivers/block/nvme.c drivers/spi/qspi_winbond.c drivers/scpi/arm_scpi.c drivers/sound/km-*.c 2>/dev/null | tail -30

OUT

./common/cmd_warp.c
./common/main.c
./include/warp.h
---
./board/mvebu/armada8k/km/mtd_flash_access.c
./drivers/mtd/mtdcore.c
./include/linux/mtd/mtd.h
---
Kconfig Makefile bios_emulator block bootcount core crypto ddr demo dfu dma efuse fpga gpio hwmon i2c input memory misc mmc mtd net pci pcmcia phy power pwm qe rtc scpi serial soc sound spi thermal tpm twserial usb video watchdog 
---
463
    39 board/mvebu/armada8k/km/ca72wdt.c
   171 board/mvebu/armada8k/km/dipvalue.c
   297 board/mvebu/armada8k/km/graphlib.c
   221 board/mvebu/armada8k/km/init_debug.c
   819 board/mvebu/armada8k/km/machine_setup.c
   338 board/mvebu/armada8k/km/mfp_config.c
  1237 board/mvebu/armada8k/km/mfp_panel.c
  1778 board/mvebu/armada8k/km/mtd_flash_access.c
  3673 board/mvebu/armada8k/km/picture.c
   128 board/mvebu/armada8k/km/pngfilter.c
   388 board/mvebu/armada8k/km/userdata.c
  2788 common/main.c
  1040 common/cmd_warp.c
  4085 drivers/block/nvme.c
  1626 drivers/spi/qspi_winbond.c
   290 drivers/scpi/arm_scpi.c
   222 drivers/sound/km-cdma.c
   202 drivers/sound/km-i2s.c
   590 drivers/sound/km-rt5640.c
   253 drivers/sound/km-sound.c
 20185 total

Grep "KM_BIZHUB" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {fs/fat/fat.c,disk/part.c,drivers/pci/pci.c,drivers/pci/pcie_dw.c,drivers/mmc/mmc.c,common/console.c,common/stdio.c,arch/arm/lib/board.c,arch/arm/lib/crt0_64.S,arch/arm/include/asm/global_data.h,arch/arm/include/asm/system.h,arch/arm/cpu/armv8/armada8k/clock.c,drivers/serial/serial_ns16550.c,drivers/serial/ns16550.c,common/fdt_support.c,lib/fdtdec.c,common/lcd.c,common/cmd_usb.c,common/usb_hub.c,board/mvebu/common/init.c,common/board_f.c,include/mmc.h,include/part.h,include/pci.h,include/fdtdec.h})

151 lines of output

# PHẦN 30 (PART 31): Custom / Vendor Code

## 0. Kết luận

Đây là U-Boot 2015.01 của Marvell (MVEBU, `[VENDOR:Marvell]`) cộng thêm lớp Konica Minolta (`[VENDOR:KM]`). Lớp KM gồm **51 file `.c/.h/.S`** mang cờ `KM_BIZHUB`/`CONFIG_KM_BIZHUB` (đếm bằng grep), cộng các file thuần KM không cần cờ (thư mục `km/`, driver `km-*`, `cmd_warp.c`, `nvme.c`, `qspi_winbond.c`, `mvqz_gpio*`, `lxk_*`, `phy-*-mv`, `pcie-mv-msb0125`). Phép so sánh "upstream vs vendor" dưới đây dựa trên sự khác biệt so với U-Boot 2015.01 phổ biến mà tôi biết, không có bản gốc để diff, nên cột "Upstream" là nhận định [S] trừ khi nói rõ.

Số dòng để ước lượng quy mô: `common/main.c` 2788 (upstream ~100), `drivers/block/nvme.c` 4085, `board/.../km/mtd_flash_access.c` 1778, `mfp_panel.c` 1237, `picture.c` 3673, `qspi_winbond.c` 1626, `cmd_warp.c` 1040, `machine_setup.c` 819.

## 1. Bảng chính

Ký hiệu: **U** = upstream U-Boot; **M** = phần bổ sung Marvell; **K** = KM.

### 1.1 Lõi khởi động và vòng chính

|File|Hàm/khối chính|Upstream (U)|Sửa đổi vendor|Lý do|Ảnh hưởng runtime|
|---|---|---|---|---|---|
|`common/main.c`|`main_loop`, `check_autoboot`, `make_bootcmds`, `make_bootargs`, `check_action`, `do_doboot`, `set_boot_type_to_main_routine`, `setStartupBypeNomalWarp`, `getNWDLExec` `getMSCDownloading`|U: `main_loop` ~30 dòng gọi `bootdelay_process`, `autoboot_command`, `cli_loop`|**[VENDOR:K]** Viết lại thành bộ chọn kiểu boot (`g_action`), Warp, IISW, USB, line card, Init/Bderase card, `chk_sboot`, ghi `bootcmd/bootargs`|Một firmware phục vụ nhiều kịch bản (boot thường, USB update, IISW, Warp resume, chẩn đoán)|Quyết định toàn bộ đường boot và mã điều khiển WDT (Phần 16, 22)|
|`common/autoboot.c`|`abortboot_normal`, `bootdelay_process(&autoboot)`, `autoboot_command`, `chkGmUpdate`, `chk_sboot`|U: đếm ngược + `run_command_list`|**[VENDOR:K]** WDT theo `g_action`, `isWarpBootMode()` ép delay 0, GM update, chốt chip bảo mật|An toàn/khôi phục|300 s WDT, phím ngắt, khoá tháo chip (Phần 21, 22)|
|`common/board_f.c`|các bước trước relocation|U|**[VENDOR:K]** Dàn dựng bộ đệm log, `reserve_lcd` VRAM, kích thước `gd`|Che log; framebuffer tĩnh|`gd` ≈ 0x418C8; VRAM 0x3FE10000 (Phần 3, 9)|
|`common/board_r.c`|`init_km`, `initr_wd_start/refresh`, `INIT_FUNC_WATCHDOG_RESET`, `_WAIT_FOREVER`, `initr_bspi`|U: chuỗi `init_sequence_r`|**[VENDOR:K]** Chèn 16 điểm WDT, `init_km` (DIMM, UART panel, HW-check), `initr_bspi`, cờ `uc_HWcheckModeBoot`|Phát hiện treo lúc khởi động; tự chẩn đoán|Phần 22, 29|
|`arch/arm/lib/crt0_64.S`, `arch/arm/include/asm/global_data.h`, `arch/arm/lib/board.c`|khởi tạo `gd`, thêm trường KM (`printBuffer`, `termFlg`, `term_sel`, `chk_ram_Size`)|U: `gd_t` nhỏ|**[VENDOR:K]** Mở rộng `gd_t`|Bộ đệm log 255×1051|SP đặt 0xFF1000, `GD_SIZE` lớn (Phần 3)|
|`common/console.c`, `common/stdio.c`|`serial_printf`, `putc`, `puts`, `BspPrintStopCheck/Valid/BufferFlush`, `termFlg`|U: in thẳng ra thiết bị|**[VENDOR:K]** Đệm log tới khi `termFlg` (từ DIP/GPIO) cho phép rồi flush **một lần**|Máy sản xuất không phát log UART|Không thấy log/exception (Phần 9, 28)|
|`include/autoboot.h`, `arch/arm/lib/reset.c`|`reset_with_recoverycode`|U: `do_reset`|**[VENDOR:K]** Reset ghi mã 0x01–0x0c + bộ đếm ≤5 + tắt SuperWarp|Tránh "màn hình lỗi mãi sau bật nguồn" (OP_BTS-12208)|Phần 22|

### 1.2 Lõi SoC / CPU (Marvell + KM)

|File|Nội dung|U|Vendor|Runtime|
|---|---|---|---|---|
|`arch/arm/cpu/armv8/{start.S,transition.S,exceptions.S}`|reset vector, vector table|U (Aarch64 generic)|**[UPSTREAM]** gần nguyên; `start.S` có nhánh MVEBU SPL|Phần 3, 28|
|`arch/arm/cpu/armv8/cache_v8.c`, `cache.S`|`dcache_disable` đảo thứ tự; `flush_dcache_all` + `__asm_flush_l3_cache`|U: flush sau khi tắt; không có LLC|**[VENDOR:K]** Flush LLC bằng thanh ghi `0xF00087FC/0xF0008700`|Đồng bộ L3 khi tắt cache|
|`arch/arm/cpu/armv8/armada8k/{soc.c,clock.c,cache_llc.c,mvebu_flc.c,spl.c}`|`dram_init_banksize`, `reset_cpu`, LLC|**[VENDOR:M]** SoC Armada 8K|**[VENDOR:K]** `dram_init_banksize` giải mã MC thật, `reset_cpu` thêm `HRESET_REQ` GPIO 66|Xác định 3 bank DRAM và reset có cấp lại nguồn|
|`arch/arm/cpu/mvebu-common/*`|`fdt.c`, `system_info.c`, `generic_timer.c`, `misc.c`|**[VENDOR:M]**||Phần 7|
|`board/mvebu/common/init.c`|`board_init`|**[VENDOR:M]**|**[VENDOR:K]** hook KM|Phần 6|
|`board/mvebu/armada8k/{armada8k.c,quartz.c,devel-board.c}`|board init, lệnh `quartz`, devel|**[BOARD:M]**|**[BOARD:K]** `quartz_late_init` (đặt `dtb_name`)|Phần 6|

### 1.3 Thư mục `board/mvebu/armada8k/km/` (mã thuần KM)

|File (dòng)|Nội dung|Ảnh hưởng runtime|
|---|---|---|
|`machine_setup.c` (819)|`misc_init_r`, `setup_MotionEnable_Power`, `misc_init_r_env` (MAC/serial/machine), `fsload_hash_check`, `boot_diag`, `sysBspSecureChip`|Bước "sau khởi tạo": ROM, panel, USB, chẩn đoán, MotionEnable|
|`mfp_panel.c` (1237)|UART panel (COM2 57600 8E1), khung LED `0xFE…0xFF`, `init_Panel`, VRAM, `DrawPanel`, `hwcDisp*`, lệnh `draw`|Màn hình và LED lỗi|
|`mtd_flash_access.c/.h` (1778)|Truy cập SPI-flash theo vùng, hai bản A/B + bộ đếm, `THRG_*`, vùng secure, GM update|Dữ liệu bền của KM (Phần 12, 21)|
|`dipvalue.c` (171)|SoftDIP, `isWarpBootMode`, `getTermState`, lệnh `db_sw`|Chọn console/Warp|
|`mfp_config.c` (338)|Cấu hình máy, lệnh `MACHINE`||
|`userdata.c` (388)|NVRAM, `C`/`CB`/`setInitJpeg`, khởi tạo card|Xoá dữ liệu|
|`init_debug.c` (221)|`initdebugdata`|Gỡ lỗi|
|`graphlib.c`, `pngfilter.c`, `picture.c` (3673)|Giải nén PNG/LZO, dữ liệu ảnh khởi động|Ảnh màn hình khởi động; lỗi → `reset_with_recoverycode(0x07–0x0a)`|
|`ca72wdt.c` (39)|`km_scpi_set_wdt_ca72_ext`|WDT (Phần 22)|

### 1.4 Driver: từ upstream/Marvell/KM

|File|Loại|Ghi chú|
|---|---|---|
|`drivers/block/nvme.c` (4085), `common/cmd_nvme.c`, `include/linux/nvme.h`|**[DRIVER:K]** — U-Boot 2015.01 không có NVMe; đây là driver KM (phần OPAL "ported 2019/11/11")|Polling, whitelist, tắt D-cache, OPAL discovery (Phần 12, 21)|
|`drivers/spi/qspi_winbond.c` (1626)|**[DRIVER:K]** (biến thể `qspi.c` của Marvell), lệnh `bspi`, `pause_r4`, ghi LPDDR4 training|KM build `qspi_winbond.o` thay `qspi.o` (Phần 12)|
|`drivers/gpio/mvqz_gpio*.c`|**[DRIVER:K]** GPIO Quartz 272 chân, lệnh `qzgpio`|Phần 11|
|`drivers/scpi/arm_scpi.c`|**[DRIVER:M→K]** Marvell SCPI/MHU; KM thêm timeout, giá trị trả `bool`, `wait_for_LPPPintoWFI`|Phần 22|
|`drivers/i2c/designware_i2c.c`|**[UPSTREAM DW]** + **[K]** chuỗi PS-CPU/RTC (`IC_STOP`, 25 µs)|Phần 22|
|`drivers/mmc/{xenon_mmc.c,mmc.c}`|**[DRIVER:M]** Xenon + KM chỉnh `mmc.c`|Phần 12|
|`drivers/pci/{pcie-mv-msb0125.c,pcie_dw.c,pci.c}`|**[DRIVER:K]** RC MSB0125; KM_PCIE_SET_MPS|Phần 12|
|`drivers/usb/host/xhci-mvebu.c`, `drivers/usb/phy/phy-utm-mv.c`, `phy-usb3-mv.c`|**[DRIVER:M]** + **[K]** UTM PHY cố định, bỏ USB3 PHY|Phần 20|
|`common/usb.c`, `usb_hub.c`, `usb_storage.c`, `cmd_usb.c`|**[UPSTREAM]** + **[K]** (`usb_init` ghi PS-CPU; `MAX_TRIES 25` thay 5; `check_usb_port`; `do_usb`/`usb_stor_curr_dev` xuất công khai; `dev->route`)|Phần 20|
|`drivers/sound/km-*.c` (i2s, cdma, rt5640, sound)|**[DRIVER:K]** Âm thanh khởi động (`KM_I2S_CDMA`)|Phần 24|
|`drivers/video/{lxk_armada_dumb.c,lxk_vga_armada.c,lcd_lvds_pll.c}`, `include/lxk_panel.h`, `common/lcd.c`|**[DRIVER:M-Lexmark? K]** LCD/LVDS; `lcd.c` VRAM tối đa `KM_PANEL_MAX_*`, `lcd_test`|Tên `lxk_*` gợi ý mã gốc của Lexmark (Marvell/Lexmark "pegmatite"); chưa xác minh [?]|
|`drivers/serial/{serial_ns16550.c,ns16550.c}`|**[UPSTREAM]** + **[K]** thêm UART CSRC/panel, chọn console theo `term_sel`|Phần 9|
|`drivers/mtd/mtdcore.c`|**[UPSTREAM]** + **[K]** `mtd_write_protection`, `mtd_read_id_with_size`|GM update|

### 1.5 Lớp trên (lệnh, image, FS)

|File|Sửa đổi vendor|Ảnh hưởng|
|---|---|---|
|`common/command.c`|**[K]** thêm lệnh `X x Y y Z z` (MAC/serial)|Phần 19|
|`common/cmd_warp.c` (1040) + `include/warp.h`|**[DRIVER:K]** Hibernate resume (`warp_boot`, `warp_clear_bootf`, lệnh `warp`, `clear_bootf`), driver lưu ở SPI `0x2B0000`|Warp/SuperWarp (Phần 31, 34)|
|`common/bootm.c`, `common/cmd_bootm.c`|**[UPSTREAM]** + **[K]** `hwcDispLoadDiag`; `booti` (Marvell/Linaro backport)|Phần 16|
|`common/fdt_support.c`, `lib/fdtdec.c`, `include/fdtdec.h`|**[K]** thuộc tính `/memory` (hddinfo, boottype, panel…, vram*), compat ID mới|Phần 8|
|`common/mvebu/*`|**[M]** lệnh `map`, `mss_boot`…|Phần 15|
|`fs/fat/fat.c`, `disk/part.c`, `include/part.h`, `include/mmc.h`|**[K]** chỉnh nhỏ (đọc FAT theo buffer căn, phân vùng hex `0:A`…)|Phần 13|
|`include/configs/{armada8k,mvebu-common,quartz-sb}.h`|**[CONFIG:M+K]**|Phần 26|

### 1.6 Cấu hình / DT / build

|File|Ghi chú|
|---|---|
|`Kconfig` (menu KonicaMinolta), `config.mk`, `Makefile:650`|Công tắc `KM_BIZHUB`|
|`configs/km_mvebu_quartz_toc_defconfig` (+ `_pd`, `emu800_*`, `egl*`, `spam*`, …)|**[CONFIG:K]** 20+ defconfig KM|
|`arch/arm/dts/{armada-quartz-emu800.dts,armada-quartz-toc.dts,armada-quartz-pd.dts,apn-806-z1.dtsi,quartz-sb.dtsi,quartz-iris-sb-*.dtsi}`|**[BOARD:M+K]**|
|`arch/arm/include/asm/arch-mvebu/gpio.h`, `system.h`|**[K]** ID GPIO (FW_CONFIG, HRESET_REQ…), `flush_l3`|
|`build_uboot.sh`, `tools/marvell/dtc`|**[K/M]** script build và dtc đi kèm|

## 2. Bản đồ "ai sở hữu gì"

```
[UPSTREAM U-Boot 2015.01]   cli_hush, env, image/bootm, FAT/EXT, MMC core, USB core, xHCI core, NS16550, DW-I2C, DT (libfdt), start.S/exceptions.S
[VENDOR Marvell MVEBU]      arch/.../armada8k, mvebu-common, board/mvebu, drivers/{scpi,phy,pci dw...}, xenon_mmc, sar, mss_boot, cmd mvebu
[VENDOR KM]                 main/autoboot/board_r/board_f hooks, console buffering, km/ (panel, flash access, DIP, picture), nvme, qspi_winbond, mvqz_gpio, warp,
                            km-sound, PHY USB2 tune, PS-CPU I2C, lệnh X/x/Y/y/Z/z, GM update, chk_sboot, lệnh doboot
```

## 3. Điểm đáng lưu ý cho người bảo trì

1. **`common/main.c` là "God file"** (2788 dòng) trộn: chọn boot, Warp, USB, SoftDIP, PCI info cho Warp, tar/INDEX USB, khởi tạo card. Sửa nhỏ ở đây dễ phá đường boot.
2. **Nhiều bản sao hằng số** (`PSCPU_SLAVE_ADDR` ở 4 file; `DEF_KM_WARP_*`).
3. **Nhiều lỗi ngầm** đã ghi ở các phần trước (Phần 19, 21, 22, 24): `y`/`z` NULL, `fsload_hash_check` ghi ngoài, mảng 3 MiB trên stack, `send_cmd` 256B vào kênh 128B, `flush_dcache_range` sai đối số.
4. **Marker phiên bản trong comment** ("2017/09/26 Sky…", 463 file có tên người/nhóm) cho phép lần theo lịch sử; không có VCS.
5. **Cờ kép**: `KM_BIZHUB` (`-D`) và `CONFIG_KM_BIZHUB` (Kconfig) dùng lẫn.
6. **Phụ thuộc phần cứng KM**: PS-CPU (I2C 0x3E), R4 (SCPI), panel UART; U-Boot không chạy đúng trên board không có chúng (dev board: `DEVEL_BOARD`).

## 4. Chưa xác minh

- Bản gốc để diff (Marvell "u-boot-armada" 2015 và U-Boot 2015.01) nên cột "Upstream" là suy luận.
- Nguồn gốc `lxk_*` (Lexmark?).
- Danh sách 463 file có comment lịch sử chưa được liệt kê.

---

Xong Phần 30. Tiếp theo **Phần 31: OTA / Firmware update**: USB update, IISW, Warp snapshot, GM update, SPI-flash map, đường cập nhật U-Boot.

Bash Read warp.h

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && sed -n 1,140p include/warp.h

OUT

/*
 * Warp!! common define  Rev. 5.1.0
 *
 */

#define WARP_HEADER_ID          0x00
#define WARP_HEADER_VERSION     0x04
#define WARP_HEADER_CAPS        0x08
#define WARP_HEADER_LOWMEM_END  0x0c
#define WARP_HEADER_TEXT_END    0x10
#define WARP_HEADER_DATA_END    0x14
#define WARP_HEADER_BSS_END     0x18
#define WARP_HEADER_SNAPSHOT    0x20
#define WARP_HEADER_HIBERNATE   0x28
#define WARP_HEADER_SWITCH      0x30

#define WARP_ID_DRIVER          0x44483557      /* W5HD */
#define WARP_ID_BOOTFLAG        0x46423557      /* W5BF */
#define WARP_ID_USER_API        0x41553357      /* W3UA */

#define WARP_PART_SHIFT         0
#define WARP_LUN_SHIFT          8
#define WARP_DEV_SHIFT          16

#define WARP_PART_MASK          (0xff << WARP_PART_SHIFT)
#define WARP_LUN_MASK           (0xff << WARP_LUN_SHIFT)
#define WARP_DEV_MASK           (0xff << WARP_DEV_SHIFT)

#define WARP_DEV_NOR            (0x01 << WARP_DEV_SHIFT)
#define WARP_DEV_NAND           (0x02 << WARP_DEV_SHIFT)
#define WARP_DEV_ATA            (0x03 << WARP_DEV_SHIFT)
#define WARP_DEV_SD             (0x04 << WARP_DEV_SHIFT)
#define WARP_DEV_MEM            (0x05 << WARP_DEV_SHIFT)
#define WARP_DEV_SPI            (0x06 << WARP_DEV_SHIFT)
#define WARP_DEV_NVME           (0x07 << WARP_DEV_SHIFT)
#define WARP_DEV_USER           (0x7e << WARP_DEV_SHIFT)
#define WARP_DEV_EXT            (0x7f << WARP_DEV_SHIFT)

#define WARP_DEV(dev, lun, part)        (WARP_DEV_##dev | \
                                         ((lun) << WARP_LUN_SHIFT) | \
                                         ((part) << WARP_PART_SHIFT))

#ifndef WARP_LUN_CONV
#define WARP_LUN_CONV(dev)      (dev)
#endif

#define WARP_DEV_TO_LUN(dev)    WARP_LUN_CONV(((dev) & WARP_LUN_MASK) >> \
                                              WARP_LUN_SHIFT)
#define WARP_DEV_TO_PART(dev)   (((dev) & WARP_PART_MASK) >> WARP_PART_SHIFT)

#define WARP_DRV_INVALID        0
#define WARP_DRV_FIXED          1
#define WARP_DRV_FLOATING       2

#define WARP_BF_LEN             0x400

#define WARP_SAVEAREA_NUM       (sizeof(warp_savearea) / \
                                 sizeof(struct warp_savearea))


struct warp_boot {
    u16         cpu_no;
    s16         switch_mode;
    s16         silent;
    s16         console;
    s32         bps;
    u32         drv_total_size;
    u64         v2p_offset;
    u64         text_v2p_offset;
    u32         lowmem_maxarea;
    u32         lowmem_maxsize;
    u64         bg;
    u64         outer_cache_base;
    u32         reserve[32];
};

typedef struct warp_area {
    u32         dev;
    u32         size;
    u64         offs;
} warp_area;

struct warp_drv_info {
    u32 mode;
    unsigned long addr;
    warp_area drv;
};

struct warp_savearea {
    warp_area bootflag;
};

#ifdef CONFIG_WARP_HIBDRV_BG

struct warp_bg {
    int ret_code;
    int boot_ready;
    u64 boot_addr;
    u64 data_addr;
};

void boot_cpu(int cpu);
void kill_cpu(int cpu);

int warp_pre_boot_bg(int saveno);
void warp_wait_boot_bg(void);

#endif

extern const struct warp_savearea warp_savearea[];

extern int warp_saveno;

extern void *warp_bfaddr;

/* 2019.01.23 sky_hatano �V���A��������������ʏ�N��������Ή�. */
#if defined(KM_BIZHUB)
void km_warp_init_device(void);
#endif	/* defined(KM_BIZHUB) */
int warp_drvload(void);
int warp_checkboot(int saveno);
int warp_boot(int saveno);
int warp_clear_bootf(int saveno);

#define WARP_DRV_ID(drv)        (*(u32 *)((void *)(drv) + WARP_HEADER_ID))
#define WARP_DRV_VERSION(drv)   (*(u32 *)((void *)(drv) + WARP_HEADER_VERSION))
#define WARP_DRV_CAPS(drv)      (*(u32 *)((void *)(drv) + WARP_HEADER_CAPS))
#define WARP_DRV_LOWMEM_END(drv) \
    (*(u32 *)((void *)(drv) + WARP_HEADER_LOWMEM_END))
#define WARP_DRV_TEXT_END(drv)  (*(u32 *)((void *)(drv) + WARP_HEADER_TEXT_END))
#define WARP_DRV_DATA_END(drv)  (*(u32 *)((void *)(drv) + WARP_HEADER_DATA_END))
#define WARP_DRV_BSS_END(drv)   (*(u32 *)((void *)(drv) + WARP_HEADER_BSS_END))

Grep "^(static |int |void |bool |ulong |u32 |unsigned)[^;]*\(|^U_BOOT_CMD|warp_drvload|WARP_DRV|jump|kernel_entry|warp_prepare_boot|hibernat|snapshot|bootflag|crc|checksum" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\cmd_warp.c)

81 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\cmd_warp.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 561-700)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\cmd_warp.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 751-1040)

# PHẦN 31 (PART 32): OTA / Firmware Update

## 0. Kết luận

**U-Boot này không tự tải hay ghi firmware ứng dụng.** Nó chỉ **chọn phần mềm nào để khởi động** và **thực hiện vài thao tác SPI-flash nhỏ** (sao lưu GM, xoá vùng). Việc cập nhật thật do một hệ Linux "cập nhật" (Subset/IISW) chạy từ USB memory hoặc từ phân vùng SSD làm. Không có `tftp`, `bubt`, `dfu`, `fastboot`, `sf write` (mạng và các lệnh này không được build). Cập nhật qua mạng (NWDL) là việc của phần mềm chính trên Linux, rồi **gài cờ trong SPI-flash** và khởi động lại để U-Boot chọn đường cập nhật.

## 1. WHAT / WHY / WHERE

|||
|---|---|
|WHAT|Các đường cập nhật/khôi phục firmware và dữ liệu bền|
|WHY|Cập nhật tại hiện trường (USB), qua mạng (NWDL), khôi phục sau lỗi, sản xuất|
|WHERE|`common/main.c` (`check_action`, `check_autoboot`, `getNWDLExec`, `getMSCDownloading`), `armada8k.h:232-284` (mẫu `bootcmd`), `autoboot.c` (`chkGmUpdate`), `mtd_flash_access.c`, `userdata.c`, `cmd_warp.c`|

## 2. Các đường cập nhật (theo `g_action`)

|`g_action`|Kích hoạt|Phần mềm được khởi động|Nơi lưu|Vai trò|
|---|---|---|---|---|
|`USB_BOOT` (1)|USB memory, `FW0000/INDEX` loại "USB_ZZZ"|Linux từ USB (`usb 0`, `root=/dev/sdc3`)|USB|Chạy phần mềm từ USB|
|`USB_UPDATE` (2)|INDEX loại "USB_DOWN", đúng `TYP_NAME`|Bộ cập nhật (Subset) từ USB, thư mục `FW0000/`|USB|**Cập nhật SSD từ USB**|
|`DEF_IISW` (3)|SoftDIP `0x06` = 5 hoặc 6 (`getNWDLExec`)|Linux từ **phân vùng 0:2** (`kmsda2`)|SSD/SD|IISW download boot (tải xuống cập nhật)|
|`DEF_UPDATE` (9)|SoftDIP `0x06` = 4, 7, 8, 9|Linux từ **phân vùng 0:A (10)**, sau khi kiểm `INDEX` trên 0:A|SSD/SD|IISW update boot|
|`DEF_BACKUP` (11)|SoftDIP = 10|0:2 (như IISW)||Sao lưu dữ liệu|
|`DEF_RESTORE` (12)|SoftDIP = 11|0:2||Khôi phục|
|`BOOT_ERR` (5)|INDEX sai loại máy, thiếu INDEX (khi update), hoặc cờ MSC-download-lỗi|0:2 (`ERRCMD`)||Khởi động lỗi có kiểm soát|
|`BOOT_LINE` (4)|USB có tar "line card" (`@TYP=…_12`/…)|Boot thường (0:5) nhưng ép console||Dây chuyền sản xuất|
|`BOOT_INIT` (6)|USB tar `@TYP=GaiaPF` + định danh initialize|**Không boot**: `initialize_card_main()` xoá SPI-flash vùng người dùng rồi treo|SPI-flash|Xoá dữ liệu người dùng ("Initialize card")|
|`BOOT_BDERASE` (13)|USB tar `@TYP=GaiaPF` + định danh board-erase|**Không boot**: `bderase_card_main()`|SPI-flash|Xoá board ("Board Erase card")|
|`DEF_BOOT` (0)|mặc định|0:5 (kernel + DTB + initrd)|SSD/SD|Boot thường|

Ánh xạ phân vùng (theo `fsl_table`, machine_setup.c:452): `0:2 = /dev/kmsda2` (SUB.img: firmware phụ/Subset), `0:5 = kmsda5` (Image, emu800.dtb, irfs.ubt), `0:7 = kmsda7` (RFS1.img, RFS3.img, root của kernel), `0:A = kmsda10` (khu vực cập nhật IISW), phân vùng 3 và 10 cũng chứa **cờ Warp** (mục 4).

### Trình tự điển hình của cập nhật qua mạng (NWDL)

```
[Linux, boot thường]  ứng dụng tải gói về SSD (phân vùng 2/10)  → cập nhật cờ SPI/SoftDIP 0x06 (=5..9)  → reboot
[U-Boot]  check_action(): USB không có → đọc SoftDIP 0x06
          → g_action = DEF_IISW / DEF_UPDATE
          → check_autoboot: bootcmd = "nvme init; fatload nvme 0:2 (hoặc 0:A) … ; booti"
[Linux bộ cập nhật (0:2/0:A)]  ghi kernel/RFS mới vào 0:5, 0:7 ; SHA-256 vào EXP_INDEX ; xoá cờ ; reboot
[U-Boot]  DEF_BOOT ; (trước đó) getMSCDownloading()/getMSCSubsetDownloading() nếu cờ dở → AUTOBOOT_SSD_SUBSET (0:2) hoặc NOTHING_BOOT_IMAGE
```

`WDT` cho các chế độ này: 120 s (`0x11E0/0x11E1/0x11E2`) (Phần 22). Trong chế độ IISW/UPDATE/BACKUP/RESTORE, `setup_MotionEnable_Power` còn tắt/nhả nguồn rơ-le M/C (`mdelay(4500)`) và **không** bật MotionEnable (Phần 20).

### Cờ và vùng lưu trong SPI-flash liên quan

|Vùng (theo offset SPI, mã KM)|Nội dung|
|---|---|
|`0x06E000…` (NVRAM/SoftDIP, hai bản A/B)|SoftDIP: offset `0x06` (`getNWDLExec`), `0x413` (`getInergrity`), `0xA1`, `0xC2` (chẩn đoán)…|
|`0x077000`/`0x078000` (BSP/Thread, A/B)|Cờ restart `0x0000`, mã recovery `0x2E`, "SuperWarp disable" `0x20`, event…|
|`0x079000`/`0x07A000` (machineinfo, A/B)|MAC, serial, kiểu máy|
|`0x07B000 (+DEF_USERDATA_OFFSET)`|`getMSCDownloading` (+0), `getMSCSubsetDownloading` (+5): tải MSC dở|
|`0x2B0000`|Driver hibernate Warp!! (ảnh nhị phân, ID `W5HD`)|
|`0x380000` (cửa sổ `0xF4380000`)|Kết quả huấn luyện LPDDR4 (Phần 12)|
|`0x3FD000`|5 chữ ký × 256 B (KERNEL, DTB, RFS1, RFS3, IRFS), **không dùng** trong U-Boot|
|`0x3FE000`|Vùng secure info (`RECOVERY 0`, `VALIDERR 0x10`, `GMBOOT 0x18`, `GMLPPP 0x20`, `SBOOT 0x28`)|
|`0x000000…0x2FFFFF`|Primary Boot (3 MiB: ATF + U-Boot)|
|`0x400000…`|GoldenMaster Boot (bản sao của Primary)|

## 3. Cập nhật bootloader (Primary/GM) và R4

- **Primary Boot** (ATF + U-Boot, offset 0, 3 MiB) **không** có đường cập nhật trong U-Boot (không `bubt`, không `sf`). Việc ghi do Linux (qua MTD) thực hiện.
- **GoldenMaster**: sau khi Linux đặt `THRD_SECAREA_GMBOOT = 1` (hoặc `GMLPPP`), lần **boot thường kế tiếp** `chkGmUpdate()` (autoboot.c:399) gọi `THRG_updateGmBoot()`: bỏ khoá ghi (`mtd_write_protection(flash,false)`), xoá 3 MiB tại `0x400000`, ghi bản chép của Primary, **đọc lại và `memcmp`**, khoá lại; thành công thì xoá cờ. Nếu lỗi: in `GM update failed.` và vẫn boot. Chỉ chạy khi `g_action == DEF_BOOT`. Rủi ro: hai mảng 3 MiB trên stack (Phần 21). GM chỉ được ghi **sau khi** Primary đã chạy ổn (đủ tin cậy để khởi động tới U-Boot), nên GM luôn là bản "đã từng khởi động được".
- **LPPP/R4 firmware** (`THRG_updateGmLPPP`): tương tự cho firmware R4; offset LPPP chưa đọc [?].
- Sao lưu **kết quả huấn luyện LPDDR4** vào SPI (`qspi_write_lpddr4_training_result`, Phần 12) phục vụ Warp: cần giống nhau giữa lần lưu snapshot và lần resume.

## 4. Warp!! (hibernate resume): "cập nhật" bộ nhớ chứ không phải firmware

Thư viện Warp!! Rev 5.1.0 của bên thứ ba, KM tích hợp trong `common/cmd_warp.c` (1040 dòng) + `include/warp.h`.

```
warp_boot(saveno)  → warp_boot_fg:
   warp_pre_boot(saveno):
      warp_checkboot(saveno):
         _warp_drvload(0)         tải driver hibernate từ SPI 0x2B0000 (WARP_DEV(SPI,0,0), mode FLOATING)
                                  vào CONFIG_WARP_LOAD_ADDR = 0x800000 ; kiểm ID 0x44483557 ("W5HD") ; nạp DATA_END byte ; flush_cache
         warp_bfaddr = drvaddr + TEXT_END ; warp_bfload(saveno): đọc bootflag 0x400 B từ SD/NVMe (partition 3 hoặc 10, offset 0)
         phải bắt đầu bằng 0x46423557 ("W5BF")  → boot_param = bfaddr + 0x400
      memset(boot_param, 0, sizeof struct warp_boot) ; silent=console=bps=-1 ; drv_total_size
   warp_prepare_boot(): smp_kick_all_cpus(); flush_dcache_all(); *(0xF0630000) = 0x100
   hibernate = drvaddr + WARP_HEADER_HIBERNATE (0x28) ; ret = hibernate()      ← khôi phục bộ nhớ từ snapshot; KHÔNG return khi thành công
   "Warp!! error can not boot %d"
```

|saveno|Ý nghĩa|Thiết bị bootflag|
|---|---|---|
|0 `WARP_SSID_NORMAL_SD`|Warp thường (SD)|SD phân vùng 3|
|1 `WARP_SSID_NORMAL_SSD`|Warp thường (SSD)|NVMe phân vùng 3|
|2 `WARP_SSID_SUPER_SD`|SuperWarp (SD)|SD phân vùng 10|
|3 `WARP_SSID_SUPER_SSD`|SuperWarp (SSD)|NVMe phân vùng 10|

(`WARP_SAVEAREA` armada8k.h:326-343; mỗi `bootflag.size = 2`, `offs = 0`.)

Quy tắc dùng trong `main_loop` (Phần 16):

1. `check_autoboot` → `checkWarpBootMode()` (cờ Warp hợp lệ, khác DIP debug `isWarpBootMode`) → `AUTOBOOT_WARP` (4).
2. Trước resume: ghi kiểu boot vào sector Warp; nếu SSD thì `nvme_opal_cmd_send(0)`; nếu panelKey ≠ 0x16 và không có lỗi từng vô hiệu SuperWarp thì `warp_boot(SuperWarp)`; **nếu trả về** thì `warp_boot(Normal)`; nếu vẫn trả về thì tiếp tục boot thường.
3. **Sau mỗi lần vào đường boot thường** U-Boot gọi `warp_clear_bootf(Normal)` và (trừ `DEF_UPDATE`) `warp_clear_bootf(SuperWarp)`, tức **snapshot là dùng một lần**: bị vô hiệu hoá ngay khi U-Boot tới điểm boot lạnh, nên nếu lần resume thất bại giữa chừng thì lần sau boot thường (không lặp vô hạn hỏng).
4. `reset_with_recoverycode` cấm SuperWarp (offset 0x20 = `0x1010`); lệnh `z` (đổi serial) cũng xoá snapshot.
5. WDT: SuperWarp 40 s, Warp 90 s.

Phần tử "cập nhật" ở đây là Linux tạo snapshot khi tắt máy để lần sau khởi động nhanh; U-Boot chỉ tiêu thụ/xoá nó.

## 5. Xác thực và toàn vẹn khi cập nhật

- U-Boot **không xác thực chữ ký** gói cập nhật (Phần 21). Bảo vệ gồm: kiểu máy (`@TYP=`) trong INDEX, CRC32 initrd, SHA-256 qua `EXP_INDEX` (chỉ ở chẩn đoán tự động), cờ SoftDIP do phần mềm chính đặt, GM tách biệt.
- Tính toàn vẹn của cập nhật do bộ cập nhật (Linux) đảm nhiệm: ghi `EXP_INDEX` (hash) và cờ tải.

## 6. Failure / recovery của quá trình cập nhật

|Sự cố|Hành vi|
|---|---|
|Mất điện giữa lúc ghi 0:5/0:7|Cờ MSC-download còn dở (`0x7B000`) → lần boot sau `AUTOBOOT_SSD_SUBSET`: boot Subset (0:2) để làm lại, hoặc `NOTHING_BOOT_IMAGE` nếu cờ subset cũng dở|
|USB sai kiểu máy|`Error. This usb memory is not for <name>` → `BOOT_ERR`|
|Thiếu INDEX ở 0:A (UPDATE)|`Can not found INDEX file.` → `BOOT_ERR`|
|GM update lỗi|Bỏ qua, vẫn boot|
|Mất điện giữa `set_secureinfo_to_spi` (xoá+ghi 4 KB)|Mất cả cờ secure/GM (Phần 21)|
|Mất điện giữa `THRG_updateGmBoot` sau khi xoá GM|GM rỗng cho tới lần cập nhật kế; Primary vẫn nguyên|
|Kernel mới hỏng|`booti` thất bại hoặc kernel không nạp WDT → R4 reset sau 300 s; không có cơ chế tự "rollback" trong U-Boot ngoài cờ do bộ cập nhật quản lý [S]|

## 7. COMMON CONFUSION

1. **"U-Boot cập nhật firmware"**: không, nó khởi động bộ cập nhật.
2. **"OTA qua mạng tại U-Boot"**: không có mạng ở U-Boot.
3. **"GM là bản dự phòng khi Primary hỏng và U-Boot tự chuyển"**: việc chuyển sang GM khi Primary hỏng do BootROM/ATF (ngoài repo) [S]; U-Boot chỉ cập nhật GM.
4. **"Warp là cập nhật firmware"**: là snapshot hibernate để khởi động nhanh, dùng một lần.
5. **`0:5`/`0:2`/`0:7`/`0:A` là đĩa khác nhau**: cùng một ổ với các phân vùng khác nhau.

## 8. MENTAL MODEL

```
Linux (chính) ── tải gói ──► SSD p2/p10, đặt cờ SoftDIP 0x06 / MSC ──► reboot
U-Boot ── check_action ──► g_action ──► bootcmd(p2 | pA | USB) ──► Linux bộ cập nhật ──► ghi p5/p7, EXP_INDEX ──► reboot ──► DEF_BOOT
U-Boot (boot thường) ── chkGmUpdate ──► sao Primary → GM (có write-protect)
Warp: snapshot 1 lần (W5BF + driver W5HD tại SPI 0x2B0000) — clear sau khi dùng
```

## 9. Chưa xác minh

- Offset và quy trình `THRG_updateGmLPPP`.
- Cơ chế BootROM/ATF chuyển sang GM.
- Định dạng `INDEX`/`EXP_INDEX`/tar và giá trị `sysBspGetUsbMemType`.
- Cách bộ cập nhật (Linux) đặt/xoá các cờ.
- Driver Warp!! chạy ở EL và trạng thái MMU nào.

---

Xong Phần 31. Tiếp theo **Phần 32: Call graph**: tổng hợp đồ thị gọi hàm từ reset tới kernel, kèm các nhánh KM.

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\board_f.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 801-960)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\include\asm\global_data.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#)

# PHẦN 32 (PART 33): Call Graph

## 0. Ghi chú đọc

Mọi cạnh dưới đây đã được xác minh ở các phần trước (mã nguồn thật). Mỗi dòng có dạng `caller → callee` kèm tác dụng. Nhãn `[KM]` là hàm/khối do KM thêm; `[MRV]` là Marvell; còn lại là upstream. Đồ thị chia theo pha; sau đó là bảng "caller → function → callee → effect" cho các hàm quan trọng.

## 1. Pha 0: từ ATF tới `_main`

```
ATF BL31 ──(nhảy, x0..x3=0, BL33_BASE=0x1000)──► _start                          start.S:22
_start:  b reset
reset:   vbar_elX = vectors ; cptr/cpacr bật FP-SIMD
         bl apply_core_errata          (chỉ A57: cpuactlr_el1)
         bl __asm_flush_dcache_all ; __asm_invalidate_icache_all ; __asm_invalidate_tlb_all
         bl lowlevel_init              (thoát ngay nếu EL != 3)
         branch_if_master → master_cpu
master_cpu: bl _main                                                              crt0_64.S
_main:   sp = CONFIG_SYS_INIT_SP_ADDR (0xFF1000) ; x18 = gd (dưới sp, GD_SIZE ≈ 0x418C8 [KM]) ; malloc_base
         bl board_init_f(0)                                                       board_f.c
```

## 2. Pha 1: `board_init_f` (trước relocation, cache và MMU tắt)

`init_sequence_f[]` (board_f.c:828), thứ tự thực tế cho cấu hình này:

```
setup_mon_len          gd->mon_len = __bss_end - _start
setup_fdt              gd->fdt_blob = __dtb_dt_begin (DTB nhúng)
initf_malloc           malloc sớm 0x5000
arch_cpu_init          (MVEBU: soc init, CCU/RFU cửa sổ)                            [MRV]
mark_bootstage ; fdtdec_check_fdt
timer_init             bật system counter 0xF0581000
env_init               env_nowhere: gd->env_addr = default_environment ; env_valid = 0
init_baud_rate         getenv_f("baudrate") → 115200
serial_init            (ns16550: AP-UART 0xF0512000, CSRC 0xE8003000…)              [KM]
console_init_f         stage 1
fdtdec_prepare_fdt ; display_options ; display_text_info ; print_cpuinfo
init_func_i2c          i2c_init_all (DW-I2C ×4)      (CONFIG_SYS_I2C từ quartz-sb.h)
announce_dram_init ; dram_init      gd->ram_size = 0x80000000 (QUARTZ_RAM_SIZE)
setup_dest_addr        ram_top = min(ram,1GB)=0x40000000
reserve_mmu (TLB 64KB) ; reserve_lcd (VRAM 0x1E0000 [KM]) ; reserve_uboot ; reserve_malloc ; reserve_board ; reserve_global_data ; reserve_fdt ; reserve_stacks
setup_dram_config → dram_init_banksize [KM: giải mã MC, 3 bank]
setup_reloc            gd->reloc_off = relocaddr − 0x1000
jump_to_copy → relocate_code(relocaddr)
```

## 3. Pha 2: relocation

```
relocate_code (relocate_64.S)   chép [__image_copy_start, __image_copy_end) → relocaddr
                                fix .rela.dyn: *(off+x9) = addend + x9     (RELATIVE=1027)
                                flush D-cache nếu bật
→ c_runtime_cpu_setup           vbar_elX = vectors (đã relocate)
→ clear BSS [__bss_start, __bss_end)
→ board_init_r(gd, relocaddr)                                               board_r.c
```

## 4. Pha 3: `board_init_r`

`init_sequence_r[]` (board_r.c:915), các bước đáng chú ý theo thứ tự:

```
initr_wd_start [KM]        SCPI: START 0x1000, 5 s
initr_trace ; initr_reloc
initr_caches → enable_caches() = icache_enable + dcache_enable → mmu_setup     (Phần 23)
initr_reloc_global_data ; initr_barrier
initr_malloc               heap 5 MiB (TOTAL_MALLOC_LEN ≈ 0x510000)
bootstage_relocate
[WDT] board_init           (board/mvebu: set_cpu_info…; quartz/devel)            [MRV]
stdio_init_tables ; initr_serial ; initr_announce
[WDT] … power_init_board
[WDT] initr_mmc            mmc_initialize (Xenon SDHCI)                          [MRV]
[WDT] initr_bspi           bspi_initialize (QSPI winbond)                        [KM]
[WDT] init_km              init_Panel_Uart ; HW-check ; GPIO fan                 [KM]
initr_env                  env_relocate → set_default_env(NULL) → himport_r → ENV_READY
[WDT] initr_secondary_cpu ; initr_pci (pci_init: PCIe RC MSB0125, NVMe bus)
stdio_add_devices          gd->arch.termFlg = 1 [KM] ; i2c_init_all ; lcd/serial devices
initr_jumptable ; console_init_r   chọn stdin/out/err = serial đầu tiên
misc_init_r [KM]           → misc_init_r_env (ethaddr, MachineInfo, SerialNo qua read_rom)
                           → init_Panel (UART panel + VRAM, Utility Key GPIO 49)
                           → setup_MotionEnable_Power → [USB init nếu phím/cờ] → check_action → g_action
                           → hwcDispBootDeviceDiag ; boot_diag (fsload_hash_check nếu tự chẩn đoán)
[WDT] interrupt_init (stub) ; initr_enable_interrupts (stub)
board_late_init (quartz_late_init: dtb_name)                                      [KM]  (theo Phần 6)
run_main_loop → main_loop()                                                      (for(;;))
```

## 5. Pha 4: `main_loop` (KM) và boot

```
main_loop (main.c)
 ├ cli_init → u_boot_hush_start (biến HUSH_VERSION)
 ├ run_preboot_environment_command
 ├ g_action == BOOT_INIT      → km_scpi WDT END → initialize_card_main() (xoá SPI, treo)
 ├ g_action == BOOT_BDERASE   → km_scpi WDT END → bderase_card_main()      (treo)
 ├ chk_sboot()                sysBspSecureChip → get_secureinfo → (treo nếu tháo chip)
 ├ s = bootdelay_process(&autoboot)
 │      → check_autoboot() → make_bootcmds / make_bootargs → setenv("bootcmd","bootargs","initrd_high")
 │      → getenv("bootcmd")
 ├ autoboot ∈ {WARP(4), LINE(7)+warp}:
 │      set_boot_type_to_main_routine → sound_stop → nvme_opal_cmd_send(0) → warp_boot(Super) → warp_boot(Normal) → (quay lại) check_autoboot_DEF_BOOT_case
 ├ warp_clear_bootf(Normal) ; warp_clear_bootf(Super)  (trừ DEF_UPDATE)
 ├ set_boot_type_to_main_routine(autoboot) ; sound_stop
 ├ autoboot ∈ {NOT_AUTOBOOT, FOR_BOOTDEV} → in "Functional restriction"
 │   else autoboot_command(s):
 │        abortboot_normal (phím bất kỳ ngắt) → WDT theo g_action → chkGmUpdate (DEF_BOOT)
 │        run_command_list(s,-1,0) → parse_string_outer → hush → run_pipe_real → cmd_process → do_xxx
 ├ km_scpi WDT END
 └ cli_loop → parse_file_outer (không trở về)
```

## 6. Pha 5: chuỗi lệnh `bootcmd` và `booti`

```
hush(bootcmd)
   nvme init          do_nvme → init_nvme (PCIe scan, tắt D-cache+MMU)      [KM]
   fatload nvme 0:5 $kernel_addr $bootfile   do_fat_fsload → fat_read_file → fs_devread → nvme_read (polling PRP)
   fatload nvme 0:5 $fdt_addr    $dtb_name
   fatload nvme 0:5 $initrd_addr $initrd_name
   booti $kernel_addr $initrd_addr $fdt_addr
        do_booti → booti_start:
              do_bootm_states(START) → bootm_start (images.verify = -1 ; lmb)
              booti_setup (magic ; memmove → text_offset ; lmb_reserve)
              bootm_find_ramdisk_fdt → boot_get_ramdisk (uImage; image_check_dcrc) ; boot_get_fdt
           bootm_disable_interrupts → usb_stop
           do_bootm_states(OS_PREP|OS_FAKE_GO|OS_GO) → do_bootm_linux
              boot_prep_linux → image_setup_linux → boot_ramdisk_high (chép initrd) ; boot_relocate_fdt ; image_setup_libfdt
                                  → fdt_chosen ; arch_fixup_fdt [KM props] ; fdt_fixup_ethernet ; fdt_initrd
              boot_jump_linux → announce_and_cleanup → cleanup_before_linux (icache off, dcache_disable[flush+LLC], invalidate)
                              → do_nonsec_virt_switch (smp_kick_all_cpus, flush, armv8_switch_to_el2 no-op)
                              → kernel_entry(fdt, 0, 0, 0)
```

## 7. Bảng caller → function → callee → effect (hàm then chốt)

|Caller|Function|Callee chính|Effect|
|---|---|---|---|
|`_main`|`board_init_f`|`init_sequence_f[]`, `relocate_code`|Khởi tạo tối thiểu, tính bố cục, chuyển U-Boot lên RAM cao|
|`board_init_f`|`dram_init` [MRV/KM]|(hằng `QUARTZ_RAM_SIZE`)|`gd->ram_size = 2 GiB`; bank thật do `dram_init_banksize`|
|`setup_dram_config`|`dram_init_banksize` [KM]|đọc `0xF0020200 + ch*0x200 + cs*8`|Điền `bi_dram[0..2]`, `gd->chk_ram_Size`|
|`board_init_r`|`initr_env`|`env_relocate` → `set_default_env` → `himport_r`|Env vào bảng băm RAM, `GD_FLG_ENV_READY`|
|`board_init_r`|`initr_mmc`|`mmc_initialize` → `xenon_mmc_initialize`|Đăng ký SD/eMMC (chưa init thẻ)|
|`board_init_r`|`initr_bspi`|`bspi_initialize` → `winbond_quad_enable`|QSPI sẵn sàng đọc (0xF4000000)|
|`board_init_r`|`init_km` [KM]|`init_Panel_Uart`, `is_execHwCheckMode`, `checkPowerSaveKey`, `gpio_*`|Cờ HW-check, khởi tạo UART panel, GPIO quạt|
|`board_init_r`|`misc_init_r` [KM]|`misc_init_r_env` → `read_rom` (SPI)|`ethaddr`, MachineInfo/No, SerialNo|
|`misc_init_r`|`setup_MotionEnable_Power`|`usb_init`, `usb_stor_scan`, `check_action`, `gpio_set_value`|`g_action`, MotionEnable|
|`check_action`|`check_usb_port`|`do_usb`, `BspUhostCheckPortDevice`|env `bootusb`|
|`main_loop`|`check_autoboot`|`make_bootcmds`, `make_bootargs`, `setenv`|`bootcmd`, `bootargs`, `initrd_high`|
|`main_loop`|`warp_boot`|`_warp_drvload`, `warp_bfload`, `hibernate()`|Khôi phục snapshot; không return nếu thành công|
|`autoboot_command`|`abortboot_normal`|`tstc`, `km_scpi_set_wdt_ca72_ext`|Đếm ngược, WDT|
|`autoboot_command`|`run_command_list`|`parse_string_outer` → `cmd_process`|Chạy `bootcmd`|
|`cmd_process`|`find_cmd`|`ll_entry_start`|Tra lệnh trong `.u_boot_list`|
|`do_fat_fsload`|`file_fat_read`|`fat_read_file` → `nvme_read`/`mmc_bread`|Nạp file vào RAM, `setenv_hex("filesize")`|
|`do_booti`|`booti_start`|`bootm_start`, `booti_setup`, `bootm_find_ramdisk_fdt`|Kiểm và chuẩn bị ảnh|
|`boot_prep_linux`|`image_setup_linux`|`boot_ramdisk_high`, `boot_relocate_fdt`, `image_setup_libfdt`|initrd/DTB lên vùng cuối, vá DTB|
|`boot_jump_linux`|`cleanup_before_linux`|`dcache_disable` → `flush_dcache_all` → `__asm_flush_l3_cache`|Đồng bộ và tắt cache/MMU|
|Bất kỳ|`panic`|`do_reset` → `reset_cpu`|Khởi động lại (`RFU_GLOBAL_SW_RST` + GPIO 66)|
|Bất kỳ|`km_scpi_set_wdt_ca72_ext`|`send_cmd` → MHU `0xE8009800` + shmem `0x7FFF0180`|Ra lệnh cho R4|

## 8. Các "lá" phần cứng (callee cuối)

|Chức năng|Điểm chạm phần cứng|
|---|---|
|Console|ns16550 tại `0xF0512000` (AP) / `0xE8003000` (CSRC) / `0xE8002800` (panel)|
|Đọc SPI-flash|cửa sổ MMIO `0xF4000000`; ghi/xoá qua `0xE8273000` (BSPI)|
|SSD|NVMe qua PCIe MSB0125 (BAR MMIO, hàng đợi trong RAM)|
|SD/eMMC|Xenon SDHCI (DT `mmc@e8278000`)|
|USB|xHCI `0xD00D0000`, UTM PHY `0xD00DC000`|
|GPIO|banks `0xE830A000 + 0x100*n`, pad `0xE8308000`|
|Reset|`RFU_GLOBAL_SW_RST 0xF06F0084`, GPIO 66|
|WDT|MHU `0xE8009800`, shmem `0x7FFF0000`|
|I2C|DW-I2C `0xE8006000/6800/7000/7800` (PS-CPU 0x3E)|
|LLC|`0xF0008000` (`CTRL 0x100`, `SYNC 0x700`, `CIW 0x7FC`)|
|Timer|system counter `0xF0581000`|

## 9. Các nhánh "không bao giờ chạm" (đường chết)

- SPL (`initr_*` chỉ cho SPL), mạng (`initr_net`, `eth_initialize`), `initr_scsi`, `initr_ide`, PSCI (`setup_psci`), FIT, DM (`CONFIG_DM` chưa bật, `initf_dm/initr_dm` rỗng), `env_save`, `bubt`.

## 10. Chưa xác minh

- Danh sách đầy đủ `init_sequence_f` sau `setup_dest_addr` (đoạn 960–1100 của board_f.c tôi chưa đọc lại).
- `board_late_init` có được định nghĩa cho cấu hình này không (chỉ suy ra từ Phần 6).
- Trình tự `initr_pci` so với `init_km` (theo thứ tự trong file đã đọc).

---

Xong Phần 32. Tiếp theo **Phần 33: Data flow**: luồng dữ liệu (env, kernel/initrd/DTB, cấu hình từ SPI-flash, log, chọn boot).

Let me verify one of the flagged items quickly before moving on.

Grep "board_late_init|CONFIG_BOARD_LATE_INIT|quartz_late_init" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {board/mvebu/**/*.c,include/configs/armada8k.h,include/configs/mvebu-common.h,include/configs/quartz-sb.h,common/board_r.c})

7 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\board_f.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 959-1068)

Trước khi sang Phần 33, một đính chính cho Phần 32: với ARM64, `jump_to_copy` **không** nằm trong `init_sequence_f` (điều kiện `!defined(CONFIG_ARM)` ở board_f.c:1006). `relocate_code` được `crt0_64.S` gọi **sau khi `board_init_f` trở về**. Phần cuối của `init_sequence_f` thực tế là: `setup_dest_addr → reserve_round_4k → reserve_mmu → reserve_lcd → reserve_trace → reserve_uboot → reserve_malloc → reserve_board → setup_machine → reserve_global_data → reserve_fdt → reserve_stacks → setup_dram_config → show_dram_config → display_new_sp → reloc_fdt → setup_reloc`. Ngoài ra `board_init_f` khởi tạo `termFlg=0`, `printStopFlg=1`, `printBufferNum=0` trước khi chạy dãy này (board_f.c:1036-1041). `board_late_init` **có** (`CONFIG_BOARD_LATE_INIT` ở mvebu-common.h:88, hàm ở `board/mvebu/common/init.c:51` gọi `quartz_late_init`), xác nhận điểm [S] của Phần 32.

# PHẦN 33 (PART 34): Data Flow

## 0. Kết luận

Có **bảy luồng dữ liệu** đáng phân biệt. Điểm chung: **mọi dữ liệu cấu hình bền đến từ SPI-flash** (không từ env), **mọi dữ liệu nạp đến từ phân vùng FAT trên SSD/SD/USB**, và **mọi thông tin gửi cho kernel đi qua DTB `/chosen` và `/memory`**.

## 1. Luồng A: cấu hình bền (SPI-flash → biến toàn cục → bootargs/DTB)

```
SPI-flash (qua mtd/BSPI, cửa sổ 0xF4000000)
  ├ machineinfo A/B (0x79000/0x7A000)  ── THRG_getMachineInfo ──► read_rom()  ─► struct rom { mac[0][6], machineInfo[], machineNo[], serial }
  │        misc_init_r_env:  gMachineInfo, gMachineNo, gSerialNo[16]  ;  setenv("ethaddr")  (chỉ nếu OUI ∈ {00:20:6B,00:50:AA,08:00:86})
  ├ SoftDIP A/B (0x6E000…)              ── THRG_getSoftDipSw(off) ──► 0x06 (NWDL/IISW), 0x413 (Integrity), 0xA1 (KernelStart event), 0xC2 (HW-check), 0x…(term) 
  ├ BSP/Thread A/B (0x77000/0x78000)    ── THRG_getBspThrRegion ──► restart flag 0x00, recovery code 0x2E, SuperWarp-disable 0x20
  ├ user data 0x7B000(+off)             ── THRF_readFlash ──► getMSCDownloading / getMSCSubsetDownloading
  └ secure info 0x3FE000                ── get_secureinfo ──► SBOOT, GMBOOT, GMLPPP
Đích: (1) quyết định g_action / boot type, (2) make_bootargs: MachineInfo=, MachineNo=, SerialNo=, TerminalState=, IntegrityState=, (3) DTB /memory (paneltype…), (4) lệnh WDT
```

Tính chất: mỗi vùng có **hai bản + bộ đếm** nên đọc chịu được mất một bản (`OK_AREA1_ERR/OK_AREA2_ERR`); đọc lỗi hoàn toàn cho giá trị **mặc định 0**, và luồng vẫn chạy (Phần 29).

## 2. Luồng B: quyết định kiểu boot (đầu vào → `g_action` → `bootcmd`)

```
Đầu vào:  GPIO 49 (Utility Key)        ┐
          cờ line-card (SSD/SD)          │  setup_MotionEnable_Power ─► [usb_init + usb_stor_scan] ─► check_usb_port ─► env bootusb
          USB: FW0000/INDEX, tar         │        ─► check_action()  ─► g_action
          SoftDIP 0x06, MSC 0x7B000      │
          HW-check (is_execHwCheckMode, PowerSaveKey GPIO)  ┘
g_action ─► check_autoboot ─► make_bootcmds(mẫu CMD_*_ENV, nvme|mmc) + make_bootargs(boottyp, root, console, …) ─► setenv bootcmd/bootargs/initrd_high
        ─► autoboot(1/2/4/5/6/7/9/10) ─► main_loop (Warp? 'Functional restriction'? autoboot_command)
```

## 3. Luồng C: dữ liệu nạp (ổ đĩa → RAM → kernel)

```
SSD  ──PCIe/NVMe (PRP, DMA, polling, D-cache tắt)──► RAM buffer (bộ đệm căn 64B)
      ──fs_devread / fat_read_file──►  0x00800000  Image (raw ARM64)      ─ booti_setup: memmove → 0x0 + text_offset ; lmb_reserve
                                       0x00001000  emu800.dtb (FDT)        ─ boot_relocate_fdt: fdt_open_into → đỉnh cửa sổ 16MB ; + FDT_PAD
                                       0x03000000  irfs.ubt (uImage ramdisk)─ image_get_ramdisk (magic, hcrc, dcrc, arch=ARM64) ; boot_ramdisk_high: lmb_alloc_base(<4GiB) + memmove
      env filesize (setenv_hex sau mỗi fatload)
Kích thước: Image ≤ ~16 MB cho DTB còn chỗ (Phần 16); initrd chép cao dưới stack U-Boot
```

Đường dẫn dữ liệu chậm: sau `nvme init` U-Boot chạy MMU tắt (Device-nGnRnE), nên nạp/chép/CRC đều không cache (Phần 23).

## 4. Luồng D: DTB, hai nguồn khác nhau

```
(1) DTB ĐIỀU KHIỂN (nhúng trong u-boot.bin, .dtb.init.rodata)  ─ gd->fdt_blob ─►  driver U-Boot (mmc, bspi, sar, pcie, usb3, ccu/rfu) ; KHÔNG gửi cho kernel
(2) DTB KERNEL (emu800.dtb trên SSD p5)  ─ fatload → 0x1000 ─► boot_get_fdt ─► boot_relocate_fdt ─► image_setup_libfdt:
        fdt_chosen           /chosen/bootargs  ◄── env bootargs
        arch_fixup_fdt [KM]  /memory/reg  ◄── bi_dram[] ; /memory: hddinfo, boottype ◄ g_action, hddexistence, sd_existence, paneltype/code/option/size ◄ gPanel*, utilitykey ◄ uc_TempUtilityKey, vramaddr/vramsize ◄ gd->fb_base
        fdt_fixup_ethernet   mac-address, local-mac-address ◄ env ethaddr (nếu có alias)
        fdt_initrd           linux,initrd-start/end ◄ vùng đã chép
     ─► x0 = địa chỉ DTB đã vá
```

## 5. Luồng E: log (một chiều, có bộ đệm)

```
printf/puts ─► console.c (KM) ─► BspPrintStopCheck ? ─► [printStopFlg=1] ghi vào gd->arch.printBuffer[≤255][1051] (sprintf)
                                                     ─► [termFlg=1 sau stdio_add_devices, getTermState()] BspPrintBufferFlush 1 lần ─► serial (AP 0xF0512000 | CSRC 0xE8003000)
Console đầu ra: tuỳ term_sel (1 = AP-UART, 0 = CSRC), `TERM_STATE_0` = console ảo (không UART) ⇒ nuốt log
Đích cuối: UART (nếu cho phép) — không có nơi lưu bền. Reset ⇒ mất.
```

## 6. Luồng F: điều khiển ra ngoài (lệnh tới bộ vi xử lý phụ)

```
U-Boot ──SCPI/MHU (0x7FFF0180, 256 B)──► R4:   0x8C {event, point, timeout, debugInfo1, debugInfo2}   (WDT)
                                              0x0A {0x0100}                                            (pause R4 khi ghi SPI)
U-Boot ──I2C0 @0x3E──► PS-CPU:   reg 0x74 ← 0x0200 (RST_SLP2n) ; reg 0xC0 ← {4 WarpStart | 10 KernelStart}
U-Boot ──UART panel 57600 8E1──► vi điều khiển panel: khung {0xFE,0x06,0x30,LED1,LED2,0,0,0xFF}+chk ; init panel ; ảnh khởi động qua VRAM
U-Boot ──GPIO──► MOTION_EN_MC/IR, BOXFAN, HRESET_REQ(66), POWER_OFF_PNL(79)
```

## 7. Luồng G: DDR và Warp

```
ATF (ngoài repo): huấn luyện LPDDR4 ─► kết quả tại 0x2_0000_0000 (Train_Result_t) + trạng thái SRAM 0xC0200000 + cờ 0xE8244834
U-Boot: qspi_write_lpddr4_training_result ─► SPI 0x380000 (tối đa 4096 B)      [KM_RESUME_REFINE]
Warp: driver (SPI 0x2B0000) + bootflag (SD/NVMe p3|p10) ─► hibernate() ─► phục hồi RAM từ snapshot (cần cùng kết quả huấn luyện)
```

## 8. Bảng: ai sở hữu dữ liệu

|Dữ liệu|Nơi lưu bền|Ai ghi|Ai đọc (U-Boot)|Ảnh hưởng|
|---|---|---|---|---|
|MAC, serial, kiểu máy|SPI machineinfo|`y`,`z` (U-Boot) / nhà máy / Linux|`misc_init_r_env`|bootargs, DTB|
|SoftDIP|SPI NVRAM|Linux|nhiều nơi|chọn boot/console/tự chẩn đoán|
|Cờ tải MSC/Subset|SPI 0x7B000|Linux/bộ cập nhật|`getMSC*`|hướng cập nhật|
|Cờ GM/SBOOT|SPI 0x3FE000|Linux (GM), U-Boot (SBOOT)|`chkGmUpdate`, `chk_sboot`|sao lưu, khoá tháo chip|
|Snapshot Warp|SD/NVMe p3/p10|Linux|`warp_boot`|boot nhanh|
|Kernel/DTB/initrd|SSD p5|bộ cập nhật|`fatload`|boot|
|Env|RAM (mất khi reset)|U-Boot (`check_autoboot`, `setenv`)|mọi nơi|tạm thời|
|Log|RAM (`gd`)|`printf`|UART/không|gỡ lỗi|

## 9. COMMON CONFUSION

1. **"Env là nơi lưu cấu hình"**: không; cấu hình bền ở SPI-flash.
2. **"DTB U-Boot cũng đi vào kernel"**: hai DTB khác nhau.
3. **"`ethaddr` dùng cho mạng U-Boot"**: chỉ để vá DTB.
4. **"`filesize` chỉ có sau `fatload` cuối"**: đổi sau mỗi lần nạp.
5. **"Log được lưu"**: không, có bộ đệm RAM tạm, có thể không bao giờ hiện.

## 10. MENTAL MODEL

```
SPI-flash ─(cấu hình/cờ)─► g_action, bootargs, DTB ─► kernel
SSD ─(kernel,dtb,initrd)─► RAM ─► x0=DTB, initrd trong /chosen
U-Boot ──(WDT/PS-CPU/panel/GPIO)──► phần cứng phụ
Log ─► bộ đệm ─(nếu phép)─► UART
```

## 11. Chưa xác minh

- Bố cục `struct rom` chính xác.
- Định dạng `INDEX/EXP_INDEX`.
- Ý nghĩa giá trị `TERM_STATE_*` và nguồn `getTermState()` (DIP nào).
- Dải PCIe inbound bao phủ RAM.

---

Xong Phần 33. Tiếp theo **Phần 34: State machine**: máy trạng thái của `g_action`/`autoboot`, Warp, USB scan, NVMe init.

Grep "TERM_STATE_[0-9]|getTermState|BspPrintValid|BspPrintStopCheck" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {board/mvebu/armada8k/km/dipvalue.c,board/mvebu/armada8k/km/dipvalue.h,common/console.c})

101 lines of output

# PHẦN 34 (PART 35): State Machines

## 0. Kết luận

U-Boot không có "máy trạng thái" khai báo tường minh. Các trạng thái nằm ở **biến toàn cục** (`g_action`, `gd->flags`, `gd->arch.termFlg`, `printStopFlg`, `usb_started`, `warp_saveno`, `nvme_info[].pdev`, `current_state` của `pause_r4`) và ở **thứ tự hàm gọi**. Dưới đây là chín máy trạng thái đã kiểm chứng từ mã, mỗi máy có bảng trạng thái, sự kiện, chuyển, và hành động.

Ghi chú bổ sung cho Phần 33 (đọc lại `console.c`): `BspPrintValid()` chỉ trả 0 khi `getTermState() == TERM_STATE_0` ("phi debug"), còn mọi trạng thái khác được in. Nếu không đọc được TERM_STATE thì mặc định `TERM_STATE_0` (console.c:317). Ngoài ra `BspPrintStopCheck()` là **cờ chống gọi lồng** (`gc_stopflg`) để tránh vòng đệ quy khi `getTermState()` tự `printf`.

## 1. FSM-1: quyết định kiểu boot (`g_action`)

Biến: `g_action` (int, khởi tạo `DEF_BOOT=0`).

```
[START] ─ setup_MotionEnable_Power ─►
   Utility Key ĐANG NHẤN (GPIO49 cao) hoặc cờ line-card = 1 ?
   ├ CÓ ─► SCAN_USB ─► check_action():
   │         USB storage có mặt? ── KHÔNG ─► đọc SoftDIP 0x06 (NWDL) : 5|6→IISW ; 4|7|8|9→UPDATE ; 10→BACKUP ; 11→RESTORE ; HW-check→HW_CHECKMODE ; else DEF_BOOT
   │         CÓ ─► đọc FWxxxx/INDEX:
   │               không có INDEX → thử tar line-card→BOOT_LINE ; tar init→BOOT_INIT ; tar bderase→BOOT_BDERASE ; else DEF_BOOT
   │               INDEX sai TYP → BOOT_ERR
   │               INDEX loại USB_DOWN→USB_UPDATE ; USB_ZZZ→USB_BOOT ; USB_SPD…
   └ KHÔNG ─► "NON_USB_MEMORY": IISW(5,6)→DEF_IISW ; (4,7,8,9)→DEF_UPDATE ; cờ MSC=1→BOOT_ERR ; 10→BACKUP ; 11→RESTORE ; HWcheck→HW_CHECKMODE ; else DEF_BOOT
[END] g_action
```

Sau đó `check_autoboot` (main.c:1870) ánh xạ `g_action` → `autoboot` (chuỗi 1,2,4,5,6,7,9,10). Ngoại lệ: `DEF_UPDATE` còn đọc `INDEX` trên 0:A; sai → `g_action = BOOT_ERR`.

## 2. FSM-2: vòng chính (`main_loop`)

|Trạng thái|Sự kiện/điều kiện|Chuyển|Hành động|
|---|---|---|---|
|M0 INIT|vào `main_loop`|M1|`cli_init`, `run_preboot_environment_command`|
|M1 CARD?|`g_action ∈ {BOOT_INIT, BOOT_BDERASE}`|M_CARD (kết thúc, treo)|WDT END; `initialize_card_main`/`bderase_card_main`|
|M1|khác|M2||
|M2 SBOOT|`chk_sboot()`|M3 hoặc M_LOCK|Chip bị tháo → khoá vĩnh viễn (WDT END)|
|M3 DELAY|`bootdelay_process` → `check_autoboot`|M4|ghi `bootcmd/bootargs`; `autoboot` = giá trị|
|M4 DISPATCH|`autoboot == WARP` hoặc (`LINE` và `checkWarpBootMode`)|M5 WARP||
|M4|`autoboot ∈ {NOT_AUTOBOOT, NOT_AUTOBOOT_FOR_BOOTDEV}`|M7 CLI|in "Functional restriction 1/2"|
|M4|khác (kể cả 8)|M6 BOOT||
|M5 WARP|`warp_boot` thành công|(không trở về)|kernel Warp|
|M5|trả về (thất bại)|M6|`check_autoboot_DEF_BOOT_case` chỉnh `autoboot`|
|M6 BOOT|`abortboot` = 0|chạy `bootcmd`|WDT theo `g_action`; `chkGmUpdate` (DEF_BOOT)|
|M6|phím bất kỳ (abort)|M7|WDT END, "Push Any Key"|
|M6|`bootcmd` trả về (lỗi)|M7|WDT END|
|M7 CLI|—|(mãi mãi)|`cli_loop`; prompt `Marvell>>`|

Trạng thái kết thúc duy nhất trong bình thường: **nhảy vào kernel** (không trở về) hoặc reset.

## 3. FSM-3: Warp!! resume

Biến: `warp_saveno`, `warp_drvaddr`, `warp_bfaddr`, `boot_param`.

```
W0 IDLE ─warp_boot(saveno)─► W1 DRV_LOAD: đọc 32 byte header từ SPI 0x2B0000
    ID != 'W5HD'   ─► W_ERR ("Can't load Warp!! driver")
    ID hợp lệ      ─► đọc DATA_END byte ; flush_cache ─► W2 BF_LOAD
W2 BF_LOAD: đọc 0x400 byte bootflag (SD/NVMe p3|p10) vào drvaddr+TEXT_END
    dấu != 'W5BF'  ─► W_ERR (rỗng/đã xoá) ─► return 1
    hợp lệ         ─► W3 PREBOOT: boot_param = bfaddr+0x400 ; memset ; silent=console=bps=-1
W3 ─► warp_prepare_boot (smp_kick, flush_dcache_all, timer 0xF0630000 ← 0x100) ─► W4 HIBERNATE: hibernate()
W4: thành công ⇒ KHÔNG trở về (RAM/CPU được khôi phục từ snapshot)
    thất bại   ⇒ in "Warp!! error can not boot %d" ⇒ W_ERR
W_ERR ─► main_loop: SuperWarp → Normal → boot thường (mục FSM-2)
Sau khi tới boot thường: warp_clear_bootf(Normal) ; warp_clear_bootf(Super) (trừ DEF_UPDATE) ⇒ bootflag bị xoá (snapshot dùng một lần)
```

Điều kiện chọn SuperWarp: `panelKey != 0x16` **và** `isDisableSuperWarp_ErrorOccurred() == 0`; nếu không thì `warp_clear_bootf(Super)` rồi Warp thường. `setStartupBypeNomalWarp` hạ `STARTUP_TYPE` từ SuperWarp xuống NormalWarp nếu `LOOP_BOOT_CNT_BOOT == 1`.

## 4. FSM-4: quét USB

```
U0 OFF (usb_started = 0)
  ─ Utility Key / cờ line-card ─► U1 INIT: usb_init()
        (PS-CPU /RST_SLP2n, PHY UTM PLL [vòng chờ vô hạn nếu hỏng], xHCI reset/start, root hub, usb_new_device)
  ─► U2 SCAN: usb_stor_scan(1) → usb_stor_curr_dev
        ≥ 0 ─► U4 CHECK (check_usb_port → bootusb → INDEX)
        < 0 và retry < 2 ─► U3 RETRY: usb_stop(); mdelay(10000) ─► U1 (retry=1)
        < 0 và retry == 2 ─► U4 với "không có USB"
  U4 ─► check_action ─► g_action
```

WDT trong khoảng: 120 s (0x1120) trước quét, 5 s (0x1121) sau. `usb_started` chỉ bằng 1 khi có ít nhất một controller khởi tạo được.

## 5. FSM-5: console và log

Biến: `printStopFlg`, `termFlg`, `printBufferNum`, `gc_stopflg`.

|Trạng thái|Điều kiện vào|Hành vi `printf/putc`|
|---|---|---|
|C0 BUFFER|`board_init_f` đặt `termFlg=0`, `printStopFlg=1`, `printBufferNum=0`|`BspPrintValid()`=0 (hoặc chưa sẵn sàng) ⇒ `BspPrintBuffering`: chép vào `printBuffer[n]` (tối đa `PRINT_BUFFER_NUM=255` mục)|
|C1 READY|`stdio_add_devices` đặt `termFlg = 1`|`getTermState()` đọc (GPIO/SoftDIP)|
|C2 FLUSH_ONCE|`getTermState() != TERM_STATE_0`|`BspPrintBufferFlush()` một lần: in toàn bộ bộ đệm|
|C3 PASS|sau flush|in thẳng ra `serial_puts`|
|C_HIDDEN|`getTermState() == TERM_STATE_0` (hoặc lỗi đọc)|ở lại C0: bộ đệm đầy 255 mục thì không lưu thêm (hành vi khi đầy chưa đọc [?])|

`console=` truyền cho kernel cũng theo `term_state` (Phần 16).

## 6. FSM-6: NVMe và cache

|Trạng thái|Chuyển|Đặc tính|
|---|---|---|
|N0 UNINIT (`nvme_info[].pdev == NULL`)|`nvme init`/`nvme_initialize`||
|N1 INIT|`init_nvme` thành công|D-cache **tắt**, MMU **tắt** (giữ nguyên)|
|N1' INIT thất bại|`ret != 0`|`nvme_dcache_enable()` khôi phục|
|N2 READY|`nvme_read`/`nvme_write` polling|không đổi cache|
|N3 STOPPED|`nvme stop` → `stop_nvme`|xoá SQ/CQ, shutdown controller, D-cache **bật lại**|
|Warp|`warp_nvme_init` lấy `nvme_init()`||

`bootcmd` không gọi `nvme stop`, nên trạng thái cuối trước kernel là N2.

## 7. FSM-7: watchdog (theo phía U-Boot)

```
OFF ──initr_wd_start(0x1000, 5s)──► ARMED_5s ──(mỗi bước init_sequence_r: refresh 0x1000|n, 5s)──►
   ├ USB path:   0x1120 (120s) ─► 0x1121 (5s)
   ├ IISW/UPD:   0x1130 (10s) ...
   ├ warp:       0x11B0 (40s) / 0x11A0 (90s)
   ├ autoboot:   0x11D0 (bootdelay×2 | 5s) ─► [abort: END] | 0x11D1 (5s)
   └ chạy bootcmd: DEF_BOOT 0x11D2 (300s) | USB_UPDATE/IISW/UPDATE 0x11E0/1/2 (120s) | khác: END
END ──(bootcmd trả về / abort / card / chk_sboot fail / một nhánh setup)──► OFF (không lưới)
```

Hết hạn trong bất kỳ trạng thái ARMED ⇒ R4 reset máy (hành vi R4 suy từ cách dùng) [S].

## 8. FSM-8: reset và recovery counter

```
R0 (counter=0) ─lỗi đồ hoạ(code)─► reset_with_recoverycode:
   đọc (code<<8|counter) @BSP 0x2E ; counter++ 
   counter ≤ 5 và đọc SPI OK ─► ghi mã+counter ; SuperWarp disable ; do_reset ─► REBOOT ─► R(counter)
   counter > 5 hoặc đọc SPI lỗi ─► in, return (tiếp tục boot, KHÔNG reset)
Firmware chính (Linux) xoá về 0 khi boot thành công.
```

Số lần tối đa reset liên tiếp: 5. `panic` (ngoại lệ) không có bộ đếm.

## 9. FSM-9: chip bảo mật (`chk_sboot`)

|Chip (PIO_SYS[36])|Cờ SPI `SBOOT`|Kết quả|
|---|---|---|
|Có (H)|≠ 1|ghi cờ = 1 ("boot flg on"), cho boot|
|Có|= 1|cho boot|
|Không (L)|≠ 1|cho boot (máy chưa từng có chip)|
|Không|= 1|**"boot failed."** → WDT END → `while(1)` (khoá)|
|Đọc cờ lỗi|—|"flash read err" → khoá (fail-closed)|

## 10. FSM-10: môi trường (env)

```
E0 (pre-reloc): env_addr = &default_environment[0], env_valid=0 ; getenv ⇒ getenv_f (buffer 32 B) ; setenv ⇒ trả 1 (bị bỏ)
E1 (initr_env → env_relocate): set_default_env(NULL): himport_r → env_htab ; gd->flags |= GD_FLG_ENV_READY
E2 (runtime): setenv (malloc + hsearch ENTER, env_id++)  ; check_autoboot ghi bootcmd/bootargs/initrd_high/device_str
E3: reset ⇒ trở về E0 (không lưu)
```

## 11. Bảng biến trạng thái toàn cục

|Biến|Giá trị/ý nghĩa|Đặt bởi|
|---|---|---|
|`g_action`|0–13|`check_action`, `doboot`|
|`uc_TempUtilityKey`|0 = nhấn, 1 = không|`init_Panel`|
|`uc_HWcheckModeBoot`, `uc_AutoHWcheckMode`|0/1|`init_km`, `misc_init_r`|
|`gPanelKey`|0x16 = phím đặc biệt|panel|
|`usb_started`, `usb_stor_curr_dev`|0/1, −1..|`usb_init`, `usb_stor_scan`|
|`warp_saveno`, `warp_drvaddr`, `boot_param`||`cmd_warp.c`|
|`gd->arch.termFlg/printStopFlg/term_sel`|log/console|`board_init_f`, `stdio_add_devices`|
|`gd->flags` `GD_FLG_ENV_READY/RELOC/SERIAL_READY/SILENT`||core|
|`nvme_info[].pdev`|NULL/không|`init_nvme`|
|`gc_stopflg`|chống gọi lồng log|`BspPrintStopCheck`|
|`current_state` (`pause_r4`)|đếm lồng|`arm_scpi.c`|

## 12. COMMON CONFUSION

1. **"`g_action` chỉ do người dùng chọn"**: do phần cứng/cờ/USB quyết định.
2. **"Warp lỗi thì dừng"**: rơi xuống boot thường.
3. **"Log bị mất là do hỏng"**: là trạng thái C_HIDDEN cố ý.
4. **"WDT luôn chạy"**: có những trạng thái END không có lưới.

## 13. MENTAL MODEL

```
Phần cứng/cờ/USB ─► [FSM-1 g_action] ─► [FSM-2 main_loop] ─┬─► [FSM-3 Warp] ─► kernel | rơi xuống
                                                           ├─► CLI
                                                           └─► bootcmd ─► kernel
Bên cạnh: [FSM-7 WDT] [FSM-5 log] [FSM-6 NVMe/cache] [FSM-8 recovery] [FSM-9 chk_sboot] [FSM-10 env]
```

## 14. Chưa xác minh

- Hành vi khi `printBuffer` đầy (`BspPrintBuffering` không đọc kỹ).
- Ý nghĩa `TERM_STATE_1..n` và nguồn `getTermState()` (DIP nào).
- Tác động phía R4 khi WDT hết hạn.

---

Xong Phần 34. Tiếp theo **Phần 35: Hardware registers**: tổng hợp các thanh ghi/địa chỉ vật lý mà U-Boot chạm vào.

Bash Read register base definitions

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && sed -n 1,80p arch/arm/include/asm/arch-armada8k/regs-base.h; echo ----; sed -n 1,60p arch/arm/include/asm/arch-mvebu/regs-base.h 2>/dev/null | head -60

OUT

/*
 * ***************************************************************************
 * Copyright (C) 2015 Marvell International Ltd.
 * ***************************************************************************
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 2 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ***************************************************************************
 */

#ifndef _REGS_BASE_H_
#define _REGS_BASE_H_

#include <asm/arch/memory-map.h>

/* Base registers for AP */
#define MPP_REGS_BASE	(MVEBU_REGS_BASE + 0x6F008C)

/* List of register base for all units */
#define MVEBU_ADEC_AP_BASE 	(MVEBU_REGS_BASE + 0x0)
#define MVEBU_ADEC_RFU_BASE 	(MVEBU_REGS_BASE + 0x680000)
#define MVEBU_ADEC_IOB_BASE 	(MVEBU_REGS_BASE + 0x2810000)

#define MVEBU_RFU_BASE		(MVEBU_REGS_BASE + 0x6F0000)

/*ERICYH_CHANGED*/
#ifdef CONFIG_PALLADIUM
#ifndef CONFIG_MVEBU_AP806_16750
#define MVEBU_UART_BASE(x)      (0xe8004000)
#else
#define MVEBU_UART_BASE(x)	(MVEBU_REGS_BASE + 0x512000 + (0x100 * x))
#endif
#else
#define MVEBU_UART_BASE(x)	(MVEBU_REGS_BASE + 0x512000 + (0x100 * x)) 
#endif

#define MVEBU_MISC_REGS_BASE	(MVEBU_REGS_BASE + 0x18200)
#define MVEBU_DEVICE_ID_REG	(MVEBU_MISC_REGS_BASE + 0x38)
#define MVEBU_DEVICE_REV_REG	(MVEBU_MISC_REGS_BASE + 0x3C)
#define MVEBU_RESET_MASK_REG	(MVEBU_MISC_REGS_BASE + 0x60)
#define MVEBU_SOFT_RESET_REG	(MVEBU_MISC_REGS_BASE + 0x64)

#define MVEBU_GICD_BASE		(MVEBU_REGS_BASE + 0x210000)
#define MVEBU_GICC_BASE		(MVEBU_REGS_BASE + 0x220000)

#define MVEBU_GENERIC_TIMER_BASE	(MVEBU_REGS_BASE + 0x581000)

#define MVEBU_LLC_BASE			(MVEBU_REGS_BASE + 0x8000)

#define MVEBU_IP_CONFIG_REG		(MVEBU_REGS_BASE + 0x6F4100)

/* stefan mci0x2 refactor for multiple ports*/
/* #define MVEBU_IHB_REGS_BASE 		(MVEBU_REGS_BASE + 0xde8238)
 */
#endif	/* _REGS_BASE_H_ */
----

Grep "define\s+(MVEBU_REGS_BASE|MVEBU_MC_CTRL_0|MVEBU_RFU_BASE|RFU_SW_RESET_OFFSET|EMMC_PHY_IO_CTRL|MVEBU_CCU_BASE|MVEBU_SAR)" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\include\asm)

7 lines of output

# PHẦN 35 (PART 36): Hardware Registers

## 0. Kết luận

Bảng dưới liệt kê **mọi địa chỉ vật lý đã được xác minh trong mã** mà U-Boot chạm vào, gom theo khối. Nền `MVEBU_REGS_BASE = 0xF0000000` (`arch-armada8k/memory-map.h:27`); vùng `0xE8xxxxxx` là southbridge **Quartz**, `0xD00xxxxx` là **Iris** (USB), `0xC00xxxxx` là khối UL/PIO, `0xF4000000` là cửa sổ bộ nhớ của QSPI. Cột "Mức" là [V] nếu thấy giá trị và cách dùng trong mã, [S] nếu suy luận. Không có tài liệu tham chiếu Marvell nên tên thanh ghi ngoài mã là suy luận.

## 1. SoC AP806 (`0xF0xxxxxx`)

|Địa chỉ|Tên/nhãn trong mã|Dùng bởi|Ý nghĩa/thao tác|Mức|
|---|---|---|---|---|
|`0xF0008000`|`MVEBU_LLC_BASE`|`cache_llc.c`, `cache.S:160`|LLC (L3)|V|
|`0xF0008100`|`LLC_CTRL`|`llc_mode_get`, `__asm_flush_l3_cache`|bit0 = EN, bit8 = exclusive|V|
|`0xF0008700`|`LLC_SYNC`|như trên|ghi 1 = đồng bộ|V|
|`0xF00087BC`|`LLC_MNT_CW`|(khai báo)|clean theo way|V|
|`0xF00087FC`|`LLC_MNT_CIW`|`flush_l3_cache`|ghi `0xFFFFFFFF` = clean-invalidate mọi way|V|
|`0xF0018200+`|`MVEBU_MISC_REGS_BASE` (`+0x38` DEVICE_ID, `+0x3C` REV, `+0x60` RESET_MASK, `+0x64` SOFT_RESET)|`soc.c`|ID/rev SoC, reset|V (địa chỉ)|
|`0xF0020200 + ch*0x200 + cs*8`|MC "chip select"|`dram_init_banksize`|bit0 valid; [20:16] mã kích thước|V|
|`0xF0001700`/`0xF0001704`|`MC_RCR`/`RSBR`|`dram_init_banksize` (KM)|ánh xạ lại bank 2|V|
|`MVEBU_MC_CTRL_0`|(ADEC?)|`print_soc_specific_info`: bus width = `(1 << ((r & 0x700)>>8)) * 4`|độ rộng bus DDR|V (công thức)|
|`0xF0210000`|`GICD_BASE`|`smp_kick_all_cpus` (`GICD_SGIR`)|GICv2 distributor|V|
|`0xF0220000`|`GICC_BASE`|(gic_init_secure_percpu, chỉ EL3)|GIC CPU interface|V|
|`0xF0512000 (+0x100·n)`|`MVEBU_UART_BASE(n)`|serial ns16550|AP-UART (console `ttyS0`)|V|
|`0xF0581000`|`MVEBU_GENERIC_TIMER_BASE`, `GTC_CNTCR` (+0)|`timer_init`|bit0 bật system counter|V|
|`0xF0630000`|`WARP_TIMER_BASE` (Timer 1)|`warp_prepare_boot`|ghi `0x100` trước resume|V (giá trị); ý nghĩa [?]|
|`0xF06F0000`|`MVEBU_RFU_BASE`|ADEC/RFU|cửa sổ RFU|V|
|`0xF06F0084`|`RFU_GLOBAL_SW_RST`|`reset_cpu`|xoá bit `RFU_SW_RESET_OFFSET` (bit 0)|V|
|`0xF06F008C`|`MPP_REGS_BASE`|MPP (không dùng)||V|
|`0xF06F4100`|`MVEBU_IP_CONFIG_REG`|||V (địa chỉ)|
|`0xF0680000`|`MVEBU_ADEC_RFU_BASE`|CCU/RFU windows||V|
|`0xF2810000`|`MVEBU_ADEC_IOB_BASE`|IOB||V|

## 2. Southbridge Quartz (`0xE8xxxxxx`) và liên quan

|Địa chỉ|Khối|Dùng bởi|Ý nghĩa|Mức|
|---|---|---|---|---|
|`0xE8002800`|UART panel (DW)|`mfp_panel.c`|vi điều khiển panel, 57600 8E1|V|
|`0xE8003000`|UART CSRC (DW)|serial|console `ttyS5`|V|
|`0xE8004000`|UART (PALLADIUM)|(chỉ mô phỏng)||V|
|`0xE8006000/6800/7000/7800`|DW-I2C bus 0..3|`quartz-sb.h`|400k/400k/100k/100k; bus0: PS-CPU 0x3E, RTC 0x3D|V|
|`0xE8008000` (`+0x0C` = `0xE800800C`)|(R4/MHU ctrl?)|`pause_r4(false)`|ghi 1 = cho R4 chạy lại|V (giá trị); tên [?]|
|`0xE8009800`|`MHU_BASE`|`arm_scpi.c`|MHU: `SCPI_RX_INT_STATUS +0`, `RX_SET +4`, `RX_CLEAR +8`, `TX_STATUS +0xC`, `TX_SET +0x10`, `TX_CLEAR +0x14`|V|
|`0xE8240080`|`CPUDBG_CLKGENCONFIG`|`get_lppp_wfi_status`|bật clock debug (`0x3`)|V|
|`0xE8244834`|cờ `[15:14]`|`qspi_write_lpddr4_training_result`|trạng thái huấn luyện DDR|V (Phần 12)|
|`0xE8246260`|`LPPP_R4_PWRSTATE`|`get_lppp_wfi_status`|bit 0x10 = R4 đang WFI|V|
|`0xE8273000`|BSPI (`mrvl,bspi`)|`qspi_winbond.c`, DT|điều khiển QSPI (`0x100` byte)|V|
|`0xE8278000`|Xenon SDHCI (`mmc@e8278000`)|`xenon_mmc.c`, DT|SD/eMMC|V|
|`0xE8308000 + 4n`|pad `FUNC_SEL`|`mvqz_gpio.c`|chọn chức năng chân|V|
|`0xE830A000 + 0x100·n`|GPIO bank A..I|`mvqz_gpio.c`|`PLR +0x0`, `PDR +0x4`, `PSR +0x8`, `PCR +0xC`, `SDR +0x1C`, `CDR +0x20`|V|

## 3. Iris/USB và khối UL

|Địa chỉ|Khối|Dùng bởi|Ý nghĩa|Mức|
|---|---|---|---|---|
|`0xD00A2000` (0x1000)|USB3 PHY (`phy=<…>`)|`phy-usb3-mv.c` (bị bỏ)|PHY SuperSpeed|V|
|`0xD00D0000`|xHCI (`usb3@d00d0000`)|`xhci-mvebu.c`|HCCR/HCOR|V|
|`0xD00DC000`|UTM PHY (USB2)|`phy-utm-mv.c`|offsets `PLL0 0x0`, `PLL1 0x4`, `CAL 0x8`, `TX_CCR0 0xC`, `RX_CC0 0x14`, `RX_CC1 0x18`, `DIG_CR_0 0x1C`|V|
|`0xC0042008`|`UL_PI2_PIO_REG`|`sysBspSecureChip`|bit `0x10` = PIO_SYS[36] (chip bảo mật)|V|
|`0xC0200000`|SRAM trạng thái huấn luyện|`qspi_write_lpddr4_training_result`||V (Phần 12)|

## 4. Bộ nhớ ánh xạ và vùng RAM có chủ đích

|Địa chỉ|Nội dung|Ghi chú|
|---|---|---|
|`0xF4000000` (+0x700000)|Cửa sổ đọc QSPI (`memmap_base`)|`bspi0` trong DT; `0xF4380000` = vùng kết quả LPDDR4|
|`0x00001000`|`TEXT_BASE` U-Boot; sau đó là `fdt_addr`||
|`0x00FF1000`|SP ban đầu (`CONFIG_SYS_INIT_SP_ADDR`)||
|`0x02000000`|`CPU_RELEASE_ADDR`, `SYS_LOAD_ADDR`, `PRE_CON_BUF_ADDR`|ba mục đích chồng|
|`0x03000000`|`initrd_addr`||
|`0x00700000` / `0x00800000`|`INDEX_LOAD_ADDRESS` / `IMAGE_LOAD_ADDRESS`, `kernel_addr`, `WARP_LOAD_ADDR`||
|`0x10000000` (4 MB)|ATF (BL1/BL2/BL31)||
|`0x10400000` (1 MB)|vùng huấn luyện DDR||
|`0x3FE10000` (0x1E0000)|VRAM (`gd->fb_base`)||
|`0x3FFF0000` (64 KB)|bảng trang (TLB)||
|`0x7FF00000` (1 MB)|`CONFIG_SCPI_MAILBOX_BASE`||
|`0x7FFF0000`|`PLAT_CSS_SCP_COM_SHARED_MEM_BASE`|kênh 1 AP→SCP tại `+0x180`, SCP→AP tại `+0x100`|
|`0x2_0000_0000`|bank 1 DRAM; kết quả huấn luyện `Train_Result_t`||

## 5. Thanh ghi hệ thống ARMv8 được U-Boot chạm

|Thanh ghi|Nơi|Mục đích|
|---|---|---|
|`VBAR_ELx`|start.S:70-86, 233-241|vector|
|`CPTR_EL2`=`0x33FF` / `CPACR_EL1`=`3<<20`|start.S:81-86|bật FP/SIMD|
|`SCR_EL3`, `CNTFRQ_EL0`|start.S:73-78 (chỉ EL3)||
|`SCTLR_ELx` (M, C, I bit)|cache_v8.c|MMU, D/I-cache|
|`TTBR0/TCR/MAIR_ELx`|`set_ttbr_tcr_mair`|bảng trang|
|`ESR_ELx`, `ELR_ELx`|exceptions.S|chẩn đoán|
|`CSSELR/CCSIDR/CLIDR_EL1`, `CTR_EL0`|cache.S|bảo trì cache theo set/way, range|
|`MIDR_EL1`|`branch_if_a57_core`|errata|
|`CurrentEL`|`switch_el`|chọn nhánh EL|
|`cpuactlr_el1 (S3_1_c15_c2_0)`|errata A57|(`CONFIG_ARM_ERRATA_*` chưa chắc bật)|
|`CNTPCT`/`CNTFRQ`|timer|`get_timer`, `udelay`|

## 6. GPIO đã biết (số hiệu → vai trò)

|GPIO|Tên|Vai trò|Hướng|
|---|---|---|---|
|49|`UTILITY_KEY`|phím tiện ích (cao = nhấn)|in|
|66|`HRESET_REQ`|yêu cầu khởi động lại nguồn|out|
|79|`POWER_OFF_PNL`|tắt nguồn panel (chung IO pad với 49 theo comment)|out|
|228|`WARP_SW`|công tắc Warp|in|
|233|`LOG_SW`|công tắc log|in|
|266–271|`FW_CONFIG0..5`|cấu hình phần cứng (271 cao ⇒ ESEVAL; 269/270 chọn UART)|in|
|—|`MOTION_EN_MC`, `MOTION_EN_IR`|cho phép cơ cấu|out|
|—|`BOXFAN_HALF_REM`, `BOX_FAN_REM`|quạt (`init_km`: half=0, full=1)|out|
|—|`PANEL_SUBSW`|phím tiết kiệm điện|in|

## 7. Nơi kiểm chứng

- Địa chỉ nền: `arch/arm/include/asm/arch-armada8k/{memory-map.h,regs-base.h}`.
- MHU/SCPI: `drivers/scpi/arm_scpi.c:40-90`.
- PHY UTM: `drivers/usb/phy/phy-utm-mv.c:20-68`.
- GPIO: `drivers/gpio/mvqz_gpio_regs.h`, `mvqz_gpio.c` (Phần 11).
- UL PIO: `board/.../km/machine_setup.c:795-819`.
- LLC: `arch/arm/cpu/armv8/armada8k/cache_llc.c:23-66`, `cache.S:159-175`.
- DDR MC: `soc.c` (`dram_init_banksize`, Phần 8).

## 8. Điểm cần cẩn trọng

1. **Truy cập byte tới thanh ghi**: `get_lppp_wfi_status` dùng `*(volatile char *)` trên `0xE8240080`/`0xE8246260` (truy cập 8-bit) chứ không phải 32-bit; đúng hay sai tuỳ thiết kế thanh ghi [?].
2. `0xE800800C` được ghi bằng `*(uint32_t *)` (không `writel`), nằm trong vùng khác MHU (`0xE8009800`); tên thật chưa xác định.
3. **Thanh ghi bằng con trỏ thô trong mã panel/SoftDIP**: khả năng thứ tự bộ nhớ không đảm bảo, Device-nGnRnE (MMU) đảm bảo ở đây (`MT_DEVICE_NGNRNE`), nhưng khi MMU tắt (sau `nvme init`) mọi truy cập cũng là Device nên vẫn an toàn về thứ tự.
4. **Không có `dsb/isb` tường minh** quanh các ghi thanh ghi quan trọng (reset, LLC) ngoài `dsb sy` trong `__asm_flush_l3_cache`; `reset_cpu` không có rào chắn trước GPIO [V bằng đọc code].

## 9. Chưa xác minh

- Tên/chức năng thanh ghi ngoài mã (không có datasheet trong repo; đường dẫn `88PAQZ21-…pdf` được nhắc ở `lcd_lvds_pll.c` nhưng không có).
- Ý nghĩa `0xE800800C`, `0xF0630000`, `MC_CTRL_0`.
- Bản đồ ADEC/IOB/CCU chi tiết (chỉ đề cập ở Phần 8).

---

Xong Phần 35. Tiếp theo **Phần 36: Important structures**: `gd_t`, `bd_t`, `cmd_tbl_t`, `image_header_t`, `bootm_headers_t`, `struct rom`, `struct warp_*`, `nvme_*`, v.v.

Bash Read gd_t definition and struct rom

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && sed -n 20,120p include/asm-generic/global_data.h; echo ----; grep -n "struct rom" -A25 board/mvebu/armada8k/km/mfp_config.h 2>/dev/null | head -50; grep -rn "struct rom" include board/mvebu/armada8k/km/*.h | head -5

OUT

 * Each architecture has its own private fields. For now all are private
 */

#ifndef __ASSEMBLY__
#include <linux/list.h>

typedef struct global_data {
	bd_t *bd;
	unsigned long flags;
	unsigned int baudrate;
	unsigned long cpu_clk;	/* CPU clock in Hz!		*/
	unsigned long bus_clk;
	/* We cannot bracket this with CONFIG_PCI due to mpc5xxx */
	unsigned long pci_clk;
	unsigned long mem_clk;
#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO)
	unsigned long fb_base;	/* Base address of framebuffer mem */
#endif
#if defined(CONFIG_POST) || defined(CONFIG_LOGBUFFER)
	unsigned long post_log_word;  /* Record POST activities */
	unsigned long post_log_res; /* success of POST test */
	unsigned long post_init_f_time;  /* When post_init_f started */
#endif
#ifdef CONFIG_BOARD_TYPES
	unsigned long board_type;
#endif
	unsigned long have_console;	/* serial_init() was called */
#ifdef CONFIG_PRE_CONSOLE_BUFFER
	unsigned long precon_buf_idx;	/* Pre-Console buffer index */
#endif
#ifdef CONFIG_MODEM_SUPPORT
	unsigned long do_mdm_init;
	unsigned long be_quiet;
#endif
	unsigned long env_addr;	/* Address  of Environment struct */
	unsigned long env_valid;	/* Checksum of Environment valid? */

	unsigned long ram_top;	/* Top address of RAM used by U-Boot */

	unsigned long relocaddr;	/* Start address of U-Boot in RAM */
	phys_size_t ram_size;	/* RAM size */
//	2018/04/06 �c�� - Sparrow ����\���ύX_No.17 Boot�f�f�Ή� .
	// 2018/08/02 �R�� AR.1057175 HW�`�F�b�N���[�h�N�����Ȃ� .
	phys_size_t chk_ram_Size;	/* DramSize for Boot diagnosis */
	unsigned long mon_len;	/* monitor len */
	unsigned long irq_sp;		/* irq stack pointer */
	unsigned long start_addr_sp;	/* start_addr_stackpointer */
	unsigned long reloc_off;
	struct global_data *new_gd;	/* relocated global data */

#ifdef CONFIG_DM
	struct udevice	*dm_root;	/* Root instance for Driver Model */
	struct udevice	*dm_root_f;	/* Pre-relocation root instance */
	struct list_head uclass_root;	/* Head of core tree */
#endif

	const void *fdt_blob;	/* Our device tree, NULL if none */
	void *new_fdt;		/* Relocated FDT */
	unsigned long fdt_size;	/* Space reserved for relocated FDT */
	void **jt;		/* jump table */
	char env_buf[32];	/* buffer for getenv() before reloc. */
#ifdef CONFIG_TRACE
	void		*trace_buff;	/* The trace buffer */
#endif
#if defined(CONFIG_SYS_I2C)
	int		cur_i2c_bus;	/* current used i2c bus */
#endif
#ifdef CONFIG_SYS_I2C_MXC
	void *srdata[10];
#endif
	unsigned long timebase_h;
	unsigned long timebase_l;
#ifdef CONFIG_SYS_MALLOC_F_LEN
	unsigned long malloc_base;	/* base address of early malloc() */
	unsigned long malloc_limit;	/* limit address */
	unsigned long malloc_ptr;	/* current address */
#endif
	struct udevice *cur_serial_dev;	/* current serial device */
	struct arch_global_data arch;	/* architecture-specific data */
} gd_t;
#endif

/*
 * Global Data Flags
 */
#define GD_FLG_RELOC		0x00001	/* Code was relocated to RAM	   */
#define GD_FLG_DEVINIT		0x00002	/* Devices have been initialized   */
#define GD_FLG_SILENT		0x00004	/* Silent mode			   */
#define GD_FLG_POSTFAIL		0x00008	/* Critical POST test failed	   */
#define GD_FLG_POSTSTOP		0x00010	/* POST seqeunce aborted	   */
#define GD_FLG_LOGINIT		0x00020	/* Log Buffer has been initialized */
#define GD_FLG_DISABLE_CONSOLE	0x00040	/* Disable console (in & out)	   */
#define GD_FLG_ENV_READY	0x00080	/* Env. imported into hash table   */
#define GD_FLG_SERIAL_READY	0x00100	/* Pre-reloc serial console ready  */
#define GD_FLG_FULL_MALLOC_INIT	0x00200	/* Full malloc() is ready	   */

#endif /* __ASM_GENERIC_GBL_DATA_H */
----
42:extern struct rom* get_rom(void);
43:struct rom* read_rom(void);
44-
45-#endif /* __MFP_CONFIG_H_ */
board/mvebu/armada8k/km/mfp_config.h:42:extern struct rom* get_rom(void);
board/mvebu/armada8k/km/mfp_config.h:43:struct rom* read_rom(void);

Grep "typedef struct bd_info|struct bd_info" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\include\asm, glob: u-boot.h)

24 lines of output

Bash Locate struct rom, fsload_table and flash_map definitions

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && grep -rn "struct rom *{" --include=*.h --include=*.c . | head; echo ----; grep -rn "struct fsload_table" -B1 -A6 board/mvebu/armada8k/km/machine_setup.h | head -20; echo ----; grep -rn "struct _flash_map" -A10 board/mvebu/armada8k/km/*.h board/mvebu/armada8k/km/userdata.c | head -30

OUT

----
6-// �C���[�W�t�@�C���Ǘ��e�[�u��.
7:struct fsload_table {
8-	char* c_filename;	// �C���[�W�t�@�C����.
9-	char* c_part_num;	// �p�[�e�B�V�����ԍ�.
10-	char* c_idx;		// �C���f�b�N�X(@xxx).
11-};
12-extern unsigned char    uc_TempSubPow;
13-extern unsigned char    uc_TempUtilityKey;
--
21-bool get_hash_value_from_INDEX(uint8_t*, const char*);
22:bool get_hash_value_from_images(struct fsload_table, uint8_t*);
23-
24-#endif  /* __INCmachine_setup */
----
board/mvebu/armada8k/km/userdata.c:58:struct _flash_map{
board/mvebu/armada8k/km/userdata.c-59-	unsigned long start_adr;
board/mvebu/armada8k/km/userdata.c-60-	unsigned long size;
board/mvebu/armada8k/km/userdata.c-61-	unsigned char flag;
board/mvebu/armada8k/km/userdata.c-62-};
board/mvebu/armada8k/km/userdata.c-63-
board/mvebu/armada8k/km/userdata.c-64-/* C�R�}���h�pSPI-Flash�}�b�v. */
board/mvebu/armada8k/km/userdata.c:65:static struct _flash_map flash_map[] = {
board/mvebu/armada8k/km/userdata.c-66-	{0x00000, 0x19000, DEF_DUAL_LAYER},					/* NVRAM�o�b�N�A�b�v�f�[�^1 1�ʖ�.*/
board/mvebu/armada8k/km/userdata.c-67-	{0x19000, 0x19000, DEF_DUAL_LAYER},					/* NVRAM�o�b�N�A�b�v�f�[�^1 2�ʖ�.*/
board/mvebu/armada8k/km/userdata.c-68-	{0x32000, 0x1E000, DEF_DUAL_LAYER},					/* NVRAM�o�b�N�A�b�v�f�[�^2 1�ʖ�.*/
board/mvebu/armada8k/km/userdata.c-69-	{0x50000, 0x1E000, DEF_DUAL_LAYER},					/* NVRAM�o�b�N�A�b�v�f�[�^2 2�ʖ�.*/
board/mvebu/armada8k/km/userdata.c-70-	{0x6E000, 0x04000, DEF_DUAL_LAYER | DEF_SOFTDIPSW},	/* SoftDipSW�f�[�^1�ʖ�.*/
board/mvebu/armada8k/km/userdata.c-71-	{0x72000, 0x04000, DEF_DUAL_LAYER | DEF_SOFTDIPSW},	/* SoftDipSW�f�[�^2�ʖ�.*/
board/mvebu/armada8k/km/userdata.c-72-	{0x76000, 0x01000, DEF_ERASE_ONLY},					/* �d�f���ۑ��̈�.*/
board/mvebu/armada8k/km/userdata.c-73-	{0x77000, 0x01000, DEF_DUAL_LAYER},					/* BSP/Thread�f�[�^1�ʖ�.*/
board/mvebu/armada8k/km/userdata.c-74-	{0x78000, 0x01000, DEF_DUAL_LAYER},					/* BSP/Thread�f�[�^2�ʖ�.*/
board/mvebu/armada8k/km/userdata.c-75-	{0x7B000, 0x01000, 0},								/* HW�`�F�b�N���[�h�p�̈�.*/
--
board/mvebu/armada8k/km/userdata.c:244:static struct _flash_map flash_map_init_card[] = {
board/mvebu/armada8k/km/userdata.c-245-/*	{0x00000, 0x19000, DEF_DUAL_LAYER},*/				/* NVRAM�o�b�N�A�b�v�f�[�^1 1�ʖ�. �N���A���Ȃ� */
board/mvebu/armada8k/km/userdata.c-246-/*	{0x19000, 0x19000, DEF_DUAL_LAYER},*/				/* NVRAM�o�b�N�A�b�v�f�[�^1 2�ʖ�. �N���A���Ȃ� */
board/mvebu/armada8k/km/userdata.c-247-/*	{0x32000, 0x1E000, DEF_DUAL_LAYER},*/				/* NVRAM�o�b�N�A�b�v�f�[�^2 1�ʖ�. �N���A���Ȃ� */
board/mvebu/armada8k/km/userdata.c-248-/*	{0x50000, 0x1E000, DEF_DUAL_LAYER},*/				/* NVRAM�o�b�N�A�b�v�f�[�^2 2�ʖ�. �N���A���Ȃ� */
board/mvebu/armada8k/km/userdata.c-249-	{0x6E000, 0x04000, DEF_DUAL_LAYER | DEF_SOFTDIPSW},	/* SoftDipSW�f�[�^1�ʖ�. */
board/mvebu/armada8k/km/userdata.c-250-	{0x72000, 0x04000, DEF_DUAL_LAYER | DEF_SOFTDIPSW},	/* SoftDipSW�f�[�^2�ʖ�. */
board/mvebu/armada8k/km/userdata.c-251-	{0x76000, 0x01000, DEF_ERASE_ONLY},					/* �d�f���ۑ��̈�. */
board/mvebu/armada8k/km/userdata.c-252-	{0x77000, 0x01000, DEF_DUAL_LAYER},					/* BSP/Thread�f�[�^1�ʖ�. */
board/mvebu/armada8k/km/userdata.c-253-	{0x78000, 0x01000, DEF_DUAL_LAYER},					/* BSP/Thread�f�[�^2�ʖ�. */
board/mvebu/armada8k/km/userdata.c-254-	{0x7B000, 0x01000, 0},								/* HW�`�F�b�N���[�h�p�̈�. */

Bash Find user data offset and SPI flash map details

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && grep -rn "DEF_USERDATA_OFFSET" --include=*.h --include=*.c . | head -8; echo ----; sed -n 60,100p board/mvebu/armada8k/km/userdata.c; echo ----; sed -n 25,60p board/mvebu/armada8k/km/mtd_flash_access.c

OUT

./board/mvebu/armada8k/km/init_debug.h:15:#define THRD_OFFSET_INITDEBUG_INIT_SEC1		(0x07E000 + DEF_USERDATA_OFFSET)	/* �f�o�b�O�p�̈�1 �N���V�[�P���X�p */
./board/mvebu/armada8k/km/init_debug.h:16:#define THRD_OFFSET_INITDEBUG_INIT_SEC2		(0x07F000 + DEF_USERDATA_OFFSET)	/* �f�o�b�O�p�̈�2 �N���V�[�P���X�p */
./board/mvebu/armada8k/km/init_debug.h:17:#define THRD_OFFSET_INITDEBUG_PM_SEC1		(0x07C000 + DEF_USERDATA_OFFSET)	/* �f�o�b�O�p�̈�1 �ȓd�̓V�[�P���X�p */
./board/mvebu/armada8k/km/init_debug.h:18:#define THRD_OFFSET_INITDEBUG_PM_SEC2		(0x07D000 + DEF_USERDATA_OFFSET)	/* �f�o�b�O�p�̈�2 �ȓd�̓V�[�P���X�p */
./board/mvebu/armada8k/km/machine_setup.c:217:		if( 0 != THRF_readFlash( 0x07B000 + DEF_USERDATA_OFFSET, &uc_MSCDLFlg, sizeof(uc_MSCDLFlg) ) ){
./board/mvebu/armada8k/km/mtd_flash_access.c:1001:#define THRD_SoftDipSWBackupArea1Adress	(0x0006E000 + DEF_USERDATA_OFFSET)	// SoftDipSW�̈�o�b�N�A�b�v�F1�ʖڃA�h���X.
./board/mvebu/armada8k/km/mtd_flash_access.c:1002:#define THRD_SoftDipSWBackupArea2Adress	(0x00072000 + DEF_USERDATA_OFFSET)	// SoftDipSW�̈�o�b�N�A�b�v�F2�ʖڃA�h���X.
./board/mvebu/armada8k/km/mtd_flash_access.c:1010:#define THRD_BspThrBackupArea1Adress	(0x00077000 + DEF_USERDATA_OFFSET)	// BSP/Thread�̈�o�b�N�A�b�v�F1�ʖڃA�h���X.
----
	unsigned long size;
	unsigned char flag;
};

/* C�R�}���h�pSPI-Flash�}�b�v. */
static struct _flash_map flash_map[] = {
	{0x00000, 0x19000, DEF_DUAL_LAYER},					/* NVRAM�o�b�N�A�b�v�f�[�^1 1�ʖ�.*/
	{0x19000, 0x19000, DEF_DUAL_LAYER},					/* NVRAM�o�b�N�A�b�v�f�[�^1 2�ʖ�.*/
	{0x32000, 0x1E000, DEF_DUAL_LAYER},					/* NVRAM�o�b�N�A�b�v�f�[�^2 1�ʖ�.*/
	{0x50000, 0x1E000, DEF_DUAL_LAYER},					/* NVRAM�o�b�N�A�b�v�f�[�^2 2�ʖ�.*/
	{0x6E000, 0x04000, DEF_DUAL_LAYER | DEF_SOFTDIPSW},	/* SoftDipSW�f�[�^1�ʖ�.*/
	{0x72000, 0x04000, DEF_DUAL_LAYER | DEF_SOFTDIPSW},	/* SoftDipSW�f�[�^2�ʖ�.*/
	{0x76000, 0x01000, DEF_ERASE_ONLY},					/* �d�f���ۑ��̈�.*/
	{0x77000, 0x01000, DEF_DUAL_LAYER},					/* BSP/Thread�f�[�^1�ʖ�.*/
	{0x78000, 0x01000, DEF_DUAL_LAYER},					/* BSP/Thread�f�[�^2�ʖ�.*/
	{0x7B000, 0x01000, 0},								/* HW�`�F�b�N���[�h�p�̈�.*/
	{0x7C000, 0x01000, DEF_SEQUENCE_DEBUG},				/* BSP�f�o�b�O�p�̈�1.*/
	{0x7D000, 0x01000, DEF_SEQUENCE_DEBUG},				/* BSP�f�o�b�O�p�̈�2.*/
	{0x7E000, 0x01000, DEF_SEQUENCE_DEBUG},				/* BSP�f�o�b�O�p�̈�3.*/
	{0x7F000, 0x01000, DEF_SEQUENCE_DEBUG},				/* BSP�f�o�b�O�p�̈�4.*/
	{ },
};

int do_clearNVdata( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned char    buf[1024];
	int              cnt;
	ALLOC_ALIGN_BUFFER(char, sc_ssdBuff, 512+10, ARCH_DMA_MINALIGN);
	int              i;                     /* 2011/03/09 �Y�� C�R�}���h�ɂ��NV-RAM�o�b�N�A�b�v�̈�N���A�Ή� */
	static char nvramClearFlagData[] = {(char)0xab};

	int i_dev = 0;
	struct mmc *pst_mmc = find_mmc_device(i_dev);

	unsigned long start = 0, size = 0;
	unsigned long count1 = 0, count2 = 0;

	/* �e�[�u���ɏ]����SPI-Flash�����N���A����. */
	for(i = 0; i < ARRAY_SIZE(flash_map) - 1; i++){
		start = flash_map[i].start_adr + DEF_USERDATA_OFFSET;
		size = flash_map[i].size;
----
#define _256K       0x00040000
#define _512K       0x00080000
#define _1M			0x00100000

#define	THRD_SPI_FlashSectorSize		(_4K)						// SPI-Flash�̃Z�N�^�T�C�Y��4KB.
#define THRD_SPI_FlashAdminCountSize	(sizeof(uint32_t))		// �Ǘ��p�J�E���^�T�C�Y(4byte).
#define	THRD_SPI_FlashEraseCountVal		(0xFFFFFFFF)				// SPI-Flash�C���[�X���̃J�E���^�l.

#define THRD_SigOffset					0x3fd000					// Boot�Ō��؂��鏐���̃x�[�X�A�h���X
#define THRD_SigSize					0x100						// Boot�Ō��؂��鏐��1�̃T�C�Y.
#define THRD_SecureInfoOffset			0x3fe000					// �Z�L���A���̈�̃A�h���X.

#define THRD_BOOT_ROM_SIZE				3 * _1M						// BootROM�T�C�Y.
#define THRD_PrBootOffset				0							// Primary Boot�̃I�t�Z�b�g.
#define THRD_GmBootOffset				0x400000					// GoldenMaster Boot�̃I�t�Z�b�g.

#define THRD_LPPP_ROM_SIZE				2 * _1M						// LPPPROM�T�C�Y.
#define THRD_PrLpppOffset				0							// Primary LPPP�̃I�t�Z�b�g.
#define THRD_GmLpppOffset				0x400000					// GoldenMaster LPPP�̃I�t�Z�b�g.

#define THRD_FLASH_CHIP_SIZE			0x800000					// Secure�pFlashChip�T�C�Y.

/*================================================================================================
�m�@�\�n	mtd��mtd�\���̂��擾.
--------------------------------------------------------------------------------------------------
�m�����n	
�m�ߒl�n	mtd�\���̃A�h���X
�m�����n	2017/10/11 Sky S800 QSPI Flash�Ή�
================================================================================================*/
static struct mtd_info *getSPIFlash( void )
{
	return get_mtd_device(NULL, 0x0);
}

/*================================================================================================
�m�@�\�n	Flash�ɃA�N�Z�X�\���`�F�b�N����.

Bash Find DEF_USERDATA_OFFSET value and mtd device usage

IN

cd "C:/Users/PC/Documents/IT6_Kernel/BootROM/BootROM/Emu800/Src" && grep -rn "define[[:space:]]\+DEF_USERDATA_OFFSET\|define[[:space:]]\+THRD_SPI_OFFSET\|DEF_DUAL_LAYER[[:space:]]" --include=*.h --include=*.c . | head -8; grep -rn "mtd_read_id_with_size" -A25 drivers/mtd/mtdcore.c | head -40; grep -n "get_mtd_device" board/mvebu/armada8k/km/mtd_flash_access.c | head

OUT

./board/mvebu/armada8k/km/mtd_flash_access.h:14:#define DEF_USERDATA_OFFSET		0x300000	/* User Data�̈�I�t�Z�b�g */
./board/mvebu/armada8k/km/mtd_flash_access.h:42:#define THRD_SPI_OFFSET_RESTART_FLG		0x0000
./board/mvebu/armada8k/km/userdata.c:51:#define DEF_DUAL_LAYER		0x01	/* 2�ʐ���p�t���O. */
./board/mvebu/armada8k/km/userdata.c:70:	{0x6E000, 0x04000, DEF_DUAL_LAYER | DEF_SOFTDIPSW},	/* SoftDipSW�f�[�^1�ʖ�.*/
./board/mvebu/armada8k/km/userdata.c:71:	{0x72000, 0x04000, DEF_DUAL_LAYER | DEF_SOFTDIPSW},	/* SoftDipSW�f�[�^2�ʖ�.*/
./board/mvebu/armada8k/km/userdata.c:123:		if( flash_map[i].flag & DEF_DUAL_LAYER ){
./board/mvebu/armada8k/km/userdata.c:249:	{0x6E000, 0x04000, DEF_DUAL_LAYER | DEF_SOFTDIPSW},	/* SoftDipSW�f�[�^1�ʖ�. */
./board/mvebu/armada8k/km/userdata.c:250:	{0x72000, 0x04000, DEF_DUAL_LAYER | DEF_SOFTDIPSW},	/* SoftDipSW�f�[�^2�ʖ�. */
1199:int mtd_read_id_with_size(struct mtd_info *mtd)
1200-{
1201-	int	ret = 0;
1202-	u8 id[SPI_NOR_MAX_ID_LEN] = "";
1203-	int num_read = 0;
1204-	const struct flash_info	*info;
1205-
1206-	ret = mtd->_read_reg(mtd, SPINOR_OP_RDID, id, SPI_NOR_MAX_ID_LEN);	// Read JEDEC ID.
1207-	if (ret < 0) {
1208-		printf("error reading JEDEC ID\n");
1209-		return ret;
1210-	}
1211-
1212-	for ( num_read=0; num_read < ARRAY_SIZE(spi_nor_ids)-1; num_read++) {
1213-		info = &spi_nor_ids[num_read];
1214-		if (info->id_len) {
1215-			if(memcmp(info->id, id, info->id_len) == 0) {
1216-				if( info->name != NULL ) {
1217-					printf("%s:0x%x\n", info->name, (info->sector_size * info->n_sectors));
1218-				}
1219-				return (info->sector_size * info->n_sectors);
1220-			}
1221-		}
1222-	}
1223-
1224-	return 0;
--
1226:EXPORT_SYMBOL_GPL(mtd_read_id_with_size);
1227-
1228-/* spi_nor_wait_till_ready
1229- * struct mtd_info *mtd MTD Data
1230- * Wait until the setting value is changed
1231- */
1232-static int spi_nor_wait_till_ready(struct mtd_info *mtd)
1233-{
1234-	int timeout = BUSY_TIMEOUT;
1235-	int ret = 0;
1236-	u8 val = 0;
1237-
1238-	do {
56:	return get_mtd_device(NULL, 0x0);
1610:	flash = get_mtd_device(NULL, 0x0);
1706:	flash = get_mtd_device(NULL, 0x1);

Đính chính bản đồ SPI-flash (ảnh hưởng Phần 12, 21, 31, 33, 35): các offset `0x6E000`, `0x72000`, `0x77000`, `0x78000`, `0x79000`, `0x7A000`, `0x7B000` mà tôi đã viết là **tương đối so với `DEF_USERDATA_OFFSET = 0x300000`** (`mtd_flash_access.h:14`, mọi truy cập cộng offset này, ví dụ `THRD_SoftDipSWBackupArea1Adress = 0x6E000 + DEF_USERDATA_OFFSET`). Địa chỉ tuyệt đối đúng là:

|Vùng|Tương đối|Tuyệt đối trong flash `mtd0`|
|---|---|---|
|Primary Boot (ATF+U-Boot+driver Warp tại 0x2B0000)|—|`0x000000–0x2FFFFF` (3 MiB)|
|NVRAM backup 1 (hai mặt) / 2 (hai mặt)|0x0, 0x19000 / 0x32000, 0x50000|0x300000, 0x319000 / 0x332000, 0x350000|
|SoftDIP mặt 1 / 2 (0x4000 mỗi mặt)|0x6E000 / 0x72000|**0x36E000 / 0x372000**|
|Vùng "điện ngắt" (chỉ xoá)|0x76000|0x376000|
|BSP/Thread mặt 1 / 2|0x77000 / 0x78000|**0x377000 / 0x378000**|
|Vùng HW-check / cờ MSC|0x7B000|**0x37B000**|
|Debug BSP 1–4|0x7C000…0x7F000|0x37C000…0x37F000|
|Kết quả huấn luyện LPDDR4|—|0x380000 (cửa sổ 0xF4380000)|
|Chữ ký ×5|—|0x3FD000|
|Secure info|—|0x3FE000|
|GoldenMaster Boot|—|0x400000–0x6FFFFF (3 MiB); cửa sổ QSPI 7 MiB kết thúc đúng tại 0x700000|

Ngoài ra có **flash thứ hai** `mtd1` (`get_mtd_device(NULL,1)`, mtd_flash_access.c:1706) cho **LPPP/R4**: Primary LPPP tại 0 (2 MiB), GM LPPP tại 0x400000 (`THRD_GmLpppOffset`); `THRG_updateGmLPPP` sao Primary→GM trên flash này. Chip phải ≥ 8 MiB (`THRD_FLASH_CHIP_SIZE`), nếu không "No Area for GM". Vùng offset trong mỗi mặt "dual layer" có bộ đếm 4 byte (`THRD_SPI_FlashAdminCountSize`) nên lệnh `C` (`do_clearNVdata`) xoá theo bảng `flash_map[]`.

# PHẦN 36 (PART 37): Important Structures

## 0. Kết luận

Các cấu trúc quan trọng chia thành sáu nhóm: **trạng thái toàn cục**, **lệnh/shell**, **ảnh/boot**, **thiết bị/driver**, **cấu trúc riêng KM**, **giao thức với bộ vi xử lý phụ**. Chỉ liệt kê các trường có ý nghĩa cho hành vi đã phân tích.

## 1. Trạng thái toàn cục

### `gd_t` (`include/asm-generic/global_data.h:24` + `arch_global_data` trong `arch/arm/include/asm/global_data.h:24`)

Con trỏ nằm ở **`x18`** (`register volatile gd_t *gd asm("x18")`, `-ffixed-x18`).

|Trường|Ý nghĩa|Ghi chú|
|---|---|---|
|`bd`|con trỏ `bd_t`|`bi_dram[0..2]`, `bi_boot_params`|
|`flags`|`GD_FLG_RELOC 0x1`, `DEVINIT 0x2`, `SILENT 0x4`, `ENV_READY 0x80`, `SERIAL_READY 0x100`, `FULL_MALLOC_INIT 0x200`…||
|`baudrate`, `cpu_clk`, `bus_clk`, `mem_clk`|||
|`fb_base`|framebuffer (VRAM)|do `reserve_lcd` (0x3FE10000)|
|`have_console`, `precon_buf_idx`|pre-console buffer (1 MiB @0x2000000)||
|`env_addr`, `env_valid`|env|Phần 14|
|`ram_top`, `relocaddr`, `ram_size`, **`chk_ram_Size`** [KM], `mon_len`, `irq_sp`, `start_addr_sp`, `reloc_off`, `new_gd`|bố cục RAM/relocation|`chk_ram_Size` = DRAM đọc từ MC (dùng bởi HW-check)|
|`fdt_blob`, `new_fdt`, `fdt_size`|DTB điều khiển và bản relocate||
|`jt`|jump table (exports)||
|`env_buf[32]`|bộ đệm `getenv` trước ENV_READY||
|`cur_i2c_bus`|bus I2C hiện tại|`CONFIG_SYS_I2C`|
|`malloc_base/limit/ptr`|malloc sớm 0x5000||
|`cur_serial_dev`|(DM, không dùng)||
|`arch`|`arch_global_data`|dưới|

`arch_global_data`: `timer_rate_hz`, `tbu/tbl/lastinc`, `timer_reset_value`, **`tlb_addr`, `tlb_size`** (bảng trang), `soc_family`, `board_family`, `reg_base`, `local_sys_info[]` (nếu `MVEBU_SYS_INFO`), và các trường KM: **`printBuffer[PRINT_BUFFER_NUM][CONFIG_SYS_PBSIZE]`** (255 × 1051 ≈ 268 KB), `printBufferNum`, `printStopFlg`, `uc_HWcheckMode_Dimm`, **`termFlg`**, **`term_sel`** (1 = AP-UART, 0 = CSRC).

Hệ quả: `gd` ≈ 0x418C8 byte, đặt trên stack ban đầu 0xFF1000 (Phần 3); ảnh sao chép `gd` sang RAM cao (`new_gd`) sau relocation.

### `bd_t`

`bi_dram[CONFIG_NR_DRAM_BANKS=3]` (`start`, `size`), `bi_boot_params`; `bi_dram[0]` dùng bởi `arch_lmb_reserve`, `booti_setup`, `getenv_bootm_size`. (Định nghĩa gốc nằm trong `asm-generic/u-boot.h` vì `SYS_GENERIC_BOARD`; ở đây chỉ nêu trường có dùng [S].)

## 2. Lệnh và shell

### `cmd_tbl_t` (`include/command.h:30`)

`name`, `maxargs`, `repeatable`, `cmd` (con trỏ `do_xxx(cmdtp, flag, argc, argv[])`), `usage`, `help` (LONGHELP), `complete` (AUTO_COMPLETE). Được sinh bằng `ll_entry_declare` vào `.u_boot_list_2_cmd_2_<name>`, căn 4 byte.

### Môi trường

`env_htab` (`struct hsearch_data` với `change_ok = env_flags_validate`), `default_environment[]` (chuỗi `k=v\0…\0\0`), `env_id`.

### hush

`struct in_str`, `struct child_prog`, `struct pipe`, `struct variables *top_vars` (biến cục bộ, `HUSH_VERSION=0.01` chỉ đọc).

## 3. Ảnh và boot

|Cấu trúc|Vị trí|Trường chính|
|---|---|---|
|`image_header_t` (64 B, BE)|image.h:264|`ih_magic 0x27051956`, `ih_hcrc`, `ih_time`, `ih_size`, `ih_load`, `ih_ep`, `ih_dcrc`, `ih_os`, `ih_arch`, `ih_type`, `ih_comp`, `ih_name[32]`|
|`struct Image_header` (ARM64)|cmd_bootm.c:634|`code0/1`, `text_offset`, `image_size`, `res1..4`, `magic 0x644d5241`, `res5`|
|`bootm_headers_t` (`images`)|image.h:291|`legacy_hdr_os`, `legacy_hdr_os_copy`, `legacy_hdr_valid`, `os` (`image_info_t`: `start,end,image_start,image_len,load,comp,type,os,arch`), `ep`, `rd_start/rd_end`, `initrd_start/end`, `ft_addr/ft_len`, `lmb`, `verify`, `state`|
|`struct lmb`|lmb.h|vùng `memory` và `reserved` (dùng trong `boot_start_lmb`, `lmb_alloc_base`, `lmb_reserve`)|
|FDT (`fdt_header`)|libfdt|magic `0xd00dfeed`, `totalsize`, offsets|
|`bootstage`|bootstage.h|báo cáo thời gian (`bootstage_report`)|

## 4. Thiết bị / driver

|Cấu trúc|Ghi chú|
|---|---|
|`struct mmc`, `struct mmc_cmd`, `struct mmc_data`|MMC core|
|`struct xenon_*`|Xenon SDHCI (DT `mmc@e8278000`)|
|`struct nvme_dev`, `nvme_queue`, `nvme_command`, `nvme_info[]`|NVMe KM; `nvme_info[dev].pdev`; `struct opal_dev`, `opal_security_info` (OPAL)|
|`struct pci_controller`, `pci_device_id`|PCI; whitelist `st_nvme_supported[]`|
|`struct usb_device`, `usb_dev[USB_MAX_DEVICE]`, `struct xhci_ctrl xhcic[]`, `xhci_hccr/hcor`|USB; KM thêm `level`, `devpath`, `route`|
|`struct mtd_info`, `spi_nor_ids[]`, `flash_info`|MTD/QSPI|
|`struct i2c_adapter`, `struct i2c_regs`|DW-I2C|
|`struct eserial_device`/`stdio_dev`, `struct udevice` (không dùng)|Serial/console|
|`struct hsearch_data`, `struct env_entry`||
|`sha256_context`|SHA-256 (tự chẩn đoán)|

## 5. Cấu trúc riêng KM

|Cấu trúc|Vị trí|Trường / Ý nghĩa|
|---|---|---|
|`struct fsload_table`|machine_setup.h:7|`c_filename`, `c_part_num`, `c_idx` (khoá `@SUB`, `@KN1`, `@IUB`, `@DB1`, `@RF1`, `@RF3`) cho kiểm hash `EXP_INDEX`|
|`struct _flash_map`|userdata.c:58|`start_adr`, `size`, `flag` (`DEF_DUAL_LAYER 0x01`, `DEF_SOFTDIPSW`, `DEF_ERASE_ONLY`, `DEF_SEQUENCE_DEBUG`) — bảng xoá của lệnh `C`/Initialize card|
|`struct rom`|(không tìm được định nghĩa trong repo bằng grep `struct rom *{`)|có `mac[0][0..5]`, `machineInfo[0]`, `machineNo[0]`, serial (dùng ở `misc_init_r_env`, `init_km`) [?]|
|`enum THRE_SIG`, `THRD_SECAREA`|mtd_flash_access.h:45-59|khe chữ ký và offset secure info|
|Mã trạng thái SPI `THRD_SET_*`, `THRD_GET_*`|:21-39|Phần 29|
|`struct warp_boot`, `warp_area`, `warp_drv_info`, `warp_savearea`, `warp_bg`|include/warp.h|`warp_boot`: `cpu_no, switch_mode, silent, console, bps, drv_total_size, v2p_offset, …, reserve[32]`; `warp_area`: `dev, size, offs`; `warp_drv_info`: `mode` (INVALID/FIXED/FLOATING), `addr`, `drv`|
|Header driver Warp (offset)|warp.h:6-15|`ID 0x00`, `VERSION 0x04`, `CAPS 0x08`, `LOWMEM_END 0x0C`, `TEXT_END 0x10`, `DATA_END 0x14`, `BSS_END 0x18`, `SNAPSHOT 0x20`, `HIBERNATE 0x28`, `SWITCH 0x30`; ID `W5HD 0x44483557`, bootflag `W5BF 0x46423557`, UserAPI `W3UA`|
|Mã thiết bị Warp|warp.h:29-40|`WARP_DEV(dev,lun,part)`: NOR 1, NAND 2, ATA 3, SD 4, MEM 5, SPI 6, NVME 7, USER 0x7e, EXT 0x7f (`dev<<16|
|`struct KM Panel: PANEL_BOOTDIAG_*`|mfp_panel.h:56-65|ST0–ST5, `CPU 0x01`, `BASE 0x02`, `STORAGE 0x04`|
|`g_action`, `AUTOBOOT_*`, `TERM_STATE_*`|action.h, main.c|Phần 16, 34|

## 6. Giao thức với bộ vi xử lý phụ

|Cấu trúc|Kích thước|Trường|
|---|---|---|
|`scpi_header_t` (arm_scpi.c:71)|8 B + payload|`command_id`(u8), `sender_if`(u8), `payload_size`(u16), `status`(u32), `payload`(u32, bắt đầu payload)|
|WDT payload (ca72wdt.c:9)|256 B|`event`(u32), `point`(u32), `timeout`(u32), `debugInfo1`(u32), `debugInfo2[240]`|
|`MHU_regs_t`|0x18|`RX_INT_STATUS/SET/CLEAR`, `TX_INT_STATUS/SET/CLEAR`|
|Khung panel|8 B + chk|`{0xFE, 0x06, 0x30, LED1, LED2, 0, 0, 0xFF}`|
|Cấu trúc kênh SCPI|128 B/ô|AP→SCP tại `base + (2n+1)·128`; SCP→AP tại `base + 2n·128`|

## 7. Khe hở và rủi ro cấu trúc

1. **`gd_t` khổng lồ** (bộ đệm log 268 KB) buộc SP ban đầu phải chịu; `SPL` không thể dùng (SRAM 156 KB) (Phần 4).
2. **`scpi` payload 256 B vào ô 128 B** (Phần 22).
3. **`struct rom` không có định nghĩa ở nơi tìm thấy**: dữ liệu đọc thô, kích thước tra bằng macro `BOOT_ROM_*` [?].
4. **`printBuffer` đo theo `CONFIG_SYS_PBSIZE`** = `CBSIZE + sizeof(PROMPT) + 16` = 1024 + 11 + 16 = 1051.
5. `cmd_tbl_t` căn 4 nhưng chứa con trỏ 64-bit: khoảng cách giữa các mục không phải bội 8 (có thể gây truy cập con trỏ không căn 8 khi duyệt bảng; an toàn nhờ `-mstrict-align` và kích thước cấu trúc bội 8) [S].

## 8. COMMON CONFUSION

1. **"`gd` là biến toàn cục thường"**: là thanh ghi `x18`.
2. **"`bd_t` là cấu trúc ARM"**: dùng bản generic.
3. **"`image_header_t` áp cho kernel Image"**: chỉ cho initrd (uImage); kernel dùng `Image_header` ARM64.
4. **"Cờ Warp `dev` là số hiệu thiết bị Linux"**: là mã nội bộ Warp.

## 9. MENTAL MODEL

```
x18 ─► gd_t ─► { bd_t(bi_dram), flags, env_*, fdt_blob, malloc_*, arch{ tlb, printBuffer, termFlg } }
.u_boot_list ─► cmd_tbl_t[]
images (bootm_headers_t) ─► os/ep/rd/ft + lmb
KM: g_action, warp_*, THRD_*, flash_map[], fsload_table[]
SCPI: scpi_header_t + payload 256B
```

## 10. Chưa xác minh

- Định nghĩa `struct rom`, `struct pt_regs`, `bd_t` generic.
- Layout `nvme_*`, `xhci_*` (chuẩn theo spec/upstream, không đọc lại).
- `image_info_t` đầy đủ.

---

Xong Phần 36. Tiếp theo **Phần 37: Concurrency / async**: mô hình luồng, race giữa CPU và thiết bị phụ, vai trò ngắt, cấu trúc dùng chung với R4/PS-CPU.

Grep "volatile|static [a-z_ ]+ [a-z_]+\s*=|CONFIG_SMP|boot_cpu|kill_cpu|CONFIG_WARP_HIBDRV_BG|CONFIG_WARP_AMP" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\include\configs, glob: {armada8k.h,mvebu-common.h})

1 line of output

Grep "nvme_wait_completion_queue|ADMIN_TIMEOUT|NVME_.*TIMEOUT|phase|cq_phase|BUSY_TIMEOUT|udelay|mdelay|get_timer" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\block\nvme.c)

31 lines of output

# PHẦN 37 (PART 38): Concurrency / Async

## 0. Kết luận

U-Boot là **chương trình đơn luồng, đơn lõi, không ngắt, không scheduler**. Mọi "đồng thời" đến từ **các tác nhân ngoài** cùng chạy song song với CPU: bộ chủ DMA (NVMe, xHCI, SDHCI, CDMA, LCD), bộ vi xử lý phụ **R4/LPPP (SCP)**, **PS-CPU**, vi điều khiển **panel**, và sau bàn giao là **Linux + ATF**. U-Boot đồng bộ với chúng bằng **polling có timeout, cờ handshake, bảo trì cache và một giao thức "tạm dừng R4"**. Không có mutex/spinlock/atomic nào trong mã KM.

## 1. Danh sách tác nhân đồng thời

|Tác nhân|Chia sẻ với U-Boot|Cơ chế đồng bộ hiện có|
|---|---|---|
|Lõi CA72 phụ (3 lõi)|không (ATF giữ PSCI; vòng `slave_cpu` ở `start.S:113` chỉ chạy nếu lõi phụ vào `_start`)|Không cần|
|**NVMe controller**|RAM (SQ/CQ/PRP/dữ liệu), thanh ghi BAR|D-cache **tắt** toàn cục; phase bit của CQ; polling `nvme_wait_completion_queue` (admin timeout 60 s)|
|xHCI|RAM (ring/context), thanh ghi|`flush/invalidate_dcache_range` trong `xhci-mem.c/ring.c`; polling|
|Xenon SDHCI|RAM (SDMA/bounce)|`flush` trước lệnh; polling|
|CDMA (âm thanh)|RAM (mô tả 64B, dữ liệu)|flush mô tả; `cdma_is_busy` polling; `sound_stop` trước boot|
|Bộ điều khiển LCD|VRAM 0x3FE10000|Không flush tường minh (Phần 23) [?]|
|**R4 (LPPP/SCP)**|SPI-flash (`mtd0`… khu vực thực thi/dữ liệu), hộp thư `0x7FFF0000`, watchdog|`pause_r4` (ghi/xoá SPI), MHU handshake (WDT), `flush 64B`/`invalidate`|
|**PS-CPU**|I2C0 (0x3E), sự kiện|Giao dịch I2C tuần tự, không khoá; kết quả không kiểm|
|Vi điều khiển panel|UART COM2|Khung có checksum, `SendData/RecvData` đồng bộ|
|ATF BL31|RAM 0x10000000 (4 MB), SMC|Không dùng SMC; không chạm vùng|
|Linux (sau bàn giao)|NVMe đang bật, mailbox, R4, GPIO, SPI|Không có (U-Boot đã thoát); xem mục 6|

## 2. Mô hình thực thi

- **Đơn tuyến tính**: `_start → _main → board_init_f → board_init_r → main_loop` (vòng vô hạn). Không có tác vụ nền. Hàm nào chạy lâu thì chặn tất cả.
- **Không ngắt** (Phần 24): `interrupt_init/enable/disable` rỗng; DAIF che. Sự kiện ngoài chỉ được thấy khi U-Boot **tự hỏi** (polling).
- **Ctrl-C** cũng là polling: các lệnh dài gọi `ctrlc()` (kiểm `tstc()` trên UART).
- **Không có thread-local/atomic**; biến `static` dùng như "trạng thái riêng" (xem 4).

## 3. Kỹ thuật đồng bộ cụ thể trong mã

### 3.1 Polling có timeout

|Nơi|Cơ chế|Timeout|
|---|---|---|
|`nvme_wait_completion_queue`|so `cq_phase` mỗi vòng, `get_timer(0) >= endtime`|`ADMIN_TIMEOUT 60` s; `NVME_CAP_TIMEOUT` (theo CAP); `SHUTDOWN_TIMEOUT`|
|`send_cmd` (MHU)|vòng `mdelay(1)` kiểm `TX_INT_STATUS` bit1 rồi `RX_INT_STATUS` bit1|1000 ms + 1000 ms|
|`wait_for_LPPPintoWFI`|`udelay(100)` × 1000|100 ms|
|`dw_i2c_read/write`|`I2C_BYTE_TO`|theo byte|
|`phy_utm_mv_init`|3 vòng `while`|**không timeout**|
|`usb` (xHCI ring)|timeout theo `usb_control_msg` (ms)||
|`abortboot`|đếm 1 s/lần với `get_timer`|bootdelay|
|`cdma_stop`|`udelay(1)` lặp|có `printf("cdma busy check timeout")`|

### 3.2 Handshake MHU/SCPI (WDT, pause)

```
AP: ghi payload vào 0x7FFF0180, flush 64 B, TX_INT_SET ← 2
R4: nhận, xử lý, xoá TX_INT bit1 (báo "đã lấy"), rồi đặt RX_INT bit1 (báo "phản hồi")
AP: thấy TX bit1 = 0 ⇒ R4 đã đọc ; thấy RX bit1 = 1 ⇒ xong ; clear RX bit1
```

Thứ tự và bộ nhớ: `flush_dcache_range` trước `write_MHU`; sau đó đọc `invalidate_dcache_range(mhu_regs …)` (vùng thanh ghi là Device nên không cần nhưng không hại). Không có `dsb` giữa flush và ghi MHU ngoài `dsb sy` trong `__asm_flush_dcache_range` (cache.S:144).

### 3.3 `pause_r4` như "khoá" thô

```
pause_r4(true):  nếu current_state++ ≠ 0 thì return          ← đếm lồng (reentrancy)
                 gửi lệnh 0x0A payload 0x100 ; chờ R4 WFI (≤100 ms)
pause_r4(false): nếu --current_state ≠ 0 thì return
                 *(0xE800800C) = 1                            ← thả R4
```

Đảm bảo R4 **không truy cập SPI-flash trong khi U-Boot ghi/xoá** (các cặp ở `qspi_winbond.c` 262/280, 468/541, 581/638, 704/769, 1227/1387). Đây là cơ chế loại trừ tương hỗ duy nhất, dựa trên hợp tác của R4. Điểm yếu: (1) timeout WFI chỉ in cảnh báo; (2) **đọc** SPI qua cửa sổ MMIO `0xF4000000` không được bao bởi `pause_r4` [chưa xác nhận R4 có truy cập đồng thời] ; (3) trong lúc R4 bị dừng, **WDT do R4 phụ trách cũng đứng** (Phần 22).

### 3.4 Cache như một cơ chế đồng bộ

- NVMe: tắt D-cache (không cần bảo trì cache).
- Các thiết bị khác: flush trước khi thiết bị đọc, invalidate trước khi CPU đọc dữ liệu thiết bị ghi.
- `invalidate_dcache_range` thực chất là clean+invalidate (Phần 23): không mất dữ liệu CPU nhưng có thể ghi đè dữ liệu DMA nếu đệm còn dirty.

## 4. Trạng thái tĩnh dùng chung (không tái nhập)

|Biến/đệm|Hàm|Rủi ro|
|---|---|---|
|`static char tmp[768]`|`make_bootcmds`|trả con trỏ tĩnh; gọi liên tiếp hai lần sẽ ghi đè kết quả trước (mã dùng ngay bằng `setenv` nên an toàn)|
|`static char chBtEv[768]`|`make_bootargs`|như trên; `sprintf` không giới hạn|
|`static struct opal_dev g_dev`, `g_info`|`nvme_opal_cmd_send`|dùng chung|
|`static CDMA_AXI_DESC4_REGS_t st_cdma_desc`|`cdma_start`|một mô tả, một lần một luồng|
|`static int count`|`initr_wd_refresh`|bộ đếm mã `point`|
|`char gc_stopflg`|`BspPrintStopCheck`|chống gọi lồng của log (in trong `getTermState`)|
|`static bool bl_first/bl_chip`|`sysBspSecureChip`|cache kết quả PIO lần đầu|
|`static int current_state`|`pause_r4`|đếm lồng|
|`static unsigned char uc_Flag[2]`|`reset_with_recoverycode`||
|`usb_dev[]`, `dev_index`, `asynch_allowed`|usb.c|không thread-safe|
|`hush`: `top_vars`, `HUSH_NO_EVAL`…|||

Trong ngữ cảnh đơn luồng không ngắt, các biến này an toàn; chúng chỉ nguy hiểm nếu sau này có ngắt hoặc SMP.

## 5. Các điểm dễ sinh "race" do thiết bị bên ngoài

1. **Ghi SPI-flash bởi hai chủ**: U-Boot và R4. Bảo vệ bằng `pause_r4` nhưng chỉ cho ghi/xoá/ID.
2. **Vùng hộp thư dùng chung** với Linux/ATF/R4: U-Boot ghi 264 B vào ô 128 B (Phần 22), có thể ghi đè ô kề nếu R4/Linux dùng kênh 2.
3. **VRAM**: CPU vẽ (ghi qua cache), bộ điều khiển LCD quét: nếu không flush thì màn hình có thể hiện dữ liệu cũ [?].
4. **NVMe còn chạy** khi kernel bắt đầu (Phần 18): controller có thể vẫn DMA vào RAM cũ (hàng đợi của U-Boot) cho tới khi driver Linux reset nó. U-Boot chỉ đọc đồng bộ (không ghi async) nên không có lệnh treo, thực tế nguy cơ thấp, nhưng buffer PRP/CQ của U-Boot nằm ở vùng heap có thể bị Linux tái sử dụng.
5. **Thời gian WDT vs timeout thiết bị**: một chờ dài hơn khoảng WDT kế tiếp sẽ khiến R4 reset máy (ví dụ NVMe admin 60 s > Warp/SuperWarp 90/40 s; USB PHY vô hạn; `mdelay(10000)` USB trong 120 s thì ổn).
6. **Cờ dùng chung với Linux** (SoftDIP, BSP/Thread, secure info, snapshot): **không có khoá liên tiến trình**; an toàn nhờ tính chất "một chủ tại một thời điểm" (Linux đã tắt khi U-Boot chạy) và cấu trúc hai mặt + bộ đếm cho phép phục hồi.

## 6. Sau bàn giao: những gì còn chạy song song với Linux

- R4 và WDT 300 s (giao thức chung `scp-shmem`).
- NVMe ở trạng thái đã init (không shutdown).
- GPIO/pinmux ở trạng thái U-Boot đặt.
- Các vùng SPI-flash do Linux ghi tiếp (SoftDIP, snapshot).
- Không còn tác nhân nào của U-Boot (VBAR vẫn trỏ vào U-Boot cho tới khi kernel đặt lại).

## 7. Failure

|Điều kiện|Hậu quả|
|---|---|
|R4 không đáp `pause_r4`|in `[SCPI] wfi timeout !!!`, vẫn tiếp tục ghi flash (nguy cơ hai chủ cùng ghi)|
|`pause_r4(true)` lồng nhưng lỡ bỏ `pause_r4(false)`|R4 bị dừng vĩnh viễn (đếm lệch)|
|Thiết bị không hoàn tất|Timeout, lệnh thất bại hoặc treo (PHY)|
|NVMe bị bỏ dở|Linux reset controller|

## 8. COMMON CONFUSION

1. **"U-Boot có thread/timer"**: không.
2. **"`volatile`/cache đủ để đồng bộ với R4"**: cần thêm handshake MHU.
3. **"`static` = thread-safe"**: chỉ an toàn vì đơn luồng.
4. **"WDT chạy song song trong U-Boot"**: nó chạy ở R4 và chỉ được gia hạn tại các điểm gọi.

## 9. MENTAL MODEL

```
CPU (1 luồng, polling) ─┬─ DMA masters (NVMe/xHCI/SD/CDMA/LCD) : đồng bộ bằng cache maintenance / tắt D-cache / phase bit
                        ├─ R4 (SCPI/MHU, pause_r4)            : handshake + tạm dừng khi ghi SPI
                        ├─ PS-CPU (I2C)                       : giao dịch tuần tự
                        └─ Panel MCU (UART)                    : khung + checksum
Không có ngắt, không khoá, không thread.
```

## 10. Chưa xác minh

- R4 có truy cập SPI khi đọc qua cửa sổ MMIO không.
- Tên/chức năng `0xE800800C`.
- Mức coherency LCD.

---

# PHẦN 38 (PART 39): Performance

## 0. Kết luận

Không có phép đo thực trong repo. Dưới đây là **các yếu tố thời gian đã xác minh trong mã** và ước lượng bậc độ lớn (đánh dấu [S] hoặc [?] khi không đo được). Điểm nghẽn có căn cứ lớn nhất là: **(1)** `bootdelay` 3 s cộng thời gian chờ đường USB, **(2)** **chạy sau `nvme init` với MMU/D-cache tắt** (nạp `Image`, chép initrd, CRC32 initrd đều không cache), **(3)** các lệnh SCPI WDT đồng bộ, **(4)** thời gian khởi tạo NVMe/PCIe. Máy còn có đường **Warp** để bỏ qua toàn bộ boot lạnh.

## 1. Bảng các khoản thời gian đã biết

|Khoản|Giá trị/nguồn|Điều kiện|
|---|---|---|
|Đếm ngược autoboot|`bootdelay` = 3 s (armada8k.h:132); = 0 nếu `isWarpBootMode()`; nếu có phím thì bỏ qua|mọi lần boot thường|
|Chờ USB thử lại|`mdelay(10000)` một lần|chỉ khi giữ Utility Key/line-card và không thấy USB|
|`mdelay(4500)`|IISW/UPDATE/BACKUP/RESTORE (relay M/C)|các chế độ cập nhật|
|Lệnh SCPI WDT|≥ vài ms mỗi lệnh (`mdelay(1)` theo vòng), ≤ 2 s khi R4 không đáp; khoảng **25–35 lệnh/lần boot** (16 điểm trong `init_sequence_r` + các điểm ở main/autoboot/USB)|mọi lần boot|
|Khởi tạo NVMe|PCIe link, enable controller (chờ `CAP.TO`), Identify, tạo I/O queue; admin timeout tối đa 60 s|mỗi `nvme init`|
|Khởi tạo SD/eMMC|`mmc rescan` (cmd chu trình)|nhánh SD|
|Nạp kernel/initrd/DTB|`fatload` sector-by-sector nếu buffer không căn; NVMe polling; **MMU tắt (Device-nGnRnE)**|mỗi boot (SSD)|
|`memmove` Image (0x800000 → text_offset)|`image_size` byte|không cache|
|CRC32 initrd|toàn bộ `ih_size`, `verify=-1` ⇒ luôn chạy|mỗi boot|
|Chép initrd lên cao|`memmove_wd` (CHUNKSZ) cỡ initrd|mỗi boot|
|Giải nén ảnh khởi động|LZO/PNG (`DrawPanel_lzo`, `graphlib`)|khi vẽ màn hình|
|`flush_dcache_all` + LLC clean-inv|set/way L1+L2 + LLC toàn bộ|mỗi lần tắt D-cache (1 lần)|
|`chk_sboot`, `read_rom`, đọc SoftDIP|vài lần đọc SPI qua MMIO|mọi boot|
|Tự chẩn đoán HW|6 tệp hash SHA-256 (gồm RFS1.img/RFS3.img lớn) nạp vào 0x800000|chỉ chế độ tự chẩn đoán|
|`init_km`, `init_Panel`|UART panel, VRAM, LED|mọi boot|

## 2. Phân tích các điểm nghẽn

### 2.1 Chạy không cache sau `nvme init` (nghẽn có cấu trúc)

`dcache_disable` xoá cả `SCTLR.M`; RAM thành Device-nGnRnE (không cache, không gộp, không reorder). Mọi thao tác sau (nạp, `memmove`, CRC, SHA) thực hiện từng giao dịch AXI riêng. Với ảnh hàng chục MB, đây có thể là **thành phần chi phối** của thời gian từ `nvme init` đến `Starting kernel` [S; cần đo]. Nhánh SD/MMC không có hiện tượng này. Hướng giảm (nếu được phép sửa): bật lại D-cache sau `init_nvme` kèm bảo trì cache theo vùng thay vì tắt toàn cục; hoặc gọi `nvme_dcache_enable` đúng cách; hoặc dùng `NVME_DCACHE_OFF` chỉ quanh lệnh DMA.

### 2.2 `verify = -1` (CRC32 initrd luôn chạy)

`getenv_yesno("verify")` trả −1 (truthy) khi biến chưa tồn tại. Đặt `verify=n` trong env mặc định sẽ bỏ CRC dữ liệu (đổi lấy bớt kiểm toàn vẹn).

### 2.3 initrd bị sao chép hai lần

`initrd_high=ffffffff` không khiến "không chép" trên ARM64 (Phần 16); nên có `memmove` cỡ initrd. Đặt `initrd_high=ffffffffffffffff` (hoặc bỏ) để dùng tại chỗ [S; cần kiểm ở kernel].

### 2.4 SCPI WDT đồng bộ

Mỗi lệnh chờ R4 ít nhất một vòng `mdelay(1)`; nhân với ~30 lệnh cho cỡ chục ms trong trường hợp bình thường, nhưng **hàng chục giây** nếu R4 chết (mỗi lệnh ≤2 s). Không có cờ bỏ qua khi R4 không đáp.

### 2.3 `bootdelay` 3 s

Chiếm phần cố định lớn nhất khi không phải Warp. `bootdelay` được `getenv("bootdelay")` (mặc định `3` trong env) nên đổi được, nhưng env không lưu (mất khi reset); chỉ đổi được bằng cấu hình biên dịch (`CONFIG_BOOTDELAY`). KM còn ép `bootdelay=0` cho Warp.

### 2.4 Log

`printf` qua `BspPrintBuffering` (bộ đệm 255 mục) tốn sprintf + chép; `serial_puts` 115200 baud ≈ 11.5 KB/s: một dòng 80 ký tự ≈ 7 ms. Khi `termFlg` cho phép, **flush một lần** toàn bộ bộ đệm (tối đa ≈ 255×1051 B nếu đầy) có thể mất **tới ~23 s** ở 115200 baud trong trường hợp xấu nhất (268 KB / 11.5 KB/s) [tính toán]; thực tế nhỏ hơn nhiều vì bộ đệm hiếm khi đầy. Lưu ý WDT trong lúc flush (5 s giữa hai lần refresh ở giai đoạn init) [S].

### 2.5 `flush_dcache_all` trên L1/L2/LLC

Một lần khi tắt D-cache; chi phí tỉ lệ với dung lượng cache (chưa biết).

### 2.6 Warp

`warp_boot`: nạp driver từ SPI (đọc XIP qua `0xF4000000`, tối đa vài trăm KB), nạp bootflag 0x400 B, nhảy `hibernate()`. Thời gian khôi phục snapshot phụ thuộc kích thước RAM lưu (không trong U-Boot). Đây là lối **rút ngắn** boot chính.

## 3. Cách đo (đã có sẵn trong mã)

|Công cụ|Cách dùng|
|---|---|
|`bootstage_report()`|`CONFIG_BOOTSTAGE`, `BOOTSTAGE_REPORT` được định nghĩa (mvebu-common.h:37-38); `announce_and_cleanup` in báo cáo khi `Starting kernel` (kết quả vào bộ đệm log)|
|Lệnh `time <cmd>` (CMD_TIME)|đo thời gian một lệnh (vd. `time fatload nvme 0:5 …`)|
|Lệnh `timer` (cmd_misc.c)|`timer start/get`|
|`get_timer(0)` trong mã|tự thêm `printf`|
|WDT `point`/`debugInfo2`|R4 ghi lại điểm cuối cùng, dùng suy ra điểm chậm/treo|
|Đồng hồ ngoài|đo thời gian từ bật nguồn tới `Starting kernel` bằng UART khi `termFlg` cho phép|

Để đo thật sự cần chạy trên máy; tôi không có số liệu.

## 4. Khuyến nghị theo mức bằng chứng

1. Đo thời gian `nvme init` → `booti` với `time` để định lượng 2.1 (ưu tiên cao nhất).
2. Cân nhắc `verify=n` và `initrd_high` đúng (2.2, 2.3) sau khi đánh giá rủi ro toàn vẹn.
3. Thêm cờ bỏ qua SCPI khi R4 không đáp (tránh cộng dồn ≤2 s × ~30).
4. Không tăng độ dài log khi `termFlg` bị ẩn (tốn `sprintf` mà không ai thấy).

## 5. COMMON CONFUSION

1. **"Chậm là do SSD"**: có thể do MMU tắt.
2. **"Không chỉnh được bootdelay"**: đổi được qua cấu hình biên dịch; env không lưu.
3. **"WDT không ảnh hưởng thời gian"**: mỗi lệnh là một round-trip.
4. **"Log miễn phí"**: đệm và flush đều tốn.

## 6. MENTAL MODEL

```
Thời gian boot ≈ bootdelay(3s) + init_sequence (WDT×16, HW init) + NVMe init + [nạp Image/DTB/initrd KHÔNG CACHE] + [CRC initrd] + [memmove Image, initrd] + cleanup + kernel
Warp = rút ngắn thay cho (nạp + kernel boot lạnh)
```

## 7. Chưa xác minh

- Mọi con số thời gian thực (không có đo).
- Dung lượng LLC/L2 và thời gian flush.
- Kích thước `Image`/`irfs.ubt` thực.

---

Xong Phần 37 và 38. Tiếp theo **Phần 39: Debugging** — các công cụ, cờ debug, log, lệnh chẩn đoán, cách bật console, giải mã lỗi.

Grep "DEBUG_NVME|debug_nvme|DEBUG_BETA5|DEBUG_BOOTKEYS|CONFIG_MVEBU_DEBUG|debug_enter|debug_exit|PRINT_ROM|#define DP\(|DP\(|SCPI_DEBUG|USB_STOR_PRINTF|CONFIG_MV_DEBUG_PUTS|CONFIG_LOGBUFFER|DEBUG\b" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\include\configs, glob: {armada8k.h,mvebu-common.h})

4 lines of output

Grep "define\s+(PRINT_ROM|DP\b|DP\()|define\s+DEBUG_[A-Z_]+|#define\s+PRINT_BUFFER_NUM" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {board/mvebu/armada8k/km/*.[ch],include/*.h,common/main.c,drivers/block/nvme.c,arch/arm/include/asm/*.h})

18 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\dipvalue.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 2-172)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\drivers\block\nvme.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 41-80)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\machine_setup.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 45-68)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\init_debug.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 2-81)

Grep "gNvmeLogFlag|"log"|nvme_log|log_flag" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src, glob: {common/cmd_nvme.c,drivers/block/nvme.c,include/linux/nvme.h})

6 lines of output

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\board\mvebu\armada8k\km\init_debug.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 151-222)

Read [C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\cmd_nvme.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/index.html?id=fa849b44-b60e-429b-b14e-b56edd7f4c99&parentId=1&origin=41667a86-0733-44d1-b408-35b5eebd996f&swVersion=6&extensionId=Anthropic.claude-code&platform=electron&vscode-resource-base-authority=vscode-resource.vscode-cdn.net&parentOrigin=vscode-file%3A%2F%2Fvscode-app&session=88f4ba1f-d3a0-4510-99a2-3f90ab92b819#) (lines 396-440)

# PHẦN 39 (PART 40): Debugging

## 0. Kết luận

Công cụ gỡ lỗi có sẵn trong ảnh này là: **console UART có điều kiện**, một bộ lệnh đọc/ghi phần cứng (`md/mw/bspi/qzgpio/nvme/mmc/usb/pci/fdt/map`), lệnh riêng của KM (`db_sw`, `initdebugdata`, `doboot … 0`, `Y/Z`), **LED panel**, **mã điểm WDT** ghi bởi R4, và **vùng debug trong SPI-flash**. Không có JTAG/KGDB, không có `coninfo`, `bootstage` lệnh, `iminfo`. Khó khăn lớn nhất là **không thấy log** trên máy sản xuất; phần 1 nói cách bật.

## 1. Bật console và log

### 1.1 Cách bật đầu ra (`getTermState`, dipvalue.c:48)

```
termFlg == 0                      → TERM_STATE_0  (đang khởi động, log vào bộ đệm)
isForceLogOutput() = 1 → TERM_STATE_1:
     ├ FW_CONFIG5 (GPIO 271) đọc = 1 (HIGH)   ("ES board luôn có log")
     └ LOG_SW (GPIO 233) = HIGH
SoftDIP SW-DipSW19:  giá trị = (byte@0x21 & 0x03) | ((byte@0x51 & 0x80) >> 5)   (offset trong vùng SoftDIP)
     0 → TERM0 (không log) ; 1 → TERM1 ; 2 → TERM2 ; 3 → TERM3 ; 4 → TERM4
đọc SoftDIP lỗi hoặc 0xFF → coi là 0
```

- Kết quả được **cache** (`static term_state`), tính một lần sau khi `termFlg=1`.
- `TERM_STATE_0` = **không in** (mọi thứ vào bộ đệm và bị mất khi reset). Bất kỳ TERM ≥ 1 sẽ **flush một lần** rồi in trực tiếp.
- Cổng UART theo `term_sel` (AP-UART 115200 tại `0xF0512000` khi `term_sel=1`; CSRC-UART tại `0xE8003000` khi 0) từ GPIO FW_CONFIG4/5 (Phần 10: pin đọc HIGH ⇒ AP-UART). Panel UART không phải console.
- `make_bootargs` cũng đổi theo TERM: `TERM_STATE_0` → `console=tty1 loglevel=1` (không UART); còn lại → `console=ttyS0,115200` hoặc `ttyS5,115200` (Phần 16). Vì thế bật log ở U-Boot đồng thời bật console kernel.

### 1.2 Lệnh kiểm tra trạng thái log

`db_sw` (dipvalue.c:146): in `Force Log Output: ON/OFF` và `Terminal State: TERM0…TERM4`. Chỉ chạy được khi đã vào prompt (cần console).

### 1.3 Ngắt autoboot để vào prompt

Bất kỳ phím nào trong 3 s (Phần 15/16). Nhấn ngay lúc bật nguồn (một byte trong bộ đệm UART cũng đủ nhờ `ZERO_BOOTDELAY_CHECK`). Nếu `isWarpBootMode()` (GPIO WARP_SW thấp) thì delay = 0 nên gần như không kịp ngắt; nhả công tắc Warp trước.

## 2. Bộ lệnh gỡ lỗi (có trong ảnh)

|Mục đích|Lệnh|Ghi chú|
|---|---|---|
|Bản đồ bộ nhớ, `relocaddr`, DRAM|`bdinfo`|`CMD_BDI`|
|Bộ nhớ/thanh ghi|`md[.b/.w/.l/.q]`, `mm`, `nm`, `mw`, `cp`, `cmp`, `crc32`, `loop`, `mtest`|dùng cho MMIO (Phần 35)|
|Cửa sổ địa chỉ SoC|`map` (`CMD_MVEBU_MISC`)|ADEC/CCU/RFU|
|SPI-flash|`bspi list`, `bspi readid <dev>`, `bspi read <dev> <addr> <size>`, `bspi readmem <dev> <dest> <addr> <size>`|**cẩn thận**: `erase/write/writemem` phá dữ liệu|
|GPIO|`qzgpio value <n>`, `dir <n> [in\|out]`, `func_sel <n>`|đọc trạng thái phím/công tắc|
|NVMe|`nvme init`, `info`, `device`, `part`, `read addr blk# cnt`, **`write`**, `flush`, **`nvme debug <mask>`**|mask: bit0 rw, 1 prp, 2 init, 3 mem, 4 queue (đặt `gNvmeLogFlag` lúc chạy)|
|SD/eMMC|`mmc rescan`, `mmcinfo`, `mmc dev/part/read`||
|USB|`usb start`, `usb tree`, `usb info`, `usb storage`|nhớ `usb_init` ghi PS-CPU|
|PCIe|`pci`, `pci <bus>`|liệt kê thiết bị|
|DTB|`fdt addr <addr>`, `fdt print /chosen`, `fdt print /memory`|kiểm tra bản vá (Phần 8/17)|
|Env|`printenv`, `setenv`, `editenv`, `run`|xem `bootcmd/bootargs` do `check_autoboot` tạo|
|Boot thử|`doboot <type> 0`|in `bootargs`/`bootcmd` cho từng `g_action` mà **không boot**|
|Thông tin máy|`Y` (MAC), `Z` (serial)|đọc SPI machineinfo|
|Thời gian|`time <lệnh>`, `timer start|get`,` sleep`|
|SPI debug KM|`initdebugdata dump`|xem mục 3.2|
|Panel/âm thanh|`draw`, `sound`|kiểm tra phần cứng|
|Phiên bản|`version`|`U-Boot 2015.01-devel-16.07.2` + compiler/linker|
|Khác|`help`/`?`, `go`, `loads/loadb/loady`, `bootelf`|tải mã thử|

Không có: `coninfo`, `bootstage`, `iminfo`, `echo` (hush chỉ có `echo`? không: `CMD_ECHO` tắt), `gpio`, `i2c` (CMD_I2C có header nhưng chưa xác định có build), `sf`, mạng.

## 3. Macro và biến debug

### 3.1 Bật/tắt trong mã (cần build lại)

|Macro|Nơi|Trạng thái|Tác dụng|
|---|---|---|---|
|`DEBUG`|mvebu-common.h:36|comment|bật `debug()` toàn cục (rất nhiều in)|
|`CONFIG_MVEBU_DEBUG_FUNC_IN_OUT`|:39|comment|`debug_enter/exit`|
|`CONFIG_MV_DEBUG_PUTS`|:40|comment||
|`DEBUG_BETA5`|armada8k.h:153|**bật** (`#if 1`)|ghi chú "dừng giữa chừng khi khởi động"; dùng bởi mã debug beta|
|`DP()`|mfp_panel.c:116, machine_setup.c:52|tắt (`#if 0`)|in tiến trình `setup_MotionEnable_Power`, panel|
|`PRINT_ROM()`/`DEBUG_ROM`|machine_setup.c:58|tắt|in MAC/MachineInfo/Serial đọc từ ROM|
|`SCPI_DEBUG`|arm_scpi.c:51|tắt|in từng truy cập MHU|
|`NVME_DEBUG` + `gNvmeLogFlag`|nvme.c:44-56|**bật khả năng**, mặc định mask 0|điều khiển bằng `nvme debug`|
|`DEBUG_NVME_WAIT`|nvme.c:45|bật|trace chờ hoàn tất|
|`DEBUG_SSD_MTACH`|main.c:398|bật|debug nhận diện SSD|
|`DEBUG_BOOTKEYS`|autoboot.c:26|0||
|`USB_STOR_PRINTF`|usb_storage.c|theo macro cục bộ|in cây USB|
|WDT debug|`km_scpi_set_wdt_ca72_ext(START, …, debugInfo1 bit0 = 1)`|tại board_r.c:884 truyền `0/*1=debug mode*/`|đổi thành 1 để in mỗi lệnh WDT (`## wd refresh point:0x… timeout:…`)|
|`CONFIG_SYS_LONGHELP`|bật||`help <lệnh>` chi tiết|
|`-g`, `-fstack-usage`, `u-boot.map`, `System.map`|build||phân tích offline|

### 3.2 Vùng debug trong SPI-flash (KM)

`initdebugdata dump` (init_debug.c) in hai loại 4 KB, mỗi loại có **hai sector** thay phiên (mặt hiện dùng chọn bằng số lần xoá; `initDebugGetCurrentSectorOffset` xử lý tràn bộ đếm):

- **Trình tự khởi động** ("SEQ INIT"): `0x37E000`/`0x37F000` (`0x07E000/0x07F000 + 0x300000`).
- **Trình tự tiết kiệm điện** ("SEQ PM"): `0x37C000`/`0x37D000`. Nội dung do phần mềm khác (Linux) ghi; U-Boot chỉ dump hex. Đây là "hộp đen" ghi thời điểm khởi động/tiết kiệm điện qua các lần chạy, hữu ích để xem máy dừng ở đâu.

## 4. Chỉ báo không cần console

|Nguồn|Cách đọc|
|---|---|
|**LED panel**|`set_Panel_BootDiagStatus` (ST1…ST5: tiến trình), `set_Panel_BootDiagResult` (CPU: Data LED nhấp nháy; Base: Power LED nhấp nháy; Storage: Warning LED nhấp nháy) (Phần 29)|
|**Điểm WDT**|R4 ghi `point` cuối cùng (0x1000|
|**Mã recovery**|BSP/Thread `0x377000+0x2E` (2 byte: code<<8|
|**Chẩn đoán CSRA**|`THRF_setAutoHwDiagnosCUP(0x02)`, `Storage(0x41/0x46)`|
|**MAC/serial**|`Y`, `Z`|
|**GPIO trạng thái**|`qzgpio value`|
|**Đo thời gian bên ngoài**|logic analyzer trên UART/GPIO|

## 5. Phân tích ngoại lệ (khi có log)

```
"Synchronous Abort" handler, esr 0x96000045          ← EC=0x25 (data abort, cùng EL), DFSC=0x05 (translation fault L3)
ELR:     0000000003ff0c34                            ← PC lỗi (địa chỉ ĐÃ relocate)
LR:      ...
x0..x29
```

Quy trình:

1. Lấy `Relocation Offset is: <off>` (in ở `display_options`/`display_new_sp` sau relocation) hoặc `reloc_off = gd->relocaddr − 0x1000` (`bdinfo` cho `relocaddr`).
2. Địa chỉ liên kết = `ELR − reloc_off` (vì `TEXT_BASE = 0x1000` và ảnh tại 0x1000-based).
3. Tra `u-boot.map`/`System.map` hoặc `objdump -d u-boot` để ra hàm/dòng (`-g` có sẵn).
4. Giải mã ESR: `EC = esr>>26`, `ISS = esr & 0x1FFFFFF`; **không có FAR** nên phải suy địa chỉ gây lỗi từ thanh ghi/mã tại ELR.
5. Nhớ: log này **chỉ thấy khi TERM ≥ 1**, và máy tự reset sau ~150 ms.

## 6. Kịch bản gỡ lỗi thường gặp

|Hiện tượng|Kiểm tra theo thứ tự|
|---|---|
|Không thấy log nào|(1) GPIO233/271, SoftDIP SW19 → TERM; (2) đúng cổng UART (`term_sel`: FW_CONFIG4/5); (3) baud 115200; (4) panel LED|
|Màn hình đen, không khởi động|LED (Phần 29); `_WAIT_FOREVER` do HW-check; điểm WDT; `chk_sboot` ("boot failed."); DIMM (`### BOOT-DIAG ERROR/Dimm ###`)|
|Vào USB update ngoài ý muốn|Utility Key (GPIO49 cao?), cờ line-card, `FW0000/INDEX`|
|Dừng ở "Hit any key" rồi vào prompt|phím ngẫu nhiên trên UART (nhiễu), `ZERO_BOOTDELAY_CHECK`|
|`Bad Linux ARM64 Image magic!`|`fatload` thất bại (xem `filesize`, `nvme part`, `fatls nvme 0:5`), hoặc Image nén|
|`Wrong Ramdisk Image Format` / `Bad Data CRC`|`irfs.ubt` sai định dạng/kiến trúc (`-A arm64`) hoặc hỏng|
|Kernel không lên, không log|`console=` (TERM_STATE_0 → `tty1`), `bootargs` (dùng `printenv bootargs`), DTB `/chosen`; kiểm `fdt print /chosen` sau `booti` (không thể: sau nhảy). Dùng `doboot <n> 0` xem trước|
|Reset lặp|ngoại lệ (`panic`), WDT (điểm), `reset_with_recoverycode` (mã 0x01–0x0c), chip bảo mật|
|Chậm|Phần 38; `time`|
|SSD không nhận|`pci`, `nvme init`, whitelist `st_nvme_supported[]` (+30 mục SPI 0x200), `nvme debug 0x4`|
|Warp không chạy|`isWarpBootMode`, `warp_checkboot` ("Can't load Warp!! driver", bootflag W5BF), SuperWarp bị vô hiệu (0x20), lệnh `warp 1` thủ công|

## 7. Kỹ thuật an toàn khi gỡ lỗi

1. **Không** dùng `bspi erase/write/writemem`, `C`, `CB`, `X`, `x`, `y`, `z`, `nvme write`, `mmc write` trên máy thật trừ khi hiểu hậu quả (xoá SoftDIP/NVRAM/MAC/serial; `z` xoá snapshot Warp).
2. Gõ `y`/`z` **kèm đối số** (không đối số gây truy cập NULL).
3. Sau `nvme init` mọi truy cập chạy MMU tắt; không phải lỗi phần cứng nếu chậm.
4. Vì env không lưu, mọi thay đổi `setenv` chỉ tồn tại tới lần reset.
5. WDT: khi ở prompt (đã END) không bị reset; nhưng nếu ngắt trong khi WDT đang mở (giai đoạn init) thì máy có thể reset.

## 8. Công cụ ngoài repo nên có

`u-boot.map`, `System.map`, `u-boot` ELF (từ build); tài liệu điểm WDT của R4; công cụ đọc SPI-flash (đọc `0x36E000` SoftDIP, `0x377000` BSP/Thread, `0x37E000` debug); JTAG tuỳ board (không có trong repo).

## 9. COMMON CONFUSION

1. **"Không có log = U-Boot hỏng"**: thường là `TERM_STATE_0`.
2. **"`debug()` in được"**: `DEBUG` bị comment.
3. **"Ngoại lệ luôn hiện"**: mất khi reset nếu console ẩn.
4. **"Địa chỉ trong ELR khớp `.map` trực tiếp"**: phải trừ `reloc_off`.
5. **"`bootstage` lệnh dùng được"**: chỉ tự in ở `Starting kernel`.

## 10. MENTAL MODEL

```
Không log?  → GPIO233/271 hoặc SoftDIP SW19 ≠ 0 ⇒ TERM≥1 ⇒ có UART (AP/CSRC theo term_sel)
Có prompt?  → md/bspi/qzgpio/nvme/mmc/usb/pci/fdt/doboot/db_sw/initdebugdata
Không prompt? → LED panel + điểm WDT + mã recovery + vùng debug SPI 0x37C000..0x37F000
Ngoại lệ?   → ELR − reloc_off → u-boot.map ; ESR decode ; (không FAR)
```

## 11. Chưa xác minh

- Cách R4/Linux hiển thị điểm WDT cuối.
- Ý nghĩa `TERM1..TERM4` khác nhau ra sao (mã chỉ phân biệt in/không in và `TERM_STATE_0` khác).
- `DEBUG_BETA5` ảnh hưởng chính xác gì.
- Sự tồn tại `CMD_I2C` thực sự build.

---

# PHẦN 40 (PART 41): Failure / Recovery

## 0. Kết luận

Chiến lược phục hồi của hệ thống là **nhiều lớp độc lập**: (1) **phần cứng/R4**: WDT 5–300 s; (2) **phần mềm U-Boot**: hang/panic/reset có giới hạn; (3) **dữ liệu**: hai mặt + bộ đếm cho vùng cấu hình; (4) **firmware**: GoldenMaster cho Boot/LPPP và "Subset" (0:2) làm đường cứu hộ; (5) **người vận hành**: LED panel, thẻ USB đặc biệt (Initialize/Board-erase), lệnh `y/z`. Điểm yếu chủ yếu nằm ở chỗ **nhiều vòng "khoá vĩnh viễn" cố ý** (không tự thoát) và **không có cơ chế rollback kernel trong U-Boot**.

## 1. Ma trận sự cố → phát hiện → phản ứng → phục hồi

|#|Sự cố|Phát hiện|Phản ứng|Phục hồi|
|---|---|---|---|---|
|F1|U-Boot treo sớm (trước `initr_wd_start`)|không có WDT (R4 chưa START)|treo|cắt nguồn / có thể R4 có WDT riêng khi boot (ngoài repo) [?]|
|F2|Treo trong `init_sequence_r`|WDT 5 s giữa các refresh|R4 reset máy|tự động (lặp nếu lỗi ổn định)|
|F3|DIMM sai (HW-check)|`init_km`|LED CPU-board, `_WAIT_FOREVER` (WDT nạp)|sửa phần cứng|
|F4|UART panel lỗi (HW-check)|`uart_error`|`BOOT-DIAG ERROR/Panel`, `_WAIT_FOREVER`|sửa panel|
|F5|Không thấy SSD/SD (HW-check)|`hwc_normal_devices == 0`|LED Storage, `_WAIT_FOREVER`|thay/thắp lại thiết bị|
|F6|SSD không nhận (boot thường)|`nvme init` lỗi|`fatload` thất bại → `booti` "bad magic" → prompt; WDT END|vào prompt hoặc reset thủ công|
|F7|Kernel/DTB/initrd hỏng|`booti` magic/CRC|trả 1 → prompt|dùng USB update/Subset|
|F8|Nạp OK nhưng kernel không chạy|WDT 300 s không được Linux nạp|R4 reset sau 300 s|tự động (lặp)|
|F9|Ngoại lệ CPU|handler|`panic` → `do_reset` (150 ms)|tự động, **không giới hạn lặp**|
|F10|Lỗi đồ hoạ (LVDS/PNG/LZO)|`reset_with_recoverycode`|reset ≤5 lần, ghi mã, tắt SuperWarp|sau 5 lần: tiếp tục với đồ hoạ hỏng|
|F11|Chip bảo mật bị tháo|`chk_sboot`|`boot failed.` + WDT END + `while(1)`|**chỉ cắm lại chip + cắt nguồn**|
|F12|Đọc secure info lỗi|`get_secureinfo`|fail-closed như F11|cắt nguồn và thử lại|
|F13|Warp resume thất bại|`warp_checkboot`/`hibernate` trả lỗi|rơi xuống boot thường; bootflag bị xoá|tự động|
|F14|Cập nhật dở (mất điện)|cờ MSC (0x37B000)|boot Subset (0:2) hoặc `NOTHING_BOOT_IMAGE`|chạy lại cập nhật|
|F15|USB sai kiểu máy|INDEX `TYP`|`BOOT_ERR` → 0:2|rút USB|
|F16|SPI-flash vùng cấu hình mất một mặt|`THRD_GET_SPI_OK_AREA1/2_ERR`|đọc từ mặt còn lại|tự lành ở lần ghi kế|
|F17|Mất cả hai mặt|`NG_ALL_AREA`|giá trị mặc định 0|nhập lại (`y`, `z`, cấu hình)|
|F18|Mất điện giữa `set_secureinfo_to_spi`|mất cờ sector 0x3FE000|mất SBOOT/GM cờ|nhập lại; nguy cơ mở khoá tháo chip [S]|
|F19|GM update lỗi|`chkGmUpdate`|in và tiếp tục|thử lại lần boot sau (cờ còn)|
|F20|R4 không đáp SCPI|`[SCPI] Send/Response timeout`|bỏ qua, chậm thêm|R4 phải được kiểm; boot vẫn tiếp tục|
|F21|NVMe admin timeout|60 s|lệnh thất bại|reset|
|F22|PHY USB không sẵn sàng|vòng vô hạn|WDT 120 s reset|thử lại|
|F23|Không tạo được FDT|`image_setup_linux`|`hang()`|WDT reset (300 s) hoặc cắt nguồn|
|F24|Người dùng ngắt autoboot|phím|WDT END, prompt|`boot`/`reset`|

## 2. Cơ chế phục hồi chi tiết

### 2.1 Phục hồi dữ liệu cấu hình (hai mặt + bộ đếm)

Vùng SoftDIP, BSP/Thread, machineinfo có **hai bản** (mỗi bản sector 4 KB) và **bộ đếm xoá** 4 byte; `initDebugGetCurrentSectorOffset`/`THRG_get*` chọn mặt hợp lệ theo bộ đếm (xử lý cả tràn `0xFFFFFFFF → 0`). Ghi luôn cập nhật mặt kia trước để mất điện giữa chừng không mất cả hai (theo thiết kế [S]). Mã trả về phân biệt `OK`, `OK_AREA1_ERR`, `OK_AREA2_ERR`, `NG_ALL_AREA`.

### 2.2 Phục hồi firmware

- **Subset (0:2)** là hệ điều hành cứu hộ trên SSD/SD (`BOOT_ERR`, `IISW`).
- **GoldenMaster** (Boot 0x400000, LPPP 0x400000 trên `mtd1`): sao Primary→GM sau khi Primary xác nhận chạy tốt. BootROM/ATF chọn GM khi Primary hỏng [S, ngoài repo].
- **USB "Subset"** làm đường cứu hộ khi SSD hỏng hoàn toàn.

### 2.3 Chống vòng lặp

- `reset_with_recoverycode`: ≤5 lần.
- Warp: snapshot dùng một lần (xoá trước khi chạy) nên không lặp.
- **Không có**: giới hạn cho `panic`, cho WDT 300 s (kernel lỗi lặp), cho R4 reset.

### 2.4 Điểm khoá có chủ đích ("fail-closed")

`chk_sboot`, `initialize_card_main`, `bderase_card_main`, `_WAIT_FOREVER` (chẩn đoán): các vòng này **không có đường tự thoát**; vận hành viên phải can thiệp (cắt nguồn) [thiết kế chủ động].

## 3. Lỗ hổng phục hồi đã xác nhận

1. **Không rollback kernel**: kernel mới hỏng chỉ dẫn tới reset 300 s lặp; không có bộ đếm "boot thất bại → quay về ảnh cũ" trong U-Boot (cờ do Linux quản lý) [S].
2. **`panic` không giới hạn lặp** và log mất khi reset (Phần 28).
3. **Sector secure info một bản** (Phần 21).
4. **`chk_sboot` khoá khi lỗi đọc tạm thời**.
5. **`GmUpdateReq` cờ không xoá khi thất bại** → thử lại mỗi lần boot thường (mảng 3 MiB trên stack mỗi lần) [S].
6. **WDT không có lưới trong khoảng giữa START và `initr_wd_start`** và sau END.
7. **`reset_with_recoverycode` sau 5 lần chạy tiếp với đồ hoạ hỏng** mà không báo trên panel (không có chỉ báo).
8. **Kết quả `send_cmd` bị bỏ qua**: WDT có thể không được nạp mà U-Boot không biết.

## 4. Quy trình khôi phục thực tế (theo tình huống)

|Tình huống|Bước|
|---|---|
|Máy không lên, LED Storage nhấp nháy|kiểm SSD/kết nối; nếu thay SSD: whitelist + `Z`/`Y`; nếu cần: USB Subset|
|Máy reset lặp|bật log (GPIO233/SoftDIP); xem `panic`/mã recovery/điểm WDT; nếu recovery code: kiểm panel/LVDS/ảnh|
|"boot failed."|cắm lại chip bảo mật (PIO_SYS[36]=H) rồi cắt nguồn; **không** tự xoá cờ SBOOT qua console (vô hiệu hoá bảo vệ)|
|Sau cập nhật máy vào Subset|cờ MSC dở → chạy lại cập nhật|
|Warp lỗi|tự rơi xuống boot thường; nếu lặp: `z <serial>` xoá snapshot, hoặc `clear_bootf`|
|Mất MAC/serial|`y <12hex>`, `z <16 ký tự>`|
|Mất cấu hình SPI|khôi phục từ bản dự phòng nhà máy (`C`/Initialize card rồi nạp lại)|

## 5. COMMON CONFUSION

1. **"GM tự thay Primary"**: chỉ sao chép Primary→GM; việc dùng GM do BootROM/ATF.
2. **"Reset = phục hồi"**: nhiều lỗi ổn định sẽ lặp.
3. **"`_WAIT_FOREVER` là treo hỏng"**: là trạng thái báo lỗi có kiểm soát.
4. **"Xoá cờ `SBOOT` là sửa lỗi"**: vô hiệu hoá bảo vệ.

## 6. MENTAL MODEL

```
Lớp 1 (R4 WDT)  ─ 5s/…/300s ─► reset toàn máy
Lớp 2 (U-Boot)  ─ panic→reset | hang | _WAIT_FOREVER(LED) | recoverycode≤5 | chk_sboot lock
Lớp 3 (dữ liệu) ─ 2 mặt + bộ đếm
Lớp 4 (firmware)─ Subset(0:2), GM(Boot/LPPP), USB Subset
Lớp 5 (người)   ─ LED, USB card, lệnh y/z
Thiếu: rollback kernel, giới hạn lặp panic
```

## 7. Chưa xác minh

- Hành vi của BootROM/ATF khi Primary hỏng (GM).
- Có WDT nào trước `initr_wd_start` không.
- Cách firmware chính xử lý `0x2201C0000000xxyy` và cờ `SBOOT`.
- Trình tự ghi hai mặt trong `THRG_set*` (không đọc chi tiết).

---

Xong Phần 39 và 40. Tiếp theo **Phần 41: Complete boot story**: câu chuyện khởi động đầy đủ từ cấp nguồn đến `Starting kernel`, tổng hợp mọi phần.

Grep "Relocation Offset|display_new_sp|New Stack Ptr|display_options|U_BOOT_VERSION_STRING|show_dram_config|DRAM:" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\board_f.c)

8 lines of output

Grep "relocaddr|reloc|Reloc|printf\(" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\cmd_bdinfo.c)

26 lines of output

Grep "Relocation Offset|New Stack|reloc_off|relocaddr|debug\(" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\arch\arm\lib\bdinfo.c)

1 line of output

Grep "TEXT_BASE|display_text_info|text_base" (in C:\Users\PC\Documents\IT6_Kernel\BootROM\BootROM\Emu800\Src\common\board_f.c)

21 lines of output

Đính chính nhỏ cho Phần 39, mục 5: dòng `Relocation Offset is: …` và `U-Boot code: … BSS: …` trong `board_f.c` (dòng 173, 759) dùng `debug()` nên **không được in** (vì `DEBUG` bị comment). Cách lấy độ dời relocation đúng là chạy `bdinfo` (in `relocaddr`, cmd_bdinfo.c:149) rồi tính `reloc_off = relocaddr − 0x1000`.

# PHẦN 41 (PART 42): Complete Boot Story

## 0. Tóm tắt một đoạn

Khi cấp nguồn, BootROM Marvell nạp ATF (BL1/BL2/BL31) từ SPI-flash; DDR/LPDDR4 được huấn luyện ngoài repo. ATF nạp **U-Boot 2015.01 (Marvell + lớp KM)** ở `0x1000` làm BL33. U-Boot chạy **một luồng, không ngắt**: khởi tạo tối thiểu (UART, DRAM 2 GiB, I2C), tự chuyển mình lên đỉnh RAM (`0x3FE…`), bật cache/MMU, khởi tạo SD/QSPI/PCIe, đọc **cấu hình bền từ SPI-flash** (MAC, kiểu máy, SoftDIP), đọc **GPIO/phím/USB** để quyết định `g_action`, ghép `bootcmd`/`bootargs`, chờ 3 s (hoặc 0 s nếu Warp), rồi hoặc **khôi phục snapshot Warp**, hoặc `nvme init → fatload Image, DTB, initrd → booti`. Trước khi nhảy nó chép initrd/DTB, vá DTB (bootargs, initrd, bộ nhớ, thuộc tính KM), tắt cache/MMU và vào kernel với `x0 = DTB`. Trong suốt quá trình, **watchdog do R4 giữ** (5 s → 300 s) và **LED panel** báo lỗi.

## 1. Dòng thời gian đầy đủ

### Pha A: ngoài U-Boot (không thuộc repo)

1. Cấp nguồn → PS-CPU và R4 chạy trước; AP806 thoát reset.
2. **BootROM** đọc SPI-flash (Primary Boot tại 0, 3 MiB; GM tại 0x400000) → nạp **ATF BL1/BL2** (SRAM) → BL2 huấn luyện **LPPP-DDR4** (kết quả ở `0x2_0000_0000`), nạp BL31 (EL3 @0x10000000, 4 MB) và **BL33 = `u-boot.bin` tại 0x1000** (bản đã `relocate-rela` cho địa chỉ này).
3. ATF nhảy vào `_start` ở EL2 (hoặc EL1) với MMU/cache tắt, DAIF che.

### Pha B: tiền relocation (`_start … board_init_f`) — cache/MMU tắt

4. `start.S:reset`: đặt `VBAR`, bật FP/SIMD; xoá/invalidate cache, TLB; `lowlevel_init` thoát (không EL3); lõi phụ vào vòng `wfe` (thực tế do ATF giữ).
5. `_main` (`crt0_64.S`): `SP = 0xFF1000`, dựng `gd` (~0x418C8 byte, gồm bộ đệm log 268 KB), `x18 = gd`, malloc sớm 20 KB.
6. `board_init_f`: `termFlg=0`, `printStopFlg=1` (log vào bộ đệm). `init_sequence_f`: `setup_mon_len`, `setup_fdt` (DTB điều khiển nhúng), `arch_cpu_init` (CCU/RFU), `timer_init` (bật system counter), `env_init` (env mặc định tại `default_environment[]`), `init_baud_rate` (115200), `serial_init` (AP-UART, CSRC…), `console_init_f`, banner, `init_func_i2c` (DW-I2C ×4), `dram_init` (**2 GiB cứng**), `setup_dest_addr` (`ram_top = 1 GiB`), các `reserve_*` (TLB 64 KB @0x3FFF0000, VRAM 0x1E0000 @0x3FE10000, U-Boot, malloc 5 MiB, `bd_t`, `gd`, DTB, stack), `dram_init_banksize` (giải mã MC → 3 bank), `reloc_fdt`, `setup_reloc`.
7. `crt0_64.S`: `relocate_code` chép `[__image_copy_start,__image_copy_end)` lên `relocaddr`, vá bảng `R_AARCH64_RELATIVE`; `c_runtime_cpu_setup` đặt lại `VBAR`; xoá BSS; `board_init_r`.

### Pha C: hậu relocation (`board_init_r`) — bật cache

8. `initr_wd_start` → **WDT START (0x1000, 5 s)**; `initr_caches` → **I-cache, D-cache, MMU bật** (bảng trang 512 MB/khối); malloc đầy đủ 5 MiB.
9. `board_init` (Marvell); `initr_serial`; `initr_mmc` (đăng ký Xenon); `initr_bspi` (QSPI Winbond, ghi kết quả huấn luyện LPDDR4 vào SPI nếu cần); **`init_km`** (UART panel, GPIO quạt, cờ HW-check, DIMM); mỗi vài bước có **WDT refresh 5 s**.
10. `initr_env` → **`set_default_env`**: nạp `bootargs`, `bootcmd`, `bootdelay=3`, `baudrate`, `kernel_addr=0x800000`, `fdt_addr=0x1000`, `initrd_addr=0x3000000`, `bootfile=Image`… vào bảng băm; env chỉ ở RAM.
11. `initr_pci` (PCIe RC MSB0125, quét NVMe/thiết bị), `stdio_add_devices` (**`termFlg=1`**, `i2c_init_all`), `console_init_r` (chọn UART theo `term_sel`), **`getTermState`** quyết định log: nếu `TERM ≥ 1` thì **flush bộ đệm một lần**.
12. **`misc_init_r` (KM)**: đọc **ROM/SPI** (`read_rom`): `ethaddr` (OUI hợp lệ), `MachineInfo`, `MachineNo`, `SerialNo`; `init_Panel` (UART panel 57600 8E1, VRAM, ảnh khởi động, đọc **Utility Key GPIO49**); **`setup_MotionEnable_Power`**: nếu Utility Key nhấn hoặc cờ line-card thì `usb_init` (PS-CPU nhả `/RST_SLP2n`, PHY UTM, xHCI), quét USB, `check_action`; ngược lại đọc **SoftDIP 0x06/MSC** → **`g_action`**; đặt `MOTION_EN`; `hwcDispBootDeviceDiag`, `boot_diag`.
13. `interrupt_init`/`enable_interrupts` (rỗng), `board_late_init` → `quartz_late_init` (đặt `dtb_name = emu800.dtb` theo loại board), `run_main_loop`.

### Pha D: `main_loop` (KM)

14. `cli_init` (hush), nếu `g_action` là **Initialize/Board-erase** thì thực hiện xoá SPI rồi treo. `chk_sboot()` (chip bảo mật; khoá nếu tháo).
15. `bootdelay_process` → **`check_autoboot`**: theo `g_action` ghép `bootcmd` (mẫu `nvme init; fatload nvme 0:5 …; booti …` hoặc `mmc rescan;…` hoặc `usb reset;…`) và `bootargs` (`boottyp=…`, `root=/dev/kmsda7`, `console=…`, `TerminalState`, `MachineInfo`, `SerialNo`, `IntegrityState`, `FwDir=FW0000`, `uio_pdrv_genirq…`), đặt `initrd_high=ffffffff`.
16. Nếu **Warp** (công tắc thấp + cờ hợp lệ): ghi kiểu boot vào sector Warp, `nvme_opal_cmd_send`, **`warp_boot(SuperWarp)`** rồi `warp_boot(Normal)`: nạp driver `W5HD` từ SPI 0x2B0000 và bootflag `W5BF` từ SD/NVMe p3/p10, `hibernate()` khôi phục RAM; **không trở về** nếu thành công.
17. Sau Warp thất bại: `warp_clear_bootf` (snapshot dùng một lần).
18. `autoboot_command`: in "Hit any key…", đếm **3 s** (phím bất kỳ ngắt; WDT `bootdelay×2`), rồi WDT **300 s** cho boot thường; `chkGmUpdate` (sao Primary→GM nếu Linux đã đặt cờ).

### Pha E: `bootcmd` và bàn giao

19. `hush` chạy chuỗi lệnh: **`nvme init`** (link PCIe, tạo hàng đợi; **D-cache và MMU tắt từ đây**); **`fatload nvme 0:5`** nạp `Image`→0x800000, `emu800.dtb`→0x1000, `irfs.ubt`→0x3000000 (NVMe polling, FAT sector-by-sector).
20. **`booti 0x800000 0x3000000 0x1000`**: kiểm magic ARM64; `memmove` Image về `0 + text_offset`; `lmb_reserve`; kiểm initrd (uImage, magic + CRC header + **CRC dữ liệu** vì `verify=-1`, arch ARM64); kiểm DTB (`fdt_check_header`). `usb_stop`.
21. `image_setup_linux`: **chép initrd** lên vùng cao dưới stack U-Boot; **chuyển DTB** lên đỉnh cửa sổ 16 MB (+ pad 0x3000); **vá DTB**: `/chosen/bootargs`, `linux,initrd-*`, `/memory` (bank + hddinfo, boottype, panel…, vram*), `mac-address` từ `ethaddr`.
22. `announce_and_cleanup`: ghi sự kiện `KernelStart` cho PS-CPU (nếu DIP 144), `bootstage_report`; `cleanup_before_linux`: tắt I-cache, flush D-cache + **LLC**, tắt MMU, invalidate. `smp_kick_all_cpus` (SGI0), `armv8_switch_to_el2` (no-op).
23. **`kernel_entry(fdt, 0, 0, 0)`** → Linux tại `text_offset`, ở cùng EL với U-Boot, MMU tắt. WDT 300 s vẫn chạy; NVMe vẫn bật; VBAR vẫn của U-Boot.

## 2. Bức tranh bộ nhớ trong suốt câu chuyện

```
Trước relocation:  U-Boot @0x1000.. ; SP @0xFF1000 (gd, stack) ; DTB điều khiển nhúng trong ảnh
Sau relocation:    U-Boot @≈0x3FExxxxx (mon_len) ; TLB @0x3FFF0000 ; VRAM @0x3FE10000 ; malloc 5 MiB ; bd/gd/fdt/stack ngay dưới
Nạp:               DTB @0x1000 ; Image @0x800000 ; initrd @0x3000000
Sau image_setup:   Image @text_offset ; DTB ≤16 MB ; initrd cao dưới stack U-Boot
Ngoài:             ATF @0x10000000 (4 MB) ; DDR training @0x10400000 ; SCPI shmem @0x7FFF0000
```

## 3. Các "người chơi" xuyên suốt

|Người chơi|Vai trò|
|---|---|
|BootROM/ATF|nạp BL33, huấn luyện DDR, giữ EL3/PSCI|
|U-Boot|quyết định boot, nạp, chuyển giao|
|R4 (SCP)|watchdog; chia sẻ SPI-flash, hộp thư|
|PS-CPU|nguồn phụ: `/RST_SLP2n`, sự kiện, `HRESET_REQ`|
|Panel MCU|LED, ảnh khởi động|
|SPI-flash|cấu hình bền, firmware Primary/GM, cờ|
|SSD/SD/USB|kernel, DTB, initrd, RFS, Subset, snapshot|
|Linux|nạp/tắt WDT, ghi cờ, cập nhật, snapshot|

## 4. Các điểm phân nhánh chính (nhìn lại)

|Điểm|Điều kiện|Nhánh|
|---|---|---|
|GPIO49 / cờ line-card|nhấn / =1|quét USB (`USB_BOOT`, `USB_UPDATE`, line-card, Initialize, Bderase)|
|SoftDIP 0x06|4–11|IISW / UPDATE / BACKUP / RESTORE|
|Cờ MSC (0x37B000)|=1|`BOOT_ERR` (Subset 0:2)|
|HW-check|tự chẩn đoán|`BOOT_HW_CHECKMODE`, `_WAIT_FOREVER` khi lỗi|
|`chk_sboot`|chip tháo|khoá|
|WARP_SW + cờ Warp|hợp lệ|Warp resume|
|Phím trong 3 s|có|prompt|
|`bootcmd`|lỗi|prompt|
|Bất kỳ ngoại lệ||reset|

## 5. Bảng đính chính tích luỹ (Phần 14–41)

|#|Đính chính|Phần|
|---|---|---|
|C1|`SYS_BOOT_RAMDISK_HIGH` **có**; initrd bị chép lên cao (`initrd_high=ffffffff` ≠ `~0UL` trên ARM64)|16|
|C2|`fsload_hash_check` chỉ ở chế độ tự chẩn đoán, không phải cổng secure-boot|16|
|C3|NVMe không `stop` trước kernel; D-cache và MMU tắt suốt sau `nvme init`|16, 23|
|C4→C10|`CONFIG_STATIC_RELA` **có tác dụng**: `u-boot.bin` được `relocate-rela` cho 0x1000|18→25|
|C5|`CONFIG_LZO` có (bootm giải LZO)|18|
|C6|Thêm lệnh KM `X x Y y Z z`|19|
|C7|`map`, `mss_boot` có; `DEVEL_BOARD` hiệu lực|19|
|C8|USB **không** tự khởi tạo; chỉ khi Utility Key/cờ line-card|20|
|C9|`USB_XHCI` = `default USB_XHCI_HCD`; `UHOST_XHCI_CUSTOMIZE` bật; 4 cổng root; không USB-Ether|20|
|C11|Có `-mstrict-align`, có `-fno-stack-protector`|25|
|C12|`FwDir = FW0000` (bản 800 không chọn máy)|26|
|C13|`TYP_NAME=@TYP=ZS_ZX0_PF`, `DEF_MACHINE_NAME=ZeusS_ZX0`|26|
|C14|Tối ưu **`-Os`** (Kconfig default y)|26|
|C15|Offset SPI-flash user-data **cộng `0x300000`** (SoftDIP 0x36E000…); có `mtd1` cho LPPP; Warp driver nằm trong vùng Primary|36|
|—|`jump_to_copy` không có trong ARM64; `relocate_code` do `crt0_64.S` gọi|32|
|—|`Relocation Offset` không in; dùng `bdinfo relocaddr`|39|
|—|`saveenv` không được biên dịch (giữ nguyên từ Phần 14)|14|

## 6. Danh sách rủi ro/lỗi đã xác nhận (tổng hợp)

1. `y`/`z` không đối số → truy cập NULL → reset.
2. `fsload_hash_check` ghi `p_str_env[filesize]` ngoài chuỗi env; parse `filesize` hệ 10.
3. `THRG_updateGmBoot/LPPP`: hai mảng 3 MiB trên stack.
4. `send_cmd`: payload 256 B vào ô 128 B; chỉ flush 64 B; kết quả bị bỏ qua; mỗi lệnh có thể tốn ≤2 s.
5. `make_bootargs`/`make_bootcmds` `sprintf` không giới hạn vào đệm tĩnh 768 B.
6. `phy_utm_mv_init` vòng chờ vô hạn.
7. `set_Panel_BootDiagResult` cộng dồn LED trên biến `static`.
8. `panic` không giới hạn lặp; log mất khi reset.
9. `chk_sboot` fail-closed cả khi lỗi đọc; secure info một bản.
10. Không có xác thực chữ ký; console không mật khẩu; lệnh phá hoại sẵn có (`bspi`, `nvme write`, `mmc write`, `C`…).
11. `flush_dcache_range` sai đối số (CDMA), `invalidate` = clean+invalidate.
12. `CONFIG_WARP_LOAD_ADDR` định nghĩa hai lần.
13. Không XN/W^X; RWX toàn RAM.

## 7. Mô hình tinh thần cuối cùng

```
[Cấp nguồn] ─► BootROM ─► ATF(BL1/2/31, DDR) ─► U-Boot(BL33 @0x1000)
   U-Boot = (1) khởi tạo tối thiểu → tự chuyển lên đỉnh RAM → bật cache
            (2) đọc CẤU HÌNH BỀN (SPI-flash) + ĐẦU VÀO (GPIO/USB/SoftDIP) → g_action
            (3) ghép bootcmd/bootargs → [Warp?] → chờ 3s → nvme init → fatload×3 → booti
            (4) chuẩn bị (chép initrd, chuyển & vá DTB), tắt cache → kernel(x0=DTB)
   Song song: R4 giữ WATCHDOG (5→300s) ; PS-CPU giữ nguồn ; Panel MCU báo LED ; log bị đệm/ẩn tuỳ TERM
   Phục hồi: reset(WDT/panic/recovery≤5) | Subset(0:2) | GM(Boot/LPPP) | USB card | khoá bảo mật
```

## 8. Chưa xác minh (tổng hợp cuối)

- Toàn bộ phần ngoài repo: BootROM, ATF (xác thực BL33, EL bàn giao, DAIF), firmware R4/PS-CPU/panel, DTB kernel, initrd/Subset, bộ cập nhật.
- Số đo thời gian, kích thước `Image`/`irfs.ubt`, bố cục bank DRAM thực, `.config` thực và `u-boot.map` (không có artifact build).
- Định nghĩa `struct rom`, ý nghĩa `TERM1..4`, `0xE800800C`, `0xF0630000`, cách R4 phản ứng khi WDT hết hạn.
- Vị trí `.dtb.init.rodata` so với `__image_copy_end`.

---

Đã hoàn tất các phần 14 → 41 theo yêu cầu (mỗi phần một lượt, tự chuyển tiếp). Phần 42 (Top-50) và Phần 43 (Mental model) không nằm trong phạm vi "5 → 41" bạn yêu cầu; bảng đính chính (mục 5), danh sách rủi ro (mục 6) và mô hình tinh thần (mục 7) ở trên đã bao phủ phần lớn nội dung đó. Nếu bạn muốn, tôi có thể viết riêng Phần 42 và 43 dưới dạng "50 điều cần nhớ" và sơ đồ mô hình tinh thần đầy đủ.