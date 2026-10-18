#!/usr/bin/env python3
"""Simulate which BB TXAGC dwords the 8822B driver writes (no hardware access).

Re-implements:
  set_tx_power_level_by_path()      hal/rtl8822b/rtl8822b_phy.c:711-728   (section order, survey / 2.4G gating)
  rtl8822b_set_tx_power_index()     hal/rtl8822b/rtl8822b_phy.c:770-812   (hal->txagc_set_buf, write when rate%4==3)
  config_phydm_write_txagc_8822b()  hal/phydm/rtl8822b/phydm_hal_api8822b.c:1078-1103  (W32 0x1D00/0x1D80 + (rate & 0xFC))
Rate sections: core/rtw_ieee80211.c:150-161; HW rate codes: include/hal_com.h:76-142 (MCS0 0x0C, MCS15 0x1B,
VHT1SS MCS0 0x2C, VHT2SS MCS0 0x36 .. MCS9 0x3F).
Power index per rate is an input (unknown without eFuse/PG data); default: index = HW rate (so bytes are traceable).

Usage: py -I txagc_sim.py [band=2g|5g] [survey=0|1] [path_count=2] [--order SECTION,...]
"""
import sys

args = [a for a in sys.argv[1:] if not a.startswith('--')]
band = args[0] if args else '2g'
survey = int(args[1]) if len(args) > 1 else 0
npath = int(args[2]) if len(args) > 2 else 2
order = None
if '--order' in sys.argv:
    order = sys.argv[sys.argv.index('--order') + 1].split(',')

SECT = {'CCK': range(0x00, 0x04), 'OFDM': range(0x04, 0x0C), 'HT1': range(0x0C, 0x14), 'HT2': range(0x14, 0x1C),
        'VHT1': range(0x2C, 0x36), 'VHT2': range(0x36, 0x40)}
if order is None:
    order = (['CCK'] if band == '2g' else []) + ['OFDM'] + ([] if survey else ['HT1', 'HT2', 'VHT1', 'VHT2'])

buf = 0          # hal->txagc_set_buf is ONE variable shared by all paths
total = 0
for path in range(npath):
    base = (0x1D00, 0x1D80)[path]
    writes = []
    for sec in order:
        for hw in SECT[sec]:
            shift = hw % 4
            buf |= (hw & 0xFF) << (shift * 8)
            if shift != 3:
                continue
            writes.append((base + (hw & 0xFC), buf, sec))
            buf = 0
    total += len(writes)
    print(f'path {"AB"[path]}: {len(writes)} x W32')
    for a, v, sec in writes:
        print(f'   0x{a:04X} <- 0x{v:08X}  (flushed while in {sec})')
print(f'total control transfers: {total};  leftover txagc_set_buf = 0x{buf:08X}' + ('  <-- NOT WRITTEN, leaks into next call' if buf else ''))
