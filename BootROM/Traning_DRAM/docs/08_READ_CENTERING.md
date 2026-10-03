# 08 — Read Centering: lpddr4_read_centering() thật, 1 hàm được gọi 4 lần khác nhau

Đọc `00`-`07` trước, đặc biệt `03` (tài liệu 03 đã giới thiệu sơ `calculate_com()` — chương này sửa lại chính xác hơn: đây không chỉ là "trung bình Vref rồi center theo delay", mà là 1 phép tính **trọng tâm (center of mass)** thật trên cả 2 trục).

# Concept

Đọc dữ liệu (Read) là hướng `DRAM → Controller`. Sau khi DQS Gate (tài liệu 07) đã mở đúng lúc, vẫn còn 1 câu hỏi: **đúng thời điểm nào trong lúc "cổng mở" là điểm an toàn nhất để lấy mẫu DQ?** Read Centering trả lời câu hỏi đó — quét 2 trục cùng lúc (delay và Vref) để tìm ra **1 điểm nằm giữa vùng PASS an toàn nhất**, không chỉ theo 1 trục.

# Why

📘[GENERAL KNOWLEDGE] Vùng PASS khi quét cả delay và Vref không phải luôn là 1 hình chữ nhật đều — hình dạng thật của Eye thường méo (giống hình thoi/quả trứng), vì delay tối ưu có thể khác nhau tuỳ mức Vref đang thử (interaction giữa timing và voltage). Nếu chỉ tìm "left+right/2" theo 1 trục ở đúng 1 mức Vref cố định, có thể lấy nhầm điểm gần biên của Eye thật (méo), không phải tâm thật.

# Hardware view + Firmware view

## Có 2 chiến lược centering thật, chọn bằng cờ toàn cục (dòng 3736-3741)

```c
if (use_read_com_flag == 1)   mrvl_msg(..., "Beginning Read Center of Mass Training\n");
else                           mrvl_msg(..., "Beginning Read Centering - Widest Window training\n");
```
[SOURCE FACT — `KM_LPDDR4_config.h:65`]: `use_read_com_flag = 1` (mặc định). Nghĩa là **thực tế S800 mặc định KHÔNG dùng công thức "left+right/2" đơn giản cho Read Centering** — nó dùng "Center of Mass" (trọng tâm), phức tạp hơn, mô tả dưới. Chiến lược "Widest Window" (bisection đơn giản, giống CA training tài liệu 05) vẫn tồn tại trong code như 1 lựa chọn thay thế, nhưng không phải đường chạy mặc định.

## `calculate_com()` — trọng tâm thật (dòng 4462-4540)

```c
for (i = 0; i < maxY; i++)   // i = từng mức Vref đã quét
{
    if (fpDly[i][ch][cs][pup][0] != -128)     // Vref này CÓ tồn tại Window (không phải toàn FAIL)
    {
        mx = 1 + (lpDly[i][...] - fpDly[i][...]);           // "khối lượng" (mass) = độ rộng Window tại Vref i
        mxSum[...] += mx;
        xmx = mx * (fpDly[i][...]*10 + (mx-1)*5);            // mass * (điểm giữa của dải PASS tại Vref i) *10
        xmxSum[...] += xmx;
    }
}
...
comResults[ch][cs][pup][0] = (xmxSum[...]/10) / mxSum[...];   // = Σ(mass_i * midpoint_i) / Σ(mass_i)
```

Đây đúng là công thức **trọng tâm vật lý** (giống tính tâm khối lượng của 1 vật có nhiều mảnh, mỗi mảnh có khối lượng và vị trí riêng): mỗi lát Vref đóng góp 1 "khối" có khối lượng = độ rộng Window tại lát đó, và vị trí = điểm giữa Window tại lát đó — điểm trọng tâm cuối cùng bị **lát rộng hơn kéo về phía nó nhiều hơn** lát hẹp. Công thức tương tự lặp lại cho trục Vref (dùng `fpVREF`/`lpVREF`, mass = độ rộng theo Vref tại mỗi mức delay).

