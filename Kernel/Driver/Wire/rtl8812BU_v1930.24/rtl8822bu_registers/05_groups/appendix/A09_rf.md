# Phụ lục A09 — RF path A/B 0x00–0xFF

Sinh tự động bởi `tools/appendix.py` từ `work/*.json` [TOOL]. Mỗi mục liệt kê **mọi** truy cập tìm thấy trong code active (kể cả nhánh chết, có ghi lý do) và mọi mục bảng. Các thanh ghi St=D/Z (không dùng) chỉ có trong `04*_catalog_*.md`. Cột "Trạng thái": `live` = chạy được với RTL8822BU; còn lại là lý do loại.

Tổng: 79 địa chỉ.

## `0x0000` — St A — `RF_0x0`, `RF_0x00`, `ODM_GAIN_SETTING`, `ODM_REG_RF_MODE_11N`

Định nghĩa: `RF_0x0 hal/phydm/phydm_regtable.h:867`; `RF_0x00 hal/phydm/phydm_regtable.h:868`; `ODM_REG_RF_MODE_11N hal/phydm/phydm_regdefine11n.h:62`; `ODM_GAIN_SETTING hal/phydm/phydm_reg.h:144`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:965` | mpt_SetSingleTone_8814A | phy_set_rf_reg | 20 | 0xF0000 | live | `phy_set_rf_reg(pAdapter, path, RF_AC_Jaguar, 0xF0000, 0x2); /*/ Tx mode: RF0x00[19:16]=4'b0010 */` |
| `hal/hal_mp.c:966` | mpt_SetSingleTone_8814A | phy_set_rf_reg | 20 | 0x1F | live | `phy_set_rf_reg(pAdapter, path, RF_AC_Jaguar, 0x1F, 0x0); /*/ Lowest RF gain index: RF_0x0[4:0] = 0*/` |
| `hal/hal_mp.c:1849` | hal_mpt_SetSingleToneTx | phy_query_rf_reg | 20 | bRFRegOffsetMask | live | `regRF = phy_query_rf_reg(pAdapter, RF_PATH_A, RF_AC_Jaguar, bRFRegOffsetMask);` |
| `hal/hal_mp.c:1859` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF0000 | live | `phy_set_rf_reg(pAdapter, p, RF_AC_Jaguar, 0xF0000, 0x2); /*/ Tx mode: RF0x00[19:16]=4'b0010 */` |
| `hal/hal_mp.c:1860` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0x1F | live | `phy_set_rf_reg(pAdapter, p, RF_AC_Jaguar, 0x1F, 0x0); /*/ Lowest RF gain index: RF_0x0[4:0] = 0*/` |
| `hal/hal_mp.c:1864` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF0000 | live | `phy_set_rf_reg(pAdapter, pMptCtx->mpt_rf_path, RF_AC_Jaguar, 0xF0000, 0x2); /*/ Tx mode: RF0x00[19:16]=4'b0010` |
| `hal/hal_mp.c:1865` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0x1F | live | `phy_set_rf_reg(pAdapter, pMptCtx->mpt_rf_path, RF_AC_Jaguar, 0x1F, 0x0); /*/ Lowest RF gain index: RF_0x0[4:0]` |
| `hal/hal_mp.c:1978` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | bRFRegOffsetMask | live | `phy_set_rf_reg(pAdapter, p, RF_AC_Jaguar, bRFRegOffsetMask, regRF);` |
| `hal/hal_mp.c:1983` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | bRFRegOffsetMask | live | `phy_set_rf_reg(pAdapter, p, RF_AC_Jaguar, bRFRegOffsetMask, regRF);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:483` | _phy_lc_calibrate_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x0, RFREGOFFSETMASK, 0x10000);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:484` | _phy_lc_calibrate_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x0, RFREGOFFSETMASK, 0x10000);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:505` | _phy_lc_calibrate_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x0, RFREGOFFSETMASK, 0x3ffff);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:506` | _phy_lc_calibrate_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x0, RFREGOFFSETMASK, 0x3ffff);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:254` | _iqk_bb_reset_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x0, MASK20BITS, 0x10000);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:255` | _iqk_bb_reset_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x0, MASK20BITS, 0x10000);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:354` | _iqk_restore_rf_8822b | odm_set_rf_reg (qua mảng backup_rf_reg) | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_A, backup_rf_reg[i], MASK20BITS,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:356` | _iqk_restore_rf_8822b | odm_set_rf_reg (qua mảng backup_rf_reg) | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_B, backup_rf_reg[i], MASK20BITS,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:851` | _iqk_rxk_gsearch_fail_8822b | odm_get_rf_reg | 20 | MASK20BITS | live | `rf_reg0 = odm_get_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:997` | _iqk_one_shot_8822b | odm_get_rf_reg | 20 | MASK20BITS | live | `odm_get_rf_reg(dm, (enum rf_path)path, RF_0x0,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1014` | _iqk_one_shot_8822b | odm_get_rf_reg | 20 | MASK20BITS | live | `(u16)(((odm_get_rf_reg(dm, (enum rf_path)path,` |
| `hal/hal_mp.c:1802` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF0000 | dead F:IS_HARDWARE_TYPE_8192E(pAdapter) | `phy_set_rf_reg(pAdapter, pMptCtx->mpt_rf_path, RF_AC, 0xF0000, 0x2); /*/ Tx mode*/` |
| `hal/hal_mp.c:1817` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF0000 | dead F:IS_HARDWARE_TYPE_8723B(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_AC, 0xF0000, 0x2); /*/ Tx mode*/` |
| `hal/hal_mp.c:1821` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF0000 | dead F:IS_HARDWARE_TYPE_8723B(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_AC, 0xF0000, 0x2); /*/ Tx mode*/` |
| `hal/hal_mp.c:1826` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF0000 | dead F:IS_HARDWARE_TYPE_8703B(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_AC, 0xF0000, 0x2); /* Tx mode */` |
| `hal/hal_mp.c:1833` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF0000 | dead F:IS_HARDWARE_TYPE_8188F(pAdapter) \|\| IS_HARDWARE_TYPE_8188GTV | `phy_set_rf_reg(pAdapter, pMptCtx->mpt_rf_path, RF_AC, 0xF0000, 0x2);` |
| `hal/hal_mp.c:1838` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT16 | dead F:IS_HARDWARE_TYPE_8723D(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_AC, BIT16, 0x0);` |
| `hal/hal_mp.c:1842` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT16 | dead F:IS_HARDWARE_TYPE_8723D(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_AC, BIT16, 0x0);` |
| `hal/hal_mp.c:1925` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF0000 | dead F:IS_HARDWARE_TYPE_8192E(pAdapter) | `phy_set_rf_reg(pAdapter, pMptCtx->mpt_rf_path, RF_AC, 0xF0000, 0x3);/*/ Tx mode*/` |
| `hal/hal_mp.c:1943` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF0000 | dead F:IS_HARDWARE_TYPE_8723B(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_AC, 0xF0000, 0x3); /*/ Rx mode*/` |
| `hal/hal_mp.c:1947` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF0000 | dead F:IS_HARDWARE_TYPE_8723B(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_AC, 0xF0000, 0x3); /*/ Rx mode*/` |
| `hal/hal_mp.c:1952` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF0000 | dead F:IS_HARDWARE_TYPE_8703B(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_AC, 0xF0000, 0x3); /* Rx mode */` |
| `hal/hal_mp.c:1956` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF0000 | dead F:IS_HARDWARE_TYPE_8188F(pAdapter) \|\| IS_HARDWARE_TYPE_8188GTV | `phy_set_rf_reg(pAdapter, pMptCtx->mpt_rf_path, RF_AC, 0xF0000, 0x3); /*Tx mode*/` |
| `hal/hal_mp.c:1963` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT16 | dead F:IS_HARDWARE_TYPE_8723D(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_AC, BIT16, 0x1);` |
| `hal/hal_mp.c:1967` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT16 | dead F:IS_HARDWARE_TYPE_8723D(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_AC, BIT16, 0x1);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:157`): 1 mục, giá trị 0x30000; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4807`): 1 mục, giá trị 0x30000; điều kiện None

## `0x0001` — St A — `RF_0x1`

Định nghĩa: `RF_0x1 hal/phydm/phydm_regtable.h:872`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:354` | _iqk_restore_rf_8822b | odm_set_rf_reg (qua mảng backup_rf_reg) | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_A, backup_rf_reg[i], MASK20BITS,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:356` | _iqk_restore_rf_8822b | odm_set_rf_reg (qua mảng backup_rf_reg) | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_B, backup_rf_reg[i], MASK20BITS,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1277` | _iqk_start_iqk_8822b | odm_get_rf_reg | 20 | MASK20BITS | cfg-off-func | `tmp = odm_get_rf_reg(dm, RF_PATH_A, RF_0x1, MASK20BITS);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1279` | _iqk_start_iqk_8822b | odm_set_rf_reg | 20 | MASK20BITS | cfg-off-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x1, MASK20BITS, tmp);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1281` | _iqk_start_iqk_8822b | odm_get_rf_reg | 20 | MASK20BITS | cfg-off-func | `tmp = odm_get_rf_reg(dm, RF_PATH_B, RF_0x1, MASK20BITS);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1283` | _iqk_start_iqk_8822b | odm_set_rf_reg | 20 | MASK20BITS | cfg-off-func | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x1, MASK20BITS, tmp);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:159`): 24 mục, giá trị 0x40029, 0x4002d; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4809`): 24 mục, giá trị 0x40029, 0x4002d; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}

## `0x0008` — St A — `RF_0x8`, `RF_0x08`

Định nghĩa: `RF_0x08 hal/phydm/phydm_regtable.h:869`; `RF_0x8 hal/phydm/phydm_regtable.h:911`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:78` | phydm_get_read_counter_8822b | odm_get_rf_reg | 20 | MASK20BITS | live | `rf_reg = odm_get_rf_reg(dm, RF_PATH_A, RF_0x8, MASK20BITS);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:85` | phydm_get_read_counter_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x8, MASK20BITS, 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:764` | halrf_iqk_set_rf0x8 | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:766` | halrf_iqk_set_rf0x8 | odm_get_rf_reg | 20 | MASK20BITS | live | `if (odm_get_rf_reg(dm, (enum rf_path)path, RF_0x8, MASK20BITS)` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:787` | _iqk_check_cal_8822b | odm_get_rf_reg | 20 | MASK20BITS | live | `if (odm_get_rf_reg(dm, (enum rf_path)path, RF_0x8, MASK20BITS)` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:4611`): 2 mục, giá trị 0x9c060; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:8872`): 2 mục, giá trị 0x9c060; điều kiện None

## `0x0018` — St A — `RF_0x18`, `ODM_CHANNEL`

Định nghĩa: `RF_0x18 hal/phydm/phydm_regtable.h:873`; `ODM_CHANNEL hal/phydm/phydm_reg.h:145`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_wlan_util.c:772` | set_channel_bwmode | rtw_hal_read_rfreg | 20 | 0xffffffff | live | `channel, rtw_hal_read_rfreg(padapter, 0, 0x18, 0xffffffff), channel_offset, bwmode);` |
| `hal/hal_mcc.c:2755` | mcc_get_reg_hdl | rtw_hal_read_rfreg (qua mảng dbg_rf_reg) | 20 | 0xffffffff | live | `reg_val = rtw_hal_read_rfreg(adapter, path, dbg_rf_reg[i], 0xffffffff);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:486` | _phy_lc_calibrate_8822b | odm_get_rf_reg | 20 | RFREGOFFSETMASK | live | `lc_cal = odm_get_rf_reg(dm, RF_PATH_A, RF_CHNLBW, RFREGOFFSETMASK);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:490` | _phy_lc_calibrate_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_CHNLBW, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:494` | _phy_lc_calibrate_8822b | odm_get_rf_reg | 20 | 0x8000 | live | `if (odm_get_rf_reg(dm, RF_PATH_A, RF_CHNLBW, 0x8000) != 0x1)` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:499` | _phy_lc_calibrate_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_CHNLBW, RFREGOFFSETMASK, lc_cal);` |
| `hal/phydm/phydm_psd.c:166` | phydm_psd | odm_get_rf_reg | 20 | RFREG_MASK | live | `dm_psd_table->rf_0x18_bkp = odm_get_rf_reg(dm, RF_PATH_A, RF_0x18,` |
| `hal/phydm/phydm_psd.c:168` | phydm_psd | odm_get_rf_reg | 20 | RFREG_MASK | live | `dm_psd_table->rf_0x18_bkp_b = odm_get_rf_reg(dm, RF_PATH_B, RF_0x18,` |
| `hal/phydm/phydm_psd.c:199` | phydm_psd | odm_set_rf_reg | 20 | 0xff | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x18, 0xff, psd_fc_channel);` |
| `hal/phydm/phydm_psd.c:200` | phydm_psd | odm_set_rf_reg | 20 | 0xff | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x18, 0xff, psd_fc_channel);` |
| `hal/phydm/phydm_psd.c:201` | phydm_psd | odm_set_rf_reg | 20 | 0x300 | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x18, 0x300, is_5G);` |
| `hal/phydm/phydm_psd.c:202` | phydm_psd | odm_set_rf_reg | 20 | 0x300 | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x18, 0x300, is_5G);` |
| `hal/phydm/phydm_psd.c:227` | phydm_psd | odm_set_rf_reg | 20 | 0xc00 | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x18, 0xc00,` |
| `hal/phydm/phydm_psd.c:230` | phydm_psd | odm_set_rf_reg | 20 | 0xc00 | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x18, 0xc00,` |
| `hal/phydm/phydm_psd.c:233` | phydm_psd | odm_set_rf_reg | 20 | 0xf0000 | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x18, 0xf0000,` |
| `hal/phydm/phydm_psd.c:235` | phydm_psd | odm_set_rf_reg | 20 | 0xf0000 | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x18, 0xf0000,` |
| `hal/phydm/phydm_psd.c:246` | phydm_psd | odm_get_rf_reg | 20 | RFREG_MASK | live | `odm_get_rf_reg(dm, RF_PATH_A, RF_0x18, RFREG_MASK));` |
| `hal/phydm/phydm_psd.c:308` | phydm_psd | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x18, RFREG_MASK,` |
| `hal/phydm/phydm_psd.c:310` | phydm_psd | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x18, RFREG_MASK,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1602` | config_phydm_switch_band_8822b | config_phydm_read_rf_reg_8822b | 20 | RFREGOFFSETMASK | live | `rf_reg18 = config_phydm_read_rf_reg_8822b(dm, RF_PATH_A, 0x18,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1742` | config_phydm_switch_band_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x18, RFREGOFFSETMASK, rf_reg18);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1745` | config_phydm_switch_band_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x18, RFREGOFFSETMASK,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1881` | config_phydm_switch_channel_8822b | config_phydm_read_rf_reg_8822b | 20 | RFREGOFFSETMASK | live | `rf_reg18 = config_phydm_read_rf_reg_8822b(dm, RF_PATH_A, 0x18,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2000` | config_phydm_switch_channel_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x18, RFREGOFFSETMASK, rf_reg18);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2003` | config_phydm_switch_channel_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x18, RFREGOFFSETMASK,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2077` | config_phydm_switch_bandwidth_8822b | config_phydm_read_rf_reg_8822b | 20 | RFREG_MASK | live | `rf_reg18 = config_phydm_read_rf_reg_8822b(dm, RF_PATH_A, 0x18,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2267` | config_phydm_switch_bandwidth_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x18, RFREGOFFSETMASK, rf_reg18);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2270` | config_phydm_switch_bandwidth_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x18, RFREGOFFSETMASK,` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:134` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x18, 0xFFFFF, 0X10124);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:137` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x18, 0xFFFFF, 0X10524);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:140` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x18, 0xFFFFF, 0X10924);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:143` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x18, 0xFFFFF, 0X10D24);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:146` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x18, 0xFFFFF, 0X30164);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:149` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x18, 0xFFFFF, 0X30564);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:152` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x18, 0xFFFFF, 0X30964);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:155` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x18, 0xFFFFF, 0X30D64);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:158` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x18, 0xFFFFF, 0X50195);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:161` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x18, 0xFFFFF, 0X50595);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:164` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x18, 0xFFFFF, 0X50995);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:167` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x18, 0xFFFFF, 0X50D95);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:285` | phydm_txcurrentcalibration | odm_get_rf_reg | 20 | 0xFFFFF | live | `orig_rf0x18_path_a = odm_get_rf_reg(dm, RF_PATH_A, RF_0x18, 0xFFFFF);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:286` | phydm_txcurrentcalibration | odm_get_rf_reg | 20 | 0xFFFFF | live | `orig_rf0x18_path_b = odm_get_rf_reg(dm, RF_PATH_B, RF_0x18, 0xFFFFF);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:304` | phydm_txcurrentcalibration | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x18, 0xFFFFF, orig_rf0x18_path_a);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:305` | phydm_txcurrentcalibration | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x18, 0xFFFFF, orig_rf0x18_path_b);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:495` | _iqk_reload_iqk_8822b | odm_get_rf_reg | 20 | MASK20BITS | cfg-off-func | `iqk->rf_reg18 = odm_get_rf_reg(dm, RF_PATH_A,` |
| `hal/phydm/phydm_psd.c:206` | phydm_psd | odm_set_rf_reg | 20 | 0x3000 | dead F:dm->support_ic_type & (ODM_RTL8822C \| ODM_RTL8812F \| ODM_RTL | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x18, 0x3000,` |
| `hal/phydm/phydm_psd.c:208` | phydm_psd | odm_set_rf_reg | 20 | 0x3000 | dead F:dm->support_ic_type & (ODM_RTL8822C \| ODM_RTL8812F \| ODM_RTL | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x18, 0x3000,` |
| `hal/phydm/phydm_psd.c:211` | phydm_psd | odm_set_rf_reg | 20 | 0x70000 | dead F:dm->support_ic_type & (ODM_RTL8822C \| ODM_RTL8812F \| ODM_RTL | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x18, 0x70000,` |
| `hal/phydm/phydm_psd.c:213` | phydm_psd | odm_set_rf_reg | 20 | 0x70000 | dead F:dm->support_ic_type & (ODM_RTL8822C \| ODM_RTL8812F \| ODM_RTL | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x18, 0x70000,` |
| `hal/phydm/phydm_psd.c:218` | phydm_psd | odm_set_rf_reg | 20 | 0x1c00 | dead F:dm->support_ic_type == ODM_RTL8721D | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x18, 0x1c00,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:710` | phydm_rfe_8822b_lps | config_phydm_read_rf_reg_8822b | 20 | 0xff | no-caller | `rf_reg18_ch = config_phydm_read_rf_reg_8822b(dm, RF_PATH_A, 0x18, 0xff);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:207`): 5 mục, giá trị 0x10524, 0x10d24, 0x18d24, 0x401; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4857`): 5 mục, giá trị 0x10524, 0x10d24, 0x18d24, 0x401; điều kiện None

## `0x001B` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:1007`): 1 mục, giá trị 0x75a40; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:5345`): 1 mục, giá trị 0x75a40; điều kiện None

## `0x0024` — St X — `ODM_RF_T_METER`, `RF_T_METER_OLD`

Định nghĩa: `RF_T_METER_OLD hal/phydm/phydm_reg.h:77`; `ODM_RF_T_METER hal/phydm/phydm_reg.h:146`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_powertracking_ce.c:877` | odm_txpowertracking_check_ce | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead F:dm->support_ic_type & (ODM_RTL8188E \| ODM_RTL8188F \| ODM_RTL | `odm_set_rf_reg(dm, RF_PATH_A, RF_T_METER_OLD,` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:926` | odm_txpowertracking_direct_ce | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead/no-caller F:dm->support_ic_type & (ODM_RTL8188E \| ODM_RTL8188F \ | `odm_set_rf_reg(dm, RF_PATH_A, RF_T_METER_OLD, RFREGOFFSETMASK, 0x60);` |

## `0x0030` — St A — `RF_0x30`, `rf_mode_table_addr`

Định nghĩa: `rf_mode_table_addr include/hal_phy_reg.h:306`; `RF_0x30 hal/phydm/phydm_regtable.h:879`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:239` | _set_tx_a_cali_value | odm_set_rf_reg | 20 | 0xFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0x30, 0xFFFF, (offset << 12 \| (modi_tx_a_value & 0xFF0) \| tmp1_byte));` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:1137`): 810 mục, giá trị 0x1203, 0x12a1, 0x12a6, 0x1303, 0x1333, 0x1334, 0x1343, 0x1361 …; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:5475`): 810 mục, giá trị 0x1203, 0x1222, 0x12a1, 0x12a6, 0x12d0, 0x12f4, 0x1303, 0x1333 …; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}

## `0x0033` — St A — `RF_0x33`

Định nghĩa: `RF_0x33 hal/phydm/phydm_regtable.h:882`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_kfree.c:423` | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0x0);` |
| `hal/phydm/halrf/halrf_kfree.c:425` | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | 20 | BIT(0) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, BIT(0), 0x1);` |
| `hal/phydm/halrf/halrf_kfree.c:427` | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | 20 | BIT(1) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, BIT(1), 0x1);` |
| `hal/phydm/halrf/halrf_kfree.c:429` | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | 20 | (BIT(1) \| BIT(0)) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, (BIT(1) \| BIT(0)), 0x3);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:567` | _iqk_rf_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:578` | _iqk_rf_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/phydm_direct_bf.c:78` | phydm_iq_gen_en | odm_set_rf_reg | 20 | 0xF | live | `odm_set_rf_reg(dm, path, RF_0x33, 0xF, 3);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2539` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x33, RFREG_MASK, 0x00001);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2542` | config_phydm_trx_mode_8822b | config_phydm_read_rf_reg_8822b | 20 | RFREG_MASK | live | `rf_reg33 = config_phydm_read_rf_reg_8822b(dm, RF_PATH_A, 0x33,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2560` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x33, RFREG_MASK, 0x00001);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2570` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x33, RFREG_MASK, 0x00001);` |
| `hal/phydm/txbf/haltxbf8822b.c:1066` | phydm_8822btxbf_rfmode | odm_set_rf_reg | 20 | 0xF | live | `odm_set_rf_reg(dm, (enum rf_path)i, RF_0x33, 0xF, 3); /*Select RX mode*/` |
| `hal/phydm/halrf/halrf_kfree.c:728` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 0);` |
| `hal/phydm/halrf/halrf_kfree.c:729` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, 0x33, 0x3F, data);` |
| `hal/phydm/halrf/halrf_kfree.c:731` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 1);` |
| `hal/phydm/halrf/halrf_kfree.c:732` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, 0x33, 0x3F, data);` |
| `hal/phydm/halrf/halrf_kfree.c:735` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 2);` |
| `hal/phydm/halrf/halrf_kfree.c:736` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, 0x33, 0x3F, data);` |
| `hal/phydm/halrf/halrf_kfree.c:738` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 3);` |
| `hal/phydm/halrf/halrf_kfree.c:739` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, 0x33, 0x3F, data);` |
| `hal/phydm/halrf/halrf_kfree.c:741` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 4);` |
| `hal/phydm/halrf/halrf_kfree.c:742` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, 0x33, 0x3F, data);` |
| `hal/phydm/halrf/halrf_kfree.c:744` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 5);` |
| `hal/phydm/halrf/halrf_kfree.c:745` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, 0x33, 0x3F, data);` |
| `hal/phydm/halrf/halrf_kfree.c:749` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 0);` |
| `hal/phydm/halrf/halrf_kfree.c:750` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, 0x33, 0x3F, data);` |
| `hal/phydm/halrf/halrf_kfree.c:752` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 1);` |
| `hal/phydm/halrf/halrf_kfree.c:753` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, 0x33, 0x3F, data);` |
| `hal/phydm/halrf/halrf_kfree.c:755` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 2);` |
| `hal/phydm/halrf/halrf_kfree.c:756` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, 0x33, 0x3F, data);` |
| `hal/phydm/halrf/halrf_kfree.c:758` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 3);` |
| `hal/phydm/halrf/halrf_kfree.c:759` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, 0x33, 0x3F, data);` |
| `hal/phydm/halrf/halrf_kfree.c:761` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 4);` |
| `hal/phydm/halrf/halrf_kfree.c:762` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, 0x33, 0x3F, data);` |
| `hal/phydm/halrf/halrf_kfree.c:764` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 5);` |
| `hal/phydm/halrf/halrf_kfree.c:765` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, 0x33, 0x3F, data);` |
| `hal/phydm/halrf/halrf_kfree.c:1012` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 0);` |
| `hal/phydm/halrf/halrf_kfree.c:1013` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x3F, pwr_offset[0]);` |
| `hal/phydm/halrf/halrf_kfree.c:1014` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 1);` |
| `hal/phydm/halrf/halrf_kfree.c:1015` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x3F, pwr_offset[0]);` |
| `hal/phydm/halrf/halrf_kfree.c:1016` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 2);` |
| `hal/phydm/halrf/halrf_kfree.c:1017` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x3F, pwr_offset[1]);` |
| `hal/phydm/halrf/halrf_kfree.c:1018` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 3);` |
| `hal/phydm/halrf/halrf_kfree.c:1019` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x3F, pwr_offset[1]);` |
| `hal/phydm/halrf/halrf_kfree.c:1020` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 4);` |
| `hal/phydm/halrf/halrf_kfree.c:1021` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x3F, pwr_offset[2]);` |
| `hal/phydm/halrf/halrf_kfree.c:1022` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | 0x70000 | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x70000, 5);` |
| `hal/phydm/halrf/halrf_kfree.c:1023` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | 0x3F | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, 0x3F, pwr_offset[2]);` |
| `hal/phydm/halrf/halrf_kfree.c:1126` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0x0);` |
| `hal/phydm/halrf/halrf_kfree.c:1129` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0x1);` |
| `hal/phydm/halrf/halrf_kfree.c:1132` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0x2);` |
| `hal/phydm/halrf/halrf_kfree.c:1135` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0x3);` |
| `hal/phydm/halrf/halrf_kfree.c:1138` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0x4);` |
| `hal/phydm/halrf/halrf_kfree.c:1141` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0x5);` |
| `hal/phydm/halrf/halrf_kfree.c:1144` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0x6);` |
| `hal/phydm/halrf/halrf_kfree.c:1147` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0x7);` |
| `hal/phydm/halrf/halrf_kfree.c:1150` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0x8);` |
| `hal/phydm/halrf/halrf_kfree.c:1153` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0x9);` |
| `hal/phydm/halrf/halrf_kfree.c:1156` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0xa);` |
| `hal/phydm/halrf/halrf_kfree.c:1159` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x33, RFREGOFFSETMASK, 0xb);` |
| … | còn 47 truy cập | | | | | |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:209`): 1193 mục, giá trị 0x0, 0x1, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 …; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4859`): 1186 mục, giá trị 0x0, 0x1, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 …; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}

## `0x003E` — St A — `RF_0x3e`

Định nghĩa: `RF_0x3e hal/phydm/phydm_regtable.h:884`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:569` | _iqk_rf_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:580` | _iqk_rf_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/phydm_direct_bf.c:80` | phydm_iq_gen_en | odm_set_rf_reg | 20 | 0xfffff | live | `odm_set_rf_reg(dm, path, RF_0x3e, 0xfffff, 0x00036);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2561` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3e, RFREG_MASK, 0x00034);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2571` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3e, RFREG_MASK, 0x00034);` |
| `hal/phydm/txbf/haltxbf8822b.c:1067` | phydm_8822btxbf_rfmode | odm_set_rf_reg | 20 | 0xfffff | live | `odm_set_rf_reg(dm, (enum rf_path)i, RF_0x3e, 0xfffff, 0x00036); /*Set Table data*/` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:83` | halrf_rf_lna_setting_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, (enum rf_path)path, RF_0x3e,` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:95` | halrf_rf_lna_setting_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, (enum rf_path)path, RF_0x3e,` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:210`): 144 mục, giá trị 0x34, 0x37, 0x3f, 0x4000, 0x4040, 0x4080, 0x4084, 0x40c8 …; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4860`): 105 mục, giá trị 0x34, 0x37, 0x3f, 0x4000, 0x4040, 0x4080, 0x40c8, 0x40d0 …; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}

