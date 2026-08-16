# Trình bày BOOT_HW_CHECKMODE

`BOOT_HW_CHECKMODE` (giá trị `7`) là chế độ **chẩn đoán phần cứng** — và là g_action duy nhất có một **lớp xử lý riêng ngay trong `misc_init_r()`**, trước cả khi `setup_MotionEnable_Power()`/`check_action()` chạy, với khả năng **dừng máy vĩnh viễn sớm hơn cả BOOT_INIT/BOOT_BDERASE** nếu chẩn đoán tự động phát hiện lỗi.

## 1. Hai nguồn kích hoạt độc lập — mới xác minh đầy đủ

Cờ `uc_HWcheckModeBoot` được set trong `init_km()` (board_r.c:452-465), **trước** cả `misc_init_r()`:

```c
if( 1 == is_execHwCheckMode() ){          // (A) FIRMWARE CHÍNH tự yêu cầu
    uc_HWcheckModeBoot = 1;
    uc_AutoHWcheckMode = 1;               // ← đánh dấu đây là chế độ TỰ ĐỘNG
    THRF_initAutoHwDiagnosCSRAData();
}
else if ( 0 == is_RestartOccured() && 1 == checkPowerSaveKey() ) {   // (B) NGƯỜI DÙNG giữ phím
    uc_HWcheckModeBoot = 1;               // uc_AutoHWcheckMode vẫn = 0 → chế độ THỦ CÔNG
}
```

- **(A) Tự động**: `is_execHwCheckMode()` đọc một cờ **tự-xoá** (`clr_execHwCheckMode()` gọi ngay sau khi đọc) từ SPI Flash offset `THRD_SPI_OFFSET_EXEC_HW_CHECK_MODE` — cờ này do **chính firmware chính (main FW)** ghi từ trước rồi tự reboot, tức người dùng bấm "chạy chẩn đoán" từ giao diện máy, main FW ghi cờ → reboot → BootROM đọc thấy → vào chế độ HW-check **tự động**.
- **(B) Thủ công**: chỉ khi **không phải warm-restart** (`is_RestartOccured()==0`, tức cold power-on thật sự) **và** phím power-save vật lý (`GPIO_PORT_PANEL_SUBSW`) đang bị giữ lúc bật nguồn.

Hai nguồn này dẫn tới **hai luồng hành vi khác nhau đáng kể** ở các bước sau.

## 2. Lớp gác đầu tiên — `boot_diag()` trong `misc_init_r()` (chỉ áp dụng nhánh tự động)

```c
void boot_diag(void)
{
    if( 1 != uc_HWcheckModeBoot ) return;          // không phải HW-checkmode → bỏ qua hoàn toàn

    if ( 1 == uc_AutoHWcheckMode ) {                // CHỈ nhánh (A) tự động mới chạy khối này
        if( true != fsload_hash_check() ) {         // xác minh SHA-256 các ảnh storage so với EXP_INDEX
            THRF_setAutoHwDiagnosStorage( 0x46 );   // ghi mã chẩn đoán lỗi
            set_Panel_BootDiagErr( PANEL_BOOTDIAG_STORAGE_ERR );
            set_Panel_BootDiagResult();
            _WAIT_FOREVER();                        // ← DỪNG VĨNH VIỄN, ngay trong misc_init_r()!
        }
        THRF_initAutoHwDiagnosStorageCSRAData();
    }
}
```

Nếu chọn nhánh **tự động** và hash SHA-256 của các ảnh lưu trữ (so với giá trị khai báo trong `EXP_INDEX`) không khớp → máy **dừng ngay tại đây**, còn sớm hơn cả điểm chặn `BOOT_INIT`/`BOOT_BDERASE` ở `main_loop()`. Nhánh **thủ công** (`uc_AutoHWcheckMode==0`) bỏ qua toàn bộ khối này, `boot_diag()` chỉ là no-op.

