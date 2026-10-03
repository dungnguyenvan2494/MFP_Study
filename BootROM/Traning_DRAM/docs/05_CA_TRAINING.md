# 05 — CA Training: lpddr4_ca_training() thật

Đọc `00`-`04` trước, đặc biệt `03` (edge/window/center — CA training dùng lại đúng công thức đó) và `04` (pad calibration phải xong trước CA training).

# Concept

CA (Command/Address) là bus **mang lệnh**, không mang dữ liệu (xem tài liệu 01). Nếu CA bị lệch thời gian (skew) so với CK (clock), DRAM có thể **hiểu sai lệnh** — ví dụ đọc nhầm địa chỉ, hoặc nhầm Activate thành Read. Đây nguy hiểm hơn lệch DQ (chỉ làm sai 1 bit dữ liệu) — lệch CA có thể làm **toàn bộ hệ thống không khởi tạo được**, vì ngay cả lệnh "bắt đầu training" cũng đi qua CA.

# Why

📘[GENERAL KNOWLEDGE] JEDEC LPDDR4 định nghĩa 1 cơ chế riêng để chính DRAM "tự báo cáo" nó nhận lệnh CA đúng hay sai, gọi là **CBT (Command Bus Training)**: đưa DRAM vào 1 mode đặc biệt (qua Mode Register Write - MRW), trong mode này CA không còn mang lệnh thật nữa mà DRAM sẽ **phản hồi lại chính giá trị CA nhận được** qua chân DQ, để firmware so sánh "tôi gửi X, DRAM nhận được gì?". Nếu khớp → PASS ở tap-delay đó.

**FSP** (Frequency Set Point) 📘[GENERAL KNOWLEDGE theo JEDEC LPDDR4]: LPDDR4 cho phép lưu **2 bộ tham số vận hành khác nhau** (FSP0/FSP1) cho 2 mức tần số khác nhau trong cùng 1 chip, chuyển đổi nhanh bằng 1 bit MR (FSP-OP = tham số vận hành hiện dùng; có thêm FSP-WR = tham số ghi trước, dùng cho lúc set MR ở tần số thấp trước khi chạy tần số cao). Đây là lý do S800 có thể "training ở 100MHz rồi nhảy lên 1050/1200MHz" — dùng FSP1 cho training/tần số cao, giữ FSP0 cho baseline.

# Hardware view + Firmware view (trace `lpddr4_ca_training()`)

[SOURCE FACT — `mv_lpddr4_apn806.c:6751`, comment gốc: "Train the CA bus, including chip select lines"]

## Bước 1 — Enter CBT (`enter_cbt()`, dòng 6523-6542)

```c
void enter_cbt(MC6_REGS_t *mc_reg, uint8_t cs)
{
    rdModWr(&mc_reg->CH0_DRAM_Config_3, CBT_MASK, CBT_MASK, VERIFY);   // set bit CBT trên MC (cả CH0, CH1)
    rdModWr(&mc_reg->CH1_DRAM_Config_3, CBT_MASK, CBT_MASK, VERIFY);

    send_mrx(WR, 13, ALL, cs);     // ghi Mode Register 13 → ra lệnh cho DRAM: "vào chế độ CBT"
    send_mrx(RD, 0, 1, 1);

    nova_ddrphy_write(0x91, 0, ALL_PUPS, 0x3);   // báo PHY: "cũng đang ở chế độ CA training"

    rdModWr(&mc_reg->CH0_DRAM_Config_2, 1 << FSP_OP_SHIFT, FSP_OP_MASK, VERIFY);  // chuyển FSP_OP = 1
    rdModWr(&mc_reg->CH1_DRAM_Config_2, 1 << FSP_OP_SHIFT, FSP_OP_MASK, VERIFY);

    rdModWr(&mc_reg->MC_Control_0, TEST_MODE_MASK, TEST_MODE_MASK, VERIFY);  // tắt tiếng Refresh command
    delay_us(1);
}
```
"Mute Refresh commands" (comment gốc) — 🧠[INFERENCE] trong lúc CBT, DRAM không xử lý lệnh bình thường (đang ở mode phản hồi CA), nên Memory Controller phải **tự ngưng gửi lệnh Refresh tự động** — nếu không, 1 lệnh Refresh chen ngang giữa lúc CBT có thể làm hỏng phép đo hoặc khiến DRAM thoát CBT ngoài ý muốn.

