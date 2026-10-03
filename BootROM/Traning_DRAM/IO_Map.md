# Bảng "Điều tra IO MAP / Register DDR PHY" — Bản dịch + Giải thích chi tiết

Đây chính là nội dung của tab **"DDR PHYレジスタ調査" (Điều tra register DDR PHY)** được nhắc đến ở cuối sheet 適用No.1 (Bug2 — "ch1 không set được delay"). Đây là **ghi chú điều tra thực địa** của kỹ sư (không phải tài liệu chính thức), nên có nhiều câu hỏi, dấu "?", và suy đoán — mình sẽ dịch sát nghĩa và giải thích để bạn hiểu vì sao họ đặt câu hỏi đó.

## Kiến thức nền cần biết trước

- **CS (Chip Select)**: tín hiệu "chọn chip" — với LPDDR4, có thể có nhiều "die" (lát bán dẫn) xếp chồng trong 1 gói chip, CS dùng để chọn đúng die nào đang được truy cập.
- **CKE (Clock Enable)**: tín hiệu bật/tắt xung nhịp nội bộ của chip nhớ — dùng để đưa chip vào/ra chế độ tiết kiệm điện.
- **CA (Command/Address)**: các đường tín hiệu mang lệnh và địa chỉ.
- **CLK / CLKOUT**: xung nhịp chủ, mọi tín hiệu CA/CS/CKE đều được "chốt" theo cạnh của xung nhịp này.
- **DQ**: đường dữ liệu (Data) — nơi dữ liệu thực sự chạy qua lại.
- **DQS**: "Data Strobe" — tín hiệu đồng bộ dùng để báo hiệu "lúc nào thì dữ liệu trên DQ là hợp lệ để đọc/ghi" (giống như 1 nhịp gõ nhịp riêng cho dữ liệu, tách biệt với CLK chính).
- **DM (Data Mask)**: tín hiệu che dữ liệu — dùng khi chỉ muốn ghi 1 phần byte, không ghi đè toàn bộ.
- **pup**: viết tắt có thể hiểu là "PHY Unit/Pad" — 1 đơn vị vật lý bên trong khối DDR PHY (mạch điều khiển tín hiệu tương tự nối ra chân chip), mỗi pup đảm nhiệm 1 nhóm nhỏ các đường tín hiệu.
- **die**: 1 lát chip bán dẫn độc lập bên trong 1 gói vỏ chip — LPDDR4 dung lượng lớn có thể xếp chồng (stack) nhiều die trong cùng 1 gói.
- **device0 / device1**: đây có thể là 2 chip nhớ vật lý riêng biệt được hàn trên board (khác với "ch0/ch1" là 2 kênh điều khiển của bộ nhớ trong SoC).
- **deskew**: quá trình chỉnh lệch pha (giống với "delay" đã nói ở các phần No.1, No.5 trước) để các tín hiệu đến đúng lúc.
- **RevA / RevB**: 2 phiên bản khác nhau của tài liệu/thiết kế (Revision A và Revision B) — có thể RevB là bản cập nhật/sửa lỗi so với RevA.

---

## Bảng 1: Control IO MAP (RevA)

Đây là bảng ánh xạ: "tại vị trí pup số mấy, slot số mấy (0-3), thì tín hiệu điều khiển nào (CLK/CKE/CS/CA) được gán vào đó" — dành cho các tín hiệu **điều khiển** (Control: CLK, CKE, CS, CA), không phải dữ liệu.

```
                pup→   0        1        2        3        4              5        6        7        8        9        10
cs0   0     ch0_CKE1    -        -        -    ch0_CK0n,ch0_CK0  ch1_CKE1  ch0_CA3  ch1_CKE0    -        -     ch0_CA5
      1     ch0_CKE0  ch0_CA1  ch0_CA0  ch0_CS0  ch1_CK0n,ch1_CK0  ch0_CA4  ch1_CA4  ch0_CA2  ch1_CA5  ch1_CA3     -
      2         -        -    ch1_CK1n    -       ch0_CK1        ch0_CK1n  ch1_CK1     -        -        -        -
      3     ch1_CS1     -    ch1_CA1  ch0_CS1        -           ch1_CA2  ch1_CA0     -        -        -     ch1_CS0
cs1                    "cs0と同じ（デスキュー値もcs0と同じものがセットされる）"
                        → Giống hệt cs0 (giá trị deskew cũng được set giống như cs0)
```

