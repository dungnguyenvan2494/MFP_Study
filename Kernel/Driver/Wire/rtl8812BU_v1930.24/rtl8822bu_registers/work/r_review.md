# R review — 44 entries

## BB 0x0D08 — R_0xd08, rA_SIRead_Jaguar  (1 tham chiếu active)

- `hal/rtl8822b/rtl8822b_phy.c:93` `bb_rf_register_definition()` — `hal->PHYRegDef[RF_PATH_A].rfLSSIReadBack = rA_SIRead_Jaguar;`

## BB 0x1A9C — R_0x1a9c  (1 tham chiếu active)

- `hal/phydm/phydm.c:165` `phydm_cck_new_agc_chk()` — `new_agc_addr = R_0x1a9c;`

## MAC 0x0044 — REG_GPIO_PIN_CTRL_8822B, R_0x44, REG_GPIO_PIN_CTRL  (15 tham chiếu active)

- `hal/hal_com.c:4686` `rtw_hal_set_output_gpio()` — `rtw_write8(padapter, REG_GPIO_PIN_CTRL + 3,`
- `hal/hal_com.c:4687` `rtw_hal_set_output_gpio()` — `rtw_read8(padapter, REG_GPIO_PIN_CTRL + 3) & ~BIT(index));`
- `hal/hal_com.c:4692` `rtw_hal_set_output_gpio()` — `rtw_write8(padapter, REG_GPIO_PIN_CTRL + 2,`
- `hal/hal_com.c:4693` `rtw_hal_set_output_gpio()` — `rtw_read8(padapter, REG_GPIO_PIN_CTRL + 2) | BIT(index));`
- `hal/hal_com.c:4697` `rtw_hal_set_output_gpio()` — `rtw_write8(padapter, REG_GPIO_PIN_CTRL + 1,`
- `hal/hal_com.c:4698` `rtw_hal_set_output_gpio()` — `rtw_read8(padapter, REG_GPIO_PIN_CTRL + 1) | BIT(index));`
- `hal/hal_com.c:4700` `rtw_hal_set_output_gpio()` — `rtw_write8(padapter, REG_GPIO_PIN_CTRL + 1,`
- `hal/hal_com.c:4701` `rtw_hal_set_output_gpio()` — `rtw_read8(padapter, REG_GPIO_PIN_CTRL + 1) & ~BIT(index));`
- `hal/hal_com.c:4742` `rtw_hal_set_input_gpio()` — `rtw_write8(padapter, REG_GPIO_PIN_CTRL + 3,`
- `hal/hal_com.c:4743` `rtw_hal_set_input_gpio()` — `rtw_read8(padapter, REG_GPIO_PIN_CTRL + 3) & ~BIT(index));`
- `hal/hal_com.c:4748` `rtw_hal_set_input_gpio()` — `rtw_write8(padapter, REG_GPIO_PIN_CTRL + 2,`
- `hal/hal_com.c:4749` `rtw_hal_set_input_gpio()` — `rtw_read8(padapter, REG_GPIO_PIN_CTRL + 2) & ~BIT(index));`
- … +3

## MAC 0x0060 — REG_GPIO_EXT_CTRL_8822B, R_0x60, REG_GPIO_EXT_CTRL  (15 tham chiếu active)

