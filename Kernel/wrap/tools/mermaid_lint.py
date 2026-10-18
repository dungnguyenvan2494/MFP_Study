#!/usr/bin/env python3
"""
mermaid_lint.py - kiem tra cu phap Mermaid kieu HEURISTIC cho docs/swap/*.md.

Khong phai renderer that (may khong co node/mermaid-cli). Bat cac loi hay gap:
  - kieu diagram dong dau khong hop le
  - so dau nhay kep le tren mot dong
  - ngoac [ ] { } ( ) khong can bang ben ngoai chuoi trong
  - |label| cua canh khong can bang
  - subgraph/alt/opt/loop/rect/par ... end khong can bang
  - dung 'end' lam node id
  - ky tu ';' hoac '#' trong text cua sequenceDiagram (';' la dau ngat lenh)
  - participant chua khai bao trong message
  - node id duoc tham chieu nhung chua bao gio co nhan/hinh dang (canh bao)
Chay:  python mermaid_lint.py ..   (thu muc chua cac file .md)
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else "..")
VALID_HEAD = ("flowchart TD", "flowchart LR", "flowchart TB", "flowchart BT", "flowchart RL",
              "graph TD", "graph LR", "sequenceDiagram", "stateDiagram-v2", "classDiagram")


def strip_quoted(s):
    return re.sub(r'"[^"]*"', '""', s)


def blocks(text):
    out, cur, start = [], None, 0
    for i, line in enumerate(text.splitlines(), 1):
        if line.strip() == "```mermaid":
            cur, start = [], i
        elif line.strip() == "```" and cur is not None:
            out.append((start, cur))
            cur = None
        elif cur is not None:
            cur.append((i, line))
    return out


def lint_flow(lines, err, warn):
    depth = 0
    shaped, referenced = set(), set()
    node_re = re.compile(r'\b([A-Za-z_][A-Za-z0-9_]*)\s*(\(\[|\[\(|\[|\{|\(\()')
    id_re = re.compile(r'(?:^|-->|-\.->|---|==>)\s*(?:\|"[^"]*"\|)?\s*([A-Za-z_][A-Za-z0-9_]*)')
    for ln, raw in lines:
        s = raw.strip()
        if not s or s.startswith("%%"):
            continue
        if s.count('"') % 2:
            err(ln, "so dau \" le")
        q = strip_quoted(s)
        for a, b in ("[]", "{}", "()"):
            if q.count(a) != q.count(b):
                err(ln, f"ngoac {a}{b} khong can bang: {s}")
        if q.count("|") % 2:
            err(ln, "dau | cua nhan canh khong can bang")
        if "#" in s:
            err(ln, "ky tu # co the bi hieu la entity")
        if s.startswith("subgraph"):
            depth += 1
            m = re.match(r'subgraph\s+([A-Za-z_][A-Za-z0-9_]*)', s)
            if m:
                shaped.add(m.group(1))
            continue
        if s == "end":
            depth -= 1
            if depth < 0:
                err(ln, "'end' thua")
            continue
        for m in node_re.finditer(q):
            if m.group(1) == "end":
                err(ln, "dung 'end' lam node id")
            shaped.add(m.group(1))
        for m in id_re.finditer(q):
            referenced.add(m.group(1))
    if depth:
        err(lines[-1][0], f"subgraph/end lech {depth}")
    for n in sorted(referenced - shaped):
        warn(0, f"node '{n}' duoc tham chieu nhung chua co nhan/hinh dang")


def lint_seq(lines, err, warn):
    parts = set()
    depth = 0
    msg_re = re.compile(r'^([A-Za-z_][A-Za-z0-9_]*)\s*(-->>|->>|-->|->|--x|-x|--\)|-\))\s*([A-Za-z_][A-Za-z0-9_]*)\s*:\s*(.*)$')
    for ln, raw in lines:
        s = raw.strip()
        if not s or s.startswith("%%") or s == "autonumber":
            continue
        m = re.match(r'(participant|actor)\s+([A-Za-z_][A-Za-z0-9_]*)', s)
        if m:
            parts.add(m.group(2))
            continue
        if re.match(r'(alt|opt|loop|par|rect|critical|break)\b', s):
            depth += 1
            continue
        if re.match(r'(else|and)\b', s):
            if depth <= 0:
                err(ln, "else/and ngoai block")
            continue
        if s == "end":
            depth -= 1
            if depth < 0:
                err(ln, "'end' thua")
            continue
        if s.startswith("Note"):
            body = s.split(":", 1)[1] if ":" in s else ""
            if ";" in body or "#" in body:
                err(ln, "';' hoac '#' trong Note")
            for p in re.findall(r'Note\s+(?:over|left of|right of)\s+([A-Za-z0-9_, ]+):', s):
                for x in p.split(","):
                    if x.strip() not in parts:
                        err(ln, f"Note tren participant chua khai bao: {x.strip()}")
            continue
        m = msg_re.match(s)
        if not m:
            err(ln, f"dong khong nhan dang duoc: {s}")
            continue
        a, _, b, text = m.groups()
        for x in (a, b):
            if x not in parts:
                err(ln, f"participant chua khai bao: {x}")
        if ";" in text or "#" in text:
            err(ln, "';' hoac '#' trong message (';' la dau ngat lenh)")
    if depth:
        err(lines[-1][0], f"block alt/opt/loop/rect lech {depth}")


def main():
    total_err = total_blocks = 0
    for md in sorted(ROOT.glob("*.md")):
        for start, lines in blocks(md.read_text(encoding="utf-8")):
            total_blocks += 1
            msgs = []
            err = lambda ln, m: msgs.append(("LOI", ln, m))
            warn = lambda ln, m: msgs.append(("CANH BAO", ln, m))
            body = [(ln, l) for ln, l in lines if l.strip()]
            head = body[0][1].strip() if body else ""
            if not head.startswith(VALID_HEAD):
                err(start, f"dong dau khong hop le: {head}")
            elif head.startswith(("flowchart", "graph")):
                lint_flow(body[1:], err, warn)
            elif head.startswith("sequenceDiagram"):
                lint_seq(body[1:], err, warn)
            n_err = sum(1 for k, *_ in msgs if k == "LOI")
            total_err += n_err
            status = "OK " if not n_err else "LOI"
            print(f"[{status}] {md.name}:{start} {head}")
            for k, ln, m in msgs:
                print(f"       {k} dong {ln}: {m}")
    print(f"\n{total_blocks} so do, {total_err} loi")
    return 1 if total_err else 0


if __name__ == "__main__":
    sys.exit(main())