**Giải thích cách đọc bảng**: mỗi cột là 1 "pup" (đơn vị pad vật lý, đánh số 0 đến 10 — tổng cộng 11 pup), mỗi cột chia thành 4 hàng con (0,1,2,3) là 4 "khe" tín hiệu trong pup đó. Nội dung mỗi ô cho biết tín hiệu logic nào (ví dụ `ch0 CKE1`, `ch1 CA3`...) đang được gán vào khe vật lý đó. Hàng `cs1` không liệt kê lại chi tiết vì giống hệt `cs0`.

### Ghi chú của kỹ sư bên cạnh bảng RevA:

> ・デバイス毎の設定は存在しない。ch毎には存在する。 → **Không tồn tại setting riêng theo từng "device". Chỉ tồn tại setting riêng theo từng "ch" (channel).**

> →デバイス毎に存在しないとそもそも結線ができないだろう。 → **→ Nếu không có setting riêng theo device thì ngay từ đầu việc đấu nối (wiring) vật lý sẽ không thực hiện được.** (Ý là: nếu phần cứng có 2 chip vật lý riêng (device0, device1) nhưng phần mềm chỉ set theo channel, thì lý thuyết ra sẽ có vấn đề — trừ khi có quy ước ngầm nào đó.)

> →ch0をdevice0用、ch1をdevice1用としている？ → **→ Có phải là ch0 dùng cho device0, ch1 dùng cho device1?** (đây là 1 giả thuyết đặt ra, có dấu "?" thể hiện chưa chắc chắn — họ đang suy đoán quy ước gán ghép)

> ・CSとCKEが足りない。 → **Số lượng tín hiệu CS và CKE là không đủ.** (Nhìn vào bảng: đếm số ô có "CS" và "CKE" xuất hiện thì thấy ít hơn số lượng cần thiết theo lý thuyết — đây chính là điều được xác nhận lại ở bảng pin datasheet bên phải, mục "4→4不足".)

> →実際には空いているところに結線されている？ → **→ Có phải trên thực tế chúng được đấu nối vào những chỗ còn trống (không được liệt kê rõ trong tài liệu)?** (Một giả thuyết nữa: có thể tài liệu/bảng này không đầy đủ, và trên thực tế phần cứng vẫn có đủ đường CS/CKE nhưng nằm ở vị trí không được ghi chú công khai.)

---

## Bảng 2: Control IO MAP (RevB)

Đây là phiên bản **RevB**, khác RevA ở chỗ **mở rộng lên 8 hàng con (0-7)** thay vì chỉ 4 hàng (0-3), và có vẻ cách sắp xếp `ch1` bị dịch chuyển sang các pup phía sau:

```
                pup→   0        1        2        3        4        5        6        7        8        9        10
cs0   0     ch0_CKE1    -        -        -        -        -    ch0_CK0n,ch0_CK0  ch0_CA3    -        -     ch0_CA5
      1     ch0_CKE0  ch0_CA1  ch0_CA0  ch0_CS0    -        -       -           ch0_CA4    -        -        -
      2         -        -        -        -    ch0_CK1  ch0_CK1n     -            -        -        -        -
      3         -        -        -        -    ch0_CS1     -         -            -        -        -        -
      4         -        -        -        -        -        -    ch1_CKE1        -        -        -        -
      5         -        -        -        -        -        -    ch1_CK0n,ch1_CK0  ch1_CA4    -        -        -
      6         -        -    ch1_CK1n    -        -        -       -           ch1_CA5    -        -        -
      7     ch1_CS1     -    ch1_CA1  ch0(?)_CA0/ch1_CA2    -        -           -            -        -        -     ch1_CS0
cs1                    "cs0と同じ（デスキュー値もcs0と同じものがセットされる）"
                        → Giống hệt cs0
```

(Lưu ý: bảng gốc có 1 số ô bị dồn/khó tách bạch trong ảnh gốc — ví dụ hàng 7 có "ch1 CA1", "ch1 CA2", "ch1 CA0" nằm gần nhau; đây là hạn chế khi đọc từ ảnh chụp bảng tính, không ảnh hưởng đến kết luận chính mà kỹ sư rút ra bên dưới.)

