#!/usr/bin/env python3
"""Register inventory for the rtl8812BU_v1930.24 build (passes 2, 3, 4).

Inputs : driver dir, mask.json (from cpp_mask.py)
Outputs (in <out_dir>):
  regs.json    every register-like macro active in the build, its resolved value,
               address space guess, alias chain, and code uses (direct + via macro)
  bits.json    every BIT_* / field macro of the halmac bit headers, mapped to the
               register section ("/* 2 REG_xxx */" comment) it is declared under
  access.json  every register-access call on an active code line: accessor,
               space, address argument (text + resolved value), function, file:line
  numeric.json subset of access.json whose address argument is a bare literal

Only lines marked active in mask.json are scanned, so dead #if branches and files
outside the build are ignored. Comments and string literals are blanked first.

Usage: py -I inventory.py <driver_dir> <mask.json> <out_dir>
"""
import json, os, re, sys
from collections import defaultdict

drv, mask_path, out_dir = sys.argv[1:4]
mask = json.load(open(mask_path))
active = {f: set(v) for f, v in mask['active'].items()}
DEFS = mask['defs']                      # name -> [[file, line, params, body], ...]

# ------------------------------------------------------------------ helpers
def strip_cs(src):
    """Blank comments and string/char literals, keep newlines and offsets."""
    out, i, n = list(src), 0, len(src)
    while i < n:
        c = src[i]
        if src.startswith('/*', i):
            j = src.find('*/', i + 2); j = n if j < 0 else j + 2
            for k in range(i, j):
                if out[k] != '\n': out[k] = ' '
            i = j; continue
        if src.startswith('//', i):
            j = src.find('\n', i); j = n if j < 0 else j
            for k in range(i, j): out[k] = ' '
            i = j; continue
        if c in '"\'':
            j = i + 1
            while j < n and src[j] != c and src[j] != '\n':
                j += 2 if src[j] == '\\' else 1
            for k in range(i + 1, min(j, n)): out[k] = ' '
            i = j + 1; continue
        i += 1
    return ''.join(out)

ID = re.compile(r'\b[A-Za-z_]\w*\b')
NUM = re.compile(r'^\s*\(?\s*(0[xX][0-9a-fA-F]+|\d+)[uUlL]*\s*\)?\s*$')

ENUMS = {}                               # enumerator -> int (filled below)

def first_def(name):
    ds = DEFS.get(name)
    if ds:
        return ds[0]
    if name in ENUMS:
        return ['<enum>', 0, None, str(ENUMS[name])]
    return None

_cache = {}
ODM_PASTE = re.compile(r'\bODM_(REG|BIT)\(\s*(\w+)\s*,[^()]*\)')
def resolve(expr, depth=0, params=None):
    """Evaluate a macro expression to int, or None. Supports BIT(n) and
    function-like macros with simple substitution."""
    key = (expr, depth)
    if params is None and key in _cache:
        return _cache[key]
    val = None
    if depth < 25:
        # ODM_REG(x, dm)/ODM_BIT(x, dm) paste _11N/_11AC by IC series at run time
        # (phydm_interface.h:163-191); RTL8822B is not in ODM_IC_11N_SERIES -> _11AC
        expr = ODM_PASTE.sub(lambda m: f'ODM_{m.group(1)}_{m.group(2)}_11AC', expr)
        s = expand(expr, depth)
        if s is not None:
            s2 = re.sub(r'\b(0[xX][0-9a-fA-F]+|\d+)[uUlL]+\b', r'\1', s)
            s2 = re.sub(r'\(\s*(u8|u16|u32|u64|s8|s16|s32|int|unsigned int|unsigned long|long)\s*\)', '', s2)
            s2 = s2.replace('&&', ' and ').replace('||', ' or ')
            s2 = re.sub(r'!(?!=)', ' not ', s2)
            if re.fullmatch(r'[0-9a-fA-FxX\s()+\-*/%<>&|^~?:andotr]*', s2) and 'r' not in s2.replace('or', ''):
                try:
                    v = eval(s2.replace('/', '//'), {}, {})
                    if isinstance(v, bool): v = int(v)
                    if isinstance(v, int): val = v & 0xFFFFFFFFFFFFFFFF
                except Exception:
                    val = None
    if params is None:
        _cache[key] = val
    return val

