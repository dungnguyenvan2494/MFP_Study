# 05.17 — Các nhóm còn lại: Beamforming/MU-MIMO, BT-coex wifi-only và chuyển anten, LA/ADC sampling, tính năng động phydm, MAC lẻ

> Nhãn như 05.01. Danh sách đầy đủ các thanh ghi live mà 05.01–05.16 chưa nhắc: [A12](appendix/A12_uncovered_live.md), sinh tự động, 119 MAC + 107 BB. Bằng chứng từng truy cập: A01–A10. File này chỉ mô tả các cụm chức năng và những điểm có bằng chứng đáng chú ý.

## 1. Beamforming (SU/MU) — `CONFIG_BEAMFORMING` bật

`CONFIG_80211AC_VHT` kéo theo `CONFIG_BEAMFORMING` (`include/autoconf.h:80-89`).

Khả năng mặc định: `rtw_beamform_cap = BIT0 | BIT1 | BIT3` (`os_dep/linux/os_intfs.c:321-329`). Theo comment cùng chỗ:

- BIT0 = VHT SU Beamformer
- BIT1 = VHT SU Beamformee
- BIT3 = VHT MU Beamformee
- BIT2 (MU Beamformer) **tắt**

### 1.1 Khởi tạo — `rtl8822b_phy_bf_init` (`hal/rtl8822b/rtl8822b_phy.c:2116-2154`)

Hàm được gọi ở **mỗi lần init**: `rtl8822b_init` → sau `rtl8822b_phy_init_haldm` (`hal/rtl8822b/rtl8822b_halinit.c:333`).

| Thanh ghi | Thao tác | Field / comment gốc |
|---|---|---|
| `0x14C0` `REG_MU_TX_CTL` | RMW 32 bit: `|= BIT16`; `R_MU_RL[15:12] = 0xA`; `&= ~BIT7`; `R_MU_TABLE_VALID[5:0] = 0` | "Enable P1 aggr new packet according to P0 transfer time", "MU Retry Limit", "Disable Tx MU-MIMO until sounding done", "Clear validity of MU STAs". Lưu ý: `BIT16`/`R_MU_RL`/`R_MU_TAB_SEL[10:8]`/`R_EN_MU_MIMO[7]` được **định nghĩa cục bộ** trong `rtl8822b_phy.c:1341-1357`, không có trong `halmac_bit_8822b.h`; header chỉ có `R_EN_REVERS_GTAB[6]` và `R_MU_TABLE_VALID[5:0]` [TOOL: bitname] |
| `0x167C` `REG_WMAC_MU_BF_OPTION` | `W8 = WMAC_TXMU_ACKPOLICY(3) \| TXMU_ACKPOLICY_EN` → `[5:4] = 3`, `[6] = 1` = `0x70` [tính tay] | "MU-MIMO Option as default value" |
| `0x1680` `REG_WMAC_MU_BF_CTL` | `W16 = 0` (`MU_BF_MYAID[11:0]`, `MU_BFRPTSEG_SEL[13:12]`, …) | "MU-MIMO Control as default value" |
| `0x42F` (byte 3 của `0x42C` `REG_TXBF_CTRL`) | `|= BIT6` → `USE_NDPA_PARAMETER[30]` | "0x42C[30] = 1 (0: from Tx desc, 1: from 0x45F)" |
| `0x45F` `REG_NDPA_OPT_CTRL` | `W8 = 0x10` | "Rate=OFDM_6M, BW20" (`BW_SIGTA[4:3]`, `R_NDPA_BW[1:0]`) |
| `0x6DF` (byte 3 của `0x6DC` `REG_BBPSF_CTRL`) | `(v & 0xC0) \| 0x04` → `WMAC_CSI_RATE[29:24] = 4` (= mã HW rate `DESC_RATE6M`) | "Temp Settings — STA2's CSI rate is fixed at 6M" |
| BB `0x1C94` | `W32 = 0xAFFFAFFF` | "Grouping bitmap parameters" — header 8822B không có tên |

Tổng cộng 10 transfer mỗi lần init [tính tay: 2 + 1 + 1 + 2 + 1 + 2 + 1].

### 1.2 Khi có peer hỗ trợ BF

`HW_VAR_SOUNDING_ENTER` → `rtl8822b_phy_bf_enter` (`rtl8822b_ops.c:2369-2370`):

