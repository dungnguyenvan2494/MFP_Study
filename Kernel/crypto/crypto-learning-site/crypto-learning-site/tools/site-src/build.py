#!/usr/bin/env python3
"""Build docs/crypto-learning-site/index.html from:
   - content/*.md   : chapter recipes (header + markdown body with directives)
   - diagrams/*.dg  : Mermaid diagrams with metadata
   - cards/*.card   : function / struct / concept cards
   - research docs  : docs/crypto-learning/** (included by section, never modified)
Directives (each on its own line):
   {{include FILE#SEC}}            body of section SEC (e.g. 3.4 / 9) from research FILE
   {{include FILE#SEC as ###}}     same, but keep the heading at the given level
   {{diagram ID}}  {{card ID}}  {{quiz FILE}}  {{tree}}  {{glossary}}  {{diagram-index}}
   :::warn|error|info|vendor|ok|concept|details Title   ...   :::
"""
import re, os, sys, glob, html, json, subprocess, unicodedata
import markdown
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dglib

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, '..', '..', '..', '..'))   # tools/site-src → repo root
RESEARCH = REPO + '/docs/crypto-learning'
KSRC = REPO + '/Kernel/Src'
OUT = REPO + '/docs/crypto-learning-site/index.html'

WARN = []
def warn(msg):
    WARN.append(msg)

# ---------------------------------------------------------------- research files
FILEMAP = {}   # research basename -> chapter id (filled from recipe headers "source:")

def research_path(name):
    for p in (RESEARCH + '/' + name, RESEARCH + '/chapters/' + name):
        if os.path.exists(p):
            return p
    raise SystemExit('research file not found: ' + name)

def split_sections(text):
    """Return list of (level, number, title, start, end) for markdown headings outside code fences."""
    lines = text.split('\n')
    heads, fence = [], False
    for i, l in enumerate(lines):
        if l.startswith('```'):
            fence = not fence
            continue
        if fence:
            continue
        m = re.match(r'^(#{1,6})\s+(.*)$', l)
        if m:
            t = m.group(2).strip()
            n = re.match(r'^(\d+(?:\.\d+)*)\.?\s+(.*)$', t)
            heads.append([len(m.group(1)), n.group(1) if n else '', n.group(2) if n else t, i])
    out = []
    for k, h in enumerate(heads):
        end = len(lines)
        for h2 in heads[k + 1:]:
            if h2[0] <= h[0]:
                end = h2[3]
                break
        out.append((h[0], h[1], h[2], h[3], end))
    return lines, out

MERMAID_DROPPED = []

def include(spec, chap):
    m = re.match(r'^(\S+?)#(\S+)(?:\s+as\s+(#+))?$', spec.strip())
    if not m:
        raise SystemExit('bad include: ' + spec)
    fname, sec, level = m.group(1), m.group(2), m.group(3)
    text = open(research_path(fname)).read()
    lines, secs = split_sections(text)
    hit = [s for s in secs if s[1] == sec] or [s for s in secs if s[2].lower().startswith(sec.lower().replace('_', ' '))]
    if not hit:
        raise SystemExit(f'section {sec} not found in {fname}')
    lv, num, title, a, b = hit[0]
    body = lines[a + 1:b]
    # demote/promote nested headings relative to the requested level
    base = len(level) if level else lv
    res = []
    fence = False
    for l in body:
        if l.startswith('```'):
            fence = not fence
        hm = None if fence else re.match(r'^(#{1,6})\s+(.*)$', l)
        if hm:
            nl = len(hm.group(1)) - lv + base
            t = re.sub(r'^\d+(?:\.\d+)*\.?\s+', '', hm.group(2))
            res.append('#' * max(2, min(nl, 6)) + ' ' + t)
        else:
            res.append(l)
    body = '\n'.join(res)
    # drop research mermaid blocks (redesigned diagrams replace them)
    def drop(mm):
        MERMAID_DROPPED.append((chap, fname, sec))
        return ''
    body = re.sub(r'```mermaid\n.*?\n```\n?', drop, body, flags=re.S)
    # raw <details> blocks of the research docs -> collapsible blocks of the site
    body = re.sub(r'<details>\s*<summary>(.*?)</summary>\s*\n(.*?)\n\s*</details>',
                  lambda m: ':::details ' + re.sub(r'</?b>', '', m.group(1)).strip() + '\n' + m.group(2).strip('\n') + '\n:::',
                  body, flags=re.S)
    body = rewrite_links(body)
    if level:
        body = level + ' ' + title + '\n\n' + body
    return body.strip('\n') + '\n'

