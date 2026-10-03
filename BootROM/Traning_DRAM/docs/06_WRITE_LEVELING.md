# 06 — Write Leveling: lpddr4_write_leveling() thật

Đọc `00`-`05` trước. ⚠️ Chương này sẽ **điều chỉnh lại** một phần khẳng định ở tài liệu 03 — không phải mọi thuật toán training đều dùng công thức "center". Write Leveling là ví dụ ngược lại.

# Concept

Khi Controller **ghi** dữ liệu (Write), chính Controller (không phải DRAM) là bên tạo ra tín hiệu DQS để báo "dữ liệu trên DQ đang hợp lệ". Nhưng Controller tạo DQS dựa trên đồng hồ nội bộ của nó — **không tự động biết** đường dây tới từng con chip DRAM dài bao nhiêu, nên DQS có thể tới DRAM sớm/muộn hơn CK một khoảng skew (xem tài liệu 01, mục 9). Write Leveling = quá trình chỉnh DQS (bên Controller) sao cho **cạnh lên của DQS đúng khớp với cạnh lên của CK tại chính chân DRAM** — không phải khớp tại chân Controller.

# Why

📘[GENERAL KNOWLEDGE — JEDEC Write Leveling] JEDEC giải quyết bài toán "Controller không biết delay đường dây" bằng cách để **DRAM tự báo cáo ngược lại**: đưa DRAM vào 1 mode đặc biệt, trong mode đó cứ mỗi lần DQS toggle, DRAM sẽ lấy mẫu mức của CK ngay lúc đó và phản hồi lại giá trị đó (0 hoặc 1) qua DQ. Controller dò từng giá trị delay của DQS, đọc phản hồi, và tìm **đúng thời điểm chuyển từ 0 sang 1** — đó chính là lúc DQS trùng cạnh CK.

**Đây là lý do khiến bài này khác tài liệu 03**: Write Leveling không đi tìm 1 "vùng PASS rộng" để lấy trung điểm — nó đi tìm **1 điểm chuyển pha (edge) duy nhất**. Mục tiêu vật lý khác nhau (align chính xác 1 cạnh, không phải tối đa hoá margin của 1 cửa sổ dữ liệu) → thuật toán khác nhau.

# Real-world analogy

Giống việc chỉnh đồng hồ đeo tay để nó "tích" đúng cùng lúc với đồng hồ chuẩn treo tường — bạn không cần một "khoảng thời gian rộng" để đồng hồ đúng, bạn cần đúng **1 khoảnh khắc** kim giây trùng nhau.

# Hardware view + Firmware view (trace `lpddr4_write_leveling()`)

[SOURCE FACT — `mv_lpddr4_apn806.c:7366`, comment gốc: "Training routine for write leveling"]

## Bước 1 — Enable Write Leveling mode (`wl_enable()`, dòng 7409 — hàm chưa đọc chi tiết, để dành nếu cần)

## Bước 2 — Đưa DQS về vị trí "0" ban đầu + đọc phản hồi khởi điểm (dòng 7411-7435)

```c
for (k=0; k<4; k++)
    nova_ddrphy_write(0, 0, dqsPupAddr[ch][k], 0x4000 | (phase[ch] << 6));   // set DQS delay = 0 (điểm bắt đầu quét)

rdModWr(&phy_reg->training_wl, TRN_WL_CS_UPD_MASK | (cs<<TRN_WL_CS_SHIFT), ..., SKIP_VERIFY);  // chọn CS đang train
while ( (read(&phy_reg->training_wl) & TRN_WL_CS_UPD_MASK) && count<100 ) count++;
if (count>=100) { "Failed waiting for Write Leveling update mask"; train_err_flg=1; }   // KM_RETRY

regval = read(&phy_reg->training_wl);
result = (regval & 0x00f00000) >> 20;   // 4 bit phản hồi, 1 bit/pup (byte-lane)
```

## Bước 3 — Quét DQS shift `j` từ 1 đến 31, tìm cạnh chuyển 0→1 (dòng 7461-7498)

```c
for ( j=1; j<32; j++ )
{
    for (k=0; k<4; k++)
        nova_ddrphy_write(0, 0, dqsPupAddr[ch][k], 0x4000 | j | (phase[ch]<<6));   // dịch DQS thêm j

    resetPHY(PHYRSTMSK);
    rdModWr(&phy_reg->training_wl, 0x1<<TRN_WL_DQS_SHIFT, TRN_WL_DQS_MASK, SKIP_VERIFY);  // toggle DQS 1 lần
    regval = read(&phy_reg->training_wl);
    result = (regval & 0x00f00000) >> 20;                     // đọc phản hồi mới

    for (k=0; k<4; k++)
    {
        if ( dqsShift[k]==0 )                    // chưa tìm được cạnh cho pup k
        {
            if ( zero[k]==0 && !(result & (1<<k)) )
                zero[k] = 1;                       // đã thấy 1 lần phản hồi = 0 (đang ở "trước" cạnh CK)
            if ( zero[k]==1 && (result & (1<<k)) )
                dqsShift[k] = j | 0x4000 | (phase[ch]<<6);   // ← LẦN ĐẦU thấy phản hồi lật 0→1 = TÌM ĐƯỢC CẠNH
        }
    }
    if ( result == 0xf ) break;   // cả 4 pup đều đã lật sang 1 → xong sớm
}
if (result != 0xf) { "WL on CH %d CS %d failed. Mask: 0x%x"; train_err_flg=1; }
```

