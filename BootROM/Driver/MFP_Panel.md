# mfp_panel.c — driver màn hình LCD panel ngoài qua UART
Đường dẫn gốc: `Src/board/mvebu/armada8k/km/mfp_panel.c` Board: Marvell Armada8k (mvebu), họ máy "KM", chạy trong U-Boot.

**Lưu ý về encoding**: toàn bộ comment trong file gốc là tiếng Nhật lưu ở Shift-JIS, nhưng bị đọc nhầm sang UTF-8 nên hiển thị thành các ký tự thay thế không đọc được (dấu hỏi trong ô vuông, mã Unicode U+FFFD). Không thể phục hồi nguyên văn tiếng Nhật, nhưng tên hàm/biến, cấu trúc lệnh gọi và các file `.h` liên quan đủ rõ để suy luận chính xác chức năng — toàn bộ phần giải thích dưới đây dựa trên việc đọc code, không đoán mò.

# 1. Hiểu tổng quan

### File này dùng để làm gì?

Đây là **driver điều khiển một cụm màn hình LCD + vi điều khiển panel rời**, gắn ngoài board chính và nói chuyện qua **UART** bằng một giao thức nhị phân riêng (khung lệnh bắt đầu `0xFE`/`0xFD`, có checksum, có ACK/NACK). Panel này không chỉ là màn hình — nó có sẵn:

- Một **vi điều khiển (panel microcomputer)** nhận lệnh vẽ/đèn qua UART.
- **3 LED trạng thái** (Power/nguồn, Warning/cảnh báo, Data/dữ liệu) dùng để báo tiến trình boot và lỗi phần cứng.
- Một màn **LCD** thật, được nạp ảnh thông qua một vùng đệm VRAM ảo trong RAM của CPU chính rồi "blit" (truyền khối) sang phần cứng LCD.

### Chức năng chính

1. **Khởi tạo kết nối** với panel (bật nguồn panel qua GPIO, mở UART riêng, bắt tay giao thức, đọc loại panel/độ phân giải/hãng sản xuất).
2. **Vẽ hình ảnh lên LCD**: logo boot, logo hãng (KM), ảnh "đang cập nhật firmware", "cập nhật xong", "cập nhật lỗi", ảnh troubleshooting — các ảnh được nén LZO sẵn trong firmware.
3. **Điều khiển LED chẩn đoán phần cứng (HW check mode)**: khi board chạy ở chế độ tự kiểm tra, file này quy đổi các lỗi (CPU lỗi, board lỗi, không có thiết bị boot) thành các kiểu nhấp nháy LED và có thể **treo máy vĩnh viễn** (vòng lặp watchdog) để báo lỗi cho người kiểm tra ngoài xưởng.
4. Cung cấp 1 lệnh debug U-Boot (`draw <n>`) để test vẽ ảnh bằng tay.

### Luồng chương trình chạy như thế nào? (tóm tắt, chi tiết ở mục 3)

```
init_Panel_Uart()  → bật nguồn panel, mở UART, bắt tay (init_PanelMicrocomputer)
        │                → xác định panel có tồn tại không (panel_exist)
        ▼
init_Panel()       → set độ phân giải LCD theo panel đã dò được
        │           → init_Picture() gán con trỏ ảnh
        │           → init_LCD() cấp phát VRAM, xác định OWN/Generic,
        │             vẽ ảnh boot (+logo nếu OWN), bật màn
        ▼
init_LCDBackLight()→ bật đèn nền
        │
        ▼
(trong lúc boot)   → DrawPanel_lzo(...) / DrawPanel(...) để cập nhật hình
                     set_Panel_BootDiagStatus()/set_Panel_BootDiagResult()
                     để cập nhật LED chẩn đoán
        │
        ▼
(nếu lỗi HW check) → hwcDispBootDeviceDiag() / hwcDispLoadDiag()
                     → bật LED lỗi, treo máy vĩnh viễn (nuôi watchdog)
```

### File liên quan

| File                                                     | Vai trò                                                                                                                                                           |
| -------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `mfp_panel.h`                                            | Header công khai của chính file này: struct `ST_PictureInfo`/`ST_DrawManager`, các `#define DEF_*`, các mã lỗi HW-diag, các hàm `extern`.                         |
| `km_panel.h`                                             | Khai báo dùng chung cho panel của board "km" (kiểu lệnh U-Boot, v.v.).                                                                                            |
| `picture.h`                                              | Dữ liệu ảnh đã biên dịch sẵn (`mbf_boot_trimmed`, `mbf_logo_trimmed`, `mbf_troub_trimmed`, `mbf_during_update`, …) — ảnh nén LZO/bitmap nhúng thẳng vào firmware. |
| `graphlib.h`                                             | Thư viện vẽ 2D mức thấp: `g_press` (vẽ ảnh thường), `g_press_lzo` (giải nén LZO rồi vẽ), `g_set_panel_res`.                                                       |
| `machine_setup.h`, `own_generic.h`, `mtd_flash_access.h` | Cấu hình theo dòng máy cụ thể, và các hàm đọc "soft DIP switch" từ SPI-Flash (`THRG_getSoftDipSw`) dùng để phân biệt bản OWN/Generic.                             |
| `lcd.h`, `ns16550.h`, `serial.h`                         | API driver LCD vật lý (`drv_lcd_init`, `lcd_image_blit*`, `lcd_set_resolution`…) và UART chuẩn của U-Boot.                                                        |
| `asm/arch-mvebu/gpio.h`, `mvqz_gpio.h`                   | Điều khiển chân GPIO (bật nguồn panel, đọc phím Utility).                                                                                                         |

## 2. Hiểu từng phần

### 2.1. Giải thích `#include`

| Include                                | Lý do cần                                                                                                                                                             |
| -------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `common.h`, `command.h`                | Kiểu dữ liệu chung của U-Boot và khai báo lệnh shell (`cmd_tbl_s`).                                                                                                   |
| `u-boot/sha256.h`                      | Có include nhưng không thấy dùng trực tiếp trong file này (có thể tàn dư, hoặc dùng gián tiếp qua header khác).                                                       |
| `mfp_panel.h`                          | Header của chính module này.                                                                                                                                          |
| `machine_setup.h`                      | Định nghĩa theo dòng máy (`CONFIG_KM_MACHINE_*`).                                                                                                                     |
| `picture.h`                            | Dữ liệu ảnh nhúng sẵn.                                                                                                                                                |
| `graphlib.h`                           | API vẽ (`g_press`, `g_press_lzo`).                                                                                                                                    |
| `malloc.h`                             | Cấp phát `virtVRAM` động.                                                                                                                                             |
| `ns16550.h` (có điều kiện)             | Nếu UART panel dùng chip NS16550.                                                                                                                                     |
| `serial.h`                             | Kiểu `struct serial_device` cho `panel_Uart`.                                                                                                                         |
| `lcd.h`                                | API điều khiển phần cứng LCD thật.                                                                                                                                    |
| `km_panel.h`                           | Định nghĩa dùng chung cho panel dòng "km".                                                                                                                            |
| `spi.h`, `spi_flash.h`                 | Đọc "soft DIP switch" lưu trong SPI-Flash để phân biệt OWN/Generic.                                                                                                   |
| `mvqz_gpio.h`, `asm/arch-mvebu/gpio.h` | Hàm/điều khiển GPIO của Marvell dùng để bật nguồn panel & đọc phím Utility.                                                                                           |
| `i2c.h`, `asm/io.h`                    | Còn lại từ phần "touch panel" (comment ghi rõ), không thấy dùng trực tiếp trong đoạn code hiện tại — khả năng dự phòng cho phần cứng touch chưa/không dùng ở bản này. |
| `mtd_flash_access.h`, `own_generic.h`  | API đọc soft-DIP-switch (`THRG_getSoftDipSw`) và cờ chế độ OWN/Generic dùng trong `is_OwnMode()`.                                                                     |
### 2.2. `#define`, biến global, typedef

