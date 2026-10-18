# 05.06 — EDCA, timing, beacon và TSF (MAC 0x0500–0x05FF)

> Nhãn như 05.01. Danh sách đầy đủ mọi truy cập: [appendix/A04_mac_500_5ff.md](appendix/A04_mac_500_5ff.md). Giá trị mặc định từ bảng MAC_REG: [appendix/A11_mac_reg_table.md](appendix/A11_mac_reg_table.md).

## 1. Thanh ghi chính

| Địa chỉ | Tên | Vai trò (theo cách code dùng) | Ghi bởi |
|---|---|---|---|
| `0x0500/0504/0508/050C` | `REG_EDCA_VO/VI/BE/BK_PARAM_8822B` | tham số truy cập kênh mỗi AC (32-bit; byte 2–3 = TXOP limit) | `rtl8822b_sethwreg(HW_VAR_AC_PARAM_*)` ghi cả 32 bit từ lớp MLME; `init_edca_cfg_8822b` ghi TXOP; bảng MAC_REG |
| `0x0512` | `REG_PIFS_8822B` | PIFS | `init_edca_cfg_8822b` = `0x19` |
| `0x0514–0x0517` | `REG_SIFS_8822B` | 4 giá trị SIFS | `init_edca_cfg_8822b` |
| `0x051B` | `REG_SLOT_8822B` | slot time | `0x09` (init, `set_beacon_related_registers`); `rtl8822b_sethwreg(HW_VAR_SLOT_TIME)` |
| `0x0522–0x0523` | `REG_TXPAUSE_8822B` | **dừng từng hàng đợi TX** (1 bit/hàng) | 14 nơi: init (`= 0`), tải FW EMEM (`\|= BIT7`), IQK, WoW gate BB (`= 0xFF`), MCC, `phydm_stop_ic_trx` |
| `0x0520–0x0521` | `REG_TX_PTCL_CTRL_8822B` | điều khiển giao thức TX | `init_edca_cfg_8822b` (`+1 &= ~BIT4`), IQK backup/restore, `phydm_stop_ic_trx` |
| `0x0524–0x0525` | `REG_RD_CTRL_8822B` | RDG/EDCCA-related | `beacon_function_enable`: `+1 = 0x6F`; `phydm_mac_edcca_state` |
| `0x0540–0x0542` | `REG_TBTT_PROHIBIT_8822B` | khoảng cấm TX quanh TBTT (setup/hold, đơn vị 32 µs) | `init_edca_cfg_8822b`, `InitBeaconParameters`, `Resume/StopTxBeacon` |
| `0x0544` | `REG_RD_NAV_NXT_8822B` | NAV cho RDG/TXOP | `init_edca_cfg_8822b` |
| `0x0550` | `REG_BCN_CTRL_8822B` | port 0: `EN_BCN_FUNCTION[3]`, `DIS_TSF_UDT[4]`, `EN_TXBCN_RPT[2]` … | 21 nơi |
| `0x0551` | `REG_BCN_CTRL_CLINT0_8822B` (`REG_BCN_CTRL_1`) | port 1 (client 0): `CLI0_EN_BCN_FUNCTION[3]`, `CLI0_DIS_TSF_UDT[4]`… | `beacon_function_enable`, `hw_var_set_mlme_disconnect`, `rw_bcn_ctrl_88xx` |
| `0x0553` | `REG_DUAL_TSF_RST_8822B` | `TSFTR_RST[0]` (port0), `TSFTR_CLI0_RST[1]` (port1) — ghi 1 = reset TSF | 17 nơi: join, disconnect, `set_beacon_related_registers`, `cfg_tsf_rst_88xx` |
| `0x0554–0x0557` | `REG_MBSSID_BCN_SPACE_8822B` | beacon interval; byte 3 `BCN_TIMER_SEL_FWRD[30:28]` = **chọn port để đọc TSF** | `cfg_bcn_space_88xx`, `rtw_hal_get_tsftr_by_port` |
| `0x0558` / `0x0559` | `REG_DRVERLYINT` / `REG_BCNDMATIM` | early-interrupt / beacon-DMA time trước TBTT | init + `InitBeaconParameters` |
| `0x055A` | `REG_ATIMWND_8822B` | cửa sổ ATIM | `set_beacon_related_registers` = 2 |
| `0x055E/055F` | `REG_RXTSF_OFFSET_CCK/OFDM` | bù TSF lúc nhận beacon | init `0x3030`; `set_beacon_related_registers` = `0x50/0x50` |
| `0x0560–0x0567` | `REG_TSFTR_8822B` | TSF 64-bit của port được chọn | `rtw_hal_get_tsftr_by_port`, `rm_state_idle` |
| `0x0577` | B3 của `REG_MISC_CTRL_8822B` | đồng bộ TSF giữa port | `hw_port0_tsf_sync_sel`, `rtl8822b_sethwreg`, `set_hw_value_88xx` |
| `0x0578–0x057A` | `REG_BCN_CTRL_CLINT1..3_8822B` | port 2–4 | `rw_bcn_ctrl_88xx` |
| `0x05B4` | `REG_TIMER0_SRC_SEL_8822B` | nguồn timer 0 | init `&= ~(BIT4\|5\|6)`, `hw_port0_tsf_sync_sel` |
| `0x0422` bit 6 | `REG_FWHW_TXQ_CTRL.EN_BCNQ_DL[22]` | cho phép phát beacon queue | `Resume/StopTxBeacon` |

