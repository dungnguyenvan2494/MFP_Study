import re, glob, os
CLASSDEFS = """    classDef api fill:#E8EEFC,stroke:#1D4ED8,stroke-width:1.5px,color:#0F1B33
    classDef core fill:#F1F5F9,stroke:#475569,color:#172033
    classDef impl fill:#ECFDF5,stroke:#15803D,color:#14532D
    classDef drv fill:#FFF7ED,stroke:#B45309,color:#431407
    classDef err fill:#FEF2F2,stroke:#B91C1C,color:#7F1D1D
    classDef ok fill:#F0FDF4,stroke:#15803D,color:#14532D
    classDef dec fill:#FFFFFF,stroke:#1D4ED8,stroke-width:1.5px,color:#172033
    classDef cb fill:#F5F3FF,stroke:#6D28D9,color:#2E1065
    classDef ext fill:#FFFFFF,stroke:#94A3B8,stroke-dasharray:4 3,color:#334155"""
def load(path_glob):
    out = []
    for f in sorted(glob.glob(path_glob)):
        txt = open(f).read()
        for block in re.split(r'^=== ', txt, flags=re.M)[1:]:
            head, _, body = block.partition('\n---\n')
            lines = head.split('\n')
            d = {'id': lines[0].strip(), 'file': os.path.basename(f)}
            for l in lines[1:]:
                if not l.strip(): continue
                k, _, v = l.partition(':')
                d[k.strip()] = v.strip()
            src = body.rstrip() + '\n'
            src = src.replace('    %% @classdefs', CLASSDEFS)
            d['src'] = src
            out.append(d)
    return out
