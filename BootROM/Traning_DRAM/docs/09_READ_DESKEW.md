# 09 — Read Deskew: 5 hàm thật, TRN_H0→H4

Đọc `00`-`08` trước. Đây là phần user's outline gọi là "khó" — đọc kỹ Concept trước khi vào Source view.

# Concept

Tài liệu 07-08 coi **cả 1 pup (byte-lane, 8 bit DQ)** như 1 khối, chỉnh 1 delay DQS chung cho cả 8 bit. Nhưng thực tế: mỗi đường dây DQ riêng lẻ (DQ0, DQ1, ..., DQ7 trong cùng 1 pup) có độ dài PCB, tải điện hơi khác nhau → **mỗi bit có skew riêng, khác nhau vài phần trăm của 1 bit-time**, dù cùng chung 1 DQS. Read Deskew = chỉnh delay **riêng cho từng bit DQ** (và cho DM — data mask, 1 tín hiệu đặc biệt) để tất cả cùng "về đúng nhịp" với DQS chung của pup đó.

# Why

Nếu không deskew riêng từng bit: giả sử pup có Window chung (theo tài liệu 07) là delay 10-30, nhưng DQ3 thực ra chỉ PASS ở 15-25 (do lệch riêng) — tại delay 12 (vẫn trong Window "chung"), DQ3 đã sai rồi. Deskew từng bit đảm bảo **mọi bit trong pup cùng PASS tại đúng 1 điểm lấy mẫu chung**, không chỉ "đa số bit PASS".

# Hardware view — vì sao DM cần thủ thuật riêng ("DM to DQ swap")

📘[GENERAL KNOWLEDGE + SOURCE FACT]: DM (Data Mask, tài liệu 01) không có sẵn 1 mạch deskew riêng độc lập trong PHY như từng bit DQ. [SOURCE FACT — `mv_lpddr4_apn806.c:3588, 2653, 3670, 2754`]: firmware dùng 1 thủ thuật — ghi vào register `CSS_ADDR+0x0100` (field mask `0xff0`) để **tạm "hoán đổi" (swap) đường DM vào đúng vị trí 1 bit DQ** trong lúc test, deskew DM y như đang deskew 1 DQ bình thường, rồi **hoán đổi lại** (`0x000, 0xff0`) khi xong. Đây là lý do các hàm liên quan DM luôn có cặp lệnh "swap vào" ở đầu và "swap ra" ở cuối.

# Firmware view — trace 5 hàm theo đúng thứ tự chạy thật (TRN_H0→H4)

## Bước 1 — `lpddr4_read_find_deskew_start()` — TRN_H0

[SOURCE FACT — `:3565-3673`, comment gốc: "Determines first point at which all DQs and DM pass", đã đọc toàn bộ]

Quét `dly` từ 0 tăng dần, set **CẢ 1 nhóm 2 pup ("device") cùng lúc** (chưa tách riêng từng bit DQ — chỉ tách riêng theo vị trí DM, qua `dmLocation[4]={7,11,2,10}` — vị trí bit DM trong kết quả test), tìm điểm `dly` đầu tiên mà bit DM tại vị trí đó PASS:
```c
regval = read_dq_cal_test(mc_reg, device, ch, cs, device);
if ( !(regval & (1 << dmLocation[2*device])) && rc_start[..]==-1 )
    rc_start[ch][cs][2*device] = dly;      // ← LEFT EDGE thô, mức pup, dùng vị trí bit DM để dò
```
**Input**: không có (bắt đầu từ dly=0). **Output**: `DQSStart[ch][cs][pup]` — 1 điểm khởi đầu AN TOÀN (đã chắc chắn PASS) cho toàn pup, dùng làm baseline cho các bước sau — **đây chưa phải giá trị per-bit cuối cùng**, chỉ là điểm bắt đầu để đỡ phải quét lại từ 0 ở các bước sau.

## Bước 2 — `lpddr4_read_dm_deskew()` — TRN_H1

[SOURCE FACT — `:2637-2769`, comment gốc: "aligns DM LPDDR4 for reads", đã đọc toàn bộ]

