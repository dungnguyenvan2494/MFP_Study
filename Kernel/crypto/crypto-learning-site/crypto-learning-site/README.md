# Website học Linux Kernel Crypto API (5.10)

Website tĩnh trình bày bộ tài liệu nghiên cứu `docs/crypto-learning/` về thư mục `Kernel/Src/crypto/`
của repo Source_MFP (Linux 5.10, baseline v5.10.240 + 4 file vendor): 28 chương, 91 sơ đồ Mermaid
vẽ lại, 33 thẻ hàm/struct, cây source tương tác (147 mục lấy từ `ls crypto/` thật), 96 thuật ngữ.

- Báo cáo kiểm tra: [QA_REPORT.md](QA_REPORT.md)
- Danh mục sơ đồ: [diagrams/README.md](diagrams/README.md)
- Nội dung chưa được xác minh: chương **"Về tài liệu & sơ đồ"** trong website (`#ch-about`) và mục 5 của QA_REPORT.

## 1. Mở website

**Chạy hoàn toàn offline**, không cần cài gì và không cần server: mở `index.html` bằng trình duyệt
(Chrome/Edge/Firefox/Safari bản gần đây).

```bash
xdg-open docs/crypto-learning-site/index.html        # Linux
open docs/crypto-learning-site/index.html            # macOS
start docs\crypto-learning-site\index.html           # Windows
```

Muốn mở qua HTTP (ví dụ để chia sẻ trong mạng nội bộ) thì một server tĩnh bất kỳ là đủ:

```bash
cd docs/crypto-learning-site && python3 -m http.server 8000   # rồi mở http://localhost:8000/
```

Website không gửi request ra ngoài: mọi thư viện được đặt sẵn trong `assets/vendor/` (đã kiểm bằng
Playwright — 0 request ngoài `file://`). Liên kết ngoài (nếu có trong nội dung) chỉ mở khi bạn bấm.

Đường dẫn trong một chương là URL hash (`index.html#ch-lookup`, `index.html#dg-lk-select`,
`index.html#fn-crypto_alloc_skcipher`) nên có thể gửi link thẳng tới một mục, sơ đồ hoặc thẻ hàm.

## 2. Tính năng

| Tính năng | Cách dùng | Ghi chú |
|---|---|---|
| Mục lục chương (trái) | Nút ☰; trên điện thoại là ngăn kéo | Nhóm theo chủ đề |
| Mục lục trong chương (phải) | Tự cập nhật mục đang đọc; nút ≡ trên màn hẹp | |
| Tìm kiếm | Phím `/` hoặc ô tìm kiếm; ↑ ↓ Enter, Esc | Tìm chương, mục, hàm, struct, file, thuật ngữ, nội dung và **mã nguồn sơ đồ**; gõ không dấu được |
| Sơ đồ | *Phóng to* (kéo để di chuyển, `+`/`−`, *Vừa khung*, Esc), *Sao chép Mermaid*, *Xem source* | Sơ đồ quá rộng: thu vừa khung nếu chữ còn đọc được, ngược lại cuộn ngang; trên điện thoại luôn thu vừa khung |
| Code | Nút *Copy* trên mỗi khối code; tô màu cú pháp C/bash/Makefile | |
| Cây source | Lọc theo tên/hàm/Kconfig, lọc theo mức ưu tiên, mở/thu gọn | Phân biệt *Đọc chi tiết* / *Khảo sát* / *Chỉ liệt kê* |
| Thuật ngữ | Ô lọc trong chương Thuật ngữ | |
| Câu hỏi tự kiểm tra | Đáp án thu gọn dưới mỗi câu; nút "Mở tất cả" đầu chương | |
| Giao diện sáng/tối | Nút mặt trăng; nhớ lựa chọn trong trình duyệt | Khung sơ đồ luôn nền sáng để giữ độ tương phản |
| Tiến độ | Thanh bên trái | Chỉ đếm số chương **đã mở** trên trình duyệt này (localStorage); không đo mức hiểu; nút "Đặt lại" |
| In | Nút máy in, hoặc Ctrl+P | In chương đang xem; khối thu gọn được mở tạm khi in; ẩn thanh điều hướng |
| Tắt JavaScript | — | Mọi chương hiển thị liên tiếp; sơ đồ không được vẽ (source vẫn có trong HTML); tìm kiếm/zoom/copy không hoạt động |

Không có file PDF đi kèm. Có thể "In → Lưu thành PDF" từ trình duyệt cho từng chương.

## 3. Cấu trúc thư mục

