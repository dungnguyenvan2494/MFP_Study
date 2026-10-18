# 05.13 — Baseband (BB/PHY): bảng init, chuyển kênh/băng thông/băng tần, DIG, bộ đếm FA/CCA, TRX path, RFE pinmux, CCX

> Nhãn như 05.01. Không gian BB = `0x0800–0x0FFF` và `0x1800–0x1FFF` trên cùng bus thanh ghi (02 §2). Mọi truy cập theo dải: [A07](appendix/A07_bb_800_fff.md), [A08](appendix/A08_bb_1800_1fff.md). Danh mục: [04b_catalog_bb.md](../04b_catalog_bb.md) (968 entry gồm BB 785 + RF-direct 32 + BB-pg 24 + other 127; BB: 501 trạng thái A).
> Mã nguồn chính: `hal/phydm/rtl8822b/phydm_hal_api8822b.c` (viết tắt **api**), `hal/phydm/phydm_dig.c`, `hal/phydm/phydm_ccx.c`, `hal/phydm/rtl8822b/halhwimg8822b_bb.c` (**bbimg**), `hal/phydm/rtl8822b/phydm_regconfig8822b.c` (**regcfg**).

## 1. Mô hình truy cập BB

| Thao tác | Hàm | Số control transfer USB |
|---|---|---|
| `odm_set_bb_reg(dm, addr, MASKDWORD, v)` | → `rtl8822b_write_bb_reg` (`hal/rtl8822b/rtl8822b_phy.c:668-685`) | **1** (`W32`) |
| `odm_set_bb_reg(dm, addr, mask≠0xFFFFFFFF, v)` | đọc `R32` → `(old & ~mask) \| ((v << shift) & mask)` → `W32` | **2** |
| `odm_get_bb_reg(dm, addr, mask)` | `R32` rồi `& mask >> shift` | 1 |
| `odm_read_4byte` / `odm_write_4byte` | thẳng `rtw_read32/write32` | 1 |

- Mọi thanh ghi BB đều được truy cập bằng 32 bit, kể cả khi chỉ cần 1 bit. Hệ quả: một hàm như `phydm_igi_toggle_8822b` (4 lần ghi field 7 bit) tốn 8 transfer, cộng thêm 1 lần đọc ban đầu [SOURCE + tính tay].
- Dải `0x800–0xFFF` nằm dưới `0x1000`, nên mỗi transfer lỗi được thử lại tối đa 10 lần (03 §3.4).
- Dải `0x1800–0x1FFF` chỉ được thử 1 lần.
- Không thanh ghi BB nào nằm trong dải kích hoạt việc ghi kèm `0x4E0` (dải đó là `0x00–0xFF` và `0x1000–0x10FF`).
- `DISABLE_BB_RF == 1` sẽ biến mọi lần ghi BB thành no-op (`:673`). Build này không bật macro đó (01 §3).
- Các tên dùng trong code thuộc ba hệ thống [TOOL]:
  - `R_0xNNN`: tên literal, chỉ mã hoá địa chỉ, không mang ý nghĩa.
  - `ODM_REG_*_11AC`: tên theo chức năng, được chọn qua `ODM_REG(x, dm)` → `_11AC` (01 §5).
  - `rXXX_Jaguar`: tên legacy.

  Danh mục gom cả ba hệ về cùng một địa chỉ (cột `names`).

## 2. Bảng khởi tạo BB: PHY_REG và AGC_TAB

### 2.1 Thứ tự và nguồn

`init_bb_reg()` → `_init_bb_reg()` (`hal/rtl8822b/rtl8822b_phy.c:134-219`):