| Vai | Hàm | Thanh ghi |
|---|---|---|
| Beamformee SU (mình nhận NDPA từ AP) | `_config_beamformer_su` (`:1620-1672`) | `0x718` `REG_SND_PTCL_CTRL = 0xDB` (byte 0: `R_WMAC_USE_NSTS[7]`, `R_DISABLE_CHECK_VHTSIGB_CRC[6]`, `R_WMAC_BFPARAM_SEL[4]`, `R_WMAC_CSISEQ_SEL[3]`, `R_WMAC_HT_NDPA_EN[1]`, `R_WMAC_VHT_NDPA_EN[0]`); MAC của beamformer → `0x6E4`/`0x6EC` (`R_WMAC_SOUNDING_RXADD_R0/R1[47:0]`, ghi từng byte); tham số CSI → `0x6F4`/`0x6F6` (`R_WMAC_BFINFO_20M_*`); `0x71B = 0x70` → `NDP_RX_STANDBY_TIMER[31:24]` |
| Beamformer SU (mình gửi NDPA) | `_config_beamformee_su` (`:1736-1794`) | `0x42C`: `R_TXBF0/1_AID` = P_AID, xoá cờ BW, `\| R_EN_NDPA_INT[28] \| USE_NDPA_PARAMETER[30] \| R_ENABLE_NDPA[31]`; `0x714` `REG_ASSOCIATED_BFMEE_SEL`: `AID0[8:0]`/`AID1[24:16]` + `TXUSER_ID0[9]`/`TXUSER_ID1[25]`, rồi `&= 0x03FFFFFF; \|= 0x60000000` (idx 0) hoặc `0xE0000000` (idx 1) — comment gốc **"unknown?"**, bit `[31:26]` không có tên trong header [TOOL] |
| Sounding | `_sounding_config_su` (`:1422-1472`) | `0x42C` `R_TXBF0/1_{80,40,20}M`: dùng `switch` **rơi xuyên** (fall-through có chủ ý), nên BW 80 bật cả 3 bit, BW 40 bật 40 và 20 |
| MU beamformee | `_config_beamformee_mu`, `_sounding_config_mu`, `rtl8822b_phy_bf_set_gid_table` | `0x14C0–0x14CC` (GID valid `R_MU_STA_GTAB_VALID`, user position), `0x1684–0x168E` (`WMAC_MU_BFEE2..7_EN/AID`, `STATUS_BFEEn[10]`), `0x45D` `R_NDPA_RATE_V1`, `0x688` RXFLTMAP3, BB `0xF4C` |
| Trạng thái sounding | `rtl8822b_phy_bf_sounding_status` (`:2337-…`) | `0x42C` `DIS_NDP_BFEN[15]`, `0x1684+2n` `STATUS_BFEEn` |
| Phía PHY | `phydm_8822btxbf_rfmode` (gọi trong `_config_beamformee_su`), `phydm_8822b_sutxbfer_workaroud` | RF `0xEF/0x33/0x3E/0x3F` (LUT — 14 §2.3), BB `0x19AC` `REG_BB_TXBF_ANT_SET_BF1`, `0x93C/0x940` (13 §6), `0x19F0`/`0x19F8`, `0xB58` |

**Hai đường BF song song** [TOOL]: `halmac_mimo_88xx.c` (`cfg_txbf_88xx`, `su_bfee_entry_init_88xx`, …) cũng ghi `REG_TXBF_CTRL`/`REG_SND_PTCL_CTRL`. Trong build này, phần lớn hàm đó là `no-caller`, ví dụ `cfg_txbf_88xx` (A06/A03). Đường thật là `rtl8822b_phy.c`. Ngoại lệ là `cfg_sounding_88xx`, `cfg_csi_rate_88xx`, `mu_bfer_entry_*`: chúng có trạng thái live trong danh mục và được gọi qua `rtw_halmac_bf_*`. Xem A12 để biết vị trí.

## 2. BT-coex "wifi-only" và chuyển anten

`CONFIG_BT_COEXIST = n` (01 §1). Vì vậy `rtl8822b_init` luôn gọi `rtw_btcoex_wifionly_hw_config` (`rtl8822b_halinit.c:340-350`) → `ex_hal8822b_wifi_only_hw_config` (`hal/btc/halbtc8822bwifionly.c`). Mọi lệnh đều đi qua `phy_set_bb_reg` (RMW 32 bit, kể cả với thanh ghi MAC):