Set DQS về đúng `DQSStart` vừa tìm (baseline), rồi quét `dly` từ `prePBS` tăng dần, set delay riêng cho từng vị trí DM:
```c
for ( dly = prePBS; dly < MAX_DESKEW+1; dly++ )
{
    nova_ddrphy_write(0x53 + cs*0x10, 0, i+2*device+4*ch, dly);   // set delay CHO RIÊNG DM
    regval = read_dq_cal_test(...);
    if ( (regval & (1<<dmLocation[i+2*device])) || dly==MAX_DESKEW )
        pbsMinDly[ch][cs][i+2*device] = dly;    // ← LEFT EDGE riêng của DM
}
```
**Input**: `DQSStart` (từ Bước 1). **Output**: `pbsMinDly[ch][cs][pup]` — left-edge delay riêng cho tín hiệu DM tại mỗi pup (biến này chính là nguồn gốc tên "PBS" — 🧠[INFERENCE, chưa xác nhận chữ viết tắt đầy đủ, ❓UNKNOWN]: rất có khả năng PBS = "Per-Bit Skew", dựa trên tên biến `pbsMinDly` = "PBS Minimum Delay" xuất hiện đúng ngay tại bước deskew-per-bit này).

## Bước 3 — `lpddr4_read_deskew()` — TRN_H2

[SOURCE FACT — header only, `:3107-3113`, comment gốc: "aligns DQs for LPDDR4 reads"; ❓[UNKNOWN — chưa đọc thân hàm ~294 dòng chi tiết] cơ chế sweep nội bộ chính xác]. 🧠[INFERENCE hợp lý dựa trên cấu trúc 4 hàm còn lại đều theo cùng khuôn mẫu "sweep dly, set riêng từng pup/bit, kiểm tra bit tương ứng trong `regval`"]: đây là bước **tương tự Bước 2 nhưng lặp cho cả 8 bit DQ (i=0..7) trong mỗi pup**, không chỉ DM — output là mảng `dqDly[ch][cs][pup][i]` (left-edge riêng từng bit DQ), chính là mảng được `lpddr4_read_deskew_center_align()` đọc lại ở Bước 5 (`dqDly[ch][cs][pup][i] = nova_ddrphy_read(dqRegAddr[ch][i+pup*8]+0x10*cs, 0, pup+4*ch)` — xem Bước 5). Việc suy luận này dựa trên bằng chứng gián tiếp (dữ liệu mà Bước 5 đọc lại), không phải đọc trực tiếp thân hàm — cần đọc đủ 294 dòng ở 1 session sau để xác nhận 100%, đặc biệt để biết chính xác điều kiện dừng và có xử lý riêng multi-window hay không (giống câu hỏi đã đặt ra ở tài liệu 07).

## Bước 4 — `lpddr4_read_find_deskew_end()` — TRN_H3

[SOURCE FACT — header only, `:3407-3413`, comment gốc: "Determines first point at which all DQs and DM **fail**"]. Đối xứng với Bước 1 nhưng tìm **RIGHT edge** (điểm bắt đầu FAIL) thay vì left edge. ❓[UNKNOWN — chưa đọc thân hàm ~146 dòng] chi tiết vòng quét, nhưng output gần như chắc chắn là mảng `DQSEnd[ch][cs][pup]` — biến chính xác được Bước 5 dùng (`nova_ddrphy_write(0x3+cs*0x4, 0, pup+4*ch, DQSEnd[ch][cs][pup])`, dòng 2833) — bằng chứng gián tiếp mạnh cho tên biến/vai trò, nhưng lại là suy luận từ cách dùng ở nơi khác, không phải đọc trực tiếp.

## Bước 5 — `lpddr4_read_deskew_center_align()` — TRN_H4

[SOURCE FACT — `:2774-2923+`, comment gốc: "Align each DQ and DM to center of data window", đã đọc phần chính]