**Định nghĩa LED / trạng thái (LED patterns)**

```c
#define PANEL_LED_OFF     0
#define PANEL_LED_ON      1
#define PANEL_LED_BREATH  2   // nhấp nháy kiểu "thở" (mờ dần)
#define PANEL_LED_FLASH   3   // nhấp nháy nhanh
```

Đây là 4 kiểu hiển thị cho mỗi LED, được nhồi vào lệnh UART gửi cho panel (mỗi LED chiếm 2 bit trong 1 byte lệnh).

**Kích thước/ định dạng panel** (`PANEL_SIZE_SHIFT/MASK`, `LVDS_FORMAT_SHIFT/MASK`): dùng để tách 1 byte `gPanelSize` nhận từ panel thành 2 trường riêng: độ phân giải và định dạng tín hiệu LVDS.

**Vị trí progress bar** (`PANEL_PROGBAR_Y_START`): tọa độ Y khác nhau tùy dòng máy (DNBMLK/SPAAS/SPABKAS/HEMLK dùng 380, còn lại dùng 472) — vì kích thước màn khác nhau giữa các dòng máy.

**`VRAM_SIZE`**: `width × height × (bpp/8)` — dung lượng bộ đệm ảo cần cấp phát, tính từ cấu hình `CONFIG_KM_PANEL_MAX_*`.

**Struct dữ liệu ảnh (không nén)** — `struct ST_PictureInfo` (định nghĩa ở `.h`):

```c
struct ST_PictureInfo {
    short ss_XStart, ss_YStart;   // toạ độ vẽ
    unsigned char uc_Bank, uc_Palet;  // bank bộ nhớ / bảng màu
    unsigned char *puc_Parts;     // con trỏ dữ liệu ảnh
};
```

Mảng `pictureInfo[]` chứa 4 ảnh: icon "Utility bật", "đang cập nhật", "cập nhật xong", "cập nhật lỗi".

**Struct quản lý animation** — `struct ST_DrawManager`: giữ danh sách chỉ số ảnh (`uc_Member[]`) và chỉ số "khung kế tiếp" (`uc_NextNo`) — dùng để phát các ảnh tuần tự như animation đơn giản (mỗi lần gọi `DrawPanel()` sẽ vẽ khung tiếp theo).

**Struct ảnh nén LZO (chỉ dùng trong file .c này)** — `ST_LZOPictureInfo`: giống `ST_PictureInfo` nhưng thêm kích thước `us_XSize/us_YSize` và độ dài dữ liệu nén `st_LZOPartsSize`, vì ảnh phải giải nén ra đúng khung hình chữ nhật đó. Mảng `lzoPictureInfo[]` chứa 3 ảnh: **boot** (progress bar), **logo KM**, **troubleshooting**, với toạ độ khác nhau theo dòng máy.

**Biến toàn cục quan trọng**

| Biến                                                                                             | Ý nghĩa                                                                                                                                         |
| ------------------------------------------------------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------- |
| `panel_exist`                                                                                    | Panel có được phát hiện/kết nối hay không (`TYPE_PanelConnect_ON/OFF`), gần như mọi hàm public đều kiểm tra biến này trước khi làm gì.          |
| `panel_Uart`                                                                                     | Con trỏ tới thiết bị UART dành riêng cho panel (`CONFIG_PANEL_DEVICE`).                                                                         |
| `virtVRAM`                                                                                       | Bộ đệm khung hình ảo (`malloc` trong RAM CPU) — nơi mọi ảnh được "vẽ" trước, sau đó mới truyền (blit) nguyên khối hoặc từng phần sang LCD thật. |
| `gPanelType`, `gPanelMaker`, <br>`gPanelTypeCode`, `gPanelOption`, <br>`gPanelSize`, `gPanelKey` | Thông tin panel đọc được lúc bắt tay UART (loại panel, hãng, mã loại, tuỳ chọn, độ phân giải, trạng thái khoá phím).                            |
| `g_PanelBootDiagErr`                                                                             | Bitmask lỗi chẩn đoán phần cứng (CPU / board / storage), tích luỹ qua `set_Panel_BootDiagErr()`.                                                |
| `hwc_normal_devices` (static)                                                                    | Bitmask các thiết bị boot đã xác nhận "bình thường" — nếu rỗng khi ở chế độ HW-check thì coi là không có thiết bị boot nào.                     |
| `uc_SelSdramArea` (static)                                                                       | Có khai báo, gán 0 khi khởi tạo, nhưng không thấy dùng ở chỗ khác trong file — khả năng dự phòng cho tính năng double-buffer chưa bật.          |
### 2.3. Danh sách hàm và chức năng

