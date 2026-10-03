# 13 — Chip Detection (GPIO/QSPI) và Swizzle

Đọc `00`-`12` trước. Chương này ghép các mảnh đã gặp rải rác (`chip_info_reg`, `CHIP_DETECT_REGS_t`, các cờ `FORCE_*`) thành 1 bức tranh, dựa trên spec chương 5 (`5_1_GPIOによるチップ判別仕様_20181015_1.pdf`, đã đọc) + source thật.

# Concept

S800 phải chạy được với **nhiều loại chip DRAM khác nhau** (Micron/Samsung, nhiều dung lượng) mà không cần build lại firmware riêng cho mỗi loại. Cần 1 cách để firmware **biết** đang chạy trên phần cứng nào trước khi bắt đầu training (vì tham số ODT/drive-strength/timing khác nhau theo chip).

# Why

[SPEC FACT — `5_1...pdf` trang 1]: Ban đầu định dùng GPIO thuần, nhưng khi nhiều biến thể chip/dung lượng xuất hiện, **số bit GPIO không đủ mở rộng**, và team sản xuất **không muốn hàn GPIO khác nhau trên các board cùng model** (tốn chi phí). → Giải pháp: **QSPI ROM lưu sẵn tham số đầy đủ**, đọc ra dùng luôn — GPIO chỉ giữ vai trò tối thiểu/fallback.

# Hardware view — GPIO thật (SPEC FACT, đã xác nhận khớp source)

[SPEC FACT — `5_1...pdf` trang 2]:

| Bit | Ý nghĩa |
|---|---|
| `GPIOH[28:26]` | Dung lượng CH0 (0x0=1GB, 0x1=2GB, 0x2=3GB, 0x3=4GB, 0x5=6GB, 0x7=8GB) |
| `GPIOH[31:29]` | Dung lượng CH1 (cùng mã hoá) |
| `GPIOI[23:22]` | Vendor CH0 (00b=Micron, 01b=Samsung) |
| `GPIOI[25:24]` | Vendor CH1 |

⚠️ [SPEC FACT, nguyên văn]: "bit vendor chỉ hiệu lực ở môi trường FUM (dev/test), **ở môi trường máy thật (機種環境) bit này bị vô hiệu — luôn coi là Micron**" — GPIO không tự phân biệt được Samsung trên máy thật, phải dùng QSPI.

[SOURCE FACT — `mv_lpddr4_apn806.c:7977-7979`]: khớp chính xác với flowchart spec (LPPP đọc GPIOH → "gửi tạm" qua Timer2 register → ATF đọc lại):
```c
m_capacity_info.gpioa_value = mrvl_regread32(0xE830A000);  // GPIOa
m_capacity_info.gpioh_value = mrvl_regread32(0xE8244824);  // Timer2 register — nơi LPPP "gửi tạm" giá trị GPIOH
m_capacity_info.gpioi_value = mrvl_regread32(0xE830A800);  // GPIOi
```
✅ **SPEC và SOURCE khớp nhau ở đây** — xác nhận đúng cơ chế "né qua Timer2" mà spec mô tả (do GPIOH dùng chung chân với chức năng khác, xem spec bảng IO_PAD).

## Vì sao phải "né qua Timer2"?

[SPEC FACT]: các chân GPIOH[26:31] **dùng chung vật lý (share pin)** với chức năng debug khác (SMT_CLK/SMT_xSYNC/SMT_DATA — giao diện test sản xuất). 🧠[INFERENCE]: LPPP (chạy trước, ở giai đoạn có quyền truy cập GPIO) đọc GPIOH 1 lần, lưu tạm vào Timer2 (1 vùng register không bị chức năng khác tranh chấp) để ATF (chạy sau) đọc lại — vì tới lúc ATF chạy, các chân GPIOH có thể đã được cấu hình lại cho mục đích khác.

# Firmware view — QSPI, checksum, và cơ chế dự phòng (REDUNDANCY) — phát hiện SPEC/SOURCE khác biệt

## Checksum cơ bản (khớp spec)

