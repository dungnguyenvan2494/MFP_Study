# 適用No.7 — Thêm thông tin Vendor & nhận diện linh kiện vào BOOT log (Bản dịch + Giải thích chi tiết)

## Kiến thức nền cần biết trước

- **BOOT log (垂れ流しログ)**: đây là log được in ra liên tục qua cổng serial console ngay khi máy vừa cấp nguồn, dùng để kỹ sư debug quan sát quá trình khởi động U-Boot (bootloader).
- **U-Boot**: phần mềm bootloader phổ biến trong nhúng, chạy trước khi load hệ điều hành chính, có nhiệm vụ khởi tạo phần cứng (bao gồm RAM/DRAM).
- **Chip vendor (nhà sản xuất chip nhớ)**: LPDDR4 trên board có thể lấy từ nhiều hãng khác nhau (Micron, Samsung, Hynix...). Firmware cần biết đang dùng chip của hãng nào để áp dụng đúng tham số training/timing tương ứng — vì mỗi hãng có thể có sai số nhỏ khác nhau.
- **Shrink version (phiên bản thu nhỏ)**: cùng một loại chip nhưng qua các thế hệ sản xuất, nhà máy có thể "shrink" (thu nhỏ tiến trình chế tạo, ví dụ từ 20nm xuống 17nm...) để rẻ hơn/tiết kiệm điện hơn — nhưng đặc tính điện có thể hơi khác, nên firmware cần phân biệt được để áp dụng tham số phù hợp.
- **MEMORY_CHIP_VERSION**: đây là 1 giá trị (32-bit) được đọc từ tham số cấu hình LPDDR4, trong đó từng nhóm bit mã hóa các thông tin khác nhau về vendor và version của chip.

---

## Tiêu đề: 適用No.7　BOOTログのベンダー、部品識別の情報追加

→ **Áp dụng No.7 — Thêm thông tin Vendor (nhà sản xuất) và nhận diện linh kiện vào BOOT log**

## 【新規】(Mới) — BOOTログについて (Về BOOT log)

> ①新規ベンダーの名称を使用可能とする。 → ① Cho phép sử dụng tên gọi của các vendor mới.

> ②部品識別の表記の仕様を変更・追加する。 → ② Thay đổi/bổ sung đặc tả cách ghi nhận diện linh kiện (part identification).

---

## Vẽ lại & giải thích: BOOTログ（垂れ流しログ）の先頭 (Đoạn đầu của BOOT log)

Đây không phải là sơ đồ khối mà là **một đoạn log thực tế** được in ra màn hình console khi máy khởi động. Mình chép lại nguyên văn và giải thích từng dòng:

```
U-Boot 2015.01-devel-16.07.2-svn9755 (Jan 25 2022 - 10:00:40)
```

→ Phiên bản U-Boot đang chạy và thời điểm build firmware này (25/01/2022, 10:00:40).

```
I2C:    ready
```

→ Bus giao tiếp I2C (dùng để nói chuyện với các chip ngoại vi khác) đã sẵn sàng.

```
DRAM:   vramsize = 1966080
```

→ Kích thước vùng RAM dành riêng cho video (VRAM) = 1,966,080 byte (~1.9MB).

```
DRAM CH0/CS0 size information
 DRAM CH0 CS0 :0x0000000080000000
 DRAM CH0 CS1 :0x0000000080000000
 DRAM CH1 CS0 :0x0000000080000000
 DRAM CH1 CS1 :0x0000000080000000
```

→ Thông tin kích thước của từng "CS" (Chip Select — tức từng "rank" chip nhớ vật lý) trên mỗi channel (CH0, CH1). Ở đây cả 4 chip select đều báo cùng địa chỉ base `0x80000000`.

```
DRAM Address decode information
 DRAM BANK 0 0x0000000000000000 - 0x00000000bfffffff
 DRAM BANK 1 0x0000000200000000 - 0x00000002ffffffff
 DRAM BANK 2 0x0000000100000000 - 0x00000013ffffffff
DRAM Total: 8 GiB
```

→ Bảng ánh xạ địa chỉ (address decode) của RAM: chia thành 3 vùng bank với dải địa chỉ tương ứng, tổng dung lượng RAM toàn hệ thống là **8GB**.

