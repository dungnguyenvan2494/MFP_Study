# 05.15 — Calibration và TX power: TXAGC 0x1D00/0x1D80, power tracking (0xC94/0xC1C), IQK (FW), kfree/PA bias/thermal trim (eFuse), crystal cap, LCK

> Nhãn như 05.01. Liên quan: 13 (BB), 14 (RF), 12 (eFuse). Phụ lục: [A07](appendix/A07_bb_800_fff.md), [A08](appendix/A08_bb_1800_1fff.md).
> Simulator dùng trong file này (không truy cập phần cứng): `tools/txagc_sim.py`, `tools/pwrtrk_sim.py`, `tools/rftab_sim.py`.

## 1. Bản đồ cơ chế → thanh ghi

| Cơ chế | Thanh ghi | Khi nào chạy | Ai thực hiện |
|---|---|---|---|
| Công suất TX theo rate | BB `0x1D00–0x1D3C` (A), `0x1D80–0x1DBC` (B) | mỗi lần đổi kênh/BW (`rtl8822b_switch_chnl_and_set_bw` → `rtw_hal_set_tx_power_level`, `hal/rtl8822b/rtl8822b_phy.c:1126`) | driver |
| Power tracking theo nhiệt | BB `0xC94/0xE94[29:25]`, `0xC1C/0xE1C[31:21]`; RF `0x42` | watchdog 2 s (đọc thermal 4 s/lần — 14 §5) | driver |
| IQK | trang BB `0x1B00` (chương trình nạp qua PHY_REG — 13 §2.2); RF `0x08` (bắt tay) | khi `bNeedIQK` (`HW_VAR_DO_IQK`, `hal_com.c:14821-14826`) lúc đổi kênh; khi `delta_iqk ≥ 8` (`halphyrf_ce.c:812-826`) | **firmware** (`rtw_iqk_fw_offload = 1`) |
| kfree (bù gain TX) | RF `0x55`, `0x65`, `0xDE` | mỗi lần đổi kênh (`phydm_config_kfree`, `rtl8822b_phy.c:1122`) nếu eFuse có dữ liệu | driver |
| PA bias trim | RF LUT `0xEF[10]` + `0x33` + `0x3F` | một lần, khi đọc eFuse (`rtw_phydm_read_efuse`, `rtl8822b_ops.c:737` → `phydm_get_power_trim_offset`) | driver |
| Thermal trim | — (chỉ là biến `power_trim_info->thermal`) | một lần | driver |
| Crystal cap | MAC `0x24[30:25]` `XTAL_CAP_XI`, `0x28[6:1]` `XTAL_CAP_XO` | sau `_init_bb_reg` (`init_bb_reg`, `rtl8822b_phy.c:205-216`); CFO tracking | driver |
| LCK | RF `0x18[15]`, `0xC4`, `0x00`; BB `0xC00/0xE00` | **chỉ MP/debug** với 8822B (14 §4); chế độ thường dùng RTK của phần cứng | — |

## 2. TXAGC — công suất theo rate

### 2.1 Cách ghi

- `rtl8822b_set_tx_power_index(adapter, idx, path, MGN_rate)` (`rtl8822b_phy.c:770-812`):
  1. `rate = MRateToHwRate(rate)`, rồi `shift = rate % 4`.
  2. `hal->txagc_set_buf |= idx << (shift*8)`.
  3. **Chỉ khi** `shift == 3`, gọi `config_phydm_write_txagc_8822b(buf, path, rate & 0xFC)` rồi xoá buffer.
