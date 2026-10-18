# 05.02 — WMAC: cấu hình RX, bộ lọc frame, địa chỉ MAC/BSSID, retry và rate (MAC 0x0400–0x07FF)

> Nhãn như 05.01. Mọi truy cập: [A03](appendix/A03_mac_300_4ff.md), [A05](appendix/A05_mac_600_7ff.md); giá trị mặc định bảng MAC_REG: [A11](appendix/A11_mac_reg_table.md).

## 1. Thanh ghi chính

| Địa chỉ                          | Tên                                             | Vai trò                                                                              | Ai ghi                                                                                                                            |
| -------------------------------- | ----------------------------------------------- | ------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------- |
| `0x0608`                         | `REG_RCR_8822B`                                 | **RX configuration**: nhận frame loại nào, kiểm tra BSSID, đính kèm FCS/ICV/MIC…     | `init_wmac_cfg_8822b`, `hw_var_rcr_config` (HW_VAR_RCR), `rtw_hal_rcr_set_chk_bssid`, `cfg_drv_info_8822b`, eFuse pre/post update |
| `0x0604`                         | `REG_TCR_8822B`                                 | TX configuration                                                                     | `init_wmac_cfg_8822b`: `+1 = +2 = 0x30`; `rtw_lps_state_chk` đọc                                                                  |
| `0x060C`                         | `REG_RX_PKT_LIMIT_8822B`                        | kích thước gói RX tối đa                                                             | `WLAN_RXPKT_MAX_SZ_512 = 12288 >> 9 = 24` (đơn vị 512 B)                                                                          |
| `0x060F`                         | `REG_RX_DRVINFO_SZ_8822B`                       | kích thước driver-info gắn trước mỗi gói RX (đơn vị 8 B)                             | `cfg_drv_info_8822b`, `init_trx_cfg_8822b`                                                                                        |
| `0x06A0`                         | `REG_RXFLTMAP0_8822B`                           | 16 bit: nhận management frame subtype 0–15                                           |                                                                                                                                   |
| `0x06A2`                         | `REG_RXFLTMAP1_8822B`                           | 16 bit: control frame subtype                                                        |                                                                                                                                   |
| `0x06A4`                         | `REG_RXFLTMAP2_8822B`                           | 16 bit: data frame subtype                                                           | join/scan/disconnect (§3)                                                                                                         |
| `0x0610`, `0x0700`               | `REG_MACID_8822B`, `REG_MACID1_8822B`           | địa chỉ MAC của port 0 / 1 (6 byte: W32 + W16)                                       | `cfg_mac_addr_88xx` (qua `rtw_halmac_set_mac_address`), `get_mac_addr_88xx`                                                       |
| `0x0618`, `0x0708`               | `REG_BSSID_8822B`, `REG_BSSID1_8822B`           | BSSID port 0 / 1                                                                     | `cfg_bssid_88xx` (offset theo port, St R vì qua biến)                                                                             |
| `0x0620`                         | `REG_MAR_8822B`                                 | multicast address filter (8 byte)                                                    | `cfg_multicast_addr_88xx`                                                                                                         |
| `0x042A`                         | `REG_RETRY_LIMIT_8822B`                         | `SRL[13:8]`, `LRL[5:0]`                                                              | `hw_var_set_mlme_join`                                                                                                            |
| `0x0440`                         | `REG_RRSR_8822B`                                | `RRSC_BITMAP[19:0]` (rate dùng để trả lời ACK/CTS), `RRSR_BW[20]`, `RRSR_RSC[22:21]` | `phydm_rrsr_set_register`, `phydm_ra_info_init`, `rtw_hal_beamforming_config_csirate`; MAC_REG                                    |
| `0x0430–0x0437`                  | `REG_DARFRC_8822B` (64-bit)                     | số lần thử mỗi bậc rate fallback (`DARF_RC1..8`, 5 bit mỗi field)                    | `phydm_ra_dynamic_retry_count`; MAC_REG (`0x430..0x437 = 00 00 00 01 04 05 07 08`)                                                |
| `0x0444–0x044F`, `0x049C–0x04AB` | `REG_ARFR0/1/4/5`                               | bitmap rate cho fallback                                                             | chỉ MAC_REG                                                                                                                       |
| `0x0454`                         | `REG_CCK_CHECK_8822B`                           | chặn CCK khi ở 5 GHz                                                                 | `cfg_ch_88xx`, `config_phydm_switch_band_8822b`                                                                                   |
| `0x0455`, `0x0458`               | `REG_AMPDU_MAX_TIME_V1`, `REG_AMPDU_MAX_LENGTH` | giới hạn A-MPDU                                                                      | `init_protocol_cfg_8822b`, `cfg_ampdu_8822b`, `HW_VAR_AMPDU_MAX_TIME` (= `0x70`)                                                  |
| `0x0480`                         | `REG_INIRTS_RATE_SEL_8822B`                     | rate RTS ban đầu                                                                     | `SetHwReg`, `init_protocol_cfg_8822b`                                                                                             |
| `0x0664`                         | `REG_RXERR_RPT_8822B`                           | bộ đếm lỗi RX (chọn loại qua `_RXERR_RPT_SEL`)                                       | `_rx_report`, `rtw_reset_mac_rx_counters`                                                                                         |
| `0x0698`, `0x069C`               | `REG_WKFMCAM_CMD/RWD_8822B`                     | CAM mẫu đánh thức WoWLAN (cửa sổ gián tiếp)                                          | `_rtw_wow_pattern_*_cam`                                                                                                          |


