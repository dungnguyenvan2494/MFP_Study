id: ch-about
title: Về tài liệu — kiểm chứng, giới hạn, chỉ mục sơ đồ
nav: Về tài liệu & sơ đồ
group: Tra cứu
kicker: Tra cứu · Phương pháp
lead: Nội dung trên website được kiểm chứng thế nào, những gì **chưa** được xác minh, và danh sách mọi sơ đồ kèm trạng thái.
meta: Trang tham chiếu
related: ch-overview, ch-roadmap
---
## Nguồn và phương pháp kiểm chứng

- Nội dung lấy từ bộ tài liệu nghiên cứu `docs/crypto-learning/` (27 file Markdown), vốn được viết
  bằng cách đọc trực tiếp `Kernel/Src/` và kiểm số dòng bằng script. Website **không sửa** các file đó;
  phần lớn văn bản được chèn nguyên đoạn (xem dòng "Nguồn nội dung" cuối mỗi chương).
- Mọi sơ đồ được **vẽ lại** cho website (sequence tách nhỏ, flowchart có nhánh lỗi riêng, call graph
  phân biệt gọi trực tiếp và gián tiếp). Trong lúc vẽ lại, từng bước được đối chiếu lại với source;
  414 vị trí `file:dòng` trong sơ đồ và thẻ hàm được kiểm tự động (`tools/site-src/refcheck.py`): file tồn
  tại, số dòng nằm trong file, và khi có tên hàm đi kèm thì tên đó xuất hiện trong đúng khoảng dòng.
- Các thẻ hàm/struct (chữ ký, vị trí, ngữ cảnh gọi, mã trả về) được viết mới từ source.
- Cả 91 sơ đồ được render bằng mermaid-cli với đúng phiên bản Mermaid và cấu hình của website (11.17.2),
  kiểm tự động chữ bị ngắt giữa định danh, note tràn khung và độ rộng; sau đó **xem ảnh từng sơ đồ** để
  tìm nhãn chồng nhau và cạnh cắt chéo. Trình duyệt Chromium (Playwright) cũng render cả 91 sơ đồ ở ba
  khổ màn hình mà không lỗi.

| Nhãn sơ đồ | Nghĩa |
|---|---|
| Đã đối chiếu source | Mỗi bước/nút tương ứng code có thật ở vị trí ghi trong phần "File & hàm" |
| Có phần suy luận | Cấu trúc đúng theo source, nhưng **lựa chọn implementation cụ thể** (vd `xts-aes-ce`, `sha256-ce`) phụ thuộc CPU và cần xác minh trên máy |
| Sơ đồ khái niệm | Mô hình giải thích, gom nhiều file hoặc lược bớt chi tiết |

## Những gì chưa được xác minh

Danh sách nội dung là **suy luận** [I] hoặc **giả thuyết** [H] — cần kiểm trên máy thật:

| Nội dung | Nhãn | Cách xác minh |
|---|---|---|
| CPU sản phẩm có ARMv8 AES / PMULL / SHA2 Crypto Extensions (quyết định `aes-ce`, `gcm-aes-ce`, `sha256-ce` có được đăng ký) | [H] | `grep Features /proc/cpuinfo` |
| Implementation được chọn cho `cbc(aes)`, `xts(aes)`, `gcm(aes)`, `hmac(sha256)` trên sản phẩm | [I] | `/proc/crypto` (cột driver, refcnt) |
| `stdrng` tốt nhất là `drbg_nopr_hmac_sha256` (priority 207) | [I] | `grep -A4 stdrng /proc/crypto` |
| SoC sản phẩm có engine SafeXcel hay không (DTS "quartz" không có node crypto) | [H] | DTS thật của board, `dmesg` |
| `crypto/` được đưa vào build bằng `core-y += crypto/` (repo không có Makefile gốc) | [I] | Makefile gốc của bản build thật |
| Thời gian mỗi dòng `@@@[CHK]timeout` ≈ 10 s (dựa trên `CONFIG_HZ=200` của Eagle_defconfig) | [I] | `.config` của bản build thật |
| Hoà priority: alg đăng ký sau thắng (suy ra từ `list_add` + so sánh `>`) | [I] | Thử nghiệm với hai alg cùng priority |
| Callback không được gọi với implementation đồng bộ (quy ước, core không ép) | [I] | Đọc implementation cụ thể |
| Bộ phát hiện hung-task có thể không cảnh báo khi kẹt trong `crypto_wait_req` bản vendor | [I] | Thử trên máy với `CONFIG_DETECT_HUNG_TASK` |
| Chất lượng entropy lúc boot (`RANDOM_TRUST_CPU`, `RANDOM_TRUST_BOOTLOADER`) | [I] | Không xác định được từ source |

Ngoài ra: không có `.config`/`vmlinux` của bản build thật, nên mọi khẳng định về **trạng thái lúc
chạy** (module đã nạp, alg đã đăng ký, alg được chọn) đều cần kiểm bằng lệnh ở chương
[Debugging](#ch-debugging).

## Khác biệt so với tài liệu nghiên cứu gốc

- Bốn chương gốc được tách để mỗi trang tập trung một chủ đề: 05 → *Đăng ký* + *Lookup*;
  06 → *Transform* + *Request*; 15 → *Memory* + *Concurrency* + *Security*; 16 → *Testing* + *Debugging*.
  Câu hỏi tự kiểm tra được chia theo chủ đề, giữ nguyên số thứ tự gốc.
- Toàn bộ sơ đồ Mermaid của tài liệu gốc được thay bằng sơ đồ thiết kế lại (xem chỉ mục dưới đây).
  Ví dụ: sơ đồ đăng ký gốc (một sequence dài có self-call) thành hai sequence + một flowchart;
  flowchart lookup gốc thành bốn sơ đồ; call graph dạng text được giữ trong khối thu gọn và bổ sung
  bằng call graph dạng hình.
- Sơ đồ "một trang ghi nhớ" của chương 18 không được vẽ lại; nội dung của nó nằm trong các sơ đồ kiến trúc.
- Thẻ hàm/struct là nội dung mới, viết từ source.

## Chỉ mục sơ đồ

Bấm tên sơ đồ để mở đúng chương và cuộn tới sơ đồ.

{{diagram-index}}
