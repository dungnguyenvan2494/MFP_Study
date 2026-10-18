# Phụ lục A07 — BB 0x0800–0x0FFF

Sinh tự động bởi `tools/appendix.py` từ `work/*.json` [TOOL]. Mỗi mục liệt kê **mọi** truy cập tìm thấy trong code active (kể cả nhánh chết, có ghi lý do) và mọi mục bảng. Các thanh ghi St=D/Z (không dùng) chỉ có trong `04*_catalog_*.md`. Cột "Trạng thái": `live` = chạy được với RTL8822BU; còn lại là lý do loại.

Tổng: 389 địa chỉ.

## `0x0800` — St A — `R_0x800`, `rFPGA0_RFMOD`, `REG_FPGA0_RFMOD`, `ODM_FPGA_PHY0_PAGE8`, `ODM_REG_BB_CTRL_11N`

Định nghĩa: `REG_FPGA0_RFMOD include/hal_phy_reg.h:143`; `R_0x800 hal/phydm/phydm_regtable.h:479`; `ODM_REG_BB_CTRL_11N hal/phydm/phydm_regdefine11n.h:81`; `ODM_FPGA_PHY0_PAGE8 hal/phydm/phydm_reg.h:93`; `rFPGA0_RFMOD include/rtl8822b_hal.h:116`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_mp.c:2224` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_800 = rtw_read32(padapter, 0x800);` |
| `core/rtw_mp.c:2225` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x1000000 | live | `phy_set_bb_reg(padapter, 0x800, 0x1000000, 0x0);` |
| `core/rtw_mp.c:2245` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_800 = rtw_read32(padapter, 0x800);` |
| `core/rtw_mp.c:2246` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x1000000 | live | `phy_set_bb_reg(padapter, 0x800, 0x1000000, 0x0);` |
| `core/rtw_mp.c:2315` | rtw_mp_psd_close | rtw_write32 | 32 |  | live | `rtw_write32(padapter, 0x800, regs->reg_800);` |
| `hal/hal_mp.c:1721` | hal_mpt_SetSingleCarrierTx | phy_query_bb_reg | 32 | bOFDMEn | live | `if (!phy_query_bb_reg(pAdapter, rFPGA0_RFMOD, bOFDMEn))` |
| `hal/hal_mp.c:1722` | hal_mpt_SetSingleCarrierTx | phy_set_bb_reg | 32 | bOFDMEn | live | `phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bOFDMEn, 1); /*set OFDM block on*/` |
| `hal/hal_mp.c:2036` | hal_mpt_SetCarrierSuppressionTx | read_bbreg | 32 | bCCKEn | live | `if (!read_bbreg(pAdapter, rFPGA0_RFMOD, bCCKEn))` |
| `hal/hal_mp.c:2037` | hal_mpt_SetCarrierSuppressionTx | write_bbreg | 32 | bCCKEn | live | `write_bbreg(pAdapter, rFPGA0_RFMOD, bCCKEn, bEnable);/*set CCK block on*/` |
| `hal/hal_mp.c:2187` | mpt_StartCckContTx | phy_query_bb_reg | 32 | bCCKEn | live | `if (!phy_query_bb_reg(pAdapter, rFPGA0_RFMOD, bCCKEn))` |
| `hal/hal_mp.c:2188` | mpt_StartCckContTx | phy_set_bb_reg | 32 | bCCKEn | live | `phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bCCKEn, 1);/*set CCK block on*/` |
| `hal/hal_mp.c:2230` | mpt_StartOfdmContTx | phy_query_bb_reg | 32 | bOFDMEn | live | `if (!phy_query_bb_reg(pAdapter, rFPGA0_RFMOD, bOFDMEn))` |
| `hal/hal_mp.c:2231` | mpt_StartOfdmContTx | phy_set_bb_reg | 32 | bOFDMEn | live | `phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bOFDMEn, 1);/*set OFDM block on*/` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2455` | phydm_config_rx_path_8822b | odm_set_bb_reg | 32 | BIT(28) | live | `odm_set_bb_reg(dm, R_0x800, BIT(28), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2462` | phydm_config_rx_path_8822b | odm_set_bb_reg | 32 | BIT(28) | live | `odm_set_bb_reg(dm, R_0x800, BIT(28), 0x1);` |
| `hal/hal_mp.c:1796` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bCCKEn | dead F:IS_HARDWARE_TYPE_8188E(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bCCKEn, 0x0);` |
| `hal/hal_mp.c:1797` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bOFDMEn | dead F:IS_HARDWARE_TYPE_8188E(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bOFDMEn, 0x0);` |
| `hal/hal_mp.c:1837` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bCCKEn\|bOFDMEn | dead F:IS_HARDWARE_TYPE_8723D(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bCCKEn\|bOFDMEn, 0);` |
| `hal/hal_mp.c:1841` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bCCKEn\|bOFDMEn | dead F:IS_HARDWARE_TYPE_8723D(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bCCKEn\|bOFDMEn, 0);` |
| `hal/hal_mp.c:1922` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bCCKEn | dead F:IS_HARDWARE_TYPE_8188E(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bCCKEn, 0x1);` |
| `hal/hal_mp.c:1923` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bOFDMEn | dead F:IS_HARDWARE_TYPE_8188E(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bOFDMEn, 0x1);` |
| `hal/hal_mp.c:1962` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bCCKEn\|bOFDMEn | dead F:IS_HARDWARE_TYPE_8723D(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bCCKEn\|bOFDMEn, 0x3);` |
| `hal/hal_mp.c:1966` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bCCKEn\|bOFDMEn | dead F:IS_HARDWARE_TYPE_8723D(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bCCKEn\|bOFDMEn, 0x3);` |
| `hal/phydm/phydm_adaptivity.c:554` | phydm_edcca_thre_calc | odm_set_bb_reg | 32 | BIT(10) | dead F:dm->support_ic_type & ODM_IC_PWDB_EDCCA | `odm_set_bb_reg(dm, R_0x800, BIT(10), 1);` |
| `hal/phydm/phydm_adaptivity.c:556` | phydm_edcca_thre_calc | odm_set_bb_reg | 32 | BIT(10) | dead F:dm->support_ic_type & ODM_IC_PWDB_EDCCA | `odm_set_bb_reg(dm, R_0x800, BIT(10), 0);` |
| `hal/phydm/phydm_api.c:741` | phydm_dis_cck_trx | odm_set_bb_reg | 32 | BIT(24) | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x800, BIT(24), 0);` |
| `hal/phydm/phydm_api.c:760` | phydm_dis_cck_trx | odm_set_bb_reg | 32 | BIT(24) | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x800, BIT(24), 1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10223`): 1 mục, giá trị 0x9020d010; điều kiện None

## `0x0804` — St A — `R_0x804`, `rFPGA0_TxInfo`, `ODM_REG_CCK_RPT_FORMAT_11AC`

Định nghĩa: `R_0x804 hal/phydm/phydm_regtable.h:481`; `ODM_REG_CCK_RPT_FORMAT_11AC hal/phydm/phydm_regdefine11ac.h:67`; `rFPGA0_TxInfo include/rtl8822b_hal.h:117`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1476` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | 0x0000000f | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_TxInfo, 0x0000000f, r_ofdm_tx_en_val);		/*/OFDM Tx*/` |
| `hal/phydm/phydm_psd.c:374` | phydm_psd_para_setting | odm_set_bb_reg | 32 | BIT(5) \| BIT(4) | dead F:dm->support_ic_type & (ODM_RTL8721D \| ODM_RTL8710C) | `odm_set_bb_reg(dm, 0x804, BIT(5) \| BIT(4), ant_sel);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10224`): 1 mục, giá trị 0x800181a0; điều kiện None

## `0x0808` — St A — `R_0x808`, `ODM_PSDREG`, `ODM_PSD_SETTING`, `rOFDMCCKEN_Jaguar`, `ODM_REG_PSD_CTRL_11N`, `ODM_REG_BB_RX_PATH_11AC`

Định nghĩa: `R_0x808 hal/phydm/phydm_regtable.h:482`; `ODM_REG_BB_RX_PATH_11AC hal/phydm/phydm_regdefine11ac.h:68`; `ODM_REG_PSD_CTRL_11N hal/phydm/phydm_regdefine11n.h:83`; `ODM_PSD_SETTING hal/phydm/phydm_reg.h:94`; `ODM_PSDREG hal/phydm/phydm_reg.h:157`; `rOFDMCCKEN_Jaguar include/rtl8822b_hal.h:118`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_mp.c:2217` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_808 = rtw_read32(padapter, 0x808);` |
| `core/rtw_mp.c:2218` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x10000000 | live | `phy_set_bb_reg(padapter, 0x808, 0x10000000, 0x0);` |
| `core/rtw_mp.c:2238` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_808 = rtw_read32(padapter, 0x808);` |
| `core/rtw_mp.c:2239` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x10000000 | live | `phy_set_bb_reg(padapter, 0x808, 0x10000000, 0x0);` |
| `core/rtw_mp.c:2261` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_808 = rtw_read32(padapter, 0x808);` |
| `core/rtw_mp.c:2262` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x10000000 | live | `phy_set_bb_reg(padapter, 0x808, 0x10000000, 0x0);` |
| `core/rtw_mp.c:2283` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_808 = rtw_read32(padapter, 0x808);` |
| `core/rtw_mp.c:2284` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x10000000 | live | `phy_set_bb_reg(padapter, 0x808, 0x10000000, 0x0);` |
| `core/rtw_mp.c:2300` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0xC000 | live | `phy_set_bb_reg(padapter, 0x808, 0xC000, 3);` |
| `core/rtw_mp.c:2350` | rtw_mp_psd_close | rtw_write32 | 32 |  | live | `rtw_write32(padapter, 0x808, regs->reg_808);` |
| `hal/hal_mp.c:1855` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | BIT29 \| BIT28 | live | `phy_set_bb_reg(pAdapter, rOFDMCCKEN_Jaguar, BIT29 \| BIT28, 0x0); /*/ Disable CCK and OFDM*/` |
| `hal/hal_mp.c:1974` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | BIT29 \| BIT28 | live | `phy_set_bb_reg(pAdapter, rOFDMCCKEN_Jaguar, BIT29 \| BIT28, 0x3); /*/ Disable CCK and OFDM*/` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:272` | _iqk_bb_reset_8822b | odm_write_1byte | 8 |  | live | `odm_write_1byte(dm, 0x808, 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:608` | _iqk_configure_macbb_8822b | odm_set_bb_reg | 32 | BIT(28) | live | `odm_set_bb_reg(dm, R_0x808, BIT(28), 0x0);` |
| `hal/phydm/phydm_api.c:734` | phydm_dis_cck_trx | odm_set_bb_reg | 32 | BIT(28) | live | `odm_set_bb_reg(dm, R_0x808, BIT(28), 0);` |
| `hal/phydm/phydm_api.c:754` | phydm_dis_cck_trx | odm_set_bb_reg | 32 | BIT(28) | live | `odm_set_bb_reg(dm, R_0x808, BIT(28), 1);` |
| `hal/phydm/phydm_dig.c:1541` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | BIT(28) | live | `cck_enable = odm_get_bb_reg(dm, ODM_REG_BB_RX_PATH_11AC, BIT(28));` |
| `hal/phydm/phydm_direct_bf.c:144` | phydm_dis_cdd | odm_set_bb_reg | 32 | 0x3ffff00 | live | `odm_set_bb_reg(dm, R_0x808, 0x3ffff00, 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1458` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0x808, MASKBYTE0,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1480` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0x808, MASKBYTE0,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1508` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0x808, MASKBYTE0,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1611` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(28) | live | `odm_set_bb_reg(dm, R_0x808, BIT(28), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1682` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(28) | live | `odm_set_bb_reg(dm, R_0x808, BIT(28), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2297` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0x808, MASKBYTE0, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2298` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0x808, MASKBYTE0, (dm->rx_ant_status \|` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2449` | phydm_config_rx_path_8822b | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0x808, MASKBYTE0, (rx_path << 4) \| rx_path);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2600` | config_phydm_parameter_init_8822b | odm_set_bb_reg | 32 | (BIT(28) \| BIT(29)) | live | `odm_set_bb_reg(dm, R_0x808, (BIT(28) \| BIT(29)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2604` | config_phydm_parameter_init_8822b | odm_set_bb_reg | 32 | (BIT(28) \| BIT(29)) | live | `odm_set_bb_reg(dm, R_0x808, (BIT(28) \| BIT(29)), 0x3);` |
| `hal/phydm/phydm_api.c:163` | phydm_trx_antenna_setting_init | odm_get_bb_reg | 32 | 0xf | dead F:dm->support_ic_type & (ODM_RTL8812 \| ODM_RTL8814A) | `dm->rx_ant_status = (u8)odm_get_bb_reg(dm, R_0x808, 0xf);` |
| `hal/phydm/phydm_api.c:243` | phydm_config_ofdm_rx_path | odm_set_bb_reg | 32 | MASKBYTE0 | no-caller T:dm->support_ic_type & (ODM_RTL8812 \| ODM_RTL8822B) | `odm_set_bb_reg(dm, R_0x808, MASKBYTE0, ((val << 4) \| val));` |
| `hal/phydm/phydm_psd.c:371` | phydm_psd_para_setting | odm_set_bb_reg | 32 | BIT(19) \| BIT(18) | dead F:dm->support_ic_type & (ODM_RTL8721D \| ODM_RTL8710C) | `odm_set_bb_reg(dm, 0x808, BIT(19) \| BIT(18), i_q_setting);` |
| `hal/phydm/phydm_psd.c:372` | phydm_psd_para_setting | odm_set_bb_reg | 32 | BIT(21) \| BIT(20) | dead F:dm->support_ic_type & (ODM_RTL8721D \| ODM_RTL8710C) | `odm_set_bb_reg(dm, 0x808, BIT(21) \| BIT(20), hw_avg_time);` |
| `hal/phydm/phydm_psd.c:373` | phydm_psd_para_setting | odm_set_bb_reg | 32 | BIT(23) \| BIT(22) | dead F:dm->support_ic_type & (ODM_RTL8721D \| ODM_RTL8710C) | `odm_set_bb_reg(dm, 0x808, BIT(23) \| BIT(22), fft_smp_point_idx);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:318` | phydm_1rcca_setting | odm_set_bb_reg | 32 | MASKBYTE0 | no-caller | `odm_set_bb_reg(dm, R_0x808, MASKBYTE0, 0x13);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:321` | phydm_1rcca_setting | odm_set_bb_reg | 32 | MASKBYTE0 | no-caller | `odm_set_bb_reg(dm, R_0x808, MASKBYTE0, 0x23);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:326` | phydm_1rcca_setting | odm_set_bb_reg | 32 | MASKBYTE0 | no-caller | `odm_set_bb_reg(dm, R_0x808, MASKBYTE0, 0x31);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:331` | phydm_1rcca_setting | odm_set_bb_reg | 32 | MASKBYTE0 | no-caller | `odm_set_bb_reg(dm, R_0x808, MASKBYTE0, 0x32);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:336` | phydm_1rcca_setting | odm_set_bb_reg | 32 | MASKBYTE0 | no-caller | `odm_set_bb_reg(dm, R_0x808, MASKBYTE0, 0x33);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10225`): 1 mục, giá trị 0xe028233; điều kiện None

## `0x080B` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_noisemonitor.c:364` | odm_inband_noise_monitor_ac | odm_read_1byte | 8 |  | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `if (odm_read_1byte(dm, 0x80B) & BIT(4)) {` |
| `hal/phydm/phydm_noisemonitor.c:365` | odm_inband_noise_monitor_ac | odm_read_1byte | 8 |  | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `val_u8 = odm_read_1byte(dm, 0x80B) & (~BIT(4));` |
| `hal/phydm/phydm_noisemonitor.c:366` | odm_inband_noise_monitor_ac | odm_write_1byte | 8 |  | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `odm_write_1byte(dm, 0x80B, val_u8);` |
| `hal/phydm/phydm_noisemonitor.c:367` | odm_inband_noise_monitor_ac | odm_read_1byte | 8 |  | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `val_u8 = odm_read_1byte(dm, 0x80B) \| BIT(4);` |
| `hal/phydm/phydm_noisemonitor.c:368` | odm_inband_noise_monitor_ac | odm_write_1byte | 8 |  | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `odm_write_1byte(dm, 0x80B, val_u8);` |

## `0x080C` — St A — `R_0x80c`, `rFPGA0_TxGainStage`, `ODM_REG_BB_TX_PATH_11AC`, `ODM_REG_TX_ANT_CTRL_11N`, `REG_FPGA0_TX_GAIN_STAGE`

Định nghĩa: `REG_FPGA0_TX_GAIN_STAGE include/hal_phy_reg.h:67`; `REG_FPGA0_TX_GAIN_STAGE include/hal_phy_reg.h:144`; `R_0x80c hal/phydm/phydm_regtable.h:483`; `ODM_REG_BB_TX_PATH_11AC hal/phydm/phydm_regdefine11ac.h:69`; `ODM_REG_TX_ANT_CTRL_11N hal/phydm/phydm_regdefine11n.h:84`; `rFPGA0_TxGainStage include/rtl8822b_hal.h:119`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:1952` | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | 32 | 0x1f00 | live | `odm_set_bb_reg(dm, R_0x80c, 0x1f00, pw_idx);` |
| `hal/phydm/phydm_api.c:1954` | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | 32 | 0x3e000 | live | `odm_set_bb_reg(dm, R_0x80c, 0x3e000, pw_idx);` |
| `hal/phydm/phydm_api.c:1956` | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | 32 | 0x3f00 | live | `odm_set_bb_reg(dm, R_0x80c, 0x3f00, pw_idx);` |
| `hal/phydm/phydm_api.c:1958` | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | 32 | 0xfc000 | live | `odm_set_bb_reg(dm, R_0x80c, 0xfc000, pw_idx);` |
| `hal/phydm/phydm_api.c:1960` | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | 32 | 0x7f00 | live | `odm_set_bb_reg(dm, R_0x80c, 0x7f00, pw_idx);` |
| `hal/phydm/phydm_api.c:1962` | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | 32 | 0x3f8000 | live | `odm_set_bb_reg(dm, R_0x80c, 0x3f8000, pw_idx);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2388` | phydm_config_tx_path_8822b | odm_set_bb_reg | 32 | (BIT(29) \| BIT(28)) | live | `odm_set_bb_reg(dm, R_0x80c, (BIT(29) \| BIT(28)), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2391` | phydm_config_tx_path_8822b | odm_set_bb_reg | 32 | BIT(30) | live | `odm_set_bb_reg(dm, R_0x80c, BIT(30), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2393` | phydm_config_tx_path_8822b | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0x80c, MASKBYTE0, (tx_path << 4) \| tx_path);` |
| `core/rtw_mp.c:1216` | SetTxAGCOffset | write_bbreg | 32 | (bXBTxAGC \| bXCTxAGC \| | no-caller | `write_bbreg(pAdapter, rFPGA0_TxGainStage,` |
| `hal/phydm/phydm_api.c:164` | phydm_trx_antenna_setting_init | odm_get_bb_reg | 32 | 0xf | dead F:dm->support_ic_type & (ODM_RTL8812 \| ODM_RTL8814A) | `dm->tx_ant_status = (u8)odm_get_bb_reg(dm, R_0x80c, 0xf);` |
| `hal/phydm/phydm_psd.c:375` | phydm_psd_para_setting | odm_set_bb_reg | 32 | BIT(23) | dead F:dm->support_ic_type & (ODM_RTL8721D \| ODM_RTL8710C) | `odm_set_bb_reg(dm, 0x80C, BIT(23), psd_input);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10226`): 1 mục, giá trị 0x10000013; điều kiện None

## `0x0810` — St A — `R_0x810`

Định nghĩa: `R_0x810 hal/phydm/phydm_regtable.h:484`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:410` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0xfff00000 | live | `odm_set_bb_reg(dm, R_0x810, 0xfff00000, 0x211);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10227`): 1 mục, giá trị 0x22101243; điều kiện None

## `0x0814` — St A — `R_0x814`

Định nghĩa: `R_0x814 hal/phydm/phydm_regtable.h:485`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1620` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | 0x0000FC00 | live | `odm_set_bb_reg(dm, R_0x814, 0x0000FC00, 15); /*@default value*/` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1691` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | 0x0000FC00 | live | `odm_set_bb_reg(dm, R_0x814, 0x0000FC00, 34);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1694` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | 0x0000FC00 | live | `odm_set_bb_reg(dm, R_0x814, 0x0000FC00, 15);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10228`): 1 mục, giá trị 0x20c3d11; điều kiện None

## `0x0818` — St A — `R_0x818`, `ODM_AFE_SETTING`, `ODM_REG_BB_PWR_SAV5_11N`

Định nghĩa: `R_0x818 hal/phydm/phydm_regtable.h:486`; `ODM_REG_BB_PWR_SAV5_11N hal/phydm/phydm_regdefine11n.h:85`; `ODM_AFE_SETTING hal/phydm/phydm_reg.h:95`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:2898` | phydm_set_weighting_mcc | odm_get_bb_reg | 32 | 0xff000000 | live | `temp_reg = odm_get_bb_reg(dm, 0x818, 0xff000000);` |
| `hal/phydm/phydm_api.c:2906` | phydm_set_weighting_mcc | odm_get_bb_reg | 32 | 0xff000000 | live | `temp_reg = odm_get_bb_reg(dm, 0x818, 0xff000000);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:86` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | 0x7000000 | live | `odm_set_bb_reg(dm, R_0x818, 0x7000000, 0x1);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:101` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | 0x7000000 | live | `odm_set_bb_reg(dm, R_0x818, 0x7000000, 0x4);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:112` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | 0x7000000 | live | `odm_set_bb_reg(dm, R_0x818, 0x7000000, 0x4);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10229`): 1 mục, giá trị 0x84a10385; điều kiện None

## `0x081C` — St A — `R_0x81c`

Định nghĩa: `R_0x81c hal/phydm/phydm_regtable.h:487`

- Bảng `array_mp_8822b_agc_tab` (`halhwimg8822b_bb.c:146`): 9823 mục, giá trị 0x15a0103, 0x15a0203, 0x15a0303, 0x15a0503, 0x15a0603, 0x15a0703, 0x15c0503, 0x15c0603 …; điều kiện ELSE, ELSE_IF {"rfe": 10}, ELSE_IF {"rfe": 11}
- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10230`): 1 mục, giá trị 0x1e1e081f; điều kiện None

## `0x0820` — St A — `R_0x820`, `rFPGA0_XA_HSSIParameter1`, `REG_FPGA0_XA_HSSI_PARAMETER1`

Định nghĩa: `REG_FPGA0_XA_HSSI_PARAMETER1 include/hal_phy_reg.h:75`; `REG_FPGA0_XA_HSSI_PARAMETER1 include/hal_phy_reg.h:147`; `R_0x820 hal/phydm/phydm_regtable.h:488`; `rFPGA0_XA_HSSIParameter1 include/rtl8822b_hal.h:120`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1915` | hal_mpt_SetSingleToneTx | write_bbreg | 32 | bMaskDWord | live | `write_bbreg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000500);` |
| `hal/hal_mp.c:2011` | hal_mpt_SetSingleToneTx | write_bbreg | 32 | bMaskDWord | live | `write_bbreg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000100);` |
| `hal/hal_mp.c:2053` | hal_mpt_SetCarrierSuppressionTx | write_bbreg | 32 | bMaskDWord | live | `write_bbreg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000500);` |
| `hal/hal_mp.c:2067` | hal_mpt_SetCarrierSuppressionTx | write_bbreg | 32 | bMaskDWord | live | `write_bbreg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000100);` |
| `hal/hal_mp.c:2136` | mpt_StopCckContTx | phy_set_bb_reg | 32 | bMaskDWord | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000100);` |
| `hal/hal_mp.c:2172` | mpt_StopOfdmContTx | phy_set_bb_reg | 32 | bMaskDWord | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000100);` |
| `hal/hal_mp.c:2212` | mpt_StartCckContTx | phy_set_bb_reg | 32 | bMaskDWord | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000500);` |
| `hal/hal_mp.c:2251` | mpt_StartOfdmContTx | phy_set_bb_reg | 32 | bMaskDWord | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000500);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10231`): 1 mục, giá trị 0x1aaaa; điều kiện None

## `0x0824` — St A — `R_0x824`, `rFPGA0_XA_HSSIParameter2`, `ODM_REG_CCK_RPT_FORMAT_11N`

Định nghĩa: `R_0x824 hal/phydm/phydm_regtable.h:489`; `ODM_REG_CCK_RPT_FORMAT_11N hal/phydm/phydm_regdefine11n.h:86`; `rFPGA0_XA_HSSIParameter2 include/rtl8822b_hal.h:121`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1367` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | 0xe | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter2, 0xe, 2);` |
| `hal/hal_mp.c:1394` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | 0xe | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter2, 0xe, 1);` |
| `hal/hal_mp.c:1421` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | 0xe | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter2, 0xe, 2);` |
| `hal/hal_mp.c:2126` | mpt_StopCckContTx | phy_set_bb_reg | 32 | BIT14 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter2, BIT14, 0);` |
| `hal/hal_mp.c:2206` | mpt_StartCckContTx | phy_set_bb_reg | 32 | BIT14 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter2, BIT14, 1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10232`): 1 mục, giá trị 0x30fe0; điều kiện None

## `0x0828` — St A — `R_0x828`, `rFPGA0_XB_HSSIParameter1`, `REG_FPGA0_XB_HSSI_PARAMETER1`

Định nghĩa: `REG_FPGA0_XB_HSSI_PARAMETER1 include/hal_phy_reg.h:78`; `REG_FPGA0_XB_HSSI_PARAMETER1 include/hal_phy_reg.h:150`; `R_0x828 hal/phydm/phydm_regtable.h:490`; `rFPGA0_XB_HSSIParameter1 include/rtl8822b_hal.h:122`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1916` | hal_mpt_SetSingleToneTx | write_bbreg | 32 | bMaskDWord | live | `write_bbreg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000500);` |
| `hal/hal_mp.c:2012` | hal_mpt_SetSingleToneTx | write_bbreg | 32 | bMaskDWord | live | `write_bbreg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000100);` |
| `hal/hal_mp.c:2054` | hal_mpt_SetCarrierSuppressionTx | write_bbreg | 32 | bMaskDWord | live | `write_bbreg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000500);` |
| `hal/hal_mp.c:2068` | hal_mpt_SetCarrierSuppressionTx | write_bbreg | 32 | bMaskDWord | live | `write_bbreg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000100);` |
| `hal/hal_mp.c:2137` | mpt_StopCckContTx | phy_set_bb_reg | 32 | bMaskDWord | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000100);` |
| `hal/hal_mp.c:2173` | mpt_StopOfdmContTx | phy_set_bb_reg | 32 | bMaskDWord | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000100);` |
| `hal/hal_mp.c:2213` | mpt_StartCckContTx | phy_set_bb_reg | 32 | bMaskDWord | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000500);` |
| `hal/hal_mp.c:2252` | mpt_StartOfdmContTx | phy_set_bb_reg | 32 | bMaskDWord | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000500);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10233`): 1 mục, giá trị 0xcccc; điều kiện None

## `0x082C` — St A — `R_0x82c`, `rFPGA0_XB_HSSIParameter2`, `ODM_REG_CCK_RPT_FORMAT_11N_B`

Định nghĩa: `R_0x82c hal/phydm/phydm_regtable.h:491`; `ODM_REG_CCK_RPT_FORMAT_11N_B hal/phydm/phydm_regdefine11n.h:87`; `rFPGA0_XB_HSSIParameter2 include/rtl8822b_hal.h:123`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:845` | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x82c, MASKDWORD, reg82c);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1227` | phydm_dsde_nbi | odm_set_bb_reg | 32 | 0xff000 | live | `odm_set_bb_reg(dm, 0x82c, 0xff000, 0x86);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1229` | phydm_dsde_nbi | odm_set_bb_reg | 32 | 0xff000 | live | `odm_set_bb_reg(dm, 0x82c, 0xff000, 0x97);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1233` | phydm_dsde_nbi | odm_set_bb_reg | 32 | 0xf000 | live | `odm_set_bb_reg(dm, 0x82c, 0xf000, 0x3);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1236` | phydm_dsde_nbi | odm_set_bb_reg | 32 | 0xf000 | live | `odm_set_bb_reg(dm, 0x82c, 0xf000, 0x7);` |
| `hal/hal_mp.c:1368` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | 0xe | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter2, 0xe, 1);` |
| `hal/hal_mp.c:1395` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | 0xe | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter2, 0xe, 2);` |
| `hal/hal_mp.c:1422` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | 0xe | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter2, 0xe, 2);` |
| `hal/hal_mp.c:2127` | mpt_StopCckContTx | phy_set_bb_reg | 32 | BIT14 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter2, BIT14, 0);` |
| `hal/hal_mp.c:2207` | mpt_StartCckContTx | phy_set_bb_reg | 32 | BIT14 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter2, BIT14, 1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10234`): 1 mục, giá trị 0x75cb7010; điều kiện None

## `0x0830` — St A — `R_0x830`, `ODM_TXAGC_B_6_18`, `rTxAGC_B_Rate18_06`, `REG_TX_AGC_B_RATE18_06`

Định nghĩa: `REG_TX_AGC_B_RATE18_06 include/hal_phy_reg.h:250`; `R_0x830 hal/phydm/phydm_regtable.h:492`; `ODM_TXAGC_B_6_18 hal/phydm/phydm_reg.h:96`; `rTxAGC_B_Rate18_06 include/rtl8822b_hal.h:124`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:852` | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x830, MASKDWORD, 0x79a0ea28);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:854` | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x830, MASKDWORD, reg830);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:856` | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x830, MASKDWORD, reg830);` |
| `hal/hal_mp.c:452` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskDWord | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_B_Rate18_06, bMaskDWord, TxAGC);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10235`): 1 mục, giá trị 0x79a0eaaa; điều kiện None

## `0x0834` — St A — `R_0x834`, `ODM_TXAGC_B_24_54`, `rTxAGC_B_Rate54_24`, `REG_TX_AGC_B_RATE54_24`

Định nghĩa: `REG_TX_AGC_B_RATE54_24 include/hal_phy_reg.h:251`; `R_0x834 hal/phydm/phydm_regtable.h:493`; `ODM_TXAGC_B_24_54 hal/phydm/phydm_reg.h:97`; `rTxAGC_B_Rate54_24 include/rtl8822b_hal.h:125`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:453` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskDWord | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_B_Rate54_24, bMaskDWord, TxAGC);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10236`): 1 mục, giá trị 0x72e6986; điều kiện None

## `0x0838` — St A — `R_0x838`, `rCCAonSec_Jaguar`, `ODM_TXAGC_B_MCS32_5`, `rTxAGC_B_CCK1_55_Mcs32`, `REG_TX_AGC_B_CCK_1_55_MCS32`

Định nghĩa: `REG_TX_AGC_B_CCK_1_55_MCS32 include/hal_phy_reg.h:229`; `R_0x838 hal/phydm/phydm_regtable.h:494`; `ODM_TXAGC_B_MCS32_5 hal/phydm/phydm_reg.h:98`; `rTxAGC_B_CCK1_55_Mcs32 include/rtl8822b_hal.h:126`; `rCCAonSec_Jaguar include/rtl8822b_hal.h:127`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:962` | mpt_SetSingleTone_8814A | phy_set_bb_reg | 32 | BIT1 | live | `phy_set_bb_reg(pAdapter, rCCAonSec_Jaguar, BIT1, 0x1); /*/ Disable CCA*/` |
| `hal/hal_mp.c:1005` | mpt_SetSingleTone_8814A | phy_set_bb_reg | 32 | BIT1 | live | `phy_set_bb_reg(pAdapter, rCCAonSec_Jaguar, BIT1, 0x0); /* Enable CCA*/` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:610` | _iqk_configure_macbb_8822b | odm_set_bb_reg | 32 | BIT(3) \| BIT(2) \| BIT( | live | `odm_set_bb_reg(dm, R_0x838, BIT(3) \| BIT(2) \| BIT(1), 0x7);` |
| `hal/phydm/phydm_api.c:680` | phydm_stop_ic_trx | odm_set_bb_reg | 32 | BIT(1) | live | `odm_set_bb_reg(dm, R_0x838, BIT(1), 1);` |
| `hal/phydm/phydm_api.c:706` | phydm_stop_ic_trx | odm_set_bb_reg | 32 | BIT(1) | live | `odm_set_bb_reg(dm, R_0x838, BIT(1), 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:858` | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x838, MASKDWORD, reg838);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:869` | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | 32 | 0xf0 | live | `odm_set_bb_reg(dm, 0x838, 0xf0, 0x4);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:78` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | 0xf0 | live | `odm_set_bb_reg(dm, 0x838, 0xf0, 0x6);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:590` | phydm_hwsetting_8822b | odm_set_bb_reg | 32 | 0xf0 | live | `odm_set_bb_reg(dm, 0x838, 0xf0, 0x6);` |
| `hal/hal_mp.c:427` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | 0xffffff00 | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_B_CCK1_55_Mcs32, 0xffffff00, TxAGC);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10237`): 1 mục, giá trị 0x87766441; điều kiện None

## `0x083C` — St A — `R_0x83c`, `rTxAGC_B_Mcs03_Mcs00`, `ODM_TXAGC_B_MCS0_MCS3`, `REG_TX_AGC_B_MCS03_MCS00`

Định nghĩa: `REG_TX_AGC_B_MCS03_MCS00 include/hal_phy_reg.h:236`; `R_0x83c hal/phydm/phydm_regtable.h:495`; `ODM_TXAGC_B_MCS0_MCS3 hal/phydm/phydm_reg.h:99`; `rTxAGC_B_Mcs03_Mcs00 include/rtl8822b_hal.h:128`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:861` | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x83c, MASKDWORD, 0x9194b2b9);` |
| `hal/hal_mp.c:454` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskDWord | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_B_Mcs03_Mcs00, bMaskDWord, TxAGC);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10238`): 1 mục, giá trị 0x9194b2b7; điều kiện None

## `0x0840` — St A — `R_0x840`, `R_0x844`

Định nghĩa: `R_0x840 hal/phydm/phydm_regtable.h:496`; `R_0x844 hal/phydm/phydm_regtable.h:497`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2188` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | 0x0000f000 | live | `odm_set_bb_reg(dm, R_0x840, 0x0000f000, 0x6);` |
| `hal/phydm/phydm_adaptivity.c:388` | phydm_edcca_decision_opt | odm_set_bb_reg | 32 | BIT(30) \| BIT(29) | dead F:dm->support_ic_type & ODM_IC_JGR3_SERIES | `odm_set_bb_reg(dm, R_0x844, BIT(30) \| BIT(29), 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10239`): 1 mục, giá trị 0x171750e0; điều kiện None

## `0x0844` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10240`): 1 mục, giá trị 0x4d3d7cdb; điều kiện None

## `0x0848` — St A — `R_0x848`, `rTxAGC_B_Mcs07_Mcs04`, `ODM_TXAGC_B_MCS4_MCS7`, `REG_TX_AGC_B_MCS07_MCS04`

Định nghĩa: `REG_TX_AGC_B_MCS07_MCS04 include/hal_phy_reg.h:237`; `R_0x848 hal/phydm/phydm_regtable.h:498`; `ODM_TXAGC_B_MCS4_MCS7 hal/phydm/phydm_reg.h:100`; `rTxAGC_B_Mcs07_Mcs04 include/rtl8822b_hal.h:129`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:455` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskDWord | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_B_Mcs07_Mcs04, bMaskDWord, TxAGC);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10241`): 1 mục, giá trị 0x4ad0408b; điều kiện None

## `0x084C` — St A — `R_0x84c`, `rTxAGC_B_Mcs11_Mcs08`, `ODM_TXAGC_B_MCS8_MCS11`, `REG_TX_AGC_B_MCS11_MCS08`

Định nghĩa: `REG_TX_AGC_B_MCS11_MCS08 include/hal_phy_reg.h:238`; `R_0x84c hal/phydm/phydm_regtable.h:499`; `ODM_TXAGC_B_MCS8_MCS11 hal/phydm/phydm_reg.h:101`; `rTxAGC_B_Mcs11_Mcs08 include/rtl8822b_hal.h:130`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:456` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskDWord | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_B_Mcs11_Mcs08, bMaskDWord, TxAGC);` |
| `hal/phydm/phydm_adaptivity.c:216` | phydm_set_edcca_threshold | odm_set_bb_reg | 32 | MASKBYTE2 | dead F:dm->support_ic_type & ODM_IC_JGR3_SERIES | `odm_set_bb_reg(dm, R_0x84c, MASKBYTE2, (u8)L2H + 0x80);` |
| `hal/phydm/phydm_adaptivity.c:217` | phydm_set_edcca_threshold | odm_set_bb_reg | 32 | MASKBYTE3 | dead F:dm->support_ic_type & ODM_IC_JGR3_SERIES | `odm_set_bb_reg(dm, R_0x84c, MASKBYTE3, (u8)H2L + 0x80);` |
| `hal/phydm/phydm_adaptivity.c:442` | phydm_adaptivity_debug | odm_get_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_JGR3_SERIES | `reg_value32 = odm_get_bb_reg(dm, R_0x84c, MASKDWORD);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10242`): 1 mục, giá trị 0x6afbf7a5; điều kiện None

