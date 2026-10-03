# 適用No.6 — Đối sách khi kết quả training bất thường (Bản dịch + Giải thích chi tiết)

## Kiến thức nền cần biết trước (để hiểu toàn bộ nội dung dưới đây)

- **SuperWarp**: đây là tên gọi chế độ "khởi động nhanh" (fast boot) của thiết bị — khác với "khởi động thường" (通常起動/normal boot) vốn chạy đầy đủ mọi bước init. SuperWarp bỏ qua một số bước để boot nhanh hơn, nhưng vẫn cần training LPDDR4 để đảm bảo truy cập RAM đúng.
- **ATF** = **ARM Trusted Firmware** — lớp phần mềm chạy ở mức đặc quyền cao nhất ngay sau khi CPU bật nguồn, trước cả khi hệ điều hành load. Đây là nơi chứa đoạn code thực hiện training LPDDR4.
- **QSPI parameter**: các tham số cấu hình LPDDR4 (bao gồm delay, timing...) được lưu trong một chip nhớ flash loại QSPI, BOOT sẽ đọc ra để dùng.
- **Checksum**: một giá trị kiểm tra toàn vẹn dữ liệu — nếu tham số bị hỏng/sai lệch, checksum sẽ không khớp, hệ thống biết tham số này không đáng tin.
- **Pad Calibration**: bước tự hiệu chỉnh điện của các chân (pad) vật lý kết nối vật lý giữa CPU và RAM, làm trước khi training.
- **train_err_cnt / RETRY_NUM**: bộ đếm số lần training bị lỗi và số lần retry tối đa được phép (được cấu hình sẵn trong tham số QSPI).

---

## Tiêu đề: 適用No.6　トレーニング結果異常値時の対策

→ **Áp dụng No.6 — Đối sách khi kết quả training có giá trị bất thường**

> SparrowH FC評価で発生した「【SYSQA-IT6 2.3.2-056】【SparrowH】SuperWarp起動中にリブートが発生」の対策を導入する。

→ Đưa vào đối sách cho vấn đề đã phát sinh khi đánh giá SparrowH FC (Final Check): **【SYSQA-IT6 2.3.2-056】【SparrowH】Bị reboot trong lúc đang khởi động SuperWarp**.

(SparrowH là tên model/dự án sản phẩm; SYSQA-IT6 2.3.2-056 là mã số ticket lỗi hệ thống ghi nhận sự cố này.)

---

## ＜問題＞ (Vấn đề)

> SuperWarp起動時に行われる、LPDDR4のトレーニング結果に対するベリファイが行われておらず、 トレーニングが失敗した場合でも成功として起動処理を継続していたため、 メインメモリへのアクセスが正常に行えない状態になり、リブート(通常起動)で立ち上がる。

→ Trong lúc khởi động SuperWarp, kết quả training LPDDR4 **không được verify (kiểm tra xác nhận)**. Do đó, ngay cả khi training thực tế bị **thất bại**, hệ thống vẫn coi là **thành công** và tiếp tục quá trình khởi động. Kết quả là việc truy cập main memory (RAM chính) không thực hiện được đúng cách, khiến máy bị **reboot** và khởi động lại bằng chế độ **thông thường (normal boot)**.

### ○垂れ流しログ (Log stream/log liên tục xuất ra) của LPDDR4:

**・正常時 (Khi bình thường):**

```
lpddr4: Training result size:2304
```

→ Kích thước kết quả training = 2304 (byte) — đây là con số kỳ vọng khi training thành công.

**・異常時 (Khi bất thường):**

```
lpddr4: Training result size:0
```

hoặc

```
lpddr4: Training result size:64997   ※数値は一定でない (giá trị này không cố định, thay đổi mỗi lần)
```

→ Khi training thất bại, kích thước ghi ra hoặc là 0, hoặc là một số rác bất kỳ (không cố định) — đây là dấu hiệu dữ liệu bị hỏng/không hoàn chỉnh.

---

## ＜修正内容＞ (Nội dung sửa chữa)

> SuperWarp起動時のLPDDR4のトレーニング終了後に、メインメモリへ書き込んだトレーニング結果サイズの ベリファイを行う。ベリファイ結果NG（期待値と異なる）場合には、 再度、LPDDR4のトレーニングを行うことで、メインメモリへ正常にアクセスできるように対策を行う。

→ Sau khi training LPDDR4 (trong lúc khởi động SuperWarp) kết thúc, tiến hành **verify kích thước kết quả training** đã được ghi vào main memory. Nếu kết quả verify là **NG** (khác với giá trị kỳ vọng), thì sẽ **thực hiện lại (retry) training LPDDR4** để đảm bảo có thể truy cập main memory bình thường.

