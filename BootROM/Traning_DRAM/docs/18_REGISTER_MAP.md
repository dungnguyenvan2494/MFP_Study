# 18 — Register Map: bảng tham chiếu tổng hợp

Đọc `00`-`17` trước. Đây là tài liệu **tổng hợp thuần** — không thêm fact mới, chỉ gom mọi register/struct đã xác nhận từ tài liệu 02-17 vào 1 nơi tra cứu nhanh. Mỗi dòng đều có link ngược tới tài liệu gốc — khi nghi ngờ, mở lại tài liệu đó để xem trích dẫn `file:line` đầy đủ.

# 4 struct/register-file chính (phân biệt theo tài liệu 02)

| Struct | File | Base address | Vai trò |
|---|---|---|---|
| `MC6_REGS_t` | `MC_regstructs.h` | `MC6_EXT_BASE = 0xf0020000` | Memory Controller thật ("Mckinley 6c") — giao thông dữ liệu bình thường |
| `DDR_PHY_REGS_t` | `ddr_phy_regstructs.h` | `MC6_EXT_PHY0_BASE = 0xf0011000` | PHY thật ("MISL DDR PHY") — sequencer lệnh training + calibration machine + cổng `prfa` |
| `TRN_RSLT_REGS_t` | `DDR_trn_result_regstructs.h` | `DDR_RSLT_EXT_BASE = 0xF4380000` (struct định nghĩa); ghi thực tế qua `TRAIN_RESULT_MEM_ADDR = 0x200000000` (DRAM) + `0xE8200100` (dự phòng) | Kết quả training đã "đóng băng", dùng khi Warm Boot |
| `CHIP_DETECT_REGS_t` | `DDR_chip_detect_regstructs.h` | `chip_detect_base_addr` (từ `g_ddr_ap806_platform_config`) | Tham số QSPI: vendor, dung lượng, mọi cờ `FORCE_*`/`KM_*` |

# Cơ chế truy cập gián tiếp — PRFA (tài liệu 02)

| Register | Offset (trong `DDR_PHY_REGS_t`) | Vai trò |
|---|---|---|
| `prfa` | `+0x6a0` | Cổng DUY NHẤT tới mọi sub-register per-pup (delay/drive/Vref) — đóng gói lệnh qua `nova_ddrphy_write()`/`read()`, poll bit 31 |
| `phy_lock_mask`/`dram_phy_lock_status` | `+0x670`/`+0x674` | Mask + trạng thái DLL/calibration lock — giá trị lock đầy đủ = `0x3ffffff` |
| `dram_main_pads_cal_mach_ctrl` | `+0x4cc` | Bật/tắt pad calibration động (`DYNPADCALEN`), cách cập nhật (`CALUPCTRL`) |
| `dram_ctrl_cal_mach_ctrl` | `+0xdc8` | Giá trị calibration tự động (`AUTONGCALVAL`/`AUTOPGCALVAL`) + override thủ công (`CALVALMANOVRD`, `MANNGCALVAL`/`MANPGCALVAL`) |
| `training_wl` | `+0x6ac` | Write Leveling: chọn CS (`TRN_WL_CS`/`_UPD`), toggle DQS (`TRN_WL_DQS`), đọc phản hồi bit [23:20] |

# Bảng "sub-register per-pup" (đi qua PRFA, không có địa chỉ MMIO riêng)

| reg_num (hex) | CnD | Vai trò | Tài liệu |
|---|---|---|---|
| `0x1` | 0 | Delay mức pup — Write | 11 |
| `0x2` | 0 | Delay mức pup — DQS Gate (`+4*cs`) | 07 |
| `0x3` | 0 | Delay mức pup — Read | 09 |
| `0x13` | 0 | Delay riêng DM — Write (`+cs*0x10`) | 11 |
| `0x53` | 0 | Delay riêng DM — Read (`+cs*0x10`) | 09 |
| `0x90` | 0 | Reset FIFO/counter (`0x4002`→`0x6002`) | 05-11 (dùng lặp lại nhiều nơi) |
| `0x91` | 0 | Vào/ra CA training mode (`0x3`=vào, `0x2`=ra) | 05 |
| `0x9A`/`0x9B` | 0 | Cờ trạng thái nhận diện Preamble — PASS khi `0x100`/`0x3F7F` | 03, 07 |
| `0xA1`/`0xA2` | 0 | Tắt CMOS receiver comparator lúc pad_cal | 04 |
| `0xA8` | 0 | Enhanced Vref (pad_cal); | 1 | Pull-down control (Sleep) | 04, 15 |
| `0xAA` | 0/1 | Set DDR mode LVSTL | 05 |
| `0xBF` | 0/1 | Áp giá trị calibration đã đo | 04 |
| `dqRegAddr[ch][i+pup*8]` | 0 | Delay riêng từng bit DQ (i=0-7), qua bảng địa chỉ động | 09, 11 |
| `caRegAddr[ch][...]`/`caPupAddr[ch][...]` | 1 | Delay CA/CS/CLK, qua bảng địa chỉ động | 05, 06 |