```text
docs/crypto-learning-site/
├── index.html                 # toàn bộ nội dung (28 <article class="chapter">), sinh bởi tools/site-src/build.py
├── assets/
│   ├── css/styles.css         # design system: token màu, layout 3 cột, component, responsive, print
│   ├── js/util.js             # fold dấu tiếng Việt, escape, toast, copy, localStorage an toàn
│   ├── js/mermaid-config.js   # cấu hình Mermaid DUY NHẤT (trình duyệt và mermaid-cli dùng chung)
│   ├── js/diagrams.js         # render tuần tự, toolbar, thu vừa khung, modal phóng to, báo lỗi
│   ├── js/search.js           # chỉ mục tìm kiếm dựng từ DOM lúc tải + giao diện kết quả
│   ├── js/app.js              # router theo hash, sidebar/TOC, breadcrumb, pager, theme, tiến độ, lọc, in
│   └── vendor/                # thư viện bên thứ ba (bản gốc, không sửa) + LICENSE
├── diagrams/README.md         # danh mục 91 sơ đồ (sinh tự động)
├── QA_REPORT.md               # báo cáo kiểm tra cuối
└── tools/                     # mã nguồn soạn thảo — KHÔNG cần để xem website
    ├── site-src/build.py      # dựng index.html
    ├── site-src/shell.html    # khung trang (topbar, sidebar, modal…)
    ├── site-src/content/*.md  # 28 "recipe" chương
    ├── site-src/diagrams/*.dg # 91 sơ đồ + metadata
    ├── site-src/cards/*.card  # 33 thẻ hàm/struct
    ├── site-src/{refcheck,notecheck,inventory,widen_notes}.py, checkdg.sh, render.sh
    └── qa/*.js                # kiểm tra bằng Playwright
```

## 4. Thư viện phụ thuộc

| Thư viện | Phiên bản | File | SHA-256 | Giấy phép | Nguồn |
|---|---|---|---|---|---|
| Mermaid | 11.17.2 | `assets/vendor/mermaid/mermaid.min.js` | `581ed7d74bd9048d0e3a91363927d72ef22942d7722546b27f7cc29e35390eb8` | MIT (`assets/vendor/mermaid/LICENSE`) | npm `mermaid@11.17.2`, `dist/mermaid.min.js` |
| highlight.js | 11.11.1 (bản *common* gồm C, bash, Makefile, ini, plaintext…) | `assets/vendor/highlight/highlight.min.js` | `c4a399dd6f488bc97a3546e3476747b3e714c99c57b9473154c6fb8d259b9381` | BSD-3-Clause (`assets/vendor/highlight/LICENSE`) | npm `@highlightjs/cdn-assets@11.11.1`, `highlight.min.js` |

Không dùng CDN, font web hay framework. JavaScript của site là ES5 thuần, không cần build.

Công cụ chỉ dùng khi **soạn lại** website (không cần để xem): Python 3 + `markdown` 3.7 (build),
Node.js + `@mermaid-js/mermaid-cli` 11.4.2 (kiểm sơ đồ), Playwright 1.56 + Chromium (QA).

## 5. Chỉnh sửa nội dung

Có hai cách. **Cách B được khuyến nghị** vì giữ nhất quán (id, mục lục, chỉ mục sơ đồ) và chạy được các bước kiểm tra.

### Cách A — sửa trực tiếp `index.html`

Mỗi chương là một `<article class="chapter" id="ch-…" data-title="…" data-group="…">`. Sửa văn bản
trong đó là xong; mục lục phải, breadcrumb, nút trước/sau, tìm kiếm đều được dựng lại từ DOM khi tải
trang. Lưu ý: lần chạy `build.py` sau sẽ **ghi đè** các sửa đổi thủ công này.

### Cách B — sửa nguồn trong `tools/site-src/` rồi build lại

```bash
cd docs/crypto-learning-site/tools/site-src
python3 build.py            # ghi ../../index.html, in số chương/sơ đồ/thẻ và mọi cảnh báo
python3 refcheck.py         # kiểm mọi `file:dòng` trong sơ đồ và thẻ với Kernel/Src
python3 notecheck.py        # tìm note sequence quá dài cho khung một participant
```

`build.py` dừng với thông báo rõ ràng nếu có sơ đồ/thẻ không tồn tại, sơ đồ dùng hai lần, hay
include sai mục; nó cảnh báo link chưa ánh xạ, chương liên quan không tồn tại, sơ đồ/thẻ không dùng.

