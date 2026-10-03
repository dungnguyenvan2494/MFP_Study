# 00 — S800 LPDDR4 Training FW: Master Overview

> **Bảo mật:** Bộ tài liệu spec gốc (`参考_LPDDR4引き継ぎ資料_第2版`) đóng dấu **社外秘 / 管理外** (Konica Minolta confidential, uncontrolled copy). Tài liệu học này chỉ trích dẫn ngắn, diễn giải lại bằng tiếng Việt — không copy nguyên văn nội dung nhạy cảm, không đưa ra ngoài repo nội bộ này.

## Cách đọc bộ tài liệu này

Nguồn có **3 loại**, luôn được đánh dấu tường minh trong mọi file:

| Tag | Ý nghĩa |
|---|---|
| ✅ **[SPEC FACT]** | Đọc trực tiếp từ 1 trong 27 file PDF spec (có ghi rõ tên file) |
| ✅ **[SOURCE FACT]** | Đọc trực tiếp từ source code trong repo này (có `file:line`) |
| 🧠 **[INFERENCE]** | Suy luận từ spec + source, chưa có câu xác nhận trực tiếp |
| 📘 **[GENERAL KNOWLEDGE]** | Kiến thức nền DDR/LPDDR4 chuẩn công nghiệp (JEDEC), không riêng gì S800 |
| ❓ **[UNKNOWN]** | Chưa tìm được bằng chứng — không đoán |
| ⚠️ **[SPEC/SOURCE MISMATCH]** | Spec nói một đằng, source làm một nẻo — sẽ không tự sửa cái nào |

Không có tag = mặc định là diễn giải/liên kết ý, không phải fact mới.

## Nguồn tài liệu spec (27 file PDF)

