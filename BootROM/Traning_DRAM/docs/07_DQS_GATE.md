# 07 — DQS Gate Training: 2 hàm thật, chọn bằng 1 cờ QSPI

Đọc `00`-`06` trước. ⚠️ Tài liệu 03 đã trích 1 đoạn code (`:5794-5829`) làm ví dụ "DQS Gate training" — session này xác nhận chính xác đoạn đó nằm **trong hàm `lpddr4_dqs_gate_training_preamble()`**, không phải hàm `lpddr4_dqs_gate_training()` (2 hàm khác nhau, xem dưới). Không sai về nội dung, chỉ bổ sung độ chính xác.

# Concept

Khi CPU **đọc** dữ liệu từ DRAM (Read), tín hiệu DQS đi kèm DQ không "luôn tồn tại" — DRAM chỉ tạo DQS trong 1 khoảng ngắn ngay khi trả dữ liệu về. Trước khoảng đó, đường dây DQS có thể ở trạng thái lửng/nhiễu (không ai chủ động lái). Nếu PHY "nghe" DQS liên tục ngay từ đầu, nó có thể nhầm nhiễu này thành 1 cạnh DQS thật → đọc sai dữ liệu ngay từ bit đầu tiên.

**DQS Gate** = 1 cái "cổng" trong PHY, mặc định đóng (không nghe DQS), chỉ **mở đúng lúc** DRAM sắp gửi DQS thật. **Preamble** 📘[GENERAL KNOWLEDGE — JEDEC] là 1 khoảng tín hiệu ngắn, có dạng biết trước (DRAM giữ DQS ở 1 mức cố định trong 1 khoảng thời gian quy định trước khi bắt đầu toggle thật) — giống như "hắng giọng" trước khi nói câu thật, để người nghe biết "sắp có gì đó". DQS Gate Training = tìm đúng thời điểm mở cổng ngay sau preamble, trước khi dữ liệu thật (Toggle) bắt đầu.

# Why

Nếu mở cổng quá sớm → bắt nhiễu trước preamble, PHY hiểu sai là dữ liệu đã bắt đầu. Nếu mở quá muộn → bỏ lỡ vài bit đầu tiên của dữ liệu thật. Cả 2 đều gây lỗi đọc. Vì preamble có độ dài quy định theo JEDEC nhưng **thời điểm preamble bắt đầu tới PHY còn phụ thuộc độ dài đường dây** (giống mọi thứ khác trong LPDDR4), firmware phải tự dò tìm bằng cách quét delay, giống các thuật toán trước.

# Hardware view + Firmware view — 2 hàm thật, chọn bằng cờ QSPI

[SOURCE FACT — `mv_lpddr4_apn806.c:8301-8315`]:
```c
if ( dqs_gate_preamble_en == 1 )
    lpddr4_dqs_gate_training_preamble( mc_reg, warmboot, train_reg );   // phương pháp PREAMBLE
else
    lpddr4_dqs_gate_training( mc_reg, warmboot, train_reg );            // phương pháp DATA TEST
```
[SOURCE FACT — `:7876, 7908`]: `dqs_gate_preamble_en` mặc định = **1** (dùng Preamble), nhưng có thể bị QSPI override qua `chip_info_reg->DQS_GATE_PREAMBLE_EN`. Đây **chính là "Toggle"** mà đề bài nhắc tới ở khía cạnh khác: phương pháp thứ 2 (`else`) không dựa vào preamble, mà **ghi 1 pattern dữ liệu thật vào DRAM rồi đọc lại** (qua `data_ptr[ch][cs]` trỏ vào `m_capacity_info.train_base_*` — vùng DRAM thật đã reserve để test) để kiểm tra PASS/FAIL bằng dữ liệu thật, thay vì bằng trạng thái preamble.

## Phương pháp Preamble (`lpddr4_dqs_gate_training_preamble()`, dòng 5684-6000, mặc định dùng)

