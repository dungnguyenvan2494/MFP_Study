# 20 — Spec vs Source: bảng tổng hợp

Đọc `00`-`19` trước. Tài liệu tổng hợp mọi đối chiếu SPEC↔SOURCE đã thực hiện — không thêm phân tích mới, chỉ gom lại để nhìn toàn cảnh.

⚠️ **Giới hạn quan trọng của tài liệu này**: mới đọc được **4/27 file PDF spec** (`0_1`, `1_0`, `3_0`, `5_1`) trong toàn bộ session. Bảng dưới chỉ phản ánh đúng phần đã đọc — **không suy rộng ra toàn bộ spec**. Các chương spec chưa đọc (4.x thuật toán chi tiết, 6.0 tần số, 8.x cold/warm boot, 9.0 swizzle, 10.0 error, Appendix A/B/C) có thể chứa thêm MATCH/MISMATCH chưa phát hiện.

| # | Chủ đề | Spec nói gì | Source thật | Kết quả | Tài liệu |
|---|---|---|---|---|---|
| 1 | Chip hỗ trợ | Micron 1-4GB (+shrink), Samsung 2-4GB (chỉ GPIO ở FUM) | `CH0/1_MEM_VENDER` = `0x11111111`(Micron)/`0x00000001`(Samsung) trong QSPI param | ✅ MATCH | 00, 13 |
| 2 | Tốc độ | LPDDR4-2400(1200MHz)/2100(1050MHz) | `ddr_top_freq` = 1200/1050, `change_clk_freq()` | ✅ MATCH (khái niệm) | 00, 01 |
| 3 | Retry | 10 mặc định, tối đa 255 qua QSPI | `retry_num = (TRAIN_RETRY_NUM>255)?10:TRAIN_RETRY_NUM` | ✅ MATCH TUYỆT ĐỐI | 00, 17 |
| 4 | Sleep resume mục tiêu | ~150ms, không train lại | `warmboot=1` → skip toàn bộ sweep ở DQS Gate/Read/Write Centering/Deskew, đọc lại `train_reg` | ✅ MATCH (cơ chế đúng như mô tả) | 00, 07, 08, 10, 15 |
| 5 | Không retrain theo nhiệt độ | Đã test 0-60°C, margin đủ theo JEDEC | ❓ chưa đọc code liên quan margin-theo-nhiệt-độ trực tiếp | ❓ UNKNOWN (chưa verify source) | 00, 03 |
| 6 | FW cấu trúc atf/ble | Liệt kê 9 file cụ thể | Tất cả 9 file tồn tại, khớp path (trừ tên gọi) | ✅ MATCH | 00 |
| 7 | FW cấu trúc "u-boot" | `u-boot/drivers/spi/qspi_winbond.c` | `BootROM\BootROM\Emu800\Src\drivers\spi\qspi_winbond.c` — cấu trúc con khớp, tên thư mục gốc khác | 🧠 MATCH nội dung, khác quy ước đặt tên | 00 |
| 8 | GPIO bit mapping | `GPIOH[26:31]` dung lượng, `GPIOI[22:25]` vendor | Đọc đúng offset `0xE830A000/0xE8244824/0xE830A800`, khớp flowchart (né qua Timer2) | ✅ MATCH HOÀN TOÀN | 13 |
| 9 | QSPI checksum | 1 vùng, khớp thì dùng, không khớp → GPIO | Source có **thêm nhiều vùng QSPI dự phòng** (`lpddr4_param[]`), bug-fix `2020/02/26 OP_BTS-20942`, SAU ngày spec phát hành | ⚠️ **MISMATCH THẬT, có ngày xác nhận** | 13 |
| 10 | Write Leveling | (chưa đọc spec 4.4 chi tiết) | Nửa "CLK-shift" của thuật toán bị **comment-out hoàn toàn**, chỉ "DQS-shift" chạy | ⚠️ NGHI VẤN MISMATCH — chưa đọc spec 4.4 để xác nhận | 06 |
| 11 | DQS Gate — "nhiều Valid Window" | B.3: nhiễu Hi-Z sau postamble → 2 giải pháp: chọn Window đầu tiên / dùng Preamble mode | 2 ngưỡng đóng Window (63/40, chọn Window ĐẦU TIÊN) + cờ `dqs_gate_preamble_en` | ✅ MATCH TUYỆT ĐỐI — cả 2 giải pháp của spec đều là 2 hàm thật đã trace | 07, 22 |
| 12 | Sleep — mất dữ liệu (B.4) | Mask vào Self-Refresh giả định luôn 2-CS, sai với chip 1-CS | `mc_enter_sr()`/`mc_enter_pd()` tính mask theo `ch0/1_cs1_isValid` đọc từ MMAP thật | ✅ MATCH TUYỆT ĐỐI — đây chính là code fix, không phải chỉ công cụ điều tra | 15, 22 |
| 13 | Sleep — drain timeout (B.5) | Root cause ở LCDC driver (DMA không tắt lúc Suspend), không phải ở code DDR | `mc_drain()` `MC_STATUS_TIMEOUT` chỉ là nơi hiện tượng lộ ra (log khớp chữ-đối-chữ), KHÔNG phải fix | ⚠️ XÁC NHẬN: fix thật NẰM NGOÀI toàn bộ code LPDDR4 đã đọc | 15, 22 |
| 14 | Samsung Write DQ Eye tách 2 (B.2.1) | Công thức center gốc không xử lý delay "vòng" (wrap-around) | `selectedCenter = fpDly + (lpDly-fpDly)/2` tại `:5010` — khớp NGUYÊN VĂN công thức trong bug report | ✅ MATCH TUYỆT ĐỐI, có trích dẫn công thức 2 chiều | 03, 08, 22 |
| 15 | Chip 1-CS không hỗ trợ (B.2.2) | Code gốc giả định luôn 2-CS trong mọi vòng lặp CS | Pattern `num_cs = ch0num_cs/ch1num_cs` lặp lại khắp CA training/WL/WC/WD | ✅ MATCH TUYỆT ĐỐI, có trích dẫn code 2 chiều | 05, 06, 10, 11, 22 |
| 16 | LPDDR4-2400 không chạy (B.1) | FIFO bất đồng bộ PHY↔MC cần wait-time lớn hơn ở tần số cao (6→7) | `CH0/1_PHY_Control_1` ghi hằng số `0x4060`, cũng đọc từ QSPI | 🧠 MATCH cấu trúc, ❓ chưa decode bit để xác nhận giá trị 6/7 | 22 |
| 17 | Samsung 8GB lỗi ngẫu nhiên (B.2.3) | Đổi tần số lúc CKE=High làm hỏng CLK-latch nội bộ DRAM; cần đổi lúc CKE=Low + đổi thứ tự exit Write-Leveling | `enter_cbt/exit_cbt`, `return_100mhz()` có mẫu SR-quanh-đổi-tần-số | 🧠 MATCH khả năng cao, ❓ chưa xác nhận trực tiếp thứ tự exit WL trong `lpddr4_write_leveling()` | 05, 06, 17, 22 |
| 14 | Đơn vị FW cấu trúc lppp | `mv_lpddr4_sleep.c`, `ddr_init.c`, `hal_main.c` | Cả 3 file tồn tại đúng path | ✅ MATCH | 00, 15 |
| 15 | Training result storage | Chương 3 chỉ nhắc "định nghĩa vùng ghi lại lúc resume" | Struct `TRN_RSLT_REGS_t` 2304 byte, khớp đúng TỪNG GIÁ TRỊ đã trace ở tài liệu 04-11 | ✅ MATCH, source chi tiết hơn spec mô tả | 00, 16 |