- `config_phydm_write_txagc_8822b` (`hal/phydm/rtl8822b/phydm_hal_api8822b.c:1078-1103`): `odm_set_bb_reg({0x1D00, 0x1D80}[path] + (rate & 0xFC), MASKDWORD, buf)`. Mỗi lần là 1 `W32`, 1 transfer, và bị từ chối nếu `hw_rate > 0x53` hoặc `path > B`. Comment gốc: "driver need to construct a 4-byte power index".
- Mỗi byte là power index (0…`txgi_max`) của một HW rate. Mã HW rate xem ở `include/hal_com.h:76-142`: `0x00–0x03` CCK, `0x04–0x0B` OFDM, `0x0C–0x1B` MCS0–15, `0x2C–0x35` VHT1SS, `0x36–0x3F` VHT2SS.
- Đọc lại `config_phydm_read_txagc_8822b` qua BB `0x1998` (`[7:0]` = rate | path<<7; `[16]` bật report): chỉ có khi `PHYDM_FW_API_FUNC_ENABLE_8822B == 1`.

Nội dung byte được tính bởi `rtl8822b_get_tx_power_index` (`:827-870`):

```
power_idx = pg(eFuse, theo kênh/BW/ntx) + min(by_rate, limit) + tpt_offset(=0, dòng "PHY_GetTxPowerTrackingOffset" bị comment) + extra_bias(=0)
```

Kết quả được kẹp trong `[0, txgi_max]`. Bảng by-rate (PHY_REG_PG) và giới hạn (TXPWR_LMT) **không ghi trực tiếp vào thanh ghi**: chúng chỉ là dữ liệu phần mềm để tính byte trên. Mặc định `rtw_load_phy_file` có BIT2 và BIT6, nên hai bảng này có thể bị file trong `/lib/firmware` ghi đè (09 mục 10).

### 2.2 Mô phỏng — dword nào được ghi (`txagc_sim.py`) [TOOL]

`set_tx_power_level_by_path` (`rtl8822b_phy.c:711-728`) gọi các section theo thứ tự: CCK (chỉ 2.4G) → OFDM → (nếu không đang survey) HT1SS → HT2SS → VHT1SS → VHT2SS. Theo thứ tự đó:

| Trường hợp | Mỗi path | Tổng (2 path) |
|---|---|---|
| 2.4G, đã kết nối | 12 × `W32`: `+0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x2C, 0x30, 0x34, 0x38, 0x3C` | 24 transfer |
| 5G, đã kết nối | 11 (không có `+0x00`) | 22 |
| 5G, đang survey | 2 (`+0x04`, `+0x08`) | 4 |

- **Không bao giờ ghi** `+0x1C … +0x28` (MCS16–31, chip 2T) và `+0x40 … +0x50` (VHT3SS/4SS).
- Dword `+0x34` gồm VHT1SS MCS8/9 (`0x34`, `0x35`) và VHT2SS MCS0/1 (`0x36`, `0x37`). Nó chỉ được ghi khi đang chạy section VHT2SS, nhờ VHT1SS (10 rate) kết thúc giữa nhóm 4 rate và VHT2SS nối ngay sau.

**Rủi ro rò buffer [SOURCE + TOOL]:** `hal->txagc_set_buf` là **một biến dùng chung cho mọi path**, và chỉ được xoá khi ghi. Nếu một chuỗi gọi dừng ở rate có `rate % 4 ≠ 3`, các byte còn lại sẽ bị **OR** vào dword được ghi kế tiếp (có thể của path khác). Đường thường luôn kết thúc ở `0x3F`, hoặc `0x0B` khi survey, nên an toàn. Đường MP thì không:

- `mpt_SetTxPower` (`hal/hal_mp.c:549-566`) với `rf_type` 1T dừng ở `MGN_VHT1SS_MCS9` (HW `0x35`).
- Mô phỏng `txagc_sim.py 5g 0 2 --order VHT1`:
  - path A ghi `0x1D2C`, `0x1D30`, và để lại `0x3534` trong buffer;
  - path B ghi `0x1DAC = 0x2F2E3D3C`, vì `0x34|0x2C`, `0x35|0x2D` bị trộn;
  - **MCS8/9 của cả hai path không được ghi**.

Đã ghi ở 09, mục 20 (chỉ MP).