**Bước 1 — Bật read-leveling mode trên DRAM + set delay 2 tầng** (dòng 5759-5786):
```c
send_mrx(1,13,ch,cs);   // "turn on read-leveling mode" — bật preamble mode trên chính DRAM qua MR13
cycle_dly = (8 * (phy_dly >> 8)) + cycle_dly_offset;    // delay THÔ: theo đơn vị 1 chu kỳ clock DRAM
adj_phy_dly = (phy_dly & 0x1f) | ((phy_dly & 0xe0) << 1);
nova_ddrphy_write(0x2 + 4*cs, 0, pup+4*ch, adj_phy_dly);   // delay MỊN: theo đơn vị PHY tap, trong 1 chu kỳ
```
`phy_dly` quét từ 0 đến 511 (9 bit) — **2 tầng delay lồng nhau**: 3 bit cao (`>>8`) chọn "cycle delay" thô (số chu kỳ clock DRAM nguyên vẹn phải chờ), phần còn lại là delay mịn trong 1 chu kỳ. 🧠[INFERENCE]: đây là kiến trúc bắt buộc vì delay cần tìm có thể lớn hơn 1 chu kỳ clock đầy đủ (đường dây dài + tần số cao → delay tuyệt đối có thể vượt qua ranh giới 1 chu kỳ), nên phải chia 2 tầng: "bao nhiêu chu kỳ nguyên" + "lệch bao nhiêu trong chu kỳ đó" — không thể biểu diễn bằng 1 số delay tuyến tính đơn.

**Bước 2 — PASS test bằng trạng thái PHY (không đọc dữ liệu thật)** (dòng 5806, đã trích ở tài liệu 03):
```c
if ( nova_ddrphy_read(0x9A,0,pup+4*ch)==0x100 && nova_ddrphy_read(0x9B,0,pup+4*ch)==0x3F7F )
    // PASS: PHY tự báo đã nhận diện đúng preamble
```

**Bước 3 — Đóng Window bằng 1 trong 2 điều kiện khác nhau** ⚠️ (chi tiết hay bị đơn giản hoá quá mức ở tài liệu 03):

- **Điều kiện A — vẫn đang PASS liên tục, window đã quá lớn** (dòng 5824, đã trích tài liệu 03): `if (window[ch][cs][pup] > 63)` → coi như đủ rộng, không cần quét tiếp, chốt ngay `center = first + (last-first)/2`.
- **Điều kiện B — VỪA FAIL, nhưng Window vừa đóng đủ lớn** (dòng 5843, **chưa trích ở tài liệu 03**): `if ( passing[ch][cs][pup]==1 && window[ch][cs][pup] > 40 )` → cũng chốt `center`, dùng ngưỡng **thấp hơn** (40, không phải 63) vì đây là trường hợp Window đã thực sự đóng lại (biết chắc right edge), không phải "đang mở, đoán sẽ còn tiếp".
- Nếu FAIL mà Window ≤ 40 → coi là nhiễu/quá hẹp, **reset `passing=0`, tiếp tục quét** tìm Window khác — đây chính là cơ chế cho phép **nhiều Window rời rạc** xuất hiện trong 1 lần quét (liên hệ Appendix B.3 dưới).

**Bước 4 — Gộp kết quả CS0/CS1, chọn cycle delay chung cho cả channel** (dòng 5901-5933):
```c
PHYDelay[ch][0][pup] = (PHYDelay[ch][0][pup] + PHYDelay[ch][1][pup]) / 2;   // trung bình 2 CS — KHÔNG phải center window
phase[ch][pup] = (8*(PHYDelay[ch][0][pup]>>8)) + ((PHYDelay[ch][0][pup]&0xe0)>>5);
if (phase[ch][pup] < min) min = phase[ch][pup];     // tìm phase NHỎ NHẤT trong 4 pup
...
chosenCycleDelay[ch] = min;                          // 1 cycle delay DUY NHẤT cho cả channel
chosenPhaseDelay[ch][pup] = phase[ch][pup] - chosenCycleDelay[ch];   // phần lệch riêng mỗi pup, so với cái chung
```
🧠[INFERENCE]: hardware chỉ có **1 thanh ghi cycle-delay chung cho cả channel** (`delay_reg[ch][cs]`, kiểu `PHY_RL_Control_CSx_Bx`), nhưng 4 pup (byte-lane) có thể cần cycle delay khác nhau đôi chút — firmware giải quyết bằng cách **chọn giá trị NHỎ NHẤT làm baseline chung**, rồi bù phần dư còn lại (`chosenPhaseDelay`) riêng cho từng pup ở tầng mịn hơn (`phase`/`phy_dly`, vẫn chỉnh được độc lập per-pup qua `nova_ddrphy_write`).

