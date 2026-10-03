# Bảng "Đối chiếu IO theo đúng tên Pin chuẩn" (Bản dịch + Giải thích chi tiết)

## Tiêu đề: ■IOの表記をPin名に合わせたもの

→ **■ Bảng đã đổi cách ghi ký hiệu IO cho khớp với tên Pin (chuẩn theo datasheet)**

Đây chính là **bước tiếp theo** của trang điều tra trước đó (bảng "DDR PHYレジスタ調査"). Ở trang trước, kỹ sư dùng ký hiệu tạm `ch0`, `ch1` (không rõ ràng, gây ra nghi vấn "ch0=device0? ch1=device1?"). Ở trang này, họ đã **thay thế toàn bộ ký hiệu cũ bằng đúng tên pin chuẩn lấy từ datasheet** (`A_...`, `B_...` — khớp với `AP_M_A_...`, `AP_M_B_...` trong bảng pin chính thức đã xem ở trang trước) để bản đồ IO trở nên rõ ràng, không còn mơ hồ.

---

## Bảng 1: Control IO MAP (đã đổi tên theo pin chuẩn)

|pup→|0|1|2|3|4|5|6|7|8|9|10|
|---|---|---|---|---|---|---|---|---|---|---|---|
|**cs0-0**|A_CKE1|–|–|–|A0_CLKn, A0_CLK|A_CKE1|A_CA3|A_CKE0|–|–|A_CA5|
|**cs0-1**|A_CKE0|A_CA1|A_CA0|A_CS0|B0_CLKn, B0_CLK|A_CA4|B_CA4|A_CA2|A_CA2|B_CA5|B_CA3|
|**cs0-2**|–|–|B_CK1n|–|A1_CLK|A1_CLKn|B_CK1|–|–|–|–|
|**cs0-3**|B_CS1|–|B_CA1|A_CS1|–|B_CA2|A_CA0|–|–|–|B_CS0|
|**cs1**|colspan: "cs0と同じ（デスキュー値もcs1と同じものがセットされる）、恐らく未使用" → Giống hệt cs0 (giá trị deskew cũng được set giống như cs1). Có lẽ không được sử dụng.|||||||||||

### Bảng chú thích đi kèm

|Ký hiệu|Ý nghĩa|
|---|---|
|A|device0 (ch0)|
|B|device1 (ch1)|
|0|CS0|
|1|CS1|

| Ví dụ                | Giải thích                            |
| -------------------- | ------------------------------------- |
| `A0_...` (vd A0_CLK) | Thuộc **device0**, thuộc nhóm **CS0** |
**Lưu ý dịch từng ô quan trọng**:

- `A_CKE1`, `A_CKE0` = tín hiệu CKE (Clock Enable) của **channel/device A**
- `A_CA0`~`A_CA5` = tín hiệu CA (Command/Address) của channel A
- `A_CS0`, `A_CS1` = tín hiệu CS (Chip Select) của channel A
- `A0_CLK`, `A0_CLKn`, `A1_CLK`, `A1_CLKn` = 2 cặp xung nhịp vi sai của channel A (khớp với `AP_M_A0_CLKOUT` và `AP_M_A1_CLKOUT` trong bảng pin đã xem ở trang trước)
- Tương tự với tiền tố `B_...`, `B0_...`, `B1_...` cho **channel/device B**

**So sánh với trang trước**: đây chính xác là cùng 1 bảng "Control IO MAP (RevA)" đã xem ở lượt trước, chỉ khác là tên ký hiệu `ch0`→ đổi thành `A`, `ch1`→ đổi thành `B`. Việc đổi tên này giúp xác nhận chắc chắn giả thuyết đã đặt ra trước đó ("ch0 dùng cho device0, ch1 dùng cho device1") — giờ được viết thẳng ra rõ ràng bằng tên chuẩn `A`/`B` khớp với tên pin thật trên chip.

**Ghi chú thêm ở hàng cs1**: có thêm cụm từ mới "**恐らく未使用**" (**có lẽ không được sử dụng**) — đây là bổ sung so với trang trước, cho thấy kỹ sư giờ nghiêng về nhận định rằng cs1 (hàng thứ 2) trong Control IO MAP có khả năng **không thực sự được dùng đến** trong thiết kế này.

---

## Bảng chú thích bên phải (đi kèm bảng Control IO MAP)

```
A: device0(ch0)     0: CS0     →
B: device1(ch1)     1: CS1
```

