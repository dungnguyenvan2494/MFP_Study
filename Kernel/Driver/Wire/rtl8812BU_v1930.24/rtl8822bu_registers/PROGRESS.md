# Tiến độ: Phân tích toàn bộ thanh ghi driver rtl8812BU_v1930.24

Driver: `Kernel/K-S800/Src/drivers/net/wireless/realtek/rtl8812BU_v1930.24` (gọi tắt `$D`)
Yêu cầu gốc: prompt 11 mục (lượt 1–6, định dạng 12 phần) — xem `00_REQUEST.md`.

## Công cụ (docs/rtl8822bu_registers/tools)
| Script | Việc | Lệnh |
|---|---|---|
| `mk_eval.py` | Đánh giá Makefile → EXTRA_CFLAGS + danh sách .o | `py -I mk_eval.py $D > work/build.json` |
| `cpp_mask.py` | Tiền xử lý chỉ-directive → dòng nào được biên dịch | `py -I cpp_mask.py $D work/build.json <Eagle_defconfig> work/mask.json` (~100 s) |
| `show.py` | In source kèm cờ `+` active / `-` inactive | `py -I show.py $D work/mask.json file:a-b` |

Giới hạn mask: header kernel (`<linux/...>`) không được parse → macro chỉ định nghĩa trong kernel header coi như undefined (trừ seed trong cpp_mask.py).

## Lượt 1 — Cấu hình build (XONG)
- Module `8812bu`, 171 TU, chip **RTL8822B** + **USB** (CONFIG_RTL8822B=y, USB_HCI=y; 8812A=n, PCI=n, SDIO=n).
- Tên thư mục "8812BU" ≠ chip HAL 8822B: Makefile `MODULE_NAME = 8812bu` khi `CONFIG_BT_COEXIST = n` (88x2bu nếu có BT). RTL8812BU = biến thể 8822B không BT → dùng HAL rtl8822b.
- Kernel 5.10.241, 14/18 defconfig KM giống nhau ở các symbol liên quan → dùng `Eagle_defconfig` (CONFIG_RTL8822BU=m).
- Flag đáng chú ý: MP_INCLUDED, POWER_SAVING, USB_AUTOSUSPEND, WOWLAN(+GPIO_WAKEUP), CONCURRENT_MODE, MCC_MODE, EFUSE_CONFIG_FILE, LOAD_PHY_PARA_FROM_FILE, RTW_DEBUG, PROC_DEBUG; BT_COEXIST=n.
- `DBG_IO` nằm trong comment (`include/autoconf.h:404`) → `rtw_read8` = `_rtw_read8` (`include/rtw_io.h:459`).
- Header thanh ghi MAC: `hal/halmac/halmac_reg_8822b.h` (714 REG_), `halmac_bit_8822b.h` (9790 define), cộng `halmac_reg2.h`/`halmac_bit2.h` (chung, không hậu tố chip) qua `halmac_api.h:81-98`; `include/hal_com_reg.h`, `include/hal_phy_reg.h` qua `hal_com.h:63-64`. `Hal8812PhyReg.h` chỉ qua `rtl8812a_hal.h` (không build).

## Lượt 1b — Đường truy cập MAC qua USB (đã đọc source)
`rtw_read32()` → `_rtw_read32()` (`core/rtw_io.c:141`, **KM thêm mutex `halmac_reg_mutex`** `:93`, comment SJIS "AR.1072290") → `io_ops._read32` = `usb_read32()` (`hal/hal_hci/hal_usb.c:423`; gán bởi `usb_set_intf_ops` `:576`, gọi từ `usb_intf.c:1412`; `rtl8822bu_set_intf_ops` có nhưng KHÔNG được gọi) → `usbctrl_vendorreq()` (`os_dep/linux/usb_ops_linux.c:71`): bRequest 0x05, wValue = addr&0xFFFF, wIndex 0, retry ≤ MAX_USBCTRL_VENDORREQ_TIMES (trừ addr ≥ FW_START_ADDRESS), sau mỗi truy cập vùng 0x00–0xFF / 0x1000–0x10FF ghi thêm 1 byte vào **0x4E0** (8822B/21C/22C).
Lỗi: -ESHUTDOWN/-ENODEV → surprise_removed; khác → Wifi_Error_Status=USB_VEN_REQ_CMD_FAIL; continual_io_error > MAX → surprise_removed.

