# 05.16 — MP (Mass Production) và Debug: đường truy cập thanh ghi tuỳ ý, dump, thanh ghi chỉ dùng ở MP/debug

> Nhãn như 05.01. Trạng thái trong danh mục: **M** = chỉ được truy cập từ đường MP; **G** = chỉ từ đường debug/dump (04_catalog_README).
> Build: `CONFIG_MP_INCLUDED` bật nhưng `mp_mode = 0` mặc định. `CONFIG_PROC_DEBUG` và `CONFIG_RTW_DEBUG` bật (01 §4), nên các đường dưới đây **có trong module** và người dùng có quyền ghi `/proc` dùng được [SOURCE: 01].

## 1. Các "cửa" cho phép người dùng chạm thanh ghi

| Cửa | Handler (nguồn) | Cú pháp / giới hạn | Đi xuống |
|---|---|---|---|
| `/proc/net/rtl8812bu/<if>/write_reg` (`RTW_PROC_NAME = DRV_NAME = "rtl8812bu"`, `rtw_proc.c:74`, `autoconf.h:65`) | `proc_set_write_reg` (`core/rtw_debug.c`) | `echo "<addr> <val> <len>"` (hex), `len` ∈ {1, 2, 4}; **không kiểm tra địa chỉ** | `rtw_write8/16/32` (03) |
| `…/read_reg` | `proc_set_read_reg` (đặt địa chỉ) + `proc_get_read_reg` (đọc) | `echo "<addr> <len>"`, rồi `cat`; địa chỉ lưu trong biến `static` **dùng chung cho mọi interface** | `rtw_read8/16/32` |
| `…/mac_reg_dump` | `mac_reg_dump` (`core/rtw_debug.c:442-474`) | MAC `0x0000–0x07FC` + `0x1000–0x17FC` (nhánh `CONFIG_RTL8822B`), bước 4 | 1024 × `R32` |
| `…/bb_reg_dump`, `bb_reg_dump_ex` | `bb_reg_dump(_ex)` (`:480-558`) | `0x0800–0x0FFC` + `0x1800–0x1FFC` | 1024 × `R32` |
| `…/rf_reg_dump` | `rf_reg_dump` (`:560-578`) | mỗi path RF `0x00–0xFF`, `rtw_hal_read_rfreg(path, i, 0xFFFFFFFF)` | 2 × 256 lần đọc RF = 512 `R32` qua cửa sổ `0x2800/0x2C00` (14 §1) |
| `…/efuse_map` | `proc_get_efuse_map` | bản đồ logic (12) | HALMAC efuse |
| phydm `cmd` (`/proc/net/rtl8812bu/<if>/odm/cmd`, `rtw_proc.c:5117`) | `proc_set_phydm_cmd` (`os_dep/linux/rtw_proc.c:4982-5025`) → `phydm_cmd` | lệnh `dumpreg` (BB `0x800–0xFFC`, `0x1800–0x18FC`, `phydm_debug.c:3633-3675`), `get_txagc`/`set_txagc`, `trxpath`, `ch_bw`, `dig`, `nhm`/`clm`/`fahm`, `rf` (halrf) … | `odm_get/set_bb_reg`, API phydm (13/14/15) |
| ioctl MP `write_reg` | `rtw_mp_write_reg` (`os_dep/linux/ioctl_mp.c:77-168`) | `b/w/d,<addr>,<data>`; **`addr ≤ 0x3FFF`**, data kiểm theo độ rộng | `rtw_write8/16/32` |
| ioctl MP `read_reg` | `rtw_mp_read_reg` (`:169-298`) | như trên | `rtw_read*` |
| ioctl MP `write_rf` / `read_rf` | `rtw_mp_write_rf` (`:299-344`) / `rtw_mp_read_rf` (`:345-…`) | `path,addr,data`; `path < RFPATH_NUM`, `addr ≤ 0xFF`, `data ≤ 0xFFFFF` | `write_rfreg` → LSSI (14) |

### 1.1 Hệ quả ở tầng USB [SOURCE + tính tay]