### Ghi chú của kỹ sư bên cạnh bảng RevB:

> ch1がpup4以降に移っている → **Ch1 đã bị dịch chuyển sang từ pup4 trở đi.** (Ở RevA, ch0 và ch1 đan xen nhau trong cùng các pup 0-4; nhưng ở RevB thì có vẻ ch0 chiếm hết pup 0-3, còn ch1 bắt đầu từ pup4.)

> 実際にはpup=4〜7は存在しないのでこれは間違い → **Nhưng thực tế thì pup=4~7 không tồn tại (trên phần cứng thật), nên đây là một sai sót (lỗi).** (Đây là phát hiện quan trọng: theo hiểu biết của kỹ sư về thiết kế phần cứng thực tế, chỉ có 4 pup (0-3) tồn tại vật lý — nhưng bảng RevB lại tham chiếu đến pup 4, 5, 6, 7, tức là những vị trí không hề có trên chip thật.)

> 可能性としてはSoCのバグにより存在しているがレジスタが見えないかも知れない。 → **Một khả năng là: do bug của SoC, các pup đó có tồn tại nhưng register (thanh ghi điều khiển) lại không hiển thị/không truy cập được.** (Một giả thuyết mở, không chắc chắn — có thể phần cứng thực ra có 8 pup nhưng do lỗi thiết kế mà chỉ 4 pup có thể điều khiển qua register.)

> →pupが4個でIOは足りておりpupを8つにする必要がない。実装の間違いの可能性が高い。 → **→ Với chỉ 4 pup thì số lượng IO (chân tín hiệu) đã là đủ rồi, không cần thiết phải có tới 8 pup. Khả năng cao đây là một lỗi khi triển khai (implementation).** (Đây là kết luận cuối cùng của kỹ sư: thay vì tin giả thuyết "bug SoC ẩn giấu 4 pup", họ nghiêng về khả năng đơn giản hơn — bảng RevB này bị viết sai/triển khai sai, vì về mặt logic 4 pup là đã đủ đáp ứng số lượng tín hiệu cần thiết, không có lý do kỹ thuật nào để mở rộng ra 8 pup.)

**→ Đây chính là mấu chốt liên quan đến Bug2 ở 適用No.1**: "ch1 không set được delay" — nếu code phần mềm (hàm `applyCsDlys()`/`applyCaDlys()`) được viết dựa theo bảng RevB sai này (nghĩ rằng ch1 nằm ở pup 4-7), trong khi phần cứng thực tế chỉ có 4 pup (0-3, với ch0/ch1 đan xen như RevA), thì phần mềm sẽ ghi nhầm địa chỉ register — khớp chính xác với hiện tượng đã mô tả: "đọc lại thì ch0 đúng giá trị, ch1 luôn ra 0" vì lệnh ghi cho ch1 đã bị gửi tới 1 vị trí pup không tồn tại.

---

## Bảng 3: Data IO MAP (RevB)

Đây là bảng tương tự nhưng dành cho tín hiệu **dữ liệu** (Data: DQ, DM) thay vì tín hiệu điều khiển:

```
                pup→   0     1     2     3     4    5    6      7     8     9    10
cs0   0   ch0  DQ0   DQ7   DQ2   DM    -    -   DQ3   DQ1   DQ4   DQ6   DQ5
      1        DQ11  DQ10  DQ9         -    -   DQ8   DQ15  DQ12  DQ13  DQ14
      2        DQ6   DQ4   DQ3         -    -   DQ5   DQ0   DQ7   DQ2   DQ1
      3        DQ14  DQ12  DQ13        -    -   DQ11  DQ9   DQ15  DQ10  DQ0(?)
      4   ch1  DQ0   DQ2   DQ13  DM    -    -   DQ3   DQ4   DQ5   DQ6   DQ7
      5        DQ8   DQ10  DQ9         -    -   DQ11  DQ12  DQ13  DQ14  DQ15
      6        DQ0   DQ2   DQ3         -    -   DQ1   DQ4   DQ5   DQ6   DQ7
      7        DQ8   DQ9   DQ10        -    -   DQ11  DQ12  DQ13  DQ14  DQ15
cs1   0   ch0  DQ0   DQ7   DQ2   DM    -    -   DQ3   DQ1   DQ4   DQ6   DQ5
      1        DQ11  DQ10  DQ9         -    -   DQ8   DQ15  DQ12  DQ13  DQ14
      2        DQ6   DQ4   DQ3         -    -   DQ5   DQ0   DQ7   DQ2   DQ1
      3        DQ14  DQ12  DQ13        -    -   DQ11  DQ9   DQ15  DQ10  DQ0(?)
```

