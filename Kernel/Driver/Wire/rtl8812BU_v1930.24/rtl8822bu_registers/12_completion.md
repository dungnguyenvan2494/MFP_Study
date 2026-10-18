# 12 — Bảng theo dõi mức độ hoàn thành (Lượt 6)

> Sinh bởi `tools/completion.py work .. 12_completion.md`. Định nghĩa 6 trạng thái theo 00_REQUEST §9 — xem docstring của script. "Live" = trạng thái A/T/M/G (có truy cập ở nhánh chạy được hoặc MP/debug).

## 1. Theo không gian

| Không gian | Tổng | Live (A/T/M/G) | S1 định nghĩa + truy vết | S2 định nghĩa, không dùng (D/Z) | S3 chỉ truy cập bằng số | S4 live đã phân tích ở tài liệu tay | S5 live có field/bit được code dùng | S6 live không có định nghĩa bit (cần tài liệu HW) |
|---|---|---|---|---|---|---|---|---|
| MAC | 910 | 395 | 276 | 454 | 99 | 364 | 62 | 184 |
| BB | 785 | 517 | 428 | 221 | 19 | 161 | 0 | 517 |
| other | 127 | 0 | 10 | 117 | 0 | 0 | 0 | 0 |
| RF | 112 | 73 | 48 | 33 | 3 | 51 | 0 | 73 |
| USB-local | 70 | 13 | 4 | 56 | 7 | 13 | 0 | 13 |
| SDIO-local | 35 | 0 | 4 | 26 | 0 | 0 | 0 | 0 |
| RF-direct | 32 | 0 | 9 | 23 | 0 | 0 | 0 | 0 |
| BB-pg | 24 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| USB3-PHY | 1 | 1 | 0 | 0 | 0 | 1 | 0 | 1 |
| **Tổng** | **2096** | **999** | **779** | **930** | **128** | **590** | **62** | **788** |

## 2. Phân bố trạng thái sử dụng

| Không gian | A | T | M | G | X | R | Z | D |
|---|---|---|---|---|---|---|---|---|
| MAC | 365 | 6 | 3 | 21 | 27 | 34 | 23 | 431 |
| BB | 501 | 0 | 5 | 11 | 45 | 2 | 73 | 148 |
| other | 0 | 0 | 0 | 0 | 8 | 2 | 17 | 100 |
| RF | 64 | 9 | 0 | 0 | 6 | 0 | 3 | 30 |
| USB-local | 13 | 0 | 0 | 0 | 1 | 0 | 0 | 56 |
| SDIO-local | 0 | 0 | 0 | 0 | 5 | 4 | 0 | 26 |
| RF-direct | 0 | 0 | 0 | 0 | 8 | 1 | 19 | 4 |
| BB-pg | 0 | 0 | 0 | 0 | 24 | 0 | 0 | 0 |
| USB3-PHY | 1 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |

## 3. Kiểm tra tài liệu

- Liên kết tương đối không tồn tại: **0**
- Ký tự điều khiển NUL/backspace trong .md: **0**

## 4. Diễn giải

- **S4 < live** không có nghĩa là thiếu bằng chứng: mọi địa chỉ live đều có danh sách truy cập đầy đủ (file:dòng, hàm, guard) trong phụ lục sinh tự động A01–A10 và A12. S4 chỉ đếm địa chỉ được **giải thích tay** trong 01–03, 05.01–05.17, 07, A11, A13. Phần lớn BB/RF chưa S4 là thanh ghi chỉ được nạp một lần từ bảng PHY_REG/RadioA/B (13 §2, 14 §2) — không có tên, field hay nơi dùng khác.
- **S6** (live nhưng source không định nghĩa field): BB/RF gần như toàn bộ — phydm dùng mask số trực tiếp; ý nghĩa bit chỉ có khi có comment trong source (đã trích ở 13/14/15) hoặc tài liệu Realtek (không có trong repository).
- Trạng thái R đã rà tay ở A13; X/Z đã kiểm chứng mẫu (PROGRESS: 5 ca dead + 6 ca sau cải tiến guard).

## 5. Vấn đề chưa xác minh (tự gom từ tài liệu tay)

### 5.1 Cần kiểm tra trên board (15)