## Lượt 1c — Đường HALMAC / BB / RF (XONG, đã đọc source)
- HALMAC: `HALMAC_REG_R8()` (`halmac_type.h:256`, nhánh HALMAC_DBG_MONITOR_IO tắt) → `api->halmac_reg_read_8` = `reg_r8_usb_88xx` (`halmac_init_88xx.c:399`, `halmac_usb_88xx.c:201`) → `PLTFM_REG_R8` (`halmac_type.h:139`) → `pltfm_api->REG_READ_8` = `_halmac_reg_read_8` (`hal_halmac.c:350/870`) → `rtw_read8(primary adapter)`.
- BB: `odm_set_bb_reg` (`phydm_interface.c:281`) → `phy_set_bb_reg` macro (`hal_intf.h:810`) → `rtw_hal_write_bbreg` (`hal_intf.c:914`) → `hal_func.write_bbreg` = `rtl8822b_write_bb_reg` (`rtl8822b_ops.c:3997`, `rtl8822b_phy.c:668`): mask≠0xFFFFFFFF → read32+RMW, rồi `rtw_write32`. Đọc: `rtl8822b_read_bb_reg` `:652`. **BB = cùng bus 16-bit với MAC**.
- RF (`config_phydm_*_rf_reg_8822b`, `phydm_hal_api8822b.c:995/1024`): đọc = BB read tại `0x2800`(A)/`0x2C00`(B) + (addr&0xFF)<<2, mask&=0xFFFFF; ghi = BB write32 tại `0xC90`(A)/`0xE90`(B) giá trị `(addr<<20)|(data&0xFFFFF)` (LSSI); mask≠0xFFFFF → đọc trước + `phydm_check_bit_mask`. Path > B → lỗi. TXAGC: BB `0x1D00`/`0x1D80`+rate.
- Đường thứ 4 `phydm_set_reg_by_fw` (H2C) chỉ khi `PHYDM_PHY_PARAM_OFFLOAD` — **không bật** (setter `rtw_hal_update_param_init_fw_offload_cap` nằm trong `CONFIG_FW_OFFLOAD_PARAM_INIT` tắt, `hal_dm.c:198`).
- **IQK offload BẬT mặc định**: `RTW_IQK_FW_OFFLOAD` (`hal_ic_cfg.h:297`) → `rtw_iqk_fw_offload=1` (`os_intfs.c:941`) → `phy_iq_calibrate_8822b` đi nhánh FW (`halrf_iqk_8822b.c:1856`).
- 0x4E0 (= `REG_NULL_PKT_STATUS_V1_8822B`) bị ghi thêm 1 byte sau mỗi truy cập vùng 0x00–0xFF/0x1000–0x10FF — lý do không có comment → chưa xác định.
- `include/hal_com_reg.h:70-739` (REG_*) nằm trong `#ifndef RTW_HALMAC` → không active; tên MAC lấy từ `halmac_reg2.h` + `halmac_reg_8822b.h`.
- Vùng bus 8822B (theo `halmac_reg_8822b.h`): MAC 0x0000–0x07FE, 0x1000–0x1708; BB 0x0800–0x0FFF, 0x1800–0x1FFF (+0x2800–0x2FFF RF-direct); USB-local 0xFE00–0xFFFF (`usbctrl_vendorreq` REG_LOCAL_SEC); SDIO-local 0x1025xxxx (không dùng qua USB).