def expand(s, depth):
    """Macro-expand identifiers in s; return None if an unknown identifier stays."""
    out, pos = [], 0
    for m in ID.finditer(s):
        if m.start() < pos:
            continue
        out.append(s[pos:m.start()])
        name = m.group(0)
        pos = m.end()
        if name in ('and', 'or', 'not'):
            out.append(name); continue
        if name in ('BIT', 'BIT_'):
            a, end = call_args(s, pos)
            if a is None or len(a) != 1: return None
            v = resolve(a[0], depth + 1)
            if v is None: return None
            out.append(str(1 << v)); pos = end; continue
        d = first_def(name)
        if d is None:
            return None
        f, l, p, body = d
        if p is None:
            v = resolve(body, depth + 1)
            if v is None: return None
            out.append(str(v))
        else:
            a, end = call_args(s, pos)
            if a is None or len(a) != len(p): return None
            b = body
            for pn, av in zip(p, a):
                b = re.sub(r'\b%s\b' % re.escape(pn), '(' + av + ')', b)
            v = resolve(b, depth + 1, params=True)
            if v is None: return None
            out.append(str(v)); pos = end
    out.append(s[pos:])
    return ''.join(out)

def call_args(s, pos):
    """Parse '(a, b, ...)' starting at s[pos] (skipping spaces). Return (args, end)."""
    i = pos
    while i < len(s) and s[i].isspace(): i += 1
    if i >= len(s) or s[i] != '(':
        return None, pos
    lvl, j, start, args = 0, i, i + 1, []
    while j < len(s):
        c = s[j]
        if c == '(':
            lvl += 1
        elif c == ')':
            lvl -= 1
            if lvl == 0:
                args.append(s[start:j].strip()); return args, j + 1
        elif c == ',' and lvl == 1:
            args.append(s[start:j].strip()); start = j + 1
        j += 1
    return None, pos

# ------------------------------------------------------------------ enums (active lines only)
ENUM_RX = re.compile(r'\benum\b\s*\w*\s*\{([^{}]*)\}', re.S)
_texts = {}
def active_text(f):
    """Comment-stripped text of f with inactive lines blanked (cached)."""
    if f in _texts:
        return _texts[f]
    try:
        raw = open(os.path.join(drv, f), encoding='latin-1').read()
    except OSError:
        _texts[f] = None; return None
    act = active.get(f, set())
    lines = strip_cs(raw).split('\n')
    t = '\n'.join(ln if i in act else '' for i, ln in enumerate(lines, 1))
    _texts[f] = t
    return t

for f in sorted(active):
    t = active_text(f)
    if not t or 'enum' not in t:
        continue
    for m in ENUM_RX.finditer(t):
        nxt = 0
        for item in m.group(1).split(','):
            item = ' '.join(item.split())
            if not item or item.startswith('#'):
                continue
            mm = re.match(r'([A-Za-z_]\w*)\s*(?:=\s*(.+))?$', item)
            if not mm:
                continue
            name, ex = mm.groups()
            if ex is not None:
                v = resolve(ex)
                if v is None:
                    nxt = None; continue
                nxt = v
            elif nxt is None:
                continue
            ENUMS.setdefault(name, nxt)
            nxt += 1
_cache.clear()

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import guard as G
GUARD = G.Guard(DEFS, ENUMS, lambda n: resolve(n))

def _match(text, i, o, c):
    """Index just past the bracket matching text[i] == o."""
    d = 0
    while i < len(text):
        if text[i] == o: d += 1
        elif text[i] == c:
            d -= 1
            if d == 0: return i + 1
        i += 1
    return -1

_er_cache = {}
def early_returns(text, start, end):
    """Conditions C of depth-0 `if (C) <return-branch>` in text[start:end]
    that are not followed by `else`."""
    key = (id(text), start)
    lst = _er_cache.get(key)
    if lst is None:
        lst = []
        i, d, n = start, 0, len(text)
        while i < n:
            ch = text[i]
            if ch == '{': d += 1
            elif ch == '}':
                d -= 1
                if d < 0: break
            elif d == 0 and text.startswith('if', i) and (i == 0 or not (text[i-1].isalnum() or text[i-1] == '_')):
                j = i + 2
                while j < n and text[j].isspace(): j += 1
                if j < n and text[j] == '(':
                    e = _match(text, j, '(', ')')
                    if e < 0: break
                    cond = text[j + 1:e - 1]
                    k = e
                    while k < n and text[k].isspace(): k += 1
                    if k < n and text[k] == '{':
                        be = _match(text, k, '{', '}')
                        body = text[k + 1:be - 1] if be > 0 else ''
                        stmts = [x.strip() for x in body.split(';') if x.strip()]
                        rets = bool(stmts) and re.match(r'(return\b|goto\b)', stmts[-1].split('}')[-1].strip() or 'x')
                        after = be
                    else:
                        se = text.find(';', k)
                        body = text[k:se] if se > 0 else ''
                        rets = re.match(r'\s*(return\b|goto\b)', body)
                        after = se + 1
                    if after <= 0: break
                    m2 = re.match(r'\s*else\b', text[after:after + 20])
                    if rets and not m2:
                        lst.append((after, cond))
                    i = after
                    continue
            i += 1
        _er_cache[key] = lst
    return [c for a, c in lst if a <= end]

