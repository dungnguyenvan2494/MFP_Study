# 16 — Training Result Storage: TRN_RSLT_REGS_t, gTrainRegReadTbl, lpddr4_backup_train_result()

Đọc `00`-`15` trước, đặc biệt `15` (Warm Boot đã hé lộ "ATF restore tham số training, LPPP không làm việc đó" — đây là tài liệu trả lời chính xác **cái gì được lưu, ở đâu, ai lưu, ai đọc lại**).

# Concept

Sau khi training PASS, hàng trăm giá trị delay/Vref (mỗi giá trị là kết quả 1 vòng sweep tốn thời gian ở tài liệu 04-11) phải được **đóng gói lại thành 1 khối dữ liệu duy nhất**, lưu vào nơi **sống sót qua mất điện** (QSPI, không phải RAM/SRAM thường), để lần resume sau đọc lại và ghi thẳng vào PHY — không cần sweep lại.

# Hardware view — `TRN_RSLT_REGS_t`, struct thật, đã đọc toàn bộ 607 dòng

[SOURCE FACT — `atf\atf_s800\ble\DDR_trn_result_regstructs.h:29-607`, comment gốc: "Register File: LPDDR4 trainging result register in QSPI ROM"]

Struct này **có kích thước cố định 0x900 = 2304 byte** (khớp chính xác với `KM_TRAIN_ADDR0_SIZE (2*1024)` gặp ở `KM_LPDDR4_config.h:190` — 🧠[INFERENCE] có thể 2048 là kích thước dự trù/cấp phát, còn 2304 là kích thước thật dùng). Bảng đối chiếu **từng vùng training đã học ở tài liệu 04-11 với đúng field lưu kết quả của nó**:

| Offset | Field | Lưu kết quả của... | Tài liệu |
|---|---|---|---|
| 0x0000 | `CH0/1_DRAM_Config_4_RSLT` | Vref CA cuối cùng | 05 |
| 0x0008-0x0074 | `CH0_CADLY0/1_CA0..CA5`, `_CS0/1`, `_CKE0/1`, `CLKDLY0/1_CK0/CK0n/CK1/CK1n` | Delay từng CA/CS/CKE/CK, CH0 | 05 |
| 0x0078-0x00E4 | (tương tự cho CH1) | | 05 |
| 0x0100-0x015C | `CH*_CS*_WLCLKDLY_CK*/CKE*` | Write Leveling — CLK-shift (❓ tài liệu 06 đã ghi nhận phần này bị comment-out trong code hiện tại — field VẪN TỒN TẠI trong struct, chỉ không được ghi giá trị mới) | 06 |
| 0x0180-0x01BC | `CH*_CS*_WLDLY_PUP0..3` | Write Leveling — DQS-shift (`dqsShift[k]`) | 06 |
| 0x0200-0x020C | `CH*_PHY_RL_Control_CS*_B0` | DQS Gate cycle delay (`delay_reg[ch][cs]`, chính xác cùng tên field đã gặp) | 07 |
| 0x0210-0x024C | `CH*_CS*_DQSDLY_PUP0..3` | DQS Gate phase/phy delay | 07 |
| 0x0280-0x04BC | `CH*_CS*_R_SKWDLY_PUP*_DQ0..7_DM` | Read Deskew — delay riêng từng bit DQ+DM (`finalDqDly`) | 09 |
| 0x0500-0x073C | `CH*_CS*_W_SKWDLY_PUP*_DQ0..7_DM` | Write Deskew — delay riêng từng bit DQ+DM | 11 |
| 0x0780-0x07BC | `CH*_CS*_R_VREF_PUP0..3` | Read Centering — Vref (`selectedVref`) | 08 |
| 0x07C0-0x07FC | `CH*_CS*_R_DQDLY_PUP0..3` | Read Centering — center delay (`selectedCenter`) | 08 |
| 0x0880-0x08BC | `CH*_CS*_W_DQDLY_PUP0..3` | Write Centering — center delay | 10 |

✅ **Xác nhận hoàn toàn**: MỌI giá trị mà tài liệu 04-11 đã trace ra ("giá trị cuối cùng ghi vào PHY") đều có đúng 1 ô nhớ tương ứng trong struct này — không có giá trị "trôi nổi" nào bị bỏ sót. Đây là bằng chứng mạnh cho thấy struct này được thiết kế **đúng theo thứ tự các bước training thật**, không phải thiết kế trừu tượng riêng.

