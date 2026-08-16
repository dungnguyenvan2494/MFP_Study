# Trình bày BOOT_BDERASE

`BOOT_BDERASE` (giá trị `13`) là lệnh **xoá board ở mức sâu nhất** — và sau khi tra lại đúng vùng SPI Flash mà nó xoá, phát hiện ra một kết nối thú vị: nó xoá **chính vùng "Secure Area"** chứa cờ chống-tháo-chip (`THRD_SECAREA_SBOOT`) đã phân tích ở tài liệu SuperWarp/`chk_sboot()` từ đầu cuộc trò chuyện này.

## 1. Nguồn gốc — cùng "type" với BOOT_INIT, khác class + mã xác thực

```c
#define TAR_FILE_NAME_BDERASE          "a789be.tar"
#define TYP_NAME_BDERASE               "@TYP=GaiaPF"      // ← GIỐNG HỆT TYP_NAME_INIT của BOOT_INIT!
#define DEF_BDERASE_CARD_IDENFITIFER   "@RGA=4ABC11345F6789AE4722FEC5550EBB8124448750BBAAFF332390DADA8536AAAF"
```

Thú vị: `BOOT_INIT` và `BOOT_BDERASE` dùng **chung một chuỗi type** `@TYP=GaiaPF` trong header tar — thứ phân biệt hai lệnh không phải "type" mà là **class USB** (`sysBspGetUsbMemType()` trả `USB_WVU` cho init, `USB_BDE` cho bderase, dựa trên field đầu tiên của header) cộng với **mã xác thực hex khác nhau hoàn toàn**. Điều kiện đủ (main.c:726-736): đúng file `a789be.tar` + type khớp + `g_UsbKind==USB_BDE` + chuỗi `@RGA=4ABC1134...` khớp trong header.

## 2. Chặn đầu `main_loop()` — không boot kernel, giống hệt cơ chế BOOT_INIT

```c
if( g_action == BOOT_BDERASE) {
    km_scpi_set_wdt_ca72_ext( KM_SCPI_WDT_CA72_END, 0, 0, 0, NULL );
    bderase_card_main();     /* không quay lại */
}
```

```c
void bderase_card_main( void ) {
    clear_spi_flash_for_bderase_card();
    printf( "Board Erase Finish.\n\n" );
    DrawPanel( DEF_Manager_Update );
    printf( "You can turn off the power.\n" );
    while( 1 ) { }
}
```

Không có `check_autoboot()`, không `bootcmd`/`bootargs`, không Warp — y hệt `BOOT_INIT` về cấu trúc.

## 3. `clear_spi_flash_for_bderase_card()` — xoá **rộng hơn hẳn** `BOOT_INIT`

|Vùng SPI Flash|Nội dung|So với `BOOT_INIT`|
|---|---|---|
|`0x00000`–`0x50000` (NVRAM Backup 1 & 2)|Dữ liệu backup NVRAM|**CÓ xoá** — trong khi `BOOT_INIT` cố tình bỏ qua (comment-out)|
|`0x6E000`, `0x72000`|SoftDipSW data|Xoá (không có bước ghi marker `0xAB` đặc biệt như `BOOT_INIT`)|
|`0x76000`|Vùng lưu trạng thái mất điện|Xoá — giống `BOOT_INIT`|
|`0x77000`, `0x78000`|BSP/Thread data|Xoá — giống `BOOT_INIT`|
|`0x7B000`|Vùng HW-checkmode|Xoá — giống `BOOT_INIT`|
|`0x7C000`–`0x7F000`|4 vùng debug sequence|Xoá (giữ 4 byte đầu) — giống `BOOT_INIT`|
|**`0x3FE000`**|**"セキュア領域" (Secure Area)**|**CHỈ CÓ ở BOOT_BDERASE — BOOT_INIT không đụng tới**|
|**`0x3FF000`**|**"Bレジスタ用領域" (vùng cho B-register)**|**CHỈ CÓ ở BOOT_BDERASE**|

## 4. Phát hiện quan trọng nhất — kết nối ngược lại `chk_sboot()`

```c
// mtd_flash_access.c:35
#define THRD_SecureInfoOffset   0x3fe000   // địa chỉ vùng Secure Area

// mtd_flash_access.h:55-59 — các offset TƯƠNG ĐỐI bên trong vùng 0x3fe000 đó:
#define THRD_SECAREA_RECOVERY  (0x00)   // thông tin phục hồi chip
#define THRD_SECAREA_VALIDERR  (0x10)   // lỗi xác thực IISW
#define THRD_SECAREA_GMBOOT    (0x18)   // yêu cầu cập nhật Golden-Master Boot
#define THRD_SECAREA_GMLPPP    (0x20)   // yêu cầu cập nhật Golden-Master LPPP
#define THRD_SECAREA_SBOOT     (0x28)   // ← CỜ "ĐÃ TỪNG GẮN SECURE CHIP BOARD" (đã phân tích ở đầu hội thoại)
```

`0x3FE000 - DEF_USERDATA_OFFSET (0x300000) + DEF_USERDATA_OFFSET = 0x3FE000` — khớp tuyệt đối với `THRD_SecureInfoOffset`. Điều này có nghĩa: **`BOOT_BDERASE` xoá sạch toàn bộ vùng Secure Area**, bao gồm cả `THRD_SECAREA_SBOOT` — cờ mà `chk_sboot()` dùng để chặn boot vĩnh viễn nếu Secure Chip Board bị tháo sau khi đã gắn.

**Ý nghĩa bảo mật**: đây chính là lý do thiết kế có 2 lệnh xoá tách biệt —

- `BOOT_INIT` (factory reset) **không** đụng tới Secure Area → nếu máy đã từng gắn Secure Chip, factory-reset **không thể** dùng để bypass cơ chế chống-tháo-chip.
- `BOOT_BDERASE` (thu hồi/tái chế board) **chủ động xoá** Secure Area → hợp lý vì đây là quy trình chính thức "khai tử" một board (thu hồi, tái chế phần cứng), cần xoá sạch mọi trạng thái bảo mật gắn với board đó trước khi nó rời khỏi vòng đời sản phẩm.

## 5. Tóm tắt

`BOOT_BDERASE` dùng chung điều kiện "type" (`@TYP=GaiaPF`) với `BOOT_INIT` nhưng khác class USB (`USB_BDE`) và mã xác thực riêng, chặn đầu `main_loop()` không boot kernel, xoá **toàn bộ** SPI Flash bao gồm cả dữ liệu NVRAM backup lẫn vùng Secure Area chứa cờ chống-tháo Secure Chip Board — chính là điểm khác biệt cốt lõi khiến nó trở thành lệnh "khai tử board" thực sự, chứ không chỉ là factory-reset như `BOOT_INIT`.

---

Đến đây đã hoàn tất trình bày chi tiết cho **cả 13/13 g_action**, với một vòng khép kín thú vị: bắt đầu từ `chk_sboot()`/Secure Chip (tài liệu SuperWarp đầu tiên) và kết thúc đúng tại g_action duy nhất thao tác lên chính vùng dữ liệu đó. Bạn có muốn mình tổng hợp toàn bộ phát hiện của cả loạt phân tích này vào artifact "Flow bootmode" không?