# 適用No.9 — Sửa hiển thị tốc độ sai trong Debug log chi tiết (Bản dịch + Giải thích chi tiết)

## Kiến thức nền cần biết trước

- **2100Mbps / 2400Mbps**: đây là **tốc độ truyền dữ liệu (data rate)** của LPDDR4, tính bằng Mbps (megabit/giây trên mỗi chân dữ liệu). LPDDR4 có nhiều "mức tốc độ chuẩn" (speed bin) như 1600, 1866, 2133, 2400, 2666, 3200... Số càng cao thì RAM chạy càng nhanh, nhưng yêu cầu tín hiệu càng khắt khe hơn (đây cũng là lý do vì sao các bài trước nói về "training" và "delay margin" quan trọng).
- **Bit field**: một thanh ghi (register) 32-bit có thể được chia thành nhiều "vùng nhỏ" (field), mỗi vùng vài bit, mỗi vùng mang 1 ý nghĩa riêng — giống như một tờ khai có nhiều ô, mỗi ô ghi 1 thông tin khác nhau. Ký hiệu `[31:28]` nghĩa là "vùng bit từ bit thứ 31 xuống bit thứ 28" (4 bit); `[31:24]` là "từ bit 31 xuống bit 24" (8 bit).
- **QSPI_PARAM_REVISION**: đây là 1 tham số trong file cấu hình LPDDR4, dùng để đánh dấu **phiên bản/tốc độ** của bộ tham số đang dùng.

---

## Tiêu đề: 適用No.9　デバッグ詳細ログの速度表示の修正

→ **Áp dụng No.9 — Sửa hiển thị tốc độ trong Debug log chi tiết**

## 【新規】(Mới) — デバッグログについて (Về Debug log)

> 詳細ログ表示時にパラメータの速度が2400の際に表示が不一致となる問題の修正 → Sửa lỗi: khi hiển thị log chi tiết, nếu tham số có tốc độ là **2400** thì phần hiển thị lại **không khớp** (hiện sai thành giá trị khác).

---

## Vẽ lại & giải thích: デバッグログ(詳細ログ)先頭 (Đầu đoạn Debug log chi tiết)

Đây là 1 đoạn log thực tế được trích ra để minh họa lỗi. Mình dịch và giải thích từng dòng:

```
isLogOut[1]
NOTICE:  Starting binary extension
syncronize_boot_get: block 0; mask 0x20000
syncronize_boot_get: value 0
syncronize_boot_get: block 0; mask 0x10000
syncronize_boot_get: value 0
syncronize_boot_get: block 0; mask 0xffff
syncronize_boot_get: value 0x4b0
```

→ (Giống hệt phần đã giải thích ở No.8: đọc các giá trị cấu hình đồng bộ hóa boot theo từng mặt nạ bit khác nhau.)

```
Proposed Configuring DDR for 1200 MHz
Configuring DDR for 1200 MHz
```

→ Lần này hệ thống **đề xuất và thực sự cấu hình DDR chạy đúng ở 1200MHz** (khác với ví dụ log ở No.8, nơi bị giới hạn xuống 1050MHz do PLL) — cho thấy đây là 1 lần chạy log khác, trong điều kiện phần cứng cho phép chạy đúng tần số mong muốn.

```
Rev.A ? -> 0
```

→ Kiểm tra board có phải rev A không → không phải (giống các log trước).

```
TRN-A0:   0 ms
```

→ Ghi lại mốc thời gian (tính bằng mili-giây, ms) khi bắt đầu **phase TRN-A0** — đây là 1 giai đoạn (phase) cụ thể trong chuỗi quá trình training LPDDR4 (nhắc lại: LPDDR4 training gồm nhiều phase nối tiếp nhau như CA training, DQ training... TRN-A0 là 1 trong các phase con của quá trình đó, mốc 0ms nghĩa là log đo thời gian tính từ lúc phase này bắt đầu).

```
msg_window_val         : 0x00000001
clk_freq_wait_time1     : reg 4, set 4
clk_freq_wait_time2     : reg 2, set 2
vref_fixed_num_read     : reg 23, set 23
vref_fixed_num_write    : reg 39, set 39
vref_countup_num_dnaread: reg 2, set 2
vref_countup_num_write  : reg 2, set 2
dly_step_read_dq_train  : reg 1, set 1
dly_step_write_dq_train : reg 1, set 1
```

→ Đây là danh sách các **tham số nội bộ cụ thể** dùng trong quá trình training (thời gian chờ tần số xung nhịp, số lần thử vref khi đọc/ghi, bước nhảy delay khi training DQ...). Định dạng `reg X, set Y` nghĩa là: giá trị đọc được từ register là X, và giá trị thực sự được set/áp dụng là Y (ở đây X luôn bằng Y, cho thấy các tham số này được set đúng như mong đợi — không có vấn đề gì ở đây).

```
memory_chip_version     : 0x0140140140
```

→ Giá trị chip version đọc được (tương ứng với register `MEMORY_CHIP_VERSION` đã nói ở No.7).

**Dòng được khoanh đỏ (trọng tâm của No.9, và là dòng bị lỗi):**

```
For LPDDR4-2100 parameter Rev.2.22
```