def access_guard(text, blocks, off):
    """Verdict for code at offset `off`: 'dead', 'selected', 'unknown', 'none'
    plus the chip-related conditions that decided it."""
    conds = []
    for bi, b in enumerate(blocks):
        if b[0] < off < b[1]:
            conds += G.chain_conds(blocks, bi)
            hm = G.HDR_SWITCH.match(b[2])
            if hm:
                # case labels in this switch before `off`, since last break/return
                seg, d, labels = text[b[0] + 1:off], 0, []
                for mm in re.finditer(r'[{}]|\bcase\b([^:]+):|\bdefault\s*:|\bbreak\s*;|\breturn\b[^;]*;', seg):
                    tok = mm.group(0)
                    if tok == '{': d += 1
                    elif tok == '}': d -= 1
                    elif d == 0:
                        if tok.startswith('case'): labels.append(mm.group(1).strip())
                        elif tok.startswith('default'): labels.append(None)
                        else: labels = []
                if labels and None not in labels:
                    conds.append((' || '.join(f'(({hm.group(1)}) == ({l}))' for l in labels), True))
    # early return: `if (C) return ...;` / `if (C) { ... return ...; }` earlier in an
    # enclosing block means code after it runs only when C is false
    for bi, b in enumerate(blocks):
        if b[0] < off < b[1]:
            for c in early_returns(text, b[0] + 1, off):
                conds.append((c, False))
    # braceless `if (...) stmt;` directly governing the statement at off
    k = max(text.rfind(';', 0, off), text.rfind('{', 0, off), text.rfind('}', 0, off))
    st = text[k + 1:off]
    mm = re.match(r'\s*(?:else\s+)?if\s*\((.*)\)\s*$', st, re.S)
    if mm:
        conds.append((mm.group(1), True))
    verdict, why = 'none', []
    cfg_off = None
    for c, pol in conds:
        r, chip = GUARD.eval(c)
        if not chip:
            rc, rel = GUARD.eval_cfg(c)
            if rel and rc is not None and rc != pol and cfg_off is None:
                cfg_off = ('C', ' '.join(c.split())[:160], pol)
            continue
        cc = ' '.join(c.split())[:160]
        if r is None:
            if verdict == 'none': verdict = 'unknown'
            why.append(('?', cc, pol))
            continue
        if r != pol:
            return 'dead', [('F', cc, pol)]
        why.append(('T', cc, pol))
        if verdict in ('none', 'unknown'): verdict = 'selected'
    if cfg_off:
        return 'cfg-off', why + [cfg_off]
    return verdict, why

# ------------------------------------------------------------------ pass 2: register-like definitions
REG_FILES = {
    'hal/halmac/halmac_reg_8822b.h': 'MAC',
    'hal/halmac/halmac_reg2.h': 'MAC',
    'hal/phydm/phydm_reg.h': None,           # mixed MAC/BB/RF, decide per name/value
    'hal/phydm/phydm_regdefine11ac.h': 'BB',
    'hal/phydm/phydm_regdefine11n.h': 'BB',
    'hal/phydm/phydm_regtable.h': None,       # R_0x* = BB/MAC bus, RF_0x* = RF
    'include/hal_phy_reg.h': None,
    'include/hal_com_reg.h': None,
    'include/rtl8822b_hal.h': None,          # 8822B legacy-style names (rFPGA0_*, rA_LSSIWrite_Jaguar, MSR...)
    'hal/rtl8822b/usb/rtl8822bu.h': None,    # REG_USB_HRPWM_8822B / HCPWM
}

def is_reg_name(name, f):
    if f in ('hal/halmac/halmac_reg_8822b.h', 'hal/halmac/halmac_reg2.h'):
        return name.startswith('REG_')
    if f == 'hal/phydm/phydm_regtable.h':
        return name.startswith(('R_0x', 'RF_0x'))
    if f in ('hal/phydm/phydm_regdefine11ac.h', 'hal/phydm/phydm_regdefine11n.h'):
        return name.startswith(('ODM_REG_', 'odm_'))
    if f == 'hal/phydm/phydm_reg.h':
        return not name.startswith(('__', 'BIT_'))
    if f == 'include/hal_phy_reg.h':
        return name.startswith(('REG_', 'rf_'))
    if f == 'include/hal_com_reg.h':
        return name.startswith('REG_')
    if f == 'include/rtl8822b_hal.h':
        return name.startswith(('REG_', 'MSR', 'RF_0x')) or bool(re.match(r'r[A-Z_]', name))
    if f == 'hal/rtl8822b/usb/rtl8822bu.h':
        return name.startswith('REG_')
    return False

