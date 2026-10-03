# 10 — Write Centering: lpddr4_write_centering(), 2 lần, bao quanh Write Deskew

Đọc `00`-`09` trước, đặc biệt `03` (công thức center) và `08` (`calculate_com()`, Center of Mass) — Write Centering tái dùng gần như nguyên bộ máy đó, chỉ đổi hướng.

# Concept

Write Centering làm cùng việc như Read Centering (tài liệu 08) nhưng theo hướng `Controller → DRAM`: tìm điểm delay+Vref tốt nhất để **DRAM** (không phải Controller) lấy mẫu đúng dữ liệu Controller gửi tới khi ghi (Write).

# Why

📘[GENERAL KNOWLEDGE] Khi ghi, chính DRAM là bên "đọc" tín hiệu — nó có 1 bộ receiver + Vref riêng, khác với receiver của Controller dùng khi đọc. Vì vậy Write Centering **không thể tái dùng nguyên kết quả của Read Centering** — phải đo lại từ góc nhìn của DRAM (thông qua phản hồi PASS/FAIL khi Controller tự đọc lại dữ liệu đã ghi, hoặc qua DMA thật).

# Hardware view + Firmware view

## Cấu trúc 2-pass, giống Read Centering, bao quanh Write Deskew (tài liệu 11)

[SOURCE FACT — `mv_lpddr4_apn806.c:8403-8438`]:

| Call | Mốc | Ý nghĩa |
|---|---|---|
| `lpddr4_write_centering(mc_reg, FIFO_TEST, write_fifo_vref_fix_en, warmboot, train_reg, skip=1)` | `TRN_IX` | Write center **lần 1**, trước deskew |
| *(giữa 2 lần là Write Deskew — tài liệu 11)* | `TRN_J0`-`TRN_J2` | |
| `lpddr4_write_centering(mc_reg, FIFO_TEST, write_fifo_vref_fix_en, warmboot, train_reg, skip=1)` | `TRN_KX` | Write center **lần 2**, sau deskew |

Đúng cấu trúc "center → deskew từng bit → center lại" như Read (tài liệu 08-09), chỉ khác: Write chỉ có **1 hàm centering dùng chung** (không có bản `_dma` riêng như Read có `TRN_L1`/`TRN_L2`) — ❓[UNKNOWN, chưa xác nhận]: chưa thấy call site DMA-based cho write centering trong các dòng đã đọc; có thể tồn tại ở vùng code khác của file (13778 dòng, chưa đọc hết) — để dành tài liệu 19 nếu cần xác nhận có/không.

## Vòng lặp chính — tái dùng khung "center of mass" (dòng 4550 trở đi, đã đọc phần khai báo ở tài liệu 08)

```c
if (warmboot == 0 || skip == 0)   // ← QUAN TRỌNG: cả 2 lần gọi đều truyền skip=1
{                                   //   → khi warmboot=1, TOÀN BỘ sweep bị bỏ qua ở CẢ 2 LẦN
    for (ch...) {
        display_measure_time(km_measure_time_flag, TRN_I0);   // (và TRN_I1 cho ch1 — cùng cơ chế)
        for (cs...) for (pup...) {
            if (bvRef_fixed) {                       // KM_TRAINING_REFINE — bỏ sweep Vref, dùng thẳng 1 giá trị
                j = vref_fixed_num_write;             // = 39 (KM_LPDDR4_config.h)
                minVREF = vref_fixed_num_write;
            }
            // ... sweep dly (và vref nếu không fixed) ...
        }
    }
}
```
[SOURCE FACT — `:4676`]: `if (warmboot==0 || skip==0)` — vì cả 2 call site đều truyền `skip=1`, **khi warmboot=1, sweep của CẢ HAI lần Write Centering đều bị bỏ qua hoàn toàn** — không chỉ 1 lần. Đây là bằng chứng thứ 2 (sau tài liệu 07, DQS Gate) cho việc warm boot bỏ qua sweep và dùng lại giá trị đã lưu — sẽ tổng hợp đầy đủ ở tài liệu 15.