❓[UNKNOWN — chưa xác nhận]: các block "reserve_trn_result*" (rất nhiều, ví dụ 0x04C0-0x04FC, 0x0800-0x087C) — 🧠[INFERENCE] đây là chỗ trống dự phòng để mở rộng thêm field sau này (giống tinh thần dự phòng đã thấy ở QSPI param, tài liệu 13), không phải dữ liệu thật.

# Firmware view — cơ chế thu thập chung: `gTrainRegReadTbl[576]`

[SOURCE FACT — `KM_LPDDR4_config.h:194-196`, `DDR_trn_result_regstructs.h:609-618`]: có 1 lớp trừu tượng hoá đứng TRÊN struct cụ thể — 1 bảng `Train_RegRead_t gTrainRegReadTbl[TRAIN_REG_READ_TBL_MAX=576]`, mỗi phần tử là `{data, mode}`:

```c
typedef struct {
    uint32_t data;   // địa chỉ MMIO thật, HOẶC mã lệnh PHY reg_num/CnD/pup (tuỳ mode)
    uint8_t  mode;   // 0: mrvl_regread32 (đọc trực tiếp MMIO)
                      // 1: nova_ddrphy_read3 (đọc gián tiếp qua PRFA, tài liệu 02)
                      // 2: Reserve area (không đọc gì, ghi 0)
} Train_RegRead_t;
```
🧠[INFERENCE]: đây là thiết kế **tách rời "danh sách cần đọc" khỏi "cách đọc"** — mỗi entry tự biết nó cần `mrvl_regread32()` thẳng (cho MC6/DDR_PHY struct thường) hay phải đi qua cơ chế PRFA gián tiếp (cho sub-register per-pup, tài liệu 02) hay bỏ qua — cho phép hàm backup chỉ cần 1 vòng `for` duy nhất, không cần biết chi tiết loại register nào ở vị trí nào.

## `lpddr4_backup_train_result()` — trace toàn bộ (dòng 13008-...)

```c
pmem  = (Train_Result_t*)TRAIN_RESULT_MEM_ADDR;   // 0x200000000 — 1 địa chỉ DRAM thật (vùng CH1)
pmem2 = (Train_Result_t*)0xE8200100;              // ← THÊM MỚI bởi bug-fix SuperWarp (2024/11/27, OP_BTS-51229)

for (i=0; i<576; i++) {
    switch (gTrainRegReadTbl[i].mode) {
    case 0: pmem->regdata[i] = mrvl_regread32(gTrainRegReadTbl[i].data);
            pmem2->regdata[i] = mrvl_regread32(gTrainRegReadTbl[i].data);   // ghi CẢ 2 nơi
            break;
    case 1: phy_regread_tmp[i] = nova_ddrphy_read3(gTrainRegReadTbl[i].data);
            pmem->regdata[i] = phy_regread_tmp[i];
            pmem2->regdata[i] = phy_regread_tmp[i];
            break;
    case 2: pmem->regdata[i] = 0; pmem2->regdata[i] = 0;   // dummy
            break;
    }
}
pmem->regsize = pmem2->regsize = sizeof(uint32_t) * i;

// clean + invalidate D-cache (toàn bộ, bằng assembly ARM64 tay) — comment: "u-boot直接" (giao tiếp trực tiếp U-Boot)
```

**2 điểm quan trọng cần hiểu đúng:**

1. **Vì sao cần flush cache tay bằng assembly?** 📘[GENERAL KNOWLEDGE]: CPU thường đọc/ghi qua cache trước, dữ liệu thật có thể chưa "xuống" DRAM vật lý ngay. `pmem` nằm ở địa chỉ DRAM (`TRAIN_RESULT_MEM_ADDR`) — nếu **U-Boot** (chạy sau, có thể ở ngữ cảnh cache khác, hoặc đọc bằng DMA/thiết bị khác không qua cache CPU) đọc trực tiếp vùng DRAM này mà ATF chưa flush cache, U-Boot có thể đọc được dữ liệu CŨ/rác. Comment gốc "u-boot直接" (U-Boot đọc trực tiếp) xác nhận đúng mục đích này.

