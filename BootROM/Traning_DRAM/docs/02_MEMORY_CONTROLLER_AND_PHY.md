# 02 — Memory Controller và PHY: MC6 và DDR_PHY thật trong S800

Đọc `00_MASTER_OVERVIEW.md` và `01_LPDDR4_FROM_ZERO.md` trước.

# Concept

Ở tài liệu 00, "Memory Controller" và "PHY" được vẽ là 2 lớp trừu tượng trong sơ đồ. Ở tài liệu này, ta xác nhận: **đây là 2 khối phần cứng (silicon IP) thật, khác nhau, có 2 struct C khác nhau, ánh xạ vào 2 vùng địa chỉ vật lý khác nhau** trong SoC AP806.

# Why

Nếu không tách biệt 2 khối này trong đầu, bạn sẽ đọc code và nhầm lẫn khi thấy 2 loại con trỏ khác nhau (`MC6_REGS_t *mc_reg` và `DDR_PHY_REGS_t *phy_reg`) được truyền qua rất nhiều hàm training. Phân biệt đúng giúp bạn biết: "hàm này đang ra lệnh nghiệp vụ (MC) hay đang chỉnh mạch analog/delay (PHY)?"

# Hardware view

[SOURCE FACT — `atf\atf_s800\ble\MC_regstructs.h:35-46`]: comment gốc trong file (Marvell, có Copyright + GPL license option, **không phải KM confidential** — khác với `KM_LPDDR4_config.h`):

```c
// Register File: Mckinley 6c (MC6)
typedef struct MC6_REGS_s
{
   volatile uint32_t MC_ID;          // 0x0  [R] : Memory Controller ID and Revision Register
   volatile uint32_t MC_STATUS_CH0;  // 0x4  [R] : MC Status Register CH0
   volatile uint32_t DRAM_STATUS;    // 0x8  [R] : DRAM Status Register
   ...
   volatile uint32_t USER_COMMAND_0; // 0x20 [W] : User Initiated Command Register 0
   ...
   volatile uint32_t PC0..PC7;       // 0x110-0x12c : Performance Counter Registers
   volatile uint32_t ISR;            // 0x140 [R/W] : Interrupt Status Register
```

[SOURCE FACT — `atf\atf_s800\ble\ddr_phy_regstructs.h:41-46`]:

```c
// Register File: MISL DDR PHY (DDR_PHY)
typedef struct DDR_PHY_REGS_s
{
   ...
   volatile uint32_t sdram_cfg;       // 0x400 : SDRAM Configuration
   volatile uint32_t sdram_timing_low/high;   // timing JEDEC
   volatile uint32_t sdram_operation; // 0x418 : SDRAM Operation
   volatile uint32_t ddr_odt_timing_low/high; // ODT timing
   volatile uint32_t dram_vert_cal;   // 0x4c8 : DRAM Vertical Calibration Machine Control
   volatile uint32_t dram_main_pads_cal_mach_ctrl; // 0x4cc
   volatile uint32_t dram_dll_timing; // 0x4e0 : DRAM DLL Timing
   volatile uint32_t dram_zq_init_timing/timing; // ZQ calibration
   volatile uint32_t phy_lock_mask;   // 0x670
   volatile uint32_t dram_phy_lock_status; // 0x674 [R]
   volatile uint32_t prfa;            // 0x6a0 : PHY Register File Access  ← QUAN TRỌNG, xem mục Firmware view
   volatile uint32_t training;        // 0x5b0
   volatile uint32_t training_wl;     // 0x6ac : Training Write Leveling
   volatile uint32_t odpg_ctrl;       // 0x600 : On-Die Pattern Generator Control
```