> A0_の場合はdevice0のCS0に属する → **Trong trường hợp là "A0_" (ví dụ A0_CLK), nó thuộc về CS0 của device0.**

**Giải thích ý nghĩa toàn bộ chú thích này**: đây chính là "chìa khóa giải mã" ký hiệu:

- Chữ cái đầu **A** hoặc **B** = cho biết đây là **device0 (tương đương ch0)** hay **device1 (tương đương ch1)**.
- Số đứng sau (0 hoặc 1, ví dụ trong `A0_CLK`) = cho biết tín hiệu này gắn với **CS0** hay **CS1** (tức đang chọn die/chip nào).
- Ví dụ cụ thể: `A0_CLK` = tín hiệu CLK, thuộc **device0**, và thuộc nhóm **CS0**.

**→ Đây chính là câu trả lời dứt khoát cho câu hỏi đặt dấu "?" ở trang trước**: đúng là **ch0 = device0**, **ch1 = device1** — giả thuyết ban đầu của kỹ sư đã được xác nhận là đúng.

---

## Bảng 2: Data IO MAP (đã đổi tên theo pin chuẩn)

### cs0

|Nhóm|pup 0|1|2|3|4|5|6|7|8|9|10|
|---|---|---|---|---|---|---|---|---|---|---|---|
|**A0_** (row0)|DQ0|DQ7|DQ2|DM0|–|–|DQ3|DQ1|DQ4|DQ6|DQ5|
|(row1)|DQ11|DQ10|DQ9||–|–|DQ8|DQ15|DQ12|DQ13|DQ14|
|**B0_** (row2)|DQ6|DQ4|DQ3|DM0|–|–|DQ5|DQ0|DQ7|DQ2|DQ1|
|(row3)|DQ14|DQ12|DQ13||–|–|DQ11|DQ8|DQ15|DQ10|DQ9|
|**A1_** (row4)|DQ0|DQ2|DQ13|DM0|–|–|DQ3|DQ4|DQ5|DQ6|DQ7|
|(row5)|DQ8|DQ10|DQ9||–|–|DQ11|DQ12|DQ13|DQ14|DQ15|
|**B1_** (row6)|DQ0|DQ2|DQ3|DM0|–|–|DQ1|DQ4|DQ5|DQ6|DQ7|
|(row7)|DQ8|DQ9|DQ10||–|–|DQ11|DQ12|DQ13|DQ14|DQ15|

### cs1 (nội dung giống hệt cs0)

|Nhóm|pup 0|1|2|3|4|5|6|7|8|9|10|
|---|---|---|---|---|---|---|---|---|---|---|---|
|**A0_** (row0)|DQ0|DQ7|DQ2|DM0|–|–|DQ3|DQ1|DQ4|DQ6|DQ5|
|(row1)|DQ11|DQ10|DQ9||–|–|DQ8|DQ15|DQ12|DQ13|DQ14|
|**B0_** (row2)|DQ6|DQ4|DQ3|DM0|–|–|DQ5|DQ0|DQ7|DQ2|DQ1|
|(row3)|DQ14|DQ12|DQ13||–|–|DQ11|DQ8|DQ15|DQ10|DQ9|
|**A1_** (row4)|DQ0|DQ2|DQ13|DM0|–|–|DQ3|DQ4|DQ5|DQ6|DQ7|
|(row5)|DQ8|DQ10|DQ9||–|–|DQ11|DQ12|DQ13|DQ14|DQ15|
|**B1_** (row6)|DQ0|DQ2|DQ3|DM0|–|–|DQ1|DQ4|DQ5|DQ6|DQ7|
|(row7)|DQ8|DQ9|DQ10||–|–|DQ11|DQ12|DQ13|DQ14|DQ15|

**Giải thích ký hiệu hàng**: `A0_`, `B0_`, `A1_`, `B1_` ở cột đầu tiên của mỗi nhóm 2 hàng — đây chính là khớp trực tiếp với tên pin `AP_M_A0_DQ`, `AP_M_B0_DQ`, `AP_M_A1_DQ`, `AP_M_B1_DQ` đã thấy trong bảng datasheet ở trang trước (nhắc lại: A/B = 2 channel/device, số 0/1 phía sau = 2 nhóm DQ độc lập trong cùng 1 channel — vì mỗi channel có tới 32 đường DQ (2 x DQ[15:0]), nên phải chia làm 2 nhóm "0" và "1").

