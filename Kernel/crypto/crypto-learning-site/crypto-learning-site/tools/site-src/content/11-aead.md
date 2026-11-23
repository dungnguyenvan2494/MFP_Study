id: ch-aead
title: AEAD — mã hoá có xác thực
nav: AEAD
group: Các loại API
kicker: Các loại API · Mã hoá có xác thực
lead: Một API vừa giữ bí mật vừa phát hiện dữ liệu bị sửa. Bố cục src/dst, các kiểm tra của core, GCM như một chuỗi async lồng nhau, và cách IPsec ESP xử lý kết quả.
meta: Đối chiếu: `crypto/aead.c`, `crypto/gcm.c`, `arch/arm64/crypto/ghash-ce-glue.c`, `net/ipv4/esp4.c` | Khoảng 40 phút
source: 09-aead.md
related: ch-skcipher, ch-hash, ch-templates, ch-request
---
## Mục tiêu

{{include 09-aead.md#1}}

## Kiến thức cần có

{{include 09-aead.md#2}}

## Vấn đề và ý tưởng

{{include 09-aead.md#3.1}}

{{include 09-aead.md#3.2}}

### Vì sao encrypt OK ≠ authenticated

{{include 09-aead.md#3.3}}

## Struct và bố cục bộ nhớ

{{diagram dg-aead-layout}}

{{include 09-aead.md#3.4}}

## Kiểm tra trong core

{{include 09-aead.md#3.5}}

{{diagram dg-aead-decrypt}}

{{card fn-crypto_aead_decrypt}}

## GCM được ghép thế nào

{{include 09-aead.md#3.6}}

{{diagram dg-aead-gcm-dec}}

## Trên sản phẩm: gcm-aes-ce

{{include 09-aead.md#3.7}}

{{diagram dg-aead-gcm-select}}

## Ví dụ: IPsec ESP

{{include 09-aead.md#4}}

{{diagram dg-aead-esp}}

## File và hàm cần biết

{{include 09-aead.md#6}}

## Lỗi thường gặp

{{include 09-aead.md#7}}

## Tóm tắt

{{include 09-aead.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 09-aead.md}}