## 1.2. Giải thích thanh ghi `REG_RCR_8822B` 

|Từ|Nghĩa đơn giản|
|---|---|
|**Thanh ghi (register)**|Một "ô nhớ" 32 bit nằm **bên trong chip Wi-Fi**. Driver ghi giá trị vào đó để ra lệnh cho phần cứng, và đọc ra để biết trạng thái.|
|**0x0608**|**Địa chỉ** của ô nhớ đó, như số nhà.|
|**REG_RCR_8822B**|Tên gọi trong code. REG = register. **RCR = Receive Configuration Register** (thanh ghi cấu hình nhận). 8822B = tên dòng chip (RTL8822B).|
|**RX**|Receive, tức phía nhận.|
|**frame**|Một "gói tin" Wi-Fi bay trong không khí.|
|**BSSID**|Địa chỉ MAC của router/AP mà bạn đang kết nối. Nó là "danh tính" của mạng Wi-Fi đó.|
|**FCS / ICV / MIC**|Các đoạn mã kiểm tra lỗi hoặc bảo mật ở đuôi frame. Xem mục 3 bên dưới.|

### Thanh ghi này làm gì? Hình dung "bảo vệ cổng"
Không khí xung quanh bạn có **rất nhiều** gói Wi-Fi bay qua: của hàng xóm, của các router khác, gói gửi cho máy khác. Anten của chip nghe thấy tất cả.

`RCR` là **bộ lọc ở cổng vào**. Nó cho chip biết:

> "Gói nào thì nhận vào đưa cho driver, gói nào thì bỏ qua?"

Nếu không có bộ lọc, driver sẽ bị ngập trong hàng nghìn gói không liên quan đến nó, làm tốn CPU và điện. Ngược lại, nếu lọc quá chặt thì mất gói cần thiết (không scan được, không nhận được dữ liệu).

### Mỗi bit là một "công tắc"

Thanh ghi 32 bit, mỗi bit là một công tắc bật/tắt. Một vài công tắc quan trọng (mô tả bên dưới là diễn giải theo tên bit và ngữ cảnh. **Tôi chưa đọc datasheet chính thức**, nên ý nghĩa chính xác cần đối chiếu thêm):

**Nhóm "nhận gói gửi cho ai":**

|Bit|Tên|Ý nghĩa (suy từ tên)|
|---|---|---|
|0|`AAP`|Accept All Packets: nhận **mọi** gói. Dùng cho chế độ monitor/sniffer.|
|1|`APM`|Accept Physical Match: nhận gói gửi **đúng địa chỉ MAC của mình**.|
|2|`AM`|Accept Multicast: nhận gói gửi cho một nhóm.|
|3|`AB`|Accept Broadcast: nhận gói gửi cho tất cả.|

**Nhóm "kiểm tra nguồn gốc":**

|Bit|Tên|Ý nghĩa|
|---|---|---|
|4|`ADD3`|Kiểm tra trường địa chỉ thứ 3 trong frame.|
|6|`CBSSID_DATA`|Check BSSID cho frame **dữ liệu**: chỉ nhận nếu đến từ AP mình đang kết nối.|
|7|`CBSSID_BCN`|Check BSSID cho frame **beacon** (gói quảng bá của router).|

**Nhóm "bỏ gói lỗi":**

|Bit|Tên|Ý nghĩa|
|---|---|---|
|8|`ACRC32`|Có nhận cả gói sai CRC32 (hỏng) hay không.|
|9|`AICV`|Có nhận cả gói sai ICV (giải mã/toàn vẹn lỗi) hay không.|

**Nhóm "gắn thêm phần đuôi vào gói trả về driver":**

