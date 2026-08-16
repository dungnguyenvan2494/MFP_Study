# Trình bày BOOT_LINE

`BOOT_LINE` (giá trị `4`) là kịch bản boot dành cho **line card** — một module mở rộng cắm thêm vào máy MFP. Đây là g_action đáng chú ý nhất trong nhóm còn lại vì nó **chia sẻ gần như mọi hạ tầng với DEF_BOOT**: cùng bootcmd, cùng partition, và là g_action **duy nhất thứ hai** (sau DEF_BOOT) có thể đi vào khối Warp!!.

## 1. Vòng đời BOOT_LINE qua từng trạm

|Trạm|Điều gì xảy ra riêng cho BOOT_LINE|
|---|---|
|**setup_MotionEnable_Power()**|Không nằm trong danh sách loại trừ Motion-Enable (dòng 274-277) → nếu utility key không active vẫn bật `uc_MotionEnable=1`; ngoài ra có `else if(g_action==BOOT_LINE && gPanelKey!=0x16)` riêng cho trường hợp utility key active → cũng bật. Chỉ bị chặn nếu `gPanelKey==0x16` (Trouble-Reset). Nằm trong nhóm **tắt WDT sớm** (dòng 335-338, cùng nhóm `DEF_BACKUP/DEF_RESTORE/BOOT_HW_CHECKMODE/BOOT_ERR/BOOT_SPEEDCHANGE`).|
|**check_action()**|Đọc `INDEX` **thất bại** trước (không phải luồng chính) → rơi vào nhánh dò `.tar`: đọc phần đầu file tar line-card, `sysBspCmpTypeName()` khớp `TYP_NAME_LINECARD`, `sysBspGetUsbMemType()` trả `USB_XYZ` → `g_action = BOOT_LINE` (main.c:655-664). Đáng chú ý: **không cần mã xác thực dài** như `BOOT_INIT`/`BOOT_BDERASE` — chỉ cần đúng loại là đủ, vì line-card boot không phải thao tác phá hủy dữ liệu.|
|**chk_sboot()**|Áp dụng bình thường, không ngoại lệ.|
|**check_autoboot()**|`case BOOT_LINE`: `bootcmd = CONFIG_CMD_ENV`, `bootargs = make_bootargs(BOOT_LINE, CONFIG_SSD_RFS_ENV)`, `autoboot = AUTOBOOT_LINE`.|
|**Khối Warp!!**|Điều kiện vào Warp ở `main_loop()`: `AUTOBOOT_WARP==autoboot|
|**autoboot_command()**|`g_action==BOOT_LINE` không khớp `{DEF_BOOT, USB_UPDATE, DEF_IISW, DEF_UPDATE}` → rơi `else` → `WDT_CA72_END`. Ngoại lệ duy nhất: nếu vừa đi qua khối Warp thành công thì máy đã boot xong từ snapshot rồi (không bao giờ chạy tới `autoboot_command()` nữa), nên "END" ở đây chỉ áp dụng cho nhánh cold-boot.|

## 2. Nội dung `bootcmd` (giải mã `CONFIG_CMD_ENV`)

```c
#define CONFIG_CMD_ENV   CMD_INITRD_ENV("%s","%s","0:5","")   // giống hệt DEF_BOOT
```

```
nvme init;                                    # (hoặc "mmc rescan;" nếu boot từ SD)
fatload nvme 0:5 $kernel_addr $bootfile;
fatload nvme 0:5 $fdt_addr    $dtb_name;
fatload nvme 0:5 $initrd_addr $initrd_name;
booti $kernel_addr $initrd_addr $fdt_addr
```

**Không có ảnh firmware riêng cho line-card** — BOOT_LINE nạp **đúng kernel/initrd production** từ partition `0:5`, y hệt DEF_BOOT. Toàn bộ sự khác biệt hành vi nằm ở `bootargs`.

## 3. Nội dung `bootargs` — điểm đặc biệt nhất của BOOT_LINE

```c
if( BOOT_LINE == boottype ){
    term_state = TERM_STATE_1;   // main.c:1624-1626 — ÉP CỨNG, bỏ qua trạng thái thật
}
```

```c
// dipvalue.h
#define TERM_STATE_0    0    /* 非デバッグ : AP806 UART0 = None */
#define TERM_STATE_1    1    /* MFPデバッグ : AP806 UART0 = Term */
```

Bình thường, `term_state` do `getTermState()` tự xác định dựa trên cấu hình DipSW/log-force (`isForceLogOutput()`...) — nếu máy đang ở chế độ non-debug (`TERM_STATE_0`), console kernel chỉ là `console=tty1 loglevel=1` (virtual console, **không** UART). Nhưng với **BOOT_LINE, giá trị này bị ép cứng thành `TERM_STATE_1`** bất kể cấu hình thật của máy — rơi vào nhánh `default` của switch, kích hoạt `console=ttyS0,115200` hoặc `console=ttyS5,115200` tuỳ `gd->arch.term_sel`.

```
boottyp=4 root=/dev/kmsda7 rootdelay=1 [nopanel] console=ttyS5,115200   ← LUÔN có UART console
TerminalState=1 MachineInfo=<hex> MachineNo=<hex> SerialNo=<...>
IISW=0 FwDir=FW9020 DevStr=nvme IntegrityState=<0|1> <CONFIG_BOOTARGS>
```

**Vì sao ép cứng debug console?** Line card là thiết bị mở rộng vật lý mới cắm vào — khả năng cao đây là bối cảnh service/bring-up cần theo dõi log trực tiếp qua UART, nên BootROM chủ động bật debug console cho mọi lần boot line-card, không phụ thuộc việc kỹ thuật viên có nhớ bật DipSW debug hay không.

## 4. Tóm tắt

`BOOT_LINE` boot **cùng một ảnh firmware production** như DEF_BOOT (partition `0:5`) và là g_action duy nhất khác ngoài DEF_BOOT được phép thử SuperWarp/Normal-Warp — nhưng luôn ép console kernel sang chế độ UART-debug (`TERM_STATE_1`) bất kể cấu hình thật của máy, phản ánh đúng bản chất: đây là một biến thể "boot bình thường, nhưng luôn bật log để phục vụ debug line-card".