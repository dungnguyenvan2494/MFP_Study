# 1. Hàm `setup_MotionEnable_Power()` -> `check_action()`

Stage 1 thực chất gồm 2 hàm lồng nhau: **`setup_MotionEnable_Power()`** (machine_setup.c) là hàm được gọi từ `misc_init_r()`, nó tự quyết định `g_action` trong nhánh không-USB, và chỉ gọi `check_action()` (main.c) khi xác nhận có USB. Đi theo đúng thứ tự thực thi.

## A. `setup_MotionEnable_Power()` — machine_setup.c:149
## A.0 Giải thích sơ
### Phân tích tổng quan

**Function này dùng để làm gì?** `setup_MotionEnable_Power()` là bước "định tuyến boot + đồng bộ phần cứng phụ trợ" chạy rất sớm trong `misc_init_r()` (trước khi `main_loop()` tồn tại). Nó làm 2 việc gộp lại: (1) xác định **loại kịch bản boot** sẽ chạy (ghi vào biến toàn cục `g_action`) bằng cách quét USB nếu cần hoặc đọc cấu hình DipSW nếu không, và (2) dựa trên kết quả đó để cấu hình cơ cấu chuyển động cơ khí (Motion Enable) và watchdog timer cho phù hợp với kịch bản boot vừa chọn.

**Input** — hàm không có tham số (`void`), input thực chất là **trạng thái toàn cục/ngoại vi** tại thời điểm gọi:

- `uc_TempUtilityKey` — trạng thái phím utility vật lý (đã được set trước đó bởi phần init khác)
- Cờ "line-card boot lần trước" đọc từ 1 sector trên SSD/SD
- Nội dung SPI Flash: DipSW ảo offset `0x06`, cờ MSC-download-dở offset `0x07B000`
- Trạng thái phần cứng USB (khi được quét)
- `gPanelKey` — trạng thái phím panel

**Output** — hàm là `void`, không có giá trị trả về; "output" thật sự là các **side effect**:

- Biến toàn cục `g_action` được set — đây là kết quả quan trọng nhất, chi phối toàn bộ boot flow phía sau.

**Side effect quan trọng:**

- Ghi lại cờ "line-card boot" xuống SSD/SD cho lần boot kế tiếp.
- Set/clear 2 chân GPIO (Motion Enable — cơ cấu chính + cảm biến IR).
- Refresh/tắt watchdog phần cứng CA72 nhiều lần với timeout khác nhau (side effect có thể gây **reset máy thật** nếu tính sai thời gian).
- Có thể chặn thực thi tới ~20 giây (2 lần retry USB × 10s) hoặc 4.5 giây (`mdelay` chờ relay cơ khí ổn định).
- Gọi `check_action()` — hàm này tự nó cũng đọc/ghi thêm dữ liệu và có thể set `g_action`.


### Flowchart mức HIGH-LEVEL

```c
[START]
   │
   ▼
[Nhận input] Đọc trạng thái phím utility + đọc cờ "line-card boot lần trước" (từ SSD/SD)
   │
   ▼
❓ Có cần kiểm tra thiết bị USB không?
   (phím utility đang active  HOẶC  cờ line-card lần trước = có)
   │
   ├── YES ──▶ [Dò & xác thực USB — có retry tối đa 2 lần, mỗi lần chờ 10s nếu thất bại]
   │              │
   │              ▼
   │           Gọi function check_action() để xác định loại boot dựa trên
   │           nội dung đọc được từ USB (file INDEX / file .tar đặc biệt)
   │              │
   ├── NO ───▶ [Tự suy loại boot từ cấu hình nội bộ — đọc DipSW + cờ MSC-download
   │            trong SPI Flash, không cần USB]
   │              │
   ▼◀─────────────┘
[g_action đã được xác định]  ← đây là "output" chính của function
   │
   ▼
[Cập nhật lại cờ "line-card boot" cho lần boot KẾ TIẾP]
   (nếu boot này là line-card mà cờ cũ chưa bật → bật; nếu không còn là line-card
    mà cờ cũ đang bật → tắt; ghi xuống SSD/SD)
   │
   ▼
[Tính toán: loại boot này có nên bật cơ cấu chuyển động cơ khí (Motion Enable) không?]
   (dựa trên: loại boot vừa xác định + ai đang thao tác phím utility + trạng thái phím panel)
   │
   ▼
❓ Việc set GPIO này có phải trách nhiệm của function hay đã có mạch khác lo rồi?
   (boot bình thường + phím utility không active → mạch nguồn phụ đã set sẵn)
   │
   ├── YES (tự set) ──▶ [Ghi GPIO Motion Enable ra phần cứng theo kết quả vừa tính]
   ├── NO (đã có nơi khác lo) ──▶ [Bỏ qua, không ghi đè]
   │
   ▼◀───────────────────┘
❓ Loại boot có phải tác vụ nhạy cảm dữ liệu không?
   (IISW / update firmware / backup / restore)
   │
   ├── YES ──▶ [Chờ ổn định relay cơ khí (~4.5s) + refresh watchdog ngắn hạn]
   ├── NO  ──▶ (bỏ qua bước này)
   │
   ▼◀───┘
❓ Loại boot có phải loại KHÔNG tự boot kernel không?
   (lỗi / chẩn đoán phần cứng / backup / restore / speed-change / line-card)
   │
   ├── YES ──▶ [Tắt hẳn watchdog — các mode này dừng lại chờ tương tác người, không cần đếm ngược]
   ├── NO  ──▶ [Giữ nguyên watchdog đang chạy — main_loop() phía sau sẽ tiếp tục refresh]
   │
   ▼◀───┘
[Return output] — không có giá trị trả về; "kết quả" là g_action + trạng thái GPIO/WDT đã set
   │
   ▼
[END] — quay lại misc_init_r() để tiếp tục các bước init board khác

```