> ※マスターの実装情報はソースファイルである為、一部仕様が異なる可能性あり (chữ đỏ) → **Lưu ý**: Vì thông tin triển khai gốc (master) lấy trực tiếp từ file source code, nên có khả năng một phần đặc tả (spec) mô tả ở đây khác với thực tế.

---

## Vẽ lại sơ đồ 1: リトライ動作のフロー (Luồng hoạt động Retry) — sơ đồ bên trái, khung "ATF"

Đây là sơ đồ mô tả **logic retry ở tầng khái quát** (nằm trong khối chương trình ATF):

```
┌─────────────────────────────────┐
│              ATF                 │
│                                   │
│   ┌───────────────────────┐      │
│   │  LPDDR4                │◄─────┼──── (quay lại đây nếu retry)
│   │  トレーニング          │      │      = Chuỗi trình tự
│   │  シーケンス            │      │        training LPDDR4
│   └───────────┬────────────┘      │
│               ▼                   │
│         ◇ トレーニング            │
│           FAIL？ ─── N ──────┐   │      = Training có FAIL không?
│               │ Y              │   │        N (Không) → đi thẳng xuống
│               ▼                 │   │        Y (Có)   → kiểm tra tiếp
│         ◇ リトライ              │   │
│           既定数超えた？ ── N ──┘   │      = Đã vượt quá số lần
│               │ Y                  │        retry quy định chưa?
│               ▼                     │       N (Chưa vượt) → quay lại
│      ┌─────────────────┐            │        training lần nữa (vòng lặp)
│      │  エラー通知       │            │       Y (Đã vượt) → báo lỗi
│      │ (Error Flag=ON)  │            │
│      └────────┬─────────┘            │
│               ▼                       │
│         ┌──────────┐                  │
│         │  完了     │ ◄────────────────┘ (cả 2 nhánh N đều dẫn tới đây)
│         └──────────┘
└─────────────────────────────────┘
   ↑ (chú thích màu đỏ bên dưới khung, có mũi tên chỉ vào box "LPDDR4トレーニングシーケンス")
   "トレーニング終了後 トレーニング結果サイズのベリファイを行い"
   → "Sau khi training kết thúc, thực hiện verify kích thước kết quả training"
```

**Giải thích luồng bằng lời:**

1. Chạy **chuỗi trình tự training LPDDR4** (khối màu xanh đậm) — đây là nơi đã được bổ sung thêm bước verify kích thước kết quả (ghi chú đỏ bên dưới chỉ rõ điều này).
2. Kiểm tra: **"Training có FAIL không?"**
    - Nếu **N (Không fail)** → coi như thành công, đi thẳng tới **"完了" (Hoàn tất)**.
    - Nếu **Y (Có fail)** → kiểm tra tiếp bước 3.
3. Kiểm tra: **"Đã vượt quá số lần retry quy định chưa?"**
    - Nếu **N (Chưa vượt)** → quay ngược lại bước 1, chạy lại training LPDDR4 (đây chính là "retry" — thử lại).
    - Nếu **Y (Đã vượt số lần cho phép)** → chuyển sang **"Thông báo lỗi (Error Flag = ON)"**, rồi tới **"完了"**.

Nói cách khác: hệ thống sẽ tự thử lại training tối đa N lần (N = số quy định trước); nếu vẫn liên tục fail vượt quá giới hạn đó thì mới chịu thua và bật cờ báo lỗi.

---

## Vẽ lại sơ đồ 2: Chuỗi chi tiết bên phải (chưa có tiêu đề riêng, mô tả chi tiết các bước init trước khi vào training)

Đây là sơ đồ **chi tiết hóa từng bước xử lý thực tế** bên trong quá trình chuẩn bị + training LPDDR4 (bổ sung ngữ cảnh cho sơ đồ 1 ở trên):