def rewrite_links(md):
    def rep(m):
        label, target = m.group(1), m.group(2)
        if re.match(r'^(https?:|#|mailto:)', target):
            return m.group(0)
        path, _, frag = target.partition('#')
        base = os.path.basename(path)
        if base in FILEMAP:
            if (re.match(r'^[Cc]hương \d\d$', label) or re.search(r'\.md\b|^[A-Z][A-Z_]{4,}', label)) and FILEMAP[base] in CH_TITLES:
                label = CH_TITLES[FILEMAP[base]]
            return f'[{label}](#{FILEMAP[base]})'
        warn(f'unmapped link {target}')
        return label
    md = re.sub(r'\[([^\]]+)\]\(([^)\s]+)\)', rep, md)
    return link_chapter_numbers(md)

RESEARCH_NUM = {os.path.basename(f)[:2]: os.path.basename(f) for f in glob.glob(RESEARCH + '/chapters/*.md')}

def link_chapter_numbers(md):
    """'Chương 04 (struct), 06' (numbering of the research docs) -> links to the site chapters."""
    def one(k):
        cid = FILEMAP.get(RESEARCH_NUM.get(k.group(1), ''))
        if cid and cid in CH_TITLES:
            return f'[{CH_TITLES[cid]}](#{cid})'
        warn('unmapped chapter number ' + k.group(1))
        return k.group(0)
    def rep(m):
        return m.group(1) + ' ' + re.sub(r'\b(\d\d)\b(?![\d.:])', one, m.group(2))
    item = r'\d\d(?![\d.:])(?:\s*\([^()]*\))?'
    return re.sub(r'(?<!\[)\b([Cc]hương)\s+(' + item + r'(?:\s*(?:,|và|/)\s*' + item + r')*)', rep, md)

# ---------------------------------------------------------------- markdown helpers
def slug(s):
    s = s.replace('đ', 'd').replace('Đ', 'D')
    s = unicodedata.normalize('NFKD', s)
    s = ''.join(c for c in s if not unicodedata.combining(c))
    s = re.sub(r'<[^>]+>', '', s).lower()
    s = re.sub(r'[^a-z0-9]+', '-', s).strip('-')
    return s[:60].strip('-') or 'sec'

def md_to_html(text):
    return markdown.markdown(text, extensions=['tables', 'fenced_code', 'sane_lists'])

def md_inline(text):
    h = md_to_html(text).strip()
    m = re.match(r'^<p>(.*)</p>$', h, re.S)
    return postprocess_inline(m.group(1) if m else h)

PATH_RE = re.compile(r'^(?:[\w.\-]+/)+[\w.\-*{},]*(?::[\d,\- ]+)?$|^[\w\-*]+\.(?:c|h|S|asn1|rst|txt)(?::[\d,\- ]+)?$|^(?:Makefile|Kconfig|Kbuild)$')
def classify_code(t):
    raw = html.unescape(t)
    if raw.startswith('CONFIG_') or re.match(r'^CRYPTO_[A-Z0-9_]+(=[ymn])?$', raw) and raw.isupper():
        return 'kconfig' if raw.startswith('CONFIG_') else 'sym'
    if PATH_RE.match(raw) and ('/' in raw or '.' in raw or raw in ('Makefile', 'Kconfig')):
        return 'path'
    if re.match(r'^(struct |enum |union )?[A-Za-z_][\w>\-.]*(\(.*\))?(\s*\*)?$', raw) or re.match(r'^[A-Za-z_]\w*\(.*\)$', raw):
        return 'sym'
    return ''

TAGS = {'F': ('tag-fact', 'Fact', 'Sự kiện đã kiểm chứng trong source'),
        'I': ('tag-inf', 'Suy luận', 'Suy luận từ source, chưa chạy thực tế'),
        'H': ('tag-hyp', 'Giả thuyết', 'Giả thuyết, cần kiểm chứng')}

def postprocess_inline(h):
    def code(m):
        c = classify_code(m.group(1))
        raw = html.unescape(m.group(1))
        if '-' in raw and len(raw) <= 28:      # keep -EINPROGRESS, cbc-aes-ce … on one line
            c = (c + ' nobr').strip()
        return f'<code class="{c}">{m.group(1)}</code>' if c else m.group(0)
    h = re.sub(r'<code>([^<]*)</code>', code, h)
    h = re.sub(r'\[([FIH])\]', lambda m: '<span class="{0}" title="{2}">{1}</span>'.format(*TAGS[m.group(1)]), h)
    return h

