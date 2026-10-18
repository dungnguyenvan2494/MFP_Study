#!/usr/bin/env python3
"""Build the register catalog: one row per (address space, address).

Inputs : work/{regs,bits,access,tables,mask}.json, driver dir
Outputs: work/catalog.json, work/catalog.csv, and Markdown catalog pages
         <docs>/04a_catalog_mac.md, 04b_catalog_bb.md, 04c_catalog_rf.md,
         04d_catalog_other.md

Status codes (one per row, first that applies):
  A  accessed on a live path of the RTL8822BU build (code or unconditional init table)
  T  only in run-time-conditional tables (RFE-type branch of PHY/RF table, GPIO pinmux)
  M  accessed only from MP (manufacturing/test) code
  G  accessed only from debug code (proc / dbg ioctl / dump helpers)
  X  accessed only on paths that are dead for RTL8822BU (other chip/interface,
     default-config-off, or functions with no caller)
  R  name referenced in active code, but not through a recognised accessor
     (e.g. stored in a struct/array, compared, used as a value)
  Z  name only appears in inactive #if branches of build files
  D  defined (active #define) but never referenced anywhere in the build
Usage: py -I report.py <driver_dir> <work_dir> <docs_dir>
"""
import csv, json, os, re, sys
from collections import defaultdict, Counter

drv, W, DOCS = sys.argv[1:4]
L = lambda n: json.load(open(os.path.join(W, n)))
regs, bits, access, tables, mask = L('regs.json'), L('bits.json'), L('access.json'), L('tables.json'), L('mask.json')
active = {f: set(v) for f, v in mask['active'].items()}

MP_FILES = ('core/rtw_mp.c', 'hal/hal_mp.c', 'os_dep/linux/ioctl_mp.c', 'hal/phydm/phydm_mp.c')
DBG_FILES = ('core/rtw_debug.c', 'os_dep/linux/rtw_proc.c', 'hal/phydm/phydm_debug.c',
             'hal/phydm/halrf/halrf_debug.c', 'hal/phydm/phydm_auto_dbg.c')
DBG_FUNCS = re.compile(r'dump|_dbg|dbg_|debug|proc_|rtw_wx_read|rtw_wx_write|rtw_dbg_port|mac_reg_dump|bb_reg_dump|rf_reg_dump')

def origin(a):
    if a['file'] in MP_FILES or re.search(r'(^|_)mpt?_|_mp$|^mp', a['func']):
        return 'M'
    if a['file'] in DBG_FILES or DBG_FUNCS.search(a['func']):
        return 'G'
    return 'A'

# ---------------------------------------------------------------- rows
rows = {}
def row(space, addr):
    k = (space, addr)
    if k not in rows:
        rows[k] = {'space': space, 'addr': addr, 'names': [], 'acc': [], 'tab': [], 'bits': []}
    return rows[k]

def norm_space(sp):
    return {'RF-direct(BB)': 'RF-direct', '?': None}.get(sp, sp)

# where each macro name is actually used as an address argument (space of the accessor)
use_space = defaultdict(set)
for a in access:
    m = re.fullmatch(r'\(?\s*([A-Za-z_]\w*)\s*\)?', a['addr'])
    if m and a['value'] is not None:
        use_space[m.group(1)].add('RF' if a['space'] == 'RF' else norm_space(a.get('region', '?')))

# macros
for name, rs in regs.items():
    for r in rs:
        v, sp = r['value'], norm_space(r['space'])
        if v is None or sp is None:
            continue
        us = use_space.get(name, set()) - {None}
        if us and sp not in us and len(us) == 1:
            sp = next(iter(us))        # e.g. RF_T_METER_NEW sits in phydm_reg.h "MAC REG" section but is an RF address
        if sp == 'RF':
            v &= 0xFF
        row(sp, v)['names'].append({'name': name, 'file': r['file'], 'line': r['line'],
                                    'alias_of': r['alias_of'], 'uses': r['uses_total'],
                                    'uses_direct': r['uses_direct'], 'where': r['where'][:8]})