# hal/phydm/phydm_reg.h section comments: /* @MAC REG */ :74, /* @LTE_COEX */ :86,
# /* @BB REG */ :92, /* RF REG */ :143, /* @ant Detect Reg */ :153 ... /* TX AGC */ :168
PHYDM_REG_SECTIONS = [(74, 'MAC'), (86, 'MAC'), (92, 'BUS'), (143, 'RF'), (153, 'BUS')]

def space_of(name, f, val, line=0):
    if f == 'hal/phydm/phydm_reg.h':
        sec = 'BUS'
        for l0, sp in PHYDM_REG_SECTIONS:
            if line >= l0:
                sec = sp
        if sec != 'BUS':
            return sec
    if f == 'hal/halmac/halmac_reg_8822b.h' or f == 'hal/halmac/halmac_reg2.h':
        if val is not None and val >= 0x10000:
            return 'SDIO-local' if (val >> 16) == 0x1025 else 'MAC-other'
        if val is not None and 0xFE00 <= val <= 0xFFFF:
            return 'USB-local'                  # REG_USB_* block (usbctrl_vendorreq REG_LOCAL_SEC)
        return 'MAC'
    if name.startswith(('RF_0x', 'RF_', 'rf_', 'ODM_REG_RF_')) or (
            f == 'hal/phydm/phydm_regdefine11n.h' and '_RF_' in name):
        return 'RF'
    return 'BUS'   # BB/MAC: decided later by address range

def bus_region(v):
    """8822B 16-bit register bus regions, from halmac_reg_8822b.h ranges + phydm use."""
    if v is None: return '?'
    if v < 0x800: return 'MAC'
    if v < 0x1000: return 'BB'
    if v < 0x1800: return 'MAC'
    if v < 0x2000: return 'BB'
    if 0x2800 <= v < 0x3000: return 'RF-direct(BB)'
    if 0xFE00 <= v <= 0xFFFF: return 'USB-local'    # usbctrl_vendorreq: REG_LOCAL_SEC
    return 'other'

regs = {}
for name, ds in DEFS.items():
    for f, l, p, body in ds:
        if p is not None or f not in REG_FILES or not is_reg_name(name, f):
            continue
        val = resolve(body)
        alias = None
        m = re.fullmatch(r'\s*\(?\s*([A-Za-z_]\w*)\s*\)?\s*', body)
        if m and first_def(m.group(1)):
            alias = m.group(1)
        sp = space_of(name, f, val, l)
        if sp == 'BUS':
            sp = bus_region(val)
        regs.setdefault(name, []).append({'file': f, 'line': l, 'body': body,
                                          'value': val, 'alias_of': alias, 'space': sp})

# ------------------------------------------------------------------ bits: section mapping
bits = {}
for bf in ('hal/halmac/halmac_bit_8822b.h', 'hal/halmac/halmac_bit2.h'):
    src = open(os.path.join(drv, bf), encoding='latin-1').read().split('\n')
    sec = None
    for i, line in enumerate(src, 1):
        m = re.match(r'/\*\s*[23]\s+(REG_\w+)\b[^*]*\*/', line)
        if m:
            sec = m.group(1); continue
        m = re.match(r'#define\s+(BIT\w*?)(\(|\s)', line)
        if m and i in active.get(bf, ()):
            bits[m.group(1)] = {'file': bf, 'line': i, 'reg': sec}

