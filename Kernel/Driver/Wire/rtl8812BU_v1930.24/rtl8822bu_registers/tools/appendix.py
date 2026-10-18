#!/usr/bin/env python3
"""Generate per-range evidence appendices (Markdown) for the group documents.

For every catalog row in the range whose status is not D/Z: names + definition
sites, bit/field macros of its section that are referenced, every code access
(file:line, function, accessor, width, mask, live/dead + reason) with the source
line, and table entries (compressed). This is the completeness layer: the hand-
written group docs analyse the important registers and point here for the rest.

Usage: py -I appendix.py <driver_dir> <work_dir> <out_dir>
"""
import json, os, re, sys
from collections import defaultdict

drv, W, OUT = sys.argv[1:4]
os.makedirs(OUT, exist_ok=True)
L = lambda n: json.load(open(os.path.join(W, n)))
cat, access, bits, tables = L('catalog.json'), L('access.json'), L('bits.json'), L('tables.json')

RANGES = [
    ('A01_mac_000_0ff', 'MAC', 0x0000, 0x00FF, 'MAC 0x0000–0x00FF (system, power, GPIO, eFuse, MCU)'),
    ('A02_mac_100_2ff', 'MAC', 0x0100, 0x02FF, 'MAC 0x0100–0x02FF (CR, mailbox, TRX DMA, FIFO)'),
    ('A03_mac_300_4ff', 'MAC', 0x0300, 0x04FF, 'MAC 0x0300–0x04FF (PCIe, queue info, protocol, rate)'),
    ('A04_mac_500_5ff', 'MAC', 0x0500, 0x05FF, 'MAC 0x0500–0x05FF (EDCA, timing, beacon, TSF)'),
    ('A05_mac_600_7ff', 'MAC', 0x0600, 0x07FF, 'MAC 0x0600–0x07FF (WMAC, RX filter, security, WoW)'),
    ('A06_mac_1000_17ff', 'MAC', 0x1000, 0x17FF, 'MAC 0x1000–0x17FF (extended MAC: CPU, DDMA, BF/MU, LTE)'),
    ('A07_bb_800_fff', 'BB', 0x0800, 0x0FFF, 'BB 0x0800–0x0FFF'),
    ('A08_bb_1800_1fff', 'BB', 0x1800, 0x1FFF, 'BB 0x1800–0x1FFF'),
    ('A09_rf', 'RF', 0x00, 0xFF, 'RF path A/B 0x00–0xFF'),
    ('A10_usb_local', 'USB-local', 0xFE00, 0xFFFF, 'USB-local 0xFE00–0xFFFF'),
]

_src = {}
def line(f, n):
    if f not in _src:
        try:
            _src[f] = open(os.path.join(drv, f), encoding='latin-1').read().split('\n')
        except OSError:
            _src[f] = []
    return _src[f][n - 1].strip() if 0 < n <= len(_src[f]) else ''

def esc(s):
    return s.replace('|', '\\|').replace('`', "'")

bits_by_reg = defaultdict(list)
for b, v in bits.items():
    if v['reg'] and v['uses_total']:
        bits_by_reg[v['reg']].append((b, v))

acc_by = defaultdict(list)
for a in access:
    if a['value'] is None:
        continue
    sp = 'RF' if a['space'] == 'RF' else {'RF-direct(BB)': 'RF-direct'}.get(a.get('region'), a.get('region'))
    acc_by[(sp, a['value'])].append(a)

tab_by = defaultdict(list)
for t in tables:
    if t['kind'] in ('delay', 'end') or t.get('cmd') in ('HALMAC_PWR_CMD_DELAY', 'HALMAC_PWR_CMD_END'):
        continue
    sp = {'write20': 'RF', 'write8': 'MAC', 'pinmux': 'MAC', 'pwrseq': 'MAC'}.get(t['kind'])
    if t['kind'] == 'write32':
        sp = 'BB' if (0x800 <= t['addr'] < 0x1000 or 0x1800 <= t['addr'] < 0x2000) else 'MAC'
    if t['kind'] == 'pwrseq' and t['addr'] >= 0xFE00:
        sp = 'USB-local'
    if sp:
        tab_by[(sp, t['addr'])].append(t)

def why(a):
    if a['live']:
        return 'live'
    g = a['guard']
    r = a['reach']
    w = '; '.join(f"{x[0]}:{x[1][:60]}" for x in a['guard_why'][:1])
    return f"{g if g not in ('none', 'selected', 'unknown') else ''}{'/' if g not in ('none','selected','unknown') and r != 'reachable?' else ''}{r if r != 'reachable?' else ''} {w}".strip()

