# 適用No.5 — Thêm điều khiển cố định delay CLK (Bản dịch + Giải thích chi tiết)

Trước khi vào bảng, mình giải thích nhanh vài khái niệm nền để bạn không bị "lạc" giữa các thuật ngữ:

## Kiến thức nền cần biết trước

- **LPDDR4** là loại RAM tốc độ cao. Để CPU (ở đây là chip S800) nói chuyện được với RAM, mỗi tín hiệu điện (CLK - xung nhịp, CA - lệnh/địa chỉ, CS - chọn chip, DQ - dữ liệu) phải "đến nơi" đúng thời điểm, sai lệch tính bằng phần nghìn tỷ giây (picosecond). Nếu tín hiệu đến sớm/muộn quá thì RAM đọc sai bit → máy không boot lên được hoặc treo máy.
- **Delay (độ trễ)** là cơ chế "nắn" thời điểm tín hiệu đến, giống như vặn một cái núm chỉnh độ trễ nhỏ cho mỗi đường dây tín hiệu, để tất cả các đường "gặp nhau" đúng lúc tại chân RAM.
- **Training (huấn luyện)** là quá trình mà BOOT firmware tự động dò từng bước, thử các giá trị delay khác nhau, đo xem giá trị nào cho tín hiệu "sạch" nhất, rồi lưu lại giá trị delay tối ưu đó. Có nhiều loại training theo từng nhóm tín hiệu: **CS training** (delay cho chân chọn chip), **CA training** (delay cho chân lệnh/địa chỉ), **DQ training** (delay cho chân dữ liệu).
- **Margin** (biên độ an toàn) là khoảng "dư" giữa thời điểm tín hiệu thực tế đến và ranh giới bắt đầu lỗi. Margin càng lớn thì càng ổn định, ít bị lỗi do nhiễu hay biến động nhiệt độ/điện áp.

Vấn đề nêu ra ở đây: qua đo đạc dạng sóng thực tế, các kỹ sư thấy **margin giữa CLK và CA quá hẹp** — nghĩa là cơ chế training tự động hiện tại chỉnh delay chưa đủ tối ưu, có nguy cơ chạy sai ở điều kiện biên (nhiệt độ, điện áp dao động). Giải pháp: cho phép **ép cứng (force)** một giá trị delay CLK/CA cụ thể do người làm phần cứng đo đạc & chọn sẵn, ghi đè lên kết quả mà thuật toán training tự động tính ra.

---

## Tiêu đề: 適用No.5　CLKディレイの固定制御の追加

→ **Áp dụng No.5 — Thêm cơ chế điều khiển cố định (ép cứng) delay của CLK**

---

## Phần 1: ・仕様追加 (Bổ sung đặc tả) — So sánh luồng training TRƯỚC và SAU

### ■現行のトレーニングフロー (Luồng training hiện hành — TRƯỚC khi sửa)

1. **・CSトレーニング** (CS training) 　CLKディレーまたはCSディレーを調整する。 → Điều chỉnh delay của CLK hoặc delay của CS.
2. **・CAトレーニング** (CA training) 　CAディレーを調整する。 → Điều chỉnh delay của CA.
3. **・DQトレーニング** (DQ training) — chạy tiếp theo như bình thường, không có bước nào khác.

### ■改版後のトレーニングフロー (Luồng training SAU khi sửa)

1. **・CSトレーニング** — CLKディレーまたはCSディレーを調整する。 (giống hệt trước, không đổi)
2. **・CAトレーニング** — CAディレーを調整する。 (giống hệt trước, không đổi)
3. **Ô tô màu vàng — bước MỚI thêm vào:** **・データの置き換え（有効時）** → **Thay thế dữ liệu (khi tính năng được bật)**
    - CLKディレーに任意値をセット → Set giá trị tùy ý (do người dùng định trước) vào delay của CLK
    - CAディレーに任意値をセット → Set giá trị tùy ý vào delay của CA
    - CSディレーに0x00をセット → Set delay của CS về 0x00