Sau khi tính trọng tâm thô, có 1 bước **lượng tử hoá (quantize) về đúng bước nhảy firmware có thể set** (dòng 4522-4532):
```c
modx = comResults[...][0] % dlyStep;
comResults[...][0] -= modx;
if (modx > dlyStep/2) comResults[...][0] += dlyStep;   // bo tròn tới bước gần nhất
```

⚠️ **Sửa lại tài liệu 03**: đoạn trích ở tài liệu 03 (dòng `:4991-5014`) mô tả đúng 1 phần thật (lấy `selectedVref` trung bình rồi tìm `fpDly`/`lpDly` tại đúng Vref đó để center theo delay) — nhưng đó là bước diễn ra **SAU** khi `calculate_com()` (dòng 4977) đã chạy và cho ra `comResults` (trục Vref). Nói cách khác: **trục Vref dùng trọng tâm thật (`calculate_com`), trục delay tại dòng 5010 lại dùng bisection đơn giản (left+right)/2 tại đúng mức Vref trọng tâm đó** — đây là **1 phép lai (hybrid)**, không hoàn toàn là "trọng tâm 2D" hay "bisection 1D" thuần túy. Ghi nhận là "corrected during audit" — cả 2 tài liệu 03 và 08 đều đúng về phần mình mô tả, chỉ cần đọc chung mới thấy đủ bức tranh.

## FIFO test vs DMA test — 2 cách đo PASS/FAIL khác nhau

[SOURCE FACT — `mv_lpddr4_apn806.c:3752-3755, 3742-3745`]:
```c
#ifdef USE_DMAS
if (test_dma == 1) mrvl_msg(..., "DMA based training\n");
#endif
if (test_dma == 1) inc = 4;   // bước nhảy khác khi dùng DMA (4) so với FIFO (2, mặc định)
```
🧠[INFERENCE — dựa trên cách gọi, chưa đọc thân hàm `fifo_test()`/`read_data_test2()` chi tiết, ❓UNKNOWN cơ chế nội bộ chính xác]: **FIFO test** 🧠 dùng 1 bộ đệm nhỏ trong chính PHY/Controller để chạy 1 phép test đọc ngắn, nhanh, không cần đụng tới bus hệ thống rộng — hợp lý làm bước early/coarse (chạy 4 lần trong flow: TRN_GX, TRN_H5, và ẩn trong vài chỗ khác). **DMA test** dùng đường DMA thật để đọc/ghi vùng nhớ DRAM thực tế lớn hơn (`train_base_*`, giống cơ chế đã thấy ở tài liệu 07 cho DQS gate "Data Test") — kiểm tra gần với điều kiện vận hành thật hơn, chạy sau khi các bước FIFO đã ổn định (TRN_L1, TRN_L2). Đây chỉ là suy luận hợp lý từ tên gọi + thứ tự gọi, cần đọc thân hàm `fifo_test()` để xác nhận chắc chắn — để dành tài liệu 19 nếu cần.

## 4 lần gọi thật, không phải 1 lần

[SOURCE FACT — `mv_lpddr4_apn806.c:8354, 8395, 8536, 8546`]:

| Call site | `fixVref` | `test_dma` | `use_pbs` | Mốc | Tên trong bảng TRN (tài liệu 00) |
|---|---|---|---|---|---|
| `:8354` | `read_fifo_vref_fix_en` | `FIFO_TEST` (0) | `ADD_PBS_TEST` (1) | `TRN_GX` | "Read center(1回目)完了" |
| `:8395` | `read_fifo_vref_fix_en` | `FIFO_TEST` (0) | `NO_PBS_TEST` (0) | `TRN_H5` | "Read center(2回目)完了" — **chạy SAU toàn bộ Read Deskew** (tài liệu 09) |
| `:8536` | `VREF_NOT_FIX` (0) | `DMA_TEST` (1) | `ADD_PBS_TEST` (1) | `TRN_L1` | "Read DMA center(1回目)完了" |
| `:8546` | `VREF_NOT_FIX` (0) | `DMA_TEST` (1) | `NO_PBS_TEST` (0) | `TRN_L2` | "remove PBS/Read DMA center(2回目)完了" |