**Nhận xét quan trọng**: nếu để ý kỹ, phần **cs1 lặp lại y hệt phần đầu của cs0** (4 hàng đầu, ch0) — điều này khớp với ghi chú bên dưới.

### Ghi chú:

> csが変わっても使用するDQは同じなのでIO MAPとしては同じになる。 → **Dù cs thay đổi (từ cs0 sang cs1) thì đường DQ được sử dụng vẫn là như nhau, nên xét về mặt bản đồ IO (IO MAP) thì nó giống hệt nhau.** (Lý do dễ hiểu: DQ là đường dữ liệu vật lý cố định, không phụ thuộc vào việc đang chọn die/chip nào qua CS — chỉ có delay/deskew là cần chỉnh khác nhau.)

> cs毎にデスキューは設定が必要なのでこちらはcs別に存在する。 → **Tuy nhiên, vì giá trị deskew (chỉnh lệch pha) cần được set riêng theo từng cs, nên phần cài đặt deskew này vẫn tồn tại tách biệt theo từng cs** (dù bản đồ IO thì giống nhau).

---

## Bảng giữa: Ánh xạ device / cs / die → tín hiệu CS vật lý

```
デバイス内 (Bên trong device)              CS信号 (Tín hiệu CS)
─────────────────────────────────────────────────────
device0   cs0   die0    ─┐
device1   cs0   die0    ─┼──►  AP_M_A_CS0 / AP_M_B_CS0
device0   cs0   die1    ─┤
device1   cs0   die1    ─┘

device0   cs1   die2    ─┐
device1   cs1   die2    ─┼──►  AP_M_A_CS1 / AP_M_B_CS1
device0   cs1   die3    ─┤
device1   cs1   die3    ─┘
```

**Giải thích**: bảng này cho thấy cách 1 tín hiệu CS vật lý (ví dụ `AP_M_A_CS0`) thực ra được **dùng chung cho nhiều die** (die0 và die1 cùng dùng chung `AP_M_A_CS0`/cs0). Tức là: mỗi tín hiệu CS0/CS1 vật lý không chỉ chọn 1 chip mà còn phải kết hợp thêm thông tin khác (có thể là địa chỉ hoặc timing) để phân biệt đâu là die0 hay die1 bên trong. Đây là cách các nhà sản xuất tiết kiệm số chân vật lý khi phải hỗ trợ chip nhớ dung lượng lớn (nhiều die xếp chồng) mà không cần tăng số lượng đường CS.

- `AP_M_A_...` = tín hiệu dành cho **channel/device A** (có thể là "ch0" hoặc "device0" tùy theo quy ước — đây chính là 1 phần thông tin giúp trả lời câu hỏi "ch0 = device0?" đặt ra ở ghi chú bảng đầu).
- `AP_M_B_...` = tín hiệu dành cho **channel/device B**.

---

## Bảng 4: Data IO MAP (RevA)

```
                pup→   0     1     2     3     4    5    6      7     8     9    10
cs0   0   ch0  DQ0   DQ7   DQ2   DM    -    -   DQ3   DQ1   DQ4   DQ6   DQ5
      1        DQ11  DQ10  DQ9         -    -   DQ8   DQ15  DQ12  DQ13  DQ14
      2        DQ6   DQ4   DQ3         -    -   DQ5   DQ0   DQ7   DQ2   DQ1
      3        DQ14  DQ12  DQ13        -    -   DQ11  DQ9   DQ15  DQ10  DQ0(?)
cs1   0   ch0  DQ0   DQ7   DQ2   DM    -    -   DQ3   DQ1   DQ4   DQ6   DQ5
      1        DQ11  DQ10  DQ9         -    -   DQ8   DQ15  DQ12  DQ13  DQ14
      2        DQ6   DQ4   DQ3         -    -   DQ5   DQ0   DQ7   DQ2   DQ1
      3        DQ14  DQ12  DQ13        -    -   DQ11  DQ9   DQ15  DQ10  DQ0(?)
```