**So với trang trước**: nội dung số liệu DQ giống hệt như "Data IO MAP (RevB)" đã xem, chỉ khác là ký hiệu hàng đã đổi từ `ch0`/`ch1` sang đúng chuẩn `A0_/B0_/A1_/B1_`.

---

## Bảng 3 (giữa): Ánh xạ Device → tín hiệu CS → cặp Dual-channel

```
デバイス内(Trong device)    CS信号(Tín hiệu CS)    dual-channelのペア(Cặp dual-channel)
──────────────────────────────────────────────────────────────
device0(ch0)  cs0  die0  →  AP_M_A_CS0  ─┐
device1(ch1)  cs0  die0  →  AP_M_B_CS0  ─┤
                                          ├─ (nhóm cặp 1)
device0(ch0)  cs0  die1  →  AP_M_A_CS0  ─┤
device1(ch1)  cs0  die1  →  AP_M_B_CS0  ─┘

device0(ch0)  cs1  die2  →  AP_M_A_CS1  ─┐
device1(ch1)  cs1  die2  →  AP_M_B_CS1  ─┤
                                          ├─ (nhóm cặp 2)
device0(ch0)  cs1  die3  →  AP_M_A_CS1  ─┤
device1(ch1)  cs1  die3  →  AP_M_B_CS1  ─┘
```

**Điểm khác biệt so với bảng tương tự ở trang trước**: giờ đã ghi rõ ràng **"device0(ch0)"** và **"device1(ch1)"** ngay trong cùng 1 ô — xác nhận dứt khoát mối quan hệ ch0↔device0, ch1↔device1 mà trước đó chỉ là suy đoán.

Thêm cột mới **"dual-channelのペア" (Cặp dual-channel)**: dấu ngoặc `{` bên phải nhóm 2 dòng lại với nhau — thể hiện trực quan việc `AP_M_A_CS0` và `AP_M_B_CS0` là **1 cặp tín hiệu song song** (dual-channel pair), tức là khi hệ thống truy cập die0 hoặc die1 dưới cs0, cả 2 tín hiệu CS0 của channel A và channel B đều được kích hoạt **đồng thời** (vì 2 device A và B luôn hoạt động song song, chia sẻ cùng nhịp điều khiển CS/CKE/CA, nhưng dữ liệu DQ thì đi trên đường riêng của mỗi device — đúng như bus 32-bit rộng gấp đôi 16-bit của 1 device).

### Ghi chú (giống trang trước, nhắc lại để nhất quán):

> csが変わっても使用するDQは同じなのでIO MAPとしては同じになる。 → Dù cs thay đổi thì DQ sử dụng vẫn giống nhau, nên bản đồ IO (IO MAP) coi như không đổi.

> cs毎にデスキューは設定が必要なのでこちらはcs別に存在する。 → Nhưng giá trị deskew cần set riêng theo từng cs, nên phần deskew vẫn tồn tại tách biệt theo cs.

---

## ## Bảng 4: Bảng Pin chuẩn (datasheet) kèm kết quả đối chiếu số lượng

