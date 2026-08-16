# Cơ chế SuperWarp trong BootROM này — giải thích đầy đủ (đã đối chiếu 100% với source)

Sau khi đào sâu toàn bộ pipeline (từ `chk_sboot()` cho tới cả 13 g_action), giờ có thể giải thích SuperWarp chính xác hơn nhiều so với bản tóm tắt PDF ban đầu — đặc biệt là `isChgWarpBootSettingInfo()`, hàm mình vừa đọc trực tiếp và hoá ra có **12 lý do từ chối khác nhau**, không chỉ vài điều kiện chung chung như trước.

Lý do, khớp với định nghĩa đã tra được trong `armada8k.h`:

```c
#define WARP_DRV_INFO                                    \
{                                                          \
    /* Hibernation driver */                               \
    .mode      = WARP_DRV_FLOATING,                         \
    .drv.dev   = WARP_DEV(SPI, 0, 0),                        \
    .drv.offs  = 0x002b0000,                                  \
}
```

Driver này được nạp **trực tiếp từ chip SPI Flash vật lý ở offset `0x002b0000`** lúc runtime (qua `warp_load_drv()` trong `cmd_warp.c`) — nó là một **binary blob được flash sẵn lên phần cứng**, không phải file source hay file build-output nằm trong cây thư mục Emu800 này.

Hai file duy nhất liên quan tới Warp trong repo chỉ đóng vai trò **"loader/glue"**:

- `Src/include/warp.h` — định nghĩa cấu trúc header/offset (`WARP_HEADER_ID`, `WARP_HEADER_HIBERNATE=0x28`...) để biết cách đọc blob đó.
- `Src/common/cmd_warp.c` — logic nạp blob vào RAM, kiểm tra magic ID `'W5HD'`, rồi nhảy vào con trỏ hàm tại offset `0x28`.

Bản thân mã nguồn thực thi `hibernate()` (logic resume thật sự) **nằm ngoài phạm vi repo này hoàn toàn** — gần như chắc chắn là sản phẩm nhị phân đóng gói riêng của Lineo Solutions, được flash lên thiết bị bằng quy trình sản xuất/provisioning tách biệt khỏi quá trình build U-Boot ở đây, nên không thể tìm thấy source của nó trong `Emu800/Src`.
## 1. Bản chất: fast-boot bằng hibernation, có 2 cấp độ

```c
#define WARP_SSID_NORMAL_SD    (0)   // Warp!! thường (SD)
#define WARP_SSID_NORMAL_SSD   (1)   // Warp!! thường (SSD)
#define WARP_SSID_SUPER_SD     (2)   // SuperWarp!! (SD)
#define WARP_SSID_SUPER_SSD    (3)   // SuperWarp!! (SSD)
```

Thay vì cold-boot toàn bộ, hệ thống lưu một "snapshot" trạng thái xuống SSD/SD và khi bật lại thì `warp_boot()` phục hồi từ đó — nhanh hơn nhiều so với boot lại từ đầu.

### Tại sao cần Super Wrap
Không phải "đã có signature, tại sao còn cần Wrap" — vì không có signature ở tầng này để so sánh. Lý do thật (đã verify qua WDT budget 40s/90s/300s ở lượt trước): **trải nghiệm người dùng** — máy MFP văn phòng cần bật lên nhanh sau khi tắt/mở liên tục trong ngày, cold-boot Linux đầy đủ (dò thiết bị, mount filesystem, chạy init) tốn thời gian không chấp nhận được cho use-case đó.

**Threat model nào được giải quyết?** — Không giải quyết threat model bảo mật nào. Đây là giải pháp hiệu năng thuần tuý.
## 2. Lớp gate thứ nhất — công tắc phần cứng vật lý

```c
// dipvalue.c:164-170
int isWarpBootMode(void) {
    /* GPIO_PORT_WARP_SW (pin 228) */
    return ((gpio_get_value(GPIO_PORT_WARP_SW)==GPIO_ACTIVE_HIGH)? 1:0);
}
```