## `0x0850` — St A — `R_0x850`

Định nghĩa: `R_0x850 hal/phydm/phydm_regtable.h:500`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2457` | phydm_config_rx_path_8822b | odm_set_bb_reg | 32 | BIT(23) | live | `odm_set_bb_reg(dm, R_0x850, BIT(23), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2464` | phydm_config_rx_path_8822b | odm_set_bb_reg | 32 | BIT(23) | live | `odm_set_bb_reg(dm, R_0x850, BIT(23), 0x1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10243`): 1 mục, giá trị 0x28a74706; điều kiện None

## `0x0854` — St A — `R_0x854`

Định nghĩa: `R_0x854 hal/phydm/phydm_regtable.h:501`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10244`): 1 mục, giá trị 0x1520c; điều kiện None

## `0x0858` — St A — `R_0x858`, `ODM_REG_RX_DEFAULT_A_11N`

Định nghĩa: `R_0x858 hal/phydm/phydm_regtable.h:502`; `ODM_REG_RX_DEFAULT_A_11N hal/phydm/phydm_regdefine11n.h:88`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10245`): 1 mục, giá trị 0x4060c000; điều kiện None

## `0x085C` — St A — `R_0x85c`, `ODM_ANALOG_REGISTER`, `ODM_REG_BB_PWR_SAV3_11N`

Định nghĩa: `R_0x85c hal/phydm/phydm_regtable.h:503`; `ODM_REG_BB_PWR_SAV3_11N hal/phydm/phydm_regdefine11n.h:90`; `ODM_ANALOG_REGISTER hal/phydm/phydm_reg.h:102`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10246`): 1 mục, giá trị 0x74010160; điều kiện None

## `0x0860` — St A — `R_0x860`, `ODM_REG_BB_ATC_11AC`, `ODM_REG_ANTSEL_CTRL_11N`, `ODM_RF_INTERFACE_OUTPUT`, `rFPGA0_XA_RFInterfaceOE`, `REG_FPGA0_XA_RF_INTERFACE_OE`

Định nghĩa: `REG_FPGA0_XA_RF_INTERFACE_OE include/hal_phy_reg.h:77`; `REG_FPGA0_XA_RF_INTERFACE_OE include/hal_phy_reg.h:149`; `R_0x860 hal/phydm/phydm_regtable.h:504`; `ODM_REG_BB_ATC_11AC hal/phydm/phydm_regdefine11ac.h:70`; `ODM_REG_ANTSEL_CTRL_11N hal/phydm/phydm_regdefine11n.h:91`; `ODM_RF_INTERFACE_OUTPUT hal/phydm/phydm_reg.h:103`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1899` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | 0x1ffe0000 | live | `odm_set_bb_reg(dm, R_0x860, 0x1ffe0000, 0x96a);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1941` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | 0x1ffe0000 | live | `odm_set_bb_reg(dm, R_0x860, 0x1ffe0000, 0x494);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1943` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | 0x1ffe0000 | live | `odm_set_bb_reg(dm, R_0x860, 0x1ffe0000, 0x453);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1945` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | 0x1ffe0000 | live | `odm_set_bb_reg(dm, R_0x860, 0x1ffe0000, 0x452);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1947` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | 0x1ffe0000 | live | `odm_set_bb_reg(dm, R_0x860, 0x1ffe0000, 0x412);` |
| `hal/hal_mp.c:1401` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT10 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XA_RFInterfaceOE, BIT10, 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10247`): 1 mục, giá trị 0x68a7c321; điều kiện None

## `0x0864` — St A — `R_0x864`, `ODM_REG_RX_ANT_CTRL_11N`, `rFPGA0_XB_RFInterfaceOE`, `REG_FPGA0_XB_RF_INTERFACE_OE`

Định nghĩa: `REG_FPGA0_XB_RF_INTERFACE_OE include/hal_phy_reg.h:81`; `REG_FPGA0_XB_RF_INTERFACE_OE include/hal_phy_reg.h:153`; `R_0x864 hal/phydm/phydm_regtable.h:505`; `ODM_REG_RX_ANT_CTRL_11N hal/phydm/phydm_regdefine11n.h:92`; `rFPGA0_XB_RFInterfaceOE include/rtl8822b_hal.h:132`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1376` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT10 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XB_RFInterfaceOE, BIT10, 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10248`): 1 mục, giá trị 0x79f27032; điều kiện None

## `0x0868` — St A — `R_0x868`, `rTxAGC_B_Mcs15_Mcs12`, `ODM_TXAGC_B_MCS12_MCS15`, `REG_TX_AGC_B_MCS15_MCS12`

Định nghĩa: `REG_TX_AGC_B_MCS15_MCS12 include/hal_phy_reg.h:239`; `R_0x868 hal/phydm/phydm_regtable.h:506`; `ODM_TXAGC_B_MCS12_MCS15 hal/phydm/phydm_reg.h:104`; `rTxAGC_B_Mcs15_Mcs12 include/rtl8822b_hal.h:133`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:457` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskDWord | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_B_Mcs15_Mcs12, bMaskDWord, TxAGC);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10249`): 1 mục, giá trị 0x8ca7a314; điều kiện None

## `0x086C` — St A — `R_0x86c`, `ODM_TXAGC_B_11_A_2_11`, `rTxAGC_B_CCK11_A_CCK2_11`, `REG_TX_AGC_B_CCK_11_A_CCK_2_11`

Định nghĩa: `REG_TX_AGC_B_CCK_11_A_CCK_2_11 include/hal_phy_reg.h:227`; `R_0x86c hal/phydm/phydm_regtable.h:507`; `ODM_TXAGC_B_11_A_2_11 hal/phydm/phydm_reg.h:105`; `rTxAGC_B_CCK11_A_CCK2_11 include/rtl8822b_hal.h:134`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:421` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | 0xffffff00 | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_B_CCK11_A_CCK2_11, 0xffffff00, TxAGC);` |
| `hal/hal_mp.c:426` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskByte0 | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_B_CCK11_A_CCK2_11, bMaskByte0, pTxPower[RF_PATH_B]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10250`): 1 mục, giá trị 0x778c2878; điều kiện None

## `0x0870` — St A — `R_0x870`, `ODM_REG_PIN_CTRL_11N`, `rFPGA0_XAB_RFInterfaceSW`, `REG_FPGA0_XAB_RF_INTERFACE_SW`

Định nghĩa: `REG_FPGA0_XAB_RF_INTERFACE_SW include/hal_phy_reg.h:73`; `REG_FPGA0_XAB_RF_INTERFACE_SW include/hal_phy_reg.h:145`; `R_0x870 hal/phydm/phydm_regtable.h:508`; `ODM_REG_PIN_CTRL_11N hal/phydm/phydm_regdefine11n.h:93`; `rFPGA0_XAB_RFInterfaceSW include/rtl8822b_hal.h:135`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1374` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT10 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT10, 0);` |
| `hal/hal_mp.c:1375` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT26 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT26, 1);` |
| `hal/hal_mp.c:1400` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT10 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT10, 1);` |
| `hal/hal_mp.c:1402` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT26 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT26, 0);` |
| `hal/hal_mp.c:1427` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT10 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT10, 0);` |
| `hal/hal_mp.c:1429` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT26 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT26, 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10251`): 1 mục, giá trị 0x77777777; điều kiện None

## `0x0874` — St A — `R_0x874`, `ODM_AD_DA_LSB_MASK`, `ODM_REG_BB_PWR_SAV1_11N`

Định nghĩa: `R_0x874 hal/phydm/phydm_regtable.h:509`; `ODM_REG_BB_PWR_SAV1_11N hal/phydm/phydm_regdefine11n.h:94`; `ODM_AD_DA_LSB_MASK hal/phydm/phydm_reg.h:106`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:814` | phydm_csi_mask_enable | odm_set_bb_reg | 32 | BIT(0) | live | `odm_set_bb_reg(dm, R_0x874, BIT(0), en);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1156` | phydm_dsde_init | odm_set_bb_reg | 32 | BIT(0) | live | `odm_set_bb_reg(dm, R_0x874, BIT(0), 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10252`): 1 mục, giá trị 0x27612c2e; điều kiện None

## `0x0878` — St A — `R_0x878`, `rFPGA0_XAB_RFParameter`, `ODM_REG_ANTSEL_PATH_11N`, `REG_FPGA0_XAB_RF_PARAMETER`

Định nghĩa: `REG_FPGA0_XAB_RF_PARAMETER include/hal_phy_reg.h:74`; `REG_FPGA0_XAB_RF_PARAMETER include/hal_phy_reg.h:146`; `R_0x878 hal/phydm/phydm_regtable.h:510`; `ODM_REG_ANTSEL_PATH_11N hal/phydm/phydm_regdefine11n.h:95`; `rFPGA0_XAB_RFParameter include/rtl8822b_hal.h:136`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1377` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT1 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFParameter, BIT1, 1);` |
| `hal/hal_mp.c:1378` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT17 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFParameter, BIT17, 0);` |
| `hal/hal_mp.c:1404` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT1 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFParameter, BIT1, 0);` |
| `hal/hal_mp.c:1405` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT17 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFParameter, BIT17, 1);` |
| `hal/hal_mp.c:1431` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT1 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFParameter, BIT1, 1);` |
| `hal/hal_mp.c:1432` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | BIT17 | dead-func | `phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFParameter, BIT17, 1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10253`): 1 mục, giá trị 0xc0003152; điều kiện None

## `0x087C` — St A — `R_0x87c`

Định nghĩa: `R_0x87c hal/phydm/phydm_regtable.h:511`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:978` | phydm_set_nbi_reg | odm_set_bb_reg | 32 | 0xfc000 | live | `odm_set_bb_reg(dm, R_0x87c, 0xfc000, reg_idx);` |
| `hal/phydm/phydm_api.c:1003` | phydm_nbi_enable | odm_set_bb_reg | 32 | BIT(13) | live | `odm_set_bb_reg(dm, R_0x87c, BIT(13), val);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1576` | phydm_spur_calibration_8822b | odm_set_bb_reg | 32 | BIT(13) | live | `odm_set_bb_reg(dm, R_0x87c, BIT(13), 0x0);` |
| `hal/phydm/phydm_api.c:1008` | phydm_nbi_enable | odm_set_bb_reg | 32 | BIT(13) | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `odm_set_bb_reg(dm, R_0x87c, BIT(13), val);` |
| `hal/phydm/phydm_api.c:1904` | phydm_stop_ck320 | odm_set_bb_reg | 32 | BIT(29) | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x87c, BIT(29), val);` |
| `hal/phydm/phydm_api.c:1906` | phydm_stop_ck320 | odm_set_bb_reg | 32 | BIT(31) | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x87c, BIT(31), val);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1760` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(13) | cfg-off C:*dm->mp_mode | `odm_set_bb_reg(dm, R_0x87c, BIT(13), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2021` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | BIT(13) | cfg-off C:*dm->mp_mode | `odm_set_bb_reg(dm, R_0x87c, BIT(13), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2286` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(13) | cfg-off C:*dm->mp_mode | `odm_set_bb_reg(dm, R_0x87c, BIT(13), 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10254`): 1 mục, giá trị 0x5c8fc000; điều kiện None

## `0x0880` — St A — `R_0x880`

Định nghĩa: `R_0x880 hal/phydm/phydm_regtable.h:512`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:848` | phydm_clean_all_csi_mask | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x880, MASKDWORD, 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1148` | phydm_dsde_init | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x880, MASKDWORD, 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10255`): 1 mục, giá trị 0x0; điều kiện None

## `0x0884` — St A — `R_0x884`

Định nghĩa: `R_0x884 hal/phydm/phydm_regtable.h:513`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:849` | phydm_clean_all_csi_mask | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x884, MASKDWORD, 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1149` | phydm_dsde_init | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x884, MASKDWORD, 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10256`): 1 mục, giá trị 0x0; điều kiện None

## `0x0888` — St A — `R_0x888`

Định nghĩa: `R_0x888 hal/phydm/phydm_regtable.h:514`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:850` | phydm_clean_all_csi_mask | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x888, MASKDWORD, 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1150` | phydm_dsde_init | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x888, MASKDWORD, 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10257`): 1 mục, giá trị 0x0; điều kiện None

## `0x088C` — St A — `R_0x88c`, `ODM_ENABLE_3_WIRE`, `ODM_REG_BB_3WIRE_11N`, `rFPGA0_AnalogParameter4`, `REG_FPGA0_ANALOG_PARAMETER4`

Định nghĩa: `REG_FPGA0_ANALOG_PARAMETER4 include/hal_phy_reg.h:139`; `R_0x88c hal/phydm/phydm_regtable.h:515`; `ODM_REG_BB_3WIRE_11N hal/phydm/phydm_regdefine11n.h:96`; `ODM_ENABLE_3_WIRE hal/phydm/phydm_reg.h:107`; `rFPGA0_AnalogParameter4 include/rtl8822b_hal.h:137`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_mp.c:2221` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_88c = rtw_read32(padapter, 0x88C);` |
| `core/rtw_mp.c:2222` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x300000 | live | `phy_set_bb_reg(padapter, 0x88C, 0x300000, 0x3);` |
| `core/rtw_mp.c:2242` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_88c = rtw_read32(padapter, 0x88C);` |
| `core/rtw_mp.c:2243` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0xF00000 | live | `phy_set_bb_reg(padapter, 0x88C, 0xF00000, 0xF);` |
| `core/rtw_mp.c:2313` | rtw_mp_psd_close | rtw_write32 | 32 |  | live | `rtw_write32(padapter, 0x88C, regs->reg_88c);` |
| `hal/phydm/phydm_api.c:851` | phydm_clean_all_csi_mask | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x88c, MASKDWORD, 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1151` | phydm_dsde_init | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x88c, MASKDWORD, 0);` |
| `hal/hal_mp.c:1800` | hal_mpt_SetSingleToneTx | phy_set_mac_reg | 32 | 0xF00000 | dead F:IS_HARDWARE_TYPE_8192E(pAdapter) | `phy_set_mac_reg(pAdapter, 0x88C, 0xF00000, 0xF);` |
| `hal/hal_mp.c:1831` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xF00000 | dead F:IS_HARDWARE_TYPE_8188F(pAdapter) \|\| IS_HARDWARE_TYPE_8188GTV | `phy_set_bb_reg(pAdapter, rFPGA0_AnalogParameter4, 0xF00000, 0xF);` |
| `hal/hal_mp.c:1928` | hal_mpt_SetSingleToneTx | phy_set_mac_reg | 32 | 0xF00000 | dead F:IS_HARDWARE_TYPE_8192E(pAdapter) | `phy_set_mac_reg(pAdapter, 0x88C, 0xF00000, 0x0);` |
| `hal/hal_mp.c:1959` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xF00000 | dead F:IS_HARDWARE_TYPE_8188F(pAdapter) \|\| IS_HARDWARE_TYPE_8188GTV | `phy_set_bb_reg(pAdapter, rFPGA0_AnalogParameter4, 0xF00000, 0xc);` |
| `hal/phydm/phydm_api.c:577` | phydm_stop_3_wire | odm_set_bb_reg | 32 | 0xf00000 | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x88c, 0xf00000, 0xf);` |
| `hal/phydm/phydm_api.c:606` | phydm_stop_3_wire | odm_set_bb_reg | 32 | 0xf00000 | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x88c, 0xf00000, 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10258`): 1 mục, giá trị 0x0; điều kiện None

## `0x0890` — St A — `R_0x890`, `ODM_REG_CLM_11N`, `ODM_REG_NHM_TH9_TH10_11N`

Định nghĩa: `R_0x890 hal/phydm/phydm_regtable.h:516`; `ODM_REG_NHM_TH9_TH10_11N hal/phydm/phydm_regdefine11n.h:100`; `ODM_REG_CLM_11N hal/phydm/phydm_regdefine11n.h:101`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:852` | phydm_clean_all_csi_mask | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x890, MASKDWORD, 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1152` | phydm_dsde_init | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x890, MASKDWORD, 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10259`): 1 mục, giá trị 0x0; điều kiện None

## `0x0894` — St A — `R_0x894`, `ODM_REG_CCX_PERIOD_11N`

Định nghĩa: `R_0x894 hal/phydm/phydm_regtable.h:517`; `ODM_REG_CCX_PERIOD_11N hal/phydm/phydm_regdefine11n.h:99`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:853` | phydm_clean_all_csi_mask | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x894, MASKDWORD, 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1153` | phydm_dsde_init | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x894, MASKDWORD, 0);` |
| `hal/phydm/phydm_ccx.c:1661` | phydm_clm_setting | odm_set_bb_reg | 32 | MASKLWORD | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0x894, MASKLWORD, clm_period);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10260`): 1 mục, giá trị 0x0; điều kiện None

## `0x0898` — St A — `R_0x898`, `ODM_REG_NHM_TH3_TO_TH0_11N`

Định nghĩa: `R_0x898 hal/phydm/phydm_regtable.h:518`; `ODM_REG_NHM_TH3_TO_TH0_11N hal/phydm/phydm_regdefine11n.h:102`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:854` | phydm_clean_all_csi_mask | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x898, MASKDWORD, 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1154` | phydm_dsde_init | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x898, MASKDWORD, 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10261`): 1 mục, giá trị 0x0; điều kiện None

## `0x089C` — St A — `R_0x89c`, `ODM_REG_NHM_TH7_TO_TH4_11N`

Định nghĩa: `R_0x89c hal/phydm/phydm_regtable.h:519`; `ODM_REG_NHM_TH7_TO_TH4_11N hal/phydm/phydm_regdefine11n.h:103`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:855` | phydm_clean_all_csi_mask | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x89c, MASKDWORD, 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1155` | phydm_dsde_init | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x89c, MASKDWORD, 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10262`): 1 mục, giá trị 0x0; điều kiện None

## `0x08A0` — St A — `R_0x8a0`

Định nghĩa: `R_0x8a0 hal/phydm/phydm_regtable.h:520`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_adaptivity.c:373` | phydm_set_forgetting_factor | odm_set_bb_reg | 32 | BIT(1) \| BIT(0) | live | `odm_set_bb_reg(dm, R_0x8a0, BIT(1) \| BIT(0), 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10263`): 1 mục, giá trị 0x13; điều kiện None

## `0x08A4` — St A — `R_0x8a4`, `rFPGA0_XB_LSSIReadBack`, `REG_FPGA0_XB_LSSI_READ_BACK`

Định nghĩa: `REG_FPGA0_XB_LSSI_READ_BACK include/hal_phy_reg.h:66`; `REG_FPGA0_XB_LSSI_READ_BACK include/hal_phy_reg.h:80`; `REG_FPGA0_XB_LSSI_READ_BACK include/hal_phy_reg.h:152`; `R_0x8a4 hal/phydm/phydm_regtable.h:521`; `rFPGA0_XB_LSSIReadBack include/rtl8822b_hal.h:138`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_adaptivity.c:222` | phydm_set_edcca_threshold | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0x8a4, MASKBYTE0, (u8)L2H);` |
| `hal/phydm/phydm_adaptivity.c:223` | phydm_set_edcca_threshold | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0x8a4, MASKBYTE1, (u8)H2L);` |
| `hal/phydm/phydm_adaptivity.c:450` | phydm_adaptivity_debug | odm_get_bb_reg | 32 | MASKDWORD | live | `reg_value32 = odm_get_bb_reg(dm, R_0x8a4, MASKDWORD);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10264`): 1 mục, giá trị 0x7f7f7f7f; điều kiện None

## `0x08A8` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10265`): 1 mục, giá trị 0x2202033e; điều kiện None

## `0x08AC` — St A — `R_0x8ac`

Định nghĩa: `R_0x8ac hal/phydm/phydm_regtable.h:522`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2103` | config_phydm_switch_bandwidth_8822b | odm_get_bb_reg | 32 | MASKDWORD | live | `val32 = odm_get_bb_reg(dm, R_0x8ac, MASKDWORD);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2106` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8ac, MASKDWORD, val32);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2140` | config_phydm_switch_bandwidth_8822b | odm_get_bb_reg | 32 | MASKDWORD | live | `val32 = odm_get_bb_reg(dm, R_0x8ac, MASKDWORD);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2143` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8ac, MASKDWORD, val32);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2174` | config_phydm_switch_bandwidth_8822b | odm_get_bb_reg | 32 | MASKDWORD | live | `val32 = odm_get_bb_reg(dm, R_0x8ac, MASKDWORD);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2177` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8ac, MASKDWORD, val32);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2214` | config_phydm_switch_bandwidth_8822b | odm_get_bb_reg | 32 | MASKDWORD | live | `val32 = odm_get_bb_reg(dm, R_0x8ac, MASKDWORD);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2217` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8ac, MASKDWORD, val32);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2248` | config_phydm_switch_bandwidth_8822b | odm_get_bb_reg | 32 | MASKDWORD | live | `val32 = odm_get_bb_reg(dm, R_0x8ac, MASKDWORD);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2251` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8ac, MASKDWORD, val32);` |
| `hal/phydm/phydm_direct_bf.c:277` | phydm_set_direct_bfer | odm_get_bb_reg | 32 | 0x3 | no-caller T:dm->support_ic_type & ODM_RTL8822B | `u32 bw = odm_get_bb_reg(dm, R_0x8ac, 0x3); //bit 11-10` |
| `hal/phydm/phydm_direct_bf.c:280` | phydm_set_direct_bfer | odm_get_bb_reg | 32 | 0x3 | no-caller T:dm->support_ic_type & ODM_RTL8822B | `u32 ndp_bw = odm_get_bb_reg(dm, R_0x8ac, 0x3); //bit 11-10` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10266`): 1 mục, giá trị 0xf00f000a; điều kiện None

## `0x08B0` — St A — `rHSSIRead_Jaguar`

Định nghĩa: `rHSSIRead_Jaguar include/rtl8822b_hal.h:139`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10267`): 1 mục, giá trị 0x600; điều kiện None

## `0x08B4` — St A — `R_0x8b4`, `ODM_PSD_REPORT`, `ODM_REG_PSD_DATA_11N`, `ODM_REG_CLM_READY_11N`

Định nghĩa: `R_0x8b4 hal/phydm/phydm_regtable.h:523`; `ODM_REG_PSD_DATA_11N hal/phydm/phydm_regdefine11n.h:98`; `ODM_REG_CLM_READY_11N hal/phydm/phydm_regdefine11n.h:105`; `ODM_PSD_REPORT hal/phydm/phydm_reg.h:108`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_adc_sampling.c:1067` | phydm_la_set_bb | odm_set_bb_reg | 32 | BIT(7) | live | `odm_set_bb_reg(dm, R_0x8b4, BIT(7), 1);/*@update rpt every pkt*/` |
| `hal/phydm/phydm_api.c:1901` | phydm_stop_ck320 | odm_set_bb_reg | 32 | BIT(6) | live | `odm_set_bb_reg(dm, R_0x8b4, BIT(6), val);` |
| `hal/phydm/phydm_api.c:1948` | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0x8b4, 0x3f, pw_idx); /*6Bit*/` |
| `hal/phydm/phydm_noisemonitor.c:323` | odm_inband_noise_monitor_ac | odm_set_bb_reg | 32 | BIT(6) | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `odm_set_bb_reg(dm, R_0x8b4, BIT(6), 1);` |
| `hal/phydm/phydm_noisemonitor.c:352` | odm_inband_noise_monitor_ac | odm_set_bb_reg | 32 | BIT(6) | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `odm_set_bb_reg(dm, R_0x8b4, BIT(6), 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10268`): 1 mục, giá trị 0xfc080; điều kiện None

## `0x08B8` — St A — `R_0x8b8`

Định nghĩa: `R_0x8b8 hal/phydm/phydm_regtable.h:524`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10269`): 1 mục, giá trị 0xec0057f7; điều kiện None

## `0x08BC` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10270`): 1 mục, giá trị 0xacb520a3; điều kiện None

## `0x08C0` — St A — `R_0x8c0`

Định nghĩa: `R_0x8c0 hal/phydm/phydm_regtable.h:525`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10271`): 1 mục, giá trị 0xffe04020; điều kiện None

## `0x08C4` — St A — `R_0x8c4`, `ODM_REG_SC_CNT_11N`

Định nghĩa: `R_0x8c4 hal/phydm/phydm_regtable.h:526`; `ODM_REG_SC_CNT_11N hal/phydm/phydm_regdefine11n.h:97`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:4179` | phydm_per_tone_evm | odm_set_bb_reg | 32 | 0xff8 | live | `odm_set_bb_reg(dm, R_0x8c4, 0xff8, tone_num);` |
| `hal/phydm/phydm_debug.c:4206` | phydm_per_tone_evm | odm_set_bb_reg | 32 | 0xff8 | live | `odm_set_bb_reg(dm, R_0x8c4, 0xff8, tone_num);` |
| `hal/phydm/phydm_debug.c:4233` | phydm_per_tone_evm | odm_set_bb_reg | 32 | 0xff8 | live | `odm_set_bb_reg(dm, R_0x8c4, 0xff8, tone_num);` |
| `hal/phydm/phydm_debug.c:4261` | phydm_per_tone_evm | odm_set_bb_reg | 32 | 0xff8 | live | `odm_set_bb_reg(dm, R_0x8c4, 0xff8, tone_num);` |
| `hal/phydm/phydm_debug.c:4288` | phydm_per_tone_evm | odm_set_bb_reg | 32 | 0xff8 | live | `odm_set_bb_reg(dm, R_0x8c4, 0xff8, tone_num);` |
| `hal/phydm/phydm_debug.c:4315` | phydm_per_tone_evm | odm_set_bb_reg | 32 | 0xff8 | live | `odm_set_bb_reg(dm, R_0x8c4, 0xff8, tone_num);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2109` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(30) | live | `odm_set_bb_reg(dm, R_0x8c4, BIT(30), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2146` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(30) | live | `odm_set_bb_reg(dm, R_0x8c4, BIT(30), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2180` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(30) | live | `odm_set_bb_reg(dm, R_0x8c4, BIT(30), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2220` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(30) | live | `odm_set_bb_reg(dm, R_0x8c4, BIT(30), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2254` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(30) | live | `odm_set_bb_reg(dm, R_0x8c4, BIT(30), 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10272`): 1 mục, giá trị 0x47c00000; điều kiện None

## `0x08C8` — St A — `R_0x8c8`

Định nghĩa: `R_0x8c8 hal/phydm/phydm_regtable.h:527`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:4038` | phydm_enable_big_jump | odm_set_bb_reg | 32 | 0xfe | live | `odm_set_bb_reg(dm, R_0x8c8, 0xfe, val);` |
| `hal/phydm/phydm_dig.c:367` | phydm_set_big_jump_step | odm_set_bb_reg | 32 | 0xe | live | `odm_set_bb_reg(dm, R_0x8c8, 0xe, i);` |
| `hal/phydm/phydm_dig.c:811` | phydm_dig_init | odm_get_bb_reg | 32 | MASKLWORD | live | `ret_value = odm_get_bb_reg(dm, R_0x8c8, MASKLWORD);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:864` | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | 32 | BIT(3) \| BIT(2) \| BIT( | live | `odm_set_bb_reg(dm, R_0x8c8, BIT(3) \| BIT(2) \| BIT(1), 0x3);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2189` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(10) | live | `odm_set_bb_reg(dm, R_0x8c8, BIT(10), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2221` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(31) | live | `odm_set_bb_reg(dm, R_0x8c8, BIT(31), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2255` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(31) | live | `odm_set_bb_reg(dm, R_0x8c8, BIT(31), 0x1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10273`): 1 mục, giá trị 0x251a5; điều kiện None

## `0x08CC` — St A — `R_0x8cc`

Định nghĩa: `R_0x8cc hal/phydm/phydm_regtable.h:528`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1635` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8cc, MASKDWORD,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1640` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8cc, MASKDWORD,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1653` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8cc, MASKDWORD,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1658` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8cc, MASKDWORD,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1708` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8cc, MASKDWORD, 0x08108000);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1718` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8cc, MASKDWORD,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1723` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8cc, MASKDWORD,` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:390` | phydm_somlrxhp_setting | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8cc, MASKDWORD, 0x08108000);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:396` | phydm_somlrxhp_setting | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8cc, MASKDWORD, 0x08108000);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:401` | phydm_somlrxhp_setting | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8cc, MASKDWORD, 0x08108000);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:408` | phydm_somlrxhp_setting | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, 0x8cc, MASKDWORD, 0x08108492);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10274`): 1 mục, giá trị 0x8108492; điều kiện None

## `0x08D0` — St A — `R_0x8d0`, `ODM_REG_CLM_RESULT_11N`, `ODM_REG_NHM_CNT9_TO_CNT8_11N`

Định nghĩa: `R_0x8d0 hal/phydm/phydm_regtable.h:529`; `ODM_REG_CLM_RESULT_11N hal/phydm/phydm_regdefine11n.h:106`; `ODM_REG_NHM_CNT9_TO_CNT8_11N hal/phydm/phydm_regdefine11n.h:111`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_ccx.c:644` | phydm_nhm_get_result | odm_get_bb_reg | 32 | 0xffff0000 | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `value32 = odm_get_bb_reg(dm, R_0x8d0, 0xffff0000);` |
| `hal/phydm/phydm_ccx.c:1782` | phydm_clm_get_result | odm_get_bb_reg | 32 | MASKLWORD | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `val = odm_get_bb_reg(dm, R_0x8d0, MASKLWORD);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10275`): 1 mục, giá trị 0xb800; điều kiện None

## `0x08D4` — St A — `R_0x8d4`, `ODM_REG_NHM_CNT10_TO_CNT11_11N`

Định nghĩa: `R_0x8d4 hal/phydm/phydm_regtable.h:530`; `ODM_REG_NHM_CNT10_TO_CNT11_11N hal/phydm/phydm_regdefine11n.h:112`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_ccx.c:647` | phydm_nhm_get_result | odm_read_4byte | 32 |  | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `value32 = odm_read_4byte(dm, R_0x8d4);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10276`): 1 mục, giá trị 0x860308a0; điều kiện None

## `0x08D8` — St A — `R_0x8d8`, `ODM_REG_NHM_CNT_11N`

Định nghĩa: `R_0x8d8 hal/phydm/phydm_regtable.h:531`; `ODM_REG_NHM_CNT_11N hal/phydm/phydm_regdefine11n.h:107`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1637` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(19) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(19), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1638` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(27) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(27), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1642` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(19) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(19), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1643` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(27) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(27), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1655` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(19) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(19), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1656` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(27) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(27), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1660` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(19) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(19), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1661` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(27) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(27), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1709` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(27) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(27), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1720` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(19) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(19), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1721` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(27) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(27), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1725` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(19) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(19), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1726` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(27) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(27), 0x1);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:68` | phydm_dynamic_switch_htstf_mumimo_8822b | odm_set_bb_reg | 32 | BIT(17) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(17), 0x1);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:70` | phydm_dynamic_switch_htstf_mumimo_8822b | odm_set_bb_reg | 32 | BIT(17) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(17), 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:83` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | BIT(17) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(17), 0x1);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:98` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | BIT(17) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(17), 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:391` | phydm_somlrxhp_setting | odm_set_bb_reg | 32 | BIT(27) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(27), 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:397` | phydm_somlrxhp_setting | odm_set_bb_reg | 32 | BIT(27) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(27), 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:402` | phydm_somlrxhp_setting | odm_set_bb_reg | 32 | BIT(27) | live | `odm_set_bb_reg(dm, R_0x8d8, BIT(27), 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:407` | phydm_somlrxhp_setting | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, 0x8d8, MASKDWORD, 0x29035612);` |
| `hal/phydm/phydm_ccx.c:638` | phydm_nhm_get_result | odm_read_4byte | 32 |  | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `value32 = odm_read_4byte(dm, R_0x8d8);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10277`): 1 mục, giá trị 0x29095612; điều kiện None

## `0x08DC` — St A — `R_0x8dc`, `ODM_REG_EDCCA_POWER_CAL`, `ODM_REG_NHM_CNT7_TO_CNT4_11N`

Định nghĩa: `R_0x8dc hal/phydm/phydm_regtable.h:532`; `ODM_REG_EDCCA_POWER_CAL hal/phydm/phydm_regdefine11ac.h:71`; `ODM_REG_NHM_CNT7_TO_CNT4_11N hal/phydm/phydm_regdefine11n.h:110`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_adaptivity.c:384` | phydm_edcca_decision_opt | odm_set_bb_reg | 32 | BIT(5) | live | `odm_set_bb_reg(dm, R_0x8dc, BIT(5), 0x1);` |
| `hal/phydm/phydm_ccx.c:641` | phydm_nhm_get_result | odm_read_4byte | 32 |  | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `value32 = odm_read_4byte(dm, R_0x8dc);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10278`): 1 mục, giá trị 0x0; điều kiện None

## `0x08E0` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10279`): 1 mục, giá trị 0x32d16777; điều kiện None

## `0x08E4` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10280`): 1 mục, giá trị 0x4c098935; điều kiện None

## `0x08E8` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10281`): 1 mục, giá trị 0xffffc42c; điều kiện None

## `0x08EC` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10282`): 1 mục, giá trị 0x99999999; điều kiện None

## `0x08F0` — St A — `R_0x8f0`

Định nghĩa: `R_0x8f0 hal/phydm/phydm_regtable.h:533`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10283`): 1 mục, giá trị 0x9999; điều kiện None

## `0x08F4` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10284`): 1 mục, giá trị 0xd80fa1; điều kiện None

## `0x08F8` — St A — `R_0x8f8`

Định nghĩa: `R_0x8f8 hal/phydm/phydm_regtable.h:534`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:257` | _iqk_bb_reset_8822b | odm_set_bb_reg | 32 | 0x0ff00000 | live | `odm_set_bb_reg(dm, R_0x8f8, 0x0ff00000, 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1368` | _iqk_tximr_selfcheck_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0x8f8, 0x41400080);` |
| `hal/phydm/phydm_debug.c:116` | phydm_bb_dbg_port_header_sel | odm_set_bb_reg | 32 | 0x3c00000 | live | `odm_set_bb_reg(dm, R_0x8f8, 0x3c00000, header_idx);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10285`): 1 mục, giá trị 0x40000080; điều kiện None

## `0x08FC` — St A — `R_0x8fc`, `ODM_REG_DBG_RPT_11AC`

Định nghĩa: `R_0x8fc hal/phydm/phydm_regtable.h:535`; `ODM_REG_DBG_RPT_11AC hal/phydm/phydm_regdefine11ac.h:72`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:260` | _iqk_bb_reset_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0x8fc, 0x0);` |
| `hal/phydm/phydm_auto_dbg.c:406` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0x000);` |
| `hal/phydm/phydm_auto_dbg.c:407` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:418` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0x007);` |
| `hal/phydm/phydm_auto_dbg.c:419` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:430` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0x204);` |
| `hal/phydm/phydm_auto_dbg.c:431` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:442` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0x278);` |
| `hal/phydm/phydm_auto_dbg.c:443` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:454` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0x290);` |
| `hal/phydm/phydm_auto_dbg.c:455` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:467` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0x2B2);` |
| `hal/phydm/phydm_auto_dbg.c:468` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:479` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0x2B8);` |
| `hal/phydm/phydm_auto_dbg.c:480` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:491` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0xA03);` |
| `hal/phydm/phydm_auto_dbg.c:492` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:503` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0xA0B);` |
| `hal/phydm/phydm_auto_dbg.c:504` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:515` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0xA0D);` |
| `hal/phydm/phydm_auto_dbg.c:516` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:527` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0xAA0);` |
| `hal/phydm/phydm_auto_dbg.c:528` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:539` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0xAB0);` |
| `hal/phydm/phydm_auto_dbg.c:540` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:551` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0xAD0);` |
| `hal/phydm/phydm_auto_dbg.c:552` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:560` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0xAD1);` |
| `hal/phydm/phydm_auto_dbg.c:561` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:569` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0xAD2);` |
| `hal/phydm/phydm_auto_dbg.c:570` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:578` | phydm_dbg_port_dump_ac | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0xAD3);` |
| `hal/phydm/phydm_auto_dbg.c:579` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_DBG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_debug.c:155` | phydm_get_bb_dbg_port_idx | odm_get_bb_reg | 32 | MASKDWORD | live | `val = odm_get_bb_reg(dm, R_0x8fc, MASKDWORD);` |
| `hal/phydm/phydm_debug.c:173` | phydm_set_bb_dbg_port | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x8fc, MASKDWORD, debug_port);` |
| `hal/phydm/phydm_noisemonitor.c:326` | odm_inband_noise_monitor_ac | odm_set_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `odm_set_bb_reg(dm, R_0x8fc, MASKDWORD, 0x200);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10286`): 1 mục, giá trị 0x130; điều kiện None

## `0x0900` — St A — `R_0x900`, `r_ANTSEL_SW_Jaguar`, `REG_ANTSEL_SW_JAGUAR`, `ODM_REG_EDCCA_DOWN_OPT`, `ODM_REG_BB_CTRL_PAGE9_11N`

Định nghĩa: `REG_ANTSEL_SW_JAGUAR include/hal_phy_reg.h:303`; `R_0x900 hal/phydm/phydm_regtable.h:536`; `ODM_REG_EDCCA_DOWN_OPT hal/phydm/phydm_regdefine11ac.h:74`; `ODM_REG_BB_CTRL_PAGE9_11N hal/phydm/phydm_regdefine11n.h:115`; `r_ANTSEL_SW_Jaguar include/rtl8822b_hal.h:203`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10287`): 1 mục, giá trị 0x800000; điều kiện None

