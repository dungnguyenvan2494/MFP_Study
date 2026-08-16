# Trình bày DEF_BOOT

`DEF_BOOT` (giá trị `0`) là **hành động boot mặc định** — kịch bản "không có gì đặc biệt xảy ra": không USB, không DipSW nào bật, không lỗi. Nó không phải là code path đơn giản nhất trong 13 g_action, mà ngược lại là con đường **dài và có nhiều nhánh nhất**, vì đây là kịch bản chính máy chạy hàng ngày — nơi toàn bộ cơ chế Warp, Golden-Master, watchdog dài hạn đều được kích hoạt.

## 1. Vòng đời DEF_BOOT qua từng trạm

|Trạm|Điều gì xảy ra riêng cho DEF_BOOT|
|---|---|
|**setup_MotionEnable_Power()**|Nhánh không-USB, chuỗi if/else "rơi" xuống tận `else` cuối cùng → `g_action = DEF_BOOT`. Motion Enable được tính là **bật** (`gPanelKey != 0x16`), nhưng việc ghi GPIO thật sự **bị bỏ qua** vì điều kiện `(g_action != DEF_BOOT)` sai — máy tin tưởng Sub-CPU đã set sẵn. Không thuộc nhóm "tắt WDT" (dòng 335-338), nên watchdog vẫn chạy tiếp xuyên suốt.|
|**check_action()**|Không liên quan trực tiếp — DEF_BOOT là giá trị _mặc định_ (`int g_action = DEF_BOOT;` tại main.c:539), không phải giá trị được _gán chủ động_ bởi USB.|
|**chk_sboot()**|Áp dụng như mọi g_action khác — kiểm tra Secure Chip, không có ngoại lệ cho DEF_BOOT.|
|**check_autoboot()**|Rơi vào nhánh `default` — đây là nhánh **duy nhất** có logic phụ: kiểm tra `checkWarpBootMode()` để quyết định `AUTOBOOT_WARP` hay không, rồi gọi `check_autoboot_DEF_BOOT_case()`.|
|**check_autoboot_DEF_BOOT_case()**|Kiểm tra `getMSCDownloading()` — nếu firmware chính (MSC) trên SSD **chưa tồn tại đầy đủ** (đang dở dang download), hạ cấp `autoboot` xuống `AUTOBOOT_SSD_SUBSET`; nếu cả subset cũng không có ảnh nào để chạy (`getMSCSubsetDownloading()`), hạ tiếp xuống `NOT_AUTOBOOT_NOTHING_BOOT_IMAGE` — tức **không boot kernel gì cả**.|
|**Khối Warp!!**|Chỉ chạy nếu `autoboot == AUTOBOOT_WARP`. Thử SuperWarp (40s WDT) → Normal Warp (90s WDT) → nếu cả hai fail, `autoboot` được tính lại **lần nữa** qua `check_autoboot_DEF_BOOT_case()` — quay về boot F/W bình thường.|
|**autoboot_command()**|WDT `REFRESH 0x11D2, 300s` + gọi `chkGmUpdate()` kiểm tra/khôi phục firmware Boot/LPPP từ Golden-Master **trước khi** thực thi `run_command_list(s)`. Đây là g_action duy nhất có timeout dài nhất (300s so với 120s của USB_UPDATE/DEF_IISW/DEF_UPDATE).|

## 2. Nội dung `bootcmd` thật sự (giải mã `CONFIG_CMD_ENV`)

```c
#define CONFIG_CMD_ENV  CMD_INITRD_ENV("%s","%s","0:5","")
```

`make_bootcmds(CONFIG_CMD_ENV)` điền 2 placeholder `%s` tuỳ theo `isBootFromSD()`:

```
# Nếu boot từ SD:
mmc rescan;
fatload mmc 0:5 $kernel_addr $bootfile;
fatload mmc 0:5 $fdt_addr $dtb_name;
fatload mmc 0:5 $initrd_addr $initrd_name;
booti $kernel_addr $initrd_addr $fdt_addr

# Nếu boot từ SSD (mặc định):
nvme init;
fatload nvme 0:5 $kernel_addr $bootfile;
fatload nvme 0:5 $fdt_addr $dtb_name;
fatload nvme 0:5 $initrd_addr $initrd_name;
booti $kernel_addr $initrd_addr $fdt_addr
```

Điểm đáng chú ý: **partition `0:5`** — khác hẳn `0:2` của DEF_IISW/BOOT_ERR hay `0:A` của DEF_UPDATE. Đây chính là "công tắc phân vùng" thật sự phân biệt các g_action ở tầng bootcmd, dù nhiều g_action dùng chung macro `CMD_INITRD_ENV`.

## 3. Nội dung `bootargs` thật sự (giải mã `make_bootargs(DEF_BOOT, CONFIG_SSD_RFS_ENV)`)

```
boottyp=0 root=/dev/kmsda7 rootdelay=1 [nopanel] console=ttyS5,115200
TerminalState=<term_state> MachineInfo=<hex> MachineNo=<hex> SerialNo=<...>
IISW=0 FwDir=FW0000 DevStr=nvme IntegrityState=<0|1> <CONFIG_BOOTARGS board-specific>
```

Giải nghĩa từng trường:

- **`boottyp=0`** — chính là giá trị `g_action` (DEF_BOOT=0), được kernel/userspace đọc lại để biết mình được BootROM boot theo kịch bản nào.
- **`IISW=0`** — vì DEF_BOOT không thuộc `{DEF_IISW, DEF_UPDATE}` (dòng 1602-1603), userspace biết đây không phải phiên IISW.
- **`console=...`** — chọn `ttyS5` (CSRC-UART) hoặc `ttyS0` (AP-UART) tuỳ `gd->arch.term_sel`, hoặc `tty1` ảo nếu `TERM_STATE_0` (panel đang hiển thị, không cần UART debug).
- **`DevStr=nvme`/`mmc`** — báo cho userspace biết thiết bị lưu trữ thật để tự mount đúng root filesystem tương ứng với `root=/dev/kmsda7` (SSD) đã set sẵn trong `CONFIG_SSD_RFS_ENV`.
- **`IntegrityState`** — cờ bảo mật/toàn vẹn từ `getInergrity()` (CQ501194), không thấy phân tích sâu trước đây — có thể là điểm đáng hỏi thêm.

Đây chính là **hợp đồng bàn giao** giữa BootROM và kernel/init userspace — toàn bộ quyết định của 7 trạm ở trên cuối cùng đọng lại thành một dòng `bootargs` duy nhất này.

## 4. Tóm tắt

`DEF_BOOT` là kịch bản boot production hàng ngày của máy: không cần USB, tự trọng tài giữa Warp (nếu điều kiện phần cứng cho phép) và cold-boot bình thường qua partition `0:5`, luôn đi kèm kiểm tra/khôi phục firmware Golden-Master và watchdog dài nhất hệ thống (300s), rồi bàn giao cho kernel một `bootargs` mã hoá đầy đủ ngữ cảnh (`boottyp`, `IISW`, thiết bị, thông tin máy) để userspace tự cấu hình tiếp theo đúng kịch bản.

---

Câu hỏi tự nhiên tiếp theo (theo đúng bộ câu hỏi đã gợi ý ở lượt trước): bạn có muốn đi sâu `getMSCDownloading()`/`getMSCSubsetDownloading()` — tức là **khi nào SSD bị coi là "chưa có firmware chính"**, tình huống khiến DEF_BOOT tụt xuống subset hoặc "không có gì để boot"?