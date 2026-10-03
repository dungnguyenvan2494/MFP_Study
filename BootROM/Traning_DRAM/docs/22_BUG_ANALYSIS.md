# 22 — Bug Analysis: Appendix B, đã đọc và xác nhận toàn bộ 7 bug

Đọc `00`-`21` trước. Session này đã đọc **toàn bộ Appendix B** (B.1, B.2.1, B.2.2, B.2.3, B.3, B.4, B.5) — mọi ứng viên ❓/🧠 đã đặt ra ở các tài liệu trước đều được xác nhận hoặc sửa lại chính xác ở đây. Đây là ví dụ rõ nhất của nguyên tắc Phần 37: SPEC ↔ SOURCE ↔ REGISTER phải được đối chiếu trực tiếp, không dừng ở suy luận.

---

## B.1 — LPDDR4-2400 không hoạt động (2018/7/29, gốc từ tài liệu Marvell)

**Hiện tượng**: FUM không đạt tốc độ mục tiêu 2400 trong thời gian dài.

**Nguyên nhân thật** [SPEC FACT]: giữa PHY và Memory Controller có 1 **FIFO bất đồng bộ (asynchronous FIFO)** cho Read Data — vì PHY và MC chạy ở 2 domain clock khác nhau, MC phải **chờ 1 khoảng thời gian cố định** (đặt bằng register) trước khi lấy dữ liệu từ FIFO này ra, để chắc chắn dữ liệu đã ổn định. Giá trị chờ này (`CH*_PHY_Control_1.phy_rfifo_rptr_dly`) cần **6** ở LPDDR4-2100 nhưng cần **7** ở LPDDR4-2400 (tần số cao hơn → chu kỳ ngắn hơn → cần chờ tỉ lệ nhiều hơn theo số chu kỳ). Dùng sai giá trị 6 ở tốc độ 2400 → nếu DQS bị delay lớn, thời gian chờ FIFO không đủ, dữ liệu chưa vào FIFO ổn định đã bị lấy ra → Window Read training bị "cắt cụt" (bên phải) — gọi là "Clipped Read Training Window".

**Đối chiếu SOURCE**: [SOURCE FACT — `mv_lpddr4_apn806.c:1474-1475`] `CH0/1_PHY_Control_1` được ghi hằng số `0x00004060` trong `MCConfig_...`, và đọc lại từ `m_capacity_info.CH0/1_PHY_Control_1` (tham số QSPI) ở nhiều nơi khác (`:1639, 1732, 1831`...). ❓[UNKNOWN — chưa decode bit `phy_rfifo_rptr_dly` cụ thể trong `0x4060`] — cần đọc `MC_regmasks.h` (9929 dòng, chưa đọc chi tiết) để xác nhận giá trị bit chính xác là 6 hay 7 tại tần số nào. Kết luận: ✅ MATCH cấu trúc (register tồn tại, được cấu hình theo QSPI/tần số), ❓ chưa xác nhận giá trị bit cuối cùng đúng 7 khi 2400.

---

## B.2.1 — Samsung: Write DQ Training, Eye bị chia làm 2 (2018)

**Hiện tượng**: Với chip Samsung 4GB, Write DQ training thấy 2 Window PASS tách biệt — code cũ chọn Window rộng hơn, nhưng đôi khi Window đó lại là **vùng NG thật** (do delay "vòng qua" ranh giới 1 chu kỳ — dly=64 ≡ dly=0).

**Nguyên nhân thật** [SPEC FACT, có trích code Marvell gốc]: thuật toán "trọng tâm" cũ dùng `fpDly[40]=0, lpDly[40]=63` cho 1 case cụ thể → tính trung điểm = 31 → RƠI VÀO VÙNG NG. Đây đúng là hệ quả của không gian delay hình tròn (0↔63 nối liền) mà tài liệu 03/08 đã ghi nhận (`if (selectedCenter<0) selectedCenter += maxDly`).

**Đối chiếu SOURCE — khớp CHÍNH XÁC**: [SPEC FACT trích công thức Marvell gốc] `selectedCenter[ch][cs][pup] = fpDly[...] + (lpDly[...] - fpDly[...]) / 2` — **đây đúng là công thức đã trích ở `mv_lpddr4_apn806.c:5010`** (tài liệu 03/08). ✅ Xác nhận: dòng code này CHÍNH LÀ công thức từng gây bug ở Samsung, đã được vá bằng kỹ thuật "Region1/Region2" (nếu phát hiện PASS cách Region1 hơn 10 bước, coi là Region2 do delay vòng, trừ đi `maxDly` rồi hợp nhất tính lại trung điểm). ✅ **Kết luận: MATCH TUYỆT ĐỐI — không chỉ ứng viên, đây là bằng chứng trực tiếp có trích dẫn công thức**.