def postprocess(h, chap, ids):
    # code blocks
    def pre(m):
        attrs, body = m.group(1), m.group(2)
        lm = re.search(r'class="language-([\w+-]+)"', attrs)
        lang = lm.group(1) if lm else 'plaintext'
        if lang in ('text', 'txt', 'console', 'output'):
            lang = 'plaintext'
        if lang == 'sh':
            lang = 'bash'
        return f'<div class="code-block" data-lang="{lang}"><pre><code class="language-{lang}">{body}</code></pre></div>'
    h = re.sub(r'<pre><code([^>]*)>(.*?)</code></pre>', pre, h, flags=re.S)
    h = re.sub(r'<table>', '<div class="table-wrap" tabindex="0"><table>', h)
    h = h.replace('</table>', '</table></div>')
    h = postprocess_inline(h)
    # headings
    def head(m):
        lvl, inner = m.group(1), m.group(2)
        base = f'{chap}-{slug(inner)}'
        hid, k = base, 2
        while hid in ids:
            hid = f'{base}-{k}'
            k += 1
        ids.add(hid)
        anchor = f' <a class="heading-anchor" href="#{hid}" aria-label="Liên kết tới mục này">#</a>' if lvl in '23' else ''
        return f'<h{lvl} id="{hid}">{inner}{anchor}</h{lvl}>'
    h = re.sub(r'<h([2-4])>(.*?)</h\1>', head, h)
    # external links open in new tab
    h = re.sub(r'<a href="(https?://[^"]+)">', r'<a href="\1" target="_blank" rel="noopener noreferrer">', h)
    return h

# ---------------------------------------------------------------- diagrams
TYPE_LABEL = {'sequence': 'Sequence', 'flowchart': 'Flowchart', 'callgraph': 'Call graph', 'state': 'State',
              'class': 'Class / struct', 'architecture': 'Kiến trúc', 'graph': 'Quan hệ', 'timeline': 'Timeline'}
STATUS_LABEL = {'verified': ('badge-status-verified', 'Đã đối chiếu source'),
                'conceptual': ('badge-status-conceptual', 'Sơ đồ khái niệm'),
                'inferred': ('badge-status-inferred', 'Có phần suy luận')}
DIAGRAMS = {}
DIAGRAM_USE = {}

def diagram_html(did, chap, chapters_by_id):
    if did not in DIAGRAMS:
        raise SystemExit('unknown diagram ' + did)
    d = DIAGRAMS[did]
    if did in DIAGRAM_USE:
        raise SystemExit(f'diagram {did} used twice ({DIAGRAM_USE[did]}, {chap})')
    DIAGRAM_USE[did] = chap
    st = STATUS_LABEL[d['status']]
    notes = []
    if d.get('caption'):
        notes.append(f'<p><span class="label">Giải thích:</span> {md_inline(rewrite_links(d["caption"]))}</p>')
    if d.get('refs'):
        notes.append(f'<p><span class="label">File &amp; hàm:</span> {md_inline(d["refs"])}</p>')
    if d.get('next'):
        notes.append(f'<p><span class="label">Tiếp theo:</span> {md_inline(rewrite_links(d["next"]))}</p>')
    if d.get('explain') and d['explain'] not in chapters_by_id:
        warn(f'{did}: explain chapter {d["explain"]} missing')
    elif d.get('explain') and d['explain'] != chap:
        t = chapters_by_id[d['explain']]['title']
        notes.append(f'<p><span class="label">Giải thích chi tiết:</span> <a href="#{d["explain"]}">{html.escape(t)}</a></p>')
    src = d['src']
    if '</script' in src.lower():
        raise SystemExit('script end tag in diagram ' + did)
    return f'''<figure class="diagram-card" id="{did}" data-type="{d['type']}" data-status="{d['status']}">
<figcaption class="diagram-head">
<div class="diagram-title-row"><span class="diagram-title">{md_inline(d['title'])}</span><span class="badge">{TYPE_LABEL[d['type']]}</span><span class="badge {st[0]}" title="Trạng thái kiểm chứng">{st[1]}</span><code class="diagram-id">{did}</code></div>
<p class="diagram-purpose"><span class="label">Mục đích:</span> {md_inline(d['purpose'])}</p>
</figcaption>
<div class="diagram-canvas" tabindex="0" role="group" aria-label="Sơ đồ: {html.escape(re.sub(r'[`*]', '', d['title']))}"><div class="diagram-render"><div class="diagram-placeholder">Sơ đồ sẽ được vẽ khi mở chương (cần JavaScript).</div></div></div>
<script type="text/x-mermaid" class="diagram-source">
{src}</script>
<div class="diagram-notes">{''.join(notes)}</div>
</figure>'''