**Nhận xét kiến trúc quan trọng** 🧠[INFERENCE, dựa trên 2 struct thật ở trên]: `MC6_REGS_t` chứa những gì một "memory controller" đúng nghĩa nên có (status, user command, TrustZone range, performance counter, interrupt) — đây là khối xử lý **giao thông dữ liệu bình thường sau khi đã train xong**. Ngược lại, `sdram_cfg/timing/operation/odt` — thứ mà Phần 0 mô tả là "Memory Controller ra lệnh" — **lại nằm trong struct DDR_PHY_REGS_t**, không nằm trong MC6_REGS_t. Điều này cho thấy: **PHY của S800 (Marvell MISL DDR PHY) có bộ sinh lệnh (sequencer) riêng, đủ để tự phát lệnh JEDEC (Activate/Read/Write/Refresh/MRW...) trực tiếp tới DRAM khi đang training**, không cần đi qua MC6. Đây là lý do vì sao PHY cần "biết" timing JEDEC (tMRD, ZQCal...) — nó tự làm command sequencer trong lúc training, còn MC6 chỉ đảm nhận vai trò đó khi hệ thống chạy bình thường (Linux đọc/viết RAM).

**Địa chỉ vật lý (base address)** [SOURCE FACT — `atf\atf_s800\ble\flex_shim.h:91-95` + `mv_lpddr4_apn806.c:124-127`]:

```c
#define MC6_EXT_BASE       ((0xf0020000))   // → g_ddr_ap806_platform_config.mc_base_addr
#define MC6_EXT_PHY0_BASE  ((0xf0011000))   // → g_ddr_ap806_platform_config.phy_base_addr
```

→ MC6 và PHY là 2 vùng địa chỉ MMIO hoàn toàn khác nhau trên bus hệ thống (`0xf0020000` vs `0xf0011000`), xác nhận chúng là 2 khối phần cứng vật lý riêng, không phải chỉ là 2 tên gọi cho cùng 1 khối.

# Firmware view — cơ chế truy cập PHY thật: PRFA

Đây là phần hay bị bỏ qua nếu chỉ đọc register list: **hầu hết các "sub-register" per-pin/per-pup của PHY (delay từng DQ, drive strength, Vref offset...) không có địa chỉ MMIO riêng của chính nó.** Chúng được truy cập **gián tiếp** qua **một** register duy nhất tên `prfa` (PHY Register File Access, offset `0x6a0`).

[SOURCE FACT — `atf\atf_s800\ble\mv_lpddr4_apn806.c:464-486`]:

```c
void nova_ddrphy_write(uint8_t reg_num, uint8_t CnD, uint8_t pup_num, uint16_t data)
{
    uint32_t regval = 0xC0000000;   // bit31=1 (busy/go), bit30=1 (write)
    int count = 0;
    DDR_PHY_REGS_t *phy_reg = (DDR_PHY_REGS_t*)g_ddr_ap806_platform_config.phy_base_addr;

    regval |= ((reg_num & 0xC0) << 22) | (CnD << 26) | (pup_num << 22)
             | ((reg_num & 0x3F) << 16) | data;
    mrvl_regwrite32((addr_p) &phy_reg->prfa, regval);   // ← ghi lệnh vào 1 register duy nhất

    regval = mrvl_regread32((addr_p) &phy_reg->prfa);
    while ( (regval & 0x80000000) && (count < 100) ) {   // poll bit31 = busy
        regval = mrvl_regread32((addr_p) &phy_reg->prfa);
        count++;
    }
    if ( count >= 100 )
        mrvl_msg(MSG_ERROR, RAW_DATA, "\n\n\n!!!Error Failed to write DDR PHY!!!\n\n\n");
}
```

Từng bước (đây chính là bản chất "1 lệnh training = 1 giao dịch PHY"):