```
1. PHY_REG : phy_ConfigBBWithParaFile(PHY_FILE_PHY_REG)   — chỉ khi rtw_load_phy_file có LOAD_BB_PARA_FILE (BIT1)
             nếu không → odm_config_bb_with_header_file(CONFIG_BB_PHY_REG) → odm_read_and_config_mp_8822b_phy_reg
   (PHY_REG_MP: #if 0 — "No parameter with MP using currently by BB@Stanley", :160)
2. AGC_TAB : tương tự, CONFIG_BB_AGC_TAB → odm_read_and_config_mp_8822b_agc_tab
3. rtw_phydm_set_crystal_cap(hal->crystal_cap)            — xem 15 (calibration)
```

Mặc định `rtw_load_phy_file = BIT0|BIT2|BIT6` (01 §6) **không có BIT1**. Vì vậy PHY_REG và AGC_TAB luôn lấy từ mảng trong `bbimg`, không đọc từ `/lib/firmware` [SOURCE: `include/hal_com_phycfg.h:380-386`].

### 2.2 Thống kê (tools/tables.py [TOOL])

| Bảng | Entry | Số địa chỉ khác nhau | Điều kiện |
|---|---|---|---|
| `array_mp_8822b_phy_reg` | 1492 | 455 | không có — mọi entry vô điều kiện |
| `array_mp_8822b_agc_tab` | 9839 | 3 (`0x81C`, `0xC50`, `0xE50`) | 19 nhánh `IF/ELSE_IF/ELSE` theo **rfe_type** (0–16, 18, else) |

Phân bố PHY_REG theo trang:

| Trang | Số entry |
|---|---|
| `0x800` | 64 |
| `0x900` | 64 |
| `0xA00` | 27 |
| `0xB00` | 36 |
| `0xC00` | 83 |
| `0xE00` | 83 |
| `0x1900` | 64 |
| `0x1B00` | 1007 |
| `0x1C00` | 64 |

**Phát hiện [TOOL + SOURCE]:** 1007 entry cuối của PHY_REG (chỉ số 485–1491) **trùng từng cặp** `(addr, value)` với mảng `array_mp_8822b_cal_init[]` trong `hal/phydm/halrf/rtl8822b/halrf_rfk_init_8822b.c:70`. Mảng đó nằm trong `#if 0` (`:69-1079`) nên chết, và hàm nạp `odm_read_and_config_mp_8822b_cal_init` (`:1081`) cũng có thân rỗng (`#if 0` `:1083-1098`). Như vậy:

- Chương trình khởi tạo calibration được nạp qua bảng PHY_REG.
- 975 trong số 1007 entry đó là các lần ghi liên tiếp vào cùng một thanh ghi `0x1B80`.
- Giá trị ghi vào `0x1B80` đi theo cặp `…5` / `…7`, ví dụ `0x090A0005`, `0x090A0007`. [INFERENCE] Đây trông như một cổng nạp lệnh với bit 1 làm strobe, nhưng source không có comment hay tên nào cho `0x1B80`, nên **ý nghĩa: không xác định**.

Chi tiết trang `0x1B00` (IQK) để ở 15.

Không có entry giả delay `0xF9–0xFE` nào trong PHY_REG 8822B. Dù vậy, `odm_config_bb_phy_8822b` (`regcfg:230-291`) vẫn xử lý chúng:

| Địa chỉ giả | Delay |
|---|---|
| `0xFE` | 50 ms |
| `0xFD` | 5 ms |
| `0xFC` | 1 ms |
| `0xFB` | 50 µs |
| `0xFA` | 5 µs |
| `0xF9` | 1 µs |

Khi bật `PHYDM_PHY_PARAM_OFFLOAD`, các delay này sẽ chuyển thành lệnh HALMAC. Build này tắt offload đó (01 §6), nên mỗi entry là một `W32` trực tiếp. Toàn bộ PHY_REG là 1492 control transfer [tính tay: 1492 × MASKDWORD].

### 2.3 Điều kiện bảng — `check_positive()` (`bbimg:64-130`)

Driver ghép trạng thái phần cứng thành `driver1`:

```
driver1 = cut[27:24] | (intf & 0xF0)<<16 | platform<<16 | pkg[15:12] | (intf & 0x0F)<<8 | rfe_type[7:0]
```

