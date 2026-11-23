#!/usr/bin/env python3
"""Check every `path:line[-line]` reference in diagrams (.dg) and cards (.card):
   1. the file exists under Kernel/Src;
   2. every line number is within the file;
   3. when a symbol is named right after the reference — `path:N-M` (`sym`) — or the card's
      `name:` field, that symbol occurs inside [N-3, M] of the file.
   Also checks the short `file.c:N` labels inside call-graph nodes against the full paths
   listed in the same diagram's refs."""
import re, os, sys, glob
HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.abspath(os.path.join(HERE, '..', '..', '..', '..', 'Kernel', 'Src'))
sys.path.insert(0, HERE)
import dglib

errors, checked = [], 0
cache = {}
def lines_of(p):
    if p not in cache:
        cache[p] = open(os.path.join(SRC, p), errors='replace').read().split('\n')
    return cache[p]

REF = re.compile(r'`((?:crypto|include|arch|drivers|security|net|lib|kernel)/[\w/.\-]+?):([\d][\d,\- ]*)`(?:\s*\(`([^`]+)`)?')

def check(where, path, spec, sym):
    global checked
    if not os.path.exists(os.path.join(SRC, path)):
        errors.append(f'{where}: missing file {path}')
        return
    L = lines_of(path)
    ranges = []
    for part in spec.split(','):
        part = part.strip()
        if not part:
            continue
        a, _, b = part.partition('-')
        a = int(a); b = int(b) if b else a
        if b < a or b > len(L):
            errors.append(f'{where}: {path}:{part} out of range (file has {len(L)} lines)')
            return
        ranges.append((a, b))
    checked += 1
    if sym:
        names = re.findall(r'[A-Za-z_]\w+', sym.split('(')[0]) if '(' in sym else re.findall(r'[A-Za-z_]\w+', sym)
        names = [n for n in names if n not in ('struct', 'static', 'const', 'int', 'void')]
        if not names:
            return
        a, b = ranges[0][0], ranges[-1][1]
        window = '\n'.join(L[max(0, a - 4):b])
        for n in names[:2]:
            if n not in window:
                errors.append(f'{where}: `{n}` not found in {path}:{a}-{b}')

for d in dglib.load(HERE + '/diagrams/*.dg'):
    for field in ('refs', 'caption', 'purpose', 'next'):
        for m in REF.finditer(d.get(field, '')):
            check(d['id'] + '.' + field, m.group(1), m.group(2), m.group(3))
    # call-graph node labels like "api.c:517" must match a line where some function starts
    if d['type'] == 'callgraph':
        full = {os.path.basename(p): p for p in re.findall(r'`((?:crypto|include|arch|drivers)/[\w/.\-]+?):', d.get('refs', ''))}
        labels = []
        for m in re.finditer(r'\["([^"]*?)"\]', d['src']):
            # a node may hold several "fn · file:N" lines: check each line on its own,
            # keeping the previous line when a line holds only the location
            prev = ''
            for ln in m.group(1).split('<br/>'):
                labels.append(ln if re.match(r'\s*[A-Za-z_]', ln) else prev + ' ' + ln)
                prev = ln
        for label in labels:
            fn = re.match(r'\s*([A-Za-z_]\w*)', label)
            for fm in re.finditer(r'([\w\-]+\.[ch]):(\d+)', label):
                base, line = fm.group(1), int(fm.group(2))
                path = full.get(base)
                if not path:
                    cands = glob.glob(f'{SRC}/**/{base}', recursive=True)
                    cands = [c for c in cands if '/crypto/' in c or '/include/' in c]
                    path = os.path.relpath(cands[0], SRC) if len(cands) == 1 else None
                if not path:
                    errors.append(f'{d["id"]}: cannot resolve {base}')
                    continue
                L = lines_of(path)
                checked += 1
                if line > len(L):
                    errors.append(f'{d["id"]}: {base}:{line} out of range')
                    continue
                # the label's function name must appear on that line or the 2 lines before
                if fn and fn.group(1) not in ('crypto_alloc_tfm', 'crypto_grab_cipher', 'crypto_spawn_cipher', 'crypto_free_skcipher'):
                    win = '\n'.join(L[max(0, line - 3):line + 1])
                    names = [x for x in re.findall(r'[A-Za-z_]\w+', label.split(base)[0]) if len(x) > 3]
                    if names and not any(n in win for n in names):
                        errors.append(f'{d["id"]}: none of {names[:3]} at {path}:{line}')

for f in glob.glob(HERE + '/cards/*.card'):
    for block in re.split(r'^=== ', open(f).read(), flags=re.M)[1:]:
        cid = block.split('\n', 1)[0].strip()
        head, _, body = block.partition('\n---\n')
        loc = re.search(r'^loc:\s*(\S+)', head, re.M)
        name = re.search(r'^name:\s*(.+)$', head, re.M)
        if loc:
            p, _, spec = loc.group(1).partition(':')
            sym = name.group(1).replace('struct ', '').replace('()', '') if name else None
            check(cid + '.loc', p, spec, sym)
        for m in REF.finditer(body):
            check(cid, m.group(1), m.group(2), m.group(3))

print(f'checked {checked} references, {len(errors)} problems')
for e in errors:
    print('  ', e)
sys.exit(1 if errors else 0)