1. **Đóng gói lệnh**: `reg_num` (thanh ghi con nào), `CnD` (Command hay Data bus — mỗi PHY pup có 2 nhóm register riêng cho đường lệnh và đường dữ liệu), `pup_num` (byte-lane nào, 0-3 hoặc `ALL_PUPS`), `data` (giá trị 16-bit muốn ghi) — tất cả được nén vào 1 số 32-bit `regval`, với bit 31 = 1 (bận/khởi động giao dịch) và bit 30 = 1 (đây là ghi, không phải đọc).
2. **Ghi 1 lần vào `prfa`** — đây là cách duy nhất PHY "hiểu" bạn muốn ghi gì, vào đâu.
3. **Poll (chờ bận)**: đọc lại `prfa`, kiểm tra bit 31 — hardware PHY tự xóa bit này khi đã xử lý xong giao dịch nội bộ. Vòng lặp tối đa 100 lần.
4. **Timeout**: quá 100 lần đọc mà bit vẫn set → in lỗi `"Failed to write DDR PHY"` — đây là bằng chứng thật cho câu hỏi "PHY write timeout thì debug ở đâu" (tài liệu 21 sẽ dùng lại).

`nova_ddrphy_read()` (dòng 510-534) làm ngược lại: set bit 31 (không set bit 30 → là lệnh đọc), poll bit 31, rồi lấy 16-bit thấp của `prfa` làm giá trị đọc được.

**Đây chính là cơ chế nền cho MỌI thao tác training cấp thấp** — pad calibration, deskew, Vref, drive strength ở các tài liệu 04–11 sau này đều gọi `nova_ddrphy_write()/read()` (hoặc biến thể `nova_ddrphy_write2/read2` xử lý thêm mux khi `pup_num > 3`, dòng 488-499 — cho các pup mở rộng >3, liên quan đa channel/CS).

# Register view

| Struct | File | Header gốc | Vai trò | Cấp phép |
|---|---|---|---|---|
| `MC6_REGS_t` | `MC_regstructs.h` | "Mckinley 6c (MC6)" | Điều khiển giao thông dữ liệu bình thường: status, user command, TrustZone, performance counter, interrupt | Marvell, GPLv2-hoặc-Commercial |
| `DDR_PHY_REGS_t` | `ddr_phy_regstructs.h` | "MISL DDR PHY" | Sequencer lệnh JEDEC khi training + toàn bộ máy trạng thái calibration (vertical/horizontal pad cal, DLL, ZQ) + cổng gián tiếp `prfa` tới delay/drive-strength per-pup | Marvell, GPLv2-hoặc-Commercial |
| `CHIP_DETECT_REGS_t` (field `MC6_CA_DRIVE` v.v., offset 0x0200) | `DDR_chip_detect_regstructs.h:148-151` | (không có comment gốc dạng "Register File") | **Không phải register phần cứng sống** — là struct **bản sao tham số** lưu trong QSPI, được KM định nghĩa để lưu/khôi phục cấu hình CA drive strength / DQ ODT / Vref offset đọc từ QSPI khi chip-detect | KM (Konica Minolta) |

| Register | Offset | Ý nghĩa | Ai ghi | Ai đọc | Vì sao quan trọng |
|---|---|---|---|---|---|
| `prfa` | PHY +0x6a0 | Cổng giao dịch duy nhất tới mọi sub-register per-pup (delay, drive, Vref) | `nova_ddrphy_write()` | `nova_ddrphy_read()` | Mọi bug "training không set được delay" phải nghi ngờ từ đây trước |
| `phy_lock_mask` / `dram_phy_lock_status` | PHY +0x670 / +0x674 | Mask + trạng thái "lock" (DLL đã khoá pha ổn định chưa) | Firmware (mask) | Firmware polling (status) | [SOURCE FACT — `mv_lpddr4_apn806.c:1294-1297`: có vòng lặp `while (...phy_base_addr+0x05ec... & 0x20000000)` và đọc offset `0x0674` — khớp đúng offset `dram_phy_lock_status` — xác nhận code thật poll lock status trước khi tiếp tục] |
| `sdram_init_ctrl` | PHY +0x480 | Điều khiển khởi tạo SDRAM | Firmware | Firmware polling | [SOURCE FACT — `mv_lpddr4_apn806.c:1342`: `while (...phy_base_addr+0x0480... & 0x1) ... count<100000`] — poll bit 0 tới 100000 lần, timeout dài hơn hẳn so với PRFA (100 lần) → gợi ý bước init SDRAM cần chờ lâu hơn 1 giao dịch PHY đơn lẻ, hợp lý vì init toàn bộ chip mất nhiều chu kỳ hơn 1 lệnh ghi delay |
| `wl_dqs_pattern` | PHY +0x6dc | Pattern DQS dùng khi Write Leveling | Firmware | PHY (đọc để so khớp) | Sẽ trace kỹ ở tài liệu 06 |