`cond1` lấy từ bảng. Các trường cut, pkg và intf chỉ được so khi `cond1` khác 0 ở trường đó. Cuối cùng hàm so **8 bit thấp**: `cond1[7:0] == rfe_type`.

`rfe_type` (`Hal_ReadRFEType`, `hal/rtl8822b/rtl8822b_ops.c:560-591`) được chọn theo thứ tự:

1. module param `rtw_RFE_type`, nếu khác `CONFIG_RTW_RFE_TYPE`;
2. nếu không, eFuse byte `0xCA` (`EEPROM_RFE_OPTION_8822B`, `include/hal_pg.h:517`), nếu khác `0xFF`;
3. nếu không, gán `rfe_type = 0`, in `"Empty EFUSE with unknown REF type!!"` và **trả `_FAIL`** → `rtl8822b_read_efuse` thất bại → probe USB dừng (`os_dep/linux/usb_intf.c:1434-1435`, 07 §8). Nghĩa là driver **không** chạy tiếp với rfe 0; muốn dùng board eFuse trống phải nạp `rtw_RFE_type=<n>`.

[KIỂM TRA TRÊN BOARD] rfe_type thực tế được in qua `RTW_INFO("EEPROM rfe_type=0x%x")`. Toàn bộ bảng AGC, RFE pinmux (§7), CCA (`ccapar_by_rfe`) và RF RadioA/B (14) đều phụ thuộc giá trị này.

### 2.4 Định dạng entry AGC `0x81C` (`odm_update_agc_big_jump_lmt_8822b`, `regcfg:158-188`)

| Bit | Field (theo tên biến trong source) |
|---|---|
| `[31:24]` | `rf_gain_idx` |
| `[23:16]` | `bb_gain_idx` |
| `[11:8]` | `agc_table_idx` |
| `[7:0]` | luôn `0x03` trong bảng (517/517 entry của nhánh rfe 0) — không có tên |

- Mỗi nhánh rfe có 517 entry, chia thành **8 bảng con**: `agc_table_idx` 0–7, mỗi bảng 64 hoặc 65 entry [TOOL].
- Cùng lúc ghi, driver tính `big_jump_lmt[agc_table_idx]`. Khi `bb_gain_idx > 0x3C` lần đầu trong khi `rf_gain_idx` không đổi, giới hạn được đặt bằng `bb_gain_idx − 2`. DIG dùng giá trị này để giới hạn bước nhảy của IGI.
- Biến `is_limit` là `static` trong hàm. [INFERENCE] Vì vậy trạng thái sẽ dính sang lần nạp bảng tiếp theo nếu bảng trước kết thúc khi `is_limit = true`. Thực tế, entry cuối của mỗi bảng con có `bb_gain_idx` nhỏ, nên biến được reset.
- Nhánh rfe 0, 13, 14 và `ELSE` kết thúc bằng `W(0xC50, 0x22)`, `W(0xC50, 0x20)`, `W(0xE50, 0x22)`, `W(0xE50, 0x20)`. Đây là IGI toggle, cùng cách làm với `phydm_igi_toggle_8822b` (§4). [INFERENCE] Mục đích là để phần cứng nạp lại gain sau khi đổi bảng.

Bảng nào đang dùng: `0x958[4:0]` chọn bảng AGC (§3.1). Với rfe 15/16 ("Microsoft case"), `phydm_rxagc_switch_8822b` (`api:1776-1830`) thêm cách chọn riêng cho path B qua `0xC1C[11:8]` / `0xE1C[11:8]`:

| Băng 5 GHz | `0xC1C[11:8]` | `0xE1C[11:8]` |
|---|---|---|
| ch 36–64 | 1 | 5 |
| ch 100–144 | 2 | 6 |
| ch ≥ 149 | 3 | 7 |

## 3. Chuyển kênh / băng thông / băng tần

