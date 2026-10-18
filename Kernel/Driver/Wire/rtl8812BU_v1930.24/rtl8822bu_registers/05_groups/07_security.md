# 05.07 — Security: CAM khoá và cấu hình mã hoá phần cứng (MAC 0x0670–0x0680)

> Nhãn như 05.01. Mọi truy cập: [appendix/A05_mac_600_7ff.md](appendix/A05_mac_600_7ff.md).

## 1. Thanh ghi

| Địa chỉ | Tên | Field (header 8822B) | Tên legacy driver dùng | Vai trò |
|---|---|---|---|---|
| `0x0670` | `REG_CAMCMD_8822B` | `SECCAM_POLLING[31]`, `SECCAM_WE[16]`, `SECCAM_ADDR_V2[9:0]` | `CAM_POLLINIG` (BIT31), `CAM_WRITE` (BIT16), `CAM_READ` (0) — `include/hal_com_reg.h:881-883` | thanh ghi lệnh của **cửa sổ gián tiếp** vào bộ nhớ CAM |
| `0x0674` | `REG_CAMWRITE_8822B` | dữ liệu 32 bit cần ghi | | |
| `0x0678` | `REG_CAMREAD_8822B` | dữ liệu 32 bit đọc ra | | |
| `0x0680–0x0681` | `REG_SECCFG_8822B` | `TXUHUSEDK[0]`, `RXUHUSEDK[1]`, `TXENC[2]`, `RXDEC[3]`, `NOSKMC[5]`, `TXBCUSEDK[6]`, `RXBCUSEDK[7]`, `CHK_KEYID[8]`, `CHK_BMC[9]` | `SCR_TxUseDK`…`SCR_CHK_BMC` (`hal_com_reg.h:1647-1656`) — **trùng vị trí bit 100%** [TOOL] | bật/tắt mã hoá TX/giải mã RX bằng phần cứng, chính sách default key |
| `0x0100` B1 | `REG_CR.MAC_SEC_EN` | | | bật khối security của MAC |

CAM (Content Addressable Memory) là **bộ nhớ trong chip** chứa khoá + địa chỉ MAC của từng STA; nó không nằm trên bus thanh ghi — chỉ truy cập được qua 3 thanh ghi cửa sổ trên. Đây là ví dụ thứ 7 của "không gian gián tiếp" (cùng RF, USB2/3-PHY, eFuse, LTE-coex, TSF-port).

## 2. Giao thức truy cập (`hal/hal_com.c:2506-2586`)

```c
/* đọc 1 dword */                                      /* ghi 1 dword */
lock(sec_cam_access_mutex);                            lock(sec_cam_access_mutex);
W32(REG_CAMCMD, CAM_POLLINIG | addr);                  W32(REG_CAMWRITE, wdata);
                                                       W32(REG_CAMCMD, CAM_POLLINIG | CAM_WRITE | addr);
do {                                                   do {
   if surprise_removed: break                             if surprise_removed: break
} while (R32(REG_CAMCMD) & CAM_POLLINIG             } while (R32(REG_CAMCMD) & CAM_POLLINIG
         && elapsed <= 200 ms);                                && elapsed <= 200 ms);
rdata = R32(REG_CAMREAD);                              unlock();
unlock();
```

- `SECCAM_POLLING` được host đặt 1 để ra lệnh và **phần cứng xoá về 0 khi xong** [SOURCE-SEQ].
- Vòng chờ **không có delay**: mỗi vòng là 1 control transfer (CAMCMD = 0x670 > 0xFF nên không kèm ghi 0x4E0, và < 0x1000 nên có retry). Timeout `SEC_CAM_ACCESS_TIMEOUT_MS = 200` (`hal_com.c:2499`).
- Hết thời gian: vẫn đọc `REG_CAMREAD` và trả về (giá trị có thể cũ); chỉ in log khi `timeout` hoặc `DBG_SEC_CAM_ACCESS` [SOURCE].

## 3. Bố cục một entry — 8 dword (`rtw_sec_write_cam_ent`, `:2636-2690`)

`addr = (id << 3) + j`, `j = 7 … 0` (ghi **ngược**, dword 0 cuối cùng):

| j | Nội dung | Phép ghép (little-endian) |
|---|---|---|
| 0 | `ctrl[15:0]` + MAC[0..1] | `ctrl \| mac[0]<<16 \| mac[1]<<24` |
| 1 | MAC[2..5] | `mac[2] \| mac[3]<<8 \| mac[4]<<16 \| mac[5]<<24` |
| 2–5 | khoá 16 byte | `key[4(j-2)+0..3]` |
| 6–7 | 0 | |