## `0x0904` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10288`): 1 mục, giá trị 0x0; điều kiện None

## `0x0908` — St A — `R_0x908`, `ODM_REG_DBG_RPT_11N`

Định nghĩa: `R_0x908 hal/phydm/phydm_regtable.h:537`; `ODM_REG_DBG_RPT_11N hal/phydm/phydm_regdefine11n.h:116`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:159` | phydm_get_bb_dbg_port_idx | odm_get_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `val = odm_get_bb_reg(dm, R_0x908, MASKDWORD);` |
| `hal/phydm/phydm_debug.c:178` | phydm_set_bb_dbg_port | odm_set_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x908, MASKDWORD, debug_port);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10289`): 1 mục, giá trị 0x0; điều kiện None

## `0x090C` — St A — `R_0x90c`, `rFPGA1_TxInfo`, `ODM_R_ANT_SELECT`, `REG_FPGA1_TX_INFO`, `ODM_REG_BB_TX_PATH_11N`

Định nghĩa: `REG_FPGA1_TX_INFO include/hal_phy_reg.h:85`; `REG_FPGA1_TX_INFO include/hal_phy_reg.h:157`; `R_0x90c hal/phydm/phydm_regtable.h:538`; `ODM_REG_BB_TX_PATH_11N hal/phydm/phydm_regdefine11n.h:117`; `ODM_R_ANT_SELECT hal/phydm/phydm_reg.h:109`; `rFPGA1_TxInfo include/rtl8822b_hal.h:144`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:596` | _iqk_configure_macbb_8822b | odm_set_bb_reg | 32 | BIT(15) | live | `odm_set_bb_reg(dm, R_0x90c, BIT(15), 0x1);` |
| `hal/hal_mp.c:1475` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | 0x7fffffff | dead-func | `phy_set_bb_reg(pAdapter, rFPGA1_TxInfo, 0x7fffffff, r_ant_select_ofdm_val);		/*/OFDM Tx*/` |
| `hal/phydm/phydm_api.c:161` | phydm_trx_antenna_setting_init | odm_get_bb_reg | 32 | 0x3 | dead F:dm->support_ic_type & (ODM_RTL8192F \| ODM_RTL8192E \| ODM_RTL | `dm->tx_ant_status = (u8)odm_get_bb_reg(dm, R_0x90c, 0x3);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10290`): 1 mục, giá trị 0xd3000000; điều kiện None

## `0x0910` — St A — `R_0x910`

Định nghĩa: `R_0x910 hal/phydm/phydm_regtable.h:539`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_mp.c:2298` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0xC000 | live | `phy_set_bb_reg(padapter, 0x910, 0xC000, 3);` |
| `hal/phydm/phydm_psd.c:364` | phydm_psd_para_setting | odm_set_bb_reg | 32 | BIT(11) \| BIT(10) | live | `odm_set_bb_reg(dm, R_0x910, BIT(11) \| BIT(10), i_q_setting);` |
| `hal/phydm/phydm_psd.c:365` | phydm_psd_para_setting | odm_set_bb_reg | 32 | BIT(13) \| BIT(12) | live | `odm_set_bb_reg(dm, R_0x910, BIT(13) \| BIT(12), hw_avg_time);` |
| `hal/phydm/phydm_psd.c:366` | phydm_psd_para_setting | odm_set_bb_reg | 32 | BIT(15) \| BIT(14) | live | `odm_set_bb_reg(dm, R_0x910, BIT(15) \| BIT(14),` |
| `hal/phydm/phydm_psd.c:368` | phydm_psd_para_setting | odm_set_bb_reg | 32 | BIT(17) \| BIT(16) | live | `odm_set_bb_reg(dm, R_0x910, BIT(17) \| BIT(16), ant_sel);` |
| `hal/phydm/phydm_psd.c:369` | phydm_psd_para_setting | odm_set_bb_reg | 32 | BIT(23) | live | `odm_set_bb_reg(dm, R_0x910, BIT(23), psd_input);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1453` | phydm_dynamic_spur_det_eliminate | odm_get_bb_reg | 32 | (BIT(15) \| BIT(14) \| B | live | `reg_910_15_12 = odm_get_bb_reg(dm, R_0x910, (BIT(15) \|` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1464` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x910, MASKDWORD,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1468` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x910, MASKDWORD,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1476` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | BIT(22) | live | `odm_set_bb_reg(dm, R_0x910, BIT(22), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1486` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x910, MASKDWORD,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1490` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x910, MASKDWORD,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1499` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | BIT(22) | live | `odm_set_bb_reg(dm, R_0x910, BIT(22), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1505` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | (BIT(15) \| BIT(14) \| B | live | `odm_set_bb_reg(dm, R_0x910, (BIT(15) \| BIT(14) \|` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10291`): 1 mục, giá trị 0xfc00; điều kiện None

## `0x0914` — St A — `R_0x914`, `ODM_REG_ANT_MAPPING1_11N`, `rSingleTone_ContTx_Jaguar`, `REG_SINGLE_TONE_CONT_TX_JAGUAR`

Định nghĩa: `REG_SINGLE_TONE_CONT_TX_JAGUAR include/hal_phy_reg.h:200`; `R_0x914 hal/phydm/phydm_regtable.h:540`; `ODM_REG_ANT_MAPPING1_11N hal/phydm/phydm_regdefine11n.h:118`; `rSingleTone_ContTx_Jaguar include/rtl8822b_hal.h:145`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1733` | hal_mpt_SetSingleCarrierTx | phy_set_bb_reg | 32 | BIT18 \| BIT17 \| BIT16 | live | `phy_set_bb_reg(pAdapter, rSingleTone_ContTx_Jaguar, BIT18 \| BIT17 \| BIT16, OFDM_SingleCarrier);` |
| `hal/hal_mp.c:1744` | hal_mpt_SetSingleCarrierTx | phy_set_bb_reg | 32 | BIT18 \| BIT17 \| BIT16 | live | `phy_set_bb_reg(pAdapter, rSingleTone_ContTx_Jaguar, BIT18 \| BIT17 \| BIT16, OFDM_ALL_OFF);` |
| `hal/hal_mp.c:2041` | hal_mpt_SetCarrierSuppressionTx | phy_set_bb_reg | 32 | BIT18 \| BIT17 \| BIT16 | live | `phy_set_bb_reg(pAdapter, 0x914, BIT18 \| BIT17 \| BIT16, OFDM_ALL_OFF); /* rSingleTone_ContTx_Jaguar*/` |
| `hal/hal_mp.c:2156` | mpt_StopOfdmContTx | phy_set_bb_reg | 32 | BIT18 \| BIT17 \| BIT16 | live | `phy_set_bb_reg(pAdapter, 0x914, BIT18 \| BIT17 \| BIT16, OFDM_ALL_OFF);` |
| `hal/hal_mp.c:2192` | mpt_StartCckContTx | phy_set_bb_reg | 32 | BIT18 \| BIT17 \| BIT16 | live | `phy_set_bb_reg(pAdapter, 0x914, BIT18 \| BIT17 \| BIT16, OFDM_ALL_OFF);` |
| `hal/hal_mp.c:2246` | mpt_StartOfdmContTx | phy_set_bb_reg | 32 | BIT18 \| BIT17 \| BIT16 | live | `phy_set_bb_reg(pAdapter, 0x914, BIT18 \| BIT17 \| BIT16, OFDM_ContinuousTx);` |
| `hal/hal_mp.c:2009` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | BIT18 \| BIT17 \| BIT16 | dead F:IS_HARDWARE_TYPE_JAGUAR(pAdapter) \|\| IS_HARDWARE_TYPE_8822B( | `phy_set_bb_reg(pAdapter, rSingleTone_ContTx_Jaguar, BIT18 \| BIT17 \| BIT16, OFDM_ALL_OFF);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10292`): 1 mục, giá trị 0xc6380000; điều kiện None

## `0x0918` — St A — `R_0x918`, `ODM_REG_ANT_MAPPING2_11N`

Định nghĩa: `R_0x918 hal/phydm/phydm_regtable.h:541`; `ODM_REG_ANT_MAPPING2_11N hal/phydm/phydm_regdefine11n.h:119`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10293`): 1 mục, giá trị 0x1c1028c0; điều kiện None

## `0x091C` — St A — `R_0x91c`

Định nghĩa: `R_0x91c hal/phydm/phydm_regtable.h:542`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10294`): 1 mục, giá trị 0x64b11a1c; điều kiện None

## `0x0920` — St A — `R_0x920`

Định nghĩa: `R_0x920 hal/phydm/phydm_regtable.h:543`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10295`): 1 mục, giá trị 0xe0767233; điều kiện None

## `0x0924` — St A — `R_0x924`

Định nghĩa: `R_0x924 hal/phydm/phydm_regtable.h:544`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10296`): 1 mục, giá trị 0x855a2500; điều kiện None

## `0x0928` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10297`): 1 mục, giá trị 0x4ab0e4e4; điều kiện None

## `0x092C` — St A — `R_0x92c`

Định nghĩa: `R_0x92c hal/phydm/phydm_regtable.h:545`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10298`): 1 mục, giá trị 0xfffeb200; điều kiện None

## `0x0930` — St A — `R_0x930`

Định nghĩa: `R_0x930 hal/phydm/phydm_regtable.h:546`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_mp.c:362` | mpt_InitHWConfig | phy_set_bb_reg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8723B(Adapter) | `phy_set_bb_reg(Adapter, 0x930, bMaskByte0, 0x77);/* 0x930[7:0]=77	  */` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10299`): 1 mục, giá trị 0xfffffffe; điều kiện None

## `0x0934` — St A — `R_0x934`

Định nghĩa: `R_0x934 hal/phydm/phydm_regtable.h:547`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10300`): 1 mục, giá trị 0x1fffff; điều kiện None

## `0x0938` — St A — `R_0x938`

Định nghĩa: `R_0x938 hal/phydm/phydm_regtable.h:548`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10301`): 1 mục, giá trị 0x8480; điều kiện None

## `0x093C` — St A — `REG_BB_TX_PATH_SEL_1_8822B`, `R_0x93c`

Định nghĩa: `R_0x93c hal/phydm/phydm_regtable.h:549`; `REG_BB_TX_PATH_SEL_1_8822B include/rtl8822b_hal.h:147`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2350` | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | 32 | 0xfff00000 | live | `odm_set_bb_reg(dm, R_0x93c, 0xfff00000, 0x001);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2352` | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | 32 | 0xfff00000 | live | `odm_set_bb_reg(dm, R_0x93c, 0xfff00000, 0x002);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2355` | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | 32 | 0xfff00000 | live | `odm_set_bb_reg(dm, R_0x93c, 0xfff00000, 0x001);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2358` | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | 32 | 0xfff00000 | live | `odm_set_bb_reg(dm, R_0x93c, 0xfff00000, 0x002);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2361` | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | 32 | 0xfff00000 | live | `odm_set_bb_reg(dm, R_0x93c, 0xfff00000, 0x043);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2387` | phydm_config_tx_path_8822b | odm_set_bb_reg | 32 | (BIT(19) \| BIT(18)) | live | `odm_set_bb_reg(dm, R_0x93c, (BIT(19) \| BIT(18)), 0x3);` |
| `hal/phydm/txbf/haltxbf8822b.c:1082` | phydm_8822btxbf_rfmode | odm_set_bb_reg | 32 | 0xfff00000 | live | `odm_set_bb_reg(dm, REG_BB_TX_PATH_SEL_1_8822B, 0xfff00000, 0x043);` |
| `hal/phydm/txbf/haltxbf8822b.c:1088` | phydm_8822btxbf_rfmode | odm_set_bb_reg | 32 | 0xfff00000 | live | `odm_set_bb_reg(dm, REG_BB_TX_PATH_SEL_1_8822B, 0xfff00000, 0x1); /*@1SS by path-A*/` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:453` | phydm_config_tx2path_8822b | odm_set_bb_reg | 32 | 0xf00000 | no-caller | `odm_set_bb_reg(dm, R_0x93c, 0xf00000, 0x3);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:455` | phydm_config_tx2path_8822b | odm_set_bb_reg | 32 | 0xf00000 | no-caller | `odm_set_bb_reg(dm, R_0x93c, 0xf00000, 0x1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10302`): 1 mục, giá trị 0xe41c0642; điều kiện None

## `0x0940` — St A — `REG_BB_TX_PATH_SEL_2_8822B`, `R_0x940`

Định nghĩa: `R_0x940 hal/phydm/phydm_regtable.h:550`; `REG_BB_TX_PATH_SEL_2_8822B include/rtl8822b_hal.h:148`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2356` | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | 32 | 0xfff0 | live | `odm_set_bb_reg(dm, R_0x940, 0xfff0, 0x043);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2359` | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | 32 | 0xfff0 | live | `odm_set_bb_reg(dm, R_0x940, 0xfff0, 0x043);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2362` | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | 32 | 0xfff0 | live | `odm_set_bb_reg(dm, R_0x940, 0xfff0, 0x043);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2372` | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | 32 | 0xf0 | live | `odm_set_bb_reg(dm, R_0x940, 0xf0, 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2373` | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | 32 | 0xff00 | live | `odm_set_bb_reg(dm, R_0x940, 0xff00, 0x0);` |
| `hal/phydm/txbf/haltxbf8822b.c:1089` | phydm_8822btxbf_rfmode | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, REG_BB_TX_PATH_SEL_2_8822B, MASKLWORD, 0x430); /*@2SS by path-A,B*/` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10303`): 1 mục, giá trị 0xe470430; điều kiện None

## `0x0944` — St A — `R_0x944`, `ODM_REG_ACBB_EDCCA_ENHANCE`

Định nghĩa: `R_0x944 hal/phydm/phydm_regtable.h:551`; `ODM_REG_ACBB_EDCCA_ENHANCE hal/phydm/phydm_regdefine11ac.h:75`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_adaptivity.c:755` | phydm_adaptivity_init | odm_set_bb_reg | 32 | BIT(29) \| BIT(28) | live | `odm_set_bb_reg(dm, R_0x944, BIT(29) \| BIT(28), 0x1);` |
| `core/rtw_mp.c:361` | mpt_InitHWConfig | phy_set_bb_reg | 32 | BIT1 \| BIT0 | dead F:IS_HARDWARE_TYPE_8723B(Adapter) | `phy_set_bb_reg(Adapter, 0x944, BIT1 \| BIT0, 0x3);	/* 0x944[1:0]=11	 */` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10304`): 1 mục, giá trị 0x0; điều kiện None

## `0x0948` — St A — `R_0x948`, `ODM_REG_RX_DFIR_MOD_97F`, `ODM_REG_EDCCA_DOWN_OPT_11N`

Định nghĩa: `R_0x948 hal/phydm/phydm_regtable.h:552`; `ODM_REG_EDCCA_DOWN_OPT_11N hal/phydm/phydm_regdefine11n.h:120`; `ODM_REG_RX_DFIR_MOD_97F hal/phydm/phydm_regdefine11n.h:121`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:880` | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | 32 | BIT(29) \| BIT(28) | live | `odm_set_bb_reg(dm, R_0x948, BIT(29) \| BIT(28), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:886` | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | 32 | BIT(29) \| BIT(28) | live | `odm_set_bb_reg(dm, R_0x948, BIT(29) \| BIT(28), 0x2);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:892` | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | 32 | BIT(29) \| BIT(28) | live | `odm_set_bb_reg(dm, R_0x948, BIT(29) \| BIT(28), 0x2);` |
| `core/rtw_mp.c:368` | mpt_InitHWConfig | phy_set_mac_reg | 32 | bMaskDWord | dead F:IS_HARDWARE_TYPE_8723B(Adapter) | `phy_set_mac_reg(Adapter, 0x948, bMaskDWord, 0x0);		/* Fix BT can't Tx */` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10305`): 1 mục, giá trị 0xac000000; điều kiện None

## `0x094C` — St A — `R_0x94c`

Định nghĩa: `R_0x94c hal/phydm/phydm_regtable.h:553`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:881` | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | 32 | BIT(29) \| BIT(28) | live | `odm_set_bb_reg(dm, R_0x94c, BIT(29) \| BIT(28), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:887` | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | 32 | BIT(29) \| BIT(28) | live | `odm_set_bb_reg(dm, R_0x94c, BIT(29) \| BIT(28), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:893` | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | 32 | BIT(29) \| BIT(28) | live | `odm_set_bb_reg(dm, R_0x94c, BIT(29) \| BIT(28), 0x2);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10306`): 1 mục, giá trị 0x10000083; điều kiện None

## `0x0950` — St A — `R_0x950`

Định nghĩa: `R_0x950 hal/phydm/phydm_regtable.h:554`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm.c:3337` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0x1ff | dead F:dm->support_ic_type & (ODM_RTL8188F \| ODM_RTL8710B \| ODM_RTL | `odm_set_bb_reg(dm, R_0x950, 0x1ff, offset_i_hex[0]);` |
| `hal/phydm/phydm.c:3338` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0x1ff0000 | dead F:dm->support_ic_type & (ODM_RTL8188F \| ODM_RTL8710B \| ODM_RTL | `odm_set_bb_reg(dm, R_0x950, 0x1ff0000, offset_q_hex[0]);` |
| `hal/phydm/phydm.c:3425` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0x1ff | dead F:dm->support_ic_type & (ODM_RTL8721D \| ODM_RTL8710C) | `odm_set_bb_reg(dm, R_0x950, 0x1ff, offset_i_hex[0]);` |
| `hal/phydm/phydm.c:3426` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0x1ff0000 | dead F:dm->support_ic_type & (ODM_RTL8721D \| ODM_RTL8710C) | `odm_set_bb_reg(dm, R_0x950, 0x1ff0000, offset_q_hex[0]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10307`): 1 mục, giá trị 0x32010080; điều kiện None

## `0x0954` — St A — `R_0x954`

Định nghĩa: `R_0x954 hal/phydm/phydm_regtable.h:555`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10308`): 1 mục, giá trị 0x84510080; điều kiện None

## `0x0958` — St A — `R_0x958`

Định nghĩa: `R_0x958 hal/phydm/phydm_regtable.h:556`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1789` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | BIT(4) | live | `odm_set_bb_reg(dm, R_0x958, BIT(4), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1795` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | BIT(4) | live | `odm_set_bb_reg(dm, R_0x958, BIT(4), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1801` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | BIT(4) | live | `odm_set_bb_reg(dm, R_0x958, BIT(4), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1813` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | BIT(4) | live | `odm_set_bb_reg(dm, R_0x958, BIT(4), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1819` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | BIT(4) | live | `odm_set_bb_reg(dm, R_0x958, BIT(4), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1825` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | BIT(4) | live | `odm_set_bb_reg(dm, R_0x958, BIT(4), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1895` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | 0x1f | live | `odm_set_bb_reg(dm, R_0x958, 0x1f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1920` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | 0x1f | live | `odm_set_bb_reg(dm, R_0x958, 0x1f, 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1923` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | 0x1f | live | `odm_set_bb_reg(dm, R_0x958, 0x1f, 0x2);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1926` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | 0x1f | live | `odm_set_bb_reg(dm, R_0x958, 0x1f, 0x3);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10309`): 1 mục, giá trị 0x1; điều kiện None

## `0x095C` — St A — `R_0x95c`, `odm_adc_trigger_jaguar2`

Định nghĩa: `R_0x95c hal/phydm/phydm_regtable.h:557`; `odm_adc_trigger_jaguar2 hal/phydm/phydm_regdefine11ac.h:76`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_adc_sampling.c:1017` | phydm_la_set_bb_dbg_port | odm_set_bb_reg | 32 | 0x1f | live | `odm_set_bb_reg(dm, R_0x95c, 0x1f, trig_sel);` |
| `hal/phydm/phydm_adc_sampling.c:1068` | phydm_la_set_bb | odm_set_bb_reg | 32 | 0xf00 | live | `odm_set_bb_reg(dm, R_0x95c, 0xf00, dma_type);` |
| `hal/phydm/phydm_adc_sampling.c:1070` | phydm_la_set_bb | odm_set_bb_reg | 32 | BIT(31) | live | `odm_set_bb_reg(dm, R_0x95c, BIT(31), edge);` |
| `hal/phydm/phydm_adc_sampling.c:1071` | phydm_la_set_bb | odm_set_bb_reg | 32 | 0xe0 | live | `odm_set_bb_reg(dm, R_0x95c, 0xe0, smp_rate);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10310`): 1 mục, giá trị 0x4248000; điều kiện None

## `0x0960` — St A — `R_0x960`

Định nghĩa: `R_0x960 hal/phydm/phydm_regtable.h:558`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10311`): 1 mục, giá trị 0x0; điều kiện None

## `0x0964` — St A — `R_0x964`

Định nghĩa: `R_0x964 hal/phydm/phydm_regtable.h:559`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10312`): 1 mục, giá trị 0x0; điều kiện None

## `0x0968` — St A — `R_0x968`

Định nghĩa: `R_0x968 hal/phydm/phydm_regtable.h:560`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10313`): 1 mục, giá trị 0x0; điều kiện None

## `0x096C` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10314`): 1 mục, giá trị 0x0; điều kiện None

## `0x0970` — St A — `R_0x970`

Định nghĩa: `R_0x970 hal/phydm/phydm_regtable.h:561`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_ccx.c:320` | phydm_fahm_set_th_by_igi | odm_set_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x970, MASKDWORD, val);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:535` | phydm_rfe_2t3r | odm_set_bb_reg | 32 | BIT(5) | no-caller | `odm_set_bb_reg(dm, R_0x970, BIT(5), 1); /* RFE_buffer */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:536` | phydm_rfe_2t3r | odm_set_bb_reg | 32 | BIT(10) | no-caller | `odm_set_bb_reg(dm, R_0x970, BIT(10), 1); /* RFE_buffer */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:552` | phydm_rfe_2t4r | odm_set_bb_reg | 32 | BIT(5) | no-caller | `odm_set_bb_reg(dm, R_0x970, BIT(5), 1); /* RFE_buffer */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:553` | phydm_rfe_2t4r | odm_set_bb_reg | 32 | BIT(10) | no-caller | `odm_set_bb_reg(dm, R_0x970, BIT(10), 1); /* RFE_buffer */` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10315`): 1 mục, giá trị 0x1fff; điều kiện None

## `0x0974` — St A — `R_0x974`

Định nghĩa: `R_0x974 hal/phydm/phydm_regtable.h:562`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/btc/halbtc8822bwifionly.c:70` | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | 32 | 0x300 | live | `halwifionly_phy_set_bb_reg(pwifionlycfg, 0x974, 0x300, 0x3);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:554` | phy_set_rf_path_switch_8822b | odm_set_bb_reg | 32 | 0xff | live | `odm_set_bb_reg(dm, R_0x974, 0xff, 0xff);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:310` | phydm_rfe_8822b_init | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0x974, 0x3f, 0x3f);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:311` | phydm_rfe_8822b_init | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0x974, (BIT(11) \| BIT(10)), 0x3);` |
| `hal/phydm/phydm_ccx.c:322` | phydm_fahm_set_th_by_igi | odm_set_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x974, MASKDWORD, val);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10316`): 1 mục, giá trị 0x44000fff; điều kiện None

## `0x0978` — St A — `R_0x978`

Định nghĩa: `R_0x978 hal/phydm/phydm_regtable.h:563`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_ccx.c:166` | phydm_fahm_trigger | odm_set_bb_reg | 32 | 0xff000000 | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x978, 0xff000000, (tgr_period & 0xff));` |
| `hal/phydm/phydm_ccx.c:324` | phydm_fahm_set_th_by_igi | odm_set_bb_reg | 32 | 0xffffff | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x978, 0xffffff, val);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10317`): 1 mục, giá trị 0x0; điều kiện None

## `0x097C` — St A — `R_0x97c`

Định nghĩa: `R_0x97c hal/phydm/phydm_regtable.h:564`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_ccx.c:167` | phydm_fahm_trigger | odm_set_bb_reg | 32 | 0xff | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x97c, 0xff, (tgr_period & 0xff00) >> 8);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10318`): 1 mục, giá trị 0x0; điều kiện None

## `0x0980` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10319`): 1 mục, giá trị 0x0; điều kiện None

## `0x0984` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10320`): 1 mục, giá trị 0x0; điều kiện None

## `0x0988` — St A — `R_0x988`

Định nghĩa: `R_0x988 hal/phydm/phydm_regtable.h:565`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_soml.c:1365` | phydm_adaptive_soml_init | odm_set_bb_reg | 32 | BIT(25) | dead F:dm->support_ic_type & (ODM_RTL8197F \| ODM_RTL8192F) | `odm_set_bb_reg(dm, 0x988, BIT(25), 1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10321`): 1 mục, giá trị 0x0; điều kiện None

## `0x098C` — St A — `R_0x98c`

Định nghĩa: `R_0x98c hal/phydm/phydm_regtable.h:566`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:2894` | phydm_set_weighting_mcc | odm_get_bb_reg | 32 | 0x00ff0000 | live | `temp_reg = odm_get_bb_reg(dm, 0x98c, 0x00ff0000);` |
| `hal/phydm/phydm_api.c:2896` | phydm_set_weighting_mcc | odm_get_bb_reg | 32 | 0xff000000 | live | `temp_reg = odm_get_bb_reg(dm, 0x98c, 0xff000000);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:85` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | 0x7fc0000 | live | `odm_set_bb_reg(dm, R_0x98c, 0x7fc0000, 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:100` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x98c, MASKDWORD, 0x43440000);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:111` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x98c, MASKDWORD, 0x43440000);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:474` | phydm_dynamic_ant_weighting_8822b | odm_set_bb_reg | 32 | 0x7fc0000 | live | `odm_set_bb_reg(dm, R_0x98c, 0x7fc0000, 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:482` | phydm_dynamic_ant_weighting_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x98c, MASKDWORD, 0x43440000);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:491` | phydm_dynamic_ant_weighting_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0x98c, MASKDWORD, 0x43440000);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10322`): 1 mục, giá trị 0x43440000; điều kiện None

## `0x0990` — St A — `R_0x990`, `ODM_REG_CCX_PERIOD_11AC`

Định nghĩa: `R_0x990 hal/phydm/phydm_regtable.h:567`; `ODM_REG_CCX_PERIOD_11AC hal/phydm/phydm_regdefine11ac.h:78`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_ccx.c:1655` | phydm_clm_setting | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0x990, MASKLWORD, clm_period);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10323`): 1 mục, giá trị 0x27100000; điều kiện None

## `0x0994` — St A — `R_0x994`, `ODM_REG_CLM_11AC`, `ODM_REG_NHM_TH9_TH10_11AC`

Định nghĩa: `R_0x994 hal/phydm/phydm_regtable.h:568`; `ODM_REG_NHM_TH9_TH10_11AC hal/phydm/phydm_regdefine11ac.h:79`; `ODM_REG_CLM_11AC hal/phydm/phydm_regdefine11ac.h:80`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10324`): 1 mục, giá trị 0xffff0100; điều kiện None

## `0x0998` — St A — `R_0x998`, `ODM_REG_SOML_97F`, `ODM_REG_NHM_TH3_TO_TH0_11AC`

Định nghĩa: `R_0x998 hal/phydm/phydm_regtable.h:569`; `ODM_REG_NHM_TH3_TO_TH0_11AC hal/phydm/phydm_regdefine11ac.h:81`; `ODM_REG_SOML_97F hal/phydm/phydm_regdefine11n.h:122`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_soml.c:113` | phydm_soml_on_off | odm_set_bb_reg | 32 | BIT(6) | dead F:dm->support_ic_type & (ODM_RTL8197F \| ODM_RTL8192F) | `odm_set_bb_reg(dm, R_0x998, BIT(6), swch);` |
| `hal/phydm/phydm_soml.c:123` | phydm_soml_on_off | odm_set_bb_reg | 32 | BIT(6) | dead F:dm->support_ic_type & (ODM_RTL8197F \| ODM_RTL8192F) | `odm_set_bb_reg(dm, R_0x998, BIT(6), swch);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10325`): 1 mục, giá trị 0xffffff5c; điều kiện None

## `0x099C` — St A — `R_0x99c`, `ODM_REG_NHM_TH7_TO_TH4_11AC`

Định nghĩa: `R_0x99c hal/phydm/phydm_regtable.h:570`; `ODM_REG_NHM_TH7_TO_TH4_11AC hal/phydm/phydm_regdefine11ac.h:82`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10326`): 1 mục, giá trị 0xffffffff; điều kiện None

## `0x09A0` — St A — `R_0x9a0`, `ODM_REG_NHM_TH8_11AC`

Định nghĩa: `R_0x9a0 hal/phydm/phydm_regtable.h:571`; `ODM_REG_NHM_TH8_11AC hal/phydm/phydm_regdefine11ac.h:83`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_adc_sampling.c:1023` | phydm_la_set_bb_dbg_port | odm_set_bb_reg | 32 | 0x1f | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x9a0, 0x1f, trig_sel);` |
| `hal/phydm/phydm_adc_sampling.c:1108` | phydm_la_set_bb | odm_set_bb_reg | 32 | 0xf00 | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x9a0, 0xf00, dma_type);` |
| `hal/phydm/phydm_adc_sampling.c:1110` | phydm_la_set_bb | odm_set_bb_reg | 32 | BIT(31) | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x9a0, BIT(31), edge);` |
| `hal/phydm/phydm_adc_sampling.c:1111` | phydm_la_set_bb | odm_set_bb_reg | 32 | 0xe0 | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0x9a0, 0xe0, smp_rate);` |
| `hal/phydm/phydm_dig.c:1567` | phydm_get_dbg_port_info | odm_get_bb_reg | 32 | BIT(29) | dead F:dm->support_ic_type & ODM_RTL8723D | `val = odm_get_bb_reg(dm, R_0x9a0, BIT(29));` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10327`): 1 mục, giá trị 0xff; điều kiện None

## `0x09A4` — St A — `R_0x9a4`, `ODM_REG_OFDM_FA_RST_11AC`

Định nghĩa: `R_0x9a4 hal/phydm/phydm_regtable.h:572`; `ODM_REG_OFDM_FA_RST_11AC hal/phydm/phydm_regdefine11ac.h:77`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16991` | rtw_reset_phy_rx_counters | phy_set_bb_reg | 32 | BIT17 | live | `phy_set_bb_reg(padapter, 0x9A4, BIT17, 0x1);/* reset  OFDA FA counter */` |
| `hal/hal_com.c:16992` | rtw_reset_phy_rx_counters | phy_set_bb_reg | 32 | BIT17 | live | `phy_set_bb_reg(padapter, 0x9A4, BIT17, 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:317` | _iqk_afe_setting_8822b | odm_set_bb_reg | 32 | BIT(31) | live | `odm_set_bb_reg(dm, R_0x9a4, BIT(31), 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/phydm_dig.c:1329` | phydm_false_alarm_counter_reg_reset | odm_set_bb_reg | 32 | BIT(17) | live | `odm_set_bb_reg(dm, R_0x9a4, BIT(17), 1);` |
| `hal/phydm/phydm_dig.c:1330` | phydm_false_alarm_counter_reg_reset | odm_set_bb_reg | 32 | BIT(17) | live | `odm_set_bb_reg(dm, R_0x9a4, BIT(17), 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10328`): 1 mục, giá trị 0x80000088; điều kiện None

## `0x09A8` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10329`): 1 mục, giá trị 0xc2f0000; điều kiện None

## `0x09AC` — St A — `R_0x9ac`

Định nghĩa: `R_0x9ac hal/phydm/phydm_regtable.h:573`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_direct_bf.c:145` | phydm_dis_cdd | odm_set_bb_reg | 32 | 0x1fff | live | `odm_set_bb_reg(dm, R_0x9ac, 0x1fff, 0);` |
| `hal/phydm/phydm_direct_bf.c:146` | phydm_dis_cdd | odm_set_bb_reg | 32 | BIT(13) | live | `odm_set_bb_reg(dm, R_0x9ac, BIT(13), 1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10330`): 1 mục, giá trị 0x1560000; điều kiện None

## `0x09B0` — St A — `R_0x9b0`

Định nghĩa: `R_0x9b0 hal/phydm/phydm_regtable.h:574`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10331`): 1 mục, giá trị 0x70000000; điều kiện None

## `0x09B4` — St A — `R_0x9b4`, `ODM_REG_CSI_CONTENT_VALUE`

Định nghĩa: `R_0x9b4 hal/phydm/phydm_regtable.h:575`; `ODM_REG_CSI_CONTENT_VALUE hal/phydm/phydm_regdefine11ac.h:85`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10332`): 1 mục, giá trị 0x0; điều kiện None

## `0x09B8` — St A — `R_0x9b8`

Định nghĩa: `R_0x9b8 hal/phydm/phydm_regtable.h:576`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10333`): 1 mục, giá trị 0x0; điều kiện None

## `0x09BC` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10334`): 1 mục, giá trị 0x0; điều kiện None

## `0x09C0` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10335`): 1 mục, giá trị 0x0; điều kiện None

## `0x09C4` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10336`): 1 mục, giá trị 0x0; điều kiện None

## `0x09C8` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10337`): 1 mục, giá trị 0x0; điều kiện None

## `0x09CC` — St A — `R_0x9cc`

Định nghĩa: `R_0x9cc hal/phydm/phydm_regtable.h:577`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10338`): 1 mục, giá trị 0x0; điều kiện None

## `0x09D0` — St A — `R_0x9d0`

Định nghĩa: `R_0x9d0 hal/phydm/phydm_regtable.h:578`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10339`): 1 mục, giá trị 0x0; điều kiện None

## `0x09D4` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10340`): 1 mục, giá trị 0x0; điều kiện None

## `0x09D8` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10341`): 1 mục, giá trị 0x0; điều kiện None

## `0x09DC` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10342`): 1 mục, giá trị 0x0; điều kiện None

## `0x09E0` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10343`): 1 mục, giá trị 0x0; điều kiện None

## `0x09E4` — St A — `R_0x9e4`

Định nghĩa: `R_0x9e4 hal/phydm/phydm_regtable.h:579`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_noisemonitor.c:219` | phydm_idle_noise_measure_ac | odm_set_bb_reg | 32 | BIT(30) | live | `odm_set_bb_reg(dm, R_0x9e4, BIT(30), 0x1);` |
| `hal/phydm/phydm_noisemonitor.c:225` | phydm_idle_noise_measure_ac | odm_set_bb_reg | 32 | BIT(30) | live | `odm_set_bb_reg(dm, R_0x9e4, BIT(30), 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10344`): 1 mục, giá trị 0x2000402; điều kiện None

## `0x09E8` — St A — `R_0x9e8`, `ODM_REG_NHM_9E8_11AC`

Định nghĩa: `R_0x9e8 hal/phydm/phydm_regtable.h:580`; `ODM_REG_NHM_9E8_11AC hal/phydm/phydm_regdefine11ac.h:84`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_direct_bf.c:316` | phydm_set_direct_bfer | odm_set_bb_reg | 32 | 0x2000000 | no-caller T:dm->support_ic_type & ODM_RTL8822B | `odm_set_bb_reg(dm, R_0x9e8, 0x2000000, 1); //abnormal txbf` |
| `hal/phydm/phydm_direct_bf.c:317` | phydm_set_direct_bfer | odm_set_bb_reg | 32 | 0x1000000 | no-caller T:dm->support_ic_type & ODM_RTL8822B | `odm_set_bb_reg(dm, R_0x9e8, 0x1000000, 0); //read phi psi` |
| `hal/phydm/phydm_direct_bf.c:318` | phydm_set_direct_bfer | odm_set_bb_reg | 32 | 0x70000000 | no-caller T:dm->support_ic_type & ODM_RTL8822B | `odm_set_bb_reg(dm, R_0x9e8, 0x70000000, su_idx); //SU user 0` |
| `hal/phydm/phydm_direct_bf.c:356` | phydm_set_direct_bfer | odm_set_bb_reg | 32 | 0x1000000 | no-caller T:dm->support_ic_type & ODM_RTL8822B | `odm_set_bb_reg(dm, 0x9e8, 0x1000000, 0);` |
| `hal/phydm/phydm_direct_bf.c:362` | phydm_set_direct_bfer | odm_set_bb_reg | 32 | 0x1000000 | no-caller T:dm->support_ic_type & ODM_RTL8822B | `odm_set_bb_reg(dm, 0x9e8, 0x1000000, 1);` |
| `hal/phydm/phydm_direct_bf.c:368` | phydm_set_direct_bfer | odm_set_bb_reg | 32 | 0x2000000 | no-caller T:dm->support_ic_type & ODM_RTL8822B | `odm_set_bb_reg(dm, 0x9e8, 0x2000000, 0); //normal txbf` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10345`): 1 mục, giá trị 0x22d4; điều kiện None

## `0x09EC` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_direct_bf.c:360` | phydm_set_direct_bfer | odm_set_bb_reg | 32 | 0xFFFFFFFF | no-caller T:dm->support_ic_type & ODM_RTL8822B | `odm_set_bb_reg(dm, 0x09ec, 0xFFFFFFFF, dw0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10346`): 1 mục, giá trị 0x0; điều kiện None

