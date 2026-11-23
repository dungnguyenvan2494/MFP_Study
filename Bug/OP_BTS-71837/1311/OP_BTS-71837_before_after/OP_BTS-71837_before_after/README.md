# OP_BTS-71837: log chẩn đoán, thư mục before / after

```
OP_BTS-71837_before_after/
  before/   9 file nguyên bản từ repo (đã so khớp từng byte với HEAD)
  after/    9 file đã thêm log (đúng bằng kết quả áp patch lên before/)
  OP_BTS-71837_diag_logging.patch                 patch dùng cho git apply
  OP_BTS-71837_diag_logging_before_after.html     trang xem song song, có "In ra" và "Cách đọc"
```

Đường dẫn bên trong before/ và after/ giống hệt đường dẫn trong repo (gốc là thư mục `Source_MFP`).

## So sánh
- Beyond Compare / WinMerge / Meld: so sánh thư mục `before` với thư mục `after`.
- Dòng lệnh: `diff -ru before after`
- Mã hóa: các file là Shift-JIS (comment tiếng Nhật). Mở bằng chế độ CP932, không đổi mã hóa khi lưu.

## Áp dụng vào repo
- Cách 1 (khuyến nghị): tại gốc repo, `git apply OP_BTS-71837_diag_logging.patch`
- Cách 2: chép từng file trong `after/` đè lên file cùng đường dẫn trong repo.
- Gỡ bỏ: `git apply -R OP_BTS-71837_diag_logging.patch`, hoặc chép lại file trong `before/`.

## Lưu ý
- Phần kernel CHƯA được biên dịch (môi trường tạo bản này không có toolchain). Hãy build trước khi nạp.
- Mọi dòng log mới có tiền tố `OP_BTS-71837:` (lọc bằng grep).
- `HW_interim.cpp` ở đây là bản chưa có 69673 (vòng 60 lần sleep). Nếu máy chạy bản có 69673, chuyển chỉnh sửa vào `if(0!=system("mount /dev/sdc1 /bd/sdc1"))`.
- Script Writer không nằm trong repo nên chưa có chỉnh sửa.
- Ảnh hưởng đến hệ thống: console là UART 115200 (kernel 5.10, printk đồng bộ), mỗi dòng cảnh báo mất khoảng 10 đến 20 ms. Vì vậy log xHCI trong ngữ cảnh ngắt chỉ in sự kiện bất thường, còn dump_stack nằm ở điểm hiếm. Chi tiết xem phần trả lời kèm theo.
- Chống spam: log port_event quan trọng luôn in; thay đổi nhỏ (link state, suspend, reset đứng riêng) giới hạn 10 dòng mỗi 5 giây.
