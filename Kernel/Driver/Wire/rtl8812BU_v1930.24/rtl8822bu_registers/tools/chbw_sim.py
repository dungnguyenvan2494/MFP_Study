#!/usr/bin/env python3
"""Simulate the register arithmetic of 8822B channel / bandwidth switching.

Re-implements (no hardware access):
  config_phydm_switch_channel_8822b()   hal/phydm/rtl8822b/phydm_hal_api8822b.c:1839-2030
  config_phydm_switch_bandwidth_8822b() same file, switch(bandwidth) part
for RF 0x18 (channel/band/BW word), RF 0xBE[17:15], BB 0x958[4:0] (AGC table),
BB 0x860[28:17] (clock-offset tracking centre freq), BB 0x8AC low bits.
Starting register values are inputs (unknown on real HW); defaults are 0.

Usage: py -I chbw_sim.py <channel> <bw 20|40|80> [primary_idx=0] [rf18_old=0x0] [bb8ac_old=0x0] [rfe_type=0]
"""
import sys

ch = int(sys.argv[1]); bw = int(sys.argv[2])
pri = int(sys.argv[3]) if len(sys.argv) > 3 else 0
rf18 = int(sys.argv[4], 0) if len(sys.argv) > 4 else 0
bb8ac = int(sys.argv[5], 0) if len(sys.argv) > 5 else 0
rfe = int(sys.argv[6]) if len(sys.argv) > 6 else 0
B = lambda n: 1 << n
low_band = [0x7, 0x6, 0x6, 0x5, 0x0, 0x0, 0x7, 0xff, 0x6, 0x5, 0x0, 0x0, 0x7, 0x6, 0x6]          # :1845
middle_band = [0x6, 0x5, 0x0, 0x0, 0x7, 0x6, 0x6, 0xff, 0x0, 0x0, 0x7, 0x6, 0x6, 0x5, 0x0, 0xff,
               0x7, 0x6, 0x6, 0x5, 0x0, 0x0, 0x7]                                                # :1847
high_band = [0x5, 0x5, 0x0, 0x7, 0x7, 0x6, 0x5, 0xff, 0x0, 0x7, 0x7, 0x6, 0x5, 0x5, 0x0]        # :1850

print(f'# channel {ch}, BW{bw}, primary_idx {pri}, RF18 old 0x{rf18:05X}, BB8AC old 0x{bb8ac:08X}, rfe {rfe}')
# ---- channel part
r = rf18 & ~(B(18) | B(17) | 0xFF) & 0xFFFFF
writes = []
if ch <= 14:
    r |= ch
    writes += [('BB 0x958[4:0]', 0x0, 'AGC table 0 (2.4G)'), ('BB 0x860[28:17]', 0x96A, 'clock offset tracking centre')]
    if ch == 14:
        writes += [('BB 0xA24', 0x00006577, 'CCK TX filter ch14'), ('BB 0xA28[15:0]', 0x0000, '')]
    else:
        writes += [('BB 0xA24', 0x384F6577, 'CCK TX filter'), ('BB 0xA28[15:0]', 0x1525, '')]
elif ch > 35:
    r |= ch
    if rfe not in (15, 16):
        idx = 1 if 36 <= ch <= 64 else 2 if 100 <= ch <= 144 else 3 if ch >= 149 else None
        if idx is None:
            sys.exit('channel not supported -> return false')
        writes.append(('BB 0x958[4:0]', idx, 'AGC table for 5G sub-band'))
    c860 = 0x494 if 36 <= ch <= 48 else 0x453 if 52 <= ch <= 64 else 0x452 if 100 <= ch <= 116 else 0x412 if 118 <= ch <= 177 else None
    writes.append(('BB 0x860[28:17]', c860, 'clock offset tracking centre'))
    if ch > 144:
        r |= B(18)
    elif ch >= 80:
        r |= B(17)
else:
    sys.exit('channel 15..35 -> return false')
if ch <= 14:
    be = 0x0
elif 36 <= ch <= 64:
    be = low_band[(ch - 36) >> 1]
elif 100 <= ch <= 144:
    be = middle_band[(ch - 100) >> 1]
elif 149 <= ch <= 177:
    be = high_band[(ch - 149) >> 1]
else:
    be = 0xff
writes.append(('RF-A 0xBE[17:15]', be, 'synth parameter per channel (0xff -> error)'))
writes.append(('RF-A 0xDF[18]', 1 if ch == 144 else 0, 'ch144 special'))
print(f'RF 0x18 after channel part: 0x{r:05X}  (channel {ch} in [7:0], BIT17/BIT18 = 5G sub-band)')
# ---- bandwidth part (reads RF18 again: use the value just written)
if ch == 165:
    bw = 20
if bw == 20:
    v = (bb8ac & 0xFFCFFC00) | 0
    r |= B(11) | B(10)
elif bw == 40:
    v = (bb8ac & 0xFF3FF300) | ((pri & 0xF) << 2) | 1
    r = (r & ~(B(11) | B(10))) | B(11)
    writes.append(('BB 0xA00[4]', 1 if pri == 1 else 0, 'CCK primary channel'))
elif bw == 80:
    v = (bb8ac & 0xFCEFCF00) | ((pri & 0xF) << 2) | 2
    r = (r & ~(B(11) | B(10))) | B(10)
    if rfe in (2, 3, 17):
        writes += [('BB 0x840[15:12]', 0x6, 'SD4 TP'), ('BB 0x8C8[10]', 1, '')]
else:
    sys.exit('bw')
writes += [('BB 0x8AC', v, 'rf mode [1:0], primary ch [5:2], small BW [7:6] (per #if 0 comments)'),
           ('BB 0x8C4[30]', 1, 'ADC buffer clock')]
print(f'RF 0x18 after BW part:      0x{r:05X}  (BW bits [11:10] = {(r >> 10) & 3:02b})')
for reg, val, why in writes:
    print(f'  {reg:18s} <- 0x{val:X}   {why}')
