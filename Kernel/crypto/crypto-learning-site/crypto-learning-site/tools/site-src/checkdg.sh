#!/bin/bash
# usage: checkdg.sh "<glob of .dg files>"
# Writes each diagram's Mermaid source to $WORK/<id>.mmd (classdefs expanded) and the site's
# Mermaid config to $WORK/config.json; prints the diagram ids. WORK defaults to tools/.work
HERE=$(cd "$(dirname "$0")" && pwd)
WORK=${WORK:-$HERE/../.work}
mkdir -p "$WORK"
node -e "const c=require('$HERE/../../assets/js/mermaid-config.js'); const o=Object.assign({},c); delete o.startOnLoad; delete o.securityLevel; delete o.suppressErrorRendering; require('fs').writeFileSync('$WORK/config.json', JSON.stringify(o))"
python3 - "$1" "$HERE" "$WORK" <<'PY'
import sys; sys.path.insert(0, sys.argv[2])
import dglib
for d in dglib.load(sys.argv[1]):
    open(f"{sys.argv[3]}/{d['id']}.mmd", 'w').write(d['src'])
    print(d['id'])
PY
