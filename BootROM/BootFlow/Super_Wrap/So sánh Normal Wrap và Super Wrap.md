Trước khi so sánh, một phát hiện quan trọng vừa tra ra: `warp_boot()` (trong `common/cmd_warp.c`, header ghi rõ **"Warp!! boot driver Rev. 5.1.0 — Copyright (C) 2008-2017 Lineo Solutions, Inc."**) không tự triển khai việc resume — nó nạp một **"hibernation driver" nhị phân** (lưu ở SPI Flash offset `0x2b0000`, nhận diện qua magic ID `'W5HD'`) rồi **nhảy thẳng vào một con trỏ hàm bên trong blob đó** (`hibernate()` tại offset `WARP_HEADER_HIBERNATE`). Nghĩa là cơ chế resume-từ-snapshot thật sự nằm trong một driver đóng gói sẵn của Lineo Solutions — **không có trong mã nguồn U-Boot này**. Mình sẽ nói rõ ở mỗi phần chỗ nào là bằng chứng trực tiếp từ source, chỗ nào là suy luận có căn cứ.

## 1. Thời gian boot

Không có số liệu benchmark thực tế trong source, nhưng có **bằng chứng gián tiếp rất mạnh**: ngân sách watchdog mà chính kỹ sư firmware đặt cho mỗi loại — phản ánh kỳ vọng thời gian hoàn thành thực tế của họ:

|Loại boot|WDT event code|Timeout|Ý nghĩa|
|---|---|---|---|
|SuperWarp|`0x11b0`|**40 giây**|Ngân sách chặt nhất trong toàn hệ thống|
|Normal Warp|`0x11a0`|**90 giây**|Gấp hơn 2 lần SuperWarp|
|Cold boot (`DEF_BOOT`)|`0x11D2`|**300 giây**|Gấp 7.5 lần SuperWarp, 3.3 lần Normal Warp|

Nếu kỹ sư không tin SuperWarp hoàn thành nhanh hơn hẳn, họ đã không dám đặt watchdog chỉ 40s — vì nếu quá hạn, watchdog sẽ **tự reset máy giữa chừng**. Đây là bằng chứng thực tế đáng tin nhất về khoảng cách tốc độ giữa 3 loại, dù không phải số đo trực tiếp.

## 2. Vì sao SuperWarp nhanh hơn — tối giản ở đâu?

### Điều verify được từ BootROM: cả hai dùng **chung một cơ chế**, chỉ khác "số slot"

```c
int si_SuperWarpSSID = isBootFromSD() ? WARP_SSID_SUPER_SD  : WARP_SSID_SUPER_SSD;   // slot 2/3
int si_NormalSSID    = isBootFromSD() ? WARP_SSID_NORMAL_SD : WARP_SSID_NORMAL_SSD;  // slot 0/1
...
warp_boot( si_SuperWarpSSID );   // thử trước
...
warp_boot( si_NormalSSID );      // thử sau nếu Super fail
```

`warp_boot(saveno)` → `warp_boot_fg(saveno)` → nạp cùng một driver blob → nhảy vào cùng một hàm `hibernate()`. **BootROM không có logic riêng nào "làm ít việc hơn" cho Super so với Normal** — khác biệt duy nhất nhìn thấy được ở tầng này là `saveno` (chỉ vị trí lưu snapshot khác nhau trên storage). Vậy phần quyết định "SuperWarp thực sự bỏ qua bước nào" nằm **hoàn toàn bên trong** driver blob nhị phân đó và bên phía kernel Linux tạo ra snapshot — ngoài tầm với của source này, nên mình không thể khẳng định chắc chắn 100%.

### Cơ chế chung của hibernate-resume (kiến thức nền tảng, áp dụng được vì đây xác nhận là sản phẩm resume-từ-RAM-image thật của Lineo)

Dù không thấy chi tiết implementation, bản chất công nghệ "Warp!!" (resume từ ảnh RAM đã lưu) về nguyên lý bỏ qua được các bước tốn thời gian nhất của cold-boot:

- **Giải nén + link kernel image** → resume chỉ cần nạp lại đúng các trang RAM đã lưu vào đúng địa chỉ vật lý, không cần giải nén/link lại từ đầu.
- **Dò & khởi tạo từng driver/thiết bị một** (PCI enumeration, USB probe, panel init...) → tất cả trạng thái driver đã "đông cứng" sẵn trong snapshot, không cần chạy lại chuỗi init tuần tự.
- **fsck + mount root filesystem** → filesystem đã ở trạng thái mounted sẵn trong snapshot.
- **Chạy lại toàn bộ init script/service userspace** → tiến trình đã ở đúng trạng thái đang chạy dở, chỉ cần khôi phục context CPU/register rồi "return" thẳng vào đúng điểm code đang thực thi lúc lưu — giống "tạm dừng rồi tiếp tục" thay vì "phát lại từ đầu".

