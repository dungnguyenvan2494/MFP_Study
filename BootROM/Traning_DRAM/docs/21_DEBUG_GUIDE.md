# 21 — Debug Guide: checklist tổng hợp

Đọc `00`-`20` (đặc biệt `22` nếu đã có — bug thật luôn ưu tiên hơn suy đoán) trước khi dùng checklist này thật. Đây tổng hợp mọi mục "Debugging" đã viết rải rác, sắp theo đúng THỨ TỰ chạy thật (tài liệu 14/19), để bạn biết "board treo ở đâu thì nhìn gì".

# Checklist theo thứ tự thất bại có thể xảy ra

## 0. Board không vào được `ble_main()`/`lpddr4_config()` (tài liệu 14)
- Không phải vấn đề LPDDR4 — kiểm tra BootROM, clock/power SoC cơ bản trước.
- Xem `p_sram` (`0xC0200000`) có bị ghi `0xE5` (sentinel init) hay không — nếu dump ra vẫn toàn `0xE5`, `ble_main()` chưa từng chạy tới đoạn sau.

## 1. Chip detection sai (tài liệu 13)
- Log `checksum_qspi`/`param_qspi_flag` — nếu vùng chính không khớp, xem tiếp `sub_param_qspi_flag` từng vùng dự phòng.
- Kiểm tra `km_board_rev` (từ `gpioa_value` bit [7:4]) đúng với board thật không — sai board_rev → sai `swizzle_patterns[]` → deskew lạ trên 1-2 đường DQ cụ thể.
- Dump toàn bộ `CHIP_DETECT_REGS_t` (`chip_info_reg`) trước tiên nếu nghi ngờ bất kỳ hành vi bất thường — đây là "bảng điều khiển trung tâm" (`FORCE_QSPI_MODE`, `FORCE_CA_TRAIN_IGNORE`, `DQS_GATE_PREAMBLE_EN`, `KM_*_SHMOO`, `TRAIN_RETRY_NUM`...).

## 2. Pad Calibration fail (tài liệu 04)
- `"!!!Failed Calibration init!!!"` → máy trạng thái nội bộ không xong — nghi PHY chưa cấp clock/reset đúng.
- `"!!!Failed Calibration propogation to IO!!!"` → tính xong nhưng chưa "tới" pad thật — nghi vấn đề board/pad vật lý.
- Cả 2 lỗi này xảy ra SỚM NHẤT trong chuỗi — nếu gặp, mọi bước sau (05-12) hầu như chắc chắn cũng fail theo.

## 3. CA Training fail (tài liệu 05)
- Kiểm tra `wait_for_dll_lock()` timeout trước — CA training chạy ở tần số cao, cần DLL khoá pha ổn định.
- Chỉ 1-2 CA bit cụ thể fail (không phải CS) → nghi vấn đề PCB/skew riêng đường đó, không phải lỗi thuật toán chung.
- Kiểm tra `FORCE_CA_TRAIN_IGNORE` — nếu vô tình bật, log dễ gây hiểu lầm "đã train xong" trong khi thực chất bị skip.

## 4. Write Leveling fail (tài liệu 06)
- `"Failed waiting for Write Leveling update mask"` → hardware không xác nhận chọn CS — nghi tầng thấp hơn (pad cal) chưa ổn định.
- `"WL on CH %d CS %d failed. Mask: 0x%x"` → đọc bit nào KHÔNG lên 1 trong mask để biết đúng pup nào có vấn đề.
- ⚠️ Nhớ: nửa "CLK-shift" của thuật toán bị comment-out trong bản hiện tại — nếu nghi ngờ vấn đề CLK (không phải DQS), delay CLK tại đây là giá trị CA training để lại, KHÔNG được Write Leveling tối ưu riêng.

## 5. DQS Gate fail (tài liệu 07, 22)
- `"!!! DQS Gate Training Error PHYDelay[ch][cs][pup] = 0"` → 1 CS không tìm được Window — nghi board/chip lỗi vật lý hoặc bước trước (CA/WL) đã sai.
- `"DQS gate training failed for CH %d"` → không pup nào hợp lệ cho cả channel — lỗi nghiêm trọng hơn.
- Nếu nghi chọn nhầm Window (đã xác nhận là bug B.3 thật) → kiểm tra `DQS_GATE_PREAMBLE_EN` đang bật hay tắt; bật `KM_DQS_GATE_SHMOO` để tự soi toàn bộ bản đồ pass/fail.

## 6. Read Centering/Deskew fail (tài liệu 08, 09)
- `left==right` hoặc left>right trong log → không có Window nào — nghi tầng thấp hơn (CA/WL/DQS Gate) sai trước.
- Window quá hẹp (1-2 giá trị) → nghi signal integrity/board, không phải lỗi thuật toán.
- Nếu 1 bit DQ/DM cụ thể luôn FAIL toàn dải — bug đó sẽ lộ ra sớm nhất ở `read_find_deskew_start`/`read_dm_deskew` (2 hàm đã đọc đầy đủ).