**Error/exception path**: đây là firmware C nhúng, không có cơ chế exception. "Đường lỗi" ở đây là các **fallback phòng thủ** rải khắp hàm: mọi lần đọc SPI Flash/SSD thất bại đều tự động dùng giá trị mặc định an toàn (0) thay vì dừng chương trình; nếu quét USB thất bại sau khi retry, hàm không báo lỗi ra ngoài mà lặng lẽ để `check_action()` (hoặc nhánh không-USB) tự quyết định `g_action` theo lối dự phòng.

**Function khác được gọi:**

- `check_action()` — để xác định chính xác loại boot dựa trên dữ liệu đọc từ USB.
- `km_linecard_boot_setting_read()` / `_write()` — để đọc/ghi cờ line-card boot trên SSD/SD.
- `usb_init()` / `usb_stor_scan()` — để dò và xác thực thiết bị lưu trữ USB.
- `THRG_getSoftDipSw()` / `THRF_readFlash()` — để đọc cấu hình từ SPI Flash khi không quét USB.
- `gpio_set_value()` — để bật/tắt phần cứng Motion Enable.
- `km_scpi_set_wdt_ca72_ext()` — để cấu hình watchdog timer phần cứng qua SCP.

### Giải thích

**Main purpose**: Đây là bước "định tuyến boot sớm" — quyết định BootROM sẽ chạy theo kịch bản nào (`g_action`) và đồng thời đồng bộ hai hệ thống phụ trợ (cơ cấu cơ khí + watchdog) cho khớp với kịch bản đó, tất cả xảy ra trước khi `main_loop()` — nơi thực sự dùng `g_action` để boot — được gọi tới.

**Happy path**: Không ai bấm phím utility, không có tồn dư cờ line-card từ lần trước → bỏ qua bước quét USB tốn thời gian → đọc DipSW không thấy gì đặc biệt → `g_action = DEF_BOOT` → Motion Enable được tính là "bật" nhưng việc ghi GPIO thực tế bị bỏ qua (đã có Sub-CPU lo) → không thuộc nhóm tác vụ nhạy cảm nên không delay → không thuộc nhóm "không tự boot" nên watchdog tiếp tục chạy → hàm trả về sau vài chục mili-giây.

**Edge case:**

- Phím utility active nhưng thực tế **không có USB cắm** → tốn tới ~20 giây chờ retry vô ích trước khi rơi về fallback DipSW.
- Cờ line-card lần trước bật nhưng SSD/SD lỗi đọc lúc này → cờ bị coi như "không có", bỏ lỡ việc bắt buộc quét USB dù đáng ra nên quét.
- Lần đầu tiên boot line-card (chưa từng có cờ) → hoàn toàn phụ thuộc vào phím utility vật lý có được bấm đúng lúc hay không.

**Điểm có thể gây lỗi:**

- Tổng thời gian retry USB (tối đa ~20s) khá sát với watchdog 120s ở bước quét, nhưng WDT được hạ xuống chỉ **5 giây** ngay trước khi gọi `check_action()` — nếu bước đó chậm bất thường trên phần cứng lỗi, có nguy cơ watchdog hết hạn giữa chừng.
- Việc "bỏ qua không ghi GPIO" dựa trên giả định Sub-CPU đã set đúng từ trước — nếu Sub-CPU lỗi hoặc mất đồng bộ, firmware sẽ không hề biết trạng thái Motion Enable thực tế đang sai.
- Cờ line-card được lưu trên **SSD/SD** (khác với hầu hết cờ boot khác nằm trên SPI Flash) — nếu ổ đĩa lưu trữ chưa sẵn sàng hoặc hỏng, cơ chế "nhớ line-card" âm thầm mất tác dụng mà không có cảnh báo riêng.

**Tóm tắt 1-3 câu**: `setup_MotionEnable_Power()` xác định loại boot (`g_action`) bằng cách quét USB khi cần hoặc đọc thẳng cấu hình DipSW khi không, rồi dùng kết quả đó để bật/tắt cơ cấu chuyển động cơ khí và cấu hình watchdog cho khớp với từng kịch bản boot — là mắt xích đầu tiên và có ảnh hưởng rộng nhất trong toàn bộ flow bootmode, vì mọi quyết định sau này (kể cả SuperWarp) đều dựa trên `g_action` mà hàm này thiết lập.
### A.1. Khai báo (149–162)

