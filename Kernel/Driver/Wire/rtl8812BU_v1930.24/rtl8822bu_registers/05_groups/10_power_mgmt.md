# 05.10 — Quản lý nguồn: IPS, LPS (RPWM/CPWM), WoWLAN, suspend/autosuspend

> Nhãn như 05.01. Ở 8822B, phần lớn trạng thái tiết kiệm năng lượng (LPS, WoWLAN) do **firmware** điều khiển; driver gửi lệnh H2C (05.08) và chỉ dùng một số thanh ghi để bắt tay hoặc chuẩn bị. Power sequence thô (bật/tắt MAC) ở 05.01 §4–5.

## 1. Bản đồ cơ chế → thanh ghi

| Cơ chế | Thanh ghi chính | Đường code |
|---|---|---|
| **IPS** (inactive PS: tắt hẳn MAC khi không kết nối) | toàn bộ power-seq card_dis/card_en; kiểm tra `0x100 == 0xEA` | `check_ips_status` (`hal/rtl8822b/rtl8822b_ops.c:757-771`); `mac_pwr_switch_usb_8822b` |
| **LPS** (leisure PS: STA đã kết nối, ngủ giữa các beacon) | `REG_USB_HRPWM_8822B` 0xFE58 (host→FW), `REG_USB_HCPWM_8822B` 0xFE57 (FW→host) | `rtw_set_rpwm`, `rtw_cpwm_polling` (`core/rtw_pwrctrl.c:567-729`) → `HW_VAR_SET_RPWM`/`HW_VAR_CPWM`/`HW_VAR_RPWM_TOG` (`hal/rtl8822b/usb/rtl8822bu_ops.c:124-184`) |
| **WoWLAN** | `REG_TXPAUSE`, `REG_SYS_FUNC_EN` (gate BB), `REG_RSV_CTRL`, `REG_WOWLAN_WAKE_REASON` 0x1C7, `REG_FE1IMR`/`REG_RXPKT_NUM`, `REG_WOW_CTRL` 0x690, `REG_WKFMCAM_CMD/RWD` 0x698/0x69C, `REG_SECCFG`, GPIO6 | `rtw_hal_wow_enable/disable` (`hal/hal_com.c:12200-12430`) |
| **Suspend/autosuspend USB** | power-off sequence | `rtw_hw_suspend`/`rtw_suspend`/`autosuspend_enter` (`os_dep/linux/usb_intf.c:934-1308`) |

## 2. LPS: bắt tay RPWM/CPWM qua thanh ghi USB-local

`CONFIG_LPS_LCLK = 1` (`include/autoconf.h:142`) và `CONFIG_DETECT_CPWM_BY_POLLING` bật (`:151`) → host **poll** CPWM thay vì chờ ngắt (USB không có ngắt — 05.03).

Định dạng byte RPWM (`hal/rtl8822b/usb/rtl8822bu_ops.c:129-134`, comment gốc viết cho 88E "modify by Thomas 2012/4/2" nhưng code 8822B dùng nguyên):

| Bit | Tên driver | Tên header (section `REG_SDIO_HRPWM1_8822B`, mượn) | Ý nghĩa theo comment |
|---|---|---|---|
| 7 | `PS_TOGGLE` | `BIT_TOGGLE_8822B` | đảo mỗi lần gửi lệnh mới |
| 6 | `PS_ACK` | `BIT_ACK_8822B` | "1: report cpwm value after success set" |
| 0 | (`PS_DPS`/`PS_LCLK`) | `BIT_REQ_PS_8822B` | "1: 32k, 0: 40MHz" (clock) |

`HW_VAR_SET_RPWM` ghi `ps_state & 0xC1` → chỉ 3 bit trên đi xuống phần cứng; các bit trạng thái khác của `pslv` (`PS_RF_OFF` BIT1, `PS_ALL_ON` BIT2, `PS_ST_ACTIVE` BIT3 — `include/rtw_pwrctrl.h:188-211`) bị **cắt bỏ** [SOURCE].

Trình tự `rtw_set_rpwm(pslv)` khi thoát LPS (đi từ S0/S2 lên S4 = active):

```
1. rpwm = PS_STATE(pslv) | pwrpriv->tog          ; tog = 0x00 hoặc 0x80
2. nếu cpwm hiện < S2 và pslv ≥ S2: rpwm |= PS_ACK  ; xin FW báo lại
3. cpwm_orig = R8(0xFE57)                          ; HW_VAR_CPWM
4. W8(0xFE58, rpwm & 0xC1)                         ; HW_VAR_SET_RPWM
5. tog += 0x80                                     ; đảo toggle cho lần sau
6. rtw_cpwm_polling: lặp mỗi 1 ms đọc 0xFE57 tới khi (cpwm_orig ^ cpwm_now) & 0x80
      ≤ LPS_CPWM_TIMEOUT_MS = 10 ms / lượt; hết lượt → gửi lại RPWM với toggle mới
      tối đa LPS_RPWM_RETRY_CNT = 3 lượt (core/rtw_pwrctrl.c:564-604)
```

Mô phỏng 2 lần thoát LPS liên tiếp (giả sử `pslv = S4 = 0x0C`, `tog` bắt đầu 0) [tính tay]:

| Lần | rpwm trước mask | ghi 0xFE58 (`& 0xC1`) | tog sau |
|---|---|---|---|
| 1 | `0x0C \| 0x00 \| 0x40 = 0x4C` | `0x40` | `0x80` |
| 2 | `0x0C \| 0x80 \| 0x40 = 0xCC` | `0xC0` | `0x00` (u8 tràn) |

