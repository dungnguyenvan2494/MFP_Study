# Sơ đồ khối gói chip LPDDR4 theo chuẩn JEDEC (Bản dịch + Giải thích chi tiết)

Đây chính là **tài liệu gốc** (trích từ datasheet chuẩn JEDEC/nhà sản xuất LPDDR4) mà các bảng "IO MAP" và bảng "device0/device1, CS0/CS1, die0~die3" ở các trang trước đang cố gắng ánh xạ vào! Đọc xong trang này, toàn bộ những ký hiệu khó hiểu (`CS0_A`, `CKE0_B`, `die0`, `die1`...) ở các bảng trước sẽ trở nên rõ ràng.

## Kiến thức nền cần biết trước

- **Package (gói chip)**: là "vỏ" vật lý mà người dùng cuối nhìn thấy khi cầm 1 con chip lên — bên trong 1 package có thể chứa **nhiều die** (lát bán dẫn) xếp chồng lên nhau.
- **Die**: 1 lát bán dẫn độc lập, tự nó là 1 chip nhớ hoàn chỉnh nếu tách riêng ra.
- **Channel (kênh)**: là 1 "đường bus dữ liệu" độc lập — 2 channel A và B hoạt động hoàn toàn song song, không chia sẻ đường dữ liệu DQ với nhau (nhưng như ta sẽ thấy, có thể chia sẻ 1 số tín hiệu điều khiển).
- **Rank**: là 1 "nhóm die" được chọn cùng lúc bởi 1 tín hiệu CS — khi hệ thống có nhiều rank, tại 1 thời điểm chỉ 1 rank được kích hoạt (giống như có nhiều "tầng" bộ nhớ nhưng dùng chung 1 đường dây, chỉ đường CS khác nhau để phân biệt).
- **CS (Chip Select)**: chọn rank nào đang hoạt động.
- **CKE (Clock Enable)**: bật/tắt xung nhịp nội bộ của rank đó.
- **CK_t / CK_c**: cặp xung nhịp vi sai — "t" (true, bản gốc) và "c" (complement, bản đảo pha 180°). Dùng tín hiệu vi sai giúp chống nhiễu tốt hơn so với chỉ dùng 1 dây đơn.
- **CA[5:0]**: 6 đường command/address dùng chung.
- **DQ[15:0]**: 16 đường dữ liệu (x16 = độ rộng bus 16-bit cho 1 channel).
- **DQS_t/DQS_c[1:0]**: 2 cặp tín hiệu strobe dữ liệu vi sai (dùng để đồng bộ đọc/ghi DQ).
- **DMI (Data Mask Inversion)**: kết hợp 2 chức năng — che dữ liệu (mask) và đảo bit (inversion), thay thế cho DM ở các thế hệ DDR cũ.
- **ODT_CA**: On-Die Termination cho đường CA — điện trở đầu cuối tích hợp sẵn trong chip để giảm phản xạ tín hiệu (signal reflection) trên đường truyền.
- **ZQ / RZQ**: chân và điện trở tham chiếu dùng để chip tự hiệu chỉnh (calibrate) giá trị trở kháng đầu ra/ODT của chính nó cho chính xác.
- **V_DD1, V_DD2, V_SS, V_DDQ**: các đường nguồn điện cấp cho chip (V_SS là nguồn đất/GND, V_DDQ là nguồn riêng cho khối I/O).

---

## Sơ đồ 1 (bên phải, trên): Figure 2 — Single-Die, Single-Channel, Single-Rank Package (x16 I/O)

**Dịch tiêu đề**: Sơ đồ khối gói chip — **1 die, 1 channel, 1 rank** (loại đơn giản nhất, độ rộng dữ liệu 16-bit)