**Xử lý lỗi khi PHYDelay = 0** (dòng 5906-5915): giá trị `0` gần như chắc chắn là "chưa từng tìm được PASS" (sentinel), không phải 1 kết quả hợp lệ — nếu 1 CS ra 0, code **tự động lấy giá trị từ CS còn lại** và in lỗi `"!!! DQS Gate Training Error PHYDelay[ch][cs][pup] = 0"`, không dừng hẳn.

## Phương pháp Data Test (`lpddr4_dqs_gate_training()`, dòng 6011-...): tương tự cấu trúc, khác PASS-test

Cấu trúc biến/vòng lặp **giống gần như 100%** hàm Preamble (cùng `cycle_dly`/`phy_dly`, cùng ngưỡng window 63/40 — 🧠[INFERENCE] chưa đọc dòng PASS-test cụ thể của hàm này để xác nhận công thức PASS chính xác, nhưng cấu trúc tổng thể xác nhận qua khai báo biến giống hệt). Khác biệt cốt lõi: dùng `data_ptr[ch][cs]` (con trỏ NONCACHE trỏ vào vùng DRAM thật `train_base_0/2/4/6`) để **ghi/đọc dữ liệu thật** thay vì đọc trạng thái nhận-diện-preamble từ PHY. ❓[UNKNOWN]: chưa đọc chi tiết điều kiện PASS/FAIL trong hàm này (dữ liệu đọc về so khớp pattern nào) — để dành nếu cần trace sâu hơn ở tài liệu 19.

# Register view

| Register/bit | Ý nghĩa |
|---|---|
| MR13 (qua `send_mrx(1,13,ch,cs)` / `send_mrx(1,13,ALL,ALL)`) | Bật/tắt "read-leveling mode" trên chính DRAM — tương tự cơ chế MR13 dùng ở Write Leveling (tài liệu 06), nhưng ý nghĩa bit khác |
| `CH0/1_DRAM_Config_3` bit `RD_PRE_TRAINING` | Bật chế độ Controller sẵn sàng nhận phản hồi kiểu "read preamble training" |
| `CH0/1_ODT_Control_2` field `PAD_TERM_SWITCH_MODE` | Tạm chỉnh chế độ ODT trong lúc quét (đặt 0 khi quét, đặt lại `0x02000000` sau khi xong 1 vòng CS) |
| `delay_reg[ch][cs]` (`CH0_PHY_RL_Control_CS0_B0`...) field `PHY_RL_CYCLE_DLY` | Cycle-delay THÔ, chung cho cả channel |
| PHY sub-reg `0x2 + 4*cs` (qua `nova_ddrphy_write`) | Delay MỊN (phase + phy_dly), riêng từng pup |
| PHY sub-reg `0x9A`/`0x9B` | Cờ trạng thái nhận diện preamble — PASS khi đúng `0x100`/`0x3F7F` (đã trích tài liệu 03) |
| `KM_DQS_GATE_SHMOO` (QSPI, qua `chip_info_reg`) | Bật ghi log toàn bộ `all_window[ch][cs][pup][phy_dly]` — chính là tính năng "Shmoo表示" mà spec chương 1 (`00_MASTER_OVERVIEW.md`, SPEC FACT "デバッグ機能") nhắc tới — ✅ xác nhận khớp giữa spec và source |
| `DQS_GATE_PREAMBLE_EN` (QSPI) | Chọn phương pháp Preamble (1, mặc định) hay Data Test (0) |

# Source view