## `0x09F0` — St A — `R_0x9f0`

Định nghĩa: `R_0x9f0 hal/phydm/phydm_regtable.h:581`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10347`): 1 mục, giá trị 0x10080; điều kiện None

## `0x09F4` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10348`): 1 mục, giá trị 0x0; điều kiện None

## `0x09F8` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10349`): 1 mục, giá trị 0x0; điều kiện None

## `0x09FC` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10350`): 1 mục, giá trị 0xeffff7f7; điều kiện None

## `0x0A00` — St A — `R_0xa00`, `rCCK0_System`, `ODM_REG_CCK_ANTDIV_PARA1_11N`

Định nghĩa: `R_0xa00 hal/phydm/phydm_regtable.h:583`; `ODM_REG_CCK_ANTDIV_PARA1_11N hal/phydm/phydm_regdefine11n.h:125`; `rCCK0_System include/rtl8822b_hal.h:154`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1725` | hal_mpt_SetSingleCarrierTx | phy_set_bb_reg | 32 | bCCKBBMode | live | `phy_set_bb_reg(pAdapter, rCCK0_System, bCCKBBMode, 0);` |
| `hal/hal_mp.c:1728` | hal_mpt_SetSingleCarrierTx | phy_set_bb_reg | 32 | bCCKScramble | live | `phy_set_bb_reg(pAdapter, rCCK0_System, bCCKScramble, 1);` |
| `hal/hal_mp.c:2045` | hal_mpt_SetCarrierSuppressionTx | write_bbreg | 32 | bCCKBBMode | live | `write_bbreg(pAdapter, rCCK0_System, bCCKBBMode, 0x2);    /*/transmit mode*/` |
| `hal/hal_mp.c:2046` | hal_mpt_SetCarrierSuppressionTx | write_bbreg | 32 | bCCKScramble | live | `write_bbreg(pAdapter, rCCK0_System, bCCKScramble, 0x0);  /*/turn off scramble setting*/` |
| `hal/hal_mp.c:2049` | hal_mpt_SetCarrierSuppressionTx | write_bbreg | 32 | bCCKTxRate | live | `write_bbreg(pAdapter, rCCK0_System, bCCKTxRate, 0x0);    /*/Set FTxRate to 1Mbps*/` |
| `hal/hal_mp.c:2059` | hal_mpt_SetCarrierSuppressionTx | write_bbreg | 32 | bCCKBBMode | live | `write_bbreg(pAdapter, rCCK0_System, bCCKBBMode, 0x0);    /*normal mode*/` |
| `hal/hal_mp.c:2060` | hal_mpt_SetCarrierSuppressionTx | write_bbreg | 32 | bCCKScramble | live | `write_bbreg(pAdapter, rCCK0_System, bCCKScramble, 0x1);  /*turn on scramble setting*/` |
| `hal/hal_mp.c:2119` | mpt_StopCckContTx | phy_set_bb_reg | 32 | bCCKBBMode | live | `phy_set_bb_reg(pAdapter, rCCK0_System, bCCKBBMode, 0x0);	/*normal mode*/` |
| `hal/hal_mp.c:2120` | mpt_StopCckContTx | phy_set_bb_reg | 32 | bCCKScramble | live | `phy_set_bb_reg(pAdapter, rCCK0_System, bCCKScramble, 0x1);	/*turn on scramble setting*/` |
| `hal/hal_mp.c:2198` | mpt_StartCckContTx | phy_set_bb_reg | 32 | bCCKTxRate | live | `phy_set_bb_reg(pAdapter, rCCK0_System, bCCKTxRate, cckrate);` |
| `hal/hal_mp.c:2200` | mpt_StartCckContTx | phy_set_bb_reg | 32 | bCCKBBMode | live | `phy_set_bb_reg(pAdapter, rCCK0_System, bCCKBBMode, 0x2);	/*transmit mode*/` |
| `hal/hal_mp.c:2201` | mpt_StartCckContTx | phy_set_bb_reg | 32 | bCCKScramble | live | `phy_set_bb_reg(pAdapter, rCCK0_System, bCCKScramble, 0x1);	/*turn on scramble setting*/` |
| `hal/hal_mp.c:2234` | mpt_StartOfdmContTx | phy_set_bb_reg | 32 | bCCKBBMode | live | `phy_set_bb_reg(pAdapter, rCCK0_System, bCCKBBMode, 0);` |
| `hal/hal_mp.c:2237` | mpt_StartOfdmContTx | phy_set_bb_reg | 32 | bCCKScramble | live | `phy_set_bb_reg(pAdapter, rCCK0_System, bCCKScramble, 1);` |
| `hal/phydm/phydm_api.c:257` | phydm_config_cck_rx_antenna_init | odm_set_bb_reg | 32 | BIT(15) | live | `odm_set_bb_reg(dm, R_0xa00, BIT(15), 0x0); /*@Disable Ant diversity*/` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2124` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(4) | live | `odm_set_bb_reg(dm, R_0xa00, BIT(4), primary_ch_idx);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2126` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(4) | live | `odm_set_bb_reg(dm, R_0xa00, BIT(4), 0);` |
| `core/rtw_mp.c:371` | mpt_InitHWConfig | phy_set_bb_reg | 32 | BIT8 | dead F:IS_HARDWARE_TYPE_8723B(Adapter) | `phy_set_bb_reg(Adapter, 0xA00, BIT8, 0x0);			/*0xA01[0] = 0*/` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10351`): 1 mục, giá trị 0xd047c8; điều kiện None

## `0x0A04` — St A — `R_0xa04`, `rCCK0_AFESetting`, `REG_CCK_0_AFE_SETTING`, `ODM_REG_CCK_ANT_SEL_11N`

Định nghĩa: `REG_CCK_0_AFE_SETTING include/hal_phy_reg.h:127`; `R_0xa04 hal/phydm/phydm_regtable.h:584`; `ODM_REG_CCK_ANT_SEL_11N hal/phydm/phydm_regdefine11n.h:126`; `rCCK0_AFESetting include/rtl8822b_hal.h:155`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:274` | _iqk_bb_reset_8822b | odm_set_bb_reg | 32 | bit_mask | live | `odm_set_bb_reg(dm, R_0xa04, bit_mask, 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/phydm_api.c:305` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(27) \| BIT(26)) | live | `odm_set_bb_reg(dm, R_0xa04, (BIT(27) \| BIT(26)), cck_path[0]);` |
| `hal/phydm/phydm_api.c:307` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(25) \| BIT(24)) | live | `odm_set_bb_reg(dm, R_0xa04, (BIT(25) \| BIT(24)), cck_path[1]);` |
| `hal/phydm/phydm_api.c:336` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(27) \| BIT(26)) | live | `odm_set_bb_reg(dm, R_0xa04, (BIT(27) \| BIT(26)), 0);` |
| `hal/phydm/phydm_api.c:337` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(25) \| BIT(24)) | live | `odm_set_bb_reg(dm, R_0xa04, (BIT(25) \| BIT(24)), 0);` |
| `hal/phydm/phydm_api.c:339` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(27) \| BIT(26)) | live | `odm_set_bb_reg(dm, R_0xa04, (BIT(27) \| BIT(26)), 1);` |
| `hal/phydm/phydm_api.c:340` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(25) \| BIT(24)) | live | `odm_set_bb_reg(dm, R_0xa04, (BIT(25) \| BIT(24)), 1);` |
| `hal/phydm/phydm_api.c:731` | phydm_dis_cck_trx | odm_get_bb_reg | 32 | 0xf0000000 | live | `api->ccktx_path = (u8)odm_get_bb_reg(dm, R_0xa04,` |
| `hal/phydm/phydm_api.c:736` | phydm_dis_cck_trx | odm_set_bb_reg | 32 | 0xf0000000 | live | `odm_set_bb_reg(dm, R_0xa04, 0xf0000000, 0x0);` |
| `hal/phydm/phydm_api.c:756` | phydm_dis_cck_trx | odm_set_bb_reg | 32 | 0xf0000000 | live | `odm_set_bb_reg(dm, R_0xa04, 0xf0000000,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2336` | phydm_config_cck_tx_path_8822b | odm_set_bb_reg | 32 | 0xf0000000 | live | `odm_set_bb_reg(dm, R_0xa04, 0xf0000000, 0x8);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2338` | phydm_config_cck_tx_path_8822b | odm_set_bb_reg | 32 | 0xf0000000 | live | `odm_set_bb_reg(dm, R_0xa04, 0xf0000000, 0x4);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2340` | phydm_config_cck_tx_path_8822b | odm_set_bb_reg | 32 | 0xf0000000 | live | `odm_set_bb_reg(dm, R_0xa04, 0xf0000000, 0xc);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2444` | phydm_config_rx_path_8822b | odm_set_bb_reg | 32 | 0x0f000000 | live | `odm_set_bb_reg(dm, R_0xa04, 0x0f000000, 0x0); /*@00,00*/` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2446` | phydm_config_rx_path_8822b | odm_set_bb_reg | 32 | 0x0f000000 | live | `odm_set_bb_reg(dm, R_0xa04, 0x0f000000, 0x5); /*@01,01*/` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:349` | phydm_dynamic_select_cck_path_8822b | odm_set_bb_reg | 32 | 0x0f000000 | live | `odm_set_bb_reg(dm, R_0xa04, 0x0f000000, 0x0); /*@fix CCK Path A if AP nums > 10*/` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:362` | phydm_dynamic_select_cck_path_8822b | odm_set_bb_reg | 32 | 0x0f000000 | live | `odm_set_bb_reg(dm, R_0xa04, 0x0f000000, 0x5); /*@change to path B collect CCKFA*/` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:368` | phydm_dynamic_select_cck_path_8822b | odm_set_bb_reg | 32 | 0x0f000000 | live | `odm_set_bb_reg(dm, R_0xa04, 0x0f000000, 0x0); /*@FA A<=B choose A*/` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:370` | phydm_dynamic_select_cck_path_8822b | odm_set_bb_reg | 32 | 0x0f000000 | live | `odm_set_bb_reg(dm, R_0xa04, 0x0f000000, 0x5); /*@FA B>A choose B*/` |
| `hal/hal_mp.c:1483` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | bMaskByte3 | dead-func | `phy_set_bb_reg(pAdapter, rCCK0_AFESetting, bMaskByte3, r_ant_select_cck_val);/*/r_ant_sel_cck_val); /CCK TxRx*` |
| `hal/phydm/phydm_api.c:316` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(27) \| BIT(26)) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa04, (BIT(27) \| BIT(26)), 0);` |
| `hal/phydm/phydm_api.c:317` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(25) \| BIT(24)) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa04, (BIT(25) \| BIT(24)), 0);` |
| `hal/phydm/phydm_api.c:322` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(27) \| BIT(26)) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa04, (BIT(27) \| BIT(26)), 1);` |
| `hal/phydm/phydm_api.c:323` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(25) \| BIT(24)) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa04, (BIT(25) \| BIT(24)), 1);` |
| `hal/phydm/phydm_api.c:328` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(27) \| BIT(26)) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa04, (BIT(27) \| BIT(26)), 0);` |
| `hal/phydm/phydm_api.c:329` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(25) \| BIT(24)) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa04, (BIT(25) \| BIT(24)), 1);` |
| `hal/phydm/phydm_api.c:353` | phydm_config_cck_tx_path | odm_set_bb_reg | 32 | 0xf0000000 | no-caller | `odm_set_bb_reg(dm, R_0xa04, 0xf0000000, 0x8);` |
| `hal/phydm/phydm_api.c:355` | phydm_config_cck_tx_path | odm_set_bb_reg | 32 | 0xf0000000 | no-caller | `odm_set_bb_reg(dm, R_0xa04, 0xf0000000, 0x4);` |
| `hal/phydm/phydm_api.c:357` | phydm_config_cck_tx_path | odm_set_bb_reg | 32 | 0xf0000000 | no-caller | `odm_set_bb_reg(dm, R_0xa04, 0xf0000000, 0xc);` |
| `hal/phydm/phydm_api.c:738` | phydm_dis_cck_trx | odm_get_bb_reg | 32 | 0xf0000000 | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `api->ccktx_path = (u8)odm_get_bb_reg(dm, R_0xa04,` |
| `hal/phydm/phydm_api.c:743` | phydm_dis_cck_trx | odm_set_bb_reg | 32 | 0xf0000000 | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0xa04, 0xf0000000, 0x0);` |
| `hal/phydm/phydm_api.c:762` | phydm_dis_cck_trx | odm_set_bb_reg | 32 | 0xf0000000 | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0xa04, 0xf0000000,` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:312` | phydm_1rcca_setting | odm_get_bb_reg | 32 | 0x0f000000 | no-caller | `reg_32 = odm_get_bb_reg(dm, R_0xa04, 0x0f000000);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:327` | phydm_1rcca_setting | odm_set_bb_reg | 32 | 0x0f000000 | no-caller | `odm_set_bb_reg(dm, R_0xa04, 0x0f000000, 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:332` | phydm_1rcca_setting | odm_set_bb_reg | 32 | 0x0f000000 | no-caller | `odm_set_bb_reg(dm, R_0xa04, 0x0f000000, 0x5);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:337` | phydm_1rcca_setting | odm_set_bb_reg | 32 | 0x0f000000 | no-caller | `odm_set_bb_reg(dm, R_0xa04, 0x0f000000, 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:448` | phydm_config_tx2path_8822b | odm_set_bb_reg | 32 | 0xf0000000 | no-caller | `odm_set_bb_reg(dm, R_0xa04, 0xf0000000, 0xc);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:450` | phydm_config_tx2path_8822b | odm_set_bb_reg | 32 | 0xf0000000 | no-caller | `odm_set_bb_reg(dm, R_0xa04, 0xf0000000, 0x8);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10352`): 1 mục, giá trị 0x81ff800c; điều kiện None

## `0x0A08` — St A — `R_0xa08`

Định nghĩa: `R_0xa08 hal/phydm/phydm_regtable.h:585`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:264` | phydm_config_cck_rx_antenna_init | odm_set_bb_reg | 32 | BIT(28) | dead F:dm->support_ic_type & (ODM_RTL8192E \| ODM_RTL8197F \| ODM_RTL | `odm_set_bb_reg(dm, R_0xa08, BIT(28), 1); /*r_cck_2nd_sel_eco*/` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10353`): 1 mục, giá trị 0x8c838300; điều kiện None

## `0x0A0A` — St A — `R_0xa0a`, `ODM_CCK_PD_THRESH`, `ODM_REG_CCK_CCA_11N`, `ODM_REG_CCK_CCA_11AC`

Định nghĩa: `R_0xa0a hal/phydm/phydm_regtable.h:586`; `ODM_REG_CCK_CCA_11AC hal/phydm/phydm_regdefine11ac.h:87`; `ODM_REG_CCK_CCA_11N hal/phydm/phydm_regdefine11n.h:127`; `ODM_CCK_PD_THRESH hal/phydm/phydm_reg.h:111`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_cck_pd.c:75` | phydm_write_cck_pd_type1 | odm_write_1byte | 8 |  | live | `odm_write_1byte(dm, R_0xa0a, cca_th);` |

## `0x0A0C` — St A — `R_0xa0c`, `ODM_REG_CCK_ANTDIV_PARA2_11N`

Định nghĩa: `R_0xa0c hal/phydm/phydm_regtable.h:587`; `ODM_REG_CCK_ANTDIV_PARA2_11N hal/phydm/phydm_regdefine11n.h:128`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_dig.c:632` | odm_write_dig | odm_set_bb_reg | 32 | 0x3f00 | live | `odm_set_bb_reg(dm, R_0xa0c, 0x3f00, (new_igi >> 1));` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10354`): 1 mục, giá trị 0x2e20100f; điều kiện None

## `0x0A10` — St A — `R_0xa10`, `ODM_REG_CCK_ANTDIV_PARA3_11N`

Định nghĩa: `R_0xa10 hal/phydm/phydm_regtable.h:588`; `ODM_REG_CCK_ANTDIV_PARA3_11N hal/phydm/phydm_regdefine11n.h:129`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10355`): 1 mục, giá trị 0x9500bb78; điều kiện None

## `0x0A14` — St A — `R_0xa14`, `ODM_REG_CCK_ANTDIV_PARA4_11N`

Định nghĩa: `R_0xa14 hal/phydm/phydm_regtable.h:589`; `ODM_REG_CCK_ANTDIV_PARA4_11N hal/phydm/phydm_regdefine11n.h:130`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:260` | phydm_config_cck_rx_antenna_init | odm_set_bb_reg | 32 | BIT(7) | live | `odm_set_bb_reg(dm, R_0xa14, BIT(7), 0); /*r_en_mrc_antsel*/` |
| `hal/hal_mp.c:2123` | mpt_StopCckContTx | phy_set_bb_reg | 32 | 0x300 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, 0xa14, 0x300, 0x0);			/* 0xa15[1:0] = 2b00*/` |
| `hal/hal_mp.c:2163` | mpt_StopOfdmContTx | phy_set_bb_reg | 32 | 0x300 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, 0xa14, 0x300, 0x0);			/* 0xa15[1:0] = 0*/` |
| `hal/hal_mp.c:2204` | mpt_StartCckContTx | phy_set_bb_reg | 32 | 0x300 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, 0xa14, 0x300, 0x3);			/* 0xa15[1:0] = 11 force cck rxiq = 0*/` |
| `hal/hal_mp.c:2240` | mpt_StartOfdmContTx | phy_set_bb_reg | 32 | 0x300 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, 0xa14, 0x300, 0x3);			/* 0xa15[1:0] = 2b'11*/` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10356`): 1 mục, giá trị 0x1114d028; điều kiện None

## `0x0A18` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10357`): 1 mục, giá trị 0x881117; điều kiện None

## `0x0A1C` — St A — `rCCK0_DSPParameter2`

Định nghĩa: `rCCK0_DSPParameter2 include/rtl8822b_hal.h:157`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:14945` | SetHwReg | rtw_write8 | 8 |  | live | `rtw_write8(adapter, rCCK0_DSPParameter2, 0xa0);` |
| `hal/hal_com.c:14950` | SetHwReg | rtw_write8 | 8 |  | live | `rtw_write8(adapter, rCCK0_DSPParameter2, 0x00);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10358`): 1 mục, giá trị 0x89140f00; điều kiện None

## `0x0A20` — St A — `R_0xa20`, `rCCK0_TxFilter1`, `ODM_CCK_MATCH_FILTER`

Định nghĩa: `R_0xa20 hal/phydm/phydm_regtable.h:590`; `ODM_CCK_MATCH_FILTER hal/phydm/phydm_reg.h:113`; `rCCK0_TxFilter1 include/rtl8822b_hal.h:158`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:298` | hal_mpt_CCKTxPowerAdjust | read_bbreg | 32 | bMaskHWord | live | `CurrCCKSwingVal = read_bbreg(Adapter, rCCK0_TxFilter1, bMaskHWord);` |
| `hal/hal_mp.c:353` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskHWord | live | `write_bbreg(Adapter, rCCK0_TxFilter1, bMaskHWord, TempVal);` |
| `hal/phydm/phydm_api.c:261` | phydm_config_cck_rx_antenna_init | odm_set_bb_reg | 32 | (BIT(5) \| BIT(4)) | live | `odm_set_bb_reg(dm, R_0xa20, (BIT(5) \| BIT(4)), 1); /*@MBC weighting*/` |
| `hal/hal_mp.c:239` | hal_mpt_CCKTxPowerAdjust | read_bbreg | 32 | bMaskHWord | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `CurrCCKSwingVal = read_bbreg(Adapter, rCCK0_TxFilter1, bMaskHWord);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10359`): 1 mục, giá trị 0x84880010; điều kiện None

## `0x0A22` — St X — `ODM_REG_CCK_FILTER_PARA1_11N`

Định nghĩa: `ODM_REG_CCK_FILTER_PARA1_11N hal/phydm/phydm_regdefine11n.h:131`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:252` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa22, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][0]);` |
| `hal/hal_mp.c:277` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa22, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][0]);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:686` | get_cck_swing_index | odm_read_1byte | 8 |  | dead F:dm->support_ic_type & (ODM_RTL8188E \| ODM_RTL8723B \| ODM_RTL | `bb_cck_swing = odm_read_1byte(dm, 0xa22);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:693` | get_cck_swing_index | odm_read_1byte | 8 |  | dead F:dm->support_ic_type & ODM_RTL8703B | `bb_cck_swing = odm_read_1byte(dm, 0xa22);` |

## `0x0A23` — St X — `ODM_REG_CCK_FILTER_PARA2_11N`

Định nghĩa: `ODM_REG_CCK_FILTER_PARA2_11N hal/phydm/phydm_regdefine11n.h:132`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:253` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa23, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][1]);` |
| `hal/hal_mp.c:278` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa23, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][1]);` |

## `0x0A24` — St A — `R_0xa24`, `rCCK0_TxFilter2`, `ODM_REG_CCK_FILTER_PARA3_11N`

Định nghĩa: `R_0xa24 hal/phydm/phydm_regtable.h:591`; `ODM_REG_CCK_FILTER_PARA3_11N hal/phydm/phydm_regdefine11n.h:133`; `rCCK0_TxFilter2 include/rtl8822b_hal.h:159`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:354` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskDWord | live | `write_bbreg(Adapter, rCCK0_TxFilter2, bMaskDWord, TempVal2);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1904` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0xa24, MASKDWORD, 0x00006577);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1907` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, R_0xa24, MASKDWORD, 0x384f6577);` |
| `hal/hal_mp.c:212` | hal_mpt_CCKTxPowerAdjust | phy_set_bb_reg | 32 | bMaskHWord | dead F:IS_HARDWARE_TYPE_8703B(Adapter) | `phy_set_bb_reg(Adapter, rCCK0_TxFilter2, bMaskHWord, 0);` |
| `hal/hal_mp.c:226` | hal_mpt_CCKTxPowerAdjust | phy_set_bb_reg | 32 | bMaskDWord | dead F:IS_HARDWARE_TYPE_8723D(Adapter) | `phy_set_bb_reg(Adapter, rCCK0_TxFilter2, bMaskDWord, 0x0000B81C);` |
| `hal/hal_mp.c:254` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa24, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][2]);` |
| `hal/hal_mp.c:279` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa24, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][2]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10360`): 1 mục, giá trị 0x384f6577; điều kiện None

## `0x0A25` — St X — `ODM_REG_CCK_FILTER_PARA4_11N`

Định nghĩa: `ODM_REG_CCK_FILTER_PARA4_11N hal/phydm/phydm_regdefine11n.h:134`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:255` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa25, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][3]);` |
| `hal/hal_mp.c:280` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa25, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][3]);` |

## `0x0A26` — St X — `ODM_REG_CCK_FILTER_PARA5_11N`

Định nghĩa: `ODM_REG_CCK_FILTER_PARA5_11N hal/phydm/phydm_regdefine11n.h:135`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:256` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa26, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][4]);` |
| `hal/hal_mp.c:281` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa26, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][4]);` |

## `0x0A27` — St X — `ODM_REG_CCK_FILTER_PARA6_11N`

Định nghĩa: `ODM_REG_CCK_FILTER_PARA6_11N hal/phydm/phydm_regdefine11n.h:136`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:257` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa27, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][5]);` |
| `hal/hal_mp.c:282` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa27, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][5]);` |

## `0x0A28` — St A — `R_0xa28`, `rCCK0_DebugPort`, `ODM_REG_CCK_FILTER_PARA7_11N`

Định nghĩa: `R_0xa28 hal/phydm/phydm_regtable.h:592`; `ODM_REG_CCK_FILTER_PARA7_11N hal/phydm/phydm_regdefine11n.h:137`; `rCCK0_DebugPort include/rtl8822b_hal.h:160`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:355` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskLWord | live | `write_bbreg(Adapter, rCCK0_DebugPort, bMaskLWord, TempVal3);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1905` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xa28, MASKLWORD, 0x0000);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1908` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xa28, MASKLWORD, 0x1525);` |
| `hal/hal_mp.c:213` | hal_mpt_CCKTxPowerAdjust | phy_set_bb_reg | 32 | bMaskLWord | dead F:IS_HARDWARE_TYPE_8703B(Adapter) | `phy_set_bb_reg(Adapter, rCCK0_DebugPort, bMaskLWord, 0);` |
| `hal/hal_mp.c:227` | hal_mpt_CCKTxPowerAdjust | phy_set_bb_reg | 32 | bMaskDWord | dead F:IS_HARDWARE_TYPE_8723D(Adapter) | `phy_set_bb_reg(Adapter, rCCK0_DebugPort, bMaskDWord, 0x00000000);` |
| `hal/hal_mp.c:258` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa28, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][6]);` |
| `hal/hal_mp.c:283` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa28, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][6]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10361`): 1 mục, giá trị 0x1525; điều kiện None

## `0x0A29` — St X — `ODM_REG_CCK_FILTER_PARA8_11N`

Định nghĩa: `ODM_REG_CCK_FILTER_PARA8_11N hal/phydm/phydm_regdefine11n.h:138`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:259` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa29, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][7]);` |
| `hal/hal_mp.c:284` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa29, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][7]);` |

## `0x0A2C` — St A — `R_0xa2c`, `ODM_REG_CCK_FA_RST_11N`, `rCCK0_FalseAlarmReport`, `ODM_REG_CCK_FA_RST_11AC`

Định nghĩa: `R_0xa2c hal/phydm/phydm_regtable.h:593`; `ODM_REG_CCK_FA_RST_11AC hal/phydm/phydm_regdefine11ac.h:88`; `ODM_REG_CCK_FA_RST_11N hal/phydm/phydm_regdefine11n.h:139`; `rCCK0_FalseAlarmReport include/rtl8822b_hal.h:161`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16994` | rtw_reset_phy_rx_counters | phy_set_bb_reg | 32 | BIT15 | live | `phy_set_bb_reg(padapter, 0xA2C, BIT15, 0x0);/* reset  CCK FA counter */` |
| `hal/hal_com.c:16995` | rtw_reset_phy_rx_counters | phy_set_bb_reg | 32 | BIT15 | live | `phy_set_bb_reg(padapter, 0xA2C, BIT15, 0x1);` |
| `hal/phydm/phydm_api.c:311` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | BIT(18) | live | `odm_set_bb_reg(dm, R_0xa2c, BIT(18), en_2R_path);` |
| `hal/phydm/phydm_api.c:313` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | BIT(22) | live | `odm_set_bb_reg(dm, R_0xa2c, BIT(22), en_2R_mrc);` |
| `hal/phydm/phydm_dig.c:1333` | phydm_false_alarm_counter_reg_reset | odm_set_bb_reg | 32 | BIT(15) | live | `odm_set_bb_reg(dm, R_0xa2c, BIT(15), 0);` |
| `hal/phydm/phydm_dig.c:1334` | phydm_false_alarm_counter_reg_reset | odm_set_bb_reg | 32 | BIT(15) | live | `odm_set_bb_reg(dm, R_0xa2c, BIT(15), 1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2438` | phydm_config_rx_path_8822b | odm_set_bb_reg | 32 | BIT(22) | live | `odm_set_bb_reg(dm, R_0xa2c, BIT(22), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2440` | phydm_config_rx_path_8822b | odm_set_bb_reg | 32 | BIT(18) | live | `odm_set_bb_reg(dm, R_0xa2c, BIT(18), 0x0);` |
| `hal/hal_com.c:17006` | rtw_reset_phy_rx_counters | phy_set_bb_reg | 32 | BIT15 | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_set_bb_reg(padapter, 0xA2C, BIT15, 0x0);/* reset  CCK FA counter */` |
| `hal/hal_com.c:17007` | rtw_reset_phy_rx_counters | phy_set_bb_reg | 32 | BIT15 | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_set_bb_reg(padapter, 0xA2C, BIT15, 0x1);` |
| `hal/phydm/phydm_api.c:319` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(18) \| BIT(17)) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa2c, (BIT(18) \| BIT(17)), 0);` |
| `hal/phydm/phydm_api.c:320` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(22) \| BIT(21)) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa2c, (BIT(22) \| BIT(21)), 0);` |
| `hal/phydm/phydm_api.c:325` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(18) \| BIT(17)) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa2c, (BIT(18) \| BIT(17)), 0);` |
| `hal/phydm/phydm_api.c:326` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(22) \| BIT(21)) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa2c, (BIT(22) \| BIT(21)), 0);` |
| `hal/phydm/phydm_api.c:331` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(18) \| BIT(17)) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa2c, (BIT(18) \| BIT(17)), 1);` |
| `hal/phydm/phydm_api.c:332` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | (BIT(22) \| BIT(21)) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa2c, (BIT(22) \| BIT(21)), 1);` |
| `hal/phydm/phydm_dig.c:1358` | phydm_false_alarm_counter_reg_hold | odm_set_bb_reg | 32 | BIT(12) | dead/no-caller F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xa2c, BIT(12), 1);` |
| `hal/phydm/phydm_dig.c:1359` | phydm_false_alarm_counter_reg_hold | odm_set_bb_reg | 32 | BIT(14) | dead/no-caller F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xa2c, BIT(14), 1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10362`): 1 mục, giá trị 0x920000; điều kiện None

## `0x0A2E` — St A — `ODM_CCK_RAKE_MAC`

Định nghĩa: `ODM_CCK_RAKE_MAC hal/phydm/phydm_reg.h:114`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:14942` | SetHwReg | rtw_write8 | 8 |  | live | `rtw_write8(adapter, rCCK0_FalseAlarmReport + 2,` |
| `hal/hal_com.c:14943` | SetHwReg | rtw_read8 | 8 |  | live | `rtw_read8(adapter, rCCK0_FalseAlarmReport + 2) & ~(BIT0));` |
| `hal/hal_com.c:14948` | SetHwReg | rtw_write8 | 8 |  | live | `rtw_write8(adapter, rCCK0_FalseAlarmReport + 2,` |
| `hal/hal_com.c:14949` | SetHwReg | rtw_read8 | 8 |  | live | `rtw_read8(adapter, rCCK0_FalseAlarmReport + 2) \| (BIT0));` |

## `0x0A5B` — St X — `ODM_CCK_FA_CNT_MSB`

Định nghĩa: `ODM_CCK_FA_CNT_MSB hal/phydm/phydm_reg.h:117`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16944` | rtw_dump_phy_rx_counters | rtw_read8 | 8 |  | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `CCK_FA = (rtw_read8(padapter, 0xA5B) << 8) \| (rtw_read8(padapter, 0xA5C));` |

## `0x0A5C` — St A — `ODM_CCK_FA_CNT_LSB`, `ODM_REG_CCK_FA_11AC`, `ODM_REG_CCK_FA_LSB_11N`

Định nghĩa: `ODM_REG_CCK_FA_11AC hal/phydm/phydm_regdefine11ac.h:89`; `ODM_REG_CCK_FA_LSB_11N hal/phydm/phydm_regdefine11n.h:141`; `ODM_CCK_FA_CNT_LSB hal/phydm/phydm_reg.h:118`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16917` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskLWord | live | `CCK_FA	= phy_query_bb_reg(padapter, 0xA5C, bMaskLWord);` |
| `hal/phydm/phydm_dig.c:1496` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKLWORD | live | `fa_t->cnt_cck_fail = odm_get_bb_reg(dm, ODM_REG_CCK_FA_11AC, MASKLWORD);` |
| `hal/hal_com.c:16944` | rtw_dump_phy_rx_counters | rtw_read8 | 8 |  | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `CCK_FA = (rtw_read8(padapter, 0xA5B) << 8) \| (rtw_read8(padapter, 0xA5C));` |

## `0x0A70` — St A — `R_0xa70`

Định nghĩa: `R_0xa70 hal/phydm/phydm_regtable.h:603`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:258` | phydm_config_cck_rx_antenna_init | odm_set_bb_reg | 32 | BIT(7) | live | `odm_set_bb_reg(dm, R_0xa70, BIT(7), 0); /*@Concurrent CCA at LSB & USB*/` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10363`): 1 mục, giá trị 0x101fff00; điều kiện None

## `0x0A74` — St A — `R_0xa74`, `ODM_REG_BB_PWR_SAV4_11N`

Định nghĩa: `R_0xa74 hal/phydm/phydm_regtable.h:604`; `ODM_REG_BB_PWR_SAV4_11N hal/phydm/phydm_regdefine11n.h:143`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:259` | phydm_config_cck_rx_antenna_init | odm_set_bb_reg | 32 | BIT(8) | live | `odm_set_bb_reg(dm, R_0xa74, BIT(8), 0); /*RX path diversity enable*/` |
| `hal/phydm/phydm_api.c:309` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | BIT(8) | live | `odm_set_bb_reg(dm, R_0xa74, BIT(8), path_div_select);` |
| `hal/phydm/phydm_api.c:318` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | BIT(8) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa74, BIT(8), 0);` |
| `hal/phydm/phydm_api.c:324` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | BIT(8) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa74, BIT(8), 0);` |
| `hal/phydm/phydm_api.c:330` | phydm_config_cck_rx_path | odm_set_bb_reg | 32 | BIT(8) | dead F:dm->support_ic_type & ODM_RTL8192F | `odm_set_bb_reg(dm, R_0xa74, BIT(8), 1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10364`): 1 mục, giá trị 0x48; điều kiện None

## `0x0A78` — St A — `R_0xa78`

Định nghĩa: `R_0xa78 hal/phydm/phydm_regtable.h:605`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm.c:3294` | phydm_dc_cancellation | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xa78, MASKBYTE1, 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10365`): 1 mục, giá trị 0x900; điều kiện None

## `0x0A7C` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10366`): 1 mục, giá trị 0x225b0606; điều kiện None

## `0x0A80` — St A — `R_0xa80`

Định nghĩa: `R_0xa80 hal/phydm/phydm_regtable.h:607`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1617` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(18) | live | `odm_set_bb_reg(dm, R_0xa80, BIT(18), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1676` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(18) | live | `odm_set_bb_reg(dm, R_0xa80, BIT(18), 0x1);` |
| `hal/phydm/phydm_phystatus.c:757` | phydm_get_cck_rssi_table_from_reg | odm_get_bb_reg | 32 | 0xFFFF | dead F:!(dm->support_ic_type & ODM_RTL8197F) | `reg_0xa80 = odm_get_bb_reg(dm, R_0xa80, 0xFFFF);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10367`): 1 mục, giá trị 0x218675b2; điều kiện None

## `0x0A84` — St A — `R_0xa84`

Định nghĩa: `R_0xa84 hal/phydm/phydm_regtable.h:608`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2522` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | BIT31 | live | `phy_set_bb_reg(Adapter, 0xA84, BIT31, 0);` |
| `hal/phydm/phydm_api.c:266` | phydm_config_cck_rx_antenna_init | odm_set_bb_reg | 32 | BIT(28) | dead F:dm->support_ic_type & ODM_RTL8814A | `odm_set_bb_reg(dm, R_0xa84, BIT(28), 1); /*@2R CCA only*/` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10368`): 1 mục, giá trị 0x80208c00; điều kiện None

## `0x0A88` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10369`): 1 mục, giá trị 0x40c0000; điều kiện None

## `0x0A8C` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10370`): 1 mục, giá trị 0x12345678; điều kiện None

## `0x0A90` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10371`): 1 mục, giá trị 0xabcdef00; điều kiện None

## `0x0A94` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10372`): 1 mục, giá trị 0x1b1b89; điều kiện None

## `0x0A98` — St A — `R_0xa98`

Định nghĩa: `R_0xa98 hal/phydm/phydm_regtable.h:609`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10373`): 1 mục, giá trị 0x30a0000; điều kiện None

## `0x0A9A` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:260` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa9a, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][8]);` |
| `hal/hal_mp.c:285` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa9a, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][8]);` |

## `0x0A9B` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:261` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa9b, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][9]);` |
| `hal/hal_mp.c:286` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa9b, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][9]);` |

## `0x0A9C` — St A — `R_0xa9c`

Định nghĩa: `R_0xa9c hal/phydm/phydm_regtable.h:610`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm.c:3321` | phydm_dc_cancellation | odm_set_bb_reg | 32 | BIT(20) | live | `odm_set_bb_reg(dm, R_0xa9c, BIT(20), 0x1);` |
| `hal/hal_mp.c:262` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa9c, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][10]);` |
| `hal/hal_mp.c:287` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa9c, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][10]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10374`): 1 mục, giá trị 0x60000; điều kiện None

## `0x0A9D` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:263` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa9d, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][11]);` |
| `hal/hal_mp.c:288` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xa9d, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][11]);` |

## `0x0AA0` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:264` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xaa0, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][12]);` |
| `hal/hal_mp.c:289` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xaa0, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][12]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10375`): 1 mục, giá trị 0x0; điều kiện None

## `0x0AA1` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:265` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xaa1, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][13]);` |
| `hal/hal_mp.c:290` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xaa1, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][13]);` |

## `0x0AA2` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:266` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xaa2, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][14]);` |
| `hal/hal_mp.c:291` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xaa2, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][14]);` |