|Bit|Tên|Ý nghĩa|
|---|---|---|
|28|`APP_PHYSTS`|Gắn thêm thông tin tầng PHY (cường độ tín hiệu, tốc độ…).|
|29|`APP_ICV`|Giữ lại trường ICV ở đuôi.|
|30|`APP_MIC`|Giữ lại trường MIC.|
|31|`APP_FCS`|Giữ lại trường FCS (4 byte kiểm tra lỗi cuối frame).|
### FCS, ICV, MIC là gì?
Cuối mỗi frame Wi-Fi thường có vài byte "kiểm tra":
- **FCS** (Frame Check Sequence): mã kiểm tra lỗi, để biết gói có bị nhiễu làm hỏng khi truyền không.
- **ICV** (Integrity Check Value): kiểm tra toàn vẹn của dữ liệu đã mã hóa, dùng trong WEP/TKIP.
- **MIC** (Message Integrity Code): mã chống giả mạo/chỉnh sửa gói, dùng trong mã hóa WPA.
Thông thường phần cứng tự kiểm tra rồi **cắt bỏ** các đuôi này trước khi trao cho driver. Các bit `APP_*` (append) cho phép **giữ lại** đuôi để driver tự xem, ví dụ khi debug hay làm sniffer.

### Vì sao driver phải ghi thanh ghi này?
Vì nhu cầu lọc **thay đổi theo hoàn cảnh**:

|Tình huống|RCR nên thế nào|
|---|---|
|Chưa kết nối, đang scan|Phải nhận beacon từ **mọi** AP để liệt kê mạng, nên **không** check BSSID.|
|Đã kết nối AP|Chỉ cần gói từ AP của mình, **bật** check BSSID để bỏ gói lạ.|
|Chế độ monitor|Nhận hết, bật `AAP`.|
|Debug|Giữ thêm FCS/ICV/MIC.|

Do đó nhiều hàm trong code cùng chạm vào thanh ghi này.

### Các hàm ở cuối dòng của bạn là gì?

Đây là danh sách các nơi trong driver **ghi/chỉnh** `REG_RCR`:

| Hàm                                | Vai trò (suy từ tên, chưa đọc từng hàm)                                                        |
| ---------------------------------- | ---------------------------------------------------------------------------------------------- |
| `init_wmac_cfg_8822b`              | Khởi tạo cấu hình MAC không dây lúc chip bật lên. Gán giá trị RCR ban đầu.                     |
| `hw_var_rcr_config` (`HW_VAR_RCR`) | Cổng chung để phần khác của driver nhờ HAL "đặt RCR thành giá trị này".                        |
| `rtw_hal_rcr_set_chk_bssid`        | Bật/tắt riêng cờ check BSSID, ví dụ khi bắt đầu hoặc kết thúc scan, kết nối.                   |
| `cfg_drv_info_8822b`               | Cấu hình việc gắn thông tin PHY (`APP_PHYSTS`) vào gói.                                        |
| eFuse pre/post update              | Trước/sau khi đọc eFuse (bộ nhớ cố định trong chip), thanh ghi này được tạm đổi rồi khôi phục. |

## 2. RCR mặc định — `WLAN_RCR_CFG = 0xE400220E` (`halmac_init_8822b.c:115`, ghi ở `:951`)

Giải bằng `tools/bitname.py` [TOOL]:

| Bit | Field (header) | = 1 |
|---|---|---|
| 31 | `APP_FCS` | gắn FCS vào gói RX đưa lên host |
| 30 | `APP_MIC` | gắn MIC |
| 29 | `APP_ICV` | gắn ICV |
| 26 | `VHT_DACK` | |
| 13 | (không có tên trong section) | |
| 9 | `AICV` | nhận cả gói lỗi ICV [INFERENCE theo tên] |
| 3 | `AB` | nhận broadcast |
| 2 | `AM` | nhận multicast |
| 1 | `APM` | nhận gói gửi đúng địa chỉ mình (physical match) |

Ý nghĩa "A*" = "Accept …" là [INFERENCE theo quy ước tên Realtek]; chỉ tên field là [SOURCE]. Bit `CBSSID_DATA[6]`, `CBSSID_BCN[7]` (kiểm tra BSSID cho data/beacon) **không** bật lúc init mà do `rtw_hal_rcr_set_chk_bssid()` quyết định theo trạng thái (§3).

`hw_var_rcr_config()` (`hal/hal_com.c:3748-3756`) ghi `W32(REG_RCR, rcr)` và **chỉ khi ghi thành công** mới cập nhật bản sao phần mềm `HalData->ReceiveConfig`; `hw_var_rcr_get()` đọc lại phần cứng và cập nhật bản sao.

## 3. Bộ lọc RX thay đổi theo trạng thái liên kết

Khởi tạo (`init_wmac_cfg_8822b`, `:948-949`): `W32(REG_RXFLTMAP0, 0x0FFFFFFF)` → RXFLTMAP0 = `0xFFFF` (mọi mgmt subtype) **và** RXFLTMAP1 = `0x0FFF` (ctrl subtype 0–11; 12–15 bị lọc) vì ghi 32-bit trải sang `0x6A2` [tính tay]; `W16(REG_RXFLTMAP2, 0xFFFF)` (mọi data subtype).