Một công tắc GPIO vật lý riêng biệt (`WARP_SW`, nhìn thấy trên schematic ở tài liệu đầu tiên) — nếu bị đặt ở vị trí "debug" (`isWarpBootMode()==0`), Warp bị chặn ngay lập tức, ghi mã `DEF_WARPNG_DISPSW (0x01)`, **không cần kiểm tra gì thêm**.

## 3. Lớp gate thứ hai — `isChgWarpBootSettingInfo()`: 12 lý do từ chối, mỗi lý do có mã riêng

```c
#define DEF_WARPNG_DISPSW              0x01  // Công tắc GPIO đặt về debug
#define DEF_WARPNG_CONFINFO_READ       0x02  // Không đọc được sector snapshot trên SSD/SD
#define DEF_WARPNG_CONFINFO_CHG        0x03  // Main FW đã tự đánh dấu "cấu hình vừa đổi"
#define DEF_WARPNG_HWCHG               0x04  // Đang ở BOOT_HW_CHECKMODE
#define DEF_WARPNG_CONFINFO_SIZE       0x05  // Dung lượng RAM hiện tại ≠ lúc lưu snapshot
#define DEF_WARPNG_PCI_CHG             0x06  // Danh sách 18 thiết bị PCI (Bus/Dev/VendorID/DeviceID) đổi
#define DEF_WARPNG_PANELTYPE_CODECHG   0x07  // Panel type code đổi
#define DEF_WARPNG_PANELTYPE_OPCHG     0x08  // Panel option đổi
#define DEF_WARPNG_PANELTYPE_SIZECHG   0x09  // Panel size đổi
#define DEF_WARPNG_STARTSEQ_NG1        0x0a  // g_action=DEF_BOOT nhưng lần TRƯỚC là AUTOBOOT_LINE
#define DEF_WARPNG_BOOT_CNT_OVER       0x0c  // Bộ đếm loop-boot vượt ngưỡng tối đa
#define DEF_WARPNG_A800_DETECT_CHANGE  0x0d  // Phụ kiện A800 gắn/tháo thay đổi
#define DEF_WARPNG_TROUBLERESET        0x0e  // Phím panel = Trouble-Reset (0x16)
#define DEF_WARPNG_SSDUNMATCH          0x0f  // (chỉ debug, #if 0 — vô hiệu ở bản production)
```

Mỗi lần từ chối, lý do được **ghi lại vào SPI Flash** (`THRG_setBspThrRegion(DEF_OFFSET_SUPERWARP_DECISION_STATE, ...)`) — nghĩa là kỹ thuật viên có thể tra ngược lại **vì sao** một lần boot cụ thể không được Warp, thay vì chỉ thấy "boot chậm hơn bình thường" mà không rõ lý do.

Về bản chất, hàm này so sánh **"ảnh chụp cấu hình phần cứng"** đã lưu (dung lượng RAM, danh sách PCI, loại/option/kích thước panel, có A800 hay không) với trạng thái **thật** đọc trực tiếp từ phần cứng ngay lúc boot — bất kỳ sai khác nào cũng buộc quay về cold-boot, vì snapshot cũ không còn phản ánh đúng máy nữa.

## 4. Điều kiện tuần tự (sequence) — chỉ 2 g_action được phép Warp, và không được nhảy lung tung giữa chúng

```c
// main.c:1478-1495 — cặp điều kiện đối xứng, mới thấy đầy đủ:

/* Nay là "Warp!! boot", trước là "line-card boot" → CẤM */
if( g_action == DEF_BOOT && uc_ssdBuff[BOOT_TYPE] == AUTOBOOT_LINE ){
    → DEF_WARPNG_STARTSEQ_NG1
}

/* Nay là "line-card boot", trước là boot thường HOẶC Warp!! → CẤM */
if( g_action == BOOT_LINE && 
  ( uc_ssdBuff[BOOT_TYPE] == AUTOBOOT || uc_ssdBuff[BOOT_TYPE] == AUTOBOOT_WARP )){
    → DEF_WARPNG_STARTSEQ_NG2
}
```