## 2. `init_edca_cfg_8822b()` (`halmac_init_8822b.c:894-930`)

| Ghi | Giá trị | Phép tính (tính tay từ define `:96-146`) |
|---|---|---|
| `TIMER0_SRC_SEL &= ~(BIT4\|BIT5\|BIT6)` | | chọn nguồn timer 0 = 0 |
| `TXPAUSE (W16) = 0x0000` | | **mở tất cả** hàng đợi |
| `SLOT = WLAN_SLOT_TIME` | `0x09` | 9 µs (slot ngắn 802.11a/g/n/ac) |
| `PIFS = WLAN_PIFS_TIME` | `0x19` | 25 µs |
| `SIFS (W32) = WLAN_SIFS_CFG` | `0x10100E0A` | `CCK_CONT_TX 0x0A` \| `OFDM_CONT_TX 0x0E<<8` \| `CCK_TRX 0x10<<16` \| `OFDM_TRX 0x10<<24` |
| `EDCA_VO_PARAM+2 (W16) = WLAN_VO_TXOP_LIMIT` | `0x186` | đơn vị 32 µs (comment) → 390 × 32 = **12 480 µs** |
| `EDCA_VI_PARAM+2 (W16) = WLAN_VI_TXOP_LIMIT` | `0x3BC` | 956 × 32 = **30 592 µs** |
| `RD_NAV_NXT (W32) = WLAN_NAV_CFG` | `0x001B0005` | `RDG_NAV 0x05` \| `TXOP_NAV 0x1B << 16` |
| `RXTSF_OFFSET_CCK (W16) = WLAN_RX_TSF_CFG` | `0x3030` | CCK 0x30, OFDM 0x30 |
| `BCN_CTRL \|= BIT_EN_BCN_FUNCTION` | bit 3 | |
| `TBTT_PROHIBIT (W32) = WLAN_TBTT_TIME` | `0x00006404` | `PROHIBIT 0x04` \| `HOLD_TIME 0x064 << 8` (đơn vị 32 µs: 4 × 32 = 128 µs setup, 100 × 32 = 3,2 ms hold) |
| `DRVERLYINT = 0x04`, `BCNDMATIM = 0x02` | | |
| `TX_PTCL_CTRL+1 &= ~BIT4` | | |

**Ghi đè sau đó**: lớp driver gọi lại `InitBeaconParameters()` (`hal/rtl8822b/rtl8822b_ops.c:773-805`) với hằng **khác**: `DRVERLYINT = DRIVER_EARLY_INT_TIME_8822B = 0x05` (comment "5ms"), `BCNDMATIM = 0x02` ("2ms"), `TBTT_PROHIBIT` setup `0x04` và hold `TBTT_PROHIBIT_HOLD_TIME_STOP_BCN = 0x64` (`include/hal_com.h:390-392`), `BCNTCFG = 0x4413`, `BCN_CTRL (W16) = DIS_TSF_UDT | DIS_TSF_UDT<<8` (cả port 0 và 1). → Giá trị cuối cùng của `DRVERLYINT` là **0x05** chứ không phải 0x04 của HALMAC [SOURCE: thứ tự gọi xem 07_sequences]. Comment "5ms"/"2ms" mâu thuẫn với đơn vị 32 µs của HALMAC; đơn vị thật **chưa xác định** từ source.

## 3. TXPAUSE — công tắc dừng TX nhanh

Ai đặt/xoá (từ phụ lục A04) [SOURCE]:

| Nơi | Thao tác | Ngữ cảnh |
|---|---|---|
| `init_edca_cfg_8822b` | `W16 = 0` | khởi tạo |
| `rtw_hal_gate_bb` | lưu, `= 0xFF`, khôi phục | WoWLAN (05.01 §6.2) |
| `free_download_firmware_88xx` | `\|= BIT7` rồi khôi phục | tải EMEM khi FW đang chạy |
| `_iqk_configure_macbb_8822b` (`halrf_iqk_8822b.c:593`) | `odm_write_1byte(0x522, 0x7f)` — **0x7F, không phải 0xFF** (bit 7 để mở) | calibrate (driver-IQK, mặc định tắt — 01 §1.3) |
| `phydm_stop_ic_trx` (`phydm_api.c:670-699`) | lưu `tx_queue_bitmap = R8(0x522)` … khôi phục | PHYDM tạm dừng TRX |
| `rtl8822b_sethwreg` (`rtl8822b_ops.c:2032, 2207`) | `HW_VAR_TXPAUSE`: `= *val`; một case khác `= 0xFF` | lớp MLME/PS |
| `mcc_get_reg_hdl` (`hal_mcc.c:2748`) | đọc qua mảng `dbg_reg` | MCC debug |

