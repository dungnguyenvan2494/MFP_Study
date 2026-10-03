# 12 — Memory Test: common_memfill(), backup/restore_train_area()

Đọc `00`-`11` trước, đặc biệt `08` (FIFO vs DMA test) và `10` (đã thấy `backup_train_area()` gọi lần đầu).

# Concept

📘 **Memory Training ≠ Memory Validation**:
- **Training** (tài liệu 04-11) chỉ kiểm tra PASS/FAIL bằng **pattern giả lập ngắn, tại vài vùng nhớ scratch cố định** (`train_base_0..7`), với mục đích duy nhất là tìm delay/Vref tối ưu — không phải để "chứng minh RAM hoạt động đúng".
- **Validation** (Memory Test, `common_memfill()` + các bước Read/Write Centering chạy lại bằng DMA sau đó) kiểm tra bằng **dữ liệu thật, khối lượng lớn hơn, ở điều kiện vận hành thật** (DBI bật, DM tắt-training-mode) — gần với cách Linux thực sự dùng RAM.

Training PASS chỉ có nghĩa: "ở đúng delay/Vref này, vài KB dữ liệu test đọc/ghi đúng". Không có nghĩa "toàn bộ dải địa chỉ RAM, mọi pattern dữ liệu, đều chắc chắn đúng" — đó là lý do cần thêm 1 lớp test riêng.

# Why

📘[GENERAL KNOWLEDGE] Pattern dùng trong training (ví dụ `0x591C5555` thấy ở tài liệu 09) là pattern **cố định, được chọn để dễ lộ lỗi timing** (nhiều chuyển mức 0↔1) — không đại diện cho dữ liệu thực tế OS sẽ ghi. Ngoài ra training chạy với DM **tắt** (bypass, qua thủ thuật swap tài liệu 09) và DBI **tắt** — khi vận hành thật, DBI (Data Bus Inversion — JEDEC LPDDR4, tự động đảo bit nếu làm giảm số lần chuyển mức trên bus, giảm nhiễu/điện năng) sẽ **bật**, thay đổi hành vi điện của bus. Vì vậy cần 1 bước test riêng, đúng với cấu hình DBI/DM thật, trước khi tin tưởng hoàn toàn.

# Hardware view + Firmware view

## `common_memfill()` — chuyển từ "chế độ training" sang "chế độ vận hành thật" (dòng 7714-7790, đã đọc toàn bộ)

```c
setReadData();
memFill((uint32_t*)NONCACHE(m_capacity_info.train_base_0)+..., TRAIN_NOISE_SIZE);   // CPU ghi pattern noise, 1 lần
// Dùng DMA (mv_xor_4dma_memcpy_list) chép pattern đó ra các train_base_1..7 còn lại — vì memFill bằng CPU cho NON-cache region quá chậm
mv_xor_4dma_memcpy_list(src, dst1, size, 3, true, 1);
mv_xor_4dma_memcpy_list(src, dst2, size, ..., true, 1);

//Enable DBI
rdModWr(&mc_reg->CH0_DRAM_Config_4, READ_DBI_MASK|WRITE_DBI_MASK, ..., VERIFY);
rdModWr(&mc_reg->CH1_DRAM_Config_4, READ_DBI_MASK|WRITE_DBI_MASK, ..., VERIFY);

//Disable DM (thoát hẳn training-mode của DM, không phải thủ thuật swap tạm nữa)
rdModWr(&mc_reg->CH0_DRAM_Config_2, 0, DM_MASK, VERIFY);
send_mrx(1,13,3,3);   // MR13 → chế độ vận hành thật
send_mrx(1,3,3,3);    // MR3  → xác nhận cấu hình DBI trên chính DRAM
```
🧠[INFERENCE]: đây chính là **ranh giới thật giữa "chế độ training" và "chế độ vận hành"** — trước điểm này, mọi test đều chạy dưới các mode đặc biệt (CBT, read-leveling, DM-swap...); sau điểm này, DRAM được đưa về đúng cấu hình sẽ dùng khi Linux chạy thật (DBI on), và **các bước Read/Write Centering chạy LẦN NỮA sau đó (TRN_L1/L2/M0, tài liệu 08/10) chính là để re-verify center vẫn đúng dưới cấu hình DBI mới này** — giải thích rõ hơn lý do có "lần 2" mà tài liệu 08 đã ghi nhận nhưng chưa biết tại sao.

