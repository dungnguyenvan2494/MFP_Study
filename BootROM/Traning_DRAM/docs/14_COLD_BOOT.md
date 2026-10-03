# 14 — Cold Boot: Entry Point thật, từ BootROM tới hoàn tất training

Đọc `00`-`13` trước. Đây là tài liệu **tổng hợp** — ghép toàn bộ chuỗi TRN_* đã trace ở các tài liệu 04-13 vào 1 timeline thật, dựa trên entry point thật (không phải sơ đồ giả định ban đầu).

# Concept

Cold Boot = lần khởi động đầu tiên, DRAM hoàn toàn "trắng" (chưa có tham số nào), phải chạy **toàn bộ** chuỗi training từ đầu. Đối lập với Warm Boot (tài liệu 15) — nơi phần lớn bước bị bỏ qua.

# Entry chain thật — trace theo Phần 6 (không suy đoán path)

[SOURCE FACT — `atf\atf_s800\ble\ble_main.c:52-92`, đã đọc toàn bộ]:

```c
int ble_main(int bootrom_flags)      // ← ENTRY POINT thật, gọi bởi BootROM
{
    // 1. Xoá SRAM (2048 byte tại 0xC0200000) bằng sentinel 0xE5 — "dọn sạch" vùng SRAM
    //    dùng để lưu retry-count/flag (liên hệ bug SuperWarp, tài liệu 12)
    for (i=0; i<2048; i++) p_sram[i] = 0xE5;

    get_it();                         // ❓UNKNOWN — chưa trace, khả năng init hardware sớm
    console_init(...);
    plat_delay_timer_init();          // BẮT BUỘC trước mọi delay_us() dùng suốt training

    lpddr4_config();                  // ← gọi vào toàn bộ logic LPDDR4 training
    ...
}
```

```c
int lpddr4_config(void)               // atf/atf_s800/ble/mv_lpddr4_apn806.c:8751
{
    warmboot = syncronize_boot_get(BOOT_SYNC_DDR_WARMBOOT(1), false);   // hỏi LPPP: cold hay warm?
    ...
    // đọc GPIO, swizzle_setting(), QSPI checksum + dự phòng   → tài liệu 13
    // xác định ddr_top_freq, revA (A0 hay B0+)
    // start Timer3 (đo thời gian) → display_measure_time(TRN_A0)

#if 1 /* 2024/11/27 OP_BTS-51229 */
    if (warmboot == 0) {              // COLD BOOT
        for (i=0; i<6; i++) {         // ← RETRY TẦNG NGOÀI, tối đa 6 lần, KHÁC retry_num=10 (tài liệu 17!)
            lpddr4_training_Error = 0;
            lpddr4_dynamic_config(config_type, warmboot);
            if (lpddr4_training_Error == 0) break;
            p_sram[0] = 1; p_sram[1] = i+1;   // ghi nhận lỗi + số lần thử vào SRAM
        }
        if (lpddr4_training_Error == 0) p_sram[2] = 0x77;   // báo OK
    } else {                          // WARM BOOT — gọi 1 lần, không có vòng lặp 6 lần
        lpddr4_dynamic_config(config_type, warmboot);
    }
#endif

    display_measure_time(km_measure_time_flag, TRN_N0);
    //read_ddr_osc();                 // ← BỊ COMMENT OUT — xem "K-S800 customization"
    display_measure_time(km_measure_time_flag, TRN_N1);

    syncronize_boot_set(BOOT_SYNC_DDR_READY(1));    // báo cho bên khác: DDR đã sẵn sàng
    return MV_OK;
}
```

⚠️ **Phát hiện quan trọng — có 2 tầng retry KHÁC NHAU, không phải 1** (sửa lại giả định đơn giản ở các tài liệu trước):

| Tầng | Biến điều khiển | Giới hạn | Vị trí | Vai trò |
|---|---|---|---|---|
| **Ngoài** (mới thấy ở đây) | `lpddr4_training_Error` | **cố định 6 lần**, chỉ Cold Boot | `lpddr4_config()` | Bọc quanh TOÀN BỘ `lpddr4_dynamic_config()` — nếu cả 1 lần gọi hàm này thất bại kiểu nghiêm trọng, gọi lại từ đầu tối đa 6 lần |
| **Trong** (đã thấy ở tài liệu 12/17) | `train_err_flg`/`train_err_cnt` | **10 lần mặc định, tối đa 255 qua QSPI** (`retry_num`) | Bên trong `lpddr4_dynamic_config()` | Retry riêng cho từng bước training cụ thể bị FAIL (CA training, Write Leveling, DQS Gate...) — dùng `return_100mhz()` để quay lại, không thoát hẳn hàm |

