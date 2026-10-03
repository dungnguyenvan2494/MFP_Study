# 17 — Error Handling & Retry: toàn bộ cơ chế thật, ghép từ tài liệu 05-16

Đọc `00`-`16` trước, đặc biệt `14` (2 tầng retry) — tài liệu này lấp đầy chi tiết còn thiếu của tầng TRONG, và sửa lại 1 hiểu nhầm về `lpddr4_training_Error`.

# Concept

Có **2 tầng retry hoàn toàn độc lập** (đã nêu ở tài liệu 14):

1. **Tầng trong** — 1 vòng `for` DUY NHẤT bọc quanh gần như TOÀN BỘ chuỗi training (CA training → ... → Memory Test), dùng `train_err_cnt`/`retry_num`.
2. **Tầng ngoài** — bọc quanh CẢ LỜI GỌI `lpddr4_dynamic_config()`, tối đa 6 lần, chỉ Cold Boot, dùng `lpddr4_training_Error`.

# Hardware view + Firmware view

## Tầng trong — vòng `for` thật (dòng 8124-8648, đã đọc đủ để xác nhận cấu trúc)

```c
retry:
for (train_err_cnt = 0; train_err_cnt < retry_num; train_err_cnt++) {
    p_sram[6] = train_err_cnt;      // ghi nhận lần thử hiện tại vào SRAM (post-mortem)

    // ... TOÀN BỘ: CA training, Write Leveling, DQS Gate, Read/Write Centering+Deskew,
    //     common_memfill, Memory Test DMA ...  (tài liệu 05-12)
    // bất kỳ bước nào FAIL đều có thể set train_err_flg = 1 (đã thấy rải rác ở tài liệu 06/07/08...)

    if (train_err_flg == 1) {
        p_sram[8] = train_err_cnt;
        train_err_flg = 0;                          // reset để vòng sau thử lại "sạch"
        if (force_train_repeat == 1) train_err_flg = 1;   // cờ debug: ép luôn coi là FAIL (đo Window nhiều lần)
        regval = train_err_cnt << 24;
        rdModWr(0xE8244834, regval, 0xFF000000, VERIFY);   // ghi số lần retry vào bit cao Timer3 (SPEC FACT? xem Register view)
        if (train_err_cnt != retry_num - 1) {
            restore_train_area();      // tài liệu 12 — dọn train_base_* trước khi thử lại
            return_100mhz(mc_reg);     // hạ tần số về baseline
        }
        // nếu ĐÂY LÀ LẦN CUỐI (train_err_cnt == retry_num-1) — KHÔNG restore/return_100mhz,
        // for-loop tự thoát ở vòng lặp tiếp theo (train_err_cnt sẽ = retry_num)
    }
    else {
        train_err_cnt = retry_num + 1;   // ← THỦ THUẬT thoát for-loop SỚM khi THÀNH CÔNG
    }
}   /* hết retry_num lần hoặc đã thoát sớm do thành công */

if (km_cal_window_flag == 1) calculate_window_margin();   // tài liệu 03 — chỉ đo margin nếu bật cờ debug
```

🧠[INFERENCE về "Error Flag = ON" của spec]: **không tìm thấy 1 register/biến riêng tên "Error Flag"** trong toàn bộ file — 🧠 nhiều khả năng "Error Flag = ON" mà spec mô tả (tài liệu 00) chính là **`train_err_flg` vẫn còn = 1 sau khi thoát vòng `for`** (vì ở lần cuối, code cố ý KHÔNG reset nó về 0 — khác các lần retry giữa đường). ❓[UNKNOWN]: chưa xác nhận có đoạn code nào SAU vòng `for` (ngoài phạm vi đã đọc) đọc lại `train_err_flg` để ghi vào 1 register công khai cho tầng sau (U-Boot?) đọc được — cần đọc thêm để xác nhận, không khẳng định chắc.

## QSPI override `retry_num` — khớp SPEC FACT chính xác

[SOURCE FACT — `:7903`]:
```c
retry_num = (chip_info_reg->TRAIN_RETRY_NUM > 255) ? RETRY_NUM /* =10 */ : chip_info_reg->TRAIN_RETRY_NUM;
```
✅ **Khớp hoàn toàn** với SPEC FACT tài liệu 00 (`1_0...pdf`): *"リトライ動作は10回まで実施される。(最大255回までQSPIパラメータとして設定可能)"* — giá trị QSPI `TRAIN_RETRY_NUM` (kiểu `uint32_t`, nhưng bị ép về dải 0-255) nếu hợp lệ (≤255) được dùng trực tiếp, nếu vượt (>255, tức QSPI chưa ghi/lỗi, giá trị mặc định khi trống thường là `0xFFFFFFFF`) → dùng `RETRY_NUM=10`. Đây là 1 trong số ít mục **SPEC và SOURCE khớp nhau tuyệt đối, có thể trích dẫn 2 chiều**.