2. **Vì sao có `pmem2` ở địa chỉ cố định `0xE8200100` (KHÔNG phải DRAM)?** 🧠[INFERENCE, cùng nhóm bug SuperWarp đã thấy ở tài liệu 12/14]: đây là **bản sao dự phòng, nằm ngoài DRAM** — 🧠 hợp lý vì lúc SuperWarp resume, nếu có nghi ngờ về tính toàn vẹn DRAM (đây chính là bối cảnh bug SuperWarp: "khởi động lại liên tục"), 1 bản sao nằm ở vùng địa chỉ cố định non-DRAM (khả năng là APB/register space nội bộ SoC, ❓UNKNOWN chưa xác nhận đây là SRAM hay loại vùng nhớ gì cụ thể) sẽ **không bị ảnh hưởng bởi bất kỳ vấn đề gì xảy ra với chính DRAM** — 1 dạng "insurance copy" độc lập khỏi chính thứ nó đang mô tả.

# Data flow đầy đủ (trả lời Phần 23 của outline)

```
Training (tài liệu 04-11) → ghi thẳng vào PHY/MC register thật
        |
        v
lpddr4_backup_train_result()   [gọi ở đâu trong chuỗi? ❓UNKNOWN — chưa xác nhận vị trí gọi chính xác
        |                        trong toàn bộ 13778 dòng, có khả năng ngay sau khi training PASS hoàn toàn]
        |
        v
gTrainRegReadTbl[576] → đọc lại TOÀN BỘ register liên quan (MC6 + PHY gián tiếp)
        |
        v
   pmem  @ 0x200000000 (DRAM, cache-flush cho U-Boot đọc)
   pmem2 @ 0xE8200100   (bản dự phòng, KHÔNG DRAM — bug SuperWarp)
        |
        v
[U-Boot] qspi_winbond.c ❓ CHƯA ĐỌC FILE NÀY — đọc pmem, ghi vào QSPI thật
        |
        v
   [Sleep/Resume xảy ra]
        |
        v
[ATF, warmboot=true] chip_info_reg/train_reg đọc lại từ vùng QSPI (đã thấy cơ chế đọc QSPI ở tài liệu 13)
        |
        v
Các hàm training (tài liệu 07/08/10...) nhận train_reg → SKIP sweep → ghi thẳng giá trị đã lưu vào PHY
```

✅ **Cập nhật — đã xác nhận trực tiếp (sửa từ ❓UNKNOWN thành SOURCE FACT)**: [SOURCE FACT — `mv_lpddr4_apn806.c:8698-8702`] `lpddr4_backup_train_result()` được gọi ngay sau khi cold-boot training PASS (nhánh `warmboot==false`), có comment gốc xác nhận đúng data flow đã suy luận ở trên:
```c
/* 電源ON起動時                       */   // "Lúc khởi động do bật nguồn (Power-ON)"
/* トレーニング結果を一時退避する     */   // "Tạm lưu kết quả training"
/* この後、u-bootにてQSPIへ保存        */   // "Sau đó, U-Boot sẽ lưu vào QSPI"
lpddr4_backup_train_result();
```
✅ Xác nhận đúng 100%: ATF chỉ tạm lưu vào `pmem`/`pmem2` (DRAM + địa chỉ dự phòng) — **U-Boot mới là nơi thực sự ghi vào QSPI**, đúng như suy luận trong mục Data flow.

⚠️ Còn 1 mắt xích chưa xác nhận: ❓ Nội dung thật của `BootROM\BootROM\Emu800\Src\drivers\spi\qspi_winbond.c` — chưa đọc file này, chỉ biết vai trò từ spec chương 3 (tài liệu 00) + comment vừa xác nhận trên.

**Phát hiện bổ sung tại cùng vị trí** [SOURCE FACT — `:8704-8717`]: nhánh `warmboot==true` gọi `check_warm_training()` — nếu trả về khác 0 (`warm_train_flag != 0`), log lỗi `"Warmboot training error !! 0x%08x. Training Retry."`, rồi `restore_train_area()` + `return_100mhz()` + **`goto retry;`** — nhảy thẳng vào LẠI vòng `for` retry tầng trong (tài liệu 17)! Đây là bằng chứng: **Warm Boot cũng có thể kích hoạt lại toàn bộ retry loop**, nếu giá trị training result restore từ QSPI bị phát hiện không hợp lệ (`check_warm_training()` — ❓UNKNOWN chưa đọc thân hàm này, chưa biết chính xác nó kiểm tra gì) — sẽ nối vào tài liệu 17.

# Register view

Đã liệt kê đầy đủ trong bảng ở mục Hardware view — không lặp lại.

# Source view