## Lượt 2–4 — Công cụ kiểm kê (XONG bản 1)
`tools/inventory.py` (+`guard.py`) → `work/{regs,bits,access,numeric,callgraph}.json` (~12 s):
- regs=3016 macro thanh ghi active (thêm `include/rtl8822b_hal.h`, `hal/rtl8822b/usb/rtl8822bu.h`); bits=19035; access=3672 (gồm 108 địa chỉ giải từ mảng cục bộ `backup_bb_reg[] = {…}`), 2025 địa chỉ literal/R_0x….
- Guard lúc chạy: chip/interface + early-return (`if (!(ic & X)) return;`) + `else`-chain + `switch/case` + cấu hình mặc định (fw_offload_ability, mp_mode). `ODM_REG(x,dm)` → `_11AC`. Enum được đọc (ODM_RTL8822B=BIT7…).
- Đã kiểm chứng tay 5 ca dead (`phydm_phystatus.c:3173`, `hal_com.c:8360/12225/5189/16932`) + early-return `phydm_api.c:2002`.
- `tools/tables.py` → `work/tables.json`: MAC_REG 125, PHY_REG 1492, AGC 9839, RadioA 3944, RadioB 3665 (có điều kiện RFE), PHY_REG_PG×10 (lưu, không ghi), power-seq 72 bước (4 chuỗi active), USB3-PHY 1, pinmux 88.
- `tools/report.py` → `work/catalog.{json,csv}` + `04a–04d`. Trạng thái A/T/M/G/X/R/Z/D (định nghĩa trong `04_catalog_README.md`).
  - (số liệu hiện hành, sau cải tiến reachability phiên 2–3)
  - MAC 910: A 365, T 6, M 3, G 21, X 27, R 34, Z 23, D 431 (sau khi thêm accessor `pltfm_api->REG_READ_*`: `0xFC` R→A, `0xF1` mới)
  - BB 785: A 501, M 5, G 11, R 2, X 45, Z 73, D 148; RF-direct 32 (X 8, Z 19); BB-pg 24 (X); other 127
  - RF 112: A 64, T 9, X 6, Z 3, D 30; USB-local 70 (A 13); USB3-PHY 1 (A); SDIO-local 35 (không A)
- `tools/report_extra.py` → `08_direct_numeric.md` (1349 truy cập số live), `09_aliases_unused.md` (**25** điểm bất thường có bằng chứng — mục 9.4, nguồn là danh sách trong `report_extra.py`), `10_index_by_address.md`, `11_index_by_name.md`.

## Tài liệu đã viết (tay)
- `README.md` (mục lục, ánh xạ 12 phần), `01_build_config.md` (Phần 1), `02_address_spaces.md` (Phần 2 + 3.5; 9 không gian gián tiếp), `03_access_architecture.md` (Phần 3 + 7, Mermaid), `04_catalog_README.md`, `07_sequences.md` (Phần 6), `12_completion.md` (Lượt 6, sinh tự động).

## Lượt 5 — nhóm đã viết
| Nhóm | File | Ghi chú |
|---|---|---|
| 01 System/Power/Clock/ChipID | `05_groups/01_system_power_clock.md` | power-on 25 bước / off 18 bước cho USB, giải bit bằng `tools/bitname.py` |
| 11 GPIO/LED/pinmux | `05_groups/11_gpio_led.md` | GPIO6 wake, LED SW mode, thuật toán pinmux ưu tiên |
| 12 eFuse | `05_groups/12_efuse.md` | giao thức EF_FLAG, unlock 0x69, LDO25 |
| 08 Firmware/H2C/C2H | `05_groups/08_firmware.md` | tải FW (rsvd page→DDMA→checksum→0xC078), mailbox 0x1CC/0x1D0/0x1F0, H2C ring 0x10D0/D4, **không gian LTE-coex gián tiếp 0x1700** |
| 03 Interrupt | `05_groups/03_interrupt.md` | USB: không có ngắt host (INT-IN pipe & SUPPORT_USB_INT tắt); HIMR chỉ trong hàm không có nơi gọi |
| 04/05 DMA/FIFO/Queue | `05_groups/04_dma_queue_trx.md` | mô phỏng `tools/txff_sim.py` (2048 page, PQ_MAP theo bulk-out), H2C ring, RX DMA pause, TX/RXDMA_STATUS → silent reset |
| 09 USB | `05_groups/09_usb.md` | U2/U3 detect (0xFF==0x20, 0xFE11), burst 0x290, PHY gián tiếp FE40/FF0C, RX agg 0x280 |
| 02 WMAC/RX filter | `05_groups/02_wmac_rx_filter.md` | RCR 0xE400220E, RXFLTMAP theo MLME, retry 0x3030, MACID/BSSID theo port |
| 06 EDCA/Beacon/TSF | `05_groups/06_edca_beacon_tsf.md` | SIFS 0x10100E0A, TBTT 0x6404, TXOP, TSF đọc cao→thấp |
| 07 Security | `05_groups/07_security.md` | CAM gián tiếp 0x670/674/678, entry 8 dword ghi j=7..0, giới hạn u8 ↔ SW limit 32, SECCFG |
| 10 PS/LPS/WoW | `05_groups/10_power_mgmt.md` | RPWM/CPWM `& 0xC1`, toggle bit7, thứ tự bước WoW |
| 13 BB/PHY | `05_groups/13_bb_phy.md` | PHY_REG 1492 (1007 entry cuối = `cal_init` bị `#if 0`), AGC 8 bảng con/rfe, kênh/BW/band (`chbw_sim.py`), DIG, FA counters (lỗi ưu tiên toán tử), TRX path, RFE pinmux, CCX |
| 14 RF | `05_groups/14_rf.md` | đường đọc/ghi + che `&0xFF`, RadioA/B theo rfe/cut (`rftab_sim.py`), LUT 0xEF/0x33/0x3E/0x3F, LCK (chỉ MP/debug), AAC, thermal 0x42, PA bias/kfree, bắt tay IQK RF 0x08 = 0xABCDE |
| 15 Calibration/TXPWR | `05_groups/15_calibration_txpower.md` | TXAGC 4-byte gom (`txagc_sim.py`, rò buffer ở MP), power tracking MIX_MODE 0xC94[29:25]/0xC1C[31:21] (`pwrtrk_sim.py`, bảng delta thật theo rfe), IQK FW + RF 0x08 handshake, trim eFuse **vật lý** 0x3D5–0x3EF, crystal cap 0x24[30:25]/0x28[6:1] |
| 16 MP/Debug | `05_groups/16_mp_debug.md` | proc write_reg/read_reg (không kiểm địa chỉ, chuỗi không kết thúc NUL), dump MAC/BB/RF (1152/1024/512 transfer), ioctl MP (chặn >0x3FFF), phydm `odm/cmd`; bảng M/G; MP single-tone ghi 0x181C/0x1A1C (path C/D của 8814A) |
| 17 Khác | `05_groups/17_misc_bf_btc.md` + `appendix/A12_uncovered_live.md` | BF/MU (bf_init 7 thanh ghi, SU/MU config, field MU định nghĩa cục bộ), wifi-only BTC (ghi LTE-coex 0x38 = gnt_wl/gnt_bt không chờ READY), LA/ADC + **cửa sổ packet buffer 0x140/0x8000** (không gian gián tiếp thứ 9), phydm dynamic features, MAC lẻ; A12 = 119 MAC + 107 BB live chưa được nhắc ở 01–16 |

