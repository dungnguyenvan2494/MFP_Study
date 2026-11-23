# Báo cáo kiểm tra (QA) — website Linux Kernel Crypto API

Ngày chạy: 2026-10-13. Tất cả số liệu dưới đây là kết quả của lần chạy **cuối cùng**, trên đúng
`index.html` được commit. Cách chạy lại: README mục 6.

## 1. Môi trường kiểm tra

| Thành phần | Phiên bản / ghi chú |
|---|---|
| Trình duyệt | Chromium (bản đi kèm Playwright 1.56.1), headless, mở site qua `file://` |
| Render sơ đồ ngoài trình duyệt | `@mermaid-js/mermaid-cli` 11.4.2 dùng Mermaid 11.17.2 — **cùng bản** với `assets/vendor/mermaid` — và cùng `assets/js/mermaid-config.js` |
| Kiểm tham chiếu source | `tools/site-src/refcheck.py` trên `Kernel/Src/` của repo |
| Kiểm trực quan | Ảnh chụp từ Chromium headless và ảnh PNG từ mermaid-cli, được xem từng ảnh |

**Không** kiểm: Firefox, Safari, trình duyệt di động thật, thiết bị cảm ứng thật, trình đọc màn hình,
máy in thật. Khổ điện thoại được giả lập (390×844, `isMobile`, `hasTouch`, DPR 2) trong Chromium.

## 2. Kết quả tự động

| # | Kiểm tra | Kết quả |
|---|---|---|
| 1 | Build `index.html` (`build.py`) | 28 chương, 91/91 sơ đồ được dùng, 33 thẻ, cây 147/147 mục của `ls crypto/`, 96 thuật ngữ, **0 cảnh báo** |
| 2 | Tham chiếu `file:dòng` trong sơ đồ và thẻ (`refcheck.py`) | 414 tham chiếu, **0 lỗi** (file tồn tại, dòng trong phạm vi, tên hàm nằm trong khoảng dòng khi được nêu); đã thử ca âm để chắc script bắt được lỗi |
| 3 | Cú pháp + render mọi sơ đồ trích từ `index.html` (mermaid-cli) | **91/91 OK**, 0 chữ bị ngắt giữa định danh, 0 note tràn khung |
| 4 | Toàn site ở 1440×900 (sáng), 1024×768 (tối), 390×844 (sáng) — `tools/qa/full.js` | Mỗi khổ: 28/28 chương mở được, **91/91 sơ đồ `data-rendered="true"`**, **0 lỗi/cảnh báo console**, **0 request ra ngoài `file://`**, 0 id trùng, 0 link `#…` hỏng, 0 phần tử tràn ngang, mọi chương có mục lục phải; chỉ mục tìm kiếm 2802 mục; tải trang ≈ 1,1–1,4 s |
| 5 | Tương tác — `tools/qa/interact.js` | **36/36 đạt** (bảng dưới) |
| 6 | Tắt JavaScript — `tools/qa/nojs.js` | 28/28 chương hiển thị, có thông báo `noscript`, 91 ô sơ đồ hiện chú thích thay thế |
| 7 | In — `tools/qa/printcheck.js` + `interact.js` | Media print ẩn topbar/sidebar/TOC/toolbar; khối thu gọn được mở trước khi in và khôi phục sau; chữ ký struct không tràn; Chromium xuất được PDF của một chương (chỉ để kiểm, **không** có PDF trong sản phẩm) |

