# 03 — Training Concept: Valid Window, Eye, Center, Margin

Đọc `00`, `01`, `02` trước. File này xây khái niệm nền mà **mọi** thuật toán training cụ thể ở tài liệu 04-11 sẽ dùng lại — không đọc file nào trong số đó trước khi hiểu file này.

# Concept

Khi firmware thử một giá trị delay/Vref cụ thể và kiểm tra dữ liệu đọc/ghi có đúng không, kết quả chỉ có 2 loại: **PASS** (đúng) hoặc **FAIL** (sai). Nếu bạn quét (sweep) qua toàn bộ dải giá trị có thể (ví dụ delay từ 0 đến 63), bạn sẽ được một dãy PASS/FAIL liên tục:

```
delay:   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
result:  F  F  F  F  P  P  P  P  P  P  P  P  F  F  F  F
                     ^                    ^
                left edge             right edge
                (delay=4)             (delay=11)
```

- **Valid Window**: đoạn các giá trị PASS liên tục (delay 4→11 ở trên).
- **Eye** (con mắt): tên gọi hình ảnh của Valid Window khi vẽ trên 2 trục cùng lúc — trục thời gian (delay) và trục điện áp (Vref) — vùng PASS trông giống hình quả trứng/con mắt khi vẽ 2D. S800 thực sự làm training theo cả 2 trục này (xem mục Source view).
- **Left edge / Right edge**: biên trái/phải của Valid Window — điểm PASS đầu tiên và PASS cuối cùng trong dãy quét.
- **Center**: điểm giữa của Window, `center = left_edge + (right_edge - left_edge) / 2`.
- **Margin**: khoảng cách từ điểm đang dùng tới biên gần nhất — đo "còn bao nhiêu chỗ để trôi" trước khi rơi vào FAIL.

# Why

📘[GENERAL KNOWLEDGE] Nếu chỉ tìm **1 điểm PASS** rồi dừng (ví dụ chọn ngay delay=4 ở trên), hệ thống *chạy được ngay lúc train*, nhưng cực kỳ mong manh: chỉ cần nhiệt độ tăng nhẹ, điện áp dao động nhẹ, hoặc chip già đi theo thời gian (aging) khiến Window dịch đi vài đơn vị — delay=4 có thể rơi ra ngoài Window mới → FAIL ngay lúc chạy thật (không phải lúc train). Chọn **center** (delay=7 hoặc 8 ở trên) cho khoảng đệm đều 2 phía — chống chịu được dịch chuyển theo cả 2 hướng, không chỉ 1 hướng. Đây chính là lý do spec S800 có mục "Window Margin" đo riêng ở Appendix A.1, và lý do spec khẳng định không cần retrain theo nhiệt độ (`00_MASTER_OVERVIEW.md` — SPEC FACT về 0-60°C) — margin được thiết kế đủ rộng để chịu được dịch chuyển do nhiệt.

# Real-world analogy

Nối lại ví dụ "hô rồi bắt bóng" ở tài liệu 00: nếu bạn thử hô "bắt" ở nhiều thời điểm khác nhau và ghi lại thời điểm nào bạn bắt được bóng, bạn sẽ có một khoảng thời gian (không phải 1 điểm) mà việc bắt luôn thành công. Chọn đúng **giữa** khoảng đó (không chọn ngay sát đầu hay cuối) để nếu tay bạn phản xạ chậm hơn 1 chút vào lần sau (tương đương "nhiệt độ tăng" làm chậm mạch điện), bạn vẫn còn dư thời gian để bắt được.

# Hardware view

Mỗi lần "quét 1 giá trị + kiểm tra PASS/FAIL" trong S800 là 1 giao dịch PHY thật (`nova_ddrphy_write`/`read`, xem tài liệu 02) cộng với 1 phép test đọc/ghi dữ liệu thật (FIFO test hoặc DMA test — tài liệu 12 sẽ giải thích 2 loại này). Không có mạch nào "biết trước" Window nằm ở đâu — firmware phải thực sự set từng giá trị, gửi lệnh, đọc kết quả, rồi mới biết PASS hay FAIL.

# Firmware view — 3 ví dụ thật trong S800, 3 thuật toán khác nhau, cùng 1 công thức

## Ví dụ 1: DQS Gate training — center theo trục **thời gian** (delay)

[SOURCE FACT — `atf\atf_s800\ble\mv_lpddr4_apn806.c:5806-5828`]:

```c
if ( ( nova_ddrphy_read(0x9A, 0, pup + 4*ch) == 0x100 ) &&
     ( nova_ddrphy_read(0x9B, 0, pup + 4*ch) == 0x3F7F ) )      // ← điều kiện PASS thật
{
    if ( passing[ch][cs][pup] == 0 )                             // lần đầu PASS
    {
        passing[ch][cs][pup] = 1;
        first_valid_phy_dly[ch][cs][pup] = phy_dly;               // ← LEFT EDGE
        last_valid_phy_dly[ch][cs][pup]  = phy_dly;
        window[ch][cs][pup] = 1;
    }
    else
    {
        last_valid_phy_dly[ch][cs][pup] = phy_dly;                // ← cập nhật RIGHT EDGE mỗi lần PASS
        window[ch][cs][pup]++;
        if (window[ch][cs][pup] > 63)
        {
            PHYDelay[ch][cs][pup] = first_valid_phy_dly[ch][cs][pup]
                + (last_valid_phy_dly[ch][cs][pup] - first_valid_phy_dly[ch][cs][pup]) / 2;  // ← CENTER
        }
    }
}
```

PASS ở đây được định nghĩa **cụ thể bằng 2 giá trị register thật**: đọc register PHY `0x9A` phải ra đúng `0x100` **và** register `0x9B` phải ra đúng `0x3F7F` — đây không phải diễn giải trừu tượng "PASS nghĩa là đúng", mà là 2 con số chính xác firmware so khớp.

## Ví dụ 2: Read Centering — center theo trục thời gian, đã lọc theo Vref tốt nhất

[SOURCE FACT — `mv_lpddr4_apn806.c:4991-5014`]: trước tiên tính Vref trung bình tốt nhất (`selectedVref`), sau đó dùng `fpDly`/`lpDly` (**f**irst-**p**ass delay / **l**ast-**p**ass delay — chính là left/right edge) **tại đúng Vref đó** để tính center:

```c
selectedVref[ch] = selectedVref[ch] / (4 * num_cs);   // Average VREF = Total VREF / (PUP*CS)
...
selectedCenter[ch][cs][pup] = fpDly[selectedVref[ch]][ch][cs][pup][0]
    + (lpDly[selectedVref[ch]][ch][cs][pup][0] - fpDly[selectedVref[ch]][ch][cs][pup][0]) / 2;
if (selectedCenter[ch][cs][pup] < 0)
    selectedCenter[ch][cs][pup] += maxDly;    // ← wrap-around: không gian delay là vòng (cycle), không phải đường thẳng
```

⚠️ Chi tiết hay bị bỏ sót: dòng `if (selectedCenter < 0) selectedCenter += maxDly` cho thấy không gian delay ở đây **không phải một đường thẳng 0→63 đơn giản** — nó có thể "vòng lại" (giống mặt đồng hồ), vì delay thực chất là độ lệch pha trong 1 chu kỳ clock. Sơ đồ ASCII "INVALID|VALID|INVALID" đơn giản ở phần lý thuyết **không thể hiện** khả năng Window "tràn qua mép" này — đây là 🧠[INFERENCE] hợp lý dựa trên code thật, không phải giả định lý thuyết.

Biến thể bo tròn lên (dùng khi độ rộng window là số lẻ) [SOURCE FACT — `mv_lpddr4_apn806.c:5091`]:
```c
selectedCenter[...] = fpDly[...] + (lpDly[...] - fpDly[...] + 1) / 2;   // +1 trước khi chia 2 → bo lên
```

## Ví dụ 3: CA training (CBT) — center theo trục **điện áp** (Vref), không phải delay

[SOURCE FACT — `mv_lpddr4_apn806.c:7101-7132`]:
```c
startVref[ch][cs] = vrefSetting;     // LEFT EDGE (Vref thấp nhất còn PASS)
endVref[ch][cs]   = startVref[ch][cs];  // cập nhật dần thành RIGHT EDGE (Vref cao nhất còn PASS)
...
vref = startVref[ch][cs] + (endVref[ch][cs] - startVref[ch][cs]) / 2;   // ← CENTER, cùng công thức, khác trục
```

**Kết luận quan trọng**: `center = left + (right - left) / 2` là **1 công thức được lặp lại y hệt ở ít nhất 3 thuật toán khác nhau** (DQS gate theo delay, Read Centering theo delay-tại-Vref-tốt-nhất, CA training theo Vref) — không phải trùng hợp, mà là **nguyên lý thiết kế chung** của toàn bộ FW training này. Khi đọc các tài liệu 04-11 sau, hãy tìm lại đúng pattern "quét → ghi nhận first/last PASS → center = trung điểm" ở mỗi thuật toán, thay vì học lại từ đầu.