# Register "nghiệp vụ" trên MC6 (`MC6_REGS_t`)

| Register | Vai trò | Tài liệu |
|---|---|---|
| `USER_COMMAND_0` | Lệnh vào/ra Self-Refresh, Power-Down (bit `0x40`/`0x80`/`0x20`) | 15 |
| `DRAM_STATUS` | Xác nhận trạng thái SR/PD (polling theo mask ch/cs) | 15 |
| `MC_STATUS_CH0/1` | Trạng thái "drain" (xả lệnh ghi chờ) | 15 |
| `CH0/1_DRAM_Config_2` | Field `FSP_OP` (CA training), `DM` (bật/tắt DM chế độ vận hành) | 05, 12 |
| `CH0/1_DRAM_Config_3` | Bit `CBT` (CA training), `RD_PRE_TRAINING` (DQS Gate preamble) | 05, 07 |
| `CH0/1_DRAM_Config_4` | Field `VREF_TRAINING_*` (CA/DQ Vref), bit `READ_DBI`/`WRITE_DBI` | 03, 12 |
| `MC_Control_0` | Bit `TEST_MODE` — mute Refresh lúc CBT | 05 |
| `CH0_PHY_RL_Control_CS0/1_B0` | Cycle-delay thô cho DQS Gate | 07 |

# Register kết quả training (`TRN_RSLT_REGS_t`) — xem bảng đầy đủ ở tài liệu 16

Không lặp lại ở đây — tài liệu 16 đã có bảng offset đầy đủ 0x0000-0x08FC.

# Register QSPI param quan trọng (`CHIP_DETECT_REGS_t`)

| Field | Vai trò | Tài liệu |
|---|---|---|
| `FORCE_QSPI_MODE` | Ép dùng QSPI dù checksum sao | 13 |
| `FORCE_CA_TRAIN_IGNORE` | Bỏ qua hoàn toàn CA training | 05 |
| `DQS_GATE_PREAMBLE_EN` | Chọn phương pháp DQS Gate (Preamble/Data-test) | 07 |
| `KM_DQS_GATE_SHMOO`/`KM_DISPLAY_SHMOO_R`/`_R_FIFO` | Bật log Shmoo | 07, 08 |
| `KM_CAL_WINDOW` | Bật tính Window Margin | 03, 17 |
| `KM_MEASUREMENT_TIME` | Bật đo thời gian từng giai đoạn (Timer3, TRN_*) | 14 |
| `TRAIN_RETRY_NUM` | Override `retry_num` (≤255) | 17 |
| `CH0/1_MEM_VENDER` | Vendor code (Micron `0x11111111`/Samsung `0x00000001`) | 13 |
| `MC6_CA_DRIVE`/`MC6_DQ_DRIVE`/`MC6_DQ_ODT`/`MC6_READ_VREF_OFFSET` | Tham số điện từ QSPI, áp thẳng vào PHY qua `nova_ddrphy_write` | 02 |

# Register/địa chỉ đặc biệt khác

| Địa chỉ | Ý nghĩa | Tài liệu |
|---|---|---|
| `0xE830A000`/`0xE8244824`/`0xE830A800` | GPIOa / Timer2 (né GPIOH) / GPIOi | 13 |
| `0xE8244834[31:24]` | Lưu `train_err_cnt` hiện tại — kênh đọc nhanh cho debug ngoài | 17 |
| `0xC0200000` (`p_sram`, 2048 byte) | Bảng trạng thái debug đầy đủ (bug SuperWarp) — xem bảng tài liệu 17 | 12, 14, 17 |
| `0xE8200100` (`pmem2`) | Bản sao dự phòng training result, ngoài DRAM | 16 |
| `CSS_ADDR + 0x0100` | "DM to DQ swap" (bit `0xff0`) + Vref/board revision liên quan | 04, 09, 11 |
| `CSS_ADDR + 0x4360`/`0x8d38` | Ref range select / Vref calibration tầng SoC, trước pad_cal | 04 |

# Cách tra cứu ngược (khi debug thật)

1. Có 1 giá trị hex đọc được từ log/debugger → tìm trong bảng "sub-register per-pup" nếu đi qua `nova_ddrphy_write/read` (đối số đầu là `reg_num`).
2. Nếu là địa chỉ MMIO trực tiếp (`0xf00...`, `0xf43...`, `0xE8...`) → so khớp base address ở bảng đầu tài liệu để biết thuộc struct nào, rồi tìm offset trong file `.h` tương ứng.
3. Nếu là 1 field trong `chip_info_reg` → tra bảng QSPI param ở trên, rồi đọc lại đúng tài liệu 05/07/13/17 để hiểu tác động.

**Tiếp theo:** `19_CALL_GRAPH.md` — call graph thật từ entry point tới return, thay mọi placeholder ở Phần 29 outline bằng tên hàm thật.