## `0x003F` — St A — `RF_0x3f`

Định nghĩa: `RF_0x3f hal/phydm/phydm_regtable.h:885`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_kfree.c:424` | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK, rf_reg_3f);` |
| `hal/phydm/halrf/halrf_kfree.c:426` | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK, rf_reg_3f);` |
| `hal/phydm/halrf/halrf_kfree.c:428` | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK, rf_reg_3f);` |
| `hal/phydm/halrf/halrf_kfree.c:430` | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK, rf_reg_3f);` |
| `hal/phydm/halrf/halrf_kfree.c:435` | phydm_set_pa_bias_to_rf_8822b | odm_get_rf_reg | 20 | (BIT(12) \| BIT(11) \| B | live | `odm_get_rf_reg(dm, e_rf_path, RF_0x3f,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:571` | _iqk_rf_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:582` | _iqk_rf_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/phydm_direct_bf.c:82` | phydm_iq_gen_en | odm_set_rf_reg | 20 | 0xfffff | live | `odm_set_rf_reg(dm, path, RF_0x3f, 0xfffff, 0x5AFCE);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2562` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, RFREG_MASK, 0x4080e);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2572` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, RFREG_MASK, 0x4080c);` |
| `hal/phydm/txbf/haltxbf8822b.c:1068` | phydm_8822btxbf_rfmode | odm_set_rf_reg | 20 | 0xfffff | live | `odm_set_rf_reg(dm, (enum rf_path)i, RF_0x3f, 0xfffff, 0x5AFCE); /*Set Table data*/` |
| `hal/phydm/halrf/halrf_kfree.c:1127` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1130` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1133` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1136` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1139` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1142` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1145` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1148` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1151` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1154` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1157` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1160` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1163` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1166` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1169` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1501` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1504` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1507` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1510` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1513` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1516` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1519` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1522` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1525` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1528` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1531` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x3f, RFREGOFFSETMASK,` |
| `hal/phydm/halrf/halrf_kfree.c:1842` | phydm_set_power_trim_rf_8195b | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:1845` | phydm_set_power_trim_rf_8195b | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:1848` | phydm_set_power_trim_rf_8195b | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:1851` | phydm_set_power_trim_rf_8195b | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:1854` | phydm_set_power_trim_rf_8195b | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:1857` | phydm_set_power_trim_rf_8195b | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:1860` | phydm_set_power_trim_rf_8195b | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:1863` | phydm_set_power_trim_rf_8195b | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:1866` | phydm_set_power_trim_rf_8195b | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:2000` | phydm_set_power_trim_rf_8721d | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:2003` | phydm_set_power_trim_rf_8721d | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:2006` | phydm_set_power_trim_rf_8721d | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:2010` | phydm_set_power_trim_rf_8721d | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:2013` | phydm_set_power_trim_rf_8721d | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:2016` | phydm_set_power_trim_rf_8721d | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:2019` | phydm_set_power_trim_rf_8721d | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/halrf_kfree.c:2022` | phydm_set_power_trim_rf_8721d | odm_set_rf_reg | 20 | 0x0000003f | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x3f, 0x0000003f,` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:85` | halrf_rf_lna_setting_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, (enum rf_path)path, RF_0x3f,` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:97` | halrf_rf_lna_setting_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, (enum rf_path)path, RF_0x3f,` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:212`): 1291 mục, giá trị 0x0, 0x10, 0x1726, 0x18, 0x2, 0x20, 0x26, 0x28 …; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4862`): 1284 mục, giá trị 0x0, 0x10, 0x18, 0x2, 0x20, 0x26, 0x265a, 0x28 …; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}

