# Trình bày DEF_BACKUP

`DEF_BACKUP` (giá trị `11`) là chế độ **sao lưu dữ liệu** — kích hoạt thuần tuý qua DipSW, không có đối tác USB tương ứng (khác `DEF_IISW`/`DEF_UPDATE` vốn có cặp `USB_UPDATE`). Về hạ tầng boot, nó gần như là **anh em song sinh với `DEF_IISW`** — cùng bootcmd, cùng partition — chỉ khác nhau ở đúng một bit trong `bootargs`.

## 1. Nguồn gốc

DipSW ảo offset `0x06 == 10` — set ở cả hai nơi quen thuộc:

```c
// setup_MotionEnable_Power() (machine_setup.c:237-239)
else if (10 == uc_IISWFlg) { g_action = DEF_BACKUP; }

// check_action() nhánh dự phòng nội bộ (main.c)
else if ( 10 == uc_NWDLExec ) { g_action = DEF_BACKUP; return DEF_BACKUP; }
```

Không có nhánh USB nào set trực tiếp `DEF_BACKUP` — không có kiểu `USB_XXX` nào ánh xạ tới giá trị này trong `UseClassTbl[]`/switch của `check_action()`.

## 2. `check_autoboot()` — dùng chung ảnh với `DEF_IISW`/`BOOT_ERR`

```c
case DEF_BACKUP:
    setenv("bootcmd",  make_bootcmds(CONFIG_IISWCMD_ENV));   // partition 0:2 — GIỐNG DEF_IISW/BOOT_ERR
    setenv("bootargs", make_bootargs(g_action,CONFIG_SSD_RFS_ENV));
    autoboot = AUTOBOOT_SSD_SUBSET;
```

**Không có bước xác thực lại nào** trước khi cam kết (khác hẳn `DEF_UPDATE` — có hẳn một khối kiểm tra lại `INDEX` trên partition `0:A` trước khi chạy). `DEF_BACKUP` đi thẳng vào `switch` mà không qua bất kỳ validation gate nào — một điểm bất đối xứng đáng chú ý so với `DEF_UPDATE`, dù cả hai đều là thao tác "nhạy cảm dữ liệu".

```
nvme init;                                    # (hoặc "mmc rescan;")
fatload nvme 0:2 $kernel_addr $bootfile;
fatload nvme 0:2 $fdt_addr    $dtb_name;
fatload nvme 0:2 $initrd_addr $initrd_name;
booti $kernel_addr $initrd_addr $fdt_addr
```

## 3. Nội dung `bootargs` — đây là câu trả lời dứt điểm cho nghi vấn từ đầu

```c
make_bootargs(DEF_BACKUP, CONFIG_SSD_RFS_ENV)
```

```
boottyp=11 root=/dev/kmsda7 rootdelay=1 [nopanel] console=...
TerminalState=<...> MachineInfo=<hex> MachineNo=<hex> SerialNo=<...>
IISW=0 FwDir=FW9020 DevStr=nvme IntegrityState=<0|1> <CONFIG_BOOTARGS>
```

`IISW=0` (vì `g_action==DEF_BACKUP` **không** khớp `g_action==DEF_IISW || g_action==DEF_UPDATE`, main.c:1602-1603). Vậy dù `DEF_BACKUP` boot **cùng một file nhị phân** (kernel + initrd trên partition `0:2`) như `DEF_IISW`/`BOOT_ERR`/`DEF_RESTORE`, chỉ có **`boottyp`** (`11`) là thứ duy nhất giúp môi trường "Subset OS" bên trong phân biệt được đây là phiên backup — toàn bộ logic backup thật sự (backup cái gì, ghi vào đâu) nằm ở userspace của ảnh `0:2`, ngoài phạm vi mã nguồn BootROM này nên mình không lần ra được thêm chi tiết.

## 4. Motion Enable & Watchdog

- **Motion Enable**: nằm trong danh sách loại trừ (dòng 274-277) → luôn **tắt**.
- **Nhóm delay**: cùng nhóm với `DEF_IISW/DEF_UPDATE/DEF_RESTORE` → `mdelay(4500)` + `WDT REFRESH 0x1130, 10s` trong `setup_MotionEnable_Power()`.
- **Khác `DEF_IISW`/`DEF_UPDATE` ở điểm này**: `DEF_BACKUP` **có mặt** trong nhóm 6 g_action tắt-WDT-sớm (`{DEF_BACKUP, DEF_RESTORE, BOOT_HW_CHECKMODE, BOOT_ERR, BOOT_SPEEDCHANGE, BOOT_LINE}`) → `WDT_CA72_END` ngay trong `setup_MotionEnable_Power()`.
- `autoboot_command()` cũng không có case riêng cho `DEF_BACKUP` → `else` → `WDT_CA72_END` (tái xác nhận, không refresh lại 120s như `DEF_IISW`/`DEF_UPDATE`/`USB_UPDATE`).
- **Warp**: `autoboot=AUTOBOOT_SSD_SUBSET` → không khớp điều kiện Warp, luôn cold-boot.

## 5. Tóm tắt

`DEF_BACKUP` chỉ kích hoạt qua DipSW (`0x06==10`), boot chung một ảnh "Subset OS" với `DEF_IISW`/`BOOT_ERR`/`DEF_RESTORE` trên partition `0:2` mà không có bước xác thực lại nào, phân biệt hoàn toàn bằng `boottyp=11` (`IISW=0`), tắt hẳn cả cơ khí lẫn watchdog trong suốt quá trình — vì đây là thao tác cần chạy ổn định, không có kỳ vọng auto-recovery giữa chừng.