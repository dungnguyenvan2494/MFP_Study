
# Trình bày USB_BOOT

`USB_BOOT` (giá trị `1`) là kịch bản **boot trực tiếp kernel/firmware từ USB memory** — khác hẳn `USB_UPDATE` (chỉ _nạp_ firmware mới rồi vẫn boot SSD) hay `BOOT_LINE` (boot line card). Đây là hành động **duy nhất được set trực tiếp bởi người vận hành cắm đúng loại USB service** vào máy, không qua DipSW.

## 1. Vòng đời USB_BOOT qua từng trạm

|Trạm|Điều gì xảy ra riêng cho USB_BOOT|
|---|---|
|**setup_MotionEnable_Power()**|Chỉ đạt tới nhánh này khi USB đã được xác nhận (A.3 đúng) và `check_action()` bên trong nó trả về `USB_BOOT`. Trong bảng Motion-Enable, `g_action == USB_BOOT` là **một nhánh riêng biệt luôn bật** `uc_MotionEnable = 1` — dù `uc_TempUtilityKey` active hay không.|
|**check_action()**|Đọc file `INDEX` thành công → xác nhận **tên máy khớp** (bắt buộc, không được miễn như `USB_SPD`) → `sysBspGetUsbMemType()` trả về `USB_ZZZ` → `g_action = USB_BOOT`, return ngay (main.c:772-777). Không cần thử các file `.tar` dự phòng vì INDEX đã đọc được ngay từ đầu.|
|**chk_sboot()**|Áp dụng như mọi g_action — không có ngoại lệ.|
|**check_autoboot()**|Có `case USB_BOOT` **riêng biệt**, tách hẳn khỏi nhánh `default` của DEF_BOOT: set `bootcmd`/`bootargs`, `initrd_high=ffffffff`, `autoboot = AUTOBOOT_FROM_USB` — rồi return ngay, **không** đi qua `check_autoboot_DEF_BOOT_case()`.|
|**Khối Warp!!**|**Bị bỏ qua hoàn toàn.** Điều kiện vào Warp chỉ chấp nhận `AUTOBOOT_WARP` hoặc `AUTOBOOT_LINE`; `AUTOBOOT_FROM_USB` không khớp cả hai → USB_BOOT luôn cold-boot, không bao giờ thử hibernation-resume.|
|**autoboot_command()**|`g_action` không khớp bất kỳ case nào trong `{DEF_BOOT, USB_UPDATE, DEF_IISW, DEF_UPDATE}` → rơi vào `else` → **`WDT_CA72_END` — tắt hẳn watchdog** trước khi boot. Khác hẳn DEF_BOOT (300s tự refresh).|
|**abortboot()**|Không bị ép về 0 như đường Warp — dùng `bootdelay` đọc bình thường từ env (`CONFIG_BOOTDELAY`), nghĩa là màn hình "Hit any key to stop autoboot" **có thể xuất hiện** trước khi thực thi USB_BOOT.|

## 2. Nội dung `bootcmd` thật sự (giải mã `CONFIG_USBCMD_ENV`)

```c
#define CONFIG_USBCMD_ENV  CMD_INITRD_ENV(USB_INIT_CMD, "usb", "0", "")
// USB_INIT_CMD = "usb reset"
```

Khai triển ra:

```
usb reset;
fatload usb 0 $kernel_addr $bootfile;
fatload usb 0 $fdt_addr $dtb_name;
fatload usb 0 $initrd_addr $initrd_name;
booti $kernel_addr $initrd_addr $fdt_addr
```

**Điểm tinh tế đáng chú ý**: `make_bootcmds()` được viết chung cho mọi loại bootcmd, cố `sprintf(tmp, env, MMC_INIT_CMD/NVME_INIT_CMD, "mmc"/"nvme", ...)` để tự động chọn thiết bị nội bộ theo `isBootFromSD()`. Nhưng vì `CONFIG_USBCMD_ENV` đã được tiền xử lý (preprocessor) thành một chuỗi **không còn ký tự `%s`** nào (khác với `CONFIG_CMD_ENV` giữ nguyên `"%s","%s"` làm placeholder), nên lệnh `sprintf` với các tham số `MMC_INIT_CMD`/`NVME_INIT_CMD` **hoàn toàn vô tác dụng** ở đây — chuỗi trả về luôn y hệt `"usb reset; ..."` bất kể máy đang có SSD hay SD. Đây là ví dụ về một hàm helper dùng chung nhưng chỉ thực sự "tham số hoá" với một nhóm config nhất định.

## 3. Nội dung `bootargs` thật sự (giải mã `make_bootargs(USB_BOOT, CONFIG_USB_RFS_ENV)`)

```c
#define CONFIG_USB_RFS_ENV   "root=/dev/sdc3 rootdelay=10"   // (hoặc /dev/mmcblk1p3 nếu CONFIG_SD_BOOT)
```

```
boottyp=1 root=/dev/sdc3 rootdelay=10 [nopanel] console=ttyS5,115200
TerminalState=<...> MachineInfo=<hex> MachineNo=<hex> SerialNo=<...>
IISW=0 FwDir=FW9020 DevStr=nvme IntegrityState=<0|1> <CONFIG_BOOTARGS>
```

- **`boottyp=1`** — chính giá trị `USB_BOOT`.
- **`root=/dev/sdc3 rootdelay=10`** — root filesystem nằm **trên chính USB memory**, khác hẳn `root=/dev/kmsda7` (SSD nội bộ) của DEF_BOOT — và `rootdelay` dài hơn (10s so với 1s) vì thiết bị USB cần thời gian enumerate lâu hơn ổ SSD/eMMC nội bộ.
- **`DevStr=nvme`** — vẫn phản ánh thiết bị lưu trữ **nội bộ** (SSD/SD) chứ không phải USB — trường này generic cho mọi bootargs bất kể nguồn boot thật sự là gì, nên với USB_BOOT nó hơi "lạc đề" (không dùng tới) nhưng vẫn được điền.
- **`IISW=0`** — vì USB_BOOT không thuộc `{DEF_IISW, DEF_UPDATE}`.

## 4. Tóm tắt

`USB_BOOT` là lối tắt "boot thẳng từ USB service" dành cho kỹ thuật viên: kích hoạt khi cắm đúng USB có `INDEX` chứa loại `@ZZZ` và đúng tên máy, luôn bật cơ cấu cơ khí, luôn cold-boot (bỏ qua hoàn toàn Warp), luôn tắt watchdog trước khi chạy (không tự phục hồi nếu treo — đúng ý đồ vì đây là thao tác có người đứng giám sát), và root filesystem trỏ thẳng vào chính USB đó thay vì SSD nội bộ.

---

Muốn tiếp tục với `USB_UPDATE` (rất gần USB_BOOT nhưng thêm biến số `sysBspGetAbort()` quyết định 2 biến thể bootcmd khác nhau), hay chuyển hướng sang nhóm `DEF_IISW`/`DEF_UPDATE`/`DEF_BACKUP`/`DEF_RESTORE` (đang dùng chung ENV, cần làm rõ khác biệt thật sự)?