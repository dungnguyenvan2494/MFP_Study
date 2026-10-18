# Danh mục thanh ghi RF (không gian RF riêng, mỗi path 8-bit địa chỉ × 20-bit dữ liệu)

Địa chỉ RF là chỉ số 0x00–0xFF **trong từng path** (A/B), không phải địa chỉ bus. Đọc: BB `0x2800/0x2C00 + addr*4`; ghi: BB `0xC90/0xE90` = `(addr<<20)|data20` (`phydm_hal_api8822b.c:995-1074`).

Tổng: **112 địa chỉ** — A=64, T=9, X=6, Z=3, D=30

Cột: **St** = trạng thái (A/T/M/G/X/R/Z/D, xem `04_catalog_README.md`); **W** = độ rộng truy cập quan sát được trong code/bảng (không phải độ rộng phần cứng); **Truy cập** = live/MP/dbg/dead/bảng; **Bit** = số macro bit/field định nghĩa trong section của thanh ghi / số được dùng; **Nhóm** = [INFERENCE] suy từ tên hàm truy cập.

| STT | Địa chỉ | Tên (alias) | Không gian | St | Nhóm | W | Kiểu | Truy cập L/M/G/X/T | Bit d/u | Hàm (≤4) | Vị trí (≤3) |
|---:|---|---|---|:-:|---|---|---|---|---|---|---|
| 1 | `0x0000` | RF_0x0 (+3: RF_0x00, ODM_GAIN_SETTING, …) | RF (enum rf_path)pathABRF_PATH_ARF_PATH_BppMptCtx->mpt_rf_pathpath | A | Calibration | 20 | R-mask RMW-mask | 11/9/0/14/2 | 0/0 | _iqk_bb_reset_8822b, _iqk_one_shot_8822b, _iqk_restore_rf_8822b, _iqk_rxk_gsearch_fail_8822b … | hal/phydm/halrf/rtl8822b/halrf_8822b.c:483, hal/phydm/halrf/rtl8822b/halrf_8822b.c:484, hal/phydm/halrf/rtl8822b/halrf_8822b.c:505 … |
| 2 | `0x0001` | RF_0x1 | RF ABRF_PATH_ARF_PATH_B | A | Init table | 20 | R-mask RMW-mask | 2/0/0/4/48 | 0/0 | _iqk_restore_rf_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:354, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:356 |
| 3 | `0x0002` | RF_0x2 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 4 | `0x0003` | RF_0x3 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 5 | `0x0004` | RF_0x4 (+1: ODM_REG_RF_PIN_11N) | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 6 | `0x0008` | RF_0x8 (+1: RF_0x08) | RF (enum rf_path)pathABRF_PATH_A | A | Init table | 20 | R-mask RMW-mask | 5/0/0/0/4 | 0/0 | _iqk_check_cal_8822b, halrf_iqk_set_rf0x8, phydm_get_read_counter_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:764, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:766, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:78 … |
| 7 | `0x000B` | ODM_REG_RF_0B_11N | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 8 | `0x000C` | RF_0x0c | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 9 | `0x000D` | RF_0xd (+1: RF_0x0d) | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 10 | `0x0018` | RF_0x18 (+1: ODM_CHANNEL) | RF 0ABRF_PATH_ARF_PATH_Bpathrf_path | A | Init table | 20 | R-mask RMW-mask | 44/0/0/7/10 | 0/0 | _phy_lc_calibrate_8822b, _set_tx_a_cali_value, config_phydm_switch_band_8822b, config_phydm_switch_bandwidth_8822b … | core/rtw_wlan_util.c:772, hal/hal_mcc.c:2755, hal/phydm/halrf/rtl8822b/halrf_8822b.c:486 … |
| 11 | `0x0019` | RF_0x19 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 12 | `0x001A` | RF_0x1a | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 13 | `0x001B` | *(không có macro)* | RF AB | A | Init table | 20 |  | 0/0/0/0/2 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 14 | `0x0024` | ODM_RF_T_METER (+1: RF_T_METER_OLD) | RF RF_PATH_A | X |  | 20 | RMW-mask | 0/0/0/2/0 | 0/0 | odm_txpowertracking_check_ce, odm_txpowertracking_direct_ce | hal/phydm/halrf/halrf_powertracking_ce.c:877, hal/phydm/halrf/halrf_powertracking_ce.c:926 |
| 15 | `0x0025` | ODM_REG_RF_25_11N | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 16 | `0x0026` | ODM_REG_RF_26_11N | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 17 | `0x0027` | ODM_REG_RF_27_11N | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 18 | `0x002B` | ODM_REG_RF_2B_11N | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 19 | `0x002C` | ODM_REG_RF_2C_11N | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 20 | `0x0030` | RF_0x30 (+1: rf_mode_table_addr) | RF ABrf_path | A | Init table | 20 | RMW-mask | 1/0/0/0/1620 | 0/0 | _set_tx_a_cali_value | hal/phydm/rtl8822b/phydm_rtl8822b.c:239 |
| 21 | `0x0031` | RF_0x31 (+1: rf_mode_table_data0) | RF | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 22 | `0x0032` | RF_0x32 (+1: rf_mode_table_data1) | RF | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 23 | `0x0033` | RF_0x33 | RF (enum rf_path)i(enum rf_path)pathABRF_PATH_Ae_rf_pathpath | A | Init table | 20 | R-mask RMW-mask | 12/0/0/95/2379 | 0/0 | _iqk_rf_setting_8822b, config_phydm_trx_mode_8822b, phydm_8822btxbf_rfmode, phydm_iq_gen_en … | hal/phydm/halrf/halrf_kfree.c:423, hal/phydm/halrf/halrf_kfree.c:425, hal/phydm/halrf/halrf_kfree.c:427 … |
| 24 | `0x0035` | RF_0x35 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 25 | `0x003E` | RF_0x3e | RF (enum rf_path)i(enum rf_path)pathABRF_PATH_Apath | A | Init table | 20 | RMW-mask | 6/0/0/2/249 | 0/0 | _iqk_rf_setting_8822b, config_phydm_trx_mode_8822b, phydm_8822btxbf_rfmode, phydm_iq_gen_en | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:569, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:580, hal/phydm/phydm_direct_bf.c:80 … |
| 26 | `0x003F` | RF_0x3f | RF (enum rf_path)i(enum rf_path)pathABRF_PATH_Ae_rf_pathpath | A | Init table | 20 | R-mask RMW-mask | 11/0/0/45/2575 | 0/0 | _iqk_rf_setting_8822b, config_phydm_trx_mode_8822b, phydm_8822btxbf_rfmode, phydm_iq_gen_en … | hal/phydm/halrf/halrf_kfree.c:424, hal/phydm/halrf/halrf_kfree.c:426, hal/phydm/halrf/halrf_kfree.c:428 … |
| 27 | `0x0042` | R_0x42 (+6: RF_0x42, RF_T_METER_NEW, …) | RF RF_PATH_ARF_PATH_BRF_PATH_CRF_PATH_Drf_path | A |  | 20 | R-mask RMW-mask | 1/2/0/20/0 | 0/0 | odm_txpowertracking_check_ce | hal/phydm/halrf/halrf_powertracking_ce.c:861 |
| 28 | `0x0043` | RF_0x43 | RF RF_PATH_ARF_PATH_B | X | Calibration | 20 | RMW-mask | 0/0/0/4/0 | 0/0 | phydm_get_set_thermal_trim_offset_8812f, phydm_get_set_thermal_trim_offset_8822c | hal/phydm/halrf/halrf_kfree.c:1092, hal/phydm/halrf/halrf_kfree.c:1101, hal/phydm/halrf/halrf_kfree.c:1452 … |
| 29 | `0x0051` | RF_0x51 | RF ABe_rf_path | A | Init table | 20 | R-mask | 1/0/0/0/22 | 0/0 | phydm_set_pa_bias_to_rf_8822b | hal/phydm/halrf/halrf_kfree.c:387 |
| 30 | `0x0052` | RF_0x52 | RF ABRF_PATH_ARF_PATH_Be_rf_path | A | Init table | 20 | R-mask RMW-mask | 1/2/0/12/22 | 0/0 | phydm_set_pa_bias_to_rf_8822b | hal/phydm/halrf/halrf_kfree.c:388 |
| 31 | `0x0053` | RF_0x53 | RF ABRF_PATH_A | T | Init table | 20 | RMW-mask | 0/0/0/4/22 | 0/0 | hal_mpt_SetSingleToneTx | hal/hal_mp.c:1827, hal/hal_mp.c:1839, hal/hal_mp.c:1953 … |
| 32 | `0x0054` | RF_0x54 | RF AB | T | Init table | 20 |  | 0/0/0/0/22 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 33 | `0x0055` | RF_0x55 (+1: REG_RF_TX_GAIN_OFFSET) | RF e_rf_pathtarget_path | A | Calibration | 20 | R-mask RMW-mask | 18/0/0/40/0 | 0/0 | phydm_clear_kfree_to_rf_8822b, phydm_set_kfree_to_rf_8821c, phydm_set_kfree_to_rf_8822b, rtw_rf_set_tx_gain_offset | core/rtw_rf.c:1317, core/rtw_rf.c:1382, hal/phydm/halrf/halrf_kfree.c:234 … |
| 34 | `0x0056` | RF_0x56 | RF (enum rf_path)pathABRF_PATH_A | A | Init table | 20 | R-mask RMW-mask | 7/0/0/4/20 | 0/0 | _iqk_one_shot_8822b, _iqk_rxk1_setting_8822b, _iqk_rxk2_setting_8822b, _iqk_txk_setting_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:666, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:673, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:695 … |
| 35 | `0x0057` | RF_0x57 | RF AB | T | Init table | 20 |  | 0/0/0/0/22 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 36 | `0x0058` | RF_0x58 | RF (enum rf_path)pathABRF_PATH_AppMptCtx->mpt_rf_pathpathrfPath | A | Init table | 20 | R-mask RMW-mask | 1/6/0/9/22 | 0/0 | _iqk_backup_iqk_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:411 |
| 37 | `0x0059` | *(không có macro)* | RF AB | A | Init table | 20 |  | 0/0/0/0/2 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 38 | `0x005C` | RF_0x5c | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 39 | `0x0060` | *(không có macro)* | RF RF_PATH_ARF_PATH_Bi | X |  | 20 | RMW-mask | 0/0/0/10/0 | 0/0 | phydm_get_pa_bias_offset_8198f, phydm_get_set_pa_bias_offset_8195b, phydm_get_set_pa_bias_offset_8197g, phydm_get_set_pa_bias_offset_8812f … | hal/phydm/halrf/halrf_kfree.c:1400, hal/phydm/halrf/halrf_kfree.c:1409, hal/phydm/halrf/halrf_kfree.c:1418 … |
| 40 | `0x0061` | RF_0x61 | RF ABrf_path | A | Init table | 20 | R-mask | 1/0/0/0/54 | 0/0 | _set_tx_a_cali_value | hal/phydm/rtl8822b/phydm_rtl8822b.c:175 |
| 41 | `0x0062` | *(không có macro)* | RF AB | T | Init table | 20 |  | 0/0/0/0/54 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 42 | `0x0063` | RF_0x63 | RF ABRF_PATH_A | T | Init table | 20 | RMW-mask | 0/0/0/2/54 | 0/0 | hal_mpt_SetSingleToneTx | hal/hal_mp.c:1843, hal/hal_mp.c:1968 |
| 43 | `0x0064` | RF_0x64 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 44 | `0x0065` | RF_0x65 | RF (enum rf_path)pathRF_PATH_ARF_PATH_Be_rf_pathtarget_path | A | Calibration | 20 | R-mask RMW-mask | 10/0/0/7/0 | 0/0 | _iqk_restore_rf_8822b, _iqk_rf_setting_8822b, phydm_clear_kfree_to_rf_8822b, phydm_set_kfree_to_rf_8821c … | hal/phydm/halrf/halrf_kfree.c:235, hal/phydm/halrf/halrf_kfree.c:244, hal/phydm/halrf/halrf_kfree.c:245 … |
| 45 | `0x0066` | RF_0x66 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 46 | `0x0067` | RF_0x67 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 47 | `0x006E` | RF_0x6e | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 48 | `0x006F` | RF_0x6f | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 49 | `0x0075` | RF_0x75 | RF p | X |  | 20 | RMW-mask | 0/0/0/1/0 | 0/0 | hal_mpt_SetSingleToneTx | hal/hal_mp.c:1986 |
| 50 | `0x0076` | RF_0x76 | RF RF_PATH_A | X |  | 20 | RMW-mask | 0/0/0/2/0 | 0/0 | hal_mpt_SetSingleToneTx | hal/hal_mp.c:1822, hal/hal_mp.c:1948 |
| 51 | `0x0078` | RF_0x78 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 52 | `0x007C` | RF_0x7c | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 53 | `0x007F` | RF_0x7f | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 54 | `0x0080` | RF_0x80 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 55 | `0x0081` | RF_0x81 | RF AB | T | Init table | 20 |  | 0/0/0/0/22 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 56 | `0x0082` | RF_0x82 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 57 | `0x0083` | RF_0x83 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 58 | `0x0084` | *(không có macro)* | RF AB | A | Init table | 20 |  | 0/0/0/0/2 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 59 | `0x0085` | RF_0x85 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 60 | `0x0086` | RF_0x86 | RF ABi | A | Init table | 20 | R-mask RMW-mask | 0/0/0/4/2 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 61 | `0x0087` | RF_0x87 | RF AB | T | Init table | 20 |  | 0/0/0/0/44 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 62 | `0x0088` | *(không có macro)* | RF ABi | A | Init table | 20 | RMW-mask | 0/0/0/1/2 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 63 | `0x0089` | *(không có macro)* | RF AB | A | Init table | 20 |  | 0/0/0/0/2 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 64 | `0x008A` | RF_0x8a | RF AB | T | Init table | 20 |  | 0/0/0/0/23 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 65 | `0x008B` | *(không có macro)* | RF AB | A | Init table | 20 |  | 0/0/0/0/2 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 66 | `0x008C` | RF_0x8c | RF AB | A | Init table | 20 |  | 0/0/0/0/2 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 67 | `0x008D` | RF_0x8d | RF AB | A | Init table | 20 |  | 0/0/0/0/2 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 68 | `0x008E` | *(không có macro)* | RF AB | A | Init table | 20 |  | 0/0/0/0/2 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 69 | `0x008F` | RF_0x8f | RF (enum rf_path)pathRF_PATH_ARF_PATH_B | A | Calibration | 20 | RMW-mask | 8/0/0/2/0 | 0/0 | _iqk_restore_rf_8822b, _iqk_rxk1_setting_8822b, _iqk_rxk2_setting_8822b, _iqk_txk_setting_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:354, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:356, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:668 … |
| 70 | `0x0093` | RF_0x93 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 71 | `0x00A9` | RF_0xa9 | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 72 | `0x00AE` | RF_0xae | RF | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 73 | `0x00B0` | RF_0xb0 | RF A | A | Init table | 20 |  | 0/0/0/0/30 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 74 | `0x00B1` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 75 | `0x00B2` | *(không có macro)* | RF ARF_PATH_A | A | Init table | 20 | RMW-mask | 1/0/0/0/2 | 0/0 | aac_check_8822b | hal/phydm/halrf/rtl8822b/halrf_8822b.c:467 |
| 76 | `0x00B3` | RF_0xb3 | RF ARF_PATH_ARF_PATH_B | A | Init table | 20 | RMW-mask | 4/0/0/0/15 | 0/0 | config_phydm_switch_band_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1666, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1668, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1731 … |
| 77 | `0x00B4` | RF_0xb4 | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 78 | `0x00B5` | *(không có macro)* | RF A | T | Init table | 20 |  | 0/0/0/0/2 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 79 | `0x00B6` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 80 | `0x00B7` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 81 | `0x00B8` | RF_0xb8 | RF (enum rf_path)0x0ARF_PATH_A | A | Init table | 20 | RMW-mask | 2/0/0/2/2 | 0/0 | config_phydm_switch_channel_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:2013, hal/phydm/rtl8822b/phydm_hal_api8822b.c:2014 |
| 82 | `0x00B9` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 83 | `0x00BA` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 84 | `0x00BB` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 85 | `0x00BC` | RF_0xbc | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 86 | `0x00BD` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 87 | `0x00BE` | RF_0xbe | RF ARF_PATH_A | A | Channel/BW | 20 | RMW-mask | 1/0/0/0/1 | 0/0 | config_phydm_switch_channel_8822b | hal/phydm/rtl8822b/phydm_hal_api8822b.c:1977 |
| 88 | `0x00BF` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 89 | `0x00C0` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 90 | `0x00C1` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 91 | `0x00C2` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 92 | `0x00C3` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 93 | `0x00C4` | RF_0xc4 | RF ARF_PATH_A | A | Init table | 20 | RMW-mask | 2/0/0/0/2 | 0/0 | _phy_lc_calibrate_8822b | hal/phydm/halrf/rtl8822b/halrf_8822b.c:488, hal/phydm/halrf/rtl8822b/halrf_8822b.c:501 |
| 94 | `0x00C5` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 95 | `0x00C6` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 96 | `0x00C7` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 97 | `0x00C8` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 98 | `0x00C9` | RF_0xc9 | RF ARF_PATH_A | A | Init table | 20 | R-mask | 1/0/0/0/2 | 0/0 | aac_check_8822b | hal/phydm/halrf/rtl8822b/halrf_8822b.c:464 |
| 99 | `0x00CA` | RF_0xca | RF ARF_PATH_A | A | Init table | 20 | RMW-mask | 1/0/0/0/2 | 0/0 | aac_check_8822b | hal/phydm/halrf/rtl8822b/halrf_8822b.c:466 |
| 100 | `0x00CB` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 101 | `0x00CC` | RF_0xcc | RF A | A | Init table | 20 |  | 0/0/0/0/2 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 102 | `0x00CD` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 103 | `0x00CE` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/9 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 104 | `0x00CF` | *(không có macro)* | RF A | A | Init table | 20 |  | 0/0/0/0/10 | 0/0 |  | bảng: array_mp_8822b_radioa |
| 105 | `0x00DD` | RF_0xdd | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 106 | `0x00DE` | RF_0xde | RF ABe_rf_path | A | Calibration | 20 | RMW-mask | 8/0/0/4/2 | 0/0 | phydm_clear_kfree_to_rf_8822b, phydm_set_kfree_to_rf_8821c, phydm_set_kfree_to_rf_8822b | hal/phydm/halrf/halrf_kfree.c:232, hal/phydm/halrf/halrf_kfree.c:233, hal/phydm/halrf/halrf_kfree.c:490 … |
| 107 | `0x00DF` | RF_0xdf | RF (enum rf_path)pathABRF_PATH_ARF_PATH_Be_rf_pathi | A | Init table | 20 | R-mask RMW-mask | 5/0/0/6/4 | 0/0 | _iqk_restore_rf_8822b, _iqk_rf_setting_8822b, config_phydm_switch_channel_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:354, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:356, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:552 … |
| 108 | `0x00ED` | RF_0xed | RF | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 109 | `0x00EE` | RF_0xee | RF ABRF_PATH_Ae_rf_path | A | Init table | 20 | RMW-mask | 0/0/0/8/4 | 0/0 |  | bảng: array_mp_8822b_radioa, array_mp_8822b_radiob |
| 110 | `0x00EF` | RF_0xef (+1: rf_welut_jaguar) | RF (enum rf_path)i(enum rf_path)pathABRF_PATH_ARF_PATH_Be_rf_pathpathrf_path | A | Init table | 20 | RMW-mask | 21/0/0/12/189 | 0/0 | _iqk_restore_rf_8822b, _iqk_rf_setting_8822b, _txa_bias_cali_4_each_path, config_phydm_trx_mode_8822b … | hal/phydm/halrf/halrf_kfree.c:422, hal/phydm/halrf/halrf_kfree.c:431, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:340 … |
| 111 | `0x00F0` | RF_0x1bf0 | RF RF_PATH_A | A | Calibration | 20 | R-mask | 1/0/0/0/0 | 0/0 | _iqk_fail_count_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:164 |
| 112 | `0x00F5` | RF_0xf5 | RF e_rf_path | X | Calibration | 20 | RMW-mask | 0/0/0/2/0 | 0/0 | phydm_set_kfree_to_rf_8192f, phydm_set_kfree_to_rf_8198f | hal/phydm/halrf/halrf_kfree.c:1003, hal/phydm/halrf/halrf_kfree.c:720 |