- **Không có kiểm tra phạm vi ở proc.** `usbctrl_vendorreq` dùng `wValue = addr & 0xFFFF` (03 §3). Vì vậy:
  - ghi `write_reg 10000 …` thực chất ghi vào **0x0000**;
  - địa chỉ `0xFE00–0xFFFF` chạm các thanh ghi USB-local (09).

  ioctl MP thì chặn `addr > 0x3FFF`.
- **Dump MAC sinh thêm ghi.** 128 trong 1024 lần đọc (64 ở `0x00–0xFC`, 64 ở `0x1000–0x10FC`) của `mac_reg_dump` rơi vào `0x00–0xFF` hoặc `0x1000–0x10FF`. Mỗi lần đó kèm **1 lần ghi `0x4E0`** (03 §3.4; 09 mục 1). Tổng cộng: 1024 `R32` + 128 `W8` = **1152 control transfer**.
  - Dải `0x000–0x7FC` (< 0x1000) được thử lại tới 10 lần, còn `0x1000–0x17FC` chỉ thử 1 lần (09 mục 12).
  - Khi đọc lỗi, giá trị in ra là `0x00000000` (09 mục 13), trông giống thanh ghi bằng 0.
- **Dump đọc qua mọi cổng dữ liệu.** Ví dụ `0x678` CAMREAD, `0x69C` WKFMCAM_RWD, `0x1D0` mailbox, cùng các bộ đếm (`0x664` RXERR_RPT…).
  - [INFERENCE] Nếu phần cứng có thanh ghi "đọc-là-xoá" hoặc FIFO pop-on-read, dump sẽ gây tác dụng phụ.
  - Source **không** khai báo thanh ghi nào như vậy, nên chưa xác định. [KIỂM TRA TRÊN BOARD] So sánh hai lần dump liên tiếp khi không có lưu lượng.
- **Không có khoá bao trùm.** Mỗi `rtw_read32` được `halmac_reg_mutex` bảo vệ riêng (09 mục 2), nhưng cả vòng dump thì không. Watchdog/phydm có thể ghi xen kẽ, nên dump **không phải ảnh chụp nguyên tử**.

### 1.2 Lỗi nhỏ ở handler proc [SOURCE + INFERENCE]

`proc_set_write_reg` (`tmp[32]`) và `proc_set_read_reg` (`tmp[16]`) chạy `copy_from_user(tmp, buffer, count)` rồi `sscanf(tmp, …)` mà **không đặt `'\0'`** sau `count` byte.

- Với `echo` thông thường, chuỗi kết thúc bằng `'\n'`, nên `sscanf` dừng đúng chỗ.
- Với `echo -n "100 1 1"`, byte rác trên stack ngay sau chuỗi có thể là chữ số hex và bị nối vào tham số cuối (`len`/`val`).
- Khi `count == sizeof(tmp)`, `sscanf` có thể đọc quá cuối buffer.

Mức ảnh hưởng: chỉ với người có quyền ghi proc. Đã ghi ở 09, mục 21.

## 2. Thanh ghi trạng thái M (chỉ MP) — sau cải tiến guard phiên 3

| Không gian | Địa chỉ | Tên | Hàm | Ghi chú |
|---|---|---|---|---|
| MAC | `0x00A0` | `REG_SYSON_FSM_MON_8822B` | `rtw_mp_efuse_get/set` (`ioctl_mp.c:2845-2847, 2906`) | `W8(0xA3, 0x05)`, rồi `R8(0xA0)` → "btwmap … BT Status"; dùng khi đọc/ghi bản đồ eFuse BT |
| MAC | `0x00A3` | (byte 3 của `0xA0`, không có tên riêng) | như trên | |
| MAC | `0x0460` | (literal) | `Rtw_MPSetMacTxEDCA` (`core/rtw_mp.c:1891-1899`) | `[15:0] = 0` — comment: "fast EDCA queue packet interval & time out value". Cùng hàm ghi `0x508 = 0x00A422` ("Disable EDCA BE Txop for MP pkt tx") và `0x458 = 0` |
| BB | `0x0F50` | `R_0xf50` | `hal_mpt_query_phytxok`, `mpt_ProSetPMacTx` | bộ đếm TX OK / PMAC |
| BB | `0x1800`, `0x1A00` | `R_0x1800`, `R_0x1a00` | `rtw_mp_psd_init/close` | |
| BB | `0x181C`, `0x1A1C` | `rC/rD_TxScale_Jaguar2` | `mpt_SetSingleTone_8814A` (`hal/hal_mp.c:913-…`, biên dịch cho cả 8822B theo `#if … || defined(CONFIG_RTL8822B)`) | được gọi **không điều kiện IC** từ đường PMAC TX của MP (`:2362`, `:2527`): lưu `0xC1C/0xE1C/0x181C/0x1A1C`, rồi ghi `[31:21] = 0` cho **cả 4** — `0x181C`/`0x1A1C` là TX scale path C/D của 8814A (comment `/*/ 0x181C[31:21]*/`); 8822B chỉ có 2 path → ý nghĩa của hai địa chỉ này trên 8822B **chưa xác định** (09 mục 22) |

