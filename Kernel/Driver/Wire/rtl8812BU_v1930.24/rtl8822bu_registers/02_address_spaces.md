# 02 — Các không gian địa chỉ và cách phân loại

> Nhãn như `01_build_config.md`. Số liệu [TOOL] lấy từ `work/catalog.json` (tái tạo bằng `tools/report.py`).

## 2.1 Ý tưởng cốt lõi: một "bus" 16-bit, nhiều khối phần cứng

Với RTL8822BU qua USB, CPU (host Linux) **không** có MMIO tới chip. Mọi thanh ghi mà host chạm tới đều đi qua **một** kênh: USB control transfer `bRequest = 0x05`, `wValue = địa chỉ 16-bit` (`hal/hal_hci/hal_usb.c:372-376`, `os_dep/linux/usb_ops_linux.c:142-154`) [SOURCE]. Phía chip, bộ giải mã địa chỉ chia không gian 16-bit đó cho nhiều khối:

```
 wValue (16 bit)        Khối phần cứng — tên khối suy từ tên macro trong dải [INFERENCE]   Nguồn xác định dải
 0x0000 ───────────────┐
        System/Power   │ MAC "page 0"      SYS_ISO/FUNC_EN/CLK/AFE/EFUSE/GPIO…   halmac_reg_8822b.h:60…
 0x0100 MAC core/FW/INT│ (MAC)             CR, HIMR/HISR, H2C/C2H mailbox…
 0x0200 TXDMA/RQPN     │                   FIFO page, queue
 0x0400 PTCL/EDCA/TSF  │                   EDCA, beacon, TSF, retry
 0x0600 WMAC/Security  │                   RCR, MAC/BSSID, CAM
 0x07FF ───────────────┘
 0x0800 ───────────────┐ BB (PHY)          AGC, CCA, TXAGC, RFE, LSSI…         phydm (R_0x800…), rtl8822b_hal.h:116…
 0x0FFF ───────────────┘
 0x1000 ───────────────┐ MAC mở rộng       FW page/DDMA/IDMA, beamforming, MU…  halmac_reg_8822b.h (0x1000–0x1708)
 0x17FF ───────────────┘
 0x1800 ───────────────┐ BB mở rộng        CCX/NHM/CLM, TXAGC table 0x1D00…     phydm (R_0x1988, 0x1D00…)
 0x1FFF ───────────────┘
 0x2800 ───────────────┐ Cửa sổ đọc RF     RF-A: 0x2800+addr*4, RF-B: 0x2C00+…  phydm_hal_api8822b.c:1000
 0x2FFF ───────────────┘
 0xFE00 ───────────────┐ Khối USB của chip  REG_USB_* (STAT, HRPWM/HCPWM, PHY…)  halmac_reg2.h, rtl8822bu.h:66-67
 0xFFFF ───────────────┘
```

Dải MAC lấy trực tiếp từ giá trị lớn/nhỏ nhất của 714 macro trong `hal/halmac/halmac_reg_8822b.h` (hai cụm `0x0000–0x07FE` và `0x1000–0x1708`) [TOOL]. Dải BB lấy từ các macro phydm và từ việc `rtl8822b_read/write_bb_reg` dùng cùng `rtw_read32/rtw_write32` (`hal/rtl8822b/rtl8822b_phy.c:652-685`) [SOURCE]. Ranh giới chính xác bên trong chip (ví dụ 0x0800 thuộc BB từ byte nào) **chưa xác định từ source** — cần datasheet; ở đây chỉ dùng như quy ước phân loại.

`usbctrl_vendorreq` tự phân 3 "section" theo `wValue` (`usb_ops_linux.c:199-209`) [SOURCE]:

| Section | Điều kiện | Hành vi thêm |
|---|---|---|
| `REG_ON_SEC` | `0x00–0xFF` hoặc `0x1000–0x10FF` | sau truy cập ghi thêm 1 byte vào `0x4E0` |
| `REG_OFF_SEC` | còn lại `< 0xFE00` | — |
| `REG_LOCAL_SEC` | `≥ 0xFE00` | — |