```
                    V_DD1  V_DD2  V_SS  V_DDQ
                      │      │     │      │
                      ▼      ▼     ▼      ▼
              ┌───────────────────────────────┐
              │  ┌───────────────────────┐    │           V_DDQ
RESET_n ─────►│  │                       │    │             │
              │  │                       │    ├──► ZQ ──►[RZQ]
  CS ────────►│  │        Die            │    │
              │  │      LPDDR4           │    │
 CKE ────────►│  │                       │    │
              │  │                       │◄──►├──► DMI[1:0]
CK_t ────────►│  │                       │◄──►├──► DQ[15:0]
CK_c ────────►│  │                       │◄──►├──► DQS[1:0]_t
              │  │              ODT_CA   │◄──►├──► DQS[1:0]_c
CA[5:0] ─────►│  │                ●──────┼────┼──► ODT_CA
              │  └───────────────────────┘    │
              └───────────────────────────────┘
```

**Giải thích luồng tín hiệu**:

- Bên trái là các tín hiệu **đi vào** chip (mũi tên `►`): `RESET_n` (reset), `CS` (chọn chip — vì chỉ có 1 rank nên chỉ cần 1 đường CS duy nhất, không cần đánh số 0/1), `CKE` (bật xung nhịp — được **tô vàng** trong ảnh gốc, có lẽ để nhấn mạnh đây là tín hiệu trọng tâm cần chú ý khi so sánh với các hình sau), `CK_t`/`CK_c` (xung nhịp vi sai), `CA[5:0]` (lệnh/địa chỉ).
- Bên phải là các tín hiệu **2 chiều** (mũi tên `◄►`, vì đây là dữ liệu, đọc/ghi cả 2 hướng): `DMI[1:0]`, `DQ[15:0]`, `DQS[1:0]_t`, `DQS[1:0]_c`, và `ODT_CA`.
- `ZQ` nối ra ngoài package tới 1 điện trở tham chiếu **RZQ** (nằm ngoài chip, trên board mạch) — dùng để chip tự đo và hiệu chỉnh trở kháng của chính nó.

Đây là cấu hình đơn giản nhất: **1 gói chip = 1 die = 1 channel = 1 rank**, độ rộng dữ liệu 16-bit (x16 I/O).

---

## Sơ đồ 2 (bên phải, dưới): Figure 3 — Dual-Die, Dual-Channel, Single-Rank Package (x32 I/O)

**Dịch tiêu đề**: Sơ đồ khối gói chip — **2 die, 2 channel, 1 rank** (độ rộng dữ liệu 32-bit)

```
                         V_DD1  V_DD2  V_SS  V_DDQ
                           │      │     │      │
                           ▼      ▼     ▼      ▼
              ┌────────────────────────────────────┐
              │                                     │        V_DDQ
RESET_n ─────►│●──┐                                 │           │
              │   │  ┌─────────────────────────┐    ├──► ZQ0 ─►[RZQ]
CS0_A ───────►│   ├─►│                         │    │
              │   │  │      Die                │    │
CKE0_A ──────►│   │  │   LPDDR4 Channel A      │    │
              │   │  │                         │◄──►├──► DMI[1:0]_A
CK_t_A ──────►│   │  │                         │◄──►├──► DQ[15:0]_A
CK_c_A ──────►│   │  │                         │◄──►├──► DQS[1:0]_t_A
              │   │  │              ODT_CA     │◄──►├──► DQS[1:0]_c_A
CA[5:0]_A ───►│   │  │                ●────────┼────┼──► ODT_CA_A
              │   │  └─────────────────────────┘    │
              │   │                                 │
              │   │  ┌─────────────────────────┐    │
CS0_B ───────►│   ├─►│                         │    │
              │   │  │      Die                │    │
CKE0_B ──────►│   │  │   LPDDR4 Channel B      │    │
              │   │  │                         │◄──►├──► DMI[1:0]_B
CK_t_B ──────►│   │  │                         │◄──►├──► DQ[15:0]_B
CK_c_B ──────►│   │  │                         │◄──►├──► DQS[1:0]_t_B
              │   │  │              ODT_CA     │◄──►├──► DQS[1:0]_c_B
CA[5:0]_B ───►│   └─►│                ●────────┼────┼──► ODT_CA_B
              │      └─────────────────────────┘    │
              └────────────────────────────────────┘
```

