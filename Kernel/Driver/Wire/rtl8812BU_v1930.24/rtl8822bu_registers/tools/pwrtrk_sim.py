#!/usr/bin/env python3
"""Simulate 8822B thermal power tracking (MIX_MODE) register writes (no hardware access).

Re-implements:
  rfe -> txpowertrack table          hal/phydm/phydm_hwconfig.c:662-700 (READ_AND_CONFIG_MP(8822b, _txpowertrack_typeN))
  delta tables                       hal/phydm/halrf/rtl8822b/halhwimg8822b_rf.c (delta_swingidx_mp_<band><path>_<p|n>_txpwrtrk[_typeN]_8822b)
  get_delta_swing_table_8822b        hal/phydm/halrf/rtl8822b/halrf_8822b.c:409-455 (5G sub-band 36-64 / 100-144 / 149-177)
  odm_get_tracking_table             hal/phydm/halrf/halphyrf_ce.c:199-380 (hot: +up[delta]; cold: -down[delta])
  odm_tx_pwr_track_set_pwr8822b      halrf_8822b.c:231-407 (offset = 63 - min(tx_pwr_idx, 63))
  get_mix_mode_tx_agc_bb_swing_offset_8822b  halrf_8822b.c:104-167
  odm_pwrtrack_method_set_pwr8822b   halrf_8822b.c:169-229 -> BB 0xC94/0xE94[29:25] and 0xC1C/0xE1C[31:21]
  tx_scaling_table_jaguar            hal/phydm/halrf/halrf_powertracking_ce.c:570-608
Default OFDM index = position of 0xC1C[31:21] in the scaling table (PHY_REG sets 0xC1C = 0x40040053 -> 0x200 -> 24).

Usage: py -I pwrtrk_sim.py <driver_root> <rfe> <channel> <thermal> <eeprom_thermal> [tx_pwr_idx=40] [path=a|b] [default_idx=24]
"""
import re
import sys

root, rfe, ch, therm, ee = sys.argv[1], int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5])
txidx = int(sys.argv[6]) if len(sys.argv) > 6 else 40
path = sys.argv[7] if len(sys.argv) > 7 else 'a'
default_idx = int(sys.argv[8]) if len(sys.argv) > 8 else 24

hw = open(f'{root}/hal/phydm/phydm_hwconfig.c', encoding='utf-8', errors='replace').read()
i0 = hw.index('READ_AND_CONFIG_MP(8822b, _txpowertrack_type0)') - 400
seg = hw[i0:hw.index('READ_AND_CONFIG_MP(8822b, _txpowertrack);', i0) + 60]
suffix = None
for cond, suf in re.findall(r'if\s*\((.*?)\)\s*\n\s*READ_AND_CONFIG_MP\(8822b,\s*_txpowertrack(\w*)\)', seg):
    vals = [int(v) for v in re.findall(r'rfe_type\s*==\s*(\d+)', cond)]
    if rfe in vals:
        suffix = suf
        break
if suffix is None:
    suffix = ''                                   # else -> _txpowertrack
tsuf = suffix + '_8822b'                          # e.g. '_type0_8822b'

src = open(f'{root}/hal/phydm/halrf/rtl8822b/halhwimg8822b_rf.c', encoding='utf-8', errors='replace').read()


def table(band, p, sign):
    name = f'delta_swingidx_mp_{band}{p}_{sign}_txpwrtrk{tsuf}'
    m = re.search(re.escape(name) + r'\s*\[\][^=]*=\s*\{(.*?)\};', src, re.S)
    nums = [int(x) for x in re.findall(r'\d+', m.group(1))]
    if band == '5g':
        return [nums[k * 30:(k + 1) * 30] for k in range(3)]
    return nums


band = '2g' if ch <= 14 else '5g'
up, dn = table(band, path, 'p'), table(band, path, 'n')
if band == '5g':
    sb = 0 if 36 <= ch <= 64 else 1 if 100 <= ch <= 144 else 2 if 149 <= ch <= 177 else None
    if sb is None:      # get_delta_swing_table_8822b keeps the 2.4G pointers for other channels
        up, dn = table('2g', path, 'p'), table('2g', path, 'n')
    else:
        up, dn = up[sb], dn[sb]
delta = min(abs(therm - ee), 29)                  # TXPWR_TRACK_TABLE_SIZE - 1
absolute = up[delta] if therm > ee else -dn[delta]

TXSCALE = [0x081, 0x088, 0x090, 0x099, 0x0A2, 0x0AC, 0x0B6, 0x0C0, 0x0CC, 0x0D8, 0x0E5, 0x0F2, 0x101, 0x110, 0x120,
           0x131, 0x143, 0x156, 0x16A, 0x180, 0x197, 0x1AF, 0x1C8, 0x1E3, 0x200, 0x21E, 0x23E, 0x261, 0x285, 0x2AB,
           0x2D3, 0x2FE, 0x32B, 0x35C, 0x38E, 0x3C4, 0x3FE]
off = 63 - min(txidx, 63)
off = min(off, 0xF)
upper, lower = default_idx + 10, 0
if 0 <= absolute <= off:
    agc, sw = absolute, default_idx
elif absolute > off:
    agc, sw = off, min(default_idx + (absolute - off), upper)
else:
    agc = 0
    sw = default_idx + absolute if default_idx > -absolute else lower
    sw = max(sw, lower)
r1, r2 = (0xC94, 0xC1C) if path == 'a' else (0xE94, 0xE1C)
print(f'# rfe {rfe} -> table "_txpowertrack{suffix}"; ch {ch} ({band}); thermal {therm} vs PG {ee}: delta {delta}')
print(f'absolute_ofdm_swing_idx = {absolute:+d}; tx_pwr_idx {txidx} -> headroom offset {off}')
print(f'BB 0x{r1:03X}[29:25] <- {agc}           (TX AGC +{agc} index)')
print(f'BB 0x{r2:03X}[31:21] <- 0x{TXSCALE[sw]:03X}       (swing idx {sw}, {(sw - 24) * 0.5:+.1f} dB vs 0x200)')
