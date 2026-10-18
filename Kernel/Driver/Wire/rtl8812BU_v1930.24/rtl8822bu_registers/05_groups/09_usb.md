# 05.09 — Giao tiếp USB: thanh ghi khối USB, chế độ U2/U3, USB PHY, DMA/aggregation

> Nhãn như 05.01. Cơ chế vendor request (cách *mọi* thanh ghi đi qua USB) đã phân tích ở `03_access_architecture.md`; ở đây là các thanh ghi **cấu hình chính khối USB** của chip.

## 1. Thanh ghi

| Địa chỉ | Tên | St | Field có bằng chứng | Hàm |
|---|---|:-:|---|---|
| `0x00FF` | B3 của `REG_SYS_CFG2_8822B` (0xFC) | A | cả byte: `== 0x20` ⇔ đang chạy **USB 3.0** (header không đặt tên bit) | `init_usb_cfg_88xx`, `cfg_usb_rx_agg_88xx`, `set_usb_mode_88xx`, `pre_init_system_cfg_8822b` |
| `0x00C4–0x00C7` | `REG_PAD_CTRL2_8822B` | A | `USB23_SW_MODE_V1`, `USB3_USB2_TRANSITION`, `RSM_EN_V1[16]`, `NO_PDN_CHIPOFF_V1[17]`, `MATCH_CNT[15:8]` | `set_usb_mode_88xx` |
| `0x0004` | `REG_SYS_PW_CTRL.APFM_OFFMAC[9]` | A | yêu cầu tắt MAC khi đổi U2/U3 | `set_usb_mode_88xx` |
| `0x10C3` | `REG_SW_MDIO.DIS_TIMEOUT_IO[24]` | A | xoá trước khi đổi U2/U3 | `set_usb_mode_88xx` (chỉ 8822B/8821C) |
| `0xFE11` | `REG_USB_USBSTAT` | A | `& 0x3 == 1` ⇔ USB 2.0 high-speed, khác ⇔ full-speed | `init_usb_cfg_88xx` |
| `0xFE40–0xFE43` | (không tên) | A | cửa sổ **USB2-PHY**: `0xFE40`=địa chỉ, `0xFE41`=dữ liệu ghi, `0xFE42`=lệnh (`0x81`), `0xFE43`=dữ liệu đọc | `usbphy_write/read_88xx` |
| `0xFF0C–0xFF0E` | (không tên) | A | cửa sổ **USB3-PHY**: `0xFF0D/0E`=dữ liệu 16-bit, `0xFF0C`=địa chỉ \| `BIT7` (ghi) / \| `BIT6` (đọc), đọc trả về `R32(0xFF0C) >> 8` | `usbphy_write/read_88xx` |
| `0xFF0A`, `0xFF0B`, `0xFF1A` | (không tên) | A | chỉ trong power-seq (05.01 §4–5) | — |
| `0xFE57` / `0xFE58` | `REG_USB_HCPWM_8822B` / `REG_USB_HRPWM_8822B` (`hal/rtl8822b/usb/rtl8822bu.h:66-67`) | A | bắt tay nguồn host↔FW: `TOGGLE[7]`, `ACK[6]`, `REQ_PS[0]` (tên mượn từ section SDIO HRPWM1, `halmac_bit_8822b.h:17766-17769`) | `sethwreg(HW_VAR_SET_RPWM)`, `gethwreg(HW_VAR_CPWM/RPWM_TOG)`, `mac_pwr_switch_usb_8822b` → chi tiết ở 10_power_mgmt |
| `0xFE5B` | `REG_USB_DMA_AGG_TO` | A | `\|= BIT4` khi U3 | `pre_init_system_cfg_8822b` |
| `0x0290` | `REG_RXDMA_MODE_8822B` | A | `DMA_MODE[1]`, `BURST_CNT[3:2]`, `BURST_SIZE[5:4]` | `init_usb_cfg_88xx` |
| `0x020C–0x020D` | `REG_TXDMA_OFFSET_CHK_8822B` | A | `DROP_DATA_EN[9]` | `init_usb_cfg_88xx` |
| `0x0280–0x0283` | `REG_RXDMA_AGG_PG_TH_8822B` | A | `RXDMA_AGG_PG_TH[7:0]` (size), `DMA_AGG_TO_V1[15:8]` (timeout), `EN_PRE_CALC[29]`, `USB_RXDMA_AGG_EN[31]` | `cfg_usb_rx_agg_88xx` |
| `0x010C` bit 2 | `REG_TXDMA_PQ_MAP.RXDMA_AGG_EN[2]` | A | bật RX aggregation | `cfg_usb_rx_agg_88xx` |
| (USB3-PHY) `0x01` | — | A* | `0xA841`, chỉ **D-cut** (`usb3_phy_param_8822b`, `halmac_phy_8822b.c:90-97`) | `phy_cfg_usb_8822b` → `parse_intf_phy_88xx` |

