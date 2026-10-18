#!/usr/bin/env python3
"""Conditional-compilation mask for the rtl8812BU_v1930.24 build.

For every .c file of the module (from mk_eval.py output) this runs a small
C preprocessor that only interprets directives (#define/#undef/#include/#if*/
#elif/#else/#endif). Ordinary code lines are not macro-expanded; they are only
marked "active" when they sit in a taken branch.

Result (JSON):
  active[file]  = sorted list of 1-based line numbers compiled in >=1 TU
  defs[name]    = list of [file, line, params|None, body] for #defines that were
                  executed (active) in >=1 TU
  missing       = includes that could not be resolved (kernel headers etc.)
  tus           = list of .c files processed

Limits (documented in the study notes): kernel headers are NOT parsed, so any
#if that depends on a macro defined only by <linux/...> headers evaluates as
if that macro were undefined (except the ones pre-seeded below).

Usage: py -I cpp_mask.py <driver_dir> <build.json> <kernel .config> <out.json>
"""
import json, os, re, sys

drv, build_json, kcfg, out = sys.argv[1:5]
build = json.load(open(build_json))

# ---------------------------------------------------------------- macro seed
base = {}
def D(name, val='1', params=None):
    base[name] = (params, val)

for f in build['cflags']:
    if f.startswith("'") and f.endswith("'"):
        f = f[1:-1]
    if f.startswith('-D'):
        k, _, v = f[2:].partition('=')
        D(k, v if _ else '1')
incdirs = [f[2:] for f in build['cflags'] if f.startswith('-I')]

for line in open(kcfg, encoding='latin-1'):
    m = re.match(r'(CONFIG_\w+)=(.*)', line.strip())
    if m:
        k, v = m.groups()
        if v == 'y':
            D(k)
        elif v == 'm':
            D(k + '_MODULE')
        else:
            D(k, v)
# Kbuild / compiler built-ins that matter for #if in this driver
D('__KERNEL__'); D('MODULE'); D('__linux__'); D('__GNUC__', '9')
D('__aarch64__'); D('__LP64__')
D('KERNEL_VERSION', '(((a) << 16) + ((b) << 8) + ((c) > 255 ? 255 : (c)))', ['a', 'b', 'c'])
D('LINUX_VERSION_CODE', str((5 << 16) + (10 << 8) + 241))
D('PLATFORM_LINUX')  # set by drv_conf/basic_types via __linux__ path; harmless

# ---------------------------------------------------------------- lexer
TOK = re.compile(r'''\s*(?:
   (?P<num>0[xX][0-9a-fA-F]+[uUlL]*|\d+[uUlL]*)
 | (?P<chr>'(?:\\.|[^'\\])')
 | (?P<str>"(?:\\.|[^"\\])*")
 | (?P<id>[A-Za-z_]\w*)
 | (?P<op>\#\#|<<|>>|<=|>=|==|!=|&&|\|\||[-+*/%<>!~&|^?:(),\#])
)''', re.X)

def lex(s):
    toks, pos = [], 0
    while pos < len(s):
        m = TOK.match(s, pos)
        if not m or m.end() == pos:
            if s[pos:].strip() == '':
                break
            pos += 1  # skip stray char
            continue
        pos = m.end()
        for k in ('num', 'chr', 'str', 'id', 'op'):
            if m.group(k) is not None:
                toks.append((k, m.group(k)))
                break
    return toks

# ---------------------------------------------------------------- expression
class Undef(Exception):
    pass

def expand(toks, macros, hide=frozenset(), depth=0):
    if depth > 60:
        return toks
    out, i = [], 0
    while i < len(toks):
        k, v = toks[i]
        if k == 'id' and v == 'defined':
            # keep defined X / defined(X) unexpanded
            if i + 1 < len(toks) and toks[i+1][1] == '(':
                out.extend(toks[i:i+4]); i += 4
            else:
                out.extend(toks[i:i+2]); i += 2
            continue
        if k == 'id' and v in macros and v not in hide:
            params, body = macros[v]
            if params is None:
                out.extend(expand(lex(body), macros, hide | {v}, depth+1))
                i += 1
                continue
            if i + 1 < len(toks) and toks[i+1][1] == '(':
                lvl, j, args, cur = 1, i + 2, [], []
                while j < len(toks) and lvl:
                    t = toks[j]
                    if t[1] == '(':
                        lvl += 1
                    elif t[1] == ')':
                        lvl -= 1
                        if lvl == 0:
                            break
                    if t[1] == ',' and lvl == 1:
                        args.append(cur); cur = []
                    else:
                        cur.append(t)
                    j += 1
                args.append(cur)
                btoks = lex(body)
                sub = []
                for bt in btoks:
                    if bt[0] == 'id' and bt[1] in params:
                        idx = params.index(bt[1])
                        sub.extend(expand(args[idx], macros, hide, depth+1) if idx < len(args) else [])
                    else:
                        sub.append(bt)
                out.extend(expand(sub, macros, hide | {v}, depth+1))
                i = j + 1
                continue
        out.append(toks[i]); i += 1
    return out