Mô phỏng bằng `tools/chbw_sim.py` [TOOL]. Script dựng lại đúng phép tính trong `config_phydm_switch_channel_8822b` (`api:1839-2030`) và `config_phydm_switch_bandwidth_8822b`. Giá trị ban đầu của các thanh ghi không biết trên phần cứng thật nên mặc định là 0.

### 3.1 Kênh — `config_phydm_switch_channel_8822b`

| Thanh ghi | 2.4 GHz | 5 GHz | Comment trong source |
|---|---|---|---|
| RF `0x18[7:0]` | số kênh | số kênh | — |
| RF `0x18[18:17]` | 00 | `BIT17` nếu 80 ≤ ch ≤ 144; `BIT18` nếu ch > 144 | — |
| BB `0x958[4:0]` | 0 | 1 (36–64), 2 (100–144), 3 (≥149); **bỏ qua khi rfe 15/16** | "AGC table selection" |
| BB `0x860[28:17]` | `0x96A` | `0x494` (36–48), `0x453` (52–64), `0x452` (100–116), `0x412` (118–177) | "clock offset tracking" |
| BB `0xA24` / `0xA28[15:0]` | `0x384F6577` / `0x1525`; ch14: `0x00006577` / `0` | không ghi | "CCK TX filter" |
| RF-A `0xBE[17:15]` | 0 | tra 3 bảng `low/middle/high_band` theo `(ch − base) >> 1`; `0xFF` ⇒ lỗi | — |
| RF-A `0xDF[18]` | 0 | 1 chỉ khi ch = 144 | — |
| RF-A `0xB8[19]` | ghi 0 rồi 1 (`api:2013-2014`) — comment: "Debug for RF resister reading error during synthesizer parameters parsing" | như 2.4G | 14 |

Sau đó hàm gọi `phydm_igi_toggle_8822b`, `phydm_ccapar_by_rfe_8822b` và phần xử lý spur. Kênh 15–35 trả `false` và không ghi gì cả.

Kết quả mô phỏng:

| Trường hợp | RF18 | Các thanh ghi khác |
|---|---|---|
| ch6 BW20 | `0x00C06` | `0x958 = 0`, `0x860 = 0x96A`, `0xA24 = 0x384F6577` |
| ch36 BW80 pri 1 | `0x00424` | `0x958 = 1`, `0x860 = 0x494`, `0x8AC = 0x6`, `0xBE = 0x7` |

### 3.2 Băng thông — `config_phydm_switch_bandwidth_8822b`

| BW | `0x8AC` (mask giữ lại bit cũ) | RF18 `[11:10]` | Thêm |
|---|---|---|---|
| 20 | `old & 0xFFCFFC00` | `11` | — |
| 40 | `old & 0xFF3FF300 \| pri<<2 \| 1` | `10` | `0xA00[4]` = (pri == 1) — CCK primary |
| 80 | `old & 0xFCEFCF00 \| pri<<2 \| 2` | `01` | rfe 2/3/17: `0x840[15:12] = 6`, `0x8C8[10] = 1` |

- Cả ba BW đều đặt `0x8C4[30] = 1` ("ADC buffer clock").
- Kênh 165 luôn bị ép về BW 20.
- `0x8AC[1:0]` là chế độ BW và `[5:2]` là primary channel index. Nguồn là các comment trong khối `#if 0` cùng hàm, nên ghi là [SOURCE-comment].

### 3.3 Băng tần — `config_phydm_switch_band_8822b`

| Thanh ghi | → 2.4 GHz | → 5 GHz |
|---|---|---|
| `0x808[28]` (bật khối CCK) | 1 | 0 |
| MAC `0x454[7]` (`REG_CCK_CHECK`) | 0 | 1 — chặn CCK |
| `0xA80[18]` | 0 | 1 |
| `0x814[15:10]` (CCA mask) | 15 | 34 hoặc 15 |
| RF18 | xoá `BIT16 \| BIT9 \| BIT8` | đặt `BIT16 \| BIT8` |

