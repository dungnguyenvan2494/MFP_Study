id: ch-rng-others
title: RNG và các nhóm bổ sung
nav: RNG & nhóm khác
group: Các loại API
kicker: Các loại API · RNG, nén, null, userspace, async_tx
lead: `crypto_rng` và DRBG, RNG mặc định dùng chung, jitterentropy — cùng những phần hay bị nhầm là "crypto": nén, `crypto_null`, AF_ALG, `async_tx`, `lib/crypto`.
meta: Đối chiếu: `crypto/rng.c`, `crypto/drbg.c`, `crypto/jitterentropy.c` | Khoảng 35 phút
source: 11-rng-and-others.md
related: ch-hash, ch-security, ch-kconfig
---
## Mục tiêu

{{include 11-rng-and-others.md#1}}

## Kiến thức cần có

{{include 11-rng-and-others.md#2}}

## Năm thứ hay bị nhầm

{{include 11-rng-and-others.md#3.1}}

{{diagram dg-rng-map}}

## API crypto_rng

{{include 11-rng-and-others.md#3.2}}

{{card fn-crypto_rng_reset}}

## stdrng và RNG mặc định

{{include 11-rng-and-others.md#3.3}}

{{diagram dg-rng-default}}

{{card fn-crypto_get_default_rng}}

## DRBG: seed, reseed, mutex

{{include 11-rng-and-others.md#3.4}}

{{diagram dg-rng-drbg-state}}

## jitterentropy

{{include 11-rng-and-others.md#3.5}}

## Các nhóm khác

{{include 11-rng-and-others.md#3.6 as ###}}

{{include 11-rng-and-others.md#3.7 as ###}}

{{include 11-rng-and-others.md#3.8 as ###}}

{{include 11-rng-and-others.md#3.9 as ###}}

{{include 11-rng-and-others.md#3.10 as ###}}

{{include 11-rng-and-others.md#3.11 as ###}}

## Ví dụ: lấy 32 byte ngẫu nhiên

{{include 11-rng-and-others.md#4}}

## File và hàm cần biết

{{include 11-rng-and-others.md#6}}

## Lỗi thường gặp

{{include 11-rng-and-others.md#7}}

## Tóm tắt

{{include 11-rng-and-others.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 11-rng-and-others.md}}