```c
unsigned char uc_MotionEnable = 0;   // cờ output: có bật GPIO Motion Enable không
unsigned char uc_IISWFlg;            // DipSW ảo đọc từ SPI Flash offset 0x06
unsigned char uc_MSCDLFlg;           // cờ "MSC firmware đang download dở"
unsigned char uc_CheckUSBFlg = 0x00; // cờ "boot trước là line-card, phải scan USB"
char uc_ssdBuff[512+10];             // đệm 1 sector, dùng để đọc/ghi cờ linecard lên SSD/SD
```

`uc_CheckUSBFlg` khởi tạo `0x00` (không bắt buộc quét USB) — giá trị thật được nạp ở khối tiếp theo.

### A.2 Đọc lại cờ "line-card boot" từ lần trước (176–187)

```c
ul_Ret = km_linecard_boot_setting_read( uc_ssdBuff );
uc_CheckUSBFlg = uc_ssdBuff[DEF_KM_WARP_SI_LINECARD_BOOT_FLG];   // offset 130
if( ul_Ret != 1 ) uc_CheckUSBFlg = 0x00;                          // đọc SSD lỗi → coi như không
else if( uc_CheckUSBFlg != 0x01 ) uc_CheckUSBFlg = 0x00;          // giá trị rác → không
```

Đáng chú ý: cờ này **không nằm trong vùng SPI Flash secure area** như các cờ khác — nó dùng byte offset 130 trong **sector 65 trên SSD/SD**, đúng vùng "Warp Setting Info" mà tài liệu SuperWarp trước đã phân tích (offset [128] trong bảng đó là A800 accessory, offset 130 là ô trống được tái sử dụng riêng cho flag này — comment `machine_setup.c:85` ghi rõ "sử dụng vùng trống cuối Warp!! setting info"). Lý do đặt ở SSD thay vì SPI Flash (comment 2020/01/28 OP_BTS-19623): tránh phải mount SPI vùng riêng chỉ để lưu 1 bit.

**Ý nghĩa cờ**: nếu lần boot **trước** là `BOOT_LINE` (boot line-card), lần **này** firmware phải chủ động quét USB dù điều kiện bình thường có thể bỏ qua — vì line card thường được rút/cắm lại giữa các lần boot.

### A.3 Điều kiện có quét USB hay không (189–190)

```c
if( (0 == uc_TempUtilityKey) || (1 == uc_CheckUSBFlg) ){   // low active
```

Vào nhánh USB nếu **một trong hai** đúng:

- `uc_TempUtilityKey == 0`: "utility key" (chân cứng, set bởi Sub-CPU nguồn phụ) đang active — nghĩa là kỹ thuật viên đã bấm/giữ phím để yêu cầu kiểm tra USB lúc bật máy.
- `uc_CheckUSBFlg == 1`: lần trước là line-card boot → bắt buộc quét dù utility key không active.

Comment dòng 189 giải thích thêm: ở `BOOT_HW_CHECKMODE` việc quét USB bị bỏ qua ở nơi khác (không liên quan trực tiếp khối này).

### A.4 Vòng quét USB với retry (191–208)

```c
km_scpi_set_wdt_ca72_ext( KM_SCPI_WDT_CA72_REFRESH, 0x1120, 120, 0, NULL );  // WDT 120s trong lúc dò USB
retry = 0;
do{
    usb_init();
    usb_stor_curr_dev = usb_stor_scan(1);
    retry++;
    usb_scan_retry = retry;                 // ghi global để debug/log biết số lần retry
    if((usb_stor_curr_dev < 0) && (retry < 2)){
        usb_stop();
        mdelay(10000);                      // chờ 10s rồi thử lại — tối đa 2 lần tổng cộng
    }
}while((usb_stor_curr_dev < 0) && (retry < 2));
km_scpi_set_wdt_ca72_ext( KM_SCPI_WDT_CA72_REFRESH, 0x1121, 5, 0, NULL );   // hạ WDT xuống 5s trước khi vào check_action()
check_action();                              // ← đây là điểm quyết định g_action thật sự (phần B bên dưới)
printf( "action = [%d]\n", g_action );
```

Chi tiết đáng chú ý:

- **Tối đa 2 lần scan**, cách nhau 10 giây — không phải retry vô hạn, để tránh treo máy nếu USB lỗi vật lý.
- WDT được refresh **hai lần với timeout khác nhau** (`0x1120`/120s bao trùm cả vòng scan có thể mất tới ~10s+, `0x1121`/5s ngay trước khi gọi `check_action()` vì hàm đó phải trả về nhanh). Các số hex `0x1120`, `0x1121` là **mã điểm kiểm tra (checkpoint id)** gửi sang LPPP — nếu máy treo, log phía LPPP sẽ cho biết treo ở đúng bước nào.
- Dòng 207 (`km_scpi_set_wdt_ca72_ext(...WDT_CA72_END...)`) bị **comment-out** kèm chú thích 2022/11/04 — trước đây USB-scan xong thì tắt WDT, nay cố tình giữ WDT chạy tiếp xuyên suốt cả USB download/IISW để không bị treo giữa chừng do thiếu refresh.
### A.5 Nhánh KHÔNG có USB (209–252)