Các kiểm tra tương tác (#5):

| Nhóm | Kiểm tra (đều đạt) |
|---|---|
| Tìm kiếm | phím `/` focus ô tìm; "larval wait" → 33 kết quả có đánh dấu; ↓ + Enter điều hướng tới thẻ `fn-crypto_larval_wait`; gõ không dấu "dang ky" → 40 kết quả; tìm driver name "gcm-aes-ce"; tìm ra thẻ hàm `crypto_req_done`; Esc đóng |
| Sơ đồ | Phóng to mở modal; nút + đổi tỉ lệ (104% → 130%); phím `−` thu nhỏ; kéo chuột di chuyển nội dung; Esc đóng; focus trở lại nút bấm; *Sao chép Mermaid* → toast + clipboard chứa `sequenceDiagram…`; *Xem source* hiện mã và đặt `aria-expanded="true"` |
| Code | nút Copy → "Đã copy" và clipboard đúng nội dung; tô màu cú pháp đã áp dụng |
| Điều hướng | mục lục phải có mục active sau khi bấm; breadcrumb hiện mục; nút "Chương sau"; link sâu tới id sơ đồ mở đúng chương; "Mở tất cả mục thu gọn" mở 5/5 |
| Giao diện | đổi sáng/tối và giữ sau khi tải lại; tiến độ hiển thị "x/28 chương đã mở" |
| Lọc | cây source theo chữ ("cryptd" → 2 file) và theo ưu tiên (P0 → 10 file); thuật ngữ ("larval" → 2) |
| Điện thoại | sidebar ẩn mặc định; mở ngăn kéo; chọn chương thì ngăn kéo đóng và chuyển chương; mở TOC; mở modal sơ đồ |
| Chung | không có lỗi JavaScript trong toàn bộ lượt tương tác |

## 3. Kiểm trực quan

- **Sơ đồ**: đã xem ảnh của **cả 91 sơ đồ** (từng ảnh hoặc tờ tổng hợp 2×2). Lỗi tìm thấy và đã sửa
  trong quá trình này gồm: note trên một participant tràn khung (25 note được trải sang hai
  participant), chữ bị ngắt giữa định danh (`crypto_skcipher_enc-…`, `SYNC_SKCIPHER_REQUEST_ON_STACK(re-…`,
  `cbc(aes-` / `ce)`), self-call trong sequence (đổi thành `Note`), self-loop chồng nhãn trong state
  diagram transform (đổi thành mô tả trạng thái), cạnh cắt qua nhãn nhóm (dg-arch-tmpl), cạnh cắt
  chéo (sơ đồ kiến trúc tổng quan, bản đồ nhóm file), subgraph bị xếp dọc ngoài ý muốn.
- **Trang**: đã xem ảnh chụp trang Tổng quan (desktop), chương Request (điện thoại), ngăn kéo điện
  thoại, sơ đồ trên điện thoại (thu vừa khung), modal trên điện thoại, kết quả tìm kiếm, cây source,
  thẻ sơ đồ ở giao diện tối, chế độ in.
- Sửa sau khi xem trang: dòng file trong cây source lệch cột; nhãn "vai trò" dài thành badge tràn
  ngang; sơ đồ rộng trên điện thoại chỉ hiện một cột (nay thu vừa khung); code ngắn như `-EINPROGRESS`
  bị xuống dòng sau dấu `-`; chữ ký struct một dòng quá dài; khối thu gọn không được in.

## 4. Hạn chế đã biết

| Hạn chế | Ảnh hưởng | Cách dùng |
|---|---|---|
| 5 sơ đồ rộng hơn 1300 px (`dg-fund-choose`, `dg-dbg-enoent`, `dg-sk-seq-template`, `dg-hw-cryptd-complete`, `dg-kc-autoload`) | Trên màn hình rộng phải cuộn ngang trong khung | Nút *Phóng to* → *Vừa khung* |
| Sơ đồ trên điện thoại được thu nhỏ để thấy toàn cảnh | Chữ nhỏ | Mở *Phóng to* rồi phóng to/kéo |
| Ô tìm kiếm trên màn hình rất hẹp chỉ hiện chữ "Tìm" | Placeholder bị cắt | Vẫn gõ và tìm bình thường |
| Clipboard khi mở qua `file://` | Một số trình duyệt chặn Clipboard API ngoài HTTPS; site tự chuyển sang `execCommand('copy')`; nếu cả hai bị chặn sẽ hiện thông báo yêu cầu chọn văn bản thủ công | Mở qua `http://localhost` nếu cần |
| Tiến độ đọc | Chỉ đếm chương đã mở, lưu trong trình duyệt hiện tại | — |
| `index.html` ≈ 1 MB, `mermaid.min.js` ≈ 3,5 MB | Lần mở đầu tiên tải ~4,7 MB từ đĩa | Sơ đồ chỉ được vẽ khi mở chương chứa nó |
| Khung sơ đồ luôn nền sáng ở giao diện tối | Chủ ý, để giữ độ tương phản của màu Mermaid | — |
| Không có bản PDF | — | In → Lưu thành PDF từ trình duyệt |

## 5. Nội dung chưa được xác minh

Toàn bộ danh sách nằm trong chương **"Về tài liệu & sơ đồ"** của website (`index.html#ch-about`),
tóm tắt:

- CPU sản phẩm có ARMv8 AES/PMULL/SHA2 Crypto Extensions hay không **[H]** — quyết định `aes-ce`,
  `gcm-aes-ce`, `sha256-ce` có được đăng ký; vì vậy implementation cụ thể được chọn cho `cbc(aes)`,
  `xts(aes)`, `gcm(aes)`, `hmac(sha256)`, `stdrng` là **suy luận** **[I]**.
- SoC sản phẩm có engine SafeXcel hay không **[H]**; `crypto/` vào build bằng `core-y` **[I]**
  (repo không có Makefile gốc); thời gian mỗi dòng `@@@[CHK]timeout` ≈ 10 s dựa trên `CONFIG_HZ=200`
  của defconfig **[I]**.
- Mọi khẳng định về trạng thái **lúc chạy** cần xác minh trên máy thật bằng `/proc/crypto`,
  `/proc/cpuinfo`, `dmesg` (lệnh trong chương Debugging).
- 5 sơ đồ mang nhãn "Có phần suy luận" vì lý do trên; 11 sơ đồ là "Sơ đồ khái niệm".