## `backup_train_area()` / `restore_train_area()` — bảo vệ dữ liệu, không chỉ cho Warm Boot

[SOURCE FACT — `:10032-10063`, đã đọc toàn bộ]: chỉ là 8 lệnh `backup_ddr()`/`restore_ddr()` lặp lại cho `train_base_0..7`, copy qua lại với buffer toàn cục `g_backup_train_base_*`.

**Câu hỏi quan trọng (đúng như user's outline đặt ra ở Phần 22)**: vì sao cần backup/restore ngay ở đây, không phải chỉ ở Warm Boot? Trả lời bằng bằng chứng thật — **có 2 lý do khác nhau, dùng chung 1 cơ chế**:

**Lý do 1 — An toàn cho RETRY** [SOURCE FACT — `:8618-8624`]:
```c
if (train_err_cnt != retry_num-1) {
    restore_train_area();     // ← khôi phục train_base TRƯỚC khi thử lại
    return_100mhz(mc_reg);
}
```
Khi training FAIL và sắp retry, firmware **phục hồi lại các vùng `train_base_*` về đúng trạng thái trước khi lần training này bắt đầu ghi pattern test/noise vào đó** — nếu không, lần retry tiếp theo có thể đọc nhầm dữ liệu noise còn sót của lần thất bại trước, làm sai lệch kết quả PASS/FAIL của chính công cụ test.

**Lý do 2 — Bảo vệ dữ liệu thật khi Warm Boot** (🧠[INFERENCE], sẽ xác nhận đầy đủ ở tài liệu 15): vì `train_base_*` là **các địa chỉ DRAM thật**, trùng với vùng nhớ Linux/OS đang dùng thật khi máy đang chạy — nếu các bước Memory Test này (ghi noise pattern, DMA test...) chạy trong lúc **resume từ Sleep** (dữ liệu Linux vẫn còn sống trong DRAM, xem tài liệu 00 SPEC FACT), backup/restore là bắt buộc để không phá dữ liệu thật của OS.

## Bug-fix thật liên quan tới retry (khác nhóm bug ở tài liệu 05/06)

[SOURCE FACT — comment tại `:8602`]: `2024/11/27 OP_BTS-51229 SuperWarp起動時にリブート繰り返す対策` (đại ý: "đối sách cho hiện tượng lặp reboot liên tục lúc khởi động SuperWarp"), gắn với dòng `p_sram[8] = (uint8_t)train_err_cnt;` — lưu số lần đã retry vào 1 vùng SRAM (`p_sram`) khi training fail. 🧠[INFERENCE]: "SuperWarp" 🧠 có khả năng là tên gọi nội bộ của 1 chế độ boot/resume đặc biệt (khác cold boot thông thường — ❓UNKNOWN chưa xác nhận định nghĩa chính thức, chưa thấy trong 27 file PDF đã đọc phần nào). Việc lưu `train_err_cnt` vào SRAM (không phải chỉ biến RAM thường) gợi ý: SRAM sống sót qua 1 kiểu reset mà RAM thường không sống sót — cho phép firmware "nhớ" đã retry bao nhiêu lần **giữa các lần reboot**, để tránh việc mỗi lần reboot lại tự tin retry lại từ đầu → vòng lặp reboot vô hạn nếu lỗi thật sự không thể sửa bằng retry. Đây là bug khác hẳn 3 bug ở tài liệu 05/06 (ticket khác: `OP_BTS-51229` vs `OP_BTS-xxxx`; ngày khác: `2024/11/27` vs `2025/03/07`) — ghi nhận riêng cho tài liệu 22.

# Register view

| Register/biến | Ý nghĩa |
|---|---|
| `CH0/1_DRAM_Config_4` bit `READ_DBI`/`WRITE_DBI` | Bật Data Bus Inversion cho chế độ vận hành thật |
| `CH0/1_DRAM_Config_2` bit `DM` | Bật/tắt DM ở mức "chế độ vận hành" (khác thủ thuật swap tạm ở tài liệu 09) |
| MR13/MR3 (qua `send_mrx`) | Chuyển DRAM về mode vận hành thật sau khi common_memfill xong |
| `train_base_0..7` | 8 vùng DRAM thật dùng làm scratch cho mọi test (FIFO/DMA) suốt quá trình training — trùng với vùng nhớ Linux sẽ dùng sau này |
| `g_backup_train_base_0..7` | Buffer (SRAM/vùng nhớ riêng ngoài `train_base`) lưu bản backup |
| `p_sram[8]` | Lưu `train_err_cnt` — sống sót qua reboot (liên quan bug SuperWarp) |

# Source view

- `mv_lpddr4_apn806.c:7714-7790` — `common_memfill()`, đã đọc toàn bộ
- `mv_lpddr4_apn806.c:10032-10063` — `backup_train_area()`/`restore_train_area()`, đã đọc toàn bộ
- `mv_lpddr4_apn806.c:9989-10023` — `restore_ddr()` (và cặp `backup_ddr()` — đã đọc `restore_ddr`, `backup_ddr` cấu trúc tương tự, ❓chưa đọc riêng thân `backup_ddr`)
- `mv_lpddr4_apn806.c:8524-8566` — nhánh chính (build có `USE_DMAS`?): `common_memfill()` → `TRN_L0` → Read/Write Centering DMA (`TRN_L1/L2/M0`)
- `mv_lpddr4_apn806.c:8570-8597` — nhánh `#else` thay thế (build khác) — cấu trúc tương tự nhưng thứ tự backup/memfill hơi khác — ❓UNKNOWN điều kiện build nào chọn nhánh nào, chưa xác nhận macro guard chính xác (chỉ thấy `#if 1 ... #else`)
- `mv_lpddr4_apn806.c:8599-8629` — logic retry, có gọi `restore_train_area()`, và bug-fix SuperWarp

# Call flow

```
(sau Write Centering lần 2 — TRN_KX, tài liệu 10)
        |
        v
backup_train_area()                    [backup train_base_0..7]
        |
        v
common_memfill()          → TRN_L0     [ghi noise pattern thật, bật DBI, tắt DM-training-mode]
        |
        v
Read Centering (DMA, ADD_PBS)  → TRN_L1
        |
        v
Read Centering (DMA, NO_PBS)   → TRN_L2
        |
        v
Write Centering (DMA)          → TRN_M0
        |
        v
   [nếu train_err_flg == 1 (fail đâu đó trong toàn chuỗi)]
        |
        +-- restore_train_area()  → return_100mhz()  → (quay lại từ đầu, tài liệu 17)
        |
        +-- [nếu PASS] → tiếp tục lưu training result (tài liệu 16)
```

# Diagram

```
train_base_0 ... train_base_7  (8 vùng DRAM thật, dùng suốt training)
        |
        v
  [backup_train_area]  →  g_backup_train_base_0..7 (nơi trú ẩn an toàn)
        |
        v
  common_memfill() ghi NOISE thật vào train_base_*  (khác pattern training cũ)
        |
        v
  Read/Write Centering DMA test lại (TRN_L1/L2/M0) — kiểm tra dưới cấu hình DBI THẬT
        |
        +-- FAIL → [restore_train_area] → train_base_* về nguyên trạng → retry
        |
        +-- PASS → tiếp tục (train_base_* vẫn còn noise pattern — ❓ai dọn lại vùng này
                    trước khi Linux dùng? Xem tài liệu 14/16 nếu có bước riêng)
```

# K-S800 customization

- Toàn bộ cơ chế backup/restore (`KM_TRAIN_BACKUP_RESTORE`) là bổ sung của KM.
- Bug-fix SuperWarp (`2024/11/27`, `OP_BTS-51229`) — lưu retry count vào SRAM để tránh reboot loop — là 1 bug thật, riêng biệt, xảy ra muộn hơn nhóm bug tháng 3/2025 ở tài liệu 05/06.

# Debugging

- Nếu nghi ngờ Memory Test (giai đoạn DMA, sau `common_memfill`) fail nhưng các bước training trước đó (FIFO) đều PASS — nghi ngờ trực tiếp tác động của DBI (chỉ bật từ điểm này) — kiểm tra xem lỗi có biến mất khi tạm tắt DBI không (nếu có cách test).
- Nếu thấy máy liên tục reboot ngay giai đoạn training (không dừng lại báo lỗi) — kiểm tra `p_sram[8]`/`train_err_cnt` c ó được đọc lại đúng sau reboot không — đây chính là cơ chế chống-vòng-lặp của bug SuperWarp; nếu cơ chế này hỏng, sẽ thấy đúng triệu chứng "reboot lặp vô hạn" mà bug gốc mô tả.
- ❓ Câu hỏi còn mở: sau khi Memory Test PASS, `train_base_*` vẫn chứa noise pattern (không phải dữ liệu Linux) — chưa xác nhận có bước "dọn dẹp" (zero-fill hoặc để nguyên vì Linux sẽ tự ghi đè khi cấp phát) trước khi bàn giao RAM cho U-Boot/Linux. Để dành tài liệu 14 (Cold Boot) hoặc 30 nếu cần.

# Summary

- Training PASS ≠ RAM chắc chắn hoạt động — cần Memory Test (`common_memfill` + Read/Write Centering DMA lại) dưới cấu hình vận hành thật (DBI on).
- `common_memfill()` là ranh giới rõ giữa "chế độ training" và "chế độ vận hành" — giải thích tại sao Read/Write Centering phải chạy lần 2 bằng DMA.
- Backup/restore `train_base_*` phục vụ **2 mục đích khác nhau**: an toàn cho retry (đã xác nhận trực tiếp trong code) VÀ bảo vệ dữ liệu Warm Boot (suy luận hợp lý, sẽ xác nhận ở tài liệu 15).
- Có 1 bug thật riêng biệt (SuperWarp reboot loop, `OP_BTS-51229`, `2024/11/27`) lưu retry count vào SRAM để sống sót qua reboot.

# Verify yourself

1. Vì sao pattern dùng trong training (`0x591C5555`) không đủ để "chứng minh RAM hoạt động đúng" cho mọi dữ liệu thật?
2. DBI là gì (nhắc lại từ mục Why) — vì sao bật DBI có thể làm 1 center đã tìm đúng lúc training (DBI tắt) không còn đúng nữa?
3. Backup/restore giải quyết vấn đề gì cho RETRY mà không liên quan gì đến Warm Boot? Giải thích bằng đúng đoạn code đã trích.
4. Nếu `p_sram[8]` không sống sót qua reboot (giả sử bị xóa mỗi lần reset), bug "SuperWarp reboot loop" sẽ tái diễn thế nào?
5. Câu hỏi mở cuối tài liệu (dọn dẹp `train_base_*` sau khi PASS) — bạn sẽ tìm câu trả lời ở tài liệu nào tiếp theo, và tại sao?

**Tiếp theo:** `13_QSPI_GPIO.md` — chip detection (GPIO vs QSPI), checksum, và Swizzle — đã gặp nhiều mảnh rời rạc của phần này (`chip_info_reg`, `CHIP_DETECT_REGS_t`, các cờ `FORCE_CA_TRAIN_IGNORE`/`DQS_GATE_PREAMBLE_EN`...) — giờ sẽ ghép thành 1 bức tranh đầy đủ.
