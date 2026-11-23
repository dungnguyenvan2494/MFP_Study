id: ch-lookup
title: Tìm kiếm & chọn implementation
nav: Lookup & chọn implementation
group: Vòng đời & luồng lõi
kicker: Vòng đời & luồng lõi · Registry (2/2)
lead: Khi caller hỏi `"cbc(aes)"`, core chọn implementation nào, nạp module ra sao, và khi chưa có thì ai dựng nó từ template.
meta: Đối chiếu: `crypto/api.c`, `crypto/algboss.c`, `include/linux/crypto.h` | Khoảng 45 phút
related: ch-registration, ch-templates, ch-transform, ch-debugging
---
## Mục tiêu

- Đọc được quy tắc chọn trong `__crypto_alg_lookup()`: tên, type/mask, priority, TESTED, INTERNAL.
- Hiểu vòng lặp `crypto_alloc_tfm_node()` và ý nghĩa của `-EAGAIN`.
- Biết khi nào kernel gọi `request_module("crypto-…")` và khi nào nhờ cryptomgr dựng instance.
- Phân biệt các lỗi `-ENOENT`, `-ELIBBAD`, `-ETIMEDOUT`, `-EINTR` của lookup.

## Kiến thức cần có

Chương [Đăng ký algorithm](#ch-registration) (larval, test larval), chương
[Kiến trúc core](#ch-architecture) (`crypto_type`, `maskset/maskclear`).

## Bức tranh tổng thể

Mọi `crypto_alloc_<type>()` đi vào cùng một vòng lặp: tìm alg, tạo tfm, và thử lại toàn bộ khi
nhận `-EAGAIN`.

{{diagram dg-lk-alloc-loop}}

{{card fn-crypto_alloc_tfm_node}}

## Quy tắc chọn implementation

{{include 05-registry-lookup.md#3.7}}

{{diagram dg-lk-select}}

{{card fn-__crypto_alg_lookup}}

:::warn Hai algorithm cùng driver name `ctr-aes-ce`
`arch/arm64/crypto/aes-glue.c` đăng ký `ctr-aes-ce` bản **sync** (priority 299) rồi tạo vỏ SIMD
**async** cùng driver name (priority 300). Cả hai cùng có trong `/proc/crypto`. Hỏi theo driver
name với `mask = 0` nhận bản async (đăng ký sau, đứng đầu danh sách); với
`mask = CRYPTO_ALG_ASYNC` nhận bản sync. Đừng giả định driver name là duy nhất.
:::

## Luồng tra cứu đầy đủ

{{diagram dg-lk-modlookup}}

{{card fn-crypto_alg_mod_lookup}}

### Cây lời gọi chi tiết

{{include 05-registry-lookup.md#3.5}}

### Nạp module tự động

{{include 05-registry-lookup.md#3.6}}

## Khi chưa có implementation: request larval và cryptomgr

Ba sơ đồ dưới đây kể cùng một câu chuyện theo ba bước: lần hỏi đầu tiên tạo request larval;
cryptomgr dựng instance từ template; instance được test và người chờ được đánh thức.

{{diagram dg-lk-seq-miss}}

{{diagram dg-lk-seq-probe}}

{{diagram dg-lk-seq-wake}}

{{diagram dg-lk-wait}}

{{card fn-crypto_larval_wait}}

## Ví dụ: encrypted-keys hỏi `cbc(aes)` lần đầu

{{include 05-registry-lookup.md#4}}

## File và hàm cần biết

{{include 05-registry-lookup.md#6}}

## Tóm tắt

{{include 05-registry-lookup.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 05-registry-lookup.md 2,3,4}}
