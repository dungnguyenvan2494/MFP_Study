#!/usr/bin/env python3
"""Register addresses touched through *tables* (not through a literal call site).

Tables parsed (only if their lines are active in mask.json):
  phydm  array_mp_8822b_{mac_reg, agc_tab, phy_reg, radioa, radiob}
         pairs (addr, data) with condition headers (BIT31/BIT30 set in v1),
         decoded like odm_read_and_config_mp_8822b_*(): IF / ELSE IF / ELSE / ENDIF.
         phy_reg pseudo-addresses 0xF9..0xFE = delays (odm_config_bb_phy_8822b),
         radio 0xFE / 0xFFE = delays (odm_config_rf_reg_8822b).
  phydm  array_mp_8822b_phy_reg_pg*  6-tuples (band, path, tx_num, addr, mask, data)
         -> stored by phy_store_tx_power_by_rate(), NOT written directly.
  halmac TRANS_*_8822B power sequences {offset, cut, intf, base, cmd, msk, value}
  halmac usb2/usb3_phy_param_8822b {offset, value, ip_sel, cut, platform}
  halmac GPIOx_*_8822B pinmux macros {func, id, type, offset, msk, value}

Usage: py -I tables.py <driver_dir> <mask.json> <out tables.json>
"""
import json, os, re, sys

drv, mask_path, out = sys.argv[1:4]
mask = json.load(open(mask_path))
active = {f: set(v) for f, v in mask['active'].items()}
DEFS = mask['defs']

def src(f):
    return open(os.path.join(drv, f), encoding='latin-1').read()

def arrays(f, name_rx):
    """Yield (name, start_line, [(value, line)])."""
    s = src(f)
    for m in re.finditer(r'\bconst\s+u32\s+(%s)\s*\[\]\s*=\s*\{(.*?)\};' % name_rx, s, re.S):
        line0 = s.count('\n', 0, m.start()) + 1
        if line0 not in active.get(f, ()):
            continue
        body_off = m.start(2)
        vals = []
        for v in re.finditer(r'0[xX][0-9a-fA-F]+|\b\d+\b', m.group(2)):
            vals.append((int(v.group(0), 0), s.count('\n', 0, body_off + v.start()) + 1))
        yield m.group(1), line0, vals

def decode_cond(v1, v2):
    """Fields used by check_positive() (halhwimg8822b_bb.c:64)."""
    d = {}
    if v1 & 0x0F000000: d['cut'] = (v1 >> 24) & 0xF
    if v1 & 0x0000F000: d['pkg'] = (v1 >> 12) & 0xF
    if v1 & 0x00000F00: d['intf'] = (v1 >> 8) & 0xF
    d['rfe'] = v1 & 0xFF
    if v2: d['type_ext'] = hex(v2)
    return d

COND_NAMES = {0: 'IF', 1: 'ELSE_IF', 2: 'ELSE', 3: 'ENDIF'}   # COND_* in phydm (bits 29:28)

entries = []
F_BB = 'hal/phydm/rtl8822b/halhwimg8822b_bb.c'
F_MAC = 'hal/phydm/rtl8822b/halhwimg8822b_mac.c'
F_RF = 'hal/phydm/halrf/rtl8822b/halhwimg8822b_rf.c'

for f, rx, space in ((F_MAC, r'array_mp_8822b_mac_reg', 'MAC'),
                     (F_BB, r'array_mp_8822b_(?:agc_tab|phy_reg)', 'BB'),
                     (F_RF, r'array_mp_8822b_radio[ab]', 'RF')):
    for name, l0, vals in arrays(f, rx):
        i, cond, branch = 0, None, None
        while i + 1 < len(vals):
            (v1, ln), (v2, _) = vals[i], vals[i + 1]
            if v1 & 0xC0000000:
                if v1 & 0x80000000:
                    c = (v1 >> 28) & 0x3
                    if c == 3:
                        cond = None; branch = None
                    elif c == 2:
                        branch = 'ELSE'
                    else:
                        branch = COND_NAMES[c] + ' ' + json.dumps(decode_cond(v1, v2))
                        cond = branch
                i += 2
                continue
            e = {'table': name, 'file': f, 'line': ln, 'space': space, 'addr': v1, 'value': v2,
                 'cond': branch}
            if space == 'RF':
                e['path'] = 'A' if name.endswith('radioa') else 'B'
                e['kind'] = 'delay' if v1 in (0xFE, 0xFFE) else 'write20'
            elif name.endswith('mac_reg'):
                e['kind'] = 'write8'
            elif name.endswith('phy_reg') and 0xF9 <= v1 <= 0xFE:
                e['kind'] = 'delay'
            else:
                e['kind'] = 'write32'
            entries.append(e)
            i += 2
    if f == F_BB:
        for name, l0, vals in arrays(f, r'array_mp_8822b_phy_reg_pg\w*'):
            for k in range(0, len(vals) - 5, 6):
                band, path, txn, addr, msk, data = [v for v, _ in vals[k:k + 6]]
                entries.append({'table': name, 'file': f, 'line': vals[k][1], 'space': 'BB',
                                'addr': addr, 'value': data, 'mask': msk, 'kind': 'pg-store',
                                'band': band, 'path': path, 'tx_num': txn, 'cond': None})

