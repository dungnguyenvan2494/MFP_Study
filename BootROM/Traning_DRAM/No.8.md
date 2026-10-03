# 適用No.8 — Thêm hiển thị Vendor vào Debug log (Bản dịch + Giải thích chi tiết)

## Kiến thức nền cần biết trước

- **Debug log** khác với **BOOT log (垂れ流しログ)** đã nói ở No.7 — đây là log **chi tiết hơn**, thường chỉ bật khi cần điều tra sâu (debug build hoặc bật flag đặc biệt), in ra nhiều thông tin nội bộ của quá trình training/cấu hình DDR mà log thường không hiện.
- **Manufacturer ID**: mỗi hãng sản xuất chip nhớ (Micron, Samsung, Hynix...) có 1 mã định danh (ID) chuẩn được ghi cứng trong chính con chip đó (theo chuẩn JEDEC). Firmware đọc mã ID này từ chip thật để biết chính xác đang chạy với chip của hãng nào, sau đó **dịch mã ID số** đó thành **tên hãng dễ đọc** để in ra log.
- **MEM_VENDER** (khác với MEMORY_CHIP_VERSION ở No.7): đây là 1 tham số riêng, lưu sẵn trong file tham số LPDDR4 (không phải đọc trực tiếp từ chip), dùng cho mục đích hiển thị debug log.

---

## Tiêu đề: 適用No.8　デバッグログのベンダー表示の追加

→ **Áp dụng No.8 — Thêm hiển thị Vendor (nhà sản xuất) vào Debug log**

## 【新規】(Mới) — デバッグログについて (Về Debug log)

> 新規ベンダーのベンダーIDを使用可能とする。 → Cho phép sử dụng **Vendor ID** của các vendor mới.

(Khác với No.7 là sửa cách hiển thị trong log thông thường dựa trên tham số cấu hình, No.8 tập trung vào việc **debug log có thể nhận diện đúng mã ID vật lý** của các hãng chip mới khi đọc trực tiếp từ chip.)

---

## Vẽ lại & giải thích: デバッグログの該当箇所 (Đoạn debug log liên quan)

Đây là đoạn log thực tế được trích ra, mình dịch và giải thích từng dòng:

```
isLogOut[1]
```

→ Biến cờ nội bộ cho biết chế độ log chi tiết đang được bật (giá trị = 1, tức "có xuất log").

```
NOTICE:  Starting binary extension
```

→ Thông báo: đang bắt đầu chạy phần mở rộng nhị phân (một module code phụ được nạp thêm, thường là phần xử lý training DDR).

```
syncronize_boot_get: block 0; mask 0x20000
syncronize_boot_get: value 0
syncronize_boot_get: block 0; mask 0x10000
syncronize_boot_get: value 0
syncronize_boot_get: block 0; mask 0xffff
syncronize_boot_get: value 0x4b0
```

→ Đây là các lệnh **đọc giá trị cấu hình đồng bộ hóa quá trình boot** (`syncronize_boot_get`) từ 1 block bộ nhớ cấu hình, với các "mask" (mặt nạ bit) khác nhau để lọc ra từng phần thông tin — mục đích để lấy các tham số ban đầu quyết định cách khởi tạo DDR (giống việc đọc từng ô nhỏ trong 1 bảng cấu hình lớn).

```
Proposed Configuring DDR for 1200 MHz
PLL is 1050, requested freq was 1200, usable frequency is 1050
Configuring DDR for 1050 MHz
```

→ Ban đầu hệ thống **định cấu hình DDR chạy ở 1200MHz**, nhưng phát hiện bộ PLL (mạch tạo xung nhịp) hiện tại chỉ đang chạy ở 1050MHz, nên tần số khả dụng thực tế chỉ là 1050MHz → **cuối cùng cấu hình DDR chạy ở 1050MHz** (thấp hơn mong muốn ban đầu, do giới hạn phần cứng ở thời điểm đó).

```
Rev.A ? -> 0
```

→ Kiểm tra xem board có phải revision A hay không → kết quả trả về 0 (không phải rev A).

```
GPIOa: 0x20ebffaf
GPIOh: 0x6e10ff5f
GPIOi: 0x3c00fff4
```

→ Đọc giá trị thô của 3 nhóm chân GPIO (nhóm a, h, i) — các chân này được dùng để mã hóa vật lý một số thông tin cấu hình board (ví dụ loại board, loại chip lắp sẵn...).