Công cụ thêm: `tools/regctx.py` (trích bằng chứng theo dải: `--brief`), `tools/bitname.py` (byte+mask → tên bit, hỗ trợ 64-bit), `tools/appendix.py` (→ `05_groups/appendix/A01–A10`), `tools/mactable.py` (→ `A11_mac_reg_table.md`), `tools/txff_sim.py`, `tools/chbw_sim.py` (RF18/0xBE/0x958/0x860/0x8AC theo kênh+BW), `tools/rftab_sim.py` (walker bảng phydm + `check_positive` theo rfe/cut: radioa/radiob/phy_reg/agc_tab), `tools/txagc_sim.py` (dword TXAGC được ghi theo thứ tự section), `tools/pwrtrk_sim.py` (power tracking MIX_MODE với bảng delta thật).

Chạy lại toàn bộ (từ `tools/`): `inventory.py "$D" $W/mask.json $W` → `tables.py "$D" $W/mask.json $W/tables.json` → `report.py "$D" $W ..` → `report_extra.py $W ..` → `appendix.py "$D" $W ../05_groups/appendix` → `mactable.py $W ../05_groups/appendix/A11_mac_reg_table.md`.

Cải tiến công cụ phiên 2: bỏ qua prototype ở phạm vi file; hàm chỉ được gán vào member của bảng `api/ops/pops/pltfm_api/hal_func` mà member đó không bao giờ được đọc/gọi → `no-caller` (ví dụ `download_flash_88xx`). Số liệu catalog sau cải tiến: MAC A 364, X 26, G 21, T 6.

Cải tiến công cụ phiên 3: `guard.py` dựng **pseudo-block** cho thân `if (C) stmt;` / `else stmt;` không có ngoặc (trước đây chỉ block `{…}` mới mang điều kiện) → 6 địa chỉ đổi trạng thái, cả 6 đã kiểm chứng tay: BB 0xD00 (M→X, `rOFDM1_LSTF` ở nhánh else sau `IS_HARDWARE_TYPE_JAGUAR_ALL`), BB 0xDF4 & RF-direct 0x2DBC (G→X, debug port 11N/JGR3), BB 0x1D70 (A→X, JGR3), MAC 0x7F0 (A→X, 8192F), RF 0x24 (A→X, `RF_T_METER_OLD`). Bản cũ: `work/catalog_before_braceless.json`, `work/guard_before_braceless.py.bak`.

