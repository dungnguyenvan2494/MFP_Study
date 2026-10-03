# 11 — Write Deskew: 3 hàm thật, TRN_J0→J2

Đọc `00`-`10` trước, đặc biệt `09` (Read Deskew — Write Deskew khác cấu trúc rõ rệt, không phải bản sao đổi hướng).

# Concept

Giống Read Deskew (tài liệu 09): mỗi đường DQ/DM khi ghi cũng có skew riêng, cần chỉnh delay riêng từng bit. Nhưng **điểm khởi đầu để deskew khác hẳn**: Read Deskew tự quét từ 0 tìm left-edge trước khi có centering nào; Write Deskew **bắt đầu từ giá trị đã được Write Centering (tài liệu 10) tìm trước** rồi dò lệch quanh đó.

# Why Write Deskew khác Read Deskew (trả lời trực tiếp câu hỏi trong outline)

[SOURCE FACT] So sánh trực tiếp qua chính comment gốc trong source:

| | Read (`lpddr4_read_find_deskew_start`) | Write (`lpddr4_write_find_deskew_start`) |
|---|---|---|
| Comment gốc | "Determines first point at which all DQs and DM **pass**" | "Determine where the first bit **fails** during write centering" |
| Điểm bắt đầu quét | `dly = 0` (từ đầu) | `dly = wcValue[ch][cs][pup]` (giá trị Write Centering đã tìm — tài liệu 10) |
| Mục tiêu | Tìm LEFT EDGE (điểm PASS đầu tiên), làm baseline AN TOÀN cho bước sau | Tìm điểm FAIL (của DM hoặc DQ, lấy MIN), làm baseline "sát biên" cho bước sau |
| Số hàm | 5 (start, dm_deskew, deskew, find_end, center_align) | 3 (start, dm_deskew, deskew — không có find_end/center_align riêng) |

🧠[INFERENCE hợp lý]: Read Deskew phải tự tìm baseline từ đầu vì nó chạy **trước** khi có bất kỳ centering nào cho từng bit riêng (chỉ có DQS Gate ở mức pup). Write Deskew chạy **sau** khi `lpddr4_write_centering()` (tài liệu 10) đã tìm 1 điểm center hợp lý ở mức pup — nên nó có thể tận dụng luôn điểm đó (`wcValue`) làm điểm khởi đầu, không cần dò lại từ 0. Đây cũng giải thích vì sao Write không cần "find_deskew_end" + "center_align" riêng: vì Write Deskew đã bắt đầu **gần center thật** (nhờ `wcValue`), tìm 1 biên (fail point) và lùi lại là đủ để ước lượng đơn giản hơn, không cần đo đủ cả 2 biên rồi tính trọng tâm/center riêng như Read.

# Firmware view — trace 3 hàm thật (TRN_J0→J2)

## Bước 1 — `lpddr4_write_find_deskew_start()` — TRN_J0

[SOURCE FACT — `:5266-5394`, đã đọc toàn bộ]

```c
for ( dly = wcValue[ch][cs][pup]; dly < 64; dly++ )   // bắt đầu TỪ giá trị Write Centering, không phải 0
{
    nova_ddrphy_write(0x1 + cs*0x4, 0, pup+4*ch, dly);
    result = fifo_test(mc_reg, ch, cs, pup/2);
    if ( result & (1<<dmLocation[pup]) || dly==63 )
        dmFail[ch][cs][pup] = dly;          // điểm DM bắt đầu FAIL
}
// ... lặp lại tương tự cho DQ, ra dqFail[ch][cs][pup] ...

deskewStart[ch][cs][pup] = min(dmFail[ch][cs][pup], dqFail[ch][cs][pup]);   // lấy biên GẦN HƠN
```
**Input**: `wcValue` (từ Write Centering, tài liệu 10). **Output**: `deskewStart[ch][cs][pup]` — biên FAIL gần nhất (của DM hoặc DQ, cái nào fail trước).

## Bước 2 — `lpddr4_write_dm_deskew()` — (giữa J0 và J1, không có mốc riêng)

[SOURCE FACT — `:5397-5488`, đã đọc toàn bộ]

```c
nova_ddrphy_write(0x1+cs*0x4, 0, pup+4*ch, deskewStart[ch][cs][pup]-1);   // set baseline = ngay TRƯỚC điểm fail
for ( dly=0; dly<32; dly++ )
{
    nova_ddrphy_write(0x13+cs*0x10, 0, i+4*ch, dly);   // set delay riêng DM
    result = fifo_test(...);
    if ( result & (1<<dmLocation[...]) || dly==MAX_DESKEW )
        minWRDQDly[ch][cs][pup] = dly;
}
```
**Input**: `deskewStart`. **Output**: `minWRDQDly[ch][cs][pup]` — delay riêng của DM, đo từ baseline mới.

## Bước 3 — `lpddr4_write_deskew()` — TRN_J2

[SOURCE FACT — `:5491-5526+`, đã đọc phần đầu (khai báo + khởi tạo); ❓[UNKNOWN — chưa đọc hết thân hàm] chi tiết vòng sweep chính cho từng bit DQ]. Comment gốc: "Aligns DQs for LPDDR4 writes". Cấu trúc khai báo (`dqDly[2][2][32]`, dùng `dqRegAddr[ch][32]` — bảng địa chỉ đánh phẳng 32 vị trí/channel, giống cách `read_deskew_center_align` dùng ở tài liệu 09) gợi ý hàm này chỉnh **riêng từng bit DQ trong số 32 bit/channel** (4 pup × 8 bit), tương tự Read Deskew, nhưng 🧠 chưa xác nhận công thức center cuối cùng có chia đôi (`/2`) như bản Read hay không — cần đọc tiếp phần thân hàm ở 1 session sau.