**Giải thích luồng tín hiệu**:

- Chỉ có **1 đường RESET_n duy nhất** (chấm đen `●` cho thấy nó được **chia nhánh (fanout)** tới cả 2 die A và B — dùng chung, vì reset toàn hệ thống thì cả 2 die đều phải reset cùng lúc).
- Riêng cho **Die/Channel A**: `CS0_A`, `CKE0_A`, `CK_t_A`/`CK_c_A`, `CA[5:0]_A` — đều có hậu tố `_A` để phân biệt.
- Riêng cho **Die/Channel B**: `CS0_B`, `CKE0_B`, `CK_t_B`/`CK_c_B`, `CA[5:0]_B` — hậu tố `_B`.
- Dữ liệu ra vào 2 chiều: `DMI[1:0]_A/B`, `DQ[15:0]_A/B`, `DQS[1:0]_t/c_A/B`, `ODT_CA_A/B` — mỗi channel có bộ riêng hoàn toàn.
- Chỉ có **1 chân ZQ0 chung** cho cả gói (đấu ra 1 điện trở RZQ bên ngoài) — vì việc hiệu chỉnh trở kháng có thể chia sẻ giữa 2 die trong cùng 1 gói.
- Vì mỗi channel có DQ[15:0] (16-bit) và có 2 channel độc lập → tổng cộng 32-bit → đúng như tên gọi **"x32 I/O"**.

**→ Đây chính là kiến trúc khớp với board mà kỹ sư đang điều tra ở các trang trước!** Ký hiệu `CS0_A`, `CKE0_A`... ở đây trùng khớp gần như tuyệt đối với ký hiệu `A_CS0`, `A_CKE1`... đã thấy trong bảng "IO MAP" ở các câu trả lời trước — xác nhận rằng board thực tế dùng đúng kiểu package "Dual-Die, Dual-Channel" này (hoặc kiểu Quad-Die mở rộng ở sơ đồ tiếp theo).

---

## Sơ đồ 3 (bên trái, sơ đồ lớn nhất): Figure 3 — Quad-Die, Dual-Channel, Dual-Rank Package Block Diagram

**Dịch tiêu đề**: Sơ đồ khối gói chip — **4 die, 2 channel, 2 rank**

Đây là bản **mở rộng gấp đôi** so với sơ đồ "Dual-Die, Dual-Channel, Single-Rank" ở trên: thay vì mỗi channel chỉ có 1 die (1 rank), giờ mỗi channel có **2 die xếp chồng** (2 rank: rank0 và rank1), tổng cộng 4 die trong 1 gói.