| #   | Hàm                                                 | Chức năng                                                                                                                                 |
| --- | --------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------- |
| 1   | `BspPanel_serial_rts`                               | Hook điều khiển RTS — để trống (panel không cần bắt tay phần cứng).                                                                       |
| 2   | `BspPanel_serial_putc` <br>/ `BspPanel_serial_getc` | Gửi/nhận 1 byte qua UART panel.                                                                                                           |
| 3   | `init_Picture`                                      | Gán con trỏ dữ liệu ảnh & kích thước ảnh nén vào các bảng `pictureInfo`/`lzoPictureInfo`.                                                 |
| 4   | `init_Panel`                                        | Điểm vào chính: set độ phân giải/LVDS, gọi `init_Picture`, `init_LCD`, `init_LCDBackLight`.                                               |
| 5   | `init_Panel_Uart`                                   | Bật nguồn panel qua GPIO, đọc phím Utility, mở UART panel, gọi bắt tay `init_PanelMicrocomputer`.                                         |
| 6   | `set_Panel_BootDiagStatus`                          | Đặt LED theo 6 "giai đoạn" chẩn đoán boot cũ (ST0–ST5).                                                                                   |
| 7   | `is_OwnMode` (static)                               | Đọc soft-DIP-switch trong SPI-Flash để xác định chế độ OWN hay Generic.                                                                   |
| 8   | `fill_VramWithFixVal`                               | Tô toàn bộ VRAM ảo bằng 1 màu nền cố định (RGB565), tối ưu bằng ghi 8 byte/lần.                                                           |
| 9   | `init_LCD` (static)                                 | Khởi tạo bảng animation, cấp phát VRAM, xác định OWN/Generic, khởi tạo LCD vật lý, vẽ ảnh boot ban đầu.                                   |
| 10  | `init_STdrawManager` (static)                       | Khởi tạo 2 chuỗi animation: "Update" và "UpdateFailed".                                                                                   |
| 11  | `DrawPanel`                                         | Vẽ khung ảnh hiện tại của 1 draw-manager (ảnh không nén), có overlay icon Utility nếu cần, rồi blit toàn bộ VRAM.                         |
| 12  | `DrawPanel_lzo`                                     | Giống trên nhưng cho ảnh nén LZO (boot/logo/troubleshooting); có thể blit toàn phần hoặc chỉ vùng thay đổi.                               |
| 13  | `TransStoredAreatoVRAM` (static)                    | Gọi `lcd_image_blit()` — truyền toàn bộ VRAM ảo sang LCD thật.                                                                            |
| 14  | `TransStoredAreatoVRAM_part` (static)               | Gọi `lcd_image_blit_part()` — chỉ truyền 1 vùng chữ nhật (tối ưu tốc độ).                                                                 |
| 15  | `IsTransStoredAreatoVRAMDone` (static)              | Kiểm tra việc truyền DMA đã xong chưa.                                                                                                    |
| 16  | `init_VRAMtoLCD` (static)                           | Gọi `drv_lcd_init()`, bật flush cache nếu cần.                                                                                            |
| 17  | `TransVRAMtoLCD` (static)                           | Hàm rỗng — giữ lại cho cấu trúc code, vì `drv_lcd_init` đã làm hết việc cần.                                                              |
| 18  | `setup_LcdDisplay`                                  | Hàm rỗng — chức năng chuyển màn hình của board đời cũ, không cần trên board 1-màn-hình này.                                               |
| 19  | `setup_PanelRTS` (static)                           | Hàm rỗng — panel không cần bắt tay phần cứng.                                                                                             |
| 20  | `init_PanelMicrocomputer` (static)                  | **Hàm bắt tay giao thức UART** với vi điều khiển panel: gửi chuỗi lệnh khởi tạo LED/buzzer, chờ ACK, dò panel type/maker/size/option/key. |
| 21  | `make_CheckSum` (static)                            | Tính checksum (bù 2, cộng dồn) và ghi vào cuối gói lệnh trước khi gửi.                                                                    |
| 22  | `check_Command` (static)                            | So khớp phần đầu gói nhận được với 1 lệnh mẫu (`memcmp`).                                                                                 |
| 23  | `SendData` (static)                                 | Gửi từng byte của gói lệnh qua UART.                                                                                                      |
| 24  | `RecvData` (static)                                 | Nhận 1 khung: byte đầu phải là `0xFD`, byte 2 là độ dài, đọc tiếp đúng số byte đó.                                                        |
| 25  | `do_drawpanel` (static)                             | Handler cho lệnh debug `draw <n>` của U-Boot shell, gọi `DrawPanel(n)`.                                                                   |
| 26  | `getPanelConnect`                                   | Trả về trạng thái `panel_exist`.                                                                                                          |
| 27  | `init_LCDBackLight` (static)                        | Gửi lệnh bật đèn nền, rồi đọc lại `gPanelOption`.                                                                                         |
| 28  | `hwcSetBootDeviceNormal`                            | Đánh dấu 1 thiết bị boot (eMMC/NVMe/SD) là "bình thường" — gọi từ driver thiết bị đó khi phát hiện thiết bị hoạt động tốt.                |
| 29  | `hwcDispBootDeviceDiag`                             | Nếu ở chế độ HW-check và không có thiết bị boot nào "bình thường" → ghi lỗi, bật LED lỗi storage, **treo máy vĩnh viễn**.                 |
| 30  | `set_Panel_BootDiagErr`                             | Gộp thêm 1 bit lỗi vào `g_PanelBootDiagErr`.                                                                                              |
| 31  | `set_Panel_BootDiagResult`                          | Chuyển bitmask lỗi hiện tại thành mẫu LED tương ứng rồi gửi cho panel.                                                                    |
| 32  | `hwcDispLoadDiag`                                   | Giống #29 nhưng gọi ở giai đoạn tải ảnh/kernel sau này — nếu lỗi thì bật LED storage-error và treo máy.                                   |

### 2.4. Hàm quan trọng nhất

Có 2 hàm đóng vai trò "xương sống", theo 2 nghĩa khác nhau:

- **`init_PanelMicrocomputer()`** — quan trọng nhất về mặt **hạ tầng**: đây là nơi thiết lập toàn bộ kênh giao tiếp với panel. Nếu hàm này thất bại (panel không phản hồi đúng giao thức), `panel_exist` chuyển sang `OFF` và **mọi hàm khác trong file tự động im lặng bỏ qua** (vì đều kiểm tra `getPanelConnect()`). Không có hàm này, cả module coi như không tồn tại.
- **`DrawPanel_lzo()`** — quan trọng nhất về mặt **chức năng nhìn thấy được**: đây là hàm được gọi mỗi khi cần đổi hình trên LCD (logo boot, hãng, troubleshooting), xử lý cả việc quyết định vẽ toàn màn hay chỉ vùng thay đổi (tối ưu tốc độ) và overlay icon khi cần.
### 2.5. Giải thích từng hàm theo cách dễ hiểu