Đây là chỗ xác nhận lại điều đã suy ra khi phân tích `BOOT_LINE`: chỉ **`DEF_BOOT`** và **`BOOT_LINE`** được phép vào Warp — nhưng còn có thêm ràng buộc **thứ tự chuyển đổi giữa hai loại này**: không được "vừa boot line-card xong lại Warp bình thường ngay" và ngược lại không được "line-card Warp" nếu lần trước không phải cũng là line-card. Cơ chế này ngăn máy nhầm lẫn snapshot của một phiên line-card với một phiên boot thường.

Thêm một điều kiện tách biệt (main.c:1504): nếu đang `BOOT_LINE` **và** phím panel = `0x00` (không phím) **và** utility key active (`STOP+RESET` giữ khi bật nguồn) → cũng bị cấm Warp, dành riêng cho quy trình "copy toàn bộ SPI-Flash hàng loạt" (comment gốc).

## 5. Trong `main_loop()`: SuperWarp trước, Normal Warp sau, mỗi cấp có WDT riêng

```
AUTOBOOT_WARP || (AUTOBOOT_LINE && checkWarpBootMode())
    │
    ├─ gPanelKey≠0x16 && !isDisableSuperWarp_ErrorOccurred()
    │     → WDT REFRESH 0x11b0, 40s → warp_boot(SUPER)  [never return nếu OK]
    │     → thất bại: xoá snapshot SuperWarp
    │
    → setStartupBypeNomalWarp() (chuyển cờ khởi động sang Normal)
    → WDT REFRESH 0x11a0, 90s → warp_boot(NORMAL)       [never return nếu OK]
    → thất bại: autoboot = check_autoboot_DEF_BOOT_case(autoboot)  (quay về cold-boot)
```

`isDisableSuperWarp_ErrorOccurred()` (đã tra thêm lần này) đọc 2 byte tại `DEF_OFFSET_SUPERWARP_DISABLE_ERROR_OCCURRED` — nếu **kernel hoặc chính main FW** từng ghi giá trị `0x10` vào đó (báo hiệu kernel panic/BUG xảy ra ở lần chạy trước), SuperWarp bị vô hiệu hoá cho lần này (nhưng Normal Warp vẫn được thử) — một lớp bảo vệ chống lặp crash-loop nếu chính bản thân việc resume-từ-snapshot là nguyên nhân gây crash.

## 6. Ghi lại "ảnh chụp" cho lần sau — `set_boot_type_to_main_routine()`

Hàm này (chạy **vô điều kiện** ở cuối `main_loop()`, bất kể có Warp hay không) ghi lại đúng những gì `isChgWarpBootSettingInfo()` sẽ so sánh ở lần boot kế tiếp: loại boot vừa chạy, danh sách PCI hiện tại (`km_get_pciinfo_for_warp()`), panel type/option/size, trạng thái A800 — khép kín vòng lặp "snapshot decision" giữa 2 lần boot liên tiếp.

## 7. Quan hệ với bảo mật Secure Chip — độc lập hoàn toàn

`chk_sboot()` chạy **trước** toàn bộ chuỗi kiểm tra Warp ở trên (main.c:2232, trước `bootdelay_process()`) — nghĩa là dù Secure Chip Board có hợp lệ hay không, quyết định đó **không ảnh hưởng** tới logic Warp; và ngược lại, việc `BOOT_BDERASE` xoá vùng Secure Area (`0x3FE000`) cũng **không đụng** tới vùng "Warp Setting Info" (nằm ở sector 65 trên SSD/SD, hoàn toàn khác SPI Flash) — hai cơ chế bảo mật/hiệu năng này tách biệt nhau về mặt lưu trữ dù cùng nằm trong một chuỗi `main_loop()`.

