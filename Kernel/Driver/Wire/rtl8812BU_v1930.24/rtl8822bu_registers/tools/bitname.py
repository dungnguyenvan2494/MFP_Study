#!/usr/bin/env python3
"""Decode (byte address, 8-bit mask) or (address, 32-bit mask) into register + bit names.

The containing register is the closest named MAC register at or below the address
whose section in halmac_bit_8822b.h / halmac_bit2.h defines bits; bit index is
(addr - base) * 8 + n. Single-bit macros BIT(n) and fields (BIT_SHIFT_x/BIT_MASK_x)
of that section are matched.

Usage: py -I bitname.py <work_dir> ADDR MASK [--dword] [ADDR MASK [--dword] ...]
       MASK applies to the byte at ADDR; a '--dword' right after a pair makes THAT pair's MASK a
       32-bit mask relative to ADDR (per pair -- before 2026-10-18 the flag was global)
As a module: Decoder(work_dir).decode(addr, mask, dword=False) -> (reg_name, base, [(name, lo, hi)])
"""
import json, os, re, sys
from collections import defaultdict


class Decoder:
    def __init__(self, W):
        mask = json.load(open(os.path.join(W, 'mask.json')))
        regs = json.load(open(os.path.join(W, 'regs.json')))
        bits = json.load(open(os.path.join(W, 'bits.json')))
        self.D = mask['defs']
        self.sec = defaultdict(list)          # section register -> [(mask32, name, lo)]
        for b, v in bits.items():
            if not v['reg']:
                continue
            if b.startswith('BIT_SHIFT_'):
                base = b[len('BIT_SHIFT_'):]
                # 64-bit registers: halmac writes (N & CPU_OPT_WIDTH) with N >= 32 (CPU_OPT_WIDTH = 0x1F);
                # the field then lives at bit N of the 64-bit register (second dword)
                raw = self.D.get(b, [[None, None, None, '']])[0][3]
                m64 = re.match(r'\(?\s*(\d+)\s*&\s*CPU_OPT_WIDTH', raw)
                sh = int(m64.group(1)) if m64 else self._val(b)
                mk = self._val('BIT_MASK_' + base)
                if sh is not None and mk is not None:
                    self.sec[v['reg']].append(((mk << sh) & 0xFFFFFFFFFFFFFFFF, base, sh))
            elif not b.startswith(('BIT_MASK_', 'BIT_GET_', 'BIT_SET_', 'BIT_CLEAR_', 'BITS_')):
                x = self._val(b)
                if x and (x & (x - 1)) == 0:
                    self.sec[v['reg']].append((x, b[4:], x.bit_length() - 1))
        self.mac = sorted((r[0]['value'], n) for n, r in regs.items()
                          if r[0]['value'] is not None and r[0]['space'] in ('MAC', 'USB-local') and n in self.sec)

    def _val(self, n):
        d = self.D.get(n)
        if not d:
            return None
        b = d[0][3].strip()
        m = re.fullmatch(r'BIT\((\d+)\)', b)
        if m:
            return 1 << int(m.group(1))
        try:
            return int(b.rstrip('uUlL'), 0)
        except ValueError:
            return None

    def decode(self, a, m, dword=False):
        cands = [(v, n) for v, n in self.mac if v <= a < v + 8]
        if not cands:
            return None, None, []
        best = defaultdict(list)
        for v, n in cands:
            best[v].append(n)
        v = max(best)
        names = sorted(best[v], key=lambda n: (not n.endswith('_8822B'), n))
        sh = (a - v) * 8                     # MASK is relative to ADDR in both modes
        m32 = (m << sh) & 0xFFFFFFFFFFFFFFFF
        hits = []
        for n in names:
            for bm, bn, lo in sorted(self.sec[n], key=lambda x: -x[2]):
                if bm & m32:
                    hits.append((bn, lo, bm.bit_length() - 1))
            if hits:
                break
        seen, uniq = set(), []
        for h in hits:
            if h[0] not in seen:
                seen.add(h[0]); uniq.append(h)
        return names[0], v, uniq


def fmt(h):
    n, lo, hi = h
    return f'{n}[{hi}]' if lo == hi else f'{n}[{hi}:{lo}]'


if __name__ == '__main__':
    W = sys.argv[1]
    # '--dword' applies to the ADDR MASK pair right before it (per pair, not global)
    pairs, toks, glob_dword = [], sys.argv[2:], False
    i = 0
    while i < len(toks):
        if toks[i] == '--dword':
            if pairs:
                pairs[-1][2] = True
            else:
                glob_dword = True            # legacy: leading --dword = all pairs are 32-bit masks
            i += 1
            continue
        pairs.append([int(toks[i], 0), int(toks[i + 1], 0), False]); i += 2
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    dec = Decoder(W)
    for a, m, dword in pairs:
        dword = dword or glob_dword
        reg, base, hits = dec.decode(a, m, dword)
        if reg is None:
            print(f'0x{a:04X} mask 0x{m:X}: no named register with bit section covers it'); continue
        sh = (a - base) * 8
        print(f"0x{a:04X} mask 0x{m:02X} -> {reg} (base 0x{base:04X}) bits 0x{(m << sh) & 0xFFFFFFFF:08X}: "
              + (', '.join(fmt(h) for h in hits) or '(không có tên bit)'))