Cả hai hướng đều gọi thêm `phydm_somlrxhp_setting` (`0xC04/0xE04/0x8CC/0x8D8`, SoML/RxHP).

`0x808[28]` cũng là cờ mà bộ đếm FA (§5) đọc để quyết định có cộng CCK hay không.

## 4. DIG — Dynamic Initial Gain (IGI)

| Thanh ghi | Field | Ghi bởi |
|---|---|---|
| `0xC50` (`ODM_REG_IGI_A_11AC`) | `ODM_BIT_IGI_11AC = 0x7F` → `[6:0]` | `phydm_write_dig_reg_c50` (`phydm_dig.c:536-554`); AGC_TAB; IGI toggle; MCC; `phydm_psd`; `phydm_idle_noise_measure_ac` |
| `0xE50` (`ODM_REG_IGI_B_11AC`) | `[6:0]` | như trên, khi `support_ic_type & PHYDM_IC_ABOVE_2SS` — 8822B là 2SS nên ghi cả hai path |

- Mỗi lần DIG đổi IGI tốn 2 thanh ghi × (R32 + W32) = 4 transfer.
- `phydm_igi_toggle_8822b` (`api:88-97`) làm như sau:
  ```
  igi = R(0xC50)[6:0]
  W(0xC50, igi−2)
  W(0xC50, igi)
  W(0xE50, igi−2)
  W(0xE50, igi)
  ```
  IGI đọc từ path A được ghi cho cả path B.

[INFERENCE] IGI là gain ban đầu của AGC. Nâng IGI giúp nhận tín hiệu yếu nhưng làm tăng false alarm. DIG điều chỉnh IGI dựa trên RSSI và số FA ở §5.

## 5. Bộ đếm False Alarm / CCA / CRC

Đọc bởi `phydm_fa_cnt_statistics_ac` (`phydm_dig.c:1460-1550`). Mọi thanh ghi đều ở trang `0xF` (chỉ đọc) [SOURCE: tên biến]:

| Địa chỉ | Tên | `[15:0]` | `[31:16]` |
|---|---|---|---|
| `0xF48` | `ODM_REG_OFDM_FA_11AC` | `cnt_ofdm_fail` | — |
| `0xA5C` | `ODM_REG_CCK_FA_11AC` | `cnt_cck_fail` | — |
| `0xF08` | `ODM_REG_CCK_CCA_CNT_11AC` | `cnt_cck_cca` | `cnt_ofdm_cca` |
| `0xF04` | `CCK_CRC32_CNT` | ok | error |
| `0xF14` | `OFDM_CRC32_CNT` | ok | error |
| `0xF10` | `HT_CRC32_CNT` | ok | error |
| `0xF0C` | `VHT_CRC32_CNT` | ok | error |
| `0xFCC` | `OFDM_FA_TYPE1` | — | `cnt_fast_fsync` |
| `0xFD0` | `OFDM_FA_TYPE2` | `cnt_sb_search_fail` | — |
| `0xFBC` | `OFDM_FA_TYPE3` | `cnt_parity_fail` | `cnt_rate_illegal` |
| `0xFC0` | `OFDM_FA_TYPE4` | `cnt_crc8_fail` | `cnt_mcs_fail` |
| `0xFC4` | `OFDM_FA_TYPE5` | `cnt_crc8_fail_vhta` | `cnt_crc8_fail_vhtb` (**lỗi**, xem dưới) |
| `0xFC8` | `OFDM_FA_TYPE6` | `cnt_mcs_fail_vht` | — |

Tổng FA được tính như sau:

- Nếu `0x808[28]` (CCK đang bật): `cnt_all = ofdm_fail + cck_fail`, `cnt_cca_all = cck_cca + ofdm_cca`.
- Nếu không: chỉ tính OFDM.

**Lỗi thứ tự toán tử [SOURCE + tính tay]** — `phydm_dig.c:1487`:

```c
fa_t->cnt_crc8_fail_vhtb = ret_value & 0xffff0000 >> 16;
```

