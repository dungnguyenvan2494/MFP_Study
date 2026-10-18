### MAC: 119 địa chỉ live chưa có ở nhóm khác

| Địa chỉ | Tên | Hàm (≤3) | Vị trí đầu | Nhóm |
|---|---|---|---|---|
| `0x0061` | — | `rtw_hal_set_output_gpio` | `hal/hal_com.c:4723` | khác |
| `0x0062` | — | `rtw_hal_set_output_gpio` | `hal/hal_com.c:4718` | khác |
| `0x0101` | — | `dl_rsvd_page_88xx`, `hw_var_set_dl_rsvd_page` | `hal/halmac/halmac_88xx/halmac_common_88xx.c:384` | khác |
| `0x0103` | — | `priority_queue_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:680` | khác |
| `0x0106` | R_0x0106 | `phydm_la_get_tx_pkt_buf` | `hal/phydm/phydm_adc_sampling.c:851` | LA/ADC |
| `0x0206` | — | `priority_queue_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:632` | khác |
| `0x0286` | — | `rtw_hal_pause_rx_dma` | `hal/hal_com.c:5075` | khác |
| `0x0418` | REG_BCNQ_INFO_8822B | `rtw_halmac_txfifo_wait_empty` | `hal/hal_halmac.c:3822` | khác |
| `0x041A` | REG_TXPKT_EMPTY_8822B | `rtl8822b_gethwreg`, `txfifo_is_empty_88xx` | `hal/halmac/halmac_88xx/halmac_common_88xx.c:3415` | khác |
| `0x041B` | — | `txfifo_is_empty_88xx` | `hal/halmac/halmac_88xx/halmac_common_88xx.c:3418` | khác |
| `0x0426` | REG_LIFETIME_EN_8822B | `cfg_txfifo_lt_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:664` | khác |
| `0x042C` | REG_TXBF_CTRL_8822B | `_config_beamformee_su`, `_reset_beamformee_su`, `_sounding_config_su` … | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:287` | BF/MU |
| `0x042F` | — | `_config_beamformee_mu`, `_sounding_reset_all`, `del_sounding_88xx` … | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:339` | BF/MU |
| `0x0434` | R_0x434 | `phydm_ra_dynamic_retry_count` | `hal/phydm/phydm_rainfo.c:395` | khác |
| `0x0442` | — | `rtw_var_set_basic_rate` | `hal/hal_com.c:14801` | khác |
| `0x0450` | R_0x450 | `phydm_fw_fix_rate` | `hal/phydm/phydm_rainfo.c:203` | khác |
| `0x045D` | REG_NDPA_RATE_8822B | `_config_beamformee_mu`, `cfg_sounding_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:288` | BF/MU |
| `0x045E` | REG_TX_HANG_CTRL_8822B | `init_protocol_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:805` | khác |
| `0x045F` | REG_NDPA_OPT_CTRL_8822B | `_config_beamformee_mu`, `rtl8822b_phy_bf_init` | `hal/rtl8822b/rtl8822b_phy.c:1910` | BF/MU |
| `0x0478` | REG_WMAC_LBK_BUF_HD_V1_8822B | `priority_queue_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:671` | khác |
| `0x0483` | REG_DATA_SC_8822B | `cfg_pri_ch_idx_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:605` | khác |
| `0x04BC` | REG_SW_AMPDU_BURST_MODE_CTRL_8822B | `init_protocol_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:802` | khác |
| `0x04C0` | REG_PKT_LIFE_TIME_8822B | `cfg_txfifo_lt_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:670` | khác |
| `0x04C8` | REG_PROT_MODE_CTRL_8822B | `init_protocol_cfg_8822b`, `phydm_ra_info_init` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:810` | khác |
| `0x04CA` | — | `cfg_ampdu_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:178` | khác |
| `0x04CB` | — | `cfg_ampdu_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:179` | khác |
| `0x04CC` | REG_BAR_MODE_CTRL_8822B | `phydm_ra_info_init` | `hal/phydm/phydm_rainfo.c:1929` | khác |
| `0x04CE` | — | `init_protocol_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:812` | khác |
| `0x04FC` | REG_DUMMY_PAGE4_V1_8822B | `set_opmode_port0` | `hal/rtl8822b/rtl8822b_ops.c:1137` | khác |
| `0x0502` | — | `init_edca_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:910` | khác |
| `0x0504` | REG_EDCA_VI_PARAM_8822B | `rtl8822b_gethwreg`, `rtl8822b_sethwreg` | `hal/rtl8822b/rtl8822b_ops.c:2115` | khác |
| `0x0506` | — | `init_edca_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:911` | khác |
| `0x050C` | REG_EDCA_BK_PARAM_8822B | `rtl8822b_gethwreg`, `rtl8822b_sethwreg` | `hal/rtl8822b/rtl8822b_ops.c:2124` | khác |
| `0x0510` | REG_BCNTCFG_8822B | `InitBeaconParameters` | `hal/rtl8822b/rtl8822b_ops.c:804` | khác |
| `0x0518` | REG_TSFTR_SYN_OFFSET_8822B | `hw_port0_tsf_sync_sel`, `set_opmode_port0` | `hal/hal_com.c:14287` | khác |
| `0x0541` | — | `InitBeaconParameters`, `ResumeTxBeacon`, `StopTxBeacon` | `hal/hal_com.c:17476` | khác |
| `0x0556` | — | `cfg_bcn_space_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:356` | khác |
| `0x055C` | REG_USTIME_TSF_8822B | `cfg_mac_clk_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:763` | khác |
| `0x055F` | REG_RXTSF_OFFSET_OFDM_8822B | `set_beacon_related_registers` | `hal/rtl8822b/rtl8822b_ops.c:881` | khác |
| `0x0564` | — | `rtw_hal_get_tsftr_by_port` | `hal/hal_com.c:14597` | khác |
| `0x0579` | REG_BCN_CTRL_CLINT2_8822B | `rw_bcn_ctrl_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:435` | khác |
| `0x05B0` | REG_DIS_ATIM_8822B | `set_opmode_port0` | `hal/rtl8822b/rtl8822b_ops.c:1090` | khác |
| `0x05B8` | REG_MBSSID_BCN_SPACE2_8822B | `cfg_bcn_space_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:361` | khác |
| `0x05BA` | — | `cfg_bcn_space_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:366` | khác |
| `0x05BC` | REG_MBSSID_BCN_SPACE3_8822B | `cfg_bcn_space_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:371` | khác |
| `0x05C0` | REG_ACMHWCTRL_8822B | `hw_var_set_acm_ctrl` | `hal/rtl8822b/rtl8822b_ops.c:1712` | khác |
| `0x05FC` | REG_PAGE5_DUMMY_8822B | `verify_io_88xx` | `hal/halmac/halmac_88xx/halmac_init_88xx.c:811` | khác |
| `0x0605` | — | `init_wmac_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:956` | khác |
| `0x0606` | — | `init_wmac_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:955` | khác |
| `0x060A` | — | `rx_clk_gate_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1128` | khác |
| `0x0614` | — | `get_mac_addr_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1208` | khác |
| `0x0624` | — | `cfg_multicast_addr_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:523` | BF/MU |
| `0x0638` | REG_USTIME_EDCA_8822B | `cfg_mac_clk_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:764` | khác |
| `0x063C` | REG_RESP_SIFS_CCK_8822B | `rtl8822b_sethwreg` | `hal/rtl8822b/rtl8822b_ops.c:2077` | khác |
| `0x063D` | — | `rtl8822b_sethwreg` | `hal/rtl8822b/rtl8822b_ops.c:2078` | khác |
| `0x063E` | REG_RESP_SIFS_OFDM_8822B | `rtl8822b_sethwreg` | `hal/rtl8822b/rtl8822b_ops.c:2080` | khác |
| `0x063F` | — | `rtl8822b_sethwreg` | `hal/rtl8822b/rtl8822b_ops.c:2081` | khác |
| `0x0652` | — | `rtl8822b_sethwreg` | `hal/rtl8822b/rtl8822b_ops.c:2345` | khác |
| `0x0660` | REG_LBDLY_8822B | `_iqk_bb_reset_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:280` | IQK (driver) |
| `0x0667` | — | `_rx_report` | `hal/rtl8822b/rtl8822b_mac.c:286` | khác |
| `0x0668` | REG_WMAC_TRXPTCL_CTL_8822B | `cfg_bw_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:628` | khác |
| `0x066A` | — | `hw_var_set_ack_preamble` | `hal/rtl8822b/rtl8822b_ops.c:1751` | khác |
| `0x066C` | — | `init_wmac_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:958` | khác |
| `0x0688` | REG_RXFLTMAP3_8822B | `_config_beamformer_mu` | `hal/rtl8822b/rtl8822b_phy.c:1732` | BF/MU |
| `0x068A` | REG_RXFLTMAP4_8822B | `cfg_sounding_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:296` | BF/MU |
| `0x0691` | REG_NAN_RX_TSF_FILTER_8822B | `rtl8822b_rx_tsf_addr_filter_config` | `hal/rtl8822b/rtl8822b_mac.c:134` | khác |
| `0x0693` | REG_WMMPS_UAPSD_TID_8822B | `rtw_hal_update_uapsd_tid` | `hal/hal_com.c:14683` | khác |
| `0x06A1` | — | `_config_beamformee_mu`, `_config_beamformer_mu`, `efuse_PreUpdateAction` | `core/efuse/rtw_efuse.c:978` | BF/MU |
| `0x06A3` | — | `efuse_PreUpdateAction` | `core/efuse/rtw_efuse.c:980` | khác |
| `0x06A5` | — | `efuse_PreUpdateAction` | `core/efuse/rtw_efuse.c:982` | khác |
| `0x06B4` | REG_STATE_MON_8822B | `_iqk_bb_reset_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:281` | IQK (driver) |
| `0x06C0` | REG_BT_COEX_TABLE_8822B | `ex_hal8822b_wifi_only_hw_config` | `hal/btc/halbtc8822bwifionly.c:81` | wifi-only/BTC |
| `0x06C4` | — | `ex_hal8822b_wifi_only_hw_config` | `hal/btc/halbtc8822bwifionly.c:82` | wifi-only/BTC |
| `0x06DC` | REG_BBPSF_CTRL_8822B | `cfg_csi_rate_88xx`, `cfg_sounding_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:282` | BF/MU |
| `0x06DF` | — | `rtl8822b_phy_bf_init` | `hal/rtl8822b/rtl8822b_phy.c:2149` | BF/MU |
| `0x06E4` | REG_ASSOCIATED_BFMER0_INFO_8822B | `_reset_beamformer_su`, `mu_bfer_entry_del_88xx`, `mu_bfer_entry_init_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:512` | BF/MU |
| `0x06E8` | — | `_reset_beamformer_su`, `mu_bfer_entry_del_88xx`, `mu_bfer_entry_init_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:513` | BF/MU |
| `0x06EA` | — | `mu_bfer_entry_init_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:514` | BF/MU |
| `0x06EC` | REG_ASSOCIATED_BFMER1_INFO_8822B | `_reset_beamformer_su` | `hal/rtl8822b/rtl8822b_phy.c:1956` | BF/MU |
| `0x06F0` | — | `_reset_beamformer_su` | `hal/rtl8822b/rtl8822b_phy.c:1957` | BF/MU |
| `0x06F4` | REG_TX_CSI_RPT_PARAM_BW20_8822B | `_reset_beamformer_su`, `mu_bfer_entry_init_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:515` | BF/MU |
| `0x06F6` | — | `_reset_beamformer_su` | `hal/rtl8822b/rtl8822b_phy.c:1958` | BF/MU |
| `0x0704` | — | `get_mac_addr_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1212` | khác |
| `0x0714` | REG_ASSOCIATED_BFMEE_SEL_8822B | `_config_beamformee_su`, `_reset_beamformee_su` | `hal/rtl8822b/rtl8822b_phy.c:1777` | BF/MU |
| `0x0718` | REG_SND_PTCL_CTRL_8822B | `_config_beamformee_mu`, `_config_beamformer_su`, `cfg_sounding_88xx` … | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:960` | BF/MU |
| `0x0719` | — | `_config_beamformee_mu`, `cfg_sounding_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:289` | BF/MU |
| `0x071A` | — | `cfg_sounding_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:290` | BF/MU |
| `0x071B` | — | `_config_beamformer_su`, `cfg_sounding_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:294` | BF/MU |
| `0x07C0` | REG_IQ_DUMP_8822B | `phydm_la_adc_smp_start`, `phydm_la_get_tx_pkt_buf`, `phydm_la_set_mac_trigger_time` | `hal/phydm/phydm_adc_sampling.c:1165` | LA/ADC |
| `0x07CC` | REG_WMAC_FTM_CTL_8822B | `phydm_la_set_mac_trigger_time` | `hal/phydm/phydm_adc_sampling.c:1164` | LA/ADC |
| `0x07D4` | — | `cfg_drv_info_8822b`, `init_wmac_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:135` | khác |
| `0x07D8` | — | `init_wmac_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:962` | khác |
| `0x1070` | REG_SYS_CFG5_8822B | `rtl8822b_gethwreg` | `hal/rtl8822b/rtl8822b_ops.c:2813` | khác |
| `0x1100` | REG_CR_EXT_8822B | `cfg_net_type_88xx`, `rtw_halmac_get_network_type` | `hal/hal_halmac.c:1989` | khác |
| `0x1118` | REG_RXFF_PTR_V1_8822B | `check_rx_count` | `hal/rtl8822b/rtl8822b_ops.c:948` | khác |
| `0x11F4` | REG_MSG3_8822B | `rtw_halmac_txfifo_wait_empty` | `hal/hal_halmac.c:3822` | khác |
| `0x11F8` | REG_MSG4_8822B | `rtw_halmac_txfifo_wait_empty` | `hal/hal_halmac.c:3822` | khác |
| `0x1448` | REG_FAST_EDCA_VOVI_SETTING_8822B | `init_protocol_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:815` | khác |
| `0x144A` | — | `init_protocol_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:816` | khác |
| `0x144C` | REG_FAST_EDCA_BEBK_SETTING_8822B | `init_protocol_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:817` | khác |
| `0x144E` | — | `init_protocol_cfg_8822b` | `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:818` | khác |
| `0x14C0` | REG_MU_TX_CTL_8822B | `_config_beamformee_mu`, `_reset_beamformee_mu`, `_sounding_config_mu` … | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:168` | BF/MU |
| `0x14C1` | — | `cfg_mu_bfee_88xx`, `cfg_mu_bfer_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:172` | BF/MU |
| `0x14C4` | REG_MU_STA_GID_VLD_8822B | `_config_beamformee_mu`, `_sounding_config_mu`, `_sounding_reset_all` … | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:175` | BF/MU |
| `0x14C8` | REG_MU_STA_USER_POS_INFO_8822B | `_config_beamformee_mu`, `cfg_mu_bfee_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:176` | BF/MU |
| `0x14CC` | R_0x14cc | `_config_beamformee_mu`, `cfg_mu_bfee_88xx` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:177` | BF/MU |
| `0x1624` | — | `get_mac_addr_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1216` | khác |
| `0x1630` | REG_MACID3_8822B | `get_mac_addr_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1219` | khác |
| `0x1634` | — | `get_mac_addr_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1220` | khác |
| `0x1640` | REG_MACID4_8822B | `get_mac_addr_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1223` | khác |
| `0x1644` | — | `get_mac_addr_88xx` | `hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1224` | khác |
| `0x167C` | REG_WMAC_MU_BF_OPTION_8822B | `rtl8822b_phy_bf_init` | `hal/rtl8822b/rtl8822b_phy.c:2135` | BF/MU |
| `0x1680` | REG_WMAC_MU_BF_CTL_8822B | `mu_bfer_entry_del_88xx`, `mu_bfer_entry_init_88xx`, `rtl8822b_phy_bf_init` | `hal/halmac/halmac_88xx/halmac_mimo_88xx.c:517` | BF/MU |
| `0x1684` | REG_WMAC_ASSOCIATED_MU_BFMEE2_8822B | `_config_beamformee_mu`, `_reset_beamformee_mu`, `rtl8822b_phy_bf_sounding_status` | `hal/rtl8822b/rtl8822b_phy.c:1893` | BF/MU |
| `0x1686` | REG_WMAC_ASSOCIATED_MU_BFMEE3_8822B | `_config_beamformee_mu`, `_reset_beamformee_mu` | `hal/rtl8822b/rtl8822b_phy.c:1893` | BF/MU |
| `0x1688` | REG_WMAC_ASSOCIATED_MU_BFMEE4_8822B | `_config_beamformee_mu`, `_reset_beamformee_mu`, `rtl8822b_phy_bf_sounding_status` | `hal/rtl8822b/rtl8822b_phy.c:1893` | BF/MU |
| `0x168A` | REG_WMAC_ASSOCIATED_MU_BFMEE5_8822B | `_config_beamformee_mu`, `_reset_beamformee_mu` | `hal/rtl8822b/rtl8822b_phy.c:1893` | BF/MU |
| `0x168C` | REG_WMAC_ASSOCIATED_MU_BFMEE6_8822B | `_config_beamformee_mu`, `_reset_beamformee_mu`, `rtl8822b_phy_bf_sounding_status` | `hal/rtl8822b/rtl8822b_phy.c:1893` | BF/MU |
| `0x168E` | REG_WMAC_ASSOCIATED_MU_BFMEE7_8822B | `_config_beamformee_mu`, `_reset_beamformee_mu` | `hal/rtl8822b/rtl8822b_phy.c:1893` | BF/MU |

### BB: 101 địa chỉ live chưa có ở nhóm khác

| Địa chỉ | Tên | Hàm (≤3) | Vị trí đầu | Nhóm |
|---|---|---|---|---|
| `0x0810` | R_0x810 | `phydm_rfe_efem` | `hal/phydm/rtl8822b/phydm_hal_api8822b.c:410` | khác |
| `0x0818` | R_0x818 | `phydm_dynamic_parameters_ota`, `phydm_set_weighting_mcc` | `hal/phydm/phydm_api.c:2898` | khác |
| `0x0874` | R_0x874 | `phydm_csi_mask_enable`, `phydm_dsde_init` | `hal/phydm/phydm_api.c:814` | BF/MU |
| `0x087C` | R_0x87c | `phydm_nbi_enable`, `phydm_set_nbi_reg`, `phydm_spur_calibration_8822b` | `hal/phydm/phydm_api.c:1003` | khác |
| `0x0880` | R_0x880 | `phydm_clean_all_csi_mask`, `phydm_dsde_init` | `hal/phydm/phydm_api.c:848` | BF/MU |
| `0x0884` | R_0x884 | `phydm_clean_all_csi_mask`, `phydm_dsde_init` | `hal/phydm/phydm_api.c:849` | BF/MU |
| `0x0888` | R_0x888 | `phydm_clean_all_csi_mask`, `phydm_dsde_init` | `hal/phydm/phydm_api.c:850` | BF/MU |
| `0x088C` | R_0x88c | `phydm_clean_all_csi_mask`, `phydm_dsde_init` | `hal/phydm/phydm_api.c:851` | BF/MU |
| `0x0890` | R_0x890 | `phydm_clean_all_csi_mask`, `phydm_dsde_init` | `hal/phydm/phydm_api.c:852` | BF/MU |
| `0x0894` | R_0x894 | `phydm_clean_all_csi_mask`, `phydm_dsde_init` | `hal/phydm/phydm_api.c:853` | BF/MU |
| `0x0898` | R_0x898 | `phydm_clean_all_csi_mask`, `phydm_dsde_init` | `hal/phydm/phydm_api.c:854` | BF/MU |
| `0x089C` | R_0x89c | `phydm_clean_all_csi_mask`, `phydm_dsde_init` | `hal/phydm/phydm_api.c:855` | BF/MU |
| `0x08A0` | R_0x8a0 | `phydm_set_forgetting_factor` | `hal/phydm/phydm_adaptivity.c:373` | khác |
| `0x08A4` | R_0x8a4 | `phydm_set_edcca_threshold` | `hal/phydm/phydm_adaptivity.c:222` | khác |
| `0x08B4` | R_0x8b4 | `phydm_bb_ctrl_txagc_ofst`, `phydm_la_set_bb`, `phydm_stop_ck320` | `hal/phydm/phydm_adc_sampling.c:1067` | LA/ADC |
| `0x08DC` | R_0x8dc | `phydm_edcca_decision_opt` | `hal/phydm/phydm_adaptivity.c:384` | khác |
| `0x08F8` | R_0x8f8 | `_iqk_bb_reset_8822b`, `_iqk_tximr_selfcheck_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1368` | IQK (driver) |
| `0x090C` | R_0x90c | `_iqk_backup_mac_bb_8822b`, `_iqk_configure_macbb_8822b`, `_iqk_restore_mac_bb_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | IQK (driver) |
| `0x0910` | R_0x910 | `phydm_dynamic_spur_det_eliminate`, `phydm_psd_para_setting` | `hal/phydm/phydm_psd.c:364` | khác |
| `0x0944` | R_0x944 | `phydm_adaptivity_init` | `hal/phydm/phydm_adaptivity.c:755` | khác |
| `0x0948` | R_0x948 | `phydm_rxdfirpar_by_bw_8822b` | `hal/phydm/rtl8822b/phydm_hal_api8822b.c:880` | khác |
| `0x094C` | R_0x94c | `phydm_rxdfirpar_by_bw_8822b` | `hal/phydm/rtl8822b/phydm_hal_api8822b.c:881` | khác |
| `0x095C` | R_0x95c | `phydm_la_set_bb` | `hal/phydm/phydm_adc_sampling.c:1068` | LA/ADC |
| `0x0974` | R_0x974 | `ex_hal8822b_wifi_only_hw_config`, `phy_set_rf_path_switch_8822b`, `phydm_rfe_8822b_init` | `hal/btc/halbtc8822bwifionly.c:70` | wifi-only/BTC |
| `0x098C` | R_0x98c | `phydm_dynamic_ant_weighting_8822b`, `phydm_dynamic_parameters_ota`, `phydm_set_weighting_mcc` | `hal/phydm/phydm_api.c:2894` | khác |
| `0x09AC` | R_0x9ac | `phydm_dis_cdd` | `hal/phydm/phydm_direct_bf.c:145` | khác |
| `0x09E4` | R_0x9e4 | `phydm_idle_noise_measure_ac` | `hal/phydm/phydm_noisemonitor.c:219` | khác |
| `0x0A0A` | R_0xa0a | `phydm_write_cck_pd_type1` | `hal/phydm/phydm_cck_pd.c:75` | khác |
| `0x0A0C` | R_0xa0c | `odm_write_dig` | `hal/phydm/phydm_dig.c:632` | khác |
| `0x0A14` | R_0xa14 | `phydm_config_cck_rx_antenna_init` | `hal/phydm/phydm_api.c:260` | khác |
| `0x0A1C` | rCCK0_DSPParameter2 | `SetHwReg` | `hal/hal_com.c:14945` | khác |
| `0x0A20` | R_0xa20 | `phydm_config_cck_rx_antenna_init` | `hal/phydm/phydm_api.c:261` | khác |
| `0x0A2E` | ODM_CCK_RAKE_MAC | `SetHwReg` | `hal/hal_com.c:14942` | khác |
| `0x0A70` | R_0xa70 | `phydm_config_cck_rx_antenna_init` | `hal/phydm/phydm_api.c:258` | khác |
| `0x0A74` | R_0xa74 | `phydm_config_cck_rx_antenna_init`, `phydm_config_cck_rx_path` | `hal/phydm/phydm_api.c:259` | khác |
| `0x0A78` | R_0xa78 | `phydm_dc_cancellation` | `hal/phydm/phydm.c:3294` | khác |
| `0x0A9C` | R_0xa9c | `phydm_dc_cancellation` | `hal/phydm/phydm.c:3321` | khác |
| `0x0C10` | R_0xc10 | `phydm_dc_cancellation` | `hal/phydm/phydm.c:3350` | khác |
| `0x0C14` | R_0xc14 | `phydm_dc_cancellation` | `hal/phydm/phydm.c:3353` | khác |
| `0x0C20` | R_0xc20 | `phydm_nbi_enable`, `phydm_rxdfirpar_by_bw_8822b`, `phydm_spur_calibration_8822b` | `hal/phydm/phydm_api.c:1004` | khác |
| `0x0C58` | R_0xc58 | `_iqk_afe_setting_8822b`, `_iqk_backup_mac_bb_8822b`, `_iqk_restore_mac_bb_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | IQK (driver) |
| `0x0C5C` | R_0xc5c | `_iqk_afe_setting_8822b`, `_iqk_backup_mac_bb_8822b`, `_iqk_restore_mac_bb_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | IQK (driver) |
| `0x0C60` | — | `_iqk_afe_setting_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:293` | IQK (driver) |
| `0x0C6C` | R_0xc6c | `_iqk_afe_setting_8822b`, `_iqk_backup_mac_bb_8822b`, `_iqk_restore_mac_bb_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | IQK (driver) |
| `0x0D0C` | R_0xd0c | `phydm_get_cfo_info_ac` | `hal/phydm/phydm_cfotracking.c:86` | khác |
| `0x0D10` | R_0xd10 | `phydm_get_cfo_info_ac`, `phydm_soml_cfo_process` | `hal/phydm/phydm_cfotracking.c:87` | khác |
| `0x0D14` | R_0xd14 | `phydm_get_cfo_info_ac`, `phydm_soml_cfo_process` | `hal/phydm/phydm_cfotracking.c:88` | khác |
| `0x0D30` | R_0xd30 | `config_phydm_read_txagc_8822b` | `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1130` | khác |
| `0x0D4C` | R_0xd4c | `phydm_get_cfo_info_ac` | `hal/phydm/phydm_cfotracking.c:91` | khác |
| `0x0D50` | R_0xd50 | `phydm_get_cfo_info_ac`, `phydm_soml_cfo_process` | `hal/phydm/phydm_cfotracking.c:92` | khác |
| `0x0D54` | R_0xd54 | `phydm_get_cfo_info_ac`, `phydm_soml_cfo_process` | `hal/phydm/phydm_cfotracking.c:93` | khác |
| `0x0E10` | R_0xe10 | `phydm_dc_cancellation` | `hal/phydm/phydm.c:3368` | khác |
| `0x0E14` | R_0xe14 | `phydm_dc_cancellation` | `hal/phydm/phydm.c:3372` | khác |
| `0x0E20` | R_0xe20 | `phydm_nbi_enable`, `phydm_rxdfirpar_by_bw_8822b`, `phydm_spur_calibration_8822b` | `hal/phydm/phydm_api.c:1006` | khác |
| `0x0E58` | — | `_iqk_afe_setting_8822b`, `_iqk_backup_mac_bb_8822b`, `_iqk_restore_mac_bb_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | IQK (driver) |
| `0x0E5C` | R_0xe5c | `_iqk_afe_setting_8822b`, `_iqk_backup_mac_bb_8822b`, `_iqk_restore_mac_bb_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | IQK (driver) |
| `0x0E60` | — | `_iqk_afe_setting_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:295` | IQK (driver) |
| `0x0E6C` | R_0xe6c | `_iqk_afe_setting_8822b`, `_iqk_backup_mac_bb_8822b`, `_iqk_restore_mac_bb_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:214` | IQK (driver) |
| `0x0EB8` | R_0xeb8 | `phydm_8822b_type18_rfe`, `phydm_rfe_efem` | `hal/phydm/rtl8822b/phydm_hal_api8822b.c:115` | khác |
| `0x0F44` | R_0xf44 | `phydm_dynamic_spur_det_eliminate` | `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1473` | khác |
| `0x0F4C` | R_0xf4c | `_sounding_config_mu` | `hal/rtl8822b/rtl8822b_phy.c:1521` | BF/MU |
| `0x0F84` | R_0xf84 | `phydm_get_condi_num_8822b` | `hal/phydm/rtl8822b/phydm_hal_api8822b.c:972` | khác |
| `0x0F87` | R_0xf87 | `phydm_get_condi_num_acc_8822b` | `hal/phydm/rtl8822b/phydm_hal_api8822b.c:953` | khác |
| `0x0F94` | R_0xf94 | `phydm_dynamic_ant_weighting_8822b` | `hal/phydm/rtl8822b/phydm_rtl8822b.c:477` | khác |
| `0x0FF0` | R_0xff0 | `phydm_idle_noise_measure_ac` | `hal/phydm/phydm_noisemonitor.c:222` | khác |
| `0x1950` | R_0x1950 | `phydm_pathb_q_matrix_rotate` | `hal/phydm/phydm_direct_bf.c:233` | khác |
| `0x1954` | R_0x1954 | `phydm_pathb_q_matrix_rotate` | `hal/phydm/phydm_direct_bf.c:232` | khác |
| `0x1988` | R_0x1988 | `phydm_get_condi_num_8822b`, `phydm_get_condi_num_acc_8822b` | `hal/phydm/rtl8822b/phydm_hal_api8822b.c:952` | khác |
| `0x198C` | R_0x198c | `_iqk_bb_reset_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:261` | IQK (driver) |
| `0x1990` | R_0x1990 | `_iqk_backup_mac_bb_8822b`, `_iqk_restore_mac_bb_8822b`, `ex_hal8822b_wifi_only_hw_config` … | `hal/btc/halbtc8822bwifionly.c:72` | wifi-only/BTC |
| `0x19A8` | R_0x19a8 | `config_phydm_switch_band_8822b`, `phydm_somlrxhp_setting` | `hal/phydm/rtl8822b/phydm_hal_api8822b.c:1627` | khác |
| `0x19AC` | REG_BB_TXBF_ANT_SET_BF1_8822B | `phydm_8822btxbf_rfmode` | `hal/phydm/txbf/haltxbf8822b.c:1073` | BF/MU |
| `0x19D4` | R_0x19d4 | `phydm_dynamic_parameters_ota` | `hal/phydm/rtl8822b/phydm_rtl8822b.c:105` | khác |
| `0x19D8` | R_0x19d8 | `phydm_dynamic_parameters_ota` | `hal/phydm/rtl8822b/phydm_rtl8822b.c:104` | khác |
| `0x19F0` | R_0x19f0 | `phydm_8822b_sutxbfer_workaroud` | `hal/phydm/txbf/haltxbf8822b.c:1106` | BF/MU |
| `0x19F8` | R_0x19f8 | `phydm_8822b_sutxbfer_workaroud` | `hal/phydm/txbf/haltxbf8822b.c:1101` | BF/MU |
| `0x1B08` | R_0x1b08 | `_iqk_check_cal_8822b`, `_iqk_one_shot_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:793` | IQK (driver) |
| `0x1B0C` | R_0x1b0c | `_iqk_rximr_selfcheck_8822b`, `_iqk_tximr_selfcheck_8822b`, `phy_get_iqk_cfir_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1369` | IQK (driver) |
| `0x1B14` | R_0x1b14 | `_iqk_get_rxk1_8822b`, `_iqk_rximr_selfcheck_8822b`, `_iqk_tximr_selfcheck_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1370` | IQK (driver) |
| `0x1B15` | — | `_iqk_rximr_selfcheck_8822b`, `_iqk_tximr_selfcheck_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1377` | IQK (driver) |
| `0x1B1C` | R_0x1b1c | `_iqk_get_rxk1_8822b`, `_iqk_rximr_selfcheck_8822b`, `_iqk_rximr_test_8822b` … | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1371` | IQK (driver) |
| `0x1B20` | R_0x1b20 | `_iqk_rximr_rxk1_test_8822b`, `_iqk_rximr_selfcheck_8822b`, `_iqk_rxk1_setting_8822b` … | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1342` | IQK (driver) |
| `0x1B24` | R_0x1b24 | `_iqk_one_shot_8822b`, `_iqk_rximr_rxk1_test_8822b`, `_iqk_rximr_selfcheck_8822b` … | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1344` | IQK (driver) |
| `0x1B2B` | — | `_iqk_rxk1_setting_8822b`, `_iqk_rxk2_setting_8822b`, `_iqk_txk_setting_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:670` | IQK (driver) |
| `0x1B2C` | R_0x1b2c | `_iqk_rximr_selfcheck_8822b`, `_iqk_tximr_selfcheck_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1380` | IQK (driver) |
| `0x1B34` | R_0x1b34 | `_iqk_rximr_selfcheck_8822b`, `_iqk_tximr_selfcheck_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1392` | IQK (driver) |
| `0x1B38` | R_0x1b38 | `_iqk_get_rxk1_8822b`, `_iqk_one_shot_8822b`, `_iqk_rximr_selfcheck_8822b` … | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1018` | IQK (driver) |
| `0x1B3C` | R_0x1b3c | `_iqk_rximr_selfcheck_8822b`, `_iqk_rxk_gsearch_fail_8822b`, `_iqk_tximr_selfcheck_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1382` | IQK (driver) |
| `0x1BB8` | R_0x1bb8 | `_iqk_rf_setting_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:548` | IQK (driver) |
| `0x1BC8` | R_0x1bc8 | `_iqk_tximr_selfcheck_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1367` | IQK (driver) |
| `0x1BCC` | R_0x1bcc | `_iqk_rxk1_setting_8822b`, `_iqk_rxk2_setting_8822b`, `_iqk_rxk_gsearch_fail_8822b` … | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1387` | IQK (driver) |
| `0x1BD4` | R_0x1bd4 | `_iqk_tximr_selfcheck_8822b`, `phy_get_iqk_cfir_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1395` | IQK (driver) |
| `0x1BD6` | R_0x1bd6 | `_iqk_rximr_selfcheck_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1475` | IQK (driver) |
| `0x1BD8` | R_0x1bd8 | `phy_get_iqk_cfir_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1938` | IQK (driver) |
| `0x1BFC` | R_0x1bfc | `_iqk_rximr_selfcheck_8822b`, `_iqk_tximr_selfcheck_8822b`, `phy_get_iqk_cfir_8822b` | `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:1396` | IQK (driver) |
| `0x1C38` | R_0x1c38 | `phydm_fahm_set_th_by_igi` | `hal/phydm/phydm_ccx.c:311` | khác |
| `0x1C78` | R_0x1c78 | `phydm_fahm_set_th_by_igi` | `hal/phydm/phydm_ccx.c:313` | khác |
| `0x1C7C` | R_0x1c7c | `phydm_fahm_set_th_by_igi` | `hal/phydm/phydm_ccx.c:315` | khác |
| `0x1C94` | R_0x1c94 | `rtl8822b_phy_bf_init` | `hal/rtl8822b/rtl8822b_phy.c:2153` | BF/MU |
| `0x1CB8` | R_0x1cb8 | `phydm_fahm_set_th_by_igi` | `hal/phydm/phydm_ccx.c:317` | khác |
| `0x1CF8` | R_0x1cf8 | `phydm_fahm_trigger` | `hal/phydm/phydm_ccx.c:162` | khác |

