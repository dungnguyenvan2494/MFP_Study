# 05.11 — GPIO, LED và pinmux (MAC 0x0040–0x007F)

> Nhãn như 05.01. Tên field lấy từ `halmac_bit_8822b.h` qua `tools/bitname.py`.

## 1. Thanh ghi

| Địa chỉ | Tên | St | Field có bằng chứng | Ai dùng |
|---|---|:-:|---|---|
| `0x0040` | `REG_GPIO_MUXCFG_8822B` (`R_0x40`) | A | `GPIOSEL[1:0]` (pinmux SW_IO/debug), `WLRFE_4_5_EN[2]`, `FSPI_EN` | `pre_init_system_cfg_8822b`, `init_system_cfg_8822b`, `phydm_rfe_8822b_init`, bảng pinmux |
| `0x0041–0x0043` | (byte 1–3 của 0x40) | T | chỉ bảng pinmux (PTA BT/WL, UART, SPI flash) | — |
| `0x0044–0x0047` | `REG_GPIO_PIN_CTRL_8822B` | A (byte 1–3) | `GPIO_IN_7_TO_0[7:0]`, `GPIO_OUT_7_TO_0[15:8]`, `GPIO_IO_SEL_7_TO_0[23:16]`, `GPIO_MOD_7_TO_0[31:24]` | `rtw_hal_set_output_gpio`, `rtw_hal_set_input_gpio` |
| `0x0048–0x004B` | `REG_GPIO_INTM_8822B` | A (pwr-seq) | `EXTWOL_EN[16]`, `EXTWOL_SEL[17]`, `GPIO9_INT_MD[9]` | pwr-seq, pinmux GPIO8/9 EXT_WOL |
| `0x004C–0x004F` | `REG_LED_CFG_8822B` (`REG_LEDCFG0`, `R_0x4c`) | A | `LED2CM[18:16]` (mode), `LED2SV[19]`, `LED2PL[20]`, `LED2DIS[21]`, `GPIO13_14_WL_CTRL_EN[22]`, `DPDT_SEL_EN[23]`, `DPDT_WLBT_SEL[24]`, `PAPE_SEL_EN[25]`, `LNAON_SEL_EN[26]` | LED: `pinmux_wl_led_*`; RFE/antenna: `pre_init_system_cfg_8822b`, `phydm_rfe_8822b_init`, `phy_set_rf_path_switch_8822b`, wifi-only BTC |
| `0x0060–0x0063` | `REG_GPIO_EXT_CTRL_8822B` (alias `REG_GPIO_PIN_CTRL_2`, `rtl8822b_hal.h:88`) | A (byte 1–3) | như 0x44 cho GPIO 15..8 | `rtw_hal_set_output/input_gpio` |
| `0x0064–0x0067` | `REG_PAD_CTRL1_8822B` (`R_0x64`) | A | `LNAON_WLBT_SEL[28]`, `PAPE_WLBT_SEL[29]` | `pre_init_system_cfg_8822b`, `phydm_rfe_8822b_init`, pwr-seq |
| `0x0068` | `REG_WL_BT_PWR_CTRL_8822B` | A | `WL_FUNC_EN`, `BT_FUNC_EN`, `WL_HWPDN_SL` (đọc) | `read_chip_version` (05.01 §6.9) |
| `0x0070–0x0073` | `REG_SYS_SDIO_CTRL_8822B` (`R_0x70`, `REG_LTECOEX_PATH_CONTROL`) | A | byte 3 = `0x0E`: `LTE_MUX_CTRL_PATH[26]`, `LTE_COEX_UART[25]`, `DBG_GNT_WL_BT[27]` … | `ex_hal8822b_wifi_only_hw_config`, `phy_set_rf_path_switch_8822b`, MP |

## 2. GPIO đa dụng: cách bật một chân làm output

Mỗi chân GPIO *n* (0–7) có **4 bit**, mỗi bit ở **một byte khác nhau** của `REG_GPIO_PIN_CTRL` (`0x44`); GPIO 8–15 dùng `REG_GPIO_EXT_CTRL` (`0x60`) với `index -= 8`. `rtw_hal_set_output_gpio(padapter, index, outputval)` (`hal/hal_com.c:4679-4734`) [SOURCE]:

| Bước | Byte | Phép toán | Field | Comment trong code |
|---|---|---|---|---|
| 1 | `+3` (0x47) | `&= ~BIT(n)` | `GPIO_MOD[24+n]` = 0 | "config GPIO mode" |
| 2 | `+2` (0x46) | `\|= BIT(n)` | `GPIO_IO_SEL[16+n]` = 1 | "config GPIO Sel — 0: input, 1: output" |
| 3 | `+1` (0x45) | `\|= BIT(n)` hoặc `&= ~BIT(n)` | `GPIO_OUT[8+n]` | "set output value" |