# Register view

| Register | Offset (theo `nova_ddrphy_read`) | Ý nghĩa trong ví dụ DQS Gate | Nguồn |
|---|---|---|---|
| `0x9A` (qua PRFA) | sub-reg PHY | Phải đọc ra đúng `0x100` để coi là PASS | [SOURCE FACT `:5806`] |
| `0x9B` (qua PRFA) | sub-reg PHY | Phải đọc ra đúng `0x3F7F` để coi là PASS | [SOURCE FACT `:5806`] |

❓[UNKNOWN]: ý nghĩa bit-field chi tiết của `0x9A`/`0x9B` (PHY sub-register nào, field gì) chưa được xác nhận từ `MC_regmasks.h` hay Appendix.C của spec — sẽ tra cứu ở tài liệu 07 (DQS Gate) và 18 (Register Map), không đoán ý nghĩa bit ở đây.

# Source view

- `mv_lpddr4_apn806.c:5794-5829` — DQS gate edge/window/center (trục delay)
- `mv_lpddr4_apn806.c:4977-5018` — Read centering, kết hợp Vref + delay (`calculate_com()` tại `:4977` — hàm này chưa được đọc chi tiết, để dành tài liệu 08)
- `mv_lpddr4_apn806.c:5091` — biến thể center bo lên (+1)/2
- `mv_lpddr4_apn806.c:7100-7132` — CA training/CBT, center theo Vref
- `mv_lpddr4_apn806.c:11092-11102` (`findMinWindow`) — hàm phụ trợ tìm window nhỏ nhất giữa 2 CS, dùng khi cả 2 CS phải cùng thoả 1 delay chung
- `mv_lpddr4_apn806.c:13163-...` (`calculate_window_margin`) — hàm **thống kê/báo cáo**, không phải hàm tìm center — tính trung bình/max/min của Vref, windowWidth, Center qua nhiều lần retry (`retry_num`), dùng để in log đo margin (ứng với Appendix A.1 "ウィンドウマージン" của spec) — ⚠️ đừng nhầm đây là thuật toán centering, nó chỉ đo lại kết quả sau khi đã centering xong

# Call flow (mẫu chung, rút ra từ 3 ví dụ)

```
for mỗi giá trị thử (delay hoặc vref) trong dải quét:
        set giá trị thử (nova_ddrphy_write hoặc set VREF_TRAINING field)
        chạy phép test (fifo_test / write_data_test2 / DRAM MR14 CBT feedback)
        if PASS:
            if đây là lần PASS đầu tiên: ghi nhận LEFT EDGE
            luôn cập nhật RIGHT EDGE = giá trị hiện tại
        (một số thuật toán dừng ngay khi FAIL sau khi đã có ít nhất 1 PASS —
         xem tài liệu 04-11 để xác nhận từng thuật toán cụ thể có dừng sớm hay quét hết dải)
CENTER = LEFT EDGE + (RIGHT EDGE - LEFT EDGE) / 2
ghi CENTER vào register PHY thật (nova_ddrphy_write) để dùng cho vận hành bình thường
```

⚠️[SPEC/SOURCE MISMATCH — chưa xác nhận]: mental model rút gọn ở trên viết "dừng ngay khi FAIL sau PASS đầu" nhưng DQS gate code thật (`:5824` `if (window > 63)`) cho thấy nó **quét tới 63 lần PASS liên tiếp mới dừng**, không dừng ngay ở FAIL đầu tiên sau đó — cách dừng vòng lặp thực tế phức tạp hơn 1 mô hình "quét tới FAIL rồi dừng" đơn giản. Sẽ xác nhận chi tiết điều kiện dừng đầy đủ ở tài liệu 07.

# Diagram

```
Trục delay (thời gian) — DQS Gate / Read Centering:
   FAIL FAIL [PASS PASS PASS PASS PASS PASS] FAIL FAIL
              ^first_valid                ^last_valid
                        \___________________/
                          center = trung điểm  → ghi vào PHYDelay[ch][cs][pup]

Trục Vref (điện áp) — CA training CBT:
   FAIL [PASS PASS PASS PASS] FAIL
        ^startVref          ^endVref
                  \___________/
                 vref = trung điểm
```

# K-S800 customization