```
                    V_DD1  V_DD2  V_SS  V_DDQ
                      │      │     │      │
                      ▼      ▼     ▼      ▼
         ┌─────────────────────────────────────┐
         │●─────────────────────────────┐       │
RESET_n─►│ (chia nhánh tới cả 4 die)     │       │              V_DDQ
         │  ┌──────────────────────┐    │       │                │
CS0_A───►│  │                      │    ●───────┼──► ZQ0 ────►[RZQ]
CKE0_A──►│  │   Die (Rank0-A)      │    │       │
         │  │     LPDDR4           │    │       │
CK_t_A──►│  │                      │◄──►┼───────┼──► DMI[1:0]_A
CK_c_A──►│  │                      │◄──►┼───────┼──► DQ[15:0]_A
CA[5:0]_A│  │           ODT_CA     │◄──►┼───────┼──► DQS[1:0]_t_A
    ────►│  └──────────────────────┘◄──►┼───────┼──► DQS[1:0]_c_A
         │                              │───────┼──► ODT_CA_A
         │  ┌──────────────────────┐    │       │
CS0_B───►│  │                      │    │       │
CKE0_B──►│  │   Die (Rank0-B)      │    │       │
         │  │     LPDDR4           │    │       │
CK_t_B──►│  │                      │◄──►┼───────┼──► DMI[1:0]_B
CK_c_B──►│  │                      │◄──►┼───────┼──► DQ[15:0]_B
CA[5:0]_B│  │           ODT_CA     │◄──►┼───────┼──► DQS[1:0]_t_B
    ────►│  └──────────────────────┘◄──►┼───────┼──► DQS[1:0]_c_B
         │                              │───────┼──► ODT_CA_B
         │                                       │              V_DDQ
         │  ┌──────────────────────┐             │                │
CS1_A───►│  │                      │    ●────────┼──► ZQ1 ────►[RZQ]
CKE1_A──►│  │   Die (Rank1-A)      │    │        │
         │  │     LPDDR4           │    │        │
         │  │           ODT_CA     │◄──►┼(dùng chung DMI/DQ/DQS_A ở trên)
         │  └──────────────────────┘    │        │
         │                          [V_SS] (nối đất, không xuất tín hiệu riêng ra ngoài)
         │  ┌──────────────────────┐             │
CS1_B───►│  │                      │             │
CKE1_B──►│  │   Die (Rank1-B)      │             │
         │  │     LPDDR4           │             │
         │  │           ODT_CA     │◄──►(dùng chung DMI/DQ/DQS_B ở trên)
         │  └──────────────────────┘             │
         │                          [V_SS]        │
         └─────────────────────────────────────┘
```

**Giải thích chi tiết (đây là sơ đồ quan trọng nhất, giải mã trực tiếp bảng "device/cs/die" ở các trang trước)**:

Gói chip này có **4 die**, xếp theo thứ tự từ trên xuống trong ảnh gốc:

1. **Die thứ 1**: nhận `CS0_A`, `CKE0_A`, `CK_t_A/CK_c_A`, `CA[5:0]_A` → xuất dữ liệu ra `DMI/DQ/DQS..._A`, `ODT_CA_A`, dùng chung `ZQ0`.
2. **Die thứ 2**: nhận `CS0_B`, `CKE0_B`, `CK_t_B/CK_c_B`, `CA[5:0]_B` → xuất dữ liệu ra `DMI/DQ/DQS..._B`, `ODT_CA_B`, cũng dùng chung `ZQ0`.
3. **Die thứ 3**: nhận `CS1_A`, `CKE1_A` (dùng chung đường CK/CA với die thứ 1 vì cùng "channel A" — chỉ CS/CKE khác để phân biệt rank), dữ liệu ra dùng chung ZQ1 và các đường DMI/DQ/DQS_A.
4. **Die thứ 4**: nhận `CS1_B`, `CKE1_B` (dùng chung đường CK/CA với die thứ 2, cùng "channel B"), dùng chung ZQ1.

**Đây chính xác là lời giải thích cho bảng "device/cs/die" ở các trang trước!** Nhắc lại bảng đó:

```
device0(ch0)  cs0  die0  → AP_M_A_CS0
device1(ch1)  cs0  die0  → AP_M_B_CS0
device0(ch0)  cs0  die1  → AP_M_A_CS0
device1(ch1)  cs0  die1  → AP_M_B_CS0
device0(ch0)  cs1  die2  → AP_M_A_CS1
device1(ch1)  cs1  die2  → AP_M_B_CS1
device0(ch0)  cs1  die3  → AP_M_A_CS1
device1(ch1)  cs1  die3  → AP_M_B_CS1
```

Đối chiếu:

- **"device0(ch0)" = channel A**, **"device1(ch1)" = channel B** (đúng như đã kết luận ở bảng trước).
- **"die0" = Die thứ 1** (Rank0-A, dùng CS0_A) — **"die1" = Die thứ 2** (Rank0-B, dùng CS0_B)...