| Hàm                                                                                                      | Cách hoạt động                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| -------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `init_Picture()`<br><br>Nối dữ liệu ảnh tĩnh vào bảng mô tả runtime                                      | Ảnh trong firmware được nhúng dưới dạng mảng byte tĩnh (biên dịch sẵn từ `picture.h`), nhưng bảng mô tả ảnh (`pictureInfo`, `lzoPictureInfo`) cần biết con trỏ và kích thước đó ở runtime. Hàm này chỉ đơn giản "nối dây": gán `puc_Parts`/`puc_LZOParts` và `st_LZOPartsSize` trỏ tới đúng dữ liệu ảnh tương ứng.                                                                                                                                                                                                                                                                                                                                                                   |
| `init_Panel()`<br><br>Điểm vào chính, khởi động toàn bộ hệ hiển thị                                      | Gọi 1 lần lúc board init. Thoát sớm nếu panel không tồn tại (`getPanelConnect()`). Cấu hình độ phân giải LCD dựa trên `gPanelSize` đã dò được lúc bắt tay UART, rồi lần lượt: load bảng ảnh → khởi tạo VRAM/LCD → bật đèn nền.                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| `init_Panel_Uart()`<br><br>Mở kênh UART riêng cho panel                                                  | Vì UART console chính và UART panel dùng chung phần cứng nhưng tốc độ khác nhau, hàm "mượn tạm" biến toàn cục baudrate của U-Boot (`gd->baudrate`) để khởi động UART panel đúng tốc độ (`CONFIG_PANEL_BAUDRATE`), rồi trả `gd->baudrate` lại giá trị cũ cho console. Trước đó, nó bật nguồn panel qua GPIO và đọc trạng thái phím Utility (nút vật lý vào chế độ đặc biệt).                                                                                                                                                                                                                                                                                                          |
| `set_Panel_BootDiagStatus()`<br><br>Báo tiến trình boot theo 5 mốc cố định (kiểu cũ)                     | 5 giai đoạn cố định ST1–ST5, mỗi giai đoạn ứng với 1 tổ hợp LED dựng sẵn (switch-case), rồi gửi lệnh set-LED qua UART và đọc phản hồi (không kiểm tra nội dung, chỉ "vét cạn" buffer).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| `is_OwnMode()`<br><br>Xác định thương hiệu firmware (OWN / Generic)                                      | Đọc 2 byte "soft DIP switch" ảo lưu trong SPI-Flash (không phải công tắc vật lý). Có logic ưu tiên: nếu có bit ép buộc cố định về 1 chế độ (Generic-cố-định hoặc OWN-cố-định) thì bit đó thắng, nếu không thì dùng bit chọn thông thường.                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| `fill_VramWithFixVal()`<br><br>Tô màu nền toàn VRAM (memset tối ưu)                                      | Vì RAM đọc/ghi nhanh nhất theo khối 8-byte căn chỉnh: (1) ghi từng byte lẻ tới khi con trỏ căn chỉnh 8-byte, (2) ghi hàng loạt theo khối 8-byte (màu nền lặp 4 lần thành 1 số 64-bit), (3) ghi nốt phần dư cuối. Kết quả: toàn VRAM tô màu nền `#FAFAFA` (gần trắng).                                                                                                                                                                                                                                                                                                                                                                                                                |
| `init_LCD()`<br><br>Khởi tạo toàn bộ phần "vẽ"                                                           | Chuẩn bị bảng animation, `malloc` VRAM, xác định OWN/Generic (ảnh hưởng có vẽ logo KM hay không, vị trí progress bar), khởi tạo phần cứng LCD, rồi — trừ khi đang ở chế độ tự-động-chẩn-đoán phần cứng — vẽ ảnh boot (và logo nếu OWN) trước khi bật hiển thị thật sự.                                                                                                                                                                                                                                                                                                                                                                                                               |
| `init_STdrawManager()`<br><br>Nạp cứng 2 chuỗi animation                                                 | Quản lý "Update" chạy `during-update → update-completion` rồi lặp lại từ đầu; quản lý "UpdateFailed" chỉ có 1 khung `update-failed`.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| `DrawPanel(uc_Number)`<br><br>Vẽ 1 khung ảnh không nén, tiến animation                                   | Xoá VRAM, vẽ khung hiện tại của draw-manager `uc_Number` (dùng `g_press`), chồng icon "Utility On" nếu phím Utility bật, blit toàn VRAM sang LCD, **bận chờ** tới khi DMA xong. Cuối cùng chuyển sang khung kế tiếp (quay vòng khi hết danh sách) — cơ chế animation từng-bước, mỗi lần gọi tiến 1 khung.                                                                                                                                                                                                                                                                                                                                                                            |
| `DrawPanel_lzo(picNo,`<br>`refreshMode)`<br><br>Vẽ ảnh nén LZO (boot/logo/troubleshooting)               | • Utility key bật + đang vẽ boot/troubleshooting → ép refresh "toàn màn" (tránh lộ vùng cũ).  <br>• `refreshMode != NO_VRAM_REFRESH` → tô nền trước bằng `fill_VramWithFixVal`.  <br>• Đang vẽ ảnh boot + chế độ OWN → dịch progress bar xuống `PANEL_PROGBAR_Y_START` riêng theo dòng máy.  <br>• Gọi `g_press_lzo` giải nén trực tiếp vào đúng vị trí trong VRAM.  <br>• Utility key bật khi vẽ boot/troubleshooting → vẽ thêm icon Utility, ép "toàn màn".  <br>• Blit toàn VRAM hoặc chỉ vùng ảnh vừa vẽ (tuỳ chế độ), chờ DMA xong, flush cache.                                                                                                                                |
| `TransStoredAreatoVRAM()`<br>/ `_part()`<br><br>Lớp bọc gọi driver LCD thật                              | Chỉ gọi thẳng `lcd_image_blit()` / `lcd_image_blit_part()` — tách riêng thành hàm `static` để dễ đọc code gọi và đổi driver sau này nếu cần.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| `init_VRAMtoLCD()`<br><br>Khởi tạo controller LCD vật lý                                                 | Gọi `drv_lcd_init()`; nếu build bật D-cache trên ARM thì bật cờ tự flush cache khi vẽ và xoá màn hình LCD.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| `init_PanelMicrocomputer()`<br><br>Bắt tay giao thức + dò thông tin panel (hàm dài & phức tạp nhất file) | **(1) Bắt tay khởi tạo**: gửi 5 lệnh cấu hình cứng (độ sáng LED, kiểu nhấp nháy LED khởi động, kiểu nhấp nháy LED mở rộng, cấu hình buzzer, số lần kêu) theo mô hình hỏi-đáp: chờ "InitCMD/SuspendCMD" từ panel → trả lời `txInitCMD` → panel trả `InitRES` → gửi từng lệnh trong `txcmd[]`, mỗi lệnh chờ ACK mới gửi tiếp; không nhận được gì hợp lệ (`uart_error`) → kết luận **panel không được gắn**.  <br>**(2) Dò thông tin**: hỏi loại panel (map byte trả về thành `gPanelType` 0–31 qua `switch`), hãng sản xuất, độ phân giải, mã loại chi tiết, tuỳ chọn option (gồm tắt buzzer nếu panel không có loa nhưng có đèn), trạng thái khoá phím (nếu Utility không được nhấn). |
| `make_CheckSum()`<br><br>Tính checksum gói lệnh                                                          | Kiểu "bù 2 của tổng": cộng dồn byte dữ liệu (bỏ header/footer), ghi `(~sum + 1) & 0x7F` vào đúng vị trí cuối gói (theo byte độ dài ở `buffer[1]`).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| `check_Command()` <br>/ `SendData()` <br>/ `RecvData()`<br><br>Nền tảng giao thức UART                   | So khớp lệnh nhận được (`memcmp`), gửi từng byte, nhận đúng khung (`0xFD` + độ dài + dữ liệu). `RecvData` trả số byte thực nhận, hoặc `-1` nếu byte đầu không phải `0xFD` (framing lỗi).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| `do_drawpanel()`<br><br>Lệnh debug U-Boot `draw <n>`                                                     | Wire vào U-Boot qua `U_BOOT_CMD`, cho phép gõ `draw <n>` ở console để test thủ công vẽ 1 draw-manager mà không cần trigger từ luồng boot thật.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| `getPanelConnect()`<br><br>Getter trạng thái kết nối panel                                               | Getter đơn giản — "cổng gác" mà gần như mọi hàm public khác dùng để tự tắt nếu panel không có mặt.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| `init_LCDBackLight()`<br><br>Bật đèn nền LCD                                                             | Gửi lệnh bật đèn nền (độ sáng cứng `0x7F`), sau đó **đọc lại option panel lần nữa** — vì phát hiện đèn nền (BLE) có độ trễ, phải hỏi lại sau khi bật đèn để lấy `gPanelOption` chính xác.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| `hwcSetBootDeviceNormal()`<br><br>Hook báo "thiết bị boot OK"                                            | Các driver thiết bị lưu trữ (eMMC, NVMe, SD…) tự OR bit tương ứng vào `hwc_normal_devices` khi phát hiện thiết bị hoạt động tốt.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| `hwcDispBootDeviceDiag()`<br><br>Chẩn đoán không tìm thấy thiết bị boot                                  | Cuối giai đoạn dò thiết bị boot lúc HW-check: nếu chưa thiết bị nào gọi `hwcSetBootDeviceNormal` → coi như hỏng lưu trữ → ghi mã chẩn đoán, bật LED lỗi, **`_WAIT_FOREVER()`** (vòng lặp vô hạn `udelay` + `initr_wd_refresh()` nuôi watchdog nhưng không boot tiếp) — có chủ đích: dừng hẳn để người kiểm tra ngoài xưởng thấy đèn báo lỗi.                                                                                                                                                                                                                                                                                                                                         |
| `set_Panel_BootDiagErr()`<br>/ `set_Panel_BootDiagResult()`<br><br>Ghi lỗi & hiển thị lỗi qua LED        | Lỗi tích luỹ dạng bitmask (nhiều lỗi cùng lúc). `set_Panel_BootDiagResult()` build lại mẫu LED **từ đầu mỗi lần gọi** dựa trên toàn bộ bitmask hiện có (không chỉ lỗi mới nhất), rồi gửi 1 lệnh LED tổng hợp.                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| `hwcDispLoadDiag()`<br><br>Chẩn đoán lỗi ở bước tải ảnh                                                  | Tương tự `hwcDispBootDeviceDiag` nhưng gọi ở bước tải kernel/image — nếu thất bại trong chế độ HW-check thì bật LED lỗi storage và treo máy vĩnh viễn.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
## 3. Sơ đồ luồng chương trình


