# 1. 0x0045

```c
ret = THRG_getBspThrRegion( 0x0045, &uc_subset_wd, sizeof(uc_subset_wd) );
```

Vùng `0x0045` do chính **Subset FW** ghi, nằm trong [IT6_Kernel/Subset/S-S800/Src/application/mfp/Linux/main.cpp:1128-1212](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Kernel/Subset/S-S800/Src/application/mfp/Linux/main.cpp#L1128-L1212). Đây là cơ chế Subset watchdog (OP_BTS-37780), không phải LPPP ghi. Tôi chưa grep các thư mục ngoài `Subset` để loại trừ nơi ghi khác.

## Ý nghĩa của giá trị

Byte này gồm 2 phần:

- **bit 4 (`0x10`)**: cờ "Subset đã khởi động mà chưa chạy đến điểm kết thúc watchdog".
- **4 bit thấp (`& 0xf`)**: bộ đếm số lần thử lại, tối đa 3, và `0xf` nghĩa là retry over.

## Chỗ ghi (3 nơi)

|Vị trí|Giá trị được ghi|Khi nào|
|---|---|---|
|`ca72wdt_start()`, [dòng 1170](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Kernel/Subset/S-S800/Src/application/mfp/Linux/main.cpp#L1170)|`uc_subset_wd \|= 0x10`|Ngay đầu `main()` của Subset (dòng 1225), trước cả `init_section`. Cờ được bật ngay mỗi lần Subset khởi động.|
|`ca72wdt_start()`, [dòng 1157](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Kernel/Subset/S-S800/Src/application/mfp/Linux/main.cpp#L1157)|`0x0f`|Khi lần boot này phát hiện WDT đã expire (vùng `0x2a == 0x10`), cờ `0x10` đang bật và đếm đã ≥ 3. Sau đó code xóa cờ download, `THRG_SetNWDLExec(0)` và reboot để thoát khỏi vòng lặp Subset.|
|`ca72wdt_end()`, [dòng 1210](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Kernel/Subset/S-S800/Src/application/mfp/Linux/main.cpp#L1210)|`& 0x0f`|Gọi từ [SYSC_SystemInit.cpp:532](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Kernel/Subset/S-S800/Src/application/mfp/system/sys/sysg/SYSC_SystemInit.cpp#L532): xóa bit `0x10`, giữ lại bộ đếm. Lệnh này cũng gửi END cho WDT CA72.|

Bộ đếm tăng tại dòng 1166 (`uc_subset_wd++`) khi phát hiện boot lần này có WDT expire và cờ `0x10` vẫn còn, tức lần trước chưa tới `ca72wdt_end()`.

## Luồng

```
Subset main()
 └─ ca72wdt_start()
     ├─ đọc 0x002a  → bl_isWdExpire = (== 0x10)   ← cờ do LPPP ghi
     ├─ đọc 0x0045
     │    ├─ ≥ 0x20 → reset về 0 (giá trị rác/erased)
     │    ├─ (có 0x10 && bl_isWdExpire):
     │    │     ├─ count ≥ 3 → ghi 0x0f, reboot ("retry over")
     │    │     └─ else count++
     │    └─ |= 0x10, ghi lại
     └─ ioctl PMU_IOCSSET_WDT_CA72 START → kernel → SCPI 0x8B → LPPP CA72WDT_Start (90 giây)

... SYSC_SystemInit → ca72wdt_end()
     ├─ ioctl END → LPPP CA72WDT_End
     └─ 0x0045 &= 0x0f   (xóa cờ 0x10)
```

START lúc này theo [app_scpi.c:975](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Kernel/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L975), gọi `CA72WDT_Start()` với timeout mặc định 90 giây, không có point (`ca72wdte_refreshPoint` giữ giá trị cũ).

## Liên hệ với `main_BSP.cpp` của MFP

Phía MFP ([main_BSP.cpp:3557-3573](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Controller/KM3/application/mfp/Linux/main_BSP.cpp#L3557-L3573)) đọc vùng này theo hướng ngược lại:

- Nếu `uc_subset_wd != 0` thì ghi lại 0 (OP_BTS-41198).
- Nếu `>= 0x20` thì coi là 0.
- Nếu `0x2a == 0x10` và `uc_subset_wd == 0` thì đây là WDT expire của MFP thường, in log `CA72WDT NewDetailCode`.
- Nếu `uc_subset_wd != 0` thì nhánh `else` ([dòng 3602](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Controller/KM3/application/mfp/Linux/main_BSP.cpp#L3602)) chạy. Detail code `0x22 02a` chỉ được ghi khi `== 0xf`.

Lưu ý: `uc_subset_wd` chỉ bằng 0 sau khi `ca72wdt_end()` đã chạy và bộ đếm = 0, vì vậy log "CA72WDT NewDetailCode" mà bạn điều tra trước đó chỉ xuất hiện khi lần boot đó không do Subset WDT gây ra.

Tôi chưa kiểm tra liệu `clearDownloadingFlag()` / bộ đếm có được reset ở chỗ khác khi Subset chạy xong update hay không. Nếu bạn muốn, tôi sẽ tìm tiếp chuỗi "Subset update → reboot" để xem khi nào vòng retry này kích hoạt.


# 2. 0x002a

```c
ret = THRG_getBspThrRegion( 0x002a, &uc_code, sizeof(uc_code) );
```

Cờ 0x10 ở vùng `0x2a` được LPPP ghi khi timer CA72WDT trên LPPP hết hạn mà không ai gọi refresh hoặc end kịp. Phần dưới là cơ chế và các tình huống làm timer hết hạn.

## 1. Nơi ghi 0x10

Chỉ có một nơi ghi, theo chuỗi `ClearWatchDogCounter` → `CA72WDT_Expire` → `LPPP_SpiFlashWrite_Ca72wdt_Reboot`:

1. Task `ClearWatchDogCounter` ([LPPP_Task.c:832-903](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Kernel/lppp_s800/ApplicationKM/LPPP_Task.c#L832-L903)) chạy mỗi 5 giây. Khi `ca72wdt_isEnable == true` và `ca72wdt_timer >= ca72wdt_timeout` thì nó in `############# CA72WDT expire` và gọi `CA72WDT_Expire()`.
2. `CA72WDT_Expire()` ([LPPP_Task.c:694-706](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Kernel/lppp_s800/ApplicationKM/LPPP_Task.c#L694-L706)) làm theo thứ tự này:
    - gọi `CA72WDT_End()`;
    - gọi `LPPP_SpiFlashWrite_Ca72wdt_Reboot()`, hàm ghi cờ vào SPI-Flash;
    - kéo `HRESET_REQN = 0` để reset cả board. Riêng khi cờ debug `CA72_WDT_FLAG_DISABLE_REBOOT` bật (bit0 của `debugInfo1` lúc START), bước này bị bỏ qua. Lúc đó SPI-Flash vẫn bị ghi 0x10 nhưng board không reset.
3. `LPPP_SpiFlashWrite_Ca72wdt_Reboot()` ([LPPP_Matrix.c:443-476](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Kernel/lppp_s800/ApplicationKM/LPPP_Matrix.c#L443-L476)) ghi vào cả hai vùng BspThrRegion1 và BspThrRegion2:

|MFP đọc (region id)|Offset LPPP|Nội dung|
|---|---|---|
|`0x002a`|`0x2e`|`0x10` (1 byte)|
|`0x0030`|`0x34`|`(debugInfo1<<16) \| refreshPoint` (4 byte), là `ul_ca72code`|
|`0x003A`|`0x3E`|`(PowerStatus<<8) \| uc_pmu_phase_rec` (2 byte), là `pmu_phase_rec`|

Phép ánh xạ region id sang offset là +4 (`0x2a`→`0x2e`, `0x30`→`0x34`, `0x3A`→`0x3E`). Tôi suy ra từ ba cặp này, chưa kiểm tra hàm `THRG_getBspThrRegion` để xác nhận.

## 2. Khi nào timer hết hạn

Timer reset về 0 (tức được refresh) khi nhận lệnh SCPI `0x8C` (`REBOOTEXT`) với event `START` hoặc `REFRESH`, hoặc lệnh `0x8B` (`CA72WDT_Refresh`). Timer bị tắt bởi event `END`. Nếu timer đang chạy mà đến hạn thì ghi cờ. Hạn là `timeout/5` chu kỳ 5 giây, nên lỗi thực tế xảy ra trong khoảng N×5 giây đến (N+1)×5 giây tính từ lần refresh cuối.

Các nguồn START, REFRESH, END theo từng giai đoạn:

|Giai đoạn|Ai gọi|Point (khớp `ul_ca72code & 0xffff`)|Timeout|
|---|---|---|---|
|LPPP vừa khởi động (nếu `isFunctionVersion2_1`)|`ClearWatchDogCounter` tự START|`0x8000`|5 giây|
|U-Boot `board_r.c`|START rồi REFRESH|`0x1000`, rồi `0x1000\|count`|5 giây|
|U-Boot `machine_setup.c`|REFRESH|`0x1120` (120 giây), `0x1121` (5 giây), `0x1130` (10 giây)||
|U-Boot `main.c`|REFRESH|`0x11a0` (Warp boot, 90 giây), `0x11b0` (SuperWarp, 40 giây)||
|U-Boot `autoboot.c`|REFRESH|`0x11D0`, `0x11D1`, `0x11D2` (Normal Boot, 300 giây), `0x11E0` đến `0x11E2` (update, 120 giây)||
|Kernel `pcie-mv-msb0125.c` (bật IRIS1)|REFRESH|`0x410B` (90 giây), `0x410C`|90 giây|
|Kernel `mdels800_pmu.c` (PCIe link, suspend/resume)|REFRESH|`0x4104`, `0x4106`|0 giây|
|MFP app (xem trước đó)|REFRESH|`0x3002`, `0x4xxx`, `0x7xxx`||

Mã U-Boot, Kernel, MFP đều gửi `0x8C`. Sau khi LPPP reset board, ba trường `ul_ca72code`, `pmu_phase_rec` và byte `0x10` mới xuất hiện trong log MFP.

Các trường hợp hết hạn thường gặp:

1. Một giai đoạn kẹt hơn timeout đã hẹn. Ví dụ U-Boot hẹn 5 giây rồi hang, kernel hẹn 90 giây nhưng PCIe link up không được, MFP hẹn 90 giây ở `SystemInit2` nhưng bị treo.
2. START được gọi nhưng không ai gọi END hoặc REFRESH tiếp theo. Tham chiếu: lệnh SCPI END bị bỏ qua khi `PWR_STATUS_SLEEP1` ([app_scpi.c:982](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Kernel/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L982)).
3. Chuỗi suspend/resume: `CA72WDT_Start` ở dòng 288 của `app_scpi.c` khi bật lại, `CA72WDT_End` ở dòng 278 khi `ALL_CPU_OFF`. Nếu resume lỗi, `pmu_phase` ghi vào `uc_pmu_phase_rec` cho biết dừng ở bước nào.
4. START lần hai khi timer đang chạy sẽ đè `timeout` và `refreshPoint` mới (xem OP_BTS-44676, [LPPP_Task.c:769-778](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/IT6_Kernel/lppp_s800/ApplicationKM/LPPP_Task.c#L769-L778)).

## 3. Cách suy ngược một lần log cụ thể

Từ `NewDetailCode` trong log:

- `& 0xffff` là `refreshPoint` cuối cùng. Tra bảng trên để biết giai đoạn bị kẹt. Giá trị `0x8000` cho biết LPPP khởi động rồi không ai refresh, nhánh này MFP cố ý không ghi vào detail code.
- `>> 32 & 0xfff` là `pmu_phase_rec`, byte thấp là phase PMU, byte cao là `LPPP_GetStatus(pwr_sd)` (trạng thái nguồn, 12 bit đầu nằm trong detail code).
- bit 31 là `uc_CPUOccup` (từ vùng `0x3F`), không do LPPP ghi trong đường này.
- bit 16 trở lên của `ul_ca72code` là `debugInfo1`, bit0 = 1 nghĩa là chế độ debug (không reset board).

Hai điểm cần lưu ý:

- Không có chỗ nào khác trong LPPP ghi `0x10` vào vùng này, nhưng tôi chưa grep hết `flash_modify_sub` trên `BspThrRegion` để loại trừ ghi trực tiếp từ nơi khác, và chưa xác minh ánh xạ id→offset ở phía MFP.
- Cờ chỉ bị xóa bởi MFP (dòng 3617 của `main_BSP.cpp`). Nếu board hết hạn rồi reset mà MFP chưa chạy tới đoạn đọc này (kẹt sớm hơn), cờ vẫn còn và log sẽ xuất hiện ở lần boot sau.

Nếu bạn có log LPPP của lần lỗi (dòng `## wd_start`, `## wd_refresh p:0x....`, `CA72WDT timer ...`, `CA72WDT expire`), tôi đối chiếu giúp để chỉ ra chính xác đoạn nào kẹt.