# ---------------------------------------------------------------- cards
CARDS = {}
CARD_USE = {}

def load_cards():
    for f in sorted(glob.glob(HERE + '/cards/*.card')):
        txt = open(f).read()
        for block in re.split(r'^=== ', txt, flags=re.M)[1:]:
            head, _, body = block.partition('\n---\n')
            lines = head.split('\n')
            c = {'id': lines[0].strip(), 'rows': []}
            for l in lines[1:]:
                if l.strip():
                    k, _, v = l.partition(':')
                    c[k.strip()] = v.strip()
            cur = None
            for l in body.rstrip().split('\n'):
                m = re.match(r'^([^:\s][^:]*?)::\s?(.*)$', l)
                if m:
                    cur = [m.group(1).strip(), m.group(2)]
                    c['rows'].append(cur)
                elif cur is not None:
                    cur[1] += '\n' + l
            if c['id'] in CARDS:
                raise SystemExit('dup card ' + c['id'])
            CARDS[c['id']] = c

def card_html(cid, chap):
    c = CARDS.get(cid)
    if not c:
        raise SystemExit('unknown card ' + cid)
    CARD_USE.setdefault(cid, []).append(chap)
    kind = c['kind']
    badge = {'fn': ('badge-fn', 'Hàm'), 'struct': ('badge-struct', 'Struct'), 'concept': ('badge-concept', 'Khái niệm'),
             'macro': ('badge-fn', 'Macro')}[kind]
    extra = ''
    if c.get('vendor') == 'yes':
        extra = '<span class="badge badge-vendor" title="Khác upstream v5.10.240">Vendor</span>'
    cls = {'fn': 'card-fn', 'macro': 'card-fn', 'struct': 'card-struct', 'concept': 'card-concept'}[kind]
    rows = ''.join(f'<dt>{html.escape(k)}</dt><dd>{postprocess(md_to_html(rewrite_links(v.strip())), chap, set())}</dd>'
                   for k, v in c['rows'])
    if kind == 'concept':
        name = f'<span class="card-title">{md_inline(c["name"])}</span>'
    else:
        name = f'<code class="card-name">{html.escape(c["name"])}</code>'
    loc = f'<span class="card-loc"><code class="path">{html.escape(c["loc"])}</code></span>' if c.get('loc') else ''
    sig = ''
    if c.get('sig') and '{' in c['sig']:   # struct: one member per line
        head, _, rest = c['sig'].partition('{')
        members = [m.strip() for m in rest.rsplit('}', 1)[0].split(';') if m.strip()]
        c = dict(c, sig=head.strip() + ' {\n' + ''.join('\t' + m + ';\n' for m in members) + '};')
    if c.get('sig'):
        sig = f'<div class="code-block sig" data-lang="c"><pre><code class="language-c">{html.escape(c["sig"])}</code></pre></div>'
    # a card used twice keeps its id only the first time
    idattr = f' id="{cid}"' if len(CARD_USE[cid]) == 1 else ''
    return f'<section class="card {cls}"{idattr}><div class="card-head"><span class="badge {badge[0]}">{badge[1]}</span>{extra}{name}{loc}</div>{sig}<dl>{rows}</dl></section>'

# ---------------------------------------------------------------- quiz
def quiz_html(fname, chap, ids, only=None):
    text = open(research_path(fname)).read()
    lines, secs = split_sections(text)
    q = [s for s in secs if s[2].startswith('Câu hỏi tự kiểm tra')]
    a = [s for s in secs if s[2].startswith('Đáp án')]
    if not q or not a:
        raise SystemExit('no quiz in ' + fname)
    def items(sec):
        body = '\n'.join(lines[sec[3] + 1:sec[4]])
        parts = re.split(r'^(\d+)\.\s', body, flags=re.M)
        res = {}
        for k in range(1, len(parts), 2):
            res[int(parts[k])] = parts[k + 1].strip()
        return res
    qs, ans = items(q[0]), items(a[0])
    if set(qs) != set(ans):
        warn(f'quiz mismatch in {fname}: {sorted(qs)} vs {sorted(ans)}')
    out = ['<ol class="quiz">']
    for n in sorted(qs):
        if only and n not in only:
            continue
        qh = md_inline(rewrite_links(qs[n]))
        ah = postprocess(md_to_html(rewrite_links(ans.get(n, '_(không có đáp án trong tài liệu gốc)_'))), chap, ids)
        out.append(f'<li value="{n}"><div class="quiz-q">{qh}</div><details class="collapsible quiz-a"><summary>Xem đáp án và giải thích</summary><div class="details-body">{ah}</div></details></li>')
    out.append('</ol>')
    return '\n'.join(out)