## `0x0042` — St A — `R_0x42`, `RF_0x42`, `RF_T_METER_NEW`, `ODM_RF_T_METER_88E`, `ODM_RF_T_METER_92D`, `ODM_RF_T_METER_92E`, `ODM_RF_T_METER_8812`

Định nghĩa: `R_0x42 hal/phydm/phydm_regtable.h:384`; `RF_0x42 hal/phydm/phydm_regtable.h:887`; `RF_T_METER_NEW hal/phydm/phydm_reg.h:78`; `ODM_RF_T_METER_92D hal/phydm/phydm_reg.h:147`; `ODM_RF_T_METER_88E hal/phydm/phydm_reg.h:148`; `ODM_RF_T_METER_92E hal/phydm/phydm_reg.h:149`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1664` | hal_mpt_TriggerRFThermalMeter | phy_set_rf_reg | 20 | BIT17 \| BIT16 | live | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x42, BIT17 \| BIT16, 0x03);` |
| `hal/hal_mp.c:1679` | hal_mpt_ReadRFThermalMeter | phy_query_rf_reg | 20 | 0xfc00 | live | `ThermalValue = (u8)phy_query_rf_reg(pAdapter, rf_path, 0x42, 0xfc00);	/*0x42: RF Reg[15:10]*/` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:861` | odm_txpowertracking_check_ce | odm_set_rf_reg | 20 | (BIT(17) \| BIT(16)) | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_T_METER_NEW,` |
| `hal/hal_mp.c:1660` | hal_mpt_TriggerRFThermalMeter | phy_set_rf_reg | 20 | BIT19 | dead F:IS_HARDWARE_TYPE_JAGUAR3(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x42, BIT19, 0x1);` |
| `hal/hal_mp.c:1661` | hal_mpt_TriggerRFThermalMeter | phy_set_rf_reg | 20 | BIT19 | dead F:IS_HARDWARE_TYPE_JAGUAR3(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x42, BIT19, 0x0);` |
| `hal/hal_mp.c:1662` | hal_mpt_TriggerRFThermalMeter | phy_set_rf_reg | 20 | BIT19 | dead F:IS_HARDWARE_TYPE_JAGUAR3(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x42, BIT19, 0x1);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:864` | odm_txpowertracking_check_ce | odm_set_rf_reg | 20 | BIT(19) | dead F:dm->support_ic_type & ODM_RTL8822C | `odm_set_rf_reg(dm, RF_PATH_A, R_0x42, BIT(19), 0x01);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:865` | odm_txpowertracking_check_ce | odm_set_rf_reg | 20 | BIT(19) | dead F:dm->support_ic_type & ODM_RTL8822C | `odm_set_rf_reg(dm, RF_PATH_A, R_0x42, BIT(19), 0x00);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:866` | odm_txpowertracking_check_ce | odm_set_rf_reg | 20 | BIT(19) | dead F:dm->support_ic_type & ODM_RTL8822C | `odm_set_rf_reg(dm, RF_PATH_A, R_0x42, BIT(19), 0x01);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:868` | odm_txpowertracking_check_ce | odm_set_rf_reg | 20 | BIT(19) | dead F:dm->support_ic_type & ODM_RTL8822C | `odm_set_rf_reg(dm, RF_PATH_B, R_0x42, BIT(19), 0x01);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:869` | odm_txpowertracking_check_ce | odm_set_rf_reg | 20 | BIT(19) | dead F:dm->support_ic_type & ODM_RTL8822C | `odm_set_rf_reg(dm, RF_PATH_B, R_0x42, BIT(19), 0x00);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:870` | odm_txpowertracking_check_ce | odm_set_rf_reg | 20 | BIT(19) | dead F:dm->support_ic_type & ODM_RTL8822C | `odm_set_rf_reg(dm, RF_PATH_B, R_0x42, BIT(19), 0x01);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:872` | odm_txpowertracking_check_ce | odm_set_rf_reg | 20 | BIT(17) | dead F:dm->support_ic_type & ODM_RTL8814B | `odm_set_rf_reg(dm, RF_PATH_A, 0x42, BIT(17), 0x1);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:873` | odm_txpowertracking_check_ce | odm_set_rf_reg | 20 | BIT(17) | dead F:dm->support_ic_type & ODM_RTL8814B | `odm_set_rf_reg(dm, RF_PATH_B, 0x42, BIT(17), 0x1);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:874` | odm_txpowertracking_check_ce | odm_set_rf_reg | 20 | BIT(17) | dead F:dm->support_ic_type & ODM_RTL8814B | `odm_set_rf_reg(dm, RF_PATH_C, 0x42, BIT(17), 0x1);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:875` | odm_txpowertracking_check_ce | odm_set_rf_reg | 20 | BIT(17) | dead F:dm->support_ic_type & ODM_RTL8814B | `odm_set_rf_reg(dm, RF_PATH_D, 0x42, BIT(17), 0x1);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:916` | odm_txpowertracking_direct_ce | odm_set_rf_reg | 20 | (BIT(17) \| BIT(16)) | no-caller T:dm->support_ic_type & (ODM_RTL8188E \| ODM_RTL8188F \| ODM | `odm_set_rf_reg(dm, RF_PATH_A, RF_T_METER_NEW, (BIT(17) \| BIT(16)), 0x03);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:918` | odm_txpowertracking_direct_ce | odm_set_rf_reg | 20 | BIT(19) | dead/no-caller F:dm->support_ic_type & ODM_RTL8822C | `odm_set_rf_reg(dm, RF_PATH_A, R_0x42, BIT(19), 0x01);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:919` | odm_txpowertracking_direct_ce | odm_set_rf_reg | 20 | BIT(19) | dead/no-caller F:dm->support_ic_type & ODM_RTL8822C | `odm_set_rf_reg(dm, RF_PATH_A, R_0x42, BIT(19), 0x00);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:920` | odm_txpowertracking_direct_ce | odm_set_rf_reg | 20 | BIT(19) | dead/no-caller F:dm->support_ic_type & ODM_RTL8822C | `odm_set_rf_reg(dm, RF_PATH_A, R_0x42, BIT(19), 0x01);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:922` | odm_txpowertracking_direct_ce | odm_set_rf_reg | 20 | BIT(19) | dead/no-caller F:dm->support_ic_type & ODM_RTL8822C | `odm_set_rf_reg(dm, RF_PATH_B, R_0x42, BIT(19), 0x01);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:923` | odm_txpowertracking_direct_ce | odm_set_rf_reg | 20 | BIT(19) | dead/no-caller F:dm->support_ic_type & ODM_RTL8822C | `odm_set_rf_reg(dm, RF_PATH_B, R_0x42, BIT(19), 0x00);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:924` | odm_txpowertracking_direct_ce | odm_set_rf_reg | 20 | BIT(19) | dead/no-caller F:dm->support_ic_type & ODM_RTL8822C | `odm_set_rf_reg(dm, RF_PATH_B, R_0x42, BIT(19), 0x01);` |

## `0x0043` — St X — `RF_0x43`

Định nghĩa: `RF_0x43 hal/phydm/phydm_regtable.h:888`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_kfree.c:1092` | phydm_get_set_thermal_trim_offset_8822c | odm_set_rf_reg | 20 | 0x000f0000 | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x43, 0x000f0000, thermal[RF_PATH_A]);` |
| `hal/phydm/halrf/halrf_kfree.c:1101` | phydm_get_set_thermal_trim_offset_8822c | odm_set_rf_reg | 20 | 0x000f0000 | dead-func | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x43, 0x000f0000, thermal[RF_PATH_B]);` |
| `hal/phydm/halrf/halrf_kfree.c:1452` | phydm_get_set_thermal_trim_offset_8812f | odm_set_rf_reg | 20 | 0x000f0000 | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0x43, 0x000f0000, thermal[RF_PATH_A]);` |
| `hal/phydm/halrf/halrf_kfree.c:1461` | phydm_get_set_thermal_trim_offset_8812f | odm_set_rf_reg | 20 | 0x000f0000 | dead-func | `odm_set_rf_reg(dm, RF_PATH_B, RF_0x43, 0x000f0000, thermal[RF_PATH_B]);` |

## `0x0051` — St A — `RF_0x51`

Định nghĩa: `RF_0x51 hal/phydm/phydm_regtable.h:889`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_kfree.c:387` | phydm_set_pa_bias_to_rf_8822b | odm_get_rf_reg | 20 | RFREGOFFSETMASK | live | `rf_reg_51 = odm_get_rf_reg(dm, e_rf_path, RF_0x51, RFREGOFFSETMASK);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:4223`): 11 mục, giá trị 0x808, 0xc00; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:8457`): 11 mục, giá trị 0x808, 0xc00; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}

## `0x0052` — St A — `RF_0x52`

Định nghĩa: `RF_0x52 hal/phydm/phydm_regtable.h:890`; `RF_0x52 include/rtl8822b_hal.h:253`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_mp.c:651` | MPT_InitializeAdapter | phy_query_rf_reg | 20 | 0x000F0 | live | `pMptCtx->backup0x52_RF_A = (u8)phy_query_rf_reg(pAdapter, RF_PATH_A, RF_0x52, 0x000F0);` |
| `core/rtw_mp.c:652` | MPT_InitializeAdapter | phy_query_rf_reg | 20 | 0x000F0 | live | `pMptCtx->backup0x52_RF_B = (u8)phy_query_rf_reg(pAdapter, RF_PATH_B, RF_0x52, 0x000F0);` |
| `hal/phydm/halrf/halrf_kfree.c:388` | phydm_set_pa_bias_to_rf_8822b | odm_get_rf_reg | 20 | RFREGOFFSETMASK | live | `rf_reg_52 = odm_get_rf_reg(dm, e_rf_path, RF_0x52, RFREGOFFSETMASK);` |
| `hal/hal_mp.c:131` | hal_mpt_SwitchRfSetting | phy_query_rf_reg | 20 | 0x000F0 | dead F:IS_HARDWARE_TYPE_8188ES(pAdapter) && (1 <= ChannelToSw && Ch | `pMptCtx->backup0x52_RF_A = (u8)phy_query_rf_reg(pAdapter, RF_PATH_A, RF_0x52, 0x000F0);` |
| `hal/hal_mp.c:132` | hal_mpt_SwitchRfSetting | phy_query_rf_reg | 20 | 0x000F0 | dead F:IS_HARDWARE_TYPE_8188ES(pAdapter) && (1 <= ChannelToSw && Ch | `pMptCtx->backup0x52_RF_B = (u8)phy_query_rf_reg(pAdapter, RF_PATH_B, RF_0x52, 0x000F0);` |
| `hal/hal_mp.c:135` | hal_mpt_SwitchRfSetting | phy_set_rf_reg | 20 | 0x000F0 | dead F:IS_HARDWARE_TYPE_8188ES(pAdapter) && (1 <= ChannelToSw && Ch | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_0x52, 0x000F0, 0xB);` |
| `hal/hal_mp.c:136` | hal_mpt_SwitchRfSetting | phy_set_rf_reg | 20 | 0x000F0 | dead F:IS_HARDWARE_TYPE_8188ES(pAdapter) && (1 <= ChannelToSw && Ch | `phy_set_rf_reg(pAdapter, RF_PATH_B, RF_0x52, 0x000F0, 0xB);` |
| `hal/hal_mp.c:138` | hal_mpt_SwitchRfSetting | phy_set_rf_reg | 20 | 0x000F0 | dead F:IS_HARDWARE_TYPE_8188ES(pAdapter) && (1 <= ChannelToSw && Ch | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_0x52, 0x000F0, 0xD);` |
| `hal/hal_mp.c:139` | hal_mpt_SwitchRfSetting | phy_set_rf_reg | 20 | 0x000F0 | dead F:IS_HARDWARE_TYPE_8188ES(pAdapter) && (1 <= ChannelToSw && Ch | `phy_set_rf_reg(pAdapter, RF_PATH_B, RF_0x52, 0x000F0, 0xD);` |
| `hal/hal_mp.c:143` | hal_mpt_SwitchRfSetting | phy_set_rf_reg | 20 | 0x000F0 | dead F:IS_HARDWARE_TYPE_8188EE(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_0x52, 0x000F0, 0xB); /*RF 0x52 = 0x0007E4BD*/` |
| `hal/hal_mp.c:144` | hal_mpt_SwitchRfSetting | phy_set_rf_reg | 20 | 0x000F0 | dead F:IS_HARDWARE_TYPE_8188EE(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_B, RF_0x52, 0x000F0, 0xB); /*RF 0x52 = 0x0007E4BD*/` |
| `hal/hal_mp.c:146` | hal_mpt_SwitchRfSetting | phy_set_rf_reg | 20 | 0x000F0 | dead F:IS_HARDWARE_TYPE_8188EE(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_0x52, 0x000F0, 0x9); /*RF 0x52 = 0x0007E49D*/` |
| `hal/hal_mp.c:147` | hal_mpt_SwitchRfSetting | phy_set_rf_reg | 20 | 0x000F0 | dead F:IS_HARDWARE_TYPE_8188EE(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_B, RF_0x52, 0x000F0, 0x9); /*RF 0x52 = 0x0007E49D*/` |
| `hal/hal_mp.c:150` | hal_mpt_SwitchRfSetting | phy_set_rf_reg | 20 | 0x000F0 | dead F:IS_HARDWARE_TYPE_8188E(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, RF_0x52, 0x000F0, pMptCtx->backup0x52_RF_A);` |
| `hal/hal_mp.c:151` | hal_mpt_SwitchRfSetting | phy_set_rf_reg | 20 | 0x000F0 | dead F:IS_HARDWARE_TYPE_8188E(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_B, RF_0x52, 0x000F0, pMptCtx->backup0x52_RF_B);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:4224`): 11 mục, giá trị 0x7c241, 0x98002; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:8458`): 11 mục, giá trị 0x7c241, 0x98002; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}

## `0x0053` — St T — `RF_0x53`

Định nghĩa: `RF_0x53 hal/phydm/phydm_regtable.h:891`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1827` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF000 | dead F:IS_HARDWARE_TYPE_8703B(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x53, 0xF000, 0x1); /* RF LO enabled */` |
| `hal/hal_mp.c:1839` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT0 | dead F:IS_HARDWARE_TYPE_8723D(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x53, BIT0, 0x1);` |
| `hal/hal_mp.c:1953` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF000 | dead F:IS_HARDWARE_TYPE_8703B(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x53, 0xF000, 0x0); /* RF LO disabled */` |
| `hal/hal_mp.c:1964` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT0 | dead F:IS_HARDWARE_TYPE_8723D(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x53, BIT0, 0x0);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:4225`): 11 mục, giá trị 0x1c069, 0xfa47; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:8459`): 11 mục, giá trị 0x1c069, 0xfa47; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}

## `0x0054` — St T — `RF_0x54`

Định nghĩa: `RF_0x54 hal/phydm/phydm_regtable.h:892`

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:4226`): 11 mục, giá trị 0x58032, 0x78032; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:8460`): 11 mục, giá trị 0x58032, 0x78032; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}

## `0x0055` — St A — `RF_0x55`, `REG_RF_TX_GAIN_OFFSET`

Định nghĩa: `RF_0x55 hal/phydm/phydm_regtable.h:893`; `REG_RF_TX_GAIN_OFFSET hal/phydm/phydm_reg.h:151`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_rf.c:1317` | rtw_rf_set_tx_gain_offset | rtw_hal_read_rfreg | 20 | 0xffffffff | live | `RTW_INFO("kfree gain_offset 0x55:0x%x ", rtw_hal_read_rfreg(adapter, target_path, 0x55, 0xffffffff));` |
| `core/rtw_rf.c:1382` | rtw_rf_set_tx_gain_offset | rtw_hal_read_rfreg | 20 | 0xffffffff | live | `val32 = rtw_hal_read_rfreg(adapter, target_path, 0x55, 0xffffffff);` |
| `hal/phydm/halrf/halrf_kfree.c:234` | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(6) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(6), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:241` | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(19), (wlg & BIT(0)));` |
| `hal/phydm/halrf/halrf_kfree.c:242` | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | 20 | gain_bmask | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, gain_bmask, (wlg >> 1));` |
| `hal/phydm/halrf/halrf_kfree.c:247` | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(19), data & BIT(0));` |
| `hal/phydm/halrf/halrf_kfree.c:248` | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | 20 | gain_bmask | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, gain_bmask,` |
| `hal/phydm/halrf/halrf_kfree.c:254` | phydm_set_kfree_to_rf_8821c | odm_get_rf_reg | 20 | s_gain_bmask | live | `odm_get_rf_reg(dm, e_rf_path, RF_0x55, s_gain_bmask),` |
| `hal/phydm/halrf/halrf_kfree.c:493` | phydm_set_kfree_to_rf_8822b | odm_set_rf_reg | 20 | BIT(5) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(5), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:495` | phydm_set_kfree_to_rf_8822b | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(19), (data & BIT(0)));` |
| `hal/phydm/halrf/halrf_kfree.c:496` | phydm_set_kfree_to_rf_8822b | odm_set_rf_reg | 20 | gain_bmask | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, gain_bmask,` |
| `hal/phydm/halrf/halrf_kfree.c:500` | phydm_set_kfree_to_rf_8822b | odm_get_rf_reg | 20 | (BIT(19) \| BIT(18) \| B | live | `odm_get_rf_reg(dm, e_rf_path, RF_0x55,` |
| `hal/phydm/halrf/halrf_kfree.c:513` | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | 20 | BIT(5) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(5), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:515` | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(19), (data & BIT(0)));` |
| `hal/phydm/halrf/halrf_kfree.c:516` | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | 20 | gain_bmask | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, gain_bmask,` |
| `hal/phydm/halrf/halrf_kfree.c:522` | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | 20 | BIT(5) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(5), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:523` | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | 20 | BIT(7) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(7), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:527` | phydm_clear_kfree_to_rf_8822b | odm_get_rf_reg | 20 | (BIT(19) \| BIT(18) \| B | live | `odm_get_rf_reg(dm, e_rf_path, RF_0x55,` |
| `core/rtw_rf.c:1311` | rtw_rf_set_tx_gain_offset | rtw_hal_read_rfreg | 20 | 0xffffffff | dead F:IS_HARDWARE_TYPE_8723D(adapter) | `rtw_hal_read_rfreg(adapter, target_path, 0x55, 0xffffffff));` |
| `core/rtw_rf.c:1378` | rtw_rf_set_tx_gain_offset | rtw_hal_read_rfreg | 20 | 0xffffffff | dead F:IS_HARDWARE_TYPE_8723D(adapter) | `val32 = rtw_hal_read_rfreg(adapter, target_path, 0x55, 0xffffffff);` |
| `hal/phydm/halrf/halrf_kfree.c:76` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(19), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:79` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(19), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:86` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(14) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(14), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:87` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | tx_gain_bitmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, tx_gain_bitmask, 0);` |
| `hal/phydm/halrf/halrf_kfree.c:91` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(14) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(14), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:92` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | tx_gain_bitmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, tx_gain_bitmask, 0);` |
| `hal/phydm/halrf/halrf_kfree.c:96` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(14) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(14), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:97` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | tx_gain_bitmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, tx_gain_bitmask, 1);` |
| `hal/phydm/halrf/halrf_kfree.c:101` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(14) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(14), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:102` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | tx_gain_bitmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, tx_gain_bitmask, 1);` |
| `hal/phydm/halrf/halrf_kfree.c:106` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(14) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(14), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:107` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | tx_gain_bitmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, tx_gain_bitmask, 2);` |
| `hal/phydm/halrf/halrf_kfree.c:111` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(14) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(14), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:112` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | tx_gain_bitmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, tx_gain_bitmask, 2);` |
| `hal/phydm/halrf/halrf_kfree.c:116` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(14) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(14), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:117` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | tx_gain_bitmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, tx_gain_bitmask, 3);` |
| `hal/phydm/halrf/halrf_kfree.c:121` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(14) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(14), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:122` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | tx_gain_bitmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, tx_gain_bitmask, 3);` |
| `hal/phydm/halrf/halrf_kfree.c:126` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(14) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(14), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:127` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | tx_gain_bitmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, tx_gain_bitmask, 4);` |
| `hal/phydm/halrf/halrf_kfree.c:131` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(14) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(14), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:132` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | tx_gain_bitmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, tx_gain_bitmask, 4);` |
| `hal/phydm/halrf/halrf_kfree.c:136` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | BIT(14) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(14), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:137` | phydm_set_kfree_to_rf_8814a | odm_set_rf_reg | 20 | tx_gain_bitmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, tx_gain_bitmask, 5);` |
| `hal/phydm/halrf/halrf_kfree.c:267` | phydm_clear_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(6) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(6), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:270` | phydm_clear_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(19), (data & BIT(0)));` |
| `hal/phydm/halrf/halrf_kfree.c:271` | phydm_clear_kfree_to_rf_8821c | odm_set_rf_reg | 20 | gain_bmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, gain_bmask, (data >> 1));` |
| `hal/phydm/halrf/halrf_kfree.c:278` | phydm_clear_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(6) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(6), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:283` | phydm_clear_kfree_to_rf_8821c | odm_get_rf_reg | 20 | s_gain_bmask | dead-func | `odm_get_rf_reg(dm, e_rf_path, RF_0x55, s_gain_bmask),` |
| `hal/phydm/halrf/halrf_kfree.c:591` | phydm_set_kfree_to_rf_8710b | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(19), (data & BIT(0)));` |
| `hal/phydm/halrf/halrf_kfree.c:592` | phydm_set_kfree_to_rf_8710b | odm_set_rf_reg | 20 | gain_bmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, gain_bmask, ((data & 0xf) >> 1));` |
| `hal/phydm/halrf/halrf_kfree.c:595` | phydm_set_kfree_to_rf_8710b | odm_get_rf_reg | 20 | (BIT(19) \| BIT(18) \| B | dead-func | `odm_get_rf_reg(dm, e_rf_path, RF_0x55,` |
| `hal/phydm/halrf/halrf_kfree.c:605` | phydm_clear_kfree_to_rf_8710b | odm_set_rf_reg | 20 | BIT(19) | no-caller | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(19), (data & BIT(0)));` |
| `hal/phydm/halrf/halrf_kfree.c:606` | phydm_clear_kfree_to_rf_8710b | odm_set_rf_reg | 20 | gain_bmask | no-caller | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, gain_bmask,` |
| `hal/phydm/halrf/halrf_kfree.c:611` | phydm_clear_kfree_to_rf_8710b | odm_get_rf_reg | 20 | (BIT(19) \| BIT(18) \| B | no-caller | `odm_get_rf_reg(dm, e_rf_path, RF_0x55,` |
| `hal/phydm/halrf/halrf_kfree.c:718` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | BIT(5) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(5), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:776` | phydm_set_kfree_to_rf_8192f | odm_get_rf_reg | 20 | (BIT(19) \| BIT(18) \| B | dead-func | `odm_get_rf_reg(dm, e_rf_path, RF_0x55,` |
| `hal/phydm/halrf/halrf_kfree.c:1001` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | BIT(5) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x55, BIT(5), 1);` |

## `0x0056` — St A — `RF_0x56`

Định nghĩa: `RF_0x56 hal/phydm/phydm_regtable.h:894`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:666` | _iqk_txk_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:673` | _iqk_txk_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:695` | _iqk_rxk1_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:705` | _iqk_rxk1_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:730` | _iqk_rxk2_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:746` | _iqk_rxk2_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:999` | _iqk_one_shot_8822b | odm_get_rf_reg | 20 | MASK20BITS | live | `odm_get_rf_reg(dm, (enum rf_path)path, RF_0x56,` |
| `hal/hal_mp.c:1818` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF | dead F:IS_HARDWARE_TYPE_8723B(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x56, 0xF, 0x1); /*/ RF LO enabled*/` |
| `hal/hal_mp.c:1944` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF | dead F:IS_HARDWARE_TYPE_8723B(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x56, 0xF, 0x0); /*/ RF LO disabled*/` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:625` | _iqk_lok_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | cfg-off-func | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:637` | _iqk_lok_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | cfg-off-func | `odm_set_rf_reg(dm, (enum rf_path)path,` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:4227`): 10 mục, giá trị 0x51000; điều kiện ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}, ELSE_IF {"cut": 3, "rfe": 4}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:8461`): 10 mục, giá trị 0x51000; điều kiện ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}, ELSE_IF {"cut": 3, "rfe": 4}

