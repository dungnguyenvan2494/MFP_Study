# Flow bootmode của BootROM

Từ lúc `misc_init_r()` chạy sau relocation cho tới khi kernel được boot (hoặc rơi vào `cli_loop()`) — theo dõi từng quyết định: `g_action` được set ở đâu, secure chip được kiểm tra thế nào, SuperWarp được chọn hay bị từ chối, và watchdog được cấu hình ra sao cho từng nhánh.

![[Pasted image 20260815141718.png]]
![[Pasted image 20260815141735.png]]

# STAGE 01. check_action() — quyết định g_action


> [!NOTE] Path
> board/mvebu/armada8k/km/machine_setup.c:149 (setup_MotionEnable_Power) → common/main.c:568 (check_action)

Đây là quyết định **sớm nhất** trong toàn bộ flow — chạy trong `misc_init_r()`, tức là trước cả khi `main_loop()` tồn tại. Biến toàn cục `g_action` chi phối gần như mọi nhánh phía sau.

**Có USB cắm vào** (`check_usb_port() == 0`): đọc file `INDEX` trên USB để biết loại thao tác (`@ZZZ`, `@DWN`, `@SPD`); nếu không đọc được INDEX thì thử đọc phần đầu các file `.tar` đặc biệt để nhận diện line card / initialize / board-erase.

**Không có USB**: đọc trực tiếp DipSW ảo tại offset SPI Flash `0x000006` qua `getNWDLExec()` — giá trị 5/6 → IISW đang chạy, 4/7/8/9 → update, 10 → backup, 11 → restore, còn lại → boot bình thường (hoặc HW-checkmode nếu cờ đó bật).

| g_action          | Giá trị | Nguồn gốc                          | Ý nghĩa                                  |
| ----------------- | ------- | ---------------------------------- | ---------------------------------------- |
| DEF_BOOT          | 0       | mặc định                           | Boot bình thường từ SSD/SD               |
| USB_BOOT          | 1       | USB, INDEX = @ZZZ                  | Boot trực tiếp từ USB memory             |
| USB_UPDATE        | 2       | USB, INDEX = @DWN                  | Nạp firmware mới từ USB                  |
| DEF_IISW          | 3       | DipSW 0x06 = 5 hoặc 6              | Đang trong phiên IISW (network download) |
| BOOT_LINE         | 4       | USB tar, type khớp line-card       | Boot dành cho line card                  |
| BOOT_ERR          | 5       | USB sai loại máy / dữ liệu hỏng    | Lỗi — boot vào ảnh initrd báo lỗi        |
| BOOT_INIT         | 6       | USB tar @WVU + identifier đặc biệt | Factory reset (xoá SPI Flash user data)  |
| BOOT_HW_CHECKMODE | 7       | `uc_HWcheckModeBoot==1`            | Chế độ chẩn đoán phần cứng               |
| BOOT_SPEEDCHANGE  | 8       | USB, INDEX = @SPD                  | Khởi động lại ở tốc độ khác              |
| DEF_UPDATE        | 9       | DipSW 0x06 = 4/7/8/9               | Chế độ cập nhật IISW                     |
| DEF_BACKUP        | 11      | DipSW 0x06 = 10                    | Sao lưu dữ liệu                          |
| DEF_RESTORE       | 12      | DipSW 0x06 = 11                    | Khôi phục dữ liệu                        |
| BOOT_BDERASE      | 13      | USB tar @BDE + identifier đặc biệt | Xoá toàn bộ board (bảo mật / tái chế)    |

# STAGE 02. main_loop() — hai lối thoát không quay lại


> [!NOTE] Path
> common/main.c:2219

Ngay khi vào `main_loop()`, hai giá trị `g_action` có đặc quyền chặn toàn bộ flow còn lại: tắt watchdog rồi gọi thẳng hàm xử lý, hàm này **không bao giờ return** — máy đứng chờ người dùng tắt nguồn.

```c
common/main.c:2219–2229

if( g_action == BOOT_INIT ) {
    km_scpi_set_wdt_ca72_ext( KM_SCPI_WDT_CA72_END, 0, 0, 0, NULL );
    initialize_card_main();   /* không return */
}
if( g_action == BOOT_BDERASE) {
    km_scpi_set_wdt_ca72_ext( KM_SCPI_WDT_CA72_END, 0, 0, 0, NULL );
    bderase_card_main();      /* không return */
}
```

|                  | initialize_card_main()            | bderase_card_main()                                         |
| ---------------- | --------------------------------- | ----------------------------------------------------------- |
| USB tar file     | a789em.tar                        | a789be.tar                                                  |
| Type trong INDEX | @WVU                              | @BDE                                                        |
| Xoá gì           | Settings user, Warp info, cờ IISW | Toàn bộ SPI Flash kể cả security/certificate, serial number |
| Mục đích         | Trả máy về trạng thái factory     | Thu hồi / tái chế phần cứng                                 |

# STAGE 03. chk_sboot() — chốt chặn Secure Chip

> [!NOTE] Path
> common/main.c:2232

Trước khi tính đến bootdelay hay Warp, firmware xác minh một board bảo mật tuỳ chọn (_Secure Chip Board_) chưa từng bị tháo ra sau khi đã gắn. Trạng thái phần cứng đọc qua GPIO (`sysBspSecureChip()`) được đối chiếu với một cờ 8-byte lưu vĩnh viễn trong SPI Flash ở offset `THRD_SECAREA_SBOOT (0x28)`.

![[Pasted image 20260815142149.png]]