**Đây xác nhận chính xác mental-model gốc trong tài liệu 00 là chưa đủ chi tiết**: không phải "Read Centering chạy 1 lần rồi qua Deskew" — thực tế **Read Centering (FIFO) chạy 2 lần bao quanh Read Deskew** (lần 1 trước deskew để có điểm khởi đầu hợp lý cho deskew; lần 2 sau deskew để tinh chỉnh lại center với từng DQ đã được deskew riêng), rồi **toàn bộ cặp đó lặp lại lần nữa bằng DMA test** (thực tế hơn) ở giai đoạn sau (gần cuối training, cùng khu vực với `common_memfill` — tài liệu 12). `use_pbs` bật ở lần đầu mỗi cặp, tắt ở lần 2 — 🧠[INFERENCE] PBS (ý nghĩa viết tắt đầy đủ ❓UNKNOWN, chưa thấy định nghĩa rõ trong file đã đọc) có vẻ là 1 kỹ thuật quét bổ sung chỉ cần chạy ở lần centering ĐẦU của mỗi cặp — dòng `vref = maxVref*use_pbs` (dòng 3836) cho thấy khi `use_pbs=1`, vòng quét Vref bắt đầu **ngay tại `maxVref`** (chỉ 1 điểm cực trị) thay vì quét đều, gợi ý PBS là 1 kiểu test nhanh ở Vref cực đại trước khi quét đầy đủ.

# Register view

| Register/Flag | Ý nghĩa |
|---|---|
| `use_read_com_flag` (KM global, default 1) | Chọn chiến lược Center of Mass (1) hay Widest Window (0) |
| `KM_DISPLAY_SHMOO_R` / `KM_DISPLAY_SHMOO_R_FIFO` (QSPI) | Bật log Shmoo cho DMA/FIFO read test riêng biệt |
| `KM_CAL_WINDOW` (QSPI) | Bật tính/log Window margin (liên hệ `calculate_window_margin()`, tài liệu 03) |
| `read_fifo_vref_fix_en` | Cờ chọn có ép Vref cố định (`VREF_FIX`) hay để tự tìm (`VREF_NOT_FIX`) cho các lần gọi FIFO |

# Source view

- `mv_lpddr4_apn806.c:3675-...` — `lpddr4_read_centering()`, đã đọc phần khai báo + đối chiếu với trích đoạn tài liệu 03
- `mv_lpddr4_apn806.c:4462-4540` — `calculate_com()` toàn bộ, đã đọc hết
- `mv_lpddr4_apn806.c:8354, 8395, 8536, 8546` — 4 call site thật, kèm mốc TRN tương ứng

# Call flow

```
(sau DQS Gate — tài liệu 07)
        |
        v
lpddr4_read_centering(FIFO, ADD_PBS)   → TRN_GX   [center thô, có PBS]
        |
        v
   (Read Deskew — tài liệu 09, chỉnh riêng từng DQ)
        |
        v
lpddr4_read_centering(FIFO, NO_PBS)    → TRN_H5   [center lại, sau khi từng DQ đã deskew]
        |
        v
   (... các bước khác ...)
        |
        v
lpddr4_read_centering(DMA, ADD_PBS)    → TRN_L1   [center bằng DMA thật, có PBS]
        |
        v
   (remove PBS — công việc riêng, tài liệu 12)
        |
        v
lpddr4_read_centering(DMA, NO_PBS)     → TRN_L2   [center DMA lần cuối]
```

# Diagram

```
Trục Vref (mỗi lát ngang là 1 mức Vref đã quét):
  Vref=10: FAIL FAIL [PASS PASS PASS PASS PASS] FAIL       mass=5, mid=giữa dải này
  Vref=20: FAIL [PASS PASS PASS PASS PASS PASS PASS] FAIL  mass=7, mid=giữa dải này (lệch 1 chút so với hàng trên)
  Vref=30: FAIL FAIL FAIL [PASS PASS PASS] FAIL FAIL FAIL  mass=3, mid=giữa dải này

comResults[...][0] (trục delay) = trọng tâm của 3 "mid" trên, CÓ TRỌNG SỐ theo mass (5,7,3)
                                  → lát Vref=20 (mass lớn nhất) kéo trọng tâm về gần nó nhất
```

# K-S800 customization