❓[UNKNOWN — quan hệ chính xác giữa 2 tầng chưa xác nhận đầy đủ]: chưa đọc rõ điều kiện chính xác khiến `lpddr4_training_Error` được set thành khác 0 (khác với `train_err_flg`) — cần đọc kỹ hơn ở tài liệu 17 (Error/Retry) để phân biệt "lỗi cấp `lpddr4_dynamic_config` toàn bộ" vs "lỗi cấp 1 bước training cụ thể".

# Timeline thật, TRN_A0 → TRN_N1 (toàn bộ 36 mốc, đối chiếu SPEC FACT)

[SOURCE FACT — tổng hợp từ `KM_LPDDR4_config.h:199-234` (tài liệu 00) + toàn bộ call site đã trace ở tài liệu 04-13]:

```
TRN_A0  Bắt đầu Timer3 đo thời gian (lpddr4_config, TRƯỚC vòng lặp retry-6-lần)
TRN_A1  Nhận diện dung lượng xong (GPIO/QSPI — tài liệu 13)
TRN_A2  Đổi tần số 25MHz → 100MHz
TRN_B0  Bắt đầu Pad Calibration (S800)                          → tài liệu 04
TRN_B1  Pad Calibration (S800) xong
TRN_B2  Pad Calibration (DRAM) xong — MCConfig_..._dpi()
TRN_C0  Bắt đầu CA Training                                     → tài liệu 05
TRN_C1  Đổi 100MHz→1050/1200MHz xong (vào CBT)
TRN_C2  CA training "load" xong
TRN_C3  Đổi 1050/1200MHz→100MHz xong (ra CBT)
TRN_C4  Đổi 100MHz→1050/1200MHz xong (lần 2, chuẩn bị bước sau)
TRN_C5  CA training xong hoàn toàn
TRN_D0  Write Leveling xong                                     → tài liệu 06
TRN_D1  applyWlDlys() xong
TRN_E0  DQS Gate Training xong                                  → tài liệu 07
TRN_F0  Read Pre-Amble/Add PBS xong
TRN_GX  Read Center (lần 1, FIFO) xong                          → tài liệu 08
TRN_H0  Read find_deskew_start xong                             → tài liệu 09
TRN_H1  Read dm_deskew xong
TRN_H2  Read deskew xong
TRN_H3  Read find_deskew_end xong
TRN_H4  Read deskew_center_align xong
TRN_H5  Read Center (lần 2, FIFO, sau deskew) xong               → tài liệu 08
TRN_I0/I1  Write com bắt đầu (Ch0/Ch1)                            → tài liệu 10
TRN_IX  Write Center (lần 1, FIFO) xong
TRN_J0  Write find_deskew_start xong                             → tài liệu 11
TRN_J1  Write dm_deskew xong
TRN_J2  Write deskew xong
TRN_KX  Write Center (lần 2, FIFO) xong → backup_train_area()    → tài liệu 10/12
TRN_L0  common_memfill xong (bật DBI, chuyển mode vận hành thật) → tài liệu 12
TRN_L1  Read Center (DMA, PBS) xong
TRN_L2  Read Center (DMA, no-PBS) xong
TRN_M0  Write Center (DMA) xong
TRN_N0  lpddr4_dynamic_config() trả về (retry-6-lần đã xong)     → lpddr4_config()
TRN_N1  read_ddr_osc() — ❓ hàm này BỊ COMMENT OUT, mốc này thực chất không đo gì cả
```

**Đối chiếu SPEC FACT** (tài liệu 00, từ `1_0...pdf`): spec đo "LPDDR4トレーニング開始 ～ LPDDR4トレーニング終了" ≈ 650ms. 🧠[INFERENCE]: khoảng đo này rất có khả năng chính là **TRN_A0 → TRN_N0** (hoặc N1) qua chính cơ chế Timer3 vừa thấy — nhưng ❓[UNKNOWN] chưa đọc trực tiếp code tính/hiển thị hiệu số thời gian giữa 2 mốc để xác nhận 100% đây là con số 650ms được đo bằng đúng cơ chế này (cần đọc `display_measure_time()` đầy đủ + đối chiếu `AP_A_3_Cold_Bootでのトレーニング時間の測定方法` — chưa đọc file này).

# K-S800 customization

