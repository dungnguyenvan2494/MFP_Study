id: ch-hash
title: Hash và MAC — shash, ahash, HMAC
nav: Hash & MAC
group: Các loại API
kicker: Các loại API · Băm và xác thực
lead: Hai giao diện băm (shash đồng bộ trên con trỏ, ahash trên scatterlist), cầu nối giữa chúng, và cách template HMAC lưu sẵn trạng thái khoá.
meta: Đối chiếu: `crypto/shash.c`, `crypto/ahash.c`, `crypto/hmac.c`, `include/crypto/sha256_base.h` | Khoảng 45 phút
source: 08-hash.md
related: ch-aead, ch-templates, ch-rng-others
---
## Mục tiêu

{{include 08-hash.md#1}}

## Kiến thức cần có

{{include 08-hash.md#2}}

## Vấn đề: dữ liệu đến từng phần

{{include 08-hash.md#3.1}}

## Vì sao hash làm được điều đó

{{include 08-hash.md#3.2}}

{{diagram dg-hash-sha256-update}}

## Thuật ngữ kích thước

{{include 08-hash.md#3.3}}

## shash — đồng bộ, đơn giản

{{diagram dg-hash-seq-shash}}

{{include 08-hash.md#3.4}}

{{card fn-crypto_shash_update}}

### Desc = "request" của shash

{{include 08-hash.md#3.5}}

## ahash — scatterlist và có thể async

{{include 08-hash.md#3.6}}

{{diagram dg-hash-ahash-init}}

{{diagram dg-hash-seq-ahash}}

{{card fn-crypto_init_shash_ops_async}}

### Khác biệt sync và async với hash

{{include 08-hash.md#3.7}}

## HMAC — template trên shash

{{include 08-hash.md#3.8}}

{{diagram dg-hash-hmac}}

## Ví dụ trong repo

{{include 08-hash.md#4}}

## File và hàm cần biết

{{include 08-hash.md#6}}

## Lỗi thường gặp

{{include 08-hash.md#7}}

## Tóm tắt

{{include 08-hash.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 08-hash.md}}