# code accesses
for a in access:
    if a['value'] is None:
        continue
    sp = 'RF' if a['space'] == 'RF' else norm_space(a.get('region', '?'))
    if sp is None:
        continue
    st = origin(a) if a['live'] else 'X'
    row(sp, a['value'])['acc'].append(dict(a, st=st))

# tables
TAB_SPACE = {'write8': 'MAC', 'write32': 'BB', 'write20': 'RF', 'pg-store': 'BB-pg', 'pwrseq': None,
             'intf-phy': None, 'pinmux': 'MAC'}
for t in tables:
    k = t['kind']
    # pwr-seq DELAY steps carry the delay amount in the "offset" field — not an address
    if k in ('delay', 'end') or (k == 'pwrseq' and t['cmd'] in ('HALMAC_PWR_CMD_END', 'HALMAC_PWR_CMD_DELAY')):
        continue
    if k == 'pwrseq':
        sp = t['space'] if t['space'] != 'MAC' else ('USB-local' if t['addr'] >= 0xFE00 else 'MAC')
        live = t['usb'] is True or t['intf'] == 'HALMAC_PWR_INTF_ALL_MSK'
    elif k == 'intf-phy':
        sp, live = t['space'], True
    elif k == 'write32':
        sp, live = ('BB' if t['addr'] < 0x2000 or t['addr'] >= 0x1800 else 'MAC'), True
        if 0x1000 <= t['addr'] < 0x1800 or t['addr'] < 0x800:
            sp = 'MAC'
    else:
        sp, live = TAB_SPACE[k], True
    row(sp, t['addr'])['tab'].append(dict(t, live=live))

# bit/field macros -> register section -> address
addr_of_name = {}
for (sp, ad), r in rows.items():
    for n in r['names']:
        addr_of_name.setdefault(n['name'], (sp, ad))
for bname, b in bits.items():
    if b['reg'] and b['reg'] in addr_of_name:
        rows[addr_of_name[b['reg']]]['bits'].append((bname, b['uses_total']))

# names referenced only in inactive code / comments: scan raw text of build files
def strip_comments(t):
    """Blank /* */ and // comments, keep newlines."""
    t = re.sub(r'/\*.*?\*/', lambda m: re.sub(r'[^\n]', ' ', m.group(0)), t, flags=re.S)
    return re.sub(r'//[^\n]*', '', t)

inactive_ids = Counter()
ID = re.compile(r'\b[A-Za-z_]\w*\b')
for f in active:
    try:
        lines = strip_comments(open(os.path.join(drv, f), encoding='latin-1').read()).split('\n')
    except OSError:
        continue
    act = active[f]
    for i, ln in enumerate(lines, 1):
        if ln.lstrip().startswith('#'):
            continue                 # definitions of other chips are not "uses"
        if i not in act and ('REG_' in ln or 'R_0x' in ln or 'RF_0x' in ln or 'ODM_' in ln or 'rRF' in ln):
            for m in ID.finditer(ln):
                inactive_ids[m.group(0)] += 1