# ------------------------------------------------------------------ pass 3+4: scan active code
ACC = {
    # name: (space, index of address arg, width, kind)
    'rtw_read8': ('BUS', 1, 8, 'R'), 'rtw_read16': ('BUS', 1, 16, 'R'), 'rtw_read32': ('BUS', 1, 32, 'R'),
    'rtw_write8': ('BUS', 1, 8, 'W'), 'rtw_write16': ('BUS', 1, 16, 'W'), 'rtw_write32': ('BUS', 1, 32, 'W'),
    'rtw_writeN': ('BUS', 1, 0, 'W'), 'rtw_write_mem': ('BUS', 1, 0, 'W'), 'rtw_read_mem': ('BUS', 1, 0, 'R'),
    'rtw_write8_async': ('BUS', 1, 8, 'W'), 'rtw_write16_async': ('BUS', 1, 16, 'W'), 'rtw_write32_async': ('BUS', 1, 32, 'W'),
    'PlatformEFIOWrite1Byte': ('BUS', 1, 8, 'W'), 'PlatformEFIOWrite2Byte': ('BUS', 1, 16, 'W'),
    'PlatformEFIOWrite4Byte': ('BUS', 1, 32, 'W'), 'PlatformEFIORead4Byte': ('BUS', 1, 32, 'R'),
    'HALMAC_REG_R8': ('BUS', 0, 8, 'R'), 'HALMAC_REG_R16': ('BUS', 0, 16, 'R'), 'HALMAC_REG_R32': ('BUS', 0, 32, 'R'),
    'HALMAC_REG_W8': ('BUS', 0, 8, 'W'), 'HALMAC_REG_W16': ('BUS', 0, 16, 'W'), 'HALMAC_REG_W32': ('BUS', 0, 32, 'W'),
    'HALMAC_REG_W8_SET': ('BUS', 0, 8, 'RMW-set'), 'HALMAC_REG_W8_CLR': ('BUS', 0, 8, 'RMW-clr'),
    'HALMAC_REG_W16_SET': ('BUS', 0, 16, 'RMW-set'), 'HALMAC_REG_W16_CLR': ('BUS', 0, 16, 'RMW-clr'),
    'HALMAC_REG_W32_SET': ('BUS', 0, 32, 'RMW-set'), 'HALMAC_REG_W32_CLR': ('BUS', 0, 32, 'RMW-clr'),
    # halmac_api.c calls the platform API directly before `api` exists (e.g. chip id read)
    'REG_READ_8': ('BUS', 1, 8, 'R'), 'REG_READ_16': ('BUS', 1, 16, 'R'), 'REG_READ_32': ('BUS', 1, 32, 'R'),
    'REG_WRITE_8': ('BUS', 1, 8, 'W'), 'REG_WRITE_16': ('BUS', 1, 16, 'W'), 'REG_WRITE_32': ('BUS', 1, 32, 'W'),
    'PLTFM_REG_R8': ('BUS', 0, 8, 'R'), 'PLTFM_REG_R16': ('BUS', 0, 16, 'R'), 'PLTFM_REG_R32': ('BUS', 0, 32, 'R'),
    'PLTFM_REG_W8': ('BUS', 0, 8, 'W'), 'PLTFM_REG_W16': ('BUS', 0, 16, 'W'), 'PLTFM_REG_W32': ('BUS', 0, 32, 'W'),
    'odm_read_1byte': ('BUS', 1, 8, 'R'), 'odm_read_2byte': ('BUS', 1, 16, 'R'), 'odm_read_4byte': ('BUS', 1, 32, 'R'),
    'odm_write_1byte': ('BUS', 1, 8, 'W'), 'odm_write_2byte': ('BUS', 1, 16, 'W'), 'odm_write_4byte': ('BUS', 1, 32, 'W'),
    'odm_set_mac_reg': ('BUS', 1, 32, 'RMW-mask'), 'odm_get_mac_reg': ('BUS', 1, 32, 'R-mask'),
    'phy_query_mac_reg': ('BUS', 1, 32, 'R-mask'), 'phy_set_mac_reg': ('BUS', 1, 32, 'RMW-mask'),
    'odm_set_bb_reg': ('BUS', 1, 32, 'RMW-mask'), 'odm_get_bb_reg': ('BUS', 1, 32, 'R-mask'),
    'phy_set_bb_reg': ('BUS', 1, 32, 'RMW-mask'), 'phy_query_bb_reg': ('BUS', 1, 32, 'R-mask'),
    'rtw_hal_write_bbreg': ('BUS', 1, 32, 'RMW-mask'), 'rtw_hal_read_bbreg': ('BUS', 1, 32, 'R-mask'),
    'rtl8822b_write_bb_reg': ('BUS', 1, 32, 'RMW-mask'), 'rtl8822b_read_bb_reg': ('BUS', 1, 32, 'R-mask'),
    'odm_set_rf_reg': ('RF', 2, 20, 'RMW-mask'), 'odm_get_rf_reg': ('RF', 2, 20, 'R-mask'),
    'phy_set_rf_reg': ('RF', 2, 20, 'RMW-mask'), 'phy_query_rf_reg': ('RF', 2, 20, 'R-mask'),
    'rtw_hal_write_rfreg': ('RF', 2, 20, 'RMW-mask'), 'rtw_hal_read_rfreg': ('RF', 2, 20, 'R-mask'),
    'config_phydm_read_rf_reg_8822b': ('RF', 2, 20, 'R-mask'),
    'config_phydm_write_rf_reg_8822b': ('RF', 2, 20, 'RMW-mask'),
    'config_phydm_direct_write_rf_reg_8822b': ('RF', 2, 20, 'W'),
    # second-level wrappers (thin pass-through functions/macros found by a wrapper scan)
    'PlatformEFIORead1Byte': ('BUS', 1, 8, 'R'), 'PlatformEFIORead2Byte': ('BUS', 1, 16, 'R'),
    'pdm_set_reg': ('BUS', 1, 32, 'RMW-mask'),
    'read_macreg': ('BUS', 1, 0, 'R'), 'write_macreg': ('BUS', 1, 0, 'W'),
    'read_bbreg': ('BUS', 1, 32, 'R-mask'), 'write_bbreg': ('BUS', 1, 32, 'RMW-mask'),
    '_read_rfreg': ('RF', 2, 20, 'R-mask'), '_write_rfreg': ('RF', 2, 20, 'RMW-mask'),
    'read_rfreg': ('RF', 2, 20, 'R'), 'write_rfreg': ('RF', 2, 20, 'W'),
    'halwifionly_write1byte': ('BUS', 1, 8, 'W'), 'halwifionly_write2byte': ('BUS', 1, 16, 'W'),
    'halwifionly_write4byte': ('BUS', 1, 32, 'W'), 'halwifionly_read1byte': ('BUS', 1, 8, 'R'),
    'halwifionly_read2byte': ('BUS', 1, 16, 'R'), 'halwifionly_read4byte': ('BUS', 1, 32, 'R'),
    'halwifionly_bitmaskwrite1byte': ('BUS', 1, 8, 'RMW-mask'),
    'halwifionly_phy_set_rf_reg': ('RF', 2, 20, 'RMW-mask'),
    'halwifionly_phy_set_bb_reg': ('BUS', 1, 32, 'RMW-mask'),
}
ACC_RX = re.compile(r'\b(' + '|'.join(sorted(ACC, key=len, reverse=True)) + r')\s*\(')
FUNC_RX = re.compile(r'([A-Za-z_]\w*)\s*\([^;{}]*\)\s*(?:__\w+\s*)*$', re.S)