```c
dqDly[ch][cs][pup][i] = nova_ddrphy_read(dqRegAddr[ch][i+pup*8]+0x10*cs, 0, pup+4*ch);  // đọc lại LEFT EDGE (từ Bước 3)
nova_ddrphy_write(0x3+cs*0x4, 0, pup+4*ch, DQSEnd[ch][cs][pup]);                          // set DQS = RIGHT EDGE (từ Bước 4)
...
for ( dly = 0; dly < MAX_DESKEW+1; dly++ )
{
    nova_ddrphy_write(dqRegAddr[...], 0, pup+4*ch, dqDly[ch][cs][pup][i] + dly);   // tăng dần TỪ left edge
    regval = read_dq_cal_test(...);
    if ( ((regval bit i) == 0) || (dqDly[...][i]+dly == MAX_DESKEW) )
        finalDqDly[ch][cs][pup][i] = dly/2;    // ← dly khi VỪA FAIL, chia 2 = CENTER
}
```
**Đây chính là công thức center quen thuộc (tài liệu 03), viết dưới dạng khác**: bắt đầu từ `left edge` (đã biết từ Bước 3), tăng dần `dly` cho tới khi FAIL — `dly` lúc đó chính là **độ rộng Window** (`right - left`), và `finalDqDly = dly/2` chính là `left + (right-left)/2` (vì left đã được cộng sẵn vào từng lần set trong vòng lặp, `dly` đo từ left edge, không phải từ 0 tuyệt đối).

# Register view

| Register/Field | Ý nghĩa |
|---|---|
| `CSS_ADDR+0x0100` field `0xff0` | "DM to DQ swap" — tạm định tuyến đường DM vào 1 slot deskew của DQ để dùng chung mạch calibrate |
| PHY sub-reg `0x53 + cs*0x10` (qua `nova_ddrphy_write`) | Delay riêng cho DM (dùng ở Bước 2 và đọc lại ở Bước 5) |
| `dqRegAddr[ch][i+pup*8]` (bảng địa chỉ, qua `g_ddr_ap806_platform_config`) | Delay riêng cho từng bit DQ `i` (0-7) trong pup |
| `DQSStart[ch][cs][pup]` | Left-edge thô mức pup (output Bước 1, input các bước sau) |
| `DQSEnd[ch][cs][pup]` | Right-edge mức pup (output Bước 4 — 🧠suy luận, input Bước 5) |
| `pbsMinDly[ch][cs][pup]` | Left-edge riêng của DM (output Bước 2) |
| `dqDly[ch][cs][pup][i]` | Left-edge riêng từng bit DQ (output 🧠suy luận của Bước 3, input Bước 5) |
| `finalDqDly[ch][cs][pup][i]` | **Giá trị cuối cùng** — center của từng bit DQ (output Bước 5) |

# Source view

- `mv_lpddr4_apn806.c:3565-3673` — `lpddr4_read_find_deskew_start()` — đọc toàn bộ
- `mv_lpddr4_apn806.c:2637-2769` — `lpddr4_read_dm_deskew()` — đọc toàn bộ
- `mv_lpddr4_apn806.c:3107-3407` — `lpddr4_read_deskew()` — **chỉ đọc header, chưa đọc thân hàm đầy đủ**
- `mv_lpddr4_apn806.c:3407-3559` — `lpddr4_read_find_deskew_end()` — **chỉ đọc header, chưa đọc thân hàm đầy đủ**
- `mv_lpddr4_apn806.c:2774-2923+` — `lpddr4_read_deskew_center_align()` — đọc phần chính (chưa đọc hết đoạn cuối hàm)
- `mv_lpddr4_apn806.c:8365-8392` — 5 call site thật, mốc `TRN_H0`→`TRN_H4`

# Call flow

```
(sau Read Centering lần 1 — TRN_GX, tài liệu 08)
        |
        v
lpddr4_read_find_deskew_start()   → TRN_H0   [tìm LEFT EDGE thô, mức PUP, dùng vị trí bit DM để dò]
        |
        v
lpddr4_read_dm_deskew()            → TRN_H1   [tìm LEFT EDGE riêng của DM]
        |
        v
lpddr4_read_deskew()                → TRN_H2   [🧠 tìm LEFT EDGE riêng từng bit DQ 0-7 — chưa đọc thân hàm]
        |
        v
lpddr4_read_find_deskew_end()       → TRN_H3   [🧠 tìm RIGHT EDGE (điểm FAIL) — chưa đọc thân hàm]
        |
        v
lpddr4_read_deskew_center_align()   → TRN_H4   [center = left + (right-left)/2, ÁP DỤNG RIÊNG từng bit DQ + DM]
        |
        v
(Read Centering lần 2 — TRN_H5, tài liệu 08 — center lại TOÀN PUP sau khi từng bit đã deskew)
```

# Diagram