index = []
for fname, space, lo, hi, title in RANGES:
    rows = [c for c in cat if c['space'] == space and lo <= c['addr'] <= hi and c['status'] not in 'DZ']
    out = [f'# Phụ lục {fname[:3]} — {title}', '',
           'Sinh tự động bởi `tools/appendix.py` từ `work/*.json` [TOOL]. Mỗi mục liệt kê **mọi** truy cập tìm thấy '
           'trong code active (kể cả nhánh chết, có ghi lý do) và mọi mục bảng. Các thanh ghi St=D/Z (không dùng) '
           'chỉ có trong `04*_catalog_*.md`. Cột "Trạng thái": `live` = chạy được với RTL8822BU; còn lại là lý do loại.', '',
           f'Tổng: {len(rows)} địa chỉ.', '']
    for c in rows:
        nm = ', '.join(f'`{n}`' for n in c['names']) or '*(không có macro)*'
        out.append(f"## `{c['addr_hex']}` — St {c['status']} — {nm}")
        out.append('')
        if c['name_defs']:
            out.append('Định nghĩa: ' + '; '.join(f"`{d}`" for d in c['name_defs'][:6]))
            out.append('')
        bl = []
        for n in c['names']:
            for b, v in bits_by_reg.get(n, []):
                if b.startswith(('BIT_MASK_', 'BIT_SHIFT_', 'BITS_')):
                    continue
                bl.append(f"`{b}` ({v['file'].split('/')[-1]}:{v['line']}, {v['uses_total']} nơi)")
        if bl:
            out.append('Bit/field macro được dùng: ' + ', '.join(dict.fromkeys(bl)))
            out.append('')
        acc = sorted(acc_by.get((space, c['addr']), []), key=lambda a: (not a['live'], a['file'], a['line']))
        if acc:
            out += ['| Vị trí | Hàm | Accessor | W | Mask | Trạng thái | Code |', '|---|---|---|---|---|---|---|']
            for a in acc[:60]:
                via = f" (qua mảng {a['via_array'].split(' ')[0]})" if a.get('via_array') else ''
                out.append(f"| `{a['file']}:{a['line']}` | {a['func']} | {a['acc']}{via} | {a['width']} | "
                           f"{esc(a.get('mask', ''))[:24]} | {esc(why(a))[:70]} | `{esc(line(a['file'], a['line']))[:110]}` |")
            if len(acc) > 60:
                out.append(f'| … | còn {len(acc) - 60} truy cập | | | | | |')
            out.append('')
        tl = tab_by.get((space, c['addr']), [])
        if tl:
            byt = defaultdict(list)
            for t in tl:
                byt[t['table']].append(t)
            for tn, ts in byt.items():
                t0 = ts[0]
                if t0['kind'] == 'pwrseq':
                    det = '; '.join(f"{t['cmd'].replace('HALMAC_PWR_CMD_','')} msk={t['mask']} val={t['value']} intf={t['intf'].replace('HALMAC_PWR_INTF_','')} @{t['line']}" for t in ts)
                elif t0['kind'] == 'pinmux':
                    det = '; '.join(f"{t['func']}/{t['gpio']} msk={t['mask']} val={t['value']}" for t in ts)
                else:
                    vals = sorted({hex(t['value']) if isinstance(t['value'], int) else str(t['value']) for t in ts})
                    conds = sorted({str(t.get('cond')) for t in ts})
                    det = f"{len(ts)} mục, giá trị {', '.join(vals[:8])}{' …' if len(vals) > 8 else ''}; điều kiện {', '.join(conds[:3])}"
                out.append(f"- Bảng `{tn}` (`{t0['file'].split('/')[-1]}:{t0['line']}`): {esc(det)[:400]}")
            out.append('')
    open(os.path.join(OUT, fname + '.md'), 'w', encoding='utf-8').write('\n'.join(out) + '\n')
    index.append((fname, title, len(rows)))

idx = ['# Phụ lục bằng chứng theo dải địa chỉ', '', '| Phụ lục | Dải | Số địa chỉ (St ≠ D/Z) |', '|---|---|---:|']
for f, t, n in index:
    idx.append(f'| [{f}]({f}.md) | {t} | {n} |')
open(os.path.join(OUT, 'README.md'), 'w', encoding='utf-8').write('\n'.join(idx) + '\n')
print(index)