→ bit 7 xen kẽ 0/1, đó là "chữ ký" mà FW dùng để biết có lệnh mới; host biết FW đã xử lý khi bit 7 của CPWM đổi. `mac_pwr_switch_usb_8822b` cũng tự đảo bit 7 của `0xFE58` nếu FW đang chạy (05.01 §4) để đánh thức FW trước power-off [SOURCE-SEQ].

Lỗi: không thấy CPWM đổi sau 3 × 10 ms → hàm trả `_FAIL`; lớp trên coi FW không đáp ứng (`DBG_CHECK_FW_PS_STATE` nếu bật sẽ dump thêm). Vì 0xFE57/0xFE58 ≥ 0x1000, mỗi lần đọc/ghi chỉ thử USB **1 lần** (03 §3.4).

## 3. IPS: kiểm tra MAC đã tắt

`check_ips_status()` đọc `0x100` (`REG_CR`) và `0x86` (byte 2 của `REG_MCU_TST_CFG_8822B` 0x84 theo bố cục header — code dùng literal, không có tên) để log, rồi `0x100 == 0xEA` ⇒ "đã vào IPS" [SOURCE]. Cùng quy ước `0xEA` với `mac_pwr_switch_usb_8822b`. Ý nghĩa của `0x86` trên USB: chưa xác định (trong pwr-seq, `0x86` chỉ được thao tác cho SDIO).

## 4. WoWLAN — các bước chạm thanh ghi (theo thứ tự trong source)

`rtw_hal_wow_enable()` (`hal/hal_com.c:12200-12330`), trích các truy cập [TOOL + SOURCE]:

| Bước | Thao tác | Ghi chú |
|---|---|---|
| 1 | `rtw_hal_gate_bb(adapter, TRUE)` (`:12219`) | lưu `TXPAUSE`, `TXPAUSE = 0xFF`, `SYS_FUNC_EN &= ~BIT0` (05.01 §6.2) |
| 2 | nếu `binstallKCK_KEK` (`CONFIG_GTK_OL`): `rtw_hal_fw_sync_cam_id` (`:12222`) → `SECCFG = 0xCC` | 05.07 §4 |
| 3 | `rtw_hal_fw_dl(adapter, TRUE)` (`:12227`) | tải **firmware WoWLAN** (05.08) |
| 4 | `rtw_hal_dl_pattern(adapter, 1)` (`:12252`) — "redownload wow pattern" | CAM mẫu, §5 |
| 5 | `rtw_hal_switch_gpio_wl_ctrl(WAKEUP_GPIO_IDX, TRUE)` (`:12275`, `CONFIG_GPIO_WAKEUP`) | GPIO6 (05.11 §2) |
| 6 | `R8(REG_WOWLAN_WAKE_REASON)` (0x1C7, `:12287`) | đọc, không lưu giá trị |
| 7 | `W8(REG_RSV_CTRL, 0x20)`; nếu không phải 8188F/GTV: `W8(REG_RSV_CTRL, 0x60)` (`:12310-12313`) | `WLOCK_1C_B6[5]`, `R_DIS_PRST[6]` (05.01 §6.6) |
| … | các lệnh H2C (WoWLAN ctrl, remote wake, AOAC rsvd page) | không phải thanh ghi |

`rtw_hal_wow_disable()` (`:12400-12430`): log `0x630`, `0x631` (`REG_WMAC_TCR_TSFT_OFS`), `0x634` (`REG_STMP_THSD`), `0x1C7`; lưu `wowlan_wake_reason = R8(0x1C7)`; `rtw_hal_check_wow_ctrl()` chờ FW xoá `FE1IMR.FS_RXDONE_INT_EN[16]` và `RXPKT_NUM.RW_RELEASE_EN[18]` (comment "Wait FW to cleare 0x120 bit16, 0x284 bit18 to 0", `:5169`) — tối đa 24 × 20 ms; `rtw_hal_force_enable_rxdma`: `WOW_CTRL(0x690) &= 0xF0`, nhả RX DMA; `rtw_hal_gate_bb(FALSE)`; tải lại firmware thường.

`REG_WOW_CTRL` (0x690) chỉ được đọc trên nhánh non-JAGUAR2 (chết với 8822B) và bị xoá nibble thấp trong `force_enable_rxdma` [TOOL].

## 5. CAM mẫu đánh thức (`hal/hal_com.c:12007-…`)

Cùng kiểu cửa sổ gián tiếp như security CAM (05.07 §2): `W32(REG_WKFMCAM_RWD 0x69C, data)`; `W32(REG_WKFMCAM_CMD 0x698, WKFCAM_POLLING_V1[31] | WKFCAM_WE[16] | ADDR)`; poll bit 31 về 0 (timeout `WOW_CAM_ACCESS_TIMEOUT_MS`), mutex `wowlan_pattern_cam_mutex`, log `RTW_ERR(... to:%u, polling:%u ...)` khi timeout. Mỗi mẫu `WKFMCAM_SIZE = 24` byte (`include/rtw_pwrctrl.h:103`). Có thêm bit `WKFCAM_CLR_V1[30]` (xoá toàn bộ) dùng ở `_rtw_wow_pattern_clean_cam` [SOURCE theo tên hàm trong phụ lục A05].

## 6. Dấu hiệu lỗi

| Hiện tượng | Kiểm tra |
|---|---|
| Ra khỏi LPS chậm / mất gói đầu | log CPWM fail; `0xFE57/0xFE58` |
| Không vào được WoWLAN | log `"Polling MGQ_CPU empty fail!"` (`gate_bb`, `:11402`); FW WoW tải lỗi (05.08 §7) |
| Thức dậy không rõ lý do | `pwrctl->wowlan_wake_reason` (= 0x1C7), xuất qua proc |
| Sau resume không nhận gói | `0x284` bit 18 còn 1 (`"Stop RX DMA failed"` / force enable) |