| Thanh ghi | Giá trị | Field (bitname) / comment gốc |
|---|---|---|
| MAC `0x4C[24:23]` | 2 | `DPDT_WLBT_SEL[24] = 1`, `DPDT_SEL_EN[23] = 0` — "BB control" |
| BB `0xCB4[7:0]` | `0x77` | RFE pinmux (13 §7) — "SW control" |
| BB `0x974[9:8]` | 3 | "antenna mux switch" |
| BB `0x1990[9:8]` | 0 | `r_rfe_path_sel` (theo comment ở `phydm_rfe_2t3r`) |
| BB `0xCBC[19]` | 0 | "RFE_buffer" (theo comment ở `phydm_rfe_2t3r`) |
| MAC `0x70[31:24]` | `0x0E` | `DBG_GNT_WL_BT[27]`, `LTE_MUX_CTRL_PATH[26]`, `LTE_COEX_UART[25]` = 1; `3W_LTE_WL_GPIO[24]` = 0 — "switch to WL side controller and gnt_wl gnt_bt debug signal" |
| MAC `0x1704` rồi `0x1700` | `0x7700`, `0xC00F0038` | **ghi gián tiếp LTE-coex offset `0x38 = 0x7700`**: `LTECOEX_ACCESS_START[31] \| WRITE_MODE[30] \| BYTE_EN[19:16] = 0xF \| ADDR = 0x38` — comment: **"gnt_wl=1 , gnt_bt=0"** |
| MAC `0x6C0`, `0x6C4` | `0xAAAAAAAA` | `REG_BT_COEX_TABLE` (`COEX_TABLE_1[31:0]`, …) |

Nhận xét [SOURCE + so sánh với 05.08 §5]:

- Đây là nơi thứ hai trong build chạm LTE-coex offset `0x38` (nơi thứ nhất là backup/restore quanh việc tải FW). Comment ở đây cho biết offset `0x38` điều khiển tín hiệu **GNT_WL/GNT_BT**.
- Khác với `ltecoex_reg_write_88xx` (05.08), đường này **không chờ** `LTECOEX_READY_BIT_V1[29]` (`0x1703 & BIT5`) trước hoặc sau lệnh ghi. Nó cũng ghi `0x1700` bằng RMW: vì mask = `0xFFFFFFFF` nên chỉ có 1 `W32`.
- Đã ghi ở 09, mục 23.

Chuyển anten theo băng (`hal8822b_wifi_only_switch_antenna`), gọi từ scan, switch band và connect notify:

| Băng | `0xCBC[9:8]` |
|---|---|
| 5 GHz | 1 |
| 2.4 GHz | 2 |

`phy_set_rf_path_switch_8822b` (MP, 16 §2) và `_phy_query_rf_path_switch_8822b` cũng dùng `0xCBC`/`0xCB4`/`0x974`.

## 3. LA mode / ADC sampling (logic analyzer)

`phydm_adc_sampling.c` dùng các thanh ghi sau: `phydm_la_set_mac_trigger_time`, `phydm_la_get_tx_pkt_buf`, `phydm_la_adc_smp_start` đọc/ghi MAC `0x7C0` (`REG_IQ_DUMP`) và `0x7CC` (`REG_WMAC_FTM_CTL`); `phydm_la_set_bb` dùng BB `0x95C`, `0x8B4`; MAC `0x0038` (`REG_PWR_OPTION_CTRL`, G — 16 §3).

- Chỉ được kích hoạt qua lệnh phydm `lamode` (16 §1), nên dù danh mục xếp A, trên thực tế đây là đường debug.
- Các nhánh `ODM_RTL8192F` (MAC `0x7F0–0x7F8`) là X sau cải tiến guard phiên 3.
- `phydm_la_get_tx_pkt_buf` (`phydm_adc_sampling.c:822-…`) với 8822B: `W8(0x106, 0x69)` (byte 2 của `REG_CR`, không có tên bit — cùng "khoá" `0x69` như mở khoá eFuse ở 05.12) rồi `R32(0x7C0)` lấy `finish_addr`; dữ liệu đọc qua **cửa sổ packet buffer**: `0x140[15:0] = 0x780 + page`, `R32(0x8000 + (addr & 0xFFF))` (`:806-817`) — không gian gián tiếp thứ 9 (02 §2.2).