[SOURCE FACT — comment trong code tại `mv_lpddr4_apn806.c:7101`]: có một comment tiếng Nhật gắn tên người và mã lỗi nội bộ, ghi ngày sửa `2025/03/07`, nội dung đại ý "sửa lỗi tính sai khi đổi đơn vị Vref" (`vref換算ミスの修正`), đúng ngay tại dòng `startVref[ch][cs] = vrefSetting;`. ⚠️ Theo nguyên tắc source-grounding: **comment có thể sai/lỗi thời**, không tự suy ra nội dung bug thật chỉ từ comment — đây chỉ là **manh mối** cho thấy công thức Vref ở khu vực CA training đã từng bị sửa sau khi phát hành spec 第2版 (18/11/07), nghĩa là **source hiện tại có thể mới hơn spec** ở phần CA-training/Vref. Việc này sẽ được điều tra kỹ (đọc `git log`/diff nếu có, hoặc so sánh logic hiện tại với Appendix B của spec) ở tài liệu 22 (Bug Analysis) — không kết luận gì thêm ở đây.

# Debugging

Nếu training FAIL ở bất kỳ thuật toán centering nào (đọc log thấy window quá nhỏ hoặc không tìm được left/right edge):

1. Kiểm tra xem log có in ra `first_valid_phy_dly`/`last_valid_phy_dly` (hoặc `startVref`/`endVref`) hợp lý không — nếu 2 giá trị này bằng nhau hoặc left > right, nghĩa là **không tìm được Window nào cả** (mọi giá trị quét đều FAIL) → nghi ngờ tầng thấp hơn (pad calibration, CA training) chưa đúng trước khi tới bước centering này.
2. Nếu Window tìm được quá hẹp (ví dụ chỉ 1-2 giá trị PASS) — đây là dấu hiệu signal integrity kém (tài liệu 01, mục 10) hoặc chip/board có vấn đề, không phải lỗi logic centering.
3. So khớp với log `calculate_window_margin()` (thống kê qua nhiều lần retry) để biết Window có **ổn định giữa các lần retry** hay dao động mạnh — dao động mạnh gợi ý vấn đề nhiễu/nguồn điện, không phải lỗi thuật toán.

# Summary

- Valid Window = đoạn PASS liên tục khi quét 1 tham số (delay hoặc Vref).
- Eye = Window nhìn theo 2 trục (thời gian + điện áp) cùng lúc.
- Center = trung điểm Window, công thức `left + (right-left)/2`, **lặp lại giống nhau ở DQS gate, Read Centering, và CA training/CBT** trong chính source S800 — không phải lý thuyết suông.
- Margin = khoảng đệm còn lại quanh điểm đang dùng — Window rộng thì Margin nhiều, chịu được nhiệt độ/nguồn điện dao động mà vẫn PASS.
- Không gian delay có thể "vòng" (wrap-around) — không phải luôn là đường thẳng 0→max.
- `calculate_window_margin()` là hàm **đo lại/báo cáo**, khác với hàm **tìm center** — 2 vai trò khác nhau, đừng nhầm.

# Verify yourself

1. Nếu PASS ở delay 10-20 (left=10, right=20), center theo công thức trên là bao nhiêu? Margin ở center đó về mỗi phía là bao nhiêu?
2. Vì sao dùng `+1` trước khi chia 2 (biến thể ở dòng 5091) lại tạo ra bo-lên (round up) thay vì bo-xuống?
3. Việc PASS được định nghĩa bằng chính xác 2 giá trị register (`0x9A==0x100` và `0x9B==0x3F7F`) khác gì so với việc chỉ đọc lại dữ liệu ghi/đọc DRAM rồi so sánh byte-by-byte? Cả 2 cách "kiểm tra PASS" này đều tồn tại trong S800 — bạn nghĩ tại sao DQS gate cần cách đầu, còn Read Centering (FIFO/DMA test) cần cách sau?
4. Nếu `selectedCenter` tính ra âm và phải `+= maxDly`, điều đó gợi ý gì về ý nghĩa vật lý của "delay âm" trong 1 hệ thống dựa trên pha của clock?
5. Bug-fix comment ở dòng 7101 (`vref換算ミスの修正`, 2025/03/07) — theo nguyên tắc source-grounding, bạn được phép kết luận gì từ 1 comment, và không được phép kết luận gì?

**Tiếp theo:** `04_PAD_CALIBRATION.md` — tại sao pad calibration tồn tại, và trace hàm calibration thật đầu tiên trong chuỗi training (TRN_B0/B1/B2).
