# Bảng Pin gốc (không có chú thích tính toán) — Bản dịch + Giải thích chi tiết

Đây chính là **bảng gốc chưa bị chú thích thêm** (không có các ô vàng "Control 8本", "data 64本=16×4"...) mà tôi đã dịch từng phần rải rác ở các câu trả lời trước. Lần này mình dịch lại đầy đủ, sạch, thành 1 bảng hoàn chỉnh để bạn có bản tham chiếu gốc rõ ràng.

## Kiến thức nền (nhắc lại ngắn gọn)

Đây là bảng trích từ **datasheet chuẩn của IP/nhà cung cấp DDR PHY**, mô tả từng nhóm chân (pin) vật lý của khối điều khiển LPDDR4 trên chip, dùng cho 2 "channel" A và B (2 device nhớ độc lập chạy song song để tạo bus dữ liệu rộng hơn — đã giải thích chi tiết ở các câu trả lời trước).

## Bảng dịch đầy đủ

|Pin Name (Tên chân)|I/O|Pin Type|Power Rail|Description (Mô tả) — bản dịch tiếng Việt|
|---|---|---|---|---|
|`AP_M_A0_CLKOUT`<br>`AP_M_A0_CLKOUTn`<br>`AP_M_A1_CLKOUT`<br>`AP_M_A1_CLKOUTn`<br>`AP_M_B0_CLKOUT`<br>`AP_M_B0_CLKOUTn`<br>`AP_M_B1_CLKOUT`<br>`AP_M_B1_CLKOUTn`|O (Output)|LVSTL|AP_VDDO_M|**Clock (vi sai)**: Tất cả tín hiệu address, command và control đều được tham chiếu (đồng bộ) theo xung nhịp này. Mỗi channel (A, B) có 2 cặp xung nhịp.|
|`AP_M_A_CKE[3:0]`<br>`AP_M_B_CKE[3:0]`|O|LVSTL SDR|AP_VDDO_M|**Clock Enable**: CKE mức HIGH kích hoạt và CKE mức LOW vô hiệu hóa các tín hiệu xung nhịp nội bộ, bộ đệm đầu vào, và các driver đầu ra. Các chế độ tiết kiệm điện được vào/ra thông qua việc chuyển trạng thái CKE. CKE được lấy mẫu tại cạnh lên (rising edge) của CLKOUT.|
|`AP_M_A_CS[3:0]`<br>`AP_M_B_CS[3:0]`|O|LVSTL SDR|AP_VDDO_M|**Chip Select**: CS là 1 phần của mã lệnh (command code). Mỗi channel (A, B) có tín hiệu CS riêng của mình.|
|`AP_M_A_CA[5:0]`<br>`AP_M_B_CA[5:0]`|O|LVSTL SDR|AP_VDDO_M|**Command/address inputs (đầu vào lệnh/địa chỉ)**: Cung cấp các đầu vào lệnh và địa chỉ theo bảng chân lý lệnh (command truth table). Mỗi channel (A, B) có tín hiệu CA riêng của mình.|
|`AP_M_A0_DQ[15:0]`<br>`AP_M_A1_DQ[15:0]`<br>`AP_M_B0_DQ[15:0]`<br>`AP_M_B1_DQ[15:0]`|I/O|LVSTL DDR|AP_VDDO_M|**Data (Dữ liệu)**: Bus dữ liệu 2 chiều.|
|`AP_M_A0_DQS[1:0]`<br>`AP_M_A0_DQSn[1:0]`<br>`AP_M_A1_DQS[1:0]`<br>`AP_M_A1_DQSn[1:0]`<br>`AP_M_B0_DQS[1:0]`<br>`AP_M_B0_DQSn[1:0]`<br>`AP_M_B1_DQS[1:0]`<br>`AP_M_B1_DQSn[1:0]`|I/O|LVSTL DDR|AP_VDDO_M|**Data Strobe (vi sai)**: Các tín hiệu 2 chiều dùng để "gõ nhịp" (strobe) dữ liệu trong lúc thực hiện lệnh READ hoặc WRITE.|
|`AP_M_A0_DM[1:0]`<br>`AP_M_A1_DM[1:0]`<br>`AP_M_B0_DM[1:0]`<br>`AP_M_B1_DM[1:0]`|I/O|LVSTL DDR|AP_VDDO_M|**Data Mask (Che dữ liệu)**: Tín hiệu 2 chiều dùng đa mục đích — vừa dùng để chỉ ra phần dữ liệu nào cần bị che (mask), vừa dùng để chỉ dữ liệu nào bị đảo (inverted) trên bus.|
|`AP_M_RESETN`|O|CMOS|AP_VDDO_M|**RESET**: Reset không đồng bộ (kích hoạt ở mức thấp — active low).|