```
Board: Marvell Quartz Palladium
```

→ Tên board phần cứng (dùng chip nền tảng Marvell).

```
SoC rev: F
Brd rev: 0xA
```

→ Phiên bản chip SoC (rev F) và phiên bản board mạch (rev 0xA).

```
Clock:          SAR             REAL
  CPU    1600 [MHz]
  DDR    1200 [MHz] -> 1200 [MHz]
  FABRIC 1200 [MHz] -> 1200 [MHz]
  PIDI   1000 [MHz]
  DDR 32 Bit width
```

→ Bảng tốc độ xung nhịp: cột "SAR" là giá trị đọc từ chân cấu hình phần cứng (Sample-At-Reset), cột "REAL" là giá trị thực tế được áp dụng sau xử lý. Ở đây: CPU chạy 1600MHz, DDR (RAM) chạy 1200MHz, Fabric (bus nội bộ) 1200MHz, PIDI 1000MHz, độ rộng bus DDR là 32-bit.

```
LLC Disabled
```

→ Bộ nhớ đệm LLC (Last Level Cache) đang bị tắt ở giai đoạn này.

```
Now running in RAM - U-Boot at: 3fc0b000
```

→ U-Boot lúc này đã copy chính nó vào chạy trong RAM, tại địa chỉ `0x3fc0b000` (trước đó có thể chạy trực tiếp từ flash, chậm hơn).

```
U-Boot DT blob at : 00000000030fb4680
```

→ Vị trí của Device Tree Blob (file mô tả cấu hình phần cứng) trong bộ nhớ.

```
MMC:    Board type detected: EMU800
ERROR:  vcc gpio is not initialized, need to implement gpio in SoC code
```

→ Phát hiện loại board là "EMU800" (có thể là board thử nghiệm/emulation); có 1 lỗi cảnh báo: GPIO điều khiển điện áp VCC chưa được khởi tạo, cần bổ sung code xử lý GPIO ở tầng SoC (đây là warning không liên quan trực tiếp đến LPDDR4, chỉ là log đi kèm).

```
XENON-SDHCI: 0
RPSII:  Block count 2
```

→ Log liên quan đến controller thẻ nhớ SD/eMMC (Xenon SDHCI) và RPSII.

**Đoạn được khoanh đỏ (quan trọng nhất, trọng tâm của No.7):**

```
lpddr4: Ch0 : Micron 4GB / Shrink version
lpddr4: Ch1 : Micron 4GB / Shrink version
```

→ **Đây chính là dòng log mà 適用No.7 tác động vào**: cho biết trên **Channel 0** đang dùng chip **Micron 4GB, phiên bản Shrink**; **Channel 1** cũng vậy. Đây là nơi hiển thị thông tin vendor + nhận diện linh kiện mà đặc tả này sẽ mở rộng thêm các vendor mới và các mức Shrink mới.

```
lpddr4: For LPDDR4-2400 parameter Rev.2.22
```

→ Đang dùng bộ tham số cấu hình cho tốc độ LPDDR4-2400, phiên bản tham số Rev 2.22.

```
lpddr4: Now working with QSPI parameter
```

→ Xác nhận đang hoạt động dựa trên tham số được đọc từ QSPI flash.

```
lpddr4: Training result size:2304
```

→ Kích thước kết quả training = 2304 — đây chính là con số "chuẩn" mà 適用No.6 (đã giải thích trước đó) dùng để verify.

---

## LPDDR4パラメータ内・チップ型式パラメータ仕様 (Trong tham số LPDDR4 — Đặc tả tham số kiểu chip)

> アドレス 0x0400：MEMORY_CHIP_VERSION → Địa chỉ `0x0400`: register **MEMORY_CHIP_VERSION**

|アドレス (Địa chỉ)|レジスタ名 (Tên register)|役割 (Vai trò)|
|---|---|---|
|`0x0400`|MEMORY_CHIP_VERSION|チップ型番 → Mã phiên bản chip|

## Bảng phân bố bit: Ch1[31:16], Ch0[15:0]