Có ~56 tên `REG_USB_*` khác (0xFE00–0xFED0: `REG_USB_VID`, `REG_USB_PID`, `REG_USB_MAC_ADDR`, `REG_USB_EP_*`…) chỉ được định nghĩa, không dùng (St D) — xem `04d_catalog_other.md`.

## 2. Nhận biết tốc độ USB — 2 thanh ghi khác nhau

```c
/* halmac_usb_88xx.c:85-97 — init_usb_cfg_88xx() */
value8 |= (BIT_DMA_MODE | (0x3 << BIT_SHIFT_BURST_CNT));       /* DMA_MODE=1, BURST_CNT=3 */
if (HALMAC_REG_R8(REG_SYS_CFG2 + 3) == 0x20)                   /* USB 3.0 */
	value8 |= (USB_BURST_SIZE_3_0 << BIT_SHIFT_BURST_SIZE);  /* 0 */
else if ((HALMAC_REG_R8(REG_USB_USBSTAT) & 0x3) == 0x1)        /* USB 2.0 HS */
	value8 |= USB_BURST_SIZE_2_0_HS << BIT_SHIFT_BURST_SIZE;  /* 1 */
else                                                           /* USB 1.1 */
	value8 |= USB_BURST_SIZE_2_0_FS << BIT_SHIFT_BURST_SIZE;  /* 2 */
HALMAC_REG_W8(REG_RXDMA_MODE, value8);
HALMAC_REG_W16_SET(REG_TXDMA_OFFSET_CHK, BIT_DROP_DATA_EN);
```

Tính giá trị ghi vào `0x0290` [tính tay, `BIT_DMA_MODE = BIT(1)`, `BIT_SHIFT_BURST_CNT = 2`, `BIT_SHIFT_BURST_SIZE = 4`, enum `usb_burst_size` `halmac_usb_88xx.c:62-68`]:

| Tốc độ | `DMA_MODE` | `BURST_CNT` | `BURST_SIZE` | Byte ghi |
|---|---|---|---|---|
| USB 3.0 | `0x02` | `0x3<<2 = 0x0C` | `0<<4 = 0x00` | **`0x0E`** |
| USB 2.0 HS | `0x02` | `0x0C` | `1<<4 = 0x10` | **`0x1E`** |
| USB 1.1 FS | `0x02` | `0x0C` | `2<<4 = 0x20` | **`0x2E`** |

Lưu ý ghi **thẳng** (không RMW): các bit khác của byte 0x290 bị xoá về 0. [INFERENCE] "Burst size" phải khớp max packet size của bulk endpoint (1024 B ở U3, 512 B ở U2 HS, 64 B ở FS); sai giá trị → RX DMA cắt gói sai → mất gói/hỏng RX aggregation.

Board thật dùng tốc độ nào: [KIỂM TRA TRÊN BOARD] (đọc `0xFF` và `0xFE11`, hoặc `lsusb -t`).

## 3. Đổi chế độ U2 ↔ U3 (`set_usb_mode_88xx`, `halmac_usb_88xx.c:465-511`)

Chỉ chạy khi module param `rtw_switch_usb_mode` = 1 (U2→U3) hoặc 2 (U3→U2) (`os_dep/linux/os_intfs.c:427`, mặc định **0** → không chạy) qua `HW_VAR_USB_MODE` (`hal/rtl8822b/usb/rtl8822bu_ops.c:143-159`).

| Bước | Thao tác | Ý nghĩa |
|---|---|---|
| 1 | `cur = (R8(0xFF) == 0x20) ? U3 : U2` | |
| 2 | `t = R32(REG_PAD_CTRL2)`; nếu `USB23_SW_MODE_V1 == 0` **và** `USB3_USB2_TRANSITION == 0` → `"[ERR]u2/u3 switch"` | chip/board không hỗ trợ chuyển |
| 3 | `W8_CLR(REG_SW_MDIO+3, BIT0)` | `DIS_TIMEOUT_IO = 0` (8822B) |
| 4 | `W32(REG_PAD_CTRL2, (t & ~USB23_SW_MODE(3)) \| USB23_SW_MODE(mode) \| RSM_EN_V1)` | chọn chế độ đích, bật "resume" |
| 5 | `W8(REG_PAD_CTRL2+1, 4)` | `MATCH_CNT[15:8] = 4` |
| 6 | `W16_SET(REG_SYS_PW_CTRL, APFM_OFFMAC)` | yêu cầu tắt MAC → chip tự re-enumerate ở tốc độ mới [INFERENCE] |
| 7 | delay 1 ms; `W32_SET(REG_PAD_CTRL2, NO_PDN_CHIPOFF_V1)` | |