## 3. Power tracking theo nhiệt (MIX_MODE)

### 3.1 Chuỗi gọi

```
watchdog → odm_txpowertracking_check_ce (kích RF 0x42[17:16]=3)          (14 §5)
         → odm_txpowertracking_callback_thermal_meter (halphyrf_ce.c:431-…)
             thermal = RF-A 0x42[15:10] + thermal_offset
             delta   = |thermal − eeprom_thermal|, kẹp ≤ 29
             odm_get_tracking_table: nóng → absolute = +up[delta]; lạnh → absolute = −down[delta]
             → odm_pwrtrk_method (8822B thuộc nhóm MIX_MODE) → odm_tx_pwr_track_set_pwr8822b(MIX_MODE, path)
                 tx_pwr_idx = odm_get_tx_power_index(path, tx_rate, bw, ch); offset = 63 − min(tx_pwr_idx, 63)
                 → get_mix_mode_tx_agc_bb_swing_offset_8822b → odm_pwrtrack_method_set_pwr8822b
             delta_lck ≥ 8 → (bỏ qua với 8822B)       delta_iqk ≥ 8 → do_iqk_8822b → IQK (FW)
```

- Bản gửi power tracking cho FW (`hal_mac_send_power_tracking_info`) nằm trong `#if 0` (`halrf_8822b.c:234-315`), nên driver tự ghi thanh ghi.
- Bảng delta được chọn theo rfe (`phydm_hwconfig.c:662-700`), ví dụ rfe 0 → `_txpowertrack_type0`, rfe ngoài danh sách → `_txpowertrack`. Chúng nằm trong `halhwimg8822b_rf.c`.
- `LOAD_RF_TXPWR_TRACK_PARA_FILE` (BIT5) không bật, nên luôn dùng mảng trong source.

### 3.2 Thanh ghi đích

| Thanh ghi | Field | Ý nghĩa (theo tên biến/bảng) | Mặc định sau PHY_REG |
|---|---|---|---|
| `0xC94` / `0xE94` | `[29:25]` | `tx_agc_index` — số bậc power index cộng thêm (0…15) | `0x01000100` → `[29:25] = 0` |
| `0xC1C` / `0xE1C` | `[31:21]` (`REG_A/B_TX_SCALE_JAGUAR` = `rA/rB_TxScale_Jaguar`) | BB swing, tra `tx_scaling_table_jaguar[37]` (`halrf_powertracking_ce.c:570-608`): idx 0 = `0x081` (−12 dB) … idx 24 = `0x200` (0 dB) … idx 36 = `0x3FE` (+6 dB), bước 0.5 dB | `0x40040053` → `0x200` → `default_ofdm_index = 24` (tìm bằng `get_swing_index`) |

Thuật toán của `get_mix_mode_tx_agc_bb_swing_offset_8822b` (`halrf_8822b.c:104-167`), với `off = min(63 − tx_pwr_idx, 15)`:

| `absolute` | `tx_agc_index` | swing idx |
|---|---|---|
| `0 ≤ absolute ≤ off` | `absolute` | default (24) |
| `absolute > off` | `off` | `24 + (absolute − off)`, tối đa `24 + 10 = 34` |
| `absolute < 0` | 0 | `24 + absolute` (tối thiểu 0) |

Ý tưởng: nhiệt tăng thì công suất giảm, nên ưu tiên tăng TXAGC (thô). Khi hết headroom 63, phần còn lại được bù bằng BB swing. Nhiệt giảm thì chỉ giảm BB swing.

### 3.3 Mô phỏng (`pwrtrk_sim.py`, bảng thật của rfe 0) [TOOL]

| Kịch bản | delta | `absolute` | `0xC94[29:25]` | `0xC1C[31:21]` |
|---|---|---|---|---|
| ch6, thermal 30 / PG 25, idx 40 | 5 | +4 | 4 | `0x200` (0 dB) |
| ch6, thermal 45 / PG 25, idx 50 (headroom 13) | 20 | +16 | 13 | `0x261` (+1.5 dB) |
| ch36 path B, thermal 15 / PG 30 | 15 | −8 | 0 | `0x143` (−4.0 dB) |
| ch40, thermal = PG | 0 | 0 | 0 | `0x200` |