code_refs = defaultdict(list)      # identifier -> [(file, line, func)]
access = []
func_defs = {}          # function name -> [(file, line)]
ref_sites = []          # (file, offset, line, enclosing_func, name)
FILEDATA = {}           # file -> (text, blocks, lineno)

for f in sorted(active):
    if f.endswith('.h') and f.startswith('hal/halmac/halmac_bit'):
        continue                     # pure definition headers: skip use-scan
    path = os.path.join(drv, f)
    try:
        raw = open(path, encoding='latin-1').read()
    except OSError:
        continue
    src = strip_cs(raw)
    lines = src.split('\n')
    act = active[f]
    # join backslash continuations of active #define lines are ignored (macro bodies);
    # keep only non-directive active code
    code = []
    in_dir = False
    for i, ln in enumerate(lines, 1):
        is_dir = in_dir or ln.lstrip().startswith('#')
        in_dir = is_dir and ln.rstrip().endswith('\\')
        code.append(ln if (i in act and not is_dir) else '')
    text = '\n'.join(code)
    # line start offsets
    starts = [0]
    for ln in code:
        starts.append(starts[-1] + len(ln) + 1)
    import bisect
    def lineno(off, starts=starts):
        return bisect.bisect_right(starts, off)
    # function ranges via brace depth
    func_at = [None] * (len(code) + 2)
    depth, cur, buf = 0, None, ''
    for i, ln in enumerate(code, 1):
        for ch in ln:
            if ch == '{':
                if depth == 0:
                    m = FUNC_RX.search(buf)
                    cur = m.group(1) if m and m.group(1) not in ('if', 'while', 'for', 'switch') else '<top>'
                    if cur != '<top>':
                        func_defs.setdefault(cur, []).append((f, i))
                depth += 1
            elif ch == '}':
                depth = max(0, depth - 1)
                if depth == 0:
                    buf = ''
            if depth == 0 and ch not in '{}':
                buf = (buf + ch)[-400:]
            if depth == 0 and ch == ';':
                buf = ''
        if depth == 0:
            buf += ' '
        func_at[i] = cur if depth > 0 else None
    # identifier refs
    for i, ln in enumerate(code, 1):
        if not ln:
            continue
        for m in ID.finditer(ln):
            code_refs[m.group(0)].append((f, i, func_at[i]))
        for m in ODM_PASTE.finditer(ln):
            code_refs[f'ODM_{m.group(1)}_{m.group(2)}_11AC'].append((f, i, func_at[i]))
    # accessor calls
    blocks = G.blocks_of(text)
    FILEDATA[f] = (text, blocks, lineno)
    for mm in ID.finditer(text):
        ln_ = lineno(mm.start())
        ref_sites.append((f, mm.start(), ln_, func_at[ln_], mm.group(0)))
    for m in ACC_RX.finditer(text):
        name = m.group(1)
        if m.start() > 0 and (text[m.start() - 1].isalnum() or text[m.start() - 1] == '_'):
            continue
        args, end = call_args(text, m.end() - 1)
        if args is None:
            continue
        sp, ai, width, kind = ACC[name]
        if ai >= len(args):
            continue
        ln = lineno(m.start())
        fn = func_at[ln]
        if fn is None:
            continue                                # e.g. inside a macro body / prototype
        a = ' '.join(args[ai].split())
        v = resolve(a)
        rec = {'file': f, 'line': ln, 'func': fn, 'acc': name, 'kind': kind, 'width': width,
               'space': sp, 'addr': a, 'value': v}
        if sp == 'RF':
            rec['path'] = ' '.join(args[1].split()) if len(args) > 1 else '?'
            if v is not None:
                rec['value'] = v & 0xFF        # 8822B masks RF addr to 8 bits
        else:
            rec['region'] = bus_region(v)
        if len(args) > ai + 1 and 'mask' in kind:
            rec['mask'] = ' '.join(args[ai + 1].split())
        rec['numeric'] = bool(NUM.match(a)) or bool(re.fullmatch(r'\(?\s*(R|RF)_0x[0-9a-fA-F]+\s*\)?', a))
        rec['guard'], rec['guard_why'] = access_guard(text, blocks, m.start())
        access.append(rec)

