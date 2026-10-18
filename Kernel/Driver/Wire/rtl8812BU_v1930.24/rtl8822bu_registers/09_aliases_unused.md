# 09 — Alias, định nghĩa không dùng, điểm chưa rõ

Sinh tự động từ `work/regs.json` + `work/catalog.json` [TOOL]; các nhận xét có nhãn được viết tay.

## 9.1 Nhiều tên cho cùng một địa chỉ (cùng không gian)

- 908 địa chỉ có ≥2 tên. Dạng phổ biến nhất: cặp `REG_X_8822B` (`halmac_reg_8822b.h`) ↔ `REG_X` (`halmac_reg2.h`) — **710 cặp, 100% cùng giá trị** [TOOL]. Đây là alias thuần (cùng thanh ghi), không đếm như 2 thanh ghi.
- Dạng thứ 2: tên theo địa chỉ `R_0xNNN` (phydm_regtable.h) trùng với tên chức năng.
- Dạng thứ 3 (cần chú ý): tên từ thế hệ chip cũ (`rFPGA0_*`, `rOFDM0_*`, `ODM_REG_*_11N`) trỏ vào địa chỉ mà ở 8822B có thể mang nghĩa khác — xem mục 9.4.

| Không gian | Địa chỉ | Các tên |
|---|---|---|
| BB | `0x0800` | R_0x800, rFPGA0_RFMOD, REG_FPGA0_RFMOD, ODM_FPGA_PHY0_PAGE8, ODM_REG_BB_CTRL_11N |
| BB | `0x0804` | R_0x804, rFPGA0_TxInfo, ODM_REG_CCK_RPT_FORMAT_11AC |
| BB | `0x0808` | R_0x808, ODM_PSDREG, ODM_PSD_SETTING, rOFDMCCKEN_Jaguar, ODM_REG_PSD_CTRL_11N, ODM_REG_BB_RX_PATH_11AC |
| BB | `0x080C` | R_0x80c, rFPGA0_TxGainStage, ODM_REG_BB_TX_PATH_11AC, ODM_REG_TX_ANT_CTRL_11N, REG_FPGA0_TX_GAIN_STAGE |
| BB | `0x0818` | R_0x818, ODM_AFE_SETTING, ODM_REG_BB_PWR_SAV5_11N |
| BB | `0x0820` | R_0x820, rFPGA0_XA_HSSIParameter1, REG_FPGA0_XA_HSSI_PARAMETER1 |
| BB | `0x0824` | R_0x824, rFPGA0_XA_HSSIParameter2, ODM_REG_CCK_RPT_FORMAT_11N |
| BB | `0x0828` | R_0x828, rFPGA0_XB_HSSIParameter1, REG_FPGA0_XB_HSSI_PARAMETER1 |
| BB | `0x082C` | R_0x82c, rFPGA0_XB_HSSIParameter2, ODM_REG_CCK_RPT_FORMAT_11N_B |
| BB | `0x0830` | R_0x830, ODM_TXAGC_B_6_18, rTxAGC_B_Rate18_06, REG_TX_AGC_B_RATE18_06 |
| BB | `0x0834` | R_0x834, ODM_TXAGC_B_24_54, rTxAGC_B_Rate54_24, REG_TX_AGC_B_RATE54_24 |
| BB | `0x0838` | R_0x838, rCCAonSec_Jaguar, ODM_TXAGC_B_MCS32_5, rTxAGC_B_CCK1_55_Mcs32, REG_TX_AGC_B_CCK_1_55_MCS32 |
| BB | `0x083C` | R_0x83c, rTxAGC_B_Mcs03_Mcs00, ODM_TXAGC_B_MCS0_MCS3, REG_TX_AGC_B_MCS03_MCS00 |
| BB | `0x0848` | R_0x848, rTxAGC_B_Mcs07_Mcs04, ODM_TXAGC_B_MCS4_MCS7, REG_TX_AGC_B_MCS07_MCS04 |
| BB | `0x084C` | R_0x84c, rTxAGC_B_Mcs11_Mcs08, ODM_TXAGC_B_MCS8_MCS11, REG_TX_AGC_B_MCS11_MCS08 |
| BB | `0x0858` | R_0x858, ODM_REG_RX_DEFAULT_A_11N |
| BB | `0x085C` | R_0x85c, ODM_ANALOG_REGISTER, ODM_REG_BB_PWR_SAV3_11N |
| BB | `0x0860` | R_0x860, ODM_REG_BB_ATC_11AC, ODM_REG_ANTSEL_CTRL_11N, ODM_RF_INTERFACE_OUTPUT, rFPGA0_XA_RFInterfaceOE, REG_FPGA0_XA_RF_INTERFACE_OE |
| BB | `0x0864` | R_0x864, ODM_REG_RX_ANT_CTRL_11N, rFPGA0_XB_RFInterfaceOE, REG_FPGA0_XB_RF_INTERFACE_OE |
| BB | `0x0868` | R_0x868, rTxAGC_B_Mcs15_Mcs12, ODM_TXAGC_B_MCS12_MCS15, REG_TX_AGC_B_MCS15_MCS12 |
| BB | `0x086C` | R_0x86c, ODM_TXAGC_B_11_A_2_11, rTxAGC_B_CCK11_A_CCK2_11, REG_TX_AGC_B_CCK_11_A_CCK_2_11 |
| BB | `0x0870` | R_0x870, ODM_REG_PIN_CTRL_11N, rFPGA0_XAB_RFInterfaceSW, REG_FPGA0_XAB_RF_INTERFACE_SW |
| BB | `0x0874` | R_0x874, ODM_AD_DA_LSB_MASK, ODM_REG_BB_PWR_SAV1_11N |
| BB | `0x0878` | R_0x878, rFPGA0_XAB_RFParameter, ODM_REG_ANTSEL_PATH_11N, REG_FPGA0_XAB_RF_PARAMETER |
| BB | `0x088C` | R_0x88c, ODM_ENABLE_3_WIRE, ODM_REG_BB_3WIRE_11N, rFPGA0_AnalogParameter4, REG_FPGA0_ANALOG_PARAMETER4 |
| BB | `0x0890` | R_0x890, ODM_REG_CLM_11N, ODM_REG_NHM_TH9_TH10_11N |
| BB | `0x0894` | R_0x894, ODM_REG_CCX_PERIOD_11N |
| BB | `0x0898` | R_0x898, ODM_REG_NHM_TH3_TO_TH0_11N |
| BB | `0x089C` | R_0x89c, ODM_REG_NHM_TH7_TO_TH4_11N |
| BB | `0x08A4` | R_0x8a4, rFPGA0_XB_LSSIReadBack, REG_FPGA0_XB_LSSI_READ_BACK |
| BB | `0x08B4` | R_0x8b4, ODM_PSD_REPORT, ODM_REG_PSD_DATA_11N, ODM_REG_CLM_READY_11N |
| BB | `0x08C4` | R_0x8c4, ODM_REG_SC_CNT_11N |
| BB | `0x08D0` | R_0x8d0, ODM_REG_CLM_RESULT_11N, ODM_REG_NHM_CNT9_TO_CNT8_11N |
| BB | `0x08D4` | R_0x8d4, ODM_REG_NHM_CNT10_TO_CNT11_11N |
| BB | `0x08D8` | R_0x8d8, ODM_REG_NHM_CNT_11N |
| BB | `0x08DC` | R_0x8dc, ODM_REG_EDCCA_POWER_CAL, ODM_REG_NHM_CNT7_TO_CNT4_11N |
| BB | `0x08FC` | R_0x8fc, ODM_REG_DBG_RPT_11AC |
| BB | `0x0900` | R_0x900, r_ANTSEL_SW_Jaguar, REG_ANTSEL_SW_JAGUAR, ODM_REG_EDCCA_DOWN_OPT, ODM_REG_BB_CTRL_PAGE9_11N |
| BB | `0x0908` | R_0x908, ODM_REG_DBG_RPT_11N |
| BB | `0x090C` | R_0x90c, rFPGA1_TxInfo, ODM_R_ANT_SELECT, REG_FPGA1_TX_INFO, ODM_REG_BB_TX_PATH_11N |
| BB | `0x0914` | R_0x914, ODM_REG_ANT_MAPPING1_11N, rSingleTone_ContTx_Jaguar, REG_SINGLE_TONE_CONT_TX_JAGUAR |
| BB | `0x0918` | R_0x918, ODM_REG_ANT_MAPPING2_11N |
| BB | `0x0944` | R_0x944, ODM_REG_ACBB_EDCCA_ENHANCE |
| BB | `0x0948` | R_0x948, ODM_REG_RX_DFIR_MOD_97F, ODM_REG_EDCCA_DOWN_OPT_11N |
| BB | `0x095C` | R_0x95c, odm_adc_trigger_jaguar2 |
| BB | `0x0990` | R_0x990, ODM_REG_CCX_PERIOD_11AC |
| BB | `0x0994` | R_0x994, ODM_REG_CLM_11AC, ODM_REG_NHM_TH9_TH10_11AC |
| BB | `0x0998` | R_0x998, ODM_REG_SOML_97F, ODM_REG_NHM_TH3_TO_TH0_11AC |
| BB | `0x099C` | R_0x99c, ODM_REG_NHM_TH7_TO_TH4_11AC |
| BB | `0x09A0` | R_0x9a0, ODM_REG_NHM_TH8_11AC |
| BB | `0x09A4` | R_0x9a4, ODM_REG_OFDM_FA_RST_11AC |
| BB | `0x09B4` | R_0x9b4, ODM_REG_CSI_CONTENT_VALUE |
| BB | `0x09E8` | R_0x9e8, ODM_REG_NHM_9E8_11AC |
| BB | `0x0A00` | R_0xa00, rCCK0_System, ODM_REG_CCK_ANTDIV_PARA1_11N |
| BB | `0x0A04` | R_0xa04, rCCK0_AFESetting, REG_CCK_0_AFE_SETTING, ODM_REG_CCK_ANT_SEL_11N |
| BB | `0x0A0A` | R_0xa0a, ODM_CCK_PD_THRESH, ODM_REG_CCK_CCA_11N, ODM_REG_CCK_CCA_11AC |
| BB | `0x0A0C` | R_0xa0c, ODM_REG_CCK_ANTDIV_PARA2_11N |
| BB | `0x0A10` | R_0xa10, ODM_REG_CCK_ANTDIV_PARA3_11N |
| BB | `0x0A14` | R_0xa14, ODM_REG_CCK_ANTDIV_PARA4_11N |
| BB | `0x0A20` | R_0xa20, rCCK0_TxFilter1, ODM_CCK_MATCH_FILTER |
| BB | `0x0A24` | R_0xa24, rCCK0_TxFilter2, ODM_REG_CCK_FILTER_PARA3_11N |
| BB | `0x0A28` | R_0xa28, rCCK0_DebugPort, ODM_REG_CCK_FILTER_PARA7_11N |
| BB | `0x0A2C` | R_0xa2c, ODM_REG_CCK_FA_RST_11N, rCCK0_FalseAlarmReport, ODM_REG_CCK_FA_RST_11AC |
| BB | `0x0A58` | R_0xa58, ODM_REG_CCK_FA_MSB_11N |
| BB | `0x0A5C` | ODM_CCK_FA_CNT_LSB, ODM_REG_CCK_FA_11AC, ODM_REG_CCK_FA_LSB_11N |
| BB | `0x0A74` | R_0xa74, ODM_REG_BB_PWR_SAV4_11N |
| BB | `0x0B2C` | R_0xb2c, ODM_REG_LNA_SWITCH_11N |
| BB | `0x0B30` | R_0xb30, PATHDIV_REG, ODM_REG_PATH_SWITCH_11N |
| BB | `0x0B38` | R_0xb38, ODM_REG_RSSI_CTRL_11N |
| BB | `0x0B58` | R_0xb58, ODM_REG_RST_RPT_11AC |
| BB | `0x0B68` | R_0xb68, ODM_REG_CONFIG_ANTA_11N |
| BB | `0x0B9C` | R_0xb9c, ODM_REG_RSSI_BT_11N |
| BB | `0x0BA0` | R_0xba0, PATHDIV_TRI |
| BB | `0x0BC0` | R_0xbc0, ODM_REG_EDCCA_DCNF_97F |
| BB | `0x0C00` | R_0xc00, ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| BB | `0x0C04` | R_0xc04, ODM_OFDM_RX_ANT, rOFDM0_TRxPathEnable, ODM_REG_BB_RX_PATH_11N, REG_OFDM_0_TRX_PATH_ENABLE |
| BB | `0x0C08` | R_0xc08, rOFDM0_TRMuxPar, ODM_REG_TRMUX_11N, ODM_REG_TRMUX_11AC, REG_OFDM_0_TR_MUX_PAR |
| BB | `0x0C0C` | R_0xc0c, ODM_REG_OFDM_FA_RSTC_11N |
| BB | `0x0C10` | R_0xc10, ODM_REG_DOWNSAM_FACTOR_11N |
| BB | `0x0C14` | R_0xc14, ODM_R_A_RXIQI, ODM_REG_RXIQI_MATRIX_11N |
| BB | `0x0C1C` | R_0xc1c, rA_TxScale_Jaguar, REG_A_TX_SCALE_JAGUAR |
| BB | `0x0C30` | R_0xc30, rOFDM0_RxDetector1 |
| BB | `0x0C4C` | R_0xc4c, rOFDM0_ECCAThreshold, REG_OFDM_0_ECCA_THRESHOLD, ODM_REG_TXIQK_MATRIX_LSB1_11N |
| BB | `0x0C50` | R_0xc50, rA_IGI_Jaguar, ODM_REG_IGI_A_11N, ODM_R_A_AGC_CORE1, rOFDM0_XAAGCCore1, ODM_REG_IGI_A_11AC, REG_OFDM_0_XA_AGC_CORE1 |
| BB | `0x0C54` | R_0xc54, ODM_R_A_AGC_CORE2, ODM_REG_ANTDIV_PARA2_11N |
| BB | `0x0C58` | R_0xc58, ODM_REG_IGI_B_11N, ODM_R_B_AGC_CORE1, rOFDM0_XBAGCCore1, REG_OFDM_0_XB_AGC_CORE1 |
| BB | `0x0C5C` | R_0xc5c, ODM_REG_ANTDIV_PARA3_11N |
| BB | `0x0C6C` | R_0xc6c, ODM_REG_L1SBD_PD_CH_11N |
| BB | `0x0C70` | R_0xc70, ODM_R_AGC_PAR, ODM_REG_BB_PWR_SAV2_11N |
| BB | `0x0C74` | R_0xc74, ODM_REG_BB_AGC_SET_2_11N |
| BB | `0x0C7C` | R_0xc7c, ODM_REG_RX_OFF_11N, ODM_R_HTSTF_AGC_PAR |
| BB | `0x0C80` | R_0xc80, rOFDM0_XATxIQImbalance, ODM_REG_TXIQK_MATRIXA_11N, REG_OFDM_0_XA_TX_IQ_IMBALANCE |
| BB | `0x0C88` | R_0xc88, ODM_REG_TXIQK_MATRIXB_11N |
| BB | `0x0C90` | R_0xc90, rA_LSSIWrite_Jaguar, ODM_TX_PWR_TRAINING_A |
| BB | `0x0C94` | R_0xc94, ODM_REG_TXIQK_MATRIXA_LSB2_11N |
| BB | `0x0C9C` | R_0xc9c, ODM_REG_TXIQK_MATRIXB_LSB2_11N |
| BB | `0x0CA0` | R_0xca0, ODM_REG_RXIQK_MATRIX_LSB_11N |
| BB | `0x0CA4` | R_0xca4, ODM_REG_ANTDIV_PARA1_11N |
| BB | `0x0CB0` | R_0xcb0, rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| BB | `0x0CB4` | R_0xcb4, rA_RFE_Inv_Jaguar |
| BB | `0x0CB8` | R_0xcb8, rA_RFE_Jaguar |
| BB | `0x0CBC` | R_0xcbc, rA_RFE_Inverse_Jaguar |
| BB | `0x0CE4` | R_0xce4, ODM_REG_SMALL_BANDWIDTH_11N |
| BB | `0x0CF0` | ODM_OFDM_FA_CNT1, ODM_REG_OFDM_FA_TYPE1_11N |
| BB | `0x0D00` | R_0xd00, rOFDM1_LSTF, ODM_OFDM_PHY0_PAGE_D, ODM_REG_OFDM_FA_RSTD_11N |
| BB | `0x0D04` | R_0xd04, rA_PIRead_Jaguar, rOFDM1_TRxPathEnable, ODM_REG_BB_RX_ANT_11N |
| BB | `0x0D08` | R_0xd08, rA_SIRead_Jaguar |
| BB | `0x0D2C` | R_0xd2c, ODM_REG_BB_ATC_11N |
| BB | `0x0D44` | R_0xd44, rB_PIRead_Jaguar |
| BB | `0x0D48` | R_0xd48, rB_SIRead_Jaguar |
| BB | `0x0DA0` | ODM_OFDM_FA_CNT2, ODM_REG_OFDM_FA_TYPE2_11N |
| BB | `0x0DA4` | ODM_OFDM_FA_CNT3, ODM_REG_OFDM_FA_TYPE3_11N |
| BB | `0x0DA8` | ODM_OFDM_FA_CNT4, ODM_REG_OFDM_FA_TYPE4_11N |
| BB | `0x0DF4` | R_0xdf4, ODM_REG_RPT_11N |
| BB | `0x0E00` | R_0xe00, ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06, REG_TX_AGC_A_RATE18_06, ODM_REG_TXAGC_A_6_18_11N |
| BB | `0x0E04` | R_0xe04, ODM_TXAGC_A_24_54, rTxAGC_A_Rate54_24, REG_TX_AGC_A_RATE54_24, ODM_REG_TXAGC_A_24_54_11N |
| BB | `0x0E08` | R_0xe08, ODM_REG_ANT_11AC_B, ODM_TXAGC_A_1_MCS32, rTxAGC_A_CCK1_Mcs32, REG_TX_AGC_A_CCK_1_MCS32, ODM_REG_TXAGC_A_1_MCS32_11N |
| BB | `0x0E10` | R_0xe10, rTxAGC_A_Mcs03_Mcs00, ODM_TXAGC_A_MCS0_MCS3, REG_TX_AGC_A_MCS03_MCS00, ODM_REG_TXAGC_A_MCS0_3_11N |
| BB | `0x0E14` | R_0xe14, rTxAGC_A_Mcs07_Mcs04, ODM_TXAGC_A_MCS4_MCS7, REG_TX_AGC_A_MCS07_MCS04, ODM_REG_TXAGC_A_MCS4_7_11N |
| BB | `0x0E18` | R_0xe18, rTxAGC_A_Mcs11_Mcs08, ODM_TXAGC_A_MCS8_MCS11, REG_TX_AGC_A_MCS11_MCS08, ODM_REG_TXAGC_A_MCS8_11_11N |
| BB | `0x0E1C` | R_0xe1c, rB_TxScale_Jaguar, rTxAGC_A_Mcs15_Mcs12, REG_B_TX_SCALE_JAGUAR, ODM_TXAGC_A_MCS12_MCS15, REG_TX_AGC_A_MCS15_MCS12, ODM_REG_TXAGC_A_MCS12_15_11N |
| BB | `0x0E24` | R_0xe24, ODM_REG_TAP_UPD_97F, ODM_REG_EDCCA_DCNF_11N |
| BB | `0x0E28` | R_0xe28, ODM_REG_NHM_TH8_11N, ODM_REG_PAGE_B1_97F, ODM_REG_FPGA0_IQK_11N |
| BB | `0x0E30` | R_0xe30, ODM_REG_TXIQK_TONE_A_11N |
| BB | `0x0E34` | R_0xe34, ODM_REG_RXIQK_TONE_A_11N |
| BB | `0x0E38` | R_0xe38, ODM_REG_TXIQK_PI_A_11N |
| BB | `0x0E3C` | R_0xe3c, ODM_REG_RXIQK_PI_A_11N |
| BB | `0x0E40` | R_0xe40, ODM_REG_TXIQK_11N |
| BB | `0x0E44` | R_0xe44, ODM_REG_RXIQK_11N |
| BB | `0x0E48` | R_0xe48, ODM_REG_IQK_AGC_PTS_11N |
| BB | `0x0E4C` | R_0xe4c, ODM_REG_IQK_AGC_RSP_11N |
| BB | `0x0E50` | R_0xe50, rB_IGI_Jaguar, ODM_REG_IGI_B_11AC |
| BB | `0x0E6C` | R_0xe6c, ODM_REG_BLUETOOTH_11N |
| BB | `0x0E70` | R_0xe70, ODM_REG_RX_WAIT_CCA_11N |
| BB | `0x0E74` | R_0xe74, ODM_REG_TX_CCK_RFON_11N |
| BB | `0x0E78` | R_0xe78, ODM_REG_TX_CCK_BBON_11N |
| BB | `0x0E7C` | R_0xe7c, ODM_REG_OFDM_RFON_11N |
| BB | `0x0E80` | R_0xe80, ODM_REG_OFDM_BBON_11N |
| BB | `0x0E84` | R_0xe84, ODM_REG_TX2RX_11N |
| BB | `0x0E88` | R_0xe88, ODM_REG_TX2TX_11N |
| BB | `0x0E8C` | R_0xe8c, ODM_REG_RX_CCK_11N |
| BB | `0x0E90` | R_0xe90, rB_LSSIWrite_Jaguar |
| BB | `0x0EB0` | R_0xeb0, rB_RFE_Pinmux_Jaguar |
| BB | `0x0EB4` | R_0xeb4, rB_RFE_Inv_Jaguar |
| BB | `0x0EB8` | R_0xeb8, rB_RFE_Jaguar |
| BB | `0x0EBC` | R_0xebc, rB_RFE_Inverse_Jaguar |
| BB | `0x0ED0` | R_0xed0, ODM_REG_RX_OFDM_11N |
| BB | `0x0ED4` | R_0xed4, ODM_REG_RX_WAIT_RIFS_11N |
| BB | `0x0ED8` | R_0xed8, ODM_REG_RX2RX_11N |
| BB | `0x0EDC` | R_0xedc, ODM_REG_STANDBY_11N |
| BB | `0x0EE0` | R_0xee0, ODM_REG_SLEEP_11N |
| BB | `0x0EEC` | R_0xeec, ODM_REG_PMPD_ANAEN_11N |
| BB | `0x0F04` | R_0xf04, ODM_REG_CCK_CRC32_CNT_11AC |
| BB | `0x0F08` | R_0xf08, ODM_REG_CCK_CCA_CNT_11AC |
| BB | `0x0F0C` | R_0xf0c, ODM_REG_VHT_CRC32_CNT_11AC |
| BB | `0x0F10` | R_0xf10, ODM_REG_HT_CRC32_CNT_11AC |
| BB | `0x0F14` | R_0xf14, ODM_REG_PAGE_F_RST_11N, ODM_REG_OFDM_CRC32_CNT_11AC |
| BB | `0x0F48` | R_0xf48, ODM_REG_OFDM_FA_11AC |
| BB | `0x0F84` | R_0xf84, ODM_REG_IGI_C_11N, ODM_REG_CCK_CRC32_ERROR_CNT_11N |
| BB | `0x0F88` | R_0xf88, ODM_REG_IGI_D_11N, ODM_REG_CCK_CRC32_OK_CNT_11N |
| BB | `0x0F90` | R_0xf90, ODM_REG_HT_CRC32_CNT_11N |
| BB | `0x0F94` | R_0xf94, ODM_REG_OFDM_CRC32_CNT_11N |
| BB | `0x0FA0` | R_0xfa0, ODM_REG_RPT_11AC |
| BB | `0x0FA4` | R_0xfa4, ODM_REG_CLM_RESULT_11AC |
| BB | `0x0FA8` | R_0xfa8, ODM_REG_NHM_CNT_11AC |
| BB | `0x0FAC` | R_0xfac, ODM_REG_NHM_CNT7_TO_CNT4_11AC |
| BB | `0x0FB0` | R_0xfb0, ODM_REG_NHM_CNT11_TO_CNT8_11AC |
| BB | `0x0FB4` | R_0xfb4, ODM_REG_NHM_DUR_READY_11AC |
| BB | `0x0FB8` | R_0xfb8, ODM_REG_HT_CRC32_CNT_11N_AGG |
| BB | `0x0FBC` | R_0xfbc, ODM_REG_OFDM_FA_TYPE3_11AC |
| BB | `0x0FC0` | R_0xfc0, ODM_REG_OFDM_FA_TYPE4_11AC |
| BB | `0x0FC4` | R_0xfc4, ODM_REG_OFDM_FA_TYPE5_11AC |
| BB | `0x0FC8` | R_0xfc8, ODM_REG_OFDM_FA_TYPE6_11AC |
| BB | `0x0FCC` | R_0xfcc, ODM_REG_OFDM_FA_TYPE1_11AC |
| BB | `0x0FD0` | R_0xfd0, ODM_REG_OFDM_FA_TYPE2_11AC |
| BB | `0x181C` | R_0x181c, rC_TxScale_Jaguar2, REG_C_TX_SCALE_JAGUAR2 |
| BB | `0x1850` | rC_IGI_Jaguar2, ODM_REG_IGI_C_11AC |
| BB | `0x1990` | R_0x1990, rA_RFE_Sel_Jaguar2 |
| BB | `0x1A1C` | rD_TxScale_Jaguar2, REG_D_TX_SCALE_JAGUAR2 |
| BB | `0x1A50` | rD_IGI_Jaguar2, ODM_REG_IGI_D_11AC |
| BB | `0x1D70` | R_0x1d70, ODM_REG_IGI_11AC3 |
| MAC | `0x0000` | REG_SYS_ISO_CTRL_8822B, R_0x0, R_0x00, REG_SYS_ISO_CTRL |
| MAC | `0x0002` | REG_SYS_FUNC_EN_8822B, ODM_BB_RESET, REG_SYS_FUNC_EN, ODM_REG_BB_RST_11N |
| MAC | `0x0010` | REG_SYS_SWR_CTRL1_8822B, R_0x10, REG_SYS_SWR_CTRL1 |
| MAC | `0x0018` | REG_SYS_SWR_CTRL3_8822B, ODM_REG_CHNBW_11N, REG_SYS_SWR_CTRL3 |
| MAC | `0x001C` | REG_RSV_CTRL_8822B, R_0x1c, REG_RSV_CTRL |
| MAC | `0x0024` | REG_AFE_CTRL1_8822B, R_0x24, REG_AFE_CTRL1, REG_AFE_XTAL_CTRL, ODM_REG_T_METER_11N |
| MAC | `0x0028` | REG_AFE_CTRL2_8822B, R_0x28, REG_AFE_CTRL2, REG_AFE_PLL_CTRL |
| MAC | `0x002C` | REG_AFE_CTRL3_8822B, R_0x2c, REG_AFE_CTRL3, REG_MAC_PHY_CTRL |
| MAC | `0x0038` | REG_PWR_OPTION_CTRL_8822B, R_0x38, REG_PWR_OPTION_CTRL |
| MAC | `0x003C` | REG_CAL_TIMER_8822B, REG_CAL_TIMER, ODM_REG_RXRF_A3_11N |
| MAC | `0x0040` | REG_GPIO_MUXCFG_8822B, R_0x40, R_0x040, REG_GPIO_MUXCFG |
| MAC | `0x0042` | ODM_REG_T_METER_88E_11N, ODM_REG_T_METER_92D_11N |
| MAC | `0x0044` | REG_GPIO_PIN_CTRL_8822B, R_0x44, REG_GPIO_PIN_CTRL |
| MAC | `0x004C` | REG_LED_CFG_8822B, R_0x4c, REG_LEDCFG0, REG_LED_CFG, ODM_REG_ANTSEL_PIN_11N |
| MAC | `0x0060` | REG_GPIO_EXT_CTRL_8822B, R_0x60, REG_GPIO_EXT_CTRL, REG_GPIO_PIN_CTRL_2 |
| MAC | `0x0064` | REG_PAD_CTRL1_8822B, R_0x64, REG_PAD_CTRL1 |
| MAC | `0x0070` | REG_SYS_SDIO_CTRL_8822B, R_0x70, REG_SYS_SDIO_CTRL, REG_LTECOEX_PATH_CONTROL |
| MAC | `0x0074` | REG_HCI_OPT_CTRL_8822B, R_0x74, REG_HCI_OPT_CTRL |
| MAC | `0x00A0` | REG_SYSON_FSM_MON_8822B, R_0xa0, REG_SYSON_FSM_MON |
| MAC | `0x00A8` | REG_PMC_DBG_CTRL1_8822B, R_0xa8, REG_PMC_DBG_CTRL1 |
| MAC | `0x00B0` | REG_HIMR0_8822B, R_0xb0, REG_HIMR0 |
| MAC | `0x00EC` | REG_WLRF1_8822B, R_0xec, REG_WLRF1 |
| MAC | `0x00F0` | REG_SYS_CFG1_8822B, R_0xf0, REG_SYS_CFG1 |
| MAC | `0x00F4` | REG_SYS_STATUS1_8822B, R_0xf4, REG_SYS_STATUS1 |
| MAC | `0x0100` | REG_CR_8822B, REG_CR, R_0x100, rPMAC_Reset |
| MAC | `0x0140` | REG_PKTBUF_DBG_CTRL_8822B, R_0x0140, REG_PKTBUF_DBG_CTRL |
| MAC | `0x0144` | REG_PKTBUF_DBG_DATA_L_8822B, R_0x0144, REG_PKTBUF_DBG_DATA_L |
| MAC | `0x0148` | REG_PKTBUF_DBG_DATA_H_8822B, R_0x0148, REG_PKTBUF_DBG_DATA_H |
| MAC | `0x01A0` | REG_C2HEVT_8822B, REG_C2HEVT, REG_C2HEVT_MSG_NORMAL |
| MAC | `0x0300` | REG_PCIE_CTRL_8822B, R_0x300, ODM_DPDT, REG_PCIE_CTRL |
| MAC | `0x039C` | REG_BD_RWPTR_CLR_8822B, REG_TSFTIMER_HCI_8822B, REG_BD_RWPTR_CLR, REG_TSFTIMER_HCI |
| MAC | `0x0430` | REG_DARFRC_8822B, R_0x430, REG_DARFRC |
| MAC | `0x0440` | REG_RRSR_8822B, R_0x440, REG_RRSR |
| MAC | `0x0444` | REG_ARFR0_8822B, R_0x444, REG_ARFR0 |
| MAC | `0x0454` | REG_CCK_CHECK_8822B, R_0x454, REG_CCK_CHECK |
| MAC | `0x049C` | REG_ARFR4_8822B, R_0x49c, REG_ARFR4 |
| MAC | `0x04A4` | REG_ARFR5_8822B, R_0x4a4, REG_ARFR5 |
| MAC | `0x04C8` | REG_PROT_MODE_CTRL_8822B, R_0x4c8, REG_PROT_MODE_CTRL |
| MAC | `0x04CC` | REG_BAR_MODE_CTRL_8822B, R_0x4cc, REG_BAR_MODE_CTRL |
| MAC | `0x04D0` | REG_MACID_SLEEP2_8822B, REG_MACID_SLEEP2, ODM_REG_EARLY_MODE_11N |
| MAC | `0x04FE` | REG_MOREDATA_8822B, ODM_DUMMY, REG_MOREDATA, ODM_REG_RSSI_MONITOR_11N |
| MAC | `0x0500` | REG_EDCA_VO_PARAM_8822B, ODM_EDCA_VO_PARAM, REG_EDCA_VO_PARAM, ODM_REG_EDCA_VO_11N |
| MAC | `0x0504` | REG_EDCA_VI_PARAM_8822B, ODM_EDCA_VI_PARAM, REG_EDCA_VI_PARAM, ODM_REG_EDCA_VI_11N |
| MAC | `0x0508` | REG_EDCA_BE_PARAM_8822B, ODM_EDCA_BE_PARAM, REG_EDCA_BE_PARAM, ODM_REG_EDCA_BE_11N |
| MAC | `0x050C` | REG_EDCA_BK_PARAM_8822B, ODM_EDCA_BK_PARAM, REG_EDCA_BK_PARAM, ODM_REG_EDCA_BK_11N |
| MAC | `0x0520` | REG_TX_PTCL_CTRL_8822B, R_0x520, REG_TX_PTCL_CTRL |
| MAC | `0x0522` | REG_TXPAUSE_8822B, R_0x522, ODM_TXPAUSE, REG_TXPAUSE, ODM_REG_TXPAUSE_11N |
| MAC | `0x0524` | REG_RD_CTRL_8822B, R_0x524, REG_RD_CTRL |
| MAC | `0x0550` | REG_BCN_CTRL_8822B, R_0x550, REG_BCN_CTRL |
| MAC | `0x0551` | REG_BCN_CTRL_CLINT0_8822B, R_0x551, REG_BCN_CTRL_1, REG_BCN_CTRL_CLINT0 |
| MAC | `0x0568` | REG_FREERUN_CNT_8822B, R_0x568, REG_FREERUN_CNT |
| MAC | `0x0578` | REG_BCN_CTRL_CLINT1_8822B, REG_BCN_CTRL_2, REG_BCN_CTRL_CLINT1 |
| MAC | `0x0579` | REG_BCN_CTRL_CLINT2_8822B, REG_BCN_CTRL_3, REG_BCN_CTRL_CLINT2 |
| MAC | `0x057A` | REG_BCN_CTRL_CLINT3_8822B, REG_BCN_CTRL_4, REG_BCN_CTRL_CLINT3 |
| MAC | `0x0588` | REG_PS_TIMER2_8822B, R_0x588, REG_PS_TIMER2 |
| MAC | `0x0604` | REG_TCR_8822B, REG_TCR, R_0x604 |
| MAC | `0x0608` | REG_RCR_8822B, REG_RCR, R_0x608 |
| MAC | `0x060F` | REG_RX_DRVINFO_SZ_8822B, R_0x60f, REG_RX_DRVINFO_SZ |
| MAC | `0x0660` | REG_LBDLY_8822B, R_0x660, REG_LBDLY |
| MAC | `0x0668` | REG_WMAC_TRXPTCL_CTL_8822B, R_0x668, REG_WMAC_TRXPTCL_CTL |
| MAC | `0x0688` | REG_RXFLTMAP3_8822B, R_0x688, REG_RXFLTMAP3 |
| MAC | `0x06A0` | REG_RXFLTMAP0_8822B, R_0x6a0, REG_RXFLTMAP0 |
| MAC | `0x06D8` | REG_WMAC_RESP_TXINFO_8822B, R_0x6d8, ODM_REG_RESP_TX_11N, ODM_REG_RESP_TX_11AC, REG_WMAC_RESP_TXINFO |
| MAC | `0x06DC` | REG_BBPSF_CTRL_8822B, R_0x6dc, REG_BBPSF_CTRL |
| MAC | `0x0700` | REG_MACID1_8822B, R_0x700, REG_MACID1 |
| MAC | `0x071C` | REG_RX_CSI_RPT_INFO_8822B, R_0x71c, REG_RX_CSI_RPT_INFO |
| MAC | `0x072C` | REG_BEAMFORMING_INFO_NSARP_8822B, R_0x72c, REG_BEAMFORMING_INFO_NSARP |
| MAC | `0x0764` | REG_BT_COEX_8822B, R_0x764, REG_BT_COEX |
| MAC | `0x07B0` | REG_ANTENNA_TRAINING_CONTROL_REGISTER_8822B, R_0x7b0, ODM_REG_ANT_TRAIN_PARA1_11N, REG_ANTENNA_TRAINING_CONTROL_REGISTER |
| MAC | `0x07B4` | R_0x7b4, ODM_REG_ANT_TRAIN_PARA2_11N |
| MAC | `0x07C0` | REG_IQ_DUMP_8822B, R_0x7c0, REG_IQ_DUMP, REG_LTECOEX_CTRL |
| MAC | `0x07C4` | R_0x7c4, REG_LTECOEX_WRITE_DATA |
| MAC | `0x07C8` | R_0x7c8, REG_LTECOEX_READ_DATA |
| MAC | `0x07CC` | REG_WMAC_FTM_CTL_8822B, R_0x7cc, REG_WMAC_FTM_CTL |
| MAC | `0x07F0` | REG_RTS_ADDRESS_0_8822B, R_0x7f0, REG_RTS_ADDRESS_0 |
| MAC | `0x07F8` | REG_RTS_ADDRESS_1_8822B, R_0x7f8, REG_RTS_ADDRESS_1 |
| MAC | `0x1080` | REG_CPU_DMEM_CON_8822B, R_0x1080, REG_CPU_DMEM_CON |
| MAC | `0x1100` | REG_CR_EXT_8822B, MSR1, REG_CR_EXT |
| MAC | `0x14C0` | REG_MU_TX_CTL_8822B, R_0x14c0, REG_MU_TX_CTL |
| MAC | `0x14C4` | REG_MU_STA_GID_VLD_8822B, R_0x14c4, REG_MU_STA_GID_VLD |
| MAC | `0x14C8` | REG_MU_STA_USER_POS_INFO_8822B, R_0x14c8, REG_MU_STA_USER_POS_INFO |
| MAC | `0x1518` | REG_CPUMGQ_PARAMETER_8822B, R_0x1518, REG_CPUMGQ_PARAMETER |
| MAC | `0x1684` | REG_WMAC_ASSOCIATED_MU_BFMEE2_8822B, R_0x1684, REG_WMAC_ASSOCIATED_MU_BFMEE2 |
| MAC | `0x1688` | REG_WMAC_ASSOCIATED_MU_BFMEE4_8822B, R_0x1688, REG_WMAC_ASSOCIATED_MU_BFMEE4 |
| MAC | `0x168C` | REG_WMAC_ASSOCIATED_MU_BFMEE6_8822B, R_0x168c, REG_WMAC_ASSOCIATED_MU_BFMEE6 |
| MAC | `0x1700` | REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL_V1_8822B, R_0x1700, REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL_V1 |
| MAC | `0x1704` | REG_WL2LTECOEX_INDIRECT_ACCESS_WRITE_DATA_V1_8822B, R_0x1704, REG_WL2LTECOEX_INDIRECT_ACCESS_WRITE_DATA_V1 |
| RF | `0x0000` | RF_0x0, RF_0x00, ODM_GAIN_SETTING, ODM_REG_RF_MODE_11N |
| RF | `0x0004` | RF_0x4, ODM_REG_RF_PIN_11N |
| RF | `0x0008` | RF_0x8, RF_0x08 |
| RF | `0x000D` | RF_0xd, RF_0x0d |
| RF | `0x0018` | RF_0x18, ODM_CHANNEL |
| RF | `0x0024` | ODM_RF_T_METER, RF_T_METER_OLD |
| RF | `0x0030` | RF_0x30, rf_mode_table_addr |
| RF | `0x0031` | RF_0x31, rf_mode_table_data0 |
| RF | `0x0032` | RF_0x32, rf_mode_table_data1 |
| RF | `0x0042` | R_0x42, RF_0x42, RF_T_METER_NEW, ODM_RF_T_METER_88E, ODM_RF_T_METER_92D, ODM_RF_T_METER_92E, ODM_RF_T_METER_8812 |
| RF | `0x0055` | RF_0x55, REG_RF_TX_GAIN_OFFSET |
| RF | `0x00EF` | RF_0xef, rf_welut_jaguar |

