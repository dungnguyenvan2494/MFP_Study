id: ch-skcipher
title: Symmetric cipher — skcipher, cipher, skcipher_walk
nav: skcipher
group: Các loại API
kicker: Các loại API · Mã hoá đối xứng
lead: API mặc định cho mã hoá đối xứng, cơ chế `skcipher_walk` biến scatterlist thành đoạn nhớ liền mạch, và ba implementation `cbc(aes)` khác nhau trên cùng sản phẩm.
meta: Đối chiếu: `crypto/skcipher.c`, `crypto/cbc.c`, `crypto/simd.c`, `arch/arm64/crypto/aes-glue.c` | Khoảng 50 phút
source: 07-skcipher.md
related: ch-transform, ch-request, ch-aead, ch-hwaccel
---
## Mục tiêu

{{include 07-skcipher.md#1}}

## Kiến thức cần có

{{include 07-skcipher.md#2}}

## Ba giao diện mã hoá đối xứng trong 5.10

{{include 07-skcipher.md#3.1}}

{{card fn-crypto_alloc_skcipher}}

## Scatterlist

{{include 07-skcipher.md#3.2}}

## struct skcipher_alg

{{include 07-skcipher.md#3.3}}

## Đặt khoá

{{diagram dg-sk-setkey}}

{{card fn-crypto_skcipher_setkey}}

## skcipher_walk — trái tim của implementation phần mềm

{{include 07-skcipher.md#3.4}}

{{diagram dg-sk-walk-next}}

{{card fn-skcipher_walk_virt}}

{{card fn-skcipher_walk_done}}

## Mã lỗi đặc trưng

{{include 07-skcipher.md#3.5}}

{{card fn-crypto_skcipher_encrypt}}

## Ba implementation cbc(aes) trên cùng repo

### Template cbc + cipher aes-ce (sync)

{{diagram dg-sk-seq-template}}

{{include 07-skcipher.md#4.1}}

### cbc-aes-ce — SIMD theo lô, vỏ async

{{diagram dg-sk-seq-simd}}

{{include 07-skcipher.md#4.2}}

### safexcel-cbc-aes — phần cứng (không build)

{{include 07-skcipher.md#4.3}}

## File và hàm cần biết

{{include 07-skcipher.md#6}}

## Lỗi thường gặp

{{include 07-skcipher.md#7}}

## Tóm tắt

{{include 07-skcipher.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 07-skcipher.md}}