`bvRef_fixed` (bật qua `KM_TRAINING_REFINE`, cùng nhóm tối ưu thời gian boot đã thấy ở tài liệu 02 — bỏ `delay_us(1)` trong PRFA) — khi bật, **không sweep Vref**, dùng thẳng giá trị cố định `vref_fixed_num_write=39` — giống hệt cơ chế `vref_fixed_num_read=23` ở Read Centering (tài liệu 08). 🧠[INFERENCE]: đây là 1 trade-off rõ ràng: đổi độ chính xác (có thể bỏ lỡ Vref tối ưu thật) để lấy tốc độ (bớt hẳn 1 trục quét) — hợp lý nếu qua nhiều lần đo thực tế, Vref tối ưu hầu như luôn rơi gần giá trị cố định này.

## `lpddr4_write_com()` — hàm liên quan, chưa xác nhận quan hệ trực tiếp

[SOURCE FACT — `KM_LPDDR4_prot.h:59`, khai báo `lpddr4_write_com(MC6_REGS_t*, int8_t test_dma, bool bvRef_fixed, bool warmboot, TRN_RSLT_REGS_t*, bool skip)` — cùng chữ ký tham số gần giống `lpddr4_write_centering`]. ❓[UNKNOWN]: chưa xác nhận `lpddr4_write_centering()` có gọi `lpddr4_write_com()` bên trong hay đây là 2 hàm độc lập làm việc song song/thay thế nhau (giống cặp `dqs_gate_training`/`dqs_gate_training_preamble` ở tài liệu 07) — cần đọc thân hàm đầy đủ ở tài liệu 19 để xác nhận, không suy đoán thêm ở đây.

## Ngay sau lần centering thứ 2 — backup kết quả training

[SOURCE FACT — `:8440-8442`]:
```c
display_measure_time(km_measure_time_flag, TRN_KX);
#ifdef KM_TRAIN_BACKUP_RESTORE
backup_train_area();   /* トレーニング前に、トレーニングで使用するエリアをバックアップ */
#endif
```
Comment gốc dịch: "trước khi training, backup vùng nhớ sẽ dùng cho training" — 🧠[INFERENCE tên gọi hơi ngược với vị trí]: dù comment nói "trước khi training", vị trí gọi thực tế là **NGAY SAU** khi Write Centering (bước gần cuối chuỗi training) hoàn tất — có thể comment mô tả ý nghĩa chung của hàm (dùng để backup trước khi 1 giai đoạn ghi/test tiếp theo diễn ra, ví dụ `common_memfill` ở tài liệu 12), không phải mô tả đúng vị trí gọi này. Đây là điểm nối trực tiếp tới tài liệu 12 (Memory Test) và 16 (Training Result Storage) — sẽ trace kỹ ở đó.

# Register view

Tái sử dụng cùng cơ chế PRFA/`nova_ddrphy_write` (tài liệu 02) và cùng field `VREF_TRAINING_VALUE_DQ`/`RANGE_DQ` trên `DRAM_Config_4` (tài liệu 03, ví dụ 2) — không có register mới đặc thù cho Write Centering ngoài những gì đã học.

# Source view

- `mv_lpddr4_apn806.c:4550-4682+` — `lpddr4_write_centering()`, đã đọc phần khai báo + điều kiện skip/warmboot/bvRef_fixed
- `mv_lpddr4_apn806.c:11505-11510` — 3 chữ ký `lpddr4_write_com()` (biến thể theo `#ifdef`) — chưa đọc thân hàm
- `mv_lpddr4_apn806.c:8403-8442` — call site thật, mốc `TRN_IX`/`TRN_KX`, và `backup_train_area()`

# Call flow

