# Trình bày BOOT_SPEEDCHANGE

`BOOT_SPEEDCHANGE` (giá trị `8`) là kịch bản **cấu hình lại "cấp tốc độ" vận hành của máy** ("速度可変モード" — chế độ tốc độ khả biến, theo comment CQ:372758) — bổ sung sau các g_action gốc (2012/10/01), dùng chung ảnh firmware production nhưng đi qua một quy trình khởi động riêng để áp dụng thay đổi.

## 1. Nguồn gốc — 2 đường kích hoạt song song

```c
static char *UseClassTbl[] = { "@ZZZ","@DWN","@XYZ","@WVU","@SPD","@BDE", ... };
#define USB_SPD   5    // "@SPD" nằm ở vị trí index 4 trong bảng → USB_SPD = 4+1 = 5
```

1. **Qua USB**: file `INDEX` bắt đầu đúng 4 ký tự `@SPD` → `sysBspGetUsbMemType()` trả `USB_SPD` → `check_action()` switch tại main.c:789-793 set `g_action = BOOT_SPEEDCHANGE`.
2. **Qua DipSW**: offset `0x06 == 8` — đọc bởi cả `setup_MotionEnable_Power()` (nhánh không-USB, `uc_IISWFlg==8` nhóm chung với `DEF_UPDATE`... nhưng đợi — thực ra giá trị `8` này đã gán `DEF_UPDATE` chứ không phải `BOOT_SPEEDCHANGE` trong nhánh DipSW! Cần đối chiếu lại: theo bảng action.h, `4/7/8/9 → DEF_UPDATE`. Vậy **`BOOT_SPEEDCHANGE` trên thực tế chỉ có một đường kích hoạt duy nhất: qua USB với `@SPD`**, không có con đường DipSW trực tiếp — mình xin đính chính lại so với bảng tổng hợp trước đó, giá trị "DipSW=8" ở đó là nhầm lẫn với `DEF_UPDATE`.

**Điểm đặc quyền duy nhất của `USB_SPD`**: đây là loại USB **duy nhất được miễn kiểm tra tên máy** ở `check_action()`:

```c
if( g_UsbKind != USB_SPD && sysBspCmpTypeName(&cindex[6],TYP_NAME) !=0 ) { g_action = BOOT_ERR; ... }
```

→ một thẻ USB đổi tốc độ có thể dùng chung cho **nhiều dòng máy khác nhau**, không bị chặn bởi cơ chế xác thực tên máy như mọi loại USB khác.

## 2. `check_autoboot()` — dùng chung ảnh production với DEF_BOOT

```c
case BOOT_SPEEDCHANGE:
    setenv("bootcmd",  make_bootcmds(CONFIG_CMD_ENV));       // partition 0:5 — GIỐNG DEF_BOOT
    setenv("bootargs", make_bootargs(g_action,CONFIG_SSD_RFS_ENV));
    autoboot = AUTOBOOT_SPEEDCHANGE;
```

```
boottyp=8 root=/dev/kmsda7 rootdelay=1 [nopanel] console=...
TerminalState=<...> MachineInfo=<hex> MachineNo=<hex> SerialNo=<...>
IISW=0 FwDir=FW9020 DevStr=nvme IntegrityState=<0|1> <CONFIG_BOOTARGS>
```

Không có tham số bootargs đặc biệt nào riêng cho `BOOT_SPEEDCHANGE` (khác `BOOT_LINE` với `TERM_STATE_1` ép cứng, hay `BOOT_HW_CHECKMODE` với `SubsetChk=1`) — chỉ khác `boottyp=8`. Toàn bộ việc "đổi tốc độ" thật sự diễn ra ở tầng userspace sau khi đọc `boottyp`, nằm ngoài phạm vi BootROM.

**Manh mối duy nhất về "tốc độ" là gì**: trong `make_bootargs()` có một nhánh riêng cho các dòng máy Sparrow (`CONFIG_KM_MACHINE_SPAM/SPAAS/SPABKM/SPABKAS`) thêm trường `Capacity=1` (comment: "PC遅延防止用スケーラビリティ対応 A3=[Capacity=0] A4=[Capacity=1]" — hỗ trợ khả năng mở rộng chống trễ PC, A3=Capacity 0, A4=Capacity 1). Điều này gợi ý mạnh: "tốc độ" ở đây liên quan tới **cấp độ hiệu năng in/scan** (ví dụ giới hạn tốc độ theo model thương mại A3 vs A4) chứ không phải CPU clock — nhưng mình không có bằng chứng trực tiếp nối `BOOT_SPEEDCHANGE` với trường `Capacity` này, nên xin nêu như một suy luận có căn cứ chứ không khẳng định chắc chắn.

## 3. Motion Enable & Watchdog

- **Motion Enable**: `BOOT_SPEEDCHANGE` nằm trong danh sách loại trừ (`machine_setup.c:277`) → luôn **tắt**, không có else-if nào bật lại.
- **WDT**: nằm trong nhóm 6 g_action **tắt-WDT-sớm** (`machine_setup.c:337`, cùng nhóm `DEF_BACKUP/DEF_RESTORE/BOOT_HW_CHECKMODE/BOOT_ERR/BOOT_LINE`); `autoboot_command()` cũng không có case riêng → `else` → `WDT_CA72_END`.
- **Warp**: `autoboot=AUTOBOOT_SPEEDCHANGE` không khớp điều kiện Warp → luôn cold-boot.

## 4. Tóm tắt

`BOOT_SPEEDCHANGE` chỉ có **một** đường kích hoạt thực sự — USB với `INDEX` bắt đầu `@SPD`, loại USB duy nhất được miễn xác thực tên máy nên dùng được xuyên suốt nhiều dòng máy — boot đúng ảnh firmware production (partition `0:5`, giống `DEF_BOOT`) với `boottyp=8` là khác biệt duy nhất, tắt cơ khí và watchdog trong lúc chờ userspace tự áp dụng cấu hình tốc độ mới.