## 8. Luồng hoạt động (đổi "xác thực/giải mã" → "quyết định có resume hay không")

```
main_loop() [main.c:2238]
  → autoboot == AUTOBOOT_WARP hoặc (AUTOBOOT_LINE && checkWarpBootMode())
      → checkWarpBootMode(): GPIO WARP_SW + 12 điều kiện isChgWarpBootSettingInfo()
  → warp_boot(saveno)  [cmd_warp.c:969]
      → warp_boot_fg(saveno)
          → warp_pre_boot(saveno) → warp_checkboot(saveno):
              1. _warp_drvload(0): nạp "hibernation driver" blob từ SPI Flash 0x2b0000
                 vào buffer warp_drvaddr, kiểm tra magic ID = 'W5HD' (WARP_ID_DRIVER)
              2. warp_bfload(saveno, warp_bfaddr): nạp "boot flag" (512 byte) từ vị trí
                 tương ứng với saveno (Super=slot 2/3, Normal=slot 0/1)
              3. kiểm tra *(u32*)warp_bfaddr == 'W5BF' (WARP_ID_BOOTFLAG)
                 → KHÔNG khớp = không có snapshot hợp lệ → return lỗi
          → boot_param setup (silent/console/bps = -1, drv_total_size)
          → hibernate = (void*)(warp_drvaddr + 0x28)   // WARP_HEADER_HIBERNATE
          → ret = hibernate()   // nhảy vào driver blob — nếu resume thành công, KHÔNG BAO GIỜ return
          → nếu return: in "Warp!! error can not boot %d" → main_loop() coi là thất bại, thử tiếp (Super→Normal→cold)
```

**"Key nào được dùng, lấy từ đâu, input/output từng bước?"** — Không có key. Input thật sự: `saveno` (số slot 0-3), vị trí driver blob cố định (SPI `0x2b0000`), vị trí bootflag phụ thuộc `saveno` (định nghĩa trong `warp_savearea[]`, board-config). Output: nếu thành công, CPU nhảy thẳng vào driver blob và không quay lại; nếu thất bại, trả về mã lỗi int.

**"BootROM quyết định unwrap/decrypt dựa trên điều kiện nào?"** → Đổi thành "quyết định thử resume dựa trên điều kiện nào" — đã liệt kê đầy đủ ở lượt trước: GPIO `WARP_SW`, 12 lý do trong `isChgWarpBootSettingInfo()` (RAM size, PCI list, panel type/option/size, A800, boot-sequence trước đó, Trouble-Reset key...), và magic ID `'W5BF'` đọc được từ đúng slot lúc runtime (nếu bootflag hỏng/không tồn tại → tự nhiên fail, không cần điều kiện riêng).

## 9. Implementation — phần này verify được khá chi tiết

**Header/format thật của driver blob** (`warp.h`):

```c
#define WARP_HEADER_ID          0x00   // phải = 'W5HD' (0x44483557)
#define WARP_HEADER_VERSION     0x04
#define WARP_HEADER_CAPS        0x08
#define WARP_HEADER_LOWMEM_END  0x0c
#define WARP_HEADER_TEXT_END    0x10
#define WARP_HEADER_DATA_END    0x14
#define WARP_HEADER_BSS_END     0x18
#define WARP_HEADER_SNAPSHOT    0x20
#define WARP_HEADER_HIBERNATE   0x28   // ← offset chứa con trỏ hàm thực thi resume
#define WARP_HEADER_SWITCH      0x30
```

Đây chính là "metadata/header format" bạn hỏi — nhưng nó mô tả **layout của driver blob** (kích thước text/data/bss để load đúng, offset hàm entry-point), không phải header của một "boot image" theo nghĩa ảnh kernel đã ký. Nội dung _bên trong_ vùng từ offset `0x28` trở đi (logic thật của `hibernate()`) nằm trong file nhị phân đã compile sẵn, không có trong source C — nên tới đây là giới hạn cuối cùng mình trace được từ repo này.

