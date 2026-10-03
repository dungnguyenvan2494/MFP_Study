# 23 — S800 LPDDR4 Training: Final Master Map

Đọc `00`-`22` trước — đây là điểm tổng hợp cuối, hiệu chỉnh hoàn toàn theo source thật (không còn placeholder từ outline gốc).

# Sơ đồ tổng thể

```
                                   S800 (SoC nền Marvell AP806/Armada)
                                              |
                +-----------------------------+------------------------------+
                |                             |                              |
              LPPP                          ATF (ble)                     U-Boot
    (mv_lpddr4_sleep.c)          (mv_lpddr4_apn806.c, 13778 dòng)   (qspi_winbond.c, ❓chưa đọc)
                |                             |                              |
      mc_configure_dram(warm)        ble_main() → lpddr4_config()      Đọc pmem (0x200000000)
      mc_save_state()  ─┐            → lpddr4_dynamic_config()        → ghi vào QSPI
      mc_restore_state()─┼─ BOOT_SYNC_DDR_WARMBOOT/CONT_BOOT/READY            |
      (Self-Refresh,     │                    |                              |
       Power-Down)        │          +---------+---------+                   |
                           │          |                   |                  |
                           │     Memory Controller       PHY                 |
                           │      (MC6_REGS_t,          (DDR_PHY_REGS_t,     |
                           │       0xf0020000)            0xf0011000)        |
                           │          |                   |                  |
                           │          +---------+---------+                  |
                           │                    |                            |
                           │              LPDDR4 DRAM (Micron/Samsung)       |
                           │                    |                            |
                           └────────────────────┴────────────────────────────┘
                                     (dữ liệu sống qua Sleep nhờ Self-Refresh)

TRAINING PIPELINE (bên trong lpddr4_dynamic_config(), tầng trong retry_num lần):

  Chip Detect (GPIO/QSPI, Swizzle)
        |
        v
  Pad Calibration  (S800 + DRAM)
        |
        v
  CA Training  (CBT, FSP0/1, Vref sweep)
        |
        v
  Write Leveling  (edge-finding, KHÔNG center — CLK-shift bị disable)
        |
        v
  DQS Gate Training  (Preamble hoặc Data-test, 2 giải pháp cho bug B.3)
        |
        v
  Read Centering (1) → Read Deskew (5 hàm) → Read Centering (2)
        |
        v
  Write Centering (1) → Write Deskew (3 hàm) → Write Centering (2)
        |
        v
  backup_train_area()  →  common_memfill() (bật DBI)  →  Read/Write Centering DMA (validation thật)
        |
        v
  [PASS] → restore_train_area() → lpddr4_backup_train_result() → pmem/pmem2 → (U-Boot → QSPI)
  [FAIL] → restore_train_area() → return_100mhz() → retry (tối đa retry_num, 10-255)
        |
        v
  [Warm Boot] check_warm_training() → nếu sai: goto retry (cùng vòng lặp trên)
```

# 10 điều bạn giờ nên tự trả lời được (đối chiếu Phần 40 outline gốc)