# ---------------------------------------------------------------- per-row summary
GROUP_RX = [
    ('Power/PS', r'pwr|power_|lps|_ps_|ps_|suspend|resume|wow|leave_ips|enter_ips|card_|pwrseq|sleep|rpwm|cpwm'),
    ('Beacon/TSF', r'bcn|beacon|tsf|tbtt|atim|dtim|port_cfg|hw_port|bssid'),
    ('Firmware/H2C/C2H', r'fw|h2c|c2h|dlfw|mailbox|msgbox|iddma|rsvd_page|txbf_fw'),
    ('Interrupt', r'int(r|errupt)?_|_isr|isr_|imr|hisr|interrupt'),
    ('Security/CAM', r'cam|_sec|sec_|key|crypto|tkip'),
    ('eFuse', r'efuse|otp|eeprom'),
    ('GPIO/LED', r'gpio|led|pinmux|wl_act|bt_act'),
    ('TX/RX DMA/Queue', r'txdma|rxdma|rqpn|queue|llt|txff|rxff|fifo|trx|pg_num|txpkt|rx_drv|agg|bulk|dma'),
    ('USB', r'usb'),
    ('Calibration', r'iqk|lck|dpk|txgapk|pwrtrack|pwr_track|kfree|thermal|imr_|_cali|cali_|txgap|rfk'),
    ('BB DM (DIG/CCA/FA/CCX)', r'dig|cca|false_alarm|fa_|ccx|nhm|clm|fahm|adaptivity|edcca|psd|noise|cfo|lna_sat|rssi|igi'),
    ('Antenna/RFE', r'rfe|ant|path_div|pathdiv|tx_path|rx_path|trx_mode|smtant'),
    ('Channel/BW', r'channel|_bw|bw_|band|switch_ch|set_ch|agc_tab'),
    ('TX power', r'txagc|tx_power|txpwr|pwr_by_rate|power_index'),
    ('Beamforming/MU', r'txbf|beamform|bf_|_bf|mu_|sounding|csi'),
    ('MAC config/WMAC', r'wmac|rcr|mac_addr|edca|retry|rrsr|rate|nav|ampdu|cfg_mac|ta_|_ta\b|aid'),
    ('System/Clock/Reset', r'sys_|_sys|clk|reset|rst|func_en|iso|afe|ldo|swr|xtal|ana_'),
]
def group_of(funcs, tabs):
    c = Counter()
    for fn in funcs:
        for g, rx in GROUP_RX:
            if re.search(rx, fn, re.I):
                c[g] += 1
                break
    for t in tabs:
        c[{'pwrseq': 'Power/PS', 'pinmux': 'GPIO/LED', 'intf-phy': 'USB'}.get(t['kind'], 'Init table')] += 1
    return c.most_common(1)[0][0] if c else ''

out = []
for (sp, ad), r in sorted(rows.items(), key=lambda kv: (kv[0][0], kv[0][1])):
    acc = r['acc']
    st_set = {a['st'] for a in acc}
    live_tab = [t for t in r['tab'] if t['live'] and t['kind'] not in ('pg-store', 'pinmux') and not t.get('cond')]
    cond_tab = [t for t in r['tab'] if t['live'] and t['kind'] != 'pg-store' and t not in live_tab]
    if 'A' in st_set or live_tab:
        st = 'A'
    elif cond_tab:
        st = 'T'          # only in run-time-conditional tables (RFE-type branch / GPIO pinmux)
    elif 'M' in st_set:
        st = 'M'
    elif 'G' in st_set:
        st = 'G'
    elif acc or r['tab']:
        st = 'X'
    else:
        used = sum(n['uses'] for n in r['names'])
        if used:
            st = 'R'
        elif any(inactive_ids.get(n['name']) for n in r['names']):
            st = 'Z'
        else:
            st = 'D'
    # primary name: prefer the chip-suffixed halmac name for MAC (real chip header)
    names = sorted({n['name'] for n in r['names']}, key=lambda n: (not n.endswith('_8822B'), len(n), n))
    live_acc = [a for a in acc if a['st'] == st] if st in 'AMG' else acc
    funcs = sorted({a['func'] for a in live_acc})
    kinds = sorted({a['kind'] for a in acc})
    widths = sorted({a['width'] for a in acc if a['width']} |
                    {{'write8': 8, 'write32': 32, 'write20': 20}.get(t['kind'], 0) for t in r['tab']} - {0})
    locs = sorted({f"{a['file']}:{a['line']}" for a in live_acc})
    tabs = sorted({t['table'] for t in r['tab']})
    bits_def = len(r['bits']); bits_used = sum(1 for b, u in r['bits'] if u)
    used_bits = [b for b, u in sorted(r['bits'], key=lambda x: -x[1]) if u][:6]
    paths = sorted({a.get('path', '') for a in acc if a['space'] == 'RF'} |
                   {t.get('path', '') for t in r['tab'] if t['kind'] == 'write20'} - {''})
    guards = Counter(a['guard'] for a in acc)
    reach = Counter(a['reach'] for a in acc)
    out.append({
        'space': sp, 'addr': ad, 'addr_hex': f'0x{ad:04X}' if ad < 0x10000 else f'0x{ad:08X}',
        'names': names, 'name_defs': [f"{n['name']} {n['file']}:{n['line']}" for n in r['names']],
        'status': st, 'group': group_of(funcs, r['tab']), 'widths': widths, 'kinds': kinds,
        'n_acc': len(acc), 'n_live': sum(1 for a in acc if a['st'] == 'A'),
        'n_mp': sum(1 for a in acc if a['st'] == 'M'), 'n_dbg': sum(1 for a in acc if a['st'] == 'G'),
        'n_dead': sum(1 for a in acc if a['st'] == 'X'),
        'funcs': funcs, 'locs': locs, 'tables': tabs, 'n_tab': len(r['tab']),
        'bits_def': bits_def, 'bits_used': bits_used, 'used_bits': used_bits,
        'paths': paths, 'guards': dict(guards), 'reach': dict(reach),
        'numeric_only': bool(acc) and not names,
    })