def cint(v):
    v = v.rstrip('uUlL')
    return int(v, 16) if v[:2] in ('0x', '0X') else (int(v, 8) if len(v) > 1 and v[0] == '0' else int(v))

class P:
    def __init__(self, toks, macros):
        self.t, self.i, self.m = toks, 0, macros
    def peek(self):
        return self.t[self.i][1] if self.i < len(self.t) else None
    def nxt(self):
        t = self.t[self.i]; self.i += 1; return t
    def primary(self):
        k, v = self.nxt()
        if v == '(':
            r = self.ternary(); self.nxt(); return r
        if v == '!':
            return int(not self.unary_val())
        if v == '-':
            return -self.unary_val()
        if v == '+':
            return self.unary_val()
        if v == '~':
            return ~self.unary_val()
        if k == 'num':
            return cint(v)
        if k == 'chr':
            c = v[1:-1]
            return ord(c[-1]) if c.startswith('\\') else ord(c)
        if k == 'id' and v == 'defined':
            if self.peek() == '(':
                self.nxt(); name = self.nxt()[1]; self.nxt()
            else:
                name = self.nxt()[1]
            return int(name in self.m)
        if k == 'id':
            if self.peek() == '(':  # unknown function-like macro -> 0
                lvl = 0
                while True:
                    t = self.nxt()[1]
                    lvl += {'(': 1, ')': -1}.get(t, 0)
                    if lvl == 0:
                        break
            return 0
        raise Undef(v)
    unary_val = primary
    BIN = [('||',), ('&&',), ('|',), ('^',), ('&',), ('==', '!='),
           ('<', '>', '<=', '>='), ('<<', '>>'), ('+', '-'), ('*', '/', '%')]
    def binop(self, lvl):
        if lvl == len(self.BIN):
            return self.primary()
        l = self.binop(lvl + 1)
        while self.peek() in self.BIN[lvl]:
            op = self.nxt()[1]
            r = self.binop(lvl + 1)
            l = {'||': lambda: int(bool(l or r)), '&&': lambda: int(bool(l and r)),
                 '|': lambda: l | r, '^': lambda: l ^ r, '&': lambda: l & r,
                 '==': lambda: int(l == r), '!=': lambda: int(l != r),
                 '<': lambda: int(l < r), '>': lambda: int(l > r),
                 '<=': lambda: int(l <= r), '>=': lambda: int(l >= r),
                 '<<': lambda: l << r, '>>': lambda: l >> r,
                 '+': lambda: l + r, '-': lambda: l - r, '*': lambda: l * r,
                 '/': lambda: int(l / r) if r else 0, '%': lambda: l % r if r else 0}[op]()
        return l
    def ternary(self):
        c = self.binop(0)
        if self.peek() == '?':
            self.nxt(); a = self.ternary(); self.nxt(); b = self.ternary()
            return a if c else b
        return c

errors = []
def evaluate(expr, macros, where):
    try:
        toks = expand(lex(expr), macros)
        return bool(P(toks, macros).ternary())
    except Exception as e:  # noqa
        errors.append(f'{where}: cannot eval "{expr}" ({e!r})')
        return False

# ---------------------------------------------------------------- file cache
cache = {}
def logical_lines(path):
    """Return list of (first_line, last_line, text) with comments stripped."""
    if path in cache:
        return cache[path]
    src = open(path, encoding='latin-1').read()
    # strip comments, keep newlines
    res, i, n, in_blk = [], 0, len(src), False
    buf = []
    while i < n:
        c = src[i]
        if in_blk:
            if src.startswith('*/', i):
                in_blk = False; i += 2; buf.append(' ')
            else:
                buf.append('\n' if c == '\n' else ' '); i += 1
            continue
        if src.startswith('/*', i):
            in_blk = True; i += 2; buf.append(' '); continue
        if src.startswith('//', i):
            j = src.find('\n', i)
            j = n if j < 0 else j
            buf.append(' ' * (j - i)); i = j; continue
        if c in '"\'':
            j = i + 1
            while j < n and src[j] != c and src[j] != '\n':
                j += 2 if src[j] == '\\' else 1
            buf.append(src[i:j+1]); i = j + 1; continue
        buf.append(c); i += 1
    text = ''.join(buf).split('\n')
    out, k = [], 0
    while k < len(text):
        first, s = k, text[k]
        while s.endswith('\\') and k + 1 < len(text):
            k += 1; s = s[:-1] + ' ' + text[k]
        out.append((first + 1, k + 1, s))
        k += 1
    # include-guard detection
    dirs = [(a, b, s.strip()) for a, b, s in out if s.strip().startswith('#')]
    guard = None
    if len(dirs) >= 3:
        m1 = re.match(r'#\s*ifndef\s+(\w+)', dirs[0][2])
        m2 = re.match(r'#\s*define\s+(\w+)', dirs[1][2])
        if m1 and m2 and m1.group(1) == m2.group(1) and re.match(r'#\s*endif', dirs[-1][2]):
            # make sure no code after the final #endif
            last = dirs[-1][1]
            if all(not s.strip() for a, b, s in out if a > last):
                guard = m1.group(1)
    cache[path] = (out, guard)
    return cache[path]