- Xoá SRAM bằng `0xE5` ngay đầu `ble_main()` — 🧠[INFERENCE] giá trị "poison" cố ý (không phải 0x00) để dễ nhận ra trong debug dump "vùng này đã từng được ble_main khởi tạo" hay chưa.
- Retry tầng ngoài (6 lần, `lpddr4_training_Error`) là bổ sung của KM (nằm trong khối `#if 1 /* 2024/11/27 OP_BTS-51229 */`) — cùng bug SuperWarp đã thấy ở tài liệu 12, xác nhận thêm: bug này sửa **cả 2 nơi cùng lúc** (đánh dấu SRAM ở `lpddr4_config()` VÀ ở nhánh retry-nội-bộ tài liệu 12).
- `read_ddr_osc()` bị comment out ở TRN_N1, và `mv_ddr_validate()` (dòng ngay trước, "this does eye monitor, but slow..") cũng bị comment out — 🧠[INFERENCE] 2 tính năng chẩn đoán/validate bổ sung của Marvell bị KM tắt hẳn để tiết kiệm thời gian boot — đánh đổi giữa tốc độ và khả năng tự-chẩn-đoán lúc chạy thật.
- `BOOT_SYNC_DDR_*` — cơ chế đồng bộ giữa ATF và 1 thành phần khác (❓UNKNOWN chính xác là LPPP hay 1 core khác) để biết DDR đã sẵn sàng.

# Debugging

- Nếu board không boot được và nghi ngờ LPDDR4 training — điểm đặt breakpoint/log đầu tiên nên là `ble_main()` (xác nhận có vào được tới đây không — nếu không, vấn đề nằm ở BootROM/hardware, không phải LPDDR4 training) rồi `lpddr4_config()` (xác nhận `warmboot` được xác định đúng chưa — nếu sai cold/warm, toàn bộ logic sau đều sai hướng).
- Nếu thấy máy retry đúng 6 lần rồi vẫn tiếp tục treo/boot lỗi khác — đây là dấu hiệu đã **vượt quá tầng retry ngoài** (6 lần cố định, không đọc từ QSPI) — khác hẳn triệu chứng "retry tới 10 hoặc 255 lần rồi báo Error Flag" (tài liệu 17) — 2 loại triệu chứng cần phân biệt khi đọc log.
- Đọc `p_sram[0..2]` (tại `0xC0200000`) sau khi treo máy (nếu có thể dump SRAM) — biết ngay được: có lỗi memory-access nghiêm trọng không (`[0]=1`), thử lại bao nhiêu lần (`[1]`), có thành công cuối cùng không (`[2]=0x77`).

# Summary

- Entry point thật: `ble_main()` → `lpddr4_config()` → (cold: loop 6 lần) `lpddr4_dynamic_config()` (toàn bộ training tài liệu 04-13).
- Có **2 tầng retry riêng biệt** — tầng ngoài (6 lần, cold-boot-only, lỗi nghiêm trọng) và tầng trong (10-255 lần, lỗi từng bước cụ thể) — không được nhầm lẫn.
- Toàn bộ 36 mốc TRN_A0→N1 đã có vị trí thật, đối chiếu đầy đủ với tài liệu 04-13.
- TRN_N1 và `mv_ddr_validate()` — 2 tính năng chẩn đoán gốc bị tắt để tiết kiệm thời gian, không phải bug.
- SRAM tại `0xC0200000` là "bảng điều khiển" quan trọng để debug boot failure — nên là nơi đầu tiên kiểm tra khi board treo lúc training.

# Verify yourself

1. Nếu `warmboot` bị xác định sai (LPPP báo warmboot nhưng thực ra là cold boot), điều gì tệ nhất có thể xảy ra khi bước vào `lpddr4_dynamic_config()`?
2. Vì sao retry tầng ngoài (6 lần) chỉ áp dụng cho Cold Boot, không áp dụng cho Warm Boot (đọc lại đúng đoạn code `if/else`)?
3. `p_sram` bị fill bằng `0xE5` ở đầu `ble_main()`, nhưng lại được ghi `1`/`i+1`/`0x77` ở nơi khác trong `lpddr4_config()` — nếu bạn dump SRAM và thấy vẫn còn `0xE5` ở byte đó, điều đó nói lên gì?
4. Tại sao việc tắt `mv_ddr_validate()`/`read_ddr_osc()` là 1 đánh đổi hợp lý cho sản phẩm thật (không phải máy dev/test)?
5. Từ bảng timeline đầy đủ ở trên, đếm xem có bao nhiêu lần chuyển đổi tần số clock (25→100, 100→cao, cao→100, 100→cao...) xảy ra trong TOÀN BỘ 1 lần cold boot? Liên hệ lại tài liệu 19 (Frequency Switching, chưa viết) về lý do cần nhiều lần đổi vậy.

**Tiếp theo:** `15_WARM_BOOT.md` — sẽ tổng hợp mọi bằng chứng "bị bỏ qua khi warmboot" đã gặp rải rác (tài liệu 07/08/09/10/11/12) thành 1 bức tranh đầy đủ, cùng `mv_lpddr4_sleep.c`.