- `mv_lpddr4_apn806.c:5677-6000` — `lpddr4_dqs_gate_training_preamble()`, đã đọc toàn bộ
- `mv_lpddr4_apn806.c:6004-...` — `lpddr4_dqs_gate_training()`, đã đọc phần đầu (khai báo + vòng quét delay), chưa đọc chi tiết điều kiện PASS
- `mv_lpddr4_apn806.c:7876, 7908` — khai báo + đọc QSPI cho `dqs_gate_preamble_en`
- `mv_lpddr4_apn806.c:8301-8321` — call site thật, mốc `TRN_E0`

# Call flow

```
(caller, sau Write Leveling — tài liệu 06)
        |
        v
if DQS_GATE_PREAMBLE_EN (QSPI) == 1:
    lpddr4_dqs_gate_training_preamble(mc_reg, warmboot, train_reg)
else:
    lpddr4_dqs_gate_training(mc_reg, warmboot, train_reg)
        |
        +-- (warmboot != 0): SKIP toàn bộ sweep, đọc lại giá trị đã lưu từ train_reg  ← xem tài liệu 15/16
        |
        +-- (warmboot == 0): for phy_dly = 0..511:
        |       set cycle_dly (thô) + adj_phy_dly (mịn)
        |       test PASS (preamble-status HOẶC data thật, tuỳ phương pháp)
        |       PASS liên tục, window>63           → chốt center, dừng pup này
        |       FAIL sau khi đã passing, window>40  → chốt center, dừng pup này
        |       FAIL, window<=40                    → reset, tiếp tục tìm Window khác (→ CHO PHÉP NHIỀU WINDOW)
        |
        +-- gộp CS0+CS1 (trung bình) → chọn cycle delay NHỎ NHẤT chung cho channel → bù riêng từng pup
        v
display_measure_time(TRN_E0)
```

# Diagram

```
phy_dly:     0 ......... 100 ............... 300 ......... 400 ......... 511
result:      FAIL       [PASS...window~50]   FAIL         [PASS...window~45]  FAIL
                          ^ window>40, đóng ở đây, CHỌN NGAY (không biết Window ở ~400 tồn tại)
```
→ Đây chính là cơ chế tạo ra khả năng **"nhiều Valid Window"** (Appendix B.3 của spec) và giải thích tại sao chọn *Window đầu tiên đủ lớn* có thể không phải Window đúng — nếu Window thật (theo ý JEDEC — ngay sau preamble thật) nằm ở vị trí thứ 2 nhưng Window giả (do alias/nhiễu định kỳ theo chu kỳ clock) xuất hiện trước và cũng đủ rộng (>40), firmware sẽ chọn nhầm Window giả.

✅ [ĐÃ XÁC NHẬN ở tài liệu 22 — Appendix B.3 đã đọc]: đây CHÍNH XÁC là bug thật (`Ap_B_3_DQS_Gateトレーニングで複数Windowが見える_20180601_1.pdf`, 2018/6/1). Nguyên nhân thật (theo spec): nhiễu xuất hiện ở trạng thái **Hi-Z SAU postamble** (không phải do "aliasing qua nhiều chu kỳ" như suy đoán ban đầu), vì Marvell mặc định không dùng Preamble mode. Cả 2 giải pháp trong bug report đều được hiện thực ở đây: (1) chọn Window ĐẦU TIÊN đủ ngưỡng — đúng logic 2-ngưỡng đã trace trên; (2) dùng Preamble mode — chính là `dqs_gate_preamble_en`. Chi tiết đầy đủ ở tài liệu 22.

# K-S800 customization

- Cờ QSPI `DQS_GATE_PREAMBLE_EN` cho phép đổi hẳn phương pháp training (Preamble ↔ Data Test) không cần build lại firmware — cùng dạng thiết kế với `FORCE_CA_TRAIN_IGNORE` (tài liệu 05).
- Cờ QSPI `KM_DQS_GATE_SHMOO` bật ghi toàn bộ bản đồ pass/fail — tính năng debug xác nhận khớp với SPEC FACT (Shmoo display) từ tài liệu 00.
- Nhánh `warmboot != 0`: toàn bộ vòng quét 512 bước bị bỏ qua, đọc thẳng từ `train_reg` (`TRN_RSLT_REGS_t`) — bằng chứng cụ thể đầu tiên cho cơ chế "không train lại khi resume" mà spec nhắc ở chương 8.2 (sẽ trace đầy đủ ở tài liệu 15).