## Bước 2 — Đổi tần số lên cao + chờ DLL lock (dòng 6821-6824, ứng mốc `TRN_C1`)

```c
change_clk_freq(ddr_top_freq);   // đổi từ 100MHz training-clock lên tần số đích (1050 hoặc 1200MHz)
wait_for_dll_lock();
display_measure_time(km_measure_time_flag, TRN_C1);   // "100MHz -> 1050MHz周波数変更完了"
```
CA training **chạy ở tần số vận hành thật đích** (không chạy ở 100MHz) — hợp lý vì skew phụ thuộc tần số: đo ở tần số thấp không phản ánh đúng hành vi ở tần số cao (chu kỳ ngắn hơn → cùng 1 lượng skew tuyệt đối chiếm tỉ lệ % lớn hơn trong 1 chu kỳ).

## Bước 3 — Set Vref khởi điểm + dải quét (dòng 6826-6880)

```c
vrefSetting = ((dram_config_4 & VALUE_CA_MASK) >> VALUE_CA_SHIFT) + (shmooRange*51);
if (vrefSetting > 50) vrefSetting -= 21;
vrefmin[ch] = (vrefShmoo > vrefSetting) ? 0 : vrefSetting - vrefShmoo;   // vrefShmoo = 8
vrefmax[ch] = (vrefShmoo+vrefSetting > 80) ? 80 : vrefShmoo + vrefSetting;
```
Đây là **thu hẹp dải quét Vref** xung quanh 1 giá trị khởi điểm đã biết trước (đọc lại từ chính register `DRAM_Config_4` — có thể là giá trị mặc định nhà sản xuất hoặc giá trị từ QSPI), quét ±8 quanh điểm đó — không quét mù toàn dải 0-80 ngay từ đầu, tiết kiệm thời gian training.

## Bước 4 — Sweep tap-delay cho mỗi tín hiệu CA + CS, tìm Window, tính Center (dòng 6888-6963)

Ví dụ cụ thể với tín hiệu **CS** (chỉ số `[6]` trong các mảng `passing/window/lpass` — 6 chỉ số 0-5 dành cho 6 bit CA, chỉ số 6 dành cho CS):

```c
for ( taps=0; taps <= maxTaps[ch]; taps += tapStep )   // tapStep = 2
{
    applyCsDlys(taps, ch);         // set thử 1 giá trị delay
    resetPHY(PHYRSTMSK);
    retVal = ca_test_pattern(mc_reg, TRAIN_CS, ch, cs, 0x3f);   // gửi pattern test, nhận phản hồi CBT
    result = retVal & 0x3f;

    if ( result != 0x3f )   // PASS (khớp pattern mong đợi)
    {
        if ( !passing[ch][6] ) { passing[ch][6]=1; lpass[ch][6]=taps; }   // LEFT EDGE
    }
    else                     // FAIL
    {
        if ( passing[ch][6] )
        {
            passing[ch][6]=0;
            window[ch][6] = taps - lpass[ch][6];        // độ rộng window vừa đóng lại
            if ( window[ch][6] > lwindow[ch][6] )        // giữ lại window LỚN NHẤT từng thấy
            {
                lwindow[ch][6] = window[ch][6];
                ltaps[ch][6] = lpass[ch][6] + lwindow[ch][6]/2;   // ← CENTER, đúng công thức tài liệu 03
            }
        }
    }
}
```
🧠[INFERENCE quan trọng]: khác với ví dụ DQS Gate ở tài liệu 03 (dừng khi đủ 64 điểm PASS liên tục), CA training ở đây **quét hết toàn bộ dải `taps`, có thể đóng-mở nhiều Window, và chỉ giữ lại Window rộng nhất (`lwindow > lwindow` cũ mới cập nhật)** — nghĩa là CA training chấp nhận khả năng có **nhiều Window rời rạc** trên cùng 1 tín hiệu, và chọn cái tốt nhất, không chỉ Window đầu tiên tìm được.

