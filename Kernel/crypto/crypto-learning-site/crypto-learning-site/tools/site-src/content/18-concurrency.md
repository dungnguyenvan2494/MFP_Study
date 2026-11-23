id: ch-concurrency
title: Concurrency — khoá, ngữ cảnh callback, song song
nav: Concurrency
group: An toàn & vận hành
kicker: An toàn & vận hành · Đồng bộ
lead: Core khoá gì cho bạn và không khoá gì; callback chạy trong ngữ cảnh nào; đăng ký, lookup và gỡ module song song an toàn ra sao.
meta: Đối chiếu: `crypto/api.c`, `crypto/cryptd.c`, `crypto/drbg.c`, `drivers/crypto/inside-secure/` | Khoảng 35 phút
source: ~15-memory-concurrency-security.md
related: ch-memory, ch-security, ch-request, ch-hwaccel
---
## Mục tiêu

- Biết từng primitive đồng bộ trong Crypto API bảo vệ dữ liệu nào.
- Phân biệt dữ liệu chia sẻ (tfm) với dữ liệu riêng (request, desc, buffer).
- Biết callback async chạy trong ngữ cảnh nào và vì sao không được ngủ.

## Kiến thức cần có

Chương [Request](#ch-request), [Tăng tốc phần cứng](#ch-hwaccel); biết `rw_semaphore`, `mutex`, `local_bh_disable`, softirq.

## Primitive đồng bộ

{{diagram dg-cc-locks}}

{{include 15-memory-concurrency-security.md#3.1}}

## Dữ liệu chia sẻ và dữ liệu riêng

{{include 15-memory-concurrency-security.md#3.2}}

## Callback chạy ở đâu?

{{diagram dg-cc-callback-ctx}}

{{include 15-memory-concurrency-security.md#3.3}}

## Đăng ký và lookup song song

{{include 15-memory-concurrency-security.md#3.4}}

## Gỡ module

{{include 15-memory-concurrency-security.md#3.5}}

## Câu hỏi tự kiểm tra

{{quiz 15-memory-concurrency-security.md 2,3,5}}