# Source view

- `atf\atf_s800\ble\MC_regstructs.h` — định nghĩa `MC6_REGS_t` (Marvell)
- `atf\atf_s800\ble\MC_regmasks.h` — 9929 dòng bitmask cho MC6/PHY fields (chưa đọc chi tiết — sẽ tham chiếu theo nhu cầu ở các tài liệu sau, không đọc hết vì quá dài)
- `atf\atf_s800\ble\ddr_phy_regstructs.h` — định nghĩa `DDR_PHY_REGS_t` (Marvell), đã đọc toàn bộ 305 dòng
- `atf\atf_s800\ble\flex_shim.h:91-95` — base address MMIO
- `atf\atf_s800\ble\mv_lpddr4_apn806.c:124-127, 345-346, 445-546` — bảng cấu hình platform + 2 hàm truy cập PHY gián tiếp

# Call flow

```
Training code (mv_lpddr4_apn806.c, ví dụ pad_cal, deskew...)
        |
        v
nova_ddrphy_write(reg_num, CnD, pup_num, data)
        |
        v
đóng gói regval 32-bit  →  ghi vào  phy_reg->prfa  (offset 0x6a0)
        |
        v
poll phy_reg->prfa, chờ bit31 (busy) tự về 0   [tối đa 100 lần]
        |
        +-- OK  → return
        |
        +-- count>=100 → mrvl_msg(MSG_ERROR, "Failed to write DDR PHY")
```

# Diagram

```
        SoC AP806 bus
             |
   +---------+----------+
   |                     |
0xf0020000            0xf0011000
MC6_REGS_t            DDR_PHY_REGS_t
(Mckinley 6c)         (MISL DDR PHY)
- MC_STATUS           - sdram_cfg/timing/odt  (sequencer lệnh JEDEC khi training)
- USER_COMMAND        - dram_vert_cal / dram_main_pads_cal_mach_ctrl (pad calibration machine)
- TrustZone range     - dram_dll_timing / dram_zq_timing
- Performance counter - phy_lock_mask / dram_phy_lock_status
- ISR                 - prfa (0x6a0) ──► cổng gián tiếp tới delay/drive/Vref per-pup
                       - training / training_wl / odpg_ctrl
```

# K-S800 customization

[SOURCE FACT — `mv_lpddr4_apn806.c:473-475, 519-521`]: trong bản Marvell gốc, sau mỗi lần ghi/đọc PRFA có `delay_us(1)` cố định. KM đã bọc dòng này trong `#ifndef KM_TRAINING_REFINE` — nghĩa là **khi cờ `KM_TRAINING_REFINE` được định nghĩa (và nó được định nghĩa, xem `KM_LPDDR4_config.h:42-53`), delay 1us này bị loại bỏ hoàn toàn**. 🧠[INFERENCE liên kết với SPEC FACT ở `00_MASTER_OVERVIEW.md`]: đây rất có thể là một phần của "起動時間短縮アルゴリズム" (thuật toán rút ngắn thời gian khởi động) mà spec `1_0` nhắc tới khi giải thích số liệu ~19ms cho 532 lần enter/exit power-down — vì nova_ddrphy_write/read được gọi **hàng ngàn lần** trong toàn bộ quá trình training (mỗi delay sweep = 1 lần gọi), bớt 1us mỗi lần là tiết kiệm đáng kể trên tổng thời gian 650ms. ❓[UNKNOWN]: chưa đọc được đúng đoạn spec nào định lượng chính xác mức tiết kiệm từ việc bỏ `delay_us(1)` này — cần đọc thêm `AP_A_3` (Cold Boot training time measurement) ở các session sau để xác nhận hoặc bác bỏ liên kết này.