# ------------------------------------------------------------------ addresses held in local arrays
# e.g. u32 backup_bb_reg[N] = {0x808, 0x90c, ...};  ...  odm_get_bb_reg(dm, backup_bb_reg[i], ...)
extra = []
for a in access:
    if a['value'] is not None:
        continue
    mm = re.fullmatch(r'\(?\s*([A-Za-z_]\w*)\s*\[[^\]]+\]\s*\)?', a['addr'])
    if not mm:
        continue
    arr = mm.group(1)
    text_, blocks_, lineno_ = FILEDATA[a['file']]
    # search the enclosing function (or file scope) for the initializer
    im = None
    for im_ in re.finditer(r'\b%s\s*\[[^\]]*\]\s*(?:\[[^\]]*\]\s*)?=\s*\{([^;]*?)\}\s*;' % re.escape(arr), text_):
        im = im_
    if not im:
        continue
    items = [x.strip() for x in re.split(r'[,{}]', im.group(1)) if x.strip()]
    vals = [resolve(x) for x in items]
    if not vals or any(v is None for v in vals):
        continue
    a['array_values'] = vals
    for v in vals:
        r = dict(a); r.pop('array_values', None)
        r['value'] = v & 0xFF if a['space'] == 'RF' else v
        if a['space'] != 'RF':
            r['region'] = bus_region(v)
        r['via_array'] = f"{arr} ({a['file']}:{lineno_(im.start())})"
        extra.append(r)
access += extra

# ------------------------------------------------------------------ call graph reachability (for RTL8822BU)
macro_body_ids = set()
for n, ds in DEFS.items():
    for f_, l_, p_, b_ in ds:
        macro_body_ids.update(ID.findall(b_))
# members invoked through a function pointer anywhere: `->m(` / `.m(` in code or macro bodies
# any member access that is not the target of an assignment (call, or read into a variable
# such as `_read8 = pintfhdl->io_ops._read8;` in core/rtw_io.c:114)
CALL_MEMBER = re.compile(r'(?:->|\.)\s*([A-Za-z_]\w*)\b(?!\s*=[^=])')
called_members = set()
for f_, (t_, _b, _l) in FILEDATA.items():
    called_members.update(CALL_MEMBER.findall(t_))
for n, ds in DEFS.items():
    for f_, l_, p_, b_ in ds:
        called_members.update(CALL_MEMBER.findall(b_))
# only driver-internal dispatch tables (halmac api, hal ops, io ops): their members are
# called from driver code; kernel callback structs (.probe, ndo_*, cfg80211 ops) are not
PTR_ASSIGN = re.compile(r'(?:\bapi|\bops|\bpops|pltfm_api|hal_func)\s*(?:->|\.)\s*([A-Za-z_]\w*)\s*=\s*&?\s*$')

