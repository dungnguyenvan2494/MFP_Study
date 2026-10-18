#!/usr/bin/env python3
"""Lượt 6 — completeness statistics + document sanity checks.

Per catalog entry, the 6 states asked for in 00_REQUEST.md §9:
  S1 defined + uses traced     : has a name AND status in A/T/M/G/X/R (some reference found)
  S2 defined, no use found     : status D or Z (Z = only in disabled code/comments)
  S3 accessed by number        : numeric_only (no macro name, accessed via literal/R_0x...)
  S4 function analysed         : address (hex) or one of its names appears in a hand-written
                                 05_groups/0*,1*.md, 07_sequences.md, 02/03 docs, or appendix A11/A13
  S5 bits/fields with evidence : bits_used non-empty (named field of the register used in code)
                                 or decoded by bitname in a group doc (approximated by S4 + bits_def)
  S6 needs hardware docs       : live (A/T/M/G) but no named bit field defined (bits_def empty)
Also checks: relative markdown links resolve; no NUL/backspace control characters.

Usage: py -I completion.py <work_dir> <docs_dir> [out.md]
"""
import glob
import json
import os
import re
import sys
from collections import Counter, defaultdict

W, DOCS = sys.argv[1], sys.argv[2]
out_path = sys.argv[3] if len(sys.argv) > 3 else None
cat = json.load(open(os.path.join(W, 'catalog.json')))

hand = []
for pat in ('05_groups/0*.md', '05_groups/1*.md', '07_sequences.md', '02_address_spaces.md',
            '03_access_architecture.md', '01_build_config.md', '05_groups/appendix/A11_*.md',
            '05_groups/appendix/A13_*.md'):
    hand += glob.glob(os.path.join(DOCS, pat))
txt = ''.join(open(f, encoding='utf-8').read() for f in hand)
low = txt.lower()
hexset = set(m.group(0) for m in re.finditer(r'0x[0-9a-f]+', low))


def mentioned(e):
    a = e['addr']
    if {f'0x{a:x}', f'0x{a:02x}', f'0x{a:03x}', f'0x{a:04x}'} & hexset:
        return True
    return any(n and n in txt for n in e['names'][:6])


LIVE = set('ATMG')
rows = defaultdict(Counter)
for e in cat:
    sp = e['space']
    st = e['status']
    named = bool(e['names'])
    r = rows[sp]
    r['total'] += 1
    r['st_' + st] += 1
    if named and st in 'ATMGXR':
        r['S1'] += 1
    if st in 'DZ':
        r['S2'] += 1
    if e.get('numeric_only'):
        r['S3'] += 1
    if st in LIVE:
        r['live'] += 1
        if mentioned(e):
            r['S4'] += 1
        if e.get('bits_used'):
            r['S5'] += 1
        if not e.get('bits_def'):
            r['S6'] += 1

# ---- doc checks
bad_links, ctrl = [], []
for f in glob.glob(os.path.join(DOCS, '**', '*.md'), recursive=True):
    if os.sep + 'work' + os.sep in f:
        continue
    raw = open(f, 'rb').read()
    for b in (b'\x00', b'\x08'):
        if b in raw:
            ctrl.append((os.path.relpath(f, DOCS), b))
    s = raw.decode('utf-8', errors='replace')
    for m in re.finditer(r'\]\(([^)#\s]+)(#[^)]*)?\)', s):
        tgt = m.group(1)
        if re.match(r'[a-z]+://', tgt):
            continue
        p = os.path.normpath(os.path.join(os.path.dirname(f), tgt))
        if not os.path.exists(p):
            bad_links.append((os.path.relpath(f, DOCS), tgt))

L = ['# 12 — Bảng theo dõi mức độ hoàn thành (Lượt 6)', '',
     '> Sinh bởi `tools/completion.py work .. 12_completion.md`. Định nghĩa 6 trạng thái theo 00_REQUEST §9 — xem docstring của script. '
     '"Live" = trạng thái A/T/M/G (có truy cập ở nhánh chạy được hoặc MP/debug).', '',
     '## 1. Theo không gian', '',
     '| Không gian | Tổng | Live (A/T/M/G) | S1 định nghĩa + truy vết | S2 định nghĩa, không dùng (D/Z) | S3 chỉ truy cập bằng số | S4 live đã phân tích ở tài liệu tay | S5 live có field/bit được code dùng | S6 live không có định nghĩa bit (cần tài liệu HW) |',
     '|---|---|---|---|---|---|---|---|---|']