**Đây chính là "edge-finding", không phải "window+center"**: `dqsShift[k]` được gán **đúng 1 lần duy nhất**, ngay khi phản hồi lật từ 0 sang 1 lần đầu tiên — không có khái niệm "quét tiếp để tìm điểm PASS cuối cùng rồi lấy trung điểm" như DQS Gate/CA training/Read Centering ở các tài liệu 03/05.

## ⚠️ Phát hiện quan trọng — nửa "CLK shift" của thuật toán đang bị TẮT (comment out)

[SOURCE FACT — `mv_lpddr4_apn806.c:7436-7459`]: có 1 khối code hoàn chỉnh, nằm nguyên trong `/* ... */` (comment block C, không phải `#if 0`), mô tả 1 vòng lặp tìm **CLK shift** (biến `i`, từ `clockDly[ch][cs]+1` tới 32) — đây đúng là **nửa đầu tiên** của Write Leveling theo lý thuyết JEDEC đầy đủ (JEDEC cho phép chỉnh CẢ CK-delay và DQS-delay khi leveling, không chỉ DQS). Toàn bộ khối này **không được compiler biên dịch vào** (nó là comment, không phải code có điều kiện `#ifdef`).

Hệ quả trực tiếp, đọc được từ chính source: biến `uint8_t i = 0;` (khai báo dòng 7368) **không bao giờ bị thay đổi** trong suốt phần code đang thực thi — chỉ khối bị comment mới gán lại `i`. Do đó dòng 7507 `wlClockDly[ch][cs] = i;` **luôn ghi giá trị 0** (hoặc giá trị rác nếu compiler tối ưu khác — nhưng theo khai báo, là 0), bất kể CS/channel nào, bất kể lần train nào.

🧠[INFERENCE — hệ quả kỹ thuật, không phải fact trực tiếp]: điều này gợi ý rằng **trong phiên bản source hiện tại, S800 chỉ thực sự chạy nửa "DQS-shift" của Write Leveling, không chạy nửa "CLK-shift"** — có thể vì (a) CA training (tài liệu 05) đã tự chỉnh CLK delay đủ tốt nên không cần bước CLK-shift riêng ở đây nữa, hoặc (b) đây là 1 tính năng bị tắt có chủ đích để đơn giản hoá/tiết kiệm thời gian, hoặc (c) code cũ bị bỏ quên khi refactor. ❓[UNKNOWN]: không có bằng chứng nào trong các file đã đọc xác nhận đây là chủ đích hay sơ suất — **đây là ứng viên mạnh cho việc đối chiếu với spec chương 4.4 (Write Leveling flowchart)** ở 1 session sau, để xem spec có mô tả rõ "chỉ chỉnh DQS, giữ CLK cố định" hay không. Nếu spec mô tả đủ cả 2 bước mà source chỉ chạy 1 → đây sẽ là ⚠️[SPEC/SOURCE MISMATCH] thật.

## Bước 4 — Reset lại CLK/DQS delay tạm về giá trị "mặc định cho lần train tiếp" (dòng 7514-7533)

```c
for (k=8; k<12; k++)
{
    // nova_ddrphy_write(regAddr[ch][k], 1, pupAddr[ch][k], clockDly[ch][cs]);   ← dòng CŨ, bị comment
    /* 2025/03/07 帆足 - OP_BTS-xxxx ch1のディレイ設定ミスの修正 */
    nova_ddrphy_write(0, 1, pupAddr[ch][k], 0);        // ← dòng MỚI (bug-fix): ghi HẰNG SỐ 0
}
for (k=0; k<4; k++)
    nova_ddrphy_write(0, 0, dqsPupAddr[ch][k], 0x4000 | (phase[ch]<<6));   // reset DQS delay về 0

wl_disable(mc_reg, phy_reg, cs, ch);
resetPHY(PHYRSTMSK);
```