Sau đó RXFLTMAP2 (data) bật/tắt theo MLME (`hal/rtl8822b/rtl8822b_ops.c`):

| Sự kiện | Thao tác | Dòng |
|---|---|---|
| bắt đầu scan | `RXFLTMAP2 = 0xFFFF` (`CONFIG_FIND_BEST_CHANNEL` bật); `rcr_set_chk_bssid(SCAN_ENTER)` | `:1535-1549` |
| kết thúc scan | nếu đang associated/AP/mesh: `RXFLTMAP2 = 0xFFFF`; `rcr_set_chk_bssid(SCAN_DONE)` | `:1559-1563` |
| join type 0 (bắt đầu) | `RXFLTMAP2 = 0xFFFF`; bật beacon func, `rx_tsf_addr_filter_config(CHK_TSF_EN \| CHK_TSF_CBSSID)` | `:1593-1611` |
| join type 1 (thất bại/kết thúc) | nếu không còn link nào: `RXFLTMAP2 = 0` | `:1615-1616` |
| disconnect | nếu không còn link: `RXFLTMAP2 = 0`; reset TSF port | `:1474-1503` |

`rtw_hal_rcr_set_chk_bssid()` (`hal/hal_com.c:3816-3926`) — quy tắc cuối cùng [SOURCE]:

```
CBSSID_DATA = 0  nếu (có AP | mesh | TDLS linked | đang scan (FIND_BEST_CHANNEL) | CTA test)
            = 1  ngược lại
CBSSID_BCN  = 0  nếu (đang scan | CTA test)
            = 1  nếu (STA đang kết nối | periodic TSF update)
            = 0  nếu (AP + wifi_spec | mesh)
            = 1  ngược lại
→ nếu RCR thay đổi: (khi không có rx_tsf_filter và CBSSID_BCN 1→0) tạm dừng cập nhật TSF; HW_VAR_RCR
```

[INFERENCE] Khi scan phải tắt kiểm tra BSSID để nhận beacon/probe-response của **mọi** AP; khi đã kết nối thì bật để chặn traffic của BSS khác. Sai bit → scan không thấy AP, hoặc nhận rác từ BSS khác.

## 4. Retry limit khi join

`hw_var_set_mlme_join()` (`:1581-1648`): `RetryLimit = RL_VAL_STA (0x30)` cho STA (trừ customer CCX), `RL_VAL_AP (7)` cho ad-hoc/AP (`include/hal_com_reg.h:1553-1557`); cuối hàm:

```c
val16 = BIT_LRL_8822B(RetryLimit) | BIT_SRL_8822B(RetryLimit);   /* LRL[5:0], SRL[13:8] */
rtw_write16(adapter, REG_RETRY_LIMIT_8822B, val16);              /* STA: 0x3030 → 48 lần cả short & long */
```

`0x3030` [tính tay]: `(0x30 & 0x3F) << 0 | (0x30 & 0x3F) << 8`. Nhánh type 2 + ad-hoc: `rtw_write8(adapter, 0x542, 0x02)` — **literal** 0x542 = byte 2 của `REG_TBTT_PROHIBIT` (05.06) — không có comment [SOURCE].

## 5. Địa chỉ MAC / BSSID theo port

`cfg_mac_addr_88xx(port)` (`halmac_cfg_wmac_88xx.c:86-125`): port 0→`REG_MACID` (0x610), 1→`REG_MACID1` (0x700), 2–4→`REG_MACID2..4`; ghi `W32(offset, addr[0..3])` + `W16(offset+4, addr[4..5])` (little-endian). `cfg_bssid_88xx` tương tự với `REG_BSSID*`. Driver gọi qua `rtw_halmac_set_mac_address/bssid()` (`hal/hal_halmac.c:2131-2190`), từ `HW_VAR_MAC_ADDR/BSSID` (ví dụ đổi port trong concurrent mode `rtl8822b_ops.c:1937-1955`). Vì offset là biến, danh mục xếp `REG_BSSID*` ở St R.

## 6. Lỗi có thể gặp

| Triệu chứng | Thanh ghi |
|---|---|
| Scan không thấy AP | `RCR.CBSSID_BCN` (0x608 bit 7) vẫn 1 khi scan; `RXFLTMAP0` |
| Kết nối được nhưng không nhận data | `RXFLTMAP2` = 0 (còn sót trạng thái disconnect) |
| Nhận gói của BSS khác | `CBSSID_DATA` = 0 |
| Gói RX bị cắt | `RX_PKT_LIMIT` (0x60C), `RX_DRVINFO_SZ` (0x60F) |
| Retry quá nhiều / quá ít | `0x42A` |
