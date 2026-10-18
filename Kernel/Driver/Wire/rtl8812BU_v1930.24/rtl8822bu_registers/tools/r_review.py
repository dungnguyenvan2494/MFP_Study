#!/usr/bin/env python3
"""List every active-code reference of the names of catalog entries with status R
("referenced, but not as the address argument of an accessor") so they can be
classified by hand.

For each R entry: names -> occurrences in active, non-directive, comment-stripped lines
(mask.json 'active'), with the enclosing function (nearest preceding top-level
function header) and the source line.

Usage: py -I r_review.py <driver_dir> <work_dir> [out.md]
"""
import json
import os
import re
import sys

import ast  # noqa: E402
# reuse inventory.strip_cs without running inventory.py (it is a script, not a module)
_inv = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'inventory.py'), encoding='utf-8').read()
_fn = next(n for n in ast.parse(_inv).body if isinstance(n, ast.FunctionDef) and n.name == 'strip_cs')
exec(compile(ast.Module(body=[_fn], type_ignores=[]), 'inventory.py', 'exec'))

drv, W = sys.argv[1], sys.argv[2]
out = open(sys.argv[3], 'w', encoding='utf-8') if len(sys.argv) > 3 else sys.stdout
mask = json.load(open(os.path.join(W, 'mask.json')))
cat = json.load(open(os.path.join(W, 'catalog.json')))
R = [e for e in cat if e['status'] == 'R']
names = {}
for e in R:
    for n in e['names']:
        if n:
            names.setdefault(n, []).append(e)
rx = re.compile(r'\b(' + '|'.join(sorted(map(re.escape, names), key=len, reverse=True)) + r')\b')
FUNC = re.compile(r'^[A-Za-z_][\w\s\*]*?\b([A-Za-z_]\w*)\s*\([^;]*$')
hits = {id(e): [] for e in R}
for f, act in mask['active'].items():
    if f.startswith('hal/halmac/halmac_bit') or f.startswith('hal/halmac/halmac_reg'):
        continue
    try:
        src = strip_cs(open(os.path.join(drv, f), encoding='latin-1').read())
    except OSError:
        continue
    raw = open(os.path.join(drv, f), encoding='latin-1').read().split('\n')
    act = set(act)
    func = None
    for i, ln in enumerate(src.split('\n'), 1):
        if ln and not ln[0].isspace() and ln[0] not in '{}#':
            m = FUNC.match(ln)
            if m:
                func = m.group(1)
        if i not in act or ln.lstrip().startswith('#'):
            continue
        for m in rx.finditer(ln):
            for e in names[m.group(1)]:
                hits[id(e)].append((f, i, func, raw[i - 1].strip()[:150]))
print(f'# R review — {len(R)} entries\n', file=out)
for e in R:
    h = hits[id(e)]
    print(f"## {e['space']} {e['addr_hex']} — {', '.join(e['names'][:3])}  ({len(h)} tham chiếu active)\n", file=out)
    for f, i, fn, s in h[:12]:
        print(f'- `{f}:{i}` `{fn}()` — `{s}`', file=out)
    if len(h) > 12:
        print(f'- … +{len(h) - 12}', file=out)
    print(file=out)