[INFERENCE] 8 bit = 8 hàng (VO/VI/BE/BK/MGQ/HIQ/BCN…); `0xFF` dừng tất cả. Nếu một luồng quên khôi phục, TX bị kẹt vĩnh viễn — điểm đầu tiên cần đọc khi "kết nối được nhưng không gửi được".

## 4. Beacon: bật/tắt phát

```c
/* ResumeTxBeacon, hal/hal_com.c:17463-17479 */
W8(REG_FWHW_TXQ_CTRL+2, R8(..+2) | BIT6);              /* EN_BCNQ_DL = 1: cho phép tải beacon từ beacon queue */
W8(REG_TBTT_PROHIBIT,   TBTT_PROHIBIT_SETUP_TIME);      /* 0x04 */
W8(REG_TBTT_PROHIBIT+1, TBTT_PROHIBIT_HOLD_TIME & 0xFF); /* 0x80 */
W8(REG_TBTT_PROHIBIT+2, (R8(..+2) & 0xF0) | (0x80 >> 8));/* nibble thấp = bit 11:8 của hold */
/* StopTxBeacon :17481-17491 */
W8(REG_FWHW_TXQ_CTRL+2, R8(..+2) & ~BIT6);
hold = TBTT_PROHIBIT_HOLD_TIME_STOP_BCN (0x64)
```

Hold time 12 bit = byte 1 + nibble thấp byte 2 (giữ nibble cao) — ví dụ đọc-sửa-ghi một field băng qua ranh giới byte [SOURCE + tính tay].

Lúc scan, nếu có AP/mesh: `StopTxBeacon()` khi vào, `ResumeTxBeacon()` + `rtw_mi_tx_beacon_hdl()` khi ra (`rtl8822b_ops.c:1551-1568`).

## 5. TSF

**Reset**: ghi `REG_DUAL_TSF_RST` = `BIT0` (port 0) hoặc `BIT1` (port 1) — join type 0 (bắt đầu kết nối), disconnect, `set_beacon_related_registers`. Trong `set_beacon_related_registers` (`:871-878`) có **2 lần ghi liên tiếp**: lần 1 theo port, lần 2 luôn `BIT_TSFTR_RST` (port 0) — tức port 1 làm AP sẽ reset cả TSF port 0 [SOURCE; có chủ ý hay không: chưa xác định].

**Đọc TSF 64-bit của một port** (`rtw_hal_get_tsftr_by_port`, `hal/hal_com.c:14572-14600`):

```c
val8 = R8(REG_MBSSID_BCN_SPACE + 3);   /* 0x557 */
val8 &= 0x8F;                          /* xoá bit 6:4 = BCN_TIMER_SEL_FWRD[30:28] */
val8 |= port << 4;                     /* chọn port */
W8(REG_MBSSID_BCN_SPACE + 3, val8);
tsftr  = (u64)R32(REG_TSFTR + 4) << 32; /* nửa cao TRƯỚC */
tsftr |= R32(REG_TSFTR);                /* nửa thấp SAU */
```

Đây là **thanh ghi cửa sổ** (window): `0x560` hiển thị TSF của port do `0x557[6:4]` chọn. Hai lần đọc 32-bit tách rời qua USB (mỗi lần ~vài trăm µs): nếu nửa thấp tràn (wrap 2³² µs ≈ 71,6 phút) giữa hai lần đọc, giá trị ghép bị lệch 2³² µs. Source không có cơ chế đọc lại/latch → [INFERENCE] sai số hiếm nhưng có thể xảy ra. Cùng dạng: không có khoá giữa bước chọn port và bước đọc — hai luồng gọi đồng thời với port khác nhau có thể đọc nhầm port (mỗi `rtw_read/write` có mutex riêng, nhưng cả chuỗi 4 lệnh thì không).

## 6. BCN_CTRL — nhiều người cùng ghi

`REG_BCN_CTRL` (0x550) có 21 truy cập; ba kiểu ghi khác nhau [SOURCE]:

| Hàm | Cách ghi | Hệ quả |
|---|---|---|
| `InitBeaconParameters` | `W16(0x550, DIS_TSF_UDT \| DIS_TSF_UDT<<8)` | ghi **đè cả** port0 và port1 (byte 0x551) |
| `beacon_function_enable` | `W8(bcn_ctrl_reg, DIS_TSF_UDT \| EN_BCN_FUNCTION)` | ghi đè cả byte |
| `hw_var_set_bcn_func` (`hal/hal_com.c:13938-13966`) | bật: `W8 = EN_BCN_FUNCTION \| EN_TXBCN_RPT`; tắt: RMW xoá 2 bit | khi bật thì **xoá** `DIS_TSF_UDT` (ghi đè) |
| `download_firmware_88xx` | backup → `(v & ~EN_BCN_FUNCTION) \| DIS_TSF_UDT` → restore | 05.08 |
| IQK backup/restore | | |

[INFERENCE] Vì phần lớn là ghi đè cả byte, giá trị cuối cùng phụ thuộc thứ tự gọi; khi debug "TSF không cập nhật" (`DIS_TSF_UDT`=1) hay "không phát beacon" (`EN_BCN_FUNCTION`=0), phải xem hàm nào ghi sau cùng.