Trong C, `>>` có độ ưu tiên cao hơn `&`, nên biểu thức được tính là `ret_value & (0xffff0000 >> 16)` = `ret_value & 0x0000FFFF`. Kết quả là `cnt_crc8_fail_vhtb` luôn bằng `cnt_crc8_fail_vhta`, và nửa cao của `0xFC4` không bao giờ được đọc. Bản JGR3 cùng file (`:490`) viết đúng `(ret_value & 0xffff0000) >> 16`. Ảnh hưởng chỉ giới hạn ở thống kê/debug: biến này không tham gia `cnt_all` nên không ảnh hưởng DIG. Đã ghi vào 09.

Reset, theo thứ tự trong `phydm_false_alarm_counter_reg_reset` (nhánh 11AC, `phydm_dig.c:1314-1340`):

| Bước | Thao tác | Comment |
|---|---|---|
| 1 | `0x9A4[17]`: 1 → 0 | "reset OFDM FA counter" |
| 2 | `0xA2C[15]`: 0 → 1 | "reset CCK FA counter" (cực tính ngược bước 1) |
| 3 | `phydm_reset_bb_hw_cnt` (`phydm_api.c:66-83`): `0xB58[0]`: 1 → 0 | "Reset all counter when 1 (including PMAC and PHY)" — "Page F counter" |

Tổng cộng 6 RMW = 12 transfer mỗi chu kỳ watchdog (`dynamic_chk_timer` 2000 ms, `core/rtw_mlme.c:3449`).

Các khối dùng cùng nhóm thanh ghi:

- `rtw_reset_phy_rx_counters` (core/MP) dùng `0x9A4`/`0xA2C`.
- `rtw_reset_phy_trx_ok_counters` dùng `0xB58`.
- `phydm_8822b_sutxbfer_workaroud` cũng chạm `0xB58`.

## 6. TRX path (chọn anten/đường A/B)

`config_phydm_trx_mode_8822b(tx_path_en, rx_path, tx_path_sel_1ss)` (`api:2470-2593`):

| Thanh ghi | Giá trị | Hàm |
|---|---|---|
| `0xC08[15:0]` / `0xE08[15:0]` | `0x3231` nếu path đó được dùng (TX hoặc RX), ngược lại `0x1111` | trx_mode |
| `0x93C[19:18]` | 3 — "Set TX antenna by Nsts" | `phydm_config_tx_path_8822b` (`:2379`) |
| `0x80C[29:28]` | 1 | như trên |
| `0x80C[30]` | 1 — "Control CCK TX path by 0xa07[7]" | như trên |
| `0x80C[7:0]` | `(tx_path<<4) \| tx_path` — "TX path HW block enable" | như trên |
| `0xA04[31:28]` | 8 = A, 4 = B, 0xC = AB (CCK TX) | `phydm_config_cck_tx_path_8822b` (`:2333`) |
| `0x93C[31:20]` | `0x001` = A, `0x002` = B, `0x043` = AB (OFDM 1ss) | `phydm_config_ofdm_tx_path_8822b` (`:2345`, nhánh `#if 1`) |
| `0x940[15:4]` | `0x043` khi 2 path; khi 1 path: `[7:4] = 1`, `[15:8] = 0` | như trên |
| `0x808[7:0]` | `(rx_path<<4) \| rx_path` | `phydm_config_rx_path_8822b` (`:2433`) |
| `0xA04[27:24]` | 0 = A, 5 = B (CCK RX) | như trên |
| `0xA2C[22]`, `0xA2C[18]` | 0 | như trên |
| `0x1904[16]`, `0x800[28]`, `0x850[23]` | 0 nếu 1 RX path, 1 nếu 2 RX path | như trên |

Tiếp theo là chuỗi RF qua cửa sổ mode table (chi tiết ở 14):