## 9.2 Alias "treo" (trỏ tới macro không tồn tại trong build)

Macro được `#define` active nhưng giá trị là tên của một macro **không được định nghĩa** trong nhánh build → nếu có code dùng chúng sẽ lỗi biên dịch; vì build thành công nên chúng chắc chắn không được dùng trong code active. [TOOL + INFERENCE]

- `include/hal_phy_reg.h`: 144 tên, ví dụ `REG_APK`→`rAPK`, `REG_A_CFO_LONG_DUMP_JAGUAR`→`rA_CfoLongDump_Jaguar`, `REG_A_CFO_SHORT_DUMP_JAGUAR`→`rA_CfoShortDump_Jaguar`, `REG_A_RSSI_DUMP_JAGUAR`→`rA_RSSIDump_Jaguar`, `REG_A_RX_SNR_DUMP_JAGUAR`→`rA_RXsnrDump_Jaguar`, `REG_BLUE_TOOTH`→`rBlue_Tooth`

## 9.3 Định nghĩa nhưng không dùng (status D/Z) theo header

| Header | Tên định nghĩa (active) | Có nơi dùng trong code active | Chỉ trong code tắt (Z) | Không dùng (D) |
|---|---:|---:|---:|---:|
| `hal/phydm/phydm_regtable.h` | 942 | 292 | 118 | 532 |
| `hal/halmac/halmac_reg2.h` | 798 | 182 | 18 | 598 |
| `hal/halmac/halmac_reg_8822b.h` | 710 | 132 | 18 | 560 |
| `include/hal_phy_reg.h` | 188 | 2 | 2 | 184 |
| `hal/phydm/phydm_regdefine11n.h` | 160 | 1 | 7 | 152 |
| `include/rtl8822b_hal.h` | 94 | 69 | 4 | 21 |
| `hal/phydm/phydm_reg.h` | 74 | 2 | 1 | 71 |
| `hal/phydm/phydm_regdefine11ac.h` | 48 | 18 | 3 | 27 |
| `hal/rtl8822b/usb/rtl8822bu.h` | 2 | 2 | 0 | 0 |