- `hal/hal_com.c:4712` `rtw_hal_set_output_gpio()` — `rtw_write8(padapter, REG_GPIO_PIN_CTRL_2 + 3,`
- `hal/hal_com.c:4713` `rtw_hal_set_output_gpio()` — `rtw_read8(padapter, REG_GPIO_PIN_CTRL_2 + 3) & ~BIT(index));`
- `hal/hal_com.c:4718` `rtw_hal_set_output_gpio()` — `rtw_write8(padapter, REG_GPIO_PIN_CTRL_2 + 2,`
- `hal/hal_com.c:4719` `rtw_hal_set_output_gpio()` — `rtw_read8(padapter, REG_GPIO_PIN_CTRL_2 + 2) | BIT(index));`
- `hal/hal_com.c:4723` `rtw_hal_set_output_gpio()` — `rtw_write8(padapter, REG_GPIO_PIN_CTRL_2 + 1,`
- `hal/hal_com.c:4724` `rtw_hal_set_output_gpio()` — `rtw_read8(padapter, REG_GPIO_PIN_CTRL_2 + 1) | BIT(index));`
- `hal/hal_com.c:4726` `rtw_hal_set_output_gpio()` — `rtw_write8(padapter, REG_GPIO_PIN_CTRL_2 + 1,`
- `hal/hal_com.c:4727` `rtw_hal_set_output_gpio()` — `rtw_read8(padapter, REG_GPIO_PIN_CTRL_2 + 1) & ~BIT(index));`
- `hal/hal_com.c:4760` `rtw_hal_set_input_gpio()` — `rtw_write8(padapter, REG_GPIO_PIN_CTRL_2 + 3,`
- `hal/hal_com.c:4761` `rtw_hal_set_input_gpio()` — `rtw_read8(padapter, REG_GPIO_PIN_CTRL_2 + 3) & ~BIT(index));`
- `hal/hal_com.c:4766` `rtw_hal_set_input_gpio()` — `rtw_write8(padapter, REG_GPIO_PIN_CTRL_2 + 2,`
- `hal/hal_com.c:4767` `rtw_hal_set_input_gpio()` — `rtw_read8(padapter, REG_GPIO_PIN_CTRL_2 + 2) & ~BIT(index));`
- … +3

## MAC 0x00CC — REG_PMC_DBG_CTRL2_8822B, REG_PMC_DBG_CTRL2  (2 tham chiếu active)

- `hal/halmac/halmac_88xx/halmac_efuse_88xx.c:1062` `None()` — `HALMAC_REG_W8(REG_PMC_DBG_CTRL2 + 3, unlock_code);`
- `hal/halmac/halmac_88xx/halmac_efuse_88xx.c:1089` `None()` — `HALMAC_REG_W8(REG_PMC_DBG_CTRL2 + 3, 0x00);`

## MAC 0x00FC — REG_SYS_CFG2_8822B, REG_SYS_CFG2  (6 tham chiếu active)

- `hal/halmac/halmac_api.c:556` `None()` — `REG_SYS_CFG2);`
- `hal/halmac/halmac_api.c:560` `None()` — `chip_id = pltfm_api->REG_READ_8(drv_adapter, REG_SYS_CFG2);`
- `hal/halmac/halmac_88xx/halmac_usb_88xx.c:87` `None()` — `if (HALMAC_REG_R8(REG_SYS_CFG2 + 3) == 0x20) {`
- `hal/halmac/halmac_88xx/halmac_usb_88xx.c:162` `None()` — `if (HALMAC_REG_R8(REG_SYS_CFG2 + 3) == 0x20) {`
- `hal/halmac/halmac_88xx/halmac_usb_88xx.c:471` `None()` — `cur_mode = (HALMAC_REG_R8(REG_SYS_CFG2 + 3) == 0x20) ?`
- `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:1005` `None()` — `if (HALMAC_REG_R8(REG_SYS_CFG2 + 3) == 0x20)`

## MAC 0x0114 — REG_TRXFF_BNDY_8822B, REG_TRXFF_BNDY  (2 tham chiếu active)

- `hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:121` `None()` — `value8 = HALMAC_REG_R8(REG_TRXFF_BNDY + 1);`
- `hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:125` `None()` — `HALMAC_REG_W8(REG_TRXFF_BNDY + 1, value8);`

## MAC 0x01D0 — REG_HMEBOX0_8822B, REG_HMEBOX0  (1 tham chiếu active)

- `hal/hal_halmac.c:4135` `rtw_halmac_send_h2c()` — `msgbox_addr = REG_HMEBOX0 + (h2c_box_num * MESSAGE_BOX_SIZE);`

## MAC 0x01F0 — REG_HMEBOX_E0_8822B, REG_HMEBOX_E0  (1 tham chiếu active)

- `hal/hal_halmac.c:4129` `rtw_halmac_send_h2c()` — `msgbox_ex_addr = REG_HMEBOX_E0 + (h2c_box_num * EX_MESSAGE_BOX_SIZE);`

## MAC 0x04D0 — REG_MACID_SLEEP2_8822B, REG_MACID_SLEEP2, ODM_REG_EARLY_MODE_11N  (1 tham chiếu active)

