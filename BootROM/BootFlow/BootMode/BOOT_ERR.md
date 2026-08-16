# Trình bày BOOT_ERR

`BOOT_ERR` (giá trị `5`) là trạng thái "dữ liệu/thiết bị không hợp lệ" — điểm đặc biệt của g_action này là nó **không có một nguồn gốc duy nhất** như các g_action khác, mà là "van an toàn" được gán từ **4 vị trí độc lập** trong toàn bộ pipeline, mỗi vị trí bắt một loại lỗi khác nhau.

## 1. Bốn nguồn gốc của BOOT_ERR

| #   | Vị trí                                                                   | Điều kiện kích hoạt                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| --- | ------------------------------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 1   | `check_action()` main.c:756-761                                          | Đọc `INDEX` từ USB thành công, nhưng **tên máy không khớp** (`sysBspCmpTypeName` fail) — trừ trường hợp `USB_SPD` được miễn.                                                                                                                                                                                                                                                                                                                                                                                                                    |
| 2   | `check_action()` main.c:794-803                                          | `sysBspGetUsbMemType()` trả `USB_INVALD` hoặc loại không nằm trong switch (`default`) — USB có dữ liệu nhưng không nhận diện được loại thao tác.                                                                                                                                                                                                                                                                                                                                                                                                |
| 3   | `setup_MotionEnable_Power()` machine_setup.c:232-236                     | **Không có USB**: cờ `uc_MSCDLFlg==1` (đọc từ SPI Flash `0x07B000+DEF_USERDATA_OFFSET`) — firmware chính (MSC) trên SSD **đang dở dang**. Ưu tiên xét **cao hơn** cả backup/restore/HW-checkmode trong chuỗi if/else.                                                                                                                                                                                                                                                                                                                           |
| 4   | `check_autoboot()` main.c:1888-1925 — **mới phát hiện, đáng chú ý nhất** | Chỉ chạy khi vào hàm với `g_action==DEF_UPDATE` sẵn từ trước: đọc lại file `INDEX` — nhưng lần này **không phải từ USB**, mà từ chính **partition `0:A` trên SSD/SD nội bộ** (`device_str="nvme"/"mmc"`, cùng partition với `CONFIG_IISWCMD_UPDATE_ENV`). Đây là bước **kiểm tra sự toàn vẹn** của gói update đã tải sẵn trước khi thực sự boot vào nó: nếu `INDEX` thiếu, sai tên máy, hoặc `g_UsbKind` không phải `USB_DOWN` → **`g_action` bị ghi đè từ `DEF_UPDATE` thành `BOOT_ERR`** ngay trước khi rơi vào `switch(g_action)` phía dưới. |

Nguồn #4 cho thấy `BOOT_ERR` không chỉ là lỗi "USB cắm sai" mà còn là cơ chế **validate-before-commit**: dù DipSW đã báo "có update đang chờ" (`DEF_UPDATE`), BootROM vẫn tự kiểm tra lại gói tin đã lưu trước khi thực sự thực thi, tránh boot vào một bản update hỏng/dở dang.

## 2. Nội dung `bootcmd` (giải mã `CONFIG_ERRCMD_ENV`)

```c
#define CONFIG_ERRCMD_ENV   CMD_INITRD_ENV("%s","%s","0:2","")   // trùng partition với CONFIG_IISWCMD_ENV
```

```
nvme init;                                    # (hoặc "mmc rescan;")
fatload nvme 0:2 $kernel_addr $bootfile;
fatload nvme 0:2 $fdt_addr    $dtb_name;
fatload nvme 0:2 $initrd_addr $initrd_name;
booti $kernel_addr $initrd_addr $fdt_addr
```

Dùng chung **partition `0:2`** với `DEF_IISW`/`DEF_BACKUP`/`DEF_RESTORE` — củng cố thêm giả thuyết đã nêu ở bảng tổng hợp trước: đây là một ảnh **"Subset OS"** đa năng, tự rẽ nhánh hành vi hoàn toàn dựa vào `bootargs`.

## 3. Nội dung `bootargs`

```c
make_bootargs(BOOT_ERR, CONFIG_SSD_RFS_ENV)   // root=/dev/kmsda7 rootdelay=1 — giống DEF_BOOT
```

```
boottyp=5 root=/dev/kmsda7 rootdelay=1 [nopanel] console=<ttyS5|ttyS0|tty1>
TerminalState=<...> MachineInfo=<hex> MachineNo=<hex> SerialNo=<...>
IISW=0 FwDir=FW9020 DevStr=nvme IntegrityState=<0|1> <CONFIG_BOOTARGS>
```

Chỉ khác `DEF_IISW` ở `boottyp=5` (thay vì `3`) và `IISW=0` (thay vì `1`) — dù cả hai boot **cùng file nhị phân** trên `0:2`. Nội dung màn hình lỗi thực tế hiển thị gì (phân biệt được lỗi #1/#2/#3/#4 ở trên hay chỉ một thông báo chung) là phần userspace/kernel đọc `boottyp`/`IISW` để quyết định — nằm ngoài phạm vi BootROM, mình chưa thấy code nào ở đây định nghĩa nội dung màn hình đó.

## 4. Motion Enable & Watchdog

- **Motion Enable**: nằm trong danh sách loại trừ (`setup_MotionEnable_Power` dòng 274-277) → **luôn tắt**, không có else-if nào bật lại — hợp lý vì đây là trạng thái lỗi, không nên vận hành cơ khí.
- **WDT**: tắt sớm ở `setup_MotionEnable_Power()` (thuộc nhóm 6 g_action tắt-WDT-sớm), và `autoboot_command()` cũng không có case riêng cho `BOOT_ERR` → rơi `else` → `WDT_CA72_END`. **Không có cơ chế tự phục hồi** — máy đứng ở màn hình lỗi cho tới khi con người can thiệp.
- **Warp**: `autoboot = AUTOBOOT_SSD_SUBSET` → không khớp điều kiện Warp, luôn cold-boot.

## 5. Tóm tắt

`BOOT_ERR` là "van an toàn" dùng chung của cả pipeline USB lẫn pipeline DipSW-update: bắt lỗi sai tên máy, USB không hợp lệ, MSC firmware dở dang, hoặc — tinh vi nhất — gói update đã tải sẵn bị hỏng khi kiểm tra lại ngay trước lúc boot; luôn boot vào cùng ảnh Subset OS như `DEF_IISW` (partition `0:2`) nhưng tắt hẳn cơ khí và watchdog, chờ con người xử lý chứ không tự động thử lại.