# Debugging

Nếu training fail và log in ra `"!!!Error Failed to write DDR PHY!!!"` hoặc `"!!!Failed to read DDR PHY!!!"`:

1. Đây là lỗi ở **tầng thấp nhất** (giao dịch PRFA không được PHY xác nhận trong 100 lần poll) — không phải lỗi thuật toán training (Vref sai, deskew sai...), mà là PHY **không phản hồi hardware** hoàn toàn.
2. Khả năng: (a) `phy_base_addr` sai/PHY chưa được cấp clock/reset đúng trước khi gọi, (b) PHY đang bận xử lý giao dịch trước đó chưa xong do timing quá nhanh giữa 2 lệnh liên tiếp (liên quan trực tiếp tới việc KM đã bỏ `delay_us(1)` ở trên — nếu bỏ delay này mà hardware thật cần nó, đây là nghi phạm hàng đầu), (c) reset PHY chưa hoàn tất (`nova_ddrphy_reset` ở dòng 445-452 — reset qua `sdram_cfg` register).
3. Không nên đoán thêm nguyên nhân ngoài 3 khả năng trên — mọi khả năng khác là ❓UNKNOWN cho tới khi trace tiếp phần pad calibration (tài liệu 04).

# Summary

- MC6 (Mckinley 6c) và DDR_PHY (MISL) là 2 khối phần cứng thật, 2 struct C khác nhau, 2 địa chỉ MMIO khác nhau (`0xf0020000` vs `0xf0011000`).
- Nghịch lý hay: các register "nghiệp vụ" như sdram timing/ODT/operation nằm trong PHY struct, không nằm trong MC — vì PHY có sequencer lệnh riêng dùng khi training.
- Hầu hết sub-register per-pup (delay, drive, Vref) không có địa chỉ MMIO riêng — phải đi qua 1 cổng duy nhất `prfa` bằng giao thức đóng gói lệnh + polling busy-bit.
- KM đã tối ưu (bỏ `delay_us(1)`) trong đường dẫn PRFA — một chi tiết nhỏ nhưng lặp lại hàng nghìn lần trong toàn bộ training, khả năng liên quan tới mục tiêu rút ngắn thời gian boot của spec.
- Lỗi "Failed to write/read DDR PHY" là lỗi tầng PHY-transaction, khác hẳn lỗi thuật toán training (sẽ gặp ở các tài liệu sau).

# Verify yourself

1. Nếu bạn thấy code gọi `mc_reg->USER_COMMAND_0 = ...`, đó là đang nói chuyện với MC6 hay PHY? Dựa vào đâu bạn biết (gợi ý: loại con trỏ, không phải tên biến)?
2. Vì sao 1 PHY chỉ cần 1 register `prfa` mà vẫn điều khiển được rất nhiều sub-register per-pup khác nhau? Giải thích cơ chế đóng gói lệnh.
3. Tại sao `sdram_init_ctrl` (tài liệu này) lại cho phép poll tới 100000 lần trong khi `prfa` chỉ cho 100 lần? Đưa ra 1 giả thuyết hợp lý (được phép tag 🧠[INFERENCE]).
4. Việc KM bỏ `delay_us(1)` sau mỗi giao dịch PRFA có rủi ro gì về mặt hardware timing, nếu PHY thật sự cần thời gian ổn định sau khi busy-bit về 0 nhưng trước khi nhận lệnh tiếp theo?
5. `CHIP_DETECT_REGS_t` (chứa `MC6_CA_DRIVE` ở offset 0x0200) có phải là register phần cứng sống không? Nếu không, nó dùng để làm gì?

**Tiếp theo:** `03_TRAINING_CONCEPT.md` — Valid Window, Eye, Center, Margin — khái niệm nền cho mọi thuật toán training cụ thể (CA training, write leveling, DQS gate...).