⚠️ Ghi chú quan trọng: comment ngay phía trên dòng này (dòng 7514, `//set clk delays back to CA training value for now`) nói ý định là **khôi phục CLK delay về giá trị CA training đã tìm được** (`clockDly[ch][cs]`) — nhưng dòng code THẬT SỰ (sau bug-fix `帆足`/`2025-03-07`/`OP_BTS-xxxx`) lại ghi **hằng số `0`**, không dùng biến `clockDly[ch][cs]` nữa, và cũng đổi cả `reg_num` từ `regAddr[ch][k]` thành hằng số `0`. 🧠[INFERENCE]: comment cũ **không còn khớp với code mới sau fix** — đây là 1 ví dụ thực tế, cụ thể, đúng như cảnh báo ở nguyên tắc source-grounding: *"comments can lie"*. ❓[UNKNOWN]: chưa xác định được liệu ghi hằng số 0 ở đây có đúng về mặt chức năng hay không (có thể đúng, vì lý do khác `clockDly` không còn cần thiết ở điểm này) — cần bug report đầy đủ (không chỉ dòng comment 1 dòng) để xác nhận, việc này để dành tài liệu 22.

# Register view

| Register/Field | Ý nghĩa | Vai trò |
|---|---|---|
| `training_wl` bit `TRN_WL_CS_UPD` / field `TRN_WL_CS` | Chọn CS nào đang được Write-Level, và báo "đang cập nhật" | Firmware ghi, hardware tự xoá `_UPD` khi xong (poll tại dòng 7422) |
| `training_wl` bit `TRN_WL_DQS` | Toggle 1 lần để yêu cầu PHY gửi 1 xung DQS thử + đọc phản hồi | Firmware ghi để trigger, đọc lại cùng register để lấy kết quả |
| `training_wl` bits [23:20] | 4-bit phản hồi từ DRAM (1 bit/pup) — giá trị CK mà DRAM lấy mẫu được lúc DQS toggle | Hardware set, firmware đọc |
| PHY sub-reg `dqsPupAddr[ch][k]` (qua `nova_ddrphy_write(0,0,...)`) | Delay áp cho từng DQS pup | Firmware ghi mỗi vòng quét |
| PHY sub-reg `pupAddr[ch][k]` cho k=8..11 (qua `nova_ddrphy_write(0,1,...)`) | Delay áp cho CLK (nhóm `k=8..11`, khác nhóm `k` dùng cho CA ở tài liệu 05) | Firmware ghi — hiện đang ghi hằng số 0 (xem Bước 4) |

# Source view

- `mv_lpddr4_apn806.c:7366-7535` — `lpddr4_write_leveling()` toàn bộ, đã đọc hết
- `mv_lpddr4_apn806.c:7436-7459` — khối CLK-shift bị comment-out hoàn toàn
- `mv_lpddr4_apn806.c:7518-7519` — bug-fix `帆足`/`2025-03-07`/`OP_BTS-xxxx` (fix thứ 3 cùng ngày, xem tài liệu 05 cho 2 fix trước)
- `mv_lpddr4_apn806.c:8286-8299` — call site: chạy trong vòng lặp `for ch, for cs` — **train riêng từng channel/CS**, sau đó `applyWlDlys()` (chưa đọc — sẽ trace nếu cần ở tài liệu 19), mốc `TRN_D0`/`TRN_D1`

# Call flow

```
(caller — sau lpddr4_ca_training(), mốc TRN_C5)
        |
        v
for each (ch, cs):
    lpddr4_write_leveling(mc_reg, phy_reg, ch, cs)
        |
        +-- wl_enable()
        +-- set DQS delay = 0 (điểm khởi đầu)
        +-- chọn CS, poll TRN_WL_CS_UPD
        +-- đọc phản hồi khởi điểm (result, 4 bit)
        +-- [KHỐI CLK-SHIFT — COMMENT OUT, KHÔNG CHẠY]
        +-- for j = 1..31:
        |       set DQS delay = j
        |       toggle TRN_WL_DQS, đọc phản hồi
        |       nếu phản hồi lật 0→1 lần đầu cho pup k: dqsShift[k] = j   (EDGE, không phải center)
        |       nếu cả 4 pup xong: break
        +-- reset CLK delay = 0 (bug-fix 2025/03/07), reset DQS delay = 0
        +-- wl_disable()
        v
display_measure_time(TRN_D0) → applyWlDlys(mc_reg) → display_measure_time(TRN_D1)
```

# Diagram

```
Phản hồi DRAM (result bit k), theo DQS shift j:
   j=1  j=2  j=3  j=4  j=5  j=6 ...
    0    0    0    1    1    1        ← lật tại j=4
              ^
         dqsShift[k] = 4   (KHÔNG lấy trung điểm — chỉ lấy đúng điểm lật)
```

# K-S800 customization