```
                    ┌─────────┐
                    │  start  │
                    └────┬────┘
                         ▼
         ┌───────────────────────────────┐
         │ GPIO_A/H/Iの値を取得            │ = Lấy giá trị GPIO_A/H/I
         └────────────────┬────────────────┘   (đọc trạng thái chân GPIO phần cứng)
                          ▼
         ┌───────────────────────────────┐
         │ Swizzle（基板の配線）設定       │ = Cài đặt Swizzle (bố trí đi dây trên board)
         └────────────────┬────────────────┘
                          ▼
         ┌───────────────────────────────┐
         │ QSPIパラメータ確認              │ = Xác nhận tham số QSPI
         │ （チェックサム確認）            │   (kiểm tra checksum)
         │ ※チェックサムでQSPIパラメータは │   ※Dùng checksum để phán định
         │   有効と判断                    │    tham số QSPI có hợp lệ không
         └────────────────┬────────────────┘
                          ▼
                ◇ QSPIパラメータは有効？   = Tham số QSPI có hợp lệ không?
                    │             │
                  NO│             │YES
                    ▼             ▼
    ┌──────────────────┐   ┌──────────────────────┐
    │ GPIOにて判別した   │   │ QSPIから取得したパラメータ │
    │ パラメータにて設定 │   │ にて設定                  │
    └────────┬──────────┘   └───────────┬────────────┘
             │                          ▼
             │              ┌──────────────────────┐
             │  (chú thích  │ 25MHz→100MHzに変更     │ = Đổi tần số từ 25MHz→100MHz
             │  cam bên phải└───────────┬────────────┘
             │  của box GPIO)          ▼
             │              ┌──────────────────────┐
             │              │ 100MHz用の駆動力設定を  │ = Áp dụng cấu hình driving
             │              │ 適用                    │   strength (độ mạnh tín hiệu)
             │              └───────────┬────────────┘    cho tần số 100MHz
             │                          ▼
             │              ┌──────────────────────┐
             │              │ SoC側のPad Calibration │ = Thực hiện Pad Calibration
             │              │ を実施                  │   phía SoC (chip xử lý)
             │              └───────────┬────────────┘   (chú thích cam: "chi tiết xem sheet post_cal")
             │                          ▼
             │              ┌──────────────────────┐
             │              │ 100MHz向けのDDRパラメータ│ = Set tham số DDR dành cho 100MHz
             │              │ を設定                  │
             │              └───────────┬────────────┘
             │                          ▼
             │              ┌──────────────────────┐
             │              │ LPDDR4初期化シーケンスを│ = Thực thi chuỗi khởi tạo LPDDR4
             │              │ 実施                    │
             └─────────────►└───────────┬────────────┘
                    (2 nhánh gộp lại)   ▼
                           ┌─────────────────────────────────┐
                           │ train_err_cnt: 0, RETRY_NUM:     │◄──── 【追加】điểm quay lại khi
                           └────────────────┬──────────────────┘      verify NG (mũi tên đỏ dài
                                            ▼                          từ trên cùng bên phải xuống đây)
                           ┌─────────────────────────────────┐
                           │ 高周波数向けのタイミング          │ = Áp dụng tham số timing/
                           │ パラメータ/駆動力を適用           │   driving strength dành cho
                           └────────────────┬──────────────────┘   tần số cao
                                            ▼
                           ┌─────────────────────────────────┐
                           │ DQS to DQパラメータを設定         │ = Set tham số DQS to DQ
                           └────────────────┬──────────────────┘
                                            ▼
                           ┌─────────────────────────────────┐
                           │ CAトレーニング                    │ = CA training  ◄─ (chú thích cam:
                           └─────────────────────────────────┘    "chi tiết xem sheet CAトレーニング")
```

**Ghi chú màu cam bên cạnh box "GPIOにて判別したパラメータにて設定":**

> Micron4G8はshrunkジシップ版パラメータがRUM(?)、SamsungもGPIOにて判別したパラメータが、適用にすべてMicronとして判...

(Đoạn text này trong ảnh bị cắt/mờ một phần, khó dịch chính xác 100%, nhưng đại ý là: ghi chú kỹ thuật về việc chip Micron 4Gb 8-die (shrink version) và cả chip Samsung khi được nhận diện qua GPIO thì sẽ tạm thời áp dụng theo tham số chung như thể là Micron.)

**Hai chú thích màu đỏ quan trọng trên sơ đồ:**

1. **【追加】ベリファイ結果NG時の戻り位置** (góc trên bên phải, mũi tên đỏ dài chạy dọc từ trên xuống điểm "train_err_cnt: 0, RETRY_NUM"): → **【Bổ sung mới】Vị trí quay lại khi kết quả verify là NG.** → Đây chính là điểm mô tả cụ thể: khi bước verify kích thước kết quả training (được thêm mới ở No.6) phát hiện NG, chương trình sẽ nhảy ngược về đúng điểm này (`train_err_cnt`, trước bước "áp dụng tham số timing tần số cao" → "DQS to DQ" → "CA training") để **chạy lại toàn bộ chuỗi training từ đầu**, chứ không phải khởi động lại từ bước GPIO/QSPI phía trên.
    
2. **トレーニングリトライn回の戻り位置** (nằm ngay phía trên box "train_err_cnt: 0, RETRY_NUM"): → **Vị trí quay lại khi retry training lần thứ n.** → Xác nhận đây chính là "điểm neo" (anchor point) của vòng lặp retry — mỗi lần retry sẽ quay về đúng chỗ này để thử lại.
    