Sau cải tiến guard (PROGRESS, phiên 3), BB `0xD00` (`rOFDM1_LSTF`, nhánh 11N) đã chuyển M→X.

Ngoài các địa chỉ chỉ-MP ở trên, MP còn dùng nhiều thanh ghi trạng thái A, đặc biệt là đường TX liên tục và single-tone:

- BB `0x914[18:16]` (`rSingleTone_ContTx_Jaguar`): `OFDM_ContinuousTx`, `OFDM_SingleCarrier`, `OFDM_SingleTone`, `OFDM_ALL_OFF`.
- RF `0x53`/`0x63` (`hal_mpt_SetSingleToneTx`, trạng thái T).
- TXAGC theo `mpt_SetTxPower` (15 §2.2, rò buffer khi 1T).

Phụ lục A07/A08 có đầy đủ các hàm.

## 3. Thanh ghi trạng thái G (chỉ debug)

| Không gian | Địa chỉ | Hàm | Nội dung |
|---|---|---|---|
| MAC | `0x0400–0x0414`, `0x0468–0x0474` | `dump_mac_qinfo` | `REG_Q0..Q7_INFO`, `REG_MGQ_INFO`, `REG_HIQ_INFO` (05.04) |
| MAC | `0x1400–0x1414` | `dump_mac_qinfo` | `REG_Q0_Q1_INFO … REG_CMDQ_BCNQ_INFO` |
| MAC | `0x0438`, `0x0484`, `0x0488`, `0x06D9` | `rtw_dbg_port` | `REG_RARFRC`, `REG_MACID_SLEEP3/1`, … |
| MAC | `0x0038` | `phydm_la_set_mac_iq_dump` | `REG_PWR_OPTION_CTRL` (logic analyzer) |
| BB | `0x0D94`, `0x0DD4`, `0x0F20`, `0x0F2C–0x0F34`, `0x0F80–0x0F90` | `phydm_bb_hw_dbg_info_ac` | thống kê phần cứng BB (11AC) |
| BB | `0x1BE8` | `halrf_iqk_dbg` | kết quả IQK |

**Debug port BB** (11AC, theo `phydm_debug.c:169-211`):

```
phydm_set_bb_dbg_port: W32(0x8FC, port)   (chỉ khi priority mới > cũ)
phydm_get_bb_dbg_port_val: R32(0xFA0)
```

Các nhánh `0x908`/`0xDF4` (11N) và `0x1C3C`/`0x2DBC` (JGR3) là X với 8822B. Cải tiến guard đã sửa trạng thái `0xDF4` và `0x2DBC` từ G sang X.

## 4. Lưu ý khi dùng để kiểm tra trên board

| Mục đích | Cách an toàn hơn |
|---|---|
| Đọc một thanh ghi MAC | `echo "<addr> 4" > read_reg; cat read_reg`. Nếu ra `0x0`, kiểm tra log USB (`usbctrl_vendorreq` lỗi), vì 0 có thể là lỗi đọc |
| Đọc RF | dùng `rf_reg_dump` hoặc ioctl `read_rf`; **không** dùng `read_reg` với `0x2800 + addr*4` cho path B nếu không chắc về địa chỉ (14 §1) |
| So sánh trước/sau một thao tác | chạy dump hai lần khi không có lưu lượng để biết thanh ghi nào tự biến động |
| Ghi thử | dùng ioctl MP (có chặn `> 0x3FFF`) thay vì proc |