```
RF 0xEF = 0x80000
RF 0x33 = 1
RF 0x3E = 0x34
RF 0x3F = 0x4080E  (MP/antenna test)  hoặc  0x4080C  (thường)
RF 0xEF = 0
```

Nếu RF `0x33` không đọc lại được `0x00001` sau 100 lần thử, hàm in log và bỏ qua (`:2540-2560`).

`CONFIG_PATH_DIVERSITY` (nếu bật) thay đổi cách chọn `tx_path_sel_1ss`. Ngược lại `tx_path_sel_1ss = tx_path_en` (`:2510-2525`).

## 7. RFE pinmux — điều khiển chân Front-End (PA/LNA/T/R switch)

Theo comment trong source: `0xCB0`/`0xEB0` là "signal source", `0xCBC`/`0xEBC` là "inverse or not", `0xCA0`/`0xEA0` là "antenna switch table". Tên legacy `rA_RFE_Pinmux_Jaguar`, `rA_RFE_Inv_Jaguar`, `rA_RFE_Jaguar` và `rA_RFE_Inverse_Jaguar` thuộc các địa chỉ `0xCB0`/`0xCB4`/`0xCB8`/`0xCBC`, path B tương ứng ở `0xEBx`.

`phydm_rfe_8822b(dm, channel)` (`api:648-702`) phân nhánh theo rfe_type:

| rfe_type | Hàm |
|---|---|
| 4, 11 | `phydm_rfe_4_11` |
| 1, 2, 6, 7, 9 | `phydm_rfe_efem` (eFEM — FEM ngoài) |
| 0, 3, 5, 8, 10, 12, 13, 14, 16, 17 | `phydm_rfe_ifem` (iFEM — FEM tích hợp) |
| 15 | `phydm_8822b_type15_rfe` ("Microsoft, 5G low/high band at S0") |
| 18 | `phydm_8822b_type18_rfe` ("Roku") |

Với kênh 15–35 hoặc kênh 0, hàm trả `false`.

`phydm_rfe_ifem` — mặc định khi rfe 0 (`api:469-511`):

| Thanh ghi | 2.4 GHz | 5 GHz |
|---|---|---|
| `0xCB0[23:0]` = `0xEB0[23:0]` | `0x745774` | `0x477547` |
| `0xCB4[15:8]` = `0xEB4[15:8]` | `0x57` | `0x75` |
| `0xCBC[5:0]`, `0xCBC[11:10]` (+ `0xEBC`) | 0 (không đảo) | 0 |
| `0xCA0[15:0]` = `0xEA0[15:0]` | 2T/2R: `0xA501`; TX = RX: `0xA500`; chéo: `0xA005` | `0xA5A5` |

[INFERENCE] Hai giá trị 2.4G/5G là cùng các nibble được hoán vị (`7-4-5-7-7-4` ↔ `4-7-7-5-4-7`). Nhiều khả năng mỗi nibble chọn tín hiệu (PAPE/TRSW/LNAON…) cho một chân RFE_CTRL. Ví dụ, `phydm_rfe_smtant` có comment `RFE_CTRL_n` cho từng nibble của `0xCB0`/`0xCB4`. Tuy nhiên, ánh xạ mã nibble → tín hiệu **không có trong source**.

Thêm hai điểm:

- `phydm_rfe_8822b_lps` (`:705`) chuyển RFE sang điều khiển bằng SW khi vào LPS.
- `_iqk_rfe_setting_8822b` / `_iqk_backup/restore` lưu và khôi phục `0xCB0–0xCBC`/`0xEB0–0xEBC` quanh IQK. IQK mặc định chạy bằng FW offload (01 §6), nên các hàm `_iqk_*` của driver chỉ là đường dự phòng (15).

## 8. CCX — đo môi trường kênh (NHM / CLM)

`phydm_ccx.c`, nhánh `ODM_IC_11AC_SERIES`:

| Thanh ghi | Tên | Dùng |
|---|---|---|
| `0x990[15:0]` | `ODM_REG_CCX_PERIOD_11AC` | chu kỳ đo CLM (`phydm_clm_setting`, `:1654`) |
| `0x994` | `ODM_REG_CLM_11AC` | `[1]`: trigger NHM, ghi 0 rồi 1 (`phydm_nhm_trigger`, `:446-470`); `[31:16]`: 2 ngưỡng NHM. Hàm gán `reg1 = R_0x994` vào biến rồi truy cập qua biến, nên danh mục đếm live = 0 [TOOL] |
| `0x998`, `0x99C`, `0x9A0` | NHM threshold th0–3, th4–7 (`ODM_REG_NHM_TH7_TO_TH4_11AC` ở `0x99C`), th8 | `phydm_nhm_set_th_reg` (`:689-735`, qua biến `reg2/3/4`) |
| `0xFA4[15:0]` | `ODM_REG_CLM_RESULT_11AC` | kết quả CLM (`phydm_clm_get_result`) |
| `0xFA8`, `0xFAC`, `0xFB0` | `ODM_REG_NHM_CNT_11AC`, `NHM_CNT7_TO_CNT4`, — | 12 bin histogram NHM, mỗi bin 1 byte, `odm_read_4byte` → `nhm_result[0..11]` (`phydm_nhm_get_result`, `:609-620`) |
| `0xFB4` | — | `[15:0]` NHM duration; `[16]` cờ ready (`phydm_nhm_check_rdy`) |

[INFERENCE theo tên] NHM là Noise Histogram Measurement, CLM là Channel Load Measurement. Kết quả được dùng cho ACS/adaptivity. Danh mục đếm thấp vì phần lớn truy cập đi qua biến `reg1/reg2/reg3` (giống trường hợp St R ở MAC). Phụ lục A07 liệt kê đủ.

## 9. Các thanh ghi BB khác hay gặp

| Địa chỉ | Hàm sử dụng | Ghi chú |
|---|---|---|
| `0xC00` / `0xE00` | `_phy_lc_calibrate_8822b`, `phydm_stop_3_wire`, `phydm_dynamic_spur_det_eliminate`, IQK | "3-wire" (LSSI) on/off — 14/15 |
| `0xC90` / `0xE90` | `config_phydm_write_rf_reg_8822b` | LSSI write port: `(addr<<20) \| data20` (03 §5) |
| `0x838`, `0x82C`, `0x830`, `0x83C` | `phydm_ccapar_by_rfe_8822b`, `phydm_dsde_nbi` | tham số CCA theo RFE; NBI (narrow-band interference) |
| `0x8C8` | `phydm_dig_init`, `phydm_set_big_jump_step` | big-jump của AGC (liên quan §2.4) |
| `0xC1C` / `0xE1C` | `get_swing_index`, `phydm_rxagc_switch_8822b` | `[31:21]` = mask `0xFFE00000`, BB swing/TX scale (`halrf_powertracking_ce.c:664`; power tracking — 15); `[11:8]` chọn AGC (rfe 15/16) |

## 10. Dấu hiệu lỗi

| Hiện tượng | Kiểm tra |
|---|---|
| Không thấy AP 5 GHz, 2.4 GHz vẫn bình thường | rfe_type sai (log `EEPROM rfe_type`); `0xCB0`/`0xCB4` không đổi khi chuyển băng; `0x958` |
| Vào kênh 5G bị log `"channel not supported"` / `return false` | kênh nằm trong 15–35, hoặc `0xBE` tra ra `0xFF` |
| Throughput thấp, nhiều retry | `0xF48`/`0xA5C` tăng nhanh (FA cao); IGI `0xC50[6:0]` |
| Chỉ một anten hoạt động | `0x808[7:0]`, `0x80C[7:0]`, `0xC08`/`0xE08` = `0x1111` |
| Log `"Empty EFUSE with unknown REF type!!"` và không có wlan interface | eFuse `0xCA = 0xFF` → probe thất bại; dùng `insmod … rtw_RFE_type=<n>` (12, 07 §8) |
