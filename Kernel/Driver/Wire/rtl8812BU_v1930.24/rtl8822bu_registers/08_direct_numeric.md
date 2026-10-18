# 08 — Truy cập thanh ghi bằng địa chỉ số (Lượt 4)

Nguồn: `work/access.json` (công cụ `tools/inventory.py`). Một truy cập được tính là "địa chỉ số" khi đối số địa chỉ là **literal** (`0x520`) hoặc macro **mang tên chính là địa chỉ** (`R_0x520`, `RF_0x18` trong `hal/phydm/phydm_regtable.h` — tên không mang ngữ nghĩa, tương đương số). [TOOL]

- Tổng: 2025 lời gọi (663 literal, 1362 R_0x/RF_0x).
- Trên đường sống (live) của RTL8822BU: **1203** (396 literal, 807 R_0x/RF_0x).
- Còn lại 822 nằm trong nhánh chip khác / cấu hình tắt / hàm không có nơi gọi.

Ý nghĩa: các địa chỉ này **không có tên chức năng** trong source → muốn biết chức năng phải dựa vào ngữ cảnh hàm, comment, hoặc tài liệu Realtek (không có trong repository). Cột "Tên tương ứng" cho biết địa chỉ đó có trùng với một macro có tên trong cùng không gian hay không (nếu có → có thể tra nghĩa).