[SOURCE FACT — `:9800-9850`, đã đọc toàn bộ 2 hàm]:
```c
uint32_t calculate_checksum_qspi(uint64_t read_addr, uint64_t read_size) {
    check_sum = 0;
    for (i=0; i<read_size; i+=4) check_sum += mrvl_regread32(read_addr+i);   // checksum = TỔNG CỘNG (không phải CRC)
    return check_sum;
}
bool judge_qspi_param(uint32_t checksum1, uint64_t read_addr_checksum2) {
    ret_val = mrvl_regread32(read_addr_checksum2);       // đọc checksum ĐÃ LƯU SẴN ở cuối vùng QSPI
    return (checksum1 == ret_val);
}
```
🧠[INFERENCE]: đây là checksum **cộng dồn đơn giản** (không phải CRC32 hay hash mạnh) — đủ để phát hiện lỗi đọc ngẫu nhiên/QSPI trống, nhưng **không chống được lỗi có chủ đích hoặc 1 số kiểu lỗi bit đối xứng** (📘[GENERAL KNOWLEDGE] hạn chế cố hữu của checksum cộng: đảo 2 byte có thể vẫn ra cùng tổng). Không phải vấn đề bảo mật ở đây — chỉ là phát hiện "QSPI có ghi tham số hợp lệ hay chưa/còn nguyên hay không", đủ dùng cho mục đích này.

## ⚠️ Phát hiện quan trọng — QSPI có CƠ CHẾ DỰ PHÒNG mà spec (2018) KHÔNG mô tả

[SOURCE FACT — comment tại `:7987, 7995`]: `2020/02/26 Harada OP_BTS-20942 LPDDR4パラメータ二重化 LPDDR4パラメータ領域の多重化対応` (đại ý: "nhân đôi tham số LPDDR4 — hỗ trợ đa vùng cho khu vực tham số LPDDR4"):

```c
checksum_qspi = calculate_checksum_qspi((uint64_t)chip_info_reg, QSPI_CHECK_SUM_READ_SIZE);
param_qspi_flag = judge_qspi_param(checksum_qspi, (uint64_t)chip_info_reg + QSPI_CHECK_SUM_READ_SIZE);

if (param_qspi_flag == 0)   // checksum vùng CHÍNH không khớp
{
    for (i=1; i<sizeof(lpddr4_param)/sizeof(lpddr4_param_t); i++) {
        sub_checksum_qspi = calculate_checksum_qspi((uint64_t)lpddr4_param[i].chip_info_reg, ...);
        sub_param_qspi_flag = judge_qspi_param(sub_checksum_qspi, ...);
        if (sub_param_qspi_flag == 1) {              // tìm được 1 vùng DỰ PHÒNG hợp lệ
            chip_info_reg = lpddr4_param[i].chip_info_reg;   // ← DÙNG LUÔN vùng dự phòng này
            goto LPDDR4_PARAM_BACKUP_USE_START_POINT;
        }
    }
}
```

⚠️[SPEC/SOURCE MISMATCH — xác nhận thật, không phải suy đoán]: Spec chương 1 (`1_0...pdf`, đã đọc ở tài liệu 00) chỉ mô tả **1 vùng QSPI param + 1 checksum**, "khớp thì dùng, không khớp thì rơi về GPIO" — **không hề mô tả cơ chế nhiều vùng dự phòng** (`lpddr4_param[]`, mảng nhiều `chip_info_reg`). Bằng chứng: comment bug-fix ghi rõ ngày `2020/02/26`, **sau** ngày phát hành spec 第2版 (`18/11/07`) hơn 1 năm. 🧠[INFERENCE]: đây là 1 cải tiến **thêm vào sau khi spec đã phát hành**, để chống trường hợp 1 vùng QSPI cụ thể bị lỗi/hỏng (ví dụ do mất điện giữa lúc ghi) — có thêm các vùng backup để vẫn đọc được tham số đúng. Đây là ví dụ ⚠️ MISMATCH **thật, có ngày tháng xác nhận**, không phải chỉ là suy luận — đúng loại phát hiện tài liệu 33 (Spec vs Source) cần.

## Cờ `FORCE_QSPI_MODE`

[SOURCE FACT — `:7993`]: `force_qspi_mode = (chip_info_reg->FORCE_QSPI_MODE != 1) ? 0 : 1;` — cùng họ với `FORCE_CA_TRAIN_IGNORE` (tài liệu 05) và `DQS_GATE_PREAMBLE_EN` (tài liệu 07): 1 cờ QSPI cho phép **ép buộc dùng QSPI** dù checksum thế nào — 🧠[INFERENCE] hữu ích khi debug/production cần chắc chắn dùng đúng 1 bộ tham số QSPI cụ thể, không để logic checksum tự quyết.

# Swizzle — ánh xạ vật lý → logic, theo board revision

[SOURCE FACT — `:12127-12175`, đã đọc toàn bộ `swizzle_setting()`]:

