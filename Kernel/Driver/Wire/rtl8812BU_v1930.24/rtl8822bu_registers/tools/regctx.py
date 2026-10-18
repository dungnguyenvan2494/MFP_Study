#!/usr/bin/env python3
"""Dump evidence for registers in an address range, for writing group docs.

For each catalog row in [lo, hi] of SPACE: names + definition lines, bit macros of
its section (with use counts), every access with the source line text, guard and
function, and table entries (compressed).

Usage: py -I regctx.py <driver_dir> <work_dir> SPACE LO HI [--all]
  --all  also list rows with status D/Z (otherwise only rows that are used)
"""
import json, os, re, sys
from collections import defaultdict

drv, W, space, lo, hi = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4], 0), int(sys.argv[5], 0)
show_all = '--all' in sys.argv
brief = '--brief' in sys.argv
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
L = lambda n: json.load(open(os.path.join(W, n)))
cat, access, bits, tables = L('catalog.json'), L('access.json'), L('bits.json'), L('tables.json')

_src = {}
def line(f, n):
    if f not in _src:
        _src[f] = open(os.path.join(drv, f), encoding='latin-1').read().split('\n')
    return _src[f][n - 1].strip() if 0 < n <= len(_src[f]) else ''

bits_by_reg = defaultdict(list)
for b, v in bits.items():
    if v['reg']:
        bits_by_reg[v['reg']].append((b, v))

for c in cat:
    if c['space'] != space or not (lo <= c['addr'] <= hi):
        continue
    if not show_all and c['status'] in 'DZ':
        continue
    print(f"\n=== {c['space']} {c['addr_hex']}  St={c['status']}  names={c['names']}")
    for d in c['name_defs'][:6]:
        print('   def', d)
    seen = set(); nb = [0]
    for n in c['names']:
        for b, v in sorted(bits_by_reg.get(n, []), key=lambda x: x[1]['line']):
            base = re.sub(r'^(BIT_SHIFT_|BIT_MASK_|BIT_GET_|BIT_SET_|BIT_CLEAR_|BITS_|BIT_)', '', b).replace('_8822B', '')
            if base in seen and not v['uses_total']:
                continue
            seen.add(base)
            if b.startswith(('BIT_GET_', 'BIT_SET_', 'BIT_CLEAR_', 'BITS_', 'BIT_MASK_')) and not v['uses_total']:
                continue
            if brief and not v['uses_total']:
                nb[0] += 1
                continue
            print(f"   bit {b:42s} {v['file'].split('/')[-1]}:{v['line']} uses={v['uses_total']} {v['where'][:2] if v['uses_total'] else ''}")
    if brief and nb[0]:
        print(f"   ({nb[0]} bit/field khác được định nghĩa, không dùng)")
    for a in access:
        if a['value'] is None:
            continue
        sp = 'RF' if a['space'] == 'RF' else {'RF-direct(BB)': 'RF-direct'}.get(a.get('region'), a.get('region'))
        if sp != c['space'] or a['value'] != c['addr']:
            continue
        flag = 'LIVE' if a['live'] else f"dead({a['guard']},{a['reach']})"
        via = f" via {a['via_array']}" if a.get('via_array') else ''
        g = ' | '.join(f"{w[0]}:{w[1][:70]}" for w in a['guard_why'][:2])
        print(f"   {flag:28s} {a['file']}:{a['line']} {a['func']}() {a['acc']} w{a['width']} {a.get('path','')}{via}")
        print(f"        > {line(a['file'], a['line'])[:150]}" + (f"   [{g}]" if g else ''))
    tabs = [t for t in tables if t['addr'] == c['addr'] and t.get('cmd') not in ('HALMAC_PWR_CMD_DELAY', 'HALMAC_PWR_CMD_END') and (
        (space == 'RF' and t['kind'] == 'write20') or (space != 'RF' and t['kind'] not in ('write20', 'delay', 'end')))]
    by = defaultdict(list)
    for t in tabs:
        by[t['table']].append(t)
    for tn, ts in by.items():
        t0 = ts[0]
        vals = sorted({str(t.get('value')) if not isinstance(t.get('value'), int) else hex(t['value']) for t in ts})
        extra = ''
        if t0['kind'] == 'pwrseq':
            extra = ' ' + '; '.join(f"{t['cmd'].replace('HALMAC_PWR_CMD_','')} msk={t['mask']} val={t['value']} intf={t['intf'].replace('HALMAC_PWR_INTF_','')} cut={t['cut'].replace('HALMAC_PWR_CUT_','')} @{t['line']}" for t in ts)
        elif t0['kind'] == 'pinmux':
            extra = ' ' + '; '.join(f"{t['func']}/{t['gpio']} msk={t['mask']} val={t['value']}" for t in ts)
        else:
            conds = sorted({str(t.get('cond')) for t in ts})
            extra = f" n={len(ts)} values={vals[:6]} conds={conds[:4]}"
        print(f"   table {tn} {t0['file'].split('/')[-1]}:{t0['line']}{extra}")