`rtw_hal_set_input_gpio` (`:4735-4771`) chỉ làm bước 1 và đặt `IO_SEL = 0`. Mỗi bước là RMW 8-bit (2 vendor request + 2 lần ghi `0x4E0` vì `0x44–0x63` thuộc REG_ON_SEC) → đổi một chân output = **12 control transfer**. Chú ý thứ tự: chân chuyển sang output (bước 2) **trước** khi đặt giá trị (bước 3) → trong khoảng thời gian giữa hai lần ghi, chân xuất giá trị `GPIO_OUT` cũ [INFERENCE — có thể tạo glitch nếu giá trị cũ khác].

**Dùng thật trong build:** chỉ cho chân **wake-up của WoWLAN** (`CONFIG_GPIO_WAKEUP` bật). `WAKEUP_GPIO_IDX = 6` (`include/hal_ic_cfg.h:270`), `HIGH_ACTIVE_DEV2HST = 0` (cờ `-D` từ Makefile) → `rtw_pwrctrl.c:2307-2308`: `val8 = (is_high_active == 0) ? 1 : 0` → **GPIO6 = output, mức 1** (mặc định không kích hoạt, vì tín hiệu đánh thức host là active-low). Chip (firmware) sẽ kéo GPIO6 xuống khi cần đánh thức host [INFERENCE: firmware thao tác, không thấy trong driver]. Thêm các nơi gọi: `hal_com.c:6446-6452`, `core/rtw_debug.c:5183-5189` (proc debug).

Mô phỏng (GPIO6, giá trị ban đầu giả định `0x47=0x40, 0x46=0x00, 0x45=0x00`):

```
B1: 0x47 = 0x40 & ~0x40 = 0x00     → GPIO6 thoát chế độ chức năng (mode=GPIO)
B2: 0x46 = 0x00 |  0x40 = 0x40     → GPIO6 = output
B3: 0x45 = 0x00 |  0x40 = 0x40     → GPIO6 xuất mức 1
```

## 3. Pinmux HALMAC — một chân, nhiều chức năng, chọn theo thứ tự ưu tiên

Mỗi GPIO có một danh sách `PINMUX_LIST_GPIOn_8822B[]` (`halmac_gpio_8822b.c:359-495`). Mỗi phần tử `{func, id, type, offset, msk, value}` nói: "chức năng *func* được chọn khi `R8(offset) & msk == value`". Danh sách **xếp theo ưu tiên** (phần tử đầu thắng).

`pinmux_switch_8822b()` (`:928-1021`) chọn chức năng `F` cho chân [SOURCE]:

```c
for (i = 0; i < size; i++) {
	v = R8(list[i].offset) & ~list[i].msk;
	if (list[i].func == F) {                   /* tới đúng chức năng: bật nó */
		W8(list[i].offset, v | (list[i].value & list[i].msk));
		break;
	}
	W8(list[i].offset, v | (~list[i].value & list[i].msk));   /* chức năng ưu tiên hơn: TẮT nó */
}
```

Tức là: mọi chức năng **đứng trước** F trong danh sách bị ghi giá trị *ngược* (vô hiệu hoá), F được ghi giá trị *đúng*, các chức năng sau không đụng tới.

**Ví dụ thật — bật LED trên GPIO8** (`rtw_halmac_led_cfg(d, TRUE, mode)` → `halmac_pinmux_set_func(HALMAC_GPIO_FUNC_WL_LED)`, `hal_halmac.c:5233`; `get_pinmux_list_8822b` chọn `PINMUX_LIST_GPIO8_8822B`, `:765-770`):

| i | Phần tử | offset | msk | value | Hành động | Field |
|---|---|---|---|---|---|---|
| 0 | `GPIO8_WL_EXT_WOL_8822B` (`:287-289`) | `0x4A` | `BIT0\|BIT1` | `BIT0\|BIT1` | ghi `~value & msk = 0` → **tắt** EXT_WOL | `EXTWOL_EN[16]`, `EXTWOL_SEL[17]` = 0 |
| 1 | `GPIO8_WL_LED_8822B` (`:290-292`) | `0x4E` | `BIT5` | `BIT5` | ghi `value` → **bật** LED, dừng | `LED2DIS[21]` = 1 (tên header; ý nghĩa "DIS" ngược trực giác → chưa xác định) |
| 2 | `GPIO8_SW_IO_8822B` | `0x40` | `BIT0\|BIT1` | 0 | không chạm | |

