# Đặc tả (giả định) register của DDR PHY — Bản dịch + Giải thích chi tiết

Đây là bước tiếp theo trong chuỗi điều tra: sau khi đã xác nhận được kiến trúc vật lý (Quad-Die/Dual-Channel/Dual-Rank) và tên pin chuẩn ở các trang trước, giờ kỹ sư **tổng hợp lại thành 1 giả thuyết hoàn chỉnh về cách tổ chức register bên trong khối DDR PHY** — vẫn là suy đoán ("実装から想定される" = suy đoán từ cách triển khai thực tế), vì tài liệu chính thức không tồn tại.

## Tiêu đề: ■DDR PHYの仕様（レジスタ仕様、実装から想定される）

→ **■ Đặc tả của DDR PHY (đặc tả register, được suy đoán từ cách triển khai thực tế)**

## Các gạch đầu dòng ghi chú (bên trái, trên):

> ・Data用、Control用の2種類がある。 → **Có 2 loại: dùng cho Data (dữ liệu) và dùng cho Control (điều khiển).**

> ・PHY1つあたり11本のIOがある。 → **Mỗi 1 khối PHY có 11 đường IO.**

> ・PHY1つだけではIOが足りないので複数のPHYを使用する。Data/Controlそれぞれに最大16。アクセスの際にはpup番号で指定する。 → **Chỉ 1 khối PHY thì không đủ số IO, nên phải dùng nhiều khối PHY. Data và Control mỗi loại tối đa 16 khối. Khi truy cập, chỉ định bằng số hiệu "pup".**

> ・IOのデスキュー設定ができる。デスキューはIO単位で設定可能。 → **Có thể cài đặt deskew cho IO. Deskew có thể cài đặt theo từng đơn vị IO riêng lẻ.**

> ・各種設定はCS0とCS1で別々に設定できる。恐らくPHYにCSの入力があり、その入力によって使用される設定が切り替わる。 → **Các loại cài đặt có thể set riêng biệt cho CS0 và CS1. Có lẽ PHY có đầu vào nhận tín hiệu CS, và tùy theo đầu vào đó mà cài đặt được sử dụng sẽ chuyển đổi.**

## ■S800の構成 (Cấu hình của S800):

> ・Data用のpupは0~~7(IOは88本)。DQの64本+DMの8本の72本が必要。 → **pup dùng cho Data là từ 0~~7 (tổng 88 đường IO). Cần 72 đường: 64 đường DQ + 8 đường DM.** (Tính: 8 pup × 11 IO/pup = 88 IO có sẵn, nhưng chỉ cần dùng 72 trong số đó cho DQ+DM.)

> ・Control用のpupは0~~3(IOは44本)。Controlは36本必要。 → **pup dùng cho Control là từ 0~~3 (tổng 44 đường IO). Control cần 36 đường.** (Tính: 4 pup × 11 IO/pup = 44 IO có sẵn, chỉ cần dùng 36.)

> →レジスタのダンプをすると4以降はall 0で存在していないと思われるが、実装上は0~~7にアクセスしている。 → **→ Khi dump (đọc toàn bộ) register thì từ pup4 trở đi toàn giá trị 0 — cho thấy có lẽ chúng không tồn tại thật, nhưng trong triển khai (code) lại đang truy cập từ pup 0~~7.**

> 　ch1のディレイが設定できない(設定下もレジスタを読み出すと0)は4~~7にアクセスしているため。 → **→ Việc ch1 không set được delay (dù có set nhưng đọc lại register vẫn ra 0) là do code đang truy cập vào pup 4~~7 (những pup không tồn tại đối với Control).**

**→ Đây chính là bằng chứng cuối cùng, xác nhận dứt điểm nguyên nhân Bug2!** Nhắc lại: Control chỉ có 4 pup thật (0-3), nhưng code phần mềm (dựa theo bảng RevB sai ở các trang trước) lại cố truy cập pup 4-7 cho ch1 → các pup đó không tồn tại về mặt vật lý cho Control → ghi vào "hư không" → đọc lại luôn ra 0. Đây là gốc rễ thực sự của bug "ch1 không set được delay".

---

## Vẽ lại sơ đồ khối: Kết nối giữa Memory Controller và DDR PHY

```
                                              DDR PHY
                          ┌───────────────────────────────────────────┐
        CS[1:0]           │                                           │
メモリ ─────────────────►│                                           │      IO0_D[10:0]        ┌──────────┐
コントローラ              │  ┌─────────────┐                          │      ┌──────────┐       │  CS0     │
(Memory                   │  │  CS0設定     │◄──Read設定──┐           │      │          │───────┤ cs       │
 Controller)  IO[10:0]_H  │  │              │            │           │──────┤          │       │  DQ      │
        ─────────────────►│  │              │──Write設定──┘           │      │          │       └──────────┘
                          │  └─────────────┘                          │      │          │
                          │  ┌─────────────┐                          │      │          │       ┌──────────┐
                          │  │  CS1設定     │◄──Read設定──┐           │      │          │───────┤  CS1     │
                          │  │              │            │           │      │          │       │  cs      │
                          │  │              │──Write設定──┘           │      │          │       │  DQ      │
                          │  └─────────────┘                          │      └──────────┘       └──────────┘
                          │                                           │
                          └───────────────────────────────────────────┘
```