- `05_groups\01_system_power_clock.md:5` — > Nhãn: [SOURCE] đọc trực tiếp; [SOURCE-SEQ] suy ra từ cách code dùng (ví dụ "ghi 1 rồi chờ về 0"); [INFERENCE]; [KIỂM TRA TRÊN BOARD]. Tên bit lấy từ `hal/halmac/halmac_bit_8822b.h` (section `/* 2 REG_xxx_8822B */`) — **tên bit là bằng chứng của header, không
- `05_groups\01_system_power_clock.md:53` — (St lấy từ `04a_catalog_mac.md`; A* = có điều kiện cut [KIỂM TRA TRÊN BOARD].)
- `05_groups\01_system_power_clock.md:288` — Lỗi: đọc `REG_SYS_CFG1` lúc MAC chưa lên nguồn → cut/ID sai → chọn sai nhánh power-seq (cut-mask) và sai bảng PHY. Driver in thông tin bằng `dump_chip_info()` — đó là dòng log nên kiểm tra đầu tiên khi bring-up [KIỂM TRA TRÊN BOARD].
- `05_groups\01_system_power_clock.md:298` — | 1 | Toàn bộ nhóm nằm trong `0x00–0xFF` = REG_ON_SEC → **mỗi** truy cập kéo theo 1 lần ghi `0x4E0` (03 §3.4). Một bước WRITE của pwr-seq = 4 control transfer (R8, ghi 0x4E0, W8, ghi 0x4E0). | Bật log USB (usbmon) [KIỂM TRA TRÊN BOARD] |
- `05_groups\04_dma_queue_trx.md:71` — (tính bằng `py -I tools/txff_sim.py <bulkout> 16 <mode>`; ghi W16 **xoá luôn bit 3:0** — chúng được đặt lại sau ở bước 9 và bởi `cfg_usb_rx_agg_88xx`/`rx_shift_88xx`.) Số bulk-out thật của board: [KIỂM TRA TRÊN BOARD] (`lsusb -v`, hoặc log `RtNumOutPipes`).
- `05_groups\04_dma_queue_trx.md:110` — Số page driver xin (`rtw_hal_get_rsvd_page_num`, `hal/hal_com.c:13908-13936`) = kết quả "chạy thử" `_rtw_hal_set_fw_rsvd_page(adapter, FALSE, &num)` + 3 (MCC) + 3 (MCC_V2) + 3 (MCC) — rồi làm tròn lên 8/16/24/32/64/128/256 (`_rsvd_page_num_drv2halmac`, `hal_ha
- `05_groups\04_dma_queue_trx.md:146` — → Hai thanh ghi trạng thái này là "chuông báo" chính dẫn đến reset ngầm. Khi thấy log reset, xem giá trị in ra để biết bit nào [KIỂM TRA TRÊN BOARD]; ý nghĩa từng bit của `TXDMA_STATUS`/`RXDMA_STATUS` có trong `halmac_bit_8822b.h` (section tương ứng) nhưng dri
- `05_groups\09_usb.md:52` — Board thật dùng tốc độ nào: [KIỂM TRA TRÊN BOARD] (đọc `0xFF` và `0xFE11`, hoặc `lsusb -t`).
- `05_groups\11_gpio_led.md:98` — | WoWLAN không đánh thức được host | GPIO6 có ở output/mức 1 sau init không: đọc `0x44–0x47` qua proc `read_reg` (`core/rtw_debug.c`) [KIỂM TRA TRÊN BOARD] |
- `05_groups\13_bb_phy.md:101` — [KIỂM TRA TRÊN BOARD] rfe_type thực tế được in qua `RTW_INFO("EEPROM rfe_type=0x%x")`. Toàn bộ bảng AGC, RFE pinmux (§7), CCA (`ccapar_by_rfe`) và RF RadioA/B (14) đều phụ thuộc giá trị này.
- `05_groups\14_rf.md:76` — [KIỂM TRA TRÊN BOARD]: đọc `0xF0[15:12]` và log `EEPROM rfe_type` để biết chính xác nhánh nào đang được dùng. Lệnh mô phỏng:
- `05_groups\15_calibration_txpower.md:108` — Mỗi lần áp dụng tốn 2 RMW × 2 path = **8 transfer**. Giá trị thermal và PG thật của board: [KIỂM TRA TRÊN BOARD]. PG đến từ eFuse (12); thermal đọc từ RF `0x42`.
- `05_groups\16_mp_debug.md:33` — - Source **không** khai báo thanh ghi nào như vậy, nên chưa xác định. [KIỂM TRA TRÊN BOARD] So sánh hai lần dump liên tiếp khi không có lưu lượng.
- `01_build_config.md:3` — > Nhãn: **[SOURCE]** = đã đọc đúng dòng code được dẫn; **[TOOL]** = kết quả của script trong `tools/` (tái chạy được); **[INFERENCE]** = suy luận, chưa có bằng chứng trực tiếp; **[KIỂM TRA TRÊN BOARD]** = chỉ biết được khi chạy trên máy thật.
- `05_groups\appendix\A11_mac_reg_table.md:3` — Nguồn: `hal/phydm/rtl8822b/halhwimg8822b_mac.c:144-268`; ghi bởi `odm_read_and_config_mp_8822b_mac_reg()` (cùng file, `:272-330`) → `odm_config_mac_8822b()` → `odm_write_1byte()` (ghi **cả byte**, không RMW). Bảng này không có header điều kiện (mọi mục đều ghi

### 5.2 Ý nghĩa chưa xác định từ source (20)

- `05_groups\01_system_power_clock.md:77` — Chỉ bit 0 đổi; 7 bit còn lại được ghi lại đúng giá trị vừa đọc. Theo `halmac_bit_8822b.h`, bit 16–17 của `REG_SYS_SWR_CTRL1` (= bit 0–1 của byte 0x12) nằm trong field `OCP_L1[17:15]` — tên là của header; source không giải thích vì sao pwr-seq thao tác riêng từ
- `05_groups\01_system_power_clock.md:89` — | 3 | :103 | `0xFF0A` | W | 0xFF → 0 | (USB-local, không tên) | — chưa xác định |
- `05_groups\01_system_power_clock.md:90` — | 4 | :108 | `0xFF0B` | W | 0xFF → 0 | (USB-local) | — chưa xác định |
- `05_groups\01_system_power_clock.md:114` — - đọc `REG_SW_MDIO + 3` (0x10C3) bit 0; nếu = 1 in `"[ALWAYS]shall R reg twice!!"` — nghĩa là khi `DIS_TIMEOUT_IO` = 1, driver được cảnh báo phải đọc thanh ghi 2 lần [SOURCE cho thông điệp; lý do phần cứng chưa xác định].
- `05_groups\01_system_power_clock.md:158` — **D. Ngữ nghĩa R/W.** Chưa xác định từ source hiện có; cần datasheet. Không có polling trên thanh ghi này.
- `05_groups\01_system_power_clock.md:193` — **D.** Chưa xác định quyền R/W, giá trị reset; cần datasheet. Không có polling.
- `05_groups\01_system_power_clock.md:227` — pwr-seq #5/#6 (byte 0x12). `phydm_cfo_tracking_init()` (`phydm_cfotracking.c:407`) với guard `support_ic_type & (8822B|8821C)` = đúng: `odm_set_mac_reg(dm, R_0x10, 0x40, 0x1)` → RMW 32-bit đặt **bit 6** — header **không có tên** cho bit 6 [TOOL]. Ý nghĩa chưa 
- `05_groups\01_system_power_clock.md:284` — Phép tính VendorType [tính tay]: `BIT_GET_VENDOR_ID_8822B(v) = (v >> 16) & 0xF`; driver dịch thêm `>> 2` → chỉ lấy 2 bit cao của field (bit 19:18). [SOURCE cho phép toán; ý nghĩa của 2 bit thấp chưa xác định].
- `05_groups\01_system_power_clock.md:292` — `0x0020 REG_AFE_LDO_CTRL` (bit 0 `POW_PLL_LDO`, bit 3 `EX_XTAL_DRV_RF1`), `0x0090/0x0093 REG_WLLPS_CTRL`, `0x00AF` (`REG_AFE_CTRL7.SEL_LDO_PC[29]`), `0x10A8–0x10AA REG_NFCPAD_CTRL` (C-cut), `0x10C3` (`REG_SW_MDIO.DIS_TIMEOUT_IO[24]`): driver không đụng tới ngo
- `05_groups\06_edca_beacon_tsf.md:49` — **Ghi đè sau đó**: lớp driver gọi lại `InitBeaconParameters()` (`hal/rtl8822b/rtl8822b_ops.c:773-805`) với hằng **khác**: `DRVERLYINT = DRIVER_EARLY_INT_TIME_8822B = 0x05` (comment "5ms"), `BCNDMATIM = 0x02` ("2ms"), `TBTT_PROHIBIT` setup `0x04` và hold `TBTT_
- `05_groups\06_edca_beacon_tsf.md:86` — **Reset**: ghi `REG_DUAL_TSF_RST` = `BIT0` (port 0) hoặc `BIT1` (port 1) — join type 0 (bắt đầu kết nối), disconnect, `set_beacon_related_registers`. Trong `set_beacon_related_registers` (`:871-878`) có **2 lần ghi liên tiếp**: lần 1 theo port, lần 2 luôn `BIT
- `05_groups\10_power_mgmt.md:54` — `check_ips_status()` đọc `0x100` (`REG_CR`) và `0x86` (byte 2 của `REG_MCU_TST_CFG_8822B` 0x84 theo bố cục header — code dùng literal, không có tên) để log, rồi `0x100 == 0xEA` ⇒ "đã vào IPS" [SOURCE]. Cùng quy ước `0xEA` với `mac_pwr_switch_usb_8822b`. Ý nghĩ
- `05_groups\11_gpio_led.md:65` — | 1 | `GPIO8_WL_LED_8822B` (`:290-292`) | `0x4E` | `BIT5` | `BIT5` | ghi `value` → **bật** LED, dừng | `LED2DIS[21]` = 1 (tên header; ý nghĩa "DIS" ngược trực giác → chưa xác định) |
- `05_groups\13_bb_phy.md:68` — - Giá trị ghi vào `0x1B80` đi theo cặp `…5` / `…7`, ví dụ `0x090A0005`, `0x090A0007`. [INFERENCE] Đây trông như một cổng nạp lệnh với bit 1 làm strobe, nhưng source không có comment hay tên nào cho `0x1B80`, nên **ý nghĩa: không xác định**.
- `05_groups\14_rf.md:115` — Source **không** cho biết mỗi bit của `0xEF` ứng với bảng nào; ý nghĩa nội dung bảng chưa xác định.
- `05_groups\16_mp_debug.md:55` — | BB | `0x181C`, `0x1A1C` | `rC/rD_TxScale_Jaguar2` | `mpt_SetSingleTone_8814A` (`hal/hal_mp.c:913-…`, biên dịch cho cả 8822B theo `#if … || defined(CONFIG_RTL8822B)`) | được gọi **không điều kiện IC** từ đường PMAC TX của MP (`:2362`, `:2527`): lưu `0xC1C/0xE
- `05_groups\17_misc_bf_btc.md:106` — Thêm 300+ thanh ghi BB khác (`L0 tab1` trong A12 / danh mục) **chỉ** được ghi một lần qua PHY_REG (13 §2). Driver không chạm lại chúng, và header 8822B không có tên hay field cho phần lớn số đó. Ý nghĩa của chúng: chưa xác định.
- `02_address_spaces.md:30` — Dải MAC lấy trực tiếp từ giá trị lớn/nhỏ nhất của 714 macro trong `hal/halmac/halmac_reg_8822b.h` (hai cụm `0x0000–0x07FE` và `0x1000–0x1708`) [TOOL]. Dải BB lấy từ các macro phydm và từ việc `rtl8822b_read/write_bb_reg` dùng cùng `rtw_read32/rtw_write32` (`ha
- `03_access_architecture.md:156` — - `phy_calculatebitshift()` duyệt bit 0→31, mask = 0 → shift = 32 (dịch 32 bit là hành vi không xác định trong C) — chỉ là rủi ro nếu ai đó gọi với mask 0.
- `03_access_architecture.md:157` — - `odm_set_mac_reg()` cũng gọi `phy_set_bb_reg()` (`phydm_interface.c:252`) → field 8-bit của thanh ghi MAC cũng bị ghi lại cả 32 bit. Nếu các byte khác chứa bit "ghi 1 để xoá" hoặc bit tự xoá, việc ghi lại giá trị vừa đọc **có thể** gây tác dụng phụ — chưa xá

