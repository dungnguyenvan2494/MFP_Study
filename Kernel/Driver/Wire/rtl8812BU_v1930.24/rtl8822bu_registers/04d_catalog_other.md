# Danh mục không gian khác: USB-local, USB3-PHY, SDIO-local

USB-local `0xFE00–0xFFFF` (khối USB của chip, `usbctrl_vendorreq` coi là REG_LOCAL_SEC); USB3-PHY: không gian PHY gián tiếp qua `usbphy_write_88xx` (`0xFF0C–0xFF0E`); SDIO-local `0x1025xxxx`: chỉ định nghĩa, không dùng được qua USB.

Tổng: **106 địa chỉ** — A=14, X=6, R=4, D=82

Cột: **St** = trạng thái (A/T/M/G/X/R/Z/D, xem `04_catalog_README.md`); **W** = độ rộng truy cập quan sát được trong code/bảng (không phải độ rộng phần cứng); **Truy cập** = live/MP/dbg/dead/bảng; **Bit** = số macro bit/field định nghĩa trong section của thanh ghi / số được dùng; **Nhóm** = [INFERENCE] suy từ tên hàm truy cập.

| STT | Địa chỉ | Tên (alias) | Không gian | St | Nhóm | W | Kiểu | Truy cập L/M/G/X/T | Bit d/u | Hàm (≤4) | Vị trí (≤3) |
|---:|---|---|---|:-:|---|---|---|---|---|---|---|
| 1 | `0x0040` | *(không có macro)* | SDIO-local | X | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDEMU_TO_CARDDIS_8822B |
| 2 | `0x0041` | *(không có macro)* | SDIO-local | X | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDEMU_TO_CARDDIS_8822B |
| 3 | `0x0042` | *(không có macro)* | SDIO-local | X | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDEMU_TO_CARDDIS_8822B |
| 4 | `0x0044` | *(không có macro)* | SDIO-local | X | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDEMU_TO_CARDDIS_8822B |
| 5 | `0x0086` | *(không có macro)* | SDIO-local | X | Power/PS |  |  | 0/0/0/0/4 | 0/0 |  | bảng: TRANS_CARDDIS_TO_CARDEMU_8822B, TRANS_CARDEMU_TO_CARDDIS_8822B |
| 6 | `0x10250000` | REG_SDIO_TX_CTRL_8822B (+1: REG_SDIO_TX_CTRL) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 34/0 |  |  |
| 7 | `0x10250014` | REG_SDIO_HIMR_8822B (+1: REG_SDIO_HIMR) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 48/0 |  |  |
| 8 | `0x10250018` | REG_SDIO_HISR_8822B (+1: REG_SDIO_HISR) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 48/0 |  |  |
| 9 | `0x1025001C` | REG_SDIO_RX_REQ_LEN_8822B (+1: REG_SDIO_RX_REQ_LEN) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 10 | `0x1025001F` | REG_SDIO_FREE_TXPG_SEQ_V1_8822B (+1: REG_SDIO_FREE_TXPG_SEQ_V1) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 11 | `0x10250020` | REG_SDIO_FREE_TXPG_8822B | SDIO-local | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 12 | `0x10250024` | REG_SDIO_FREE_TXPG2_8822B (+1: REG_SDIO_FREE_TXPG2) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 13 | `0x10250028` | REG_SDIO_OQT_FREE_TXPG_V1_8822B (+1: REG_SDIO_OQT_FREE_TXPG_V1) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 42/0 |  |  |
| 14 | `0x10250030` | REG_SDIO_HTSFR_INFO_8822B (+1: REG_SDIO_HTSFR_INFO) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 28/0 |  |  |
| 15 | `0x10250038` | REG_SDIO_HCPWM1_V2_8822B (+1: REG_SDIO_HCPWM1_V2) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 2/0 |  |  |
| 16 | `0x1025003A` | REG_SDIO_HCPWM2_V2_8822B (+1: REG_SDIO_HCPWM2_V2) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 17 | `0x10250040` | REG_SDIO_INDIRECT_REG_CFG_8822B (+1: REG_SDIO_INDIRECT_REG_CFG) | SDIO-local | R |  |  |  | 0/0/0/0/0 | 34/0 |  |  |
| 18 | `0x10250044` | REG_SDIO_INDIRECT_REG_DATA_8822B (+1: REG_SDIO_INDIRECT_REG_DATA) | SDIO-local | R |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 19 | `0x10250060` | REG_SDIO_H2C_8822B (+1: REG_SDIO_H2C) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 20 | `0x10250064` | REG_SDIO_C2H_8822B (+1: REG_SDIO_C2H) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 21 | `0x10250080` | REG_SDIO_HRPWM1_8822B (+1: REG_SDIO_HRPWM1) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 4/1 |  |  |
| 22 | `0x10250082` | REG_SDIO_HRPWM2_8822B (+1: REG_SDIO_HRPWM2) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 23 | `0x10250084` | REG_SDIO_HPS_CLKR_8822B (+1: REG_SDIO_HPS_CLKR) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 24 | `0x10250085` | REG_SDIO_BUS_CTRL_8822B (+1: REG_SDIO_BUS_CTRL) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 8/0 |  |  |
| 25 | `0x10250086` | REG_SDIO_HSUS_CTRL_8822B (+1: REG_SDIO_HSUS_CTRL) | SDIO-local | R |  |  |  | 0/0/0/0/0 | 10/0 |  |  |
| 26 | `0x10250088` | REG_SDIO_RESPONSE_TIMER_8822B (+1: REG_SDIO_RESPONSE_TIMER) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 27 | `0x1025008A` | REG_SDIO_CMD_CRC_8822B (+1: REG_SDIO_CMD_CRC) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 28 | `0x10250090` | REG_SDIO_HSISR_8822B (+1: REG_SDIO_HSISR) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 4/0 |  |  |
| 29 | `0x102500C0` | REG_SDIO_ERR_RPT_8822B (+1: REG_SDIO_ERR_RPT) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 21/0 |  |  |
| 30 | `0x102500C2` | REG_SDIO_CMD_ERRCNT_8822B (+1: REG_SDIO_CMD_ERRCNT) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 31 | `0x102500C3` | REG_SDIO_DATA_ERRCNT_8822B (+1: REG_SDIO_DATA_ERRCNT) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 32 | `0x102500C4` | REG_SDIO_CMD_ERR_CONTENT_8822B (+1: REG_SDIO_CMD_ERR_CONTENT) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 33 | `0x102500C9` | REG_SDIO_CRC_ERR_IDX_8822B (+1: REG_SDIO_CRC_ERR_IDX) | SDIO-local | R |  |  |  | 0/0/0/0/0 | 10/0 |  |  |
| 34 | `0x102500CA` | REG_SDIO_DATA_CRC_8822B (+1: REG_SDIO_DATA_CRC) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 14/0 |  |  |
| 35 | `0x102500CB` | REG_SDIO_DATA_REPLY_TIME_8822B (+1: REG_SDIO_DATA_REPLY_TIME) | SDIO-local | D |  |  |  | 0/0/0/0/0 | 7/0 |  |  |
| 36 | `0xFE00` | REG_USB_SETUP_DEC_INT | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 37 | `0xFE01` | REG_USB_DMACTL | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 38 | `0xFE02` | REG_USB_IRQSTAT0 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 39 | `0xFE03` | REG_USB_IRQSTAT1 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 40 | `0xFE04` | REG_USB_IRQEN0 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 41 | `0xFE05` | REG_USB_IRQEN1 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 42 | `0xFE06` | REG_USB_AUTOPTRL | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 43 | `0xFE07` | REG_USB_AUTOPTRH | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 44 | `0xFE08` | REG_USB_AUTODAT | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 45 | `0xFE09` | REG_USB_SCRATCH0 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 46 | `0xFE0A` | REG_USB_SCRATCH1 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 47 | `0xFE0B` | REG_USB_SEEPROM | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 48 | `0xFE0C` | REG_USB_GPIO0 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 49 | `0xFE0D` | REG_USB_GPIO0DIR | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 50 | `0xFE0E` | REG_USB_CLKSEL | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 51 | `0xFE0F` | REG_USB_BOOTCTL | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 52 | `0xFE10` | REG_USB_USBCTL | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 53 | `0xFE11` | REG_USB_USBSTAT | USB-local | A | USB | 8 | R | 1/0/0/0/0 | 0/0 | init_usb_cfg_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:91 |
| 54 | `0xFE12` | REG_USB_DEVADDR | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 55 | `0xFE13` | REG_USB_USBTEST | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 56 | `0xFE14` | REG_USB_FNUM0 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 57 | `0xFE15` | REG_USB_FNUM1 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 58 | `0xFE17` | REG_USB_INFO | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 59 | `0xFE20` | REG_USB_EP_IDX | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 60 | `0xFE21` | REG_USB_EP_CFG | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 61 | `0xFE22` | REG_USB_EP_CTL | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 62 | `0xFE23` | REG_USB_EP_STAT | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 63 | `0xFE24` | REG_USB_EP_IRQ | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 64 | `0xFE25` | REG_USB_EP_IRQEN | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 65 | `0xFE26` | REG_USB_EP_MAXPKT0 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 66 | `0xFE27` | REG_USB_EP_MAXPKT1 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 67 | `0xFE28` | REG_USB_EP_DAT | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 68 | `0xFE29` | REG_USB_EP_BC0 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 69 | `0xFE2A` | REG_USB_EP_BC1 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 70 | `0xFE2B` | REG_USB_EP_TC0 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 71 | `0xFE2C` | REG_USB_EP_TC1 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 72 | `0xFE2D` | REG_USB_EP_TC2 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 73 | `0xFE2E` | REG_USB_EP_CTL2 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 74 | `0xFE40` | *(không có macro)* | USB-local | A | USB | 8 | W | 1/0/0/1/0 | 0/0 | usbphy_write_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:524 |
| 75 | `0xFE41` | *(không có macro)* | USB-local | A | USB | 8 | W | 1/0/0/0/0 | 0/0 | usbphy_write_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:523 |
| 76 | `0xFE42` | *(không có macro)* | USB-local | A | USB | 8 | W | 1/0/0/1/0 | 0/0 | usbphy_write_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:525 |
| 77 | `0xFE43` | *(không có macro)* | USB-local | X | USB | 8 | R | 0/0/0/1/0 | 0/0 | usbphy_read_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:549 |
| 78 | `0xFE4F` | REG_USB_ALTERNATE_SETTING | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 79 | `0xFE55` | REG_USB_SPECIAL_OPTION | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 80 | `0xFE57` | REG_USB_HCPWM_8822B | USB-local | A |  | 8 | R | 1/0/0/0/0 | 0/0 | gethwreg | hal/rtl8822b/usb/rtl8822bu_ops.c:175 |
| 81 | `0xFE58` | REG_USB_HRPWM_8822B | USB-local | A | Power/PS | 8 | R W | 4/0/0/0/0 | 0/0 | gethwreg, mac_pwr_switch_usb_8822b, sethwreg | hal/halmac/halmac_88xx/halmac_8822b/halmac_usb_8822b.c:86, hal/halmac/halmac_88xx/halmac_8822b/halmac_usb_8822b.c:92, hal/rtl8822b/usb/rtl8822bu_ops.c:136 … |
| 82 | `0xFE5B` | REG_USB_DMA_AGG_TO | USB-local | A | System/Clock/Reset | 8 | R W | 2/0/0/0/0 | 0/0 | pre_init_system_cfg_8822b | hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:1006 |
| 83 | `0xFE5C` | REG_USB_AGG_TO | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 84 | `0xFE5D` | REG_USB_AGG_TH | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 85 | `0xFE60` | REG_USB_VID | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 86 | `0xFE62` | REG_USB_PID | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 87 | `0xFE64` | REG_USB_OPT | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 88 | `0xFE65` | REG_USB_CONFIG | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 89 | `0xFE68` | REG_USB_PHY_PARA1 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 90 | `0xFE69` | REG_USB_PHY_PARA2 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 91 | `0xFE6A` | REG_USB_PHY_PARA3 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 92 | `0xFE6B` | REG_USB_PHY_PARA4 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 93 | `0xFE6C` | REG_USB_OPT2 | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 94 | `0xFE6D` | REG_USB_GPS_EP_CONFIG | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 95 | `0xFE6E` | REG_USB_INT_BINTERVAL | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 96 | `0xFE70` | REG_USB_MAC_ADDR | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 97 | `0xFE80` | REG_USB_MANUFACTURE_SETTING | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 98 | `0xFEA0` | REG_USB_PRODUCT_STRING | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 99 | `0xFED0` | REG_USB_SERIAL_NUMBER_STRING | USB-local | D |  |  |  | 0/0/0/0/0 | 0/0 |  |  |
| 100 | `0xFF0A` | *(không có macro)* | USB-local | A | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDEMU_TO_ACT_8822B |
| 101 | `0xFF0B` | *(không có macro)* | USB-local | A | Power/PS |  |  | 0/0/0/0/1 | 0/0 |  | bảng: TRANS_CARDEMU_TO_ACT_8822B |
| 102 | `0xFF0C` | *(không có macro)* | USB-local | A | USB | 8/32 | R W | 1/0/0/2/0 | 0/0 | usbphy_write_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:521 |
| 103 | `0xFF0D` | *(không có macro)* | USB-local | A | USB | 8 | W | 1/0/0/0/0 | 0/0 | usbphy_write_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:519 |
| 104 | `0xFF0E` | *(không có macro)* | USB-local | A | USB | 8 | W | 1/0/0/0/0 | 0/0 | usbphy_write_88xx | hal/halmac/halmac_88xx/halmac_usb_88xx.c:520 |
| 105 | `0xFF1A` | *(không có macro)* | USB-local | A | Power/PS |  |  | 0/0/0/0/2 | 0/0 |  | bảng: TRANS_ACT_TO_CARDEMU_8822B, TRANS_CARDEMU_TO_ACT_8822B |
| 106 | `0x0001` | *(không có macro)* | USB3-PHY | A | USB |  |  | 0/0/0/0/1 | 0/0 |  | bảng: usb3_phy_param_8822b |
