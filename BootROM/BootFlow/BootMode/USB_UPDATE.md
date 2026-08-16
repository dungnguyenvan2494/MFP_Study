# Trình bày USB_UPDATE

`USB_UPDATE` (giá trị `2`) là kịch bản **nạp/cài đặt firmware mới từ USB** — khác hẳn `USB_BOOT` (boot thẳng OS từ USB) và cũng khác `DEF_UPDATE` (giá trị `9`, kích hoạt bởi DipSW chứ không phải USB — hai cái tên rất dễ nhầm nhau dù cơ chế hoàn toàn tách biệt).

## 1. Vòng đời USB_UPDATE qua từng trạm

|Trạm|Điều gì xảy ra riêng cho USB_UPDATE|
|---|---|
|**setup_MotionEnable_Power()**|`USB_UPDATE` nằm trong **danh sách loại trừ** ở nhánh "utility key không active" (dòng 274-277) — nghĩa là Motion Enable **không** được bật; và cũng không khớp bất kỳ `else if` riêng nào (chỉ DEF_BOOT/USB_BOOT/BOOT_LINE có nhánh riêng) → giữ nguyên `uc_MotionEnable = 0`. Không thuộc nhóm "chờ relay 4.5s" (`{DEF_IISW,DEF_UPDATE,DEF_BACKUP,DEF_RESTORE}`), cũng không thuộc nhóm "tắt WDT sớm" — tức WDT vẫn chạy tiếp tới `autoboot_command()`.|
|**check_action()**|Đọc `INDEX` thành công, tên máy khớp, `sysBspGetUsbMemType()` trả `USB_DOWN` → `g_action = USB_UPDATE` (main.c:766-771).|
|**chk_sboot()**|Áp dụng bình thường, không ngoại lệ.|
|**check_autoboot()**|`case USB_UPDATE` set `autoboot = AUTOBOOT_USB_SUBSET` trực tiếp — **không** qua `check_autoboot_DEF_BOOT_case()`. Có thêm một `if/else` **chỉ riêng g_action này có**: gọi `sysBspGetAbort()` để chọn 1 trong 2 biến thể `bootcmd` (chi tiết mục 2).|
|**Khối Warp!!**|Bị bỏ qua — `AUTOBOOT_USB_SUBSET` không khớp điều kiện vào Warp, giống USB_BOOT.|
|**autoboot_command()**|`g_action == USB_UPDATE` → `WDT REFRESH 0x11E0, 120s` ("USB Update.\n") — khớp đúng với comment 2022/11/04 đã thấy ở `setup_MotionEnable_Power()`: "USB download/IISW giữ WDT active xuyên suốt", không tắt sớm như USB_BOOT.|

## 2. Điểm đặc biệt: `sysBspGetAbort()` quyết định 2 biến thể `bootcmd`

Đây là câu hỏi còn treo từ lượt trước — giờ đã lần ra nguồn gốc thật:

```c
static bool b_abort = false;               // autoboot.c:196 / main.c
void sysBspSetAbort(void) { b_abort = true; }
```

`sysBspSetAbort()` **chỉ được gọi ở đúng một nơi**: `abortboot_normal()` (autoboot.c:230-235), khi người dùng thực sự **bấm phím** trong lúc đếm ngược "Hit any key to stop autoboot" — tức là autoboot bị **hủy giữa chừng**, watchdog bị tắt (`WDT_CA72_END`), và firmware rơi vào `cli_loop()` (shell tương tác). Nói cách khác: `b_abort == true` nghĩa là _"phiên boot này từng bị người dùng ngắt để vào CLI"_.

```c
case USB_UPDATE:
    if (sysBspGetAbort() == true) {
        setenv("bootcmd", make_bootcmds(CONFIG_USBCMD_SUBSET_ENV));         // CÓ "usb reset;"
    } else {
        setenv("bootcmd", make_bootcmds(CONFIG_USBCMD_SUBSET_NORESET_ENV)); // KHÔNG có "usb reset;"
    }
```