Mỗi lần áp dụng tốn 2 RMW × 2 path = **8 transfer**. Giá trị thermal và PG thật của board: [KIỂM TRA TRÊN BOARD]. PG đến từ eFuse (12); thermal đọc từ RF `0x42`.

## 4. IQK

- **Chế độ thường: FW làm** (`phy_iq_calibrate_8822b`, `halrf/rtl8822b/halrf_iqk_8822b.c:1849-1872`), theo trình tự:
  1. `_phy_iq_calibrate_by_fw_8822b` → `odm_iq_calibrate_by_fw` (H2C, 08);
  2. delay 100 × 10 µs;
  3. `phydm_get_read_counter_8822b`: poll RF-A `0x08` tới khi `== 0xABCDE` (≤ 20000 × 10 µs), rồi ghi 0 (14 §3);
  4. `halrf_iqk_check_if_reload`;
  5. `_iqk_fail_count_8822b` (đọc RF-A `0xF0` do che địa chỉ — 09 mục 17).
- Kích hoạt:
  - lúc đổi kênh, nếu `bNeedIQK` (`HW_VAR_DO_IQK`), hoặc ở MP;
  - từ power tracking khi `delta_iqk ≥ IQK_THRESHOLD = 8` (`halrf.h:428`; comment "Delta temperature is equal or larger than 20 Celsius"), không khi đang scan hay `rfk_forbidden`.
- Đường driver `_phy_iq_calibrate_8822b` (≈ 48 truy cập trang `0x1B00`, `0xC00–0xCBC` backup/restore, RF `0x56/0x65/0x8F/0xDF/0xEF`) chỉ chạy khi `rtw_iqk_fw_offload = 0` (09 mục 9).
- Trang `0x1B00`: chương trình 1007 entry được nạp sẵn qua PHY_REG (13 §2.2, 09 mục 16). Ý nghĩa từng thanh ghi `0x1B00–0x1BFF`: không có tên trong header 8822B ngoài các literal `R_0x1bxx` [TOOL: danh mục BB].

## 5. Dữ liệu trim từ eFuse **vật lý**

`phydm_get_power_trim_offset` ← `rtw_phydm_read_efuse` ← `rtl8822b_ops.c:737`.

Các hàm đọc bằng `odm_efuse_one_byte_read` → `efuse_OneByteRead` → `rtw_halmac_read_physical_efuse` (`core/efuse/rtw_efuse.c:1975-1990`). Đây là địa chỉ **vật lý**, không phải bản đồ logic (12). Địa chỉ lấy từ `halrf_kfree.h:109-134`:

| Byte vật lý | Macro | Giải mã | Dùng ở |
|---|---|---|---|
| `0x3EF` | `PPG_THERMAL_OFFSET_22B` | `v & 0x1F`; `BIT0 = 0` → âm; độ lớn `v >> 1` | `power_trim_info->thermal` (`halrf_kfree.c:287-312`) |
| `0x3EE` | `PPG_2G_TXAB_22B` | nibble thấp = path A, cao = path B | kfree 2.4G (`bb_gain[0][*]`) |
| `0x3EC`/`0x3EB`, `0x3E8`/`0x3E7`, `0x3E4`/`0x3E3`, `0x3E0`/`0x3DF`, `0x3DC`/`0x3DB` | `PPG_5GL1/5GL2/5GM1/5GM2/5GH1_TXA/TXB_22B` | nguyên byte | kfree 5G, 5 dải × 2 path |
| `0x3D5`, `0x3D6` | `PPG_PABIAS_2GA/2GB_22B` | `v & 0xF`; `BIT0 = 0` → âm; độ lớn `v >> 1` (0…7) | `phydm_set_pa_bias_to_rf_8822b` (14 §6) |