## Tầng ngoài — `lpddr4_training_Error`, Ý NGHĨA THẬT khác với tài liệu 14 dự đoán

⚠️ **Sửa lại/làm rõ tài liệu 14 và 16**: đọc kỹ toàn bộ nơi `lpddr4_training_Error` được set (`:13112, 13120`, trong chính `lpddr4_backup_train_result()`, ngay sau đoạn flush cache đã trace ở tài liệu 16):
```c
for (i=0; i<576; i++) {
    if (pmem->regdata[i] != pmem2->regdata[i])   // so sánh 2 BẢN SAO training result vừa ghi
        lpddr4_training_Error = 1;
}
if (pmem->regsize != pmem2->regsize) lpddr4_training_Error = 1;
```
🧠[INFERENCE, đã CHẮC CHẮN hơn nhiều so với suy đoán ở tài liệu 14]: `lpddr4_training_Error` **KHÔNG đo "training PASS/FAIL"** — nó đo **"2 bản backup training-result (`pmem` DRAM và `pmem2` non-DRAM) có khớp nhau tuyệt đối không"**. Đây là 1 phép **kiểm tra tính nhất quán (consistency check)** của chính bước ghi-nhớ-kết-quả, không phải kiểm tra chất lượng training. 🧠 Diễn giải hợp lý: nếu 1 trong 2 lần ghi (`pmem`/`pmem2`) bị lỗi (ví dụ do đúng lúc ghi thì có 1 sự kiện bất thường phần cứng, hoặc do chính DRAM chưa ổn định — quay lại đúng bối cảnh bug SuperWarp), 2 bản sẽ lệch nhau → phát hiện được lỗi **ở khâu LƯU kết quả**, không phải lỗi ở khâu TÌM kết quả.

**Kết luận sửa lại**: Tầng ngoài (6 lần) KHÔNG lặp lại vì training tự nó fail (đó là việc của tầng trong, `train_err_flg`/`retry_num`) — nó lặp lại khi **chính bước lưu kết quả có dấu hiệu không đáng tin** (2 bản sao lệch nhau). 2 tầng retry giải quyết 2 lớp rủi ro khác nhau: tầng trong = "training tìm sai giá trị", tầng ngoài = "lưu kết quả không đáng tin".

# Register view

| Biến/Register | Ý nghĩa | Nguồn |
|---|---|---|
| `retry_num` | Giới hạn tầng trong, đọc QSPI, fallback 10 | `:7903` |
| `train_err_cnt`/`train_err_flg` | Trạng thái tầng trong | `KM_LPDDR4_config.h:80-83` |
| `lpddr4_training_Error` | Kết quả so sánh `pmem`/`pmem2` — trạng thái tầng ngoài | `:13112,13120` |
| `0xE8244834` bit [31:24] | Lưu `train_err_cnt` hiện tại vào 8 bit cao của 1 register Timer3 — 🧠 có thể là kênh để công cụ đo bên ngoài (log/debugger) đọc nhanh mà không cần dừng CPU | `:8615` |
| `p_sram[0,1,2,4,5,6,7,8]` (tại `0xC0200000`) | Bảng trạng thái đầy đủ cho debug post-mortem — xem bảng dưới | rải khắp file |

**Bảng `p_sram[]` đầy đủ** (tổng hợp từ tài liệu 12/14/16 + phát hiện mới):

| Index | Ý nghĩa |
|---|---|
| `[0]` | =1 nếu tầng ngoài gặp lỗi (memory access error) |
| `[1]` | Số lần đã thử ở tầng ngoài (i+1) |
| `[2]` | =0x77 nếu tầng ngoài cuối cùng THÀNH CÔNG |
| `[4]` | `staticOrDynamic` (config_type) |
| `[5]` | `warmboot` (0/1) |
| `[6]` | `train_err_cnt` hiện tại (sống, cập nhật mỗi vòng tầng trong) |
| `[7]` | `retry_num` đã đọc từ QSPI |
| `[8]` | `train_err_cnt` tại đúng thời điểm phát hiện FAIL |

# Source view

- `mv_lpddr4_apn806.c:7903` — QSPI override `retry_num`
- `mv_lpddr4_apn806.c:8124-8653` — toàn bộ vòng `for` tầng trong
- `mv_lpddr4_apn806.c:13108-13122` — nơi `lpddr4_training_Error` thực sự được set
- `mv_lpddr4_apn806.c:13127-13137` — `return_100mhz()`, đã đọc toàn bộ

# Call flow

