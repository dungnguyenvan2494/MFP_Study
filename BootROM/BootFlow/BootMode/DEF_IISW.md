# Trình bày DEF_IISW

`DEF_IISW` (giá trị `3`) là kịch bản **đang trong một tiến trình tải/cài đặt firmware qua mạng (network download)** — không liên quan gì tới USB, hoàn toàn kích hoạt bởi một cờ DipSW ảo lưu trong SPI Flash. Tên hàm đọc cờ này — `getNWDLExec()` (**Get N**et**W**ork-**D**own**L**oad-**Exec**uting) — là manh mối rõ nhất cho ý nghĩa: đây là trạng thái "hệ thống đang thực hiện tải xuống qua mạng".

## 1. Vòng đời DEF_IISW qua từng trạm

|Trạm|Điều gì xảy ra riêng cho DEF_IISW|
|---|---|
|**setup_MotionEnable_Power()**|Nhánh không-USB: `uc_IISWFlg == 5` hoặc `6` (đọc từ DipSW offset `0x06`) → `g_action = DEF_IISW`, in log `"IISW boot.\n"` (machine_setup.c:222-226). Nằm trong **danh sách loại trừ** Motion-Enable (dòng 274) → cơ cấu cơ khí **không** bật. Nằm trong nhóm **chờ ổn định relay**: `mdelay(4500)` + `WDT REFRESH 0x1130, 10s` (dòng 315-317) — máy chủ động dừng 4.5 giây trước khi tiếp tục, có thể để đảm bảo relay nguồn cơ khí ở trạng thái an toàn trong lúc có tiến trình cập nhật chạy song song.|
|**check_action()**|Có một nhánh nội bộ **giống hệt** (dòng 603-606): nếu không có USB, tự đọc lại DipSW và set `DEF_IISW` — như đã nói ở phần phân tích `check_action()`, đây gần như là bản sao logic của `setup_MotionEnable_Power()`.|
|**chk_sboot()**|Áp dụng bình thường.|
|**check_autoboot()**|`case DEF_IISW` → `bootcmd` từ `CONFIG_IISWCMD_ENV`, `bootargs` từ `CONFIG_SSD_RFS_ENV`, `autoboot = AUTOBOOT_SSD_SUBSET`.|
|**Khối Warp!!**|Bị bỏ qua — `AUTOBOOT_SSD_SUBSET` không khớp điều kiện Warp. **Hợp lý về mặt thiết kế**: máy đang giữa chừng một phiên tải firmware, tuyệt đối không nên "resume từ snapshot cũ" — phải luôn cold-boot vào đúng kernel IISW để tiếp tục hoặc hoàn tất tiến trình đó.|
|**autoboot_command()**|`WDT REFRESH 0x11E1, 120s` ("IISW download.\n") — khớp với ghi chú 2022/11/04 đã thấy: giữ watchdog sống xuyên suốt phiên IISW thay vì tắt sớm.|

## 2. Nội dung `bootcmd` (giải mã `CONFIG_IISWCMD_ENV`)

```c
#define CONFIG_IISWCMD_ENV   CMD_INITRD_ENV("%s","%s","0:2","")
```

Khác với USB-related macro, cái này **vẫn giữ `%s %s`** nên `make_bootcmds()` hoạt động đúng nghĩa, tự chọn theo `isBootFromSD()`:

```
nvme init;                                    # (hoặc "mmc rescan;" nếu boot từ SD)
fatload nvme 0:2 $kernel_addr $bootfile;
fatload nvme 0:2 $fdt_addr    $dtb_name;
fatload nvme 0:2 $initrd_addr $initrd_name;
booti $kernel_addr $initrd_addr $fdt_addr
```

**Điểm mấu chốt: partition `0:2`** — khác hẳn `0:5` của DEF_BOOT. Đây là bằng chứng rõ ràng có một **kernel/initrd riêng dành cho chế độ IISW** được lưu ở phân vùng FAT khác trên SSD, tách biệt hoàn toàn khỏi ảnh OS production.

## 3. Nội dung `bootargs`

```c
make_bootargs(DEF_IISW, CONFIG_SSD_RFS_ENV)   // giống DEF_BOOT: root=/dev/kmsda7 rootdelay=1
```

```
boottyp=3 root=/dev/kmsda7 rootdelay=1 [nopanel] console=ttyS5,115200
TerminalState=<...> MachineInfo=<hex> MachineNo=<hex> SerialNo=<...>
IISW=1 FwDir=FW9020 DevStr=nvme IntegrityState=<0|1> <CONFIG_BOOTARGS>
```

Hai điểm khác DEF_BOOT:

- **`boottyp=3`**.
- **`IISW=1`** — vì `g_action == DEF_IISW` khớp điều kiện `(main.c:1602)` `g_action==DEF_IISW || g_action==DEF_UPDATE`. Đây chính là trường userspace/kernel dùng để biết mình đang ở chế độ IISW dù `root=` vẫn trỏ **cùng một** `/dev/kmsda7` như boot bình thường — tức kernel/initrd tải từ partition `0:2` riêng, nhưng sau khi chuyển sang Linux thật thì mount **chung** root filesystem với DEF_BOOT.

## 4. Một câu hỏi còn bỏ ngỏ

DipSW offset `0x06` có ít nhất 6 giá trị khác nhau chia thành 2 nhóm: `{5,6} → DEF_IISW`, `{4,7,8,9} → DEF_UPDATE`. Comment tại chỗ định nghĩa cờ này (dòng 221, 227 trong `machine_setup.c`) lặp lại đúng một cụm — dịch được phần "IISW... đối ứng" nhưng phần đầu bị lỗi encoding Shift-JIS, không chắc chắn 100% ý nghĩa từng con số riêng lẻ (ví dụ 5 khác 6 ở điểm gì, 4/7/8/9 khác nhau ra sao). Đây là chỗ mình không dám khẳng định để tránh đoán sai — nếu bạn có tài liệu spec DipSW gốc (không qua encoding lỗi) thì sẽ giải quyết dứt điểm.

## 5. Tóm tắt

`DEF_IISW` đại diện cho trạng thái "đang giữa chừng một phiên tải firmware qua mạng" — nhận diện qua DipSW offset `0x06` (giá trị 5/6), luôn cold-boot một kernel/initrd riêng từ partition `0:2` (không bao giờ qua Warp), tắt cơ cấu cơ khí, chủ động chờ 4.5s ổn định relay, và bàn giao cho kernel cờ `IISW=1` trong khi vẫn mount chung root filesystem `/dev/kmsda7` như boot thường.

---

Tiếp theo hợp lý nhất là `DEF_UPDATE` — cùng nhóm cờ DipSW, cùng đặt `IISW=1`, nhưng bootcmd trỏ tới partition `0:A` (`CONFIG_IISWCMD_UPDATE_ENV`) thay vì `0:2`. Trình bày luôn không?