| File | Số lời gọi live |
|---|---:|
| `hal/phydm/rtl8822b/phydm_hal_api8822b.c` | 380 |
| `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c` | 254 |
| `hal/phydm/rtl8822b/phydm_rtl8822b.c` | 66 |
| `core/rtw_mp.c` | 64 |
| `hal/phydm/phydm_api.c` | 57 |
| `hal/hal_mp.c` | 45 |
| `hal/phydm/halrf/halrf_kfree.c` | 44 |
| `hal/phydm/phydm_debug.c` | 40 |
| `os_dep/linux/ioctl_linux.c` | 29 |
| `hal/phydm/halrf/rtl8822b/halrf_8822b.c` | 26 |
| `hal/hal_com.c` | 21 |
| `hal/phydm/phydm_psd.c` | 19 |
| `hal/phydm/txbf/haltxbf8822b.c` | 19 |
| `hal/phydm/phydm_adc_sampling.c` | 13 |
| `hal/btc/halbtc8822bwifionly.c` | 12 |
| `hal/phydm/phydm_ccx.c` | 11 |
| `hal/phydm/phydm.c` | 10 |
| `hal/phydm/phydm_adaptivity.c` | 10 |
| `hal/phydm/phydm_direct_bf.c` | 10 |
| `hal/phydm/phydm_rainfo.c` | 10 |
| `hal/phydm/phydm_cfotracking.c` | 9 |
| `hal/phydm/phydm_dig.c` | 8 |
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c` | 6 |
| `hal/phydm/halrf/halrf.c` | 5 |
| `hal/phydm/phydm_noisemonitor.c` | 5 |
| `hal/phydm/phydm_soml.c` | 4 |
| `hal/rtl8822b/rtl8822b_ops.c` | 4 |
| `hal/rtl8822b/rtl8822b_phy.c` | 4 |
| `hal/halmac/halmac_88xx/halmac_mimo_88xx.c` | 3 |
| `core/efuse/rtw_efuse.c` | 2 |
| `core/rtw_rf.c` | 2 |
| `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c` | 2 |
| `hal/halmac/halmac_88xx/halmac_8822b/halmac_usb_8822b.c` | 2 |
| `hal/phydm/phydm_auto_dbg.c` | 2 |
| `core/rtw_pwrctrl.c` | 1 |
| `core/rtw_wlan_util.c` | 1 |
| `hal/phydm/halrf/halrf_powertracking_ce.c` | 1 |
| `hal/phydm/phydm_cck_pd.c` | 1 |
| `os_dep/linux/rtw_proc.c` | 1 |

## `hal/phydm/rtl8822b/phydm_hal_api8822b.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 92 | phydm_igi_toggle_8822b | odm_get_bb_reg | BB | `R_0xc50` | 0x7f | rA_IGI_Jaguar, ODM_REG_IGI_A_11N |
| 93 | phydm_igi_toggle_8822b | odm_set_bb_reg | BB | `R_0xc50` | 0x7f | rA_IGI_Jaguar, ODM_REG_IGI_A_11N |
| 94 | phydm_igi_toggle_8822b | odm_set_bb_reg | BB | `R_0xc50` | 0x7f | rA_IGI_Jaguar, ODM_REG_IGI_A_11N |
| 95 | phydm_igi_toggle_8822b | odm_set_bb_reg | BB | `R_0xe50` | 0x7f | rB_IGI_Jaguar, ODM_REG_IGI_B_11AC |
| 96 | phydm_igi_toggle_8822b | odm_set_bb_reg | BB | `R_0xe50` | 0x7f | rB_IGI_Jaguar, ODM_REG_IGI_B_11AC |
| 104 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 105 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 106 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xcb4` | MASKBYTE1 | rA_RFE_Inv_Jaguar |
| 107 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xeb4` | MASKBYTE1 | rB_RFE_Inv_Jaguar |
| 110 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 111 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 112 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xcb4` | MASKLWORD | rA_RFE_Inv_Jaguar |
| 113 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xeb4` | MASKLWORD | rB_RFE_Inv_Jaguar |
| 114 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xcb8` | BIT(5) | rA_RFE_Jaguar |
| 115 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xeb8` | BIT(5) | rB_RFE_Jaguar |
| 118 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 119 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 120 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xcb4` | MASKLWORD | rA_RFE_Inv_Jaguar |
| 121 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xeb4` | MASKLWORD | rB_RFE_Inv_Jaguar |
| 122 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xcb8` | BIT(5) | rA_RFE_Jaguar |
| 123 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xeb8` | BIT(5) | rB_RFE_Jaguar |
| 129 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xcbc` | 0x3f | rA_RFE_Inverse_Jaguar |
| 130 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xcbc` | (BIT(11) \| BIT(10)) | rA_RFE_Inverse_Jaguar |
| 131 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xebc` | 0x3f | rB_RFE_Inverse_Jaguar |
| 132 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xebc` | (BIT(11) \| BIT(10)) | rB_RFE_Inverse_Jaguar |
| 138 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 139 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 142 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 143 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 146 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 147 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 152 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xcbc` | 0x3f | rA_RFE_Inverse_Jaguar |
| 153 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xcbc` | (BIT(11) \| BIT(10) \| BIT(9 | rA_RFE_Inverse_Jaguar |
| 155 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xebc` | 0x3f | rB_RFE_Inverse_Jaguar |
| 156 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xebc` | (BIT(11) \| BIT(10) \| BIT(9 | rB_RFE_Inverse_Jaguar |
| 160 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `0xcbc` | BIT(9) | rA_RFE_Inverse_Jaguar |
| 162 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `0xcbc` | BIT(8) | rA_RFE_Inverse_Jaguar |
| 172 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 173 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 176 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 177 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 180 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 181 | phydm_8822b_type18_rfe | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 191 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 192 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 193 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xcb4` | MASKBYTE1 | rA_RFE_Inv_Jaguar |
| 194 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xeb4` | MASKBYTE1 | rB_RFE_Inv_Jaguar |
| 198 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 199 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 200 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xcb4` | MASKBYTE0 | rA_RFE_Inv_Jaguar |
| 201 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xeb4` | MASKBYTE0 | rB_RFE_Inv_Jaguar |
| 205 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 206 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 207 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xcb4` | MASKBYTE0 | rA_RFE_Inv_Jaguar |
| 208 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xeb4` | MASKBYTE0 | rB_RFE_Inv_Jaguar |
| 215 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xcbc` | 0x3f | rA_RFE_Inverse_Jaguar |
| 216 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xcbc` | BIT(11) \| BIT(10) \| BIT(9) | rA_RFE_Inverse_Jaguar |
| 217 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xebc` | 0x3f | rB_RFE_Inverse_Jaguar |
| 218 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xebc` | BIT(11) \| BIT(10) \| BIT(9) | rB_RFE_Inverse_Jaguar |
| 225 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 226 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 229 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 230 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 233 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 234 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 237 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 238 | phydm_8822b_type15_rfe | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 301 | phydm_rfe_8822b_init | odm_set_bb_reg | MAC | `R_0x64` | BIT(29) \| BIT(28) | REG_PAD_CTRL1_8822B, REG_PAD_CTRL1 |
| 302 | phydm_rfe_8822b_init | odm_set_bb_reg | MAC | `R_0x4c` | BIT(26) \| BIT(25) | REG_LED_CFG_8822B, REG_LEDCFG0 |
| 303 | phydm_rfe_8822b_init | odm_set_bb_reg | MAC | `R_0x40` | BIT(2) | REG_GPIO_MUXCFG_8822B, REG_GPIO_MUXCFG |
| 306 | phydm_rfe_8822b_init | odm_set_bb_reg | BB | `R_0x1990` | 0x3f | rA_RFE_Sel_Jaguar2 |
| 307 | phydm_rfe_8822b_init | odm_set_bb_reg | BB | `R_0x1990` | (BIT(11) \| BIT(10)) | rA_RFE_Sel_Jaguar2 |
| 310 | phydm_rfe_8822b_init | odm_set_bb_reg | BB | `R_0x974` | 0x3f |  |
| 311 | phydm_rfe_8822b_init | odm_set_bb_reg | BB | `R_0x974` | (BIT(11) \| BIT(10)) |  |
| 331 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 332 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 333 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xcb4` | MASKBYTE1 | rA_RFE_Inv_Jaguar |
| 334 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xeb4` | MASKBYTE1 | rB_RFE_Inv_Jaguar |
| 337 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xcbc` | 0x3f | rA_RFE_Inverse_Jaguar |
| 338 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xcbc` | (BIT(11) \| BIT(10)) | rA_RFE_Inverse_Jaguar |
| 339 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xebc` | 0x3f | rB_RFE_Inverse_Jaguar |
| 340 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xebc` | (BIT(11) \| BIT(10)) | rB_RFE_Inverse_Jaguar |
| 346 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 347 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 350 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 351 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 354 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 355 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 360 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 361 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 362 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xcb4` | MASKBYTE1 | rA_RFE_Inv_Jaguar |
| 363 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xeb4` | MASKBYTE1 | rB_RFE_Inv_Jaguar |
| 366 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xcbc` | 0x3f | rA_RFE_Inverse_Jaguar |
| 367 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xcbc` | (BIT(11) \| BIT(10)) | rA_RFE_Inverse_Jaguar |
| 368 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xebc` | 0x3f | rB_RFE_Inverse_Jaguar |
| 369 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xebc` | (BIT(11) \| BIT(10)) | rB_RFE_Inverse_Jaguar |
| 375 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 376 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 379 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 380 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 383 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 384 | phydm_rfe_4_11 | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 398 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 399 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 400 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcb4` | MASKBYTE1 | rA_RFE_Inv_Jaguar |
| 401 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xeb4` | MASKBYTE1 | rB_RFE_Inv_Jaguar |
| 403 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 404 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 405 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcb4` | MASKBYTE1 | rA_RFE_Inv_Jaguar |
| 406 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xeb4` | MASKBYTE1 | rB_RFE_Inv_Jaguar |
| 410 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0x810` | 0xfff00000 |  |
| 413 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 414 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 417 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcbc` | 0x3f | rA_RFE_Inverse_Jaguar |
| 418 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcbc` | (BIT(11) \| BIT(10)) | rA_RFE_Inverse_Jaguar |
| 419 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xebc` | 0x3f | rB_RFE_Inverse_Jaguar |
| 420 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xebc` | (BIT(11) \| BIT(10)) | rB_RFE_Inverse_Jaguar |
| 424 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 425 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 426 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcb4` | MASKBYTE1 | rA_RFE_Inv_Jaguar |
| 427 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xeb4` | MASKBYTE1 | rB_RFE_Inv_Jaguar |
| 428 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcb8` | BIT(4) | rA_RFE_Jaguar |
| 429 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xeb8` | BIT(4) | rB_RFE_Jaguar |
| 432 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 433 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 434 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcb4` | MASKBYTE1 | rA_RFE_Inv_Jaguar |
| 435 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xeb4` | MASKBYTE1 | rB_RFE_Inv_Jaguar |
| 436 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcb8` | BIT(5) | rA_RFE_Jaguar |
| 437 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xeb8` | BIT(5) | rB_RFE_Jaguar |
| 441 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcbc` | 0x3f | rA_RFE_Inverse_Jaguar |
| 442 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xcbc` | (BIT(11) \| BIT(10)) | rA_RFE_Inverse_Jaguar |
| 443 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xebc` | 0x3f | rB_RFE_Inverse_Jaguar |
| 444 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xebc` | (BIT(11) \| BIT(10)) | rB_RFE_Inverse_Jaguar |
| 454 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 455 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 458 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 459 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 462 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 463 | phydm_rfe_efem | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 475 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 476 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 477 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xcb4` | MASKBYTE1 | rA_RFE_Inv_Jaguar |
| 478 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xeb4` | MASKBYTE1 | rB_RFE_Inv_Jaguar |
| 481 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xcb0` | 0xffffff | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 482 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xeb0` | 0xffffff | rB_RFE_Pinmux_Jaguar |
| 483 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xcb4` | MASKBYTE1 | rA_RFE_Inv_Jaguar |
| 484 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xeb4` | MASKBYTE1 | rB_RFE_Inv_Jaguar |
| 488 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xcbc` | 0x3f | rA_RFE_Inverse_Jaguar |
| 489 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xcbc` | (BIT(11) \| BIT(10)) | rA_RFE_Inverse_Jaguar |
| 490 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xebc` | 0x3f | rB_RFE_Inverse_Jaguar |
| 491 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xebc` | (BIT(11) \| BIT(10)) | rB_RFE_Inverse_Jaguar |
| 498 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 499 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 502 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 503 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 506 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 507 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 510 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xca0` | MASKLWORD | ODM_REG_RXIQK_MATRIX_LSB_11N |
| 511 | phydm_rfe_ifem | odm_set_bb_reg | BB | `R_0xea0` | MASKLWORD |  |
| 845 | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | BB | `R_0x82c` | MASKDWORD | rFPGA0_XB_HSSIParameter2, ODM_REG_CCK_RPT_FORMAT_11N_B |
| 852 | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | BB | `R_0x830` | MASKDWORD | ODM_TXAGC_B_6_18, rTxAGC_B_Rate18_06 |
| 854 | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | BB | `R_0x830` | MASKDWORD | ODM_TXAGC_B_6_18, rTxAGC_B_Rate18_06 |
| 856 | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | BB | `R_0x830` | MASKDWORD | ODM_TXAGC_B_6_18, rTxAGC_B_Rate18_06 |
| 858 | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | BB | `R_0x838` | MASKDWORD | rCCAonSec_Jaguar, ODM_TXAGC_B_MCS32_5 |
| 861 | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | BB | `R_0x83c` | MASKDWORD | rTxAGC_B_Mcs03_Mcs00, ODM_TXAGC_B_MCS0_MCS3 |
| 864 | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | BB | `R_0x8c8` | BIT(3) \| BIT(2) \| BIT(1) |  |
| 869 | phydm_ccapar_by_rfe_8822b | odm_set_bb_reg | BB | `0x838` | 0xf0 | rCCAonSec_Jaguar, ODM_TXAGC_B_MCS32_5 |
| 880 | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | BB | `R_0x948` | BIT(29) \| BIT(28) | ODM_REG_RX_DFIR_MOD_97F, ODM_REG_EDCCA_DOWN_OPT_11N |
| 881 | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | BB | `R_0x94c` | BIT(29) \| BIT(28) |  |
| 882 | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | BB | `R_0xc20` | BIT(31) |  |
| 883 | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | BB | `R_0xe20` | BIT(31) |  |
| 886 | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | BB | `R_0x948` | BIT(29) \| BIT(28) | ODM_REG_RX_DFIR_MOD_97F, ODM_REG_EDCCA_DOWN_OPT_11N |
| 887 | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | BB | `R_0x94c` | BIT(29) \| BIT(28) |  |
| 888 | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | BB | `R_0xc20` | BIT(31) |  |
| 889 | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | BB | `R_0xe20` | BIT(31) |  |
| 892 | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | BB | `R_0x948` | BIT(29) \| BIT(28) | ODM_REG_RX_DFIR_MOD_97F, ODM_REG_EDCCA_DOWN_OPT_11N |
| 893 | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | BB | `R_0x94c` | BIT(29) \| BIT(28) |  |
| 894 | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | BB | `R_0xc20` | BIT(31) |  |
| 895 | phydm_rxdfirpar_by_bw_8822b | odm_set_bb_reg | BB | `R_0xe20` | BIT(31) |  |
| 952 | phydm_get_condi_num_acc_8822b | odm_set_bb_reg | BB | `R_0x1988` | 0x3fff00 |  |
| 953 | phydm_get_condi_num_acc_8822b | odm_read_1byte | BB | `R_0xf87` |  |  |
| 969 | phydm_get_condi_num_8822b | odm_set_bb_reg | BB | `R_0x1988` | BIT(22) |  |
| 972 | phydm_get_condi_num_8822b | odm_get_bb_reg | BB | `R_0xf84` | (BIT(17) \| BIT(16) \| MASKL | ODM_REG_IGI_C_11N, ODM_REG_CCK_CRC32_ERROR_CNT_11N |
| 1121 | config_phydm_read_txagc_8822b | odm_set_bb_reg | BB | `R_0x1998` | BIT(16) |  |
| 1124 | config_phydm_read_txagc_8822b | odm_set_bb_reg | BB | `R_0x1998` | MASKBYTE0 |  |
| 1127 | config_phydm_read_txagc_8822b | odm_set_bb_reg | BB | `R_0x1998` | BIT(16) |  |
| 1130 | config_phydm_read_txagc_8822b | odm_get_bb_reg | BB | `R_0xd30` | 0x7f0000 |  |
| 1134 | config_phydm_read_txagc_8822b | odm_set_bb_reg | BB | `R_0x1998` | BIT(16) |  |
| 1148 | phydm_dsde_init | odm_set_bb_reg | BB | `R_0x880` | MASKDWORD |  |
| 1149 | phydm_dsde_init | odm_set_bb_reg | BB | `R_0x884` | MASKDWORD |  |
| 1150 | phydm_dsde_init | odm_set_bb_reg | BB | `R_0x888` | MASKDWORD |  |
| 1151 | phydm_dsde_init | odm_set_bb_reg | BB | `R_0x88c` | MASKDWORD | ODM_ENABLE_3_WIRE, ODM_REG_BB_3WIRE_11N |
| 1152 | phydm_dsde_init | odm_set_bb_reg | BB | `R_0x890` | MASKDWORD | ODM_REG_CLM_11N, ODM_REG_NHM_TH9_TH10_11N |
| 1153 | phydm_dsde_init | odm_set_bb_reg | BB | `R_0x894` | MASKDWORD | ODM_REG_CCX_PERIOD_11N |
| 1154 | phydm_dsde_init | odm_set_bb_reg | BB | `R_0x898` | MASKDWORD | ODM_REG_NHM_TH3_TO_TH0_11N |
| 1155 | phydm_dsde_init | odm_set_bb_reg | BB | `R_0x89c` | MASKDWORD | ODM_REG_NHM_TH7_TO_TH4_11N |
| 1156 | phydm_dsde_init | odm_set_bb_reg | BB | `R_0x874` | BIT(0) | ODM_AD_DA_LSB_MASK, ODM_REG_BB_PWR_SAV1_11N |
| 1227 | phydm_dsde_nbi | odm_set_bb_reg | BB | `0x82c` | 0xff000 | rFPGA0_XB_HSSIParameter2, ODM_REG_CCK_RPT_FORMAT_11N_B |
| 1229 | phydm_dsde_nbi | odm_set_bb_reg | BB | `0x82c` | 0xff000 | rFPGA0_XB_HSSIParameter2, ODM_REG_CCK_RPT_FORMAT_11N_B |
| 1233 | phydm_dsde_nbi | odm_set_bb_reg | BB | `0x82c` | 0xf000 | rFPGA0_XB_HSSIParameter2, ODM_REG_CCK_RPT_FORMAT_11N_B |
| 1236 | phydm_dsde_nbi | odm_set_bb_reg | BB | `0x82c` | 0xf000 | rFPGA0_XB_HSSIParameter2, ODM_REG_CCK_RPT_FORMAT_11N_B |
| 1451 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0xc00` | MASKBYTE0 | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 1452 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0xe00` | MASKBYTE0 | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 1453 | phydm_dynamic_spur_det_eliminate | odm_get_bb_reg | BB | `R_0x910` | (BIT(15) \| BIT(14) \| BIT(1 |  |
| 1458 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0x808` | MASKBYTE0 | ODM_PSDREG, ODM_PSD_SETTING |
| 1464 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0x910` | MASKDWORD |  |
| 1468 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0x910` | MASKDWORD |  |
| 1473 | phydm_dynamic_spur_det_eliminate | odm_get_bb_reg | BB | `R_0xf44` | MASKLWORD |  |
| 1476 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0x910` | BIT(22) |  |
| 1480 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0x808` | MASKBYTE0 | ODM_PSDREG, ODM_PSD_SETTING |
| 1486 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0x910` | MASKDWORD |  |
| 1490 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0x910` | MASKDWORD |  |
| 1495 | phydm_dynamic_spur_det_eliminate | odm_get_bb_reg | BB | `R_0xf44` | MASKLWORD |  |
| 1499 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0x910` | BIT(22) |  |
| 1503 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0xc00` | MASKBYTE0 | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 1504 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0xe00` | MASKBYTE0 | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 1505 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0x910` | (BIT(15) \| BIT(14) \| BIT(1 |  |
| 1508 | phydm_dynamic_spur_det_eliminate | odm_set_bb_reg | BB | `R_0x808` | MASKBYTE0 | ODM_PSDREG, ODM_PSD_SETTING |
| 1576 | phydm_spur_calibration_8822b | odm_set_bb_reg | BB | `R_0x87c` | BIT(13) |  |
| 1577 | phydm_spur_calibration_8822b | odm_set_bb_reg | BB | `R_0xc20` | BIT(28) |  |
| 1578 | phydm_spur_calibration_8822b | odm_set_bb_reg | BB | `R_0xe20` | BIT(28) |  |
| 1602 | config_phydm_switch_band_8822b | config_phydm_read_rf_reg_8822b | RF RF_PATH_A | `0x18` | RFREGOFFSETMASK | ODM_CHANNEL |
| 1611 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x808` | BIT(28) | ODM_PSDREG, ODM_PSD_SETTING |
| 1614 | config_phydm_switch_band_8822b | odm_set_bb_reg | MAC | `R_0x454` | BIT(7) | REG_CCK_CHECK_8822B, REG_CCK_CHECK |
| 1617 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0xa80` | BIT(18) |  |
| 1620 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x814` | 0x0000FC00 |  |
| 1627 | config_phydm_switch_band_8822b | odm_get_bb_reg | BB | `R_0x19a8` | BIT(31) |  |
| 1631 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0xc04` | (BIT(18) \| BIT(21)) | ODM_OFDM_RX_ANT, rOFDM0_TRxPathEnable |
| 1632 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0xe04` | (BIT(18) \| BIT(21)) | ODM_TXAGC_A_24_54, rTxAGC_A_Rate54_24 |
| 1635 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8cc` | MASKDWORD |  |
| 1637 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(19) | ODM_REG_NHM_CNT_11N |
| 1638 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(27) | ODM_REG_NHM_CNT_11N |
| 1640 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8cc` | MASKDWORD |  |
| 1642 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(19) | ODM_REG_NHM_CNT_11N |
| 1643 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(27) | ODM_REG_NHM_CNT_11N |
| 1649 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0xc04` | (BIT(18) \| BIT(21)) | ODM_OFDM_RX_ANT, rOFDM0_TRxPathEnable |
| 1650 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0xe04` | (BIT(18) \| BIT(21)) | ODM_TXAGC_A_24_54, rTxAGC_A_Rate54_24 |
| 1653 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8cc` | MASKDWORD |  |
| 1655 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(19) | ODM_REG_NHM_CNT_11N |
| 1656 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(27) | ODM_REG_NHM_CNT_11N |
| 1658 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8cc` | MASKDWORD |  |
| 1660 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(19) | ODM_REG_NHM_CNT_11N |
| 1661 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(27) | ODM_REG_NHM_CNT_11N |
| 1666 | config_phydm_switch_band_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xb3` | RFREGOFFSETMASK |  |
| 1668 | config_phydm_switch_band_8822b | odm_set_rf_reg | RF RF_PATH_B | `RF_0xb3` | RFREGOFFSETMASK |  |
| 1676 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0xa80` | BIT(18) |  |
| 1679 | config_phydm_switch_band_8822b | odm_set_bb_reg | MAC | `R_0x454` | BIT(7) | REG_CCK_CHECK_8822B, REG_CCK_CHECK |
| 1682 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x808` | BIT(28) | ODM_PSDREG, ODM_PSD_SETTING |
| 1691 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x814` | 0x0000FC00 |  |
| 1694 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x814` | 0x0000FC00 |  |
| 1702 | config_phydm_switch_band_8822b | odm_get_bb_reg | BB | `R_0x19a8` | BIT(31) |  |
| 1706 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0xc04` | (BIT(18) \| BIT(21)) | ODM_OFDM_RX_ANT, rOFDM0_TRxPathEnable |
| 1707 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0xe04` | (BIT(18) \| BIT(21)) | ODM_TXAGC_A_24_54, rTxAGC_A_Rate54_24 |
| 1708 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8cc` | MASKDWORD |  |
| 1709 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(27) | ODM_REG_NHM_CNT_11N |
| 1714 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0xc04` | (BIT(18) \| BIT(21)) | ODM_OFDM_RX_ANT, rOFDM0_TRxPathEnable |
| 1715 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0xe04` | (BIT(18) \| BIT(21)) | ODM_TXAGC_A_24_54, rTxAGC_A_Rate54_24 |
| 1718 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8cc` | MASKDWORD |  |
| 1720 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(19) | ODM_REG_NHM_CNT_11N |
| 1721 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(27) | ODM_REG_NHM_CNT_11N |
| 1723 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8cc` | MASKDWORD |  |
| 1725 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(19) | ODM_REG_NHM_CNT_11N |
| 1726 | config_phydm_switch_band_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(27) | ODM_REG_NHM_CNT_11N |
| 1731 | config_phydm_switch_band_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xb3` | RFREGOFFSETMASK |  |
| 1733 | config_phydm_switch_band_8822b | odm_set_rf_reg | RF RF_PATH_B | `RF_0xb3` | RFREGOFFSETMASK |  |
| 1742 | config_phydm_switch_band_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x18` | RFREGOFFSETMASK | ODM_CHANNEL |
| 1745 | config_phydm_switch_band_8822b | odm_set_rf_reg | RF RF_PATH_B | `RF_0x18` | RFREGOFFSETMASK | ODM_CHANNEL |
| 1789 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0x958` | BIT(4) |  |
| 1790 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0xc1c` | (BIT(11) \| BIT(10) \| BIT(9 | rA_TxScale_Jaguar, REG_A_TX_SCALE_JAGUAR |
| 1792 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0xe1c` | (BIT(11) \| BIT(10) \| BIT(9 | rB_TxScale_Jaguar, rTxAGC_A_Mcs15_Mcs12 |
| 1795 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0x958` | BIT(4) |  |
| 1796 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0xc1c` | (BIT(11) \| BIT(10) \| BIT(9 | rA_TxScale_Jaguar, REG_A_TX_SCALE_JAGUAR |
| 1798 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0xe1c` | (BIT(11) \| BIT(10) \| BIT(9 | rB_TxScale_Jaguar, rTxAGC_A_Mcs15_Mcs12 |
| 1801 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0x958` | BIT(4) |  |
| 1802 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0xc1c` | (BIT(11) \| BIT(10) \| BIT(9 | rA_TxScale_Jaguar, REG_A_TX_SCALE_JAGUAR |
| 1804 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0xe1c` | (BIT(11) \| BIT(10) \| BIT(9 | rB_TxScale_Jaguar, rTxAGC_A_Mcs15_Mcs12 |
| 1813 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0x958` | BIT(4) |  |
| 1814 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0xc1c` | (BIT(11) \| BIT(10) \| BIT(9 | rA_TxScale_Jaguar, REG_A_TX_SCALE_JAGUAR |
| 1816 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0xe1c` | (BIT(11) \| BIT(10) \| BIT(9 | rB_TxScale_Jaguar, rTxAGC_A_Mcs15_Mcs12 |
| 1819 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0x958` | BIT(4) |  |
| 1820 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0xc1c` | (BIT(11) \| BIT(10) \| BIT(9 | rA_TxScale_Jaguar, REG_A_TX_SCALE_JAGUAR |
| 1822 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0xe1c` | (BIT(11) \| BIT(10) \| BIT(9 | rB_TxScale_Jaguar, rTxAGC_A_Mcs15_Mcs12 |
| 1825 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0x958` | BIT(4) |  |
| 1826 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0xc1c` | (BIT(11) \| BIT(10) \| BIT(9 | rA_TxScale_Jaguar, REG_A_TX_SCALE_JAGUAR |
| 1828 | phydm_rxagc_switch_8822b | odm_set_bb_reg | BB | `R_0xe1c` | (BIT(11) \| BIT(10) \| BIT(9 | rB_TxScale_Jaguar, rTxAGC_A_Mcs15_Mcs12 |
| 1881 | config_phydm_switch_channel_8822b | config_phydm_read_rf_reg_8822b | RF RF_PATH_A | `0x18` | RFREGOFFSETMASK | ODM_CHANNEL |
| 1895 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0x958` | 0x1f |  |
| 1899 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0x860` | 0x1ffe0000 | ODM_REG_BB_ATC_11AC, ODM_REG_ANTSEL_CTRL_11N |
| 1904 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0xa24` | MASKDWORD | rCCK0_TxFilter2, ODM_REG_CCK_FILTER_PARA3_11N |
| 1905 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0xa28` | MASKLWORD | rCCK0_DebugPort, ODM_REG_CCK_FILTER_PARA7_11N |
| 1907 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0xa24` | MASKDWORD | rCCK0_TxFilter2, ODM_REG_CCK_FILTER_PARA3_11N |
| 1908 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0xa28` | MASKLWORD | rCCK0_DebugPort, ODM_REG_CCK_FILTER_PARA7_11N |
| 1920 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0x958` | 0x1f |  |
| 1923 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0x958` | 0x1f |  |
| 1926 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0x958` | 0x1f |  |
| 1941 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0x860` | 0x1ffe0000 | ODM_REG_BB_ATC_11AC, ODM_REG_ANTSEL_CTRL_11N |
| 1943 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0x860` | 0x1ffe0000 | ODM_REG_BB_ATC_11AC, ODM_REG_ANTSEL_CTRL_11N |
| 1945 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0x860` | 0x1ffe0000 | ODM_REG_BB_ATC_11AC, ODM_REG_ANTSEL_CTRL_11N |
| 1947 | config_phydm_switch_channel_8822b | odm_set_bb_reg | BB | `R_0x860` | 0x1ffe0000 | ODM_REG_BB_ATC_11AC, ODM_REG_ANTSEL_CTRL_11N |
| 1977 | config_phydm_switch_channel_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xbe` | (BIT(17) \| BIT(16) \| BIT(1 |  |
| 1989 | config_phydm_switch_channel_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xdf` | BIT(18) |  |
| 1992 | config_phydm_switch_channel_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xdf` | BIT(18) |  |
| 2000 | config_phydm_switch_channel_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x18` | RFREGOFFSETMASK | ODM_CHANNEL |
| 2003 | config_phydm_switch_channel_8822b | odm_set_rf_reg | RF RF_PATH_B | `RF_0x18` | RFREGOFFSETMASK | ODM_CHANNEL |
| 2013 | config_phydm_switch_channel_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xb8` | BIT(19) |  |
| 2014 | config_phydm_switch_channel_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xb8` | BIT(19) |  |
| 2077 | config_phydm_switch_bandwidth_8822b | config_phydm_read_rf_reg_8822b | RF RF_PATH_A | `0x18` | RFREG_MASK | ODM_CHANNEL |
| 2103 | config_phydm_switch_bandwidth_8822b | odm_get_bb_reg | BB | `R_0x8ac` | MASKDWORD |  |
| 2106 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8ac` | MASKDWORD |  |
| 2109 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8c4` | BIT(30) | ODM_REG_SC_CNT_11N |
| 2124 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0xa00` | BIT(4) | rCCK0_System, ODM_REG_CCK_ANTDIV_PARA1_11N |
| 2126 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0xa00` | BIT(4) | rCCK0_System, ODM_REG_CCK_ANTDIV_PARA1_11N |
| 2140 | config_phydm_switch_bandwidth_8822b | odm_get_bb_reg | BB | `R_0x8ac` | MASKDWORD |  |
| 2143 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8ac` | MASKDWORD |  |
| 2146 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8c4` | BIT(30) | ODM_REG_SC_CNT_11N |
| 2174 | config_phydm_switch_bandwidth_8822b | odm_get_bb_reg | BB | `R_0x8ac` | MASKDWORD |  |
| 2177 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8ac` | MASKDWORD |  |
| 2180 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8c4` | BIT(30) | ODM_REG_SC_CNT_11N |
| 2188 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x840` | 0x0000f000 |  |
| 2189 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8c8` | BIT(10) |  |
| 2214 | config_phydm_switch_bandwidth_8822b | odm_get_bb_reg | BB | `R_0x8ac` | MASKDWORD |  |
| 2217 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8ac` | MASKDWORD |  |
| 2220 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8c4` | BIT(30) | ODM_REG_SC_CNT_11N |
| 2221 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8c8` | BIT(31) |  |
| 2248 | config_phydm_switch_bandwidth_8822b | odm_get_bb_reg | BB | `R_0x8ac` | MASKDWORD |  |
| 2251 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8ac` | MASKDWORD |  |
| 2254 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8c4` | BIT(30) | ODM_REG_SC_CNT_11N |
| 2255 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x8c8` | BIT(31) |  |
| 2267 | config_phydm_switch_bandwidth_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x18` | RFREGOFFSETMASK | ODM_CHANNEL |
| 2270 | config_phydm_switch_bandwidth_8822b | odm_set_rf_reg | RF RF_PATH_B | `RF_0x18` | RFREGOFFSETMASK | ODM_CHANNEL |
| 2297 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x808` | MASKBYTE0 | ODM_PSDREG, ODM_PSD_SETTING |
| 2298 | config_phydm_switch_bandwidth_8822b | odm_set_bb_reg | BB | `R_0x808` | MASKBYTE0 | ODM_PSDREG, ODM_PSD_SETTING |
| 2336 | phydm_config_cck_tx_path_8822b | odm_set_bb_reg | BB | `R_0xa04` | 0xf0000000 | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 2338 | phydm_config_cck_tx_path_8822b | odm_set_bb_reg | BB | `R_0xa04` | 0xf0000000 | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 2340 | phydm_config_cck_tx_path_8822b | odm_set_bb_reg | BB | `R_0xa04` | 0xf0000000 | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 2350 | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | BB | `R_0x93c` | 0xfff00000 | REG_BB_TX_PATH_SEL_1_8822B |
| 2352 | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | BB | `R_0x93c` | 0xfff00000 | REG_BB_TX_PATH_SEL_1_8822B |
| 2355 | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | BB | `R_0x93c` | 0xfff00000 | REG_BB_TX_PATH_SEL_1_8822B |
| 2356 | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | BB | `R_0x940` | 0xfff0 | REG_BB_TX_PATH_SEL_2_8822B |
| 2358 | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | BB | `R_0x93c` | 0xfff00000 | REG_BB_TX_PATH_SEL_1_8822B |
| 2359 | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | BB | `R_0x940` | 0xfff0 | REG_BB_TX_PATH_SEL_2_8822B |
| 2361 | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | BB | `R_0x93c` | 0xfff00000 | REG_BB_TX_PATH_SEL_1_8822B |
| 2362 | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | BB | `R_0x940` | 0xfff0 | REG_BB_TX_PATH_SEL_2_8822B |
| 2372 | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | BB | `R_0x940` | 0xf0 | REG_BB_TX_PATH_SEL_2_8822B |
| 2373 | phydm_config_ofdm_tx_path_8822b | odm_set_bb_reg | BB | `R_0x940` | 0xff00 | REG_BB_TX_PATH_SEL_2_8822B |
| 2387 | phydm_config_tx_path_8822b | odm_set_bb_reg | BB | `R_0x93c` | (BIT(19) \| BIT(18)) | REG_BB_TX_PATH_SEL_1_8822B |
| 2388 | phydm_config_tx_path_8822b | odm_set_bb_reg | BB | `R_0x80c` | (BIT(29) \| BIT(28)) | rFPGA0_TxGainStage, ODM_REG_BB_TX_PATH_11AC |
| 2391 | phydm_config_tx_path_8822b | odm_set_bb_reg | BB | `R_0x80c` | BIT(30) | rFPGA0_TxGainStage, ODM_REG_BB_TX_PATH_11AC |
| 2393 | phydm_config_tx_path_8822b | odm_set_bb_reg | BB | `R_0x80c` | MASKBYTE0 | rFPGA0_TxGainStage, ODM_REG_BB_TX_PATH_11AC |
| 2438 | phydm_config_rx_path_8822b | odm_set_bb_reg | BB | `R_0xa2c` | BIT(22) | ODM_REG_CCK_FA_RST_11N, rCCK0_FalseAlarmReport |
| 2440 | phydm_config_rx_path_8822b | odm_set_bb_reg | BB | `R_0xa2c` | BIT(18) | ODM_REG_CCK_FA_RST_11N, rCCK0_FalseAlarmReport |
| 2444 | phydm_config_rx_path_8822b | odm_set_bb_reg | BB | `R_0xa04` | 0x0f000000 | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 2446 | phydm_config_rx_path_8822b | odm_set_bb_reg | BB | `R_0xa04` | 0x0f000000 | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 2449 | phydm_config_rx_path_8822b | odm_set_bb_reg | BB | `R_0x808` | MASKBYTE0 | ODM_PSDREG, ODM_PSD_SETTING |
| 2453 | phydm_config_rx_path_8822b | odm_set_bb_reg | BB | `R_0x1904` | BIT(16) |  |
| 2455 | phydm_config_rx_path_8822b | odm_set_bb_reg | BB | `R_0x800` | BIT(28) | rFPGA0_RFMOD, REG_FPGA0_RFMOD |
| 2457 | phydm_config_rx_path_8822b | odm_set_bb_reg | BB | `R_0x850` | BIT(23) |  |
| 2460 | phydm_config_rx_path_8822b | odm_set_bb_reg | BB | `R_0x1904` | BIT(16) |  |
| 2462 | phydm_config_rx_path_8822b | odm_set_bb_reg | BB | `R_0x800` | BIT(28) | rFPGA0_RFMOD, REG_FPGA0_RFMOD |
| 2464 | phydm_config_rx_path_8822b | odm_set_bb_reg | BB | `R_0x850` | BIT(23) |  |
| 2503 | config_phydm_trx_mode_8822b | odm_set_bb_reg | BB | `R_0xc08` | MASKLWORD | rOFDM0_TRMuxPar, ODM_REG_TRMUX_11N |
| 2505 | config_phydm_trx_mode_8822b | odm_set_bb_reg | BB | `R_0xc08` | MASKLWORD | rOFDM0_TRMuxPar, ODM_REG_TRMUX_11N |
| 2508 | config_phydm_trx_mode_8822b | odm_set_bb_reg | BB | `R_0xe08` | MASKLWORD | ODM_REG_ANT_11AC_B, ODM_TXAGC_A_1_MCS32 |
| 2510 | config_phydm_trx_mode_8822b | odm_set_bb_reg | BB | `R_0xe08` | MASKLWORD | ODM_REG_ANT_11AC_B, ODM_TXAGC_A_1_MCS32 |
| 2538 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xef` | RFREG_MASK | rf_welut_jaguar |
| 2539 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x33` | RFREG_MASK |  |
| 2542 | config_phydm_trx_mode_8822b | config_phydm_read_rf_reg_8822b | RF RF_PATH_A | `0x33` | RFREG_MASK |  |
| 2559 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xef` | RFREG_MASK | rf_welut_jaguar |
| 2560 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x33` | RFREG_MASK |  |
| 2561 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x3e` | RFREG_MASK |  |
| 2562 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x3f` | RFREG_MASK |  |
| 2563 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xef` | RFREG_MASK | rf_welut_jaguar |
| 2569 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xef` | RFREG_MASK | rf_welut_jaguar |
| 2570 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x33` | RFREG_MASK |  |
| 2571 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x3e` | RFREG_MASK |  |
| 2572 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x3f` | RFREG_MASK |  |
| 2573 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xef` | RFREG_MASK | rf_welut_jaguar |
| 2578 | config_phydm_trx_mode_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xef` | RFREG_MASK | rf_welut_jaguar |
| 2600 | config_phydm_parameter_init_8822b | odm_set_bb_reg | BB | `R_0x808` | (BIT(28) \| BIT(29)) | ODM_PSDREG, ODM_PSD_SETTING |
| 2604 | config_phydm_parameter_init_8822b | odm_set_bb_reg | BB | `R_0x808` | (BIT(28) \| BIT(29)) | ODM_PSDREG, ODM_PSD_SETTING |

## `hal/phydm/halrf/rtl8822b/halrf_iqk_8822b.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 78 | phydm_get_read_counter_8822b | odm_get_rf_reg | RF RF_PATH_A | `RF_0x8` | MASK20BITS |  |
| 85 | phydm_get_read_counter_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x8` | MASK20BITS |  |
| 164 | _iqk_fail_count_8822b | odm_get_rf_reg | RF RF_PATH_A | `RF_0x1bf0` | BIT(16) |  |
| 171 | _iqk_fail_count_8822b | odm_get_bb_reg | BB | `R_0x1bf0` | BIT(i) |  |
| 239 | _iqk_agc_bnd_int_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 240 | _iqk_agc_bnd_int_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 241 | _iqk_agc_bnd_int_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 242 | _iqk_agc_bnd_int_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 254 | _iqk_bb_reset_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x0` | MASK20BITS | ODM_GAIN_SETTING, ODM_REG_RF_MODE_11N |
| 255 | _iqk_bb_reset_8822b | odm_set_rf_reg | RF RF_PATH_B | `RF_0x0` | MASK20BITS | ODM_GAIN_SETTING, ODM_REG_RF_MODE_11N |
| 257 | _iqk_bb_reset_8822b | odm_set_bb_reg | BB | `R_0x8f8` | 0x0ff00000 |  |
| 260 | _iqk_bb_reset_8822b | odm_write_4byte | BB | `0x8fc` |  | ODM_REG_DBG_RPT_11AC |
| 261 | _iqk_bb_reset_8822b | odm_set_bb_reg | BB | `R_0x198c` | 0x7 |  |
| 262 | _iqk_bb_reset_8822b | odm_get_bb_reg | BB | `R_0xfa0` | BIT(3) | ODM_REG_RPT_11AC |
| 272 | _iqk_bb_reset_8822b | odm_write_1byte | BB | `0x808` |  | ODM_PSDREG, ODM_PSD_SETTING |
| 274 | _iqk_bb_reset_8822b | odm_set_bb_reg | BB | `R_0xa04` | bit_mask | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 277 | _iqk_bb_reset_8822b | odm_set_bb_reg | MAC | `R_0x0` | BIT(16) | REG_SYS_ISO_CTRL_8822B, REG_SYS_ISO_CTRL |
| 278 | _iqk_bb_reset_8822b | odm_set_bb_reg | MAC | `R_0x0` | BIT(16) | REG_SYS_ISO_CTRL_8822B, REG_SYS_ISO_CTRL |
| 280 | _iqk_bb_reset_8822b | odm_get_bb_reg | MAC | `R_0x660` | BIT(16) | REG_LBDLY_8822B, REG_LBDLY |
| 281 | _iqk_bb_reset_8822b | odm_write_4byte | MAC | `0x6b4` |  | REG_STATE_MON_8822B, REG_STATE_MON |
| 293 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xc60` |  |  |
| 294 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xc60` |  |  |
| 295 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xe60` |  |  |
| 296 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xe60` |  |  |
| 297 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xc58` |  | ODM_REG_IGI_B_11N, ODM_R_B_AGC_CORE1 |
| 298 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xc5c` |  | ODM_REG_ANTDIV_PARA3_11N |
| 299 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xc6c` |  | ODM_REG_L1SBD_PD_CH_11N |
| 300 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xe58` |  |  |
| 301 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xe5c` |  |  |
| 302 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xe6c` |  | ODM_REG_BLUETOOTH_11N |
| 308 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xc60` |  |  |
| 309 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xc60` |  |  |
| 310 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xe60` |  |  |
| 311 | _iqk_afe_setting_8822b | odm_write_4byte | BB | `0xe60` |  |  |
| 317 | _iqk_afe_setting_8822b | odm_set_bb_reg | BB | `R_0x9a4` | BIT(31) | ODM_REG_OFDM_FA_RST_11AC |
| 340 | _iqk_restore_rf_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xef` | MASK20BITS | rf_welut_jaguar |
| 341 | _iqk_restore_rf_8822b | odm_set_rf_reg | RF RF_PATH_B | `RF_0xef` | MASK20BITS | rf_welut_jaguar |
| 411 | _iqk_backup_iqk_8822b | odm_get_rf_reg | RF (enum rf_path)path | `RF_0x58` | MASK20BITS |  |
| 517 | _iqk_rfe_setting_8822b | odm_write_4byte | BB | `0xcb0` |  | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 518 | _iqk_rfe_setting_8822b | odm_write_4byte | BB | `0xcb4` |  | rA_RFE_Inv_Jaguar |
| 519 | _iqk_rfe_setting_8822b | odm_write_4byte | BB | `0xcbc` |  | rA_RFE_Inverse_Jaguar |
| 520 | _iqk_rfe_setting_8822b | odm_write_4byte | BB | `0xeb0` |  | rB_RFE_Pinmux_Jaguar |
| 521 | _iqk_rfe_setting_8822b | odm_write_4byte | BB | `0xeb4` |  | rB_RFE_Inv_Jaguar |
| 522 | _iqk_rfe_setting_8822b | odm_write_4byte | BB | `0xebc` |  | rB_RFE_Inverse_Jaguar |
| 529 | _iqk_rfe_setting_8822b | odm_write_4byte | BB | `0xcb0` |  | rA_RFE_Pinmux_Jaguar, REG_A_RFE_PINMUX_JAGUAR |
| 530 | _iqk_rfe_setting_8822b | odm_write_4byte | BB | `0xcb4` |  | rA_RFE_Inv_Jaguar |
| 531 | _iqk_rfe_setting_8822b | odm_write_4byte | BB | `0xcbc` |  | rA_RFE_Inverse_Jaguar |
| 532 | _iqk_rfe_setting_8822b | odm_write_4byte | BB | `0xeb0` |  | rB_RFE_Pinmux_Jaguar |
| 533 | _iqk_rfe_setting_8822b | odm_write_4byte | BB | `0xeb4` |  | rB_RFE_Inv_Jaguar |
| 534 | _iqk_rfe_setting_8822b | odm_write_4byte | BB | `0xebc` |  | rB_RFE_Inverse_Jaguar |
| 547 | _iqk_rf_setting_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 548 | _iqk_rf_setting_8822b | odm_write_4byte | BB | `0x1bb8` |  |  |
| 552 | _iqk_rf_setting_8822b | odm_get_rf_reg | RF (enum rf_path)path | `RF_0xdf` | MASK20BITS |  |
| 561 | _iqk_rf_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x65` | MASK20BITS |  |
| 565 | _iqk_rf_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0xef` | BIT(19) | rf_welut_jaguar |
| 567 | _iqk_rf_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x33` | MASK20BITS |  |
| 569 | _iqk_rf_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x3e` | MASK20BITS |  |
| 571 | _iqk_rf_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x3f` | MASK20BITS |  |
| 573 | _iqk_rf_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0xef` | BIT(19) | rf_welut_jaguar |
| 576 | _iqk_rf_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0xef` | BIT(19) | rf_welut_jaguar |
| 578 | _iqk_rf_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x33` | MASK20BITS |  |
| 580 | _iqk_rf_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x3e` | MASK20BITS |  |
| 582 | _iqk_rf_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x3f` | MASK20BITS |  |
| 584 | _iqk_rf_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0xef` | BIT(19) | rf_welut_jaguar |
| 593 | _iqk_configure_macbb_8822b | odm_write_1byte | MAC | `0x522` |  | REG_TXPAUSE_8822B, ODM_TXPAUSE |
| 594 | _iqk_configure_macbb_8822b | odm_set_bb_reg | MAC | `R_0x550` | BIT(11) \| BIT(3) | REG_BCN_CTRL_8822B, REG_BCN_CTRL |
| 596 | _iqk_configure_macbb_8822b | odm_set_bb_reg | BB | `R_0x90c` | BIT(15) | rFPGA1_TxInfo, ODM_R_ANT_SELECT |
| 598 | _iqk_configure_macbb_8822b | odm_set_bb_reg | BB | `R_0xc94` | BIT(0) | ODM_REG_TXIQK_MATRIXA_LSB2_11N |
| 599 | _iqk_configure_macbb_8822b | odm_set_bb_reg | BB | `R_0xe94` | BIT(0) |  |
| 600 | _iqk_configure_macbb_8822b | odm_set_bb_reg | BB | `R_0xc94` | (BIT(11) \| BIT(10)) | ODM_REG_TXIQK_MATRIXA_LSB2_11N |
| 601 | _iqk_configure_macbb_8822b | odm_set_bb_reg | BB | `R_0xe94` | (BIT(11) \| BIT(10)) |  |
| 603 | _iqk_configure_macbb_8822b | odm_write_4byte | BB | `0xc00` |  | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 604 | _iqk_configure_macbb_8822b | odm_write_4byte | BB | `0xe00` |  | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 606 | _iqk_configure_macbb_8822b | odm_set_bb_reg | BB | `R_0xb00` | BIT(8) |  |
| 608 | _iqk_configure_macbb_8822b | odm_set_bb_reg | BB | `R_0x808` | BIT(28) | ODM_PSDREG, ODM_PSD_SETTING |
| 610 | _iqk_configure_macbb_8822b | odm_set_bb_reg | BB | `R_0x838` | BIT(3) \| BIT(2) \| BIT(1) | rCCAonSec_Jaguar, ODM_TXAGC_B_MCS32_5 |
| 654 | _iqk_txk_setting_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 655 | _iqk_txk_setting_8822b | odm_write_4byte | BB | `0x1bcc` |  |  |
| 656 | _iqk_txk_setting_8822b | odm_write_4byte | BB | `0x1b20` |  |  |
| 659 | _iqk_txk_setting_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 661 | _iqk_txk_setting_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 662 | _iqk_txk_setting_8822b | odm_write_4byte | BB | `0x1bcc` |  |  |
| 666 | _iqk_txk_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x56` | MASK20BITS |  |
| 668 | _iqk_txk_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x8f` | MASK20BITS |  |
| 670 | _iqk_txk_setting_8822b | odm_write_1byte | BB | `0x1b2b` |  |  |
| 673 | _iqk_txk_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x56` | MASK20BITS |  |
| 675 | _iqk_txk_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x8f` | MASK20BITS |  |
| 677 | _iqk_txk_setting_8822b | odm_write_1byte | BB | `0x1b2b` |  |  |
| 687 | _iqk_rxk1_setting_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 691 | _iqk_rxk1_setting_8822b | odm_write_1byte | BB | `0x1bcc` |  |  |
| 692 | _iqk_rxk1_setting_8822b | odm_write_1byte | BB | `0x1b2b` |  |  |
| 693 | _iqk_rxk1_setting_8822b | odm_write_4byte | BB | `0x1b20` |  |  |
| 694 | _iqk_rxk1_setting_8822b | odm_write_4byte | BB | `0x1b24` |  |  |
| 695 | _iqk_rxk1_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x56` | MASK20BITS |  |
| 697 | _iqk_rxk1_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x8f` | MASK20BITS |  |
| 701 | _iqk_rxk1_setting_8822b | odm_write_1byte | BB | `0x1bcc` |  |  |
| 702 | _iqk_rxk1_setting_8822b | odm_write_1byte | BB | `0x1b2b` |  |  |
| 703 | _iqk_rxk1_setting_8822b | odm_write_4byte | BB | `0x1b20` |  |  |
| 704 | _iqk_rxk1_setting_8822b | odm_write_4byte | BB | `0x1b24` |  |  |
| 705 | _iqk_rxk1_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x56` | MASK20BITS |  |
| 707 | _iqk_rxk1_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x8f` | MASK20BITS |  |
| 720 | _iqk_rxk2_setting_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 726 | _iqk_rxk2_setting_8822b | odm_write_1byte | BB | `0x1bcc` |  |  |
| 727 | _iqk_rxk2_setting_8822b | odm_write_1byte | BB | `0x1b2b` |  |  |
| 728 | _iqk_rxk2_setting_8822b | odm_write_4byte | BB | `0x1b20` |  |  |
| 729 | _iqk_rxk2_setting_8822b | odm_write_4byte | BB | `0x1b24` |  |  |
| 730 | _iqk_rxk2_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x56` | MASK20BITS |  |
| 732 | _iqk_rxk2_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x8f` | MASK20BITS |  |
| 742 | _iqk_rxk2_setting_8822b | odm_write_1byte | BB | `0x1bcc` |  |  |
| 743 | _iqk_rxk2_setting_8822b | odm_write_1byte | BB | `0x1b2b` |  |  |
| 744 | _iqk_rxk2_setting_8822b | odm_write_4byte | BB | `0x1b20` |  |  |
| 745 | _iqk_rxk2_setting_8822b | odm_write_4byte | BB | `0x1b24` |  |  |
| 746 | _iqk_rxk2_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x56` | MASK20BITS |  |
| 748 | _iqk_rxk2_setting_8822b | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x8f` | MASK20BITS |  |
| 762 | halrf_iqk_set_rf0x8 | odm_set_rf_reg | RF (enum rf_path)path | `RF_0xef` | MASK20BITS | rf_welut_jaguar |
| 764 | halrf_iqk_set_rf0x8 | odm_set_rf_reg | RF (enum rf_path)path | `RF_0x8` | MASK20BITS |  |
| 766 | halrf_iqk_set_rf0x8 | odm_get_rf_reg | RF (enum rf_path)path | `RF_0x8` | MASK20BITS |  |
| 777 | halrf_iqk_check_if_reload | odm_get_bb_reg | BB | `R_0x1bf0` | BIT(16) |  |
| 787 | _iqk_check_cal_8822b | odm_get_rf_reg | RF (enum rf_path)path | `RF_0x8` | MASK20BITS |  |
| 793 | _iqk_check_cal_8822b | odm_get_bb_reg | BB | `R_0x1b08` | BIT(26) |  |
| 827 | _iqk_rxk_gsearch_fail_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 828 | _iqk_rxk_gsearch_fail_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 840 | _iqk_rxk_gsearch_fail_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 841 | _iqk_rxk_gsearch_fail_8822b | odm_write_4byte | BB | `0x1bcc` |  |  |
| 846 | _iqk_rxk_gsearch_fail_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 847 | _iqk_rxk_gsearch_fail_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 851 | _iqk_rxk_gsearch_fail_8822b | odm_get_rf_reg | RF (enum rf_path)path | `RF_0x0` | MASK20BITS | ODM_GAIN_SETTING, ODM_REG_RF_MODE_11N |
| 853 | _iqk_rxk_gsearch_fail_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 857 | _iqk_rxk_gsearch_fail_8822b | odm_read_4byte | BB | `0x1b3c` |  |  |
| 888 | _iqk_rxk_gsearch_fail_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 889 | _iqk_rxk_gsearch_fail_8822b | odm_read_4byte | BB | `0x1b24` |  |  |
| 891 | _iqk_rxk_gsearch_fail_8822b | odm_write_4byte | BB | `0x1b24` |  |  |
| 976 | _iqk_one_shot_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 977 | _iqk_one_shot_8822b | odm_read_4byte | BB | `0x1b24` |  |  |
| 979 | _iqk_one_shot_8822b | odm_write_4byte | BB | `0x1b24` |  |  |
| 981 | _iqk_one_shot_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 982 | _iqk_one_shot_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 987 | _iqk_one_shot_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 990 | _iqk_one_shot_8822b | odm_read_4byte | BB | `0x1b00` |  |  |
| 990 | _iqk_one_shot_8822b | odm_read_4byte | BB | `0x1b08` |  |  |
| 997 | _iqk_one_shot_8822b | odm_get_rf_reg | RF (enum rf_path)path | `RF_0x0` | MASK20BITS | ODM_GAIN_SETTING, ODM_REG_RF_MODE_11N |
| 999 | _iqk_one_shot_8822b | odm_get_rf_reg | RF (enum rf_path)path | `RF_0x56` | MASK20BITS |  |
| 1003 | _iqk_one_shot_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 1014 | _iqk_one_shot_8822b | odm_get_rf_reg | RF (enum rf_path)path | `RF_0x0` | MASK20BITS | ODM_GAIN_SETTING, ODM_REG_RF_MODE_11N |
| 1018 | _iqk_one_shot_8822b | odm_write_4byte | BB | `0x1b38` |  |  |
| 1341 | _iqk_rximr_rxk1_test_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 1342 | _iqk_rximr_rxk1_test_8822b | odm_read_4byte | BB | `0x1b20` |  |  |
| 1343 | _iqk_rximr_rxk1_test_8822b | odm_write_4byte | BB | `0x1b20` |  |  |
| 1344 | _iqk_rximr_rxk1_test_8822b | odm_read_4byte | BB | `0x1b24` |  |  |
| 1345 | _iqk_rximr_rxk1_test_8822b | odm_write_4byte | BB | `0x1b24` |  |  |
| 1348 | _iqk_rximr_rxk1_test_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 1349 | _iqk_rximr_rxk1_test_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 1366 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 1367 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1bc8` |  |  |
| 1368 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x8f8` |  |  |
| 1369 | _iqk_tximr_selfcheck_8822b | odm_read_4byte | BB | `0x1b0c` |  |  |
| 1370 | _iqk_tximr_selfcheck_8822b | odm_read_4byte | BB | `0x1b14` |  |  |
| 1371 | _iqk_tximr_selfcheck_8822b | odm_read_4byte | BB | `0x1b1c` |  |  |
| 1372 | _iqk_tximr_selfcheck_8822b | odm_read_4byte | BB | `0x1b20` |  |  |
| 1373 | _iqk_tximr_selfcheck_8822b | odm_read_4byte | BB | `0x1b24` |  |  |
| 1375 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b0c` |  |  |
| 1376 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b1c` |  |  |
| 1377 | _iqk_tximr_selfcheck_8822b | odm_write_1byte | BB | `0x1b15` |  |  |
| 1378 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b20` |  |  |
| 1379 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b24` |  |  |
| 1380 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b2c` |  |  |
| 1381 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b38` |  |  |
| 1382 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b3c` |  |  |
| 1385 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 1387 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1bcc` |  |  |
| 1389 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1bcc` |  |  |
| 1392 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b34` |  |  |
| 1393 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b34` |  |  |
| 1395 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1bd4` |  |  |
| 1396 | _iqk_tximr_selfcheck_8822b | odm_read_4byte | BB | `0x1bfc` |  |  |
| 1397 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1bd4` |  |  |
| 1398 | _iqk_tximr_selfcheck_8822b | odm_read_4byte | BB | `0x1bfc` |  |  |
| 1401 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 1402 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b0c` |  |  |
| 1403 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b14` |  |  |
| 1404 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b1c` |  |  |
| 1405 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b20` |  |  |
| 1406 | _iqk_tximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b24` |  |  |
| 1450 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 1451 | _iqk_rximr_selfcheck_8822b | odm_read_4byte | BB | `0x1b0c` |  |  |
| 1452 | _iqk_rximr_selfcheck_8822b | odm_read_4byte | BB | `0x1b14` |  |  |
| 1453 | _iqk_rximr_selfcheck_8822b | odm_read_4byte | BB | `0x1b1c` |  |  |
| 1454 | _iqk_rximr_selfcheck_8822b | odm_read_4byte | BB | `0x1b20` |  |  |
| 1455 | _iqk_rximr_selfcheck_8822b | odm_read_4byte | BB | `0x1b24` |  |  |
| 1457 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b0c` |  |  |
| 1458 | _iqk_rximr_selfcheck_8822b | odm_write_1byte | BB | `0x1b15` |  |  |
| 1459 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b1c` |  |  |
| 1460 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b20` |  |  |
| 1461 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b24` |  |  |
| 1462 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b2c` |  |  |
| 1463 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b38` |  |  |
| 1464 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b3c` |  |  |
| 1468 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b1c` |  |  |
| 1470 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b1c` |  |  |
| 1472 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b34` |  |  |
| 1473 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b34` |  |  |
| 1475 | _iqk_rximr_selfcheck_8822b | odm_write_1byte | BB | `0x1bd6` |  |  |
| 1478 | _iqk_rximr_selfcheck_8822b | odm_get_bb_reg | BB | `R_0x1bfc` | BIT(1) |  |
| 1484 | _iqk_rximr_selfcheck_8822b | odm_write_1byte | BB | `0x1bd6` |  |  |
| 1485 | _iqk_rximr_selfcheck_8822b | odm_read_4byte | BB | `0x1bfc` |  |  |
| 1486 | _iqk_rximr_selfcheck_8822b | odm_write_1byte | BB | `0x1bd6` |  |  |
| 1487 | _iqk_rximr_selfcheck_8822b | odm_read_4byte | BB | `0x1bfc` |  |  |
| 1494 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b0c` |  |  |
| 1495 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b14` |  |  |
| 1496 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b1c` |  |  |
| 1497 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b20` |  |  |
| 1498 | _iqk_rximr_selfcheck_8822b | odm_write_4byte | BB | `0x1b24` |  |  |
| 1589 | _iqk_get_rxk1_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 1591 | _iqk_get_rxk1_8822b | odm_write_4byte | BB | `0x1b1c` |  |  |
| 1592 | _iqk_get_rxk1_8822b | odm_write_4byte | BB | `0x1b14` |  |  |
| 1593 | _iqk_get_rxk1_8822b | odm_write_4byte | BB | `0x1b14` |  |  |
| 1594 | _iqk_get_rxk1_8822b | odm_read_4byte | BB | `0x1b38` |  |  |
| 1693 | _iqk_rximr_test_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 1694 | _iqk_rximr_test_8822b | odm_read_4byte | BB | `0x1b1c` |  |  |
| 1711 | _iqk_rximr_test_8822b | odm_write_4byte | BB | `0x1b00` |  |  |
| 1712 | _iqk_rximr_test_8822b | odm_write_4byte | BB | `0x1b1c` |  |  |
| 1713 | _iqk_rximr_test_8822b | odm_write_4byte | BB | `0x1b38` |  |  |
| 1931 | phy_get_iqk_cfir_8822b | odm_set_bb_reg | BB | `R_0x1b00` | MASKDWORD |  |
| 1933 | phy_get_iqk_cfir_8822b | odm_set_bb_reg | BB | `R_0x1b0c` | BIT(13) \| BIT(12) |  |
| 1935 | phy_get_iqk_cfir_8822b | odm_set_bb_reg | BB | `R_0x1b0c` | BIT(13) \| BIT(12) |  |
| 1936 | phy_get_iqk_cfir_8822b | odm_set_bb_reg | BB | `R_0x1bd4` | bit_mask_20_16 |  |
| 1938 | phy_get_iqk_cfir_8822b | odm_set_bb_reg | BB | `R_0x1bd8` | MASKDWORD |  |
| 1939 | phy_get_iqk_cfir_8822b | odm_get_bb_reg | BB | `R_0x1bfc` | MASKDWORD |  |
| 1944 | phy_get_iqk_cfir_8822b | odm_set_bb_reg | BB | `R_0x1bd8` | MASKDWORD |  |
| 1945 | phy_get_iqk_cfir_8822b | odm_set_bb_reg | BB | `R_0x1b0c` | BIT(13) \| BIT(12) |  |
| 1997 | phy_iqk_dbg_cfir_backup_update_8822b | odm_set_bb_reg | BB | `R_0x1b00` | MASKDWORD |  |
| 1999 | phy_iqk_dbg_cfir_backup_update_8822b | odm_set_bb_reg | BB | `R_0x1b2c` | MASKDWORD |  |
| 2000 | phy_iqk_dbg_cfir_backup_update_8822b | odm_set_bb_reg | BB | `R_0x1b38` | MASKDWORD |  |
| 2001 | phy_iqk_dbg_cfir_backup_update_8822b | odm_set_bb_reg | BB | `R_0x1b3c` | MASKDWORD |  |
| 2002 | phy_iqk_dbg_cfir_backup_update_8822b | odm_set_bb_reg | BB | `R_0x1bcc` | MASKDWORD |  |
| 2004 | phy_iqk_dbg_cfir_backup_update_8822b | odm_set_bb_reg | BB | `R_0x1b0c` | bmask13_12 |  |
| 2006 | phy_iqk_dbg_cfir_backup_update_8822b | odm_set_bb_reg | BB | `R_0x1b0c` | bmask13_12 |  |
| 2007 | phy_iqk_dbg_cfir_backup_update_8822b | odm_set_bb_reg | BB | `R_0x1bd4` | bmask20_16 |  |
| 2012 | phy_iqk_dbg_cfir_backup_update_8822b | odm_write_4byte | BB | `0x1bd8` |  |  |
| 2016 | phy_iqk_dbg_cfir_backup_update_8822b | odm_write_4byte | BB | `0x1bd8` |  |  |
| 2023 | phy_iqk_dbg_cfir_backup_update_8822b | odm_set_bb_reg | BB | `R_0x1bd8` | MASKDWORD |  |
| 2024 | phy_iqk_dbg_cfir_backup_update_8822b | odm_set_bb_reg | BB | `R_0x1b0c` | bmask13_12 |  |
| 2044 | phy_iqk_dbg_cfir_reload_8822b | odm_set_bb_reg | BB | `R_0x1b00` | MASKDWORD |  |
| 2046 | phy_iqk_dbg_cfir_reload_8822b | odm_set_bb_reg | BB | `R_0x1b2c` | MASKDWORD |  |
| 2047 | phy_iqk_dbg_cfir_reload_8822b | odm_set_bb_reg | BB | `R_0x1b38` | MASKDWORD |  |
| 2048 | phy_iqk_dbg_cfir_reload_8822b | odm_set_bb_reg | BB | `R_0x1b3c` | MASKDWORD |  |
| 2049 | phy_iqk_dbg_cfir_reload_8822b | odm_set_bb_reg | BB | `R_0x1bcc` | MASKDWORD |  |
| 2051 | phy_iqk_dbg_cfir_reload_8822b | odm_set_bb_reg | BB | `R_0x1b0c` | bmask13_12 |  |
| 2053 | phy_iqk_dbg_cfir_reload_8822b | odm_set_bb_reg | BB | `R_0x1b0c` | bmask13_12 |  |
| 2054 | phy_iqk_dbg_cfir_reload_8822b | odm_set_bb_reg | BB | `R_0x1bd4` | bmask20_16 |  |
| 2063 | phy_iqk_dbg_cfir_reload_8822b | odm_write_4byte | BB | `0x1bd8` |  |  |
| 2067 | phy_iqk_dbg_cfir_reload_8822b | odm_write_4byte | BB | `0x1bd8` |  |  |
| 2070 | phy_iqk_dbg_cfir_reload_8822b | odm_set_bb_reg | BB | `R_0x1bd8` | MASKDWORD |  |
| 2071 | phy_iqk_dbg_cfir_reload_8822b | odm_set_bb_reg | BB | `R_0x1b0c` | bmask13_12 |  |

## `hal/phydm/rtl8822b/phydm_rtl8822b.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 68 | phydm_dynamic_switch_htstf_mumimo_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(17) | ODM_REG_NHM_CNT_11N |
| 70 | phydm_dynamic_switch_htstf_mumimo_8822b | odm_set_bb_reg | BB | `R_0x8d8` | BIT(17) | ODM_REG_NHM_CNT_11N |
| 78 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `0x838` | 0xf0 | rCCAonSec_Jaguar, ODM_TXAGC_B_MCS32_5 |
| 83 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x8d8` | BIT(17) | ODM_REG_NHM_CNT_11N |
| 85 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x98c` | 0x7fc0000 |  |
| 86 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x818` | 0x7000000 | ODM_AFE_SETTING, ODM_REG_BB_PWR_SAV5_11N |
| 87 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0xc04` | BIT(18) | ODM_OFDM_RX_ANT, rOFDM0_TRxPathEnable |
| 88 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0xe04` | BIT(18) | ODM_TXAGC_A_24_54, rTxAGC_A_Rate54_24 |
| 90 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x19d8` | MASKDWORD |  |
| 91 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x19d4` | MASKDWORD |  |
| 93 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x19d8` | MASKDWORD |  |
| 94 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x19d4` | MASKDWORD |  |
| 98 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x8d8` | BIT(17) | ODM_REG_NHM_CNT_11N |
| 100 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x98c` | MASKDWORD |  |
| 101 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x818` | 0x7000000 | ODM_AFE_SETTING, ODM_REG_BB_PWR_SAV5_11N |
| 102 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0xc04` | (BIT(18) \| BIT(21)) | ODM_OFDM_RX_ANT, rOFDM0_TRxPathEnable |
| 103 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0xe04` | (BIT(18) \| BIT(21)) | ODM_TXAGC_A_24_54, rTxAGC_A_Rate54_24 |
| 104 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x19d8` | MASKDWORD |  |
| 105 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x19d4` | MASKDWORD |  |
| 111 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x98c` | MASKDWORD |  |
| 112 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x818` | 0x7000000 | ODM_AFE_SETTING, ODM_REG_BB_PWR_SAV5_11N |
| 113 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0xc04` | (BIT(18) \| BIT(21)) | ODM_OFDM_RX_ANT, rOFDM0_TRxPathEnable |
| 114 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0xe04` | (BIT(18) \| BIT(21)) | ODM_TXAGC_A_24_54, rTxAGC_A_Rate54_24 |
| 115 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x19d8` | MASKDWORD |  |
| 116 | phydm_dynamic_parameters_ota | odm_set_bb_reg | BB | `R_0x19d4` | MASKDWORD |  |
| 134 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 137 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 140 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 143 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 146 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 149 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 152 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 155 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 158 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 161 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 164 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 167 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 175 | _set_tx_a_cali_value | odm_get_rf_reg | RF rf_path | `RF_0x61` | 0xFFFFF |  |
| 239 | _set_tx_a_cali_value | odm_set_rf_reg | RF rf_path | `RF_0x30` | 0xFFFF | rf_mode_table_addr |
| 249 | _txa_bias_cali_4_each_path | odm_set_rf_reg | RF rf_path | `RF_0xef` | 0xFFFFF | rf_welut_jaguar |
| 266 | _txa_bias_cali_4_each_path | odm_set_rf_reg | RF rf_path | `RF_0xef` | 0xFFFFF | rf_welut_jaguar |
| 285 | phydm_txcurrentcalibration | odm_get_rf_reg | RF RF_PATH_A | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 286 | phydm_txcurrentcalibration | odm_get_rf_reg | RF RF_PATH_B | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 304 | phydm_txcurrentcalibration | odm_set_rf_reg | RF RF_PATH_A | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 305 | phydm_txcurrentcalibration | odm_set_rf_reg | RF RF_PATH_B | `RF_0x18` | 0xFFFFF | ODM_CHANNEL |
| 349 | phydm_dynamic_select_cck_path_8822b | odm_set_bb_reg | BB | `R_0xa04` | 0x0f000000 | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 362 | phydm_dynamic_select_cck_path_8822b | odm_set_bb_reg | BB | `R_0xa04` | 0x0f000000 | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 368 | phydm_dynamic_select_cck_path_8822b | odm_set_bb_reg | BB | `R_0xa04` | 0x0f000000 | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 370 | phydm_dynamic_select_cck_path_8822b | odm_set_bb_reg | BB | `R_0xa04` | 0x0f000000 | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 382 | phydm_somlrxhp_setting | odm_set_bb_reg | BB | `R_0x19a8` | MASKDWORD |  |
| 385 | phydm_somlrxhp_setting | odm_set_bb_reg | BB | `R_0x19a8` | MASKDWORD |  |
| 390 | phydm_somlrxhp_setting | odm_set_bb_reg | BB | `R_0x8cc` | MASKDWORD |  |
| 391 | phydm_somlrxhp_setting | odm_set_bb_reg | BB | `R_0x8d8` | BIT(27) | ODM_REG_NHM_CNT_11N |
| 396 | phydm_somlrxhp_setting | odm_set_bb_reg | BB | `R_0x8cc` | MASKDWORD |  |
| 397 | phydm_somlrxhp_setting | odm_set_bb_reg | BB | `R_0x8d8` | BIT(27) | ODM_REG_NHM_CNT_11N |
| 401 | phydm_somlrxhp_setting | odm_set_bb_reg | BB | `R_0x8cc` | MASKDWORD |  |
| 402 | phydm_somlrxhp_setting | odm_set_bb_reg | BB | `R_0x8d8` | BIT(27) | ODM_REG_NHM_CNT_11N |
| 407 | phydm_somlrxhp_setting | odm_set_bb_reg | BB | `0x8d8` | MASKDWORD | ODM_REG_NHM_CNT_11N |
| 408 | phydm_somlrxhp_setting | odm_set_bb_reg | BB | `0x8cc` | MASKDWORD |  |
| 474 | phydm_dynamic_ant_weighting_8822b | odm_set_bb_reg | BB | `R_0x98c` | 0x7fc0000 |  |
| 477 | phydm_dynamic_ant_weighting_8822b | odm_get_bb_reg | BB | `R_0xf94` | BIT(0) \| BIT(1) \| BIT(2) | ODM_REG_OFDM_CRC32_CNT_11N |
| 482 | phydm_dynamic_ant_weighting_8822b | odm_set_bb_reg | BB | `R_0x98c` | MASKDWORD |  |
| 485 | phydm_dynamic_ant_weighting_8822b | odm_get_bb_reg | BB | `R_0xf94` | BIT(0) \| BIT(1) \| BIT(2) | ODM_REG_OFDM_CRC32_CNT_11N |
| 491 | phydm_dynamic_ant_weighting_8822b | odm_set_bb_reg | BB | `R_0x98c` | MASKDWORD |  |
| 493 | phydm_dynamic_ant_weighting_8822b | odm_get_bb_reg | BB | `R_0xf94` | BIT(0) \| BIT(1) \| BIT(2) | ODM_REG_OFDM_CRC32_CNT_11N |
| 590 | phydm_hwsetting_8822b | odm_set_bb_reg | BB | `0x838` | 0xf0 | rCCAonSec_Jaguar, ODM_TXAGC_B_MCS32_5 |

## `core/rtw_mp.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 398 | mpt_InitHWConfig | phy_set_bb_reg | MAC | `0x70` | bMaskByte3 | REG_SYS_SDIO_CTRL_8822B, REG_SYS_SDIO_CTRL |
| 399 | mpt_InitHWConfig | phy_query_bb_reg | MAC | `0x70` | bMaskByte3 | REG_SYS_SDIO_CTRL_8822B, REG_SYS_SDIO_CTRL |
| 400 | mpt_InitHWConfig | phy_set_bb_reg | MAC | `0x1704` | bMaskDWord | REG_WL2LTECOEX_INDIRECT_ACCESS_WRITE_DATA_V1_8822B, REG_WL2LTECOEX_INDIRECT_ACCESS_WRITE_DATA_V1 |
| 401 | mpt_InitHWConfig | phy_query_bb_reg | MAC | `0x1704` | bMaskDWord | REG_WL2LTECOEX_INDIRECT_ACCESS_WRITE_DATA_V1_8822B, REG_WL2LTECOEX_INDIRECT_ACCESS_WRITE_DATA_V1 |
| 402 | mpt_InitHWConfig | phy_set_bb_reg | MAC | `0x1700` | bMaskDWord | REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL_V1_8822B, REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL_V1 |
| 403 | mpt_InitHWConfig | phy_query_bb_reg | MAC | `0x1700` | bMaskDWord | REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL_V1_8822B, REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL_V1 |
| 651 | MPT_InitializeAdapter | phy_query_rf_reg | RF RF_PATH_A | `RF_0x52` | 0x000F0 |  |
| 652 | MPT_InitializeAdapter | phy_query_rf_reg | RF RF_PATH_B | `RF_0x52` | 0x000F0 |  |
| 1891 | Rtw_MPSetMacTxEDCA | rtw_write32 | MAC | `0x508` |  | REG_EDCA_BE_PARAM_8822B, ODM_EDCA_BE_PARAM |
| 1893 | Rtw_MPSetMacTxEDCA | phy_set_mac_reg | MAC | `0x458` | bMaskDWord | REG_AMPDU_MAX_LENGTH_8822B, REG_AMPDU_MAX_LENGTH |
| 1895 | Rtw_MPSetMacTxEDCA | phy_set_mac_reg | MAC | `0x460` | bMaskLWord |  |
| 1899 | Rtw_MPSetMacTxEDCA | phy_query_bb_reg | MAC | `0x460` | bMaskDWord |  |
| 2121 | SetPacketRx | write_bbreg | MAC | `0x550` | BIT3 | REG_BCN_CTRL_8822B, REG_BCN_CTRL |
| 2215 | rtw_mp_psd_init | rtw_read32 | BB | `0xC00` |  | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 2216 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0xC00` | 0x3 | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 2217 | rtw_mp_psd_init | rtw_read32 | BB | `0x808` |  | ODM_PSDREG, ODM_PSD_SETTING |
| 2218 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x808` | 0x10000000 | ODM_PSDREG, ODM_PSD_SETTING |
| 2221 | rtw_mp_psd_init | rtw_read32 | BB | `0x88C` |  | ODM_ENABLE_3_WIRE, ODM_REG_BB_3WIRE_11N |
| 2222 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x88C` | 0x300000 | ODM_ENABLE_3_WIRE, ODM_REG_BB_3WIRE_11N |
| 2224 | rtw_mp_psd_init | rtw_read32 | BB | `0x800` |  | rFPGA0_RFMOD, REG_FPGA0_RFMOD |
| 2225 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x800` | 0x1000000 | rFPGA0_RFMOD, REG_FPGA0_RFMOD |
| 2234 | rtw_mp_psd_init | rtw_read32 | BB | `0xC00` |  | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 2235 | rtw_mp_psd_init | rtw_read32 | BB | `0xE00` |  | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 2236 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0xC00` | 0x3 | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 2237 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0xE00` | 0x3 | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 2238 | rtw_mp_psd_init | rtw_read32 | BB | `0x808` |  | ODM_PSDREG, ODM_PSD_SETTING |
| 2239 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x808` | 0x10000000 | ODM_PSDREG, ODM_PSD_SETTING |
| 2242 | rtw_mp_psd_init | rtw_read32 | BB | `0x88C` |  | ODM_ENABLE_3_WIRE, ODM_REG_BB_3WIRE_11N |
| 2243 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x88C` | 0xF00000 | ODM_ENABLE_3_WIRE, ODM_REG_BB_3WIRE_11N |
| 2245 | rtw_mp_psd_init | rtw_read32 | BB | `0x800` |  | rFPGA0_RFMOD, REG_FPGA0_RFMOD |
| 2246 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x800` | 0x1000000 | rFPGA0_RFMOD, REG_FPGA0_RFMOD |
| 2255 | rtw_mp_psd_init | rtw_read32 | BB | `0xC00` |  | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 2256 | rtw_mp_psd_init | rtw_read32 | BB | `0xE00` |  | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 2257 | rtw_mp_psd_init | rtw_read32 | BB | `0x1800` |  |  |
| 2258 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0xC00` | 0x3 | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 2259 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0xE00` | 0x3 | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 2260 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x1800` | 0x3 |  |
| 2261 | rtw_mp_psd_init | rtw_read32 | BB | `0x808` |  | ODM_PSDREG, ODM_PSD_SETTING |
| 2262 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x808` | 0x10000000 | ODM_PSDREG, ODM_PSD_SETTING |
| 2275 | rtw_mp_psd_init | rtw_read32 | BB | `0xC00` |  | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 2276 | rtw_mp_psd_init | rtw_read32 | BB | `0xE00` |  | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 2277 | rtw_mp_psd_init | rtw_read32 | BB | `0x1800` |  |  |
| 2278 | rtw_mp_psd_init | rtw_read32 | BB | `0x1A00` |  |  |
| 2279 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0xC00` | 0x3 | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 2280 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0xE00` | 0x3 | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 2281 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x1800` | 0x3 |  |
| 2282 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x1A00` | 0x3 |  |
| 2283 | rtw_mp_psd_init | rtw_read32 | BB | `0x808` |  | ODM_PSDREG, ODM_PSD_SETTING |
| 2284 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x808` | 0x10000000 | ODM_PSDREG, ODM_PSD_SETTING |
| 2298 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x910` | 0xC000 |  |
| 2300 | rtw_mp_psd_init | phy_set_bb_reg | BB | `0x808` | 0xC000 | ODM_PSDREG, ODM_PSD_SETTING |
| 2313 | rtw_mp_psd_close | rtw_write32 | BB | `0x88C` |  | ODM_ENABLE_3_WIRE, ODM_REG_BB_3WIRE_11N |
| 2315 | rtw_mp_psd_close | rtw_write32 | BB | `0x800` |  | rFPGA0_RFMOD, REG_FPGA0_RFMOD |
| 2323 | rtw_mp_psd_close | rtw_write32 | BB | `0xC00` |  | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 2327 | rtw_mp_psd_close | rtw_write32 | BB | `0xC00` |  | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 2328 | rtw_mp_psd_close | rtw_write32 | BB | `0xE00` |  | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 2332 | rtw_mp_psd_close | rtw_write32 | BB | `0xC00` |  | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 2333 | rtw_mp_psd_close | rtw_write32 | BB | `0xE00` |  | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 2334 | rtw_mp_psd_close | rtw_write32 | BB | `0x1800` |  |  |
| 2339 | rtw_mp_psd_close | rtw_write32 | BB | `0xC00` |  | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 2340 | rtw_mp_psd_close | rtw_write32 | BB | `0xE00` |  | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 2341 | rtw_mp_psd_close | rtw_write32 | BB | `0x1800` |  |  |
| 2342 | rtw_mp_psd_close | rtw_write32 | BB | `0x1A00` |  |  |
| 2350 | rtw_mp_psd_close | rtw_write32 | BB | `0x808` |  | ODM_PSDREG, ODM_PSD_SETTING |

## `hal/phydm/phydm_api.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 77 | phydm_reset_bb_hw_cnt | odm_set_bb_reg | BB | `R_0xb58` | BIT(0) | ODM_REG_RST_RPT_11AC |
| 78 | phydm_reset_bb_hw_cnt | odm_set_bb_reg | BB | `R_0xb58` | BIT(0) | ODM_REG_RST_RPT_11AC |
| 257 | phydm_config_cck_rx_antenna_init | odm_set_bb_reg | BB | `R_0xa00` | BIT(15) | rCCK0_System, ODM_REG_CCK_ANTDIV_PARA1_11N |
| 258 | phydm_config_cck_rx_antenna_init | odm_set_bb_reg | BB | `R_0xa70` | BIT(7) |  |
| 259 | phydm_config_cck_rx_antenna_init | odm_set_bb_reg | BB | `R_0xa74` | BIT(8) | ODM_REG_BB_PWR_SAV4_11N |
| 260 | phydm_config_cck_rx_antenna_init | odm_set_bb_reg | BB | `R_0xa14` | BIT(7) | ODM_REG_CCK_ANTDIV_PARA4_11N |
| 261 | phydm_config_cck_rx_antenna_init | odm_set_bb_reg | BB | `R_0xa20` | (BIT(5) \| BIT(4)) | rCCK0_TxFilter1, ODM_CCK_MATCH_FILTER |
| 305 | phydm_config_cck_rx_path | odm_set_bb_reg | BB | `R_0xa04` | (BIT(27) \| BIT(26)) | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 307 | phydm_config_cck_rx_path | odm_set_bb_reg | BB | `R_0xa04` | (BIT(25) \| BIT(24)) | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 309 | phydm_config_cck_rx_path | odm_set_bb_reg | BB | `R_0xa74` | BIT(8) | ODM_REG_BB_PWR_SAV4_11N |
| 311 | phydm_config_cck_rx_path | odm_set_bb_reg | BB | `R_0xa2c` | BIT(18) | ODM_REG_CCK_FA_RST_11N, rCCK0_FalseAlarmReport |
| 313 | phydm_config_cck_rx_path | odm_set_bb_reg | BB | `R_0xa2c` | BIT(22) | ODM_REG_CCK_FA_RST_11N, rCCK0_FalseAlarmReport |
| 336 | phydm_config_cck_rx_path | odm_set_bb_reg | BB | `R_0xa04` | (BIT(27) \| BIT(26)) | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 337 | phydm_config_cck_rx_path | odm_set_bb_reg | BB | `R_0xa04` | (BIT(25) \| BIT(24)) | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 339 | phydm_config_cck_rx_path | odm_set_bb_reg | BB | `R_0xa04` | (BIT(27) \| BIT(26)) | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 340 | phydm_config_cck_rx_path | odm_set_bb_reg | BB | `R_0xa04` | (BIT(25) \| BIT(24)) | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 574 | phydm_stop_3_wire | odm_set_bb_reg | BB | `R_0xc00` | 0xf | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 575 | phydm_stop_3_wire | odm_set_bb_reg | BB | `R_0xe00` | 0xf | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 603 | phydm_stop_3_wire | odm_set_bb_reg | BB | `R_0xc00` | 0xf | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 604 | phydm_stop_3_wire | odm_set_bb_reg | BB | `R_0xe00` | 0xf | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 670 | phydm_stop_ic_trx | odm_read_1byte | MAC | `R_0x522` |  | REG_TXPAUSE_8822B, ODM_TXPAUSE |
| 673 | phydm_stop_ic_trx | odm_set_mac_reg | MAC | `R_0x520` | 0xff0000 | REG_TX_PTCL_CTRL_8822B, REG_TX_PTCL_CTRL |
| 680 | phydm_stop_ic_trx | odm_set_bb_reg | BB | `R_0x838` | BIT(1) | rCCAonSec_Jaguar, ODM_TXAGC_B_MCS32_5 |
| 699 | phydm_stop_ic_trx | odm_write_1byte | MAC | `R_0x522` |  | REG_TXPAUSE_8822B, ODM_TXPAUSE |
| 706 | phydm_stop_ic_trx | odm_set_bb_reg | BB | `R_0x838` | BIT(1) | rCCAonSec_Jaguar, ODM_TXAGC_B_MCS32_5 |
| 731 | phydm_dis_cck_trx | odm_get_bb_reg | BB | `R_0xa04` | 0xf0000000 | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 734 | phydm_dis_cck_trx | odm_set_bb_reg | BB | `R_0x808` | BIT(28) | ODM_PSDREG, ODM_PSD_SETTING |
| 736 | phydm_dis_cck_trx | odm_set_bb_reg | BB | `R_0xa04` | 0xf0000000 | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 754 | phydm_dis_cck_trx | odm_set_bb_reg | BB | `R_0x808` | BIT(28) | ODM_PSDREG, ODM_PSD_SETTING |
| 756 | phydm_dis_cck_trx | odm_set_bb_reg | BB | `R_0xa04` | 0xf0000000 | rCCK0_AFESetting, REG_CCK_0_AFE_SETTING |
| 814 | phydm_csi_mask_enable | odm_set_bb_reg | BB | `R_0x874` | BIT(0) | ODM_AD_DA_LSB_MASK, ODM_REG_BB_PWR_SAV1_11N |
| 848 | phydm_clean_all_csi_mask | odm_set_bb_reg | BB | `R_0x880` | MASKDWORD |  |
| 849 | phydm_clean_all_csi_mask | odm_set_bb_reg | BB | `R_0x884` | MASKDWORD |  |
| 850 | phydm_clean_all_csi_mask | odm_set_bb_reg | BB | `R_0x888` | MASKDWORD |  |
| 851 | phydm_clean_all_csi_mask | odm_set_bb_reg | BB | `R_0x88c` | MASKDWORD | ODM_ENABLE_3_WIRE, ODM_REG_BB_3WIRE_11N |
| 852 | phydm_clean_all_csi_mask | odm_set_bb_reg | BB | `R_0x890` | MASKDWORD | ODM_REG_CLM_11N, ODM_REG_NHM_TH9_TH10_11N |
| 853 | phydm_clean_all_csi_mask | odm_set_bb_reg | BB | `R_0x894` | MASKDWORD | ODM_REG_CCX_PERIOD_11N |
| 854 | phydm_clean_all_csi_mask | odm_set_bb_reg | BB | `R_0x898` | MASKDWORD | ODM_REG_NHM_TH3_TO_TH0_11N |
| 855 | phydm_clean_all_csi_mask | odm_set_bb_reg | BB | `R_0x89c` | MASKDWORD | ODM_REG_NHM_TH7_TO_TH4_11N |
| 978 | phydm_set_nbi_reg | odm_set_bb_reg | BB | `R_0x87c` | 0xfc000 |  |
| 1003 | phydm_nbi_enable | odm_set_bb_reg | BB | `R_0x87c` | BIT(13) |  |
| 1004 | phydm_nbi_enable | odm_set_bb_reg | BB | `R_0xc20` | BIT(28) |  |
| 1006 | phydm_nbi_enable | odm_set_bb_reg | BB | `R_0xe20` | BIT(28) |  |
| 1901 | phydm_stop_ck320 | odm_set_bb_reg | BB | `R_0x8b4` | BIT(6) | ODM_PSD_REPORT, ODM_REG_PSD_DATA_11N |
| 1948 | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | BB | `R_0x8b4` | 0x3f | ODM_PSD_REPORT, ODM_REG_PSD_DATA_11N |
| 1952 | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | BB | `R_0x80c` | 0x1f00 | rFPGA0_TxGainStage, ODM_REG_BB_TX_PATH_11AC |
| 1954 | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | BB | `R_0x80c` | 0x3e000 | rFPGA0_TxGainStage, ODM_REG_BB_TX_PATH_11AC |
| 1956 | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | BB | `R_0x80c` | 0x3f00 | rFPGA0_TxGainStage, ODM_REG_BB_TX_PATH_11AC |
| 1958 | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | BB | `R_0x80c` | 0xfc000 | rFPGA0_TxGainStage, ODM_REG_BB_TX_PATH_11AC |
| 1960 | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | BB | `R_0x80c` | 0x7f00 | rFPGA0_TxGainStage, ODM_REG_BB_TX_PATH_11AC |
| 1962 | phydm_bb_ctrl_txagc_ofst | odm_set_bb_reg | BB | `R_0x80c` | 0x3f8000 | rFPGA0_TxGainStage, ODM_REG_BB_TX_PATH_11AC |
| 2880 | phydm_set_weighting_cmn | odm_set_bb_reg | BB | `0xc04` | (BIT(18) \| BIT(21)) | ODM_OFDM_RX_ANT, rOFDM0_TRxPathEnable |
| 2881 | phydm_set_weighting_cmn | odm_set_bb_reg | BB | `0xe04` | (BIT(18) \| BIT(21)) | ODM_TXAGC_A_24_54, rTxAGC_A_Rate54_24 |
| 2894 | phydm_set_weighting_mcc | odm_get_bb_reg | BB | `0x98c` | 0x00ff0000 |  |
| 2896 | phydm_set_weighting_mcc | odm_get_bb_reg | BB | `0x98c` | 0xff000000 |  |
| 2898 | phydm_set_weighting_mcc | odm_get_bb_reg | BB | `0x818` | 0xff000000 | ODM_AFE_SETTING, ODM_REG_BB_PWR_SAV5_11N |
| 2906 | phydm_set_weighting_mcc | odm_get_bb_reg | BB | `0x818` | 0xff000000 | ODM_AFE_SETTING, ODM_REG_BB_PWR_SAV5_11N |

## `hal/hal_mp.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 1664 | hal_mpt_TriggerRFThermalMeter | phy_set_rf_reg | RF RF_PATH_A | `0x42` | BIT17 \| BIT16 | RF_T_METER_NEW, ODM_RF_T_METER_88E |
| 1679 | hal_mpt_ReadRFThermalMeter | phy_query_rf_reg | RF rf_path | `0x42` | 0xfc00 | RF_T_METER_NEW, ODM_RF_T_METER_88E |
| 2041 | hal_mpt_SetCarrierSuppressionTx | phy_set_bb_reg | BB | `0x914` | BIT18 \| BIT17 \| BIT16 | ODM_REG_ANT_MAPPING1_11N, rSingleTone_ContTx_Jaguar |
| 2093 | hal_mpt_query_phytxok | phy_query_bb_reg | BB | `0xF50` | bMaskLWord |  |
| 2095 | hal_mpt_query_phytxok | phy_query_bb_reg | BB | `0xF50` | bMaskHWord |  |
| 2156 | mpt_StopOfdmContTx | phy_set_bb_reg | BB | `0x914` | BIT18 \| BIT17 \| BIT16 | ODM_REG_ANT_MAPPING1_11N, rSingleTone_ContTx_Jaguar |
| 2192 | mpt_StartCckContTx | phy_set_bb_reg | BB | `0x914` | BIT18 \| BIT17 \| BIT16 | ODM_REG_ANT_MAPPING1_11N, rSingleTone_ContTx_Jaguar |
| 2246 | mpt_StartOfdmContTx | phy_set_bb_reg | BB | `0x914` | BIT18 \| BIT17 \| BIT16 | ODM_REG_ANT_MAPPING1_11N, rSingleTone_ContTx_Jaguar |
| 2343 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb04` | 0xf |  |
| 2349 | mpt_ProSetPMacTx | phy_query_bb_reg | BB | `0xf50` | bMaskLWord |  |
| 2350 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb1c` | bMaskLWord |  |
| 2351 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb04` | 0xf |  |
| 2353 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb04` | 0xf |  |
| 2398 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb1c` | bMaskDWord |  |
| 2401 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb40` | bMaskDWord |  |
| 2403 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb44` | bMaskLWord |  |
| 2406 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb0c` | BIT27 |  |
| 2408 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb0c` | BIT27 |  |
| 2410 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb18` | 0xfffff |  |
| 2413 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb08` | bMaskDWord |  |
| 2423 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb0c` | bMaskDWord |  |
| 2425 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb10` | 0xffffff |  |
| 2428 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb0c` | bMaskDWord |  |
| 2430 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb10` | 0xffffff |  |
| 2433 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb14` | bMaskDWord |  |
| 2438 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb20` | bMaskDWord |  |
| 2441 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb24` | bMaskDWord |  |
| 2445 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb28` | bMaskDWord |  |
| 2449 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb2C` | bMaskDWord | ODM_REG_LNA_SWITCH_11N |
| 2452 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb38` | bMaskDWord | ODM_REG_RSSI_CTRL_11N |
| 2456 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb30` | bMaskDWord | PATHDIV_REG, ODM_REG_PATH_SWITCH_11N |
| 2460 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb34` | bMaskDWord |  |
| 2466 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb20` | bMaskDWord |  |
| 2468 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb24` | bMaskDWord |  |
| 2473 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb28` | bMaskDWord |  |
| 2476 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb34` | bMaskDWord |  |
| 2480 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb2c` | bMaskDWord | ODM_REG_LNA_SWITCH_11N |
| 2484 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb30` | bMaskDWord | PATHDIV_REG, ODM_REG_PATH_SWITCH_11N |
| 2488 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb38` | bMaskDWord | ODM_REG_RSSI_CTRL_11N |
| 2491 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb48` | bMaskByte3 |  |
| 2495 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb4c` | 0x1ff |  |
| 2518 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb00` | BIT8 |  |
| 2521 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb04` | 0xf |  |
| 2522 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xA84` | BIT31 |  |
| 2524 | mpt_ProSetPMacTx | phy_set_bb_reg | BB | `0xb04` | 0xf |  |

## `hal/phydm/halrf/halrf_kfree.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 232 | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | RF e_rf_path | `RF_0xde` | BIT(0) |  |
| 233 | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | RF e_rf_path | `RF_0xde` | BIT(5) |  |
| 234 | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | BIT(6) | REG_RF_TX_GAIN_OFFSET |
| 235 | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | RF e_rf_path | `RF_0x65` | BIT(6) |  |
| 241 | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | BIT(19) | REG_RF_TX_GAIN_OFFSET |
| 242 | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | gain_bmask | REG_RF_TX_GAIN_OFFSET |
| 244 | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | RF e_rf_path | `RF_0x65` | BIT(19) |  |
| 245 | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | RF e_rf_path | `RF_0x65` | gain_bmask |  |
| 247 | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | BIT(19) | REG_RF_TX_GAIN_OFFSET |
| 248 | phydm_set_kfree_to_rf_8821c | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | gain_bmask | REG_RF_TX_GAIN_OFFSET |
| 254 | phydm_set_kfree_to_rf_8821c | odm_get_rf_reg | RF e_rf_path | `RF_0x55` | s_gain_bmask | REG_RF_TX_GAIN_OFFSET |
| 255 | phydm_set_kfree_to_rf_8821c | odm_get_rf_reg | RF e_rf_path | `RF_0x65` | s_gain_bmask |  |
| 387 | phydm_set_pa_bias_to_rf_8822b | odm_get_rf_reg | RF e_rf_path | `RF_0x51` | RFREGOFFSETMASK |  |
| 388 | phydm_set_pa_bias_to_rf_8822b | odm_get_rf_reg | RF e_rf_path | `RF_0x52` | RFREGOFFSETMASK |  |
| 422 | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0xef` | BIT(10) | rf_welut_jaguar |
| 423 | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x33` | RFREGOFFSETMASK |  |
| 424 | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x3f` | RFREGOFFSETMASK |  |
| 425 | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x33` | BIT(0) |  |
| 426 | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x3f` | RFREGOFFSETMASK |  |
| 427 | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x33` | BIT(1) |  |
| 428 | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x3f` | RFREGOFFSETMASK |  |
| 429 | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x33` | (BIT(1) \| BIT(0)) |  |
| 430 | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x3f` | RFREGOFFSETMASK |  |
| 431 | phydm_set_pa_bias_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0xef` | BIT(10) | rf_welut_jaguar |
| 435 | phydm_set_pa_bias_to_rf_8822b | odm_get_rf_reg | RF e_rf_path | `RF_0x3f` | (BIT(12) \| BIT(11) \| BIT(1 |  |
| 490 | phydm_set_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0xde` | BIT(0) |  |
| 491 | phydm_set_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0xde` | BIT(4) |  |
| 492 | phydm_set_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x65` | MASKLWORD |  |
| 493 | phydm_set_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | BIT(5) | REG_RF_TX_GAIN_OFFSET |
| 495 | phydm_set_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | BIT(19) | REG_RF_TX_GAIN_OFFSET |
| 496 | phydm_set_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | gain_bmask | REG_RF_TX_GAIN_OFFSET |
| 500 | phydm_set_kfree_to_rf_8822b | odm_get_rf_reg | RF e_rf_path | `RF_0x55` | (BIT(19) \| BIT(18) \| BIT(1 | REG_RF_TX_GAIN_OFFSET |
| 510 | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0xde` | BIT(0) |  |
| 511 | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0xde` | BIT(4) |  |
| 512 | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x65` | MASKLWORD |  |
| 513 | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | BIT(5) | REG_RF_TX_GAIN_OFFSET |
| 515 | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | BIT(19) | REG_RF_TX_GAIN_OFFSET |
| 516 | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | gain_bmask | REG_RF_TX_GAIN_OFFSET |
| 519 | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0xde` | BIT(0) |  |
| 520 | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0xde` | BIT(4) |  |
| 521 | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x65` | MASKLWORD |  |
| 522 | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | BIT(5) | REG_RF_TX_GAIN_OFFSET |
| 523 | phydm_clear_kfree_to_rf_8822b | odm_set_rf_reg | RF e_rf_path | `RF_0x55` | BIT(7) | REG_RF_TX_GAIN_OFFSET |
| 527 | phydm_clear_kfree_to_rf_8822b | odm_get_rf_reg | RF e_rf_path | `RF_0x55` | (BIT(19) \| BIT(18) \| BIT(1 | REG_RF_TX_GAIN_OFFSET |

## `hal/phydm/phydm_debug.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 116 | phydm_bb_dbg_port_header_sel | odm_set_bb_reg | BB | `R_0x8f8` | 0x3c00000 |  |
| 144 | phydm_bb_dbg_port_clock_en | odm_set_bb_reg | BB | `R_0x198c` | 0x7 |  |
| 155 | phydm_get_bb_dbg_port_idx | odm_get_bb_reg | BB | `R_0x8fc` | MASKDWORD | ODM_REG_DBG_RPT_11AC |
| 173 | phydm_set_bb_dbg_port | odm_set_bb_reg | BB | `R_0x8fc` | MASKDWORD | ODM_REG_DBG_RPT_11AC |
| 207 | phydm_get_bb_dbg_port_val | odm_get_bb_reg | BB | `R_0xfa0` | MASKDWORD | ODM_REG_RPT_11AC |
| 479 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xf80` | MASKDWORD |  |
| 519 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xf90` | MASKBYTE1 | ODM_REG_HT_CRC32_CNT_11N |
| 524 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xd14` | MASKDWORD |  |
| 528 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xd54` | MASKDWORD |  |
| 532 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xd94` | MASKDWORD |  |
| 536 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xdd4` | MASKDWORD |  |
| 549 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xf08` | 0xFFFF0000 | ODM_REG_CCK_CCA_CNT_11AC |
| 553 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xfd0` | 0xFFFF | ODM_REG_OFDM_FA_TYPE2_11AC |
| 555 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xfc4` | MASKDWORD | ODM_REG_OFDM_FA_TYPE5_11AC |
| 562 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xfcc` | 0xFFFF | ODM_REG_OFDM_FA_TYPE1_11AC |
| 564 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xfbc` | MASKDWORD | ODM_REG_OFDM_FA_TYPE3_11AC |
| 572 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xfc0` | (0xFFFF0000 >> 16) | ODM_REG_OFDM_FA_TYPE4_11AC |
| 573 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xfc8` | 0xFFFF | ODM_REG_OFDM_FA_TYPE6_11AC |
| 576 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xf8c` | MASKDWORD |  |
| 584 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xf88` | MASKDWORD | ODM_REG_IGI_D_11N, ODM_REG_CCK_CRC32_OK_CNT_11N |
| 601 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xf8c` | MASKDWORD |  |
| 617 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xf20` | MASKDWORD |  |
| 636 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xf2c` | MASKDWORD |  |
| 648 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xf30` | MASKDWORD |  |
| 673 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xf2c` | MASKDWORD |  |
| 693 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xf30` | MASKDWORD |  |
| 714 | phydm_bb_hw_dbg_info_ac | odm_get_bb_reg | BB | `R_0xf34` | MASKDWORD |  |
| 4038 | phydm_enable_big_jump | odm_set_bb_reg | BB | `R_0x8c8` | 0xfe |  |
| 4179 | phydm_per_tone_evm | odm_set_bb_reg | BB | `R_0x8c4` | 0xff8 | ODM_REG_SC_CNT_11N |
| 4183 | phydm_per_tone_evm | odm_read_4byte | BB | `R_0xf8c` |  |  |
| 4206 | phydm_per_tone_evm | odm_set_bb_reg | BB | `R_0x8c4` | 0xff8 | ODM_REG_SC_CNT_11N |
| 4210 | phydm_per_tone_evm | odm_read_4byte | BB | `R_0xf8c` |  |  |
| 4233 | phydm_per_tone_evm | odm_set_bb_reg | BB | `R_0x8c4` | 0xff8 | ODM_REG_SC_CNT_11N |
| 4237 | phydm_per_tone_evm | odm_read_4byte | BB | `R_0xf8c` |  |  |
| 4261 | phydm_per_tone_evm | odm_set_bb_reg | BB | `R_0x8c4` | 0xff8 | ODM_REG_SC_CNT_11N |
| 4265 | phydm_per_tone_evm | odm_read_4byte | BB | `R_0xf8c` |  |  |
| 4288 | phydm_per_tone_evm | odm_set_bb_reg | BB | `R_0x8c4` | 0xff8 | ODM_REG_SC_CNT_11N |
| 4292 | phydm_per_tone_evm | odm_read_4byte | BB | `R_0xf8c` |  |  |
| 4315 | phydm_per_tone_evm | odm_set_bb_reg | BB | `R_0x8c4` | 0xff8 | ODM_REG_SC_CNT_11N |
| 4319 | phydm_per_tone_evm | odm_read_4byte | BB | `R_0xf8c` |  |  |

## `os_dep/linux/ioctl_linux.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 6215 | rtw_dbg_port | rtw_write16 | MAC | `0x6d9` |  |  |
| 6730 | rtw_dbg_port | rtw_write8 | BB | `0xc50` |  | rA_IGI_Jaguar, ODM_REG_IGI_A_11N |
| 6731 | rtw_dbg_port | rtw_read8 | BB | `0xc50` |  | rA_IGI_Jaguar, ODM_REG_IGI_A_11N |
| 6732 | rtw_dbg_port | rtw_write8 | BB | `0xc58` |  | ODM_REG_IGI_B_11N, ODM_R_B_AGC_CORE1 |
| 6733 | rtw_dbg_port | rtw_read8 | BB | `0xc58` |  | ODM_REG_IGI_B_11N, ODM_R_B_AGC_CORE1 |
| 6736 | rtw_dbg_port | rtw_read8 | BB | `0xc50` |  | rA_IGI_Jaguar, ODM_REG_IGI_A_11N |
| 6737 | rtw_dbg_port | rtw_read8 | BB | `0xc58` |  | ODM_REG_IGI_B_11N, ODM_R_B_AGC_CORE1 |
| 6740 | rtw_dbg_port | rtw_read32 | MAC | `0x210` |  | REG_TXDMA_STATUS_8822B, REG_TXDMA_STATUS |
| 6741 | rtw_dbg_port | rtw_read32 | MAC | `0x608` |  | REG_RCR_8822B, REG_RCR |
| 6742 | rtw_dbg_port | rtw_read32 | MAC | `0x280` |  | REG_RXDMA_AGG_PG_TH_8822B, REG_RXDMA_AGG_PG_TH |
| 6743 | rtw_dbg_port | rtw_read32 | MAC | `0x284` |  | REG_RXPKT_NUM_8822B, REG_RXPKT_NUM |
| 6744 | rtw_dbg_port | rtw_read32 | MAC | `0x288` |  | REG_RXDMA_STATUS_8822B, REG_RXDMA_STATUS |
| 6746 | rtw_dbg_port | rtw_read32 | MAC | `0x664` |  | REG_RXERR_RPT_8822B, REG_RXERR_RPT |
| 6751 | rtw_dbg_port | rtw_read32 | MAC | `0x430` |  | REG_DARFRC_8822B, REG_DARFRC |
| 6752 | rtw_dbg_port | rtw_read32 | MAC | `0x438` |  | REG_RARFRC_8822B, REG_RARFRC |
| 6754 | rtw_dbg_port | rtw_read32 | MAC | `0x440` |  | REG_RRSR_8822B, REG_RRSR |
| 6756 | rtw_dbg_port | rtw_read32 | MAC | `0x458` |  | REG_AMPDU_MAX_LENGTH_8822B, REG_AMPDU_MAX_LENGTH |
| 6758 | rtw_dbg_port | rtw_read32 | MAC | `0x484` |  | REG_MACID_SLEEP3_8822B, REG_MACID_SLEEP3 |
| 6759 | rtw_dbg_port | rtw_read32 | MAC | `0x488` |  | REG_MACID_SLEEP1_8822B, REG_MACID_SLEEP1 |
| 6761 | rtw_dbg_port | rtw_read32 | MAC | `0x444` |  | REG_ARFR0_8822B, REG_ARFR0 |
| 6762 | rtw_dbg_port | rtw_read32 | MAC | `0x448` |  |  |
| 6763 | rtw_dbg_port | rtw_read32 | MAC | `0x44c` |  | REG_ARFR1_V1_8822B, REG_ARFR1_V1 |
| 6764 | rtw_dbg_port | rtw_read32 | MAC | `0x450` |  |  |
| 9032 | rtw_mp_efuse_get | rtw_write8 | MAC | `0xa3` |  |  |
| 9033 | rtw_mp_efuse_get | rtw_read8 | MAC | `0xa0` |  | REG_SYSON_FSM_MON_8822B, REG_SYSON_FSM_MON |
| 9630 | rtw_mp_efuse_set | rtw_write8 | MAC | `0xa3` |  |  |
| 9631 | rtw_mp_efuse_set | rtw_read8 | MAC | `0xa0` |  | REG_SYSON_FSM_MON_8822B, REG_SYSON_FSM_MON |
| 9753 | rtw_mp_efuse_set | rtw_write8 | MAC | `0xa3` |  |  |
| 9754 | rtw_mp_efuse_set | rtw_read8 | MAC | `0xa0` |  | REG_SYSON_FSM_MON_8822B, REG_SYSON_FSM_MON |

## `hal/phydm/halrf/rtl8822b/halrf_8822b.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 464 | aac_check_8822b | odm_get_rf_reg | RF RF_PATH_A | `0xc9` | 0xf8 |  |
| 466 | aac_check_8822b | odm_set_rf_reg | RF RF_PATH_A | `0xca` | BIT(19) |  |
| 467 | aac_check_8822b | odm_set_rf_reg | RF RF_PATH_A | `0xb2` | 0x7c000 |  |
| 479 | _phy_lc_calibrate_8822b | odm_read_4byte | BB | `0xc00` |  | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 480 | _phy_lc_calibrate_8822b | odm_read_4byte | BB | `0xe00` |  | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 481 | _phy_lc_calibrate_8822b | odm_write_4byte | BB | `0xc00` |  | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 482 | _phy_lc_calibrate_8822b | odm_write_4byte | BB | `0xe00` |  | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 483 | _phy_lc_calibrate_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x0` | RFREGOFFSETMASK | ODM_GAIN_SETTING, ODM_REG_RF_MODE_11N |
| 484 | _phy_lc_calibrate_8822b | odm_set_rf_reg | RF RF_PATH_B | `RF_0x0` | RFREGOFFSETMASK | ODM_GAIN_SETTING, ODM_REG_RF_MODE_11N |
| 488 | _phy_lc_calibrate_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xc4` | RFREGOFFSETMASK |  |
| 501 | _phy_lc_calibrate_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0xc4` | RFREGOFFSETMASK |  |
| 503 | _phy_lc_calibrate_8822b | odm_write_4byte | BB | `0xc00` |  | ODM_OFDM_PHY0_PAGE_C, ODM_REG_OFDM_FA_HOLDC_11N |
| 504 | _phy_lc_calibrate_8822b | odm_write_4byte | BB | `0xe00` |  | ODM_TXAGC_A_6_18, rTxAGC_A_Rate18_06 |
| 505 | _phy_lc_calibrate_8822b | odm_set_rf_reg | RF RF_PATH_A | `RF_0x0` | RFREGOFFSETMASK | ODM_GAIN_SETTING, ODM_REG_RF_MODE_11N |
| 506 | _phy_lc_calibrate_8822b | odm_set_rf_reg | RF RF_PATH_B | `RF_0x0` | RFREGOFFSETMASK | ODM_GAIN_SETTING, ODM_REG_RF_MODE_11N |
| 552 | phy_set_rf_path_switch_8822b | odm_set_bb_reg | MAC | `R_0x4c` | (BIT(24) \| BIT(23)) | REG_LED_CFG_8822B, REG_LEDCFG0 |
| 554 | phy_set_rf_path_switch_8822b | odm_set_bb_reg | BB | `R_0x974` | 0xff |  |
| 559 | phy_set_rf_path_switch_8822b | odm_set_bb_reg | BB | `R_0x1990` | (BIT(9) \| BIT(8)) | rA_RFE_Sel_Jaguar2 |
| 564 | phy_set_rf_path_switch_8822b | odm_set_bb_reg | BB | `R_0xcbc` | BIT(19) | rA_RFE_Inverse_Jaguar |
| 566 | phy_set_rf_path_switch_8822b | odm_set_bb_reg | BB | `R_0xcb4` | 0xff | rA_RFE_Inv_Jaguar |
| 568 | phy_set_rf_path_switch_8822b | odm_set_bb_reg | MAC | `R_0x70` | MASKBYTE3 | REG_SYS_SDIO_CTRL_8822B, REG_SYS_SDIO_CTRL |
| 569 | phy_set_rf_path_switch_8822b | odm_set_bb_reg | MAC | `R_0x1704` | MASKDWORD | REG_WL2LTECOEX_INDIRECT_ACCESS_WRITE_DATA_V1_8822B, REG_WL2LTECOEX_INDIRECT_ACCESS_WRITE_DATA_V1 |
| 570 | phy_set_rf_path_switch_8822b | odm_set_bb_reg | MAC | `R_0x1700` | MASKDWORD | REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL_V1_8822B, REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL_V1 |
| 577 | phy_set_rf_path_switch_8822b | odm_set_bb_reg | BB | `R_0xcbc` | (BIT(9) \| BIT(8)) | rA_RFE_Inverse_Jaguar |
| 582 | phy_set_rf_path_switch_8822b | odm_set_bb_reg | BB | `R_0xcbc` | (BIT(9) \| BIT(8)) | rA_RFE_Inverse_Jaguar |
| 599 | _phy_query_rf_path_switch_8822b | odm_get_bb_reg | BB | `R_0xcbc` | (BIT(9) \| BIT(8)) | rA_RFE_Inverse_Jaguar |

## `hal/hal_com.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 12413 | rtw_hal_wow_disable | rtw_read8 | MAC | `0x630` |  | REG_WMAC_TCR_TSFT_OFS_8822B, REG_WMAC_TCR_TSFT_OFS |
| 12414 | rtw_hal_wow_disable | rtw_read8 | MAC | `0x631` |  |  |
| 12415 | rtw_hal_wow_disable | rtw_read8 | MAC | `0x634` |  | REG_STMP_THSD_8822B, REG_STMP_THSD |
| 12416 | rtw_hal_wow_disable | rtw_read8 | MAC | `0x1c7` |  | REG_WOWLAN_WAKE_REASON |
| 14157 | hw_var_set_mlme_join | rtw_write8 | MAC | `0x542` |  |  |
| 16909 | rtw_dump_phy_rx_counters | phy_query_bb_reg | BB | `0xF04` | 0x3FFF | ODM_REG_CCK_CRC32_CNT_11AC |
| 16910 | rtw_dump_phy_rx_counters | phy_query_bb_reg | BB | `0xF14` | 0x3FFF | ODM_REG_PAGE_F_RST_11N, ODM_REG_OFDM_CRC32_CNT_11AC |
| 16911 | rtw_dump_phy_rx_counters | phy_query_bb_reg | BB | `0xF10` | 0x3FFF | ODM_REG_HT_CRC32_CNT_11AC |
| 16912 | rtw_dump_phy_rx_counters | phy_query_bb_reg | BB | `0xF0C` | 0x3FFF | ODM_REG_VHT_CRC32_CNT_11AC |
| 16913 | rtw_dump_phy_rx_counters | phy_query_bb_reg | BB | `0xF04` | 0x3FFF0000 | ODM_REG_CCK_CRC32_CNT_11AC |
| 16914 | rtw_dump_phy_rx_counters | phy_query_bb_reg | BB | `0xF14` | 0x3FFF0000 | ODM_REG_PAGE_F_RST_11N, ODM_REG_OFDM_CRC32_CNT_11AC |
| 16915 | rtw_dump_phy_rx_counters | phy_query_bb_reg | BB | `0xF10` | 0x3FFF0000 | ODM_REG_HT_CRC32_CNT_11AC |
| 16916 | rtw_dump_phy_rx_counters | phy_query_bb_reg | BB | `0xF0C` | 0x3FFF0000 | ODM_REG_VHT_CRC32_CNT_11AC |
| 16917 | rtw_dump_phy_rx_counters | phy_query_bb_reg | BB | `0xA5C` | bMaskLWord | ODM_CCK_FA_CNT_LSB, ODM_REG_CCK_FA_11AC |
| 16918 | rtw_dump_phy_rx_counters | phy_query_bb_reg | BB | `0xF48` | bMaskLWord | ODM_REG_OFDM_FA_11AC |
| 16964 | rtw_reset_phy_trx_ok_counters | phy_set_bb_reg | BB | `0xB58` | BIT0 | ODM_REG_RST_RPT_11AC |
| 16965 | rtw_reset_phy_trx_ok_counters | phy_set_bb_reg | BB | `0xB58` | BIT0 | ODM_REG_RST_RPT_11AC |
| 16991 | rtw_reset_phy_rx_counters | phy_set_bb_reg | BB | `0x9A4` | BIT17 | ODM_REG_OFDM_FA_RST_11AC |
| 16992 | rtw_reset_phy_rx_counters | phy_set_bb_reg | BB | `0x9A4` | BIT17 | ODM_REG_OFDM_FA_RST_11AC |
| 16994 | rtw_reset_phy_rx_counters | phy_set_bb_reg | BB | `0xA2C` | BIT15 | ODM_REG_CCK_FA_RST_11N, rCCK0_FalseAlarmReport |
| 16995 | rtw_reset_phy_rx_counters | phy_set_bb_reg | BB | `0xA2C` | BIT15 | ODM_REG_CCK_FA_RST_11N, rCCK0_FalseAlarmReport |

## `hal/phydm/phydm_psd.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 166 | phydm_psd | odm_get_rf_reg | RF RF_PATH_A | `RF_0x18` | RFREG_MASK | ODM_CHANNEL |
| 168 | phydm_psd | odm_get_rf_reg | RF RF_PATH_B | `RF_0x18` | RFREG_MASK | ODM_CHANNEL |
| 199 | phydm_psd | odm_set_rf_reg | RF RF_PATH_A | `RF_0x18` | 0xff | ODM_CHANNEL |
| 200 | phydm_psd | odm_set_rf_reg | RF RF_PATH_B | `RF_0x18` | 0xff | ODM_CHANNEL |
| 201 | phydm_psd | odm_set_rf_reg | RF RF_PATH_A | `RF_0x18` | 0x300 | ODM_CHANNEL |
| 202 | phydm_psd | odm_set_rf_reg | RF RF_PATH_B | `RF_0x18` | 0x300 | ODM_CHANNEL |
| 227 | phydm_psd | odm_set_rf_reg | RF RF_PATH_A | `RF_0x18` | 0xc00 | ODM_CHANNEL |
| 230 | phydm_psd | odm_set_rf_reg | RF RF_PATH_B | `RF_0x18` | 0xc00 | ODM_CHANNEL |
| 233 | phydm_psd | odm_set_rf_reg | RF RF_PATH_A | `RF_0x18` | 0xf0000 | ODM_CHANNEL |
| 235 | phydm_psd | odm_set_rf_reg | RF RF_PATH_B | `RF_0x18` | 0xf0000 | ODM_CHANNEL |
| 243 | phydm_psd | odm_get_bb_reg | BB | `R_0xc50` | MASKDWORD | rA_IGI_Jaguar, ODM_REG_IGI_A_11N |
| 246 | phydm_psd | odm_get_rf_reg | RF RF_PATH_A | `RF_0x18` | RFREG_MASK | ODM_CHANNEL |
| 308 | phydm_psd | odm_set_rf_reg | RF RF_PATH_A | `RF_0x18` | RFREG_MASK | ODM_CHANNEL |
| 310 | phydm_psd | odm_set_rf_reg | RF RF_PATH_B | `RF_0x18` | RFREG_MASK | ODM_CHANNEL |
| 364 | phydm_psd_para_setting | odm_set_bb_reg | BB | `R_0x910` | BIT(11) \| BIT(10) |  |
| 365 | phydm_psd_para_setting | odm_set_bb_reg | BB | `R_0x910` | BIT(13) \| BIT(12) |  |
| 366 | phydm_psd_para_setting | odm_set_bb_reg | BB | `R_0x910` | BIT(15) \| BIT(14) |  |
| 368 | phydm_psd_para_setting | odm_set_bb_reg | BB | `R_0x910` | BIT(17) \| BIT(16) |  |
| 369 | phydm_psd_para_setting | odm_set_bb_reg | BB | `R_0x910` | BIT(23) |  |

## `hal/phydm/txbf/haltxbf8822b.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 1065 | phydm_8822btxbf_rfmode | odm_set_rf_reg | RF (enum rf_path)i | `RF_0xef` | BIT(19) | rf_welut_jaguar |
| 1066 | phydm_8822btxbf_rfmode | odm_set_rf_reg | RF (enum rf_path)i | `RF_0x33` | 0xF |  |
| 1067 | phydm_8822btxbf_rfmode | odm_set_rf_reg | RF (enum rf_path)i | `RF_0x3e` | 0xfffff |  |
| 1068 | phydm_8822btxbf_rfmode | odm_set_rf_reg | RF (enum rf_path)i | `RF_0x3f` | 0xfffff |  |
| 1069 | phydm_8822btxbf_rfmode | odm_set_rf_reg | RF (enum rf_path)i | `RF_0xef` | BIT(19) | rf_welut_jaguar |
| 1101 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0x19f8` | BIT(22) \| BIT(21) \| BIT(20 |  |
| 1102 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0x19f8` | BIT(25) \| BIT(24) \| BIT(23 |  |
| 1103 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0x19f8` | BIT(16) |  |
| 1106 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0x19f0` | BIT(5) \| BIT(4) \| BIT(3) \ |  |
| 1108 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0x19f0` | BIT(5) \| BIT(4) \| BIT(3) \ |  |
| 1110 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0x19f0` | BIT(7) \| BIT(6) |  |
| 1111 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0x19f0` | BIT(9) \| BIT(8) |  |
| 1112 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0x19f0` | BIT(11) \| BIT(10) |  |
| 1113 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0x19f0` | BIT(13) \| BIT(12) |  |
| 1115 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0xb58` | BIT(3) \| BIT(2) | ODM_REG_RST_RPT_11AC |
| 1116 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0xb58` | BIT(7) \| BIT(6) \| BIT(5) \ | ODM_REG_RST_RPT_11AC |
| 1117 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0xb58` | BIT(9) \| BIT(8) | ODM_REG_RST_RPT_11AC |
| 1118 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0xb58` | BIT(13) \| BIT(12) \| BIT(11 | ODM_REG_RST_RPT_11AC |
| 1120 | phydm_8822b_sutxbfer_workaroud | odm_set_bb_reg | BB | `R_0x19f8` | BIT(16) |  |

## `hal/phydm/phydm_adc_sampling.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 814 | phydm_la_access_tx_pkt_buf | odm_set_mac_reg | MAC | `R_0x0140` | MASKLWORD | REG_PKTBUF_DBG_CTRL_8822B, REG_PKTBUF_DBG_CTRL |
| 851 | phydm_la_get_tx_pkt_buf | odm_write_1byte | MAC | `R_0x0106` |  |  |
| 852 | phydm_la_get_tx_pkt_buf | odm_read_4byte | MAC | `R_0x7c0` |  | REG_IQ_DUMP_8822B, REG_IQ_DUMP |
| 953 | phydm_la_set_mac_iq_dump | odm_set_mac_reg | MAC | `R_0xf4` | BIT(16) | REG_SYS_STATUS1_8822B, REG_SYS_STATUS1 |
| 954 | phydm_la_set_mac_iq_dump | odm_set_mac_reg | MAC | `R_0x38` | 0xff0000 | REG_PWR_OPTION_CTRL_8822B, REG_PWR_OPTION_CTRL |
| 1017 | phydm_la_set_bb_dbg_port | odm_set_bb_reg | BB | `R_0x95c` | 0x1f | odm_adc_trigger_jaguar2 |
| 1067 | phydm_la_set_bb | odm_set_bb_reg | BB | `R_0x8b4` | BIT(7) | ODM_PSD_REPORT, ODM_REG_PSD_DATA_11N |
| 1068 | phydm_la_set_bb | odm_set_bb_reg | BB | `R_0x95c` | 0xf00 | odm_adc_trigger_jaguar2 |
| 1070 | phydm_la_set_bb | odm_set_bb_reg | BB | `R_0x95c` | BIT(31) | odm_adc_trigger_jaguar2 |
| 1071 | phydm_la_set_bb | odm_set_bb_reg | BB | `R_0x95c` | 0xe0 | odm_adc_trigger_jaguar2 |
| 1164 | phydm_la_set_mac_trigger_time | odm_set_mac_reg | MAC | `R_0x7cc` | BIT(20) \| BIT(19) \| BIT(18 | REG_WMAC_FTM_CTL_8822B, REG_WMAC_FTM_CTL |
| 1165 | phydm_la_set_mac_trigger_time | odm_set_mac_reg | MAC | `R_0x7c0` | 0x7f00 | REG_IQ_DUMP_8822B, REG_IQ_DUMP |
| 1288 | phydm_la_adc_smp_start | odm_read_1byte | MAC | `R_0x7c0` |  | REG_IQ_DUMP_8822B, REG_IQ_DUMP |

## `hal/btc/halbtc8822bwifionly.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 66 | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | MAC | `0x4c` | 0x01800000 | REG_LED_CFG_8822B, REG_LEDCFG0 |
| 68 | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | BB | `0xcb4` | 0xff | rA_RFE_Inv_Jaguar |
| 70 | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | BB | `0x974` | 0x300 |  |
| 72 | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | BB | `0x1990` | 0x300 | rA_RFE_Sel_Jaguar2 |
| 74 | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | BB | `0xcbc` | 0x80000 | rA_RFE_Inverse_Jaguar |
| 76 | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | MAC | `0x70` | 0xff000000 | REG_SYS_SDIO_CTRL_8822B, REG_SYS_SDIO_CTRL |
| 78 | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | MAC | `0x1704` | 0xffffffff | REG_WL2LTECOEX_INDIRECT_ACCESS_WRITE_DATA_V1_8822B, REG_WL2LTECOEX_INDIRECT_ACCESS_WRITE_DATA_V1 |
| 79 | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | MAC | `0x1700` | 0xffffffff | REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL_V1_8822B, REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL_V1 |
| 81 | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | MAC | `0x6c0` | 0xffffffff | REG_BT_COEX_TABLE_8822B, REG_BT_COEX_TABLE |
| 82 | ex_hal8822b_wifi_only_hw_config | halwifionly_phy_set_bb_reg | MAC | `0x6c4` | 0xffffffff |  |
| 119 | hal8822b_wifi_only_switch_antenna | halwifionly_phy_set_bb_reg | BB | `0xcbc` | 0x300 | rA_RFE_Inverse_Jaguar |
| 121 | hal8822b_wifi_only_switch_antenna | halwifionly_phy_set_bb_reg | BB | `0xcbc` | 0x300 | rA_RFE_Inverse_Jaguar |

## `hal/phydm/phydm_ccx.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 162 | phydm_fahm_trigger | odm_set_bb_reg | BB | `R_0x1cf8` | 0xffff00 |  |
| 311 | phydm_fahm_set_th_by_igi | odm_set_bb_reg | BB | `R_0x1c38` | 0xffffff00 |  |
| 313 | phydm_fahm_set_th_by_igi | odm_set_bb_reg | BB | `R_0x1c78` | 0xffffff00 |  |
| 315 | phydm_fahm_set_th_by_igi | odm_set_bb_reg | BB | `R_0x1c7c` | 0xffff0000 |  |
| 317 | phydm_fahm_set_th_by_igi | odm_set_bb_reg | BB | `R_0x1cb8` | 0xffffff00 |  |
| 610 | phydm_nhm_get_result | odm_read_4byte | BB | `R_0xfa8` |  | ODM_REG_NHM_CNT_11AC |
| 613 | phydm_nhm_get_result | odm_read_4byte | BB | `R_0xfac` |  | ODM_REG_NHM_CNT7_TO_CNT4_11AC |
| 616 | phydm_nhm_get_result | odm_read_4byte | BB | `R_0xfb0` |  | ODM_REG_NHM_CNT11_TO_CNT8_11AC |
| 620 | phydm_nhm_get_result | odm_read_4byte | BB | `R_0xfb4` |  | ODM_REG_NHM_DUR_READY_11AC |
| 1655 | phydm_clm_setting | odm_set_bb_reg | BB | `R_0x990` | MASKLWORD | ODM_REG_CCX_PERIOD_11AC |
| 1774 | phydm_clm_get_result | odm_get_bb_reg | BB | `R_0xfa4` | MASKLWORD | ODM_REG_CLM_RESULT_11AC |

## `hal/phydm/phydm.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 3294 | phydm_dc_cancellation | odm_set_bb_reg | BB | `R_0xa78` | MASKBYTE1 |  |
| 3321 | phydm_dc_cancellation | odm_set_bb_reg | BB | `R_0xa9c` | BIT(20) |  |
| 3350 | phydm_dc_cancellation | odm_set_bb_reg | BB | `R_0xc10` | 0x3c000000 | ODM_REG_DOWNSAM_FACTOR_11N |
| 3352 | phydm_dc_cancellation | odm_set_bb_reg | BB | `R_0xc10` | 0xfc00 | ODM_REG_DOWNSAM_FACTOR_11N |
| 3353 | phydm_dc_cancellation | odm_set_bb_reg | BB | `R_0xc14` | 0x3c000000 | ODM_R_A_RXIQI, ODM_REG_RXIQI_MATRIX_11N |
| 3355 | phydm_dc_cancellation | odm_set_bb_reg | BB | `R_0xc14` | 0xfc00 | ODM_R_A_RXIQI, ODM_REG_RXIQI_MATRIX_11N |
| 3368 | phydm_dc_cancellation | odm_set_bb_reg | BB | `R_0xe10` | 0x3c000000 | rTxAGC_A_Mcs03_Mcs00, ODM_TXAGC_A_MCS0_MCS3 |
| 3370 | phydm_dc_cancellation | odm_set_bb_reg | BB | `R_0xe10` | 0xfc00 | rTxAGC_A_Mcs03_Mcs00, ODM_TXAGC_A_MCS0_MCS3 |
| 3372 | phydm_dc_cancellation | odm_set_bb_reg | BB | `R_0xe14` | 0x3c000000 | rTxAGC_A_Mcs07_Mcs04, ODM_TXAGC_A_MCS4_MCS7 |
| 3374 | phydm_dc_cancellation | odm_set_bb_reg | BB | `R_0xe14` | 0xfc00 | rTxAGC_A_Mcs07_Mcs04, ODM_TXAGC_A_MCS4_MCS7 |

## `hal/phydm/phydm_adaptivity.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 222 | phydm_set_edcca_threshold | odm_set_bb_reg | BB | `R_0x8a4` | MASKBYTE0 | rFPGA0_XB_LSSIReadBack, REG_FPGA0_XB_LSSI_READ_BACK |
| 223 | phydm_set_edcca_threshold | odm_set_bb_reg | BB | `R_0x8a4` | MASKBYTE1 | rFPGA0_XB_LSSIReadBack, REG_FPGA0_XB_LSSI_READ_BACK |
| 233 | phydm_mac_edcca_state | odm_set_mac_reg | MAC | `R_0x520` | BIT(15) | REG_TX_PTCL_CTRL_8822B, REG_TX_PTCL_CTRL |
| 235 | phydm_mac_edcca_state | odm_set_mac_reg | MAC | `R_0x524` | BIT(11) | REG_RD_CTRL_8822B, REG_RD_CTRL |
| 238 | phydm_mac_edcca_state | odm_set_mac_reg | MAC | `R_0x520` | BIT(15) | REG_TX_PTCL_CTRL_8822B, REG_TX_PTCL_CTRL |
| 240 | phydm_mac_edcca_state | odm_set_mac_reg | MAC | `R_0x524` | BIT(11) | REG_RD_CTRL_8822B, REG_RD_CTRL |
| 373 | phydm_set_forgetting_factor | odm_set_bb_reg | BB | `R_0x8a0` | BIT(1) \| BIT(0) |  |
| 384 | phydm_edcca_decision_opt | odm_set_bb_reg | BB | `R_0x8dc` | BIT(5) | ODM_REG_EDCCA_POWER_CAL, ODM_REG_NHM_CNT7_TO_CNT4_11N |
| 450 | phydm_adaptivity_debug | odm_get_bb_reg | BB | `R_0x8a4` | MASKDWORD | rFPGA0_XB_LSSIReadBack, REG_FPGA0_XB_LSSI_READ_BACK |
| 755 | phydm_adaptivity_init | odm_set_bb_reg | BB | `R_0x944` | BIT(29) \| BIT(28) | ODM_REG_ACBB_EDCCA_ENHANCE |

## `hal/phydm/phydm_direct_bf.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 76 | phydm_iq_gen_en | odm_set_rf_reg | RF path | `RF_0xef` | BIT(19) | rf_welut_jaguar |
| 78 | phydm_iq_gen_en | odm_set_rf_reg | RF path | `RF_0x33` | 0xF |  |
| 80 | phydm_iq_gen_en | odm_set_rf_reg | RF path | `RF_0x3e` | 0xfffff |  |
| 82 | phydm_iq_gen_en | odm_set_rf_reg | RF path | `RF_0x3f` | 0xfffff |  |
| 84 | phydm_iq_gen_en | odm_set_rf_reg | RF path | `RF_0xef` | BIT(19) | rf_welut_jaguar |
| 144 | phydm_dis_cdd | odm_set_bb_reg | BB | `R_0x808` | 0x3ffff00 | ODM_PSDREG, ODM_PSD_SETTING |
| 145 | phydm_dis_cdd | odm_set_bb_reg | BB | `R_0x9ac` | 0x1fff |  |
| 146 | phydm_dis_cdd | odm_set_bb_reg | BB | `R_0x9ac` | BIT(13) |  |
| 232 | phydm_pathb_q_matrix_rotate | odm_set_bb_reg | BB | `R_0x1954` | 0xffffff |  |
| 233 | phydm_pathb_q_matrix_rotate | odm_set_bb_reg | BB | `R_0x1950` | 0xffffff |  |

## `hal/phydm/phydm_rainfo.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 203 | phydm_fw_fix_rate | odm_set_bb_reg | MAC | `R_0x450` | MASKDWORD |  |
| 394 | phydm_ra_dynamic_retry_count | odm_set_mac_reg | MAC | `R_0x430` | MASKDWORD | REG_DARFRC_8822B, REG_DARFRC |
| 395 | phydm_ra_dynamic_retry_count | odm_set_mac_reg | MAC | `R_0x434` | MASKDWORD |  |
| 398 | phydm_ra_dynamic_retry_count | odm_set_mac_reg | MAC | `R_0x430` | MASKDWORD | REG_DARFRC_8822B, REG_DARFRC |
| 399 | phydm_ra_dynamic_retry_count | odm_set_mac_reg | MAC | `R_0x434` | MASKDWORD |  |
| 1813 | phydm_rrsr_set_register | odm_set_mac_reg | MAC | `R_0x440` | 0xfffff | REG_RRSR_8822B, REG_RRSR |
| 1825 | phydm_masked_rrsr_set_register | odm_set_mac_reg | MAC | `R_0x440` | 0xfffff | REG_RRSR_8822B, REG_RRSR |
| 1921 | phydm_ra_info_init | odm_get_mac_reg | MAC | `R_0x440` | MASKDWORD | REG_RRSR_8822B, REG_RRSR |
| 1928 | phydm_ra_info_init | odm_get_bb_reg | MAC | `R_0x4c8` | MASKBYTE2 | REG_PROT_MODE_CTRL_8822B, REG_PROT_MODE_CTRL |
| 1929 | phydm_ra_info_init | odm_set_bb_reg | MAC | `R_0x4cc` | MASKBYTE3 | REG_BAR_MODE_CTRL_8822B, REG_BAR_MODE_CTRL |

## `hal/phydm/phydm_cfotracking.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 86 | phydm_get_cfo_info_ac | odm_read_4byte | BB | `R_0xd0c` |  |  |
| 87 | phydm_get_cfo_info_ac | odm_read_4byte | BB | `R_0xd10` |  |  |
| 88 | phydm_get_cfo_info_ac | odm_get_bb_reg | BB | `R_0xd14` | 0x1fff0000 |  |
| 91 | phydm_get_cfo_info_ac | odm_read_4byte | BB | `R_0xd4c` |  |  |
| 92 | phydm_get_cfo_info_ac | odm_read_4byte | BB | `R_0xd50` |  |  |
| 93 | phydm_get_cfo_info_ac | odm_get_bb_reg | BB | `R_0xd54` | 0x1fff0000 |  |
| 289 | phydm_set_crystal_cap_reg | odm_set_mac_reg | MAC | `R_0x24` | 0x7e000000 | REG_AFE_CTRL1_8822B, REG_AFE_CTRL1 |
| 290 | phydm_set_crystal_cap_reg | odm_set_mac_reg | MAC | `R_0x28` | 0x7e | REG_AFE_CTRL2_8822B, REG_AFE_CTRL2 |
| 407 | phydm_cfo_tracking_init | odm_set_mac_reg | MAC | `R_0x10` | 0x40 | REG_SYS_SWR_CTRL1_8822B, REG_SYS_SWR_CTRL1 |

## `hal/phydm/phydm_dig.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 367 | phydm_set_big_jump_step | odm_set_bb_reg | BB | `R_0x8c8` | 0xe |  |
| 632 | odm_write_dig | odm_set_bb_reg | BB | `R_0xa0c` | 0x3f00 | ODM_REG_CCK_ANTDIV_PARA2_11N |
| 811 | phydm_dig_init | odm_get_bb_reg | BB | `R_0x8c8` | MASKLWORD |  |
| 1329 | phydm_false_alarm_counter_reg_reset | odm_set_bb_reg | BB | `R_0x9a4` | BIT(17) | ODM_REG_OFDM_FA_RST_11AC |
| 1330 | phydm_false_alarm_counter_reg_reset | odm_set_bb_reg | BB | `R_0x9a4` | BIT(17) | ODM_REG_OFDM_FA_RST_11AC |
| 1333 | phydm_false_alarm_counter_reg_reset | odm_set_bb_reg | BB | `R_0xa2c` | BIT(15) | ODM_REG_CCK_FA_RST_11N, rCCK0_FalseAlarmReport |
| 1334 | phydm_false_alarm_counter_reg_reset | odm_set_bb_reg | BB | `R_0xa2c` | BIT(15) | ODM_REG_CCK_FA_RST_11N, rCCK0_FalseAlarmReport |
| 1493 | phydm_fa_cnt_statistics_ac | odm_get_bb_reg | BB | `R_0xf48` | MASKLWORD | ODM_REG_OFDM_FA_11AC |

## `hal/halmac/halmac_88xx/halmac_usb_88xx.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 519 | usbphy_write_88xx | HALMAC_REG_W8 | USB-local | `0xff0d` |  |  |
| 520 | usbphy_write_88xx | HALMAC_REG_W8 | USB-local | `0xff0e` |  |  |
| 521 | usbphy_write_88xx | HALMAC_REG_W8 | USB-local | `0xff0c` |  |  |
| 523 | usbphy_write_88xx | HALMAC_REG_W8 | USB-local | `0xfe41` |  |  |
| 524 | usbphy_write_88xx | HALMAC_REG_W8 | USB-local | `0xfe40` |  |  |
| 525 | usbphy_write_88xx | HALMAC_REG_W8 | USB-local | `0xfe42` |  |  |

## `hal/phydm/halrf/halrf.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 87 | _iqk_page_switch | odm_write_4byte | BB | `0x1b00` |  |  |
| 333 | halrf_iqk_info_dump | odm_get_bb_reg | BB | `R_0x1bf0` | BIT(16) |  |
| 375 | halrf_iqk_info_dump | odm_read_4byte | BB | `0x1bf0` |  |  |
| 473 | halrf_iqk_dbg | odm_read_4byte | BB | `0x1bf0` |  |  |
| 481 | halrf_iqk_dbg | odm_read_4byte | BB | `0x1be8` |  |  |

## `hal/phydm/phydm_noisemonitor.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 219 | phydm_idle_noise_measure_ac | odm_set_bb_reg | BB | `R_0x9e4` | BIT(30) |  |
| 222 | phydm_idle_noise_measure_ac | odm_get_bb_reg | BB | `R_0xff0` | MASKDWORD |  |
| 225 | phydm_idle_noise_measure_ac | odm_set_bb_reg | BB | `R_0x9e4` | BIT(30) |  |
| 257 | phydm_idle_noise_measure_ac | odm_get_bb_reg | BB | `R_0xc50` | MASKBYTE0 | rA_IGI_Jaguar, ODM_REG_IGI_A_11N |
| 264 | phydm_idle_noise_measure_ac | odm_get_bb_reg | BB | `R_0xe50` | MASKBYTE0 | rB_IGI_Jaguar, ODM_REG_IGI_B_11AC |

## `hal/phydm/phydm_soml.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 318 | phydm_soml_cfo_process | odm_get_bb_reg | BB | `R_0xd10` | MASKDWORD |  |
| 319 | phydm_soml_cfo_process | odm_get_bb_reg | BB | `R_0xd14` | MASKDWORD |  |
| 320 | phydm_soml_cfo_process | odm_get_bb_reg | BB | `R_0xd50` | MASKDWORD |  |
| 321 | phydm_soml_cfo_process | odm_get_bb_reg | BB | `R_0xd54` | MASKDWORD |  |

## `hal/rtl8822b/rtl8822b_ops.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 764 | check_ips_status | rtw_read8 | MAC | `0x100` |  | REG_CR_8822B, REG_CR |
| 764 | check_ips_status | rtw_read8 | MAC | `0x86` |  |  |
| 766 | check_ips_status | rtw_read8 | MAC | `0x100` |  | REG_CR_8822B, REG_CR |
| 1635 | hw_var_set_mlme_join | rtw_write8 | MAC | `0x542` |  |  |

## `hal/rtl8822b/rtl8822b_phy.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 1521 | _sounding_config_mu | phy_query_bb_reg | BB | `0xF4C` | 0xFFFF0000 |  |
| 2149 | rtl8822b_phy_bf_init | rtw_read8 | MAC | `0x6DF` |  |  |
| 2151 | rtl8822b_phy_bf_init | rtw_write8 | MAC | `0x6DF` |  |  |
| 2153 | rtl8822b_phy_bf_init | rtw_write32 | BB | `0x1C94` |  |  |

## `hal/halmac/halmac_88xx/halmac_mimo_88xx.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 517 | mu_bfer_entry_init_88xx | HALMAC_REG_R16 | MAC | `0x1680` |  | REG_WMAC_MU_BF_CTL_8822B, REG_WMAC_MU_BF_CTL |
| 519 | mu_bfer_entry_init_88xx | HALMAC_REG_W16 | MAC | `0x1680` |  | REG_WMAC_MU_BF_CTL_8822B, REG_WMAC_MU_BF_CTL |
| 642 | mu_bfer_entry_del_88xx | HALMAC_REG_W16 | MAC | `0x1680` |  | REG_WMAC_MU_BF_CTL_8822B, REG_WMAC_MU_BF_CTL |

## `core/efuse/rtw_efuse.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 707 | hal_EfuseSwitchToBank | rtw_read32 | MAC | `0x34` |  | REG_LDO_EFUSE_CTRL_8822B, REG_LDO_EFUSE_CTRL |
| 727 | hal_EfuseSwitchToBank | rtw_write32 | MAC | `0x34` |  | REG_LDO_EFUSE_CTRL_8822B, REG_LDO_EFUSE_CTRL |

## `core/rtw_rf.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 1317 | rtw_rf_set_tx_gain_offset | rtw_hal_read_rfreg | RF target_path | `0x55` | 0xffffffff | REG_RF_TX_GAIN_OFFSET |
| 1382 | rtw_rf_set_tx_gain_offset | rtw_hal_read_rfreg | RF target_path | `0x55` | 0xffffffff | REG_RF_TX_GAIN_OFFSET |

## `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 1006 | pre_init_system_cfg_8822b | HALMAC_REG_W8 | USB-local | `0xFE5B` |  | REG_USB_DMA_AGG_TO |
| 1006 | pre_init_system_cfg_8822b | HALMAC_REG_R8 | USB-local | `0xFE5B` |  | REG_USB_DMA_AGG_TO |

## `hal/halmac/halmac_88xx/halmac_8822b/halmac_usb_8822b.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 86 | mac_pwr_switch_usb_8822b | HALMAC_REG_R8 | USB-local | `0xFE58` |  | REG_USB_HRPWM_8822B |
| 92 | mac_pwr_switch_usb_8822b | HALMAC_REG_W8 | USB-local | `0xFE58` |  | REG_USB_HRPWM_8822B |

## `hal/phydm/phydm_auto_dbg.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 397 | phydm_dbg_port_dump_ac | odm_get_bb_reg | BB | `R_0xf80` | MASKDWORD |  |
| 402 | phydm_dbg_port_dump_ac | odm_set_bb_reg | BB | `R_0x198c` | BIT(2) \| BIT(1) \| BIT(0) |  |

## `core/rtw_pwrctrl.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 185 | _ips_leave | rtw_read32 | MAC | `0x4c` |  | REG_LED_CFG_8822B, REG_LEDCFG0 |

## `core/rtw_wlan_util.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 772 | set_channel_bwmode | rtw_hal_read_rfreg | RF 0 | `0x18` | 0xffffffff | ODM_CHANNEL |

## `hal/phydm/halrf/halrf_powertracking_ce.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 664 | get_swing_index | odm_get_bb_reg | BB | `R_0xc1c` | 0xFFE00000 | rA_TxScale_Jaguar, REG_A_TX_SCALE_JAGUAR |

## `hal/phydm/phydm_cck_pd.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 75 | phydm_write_cck_pd_type1 | odm_write_1byte | BB | `R_0xa0a` |  | ODM_CCK_PD_THRESH, ODM_REG_CCK_CCA_11N |

## `os_dep/linux/rtw_proc.c`

| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |
|---:|---|---|---|---|---|---|
| 2134 | proc_get_sec_cam | rtw_read16 | MAC | `0x680` |  | REG_SECCFG_8822B, REG_SECCFG |

