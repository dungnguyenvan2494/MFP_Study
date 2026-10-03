# 01 — LPDDR4 từ con số 0

Đọc `00_MASTER_OVERVIEW.md` trước. File này dựa trên mental-model đã dựng ở đó (CPU → Memory Controller → PHY → LPDDR4 → DRAM).

## 1. DRAM là gì?

📘[GENERAL KNOWLEDGE] DRAM (Dynamic Random-Access Memory) lưu 1 bit bằng **1 tụ điện nhỏ** (capacitor): tụ có điện tích = bit 1, tụ rỗng = bit 0. "Dynamic" vì tụ điện tự rò điện tích qua thời gian (vài chục ms) → mạch phải liên tục **refresh** (đọc rồi ghi lại) từng dòng nhớ, nếu không dữ liệu biến mất dù vẫn có điện. Đây khác với SRAM (dùng transistor giữ trạng thái, không cần refresh nhưng đắt/to hơn) và Flash/ROM (giữ dữ liệu không cần điện).

## 2. DDR nghĩa là gì?

📘[GENERAL KNOWLEDGE] DDR = **Double Data Rate**. Ý tưởng: bus dữ liệu truyền **2 bit trên mỗi chu kỳ clock** — 1 bit ở cạnh lên (rising edge), 1 bit ở cạnh xuống (falling edge) của xung clock — thay vì chỉ 1 bit/chu kỳ như SDR (Single Data Rate) đời trước. Vì vậy tốc độ dữ liệu (Mbps) luôn gấp đôi tần số clock thật (MHz). Đây chính là gốc của con số "2400" vs "1200MHz" mà spec S800 nhắc tới — sẽ giải thích chi tiết ở mục dưới.

## 3. LPDDR khác DDR thông thường thế nào?

📘[GENERAL KNOWLEDGE] LPDDR = **Low Power DDR**, dòng chuẩn DDR dành riêng cho thiết bị cần tiết kiệm điện (điện thoại, thiết bị nhúng, máy in...) thay vì máy tính để bàn/server. Khác biệt chính: điện áp lõi thấp hơn (ví dụ LPDDR4 dùng VDDQ ~1.1V so với DDR4 ~1.2V, và có thêm các mode ngủ sâu như Self-Refresh với điện áp cực thấp), gói chip khác (thường hàn trực tiếp lên board dạng POP/PoP hoặc BGA gần CPU, không dùng khe DIMM rời), và có tập lệnh quản lý năng lượng riêng.

## 4. LPDDR4 là gì?

📘[GENERAL KNOWLEDGE] LPDDR4 là phiên bản thứ 4 của chuẩn LPDDR (do JEDEC ban hành), tốc độ dữ liệu danh định từ khoảng 1600 đến 4266 Mbps/pin. [SPEC FACT — `1_0` §1] S800 dùng 2 mức: **LPDDR4-2100** (clock thật 1050MHz) và **LPDDR4-2400** (clock thật 1200MHz).

**Data rate vs Clock frequency — không nhầm lẫn:**

```
Data rate (Mbps/pin) = Clock frequency (MHz) × 2   (vì DDR truyền 2 bit/chu kỳ, xem mục 2)

2400 Mbps  =  1200 MHz × 2
2100 Mbps  =  1050 MHz × 2
```

"2400" và "2100" trong spec là **tốc độ dữ liệu**, không phải tần số clock thật chạy trên chân CK — chân CK thật sự dao động ở 1200MHz hoặc 1050MHz.

## 5. Bus dữ liệu là gì?

📘[GENERAL KNOWLEDGE] Bus dữ liệu là tập hợp các đường dây song song (thường 16 hoặc 32 đường DQ cho mỗi channel LPDDR4) dùng để truyền nhiều bit cùng lúc, thay vì truyền nối tiếp 1 bit. 🧠[INFERENCE, dựa trên mảng `[2][2][4]` xuất hiện lặp đi lặp lại trong `atf/atf_s800/ble/mv_lpddr4_apn806.c:3696-3713` và `KM_LPDDR4_config.h:67-115`] Firmware S800 tổ chức bus theo 3 chiều: **2 channel (CH0/CH1)** × **2 CS (chip-select, tức 2 "hạng" chip nhớ trên cùng 1 channel)** × **4 "pup"** (PHY Unit Pad — mỗi pup quản lý một nhóm DQ, thường là 1 byte-lane). Số chiều `[4]` khớp với cách chia DQ 32-bit/channel thành 4 nhóm 8-bit (byte lane) để mỗi nhóm có Vref/delay riêng.

## 6. Clock là gì?

