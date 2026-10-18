#!/usr/bin/env python3
"""Generate parts 8-11 of the register study from work/*.json.

  08_direct_numeric.md   accesses whose address argument is a literal number
                         (or an address-named macro R_0x.../RF_0x...), live only, by file
  09_aliases_unused.md   alias families, dangling aliases, defined-but-unused per header
  10_index_by_address.md compact address -> name/status index per space
  11_index_by_name.md    name -> space/address/status index (every register macro)
Usage: py -I report_extra.py <work_dir> <docs_dir>
"""
import json, os, re, sys
from collections import defaultdict, Counter

W, DOCS = sys.argv[1:3]
L = lambda n: json.load(open(os.path.join(W, n)))
cat, access, regs = L('catalog.json'), L('access.json'), L('regs.json')
PAGE = {'MAC': '04a_catalog_mac.md', 'BB': '04b_catalog_bb.md', 'RF-direct': '04b_catalog_bb.md',
        'BB-pg': '04b_catalog_bb.md', 'other': '04b_catalog_bb.md', 'RF': '04c_catalog_rf.md'}
def page_of(sp): return PAGE.get(sp, '04d_catalog_other.md')
def hx(v): return f'0x{v:04X}' if v < 0x10000 else f'0x{v:08X}'
def esc(s): return s.replace('|', '\\|')
by_key = {(c['space'], c['addr']): c for c in cat}

# ------------------------------------------------------------------ 08
lit = re.compile(r'^\(?\s*(0[xX][0-9a-fA-F]+|\d+)[uUlL]*\s*\)?$')
named = re.compile(r'^\(?\s*(R|RF)_0x[0-9a-fA-F]+\s*\)?$')
num = [a for a in access if a['numeric'] and not a.get('via_array')]
live = [a for a in num if a['live']]
kind = Counter('literal' if lit.match(a['addr']) else 'R_0x/RF_0x' for a in num)
klive = Counter('literal' if lit.match(a['addr']) else 'R_0x/RF_0x' for a in live)
out = ['# 08 — Truy cập thanh ghi bằng địa chỉ số (Lượt 4)', '',
       'Nguồn: `work/access.json` (công cụ `tools/inventory.py`). Một truy cập được tính là "địa chỉ số" khi '
       'đối số địa chỉ là **literal** (`0x520`) hoặc macro **mang tên chính là địa chỉ** (`R_0x520`, `RF_0x18` '
       'trong `hal/phydm/phydm_regtable.h` — tên không mang ngữ nghĩa, tương đương số). [TOOL]', '',
       f'- Tổng: {len(num)} lời gọi ({kind["literal"]} literal, {kind["R_0x/RF_0x"]} R_0x/RF_0x).',
       f'- Trên đường sống (live) của RTL8822BU: **{len(live)}** ({klive["literal"]} literal, {klive["R_0x/RF_0x"]} R_0x/RF_0x).',
       f'- Còn lại {len(num) - len(live)} nằm trong nhánh chip khác / cấu hình tắt / hàm không có nơi gọi.',
       '',
       'Ý nghĩa: các địa chỉ này **không có tên chức năng** trong source → muốn biết chức năng phải dựa vào '
       'ngữ cảnh hàm, comment, hoặc tài liệu Realtek (không có trong repository). Cột "Tên tương ứng" cho biết '
       'địa chỉ đó có trùng với một macro có tên trong cùng không gian hay không (nếu có → có thể tra nghĩa).', '']
byfile = defaultdict(list)
for a in live:
    byfile[a['file']].append(a)
out.append('| File | Số lời gọi live |'); out.append('|---|---:|')
for f, xs in sorted(byfile.items(), key=lambda kv: -len(kv[1])):
    out.append(f'| `{f}` | {len(xs)} |')