## 4. Tính năng động phydm (watchdog) — thanh ghi BB chưa có ở 13

Danh sách đầy đủ ở A12. Phân theo hàm; mục đích là [INFERENCE theo tên hàm/biến] trừ khi có ghi chú:

| Tính năng | Hàm | Thanh ghi BB |
|---|---|---|
| EDCCA / adaptivity (ngưỡng phát hiện năng lượng — quy định ETSI) | `phydm_set_edcca_threshold`, `phydm_set_forgetting_factor`, `phydm_edcca_decision_opt`, `phydm_adaptivity_init` | `0x8A4`, `0x8A0`, `0x8DC`, `0x944` |
| NBI (narrow-band interference) / CSI mask / spur | `phydm_nbi_enable`, `phydm_set_nbi_reg`, `phydm_spur_calibration_8822b`, `phydm_csi_mask_enable`, `phydm_clean_all_csi_mask`, `phydm_dsde_init`, `phydm_dynamic_spur_det_eliminate` | `0x87C`, `0xC20`/`0xE20`, `0x874`, `0x880–0x89C`, `0x910`, `0xF44` |
| RX DFIR theo BW | `phydm_rxdfirpar_by_bw_8822b` | `0x948`, `0x94C`, `0xC20`/`0xE20` |
| CFO (lệch tần số sóng mang) — đầu vào cho crystal cap (15 §6) | `phydm_get_cfo_info_ac`, `phydm_soml_cfo_process` | `0xD0C–0xD14`, `0xD4C–0xD54` (đọc) |
| DC cancellation | `phydm_dc_cancellation` (`phydm.c:3270-3380`) | `0xA78` (byte 1 = 0: "disable CCK DCNF"), `0xA9C`, `0xC10`/`0xC14`, `0xE10`/`0xE14`; dùng debug port `0x8FC`/`0xFA0` (16 §3) |
| Trọng số anten / SoML / OTA | `phydm_dynamic_ant_weighting_8822b`, `phydm_dynamic_parameters_ota`, `phydm_set_weighting_mcc` | `0x98C`, `0xF94`, `0x818`, `0x19D4`/`0x19D8` |
| CCK PD / CCK RX antenna | `phydm_write_cck_pd_type1`, `phydm_config_cck_rx_antenna_init`, `phydm_config_cck_rx_path` | `0xA0A`, `0xA14`, `0xA20`, `0xA70`, `0xA74` |
| CCK DIG phụ | `odm_write_dig` | `0xA0C` |
| `SetHwReg` (CCK rake/DSP) | `SetHwReg` (`hal_com.c:14942-14945`) | `0xA2E` (`ODM_CCK_RAKE_MAC`), `0xA1C` (`rCCK0_DSPParameter2`) |
| Noise monitor | `phydm_idle_noise_measure_ac` | `0x9E4`, `0xFF0` |
| Condition number (MIMO) | `phydm_get_condi_num_8822b`, `_acc_8822b` | `0x1988`, `0xF84`, `0xF87` |
| Direct BF / CDD | `phydm_dis_cdd`, `phydm_pathb_q_matrix_rotate` | `0x9AC`, `0x1950`/`0x1954` |
| FAHM (bổ sung cho CCX — 13 §8) | `phydm_fahm_set_th_by_igi`, `phydm_fahm_trigger` | `0x1C38`, `0x1C78`, `0x1C7C`, `0x1CB8`, `0x1CF8` (nhánh `ODM_IC_11AC_SERIES`, `phydm_ccx.c:309-318`) |
| SoML RxHP | `phydm_somlrxhp_setting`, `config_phydm_switch_band_8822b` | `0x19A8` (+ `0xC04/0xE04/0x8CC/0x8D8` — 13 §3.3) |

Thêm 300+ thanh ghi BB khác (`L0 tab1` trong A12 / danh mục) **chỉ** được ghi một lần qua PHY_REG (13 §2). Driver không chạm lại chúng, và header 8822B không có tên hay field cho phần lớn số đó. Ý nghĩa của chúng: chưa xác định.

## 5. MAC lẻ — chưa có ở các nhóm 01–16

Chọn lọc từ A12 [SOURCE: tên hàm + tên thanh ghi]:

| Nhóm | Thanh ghi | Hàm |
|---|---|---|
| Protocol/AMPDU | `0x4BC` `SW_AMPDU_BURST_MODE_CTRL`, `0x4C8` `PROT_MODE_CTRL`, `0x4CA/0x4CB` (`cfg_ampdu_8822b`), `0x4CE`, `0x45E` `TX_HANG_CTRL`, `0x1448–0x144E` `FAST_EDCA_*_SETTING` | `init_protocol_cfg_8822b` (`halmac_init_8822b.c:800-818`) |
| Lifetime | `0x426` `LIFETIME_EN`, `0x4C0` `PKT_LIFE_TIME` | `cfg_txfifo_lt_88xx` |
| TX FIFO rỗng | `0x41A` `TXPKT_EMPTY`, `0x41B`, `0x418` `BCNQ_INFO`, `0x11F4/0x11F8` `MSG3/MSG4` | `txfifo_is_empty_88xx`, `rtw_halmac_txfifo_wait_empty` |
| Rate | `0x434` (DARFRC — 05.02), `0x442` (byte 2 RRSR, `rtw_var_set_basic_rate`), `0x450` (`phydm_fw_fix_rate`), `0x4CC` `BAR_MODE_CTRL` | phydm RA, `hal_com.c` |
| EDCA/beacon/TSF bổ sung | `0x504`/`0x50C` (VI/BK param qua `HW_VAR_AC_PARAM_*`), `0x502/0x506` (TXOP — 05.06), `0x510` `BCNTCFG`, `0x518` `TSFTR_SYN_OFFSET`, `0x541`, `0x556`, `0x55C` `USTIME_TSF`, `0x55F`, `0x564`, `0x579`, `0x5B0` `DIS_ATIM`, `0x5B8–0x5BC` `MBSSID_BCN_SPACE2/3`, `0x5C0` `ACMHWCTRL`, `0x638` `USTIME_EDCA`, `0x63C–0x63F` `RESP_SIFS_CCK/OFDM`, `0x652` | `InitBeaconParameters`, `cfg_bcn_space_88xx`, `cfg_mac_clk_88xx`, `rtl8822b_sethwreg` |
| WMAC bổ sung | `0x605/0x606` (TCR byte), `0x60A` (`rx_clk_gate_88xx`), `0x614` (MACID byte 4–5), `0x667` (RXERR_RPT byte 3, `_rx_report`), `0x668` `WMAC_TRXPTCL_CTL` (`cfg_bw_88xx`), `0x66A` (`hw_var_set_ack_preamble`), `0x66C`, `0x691` `NAN_RX_TSF_FILTER`, `0x693` `WMMPS_UAPSD_TID`, `0x6A3/0x6A5` (`efuse_PreUpdateAction` — 12), `0x7D4/0x7D8` | `init_wmac_cfg_8822b`, … |
| Kiểm tra IO | `0x5FC` `PAGE5_DUMMY` | `verify_io_88xx` (HALMAC tự kiểm tra đọc/ghi) |
| Loại mạng/port | `0x1100` `CR_EXT` | `cfg_net_type_88xx`, `rtw_halmac_get_network_type` |
| Khác | `0x061/0x062` (GPIO output — 05.11), `0x101`, `0x103`, `0x206`, `0x286`, `0x478` (queue/rsvd page/RX DMA — 05.04), `0x483` `DATA_SC` (`cfg_pri_ch_idx_88xx`), `0x4FC` `DUMMY_PAGE4`, `0x1070` `SYS_CFG5`, `0x1118` `RXFF_PTR` (`check_rx_count`), `0x1624–0x1644` (`MACID2..4` — 05.02 §5) | |

## 6. Dấu hiệu lỗi

| Hiện tượng | Kiểm tra |
|---|---|
| Throughput 11ac thấp với AP hỗ trợ BF | `0x718` (`0xDB`), `0x6E4…` (MAC của AP), `0x42C`; log `HW_VAR_SOUNDING_ENTER` |
| Anten 5G/2.4G đổi sai | `0xCBC[9:8]` (1 = 5G, 2 = 2.4G), `0x4C[24:23]` |
| Nghi xung đột GNT_WL/GNT_BT | LTE-coex `0x38` (đọc gián tiếp như 05.08 §5), `0x70[31:24]` |
| Không truyền được ở kênh DFS/ETSI | EDCCA `0x8A4` |