### Vì sao Super nhanh hơn Normal — suy luận có căn cứ (chưa verify trực tiếp được nội dung 2 loại snapshot)

Từ 2 bằng chứng thực tế đã có ở BootROM: (a) ngân sách thời gian SuperWarp chặt hơn hẳn, (b) cơ chế `isDisableSuperWarp_ErrorOccurred()` **chỉ vô hiệu hoá SuperWarp**, không đụng tới Normal Warp khi có crash — mình suy luận có căn cứ rằng: **SuperWarp phục hồi từ một snapshot "khớp chính xác" hơn** (bám sát trạng thái đang chạy ngay trước đó → resume nhanh nhưng đòi hỏi điều kiện khắt khe hơn để đúng), còn **Normal Warp là bản dự phòng bảo thủ hơn** (chấp nhận chậm hơn để đổi lấy độ ổn định cao hơn). Đây là suy luận từ hành vi quan sát được, không phải đọc trực tiếp code tạo snapshot.

## 3. SuperWarp có gây vấn đề gì không? — CÓ, và có bằng chứng cụ thể

### (a) Đã từng gây kernel panic thật trong quá khứ — có mã bug tracking cụ thể

```c
/* 2017/05/23 AR1023389 sky-ishiga KernelPanic/BUG(Oops)発生時に次回SuperWarp不可とする */
static uchar isDisableSuperWarp_ErrorOccurred( void )
```

Đây là bằng chứng **trực tiếp, không suy luận**: từng có sự cố thực tế (mã `AR1023389`) khiến kernel panic/BUG(Oops) — nghi ngờ liên quan tới việc resume từ snapshot SuperWarp — đủ nghiêm trọng để kỹ sư phải xây riêng một cơ chế: nếu kernel/main FW ghi cờ `0x10` báo lỗi, **lần boot kế tiếp tự động cấm SuperWarp** (nhưng Normal Warp vẫn được thử). Đây trực tiếp trả lời câu hỏi "có gây vấn đề không" — có, và đủ nghiêm trọng để cần một lớp tự-bảo-vệ riêng.

### (b) Hệ thống 12-lý-do từ chối tồn tại chính vì rủi ro resume-nhầm-snapshot

Toàn bộ `isChgWarpBootSettingInfo()` (RAM size, PCI devices, panel type/option/size, A800...) là hàng rào chống lại kịch bản: phần cứng đã đổi nhưng vẫn cố resume vào một snapshot cũ — dẫn tới driver điều khiển sai thiết bị, vẽ sai kích thước panel, hoặc crash vì thiếu thiết bị mà snapshot tưởng vẫn còn. Việc cần tới 12 điều kiện kiểm tra riêng biệt cho thấy nhóm phát triển coi đây là rủi ro thực sự đáng lo, không phải lý thuyết.

### (c) Chi phí khi thất bại — tệ hơn cả không dùng Warp

Nếu SuperWarp thất bại **và** Normal Warp cũng thất bại, máy phải trả giá cho **cả 2 lần thử hỏng** trước khi mới rơi về cold-boot — tổng thời gian trong kịch bản xấu nhất còn **chậm hơn** so với việc cold-boot ngay từ đầu.

### (d) Có cả "cửa thoát" vật lý dành riêng cho khi Warp có vấn đề

Công tắc GPIO `WARP_SW` (bắt buộc phải ở đúng vị trí mới cho phép thử Warp) và phím panel Trouble-Reset (`0x16`, chặn cả 2 loại Warp) đều là cơ chế **con người chủ động ép cold-boot** — tồn tại vì kỹ thuật viên đôi khi cần bỏ qua Warp khi nghi ngờ nó là nguyên nhân gây lỗi, một dấu hiệu nữa cho thấy đây là tính năng có rủi ro đã được lường trước trong thiết kế, không chỉ là tối ưu thuần tuý không đánh đổi gì.

## Tóm tắt

SuperWarp nhanh hơn Normal Warp (ngân sách 40s so với 90s) nhờ cùng cơ chế resume-từ-snapshot-RAM chung của cả hai (bỏ qua giải nén/link kernel, dò thiết bị, mount filesystem, chạy lại init) nhưng có khả năng dùng một snapshot "khớp sát" hơn nên nhanh hơn — đổi lại, đây là tính năng **có rủi ro đã được ghi nhận thực tế** (một sự cố kernel panic cụ thể từng buộc phải xây cơ chế tự-vô-hiệu-hoá), nên hệ thống bao quanh nó bằng nhiều lớp phòng thủ: 12 điều kiện đối chiếu phần cứng, cờ tự-tắt sau crash, và cả công tắc vật lý cho phép con người ép cold-boot khi cần.