## 7. Write Centering/Deskew fail (tài liệu 10, 11)
- Nếu lỗi CHỈ xuất hiện sau warm boot resume, không ở cold boot — nhớ cả 2 lần Write Centering bị SKIP hoàn toàn khi warmboot (`skip=1`) — nghi vấn đề nằm ở training result restore (tài liệu 16), không phải chính thuật toán.
- `deskewStart` luôn = 63 (sentinel) → Write Centering trước đó có thể đã chọn 1 điểm quá an toàn/lệch xa Window thật.

## 8. Memory Test fail (tài liệu 12)
- Lỗi CHỈ xuất hiện sau `common_memfill()` (giai đoạn DMA), không ở FIFO trước đó → nghi tác động của việc bật DBI (chỉ bật từ điểm này).
- Máy reboot lặp lại (không dừng báo lỗi rõ) → xem `p_sram[0]/[1]/[2]` — nếu tăng dần tới 6 lần, đây là TẦNG NGOÀI (tài liệu 17) đang retry.

## 9. Retry/Error tổng thể (tài liệu 17)
- Đọc `p_sram[6]`/`[7]`/`[8]` biết ngay: đang ở lần retry thứ mấy (tầng trong), giới hạn là bao nhiêu (đã đọc QSPI đúng chưa), và fail ở lần nào.
- Nếu tầng trong đã dùng hết `retry_num` mà vẫn fail → vấn đề training THẬT, không phải lỗi lưu trữ.
- Nếu tầng ngoài retry (```p_sram[0]=1```, `[1]` tăng) mà tầng trong (`[6..8]`) cho thấy training tự nó PASS → nghi vấn đề tính NHẤT QUÁN lưu kết quả (`pmem`/`pmem2`), KHÔNG phải lỗi thuật toán training.

## 10. Sleep/Resume fail (tài liệu 15, 22 — đã xác nhận 2 bug thật)
- Log `"CH0 timeout!!"`/`"CH1 timeout!!"` khi resume → **ĐÂY LÀ BUG B.5 ĐÃ BIẾT** — root cause thật ở driver LCDC (`0xc057e190` bit `CFG_GRA_ENA`), KHÔNG phải sửa trong code DDR. Kiểm tra LCDC Suspend có tắt DMA đúng không TRƯỚC KHI nghi ngờ code DDR.
- Dữ liệu sai/mất sau khi resume, đặc biệt với chip 1-CS → **ĐÂY LÀ HƯỚNG CỦA BUG B.4** — kiểm tra `ch0_cs1_isValid`/`ch1_cs1_isValid` và `mask` tính trong `mc_enter_sr`/`mc_enter_pd` có đúng với board thật (số CS thật) không.
- Resume chạy chậm (không đạt ~150ms) → kiểm tra `BOOT_SYNC_DDR_WARMBOOT` có được LPPP set đúng trước khi ATF chạy không.

## 11. Nghi ngờ theo board/chip cụ thể (tài liệu 22)
- **Samsung + Write DQ lỗi, Eye 2 vùng** → bug B.2.1 đã biết, đã vá bằng "Region1/Region2 + wrap-around" — nếu vẫn thấy hiện tượng này, nghi ngờ 1 case wrap-around MỚI mà code hiện tại chưa xử lý.
- **Chip 1-CS (Micron 1GB/2GB) + lỗi lan sang byte-lane khác** → bug B.2.2 đã biết — kiểm tra TẤT CẢ nơi dùng `num_cs`, không chỉ 1 hàm.
- **Samsung 8GB + lỗi Write DQ ngẫu nhiên ~1/25** → hướng bug B.2.3 (CKE state lúc đổi tần số) — kiểm tra thứ tự Enter/Exit Self-Refresh quanh `change_clk_freq()` và thứ tự exit Write-Leveling-mode.
- **LPDDR4-2400 cụ thể (không phải 2100) lỗi Read** → hướng bug B.1 — kiểm tra `phy_rfifo_rptr_dly`/`PHY_Control_1` đúng giá trị theo tần số.

# Nguyên tắc chung khi debug (nhắc lại)

1. Luôn xác định TẦNG nào đang fail trước (tài liệu 17: tầng trong retry_num hay tầng ngoài 6-lần) — quyết định hướng điều tra hoàn toàn khác nhau.
2. Không kết luận root cause chỉ từ VỊ TRÍ log xuất hiện — bug B.5 là ví dụ trực tiếp: log xuất phát từ code DDR nhưng root cause ở LCDC.
3. Khi nghi ngờ 1 hàm training cụ thể, luôn kiểm tra bước NGAY TRƯỚC nó trước — hầu hết lỗi ở tầng cao (Deskew, Centering) là hệ quả của lỗi tầng thấp (Pad Cal, CA training) chưa được phát hiện.
4. Tra cứu nhanh register/struct qua `18_REGISTER_MAP.md`; tra cứu bug đã biết qua `22_BUG_ANALYSIS.md` TRƯỚC KHI tự suy luận root cause mới.

**Tiếp theo:** `23_FINAL_MASTER_MAP.md` — sơ đồ tổng thể cuối cùng.
