# Trình bày BOOT_INIT

`BOOT_INIT` (giá trị `6`) là lệnh **factory reset ở cấp phần cứng** — không boot kernel, không qua bất kỳ trạm nào của `check_autoboot()`/`autoboot_command()`, mà chặn thẳng ngay đầu `main_loop()` và xoá một tập hợp vùng SPI Flash cụ thể trước khi dừng máy vĩnh viễn chờ tắt nguồn.

## 1. Nguồn gốc — điều kiện kích hoạt rất chặt

Từ `check_action()` (main.c:679-719), phải thoả **đồng thời 3 điều kiện**:

```c
#define TAR_FILE_NAME_INIT   "a789em.tar"      // (hoặc "a789es.tar" — TAR_FILE_NAME_INIT_2, riêng cho Shindoh/HeliosMLK)
#define TYP_NAME_INIT         "@TYP=GaiaPF"    // tên loại phải khớp đúng chuỗi này
#define DEF_INITIALIZE_CARD_IDENFITIFER    "@RGA=600A8555B78A745E1245E57D299110DA58D359ED158C6969CF9D32DF4E29ECB6"
#define DEF_INITIALIZE_CARD_IDENFITIFER_2  "@RGA=A216AD774728041E1F95305095A3D8F8D90BE6F114AE8BE738B5A0FF47069B7D"
```

1. USB phải chứa đúng file **`a789em.tar`** (hoặc `a789es.tar`).
2. Type trong header tar phải khớp chuỗi **`@TYP=GaiaPF`**.
3. `sysBspGetUsbMemType()` phải trả về `USB_WVU`, **và** nội dung header phải chứa đúng chuỗi hex 64-ký-tự `@RGA=600A8555...` (hoặc biến thể `_2`) — đóng vai trò như "mật khẩu" chống kích hoạt nhầm.

Thiếu bất kỳ điều kiện nào → rơi về `DEF_BOOT` bình thường, không có cảnh báo trung gian.

## 2. Chặn ngay đầu main_loop() — không đi qua bất kỳ trạm nào khác

```c
if( g_action == BOOT_INIT ) {
    km_scpi_set_wdt_ca72_ext( KM_SCPI_WDT_CA72_END, 0, 0, 0, NULL );
    initialize_card_main();     /* không quay lại */
}
```

`chk_sboot()`, `check_autoboot()`, `make_bootcmds()`/`make_bootargs()`, Warp — **tất cả bị bỏ qua**. Không có khái niệm `bootcmd`/`bootargs` cho BOOT_INIT.

```c
void initialize_card_main( void )
{
    clear_spi_flash_for_init_card();
    printf( "done.\n\n" );
    DrawPanel( DEF_Manager_Update );          // vẽ màn hình "hoàn thành"
    printf( "You can turn off the power.\n" );
    while( 1 ) { }                             // dừng vĩnh viễn, chờ tắt nguồn
}
```

## 3. `clear_spi_flash_for_init_card()` — xoá chính xác những vùng nào

Đây là phần mình vừa xác minh trực tiếp từ `userdata.c` — **chi tiết hơn nhiều** so với mô tả chung chung trước đây ("xoá settings user"). Hàm này duyệt qua bảng `flash_map_init_card[]`:

|Vùng SPI Flash|Nội dung|Xử lý|
|---|---|---|
|`0x00000`–`0x50000` (NVRAM Backup data 1 & 2)|Dữ liệu backup NVRAM|**BỊ COMMENT — KHÔNG xoá!**|
|`0x6E000`, `0x72000`|SoftDipSW data (2 bản sao)|Erase + ghi lại marker "NV clear flag" (`0xAB`) để phần đọc sau biết cần dùng giá trị mặc định|
|`0x76000`|Vùng lưu trạng thái mất điện|Erase thuần|
|`0x77000`, `0x78000`|Dữ liệu BSP/Thread (2 bản sao)|Erase (giữ nguyên 4 byte đầu/cuối làm checksum, nhờ cờ `DUAL_LAYER`)|
|`0x7B000`|**Vùng HW-checkmode** — chính là nơi lưu `uc_MSCDLFlg` đã gặp ở `setup_MotionEnable_Power()`/`BOOT_ERR`|Erase|
|`0x7C000`–`0x7F000`|4 vùng debug sequence (BSP debug)|Erase nhưng **giữ nguyên 4 byte đầu** (một bộ đếm)|

**Phát hiện quan trọng nhất — sửa lại thông tin trước đây**: `BOOT_INIT` **không xoá dữ liệu NVRAM backup** (`0x00000`–`0x50000`) — các dòng đó tồn tại trong bảng nhưng bị **comment-out có chủ đích**, kèm chú thích tiếng Nhật dịch được là "không clear". Đây chính là điểm khác biệt cốt lõi so với `BOOT_BDERASE`, hàm dùng một bảng khác (`flash_map_bderase_card[]`) — **có** xoá đúng những vùng NVRAM backup này, cộng thêm một vùng bảo mật riêng ở `0x3FE000`.

Nói cách khác: `BOOT_INIT` = "đưa máy về cấu hình mặc định nhà máy nhưng vẫn giữ lại dữ liệu backup NVRAM" — còn `BOOT_BDERASE` mới thực sự xoá sạch không chừa gì.

## 4. Tóm tắt

`BOOT_INIT` là factory-reset có điều kiện kích hoạt rất chặt (đúng tên file, đúng type, đúng mã xác thực 64-hex), chặn thẳng đầu `main_loop()` để không bao giờ chạm tới bootcmd/bootargs/Warp, chỉ xoá các vùng cấu hình vận hành (SoftDipSW, BSP/Thread, HW-checkmode, debug sequence) trong khi **cố tình giữ nguyên** dữ liệu backup NVRAM — rồi dừng máy vĩnh viễn chờ tắt nguồn.