- `use_read_com_flag`/`use_write_com_flag` (mặc định cả 2 = 1) là cờ **do KM thêm** (không thấy trong comment gốc Marvell nào ở phần đã đọc) — cho phép chuyển đổi giữa 2 triết lý centering hoàn toàn khác nhau (trọng tâm thống kê vs bisection hình học đơn giản) mà không cần build lại.
- 4 lần gọi cùng 1 hàm với tổ hợp cờ khác nhau (FIFO/DMA × PBS/no-PBS) là thiết kế của chính KM cho quy trình 2-pass-trước-2-pass-sau — không phải đặc điểm chung của mv_ddr Marvell gốc (🧠[INFERENCE], chưa xác nhận trực tiếp — cần so `atf/atf_s800/drivers/marvell/mv_ddr/apn806/mv_ddr_apn806.c` để biết Marvell gốc có gọi lặp kiểu này hay không).

# Debugging

- Nếu nghi ngờ Read Centering chọn nhầm điểm (không phải do Deskew, tài liệu 09) — kiểm tra `use_read_com_flag`: nếu bật (Center of Mass) mà Eye có hình dạng bất thường (nhiều mảnh rời rạc do nhiễu), trọng tâm tính ra có thể rơi vào vùng **không hề PASS thật** (trọng tâm của nhiều mảnh rời rạc có thể nằm giữa khoảng trống) — đây là rủi ro lý thuyết của mọi phép tính trọng tâm trên dữ liệu rời rạc, không riêng gì code này.
- So sánh log giữa `TRN_GX`/`TRN_H5` (FIFO) và `TRN_L1`/`TRN_L2` (DMA) cho cùng 1 pup — nếu 2 kết quả lệch nhiều, nghi ngờ khác biệt giữa test nhanh (FIFO) và test thật (DMA) phản ánh vấn đề chỉ xuất hiện ở tải/địa chỉ bộ nhớ thật, không phải lỗi thuật toán.

# Summary

- Read Centering không dùng 1 công thức duy nhất — có 2 chiến lược thật (Center of Mass mặc định, Widest Window thay thế), chọn qua cờ toàn cục.
- `calculate_com()` tính trọng tâm thật (weighted centroid), không phải trung bình đơn giản — lát PASS rộng hơn có ảnh hưởng lớn hơn tới điểm cuối.
- Có sự lai giữa 2 kỹ thuật: trọng tâm cho trục Vref, bisection cho trục delay tại đúng Vref trọng tâm đó — sửa lại mô tả đơn giản hoá ở tài liệu 03.
- 1 hàm `lpddr4_read_centering()` được gọi **4 lần thật** với tổ hợp FIFO/DMA × PBS/no-PBS khác nhau, bao quanh Read Deskew (tài liệu 09) — không phải chạy 1 lần như mental model gốc.
- FIFO test và DMA test là 2 cách đo PASS/FAIL vật lý khác nhau — DMA chạy sau, gần điều kiện thật hơn.

# Verify yourself

1. Nếu 1 lát Vref có Window rộng 10, 1 lát khác Window rộng 2, lát nào "kéo" trọng tâm về gần nó hơn? Vì sao?
2. Giải thích vì sao 1 phép tính trọng tâm trên dữ liệu rời rạc/nhiễu có thể ra 1 điểm không PASS thật — cho 1 ví dụ số cụ thể.
3. Tại sao Read Centering (FIFO) phải chạy LẦN THỨ HAI (TRN_H5) sau khi Read Deskew (tài liệu 09) đã chạy — không chỉ chạy 1 lần trước deskew là đủ?
4. Bước lượng tử hoá (`modx = comResults % dlyStep; if (modx > dlyStep/2) += dlyStep`) làm gì về bản chất số học?
5. Nếu bạn thấy `use_read_com_flag = 0` trên 1 board cụ thể (khác mặc định), bạn sẽ nghi ngờ điều gì trước tiên?

**Tiếp theo:** `09_READ_DESKEW.md` — 5 giai đoạn thật (`read_find_deskew_start`, `read_dm_deskew`, `read_deskew`, `read_find_deskew_end`, `read_deskew_center_align`) chạy giữa 2 lần Read Centering (FIFO) vừa thấy ở trên.