# Phân loại tổng hợp (cập nhật sau khi đọc toàn bộ Appendix B ở tài liệu 22)

- **✅ MATCH xác nhận trực tiếp**: 1, 2, 3, 4, 6, 8, 11, 12, 14, 15 — bao gồm cả cấu trúc file/GPIO/số liệu VÀ 4 bug (B.2.1, B.2.2, B.3, B.4) đã đọc trực tiếp spec + đối chiếu đúng dòng code.
- **⚠️ MISMATCH thật, có bằng chứng ngày tháng**: 9 (QSPI dự phòng 2020) — và 13 (B.5): fix thật nằm NGOÀI toàn bộ code LPDDR4, khác hẳn giả định ban đầu rằng `mc_drain()` timeout là fix.
- **🧠 MATCH khả năng cao, còn 1 chi tiết nhỏ chưa decode**: 16 (B.1 — chưa decode bit register), 17 (B.2.3 — chưa xác nhận thứ tự exit Write-Leveling).
- **❓ UNKNOWN, chưa đủ dữ liệu**: 5 — chưa đọc code liên quan margin-nhiệt-độ.
- **Bài học quan trọng nhất từ việc đọc đủ Appendix B**: 2 trong 7 giả thuyết ban đầu (dựa trên tên biến/comment khớp) đã **SAI khi đối chiếu trực tiếp** (B.4: không phải `ddr_verify_crc`; B.5: không phải `mc_drain` timeout) — xác nhận đúng nguyên tắc Phần 37/38: không được dừng ở suy luận dù có vẻ hợp lý, PHẢI đọc bằng chứng gốc.

# Nguyên tắc đã tuân thủ (nhắc lại cho người đọc mới)

Không có mục nào trong bảng trên được "tự sửa" theo hướng: nếu spec và source khác nhau, **cả 2 đều được giữ nguyên, chỉ ghi nhận sự khác biệt** — không đoán bên nào "đúng hơn" trừ khi có bằng chứng thời gian rõ ràng (như mục 9).

**Tiếp theo:** `21_DEBUG_GUIDE.md` — checklist debug tổng hợp từ mọi mục "Debugging" đã viết rải rác ở tài liệu 02-17.