# Debugging

- `"!!! DQS Gate Training Error PHYDelay[ch][cs][pup] = 0"` → 1 CS không tìm được Window nào cả (giá trị sentinel 0) — kiểm tra CS đó có board/chip lỗi vật lý, hoặc CA training (tài liệu 05)/Write Leveling (tài liệu 06) trước đó đã sai.
- `"DQS gate training failed for CH %d"` (`chosenCycleDelay[ch] < 0`) → **không có pup nào** tìm được cycle delay hợp lệ cho toàn channel — lỗi nghiêm trọng hơn, set `train_err_flg=1`.
- Nếu nghi ngờ chọn nhầm Window (đọc RAM lúc chạy thật không ổn định dù training báo PASS) → bật `KM_DQS_GATE_SHMOO`, đọc lại toàn bộ bản đồ `all_window[...]` để tự mắt kiểm tra có bao nhiêu Window thật tồn tại trên dải quét, so với Window mà firmware đã chọn.

# Summary

- DQS Gate mở "cổng nghe DQS" đúng lúc sau preamble — tránh đọc nhầm nhiễu trước preamble thành dữ liệu.
- 2 phương pháp thật tồn tại song song trong source (Preamble-status vs Data-test), chọn bằng 1 cờ QSPI, mặc định dùng Preamble.
- Delay chia 2 tầng: cycle (thô, theo chu kỳ clock) + phy_dly (mịn, trong 1 chu kỳ) — vì delay cần tìm có thể vượt quá 1 chu kỳ.
- Ngưỡng đóng Window **không phải 1 số duy nhất** — 63 nếu vẫn đang PASS, 40 nếu vừa FAIL — và code chọn Window ĐẦU TIÊN đủ ngưỡng, không phải Window rộng nhất trong toàn dải — đây là điểm khác CA training, và là ứng viên giải thích cho bug "nhiều Valid Window" trong spec.
- Cycle delay chỉ có 1 giá trị chung/channel (hardware), 4 pup chia sẻ bằng cách lấy min + bù riêng phần dư.
- Khi warm boot, toàn bộ bước này bị bỏ qua — đọc lại từ training result đã lưu.

# Verify yourself

1. Vì sao PHY cần "đóng cổng" trước preamble, thay vì chỉ tin tưởng rằng DQS lúc DRAM không gửi gì sẽ luôn ở mức ổn định (không cần gate)?
2. Giải thích lại bằng lời của bạn: vì sao delay ở đây cần 2 tầng (cycle + phy_dly) mà không dùng 1 số duy nhất như CA training (tài liệu 05) chỉ dùng `taps`?
3. Nếu ngưỡng đóng Window khi đang PASS (63) và khi vừa FAIL (40) bằng nhau, thuật toán có thay đổi hành vi không? Thử nghĩ 1 trường hợp cụ thể để kiểm tra.
4. Tại sao chọn cycle delay là giá trị NHỎ NHẤT trong 4 pup, không phải lớn nhất hay trung bình?
5. Nếu bạn phải sửa lỗi "chọn nhầm Window" (Appendix B.3), bạn sẽ đề xuất thay đổi gì trong logic ở trên — dựa trên chính cấu trúc code đã đọc, không suy đoán ngoài phạm vi?

**Tiếp theo:** `08_READ_CENTERING.md` — đã thấy 1 phần ở tài liệu 03 (ví dụ 2, `calculate_com`) — sẽ trace đầy đủ, phân biệt FIFO test và DMA test, và giải thích 2 lần Read Centering (`TRN_GX` lần 1, `TRN_H5`/`TRN_L1`/`TRN_L2` lần 2) trong bảng stage thật ở tài liệu 00.