# ---------------------------------------------------------------- source tree
def tree_html():
    text = open(RESEARCH + '/SOURCE_TREE_MAP.md').read()
    lines, secs = split_sections(text)
    sec4 = [s for s in secs if s[1] == '4'][0]
    groups = []
    known = {}
    for lv, num, title, a, b in secs:
        if not num.startswith('4.') or lv != 3:
            continue
        rows = [l for l in lines[a + 1:b] if l.startswith('|')]
        if len(rows) < 3:
            continue
        hdr = [c.strip() for c in rows[0].strip('|').split('|')]
        grp = {'title': title, 'files': []}
        for r in rows[2:]:
            cells = [c.strip() for c in r.strip('|').split('|')]
            row = dict(zip(hdr, cells))
            first = cells[0]
            names = re.findall(r'`([^`]+)`', first)
            if not names:
                continue
            desc = row.get('Trách nhiệm') or row.get('Nội dung') or row.get('Vai trò') or ''
            pri = row.get('Ưu tiên & lý do') or row.get('Ưu tiên') or ''
            pm = re.search(r'P[012]', pri)
            prio = pm.group(0) if pm else ('P2' if 'n' == row.get('Eagle', '').strip() else 'P2')
            role = row.get('Vai trò', '')
            if role == desc or len(re.sub(r'[*`]', '', role)) > 16:   # long "Vai trò" is a description, not a role code
                role = ''
            grp['files'].append({'names': names, 'kconfig': row.get('Kconfig', ''), 'eagle': row.get('Eagle', '').strip(),
                                 'role': role, 'desc': desc, 'prio': prio, 'why': pri})
        groups.append(grp)
    # real directory listing
    real = sorted(os.listdir(KSRC + '/crypto'))
    covered = set()
    allrefs = ''
    for f in glob.glob(RESEARCH + '/**/*.md', recursive=True):
        allrefs += open(f).read()
    def depth(n):
        if re.search(r'crypto/' + re.escape(n) + r':\d', allrefs):
            return ('depth-detailed', 'Đọc chi tiết', 'Tài liệu trích dẫn số dòng trong file này')
        return ('', 'Khảo sát', 'Có trong bảng khảo sát, chưa đọc từng dòng')
    out = []
    total = 0
    for gi, g in enumerate(groups):
        items = []
        for f in g['files']:
            exist = []
            for n in f['names']:
                n2 = n.rstrip('/')
                if '*' in n2 or '…' in n2:
                    exist += [x for x in real if glob.fnmatch.fnmatch(x, n2)]
                elif n2 in real:
                    exist.append(n2)
            if not exist:
                continue
            for n in exist:
                covered.add(n)
            fid = 'file-' + slug(exist[0])
            d = depth(exist[0])
            cfg = f['eagle']
            cfgc = {'y': 'cfg-y', 'm': 'cfg-m', 'n': 'cfg-n'}.get(cfg[:1], '')
            namehtml = ', '.join(html.escape(x + ('/' if os.path.isdir(KSRC + '/crypto/' + x) else '')) for x in exist)
            tags = f'<span class="badge pri-{f["prio"]}" title="{html.escape(re.sub("[*`]", "", f["why"]))}">{f["prio"]}</span>'
            if cfg and cfg not in ('—', '-'):
                tags += f'<span class="badge {cfgc}" title="Giá trị trong Eagle_defconfig">Eagle: {html.escape(cfg)}</span>'
            if f['kconfig'] and f['kconfig'] not in ('—', '-'):
                tags += f'<code class="kconfig">{md_inline(f["kconfig"]) if "`" in f["kconfig"] else html.escape(f["kconfig"])}</code>'
            tags += f'<span class="badge {d[0]}" title="{d[2]}">{d[1]}</span>'
            if f['role']:
                tags += f'<span class="badge" title="Vai trò">{html.escape(re.sub("[*`]", "", f["role"]))}</span>'
            total += len(exist)
            items.append(f'<li class="tree-file" id="{fid}" data-priority="{f["prio"]}"><span class="f-name">crypto/{namehtml}</span><span class="f-desc">{md_inline(f["desc"])}</span><span class="f-tags">{tags}</span></li>')
        title = re.sub(r'^G[\d/]+ — ', '', g['title'])
        gm = re.match(r'^(G[\d/]+)', g['title'])
        out.append(f'<details{" open" if gi < 2 else ""}><summary><span class="grp-name">{html.escape(g["title"])}</span><span class="grp-count">{len(items)} mục</span></summary><ul>{"".join(items)}</ul></details>')
    rest = [x for x in real if x not in covered]
    if rest:
        items = []
        for n in rest:
            kind = 'Thư mục' if os.path.isdir(KSRC + '/crypto/' + n) else 'File'
            items.append(f'<li class="tree-file" id="file-{slug(n)}" data-priority="P2"><span class="f-name">crypto/{html.escape(n)}</span><span class="f-desc">{kind} có trong repo; chưa được mô tả riêng trong bảng khảo sát (thuộc nhóm thuật toán/hỗ trợ cùng loại).</span><span class="f-tags"><span class="badge pri-P2">P2</span><span class="badge" title="Chỉ có trong danh sách thư mục">Chỉ liệt kê</span></span></li>')
        total += len(rest)
        out.append(f'<details><summary><span class="grp-name">Các file còn lại trong crypto/</span><span class="grp-desc">liệt kê từ <code>ls crypto/</code></span><span class="grp-count">{len(rest)} mục</span></summary><ul>{"".join(items)}</ul></details>')
    toolbar = ('<div class="tree-toolbar" role="search"><label class="visually-hidden" for="tree-filter">Lọc file</label>'
               '<input id="tree-filter" type="search" placeholder="Lọc theo tên file, hàm, Kconfig…" autocomplete="off">'
               '<label class="visually-hidden" for="tree-priority">Mức ưu tiên</label><select id="tree-priority"><option value="">Mọi mức ưu tiên</option>'
               '<option value="P0">P0 — bắt buộc</option><option value="P1">P1 — quan trọng</option><option value="P2">P2 — mở rộng</option></select>'
               '<button type="button" class="text-btn" id="tree-expand">Mở tất cả</button><button type="button" class="text-btn" id="tree-collapse">Thu gọn</button>'
               '<span class="tree-count" id="tree-count" aria-live="polite"></span></div>')
    legend = ('<div class="legend"><span><span class="badge pri-P0">P0</span> bắt buộc để hiểu kiến trúc</span>'
              '<span><span class="badge pri-P1">P1</span> luồng chính</span><span><span class="badge pri-P2">P2</span> mở rộng</span>'
              '<span><span class="badge depth-detailed">Đọc chi tiết</span> tài liệu có trích số dòng</span>'
              '<span><span class="badge">Khảo sát</span> chỉ có mô tả trong bảng</span><span><span class="badge">Chỉ liệt kê</span> có trong <code>ls</code>, chưa mô tả</span></div>')
    return toolbar + legend + f'<div class="tree" data-total="{total}">' + ''.join(out) + '</div>', total, len(real)