📘[GENERAL KNOWLEDGE] Clock (CK) là tín hiệu tuần hoàn (sóng vuông) dùng làm "nhịp đếm thời gian chung" cho cả Memory Controller và DRAM, để cả 2 bên biết "bây giờ là chu kỳ thứ mấy" và đồng bộ khi nào lệnh/dữ liệu hợp lệ.

## 7. Timing là gì?

📘[GENERAL KNOWLEDGE] Timing là tập các mốc thời gian JEDEC quy định DRAM phải tuân theo (ví dụ: sau lệnh Activate phải chờ bao lâu mới được Read, sau khi ghi Mode Register phải chờ bao lâu mới gửi lệnh tiếp theo...). Các mốc này thường được ký hiệu `tXXX` (ví dụ tMRD, tRCD...). Phần 28 của curriculum (chưa viết) sẽ liệt kê các tXXX mà spec S800 thực sự dùng.

## 8. Delay là gì?

📘[GENERAL KNOWLEDGE] Delay là khoảng thời gian một tín hiệu bị "trễ" khi đi qua một đường dây/mạch, so với thời điểm nó được phát ra. Trong PHY, có các **delay line** — mạch điện có thể chỉnh bằng số (ghi 1 giá trị vào register) để **thêm chủ động** một khoảng trễ nhỏ (cỡ picosecond) vào một tín hiệu cụ thể (ví dụ trễ thêm DQS đi vài chục ps). Đây chính là "nút chỉnh" mà mọi thuật toán training thực chất đang dò tìm giá trị tối ưu để ghi vào.

## 9. Skew là gì?

📘[GENERAL KNOWLEDGE] Skew là **độ lệch thời gian không mong muốn** giữa 2 tín hiệu lẽ ra phải đến cùng lúc (ví dụ DQ0 và DQ1 của cùng 1 byte lane, hoặc CK và DQS). Nguyên nhân: độ dài đường dây PCB khác nhau, tải điện (capacitance) khác nhau ở mỗi chân, sai lệch sản xuất silicon giữa các driver/receiver trong PHY. Skew là "kẻ thù" mà deskew training (Phần 09/11) phải đo và triệt tiêu bằng delay line.

## 10. Signal integrity là gì?

📘[GENERAL KNOWLEDGE] Signal integrity là mức độ "sạch" của dạng sóng điện thực tế nhận được, so với dạng sóng vuông lý tưởng phát ra. Ở tốc độ cao (1200MHz+), tín hiệu bị biến dạng bởi: phản xạ trên đường dây (nếu trở kháng không khớp — đây là lý do có **ODT**, On-Die Termination, và **drive strength** phải calibrate), nhiễu xuyên âm giữa các đường dây cạnh nhau (crosstalk), suy hao theo tần số. Kết quả là "cửa sổ" thời gian/điện áp mà tín hiệu còn đọc được chính xác bị co lại — đây chính là khái niệm **eye/window** sẽ học ở tài liệu 03.

---

## Sơ đồ tín hiệu thật

```
                    Memory Controller (MC6)
                             |
              +--------------+---------------+
              |                              |
           Clock/Command                    Data
              |                              |
             CK                         DQ / DQS / DM
              |
           CA / CS / CKE
```

Giải thích từng tín hiệu (📘[GENERAL KNOWLEDGE], tên chân theo chuẩn JEDEC LPDDR4 — đối chiếu với evidence thật trong source S800 khi có):