1. **LPDDR4 là gì?** — 1 chuẩn giao tiếp JEDEC, không phải chip; DRAM là chip tuân theo chuẩn đó (tài liệu 01).
2. **Tại sao cần training?** — Đường dây PCB + silicon mỗi board/chip khác nhau, không tính được bằng lý thuyết, phải đo lúc chạy thật (tài liệu 00).
3. **PHY và Memory Controller khác gì?** — 2 khối silicon thật, 2 struct C khác nhau, 2 địa chỉ MMIO khác nhau; nghịch lý: PHY có sequencer lệnh JEDEC riêng dùng khi training (tài liệu 02).
4. **Window/Eye/Center/Margin?** — PASS liên tục = Window; 2 trục = Eye; trung điểm = Center (công thức lặp lại nhiều nơi, nhưng có ít nhất 2 biến thể: bisection và center-of-mass); khoảng đệm = Margin (tài liệu 03).
5. **Vì sao cần cả GPIO và QSPI?** — GPIO tối thiểu, không mở rộng được; QSPI đủ chi tiết + có dự phòng đa vùng (bổ sung 2020) (tài liệu 13).
6. **Cold Boot vs Warm Boot khác gì?** — Cold: full training qua retry_num lần; Warm: skip sweep, đọc lại `train_reg`, nhưng vẫn có thể quay lại retry qua `check_warm_training()` (tài liệu 14, 15, 16).
7. **Dữ liệu DRAM có mất khi Sleep không?** — Không, nhờ Self-Refresh — độc lập với việc "không cần train lại" (tài liệu 15).
8. **Retry hoạt động thế nào?** — 2 tầng độc lập: trong (retry_num, lỗi training) và ngoài (6 lần, lỗi tính-nhất-quán-lưu-kết-quả) (tài liệu 17).
9. **Training PASS có nghĩa RAM chắc chắn chạy đúng không?** — Không hẳn — cần thêm Memory Test dưới cấu hình vận hành thật (DBI on) (tài liệu 12).
10. **Board không boot vì DDR thì debug từ đâu?** — Theo đúng thứ tự pipeline ở trên, xem tài liệu 21; và tra bug đã biết ở tài liệu 22 TRƯỚC khi tự suy luận root cause mới.

# Những gì tài liệu này KHÔNG dám khẳng định (trung thực tới cuối)

- Chỉ đọc được **4/27 file PDF spec gốc** trực tiếp cho các chương chính (0_1, 1_0, 3_0, 5_1) + toàn bộ Appendix B (7 bug) — các chương 2, 4.x (thuật toán chi tiết dạng spec), 6.0 (tần số), 7 (mode), 8.x (cold/warm boot theo spec), 9.0 (swizzle spec), 10.0 (error spec), Appendix A (đo đạc), Appendix C (register bổ sung) **chưa được đọc trực tiếp** — mọi mô tả về các chương này trong bộ tài liệu là suy luận từ source + đối chiếu gián tiếp qua tiêu đề mục lục.
- `lpddr4_read_deskew()`, `lpddr4_read_find_deskew_end()`, `lpddr4_write_deskew()` — chỉ đọc header, chưa đọc hết thân hàm.
- `lpddr4_write_com()` — quan hệ với `lpddr4_write_centering()` chưa xác nhận.
- `qspi_winbond.c` (U-Boot) — hoàn toàn chưa đọc.
- Caller thật của `mc_configure_dram(warm)` (ai quyết định cold/warm ở LPPP) — chưa trace.
- B.1 (giá trị bit `phy_rfifo_rptr_dly`) và B.2.3 (thứ tự exit Write-Leveling) — match ở mức cấu trúc, chưa decode/xác nhận chi tiết cuối.

# Cách tiếp tục (cho session sau, nếu cần đào sâu hơn)

Theo đúng thứ tự ưu tiên (giá trị thông tin / công sức):
1. Đọc `qspi_winbond.c` — khép kín hoàn toàn data flow training result.
2. Đọc thân hàm 3 hàm còn ❓ ở Read/Write Deskew — hoàn thiện tài liệu 09/11.
3. Đọc spec chương 4.x (thuật toán) — đối chiếu sâu hơn với source, có thể tìm thêm MISMATCH như đã tìm ở QSPI dự phòng.
4. Đọc spec 8.1/8.2 (Cold/Warm Boot chi tiết theo spec) — đối chiếu với tài liệu 14/15 đã dựng từ source.
5. Đọc Appendix A (đo đạc margin/thời gian) — trả lời câu hỏi còn treo ở tài liệu 00/03 về nhiệt độ.

---

*Hết bộ tài liệu S800 LPDDR4 Training — 24 file (00-23), dựng từ source thật (13778 dòng `mv_lpddr4_apn806.c` + các file liên quan) và 11/27 file spec PDF đã đọc trực tiếp (4 chương chính + toàn bộ Appendix B). Mọi claim đều gắn tag [SPEC FACT]/[SOURCE FACT]/[INFERENCE]/❓[UNKNOWN] theo đúng nguyên tắc đã thống nhất từ tài liệu 00.*