→ Log hiển thị: **"Đang dùng tham số cho LPDDR4-2100, phiên bản Rev 2.22"** → **NHƯNG** thực tế bộ tham số đang dùng là dành cho tốc độ **2400**, không phải 2100! Đây chính là lỗi: "khi dùng tham số 2400 lại bị hiển thị nhầm thành 2100".

```
GPIOa: 0x5tebfeaf
GPIOh: 0x6e10ff5f
GPIOi: 0x7c00fff4
```

→ (Giống các log trước, đọc giá trị thô của các nhóm chân GPIO.)

**Ghi chú màu đỏ bên dưới ảnh:**

> 該当箇所は2400のパラメータを使用しても2100の表示となってしまう。 → Tại vị trí này (dòng log "For LPDDR4-xxxx parameter"), dù đang sử dụng tham số 2400 nhưng vẫn bị hiển thị thành 2100.

---

## 当該箇所はパラメータ内のQSPI_PARAM_REVISION(Addr:0x404)を参照すると思われる。

→ Vị trí gây lỗi này được cho là đang tham chiếu tới **QSPI_PARAM_REVISION** (địa chỉ `0x404`) trong tham số.

|アドレス (Địa chỉ)|レジスタ名 (Tên register)|役割 (Vai trò)|
|---|---|---|
|`0x0404`|QSPI_PARAM_REVISION|Phiên bản (Revision) của tham số QSPI|

## Bảng bit field của QSPI_PARAM_REVISION

|bit|内容 (Nội dung)||
|---|---|---|
|`[31:28]`|0 : 2100Mbps, 1 : 2400Mbps|← **仕様を変更** (đây là vùng bit theo **spec cũ**, sẽ bị thay đổi)|
|`[31:24]`|(không ghi nội dung, để trống trong bảng)|← vùng bit thực tế **đang được dùng**|

---

## Giải thích nguyên nhân gốc rễ

> 本来の仕様は[31:28]の4bitの範囲となっていたが → Đặc tả ban đầu (theo thiết kế/tài liệu spec) quy định vùng bit đánh dấu tốc độ nằm ở **[31:28]** (chỉ 4 bit).

> 対象箇所以外のソフト処理、及びLPDDR4パラメータは[31:24]の8bitを使用している。 → Nhưng trên thực tế, **tất cả các phần xử lý phần mềm khác** (ngoại trừ đúng đoạn code bị lỗi này) và **cả file tham số LPDDR4** đều đang sử dụng vùng bit rộng hơn: **[31:24] (8 bit)**.

**Nói cách khác**: có 1 sự "lệch pha" giữa tài liệu spec gốc (nói dùng 4 bit `[31:28]`) và thực tế triển khai (toàn bộ hệ thống đã âm thầm chuyển sang dùng 8 bit `[31:24]` từ lâu, chỉ riêng đoạn code in ra dòng log "For LPDDR4-xxxx parameter" là vẫn đọc theo đúng 4 bit cũ). Vì đọc thiếu 4 bit cao hơn (bit 27~24), nên khi giá trị thực tế đại diện cho "2400" nằm trong khoảng 8-bit đó, đoạn code cũ (chỉ nhìn 4 bit) lại hiểu nhầm ra kết quả tương ứng với "2100".

> 影響範囲を鑑みて、仕様側の修正と当該ログ表示の修正のみとしたい。 → Xét đến phạm vi ảnh hưởng (nếu sửa thay đổi toàn bộ hệ thống về đúng chuẩn 4-bit ban đầu sẽ ảnh hưởng rất rộng, rủi ro cao), nhóm quyết định: **chỉ sửa lại tài liệu spec** (chính thức công nhận vùng bit là `[31:24]` — 8 bit, theo đúng thực tế đang chạy) **và sửa đúng đoạn code hiển thị log bị lỗi** để nó cũng đọc theo `[31:24]` giống mọi nơi khác — không đụng chạm/thay đổi gì đến các phần xử lý khác.

---

## Tổng kết dễ hiểu 適用No.9

Đây là 1 kiểu bug "documentation mismatch" (tài liệu và code lệch nhau) rất phổ biến trong phát triển firmware lâu năm: tài liệu spec ban đầu ghi vùng bit đánh dấu tốc độ là 4-bit `[31:28]`, nhưng qua thời gian, khi cần thêm giá trị mới (2400Mbps), đội phát triển đã âm thầm mở rộng ra 8-bit `[31:24]` ở hầu hết mọi nơi trong code và trong file tham số — nhưng lại **bỏ sót đúng 1 chỗ**: đoạn code in dòng log "For LPDDR4-xxxx parameter Rev.x.xx" ở đầu debug log, vẫn đọc theo định nghĩa 4-bit cũ. Kết quả là chức năng thực tế (training, cấu hình DDR) vẫn chạy đúng ở 2400Mbps bình thường — **chỉ có dòng chữ hiển thị trên log debug là bị sai**, gây khó khăn/nhầm lẫn cho kỹ sư khi đọc log để debug. Giải pháp chọn là cách an toàn nhất: chính thức cập nhật lại spec cho khớp thực tế, và chỉ sửa đúng chỗ hiển thị log bị sai, không động vào bất kỳ logic xử lý nào khác để tránh rủi ro phát sinh lỗi mới.