| Tín hiệu | Tên đầy đủ | Vai trò | Bằng chứng trong S800 |
|---|---|---|---|
| **CK** | Clock | Nhịp đồng hồ chung, mọi tín hiệu khác được lấy mẫu theo cạnh của CK | 📘 — chưa grep được register CK riêng lẻ trong `ddr_phy_regstructs.h`; ❓[UNKNOWN] tên register CK delay cụ thể, sẽ tìm ở tài liệu 05 (CA training) |
| **CA** | Command/Address (bus) | Gửi lệnh (Activate/Read/Write/Refresh/MRW...) và địa chỉ hàng/cột dưới dạng mã hoá trên vài đường CA song song, đồng bộ theo CK | Chương 4.3 của spec gọi thẳng là "CAトレーニング" — [SPEC FACT, TOC] |
| **CS** | Chip Select | Chọn "chip/rank" nào trong số nhiều chip trên cùng channel sẽ nhận lệnh hiện tại. S800 có 2 CS (CS0/CS1) mỗi channel | [SOURCE FACT — `KM_LPDDR4_config.h:157-173`: các register `CH0_MC_CONFIG_CS0`, `CH0_MC_CONFIG_CS1`, `CH1_MC_CONFIG_CS0`, `CH1_MC_CONFIG_CS1`, và mảng `[2][2][4]` = [CH][CS][PUP] ở `mv_lpddr4_apn806.c:3696-3713`] |
| **CKE** | Clock Enable | Cho phép/tắt việc DRAM tiếp tục nhận nhịp CK — dùng để đưa DRAM vào các mode tiết kiệm điện (Power-Down, Self-Refresh) mà không cắt hẳn nguồn | 📘 — chưa tìm register CKE cụ thể trong các file đã đọc; ❓[UNKNOWN] |
| **DQ** | Data | Bus dữ liệu thật (đọc/ghi), tổ chức theo byte-lane (pup) | [SOURCE FACT — `mv_lpddr4_apn806.c:3683` hàm `lpddr4_read_centering(MC6_REGS_t *mc_reg, ...)` xử lý theo `[ch][cs][pup]`] |
| **DQS** | Data Strobe | Tín hiệu "báo hiệu thời điểm dữ liệu trên DQ hợp lệ để lấy mẫu" — đi kèm sát DQ, do bên gửi dữ liệu (DRAM khi đọc, Controller khi ghi) tạo ra | [SOURCE FACT — `atf/atf_s800/ble/ddr_phy_regstructs.h:142`: register `wl_dqs_pattern // 0x6dc — Write Leveling DQS Pattern`] |
| **DM** | Data Mask | 1 bit đi kèm mỗi byte DQ, cho phép Controller báo "byte này bỏ qua, không ghi" khi Write — dùng khi chỉ muốn ghi 1 phần dữ liệu | [SPEC FACT — TOC mục 4.7.2/4.9.2 gọi thẳng "read dm deskew"/"write dm deskew"; và `KM_LPDDR4_prot.h` liệt kê hàm liên quan DBI/DM: `enable_dbi_dm()` tại dòng 86] |

⚠️ Ghi chú tính trung thực: các ô ❓[UNKNOWN] ở trên (địa chỉ chính xác của CK/CKE trong PHY register map) **chưa được xác minh** trong session này — không suy đoán địa chỉ. Sẽ lấp đầy khi đọc kỹ `ddr_phy_regstructs.h` toàn bộ 305 dòng ở tài liệu 02/18.

## NẾU BẠN CHỈ NHỚ 5 ĐIỀU (Phần 1)

1. **2400/2100 là tốc độ dữ liệu (Mbps), không phải tần số clock** — clock thật là 1200MHz/1050MHz, vì DDR truyền 2 bit/chu kỳ.
2. DRAM lưu bit bằng tụ điện → tự mất điện tích → phải refresh liên tục.
3. CA = lệnh+địa chỉ, CS = chọn chip nào nhận lệnh, CKE = cho phép nhận clock (quản lý power mode), DQ = dữ liệu thật, DQS = "còi báo giờ đọc dữ liệu", DM = mặt nạ che byte không muốn ghi.
4. Skew (lệch thời gian không mong muốn) và Signal integrity (méo dạng sóng) là 2 lý do vật lý khiến delay không thể tính sẵn trên giấy.
5. S800 tổ chức bus theo 3 chiều thật trong code: 2 channel × 2 CS × 4 pup (byte-lane) — không phải khái niệm trừu tượng, mà là hình dạng mảng thật trong `mv_lpddr4_apn806.c`.

## Verify yourself

1. Nếu clock thật là 1050MHz (LPDDR4-2100), tốc độ dữ liệu Mbps là bao nhiêu? Tính bằng công thức ở mục 4.
2. DM và DQS khác nhau ở điểm nào — một cái báo "khi nào đọc", một cái báo "phần nào bỏ qua khi ghi". Nếu DM bị lệch thời gian (skew) so với DQ, chuyện gì có thể xảy ra khi ghi dữ liệu?
3. Vì sao CKE (không phải CK) là tín hiệu dùng để đưa DRAM vào Self-Refresh, mà không phải là việc tắt hẳn CK?
4. Mảng `[2][2][4]` trong source ứng với [CH][CS][PUP] — nếu S800 có tổng cộng 2 channel, mỗi channel 2 CS, mỗi CS có DQ 32-bit chia thành 4 pup, thì mỗi pup quản lý bao nhiêu bit DQ?
5. Tại sao "signal integrity kém" (nhiều nhiễu/phản xạ) lại làm cửa sổ thời gian hợp lệ (window) bị **co lại**, chứ không phải làm tín hiệu sai hoàn toàn mọi lúc?

**Tiếp theo:** `02_MEMORY_CONTROLLER_AND_PHY.md` — MC6 và PHY là gì trong S800 cụ thể, dựa trên `ddr_phy_regstructs.h` (305 dòng, sẽ đọc toàn bộ) và `MC_regmasks.h`.