## `0x0057` — St T — `RF_0x57`

Định nghĩa: `RF_0x57 hal/phydm/phydm_regtable.h:895`

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:4228`): 11 mục, giá trị 0xce0a; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:8462`): 11 mục, giá trị 0xce0a; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}

## `0x0058` — St A — `RF_0x58`

Định nghĩa: `RF_0x58 hal/phydm/phydm_regtable.h:896`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:968` | mpt_SetSingleTone_8814A | phy_set_rf_reg | 20 | BIT1 | live | `phy_set_rf_reg(pAdapter, path, lna_low_gain_3, BIT1, 0x1); /*/ RF LO enabled*/` |
| `hal/hal_mp.c:1003` | mpt_SetSingleTone_8814A | phy_set_rf_reg | 20 | BIT1 | live | `phy_set_rf_reg(pAdapter, path, lna_low_gain_3, BIT1, 0x0); /* RF LO disabled */` |
| `hal/hal_mp.c:1861` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT1 | live | `phy_set_rf_reg(pAdapter, p, lna_low_gain_3, BIT1, 0x1); /*/ RF LO enabled*/` |
| `hal/hal_mp.c:1871` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT1 | live | `phy_set_rf_reg(pAdapter, pMptCtx->mpt_rf_path, lna_low_gain_3, BIT1, 0x1); /*/ RF LO enabled*/` |
| `hal/hal_mp.c:1979` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT1 | live | `phy_set_rf_reg(pAdapter, p, lna_low_gain_3, BIT1, 0x0); /*/ RF LO disabled*/` |
| `hal/hal_mp.c:1988` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT1 | live | `phy_set_rf_reg(pAdapter, p, lna_low_gain_3, BIT1, 0x0); /*/ RF LO disabled*/` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:411` | _iqk_backup_iqk_8822b | odm_get_rf_reg | 20 | MASK20BITS | live | `iqk->lok_idac[0][path] = odm_get_rf_reg(dm, (enum rf_path)path,` |
| `hal/hal_mp.c:1794` | hal_mpt_SetSingleToneTx | phy_query_rf_reg | 20 | bRFRegOffsetMask | dead F:IS_HARDWARE_TYPE_8188E(pAdapter) | `regRF = phy_query_rf_reg(pAdapter, rfPath, lna_low_gain_3, bRFRegOffsetMask);` |
| `hal/hal_mp.c:1795` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT1 | dead F:IS_HARDWARE_TYPE_8188E(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, lna_low_gain_3, BIT1, 0x1); /*/ RF LO enabled*/` |
| `hal/hal_mp.c:1801` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT1 | dead F:IS_HARDWARE_TYPE_8192E(pAdapter) | `phy_set_rf_reg(pAdapter, pMptCtx->mpt_rf_path, lna_low_gain_3, BIT1, 0x1); /*/ RF LO disabled*/` |
| `hal/hal_mp.c:1832` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT1 | dead F:IS_HARDWARE_TYPE_8188F(pAdapter) \|\| IS_HARDWARE_TYPE_8188GTV | `phy_set_rf_reg(pAdapter, pMptCtx->mpt_rf_path, lna_low_gain_3, BIT1, 0x1);` |
| `hal/hal_mp.c:1921` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | bRFRegOffsetMask | dead F:IS_HARDWARE_TYPE_8188E(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, lna_low_gain_3, bRFRegOffsetMask, regRF);` |
| `hal/hal_mp.c:1926` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT1 | dead F:IS_HARDWARE_TYPE_8192E(pAdapter) | `phy_set_rf_reg(pAdapter, pMptCtx->mpt_rf_path, lna_low_gain_3, BIT1, 0x0);/*/ RF LO disabled */` |
| `hal/hal_mp.c:1957` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT1 | dead F:IS_HARDWARE_TYPE_8188F(pAdapter) \|\| IS_HARDWARE_TYPE_8188GTV | `phy_set_rf_reg(pAdapter, pMptCtx->mpt_rf_path, lna_low_gain_3, BIT1, 0x0); /*RF LO disabled*/` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:442` | _iqk_reload_iqk_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | cfg-off-func | `odm_set_rf_reg(dm, (enum rf_path)path, RF_0x58,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:919` | _lok_one_shot_8822b | odm_get_rf_reg | 20 | MASK20BITS | cfg-off-func | `LOK_temp = odm_get_rf_reg(dm, (enum rf_path)path,` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:4229`): 11 mục, giá trị 0x58750, 0x82030; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:8463`): 11 mục, giá trị 0x58750, 0x82030; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}

## `0x0059` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:4732`): 1 mục, giá trị 0x20000; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:8890`): 1 mục, giá trị 0x20000; điều kiện None

## `0x0060` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_kfree.c:917` | phydm_get_pa_bias_offset_8198f | odm_set_rf_reg | 20 | 0x0000f000 | dead-func | `odm_set_rf_reg(dm, i, 0x60, 0x0000f000, tx_pa_bias[i]);` |
| `hal/phydm/halrf/halrf_kfree.c:1400` | phydm_get_set_pa_bias_offset_8822c | odm_set_rf_reg | 20 | 0x0000f000 | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, 0x60, 0x0000f000, pg_pa_bias);` |
| `hal/phydm/halrf/halrf_kfree.c:1409` | phydm_get_set_pa_bias_offset_8822c | odm_set_rf_reg | 20 | 0x0000f000 | dead-func | `odm_set_rf_reg(dm, RF_PATH_B, 0x60, 0x0000f000, pg_pa_bias);` |
| `hal/phydm/halrf/halrf_kfree.c:1418` | phydm_get_set_pa_bias_offset_8822c | odm_set_rf_reg | 20 | 0x000f0000 | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, 0x60, 0x000f0000, pg_pa_bias);` |
| `hal/phydm/halrf/halrf_kfree.c:1427` | phydm_get_set_pa_bias_offset_8822c | odm_set_rf_reg | 20 | 0x000f0000 | dead-func | `odm_set_rf_reg(dm, RF_PATH_B, 0x60, 0x000f0000, pg_pa_bias);` |
| `hal/phydm/halrf/halrf_kfree.c:1784` | phydm_get_set_pa_bias_offset_8812f | odm_set_rf_reg | 20 | 0x000f0000 | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, 0x60, 0x000f0000, pg_pa_bias);` |
| `hal/phydm/halrf/halrf_kfree.c:1793` | phydm_get_set_pa_bias_offset_8812f | odm_set_rf_reg | 20 | 0x000f0000 | dead-func | `odm_set_rf_reg(dm, RF_PATH_B, 0x60, 0x000f0000, pg_pa_bias);` |
| `hal/phydm/halrf/halrf_kfree.c:1946` | phydm_get_set_pa_bias_offset_8195b | odm_set_rf_reg | 20 | 0x0000f000 | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, 0x60, 0x0000f000, pg_pa_bias);` |
| `hal/phydm/halrf/halrf_kfree.c:1955` | phydm_get_set_pa_bias_offset_8195b | odm_set_rf_reg | 20 | 0x000f0000 | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, 0x60, 0x000f0000, pg_pa_bias);` |
| `hal/phydm/halrf/halrf_kfree.c:2253` | phydm_get_set_pa_bias_offset_8197g | odm_set_rf_reg | 20 | 0x0000f000 | dead-func | `odm_set_rf_reg(dm, i, 0x60, 0x0000f000, tx_pa_bias[i]);` |

## `0x0061` — St A — `RF_0x61`

Định nghĩa: `RF_0x61 hal/phydm/phydm_regtable.h:898`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:175` | _set_tx_a_cali_value | odm_get_rf_reg | 20 | 0xFFFFF | live | `modi_tx_a_value = odm_get_rf_reg(dm, rf_path, RF_0x61, 0xFFFFF);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:1027`): 27 mục, giá trị 0x5d2a1, 0x5d301, 0x5d3d0, 0x5d3d1, 0x5d4a0; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:5365`): 27 mục, giá trị 0x5d2a1, 0x5d3d0, 0x5d3d1, 0x5d4a0; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}

## `0x0062` — St T — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:1028`): 27 mục, giá trị 0xd203, 0xd303, 0xd3a2; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:5366`): 27 mục, giá trị 0xd203, 0xd303, 0xd3a2; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}

## `0x0063` — St T — `RF_0x63`

Định nghĩa: `RF_0x63 hal/phydm/phydm_regtable.h:899`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1843` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT0 | dead F:IS_HARDWARE_TYPE_8723D(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x63, BIT0, 0x1);` |
| `hal/hal_mp.c:1968` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT0 | dead F:IS_HARDWARE_TYPE_8723D(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x63, BIT0, 0x0);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:1029`): 27 mục, giá trị 0x2, 0x62; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:5367`): 27 mục, giá trị 0x2, 0x62; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}

## `0x0065` — St A — `RF_0x65`

Định nghĩa: `RF_0x65 hal/phydm/phydm_regtable.h:901`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_kfree.c:235` | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(6) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x65, BIT(6), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:244` | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x65, BIT(19), (btg & BIT(0)));` |
| `hal/phydm/halrf/halrf_kfree.c:245` | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | 20 | gain_bmask | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x65, gain_bmask, (btg >> 1));` |
| `hal/phydm/halrf/halrf_kfree.c:255` | phydm_set_kfree_to_rf_8821c | odm_get_rf_reg | 20 | s_gain_bmask | live | `odm_get_rf_reg(dm, e_rf_path, RF_0x65, s_gain_bmask));` |
| `hal/phydm/halrf/halrf_kfree.c:492` | phydm_set_kfree_to_rf_8822b | odm_set_rf_reg | 20 | MASKLWORD | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x65, MASKLWORD, 0x9000);` |
| `hal/phydm/halrf/halrf_kfree.c:512` | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | 20 | MASKLWORD | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x65, MASKLWORD, 0x9000);` |
| `hal/phydm/halrf/halrf_kfree.c:521` | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | 20 | MASKLWORD | live | `odm_set_rf_reg(dm, e_rf_path, RF_0x65, MASKLWORD, 0x9000);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:354` | _iqk_restore_rf_8822b | odm_set_rf_reg (qua mảng backup_rf_reg) | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_A, backup_rf_reg[i], MASK20BITS,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:356` | _iqk_restore_rf_8822b | odm_set_rf_reg (qua mảng backup_rf_reg) | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_B, backup_rf_reg[i], MASK20BITS,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:561` | _iqk_rf_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path, RF_0x65,` |
| `core/rtw_rf.c:1314` | rtw_rf_set_tx_gain_offset | rtw_hal_read_rfreg | 20 | 0xffffffff | dead F:IS_HARDWARE_TYPE_8723D(adapter) | `rtw_hal_read_rfreg(adapter, target_path, 0x65, 0xffffffff));` |
| `core/rtw_rf.c:1380` | rtw_rf_set_tx_gain_offset | rtw_hal_read_rfreg | 20 | 0xffffffff | dead F:IS_HARDWARE_TYPE_8723D(adapter) | `val32 = rtw_hal_read_rfreg(adapter, target_path, 0x65, 0xffffffff);` |
| `hal/phydm/halrf/halrf_kfree.c:268` | phydm_clear_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(6) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x65, BIT(6), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:273` | phydm_clear_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x65, BIT(19), (data & BIT(0)));` |
| `hal/phydm/halrf/halrf_kfree.c:274` | phydm_clear_kfree_to_rf_8821c | odm_set_rf_reg | 20 | gain_bmask | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x65, gain_bmask, (data >> 1));` |
| `hal/phydm/halrf/halrf_kfree.c:279` | phydm_clear_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(6) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0x65, BIT(6), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:284` | phydm_clear_kfree_to_rf_8821c | odm_get_rf_reg | 20 | s_gain_bmask | dead-func | `odm_get_rf_reg(dm, e_rf_path, RF_0x65, s_gain_bmask));` |

