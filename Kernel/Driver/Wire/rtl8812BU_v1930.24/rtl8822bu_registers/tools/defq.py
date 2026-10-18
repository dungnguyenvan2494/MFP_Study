#!/usr/bin/env python3
"""Look up macro definitions that are active in this build (from mask.json).

Usage: py -I defq.py <mask.json> NAME [NAME ...]     exact names
       py -I defq.py <mask.json> -r REGEX              names matching REGEX
Prints: NAME  file:line  (params)  body
'(none active)' means no #define of that name sits in a compiled branch.
"""
import json, re, sys

sys.stdout.reconfigure(encoding='utf-8', errors='replace')
m = json.load(open(sys.argv[1]))
defs = m['defs']
args = sys.argv[2:]
if args and args[0] == '-r':
    rx = re.compile(args[1])
    names = sorted(n for n in defs if rx.search(n))
else:
    names = args
for n in names:
    ds = defs.get(n)
    if not ds:
        print(f'{n}  (none active)')
        continue
    for f, l, p, b in ds:
        ps = f'({",".join(p)})' if p is not None else ''
        print(f'{n}{ps}  {f}:{l}  {b}')
