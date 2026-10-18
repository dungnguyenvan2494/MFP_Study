# A13 — Rà soát tay 44 mục trạng thái **R** (được tham chiếu nhưng không làm đối số địa chỉ của accessor)

> Đầu vào: `tools/r_review.py <driver> work work/r_review.md`, liệt kê mọi tham chiếu trên dòng active. Mỗi mục bên dưới đã được đọc tay tại vị trí trích. Phân loại:
> - **R-off**: dùng "base + hằng"; truy cập thật nằm ở địa chỉ byte/word khác, đã có trong danh mục với trạng thái riêng.
> - **R-var**: địa chỉ được gán vào biến, bảng hoặc struct rồi mới truy cập. Địa chỉ **có** được truy cập thật nếu nhánh còn sống.
> - **R-dead**: tham chiếu chỉ ở nhánh không chạy với 8822B/USB, hoặc giá trị không bao giờ được đọc.

## 1. Kết quả

| Không gian | Địa chỉ | Tên | Loại | Bằng chứng (vị trí tham chiếu) | Kết luận |
|---|---|---|---|---|---|
| MAC | `0x0044` | `REG_GPIO_PIN_CTRL` | R-off | `hal/hal_com.c:4686-4749` (`+1..+3`, `rtw_hal_set_output/input_gpio`) | truy cập thật ở `0x45–0x47` (05.11) |
| MAC | `0x0060` | `REG_GPIO_EXT_CTRL` (= legacy `REG_GPIO_PIN_CTRL_2`) | R-off | `hal_com.c:4712-4767` (`+1..+3`) | `0x61–0x63` (05.11) |
| MAC | `0x00CC` | `REG_PMC_DBG_CTRL2` | R-off | `halmac_efuse_88xx.c:1062,1089` (`+3`, ghi `unlock_code`/0) | `0xCF` = khoá mở ghi eFuse (05.12) |
| MAC | `0x00FC` | `REG_SYS_CFG2` | R-off | `halmac_usb_88xx.c:87,162,471`, `halmac_init_8822b.c:1005` (`+3 == 0x20`); `halmac_api.c:556-560` (đọc chip ID) | `0xFF` = phát hiện USB3 (05.09); `0xFC` đọc chip ID qua `pltfm_api->REG_READ_8` lúc `halmac_init_adapter` (`halmac_api.c:560`) → **đã sửa**: thêm accessor, nay `0xFC` là A (và `0xF1` — chip version `>> 4` — được thêm vào danh mục) |
| MAC | `0x0114` | `REG_TRXFF_BNDY` | R-off | `halmac_cfg_wmac_8822b.c:121,125` (`+1`) | `0x115` (05.04) |
| MAC | `0x01D0` | `REG_HMEBOX0` | R-var | `hal/hal_halmac.c:4135` `msgbox_addr = REG_HMEBOX0 + box*4` | truy cập thật `0x1D0–0x1DC` (mailbox H2C — 05.08) |
| MAC | `0x01F0` | `REG_HMEBOX_E0` | R-var | `hal_halmac.c:4129` | `0x1F0–0x1FC` (05.08) |
| MAC | `0x04D0` | `REG_MACID_SLEEP2` | R-var | `rtl8822b_halinit.c:105` (bảng `rtw_macid_ctl_init_sleep_reg`) | một trong 4 địa chỉ `REG_MACID_SLEEP`, `SLEEP1`, `SLEEP2`, `SLEEP3` đưa vào `rtw_macid_ctl_init_sleep_reg` (`rtl8822b_halinit.c:102-106`); truy cập về sau qua bảng địa chỉ đó (sleep bitmap theo macid) |
| MAC | `0x04D8–0x04DE` | `REG_HW_SEQ0..3` | R-var | `rtl8822b_ops.c:2211-2258` `reg_hw_ssn = REG_HW_SEQn` (`rtl8822b_sethwreg`) | HW sequence number theo AC (theo tên) — truy cập qua biến |
| MAC | `0x0618`, `0x0708`, `0x1628`, `0x1638`, `0x1648` | `REG_BSSID`, `REG_BSSID1..4` | R-var | bảng `port_cfg[]` (`rtl8822b_ops.c:77-137` `.bssid`), `halmac_cfg_wmac_88xx.c:152-164` `offset = REG_BSSIDn` | ghi qua `cfg_bssid_88xx` (05.02 §5) |
| MAC | `0x16A0–0x16C0` | `REG_TRANSMIT_ADDRSS_0..4` | R-var | `port_cfg[]` `.ta` (`rtl8822b_ops.c:85-145`), `halmac_cfg_wmac_88xx.c:202-214` | TA theo port (`cfg_transmitter_addr_88xx`) |
| MAC | `0x06A8`, `0x0710`, `0x1600–0x1608` | `REG_BCN_PSR_RPT`, `…RPT1..4` | R-var | `port_cfg[]` `.ps_aid` (`rtl8822b_ops.c:84-144`), `hal_halmac.c:2399-2427` (`rtw_halmac_set_aid`) | AID cho PS-Poll/beacon theo port (05.10) |
| MAC | `0x0650` | `REG_NAV_CTRL` | R-off | `rtl8822b_ops.c:2345` (`+2`, `HW_VAR_NAV_UPPER`) | `0x652` (05.17 §5) |
| MAC | `0x07C4`, `0x07C8` | `R_0x7c4`/`R_0x7c8` — **tên chung `REG_LTECOEX_WRITE/READ_DATA` (halmac_reg2.h)** | R-var | `phydm_adc_sampling.c:935-936` (`reg2/reg3`, nhánh không phải 8192F) | LA mode (debug — 05.17 §3). **Va chạm tên**: header chung (nhiều chip) đặt LTE-coex ở `0x7C0–0x7C8`, nhưng 8822B dùng `0x1700–0x1708` cho LTE-coex và `0x7C0` = `REG_IQ_DUMP_8822B` (09 mục 25) |
| MAC | `0x07F4`, `0x07F8` | `R_0x7f4`, `REG_RTS_ADDRESS_1`/`R_0x7f8` | R-dead | `phydm_adc_sampling.c:931-932` — nhánh `ODM_RTL8192F` | không truy cập với 8822B |
| MAC | `0x07D0` | `REG_WMAC_OPTION_FUNCTION` | R-off | `halmac_cfg_wmac_8822b.c:135,141` (`+4`), `halmac_init_8822b.c:962,969` (`+8`, `+4`) | `0x7D4`, `0x7D8` (05.17 §5) |
| MAC | `0x10C0` | `REG_SW_MDIO` | R-off | `halmac_usb_88xx.c:490`, `halmac_usb_8822b.c:131` (`+3` BIT0) | `0x10C3` (05.09) |
| MAC | `0x111C` | `REG_RXFF_WTR_V1` | R-var | `halmac_common_88xx.c:1413-1447` (`i = …` trong vòng dump) | chỉ trong hàm dump (debug) |
| BB | `0x0D08` | `rA_SIRead_Jaguar` | R-dead | `rtl8822b_phy.c:93` gán `PHYRegDef[A].rfLSSIReadBack` — **trường này không được đọc ở đâu** (grep) | 8822B đọc RF qua cửa sổ `0x2800` (14 §1) |
| BB | `0x1A9C` | `R_0x1a9c` | R-dead | `phydm.c:163-165` — nhánh 8198F/8822C/8814B/8197G; 8822B dùng `0xA9C` | |
| RF-direct | `0x2D90` | `R_0x2d90` | R-dead | `phydm_psd.c:394-396` — nhánh `ODM_IC_JGR3_SERIES` | |
| other | `0x3A00` | `R_0x3a00` | R-dead | `phydm_api.c:1999-2002` — hàm trả về ngay nếu không phải JGR3 | |
| other | `0x8000` | `R_0x8000` | R-off | `phydm_adc_sampling.c:816-817` (`+ (addr & 0xFFF)`); `halmac_common_88xx.c:2206` (vòng `0x8000…0x8FFF`) | **cửa sổ packet buffer** — không gian gián tiếp (02 §2.2) |
| SDIO-local | `0x10250040/44/86/C9` | `REG_SDIO_*` | R-dead | `halmac_api.c:542-632`, `halmac_common_88xx.c:1405` — đường SDIO (`pltfm_reg_w8_sdio`, `PLTFM_SDIO_CMD52_R`) | không áp dụng cho USB |

Tổng hợp 44 mục:

| Loại | Số mục |
|---|---|
| R-off | 9 |
| R-var | 25 (gồm 15 thanh ghi theo port: BSSID/TA/AID) |
| R-dead | 10 (4 SDIO + 6 nhánh IC khác / trường không đọc) |

## 2. Hệ quả cho công cụ

- **R-off và R-var là "dùng thật"**, chỉ khác cách lấy địa chỉ. Muốn đếm số thanh ghi được chạm thì cộng R-var vào A. R-off đã được tính ở địa chỉ byte thực.
- `halmac_api.c:556-562` đọc `REG_SYS_CFG2` và `REG_SYS_CFG1 + 1` qua `pltfm_api->REG_READ_8(drv_adapter, …)` ngay trong `halmac_init_adapter` (trước khi `api` sẵn sàng). Phiên 3 đã thêm `REG_READ_8/16/32` và `REG_WRITE_8/16/32` vào bảng ACC của `inventory.py`, nên số mục R giảm từ 44 xuống 43; bảng trên giữ nguyên để truy vết.
- Ngoài thay đổi `0xFC`/`0xF1`, danh mục chính (04a–04d) không đổi. Bảng trên là lớp chú thích bổ sung.