```c
uc_Ret = THRG_getSoftDipSw( 0x000006, &uc_IISWFlg, sizeof(uc_IISWFlg) );
if( THRD_GET_SPI_OK != uc_Ret && ... ) uc_IISWFlg = 0x00;   // đọc lỗi → coi như DipSW=0

if( 0 != THRF_readFlash( 0x07B000 + DEF_USERDATA_OFFSET, &uc_MSCDLFlg, sizeof(uc_MSCDLFlg) ) )
    uc_MSCDLFlg = 0x00;
```

Đây là bản đọc **trực tiếp** offset `0x000006` (soft-DipSW ảo trong SPI Flash) — cùng offset mà `getNWDLExec()` trong `check_action()` sẽ đọc lại (phần B), nhưng ở đây gọi thẳng `THRG_getSoftDipSw()` chứ không qua hàm trung gian. Thêm một cờ mới không xuất hiện trong nhánh USB: `uc_MSCDLFlg` tại `0x07B000 + DEF_USERDATA_OFFSET` — đánh dấu "firmware chính (MSC) đang dở dang download".

```c
if( 5 == uc_IISWFlg || 6 == uc_IISWFlg )                 g_action = DEF_IISW;
else if( 4/7/8/9 == uc_IISWFlg )                          g_action = DEF_UPDATE;
else if( 1 == uc_MSCDLFlg )                               g_action = BOOT_ERR;   // ★ chỉ có ở nhánh này
else if( 10 == uc_IISWFlg )                               g_action = DEF_BACKUP;
else if( 11 == uc_IISWFlg )                               g_action = DEF_RESTORE;
else if( 1 == uc_HWcheckModeBoot )                        g_action = BOOT_HW_CHECKMODE;
else                                                       g_action = DEF_BOOT;
```

So với `check_action()` (nhánh USB-absent nội bộ của nó ở B.1), thứ tự ưu tiên **khác nhau đáng kể**: ở đây `uc_MSCDLFlg` (MSC chưa tải xong) được chèn ưu tiên **cao hơn** backup/restore/HW-checkmode — nghĩa là nếu firmware chính chưa hoàn tất mà không có DipSW update nào set, máy sẽ báo lỗi boot ngay thay vì cố boot bình thường. Đây là khác biệt tinh vi dễ bị bỏ sót khi đọc lướt.

### A.6 Ghi lại cờ line-card cho lần boot sau (254–269)

```c
if( g_action == BOOT_LINE && uc_CheckUSBFlg == 0x00 ){
    uc_CheckUSBFlg = 0x01;
    uc_ssdBuff[DEF_KM_WARP_SI_LINECARD_BOOT_FLG] = uc_CheckUSBFlg;
    km_linecard_boot_setting_write( uc_ssdBuff );          // bật cờ
} else if( g_action != BOOT_LINE && uc_CheckUSBFlg == 0x01 ){
    uc_CheckUSBFlg = 0x00;
    uc_ssdBuff[DEF_KM_WARP_SI_LINECARD_BOOT_FLG] = uc_CheckUSBFlg;
    km_linecard_boot_setting_write( uc_ssdBuff );          // tắt cờ
}
```

Đây chính là cơ chế khép vòng với A.2: boot này line-card → set cờ để **boot kế tiếp** bắt buộc quét USB (A.3); boot này không còn line-card nữa → xoá cờ, quay lại hành vi thường (chỉ quét khi utility key active).

### A.7 Motion Enable — không liên quan trực tiếp g_action nhưng dùng g_action làm điều kiện (271–312)

```c
if(0 != uc_TempUtilityKey){                 // utility key KHÔNG active (bình thường)
    if( g_action không thuộc {USB_UPDATE, DEF_IISW, DEF_UPDATE, BOOT_ERR, DEF_BACKUP, DEF_RESTORE, BOOT_SPEEDCHANGE} )
        uc_MotionEnable = 1;
} else if(g_action == DEF_BOOT && gPanelKey != 0x16)   uc_MotionEnable = 1;
else if(g_action == USB_BOOT)                           uc_MotionEnable = 1;
else if(g_action == BOOT_LINE && gPanelKey != 0x16)     uc_MotionEnable = 1;
```

Logic: bật GPIO "Motion Enable" (cho cơ cấu chuyển động cơ khí của máy MFP) trong hầu hết trường hợp boot bình thường, **trừ** các chế độ liên quan tới download/update/lỗi (nơi cơ cấu cơ khí không nên hoạt động) — và trừ khi phím panel đang ở trạng thái Trouble-Reset (`0x16`, cùng giá trị panel key đã gặp ở phần SuperWarp).

```c
if( ( g_action != DEF_BOOT ) || ( 0 == uc_TempUtilityKey ) ) {
    gpio_set_value( GPIO_PORT_MOTION_EN_MC, uc_MotionEnable ? HIGH : LOW );
    gpio_set_value( GPIO_PORT_MOTION_EN_IR, uc_MotionEnable ? HIGH : LOW );
}
```

