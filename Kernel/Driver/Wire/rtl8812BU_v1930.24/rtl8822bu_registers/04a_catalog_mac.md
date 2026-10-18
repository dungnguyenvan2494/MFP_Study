# Danh mục thanh ghi MAC (bus 16-bit, vùng MAC)

Vùng MAC của bus thanh ghi 8822B: `0x0000–0x07FF` và `0x1000–0x17FF` (phạm vi theo `hal/halmac/halmac_reg_8822b.h`). Truy cập qua `rtw_read/write8/16/32` → USB vendor request 0x05.

Tổng: **910 địa chỉ** — A=365, T=6, M=3, G=21, X=27, R=34, Z=23, D=431

Cột: **St** = trạng thái (A/T/M/G/X/R/Z/D, xem `04_catalog_README.md`); **W** = độ rộng truy cập quan sát được trong code/bảng (không phải độ rộng phần cứng); **Truy cập** = live/MP/dbg/dead/bảng; **Bit** = số macro bit/field định nghĩa trong section của thanh ghi / số được dùng; **Nhóm** = [INFERENCE] suy từ tên hàm truy cập.

| STT | Địa chỉ | Tên (alias) | Không gian | St | Nhóm | W | Kiểu | Truy cập L/M/G/X/T | Bit d/u | Hàm (≤4) | Vị trí (≤3) |
|---:|---|---|---|:-:|---|---|---|---|---|---|---|
| 1 | `0x0000` | REG_SYS_ISO_CTRL_8822B (+3: R_0x0, R_0x00, …) | MAC | A | Power/PS | 32 | RMW-mask | 2/0/0/0/2 | 32/0 | _iqk_bb_reset_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:277, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:278 |
| 2 | `0x0002` | REG_SYS_FUNC_EN_8822B (+3: ODM_BB_RESET, REG_SYS_FUNC_EN, …) | MAC | A | Power/PS | 8 | R W | 10/0/0/4/1 | 32/0 | enable_bb_rf_88xx, rtw_hal_gate_bb | hal/hal_com.c:11407, hal/hal_com.c:11409, hal/hal_com.c:11412 … |
| 3 | `0x0003` | *(không có macro)* | MAC | A | Power/PS | 8 | R W | 9/0/0/0/1 | 0/0 | download_firmware_88xx, init_system_cfg_8822b, pwr_state_88xx, wlan_cpu_en_88xx | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:770, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:771, hal/halmac/halmac_88xx/halmac_common_88xx.c:3530 … |
| 4 | `0x0004` | REG_SYS_PW_CTRL_8822B (+1: REG_SYS_PW_CTRL) | MAC | A | USB | 8/16 | R RMW-set W | 1/0/0/3/0 | 52/1 | set_usb_mode_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:506 |
| 5 | `0x0005` | *(không có macro)* | MAC | A | Power/PS |  |  | 0/0/0/0/11 | 0/0 |  | bảng: TRANS_ACT_TO_CARDEMU_8822B, TRANS_CARDDIS_TO_CARDEMU_8822B |
| 6 | `0x0006` | *(không có macro)* | MAC | A | Power/PS |  |  | 0/0/0/0/3 | 0/0 |  | bảng: TRANS_ACT_TO_CARDEMU_8822B, TRANS_CARDEMU_TO_ACT_8822B |
| 7 | `0x0007` | *(không có macro)* | MAC | A | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDEMU_TO_CARDDIS_8822B |
| 8 | `0x0008` | REG_SYS_CLK_CTRL_8822B (+1: REG_SYS_CLK_CTRL) | MAC | A |  | 8 | R | 1/0/0/0/0 | 32/0 | rtl8822b_gethwreg | hal/rtl8822b/rtl8822b_ops.c:2774 |
| 9 | `0x0009` | *(không có macro)* | MAC | A | System/Clock/Reset | 8 | R W | 4/0/0/0/0 | 0/0 | pltfm_reset_88xx | hal/halmac/halmac_88xx/halmac_fw_88xx.c:438, hal/halmac/halmac_88xx/halmac_fw_88xx.c:439, hal/halmac/halmac_88xx/halmac_fw_88xx.c:447 … |
| 10 | `0x000A` | REG_SYS_EEPROM_CTRL_8822B (+1: REG_SYS_EEPROM_CTRL) | MAC | A | eFuse | 8 | R | 1/0/0/0/0 | 40/2 | rtl8822b_read_efuse | hal/rtl8822b/rtl8822b_ops.c:673 |
| 11 | `0x000C` | REG_EE_VPD_8822B (+1: REG_EE_VPD) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 12 | `0x0010` | REG_SYS_SWR_CTRL1_8822B (+2: R_0x10, REG_SYS_SWR_CTRL1) | MAC | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 1/0/0/0/0 | 92/0 | phydm_cfo_tracking_init | hal/phydm/phydm_cfotracking.c:407 |
| 13 | `0x0012` | *(không có macro)* | MAC | A | Power/PS |  |  | 0/0/0/0/2 | 0/0 |  | bảng: TRANS_CARDEMU_TO_ACT_8822B |
| 14 | `0x0014` | REG_SYS_SWR_CTRL2_8822B (+1: REG_SYS_SWR_CTRL2) | MAC | D |  |  |  | 0/0/0/0/0 | 106/0 |  |  |
| 15 | `0x0018` | REG_SYS_SWR_CTRL3_8822B (+2: ODM_REG_CHNBW_11N, REG_SYS_SWR_CTRL3) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 16 | `0x001C` | REG_RSV_CTRL_8822B (+2: R_0x1c, REG_RSV_CTRL) | MAC | A | System/Clock/Reset | 8 | W | 3/0/0/0/0 | 20/0 | pre_init_system_cfg_8822b, rtw_hal_wow_enable | hal/hal_com.c:12310, hal/hal_com.c:12313, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:996 |
| 17 | `0x001D` | *(không có macro)* | MAC | A |  | 8 | R W | 4/0/0/0/0 | 0/0 | wlan_cpu_en_88xx | hal/halmac/halmac_88xx/halmac_fw_88xx.c:404, hal/halmac/halmac_88xx/halmac_fw_88xx.c:406, hal/halmac/halmac_88xx/halmac_fw_88xx.c:420 … |
| 18 | `0x001F` | REG_RF_CTRL_8822B (+1: REG_RF_CTRL) | MAC | A | Power/PS | 8 | R W | 4/0/0/0/1 | 6/0 | enable_bb_rf_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:692, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:694, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:704 … |
| 19 | `0x0020` | REG_AFE_LDO_CTRL_8822B (+1: REG_AFE_LDO_CTRL) | MAC | A | Power/PS |  |  | 0/0/0/0/3 | 57/0 |  | bảng: TRANS_ACT_TO_CARDEMU_8822B, TRANS_CARDEMU_TO_ACT_8822B |
| 20 | `0x0024` | REG_AFE_CTRL1_8822B (+4: R_0x24, REG_AFE_CTRL1, …) | MAC | A | MAC config/WMAC | 32 | R RMW-mask W | 3/0/0/1/1 | 137/3 | cfg_mac_clk_88xx, phydm_set_crystal_cap_reg | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:759, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:761, hal/phydm/phydm_cfotracking.c:289 |
| 21 | `0x0025` | *(không có macro)* | MAC | A |  | 8 | RMW-set | 1/0/0/0/0 | 0/0 | board_rf_fine_tune_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:745 |
| 22 | `0x0028` | REG_AFE_CTRL2_8822B (+3: R_0x28, REG_AFE_CTRL2, …) | MAC | A |  | 32 | RMW-mask | 1/0/0/0/0 | 138/0 | phydm_set_crystal_cap_reg | hal/phydm/phydm_cfotracking.c:290 |
| 23 | `0x0029` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/2 | 0/0 |  | bảng: TRANS_CARDEMU_TO_ACT_8822B, array_mp_8822b_mac_reg |
| 24 | `0x002C` | REG_AFE_CTRL3_8822B (+3: R_0x2c, REG_AFE_CTRL3, …) | MAC | Z |  |  |  | 0/0/0/0/0 | 36/0 |  |  |
| 25 | `0x0030` | REG_EFUSE_CTRL_8822B (+1: REG_EFUSE_CTRL) | MAC | A | eFuse | 32 | R W | 6/0/0/0/0 | 76/5 | read_hw_efuse_88xx, write_hw_efuse_88xx | hal/halmac/halmac_88xx/halmac_efuse_88xx.c:1022, hal/halmac/halmac_88xx/halmac_efuse_88xx.c:1027, hal/halmac/halmac_88xx/halmac_efuse_88xx.c:1032 … |
| 26 | `0x0034` | REG_LDO_EFUSE_CTRL_8822B (+1: REG_LDO_EFUSE_CTRL) | MAC | A | eFuse | 32 | R W | 2/0/0/0/0 | 78/0 | hal_EfuseSwitchToBank | core/efuse/rtw_efuse.c:707, core/efuse/rtw_efuse.c:727 |
| 27 | `0x0035` | *(không có macro)* | MAC | A | eFuse | 8 | R W | 3/0/0/0/0 | 0/0 | switch_efuse_bank_88xx | hal/halmac/halmac_88xx/halmac_efuse_88xx.c:922, hal/halmac/halmac_88xx/halmac_efuse_88xx.c:929, hal/halmac/halmac_88xx/halmac_efuse_88xx.c:931 |
| 28 | `0x0037` | *(không có macro)* | MAC | A | System/Clock/Reset | 8 | R W | 3/0/0/0/0 | 0/0 | cfg_ldo25_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_common_8822b.c:206, hal/halmac/halmac_88xx/halmac_8822b/halmac_common_8822b.c:209, hal/halmac/halmac_88xx/halmac_8822b/halmac_common_8822b.c:211 |
| 29 | `0x0038` | REG_PWR_OPTION_CTRL_8822B (+2: R_0x38, REG_PWR_OPTION_CTRL) | MAC | G |  | 32 | RMW-mask | 0/0/1/1/0 | 101/0 | phydm_la_set_mac_iq_dump | hal/phydm/phydm_adc_sampling.c:954 |
| 30 | `0x003C` | REG_CAL_TIMER_8822B (+2: REG_CAL_TIMER, ODM_REG_RXRF_A3_11N) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 31 | `0x003E` | REG_ACLK_MON_8822B (+1: REG_ACLK_MON) | MAC | D |  |  |  | 0/0/0/0/0 | 32/0 |  |  |
| 32 | `0x0040` | REG_GPIO_MUXCFG_8822B (+3: R_0x40, R_0x040, …) | MAC | A | GPIO/LED | 32 | R RMW-mask W | 5/0/0/4/44 | 57/1 | init_system_cfg_8822b, phydm_rfe_8822b_init, pre_init_system_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:1026, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:1029, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:777 … |
| 33 | `0x0041` | *(không có macro)* | MAC | T | GPIO/LED |  |  | 0/0/0/0/10 | 0/0 |  | bảng: GPIO0_BT_ACT_8822B, GPIO0_WL_ACT_8822B |
| 34 | `0x0042` | ODM_REG_T_METER_88E_11N (+1: ODM_REG_T_METER_92D_11N) | MAC | T | GPIO/LED |  |  | 0/0/0/0/4 | 0/0 |  | bảng: GPIO4_WL_SPI_D0_8822B, GPIO5_WL_SPI_D1_8822B |
| 35 | `0x0044` | REG_GPIO_PIN_CTRL_8822B (+2: R_0x44, REG_GPIO_PIN_CTRL) | MAC | R |  |  |  | 0/0/0/0/0 | 56/0 |  |  |
| 36 | `0x0045` | *(không có macro)* | MAC | A | GPIO/LED | 8 | R W | 4/0/0/0/0 | 0/0 | rtw_hal_set_output_gpio | hal/hal_com.c:4697, hal/hal_com.c:4698, hal/hal_com.c:4700 … |
| 37 | `0x0046` | *(không có macro)* | MAC | A | Power/PS | 8 | R W | 2/0/0/2/2 | 0/0 | rtw_hal_set_output_gpio | hal/hal_com.c:4692, hal/hal_com.c:4693 |
| 38 | `0x0047` | *(không có macro)* | MAC | A | GPIO/LED | 8 | R W | 2/0/0/2/0 | 0/0 | rtw_hal_set_output_gpio | hal/hal_com.c:4686, hal/hal_com.c:4687 |
| 39 | `0x0048` | REG_GPIO_INTM_8822B (+1: REG_GPIO_INTM) | MAC | Z |  |  |  | 0/0/0/0/0 | 50/0 |  |  |
| 40 | `0x0049` | *(không có macro)* | MAC | A | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_ACT_TO_CARDEMU_8822B |
| 41 | `0x004A` | *(không có macro)* | MAC | A | Power/PS |  |  | 0/0/0/0/4 | 0/0 |  | bảng: GPIO8_WL_EXT_WOL_8822B, GPIO9_WL_EXT_WOL_8822B |
| 42 | `0x004C` | REG_LED_CFG_8822B (+4: R_0x4c, REG_LEDCFG0, …) | MAC | A | Power/PS | 32 | R RMW-mask W | 6/1/0/5/0 | 86/0 | _ips_leave, ex_hal8822b_wifi_only_hw_config, phy_set_rf_path_switch_8822b, phydm_rfe_8822b_init … | core/rtw_pwrctrl.c:185, hal/btc/halbtc8822bwifionly.c:66, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:1022 … |
| 43 | `0x004E` | *(không có macro)* | MAC | A | GPIO/LED | 8 | R W | 4/0/0/0/3 | 0/0 | pinmux_wl_led_mode_88xx, pinmux_wl_led_sw_ctrl_88xx | hal/halmac/halmac_88xx/halmac_gpio_88xx.c:101, hal/halmac/halmac_88xx/halmac_gpio_88xx.c:122, hal/halmac/halmac_88xx/halmac_gpio_88xx.c:125 … |
| 44 | `0x004F` | *(không có macro)* | MAC | X | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDEMU_TO_CARDDIS_8822B |
| 45 | `0x0050` | REG_FSIMR_8822B (+1: REG_FSIMR) | MAC | D |  |  |  | 0/0/0/0/0 | 60/0 |  |  |
| 46 | `0x0054` | REG_FSISR_8822B (+1: REG_FSISR) | MAC | D |  |  |  | 0/0/0/0/0 | 57/0 |  |  |
| 47 | `0x0058` | REG_HSIMR_8822B (+1: REG_HSIMR) | MAC | Z |  |  |  | 0/0/0/0/0 | 40/0 |  |  |
| 48 | `0x005C` | REG_HSISR_8822B (+1: REG_HSISR) | MAC | D |  |  |  | 0/0/0/0/0 | 41/0 |  |  |
| 49 | `0x0060` | REG_GPIO_EXT_CTRL_8822B (+3: R_0x60, REG_GPIO_EXT_CTRL, …) | MAC | R |  |  |  | 0/0/0/0/0 | 64/0 |  |  |
| 50 | `0x0061` | *(không có macro)* | MAC | A | GPIO/LED | 8 | R W | 4/0/0/0/0 | 0/0 | rtw_hal_set_output_gpio | hal/hal_com.c:4723, hal/hal_com.c:4724, hal/hal_com.c:4726 … |
| 51 | `0x0062` | *(không có macro)* | MAC | A | GPIO/LED | 8 | R W | 2/0/0/2/1 | 0/0 | rtw_hal_set_output_gpio | hal/hal_com.c:4718, hal/hal_com.c:4719 |
| 52 | `0x0063` | *(không có macro)* | MAC | A | GPIO/LED | 8 | R W | 2/0/0/2/0 | 0/0 | rtw_hal_set_output_gpio | hal/hal_com.c:4712, hal/hal_com.c:4713 |
| 53 | `0x0064` | REG_PAD_CTRL1_8822B (+2: R_0x64, REG_PAD_CTRL1) | MAC | A | Antenna/RFE | 32 | R RMW-mask W | 3/0/0/3/0 | 66/0 | phydm_rfe_8822b_init, pre_init_system_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:1017, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:1020, hal/phydm/rtl8822b/phydm_hal_api8822b.c:301 |
| 54 | `0x0066` | R_0x66 | MAC | T | GPIO/LED |  |  | 0/0/0/0/13 | 0/0 |  | bảng: GPIO0_BT_GPIO0_8822B, GPIO11_USOUT0_8822B |
| 55 | `0x0067` | *(không có macro)* | MAC | A | GPIO/LED |  |  | 0/0/0/0/12 | 0/0 |  | bảng: GPIO4_JTAG_TRST_8822B, GPIO5_JTAG_TDI_8822B |
| 56 | `0x0068` | REG_WL_BT_PWR_CTRL_8822B (+1: REG_WL_BT_PWR_CTRL) | MAC | A | Power/PS | 32 | R | 1/0/0/0/2 | 58/3 | read_chip_version | hal/rtl8822b/rtl8822b_ops.c:250 |
| 57 | `0x006A` | *(không có macro)* | MAC | T | GPIO/LED |  |  | 0/0/0/0/1 | 0/0 |  | bảng: GPIO11_DIS_BT_N_8822B |
| 58 | `0x006C` | REG_SDM_DEBUG_8822B (+1: REG_SDM_DEBUG) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 59 | `0x0070` | REG_SYS_SDIO_CTRL_8822B (+3: R_0x70, REG_SYS_SDIO_CTRL, …) | MAC | A |  | 32 | R-mask RMW-mask | 2/2/0/0/0 | 36/0 | ex_hal8822b_wifi_only_hw_config, phy_set_rf_path_switch_8822b | hal/btc/halbtc8822bwifionly.c:76, hal/phydm/halrf/rtl8822b/halrf_8822b.c:568 |
| 60 | `0x0072` | *(không có macro)* | MAC | T | Interrupt | 8 | R W | 0/0/0/2/1 | 0/0 | pinmux_sdio_int_polarity_88xx | hal/halmac/halmac_88xx/halmac_gpio_88xx.c:142, hal/halmac/halmac_88xx/halmac_gpio_88xx.c:145 |
| 61 | `0x0073` | *(không có macro)* | MAC | T | GPIO/LED |  |  | 0/0/0/0/2 | 0/0 |  | bảng: GPIO4_DBG_GNT_WL_8822B, GPIO5_DBG_GNT_BT_8822B |
| 62 | `0x0074` | REG_HCI_OPT_CTRL_8822B (+2: R_0x74, REG_HCI_OPT_CTRL) | MAC | X | Power/PS |  |  | 0/0/0/0/1 | 53/0 |  | bảng: TRANS_CARDEMU_TO_ACT_8822B |
| 63 | `0x0075` | *(không có macro)* | MAC | X | Power/PS |  |  | 0/0/0/0/2 | 0/0 |  | bảng: TRANS_CARDEMU_TO_ACT_8822B |
| 64 | `0x0078` | REG_AFE_CTRL4_8822B (+1: REG_AFE_CTRL4) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 65 | `0x007C` | REG_LDO_SWR_CTRL_8822B (+1: REG_LDO_SWR_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 112/0 |  |  |
| 66 | `0x0080` | REG_MCUFW_CTRL_8822B (+1: REG_MCUFW_CTRL) | MAC | A | Firmware/H2C/C2H | 8/16/32 | R W | 15/0/0/0/0 | 64/6 | check_fw_chksum_88xx, dlfw_end_flow_88xx, download_firmware_88xx, init_system_cfg_8822b … | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:774, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:776, hal/halmac/halmac_88xx/halmac_8822b/halmac_usb_8822b.c:89 … |
| 67 | `0x0081` | *(không có macro)* | MAC | A | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDEMU_TO_CARDDIS_8822B |
| 68 | `0x0084` | REG_MCU_TST_CFG_8822B (+1: REG_MCU_TST_CFG) | MAC | A | Firmware/H2C/C2H | 8 | R W | 4/0/0/0/0 | 14/0 | free_dl_fw_end_flow_88xx, get_cpu_mode_88xx | hal/halmac/halmac_88xx/halmac_fw_88xx.c:1069, hal/halmac/halmac_88xx/halmac_fw_88xx.c:1072, hal/halmac/halmac_88xx/halmac_fw_88xx.c:747 … |
| 69 | `0x0086` | *(không có macro)* | MAC | A | Power/PS | 8 | R | 1/0/0/0/0 | 0/0 | check_ips_status | hal/rtl8822b/rtl8822b_ops.c:764 |
| 70 | `0x0088` | REG_HMEBOX_E0_E1_8822B (+1: REG_HMEBOX_E0_E1) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 71 | `0x008B` | *(không có macro)* | MAC | X | MAC config/WMAC | 8 | W | 0/0/0/1/0 | 0/0 | rtw_hal_backup_rate | hal/hal_com.c:5267 |
| 72 | `0x008C` | REG_HMEBOX_E2_E3_8822B (+1: REG_HMEBOX_E2_E3) | MAC | X | MAC config/WMAC | 8 | W | 0/0/0/1/0 | 28/0 | rtw_hal_backup_rate | hal/hal_com.c:5265 |
| 73 | `0x008D` | *(không có macro)* | MAC | X | MAC config/WMAC | 8 | W | 0/0/0/1/0 | 0/0 | rtw_hal_backup_rate | hal/hal_com.c:5264 |
| 74 | `0x008F` | *(không có macro)* | MAC | X | MAC config/WMAC | 8 | W | 0/0/0/1/0 | 0/0 | rtw_hal_backup_rate | hal/hal_com.c:5266 |
| 75 | `0x0090` | REG_WLLPS_CTRL_8822B (+1: REG_WLLPS_CTRL) | MAC | A | Power/PS |  |  | 0/0/0/0/1 | 55/0 |  | bảng: TRANS_CARDEMU_TO_CARDDIS_8822B |
| 76 | `0x0093` | *(không có macro)* | MAC | A | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_ACT_TO_CARDEMU_8822B |
| 77 | `0x0094` | REG_AFE_CTRL5_8822B (+1: REG_AFE_CTRL5) | MAC | D |  |  |  | 0/0/0/0/0 | 62/0 |  |  |
| 78 | `0x0098` | REG_GPIO_DEBOUNCE_CTRL_8822B (+1: REG_GPIO_DEBOUNCE_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 32/0 |  |  |
| 79 | `0x009C` | REG_RPWM2_8822B (+1: REG_RPWM2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 80 | `0x00A0` | REG_SYSON_FSM_MON_8822B (+2: R_0xa0, REG_SYSON_FSM_MON) | MAC | M | eFuse | 8 | R | 0/3/0/0/0 | 32/0 | rtw_mp_efuse_get, rtw_mp_efuse_set | os_dep/linux/ioctl_linux.c:9033, os_dep/linux/ioctl_linux.c:9631, os_dep/linux/ioctl_linux.c:9754 |
| 81 | `0x00A3` | *(không có macro)* | MAC | M | eFuse | 8 | W | 0/3/0/0/0 | 0/0 | rtw_mp_efuse_get, rtw_mp_efuse_set | os_dep/linux/ioctl_linux.c:9032, os_dep/linux/ioctl_linux.c:9630, os_dep/linux/ioctl_linux.c:9753 |
| 82 | `0x00A4` | REG_AFE_CTRL6_8822B (+1: REG_AFE_CTRL6) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 83 | `0x00A8` | REG_PMC_DBG_CTRL1_8822B (+2: R_0xa8, REG_PMC_DBG_CTRL1) | MAC | Z |  |  |  | 0/0/0/0/0 | 32/0 |  |  |
| 84 | `0x00AC` | REG_AFE_CTRL7_8822B (+1: REG_AFE_CTRL7) | MAC | D |  |  |  | 0/0/0/0/0 | 40/0 |  |  |
| 85 | `0x00AF` | *(không có macro)* | MAC | A | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDEMU_TO_ACT_8822B |
| 86 | `0x00B0` | REG_HIMR0_8822B (+2: R_0xb0, REG_HIMR0) | MAC | X | Interrupt | 32 | W | 0/0/0/1/0 | 63/0 | rtl8822bu_update_interrupt_mask | hal/rtl8822b/usb/rtl8822bu_halinit.c:388 |
| 87 | `0x00B4` | REG_HISR0_8822B (+1: REG_HISR0) | MAC | D |  |  |  | 0/0/0/0/0 | 37/0 |  |  |
| 88 | `0x00B8` | REG_HIMR1_8822B (+1: REG_HIMR1) | MAC | X | Interrupt | 32 | W | 0/0/0/1/0 | 71/0 | rtl8822bu_update_interrupt_mask | hal/rtl8822b/usb/rtl8822bu_halinit.c:390 |
| 89 | `0x00BC` | REG_HISR1_8822B (+1: REG_HISR1) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 90 | `0x00C0` | REG_DBG_PORT_SEL_8822B (+1: REG_DBG_PORT_SEL) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 91 | `0x00C4` | REG_PAD_CTRL2_8822B (+1: REG_PAD_CTRL2) | MAC | A | USB | 32 | R RMW-set W | 4/0/0/0/0 | 41/7 | set_usb_mode_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:475, hal/halmac/halmac_88xx/halmac_usb_88xx.c:495, hal/halmac/halmac_88xx/halmac_usb_88xx.c:500 … |
| 92 | `0x00C5` | *(không có macro)* | MAC | A | USB | 8 | W | 1/0/0/0/0 | 0/0 | set_usb_mode_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:505 |
| 93 | `0x00CC` | REG_PMC_DBG_CTRL2_8822B (+1: REG_PMC_DBG_CTRL2) | MAC | R |  |  |  | 0/0/0/0/0 | 40/0 |  |  |
| 94 | `0x00CF` | *(không có macro)* | MAC | A | eFuse | 8 | W | 2/0/0/0/0 | 0/0 | write_hw_efuse_88xx | hal/halmac/halmac_88xx/halmac_efuse_88xx.c:1062, hal/halmac/halmac_88xx/halmac_efuse_88xx.c:1089 |
| 95 | `0x00D0` | REG_BIST_CTRL_8822B (+1: REG_BIST_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 32/0 |  |  |
| 96 | `0x00D4` | REG_BIST_RPT_8822B (+1: REG_BIST_RPT) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 97 | `0x00D8` | REG_MEM_CTRL_8822B (+1: REG_MEM_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 100/0 |  |  |
| 98 | `0x00DC` | REG_AFE_CTRL8_8822B (+1: REG_AFE_CTRL8) | MAC | D |  |  |  | 0/0/0/0/0 | 20/0 |  |  |
| 99 | `0x00E0` | REG_USB_SIE_INTF_8822B (+1: REG_USB_SIE_INTF) | MAC | D |  |  |  | 0/0/0/0/0 | 50/0 |  |  |
| 100 | `0x00E4` | REG_PCIE_MIO_INTF_8822B (+1: REG_PCIE_MIO_INTF) | MAC | D |  |  |  | 0/0/0/0/0 | 32/0 |  |  |
| 101 | `0x00E8` | REG_PCIE_MIO_INTD_8822B (+1: REG_PCIE_MIO_INTD) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 102 | `0x00EC` | REG_WLRF1_8822B (+2: R_0xec, REG_WLRF1) | MAC | A |  | 32 | R W | 4/0/0/0/0 | 14/0 | enable_bb_rf_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:696, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:698, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:708 … |
| 103 | `0x00EF` | *(không có macro)* | MAC | A | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_ACT_TO_CARDEMU_8822B |
| 104 | `0x00F0` | REG_SYS_CFG1_8822B (+2: R_0xf0, REG_SYS_CFG1) | MAC | A |  | 32 | R | 1/0/0/0/0 | 80/9 | read_chip_version | hal/rtl8822b/rtl8822b_ops.c:223 |
| 105 | `0x00F1` | *(không có macro)* | MAC | A |  | 8 | R | 1/0/0/0/0 | 0/0 | get_chip_info | hal/halmac/halmac_api.c:561 |
| 106 | `0x00F2` | *(không có macro)* | MAC | A | System/Clock/Reset | 8 | R | 1/0/0/0/0 | 0/0 | pre_init_system_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:1034 |
| 107 | `0x00F4` | REG_SYS_STATUS1_8822B (+2: R_0xf4, REG_SYS_STATUS1) | MAC | A |  | 32 | R RMW-mask | 1/0/1/1/0 | 82/3 | read_chip_version | hal/rtl8822b/rtl8822b_ops.c:245 |
| 108 | `0x00F5` | *(không có macro)* | MAC | A | Power/PS | 8 | R RMW-clr | 2/0/0/0/0 | 0/0 | mac_pwr_switch_usb_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_usb_8822b.c:129, hal/halmac/halmac_88xx/halmac_8822b/halmac_usb_8822b.c:99 |
| 109 | `0x00F8` | REG_SYS_STATUS2_8822B (+1: REG_SYS_STATUS2) | MAC | D |  |  |  | 0/0/0/0/0 | 36/0 |  |  |
| 110 | `0x00FC` | REG_SYS_CFG2_8822B (+1: REG_SYS_CFG2) | MAC | A |  | 8 | R | 1/0/0/0/0 | 16/0 | get_chip_info | hal/halmac/halmac_api.c:560 |
| 111 | `0x00FF` | *(không có macro)* | MAC | A | USB | 8 | R | 4/0/0/0/0 | 0/0 | cfg_usb_rx_agg_88xx, init_usb_cfg_88xx, pre_init_system_cfg_8822b, set_usb_mode_88xx | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:1005, hal/halmac/halmac_88xx/halmac_usb_88xx.c:162, hal/halmac/halmac_88xx/halmac_usb_88xx.c:471 … |
| 112 | `0x0100` | REG_CR_8822B (+3: REG_CR, R_0x100, …) | MAC | A | Power/PS | 8/16/32 | R RMW-mask RMW-set W | 12/8/0/1/0 | 68/16 | cfg_sounding_88xx, check_ips_status, download_firmware_88xx, init_trx_cfg_8822b … | hal/hal_halmac.c:1979, hal/hal_halmac.c:1984, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:484 … |
| 113 | `0x0101` | *(không có macro)* | MAC | A | Firmware/H2C/C2H | 8 | R W | 8/0/0/6/0 | 0/0 | dl_rsvd_page_88xx, hw_var_set_dl_rsvd_page | hal/halmac/halmac_88xx/halmac_common_88xx.c:384, hal/halmac/halmac_88xx/halmac_common_88xx.c:387, hal/halmac/halmac_88xx/halmac_common_88xx.c:414 … |
| 114 | `0x0102` | MSR | MAC | A |  | 8 | R W | 4/0/0/0/0 | 0/0 | cfg_net_type_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:256, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:257, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:261 … |
| 115 | `0x0103` | *(không có macro)* | MAC | A | TX/RX DMA/Queue | 8 | W | 1/0/0/0/0 | 0/0 | priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:680 |
| 116 | `0x0106` | R_0x0106 | MAC | A |  | 8 | R W | 1/0/0/2/0 | 0/0 | phydm_la_get_tx_pkt_buf | hal/phydm/phydm_adc_sampling.c:851 |
| 117 | `0x0108` | REG_TSF_CLK_STATE_8822B (+1: REG_TSF_CLK_STATE) | MAC | D |  |  |  | 0/0/0/0/0 | 10/0 |  |  |
| 118 | `0x010C` | REG_TXDMA_PQ_MAP_8822B (+1: REG_TXDMA_PQ_MAP) | MAC | A | TX/RX DMA/Queue | 8/16 | R RMW-set W | 7/0/0/0/0 | 90/19 | cfg_usb_rx_agg_88xx, init_trx_cfg_8822b, rx_shift_88xx, txdma_queue_mapping_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:512, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:558, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1068 … |
| 119 | `0x010D` | *(không có macro)* | MAC | A |  | 8 | R W | 2/0/0/0/0 | 0/0 | download_firmware_88xx | hal/halmac/halmac_88xx/halmac_fw_88xx.c:191, hal/halmac/halmac_88xx/halmac_fw_88xx.c:194 |
| 120 | `0x0114` | REG_TRXFF_BNDY_8822B (+1: REG_TRXFF_BNDY) | MAC | R |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 121 | `0x0115` | *(không có macro)* | MAC | A |  | 8 | R W | 2/0/0/0/0 | 0/0 | cfg_drv_info_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:121, hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:125 |
| 122 | `0x0118` | REG_PTA_I2C_MBOX_8822B (+1: REG_PTA_I2C_MBOX) | MAC | D |  |  |  | 0/0/0/0/0 | 7/0 |  |  |
| 123 | `0x011C` | REG_RXFF_BNDY_8822B (+1: REG_RXFF_BNDY) | MAC | A | TX/RX DMA/Queue | 32 | W | 1/0/0/0/0 | 0/0 | priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:647 |
| 124 | `0x0120` | REG_FE1IMR_8822B (+1: REG_FE1IMR) | MAC | A | Power/PS | 32 | R | 2/0/0/0/0 | 54/1 | rtw_hal_check_wow_ctrl | hal/hal_com.c:5170, hal/hal_com.c:5176 |
| 125 | `0x0124` | REG_FE1ISR_8822B (+1: REG_FE1ISR) | MAC | D |  |  |  | 0/0/0/0/0 | 54/0 |  |  |
| 126 | `0x012C` | REG_CPWM_8822B (+1: REG_CPWM) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 127 | `0x0130` | REG_FWIMR_8822B (+1: REG_FWIMR) | MAC | D |  |  |  | 0/0/0/0/0 | 62/0 |  |  |
| 128 | `0x0134` | REG_FWISR_8822B (+1: REG_FWISR) | MAC | D |  |  |  | 0/0/0/0/0 | 62/0 |  |  |
| 129 | `0x0138` | REG_FTIMR_8822B (+1: REG_FTIMR) | MAC | D |  |  |  | 0/0/0/0/0 | 40/0 |  |  |
| 130 | `0x013C` | REG_FTISR_8822B (+1: REG_FTISR) | MAC | D |  |  |  | 0/0/0/0/0 | 40/0 |  |  |
| 131 | `0x0140` | REG_PKTBUF_DBG_CTRL_8822B (+2: R_0x0140, REG_PKTBUF_DBG_CTRL) | MAC | A |  | 16/32 | R RMW-mask W | 4/0/0/3/0 | 17/0 | phydm_la_access_tx_pkt_buf, read_buf_88xx | hal/halmac/halmac_88xx/halmac_common_88xx.c:2201, hal/halmac/halmac_88xx/halmac_common_88xx.c:2204, hal/halmac/halmac_88xx/halmac_common_88xx.c:2220 … |
| 132 | `0x0144` | REG_PKTBUF_DBG_DATA_L_8822B (+2: R_0x0144, REG_PKTBUF_DBG_DATA_L) | MAC | X | TX/RX DMA/Queue | 32 | R | 0/0/0/2/0 | 14/0 | dump_TX_FIFO | hal/hal_com.c:16565, hal/hal_com.c:16567 |
| 133 | `0x0148` | REG_PKTBUF_DBG_DATA_H_8822B (+2: R_0x0148, REG_PKTBUF_DBG_DATA_H) | MAC | X | TX/RX DMA/Queue | 32 | R | 0/0/0/2/0 | 14/0 | dump_TX_FIFO | hal/hal_com.c:16565, hal/hal_com.c:16567 |
| 134 | `0x014C` | REG_CPWM2_8822B (+1: REG_CPWM2) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 135 | `0x0150` | REG_TC0_CTRL_8822B (+1: REG_TC0_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 20/0 |  |  |
| 136 | `0x0154` | REG_TC1_CTRL_8822B (+1: REG_TC1_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 20/0 |  |  |
| 137 | `0x0158` | REG_TC2_CTRL_8822B (+1: REG_TC2_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 20/0 |  |  |
| 138 | `0x015C` | REG_TC3_CTRL_8822B (+1: REG_TC3_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 20/0 |  |  |
| 139 | `0x0160` | REG_TC4_CTRL_8822B (+1: REG_TC4_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 20/0 |  |  |
| 140 | `0x0164` | REG_TCUNIT_BASE_8822B (+1: REG_TCUNIT_BASE) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 141 | `0x0168` | REG_TC5_CTRL_8822B (+1: REG_TC5_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 20/0 |  |  |
| 142 | `0x016C` | REG_TC6_CTRL_8822B (+1: REG_TC6_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 27/0 |  |  |
| 143 | `0x0170` | REG_MBIST_FAIL_8822B (+1: REG_MBIST_FAIL) | MAC | D |  |  |  | 0/0/0/0/0 | 56/0 |  |  |
| 144 | `0x0174` | REG_MBIST_START_PAUSE_8822B (+1: REG_MBIST_START_PAUSE) | MAC | D |  |  |  | 0/0/0/0/0 | 56/0 |  |  |
| 145 | `0x0178` | REG_MBIST_DONE_8822B (+1: REG_MBIST_DONE) | MAC | D |  |  |  | 0/0/0/0/0 | 56/0 |  |  |
| 146 | `0x017C` | REG_MBIST_FAIL_NRML_8822B (+1: REG_MBIST_FAIL_NRML) | MAC | D |  |  |  | 0/0/0/0/0 | 21/0 |  |  |
| 147 | `0x0180` | REG_AES_DECRPT_DATA_8822B (+1: REG_AES_DECRPT_DATA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 148 | `0x0184` | REG_AES_DECRPT_CFG_8822B (+1: REG_AES_DECRPT_CFG) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 149 | `0x0190` | REG_TMETER_8822B (+1: REG_TMETER) | MAC | D |  |  |  | 0/0/0/0/0 | 46/0 |  |  |
| 150 | `0x0194` | REG_OSC_32K_CTRL_8822B (+1: REG_OSC_32K_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 34/0 |  |  |
| 151 | `0x0198` | REG_32K_CAL_REG1_8822B (+1: REG_32K_CAL_REG1) | MAC | D |  |  |  | 0/0/0/0/0 | 32/0 |  |  |
| 152 | `0x01A0` | REG_C2HEVT_8822B (+2: REG_C2HEVT, REG_C2HEVT_MSG_NORMAL) | MAC | A |  | 8 | R W | 3/0/0/0/0 | 14/0 | hal_read_mac_hidden_rpt | hal/hal_com.c:1785, hal/hal_com.c:1798, hal/hal_com.c:1811 |
| 153 | `0x01A4` | REG_C2HEVT_1_8822B (+1: REG_C2HEVT_1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 154 | `0x01A8` | REG_C2HEVT_2_8822B (+1: REG_C2HEVT_2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 155 | `0x01AC` | REG_C2HEVT_3_8822B (+1: REG_C2HEVT_3) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 156 | `0x01AF` | REG_C2HEVT_CLEAR | MAC | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 157 | `0x01B8` | REG_SW_DEFINED_PAGE1_8822B (+1: REG_SW_DEFINED_PAGE1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 158 | `0x01C0` | REG_MCUTST_I_8822B (+1: REG_MCUTST_I) | MAC | X | Firmware/H2C/C2H | 8/32 | R RMW-clr W | 0/0/0/11/0 | 14/0 | download_flash_88xx, erase_flash_88xx, fw_fatal_status_debug_88xx, read_flash_88xx | hal/halmac/halmac_88xx/halmac_flash_88xx.c:143, hal/halmac/halmac_88xx/halmac_flash_88xx.c:145, hal/halmac/halmac_88xx/halmac_flash_88xx.c:152 … |
| 159 | `0x01C4` | REG_MCUTST_II_8822B (+1: REG_MCUTST_II) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 160 | `0x01C7` | REG_WOWLAN_WAKE_REASON | MAC | A | Power/PS | 8 | R | 5/0/0/0/0 | 0/0 | rtw_hal_check_wow_ctrl, rtw_hal_wow_disable, rtw_hal_wow_enable | hal/hal_com.c:12287, hal/hal_com.c:12416, hal/hal_com.c:12419 … |
| 161 | `0x01C8` | REG_FMETHR_8822B (+1: REG_FMETHR) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 162 | `0x01CC` | REG_HMETFR_8822B (+1: REG_HMETFR) | MAC | A | Firmware/H2C/C2H | 8 | R | 2/0/0/0/0 | 22/0 | _is_fw_read_cmd_down | hal/hal_halmac.c:4066, hal/hal_halmac.c:4074 |
| 163 | `0x01CF` | *(không có macro)* | MAC | A | Power/PS | 8 | R W | 4/0/0/0/0 | 0/0 | enter_cpu_sleep_mode_88xx, free_dl_fw_end_flow_88xx | hal/halmac/halmac_88xx/halmac_fw_88xx.c:1025, hal/halmac/halmac_88xx/halmac_fw_88xx.c:1034, hal/halmac/halmac_88xx/halmac_fw_88xx.c:735 … |
| 164 | `0x01D0` | REG_HMEBOX0_8822B (+1: REG_HMEBOX0) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 165 | `0x01D4` | REG_HMEBOX1_8822B (+1: REG_HMEBOX1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 166 | `0x01D8` | REG_HMEBOX2_8822B (+1: REG_HMEBOX2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 167 | `0x01DC` | REG_HMEBOX3_8822B (+1: REG_HMEBOX3) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 168 | `0x01E0` | REG_LLT_INIT_8822B (+1: REG_LLT_INIT) | MAC | D |  |  |  | 0/0/0/0/0 | 35/0 |  |  |
| 169 | `0x01E4` | REG_LLT_INIT_ADDR_8822B (+1: REG_LLT_INIT_ADDR) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 170 | `0x01E8` | REG_BB_ACCESS_CTRL_8822B (+1: REG_BB_ACCESS_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 44/0 |  |  |
| 171 | `0x01EC` | REG_BB_ACCESS_DATA_8822B (+1: REG_BB_ACCESS_DATA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 172 | `0x01F0` | REG_HMEBOX_E0_8822B (+1: REG_HMEBOX_E0) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 173 | `0x01F4` | REG_HMEBOX_E1_8822B (+1: REG_HMEBOX_E1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 174 | `0x01F8` | REG_HMEBOX_E2_8822B (+1: REG_HMEBOX_E2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 175 | `0x01FC` | REG_HMEBOX_E3_8822B (+1: REG_HMEBOX_E3) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 176 | `0x0200` | REG_FIFOPAGE_CTRL_1_8822B (+1: REG_FIFOPAGE_CTRL_1) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 177 | `0x0204` | REG_FIFOPAGE_CTRL_2_8822B (+1: REG_FIFOPAGE_CTRL_2) | MAC | A | Firmware/H2C/C2H | 16 | W | 3/0/0/2/0 | 32/1 | dl_rsvd_page_88xx, priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:616, hal/halmac/halmac_88xx/halmac_common_88xx.c:382, hal/halmac/halmac_88xx/halmac_common_88xx.c:412 |
| 178 | `0x0205` | *(không có macro)* | MAC | A | Beacon/TSF | 8 | R W | 4/0/0/0/0 | 0/0 | dl_rsvd_page_88xx, hw_var_get_bcn_valid, hw_var_set_bcn_valid | hal/halmac/halmac_88xx/halmac_common_88xx.c:401, hal/rtl8822b/rtl8822b_ops.c:1741, hal/rtl8822b/rtl8822b_ops.c:1743 … |
| 179 | `0x0206` | *(không có macro)* | MAC | A | TX/RX DMA/Queue | 16 | W | 1/0/0/0/0 | 0/0 | priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:632 |
| 180 | `0x0208` | REG_AUTO_LLT_V1_8822B (+1: REG_AUTO_LLT_V1) | MAC | A | TX/RX DMA/Queue | 8 | R RMW-set W | 4/0/0/0/0 | 50/3 | priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:652, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:655, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:661 … |
| 181 | `0x0209` | *(không có macro)* | MAC | X | TX/RX DMA/Queue | 8 | R | 0/0/0/1/0 | 0/0 | dump_TX_FIFO | hal/hal_com.c:16559 |
| 182 | `0x020B` | *(không có macro)* | MAC | A | TX/RX DMA/Queue | 8 | W | 1/0/0/0/0 | 0/0 | priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:657 |
| 183 | `0x020C` | REG_TXDMA_OFFSET_CHK_8822B (+1: REG_TXDMA_OFFSET_CHK) | MAC | A | USB | 16 | R RMW-set W | 4/0/0/0/0 | 50/1 | init_usb_cfg_88xx, tx_desc_chksum_88xx | hal/halmac/halmac_88xx/halmac_init_88xx.c:734, hal/halmac/halmac_88xx/halmac_init_88xx.c:736, hal/halmac/halmac_88xx/halmac_init_88xx.c:738 … |
| 184 | `0x020D` | *(không có macro)* | MAC | A | Firmware/H2C/C2H | 8 | R RMW-set W | 3/0/0/0/0 | 0/0 | init_h2c_8822b, priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:658, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:869, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:871 |
| 185 | `0x0210` | REG_TXDMA_STATUS_8822B (+1: REG_TXDMA_STATUS) | MAC | A | Firmware/H2C/C2H | 32 | R W | 4/0/1/0/0 | 53/0 | dlfw_end_flow_88xx, rtw_halmac_txfifo_wait_empty, sreset_get_wifi_status, xmit_status_check | core/rtw_sreset.c:97, hal/hal_halmac.c:3822, hal/halmac/halmac_88xx/halmac_fw_88xx.c:696 … |
| 186 | `0x0214` | REG_TX_DMA_DBG_8822B (+1: REG_TX_DMA_DBG) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 187 | `0x0218` | REG_TQPNT1_8822B (+1: REG_TQPNT1) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 188 | `0x021C` | REG_TQPNT2_8822B (+1: REG_TQPNT2) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 189 | `0x0220` | REG_TQPNT3_8822B (+1: REG_TQPNT3) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 190 | `0x0224` | REG_TQPNT4_8822B (+1: REG_TQPNT4) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 191 | `0x0228` | REG_RQPN_CTRL_1_8822B (+1: REG_RQPN_CTRL_1) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 192 | `0x022C` | REG_RQPN_CTRL_2_8822B (+1: REG_RQPN_CTRL_2) | MAC | A | TX/RX DMA/Queue | 32 | R RMW-set W | 3/0/0/0/0 | 9/0 | download_firmware_88xx, priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:608, hal/halmac/halmac_88xx/halmac_fw_88xx.c:217, hal/halmac/halmac_88xx/halmac_fw_88xx.c:220 |
| 193 | `0x0230` | REG_FIFOPAGE_INFO_1_8822B (+1: REG_FIFOPAGE_INFO_1) | MAC | A | TX/RX DMA/Queue | 16/32 | R W | 4/0/1/0/0 | 28/0 | download_firmware_88xx, priority_queue_cfg_8822b, rtw_halmac_txfifo_wait_empty | hal/hal_halmac.c:3822, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:603, hal/halmac/halmac_88xx/halmac_fw_88xx.c:213 … |
| 194 | `0x0234` | REG_FIFOPAGE_INFO_2_8822B (+1: REG_FIFOPAGE_INFO_2) | MAC | A | TX/RX DMA/Queue | 16/32 | R W | 2/0/1/0/0 | 28/0 | priority_queue_cfg_8822b, rtw_halmac_txfifo_wait_empty | hal/hal_halmac.c:3822, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:604 |
| 195 | `0x0238` | REG_FIFOPAGE_INFO_3_8822B (+1: REG_FIFOPAGE_INFO_3) | MAC | A | TX/RX DMA/Queue | 16/32 | R W | 2/0/1/0/0 | 28/0 | priority_queue_cfg_8822b, rtw_halmac_txfifo_wait_empty | hal/hal_halmac.c:3822, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:605 |
| 196 | `0x023C` | REG_FIFOPAGE_INFO_4_8822B (+1: REG_FIFOPAGE_INFO_4) | MAC | A | TX/RX DMA/Queue | 16/32 | R W | 2/0/1/0/0 | 28/0 | priority_queue_cfg_8822b, rtw_halmac_txfifo_wait_empty | hal/hal_halmac.c:3822, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:606 |
| 197 | `0x0240` | REG_FIFOPAGE_INFO_5_8822B (+1: REG_FIFOPAGE_INFO_5) | MAC | A | TX/RX DMA/Queue | 16/32 | R W | 2/0/1/0/0 | 28/0 | priority_queue_cfg_8822b, rtw_halmac_txfifo_wait_empty | hal/hal_halmac.c:3822, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:607 |
| 198 | `0x0244` | REG_H2C_HEAD_8822B (+1: REG_H2C_HEAD) | MAC | A | Firmware/H2C/C2H | 32 | R W | 2/0/0/0/0 | 14/0 | init_h2c_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:848, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:850 |
| 199 | `0x0248` | REG_H2C_TAIL_8822B (+1: REG_H2C_TAIL) | MAC | A | Firmware/H2C/C2H | 32 | R W | 2/0/0/0/0 | 14/0 | init_h2c_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:856, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:859 |
| 200 | `0x024C` | REG_H2C_READ_ADDR_8822B (+1: REG_H2C_READ_ADDR) | MAC | A | Firmware/H2C/C2H | 32 | R W | 2/0/0/0/0 | 14/0 | init_h2c_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:852, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:854 |
| 201 | `0x0250` | REG_H2C_WR_ADDR_8822B (+1: REG_H2C_WR_ADDR) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 202 | `0x0254` | REG_H2C_INFO_8822B (+1: REG_H2C_INFO) | MAC | A | Firmware/H2C/C2H | 8 | R W | 4/0/0/0/0 | 25/0 | init_h2c_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:861, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:863, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:865 … |
| 203 | `0x0280` | REG_RXDMA_AGG_PG_TH_8822B (+1: REG_RXDMA_AGG_PG_TH) | MAC | A | TX/RX DMA/Queue | 16/32 | R W | 6/0/1/2/0 | 50/2 | cfg_usb_rx_agg_88xx, rtw_set_usb_agg_by_mode_normal | hal/hal_com.c:16369, hal/hal_com.c:16399, hal/halmac/halmac_88xx/halmac_usb_88xx.c:176 … |
| 204 | `0x0283` | *(không có macro)* | MAC | A | TX/RX DMA/Queue | 8 | R W | 2/0/0/0/0 | 0/0 | cfg_usb_rx_agg_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:139, hal/halmac/halmac_88xx/halmac_usb_88xx.c:183 |
| 205 | `0x0284` | REG_RXPKT_NUM_8822B (+1: REG_RXPKT_NUM) | MAC | A | TX/RX DMA/Queue | 32 | R W | 12/0/1/0/0 | 50/2 | rtl8822b_sethwreg, rtw_hal_check_wow_ctrl, rtw_hal_force_enable_rxdma, rtw_hal_pause_rx_dma … | hal/hal_com.c:5002, hal/hal_com.c:5003, hal/hal_com.c:5024 … |
| 206 | `0x0286` | *(không có macro)* | MAC | A | TX/RX DMA/Queue | 16 | R | 1/0/0/0/0 | 0/0 | rtw_hal_pause_rx_dma | hal/hal_com.c:5075 |
| 207 | `0x0288` | REG_RXDMA_STATUS_8822B (+1: REG_RXDMA_STATUS) | MAC | A |  | 32 | R | 1/0/1/0/0 | 14/0 | linked_status_check | hal/rtl8822b/rtl8822b_ops.c:982 |
| 208 | `0x028C` | REG_RXDMA_DPR_8822B (+1: REG_RXDMA_DPR) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 209 | `0x0290` | REG_RXDMA_MODE_8822B (+1: REG_RXDMA_MODE) | MAC | A | USB | 8 | W | 1/0/0/0/0 | 45/3 | init_usb_cfg_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:97 |
| 210 | `0x0294` | REG_C2H_PKT_8822B (+1: REG_C2H_PKT) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 211 | `0x0298` | REG_FWFF_C2H_8822B (+1: REG_FWFF_C2H) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 212 | `0x029C` | REG_FWFF_CTRL_8822B (+1: REG_FWFF_CTRL) | MAC | A | Firmware/H2C/C2H | 16 | R W | 2/0/0/0/0 | 30/0 | fwff_is_empty_88xx, init_trx_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:486, hal/halmac/halmac_88xx/halmac_init_88xx.c:975 |
| 213 | `0x02A0` | REG_FWFF_PKT_INFO_8822B (+1: REG_FWFF_PKT_INFO) | MAC | A | Firmware/H2C/C2H | 16 | R | 2/0/0/0/0 | 28/0 | fwff_is_empty_88xx, init_trx_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:485, hal/halmac/halmac_88xx/halmac_init_88xx.c:976 |
| 214 | `0x02F0` | *(không có macro)* | MAC | X | MAC config/WMAC | 8 | R | 0/0/0/1/0 | 0/0 | rtw_hal_backup_rate | hal/hal_com.c:5267 |
| 215 | `0x0300` | REG_PCIE_CTRL_8822B (+3: R_0x300, ODM_DPDT, …) | MAC | X | Power/PS |  |  | 0/0/0/0/1 | 57/0 |  | bảng: TRANS_CARDDIS_TO_CARDEMU_8822B |
| 216 | `0x0301` | *(không có macro)* | MAC | X | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDDIS_TO_CARDEMU_8822B |
| 217 | `0x0304` | REG_INT_MIG_8822B (+1: REG_INT_MIG) | MAC | Z |  |  |  | 0/0/0/0/0 | 70/0 |  |  |
| 218 | `0x0308` | REG_BCNQ_TXBD_DESA_8822B (+1: REG_BCNQ_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 219 | `0x0310` | REG_MGQ_TXBD_DESA_8822B (+1: REG_MGQ_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 220 | `0x0318` | REG_VOQ_TXBD_DESA_8822B (+1: REG_VOQ_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 221 | `0x0320` | REG_VIQ_TXBD_DESA_8822B (+1: REG_VIQ_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 222 | `0x0328` | REG_BEQ_TXBD_DESA_8822B (+1: REG_BEQ_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 223 | `0x0330` | REG_BKQ_TXBD_DESA_8822B (+1: REG_BKQ_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 224 | `0x0338` | REG_RXQ_RXBD_DESA_8822B (+1: REG_RXQ_RXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 225 | `0x0340` | REG_HI0Q_TXBD_DESA_8822B (+1: REG_HI0Q_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 226 | `0x0348` | REG_HI1Q_TXBD_DESA_8822B (+1: REG_HI1Q_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 227 | `0x0350` | REG_HI2Q_TXBD_DESA_8822B (+1: REG_HI2Q_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 228 | `0x0358` | REG_HI3Q_TXBD_DESA_8822B (+1: REG_HI3Q_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 229 | `0x0360` | REG_HI4Q_TXBD_DESA_8822B (+1: REG_HI4Q_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 230 | `0x0368` | REG_HI5Q_TXBD_DESA_8822B (+1: REG_HI5Q_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 231 | `0x0370` | REG_HI6Q_TXBD_DESA_8822B (+1: REG_HI6Q_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 232 | `0x0378` | REG_HI7Q_TXBD_DESA_8822B (+1: REG_HI7Q_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 233 | `0x0380` | REG_MGQ_TXBD_NUM_8822B (+1: REG_MGQ_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 234 | `0x0382` | REG_RX_RXBD_NUM_8822B (+1: REG_RX_RXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 32/0 |  |  |
| 235 | `0x0384` | REG_VOQ_TXBD_NUM_8822B (+1: REG_VOQ_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 236 | `0x0386` | REG_VIQ_TXBD_NUM_8822B (+1: REG_VIQ_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 237 | `0x0388` | REG_BEQ_TXBD_NUM_8822B (+1: REG_BEQ_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 238 | `0x038A` | REG_BKQ_TXBD_NUM_8822B (+1: REG_BKQ_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 239 | `0x038C` | REG_HI0Q_TXBD_NUM_8822B (+1: REG_HI0Q_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 240 | `0x038E` | REG_HI1Q_TXBD_NUM_8822B (+1: REG_HI1Q_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 241 | `0x0390` | REG_HI2Q_TXBD_NUM_8822B (+1: REG_HI2Q_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 242 | `0x0392` | REG_HI3Q_TXBD_NUM_8822B (+1: REG_HI3Q_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 243 | `0x0394` | REG_HI4Q_TXBD_NUM_8822B (+1: REG_HI4Q_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 244 | `0x0396` | REG_HI5Q_TXBD_NUM_8822B (+1: REG_HI5Q_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 245 | `0x0398` | REG_HI6Q_TXBD_NUM_8822B (+1: REG_HI6Q_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 246 | `0x039A` | REG_HI7Q_TXBD_NUM_8822B (+1: REG_HI7Q_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 247 | `0x039C` | REG_BD_RWPTR_CLR_8822B (+3: REG_TSFTIMER_HCI_8822B, REG_BD_RWPTR_CLR, …) | MAC | D |  |  |  | 0/0/0/0/0 | 84/0 |  |  |
| 248 | `0x03A0` | REG_VOQ_TXBD_IDX_8822B (+1: REG_VOQ_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 249 | `0x03A4` | REG_VIQ_TXBD_IDX_8822B (+1: REG_VIQ_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 250 | `0x03A8` | REG_BEQ_TXBD_IDX_8822B (+1: REG_BEQ_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 251 | `0x03AC` | REG_BKQ_TXBD_IDX_8822B (+1: REG_BKQ_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 252 | `0x03B0` | REG_MGQ_TXBD_IDX_8822B (+1: REG_MGQ_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 253 | `0x03B4` | REG_RXQ_RXBD_IDX_8822B (+1: REG_RXQ_RXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 254 | `0x03B8` | REG_HI0Q_TXBD_IDX_8822B (+1: REG_HI0Q_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 255 | `0x03BC` | REG_HI1Q_TXBD_IDX_8822B (+1: REG_HI1Q_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 256 | `0x03C0` | REG_HI2Q_TXBD_IDX_8822B (+1: REG_HI2Q_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 257 | `0x03C4` | REG_HI3Q_TXBD_IDX_8822B (+1: REG_HI3Q_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 258 | `0x03C8` | REG_HI4Q_TXBD_IDX_8822B (+1: REG_HI4Q_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 259 | `0x03CC` | REG_HI5Q_TXBD_IDX_8822B (+1: REG_HI5Q_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 260 | `0x03D0` | REG_HI6Q_TXBD_IDX_8822B (+1: REG_HI6Q_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 261 | `0x03D4` | REG_HI7Q_TXBD_IDX_8822B (+1: REG_HI7Q_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 262 | `0x03D8` | REG_DBG_SEL_V1_8822B (+1: REG_DBG_SEL_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 263 | `0x03D9` | REG_PCIE_HRPWM1_V1_8822B (+1: REG_PCIE_HRPWM1_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 264 | `0x03DA` | REG_PCIE_HCPWM1_V1_8822B (+1: REG_PCIE_HCPWM1_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 265 | `0x03DB` | REG_PCIE_CTRL2_8822B (+1: REG_PCIE_CTRL2) | MAC | D |  |  |  | 0/0/0/0/0 | 23/0 |  |  |
| 266 | `0x03DC` | REG_PCIE_HRPWM2_V1_8822B (+1: REG_PCIE_HRPWM2_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 267 | `0x03DE` | REG_PCIE_HCPWM2_V1_8822B (+1: REG_PCIE_HCPWM2_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 268 | `0x03E0` | REG_PCIE_H2C_MSG_V1_8822B (+1: REG_PCIE_H2C_MSG_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 22/0 |  |  |
| 269 | `0x03E4` | REG_PCIE_C2H_MSG_V1_8822B (+1: REG_PCIE_C2H_MSG_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 270 | `0x03E8` | REG_DBI_WDATA_V1_8822B (+1: REG_DBI_WDATA_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 271 | `0x03EC` | REG_DBI_RDATA_V1_8822B (+1: REG_DBI_RDATA_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 272 | `0x03F0` | REG_DBI_FLAG_V1_8822B (+1: REG_DBI_FLAG_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 19/0 |  |  |
| 273 | `0x03F4` | REG_MDIO_V1_8822B (+1: REG_MDIO_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 274 | `0x03F8` | REG_PCIE_MIX_CFG_8822B (+1: REG_PCIE_MIX_CFG) | MAC | D |  |  |  | 0/0/0/0/0 | 34/0 |  |  |
| 275 | `0x03FC` | REG_HCI_MIX_CFG_8822B (+1: REG_HCI_MIX_CFG) | MAC | D |  |  |  | 0/0/0/0/0 | 60/0 |  |  |
| 276 | `0x0400` | REG_Q0_INFO_8822B (+1: REG_Q0_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 58/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2515 |
| 277 | `0x0404` | REG_Q1_INFO_8822B (+1: REG_Q1_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 58/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2516 |
| 278 | `0x0408` | REG_Q2_INFO_8822B (+1: REG_Q2_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 58/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2517 |
| 279 | `0x040C` | REG_Q3_INFO_8822B (+1: REG_Q3_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 58/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2518 |
| 280 | `0x0410` | REG_MGQ_INFO_8822B (+1: REG_MGQ_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 58/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2523 |
| 281 | `0x0414` | REG_HIQ_INFO_8822B (+1: REG_HIQ_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 58/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2524 |
| 282 | `0x0418` | REG_BCNQ_INFO_8822B (+1: REG_BCNQ_INFO) | MAC | A | TX/RX DMA/Queue | 16/32 | R | 1/0/1/0/0 | 14/0 | rtw_halmac_txfifo_wait_empty | hal/hal_halmac.c:3822 |
| 283 | `0x041A` | REG_TXPKT_EMPTY_8822B (+1: REG_TXPKT_EMPTY) | MAC | A | TX/RX DMA/Queue | 8/16 | R | 3/0/0/0/0 | 16/2 | rtl8822b_gethwreg, txfifo_is_empty_88xx | hal/halmac/halmac_88xx/halmac_common_88xx.c:3415, hal/rtl8822b/rtl8822b_ops.c:2783, hal/rtl8822b/rtl8822b_ops.c:2787 |
| 284 | `0x041B` | *(không có macro)* | MAC | A | TX/RX DMA/Queue | 8 | R | 1/0/0/0/0 | 0/0 | txfifo_is_empty_88xx | hal/halmac/halmac_88xx/halmac_common_88xx.c:3418 |
| 285 | `0x041C` | REG_CPU_MGQ_INFO_8822B (+1: REG_CPU_MGQ_INFO) | MAC | Z |  |  |  | 0/0/0/0/0 | 22/0 |  |  |
| 286 | `0x0420` | REG_FWHW_TXQ_CTRL_8822B (+1: REG_FWHW_TXQ_CTRL) | MAC | A | Beacon/TSF | 8/32 | R W | 8/0/0/0/1 | 38/5 | hw_var_set_bcn_func, rtl8822b_init_misc, set_opmode_port0 | hal/rtl8822b/rtl8822b_halinit.c:289, hal/rtl8822b/rtl8822b_halinit.c:290, hal/rtl8822b/rtl8822b_halinit.c:298 … |
| 287 | `0x0421` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 288 | `0x0422` | *(không có macro)* | MAC | A | Power/PS | 8 | R RMW-set W | 8/0/0/6/0 | 0/0 | ResumeTxBeacon, StopTxBeacon, dl_rsvd_page_88xx, priority_queue_cfg_8822b | hal/hal_com.c:17466, hal/hal_com.c:17467, hal/hal_com.c:17484 … |
| 289 | `0x0423` | REG_DATAFB_SEL_8822B (+1: REG_DATAFB_SEL) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 290 | `0x0424` | REG_BCNQ_BDNY_V1_8822B (+1: REG_BCNQ_BDNY_V1) | MAC | A | TX/RX DMA/Queue | 8/16 | W | 1/0/0/1/0 | 14/0 | priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:629 |
| 291 | `0x0425` | *(không có macro)* | MAC | X | TX/RX DMA/Queue | 8 | W | 0/0/0/1/0 | 0/0 | priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:625 |
| 292 | `0x0426` | REG_LIFETIME_EN_8822B (+1: REG_LIFETIME_EN) | MAC | A | TX/RX DMA/Queue | 8 | R W | 4/0/0/0/0 | 14/0 | cfg_txfifo_lt_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:664, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:666, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:672 … |
| 293 | `0x0428` | REG_SPEC_SIFS_8822B (+1: REG_SPEC_SIFS) | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 28/0 |  | bảng: array_mp_8822b_mac_reg |
| 294 | `0x0429` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 295 | `0x042A` | REG_RETRY_LIMIT_8822B (+1: REG_RETRY_LIMIT) | MAC | A |  | 16 | R W | 5/0/0/0/0 | 28/12 | hw_var_set_mlme_join, rtl8822b_sethwreg | hal/hal_com.c:14170, hal/rtl8822b/rtl8822b_ops.c:1648, hal/rtl8822b/rtl8822b_ops.c:2274 … |
| 296 | `0x042C` | REG_TXBF_CTRL_8822B (+1: REG_TXBF_CTRL) | MAC | A | Beamforming/MU | 16/32 | R RMW-set W | 9/0/0/7/0 | 52/31 | _config_beamformee_su, _reset_beamformee_su, _sounding_config_su, cfg_sounding_88xx … | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:287, hal/rtl8822b/rtl8822b_phy.c:1427, hal/rtl8822b/rtl8822b_phy.c:1471 … |
| 297 | `0x042E` | *(không có macro)* | MAC | X | Beamforming/MU | 16 | R W | 0/0/0/6/0 | 0/0 | cfg_txbf_88xx, su_bfee_entry_del_88xx, su_bfee_entry_init_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:125, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:127, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:382 … |
| 298 | `0x042F` | *(không có macro)* | MAC | A | Beamforming/MU | 8 | R W | 6/0/0/0/0 | 0/0 | _config_beamformee_mu, _sounding_reset_all, del_sounding_88xx, rtl8822b_phy_bf_init | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:339, hal/rtl8822b/rtl8822b_phy.c:1400, hal/rtl8822b/rtl8822b_phy.c:1902 … |
| 299 | `0x0430` | REG_DARFRC_8822B (+2: R_0x430, REG_DARFRC) | MAC | A | MAC config/WMAC | 8/32 | R RMW-mask | 2/0/1/0/1 | 112/0 | phydm_ra_dynamic_retry_count | hal/phydm/phydm_rainfo.c:394, hal/phydm/phydm_rainfo.c:398 |
| 300 | `0x0431` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 301 | `0x0432` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 302 | `0x0433` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 303 | `0x0434` | R_0x434 | MAC | A | MAC config/WMAC | 8/32 | RMW-mask | 2/0/0/0/1 | 0/0 | phydm_ra_dynamic_retry_count | hal/phydm/phydm_rainfo.c:395, hal/phydm/phydm_rainfo.c:399 |
| 304 | `0x0435` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 305 | `0x0436` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 306 | `0x0437` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 307 | `0x0438` | REG_RARFRC_8822B (+1: REG_RARFRC) | MAC | G |  | 32 | R | 0/0/1/0/0 | 112/0 | rtw_dbg_port | os_dep/linux/ioctl_linux.c:6752 |
| 308 | `0x043C` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 309 | `0x043D` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 310 | `0x043E` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 311 | `0x043F` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 312 | `0x0440` | REG_RRSR_8822B (+2: R_0x440, REG_RRSR) | MAC | A | MAC config/WMAC | 8/32 | R R-mask RMW-mask | 5/0/1/0/1 | 30/0 | phydm_masked_rrsr_set_register, phydm_ra_info_init, phydm_rrsr_set_register, rtw_hal_beamforming_config_csirate … | hal/hal_com.c:14796, hal/hal_com.c:15366, hal/phydm/phydm_rainfo.c:1813 … |
| 313 | `0x0441` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 314 | `0x0442` | *(không có macro)* | MAC | A | MAC config/WMAC | 8 | R W | 2/0/0/0/1 | 0/0 | rtw_var_set_basic_rate | hal/hal_com.c:14801 |
| 315 | `0x0444` | REG_ARFR0_8822B (+2: R_0x444, REG_ARFR0) | MAC | A | Init table | 8/32 | R | 0/0/1/0/1 | 14/0 |  | bảng: array_mp_8822b_mac_reg |
| 316 | `0x0445` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 317 | `0x0446` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 318 | `0x0447` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 319 | `0x0448` | R_0x448 | MAC | A | Init table | 8/32 | R RMW-mask | 0/0/1/1/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 320 | `0x0449` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 321 | `0x044A` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 322 | `0x044B` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 323 | `0x044C` | REG_ARFR1_V1_8822B (+1: REG_ARFR1_V1) | MAC | A | Init table | 8/32 | R | 0/0/1/0/1 | 14/0 |  | bảng: array_mp_8822b_mac_reg |
| 324 | `0x044D` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 325 | `0x044E` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 326 | `0x044F` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 327 | `0x0450` | R_0x450 | MAC | A | Firmware/H2C/C2H | 8/32 | R RMW-mask | 1/0/1/0/1 | 0/0 | phydm_fw_fix_rate | hal/phydm/phydm_rainfo.c:203 |
| 328 | `0x0451` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 329 | `0x0452` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 330 | `0x0453` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 331 | `0x0454` | REG_CCK_CHECK_8822B (+2: R_0x454, REG_CCK_CHECK) | MAC | A | Channel/BW | 8/32 | R RMW-mask W | 6/0/0/0/0 | 16/1 | cfg_ch_88xx, config_phydm_switch_band_8822b, set_opmode_port0 | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:578, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:584, hal/phydm/rtl8822b/phydm_hal_api8822b.c:1614 … |
| 332 | `0x0455` | REG_AMPDU_MAX_TIME_V1_8822B (+1: REG_AMPDU_MAX_TIME_V1) | MAC | A | Init table | 8 | R W | 2/0/0/1/1 | 14/0 | init_protocol_cfg_8822b, sethwreg | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:804, hal/rtl8822b/usb/rtl8822bu_ops.c:141 |
| 333 | `0x0456` | REG_BCNQ1_BDNY_V1_8822B (+1: REG_BCNQ1_BDNY_V1) | MAC | A | TX/RX DMA/Queue | 8/16 | W | 1/0/0/1/0 | 14/0 | priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:644 |
| 334 | `0x0457` | *(không có macro)* | MAC | X | TX/RX DMA/Queue | 8 | W | 0/0/0/1/0 | 0/0 | priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:640 |
| 335 | `0x0458` | REG_AMPDU_MAX_LENGTH_8822B (+1: REG_AMPDU_MAX_LENGTH) | MAC | A | MAC config/WMAC | 32 | R RMW-mask W | 2/1/1/0/0 | 14/0 | cfg_ampdu_8822b, rtl8822b_sethwreg | hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:182, hal/rtl8822b/rtl8822b_ops.c:2139 |
| 336 | `0x045C` | REG_ACQ_STOP_8822B (+1: REG_ACQ_STOP) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 337 | `0x045D` | REG_NDPA_RATE_8822B (+1: REG_NDPA_RATE) | MAC | A | Beamforming/MU | 8 | W | 2/0/0/0/0 | 14/0 | _config_beamformee_mu, cfg_sounding_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:288, hal/rtl8822b/rtl8822b_phy.c:1908 |
| 338 | `0x045E` | REG_TX_HANG_CTRL_8822B (+1: REG_TX_HANG_CTRL) | MAC | A | Init table | 8 | RMW-set | 1/0/0/0/1 | 8/1 | init_protocol_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:805 |
| 339 | `0x045F` | REG_NDPA_OPT_CTRL_8822B (+1: REG_NDPA_OPT_CTRL) | MAC | A | Beamforming/MU | 8 | R W | 3/0/0/0/0 | 32/6 | _config_beamformee_mu, rtl8822b_phy_bf_init | hal/rtl8822b/rtl8822b_phy.c:1910, hal/rtl8822b/rtl8822b_phy.c:1912, hal/rtl8822b/rtl8822b_phy.c:2145 |
| 340 | `0x0460` | *(không có macro)* | MAC | M | MAC config/WMAC | 32 | R-mask RMW-mask | 0/2/0/0/0 | 0/0 | Rtw_MPSetMacTxEDCA | core/rtw_mp.c:1895, core/rtw_mp.c:1899 |
| 341 | `0x0463` | REG_RD_RESP_PKT_TH_8822B (+1: REG_RD_RESP_PKT_TH) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 342 | `0x0464` | REG_CMDQ_INFO_8822B (+1: REG_CMDQ_INFO) | MAC | D |  |  |  | 0/0/0/0/0 | 58/0 |  |  |
| 343 | `0x0468` | REG_Q4_INFO_8822B (+1: REG_Q4_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 58/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2519 |
| 344 | `0x046C` | REG_Q5_INFO_8822B (+1: REG_Q5_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 58/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2520 |
| 345 | `0x0470` | REG_Q6_INFO_8822B (+1: REG_Q6_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 58/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2521 |
| 346 | `0x0474` | REG_Q7_INFO_8822B (+1: REG_Q7_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 58/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2522 |
| 347 | `0x0478` | REG_WMAC_LBK_BUF_HD_V1_8822B (+1: REG_WMAC_LBK_BUF_HD_V1) | MAC | A | TX/RX DMA/Queue | 16 | W | 1/0/0/0/0 | 14/0 | priority_queue_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:671 |
| 348 | `0x047A` | REG_MGQ_BDNY_V1_8822B (+1: REG_MGQ_BDNY_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 349 | `0x047C` | REG_TXRPT_CTRL_8822B (+1: REG_TXRPT_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 56/0 |  |  |
| 350 | `0x0480` | REG_INIRTS_RATE_SEL_8822B (+1: REG_INIRTS_RATE_SEL) | MAC | A | Channel/BW | 8 | R W | 6/0/0/0/0 | 2/0 | SetHwReg, init_protocol_cfg_8822b, rts_full_bw_88xx | hal/hal_com.c:14890, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:820, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:821 … |
| 351 | `0x0481` | REG_BASIC_CFEND_RATE_8822B (+1: REG_BASIC_CFEND_RATE) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 352 | `0x0482` | REG_STBC_CFEND_RATE_8822B (+1: REG_STBC_CFEND_RATE) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 353 | `0x0483` | REG_DATA_SC_8822B (+1: REG_DATA_SC) | MAC | A |  | 8 | W | 1/0/0/0/0 | 28/6 | cfg_pri_ch_idx_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:605 |
| 354 | `0x0484` | REG_MACID_SLEEP3_8822B (+1: REG_MACID_SLEEP3) | MAC | G |  | 32 | R | 0/0/1/0/0 | 14/0 | rtw_dbg_port | os_dep/linux/ioctl_linux.c:6758 |
| 355 | `0x0488` | REG_MACID_SLEEP1_8822B (+1: REG_MACID_SLEEP1) | MAC | G |  | 32 | R | 0/0/1/0/0 | 14/0 | rtw_dbg_port | os_dep/linux/ioctl_linux.c:6759 |
| 356 | `0x048C` | REG_ARFR2_V1_8822B (+1: REG_ARFR2_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 357 | `0x0494` | REG_ARFR3_V1_8822B (+1: REG_ARFR3_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 358 | `0x049C` | REG_ARFR4_8822B (+2: R_0x49c, REG_ARFR4) | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 14/0 |  | bảng: array_mp_8822b_mac_reg |
| 359 | `0x049D` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 360 | `0x049E` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 361 | `0x049F` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 362 | `0x04A0` | R_0x4a0 | MAC | A | Init table | 8/32 | RMW-mask | 0/0/0/1/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 363 | `0x04A1` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 364 | `0x04A2` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 365 | `0x04A3` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 366 | `0x04A4` | REG_ARFR5_8822B (+2: R_0x4a4, REG_ARFR5) | MAC | A | Init table | 8/32 | R | 0/0/0/1/1 | 14/0 |  | bảng: array_mp_8822b_mac_reg |
| 367 | `0x04A5` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 368 | `0x04A6` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 369 | `0x04A7` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 370 | `0x04A8` | R_0x4a8 | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 371 | `0x04A9` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 372 | `0x04AA` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 373 | `0x04AB` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 374 | `0x04AC` | REG_TXRPT_START_OFFSET_8822B (+1: REG_TXRPT_START_OFFSET) | MAC | D |  |  |  | 0/0/0/0/0 | 44/0 |  |  |
| 375 | `0x04B4` | REG_POWER_STAGE1_8822B (+1: REG_POWER_STAGE1) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 376 | `0x04B8` | REG_POWER_STAGE2_8822B (+1: REG_POWER_STAGE2) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 377 | `0x04BC` | REG_SW_AMPDU_BURST_MODE_CTRL_8822B (+1: REG_SW_AMPDU_BURST_MODE_CTRL) | MAC | A |  | 8 | RMW-clr | 1/0/0/0/0 | 62/0 | init_protocol_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:802 |
| 378 | `0x04C0` | REG_PKT_LIFE_TIME_8822B (+1: REG_PKT_LIFE_TIME) | MAC | A | TX/RX DMA/Queue | 32 | W | 1/0/0/0/0 | 28/0 | cfg_txfifo_lt_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:670 |
| 379 | `0x04C4` | REG_STBC_SETTING_8822B (+1: REG_STBC_SETTING) | MAC | D |  |  |  | 0/0/0/0/0 | 42/0 |  |  |
| 380 | `0x04C5` | REG_STBC_SETTING2_8822B (+1: REG_STBC_SETTING2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 381 | `0x04C6` | REG_QUEUE_CTRL_8822B (+1: REG_QUEUE_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 12/0 |  |  |
| 382 | `0x04C7` | REG_SINGLE_AMPDU_CTRL_8822B (+1: REG_SINGLE_AMPDU_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 2/0 |  |  |
| 383 | `0x04C8` | REG_PROT_MODE_CTRL_8822B (+2: R_0x4c8, REG_PROT_MODE_CTRL) | MAC | A | Init table | 8/32 | R-mask W | 2/0/0/0/1 | 56/0 | init_protocol_cfg_8822b, phydm_ra_info_init | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:810, hal/phydm/phydm_rainfo.c:1928 |
| 384 | `0x04C9` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 385 | `0x04CA` | *(không có macro)* | MAC | A | MAC config/WMAC | 8 | W | 1/0/0/0/1 | 0/0 | cfg_ampdu_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:178 |
| 386 | `0x04CB` | *(không có macro)* | MAC | A | MAC config/WMAC | 8 | W | 1/0/0/0/1 | 0/0 | cfg_ampdu_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:179 |
| 387 | `0x04CC` | REG_BAR_MODE_CTRL_8822B (+2: R_0x4cc, REG_BAR_MODE_CTRL) | MAC | A | Init table | 8/32 | RMW-mask | 1/0/0/0/1 | 44/0 | phydm_ra_info_init | hal/phydm/phydm_rainfo.c:1929 |
| 388 | `0x04CD` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 389 | `0x04CE` | *(không có macro)* | MAC | A | Init table | 8/16 | W | 1/0/0/0/1 | 0/0 | init_protocol_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:812 |
| 390 | `0x04CF` | REG_RA_TRY_RATE_AGG_LMT_8822B (+1: REG_RA_TRY_RATE_AGG_LMT) | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 14/0 |  | bảng: array_mp_8822b_mac_reg |
| 391 | `0x04D0` | REG_MACID_SLEEP2_8822B (+2: REG_MACID_SLEEP2, ODM_REG_EARLY_MODE_11N) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 392 | `0x04D4` | REG_MACID_SLEEP_8822B (+1: REG_MACID_SLEEP) | MAC | A |  | 32 | R | 1/0/0/0/0 | 14/0 | mcc_get_reg_hdl | hal/hal_mcc.c:2748 |
| 393 | `0x04D8` | REG_HW_SEQ0_8822B (+1: REG_HW_SEQ0) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 394 | `0x04DA` | REG_HW_SEQ1_8822B (+1: REG_HW_SEQ1) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 395 | `0x04DC` | REG_HW_SEQ2_8822B (+1: REG_HW_SEQ2) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 396 | `0x04DE` | REG_HW_SEQ3_8822B (+1: REG_HW_SEQ3) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 397 | `0x04E0` | REG_NULL_PKT_STATUS_V1_8822B (+1: REG_NULL_PKT_STATUS_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 398 | `0x04E2` | REG_PTCL_ERR_STATUS_8822B (+1: REG_PTCL_ERR_STATUS) | MAC | D |  |  |  | 0/0/0/0/0 | 15/0 |  |  |
| 399 | `0x04E3` | REG_NULL_PKT_STATUS_EXTEND_8822B (+1: REG_NULL_PKT_STATUS_EXTEND) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 400 | `0x04E4` | REG_VIDEO_ENHANCEMENT_FUN_8822B (+1: REG_VIDEO_ENHANCEMENT_FUN) | MAC | D |  |  |  | 0/0/0/0/0 | 4/0 |  |  |
| 401 | `0x04E8` | REG_BT_POLLUTE_PKT_CNT_8822B (+1: REG_BT_POLLUTE_PKT_CNT) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 402 | `0x04EC` | REG_PTCL_DBG_8822B (+1: REG_PTCL_DBG) | MAC | D |  |  |  | 0/0/0/0/0 | 7/0 |  |  |
| 403 | `0x04F4` | REG_CPUMGQ_TIMER_CTRL2_8822B (+1: REG_CPUMGQ_TIMER_CTRL2) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 404 | `0x04FC` | REG_DUMMY_PAGE4_V1_8822B (+1: REG_DUMMY_PAGE4_V1) | MAC | A |  | 8 | W | 1/0/0/0/0 | 11/0 | set_opmode_port0 | hal/rtl8822b/rtl8822b_ops.c:1137 |
| 405 | `0x04FE` | REG_MOREDATA_8822B (+3: ODM_DUMMY, REG_MOREDATA, …) | MAC | D |  |  |  | 0/0/0/0/0 | 6/0 |  |  |
| 406 | `0x0500` | REG_EDCA_VO_PARAM_8822B (+3: ODM_EDCA_VO_PARAM, REG_EDCA_VO_PARAM, …) | MAC | A | Init table | 8/32 | R W | 2/0/0/0/1 | 21/0 | rtl8822b_gethwreg, rtl8822b_sethwreg | hal/rtl8822b/rtl8822b_ops.c:2111, hal/rtl8822b/rtl8822b_ops.c:2667 |
| 407 | `0x0501` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 408 | `0x0502` | *(không có macro)* | MAC | A | MAC config/WMAC | 8/16 | W | 1/0/0/0/1 | 0/0 | init_edca_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:910 |
| 409 | `0x0503` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 410 | `0x0504` | REG_EDCA_VI_PARAM_8822B (+3: ODM_EDCA_VI_PARAM, REG_EDCA_VI_PARAM, …) | MAC | A | Init table | 8/32 | R W | 2/0/0/0/1 | 0/0 | rtl8822b_gethwreg, rtl8822b_sethwreg | hal/rtl8822b/rtl8822b_ops.c:2115, hal/rtl8822b/rtl8822b_ops.c:2675 |
| 411 | `0x0505` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 412 | `0x0506` | *(không có macro)* | MAC | A | MAC config/WMAC | 8/16 | W | 1/0/0/0/1 | 0/0 | init_edca_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:911 |
| 413 | `0x0507` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 414 | `0x0508` | REG_EDCA_BE_PARAM_8822B (+3: ODM_EDCA_BE_PARAM, REG_EDCA_BE_PARAM, …) | MAC | A | Init table | 8/32 | R W | 2/1/0/0/1 | 0/0 | rtl8822b_gethwreg, rtl8822b_sethwreg | hal/rtl8822b/rtl8822b_ops.c:2120, hal/rtl8822b/rtl8822b_ops.c:2683 |
| 415 | `0x0509` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 416 | `0x050A` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 417 | `0x050B` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 418 | `0x050C` | REG_EDCA_BK_PARAM_8822B (+3: ODM_EDCA_BK_PARAM, REG_EDCA_BK_PARAM, …) | MAC | A | Init table | 8/32 | R W | 2/0/0/0/1 | 0/0 | rtl8822b_gethwreg, rtl8822b_sethwreg | hal/rtl8822b/rtl8822b_ops.c:2124, hal/rtl8822b/rtl8822b_ops.c:2691 |
| 419 | `0x050D` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 420 | `0x050E` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 421 | `0x050F` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 422 | `0x0510` | REG_BCNTCFG_8822B (+1: REG_BCNTCFG) | MAC | A | Beacon/TSF | 16 | W | 1/0/0/0/0 | 42/0 | InitBeaconParameters | hal/rtl8822b/rtl8822b_ops.c:804 |
| 423 | `0x0512` | REG_PIFS_8822B (+1: REG_PIFS) | MAC | A | MAC config/WMAC | 8 | W | 1/0/0/0/1 | 14/0 | init_edca_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:907 |
| 424 | `0x0513` | REG_RDG_PIFS_8822B (+1: REG_RDG_PIFS) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 425 | `0x0514` | REG_SIFS_8822B (+1: REG_SIFS) | MAC | A | MAC config/WMAC | 8/32 | W | 1/0/0/0/1 | 56/3 | init_edca_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:908 |
| 426 | `0x0516` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 427 | `0x0518` | REG_TSFTR_SYN_OFFSET_8822B (+1: REG_TSFTR_SYN_OFFSET) | MAC | A | Beacon/TSF | 16 | W | 2/0/0/0/0 | 14/0 | hw_port0_tsf_sync_sel, set_opmode_port0 | hal/hal_com.c:14287, hal/rtl8822b/rtl8822b_ops.c:1153 |
| 428 | `0x051A` | REG_AGGR_BREAK_TIME_8822B (+1: REG_AGGR_BREAK_TIME) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 429 | `0x051B` | REG_SLOT_8822B (+1: REG_SLOT) | MAC | A | MAC config/WMAC | 8 | W | 3/0/0/0/0 | 14/0 | init_edca_cfg_8822b, rtl8822b_sethwreg, set_beacon_related_registers | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:906, hal/rtl8822b/rtl8822b_ops.c:2072, hal/rtl8822b/rtl8822b_ops.c:868 |
| 430 | `0x0520` | REG_TX_PTCL_CTRL_8822B (+2: R_0x520, REG_TX_PTCL_CTRL) | MAC | A | Calibration | 32 | R RMW-mask W | 5/0/0/0/0 | 38/0 | _iqk_backup_mac_bb_8822b, _iqk_restore_mac_bb_8822b, phydm_mac_edcca_state, phydm_stop_ic_trx | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:211, hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:327, hal/phydm/phydm_adaptivity.c:233 … |
| 431 | `0x0521` | *(không có macro)* | MAC | A | MAC config/WMAC | 8 | RMW-clr | 1/0/0/0/1 | 0/0 | init_edca_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:925 |
| 432 | `0x0522` | REG_TXPAUSE_8822B (+4: R_0x522, ODM_TXPAUSE, …) | MAC | A | Calibration | 8/16/32 | R W | 14/0/0/0/0 | 16/0 | _iqk_configure_macbb_8822b, free_download_firmware_88xx, init_edca_cfg_8822b, mcc_get_reg_hdl … | hal/hal_com.c:11405, hal/hal_com.c:11406, hal/hal_com.c:11422 … |
| 433 | `0x0523` | REG_DIS_TXREQ_CLR_8822B (+1: REG_DIS_TXREQ_CLR) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 434 | `0x0524` | REG_RD_CTRL_8822B (+2: R_0x524, REG_RD_CTRL) | MAC | A | BB DM (DIG/CCA/FA/CCX) | 32 | RMW-mask | 2/0/0/0/0 | 30/0 | phydm_mac_edcca_state | hal/phydm/phydm_adaptivity.c:235, hal/phydm/phydm_adaptivity.c:240 |
| 435 | `0x0525` | *(không có macro)* | MAC | A | Beacon/TSF | 8 | W | 1/0/0/0/1 | 0/0 | beacon_function_enable | hal/rtl8822b/rtl8822b_ops.c:824 |
| 436 | `0x0526` | REG_MBSSID_CTRL_8822B (+1: REG_MBSSID_CTRL) | MAC | Z |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 437 | `0x0527` | REG_P2PPS_CTRL_8822B (+1: REG_P2PPS_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 9/0 |  |  |
| 438 | `0x0528` | REG_PKT_LIFETIME_CTRL_8822B (+1: REG_PKT_LIFETIME_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 44/0 |  |  |
| 439 | `0x052B` | REG_P2PPS_SPEC_STATE_8822B (+1: REG_P2PPS_SPEC_STATE) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 440 | `0x052C` | REG_TXOP_LIMIT_CTRL_8822B (+1: REG_TXOP_LIMIT_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 72/0 |  |  |
| 441 | `0x0530` | REG_BAR_TX_CTRL_8822B (+1: REG_BAR_TX_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 442 | `0x0531` | REG_P2PON_DIS_TXTIME_8822B (+1: REG_P2PON_DIS_TXTIME) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 443 | `0x0538` | REG_QUEUE_INCOL_THR_8822B (+1: REG_QUEUE_INCOL_THR) | MAC | D |  |  |  | 0/0/0/0/0 | 56/0 |  |  |
| 444 | `0x053C` | REG_QUEUE_INCOL_EN_8822B (+1: REG_QUEUE_INCOL_EN) | MAC | D |  |  |  | 0/0/0/0/0 | 58/0 |  |  |
| 445 | `0x0540` | REG_TBTT_PROHIBIT_8822B (+1: REG_TBTT_PROHIBIT) | MAC | A | Beacon/TSF | 8/32 | W | 3/0/0/0/0 | 21/1 | InitBeaconParameters, ResumeTxBeacon, init_edca_cfg_8822b | hal/hal_com.c:17472, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:921, hal/rtl8822b/rtl8822b_ops.c:790 |
| 446 | `0x0541` | *(không có macro)* | MAC | A | Beacon/TSF | 8 | W | 3/0/0/0/0 | 0/0 | InitBeaconParameters, ResumeTxBeacon, StopTxBeacon | hal/hal_com.c:17476, hal/hal_com.c:17488, hal/rtl8822b/rtl8822b_ops.c:793 |
| 447 | `0x0542` | *(không có macro)* | MAC | A | Beacon/TSF | 8 | R W | 8/0/0/0/0 | 0/0 | InitBeaconParameters, ResumeTxBeacon, StopTxBeacon, hw_var_set_mlme_join | hal/hal_com.c:14157, hal/hal_com.c:17477, hal/hal_com.c:17478 … |
| 448 | `0x0543` | REG_P2PPS_STATE_8822B (+1: REG_P2PPS_STATE) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 449 | `0x0544` | REG_RD_NAV_NXT_8822B (+1: REG_RD_NAV_NXT) | MAC | A | MAC config/WMAC | 32 | W | 1/0/0/0/0 | 14/0 | init_edca_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:913 |
| 450 | `0x0546` | REG_NAV_PROT_LEN_8822B (+1: REG_NAV_PROT_LEN) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 451 | `0x0550` | REG_BCN_CTRL_8822B (+2: R_0x550, REG_BCN_CTRL) | MAC | A | Beacon/TSF | 8/16/32 | R RMW-mask W | 21/1/0/8/0 | 10/7 | InitBeaconParameters, _iqk_backup_mac_bb_8822b, _iqk_configure_macbb_8822b, _iqk_restore_mac_bb_8822b … | hal/hal_com.c:14283, hal/hal_com.c:14309, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:917 … |
| 452 | `0x0551` | REG_BCN_CTRL_CLINT0_8822B (+3: R_0x551, REG_BCN_CTRL_1, …) | MAC | A | Beacon/TSF | 8 | R W | 6/0/0/0/1 | 12/3 | hw_var_set_mlme_disconnect, rw_bcn_ctrl_88xx, set_opmode_port1 | hal/hal_com.c:14023, hal/hal_com.c:14025, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:429 … |
| 453 | `0x0552` | REG_MBID_NUM_8822B (+1: REG_MBID_NUM) | MAC | Z |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 454 | `0x0553` | REG_DUAL_TSF_RST_8822B (+1: REG_DUAL_TSF_RST) | MAC | A | Beacon/TSF | 8 | R W | 17/0/0/0/0 | 12/11 | cfg_tsf_rst_88xx, hw_var_set_mlme_disconnect, hw_var_set_mlme_join, rtl8822b_sethwreg … | hal/hal_com.c:14013, hal/hal_com.c:14031, hal/hal_com.c:14150 … |
| 455 | `0x0554` | REG_MBSSID_BCN_SPACE_8822B (+1: REG_MBSSID_BCN_SPACE) | MAC | A | Beacon/TSF | 16 | W | 1/0/0/0/0 | 42/0 | cfg_bcn_space_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:353 |
| 456 | `0x0556` | *(không có macro)* | MAC | A | Beacon/TSF | 16 | R W | 2/0/0/0/0 | 0/0 | cfg_bcn_space_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:356, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:358 |
| 457 | `0x0557` | *(không có macro)* | MAC | A | Beacon/TSF | 8 | R W | 2/0/0/0/0 | 0/0 | rtw_hal_get_tsftr_by_port | hal/hal_com.c:14592, hal/hal_com.c:14595 |
| 458 | `0x0558` | REG_DRVERLYINT_8822B (+1: REG_DRVERLYINT) | MAC | A | Beacon/TSF | 8 | W | 2/0/0/0/0 | 7/0 | InitBeaconParameters, init_edca_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:922, hal/rtl8822b/rtl8822b_ops.c:797 |
| 459 | `0x0559` | REG_BCNDMATIM_8822B (+1: REG_BCNDMATIM) | MAC | A | Beacon/TSF | 8 | W | 3/0/0/0/1 | 7/0 | InitBeaconParameters, init_edca_cfg_8822b, set_opmode_port0 | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:923, hal/rtl8822b/rtl8822b_ops.c:1148, hal/rtl8822b/rtl8822b_ops.c:798 |
| 460 | `0x055A` | REG_ATIMWND_8822B (+1: REG_ATIMWND) | MAC | A | Beacon/TSF | 8/16 | W | 2/0/0/0/0 | 14/0 | set_beacon_related_registers, set_opmode_port0 | hal/rtl8822b/rtl8822b_ops.c:1150, hal/rtl8822b/rtl8822b_ops.c:859 |
| 461 | `0x055C` | REG_USTIME_TSF_8822B (+1: REG_USTIME_TSF) | MAC | A | MAC config/WMAC | 8 | W | 1/0/0/0/1 | 14/0 | cfg_mac_clk_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:763 |
| 462 | `0x055D` | REG_BCN_MAX_ERR_8822B (+1: REG_BCN_MAX_ERR) | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 14/0 |  | bảng: array_mp_8822b_mac_reg |
| 463 | `0x055E` | REG_RXTSF_OFFSET_CCK_8822B (+1: REG_RXTSF_OFFSET_CCK) | MAC | A | MAC config/WMAC | 8/16 | W | 2/0/0/0/0 | 14/0 | init_edca_cfg_8822b, set_beacon_related_registers | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:914, hal/rtl8822b/rtl8822b_ops.c:880 |
| 464 | `0x055F` | REG_RXTSF_OFFSET_OFDM_8822B (+1: REG_RXTSF_OFFSET_OFDM) | MAC | A | Beacon/TSF | 8 | W | 1/0/0/0/0 | 14/0 | set_beacon_related_registers | hal/rtl8822b/rtl8822b_ops.c:881 |
| 465 | `0x0560` | REG_TSFTR_8822B (+1: REG_TSFTR) | MAC | A | Beacon/TSF | 32 | R | 2/0/0/0/0 | 14/0 | rm_state_idle, rtw_hal_get_tsftr_by_port | core/rtw_rm_fsm.c:595, hal/hal_com.c:14599 |
| 466 | `0x0564` | *(không có macro)* | MAC | A | Beacon/TSF | 32 | R | 1/0/0/0/0 | 0/0 | rtw_hal_get_tsftr_by_port | hal/hal_com.c:14597 |
| 467 | `0x0568` | REG_FREERUN_CNT_8822B (+2: R_0x568, REG_FREERUN_CNT) | MAC | Z |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 468 | `0x0570` | REG_ATIMWND1_V1_8822B (+1: REG_ATIMWND1_V1) | MAC | Z |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 469 | `0x0571` | REG_TBTT_PROHIBIT_INFRA_8822B (+1: REG_TBTT_PROHIBIT_INFRA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 470 | `0x0572` | REG_CTWND_8822B (+1: REG_CTWND) | MAC | D |  |  |  | 0/0/0/0/0 | 7/0 |  |  |
| 471 | `0x0573` | REG_BCNIVLCUNT_8822B (+1: REG_BCNIVLCUNT) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 472 | `0x0574` | REG_BCNDROPCTRL_8822B (+1: REG_BCNDROPCTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 473 | `0x0575` | REG_HGQ_TIMEOUT_PERIOD_8822B (+1: REG_HGQ_TIMEOUT_PERIOD) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 474 | `0x0576` | REG_TXCMD_TIMEOUT_PERIOD_8822B (+1: REG_TXCMD_TIMEOUT_PERIOD) | MAC | D |  |  |  | 0/0/0/0/0 | 21/0 |  |  |
| 475 | `0x0577` | REG_MISC_CTRL_8822B (+1: REG_MISC_CTRL) | MAC | A | Beacon/TSF | 8 | R RMW-set W | 9/0/0/0/1 | 24/2 | hw_port0_tsf_sync_sel, rtl8822b_sethwreg, set_hw_value_88xx | hal/hal_com.c:14290, hal/halmac/halmac_88xx/halmac_common_88xx.c:611, hal/rtl8822b/rtl8822b_ops.c:2429 … |
| 476 | `0x0578` | REG_BCN_CTRL_CLINT1_8822B (+2: REG_BCN_CTRL_2, REG_BCN_CTRL_CLINT1) | MAC | A | Beacon/TSF | 8 | R W | 2/0/0/0/0 | 12/2 | rw_bcn_ctrl_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:432, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:453 |
| 477 | `0x0579` | REG_BCN_CTRL_CLINT2_8822B (+2: REG_BCN_CTRL_3, REG_BCN_CTRL_CLINT2) | MAC | A | Beacon/TSF | 8 | R W | 2/0/0/0/0 | 12/2 | rw_bcn_ctrl_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:435, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:456 |
| 478 | `0x057A` | REG_BCN_CTRL_CLINT3_8822B (+2: REG_BCN_CTRL_4, REG_BCN_CTRL_CLINT3) | MAC | A | Beacon/TSF | 8 | R W | 2/0/0/0/0 | 12/2 | rw_bcn_ctrl_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:438, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:459 |
| 479 | `0x057B` | REG_EXTEND_CTRL_8822B (+1: REG_EXTEND_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 480 | `0x057C` | REG_P2PPS1_SPEC_STATE_8822B (+1: REG_P2PPS1_SPEC_STATE) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 481 | `0x057D` | REG_P2PPS1_STATE_8822B (+1: REG_P2PPS1_STATE) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 482 | `0x057E` | REG_P2PPS2_SPEC_STATE_8822B (+1: REG_P2PPS2_SPEC_STATE) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 483 | `0x057F` | REG_P2PPS2_STATE_8822B (+1: REG_P2PPS2_STATE) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 484 | `0x0580` | REG_PS_TIMER0_8822B (+1: REG_PS_TIMER0) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 485 | `0x0584` | REG_PS_TIMER1_8822B (+1: REG_PS_TIMER1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 486 | `0x0588` | REG_PS_TIMER2_8822B (+2: R_0x588, REG_PS_TIMER2) | MAC | Z |  |  |  | 0/0/0/0/0 | 21/0 |  |  |
| 487 | `0x058C` | REG_TBTT_CTN_AREA_8822B (+1: REG_TBTT_CTN_AREA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 488 | `0x058E` | REG_FORCE_BCN_IFS_8822B (+1: REG_FORCE_BCN_IFS) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 489 | `0x0590` | REG_TXOP_MIN_8822B (+1: REG_TXOP_MIN) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 490 | `0x0592` | REG_PRE_BKF_TIME_8822B (+1: REG_PRE_BKF_TIME) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 491 | `0x0593` | REG_CROSS_TXOP_CTRL_8822B (+1: REG_CROSS_TXOP_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 6/0 |  |  |
| 492 | `0x05A0` | REG_ATIMWND2_8822B (+1: REG_ATIMWND2) | MAC | Z |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 493 | `0x05A1` | REG_ATIMWND3_8822B (+1: REG_ATIMWND3) | MAC | Z |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 494 | `0x05A2` | REG_ATIMWND4_8822B (+1: REG_ATIMWND4) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 495 | `0x05A3` | REG_ATIMWND5_8822B (+1: REG_ATIMWND5) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 496 | `0x05A4` | REG_ATIMWND6_8822B (+1: REG_ATIMWND6) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 497 | `0x05A5` | REG_ATIMWND7_8822B (+1: REG_ATIMWND7) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 498 | `0x05A6` | REG_ATIMUGT_8822B (+1: REG_ATIMUGT) | MAC | D |  |  |  | 0/0/0/0/0 | 7/0 |  |  |
| 499 | `0x05A7` | REG_HIQ_NO_LMT_EN_8822B (+1: REG_HIQ_NO_LMT_EN) | MAC | Z |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 500 | `0x05A8` | REG_DTIM_COUNTER_ROOT_8822B (+1: REG_DTIM_COUNTER_ROOT) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 501 | `0x05A9` | REG_DTIM_COUNTER_VAP1_8822B (+1: REG_DTIM_COUNTER_VAP1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 502 | `0x05AA` | REG_DTIM_COUNTER_VAP2_8822B (+1: REG_DTIM_COUNTER_VAP2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 503 | `0x05AB` | REG_DTIM_COUNTER_VAP3_8822B (+1: REG_DTIM_COUNTER_VAP3) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 504 | `0x05AC` | REG_DTIM_COUNTER_VAP4_8822B (+1: REG_DTIM_COUNTER_VAP4) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 505 | `0x05AD` | REG_DTIM_COUNTER_VAP5_8822B (+1: REG_DTIM_COUNTER_VAP5) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 506 | `0x05AE` | REG_DTIM_COUNTER_VAP6_8822B (+1: REG_DTIM_COUNTER_VAP6) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 507 | `0x05AF` | REG_DTIM_COUNTER_VAP7_8822B (+1: REG_DTIM_COUNTER_VAP7) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 508 | `0x05B0` | REG_DIS_ATIM_8822B (+1: REG_DIS_ATIM) | MAC | A |  | 8 | R W | 4/0/0/0/0 | 17/1 | set_opmode_port0 | hal/rtl8822b/rtl8822b_ops.c:1090, hal/rtl8822b/rtl8822b_ops.c:1092, hal/rtl8822b/rtl8822b_ops.c:1128 … |
| 509 | `0x05B1` | REG_EARLY_128US_8822B (+1: REG_EARLY_128US) | MAC | D |  |  |  | 0/0/0/0/0 | 21/0 |  |  |
| 510 | `0x05B2` | REG_P2PPS1_CTRL_8822B (+1: REG_P2PPS1_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 10/0 |  |  |
| 511 | `0x05B3` | REG_P2PPS2_CTRL_8822B (+1: REG_P2PPS2_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 10/0 |  |  |
| 512 | `0x05B4` | REG_TIMER0_SRC_SEL_8822B (+1: REG_TIMER0_SRC_SEL) | MAC | A | Beacon/TSF | 8 | R RMW-clr W | 4/0/0/0/0 | 28/0 | hw_port0_tsf_sync_sel, init_edca_cfg_8822b | hal/hal_com.c:14279, hal/hal_com.c:14306, hal/hal_com.c:14316 … |
| 513 | `0x05B5` | REG_NOA_UNIT_SEL_8822B (+1: REG_NOA_UNIT_SEL) | MAC | D |  |  |  | 0/0/0/0/0 | 42/0 |  |  |
| 514 | `0x05B7` | REG_P2POFF_DIS_TXTIME_8822B (+1: REG_P2POFF_DIS_TXTIME) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 515 | `0x05B8` | REG_MBSSID_BCN_SPACE2_8822B (+1: REG_MBSSID_BCN_SPACE2) | MAC | A | Beacon/TSF | 16 | R W | 2/0/0/0/0 | 28/0 | cfg_bcn_space_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:361, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:363 |
| 516 | `0x05BA` | *(không có macro)* | MAC | A | Beacon/TSF | 16 | R W | 2/0/0/0/0 | 0/0 | cfg_bcn_space_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:366, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:368 |
| 517 | `0x05BC` | REG_MBSSID_BCN_SPACE3_8822B (+1: REG_MBSSID_BCN_SPACE3) | MAC | A | Beacon/TSF | 16 | R W | 2/0/0/0/0 | 28/0 | cfg_bcn_space_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:371, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:373 |
| 518 | `0x05BE` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 519 | `0x05C0` | REG_ACMHWCTRL_8822B (+1: REG_ACMHWCTRL) | MAC | A |  | 8 | W | 1/0/0/0/0 | 14/4 | hw_var_set_acm_ctrl | hal/rtl8822b/rtl8822b_ops.c:1712 |
| 520 | `0x05C1` | REG_ACMRSTCTRL_8822B (+1: REG_ACMRSTCTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 6/0 |  |  |
| 521 | `0x05C2` | REG_ACMAVG_8822B (+1: REG_ACMAVG) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 522 | `0x05C4` | REG_VO_ADMTIME_8822B (+1: REG_VO_ADMTIME) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 523 | `0x05C6` | REG_VI_ADMTIME_8822B (+1: REG_VI_ADMTIME) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 524 | `0x05C8` | REG_BE_ADMTIME_8822B (+1: REG_BE_ADMTIME) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 525 | `0x05CC` | REG_EDCA_RANDOM_GEN_8822B (+1: REG_EDCA_RANDOM_GEN) | MAC | D |  |  |  | 0/0/0/0/0 | 21/0 |  |  |
| 526 | `0x05CF` | REG_TXCMD_NOA_SEL_8822B (+1: REG_TXCMD_NOA_SEL) | MAC | D |  |  |  | 0/0/0/0/0 | 21/0 |  |  |
| 527 | `0x05E0` | REG_NOA_PARAM_8822B (+1: REG_NOA_PARAM) | MAC | D |  |  |  | 0/0/0/0/0 | 42/0 |  |  |
| 528 | `0x05F0` | REG_P2P_RST_8822B (+1: REG_P2P_RST) | MAC | D |  |  |  | 0/0/0/0/0 | 12/0 |  |  |
| 529 | `0x05F1` | REG_SCHEDULER_RST_8822B (+1: REG_SCHEDULER_RST) | MAC | D |  |  |  | 0/0/0/0/0 | 9/2 |  |  |
| 530 | `0x05F8` | REG_SCH_TXCMD_8822B (+1: REG_SCH_TXCMD) | MAC | D |  |  |  | 0/0/0/0/0 | 44/2 |  |  |
| 531 | `0x05FC` | REG_PAGE5_DUMMY_8822B (+1: REG_PAGE5_DUMMY) | MAC | A |  | 32 | R W | 2/0/0/0/0 | 0/0 | verify_io_88xx | hal/halmac/halmac_88xx/halmac_init_88xx.c:811, hal/halmac/halmac_88xx/halmac_init_88xx.c:813 |
| 532 | `0x0600` | REG_WMAC_CR_8822B (+1: REG_WMAC_CR) | MAC | D |  |  |  | 0/0/0/0/0 | 2/0 |  |  |
| 533 | `0x0601` | REG_WMAC_FWPKT_CR_8822B (+1: REG_WMAC_FWPKT_CR) | MAC | A | TX/RX DMA/Queue | 8 | R RMW-clr RMW-set | 3/0/0/0/0 | 22/1 | init_trx_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:477, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:479, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:491 |
| 534 | `0x0603` | REG_BWOPMODE_8822B (+1: REG_BWOPMODE) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 535 | `0x0604` | REG_TCR_8822B (+2: REG_TCR, R_0x604) | MAC | A | Power/PS | 8 | R | 1/0/0/0/0 | 58/1 | rtw_lps_state_chk | hal/hal_com.c:14728 |
| 536 | `0x0605` | *(không có macro)* | MAC | A | MAC config/WMAC | 8 | W | 1/0/0/0/1 | 0/0 | init_wmac_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:956 |
| 537 | `0x0606` | *(không có macro)* | MAC | A | MAC config/WMAC | 8 | W | 1/0/0/0/0 | 0/0 | init_wmac_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:955 |
| 538 | `0x0608` | REG_RCR_8822B (+2: REG_RCR, R_0x608) | MAC | A | MAC config/WMAC | 8/32 | R R-mask RMW-mask W | 9/2/1/1/1 | 67/10 | cfg_drv_info_8822b, efuse_PostUpdateAction, efuse_PreUpdateAction, hw_var_rcr_config … | core/efuse/rtw_efuse.c:970, core/efuse/rtw_efuse.c:976, core/efuse/rtw_efuse.c:999 … |
| 539 | `0x0609` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 540 | `0x060A` | *(không có macro)* | MAC | A | System/Clock/Reset | 8 | R W | 3/0/2/0/0 | 0/0 | rx_clk_gate_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1128, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1131, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1133 |
| 541 | `0x060C` | REG_RX_PKT_LIMIT_8822B (+1: REG_RX_PKT_LIMIT) | MAC | A | MAC config/WMAC | 8 | W | 1/0/0/0/1 | 7/0 | init_wmac_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:953 |
| 542 | `0x060D` | REG_RX_DLK_TIME_8822B (+1: REG_RX_DLK_TIME) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 543 | `0x060F` | REG_RX_DRVINFO_SZ_8822B (+2: R_0x60f, REG_RX_DRVINFO_SZ) | MAC | A | TX/RX DMA/Queue | 8 | W | 2/0/0/0/0 | 24/0 | cfg_drv_info_8822b, init_trx_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:119, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:503 |
| 544 | `0x0610` | REG_MACID_8822B (+1: REG_MACID) | MAC | A | MAC config/WMAC | 32 | R | 1/0/0/0/0 | 14/0 | get_mac_addr_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1207 |
| 545 | `0x0614` | *(không có macro)* | MAC | A | MAC config/WMAC | 16 | R | 1/0/0/0/0 | 0/0 | get_mac_addr_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1208 |
| 546 | `0x0618` | REG_BSSID_8822B (+1: REG_BSSID) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 547 | `0x0620` | REG_MAR_8822B (+1: REG_MAR) | MAC | A | Init table | 8/32 | W | 1/0/0/0/1 | 14/0 | cfg_multicast_addr_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:522 |
| 548 | `0x0621` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 549 | `0x0622` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 550 | `0x0623` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 551 | `0x0624` | *(không có macro)* | MAC | A | Init table | 8/16 | W | 1/0/0/0/1 | 0/0 | cfg_multicast_addr_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:523 |
| 552 | `0x0625` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 553 | `0x0626` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 554 | `0x0627` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 555 | `0x0628` | REG_MBIDCAMCFG_1_8822B (+1: REG_MBIDCAMCFG_1) | MAC | Z |  |  |  | 0/0/0/0/0 | 24/0 |  |  |
| 556 | `0x062C` | REG_MBIDCAMCFG_2_8822B (+1: REG_MBIDCAMCFG_2) | MAC | Z |  |  |  | 0/0/0/0/0 | 19/0 |  |  |
| 557 | `0x0630` | REG_WMAC_TCR_TSFT_OFS_8822B (+1: REG_WMAC_TCR_TSFT_OFS) | MAC | A | Power/PS | 8 | R | 1/0/0/0/0 | 14/0 | rtw_hal_wow_disable | hal/hal_com.c:12413 |
| 558 | `0x0631` | *(không có macro)* | MAC | A | Power/PS | 8 | R | 1/0/0/0/0 | 0/0 | rtw_hal_wow_disable | hal/hal_com.c:12414 |
| 559 | `0x0632` | REG_UDF_THSD_8822B (+1: REG_UDF_THSD) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 560 | `0x0633` | REG_ZLD_NUM_8822B (+1: REG_ZLD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 561 | `0x0634` | REG_STMP_THSD_8822B (+1: REG_STMP_THSD) | MAC | A | Power/PS | 8 | R | 1/0/0/0/0 | 14/0 | rtw_hal_wow_disable | hal/hal_com.c:12415 |
| 562 | `0x0635` | REG_WMAC_TXTIMEOUT_8822B (+1: REG_WMAC_TXTIMEOUT) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 563 | `0x0636` | REG_MCU_TEST_2_V1_8822B (+1: REG_MCU_TEST_2_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 564 | `0x0638` | REG_USTIME_EDCA_8822B (+1: REG_USTIME_EDCA) | MAC | A | MAC config/WMAC | 8 | W | 1/0/0/0/1 | 14/0 | cfg_mac_clk_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:764 |
| 565 | `0x063A` | REG_MAC_SPEC_SIFS_8822B (+1: REG_MAC_SPEC_SIFS) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 566 | `0x063C` | REG_RESP_SIFS_CCK_8822B (+1: REG_RESP_SIFS_CCK) | MAC | A | Init table | 8 | W | 1/0/0/0/1 | 28/0 | rtl8822b_sethwreg | hal/rtl8822b/rtl8822b_ops.c:2077 |
| 567 | `0x063D` | *(không có macro)* | MAC | A | Init table | 8 | W | 1/0/0/0/1 | 0/0 | rtl8822b_sethwreg | hal/rtl8822b/rtl8822b_ops.c:2078 |
| 568 | `0x063E` | REG_RESP_SIFS_OFDM_8822B (+1: REG_RESP_SIFS_OFDM) | MAC | A | Init table | 8 | W | 1/0/0/0/1 | 28/0 | rtl8822b_sethwreg | hal/rtl8822b/rtl8822b_ops.c:2080 |
| 569 | `0x063F` | *(không có macro)* | MAC | A | Init table | 8 | W | 1/0/0/0/1 | 0/0 | rtl8822b_sethwreg | hal/rtl8822b/rtl8822b_ops.c:2081 |
| 570 | `0x0640` | REG_ACKTO_8822B (+1: REG_ACKTO) | MAC | A | Init table | 8 | R W | 0/0/2/0/1 | 14/0 |  | bảng: array_mp_8822b_mac_reg |
| 571 | `0x0641` | REG_CTS2TO_8822B (+1: REG_CTS2TO) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 572 | `0x0642` | REG_EIFS_8822B (+1: REG_EIFS) | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 14/0 |  | bảng: array_mp_8822b_mac_reg |
| 573 | `0x0643` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 574 | `0x0650` | REG_NAV_CTRL_8822B (+1: REG_NAV_CTRL) | MAC | R |  |  |  | 0/0/0/0/0 | 42/0 |  |  |
| 575 | `0x0652` | *(không có macro)* | MAC | A | Init table | 8 | W | 1/0/0/0/1 | 0/0 | rtl8822b_sethwreg | hal/rtl8822b/rtl8822b_ops.c:2345 |
| 576 | `0x0654` | REG_BACAMCMD_8822B (+1: REG_BACAMCMD) | MAC | D |  |  |  | 0/0/0/0/0 | 48/0 |  |  |
| 577 | `0x0658` | REG_BACAMCONTENT_8822B (+1: REG_BACAMCONTENT) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 578 | `0x0660` | REG_LBDLY_8822B (+2: R_0x660, REG_LBDLY) | MAC | A | Calibration | 32 | R-mask | 1/0/0/0/0 | 7/0 | _iqk_bb_reset_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:280 |
| 579 | `0x0661` | REG_WMAC_BACAM_RPMEN_8822B (+1: REG_WMAC_BACAM_RPMEN) | MAC | D |  |  |  | 0/0/0/0/0 | 10/0 |  |  |
| 580 | `0x0662` | REG_TX_RX_8822B (+1: REG_TX_RX) | MAC | D |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 581 | `0x0663` | REG_WMAC_BITMAP_CTL_8822B (+1: REG_WMAC_BITMAP_CTL) | MAC | D |  |  |  | 0/0/0/0/0 | 26/0 |  |  |
| 582 | `0x0664` | REG_RXERR_RPT_8822B (+1: REG_RXERR_RPT) | MAC | A | System/Clock/Reset | 16/32 | R R-mask RMW-mask W | 3/2/31/2/0 | 70/6 | _rx_report, rtw_reset_mac_rx_counters | hal/hal_com.c:16897, hal/hal_com.c:16898, hal/rtl8822b/rtl8822b_mac.c:289 |
| 583 | `0x0667` | *(không có macro)* | MAC | A |  | 8 | R W | 1/0/0/2/0 | 0/0 | _rx_report | hal/rtl8822b/rtl8822b_mac.c:286 |
| 584 | `0x0668` | REG_WMAC_TRXPTCL_CTL_8822B (+2: R_0x668, REG_WMAC_TRXPTCL_CTL) | MAC | A | Channel/BW | 32 | R W | 2/0/0/0/0 | 158/0 | cfg_bw_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:628, hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:646 |
| 585 | `0x066A` | *(không có macro)* | MAC | A |  | 8 | R W | 2/0/0/0/0 | 0/0 | hw_var_set_ack_preamble | hal/rtl8822b/rtl8822b_ops.c:1751, hal/rtl8822b/rtl8822b_ops.c:1759 |
| 586 | `0x066C` | *(không có macro)* | MAC | A | MAC config/WMAC | 8 | RMW-set | 1/0/0/0/0 | 0/0 | init_wmac_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:958 |
| 587 | `0x066E` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 588 | `0x0670` | REG_CAMCMD_8822B (+1: REG_CAMCMD) | MAC | A | Security/CAM | 32 | R W | 5/0/0/4/0 | 22/2 | rtl8822b_sethwreg, rtw_sec_read_cam, rtw_sec_write_cam | hal/hal_com.c:2517, hal/hal_com.c:2527, hal/hal_com.c:2560 … |
| 589 | `0x0674` | REG_CAMWRITE_8822B (+1: REG_CAMWRITE) | MAC | A | Security/CAM | 32 | W | 1/0/0/3/0 | 14/0 | rtw_sec_write_cam | hal/hal_com.c:2559 |
| 590 | `0x0678` | REG_CAMREAD_8822B (+1: REG_CAMREAD) | MAC | A | Security/CAM | 32 | R | 1/0/0/1/0 | 14/0 | rtw_sec_read_cam | hal/hal_com.c:2537 |
| 591 | `0x067C` | REG_CAMDBG_8822B (+1: REG_CAMDBG) | MAC | D |  |  |  | 0/0/0/0/0 | 48/0 |  |  |
| 592 | `0x0680` | REG_SECCFG_8822B (+1: REG_SECCFG) | MAC | A | Security/CAM | 8/16/32 | R W | 9/0/1/2/0 | 30/8 | SetHwReg, hw_var_set_sec_dk_cfg, rtw_hal_fw_sync_cam_id, rtw_hal_update_gtk_offload_info | hal/hal_com.c:14899, hal/hal_com.c:14920, hal/hal_com.c:14925 … |
| 593 | `0x0681` | *(không có macro)* | MAC | X | Security/CAM | 8 | RMW-set | 0/0/0/1/0 | 0/0 | config_security_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:833 |
| 594 | `0x0682` | REG_RXFILTER_CATEGORY_1_8822B (+1: REG_RXFILTER_CATEGORY_1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 595 | `0x0683` | REG_RXFILTER_ACTION_1_8822B (+1: REG_RXFILTER_ACTION_1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 596 | `0x0684` | REG_RXFILTER_CATEGORY_2_8822B (+1: REG_RXFILTER_CATEGORY_2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 597 | `0x0685` | REG_RXFILTER_ACTION_2_8822B (+1: REG_RXFILTER_ACTION_2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 598 | `0x0686` | REG_RXFILTER_CATEGORY_3_8822B (+1: REG_RXFILTER_CATEGORY_3) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 599 | `0x0687` | REG_RXFILTER_ACTION_3_8822B (+1: REG_RXFILTER_ACTION_3) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 600 | `0x0688` | REG_RXFLTMAP3_8822B (+2: R_0x688, REG_RXFLTMAP3) | MAC | A | Beamforming/MU | 32 | RMW-mask | 1/0/0/0/0 | 32/0 | _config_beamformer_mu | hal/rtl8822b/rtl8822b_phy.c:1732 |
| 601 | `0x068A` | REG_RXFLTMAP4_8822B (+1: REG_RXFLTMAP4) | MAC | A | Beamforming/MU | 8 | RMW-clr | 1/0/0/0/0 | 32/0 | cfg_sounding_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:296 |
| 602 | `0x068C` | REG_RXFLTMAP5_8822B (+1: REG_RXFLTMAP5) | MAC | D |  |  |  | 0/0/0/0/0 | 32/0 |  |  |
| 603 | `0x068E` | REG_RXFLTMAP6_8822B (+1: REG_RXFLTMAP6) | MAC | D |  |  |  | 0/0/0/0/0 | 32/0 |  |  |
| 604 | `0x0690` | REG_WOW_CTRL_8822B (+1: REG_WOW_CTRL) | MAC | A | TX/RX DMA/Queue | 8 | R W | 2/0/0/3/0 | 26/0 | rtw_hal_force_enable_rxdma | hal/hal_com.c:4999, hal/hal_com.c:5000 |
| 605 | `0x0691` | REG_NAN_RX_TSF_FILTER_8822B (+1: REG_NAN_RX_TSF_FILTER) | MAC | A | Beacon/TSF | 8 | W | 1/0/0/0/0 | 6/2 | rtl8822b_rx_tsf_addr_filter_config | hal/rtl8822b/rtl8822b_mac.c:134 |
| 606 | `0x0692` | REG_PS_RX_INFO_8822B (+1: REG_PS_RX_INFO) | MAC | D |  |  |  | 0/0/0/0/0 | 40/0 |  |  |
| 607 | `0x0693` | REG_WMMPS_UAPSD_TID_8822B (+1: REG_WMMPS_UAPSD_TID) | MAC | A | BB DM (DIG/CCA/FA/CCX) | 8 | W | 1/0/0/0/0 | 61/0 | rtw_hal_update_uapsd_tid | hal/hal_com.c:14683 |
| 608 | `0x0694` | REG_LPNAV_CTRL_8822B (+1: REG_LPNAV_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 609 | `0x0698` | REG_WKFMCAM_CMD_8822B (+1: REG_WKFMCAM_CMD) | MAC | A | Power/PS | 32 | R W | 6/0/0/0/0 | 34/6 | _rtw_wow_pattern_clean_cam, _rtw_wow_pattern_read_cam, _rtw_wow_pattern_write_cam | hal/hal_com.c:11948, hal/hal_com.c:11956, hal/hal_com.c:12019 … |
| 610 | `0x069C` | REG_WKFMCAM_RWD_8822B (+1: REG_WKFMCAM_RWD) | MAC | A | Power/PS | 32 | R W | 2/0/0/0/0 | 14/0 | _rtw_wow_pattern_read_cam, _rtw_wow_pattern_write_cam | hal/hal_com.c:11966, hal/hal_com.c:12018 |
| 611 | `0x06A0` | REG_RXFLTMAP0_8822B (+2: R_0x6a0, REG_RXFLTMAP0) | MAC | A | eFuse | 8/16/32 | R-mask RMW-mask W | 5/2/0/0/1 | 16/1 | _config_beamformer_mu, efuse_PostUpdateAction, efuse_PreUpdateAction, init_wmac_cfg_8822b | core/efuse/rtw_efuse.c:1000, core/efuse/rtw_efuse.c:971, core/efuse/rtw_efuse.c:977 … |
| 612 | `0x06A1` | *(không có macro)* | MAC | A | Beamforming/MU | 8 | R W | 5/0/0/0/1 | 0/0 | _config_beamformee_mu, _config_beamformer_mu, efuse_PreUpdateAction | core/efuse/rtw_efuse.c:978, hal/rtl8822b/rtl8822b_phy.c:1720, hal/rtl8822b/rtl8822b_phy.c:1722 … |
| 613 | `0x06A2` | REG_RXFLTMAP1_8822B (+1: REG_RXFLTMAP1) | MAC | A | Beamforming/MU | 8/16 | R RMW-clr W | 12/1/0/0/1 | 16/3 | SetHwReg, _config_beamformer_mu, cfg_sounding_88xx, efuse_PreUpdateAction … | core/efuse/rtw_efuse.c:979, hal/hal_com.c:15065, hal/hal_com.c:15068 … |
| 614 | `0x06A3` | *(không có macro)* | MAC | A | eFuse | 8 | W | 1/0/0/0/1 | 0/0 | efuse_PreUpdateAction | core/efuse/rtw_efuse.c:980 |
| 615 | `0x06A4` | REG_RXFLTMAP2_8822B (+1: REG_RXFLTMAP2) | MAC | A | eFuse | 8/16/32 | R-mask RMW-mask W | 15/1/0/0/1 | 16/0 | efuse_PostUpdateAction, efuse_PreUpdateAction, hw_var_set_mlme_disconnect, hw_var_set_mlme_join … | core/efuse/rtw_efuse.c:1001, core/efuse/rtw_efuse.c:972, core/efuse/rtw_efuse.c:981 … |
| 616 | `0x06A5` | *(không có macro)* | MAC | A | eFuse | 8 | W | 1/0/0/0/1 | 0/0 | efuse_PreUpdateAction | core/efuse/rtw_efuse.c:982 |
| 617 | `0x06A8` | REG_BCN_PSR_RPT_8822B (+1: REG_BCN_PSR_RPT) | MAC | R |  |  |  | 0/0/0/0/0 | 39/6 |  |  |
| 618 | `0x06AC` | REG_FLC_RPC_8822B (+1: REG_FLC_RPC) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 619 | `0x06AD` | REG_FLC_RPCT_8822B (+1: REG_FLC_RPCT) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 620 | `0x06AE` | REG_FLC_PTS_8822B (+1: REG_FLC_PTS) | MAC | D |  |  |  | 0/0/0/0/0 | 6/0 |  |  |
| 621 | `0x06AF` | REG_FLC_TRPC_8822B (+1: REG_FLC_TRPC) | MAC | D |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 622 | `0x06B0` | REG_RXPKTMON_CTRL_8822B (+1: REG_RXPKTMON_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 70/0 |  |  |
| 623 | `0x06B4` | REG_STATE_MON_8822B (+1: REG_STATE_MON) | MAC | A | Calibration | 32 | W | 1/0/0/0/0 | 59/0 | _iqk_bb_reset_8822b | hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c:281 |
| 624 | `0x06B8` | REG_ERROR_MON_8822B (+1: REG_ERROR_MON) | MAC | D |  |  |  | 0/0/0/0/0 | 6/0 |  |  |
| 625 | `0x06BC` | REG_SEARCH_MACID_8822B (+1: REG_SEARCH_MACID) | MAC | D |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 626 | `0x06C0` | REG_BT_COEX_TABLE_8822B (+1: REG_BT_COEX_TABLE) | MAC | A |  | 32 | RMW-mask | 1/0/0/0/0 | 93/0 | ex_hal8822b_wifi_only_hw_config | hal/btc/halbtc8822bwifionly.c:81 |
| 627 | `0x06C4` | *(không có macro)* | MAC | A |  | 32 | RMW-mask | 1/0/0/0/0 | 0/0 | ex_hal8822b_wifi_only_hw_config | hal/btc/halbtc8822bwifionly.c:82 |
| 628 | `0x06D0` | REG_RXCMD_0_8822B (+1: REG_RXCMD_0) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 629 | `0x06D4` | REG_RXCMD_1_8822B (+1: REG_RXCMD_1) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 630 | `0x06D8` | REG_WMAC_RESP_TXINFO_8822B (+4: R_0x6d8, ODM_REG_RESP_TX_11N, …) | MAC | Z |  |  |  | 0/0/0/0/0 | 70/0 |  |  |
| 631 | `0x06D9` | *(không có macro)* | MAC | G |  | 16 | W | 0/0/1/0/0 | 0/0 | rtw_dbg_port | os_dep/linux/ioctl_linux.c:6215 |
| 632 | `0x06DC` | REG_BBPSF_CTRL_8822B (+2: R_0x6dc, REG_BBPSF_CTRL) | MAC | A | Beamforming/MU | 32 | R W | 6/0/0/0/0 | 52/5 | cfg_csi_rate_88xx, cfg_sounding_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:282, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:315, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:317 … |
| 633 | `0x06DE` | *(không có macro)* | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 0/0 |  | bảng: array_mp_8822b_mac_reg |
| 634 | `0x06DF` | *(không có macro)* | MAC | A | Beamforming/MU | 8 | R W | 2/0/0/0/0 | 0/0 | rtl8822b_phy_bf_init | hal/rtl8822b/rtl8822b_phy.c:2149, hal/rtl8822b/rtl8822b_phy.c:2151 |
| 635 | `0x06E0` | REG_P2P_RX_BCN_NOA_8822B (+1: REG_P2P_RX_BCN_NOA) | MAC | D |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 636 | `0x06E4` | REG_ASSOCIATED_BFMER0_INFO_8822B (+1: REG_ASSOCIATED_BFMER0_INFO) | MAC | A | Beamforming/MU | 32 | W | 3/0/0/3/0 | 21/0 | _reset_beamformer_su, mu_bfer_entry_del_88xx, mu_bfer_entry_init_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:512, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:640, hal/rtl8822b/rtl8822b_phy.c:1952 |
| 637 | `0x06E8` | *(không có macro)* | MAC | A | Beamforming/MU | 16/32 | W | 3/0/0/3/0 | 0/0 | _reset_beamformer_su, mu_bfer_entry_del_88xx, mu_bfer_entry_init_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:513, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:641, hal/rtl8822b/rtl8822b_phy.c:1953 |
| 638 | `0x06EA` | *(không có macro)* | MAC | A | Beamforming/MU | 16 | W | 1/0/0/1/0 | 0/0 | mu_bfer_entry_init_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:514 |
| 639 | `0x06EC` | REG_ASSOCIATED_BFMER1_INFO_8822B (+1: REG_ASSOCIATED_BFMER1_INFO) | MAC | A | Beamforming/MU | 32 | W | 1/0/0/3/0 | 21/0 | _reset_beamformer_su | hal/rtl8822b/rtl8822b_phy.c:1956 |
| 640 | `0x06F0` | *(không có macro)* | MAC | A | Beamforming/MU | 16/32 | W | 1/0/0/3/0 | 0/0 | _reset_beamformer_su | hal/rtl8822b/rtl8822b_phy.c:1957 |
| 641 | `0x06F2` | *(không có macro)* | MAC | X | Beamforming/MU | 16 | W | 0/0/0/1/0 | 0/0 | su_bfer_entry_init_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:450 |
| 642 | `0x06F4` | REG_TX_CSI_RPT_PARAM_BW20_8822B (+1: REG_TX_CSI_RPT_PARAM_BW20) | MAC | A | Beamforming/MU | 16 | W | 2/0/0/2/0 | 14/0 | _reset_beamformer_su, mu_bfer_entry_init_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:515, hal/rtl8822b/rtl8822b_phy.c:1954 |
| 643 | `0x06F6` | *(không có macro)* | MAC | A | Beamforming/MU | 16 | W | 1/0/0/2/0 | 0/0 | _reset_beamformer_su | hal/rtl8822b/rtl8822b_phy.c:1958 |
| 644 | `0x06F8` | REG_TX_CSI_RPT_PARAM_BW40_8822B (+1: REG_TX_CSI_RPT_PARAM_BW40) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 645 | `0x06FC` | REG_TX_CSI_RPT_PARAM_BW80_8822B (+1: REG_TX_CSI_RPT_PARAM_BW80) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 646 | `0x0700` | REG_MACID1_8822B (+2: R_0x700, REG_MACID1) | MAC | A | MAC config/WMAC | 32 | R | 1/0/0/0/0 | 14/0 | get_mac_addr_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1211 |
| 647 | `0x0704` | *(không có macro)* | MAC | A | MAC config/WMAC | 16 | R | 1/0/0/0/0 | 0/0 | get_mac_addr_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1212 |
| 648 | `0x0708` | REG_BSSID1_8822B (+1: REG_BSSID1) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 649 | `0x0710` | REG_BCN_PSR_RPT1_8822B (+1: REG_BCN_PSR_RPT1) | MAC | R |  |  |  | 0/0/0/0/0 | 46/6 |  |  |
| 650 | `0x0714` | REG_ASSOCIATED_BFMEE_SEL_8822B (+1: REG_ASSOCIATED_BFMEE_SEL) | MAC | A | Beamforming/MU | 16/32 | R W | 4/0/0/3/0 | 16/14 | _config_beamformee_su, _reset_beamformee_su | hal/rtl8822b/rtl8822b_phy.c:1777, hal/rtl8822b/rtl8822b_phy.c:1793, hal/rtl8822b/rtl8822b_phy.c:2000 … |
| 651 | `0x0716` | *(không có macro)* | MAC | X | Beamforming/MU | 16 | W | 0/0/0/2/0 | 0/0 | su_bfee_entry_del_88xx, su_bfee_entry_init_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:386, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:553 |
| 652 | `0x0718` | REG_SND_PTCL_CTRL_8822B (+1: REG_SND_PTCL_CTRL) | MAC | A | Beamforming/MU | 8/32 | R RMW-set W | 8/0/0/0/1 | 62/2 | _config_beamformee_mu, _config_beamformer_su, cfg_sounding_88xx, del_sounding_88xx … | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:960, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:293, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:342 … |
| 653 | `0x0719` | *(không có macro)* | MAC | A | Beamforming/MU | 8 | R W | 3/0/0/0/0 | 0/0 | _config_beamformee_mu, cfg_sounding_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:289, hal/rtl8822b/rtl8822b_phy.c:1925, hal/rtl8822b/rtl8822b_phy.c:1927 |
| 654 | `0x071A` | *(không có macro)* | MAC | A | Beamforming/MU | 8 | W | 1/0/0/0/0 | 0/0 | cfg_sounding_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:290 |
| 655 | `0x071B` | *(không có macro)* | MAC | A | Beamforming/MU | 8 | W | 2/0/0/0/0 | 0/0 | _config_beamformer_su, cfg_sounding_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:294, hal/rtl8822b/rtl8822b_phy.c:1670 |
| 656 | `0x071C` | REG_RX_CSI_RPT_INFO_8822B (+2: R_0x71c, REG_RX_CSI_RPT_INFO) | MAC | X | Beamforming/MU | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_set_direct_bfer | hal/phydm/phydm_direct_bf.c:309 |
| 657 | `0x0720` | REG_NS_ARP_CTRL_8822B (+1: REG_NS_ARP_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 48/0 |  |  |
| 658 | `0x0724` | REG_NS_ARP_INFO_8822B (+1: REG_NS_ARP_INFO) | MAC | D |  |  |  | 0/0/0/0/0 | 52/0 |  |  |
| 659 | `0x0728` | REG_BEAMFORMING_INFO_NSARP_V1_8822B (+1: REG_BEAMFORMING_INFO_NSARP_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 660 | `0x072C` | REG_BEAMFORMING_INFO_NSARP_8822B (+2: R_0x72c, REG_BEAMFORMING_INFO_NSARP) | MAC | X | Beamforming/MU | 32 | RMW-mask | 0/0/0/1/0 | 14/0 | phydm_set_direct_bfer | hal/phydm/phydm_direct_bf.c:308 |
| 661 | `0x0750` | REG_WMAC_RTX_CTX_SUBTYPE_CFG_8822B (+1: REG_WMAC_RTX_CTX_SUBTYPE_CFG) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 662 | `0x0760` | REG_WMAC_SWAES_CFG_8822B (+1: REG_WMAC_SWAES_CFG) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 663 | `0x0762` | REG_BT_COEX_V2_8822B (+1: REG_BT_COEX_V2) | MAC | D |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 664 | `0x0764` | REG_BT_COEX_8822B (+2: R_0x764, REG_BT_COEX) | MAC | D |  |  |  | 0/0/0/0/0 | 24/0 |  |  |
| 665 | `0x0768` | REG_WLAN_ACT_MASK_CTRL_8822B (+1: REG_WLAN_ACT_MASK_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 42/0 |  |  |
| 666 | `0x076E` | REG_BT_COEX_ENHANCED_INTR_CTRL_8822B (+1: REG_BT_COEX_ENHANCED_INTR_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 50/0 |  |  |
| 667 | `0x0770` | REG_BT_ACT_STATISTICS_8822B (+1: REG_BT_ACT_STATISTICS) | MAC | D |  |  |  | 0/0/0/0/0 | 56/0 |  |  |
| 668 | `0x0778` | REG_BT_STATISTICS_CONTROL_REGISTER_8822B (+1: REG_BT_STATISTICS_CONTROL_REGISTER) | MAC | X |  | 32 | RMW-mask | 0/0/0/1/0 | 54/0 | mpt_InitHWConfig | core/rtw_mp.c:366 |
| 669 | `0x077C` | REG_BT_STATUS_REPORT_REGISTER_8822B (+1: REG_BT_STATUS_REPORT_REGISTER) | MAC | D |  |  |  | 0/0/0/0/0 | 56/0 |  |  |
| 670 | `0x0780` | REG_BT_INTERRUPT_CONTROL_REGISTER_8822B (+1: REG_BT_INTERRUPT_CONTROL_REGISTER) | MAC | D |  |  |  | 0/0/0/0/0 | 58/0 |  |  |
| 671 | `0x0784` | REG_WLAN_REPORT_TIME_OUT_CONTROL_REGISTER_8822B (+1: REG_WLAN_REPORT_TIME_OUT_CONTROL_REGISTER) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 672 | `0x0785` | REG_BT_ISOLATION_TABLE_REGISTER_REGISTER_8822B (+1: REG_BT_ISOLATION_TABLE_REGISTER_REGISTER) | MAC | D |  |  |  | 0/0/0/0/0 | 23/0 |  |  |
| 673 | `0x078F` | REG_BT_INTERRUPT_STATUS_REGISTER_8822B (+1: REG_BT_INTERRUPT_STATUS_REGISTER) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 674 | `0x0790` | REG_BT_TDMA_TIME_REGISTER_8822B (+1: REG_BT_TDMA_TIME_REGISTER) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 675 | `0x0794` | REG_BT_ACT_REGISTER_8822B (+1: REG_BT_ACT_REGISTER) | MAC | D |  |  |  | 0/0/0/0/0 | 41/0 |  |  |
| 676 | `0x0798` | REG_OBFF_CTRL_BASIC_8822B (+1: REG_OBFF_CTRL_BASIC) | MAC | D |  |  |  | 0/0/0/0/0 | 88/0 |  |  |
| 677 | `0x079C` | REG_OBFF_CTRL2_TIMER_8822B (+1: REG_OBFF_CTRL2_TIMER) | MAC | D |  |  |  | 0/0/0/0/0 | 56/0 |  |  |
| 678 | `0x07A0` | REG_LTR_CTRL_BASIC_8822B (+1: REG_LTR_CTRL_BASIC) | MAC | D |  |  |  | 0/0/0/0/0 | 90/0 |  |  |
| 679 | `0x07A4` | REG_LTR_CTRL2_TIMER_THRESHOLD_8822B (+1: REG_LTR_CTRL2_TIMER_THRESHOLD) | MAC | D |  |  |  | 0/0/0/0/0 | 98/0 |  |  |
| 680 | `0x07A8` | REG_LTR_IDLE_LATENCY_V1_8822B (+1: REG_LTR_IDLE_LATENCY_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 681 | `0x07AC` | REG_LTR_ACTIVE_LATENCY_V1_8822B (+1: REG_LTR_ACTIVE_LATENCY_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 682 | `0x07B0` | REG_ANTENNA_TRAINING_CONTROL_REGISTER_8822B (+3: R_0x7b0, ODM_REG_ANT_TRAIN_PARA1_11N, …) | MAC | Z |  |  |  | 0/0/0/0/0 | 20/0 |  |  |
| 683 | `0x07B4` | R_0x7b4 (+1: ODM_REG_ANT_TRAIN_PARA2_11N) | MAC | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 684 | `0x07B8` | REG_WMAC_PKTCNT_RWD_8822B (+1: REG_WMAC_PKTCNT_RWD) | MAC | D |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 685 | `0x07BC` | REG_WMAC_PKTCNT_CTRL_8822B (+1: REG_WMAC_PKTCNT_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 686 | `0x07C0` | REG_IQ_DUMP_8822B (+3: R_0x7c0, REG_IQ_DUMP, …) | MAC | A |  | 8/32 | R RMW-mask | 3/0/0/0/0 | 119/0 | phydm_la_adc_smp_start, phydm_la_get_tx_pkt_buf, phydm_la_set_mac_trigger_time | hal/phydm/phydm_adc_sampling.c:1165, hal/phydm/phydm_adc_sampling.c:1288, hal/phydm/phydm_adc_sampling.c:852 |
| 687 | `0x07C4` | R_0x7c4 (+1: REG_LTECOEX_WRITE_DATA) | MAC | R |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 688 | `0x07C8` | R_0x7c8 (+1: REG_LTECOEX_READ_DATA) | MAC | R |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 689 | `0x07CC` | REG_WMAC_FTM_CTL_8822B (+2: R_0x7cc, REG_WMAC_FTM_CTL) | MAC | A |  | 32 | RMW-mask | 1/0/0/2/0 | 12/0 | phydm_la_set_mac_trigger_time | hal/phydm/phydm_adc_sampling.c:1164 |
| 690 | `0x07CE` | REG_WMAC_IQ_MDPK_FUNC_8822B (+1: REG_WMAC_IQ_MDPK_FUNC) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 691 | `0x07D0` | REG_WMAC_OPTION_FUNCTION_8822B (+1: REG_WMAC_OPTION_FUNCTION) | MAC | R |  |  |  | 0/0/0/0/0 | 52/0 |  |  |
| 692 | `0x07D4` | *(không có macro)* | MAC | A | MAC config/WMAC | 8/32 | R W | 3/0/0/0/1 | 0/0 | cfg_drv_info_8822b, init_wmac_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:135, hal/halmac/halmac_88xx/halmac_8822b/halmac_cfg_wmac_8822b.c:141, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:969 |
| 693 | `0x07D8` | *(không có macro)* | MAC | A | MAC config/WMAC | 32 | W | 1/0/0/0/0 | 0/0 | init_wmac_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:962 |
| 694 | `0x07DA` | REG_RX_FILTER_FUNCTION_8822B (+1: REG_RX_FILTER_FUNCTION) | MAC | A | Init table | 8 |  | 0/0/0/0/1 | 30/0 |  | bảng: array_mp_8822b_mac_reg |
| 695 | `0x07E0` | REG_NDP_SIG_8822B (+1: REG_NDP_SIG) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 696 | `0x07E4` | REG_TXCMD_INFO_FOR_RSP_PKT_8822B (+1: REG_TXCMD_INFO_FOR_RSP_PKT) | MAC | D |  |  |  | 0/0/0/0/0 | 42/0 |  |  |
| 697 | `0x07F0` | REG_RTS_ADDRESS_0_8822B (+2: R_0x7f0, REG_RTS_ADDRESS_0) | MAC | X |  | 8/32 | R RMW-mask | 0/0/0/3/0 | 0/0 | phydm_la_adc_smp_start, phydm_la_get_tx_pkt_buf, phydm_la_set_mac_trigger_time | hal/phydm/phydm_adc_sampling.c:1157, hal/phydm/phydm_adc_sampling.c:1286, hal/phydm/phydm_adc_sampling.c:847 |
| 698 | `0x07F4` | R_0x7f4 | MAC | R |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 699 | `0x07F8` | REG_RTS_ADDRESS_1_8822B (+2: R_0x7f8, REG_RTS_ADDRESS_1) | MAC | R |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 700 | `0x07FC` | R_0x7fc | MAC | X |  | 32 | RMW-mask | 0/0/0/1/0 | 0/0 | phydm_la_set_mac_trigger_time | hal/phydm/phydm_adc_sampling.c:1156 |
| 701 | `0x07FE` | REG__RPFM_MAP1_8822B (+1: REG__RPFM_MAP1) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 702 | `0x1000` | REG_SYS_CFG3_8822B (+1: REG_SYS_CFG3) | MAC | D |  |  |  | 0/0/0/0/0 | 13/0 |  |  |
| 703 | `0x1034` | REG_SYS_CFG4_8822B (+1: REG_SYS_CFG4) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 704 | `0x1038` | R_0x1038 | MAC | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 705 | `0x103C` | R_0x103c | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 706 | `0x1040` | R_0x1040 | MAC | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 707 | `0x1048` | R_0x1048 | MAC | Z |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 708 | `0x1070` | REG_SYS_CFG5_8822B (+1: REG_SYS_CFG5) | MAC | A |  | 8 | R | 1/0/0/0/0 | 8/0 | rtl8822b_gethwreg | hal/rtl8822b/rtl8822b_ops.c:2813 |
| 709 | `0x1080` | REG_CPU_DMEM_CON_8822B (+2: R_0x1080, REG_CPU_DMEM_CON) | MAC | A | System/Clock/Reset | 32 | R W | 2/0/0/0/0 | 24/1 | init_system_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:767, hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:768 |
| 710 | `0x1082` | *(không có macro)* | MAC | A | System/Clock/Reset | 8 | R W | 4/0/0/0/0 | 0/0 | pltfm_reset_88xx | hal/halmac/halmac_88xx/halmac_fw_88xx.c:432, hal/halmac/halmac_88xx/halmac_fw_88xx.c:433, hal/halmac/halmac_88xx/halmac_fw_88xx.c:442 … |
| 711 | `0x1088` | REG_BOOT_REASON_8822B (+1: REG_BOOT_REASON) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 712 | `0x10A8` | REG_NFCPAD_CTRL_8822B (+1: REG_NFCPAD_CTRL) | MAC | A | Power/PS |  |  | 0/0/0/0/1 | 56/0 |  | bảng: TRANS_CARDEMU_TO_ACT_8822B |
| 713 | `0x10A9` | *(không có macro)* | MAC | A | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDEMU_TO_ACT_8822B |
| 714 | `0x10AA` | *(không có macro)* | MAC | A | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDEMU_TO_ACT_8822B |
| 715 | `0x10B0` | REG_HIMR2_8822B (+1: REG_HIMR2) | MAC | D |  |  |  | 0/0/0/0/0 | 75/0 |  |  |
| 716 | `0x10B4` | REG_HISR2_8822B (+1: REG_HISR2) | MAC | D |  |  |  | 0/0/0/0/0 | 25/0 |  |  |
| 717 | `0x10B8` | REG_HIMR3_8822B (+1: REG_HIMR3) | MAC | D |  |  |  | 0/0/0/0/0 | 48/0 |  |  |
| 718 | `0x10BC` | REG_HISR3_8822B (+1: REG_HISR3) | MAC | D |  |  |  | 0/0/0/0/0 | 23/0 |  |  |
| 719 | `0x10C0` | REG_SW_MDIO_8822B (+1: REG_SW_MDIO) | MAC | R |  |  |  | 0/0/0/0/0 | 2/0 |  |  |
| 720 | `0x10C3` | *(không có macro)* | MAC | A | Power/PS | 8 | R RMW-clr | 2/0/0/0/2 | 0/0 | mac_pwr_switch_usb_8822b, set_usb_mode_88xx | hal/halmac/halmac_88xx/halmac_8822b/halmac_usb_8822b.c:131, hal/halmac/halmac_88xx/halmac_usb_88xx.c:490 |
| 721 | `0x10C4` | REG_SW_FLUSH_8822B (+1: REG_SW_FLUSH) | MAC | D |  |  |  | 0/0/0/0/0 | 34/0 |  |  |
| 722 | `0x10D0` | REG_H2C_PKT_READADDR_8822B (+1: REG_H2C_PKT_READADDR) | MAC | A | Firmware/H2C/C2H | 32 | R | 1/0/1/0/0 | 14/0 | get_h2c_buf_free_space_88xx | hal/halmac/halmac_88xx/halmac_common_88xx.c:755 |
| 723 | `0x10D4` | REG_H2C_PKT_WRITEADDR_8822B (+1: REG_H2C_PKT_WRITEADDR) | MAC | A | Firmware/H2C/C2H | 32 | R | 1/0/0/0/0 | 14/0 | get_h2c_buf_free_space_88xx | hal/halmac/halmac_88xx/halmac_common_88xx.c:754 |
| 724 | `0x10D8` | REG_MEM_PWR_CRTL_8822B (+1: REG_MEM_PWR_CRTL) | MAC | D |  |  |  | 0/0/0/0/0 | 26/0 |  |  |
| 725 | `0x10E0` | REG_FW_DBG0_8822B (+1: REG_FW_DBG0) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 726 | `0x10E4` | REG_FW_DBG1_8822B (+1: REG_FW_DBG1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 727 | `0x10E8` | REG_FW_DBG2_8822B (+1: REG_FW_DBG2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 728 | `0x10EC` | REG_FW_DBG3_8822B (+1: REG_FW_DBG3) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 729 | `0x10F0` | REG_FW_DBG4_8822B (+1: REG_FW_DBG4) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 730 | `0x10F4` | REG_FW_DBG5_8822B (+1: REG_FW_DBG5) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 731 | `0x10F8` | REG_FW_DBG6_8822B (+1: REG_FW_DBG6) | MAC | A | TX/RX DMA/Queue | 32 | R | 1/0/0/2/0 | 14/0 | rtw_halmac_txfifo_wait_empty | hal/hal_halmac.c:3822 |
| 732 | `0x10FC` | REG_FW_DBG7_8822B (+1: REG_FW_DBG7) | MAC | A | Firmware/H2C/C2H | 32 | R | 2/0/0/3/0 | 14/0 | dlfw_end_flow_88xx, rtw_halmac_txfifo_wait_empty | hal/hal_halmac.c:3822, hal/halmac/halmac_88xx/halmac_fw_88xx.c:712 |
| 733 | `0x1100` | REG_CR_EXT_8822B (+2: MSR1, REG_CR_EXT) | MAC | A |  | 8/32 | R W | 9/0/0/0/0 | 58/9 | cfg_net_type_88xx, rtw_halmac_get_network_type | hal/hal_halmac.c:1989, hal/hal_halmac.c:1994, hal/hal_halmac.c:1999 … |
| 734 | `0x1114` | REG_FWFF_8822B (+1: REG_FWFF) | MAC | D |  |  |  | 0/0/0/0/0 | 42/0 |  |  |
| 735 | `0x1118` | REG_RXFF_PTR_V1_8822B (+1: REG_RXFF_PTR_V1) | MAC | A |  | 16 | R | 1/0/0/0/0 | 0/0 | check_rx_count | hal/rtl8822b/rtl8822b_ops.c:948 |
| 736 | `0x111C` | REG_RXFF_WTR_V1_8822B (+1: REG_RXFF_WTR_V1) | MAC | R |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 737 | `0x1120` | REG_FE2IMR_8822B (+1: REG_FE2IMR) | MAC | D |  |  |  | 0/0/0/0/0 | 52/0 |  |  |
| 738 | `0x1124` | REG_FE2ISR_8822B (+1: REG_FE2ISR) | MAC | D |  |  |  | 0/0/0/0/0 | 52/0 |  |  |
| 739 | `0x1128` | REG_FE3IMR_8822B (+1: REG_FE3IMR) | MAC | D |  |  |  | 0/0/0/0/0 | 58/0 |  |  |
| 740 | `0x112C` | REG_FE3ISR_8822B (+1: REG_FE3ISR) | MAC | D |  |  |  | 0/0/0/0/0 | 58/0 |  |  |
| 741 | `0x1130` | REG_FE4IMR_8822B (+1: REG_FE4IMR) | MAC | D |  |  |  | 0/0/0/0/0 | 40/0 |  |  |
| 742 | `0x1134` | REG_FE4ISR_8822B (+1: REG_FE4ISR) | MAC | D |  |  |  | 0/0/0/0/0 | 40/0 |  |  |
| 743 | `0x1138` | REG_FT1IMR_8822B (+1: REG_FT1IMR) | MAC | D |  |  |  | 0/0/0/0/0 | 62/0 |  |  |
| 744 | `0x113C` | REG_FT1ISR_8822B (+1: REG_FT1ISR) | MAC | D |  |  |  | 0/0/0/0/0 | 62/0 |  |  |
| 745 | `0x1140` | REG_SPWR0_8822B (+1: REG_SPWR0) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 746 | `0x1144` | REG_SPWR1_8822B (+1: REG_SPWR1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 747 | `0x1148` | REG_SPWR2_8822B (+1: REG_SPWR2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 748 | `0x114C` | REG_SPWR3_8822B (+1: REG_SPWR3) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 749 | `0x1150` | REG_POWSEQ_8822B (+1: REG_POWSEQ) | MAC | D |  |  |  | 0/0/0/0/0 | 21/0 |  |  |
| 750 | `0x1158` | REG_TC7_CTRL_V1_8822B (+1: REG_TC7_CTRL_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 20/0 |  |  |
| 751 | `0x115C` | REG_TC8_CTRL_V1_8822B (+1: REG_TC8_CTRL_V1) | MAC | D |  |  |  | 0/0/0/0/0 | 20/0 |  |  |
| 752 | `0x11E0` | REG_FT2IMR_8822B (+1: REG_FT2IMR) | MAC | D |  |  |  | 0/0/0/0/0 | 52/0 |  |  |
| 753 | `0x11E4` | REG_FT2ISR_8822B (+1: REG_FT2ISR) | MAC | D |  |  |  | 0/0/0/0/0 | 52/0 |  |  |
| 754 | `0x11F0` | REG_MSG2_8822B (+1: REG_MSG2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 755 | `0x11F4` | REG_MSG3_8822B (+1: REG_MSG3) | MAC | A | TX/RX DMA/Queue | 32 | R | 1/0/0/0/0 | 14/0 | rtw_halmac_txfifo_wait_empty | hal/hal_halmac.c:3822 |
| 756 | `0x11F8` | REG_MSG4_8822B (+1: REG_MSG4) | MAC | A | TX/RX DMA/Queue | 32 | R | 1/0/0/0/0 | 14/0 | rtw_halmac_txfifo_wait_empty | hal/hal_halmac.c:3822 |
| 757 | `0x11FC` | REG_MSG5_8822B (+1: REG_MSG5) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 758 | `0x1200` | REG_DDMA_CH0SA_8822B (+1: REG_DDMA_CH0SA) | MAC | A | Firmware/H2C/C2H | 32 | W | 1/0/0/0/0 | 14/0 | iddma_en_88xx | hal/halmac/halmac_88xx/halmac_fw_88xx.c:831 |
| 759 | `0x1204` | REG_DDMA_CH0DA_8822B (+1: REG_DDMA_CH0DA) | MAC | A | Firmware/H2C/C2H | 32 | W | 1/0/0/0/0 | 14/0 | iddma_en_88xx | hal/halmac/halmac_88xx/halmac_fw_88xx.c:832 |
| 760 | `0x1208` | REG_DDMA_CH0CTRL_8822B (+1: REG_DDMA_CH0CTRL) | MAC | A | Firmware/H2C/C2H | 32 | R RMW-set W | 5/0/0/0/0 | 30/6 | check_fw_chksum_88xx, dlfw_to_mem_88xx, iddma_dlfw_88xx, iddma_en_88xx | hal/halmac/halmac_88xx/halmac_fw_88xx.c:623, hal/halmac/halmac_88xx/halmac_fw_88xx.c:804, hal/halmac/halmac_88xx/halmac_fw_88xx.c:833 … |
| 761 | `0x1210` | REG_DDMA_CH1SA_8822B (+1: REG_DDMA_CH1SA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 762 | `0x1214` | REG_DDMA_CH1DA_8822B (+1: REG_DDMA_CH1DA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 763 | `0x1218` | REG_DDMA_CH1CTRL_8822B (+1: REG_DDMA_CH1CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 764 | `0x1220` | REG_DDMA_CH2SA_8822B (+1: REG_DDMA_CH2SA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 765 | `0x1224` | REG_DDMA_CH2DA_8822B (+1: REG_DDMA_CH2DA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 766 | `0x1228` | REG_DDMA_CH2CTRL_8822B (+1: REG_DDMA_CH2CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 767 | `0x1230` | REG_DDMA_CH3SA_8822B (+1: REG_DDMA_CH3SA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 768 | `0x1234` | REG_DDMA_CH3DA_8822B (+1: REG_DDMA_CH3DA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 769 | `0x1238` | REG_DDMA_CH3CTRL_8822B (+1: REG_DDMA_CH3CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 770 | `0x1240` | REG_DDMA_CH4SA_8822B (+1: REG_DDMA_CH4SA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 771 | `0x1244` | REG_DDMA_CH4DA_8822B (+1: REG_DDMA_CH4DA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 772 | `0x1248` | REG_DDMA_CH4CTRL_8822B (+1: REG_DDMA_CH4CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 33/0 |  |  |
| 773 | `0x1250` | REG_DDMA_CH5SA_8822B (+1: REG_DDMA_CH5SA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 774 | `0x1254` | REG_DDMA_CH5DA_8822B (+1: REG_DDMA_CH5DA) | MAC | D |  |  |  | 0/0/0/0/0 | 26/0 |  |  |
| 775 | `0x1258` | REG_REG_DDMA_CH5CTRL_8822B (+1: REG_REG_DDMA_CH5CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 15/0 |  |  |
| 776 | `0x12E0` | REG_DDMA_INT_MSK_8822B (+1: REG_DDMA_INT_MSK) | MAC | D |  |  |  | 0/0/0/0/0 | 12/0 |  |  |
| 777 | `0x12E8` | REG_DDMA_CHSTATUS_8822B (+1: REG_DDMA_CHSTATUS) | MAC | D |  |  |  | 0/0/0/0/0 | 12/0 |  |  |
| 778 | `0x12F0` | REG_DDMA_CHKSUM_8822B (+1: REG_DDMA_CHKSUM) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 779 | `0x12FC` | REG_DDMA_MONITOR_8822B (+1: REG_DDMA_MONITOR) | MAC | D |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 780 | `0x1300` | REG_STC_INT_CS_8822B (+1: REG_STC_INT_CS) | MAC | D |  |  |  | 0/0/0/0/0 | 59/0 |  |  |
| 781 | `0x1304` | REG_ST_INT_CFG_8822B (+1: REG_ST_INT_CFG) | MAC | D |  |  |  | 0/0/0/0/0 | 15/0 |  |  |
| 782 | `0x1310` | REG_CMU_DLY_CTRL_8822B (+1: REG_CMU_DLY_CTRL) | MAC | D |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 783 | `0x1314` | REG_CMU_DLY_CFG_8822B (+1: REG_CMU_DLY_CFG) | MAC | D |  |  |  | 0/0/0/0/0 | 56/0 |  |  |
| 784 | `0x1320` | REG_H2CQ_TXBD_DESA_8822B (+1: REG_H2CQ_TXBD_DESA) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 785 | `0x1328` | REG_H2CQ_TXBD_NUM_8822B (+1: REG_H2CQ_TXBD_NUM) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 786 | `0x132C` | REG_H2CQ_TXBD_IDX_8822B (+1: REG_H2CQ_TXBD_IDX) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 787 | `0x1330` | REG_H2CQ_CSR_8822B (+1: REG_H2CQ_CSR) | MAC | A | TX/RX DMA/Queue | 32 | W | 2/0/0/0/0 | 8/0 | download_firmware_88xx, init_trx_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:493, hal/halmac/halmac_88xx/halmac_fw_88xx.c:208 |
| 788 | `0x1350` | REG_CHANGE_PCIE_SPEED_8822B (+1: REG_CHANGE_PCIE_SPEED) | MAC | D |  |  |  | 0/0/0/0/0 | 54/0 |  |  |
| 789 | `0x13F4` | REG_OLD_DEHANG_8822B (+1: REG_OLD_DEHANG) | MAC | D |  |  |  | 0/0/0/0/0 | 2/0 |  |  |
| 790 | `0x1400` | REG_Q0_Q1_INFO_8822B (+1: REG_Q0_Q1_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 28/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2527 |
| 791 | `0x1404` | REG_Q2_Q3_INFO_8822B (+1: REG_Q2_Q3_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 28/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2528 |
| 792 | `0x1408` | REG_Q4_Q5_INFO_8822B (+1: REG_Q4_Q5_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 28/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2529 |
| 793 | `0x140C` | REG_Q6_Q7_INFO_8822B (+1: REG_Q6_Q7_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 44/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2530 |
| 794 | `0x1410` | REG_MGQ_HIQ_INFO_8822B (+1: REG_MGQ_HIQ_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 28/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2531 |
| 795 | `0x1414` | REG_CMDQ_BCNQ_INFO_8822B (+1: REG_CMDQ_BCNQ_INFO) | MAC | G |  | 32 | R | 0/0/1/0/0 | 28/0 | dump_mac_qinfo | hal/rtl8822b/rtl8822b_ops.c:2532 |
| 796 | `0x1420` | REG_USEREG_SETTING_8822B (+1: REG_USEREG_SETTING) | MAC | D |  |  |  | 0/0/0/0/0 | 32/0 |  |  |
| 797 | `0x1424` | REG_AESIV_SETTING_8822B (+1: REG_AESIV_SETTING) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 798 | `0x1428` | REG_BF0_TIME_SETTING_8822B (+1: REG_BF0_TIME_SETTING) | MAC | D |  |  |  | 0/0/0/0/0 | 36/0 |  |  |
| 799 | `0x142C` | REG_BF1_TIME_SETTING_8822B (+1: REG_BF1_TIME_SETTING) | MAC | D |  |  |  | 0/0/0/0/0 | 36/0 |  |  |
| 800 | `0x1430` | REG_BF_TIMEOUT_EN_8822B (+1: REG_BF_TIMEOUT_EN) | MAC | D |  |  |  | 0/0/0/0/0 | 8/0 |  |  |
| 801 | `0x1434` | REG_MACID_RELEASE0_8822B (+1: REG_MACID_RELEASE0) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 802 | `0x1438` | REG_MACID_RELEASE1_8822B (+1: REG_MACID_RELEASE1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 803 | `0x143C` | REG_MACID_RELEASE2_8822B (+1: REG_MACID_RELEASE2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 804 | `0x1440` | REG_MACID_RELEASE3_8822B (+1: REG_MACID_RELEASE3) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 805 | `0x1444` | REG_MACID_RELEASE_SETTING_8822B (+1: REG_MACID_RELEASE_SETTING) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 806 | `0x1448` | REG_FAST_EDCA_VOVI_SETTING_8822B (+1: REG_FAST_EDCA_VOVI_SETTING) | MAC | A | Init table | 8 | W | 1/0/0/0/1 | 60/0 | init_protocol_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:815 |
| 807 | `0x144A` | *(không có macro)* | MAC | A | Init table | 8 | W | 1/0/0/0/1 | 0/0 | init_protocol_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:816 |
| 808 | `0x144C` | REG_FAST_EDCA_BEBK_SETTING_8822B (+1: REG_FAST_EDCA_BEBK_SETTING) | MAC | A | Init table | 8 | W | 1/0/0/0/1 | 60/0 | init_protocol_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:817 |
| 809 | `0x144E` | *(không có macro)* | MAC | A | Init table | 8 | W | 1/0/0/0/1 | 0/0 | init_protocol_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:818 |
| 810 | `0x1450` | REG_MACID_DROP0_8822B (+1: REG_MACID_DROP0) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 811 | `0x1454` | REG_MACID_DROP1_8822B (+1: REG_MACID_DROP1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 812 | `0x1458` | REG_MACID_DROP2_8822B (+1: REG_MACID_DROP2) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 813 | `0x145C` | REG_MACID_DROP3_8822B (+1: REG_MACID_DROP3) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 814 | `0x1460` | REG_R_MACID_RELEASE_SUCCESS_0_8822B (+1: REG_R_MACID_RELEASE_SUCCESS_0) | MAC | D |  |  |  | 0/0/0/0/0 | 7/0 |  |  |
| 815 | `0x1464` | REG_R_MACID_RELEASE_SUCCESS_1_8822B (+1: REG_R_MACID_RELEASE_SUCCESS_1) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 816 | `0x1468` | REG_R_MACID_RELEASE_SUCCESS_2_8822B (+1: REG_R_MACID_RELEASE_SUCCESS_2) | MAC | D |  |  |  | 0/0/0/0/0 | 7/0 |  |  |
| 817 | `0x146C` | REG_R_MACID_RELEASE_SUCCESS_3_8822B (+1: REG_R_MACID_RELEASE_SUCCESS_3) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 818 | `0x1470` | REG_MGG_FIFO_CRTL_8822B (+1: REG_MGG_FIFO_CRTL) | MAC | D |  |  |  | 0/0/0/0/0 | 80/0 |  |  |
| 819 | `0x1474` | REG_MGG_FIFO_INT_8822B (+1: REG_MGG_FIFO_INT) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 820 | `0x1478` | REG_MGG_FIFO_LIFETIME_8822B (+1: REG_MGG_FIFO_LIFETIME) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 821 | `0x147C` | REG_R_MACID_RELEASE_SUCCESS_CLEAR_OFFSET_8822B (+1: REG_R_MACID_RELEASE_SUCCESS_CLEAR_OFFSET) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 822 | `0x1480` | REG_SHCUT_SETTING_8822B (+1: REG_SHCUT_SETTING) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 823 | `0x1484` | REG_SHCUT_LLC_ETH_TYPE0_8822B (+1: REG_SHCUT_LLC_ETH_TYPE0) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 824 | `0x1488` | REG_SHCUT_LLC_ETH_TYPE1_8822B (+1: REG_SHCUT_LLC_ETH_TYPE1) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 825 | `0x148C` | REG_SHCUT_LLC_OUI0_8822B (+1: REG_SHCUT_LLC_OUI0) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 826 | `0x1490` | REG_SHCUT_LLC_OUI1_8822B (+1: REG_SHCUT_LLC_OUI1) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 827 | `0x1494` | REG_SHCUT_LLC_OUI2_8822B (+1: REG_SHCUT_LLC_OUI2) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 828 | `0x1498` | REG_SHCUT_LLC_OUI3_8822B (+1: REG_SHCUT_LLC_OUI3) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 829 | `0x14C0` | REG_MU_TX_CTL_8822B (+2: R_0x14c0, REG_MU_TX_CTL) | MAC | A | Beamforming/MU | 8/16/32 | R RMW-clr W | 22/0/0/6/0 | 16/7 | _config_beamformee_mu, _reset_beamformee_mu, _sounding_config_mu, _sounding_reset_all … | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:168, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:169, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:200 … |
| 830 | `0x14C1` | *(không có macro)* | MAC | A | Beamforming/MU | 8 | R W | 5/0/0/0/0 | 0/0 | cfg_mu_bfee_88xx, cfg_mu_bfer_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:172, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:174, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:179 … |
| 831 | `0x14C4` | REG_MU_STA_GID_VLD_8822B (+2: R_0x14c4, REG_MU_STA_GID_VLD) | MAC | A | Beamforming/MU | 32 | W | 7/0/0/1/0 | 7/0 | _config_beamformee_mu, _sounding_config_mu, _sounding_reset_all, cfg_mu_bfee_88xx … | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:175, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:180, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:252 … |
| 832 | `0x14C8` | REG_MU_STA_USER_POS_INFO_8822B (+2: R_0x14c8, REG_MU_STA_USER_POS_INFO) | MAC | A | Beamforming/MU | 32 | W | 3/0/0/1/0 | 0/0 | _config_beamformee_mu, cfg_mu_bfee_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:176, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:181, hal/rtl8822b/rtl8822b_phy.c:1883 |
| 833 | `0x14CC` | R_0x14cc | MAC | A | Beamforming/MU | 32 | W | 3/0/0/1/0 | 0/0 | _config_beamformee_mu, cfg_mu_bfee_88xx | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:177, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:182, hal/rtl8822b/rtl8822b_phy.c:1884 |
| 834 | `0x14D0` | REG_MU_TRX_DBG_CNT_8822B (+1: REG_MU_TRX_DBG_CNT) | MAC | D |  |  |  | 0/0/0/0/0 | 30/0 |  |  |
| 835 | `0x1500` | REG_CPUMGQ_TX_TIMER_8822B (+1: REG_CPUMGQ_TX_TIMER) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 836 | `0x1504` | REG_PS_TIMER_A_8822B (+1: REG_PS_TIMER_A) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 837 | `0x1508` | REG_PS_TIMER_B_8822B (+1: REG_PS_TIMER_B) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 838 | `0x150C` | REG_PS_TIMER_C_8822B (+1: REG_PS_TIMER_C) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 839 | `0x1510` | REG_PS_TIMER_ABC_CPUMGQ_TIMER_CRTL_8822B (+1: REG_PS_TIMER_ABC_CPUMGQ_TIMER_CRTL) | MAC | D |  |  |  | 0/0/0/0/0 | 66/0 |  |  |
| 840 | `0x1514` | REG_CPUMGQ_TX_TIMER_EARLY_8822B (+1: REG_CPUMGQ_TX_TIMER_EARLY) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 841 | `0x1515` | REG_PS_TIMER_A_EARLY_8822B (+1: REG_PS_TIMER_A_EARLY) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 842 | `0x1516` | REG_PS_TIMER_B_EARLY_8822B (+1: REG_PS_TIMER_B_EARLY) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 843 | `0x1517` | REG_PS_TIMER_C_EARLY_8822B (+1: REG_PS_TIMER_C_EARLY) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 844 | `0x1518` | REG_CPUMGQ_PARAMETER_8822B (+2: R_0x1518, REG_CPUMGQ_PARAMETER) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 845 | `0x1600` | REG_BCN_PSR_RPT2_8822B (+1: REG_BCN_PSR_RPT2) | MAC | R |  |  |  | 0/0/0/0/0 | 46/6 |  |  |
| 846 | `0x1604` | REG_BCN_PSR_RPT3_8822B (+1: REG_BCN_PSR_RPT3) | MAC | R |  |  |  | 0/0/0/0/0 | 46/6 |  |  |
| 847 | `0x1608` | REG_BCN_PSR_RPT4_8822B (+1: REG_BCN_PSR_RPT4) | MAC | R |  |  |  | 0/0/0/0/0 | 46/6 |  |  |
| 848 | `0x160C` | REG_A1_ADDR_MASK_8822B (+1: REG_A1_ADDR_MASK) | MAC | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 849 | `0x1620` | REG_MACID2_8822B (+1: REG_MACID2) | MAC | A | MAC config/WMAC | 32 | R | 1/0/0/0/0 | 14/0 | get_mac_addr_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1215 |
| 850 | `0x1624` | *(không có macro)* | MAC | A | MAC config/WMAC | 16 | R | 1/0/0/0/0 | 0/0 | get_mac_addr_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1216 |
| 851 | `0x1628` | REG_BSSID2_8822B (+1: REG_BSSID2) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 852 | `0x1630` | REG_MACID3_8822B (+1: REG_MACID3) | MAC | A | MAC config/WMAC | 32 | R | 1/0/0/0/0 | 14/0 | get_mac_addr_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1219 |
| 853 | `0x1634` | *(không có macro)* | MAC | A | MAC config/WMAC | 16 | R | 1/0/0/0/0 | 0/0 | get_mac_addr_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1220 |
| 854 | `0x1638` | REG_BSSID3_8822B (+1: REG_BSSID3) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 855 | `0x1640` | REG_MACID4_8822B (+1: REG_MACID4) | MAC | A | MAC config/WMAC | 32 | R | 1/0/0/0/0 | 14/0 | get_mac_addr_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1223 |
| 856 | `0x1644` | *(không có macro)* | MAC | A | MAC config/WMAC | 16 | R | 1/0/0/0/0 | 0/0 | get_mac_addr_88xx | hal/halmac/halmac_88xx/halmac_cfg_wmac_88xx.c:1224 |
| 857 | `0x1648` | REG_BSSID4_8822B (+1: REG_BSSID4) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 858 | `0x1650` | REG_NOA_REPORT_8822B (+1: REG_NOA_REPORT) | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 859 | `0x1660` | REG_PWRBIT_SETTING_8822B (+1: REG_PWRBIT_SETTING) | MAC | D |  |  |  | 0/0/0/0/0 | 16/0 |  |  |
| 860 | `0x1661` | REG_MACRX_HANG_TIMER_CONTROL_8822B (+1: REG_MACRX_HANG_TIMER_CONTROL) | MAC | D |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 861 | `0x167C` | REG_WMAC_MU_BF_OPTION_8822B (+1: REG_WMAC_MU_BF_OPTION) | MAC | A | Beamforming/MU | 8 | W | 1/0/0/0/0 | 18/4 | rtl8822b_phy_bf_init | hal/rtl8822b/rtl8822b_phy.c:2135 |
| 862 | `0x167E` | REG_WMAC_MU_ARB_8822B (+1: REG_WMAC_MU_ARB) | MAC | D |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 863 | `0x167F` | REG_WMAC_MU_OPTION_8822B (+1: REG_WMAC_MU_OPTION) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 864 | `0x1680` | REG_WMAC_MU_BF_CTL_8822B (+1: REG_WMAC_MU_BF_CTL) | MAC | A | Beamforming/MU | 16 | R W | 4/0/0/0/0 | 32/0 | mu_bfer_entry_del_88xx, mu_bfer_entry_init_88xx, rtl8822b_phy_bf_init | hal/halmac/halmac_88xx/halmac_mimo_88xx.c:517, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:519, hal/halmac/halmac_88xx/halmac_mimo_88xx.c:642 … |
| 865 | `0x1682` | REG_WMAC_MU_BFRPT_PARA_8822B (+1: REG_WMAC_MU_BFRPT_PARA) | MAC | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 866 | `0x1684` | REG_WMAC_ASSOCIATED_MU_BFMEE2_8822B (+2: R_0x1684, REG_WMAC_ASSOCIATED_MU_BFMEE2) | MAC | A | Beamforming/MU | 16/32 | R W | 4/0/0/3/0 | 18/5 | _config_beamformee_mu, _reset_beamformee_mu, rtl8822b_phy_bf_sounding_status | hal/rtl8822b/rtl8822b_phy.c:1893, hal/rtl8822b/rtl8822b_phy.c:1897, hal/rtl8822b/rtl8822b_phy.c:2044 … |
| 867 | `0x1686` | REG_WMAC_ASSOCIATED_MU_BFMEE3_8822B (+1: REG_WMAC_ASSOCIATED_MU_BFMEE3) | MAC | A | Beamforming/MU | 16 | R W | 3/0/0/3/0 | 18/5 | _config_beamformee_mu, _reset_beamformee_mu | hal/rtl8822b/rtl8822b_phy.c:1893, hal/rtl8822b/rtl8822b_phy.c:1897, hal/rtl8822b/rtl8822b_phy.c:2044 |
| 868 | `0x1688` | REG_WMAC_ASSOCIATED_MU_BFMEE4_8822B (+2: R_0x1688, REG_WMAC_ASSOCIATED_MU_BFMEE4) | MAC | A | Beamforming/MU | 16/32 | R W | 4/0/0/3/0 | 18/5 | _config_beamformee_mu, _reset_beamformee_mu, rtl8822b_phy_bf_sounding_status | hal/rtl8822b/rtl8822b_phy.c:1893, hal/rtl8822b/rtl8822b_phy.c:1897, hal/rtl8822b/rtl8822b_phy.c:2044 … |
| 869 | `0x168A` | REG_WMAC_ASSOCIATED_MU_BFMEE5_8822B (+1: REG_WMAC_ASSOCIATED_MU_BFMEE5) | MAC | A | Beamforming/MU | 16 | R W | 3/0/0/3/0 | 18/5 | _config_beamformee_mu, _reset_beamformee_mu | hal/rtl8822b/rtl8822b_phy.c:1893, hal/rtl8822b/rtl8822b_phy.c:1897, hal/rtl8822b/rtl8822b_phy.c:2044 |
| 870 | `0x168C` | REG_WMAC_ASSOCIATED_MU_BFMEE6_8822B (+2: R_0x168c, REG_WMAC_ASSOCIATED_MU_BFMEE6) | MAC | A | Beamforming/MU | 16/32 | R W | 4/0/0/0/0 | 18/1 | _config_beamformee_mu, _reset_beamformee_mu, rtl8822b_phy_bf_sounding_status | hal/rtl8822b/rtl8822b_phy.c:1893, hal/rtl8822b/rtl8822b_phy.c:1897, hal/rtl8822b/rtl8822b_phy.c:2044 … |
| 871 | `0x168E` | REG_WMAC_ASSOCIATED_MU_BFMEE7_8822B (+1: REG_WMAC_ASSOCIATED_MU_BFMEE7) | MAC | A | Beamforming/MU | 16 | R W | 3/0/0/0/0 | 18/1 | _config_beamformee_mu, _reset_beamformee_mu | hal/rtl8822b/rtl8822b_phy.c:1893, hal/rtl8822b/rtl8822b_phy.c:1897, hal/rtl8822b/rtl8822b_phy.c:2044 |
| 872 | `0x16A0` | REG_TRANSMIT_ADDRSS_0_8822B (+1: REG_TRANSMIT_ADDRSS_0) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 873 | `0x16A8` | REG_TRANSMIT_ADDRSS_1_8822B (+1: REG_TRANSMIT_ADDRSS_1) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 874 | `0x16B0` | REG_TRANSMIT_ADDRSS_2_8822B (+1: REG_TRANSMIT_ADDRSS_2) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 875 | `0x16B8` | REG_TRANSMIT_ADDRSS_3_8822B (+1: REG_TRANSMIT_ADDRSS_3) | MAC | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 876 | `0x16C0` | REG_TRANSMIT_ADDRSS_4_8822B (+1: REG_TRANSMIT_ADDRSS_4) | MAC | R |  |  |  | 0/0/0/0/0 | 18/0 |  |  |
| 877 | `0x1700` | REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL_V1_8822B (+2: R_0x1700, REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL_V1) | MAC | A |  | 32 | R-mask RMW-mask W | 4/2/0/0/0 | 34/0 | ex_hal8822b_wifi_only_hw_config, ltecoex_reg_read_88xx, ltecoex_reg_write_88xx, phy_set_rf_path_switch_8822b | hal/btc/halbtc8822bwifionly.c:79, hal/halmac/halmac_88xx/halmac_common_88xx.c:3488, hal/halmac/halmac_88xx/halmac_common_88xx.c:3520 … |
| 878 | `0x1703` | *(không có macro)* | MAC | A |  | 8 | R | 2/0/0/0/0 | 0/0 | ltecoex_reg_read_88xx, ltecoex_reg_write_88xx | hal/halmac/halmac_88xx/halmac_common_88xx.c:3479, hal/halmac/halmac_88xx/halmac_common_88xx.c:3510 |
| 879 | `0x1704` | REG_WL2LTECOEX_INDIRECT_ACCESS_WRITE_DATA_V1_8822B (+2: R_0x1704, REG_WL2LTECOEX_INDIRECT_ACCESS_WRITE_DATA_V1) | MAC | A |  | 32 | R-mask RMW-mask W | 3/2/0/0/0 | 14/0 | ex_hal8822b_wifi_only_hw_config, ltecoex_reg_write_88xx, phy_set_rf_path_switch_8822b | hal/btc/halbtc8822bwifionly.c:78, hal/halmac/halmac_88xx/halmac_common_88xx.c:3519, hal/phydm/halrf/rtl8822b/halrf_8822b.c:569 |
| 880 | `0x1708` | REG_WL2LTECOEX_INDIRECT_ACCESS_READ_DATA_V1_8822B (+1: REG_WL2LTECOEX_INDIRECT_ACCESS_READ_DATA_V1) | MAC | A |  | 32 | R | 1/0/0/0/0 | 14/0 | ltecoex_reg_read_88xx | hal/halmac/halmac_88xx/halmac_common_88xx.c:3489 |
| 881 | `0xFB00` | REG_RXPKTBUF_STARTADDR | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 882 | `0xFC00` | REG_TXPKTBUF_STARTADDR | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 883 | `0xFD00` | REG_SYS_CTRL | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 884 | `0xFD01` | REG_PONSTS_RPT1 | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 885 | `0xFD02` | REG_PONSTS_RPT2 | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 886 | `0xFD03` | REG_PONSTS_RPT3 | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 887 | `0xFD04` | REG_PONSTS_RPT4 | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 888 | `0xFD05` | REG_PONSTS_RPT5 | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 889 | `0xFD08` | REG_8051ERRFLAG | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 890 | `0xFD09` | REG_8051ERRFLAG_MASK | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 891 | `0xFD10` | REG_TXADDRH | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 892 | `0xFD11` | REG_RXADDRH | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 893 | `0xFD12` | REG_TXADDRH_EXT | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 894 | `0xFD48` | REG_U3_STATE | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 895 | `0xFD50` | REG_OUTDATA0 | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 896 | `0xFD54` | REG_OUTDATA1 | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 897 | `0xFD58` | REG_OUTRDY | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 898 | `0xFD60` | REG_INDATA0 | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 899 | `0xFD64` | REG_INDATA1 | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 900 | `0xFD68` | REG_INRDY | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 901 | `0xFD90` | REG_MCUERR_PCLSB | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 902 | `0xFD91` | REG_MCUERR_PCMSB | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 903 | `0xFD92` | REG_MCUERR_ACC | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 904 | `0xFD93` | REG_MCUERR_B | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 905 | `0xFD94` | REG_MCUERR_DPTRLSB | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 906 | `0xFD95` | REG_MCUERR_DPTRMSB | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 907 | `0xFD96` | REG_MCUERR_SP | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 908 | `0xFD97` | REG_MCUERR_IE | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 909 | `0xFD98` | REG_MCUERR_EIE | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 910 | `0xFD9F` | REG_VERA_SIM | MAC | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
