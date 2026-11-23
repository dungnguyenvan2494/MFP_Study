# dbgAllClear Deep Dive – tài liệu kỹ thuật tương tác

Tài liệu HTML phân tích lệnh debug `dbgAllClear`: NVRAM, SPI-Flash (SoftDipSW), 3 backup group, NVRAM Clear Flag và FactorySetup lúc boot. Mọi kết luận lấy từ source code của repository này và có nhãn mức bằng chứng `VERIFIED` / `INFERRED` / `UNVERIFIED`.

## Cách mở

Mở trực tiếp `index.html` bằng trình duyệt (Chrome, Edge, Firefox). Không cần web server.

```text
docs/dbgAllClear-documentation/index.html
```

## Cấu trúc thư mục

```text
dbgAllClear-documentation/
├── index.html                  # trang tài liệu (15 chương)
├── README.md
├── research/
│   └── dbgAllClear-deep-dive.md # báo cáo nghiên cứu dạng Markdown (nguồn nội dung)
└── assets/
    ├── css/style.css           # design system (light/dark, responsive)
    ├── js/main.js              # theme, sidebar, scroll-spy, TOC, code block, tabs, checklist, glossary
    ├── js/diagrams.js          # render Mermaid (lazy, đổi theme), phóng to sơ đồ
    ├── js/search.js            # tìm kiếm toàn văn theo heading
    ├── js/quiz.js              # 27 câu hỏi kiểm tra (3 cấp độ)
    └── vendor/                 # thư viện đóng gói sẵn để chạy offline
        ├── mermaid.min.js      # Mermaid 10.9.1 (MIT)
        ├── highlight.min.js    # highlight.js 11.9.0 (BSD-3-Clause)
        └── hljs-cpp.min.js
```

## Chức năng tương tác

| Chức năng | Cách dùng |
|---|---|
| Sidebar | Click mục để cuộn tới; thu gọn/mở nhóm; mục đang đọc được highlight. Trên mobile mở bằng nút ☰ (drawer). |
| Tìm kiếm | Ô tìm kiếm trên cùng (phím tắt `/`). Tìm theo tên hàm, macro, file path, khái niệm. `↑`/`↓`/`Enter` để chọn. |
| Light/Dark | Nút mặt trời/mặt trăng. Lựa chọn được nhớ trong trình duyệt. |
| Reading progress | Thanh tiến độ dưới top bar. |
| TOC bên phải | Heading của chương hiện tại (màn hình ≥ 1280px). |
| Zeus command tabs | Chương 06: tab Cmd 0–5 (hỗ trợ phím mũi tên), mỗi tab có flowchart riêng. |
| Sơ đồ | Mermaid render khi cuộn tới; nút “Phóng to” hoặc double-click để mở hộp thoại zoom (+/−/100%). |
| Code block | Highlight C/C++, số dòng thật trong source, dòng quan trọng được tô màu, nút Copy. |
| Accordion | Phần Failure Analysis. |
| Checklist debug | Tick từng bước; trạng thái lưu **cục bộ** trong localStorage của trình duyệt này (không đồng bộ). |
| Quiz | 27 câu, lọc theo cấp độ, chấm điểm, giải thích, Reset. |
| Glossary | Ô lọc thuật ngữ. |
| Back to top | Nút góc phải dưới. |

## Phụ thuộc và khả năng offline

- Mermaid và highlight.js nằm trong `assets/vendor`, nên sơ đồ và highlight vẫn chạy khi không có mạng.
- Font **Inter** và **JetBrains Mono** tải từ Google Fonts. Khi offline, trang tự dùng font hệ thống (`system-ui`, `ui-monospace`). Đây là phụ thuộc mạng duy nhất.
- Nếu `mermaid.min.js` không tải được, mỗi sơ đồ hiển thị mã nguồn Mermaid kèm thông báo thay vì để trống.

## Kiểm thử đã thực hiện

Bằng Chromium headless (Playwright), mở trực tiếp qua `file://`:

- Desktop 1440×900 (light, dark) và mobile 375×812 (light, dark): không có lỗi console/page error; 20/20 sơ đồ render thành SVG; không có cuộn ngang toàn trang ở 375px.
- Tìm kiếm trả kết quả và nhảy tới mục; tab command; quiz chấm điểm; đổi theme render lại cả 20 sơ đồ; hộp thoại zoom; checklist; drawer mobile.

Chưa kiểm thử: Safari/Firefox thật, tablet, chế độ in, đọc bằng screen reader.

## Lưu ý an toàn

`dbgAllClear` xoá dữ liệu không thể khôi phục. Không chạy trên thiết bị production. Trên Zeus, gọi không tham số tương đương command 0 (xoá nhiều nhất).