---

## B.2.2 — Write DQ Training không hỗ trợ chip 1-CS (2018/5/20)

**Hiện tượng**: Chip Micron 1GB/2GB (cấu hình chỉ 1-CS, không phải 2-CS) bị lỗi training sau khi vá bug B.2.1.

**Nguyên nhân thật** [SPEC FACT, có trích code Marvell gốc và code KM sửa]: code Marvell gốc giả định LUÔN có 2 CS (`for (cs=0; cs<NUM_CS; cs++)` với `NUM_CS` cố định = 2) — với chip 1-CS, vòng lặp CS1 vẫn chạy dù không có dữ liệu thật, khiến biến trung gian `lpAvg` (chưa init cho CS1) mang giá trị RÁC từ lần tính trước, làm sai lệch toàn bộ phép so sánh Region1/Region2 ở byte-lane khác.

**Đối chiếu SOURCE — khớp CHÍNH XÁC**: [SPEC FACT trích code fix]:
```c
if (ch==0) num_cs = m_capacity_info.ch0num_cs;
else if (ch==1) num_cs = m_capacity_info.ch1num_cs;
for (cs=0; cs<num_cs; cs++) { ... }
```
✅ **Đây CHÍNH XÁC là pattern `num_cs = m_capacity_info.ch0num_cs`/`ch1num_cs`** xuất hiện **RẤT NHIỀU LẦN** khắp `mv_lpddr4_apn806.c` (CA training, Write Leveling, Write Centering, Write Deskew — tài liệu 05, 06, 10, 11 đều trích dòng này). ✅ **Kết luận: MATCH TUYỆT ĐỐI** — pattern "lấy đúng số CS theo từng channel" mà nhiều tài liệu trước coi là chi tiết nhỏ, thực chất là **fix trực tiếp cho bug B.2.2**, lan ra khắp codebase sau khi phát hiện.

---

## B.2.3 — Samsung 8GB: Write DQ Training lỗi ngẫu nhiên ~1/25 (2018/7/20-24)

**Hiện tượng**: Samsung 4GB×2 (8GB) — Write DQ training đôi khi TOÀN BỘ byte-lane sai (không phải 1 bit, mà cả byte trả về `0xFF`/`0x00`) — tỉ lệ ~1/25.

**Nguyên nhân thật** [SPEC FACT, đã xác nhận qua trao đổi trực tiếp với Samsung/Micron 2018/7/24]: đổi tần số 100MHz→1050/1200MHz lúc `CKE=High` có thể làm hỏng **Duty cycle** của CK trong khoảnh khắc chuyển tiếp → mạch "CLK latch" bên TRONG chip DRAM bị lệch. Nếu ngay lúc đó thoát Write-Leveling-mode theo thứ tự **DRAM thoát trước, Controller thoát sau**, DRAM đã về Normal Operation nhưng Controller vẫn nghĩ đang Write-Leveling → Controller gửi lệnh mà DRAM (với CLK-latch bị lệch) hiểu là Illegal Command → DRAM rơi vào "Bad state" → toàn bộ byte sai. Khuyến nghị của Samsung: đổi tần số lúc `CKE=Low` (trong Power-Down/Self-Refresh, việc này reset CLK-latch).

**Đối chiếu SOURCE**: [SOURCE FACT — đã trace ở tài liệu 05/06/17] chuỗi `enter_cbt()`→...→`exit_cbt()` (tài liệu 05) và `return_100mhz()` (tài liệu 17) đều có mẫu "đổi CS/CKE state → đổi tần số → đổi lại" — 🧠[INFERENCE, khớp cao] đây rất có khả năng chính là hiện thực hoá khuyến nghị "đổi tần số lúc CKE=Low" (vào Self-Refresh trước khi đổi, ra sau khi đổi xong) mà bug B.2.3 yêu cầu. ❓[UNKNOWN — chưa xác nhận trực tiếp]: chưa đọc dòng code cụ thể nào ghi rõ "Enter SR PowerDown → đổi freq → Exit SR PowerDown" bọc đúng quanh lệnh `change_clk_freq()` để xác nhận 100%; cũng chưa xác nhận thứ tự exit Write-Leveling-mode (Controller trước hay DRAM trước) trong `lpddr4_write_leveling()` (tài liệu 06) khớp với fix "Controller→Device" mà spec khuyến nghị.