4. **・DQトレーニング以降は従来通り** → Từ bước DQ training trở đi, vẫn giữ nguyên như cũ.

**Ý nghĩa**: Sau khi CS training và CA training tự động chạy xong (để không phá vỡ logic cũ), hệ thống sẽ **ghi đè** kết quả CLK/CA delay bằng giá trị cố định người dùng đã nhập sẵn trong tham số (nếu tính năng này được bật) — coi như "sửa tay" sau khi máy tự động dò xong, để đảm bảo margin tốt hơn giá trị tự động tính ra.

---

## Phần 2: ・パラメータの拡張 (Mở rộng tham số)

> FORCE_TRAIN_DLY_ENにて固定ディレイの有効/無効を制御する。 → Dùng bit **FORCE_TRAIN_DLY_EN** để điều khiển bật/tắt (có hiệu lực hay không) cơ chế delay cố định.

> CADLY、CLKDLYの設定値をトレーニングのパラメーターから反映させる。 → Giá trị cài đặt của **CADLY** (delay CA) và **CLKDLY** (delay CLK) sẽ được lấy trực tiếp từ tham số training (do người làm phần cứng nhập sẵn trong file tham số nạp cho BOOT).

### Bảng register mới:

|Địa chỉ|Tên register|Vai trò|Ghi chú (vị trí bit)|Giá trị khởi tạo|
|---|---|---|---|---|
|`0x0128`|CH0_FORCE_TRAIN_DLY|Giá trị delay cố định dùng sau CA training của **CH0**.<br>**CLKDLY**: giá trị cố định delay CLOCK, tối đa 31 (0x1F)<br>**CADLY**: giá trị cố định delay CA, tối đa 31 (0x1F)|bit[12:8]=CADLY, bit[4:0]=CLKDLY|`0x00000000`|
|`0x01A8`|CH1_FORCE_TRAIN_DLY|Tương tự như trên nhưng dành cho **CH1**|bit[12:8]=CADLY, bit[4:0]=CLKDLY|`0x00000000`|
|`0x0278`|FORCE_TRAIN_DLY_EN|Bật/tắt việc dùng giá trị delay cố định sau CA training (áp dụng cho cả 2 register `0x0128` và `0x01A8` ở trên).<br>**EN=0 (tắt)**: dùng kết quả CA training tự động (thuật toán FUM)<br>**EN=1 (bật)**: dùng giá trị đã set trong FORCE_TRAIN_DLY|bit[0]=EN|`0x00000000`|

> Giải thích thêm: "CH0" và "CH1" là 2 kênh (channel) bộ nhớ độc lập — LPDDR4 trên board này có 2 channel dữ liệu chạy song song, mỗi channel có bộ register delay riêng. "FUM アルゴ" (thuật toán FUM) là tên thuật toán tự động dò delay tối ưu mà firmware dùng trong training — khi tắt tính năng force, hệ thống quay lại tin tưởng hoàn toàn vào thuật toán này.

---

## Phần 3: ・実装イメージ：LPDDR4のトレーニングシーケンス (Hình ảnh minh họa: chuỗi trình tự training LPDDR4)

Đây là 2 sơ đồ khối (flowchart) mô tả luồng xử lý mã nguồn thực tế.

### Sơ đồ khối lớn bên trái (luồng tổng thể):

```
[DQS to DQパラメータを設定]  → Set tham số DQS to DQ
        ↓
[CAトレーニング]  ← (khoanh đỏ, có mũi tên chỉ xuống sơ đồ chi tiết bên dưới)
   → CA training (đây chính là bước sẽ được mở rộng thêm logic mới)
        ↓
[Write leveling]
   → Bước canh chỉnh thời điểm ghi (1 loại training khác, không đổi)
        ↓
[Write levelingで決まったパラメータを適用]
   → Áp dụng tham số đã xác định được từ Write leveling
```