## `0x0AA3` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:267` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xaa3, bMaskByte0, cck_swing_table_ch1_ch13_88f[CCKSwingIndex][15]);` |
| `hal/hal_mp.c:292` | hal_mpt_CCKTxPowerAdjust | write_bbreg | 32 | bMaskByte0 | dead F:IS_HARDWARE_TYPE_8188F(Adapter) \|\| IS_HARDWARE_TYPE_8188GTV( | `write_bbreg(Adapter, 0xaa3, bMaskByte0, cck_swing_table_ch14_88f[CCKSwingIndex][15]);` |

## `0x0AA4` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10376`): 1 mục, giá trị 0x4000f; điều kiện None

## `0x0AA8` — St A — `R_0xaa8`

Định nghĩa: `R_0xaa8 hal/phydm/phydm_regtable.h:611`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10377`): 1 mục, giá trị 0x200; điều kiện None

## `0x0AAC` — St X — `R_0xaac`

Định nghĩa: `R_0xaac hal/phydm/phydm_regtable.h:612`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:228` | hal_mpt_CCKTxPowerAdjust | phy_set_bb_reg | 32 | bMaskDWord | dead F:IS_HARDWARE_TYPE_8723D(Adapter) | `phy_set_bb_reg(Adapter, 0xAAC, bMaskDWord, 0x00003667);` |

## `0x0AB4` — St X — `R_0xab4`

Định nghĩa: `R_0xab4 hal/phydm/phydm_regtable.h:613`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halphyrf_ce.c:505` | odm_txpowertracking_callback_thermal_meter | odm_get_bb_reg | 32 | 0x000007FF | dead F:dm->support_ic_type & (ODM_RTL8723D \| ODM_RTL8710B) | `regab4 = odm_get_bb_reg(dm, R_0xab4, 0x000007FF);` |

## `0x0ABC` — St X — `R_0xabc`

Định nghĩa: `R_0xabc hal/phydm/phydm_regtable.h:614`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_phystatus.c:758` | phydm_get_cck_rssi_table_from_reg | odm_get_bb_reg | 32 | MASKDWORD | dead F:!(dm->support_ic_type & ODM_RTL8197F) | `reg_0xabc = odm_get_bb_reg(dm, R_0xabc, MASKDWORD);` |

## `0x0B00` — St A — `R_0xb00`

Định nghĩa: `R_0xb00 hal/phydm/phydm_regtable.h:619`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2518` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | BIT8 | live | `phy_set_bb_reg(Adapter, 0xb00, BIT8, 1);		/*	Turn on PMAC*/` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:606` | _iqk_configure_macbb_8822b | odm_set_bb_reg | 32 | BIT(8) | live | `odm_set_bb_reg(dm, R_0xb00, BIT(8), 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10378`): 1 mục, giá trị 0xe1000440; điều kiện None

## `0x0B03` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_noisemonitor.c:359` | odm_inband_noise_monitor_ac | odm_read_1byte | 8 |  | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `val_u8 = odm_read_1byte(dm, 0xB03) & (~BIT(0));` |
| `hal/phydm/phydm_noisemonitor.c:360` | odm_inband_noise_monitor_ac | odm_write_1byte | 8 |  | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `odm_write_1byte(dm, 0xB03, val_u8);` |
| `hal/phydm/phydm_noisemonitor.c:361` | odm_inband_noise_monitor_ac | odm_read_1byte | 8 |  | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `val_u8 = odm_read_1byte(dm, 0xB03) \| BIT(0);` |
| `hal/phydm/phydm_noisemonitor.c:362` | odm_inband_noise_monitor_ac | odm_write_1byte | 8 |  | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `odm_write_1byte(dm, 0xB03, val_u8);` |

## `0x0B04` — St A — `R_0xb04`

Định nghĩa: `R_0xb04 hal/phydm/phydm_regtable.h:620`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2343` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | 0xf | live | `phy_set_bb_reg(Adapter, 0xb04, 0xf, 2);			/*	TX Stop*/` |
| `hal/hal_mp.c:2351` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | 0xf | live | `phy_set_bb_reg(Adapter, 0xb04, 0xf, 2);		/*TX Stop*/` |
| `hal/hal_mp.c:2353` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | 0xf | live | `phy_set_bb_reg(Adapter, 0xb04, 0xf, 2);		/*	TX Stop*/` |
| `hal/hal_mp.c:2521` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | 0xf | live | `phy_set_bb_reg(Adapter, 0xb04, 0xf, 8);		/*TX CCK ON*/` |
| `hal/hal_mp.c:2524` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | 0xf | live | `phy_set_bb_reg(Adapter, 0xb04, 0xf, 4);		/*	TX Ofdm ON	*/` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10379`): 1 mục, giá trị 0x800000; điều kiện None

## `0x0B08` — St A — `R_0xb08`

Định nghĩa: `R_0xb08 hal/phydm/phydm_regtable.h:622`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2413` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb08, bMaskDWord, u4bTmp);	/*	Set 0xb08[23:0] = LSIG, 0xb08[31:24] =  Data init octet` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10380`): 1 mục, giá trị 0xff02030b; điều kiện None

## `0x0B0C` — St A — `R_0xb0c`

Định nghĩa: `R_0xb0c hal/phydm/phydm_regtable.h:623`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2406` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | BIT27 | live | `phy_set_bb_reg(Adapter, 0xb0c, BIT27, 0);` |
| `hal/hal_mp.c:2408` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | BIT27 | live | `phy_set_bb_reg(Adapter, 0xb0c, BIT27, 1);` |
| `hal/hal_mp.c:2423` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb0c, bMaskDWord, u4bTmp);` |
| `hal/hal_mp.c:2428` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb0c, bMaskDWord, u4bTmp);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10381`): 1 mục, giá trị 0x1eaa406; điều kiện None

## `0x0B10` — St A — `R_0xb10`

Định nghĩa: `R_0xb10 hal/phydm/phydm_regtable.h:624`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2425` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | 0xffffff | live | `phy_set_bb_reg(Adapter, 0xb10, 0xffffff, u4bTmp);` |
| `hal/hal_mp.c:2430` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | 0xffffff | live | `phy_set_bb_reg(Adapter, 0xb10, 0xffffff, u4bTmp);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10382`): 1 mục, giá trị 0x30690; điều kiện None

## `0x0B14` — St A — `R_0xb14`

Định nghĩa: `R_0xb14 hal/phydm/phydm_regtable.h:625`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2433` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb14, bMaskDWord, u4bTmp);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10383`): 1 mục, giá trị 0x6000fa; điều kiện None

## `0x0B18` — St A — `R_0xb18`

Định nghĩa: `R_0xb18 hal/phydm/phydm_regtable.h:626`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2410` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | 0xfffff | live | `phy_set_bb_reg(Adapter, 0xb18, 0xfffff, PMacTxInfo.PacketCount);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10384`): 1 mục, giá trị 0x2; điều kiện None

## `0x0B1C` — St A — `R_0xb1c`

Định nghĩa: `R_0xb1c hal/phydm/phydm_regtable.h:627`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2350` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskLWord | live | `phy_set_bb_reg(Adapter, 0xb1c, bMaskLWord, u4bTmp + 50);` |
| `hal/hal_mp.c:2398` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb1c, bMaskDWord, u4bTmp);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10385`): 1 mục, giá trị 0x2; điều kiện None

## `0x0B20` — St A — `R_0xb20`

Định nghĩa: `R_0xb20 hal/phydm/phydm_regtable.h:628`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2438` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb20, bMaskDWord, u4bTmp);` |
| `hal/hal_mp.c:2466` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb20, bMaskDWord, PMacTxInfo.PacketPeriod);	/* for TX interval*/` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10386`): 1 mục, giá trị 0x4b00001f; điều kiện None

## `0x0B24` — St A — `R_0xb24`

Định nghĩa: `R_0xb24 hal/phydm/phydm_regtable.h:629`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2441` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb24, bMaskDWord, u4bTmp);` |
| `hal/hal_mp.c:2468` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb24, bMaskDWord, 0x00000040);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10387`): 1 mục, giá trị 0x4e8e3e40; điều kiện None

## `0x0B28` — St A — `R_0xb28`

Định nghĩa: `R_0xb28 hal/phydm/phydm_regtable.h:630`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2445` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb28, bMaskDWord, 0x00000040);` |
| `hal/hal_mp.c:2473` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb28, bMaskDWord, u4bTmp);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10388`): 1 mục, giá trị 0x3020100; điều kiện None

## `0x0B2C` — St A — `R_0xb2c`, `ODM_REG_LNA_SWITCH_11N`

Định nghĩa: `R_0xb2c hal/phydm/phydm_regtable.h:633`; `ODM_REG_LNA_SWITCH_11N hal/phydm/phydm_regdefine11n.h:145`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2449` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb2C, bMaskDWord, u4bTmp);` |
| `hal/hal_mp.c:2480` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb2c, bMaskDWord, u4bTmp);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10389`): 1 mục, giá trị 0x7060504; điều kiện None

## `0x0B30` — St A — `R_0xb30`, `PATHDIV_REG`, `ODM_REG_PATH_SWITCH_11N`

Định nghĩa: `R_0xb30 hal/phydm/phydm_regtable.h:634`; `ODM_REG_PATH_SWITCH_11N hal/phydm/phydm_regdefine11n.h:146`; `PATHDIV_REG hal/phydm/phydm_reg.h:160`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2456` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb30, bMaskDWord, u4bTmp);` |
| `hal/hal_mp.c:2484` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb30, bMaskDWord, u4bTmp);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10390`): 1 mục, giá trị 0xb0a0908; điều kiện None

## `0x0B34` — St A — `R_0xb34`

Định nghĩa: `R_0xb34 hal/phydm/phydm_regtable.h:635`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2460` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb34, bMaskDWord, u4bTmp);` |
| `hal/hal_mp.c:2476` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb34, bMaskDWord, u4bTmp);` |
| `hal/hal_mp.c:2128` | mpt_StopCckContTx | phy_set_bb_reg | 32 | BIT14 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, 0x0B34, BIT14, 0);` |
| `hal/hal_mp.c:2208` | mpt_StartCckContTx | phy_set_bb_reg | 32 | BIT14 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, 0x0B34, BIT14, 1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10391`): 1 mục, giá trị 0xf0e0d0c; điều kiện None

## `0x0B38` — St A — `R_0xb38`, `ODM_REG_RSSI_CTRL_11N`

Định nghĩa: `R_0xb38 hal/phydm/phydm_regtable.h:636`; `ODM_REG_RSSI_CTRL_11N hal/phydm/phydm_regdefine11n.h:147`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2452` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb38, bMaskDWord, u4bTmp);` |
| `hal/hal_mp.c:2488` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb38, bMaskDWord, u4bTmp);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10392`): 1 mục, giá trị 0x13121110; điều kiện None

## `0x0B3C` — St A — `R_0xb3c`

Định nghĩa: `R_0xb3c hal/phydm/phydm_regtable.h:638`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10393`): 1 mục, giá trị 0x3a; điều kiện None

## `0x0B40` — St A — `R_0xb40`

Định nghĩa: `R_0xb40 hal/phydm/phydm_regtable.h:639`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2401` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(Adapter, 0xb40, bMaskDWord, u4bTmp);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10394`): 1 mục, giá trị 0x0; điều kiện None

## `0x0B44` — St A — `R_0xb44`

Định nghĩa: `R_0xb44 hal/phydm/phydm_regtable.h:640`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2403` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskLWord | live | `phy_set_bb_reg(Adapter, 0xb44, bMaskLWord, u4bTmp);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10395`): 1 mục, giá trị 0x80000000; điều kiện None

## `0x0B48` — St A — `R_0xb48`

Định nghĩa: `R_0xb48 hal/phydm/phydm_regtable.h:641`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2491` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | bMaskByte3 | live | `phy_set_bb_reg(Adapter, 0xb48, bMaskByte3, PMacTxInfo.TX_RATE_HEX);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10396`): 1 mục, giá trị 0x3f0000fa; điều kiện None

## `0x0B4C` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2495` | mpt_ProSetPMacTx | phy_set_bb_reg | 32 | 0x1ff | live | `phy_set_bb_reg(Adapter, 0xb4c, 0x1ff, u4bTmp);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10397`): 1 mục, giá trị 0x88c80020; điều kiện None

## `0x0B50` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10398`): 1 mục, giá trị 0x0; điều kiện None

## `0x0B54` — St A — `R_0xb54`

Định nghĩa: `R_0xb54 hal/phydm/phydm_regtable.h:642`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10399`): 1 mục, giá trị 0x4241; điều kiện None

## `0x0B58` — St A — `R_0xb58`, `ODM_REG_RST_RPT_11AC`

Định nghĩa: `R_0xb58 hal/phydm/phydm_regtable.h:643`; `ODM_REG_RST_RPT_11AC hal/phydm/phydm_regdefine11ac.h:91`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16964` | rtw_reset_phy_trx_ok_counters | phy_set_bb_reg | 32 | BIT0 | live | `phy_set_bb_reg(padapter, 0xB58, BIT0, 0x1);` |
| `hal/hal_com.c:16965` | rtw_reset_phy_trx_ok_counters | phy_set_bb_reg | 32 | BIT0 | live | `phy_set_bb_reg(padapter, 0xB58, BIT0, 0x0);` |
| `hal/phydm/phydm_api.c:77` | phydm_reset_bb_hw_cnt | odm_set_bb_reg | 32 | BIT(0) | live | `odm_set_bb_reg(dm, R_0xb58, BIT(0), 1);` |
| `hal/phydm/phydm_api.c:78` | phydm_reset_bb_hw_cnt | odm_set_bb_reg | 32 | BIT(0) | live | `odm_set_bb_reg(dm, R_0xb58, BIT(0), 0);` |
| `hal/phydm/txbf/haltxbf8822b.c:1115` | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | 32 | BIT(3) \| BIT(2) | live | `odm_set_bb_reg(dm, R_0xb58, BIT(3) \| BIT(2), BW);` |
| `hal/phydm/txbf/haltxbf8822b.c:1116` | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | 32 | BIT(7) \| BIT(6) \| BIT( | live | `odm_set_bb_reg(dm, R_0xb58, BIT(7) \| BIT(6) \| BIT(5) \| BIT(4), 0x0);` |
| `hal/phydm/txbf/haltxbf8822b.c:1117` | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | 32 | BIT(9) \| BIT(8) | live | `odm_set_bb_reg(dm, R_0xb58, BIT(9) \| BIT(8), BW);` |
| `hal/phydm/txbf/haltxbf8822b.c:1118` | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | 32 | BIT(13) \| BIT(12) \| BI | live | `odm_set_bb_reg(dm, R_0xb58, BIT(13) \| BIT(12) \| BIT(11) \| BIT(10), 0x0);` |
| `hal/phydm/phydm_direct_bf.c:299` | phydm_set_direct_bfer | odm_set_bb_reg | 32 | 0x000FFFFC | no-caller T:dm->support_ic_type & ODM_RTL8822B | `odm_set_bb_reg(dm, R_0xb58, 0x000FFFFC, ndp_info);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10400`): 1 mục, giá trị 0xe0008208; điều kiện None

## `0x0B5C` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10401`): 1 mục, giá trị 0x41effff9; điều kiện None

## `0x0B60` — St A — `R_0xb60`

Định nghĩa: `R_0xb60 hal/phydm/phydm_regtable.h:644`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10402`): 1 mục, giá trị 0x0; điều kiện None

## `0x0B64` — St A — `R_0xb64`

Định nghĩa: `R_0xb64 hal/phydm/phydm_regtable.h:645`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10403`): 1 mục, giá trị 0x200063; điều kiện None

## `0x0B68` — St A — `R_0xb68`, `ODM_REG_CONFIG_ANTA_11N`

Định nghĩa: `R_0xb68 hal/phydm/phydm_regtable.h:646`; `ODM_REG_CONFIG_ANTA_11N hal/phydm/phydm_regdefine11n.h:148`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10404`): 1 mục, giá trị 0x3a; điều kiện None

## `0x0B6C` — St A — `R_0xb6c`

Định nghĩa: `R_0xb6c hal/phydm/phydm_regtable.h:649`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10405`): 1 mục, giá trị 0x102; điều kiện None

## `0x0B70` — St A — `R_0xb70`

Định nghĩa: `R_0xb70 hal/phydm/phydm_regtable.h:651`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10406`): 1 mục, giá trị 0x4e6d1870; điều kiện None

## `0x0B74` — St A — `R_0xb74`

Định nghĩa: `R_0xb74 hal/phydm/phydm_regtable.h:652`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10407`): 1 mục, giá trị 0x3020100; điều kiện None

## `0x0B78` — St A — `R_0xb78`

Định nghĩa: `R_0xb78 hal/phydm/phydm_regtable.h:654`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10408`): 1 mục, giá trị 0x7060504; điều kiện None

## `0x0B7C` — St A — `R_0xb7c`

Định nghĩa: `R_0xb7c hal/phydm/phydm_regtable.h:655`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10409`): 1 mục, giá trị 0xb0a0908; điều kiện None

## `0x0B80` — St A — `R_0xb80`

Định nghĩa: `R_0xb80 hal/phydm/phydm_regtable.h:656`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10410`): 1 mục, giá trị 0xf0e0d0c; điều kiện None

## `0x0B84` — St A — `R_0xb84`

Định nghĩa: `R_0xb84 hal/phydm/phydm_regtable.h:657`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10411`): 1 mục, giá trị 0x13121110; điều kiện None

## `0x0B88` — St A — `R_0xb88`

Định nghĩa: `R_0xb88 hal/phydm/phydm_regtable.h:658`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10412`): 1 mục, giá trị 0x0; điều kiện None

## `0x0B8C` — St A — `R_0xb8c`

Định nghĩa: `R_0xb8c hal/phydm/phydm_regtable.h:659`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10413`): 1 mục, giá trị 0x0; điều kiện None

## `0x0BC0` — St X — `R_0xbc0`, `ODM_REG_EDCCA_DCNF_97F`

Định nghĩa: `R_0xbc0 hal/phydm/phydm_regtable.h:670`; `ODM_REG_EDCCA_DCNF_97F hal/phydm/phydm_regdefine11n.h:151`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_adaptivity.c:746` | phydm_adaptivity_init | odm_set_bb_reg | 32 | BIT(27) \| BIT(26) | dead F:dm->support_ic_type & ODM_IC_11N_SERIES && !(dm->support_ic_ | `odm_set_bb_reg(dm, R_0xbc0, BIT(27) \| BIT(26), 0x1);` |

## `0x0C00` — St A — `R_0xc00`, `ODM_OFDM_PHY0_PAGE_C`, `ODM_REG_OFDM_FA_HOLDC_11N`

Định nghĩa: `R_0xc00 hal/phydm/phydm_regtable.h:683`; `ODM_REG_OFDM_FA_HOLDC_11N hal/phydm/phydm_regdefine11n.h:154`; `ODM_OFDM_PHY0_PAGE_C hal/phydm/phydm_reg.h:120`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_mp.c:2215` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_c00 = rtw_read32(padapter, 0xC00);` |
| `core/rtw_mp.c:2216` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x3 | live | `phy_set_bb_reg(padapter, 0xC00, 0x3, 0x00);` |
| `core/rtw_mp.c:2234` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_c00 = rtw_read32(padapter, 0xC00);` |
| `core/rtw_mp.c:2236` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x3 | live | `phy_set_bb_reg(padapter, 0xC00, 0x3, 0x00);` |
| `core/rtw_mp.c:2255` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_c00 = rtw_read32(padapter, 0xC00);` |
| `core/rtw_mp.c:2258` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x3 | live | `phy_set_bb_reg(padapter, 0xC00, 0x3, 0x00);` |
| `core/rtw_mp.c:2275` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_c00 = rtw_read32(padapter, 0xC00);` |
| `core/rtw_mp.c:2279` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x3 | live | `phy_set_bb_reg(padapter, 0xC00, 0x3, 0x00);` |
| `core/rtw_mp.c:2323` | rtw_mp_psd_close | rtw_write32 | 32 |  | live | `rtw_write32(padapter, 0xC00, regs->reg_c00);` |
| `core/rtw_mp.c:2327` | rtw_mp_psd_close | rtw_write32 | 32 |  | live | `rtw_write32(padapter, 0xC00, regs->reg_c00);` |
| `core/rtw_mp.c:2332` | rtw_mp_psd_close | rtw_write32 | 32 |  | live | `rtw_write32(padapter, 0xC00, regs->reg_c00);` |
| `core/rtw_mp.c:2339` | rtw_mp_psd_close | rtw_write32 | 32 |  | live | `rtw_write32(padapter, 0xC00, regs->reg_c00);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:479` | _phy_lc_calibrate_8822b | odm_read_4byte | 32 |  | live | `tmp0xc00 = odm_read_4byte(dm, 0xc00);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:481` | _phy_lc_calibrate_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xc00, 0x4);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:503` | _phy_lc_calibrate_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xc00, tmp0xc00);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:603` | _iqk_configure_macbb_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xc00, 0x00000004);` |
| `hal/phydm/phydm_api.c:574` | phydm_stop_3_wire | odm_set_bb_reg | 32 | 0xf | live | `odm_set_bb_reg(dm, R_0xc00, 0xf, 0x4);` |
| `hal/phydm/phydm_api.c:603` | phydm_stop_3_wire | odm_set_bb_reg | 32 | 0xf | live | `odm_set_bb_reg(dm, R_0xc00, 0xf, 0x7);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1451` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0xc00, MASKBYTE0, 0x4);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1503` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0xc00, MASKBYTE0, 0x7);` |
| `hal/phydm/phydm_dig.c:1353` | phydm_false_alarm_counter_reg_hold | odm_set_bb_reg | 32 | BIT(31) | dead/no-caller F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xc00, BIT(31), 1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10414`): 1 mục, giá trị 0x7; điều kiện None

## `0x0C04` — St A — `R_0xc04`, `ODM_OFDM_RX_ANT`, `rOFDM0_TRxPathEnable`, `ODM_REG_BB_RX_PATH_11N`, `REG_OFDM_0_TRX_PATH_ENABLE`

Định nghĩa: `REG_OFDM_0_TRX_PATH_ENABLE include/hal_phy_reg.h:93`; `REG_OFDM_0_TRX_PATH_ENABLE include/hal_phy_reg.h:165`; `R_0xc04 hal/phydm/phydm_regtable.h:684`; `ODM_REG_BB_RX_PATH_11N hal/phydm/phydm_regdefine11n.h:155`; `ODM_OFDM_RX_ANT hal/phydm/phydm_reg.h:121`; `rOFDM0_TRxPathEnable include/rtl8822b_hal.h:166`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:2880` | phydm_set_weighting_cmn | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, 0xc04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1631` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, R_0xc04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1649` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, R_0xc04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1706` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, R_0xc04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1714` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, R_0xc04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:87` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | BIT(18) | live | `odm_set_bb_reg(dm, R_0xc04, BIT(18), 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:102` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, R_0xc04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:113` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, R_0xc04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/hal_mp.c:1477` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | 0x0000000f | dead-func | `phy_set_bb_reg(pAdapter, rOFDM0_TRxPathEnable, 0x0000000f, r_rx_antenna_ofdm);	/*/OFDM Rx*/` |
| `hal/hal_mp.c:1480` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | 0x000000F0 | dead/dead-func F:IS_HARDWARE_TYPE_8192E(pAdapter) | `phy_set_bb_reg(pAdapter, rOFDM0_TRxPathEnable, 0x000000F0, r_rx_antenna_ofdm);	/*/OFDM Rx*/` |
| `hal/phydm/phydm_api.c:160` | phydm_trx_antenna_setting_init | odm_get_bb_reg | 32 | 0x3 | dead F:dm->support_ic_type & (ODM_RTL8192F \| ODM_RTL8192E \| ODM_RTL | `dm->rx_ant_status = (u8)odm_get_bb_reg(dm, R_0xc04, 0x3);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10415`): 1 mục, giá trị 0x20; điều kiện None

## `0x0C08` — St A — `R_0xc08`, `rOFDM0_TRMuxPar`, `ODM_REG_TRMUX_11N`, `ODM_REG_TRMUX_11AC`, `REG_OFDM_0_TR_MUX_PAR`

Định nghĩa: `REG_OFDM_0_TR_MUX_PAR include/hal_phy_reg.h:92`; `REG_OFDM_0_TR_MUX_PAR include/hal_phy_reg.h:164`; `R_0xc08 hal/phydm/phydm_regtable.h:685`; `ODM_REG_TRMUX_11AC hal/phydm/phydm_regdefine11ac.h:93`; `ODM_REG_TRMUX_11N hal/phydm/phydm_regdefine11n.h:156`; `rOFDM0_TRMuxPar include/rtl8822b_hal.h:167`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2503` | config_phydm_trx_mode_8822b | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xc08, MASKLWORD, 0x3231);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2505` | config_phydm_trx_mode_8822b | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xc08, MASKLWORD, 0x1111);` |
| `hal/hal_mp.c:2124` | mpt_StopCckContTx | phy_set_bb_reg | 32 | 0x10000 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rOFDM0_TRMuxPar, 0x10000, 0x0);		/* 0xc08[16] = 0*/` |
| `hal/hal_mp.c:2164` | mpt_StopOfdmContTx | phy_set_bb_reg | 32 | 0x10000 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rOFDM0_TRMuxPar, 0x10000, 0x0);		/* 0xc08[16] = 0*/` |
| `hal/hal_mp.c:2205` | mpt_StartCckContTx | phy_set_bb_reg | 32 | 0x10000 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rOFDM0_TRMuxPar, 0x10000, 0x1);		/* 0xc08[16] = 1 force ofdm rxiq = ofdm txiq*/` |
| `hal/hal_mp.c:2241` | mpt_StartOfdmContTx | phy_set_bb_reg | 32 | 0x10000 | dead F:!IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rOFDM0_TRMuxPar, 0x10000, 0x1);		/* 0xc08[16] = 1*/` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10416`): 1 mục, giá trị 0x60403231; điều kiện None

## `0x0C0C` — St A — `R_0xc0c`, `ODM_REG_OFDM_FA_RSTC_11N`

Định nghĩa: `R_0xc0c hal/phydm/phydm_regtable.h:686`; `ODM_REG_OFDM_FA_RSTC_11N hal/phydm/phydm_regdefine11n.h:157`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:17002` | rtw_reset_phy_rx_counters | phy_set_bb_reg | 32 | BIT31 | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_set_bb_reg(padapter, 0xC0C, BIT31, 0x1);/* reset  OFDA FA counter */` |
| `hal/hal_com.c:17004` | rtw_reset_phy_rx_counters | phy_set_bb_reg | 32 | BIT31 | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_set_bb_reg(padapter, 0xC0C, BIT31, 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10417`): 1 mục, giá trị 0x12345; điều kiện None

## `0x0C10` — St A — `R_0xc10`, `ODM_REG_DOWNSAM_FACTOR_11N`

Định nghĩa: `R_0xc10 hal/phydm/phydm_regtable.h:687`; `ODM_REG_DOWNSAM_FACTOR_11N hal/phydm/phydm_regdefine11n.h:158`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm.c:3350` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0x3c000000 | live | `odm_set_bb_reg(dm, R_0xc10, 0x3c000000,` |
| `hal/phydm/phydm.c:3352` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0xfc00 | live | `odm_set_bb_reg(dm, R_0xc10, 0xfc00, 0x3f & offset_i_hex[0]);` |
| `hal/phydm/phydm.c:3392` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0xff | dead F:dm->support_ic_type & (ODM_RTL8192F) | `odm_set_bb_reg(dm, R_0xc10, 0xff, offset_i_hex[0]);` |
| `hal/phydm/phydm.c:3393` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0xff00 | dead F:dm->support_ic_type & (ODM_RTL8192F) | `odm_set_bb_reg(dm, R_0xc10, 0xff00, offset_q_hex[0]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10418`): 1 mục, giá trị 0x100; điều kiện None

## `0x0C14` — St A — `R_0xc14`, `ODM_R_A_RXIQI`, `ODM_REG_RXIQI_MATRIX_11N`

Định nghĩa: `R_0xc14 hal/phydm/phydm_regtable.h:688`; `ODM_REG_RXIQI_MATRIX_11N hal/phydm/phydm_regdefine11n.h:159`; `ODM_R_A_RXIQI hal/phydm/phydm_reg.h:122`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm.c:3353` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0x3c000000 | live | `odm_set_bb_reg(dm, R_0xc14, 0x3c000000,` |
| `hal/phydm/phydm.c:3355` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0xfc00 | live | `odm_set_bb_reg(dm, R_0xc14, 0xfc00, 0x3f & offset_q_hex[0]);` |
| `hal/phydm/phydm_api.c:682` | phydm_stop_ic_trx | odm_read_4byte | 32 |  | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `api->rxiqc_reg1 = odm_read_4byte(dm, R_0xc14);` |
| `hal/phydm/phydm_api.c:687` | phydm_stop_ic_trx | odm_set_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0xc14, MASKDWORD, 0x0);` |
| `hal/phydm/phydm_api.c:709` | phydm_stop_ic_trx | odm_write_4byte | 32 |  | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_write_4byte(dm, R_0xc14, api->rxiqc_reg1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10419`): 1 mục, giá trị 0x1000000; điều kiện None

## `0x0C18` — St A — `R_0xc18`

Định nghĩa: `R_0xc18 hal/phydm/phydm_regtable.h:689`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm.c:3411` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0xff | dead F:dm->support_ic_type & (ODM_RTL8192F) | `odm_set_bb_reg(dm, R_0xc18, 0xff, offset_i_hex[1]);` |
| `hal/phydm/phydm.c:3412` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0xff00 | dead F:dm->support_ic_type & (ODM_RTL8192F) | `odm_set_bb_reg(dm, R_0xc18, 0xff00, offset_q_hex[1]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10420`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C1C` — St A — `R_0xc1c`, `rA_TxScale_Jaguar`, `REG_A_TX_SCALE_JAGUAR`

Định nghĩa: `REG_A_TX_SCALE_JAGUAR include/hal_phy_reg.h:70`; `REG_A_TX_SCALE_JAGUAR include/hal_phy_reg.h:113`; `R_0xc1c hal/phydm/phydm_regtable.h:690`; `rA_TxScale_Jaguar include/rtl8822b_hal.h:168`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:925` | mpt_SetSingleTone_8814A | phy_query_bb_reg | 32 | bMaskDWord | live | `regIG0 = phy_query_bb_reg(pAdapter, rA_TxScale_Jaguar, bMaskDWord);		/*/ 0xC1C[31:21]*/` |
| `hal/hal_mp.c:971` | mpt_SetSingleTone_8814A | phy_set_bb_reg | 32 | 0xFFE00000 | live | `phy_set_bb_reg(pAdapter, rA_TxScale_Jaguar, 0xFFE00000, 0); /*/ 0xC1C[31:21]*/` |
| `hal/hal_mp.c:1010` | mpt_SetSingleTone_8814A | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(pAdapter, rA_TxScale_Jaguar, bMaskDWord, regIG0); /* 0xC1C[31:21]*/` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:664` | get_swing_index | odm_get_bb_reg | 32 | 0xFFE00000 | live | `bb_swing = odm_get_bb_reg(dm, R_0xc1c, 0xFFE00000);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1790` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xc1c, (BIT(11) \| BIT(10) \|` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1796` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xc1c, (BIT(11) \| BIT(10) \|` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1802` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xc1c, (BIT(11) \| BIT(10) \|` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1814` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xc1c, (BIT(11) \| BIT(10) \|` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1820` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xc1c, (BIT(11) \| BIT(10) \|` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1826` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xc1c, (BIT(11) \| BIT(10) \|` |
| `hal/phydm/phydm_api.c:683` | phydm_stop_ic_trx | odm_read_4byte | 32 |  | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `api->rxiqc_reg2 = odm_read_4byte(dm, R_0xc1c);` |
| `hal/phydm/phydm_api.c:688` | phydm_stop_ic_trx | odm_set_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0xc1c, MASKDWORD, 0x0);` |
| `hal/phydm/phydm_api.c:710` | phydm_stop_ic_trx | odm_write_4byte | 32 |  | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_write_4byte(dm, R_0xc1c, api->rxiqc_reg2);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10421`): 1 mục, giá trị 0x40040053; điều kiện None

## `0x0C20` — St A — `R_0xc20`

Định nghĩa: `R_0xc20 hal/phydm/phydm_regtable.h:691`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:1004` | phydm_nbi_enable | odm_set_bb_reg | 32 | BIT(28) | live | `odm_set_bb_reg(dm, R_0xc20, BIT(28), val);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:882` | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | 32 | BIT(31) | live | `odm_set_bb_reg(dm, R_0xc20, BIT(31), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:888` | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | 32 | BIT(31) | live | `odm_set_bb_reg(dm, R_0xc20, BIT(31), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:894` | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | 32 | BIT(31) | live | `odm_set_bb_reg(dm, R_0xc20, BIT(31), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1577` | phydm_spur_calibration_8822b | odm_set_bb_reg | 32 | BIT(28) | live | `odm_set_bb_reg(dm, R_0xc20, BIT(28), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1761` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(28) | cfg-off C:*dm->mp_mode | `odm_set_bb_reg(dm, R_0xc20, BIT(28), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2022` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | BIT(28) | cfg-off C:*dm->mp_mode | `odm_set_bb_reg(dm, R_0xc20, BIT(28), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2287` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(28) | cfg-off C:*dm->mp_mode | `odm_set_bb_reg(dm, R_0xc20, BIT(28), 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10422`): 1 mục, giá trị 0x40020103; điều kiện None

## `0x0C24` — St A — `R_0xc24`

Định nghĩa: `R_0xc24 hal/phydm/phydm_regtable.h:692`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10423`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C28` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10424`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C2C` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10425`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C30` — St A — `R_0xc30`, `rOFDM0_RxDetector1`

Định nghĩa: `R_0xc30 hal/phydm/phydm_regtable.h:693`; `rOFDM0_RxDetector1 include/rtl8822b_hal.h:169`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_mp.c:650` | MPT_InitializeAdapter | phy_query_bb_reg | 32 | bMaskByte0 | live | `pMptCtx->backup0xc30 = (u8)phy_query_bb_reg(pAdapter, rOFDM0_RxDetector1, bMaskByte0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10426`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C34` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10427`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C38` — St A — `R_0xc38`

Định nghĩa: `R_0xc38 hal/phydm/phydm_regtable.h:694`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10428`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C3C` — St A — `R_0xc3c`

Định nghĩa: `R_0xc3c hal/phydm/phydm_regtable.h:695`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10429`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C40` — St A — `R_0xc40`

Định nghĩa: `R_0xc40 hal/phydm/phydm_regtable.h:696`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:973` | phydm_set_nbi_reg | odm_set_bb_reg | 32 | 0x1f000000 | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xc40, 0x1f000000, reg_idx);` |
| `hal/phydm/phydm_api.c:999` | phydm_nbi_enable | odm_set_bb_reg | 32 | BIT(9) | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xc40, BIT(9), val);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10430`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C44` — St A — `R_0xc44`

Định nghĩa: `R_0xc44 hal/phydm/phydm_regtable.h:697`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10431`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C48` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10432`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C4C` — St A — `R_0xc4c`, `rOFDM0_ECCAThreshold`, `REG_OFDM_0_ECCA_THRESHOLD`, `ODM_REG_TXIQK_MATRIX_LSB1_11N`

Định nghĩa: `REG_OFDM_0_ECCA_THRESHOLD include/hal_phy_reg.h:65`; `REG_OFDM_0_ECCA_THRESHOLD include/hal_phy_reg.h:90`; `REG_OFDM_0_ECCA_THRESHOLD include/hal_phy_reg.h:162`; `R_0xc4c hal/phydm/phydm_regtable.h:698`; `ODM_REG_TXIQK_MATRIX_LSB1_11N hal/phydm/phydm_regdefine11n.h:160`; `rOFDM0_ECCAThreshold include/rtl8822b_hal.h:170`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_adaptivity.c:219` | phydm_set_edcca_threshold | odm_set_bb_reg | 32 | MASKBYTE0 | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xc4c, MASKBYTE0, (u8)L2H);` |
| `hal/phydm/phydm_adaptivity.c:220` | phydm_set_edcca_threshold | odm_set_bb_reg | 32 | MASKBYTE2 | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xc4c, MASKBYTE2, (u8)H2L);` |
| `hal/phydm/phydm_adaptivity.c:446` | phydm_adaptivity_debug | odm_get_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `reg_value32 = odm_get_bb_reg(dm, R_0xc4c, MASKDWORD);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10433`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C50` — St A — `R_0xc50`, `rA_IGI_Jaguar`, `ODM_REG_IGI_A_11N`, `ODM_R_A_AGC_CORE1`, `rOFDM0_XAAGCCore1`, `ODM_REG_IGI_A_11AC`, `REG_OFDM_0_XA_AGC_CORE1`

Định nghĩa: `REG_OFDM_0_XA_AGC_CORE1 include/hal_phy_reg.h:68`; `REG_OFDM_0_XA_AGC_CORE1 include/hal_phy_reg.h:94`; `REG_OFDM_0_XA_AGC_CORE1 include/hal_phy_reg.h:166`; `R_0xc50 hal/phydm/phydm_regtable.h:699`; `ODM_REG_IGI_A_11AC hal/phydm/phydm_regdefine11ac.h:94`; `ODM_REG_IGI_A_11N hal/phydm/phydm_regdefine11n.h:161`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_mp.c:648` | MPT_InitializeAdapter | phy_query_bb_reg | 32 | bMaskByte0 | live | `pMptCtx->backup0xc50 = (u8)phy_query_bb_reg(pAdapter, rOFDM0_XAAGCCore1, bMaskByte0);` |
| `hal/hal_mcc.c:2748` | mcc_get_reg_hdl | rtw_read32 (qua mảng dbg_reg) | 32 |  | live | `reg_val = rtw_read32(adapter, dbg_reg[i]);` |
| `hal/phydm/phydm_dig.c:542` | phydm_write_dig_reg_c50 | odm_set_bb_reg | 32 | ODM_BIT(IGI, dm) | live | `odm_set_bb_reg(dm, ODM_REG(IGI_A, dm), ODM_BIT(IGI, dm), igi);` |
| `hal/phydm/phydm_dig.c:663` | phydm_get_igi_reg_val | odm_get_bb_reg | 32 | bit_map | live | `val = odm_get_bb_reg(dm, ODM_REG(IGI_A, dm), bit_map);` |
| `hal/phydm/phydm_noisemonitor.c:257` | phydm_idle_noise_measure_ac | odm_get_bb_reg | 32 | MASKBYTE0 | live | `reg_c50 = (u8)odm_get_bb_reg(dm, R_0xc50, MASKBYTE0);` |
| `hal/phydm/phydm_psd.c:243` | phydm_psd | odm_get_bb_reg | 32 | MASKDWORD | live | `odm_get_bb_reg(dm, R_0xc50, MASKDWORD));` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:92` | phydm_igi_toggle_8822b | odm_get_bb_reg | 32 | 0x7f | live | `igi = odm_get_bb_reg(dm, R_0xc50, 0x7f);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:93` | phydm_igi_toggle_8822b | odm_set_bb_reg | 32 | 0x7f | live | `odm_set_bb_reg(dm, R_0xc50, 0x7f, (igi - 2));` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:94` | phydm_igi_toggle_8822b | odm_set_bb_reg | 32 | 0x7f | live | `odm_set_bb_reg(dm, R_0xc50, 0x7f, igi);` |
| `os_dep/linux/ioctl_linux.c:6730` | rtw_dbg_port | rtw_write8 | 8 |  | live | `rtw_write8(padapter, 0xc50, arg);` |
| `os_dep/linux/ioctl_linux.c:6731` | rtw_dbg_port | rtw_read8 | 8 |  | live | `RTW_INFO("wr(0xc50)=0x%x\n", rtw_read8(padapter, 0xc50));` |
| `os_dep/linux/ioctl_linux.c:6736` | rtw_dbg_port | rtw_read8 | 8 |  | live | `RTW_INFO("rd(0xc50)=0x%x\n", rtw_read8(padapter, 0xc50));` |
| `hal/phydm/phydm_api.c:997` | phydm_nbi_enable | odm_set_bb_reg | 32 | 0xf000000 | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xc50, 0xf000000, val);` |

- Bảng `array_mp_8822b_agc_tab` (`halhwimg8822b_bb.c:10129`): 8 mục, giá trị 0x20, 0x22; điều kiện ELSE, ELSE_IF {"rfe": 13}, ELSE_IF {"rfe": 14}
- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10434`): 1 mục, giá trị 0x20; điều kiện None

## `0x0C54` — St A — `R_0xc54`, `ODM_R_A_AGC_CORE2`, `ODM_REG_ANTDIV_PARA2_11N`

Định nghĩa: `R_0xc54 hal/phydm/phydm_regtable.h:700`; `ODM_REG_ANTDIV_PARA2_11N hal/phydm/phydm_regdefine11n.h:162`; `ODM_R_A_AGC_CORE2 hal/phydm/phydm_reg.h:124`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10435`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C58` — St A — `R_0xc58`, `ODM_REG_IGI_B_11N`, `ODM_R_B_AGC_CORE1`, `rOFDM0_XBAGCCore1`, `REG_OFDM_0_XB_AGC_CORE1`

Định nghĩa: `REG_OFDM_0_XB_AGC_CORE1 include/hal_phy_reg.h:69`; `REG_OFDM_0_XB_AGC_CORE1 include/hal_phy_reg.h:97`; `REG_OFDM_0_XB_AGC_CORE1 include/hal_phy_reg.h:169`; `R_0xc58 hal/phydm/phydm_regtable.h:701`; `ODM_REG_IGI_B_11N hal/phydm/phydm_regdefine11n.h:163`; `ODM_R_B_AGC_CORE1 hal/phydm/phydm_reg.h:125`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_mp.c:649` | MPT_InitializeAdapter | phy_query_bb_reg | 32 | bMaskByte0 | live | `pMptCtx->backup0xc58 = (u8)phy_query_bb_reg(pAdapter, rOFDM0_XBAGCCore1, bMaskByte0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:297` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xc58, 0xd8000402);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `os_dep/linux/ioctl_linux.c:6732` | rtw_dbg_port | rtw_write8 | 8 |  | live | `rtw_write8(padapter, 0xc58, arg);` |
| `os_dep/linux/ioctl_linux.c:6733` | rtw_dbg_port | rtw_read8 | 8 |  | live | `RTW_INFO("wr(0xc58)=0x%x\n", rtw_read8(padapter, 0xc58));` |
| `os_dep/linux/ioctl_linux.c:6737` | rtw_dbg_port | rtw_read8 | 8 |  | live | `RTW_INFO("rd(0xc58)=0x%x\n", rtw_read8(padapter, 0xc58));` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10436`): 1 mục, giá trị 0xd8020402; điều kiện None

## `0x0C5C` — St A — `R_0xc5c`, `ODM_REG_ANTDIV_PARA3_11N`

Định nghĩa: `R_0xc5c hal/phydm/phydm_regtable.h:702`; `ODM_REG_ANTDIV_PARA3_11N hal/phydm/phydm_regdefine11n.h:164`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:298` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xc5c, 0xd1000120);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10437`): 1 mục, giá trị 0xde000120; điều kiện None

## `0x0C60` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:293` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xc60, 0x50000000);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:294` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xc60, 0x70070040);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:308` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xc60, 0x50000000);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:309` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xc60, 0x70038040);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10656`): 17 mục, giá trị 0x70038040, 0x70146040, 0x70246040, 0x70346040, 0x70446040, 0x70532040, 0x70646040, 0x70738040 …; điều kiện None