# ---------------------------------------------------------------- power sequences
F_PWR = 'hal/halmac/halmac_88xx/halmac_8822b/halmac_pwr_seq_8822b.c'
s = src(F_PWR)
consts = {n: d[0][3] for n, d in DEFS.items() if n.startswith('HALMAC_PWR_')}
def cval(t):
    t = t.strip()
    if t in consts:
        return cval(consts[t])
    t2 = re.sub(r'BIT\((\d+)\)', lambda m: str(1 << int(m.group(1))), t)
    t2 = re.sub(r'\b[A-Z_][A-Z0-9_]*\b', lambda m: cval(m.group(0)) if m.group(0) in consts else m.group(0), t2)
    try:
        return str(eval(t2, {'__builtins__': {}}, {}))
    except Exception:
        return t
for m in re.finditer(r'static\s+struct\s+halmac_wlan_pwr_cfg\s+(\w+)\[\]\s*=\s*\{(.*?)\n\};', s, re.S):
    l0 = s.count('\n', 0, m.start()) + 1
    if l0 not in active.get(F_PWR, ()):
        continue
    for it in re.finditer(r'\{\s*(0x[0-9a-fA-F]+)\s*,(.*?)\}', m.group(2), re.S):
        parts = [p.strip() for p in it.group(2).split(',')]
        if len(parts) != 6:
            continue
        flat = parts
        ln = s.count('\n', 0, m.start(2) + it.start()) + 1
        intf_v = int(cval(parts[1])) if cval(parts[1]).isdigit() else None
        e = {'table': m.group(1), 'file': F_PWR, 'line': ln, 'addr': int(it.group(1), 16),
             'cut': parts[0], 'intf': parts[1], 'base': parts[2], 'cmd': parts[3],
             'mask': flat[-2], 'value': flat[-1],
             'usb': None if intf_v is None else bool(intf_v & 0x2)}
        b = cval(parts[2])
        e['space'] = {'0': 'MAC', '1': 'USB?', '2': 'PCIe?', '3': 'SDIO-local'}.get(b, parts[2])
        entries.append(dict(e, kind='pwrseq'))

# ---------------------------------------------------------------- USB PHY params
F_PHY = 'hal/halmac/halmac_88xx/halmac_8822b/halmac_phy_8822b.c'
s = src(F_PHY)
for m in re.finditer(r'struct\s+halmac_intf_phy_para\s+(\w+)\[\]\s*=\s*\{(.*?)\n\};', s, re.S):
    l0 = s.count('\n', 0, m.start()) + 1
    if l0 not in active.get(F_PHY, ()) or 'usb' not in m.group(1):
        continue
    for it in re.finditer(r'\{\s*(0x[0-9a-fA-F]+)\s*,\s*(0x[0-9a-fA-F]+)\s*,(.*?)\}', m.group(2), re.S):
        ln = s.count('\n', 0, m.start(2) + it.start()) + 1
        rest = [p.strip() for p in it.group(3).split(',')]
        entries.append({'table': m.group(1), 'file': F_PHY, 'line': ln,
                        'space': 'USB2-PHY' if 'usb2' in m.group(1) else 'USB3-PHY',
                        'addr': int(it.group(1), 16), 'value': int(it.group(2), 16),
                        'cut': rest[1] if len(rest) > 1 else '', 'kind': 'end' if it.group(1).upper() == '0XFFFF' else 'intf-phy'})

# ---------------------------------------------------------------- GPIO pinmux
F_GPIO = 'hal/halmac/halmac_88xx/halmac_8822b/halmac_gpio_8822b.c'
for n, ds in DEFS.items():
    for f, l, p, b in ds:
        if f != F_GPIO or not n.startswith('GPIO'):
            continue
        mm = re.match(r'\{\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*,\s*(0x[0-9a-fA-F]+)\s*,\s*(.+?)\s*,\s*(.+?)\s*\}$', b)
        if mm:
            entries.append({'table': n, 'file': f, 'line': l, 'space': 'MAC', 'addr': int(mm.group(4), 16),
                            'func': mm.group(1), 'gpio': mm.group(2), 'mask': mm.group(5),
                            'value': mm.group(6), 'kind': 'pinmux'})

json.dump(entries, open(out, 'w'), indent=0)
from collections import Counter
print(Counter((e['table'] if e['kind'] != 'pinmux' else 'pinmux', e['kind']) for e in entries), file=sys.stderr)