## Bước 5 — Áp giá trị Center vừa tìm + set lại maxTaps để tránh tràn (dòng 6947-6963)

```c
if ( ltaps[ch][6] >= 31 )
{
    maxTaps[ch] -= (ltaps[ch][6]-31);
    setDelay_ClkCsCa( ch, 0, ltaps[ch][6] - 31, 0 );   // ghi giá trị CS delay cuối cùng — xem "K-S800 customization"
}
```

## Bước 6 — Exit CBT + quay lại tần số thấp (`exit_cbt()`, dòng 6553-...), ứng mốc `TRN_C2`→`TRN_C3`

```c
rdModWr(&mc_reg->MC_Control_0, 0x0, TEST_MODE_MASK, VERIFY);   // bật lại Refresh
change_clk_freq(100);                                          // hạ về 100MHz
rdModWr(&mc_reg->CH0_DRAM_Config_3, 0x0, CBT_MASK, VERIFY);     // thoát CBT trên MC
rdModWr(&mc_reg->CH1_DRAM_Config_3, 0x0, CBT_MASK, VERIFY);
rdModWr(&mc_reg->CH0_DRAM_Config_2, 0x0, FSP_OP_MASK, VERIFY);  // FSP_OP về 0
send_mrx(WR,13,ALL,cs);   // ghi lại MR13 để thoát CBT trên DRAM
nova_ddrphy_write(0x91, 0, ALL_PUPS, 0x2);                      // báo PHY thoát CA training mode
```
Sau `exit_cbt()`, mốc `TRN_C4` đánh dấu quay lại tần số cao (`100MHz->1050MHz` — dòng 7163, `set_freq_wr(mc_reg,1)`), để chuẩn bị cho các bước training tiếp theo (Write Leveling ở tài liệu 06 chạy ở tần số vận hành thật, không phải 100MHz).

# Register view

| Register/Field | Ý nghĩa | Ghi bởi |
|---|---|---|
| `CH0/1_DRAM_Config_3` bit `CBT` | Bật/tắt chế độ CBT trên Memory Controller | `enter_cbt()`/`exit_cbt()` |
| `CH0/1_DRAM_Config_2` field `FSP_OP` | Chọn FSP0/FSP1 đang áp dụng | `enter_cbt()`/`exit_cbt()` |
| `MC_Control_0` bit `TEST_MODE` | Mute lệnh Refresh tự động | `enter_cbt()`/`exit_cbt()` |
| `CH0/1_DRAM_Config_4` field `VREF_TRAINING_VALUE_CA` / `RANGE_CA` | Vref áp cho đường CA lúc training | `lpddr4_ca_training()` |
| MR13 (qua `send_mrx(WR,13,...)`) | Lệnh JEDEC bật/tắt CBT trên chính DRAM | `enter_cbt()`/`exit_cbt()` |
| PHY sub-reg `0x91` (qua `nova_ddrphy_write`) | Báo PHY vào/ra chế độ CA training (giá trị `0x3`=vào, `0x2`=ra) | `enter_cbt()`/`exit_cbt()` |

❓[UNKNOWN]: địa chỉ chính xác/tên chính thức của PHY sub-register `0x91` chưa xác nhận từ `MC_regmasks.h` hay Appendix.C — chỉ biết qua cách dùng trong code.

# Source view

- `mv_lpddr4_apn806.c:6523-6542` — `enter_cbt()`
- `mv_lpddr4_apn806.c:6553-...` — `exit_cbt()`
- `mv_lpddr4_apn806.c:6751-7163` — `lpddr4_ca_training()` (thân hàm dài, đã đọc phần khởi tạo + 1 vòng sweep mẫu; phần Vref-center tổng ở dòng 7100-7132 đã trace tại tài liệu 03)
- `mv_lpddr4_apn806.c:8280-8284` — call site, mốc `TRN_C0`/`TRN_C5`