**Chống bypass:** một khi Secure Chip Board đã gắn và máy đã boot ít nhất một lần, không thể tháo chip ra mà vẫn boot được nữa — ngăn kẻ tấn công tháo chip để bỏ qua kiểm tra bảo mật, hoặc chuyển chip sang máy khác để giả mạo chứng nhận.

STAGE 04

# STAGE 04. check_autoboot() — ánh xạ g_action → lệnh boot


> [!NOTE] Path
> common/main.c:1870, gọi qua bootdelay_process(&autoboot)

Hàm này set `bootcmd`/`bootargs` vào environment variable và trả về một **loại autoboot** khác với `g_action` — dùng để quyết định abort-countdown, WDT timeout, và có cho phép Warp hay không.

|g_action|bootcmd nguồn từ|autoboot type|
|---|---|---|
|USB_BOOT|CONFIG_USBCMD_ENV|AUTOBOOT_FROM_USB|
|USB_UPDATE|CONFIG_USBCMD_SUBSET(_NORESET)_ENV|AUTOBOOT_USB_SUBSET|
|DEF_IISW|CONFIG_IISWCMD_ENV|AUTOBOOT_SSD_SUBSET|
|BOOT_LINE|CONFIG_CMD_ENV|AUTOBOOT_LINE|
|BOOT_ERR|CONFIG_ERRCMD_ENV|AUTOBOOT_SSD_SUBSET|
|BOOT_HW_CHECKMODE|CONFIG_CMD_ENV|AUTOBOOT_HW_CHECKMODE|
|BOOT_SPEEDCHANGE|CONFIG_CMD_ENV|AUTOBOOT_SPEEDCHANGE|
|DEF_UPDATE|CONFIG_IISWCMD_UPDATE_ENV|AUTOBOOT_SSD_SUBSET|
|DEF_BACKUP / DEF_RESTORE|CONFIG_IISWCMD_ENV|AUTOBOOT_SSD_SUBSET|
|DEF_BOOT (default)|CONFIG_CMD_ENV|AUTOBOOT_WARP nếu checkWarpBootMode(), ngược lại AUTOBOOT qua check_autoboot_DEF_BOOT_case()|

Nhánh `default` (DEF_BOOT) đi qua `check_autoboot_DEF_BOOT_case()` — hàm này xử lý riêng trường hợp firmware chính (MSC) trên SSD chưa tải xong: nếu đang trong quá trình download thì hạ cấp xuống `AUTOBOOT_SSD_SUBSET` (initrd) hoặc thậm chí `NOT_AUTOBOOT_NOTHING_BOOT_IMAGE` nếu không còn ảnh nào để chạy.

# STATE 05.Khối Warp!! — SuperWarp trước, Normal Warp sau

common/main.c:2238–2297 · điều kiện vào: AUTOBOOT_WARP hoặc (AUTOBOOT_LINE và checkWarpBootMode())

Chi tiết cơ chế SuperWarp/Normal Warp (hibernation snapshot, GPIO `WARP_SW`, so sánh cấu hình phần cứng qua `isChgWarpBootSettingInfo()`) đã được đào sâu ở tài liệu trước — ở đây chỉ chốt lại vị trí của nó trong flow tổng: đây là **nhánh rẽ duy nhất** có khả năng bỏ qua toàn bộ phần còn lại của `main_loop()` nếu `warp_boot()` thành công (hàm đó không return).

- **Thử SuperWarp trước** — chỉ khi phím panel không phải Trouble-Reset (`0x16`) và không có lỗi treo cờ `isDisableSuperWarp_ErrorOccurred()`. WDT được set 40 giây cho lần thử này.
- **Thất bại → xoá snapshot SuperWarp**, chuyển cờ khởi động sang Normal Warp, rồi thử lại với WDT 90 giây.
- **Cả hai thất bại** → rơi xuống (falls through), `autoboot` được tính lại qua `check_autoboot_DEF_BOOT_case()` — nghĩa là quay về boot F/W bình thường từ SSD, không còn cố Warp nữa trong lần chạy này.

**Ghi nhớ:** ngoài IISW update (`DEF_UPDATE`), mọi lần rời khỏi khối này đều xoá cả hai snapshot Normal + SuperWarp trên SSD/SD trước khi tiếp tục — tránh dùng nhầm snapshot cũ ở lần boot kế tiếp.

# STAGE 06.autoboot_command() — đếm ngược, watchdog, thực thi


> [!NOTE] Path
> common/autoboot.c:343

Đây là trạm cuối trước khi kernel nhận quyền điều khiển. `stored_bootdelay` (được `bootdelay_process()` lưu lại trước đó) quyết định có đếm ngược "Hit any key" hay không — với Warp, giá trị này bằng 0 nên bỏ qua bước chờ.

|g_action tại thời điểm chạy|WDT event|Timeout|Ghi chú|
|---|---|---|---|
|DEF_BOOT|REFRESH 0x11D2|300s|đồng thời gọi `chkGmUpdate()` kiểm tra Golden-Master firmware|
|USB_UPDATE|REFRESH 0x11E0|120s|—|
|DEF_IISW|REFRESH 0x11E1|120s|—|
|DEF_UPDATE|REFRESH 0x11E2|120s|—|
|còn lại|END|—|tắt watchdog, không cần refresh định kỳ|

Nếu `abortboot()` phát hiện phím bấm trong lúc đếm ngược, toàn bộ nhánh boot bị bỏ qua và firmware rơi thẳng xuống `cli_loop()` — shell dòng lệnh U-Boot tiêu chuẩn, dùng cho debug tại xưởng.