[INFERENCE] Tên "ON/OFF" gợi ý 2 miền nguồn (always-on vs. power-off domain) của chip; source không nói lý do của lần ghi 0x4E0 (xem 09 §9.4 #1).

## 2.2 Bảng không gian địa chỉ

| Không gian | Bản chất | Cách host truy cập | Độ rộng địa chỉ / dữ liệu | Số địa chỉ trong danh mục (A = dùng thật) [TOOL] |
|---|---|---|---|---|
| **MAC** | Thanh ghi điều khiển MAC, hệ thống, nguồn, DMA, FW, ngắt, security… | `rtw_read/write8/16/32` trực tiếp, hoặc `HALMAC_REG_R*/W*` (halmac) → cùng đường | 16-bit / 8,16,32 | 910 (A 365, T 6) |
| **BB** | Thanh ghi baseband (PHY số) | `phy_set_bb_reg` / `odm_set_bb_reg` → `rtw_read32` + `rtw_write32` (RMW nếu có mask) | 16-bit / luôn 32 ở tầng bus | 968 gồm BB 785, RF-direct 32, BB-pg 24, other 127 (BB: A 501) |
| **RF** | Thanh ghi trong chip RF (transceiver analog), **mỗi path một bộ** | **Gián tiếp qua BB**: ghi qua `0xC90/0xE90` (LSSI), đọc qua cửa sổ `0x2800/0x2C00` | 8-bit (0x00–0xFF) / 20-bit | 112 (A 64, T 9), path A và B |
| **USB-local** | Khối USB device của chip | `rtw_read/write8` tới `0xFE00–0xFFFF` | 16-bit / 8 | 70 địa chỉ, 60 tên; 13 địa chỉ được dùng (A) |
| **USB3-PHY** | Thanh ghi PHY USB3 | Gián tiếp: `usbphy_write_88xx` ghi `0xFF0D/0xFF0E` (dữ liệu) + `0xFF0C` (địa chỉ) | 8-bit / 16 | 1 (`0x01 = 0xA841`, chỉ cut D) |
| **LTE-coex (gián tiếp)** | Thanh ghi của khối đồng tồn tại LTE | Cửa sổ `0x1700` (ctrl: `LTECOEX_ACCESS_START[31]`, `WRITE_MODE[30]`, `READY[29]`, `WRITE_BYTE_EN[19:16]`, offset[15:0]), `0x1704` (data ghi), `0x1708` (data đọc) — `ltecoex_reg_read/write_88xx` | 16-bit / 32 | chỉ 1 offset dùng: `0x38` — backup/restore quanh tải FW (05.08 §5) **và** ghi `0x7700` trong `ex_hal8822b_wifi_only_hw_config` (comment "gnt_wl=1 , gnt_bt=0", ghi thẳng `0x1704`/`0x1700` không chờ READY — 05.17 §2) |
| **Packet buffer (gián tiếp, cửa sổ trang)** | Bộ nhớ đệm trong chip: TX FIFO/reserved page, RX FIFO, report, LLT, RX buffer của FW | `REG_PKTBUF_DBG_CTRL` (0x140)`[11:0]` = trang (TX/rsvd `0x780+`, RX `0x700+`, report `0x660+`, LLT `0x650+`, RXBUF_FW `0x680+`; giữ `[15:12]`), rồi `R32(0x8000 + off)` với `off` 0…0xFFF (4 KB/trang) — `read_buf_88xx` (`halmac_common_88xx.c:2170-2218`), `phydm_la_access_tx_pkt_buf` | trang 12-bit + offset 12-bit / 32 | chỉ đọc; dùng ở debug: proc `dump_fifo`/`rsvd_page`, ioctl, `rtw_hal_get_rsvd_page`, LA mode (05.16, 05.17 §3). Danh mục xếp `0x8000` là `other`/R (truy cập qua biến vòng lặp) |
| **SDIO-local** | Thanh ghi khối SDIO (`0x1025xxxx`) | Không dùng qua USB — chỉ có định nghĩa và vài nhánh SDIO đã chết | — | 35 (A 0) |
| **eFuse/OTP** | Bộ nhớ một lần ghi chứa MAC address, công suất, RFE type… | Gián tiếp qua `REG_EFUSE_CTRL` (0x30) + `REG_EFUSE_*`; hoặc file map (`CONFIG_EFUSE_CONFIG_FILE`) | offset riêng | (là *dữ liệu*, không phải thanh ghi — xem 2.4) |
| **FW (H2C/C2H)** | "Thanh ghi ảo" do firmware 8051 trong chip diễn giải | H2C qua mailbox `REG_HMEBOX0..3` (0x1D0…) + `REG_HMEBOX_E0..` (0x1F0…), hoặc gói H2C qua bulk-out | — | (là *lệnh*, không phải thanh ghi) |

## 2.3 Cùng con số, khác thanh ghi — các va chạm thực sự có trong code

Có **13 con số** vừa là địa chỉ MAC đang dùng (A) vừa là địa chỉ RF đang dùng (A) [TOOL]. Ví dụ (kèm 2 trường hợp chỉ một bên được dùng):

| Số | Là MAC… | Là RF… | Vì sao không bao giờ lẫn trong phần cứng |
|---|---|---|---|
| `0x00` | `REG_SYS_ISO_CTRL_8822B` (cách ly nguồn) | `RF_0x00` — mode/gain của path (`ODM_GAIN_SETTING`) | RF 0x00 chỉ tới được qua LSSI `0xC90/0xE90` hoặc `0x2800/0x2C00` |
| `0x08` | `REG_SYS_CLK_CTRL_8822B` | `RF_0x08` | như trên |
| `0x18` | `REG_SYS_SWR_CTRL3_8822B` (chỉ định nghĩa — D) | `RF_0x18` — channel/bandwidth (`ODM_CHANNEL`, `phydm_hal_api8822b.c:710`) | như trên |
| `0x24` | `REG_AFE_CTRL1_8822B` (alias `REG_AFE_XTAL_CTRL`, `rtl8822b_hal.h:74`) | `RF_T_METER_OLD` — nhiệt kế (`phydm_reg.h:77`) | như trên |
| `0x30` | `REG_EFUSE_CTRL_8822B` | `RF_0x30` / `rf_mode_table_addr` | như trên |
| `0x42` | không có tên MAC 8822B; chỉ xuất hiện trong bảng pinmux GPIO4–7 (T) | `RF_T_METER_NEW` — nhiệt kế (`phydm_reg.h:78`) | như trên |

Quy tắc đọc code: **không gian được quyết định bởi hàm truy cập, không phải bởi con số**. `odm_get_rf_reg(dm, RF_PATH_A, 0x18, …)` là RF; `rtw_read8(adapter, 0x18)` là MAC. Danh mục 04 xếp mỗi địa chỉ theo hàm thực sự dùng nó.

## 2.4 Những thứ dễ nhầm là "thanh ghi"

| Loại | Ví dụ trong build | Có phải thanh ghi phần cứng? | Cách nhận biết |
|---|---|---|---|
| Field/bit mask | `BIT_FEN_CPUEN_8822B` = BIT(2), `BIT_SHIFT_/BIT_MASK_/BIT_GET_/BIT_SET_/BIT_CLEAR_` (`halmac_bit_8822b.h`, 9 790 define) | Không — mô tả vị trí bit *trong* một thanh ghi | Section comment `/* 2 REG_xxx_8822B */` cho biết thuộc thanh ghi nào |
| Tên theo địa chỉ | `R_0x1988`, `RF_0x18` (`phydm_regtable.h`, 942 tên) | Là địa chỉ, nhưng **không mang nghĩa** | Phải tra ngược ra tên chức năng (nếu có) |
| Địa chỉ gián tiếp | RF addr trong `[27:20]` của word ghi vào `0xC90`; địa chỉ USB3-PHY ghi vào `0xFF0C` | Là địa chỉ ở **không gian khác** | Xem hàm đóng gói (`config_phydm_write_rf_reg_8822b`, `usbphy_write_88xx`) |
| Giá trị trong bảng | `array_mp_8822b_phy_reg[] = {0x800, 0x9020D010, …}` | Cặp (địa chỉ, giá trị) sẽ được **ghi** | Có header điều kiện (bit31/30 set) theo RFE/cut/package |
| Bảng công suất | `array_mp_8822b_phy_reg_pg*` (6-tuple) | **Không ghi trực tiếp** — `phy_store_tx_power_by_rate()` lưu vào bảng phần mềm; địa chỉ chỉ là *khóa* kiểu Jaguar cũ | `odm_config_bb_phy_reg_pg_8822b`, `phydm_regconfig8822b.c:210-228` |
| Địa chỉ giả (pseudo) | BB table `0xF9–0xFE` = delay 1 µs…50 ms; RF table `0xFE/0xFFE` = delay | Không — lệnh chờ | `odm_config_bb_phy_8822b` `:270-287`, `odm_config_rf_reg_8822b` `:99-116` |
| eFuse/OTP | `EEPROM_*`, `PPG_*` offsets (`include/hal_pg.h`, 580 define) | Không — offset trong bộ nhớ cấu hình | Đọc qua `rtw_efuse`/halmac efuse, rồi lưu vào `hal_data` |
| Descriptor | `SET_TX_DESC_*`/`GET_RX_DESC_*` (`halmac_tx_desc_nic.h`, `halmac_rx_desc_nic.h`) | Không — field trong **gói dữ liệu** đi kèm frame qua bulk | Thao tác trên buffer RAM, không qua `rtw_write*` |
| H2C/C2H | `CMD_ID_*`, `H2C_*` (`halmac_original_h2c_nic.h`, `halmac_fw_offload_h2c_nic.h`, `hal_com_h2c.h`) | Không — lệnh/sự kiện firmware | Gửi qua mailbox hoặc gói H2C |
| HW_VAR | `HW_VAR_BSSID`, `HW_VAR_RCR`… (`rtw_hal_set_hwreg`) | Không — "biến HAL" trừu tượng; *bên trong* hàm set_hwreg mới ghi thanh ghi | `rtl8822b_sethwreg` (`rtl8822b_ops.c`) |
| Pinmux list | `GPIO0_BT_GPIO0_8822B = {HALMAC_BT_GPIO, HALMAC_GPIO0, …, 0x66, BIT(2), BIT(2)}` | Chứa địa chỉ MAC thật (0x66…) để RMW | `halmac_gpio_8822b.c:64…` (88 mục) |
| Hằng số trùng tiền tố | `RX_SMOOTH_FACTOR` (=20), `RXERR_TYPE_*`, `REG_ON_SEC` | Không | Bị loại khỏi danh mục |

## 2.5 Header định nghĩa thanh ghi — cái nào thật sự được dùng cho 8822B

| Header | Vai trò | Tên active / có dùng [TOOL] | Ghi chú |
|---|---|---:|---|
| `hal/halmac/halmac_reg_8822b.h` | Địa chỉ MAC của **chip 8822B** | 710 / 132 | Nguồn tin cậy nhất cho MAC 8822B |
| `hal/halmac/halmac_reg2.h` | Địa chỉ MAC "chung" (không hậu tố), nhiều khối `#if HALMAC_xxxx_SUPPORT` | 798 / 182 | 710 tên trùng 100% với `_8822B` → alias |
| `hal/halmac/halmac_bit_8822b.h`, `halmac_bit2.h` | Bit/field | 19 035 (macro bit) | Section comment ánh xạ bit → thanh ghi |
| `include/rtl8822b_hal.h:74-219` | Tên kiểu cũ cho 8822B (`rFPGA0_*`, `rA_LSSIWrite_Jaguar`=0xC90, `MSR`…) **có comment mục đích** | 94 / 69 | Comment kiểu "RF write addr, LSSI Parameter (rtl8822b_phy.c)" là bằng chứng chức năng |
| `hal/rtl8822b/usb/rtl8822bu.h:66-67` | `REG_USB_HRPWM_8822B` 0xFE58, `REG_USB_HCPWM_8822B` 0xFE57 | 2 / 2 | Power-mode handshake host↔FW |
| `hal/phydm/phydm_regtable.h` | `R_0x…`, `RF_0x…` | 942 / 292 | Tên theo số |
| `hal/phydm/phydm_regdefine11ac.h` | `ODM_REG_*_11AC` — chọn qua `ODM_REG(x, dm)` | 48 / 18 | 8822B ∉ `ODM_IC_11N_SERIES` → luôn `_11AC` (`phydm_interface.h:163-191`) |
| `hal/phydm/phydm_regdefine11n.h` | `ODM_REG_*_11N` | 160 / 1 | Gần như chết với 8822B |
| `hal/phydm/phydm_reg.h` | `ODM_*` chung, có section MAC/BB/RF | 74 / 2 | Có đặt nhầm RF vào section MAC (09 §9.4 #4) |
| `include/hal_phy_reg.h` | Alias `REG_* → r*` | 188 / 2 | 144 alias treo (đích không tồn tại) |
| `include/hal_com_reg.h` | Khối `REG_*` (dòng 70–739) | 0 active | `#ifndef RTW_HALMAC` → loại với 8822B; phần còn lại là bit kiểu cũ (`IMR_*`, `RCR_*`…) |
| `Hal8812PhyReg.h` | Thanh ghi PHY 8812A | không vào build | chỉ qua `rtl8812a_hal.h` |