- **`sysBspGetAbort()==false`** (đường tự động, phổ biến nhất): USB đã được `usb_init()`/`usb_stor_scan()` xong xuôi ngay trong `setup_MotionEnable_Power()` từ trước rồi → không cần reset lại, dùng `CONFIG_USBCMD_SUBSET_NORESET_ENV`.
- **`sysBspGetAbort()==true`** (đường thủ công — kỹ thuật viên đã dừng autoboot, gõ lệnh ở CLI để tự kích hoạt update): có thể đã có thời gian trôi qua hoặc USB bị thay, nên phải `usb reset` lại từ đầu → dùng `CONFIG_USBCMD_SUBSET_ENV`.

### Nội dung bootcmd khai triển (khác USB_BOOT ở tham số `DIR`):

```c
#define CONFIG_USBCMD_SUBSET_ENV          CMD_INITRD_ENV(USB_INIT_CMD, "usb", "0", FW_DIRECTRY "/")
#define CONFIG_USBCMD_SUBSET_NORESET_ENV  CMD_INITRD_ENV_NOINIT(       "usb", "0", FW_DIRECTRY "/")
```

```
[usb reset;]                                  ← chỉ có nếu sysBspGetAbort()==true
fatload usb 0 $kernel_addr FW9020/$bootfile;
fatload usb 0 $fdt_addr    FW9020/$dtb_name;
fatload usb 0 $initrd_addr FW9020/$initrd_name;
booti $kernel_addr $initrd_addr $fdt_addr
```

**Khác biệt cốt lõi so với `CONFIG_USBCMD_ENV` của USB_BOOT**: tham số `DIR` ở đây là `FW_DIRECTRY "/"` (ví dụ `"FW9020/"`) thay vì chuỗi rỗng `""` — nghĩa là USB_UPDATE boot một **kernel/initrd "Subset" nằm trong thư mục firmware riêng** trên USB (đúng như tên macro "Subset-USB"), tức một môi trường Linux tối giản chuyên dùng để _thực hiện_ việc flash firmware, tách biệt hẳn khỏi ảnh OS đầy đủ mà USB_BOOT nạp thẳng từ gốc USB.

## 3. Nội dung `bootargs`

```c
make_bootargs(USB_UPDATE, CONFIG_USB_RFS_ENV)
```

```
boottyp=2 root=/dev/sdc3 rootdelay=10 [nopanel] console=ttyS5,115200
TerminalState=<...> MachineInfo=<hex> MachineNo=<hex> SerialNo=<...>
IISW=0 FwDir=FW9020 DevStr=nvme IntegrityState=<0|1> <CONFIG_BOOTARGS>
```

Giống hệt cấu trúc USB_BOOT (cùng dùng `CONFIG_USB_RFS_ENV`, root nằm trên USB) — chỉ khác `boottyp=2`. `IISW=0` vẫn đúng vì `USB_UPDATE` không thuộc `{DEF_IISW, DEF_UPDATE}` — userspace "Subset" cần tự dựa vào `boottyp=2` để biết đây là phiên update-từ-USB, không phải IISW.

## 4. Tóm tắt

`USB_UPDATE` boot một môi trường Linux tối giản ("Subset") từ thư mục firmware riêng trên chính USB để thực hiện việc cập nhật, không bật cơ cấu cơ khí, giữ watchdog 120s xuyên suốt (không tắt như USB_BOOT), và tự chọn có cần `usb reset` lại hay không tuỳ vào việc phiên boot này đến từ tự động hay từ người dùng chủ động ngắt autoboot để gõ lệnh — một chi tiết chỉ có ở USB_UPDATE trong toàn bộ 13 g_action.

---

Tiếp theo bạn muốn `DEF_IISW`/`DEF_UPDATE` (cặp gần giống USB_UPDATE nhưng kích hoạt từ DipSW, cùng dùng `CONFIG_IISWCMD_ENV`/`CONFIG_IISWCMD_UPDATE_ENV`), hay nhảy sang `DEF_BACKUP`/`DEF_RESTORE` (điểm nghi vấn từ đầu — dùng chung ENV với IISW)?