Nghĩa là giá trị 32-bit của register này được **chia đôi**: 16 bit thấp (`[15:0]`) dành cho thông tin của **Channel 0**, 16 bit cao (`[31:16]`) dành cho thông tin của **Channel 1** — mỗi channel có bộ thông tin riêng vì 2 channel có thể lắp chip khác nhau.

|Ch0 (bit)|Ch1 (bit)|内容 (Nội dung)|
|---|---|---|
|`[3:0]`|`[19:16]`|**memory Vender** (Nhà sản xuất memory)|

**Giá trị cho trường "memory Vender":**

|Mã|Vendor|Trạng thái|
|---|---|---|
|`0x0`|Micron|(có sẵn từ trước)|
|`0x1`|Samsung|(có sẵn từ trước)|
|`0x2`|Hynix|(có sẵn từ trước)|
|`0x3`|**Nanya**|追加 = **Thêm mới**|
|`0x4`|**Winbond**|追加 = **Thêm mới**|
|`0x5`|**ISSI**|追加 = **Thêm mới**|

|Ch0 (bit)|Ch1 (bit)|内容 (Nội dung)|
|---|---|---|
|`[11:8]`|`[27:24]`|**部品識別** (Nhận diện linh kiện / phân biệt phiên bản Shrink)|

**Giá trị cho trường "部品識別" (nhận diện linh kiện):**

|Mã|Ý nghĩa|Trạng thái|
|---|---|---|
|`0x0`|Not Shrink version (chưa qua shrink)|変更 = **Thay đổi** (cách hiển thị/điều kiện áp dụng)|
|`0x1`|Shrink version (đã qua shrink lần 1)|変更 = **Thay đổi**|
|`0x2~F`|**Shrink _(số thứ tự thập phân)_** — ví dụ 0x2 → "Shrink 2"|追加 = **Thêm mới**|

---

## Giải thích 2 mục thay đổi chính

### ①memory Vender (Nhà sản xuất memory)

> 新ベンダー名に対応する。 → Hỗ trợ thêm tên các vendor mới (Nanya, Winbond, ISSI — như bảng trên).

### ②部品識別 (Nhận diện linh kiện)

> ベンダーの条件(Samsungの特定サイズ条件を含む)を外し、共通化する。 → **Bỏ điều kiện ràng buộc theo vendor** (bao gồm cả điều kiện đặc biệt về dung lượng cụ thể của Samsung), làm cho cách hiển thị **thống nhất/chung** cho mọi vendor.

> 2次シュリンク以降の製品向けの表示方法を追加する。 → Thêm cách hiển thị dành cho các sản phẩm từ **lần shrink thứ 2 trở đi**.

---

## ※部品識別の現行仕様 (Đặc tả hiện hành của việc nhận diện linh kiện — TRƯỚC khi sửa)

Đây chính là logic **cũ, bị giới hạn**, giải thích tại sao cần sửa:

**部品識別=0x0の場合 (Trường hợp mã nhận diện = 0x0):**

- ベンダー 0x0=Micron の場合：**Not Shrink version** → Nếu vendor là Micron (bất kỳ dung lượng nào) → hiển thị "Not Shrink version"
- ベンダー 0x1=Samsung かつ メモリ容量 0x2=2GBの場合：**Not Shrink version** → Nếu vendor là Samsung **VÀ ĐỒNG THỜI** dung lượng = 2GB → mới hiển thị "Not Shrink version"
- それ以外：**付けない** (Ngoài 2 trường hợp trên: **không hiển thị gì cả**)

**部品識別=0x1の場合 (Trường hợp mã nhận diện = 0x1):** tương tự logic trên nhưng cho "Shrink version":

- Micron (bất kỳ dung lượng) → "Shrink version"
- Samsung VÀ dung lượng 2GB → "Shrink version"
- Ngoài ra → không hiển thị gì

**それ以外：エラーメッセージ表示** (Các giá trị khác 0x0/0x1): hiển thị thông báo lỗi **"Error!! unknown chip was detected !!"**

**→ Vấn đề của logic cũ này**:

1. Nó chỉ hỗ trợ đúng 2 giá trị (0x0, 0x1) — không có chỗ cho "Shrink lần 2, lần 3..." — nếu có chip shrink thế hệ mới thì hệ thống sẽ báo lỗi "unknown chip".
2. Nó có điều kiện ràng buộc rất "vá víu": chỉ Samsung dung lượng đúng 2GB mới hiện được nhãn Shrink, các Samsung dung lượng khác thì im lặng — không nhất quán, khó bảo trì, và **không tương thích với vendor mới** (Nanya, Winbond, ISSI) vì các vendor này chưa từng được xét đến trong điều kiện.

---

## ・表示例 (Ví dụ hiển thị) — So sánh (既存) Hiện tại/cũ vs (変更) Sau khi thay đổi

|(既存) Hiện tại/Cũ|(変更) Sau khi thay đổi|
|---|---|
|**>ベンダー=0x0、部品識別=0x1** (Vendor=Micron, nhận diện=Shrink)<br>`lpddr4: Ch0 : Micron 4GB / Shrink version`<br>`lpddr4: Ch1 : Micron 4GB / Shrink version`|**>ベンダー=0x0、部品識別=0x1 （変更なし）** (không đổi)<br>`lpddr4: Ch0 : Micron 4GB / Shrink version`<br>`lpddr4: Ch1 : Micron 4GB / Shrink version`|
|**>ベンダー=0x1、部品識別=0x1** (Vendor=Samsung, nhận diện=Shrink)<br>`lpddr4: Ch0 : Samsung 4GB` ← **thiếu nhãn Shrink!** (vì Ch0 là 4GB, không phải 2GB nên điều kiện cũ không cho hiện)<br>`lpddr4: Ch1 : Samsung 2GB / Shrink version` ← Ch1 là 2GB nên mới hiện đúng|**>ベンダー=0x1、部品識別=0x1**<br>`lpddr4: Ch0 : Samsung 4GB / Shrink version` ← **giờ đã hiện đúng nhãn Shrink dù là 4GB**<br>`lpddr4: Ch1 : Samsung 2GB / Shrink version`|
|_(không có, vì trước đây gặp giá trị 0x2 sẽ báo lỗi)_|**>ベンダー=0x0、部品識別=0x2** (Vendor=Micron, nhận diện=Shrink thế hệ 2)<br>`lpddr4: Ch0 : Micron 4GB / Shrink 2`<br>`lpddr4: Ch1 : Micron 4GB / Shrink 2` ← **định dạng mới cho shrink đời sau**|
|_(không có, vì ISSI là vendor mới chưa từng tồn tại)_|**>ベンダー=0x5、部品識別=0x1** (Vendor=ISSI mới, nhận diện=Shrink)<br>`lpddr4: Ch0 : ISSI 4GB / Shrink version`<br>`lpddr4: Ch1 : ISSI 4GB / Shrink version` ← **vendor hoàn toàn mới giờ đã hiển thị được**|

---

## Tổng kết dễ hiểu 適用No.7

Trước đây, dòng log báo "chip nhớ này là của hãng nào, có phải bản shrink hay không" bị viết theo kiểu **liệt kê từng trường hợp cụ thể** (if Micron thì thế này, if Samsung-2GB thì thế kia, còn lại thì im lặng hoặc báo lỗi) — cách viết này chỉ đúng với 3 hãng (Micron, Samsung, Hynix) và chỉ 2 mức "có/không shrink". Khi công ty chuyển sang dùng thêm linh kiện từ các hãng mới (Nanya, Winbond, ISSI) hoặc dùng chip đã qua nhiều lần shrink (shrink 2, shrink 3...), logic cũ này **không đáp ứng được** — hoặc là im lặng không hiện thông tin, hoặc tệ hơn là báo lỗi "unknown chip".

Bản sửa No.7 làm 2 việc:

1. Mở rộng bảng mã vendor thêm 3 hãng mới (0x3=Nanya, 0x4=Winbond, 0x5=ISSI).
2. Viết lại logic hiển thị theo hướng **tổng quát hóa**: bỏ hẳn điều kiện ràng buộc riêng theo từng vendor/dung lượng, và cho phép hiển thị "Shrink _N_" với N từ 1 đến 14 (0x2~0xF), để log luôn hiển thị đúng và đầy đủ thông tin bất kể linh kiện thực tế lắp trên board là gì.