Sau bước 6 thiết bị biến mất khỏi bus rồi xuất hiện lại → driver phải probe lại (`usb_reprobe_switch_usb_mode`, `os_dep/linux/usb_intf.c:739`). Các lần đọc/ghi sau bước 6 có thể lỗi USB — đó là hành vi dự kiến [INFERENCE].

## 4. Truy cập USB-PHY gián tiếp

`usbphy_write_88xx(addr, data, speed)` / `usbphy_read_88xx(addr, speed)` (`halmac_usb_88xx.c:514-560`):

| | USB3-PHY | USB2-PHY |
|---|---|---|
| Ghi | `W8(0xFF0D, data & 0xFF)`; `W8(0xFF0E, data >> 8)`; `W8(0xFF0C, addr \| BIT7)` | `W8(0xFE41, data)`; `W8(0xFE40, addr)`; `W8(0xFE42, 0x81)` |
| Đọc | `W8(0xFF0C, addr \| BIT6)`; `R32(0xFF0C) >> 8` (16 bit) | địa chỉ ≥ 0xE0 trừ 0x20; chỉ cho 0xC0–0xDF: `W8(0xFE40, addr)`; `W8(0xFE42, 0x81)`; `R8(0xFE43)` |
| Độ rộng dữ liệu | 16 bit | 8 bit |

Không gian USB2-PHY và USB3-PHY là **hai không gian độc lập** với MAC (địa chỉ `0x01` của USB3-PHY không liên quan MAC `0x01`). Ghi xong không có polling — driver giả định PHY nhận ngay [SOURCE].

Điểm lạ [SOURCE]: `usbphy_read_88xx` trả về kiểu `u16` nhưng nhánh lỗi `return HALMAC_RET_NOT_SUPPORT;` — mã lỗi bị trả về **như dữ liệu**, caller không phân biệt được.

Dùng thật: `phy_cfg_usb_8822b()` (`halmac_usb_8822b.c:150-172`) duyệt `usb2_phy_param_8822b` (rỗng — chỉ có mục kết thúc `0xFFFF`) rồi `usb3_phy_param_8822b` (`{0x0001, 0xA841, INTF_PHY, CUT_D, ALL}`) — `parse_intf_phy_88xx` chỉ áp dụng mục khớp cut (`halmac_common_88xx.c:3325-3360`). → Chỉ chip **D-cut** được ghi USB3-PHY[0x01] = 0xA841; với cut khác, không có thao tác PHY nào [SOURCE].

## 5. RX aggregation (`cfg_usb_rx_agg_88xx`, `halmac_usb_88xx.c:127-190`)

Driver chọn `rxagg_mode = RX_AGG_USB` (`hal/rtl8822b/usb/rtl8822bu_halinit.c:468`) → `HALMAC_RX_AGG_MODE_USB` (`hal/hal_halmac.c:4644-4652`), `size_limit_en = 1`.

| Thao tác | Kết quả (USB mode, ngưỡng mặc định) |
|---|---|
| `agg = R8(0x10C) \| RXDMA_AGG_EN(BIT2)` | bật aggregation |
| `dma_usb_agg = R8(0x283) & ~BIT7` | `USB_RXDMA_AGG_EN[31] = 0` — mode USB xoá bit này, mode DMA đặt bit này [SOURCE; tên bit ngược với trực giác] |
| ngưỡng: U3 → `size = 0x5, timeout = 0xA`; U2 → `size = 0x5, timeout = 0x20` | |
| `R32(0x280) \| EN_PRE_CALC(BIT29)` → W32 | |
| `W8(0x10C, agg)`; `W8(0x283, dma_usb_agg)` | |
| `W16(0x280, size \| (timeout << 8))` | U2: **`0x2005`**, U3: **`0x0A05`** |

[INFERENCE] Aggregation gom nhiều frame RX vào một bulk-in transfer: `size` là ngưỡng (đơn vị page/KB theo hardware), `timeout` là thời gian chờ tối đa trước khi đẩy lên host. Timeout dài hơn ở U2 (0x20) giảm số transfer nhưng tăng độ trễ.

## 6. Dấu hiệu lỗi

| Hiện tượng | Thanh ghi / log |
|---|---|
| Throughput RX thấp bất thường | `0x280–0x283`, `0x10C` bit 2, `0x290` (burst size sai tốc độ) |
| Thiết bị biến mất khi load | có đặt `rtw_switch_usb_mode`? (`"[ERR]u2/u3 switch"`, `"[ERR]u2/u3 unchange"`) |
| `"[ERR]Error USB Speed !"` / `"[ERR]phy offset"` | tham số `speed`/`addr` của PHY sai |
| Mọi I/O fail sau một thời gian | xem 03 §3.4: `continual_io_error > 4` → surprise removed |