## `0x0075` — St X — `RF_0x75`

Định nghĩa: `RF_0x75 hal/phydm/phydm_regtable.h:906`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1986` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | BIT16 | dead F:IS_HARDWARE_TYPE_8821C(pAdapter) | `phy_set_rf_reg(pAdapter, p, 0x75, BIT16, 0x0); /* RF LO (for BTG) disabled */` |

## `0x0076` — St X — `RF_0x76`

Định nghĩa: `RF_0x76 hal/phydm/phydm_regtable.h:907`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1822` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF | dead F:IS_HARDWARE_TYPE_8723B(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x76, 0xF, 0x1); /*/ RF LO enabled*/` |
| `hal/hal_mp.c:1948` | hal_mpt_SetSingleToneTx | phy_set_rf_reg | 20 | 0xF | dead F:IS_HARDWARE_TYPE_8723B(pAdapter) | `phy_set_rf_reg(pAdapter, RF_PATH_A, 0x76, 0xF, 0x0); /*/ RF LO disabled*/` |

## `0x007F` — St A — `RF_0x7f`

Định nghĩa: `RF_0x7f hal/phydm/phydm_regtable.h:910`

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:235`): 1 mục, giá trị 0x0; điều kiện None

## `0x0081` — St T — `RF_0x81`

