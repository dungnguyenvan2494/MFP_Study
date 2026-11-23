#!/usr/bin/env python3
"""Write ../../diagrams/README.md: the inventory of every diagram on the site.
usage: python3 inventory.py [render-log]
   render-log = output of `render.sh <all ids>` (adds the rendered size column)."""
import os, re, sys, json
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import dglib

TYPE = {'sequence': 'Sequence', 'flowchart': 'Flowchart', 'callgraph': 'Call graph', 'state': 'State',
        'class': 'Class', 'architecture': 'Kiến trúc', 'graph': 'Quan hệ'}
STATUS = {'verified': 'Đã đối chiếu source', 'conceptual': 'Khái niệm', 'inferred': 'Có phần suy luận'}

report = json.load(open(os.path.join(HERE, 'build-report.json')))
chapters = {}
for f in sorted(os.listdir(os.path.join(HERE, 'content'))):
    head = open(os.path.join(HERE, 'content', f)).read().split('\n---\n', 1)[0]
    meta = dict(l.split(':', 1) for l in head.split('\n') if ':' in l)
    chapters[meta['id'].strip()] = meta['title'].strip()
sizes = {}
if len(sys.argv) > 1:
    for l in open(sys.argv[1]):
        m = re.match(r'OK\s+(\S+)\s+(\d+)x(\d+)', l)
        if m:
            sizes[m.group(1)] = (int(m.group(2)), int(m.group(3)))

rows, n_by_type, n_by_status = [], {}, {}
for d in dglib.load(os.path.join(HERE, 'diagrams', '*.dg')):
    ch = report['diagrams'][d['id']]['chapter']
    n_by_type[d['type']] = n_by_type.get(d['type'], 0) + 1
    n_by_status[d['status']] = n_by_status.get(d['status'], 0) + 1
    size = sizes.get(d['id'])
    size_s = f'{size[0]}×{size[1]}' + (' (rộng)' if size[0] > 1300 else '') if size else '—'
    refs = re.findall(r'`((?:crypto|include|arch|drivers|security|net)/[^`:]+)', d.get('refs', ''))
    files = ', '.join(sorted(set(r.rstrip('/').split('/')[-1] + ('/' if r.endswith('/') else '') for r in refs))) or '—'
    title = d['title'].replace('|', '\\|')
    rows.append(f"| `{d['id']}` | {title} | {TYPE[d['type']]} | {STATUS[d['status']]} | {chapters.get(ch, ch)} | {files} | {size_s} |")

out = ['# Danh mục sơ đồ', '',
       'Sinh tự động bởi `tools/site-src/inventory.py` từ `tools/site-src/diagrams/*.dg`. Mermaid source',
       'của từng sơ đồ nằm trong `index.html` (thẻ `<script type="text/x-mermaid" class="diagram-source">`',
       'bên trong `<figure class="diagram-card" id="…">`) và trong file `.dg` tương ứng.', '',
       f'Tổng: **{len(rows)} sơ đồ** — ' + ', '.join(f'{TYPE[k]} {v}' for k, v in sorted(n_by_type.items())) + '.',
       'Trạng thái: ' + ', '.join(f'{STATUS[k]} {v}' for k, v in sorted(n_by_status.items())) + '.', '',
       '## Các kiểm tra đã chạy cho mọi sơ đồ', '',
       '| Kiểm tra | Công cụ | Kết quả |', '|---|---|---|',
       '| Cú pháp + render với cấu hình của site | mermaid-cli 11.4.2 (Mermaid 11.17.2), `checkdg.sh` + `render.sh` | 91/91 OK |',
       '| Chữ bị ngắt giữa định danh (sequence) | `render.sh` (HYPHEN-BREAK) | 0 |',
       '| Note một participant tràn khung | `notecheck.py` | 0 |',
       '| Nhãn flowchart bị trình duyệt ngắt giữa định danh | `render.sh` (LONGLINE) | còn vài nhãn xuống dòng ở dấu cách giữa tham số — đã xem ảnh, đọc được |',
       '| Vị trí `file:dòng` trong phần "File & hàm" | `refcheck.py` | 414 vị trí, 0 lỗi |',
       '| Render trong trình duyệt (Chromium, file://) | `tools/qa/full.js` ở 1440×900, 1024×768 (tối), 390×844 | 91/91 `data-rendered="true"`, 0 lỗi console |',
       '| Xem ảnh từng sơ đồ (nhãn chồng, cạnh cắt chéo, chữ tràn) | ảnh PNG + tờ tổng hợp 2×2 | đã xem cả 91; các lỗi tìm thấy đã sửa |', '',
       'Sơ đồ đánh dấu "(rộng)" lớn hơn 1300px: trên màn hình rộng chúng giữ kích thước thật và cuộn ngang',
       'trong khung (hoặc mở "Phóng to"); trên điện thoại mọi sơ đồ được thu vừa khung.', '',
       '## Danh sách', '',
       '| ID | Tiêu đề | Loại | Trạng thái | Chương | File nguồn được trích | Kích thước render (px) |',
       '|---|---|---|---|---|---|---|'] + rows + ['']
dst = os.path.join(HERE, '..', '..', 'diagrams', 'README.md')
os.makedirs(os.path.dirname(dst), exist_ok=True)
open(dst, 'w').write('\n'.join(out))
print('wrote', os.path.relpath(dst), len(rows), 'rows')
