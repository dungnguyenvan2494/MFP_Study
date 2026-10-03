# 適用No.1 — Bản dịch tiếng Việt

**Nguồn: BSP残案件20221014より (Từ danh sách tồn đọng BSP ngày 2022/10/14)**

Cột: No. / Hạng mục lớn / Mục / Nội dung / Tình trạng / AI (người phụ trách) / Lịch sử xử lý-Ghi chú của AI / Ghi chú

|No|Hạng mục lớn|Mục|Nội dung|Tình trạng|AI|Lịch sử xử lý / Ghi chú|Ghi chú|
|---|---|---|---|---|---|---|---|
|1|CA training|Bug1|■Delay CA/CS/CLK của CA/CS training không được set đúng.<br>Trong `applyCsDlys()` và `applyCaDlys()`, việc set delay CA/CS/CLK được thực hiện theo điều kiện "taps" một cách cục bộ. Có thể mục đích là để giảm số lần ghi vào register — nếu giá trị không đổi so với lần set trước thì bỏ qua việc ghi đó — nhưng cách triển khai này có sai sót, dẫn đến giá trị cuối cùng không như mong đợi.|• Đã tạo xong source sửa lỗi<br>• Đã thống nhất nhận thức OK với anh Okamoto (Itami)||||
|2|CA training|Bug2|■Không set được delay của ch1.<br>Trong `applyCsDlys()` và `applyCaDlys()`, sau khi set register DDR PHY, thử thêm cơ chế đọc lại giá trị đó thì thấy ch0 hiển thị đúng giá trị đã set, nhưng ch1 luôn hiển thị 0.<br>Nghi ngờ register của ch1 bị sai.|• Đã thống nhất nhận thức OK với anh Okamoto (Itami)<br>• Đã tạo ROM sửa theo lỗi triển khai dự đoán, đã xác nhận hoạt động. Do không rõ IO mapping của DDR PHY nên chưa biết bản sửa có đúng hay không.|Điều tra các register DDR PHY (Kawamura)|**2022/10/07**<br>• Xác nhận hoạt động ROM dùng phần triển khai Control PHY theo bản RevA thì không có vấn đề (do IO MAP không rõ nên chưa biết bản sửa có đúng không).<br>**2022/10/03**<br>• Tài liệu spec của AP806 có ghi register DDR PHY. Do không có ghi chú về IO mapping nên chưa xác nhận được việc set delay (deskew) cho đúng đường tín hiệu đã đúng hay chưa.<br>**2022/9/29**<br>• Kiểm tra xem tài liệu spec có ghi register DDR PHY không (Kawamura) → Không có ghi chú<br>• Kiểm tra xem có tài liệu spec của IP không (Kawamura) → Trong danh sách IP không có ghi chú liên quan đến DDR PHY.<br>• Kiểm tra xem có tài liệu spec register DDR PHY ở đâu khác không (Kawamura) → Không tìm thấy. Các mục đã kiểm tra xem ở tab "Điều tra register DDR PHY"|_(có ảnh chụp đính kèm minh họa)_|
|3|CA training|Bug3|■Do giá trị khởi tạo vref mà vref bị set ra giá trị vượt ngoài spec.<br>range=1 vref=29 — trước giá trị này thì OK<br>range=1 vref=30 — từ giá trị này trở đi thì NG →set thành range=1 vref=51<br><br>Đang tính theo công thức sai:<br>`endVref[ch][cs] = vref + 51*range;`<br>Công thức đúng phải là:<br>`endVref[ch][cs] = vref + 30*range;`<br>※Không cần quy đổi phức tạp, có thể dùng thẳng giá trị `vrefSetting` — bản sửa đã dùng `vrefSetting`.|• Đã tạo xong source sửa lỗi<br>• Đã thống nhất nhận thức OK với anh Okamoto (Itami)||||
|4|CA training|Test pattern toàn miền "passed"|Từ trước đến nay chưa từng phát sinh kết quả test pattern bị "failed" lần nào, nên có nghi vấn liệu có đang hoạt động đúng hay không?|• Phần triển khai phán định test pattern (input/output của pattern) có vẻ không có vấn đề<br>• Setting vref có vẻ không có vấn đề<br>• Về setting delay thì có các bug liên quan (Bug1, Bug2)|Xác nhận lại sau khi sửa bug|**2022/10/7**<br>• Dù đã thực hiện đối sách Bug1~Bug3, phán định test pattern vẫn OK toàn miền.||
|5|CA training|Xem xét phạm vi vref|Đánh giá xem training với phạm vi ±8 tính từ giá trị khởi tạo vref có vấn đề gì không|• Chưa đủ tài liệu căn cứ để phán đoán|Liệt kê xem cần những tài liệu căn cứ nào|||
|6|CA training|Training theo từng ch (kênh)|■Có phải đang không training riêng theo từng ch không?<br>Training được thực hiện riêng theo ch, nhưng do giá trị khởi tạo vref không được phân theo ch riêng (dù register tồn tại riêng nhưng chỉ tham chiếu ch0), nên khi test pattern toàn miền cho ra cùng kết quả OK thì nhìn từ bên ngoài sẽ **trông như** không training riêng theo ch.<br>→ Nên sửa để giá trị khởi tạo vref được tham chiếu riêng theo từng ch.|• Đã tạo xong source sửa lỗi||**2022/09/30**<br>• Hoàn thành tạo source. Xác nhận hoạt động OK.||
|7|DQ training||Hiện tại chưa thấy sự cố cụ thể nào, nhưng nhìn theo tình hình của CA training thì cần điều tra thêm.||Tiến hành phân tích (Kawamura)|||
|8|DQ training|Training theo từng ch (kênh)|||Xác nhận từ mức độ cần thiết|||
|9|Đối sách cho trường hợp dung lượng khác nhau||Trường hợp ch0 và ch1 có kích thước khác nhau thì có cần thực hiện riêng theo ch hay không<br>→Giống như mục "Training theo từng ch"|||||
|10|Khác|Một phần log đo performance không xuất ra|Vấn đề một phần log đo performance không xuất ra.<br>Flag đo lường nằm trong tham số LPDDR4, khi bật flag này thì checksum sẽ không khớp. Do cơ chế song trùng hóa (2 bản) tham số LPDDR4, khi checksum không khớp thì sẽ tham chiếu bên backup. Vị trí tham chiếu flag khác nhau nên có chỗ tham chiếu bản gốc, có chỗ tham chiếu bản backup — dẫn đến có chỗ log ra, có chỗ không ra.|• Chỉ điều tra đến nguyên nhân|Phán định Close (đóng vấn đề)|**2022/10/4**<br>• Bổ sung. Hiện tại đây chưa phải là vấn đề bắt buộc phải sửa ngay.||

---

## Phần trích dẫn cuối trang: "Điều tra register DDR PHY" (điều tra cho Bug2)

- Trong danh sách IP có ghi chú về DDR PHY không? → Không có. → Đã xác định memory controller là **McKinley6**.
    
- Kiểm tra tài liệu spec của McKinley6. → McKinley6 không bao gồm DDR PHY, không có thông tin.
    
- Kiểm tra tài liệu spec của ARMADA 8K (được cho là nền tảng gốc của S800). → Không có ghi chú về spec register DDR PHY (giống như tài liệu spec S800).
    
- Kiểm tra source trên GitHub: `https://github.com/MarvellEmbeddedProcessors` → Không tìm thấy định nghĩa register DDR PHY hay phần triển khai liên quan đến training.
    
- Kiểm tra "hardware training mechanism" trong tài liệu spec S800. → Chỉ hỗ trợ DDR3/DDR4. LPDDR4 chỉ hỗ trợ pad calibration. → Có lẽ đây là lý do vì sao S800 phải tự triển khai SW training (training bằng phần mềm)? → Vì ARMADA có thể dùng "hardware training mechanism" cho DDR4, nên có khả năng không tồn tại phần triển khai/thông tin liên quan đến SW training.