Định nghĩa: `RF_0x81 hal/phydm/phydm_regtable.h:913`

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:4221`): 11 mục, giá trị 0xf000, 0xf400; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:8455`): 11 mục, giá trị 0xf000, 0xf400; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}

## `0x0084` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:929`): 1 mục, giá trị 0x1209; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:5278`): 1 mục, giá trị 0x1209; điều kiện None

## `0x0086` — St A — `RF_0x86`

Định nghĩa: `RF_0x86 hal/phydm/phydm_regtable.h:917`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_kfree.c:964` | phydm_get_set_lna_offset_8198f | odm_get_rf_reg | 20 | (BIT(19) \| BIT(18)) | dead-func | `rf_reg = odm_get_rf_reg(dm, i, 0x86, (BIT(19) \| BIT(18)));` |
| `hal/phydm/halrf/halrf_kfree.c:968` | phydm_get_set_lna_offset_8198f | odm_set_rf_reg | 20 | (BIT(19) \| BIT(18)) | dead-func | `odm_set_rf_reg(dm, i, 0x86, (BIT(19) \| BIT(18)), rf_reg);` |
| `hal/phydm/halrf/halrf_kfree.c:973` | phydm_get_set_lna_offset_8198f | odm_get_rf_reg | 20 | (BIT(17) \| BIT(16)) | dead-func | `rf_reg = odm_get_rf_reg(dm, i, 0x86, (BIT(17) \| BIT(16)));` |
| `hal/phydm/halrf/halrf_kfree.c:977` | phydm_get_set_lna_offset_8198f | odm_set_rf_reg | 20 | (BIT(17) \| BIT(16)) | dead-func | `odm_set_rf_reg(dm, i, 0x86, (BIT(17) \| BIT(16)), rf_reg);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:930`): 1 mục, giá trị 0x1a0; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:5279`): 1 mục, giá trị 0x1a0; điều kiện None