Comment dòng 291 giải thích: với `DEF_BOOT` **và** utility key không active (trường hợp phổ biến nhất — boot bình thường không ai bấm phím), GPIO này **đã được set sẵn bởi Sub-CPU nguồn phụ** trước đó rồi, nên code ở đây chủ động **bỏ qua** để khỏi ghi đè.

### A.8 Delay cơ khí + tắt WDT có điều kiện (314–339)

```c
if( g_action == DEF_IISW || DEF_UPDATE || DEF_BACKUP || DEF_RESTORE ){
    km_scpi_set_wdt_ca72_ext( KM_SCPI_WDT_CA72_REFRESH, 0x1130, 10, 0, NULL );
    mdelay(4500);   // 4.5s chờ relay cơ khí ổn định (code điều khiển relay MC_P_ON thật đã bị comment-out, chờ HW mới)
}

if( g_action thuộc {DEF_BACKUP, DEF_RESTORE, BOOT_HW_CHECKMODE, BOOT_ERR, BOOT_SPEEDCHANGE, BOOT_LINE} ){
    km_scpi_set_wdt_ca72_ext( KM_SCPI_WDT_CA72_END, 0, 0, 0, NULL );   // các mode "không tự boot kernel" thì tắt WDT hẳn
}
```

Ghi chú: các `g_action` bị tắt WDT ở đây đều là mode dừng lại chờ tương tác người (backup/restore/HW-check/error/speed-change/line) chứ không boot thẳng vào kernel — nên không cần watchdog tự động reset nữa.

## B. `check_action()` — main.c:568 (chỉ được gọi khi A.4 xác nhận có/nghi có USB)

## B.0 Tổng quát
### Phân tích tổng quan

**Function này dùng để làm gì?** `check_action()` là "bộ phân loại yêu cầu USB" của BootROM. Nó đọc nội dung trên thiết bị USB đang cắm (file `INDEX`, hoặc nếu không có thì dò các file `.tar` đặc biệt) để xác định chính xác **thao tác nào** người dùng/kỹ thuật viên muốn thực hiện — boot bình thường, update firmware, backup/restore, line-card, factory-reset, board-erase, hay đổi tốc độ khởi động — rồi trả kết quả đó ra dưới dạng `g_action`.

**Input** — hàm không có tham số (`void`), input thực chất là:

- Trạng thái cổng USB (`check_usb_port()`) — kiểm tra lại độc lập, không tin tưởng hoàn toàn caller.
- Biến môi trường `bootusb` (chỉ số thiết bị USB) — do bước scan USB ở hàm gọi nó set sẵn.
- Nội dung file trên phân vùng FAT của USB: file `INDEX`, hoặc phần đầu 3 loại file `.tar` đặc biệt.
- (Nhánh dự phòng) cấu hình DipSW ảo trong SPI Flash offset `0x06`, cờ `uc_HWcheckModeBoot`.

**Output** — trả về `int` = `g_action` vừa xác định (đồng thời gán vào biến toàn cục `g_action`).

**Side effect quan trọng:**

- Ghi biến toàn cục `g_action` — kết quả chính, chi phối toàn bộ flow phía sau.
- Ghi biến toàn cục `g_UsbKind` — loại USB vừa nhận diện được.
- Thực hiện I/O thật lên USB nhiều lần (`do_fat_fsload`) — có thể chậm hoặc lỗi nếu thiết bị/thẻ có vấn đề.
- In log chẩn đoán qua `printf` ở hầu hết các nhánh.
### Flowchart mức HIGH-LEVEL

```c
[START]
   │
   ▼
[Nhận input] Chuẩn bị vùng đệm RAM để tải file, đọc biến "bootusb"
   │
   ▼
❓ Cổng USB có thực sự được phát hiện không? (kiểm tra độc lập, không tin tưởng caller)
   │
   ├── NO ──▶ [Đọc cấu hình DipSW nội bộ trong SPI Flash để tự suy loại boot
   │           — nhánh dự phòng, không cần USB]
   │              │
   │              ▼
   │           [Gán g_action tương ứng theo giá trị DipSW] ──▶ RETURN (nhánh dự phòng)
   │
   ▼ YES
❓ Biến "bootusb" (chỉ số thiết bị) có tồn tại không?
   │
   ├── NO ──▶ return DEF_BOOT  (không biết đọc thiết bị nào)
   │
   ▼ YES
[Gọi function do_fat_fsload() để tải file INDEX từ USB vào bộ nhớ]
   │
   ▼
❓ Đọc file INDEX thành công không?
   │
   ├── NO ──▶ [Lần lượt thử tải phần đầu 3 loại file .tar đặc biệt:
   │           line-card / init-card / board-erase]
   │              │
   │              ▼
   │           Với mỗi file: gọi function sysBspCmpTypeName() để so khớp loại,
   │           và sysBspGetUsbMemType() để xác định loại USB;
   │           riêng init-card/board-erase còn so khớp thêm 1 "mã xác thực" dài
   │              │
   │              ▼
   │           ❓ Có loại nào khớp đủ điều kiện không?
   │              ├── YES ──▶ gán g_action = BOOT_LINE / BOOT_INIT / BOOT_BDERASE ──▶ RETURN
   │              └── NO  ──▶ return DEF_BOOT  (không khớp gì cả)
   │
   ▼ YES (đọc INDEX thành công)
[Gọi function sysBspGetUsbMemType() để xác định loại yêu cầu ghi trong INDEX]
   │
   ▼
❓ Tên máy trong INDEX có khớp với máy hiện tại không?
   (miễn kiểm tra riêng cho trường hợp Speed-Change)
   │
   ├── NO ──▶ g_action = BOOT_ERR ──▶ RETURN  (từ chối — sai loại USB cho máy này)
   │
   ▼ YES
[switch theo loại USB đã nhận diện]
   ├── Yêu cầu download update ─────▶ g_action = USB_UPDATE
   ├── Yêu cầu boot trực tiếp từ USB ─▶ g_action = USB_BOOT
   ├── Yêu cầu đổi tốc độ khởi động ──▶ g_action = BOOT_SPEEDCHANGE
   └── Dữ liệu không hợp lệ/không rõ ─▶ g_action = BOOT_ERR
   │
   ▼
RETURN g_action
   │
   ▼
[END]
```