| Muốn… | Làm |
|---|---|
| **Thêm chương** | Tạo `content/NN-ten.md` (NN quyết định thứ tự). Phần đầu là các dòng `khoá: giá trị` (`id`, `title`, `nav`, `group`, `kicker`, `lead`, `meta` — các ý cách nhau bằng `\|`, `source` — file tài liệu gốc, thêm `~` phía trước nếu chỉ hiển thị, `related` — danh sách id chương), sau dòng `---` là Markdown. Nhóm (`group`) mới tự xuất hiện trong thanh bên. |
| Chèn đoạn từ tài liệu gốc | `{{include 07-skcipher.md#3.4}}` (chỉ thân mục) hoặc `{{include 07-skcipher.md#3.4 as ###}}` (giữ tiêu đề ở cấp ###). Mục chọn theo số hoặc tiền tố tiêu đề (dấu cách viết là `_`). Sơ đồ Mermaid gốc bị bỏ; link sang file `.md` được đổi thành link chương. |
| **Thêm thẻ hàm/struct** | Thêm khối vào `cards/*.card`: dòng `=== fn-ten`, các dòng `kind: fn\|struct\|concept`, `name:`, `loc: crypto/file.c:A-B`, `sig:`, `vendor: yes` (tuỳ chọn), rồi `---` và các dòng `Nhãn:: nội dung Markdown`. Chèn bằng `{{card fn-ten}}`. Chạy `refcheck.py` để kiểm `loc`. |
| **Thêm sơ đồ** | Thêm khối vào `diagrams/*.dg`: `=== dg-ten`, `type:` (sequence/flowchart/callgraph/state/class/architecture/graph), `status:` (verified/conceptual/inferred), `title:`, `purpose:`, `caption:`, `refs:`, `explain:`/`next:` (tuỳ chọn), `---`, rồi Mermaid. Trong flowchart viết `%% @classdefs` để dùng các lớp màu `api core impl drv err ok dec cb ext`. Chèn bằng `{{diagram dg-ten}}` (mỗi sơ đồ một lần). Kiểm: `./checkdg.sh "diagrams/xx-*.dg"` rồi `PNG=1 ./render.sh <id…>` (cần `MMDC=/đường/dẫn/mmdc`, và `PUPPETEER_CFG` nếu Chromium không tự tìm thấy) và **xem ảnh** trong `tools/.work/`. |
| Khối nhấn mạnh | `:::warn Tiêu đề` … `:::` (cũng có `error`, `info`, `vendor`, `ok`, `concept`, `details` = khối thu gọn) |
| Câu hỏi tự kiểm tra | `{{quiz 07-skcipher.md}}` hoặc chỉ một số câu: `{{quiz 15-memory-concurrency-security.md 2,3,5}}` |
| **Thêm thuật ngữ** | Thêm dòng vào bảng trong `docs/crypto-learning/GLOSSARY.md` rồi build lại (chương Thuật ngữ đọc trực tiếp file đó). |
| **Đổi màu / font** | Sửa token trong `:root` và `:root[data-theme="dark"]` ở đầu `assets/css/styles.css`. Màu trong sơ đồ: `themeVariables` trong `assets/js/mermaid-config.js` và `CLASSDEFS` trong `tools/site-src/dglib.py`. |
| **Đổi menu / thứ tự chương** | Đổi tiền tố số hoặc `group`/`nav` trong `content/*.md`; menu được sinh lại. |
| Đổi cấu hình Mermaid | Chỉ sửa `assets/js/mermaid-config.js` — trình duyệt và `checkdg.sh` cùng đọc file này. |
| Cập nhật danh mục sơ đồ | `./render.sh <tất cả id> > ../.work/render.txt` rồi `python3 inventory.py ../.work/render.txt`. |

Quy tắc vẽ sơ đồ đã áp dụng (nên giữ khi thêm mới): sequence luôn `autonumber`, ≤ 5 participant,
không self-call (dùng `Note`), nhãn ngắn và xuống dòng thủ công bằng `<br/>` sau `(`, note dài trải
trên hai participant; flowchart từ trên xuống, quyết định là hình lục giác `{{"…"}}`, mỗi nhánh lỗi có
nút kết thúc riêng; call graph: mũi tên liền = gọi trực tiếp, mũi tên đứt có nhãn = con trỏ hàm /
notifier / kthread / workqueue; state diagram chỉ dùng cho trạng thái có thật trong code.

## 6. Kiểm tra lại (QA)

```bash
cd docs/crypto-learning-site/tools/qa
node full.js 1440 900 light     # mọi chương: lỗi console, sơ đồ lỗi, request ngoài, id trùng, link hỏng, tràn ngang
node full.js 390 844 light      # khổ điện thoại
node interact.js                # 36 kiểm tra tương tác: tìm kiếm, modal, copy, theme, lọc, in, drawer mobile…
```

Kết quả và ảnh chụp ghi vào `tools/.work/qa/` (không commit). Kết quả lần chạy cuối: [QA_REPORT.md](QA_REPORT.md).