## `0x0087` — St T — `RF_0x87`

Định nghĩa: `RF_0x87 hal/phydm/phydm_regtable.h:918`

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:932`): 22 mục, giá trị 0x16040, 0x68080, 0xe8180; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:5281`): 22 mục, giá trị 0x16040, 0x68080, 0xe8180; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}

## `0x0088` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_kfree.c:2281` | phydm_get_set_lna_offset_8197g | odm_set_rf_reg | 20 | 0x00000f00 | no-caller | `odm_set_rf_reg(dm, i, 0x88, 0x00000f00, pg_lna[i]);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:954`): 1 mục, giá trị 0x70020; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:5303`): 1 mục, giá trị 0x70020; điều kiện None

## `0x0089` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:362`): 1 mục, giá trị 0x207; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4887`): 1 mục, giá trị 0x207; điều kiện None

## `0x008A` — St T — `RF_0x8a`

Định nghĩa: `RF_0x8a hal/phydm/phydm_regtable.h:919`

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:364`): 14 mục, giá trị 0xfe186, 0xff186; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4889`): 9 mục, giá trị 0xfe186, 0xff186; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 18}

## `0x008B` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:392`): 1 mục, giá trị 0x61e3c; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4907`): 1 mục, giá trị 0x61e3c; điều kiện None

## `0x008C` — St A — `RF_0x8c`

Định nghĩa: `RF_0x8c hal/phydm/phydm_regtable.h:920`

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:393`): 1 mục, giá trị 0x112c7; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4908`): 1 mục, giá trị 0x112c7; điều kiện None

## `0x008D` — St A — `RF_0x8d`

Định nghĩa: `RF_0x8d hal/phydm/phydm_regtable.h:921`

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:394`): 1 mục, giá trị 0xf4988; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4909`): 1 mục, giá trị 0xf4988; điều kiện None

## `0x008E` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:395`): 1 mục, giá trị 0x64d40; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4910`): 1 mục, giá trị 0x64d40; điều kiện None

## `0x008F` — St A — `RF_0x8f`

Định nghĩa: `RF_0x8f hal/phydm/phydm_regtable.h:922`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:354` | _iqk_restore_rf_8822b | odm_set_rf_reg (qua mảng backup_rf_reg) | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_A, backup_rf_reg[i], MASK20BITS,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:356` | _iqk_restore_rf_8822b | odm_set_rf_reg (qua mảng backup_rf_reg) | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_B, backup_rf_reg[i], MASK20BITS,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:668` | _iqk_txk_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:675` | _iqk_txk_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:697` | _iqk_rxk1_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:707` | _iqk_rxk1_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:732` | _iqk_rxk2_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:748` | _iqk_rxk2_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:627` | _iqk_lok_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | cfg-off-func | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:639` | _iqk_lok_setting_8822b | odm_set_rf_reg | 20 | MASK20BITS | cfg-off-func | `odm_set_rf_reg(dm, (enum rf_path)path,` |

## `0x00B0` — St A — `RF_0xb0`

Định nghĩa: `RF_0xb0 hal/phydm/phydm_regtable.h:926`

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:237`): 30 mục, giá trị 0xfb0f8, 0xff0f8, 0xff0fa, 0xff0fc; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}

## `0x00B1` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:291`): 1 mục, giá trị 0x7dbe4; điều kiện None

## `0x00B2` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:467` | aac_check_8822b | odm_set_rf_reg | 20 | 0x7c000 | live | `odm_set_rf_reg(dm, RF_PATH_A, 0xb2, 0x7c000, 0x6);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:293`): 2 mục, giá trị 0x215d1, 0x225d1; điều kiện ELSE, IF {"cut": 3, "rfe": 12}

## `0x00B3` — St A — `RF_0xb3`

Định nghĩa: `RF_0xb3 hal/phydm/phydm_regtable.h:927`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1666` | config_phydm_switch_band_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xb3, RFREGOFFSETMASK,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1668` | config_phydm_switch_band_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0xb3, RFREGOFFSETMASK,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1731` | config_phydm_switch_band_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xb3, RFREGOFFSETMASK,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1733` | config_phydm_switch_band_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0xb3, RFREGOFFSETMASK,` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:298`): 15 mục, giá trị 0x3c360, 0x7c330, 0xfc760; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 11}, ELSE_IF {"cut": 3, "rfe": 12}

## `0x00B4` — St A — `RF_0xb4`

Định nghĩa: `RF_0xb4 hal/phydm/phydm_regtable.h:928`

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:328`): 1 mục, giá trị 0x99dd0; điều kiện None

## `0x00B5` — St T — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:330`): 2 mục, giá trị 0x100fc, 0x400fc; điều kiện ELSE, IF {"cut": 3, "rfe": 12}

## `0x00B6` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:334`): 1 mục, giá trị 0x187f0; điều kiện None

## `0x00B7` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:335`): 1 mục, giá trị 0x30018; điều kiện None

## `0x00B8` — St A — `RF_0xb8`

Định nghĩa: `RF_0xb8 hal/phydm/phydm_regtable.h:929`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2013` | config_phydm_switch_channel_8822b | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xb8, BIT(19), 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2014` | config_phydm_switch_channel_8822b | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xb8, BIT(19), 1);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:134` | _iqk_rf0xb0_workaround_8822b | odm_set_rf_reg | 20 | MASK20BITS | cfg-off-func | `odm_set_rf_reg(dm, (enum rf_path)0x0, RF_0xb8, MASK20BITS, 0x00a00);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:135` | _iqk_rf0xb0_workaround_8822b | odm_set_rf_reg | 20 | MASK20BITS | cfg-off-func | `odm_set_rf_reg(dm, (enum rf_path)0x0, RF_0xb8, MASK20BITS, 0x80a00);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:336`): 2 mục, giá trị 0x80800, 0x80a00; điều kiện None

## `0x00B9` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:337`): 1 mục, giá trị 0x0; điều kiện None

## `0x00BA` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:338`): 1 mục, giá trị 0x8000; điều kiện None

## `0x00BB` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:339`): 1 mục, giá trị 0x0; điều kiện None

## `0x00BC` — St A — `RF_0xbc`

Định nghĩa: `RF_0xbc hal/phydm/phydm_regtable.h:930`

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:340`): 1 mục, giá trị 0x40030; điều kiện None

## `0x00BD` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:341`): 1 mục, giá trị 0x0; điều kiện None

## `0x00BE` — St A — `RF_0xbe`

Định nghĩa: `RF_0xbe hal/phydm/phydm_regtable.h:931`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1977` | config_phydm_switch_channel_8822b | odm_set_rf_reg | 20 | (BIT(17) \| BIT(16) \| B | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xbe, (BIT(17) \| BIT(16) \|` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:342`): 1 mục, giá trị 0x0; điều kiện None

## `0x00BF` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:343`): 1 mục, giá trị 0x0; điều kiện None

## `0x00C0` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:344`): 1 mục, giá trị 0x0; điều kiện None

## `0x00C1` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:345`): 1 mục, giá trị 0x0; điều kiện None

## `0x00C2` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:346`): 1 mục, giá trị 0x0; điều kiện None

## `0x00C3` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:347`): 1 mục, giá trị 0x0; điều kiện None

## `0x00C4` — St A — `RF_0xc4`

Định nghĩa: `RF_0xc4 hal/phydm/phydm_regtable.h:932`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:488` | _phy_lc_calibrate_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xc4, RFREGOFFSETMASK, 0x01402);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:501` | _phy_lc_calibrate_8822b | odm_set_rf_reg | 20 | RFREGOFFSETMASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xc4, RFREGOFFSETMASK, 0x81402);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:348`): 2 mục, giá trị 0x2402, 0x81402; điều kiện None

## `0x00C5` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:349`): 1 mục, giá trị 0x9; điều kiện None

## `0x00C6` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:350`): 1 mục, giá trị 0x40299; điều kiện None

## `0x00C7` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:351`): 1 mục, giá trị 0x55555; điều kiện None

## `0x00C8` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:352`): 1 mục, giá trị 0xc16c; điều kiện None

## `0x00C9` — St A — `RF_0xc9`

