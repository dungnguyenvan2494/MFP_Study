# Trình bày DEF_RESTORE

`DEF_RESTORE` (giá trị `12`) là chế độ **khôi phục dữ liệu** — về mặt kỹ thuật, đây là bản sao gần như **hoàn toàn giống `DEF_BACKUP`**, chỉ khác đúng 2 con số (DipSW trigger và `boottyp`). Đây cũng là g_action cuối cùng trong 13 giá trị — hoàn tất toàn bộ bức tranh.

## 1. Nguồn gốc

DipSW ảo offset `0x06 == 11`:

```c
// setup_MotionEnable_Power() (machine_setup.c:241-243)
else if (11 == uc_IISWFlg) { g_action = DEF_RESTORE; }

// check_action() nhánh dự phòng nội bộ (main.c)
else if ( 11 == uc_NWDLExec ) { g_action = DEF_RESTORE; return DEF_RESTORE; }
```

Cũng như `DEF_BACKUP`, không có nhánh USB nào trực tiếp set `DEF_RESTORE` — thuần tuý DipSW.

## 2. `check_autoboot()` — cùng ảnh, cùng cơ chế với DEF_BACKUP

```c
case DEF_RESTORE:
    setenv("bootcmd",  make_bootcmds(CONFIG_IISWCMD_ENV));   // partition 0:2 — GIỐNG DEF_BACKUP/DEF_IISW/BOOT_ERR
    setenv("bootargs", make_bootargs(g_action,CONFIG_SSD_RFS_ENV));
    autoboot = AUTOBOOT_SSD_SUBSET;
```

Không có bước xác thực lại (giống `DEF_BACKUP`, khác `DEF_UPDATE`).

```
nvme init;                                    # (hoặc "mmc rescan;")
fatload nvme 0:2 $kernel_addr $bootfile;
fatload nvme 0:2 $fdt_addr    $dtb_name;
fatload nvme 0:2 $initrd_addr $initrd_name;
booti $kernel_addr $initrd_addr $fdt_addr
```

## 3. Nội dung `bootargs`

```
boottyp=12 root=/dev/kmsda7 rootdelay=1 [nopanel] console=...
TerminalState=<...> MachineInfo=<hex> MachineNo=<hex> SerialNo=<...>
IISW=0 FwDir=FW9020 DevStr=nvme IntegrityState=<0|1> <CONFIG_BOOTARGS>
```

Chỉ khác `DEF_BACKUP` ở `boottyp=12` — cùng chạy trên một binary `0:2`, cùng `IISW=0`. Logic thật sự "khôi phục từ đâu, ghi vào đâu" nằm hoàn toàn ở userspace của ảnh Subset OS này, ngoài phạm vi BootROM.

## 4. Motion Enable & Watchdog — giống hệt DEF_BACKUP

- **Motion Enable**: trong danh sách loại trừ → tắt.
- **Nhóm delay**: cùng nhóm `DEF_IISW/DEF_UPDATE/DEF_BACKUP` → `mdelay(4500)` + `WDT REFRESH 0x1130, 10s`.
- **Nhóm tắt-WDT-sớm**: có mặt (`{DEF_BACKUP, DEF_RESTORE, BOOT_HW_CHECKMODE, BOOT_ERR, BOOT_SPEEDCHANGE, BOOT_LINE}`) → `WDT_CA72_END` sớm.
- `autoboot_command()`: không có case riêng → `else` → `WDT_CA72_END`.
- **Warp**: `autoboot=AUTOBOOT_SSD_SUBSET` → không khớp, luôn cold-boot.

## 5. Tóm tắt

`DEF_RESTORE` là ảnh gương của `DEF_BACKUP`: cùng kích hoạt qua DipSW (`0x06==11`), cùng boot vào ảnh Subset OS chung với `DEF_IISW/DEF_BACKUP/BOOT_ERR` trên partition `0:2`, không có bước xác thực bổ sung, tắt cơ khí và watchdog trong suốt quá trình — chỉ phân biệt với `DEF_BACKUP` qua `boottyp=12` mà userspace tự đọc để biết chạy chiều "ghi vào máy" thay vì "đọc từ máy".

---

Đến đây đã trình bày đầy đủ cả **13/13 g_action**. Bạn có muốn mình tổng hợp lại thành một bảng/artifact hoàn chỉnh cuối cùng (bổ sung các phát hiện mới — vùng SPI Flash thật của BOOT_INIT, cơ chế validate của DEF_UPDATE, ép TERM_STATE của BOOT_LINE, 4 nguồn gốc của BOOT_ERR...) vào artifact "Flow bootmode" đã có từ trước, hay để nguyên dạng hội thoại này?