- `hal/rtl8822b/rtl8822b_halinit.c:105` `rtl8822b_init_hal_spec()` — `, REG_MACID_SLEEP2_8822B`

## MAC 0x04D8 — REG_HW_SEQ0_8822B, REG_HW_SEQ0  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:2217` `rtl8822b_sethwreg()` — `reg_hw_ssn = REG_HW_SEQ0_8822B;`
- `hal/rtl8822b/rtl8822b_ops.c:2258` `rtl8822b_sethwreg()` — `reg_hw_ssn = REG_HW_SEQ0_8822B;`

## MAC 0x04DA — REG_HW_SEQ1_8822B, REG_HW_SEQ1  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:2211` `rtl8822b_sethwreg()` — `reg_hw_ssn = REG_HW_SEQ1_8822B;`
- `hal/rtl8822b/rtl8822b_ops.c:2252` `rtl8822b_sethwreg()` — `reg_hw_ssn = REG_HW_SEQ1_8822B;`

## MAC 0x04DC — REG_HW_SEQ2_8822B, REG_HW_SEQ2  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:2213` `rtl8822b_sethwreg()` — `reg_hw_ssn = REG_HW_SEQ2_8822B;`
- `hal/rtl8822b/rtl8822b_ops.c:2254` `rtl8822b_sethwreg()` — `reg_hw_ssn = REG_HW_SEQ2_8822B;`

## MAC 0x04DE — REG_HW_SEQ3_8822B, REG_HW_SEQ3  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:2215` `rtl8822b_sethwreg()` — `reg_hw_ssn = REG_HW_SEQ3_8822B;`
- `hal/rtl8822b/rtl8822b_ops.c:2256` `rtl8822b_sethwreg()` — `reg_hw_ssn = REG_HW_SEQ3_8822B;`

## MAC 0x0618 — REG_BSSID_8822B, REG_BSSID  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:77` `None()` — `.bssid = REG_BSSID_8822B,`
- `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:152` `None()` — `offset = REG_BSSID;`

## MAC 0x0650 — REG_NAV_CTRL_8822B, REG_NAV_CTRL  (1 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:2345` `rtl8822b_sethwreg()` — `rtw_write8(adapter, REG_NAV_CTRL_8822B + 2, (u8)usNavUpper);`

## MAC 0x06A8 — REG_BCN_PSR_RPT_8822B, REG_BCN_PSR_RPT  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:84` `None()` — `.ps_aid = REG_BCN_PSR_RPT_8822B,`
- `hal/hal_halmac.c:2399` `rtw_halmac_set_aid()` — `addr = REG_BCN_PSR_RPT;`

## MAC 0x0708 — REG_BSSID1_8822B, REG_BSSID1  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:92` `None()` — `.bssid = REG_BSSID1_8822B,`
- `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:155` `None()` — `offset = REG_BSSID1;`

## MAC 0x0710 — REG_BCN_PSR_RPT1_8822B, REG_BCN_PSR_RPT1  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:99` `None()` — `.ps_aid = REG_BCN_PSR_RPT1_8822B,`
- `hal/hal_halmac.c:2406` `rtw_halmac_set_aid()` — `addr = REG_BCN_PSR_RPT1;`

## MAC 0x07C4 — R_0x7c4, REG_LTECOEX_WRITE_DATA  (1 tham chiếu active)

- `hal/phydm/phydm_adc_sampling.c:935` `phydm_la_set_mac_iq_dump()` — `reg2 = R_0x7c4;`

## MAC 0x07C8 — R_0x7c8, REG_LTECOEX_READ_DATA  (1 tham chiếu active)

- `hal/phydm/phydm_adc_sampling.c:936` `phydm_la_set_mac_iq_dump()` — `reg3 = R_0x7c8;`

## MAC 0x07D0 — REG_WMAC_OPTION_FUNCTION_8822B, REG_WMAC_OPTION_FUNCTION  (4 tham chiếu active)

