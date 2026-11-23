import sys, re, glob
sys.path.insert(0,'.')
import dglib
# single-participant notes are ~176px wide (sequence.width); at 13px font a char ≈ 7px
bad=0
for d in dglib.load('diagrams/*.dg'):
    if d['type']!='sequence': continue
    for ln in d['src'].split('\n'):
        m=re.match(r'\s*Note (over|left of|right of) ([^:]+):\s*(.*)', ln)
        if not m or ',' in m.group(2): continue
        for part in re.split(r'<br/>', m.group(3)):
            toks=part.split()
            w=len(part)*7.0
            longtok=max((len(t) for t in toks), default=0)*7.0
            if w>168 and longtok>150 or len(part)>30:
                print(d['id'], '|', ln.strip()[:110]); bad+=1; break
print('flagged', bad)