Trước đó, `misc_init_r()` còn vẽ riêng một "màn hình troubleshooting" (`DrawPanel_lzo(DEF_LZO_TROUB_PIC, ...)`) **chỉ khi** `uc_AutoHWcheckMode==1` — người bấm giữ phím thủ công sẽ không thấy màn hình này.

## 3. Nếu qua được — vào `setup_MotionEnable_Power()` / `check_action()` như bình thường

- **Motion Enable**: `BOOT_HW_CHECKMODE` **không** nằm trong danh sách loại trừ (dòng 274-277) → nếu utility key không active vẫn bật `uc_MotionEnable=1` — hợp lý vì chẩn đoán phần cứng thường cần vận hành thử cơ cấu cơ khí.
- **WDT**: nằm trong nhóm 6 g_action **tắt WDT sớm** ở `setup_MotionEnable_Power()`.
- `check_action()`/DipSW không set lại g_action này (nó đã được `init_km()` gán rất sớm, giữ nguyên xuyên suốt).

## 4. `check_autoboot()` — dùng chung ảnh production, khác `DEF_BOOT` chỉ ở 1 tham số

```c
case BOOT_HW_CHECKMODE:
    setenv("bootcmd",  make_bootcmds(CONFIG_CMD_ENV));      // partition 0:5 — GIỐNG DEF_BOOT
    setenv("bootargs", make_bootargs(g_action,CONFIG_SSD_RFS_ENV));
    autoboot = AUTOBOOT_HW_CHECKMODE;
```

Trong `make_bootargs()` có thêm một đoạn **chỉ áp dụng cho BOOT_HW_CHECKMODE**:

```c
if ( ( BOOT_HW_CHECKMODE == boottype ) && ( 1 == uc_AutoHWcheckMode ) ) {
    strncat( chBtEv, " SubsetChk=1 ", ... );   // chỉ nối thêm nếu là nhánh TỰ ĐỘNG
}
```

```
boottyp=7 root=/dev/kmsda7 rootdelay=1 [nopanel] console=...
TerminalState=<...> MachineInfo=<hex> MachineNo=<hex> SerialNo=<...>
IISW=0 FwDir=FW9020 DevStr=nvme IntegrityState=<0|1> [SubsetChk=1] <CONFIG_BOOTARGS>
```

`SubsetChk=1` **chỉ xuất hiện khi kích hoạt bằng nhánh (A) tự động** — nhánh (B) thủ công (giữ phím) không có tham số này. Đây là cách userspace phân biệt "main FW tự yêu cầu chẩn đoán toàn diện" với "kỹ thuật viên giữ phím vào chế độ kiểm tra tay".

## 5. WDT cuối & Warp

- **`autoboot_command()`**: `g_action==BOOT_HW_CHECKMODE` không khớp `{DEF_BOOT,USB_UPDATE,DEF_IISW,DEF_UPDATE}` → `else` → `WDT_CA72_END`.
- **Warp**: `autoboot=AUTOBOOT_HW_CHECKMODE` không khớp điều kiện Warp → luôn cold-boot, dùng **đúng ảnh firmware production** như `DEF_BOOT`.

## 6. Tóm tắt

`BOOT_HW_CHECKMODE` có 2 đường kích hoạt (main FW tự yêu cầu qua cờ SPI tự-xoá, hoặc kỹ thuật viên giữ phím power-save lúc cold-boot) dẫn tới hai hành vi khác nhau: nhánh tự động có thể **dừng máy vĩnh viễn ngay trong `misc_init_r()`** nếu hash storage sai, còn nếu qua được thì cả hai nhánh đều boot **đúng kernel production** (partition `0:5`, giống hệt `DEF_BOOT`) nhưng gắn thêm `boottyp=7` (+ `SubsetChk=1` riêng cho nhánh tự động) để userspace tự chuyển sang giao diện chẩn đoán thay vì vận hành bình thường.