out.append('')
for f, xs in sorted(byfile.items(), key=lambda kv: -len(kv[1])):
    out += [f'## `{f}`', '', '| Dòng | Hàm | Accessor | Không gian | Địa chỉ | Mask | Tên tương ứng |',
            '|---:|---|---|---|---|---|---|']
    for a in sorted(xs, key=lambda a: a['line']):
        sp = 'RF' if a['space'] == 'RF' else a.get('region', '?')
        sp2 = {'RF-direct(BB)': 'RF-direct'}.get(sp, sp)
        c = by_key.get((sp2, a['value'])) if a['value'] is not None else None
        names = [n for n in (c['names'] if c else []) if not re.match(r'(R|RF)_0x', n)]
        sp_s = sp + (f" {a.get('path','')}" if a['space'] == 'RF' else '')
        out.append(f"| {a['line']} | {a['func']} | {a['acc']} | {sp_s} | `{a['addr']}` | "
                   f"{esc(a.get('mask', ''))[:28]} | {esc(', '.join(names[:2]))} |")
    out.append('')
open(os.path.join(DOCS, '08_direct_numeric.md'), 'w', encoding='utf-8').write('\n'.join(out) + '\n')

# ------------------------------------------------------------------ 09
out = ['# 09 — Alias, định nghĩa không dùng, điểm chưa rõ', '',
       'Sinh tự động từ `work/regs.json` + `work/catalog.json` [TOOL]; các nhận xét có nhãn được viết tay.', '']
# 9.1 alias families
multi = [c for c in cat if len(c['names']) > 1]
suffix_pairs = sum(1 for n in regs if n.endswith('_8822B') and n[:-6] in regs)
out += ['## 9.1 Nhiều tên cho cùng một địa chỉ (cùng không gian)', '',
        f'- {len(multi)} địa chỉ có ≥2 tên. Dạng phổ biến nhất: cặp `REG_X_8822B` (`halmac_reg_8822b.h`) ↔ `REG_X` '
        f'(`halmac_reg2.h`) — **{suffix_pairs} cặp, 100% cùng giá trị** [TOOL]. Đây là alias thuần (cùng thanh ghi), '
        'không đếm như 2 thanh ghi.',
        '- Dạng thứ 2: tên theo địa chỉ `R_0xNNN` (phydm_regtable.h) trùng với tên chức năng.',
        '- Dạng thứ 3 (cần chú ý): tên từ thế hệ chip cũ (`rFPGA0_*`, `rOFDM0_*`, `ODM_REG_*_11N`) trỏ vào địa chỉ mà '
        'ở 8822B có thể mang nghĩa khác — xem mục 9.4.', '',
        '| Không gian | Địa chỉ | Các tên |', '|---|---|---|']
for c in sorted(multi, key=lambda c: (c['space'], c['addr'])):
    if len(c['names']) >= 3 or any(not n.endswith('_8822B') and n + '_8822B' not in c['names'] and not n.startswith('R_0x') for n in c['names']):
        out.append(f"| {c['space']} | `{c['addr_hex']}` | {esc(', '.join(c['names']))} |")
out.append('')
# 9.2 dangling aliases
dang = sorted(n for n, rs in regs.items() if rs[0]['value'] is None)
byf = defaultdict(list)
for n in dang:
    byf[regs[n][0]['file']].append(n)
out += ['## 9.2 Alias "treo" (trỏ tới macro không tồn tại trong build)', '',
        'Macro được `#define` active nhưng giá trị là tên của một macro **không được định nghĩa** trong nhánh build → '
        'nếu có code dùng chúng sẽ lỗi biên dịch; vì build thành công nên chúng chắc chắn không được dùng trong code active. '
        '[TOOL + INFERENCE]', '']
for f, ns in byf.items():
    out.append(f'- `{f}`: {len(ns)} tên, ví dụ ' + ', '.join(f'`{n}`→`{regs[n][0]["body"]}`' for n in ns[:6]))
out.append('')
# 9.3 defined but unused per header
out += ['## 9.3 Định nghĩa nhưng không dùng (status D/Z) theo header', '',
        '| Header | Tên định nghĩa (active) | Có nơi dùng trong code active | Chỉ trong code tắt (Z) | Không dùng (D) |',
        '|---|---:|---:|---:|---:|']
stat_of = {}
for c in cat:
    for n in c['names']:
        stat_of[n] = c['status']
hdr = defaultdict(Counter)
for n, rs in regs.items():
    f = rs[0]['file']
    hdr[f]['all'] += 1
    st = stat_of.get(n)
    if rs[0]['uses_total']:
        hdr[f]['used'] += 1
    elif st == 'Z':
        hdr[f]['Z'] += 1
    else:
        hdr[f]['D'] += 1