# ---------------------------------------------------------------- glossary
def glossary_html():
    text = open(RESEARCH + '/GLOSSARY.md').read()
    rows = [l for l in text.split('\n') if l.startswith('|')]
    hdr = None
    terms = []
    for r in rows:
        cells = [c.strip() for c in r.strip().strip('|').split('|')]
        if hdr is None or (cells and cells[0].lower().startswith('thuật ngữ')) or cells[0] in ('Term',):
            hdr = cells
            continue
        if set(''.join(cells)) <= set('-: '):
            continue
        terms.append(dict(zip(hdr, cells)))
    return terms

def glossary_block():
    terms = glossary_html()
    hdr_keys = list(terms[0].keys()) if terms else []
    items = []
    seen = set()
    for t in terms:
        vals = list(t.values())
        en = re.sub(r'[*`]', '', vals[0])
        tid = 'term-' + slug(en)
        if tid in seen:
            tid += '-2'
        seen.add(tid)
        vi = md_inline(vals[1]) if len(vals) > 1 else ''
        body = ''.join(f'<p>{"<strong>" + html.escape(k) + ":</strong> " if k else ""}{md_inline(rewrite_links(v))}</p>' for k, v in list(t.items())[2:] if v)
        items.append(f'<div class="term" id="{tid}"><div class="term-head"><span class="term-en">{html.escape(en)}</span><span class="term-vi">{vi}</span></div><div class="term-body">{body}</div></div>')
    filt = ('<div class="glossary-filter" role="search"><label class="visually-hidden" for="glossary-filter">Lọc thuật ngữ</label>'
            '<input id="glossary-filter" type="search" placeholder="Lọc thuật ngữ (gõ không dấu cũng được)…" autocomplete="off">'
            f'<span class="tree-count" id="glossary-count" aria-live="polite">{len(items)} thuật ngữ</span></div>')
    return filt + '<div class="glossary-list">' + ''.join(items) + '</div>', len(items)