# Call flow

```
(caller, mốc TRN_C0)
        |
        v
lpddr4_ca_training(mc_reg)
        |
        +-- enter_cbt()                     [MR13, CBT bit, FSP_OP=1, mute refresh]
        +-- change_clk_freq(ddr_top_freq)    [TRN_C1: lên tần số đích]
        +-- wait_for_dll_lock()
        +-- set Vref khởi điểm (đọc lại DRAM_Config_4 cũ)
        +-- for mỗi CA bit (0-5) và CS (6):
        |       for taps in dải quét:
        |           applyCsDlys/set delay → ca_test_pattern() → PASS/FAIL
        |           theo dõi lpass/window/lwindow/ltaps → giữ Window LỚN NHẤT
        |       setDelay_ClkCsCa(...) hoặc nova_ddrphy_write(...) → ghi Center cuối
        +-- (lặp lại toàn bộ trên cho các mức Vref khác nếu cần — chi tiết vòng ngoài
        |    chưa trace hết, ❓UNKNOWN phần vòng lặp Vref-ngoài đầy đủ)
        +-- TRN_C2 (CA training load hoàn tất)
        +-- exit_cbt()                       [về 100MHz, thoát CBT, FSP_OP=0]
        +-- TRN_C3
        +-- set_freq_wr(mc_reg,1)             [TRN_C4: lên lại tần số cao]
        v
(caller, mốc TRN_C5)
```

# Diagram

```
CA bit 0..5  ]
CS (bit 6)   ]──► mỗi tín hiệu quét riêng theo taps (delay), tìm Window RỘNG NHẤT (có thể nhiều Window rời rạc)
                    |
                    v
              Center = lpass + lwindow/2     (công thức tài liệu 03)
                    |
                    v
        setDelay_ClkCsCa() ghi vào PHY — cố định delay cho CA/CS/CLK
```

# K-S800 customization — 2 bug-fix thật, cùng ngày, cùng ticket

[SOURCE FACT — comment tại `mv_lpddr4_apn806.c:6951` và `:7101`]: cả 2 vị trí đều có comment tiếng Nhật cùng dạng, cùng tên người (`帆足`), cùng ngày `2025/03/07`, cùng mã ticket nội bộ (`OP_BTS-xxxx`, số thật bị ẩn/không đọc được rõ trong comment):

- Tại `:6951-6956`: nội dung đại ý *"CA/CS training không set đúng CA/CS/CLK delay"* — fix bằng cách gọi hàm mới `setDelay_ClkCsCa(ch, 0, ltaps[ch][6]-31, 0)` thay cho 2 lời gọi `nova_ddrphy_write()` trực tiếp cũ (nay nằm trong nhánh `#else`, tức là **dead code được giữ lại để đối chiếu**, không bị xoá).
- Tại `:7101`: nội dung đại ý *"sửa lỗi tính sai khi đổi đơn vị Vref"* (`vref換算ミスの修正`), ngay tại dòng gán `startVref[ch][cs] = vrefSetting`.

🧠[INFERENCE]: đây rất có khả năng là **2 phần sửa của cùng 1 bug report duy nhất** (cùng ngày, cùng người, cùng mã ticket) — có thể liên quan tới nhau (một lỗi tính Vref sai kéo theo delay tính theo Vref đó cũng sai). ⚠️ Theo nguyên tắc source-grounding: **đây chỉ là suy luận từ 2 comment giống nhau, không phải bằng chứng trực tiếp rằng 2 fix này thực sự liên quan về mặt kỹ thuật** — cần đọc kỹ nội dung đầy đủ 2 đoạn code trước/sau fix (không chỉ dòng comment) để xác nhận có đúng là 1 chuỗi nhân-quả hay là 2 bug độc lập tình cờ được sửa cùng ngày. Việc này để dành tài liệu 22 (Bug Analysis) — session này chỉ ghi nhận vị trí, không kết luận thêm.

