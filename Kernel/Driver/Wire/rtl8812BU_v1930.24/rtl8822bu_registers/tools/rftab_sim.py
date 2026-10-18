#!/usr/bin/env python3
"""Simulate the phydm "header file" table walkers for 8822B (no hardware access).

Re-implements, from the raw C arrays:
  odm_read_and_config_mp_8822b_<table>()   hal/phydm/halrf/rtl8822b/halhwimg8822b_rf.c (radioa/radiob)
                                           hal/phydm/rtl8822b/halhwimg8822b_bb.c       (phy_reg/agc_tab)
  check_positive()                         same files (cut [27:24], pkg [15:12], intf [11:8], rfe [7:0])
Walker: v1 BIT31 = positive cond word (c_cond [29:28]: 0/1 IF/ELSE_IF, 2 ELSE, 3 ENDIF),
        v1 BIT30 = negative word -> evaluates the pending IF/ELSE_IF; else (addr, data) is written if matched.

Usage: py -I rftab_sim.py <driver_root> <radioa|radiob|phy_reg|agc_tab> <rfe_type> [cut=3] [pkg=0] [intf=2]
       add --dump to print every effective write (addr data), --final for last value per address.
"""
import re
import sys

args = [a for a in sys.argv[1:] if not a.startswith('--')]
flags = {a for a in sys.argv[1:] if a.startswith('--')}
root, table, rfe = args[0], args[1], int(args[2], 0)
cut = int(args[3], 0) if len(args) > 3 else 3
pkg = int(args[4], 0) if len(args) > 4 else 0
intf = int(args[5], 0) if len(args) > 5 else 2          # ODM_ITRF_USB = 0x2 (phydm_pre_define.h:399)
PLATFORM = 0x04                                          # ODM_CE
fn = {'radioa': 'hal/phydm/halrf/rtl8822b/halhwimg8822b_rf.c', 'radiob': 'hal/phydm/halrf/rtl8822b/halhwimg8822b_rf.c',
      'phy_reg': 'hal/phydm/rtl8822b/halhwimg8822b_bb.c', 'agc_tab': 'hal/phydm/rtl8822b/halhwimg8822b_bb.c'}[table]
src = open(f'{root}/{fn}', encoding='utf-8', errors='replace').read()
m = re.search(r'array_mp_8822b_%s\[\]\s*=\s*\{(.*?)\};' % table, src, re.S)
body = re.sub(r'/\*.*?\*/', '', m.group(1), flags=re.S)
arr = [int(x, 16) for x in re.findall(r'0x([0-9A-Fa-f]+)', body)]

cut_p = 15 if cut == 0 else cut                          # ODM_CUT_A -> 15
pkg_p = 15 if pkg == 0 else pkg
driver1 = (cut_p << 24) | ((intf & 0xF0) << 16) | (PLATFORM << 16) | (pkg_p << 12) | ((intf & 0x0F) << 8) | rfe


def check_positive(c1):
    for fld in (0x0F000000, 0x0000F000, 0x00000F00):
        if (c1 & fld) and (c1 & fld) != (driver1 & fld):
            return False
    return (c1 & 0xFF) == (driver1 & 0xFF)


writes, delays = [], 0
matched, skipped, pre = True, False, 0
i = 0
while i + 1 < len(arr):
    v1, v2 = arr[i], arr[i + 1]
    if v1 & 0xC0000000:
        if v1 & 0x80000000:
            c = (v1 >> 28) & 3
            if c == 3:
                matched, skipped = True, False
            elif c == 2:
                matched = not skipped
            else:
                pre = v1
        else:
            if not skipped:
                matched = skipped = check_positive(pre)
            else:
                matched = False
    elif matched:
        writes.append((v1, v2))
    i += 2

print(f'# {table}: rfe={rfe} cut={cut} pkg={pkg} intf={intf} driver1=0x{driver1:08X}  array words={len(arr)}')
eff = [(a, d) for a, d in writes]
dl = [a for a, _ in eff if a in (0xFE, 0xFFE, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD)]
print(f'effective entries: {len(eff)}  (delay pseudo-addr: {len(dl)})  distinct addr: {len({a for a, _ in eff})}')
if '--dump' in flags:
    for a, d in eff:
        print(f'0x{a:03X} 0x{d:05X}' if table.startswith('radio') else f'0x{a:04X} 0x{d:08X}')
if '--final' in flags:
    last = {}
    for a, d in eff:
        last[a] = d
    for a in sorted(last):
        print(f'0x{a:03X} = 0x{last[a]:05X}  (writes: {sum(1 for x, _ in eff if x == a)})')