```
(sau Read Centering lần 2 — TRN_H5, tài liệu 08)
        |
        v
lpddr4_write_centering(FIFO, vref_fix_en, skip=1)   → TRN_I0/I1 (mỗi channel) → TRN_IX
        |
        v
   (Write Deskew — tài liệu 11: find_deskew_start → dm_deskew → deskew)
        |                                             TRN_J0        →(J1)      TRN_J2
        v
lpddr4_write_centering(FIFO, vref_fix_en, skip=1)   → TRN_KX
        |
        v
backup_train_area()
```

# K-S800 customization

- `bvRef_fixed`/`vref_fixed_num_write` — tối ưu thời gian boot, cùng nhóm `KM_TRAINING_REFINE` đã gặp ở tài liệu 02/08.
- `skip` parameter — cơ chế KM thêm để cho phép warm boot bỏ qua sweep mà không cần đổi cấu trúc lời gọi hàm ở caller (chỉ đổi giá trị tham số truyền vào).

# Debugging

- Nếu nghi ngờ Write Centering chọn sai Vref khi `bvRef_fixed=1` (giá trị cố định 39 không hợp với 1 lô chip cụ thể) — đây là rủi ro thật của tối ưu tốc độ: kiểm tra bằng cách tạm set `bvRef_fixed=0` (nếu có cờ QSPI tương ứng) để xem Vref tối ưu thật đo được có lệch nhiều so với 39 không.
- Nếu lỗi chỉ xuất hiện sau warm boot resume (không xuất hiện ở cold boot) — nhớ lại rằng cả 2 lần Write Centering **bị skip hoàn toàn** khi warmboot — vấn đề nhiều khả năng nằm ở dữ liệu training result được restore (tài liệu 16), không nằm ở chính thuật toán centering.

# Summary

- Write Centering dùng lại nguyên khung "center of mass" (tài liệu 08) và công thức center (tài liệu 03), chỉ đổi hướng đo.
- Chạy 2 lần, bao quanh Write Deskew (tài liệu 11) — giống cấu trúc Read.
- Cả 2 lần đều bị bỏ qua hoàn toàn khi warm boot (`skip=1` cả 2 nơi) — bằng chứng thứ 2 cho cơ chế "không train lại khi resume".
- Có chế độ Vref cố định (39) để tăng tốc, cùng nhóm tối ưu với các phát hiện ở tài liệu 02/08.
- `lpddr4_write_com()` tồn tại với chữ ký tương tự nhưng quan hệ với `lpddr4_write_centering()` chưa được xác nhận trực tiếp — ❓UNKNOWN, để dành tài liệu 19.

# Verify yourself

1. Vì sao Write Centering không thể tái dùng thẳng kết quả Vref đã đo ở Read Centering?
2. Điều kiện `if (warmboot==0 || skip==0)` — nếu `skip=0` được truyền (giả sử ai đó đổi lại), sweep có chạy dù đang warmboot không? Đọc lại biểu thức logic cẩn thận.
3. Đánh đổi giữa "Vref cố định" và "Vref tự tìm" là gì? Khi nào bạn s�ẽ muốn tắt Vref cố định để debug?
4. `backup_train_area()` được gọi ngay sau Write Centering lần 2 — theo bạn, tại sao vị trí này (chưa phải cuối training) lại hợp lý để backup, chứ không phải chờ đến hoàn toàn cuối cùng?
5. Danh sách những gì tài liệu này gắn cờ ❓UNKNOWN — bạn nghĩ cái nào quan trọng nhất cần xác minh trước khi tin tưởng hoàn toàn vào tài liệu này?

**Tiếp theo:** `11_WRITE_DESKEW.md` — 3 hàm thật (`write_find_deskew_start`, `write_dm_deskew`, `write_deskew`) — ít bước hơn Read Deskew (không có "find_deskew_end"/"center_align" riêng), sẽ giải thích vì sao.