**Error/exception path**: không có cơ chế exception (C nhúng) — "đường lỗi" là các **return phòng thủ**: `DEF_BOOT` khi không tìm ra gì hợp lệ (coi như bỏ qua, boot bình thường), `BOOT_ERR` khi phát hiện dữ liệu sai (sai tên máy, sai loại thẻ, USB không hợp lệ).

**Function khác được gọi:**

- `do_fat_fsload()` — để tải file (INDEX hoặc phần đầu file tar) từ USB vào RAM.
- `sysBspCmpTypeName()` — để so khớp chuỗi loại máy/loại thao tác ghi trong file.
- `sysBspGetUsbMemType()` — để phân loại nội dung INDEX/tar thành một trong các loại USB đã định nghĩa.
- `getNWDLExec()` — để đọc DipSW ảo trong nhánh dự phòng không-USB.

### Giải thích

**Main purpose**: Đây là "cổng kiểm duyệt" mọi thao tác khởi động từ USB — nó đọc dữ liệu thật trên thiết bị để phân biệt chính xác 7 loại yêu cầu khác nhau, và chủ động từ chối (`BOOT_ERR`) nếu dữ liệu không hợp lệ hoặc không dành cho đúng dòng máy.

**Happy path**: USB đã cắm, `bootusb` đã có sẵn từ bước scan trước, đọc `INDEX` thành công ngay lần đầu, tên máy khớp, loại nhận diện được (ví dụ `@ZZZ`) → `g_action = USB_BOOT` → return ngay, không cần thử các file `.tar` dự phòng.

**Edge case:**

- USB cắm vào nhưng không có file `INDEX` (USB thường, không phải USB service) → hàm không kết luận lỗi ngay mà thử tiếp cả 3 loại file `.tar` đặc biệt trước khi rơi về `DEF_BOOT`.
- Có file `.tar` đúng tên loại (init-card/board-erase) nhưng **sai mã xác thực** bên trong → bị từ chối âm thầm, rơi về `DEF_BOOT` mà không có thông báo rõ vì sao factory-reset/board-erase không chạy — khó debug tại hiện trường.
- Cổng USB "biến mất" ngay trong chính hàm này dù caller đã nghĩ là có USB → rơi vào nhánh dự phòng nội bộ, gần như lặp lại logic đã có ở `setup_MotionEnable_Power()`.
- `USB_SPD` (speed-change) được đặc cách **miễn kiểm tra tên máy** — nếu dùng nhầm USB speed-change từ dòng máy khác, hàm vẫn chấp nhận.

**Điểm có thể gây lỗi:**

- Buffer `cindex` có kích thước cố định (`CF_INDEX_SIZE*TARACES_BLOCK`); nếu nội dung file lớn hơn buffer, dữ liệu đọc có thể bị cắt và so khớp sai mà không có cảnh báo.
- Việc so khớp dựa vào **offset cố định** trong dữ liệu nhị phân của header tar (`TAR_HEAD_BLOCK+6`) — nếu công cụ đóng gói tar khác chuẩn một chút, toàn bộ nhận diện lệch mà không phát hiện được ở runtime.
- Nhánh dự phòng "không có USB" bên trong `check_action()` gần như **trùng lặp logic** với nhánh tương tự ở `setup_MotionEnable_Power()` (đã phân tích lần trước) — dễ lệch pha nếu chỉ một bên được sửa; và nhánh `default` ở đây (dòng trả `DEF_BOOT`) **không chủ động gán lại `g_action`**, chỉ dựa vào giá trị mặc định ban đầu của biến toàn cục.