json.dump(out, open(os.path.join(W, 'catalog.json'), 'w'), indent=0)
with open(os.path.join(W, 'catalog.csv'), 'w', newline='', encoding='utf-8') as fh:
    w = csv.writer(fh)
    w.writerow(['space', 'addr', 'status', 'group', 'names', 'widths', 'kinds', 'n_live', 'n_mp', 'n_dbg',
                'n_dead', 'n_tab', 'tables', 'bits_def', 'bits_used', 'funcs', 'locs'])
    for o in out:
        w.writerow([o['space'], o['addr_hex'], o['status'], o['group'], ' '.join(o['names']),
                    '/'.join(map(str, o['widths'])), ' '.join(o['kinds']), o['n_live'], o['n_mp'], o['n_dbg'],
                    o['n_dead'], o['n_tab'], ' '.join(o['tables']), o['bits_def'], o['bits_used'],
                    ' '.join(o['funcs']), ' '.join(o['locs'])])

# ---------------------------------------------------------------- markdown pages
def md_escape(s):
    return s.replace('|', '\\|')

def page(fname, title, intro, spaces):
    sel = [o for o in out if o['space'] in spaces]
    cnt = Counter(o['status'] for o in sel)
    lines = [f'# {title}', '', intro, '',
             f'Tổng: **{len(sel)} địa chỉ** — ' + ', '.join(f'{k}={cnt[k]}' for k in 'ATMGXRZD' if cnt[k]), '',
             'Cột: **St** = trạng thái (A/T/M/G/X/R/Z/D, xem `04_catalog_README.md`); **W** = độ rộng truy cập quan sát '
             'được trong code/bảng (không phải độ rộng phần cứng); **Truy cập** = live/MP/dbg/dead/bảng; '
             '**Bit** = số macro bit/field định nghĩa trong section của thanh ghi / số được dùng; '
             '**Nhóm** = [INFERENCE] suy từ tên hàm truy cập.', '',
             '| STT | Địa chỉ | Tên (alias) | Không gian | St | Nhóm | W | Kiểu | Truy cập L/M/G/X/T | Bit d/u | Hàm (≤4) | Vị trí (≤3) |',
             '|---:|---|---|---|:-:|---|---|---|---|---|---|---|']
    for i, o in enumerate(sel, 1):
        nm = o['names'][0] if o['names'] else '*(không có macro)*'
        if len(o['names']) > 1:
            nm += f" (+{len(o['names']) - 1}: " + ', '.join(o['names'][1:3]) + (', …' if len(o['names']) > 3 else '') + ')'
        acc = f"{o['n_live']}/{o['n_mp']}/{o['n_dbg']}/{o['n_dead']}/{o['n_tab']}"
        fn = ', '.join(o['funcs'][:4]) + (' …' if len(o['funcs']) > 4 else '')
        loc = ', '.join(o['locs'][:3]) + (' …' if len(o['locs']) > 3 else '')
        if not loc and o['tables']:
            loc = 'bảng: ' + ', '.join(o['tables'][:2])
        spc = o['space'] + (f" {''.join(o['paths'])}" if o['paths'] else '')
        lines.append(f"| {i} | `{o['addr_hex']}` | {md_escape(nm)} | {spc} | {o['status']} | {o['group']} | "
                     f"{'/'.join(map(str, o['widths']))} | {' '.join(o['kinds'])[:30]} | {acc} | "
                     f"{o['bits_def']}/{o['bits_used']} | {md_escape(fn)} | {md_escape(loc)} |")
    open(os.path.join(DOCS, fname), 'w', encoding='utf-8').write('\n'.join(lines) + '\n')
    return cnt, len(sel)