| Pin Name                                                                                                                                           | I/O | Pin Type  | Power Rail | Description                                                                                                                                                               | Kết quả đối chiếu   |
| -------------------------------------------------------------------------------------------------------------------------------------------------- | --- | --------- | ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------- |
| AP_M_A0_CLKOUT, AP_M_A0_CLKOUTn, AP_M_A1_CLKOUT, AP_M_A1_CLKOUTn, AP_M_B0_CLKOUT, AP_M_B0_CLKOUTn, AP_M_B1_CLKOUT, AP_M_B1_CLKOUTn                 | O   | LVSTL     | AP_VDDO_M  | **Clock (vi sai)**: Mọi tín hiệu address, command, control đều tham chiếu theo xung nhịp này. Mỗi channel (A,B) có 2 cặp xung nhịp.                                       | **8→OK** (đủ)       |
| AP_M_A_CKE[3:0], AP_M_B_CKE[3:0]                                                                                                                   | O   | LVSTL     | AP_VDDO_M  | **Clock Enable**: CKE HIGH kích hoạt, LOW vô hiệu hóa xung nhịp nội bộ, bộ đệm đầu vào, driver đầu ra. Vào/ra chế độ tiết kiệm điện qua CKE. Lấy mẫu tại cạnh lên CLKOUT. | **4→4不足** (thiếu 4) |
| AP_M_A_CS[3:0], AP_M_B_CS[3:0]                                                                                                                     | O   | LVSTL SDR | AP_VDDO_M  | **Chip Select**: CS là 1 phần mã lệnh. Mỗi channel (A,B) có CS riêng.                                                                                                     | **4→4不足** (thiếu 4) |
| AP_M_A_CA[5:0], AP_M_B_CA[5:0]                                                                                                                     | O   | LVSTL     | AP_VDDO_M  | **Command/address inputs**: Cấp lệnh và địa chỉ theo bảng chân lý lệnh. Mỗi channel (A,B) có CA riêng.                                                                    | **12→OK** (đủ)      |
| AP_M_A0_DQ[15:0], AP_M_A1_DQ[15:0], AP_M_B0_DQ[15:0], AP_M_B1_DQ[15:0]                                                                             | I/O | LVSTL DDR | AP_VDDO_M  | **Data**: Bus dữ liệu 2 chiều.                                                                                                                                            | –                   |
| AP_M_A0_DQS[1:0], AP_M_A0_DQSn[1:0], AP_M_A1_DQS[1:0], AP_M_A1_DQSn[1:0], AP_M_B0_DQS[1:0], AP_M_B0_DQSn[1:0], AP_M_B1_DQS[1:0], AP_M_B1_DQSn[1:0] | I/O | LVSTL DDR | AP_VDDO_M  | **Data Strobe (vi sai)**: Tín hiệu 2 chiều dùng để "gõ nhịp" dữ liệu lúc READ/WRITE.                                                                                      | –                   |
| AP_M_A0_DM[1:0], AP_M_A1_DM[1:0], AP_M_B0_DM[1:0], AP_M_B1_DM[1:0]                                                                                 | I/O | LVSTL DDR | AP_VDDO_M  | **Data Mask**: Tín hiệu đa mục đích, chỉ dữ liệu cần che, và dữ liệu đảo trên bus.                                                                                        | –                   |
| AP_M_RESETN                                                                                                                                        | O   | CMOS      | AP_VDDO_M  | **RESET**: Reset không đồng bộ (active low).                                                                                                                              | –                   |

Bảng pin bên dưới cùng (Pin Name / I/O / Pin Type / Power Rail / Description) với các chú thích vàng **"8→OK"**, **"4→4不足"** (x2), **"12→OK"** — nội dung **hoàn toàn giống bảng đã giải thích chi tiết ở lượt trả lời trước** (về CLKOUT, CKE, CS, CA, DQ, DQS, DM, RESETN). Mình không lặp lại toàn bộ diễn giải, chỉ nhắc lại kết luận chính: **CKE và CS mỗi loại thiếu 4 đường so với lý thuyết**, trong khi CLKOUT và CA thì đủ số lượng.

---

## Tổng kết ý nghĩa của trang này so với trang trước

Nếu trang trước là **giai đoạn đặt câu hỏi/nghi vấn** (ký hiệu ch0/ch1 mơ hồ, chưa chắc ch0=device0 hay không, chưa rõ CS/CKE thiếu ở đâu), thì trang này là **giai đoạn xác nhận/chốt kết luận**:

1. **Xác nhận: ch0 = device0(A), ch1 = device1(B)** — không còn là giả thuyết nữa, mà được viết thẳng vào bảng.
2. **Đổi toàn bộ ký hiệu nội bộ (ch0/ch1, pup...) sang đúng tên pin chuẩn** (`A_CKE1`, `A0_CLK`, `B_CS0`...) để bất kỳ ai đọc lại tài liệu sau này cũng tra cứu được trực tiếp với datasheet chính thức, không cần đoán mò như lúc đầu.
3. **Làm rõ khái niệm "dual-channel pair"**: mỗi cặp tín hiệu CS0 (của cả A và B) luôn đi cùng nhau — giúp hiểu rõ hơn kiến trúc vật lý: 2 "channel" A và B thực chất là 2 device DRAM riêng biệt chạy song song để tạo thành bus dữ liệu rộng hơn, còn tín hiệu điều khiển (CLK/CKE/CS/CA) thì có bộ riêng cho mỗi bên nhưng được đồng bộ hoạt động cùng lúc.

Đây là bước chuẩn hóa tài liệu nội bộ quan trọng, giúp cho việc sửa Bug2 ("ch1 không set delay") ở 適用No.1 có cơ sở chắc chắn hơn — vì giờ đây kỹ sư đã biết chính xác ch1 tương ứng với ký hiệu pin nào trên datasheet thật, thay vì phải đoán mò như ban đầu.