[INFERENCE có cơ sở] Ghi dword 0 (chứa `ctrl`, trong đó có bit "valid" — `CAM_VALID = BIT(15)` ở `hal_com_reg.h`) **sau cùng** để entry chỉ trở nên hợp lệ khi khoá và MAC đã nằm sẵn — tránh phần cứng dùng khoá dở dang. `rtw_sec_clr_cam_ent` chỉ ghi `addr = id<<3` = 0 (xoá ctrl → invalid) mà không xoá khoá.

Ví dụ ghi entry id 5 [tính tay]: địa chỉ CAM `0x2F, 0x2E, …, 0x28`; mỗi dword = 1 `W32(0x674)` + 1 `W32(0x670, 0x8001_00xx)` + ≥1 `R32(0x670)` → **≥ 24 control transfer** cho một khoá.

Giới hạn địa chỉ: field `SECCAM_ADDR_V2` 10 bit, nhưng các hàm nhận `u8 addr`. Phần cứng có `SEC_CAM_NUM_8822B = 64` entry (`halmac_8822b_cfg.h:102`) → cần tới địa chỉ 511; driver giới hạn `cam_ctl.num = min(64, SEC_CAM_ENT_NUM_SW_LIMIT = 32)` (`hal/hal_intf.c:183`, `include/drv_conf.h:495`) → id ≤ 31 → địa chỉ ≤ 255 vừa khít `u8`. Nếu nâng giới hạn SW lên > 32, `(id << 3)` sẽ tràn `u8` và ghi nhầm entry [SOURCE + tính tay].

## 4. `REG_SECCFG` — hai đường cài đặt

| Đường | Hàm | Thao tác |
|---|---|---|
| `HW_VAR_SEC_CFG` | `SetHwReg` (`hal/hal_com.c:14895-14921`) | `R16`; đặt `CHK_KEYID \| RXDEC \| TXENC`; xoá TXENC nếu `sw_encrypt`, RXDEC nếu `sw_decrypt`; `CHK_BMC` nếu capability; `NOSKMC` nếu cờ "PK/GK conflict"; chỉ ghi `W16` nếu khác giá trị cũ |
| `HW_VAR_SEC_DK_CFG` | `hw_var_set_sec_dk_cfg` (`hal/rtl8822b/rtl8822b_ops.c:1715-1734`) — bản 8822B, được `rtl8822b_sethwreg` gọi (`:2096-2098`) | bật: `TXBCUSEDK`; nếu không phải 802.1X: thêm `RXUHUSEDK \| TXUHUSEDK`. Tắt: xoá cả 4 bit DK. Ghi `W8` rồi log `"[HW_VAR_SEC_DK_CFG] 0x680=…"` |
| (trùng lặp) | `SetHwReg` case `HW_VAR_SEC_DK_CFG` (`hal_com.c:14923-14935`) | cùng logic, tên legacy — chỉ tới được nếu `rtl8822b_sethwreg` không xử lý case này [INFERENCE: thứ tự switch] |
| WoWLAN / GTK offload | `rtw_hal_fw_sync_cam_id()` (`hal_com.c:5300`, sau khi hoán đổi CAM id cho 4 key) → `W8(REG_SECCFG, 0xCC)`; `rtw_hal_update_gtk_offload_info()` (`:5404`, sau `rtw_clean_dk_section`) → `W8(REG_SECCFG, 0x0C)` | ghi đè cả byte |

Giải mã hai giá trị [tính tay]: `0xCC = 1100 1100₂` → `RXBCUSEDK[7]`, `TXBCUSEDK[6]`, `RXDEC[3]`, `TXENC[2]`; `0x0C` → chỉ `RXDEC`, `TXENC`.

`config_security_88xx` (HALMAC, `halmac_cfg_wmac_88xx.c:830`, đặt `MAC_SEC_EN`) **không có nơi gọi** trong build [TOOL]; `MAC_SEC_EN` được đặt bởi `rtl8822b_init_misc` (`hal/rtl8822b/rtl8822b_halinit.c:285`: `W16(REG_CR, R16(REG_CR) | BIT_MAC_SEC_EN)`).

## 5. Lỗi có thể gặp

| Hiện tượng | Kiểm tra |
|---|---|
| Kết nối WPA2 xong nhưng không truyền được dữ liệu | `0x680` có `TXENC/RXDEC`? CAM entry đúng MAC? (đọc qua proc debug `cam`) |
| Log `…timeout:1…` từ `rtw_sec_read/write_cam` | `SECCAM_POLLING` không về 0 trong 200 ms — MAC tắt/treo |
| Khoá group lẫn khoá pairwise | `NOSKMC`, `CHK_BMC` |