stats = {}
stats['mac'] = page('04a_catalog_mac.md', 'Danh mục thanh ghi MAC (bus 16-bit, vùng MAC)',
     'Vùng MAC của bus thanh ghi 8822B: `0x0000–0x07FF` và `0x1000–0x17FF` '
     '(phạm vi theo `hal/halmac/halmac_reg_8822b.h`). Truy cập qua `rtw_read/write8/16/32` → USB vendor request 0x05.',
     ('MAC',))
stats['bb'] = page('04b_catalog_bb.md', 'Danh mục thanh ghi BB/PHY (bus 16-bit, vùng BB)',
     'Vùng BB: `0x0800–0x0FFF`, `0x1800–0x1FFF`; vùng `0x2800–0x2FFF` = cửa sổ đọc RF trực tiếp (RF-direct). '
     'Cùng bus với MAC — `phy_set_bb_reg` cuối cùng vẫn là `rtw_write32`. BB-pg = khoá địa chỉ trong bảng '
     'PHY_REG_PG (lưu vào bảng công suất, không ghi trực tiếp).',
     ('BB', 'RF-direct', 'BB-pg', 'other'))
stats['rf'] = page('04c_catalog_rf.md', 'Danh mục thanh ghi RF (không gian RF riêng, mỗi path 8-bit địa chỉ × 20-bit dữ liệu)',
     'Địa chỉ RF là chỉ số 0x00–0xFF **trong từng path** (A/B), không phải địa chỉ bus. '
     'Đọc: BB `0x2800/0x2C00 + addr*4`; ghi: BB `0xC90/0xE90` = `(addr<<20)|data20` '
     '(`phydm_hal_api8822b.c:995-1074`).',
     ('RF',))
stats['other'] = page('04d_catalog_other.md', 'Danh mục không gian khác: USB-local, USB3-PHY, SDIO-local',
     'USB-local `0xFE00–0xFFFF` (khối USB của chip, `usbctrl_vendorreq` coi là REG_LOCAL_SEC); '
     'USB3-PHY: không gian PHY gián tiếp qua `usbphy_write_88xx` (`0xFF0C–0xFF0E`); '
     'SDIO-local `0x1025xxxx`: chỉ định nghĩa, không dùng được qua USB.',
     ('USB-local', 'USB3-PHY', 'USB2-PHY', 'SDIO-local', 'MAC-other'))
json.dump({k: [dict(v[0]), v[1]] for k, v in stats.items()}, open(os.path.join(W, 'catalog_stats.json'), 'w'))
print({k: (v[1], dict(v[0])) for k, v in stats.items()})