- `hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:135` `None()` — `value32 = HALMAC_REG_R32(REG_WMAC_OPTION_FUNCTION + 4);`
- `hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:141` `None()` — `HALMAC_REG_W32(REG_WMAC_OPTION_FUNCTION + 4, value32);`
- `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:962` `None()` — `HALMAC_REG_W32(REG_WMAC_OPTION_FUNCTION + 8, WLAN_MAC_OPT_FUNC2);`
- `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:969` `None()` — `HALMAC_REG_W8(REG_WMAC_OPTION_FUNCTION + 4, value8);`

## MAC 0x07F4 — R_0x7f4  (1 tham chiếu active)

- `hal/phydm/phydm_adc_sampling.c:931` `phydm_la_set_mac_iq_dump()` — `reg2 = R_0x7f4;`

## MAC 0x07F8 — REG_RTS_ADDRESS_1_8822B, R_0x7f8, REG_RTS_ADDRESS_1  (1 tham chiếu active)

- `hal/phydm/phydm_adc_sampling.c:932` `phydm_la_set_mac_iq_dump()` — `reg3 = R_0x7f8;`

## MAC 0x10C0 — REG_SW_MDIO_8822B, REG_SW_MDIO  (2 tham chiếu active)

- `hal/halmac/halmac_88xx/halmac_usb_88xx.c:490` `None()` — `HALMAC_REG_W8_CLR(REG_SW_MDIO + 3, BIT(0));`
- `hal/halmac/halmac_88xx/halmac_8822b/halmac_usb_8822b.c:131` `None()` — `if ((HALMAC_REG_R8(REG_SW_MDIO + 3) & BIT(0)) == BIT(0))`

## MAC 0x111C — REG_RXFF_WTR_V1_8822B, REG_RXFF_WTR_V1  (4 tham chiếu active)

- `hal/halmac/halmac_88xx/halmac_common_88xx.c:1413` `None()` — `i = REG_RXFF_WTR_V1;`
- `hal/halmac/halmac_88xx/halmac_common_88xx.c:1419` `None()` — `i = REG_RXFF_WTR_V1;`
- `hal/halmac/halmac_88xx/halmac_common_88xx.c:1441` `None()` — `i = REG_RXFF_WTR_V1;`
- `hal/halmac/halmac_88xx/halmac_common_88xx.c:1447` `None()` — `i = REG_RXFF_WTR_V1;`

## MAC 0x1600 — REG_BCN_PSR_RPT2_8822B, REG_BCN_PSR_RPT2  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:114` `None()` — `.ps_aid = REG_BCN_PSR_RPT2_8822B,`
- `hal/hal_halmac.c:2413` `rtw_halmac_set_aid()` — `addr = REG_BCN_PSR_RPT2;`

## MAC 0x1604 — REG_BCN_PSR_RPT3_8822B, REG_BCN_PSR_RPT3  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:129` `None()` — `.ps_aid = REG_BCN_PSR_RPT3_8822B,`
- `hal/hal_halmac.c:2420` `rtw_halmac_set_aid()` — `addr = REG_BCN_PSR_RPT3;`

## MAC 0x1608 — REG_BCN_PSR_RPT4_8822B, REG_BCN_PSR_RPT4  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:144` `None()` — `.ps_aid = REG_BCN_PSR_RPT4_8822B,`
- `hal/hal_halmac.c:2427` `rtw_halmac_set_aid()` — `addr = REG_BCN_PSR_RPT4;`

## MAC 0x1628 — REG_BSSID2_8822B, REG_BSSID2  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:107` `None()` — `.bssid = REG_BSSID2_8822B,`
- `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:158` `None()` — `offset = REG_BSSID2;`

## MAC 0x1638 — REG_BSSID3_8822B, REG_BSSID3  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:122` `None()` — `.bssid = REG_BSSID3_8822B,`
- `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:161` `None()` — `offset = REG_BSSID3;`

## MAC 0x1648 — REG_BSSID4_8822B, REG_BSSID4  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:137` `None()` — `.bssid = REG_BSSID4_8822B,`
- `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:164` `None()` — `offset = REG_BSSID4;`

## MAC 0x16A0 — REG_TRANSMIT_ADDRSS_0_8822B, REG_TRANSMIT_ADDRSS_0  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:85` `None()` — `.ta = REG_TRANSMIT_ADDRSS_0_8822B,`
- `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:202` `None()` — `offset = REG_TRANSMIT_ADDRSS_0;`

