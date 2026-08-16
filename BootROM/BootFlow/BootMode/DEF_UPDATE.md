`DEF_UPDATE` (giá trị `9`) là "IISW update mode" — kích hoạt bởi DipSW (khác hẳn `USB_UPDATE` kích hoạt bởi USB) — và là g_action **duy nhất** có một bước **tự kiểm tra lại tính toàn vẹn của gói update đã tải sẵn** ngay bên trong `check_autoboot()`, trước khi thực sự cam kết boot vào nó.

## 1. Nguồn gốc

DipSW ảo offset `0x06 ∈ {4, 7, 8, 9}` — đọc ở 2 nơi cho cùng kết quả:

- `setup_MotionEnable_Power()` nhánh không-USB (machine_setup.c:228-231).
- `check_action()` nhánh dự phòng nội bộ khi USB không có mặt (main.c:608-611).

_(Đính chính lại từ câu trả lời `BOOT_SPEEDCHANGE` trước: giá trị DipSW `8` thuộc về nhóm này — `DEF_UPDATE` — chứ không phải `BOOT_SPEEDCHANGE`, vốn chỉ kích hoạt qua USB `@SPD`.)_

## 2. Bước đặc biệt nhất: `check_autoboot()` tự xác thực lại gói update trước khi commit

Đây là điều **không có** g_action nào khác trong 13 giá trị có được. Ngay khi `check_autoboot()` được gọi với `g_action==DEF_UPDATE`, trước khi chạy `switch(g_action)`, nó chủ động đọc lại:

```c
char *argsIISW[6] = { "fatload", device_str, "0:A", adr_str, "INDEX", size_str };
// device_str = "nvme" hoặc "mmc" tuỳ isBootFromSD() — ĐỌC TỪ Ổ ĐĨA NỘI BỘ, KHÔNG PHẢI USB!

if(g_action == DEF_UPDATE){
    if( do_fat_fsload(...) == 0 ){                      // có file INDEX trên partition 0:A?
        if( sysBspCmpTypeName(&cindex[6],TYP_NAME)==0 ){ // tên máy trong INDEX có khớp?
            g_UsbKind = sysBspGetUsbMemType(cindex);
            switch( g_UsbKind ){
            case USB_DOWN: printf("IISW download boot.\n"); break;   // OK, giữ nguyên DEF_UPDATE
            default:       g_action = BOOT_ERR; break;               // sai loại → huỷ
            }
        } else { g_action = BOOT_ERR; }                  // sai tên máy → huỷ
    } else { g_action = BOOT_ERR; }                       // không tìm thấy INDEX → huỷ
}
```

Nói cách khác: DipSW chỉ báo "**có** một phiên update đang chờ", nhưng `check_autoboot()` không tin tưởng mù quáng — nó đọc lại đúng file `INDEX` đã được **tải và lưu sẵn từ trước** trên chính partition `0:A` (nội bộ), kiểm tra type `@DWN` (`USB_DOWN`) và tên máy còn nguyên vẹn. Nếu bất kỳ bước nào thất bại, `g_action` bị **ghi đè thành `BOOT_ERR`** ngay tại chỗ, và `switch(g_action)` phía dưới sẽ chạy đúng case `BOOT_ERR` thay vì `DEF_UPDATE`.

## 3. Nội dung `bootcmd` (nếu qua được kiểm tra ở mục 2)

```c
#define CONFIG_IISWCMD_UPDATE_ENV   CMD_INITRD_ENV("%s","%s","0:A","")
```

```
nvme init;                                    # (hoặc "mmc rescan;")
fatload nvme 0:A $kernel_addr $bootfile;
fatload nvme 0:A $fdt_addr    $dtb_name;
fatload nvme 0:A $initrd_addr $initrd_name;
booti $kernel_addr $initrd_addr $fdt_addr
```

**Partition `0:A` là duy nhất** trong toàn bộ 13 g_action — không trùng với `0:5` (DEF_BOOT/BOOT_LINE/BOOT_HW_CHECKMODE/BOOT_SPEEDCHANGE) hay `0:2` (DEF_IISW/BOOT_ERR/DEF_BACKUP/DEF_RESTORE). Xác nhận: đây là ảnh kernel/initrd **riêng biệt thứ ba** trong hệ thống, chuyên trách việc thực thi cập nhật.

## 4. Nội dung `bootargs`

```c
make_bootargs(DEF_UPDATE, CONFIG_SSD_RFS_ENV)   // root=/dev/kmsda7 rootdelay=1
```

```
boottyp=9 root=/dev/kmsda7 rootdelay=1 [nopanel] console=...
TerminalState=<...> MachineInfo=<hex> MachineNo=<hex> SerialNo=<...>
IISW=1 FwDir=FW9020 DevStr=nvme IntegrityState=<0|1> <CONFIG_BOOTARGS>
```

`IISW=1` (khớp `g_action==DEF_IISW || g_action==DEF_UPDATE`, main.c:1602-1603) — cùng cờ với `DEF_IISW`, dù `boottyp` và partition khác hẳn. Userspace phân biệt "đang tải" (`DEF_IISW`, `boottyp=3`) với "đã tải xong, đang cài đặt" (`DEF_UPDATE`, `boottyp=9`) thuần tuý qua `boottyp`.

## 5. Motion Enable & Watchdog

- **Motion Enable**: nằm trong danh sách loại trừ (dòng 274-277) → luôn **tắt**.
- **Nhóm delay đặc biệt**: `DEF_UPDATE` cùng nhóm với `DEF_IISW/DEF_BACKUP/DEF_RESTORE` — chủ động `mdelay(4500)` + `WDT REFRESH 0x1130, 10s` trong `setup_MotionEnable_Power()` (chờ ổn định relay cơ khí).
- **Không** thuộc nhóm 6 g_action tắt-WDT-sớm → watchdog **giữ nguyên hoạt động** xuyên suốt.
- **`autoboot_command()`**: `WDT REFRESH 0x11E2, 120s` ("IISW Update.\n") — cùng mức 120s với `USB_UPDATE`/`DEF_IISW`, khác hẳn nhóm tắt-hẳn (`BOOT_ERR`/`DEF_BACKUP`/`DEF_RESTORE`/...).
- **Warp**: `autoboot=AUTOBOOT_SSD_SUBSET` → không khớp điều kiện Warp, luôn cold-boot.

## 6. Tóm tắt

`DEF_UPDATE` là bước "thực thi" của một phiên cập nhật firmware đã được tải sẵn qua `DEF_IISW` từ trước — nhưng trước khi cam kết, `check_autoboot()` tự đọc lại và xác thực gói tin trên chính partition `0:A` nội bộ, sẵn sàng tự hạ cấp xuống `BOOT_ERR` nếu phát hiện dữ liệu hỏng; nếu qua được, boot vào một ảnh kernel/initrd riêng biệt (không chia sẻ với bất kỳ g_action nào khác), giữ watchdog 120s hoạt động liên tục và tắt hẳn cơ cấu cơ khí trong suốt quá trình cài đặt.