```
       DQ0 ------->|PASS PASS PASS|-----   left0=5   right0=15  center0=10
       DQ1 -------------->|PASS PASS PASS|-  left1=9   right1=17  center1=13
       DQ2 --->|PASS PASS PASS PASS|------   left2=3   right2=13  center2=8
       DM  ---------->|PASS PASS PASS|----   leftDM=7  rightDM=15 centerDM=11
                       ^
              mỗi đường có delay RIÊNG được ghi vào register RIÊNG (dqRegAddr[ch][i+pup*8])
              để tất cả "nhìn" đúng cùng 1 thời điểm DQS, dù đường dây vật lý khác nhau
```

# K-S800 customization

Không thấy `#ifdef KM_*` nào bọc riêng 5 hàm này (khác hẳn tài liệu 05/06/08) — 🧠[INFERENCE] các hàm Read Deskew có khả năng **giữ nguyên gần với code Marvell gốc**, không bị KM tùy biến nhiều — hợp lý vì deskew per-bit là thuật toán "cơ khí" ít phụ thuộc đặc thù sản phẩm S800 (khác Vref/timing vốn phụ thuộc chip Micron/Samsung cụ thể). ❓UNKNOWN — chưa xác nhận trực tiếp bằng cách so sánh với `mv_ddr_apn806.c`.

# Debugging

- Log `"All DM are failed. find the deskew values!!"` (Bước 2) → tất cả DM đã tìm được left-edge, dừng sớm — bình thường, không phải lỗi.
- Log `"All DQs are passed at right side of window. get deskew value"` (Bước 5) → tất cả 16 bit (8 DQ × 2 device) trong mask đã tìm được center — bình thường.
- Nếu **không** thấy các log trên mà vòng lặp chạy hết tới `MAX_DESKEW` — nghi ngờ 1 bit DQ/DM cụ thể không bao giờ PASS (đứt mạch, hoặc Window quá hẹp/không tồn tại do lỗi ở CA training/Write Leveling/DQS Gate trước đó) — kiểm tra `finalDqDly[ch][cs][pup][i]` bit nào vẫn giữ giá trị sentinel `-1`.

# Summary

- Read Deskew chỉnh delay **riêng từng bit DQ + DM**, không chỉ 1 delay chung cho cả pup như tài liệu 07-08.
- DM cần thủ thuật "swap" tạm thời vào 1 slot deskew của DQ vì không có mạch riêng.
- 5 hàm thật theo đúng tên user đã liệt — nhưng thứ tự thật là start→dm_deskew→deskew→find_end→center_align (không phải center_align chạy ngay sau start).
- Công thức center vẫn là `left + (right-left)/2`, chỉ viết dưới dạng "quét dần từ left, chia 2 khi FAIL" ở bước cuối.
- 2 trong 5 hàm (`read_deskew`, `read_find_deskew_end`) **chưa được đọc chi tiết thân hàm** trong session này — chỉ xác nhận qua header comment + bằng chứng gián tiếp từ nơi dùng biến output của chúng. Đánh dấu rõ để tránh coi suy luận là fact.

# Verify yourself

1. Vì sao DM không có mạch deskew riêng mà phải "mượn" mạch của DQ? (Gợi ý: liên hệ số lượng tín hiệu — DM có bao nhiêu bit trên 1 pup, so với DQ?)
2. Nếu Bước 1 (find_deskew_start) không chạy trước, mà Bước 3 (read_deskew) phải tự quét từ dly=0, hậu quả về thời gian sẽ thế nào?
3. Tại sao ở Bước 5, code tăng `dly` TỪ left edge (không quét lại từ 0), rồi chia 2 khi FAIL — thay vì lưu cả left và right riêng rồi trừ như tài liệu 03 làm với DQS Gate?
4. Theo đúng nguyên tắc source-grounding, phần nào trong tài liệu này là fact, phần nào là suy luận cần xác minh thêm? Liệt kê 2 chỗ.
5. Nếu bạn phát hiện 1 bit DQ cụ thể luôn FAIL toàn dải delay, bước nào trong 5 bước trên là nơi đầu tiên bạn sẽ thấy dấu hiệu này qua log?

**Tiếp theo:** `10_WRITE_CENTERING.md` — sẽ nhanh hơn vì đã có `calculate_com()` (tài liệu 08) và Vref/center (tài liệu 03) làm nền — Write Centering đi hướng ngược (Controller → DRAM).