## MAC 0x16A8 — REG_TRANSMIT_ADDRSS_1_8822B, REG_TRANSMIT_ADDRSS_1  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:100` `None()` — `.ta = REG_TRANSMIT_ADDRSS_1_8822B,`
- `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:205` `None()` — `offset = REG_TRANSMIT_ADDRSS_1;`

## MAC 0x16B0 — REG_TRANSMIT_ADDRSS_2_8822B, REG_TRANSMIT_ADDRSS_2  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:115` `None()` — `.ta = REG_TRANSMIT_ADDRSS_2_8822B,`
- `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:208` `None()` — `offset = REG_TRANSMIT_ADDRSS_2;`

## MAC 0x16B8 — REG_TRANSMIT_ADDRSS_3_8822B, REG_TRANSMIT_ADDRSS_3  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:130` `None()` — `.ta = REG_TRANSMIT_ADDRSS_3_8822B,`
- `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:211` `None()` — `offset = REG_TRANSMIT_ADDRSS_3;`

## MAC 0x16C0 — REG_TRANSMIT_ADDRSS_4_8822B, REG_TRANSMIT_ADDRSS_4  (2 tham chiếu active)

- `hal/rtl8822b/rtl8822b_ops.c:145` `None()` — `.ta = REG_TRANSMIT_ADDRSS_4_8822B,`
- `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:214` `None()` — `offset = REG_TRANSMIT_ADDRSS_4;`

## RF-direct 0x2D90 — R_0x2d90  (1 tham chiếu active)

- `hal/phydm/phydm_psd.c:396` `phydm_psd_init()` — `dm_psd_table->psd_report_reg = R_0x2d90;`

## SDIO-local 0x10250040 — REG_SDIO_INDIRECT_REG_CFG_8822B, REG_SDIO_INDIRECT_REG_CFG  (1 tham chiếu active)

- `hal/halmac/halmac_api.c:631` `None()` — `u32 reg_cfg = REG_SDIO_INDIRECT_REG_CFG;`

## SDIO-local 0x10250044 — REG_SDIO_INDIRECT_REG_DATA_8822B, REG_SDIO_INDIRECT_REG_DATA  (1 tham chiếu active)

- `hal/halmac/halmac_api.c:632` `None()` — `u32 reg_data = REG_SDIO_INDIRECT_REG_DATA;`

## SDIO-local 0x10250086 — REG_SDIO_HSUS_CTRL_8822B, REG_SDIO_HSUS_CTRL  (3 tham chiếu active)

- `hal/halmac/halmac_api.c:542` `None()` — `pltfm_reg_w8_sdio(drv_adapter, pltfm_api, REG_SDIO_HSUS_CTRL,`
- `hal/halmac/halmac_api.c:544` `None()` — `REG_SDIO_HSUS_CTRL) &`
- `hal/halmac/halmac_api.c:549` `None()` — `REG_SDIO_HSUS_CTRL) & BIT(1))) {`

## SDIO-local 0x102500C9 — REG_SDIO_CRC_ERR_IDX_8822B, REG_SDIO_CRC_ERR_IDX  (1 tham chiếu active)

- `hal/halmac/halmac_88xx/halmac_common_88xx.c:1405` `None()` — `tmp8 = PLTFM_SDIO_CMD52_R(REG_SDIO_CRC_ERR_IDX);`

## other 0x3A00 — R_0x3a00  (1 tham chiếu active)

- `hal/phydm/phydm_api.c:1999` `phydm_reset_txagc()` — `u32 r_txagc_diff = R_0x3a00;`

## other 0x8000 — R_0x8000  (2 tham chiếu active)

- `hal/phydm/phydm_adc_sampling.c:816` `phydm_la_access_tx_pkt_buf()` — `data_l = odm_read_4byte(dm, R_0x8000 + (addr & 0xfff));`
- `hal/phydm/phydm_adc_sampling.c:817` `phydm_la_access_tx_pkt_buf()` — `data_h = odm_read_4byte(dm, R_0x8000 + (addr & 0xfff) + 4);`

