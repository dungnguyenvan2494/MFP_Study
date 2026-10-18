#!/usr/bin/env python3
"""Print source lines with an active/inactive marker from mask.json.

Usage: py -I show.py <driver_dir> <mask.json> <file>:<from>-<to> [...]
  '+' line compiled in >=1 TU of this build
  '-' code line inside an inactive #if branch (or file not in build)
  ' ' blank / comment-only line
"""
import json, os, re, sys

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
drv, mask = sys.argv[1], json.load(open(sys.argv[2]))
act = {k: set(v) for k, v in mask['active'].items()}

def code_lines(src):
    """Return per-line flag: True if line has non-comment, non-blank text."""
    out, cur, i, n, blk = [], False, 0, len(src), False
    while i < n:
        c = src[i]
        if c == '\n':
            out.append(cur); cur = False; i += 1; continue
        if blk:
            if src.startswith('*/', i):
                blk = False; i += 2
            else:
                i += 1
            continue
        if src.startswith('/*', i):
            blk = True; i += 2; continue
        if src.startswith('//', i):
            j = src.find('\n', i); i = n if j < 0 else j; continue
        if not c.isspace():
            cur = True
        i += 1
    out.append(cur)
    return out

for spec in sys.argv[3:]:
    f, _, rng = spec.rpartition(':')
    a, _, b = rng.partition('-')
    a, b = int(a), int(b or a)
    src = open(os.path.join(drv, f), encoding='latin-1').read()
    lines, code = src.split('\n'), code_lines(src)
    s = act.get(f, set())
    print(f'==== {f}:{a}-{b}  (file {"compiled" if f in act else "NOT in build"})')
    for i in range(a, min(b, len(lines)) + 1):
        mark = '+' if i in s else ('-' if code[i-1] else ' ')
        print(f'{mark}{i:6d}| {lines[i-1]}')