Sau đó `pinmux_wl_led_mode_88xx(mode)` (`halmac_gpio_88xx.c:71-106`) sửa byte `0x4E`:

```
v = R8(0x4E)
v &= ~BIT6          → GPIO13_14_WL_CTRL_EN[22] = 0
v |=  BIT3          → LED2SV[19] = 1
v &= ~(BIT0|1|2)    → LED2CM[18:16] = 0
v |= mode           → TRX=2, TX=4, RX=6, SW_CTRL=0
W8(0x4E, v)
```

`pinmux_wl_led_sw_ctrl_88xx(on)` (`:117-126`): `on` → xoá `LED2SV`, `off` → đặt `LED2SV`. Vậy **LED2SV = 1 nghĩa là LED tắt** khi ở chế độ SW [SOURCE-SEQ].

**Driver dùng chế độ nào?** `CONFIG_RTW_LED` và `CONFIG_RTW_SW_LED` bật (`autoconf.h:234-236`), `CONFIG_RTW_HW_LED` tắt. `hal_init_misc()` (`hal/rtl8822b/usb/rtl8822bu_halinit.c:221-233`): `init_hwled()` chỉ làm gì khi `LedStrategy == HW_LED`; nếu `bRegUseLed` → `rtw_halmac_led_cfg(…, TRUE, 3)` → `_led_mode_drv2halmac(3) = HALMAC_WLLED_MODE_SW_CTRL` (`hal_halmac.c:5191-5192`). Sau đó LED được bật/tắt bằng phần mềm: `swledon/swledoff` (`rtl8822bu_led.c:90-130`) → `rtw_halmac_led_switch(d, 1/0)` → `pinmux_wl_led_sw_ctrl`. Mỗi lần nháy LED = 1 RMW 8-bit = 4 control transfer [INFERENCE từ 03 §3.4].

## 4. Các bit RFE/antenna trong cùng dải

| Nơi | Thao tác | Field | Nguồn |
|---|---|---|---|
| `pre_init_system_cfg_8822b` | `PAD_CTRL1[29:28] = 11`; `LED_CFG[26:25] = 00`; `GPIO_MUXCFG[2] = 1` | `PAPE_WLBT_SEL`, `LNAON_WLBT_SEL`; `LNAON_SEL_EN`, `PAPE_SEL_EN`; `WLRFE_4_5_EN` | `halmac_init_8822b.c:1017-1029` |
| `phydm_rfe_8822b_init` | **lặp lại đúng 3 thao tác trên** qua `odm_set_bb_reg` (RMW 32-bit) | | `phydm_hal_api8822b.c:301-303` |
| `phy_set_rf_path_switch_8822b` | `LED_CFG[24:23] = 10b`; `SYS_SDIO_CTRL[31:24] = 0x0E` | `DPDT_WLBT_SEL=1`, `DPDT_SEL_EN=0`; `LTE_MUX_CTRL_PATH`, `LTE_COEX_UART`, `DBG_GNT_WL_BT`… | `halrf_8822b.c:552,568` |
| `ex_hal8822b_wifi_only_hw_config` | cùng 2 giá trị trên qua `halwifionly_phy_set_bb_reg` | | `hal/btc/halbtc8822bwifionly.c:66,76` |

[INFERENCE theo tên] PAPE = PA enable, LNAON = LNA on, DPDT = công tắc anten 2 cực 2 ngả; `*_WLBT_SEL` chọn WiFi hay BT điều khiển chân. Các chân này nối ra front-end ngoài (FEM); cấu hình sai → anten/PA không bật → mất tín hiệu một path. Việc HALMAC và PHYDM cùng ghi các bit này (dư thừa) là điểm cần nhớ khi debug: giá trị cuối cùng là của ai ghi sau [SOURCE].

## 5. Dấu hiệu lỗi

| Triệu chứng | Kiểm tra |
|---|---|
| WoWLAN không đánh thức được host | GPIO6 có ở output/mức 1 sau init không: đọc `0x44–0x47` qua proc `read_reg` (`core/rtw_debug.c`) [KIỂM TRA TRÊN BOARD] |
| LED không sáng | log `"pinmux set fail!(0x%x)"` / `"mode set fail!"` (`hal_halmac.c:5236,5244`); `HALMAC_RET_BUSY_STATE` nếu `gpio_cfg_state` kẹt BUSY |
| Mất một path anten | `0x4C[26:23]`, `0x64[29:28]`, `0x40[2]` |