### Sơ đồ khối "CAトレーニングの実" (Chi tiết bên trong khối CA training) — khung có viền xanh dương bên trái dưới:

```
[LPDDR4デバイスをパワーダウンから復帰]
   → Đánh thức thiết bị LPDDR4 khỏi trạng thái power-down
        ↓
◇[CAトレーニングエラーはなかった？ かつ QSPIでCAトレーニングスキップモードに設定してない]
   → (Hình thoi = điều kiện rẽ nhánh) Kiểm tra: "CA training có bị lỗi không?" VÀ "chế độ QSPI có đang set bỏ qua (skip) CA training không?"
        ↓ (nếu điều kiện đúng)
[CS/CAのディレイ調整の結果を適用]  ← (khoanh đỏ — đây là điểm mà sơ đồ MỚI sẽ được chèn vào, xem mũi tên xanh nối sang bên phải)
   → Áp dụng kết quả điều chỉnh delay của CS/CA (từ training tự động)
        ↓
[完了] → Hoàn tất
```

Nhánh còn lại (điều kiện sai, đi sang phải): `[CS/CAディレイ調整の結果を無視して、初期値を設定]` → Bỏ qua kết quả điều chỉnh CS/CA, dùng giá trị khởi tạo mặc định → cũng dẫn tới [完了] Hoàn tất.

### Sơ đồ khối MỚI THÊM VÀO (khung nét đứt, nhãn "追加" = "Bổ sung mới") — nằm giữa bên phải:

```
◇[CLKディレイ固定が有効？]
   → (Điều kiện rẽ nhánh) "Cơ chế cố định delay CLK có đang bật không?"
   
   Nếu Y (Yes/Có):
   [固定ディレイ値設定(FORCE_TRAIN_DLY)を適用(CLK,CAのディレイ)]
   → Áp dụng giá trị delay cố định đã cài trong FORCE_TRAIN_DLY (cho cả CLK và CA)
   
   Nếu N (No/Không):
   [既存メモリはこちらの運用とする]
   → Với memory cũ (hiện tại đang dùng), vẫn vận hành theo cách này (tức giữ nguyên, dùng [CS/CAのディレイ調整の結果を適用] như luồng cũ)
        ↓
   Cả 2 nhánh đều dẫn tới [完了] Hoàn tất
```

**Tóm lại ý nghĩa của toàn bộ sơ đồ**: Sau khi CA training tự động chạy xong và không lỗi, hệ thống kiểm tra thêm 1 điều kiện mới: "Có bật ép cứng delay CLK không?"

- Nếu **có bật** → bỏ qua/ghi đè kết quả tự động, dùng thẳng giá trị delay đã định sẵn trong tham số (FORCE_TRAIN_DLY) cho cả CLK và CA.
- Nếu **không bật** (trường hợp dùng cho memory cũ, đảm bảo tương thích ngược, không ảnh hưởng hệ thống đang chạy tốt) → vẫn dùng kết quả training tự động như trước giờ.

Đây chính là lý do vì sao ở bảng tổng hợp trước đó ghi chú "cần cẩn thận khi nhập tham số — nếu set sai delay cố định có thể khiến máy không boot lên được": vì giờ đây có 1 đường tắt ghi đè hẳn kết quả dò tự động bằng số liệu con người tự nhập, nếu nhập sai số thì tín hiệu sẽ lệch giờ hẳn.

---

## Phần 4: ・ディレイ値の反映先 (Các register đích mà giá trị delay sẽ được ghi vào)

Đây là danh sách liệt kê: khi giá trị CADLY/CLKDLY được xác định (dù là từ training tự động hay từ giá trị ép cứng), nó sẽ được ghi đồng loạt vào những register vật lý nào bên dưới. Mỗi tín hiệu CA (CA0~CA5) và mỗi tín hiệu liên quan CLK (CKE0, CKE1, CK0, CK0n, CK1, CK1n) đều có **2 bản ghi** (hậu tố `0` và `1`) — đây là do mỗi chân tín hiệu vật lý có 2 giai đoạn lấy mẫu (ví dụ theo cạnh lên/xuống của xung nhịp, gọi là "double data rate" — DDR), nên cần 2 giá trị delay riêng biệt cho từng giai đoạn.