for f, c in sorted(hdr.items(), key=lambda kv: -kv[1]['all']):
    out.append(f"| `{f}` | {c['all']} | {c['used']} | {c['Z']} | {c['D']} |")
out += ['', '## 9.4 Điểm bất thường / chưa rõ (ghi tay, có bằng chứng)', '',
        '| # | Quan sát | Bằng chứng | Mức chắc chắn |', '|---|---|---|---|',
        '| 1 | Sau mỗi vendor request tới vùng 0x00–0xFF hoặc 0x1000–0x10FF, driver ghi thêm **1 byte vào 0x4E0** (`REG_NULL_PKT_STATUS_V1_8822B`) với nội dung buffer vừa dùng; không có comment giải thích | `os_dep/linux/usb_ops_linux.c:199-228`, `halmac_reg_8822b.h:421` | Source xác nhận hành vi; **mục đích chưa xác định** — cần tài liệu Realtek |',
        '| 2 | KM thêm mutex toàn cục `halmac_reg_mutex` quanh mọi `_rtw_read/write*`, trong khi `usbctrl_vendorreq` đã có `usb_vendor_req_mutex` (`CONFIG_USB_VENDOR_REQ_MUTEX` bật) | `core/rtw_io.c:93,117-224`; `usb_ops_linux.c:114-116`; `autoconf.h:295-297` | Source xác nhận; việc mutex thứ 2 có cần thiết hay không: [INFERENCE] chỉ khác biệt nếu có đường I/O không qua `usbctrl_vendorreq` hoặc cần bao trùm nhiều truy cập — nhưng read-modify-write (2 lời gọi) **vẫn không nguyên tử** |',
        '| 3 | `rtl8822bu_set_intf_ops()` tồn tại và giống hệt `usb_set_intf_ops()` nhưng **không được gọi**; probe dùng `usb_set_intf_ops` | `hal/rtl8822b/usb/rtl8822bu_io.c:61`, `hal/hal_hci/hal_usb.c:576`, `os_dep/linux/usb_intf.c:1412` | Source xác nhận |',
        '| 4 | `RF_T_METER_OLD/NEW` (địa chỉ RF 0x24/0x42) được đặt trong section `/* @MAC REG */` của `phydm_reg.h` | `hal/phydm/phydm_reg.h:74-78`; dùng với `odm_get_rf_reg` | Source xác nhận — dễ gây nhầm MAC 0x24 (`REG_AFE_CTRL1`) với RF 0x24 |',
        '| 5 | `ODM_CHANNEL` = 0x18 là **RF** 0x18 (channel/BW), trùng số với MAC `REG_SYS_SWR_CTRL3` 0x18 | `phydm_reg.h:143-152` (section RF REG) | Source xác nhận |',
        '| 6 | Vùng `0x2800–0x2FFF` của bus là cửa sổ đọc RF trực tiếp (không phải thanh ghi BB độc lập): `0x2800 + addr*4` (path A), `0x2C00 + addr*4` (path B) | `phydm_hal_api8822b.c:1000-1016` | Source xác nhận |',
        '| 7 | `odm_set_mac_reg()` thực chất gọi `phy_set_bb_reg()` (cùng hàm RMW 32-bit với BB) | `phydm_interface.c:234-252` | Source xác nhận — "MAC" và "BB" chỉ khác dải địa chỉ, không khác cơ chế |',
        '| 8 | `rtl8822b_write_bb_reg` với mask ≠ 0xFFFFFFFF luôn **đọc 32-bit rồi ghi 32-bit** → ghi một field 8-bit của thanh ghi MAC qua `phy_set_mac_reg` sẽ ghi lại cả 3 byte còn lại (giá trị vừa đọc). Nếu byte khác là bit W1C/tự xoá, có thể gây tác dụng phụ | `rtl8822b_phy.c:668-685` | Hành vi source xác nhận; tác dụng phụ là [INFERENCE] — phụ thuộc ngữ nghĩa phần cứng chưa có tài liệu |',
        '| 9 | IQK mặc định do firmware làm (`rtw_iqk_fw_offload=1`), nên ~48 truy cập trong luồng IQK của driver chỉ chạy khi đổi module param | `hal_ic_cfg.h:296-298`, `os_intfs.c:940-945`, `halrf_iqk_8822b.c:1856-1865` | Source xác nhận |',
        '| 10 | `MAC_REG.txt`, `PHY_REG_PG.txt`, `TXPWR_LMT.txt` trong `/lib/firmware/` (nếu có) **ghi đè** bảng trong source | `os_intfs.c:837`, `rtl8822b_phy.c:118-129` | [KIỂM TRA TRÊN BOARD] — có file hay không |',
        '| 11 | `phydm_check_bit_mask()` (RMW cho RF) trả `(orig & ~mask) \\| (data << shift)` **không `& mask`** — khác `rtl8822b_write_bb_reg` có `& mask`; giá trị rộng hơn field tràn sang bit kề | `hal/phydm/rtl8822b/phydm_hal_api8822b.c:243-256` vs `hal/rtl8822b/rtl8822b_phy.c:681` | Source xác nhận; có gây lỗi thực tế hay không phụ thuộc caller |',
        '| 12 | Vendor request chỉ retry khi `value < FW_START_ADDRESS (0x1000)`; mọi thanh ghi ≥0x1000 (MAC mở rộng, BB 0x1800+, cửa sổ đọc RF 0x2800+, USB-local) chỉ thử 1 lần | `os_dep/linux/usb_ops_linux.c:193-195` | Source xác nhận |',
        '| 14 | Thanh ghi 64-bit (ví dụ `REG_DARFRC_8822B` 0x430–0x437) được mô tả bằng `BIT_SHIFT_x (N & CPU_OPT_WIDTH)` với `CPU_OPT_WIDTH = 0x1F` (`halmac_bit_8822b.h:61`): shift thực = N mod 32 **trong dword thứ N/32** — ai dùng macro `BIT_x()` trên một biến 32-bit phải tự cộng 4 vào địa chỉ | `halmac_bit_8822b.h:8444,8456,8468` | Source xác nhận quy ước; driver không dùng các field này qua macro (bảng MAC_REG ghi theo byte) |',
        '| 13 | `usb_read8/16/32` bỏ qua mã lỗi của `usbctrl_vendorreq` và trả `data = 0` khi lỗi → caller không phân biệt được "đọc lỗi" với "giá trị 0" | `hal/hal_hci/hal_usb.c:362-451` | Source xác nhận |',
        '| 15 | `phydm_fa_cnt_statistics_ac`: `cnt_crc8_fail_vhtb = ret_value & 0xffff0000 >> 16` — `>>` ưu tiên hơn `&` nên thành `ret_value & 0xFFFF` (= `cnt_crc8_fail_vhta`); nửa cao của BB `0xFC4` không bao giờ được đọc. Bản JGR3 (`:490`) có ngoặc đúng | `hal/phydm/phydm_dig.c:1487` vs `:490` | Source xác nhận; chỉ ảnh hưởng thống kê/debug, không vào `cnt_all` |',
        '| 16 | 1007 entry cuối của `array_mp_8822b_phy_reg` (chỉ số 485–1491; 975 lần ghi `0x1B80`) trùng từng cặp với `array_mp_8822b_cal_init[]` vốn nằm trong `#if 0` → chương trình khởi tạo calibration được nạp qua PHY_REG; ý nghĩa cổng `0x1B80` không có tên/comment | `halhwimg8822b_bb.c` (bảng), `halrf/rtl8822b/halrf_rfk_init_8822b.c:69-1079` (`#if 0`); hàm `odm_read_and_config_mp_8822b_cal_init` (`:1081`) có thân rỗng (`#if 0` `:1083-1098`) | So sánh bằng script [TOOL]; ý nghĩa 0x1B80 chưa xác định |',
        '| 17 | `_iqk_fail_count_8822b` gọi `odm_get_rf_reg(dm, RF_PATH_A, RF_0x1bf0, BIT(16))` với `RF_0x1bf0 = 0x1BF0`; `config_phydm_read_rf_reg_8822b` che `reg_addr &= 0xff` → thực tế đọc **RF-A 0xF0[16]** (BB 0x2BC0), không phải BB 0x1BF0 mà vòng lặp ngay sau dùng; chạy sau **mọi** IQK (kể cả FW offload) và quyết định `is_reload` | `halrf_iqk_8822b.c:164,1866`; `phydm_regtable.h:876`; `phydm_hal_api8822b.c:1009` | Source xác nhận hành vi; ý định tác giả là [INFERENCE]; chỉ ảnh hưởng thống kê IQK |',
        '| 18 | Kiểm tra "RF is disable" (`config_phydm_read_rf_check_8822b(x) = x != 0xFFFFFFFF`) không thể đúng với path A/B: đọc RF luôn che `mask &= 0xFFFFF` → tối đa 0xFFFFF; cộng với `usb_read32` trả 0 khi lỗi (mục 13) → đọc RF hỏng thành 0 và RMW field ghi đè các bit còn lại bằng 0 | `phydm_hal_api8822b.h:80`; `phydm_hal_api8822b.c:1013,1047,1605,1884,2080` | Source + số học; tác động thực tế phụ thuộc tần suất lỗi USB |',
        '| 19 | LCK driver (`_phy_lc_calibrate_8822b`) không chạy ở chế độ thường: power tracking loại trừ `ODM_RTL8822B` ("Use RTLCK, close power tracking driver LCK"), chỉ còn đường MP/debug; HW dùng RTK (RF 0xC4 = 0x81402 từ bảng). Danh mục vẫn xếp A cho các truy cập của hàm này vì guard nằm quanh lời gọi qua con trỏ hàm | `halrf/halphyrf_ce.c:580-593`; `halrf_8822b.c:473-530` | Source xác nhận; giới hạn của tool reachability |',
        '| 20 | `rtl8822b_set_tx_power_index` gom 4 byte vào `hal->txagc_set_buf` (một biến chung cho mọi path) và chỉ ghi `0x1D00/0x1D80 + (rate & 0xFC)` khi `rate % 4 == 3`. Chuỗi gọi dừng giữa nhóm 4 rate → byte dư bị OR vào dword ghi kế tiếp. Đường thường an toàn (kết thúc ở 0x3F/0x0B); đường MP `mpt_SetTxPower` VHT với `rf_type` 1T dừng ở HW 0x35 → MCS8/9 không được ghi và path B nhận `0x2F2E3D3C` | `hal/rtl8822b/rtl8822b_phy.c:770-812`; `hal/hal_mp.c:549-566`; mô phỏng `tools/txagc_sim.py 5g 0 2 --order VHT1` | Source + TOOL; chỉ MP |',
        '| 21 | `proc_set_write_reg`/`proc_set_read_reg` `copy_from_user(tmp, buffer, count)` rồi `sscanf` mà không kết thúc chuỗi ký tự NUL; input không có ký tự xuống dòng (`echo -n`) hoặc dài đúng `sizeof(tmp)` có thể đọc byte rác trên stack; proc cũng không kiểm phạm vi địa chỉ (`wValue = addr & 0xFFFF` → 0x10000 ghi vào 0x0000) | `core/rtw_debug.c` (`proc_set_write_reg`, `proc_set_read_reg`); `os_dep/linux/usb_ops_linux.c` | Source; tác động chỉ với người có quyền ghi /proc |',
        '| 22 | MP single-tone (`mpt_SetSingleTone_8814A`, biên dịch cho 8822B và được gọi không điều kiện IC từ PMAC TX) đọc/ghi `0x181C`/`0x1A1C` (`rC/rD_TxScale_Jaguar2`, TX scale path C/D của 8814A) `[31:21] = 0` trên chip 2 path | `hal/hal_mp.c:905-1000, 2362, 2527` | Source; ý nghĩa của 0x181C/0x1A1C trên 8822B chưa xác định; chỉ MP |',
        '| 23 | `ex_hal8822b_wifi_only_hw_config` ghi LTE-coex offset 0x38 = 0x7700 ("gnt_wl=1 , gnt_bt=0") bằng 2 lệnh `phy_set_bb_reg` thẳng vào `0x1704`/`0x1700` (`0xC00F0038`) — không chờ `LTECOEX_READY_BIT_V1` như `ltecoex_reg_write_88xx`; chạy ở mọi lần init (`CONFIG_BT_COEXIST = n`) | `hal/btc/halbtc8822bwifionly.c`; `hal/rtl8822b/rtl8822b_halinit.c:340-350`; so với `halmac_common_88xx.c:3473-3523` | Source; tác động phụ thuộc thời gian xử lý của khối LTE-coex (chưa rõ) |',
        '| 24 | Field của `REG_MU_TX_CTL` (0x14C0) mà driver dùng (`R_MU_P1_WAIT_STATE_EN[16]`, `R_MU_RL[15:12]`, `R_MU_TAB_SEL[10:8]`, `R_EN_MU_MIMO[7]`) được định nghĩa cục bộ trong `rtl8822b_phy.c:1341-1357`, không có trong `halmac_bit_8822b.h` (header chỉ có `[6]` và `[5:0]`); `_config_beamformee_su` ghi `0x714[31:26]` với comment gốc "unknown?" | `hal/rtl8822b/rtl8822b_phy.c:1341-1357, 1780-1792` | Source |',
        '| 25 | Header chung `halmac_reg2.h` đặt tên `REG_LTECOEX_WRITE_DATA`/`READ_DATA` cho 0x7C4/0x7C8 (bố cục của chip khác), nhưng 8822B dùng 0x1700–0x1708 cho LTE-coex và 0x7C0 là `REG_IQ_DUMP_8822B`; phydm dùng 0x7C4/0x7C8 làm thanh ghi LA mode (`reg2/reg3`) → đọc code theo tên chung dễ hiểu nhầm | `hal/halmac/halmac_reg2.h`; `phydm_adc_sampling.c:926-940`; `halmac_reg_8822b.h` | Source; xem A13 |',
        '']