So với bảng Data IO MAP (RevB) ở trên, phiên bản RevA này **chỉ có 4 hàng con (0-3)** cho mỗi cs, không có phần "ch1" mở rộng ở hàng 4-7.

### Ghi chú:

> device0とdevice1でDQを共有する構成と思われる。 → **Đây được cho là cấu hình mà device0 và device1 chia sẻ chung đường DQ với nhau.** (Giải thích thêm ý này: khác với CS/CKE/CA là mỗi device có đường riêng, đường dữ liệu DQ vật lý có thể được đấu nối dùng chung giữa 2 device — đây cũng là lý do tại sao bảng Data IO MAP không cần phân biệt "device" như bảng Control IO MAP.)

---

## Bảng bên phải: Bảng Pin tra từ tài liệu Datasheet (đối chiếu số lượng chân thực tế)

Đây là bảng trích từ tài liệu spec chính thức (datasheet của IP/chip), dùng để **đối chiếu, kiểm chứng lại** số lượng chân tín hiệu thực tế có bao nhiêu — nhằm trả lời câu hỏi "CS và CKE có thiếu không" đặt ra ở ghi chú bảng đầu.

|Pin Name (Tên chân)|I/O|Pin Type|Power Rail|Description (Mô tả)|
|---|---|---|---|---|
|`AP_M_A0_CLKOUT`, `AP_M_A0_CLKOUTn`, `AP_M_A1_CLKOUT`, `AP_M_A1_CLKOUTn`, `AP_M_B0_CLKOUT`, `AP_M_B0_CLKOUTn`, `AP_M_B1_CLKOUT`, `AP_M_B1_CLKOUTn`|O (Output)|LVSTL|AP_VDDO_M|**Clock (vi sai)**: Mọi tín hiệu address, command và control đều được tham chiếu theo xung nhịp này. Mỗi channel (A, B) có 2 cặp xung nhịp.|
|`AP_M_A_CKE[3:0]`, `AP_M_B_CKE[3:0]`|O|LVSTL|AP_VDDO_M|**Clock Enable**: CKE mức HIGH kích hoạt, mức LOW vô hiệu hóa xung nhịp nội bộ, bộ đệm đầu vào, và driver đầu ra. Chế độ tiết kiệm điện được vào/ra thông qua chuyển trạng thái CKE. CKE được lấy mẫu tại cạnh lên của CLKOUT.|
|`AP_M_A_CS[3:0]`, `AP_M_B_CS[3:0]`|O|LVSTL SDR|AP_VDDO_M|**Chip Select**: CS là 1 phần của mã lệnh (command code). Mỗi channel (A, B) có tín hiệu CS riêng của mình.|
|`AP_M_A_CA[5:0]`, `AP_M_B_CA[5:0]`|O|LVSTL|AP_VDDO_M|**Command/address inputs**: Cung cấp lệnh và địa chỉ theo bảng chân lý lệnh (command truth table). Mỗi channel (A, B) có tín hiệu CA riêng của mình.|
|`AP_M_A0_DQ[15:0]`, `AP_M_A1_DQ[15:0]`, `AP_M_B0_DQ[15:0]`, `AP_M_B1_DQ[15:0]`|I/O|LVSTL DDR|AP_VDDO_M|**Data**: Bus dữ liệu 2 chiều.|
|`AP_M_A0_DQS[1:0]`, `AP_M_A0_DQSn[1:0]`, `AP_M_A1_DQS[1:0]`, `AP_M_A1_DQSn[1:0]`, `AP_M_B0_DQS[1:0]`, `AP_M_B0_DQSn[1:0]`, `AP_M_B1_DQS[1:0]`, `AP_M_B1_DQSn[1:0]`|I/O|LVSTL DDR|AP_VDDO_M|**Data Strobe (vi sai)**: Tín hiệu 2 chiều dùng để "gõ nhịp" dữ liệu trong lúc READ hoặc WRITE.|
|`AP_M_A0_DM[1:0]`, `AP_M_A1_DM[1:0]`, `AP_M_B0_DM[1:0]`, `AP_M_B1_DM[1:0]`|I/O|LVSTL DDR|AP_VDDO_M|**Data Mask**: Tín hiệu 2 chiều dùng đa mục đích, chỉ ra dữ liệu nào cần bị che (mask), và dữ liệu nào bị đảo trên bus.|
|`AP_M_RESETN`|O|CMOS|AP_VDDO_M|**RESET**: Reset không đồng bộ (kích hoạt ở mức thấp - active low).|

