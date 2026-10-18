# Phụ lục A10 — USB-local 0xFE00–0xFFFF

Sinh tự động bởi `tools/appendix.py` từ `work/*.json` [TOOL]. Mỗi mục liệt kê **mọi** truy cập tìm thấy trong code active (kể cả nhánh chết, có ghi lý do) và mọi mục bảng. Các thanh ghi St=D/Z (không dùng) chỉ có trong `04*_catalog_*.md`. Cột "Trạng thái": `live` = chạy được với RTL8822BU; còn lại là lý do loại.

Tổng: 14 địa chỉ.

## `0xFE11` — St A — `REG_USB_USBSTAT`

Định nghĩa: `REG_USB_USBSTAT hal/halmac/halmac_reg2.h:8558`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c:91` | init_usb_cfg_88xx | HALMAC_REG_R8 | 8 |  | live | `if ((HALMAC_REG_R8(REG_USB_USBSTAT) & 0x3) == 0x1)/* usb2.0 */` |

## `0xFE40` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c:524` | usbphy_write_88xx | HALMAC_REG_W8 | 8 |  | live | `HALMAC_REG_W8(0xfe40, addr);` |
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c:547` | usbphy_read_88xx | HALMAC_REG_W8 | 8 |  | no-caller | `HALMAC_REG_W8(0xfe40, addr);` |

## `0xFE41` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c:523` | usbphy_write_88xx | HALMAC_REG_W8 | 8 |  | live | `HALMAC_REG_W8(0xfe41, (u8)data);` |

## `0xFE42` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c:525` | usbphy_write_88xx | HALMAC_REG_W8 | 8 |  | live | `HALMAC_REG_W8(0xfe42, 0x81);` |
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c:548` | usbphy_read_88xx | HALMAC_REG_W8 | 8 |  | no-caller | `HALMAC_REG_W8(0xfe42, 0x81);` |

## `0xFE43` — St X — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c:549` | usbphy_read_88xx | HALMAC_REG_R8 | 8 |  | no-caller | `value = HALMAC_REG_R8(0xfe43);` |

## `0xFE57` — St A — `REG_USB_HCPWM_8822B`

Định nghĩa: `REG_USB_HCPWM_8822B hal/rtl8822b/usb/rtl8822bu.h:67`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/rtl8822b/usb/rtl8822bu_ops.c:175` | gethwreg | rtw_read8 | 8 |  | live | `*val = rtw_read8(padapter, REG_USB_HCPWM_8822B);` |

## `0xFE58` — St A — `REG_USB_HRPWM_8822B`

Định nghĩa: `REG_USB_HRPWM_8822B hal/rtl8822b/usb/rtl8822bu.h:66`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/halmac/halmac_88xx/halmac_8822b/halmac_usb_8822b.c:86` | mac_pwr_switch_usb_8822b | HALMAC_REG_R8 | 8 |  | live | `adapter->rpwm = HALMAC_REG_R8(0xFE58);` |
| `hal/halmac/halmac_88xx/halmac_8822b/halmac_usb_8822b.c:92` | mac_pwr_switch_usb_8822b | HALMAC_REG_W8 | 8 |  | live | `HALMAC_REG_W8(0xFE58, rpwm);` |
| `hal/rtl8822b/usb/rtl8822bu_ops.c:136` | sethwreg | rtw_write8 | 8 |  | live | `rtw_write8(padapter, REG_USB_HRPWM_8822B, ps_state);` |
| `hal/rtl8822b/usb/rtl8822bu_ops.c:181` | gethwreg | rtw_read8 | 8 |  | live | `*val = rtw_read8(padapter, REG_USB_HRPWM_8822B);` |

## `0xFE5B` — St A — `REG_USB_DMA_AGG_TO`

Định nghĩa: `REG_USB_DMA_AGG_TO hal/halmac/halmac_reg2.h:8582`

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:1006` | pre_init_system_cfg_8822b | HALMAC_REG_W8 | 8 |  | live | `HALMAC_REG_W8(0xFE5B, HALMAC_REG_R8(0xFE5B) \| BIT(4));` |
| `hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:1006` | pre_init_system_cfg_8822b | HALMAC_REG_R8 | 8 |  | live | `HALMAC_REG_W8(0xFE5B, HALMAC_REG_R8(0xFE5B) \| BIT(4));` |

## `0xFF0A` — St A — *(không có macro)*

- Bảng `TRANS_CARDEMU_TO_ACT_8822B` (`halmac_pwr_seq_8822b.c:103`): WRITE msk=0xFF val=0 intf=USB_MSK @103

## `0xFF0B` — St A — *(không có macro)*

- Bảng `TRANS_CARDEMU_TO_ACT_8822B` (`halmac_pwr_seq_8822b.c:108`): WRITE msk=0xFF val=0 intf=USB_MSK @108

## `0xFF0C` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c:521` | usbphy_write_88xx | HALMAC_REG_W8 | 8 |  | live | `HALMAC_REG_W8(0xff0c, addr \| BIT(7));` |
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c:541` | usbphy_read_88xx | HALMAC_REG_W8 | 8 |  | no-caller | `HALMAC_REG_W8(0xff0c, addr \| BIT(6));` |
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c:542` | usbphy_read_88xx | HALMAC_REG_R32 | 32 |  | no-caller | `value = (u16)(HALMAC_REG_R32(0xff0c) >> 8);` |

## `0xFF0D` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c:519` | usbphy_write_88xx | HALMAC_REG_W8 | 8 |  | live | `HALMAC_REG_W8(0xff0d, (u8)data);` |

## `0xFF0E` — St A — *(không có macro)*

| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |
|---|---|---|---|---|---|---|
| `hal/halmac/halmac_88xx/halmac_usb_88xx.c:520` | usbphy_write_88xx | HALMAC_REG_W8 | 8 |  | live | `HALMAC_REG_W8(0xff0e, (u8)(data >> 8));` |

## `0xFF1A` — St A — *(không có macro)*

- Bảng `TRANS_CARDEMU_TO_ACT_8822B` (`halmac_pwr_seq_8822b.c:158`): WRITE msk=0xFF val=0 intf=USB_MSK @158
- Bảng `TRANS_ACT_TO_CARDEMU_8822B` (`halmac_pwr_seq_8822b.c:267`): WRITE msk=0xFF val=0x30 intf=USB_MSK @267