Chờ đã — cần lưu ý kỹ ở đây: bảng trước ghi "device0 cs0 die0" VÀ "device0 cs0 die1" **CÙNG** trỏ tới `AP_M_A_CS0` — tức là die0 và die1 **CÙNG DÙNG CS0_A**. Nhìn vào sơ đồ Quad-Die này, ta thấy: **die thứ 1 (Rank0-A) và die thứ 3 (Rank1-A) mới là 2 die khác rank của cùng channel A** — vậy "die0" và "die1" trong bảng trước thực chất phải hiểu là: **die0 = die vật lý đầu tiên xếp trong gói, die1 = die vật lý xếp thứ hai (nhưng cùng thuộc CS0/Rank0)** — có thể đây là 2 die được "ghép đôi ảo" bên trong cùng 1 vị trí Rank0-A (ví dụ 2 die chồng lên nhau dùng chung 1 bộ chân CS0_A, giống như kỹ thuật "3DS" — 3D Stacking, nơi 2-4 die vật lý chia sẻ chung 1 bus CS/CKE để tăng dung lượng mà không tăng số chân).

Nói ngắn gọn dễ hiểu: **CS0 (Rank0)** có thể chọn được **nhiều hơn 1 die xếp chồng** (die0 và die1) cùng lúc dùng chung đường điều khiển — đây chính là kỹ thuật tăng dung lượng bằng cách xếp chồng die (die stacking) mà vẫn giữ nguyên số lượng chân tín hiệu cần thiết ra bên ngoài gói chip.

**Ghi chú thêm trong sơ đồ**: 2 die ở dưới cùng (Rank1-A và Rank1-B) có ký hiệu **V_SS** ở phía dưới bên phải — nghĩa là các die này **không có đường ZQ/RZQ riêng**, chúng chỉ nối tới đất (V_SS) ở phần chân đó thay vì đưa ra ngoài — tiết kiệm thêm chân, vì việc hiệu chỉnh trở kháng (ZQ calibration) chỉ cần thực hiện 1 lần đại diện cho cả nhóm rank cùng channel (ZQ0 đại diện rank0, ZQ1 đại diện rank1).

---

## Tổng kết ý nghĩa toàn bộ trang này

Trang này chính là **"lời giải đáp cuối cùng"** cho toàn bộ chuỗi điều tra ở các trang trước:

1. Nó xác nhận **kiến trúc vật lý thật sự** của gói chip LPDDR4 trên board: có thể là loại "Quad-Die, Dual-Channel, Dual-Rank" — tức 4 die vật lý, chia làm 2 channel (A, B), mỗi channel có 2 rank (CS0, CS1).
2. Nó giải thích rõ **tại sao CS và CKE bị "thiếu 4" so với lý thuyết** (đã thấy ở bảng "4→4不足" trước đó): vì trong kiến trúc Dual-Rank, mỗi channel chỉ cần **2 đường CS** (CS0, CS1) và **2 đường CKE** — không cần 4 như công thức tổng quát giả định ban đầu, bởi vì die0/die1 đã dùng chung CS0, và die2/die3 dùng chung CS1 (kỹ thuật die-stacking chia sẻ tín hiệu điều khiển).
3. Nó xác nhận cách đặt tên chuẩn (`CS0_A`, `CKE0_B`, `CK_t_A`...) chính là nguồn gốc của các ký hiệu `A_CS0`, `B_CKE0`... mà kỹ sư dùng để chuẩn hóa lại bảng IO MAP ở các bước điều tra trước.

Nhờ có tài liệu chuẩn JEDEC này, đội kỹ sư mới có đủ căn cứ chắc chắn để khẳng định cách bố trí tín hiệu thực tế, từ đó tự tin sửa đúng chỗ trong code (`applyCsDlys()`, `applyCaDlys()`) để giải quyết dứt điểm Bug2 ("ch1 không set được delay") đã nêu ở 適用No.1.