## `0x0C64` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10690`): 9 mục, giá trị 0x4000, 0x800000, 0x800002, 0x800003, 0x800004, 0x800005, 0x800006, 0x8800001 …; điều kiện None

## `0x0C68` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10438`): 1 mục, giá trị 0x5979993f; điều kiện None

## `0x0C6C` — St A — `R_0xc6c`, `ODM_REG_L1SBD_PD_CH_11N`

Định nghĩa: `R_0xc6c hal/phydm/phydm_regtable.h:703`; `ODM_REG_L1SBD_PD_CH_11N hal/phydm/phydm_regdefine11n.h:165`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:299` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xc6c, 0x00000a15);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10439`): 1 mục, giá trị 0x122a; điều kiện None

## `0x0C70` — St A — `R_0xc70`, `ODM_R_AGC_PAR`, `ODM_REG_BB_PWR_SAV2_11N`

Định nghĩa: `R_0xc70 hal/phydm/phydm_regtable.h:704`; `ODM_REG_BB_PWR_SAV2_11N hal/phydm/phydm_regdefine11n.h:166`; `ODM_R_AGC_PAR hal/phydm/phydm_reg.h:126`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10440`): 1 mục, giá trị 0x99795979; điều kiện None

## `0x0C74` — St A — `R_0xc74`, `ODM_REG_BB_AGC_SET_2_11N`

Định nghĩa: `R_0xc74 hal/phydm/phydm_regtable.h:705`; `ODM_REG_BB_AGC_SET_2_11N hal/phydm/phydm_regdefine11n.h:167`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_dig.c:369` | phydm_set_big_jump_step | odm_set_bb_reg | 32 | 0xe | dead F:dm->support_ic_type & (ODM_RTL8197F \| ODM_RTL8192F) | `odm_set_bb_reg(dm, ODM_REG_BB_AGC_SET_2_11N, 0xe, i);` |
| `hal/phydm/phydm_dig.c:813` | phydm_dig_init | odm_get_bb_reg | 32 | MASKLWORD | dead F:dm->support_ic_type & (ODM_RTL8197F \| ODM_RTL8192F) | `ret_value = odm_get_bb_reg(dm, R_0xc74, MASKLWORD);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10441`): 1 mục, giá trị 0x99795979; điều kiện None

## `0x0C78` — St A — `R_0xc78`

Định nghĩa: `R_0xc78 hal/phydm/phydm_regtable.h:706`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10442`): 1 mục, giá trị 0x99799979; điều kiện None

## `0x0C7C` — St A — `R_0xc7c`, `ODM_REG_RX_OFF_11N`, `ODM_R_HTSTF_AGC_PAR`

Định nghĩa: `R_0xc7c hal/phydm/phydm_regtable.h:707`; `ODM_REG_RX_OFF_11N hal/phydm/phydm_regdefine11n.h:168`; `ODM_R_HTSTF_AGC_PAR hal/phydm/phydm_reg.h:127`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10443`): 1 mục, giá trị 0x99791979; điều kiện None

## `0x0C80` — St A — `R_0xc80`, `rOFDM0_XATxIQImbalance`, `ODM_REG_TXIQK_MATRIXA_11N`, `REG_OFDM_0_XA_TX_IQ_IMBALANCE`

Định nghĩa: `REG_OFDM_0_XA_TX_IQ_IMBALANCE include/hal_phy_reg.h:64`; `REG_OFDM_0_XA_TX_IQ_IMBALANCE include/hal_phy_reg.h:96`; `REG_OFDM_0_XA_TX_IQ_IMBALANCE include/hal_phy_reg.h:168`; `R_0xc80 hal/phydm/phydm_regtable.h:708`; `ODM_REG_TXIQK_MATRIXA_11N hal/phydm/phydm_regdefine11n.h:169`; `rOFDM0_XATxIQImbalance include/rtl8822b_hal.h:174`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halphyrf_ce.c:502` | odm_txpowertracking_callback_thermal_meter | odm_get_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & (ODM_RTL8723D \| ODM_RTL8710B) | `regc80 = odm_get_bb_reg(dm, R_0xc80, MASKDWORD);` |
| `hal/phydm/halrf/halrf_powertracking_ce.c:647` | get_swing_index | odm_get_bb_reg | 32 | 0xFFC00000 | dead F:dm->support_ic_type & (ODM_RTL8188E \| ODM_RTL8723B \| ODM_RTL | `bb_swing = odm_get_bb_reg(dm, R_0xc80, 0xFFC00000);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10444`): 1 mục, giá trị 0x19791979; điều kiện None

## `0x0C84` — St A — `R_0xc84`

Định nghĩa: `R_0xc84 hal/phydm/phydm_regtable.h:709`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10445`): 1 mục, giá trị 0x19791979; điều kiện None

## `0x0C88` — St A — `R_0xc88`, `ODM_REG_TXIQK_MATRIXB_11N`

Định nghĩa: `R_0xc88 hal/phydm/phydm_regtable.h:710`; `ODM_REG_TXIQK_MATRIXB_11N hal/phydm/phydm_regdefine11n.h:170`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10446`): 1 mục, giá trị 0x0; điều kiện None

## `0x0C8C` — St A — `R_0xc8c`

Định nghĩa: `R_0xc8c hal/phydm/phydm_regtable.h:711`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10447`): 1 mục, giá trị 0x7000000; điều kiện None

## `0x0C90` — St A — `R_0xc90`, `rA_LSSIWrite_Jaguar`, `ODM_TX_PWR_TRAINING_A`

Định nghĩa: `R_0xc90 hal/phydm/phydm_regtable.h:712`; `ODM_TX_PWR_TRAINING_A hal/phydm/phydm_reg.h:128`; `rA_LSSIWrite_Jaguar include/rtl8822b_hal.h:175`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1061` | config_phydm_write_rf_reg_8822b | odm_set_bb_reg (qua mảng offset_write_rf) | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, offset_write_rf[path], MASKDWORD, data_and_addr);` |

## `0x0C94` — St A — `R_0xc94`, `ODM_REG_TXIQK_MATRIXA_LSB2_11N`

Định nghĩa: `R_0xc94 hal/phydm/phydm_regtable.h:713`; `ODM_REG_TXIQK_MATRIXA_LSB2_11N hal/phydm/phydm_regdefine11n.h:171`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:598` | _iqk_configure_macbb_8822b | odm_set_bb_reg | 32 | BIT(0) | live | `odm_set_bb_reg(dm, R_0xc94, BIT(0), 0x1);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:600` | _iqk_configure_macbb_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xc94, (BIT(11) \| BIT(10)), 0x1);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1007` | _iqk_one_shot_8822b | odm_set_bb_reg (qua mảng iqk_apply) | 32 | BIT(0) | live | `odm_set_bb_reg(dm, iqk_apply[path], BIT(0), 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1021` | _iqk_one_shot_8822b | odm_set_bb_reg (qua mảng iqk_apply) | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, iqk_apply[path],` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:470` | _iqk_reload_iqk_setting_8822b | odm_set_bb_reg (qua mảng iqk_apply) | 32 | BIT(0) | cfg-off-func | `odm_set_bb_reg(dm, iqk_apply[path],` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:474` | _iqk_reload_iqk_setting_8822b | odm_set_bb_reg (qua mảng iqk_apply) | 32 | BIT(10) | cfg-off-func | `odm_set_bb_reg(dm, iqk_apply[path],` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10448`): 1 mục, giá trị 0x1000100; điều kiện None

## `0x0C98` — St A — `ODM_TX_PWR_TRAINING_B`

Định nghĩa: `ODM_TX_PWR_TRAINING_B hal/phydm/phydm_reg.h:129`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10449`): 1 mục, giá trị 0x201c8000; điều kiện None

## `0x0C9C` — St A — `R_0xc9c`, `ODM_REG_TXIQK_MATRIXB_LSB2_11N`

Định nghĩa: `R_0xc9c hal/phydm/phydm_regtable.h:714`; `ODM_REG_TXIQK_MATRIXB_LSB2_11N hal/phydm/phydm_regdefine11n.h:172`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10450`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CA0` — St A — `R_0xca0`, `ODM_REG_RXIQK_MATRIX_LSB_11N`

Định nghĩa: `R_0xca0 hal/phydm/phydm_regtable.h:715`; `ODM_REG_RXIQK_MATRIX_LSB_11N hal/phydm/phydm_regdefine11n.h:173`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:138` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xf050);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:142` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xf055);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:146` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xf550);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:172` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa501);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:176` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa500);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:180` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa005);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:225` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa501);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:229` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa500);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:233` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa005);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:237` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa5a5);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:346` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xf050);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:350` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xf055);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:354` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xf550);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:375` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa501);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:379` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa500);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:383` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa005);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:413` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa555);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:454` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa501);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:458` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa500);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:462` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa005);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:498` | phydm_rfe_ifem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa501);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:502` | phydm_rfe_ifem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa500);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:506` | phydm_rfe_ifem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa005);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:510` | phydm_rfe_ifem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa5a5);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:633` | phydm_rfe_smtant | odm_set_bb_reg | 32 | MASKLWORD | no-caller | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa501);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:637` | phydm_rfe_smtant | odm_set_bb_reg | 32 | MASKLWORD | no-caller | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa500);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:641` | phydm_rfe_smtant | odm_set_bb_reg | 32 | MASKLWORD | no-caller | `odm_set_bb_reg(dm, R_0xca0, MASKLWORD, 0xa005);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10451`): 1 mục, giá trị 0xa555; điều kiện None

## `0x0CA4` — St A — `R_0xca4`, `ODM_REG_ANTDIV_PARA1_11N`

Định nghĩa: `R_0xca4 hal/phydm/phydm_regtable.h:716`; `ODM_REG_ANTDIV_PARA1_11N hal/phydm/phydm_regdefine11n.h:174`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10452`): 1 mục, giá trị 0x8040201; điều kiện None

## `0x0CA8` — St A — `R_0xca8`

Định nghĩa: `R_0xca8 hal/phydm/phydm_regtable.h:717`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10453`): 1 mục, giá trị 0x80402010; điều kiện None

## `0x0CAC` — St A — `R_0xcac`

Định nghĩa: `R_0xcac hal/phydm/phydm_regtable.h:718`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10454`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CB0` — St A — `R_0xcb0`, `rA_RFE_Pinmux_Jaguar`, `REG_A_RFE_PINMUX_JAGUAR`

Định nghĩa: `REG_A_RFE_PINMUX_JAGUAR include/hal_phy_reg.h:107`; `R_0xcb0 hal/phydm/phydm_regtable.h:719`; `rA_RFE_Pinmux_Jaguar include/rtl8822b_hal.h:195`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1850` | hal_mpt_SetSingleToneTx | phy_query_bb_reg | 32 | bMaskDWord | live | `regBB0 = phy_query_bb_reg(pAdapter, rA_RFE_Pinmux_Jaguar, bMaskDWord);` |
| `hal/hal_mp.c:1875` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(pAdapter, rA_RFE_Pinmux_Jaguar, bMaskDWord, 0x77777777);  /* 0xCB0=0x77777777*/` |
| `hal/hal_mp.c:1991` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(pAdapter, rA_RFE_Pinmux_Jaguar, bMaskDWord, regBB0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:517` | _iqk_rfe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xcb0, 0x77777777);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:529` | _iqk_rfe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xcb0, 0x77777777);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:104` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x745774);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:110` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x177517);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:118` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x177517);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:191` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x777777);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:198` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x777747);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:205` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x777747);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:331` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x745774);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:360` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x477547);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:398` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x704570);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:403` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x174517);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:424` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x705770);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:432` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x177517);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:475` | phydm_rfe_ifem | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x745774);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:481` | phydm_rfe_ifem | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x477547);` |
| `hal/hal_mp.c:1882` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xF0F0 | dead F:IS_HARDWARE_TYPE_8821C(pAdapter) | `phy_set_bb_reg(pAdapter, rA_RFE_Pinmux_Jaguar, 0xF0F0, 0x707);  /* 0xCB0[[15:12, 7:4] = 0x707*/` |
| `hal/hal_mp.c:1895` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xFF00F0 | dead F:IS_HARDWARE_TYPE_8822B(pAdapter) | `phy_set_bb_reg(pAdapter, rA_RFE_Pinmux_Jaguar, 0xFF00F0, 0x77007);  /*/ 0xCB0[[23:16, 7:4] = 0x77007*/` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:282` | phydm_rfe_8822b_setting | odm_set_bb_reg | 32 | ((0xf) << (rfe_n * 4)) | no-caller | `odm_set_bb_reg(dm, R_0xcb0, ((0xf) << (rfe_n * 4)),` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:544` | phydm_rfe_2t3r | odm_set_bb_reg | 32 | 0xF00000 | no-caller | `odm_set_bb_reg(dm, R_0xcb0, 0xF00000, 0x8); /* path-A, RFE_CTRL_5 */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:562` | phydm_rfe_2t4r | odm_set_bb_reg | 32 | 0xF00000 | no-caller | `odm_set_bb_reg(dm, R_0xcb0, 0xF00000, 0x8); /* path-A, RFE_CTRL_5 */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:600` | phydm_rfe_smtant | odm_set_bb_reg | 32 | MASKDWORD | no-caller | `odm_set_bb_reg(dm, R_0xcb0, MASKDWORD, 0x77178519);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:717` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x745774);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:722` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x777777);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:730` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x477547);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:735` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x777777);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:750` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x705770);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:755` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x777777);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:763` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x177517);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:768` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xcb0, 0xffffff, 0x777777);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10455`): 1 mục, giá trị 0x77777777; điều kiện None

## `0x0CB4` — St A — `R_0xcb4`, `rA_RFE_Inv_Jaguar`

Định nghĩa: `R_0xcb4 hal/phydm/phydm_regtable.h:720`; `rA_RFE_Inv_Jaguar include/rtl8822b_hal.h:197`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/btc/halbtc8822bwifionly.c:68` | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | 32 | 0xff | live | `halwifionly_phy_set_bb_reg(pwifionlycfg, 0xcb4, 0xff, 0x77);` |
| `hal/hal_mp.c:1852` | hal_mpt_SetSingleToneTx | phy_query_bb_reg | 32 | bMaskDWord | live | `regBB2 = phy_query_bb_reg(pAdapter, rA_RFE_Pinmux_Jaguar + 4, bMaskDWord);` |
| `hal/hal_mp.c:1877` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bMaskLWord | live | `phy_set_bb_reg(pAdapter, rA_RFE_Pinmux_Jaguar + 4, bMaskLWord, 0x7777);  /* 0xCB4[15:0] = 0x7777*/` |
| `hal/hal_mp.c:1993` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(pAdapter, rA_RFE_Pinmux_Jaguar + 4, bMaskDWord, regBB2);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:566` | phy_set_rf_path_switch_8822b | odm_set_bb_reg | 32 | 0xff | live | `odm_set_bb_reg(dm, R_0xcb4, 0xff, 0x77);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:518` | _iqk_rfe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xcb4, 0x00007777);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:530` | _iqk_rfe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xcb4, 0x00007777);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:106` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:112` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xcb4, MASKLWORD, 0x7577);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:120` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xcb4, MASKLWORD, 0x7577);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:193` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x77);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:200` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE0, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:207` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE0, 0x75);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:333` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:362` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x75);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:400` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x45);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:405` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x45);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:426` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:434` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x75);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:477` | phydm_rfe_ifem | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:483` | phydm_rfe_ifem | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x75);` |
| `hal/hal_mp.c:1886` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xA00000 | dead F:IS_HARDWARE_TYPE_8821C(pAdapter) | `phy_set_bb_reg(pAdapter, rA_RFE_Pinmux_Jaguar + 4, 0xA00000, 0x1); /* 0xCB4[23, 21] = 0x1*/` |
| `hal/hal_mp.c:1890` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xA00000 | dead F:IS_HARDWARE_TYPE_8821C(pAdapter) | `phy_set_bb_reg(pAdapter, rA_RFE_Pinmux_Jaguar + 4, 0xA00000, 0x1); /* 0xCB4[23, 21] = 0x1*/` |
| `hal/hal_mp.c:1899` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xFF00000 | dead F:IS_HARDWARE_TYPE_8822B(pAdapter) | `phy_set_bb_reg(pAdapter, rA_RFE_Pinmux_Jaguar + 4, 0xFF00000, 0x12); /*/ 0xCB4[23:16] = 0x12*/` |
| `hal/hal_mp.c:1902` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xFF00000 | dead F:IS_HARDWARE_TYPE_8822B(pAdapter) | `phy_set_bb_reg(pAdapter, rA_RFE_Pinmux_Jaguar + 4, 0xFF00000, 0x11); /*/ 0xCB4[23:16] = 0x11*/` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:287` | phydm_rfe_8822b_setting | odm_set_bb_reg | 32 | ((0xf) << ((rfe_n - 8) * | no-caller | `odm_set_bb_reg(dm, R_0xcb4, ((0xf) << ((rfe_n - 8) * 4)),` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:545` | phydm_rfe_2t3r | odm_set_bb_reg | 32 | 0xF00 | no-caller | `odm_set_bb_reg(dm, R_0xcb4, 0xF00, 0x8); /* path-A, RFE_CTRL_10 */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:605` | phydm_rfe_smtant | odm_set_bb_reg | 32 | MASKDWORD | no-caller | `odm_set_bb_reg(dm, R_0xcb4, MASKDWORD, 0x757a);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:719` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:724` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x77);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:732` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x75);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:737` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x77);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:752` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:757` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x77);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:765` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x75);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:770` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xcb4, MASKBYTE1, 0x77);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10456`): 1 mục, giá trị 0x7777; điều kiện None

## `0x0CB8` — St A — `R_0xcb8`, `rA_RFE_Jaguar`

Định nghĩa: `R_0xcb8 hal/phydm/phydm_regtable.h:721`; `rA_RFE_Jaguar include/rtl8822b_hal.h:199`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:114` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | BIT(5) | live | `odm_set_bb_reg(dm, R_0xcb8, BIT(5), 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:122` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | BIT(5) | live | `odm_set_bb_reg(dm, R_0xcb8, BIT(5), 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:428` | phydm_rfe_efem | odm_set_bb_reg | 32 | BIT(4) | live | `odm_set_bb_reg(dm, R_0xcb8, BIT(4), 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:436` | phydm_rfe_efem | odm_set_bb_reg | 32 | BIT(5) | live | `odm_set_bb_reg(dm, R_0xcb8, BIT(5), 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:621` | phydm_rfe_smtant | odm_set_bb_reg | 32 | BIT(5) | no-caller | `odm_set_bb_reg(dm, R_0xcb8, BIT(5), 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10457`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CBC` — St A — `R_0xcbc`, `rA_RFE_Inverse_Jaguar`

Định nghĩa: `R_0xcbc hal/phydm/phydm_regtable.h:722`; `rA_RFE_Inverse_Jaguar include/rtl8822b_hal.h:201`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/btc/halbtc8822bwifionly.c:74` | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | 32 | 0x80000 | live | `halwifionly_phy_set_bb_reg(pwifionlycfg, 0xcbc, 0x80000, 0x0);` |
| `hal/btc/halbtc8822bwifionly.c:119` | hal8822b_wifi_only_switch_antenna | halwifionly_phy_set_bb_reg | 32 | 0x300 | live | `halwifionly_phy_set_bb_reg(pwifionlycfg, 0xcbc, 0x300, 0x1);` |
| `hal/btc/halbtc8822bwifionly.c:121` | hal8822b_wifi_only_switch_antenna | halwifionly_phy_set_bb_reg | 32 | 0x300 | live | `halwifionly_phy_set_bb_reg(pwifionlycfg, 0xcbc, 0x300, 0x2);` |
| `hal/hal_mp.c:1879` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xFFF | live | `phy_set_bb_reg(pAdapter, rA_RFE_Inverse_Jaguar, 0xFFF, 0xb); /* 0xCBC[23:16] = 0x12*/` |
| `hal/hal_mp.c:1998` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xfff | live | `phy_set_bb_reg(pAdapter, rA_RFE_Inverse_Jaguar, 0xfff, 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:564` | phy_set_rf_path_switch_8822b | odm_set_bb_reg | 32 | BIT(19) | live | `odm_set_bb_reg(dm, R_0xcbc, BIT(19), 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:577` | phy_set_rf_path_switch_8822b | odm_set_bb_reg | 32 | (BIT(9) \| BIT(8)) | live | `odm_set_bb_reg(dm, R_0xcbc, (BIT(9) \| BIT(8)), 0x2); /*WiFi*/` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:582` | phy_set_rf_path_switch_8822b | odm_set_bb_reg | 32 | (BIT(9) \| BIT(8)) | live | `odm_set_bb_reg(dm, R_0xcbc, (BIT(9) \| BIT(8)), 0x1); /*BT*/` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:599` | _phy_query_rf_path_switch_8822b | odm_get_bb_reg | 32 | (BIT(9) \| BIT(8)) | live | `if (odm_get_bb_reg(dm, R_0xcbc, (BIT(9) \| BIT(8))) == 0x2) /*WiFi*/` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:519` | _iqk_rfe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xcbc, 0x0000083B);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:531` | _iqk_rfe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xcbc, 0x00000100);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:129` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xcbc, 0x3f, 0x8);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:130` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xcbc, (BIT(11) \| BIT(10)), 0x2);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:152` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xcbc, 0x3f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:153` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xcbc, (BIT(11) \| BIT(10) \| BIT(9)` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:160` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | BIT(9) | live | `odm_set_bb_reg(dm, 0xcbc, BIT(9), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:162` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | BIT(8) | live | `odm_set_bb_reg(dm, 0xcbc, BIT(8), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:215` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xcbc, 0x3f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:216` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | BIT(11) \| BIT(10) \| BI | live | `odm_set_bb_reg(dm, R_0xcbc, BIT(11) \| BIT(10) \| BIT(9) \| BIT(8), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:337` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xcbc, 0x3f, 0x8);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:338` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xcbc, (BIT(11) \| BIT(10)), 0x2);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:366` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xcbc, 0x3f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:367` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xcbc, (BIT(11) \| BIT(10)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:417` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xcbc, 0x3f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:418` | phydm_rfe_efem | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xcbc, (BIT(11) \| BIT(10)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:441` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xcbc, 0x3f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:442` | phydm_rfe_efem | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xcbc, (BIT(11) \| BIT(10)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:488` | phydm_rfe_ifem | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xcbc, 0x3f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:489` | phydm_rfe_ifem | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xcbc, (BIT(11) \| BIT(10)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:277` | phydm_rfe_8822b_setting | odm_set_bb_reg | 32 | BIT(rfe_n) | no-caller | `odm_set_bb_reg(dm, R_0xcbc, BIT(rfe_n), (u32)inv_en);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:541` | phydm_rfe_2t3r | odm_set_bb_reg | 32 | BIT(19) | no-caller | `odm_set_bb_reg(dm, R_0xcbc, BIT(19), 0x0); /* RFE_buffer */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:542` | phydm_rfe_2t3r | odm_set_bb_reg | 32 | BIT(5) | no-caller | `odm_set_bb_reg(dm, R_0xcbc, BIT(5), 0x0); /* rfe_inv  (RFE_CTRL_5) */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:543` | phydm_rfe_2t3r | odm_set_bb_reg | 32 | BIT(10) | no-caller | `odm_set_bb_reg(dm, R_0xcbc, BIT(10), 0x1); /* rfe_inv  (RFE_CTRL_10) */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:558` | phydm_rfe_2t4r | odm_set_bb_reg | 32 | BIT(19) | no-caller | `odm_set_bb_reg(dm, R_0xcbc, BIT(19), 0x0); /* RFE_buffer */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:560` | phydm_rfe_2t4r | odm_set_bb_reg | 32 | BIT(5) | no-caller | `odm_set_bb_reg(dm, R_0xcbc, BIT(5), 0x0); /* rfe_inv  (RFE_CTRL_5) */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:612` | phydm_rfe_smtant | odm_set_bb_reg | 32 | 0xfff | no-caller | `odm_set_bb_reg(dm, R_0xcbc, 0xfff, 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10458`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CC0` — St A — `R_0xcc0`

Định nghĩa: `R_0xcc0 hal/phydm/phydm_regtable.h:725`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10459`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CC4` — St A — `R_0xcc4`

Định nghĩa: `R_0xcc4 hal/phydm/phydm_regtable.h:726`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10460`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CC8` — St A — `R_0xcc8`

Định nghĩa: `R_0xcc8 hal/phydm/phydm_regtable.h:727`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10461`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CCC` — St A — `R_0xccc`

Định nghĩa: `R_0xccc hal/phydm/phydm_regtable.h:728`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10462`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CD0` — St A — `R_0xcd0`

Định nghĩa: `R_0xcd0 hal/phydm/phydm_regtable.h:729`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halphyrf_ce.c:503` | odm_txpowertracking_callback_thermal_meter | odm_get_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & (ODM_RTL8723D \| ODM_RTL8710B) | `regcd0 = odm_get_bb_reg(dm, R_0xcd0, MASKDWORD);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10463`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CD4` — St A — `R_0xcd4`

Định nghĩa: `R_0xcd4 hal/phydm/phydm_regtable.h:730`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/halphyrf_ce.c:504` | odm_txpowertracking_callback_thermal_meter | odm_get_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & (ODM_RTL8723D \| ODM_RTL8710B) | `regcd4 = odm_get_bb_reg(dm, R_0xcd4, MASKDWORD);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10464`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CD8` — St A — `R_0xcd8`

Định nghĩa: `R_0xcd8 hal/phydm/phydm_regtable.h:731`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10465`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CDC` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10466`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CE0` — St A — `R_0xce0`

Định nghĩa: `R_0xce0 hal/phydm/phydm_regtable.h:732`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10467`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CE4` — St A — `R_0xce4`, `ODM_REG_SMALL_BANDWIDTH_11N`

Định nghĩa: `R_0xce4 hal/phydm/phydm_regtable.h:733`; `ODM_REG_SMALL_BANDWIDTH_11N hal/phydm/phydm_regdefine11n.h:175`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10468`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CE8` — St A — `R_0xce8`

Định nghĩa: `R_0xce8 hal/phydm/phydm_regtable.h:734`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_adaptivity.c:386` | phydm_edcca_decision_opt | odm_set_bb_reg | 32 | BIT(13) | dead F:dm->support_ic_type & (ODM_RTL8197F \| ODM_RTL8192F) | `odm_set_bb_reg(dm, R_0xce8, BIT(13), 0x1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10469`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CEC` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10470`): 1 mục, giá trị 0x0; điều kiện None

## `0x0CF0` — St X — `ODM_OFDM_FA_CNT1`, `ODM_REG_OFDM_FA_TYPE1_11N`

Định nghĩa: `ODM_REG_OFDM_FA_TYPE1_11N hal/phydm/phydm_regdefine11n.h:176`; `ODM_OFDM_FA_CNT1 hal/phydm/phydm_reg.h:130`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16940` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskLWord | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `OFDM_FA = phy_query_bb_reg(padapter, 0xCF0, bMaskLWord) + phy_query_bb_reg(padapter, 0xCF2, bMaskLWord) +` |

## `0x0CF2` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16940` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskLWord | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `OFDM_FA = phy_query_bb_reg(padapter, 0xCF0, bMaskLWord) + phy_query_bb_reg(padapter, 0xCF2, bMaskLWord) +` |

## `0x0D00` — St X — `R_0xd00`, `rOFDM1_LSTF`, `ODM_OFDM_PHY0_PAGE_D`, `ODM_REG_OFDM_FA_RSTD_11N`

Định nghĩa: `R_0xd00 hal/phydm/phydm_regtable.h:735`; `ODM_REG_OFDM_FA_RSTD_11N hal/phydm/phydm_regdefine11n.h:178`; `ODM_OFDM_PHY0_PAGE_D hal/phydm/phydm_reg.h:131`; `rOFDM1_LSTF include/rtl8822b_hal.h:177`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:17001` | rtw_reset_phy_rx_counters | phy_set_bb_reg | 32 | BIT27 | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_set_bb_reg(padapter, 0xD00, BIT27, 0x1);/* reset  OFDA FA counter */` |
| `hal/hal_com.c:17003` | rtw_reset_phy_rx_counters | phy_set_bb_reg | 32 | BIT27 | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_set_bb_reg(padapter, 0xD00, BIT27, 0x0);` |
| `hal/hal_mp.c:1736` | hal_mpt_SetSingleCarrierTx | phy_set_bb_reg | 32 | BIT30 \| BIT29 \| BIT28 | dead F:IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 \| BIT29 \| BIT28, OFDM_SingleCarrier);` |
| `hal/hal_mp.c:1747` | hal_mpt_SetSingleCarrierTx | phy_set_bb_reg | 32 | BIT30 \| BIT29 \| BIT28 | dead F:IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 \| BIT29 \| BIT28, OFDM_ALL_OFF);` |
| `hal/hal_mp.c:1913` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | BIT30 \| BIT29 \| BIT28 | dead F:IS_HARDWARE_TYPE_JAGUAR(pAdapter) \|\| IS_HARDWARE_TYPE_8822B( | `phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 \| BIT29 \| BIT28, OFDM_SingleTone);` |
| `hal/hal_mp.c:2043` | hal_mpt_SetCarrierSuppressionTx | phy_set_bb_reg | 32 | BIT30 \| BIT29 \| BIT28 | dead F:IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 \| BIT29 \| BIT28, OFDM_ALL_OFF);` |
| `hal/hal_mp.c:2158` | mpt_StopOfdmContTx | phy_set_bb_reg | 32 | BIT30 \| BIT29 \| BIT28 | dead F:IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 \| BIT29 \| BIT28, OFDM_ALL_OFF);` |
| `hal/hal_mp.c:2194` | mpt_StartCckContTx | phy_set_bb_reg | 32 | BIT30 \| BIT29 \| BIT28 | dead F:IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 \| BIT29 \| BIT28, OFDM_ALL_OFF);` |
| `hal/hal_mp.c:2248` | mpt_StartOfdmContTx | phy_set_bb_reg | 32 | BIT30 \| BIT29 \| BIT28 | dead F:IS_HARDWARE_TYPE_JAGUAR_ALL(pAdapter) | `phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 \| BIT29 \| BIT28, OFDM_ContinuousTx);` |
| `hal/phydm/phydm_adc_sampling.c:1100` | phydm_la_set_bb | odm_set_bb_reg | 32 | BIT(26) | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `odm_set_bb_reg(dm, R_0xd00, BIT(26), 0x1); /*@update rpt every pkt*/` |
| `hal/phydm/phydm_dig.c:1355` | phydm_false_alarm_counter_reg_hold | odm_set_bb_reg | 32 | BIT(31) | dead/no-caller F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xd00, BIT(31), 1);` |

## `0x0D04` — St X — `R_0xd04`, `rA_PIRead_Jaguar`, `rOFDM1_TRxPathEnable`, `ODM_REG_BB_RX_ANT_11N`

Định nghĩa: `R_0xd04 hal/phydm/phydm_regtable.h:736`; `ODM_REG_BB_RX_ANT_11N hal/phydm/phydm_regdefine11n.h:179`; `rOFDM1_TRxPathEnable include/rtl8822b_hal.h:178`; `rA_PIRead_Jaguar include/rtl8822b_hal.h:179`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1478` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | 0x0000000f | dead-func | `phy_set_bb_reg(pAdapter, rOFDM1_TRxPathEnable, 0x0000000f, r_rx_antenna_ofdm);	/*/OFDM Rx*/` |
| `hal/hal_mp.c:1481` | mpt_SetRFPath_819X | phy_set_bb_reg | 32 | 0x000000F0 | dead/dead-func F:IS_HARDWARE_TYPE_8192E(pAdapter) | `phy_set_bb_reg(pAdapter, rOFDM1_TRxPathEnable, 0x000000F0, r_rx_antenna_ofdm);	/*/OFDM Rx*/` |

## `0x0D08` — St R — `R_0xd08`, `rA_SIRead_Jaguar`

Định nghĩa: `R_0xd08 hal/phydm/phydm_regtable.h:737`; `rA_SIRead_Jaguar include/rtl8822b_hal.h:180`

## `0x0D0C` — St A — `R_0xd0c`

Định nghĩa: `R_0xd0c hal/phydm/phydm_regtable.h:738`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_cfotracking.c:86` | phydm_get_cfo_info_ac | odm_read_4byte | 32 |  | live | `val[0] = odm_read_4byte(dm, R_0xd0c);` |

## `0x0D10` — St A — `R_0xd10`

Định nghĩa: `R_0xd10 hal/phydm/phydm_regtable.h:739`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_cfotracking.c:87` | phydm_get_cfo_info_ac | odm_read_4byte | 32 |  | live | `val_1[0] = odm_read_4byte(dm, R_0xd10);` |
| `hal/phydm/phydm_soml.c:318` | phydm_soml_cfo_process | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xd10, MASKDWORD);` |

## `0x0D14` — St A — `R_0xd14`

Định nghĩa: `R_0xd14 hal/phydm/phydm_regtable.h:740`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_cfotracking.c:88` | phydm_get_cfo_info_ac | odm_get_bb_reg | 32 | 0x1fff0000 | live | `val_2[0] = odm_get_bb_reg(dm, R_0xd14, 0x1fff0000);` |
| `hal/phydm/phydm_debug.c:524` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xd14, MASKDWORD);` |
| `hal/phydm/phydm_soml.c:319` | phydm_soml_cfo_process | odm_get_bb_reg | 32 | MASKDWORD | live | `value32_1 = odm_get_bb_reg(dm, R_0xd14, MASKDWORD);` |

## `0x0D2C` — St X — `R_0xd2c`, `ODM_REG_BB_ATC_11N`

Định nghĩa: `R_0xd2c hal/phydm/phydm_regtable.h:741`; `ODM_REG_BB_ATC_11N hal/phydm/phydm_regdefine11n.h:180`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:804` | phydm_csi_mask_enable | odm_set_bb_reg | 32 | BIT(28) | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xd2c, BIT(28), en);` |

## `0x0D30` — St A — `R_0xd30`

Định nghĩa: `R_0xd30 hal/phydm/phydm_regtable.h:742`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1130` | config_phydm_read_txagc_8822b | odm_get_bb_reg | 32 | 0x7f0000 | live | `read_back_data = (u8)odm_get_bb_reg(dm, R_0xd30, 0x7f0000);` |