open(os.path.join(DOCS, '09_aliases_unused.md'), 'w', encoding='utf-8').write('\n'.join(out) + '\n')

# ------------------------------------------------------------------ 10
out = ['# 10 — Chỉ mục theo địa chỉ', '',
       'Mỗi dòng: địa chỉ → tên chính → trạng thái → trang danh mục. Trạng thái: A dùng thật · T chỉ trong bảng có điều kiện · M chỉ MP · G chỉ debug · '
       'X chỉ ở nhánh chết · R được tham chiếu không qua accessor · Z chỉ trong code tắt · D không dùng.', '']
for sp in ['MAC', 'BB', 'RF-direct', 'BB-pg', 'other', 'RF', 'USB-local', 'USB3-PHY', 'SDIO-local', 'MAC-other']:
    xs = [c for c in cat if c['space'] == sp]
    if not xs:
        continue
    out += [f'## {sp} ({len(xs)})', '', '| Địa chỉ | Tên | St | Trang |', '|---|---|:-:|---|']
    for c in xs:
        nm = c['names'][0] if c['names'] else '—'
        out.append(f"| `{c['addr_hex']}` | {esc(nm)} | {c['status']} | [{page_of(sp)}]({page_of(sp)}) |")
    out.append('')
open(os.path.join(DOCS, '10_index_by_address.md'), 'w', encoding='utf-8').write('\n'.join(out) + '\n')

# ------------------------------------------------------------------ 11
rows = []
for c in cat:
    for n in c['names']:
        rows.append((n, c['space'], c['addr_hex'], c['status']))
for n, rs in regs.items():
    if rs[0]['value'] is None:
        rows.append((n, '—', '(không giải được)', '—'))
rows.sort(key=lambda r: r[0].lower())
out = ['# 11 — Chỉ mục theo tên macro', '', f'{len(rows)} tên. Cột St như mục 10.', '',
       '| Tên | Không gian | Địa chỉ | St |', '|---|---|---|:-:|']
for n, sp, a, st in rows:
    out.append(f'| `{n}` | {sp} | `{a}` | {st} |')
open(os.path.join(DOCS, '11_index_by_name.md'), 'w', encoding='utf-8').write('\n'.join(out) + '\n')
print('ok', len(num), len(live), len(multi), len(dang), len(rows))