## Phát hiện đáng chú ý (đã có bằng chứng)
- Retry vendor request chỉ khi addr < 0x1000 → BB 0x1800+, đọc RF (0x2800+), USB-local chỉ thử 1 lần (`usb_ops_linux.c:193-195`).
- `usb_read*` trả 0 khi lỗi (không phân biệt được) (`hal_usb.c:362-451`).
- `phydm_check_bit_mask` không `& mask` (`phydm_hal_api8822b.c:243-256`).
- KM mutex `halmac_reg_mutex` chồng lên `usb_vendor_req_mutex`; RMW vẫn không nguyên tử.
- (phiên 3) `cnt_crc8_fail_vhtb` lỗi ưu tiên `&`/`>>` (`phydm_dig.c:1487`); PHY_REG chứa nguyên `array_mp_8822b_cal_init` (975 ghi 0x1B80); `RF_0x1bf0` bị che thành RF 0xF0; kiểm tra "RF is disable" vô hiệu; LCK driver không chạy với 8822B (RTK của HW).
- (phiên 3) `tools/bitname.py`: `--dword` nay áp dụng cho **cặp đứng trước** (đặt ở đầu = toàn cục, giữ nghĩa lệnh cũ); chế độ dword tính mask tương đối với ADDR (trước đây tương đối với base) — đã kiểm: mọi lệnh cũ dùng ADDR = base nên kết quả cũ không đổi; A11 sinh lại giống hệt.
- **Giới hạn tool**: guard quanh lời gọi qua con trỏ hàm (ví dụ `config->phy_lc_calibrate`) không được đánh giá → truy cập của `_phy_lc_calibrate_8822b` (RF 0xC4, BB 0xC00/0xE00 một phần) bị xếp A dù thực tế chỉ MP/debug. Khi viết nhóm cần kiểm tra tay các hàm calibration.

## Việc tiếp theo (theo thứ tự)
- [x] Lượt 5: tài liệu nhóm `05_groups/NN_*.md` — mỗi nhóm: bảng thanh ghi (từ catalog) + phân tích A–E cho thanh ghi chính + bit có bằng chứng. Thứ tự đề xuất: 01 System/Power/Clock (0x000–0x0FF + power-seq) → 08 FW/H2C → 09 USB → 03 Interrupt → 04/05 DMA/TRX/Queue → 06 Beacon/TSF → 02 MAC/WMAC → 07 Security → 10 PS/LPS/WoW → 11 GPIO/LED → 12 eFuse → 13 BB → 14 RF → 15 Calibration/TXPWR → 16 MP/Debug → 17 khác
- [x] Phần 6/7: `07_sequences.md` — probe, ifup (power on→FW→init_mac_cfg→MAC_REG→PHY/RF→BF→wifi-only→misc), đổi kênh, watchdog (thứ tự phydm_watchdog), TX/RX, PS, lỗi (eFuse RFE trống ⇒ probe thất bại), gỡ
- [x] Mục "R": 44 mục (35 MAC) rà tay → `05_groups/appendix/A13_status_R_review.md` (R-off 9, R-var 25, R-dead 10); công cụ `tools/r_review.py`
- [x] Lượt 6: `12_completion.md` (sinh bởi `tools/completion.py`): 2096 địa chỉ, 999 live; S1–S6 theo 00_REQUEST §9; 0 liên kết hỏng, 0 ký tự điều khiển; gom 15 điểm [KIỂM TRA TRÊN BOARD] + 20 điểm "chưa xác định". Mục lục gốc: `README.md`.

## Còn mở (không chặn hoàn thành; cần board hoặc tài liệu Realtek)
- Các mục ở `12_completion.md` §5.1 (đo trên board: rfe_type, cut, thermal/PG, dump hai lần để tìm thanh ghi tự biến động, file /lib/firmware ghi đè…).
- Ý nghĩa bit BB/RF không có trong source (S6), mục đích ghi 0x4E0, cổng 0x1B80, LTE-coex 0x38 từng bit, 0x181C/0x1A1C trên 8822B.
- Tool: guard quanh lời gọi qua con trỏ hàm chưa được đánh giá (ví dụ LCK); vòng `for` chứa `if` không ngoặc chưa thành pseudo-block.