# ---------------------------------------------------------------- recipes
def load_recipes():
    recs = []
    for f in sorted(glob.glob(HERE + '/content/*.md')):
        txt = open(f).read()
        head, _, body = txt.partition('\n---\n')
        r = {'file': os.path.basename(f), 'body': body}
        for l in head.split('\n'):
            if l.strip():
                k, _, v = l.partition(':')
                r[k.strip()] = v.strip()
        recs.append(r)
        for s in r.get('source', '').split(','):
            s = s.strip()
            if s and not s.startswith('~') and s not in FILEMAP:
                FILEMAP[s] = r['id']
    return recs

def render_body(body, chap, chapters_by_id, ids, extras):
    blocks = []
    def ph(h):
        blocks.append(h)
        return f'\n\n@@BLOCK{len(blocks) - 1}@@\n\n'
    # includes first (they may contain nothing but markdown)
    body = re.sub(r'^\{\{include (.+?)\}\}\s*$', lambda m: include(m.group(1), chap), body, flags=re.M)
    # callout blocks (non-nested)
    def callout(m):
        kind, title, inner = m.group(1), m.group(2).strip(), m.group(3)
        inner_html = render_body(inner, chap, chapters_by_id, ids, extras)
        if kind == 'details':
            return ph(f'<details class="collapsible"><summary>{md_inline(title)}</summary><div class="details-body">{inner_html}</div></details>')
        if kind == 'concept':
            return ph(f'<section class="card card-concept"><div class="card-head"><span class="badge badge-concept">Khái niệm</span><span class="card-title">{md_inline(title)}</span></div>{inner_html}</section>')
        t = f'<div class="callout-title">{md_inline(title)}</div>' if title else ''
        return ph(f'<div class="callout callout-{kind}">{t}{inner_html}</div>')
    body = re.sub(r'^:::(warn|error|info|vendor|ok|concept|details)[ \t]*(.*?)\n(.*?)^:::[ \t]*$', callout, body, flags=re.M | re.S)
    body = re.sub(r'^\{\{diagram (\S+)\}\}\s*$', lambda m: ph(diagram_html(m.group(1), chap, chapters_by_id)), body, flags=re.M)
    body = re.sub(r'^\{\{card (\S+)\}\}\s*$', lambda m: ph(card_html(m.group(1), chap)), body, flags=re.M)
    body = re.sub(r'^\{\{quiz (\S+)(?: ([\d,]+))?\}\}\s*$', lambda m: ph(quiz_html(m.group(1), chap, ids,
                  [int(x) for x in m.group(2).split(',')] if m.group(2) else None)), body, flags=re.M)
    body = re.sub(r'^\{\{(tree|glossary|diagram-index)\}\}\s*$', lambda m: ph(extras[m.group(1)]()), body, flags=re.M)
    body = rewrite_links(body)
    h = postprocess(md_to_html(body), chap, ids)
    for i, b in enumerate(blocks):
        h = h.replace(f'<p>@@BLOCK{i}@@</p>', b).replace(f'@@BLOCK{i}@@', b)
    return h

def chapter_html(r, chapters_by_id, extras):
    chap = r['id']
    ids = set()
    body = render_body(r['body'], chap, chapters_by_id, ids, extras)
    meta = ''.join(f'<span>{md_inline(x.strip())}</span>' for x in r.get('meta', '').split('|') if x.strip())
    rel = [x.strip() for x in r.get('related', '').split(',') if x.strip()]
    for x in rel:
        if x not in chapters_by_id:
            warn(f'{chap}: related chapter {x} missing')
    rel = [x for x in rel if x in chapters_by_id]
    rel_html = ''
    if rel:
        rel_html = '<p class="related-title">Đọc tiếp / liên quan</p><ul class="related">' + ''.join(
            f'<li><a href="#{x}">{html.escape(chapters_by_id[x]["title"])}</a></li>' for x in rel) + '</ul>'
    src = ''
    if r.get('source'):
        src = '<p class="chapter-source">Nguồn nội dung: ' + ', '.join(
            f'<code class="path">docs/crypto-learning/{"chapters/" if re.match(r"^~?\d", s.strip()) else ""}{s.strip().lstrip("~")}</code>' for s in r['source'].split(',')) + '</p>'
    return f'''<article class="chapter" id="{chap}" data-title="{html.escape(r['title'])}" data-group="{html.escape(r['group'])}" tabindex="-1">
<header class="chapter-header">
<p class="chapter-kicker">{html.escape(r.get('kicker', r['group']))}</p>
<h1>{md_inline(r['title'])}</h1>
<p class="chapter-lead">{md_inline(r.get('lead', ''))}</p>
<div class="chapter-meta">{meta}</div>
</header>
<div class="chapter-body">
{body}
</div>
<footer class="chapter-footer">{rel_html}{src}<nav class="pager" aria-label="Chuyển chương"></nav></footer>
</article>'''