Hàm này nhận cả `warmboot`/`train_reg` (khác 2 hàm trước không có tham số này) — 🧠[INFERENCE, nhất quán với phát hiện ở tài liệu 07/08/10]: rất có khả năng khi `warmboot=1`, hàm này đọc lại giá trị đã lưu từ `train_reg` thay vì sweep — cùng khuôn mẫu đã thấy lặp lại nhiều lần, nhưng ❓UNKNOWN chưa đọc đủ thân hàm để xác nhận 100% cho riêng hàm này.

# Register view

| Register/biến | Ý nghĩa |
|---|---|
| PHY sub-reg `0x1 + cs*0x4` | Delay mức pup cho Write (tương ứng `0x3+cs*0x4` dùng cho Read ở tài liệu 09) |
| PHY sub-reg `0x13 + cs*0x10` | Delay riêng cho DM khi Write (tương ứng `0x53+cs*0x10` dùng cho Read) |
| `wcValue[ch][cs][pup]` | Kết quả Write Centering (tài liệu 10) — điểm khởi đầu cho deskew |
| `deskewStart[ch][cs][pup]` | Biên FAIL gần nhất, DM hoặc DQ (output Bước 1) |
| `minWRDQDly[ch][cs][pup]` | Delay riêng DM khi ghi (output Bước 2) |

# Source view

- `mv_lpddr4_apn806.c:5266-5394` — `lpddr4_write_find_deskew_start()` — đọc toàn bộ
- `mv_lpddr4_apn806.c:5397-5488` — `lpddr4_write_dm_deskew()` — đọc toàn bộ
- `mv_lpddr4_apn806.c:5491-5526+` — `lpddr4_write_deskew()` — chỉ đọc phần đầu
- `mv_lpddr4_apn806.c:8413-8430` — call site thật, mốc `TRN_J0`/`TRN_J1`/`TRN_J2`

# Call flow

```
(sau Write Centering lần 1 — TRN_IX, tài liệu 10)
        |
        v
lpddr4_write_find_deskew_start()   → TRN_J0   [từ wcValue, tìm biên FAIL gần nhất (DM hoặc DQ)]
        |
        v
lpddr4_write_dm_deskew()            → (TRN_J1) [set baseline = biên-1, deskew riêng DM]
        |
        v
lpddr4_write_deskew()                → TRN_J2   [🧠 deskew riêng từng bit DQ — chưa đọc hết]
        |
        v
(Write Centering lần 2 — TRN_KX, tài liệu 10)
```

# K-S800 customization

Không thấy `#ifdef KM_*` đặc thù bọc riêng 3 hàm này (giống nhận xét ở tài liệu 09 cho Read Deskew) — 🧠[INFERENCE] khả năng cao đây cũng là code gần với Marvell gốc.

# Debugging

- Nếu `deskewStart[ch][cs][pup]` luôn bằng 63 (giá trị sentinel/max) — nghĩa là quét hết 64 giá trị mà cả DM và DQ **không hề fail** — bất thường (Write Centering trước đó có thể đã chọn 1 điểm quá an toàn/xa Window thật) — kiểm tra lại kết quả Write Centering (tài liệu 10) trước khi nghi ngờ chính Write Deskew.
- So sánh `dmFail` và `dqFail` riêng — nếu 1 trong 2 luôn nhỏ hơn hẳn cái còn lại trên nhiều pup, gợi ý DM và DQ có đặc tính lệch khác biệt hệ thống trên board đó (không phải nhiễu ngẫu nhiên).

# Summary

- Write Deskew có 3 hàm (không phải 5 như Read) vì nó khởi đầu gần center thật (nhờ Write Centering chạy trước), không cần đo đủ cả 2 biên riêng.
- Chiến lược ngược hướng so với Read: Read tìm "PASS đầu tiên" làm baseline; Write tìm "FAIL đầu tiên" (từ 1 điểm đã gần center) làm baseline.
- DM vẫn cần thủ thuật swap riêng, giống Read.
- Hàm cuối (`lpddr4_write_deskew`) chưa đọc hết thân — phần công thức center cuối cùng cho từng bit DQ khi ghi vẫn là ❓UNKNOWN, chỉ suy luận structure tương tự Read.

# Verify yourself

1. Vì sao Write Deskew có thể "tiết kiệm" 2 hàm (không cần find_end/center_align riêng) so với Read?
2. `deskewStart = min(dmFail, dqFail)` — tại sao lấy MIN (biên gần hơn) mà không lấy MAX?
3. Nếu Write Centering (tài liệu 10) chọn 1 điểm center rất tệ (gần biên thật), điều gì sẽ xảy ra với `lpddr4_write_find_deskew_start()` khi nó bắt đầu quét từ điểm đó?
4. Tại sao tài liệu này đánh dấu công thức center cuối của Write Deskew là ❓UNKNOWN trong khi Read Deskew (tài liệu 09) đã xác nhận được công thức `/2`? Bạn nghĩ nên làm gì tiếp theo để lấp đầy chỗ này?
5. So sánh bảng ở đầu tài liệu (Read vs Write find_deskew_start) — nếu phải tóm tắt sự khác biệt triết lý trong 1 câu, bạn sẽ viết thế nào?

**Tiếp theo:** `12_MEMORY_TEST.md` — `common_memfill()`, phân biệt Training vs Validation, và backup/restore DRAM data (nối tiếp `backup_train_area()` đã thấy ở tài liệu 10).