Định nghĩa: `RF_0xc9 hal/phydm/phydm_regtable.h:933`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:464` | aac_check_8822b | odm_get_rf_reg | 20 | 0xf8 | live | `temp = odm_get_rf_reg(dm, RF_PATH_A, 0xc9, 0xf8);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:353`): 2 mục, giá trị 0x1c141, 0x1c146; điều kiện None

## `0x00CA` — St A — `RF_0xca`

Định nghĩa: `RF_0xca hal/phydm/phydm_regtable.h:934`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:466` | aac_check_8822b | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, RF_PATH_A, 0xca, BIT(19), 0x0);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:354`): 2 mục, giá trị 0x0, 0x80000; điều kiện None

## `0x00CB` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:355`): 1 mục, giá trị 0x0; điều kiện None

## `0x00CC` — St A — `RF_0xcc`

Định nghĩa: `RF_0xcc hal/phydm/phydm_regtable.h:935`

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:356`): 2 mục, giá trị 0x0, 0x82000; điều kiện None

## `0x00CD` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:357`): 1 mục, giá trị 0x0; điều kiện None

## `0x00CE` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:358`): 9 mục, giá trị 0x90c00, 0x94400, 0x94c00; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 16}, ELSE_IF {"cut": 3, "rfe": 17}

## `0x00CF` — St A — *(không có macro)*

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:359`): 10 mục, giá trị 0x64700, 0x6d200, 0x72f00; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 12}, ELSE_IF {"cut": 3, "rfe": 16}

## `0x00DE` — St A — `RF_0xde`

Định nghĩa: `RF_0xde hal/phydm/phydm_regtable.h:938`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_kfree.c:232` | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(0) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0xde, BIT(0), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:233` | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(5) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0xde, BIT(5), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:490` | phydm_set_kfree_to_rf_8822b | odm_set_rf_reg | 20 | BIT(0) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0xde, BIT(0), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:491` | phydm_set_kfree_to_rf_8822b | odm_set_rf_reg | 20 | BIT(4) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0xde, BIT(4), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:510` | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | 20 | BIT(0) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0xde, BIT(0), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:511` | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | 20 | BIT(4) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0xde, BIT(4), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:519` | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | 20 | BIT(0) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0xde, BIT(0), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:520` | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | 20 | BIT(4) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0xde, BIT(4), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:265` | phydm_clear_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(0) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xde, BIT(0), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:266` | phydm_clear_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(5) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xde, BIT(5), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:276` | phydm_clear_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(0) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xde, BIT(0), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:277` | phydm_clear_kfree_to_rf_8821c | odm_set_rf_reg | 20 | BIT(5) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xde, BIT(5), 0);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:955`): 1 mục, giá trị 0x10; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:5304`): 1 mục, giá trị 0x10; điều kiện None

## `0x00DF` — St A — `RF_0xdf`

Định nghĩa: `RF_0xdf hal/phydm/phydm_regtable.h:939`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:354` | _iqk_restore_rf_8822b | odm_set_rf_reg (qua mảng backup_rf_reg) | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_A, backup_rf_reg[i], MASK20BITS,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:356` | _iqk_restore_rf_8822b | odm_set_rf_reg (qua mảng backup_rf_reg) | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_B, backup_rf_reg[i], MASK20BITS,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:552` | _iqk_rf_setting_8822b | odm_get_rf_reg | 20 | MASK20BITS | live | `tmp = odm_get_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1989` | config_phydm_switch_channel_8822b | odm_set_rf_reg | 20 | BIT(18) | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xdf, BIT(18), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1992` | config_phydm_switch_channel_8822b | odm_set_rf_reg | 20 | BIT(18) | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xdf, BIT(18), 0x0);` |
| `hal/phydm/halrf/halrf_kfree.c:722` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | BIT(7) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xdf, BIT(7), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:770` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | BIT(7) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xdf, BIT(7), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:961` | phydm_get_set_lna_offset_8198f | odm_set_rf_reg | 20 | RFREGOFFSETMASK | dead-func | `odm_set_rf_reg(dm, i, 0xdf, RFREGOFFSETMASK, 0x2);` |
| `hal/phydm/halrf/halrf_kfree.c:1005` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | BIT(7) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xdf, BIT(7), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:1069` | phydm_clear_kfree_to_rf_8198f | odm_set_rf_reg | 20 | BIT(7) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xdf, BIT(7), 1);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:438` | _iqk_reload_iqk_setting_8822b | odm_get_rf_reg | 20 | MASK20BITS | cfg-off-func | `tmp = odm_get_rf_reg(dm, (enum rf_path)path,` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:220`): 2 mục, giá trị 0x2449, 0x9; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4870`): 2 mục, giá trị 0x2449, 0x9; điều kiện None

## `0x00EE` — St A — `RF_0xee`

Định nghĩa: `RF_0xee hal/phydm/phydm_regtable.h:941`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_kfree.c:1124` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xee, BIT(19), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:1172` | phydm_set_power_trim_offset_8822c | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xee, BIT(19), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:1484` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xee, BIT(19), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:1534` | phydm_set_power_trim_offset_8812f | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xee, BIT(19), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:1839` | phydm_set_power_trim_rf_8195b | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xee, BIT(19), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:1869` | phydm_set_power_trim_rf_8195b | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xee, BIT(19), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:1997` | phydm_set_power_trim_rf_8721d | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xee, BIT(19), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:2025` | phydm_set_power_trim_rf_8721d | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xee, BIT(19), 0);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:1008`): 2 mục, giá trị 0x0, 0x2; điều kiện None
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:5346`): 2 mục, giá trị 0x0, 0x2; điều kiện None

## `0x00EF` — St A — `RF_0xef`, `rf_welut_jaguar`

Định nghĩa: `rf_welut_jaguar include/hal_phy_reg.h:305`; `RF_0xef hal/phydm/phydm_regtable.h:942`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_kfree.c:422` | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | 20 | BIT(10) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0xef, BIT(10), 0x1);` |
| `hal/phydm/halrf/halrf_kfree.c:431` | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | 20 | BIT(10) | live | `odm_set_rf_reg(dm, e_rf_path, RF_0xef, BIT(10), 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:340` | _iqk_restore_rf_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xef, MASK20BITS, 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:341` | _iqk_restore_rf_8822b | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, RF_PATH_B, RF_0xef, MASK20BITS, 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:565` | _iqk_rf_setting_8822b | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:573` | _iqk_rf_setting_8822b | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:576` | _iqk_rf_setting_8822b | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:584` | _iqk_rf_setting_8822b | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:762` | halrf_iqk_set_rf0x8 | odm_set_rf_reg | 20 | MASK20BITS | live | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/phydm_direct_bf.c:76` | phydm_iq_gen_en | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, path, RF_0xef, BIT(19), 0x1);` |
| `hal/phydm/phydm_direct_bf.c:84` | phydm_iq_gen_en | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, path, RF_0xef, BIT(19), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2538` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xef, RFREG_MASK, 0x80000);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2559` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xef, RFREG_MASK, 0x80000);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2563` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xef, RFREG_MASK, 0x00000);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2569` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xef, RFREG_MASK, 0x80000);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2573` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xef, RFREG_MASK, 0x00000);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2578` | config_phydm_trx_mode_8822b | odm_set_rf_reg | 20 | RFREG_MASK | live | `odm_set_rf_reg(dm, RF_PATH_A, RF_0xef, RFREG_MASK, 0x00000);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:249` | _txa_bias_cali_4_each_path | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0xef, 0xFFFFF, 0x200);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:266` | _txa_bias_cali_4_each_path | odm_set_rf_reg | 20 | 0xFFFFF | live | `odm_set_rf_reg(dm, rf_path, RF_0xef, 0xFFFFF, 0x0);` |
| `hal/phydm/txbf/haltxbf8822b.c:1065` | phydm_8822btxbf_rfmode | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, (enum rf_path)i, RF_0xef, BIT(19), 0x1); /*RF mode table write enable*/` |
| `hal/phydm/txbf/haltxbf8822b.c:1069` | phydm_8822btxbf_rfmode | odm_set_rf_reg | 20 | BIT(19) | live | `odm_set_rf_reg(dm, (enum rf_path)i, RF_0xef, BIT(19), 0x0); /*RF mode table write disable*/` |
| `hal/phydm/halrf/halrf_kfree.c:724` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | BIT(7) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xef, BIT(7), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:772` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | BIT(7) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xef, BIT(7), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:1007` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | BIT(7) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xef, BIT(7), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:1028` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | BIT(7) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xef, BIT(7), 0);` |
| `hal/phydm/halrf/halrf_kfree.c:2164` | phydm_set_power_trim_offset_8197g | odm_set_rf_reg | 20 | BIT(7) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xef, BIT(7), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:2185` | phydm_set_power_trim_offset_8197g | odm_set_rf_reg | 20 | BIT(7) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xef, BIT(7), 0);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:79` | halrf_rf_lna_setting_8822b | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, (enum rf_path)path, RF_0xef, BIT(19),` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:87` | halrf_rf_lna_setting_8822b | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, (enum rf_path)path, RF_0xef, BIT(19),` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:91` | halrf_rf_lna_setting_8822b | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, (enum rf_path)path, RF_0xef, BIT(19),` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:99` | halrf_rf_lna_setting_8822b | odm_set_rf_reg | 20 | BIT(19) | dead-func | `odm_set_rf_reg(dm, (enum rf_path)path, RF_0xef, BIT(19),` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:630` | _iqk_lok_setting_8822b | odm_set_rf_reg | 20 | BIT(4) | cfg-off-func | `odm_set_rf_reg(dm, (enum rf_path)path,` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:642` | _iqk_lok_setting_8822b | odm_set_rf_reg | 20 | BIT(4) | cfg-off-func | `odm_set_rf_reg(dm, (enum rf_path)path, RF_0xef, BIT(4), 0x1);` |

- Bảng `array_mp_8822b_radioa` (`halhwimg8822b_rf.c:208`): 155 mục, giá trị 0x0, 0x10, 0x100, 0x200, 0x20000, 0x4, 0x40, 0x400 …; điều kiện ELSE, ELSE_IF {"cut": 3, "rfe": 10}, ELSE_IF {"cut": 3, "rfe": 11}
- Bảng `array_mp_8822b_radiob` (`halhwimg8822b_rf.c:4858`): 34 mục, giá trị 0x0, 0x10, 0x100, 0x200, 0x20000, 0x40, 0x400, 0x4000 …; điều kiện ELSE, IF {"cut": 3, "rfe": 12}, None

## `0x00F0` — St A — `RF_0x1bf0`

Định nghĩa: `RF_0x1bf0 hal/phydm/phydm_regtable.h:876`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:164` | _iqk_fail_count_8822b | odm_get_rf_reg | 20 | BIT(16) | live | `if (odm_get_rf_reg(dm, RF_PATH_A, RF_0x1bf0, BIT(16)) == 1)` |

## `0x00F5` — St X — `RF_0xf5`

Định nghĩa: `RF_0xf5 hal/phydm/phydm_regtable.h:943`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halrf_kfree.c:720` | phydm_set_kfree_to_rf_8192f | odm_set_rf_reg | 20 | BIT(18) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xf5, BIT(18), 1);` |
| `hal/phydm/halrf/halrf_kfree.c:1003` | phydm_set_kfree_to_rf_8198f | odm_set_rf_reg | 20 | BIT(18) | dead-func | `odm_set_rf_reg(dm, e_rf_path, RF_0xf5, BIT(18), 1);` |