## `0x0D40` — St X — `R_0xd40`

Định nghĩa: `R_0xd40 hal/phydm/phydm_regtable.h:743`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:825` | phydm_clean_all_csi_mask | odm_set_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xd40, MASKDWORD, 0);` |

## `0x0D44` — St X — `R_0xd44`, `rB_PIRead_Jaguar`

Định nghĩa: `R_0xd44 hal/phydm/phydm_regtable.h:744`; `rB_PIRead_Jaguar include/rtl8822b_hal.h:181`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:826` | phydm_clean_all_csi_mask | odm_set_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xd44, MASKDWORD, 0);` |

## `0x0D48` — St X — `R_0xd48`, `rB_SIRead_Jaguar`

Định nghĩa: `R_0xd48 hal/phydm/phydm_regtable.h:745`; `rB_SIRead_Jaguar include/rtl8822b_hal.h:182`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:827` | phydm_clean_all_csi_mask | odm_set_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xd48, MASKDWORD, 0);` |

## `0x0D4C` — St A — `R_0xd4c`

Định nghĩa: `R_0xd4c hal/phydm/phydm_regtable.h:746`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_cfotracking.c:91` | phydm_get_cfo_info_ac | odm_read_4byte | 32 |  | live | `val[1] = odm_read_4byte(dm, R_0xd4c);` |
| `hal/phydm/phydm_api.c:828` | phydm_clean_all_csi_mask | odm_set_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xd4c, MASKDWORD, 0);` |

## `0x0D50` — St A — `R_0xd50`

Định nghĩa: `R_0xd50 hal/phydm/phydm_regtable.h:747`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_cfotracking.c:92` | phydm_get_cfo_info_ac | odm_read_4byte | 32 |  | live | `val_1[1] = odm_read_4byte(dm, R_0xd50);` |
| `hal/phydm/phydm_soml.c:320` | phydm_soml_cfo_process | odm_get_bb_reg | 32 | MASKDWORD | live | `value32_2 = odm_get_bb_reg(dm, R_0xd50, MASKDWORD);` |

## `0x0D54` — St A — `R_0xd54`

Định nghĩa: `R_0xd54 hal/phydm/phydm_regtable.h:748`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_cfotracking.c:93` | phydm_get_cfo_info_ac | odm_get_bb_reg | 32 | 0x1fff0000 | live | `val_2[1] = odm_get_bb_reg(dm, R_0xd54, 0x1fff0000);` |
| `hal/phydm/phydm_debug.c:528` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xd54, MASKDWORD);` |
| `hal/phydm/phydm_soml.c:321` | phydm_soml_cfo_process | odm_get_bb_reg | 32 | MASKDWORD | live | `value32_3 = odm_get_bb_reg(dm, R_0xd54, MASKDWORD);` |

## `0x0D94` — St G — `R_0xd94`

Định nghĩa: `R_0xd94 hal/phydm/phydm_regtable.h:756`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:532` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xd94, MASKDWORD);` |

## `0x0DA2` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16941` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskLWord | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_query_bb_reg(padapter, 0xDA2, bMaskLWord) + phy_query_bb_reg(padapter, 0xDA4, bMaskLWord) +` |

## `0x0DA4` — St X — `ODM_OFDM_FA_CNT3`, `ODM_REG_OFDM_FA_TYPE3_11N`

Định nghĩa: `ODM_REG_OFDM_FA_TYPE3_11N hal/phydm/phydm_regdefine11n.h:182`; `ODM_OFDM_FA_CNT3 hal/phydm/phydm_reg.h:133`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16941` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskLWord | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_query_bb_reg(padapter, 0xDA2, bMaskLWord) + phy_query_bb_reg(padapter, 0xDA4, bMaskLWord) +` |

## `0x0DA6` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16942` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskLWord | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_query_bb_reg(padapter, 0xDA6, bMaskLWord) + phy_query_bb_reg(padapter, 0xDA8, bMaskLWord);` |

## `0x0DA8` — St X — `ODM_OFDM_FA_CNT4`, `ODM_REG_OFDM_FA_TYPE4_11N`

Định nghĩa: `ODM_REG_OFDM_FA_TYPE4_11N hal/phydm/phydm_regdefine11n.h:183`; `ODM_OFDM_FA_CNT4 hal/phydm/phydm_reg.h:134`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16942` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskLWord | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_query_bb_reg(padapter, 0xDA6, bMaskLWord) + phy_query_bb_reg(padapter, 0xDA8, bMaskLWord);` |

## `0x0DD4` — St G — `R_0xdd4`

Định nghĩa: `R_0xdd4 hal/phydm/phydm_regtable.h:764`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:536` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xdd4, MASKDWORD);` |

## `0x0DF4` — St X — `R_0xdf4`, `ODM_REG_RPT_11N`

Định nghĩa: `R_0xdf4 hal/phydm/phydm_regtable.h:768`; `ODM_REG_RPT_11N hal/phydm/phydm_regdefine11n.h:184`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:211` | phydm_get_bb_dbg_port_val | odm_get_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & ODM_IC_11AC_SERIES | `dbg_port_value = odm_get_bb_reg(dm, R_0xdf4, MASKDWORD);` |

## `0x0E00` — St A — `R_0xe00`, `ODM_TXAGC_A_6_18`, `rTxAGC_A_Rate18_06`, `REG_TX_AGC_A_RATE18_06`, `ODM_REG_TXAGC_A_6_18_11N`

Định nghĩa: `REG_TX_AGC_A_RATE18_06 include/hal_phy_reg.h:225`; `R_0xe00 hal/phydm/phydm_regtable.h:769`; `ODM_REG_TXAGC_A_6_18_11N hal/phydm/phydm_regdefine11n.h:186`; `ODM_TXAGC_A_6_18 hal/phydm/phydm_reg.h:135`; `rTxAGC_A_Rate18_06 include/rtl8822b_hal.h:184`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `core/rtw_mp.c:2235` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_e00 = rtw_read32(padapter, 0xE00);` |
| `core/rtw_mp.c:2237` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x3 | live | `phy_set_bb_reg(padapter, 0xE00, 0x3, 0x00);` |
| `core/rtw_mp.c:2256` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_e00 = rtw_read32(padapter, 0xE00);` |
| `core/rtw_mp.c:2259` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x3 | live | `phy_set_bb_reg(padapter, 0xE00, 0x3, 0x00);` |
| `core/rtw_mp.c:2276` | rtw_mp_psd_init | rtw_read32 | 32 |  | live | `regs->reg_e00 = rtw_read32(padapter, 0xE00);` |
| `core/rtw_mp.c:2280` | rtw_mp_psd_init | phy_set_bb_reg | 32 | 0x3 | live | `phy_set_bb_reg(padapter, 0xE00, 0x3, 0x00);` |
| `core/rtw_mp.c:2328` | rtw_mp_psd_close | rtw_write32 | 32 |  | live | `rtw_write32(padapter, 0xE00, regs->reg_e00);` |
| `core/rtw_mp.c:2333` | rtw_mp_psd_close | rtw_write32 | 32 |  | live | `rtw_write32(padapter, 0xE00, regs->reg_e00);` |
| `core/rtw_mp.c:2340` | rtw_mp_psd_close | rtw_write32 | 32 |  | live | `rtw_write32(padapter, 0xE00, regs->reg_e00);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:480` | _phy_lc_calibrate_8822b | odm_read_4byte | 32 |  | live | `tmp0xe00 = odm_read_4byte(dm, 0xe00);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:482` | _phy_lc_calibrate_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xe00, 0x4);` |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c:504` | _phy_lc_calibrate_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xe00, tmp0xe00);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:604` | _iqk_configure_macbb_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xe00, 0x00000004);` |
| `hal/phydm/phydm_api.c:575` | phydm_stop_3_wire | odm_set_bb_reg | 32 | 0xf | live | `odm_set_bb_reg(dm, R_0xe00, 0xf, 0x4);` |
| `hal/phydm/phydm_api.c:604` | phydm_stop_3_wire | odm_set_bb_reg | 32 | 0xf | live | `odm_set_bb_reg(dm, R_0xe00, 0xf, 0x7);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1452` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0xe00, MASKBYTE0, 0x4);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1504` | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0xe00, MASKBYTE0, 0x7);` |
| `hal/hal_mp.c:440` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskDWord | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_A_Rate18_06, bMaskDWord, TxAGC);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10471`): 1 mục, giá trị 0x7; điều kiện None

## `0x0E04` — St A — `R_0xe04`, `ODM_TXAGC_A_24_54`, `rTxAGC_A_Rate54_24`, `REG_TX_AGC_A_RATE54_24`, `ODM_REG_TXAGC_A_24_54_11N`

Định nghĩa: `REG_TX_AGC_A_RATE54_24 include/hal_phy_reg.h:226`; `R_0xe04 hal/phydm/phydm_regtable.h:770`; `ODM_REG_TXAGC_A_24_54_11N hal/phydm/phydm_regdefine11n.h:187`; `ODM_TXAGC_A_24_54 hal/phydm/phydm_reg.h:136`; `rTxAGC_A_Rate54_24 include/rtl8822b_hal.h:185`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:2881` | phydm_set_weighting_cmn | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, 0xe04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1632` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, R_0xe04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1650` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, R_0xe04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1707` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, R_0xe04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1715` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, R_0xe04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:88` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | BIT(18) | live | `odm_set_bb_reg(dm, R_0xe04, BIT(18), 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:103` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, R_0xe04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:114` | phydm_dynamic_parameters_ota | odm_set_bb_reg | 32 | (BIT(18) \| BIT(21)) | live | `odm_set_bb_reg(dm, R_0xe04, (BIT(18) \| BIT(21)), 0x0);` |
| `hal/hal_mp.c:441` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskDWord | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_A_Rate54_24, bMaskDWord, TxAGC);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10472`): 1 mục, giá trị 0x20; điều kiện None

## `0x0E08` — St A — `R_0xe08`, `ODM_REG_ANT_11AC_B`, `ODM_TXAGC_A_1_MCS32`, `rTxAGC_A_CCK1_Mcs32`, `REG_TX_AGC_A_CCK_1_MCS32`, `ODM_REG_TXAGC_A_1_MCS32_11N`

Định nghĩa: `REG_TX_AGC_A_CCK_1_MCS32 include/hal_phy_reg.h:204`; `R_0xe08 hal/phydm/phydm_regtable.h:771`; `ODM_REG_ANT_11AC_B hal/phydm/phydm_regdefine11ac.h:97`; `ODM_REG_TXAGC_A_1_MCS32_11N hal/phydm/phydm_regdefine11n.h:188`; `ODM_TXAGC_A_1_MCS32 hal/phydm/phydm_reg.h:137`; `rTxAGC_A_CCK1_Mcs32 include/rtl8822b_hal.h:186`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2508` | config_phydm_trx_mode_8822b | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xe08, MASKLWORD, 0x3231);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2510` | config_phydm_trx_mode_8822b | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xe08, MASKLWORD, 0x1111);` |
| `hal/hal_mp.c:420` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskByte1 | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_A_CCK1_Mcs32, bMaskByte1, pTxPower[RF_PATH_A]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10473`): 1 mục, giá trị 0x60403231; điều kiện None

## `0x0E0C` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10474`): 1 mục, giá trị 0x12345; điều kiện None

## `0x0E10` — St A — `R_0xe10`, `rTxAGC_A_Mcs03_Mcs00`, `ODM_TXAGC_A_MCS0_MCS3`, `REG_TX_AGC_A_MCS03_MCS00`, `ODM_REG_TXAGC_A_MCS0_3_11N`

Định nghĩa: `REG_TX_AGC_A_MCS03_MCS00 include/hal_phy_reg.h:211`; `R_0xe10 hal/phydm/phydm_regtable.h:772`; `ODM_REG_TXAGC_A_MCS0_3_11N hal/phydm/phydm_regdefine11n.h:189`; `ODM_TXAGC_A_MCS0_MCS3 hal/phydm/phydm_reg.h:138`; `rTxAGC_A_Mcs03_Mcs00 include/rtl8822b_hal.h:187`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm.c:3368` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0x3c000000 | live | `odm_set_bb_reg(dm, R_0xe10, 0x3c000000,` |
| `hal/phydm/phydm.c:3370` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0xfc00 | live | `odm_set_bb_reg(dm, R_0xe10, 0xfc00,` |
| `hal/hal_mp.c:442` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskDWord | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_A_Mcs03_Mcs00, bMaskDWord, TxAGC);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10475`): 1 mục, giá trị 0x100; điều kiện None

## `0x0E14` — St A — `R_0xe14`, `rTxAGC_A_Mcs07_Mcs04`, `ODM_TXAGC_A_MCS4_MCS7`, `REG_TX_AGC_A_MCS07_MCS04`, `ODM_REG_TXAGC_A_MCS4_7_11N`

Định nghĩa: `REG_TX_AGC_A_MCS07_MCS04 include/hal_phy_reg.h:212`; `R_0xe14 hal/phydm/phydm_regtable.h:773`; `ODM_REG_TXAGC_A_MCS4_7_11N hal/phydm/phydm_regdefine11n.h:190`; `ODM_TXAGC_A_MCS4_MCS7 hal/phydm/phydm_reg.h:139`; `rTxAGC_A_Mcs07_Mcs04 include/rtl8822b_hal.h:188`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm.c:3372` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0x3c000000 | live | `odm_set_bb_reg(dm, R_0xe14, 0x3c000000,` |
| `hal/phydm/phydm.c:3374` | phydm_dc_cancellation | odm_set_bb_reg | 32 | 0xfc00 | live | `odm_set_bb_reg(dm, R_0xe14, 0xfc00,` |
| `hal/hal_mp.c:443` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskDWord | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_A_Mcs07_Mcs04, bMaskDWord, TxAGC);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10476`): 1 mục, giá trị 0x1000000; điều kiện None

## `0x0E18` — St A — `R_0xe18`, `rTxAGC_A_Mcs11_Mcs08`, `ODM_TXAGC_A_MCS8_MCS11`, `REG_TX_AGC_A_MCS11_MCS08`, `ODM_REG_TXAGC_A_MCS8_11_11N`

Định nghĩa: `REG_TX_AGC_A_MCS11_MCS08 include/hal_phy_reg.h:213`; `R_0xe18 hal/phydm/phydm_regtable.h:774`; `ODM_REG_TXAGC_A_MCS8_11_11N hal/phydm/phydm_regdefine11n.h:191`; `ODM_TXAGC_A_MCS8_MCS11 hal/phydm/phydm_reg.h:140`; `rTxAGC_A_Mcs11_Mcs08 include/rtl8822b_hal.h:189`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:444` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskDWord | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_A_Mcs11_Mcs08, bMaskDWord, TxAGC);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10477`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E1C` — St A — `R_0xe1c`, `rB_TxScale_Jaguar`, `rTxAGC_A_Mcs15_Mcs12`, `REG_B_TX_SCALE_JAGUAR`, `ODM_TXAGC_A_MCS12_MCS15`, `REG_TX_AGC_A_MCS15_MCS12`, `ODM_REG_TXAGC_A_MCS12_15_11N`

Định nghĩa: `REG_B_TX_SCALE_JAGUAR include/hal_phy_reg.h:71`; `REG_B_TX_SCALE_JAGUAR include/hal_phy_reg.h:125`; `REG_TX_AGC_A_MCS15_MCS12 include/hal_phy_reg.h:214`; `R_0xe1c hal/phydm/phydm_regtable.h:775`; `ODM_REG_TXAGC_A_MCS12_15_11N hal/phydm/phydm_regdefine11n.h:192`; `ODM_TXAGC_A_MCS12_MCS15 hal/phydm/phydm_reg.h:141`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:926` | mpt_SetSingleTone_8814A | phy_query_bb_reg | 32 | bMaskDWord | live | `regIG1 = phy_query_bb_reg(pAdapter, rB_TxScale_Jaguar, bMaskDWord);		/*/ 0xE1C[31:21]*/` |
| `hal/hal_mp.c:972` | mpt_SetSingleTone_8814A | phy_set_bb_reg | 32 | 0xFFE00000 | live | `phy_set_bb_reg(pAdapter, rB_TxScale_Jaguar, 0xFFE00000, 0); /*/ 0xE1C[31:21]*/` |
| `hal/hal_mp.c:1011` | mpt_SetSingleTone_8814A | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(pAdapter, rB_TxScale_Jaguar, bMaskDWord, regIG1); /* 0xE1C[31:21]*/` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1792` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xe1c, (BIT(11) \| BIT(10) \|` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1798` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xe1c, (BIT(11) \| BIT(10) \|` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1804` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xe1c, (BIT(11) \| BIT(10) \|` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1816` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xe1c, (BIT(11) \| BIT(10) \|` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1822` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xe1c, (BIT(11) \| BIT(10) \|` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1828` | phydm_rxagc_switch_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xe1c, (BIT(11) \| BIT(10) \|` |
| `hal/hal_mp.c:445` | mpt_SetTxPower_Old | phy_set_bb_reg | 32 | bMaskDWord | dead-func | `phy_set_bb_reg(pAdapter, rTxAGC_A_Mcs15_Mcs12, bMaskDWord, TxAGC);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10478`): 1 mục, giá trị 0x40040053; điều kiện None

## `0x0E20` — St A — `R_0xe20`

Định nghĩa: `R_0xe20 hal/phydm/phydm_regtable.h:776`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_api.c:1006` | phydm_nbi_enable | odm_set_bb_reg | 32 | BIT(28) | live | `odm_set_bb_reg(dm, R_0xe20, BIT(28), val);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:883` | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | 32 | BIT(31) | live | `odm_set_bb_reg(dm, R_0xe20, BIT(31), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:889` | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | 32 | BIT(31) | live | `odm_set_bb_reg(dm, R_0xe20, BIT(31), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:895` | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | 32 | BIT(31) | live | `odm_set_bb_reg(dm, R_0xe20, BIT(31), 0x1);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1578` | phydm_spur_calibration_8822b | odm_set_bb_reg | 32 | BIT(28) | live | `odm_set_bb_reg(dm, R_0xe20, BIT(28), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1762` | config_phydm_switch_band_8822b | odm_set_bb_reg | 32 | BIT(28) | cfg-off C:*dm->mp_mode | `odm_set_bb_reg(dm, R_0xe20, BIT(28), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2023` | config_phydm_switch_channel_8822b | odm_set_bb_reg | 32 | BIT(28) | cfg-off C:*dm->mp_mode | `odm_set_bb_reg(dm, R_0xe20, BIT(28), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:2288` | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | 32 | BIT(28) | cfg-off C:*dm->mp_mode | `odm_set_bb_reg(dm, R_0xe20, BIT(28), 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10479`): 1 mục, giá trị 0x40020103; điều kiện None

## `0x0E24` — St A — `R_0xe24`, `ODM_REG_TAP_UPD_97F`, `ODM_REG_EDCCA_DCNF_11N`

Định nghĩa: `R_0xe24 hal/phydm/phydm_regtable.h:777`; `ODM_REG_EDCCA_DCNF_11N hal/phydm/phydm_regdefine11n.h:193`; `ODM_REG_TAP_UPD_97F hal/phydm/phydm_regdefine11n.h:194`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_adaptivity.c:750` | phydm_adaptivity_init | odm_set_bb_reg | 32 | BIT(21) \| BIT(20) | dead F:dm->support_ic_type & ODM_IC_11N_SERIES && !(dm->support_ic_ | `odm_set_bb_reg(dm, R_0xe24, BIT(21) \| BIT(20), 0x1);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10480`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E28` — St A — `R_0xe28`, `ODM_REG_NHM_TH8_11N`, `ODM_REG_PAGE_B1_97F`, `ODM_REG_FPGA0_IQK_11N`

Định nghĩa: `R_0xe28 hal/phydm/phydm_regtable.h:778`; `ODM_REG_NHM_TH8_11N hal/phydm/phydm_regdefine11n.h:104`; `ODM_REG_FPGA0_IQK_11N hal/phydm/phydm_regdefine11n.h:195`; `ODM_REG_PAGE_B1_97F hal/phydm/phydm_regdefine11n.h:196`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_adaptivity.c:744` | phydm_adaptivity_init | odm_set_bb_reg | 32 | BIT(30) | dead F:dm->support_ic_type & ODM_IC_11N_SERIES && !(dm->support_ic_ | `odm_set_bb_reg(dm, R_0xe28, BIT(30), 0x1);` |
| `hal/phydm/phydm_adaptivity.c:747` | phydm_adaptivity_init | odm_set_bb_reg | 32 | BIT(30) | dead F:dm->support_ic_type & ODM_IC_11N_SERIES && !(dm->support_ic_ | `odm_set_bb_reg(dm, R_0xe28, BIT(30), 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10481`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E2C` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10482`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E30` — St A — `R_0xe30`, `ODM_REG_TXIQK_TONE_A_11N`

Định nghĩa: `R_0xe30 hal/phydm/phydm_regtable.h:779`; `ODM_REG_TXIQK_TONE_A_11N hal/phydm/phydm_regdefine11n.h:197`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10483`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E34` — St A — `R_0xe34`, `ODM_REG_RXIQK_TONE_A_11N`

Định nghĩa: `R_0xe34 hal/phydm/phydm_regtable.h:780`; `ODM_REG_RXIQK_TONE_A_11N hal/phydm/phydm_regdefine11n.h:198`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10484`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E38` — St A — `R_0xe38`, `ODM_REG_TXIQK_PI_A_11N`

Định nghĩa: `R_0xe38 hal/phydm/phydm_regtable.h:781`; `ODM_REG_TXIQK_PI_A_11N hal/phydm/phydm_regdefine11n.h:199`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10485`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E3C` — St A — `R_0xe3c`, `ODM_REG_RXIQK_PI_A_11N`

Định nghĩa: `R_0xe3c hal/phydm/phydm_regtable.h:782`; `ODM_REG_RXIQK_PI_A_11N hal/phydm/phydm_regdefine11n.h:200`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10486`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E40` — St A — `R_0xe40`, `ODM_REG_TXIQK_11N`

Định nghĩa: `R_0xe40 hal/phydm/phydm_regtable.h:783`; `ODM_REG_TXIQK_11N hal/phydm/phydm_regdefine11n.h:201`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10487`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E44` — St A — `R_0xe44`, `ODM_REG_RXIQK_11N`

Định nghĩa: `R_0xe44 hal/phydm/phydm_regtable.h:784`; `ODM_REG_RXIQK_11N hal/phydm/phydm_regdefine11n.h:202`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10488`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E48` — St A — `R_0xe48`, `ODM_REG_IQK_AGC_PTS_11N`

Định nghĩa: `R_0xe48 hal/phydm/phydm_regtable.h:785`; `ODM_REG_IQK_AGC_PTS_11N hal/phydm/phydm_regdefine11n.h:203`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10489`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E4C` — St A — `R_0xe4c`, `ODM_REG_IQK_AGC_RSP_11N`

Định nghĩa: `R_0xe4c hal/phydm/phydm_regtable.h:786`; `ODM_REG_IQK_AGC_RSP_11N hal/phydm/phydm_regdefine11n.h:204`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10490`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E50` — St A — `R_0xe50`, `rB_IGI_Jaguar`, `ODM_REG_IGI_B_11AC`

Định nghĩa: `R_0xe50 hal/phydm/phydm_regtable.h:787`; `ODM_REG_IGI_B_11AC hal/phydm/phydm_regdefine11ac.h:96`; `rB_IGI_Jaguar include/rtl8822b_hal.h:192`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mcc.c:2748` | mcc_get_reg_hdl | rtw_read32 (qua mảng dbg_reg) | 32 |  | live | `reg_val = rtw_read32(adapter, dbg_reg[i]);` |
| `hal/phydm/phydm_dig.c:546` | phydm_write_dig_reg_c50 | odm_set_bb_reg | 32 | ODM_BIT(IGI, dm) | live | `odm_set_bb_reg(dm, ODM_REG(IGI_B, dm), ODM_BIT(IGI, dm), igi);` |
| `hal/phydm/phydm_dig.c:667` | phydm_get_igi_reg_val | odm_get_bb_reg | 32 | bit_map | live | `val = odm_get_bb_reg(dm, ODM_REG(IGI_B, dm), bit_map);` |
| `hal/phydm/phydm_noisemonitor.c:264` | phydm_idle_noise_measure_ac | odm_get_bb_reg | 32 | MASKBYTE0 | live | `reg_e50 = (u8)odm_get_bb_reg(dm, R_0xe50, MASKBYTE0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:95` | phydm_igi_toggle_8822b | odm_set_bb_reg | 32 | 0x7f | live | `odm_set_bb_reg(dm, R_0xe50, 0x7f, (igi - 2));` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:96` | phydm_igi_toggle_8822b | odm_set_bb_reg | 32 | 0x7f | live | `odm_set_bb_reg(dm, R_0xe50, 0x7f, igi);` |

- Bảng `array_mp_8822b_agc_tab` (`halhwimg8822b_bb.c:10131`): 8 mục, giá trị 0x20, 0x22; điều kiện ELSE, ELSE_IF {"rfe": 13}, ELSE_IF {"rfe": 14}
- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10491`): 1 mục, giá trị 0x20; điều kiện None

## `0x0E54` — St A — `R_0xe54`

Định nghĩa: `R_0xe54 hal/phydm/phydm_regtable.h:788`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10492`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E58` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:300` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xe58, 0xd8000402);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10493`): 1 mục, giá trị 0xd8120402; điều kiện None

## `0x0E5C` — St A — `R_0xe5c`

Định nghĩa: `R_0xe5c hal/phydm/phydm_regtable.h:789`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:301` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xe5c, 0xd1000120);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10494`): 1 mục, giá trị 0xde000120; điều kiện None

## `0x0E60` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:295` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xe60, 0x50000000);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:296` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xe60, 0x70070040);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:310` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xe60, 0x50000000);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:311` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xe60, 0x70038040);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10673`): 17 mục, giá trị 0x70038040, 0x70146040, 0x70246040, 0x70346040, 0x70446040, 0x70532040, 0x70646040, 0x70738040 …; điều kiện None

## `0x0E64` — St A — `R_0xe64`

Định nghĩa: `R_0xe64 hal/phydm/phydm_regtable.h:790`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10699`): 9 mục, giá trị 0x4000, 0x800000, 0x800002, 0x800003, 0x800004, 0x800005, 0x800006, 0x8800001 …; điều kiện None

## `0x0E68` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10495`): 1 mục, giá trị 0x5979993f; điều kiện None

## `0x0E6C` — St A — `R_0xe6c`, `ODM_REG_BLUETOOTH_11N`

Định nghĩa: `R_0xe6c hal/phydm/phydm_regtable.h:791`; `ODM_REG_BLUETOOTH_11N hal/phydm/phydm_regdefine11n.h:205`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:302` | _iqk_afe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xe6c, 0x00000a15);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10496`): 1 mục, giá trị 0x122a; điều kiện None

## `0x0E70` — St A — `R_0xe70`, `ODM_REG_RX_WAIT_CCA_11N`

Định nghĩa: `R_0xe70 hal/phydm/phydm_regtable.h:792`; `ODM_REG_RX_WAIT_CCA_11N hal/phydm/phydm_regdefine11n.h:206`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10497`): 1 mục, giá trị 0x99795979; điều kiện None

## `0x0E74` — St A — `R_0xe74`, `ODM_REG_TX_CCK_RFON_11N`

Định nghĩa: `R_0xe74 hal/phydm/phydm_regtable.h:793`; `ODM_REG_TX_CCK_RFON_11N hal/phydm/phydm_regdefine11n.h:207`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10498`): 1 mục, giá trị 0x99795979; điều kiện None

## `0x0E78` — St A — `R_0xe78`, `ODM_REG_TX_CCK_BBON_11N`

Định nghĩa: `R_0xe78 hal/phydm/phydm_regtable.h:794`; `ODM_REG_TX_CCK_BBON_11N hal/phydm/phydm_regdefine11n.h:208`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10499`): 1 mục, giá trị 0x99799979; điều kiện None

## `0x0E7C` — St A — `R_0xe7c`, `ODM_REG_OFDM_RFON_11N`

Định nghĩa: `R_0xe7c hal/phydm/phydm_regtable.h:795`; `ODM_REG_OFDM_RFON_11N hal/phydm/phydm_regdefine11n.h:209`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10500`): 1 mục, giá trị 0x99791979; điều kiện None

## `0x0E80` — St A — `R_0xe80`, `ODM_REG_OFDM_BBON_11N`

Định nghĩa: `R_0xe80 hal/phydm/phydm_regtable.h:796`; `ODM_REG_OFDM_BBON_11N hal/phydm/phydm_regdefine11n.h:210`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10501`): 1 mục, giá trị 0x19791979; điều kiện None

## `0x0E84` — St A — `R_0xe84`, `ODM_REG_TX2RX_11N`

Định nghĩa: `R_0xe84 hal/phydm/phydm_regtable.h:797`; `ODM_REG_TX2RX_11N hal/phydm/phydm_regdefine11n.h:211`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10502`): 1 mục, giá trị 0x19791979; điều kiện None

## `0x0E88` — St A — `R_0xe88`, `ODM_REG_TX2TX_11N`

Định nghĩa: `R_0xe88 hal/phydm/phydm_regtable.h:798`; `ODM_REG_TX2TX_11N hal/phydm/phydm_regdefine11n.h:212`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10503`): 1 mục, giá trị 0x0; điều kiện None

## `0x0E8C` — St A — `R_0xe8c`, `ODM_REG_RX_CCK_11N`

Định nghĩa: `R_0xe8c hal/phydm/phydm_regtable.h:799`; `ODM_REG_RX_CCK_11N hal/phydm/phydm_regdefine11n.h:213`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10504`): 1 mục, giá trị 0x7000000; điều kiện None

## `0x0E90` — St A — `R_0xe90`, `rB_LSSIWrite_Jaguar`

Định nghĩa: `R_0xe90 hal/phydm/phydm_regtable.h:800`; `rB_LSSIWrite_Jaguar include/rtl8822b_hal.h:193`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1061` | config_phydm_write_rf_reg_8822b | odm_set_bb_reg (qua mảng offset_write_rf) | 32 | MASKDWORD | live | `odm_set_bb_reg(dm, offset_write_rf[path], MASKDWORD, data_and_addr);` |

## `0x0E94` — St A — `R_0xe94`

Định nghĩa: `R_0xe94 hal/phydm/phydm_regtable.h:801`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:599` | _iqk_configure_macbb_8822b | odm_set_bb_reg | 32 | BIT(0) | live | `odm_set_bb_reg(dm, R_0xe94, BIT(0), 0x1);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:601` | _iqk_configure_macbb_8822b | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xe94, (BIT(11) \| BIT(10)), 0x1);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1007` | _iqk_one_shot_8822b | odm_set_bb_reg (qua mảng iqk_apply) | 32 | BIT(0) | live | `odm_set_bb_reg(dm, iqk_apply[path], BIT(0), 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1021` | _iqk_one_shot_8822b | odm_set_bb_reg (qua mảng iqk_apply) | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, iqk_apply[path],` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:470` | _iqk_reload_iqk_setting_8822b | odm_set_bb_reg (qua mảng iqk_apply) | 32 | BIT(0) | cfg-off-func | `odm_set_bb_reg(dm, iqk_apply[path],` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:474` | _iqk_reload_iqk_setting_8822b | odm_set_bb_reg (qua mảng iqk_apply) | 32 | BIT(10) | cfg-off-func | `odm_set_bb_reg(dm, iqk_apply[path],` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10505`): 1 mục, giá trị 0x1000100; điều kiện None

## `0x0E98` — St A — `R_0xe98`

Định nghĩa: `R_0xe98 hal/phydm/phydm_regtable.h:802`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10506`): 1 mục, giá trị 0x201c8000; điều kiện None

## `0x0E9C` — St A — `R_0xe9c`

Định nghĩa: `R_0xe9c hal/phydm/phydm_regtable.h:803`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10507`): 1 mục, giá trị 0x0; điều kiện None

## `0x0EA0` — St A — `R_0xea0`

Định nghĩa: `R_0xea0 hal/phydm/phydm_regtable.h:804`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:139` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xf050);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:143` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xf055);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:147` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xf550);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:173` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa501);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:177` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa500);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:181` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa005);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:226` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa501);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:230` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa500);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:234` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa005);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:238` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa5a5);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:347` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xf050);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:351` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xf055);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:355` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xf550);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:376` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa501);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:380` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa500);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:384` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa005);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:414` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa555);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:455` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa501);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:459` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa500);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:463` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa005);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:499` | phydm_rfe_ifem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa501);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:503` | phydm_rfe_ifem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa500);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:507` | phydm_rfe_ifem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa005);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:511` | phydm_rfe_ifem | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa5a5);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:634` | phydm_rfe_smtant | odm_set_bb_reg | 32 | MASKLWORD | no-caller | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa501);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:638` | phydm_rfe_smtant | odm_set_bb_reg | 32 | MASKLWORD | no-caller | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa500);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:642` | phydm_rfe_smtant | odm_set_bb_reg | 32 | MASKLWORD | no-caller | `odm_set_bb_reg(dm, R_0xea0, MASKLWORD, 0xa005);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10508`): 1 mục, giá trị 0xa555; điều kiện None

## `0x0EA4` — St A — `R_0xea4`

Định nghĩa: `R_0xea4 hal/phydm/phydm_regtable.h:805`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10509`): 1 mục, giá trị 0x8040201; điều kiện None

## `0x0EA8` — St A — `R_0xea8`

Định nghĩa: `R_0xea8 hal/phydm/phydm_regtable.h:806`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10510`): 1 mục, giá trị 0x80402010; điều kiện None

## `0x0EAC` — St A — `R_0xeac`