![[Pasted image 20260814035138.png]]
![[Pasted image 20260814035200.png]]

## 4. Ví dụ cụ thể: input → xử lý → output

### Ví dụ A — Vẽ logo boot lên màn hình

**Input**: lời gọi `DrawPanel_lzo(DEF_LZO_BOOT_PIC, NO_VRAM_REFRESH);` (đây chính là lệnh được gọi trong `init_LCD()`).

**Xử lý**:

1. Kiểm tra `getPanelConnect()` — nếu panel không tồn tại thì thoát ngay, không làm gì cả.
2. Lấy bản ghi `lzoPictureInfo[DEF_LZO_BOOT_PIC]` → biết toạ độ (X, Y), kích thước (200×117 hoặc 190×110 tuỳ dòng máy), con trỏ dữ liệu nén và độ dài dữ liệu.
3. Vì phím Utility không bật (giả sử) và chế độ refresh là `NO_VRAM_REFRESH` → **không** tô nền lại, giữ nguyên VRAM hiện có.
4. Vì đang ở chế độ OWN (giả sử `g_OWN_Mode == OWN_MODE_ON`) → toạ độ Y của ảnh boot được dịch xuống `PANEL_PROGBAR_Y_START` (472 hoặc 380 tuỳ dòng máy) để nhường chỗ cho logo KM phía trên.
5. Gọi `g_press_lzo(x, y, xsize, ysize, bank, palet, du_lieu_nen, kich_thuoc_nen)` → hàm này (trong `graphlib`) giải nén LZO và ghi trực tiếp các pixel vào đúng vùng chữ nhật đó trong `virtVRAM`.
6. Vì `uc_RefreshMode` vẫn là `NO_VRAM_REFRESH` (không bị ép đổi) → chỉ truyền **vùng vừa vẽ**, gọi `TransStoredAreatoVRAM_part(virtVRAM, x, y, xsize, ysize)` thay vì truyền cả VRAM.
7. Bận chờ `IsTransStoredAreatoVRAMDone()` tới khi phần cứng báo DMA blit xong.
8. `lcd_sync()` flush cache (nếu D-cache đang bật) để đảm bảo dữ liệu CPU vừa ghi đã thực sự tới bộ nhớ mà LCD controller đọc.

**Output**: hình logo boot xuất hiện đúng vị trí trên màn hình LCD vật lý gắn ngoài — người đứng trước máy nhìn thấy ngay logo trong lúc U-Boot đang khởi động, không cần chờ hệ điều hành.

### Ví dụ B — Dò độ phân giải panel qua giao thức UART (trong `init_PanelMicrocomputer`)

**Input**: mảng lệnh tĩnh `txSizeCMD = {0xFE, 0x02, 0x2A, 0xFF}` (chưa có checksum).

**Xử lý**:

1. `make_CheckSum(txSizeCMD)`: đọc `buffer[1] = 0x02` (độ dài dữ liệu), cộng dồn byte dữ liệu (`0x2A`), tính `checksum = (~sum + 1) & 0x7F`, ghi đè byte `0xFF` cuối cùng bằng giá trị checksum thật. Gói lệnh giờ đã hoàn chỉnh.
2. `SendData(txSizeCMD, sizeof(txSizeCMD))`: gửi từng byte một qua `BspPanel_serial_putc` tới vi điều khiển panel.
3. Panel xử lý lệnh "hỏi độ phân giải" nội bộ, trả lời 1 khung bắt đầu bằng `0xFD`.
4. `RecvData(buff, sizeof(buff))`: đọc byte đầu (`0xFD` — nếu không đúng, trả về lỗi `-1`), đọc byte 2 là độ dài, rồi đọc đúng số byte dữ liệu đó vào `buff`.
5. Code lấy `gPanelSize = buff[3];` — byte dữ liệu đầu tiên trong phần thân gói chính là mã độ phân giải + định dạng LVDS đã đóng gói.

**Output**: biến toàn cục `gPanelSize` được thiết lập. Giá trị này sau đó được `init_Panel()` tách ra bằng `PANEL_SIZE_MASK`/`LVDS_FORMAT_MASK` để gọi `lcd_set_resolution()` và `lcd_set_lvds_format()`, tức là **toàn bộ độ phân giải hiển thị thực tế của LCD được cấu hình tự động dựa trên loại panel vật lý đang gắn vào board** — không cần hard-code cho từng loại màn hình.


# 5. Tại sao enable DP rồi nhưng không in được log.