```
Detected Emu_board
```

→ Xác định đây là loại board "Emu_board" (board thử nghiệm/mô phỏng, giống với "EMU800" đã thấy ở log No.7).

**Dòng được khoanh đỏ (trọng tâm của No.8):**

```
Memory Vender: CH0(Micron), CH1(Micron)
```

→ **Đây chính là dòng log mà 適用No.8 tác động vào**: hiển thị tên nhà sản xuất memory cho từng channel — ở đây cả CH0 và CH1 đều là **Micron**. Vấn đề là: cách hiển thị này hiện tại **chỉ nhận diện được Micron và Samsung**, nếu chip thực tế là Hynix/Nanya/Winbond/ISSI thì debug log sẽ không hiển thị đúng tên (có thể hiện sai hoặc trống).

```
Memory Size  : 8GB(4GB+4GB)
Start Training by using QSPI Parameter
abcdefghijklm
```

→ Tổng dung lượng RAM = 8GB (chia đều 4GB cho mỗi channel); bắt đầu quá trình training sử dụng tham số đọc từ QSPI; dòng cuối `abcdefghijklm` là 1 chuỗi ký tự kiểm tra/test pattern (không mang ý nghĩa đặc biệt, chỉ là dữ liệu mẫu dùng để test truyền nhận).

---

## 当該箇所はLPDDR4パラメータ内のMEM_VENDER(Addr:0x10、0x14)を参照する。

→ Đoạn log này tham chiếu tới tham số **MEM_VENDER** nằm trong tham số LPDDR4 (địa chỉ `0x10` và `0x14`).

|アドレス (Địa chỉ)|レジスタ名 (Tên register)|役割 (Vai trò)|
|---|---|---|
|`0x0010`|CH0_MEM_VENDER|Tên hãng sản xuất DRAM của **CH0**|
|`0x0014`|CH1_MEM_VENDER|Tên hãng sản xuất DRAM của **CH1**|

---

## Micron・Samsung以外のベンダー情報を追加して表示させる。

→ Bổ sung thông tin của các vendor **ngoài Micron và Samsung** để hiển thị được đầy đủ.

## Bảng: Manufacturer ID表 (Bảng mã định danh nhà sản xuất)

|メーカー (Hãng)|Manufacturer ID|Trạng thái|
|---|---|---|
|Micron|`0x11111111` (0xFF)|(có sẵn từ trước)|
|Samsung|`0x00000001` (0x01)|(có sẵn từ trước)|
|**Hynix**|`0x00000110` (0x06)|追加 = **Thêm mới**|
|**Nanya**|`0x00000101` (0x05)|追加 = **Thêm mới**|
|**Winbond**|`0x11101111` (0xEF)|追加 = **Thêm mới**|
|**ISSI**|`0x00010011` (0x13)|追加 = **Thêm mới**|

> Giải thích thêm về Manufacturer ID: đây là mã nhận diện chuẩn JEDEC được ghi sẵn trong chính con chip DRAM (đọc được qua lệnh Mode Register Read khi giao tiếp với chip). Giá trị trong ngoặc đơn (ví dụ "0xFF" cho Micron) là dạng viết gọn/mã hex rút gọn của giá trị đầy đủ bên trái — có thể do một số bit lặp lại theo mẫu (pattern) nên chỉ cần 1 byte đại diện là đủ phân biệt.

---

## Tổng kết dễ hiểu 適用No.8

Nếu No.7 là sửa **log hiển thị chuẩn (BOOT log)** để hiện đúng tên vendor + version shrink dựa trên tham số cấu hình lưu sẵn, thì No.8 là sửa **log debug chi tiết hơn** để nó có thể **dịch đúng mã Manufacturer ID vật lý đọc trực tiếp từ chip** thành tên hãng — vì hiện tại bảng tra cứu (lookup table) trong code chỉ có 2 mục (Micron, Samsung), nên khi gặp chip Hynix/Nanya/Winbond/ISSI thật, debug log sẽ không nhận diện được đúng tên. Bản sửa này bổ sung thêm 4 mục mới vào bảng tra cứu đó (Hynix, Nanya, Winbond, ISSI) với đúng mã Manufacturer ID chuẩn của từng hãng, để dòng log `Memory Vender: CH0(...), CH1(...)` luôn hiển thị đúng tên hãng thực tế bất kể linh kiện nào được lắp trên board.