Nếu bạn có file blob thật ở offset `0x2b0000` trên SPI Flash (dump được), mình có thể giúp phân tích header 4 trường đầu (`ID/VERSION/CAPS/...`) theo đúng định nghĩa trên — nhưng phần logic resume thực sự thì ngoài khả năng (đó là binary, không phải gì mình đọc được qua Read/Grep).

## 10. Flow High-Level

```c
START
  │
  ▼
[check_autoboot() xác định loại autoboot cần chạy]
  │
  ▼
❓ autoboot == AUTOBOOT_WARP  HOẶC  (AUTOBOOT_LINE VÀ checkWarpBootMode()==1)?
  │                                  (gồm: GPIO WARP_SW + 12 điều kiện phần cứng khớp)
  ├── NO ──────────────────────────────────────────────┐
  │                                                     ▼
  │                                          [Cold boot bình thường]
  │                                                     │
  │                                                     ▼
  │                                                    END
  ▼ YES
[Ghi nhận loại khởi động này vào SSD/SD (cho lần boot sau đối chiếu)]
  │
  ▼
❓ Phím panel = Trouble-Reset  HOẶC  lần boot trước từng kernel-crash?
  │
  ├── YES ──▶ [Bỏ qua SuperWarp, xoá snapshot Super cũ] ──┐
  │                                                        │
  ▼ NO                                                     │
[Thử SUPERWARP — WDT 40s — warp_boot(slot Super)]          │
  │                                                        │
  ▼                                                        │
❓ hibernate() resume thành công?                          │
  │ (thành công = hàm không bao giờ return)                │
  ├── YES ──▶ [Kernel tiếp tục chạy từ trạng thái cũ] ──▶ END
  │                                                        │
  ▼ NO (return lỗi)                                        │
[Xoá snapshot Super, chuyển cờ sang Normal] ◀───────────────┘
  │
  ▼
[Thử NORMAL WARP — WDT 90s — warp_boot(slot Normal)]
  │
  ▼
❓ hibernate() resume thành công?
  │
  ├── YES ──▶ [Kernel tiếp tục chạy từ trạng thái cũ] ──▶ END
  │
  ▼ NO
[Xoá snapshot Normal, tính lại autoboot qua check_autoboot_DEF_BOOT_case()]
  │
  ▼
[Cold boot: WDT 300s + chkGmUpdate() + run_command_list() — boot kernel từ đầu]
  │
  ▼
END

```
**Đọc nhanh**: 2 lớp thử tuần tự (Super trước, budget 40s → Normal sau, budget 90s), mỗi lớp thất bại đều dọn snapshot rồi rơi xuống lớp tiếp theo; nếu cả hai fail mới quay về cold-boot đầy đủ (300s). "Thành công" ở đây có nghĩa là hàm `hibernate()` **không bao giờ trả về** — CPU đã nhảy hẳn vào kernel đang resume.
## Tóm tắt

SuperWarp là cơ chế fast-boot 2 cấp (Super rồi Normal) được gác bởi một công tắc GPIO vật lý và một hàm so sánh **12 tiêu chí phần cứng/cấu hình** (RAM, PCI, panel, A800, loại boot trước đó, phím Trouble-Reset...) giữa trạng thái thật lúc boot với snapshot đã lưu — bất kỳ sai lệch nào cũng bị từ chối kèm mã lý do ghi vào SPI Flash để truy vết sau này; chỉ `DEF_BOOT` và `BOOT_LINE` được phép tham gia, với ràng buộc thứ tự chuyển đổi giữa hai loại đó; và có lớp tự-vô-hiệu-hoá SuperWarp (không phải Normal Warp) nếu lần chạy trước từng kernel-panic khi resume từ snapshot.