Đã tìm ra nguyên nhân chính xác — **không nằm ở macro `DP`**, mà nằm ở một lớp cổng in log riêng của KM_BIZHUB nằm bên dưới `printf()`. Đây là chuỗi nhân quả đầy đủ:

#### `DP()` không phải là vấn đề

Khi bạn sửa `#if 0` → `#if 1`, `DP(fmt, args...)` đúng là đã expand thành `printf(fmt, ##args)`. Macro hoạt động đúng như mong đợi.

#### Nhưng `printf()` → `puts()` trên bản KM_BIZHUB này có 1 cổng chặn riêng

Trong [console.c:589-606](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/console.c#L589-L606), hàm `puts()` (mà `printf()` gọi tới) có đoạn:

```c
void puts(const char *s)
{
#if defined(KM_BIZHUB)
	if (BspPrintStopCheck()) return;          // chặn gọi đệ quy
	if (BspPrintValid()) {
		BspPrintBufferFlush();                // được phép in → xả buffer rồi in tiếp
	} else {
		BspPrintBuffering(s);                 // CHƯA được phép in → cất vào buffer, return luôn
		return;
	}
#endif
	...
```

`BspPrintValid()` ([console.c:312-328](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/console.c#L312-L328)) gọi `getTermState()`; nếu trả về `TERM_STATE_0` thì coi như **"chưa được phép in"**, và chuỗi bị nhét vào `gd->arch.printBuffer[]` (tối đa 255 dòng, [mvebu-common.h:261](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/include/configs/mvebu-common.h#L261)) thay vì được đẩy ra UART thật.

#### `getTermState()` chắc chắn trả `TERM_STATE_0` tại thời điểm DP() chạy

[dipvalue.c:48-55](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/board/mvebu/armada8k/km/dipvalue.c#L48-L55):

```c
unsigned char getTermState(void)
{
	static unsigned char term_state = 0xff;
	if( 0 == gd->arch.termFlg ){
		return TERM_STATE_0;   // <-- luôn về đây nếu termFlg chưa bật
	}
	...
```

`gd->arch.termFlg` được reset về `0` sớm trong `board_init_f` ([board_f.c:1037](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/board_f.c#L1037)), và **chỉ được bật lên 1** bên trong `stdio_add_devices()` ([stdio.c:249](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/stdio.c#L249)).

#### Vấn đề là thứ tự gọi trong `init_sequence_r[]`

Trong [board_r.c:915-1047](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/board_r.c#L915-L1047):

|Thứ tự|Hàm|Việc gì xảy ra|
|---|---|---|
|dòng 1017|`init_km`|gọi `init_Panel_Uart()` → `init_PanelMicrocomputer()` → `SendData/RecvData` → **`DP()` chạy ở đây**|
|dòng 1042|`stdio_add_devices`|**`gd->arch.termFlg = 1;` mới được set ở đây**|

→ Tại thời điểm mọi lệnh `DP("TX: ...")` / `DP("RX: ...")` trong quá trình bắt tay UART với panel chạy, `termFlg` **vẫn còn là 0** → `getTermState()` trả `TERM_STATE_0` → `BspPrintValid()` = 0 → mọi dòng log của bạn bị **âm thầm buffer lại**, không hề ra UART, dù `DP` đã đúng thành `printf`.

Đây cũng chính là lý do vì sao chính file `mfp_panel.c`/`mfp_panel.h` không dùng `printf` cho các message quan trọng, mà dùng macro `_CONSOLE_OUT` (gọi thẳng `serial_puts()`, bỏ qua hoàn toàn lớp `puts()`/`BspPrintValid()` này) — họ đã biết trước cái bẫy này.


####  Cách khắc phục (chọn 1 trong các cách)

- **Nhanh nhất để debug**: đổi `DP` thành gọi thẳng `serial_puts()` (giống `_CONSOLE_OUT`), bỏ qua toàn bộ cổng buffer:
    
    ```
    #define DP( fmt, args... ) do { char _dp[128]; snprintf(_dp, sizeof(_dp), fmt, ##args); serial_puts(_dp); } while(0)
    ```
    
- **Đúng bản chất thiết kế board**: log chỉ thực sự hiện ra khi DIP-switch phần cứng chọn 1 chế độ terminal hợp lệ (không phải `TERM_STATE_0` = "AP806 UART0 = None"), hoặc GPIO `GPIO_PORT_LOG_SW`/`GPIO_PORT_FW_CONFIG5` ép force-log-output ([dipvalue.c:24-35](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/board/mvebu/armada8k/km/dipvalue.c#L24-L35)). Nếu board bạn đang cắm DIP-switch ở vị trí "None", **theo thiết kế nó cố tình không in gì** cho tới khi qua `stdio_add_devices()` (và cả lúc đó vẫn cần DIP-switch hợp lệ).
- Khi đã lên được console (sau `console_init_r`), gõ lệnh `db_sw` để xem trạng thái terminal hiện tại ([dipvalue.c:146-150](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/board/mvebu/armada8k/km/dipvalue.c#L146-L150)) — nó in ra `Force Log Output: ON/OFF` và `Terminal State: TERM0..4`.
- Buffer chỉ giữ tối đa 255 dòng (`PRINT_BUFFER_NUM`) — nếu vượt quá trước khi được flush thì các dòng dư cũng mất luôn.

# 6. Tại sao dòng log dưới vẫn in ra được

```c
if ( 0 != ( uc_Offset14 & 0x02) ) {
	Ret= True;	// OWN���[�h
	printf( "is_OwnMode:%d\n", __LINE__);
}
```

`is_OwnMode()` chỉ được gọi từ `init_LCD()` ([mfp_panel.c:346](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/board/mvebu/armada8k/km/mfp_panel.c#L346)), và `init_LCD()` chỉ được gọi từ `init_Panel()` ([mfp_panel.c:160-172](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/board/mvebu/armada8k/km/mfp_panel.c#L160)) — mà `init_Panel()` lại được gọi từ [machine_setup.c:696](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/board/mvebu/armada8k/km/machine_setup.c#L696), bên trong `misc_init_r()`.

## So sánh vị trí trong `init_sequence_r[]` ([board_r.c:915-1055](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/board_r.c#L915-L1055))

| Thứ tự    | Hàm                                                                                                                        | `termFlg` lúc đó                                       |
| --------- | -------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------ |
| dòng 1017 | `init_km` → `init_Panel_Uart` → `init_PanelMicrocomputer` → **`DP()` chạy ở đây**                                          | **`0`** (chưa set) → bị `BspPrintBuffering()` nuốt mất |
| dòng 1042 | `stdio_add_devices` → **`gd->arch.termFlg = 1;`**                                                                          | flag được bật ở đây                                    |
| dòng 1047 | `console_init_r`                                                                                                           | console đã sống                                        |
| dòng 1055 | `misc_init_r` → `init_Panel()` → `init_LCD()` → `is_OwnMode()` → **dòng `printf("is_OwnMode:%d\n", __LINE__)` chạy ở đây** | **`1`** rồi (đã set từ dòng 1042, 13 bước trước đó)    |

## Kết quả

Khi `is_OwnMode()` chạy, `gd->arch.termFlg == 1` và console đã được `console_init_r()` khởi tạo đầy đủ, nên `getTermState()` không còn tự động trả `TERM_STATE_0` nữa (giả sử DIP-switch của board đang ở trạng thái terminal hợp lệ) → `BspPrintValid()` trả `1` → `puts()` đi thẳng ra UART thay vì bị `BspPrintBuffering()` giữ lại → dòng log in ra **ngay lập tức**, đúng như bạn thấy.

Đây chính là bằng chứng gián tiếp xác nhận lại chẩn đoán trước: vấn đề chưa từng nằm ở macro `DP`, mà ở **thời điểm gọi hàm chứa nó** so với lúc `stdio_add_devices()` bật `termFlg`. Nếu bạn di chuyển lệnh gọi `init_Panel_Uart()`/`init_PanelMicrocomputer()` xuống sau `stdio_add_devices` (hoặc đổi `DP()` sang gọi thẳng `serial_puts()` như tôi đề xuất ở câu trước), các dòng `DP("TX:...")`/`DP("RX:...")` sẽ in ra ngay lập tức giống như dòng `is_OwnMode` này.


# 7. Cách kiểm tra xem panel có lỗi không
Nguyên nhân "màn hình không lên" gần như chắc chắn nằm ở **bắt tay UART với vi điều khiển panel bị timeout**, và toàn bộ pipeline vẽ sau đó **tự tắt một cách im lặng** — không crash, không báo lỗi rõ ràng, chỉ đơn giản là không vẽ gì cả. Đây là chuỗi lỗi cụ thể trong code:

### Cơ chế lỗi (tại sao im lặng)

1. Khi đọc byte từ UART panel, [ns16550.c:255-278](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/drivers/serial/ns16550.c#L255-L278) có timeout riêng cho cổng panel: nếu không nhận được byte nào trong **500ms** (`5000 × 100us`), nó set `uart_error = 1` và in `"uart timeout error !!!\n"`.
2. Vòng lặp bắt tay trong `init_PanelMicrocomputer()` kiểm tra cờ này ngay ([mfp_panel.c:744-750](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/board/mvebu/armada8k/km/mfp_panel.c#L744-L750)):
    
    ```c
    if( uart_error ){
        printf("Panel is not attached probably !!!\n");
        panel_exist = TYPE_PanelConnect_OFF;
        return;
    }
    ```
    
3. Từ đây, **mọi hàm public trong `mfp_panel.c` đều tự no-op** vì đều mở đầu bằng `if (getPanelConnect() != TYPE_PanelConnect_ON) return;` — `init_Panel()`, `DrawPanel()`, `DrawPanel_lzo()` đều thoát ngay lập tức, không vẽ gì, không báo lỗi thêm. **Đây rất có thể là đúng bug bạn đang gặp**: phần cứng/kết nối panel có vấn đề → handshake fail → cả hệ thống hiển thị tắt câm lặng.

Lưu ý thêm: cả 2 dòng `printf` báo lỗi ở bước 1 và 2 đều chạy **trong `init_km()`, trước khi `termFlg` được bật** (như đã phân tích ở câu trước) → **bị buffer, không hiện ra ngay lúc lỗi xảy ra**, chỉ xuất hiện dồn cục sau này khi được flush. Nên nếu bạn nhìn log theo thời gian thực mà không thấy 2 dòng này, đừng vội kết luận là không lỗi — hãy tìm chúng ở đoạn log xuất hiện dồn dập ngay sau khi console lên.

### Checklist kiểm tra theo thứ tự

|Bước|Kiểm tra gì|Cách xem|
|---|---|---|
|1|Handshake UART có OK không|Tìm dòng `"Panel is not attached probably !!!"` hoặc `"uart timeout error !!!"` trong log (nhớ: có thể bị delay do buffer)|
|2|`panel_exist` cuối cùng là gì|Thêm tạm 1 dòng debug **không bị buffer** (xem gợi ý patch bên dưới) ngay sau `init_PanelMicrocomputer()` trong `init_Panel_Uart()`|
|3|`gPanelType`, `gPanelMaker`, `gPanelSize`, `gPanelOption`, `gPanelKey` có giá trị hợp lý không|Nếu handshake fail, các biến này giữ giá trị mặc định (0) — dễ nhầm là panel loại "TPI" (case 0x31→0) dù thực ra chưa từng đọc được gì|
|4|`virtVRAM` có `malloc` thành công không|Xem log `"*WARNING* malloc error virtVRAM"` — dòng này chạy trong `init_LCD()` gọi từ `misc_init_r`, **không bị buffer**, sẽ hiện real-time|
|5|Pipeline vẽ có chạy tới nơi không (tách lỗi UART panel ra khỏi lỗi LCD vật lý)|Nếu lên được U-Boot shell, gõ `draw 0` — nếu vẫn không có gì hiện, và bước 2 xác nhận `panel_exist == ON`, thì lỗi nằm ở tầng LCD vật lý (`drv_lcd_init`, backlight, cáp LVDS) chứ không phải UART panel|
|6|Phần cứng|GPIO bật nguồn panel (`GPIO_PORT_POWER_OFF_PNL`, [mfp_panel.c:198](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/board/mvebu/armada8k/km/mfp_panel.c#L198)) có thực sự lên mức đúng không; dây TX/RX của UART panel có bị đấu ngược không; baudrate `CONFIG_PANEL_BAUDRATE` có khớp với firmware panel không; `CONFIG_PANEL_PORT` trong ns16550.c có đúng là cổng UART vật lý nối tới panel không|

### Patch tạm để thấy kết quả ngay lập tức (không bị buffer)

Thêm dòng này ngay sau lời gọi `init_PanelMicrocomputer();` trong `init_Panel_Uart()` ([mfp_panel.c:191](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/board/mvebu/armada8k/km/mfp_panel.c#L191) khu vực cuối hàm):

```c
{
    char _dbg[64];
    snprintf(_dbg, sizeof(_dbg), "panel_exist=%d uart_error=%d\n", panel_exist, uart_error);
    serial_puts(_dbg);   // giống cơ chế _CONSOLE_OUT, bỏ qua buffer
}
```

Dòng này in ngay lập tức bất kể `termFlg`, cho bạn biết chính xác handshake pass hay fail **ngay tại thời điểm nó xảy ra**, không phải đoán qua log bị dồn cục sau này.


# 8. Lúc nào serial debug log sẵn sàng
Có **2 lớp điều kiện độc lập**, cả hai đều phải thoả thì log qua `printf()`/`puts()` mới thực sự chảy ra cổng serial. Đây là bức tranh đầy đủ sau khi soi lại toàn bộ [console.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/console.c):

## Lớp 1 — Lớp console chuẩn của U-Boot (gần như luôn sẵn sàng rất sớm)

|Mốc|Nơi xảy ra|Ý nghĩa|
|---|---|---|
|`board_init_f` (trước relocation)|`console_init_f()` ([console.c:831-843](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/console.c#L831-L843))|set `gd->have_console = 1` — cực sớm|
|`board_r.c:950`|`initr_serial()` → `serial_initialize()` ([board_r.c:200-204](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/board_r.c#L200-L204))|khởi tạo lại driver UART sau relocation|
|`board_r.c:1047`|`console_init_r()`|set `GD_FLG_DEVINIT` ([console.c:948](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/console.c#L948)) — từ đây output đi qua tầng stdio đầy đủ (`fputs(stdout,...)`) thay vì gọi thẳng `serial_puts()`|

Lớp này **gần như không phải là điểm nghẽn thực tế** trên board KM_BIZHUB — vì nó bị lớp 2 chặn từ trước khi chạm tới nó.

## Lớp 2 — Cổng riêng của KM_BIZHUB (đây mới là cái quyết định)

Nhìn lại `puts()` ([console.c:589-606](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/console.c#L589-L606)): khối `#if defined(KM_BIZHUB)` được kiểm tra **đầu tiên**, trước cả khi chạm tới `gd->have_console`/`GD_FLG_DEVINIT` ở lớp 1. Nên dù lớp 1 đã sẵn sàng từ rất sớm, log vẫn bị chặn nếu lớp 2 chưa đủ điều kiện. Cần **đồng thời cả 2 điều kiện** sau:

**(a) `gd->arch.termFlg == 1`**

- Reset về 0 ở [board_f.c:1037](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/board_f.c#L1037)
- Chỉ được set = 1 trong `stdio_add_devices()` ([stdio.c:249](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/stdio.c#L249)), gọi tại [board_r.c:1042](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/board_r.c#L1042)

**(b) `getTermState() != TERM_STATE_0`** ([dipvalue.c:48-112](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/board/mvebu/armada8k/km/dipvalue.c#L48-L112))

- Đọc "soft DIP-switch" trong SPI-Flash (offset 0x51/0x21) để chọn TERM1–TERM4, hoặc
- Bị GPIO ép buộc qua `isForceLogOutput()` ([dipvalue.c:24-35](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/board/mvebu/armada8k/km/dipvalue.c#L24-L35)): `GPIO_PORT_FW_CONFIG5` mức thấp, hoặc `GPIO_PORT_LOG_SW` mức cao
- Nếu DIP-switch board bạn đang set ở vị trí "None" **và** GPIO force-log không active → hàm này **luôn** trả `TERM_STATE_0`, bất kể `termFlg` đã lên 1 hay chưa → log **vĩnh viễn không bao giờ chảy ra**, chỉ tích luỹ vào buffer tới khi tràn (255 dòng, `PRINT_BUFFER_NUM`) rồi bị bỏ.

## Kết luận thực tế

```
board_init_f ────────────────────────► board_r.c:1017 (init_km, DP() chạy ở đây)
                                              │  termFlg vẫn = 0 → LUÔN bị buffer
                                              ▼
                              board_r.c:1042 (stdio_add_devices, termFlg=1)
                                              │
                                              ▼
                         từ đây: log chảy ra được  NẾU  DIP-switch/GPIO chọn 1 mode
                         terminal hợp lệ (TERM1-4) hoặc force-log-output đang bật
```

Nên câu trả lời ngắn gọn: **log qua `printf`/`puts` chỉ dùng được sớm nhất là sau `stdio_add_devices()` (~board_r.c dòng 1042), và chỉ khi DIP-switch phần cứng của board không ở vị trí "None"** (hoặc GPIO force-log đang bật). Muốn kiểm tra board bạn hiện đang ở trạng thái nào, lên được console rồi gõ lệnh `db_sw` ([dipvalue.c:146-150](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/board/mvebu/armada8k/km/dipvalue.c#L146-L150)).

Nếu cần in log **sớm hơn mốc đó** hoặc **bất kể DIP-switch** (ví dụ để debug đúng đoạn `init_Panel_Uart`/handshake panel như bug trước), cách duy nhất chắc chắn là gọi thẳng `serial_puts()`/`serial_putc()` (như `_CONSOLE_OUT` macro đang làm) để bỏ qua hoàn toàn cả 2 lớp cổng này.


# 9. Khi nào phần cứng serial được khởi tạo

Phần cứng serial console (không phải panel UART) **đã được khởi tạo từ rất sớm rồi** — sớm hơn nhiều so với thời điểm `init_PanelMicrocomputer()` chạy. Cụ thể:

#### Serial console được khởi tạo phần cứng 2 lần, đều trước `init_km`

**Lần 1 — ngay trong `board_init_f` (giai đoạn đầu tiên, trước relocation)**, [board_f.c:883-885](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/board_f.c#L883-L885):

```c
init_baud_rate,   /* thiết lập baudrate */
serial_init,      /* khởi tạo phần cứng UART (thanh ghi, chia baud, enable) */
console_init_f,   /* stage 1 init console, set gd->have_console = 1 */
```

`board_init_f` chạy **trước tất cả mọi thứ** trong `board_init_r` — tức trước `init_km` (dòng 1017) rất xa.

**Lần 2 — sau relocation, đầu `board_init_r`**, `initr_serial()` ([board_r.c:200-204](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/board_r.c#L200-L204)):

```c
static int initr_serial(void)
{
	serial_initialize();
	return 0;
}
```

được gọi tại [board_r.c:950](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/common/board_r.c#L950) — sở dĩ cần init lại lần 2 là vì sau relocation, code/data đã copy sang vị trí khác trong RAM, nên driver cần re-init con trỏ/thanh ghi. Nhưng mốc này **vẫn ở dòng 950, trước `init_km` ở dòng 1017 tới 67 bước**.

#### Vậy tại `init_PanelMicrocomputer()` (chạy từ `init_km`, dòng 1017), tình trạng là:

|Thành phần|Trạng thái|
|---|---|
|Phần cứng UART console (thanh ghi, baudrate)|✅ Đã sẵn sàng từ lâu (2 lần init, cả 2 đều trước dòng 1017)|
|`gd->have_console`|✅ Đã = 1 từ `console_init_f` (trong `board_init_f`, cực sớm)|
|`gd->arch.termFlg` (cổng riêng KM_BIZHUB)|❌ Vẫn = 0, chỉ lên 1 ở `stdio_add_devices()` (dòng 1042, **sau** 1017)|

Nên **cái duy nhất chưa sẵn sàng là cờ phần mềm `termFlg` của KM_BIZHUB**, chứ không phải phần cứng. `serial_puts()` gọi thẳng xuống driver NS16550 vốn đã init xong từ 2 bước trước đó, nên hoàn toàn gửi được byte ra UART thật — nó chỉ là bỏ qua cái cổng chặn phần mềm (`BspPrintValid()`) chứ không đụng gì tới phần cứng cả.