uses_of = defaultdict(list)      # func -> [(caller or None, guard, file, line)]
for f, off, ln_, caller, name in ref_sites:
    if name not in func_defs:
        continue
    if caller == name and any(df == f and dl == ln_ for df, dl in func_defs[name]):
        continue
    text_, blocks_, _ = FILEDATA[f]
    after = text_[off + len(name):off + len(name) + 40].lstrip()
    # file scope + followed by '(' = definition header or prototype, not a use
    if caller is None and after.startswith('('):
        continue
    g = access_guard(text_, blocks_, off)[0] if caller is not None else 'none'
    # `x->member = func` / `.member = func`: only a use if member is ever called
    pm = PTR_ASSIGN.search(text_[max(0, off - 80):off])
    if pm and pm.group(1) not in called_members and not after.startswith('('):
        g = 'uncalled-ptr'
    uses_of[name].append((caller, g, f, ln_))

def unreachable(bad_guards):
    """Functions whose every use site is under a guard in bad_guards or inside
    another function of the returned set (fixed point)."""
    out, changed = set(), True
    while changed:
        changed = False
        for fn in func_defs:
            if fn in out:
                continue
            us = uses_of.get(fn, [])
            if not us or fn in macro_body_ids:
                continue
            if all(g in bad_guards or (c is not None and c in out and c != fn) for c, g, _, _ in us):
                out.add(fn); changed = True
    return out

dead_funcs = unreachable(('dead',))                      # never for RTL8822BU
cfgoff_funcs = unreachable(('dead', 'cfg-off')) - dead_funcs   # off in default config
no_caller = {fn for fn in func_defs if not uses_of.get(fn) and fn not in macro_body_ids}
# only stored in a function-pointer member that nobody calls (e.g. unused halmac api slots)
no_caller |= unreachable(('dead', 'cfg-off', 'uncalled-ptr')) - dead_funcs - cfgoff_funcs
for a in access:
    if a['func'] in dead_funcs:
        a['reach'] = 'dead-func'
    elif a['func'] in cfgoff_funcs:
        a['reach'] = 'cfg-off-func'
    elif a['func'] in no_caller:
        a['reach'] = 'no-caller'
    else:
        a['reach'] = 'reachable?'
    a['live'] = a['guard'] not in ('dead', 'cfg-off') and a['reach'] == 'reachable?'
json.dump({'dead_funcs': sorted(dead_funcs), 'cfgoff_funcs': sorted(cfgoff_funcs), 'no_caller': sorted(no_caller),
           'uses_of': {k: v[:30] for k, v in uses_of.items()}},
          open(os.path.join(out_dir, 'callgraph.json'), 'w'), indent=0)

# ------------------------------------------------------------------ macro-reference graph (alias / wrapper use)
body_refs = defaultdict(set)   # macro -> names referenced in its body
for n, ds in DEFS.items():
    for f, l, p, b in ds:
        for m in ID.finditer(b):
            if m.group(0) != n:
                body_refs[n].add(m.group(0))
users_of = defaultdict(set)
for n, rs in body_refs.items():
    for r in rs:
        users_of[r].add(n)

used_cache = {}
def used_via(name, seen=None):
    """Return list of (via_macro, file, line, func) code uses reaching `name`."""
    if name in used_cache:
        return used_cache[name]
    seen = seen or set()
    if name in seen:
        return []
    seen.add(name)
    res = [(None, f, l, fn) for f, l, fn in code_refs.get(name, [])]
    for u in users_of.get(name, ()):
        if len(res) > 400:
            break
        for via, f, l, fn in used_via(u, seen):
            res.append((via or u, f, l, fn))
    used_cache[name] = res
    return res

for name, recs in regs.items():
    uses = used_via(name)
    direct = [u for u in uses if u[0] is None]
    for r in recs:
        r['uses_direct'] = len(direct)
        r['uses_total'] = len(uses)
        r['where'] = [f'{f}:{l}' + (f' {fn}()' if fn else '') + (f' via {v}' if v else '')
                      for v, f, l, fn in uses[:40]]

for name, b in bits.items():
    uses = used_via(name)
    b['uses_total'] = len(uses)
    b['where'] = [f'{f}:{l}' + (f' {fn}()' if fn else '') + (f' via {v}' if v else '')
                  for v, f, l, fn in uses[:20]]

os.makedirs(out_dir, exist_ok=True)
json.dump(regs, open(os.path.join(out_dir, 'regs.json'), 'w'), indent=0)
json.dump(bits, open(os.path.join(out_dir, 'bits.json'), 'w'), indent=0)
json.dump(access, open(os.path.join(out_dir, 'access.json'), 'w'), indent=0)
json.dump([a for a in access if a['numeric']], open(os.path.join(out_dir, 'numeric.json'), 'w'), indent=0)
print(f'regs={len(regs)} bits={len(bits)} access={len(access)} '
      f'numeric={sum(a["numeric"] for a in access)} unresolved_addr={sum(a["value"] is None for a in access)}',
      file=sys.stderr)