active = {}
defs = {}
missing = set()
included_by = {}

def rel(p):
    return os.path.relpath(p, drv).replace('\\', '/')

def resolve(name, cur_dir, quoted):
    cands = ([cur_dir] if quoted else []) + incdirs
    for d in cands:
        p = os.path.normpath(os.path.join(d, name))
        if os.path.isfile(p):
            return p
    return None

def process(path, macros, depth=0):
    lines, guard = logical_lines(path)
    if guard and guard in macros:
        return
    r = rel(path)
    act = active.setdefault(r, set())
    stack = []  # [taking, any_taken, parent_active]
    cur_dir = os.path.dirname(path)
    for first, last, s in lines:
        st = s.strip()
        on = all(x[0] for x in stack)
        if st.startswith('#'):
            d = st[1:].strip()
            m = re.match(r'(\w+)\s*(.*)', d)
            if not m:
                continue
            kw, rest = m.group(1), m.group(2).strip()
            if kw in ('if', 'ifdef', 'ifndef'):
                if not on:
                    stack.append([False, True, False]); continue
                if kw == 'ifdef':
                    c = rest.split()[0] in macros if rest else False
                elif kw == 'ifndef':
                    c = rest.split()[0] not in macros if rest else True
                else:
                    c = evaluate(rest, macros, f'{r}:{first}')
                stack.append([c, c, True])
                act.update(range(first, last + 1))
                continue
            if kw == 'elif':
                top = stack[-1]
                if top[2] and not top[1]:
                    c = evaluate(rest, macros, f'{r}:{first}')
                    top[0] = c; top[1] = c
                    act.update(range(first, last + 1))
                else:
                    top[0] = False
                continue
            if kw == 'else':
                top = stack[-1]
                top[0] = top[2] and not top[1]
                top[1] = True
                if top[2]:
                    act.update(range(first, last + 1))
                continue
            if kw == 'endif':
                top = stack.pop()
                if top[2]:
                    act.update(range(first, last + 1))
                continue
            if not on:
                continue
            act.update(range(first, last + 1))
            if kw == 'define':
                m = re.match(r'(\w+)(\(([^)]*)\))?\s*(.*)', rest)
                if m:
                    name = m.group(1)
                    params = [p.strip() for p in m.group(3).split(',')] if m.group(2) else None
                    body = m.group(4).strip()
                    macros[name] = (params, body)
                    key = (r, first)
                    lst = defs.setdefault(name, {})
                    lst[key] = (params, body)
            elif kw == 'undef':
                macros.pop(rest.split()[0], None) if rest else None
            elif kw == 'include':
                mq = re.match(r'"([^"]+)"', rest)
                ma = re.match(r'<([^>]+)>', rest)
                name, quoted = (mq.group(1), True) if mq else ((ma.group(1), False) if ma else (None, None))
                if name is None:
                    continue
                p = resolve(name, cur_dir, quoted)
                if p is None:
                    missing.add(name); continue
                included_by.setdefault(rel(p), set()).add(r)
                if depth < 40:
                    process(p, macros, depth + 1)
            continue
        if on and st:
            act.update(range(first, last + 1))

tus = [os.path.join(drv, o[:-2] + '.c') for o in build['objs']]
for i, tu in enumerate(tus):
    if not os.path.isfile(tu):
        errors.append(f'missing TU {tu}'); continue
    macros = dict(base)
    process(tu, macros)
    print(f'[{i+1}/{len(tus)}] {rel(tu)}', file=sys.stderr)

json.dump({
    'tus': [rel(t) for t in tus],
    'active': {k: sorted(v) for k, v in active.items()},
    'defs': {n: [[f, l, p, b] for (f, l), (p, b) in d.items()] for n, d in defs.items()},
    'missing': sorted(missing),
    'included_by': {k: sorted(v) for k, v in included_by.items()},
    'errors': errors[:2000],
}, open(out, 'w'))
print(f'files={len(active)} defs={len(defs)} missing={len(missing)} errors={len(errors)}', file=sys.stderr)