```
lpddr4_config()
  for i in 0..6 (tầng NGOÀI, cold boot only):
      lpddr4_dynamic_config()
          for train_err_cnt in 0..retry_num (tầng TRONG):
              [toàn bộ training tài liệu 05-12]
              if FAIL: train_err_flg=1 → (nếu chưa phải lần cuối) restore_train_area()+return_100mhz(), thử lại
              if OK:   train_err_cnt = retry_num+1  → thoát sớm
          lpddr4_backup_train_result()
              so sánh pmem/pmem2 → lpddr4_training_Error = (khác nhau) ? 1 : 0
      if lpddr4_training_Error == 0: break (thoát tầng NGOÀI)
      else: p_sram[0]=1, p_sram[1]=i+1, thử lại TOÀN BỘ lpddr4_dynamic_config() từ đầu
```

# K-S800 customization

- Toàn bộ tầng trong (`KM_RETRY`) + QSPI override + `p_sram` dump là bổ sung KM.
- Tầng ngoài (`lpddr4_training_Error`, kiểm tra nhất quán backup) là bổ sung của đúng bug-fix SuperWarp (`2024/11/27`).

# Debugging

- Board FAIL liên tục ngay từ 1 bước cụ thể (ví dụ luôn ở CA training) → xem `p_sram[6]`/`[8]` để biết đã retry bao nhiêu lần ở tầng trong trước khi tầng ngoài can thiệp — nếu tầng trong đã dùng hết `retry_num` (`p_sram[7]`) mà vẫn fail, vấn đề là training THẬT, không phải tính nhất quán lưu trữ.
- Board reboot lặp lại nhiều lần rồi mới boot được (hoặc không boot được luôn) → xem `p_sram[0]`/`[1]`/`[2]` — nếu `[0]=1` và `[1]` tăng dần tới 6, đây là tầng NGOÀI đang retry — nghĩa là **training tự nó PASS nhưng việc lưu kết quả không nhất quán** — nghi ngờ hardware liên quan tới đường ghi `pmem2` (`0xE8200100`) hoặc chính bộ nhớ DRAM đang không ổn định (không phải lỗi thuật toán training).
- Đếm số bit set trong `0xE8244834[31:24]` qua các lần reset — cho biết xu hướng retry có tăng dần theo thời gian sử dụng máy không (gợi ý lão hoá phần cứng) hay ngẫu nhiên (gợi ý nhiễu môi trường).

# Summary

- 2 tầng retry, 2 mục đích khác nhau — không được nhầm lẫn.
- `retry_num` (tầng trong) khớp SPEC FACT tuyệt đối: 10 mặc định, tối đa 255 qua QSPI.
- Tầng trong dùng thủ thuật `train_err_cnt = retry_num+1` để thoát sớm khi thành công, thay vì `break`.
- **Sửa lại hiểu nhầm quan trọng**: `lpddr4_training_Error` (tầng ngoài) đo tính NHẤT QUÁN của việc backup kết quả (`pmem` vs `pmem2`), KHÔNG đo chất lượng training trực tiếp.
- `p_sram[]` là bảng trạng thái debug rất đầy đủ — nên là nguồn đầu tiên đọc khi debug bất kỳ vấn đề boot/training nào.
- "Error Flag = ON" của spec nhiều khả năng ứng với `train_err_flg` còn lại =1 sau khi hết `retry_num` lần — chưa xác nhận có kênh báo ra ngoài (ví dụ U-Boot đọc được) hay không.

# Verify yourself

1. Vì sao dùng `train_err_cnt = retry_num+1` để thoát for-loop sớm lại hoạt động đúng, xét kỹ điều kiện `train_err_cnt < retry_num` của vòng lặp?
2. Nếu `lpddr4_training_Error` không đo chất lượng training, điều gì có thể khiến `pmem` và `pmem2` (2 bản backup y hệt nhau về logic ghi) lại LỆCH NHAU trên thực tế? Đưa ra 1-2 giả thuyết hợp lý (được phép 🧠).
3. Tại sao ở LẦN CUỐI của tầng trong (`train_err_cnt == retry_num-1`), code cố ý KHÔNG gọi `restore_train_area()`/`return_100mhz()` dù vẫn đang FAIL?
4. Nếu bạn chỉ có quyền đọc 1 giá trị duy nhất trong `p_sram[]` để biết "máy vừa boot có ổn không", bạn chọn index nào? Vì sao?
5. 2 tầng retry độc lập nghĩa là 1 lần boot tệ nhất có thể tốn tối đa bao nhiêu lần chạy toàn bộ (hoặc gần toàn bộ) chuỗi training? Tính bằng công thức từ `retry_num` (tối đa 255) và số lần tầng ngoài (6).

**Tiếp theo:** `18_REGISTER_MAP.md` — tổng hợp toàn bộ register quan trọng đã gặp (MC6, DDR_PHY, TRN_RSLT, CHIP_DETECT) thành 1 bảng tham chiếu duy nhất.