**Tóm tắt 1-3 câu**: `check_action()` là bộ phân loại yêu cầu USB của BootROM — đọc file `INDEX` hoặc dò các file `.tar` đặc biệt trên thiết bị USB đang cắm để xác định chính xác `g_action` cần chạy (boot/update/backup/restore/factory-reset/board-erase/speed-change), từ chối bằng `BOOT_ERR` nếu dữ liệu không hợp lệ hoặc sai dòng máy, và có nhánh dự phòng tự suy loại boot từ SPI Flash nếu hoá ra không có USB thật.
### B.1 Khai báo & mảng tham số `fatload` (568–593)

```c
ALLOC_CACHE_ALIGN_BUFFER(char, cindex, CF_INDEX_SIZE*TARACES_BLOCK);
// CF_INDEX_SIZE=50 → cindex là buffer align-cache, kích thước 50×TARACES_BLOCK byte, dùng làm nơi fatload nạp file INDEX/tar vào

char *args[6]      = {"fatload","usb",dev_str,adr_str, FW_DIRECTRY"/INDEX", size_str};      // đọc file INDEX
char *argsLine[6]  = {"fatload","usb",dev_str,adr_str, TAR_FILE_NAME,       size_str};      // đọc đầu file tar line-card
char *argsInit[6]  = {"fatload","usb",dev_str,adr_str, TAR_FILE_NAME_INIT,  size_str};      // đọc đầu file tar init-card
char *argsBdErase[6]={"fatload","usb",dev_str,adr_str, TAR_FILE_NAME_BDERASE,size_str};     // đọc đầu file tar board-erase
```

Đây thực chất là **mảng tham số giả lập gọi lệnh shell `fatload usb ... <file>`** trực tiếp bằng C, không qua console — mỗi mảng ứng với một tên file cụ thể sẽ thử đọc trên phân vùng FAT của USB.

### B.2 "Nhánh không-USB" bên trong chính `check_action()` (598–630)

```c
if( check_usb_port() == 0 ){          // check_usb_port() TỰ kiểm tra lại, độc lập với A.3
    uc_NWDLExec = getNWDLExec();      // đọc lại offset 0x06 — TRÙNG với A.5 nhưng qua hàm khác
    if( 5==uc_NWDLExec || 6==uc_NWDLExec )      { g_action=DEF_IISW;  return DEF_IISW; }
    else if( 4/7/8/9==uc_NWDLExec )             { g_action=DEF_UPDATE; return DEF_UPDATE; }
    else if( 10==uc_NWDLExec )                  { g_action=DEF_BACKUP; return DEF_BACKUP; }
    else if( 11==uc_NWDLExec )                  { g_action=DEF_RESTORE; return DEF_RESTORE; }
    else {
        if( 1==uc_HWcheckModeBoot ) { g_action=BOOT_HW_CHECKMODE; return BOOT_HW_CHECKMODE; }
        else return DEF_BOOT;                    // ★ KHÔNG set g_action — giữ nguyên giá trị cũ!
    }
}
```

**Điểm cần nhấn mạnh**: đây gần như là **bản sao logic** của A.5, nhưng vẫn có thể được kích hoạt thật sự — vì `check_action()` chỉ được gọi khi A.3 đúng (`uc_TempUtilityKey==0` **hoặc** cờ line-card cũ), điều đó **không đảm bảo** `usb_stor_scan()` ở A.4 thực sự tìm thấy thiết bị. Ví dụ: cờ line-card từ lần trước bật (A.3 đúng), nhưng lần này người dùng **không** cắm lại line-card → `usb_stor_scan` thất bại, `check_usb_port()` ở đây trả 0, và ta rơi vào đúng nhánh này thay vì nhánh USB thật bên dưới. Trường hợp default (dòng 627 `return DEF_BOOT`) là bug tiềm ẩn nhỏ: nó return giá trị nhưng **không gán `g_action = DEF_BOOT`**, khác với mọi case khác — may là `g_action` khởi tạo mặc định đã là `DEF_BOOT` (main.c:539) nên không gây sai lệch trong thực tế, nhưng không nhất quán về style.

### B.3 Chuẩn bị đọc INDEX thật (632–644)

```c
bootusb_str = getenv( "bootusb" );
if( NULL == bootusb_str ) return DEF_BOOT;      // biến env "bootusb" (số thiết bị USB, set bởi usb_stor_scan) không có → bỏ cuộc

sprintf( dev_str, "%s:1", bootusb_str );         // vd "0:1" — partition 1 của thiết bị USB đó
sprintf( adr_str, "%lx", (unsigned long)cindex ); // địa chỉ RAM đích dạng hex, chính là buffer cindex ở B.1
sprintf( size_str, "%x", CF_INDEX_SIZE*TARACES_BLOCK );

if( do_fat_fsload( NULL, 0, 6, args ) != 0 ){    // thử đọc FW0000/INDEX — thất bại (!=0) thì đi vào khối B.4
```

### B.4 INDEX không đọc được → dò lần lượt 3 loại file `.tar` đặc biệt (645–749)

Cấu trúc lặp lại 3 lần theo cùng một khuôn mẫu — mỗi khối: (1) `do_fat_fsload` đọc 2560 byte đầu file tar cụ thể, (2) so khớp "type name" tại offset `TAR_HEAD_BLOCK+6` trong header tar, (3) nếu khớp thì gọi `sysBspGetUsbMemType()` để lấy `g_UsbKind` từ 5 ký tự đầu, (4) so khớp thêm một **identifier bảo mật dài** (chuỗi hex ~40-50 ký tự) bằng `strstr()` trước khi chấp nhận:

```c
// (1) Line-card:
if( sysBspCmpTypeName(&cindex[TAR_HEAD_BLOCK+6], TYP_NAME_LINECARD) == 0 ) {
    g_UsbKind = sysBspGetUsbMemType(&cindex[TAR_HEAD_BLOCK]);
    if( g_UsbKind == USB_XYZ ){ g_action = BOOT_LINE; return BOOT_LINE; }
}
// (2) Initialize-card (@WVU) — thêm bước strstr() tìm identifier DEF_INITIALIZE_CARD_IDENFITIFER:
if( sysBspCmpTypeName(&cindex[TAR_HEAD_BLOCK+6], TYP_NAME_INIT) == 0 ) {
    g_UsbKind = sysBspGetUsbMemType(&cindex[TAR_HEAD_BLOCK]);
    if( g_UsbKind == USB_WVU && strstr(&cindex[TAR_HEAD_BLOCK+6], DEF_INITIALIZE_CARD_IDENFITIFER) != NULL ){
        g_action = BOOT_INIT; return BOOT_INIT;
    }
}
// (3) Board-erase (@BDE) — tương tự, thêm strstr() với DEF_BDERASE_CARD_IDENFITIFER:
if( sysBspCmpTypeName(&cindex[TAR_HEAD_BLOCK+6], TYP_NAME_BDERASE) == 0 ) {
    g_UsbKind = sysBspGetUsbMemType(&cindex[TAR_HEAD_BLOCK]);
    if( g_UsbKind == USB_BDE && strstr(&cindex[TAR_HEAD_BLOCK+6], DEF_BDERASE_CARD_IDENFITIFER) != NULL ){
        g_action = BOOT_BDERASE; return BOOT_BDERASE;
    }
}
return DEF_BOOT;   // không khớp gì cả → boot bình thường
```

**Vì sao có bước `strstr()` identifier riêng cho `BOOT_INIT`/`BOOT_BDERASE`** mà `BOOT_LINE` thì không? Vì hai lệnh này là thao tác **phá hủy dữ liệu không hồi phục** (factory reset / board erase — đã phân tích ở tài liệu SuperWarp trước) — chuỗi hex dài đóng vai trò như "mật khẩu" nhúng trong tên file tar, chống việc vô tình (hoặc cố ý) trigger chỉ bằng cách đặt tên file `a789em.tar`/`a789be.tar` mà không có nội dung xác thực đúng bên trong.

### B.5 INDEX đọc thành công → nhánh chính (754–804)

```c
g_UsbKind = sysBspGetUsbMemType(cindex);          // đọc 5 ký tự đầu của chính file INDEX
if( g_UsbKind != USB_SPD && sysBspCmpTypeName(&cindex[6],TYP_NAME) != 0 ) {
    // USB_SPD được miễn kiểm tra tên máy (speed-change dùng chung cho nhiều model)
    g_action = BOOT_ERR; return BOOT_ERR;          // sai tên máy trong INDEX → từ chối thẳng
}

switch( g_UsbKind ){
    case USB_DOWN:   g_action = USB_UPDATE;        return USB_UPDATE;   // download FW
    case USB_ZZZ:    g_action = USB_BOOT;           return USB_BOOT;    // boot từ USB memory
    case USB_SPD:     g_action = BOOT_SPEEDCHANGE;   return BOOT_SPEEDCHANGE;
    case USB_INVALD:  g_action = BOOT_ERR;           return BOOT_ERR;
    default:          g_action = BOOT_ERR;           return BOOT_ERR;
}
```

`case USB_XYZ` (line-card qua INDEX trực tiếp) bị **bọc trong `#if 0`** (dòng 780–787) — comment giải thích: đường dẫn line-card giờ chỉ đi qua nhánh tar-file ở B.4, không còn cho phép chỉ định trực tiếp qua INDEX nữa, code cũ được giữ lại dạng tài liệu tham khảo chứ không biên dịch.

## Tóm lại — 3 điều dễ hiểu nhầm nếu chỉ đọc lướt

1. **`g_action` có thể bị set ở 2 nơi khác nhau cho cùng một lần boot**: A.5 (không-USB, đọc DipSW trực tiếp) hoặc B.2+B.5 (`check_action()`, khi thật sự có USB) — nhưng cả hai không bao giờ chạy cùng lúc vì A.3 định tuyến trước.
2. **`check_action()` tự nó cũng có một nhánh không-USB gần như trùng lặp** (B.2) — không phải code chết, mà là lưới an toàn cho trường hợp "tưởng có USB nhưng scan thất bại".
3. **Chỉ `BOOT_INIT` và `BOOT_BDERASE`** — hai lệnh phá hủy dữ liệu — mới yêu cầu thêm identifier hex dài bên trong file tar; `BOOT_LINE` chỉ cần đúng tên loại (`USB_XYZ`) là đủ.