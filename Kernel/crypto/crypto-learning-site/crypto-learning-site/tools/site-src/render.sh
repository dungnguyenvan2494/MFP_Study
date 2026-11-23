#!/bin/bash
# usage: [PNG=1] render.sh <id>...   (after checkdg.sh)
# Renders $WORK/<id>.mmd with mermaid-cli using the site config; reports the SVG size,
# WIDE (> 1300px), HYPHEN-BREAK (sequence text split inside an identifier) and LONGLINE
# (flowchart label wrapped by the browser inside an identifier).
# Needs mermaid-cli: set MMDC=/path/to/mmdc (default: mmdc on PATH) and, if Chromium is not
# auto-detected, PUPPETEER_CFG=/path/to/puppeteer.json ({"executablePath": "...", "args": ["--no-sandbox"]}).
HERE=$(cd "$(dirname "$0")" && pwd)
WORK=${WORK:-$HERE/../.work}
MMDC=${MMDC:-mmdc}
PP=()
[ -n "$PUPPETEER_CFG" ] && PP=(-p "$PUPPETEER_CFG")
for id in "$@"; do
  if timeout 120 "$MMDC" "${PP[@]}" -c "$WORK/config.json" -i "$WORK/$id.mmd" -o "$WORK/$id.svg" -b white >/dev/null 2>"$WORK/$id.err"; then
    python3 - "$WORK/$id.svg" "$id" <<'PY'
import sys, re, html as H
s = open(sys.argv[1]).read()
m = re.search(r'viewBox="[-\d.]+ [-\d.]+ ([\d.]+) ([\d.]+)"', s)
w, h = (float(m.group(1)), float(m.group(2))) if m else (0, 0)
broken = [t for t in re.findall(r'>([^<>]*)</(?:text|tspan)>', s) if re.search(r'\w-$', t.strip())]
flag = ' WIDE' if w > 1300 else ''
longl = []
for m in re.finditer(r'<foreignObject[^>]*><div[^>]*style="([^"]*)"[^>]*>(.*?)</div></foreignObject>', s, re.S):
    if 'break-spaces' not in m.group(1): continue
    for line in re.split(r'<br\s*/?>', m.group(2)):
        t = H.unescape(re.sub(r'<[^>]+>', '', line)).strip()
        if len(t) > 27 and (re.search(r'\w-\w', t) or any(len(tok) > 24 for tok in t.split())):
            longl.append(t)
if longl: flag += '  LONGLINE: ' + repr(longl[:4])
print(f"OK  {sys.argv[2]} {int(w)}x{int(h)}{flag}" + (f"  HYPHEN-BREAK: {broken[:4]}" if broken else ''))
PY
    [ "$PNG" = 1 ] && timeout 120 "$MMDC" "${PP[@]}" -c "$WORK/config.json" -i "$WORK/$id.mmd" -o "$WORK/$id.png" -b white >/dev/null 2>&1
  else echo "FAIL $id: $(grep -m1 -A3 'Error' "$WORK/$id.err" | tr '\n' ' ' | cut -c1-300)"; fi
done
