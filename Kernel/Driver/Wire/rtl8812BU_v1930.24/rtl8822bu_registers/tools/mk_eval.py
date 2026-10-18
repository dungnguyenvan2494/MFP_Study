#!/usr/bin/env python3
"""Minimal GNU-make evaluator for the rtl8812BU_v1930.24 Makefile.

Only supports what that Makefile uses: =, :=, ?=, +=, ifeq/ifneq/ifdef/ifndef,
else, endif, include, $(VAR) expansion. $(shell ...) / $(wildcard ...) expand
to "". Recipes (lines starting with TAB) are ignored.

Usage: py -I mk_eval.py <driver_dir>  -> prints JSON {cflags:[...], objs:[...]}
KERNELRELEASE is treated as non-empty (Kbuild pass), as in a real module build.
"""
import json, os, re, sys

drv = sys.argv[1]
V = {"src": drv, "KERNELRELEASE": "5.10.241", "PWD": drv}

def expand(s, depth=0):
    if depth > 20:
        return s
    out, i = [], 0
    while i < len(s):
        if s[i] == '$' and i + 1 < len(s) and s[i+1] == '(':
            lvl, j = 1, i + 2
            while j < len(s) and lvl:
                lvl += {'(': 1, ')': -1}.get(s[j], 0)
                j += 1
            inner = s[i+2:j-1]
            if inner.startswith(('shell ', 'wildcard ', 'subst ', 'patsubst ', 'strip ', 'filter ')):
                out.append('')
            else:
                out.append(expand(V.get(expand(inner, depth+1), ''), depth+1))
            i = j
        elif s[i] == '$' and i + 1 < len(s) and s[i+1] == '$':
            out.append('$'); i += 2
        else:
            out.append(s[i]); i += 1
    return ''.join(out)

def cond(line):
    m = re.match(r'(ifeq|ifneq)\s*\((.*)\)\s*$', line)
    if m:
        a, b = [expand(x).strip() for x in split_args(m.group(2))]
        return (a == b) if m.group(1) == 'ifeq' else (a != b)
    m = re.match(r'(ifdef|ifndef)\s+(\S+)', line)
    if m:
        d = bool(V.get(expand(m.group(2)), ''))
        return d if m.group(1) == 'ifdef' else not d
    raise ValueError(line)

def split_args(s):
    lvl = 0
    for i, c in enumerate(s):
        if c == '(':
            lvl += 1
        elif c == ')':
            lvl -= 1
        elif c == ',' and lvl == 0:
            return s[:i], s[i+1:]
    return s, ''

def run(path):
    lines = open(path, encoding='latin-1').read().replace('\\\n', ' ').split('\n')
    stack = []  # each: [active, taken]
    for raw in lines:
        if raw.startswith('\t'):
            continue
        line = raw.split('#', 1)[0].rstrip() if not raw.lstrip().startswith('#') else ''
        s = line.strip()
        if not s:
            continue
        active = all(a for a, _ in stack)
        if re.match(r'(ifeq|ifneq|ifdef|ifndef)\b', s):
            c = cond(s) if active else False
            stack.append([c, c]); continue
        if s.startswith('else'):
            rest = s[4:].strip()
            parent = all(a for a, _ in stack[:-1])
            if rest:
                c = (not stack[-1][1]) and parent and cond(rest)
            else:
                c = (not stack[-1][1]) and parent
            stack[-1][0] = c
            stack[-1][1] = stack[-1][1] or c
            continue
        if s.startswith('endif'):
            stack.pop(); continue
        if not active:
            continue
        if s.startswith('include ') or s.startswith('-include '):
            p = expand(s.split(None, 1)[1]).strip()
            if os.path.exists(p):
                run(p)
            continue
        if s.startswith('export '):
            s = s[7:]
        m = re.match(r'([A-Za-z0-9_.$()\-]+)\s*(\+=|:=|\?=|=)\s*(.*)$', s)
        if m:
            name, op, val = expand(m.group(1)), m.group(2), m.group(3)
            if op == '+=':
                V[name] = (V.get(name, '') + ' ' + expand(val)).strip()
            elif op == ':=':
                V[name] = expand(val)
            elif op == '?=':
                V.setdefault(name, expand(val))
            else:
                V[name] = expand(val)  # recursive vars simplified to immediate

run(os.path.join(drv, 'Makefile'))
mod = V.get('MODULE_NAME', '')
objs = V.get(mod + '-y', '').split()
print(json.dumps({'module': mod, 'cflags': V.get('EXTRA_CFLAGS', '').split(),
                  'objs': objs}, indent=1))