**Giải thích luồng**:

- **Memory Controller** (bộ điều khiển bộ nhớ, nằm trong SoC) gửi 2 loại tín hiệu vào DDR PHY:
    - `CS[1:0]`: chọn xem đang thao tác với **CS0 hay CS1** (rank 0 hay rank 1).
    - `IO[10:0]_H`: dữ liệu điều khiển (giá trị cấu hình) truyền vào PHY.
- Bên trong PHY có **2 khối cài đặt riêng biệt**: **CS0設定** (cài đặt CS0) và **CS1設定** (cài đặt CS1) — mỗi khối lại chia thành 2 hướng:
    - **←Read設定** (cài đặt đọc): giá trị cấu hình dùng khi đang thực hiện lệnh READ.
    - **→Write設定** (cài đặt ghi): giá trị cấu hình dùng khi đang thực hiện lệnh WRITE. → Đây chính là minh chứng cụ thể cho gạch đầu dòng "mỗi loại cài đặt có thể set riêng cho CS0/CS1" đã nêu ở trên — và còn chi tiết hơn: mỗi CS còn tách riêng thành Read/Write.
- Đầu ra của PHY là `IO0_D[10:0]` — kết nối vật lý thực sự ra ngoài tới chip nhớ, tại đó lại phân nhánh theo `CS0`/`CS1` với các đường `cs` (chip select) và `DQ` (dữ liệu) tương ứng.

---

## Danh sách tên các Register (bên trái, dưới sơ đồ)

Đây là **danh sách tên các loại register** mà kỹ sư suy đoán/xác nhận là tồn tại bên trong PHY, dựa theo việc dump thực tế:

|Tên Register (giữ nguyên tiếng Anh vì đây là tên kỹ thuật)|Dịch nghĩa chức năng|
|---|---|
|Write Leveling Register|Register lưu kết quả canh chỉnh thời điểm ghi (Write Leveling)|
|Write Centralization Register|Register lưu kết quả canh giữa (Centralization) cho ghi|
|Read Leveling Register|Register lưu kết quả canh chỉnh thời điểm đọc (Read Leveling)|
|Read Centralization Register|Register lưu kết quả canh giữa cho đọc|
|IOb Write Deskew Register|Register deskew (chỉnh lệch pha) cho ghi, ở mức IOb (1 IO đơn lẻ)|
|LPDDR4 DQDQS Leveling Register|Register leveling giữa DQ và DQS dành riêng cho LPDDR4|
|Write Deskew Broadcast Register|Register deskew cho ghi, kiểu "broadcast" (áp dụng đồng loạt nhiều IO cùng lúc)|
|IOb Read Deskew Register|Register deskew cho đọc, ở mức IOb|
|Read Deskew Broadcast Register|Register deskew cho đọc, kiểu broadcast|

> Giải thích thêm: "Leveling" là quá trình canh chỉnh thời điểm tổng thể (thô), còn "Deskew" là tinh chỉnh lệch pha ở mức chi tiết hơn cho từng IO riêng lẻ; "Centralization" là canh cho tín hiệu nằm ở giữa "cửa sổ an toàn" (eye window) thay vì lệch về 1 phía. "Broadcast" nghĩa là 1 lệnh ghi có thể áp dụng cùng lúc cho nhiều IO thay vì phải ghi từng IO một — đây có thể chính là loại register liên quan trực tiếp đến bug "tối ưu ghi register" đã nhắc ở Bug1 của 適用No.1 (logic chỉ ghi khi giá trị thay đổi so với lần trước).

---

## Bảng Pin (bên phải) — đối chiếu số lượng pin với số lượng pup×IO

Bảng này **giống với bảng pin chuẩn đã thấy ở các trang trước**, nhưng giờ có thêm các ô chú thích màu vàng **tính toán ngược lại xem con số pin thực tế có khớp với giả thuyết "pup × IO" hay không**:

|Pin Name|I/O|Pin Type|Power Rail|Description|Tính toán đối chiếu|
|---|---|---|---|---|---|
|`AP_M_A0_CLKOUT`, `..CLKOUTn`, `AP_M_A1_CLKOUT`, `..CLKOUTn`, `AP_M_B0_CLKOUT`, `..CLKOUTn`, `AP_M_B1_CLKOUT`, `..CLKOUTn`|O|LVSTL|AP_VDDO_M|Clock (differential)...|**Control 8本** (Control 8 đường)|
|`AP_M_A_CKE[3:0]`, `AP_M_B_CKE[3:0]`|O|LVSTL SDR|AP_VDDO_M|Clock Enable...|**Control 8本 = 4×2** (4 đường/channel × 2 channel)|
|`AP_M_A_CS[3:0]`, `AP_M_B_CS[3:0]`|O|...|AP_VDDO_M|Chip Select...|**Control 8本 = 4×2**|
|`AP_M_A_CA[5:0]`, `AP_M_B_CA[5:0]`|O|...|AP_VDDO_M|Command/address...|**Control 12本 = 6×2** (6 đường/channel × 2 channel)|
|`AP_M_A0_DQ[15:0]`, `AP_M_A1_DQ[15:0]`, `AP_M_B0_DQ[15:0]`, `AP_M_B1_DQ[15:0]`|I/O|LVSTL DDR|AP_VDDO_M|Data...|**data 64本 = 16×4** (16 đường × 4 nhóm A0/A1/B0/B1)|
|`AP_M_A0_DQS[1:0]`, `..DQSn[1:0]`, `AP_M_A1_DQS[1:0]`, `..DQSn[1:0]`, `AP_M_B0_DQS[1:0]`, `..DQSn[1:0]`, `AP_M_B1_DQS[1:0]`, `..DQSn[1:0]`|I/O|...|AP_VDDO_M|Data Strobe...|**"data だけど(汎用)IOではなく、専用のIOがあると思われる"**|
|`AP_M_A0_DM[1:0]`, `AP_M_A1_DM[1:0]`, `AP_M_B0_DM[1:0]`, `AP_M_B1_DM[1:0]`|I/O|...|AP_VDDO_M|Data Mask...|**data 8本 = 2×4** (2 đường × 4 nhóm)|
|`AP_M_RESETN`|O|CMOS|AP_VDDO_M|RESET...|(không có chú thích)|

### Giải thích ý nghĩa từng chú thích tính toán:

- **"Control 8本"** (dòng CLKOUT): đếm được 8 đường CLKOUT → xếp vào nhóm **Control** (vì CLK là tín hiệu điều khiển, không phải data).
- **"Control 8本 = 4×2"** (CKE và CS): công thức `[3:0]` = 4 bit/đường mỗi channel, nhân với 2 channel (A, B) = 8 → khớp với số đường CKE/CS thực tế đếm được.
- **"Control 12本 = 6×2"** (CA): `[5:0]` = 6 đường/channel × 2 channel = 12.
- **"data 64本 = 16×4"** (DQ): `[15:0]` = 16 đường mỗi nhóm, có 4 nhóm (A0, A1, B0, B1 — ứng với 4 die trong kiến trúc Quad-Die đã học ở trang trước!) → 16×4=64.
- **"data だけど(汎用)IOではなく、専用のIOがあると思われる"** (DQS): dịch là **"Tuy là loại data, nhưng có lẽ đây không dùng IO (chân) đa dụng (chung), mà có IO chuyên dụng riêng."** — ý là: DQS không nằm trong nhóm "pup Data dùng chung" như DQ/DM, mà có thể có 1 mạch/pad vật lý riêng biệt chuyên trách cho tín hiệu strobe này (vì DQS đóng vai trò đặc biệt — nó là tín hiệu "gõ nhịp" chứ không phải dữ liệu thô, nên về mặt thiết kế PHY thường được xử lý theo mạch riêng, tách khỏi các pad DQ thông thường).
- **"data 8本 = 2×4"** (DM): `[1:0]` = 2 đường/nhóm × 4 nhóm = 8 → khớp với con số "DM 8 bản" đã nêu ở gạch đầu dòng "72本 = 64本(DQ) + 8本(DM)" phía trên.

---

## Tổng kết dễ hiểu toàn bộ trang này

Đây là **bước tổng hợp cuối cùng** của toàn bộ quá trình điều tra DDR PHY (đã trải qua nhiều trang trước: từ chưa biết gì → đoán ch0/ch1 → xác nhận A/B → xác nhận kiến trúc Quad-Die → và giờ là **kết luận trực tiếp nguyên nhân bug**):

1. Đội kỹ sư xác định được cấu trúc PHY có 2 loại pup (Data và Control), mỗi pup gồm 11 đường IO, và số lượng pup thực tế cần dùng: **Data cần 8 pup (0-7)**, **Control chỉ cần 4 pup (0-3)**.
2. Họ phát hiện bằng chứng trực tiếp: khi dump toàn bộ register, **pup Control từ số 4 trở đi luôn ra giá trị 0** — nghĩa là những pup này **không tồn tại thật** đối với khối Control.
3. Nhưng đoạn code phần mềm xử lý delay cho **ch1** lại đang cố truy cập pup 4~7 (do bảng tài liệu RevB sai từ trước, đã phân tích ở phần trước) → ghi vào chỗ không tồn tại → **đây chính là nguyên nhân gốc rễ 100% khớp với hiện tượng Bug2** ("ch1 không set được delay, đọc lại register luôn ra 0") ở 適用No.1.
4. Sơ đồ khối và bảng pin ở cuối trang giúp xác nhận lại toàn bộ các con số (số lượng đường CLK/CKE/CS/CA/DQ/DM) đều khớp đúng với giả thuyết cấu trúc "PHY chia theo pup, mỗi pup 11 IO" mà họ vừa xây dựng — củng cố độ tin cậy cho toàn bộ chuỗi suy luận này, dù đây vẫn chỉ là "suy đoán từ thực nghiệm" (vì tài liệu chính thức của register DDR PHY chưa bao giờ được tìm thấy, như đã nêu rất rõ ngay từ 適用No.1).