**Chú thích cam bên cạnh box RETRY_NUM:**

> RETRY_NUMはQSPIパラメータにて設定可能な模様。デフォルト10 → **RETRY_NUM** (số lần retry tối đa) có vẻ như có thể cấu hình được thông qua tham số QSPI. **Giá trị mặc định là 10.**

---

## Bảng cuối: 変更内容資料より (Trích từ tài liệu mô tả nội dung thay đổi)

|Mục|Nội dung|
|---|---|
|**JIRA番号** (Mã JIRA)|**OP_BTS-51229**|
|**現象** (Hiện tượng)|SuperWarp起動時に行われる、LPDDR4のトレーニングに失敗したため、メインメモリへのアクセスが正常に行えない状態になる。<br>→ Do training LPDDR4 (thực hiện lúc khởi động SuperWarp) bị thất bại, dẫn đến trạng thái không thể truy cập main memory bình thường.|
|**原因** (Nguyên nhân)|LPDDR4のトレーニング結果に対するベリファイが行われておらず、トレーニングが失敗した場合でも成功として処理を継続していたため。<br>→ Do không thực hiện verify đối với kết quả training LPDDR4, nên ngay cả khi training thất bại vẫn tiếp tục xử lý như thể đã thành công.|
|**対策** (Đối sách)|LPDDR4のトレーニングに失敗した時に、再度、LPDDR4のトレーニングを実行し復旧を行う。<br>→ Khi training LPDDR4 thất bại, sẽ thực hiện lại (retry) training LPDDR4 để khôi phục.<br><br>LPDDR4のトレーニング終了後に、メインメモリへ書き込んだトレーニング結果サイズのベリファイを行う。ベリファイ結果NG（期待値と異なる）場合には、再度、LPDDR4のトレーニングを行うことで、メインメモリへ正常にアクセスできるように対策を行う。<br>→ Sau khi training LPDDR4 kết thúc, tiến hành verify kích thước kết quả training đã ghi vào main memory. Nếu kết quả verify NG (khác với giá trị kỳ vọng), sẽ thực hiện lại training LPDDR4 để đảm bảo có thể truy cập main memory bình thường.|

---

## Tổng kết dễ hiểu toàn bộ 適用No.6

Hãy hình dung quá trình này như sau: mỗi lần máy khởi động (chế độ SuperWarp — boot nhanh), CPU phải "dạy" (train) cho RAM biết chính xác thời điểm gửi/nhận tín hiệu (giống việc canh giờ bắt tay giữa 2 người). Kết quả của quá trình "dạy" này được lưu vào 1 vùng nhớ, với kích thước chuẩn là **2304 byte**.

**Bug trước khi sửa**: hệ thống ghi kết quả training vào bộ nhớ nhưng **không bao giờ kiểm tra lại** xem kết quả đó có đúng/đủ hay không — cứ mặc định là "xong rồi, chắc là đúng" rồi đi tiếp. Nếu chẳng may lần đó training bị lỗi (ra kích thước 0 hoặc số rác ngẫu nhiên), hệ thống vẫn coi như thành công và tiếp tục dùng RAM → nhưng RAM chưa được "dạy" đúng cách → truy cập sai dữ liệu → máy tự khởi động lại (reboot) ở chế độ chậm hơn (normal boot) để "chữa cháy".

**Sau khi sửa (適用No.6)**: thêm 1 bước **kiểm tra (verify)** ngay sau khi training xong — so sánh kích thước kết quả với giá trị kỳ vọng (2304). Nếu khớp → đi tiếp bình thường. Nếu **không khớp (NG)** → quay lại chạy training từ đầu (retry), tối đa **10 lần** (giá trị mặc định `RETRY_NUM`, có thể chỉnh qua tham số QSPI). Nếu retry đủ 10 lần vẫn không được thì mới báo lỗi thật sự (`Error Flag = ON`).

Đây chính là lý do trong bảng tổng hợp trước đó có ghi 2 tác dụng phụ đáng lưu ý:

- **Thời gian boot có thể lâu hơn** nếu phải retry nhiều lần (vì mỗi lần retry tốn thêm thời gian chạy lại toàn bộ chuỗi training).
- **Regression khi phục hồi từ Sleep2**: có khả năng thay đổi này vô tình ảnh hưởng đến luồng xử lý khi máy thức dậy từ chế độ ngủ sâu (Sleep2), gây ra hiện tượng bị reboot không mong muốn ở tình huống đó — đây là điểm cần các kỹ sư theo dõi thêm.