**CH0: CADLY (delay tín hiệu địa chỉ/lệnh) ghi vào 12 register**, ứng với 6 đường CA (CA0 đến CA5), mỗi đường có 2 bản (0 và 1):

```
CH0_CADLY0_CA0, CH0_CADLY1_CA0
CH0_CADLY0_CA1, CH0_CADLY1_CA1
CH0_CADLY0_CA2, CH0_CADLY1_CA2
CH0_CADLY0_CA3, CH0_CADLY1_CA3
CH0_CADLY0_CA4, CH0_CADLY1_CA4
CH0_CADLY0_CA5, CH0_CADLY1_CA5
```

**CH0: CLKDLY (delay tín hiệu clock và các tín hiệu liên quan) ghi vào 12 register**:

```
CH0_CADLY0_CKE0, CH0_CADLY1_CKE0   (CKE = Clock Enable, chân bật xung nhịp)
CH0_CADLY0_CKE1, CH0_CADLY1_CKE1
CH0_CLKDLY0_CK0, CH0_CLKDLY1_CK0     (CK0 = xung nhịp chính, pha dương)
CH0_CLKDLY0_CK0n, CH0_CLKDLY1_CK0n   (CK0n = xung nhịp chính, pha âm/đảo — LPDDR4 dùng xung nhịp vi sai)
CH0_CLKDLY0_CK1, CH0_CLKDLY1_CK1     (cặp xung nhịp thứ 2, cho chip/rank thứ 2 nếu có)
CH0_CLKDLY0_CK1n, CH0_CLKDLY1_CK1n
```

**CH1: CADLY** — 12 register tương tự, đổi tiền tố thành `CH1_CADLY...CA0` đến `CA5`.

**CH1: CLKDLY** — 12 register tương tự, đổi tiền tố thành `CH1_CADLY...CKE0/CKE1` và `CH1_CLKDLY...CK0/CK0n/CK1/CK1n`.

**Tổng cộng**: mỗi channel (CH0, CH1) có 12 register CADLY + 12 register liên quan CLKDLY = 24 register/channel, tổng 48 register cho cả 2 channel — đây chính là toàn bộ các "núm chỉnh" thời điểm tín hiệu vật lý mà 2 giá trị CADLY/CLKDLY (nằm gọn trong 1 register FORCE_TRAIN_DLY) sẽ được **phân phối/nhân bản** ra khi áp dụng.

---

### Tóm tắt toàn bộ ý nghĩa của 適用No.5

1. **Vấn đề**: margin thời gian giữa tín hiệu CLK và CA quá hẹp sau khi chạy training tự động — rủi ro lỗi khi điều kiện biên (nhiệt độ, điện áp) thay đổi.
2. **Giải pháp**: cho phép ghi đè kết quả training tự động bằng giá trị delay CLK/CA do kỹ sư phần cứng đo đạc và chốt sẵn (được lưu trong tham số BOOT), thông qua 3 register mới (`0x0128`, `0x01A8`, `0x0278`).
3. **Cơ chế bật/tắt**: có công tắc riêng (`FORCE_TRAIN_DLY_EN`) để không ảnh hưởng đến memory cũ đang hoạt động ổn định (memory cũ vẫn tiếp tục dùng kết quả tự động như trước).
4. **Rủi ro**: vì đây là ghi đè cứng không qua kiểm tra tự động, nếu nhập sai giá trị delay thì thiết bị LPDDR4 có thể lỗi training và máy không khởi động được — đòi hỏi việc nhập tham số phải rất cẩn thận, chính xác.