Ghi chú quan trọng khác: `force_ca_training_ignore_flag` (đọc từ `chip_info_reg->FORCE_CA_TRAIN_IGNORE` qua QSPI, dòng 6802) — **KM cung cấp 1 cờ QSPI cho phép BỎ QUA hoàn toàn CA training** ("force CA training ignore mode", dòng 6805). ❓[UNKNOWN]: chưa xác nhận toàn bộ hàm có thực sự skip sweep khi cờ này = 1, hay chỉ đổi log message — cần đọc tiếp phần thân hàm dùng biến `ca_training_err_flag`/`force_ca_training_ignore_flag` (chưa trace hết, hàm dài 1163 dòng) để xác nhận ở 1 session sau.

# Debugging

Nếu CA training FAIL (nghi ngờ khi thấy log window quá hẹp hoặc `ltaps` không hội tụ):
1. Kiểm tra pad calibration (tài liệu 04) đã PASS trước đó chưa — CA training phụ thuộc driver strength/Vref đã calibrate đúng.
2. Kiểm tra `wait_for_dll_lock()` có timeout không — CA training chạy ở tần số cao, cần DLL PHY đã khoá pha ổn định trước khi sweep có ý nghĩa.
3. Nếu chỉ 1 vài CA bit cụ thể (không phải CS) fail, khả năng là vấn đề PCB layout/skew riêng cho đường dây đó — không phải lỗi thuật toán chung.
4. Kiểm tra cờ `FORCE_CA_TRAIN_IGNORE` trong QSPI — nếu vô tình bật, log sẽ nói "force CA training ignore mode" dễ gây hiểu lầm là đã train xong trong khi thực chất đã bị bỏ qua.

# Summary

- CA training dùng cơ chế CBT (JEDEC): DRAM phản hồi lại đúng giá trị CA nhận được qua DQ, để so khớp.
- Chạy ở **tần số đích thật** (1050/1200MHz), không ở 100MHz — vì skew phụ thuộc tần số.
- Cùng công thức Center = left + window/2 như tài liệu 03, nhưng cho phép nhiều Window rời rạc, chọn Window rộng nhất.
- CA (6 bit) và CS được train chung 1 vòng lặp, đánh index [0..5]=CA, [6]=CS.
- FSP_OP chuyển giữa 2 bộ tham số JEDEC khi vào/ra CBT.
- KM có 2 bug-fix thật cùng ngày (2025/03/07, ticket OP_BTS-xxxx) trong chính khu vực này — nghi vấn liên quan tới nhau, chưa xác nhận.
- KM có cờ QSPI cho phép bỏ qua hoàn toàn CA training — cần cẩn trọng khi debug log.

# Verify yourself

1. Vì sao CA training cần đưa DRAM vào 1 "mode đặc biệt" (CBT) thay vì chỉ gửi lệnh CA bình thường rồi xem DRAM có phản ứng đúng không?
2. FSP0/FSP1 giải quyết vấn đề gì — vì sao không dùng luôn 1 bộ tham số cho cả tần số thấp và cao?
3. Vì sao "Mute Refresh" lại cần thiết trong lúc CBT — nếu không mute, điều gì tệ nhất có thể xảy ra?
4. CA training cho phép nhiều Window rời rạc và chọn window rộng nhất — điều này khác gì so với DQS Gate (tài liệu 03) chỉ dừng ở Window đầu tiên đủ 64 điểm PASS? Bạn nghĩ vì sao 2 thuật toán chọn chiến lược khác nhau?
5. Nếu bạn là kỹ sư debug và thấy log "force CA training ignore mode" trên 1 board đang lỗi RAM — bước tiếp theo hợp lý nhất là gì?

**Tiếp theo:** `06_WRITE_LEVELING.md` — write leveling thật, quan hệ CK-to-DQS, và vì sao nó phải chạy NGAY SAU CA training (đã có CA/CS delay đúng) nhưng TRƯỚC DQS Gate training.