---

## B.3 — DQS Gate: nhìn thấy nhiều Valid Window (2018/6/1)

**Hiện tượng**: Với Micron shrink, DQS Gate thấy ≥2 Valid Window; code cũ chọn Window RỘNG NHẤT, đôi khi chọn nhầm.

**Nguyên nhân thật** [SPEC FACT]: khi KHÔNG dùng Preamble mode (Marvell mặc định), sau **postamble** (ngay sau khi DQS ngừng toggle) đường DQS về trạng thái **Hi-Z + nhiễu** — nhiễu này đôi khi bị hiểu nhầm thành cạnh DQS thật → tạo ra "Valid Window giả" thứ 2. Window ĐẦU TIÊN luôn là Window THẬT (vì preamble luôn dài hơn 1 chu kỳ — theo JEDEC tRPRE ≥ 1.8tCK — nên không bao giờ bị hiểu nhầm là nhiễu).

**Đối chiếu SOURCE — khớp CHÍNH XÁC, đúng như dự đoán ở tài liệu 07**: [SPEC FACT] có **2 giải pháp, CẢ 2 ĐỀU ĐƯỢC HIỆN THỰC, chọn qua tham số QSPI**:
1. **Luôn chọn Window ĐẦU TIÊN tìm được, dừng ngay** — ✅ đây chính là logic 2-ngưỡng (63 khi đang PASS / 40 khi vừa FAIL, dừng ngay khi đủ ngưỡng) mà tài liệu 07 đã trace trong `lpddr4_dqs_gate_training()` — không phải "chọn rộng nhất" như code Marvell gốc mô tả trong bug, mà đã được sửa thành "chọn đầu tiên đủ ngưỡng".
2. **Dùng Preamble training mode** — ✅ đây CHÍNH XÁC là `dqs_gate_preamble_en`/`DQS_GATE_PREAMBLE_EN` (mặc định = 1!) đã trace ở tài liệu 07, chọn giữa `lpddr4_dqs_gate_training_preamble()` và `lpddr4_dqs_gate_training()`.

✅ **Kết luận: MATCH TUYỆT ĐỐI — cả 2 hàm ở tài liệu 07 chính là 2 giải pháp thật của bug này, không phải 2 phương pháp độc lập tình cờ cùng tồn tại.** ⚠️ Sửa lại 1 chi tiết ở tài liệu 07/20: nguyên nhân nhiễu là ở **Hi-Z SAU postamble**, không phải hiện tượng "aliasing qua nhiều chu kỳ clock" như suy đoán ban đầu.

---

## B.4 — Mất dữ liệu nếu Resume sau khi chờ lâu (2018/7/30)

**Hiện tượng**: DeepSleep ~30s rồi resume → không resume được / dữ liệu như bị phá — CHỈ xảy ra với Micron shrink 4GB(2+2) (không xảy ra ở cấu hình khác).

**Nguyên nhân thật** [SPEC FACT]: bug ở **LPPP FW** (không phải ATF) — lúc Suspend, code tính mask để vào Self-Refresh **giả định LUÔN có 2 CS**; với chip có cấu hình mmap khác (số CS thực tế), mask sai → **Self-Refresh KHÔNG được vào đúng** → trong lúc tưởng như đang DeepSleep, DRAM thực ra không tự refresh → dữ liệu rò điện tích mất dần theo thời gian.

**Đối chiếu SOURCE — khớp CHÍNH XÁC, xác nhận trực tiếp**: [SOURCE FACT — `mv_lpddr4_sleep.c:182-357`, đã trace đầy đủ ở tài liệu 15] `mc_enter_sr()`/`mc_exit_sr()`/`mc_enter_pd()` đọc `MMAP1_Low_CH0`/`MMAP1_Low_CH1` để xác định `ch0_cs1_isValid`/`ch1_cs1_isValid`, rồi tính `mask` theo ĐÚNG số CS thật (nhiều nhánh `switch` theo từng combo CS valid) — **đây CHÍNH LÀ đoạn code fix cho bug B.4**, không phải chỉ là "ứng viên nghi vấn" như tài liệu 15 đặt ra ban đầu. ✅ **Kết luận: MATCH TUYỆT ĐỐI**. ⚠️ Sửa lại tài liệu 15/20: `ddr_verify_crc()` (CRC32 debug tool) **không phải** công cụ chính điều tra bug này — công cụ debug đó là phụ; **fix thật** là chính logic tính mask theo CS-valid trong `mc_enter_sr`/`mc_enter_pd`.

---