- Bug-fix thứ 3 trong cùng ngày `2025/03/07`, cùng người `帆足`, cùng ticket `OP_BTS-xxxx` (2 fix trước ở tài liệu 05) — nội dung: sửa lỗi thiết lập delay cho **CH1** (`ch1のディレイ設定ミスの修正`). 🧠[INFERENCE]: việc cả 3 fix cùng ngày, cùng người, rải trên CA training và Write Leveling, đều liên quan tới "delay không được set đúng" — gợi ý mạnh đây là 1 đợt sửa lỗi có hệ thống cho toàn bộ đường dẫn CLK/CA/CS/DQS delay, không phải 3 bug rời rạc. Vẫn cần đọc bug report gốc (nếu có trong Appendix B hoặc changelog khác) để xác nhận — không kết luận thêm ở đây.
- Khối CLK-shift bị tắt (comment out) — chưa rõ là chủ đích KM hay tồn tại từ code Marvell gốc; cần so sánh với `atf/atf_s800/drivers/marvell/mv_ddr/apn806/mv_ddr_apn806.c` để phân biệt.

# Debugging

- Log `"Failed waiting for Write Leveling update mask"` → hardware không xác nhận chọn CS thành công trong 100 lần poll — nghi ngờ tầng thấp hơn (PHY reset, pad calibration) chưa ổn định.
- Log `"WL on CH %d CS %d failed. Mask: 0x%x"` với `result != 0xf` → quét hết `j=1..31` mà không phải cả 4 pup đều tìm được cạnh lật — đọc kỹ giá trị mask in ra: bit nào **không** lên 1 chính là pup nào đang có vấn đề (ví dụ mask `0x7` = pup 3 chưa lật, 3 pup khác đã lật được).
- Cả 2 lỗi trên đều set `train_err_flg = 1` (dưới `#ifdef KM_RETRY`) — liên kết trực tiếp tới cơ chế retry toàn cục sẽ học ở tài liệu 17.
- ⚠️ Nếu nghi ngờ vấn đề liên quan tới CLK delay (không phải DQS) mà không thấy cải thiện dù đã thử chỉnh nhiều tham số khác — nhớ lại rằng **CLK-shift search đã bị tắt trong hàm này**; delay CLK tại điểm vào Write Leveling là delay còn sót lại từ CA training (tài liệu 05, `setDelay_ClkCsCa()`), không phải giá trị được tối ưu riêng cho Write Leveling.

# Summary

- Write Leveling canh cạnh CK-DQS bằng phản hồi ngược từ DRAM (JEDEC), khác Read/Write Centering (canh giữa 1 vùng dữ liệu).
- Thuật toán tìm **1 điểm lật (edge)**, không tìm window rồi lấy trung điểm — khác với hầu hết thuật toán ở tài liệu 03/05.
- **Phát hiện thật quan trọng**: nửa thuật toán "CLK-shift" bị comment-out hoàn toàn trong source hiện tại — chỉ "DQS-shift" thực sự chạy. Cần đối chiếu spec chương 4.4 ở session sau.
- 1 bug-fix thật (`帆足`, `2025/03/07`, cùng ticket với 2 fix ở tài liệu 05) thay đổi code khôi phục CLK delay thành hằng số 0 — không khớp với comment mô tả ý định gốc ("về giá trị CA training") — ví dụ cụ thể "comment có thể sai/lỗi thời".
- Lỗi Write Leveling set cờ `train_err_flg` — liên kết trực tiếp tới cơ chế retry (tài liệu 17).

# Verify yourself

1. Vì sao Write Leveling cần DRAM "phản hồi ngược" lại giá trị CK nó lấy mẫu được, thay vì Controller tự đo trực tiếp?
2. Vì sao thuật toán này tìm 1 điểm lật (edge) mà không tìm window rồi lấy trung điểm như các thuật toán khác? Liên hệ lại mục Why.
3. Nếu khối CLK-shift bị tắt thật sự làm mất đi 1 phần chính xác của Write Leveling, hậu quả có thể xuất hiện rõ nhất ở bước training nào sau đó (DQS Gate? Read Centering?) — và tại sao?
4. Đọc lại đoạn Bước 4: comment nói "về giá trị CA training" nhưng code ghi hằng số 0 — theo nguyên tắc source-grounding, bạn tin cái nào hơn khi 2 thứ mâu thuẫn: comment hay code thực thi? Vì sao?
5. `result == 0xf` nghĩa là gì về mặt bit, và tại sao đó là điều kiện dừng sớm hợp lý?

**Tiếp theo:** `07_DQS_GATE.md` — đã thấy 1 phần thuật toán này ở tài liệu 03 (ví dụ 1); tài liệu 07 sẽ trace đầy đủ hàm thật, bao gồm Preamble/Toggle và bug "multiple valid windows" mà spec Appendix B.3 nhắc tới.