## 9.4 Điểm bất thường / chưa rõ (ghi tay, có bằng chứng)

| # | Quan sát | Bằng chứng | Mức chắc chắn |
|---|---|---|---|
| 1 | Sau mỗi vendor request tới vùng 0x00–0xFF hoặc 0x1000–0x10FF, driver ghi thêm **1 byte vào 0x4E0** (`REG_NULL_PKT_STATUS_V1_8822B`) với nội dung buffer vừa dùng; không có comment giải thích | `os_dep/linux/usb_ops_linux.c:199-228`, `halmac_reg_8822b.h:421` | Source xác nhận hành vi; **mục đích chưa xác định** — cần tài liệu Realtek |
| 2 | KM thêm mutex toàn cục `halmac_reg_mutex` quanh mọi `_rtw_read/write*`, trong khi `usbctrl_vendorreq` đã có `usb_vendor_req_mutex` (`CONFIG_USB_VENDOR_REQ_MUTEX` bật) | `core/rtw_io.c:93,117-224`; `usb_ops_linux.c:114-116`; `autoconf.h:295-297` | Source xác nhận; việc mutex thứ 2 có cần thiết hay không: [INFERENCE] chỉ khác biệt nếu có đường I/O không qua `usbctrl_vendorreq` hoặc cần bao trùm nhiều truy cập — nhưng read-modify-write (2 lời gọi) **vẫn không nguyên tử** |
| 3 | `rtl8822bu_set_intf_ops()` tồn tại và giống hệt `usb_set_intf_ops()` nhưng **không được gọi**; probe dùng `usb_set_intf_ops` | `hal/rtl8822b/usb/rtl8822bu_io.c:61`, `hal/hal_hci/hal_usb.c:576`, `os_dep/linux/usb_intf.c:1412` | Source xác nhận |
| 4 | `RF_T_METER_OLD/NEW` (địa chỉ RF 0x24/0x42) được đặt trong section `/* @MAC REG */` của `phydm_reg.h` | `hal/phydm/phydm_reg.h:74-78`; dùng với `odm_get_rf_reg` | Source xác nhận — dễ gây nhầm MAC 0x24 (`REG_AFE_CTRL1`) với RF 0x24 |
| 5 | `ODM_CHANNEL` = 0x18 là **RF** 0x18 (channel/BW), trùng số với MAC `REG_SYS_SWR_CTRL3` 0x18 | `phydm_reg.h:143-152` (section RF REG) | Source xác nhận |
| 6 | Vùng `0x2800–0x2FFF` của bus là cửa sổ đọc RF trực tiếp (không phải thanh ghi BB độc lập): `0x2800 + addr*4` (path A), `0x2C00 + addr*4` (path B) | `phydm_hal_api8822b.c:1000-1016` | Source xác nhận |
| 7 | `odm_set_mac_reg()` thực chất gọi `phy_set_bb_reg()` (cùng hàm RMW 32-bit với BB) | `phydm_interface.c:234-252` | Source xác nhận — "MAC" và "BB" chỉ khác dải địa chỉ, không khác cơ chế |
| 8 | `rtl8822b_write_bb_reg` với mask ≠ 0xFFFFFFFF luôn **đọc 32-bit rồi ghi 32-bit** → ghi một field 8-bit của thanh ghi MAC qua `phy_set_mac_reg` sẽ ghi lại cả 3 byte còn lại (giá trị vừa đọc). Nếu byte khác là bit W1C/tự xoá, có thể gây tác dụng phụ | `rtl8822b_phy.c:668-685` | Hành vi source xác nhận; tác dụng phụ là [INFERENCE] — phụ thuộc ngữ nghĩa phần cứng chưa có tài liệu |
| 9 | IQK mặc định do firmware làm (`rtw_iqk_fw_offload=1`), nên ~48 truy cập trong luồng IQK của driver chỉ chạy khi đổi module param | `hal_ic_cfg.h:296-298`, `os_intfs.c:940-945`, `halrf_iqk_8822b.c:1856-1865` | Source xác nhận |
| 10 | `MAC_REG.txt`, `PHY_REG_PG.txt`, `TXPWR_LMT.txt` trong `/lib/firmware/` (nếu có) **ghi đè** bảng trong source | `os_intfs.c:837`, `rtl8822b_phy.c:118-129` | [KIỂM TRA TRÊN BOARD] — có file hay không |
| 11 | `phydm_check_bit_mask()` (RMW cho RF) trả `(orig & ~mask) \| (data << shift)` **không `& mask`** — khác `rtl8822b_write_bb_reg` có `& mask`; giá trị rộng hơn field tràn sang bit kề | `hal/phydm/rtl8822b/phydm_hal_api8822b.c:243-256` vs `hal/rtl8822b/rtl8822b_phy.c:681` | Source xác nhận; có gây lỗi thực tế hay không phụ thuộc caller |
| 12 | Vendor request chỉ retry khi `value < FW_START_ADDRESS (0x1000)`; mọi thanh ghi ≥0x1000 (MAC mở rộng, BB 0x1800+, cửa sổ đọc RF 0x2800+, USB-local) chỉ thử 1 lần | `os_dep/linux/usb_ops_linux.c:193-195` | Source xác nhận |
| 14 | Thanh ghi 64-bit (ví dụ `REG_DARFRC_8822B` 0x430–0x437) được mô tả bằng `BIT_SHIFT_x (N & CPU_OPT_WIDTH)` với `CPU_OPT_WIDTH = 0x1F` (`halmac_bit_8822b.h:61`): shift thực = N mod 32 **trong dword thứ N/32** — ai dùng macro `BIT_x()` trên một biến 32-bit phải tự cộng 4 vào địa chỉ | `halmac_bit_8822b.h:8444,8456,8468` | Source xác nhận quy ước; driver không dùng các field này qua macro (bảng MAC_REG ghi theo byte) |
| 13 | `usb_read8/16/32` bỏ qua mã lỗi của `usbctrl_vendorreq` và trả `data = 0` khi lỗi → caller không phân biệt được "đọc lỗi" với "giá trị 0" | `hal/hal_hci/hal_usb.c:362-451` | Source xác nhận |
| 15 | `phydm_fa_cnt_statistics_ac`: `cnt_crc8_fail_vhtb = ret_value & 0xffff0000 >> 16` — `>>` ưu tiên hơn `&` nên thành `ret_value & 0xFFFF` (= `cnt_crc8_fail_vhta`); nửa cao của BB `0xFC4` không bao giờ được đọc. Bản JGR3 (`:490`) có ngoặc đúng | `hal/phydm/phydm_dig.c:1487` vs `:490` | Source xác nhận; chỉ ảnh hưởng thống kê/debug, không vào `cnt_all` |
| 16 | 1007 entry cuối của `array_mp_8822b_phy_reg` (chỉ số 485–1491; 975 lần ghi `0x1B80`) trùng từng cặp với `array_mp_8822b_cal_init[]` vốn nằm trong `#if 0` → chương trình khởi tạo calibration được nạp qua PHY_REG; ý nghĩa cổng `0x1B80` không có tên/comment | `halhwimg8822b_bb.c` (bảng), `halrf/rtl8822b/halrf_rfk_init_8822b.c:69-1079` (`#if 0`); hàm `odm_read_and_config_mp_8822b_cal_init` (`:1081`) có thân rỗng (`#if 0` `:1083-1098`) | So sánh bằng script [TOOL]; ý nghĩa 0x1B80 chưa xác định |
| 17 | `_iqk_fail_count_8822b` gọi `odm_get_rf_reg(dm, RF_PATH_A, RF_0x1bf0, BIT(16))` với `RF_0x1bf0 = 0x1BF0`; `config_phydm_read_rf_reg_8822b` che `reg_addr &= 0xff` → thực tế đọc **RF-A 0xF0[16]** (BB 0x2BC0), không phải BB 0x1BF0 mà vòng lặp ngay sau dùng; chạy sau **mọi** IQK (kể cả FW offload) và quyết định `is_reload` | `halrf_iqk_8822b.c:164,1866`; `phydm_regtable.h:876`; `phydm_hal_api8822b.c:1009` | Source xác nhận hành vi; ý định tác giả là [INFERENCE]; chỉ ảnh hưởng thống kê IQK |
| 18 | Kiểm tra "RF is disable" (`config_phydm_read_rf_check_8822b(x) = x != 0xFFFFFFFF`) không thể đúng với path A/B: đọc RF luôn che `mask &= 0xFFFFF` → tối đa 0xFFFFF; cộng với `usb_read32` trả 0 khi lỗi (mục 13) → đọc RF hỏng thành 0 và RMW field ghi đè các bit còn lại bằng 0 | `phydm_hal_api8822b.h:80`; `phydm_hal_api8822b.c:1013,1047,1605,1884,2080` | Source + số học; tác động thực tế phụ thuộc tần suất lỗi USB |
| 19 | LCK driver (`_phy_lc_calibrate_8822b`) không chạy ở chế độ thường: power tracking loại trừ `ODM_RTL8822B` ("Use RTLCK, close power tracking driver LCK"), chỉ còn đường MP/debug; HW dùng RTK (RF 0xC4 = 0x81402 từ bảng). Danh mục vẫn xếp A cho các truy cập của hàm này vì guard nằm quanh lời gọi qua con trỏ hàm | `halrf/halphyrf_ce.c:580-593`; `halrf_8822b.c:473-530` | Source xác nhận; giới hạn của tool reachability |
| 20 | `rtl8822b_set_tx_power_index` gom 4 byte vào `hal->txagc_set_buf` (một biến chung cho mọi path) và chỉ ghi `0x1D00/0x1D80 + (rate & 0xFC)` khi `rate % 4 == 3`. Chuỗi gọi dừng giữa nhóm 4 rate → byte dư bị OR vào dword ghi kế tiếp. Đường thường an toàn (kết thúc ở 0x3F/0x0B); đường MP `mpt_SetTxPower` VHT với `rf_type` 1T dừng ở HW 0x35 → MCS8/9 không được ghi và path B nhận `0x2F2E3D3C` | `hal/rtl8822b/rtl8822b_phy.c:770-812`; `hal/hal_mp.c:549-566`; mô phỏng `tools/txagc_sim.py 5g 0 2 --order VHT1` | Source + TOOL; chỉ MP |
| 21 | `proc_set_write_reg`/`proc_set_read_reg` `copy_from_user(tmp, buffer, count)` rồi `sscanf` mà không kết thúc chuỗi ký tự NUL; input không có ký tự xuống dòng (`echo -n`) hoặc dài đúng `sizeof(tmp)` có thể đọc byte rác trên stack; proc cũng không kiểm phạm vi địa chỉ (`wValue = addr & 0xFFFF` → 0x10000 ghi vào 0x0000) | `core/rtw_debug.c` (`proc_set_write_reg`, `proc_set_read_reg`); `os_dep/linux/usb_ops_linux.c` | Source; tác động chỉ với người có quyền ghi /proc |
| 22 | MP single-tone (`mpt_SetSingleTone_8814A`, biên dịch cho 8822B và được gọi không điều kiện IC từ PMAC TX) đọc/ghi `0x181C`/`0x1A1C` (`rC/rD_TxScale_Jaguar2`, TX scale path C/D của 8814A) `[31:21] = 0` trên chip 2 path | `hal/hal_mp.c:905-1000, 2362, 2527` | Source; ý nghĩa của 0x181C/0x1A1C trên 8822B chưa xác định; chỉ MP |
| 23 | `ex_hal8822b_wifi_only_hw_config` ghi LTE-coex offset 0x38 = 0x7700 ("gnt_wl=1 , gnt_bt=0") bằng 2 lệnh `phy_set_bb_reg` thẳng vào `0x1704`/`0x1700` (`0xC00F0038`) — không chờ `LTECOEX_READY_BIT_V1` như `ltecoex_reg_write_88xx`; chạy ở mọi lần init (`CONFIG_BT_COEXIST = n`) | `hal/btc/halbtc8822bwifionly.c`; `hal/rtl8822b/rtl8822b_halinit.c:340-350`; so với `halmac_common_88xx.c:3473-3523` | Source; tác động phụ thuộc thời gian xử lý của khối LTE-coex (chưa rõ) |
| 24 | Field của `REG_MU_TX_CTL` (0x14C0) mà driver dùng (`R_MU_P1_WAIT_STATE_EN[16]`, `R_MU_RL[15:12]`, `R_MU_TAB_SEL[10:8]`, `R_EN_MU_MIMO[7]`) được định nghĩa cục bộ trong `rtl8822b_phy.c:1341-1357`, không có trong `halmac_bit_8822b.h` (header chỉ có `[6]` và `[5:0]`); `_config_beamformee_su` ghi `0x714[31:26]` với comment gốc "unknown?" | `hal/rtl8822b/rtl8822b_phy.c:1341-1357, 1780-1792` | Source |
| 25 | Header chung `halmac_reg2.h` đặt tên `REG_LTECOEX_WRITE_DATA`/`READ_DATA` cho 0x7C4/0x7C8 (bố cục của chip khác), nhưng 8822B dùng 0x1700–0x1708 cho LTE-coex và 0x7C0 là `REG_IQ_DUMP_8822B`; phydm dùng 0x7C4/0x7C8 làm thanh ghi LA mode (`reg2/reg3`) → đọc code theo tên chung dễ hiểu nhầm | `hal/halmac/halmac_reg2.h`; `phydm_adc_sampling.c:926-940`; `halmac_reg_8822b.h` | Source; xem A13 |