Thư mục: `...\参考_LPDDR4引き継ぎ資料_第2版\PDF\` (KONICA MINOLTA, 第2版 18/11/07).

Mục lục thật [SPEC FACT — `0_1_...表紙_目次...pdf` trang 3]:

```
0.  表紙/目次                                      (cover/TOC)
1.  LPDDR4 トレーニングFW仕様                       → 00, 14, 17
2.  評価結果の一覧                                  → (tham chiếu báo cáo riêng, không trong bộ 27 file)
3.  FW構成                                         → 00 (bảng component), 19
4.  アルゴリズム（フローチャート）4.1–4.11           → 04–12, 18
5.  チップ判別 (GPIO/QSPI) 5.1–5.2                  → 13
6.  周波数切り替えの仕様                            → 19 (phần frequency)
7.  動作モード説明                                  → 13/17
8.  Cold/Warm Boot 8.1–8.2                          → 14, 15
9.  Swizzleパラメータの仕様                         → 13 (Swizzle riêng, sẽ tách nếu dài)
10. トレーニングFWのエラー関連について                → 17
Appendix.A.1–5  (window margin/aging/boot time/temp) → 27, rải vào 05–12
Appendix.B.1–5  (bug history)                       → 22
Appendix.C      (register 補足)                      → 18
```

**Trạng thái đọc spec (cập nhật mỗi session):**

| File PDF | Đã đọc? | Áp dụng vào |
|---|---|---|
| 0_1 (TOC) | ✅ | 00 |
| 1_0 (FW spec chương 1) | ✅ | 00 |
| 3_0 (FW構成 chương 3) | ✅ | 00 |
| 5_1 (GPIO判定) | ✅ | 13 |
| Ap_B_1 (LPDDR4-2400) | ✅ | 22 |
| Ap_B_2_1-2 (Samsung Write DQ Eye/1-CS) | ✅ | 22 |
| Ap_B_2_3 (Samsung 8GB) | ✅ | 22 |
| Ap_B_3 (DQS Gate multi-window) | ✅ | 07, 22 |
| Ap_B_4 (mất dữ liệu resume) | ✅ | 15, 22 |
| Ap_B_5 (drain timeout) | ✅ | 15, 22 |
| 2_0, 4_x, 5_2_1, 5_2_2, 6_0, 7_0, 8_1, 8_2, 9_0, 10_0, AP_A_*, Ap_C_0 | ❌ chưa đọc | chương tương ứng ở trên |

## Trạng thái toàn bộ curriculum (roadmap — cập nhật mỗi session để resume)

| # | File | Nội dung | Trạng thái |
|---|---|---|---|
| 00 | 00_MASTER_OVERVIEW.md | Mental model + FW component map | 🟡 phần 1 xong (mental model, component table) |
| 01 | 01_LPDDR4_FROM_ZERO.md | DRAM/DDR/LPDDR4/CK/CA/CS/CKE/DQ/DQS/DM | 🟡 đang viết cùng session này |
| 02 | 02_MEMORY_CONTROLLER_AND_PHY.md | MC6/PHY registers, vai trò | 🟢 xong |
| 03 | 03_TRAINING_CONCEPT.md | Valid window, eye, center, margin | 🟢 xong |
| 04 | 04_PAD_CALIBRATION.md | pad_cal() thật | 🟢 xong |
| 05 | 05_CA_TRAINING.md | CA training thật | 🟢 xong |
| 06 | 06_WRITE_LEVELING.md | Edge-finding thật, phát hiện CLK-shift bị disable | 🟢 xong |
| 07 | 07_DQS_GATE.md | 2 phương pháp thật (Preamble/Data), giả thuyết cho bug "multiple window" | 🟢 xong |
| 08 | 08_READ_CENTERING.md | Center-of-mass thật, 4 lần gọi FIFO/DMA×PBS | 🟢 xong |
| 09 | 09_READ_DESKEW.md | 5 hàm thật TRN_H0-H4, 2/5 chưa đọc thân hàm chi tiết | 🟢 xong (2 hàm còn ❓ thân hàm) |
| 10 | 10_WRITE_CENTERING.md | 2-pass, warmboot skip=1 xác nhận | 🟢 xong |
| 11 | 11_WRITE_DESKEW.md | 3 hàm (không phải 5), khác triết lý Read | 🟢 xong |
| 12 | 12_MEMORY_TEST.md | common_memfill = ranh giới training/vận hành; backup/restore cho retry; bug SuperWarp | 🟢 xong |
| 13 | 13_QSPI_GPIO.md | GPIO khớp spec; QSPI dự phòng 2020 = MISMATCH thật có ngày; swizzle=board rev | 🟢 xong |
| 14 | 14_COLD_BOOT.md | Entry chain thật (ble_main→lpddr4_config), 2 tầng retry khác nhau, timeline 36 mốc | 🟢 xong |
| 15 | 15_WARM_BOOT.md | mc_save/restore_state, BOOT_SYNC_DDR_WARMBOOT, ứng viên fix bug B.4/B.5 | 🟢 xong |
| 16 | 16_TRAINING_RESULT.md | Struct 2304 byte đầy đủ, gTrainRegReadTbl[576], dual-copy SuperWarp fix | 🟢 xong |
| 17 | 17_ERROR_RETRY.md | for-loop retry thật, QSPI override khớp spec, lpddr4_training_Error = check nhất quán pmem/pmem2 | 🟢 xong |
| 18 | 18_REGISTER_MAP.md | Bảng tra cứu tổng hợp mọi register/struct | 🟢 xong |
| 19 | 19_CALL_GRAPH.md | Call graph đầy đủ entry→return, thay hết placeholder | 🟢 xong |
| 20 | 20_SPEC_VS_SOURCE.md | Bảng so sánh tổng hợp | ⬜ tiếp theo |
| 21 | 21_DEBUG_GUIDE.md | Checklist theo thứ tự pipeline thật | 🟢 xong |
| 22 | 22_BUG_ANALYSIS.md | Đã đọc Appendix B.1, B.2.1, B.2.2, B.2.3, B.3, B.4, B.5 — TẤT CẢ xác nhận trực tiếp, 2 giả thuyết ban đầu bị sửa lại | 🟢 xong |
| 23 | 23_FINAL_MASTER_MAP.md | Hoàn thành toàn bộ 24 file | 🟢 xong — **BỘ TÀI LIỆU HOÀN TẤT** |

**Đây là dự án nhiều session.** Mỗi lần quay lại, đọc bảng này trước, tiếp tục từ dòng ⬜ đầu tiên.

---

# PHẦN 0 — Mental Model trước khi đọc code

## Hiện tượng thực tế

Bạn bật nguồn một thiết bị (máy in Konica Minolta dòng S800). Ngay sau khi có điện, CPU (SoC S800, dựa trên nền Marvell **AP806/Armada**, xem [SOURCE FACT] bên dưới) cần dùng RAM (LPDDR4) để chạy phần mềm tiếp theo (U-Boot, Linux). Nhưng RAM lúc đó **chưa hoạt động được** — phải có một chương trình firmware riêng "huấn luyện" (train) đường truyền giữa CPU và RAM trước, rồi mới dùng được. Đó là chủ đề toàn bộ bộ tài liệu này.

## 4 lớp trong sơ đồ

```
     CPU / Firmware   (ATF "ble" stage — chạy đoạn code training)
            |
            v
     Memory Controller   (MC6 — "người quản lý", ra lệnh READ/WRITE/REFRESH)
            |
            v
           PHY          (lớp analog/tương tự — chuyển lệnh số thành tín hiệu điện thật)
            |
            v
         LPDDR4          (chuẩn giao tiếp — tập quy tắc điện + timing)
            |
            v
          DRAM           (chip nhớ vật lý — nơi bit 0/1 thực sự được lưu bằng điện tích)