- `atf\atf_s800\ble\DDR_trn_result_regstructs.h` — đã đọc toàn bộ 621 dòng
- `atf\atf_s800\ble\mv_lpddr4_apn806.c:13008-13100+` — `lpddr4_backup_train_result()`, đã đọc phần chính (thân vòng lặp + cache flush); chưa đọc hết đoạn assembly flush cache tới cuối
- `atf\atf_s800\ble\KM_LPDDR4_config.h:194-196` — khai báo `gTrainRegReadTbl`
- ❓ Chưa đọc `BootROM\BootROM\Emu800\Src\drivers\spi\qspi_winbond.c`

# K-S800 customization

- Toàn bộ cơ chế `TRN_RSLT_REGS_t`/`gTrainRegReadTbl`/`lpddr4_backup_train_result()` là bổ sung của KM (`KM_RESUME_REFINE`).
- `pmem2` (bản sao dự phòng non-DRAM) — bổ sung của bug-fix SuperWarp (2024/11/27) — cùng 1 triết lý với `p_sram[]` (tài liệu 12/14): **khi nghi ngờ độ tin cậy của đường phục hồi chính, thêm 1 bản sao độc lập ở nơi khác**.

# Debugging

- Nếu resume xong mà training result "sai" (delay áp vào không hợp lý, ví dụ 0 hết) — nghi ngờ: (a) cache chưa flush đúng trước khi U-Boot đọc `pmem`, (b) QSPI ghi từ `pmem` bị lỗi/không xảy ra, (c) đọc lại từ QSPI sai offset. Kiểm tra `pmem2` (`0xE8200100`) làm điểm đối chiếu — nếu `pmem2` đúng nhưng hành vi thật sai, khả năng cao lỗi nằm ở khâu ghi/đọc QSPI (U-Boot), không phải khâu backup trong ATF.
- `regsize` field cho biết đúng bao nhiêu byte thật được dùng (`576 * 4` byte nếu đọc hết bảng) — nếu giá trị này khác `576*4`, có gì đó dừng vòng lặp sớm bất thường.

# Summary

- `TRN_RSLT_REGS_t` (2304 byte) là "bộ nhớ dài hạn" của toàn bộ training — mọi giá trị từ tài liệu 04-11 đều có 1 ô tương ứng, đã xác nhận khớp gần như hoàn toàn.
- Thu thập qua 1 bảng tổng quát `gTrainRegReadTbl[576]` tách "địa chỉ" khỏi "cách đọc" (trực tiếp/gián tiếp/dummy).
- Có 2 bản sao (`pmem` DRAM + `pmem2` non-DRAM) — bản 2 là bổ sung an toàn từ bug SuperWarp.
- Cache phải flush tay bằng assembly vì U-Boot đọc trực tiếp vùng DRAM này, không qua cùng cache context với ATF.
- 2 mắt xích của toàn bộ data flow (nơi gọi backup, và nội dung `qspi_winbond.c`) vẫn còn ❓UNKNOWN — thành thật ghi nhận, không lấp bằng suy đoán.

# Verify yourself

1. Nếu bạn phải tìm 1 giá trị Vref cụ thể của Read Centering (CH1, CS0, PUP2) trong dump QSPI thật, bạn tính offset thế nào từ bảng struct ở trên?
2. Vì sao cần "cache flush" trước khi U-Boot đọc, nhưng bản thân ATF đọc/viết register PHY/MC (qua `mrvl_regread32`/PRFA) lại không cần lo vấn đề cache tương tự?
3. `pmem2` không nằm trong DRAM — điều đó có ý nghĩa gì cho khả năng "sống sót" của nó nếu chính DRAM đang gặp vấn đề (liên hệ bug SuperWarp)?
4. Nếu 1 field mới cần thêm vào struct (ví dụ do phát hiện thêm 1 loại delay chưa lưu), bạn sẽ dùng field nào có sẵn để không phải đổi kích thước tổng (gợi ý: đọc lại tên các field "reserve_trn_result*")?
5. Bảng đối chiếu ở đầu tài liệu là bằng chứng cho điều gì về CÁCH thiết kế struct này — thiết kế trước rồi code theo, hay code trước rồi struct được tạo theo sau? Bạn nghĩ sao (được phép suy đoán, nhớ đánh dấu 🧠).

**Tiếp theo:** `17_ERROR_RETRY.md` — tổng hợp đầy đủ toàn bộ cơ chế lỗi/retry đã gặp rải rác (train_err_flg, retry_num, lpddr4_training_Error, Error Flag) thành 1 bức tranh hoàn chỉnh.
