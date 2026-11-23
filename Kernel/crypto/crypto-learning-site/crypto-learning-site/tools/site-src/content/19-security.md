id: ch-security
title: Security — thuật toán yếu, khoá, side channel, vendor
nav: Security
group: An toàn & vận hành
kicker: An toàn & vận hành · Bảo mật
lead: Những gì kernel kiểm và không kiểm, thuật toán lỗi thời còn bật trên sản phẩm, rủi ro từ self-test tắt và từ thay đổi của vendor, cùng một bug lifetime điển hình.
meta: Đánh giá chỉ bằng đọc source — xem giới hạn ở cuối | Khoảng 30 phút
source: ~15-memory-concurrency-security.md
related: ch-memory, ch-concurrency, ch-testing, ch-debugging
---
## Mục tiêu

- Nhận diện thuật toán yếu/lỗi thời có trong cấu hình sản phẩm.
- Biết trách nhiệm nào thuộc caller: chất lượng khoá, nonce, xoá buffer khoá.
- Hiểu hệ quả của self-test tắt và của `crypto_wait_req` bản vendor.

## Thuật toán yếu hoặc lỗi thời

{{include 15-memory-concurrency-security.md#5.1}}

## Khoá, nonce, tag

{{include 15-memory-concurrency-security.md#5.2}}

## Constant-time và side channel

{{include 15-memory-concurrency-security.md#5.3}}

## FIPS và self-test

{{include 15-memory-concurrency-security.md#5.4}}

## Entropy

{{include 15-memory-concurrency-security.md#5.5}}

## Rủi ro từ thay đổi vendor

:::vendor crypto_wait_req và notifier dưới lock
{{include 15-memory-concurrency-security.md#5.6}}
:::

## Một bug lifetime điển hình

{{diagram dg-sec-stack-bug}}

{{include 15-memory-concurrency-security.md#5.8}}

## Lỗi thường gặp và hậu quả

{{include 15-memory-concurrency-security.md#8}}

## Giới hạn của đánh giá chỉ bằng source

{{include 15-memory-concurrency-security.md#5.7}}

## Câu hỏi tự kiểm tra

{{quiz 15-memory-concurrency-security.md 4}}