```c
board_type = (gpioi_value & 0x80000000) ? 1/*ES*/ : 0/*Emu*/;
if (board_type == 1) km_board_rev = 0;
else {
    switch ((gpioa_value & 0xF0) >> 4) {   // HW VER bit[3:0]
        case 0xF: km_board_rev = 0; break;  // A0 (old board)
        case 0xE: km_board_rev = 1; break;  // A1 (old board)
        case 0xD: km_board_rev = 2; break;  // A1 (mod board)
        case 0xC: km_board_rev = 3; break;  // A1S (mod board)
        default:  km_board_rev = 3;
    }
}
if (km_board_rev == 0 || km_board_rev == 1)
    swizzle_patterns[0..3] = {0x591c72a5, 0x591c5527, 0x591c36b8, 0x591caad8};
else
    swizzle_patterns[0..3] = {0x591c74a5, 0x591c5527, 0x591c36b8, 0x591cc3d8};
```

**Đây chính là khái niệm "Swizzle" từ zero**: `swizzle_patterns[]` là **1 tập pattern huấn luyện KHÁC NHAU tuỳ theo revision bo mạch** (`km_board_rev`), được dùng làm test-pattern khi deskew (`setTrainginPatterns(mc_reg, swizzle_patterns[N])`, thấy ở tài liệu 09/11 dưới `#ifdef BOARD_SWIZZLE`). 🧠[INFERENCE]: 2 revision board có cách đi dây DQ/CA vật lý (physical routing trên PCB) **hơi khác nhau** — có thể do sửa layout giữa các đợt sản xuất (A0→A1→A1 mod→A1S) — nên pattern logic gửi ra phải "xáo" (swizzle) khác nhau để bù cho việc bit thứ mấy nối vào đường vật lý nào đã đổi giữa các board rev. Đây là bằng chứng cụ thể, có số liệu, cho khái niệm "physical wiring → logical mapping" mà user's outline (Phần 8) muốn hiểu — chỉ có 1 hằng số pattern (`swizzle_patterns[0]` và `swizzle_patterns[3]`) thay đổi giữa 2 nhóm revision, 2 pattern giữa (`[1]`,`[2]`) giữ nguyên — 🧠 gợi ý chỉ 1-2 đường DQ cụ thể bị đấu khác giữa các board rev, không phải toàn bộ bus.

# Register view

| Register/Struct | Ý nghĩa |
|---|---|
| `0xE830A000`/`0xE8244824`/`0xE830A800` | GPIOa / (Timer2, chứa GPIOH đã né) / GPIOi thật |
| `CHIP_DETECT_REGS_t` (`chip_info_reg`) | Struct tham số QSPI đầy đủ — chứa `CH0_MEM_VENDER`, `FORCE_QSPI_MODE`, `FORCE_CA_TRAIN_IGNORE`, `DQS_GATE_PREAMBLE_EN`, `KM_DQS_GATE_SHMOO`, checksum... |
| `lpddr4_param[]` | Mảng các vùng QSPI dự phòng (thêm 2020) |
| `swizzle_patterns[0..3]` | 4 hằng số pattern, chọn theo `km_board_rev` |
| `CH0_MEM_VENDER` giá trị `0x11111111`/`0x00000001` | Micron / Samsung (đọc trong `memory_capacity_setting_qspi()`) |

# Source view

- Spec: `5_1_GPIOによるチップ判別仕様_20181015_1.pdf` — đã đọc toàn bộ (2 trang)
- `mv_lpddr4_apn806.c:7977-8014` — đọc GPIO + logic checksum QSPI + dự phòng, đã đọc toàn bộ
- `mv_lpddr4_apn806.c:9793-9850` — `calculate_checksum_qspi()`/`judge_qspi_param()`, đã đọc toàn bộ
- `mv_lpddr4_apn806.c:12127-12175` — `swizzle_setting()`, đã đọc toàn bộ
- `mv_lpddr4_apn806.c:12178+` — `memory_capacity_setting_qspi()` — đã đọc phần đầu (vendor detect)
- ❓ Chưa đọc `5_2_1`/`5_2_2` (QSPI chi tiết hơn) và `9_0` (Swizzle riêng) — để dành nếu cần bổ sung

# Call flow

```
Power ON
  |
  v
LPPP: đọc GPIOH → ghi tạm vào Timer2 register
  |
  v
ATF (ble): đọc GPIOa/GPIOi trực tiếp + đọc GPIOH từ Timer2
  |
  v
swizzle_setting()   [xác định board_type/km_board_rev từ GPIO → chọn swizzle_patterns[]]
  |
  v
calculate_checksum_qspi(vùng QSPI CHÍNH) + judge_qspi_param()
  |
  +-- khớp → dùng vùng QSPI chính
  |
  +-- không khớp → for mỗi vùng dự phòng lpddr4_param[i]:
  |         checksum lại → khớp → dùng vùng đó (goto LPDDR4_PARAM_BACKUP_USE_START_POINT)
  |
  +-- (nếu KHÔNG vùng nào khớp, ❓UNKNOWN — chưa đọc code fallback cuối cùng có rơi về GPIO thuần như spec mô tả không)
  |
  v
LPDDR4 Training bắt đầu (dùng tham số đã chọn)
```