## Giải thích ý nghĩa từng cột

- **Pin Name**: tên chân chuẩn. Tiền tố `AP_M_` là quy ước đặt tên nội bộ (AP = Application Processor, M = Memory); `A`/`B` = channel A hoặc B; số `0`/`1` sau đó (ví dụ `A0_`, `A1_`) = 2 nhóm con trong cùng 1 channel (ứng với 2 "die" ghép cặp dùng chung CS như đã giải thích ở các câu trước); `[15:0]`, `[3:0]`, `[5:0]`, `[1:0]` = độ rộng bus của tín hiệu đó (số bit).
- **I/O**: hướng tín hiệu nhìn từ phía SoC (chip xử lý) — `O` (Output/chỉ xuất ra) nghĩa là SoC chỉ gửi tín hiệu này ra RAM chứ không nhận lại; `I/O` (2 chiều) nghĩa là tín hiệu này có lúc SoC gửi ra (khi WRITE - ghi), có lúc SoC nhận vào (khi READ - đọc).
- **Pin Type**: chuẩn điện áp/công nghệ vật lý của chân đó — `LVSTL` (Low Voltage Stub Series Terminated Logic, 1 chuẩn tín hiệu điện áp thấp dùng phổ biến cho LPDDR4), `SDR` (Single Data Rate — với CKE/CS/CA, dữ liệu chỉ lấy mẫu 1 lần mỗi chu kỳ xung nhịp), `DDR` (Double Data Rate — với DQ/DQS/DM, dữ liệu được lấy mẫu 2 lần mỗi chu kỳ xung nhịp, ở cả cạnh lên và cạnh xuống, giúp tăng gấp đôi băng thông), `CMOS` (chuẩn điện áp CMOS thông thường, dùng cho tín hiệu reset đơn giản không cần tốc độ cao).
- **Power Rail**: đường nguồn điện cấp cho nhóm chân đó — ở đây tất cả đều dùng chung `AP_VDDO_M` (1 đường nguồn I/O riêng dành cho khối Memory).
- **Description**: mô tả chức năng — đã dịch chi tiết ở cột trên.

## Vai trò của bảng này trong toàn bộ câu chuyện điều tra

Đây chính là **"nguồn chân lý"** (tài liệu tham chiếu gốc, đáng tin cậy nhất) mà đội kỹ sư trong 適用No.1 (Bug2 — "ch1 không set được delay") đã dùng để đối chiếu ngược lại, sau khi không tìm thấy tài liệu register DDR PHY chính thức nào khác (đã tìm khắp IP list, ARMADA 8K, GitHub Marvell...). Từ bảng pin chuẩn này, họ đã:

1. Đếm số lượng chân thực tế của từng loại tín hiệu (CLKOUT=8, CKE=8, CS=8, CA=12...).
2. Phát hiện ra CS và CKE **thiếu hụt** so với công thức lý thuyết ban đầu (dẫn đến chú thích "4→4不足" ở các trang trước).
3. Dùng đúng tên pin trong bảng này (`A_CS0`, `A0_CLKOUT`...) để chuẩn hóa lại toàn bộ ký hiệu `ch0`/`ch1` mơ hồ trong code, từ đó xây dựng nên sơ đồ "Control IO MAP", "Data IO MAP" và cuối cùng xác định chính xác nguyên nhân bug nằm ở việc phần mềm truy cập nhầm vào các "pup" (đơn vị IO vật lý) không tồn tại cho ch1.