tot = Counter()
for sp in sorted(rows, key=lambda s: -rows[s]['total']):
    r = rows[sp]
    tot.update(r)
    L.append(f"| {sp} | {r['total']} | {r['live']} | {r['S1']} | {r['S2']} | {r['S3']} | {r['S4']} | {r['S5']} | {r['S6']} |")
L.append(f"| **Tổng** | **{tot['total']}** | **{tot['live']}** | **{tot['S1']}** | **{tot['S2']}** | **{tot['S3']}** | **{tot['S4']}** | **{tot['S5']}** | **{tot['S6']}** |")
L += ['', '## 2. Phân bố trạng thái sử dụng', '', '| Không gian | ' + ' | '.join('ATMGXRZD') + ' |', '|---' * 9 + '|']
for sp in sorted(rows, key=lambda s: -rows[s]['total']):
    L.append(f'| {sp} | ' + ' | '.join(str(rows[sp]['st_' + c]) for c in 'ATMGXRZD') + ' |')
L += ['', '## 3. Kiểm tra tài liệu', '',
      f'- Liên kết tương đối không tồn tại: **{len(bad_links)}**' + (''.join(f'\n  - `{a}` → `{b}`' for a, b in bad_links[:40])),
      f'- Ký tự điều khiển NUL/backspace trong .md: **{len(ctrl)}**' + (''.join(f'\n  - `{a}` {b!r}' for a, b in ctrl)), '']
L += ['## 4. Diễn giải', '',
      '- **S4 < live** không có nghĩa là thiếu bằng chứng: mọi địa chỉ live đều có danh sách truy cập đầy đủ (file:dòng, hàm, guard) '
      'trong phụ lục sinh tự động A01–A10 và A12. S4 chỉ đếm địa chỉ được **giải thích tay** trong 01–03, 05.01–05.17, 07, A11, A13. '
      'Phần lớn BB/RF chưa S4 là thanh ghi chỉ được nạp một lần từ bảng PHY_REG/RadioA/B (13 §2, 14 §2) — không có tên, field hay nơi dùng khác.',
      '- **S6** (live nhưng source không định nghĩa field): BB/RF gần như toàn bộ — phydm dùng mask số trực tiếp; ý nghĩa bit chỉ có '
      'khi có comment trong source (đã trích ở 13/14/15) hoặc tài liệu Realtek (không có trong repository).',
      '- Trạng thái R đã rà tay ở A13; X/Z đã kiểm chứng mẫu (PROGRESS: 5 ca dead + 6 ca sau cải tiến guard).', '']
marks = []
for f in hand:
    rel = os.path.relpath(f, DOCS)
    for i, ln in enumerate(open(f, encoding='utf-8').read().split('\n'), 1):
        if 'KIỂM TRA TRÊN BOARD' in ln or 'KIỂM TRA:' in ln:
            marks.append(('board', rel, i, ln.strip()))
        elif re.search(r'chưa xác định|không xác định', ln, re.I):
            marks.append(('unk', rel, i, ln.strip()))
L += ['## 5. Vấn đề chưa xác minh (tự gom từ tài liệu tay)', '',
      f'### 5.1 Cần kiểm tra trên board ({sum(1 for m in marks if m[0] == "board")})', '']
for k, rel, i, ln in marks:
    if k == 'board':
        L.append(f'- `{rel}:{i}` — {ln[:260]}')
L += ['', f'### 5.2 Ý nghĩa chưa xác định từ source ({sum(1 for m in marks if m[0] == "unk")})', '']
for k, rel, i, ln in marks:
    if k == 'unk':
        L.append(f'- `{rel}:{i}` — {ln[:260]}')
L.append('')
text = '\n'.join(L) + '\n'
if out_path:
    open(out_path, 'w', encoding='utf-8').write(text)
sys.stdout.reconfigure(encoding='utf-8', errors='replace')
print(text)