Định nghĩa: `R_0xeac hal/phydm/phydm_regtable.h:807`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10511`): 1 mục, giá trị 0x0; điều kiện None

## `0x0EB0` — St A — `R_0xeb0`, `rB_RFE_Pinmux_Jaguar`

Định nghĩa: `R_0xeb0 hal/phydm/phydm_regtable.h:808`; `rB_RFE_Pinmux_Jaguar include/rtl8822b_hal.h:196`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1851` | hal_mpt_SetSingleToneTx | phy_query_bb_reg | 32 | bMaskDWord | live | `regBB1 = phy_query_bb_reg(pAdapter, rB_RFE_Pinmux_Jaguar, bMaskDWord);` |
| `hal/hal_mp.c:1876` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(pAdapter, rB_RFE_Pinmux_Jaguar, bMaskDWord, 0x77777777);  /* 0xEB0=0x77777777*/` |
| `hal/hal_mp.c:1992` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(pAdapter, rB_RFE_Pinmux_Jaguar, bMaskDWord, regBB1);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:520` | _iqk_rfe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xeb0, 0x77777777);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:532` | _iqk_rfe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xeb0, 0x77777777);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:105` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x745774);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:111` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x177517);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:119` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x177517);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:192` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x777777);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:199` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x777747);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:206` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x777747);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:332` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x745774);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:361` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x477547);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:399` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x704570);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:404` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x174517);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:425` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x705770);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:433` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x177517);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:476` | phydm_rfe_ifem | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x745774);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:482` | phydm_rfe_ifem | odm_set_bb_reg | 32 | 0xffffff | live | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x477547);` |
| `hal/hal_mp.c:1896` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xFF00F0 | dead F:IS_HARDWARE_TYPE_8822B(pAdapter) | `phy_set_bb_reg(pAdapter, rB_RFE_Pinmux_Jaguar, 0xFF00F0, 0x77007);  /*/ 0xCB0[[23:16, 7:4] = 0x77007*/` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:284` | phydm_rfe_8822b_setting | odm_set_bb_reg | 32 | ((0xf) << (rfe_n * 4)) | no-caller | `odm_set_bb_reg(dm, R_0xeb0, ((0xf) << (rfe_n * 4)),` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:604` | phydm_rfe_smtant | odm_set_bb_reg | 32 | MASKDWORD | no-caller | `odm_set_bb_reg(dm, R_0xeb0, MASKDWORD, 0x771c7517);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:718` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x745774);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:723` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x777777);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:731` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x477547);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:736` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x777777);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:751` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x705770);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:756` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x777777);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:764` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x177517);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:769` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | 0xffffff | no-caller | `odm_set_bb_reg(dm, R_0xeb0, 0xffffff, 0x777777);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10512`): 1 mục, giá trị 0x77777777; điều kiện None

## `0x0EB4` — St A — `R_0xeb4`, `rB_RFE_Inv_Jaguar`

Định nghĩa: `R_0xeb4 hal/phydm/phydm_regtable.h:809`; `rB_RFE_Inv_Jaguar include/rtl8822b_hal.h:198`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1853` | hal_mpt_SetSingleToneTx | phy_query_bb_reg | 32 | bMaskDWord | live | `regBB3 = phy_query_bb_reg(pAdapter, rB_RFE_Pinmux_Jaguar + 4, bMaskDWord);` |
| `hal/hal_mp.c:1878` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bMaskLWord | live | `phy_set_bb_reg(pAdapter, rB_RFE_Pinmux_Jaguar + 4, bMaskLWord, 0x7777);  /* 0xEB4[15:0] = 0x7777*/` |
| `hal/hal_mp.c:1994` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | bMaskDWord | live | `phy_set_bb_reg(pAdapter, rB_RFE_Pinmux_Jaguar + 4, bMaskDWord, regBB3);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:521` | _iqk_rfe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xeb4, 0x00007777);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:533` | _iqk_rfe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xeb4, 0x00007777);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:107` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:113` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xeb4, MASKLWORD, 0x7577);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:121` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | MASKLWORD | live | `odm_set_bb_reg(dm, R_0xeb4, MASKLWORD, 0x7577);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:194` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x77);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:201` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE0, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:208` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | MASKBYTE0 | live | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE0, 0x75);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:334` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:363` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x75);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:401` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x45);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:406` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x45);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:427` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:435` | phydm_rfe_efem | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x75);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:478` | phydm_rfe_ifem | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:484` | phydm_rfe_ifem | odm_set_bb_reg | 32 | MASKBYTE1 | live | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x75);` |
| `hal/hal_mp.c:1900` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xFF00000 | dead F:IS_HARDWARE_TYPE_8822B(pAdapter) | `phy_set_bb_reg(pAdapter, rB_RFE_Pinmux_Jaguar + 4, 0xFF00000, 0x12); /*/ 0xEB4[23:16] = 0x12*/` |
| `hal/hal_mp.c:1903` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xFF00000 | dead F:IS_HARDWARE_TYPE_8822B(pAdapter) | `phy_set_bb_reg(pAdapter, rB_RFE_Pinmux_Jaguar + 4, 0xFF00000, 0x11); /*/ 0xEB4[23:16] = 0x11*/` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:289` | phydm_rfe_8822b_setting | odm_set_bb_reg | 32 | ((0xf) << ((rfe_n - 8) * | no-caller | `odm_set_bb_reg(dm, R_0xeb4, ((0xf) << ((rfe_n - 8) * 4)),` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:563` | phydm_rfe_2t4r | odm_set_bb_reg | 32 | 0xF00 | no-caller | `odm_set_bb_reg(dm, R_0xeb4, 0xF00, 0x8); /* path-B, RFE_CTRL_10 */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:609` | phydm_rfe_smtant | odm_set_bb_reg | 32 | MASKDWORD | no-caller | `odm_set_bb_reg(dm, R_0xeb4, MASKDWORD, 0xd5e7);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:720` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:725` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x77);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:733` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x75);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:738` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x77);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:753` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x57);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:758` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x77);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:766` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x75);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:771` | phydm_rfe_8822b_lps | odm_set_bb_reg | 32 | MASKBYTE1 | no-caller | `odm_set_bb_reg(dm, R_0xeb4, MASKBYTE1, 0x77);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10513`): 1 mục, giá trị 0x7777; điều kiện None

## `0x0EB8` — St A — `R_0xeb8`, `rB_RFE_Jaguar`

Định nghĩa: `R_0xeb8 hal/phydm/phydm_regtable.h:810`; `rB_RFE_Jaguar include/rtl8822b_hal.h:200`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:115` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | BIT(5) | live | `odm_set_bb_reg(dm, R_0xeb8, BIT(5), 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:123` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | BIT(5) | live | `odm_set_bb_reg(dm, R_0xeb8, BIT(5), 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:429` | phydm_rfe_efem | odm_set_bb_reg | 32 | BIT(4) | live | `odm_set_bb_reg(dm, R_0xeb8, BIT(4), 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:437` | phydm_rfe_efem | odm_set_bb_reg | 32 | BIT(5) | live | `odm_set_bb_reg(dm, R_0xeb8, BIT(5), 0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:622` | phydm_rfe_smtant | odm_set_bb_reg | 32 | BIT(5) | no-caller | `odm_set_bb_reg(dm, R_0xeb8, BIT(5), 0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10514`): 1 mục, giá trị 0x0; điều kiện None

## `0x0EBC` — St A — `R_0xebc`, `rB_RFE_Inverse_Jaguar`

Định nghĩa: `R_0xebc hal/phydm/phydm_regtable.h:811`; `rB_RFE_Inverse_Jaguar include/rtl8822b_hal.h:202`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:1880` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xFFF | live | `phy_set_bb_reg(pAdapter, rB_RFE_Inverse_Jaguar, 0xFFF, 0x830); /* 0xEBC[23:16] = 0x12*/` |
| `hal/hal_mp.c:1999` | hal_mpt_SetSingleToneTx | phy_set_bb_reg | 32 | 0xfff | live | `phy_set_bb_reg(pAdapter, rB_RFE_Inverse_Jaguar, 0xfff, 0x0);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | _iqk_backup_mac_bb_8822b | odm_read_4byte (qua mảng backup_bb_reg) | 32 |  | live | `BB_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:329` | _iqk_restore_mac_bb_8822b | odm_write_4byte (qua mảng backup_bb_reg) | 32 |  | live | `odm_write_4byte(dm, backup_bb_reg[i], BB_backup[i]);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:522` | _iqk_rfe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xebc, 0x0000083B);` |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:534` | _iqk_rfe_setting_8822b | odm_write_4byte | 32 |  | live | `odm_write_4byte(dm, 0xebc, 0x00000100);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:131` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xebc, 0x3f, 0x8);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:132` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xebc, (BIT(11) \| BIT(10)), 0x2);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:155` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xebc, 0x3f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:156` | phydm_8822b_type18_rfe | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10) \| B | live | `odm_set_bb_reg(dm, R_0xebc, (BIT(11) \| BIT(10) \| BIT(9)` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:217` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xebc, 0x3f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:218` | phydm_8822b_type15_rfe | odm_set_bb_reg | 32 | BIT(11) \| BIT(10) \| BI | live | `odm_set_bb_reg(dm, R_0xebc, BIT(11) \| BIT(10) \| BIT(9) \| BIT(8), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:339` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xebc, 0x3f, 0x8);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:340` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xebc, (BIT(11) \| BIT(10)), 0x2);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:368` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xebc, 0x3f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:369` | phydm_rfe_4_11 | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xebc, (BIT(11) \| BIT(10)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:419` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xebc, 0x3f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:420` | phydm_rfe_efem | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xebc, (BIT(11) \| BIT(10)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:443` | phydm_rfe_efem | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xebc, 0x3f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:444` | phydm_rfe_efem | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xebc, (BIT(11) \| BIT(10)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:490` | phydm_rfe_ifem | odm_set_bb_reg | 32 | 0x3f | live | `odm_set_bb_reg(dm, R_0xebc, 0x3f, 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:491` | phydm_rfe_ifem | odm_set_bb_reg | 32 | (BIT(11) \| BIT(10)) | live | `odm_set_bb_reg(dm, R_0xebc, (BIT(11) \| BIT(10)), 0x0);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:278` | phydm_rfe_8822b_setting | odm_set_bb_reg | 32 | BIT(rfe_n) | no-caller | `odm_set_bb_reg(dm, R_0xebc, BIT(rfe_n), (u32)inv_en);` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:559` | phydm_rfe_2t4r | odm_set_bb_reg | 32 | BIT(19) | no-caller | `odm_set_bb_reg(dm, R_0xebc, BIT(19), 0x0); /* RFE_buffer */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:561` | phydm_rfe_2t4r | odm_set_bb_reg | 32 | BIT(10) | no-caller | `odm_set_bb_reg(dm, R_0xebc, BIT(10), 0x0); /* rfe_inv  (RFE_CTRL_10) */` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:613` | phydm_rfe_smtant | odm_set_bb_reg | 32 | 0xfff | no-caller | `odm_set_bb_reg(dm, R_0xebc, 0xfff, 0x0);` |

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10515`): 1 mục, giá trị 0x0; điều kiện None

## `0x0EC0` — St A — `R_0xec0`

Định nghĩa: `R_0xec0 hal/phydm/phydm_regtable.h:813`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10516`): 1 mục, giá trị 0x0; điều kiện None

## `0x0EC4` — St A — `R_0xec4`

Định nghĩa: `R_0xec4 hal/phydm/phydm_regtable.h:814`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10517`): 1 mục, giá trị 0x0; điều kiện None

## `0x0EC8` — St A — `R_0xec8`

Định nghĩa: `R_0xec8 hal/phydm/phydm_regtable.h:815`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10518`): 1 mục, giá trị 0x0; điều kiện None

## `0x0ECC` — St A — `R_0xecc`

Định nghĩa: `R_0xecc hal/phydm/phydm_regtable.h:816`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10519`): 1 mục, giá trị 0x0; điều kiện None

## `0x0ED0` — St A — `R_0xed0`, `ODM_REG_RX_OFDM_11N`

Định nghĩa: `R_0xed0 hal/phydm/phydm_regtable.h:817`; `ODM_REG_RX_OFDM_11N hal/phydm/phydm_regdefine11n.h:214`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10520`): 1 mục, giá trị 0x0; điều kiện None

## `0x0ED4` — St A — `R_0xed4`, `ODM_REG_RX_WAIT_RIFS_11N`

Định nghĩa: `R_0xed4 hal/phydm/phydm_regtable.h:818`; `ODM_REG_RX_WAIT_RIFS_11N hal/phydm/phydm_regdefine11n.h:215`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10521`): 1 mục, giá trị 0x0; điều kiện None

## `0x0ED8` — St A — `R_0xed8`, `ODM_REG_RX2RX_11N`

Định nghĩa: `R_0xed8 hal/phydm/phydm_regtable.h:819`; `ODM_REG_RX2RX_11N hal/phydm/phydm_regdefine11n.h:216`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10522`): 1 mục, giá trị 0x0; điều kiện None

## `0x0EDC` — St A — `R_0xedc`, `ODM_REG_STANDBY_11N`

Định nghĩa: `R_0xedc hal/phydm/phydm_regtable.h:820`; `ODM_REG_STANDBY_11N hal/phydm/phydm_regdefine11n.h:217`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10523`): 1 mục, giá trị 0x0; điều kiện None

## `0x0EE0` — St A — `R_0xee0`, `ODM_REG_SLEEP_11N`

Định nghĩa: `R_0xee0 hal/phydm/phydm_regtable.h:821`; `ODM_REG_SLEEP_11N hal/phydm/phydm_regdefine11n.h:218`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10524`): 1 mục, giá trị 0x0; điều kiện None

## `0x0EE4` — St A — *(không có macro)*

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10525`): 1 mục, giá trị 0x0; điều kiện None

## `0x0EE8` — St A — `R_0xee8`

Định nghĩa: `R_0xee8 hal/phydm/phydm_regtable.h:822`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10526`): 1 mục, giá trị 0x0; điều kiện None

## `0x0EEC` — St A — `R_0xeec`, `ODM_REG_PMPD_ANAEN_11N`

Định nghĩa: `R_0xeec hal/phydm/phydm_regtable.h:823`; `ODM_REG_PMPD_ANAEN_11N hal/phydm/phydm_regdefine11n.h:219`

- Bảng `array_mp_8822b_phy_reg` (`halhwimg8822b_bb.c:10527`): 1 mục, giá trị 0x0; điều kiện None

## `0x0F04` — St A — `R_0xf04`, `ODM_REG_CCK_CRC32_CNT_11AC`

Định nghĩa: `R_0xf04 hal/phydm/phydm_regtable.h:826`; `ODM_REG_CCK_CRC32_CNT_11AC hal/phydm/phydm_regdefine11ac.h:99`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16909` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | 0x3FFF | live | `cckok	= phy_query_bb_reg(padapter, 0xF04, 0x3FFF);	     /* [13:0] */` |
| `hal/hal_com.c:16913` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | 0x3FFF0000 | live | `cckcrc	= phy_query_bb_reg(padapter, 0xF04, 0x3FFF0000); /* [29:16]	 */` |
| `hal/phydm/phydm_dig.c:1504` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `ret_value = odm_get_bb_reg(dm, ODM_REG_CCK_CRC32_CNT_11AC, MASKDWORD);` |

## `0x0F08` — St A — `R_0xf08`, `ODM_REG_CCK_CCA_CNT_11AC`

Định nghĩa: `R_0xf08 hal/phydm/phydm_regtable.h:827`; `ODM_REG_CCK_CCA_CNT_11AC hal/phydm/phydm_regdefine11ac.h:100`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:549` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | 0xFFFF0000 | live | `odm_get_bb_reg(dm, R_0xf08, 0xFFFF0000));` |
| `hal/phydm/phydm_dig.c:1499` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `ret_value = odm_get_bb_reg(dm, ODM_REG_CCK_CCA_CNT_11AC, MASKDWORD);` |

## `0x0F0C` — St A — `R_0xf0c`, `ODM_REG_VHT_CRC32_CNT_11AC`

Định nghĩa: `R_0xf0c hal/phydm/phydm_regtable.h:828`; `ODM_REG_VHT_CRC32_CNT_11AC hal/phydm/phydm_regdefine11ac.h:101`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16912` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | 0x3FFF | live | `vht_ok	= phy_query_bb_reg(padapter, 0xF0C, 0x3FFF);     /* [13:0] */` |
| `hal/hal_com.c:16916` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | 0x3FFF0000 | live | `vht_err	= phy_query_bb_reg(padapter, 0xF0C, 0x3FFF0000); /* [29:16] */` |
| `hal/phydm/phydm_dig.c:1519` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `ret_value = odm_get_bb_reg(dm, ODM_REG_VHT_CRC32_CNT_11AC, MASKDWORD);` |

## `0x0F10` — St A — `R_0xf10`, `ODM_REG_HT_CRC32_CNT_11AC`

Định nghĩa: `R_0xf10 hal/phydm/phydm_regtable.h:829`; `ODM_REG_HT_CRC32_CNT_11AC hal/phydm/phydm_regdefine11ac.h:102`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16911` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | 0x3FFF | live | `htok		= phy_query_bb_reg(padapter, 0xF10, 0x3FFF);     /* [13:0] */` |
| `hal/hal_com.c:16915` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | 0x3FFF0000 | live | `htcrc	= phy_query_bb_reg(padapter, 0xF10, 0x3FFF0000); /* [29:16] */` |
| `hal/phydm/phydm_dig.c:1514` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `ret_value = odm_get_bb_reg(dm, ODM_REG_HT_CRC32_CNT_11AC, MASKDWORD);` |

## `0x0F14` — St A — `R_0xf14`, `ODM_REG_PAGE_F_RST_11N`, `ODM_REG_OFDM_CRC32_CNT_11AC`

Định nghĩa: `R_0xf14 hal/phydm/phydm_regtable.h:830`; `ODM_REG_OFDM_CRC32_CNT_11AC hal/phydm/phydm_regdefine11ac.h:103`; `ODM_REG_PAGE_F_RST_11N hal/phydm/phydm_regdefine11n.h:221`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16910` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | 0x3FFF | live | `ofdmok	= phy_query_bb_reg(padapter, 0xF14, 0x3FFF);	     /* [13:0] */` |
| `hal/hal_com.c:16914` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | 0x3FFF0000 | live | `ofdmcrc	= phy_query_bb_reg(padapter, 0xF14, 0x3FFF0000); /* [29:16] */` |
| `hal/phydm/phydm_dig.c:1509` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `ret_value = odm_get_bb_reg(dm, ODM_REG_OFDM_CRC32_CNT_11AC, MASKDWORD);` |
| `hal/hal_com.c:16970` | rtw_reset_phy_trx_ok_counters | phy_set_bb_reg | 32 | BIT16 | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_set_bb_reg(padapter, 0xF14, BIT16, 0x1);` |
| `hal/hal_com.c:16971` | rtw_reset_phy_trx_ok_counters | phy_set_bb_reg | 32 | BIT16 | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_set_bb_reg(padapter, 0xF14, BIT16, 0x0);` |
| `hal/hal_com.c:16997` | rtw_reset_phy_rx_counters | phy_set_bb_reg | 32 | BIT16 | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_set_bb_reg(padapter, 0xF14, BIT16, 0x1);` |
| `hal/hal_com.c:16999` | rtw_reset_phy_rx_counters | phy_set_bb_reg | 32 | BIT16 | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `phy_set_bb_reg(padapter, 0xF14, BIT16, 0x0);` |
| `hal/phydm/phydm_api.c:80` | phydm_reset_bb_hw_cnt | odm_set_bb_reg | 32 | BIT(16) | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xf14, BIT(16), 0x1);` |
| `hal/phydm/phydm_api.c:81` | phydm_reset_bb_hw_cnt | odm_set_bb_reg | 32 | BIT(16) | dead F:dm->support_ic_type & ODM_IC_11N_SERIES | `odm_set_bb_reg(dm, R_0xf14, BIT(16), 0x0);` |

## `0x0F20` — St G — `R_0xf20`

Định nghĩa: `R_0xf20 hal/phydm/phydm_regtable.h:831`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:617` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xf20, MASKDWORD);` |

## `0x0F2C` — St G — `R_0xf2c`

Định nghĩa: `R_0xf2c hal/phydm/phydm_regtable.h:832`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:636` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xf2c, MASKDWORD);` |
| `hal/phydm/phydm_debug.c:673` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xf2c, MASKDWORD);` |

## `0x0F30` — St G — `R_0xf30`

Định nghĩa: `R_0xf30 hal/phydm/phydm_regtable.h:833`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:648` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xf30, MASKDWORD);` |
| `hal/phydm/phydm_debug.c:693` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xf30, MASKDWORD);` |

## `0x0F34` — St G — `R_0xf34`

Định nghĩa: `R_0xf34 hal/phydm/phydm_regtable.h:834`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:714` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xf34, MASKDWORD);` |

## `0x0F44` — St A — `R_0xf44`

Định nghĩa: `R_0xf44 hal/phydm/phydm_regtable.h:836`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1473` | phydm_dynamic_spur_det_eliminate | odm_get_bb_reg | 32 | MASKLWORD | live | `psd_set[j] = odm_get_bb_reg(dm, R_0xf44,` |
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1495` | phydm_dynamic_spur_det_eliminate | odm_get_bb_reg | 32 | MASKLWORD | live | `psd_set_B[j] = odm_get_bb_reg(dm, R_0xf44,` |

## `0x0F48` — St A — `R_0xf48`, `ODM_REG_OFDM_FA_11AC`

Định nghĩa: `R_0xf48 hal/phydm/phydm_regtable.h:837`; `ODM_REG_OFDM_FA_11AC hal/phydm/phydm_regdefine11ac.h:104`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_com.c:16918` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskLWord | live | `OFDM_FA	= phy_query_bb_reg(padapter, 0xF48, bMaskLWord);` |
| `hal/phydm/phydm_dig.c:1493` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKLWORD | live | `fa_t->cnt_ofdm_fail = odm_get_bb_reg(dm, R_0xf48, MASKLWORD);` |

## `0x0F4C` — St A — `R_0xf4c`

Định nghĩa: `R_0xf4c hal/phydm/phydm_regtable.h:838`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/rtl8822b/rtl8822b_phy.c:1521` | _sounding_config_mu | phy_query_bb_reg | 32 | 0xFFFF0000 | live | `val32 = phy_query_bb_reg(adapter, 0xF4C, 0xFFFF0000);` |

## `0x0F50` — St M — `R_0xf50`

Định nghĩa: `R_0xf50 hal/phydm/phydm_regtable.h:839`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/hal_mp.c:2093` | hal_mpt_query_phytxok | phy_query_bb_reg | 32 | bMaskLWord | live | `count = phy_query_bb_reg(pAdapter, 0xF50, bMaskLWord); /* [15:0]*/` |
| `hal/hal_mp.c:2095` | hal_mpt_query_phytxok | phy_query_bb_reg | 32 | bMaskHWord | live | `count = phy_query_bb_reg(pAdapter, 0xF50, bMaskHWord); /* [31:16]*/` |
| `hal/hal_mp.c:2349` | mpt_ProSetPMacTx | phy_query_bb_reg | 32 | bMaskLWord | live | `u4bTmp = phy_query_bb_reg(Adapter, 0xf50, bMaskLWord);` |

## `0x0F80` — St G — `R_0xf80`

Định nghĩa: `R_0xf80 hal/phydm/phydm_regtable.h:845`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_auto_dbg.c:397` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xf80, MASKDWORD);` |
| `hal/phydm/phydm_debug.c:479` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xf80, MASKDWORD);` |

## `0x0F84` — St A — `R_0xf84`, `ODM_REG_IGI_C_11N`, `ODM_REG_CCK_CRC32_ERROR_CNT_11N`

Định nghĩa: `R_0xf84 hal/phydm/phydm_regtable.h:846`; `ODM_REG_IGI_C_11N hal/phydm/phydm_regdefine11n.h:222`; `ODM_REG_CCK_CRC32_ERROR_CNT_11N hal/phydm/phydm_regdefine11n.h:224`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:972` | phydm_get_condi_num_8822b | odm_get_bb_reg | 32 | (BIT(17) \| BIT(16) \| M | live | `val = odm_get_bb_reg(dm, R_0xf84, (BIT(17) \| BIT(16) \| MASKLWORD));` |
| `hal/hal_com.c:16936` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskDWord | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `cckcrc	= phy_query_bb_reg(padapter, 0xF84, bMaskDWord);` |

## `0x0F87` — St A — `R_0xf87`

Định nghĩa: `R_0xf87 hal/phydm/phydm_regtable.h:847`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c:953` | phydm_get_condi_num_acc_8822b | odm_read_1byte | 8 |  | live | `dbg->condi_num_cdf[i] = odm_read_1byte(dm, R_0xf87);` |

## `0x0F88` — St G — `R_0xf88`, `ODM_REG_IGI_D_11N`, `ODM_REG_CCK_CRC32_OK_CNT_11N`

Định nghĩa: `R_0xf88 hal/phydm/phydm_regtable.h:848`; `ODM_REG_IGI_D_11N hal/phydm/phydm_regdefine11n.h:223`; `ODM_REG_CCK_CRC32_OK_CNT_11N hal/phydm/phydm_regdefine11n.h:225`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:584` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xf88, MASKDWORD);` |
| `hal/hal_com.c:16932` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskDWord | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `cckok	= phy_query_bb_reg(padapter, 0xF88, bMaskDWord);` |

## `0x0F8C` — St G — `R_0xf8c`

Định nghĩa: `R_0xf8c hal/phydm/phydm_regtable.h:849`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:576` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xf8c, MASKDWORD);` |
| `hal/phydm/phydm_debug.c:601` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xf8c, MASKDWORD);` |
| `hal/phydm/phydm_debug.c:4183` | phydm_per_tone_evm | odm_read_4byte | 32 |  | live | `val = odm_read_4byte(dm, R_0xf8c);` |
| `hal/phydm/phydm_debug.c:4210` | phydm_per_tone_evm | odm_read_4byte | 32 |  | live | `val = odm_read_4byte(dm, R_0xf8c);` |
| `hal/phydm/phydm_debug.c:4237` | phydm_per_tone_evm | odm_read_4byte | 32 |  | live | `val = odm_read_4byte(dm, R_0xf8c);` |
| `hal/phydm/phydm_debug.c:4265` | phydm_per_tone_evm | odm_read_4byte | 32 |  | live | `val = odm_read_4byte(dm, R_0xf8c);` |
| `hal/phydm/phydm_debug.c:4292` | phydm_per_tone_evm | odm_read_4byte | 32 |  | live | `val = odm_read_4byte(dm, R_0xf8c);` |
| `hal/phydm/phydm_debug.c:4319` | phydm_per_tone_evm | odm_read_4byte | 32 |  | live | `val = odm_read_4byte(dm, R_0xf8c);` |
| `hal/phydm/phydm_soml.c:84` | phydm_dynamicsoftmletting | odm_get_bb_reg | 32 | MASKBYTE0 | no-caller T:dm->support_ic_type & ODM_RTL8822B | `ret_val = odm_get_bb_reg(dm, R_0xf8c, MASKBYTE0);` |

## `0x0F90` — St G — `R_0xf90`, `ODM_REG_HT_CRC32_CNT_11N`

Định nghĩa: `R_0xf90 hal/phydm/phydm_regtable.h:850`; `ODM_REG_HT_CRC32_CNT_11N hal/phydm/phydm_regdefine11n.h:226`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:519` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKBYTE1 | live | `pwdb = (u8)odm_get_bb_reg(dm, R_0xf90, MASKBYTE1);` |
| `hal/hal_com.c:16934` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskLWord | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `htok		= phy_query_bb_reg(padapter, 0xF90, bMaskLWord);` |
| `hal/hal_com.c:16938` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskHWord | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `htcrc	= phy_query_bb_reg(padapter, 0xF90, bMaskHWord);` |

## `0x0F94` — St A — `R_0xf94`, `ODM_REG_OFDM_CRC32_CNT_11N`

Định nghĩa: `R_0xf94 hal/phydm/phydm_regtable.h:851`; `ODM_REG_OFDM_CRC32_CNT_11N hal/phydm/phydm_regdefine11n.h:227`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:477` | phydm_dynamic_ant_weighting_8822b | odm_get_bb_reg | 32 | BIT(0) \| BIT(1) \| BIT( | live | `reg_8 = (u8)odm_get_bb_reg(dm, R_0xf94, BIT(0) \| BIT(1) \| BIT(2));` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:485` | phydm_dynamic_ant_weighting_8822b | odm_get_bb_reg | 32 | BIT(0) \| BIT(1) \| BIT( | live | `reg_8 = (u8)odm_get_bb_reg(dm, R_0xf94, BIT(0) \| BIT(1) \| BIT(2));` |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c:493` | phydm_dynamic_ant_weighting_8822b | odm_get_bb_reg | 32 | BIT(0) \| BIT(1) \| BIT( | live | `reg_8 = (u8)odm_get_bb_reg(dm, R_0xf94, BIT(0) \| BIT(1) \| BIT(2));` |
| `hal/hal_com.c:16933` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskLWord | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `ofdmok	= phy_query_bb_reg(padapter, 0xF94, bMaskLWord);` |
| `hal/hal_com.c:16937` | rtw_dump_phy_rx_counters | phy_query_bb_reg | 32 | bMaskHWord | dead F:IS_HARDWARE_TYPE_JAGUAR(padapter) \|\| IS_HARDWARE_TYPE_JAGUAR | `ofdmcrc	= phy_query_bb_reg(padapter, 0xF94, bMaskHWord);` |

## `0x0FA0` — St A — `R_0xfa0`, `ODM_REG_RPT_11AC`

Định nghĩa: `R_0xfa0 hal/phydm/phydm_regtable.h:853`; `ODM_REG_RPT_11AC hal/phydm/phydm_regdefine11ac.h:111`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:262` | _iqk_bb_reset_8822b | odm_get_bb_reg | 32 | BIT(3) | live | `cca_ing = (boolean)odm_get_bb_reg(dm, R_0xfa0, BIT(3));` |
| `hal/phydm/phydm_auto_dbg.c:411` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:423` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:435` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:447` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:459` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:472` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:484` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:496` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:508` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:520` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:532` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:544` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:556` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:565` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:574` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_auto_dbg.c:583` | phydm_dbg_port_dump_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, ODM_REG_RPT_11AC, MASKDWORD);` |
| `hal/phydm/phydm_debug.c:207` | phydm_get_bb_dbg_port_val | odm_get_bb_reg | 32 | MASKDWORD | live | `dbg_port_value = odm_get_bb_reg(dm, R_0xfa0, MASKDWORD);` |
| `hal/phydm/phydm_noisemonitor.c:328` | odm_inband_noise_monitor_ac | odm_get_bb_reg | 32 | MASKDWORD | dead F:dm->support_ic_type & (ODM_RTL8822B \| ODM_RTL8821C) | `value32 = odm_get_bb_reg(dm, R_0xfa0, MASKDWORD);` |

## `0x0FA4` — St A — `R_0xfa4`, `ODM_REG_CLM_RESULT_11AC`

Định nghĩa: `R_0xfa4 hal/phydm/phydm_regtable.h:854`; `ODM_REG_CLM_RESULT_11AC hal/phydm/phydm_regdefine11ac.h:112`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_ccx.c:1774` | phydm_clm_get_result | odm_get_bb_reg | 32 | MASKLWORD | live | `val = odm_get_bb_reg(dm, R_0xfa4, MASKLWORD);` |

## `0x0FA8` — St A — `R_0xfa8`, `ODM_REG_NHM_CNT_11AC`

Định nghĩa: `R_0xfa8 hal/phydm/phydm_regtable.h:855`; `ODM_REG_NHM_CNT_11AC hal/phydm/phydm_regdefine11ac.h:113`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_ccx.c:610` | phydm_nhm_get_result | odm_read_4byte | 32 |  | live | `value32 = odm_read_4byte(dm, R_0xfa8);` |

## `0x0FAC` — St A — `R_0xfac`, `ODM_REG_NHM_CNT7_TO_CNT4_11AC`

Định nghĩa: `R_0xfac hal/phydm/phydm_regtable.h:856`; `ODM_REG_NHM_CNT7_TO_CNT4_11AC hal/phydm/phydm_regdefine11ac.h:116`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_ccx.c:613` | phydm_nhm_get_result | odm_read_4byte | 32 |  | live | `value32 = odm_read_4byte(dm, R_0xfac);` |

## `0x0FB0` — St A — `R_0xfb0`, `ODM_REG_NHM_CNT11_TO_CNT8_11AC`

Định nghĩa: `R_0xfb0 hal/phydm/phydm_regtable.h:857`; `ODM_REG_NHM_CNT11_TO_CNT8_11AC hal/phydm/phydm_regdefine11ac.h:117`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_ccx.c:616` | phydm_nhm_get_result | odm_read_4byte | 32 |  | live | `value32 = odm_read_4byte(dm, R_0xfb0);` |

## `0x0FB4` — St A — `R_0xfb4`, `ODM_REG_NHM_DUR_READY_11AC`

Định nghĩa: `R_0xfb4 hal/phydm/phydm_regtable.h:858`; `ODM_REG_NHM_DUR_READY_11AC hal/phydm/phydm_regdefine11ac.h:114`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_ccx.c:620` | phydm_nhm_get_result | odm_read_4byte | 32 |  | live | `value32 = odm_read_4byte(dm, R_0xfb4);` |

## `0x0FBC` — St A — `R_0xfbc`, `ODM_REG_OFDM_FA_TYPE3_11AC`

Định nghĩa: `R_0xfbc hal/phydm/phydm_regtable.h:860`; `ODM_REG_OFDM_FA_TYPE3_11AC hal/phydm/phydm_regdefine11ac.h:107`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:564` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xfbc, MASKDWORD);` |
| `hal/phydm/phydm_dig.c:1477` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `ret_value = odm_get_bb_reg(dm, ODM_REG_OFDM_FA_TYPE3_11AC, MASKDWORD);` |

## `0x0FC0` — St A — `R_0xfc0`, `ODM_REG_OFDM_FA_TYPE4_11AC`

Định nghĩa: `R_0xfc0 hal/phydm/phydm_regtable.h:861`; `ODM_REG_OFDM_FA_TYPE4_11AC hal/phydm/phydm_regdefine11ac.h:108`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:572` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | (0xFFFF0000 >> 16) | live | `odm_get_bb_reg(dm, R_0xfc0, (0xFFFF0000 >> 16)),` |
| `hal/phydm/phydm_dig.c:1481` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `ret_value = odm_get_bb_reg(dm, ODM_REG_OFDM_FA_TYPE4_11AC, MASKDWORD);` |

## `0x0FC4` — St A — `R_0xfc4`, `ODM_REG_OFDM_FA_TYPE5_11AC`

Định nghĩa: `R_0xfc4 hal/phydm/phydm_regtable.h:862`; `ODM_REG_OFDM_FA_TYPE5_11AC hal/phydm/phydm_regdefine11ac.h:109`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:555` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `value32 = odm_get_bb_reg(dm, R_0xfc4, MASKDWORD);` |
| `hal/phydm/phydm_dig.c:1485` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `ret_value = odm_get_bb_reg(dm, ODM_REG_OFDM_FA_TYPE5_11AC, MASKDWORD);` |

## `0x0FC8` — St A — `R_0xfc8`, `ODM_REG_OFDM_FA_TYPE6_11AC`

Định nghĩa: `R_0xfc8 hal/phydm/phydm_regtable.h:863`; `ODM_REG_OFDM_FA_TYPE6_11AC hal/phydm/phydm_regdefine11ac.h:110`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:573` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | 0xFFFF | live | `odm_get_bb_reg(dm, R_0xfc8, 0xFFFF));` |
| `hal/phydm/phydm_dig.c:1489` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `ret_value = odm_get_bb_reg(dm, ODM_REG_OFDM_FA_TYPE6_11AC, MASKDWORD);` |

## `0x0FCC` — St A — `R_0xfcc`, `ODM_REG_OFDM_FA_TYPE1_11AC`

Định nghĩa: `R_0xfcc hal/phydm/phydm_regtable.h:864`; `ODM_REG_OFDM_FA_TYPE1_11AC hal/phydm/phydm_regdefine11ac.h:105`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:562` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | 0xFFFF | live | `odm_get_bb_reg(dm, R_0xfcc, 0xFFFF));` |
| `hal/phydm/phydm_dig.c:1471` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `ret_value = odm_get_bb_reg(dm, ODM_REG_OFDM_FA_TYPE1_11AC, MASKDWORD);` |

## `0x0FD0` — St A — `R_0xfd0`, `ODM_REG_OFDM_FA_TYPE2_11AC`

Định nghĩa: `R_0xfd0 hal/phydm/phydm_regtable.h:865`; `ODM_REG_OFDM_FA_TYPE2_11AC hal/phydm/phydm_regdefine11ac.h:106`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_debug.c:553` | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | 32 | 0xFFFF | live | `odm_get_bb_reg(dm, R_0xfd0, 0xFFFF));` |
| `hal/phydm/phydm_dig.c:1474` | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `ret_value = odm_get_bb_reg(dm, ODM_REG_OFDM_FA_TYPE2_11AC, MASKDWORD);` |

## `0x0FF0` — St A — `R_0xff0`

Định nghĩa: `R_0xff0 hal/phydm/phydm_regtable.h:866`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/phydm/phydm_noisemonitor.c:222` | phydm_idle_noise_measure_ac | odm_get_bb_reg | 32 | MASKDWORD | live | `tmp4b = odm_get_bb_reg(dm, R_0xff0, MASKDWORD);` |