⚠️[SPEC/SOURCE MISMATCH — chưa đọc đủ để xác nhận]: spec mô tả rõ "không khớp checksum → rơi về GPIO" (chỉ 2 nhánh: QSPI hoặc GPIO). Source cho thấy **3+ nhánh thật**: QSPI chính → QSPI dự phòng (nhiều vùng) → (❓ GPIO, chưa xác nhận trực tiếp code fallback cuối). Cần đọc tiếp đoạn code sau dòng 8014 để xác nhận có đúng còn nhánh GPIO cuối cùng hay không.

# K-S800 customization

- Toàn bộ `KM_CHIP_DETECT` (QSPI + dự phòng + các cờ FORCE_*) là bổ sung của KM lên trên GPIO detection gốc.
- Bug-fix `2020/02/26 Harada OP_BTS-20942` — thêm cơ chế dự phòng đa vùng — là ví dụ MISMATCH thật giữa spec (2018) và source (đã cập nhật 2020).

# Debugging

- Log in ra `checksum_qspi`/`param_qspi_flag` ngay khi boot — nếu `param_qspi_flag=0` ở vùng chính nhưng training vẫn chạy đúng tham số, kiểm tra tiếp `sub_param_qspi_flag` từng vùng dự phòng trong log để biết chính xác vùng nào được dùng.
- Nếu nghi ngờ board dùng sai swizzle pattern (deskew lạ bất thường trên đúng 1-2 đường DQ cụ thể, không phải toàn bộ) — kiểm tra `km_board_rev` tính ra có đúng với board thật không (dựa vào `gpioa_value` bit [7:4]).
- `FORCE_QSPI_MODE`/`FORCE_CA_TRAIN_IGNORE`/`DQS_GATE_PREAMBLE_EN` đều đọc từ CÙNG 1 struct `chip_info_reg` — nếu nghi ngờ hành vi bất thường, dump toàn bộ struct này ra log trước tiên, đây là "bảng điều khiển trung tâm" của rất nhiều quyết định training.

# Summary

- GPIO: dùng bit cố định trên GPIOH (dung lượng) + GPIOI (vendor, chỉ hiệu lực ở FUM) — xác nhận khớp hoàn toàn giữa spec và source.
- QSPI: checksum cộng dồn đơn giản, không phải CRC mạnh — đủ dùng cho mục đích phát hiện lỗi ghi/đọc.
- **Phát hiện MISMATCH thật, có ngày xác nhận**: source có cơ chế nhiều vùng QSPI dự phòng (2020) mà spec (2018) không mô tả.
- Swizzle = tập pattern khác nhau theo board revision, bù cho khác biệt đi dây vật lý — chỉ 1-2 pattern thay đổi giữa các nhóm revision, không phải toàn bộ.
- Cả 3 cờ FORCE_* đều sống trong `CHIP_DETECT_REGS_t` — 1 "trung tâm điều khiển" quan trọng khi debug.

# Verify yourself

1. Vì sao GPIO không thể phân biệt Samsung trên máy thật (không phải môi trường FUM)? Nhắc lại đúng lý do spec ghi.
2. Checksum cộng dồn khác CRC32 ở điểm nào — cho 1 ví dụ 2 chuỗi byte khác nhau nhưng có checksum cộng dồn giống nhau.
3. Tại sao phải đợi tới 2020 (2 năm sau khi hệ thống chính đã chạy) mới thêm cơ chế QSPI dự phòng — thử suy luận từ góc nhìn thực tế sản xuất (gợi ý: 1 lần hỏng QSPI thật xảy ra ngoài field trông sẽ như thế nào).
4. Nếu 2 board có cùng `km_board_rev` nhưng bị lắp nhầm chip DRAM khác dòng, GPIO detect sẽ báo gì, và QSPI checksum sẽ phản ứng thế nào?
5. Tài liệu này để lại 1 câu hỏi ❓UNKNOWN (fallback cuối cùng có về GPIO không) — bạn sẽ đọc dòng code nào tiếp theo để trả lời?

**Tiếp theo:** `14_COLD_BOOT.md` — dựng lại toàn bộ timeline Cold Boot thật (không phải sơ đồ giả định ban đầu), dựa trên tất cả TRN_* stage đã trace từ tài liệu 04-13.