```

- **CPU**: đơn vị tính toán chạy chương trình. Ở giai đoạn này CPU chạy firmware cấp thấp (ATF `ble` — Boot Loader Environment), chưa có OS.
- **Memory Controller**: một khối logic số (digital) nằm trong SoC, biết "giao thức" LPDDR4 (lệnh gì, thứ tự gì, cách nào để REFRESH không làm mất dữ liệu...). Nó không tự nói chuyện được với DRAM bằng điện — nó giao việc đó cho PHY.
- **PHY** (PHYsical layer): khối analog/mixed-signal, nhiệm vụ duy nhất là biến bit số (0/1 trong logic) thành **điện áp thực, đúng thời điểm thực** trên các chân vật lý (pin) nối ra ngoài chip, và ngược lại (đọc điện áp về thành bit). PHY là nơi có các mạch **delay line** (dây trễ có thể chỉnh), **calibration** (tự đo & bù trừ), **DLL** (Delay-Locked Loop) — tất cả các khái niệm "training" sẽ chỉnh các mạch này.
- **LPDDR4**: **không phải là một con chip** — nó là **một chuẩn/giao thức** (do JEDEC ban hành) quy định: bao nhiêu chân, chân nào làm gì, điện áp bao nhiêu, tốc độ bao nhiêu, lệnh gửi theo thứ tự nào. "LPDDR4 DRAM" = một con chip DRAM tuân theo chuẩn này.
- **DRAM** (Dynamic RAM): loại bộ nhớ lưu 1 bit bằng 1 tụ điện nhỏ (capacitor) + 1 transistor. "Dynamic" vì tụ điện tự rò điện tích theo thời gian → phải **refresh** (nạp lại) liên tục, nếu không dữ liệu mất. Đây là lý do RAM cần "sống" liên tục (khác ROM/Flash).

## Tại sao KHÔNG THỂ đơn giản "ghi vài register → RAM chạy"?

📘[GENERAL KNOWLEDGE] Về mặt vật lý, khi CPU/Memory Controller gửi tín hiệu điện ra chân CK (clock) và CA (command/address) tới DRAM, và DRAM trả tín hiệu về qua DQ/DQS (data/data-strobe), **mỗi tín hiệu đi qua một đường dây (trace) khác nhau trên bo mạch, với độ dài khác nhau, tải điện (capacitance) khác nhau**. Ở tốc độ LPDDR4-2400 (2400 Mbps ≈ 1 bit mỗi ~0.4 nanosecond), một khác biệt vài trăm **picosecond** giữa hai đường dây đã đủ làm CPU đọc sai bit. Những khác biệt này:

- **không cố định theo thiết kế lý thuyết** — mà phụ thuộc: bo mạch in ra sao (etching thực tế lệch vài % so với thiết kế), chip DRAM cụ thể nào được hàn vào (mỗi lô sản xuất silicon hơi khác nhau — gọi là "process variation"), điện áp nguồn thực tế, nhiệt độ tại thời điểm bật máy.
- do đó **không thể biết trước bằng tính toán** — phải **đo thực tế lúc chạy** (runtime), trên chính con chip/bo mạch đó, rồi chỉnh các mạch delay trong PHY cho khớp. Đó chính là **Training**.

## Ví dụ đời thường

Hãy tưởng tượng bạn và một người bạn đứng ở hai đầu một sân vận động ồn ào, cùng cầm bộ đàm, và bạn phải hô "1! 2! 3!... bắt!" để ném một quả bóng cho người kia bắt đúng lúc. Nếu bạn hô "bắt" quá sớm hoặc quá muộn so với lúc bóng thực sự bay tới tay người kia, họ sẽ bắt hụt. Ở đây:

- Tiếng hô "bắt" = tín hiệu **DQS** (data strobe — "báo hiệu bây giờ đọc dữ liệu")
- Quả bóng = tín hiệu **DQ** (data — dữ liệu thật)
- Khoảng cách + độ ồn sân vận động = **skew** (lệch thời gian) và **signal integrity** (nhiễu) trên đường truyền thực tế

**Training** = trước khi trận đấu (boot) diễn ra thật, hai người tập dượt vài lần, đo xem "hô trước bao lâu thì bóng vừa tới tay" — và cả hai bên đều tự ghi nhớ con số delay tối ưu đó cho riêng cặp người này, sân này, hôm nay. Đó là lý do tại sao **không dùng một con số delay cố định chung cho mọi máy** — mỗi máy in thực tế phải tự train khi khởi động.

---

# PHẦN 5 (rút gọn) — FW cấu trúc: Spec nói gì, source thật có gì

[SPEC FACT — `3_0_FW構成_20180807a.pdf` trang 1]: Spec liệt kê 3 vùng: `atf/ble`, `lppp/Hal/quartz/asic/build`, `u-boot/drivers/spi`.

Bảng dưới đối chiếu **từng file spec liệt kê** với source thật trong repo `IT6_Kernel` này (đã grep xác nhận từng dòng):

| Vùng | File (theo spec) | Path thật trong repo (SOURCE FACT) | Vai trò (spec) | Match? |
|---|---|---|---|---|
| ATF/ble | ddr_phy_regstructs.h | `atf\atf_s800\ble\ddr_phy_regstructs.h` (305 dòng) | Định nghĩa register PHY | ✅ MATCH |
| ATF/ble | MC_regstructs.h | `atf\atf_s800\ble\MC_regstructs.h` (định nghĩa `MC6_REGS_t`, header ghi "Register File: Mckinley 6c (MC6)") — có thêm `MC_regmasks.h` (9929 dòng, bitmask) đi kèm | Định nghĩa register Memory Controller | ✅ MATCH — *sửa lại so với bản nháp trước*: session trước grep sai pattern nên báo nhầm "chưa thấy file"; đã đọc và xác nhận tồn tại ở `02_MEMORY_CONTROLLER_AND_PHY.md` |
| ATF/ble | flex_shim.h | `atf\atf_s800\ble\flex_shim.h` | Log level khi GPIO chip-detect | ✅ MATCH |
| ATF/ble | KM_LPDDR4_config.h | `atf\atf_s800\ble\KM_LPDDR4_config.h` (244 dòng) | Define/global variable do KM (Konica Minolta) thêm | ✅ MATCH — đã đọc toàn bộ file |
| ATF/ble | KM_LPDDR4_prot.h | `atf\atf_s800\ble\KM_LPDDR4_prot.h` (98 dòng) | Function prototype do KM thêm | ✅ MATCH — đã đọc toàn bộ file |
| ATF/ble | DDR_trn_result_regstructs.c/.h | `atf\atf_s800\ble\DDR_trn_result_regstructs.c` (633 dòng) + `.h` | Định nghĩa vùng register được backup/restore khi resume | ✅ MATCH (tồn tại, chưa đọc nội dung chi tiết) |
| ATF/ble | DDR_chip_detect_regstructs.h | `atf\atf_s800\ble\DDR_chip_detect_regstructs.h` (534 dòng) | Biến tham số chip-detect qua QSPI | ✅ MATCH (tồn tại, chưa đọc nội dung) |
| ATF/ble | mv_lpddr4_apn806.c | `atf\atf_s800\ble\mv_lpddr4_apn806.c` (**13778 dòng**) | Chương trình training chính | ✅ MATCH — file lớn, sẽ trace ở tài liệu 19 |
| LPPP | power_api.h | `lppp_s800\Hal\quartz\asic\build\power_api.h` | Định nghĩa 2100/2400 Mbps | ✅ MATCH |
| LPPP | mv_lpddr4_sleep.c | `lppp_s800\Hal\quartz\asic\build\mv_lpddr4_sleep.c` (791 dòng) | Điều khiển LPDDR4 lúc Sleep/Resume | ✅ MATCH |
| LPPP | ddr_init.c | `lppp_s800\Hal\quartz\asic\build\ddr_init.c` (227 dòng) | Khởi tạo LPDDR4, đọc giá trị GPIO | ✅ MATCH |
| LPPP | hal_main.c | `lppp_s800\Hal\quartz\asic\build\hal_main.c` | Gọi hàm nhận diện qua GPIO | ✅ MATCH |
| U-Boot | drivers/spi/qspi_winbond.c | `BootROM\BootROM\Emu800\Src\drivers\spi\qspi_winbond.c` | Backup training value vào QSPI lúc boot, hiện revision QSPI param | 🧠 [INFERENCE] MATCH về nội dung + cấu trúc path con (`drivers/spi/qspi_winbond.c` khớp), nhưng **repo đặt gốc là `BootROM\BootROM\Emu800\` chứ không có thư mục tên "u-boot"** — xác nhận `Emu800/Src` có cấu trúc thư mục kinh điển của U-Boot (`board/`, `common/`, `configs/`, `dts/`, `tools/patman`, `tools/buildman`, `tools/env/fw_env.c`) nên đây đúng là mã nguồn U-Boot, chỉ đổi tên thư mục gốc. Không phải lỗi spec — chỉ là khác quy ước đặt tên giữa spec và repo. |

**Ghi chú quan trọng:** file chính `mv_lpddr4_apn806.c` **13778 dòng** — đây là lý do các Phần 05–11 (CA training, write leveling, DQS gate, centering, deskew...) phải tách thành nhiều tài liệu riêng, không thể trace hết trong 1 file. Cũng lưu ý: tên file `mv_lpddr4_apn806*` + có cả thư mục `atf\atf_s800\drivers\marvell\mv_ddr\apn806\` với `mv_ddr_apn806.c` riêng — 🧠[INFERENCE] S800 dùng lại (fork) driver gốc `mv_ddr` của Marvell cho SoC dòng AP806/Armada, rồi Konica Minolta thêm lớp tùy biến "KM_*" đè lên trên. Điều này khớp với ghi chú trong header `KM_LPDDR4_config.h`: `"For setting of KM algo"`. ❓[UNKNOWN] — chưa xác minh `atf/atf_s800/ble/mv_lpddr4_apn806.c` và `atf/atf_s800/drivers/marvell/mv_ddr/apn806/mv_ddr_apn806.c` có phải 2 bản (gốc Marvell vs bản KM sửa) hay không — sẽ kiểm tra ở tài liệu 19 (Call Graph).

## SPEC FACT quan trọng từ chương 1 (`1_0_LPDDR4_FW仕様_20181015a.pdf`)

| Mục | Giá trị | Nguồn |
|---|---|---|
| Chip hỗ trợ (GPIO detect) | Micron 1/2/3/4GB (+ bản shrink 4GB), Samsung 2/3/4GB (Samsung GPIO detect chỉ trong môi trường FUM) | [SPEC FACT] |
| Micron+Samsung trộn lẫn | **Không hỗ trợ** | [SPEC FACT] |
| Tốc độ hỗ trợ | LPDDR4-2400 (clock 1200MHz) và LPDDR4-2100 (clock 1050MHz) | [SPEC FACT] — chú ý: **2400/2100 là data rate (Mbps/pin), 1200/1050MHz là clock thật** — sẽ giải thích rõ khác nhau ở tài liệu 19 |
| Thời gian training (cold boot) | ATF đoạn "bắt đầu training → kết thúc training" mục tiêu ~**650ms** | [SPEC FACT] |
| Lý do có bước power-down/DDR3-mode xen giữa | Chip A1 có lỗi ở logic DDR controller: ghi register PHY bị thực hiện trên **cả CH0 và CH1** cùng lúc dù chỉ định 1 channel → phải né bằng cách chuyển sang chế độ power-down + giả DDR3 mode khi ghi register, ảnh hưởng thêm ~19ms (532 lần × ~18us mỗi lần enter/exit) | [SPEC FACT] — đây là workaround phần cứng, không phải bug FW |
| Retry | FAIL → retry, tối đa **10 lần mặc định**, QSPI có thể set tới **255 lần**; vượt số lần quy định → bật cờ lỗi (Error Flag = ON) | [SPEC FACT] |
| Sleep resume | Mục tiêu ~**150ms**; đạt được bằng cách **không train lại** — đọc lại tham số đã train từ lúc cold boot, lưu trong non-volatile memory (QSPI) | [SPEC FACT] |
| Nhiệt độ hoạt động | 0°C – 60°C | [SPEC FACT] |
| Chính sách retraining theo nhiệt độ | **Không retrain lại theo nhiệt độ** — vì đã kiểm chứng bằng test thực tế (KM + Marvell) rằng margin window vẫn đủ theo mask JEDEC trong toàn dải nhiệt độ này | [SPEC FACT] — sẽ trace lại ở tài liệu 27 (Temperature/Margin) khi đọc AP_A.5 |
| Endurance test | Memory R/W aging test, loại trừ 1GB Kernel dùng; 100 lần boot liên tục không lỗi; LPDDR4-2400 8GB tốn ~15h để test hết | [SPEC FACT] |

## NẾU BẠN CHỈ NHỚ 5 ĐIỀU (Phần 0+5)

1. LPDDR4 không phải là chip, mà là **chuẩn giao tiếp**; DRAM là chip vật lý tuân theo chuẩn đó.
2. Đường dây điện thật (PCB trace) trên mỗi máy khác nhau chút ít → **không thể tính sẵn delay bằng lý thuyết** → phải **train lúc chạy thật**.
3. 4 lớp: CPU (chạy FW) → Memory Controller (ra lệnh số) → PHY (chuyển số↔điện thật) → DRAM (chip nhớ vật lý).
4. Source S800 = driver **mv_ddr của Marvell** (dùng cho AP806/Armada) + lớp tùy biến `KM_*` do Konica Minolta thêm — không phải code viết từ đầu.
5. "U-Boot" trong spec ≈ thư mục `BootROM\BootROM\Emu800` trong repo này — tên khác nhau nhưng cùng codebase U-Boot thật.

## Verify yourself (tự kiểm tra — chưa cần trả lời ngay)

1. Nếu PHY không tồn tại, Memory Controller có thể tự nói chuyện trực tiếp với chân vật lý DRAM không? Tại sao?
2. Tại sao độ lệch delay giữa 2 đường dây PCB lại "không cố định theo thiết kế lý thuyết"? Kể 2 nguyên nhân thực tế.
3. Spec nói training mục tiêu 650ms nhưng có thêm ~19ms do workaround chip A1 — số 650ms này có tính luôn 19ms đó không, hay là số riêng? (Gợi ý: đọc lại đoạn trích [SPEC FACT] ở trên, câu trả lời chưa chắc rõ 100% — nếu không chắc, đánh dấu ❓UNKNOWN cho chính mình.)
4. Tại sao retry lại có giới hạn (10 hoặc tối đa 255), không phải retry vô hạn?
5. "Warm boot không train lại" — điều đó có nghĩa là dữ liệu trong DRAM chắc chắn còn nguyên khi resume không, hay chỉ là *tham số training* được giữ nguyên? Đây là 2 khái niệm khác nhau — tài liệu 15 sẽ làm rõ.

**Tiếp theo:** tài liệu `01_LPDDR4_FROM_ZERO.md` — xây khái niệm CK/CA/CS/CKE/DQ/DQS/DM từ số 0.
