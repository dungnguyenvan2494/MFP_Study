"""Rewrite single-participant sequence notes whose text cannot fit the fixed note width
(~176px ≈ 22 chars at 13px) into notes spanning the participant and its neighbour."""
import re, glob
LIMIT = 22
changed = 0
for f in sorted(glob.glob('diagrams/*.dg')):
    txt = open(f).read()
    out = []
    for block in re.split(r'(?m)^(?==== )', txt):
        if '\nsequenceDiagram' not in block:
            out.append(block); continue
        parts = re.findall(r'^\s*participant (\w+)', block, re.M)
        def fix(m):
            global changed
            kind, who, text = m.group(2), m.group(3).strip(), m.group(4)
            if ',' in who: return m.group(0)
            if max(len(x) for x in text.split('<br/>')) <= LIMIT: return m.group(0)
            i = parts.index(who)
            if kind == 'left of': pair = (parts[i-1], who) if i > 0 else (who, parts[i+1])
            else: pair = (who, parts[i+1]) if i + 1 < len(parts) else (parts[i-1], who)
            changed += 1
            return f'{m.group(1)}Note over {pair[0]},{pair[1]}: {text}'
        block = re.sub(r'^(\s*)Note (over|left of|right of) ([^:]+):\s*(.*)$', fix, block, flags=re.M)
        out.append(block)
    open(f, 'w').write(''.join(out))
print('notes widened:', changed)