## B.5 — Resume timeout ở bước drain (2018/10/15)

**Hiện tượng**: Resume treo ở `wfi`, log "CH0 timeout!!"/"CH1 timeout!!", `ch0 cs0` không vào được Self-Refresh.

**Nguyên nhân thật** [SPEC FACT] — ⚠️ **KHÁC HOÀN TOÀN** suy đoán ban đầu ở tài liệu 15: root cause **KHÔNG nằm trong `mv_lpddr4_sleep.c`/DDR code**. Nguyên nhân thật: **LCDC** (bộ điều khiển màn hình) lúc Suspend chỉ tắt Clock nhưng **để DMA (bit `CFG_GRA_ENA`) vẫn bật** — DMA này ở trạng thái không ổn định, liên tục phát sinh Read request tới DRAM → `mc_drain()` (chờ MC hết việc ghi/đọc đang chờ) **không bao giờ thấy MC idle thật** → timeout → ép vào Self-Refresh dù chưa "sạch" → ngay sau đó Read request tới lại phá Self-Refresh → lặp lại → dữ liệu mất theo thời gian **giống hệt cơ chế của B.4, nhưng nguyên nhân gốc khác hẳn**.

**Đối chiếu SOURCE**: [SOURCE FACT — `mv_lpddr4_sleep.c:490-533`, `mc_drain()`, đã trace ở tài liệu 15] log message `"CH0 timeout!!"`/`"CH1 timeout!!"` **khớp chữ-đối-chữ** với log được nêu trong chính bug report — ✅ xác nhận `mc_drain()` với `MC_STATUS_TIMEOUT` là **NƠI HIỆN TƯỢNG BỊ PHÁT HIỆN**, nhưng ⚠️ **KHÔNG phải nơi fix thật**. Fix thật nằm ở **driver LCDC** (`0xc057e190`, bit `CFG_GRA_ENA`), **ngoài phạm vi toàn bộ mã nguồn LPDDR4** đã đọc trong dự án này (không tìm trong `atf`, `lppp_s800/Hal/quartz` liên quan DDR). ⚠️ **Sửa lại quan trọng tài liệu 15/20**: giả thuyết "`MC_STATUS_TIMEOUT` chính là fix cho B.5" là **SAI** — timeout chỉ là cơ chế chống-treo-cứng đã có sẵn (có thể từ trước bug này), giúp lộ ra log để chẩn đoán, không phải bản thân là fix.

---

# Bài học tổng hợp (đúng tinh thần Phần 26 outline: không chỉ tóm tắt, phải hiểu cơ chế)

| Bug | Vị trí fix thật | Bài học |
|---|---|---|
| B.1 | PHY register (`PHY_Control_1`, giá trị theo tần số) | Timing bất đồng bộ giữa 2 clock domain (PHY/MC) cần margin lớn hơn ở tần số cao hơn |
| B.2.1 | Công thức center of mass (đã trace tài liệu 03/08) | Không gian delay là VÒNG (0↔63 nối liền), không phải đường thẳng — mọi thuật toán center phải xử lý wrap-around |
| B.2.2 | Toàn bộ codebase (`num_cs` per-channel) | 1 giả định sai ("luôn 2 CS") lan khắp hàng chục hàm — sửa 1 chỗ không đủ, phải sửa toàn bộ pattern |
| B.2.3 | Thứ tự exit mode + CKE state lúc đổi tần số | Silicon DRAM có trạng thái nội bộ (CLK latch) nhạy với thời điểm chuyển tần số — không chỉ là vấn đề timing tín hiệu bên ngoài |
| B.3 | 2 giải pháp song song, chọn bằng QSPI flag | Đôi khi không có 1 fix "đúng nhất" — cung cấp cả 2 lựa chọn, để field/QSPI quyết định theo từng board |
| B.4 | Tính mask theo CS-valid thật (LPPP) | Giả định cấu hình cố định (luôn 2 CS) là nguồn lỗi LẶP LẠI (giống B.2.2) ở 1 lớp code hoàn toàn khác (LPPP, không phải ATF) |
| B.5 | LCDC driver (ngoài toàn bộ code LPDDR4) | ⚠️ Bug "trông giống lỗi DDR" (log xuất phát từ code DDR) có thể có root cause ở **subsystem hoàn toàn khác** — không được kết luận vội chỉ từ vị trí xuất hiện log |

**Tiếp theo:** `23_FINAL_MASTER_MAP.md` — sơ đồ tổng thể cuối cùng, đã được hiệu chỉnh hoàn toàn theo source thật.
