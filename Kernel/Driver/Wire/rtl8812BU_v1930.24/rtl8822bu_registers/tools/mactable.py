#!/usr/bin/env python3
"""Render array_mp_8822b_mac_reg (default MAC init table) with field decoding.

Each entry is (byte address, 8-bit value) written by odm_config_mac_8822b() ->
odm_write_1byte() (hal/phydm/rtl8822b/phydm_regconfig8822b.c:142-156), unless
/lib/firmware/MAC_REG.txt exists (rtw_load_phy_file BIT0, 01_build_config §1.3).

Usage: py -I mactable.py <work_dir> <out.md>
"""
import json, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bitname import Decoder, fmt

W, out = sys.argv[1:3]
T = json.load(open(os.path.join(W, 'tables.json')))
dec = Decoder(W)
rows = [t for t in T if t['table'] == 'array_mp_8822b_mac_reg']
lines = ['# Bảng `array_mp_8822b_mac_reg` — giá trị MAC mặc định khi khởi tạo', '',
         'Nguồn: `hal/phydm/rtl8822b/halhwimg8822b_mac.c:144-268`; ghi bởi `odm_read_and_config_mp_8822b_mac_reg()` '
         '(cùng file, `:272-330`) → `odm_config_mac_8822b()` → `odm_write_1byte()` (ghi **cả byte**, không RMW). '
         'Bảng này không có header điều kiện (mọi mục đều ghi) [TOOL: `tools/tables.py`]. '
         'Nếu `/lib/firmware/MAC_REG.txt` tồn tại thì file đó được dùng thay bảng [KIỂM TRA TRÊN BOARD]. '
         'Cột "Field (header)" giải bằng `tools/bitname.py`: liệt kê các field mà byte đó chạm tới.', '',
         f'{len(rows)} mục.', '',
         '| # | Dòng | Địa chỉ | Giá trị | Thanh ghi chứa | Field (header) bị ghi |', '|---:|---:|---|---|---|---|']
for i, t in enumerate(rows, 1):
    reg, base, hits = dec.decode(t['addr'], 0xFF)
    regs = f'`{reg}`+{t["addr"] - base}' if reg else '—'
    lines.append(f"| {i} | {t['line']} | `0x{t['addr']:04X}` | `0x{t['value']:02X}` | {regs} | "
                 f"{', '.join(fmt(h) for h in hits)[:200] or '—'} |")
open(out, 'w', encoding='utf-8').write('\n'.join(lines) + '\n')
print(len(rows))