- `0xFF` nghĩa là chưa lập trình: bỏ qua, không bật cờ `KFREE_FLAG_*` / `PA_BIAS_FLAG_ON`.
- `phydm_config_kfree` (`halrf_kfree.c:2594-2620`) không làm gì nếu `reg_rf_kfree_enable == 2` hoặc chưa có `KFREE_FLAG_ON`. Ngược lại, `phydm_do_kfree` ghi RF `0xDE/0x65/0x55` (14 §6).
- Trường hợp PA bias, ví dụ tính tay: byte `0x05` → `& 0xF = 5`, `BIT0 = 1` → dương, `5 >> 1 = 2` → `+2`. Trong ví dụ ở 14 §6, giá trị này dẫn tới `rf3f = 0x423`.

## 6. Crystal cap (bù tần số thạch anh)

| Thanh ghi | Field (`tools/bitname.py`) | Ghi bởi |
|---|---|---|
| MAC `0x24` `REG_AFE_CTRL1_8822B` | `XTAL_CAP_XI_8822B[30:25]` | `phydm_set_crystal_cap_reg` (`hal/phydm/phydm_cfotracking.c:284-291`, comment "write 0x24[30:25] = 0x28[6:1] = crystal_cap") |
| MAC `0x28` `REG_AFE_CTRL2_8822B` | `XTAL_CAP_XO_8822B[6:1]` | như trên |

- Nguồn: eFuse logic `EEPROM_XTAL_8822B = 0xB9` (`include/hal_pg.h:496`), hoặc `EEPROM_Default_CrystalCap` nếu eFuse không hợp lệ (`rtl8822b_ops.c:373-377`, log `"EEPROM crystal_cap=0x%02x"`).
- Ghi lần đầu trong `init_bb_reg` ngay sau PHY_REG/AGC. Nếu ghi lỗi, in `"Init crystal_cap failed"` và `rtw_warn_on`.
- Sau đó CFO tracking (`phydm_cfotracking.c:340-380`) chỉnh ±1 quanh `def_x_cap`.
- Mỗi lần ghi gồm 2 RMW qua `odm_set_mac_reg` (09 mục 7), tức 4 transfer. Cả `0x24` và `0x28` đều thuộc dải `0x00–0xFF`, nên **mỗi** transfer còn kèm 1 lần ghi `0x4E0` (03 §3.4). Tổng cộng 8 transfer [tính tay].

## 7. Bảng tổng hợp chi phí USB (tính tay từ mô phỏng)

| Hoạt động | Transfer |
|---|---|
| PHY_REG + AGC (rfe 0) | 1492 + 521 = 2013 |
| RadioA + RadioB (rfe 0) | 755 |
| TX power, đổi kênh 2.4G (2 path) | 24 |
| Power tracking mỗi lần áp dụng | 8 (+ đọc thermal: 1 R32; kích: 2) |
| IQK FW: poll RF `0x08` | 1 R32 mỗi 10 µs cho tới khi FW xong |
| Crystal cap | 8 |

## 8. Dấu hiệu lỗi

| Hiện tượng | Kiểm tra |
|---|---|
| Công suất VHT MCS8/9 bất thường ở MP 1T | rò `txagc_set_buf` (§2.2, 09 mục 20) |
| Công suất giảm khi nóng | `0xC94[29:25]`, `0xC1C[31:21]`; thermal `0x42[15:10]`; PG thermal (eFuse) |
| Lệch tần số / CFO lớn | `0x24[30:25]`, `0x28[6:1]`; eFuse `0xB9` |
| EVM xấu sau khi đổi kênh | IQK FW: log `"[IQK]FWIQK fail!!!"`; `n_iqk_fail_cnt` (lưu ý mục 17) |
| Không có kfree | eFuse vật lý `0x3EE`/`0x3EC… = 0xFF` |
