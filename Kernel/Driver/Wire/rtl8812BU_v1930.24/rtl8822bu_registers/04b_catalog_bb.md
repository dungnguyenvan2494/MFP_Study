# Danh mục thanh ghi BB/PHY (bus 16-bit, vùng BB)

Vùng BB: `0x0800–0x0FFF`, `0x1800–0x1FFF`; vùng `0x2800–0x2FFF` = cửa sổ đọc RF trực tiếp (RF-direct). Cùng bus với MAC — `phy_set_bb_reg` cuối cùng vẫn là `rtw_write32`. BB-pg = khoá địa chỉ trong bảng PHY_REG_PG (lưu vào bảng công suất, không ghi trực tiếp).

Tổng: **968 địa chỉ** — A=501, M=5, G=11, X=85, R=5, Z=109, D=252

Cột: **St** = trạng thái (A/T/M/G/X/R/Z/D, xem `04_catalog_README.md`); **W** = độ rộng truy cập quan sát được trong code/bảng (không phải độ rộng phần cứng); **Truy cập** = live/MP/dbg/dead/bảng; **Bit** = số macro bit/field định nghĩa trong section của thanh ghi / số được dùng; **Nhóm** = [INFERENCE] suy từ tên hàm truy cập.

| STT | Địa chỉ | Tên (alias) | Không gian | St | Nhóm | W | Kiểu | Truy cập L/M/G/X/T | Bit d/u | Hàm (≤4) | Vị trí (≤3) |
|---:|---|---|---|:-:|---|---|---|---|---|---|---|
| 1 | `0x0800` | R_0x800 (+4: rFPGA0_RFMOD, REG_FPGA0_RFMOD, …) | BB | A | Antenna/RFE | 32 | R R-mask RMW-mask W | 2/13/0/12/1 | 0/0 | phydm_config_rx_path_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:2455, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2462 |
| 2 | `0x0804` | R_0x804 (+2: rFPGA0_TxInfo, ODM_REG_CCK_RPT_FORMAT_11AC) | BB | A | Init table | 32 | RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 3 | `0x0808` | R_0x808 (+5: ODM_PSDREG, ODM_PSD_SETTING, …) | BB | A | Calibration | 8/32 | R R-mask RMW-mask W | 18/12/0/10/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_bb_reset_8822b, _iqk_configure_macbb_8822b, _iqk_restore_mac_bb_8822b … | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:272, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329 … |
| 4 | `0x080B` | *(không có macro)* | BB | X | BB DM (DIG/CCA/FA/CCX) | 8 | R W | 0/0/0/5/0 | 0/0 | odm_inband_noise_monitor_ac | hal/phydm/phydm_noisemonitor.c:364, hal/phydm/phydm_noisemonitor.c:365, hal/phydm/phydm_noisemonitor.c:366 … |
| 5 | `0x080C` | R_0x80c (+4: rFPGA0_TxGainStage, ODM_REG_BB_TX_PATH_11AC, …) | BB | A | TX power | 32 | R-mask RMW-mask | 9/0/0/3/1 | 0/0 | phydm_bb_ctrl_txagc_ofst, phydm_config_tx_path_8822b | hal/phydm/phydm_api.c:1952, hal/phydm/phydm_api.c:1954, hal/phydm/phydm_api.c:1956 … |
| 6 | `0x0810` | R_0x810 | BB | A | Antenna/RFE | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | phydm_rfe_efem | hal/phydm/rtl8822b/phydm_hal_api8822b.c:410 |
| 7 | `0x0814` | R_0x814 | BB | A | Channel/BW | 32 | RMW-mask | 3/0/0/0/1 | 0/0 | config_phydm_switch_band_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1620, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1691, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1694 |
| 8 | `0x0818` | R_0x818 (+2: ODM_AFE_SETTING, ODM_REG_BB_PWR_SAV5_11N) | BB | A | Init table | 32 | R-mask RMW-mask | 5/0/0/0/1 | 0/0 | phydm_dynamic_parameters_ota, phydm_set_weighting_mcc | hal/phydm/phydm_api.c:2898, hal/phydm/phydm_api.c:2906, hal/phydm/rtl8822b/phydm_rtl8822b.c:101 … |
| 9 | `0x081C` | R_0x81c | BB | A | Init table | 32 |  | 0/0/0/0/9824 | 0/0 |  | bảng: array_mp_8822b_agc_tab, array_mp_8822b_phy_reg |
| 10 | `0x0820` | R_0x820 (+2: rFPGA0_XA_HSSIParameter1, REG_FPGA0_XA_HSSI_PARAMETER1) | BB | A | Init table | 32 | RMW-mask | 0/4/0/4/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 11 | `0x0824` | R_0x824 (+2: rFPGA0_XA_HSSIParameter2, ODM_REG_CCK_RPT_FORMAT_11N) | BB | A | Init table | 32 | RMW-mask | 0/0/0/5/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 12 | `0x0828` | R_0x828 (+2: rFPGA0_XB_HSSIParameter1, REG_FPGA0_XB_HSSI_PARAMETER1) | BB | A | Init table | 32 | RMW-mask | 0/4/0/4/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 13 | `0x082C` | R_0x82c (+2: rFPGA0_XB_HSSIParameter2, ODM_REG_CCK_RPT_FORMAT_11N_B) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 5/0/0/5/1 | 0/0 | phydm_ccapar_by_rfe_8822b, phydm_dsde_nbi | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1227, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1229, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1233 … |
| 14 | `0x0830` | R_0x830 (+3: ODM_TXAGC_B_6_18, rTxAGC_B_Rate18_06, …) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 3/0/0/1/1 | 0/0 | phydm_ccapar_by_rfe_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:852, hal/phydm/rtl8822b/phydm_hal_api8822b.c:854, hal/phydm/rtl8822b/phydm_hal_api8822b.c:856 |
| 15 | `0x0834` | R_0x834 (+3: ODM_TXAGC_B_24_54, rTxAGC_B_Rate54_24, …) | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 16 | `0x0838` | R_0x838 (+4: rCCAonSec_Jaguar, ODM_TXAGC_B_MCS32_5, …) | BB | A | Calibration | 32 | R RMW-mask W | 9/2/0/1/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_configure_macbb_8822b, _iqk_restore_mac_bb_8822b, phydm_ccapar_by_rfe_8822b … | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:610 … |
| 17 | `0x083C` | R_0x83c (+3: rTxAGC_B_Mcs03_Mcs00, ODM_TXAGC_B_MCS0_MCS3, …) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 1/0/0/1/1 | 0/0 | phydm_ccapar_by_rfe_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:861 |
| 18 | `0x0840` | R_0x840 (+1: R_0x844) | BB | A | Channel/BW | 32 | RMW-mask | 1/0/0/1/1 | 0/0 | config_phydm_switch_bandwidth_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:2188 |
| 19 | `0x0844` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 20 | `0x0848` | R_0x848 (+3: rTxAGC_B_Mcs07_Mcs04, ODM_TXAGC_B_MCS4_MCS7, …) | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 21 | `0x084C` | R_0x84c (+3: rTxAGC_B_Mcs11_Mcs08, ODM_TXAGC_B_MCS8_MCS11, …) | BB | A | Init table | 32 | R-mask RMW-mask | 0/0/0/4/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 22 | `0x0850` | R_0x850 | BB | A | Antenna/RFE | 32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_config_rx_path_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:2457, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2464 |
| 23 | `0x0854` | R_0x854 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 24 | `0x0858` | R_0x858 (+1: ODM_REG_RX_DEFAULT_A_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 25 | `0x085A` | ODM_REG_RX_DEFAULT_B_11N | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 26 | `0x085C` | R_0x85c (+2: ODM_ANALOG_REGISTER, ODM_REG_BB_PWR_SAV3_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 27 | `0x0860` | R_0x860 (+5: ODM_REG_BB_ATC_11AC, ODM_REG_ANTSEL_CTRL_11N, …) | BB | A | Channel/BW | 32 | RMW-mask | 5/0/0/1/1 | 0/0 | config_phydm_switch_channel_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1899, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1941, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1943 … |
| 28 | `0x0864` | R_0x864 (+3: ODM_REG_RX_ANT_CTRL_11N, rFPGA0_XB_RFInterfaceOE, …) | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 29 | `0x0868` | R_0x868 (+3: rTxAGC_B_Mcs15_Mcs12, ODM_TXAGC_B_MCS12_MCS15, …) | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 30 | `0x086C` | R_0x86c (+3: ODM_TXAGC_B_11_A_2_11, rTxAGC_B_CCK11_A_CCK2_11, …) | BB | A | Init table | 32 | RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 31 | `0x0870` | R_0x870 (+3: ODM_REG_PIN_CTRL_11N, rFPGA0_XAB_RFInterfaceSW, …) | BB | A | Init table | 32 | RMW-mask | 0/0/0/6/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 32 | `0x0874` | R_0x874 (+2: ODM_AD_DA_LSB_MASK, ODM_REG_BB_PWR_SAV1_11N) | BB | A | Beamforming/MU | 32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_csi_mask_enable, phydm_dsde_init | hal/phydm/phydm_api.c:814, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1156 |
| 33 | `0x0878` | R_0x878 (+3: rFPGA0_XAB_RFParameter, ODM_REG_ANTSEL_PATH_11N, …) | BB | A | Init table | 32 | RMW-mask | 0/0/0/6/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 34 | `0x087C` | R_0x87c | BB | A | Calibration | 32 | RMW-mask | 3/0/0/6/1 | 0/0 | phydm_nbi_enable, phydm_set_nbi_reg, phydm_spur_calibration_8822b | hal/phydm/phydm_api.c:1003, hal/phydm/phydm_api.c:978, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1576 |
| 35 | `0x0880` | R_0x880 | BB | A | Beamforming/MU | 32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_clean_all_csi_mask, phydm_dsde_init | hal/phydm/phydm_api.c:848, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1148 |
| 36 | `0x0884` | R_0x884 | BB | A | Beamforming/MU | 32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_clean_all_csi_mask, phydm_dsde_init | hal/phydm/phydm_api.c:849, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1149 |
| 37 | `0x0888` | R_0x888 | BB | A | Beamforming/MU | 32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_clean_all_csi_mask, phydm_dsde_init | hal/phydm/phydm_api.c:850, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1150 |
| 38 | `0x088C` | R_0x88c (+4: ODM_ENABLE_3_WIRE, ODM_REG_BB_3WIRE_11N, …) | BB | A | Beamforming/MU | 32 | R RMW-mask W | 2/5/0/6/1 | 0/0 | phydm_clean_all_csi_mask, phydm_dsde_init | hal/phydm/phydm_api.c:851, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1151 |
| 39 | `0x0890` | R_0x890 (+2: ODM_REG_CLM_11N, ODM_REG_NHM_TH9_TH10_11N) | BB | A | Beamforming/MU | 32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_clean_all_csi_mask, phydm_dsde_init | hal/phydm/phydm_api.c:852, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1152 |
| 40 | `0x0894` | R_0x894 (+1: ODM_REG_CCX_PERIOD_11N) | BB | A | Beamforming/MU | 32 | RMW-mask | 2/0/0/1/1 | 0/0 | phydm_clean_all_csi_mask, phydm_dsde_init | hal/phydm/phydm_api.c:853, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1153 |
| 41 | `0x0898` | R_0x898 (+1: ODM_REG_NHM_TH3_TO_TH0_11N) | BB | A | Beamforming/MU | 32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_clean_all_csi_mask, phydm_dsde_init | hal/phydm/phydm_api.c:854, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1154 |
| 42 | `0x089C` | R_0x89c (+1: ODM_REG_NHM_TH7_TO_TH4_11N) | BB | A | Beamforming/MU | 32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_clean_all_csi_mask, phydm_dsde_init | hal/phydm/phydm_api.c:855, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1155 |
| 43 | `0x08A0` | R_0x8a0 | BB | A | Init table | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | phydm_set_forgetting_factor | hal/phydm/phydm_adaptivity.c:373 |
| 44 | `0x08A4` | R_0x8a4 (+2: rFPGA0_XB_LSSIReadBack, REG_FPGA0_XB_LSSI_READ_BACK) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask RMW-mask | 2/0/1/0/1 | 0/0 | phydm_set_edcca_threshold | hal/phydm/phydm_adaptivity.c:222, hal/phydm/phydm_adaptivity.c:223 |
| 45 | `0x08A8` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 46 | `0x08AC` | R_0x8ac | BB | A | Channel/BW | 32 | R-mask RMW-mask | 10/0/0/2/1 | 0/0 | config_phydm_switch_bandwidth_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:2103, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2106, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2140 … |
| 47 | `0x08B0` | rHSSIRead_Jaguar | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 48 | `0x08B4` | R_0x8b4 (+3: ODM_PSD_REPORT, ODM_REG_PSD_DATA_11N, …) | BB | A | TX power | 32 | RMW-mask | 3/0/0/2/1 | 0/0 | phydm_bb_ctrl_txagc_ofst, phydm_la_set_bb, phydm_stop_ck320 | hal/phydm/phydm_adc_sampling.c:1067, hal/phydm/phydm_api.c:1901, hal/phydm/phydm_api.c:1948 |
| 49 | `0x08B8` | R_0x8b8 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 50 | `0x08BC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 51 | `0x08C0` | R_0x8c0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 52 | `0x08C4` | R_0x8c4 (+1: ODM_REG_SC_CNT_11N) | BB | A | Channel/BW | 32 | RMW-mask | 5/0/6/0/1 | 0/0 | config_phydm_switch_bandwidth_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:2109, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2146, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2180 … |
| 53 | `0x08C8` | R_0x8c8 | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask RMW-mask | 6/0/1/0/1 | 0/0 | config_phydm_switch_bandwidth_8822b, phydm_ccapar_by_rfe_8822b, phydm_dig_init, phydm_set_big_jump_step | hal/phydm/phydm_dig.c:367, hal/phydm/phydm_dig.c:811, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2189 … |
| 54 | `0x08CC` | R_0x8cc | BB | A | Channel/BW | 32 | RMW-mask | 11/0/0/0/1 | 0/0 | config_phydm_switch_band_8822b, phydm_somlrxhp_setting | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1635, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1640, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1653 … |
| 55 | `0x08D0` | R_0x8d0 (+2: ODM_REG_CLM_RESULT_11N, ODM_REG_NHM_CNT9_TO_CNT8_11N) | BB | A | Init table | 32 | R-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 56 | `0x08D4` | R_0x8d4 (+1: ODM_REG_NHM_CNT10_TO_CNT11_11N) | BB | A | Init table | 32 | R | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 57 | `0x08D8` | R_0x8d8 (+1: ODM_REG_NHM_CNT_11N) | BB | A | Channel/BW | 32 | R RMW-mask | 21/0/0/1/1 | 0/0 | config_phydm_switch_band_8822b, phydm_dynamic_parameters_ota, phydm_dynamic_switch_htstf_mumimo_8822b, phydm_somlrxhp_setting | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1637, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1638, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1642 … |
| 58 | `0x08DC` | R_0x8dc (+2: ODM_REG_EDCCA_POWER_CAL, ODM_REG_NHM_CNT7_TO_CNT4_11N) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R RMW-mask | 1/0/0/1/1 | 0/0 | phydm_edcca_decision_opt | hal/phydm/phydm_adaptivity.c:384 |
| 59 | `0x08E0` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 60 | `0x08E4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 61 | `0x08E8` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 62 | `0x08EC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 63 | `0x08F0` | R_0x8f0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 64 | `0x08F4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 65 | `0x08F8` | R_0x8f8 | BB | A | Calibration | 32 | RMW-mask W | 2/0/1/0/1 | 0/0 | _iqk_bb_reset_8822b, _iqk_tximr_selfcheck_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1368, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:257 |
| 66 | `0x08FC` | R_0x8fc (+1: ODM_REG_DBG_RPT_11AC) | BB | A | Calibration | 32 | R-mask RMW-mask W | 1/0/34/1/1 | 0/0 | _iqk_bb_reset_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:260 |
| 67 | `0x0900` | R_0x900 (+4: r_ANTSEL_SW_Jaguar, REG_ANTSEL_SW_JAGUAR, …) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 68 | `0x0904` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 69 | `0x0908` | R_0x908 (+1: ODM_REG_DBG_RPT_11N) | BB | A | Init table | 32 | R-mask RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 70 | `0x090C` | R_0x90c (+4: rFPGA1_TxInfo, ODM_R_ANT_SELECT, …) | BB | A | Calibration | 32 | R R-mask RMW-mask W | 3/0/0/2/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_configure_macbb_8822b, _iqk_restore_mac_bb_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:596 |
| 71 | `0x0910` | R_0x910 | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask RMW-mask | 13/1/0/0/1 | 0/0 | phydm_dynamic_spur_det_eliminate, phydm_psd_para_setting | hal/phydm/phydm_psd.c:364, hal/phydm/phydm_psd.c:365, hal/phydm/phydm_psd.c:366 … |
| 72 | `0x0914` | R_0x914 (+3: ODM_REG_ANT_MAPPING1_11N, rSingleTone_ContTx_Jaguar, …) | BB | A | Init table | 32 | RMW-mask | 0/6/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 73 | `0x0918` | R_0x918 (+1: ODM_REG_ANT_MAPPING2_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 74 | `0x091C` | R_0x91c | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 75 | `0x0920` | R_0x920 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 76 | `0x0924` | R_0x924 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 77 | `0x0928` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 78 | `0x092C` | R_0x92c | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 79 | `0x0930` | R_0x930 | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 80 | `0x0934` | R_0x934 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 81 | `0x0938` | R_0x938 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 82 | `0x093C` | REG_BB_TX_PATH_SEL_1_8822B (+1: R_0x93c) | BB | A | Antenna/RFE | 32 | RMW-mask | 8/0/0/2/1 | 0/0 | phydm_8822btxbf_rfmode, phydm_config_ofdm_tx_path_8822b, phydm_config_tx_path_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:2350, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2352, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2355 … |
| 83 | `0x0940` | REG_BB_TX_PATH_SEL_2_8822B (+1: R_0x940) | BB | A | Beamforming/MU | 32 | RMW-mask | 6/0/0/0/1 | 0/0 | phydm_8822btxbf_rfmode, phydm_config_ofdm_tx_path_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:2356, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2359, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2362 … |
| 84 | `0x0944` | R_0x944 (+1: ODM_REG_ACBB_EDCCA_ENHANCE) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 1/0/0/1/1 | 0/0 | phydm_adaptivity_init | hal/phydm/phydm_adaptivity.c:755 |
| 85 | `0x0948` | R_0x948 (+2: ODM_REG_RX_DFIR_MOD_97F, ODM_REG_EDCCA_DOWN_OPT_11N) | BB | A | Channel/BW | 32 | RMW-mask | 3/0/0/1/1 | 0/0 | phydm_rxdfirpar_by_bw_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:880, hal/phydm/rtl8822b/phydm_hal_api8822b.c:886, hal/phydm/rtl8822b/phydm_hal_api8822b.c:892 |
| 86 | `0x094C` | R_0x94c | BB | A | Channel/BW | 32 | RMW-mask | 3/0/0/0/1 | 0/0 | phydm_rxdfirpar_by_bw_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:881, hal/phydm/rtl8822b/phydm_hal_api8822b.c:887, hal/phydm/rtl8822b/phydm_hal_api8822b.c:893 |
| 87 | `0x0950` | R_0x950 | BB | A | Init table | 32 | RMW-mask | 0/0/0/4/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 88 | `0x0954` | R_0x954 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 89 | `0x0958` | R_0x958 | BB | A | Channel/BW | 32 | RMW-mask | 10/0/0/0/1 | 0/0 | config_phydm_switch_channel_8822b, phydm_rxagc_switch_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1789, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1795, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1801 … |
| 90 | `0x095C` | R_0x95c (+1: odm_adc_trigger_jaguar2) | BB | A | Init table | 32 | RMW-mask | 3/0/1/0/1 | 0/0 | phydm_la_set_bb | hal/phydm/phydm_adc_sampling.c:1068, hal/phydm/phydm_adc_sampling.c:1070, hal/phydm/phydm_adc_sampling.c:1071 |
| 91 | `0x0960` | R_0x960 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 92 | `0x0964` | R_0x964 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 93 | `0x0968` | R_0x968 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 94 | `0x096C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 95 | `0x0970` | R_0x970 | BB | A | Init table | 32 | RMW-mask | 0/0/0/5/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 96 | `0x0974` | R_0x974 | BB | A | Antenna/RFE | 32 | RMW-mask | 4/0/0/1/1 | 0/0 | ex_hal8822b_wifi_only_hw_config, phy_set_rf_path_switch_8822b, phydm_rfe_8822b_init | hal/btc/halbtc8822bwifionly.c:70, hal/phydm/halrf/rtl8822b/halrf_8822b.c:554, hal/phydm/rtl8822b/phydm_hal_api8822b.c:310 … |
| 97 | `0x0978` | R_0x978 | BB | A | Init table | 32 | RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 98 | `0x097C` | R_0x97c | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 99 | `0x0980` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 100 | `0x0984` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 101 | `0x0988` | R_0x988 | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 102 | `0x098C` | R_0x98c | BB | A | Antenna/RFE | 32 | R-mask RMW-mask | 8/0/0/0/1 | 0/0 | phydm_dynamic_ant_weighting_8822b, phydm_dynamic_parameters_ota, phydm_set_weighting_mcc | hal/phydm/phydm_api.c:2894, hal/phydm/phydm_api.c:2896, hal/phydm/rtl8822b/phydm_rtl8822b.c:100 … |
| 103 | `0x0990` | R_0x990 (+1: ODM_REG_CCX_PERIOD_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | phydm_clm_setting | hal/phydm/phydm_ccx.c:1655 |
| 104 | `0x0994` | R_0x994 (+2: ODM_REG_CLM_11AC, ODM_REG_NHM_TH9_TH10_11AC) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 105 | `0x0998` | R_0x998 (+2: ODM_REG_SOML_97F, ODM_REG_NHM_TH3_TO_TH0_11AC) | BB | A | Init table | 32 | RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 106 | `0x099C` | R_0x99c (+1: ODM_REG_NHM_TH7_TO_TH4_11AC) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 107 | `0x09A0` | R_0x9a0 (+1: ODM_REG_NHM_TH8_11AC) | BB | A | Init table | 32 | R-mask RMW-mask | 0/0/0/5/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 108 | `0x09A4` | R_0x9a4 (+1: ODM_REG_OFDM_FA_RST_11AC) | BB | A | Calibration | 32 | R RMW-mask W | 7/0/0/0/1 | 0/0 | _iqk_afe_setting_8822b, _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b, phydm_false_alarm_counter_reg_reset … | hal/hal_com.c:16991, hal/hal_com.c:16992, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214 … |
| 109 | `0x09A8` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 110 | `0x09AC` | R_0x9ac | BB | A | Init table | 32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_dis_cdd | hal/phydm/phydm_direct_bf.c:145, hal/phydm/phydm_direct_bf.c:146 |
| 111 | `0x09B0` | R_0x9b0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 112 | `0x09B4` | R_0x9b4 (+1: ODM_REG_CSI_CONTENT_VALUE) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 113 | `0x09B8` | R_0x9b8 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 114 | `0x09BC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 115 | `0x09C0` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 116 | `0x09C4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 117 | `0x09C8` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 118 | `0x09CC` | R_0x9cc | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 119 | `0x09D0` | R_0x9d0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 120 | `0x09D4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 121 | `0x09D8` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 122 | `0x09DC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 123 | `0x09E0` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 124 | `0x09E4` | R_0x9e4 | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_idle_noise_measure_ac | hal/phydm/phydm_noisemonitor.c:219, hal/phydm/phydm_noisemonitor.c:225 |
| 125 | `0x09E8` | R_0x9e8 (+1: ODM_REG_NHM_9E8_11AC) | BB | A | Init table | 32 | RMW-mask | 0/0/0/6/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 126 | `0x09EC` | *(không có macro)* | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 127 | `0x09F0` | R_0x9f0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 128 | `0x09F4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 129 | `0x09F8` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 130 | `0x09FC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 131 | `0x0A00` | R_0xa00 (+2: rCCK0_System, ODM_REG_CCK_ANTDIV_PARA1_11N) | BB | A | Channel/BW | 32 | RMW-mask | 3/14/0/1/1 | 0/0 | config_phydm_switch_bandwidth_8822b, phydm_config_cck_rx_antenna_init | hal/phydm/phydm_api.c:257, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2124, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2126 |
| 132 | `0x0A04` | R_0xa04 (+3: rCCK0_AFESetting, REG_CCK_0_AFE_SETTING, …) | BB | A | Calibration | 32 | R R-mask RMW-mask W | 21/0/0/19/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_bb_reset_8822b, _iqk_restore_mac_bb_8822b, phydm_config_cck_rx_path … | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:274, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329 … |
| 133 | `0x0A07` | ODM_CCK_ANT_SELECT | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 134 | `0x0A08` | R_0xa08 | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 135 | `0x0A0A` | R_0xa0a (+3: ODM_CCK_PD_THRESH, ODM_REG_CCK_CCA_11N, …) | BB | A |  | 8 | W | 1/0/0/0/0 | 0/0 | phydm_write_cck_pd_type1 | hal/phydm/phydm_cck_pd.c:75 |
| 136 | `0x0A0C` | R_0xa0c (+1: ODM_REG_CCK_ANTDIV_PARA2_11N) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | odm_write_dig | hal/phydm/phydm_dig.c:632 |
| 137 | `0x0A10` | R_0xa10 (+1: ODM_REG_CCK_ANTDIV_PARA3_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 138 | `0x0A11` | ODM_CCK_RF_REG1 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 139 | `0x0A14` | R_0xa14 (+1: ODM_REG_CCK_ANTDIV_PARA4_11N) | BB | A | Antenna/RFE | 32 | RMW-mask | 1/0/0/4/1 | 0/0 | phydm_config_cck_rx_antenna_init | hal/phydm/phydm_api.c:260 |
| 140 | `0x0A18` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 141 | `0x0A1C` | rCCK0_DSPParameter2 | BB | A | Init table | 8/32 | W | 2/0/0/0/1 | 0/0 | SetHwReg | hal/hal_com.c:14945, hal/hal_com.c:14950 |
| 142 | `0x0A20` | R_0xa20 (+2: rCCK0_TxFilter1, ODM_CCK_MATCH_FILTER) | BB | A | Antenna/RFE | 32 | R-mask RMW-mask | 1/2/0/1/1 | 0/0 | phydm_config_cck_rx_antenna_init | hal/phydm/phydm_api.c:261 |
| 143 | `0x0A22` | ODM_REG_CCK_FILTER_PARA1_11N | BB | X |  | 8/32 | R RMW-mask | 0/0/0/4/0 | 0/0 | get_cck_swing_index, hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:252, hal/hal_mp.c:277, hal/phydm/halrf/halrf_powertracking_ce.c:686 … |
| 144 | `0x0A23` | ODM_REG_CCK_FILTER_PARA2_11N | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:253, hal/hal_mp.c:278 |
| 145 | `0x0A24` | R_0xa24 (+2: rCCK0_TxFilter2, ODM_REG_CCK_FILTER_PARA3_11N) | BB | A | Channel/BW | 32 | RMW-mask | 2/1/0/4/1 | 0/0 | config_phydm_switch_channel_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1904, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1907 |
| 146 | `0x0A25` | ODM_REG_CCK_FILTER_PARA4_11N | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:255, hal/hal_mp.c:280 |
| 147 | `0x0A26` | ODM_REG_CCK_FILTER_PARA5_11N | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:256, hal/hal_mp.c:281 |
| 148 | `0x0A27` | ODM_REG_CCK_FILTER_PARA6_11N | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:257, hal/hal_mp.c:282 |
| 149 | `0x0A28` | R_0xa28 (+2: rCCK0_DebugPort, ODM_REG_CCK_FILTER_PARA7_11N) | BB | A | Channel/BW | 32 | RMW-mask | 2/1/0/4/1 | 0/0 | config_phydm_switch_channel_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1905, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1908 |
| 150 | `0x0A29` | ODM_REG_CCK_FILTER_PARA8_11N | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:259, hal/hal_mp.c:284 |
| 151 | `0x0A2C` | R_0xa2c (+3: ODM_REG_CCK_FA_RST_11N, rCCK0_FalseAlarmReport, …) | BB | A | Antenna/RFE | 32 | RMW-mask | 8/0/0/10/1 | 0/0 | phydm_config_cck_rx_path, phydm_config_rx_path_8822b, phydm_false_alarm_counter_reg_reset, rtw_reset_phy_rx_counters | hal/hal_com.c:16994, hal/hal_com.c:16995, hal/phydm/phydm_api.c:311 … |
| 152 | `0x0A2D` | ODM_CCK_CNT_RESET | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 153 | `0x0A2E` | ODM_CCK_RAKE_MAC | BB | A |  | 8 | R W | 4/0/0/0/0 | 0/0 | SetHwReg | hal/hal_com.c:14942, hal/hal_com.c:14943, hal/hal_com.c:14948 … |
| 154 | `0x0A2F` | ODM_CCK_TX_DIVERSITY | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 155 | `0x0A40` | R_0xa40 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 156 | `0x0A44` | R_0xa44 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 157 | `0x0A48` | R_0xa48 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 158 | `0x0A4C` | R_0xa4c | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 159 | `0x0A50` | R_0xa50 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 160 | `0x0A54` | R_0xa54 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 161 | `0x0A58` | R_0xa58 (+1: ODM_REG_CCK_FA_MSB_11N) | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 162 | `0x0A5B` | ODM_CCK_FA_CNT_MSB | BB | X |  | 8 | R | 0/0/0/1/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16944 |
| 163 | `0x0A5C` | ODM_CCK_FA_CNT_LSB (+2: ODM_REG_CCK_FA_11AC, ODM_REG_CCK_FA_LSB_11N) | BB | A | BB DM (DIG/CCA/FA/CCX) | 8/32 | R R-mask | 1/0/1/1/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1496 |
| 164 | `0x0A60` | ODM_REG_CCK_CCA_CNT_11N | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 165 | `0x0A68` | R_0xa68 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 166 | `0x0A6C` | R_0xa6c | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 167 | `0x0A70` | R_0xa70 | BB | A | Antenna/RFE | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | phydm_config_cck_rx_antenna_init | hal/phydm/phydm_api.c:258 |
| 168 | `0x0A74` | R_0xa74 (+1: ODM_REG_BB_PWR_SAV4_11N) | BB | A | Antenna/RFE | 32 | RMW-mask | 2/0/0/3/1 | 0/0 | phydm_config_cck_rx_antenna_init, phydm_config_cck_rx_path | hal/phydm/phydm_api.c:259, hal/phydm/phydm_api.c:309 |
| 169 | `0x0A75` | ODM_CCK_NEW_FUNCTION | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 170 | `0x0A78` | R_0xa78 | BB | A | Init table | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | phydm_dc_cancellation | hal/phydm/phydm.c:3294 |
| 171 | `0x0A7C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 172 | `0x0A80` | R_0xa80 | BB | A | Channel/BW | 32 | R-mask RMW-mask | 2/0/0/1/1 | 0/0 | config_phydm_switch_band_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1617, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1676 |
| 173 | `0x0A84` | R_0xa84 | BB | A | Init table | 32 | RMW-mask | 0/1/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 174 | `0x0A88` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 175 | `0x0A8C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 176 | `0x0A90` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 177 | `0x0A94` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 178 | `0x0A98` | R_0xa98 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 179 | `0x0A9A` | *(không có macro)* | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:260, hal/hal_mp.c:285 |
| 180 | `0x0A9B` | *(không có macro)* | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:261, hal/hal_mp.c:286 |
| 181 | `0x0A9C` | R_0xa9c | BB | A | Init table | 32 | RMW-mask | 1/0/0/2/1 | 0/0 | phydm_dc_cancellation | hal/phydm/phydm.c:3321 |
| 182 | `0x0A9D` | *(không có macro)* | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:263, hal/hal_mp.c:288 |
| 183 | `0x0AA0` | *(không có macro)* | BB | A | Init table | 32 | RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 184 | `0x0AA1` | *(không có macro)* | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:265, hal/hal_mp.c:290 |
| 185 | `0x0AA2` | *(không có macro)* | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:266, hal/hal_mp.c:291 |
| 186 | `0x0AA3` | *(không có macro)* | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:267, hal/hal_mp.c:292 |
| 187 | `0x0AA4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 188 | `0x0AA8` | R_0xaa8 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 189 | `0x0AAC` | R_0xaac | BB | X |  | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | hal_mpt_CCKTxPowerAdjust | hal/hal_mp.c:228 |
| 190 | `0x0AB4` | R_0xab4 | BB | X | Calibration | 32 | R-mask | 0/0/0/1/0 | 0/0 | odm_txpowertracking_callback_thermal_meter | hal/phydm/halrf/halphyrf_ce.c:505 |
| 191 | `0x0ABC` | R_0xabc | BB | X | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 0/0/0/1/0 | 0/0 | phydm_get_cck_rssi_table_from_reg | hal/phydm/phydm_phystatus.c:758 |
| 192 | `0x0AC8` | R_0xac8 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 193 | `0x0ACC` | R_0xacc | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 194 | `0x0AD0` | R_0xad0 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 195 | `0x0B00` | R_0xb00 | BB | A | Calibration | 32 | R RMW-mask W | 3/1/0/0/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_configure_macbb_8822b, _iqk_restore_mac_bb_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:606 |
| 196 | `0x0B03` | *(không có macro)* | BB | X | BB DM (DIG/CCA/FA/CCX) | 8 | R W | 0/0/0/4/0 | 0/0 | odm_inband_noise_monitor_ac | hal/phydm/phydm_noisemonitor.c:359, hal/phydm/phydm_noisemonitor.c:360, hal/phydm/phydm_noisemonitor.c:361 … |
| 197 | `0x0B04` | R_0xb04 | BB | A | Init table | 32 | RMW-mask | 0/5/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 198 | `0x0B07` | R_0xb07 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 199 | `0x0B08` | R_0xb08 | BB | A | Init table | 32 | RMW-mask | 0/1/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 200 | `0x0B0C` | R_0xb0c | BB | A | Init table | 32 | RMW-mask | 0/4/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 201 | `0x0B10` | R_0xb10 | BB | A | Init table | 32 | RMW-mask | 0/2/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 202 | `0x0B14` | R_0xb14 | BB | A | Init table | 32 | RMW-mask | 0/1/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 203 | `0x0B18` | R_0xb18 | BB | A | Init table | 32 | RMW-mask | 0/1/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 204 | `0x0B1C` | R_0xb1c | BB | A | Init table | 32 | RMW-mask | 0/2/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 205 | `0x0B20` | R_0xb20 | BB | A | Init table | 32 | RMW-mask | 0/2/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 206 | `0x0B24` | R_0xb24 | BB | A | Init table | 32 | RMW-mask | 0/2/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 207 | `0x0B28` | R_0xb28 | BB | A | Init table | 32 | RMW-mask | 0/2/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 208 | `0x0B2A` | R_0xb2a | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 209 | `0x0B2B` | R_0xb2b | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 210 | `0x0B2C` | R_0xb2c (+1: ODM_REG_LNA_SWITCH_11N) | BB | A | Init table | 32 | RMW-mask | 0/2/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 211 | `0x0B30` | R_0xb30 (+2: PATHDIV_REG, ODM_REG_PATH_SWITCH_11N) | BB | A | Init table | 32 | RMW-mask | 0/2/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 212 | `0x0B34` | R_0xb34 | BB | A | Init table | 32 | RMW-mask | 0/2/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 213 | `0x0B38` | R_0xb38 (+1: ODM_REG_RSSI_CTRL_11N) | BB | A | Init table | 32 | RMW-mask | 0/2/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 214 | `0x0B3B` | R_0xb3b | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 215 | `0x0B3C` | R_0xb3c | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 216 | `0x0B40` | R_0xb40 | BB | A | Init table | 32 | RMW-mask | 0/1/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 217 | `0x0B44` | R_0xb44 | BB | A | Init table | 32 | RMW-mask | 0/1/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 218 | `0x0B48` | R_0xb48 | BB | A | Init table | 32 | RMW-mask | 0/1/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 219 | `0x0B4C` | *(không có macro)* | BB | A | Init table | 32 | RMW-mask | 0/1/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 220 | `0x0B50` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 221 | `0x0B54` | R_0xb54 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 222 | `0x0B58` | R_0xb58 (+1: ODM_REG_RST_RPT_11AC) | BB | A | Beamforming/MU | 32 | RMW-mask | 8/0/0/1/1 | 0/0 | phydm_8822b_sutxbfer_workaroud, phydm_reset_bb_hw_cnt, rtw_reset_phy_trx_ok_counters | hal/hal_com.c:16964, hal/hal_com.c:16965, hal/phydm/phydm_api.c:77 … |
| 223 | `0x0B5C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 224 | `0x0B60` | R_0xb60 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 225 | `0x0B64` | R_0xb64 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 226 | `0x0B68` | R_0xb68 (+1: ODM_REG_CONFIG_ANTA_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 227 | `0x0B6A` | R_0xb6a | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 228 | `0x0B6B` | R_0xb6b | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 229 | `0x0B6C` | R_0xb6c | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 230 | `0x0B6E` | R_0xb6e | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 231 | `0x0B70` | R_0xb70 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 232 | `0x0B74` | R_0xb74 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 233 | `0x0B77` | R_0xb77 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 234 | `0x0B78` | R_0xb78 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 235 | `0x0B7C` | R_0xb7c | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 236 | `0x0B80` | R_0xb80 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 237 | `0x0B84` | R_0xb84 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 238 | `0x0B88` | R_0xb88 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 239 | `0x0B8C` | R_0xb8c | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 240 | `0x0B90` | R_0xb90 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 241 | `0x0B94` | R_0xb94 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 242 | `0x0B98` | R_0xb98 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 243 | `0x0B9B` | R_0xb9b | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 244 | `0x0B9C` | R_0xb9c (+1: ODM_REG_RSSI_BT_11N) | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 245 | `0x0BA0` | R_0xba0 (+1: PATHDIV_TRI) | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 246 | `0x0BA4` | R_0xba4 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 247 | `0x0BA8` | R_0xba8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 248 | `0x0BAC` | R_0xbac | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 249 | `0x0BAD` | R_0xbad | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 250 | `0x0BB0` | ODM_REG_RXCK_RFMOD | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 251 | `0x0BC0` | R_0xbc0 (+1: ODM_REG_EDCCA_DCNF_97F) | BB | X | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_adaptivity_init | hal/phydm/phydm_adaptivity.c:746 |
| 252 | `0x0BC4` | R_0xbc4 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 253 | `0x0BC8` | R_0xbc8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 254 | `0x0BCC` | R_0xbcc | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 255 | `0x0BD8` | R_0xbd8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 256 | `0x0BDC` | R_0xbdc | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 257 | `0x0BE0` | R_0xbe0 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 258 | `0x0BE4` | R_0xbe4 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 259 | `0x0BE8` | R_0xbe8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 260 | `0x0BEC` | R_0xbec | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 261 | `0x0BF0` | R_0xbf0 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 262 | `0x0BF4` | R_0xbf4 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 263 | `0x0BF8` | R_0xbf8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 264 | `0x0C00` | R_0xc00 (+2: ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N) | BB | A | Calibration | 32 | R RMW-mask W | 10/12/0/1/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_configure_macbb_8822b, _iqk_restore_mac_bb_8822b, _phy_lc_calibrate_8822b … | hal/phydm/halrf/rtl8822b/halrf_8822b.c:479, hal/phydm/halrf/rtl8822b/halrf_8822b.c:481, hal/phydm/halrf/rtl8822b/halrf_8822b.c:503 … |
| 265 | `0x0C04` | R_0xc04 (+4: ODM_OFDM_RX_ANT, rOFDM0_TRxPathEnable, …) | BB | A | Channel/BW | 32 | R-mask RMW-mask | 8/0/0/3/1 | 0/0 | config_phydm_switch_band_8822b, phydm_dynamic_parameters_ota, phydm_set_weighting_cmn | hal/phydm/phydm_api.c:2880, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1631, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1649 … |
| 266 | `0x0C08` | R_0xc08 (+4: rOFDM0_TRMuxPar, ODM_REG_TRMUX_11N, …) | BB | A | TX/RX DMA/Queue | 32 | RMW-mask | 2/0/0/4/1 | 0/0 | config_phydm_trx_mode_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:2503, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2505 |
| 267 | `0x0C0C` | R_0xc0c (+1: ODM_REG_OFDM_FA_RSTC_11N) | BB | A | Init table | 32 | RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 268 | `0x0C10` | R_0xc10 (+1: ODM_REG_DOWNSAM_FACTOR_11N) | BB | A | Init table | 32 | RMW-mask | 2/0/0/2/1 | 0/0 | phydm_dc_cancellation | hal/phydm/phydm.c:3350, hal/phydm/phydm.c:3352 |
| 269 | `0x0C14` | R_0xc14 (+2: ODM_R_A_RXIQI, ODM_REG_RXIQI_MATRIX_11N) | BB | A | Init table | 32 | R RMW-mask W | 2/0/0/3/1 | 0/0 | phydm_dc_cancellation | hal/phydm/phydm.c:3353, hal/phydm/phydm.c:3355 |
| 270 | `0x0C18` | R_0xc18 | BB | A | Init table | 32 | RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 271 | `0x0C1C` | R_0xc1c (+2: rA_TxScale_Jaguar, REG_A_TX_SCALE_JAGUAR) | BB | A | Init table | 32 | R R-mask RMW-mask W | 7/3/0/3/1 | 0/0 | get_swing_index, phydm_rxagc_switch_8822b | hal/phydm/halrf/halrf_powertracking_ce.c:664, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1790, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1796 … |
| 272 | `0x0C20` | R_0xc20 | BB | A | Channel/BW | 32 | RMW-mask | 5/0/0/3/1 | 0/0 | phydm_nbi_enable, phydm_rxdfirpar_by_bw_8822b, phydm_spur_calibration_8822b | hal/phydm/phydm_api.c:1004, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1577, hal/phydm/rtl8822b/phydm_hal_api8822b.c:882 … |
| 273 | `0x0C24` | R_0xc24 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 274 | `0x0C28` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 275 | `0x0C2C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 276 | `0x0C30` | R_0xc30 (+1: rOFDM0_RxDetector1) | BB | A | Init table | 32 | R-mask | 0/1/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 277 | `0x0C34` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 278 | `0x0C38` | R_0xc38 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 279 | `0x0C3C` | R_0xc3c | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 280 | `0x0C40` | R_0xc40 | BB | A | Init table | 32 | RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 281 | `0x0C44` | R_0xc44 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 282 | `0x0C48` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 283 | `0x0C4C` | R_0xc4c (+3: rOFDM0_ECCAThreshold, REG_OFDM_0_ECCA_THRESHOLD, …) | BB | A | Init table | 32 | R-mask RMW-mask | 0/0/0/3/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 284 | `0x0C50` | R_0xc50 (+6: rA_IGI_Jaguar, ODM_REG_IGI_A_11N, …) | BB | A | Init table | 8/32 | R R-mask RMW-mask W | 8/1/3/1/9 | 0/0 | mcc_get_reg_hdl, phydm_get_igi_reg_val, phydm_idle_noise_measure_ac, phydm_igi_toggle_8822b … | hal/hal_mcc.c:2748, hal/phydm/phydm_dig.c:542, hal/phydm/phydm_dig.c:663 … |
| 285 | `0x0C54` | R_0xc54 (+2: ODM_R_A_AGC_CORE2, ODM_REG_ANTDIV_PARA2_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 286 | `0x0C58` | R_0xc58 (+4: ODM_REG_IGI_B_11N, ODM_R_B_AGC_CORE1, …) | BB | A | Calibration | 8/32 | R R-mask W | 3/1/3/0/1 | 0/0 | _iqk_afe_setting_8822b, _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:297, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329 |
| 287 | `0x0C5C` | R_0xc5c (+1: ODM_REG_ANTDIV_PARA3_11N) | BB | A | Calibration | 32 | R W | 3/0/0/0/1 | 0/0 | _iqk_afe_setting_8822b, _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:298, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329 |
| 288 | `0x0C60` | *(không có macro)* | BB | A | Init table | 32 | W | 4/0/0/0/17 | 0/0 | _iqk_afe_setting_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:293, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:294, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:308 … |
| 289 | `0x0C64` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/9 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 290 | `0x0C68` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 291 | `0x0C6C` | R_0xc6c (+1: ODM_REG_L1SBD_PD_CH_11N) | BB | A | Calibration | 32 | R W | 3/0/0/0/1 | 0/0 | _iqk_afe_setting_8822b, _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:299, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329 |
| 292 | `0x0C70` | R_0xc70 (+2: ODM_R_AGC_PAR, ODM_REG_BB_PWR_SAV2_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 293 | `0x0C74` | R_0xc74 (+1: ODM_REG_BB_AGC_SET_2_11N) | BB | A | Init table | 32 | R-mask RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 294 | `0x0C78` | R_0xc78 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 295 | `0x0C7C` | R_0xc7c (+2: ODM_REG_RX_OFF_11N, ODM_R_HTSTF_AGC_PAR) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 296 | `0x0C80` | R_0xc80 (+3: rOFDM0_XATxIQImbalance, ODM_REG_TXIQK_MATRIXA_11N, …) | BB | A | Init table | 32 | R-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 297 | `0x0C84` | R_0xc84 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 298 | `0x0C88` | R_0xc88 (+1: ODM_REG_TXIQK_MATRIXB_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 299 | `0x0C8C` | R_0xc8c | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 300 | `0x0C90` | R_0xc90 (+2: rA_LSSIWrite_Jaguar, ODM_TX_PWR_TRAINING_A) | BB | A |  | 32 | RMW-mask | 1/0/0/0/0 | 0/0 | config_phydm_write_rf_reg_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1061 |
| 301 | `0x0C94` | R_0xc94 (+1: ODM_REG_TXIQK_MATRIXA_LSB2_11N) | BB | A | Calibration | 32 | RMW-mask | 4/0/0/2/1 | 0/0 | _iqk_configure_macbb_8822b, _iqk_one_shot_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1007, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1021, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:598 … |
| 302 | `0x0C98` | ODM_TX_PWR_TRAINING_B | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 303 | `0x0C9C` | R_0xc9c (+1: ODM_REG_TXIQK_MATRIXB_LSB2_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 304 | `0x0CA0` | R_0xca0 (+1: ODM_REG_RXIQK_MATRIX_LSB_11N) | BB | A | Antenna/RFE | 32 | RMW-mask | 24/0/0/3/1 | 0/0 | phydm_8822b_type15_rfe, phydm_8822b_type18_rfe, phydm_rfe_4_11, phydm_rfe_efem … | hal/phydm/rtl8822b/phydm_hal_api8822b.c:138, hal/phydm/rtl8822b/phydm_hal_api8822b.c:142, hal/phydm/rtl8822b/phydm_hal_api8822b.c:146 … |
| 305 | `0x0CA4` | R_0xca4 (+1: ODM_REG_ANTDIV_PARA1_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 306 | `0x0CA8` | R_0xca8 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 307 | `0x0CAC` | R_0xcac | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 308 | `0x0CB0` | R_0xcb0 (+2: rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR) | BB | A | Antenna/RFE | 32 | R R-mask RMW-mask W | 18/3/0/14/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b, _iqk_rfe_setting_8822b, phydm_8822b_type15_rfe … | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:517 … |
| 309 | `0x0CB4` | R_0xcb4 (+1: rA_RFE_Inv_Jaguar) | BB | A | Antenna/RFE | 32 | R R-mask RMW-mask W | 20/3/0/15/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b, _iqk_rfe_setting_8822b, ex_hal8822b_wifi_only_hw_config … | hal/btc/halbtc8822bwifionly.c:68, hal/phydm/halrf/rtl8822b/halrf_8822b.c:566, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214 … |
| 310 | `0x0CB8` | R_0xcb8 (+1: rA_RFE_Jaguar) | BB | A | Antenna/RFE | 32 | RMW-mask | 4/0/0/1/1 | 0/0 | phydm_8822b_type18_rfe, phydm_rfe_efem | hal/phydm/rtl8822b/phydm_hal_api8822b.c:114, hal/phydm/rtl8822b/phydm_hal_api8822b.c:122, hal/phydm/rtl8822b/phydm_hal_api8822b.c:428 … |
| 311 | `0x0CBC` | R_0xcbc (+1: rA_RFE_Inverse_Jaguar) | BB | A | Antenna/RFE | 32 | R R-mask RMW-mask W | 29/2/0/7/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b, _iqk_rfe_setting_8822b, _phy_query_rf_path_switch_8822b … | hal/btc/halbtc8822bwifionly.c:119, hal/btc/halbtc8822bwifionly.c:121, hal/btc/halbtc8822bwifionly.c:74 … |
| 312 | `0x0CBD` | R_0xcbd | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 313 | `0x0CBE` | R_0xcbe | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 314 | `0x0CC0` | R_0xcc0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 315 | `0x0CC4` | R_0xcc4 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 316 | `0x0CC8` | R_0xcc8 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 317 | `0x0CCC` | R_0xccc | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 318 | `0x0CD0` | R_0xcd0 | BB | A | Init table | 32 | R-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 319 | `0x0CD4` | R_0xcd4 | BB | A | Init table | 32 | R-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 320 | `0x0CD8` | R_0xcd8 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 321 | `0x0CDC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 322 | `0x0CE0` | R_0xce0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 323 | `0x0CE4` | R_0xce4 (+1: ODM_REG_SMALL_BANDWIDTH_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 324 | `0x0CE8` | R_0xce8 | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 325 | `0x0CEC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 326 | `0x0CF0` | ODM_OFDM_FA_CNT1 (+1: ODM_REG_OFDM_FA_TYPE1_11N) | BB | X |  | 32 | R-mask | 0/0/0/1/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16940 |
| 327 | `0x0CF2` | *(không có macro)* | BB | X |  | 32 | R-mask | 0/0/0/1/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16940 |
| 328 | `0x0D00` | R_0xd00 (+3: rOFDM1_LSTF, ODM_OFDM_PHY0_PAGE_D, …) | BB | X | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 0/0/0/11/0 | 0/0 | hal_mpt_SetCarrierSuppressionTx, hal_mpt_SetSingleCarrierTx, hal_mpt_SetSingleToneTx, mpt_StartCckContTx … | hal/hal_com.c:17001, hal/hal_com.c:17003, hal/hal_mp.c:1736 … |
| 329 | `0x0D04` | R_0xd04 (+3: rA_PIRead_Jaguar, rOFDM1_TRxPathEnable, …) | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | mpt_SetRFPath_819X | hal/hal_mp.c:1478, hal/hal_mp.c:1481 |
| 330 | `0x0D08` | R_0xd08 (+1: rA_SIRead_Jaguar) | BB | R |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 331 | `0x0D0C` | R_0xd0c | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R | 1/0/0/0/0 | 0/0 | phydm_get_cfo_info_ac | hal/phydm/phydm_cfotracking.c:86 |
| 332 | `0x0D10` | R_0xd10 | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R R-mask | 2/0/0/0/0 | 0/0 | phydm_get_cfo_info_ac, phydm_soml_cfo_process | hal/phydm/phydm_cfotracking.c:87, hal/phydm/phydm_soml.c:318 |
| 333 | `0x0D14` | R_0xd14 | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 2/0/1/0/0 | 0/0 | phydm_get_cfo_info_ac, phydm_soml_cfo_process | hal/phydm/phydm_cfotracking.c:88, hal/phydm/phydm_soml.c:319 |
| 334 | `0x0D2C` | R_0xd2c (+1: ODM_REG_BB_ATC_11N) | BB | X | Beamforming/MU | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_csi_mask_enable | hal/phydm/phydm_api.c:804 |
| 335 | `0x0D30` | R_0xd30 | BB | A | TX power | 32 | R-mask | 1/0/0/0/0 | 0/0 | config_phydm_read_txagc_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1130 |
| 336 | `0x0D40` | R_0xd40 | BB | X | Beamforming/MU | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_clean_all_csi_mask | hal/phydm/phydm_api.c:825 |
| 337 | `0x0D44` | R_0xd44 (+1: rB_PIRead_Jaguar) | BB | X | Beamforming/MU | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_clean_all_csi_mask | hal/phydm/phydm_api.c:826 |
| 338 | `0x0D48` | R_0xd48 (+1: rB_SIRead_Jaguar) | BB | X | Beamforming/MU | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_clean_all_csi_mask | hal/phydm/phydm_api.c:827 |
| 339 | `0x0D4C` | R_0xd4c | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R RMW-mask | 1/0/0/1/0 | 0/0 | phydm_get_cfo_info_ac | hal/phydm/phydm_cfotracking.c:91 |
| 340 | `0x0D50` | R_0xd50 | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R R-mask | 2/0/0/0/0 | 0/0 | phydm_get_cfo_info_ac, phydm_soml_cfo_process | hal/phydm/phydm_cfotracking.c:92, hal/phydm/phydm_soml.c:320 |
| 341 | `0x0D54` | R_0xd54 | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 2/0/1/0/0 | 0/0 | phydm_get_cfo_info_ac, phydm_soml_cfo_process | hal/phydm/phydm_cfotracking.c:93, hal/phydm/phydm_soml.c:321 |
| 342 | `0x0D5C` | R_0xd5c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 343 | `0x0D6C` | R_0xd6c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 344 | `0x0D7C` | R_0xd7c | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 345 | `0x0D80` | R_0xd80 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 346 | `0x0D84` | R_0xd84 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 347 | `0x0D8C` | R_0xd8c | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 348 | `0x0D90` | R_0xd90 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 349 | `0x0D94` | R_0xd94 | BB | G |  | 32 | R-mask | 0/0/1/0/0 | 0/0 | phydm_bb_hw_dbg_info_ac | hal/phydm/phydm_debug.c:532 |
| 350 | `0x0DA0` | ODM_OFDM_FA_CNT2 (+1: ODM_REG_OFDM_FA_TYPE2_11N) | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 351 | `0x0DA2` | *(không có macro)* | BB | X |  | 32 | R-mask | 0/0/0/1/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16941 |
| 352 | `0x0DA4` | ODM_OFDM_FA_CNT3 (+1: ODM_REG_OFDM_FA_TYPE3_11N) | BB | X |  | 32 | R-mask | 0/0/0/1/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16941 |
| 353 | `0x0DA6` | *(không có macro)* | BB | X |  | 32 | R-mask | 0/0/0/1/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16942 |
| 354 | `0x0DA8` | ODM_OFDM_FA_CNT4 (+1: ODM_REG_OFDM_FA_TYPE4_11N) | BB | X |  | 32 | R-mask | 0/0/0/1/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16942 |
| 355 | `0x0DAC` | R_0xdac | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 356 | `0x0DB0` | R_0xdb0 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 357 | `0x0DB4` | R_0xdb4 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 358 | `0x0DB8` | R_0xdb8 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 359 | `0x0DBC` | R_0xdbc | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 360 | `0x0DCC` | R_0xdcc | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 361 | `0x0DD0` | R_0xdd0 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 362 | `0x0DD4` | R_0xdd4 | BB | G |  | 32 | R-mask | 0/0/1/0/0 | 0/0 | phydm_bb_hw_dbg_info_ac | hal/phydm/phydm_debug.c:536 |
| 363 | `0x0DD8` | R_0xdd8 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 364 | `0x0DE0` | R_0xde0 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 365 | `0x0DEC` | R_0xdec | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 366 | `0x0DF4` | R_0xdf4 (+1: ODM_REG_RPT_11N) | BB | X |  | 32 | R-mask | 0/0/0/1/0 | 0/0 | phydm_get_bb_dbg_port_val | hal/phydm/phydm_debug.c:211 |
| 367 | `0x0E00` | R_0xe00 (+4: ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06, …) | BB | A | Calibration | 32 | R RMW-mask W | 10/9/0/1/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_configure_macbb_8822b, _iqk_restore_mac_bb_8822b, _phy_lc_calibrate_8822b … | hal/phydm/halrf/rtl8822b/halrf_8822b.c:480, hal/phydm/halrf/rtl8822b/halrf_8822b.c:482, hal/phydm/halrf/rtl8822b/halrf_8822b.c:504 … |
| 368 | `0x0E04` | R_0xe04 (+4: ODM_TXAGC_A_24_54, rTxAGC_A_Rate54_24, …) | BB | A | Channel/BW | 32 | RMW-mask | 8/0/0/1/1 | 0/0 | config_phydm_switch_band_8822b, phydm_dynamic_parameters_ota, phydm_set_weighting_cmn | hal/phydm/phydm_api.c:2881, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1632, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1650 … |
| 369 | `0x0E08` | R_0xe08 (+5: ODM_REG_ANT_11AC_B, ODM_TXAGC_A_1_MCS32, …) | BB | A | TX/RX DMA/Queue | 32 | RMW-mask | 2/0/0/1/1 | 0/0 | config_phydm_trx_mode_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:2508, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2510 |
| 370 | `0x0E0C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 371 | `0x0E10` | R_0xe10 (+4: rTxAGC_A_Mcs03_Mcs00, ODM_TXAGC_A_MCS0_MCS3, …) | BB | A | Init table | 32 | RMW-mask | 2/0/0/1/1 | 0/0 | phydm_dc_cancellation | hal/phydm/phydm.c:3368, hal/phydm/phydm.c:3370 |
| 372 | `0x0E14` | R_0xe14 (+4: rTxAGC_A_Mcs07_Mcs04, ODM_TXAGC_A_MCS4_MCS7, …) | BB | A | Init table | 32 | RMW-mask | 2/0/0/1/1 | 0/0 | phydm_dc_cancellation | hal/phydm/phydm.c:3372, hal/phydm/phydm.c:3374 |
| 373 | `0x0E18` | R_0xe18 (+4: rTxAGC_A_Mcs11_Mcs08, ODM_TXAGC_A_MCS8_MCS11, …) | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 374 | `0x0E1C` | R_0xe1c (+6: rB_TxScale_Jaguar, rTxAGC_A_Mcs15_Mcs12, …) | BB | A | Init table | 32 | R-mask RMW-mask | 6/3/0/1/1 | 0/0 | phydm_rxagc_switch_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1792, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1798, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1804 … |
| 375 | `0x0E20` | R_0xe20 | BB | A | Channel/BW | 32 | RMW-mask | 5/0/0/3/1 | 0/0 | phydm_nbi_enable, phydm_rxdfirpar_by_bw_8822b, phydm_spur_calibration_8822b | hal/phydm/phydm_api.c:1006, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1578, hal/phydm/rtl8822b/phydm_hal_api8822b.c:883 … |
| 376 | `0x0E24` | R_0xe24 (+2: ODM_REG_TAP_UPD_97F, ODM_REG_EDCCA_DCNF_11N) | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 377 | `0x0E28` | R_0xe28 (+3: ODM_REG_NHM_TH8_11N, ODM_REG_PAGE_B1_97F, …) | BB | A | Init table | 32 | RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 378 | `0x0E2C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 379 | `0x0E30` | R_0xe30 (+1: ODM_REG_TXIQK_TONE_A_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 380 | `0x0E34` | R_0xe34 (+1: ODM_REG_RXIQK_TONE_A_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 381 | `0x0E38` | R_0xe38 (+1: ODM_REG_TXIQK_PI_A_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 382 | `0x0E3C` | R_0xe3c (+1: ODM_REG_RXIQK_PI_A_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 383 | `0x0E40` | R_0xe40 (+1: ODM_REG_TXIQK_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 384 | `0x0E44` | R_0xe44 (+1: ODM_REG_RXIQK_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 385 | `0x0E48` | R_0xe48 (+1: ODM_REG_IQK_AGC_PTS_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 386 | `0x0E4C` | R_0xe4c (+1: ODM_REG_IQK_AGC_RSP_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 387 | `0x0E50` | R_0xe50 (+2: rB_IGI_Jaguar, ODM_REG_IGI_B_11AC) | BB | A | Init table | 32 | R R-mask RMW-mask | 6/0/0/0/9 | 0/0 | mcc_get_reg_hdl, phydm_get_igi_reg_val, phydm_idle_noise_measure_ac, phydm_igi_toggle_8822b … | hal/hal_mcc.c:2748, hal/phydm/phydm_dig.c:546, hal/phydm/phydm_dig.c:667 … |
| 388 | `0x0E54` | R_0xe54 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 389 | `0x0E58` | *(không có macro)* | BB | A | Calibration | 32 | R W | 3/0/0/0/1 | 0/0 | _iqk_afe_setting_8822b, _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:300, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329 |
| 390 | `0x0E5C` | R_0xe5c | BB | A | Calibration | 32 | R W | 3/0/0/0/1 | 0/0 | _iqk_afe_setting_8822b, _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:301, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329 |
| 391 | `0x0E60` | *(không có macro)* | BB | A | Init table | 32 | W | 4/0/0/0/17 | 0/0 | _iqk_afe_setting_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:295, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:296, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:310 … |
| 392 | `0x0E64` | R_0xe64 | BB | A | Init table | 32 |  | 0/0/0/0/9 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 393 | `0x0E68` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 394 | `0x0E6C` | R_0xe6c (+1: ODM_REG_BLUETOOTH_11N) | BB | A | Calibration | 32 | R W | 3/0/0/0/1 | 0/0 | _iqk_afe_setting_8822b, _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:302, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329 |
| 395 | `0x0E70` | R_0xe70 (+1: ODM_REG_RX_WAIT_CCA_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 396 | `0x0E74` | R_0xe74 (+1: ODM_REG_TX_CCK_RFON_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 397 | `0x0E78` | R_0xe78 (+1: ODM_REG_TX_CCK_BBON_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 398 | `0x0E7C` | R_0xe7c (+1: ODM_REG_OFDM_RFON_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 399 | `0x0E80` | R_0xe80 (+1: ODM_REG_OFDM_BBON_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 400 | `0x0E84` | R_0xe84 (+1: ODM_REG_TX2RX_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 401 | `0x0E88` | R_0xe88 (+1: ODM_REG_TX2TX_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 402 | `0x0E8C` | R_0xe8c (+1: ODM_REG_RX_CCK_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 403 | `0x0E90` | R_0xe90 (+1: rB_LSSIWrite_Jaguar) | BB | A |  | 32 | RMW-mask | 1/0/0/0/0 | 0/0 | config_phydm_write_rf_reg_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1061 |
| 404 | `0x0E94` | R_0xe94 | BB | A | Calibration | 32 | RMW-mask | 4/0/0/2/1 | 0/0 | _iqk_configure_macbb_8822b, _iqk_one_shot_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1007, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1021, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:599 … |
| 405 | `0x0E98` | R_0xe98 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 406 | `0x0E9C` | R_0xe9c | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 407 | `0x0EA0` | R_0xea0 | BB | A | Antenna/RFE | 32 | RMW-mask | 24/0/0/3/1 | 0/0 | phydm_8822b_type15_rfe, phydm_8822b_type18_rfe, phydm_rfe_4_11, phydm_rfe_efem … | hal/phydm/rtl8822b/phydm_hal_api8822b.c:139, hal/phydm/rtl8822b/phydm_hal_api8822b.c:143, hal/phydm/rtl8822b/phydm_hal_api8822b.c:147 … |
| 408 | `0x0EA4` | R_0xea4 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 409 | `0x0EA8` | R_0xea8 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 410 | `0x0EAC` | R_0xeac | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 411 | `0x0EB0` | R_0xeb0 (+1: rB_RFE_Pinmux_Jaguar) | BB | A | Antenna/RFE | 32 | R R-mask RMW-mask W | 18/3/0/11/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b, _iqk_rfe_setting_8822b, phydm_8822b_type15_rfe … | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:520 … |
| 412 | `0x0EB4` | R_0xeb4 (+1: rB_RFE_Inv_Jaguar) | BB | A | Antenna/RFE | 32 | R R-mask RMW-mask W | 18/3/0/13/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b, _iqk_rfe_setting_8822b, phydm_8822b_type15_rfe … | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:521 … |
| 413 | `0x0EB8` | R_0xeb8 (+1: rB_RFE_Jaguar) | BB | A | Antenna/RFE | 32 | RMW-mask | 4/0/0/1/1 | 0/0 | phydm_8822b_type18_rfe, phydm_rfe_efem | hal/phydm/rtl8822b/phydm_hal_api8822b.c:115, hal/phydm/rtl8822b/phydm_hal_api8822b.c:123, hal/phydm/rtl8822b/phydm_hal_api8822b.c:429 … |
| 414 | `0x0EBC` | R_0xebc (+1: rB_RFE_Inverse_Jaguar) | BB | A | Antenna/RFE | 32 | R RMW-mask W | 20/2/0/4/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b, _iqk_rfe_setting_8822b, phydm_8822b_type15_rfe … | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:522 … |
| 415 | `0x0EC0` | R_0xec0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 416 | `0x0EC4` | R_0xec4 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 417 | `0x0EC8` | R_0xec8 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 418 | `0x0ECC` | R_0xecc | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 419 | `0x0ED0` | R_0xed0 (+1: ODM_REG_RX_OFDM_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 420 | `0x0ED4` | R_0xed4 (+1: ODM_REG_RX_WAIT_RIFS_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 421 | `0x0ED8` | R_0xed8 (+1: ODM_REG_RX2RX_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 422 | `0x0EDC` | R_0xedc (+1: ODM_REG_STANDBY_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 423 | `0x0EE0` | R_0xee0 (+1: ODM_REG_SLEEP_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 424 | `0x0EE4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 425 | `0x0EE8` | R_0xee8 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 426 | `0x0EEC` | R_0xeec (+1: ODM_REG_PMPD_ANAEN_11N) | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 427 | `0x0F00` | R_0xf00 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 428 | `0x0F04` | R_0xf04 (+1: ODM_REG_CCK_CRC32_CNT_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/2/0/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1504 |
| 429 | `0x0F08` | R_0xf08 (+1: ODM_REG_CCK_CCA_CNT_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/1/0/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1499 |
| 430 | `0x0F0C` | R_0xf0c (+1: ODM_REG_VHT_CRC32_CNT_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/2/0/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1519 |
| 431 | `0x0F10` | R_0xf10 (+1: ODM_REG_HT_CRC32_CNT_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/2/0/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1514 |
| 432 | `0x0F14` | R_0xf14 (+2: ODM_REG_PAGE_F_RST_11N, ODM_REG_OFDM_CRC32_CNT_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask RMW-mask | 1/0/2/6/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1509 |
| 433 | `0x0F20` | R_0xf20 | BB | G |  | 32 | R-mask | 0/0/1/0/0 | 0/0 | phydm_bb_hw_dbg_info_ac | hal/phydm/phydm_debug.c:617 |
| 434 | `0x0F2C` | R_0xf2c | BB | G |  | 32 | R-mask | 0/0/2/0/0 | 0/0 | phydm_bb_hw_dbg_info_ac | hal/phydm/phydm_debug.c:636, hal/phydm/phydm_debug.c:673 |
| 435 | `0x0F30` | R_0xf30 | BB | G |  | 32 | R-mask | 0/0/2/0/0 | 0/0 | phydm_bb_hw_dbg_info_ac | hal/phydm/phydm_debug.c:648, hal/phydm/phydm_debug.c:693 |
| 436 | `0x0F34` | R_0xf34 | BB | G |  | 32 | R-mask | 0/0/1/0/0 | 0/0 | phydm_bb_hw_dbg_info_ac | hal/phydm/phydm_debug.c:714 |
| 437 | `0x0F44` | R_0xf44 | BB | A |  | 32 | R-mask | 2/0/0/0/0 | 0/0 | phydm_dynamic_spur_det_eliminate | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1473, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1495 |
| 438 | `0x0F48` | R_0xf48 (+1: ODM_REG_OFDM_FA_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/1/0/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1493 |
| 439 | `0x0F4C` | R_0xf4c | BB | A | Beamforming/MU | 32 | R-mask | 1/0/0/0/0 | 0/0 | _sounding_config_mu | hal/rtl8822b/rtl8822b_phy.c:1521 |
| 440 | `0x0F50` | R_0xf50 | BB | M |  | 32 | R-mask | 0/3/0/0/0 | 0/0 | hal_mpt_query_phytxok, mpt_ProSetPMacTx | hal/hal_mp.c:2093, hal/hal_mp.c:2095, hal/hal_mp.c:2349 |
| 441 | `0x0F54` | R_0xf54 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 442 | `0x0F58` | R_0xf58 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 443 | `0x0F5C` | R_0xf5c | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 444 | `0x0F70` | R_0xf70 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 445 | `0x0F74` | R_0xf74 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 446 | `0x0F80` | R_0xf80 | BB | G |  | 32 | R-mask | 0/0/2/0/0 | 0/0 | phydm_bb_hw_dbg_info_ac, phydm_dbg_port_dump_ac | hal/phydm/phydm_auto_dbg.c:397, hal/phydm/phydm_debug.c:479 |
| 447 | `0x0F84` | R_0xf84 (+2: ODM_REG_IGI_C_11N, ODM_REG_CCK_CRC32_ERROR_CNT_11N) | BB | A |  | 32 | R-mask | 1/0/0/1/0 | 0/0 | phydm_get_condi_num_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:972 |
| 448 | `0x0F87` | R_0xf87 | BB | A |  | 8 | R | 1/0/0/0/0 | 0/0 | phydm_get_condi_num_acc_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:953 |
| 449 | `0x0F88` | R_0xf88 (+2: ODM_REG_IGI_D_11N, ODM_REG_CCK_CRC32_OK_CNT_11N) | BB | G |  | 32 | R-mask | 0/0/1/1/0 | 0/0 | phydm_bb_hw_dbg_info_ac | hal/phydm/phydm_debug.c:584 |
| 450 | `0x0F8C` | R_0xf8c | BB | G |  | 32 | R R-mask | 0/0/8/1/0 | 0/0 | phydm_bb_hw_dbg_info_ac, phydm_per_tone_evm | hal/phydm/phydm_debug.c:4183, hal/phydm/phydm_debug.c:4210, hal/phydm/phydm_debug.c:4237 … |
| 451 | `0x0F90` | R_0xf90 (+1: ODM_REG_HT_CRC32_CNT_11N) | BB | G |  | 32 | R-mask | 0/0/1/2/0 | 0/0 | phydm_bb_hw_dbg_info_ac | hal/phydm/phydm_debug.c:519 |
| 452 | `0x0F94` | R_0xf94 (+1: ODM_REG_OFDM_CRC32_CNT_11N) | BB | A | Antenna/RFE | 32 | R-mask | 3/0/0/2/0 | 0/0 | phydm_dynamic_ant_weighting_8822b | hal/phydm/rtl8822b/phydm_rtl8822b.c:477, hal/phydm/rtl8822b/phydm_rtl8822b.c:485, hal/phydm/rtl8822b/phydm_rtl8822b.c:493 |
| 453 | `0x0F98` | R_0xf98 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 454 | `0x0FA0` | R_0xfa0 (+1: ODM_REG_RPT_11AC) | BB | A | Calibration | 32 | R-mask | 1/0/17/1/0 | 0/0 | _iqk_bb_reset_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:262 |
| 455 | `0x0FA4` | R_0xfa4 (+1: ODM_REG_CLM_RESULT_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/0/0/0 | 0/0 | phydm_clm_get_result | hal/phydm/phydm_ccx.c:1774 |
| 456 | `0x0FA8` | R_0xfa8 (+1: ODM_REG_NHM_CNT_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R | 1/0/0/0/0 | 0/0 | phydm_nhm_get_result | hal/phydm/phydm_ccx.c:610 |
| 457 | `0x0FAC` | R_0xfac (+1: ODM_REG_NHM_CNT7_TO_CNT4_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R | 1/0/0/0/0 | 0/0 | phydm_nhm_get_result | hal/phydm/phydm_ccx.c:613 |
| 458 | `0x0FB0` | R_0xfb0 (+1: ODM_REG_NHM_CNT11_TO_CNT8_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R | 1/0/0/0/0 | 0/0 | phydm_nhm_get_result | hal/phydm/phydm_ccx.c:616 |
| 459 | `0x0FB4` | R_0xfb4 (+1: ODM_REG_NHM_DUR_READY_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R | 1/0/0/0/0 | 0/0 | phydm_nhm_get_result | hal/phydm/phydm_ccx.c:620 |
| 460 | `0x0FB8` | R_0xfb8 (+1: ODM_REG_HT_CRC32_CNT_11N_AGG) | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 461 | `0x0FBC` | R_0xfbc (+1: ODM_REG_OFDM_FA_TYPE3_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/1/0/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1477 |
| 462 | `0x0FC0` | R_0xfc0 (+1: ODM_REG_OFDM_FA_TYPE4_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/1/0/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1481 |
| 463 | `0x0FC4` | R_0xfc4 (+1: ODM_REG_OFDM_FA_TYPE5_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/1/0/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1485 |
| 464 | `0x0FC8` | R_0xfc8 (+1: ODM_REG_OFDM_FA_TYPE6_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/1/0/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1489 |
| 465 | `0x0FCC` | R_0xfcc (+1: ODM_REG_OFDM_FA_TYPE1_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/1/0/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1471 |
| 466 | `0x0FD0` | R_0xfd0 (+1: ODM_REG_OFDM_FA_TYPE2_11AC) | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/1/0/0 | 0/0 | phydm_fa_cnt_statistics_ac | hal/phydm/phydm_dig.c:1474 |
| 467 | `0x0FF0` | R_0xff0 | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 1/0/0/0/0 | 0/0 | phydm_idle_noise_measure_ac | hal/phydm/phydm_noisemonitor.c:222 |
| 468 | `0x1800` | R_0x1800 | BB | M | BB DM (DIG/CCA/FA/CCX) | 32 | R RMW-mask W | 0/6/0/0/0 | 0/0 | rtw_mp_psd_close, rtw_mp_psd_init | core/rtw_mp.c:2257, core/rtw_mp.c:2260, core/rtw_mp.c:2277 … |
| 469 | `0x1804` | R_0x1804 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 470 | `0x1808` | R_0x1808 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 471 | `0x180C` | R_0x180c | BB | X |  | 32 | RMW-mask | 0/0/0/4/0 | 0/0 | phydm_stop_3_wire | hal/phydm/phydm_api.c:555, hal/phydm/phydm_api.c:556, hal/phydm/phydm_api.c:584 … |
| 472 | `0x1810` | R_0x1810 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 473 | `0x1814` | R_0x1814 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 474 | `0x1818` | R_0x1818 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 475 | `0x181C` | R_0x181c (+2: rC_TxScale_Jaguar2, REG_C_TX_SCALE_JAGUAR2) | BB | M |  | 32 | R-mask RMW-mask | 0/3/0/0/0 | 0/0 | mpt_SetSingleTone_8814A | hal/hal_mp.c:1012, hal/hal_mp.c:927, hal/hal_mp.c:973 |
| 476 | `0x1830` | R_0x1830 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 477 | `0x1834` | R_0x1834 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 478 | `0x1838` | R_0x1838 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 479 | `0x183C` | R_0x183c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 480 | `0x1840` | R_0x1840 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 481 | `0x1844` | R_0x1844 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 482 | `0x1848` | R_0x1848 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 483 | `0x1850` | rC_IGI_Jaguar2 (+1: ODM_REG_IGI_C_11AC) | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 484 | `0x1860` | R_0x1860 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 485 | `0x1864` | R_0x1864 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 486 | `0x1868` | R_0x1868 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 487 | `0x186C` | R_0x186c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 488 | `0x1870` | R_0x1870 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 489 | `0x1880` | R_0x1880 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 490 | `0x1884` | R_0x1884 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 491 | `0x188C` | R_0x188c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 492 | `0x1894` | R_0x1894 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 493 | `0x189C` | R_0x189c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 494 | `0x18A0` | R_0x18a0 | BB | X | TX power | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_reset_txagc | hal/phydm/phydm_api.c:2008 |
| 495 | `0x18A4` | R_0x18a4 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 496 | `0x18A8` | R_0x18a8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 497 | `0x18AC` | R_0x18ac | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 498 | `0x18B4` | rC_RFE_Pinmux_Jaguar | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 499 | `0x18E0` | R_0x18e0 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 500 | `0x18E8` | R_0x18e8 | BB | X | TX power | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_reset_txagc | hal/phydm/phydm_api.c:2009 |
| 501 | `0x18EC` | R_0x18ec | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 502 | `0x18F0` | R_0x18f0 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 503 | `0x18F8` | R_0x18f8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 504 | `0x18FC` | R_0x18fc | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 505 | `0x1900` | R_0x1900 | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 506 | `0x1904` | R_0x1904 | BB | A | Antenna/RFE | 32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_config_rx_path_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:2453, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2460 |
| 507 | `0x1908` | R_0x1908 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 508 | `0x190C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 509 | `0x1910` | R_0x1910 | BB | A | Init table | 32 | RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 510 | `0x1914` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 511 | `0x1918` | R_0x1918 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 512 | `0x191C` | R_0x191c | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 513 | `0x1920` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 514 | `0x1924` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 515 | `0x1928` | R_0x1928 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 516 | `0x192C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 517 | `0x1930` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 518 | `0x1934` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 519 | `0x1938` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 520 | `0x193C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 521 | `0x1940` | R_0x1940 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 522 | `0x1944` | R_0x1944 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 523 | `0x1948` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 524 | `0x194C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 525 | `0x1950` | R_0x1950 | BB | A | Init table | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | phydm_pathb_q_matrix_rotate | hal/phydm/phydm_direct_bf.c:233 |
| 526 | `0x1954` | R_0x1954 | BB | A | Init table | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | phydm_pathb_q_matrix_rotate | hal/phydm/phydm_direct_bf.c:232 |
| 527 | `0x1958` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 528 | `0x195C` | R_0x195c | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 529 | `0x1960` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 530 | `0x1964` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 531 | `0x1968` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 532 | `0x196C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 533 | `0x1970` | R_0x1970 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 534 | `0x1974` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 535 | `0x1978` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 536 | `0x197C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 537 | `0x1980` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 538 | `0x1984` | R_0x1984 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 539 | `0x1988` | R_0x1988 | BB | A | Init table | 32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_get_condi_num_8822b, phydm_get_condi_num_acc_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:952, hal/phydm/rtl8822b/phydm_hal_api8822b.c:969 |
| 540 | `0x198C` | R_0x198c | BB | A | Calibration | 32 | RMW-mask | 1/0/2/0/1 | 0/0 | _iqk_bb_reset_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:261 |
| 541 | `0x1990` | R_0x1990 (+1: rA_RFE_Sel_Jaguar2) | BB | A | Calibration | 32 | R RMW-mask W | 6/0/0/11/1 | 0/0 | _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b, ex_hal8822b_wifi_only_hw_config, phy_set_rf_path_switch_8822b … | hal/btc/halbtc8822bwifionly.c:72, hal/phydm/halrf/rtl8822b/halrf_8822b.c:559, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214 … |
| 542 | `0x1991` | R_0x1991 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 543 | `0x1994` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 544 | `0x1998` | R_0x1998 | BB | A | TX power | 32 | RMW-mask | 4/0/0/0/1 | 0/0 | config_phydm_read_txagc_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1121, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1124, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1127 … |
| 545 | `0x199C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 546 | `0x19A0` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 547 | `0x19A4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 548 | `0x19A8` | R_0x19a8 | BB | A | Channel/BW | 32 | R-mask RMW-mask | 4/0/0/0/1 | 0/0 | config_phydm_switch_band_8822b, phydm_somlrxhp_setting | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1627, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1702, hal/phydm/rtl8822b/phydm_rtl8822b.c:382 … |
| 549 | `0x19AC` | REG_BB_TXBF_ANT_SET_BF1_8822B | BB | A | Beamforming/MU | 32 | RMW-mask | 6/0/0/0/1 | 0/0 | phydm_8822btxbf_rfmode | hal/phydm/txbf/haltxbf8822b.c:1073, hal/phydm/txbf/haltxbf8822b.c:1077, hal/phydm/txbf/haltxbf8822b.c:1078 … |
| 550 | `0x19B0` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 551 | `0x19B4` | REG_BB_TXBF_ANT_SET_BF0_8822B | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 552 | `0x19B8` | R_0x19b8 | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 553 | `0x19BC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 554 | `0x19C0` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 555 | `0x19C4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 556 | `0x19C8` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 557 | `0x19CC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 558 | `0x19D0` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 559 | `0x19D4` | R_0x19d4 | BB | A | Init table | 32 | RMW-mask | 4/0/0/0/1 | 0/0 | phydm_dynamic_parameters_ota | hal/phydm/rtl8822b/phydm_rtl8822b.c:105, hal/phydm/rtl8822b/phydm_rtl8822b.c:116, hal/phydm/rtl8822b/phydm_rtl8822b.c:91 … |
| 560 | `0x19D8` | R_0x19d8 | BB | A | Init table | 32 | RMW-mask | 4/0/0/0/1 | 0/0 | phydm_dynamic_parameters_ota | hal/phydm/rtl8822b/phydm_rtl8822b.c:104, hal/phydm/rtl8822b/phydm_rtl8822b.c:115, hal/phydm/rtl8822b/phydm_rtl8822b.c:90 … |
| 561 | `0x19DC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 562 | `0x19E0` | R_0x19e0 | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 563 | `0x19E4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 564 | `0x19E8` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 565 | `0x19EC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 566 | `0x19F0` | R_0x19f0 | BB | A | Beamforming/MU | 32 | RMW-mask | 6/0/0/0/1 | 0/0 | phydm_8822b_sutxbfer_workaroud | hal/phydm/txbf/haltxbf8822b.c:1106, hal/phydm/txbf/haltxbf8822b.c:1108, hal/phydm/txbf/haltxbf8822b.c:1110 … |
| 567 | `0x19F4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 568 | `0x19F8` | R_0x19f8 | BB | A | Beamforming/MU | 32 | RMW-mask | 4/0/0/2/1 | 0/0 | phydm_8822b_sutxbfer_workaroud | hal/phydm/txbf/haltxbf8822b.c:1101, hal/phydm/txbf/haltxbf8822b.c:1102, hal/phydm/txbf/haltxbf8822b.c:1103 … |
| 569 | `0x19FC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 570 | `0x1A00` | R_0x1a00 | BB | M | BB DM (DIG/CCA/FA/CCX) | 32 | R RMW-mask W | 0/3/0/0/0 | 0/0 | rtw_mp_psd_close, rtw_mp_psd_init | core/rtw_mp.c:2278, core/rtw_mp.c:2282, core/rtw_mp.c:2342 |
| 571 | `0x1A04` | R_0x1a04 | BB | X | TX/RX DMA/Queue | 32 | R-mask RMW-mask | 0/0/0/3/0 | 0/0 | phydm_dis_cck_trx | hal/phydm/phydm_api.c:724, hal/phydm/phydm_api.c:729, hal/phydm/phydm_api.c:750 |
| 572 | `0x1A14` | R_0x1a14 | BB | X | TX/RX DMA/Queue | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | phydm_dis_cck_trx | hal/phydm/phydm_api.c:727, hal/phydm/phydm_api.c:748 |
| 573 | `0x1A1C` | rD_TxScale_Jaguar2 (+1: REG_D_TX_SCALE_JAGUAR2) | BB | M |  | 32 | R-mask RMW-mask | 0/3/0/0/0 | 0/0 | mpt_SetSingleTone_8814A | hal/hal_mp.c:1013, hal/hal_mp.c:928, hal/hal_mp.c:974 |
| 574 | `0x1A20` | R_0x1a20 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 575 | `0x1A24` | R_0x1a24 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 576 | `0x1A28` | R_0x1a28 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 577 | `0x1A2C` | R_0x1a2c | BB | X | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 0/0/0/6/0 | 0/0 | phydm_false_alarm_counter_reg_hold, rtw_reset_phy_rx_counters | hal/hal_com.c:16980, hal/hal_com.c:16981, hal/hal_com.c:16984 … |
| 578 | `0x1A50` | rD_IGI_Jaguar2 (+1: ODM_REG_IGI_D_11AC) | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 579 | `0x1A5C` | R_0x1a5c | BB | X |  | 32 | R-mask | 0/0/0/1/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16928 |
| 580 | `0x1A70` | R_0x1a70 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 581 | `0x1A74` | R_0x1a74 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 582 | `0x1A80` | R_0x1a80 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 583 | `0x1A84` | R_0x1a84 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 584 | `0x1A8C` | R_0x1a8c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 585 | `0x1A94` | R_0x1a94 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 586 | `0x1A98` | R_0x1a98 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 587 | `0x1A9C` | R_0x1a9c | BB | R |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 588 | `0x1AA0` | R_0x1aa0 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 589 | `0x1AA8` | R_0x1aa8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 590 | `0x1AAC` | R_0x1aac | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 591 | `0x1AB0` | R_0x1ab0 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 592 | `0x1AB4` | rD_RFE_Pinmux_Jaguar | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 593 | `0x1ABC` | R_0x1abc | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 594 | `0x1AC0` | R_0x1ac0 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 595 | `0x1AC8` | R_0x1ac8 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 596 | `0x1ACC` | R_0x1acc | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 597 | `0x1AD0` | R_0x1ad0 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 598 | `0x1AD4` | R_0x1ad4 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 599 | `0x1AE8` | R_0x1ae8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 600 | `0x1AEC` | R_0x1aec | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 601 | `0x1B00` | R_0x1b00 | BB | A | Calibration | 32 | R RMW-mask W | 35/0/2/16/6 | 0/0 | _iqk_agc_bnd_int_8822b, _iqk_get_rxk1_8822b, _iqk_one_shot_8822b, _iqk_page_switch … | hal/phydm/halrf/halrf.c:87, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1003, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1341 … |
| 602 | `0x1B04` | R_0x1b04 | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 603 | `0x1B08` | R_0x1b08 | BB | A | Calibration | 32 | R R-mask RMW-mask | 2/0/0/1/1 | 0/0 | _iqk_check_cal_8822b, _iqk_one_shot_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:793, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:990 |
| 604 | `0x1B0C` | R_0x1b0c | BB | A | Interrupt | 32 | R RMW-mask W | 9/0/6/4/1 | 0/0 | _iqk_rximr_selfcheck_8822b, _iqk_tximr_selfcheck_8822b, phy_get_iqk_cfir_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1369, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1375, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1402 … |
| 605 | `0x1B10` | R_0x1b10 | BB | A | Init table | 32 | W | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 606 | `0x1B14` | R_0x1b14 | BB | A | Interrupt | 32 | R RMW-mask W | 6/0/0/11/1 | 0/0 | _iqk_get_rxk1_8822b, _iqk_rximr_selfcheck_8822b, _iqk_tximr_selfcheck_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1370, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1403, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1452 … |
| 607 | `0x1B15` | *(không có macro)* | BB | A | Interrupt | 8 | W | 2/0/0/0/0 | 0/0 | _iqk_rximr_selfcheck_8822b, _iqk_tximr_selfcheck_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1377, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1458 |
| 608 | `0x1B18` | R_0x1b18 | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 609 | `0x1B1C` | R_0x1b1c | BB | A | Interrupt | 32 | R R-mask RMW-mask W | 11/0/0/17/2 | 0/0 | _iqk_get_rxk1_8822b, _iqk_rximr_selfcheck_8822b, _iqk_rximr_test_8822b, _iqk_tximr_selfcheck_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1371, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1376, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1404 … |
| 610 | `0x1B20` | R_0x1b20 | BB | A | Interrupt | 32 | R RMW-mask W | 13/0/0/2/2 | 0/0 | _iqk_rximr_rxk1_test_8822b, _iqk_rximr_selfcheck_8822b, _iqk_rxk1_setting_8822b, _iqk_rxk2_setting_8822b … | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1342, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1343, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1372 … |
| 611 | `0x1B23` | R_0x1b23 | BB | X | Calibration | 8 | W | 0/0/0/1/0 | 0/0 | _iqk_lok_setting_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:620 |
| 612 | `0x1B24` | R_0x1b24 | BB | A | Calibration | 32 | R RMW-mask W | 16/0/0/1/2 | 0/0 | _iqk_one_shot_8822b, _iqk_rximr_rxk1_test_8822b, _iqk_rximr_selfcheck_8822b, _iqk_rxk1_setting_8822b … | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1344, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1345, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1373 … |
| 613 | `0x1B28` | R_0x1b28 | BB | A | Init table | 32 | RMW-mask | 0/0/0/2/2 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 614 | `0x1B2B` | *(không có macro)* | BB | A | Calibration | 8 | W | 6/0/0/2/0 | 0/0 | _iqk_rxk1_setting_8822b, _iqk_rxk2_setting_8822b, _iqk_txk_setting_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:670, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:677, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:692 … |
| 615 | `0x1B2C` | R_0x1b2c | BB | A | Interrupt | 32 | R-mask RMW-mask W | 2/0/2/5/2 | 0/0 | _iqk_rximr_selfcheck_8822b, _iqk_tximr_selfcheck_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1380, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1462 |
| 616 | `0x1B30` | R_0x1b30 | BB | A | Init table | 32 | RMW-mask | 0/0/0/1/2 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 617 | `0x1B34` | R_0x1b34 | BB | A | Interrupt | 32 | RMW-mask W | 4/0/0/2/2 | 0/0 | _iqk_rximr_selfcheck_8822b, _iqk_tximr_selfcheck_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1392, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1393, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1472 … |
| 618 | `0x1B38` | R_0x1b38 | BB | A | Interrupt | 32 | R RMW-mask W | 5/0/2/8/0 | 0/0 | _iqk_get_rxk1_8822b, _iqk_one_shot_8822b, _iqk_rximr_selfcheck_8822b, _iqk_rximr_test_8822b … | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1018, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1381, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1463 … |
| 619 | `0x1B3C` | R_0x1b3c | BB | A | Interrupt | 32 | R RMW-mask W | 3/0/2/1/2 | 0/0 | _iqk_rximr_selfcheck_8822b, _iqk_rxk_gsearch_fail_8822b, _iqk_tximr_selfcheck_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1382, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1464, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:857 |
| 620 | `0x1B40` | R_0x1b40 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 621 | `0x1B44` | R_0x1b44 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 622 | `0x1B48` | R_0x1b48 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 623 | `0x1B4C` | R_0x1b4c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 624 | `0x1B50` | R_0x1b50 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 625 | `0x1B54` | R_0x1b54 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 626 | `0x1B58` | R_0x1b58 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 627 | `0x1B5C` | R_0x1b5c | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 628 | `0x1B60` | R_0x1b60 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 629 | `0x1B64` | R_0x1b64 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 630 | `0x1B67` | R_0x1b67 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 631 | `0x1B68` | R_0x1b68 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 632 | `0x1B6C` | R_0x1b6c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 633 | `0x1B70` | R_0x1b70 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 634 | `0x1B74` | R_0x1b74 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 635 | `0x1B78` | R_0x1b78 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 636 | `0x1B7C` | R_0x1b7c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 637 | `0x1B80` | R_0x1b80 | BB | A | Init table | 32 |  | 0/0/0/0/975 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 638 | `0x1B83` | R_0x1b83 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 639 | `0x1B84` | R_0x1b84 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 640 | `0x1B88` | R_0x1b88 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 641 | `0x1B8C` | R_0x1b8c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 642 | `0x1B90` | R_0x1b90 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 643 | `0x1B92` | R_0x1b92 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 644 | `0x1B94` | R_0x1b94 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 645 | `0x1B97` | R_0x1b97 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 646 | `0x1B98` | R_0x1b98 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 647 | `0x1B9C` | R_0x1b9c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 648 | `0x1BA0` | R_0x1ba0 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 649 | `0x1BA4` | R_0x1ba4 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 650 | `0x1BA8` | R_0x1ba8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 651 | `0x1BAC` | R_0x1bac | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 652 | `0x1BB0` | R_0x1bb0 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 653 | `0x1BB4` | R_0x1bb4 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 654 | `0x1BB8` | R_0x1bb8 | BB | A | Calibration | 32 | W | 1/0/0/0/0 | 0/0 | _iqk_rf_setting_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:548 |
| 655 | `0x1BBC` | R_0x1bbc | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 656 | `0x1BC0` | R_0x1bc0 | BB | A | Init table | 32 |  | 0/0/0/0/2 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 657 | `0x1BC8` | R_0x1bc8 | BB | A | Interrupt | 32 | W | 1/0/0/0/0 | 0/0 | _iqk_tximr_selfcheck_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1367 |
| 658 | `0x1BCA` | R_0x1bca | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 659 | `0x1BCB` | R_0x1bcb | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 660 | `0x1BCC` | R_0x1bcc | BB | A | Calibration | 8/32 | RMW-mask W | 9/0/2/4/2 | 0/0 | _iqk_rxk1_setting_8822b, _iqk_rxk2_setting_8822b, _iqk_rxk_gsearch_fail_8822b, _iqk_tximr_selfcheck_8822b … | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1387, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1389, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:655 … |
| 661 | `0x1BCE` | R_0x1bce | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 662 | `0x1BD0` | R_0x1bd0 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 663 | `0x1BD4` | R_0x1bd4 | BB | A | Interrupt | 32 | RMW-mask W | 3/0/2/7/0 | 0/0 | _iqk_tximr_selfcheck_8822b, phy_get_iqk_cfir_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1395, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1397, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1936 |
| 664 | `0x1BD6` | R_0x1bd6 | BB | A | Interrupt | 8 | W | 3/0/0/0/0 | 0/0 | _iqk_rximr_selfcheck_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1475, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1484, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1486 |
| 665 | `0x1BD8` | R_0x1bd8 | BB | A | Calibration | 32 | RMW-mask W | 2/0/6/3/0 | 0/0 | phy_get_iqk_cfir_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1938, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1944 |
| 666 | `0x1BDC` | R_0x1bdc | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 667 | `0x1BE4` | R_0x1be4 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 668 | `0x1BE8` | R_0x1be8 | BB | G | Calibration | 32 | R W | 0/0/1/1/0 | 0/0 | halrf_iqk_dbg | hal/phydm/halrf/halrf.c:481 |
| 669 | `0x1BEB` | R_0x1beb | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 670 | `0x1BEC` | R_0x1bec | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 671 | `0x1BEF` | R_0x1bef | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 672 | `0x1BF0` | R_0x1bf0 | BB | A | Calibration | 32 | R R-mask RMW-mask | 2/0/3/4/0 | 0/0 | _iqk_fail_count_8822b, halrf_iqk_check_if_reload | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:171, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:777 |
| 673 | `0x1BF4` | R_0x1bf4 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 674 | `0x1BF8` | R_0x1bf8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 675 | `0x1BFC` | R_0x1bfc | BB | A | Interrupt | 32 | R R-mask | 6/0/0/4/0 | 0/0 | _iqk_rximr_selfcheck_8822b, _iqk_tximr_selfcheck_8822b, phy_get_iqk_cfir_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1396, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1398, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1478 … |
| 676 | `0x1C00` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 677 | `0x1C04` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 678 | `0x1C08` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 679 | `0x1C0C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 680 | `0x1C10` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 681 | `0x1C14` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 682 | `0x1C18` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 683 | `0x1C1C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 684 | `0x1C20` | R_0x1c20 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 685 | `0x1C24` | R_0x1c24 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 686 | `0x1C28` | R_0x1c28 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 687 | `0x1C2C` | R_0x1c2c | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 688 | `0x1C30` | R_0x1c30 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 689 | `0x1C34` | R_0x1c34 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 690 | `0x1C38` | R_0x1c38 | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | phydm_fahm_set_th_by_igi | hal/phydm/phydm_ccx.c:311 |
| 691 | `0x1C3C` | R_0x1c3c | BB | A | Init table | 32 | R-mask RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 692 | `0x1C40` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 693 | `0x1C44` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 694 | `0x1C48` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 695 | `0x1C4C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 696 | `0x1C50` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 697 | `0x1C54` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 698 | `0x1C58` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 699 | `0x1C5C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 700 | `0x1C60` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 701 | `0x1C64` | R_0x1c64 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 702 | `0x1C68` | R_0x1c68 | BB | A | Init table | 32 | RMW-mask | 0/0/0/2/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 703 | `0x1C6C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 704 | `0x1C70` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 705 | `0x1C74` | R_0x1c74 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 706 | `0x1C78` | R_0x1c78 | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | phydm_fahm_set_th_by_igi | hal/phydm/phydm_ccx.c:313 |
| 707 | `0x1C7C` | R_0x1c7c | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | phydm_fahm_set_th_by_igi | hal/phydm/phydm_ccx.c:315 |
| 708 | `0x1C80` | R_0x1c80 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 709 | `0x1C84` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 710 | `0x1C88` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 711 | `0x1C8C` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 712 | `0x1C90` | R_0x1c90 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 713 | `0x1C94` | R_0x1c94 | BB | A | Beamforming/MU | 32 | W | 1/0/0/0/1 | 0/0 | rtl8822b_phy_bf_init | hal/rtl8822b/rtl8822b_phy.c:2153 |
| 714 | `0x1C98` | R_0x1c98 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 715 | `0x1C9C` | R_0x1c9c | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 716 | `0x1CA0` | R_0x1ca0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 717 | `0x1CA4` | R_0x1ca4 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 718 | `0x1CA8` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 719 | `0x1CAC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 720 | `0x1CB0` | R_0x1cb0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 721 | `0x1CB4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 722 | `0x1CB8` | R_0x1cb8 | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | phydm_fahm_set_th_by_igi | hal/phydm/phydm_ccx.c:317 |
| 723 | `0x1CBC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 724 | `0x1CC0` | R_0x1cc0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 725 | `0x1CC4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 726 | `0x1CC8` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 727 | `0x1CCC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 728 | `0x1CD0` | R_0x1cd0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 729 | `0x1CD4` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 730 | `0x1CD8` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 731 | `0x1CDC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 732 | `0x1CE0` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 733 | `0x1CE4` | R_0x1ce4 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 734 | `0x1CE8` | R_0x1ce8 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 735 | `0x1CEC` | R_0x1cec | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 736 | `0x1CF0` | R_0x1cf0 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 737 | `0x1CF4` | R_0x1cf4 | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 738 | `0x1CF8` | R_0x1cf8 | BB | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 1/0/0/0/1 | 0/0 | phydm_fahm_trigger | hal/phydm/phydm_ccx.c:162 |
| 739 | `0x1CFC` | *(không có macro)* | BB | A | Init table | 32 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_phy_reg |
| 740 | `0x1D04` | R_0x1d04 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 741 | `0x1D08` | R_0x1d08 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 742 | `0x1D0C` | R_0x1d0c | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 743 | `0x1D10` | R_0x1d10 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 744 | `0x1D2C` | R_0x1d2c | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 745 | `0x1D30` | R_0x1d30 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 746 | `0x1D3C` | R_0x1d3c | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 747 | `0x1D44` | R_0x1d44 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 748 | `0x1D48` | R_0x1d48 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 749 | `0x1D58` | R_0x1d58 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 750 | `0x1D60` | R_0x1d60 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 751 | `0x1D6C` | R_0x1d6c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 752 | `0x1D70` | R_0x1d70 (+1: ODM_REG_IGI_11AC3) | BB | X | BB DM (DIG/CCA/FA/CCX) | 32 | R-mask | 0/0/0/1/0 | 0/0 | phydm_psd | hal/phydm/phydm_psd.c:240 |
| 753 | `0x1D90` | R_0x1d90 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 754 | `0x1D94` | R_0x1d94 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 755 | `0x1D9C` | R_0x1d9c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 756 | `0x1DA4` | R_0x1da4 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 757 | `0x1DA8` | R_0x1da8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 758 | `0x1DE8` | R_0x1de8 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 759 | `0x1E14` | R_0x1e14 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 760 | `0x1E18` | R_0x1e18 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 761 | `0x1E1C` | R_0x1e1c | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 762 | `0x1E24` | R_0x1e24 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 763 | `0x1E28` | R_0x1e28 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 764 | `0x1E2C` | R_0x1e2c | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 765 | `0x1E30` | R_0x1e30 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 766 | `0x1E40` | R_0x1e40 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 767 | `0x1E44` | R_0x1e44 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 768 | `0x1E48` | R_0x1e48 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 769 | `0x1E5C` | R_0x1e5c | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 770 | `0x1E60` | R_0x1e60 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 771 | `0x1E64` | R_0x1e64 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 772 | `0x1E68` | R_0x1e68 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 773 | `0x1E6C` | R_0x1e6c | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 774 | `0x1E70` | R_0x1e70 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 775 | `0x1E7C` | R_0x1e7c | BB | X |  | 32 | RMW-mask | 0/0/0/2/0 | 0/0 | halrf_set_tssi_enable | hal/phydm/halrf/halrf.c:3003, hal/phydm/halrf/halrf.c:3006 |
| 776 | `0x1E84` | R_0x1e84 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 777 | `0x1E88` | R_0x1e88 | BB | X | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 0/0/0/4/0 | 0/0 | phydm_get_psd_data, phydm_psd_para_setting | hal/phydm/phydm_psd.c:353, hal/phydm/phydm_psd.c:355, hal/phydm/phydm_psd.c:357 … |
| 778 | `0x1E8C` | R_0x1e8c | BB | X | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 0/0/0/6/0 | 0/0 | phydm_get_psd_data, phydm_psd_para_setting | hal/phydm/phydm_psd.c:349, hal/phydm/phydm_psd.c:350, hal/phydm/phydm_psd.c:358 … |
| 779 | `0x1EA4` | R_0x1ea4 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 780 | `0x1EB4` | R_0x1eb4 | BB | X | System/Clock/Reset | 32 | RMW-mask | 0/0/0/4/0 | 0/0 | phydm_reset_bb_hw_cnt, rtw_reset_phy_trx_ok_counters | hal/hal_com.c:16967, hal/hal_com.c:16968, hal/phydm/phydm_api.c:72 … |
| 781 | `0x1EE8` | R_0x1ee8 | BB | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 782 | `0x1EEC` | R_0x1eec | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 783 | `0x1EF0` | R_0x1ef0 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 784 | `0x1EF4` | R_0x1ef4 | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 785 | `0x1EFC` | R_0x1efc | BB | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 786 | `0x0C20` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/10 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 787 | `0x0C24` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 788 | `0x0C28` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 789 | `0x0C2C` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 790 | `0x0C30` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 791 | `0x0C34` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 792 | `0x0C38` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 793 | `0x0C3C` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 794 | `0x0C40` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 795 | `0x0C44` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 796 | `0x0C48` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 797 | `0x0C4C` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 798 | `0x0E20` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/10 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 799 | `0x0E24` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 800 | `0x0E28` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 801 | `0x0E2C` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 802 | `0x0E30` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 803 | `0x0E34` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 804 | `0x0E38` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 805 | `0x0E3C` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 806 | `0x0E40` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 807 | `0x0E44` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 808 | `0x0E48` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 809 | `0x0E4C` | *(không có macro)* | BB-pg | X | Init table |  |  | 0/0/0/0/20 | 0/0 |  | bảng: array_mp_8822b_phy_reg_pg, array_mp_8822b_phy_reg_pg_type12 |
| 810 | `0x28A4` | R_0x28a4 | RF-direct | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 811 | `0x2C04` | R_0x2c04 | RF-direct | X |  | 32 | R-mask | 0/0/0/2/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16920, hal/hal_com.c:16924 |
| 812 | `0x2C08` | R_0x2c08 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 813 | `0x2C0C` | R_0x2c0c | RF-direct | X |  | 32 | R-mask | 0/0/0/2/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16923, hal/hal_com.c:16927 |
| 814 | `0x2C10` | R_0x2c10 | RF-direct | X |  | 32 | R-mask | 0/0/0/2/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16922, hal/hal_com.c:16926 |
| 815 | `0x2C14` | R_0x2c14 | RF-direct | X |  | 32 | R-mask | 0/0/0/2/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16921, hal/hal_com.c:16925 |
| 816 | `0x2C20` | R_0x2c20 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 817 | `0x2C2C` | R_0x2c2c | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 818 | `0x2C30` | R_0x2c30 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 819 | `0x2C34` | R_0x2c34 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 820 | `0x2D00` | R_0x2d00 | RF-direct | X |  | 32 | R-mask | 0/0/0/1/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16929 |
| 821 | `0x2D04` | R_0x2d04 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 822 | `0x2D08` | R_0x2d08 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 823 | `0x2D0C` | R_0x2d0c | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 824 | `0x2D10` | R_0x2d10 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 825 | `0x2D20` | R_0x2d20 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 826 | `0x2D38` | R_0x2d38 | RF-direct | X |  | 32 | R-mask | 0/0/0/1/0 | 0/0 | phydm_get_dbg_port_info | hal/phydm/phydm_dig.c:1569 |
| 827 | `0x2D40` | R_0x2d40 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 828 | `0x2D44` | R_0x2d44 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 829 | `0x2D48` | R_0x2d48 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 830 | `0x2D4C` | R_0x2d4c | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 831 | `0x2D88` | R_0x2d88 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 832 | `0x2D90` | R_0x2d90 | RF-direct | R |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 833 | `0x2D9C` | R_0x2d9c | RF-direct | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 834 | `0x2DB4` | R_0x2db4 | RF-direct | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 835 | `0x2DB8` | R_0x2db8 | RF-direct | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 836 | `0x2DBC` | R_0x2dbc | RF-direct | X |  | 32 | R-mask | 0/0/0/4/0 | 0/0 | halrf_mode, phydm_get_bb_dbg_port_val | hal/phydm/halrf/halrf.c:3391, hal/phydm/halrf/halrf.c:3435, hal/phydm/halrf/halrf.c:3438 … |
| 837 | `0x2DE0` | R_0x2de0 | RF-direct | X |  | 32 | R-mask | 0/0/0/1/0 | 0/0 | rtw_dump_phy_rx_counters | hal/hal_com.c:16929 |
| 838 | `0x2DE4` | R_0x2de4 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 839 | `0x2DE8` | R_0x2de8 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 840 | `0x2E00` | R_0x2e00 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 841 | `0x2E20` | R_0x2e20 | RF-direct | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 842 | `0x3A00` | R_0x3a00 | other | R |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 843 | `0x3A04` | R_0x3a04 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 844 | `0x3A08` | R_0x3a08 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 845 | `0x3A0C` | R_0x3a0c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 846 | `0x3A10` | R_0x3a10 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 847 | `0x3A14` | R_0x3a14 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 848 | `0x3A18` | R_0x3a18 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 849 | `0x3A1C` | R_0x3a1c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 850 | `0x3A20` | R_0x3a20 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 851 | `0x3A24` | R_0x3a24 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 852 | `0x3A28` | R_0x3a28 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 853 | `0x3A2C` | R_0x3a2c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 854 | `0x3A30` | R_0x3a30 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 855 | `0x3A34` | R_0x3a34 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 856 | `0x3A38` | R_0x3a38 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 857 | `0x3A3C` | R_0x3a3c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 858 | `0x3A40` | R_0x3a40 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 859 | `0x3A44` | R_0x3a44 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 860 | `0x3A48` | R_0x3a48 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 861 | `0x3A4C` | R_0x3a4c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 862 | `0x3A50` | R_0x3a50 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 863 | `0x3A54` | R_0x3a54 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 864 | `0x3A58` | R_0x3a58 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 865 | `0x3A5C` | R_0x3a5c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 866 | `0x3A60` | R_0x3a60 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 867 | `0x3A64` | R_0x3a64 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 868 | `0x3A68` | R_0x3a68 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 869 | `0x3A6C` | R_0x3a6c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 870 | `0x3A70` | R_0x3a70 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 871 | `0x3A74` | R_0x3a74 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 872 | `0x3A78` | R_0x3a78 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 873 | `0x3A7C` | R_0x3a7c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 874 | `0x3A80` | R_0x3a80 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 875 | `0x3A84` | R_0x3a84 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 876 | `0x3A88` | R_0x3a88 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 877 | `0x3A8C` | R_0x3a8c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 878 | `0x3A90` | R_0x3a90 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 879 | `0x3A94` | R_0x3a94 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 880 | `0x3A98` | R_0x3a98 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 881 | `0x3A9C` | R_0x3a9c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 882 | `0x3AA0` | R_0x3aa0 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 883 | `0x3AA4` | R_0x3aa4 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 884 | `0x3C00` | R_0x3c00 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 885 | `0x4000` | R_0x4000 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 886 | `0x4008` | R_0x4008 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 887 | `0x4018` | R_0x4018 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 888 | `0x401C` | R_0x401c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 889 | `0x4028` | R_0x4028 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 890 | `0x4040` | R_0x4040 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 891 | `0x4044` | R_0x4044 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 892 | `0x4100` | R_0x4100 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 893 | `0x4104` | R_0x4104 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 894 | `0x4108` | R_0x4108 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 895 | `0x410C` | R_0x410c | other | X |  | 32 | RMW-mask | 0/0/0/4/0 | 0/0 | phydm_stop_3_wire | hal/phydm/phydm_api.c:560, hal/phydm/phydm_api.c:561, hal/phydm/phydm_api.c:589 … |
| 896 | `0x4110` | R_0x4110 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 897 | `0x4114` | R_0x4114 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 898 | `0x4118` | R_0x4118 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 899 | `0x411C` | R_0x411c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 900 | `0x4130` | R_0x4130 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 901 | `0x4134` | R_0x4134 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 902 | `0x4138` | R_0x4138 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 903 | `0x413C` | R_0x413c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 904 | `0x4140` | R_0x4140 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 905 | `0x4144` | R_0x4144 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 906 | `0x4148` | R_0x4148 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 907 | `0x4160` | R_0x4160 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 908 | `0x4164` | R_0x4164 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 909 | `0x4168` | R_0x4168 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 910 | `0x416C` | R_0x416c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 911 | `0x4180` | R_0x4180 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 912 | `0x419C` | R_0x419c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 913 | `0x41A0` | R_0x41a0 | other | X | TX power | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_reset_txagc | hal/phydm/phydm_api.c:2008 |
| 914 | `0x41A4` | R_0x41a4 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 915 | `0x41A8` | R_0x41a8 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 916 | `0x41AC` | R_0x41ac | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 917 | `0x41E0` | R_0x41e0 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 918 | `0x41E8` | R_0x41e8 | other | X | TX power | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_reset_txagc | hal/phydm/phydm_api.c:2009 |
| 919 | `0x41EC` | R_0x41ec | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 920 | `0x41F0` | R_0x41f0 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 921 | `0x41F8` | R_0x41f8 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 922 | `0x41FC` | R_0x41fc | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 923 | `0x45A4` | R_0x45a4 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 924 | `0x4C00` | R_0x4c00 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 925 | `0x5000` | R_0x5000 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 926 | `0x5008` | R_0x5008 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 927 | `0x5018` | R_0x5018 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 928 | `0x501C` | R_0x501c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 929 | `0x5028` | R_0x5028 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 930 | `0x5040` | R_0x5040 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 931 | `0x5044` | R_0x5044 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 932 | `0x5100` | R_0x5100 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 933 | `0x5108` | R_0x5108 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 934 | `0x5118` | R_0x5118 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 935 | `0x511C` | R_0x511c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 936 | `0x5128` | R_0x5128 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 937 | `0x5140` | R_0x5140 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 938 | `0x5144` | R_0x5144 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 939 | `0x5200` | R_0x5200 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 940 | `0x520C` | R_0x520c | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 941 | `0x5230` | R_0x5230 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 942 | `0x5234` | R_0x5234 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 943 | `0x5238` | R_0x5238 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 944 | `0x523C` | R_0x523c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 945 | `0x5240` | R_0x5240 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 946 | `0x5244` | R_0x5244 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 947 | `0x5248` | R_0x5248 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 948 | `0x526C` | R_0x526c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 949 | `0x52A0` | R_0x52a0 | other | X | TX power | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_reset_txagc | hal/phydm/phydm_api.c:2008 |
| 950 | `0x52A4` | R_0x52a4 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 951 | `0x52AC` | R_0x52ac | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 952 | `0x52E8` | R_0x52e8 | other | X | TX power | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_reset_txagc | hal/phydm/phydm_api.c:2009 |
| 953 | `0x5300` | R_0x5300 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 954 | `0x530C` | R_0x530c | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 955 | `0x5330` | R_0x5330 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 956 | `0x5334` | R_0x5334 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 957 | `0x5338` | R_0x5338 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 958 | `0x533C` | R_0x533c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 959 | `0x5340` | R_0x5340 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 960 | `0x5344` | R_0x5344 | other | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 961 | `0x5348` | R_0x5348 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 962 | `0x536C` | R_0x536c | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 963 | `0x53A0` | R_0x53a0 | other | X | TX power | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_reset_txagc | hal/phydm/phydm_api.c:2008 |
| 964 | `0x53A4` | R_0x53a4 | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 965 | `0x53AC` | R_0x53ac | other | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 966 | `0x53E8` | R_0x53e8 | other | X | TX power | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_reset_txagc | hal/phydm/phydm_api.c:2009 |
| 967 | `0x8000` | R_0x8000 | other | R |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 968 | `0x10250020` | REG_SDIO_FREE_TXPG | other | X |  | 32 | R | 0/0/0/1/0 | 14/0 | download_firmware_88xx | hal/halmac/halmac_88xx/halmac_fw_88xx.c:232 |