def diagram_index():
    rows = []
    for did, d in DIAGRAMS.items():
        st = STATUS_LABEL[d['status']]
        rows.append(f'<tr><td><a href="#{did}">{md_inline(d["title"])}</a></td><td>{TYPE_LABEL[d["type"]]}</td><td><span class="badge {st[0]}">{st[1]}</span></td><td><a href="#{DIAGRAM_USE.get(did, "")}">{html.escape(CH_TITLES.get(DIAGRAM_USE.get(did, ""), "—"))}</a></td></tr>')
    return ('<div class="table-wrap" tabindex="0"><table><thead><tr><th>Sơ đồ</th><th>Loại</th><th>Trạng thái</th><th>Chương</th></tr></thead><tbody>'
            + ''.join(rows) + '</tbody></table></div>')

CH_TITLES = {}

def main():
    for d in dglib.load(HERE + '/diagrams/*.dg'):
        if d['id'] in DIAGRAMS:
            raise SystemExit('dup diagram ' + d['id'])
        for k in ('type', 'status', 'title', 'purpose'):
            if not d.get(k):
                raise SystemExit(f'diagram {d["id"]} missing {k}')
        DIAGRAMS[d['id']] = d
    load_cards()
    recs = load_recipes()
    by_id = {r['id']: r for r in recs}
    for r in recs:
        CH_TITLES[r['id']] = r['title']
    tree_cache, glossary_cache = {}, {}
    def tree():
        h, n, nreal = tree_html()
        tree_cache['n'] = (n, nreal)
        return h
    def gloss():
        h, n = glossary_block()
        glossary_cache['n'] = n
        return h
    pending_index = []
    def dindex():
        pending_index.append(True)
        return '@@DIAGRAM-INDEX@@'
    extras = {'tree': tree, 'glossary': gloss, 'diagram-index': dindex}
    arts = [chapter_html(r, by_id, extras) for r in recs]
    # sidebar
    nav, group, n = [], None, 0
    for r in recs:
        if r['group'] != group:
            if group is not None:
                nav.append('</ol></div>')
            group = r['group']
            nav.append(f'<div class="nav-group"><p class="nav-group-title">{html.escape(group)}</p><ol class="nav-list">')
        nav.append(f'<li><a href="#{r["id"]}" data-chapter="{r["id"]}"><span class="nav-num">{n}</span><span class="nav-text">{html.escape(r.get("nav", r["title"]))}</span></a></li>')
        n += 1
    nav.append('</ol></div>')
    page = open(HERE + '/shell.html').read()
    content = '\n'.join(arts)
    if pending_index:
        content = content.replace('@@DIAGRAM-INDEX@@', diagram_index())
    page = page.replace('@@NAV@@', ''.join(nav)).replace('@@CONTENT@@', content)
    page = page.replace('@@NDIAGRAMS@@', str(len(DIAGRAM_USE))).replace('@@NCHAPTERS@@', str(len(recs)))
    unused = [d for d in DIAGRAMS if d not in DIAGRAM_USE]
    if unused:
        warn('unused diagrams: ' + ', '.join(unused))
    unused_c = [c for c in CARDS if c not in CARD_USE]
    if unused_c:
        warn('unused cards: ' + ', '.join(unused_c))
    open(OUT, 'w').write(page)
    json.dump({'diagrams': {k: {kk: v for kk, v in d.items() if kk != 'src'} | {'chapter': DIAGRAM_USE.get(k)} for k, d in DIAGRAMS.items()},
               'cards': {k: CARD_USE[k] for k in CARD_USE}, 'mermaid_dropped': MERMAID_DROPPED,
               'tree': tree_cache.get('n'), 'glossary': glossary_cache.get('n')},
              open(HERE + '/build-report.json', 'w'), ensure_ascii=False, indent=1)
    print(f'chapters={len(recs)} diagrams={len(DIAGRAM_USE)}/{len(DIAGRAMS)} cards={len(CARD_USE)} '
          f'tree={tree_cache.get("n")} glossary={glossary_cache.get("n")} bytes={len(page)}')
    for w in WARN:
        print('WARN', w)

if __name__ == '__main__':
    main()