### Các ô chú thích màu vàng (kết quả đối chiếu số lượng chân của kỹ sư):

- Cạnh dòng **CLKOUT**: **"8→OK"** → Đếm được 8 chân CLKOUT (4 cặp vi sai: A0, A1, B0, B1) → **khớp đủ, không thiếu** (OK).
- Cạnh dòng **CKE[3:0]**: **"4→4不足"** → Với công thức `[3:0]` nghĩa là 4 bit/4 đường CKE cho mỗi channel, tổng 2 channel (A,B) x 4 = 8 đường kỳ vọng, nhưng thực tế đếm/đối chiếu ra chỉ có 4 → **thiếu mất 4 đường CKE**.
- Cạnh dòng **CS[3:0]**: **"4→4不足"** → Tương tự, thiếu mất 4 đường CS.
- Cạnh dòng **CA[5:0]**: **"12→OK"** → `[5:0]` = 6 đường CA/channel x 2 channel (A,B) = 12 đường → đếm đủ 12 → **OK, không thiếu**.

**→ Đây chính là bằng chứng xác nhận trực tiếp cho ghi chú "CSとCKEが足りない" (CS và CKE không đủ) ở đầu trang!** Kỹ sư đã tự tay đếm lại theo tài liệu datasheet chính thức và xác nhận: đúng là số lượng chân CS và CKE thực tế trên phần cứng ít hơn một nửa so với những gì lẽ ra cần có nếu tính đầy đủ theo công thức "mỗi channel x mỗi bit". Đây là cơ sở dẫn tới giả thuyết "có thể chúng được đấu nối vào chỗ trống nào đó không được ghi chép rõ ràng" như đã nêu ở ghi chú đầu bảng.

_(Lưu ý: trong ảnh gốc có 1 dòng chữ mờ chạy chéo qua bảng bên phải, dạng watermark như "DELL.COM/BUYDELL... UNDER NDA..." — đây chỉ là watermark bảo mật tài liệu (đánh dấu tài liệu thuộc diện NDA - thỏa thuận bảo mật), không phải nội dung kỹ thuật, nên mình không dịch/diễn giải phần đó.)_

---

## Tổng kết dễ hiểu toàn bộ trang này

Đây là **nhật ký điều tra thực địa** của kỹ sư khi cố gắng tìm hiểu vì sao **"ch1 không set được delay"** (Bug2, đã nêu ở 適用No.1). Vì tài liệu chính thức về register DDR PHY của chip S800 không tồn tại (như đã nói ở phần trước — họ tìm khắp nơi: IP list, ARMADA 8K, GitHub Marvell... đều không có), họ phải **tự dựng lại bản đồ ánh xạ pin/pad** (IO MAP) bằng cách so sánh 2 phiên bản tài liệu nội bộ (RevA và RevB) với nhau, và đối chiếu ngược lại với 1 bảng pin chuẩn lấy từ datasheet của IP/nhà cung cấp chip.

Kết quả điều tra hé lộ 2 manh mối quan trọng:

1. **Bảng RevB có khả năng bị lỗi triển khai**: nó giả định tồn tại 8 "pup" (0-7) trong khi phần cứng thật chỉ có 4 pup, và ch1 trong RevB bị đặt vào các pup không tồn tại (4-7) — đây là ứng viên hàng đầu để giải thích tại sao code phần mềm viết theo tài liệu này lại set sai địa chỉ, dẫn đến hiện tượng ch1 luôn đọc ra giá trị 0.
2. **Thiếu tín hiệu CS/CKE so với lý thuyết** (chỉ có 4/8 đường mỗi loại) — cho thấy tài liệu thiết kế chưa hoàn chỉnh 100%, một số đường tín hiệu có thể được đấu nối theo cách không chuẩn/không ghi chép đầy đủ, khiến việc suy luận địa chỉ register chính xác cho ch1 càng thêm khó khăn — đây cũng lý giải vì sao ở phần cuối 適用No.1 kỹ sư kết luận "bản sửa dựa trên phỏng đoán, chưa chắc chắn 100% vì không rõ IO mapping thật sự".