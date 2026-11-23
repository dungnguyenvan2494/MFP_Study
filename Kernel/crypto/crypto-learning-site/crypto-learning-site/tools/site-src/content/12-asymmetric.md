id: ch-asymmetric
title: Mật mã khoá công khai — akcipher và kpp
nav: Asymmetric & KPP
group: Các loại API
kicker: Các loại API · Khoá công khai
lead: RSA qua `akcipher` và template `pkcs1pad`, trao đổi khoá qua `kpp`, và đường kiểm chữ ký X.509/PKCS#7 thật sự chạy trên sản phẩm.
meta: Đối chiếu: `crypto/akcipher.c`, `crypto/rsa.c`, `crypto/rsa-pkcs1pad.c`, `crypto/asymmetric_keys/public_key.c` | Khoảng 35 phút
source: 10-asymmetric-kpp.md
related: ch-templates, ch-request, ch-security
---
## Mục tiêu

{{include 10-asymmetric-kpp.md#1}}

## Kiến thức cần có

{{include 10-asymmetric-kpp.md#2}}

## Ba bài toán khác nhau

{{include 10-asymmetric-kpp.md#3.1}}

## akcipher

{{include 10-asymmetric-kpp.md#3.2}}

## pkcs1pad — template cho akcipher

{{include 10-asymmetric-kpp.md#3.3}}

{{diagram dg-asym-pkcs1-codes}}

## kpp

{{include 10-asymmetric-kpp.md#3.4}}

{{diagram dg-asym-kpp}}

## Ranh giới với subsystem khác

{{include 10-asymmetric-kpp.md#3.5}}

## Ví dụ: xác minh chữ ký X.509/PKCS#7

{{diagram dg-asym-verify-setup}}

{{diagram dg-asym-verify-run}}

{{include 10-asymmetric-kpp.md#4}}

{{card fn-public_key_verify_signature}}

## File và hàm cần biết

{{include 10-asymmetric-kpp.md#6}}

## Lỗi thường gặp

{{include 10-asymmetric-kpp.md#7}}

## Tóm tắt

{{include 10-asymmetric-kpp.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 10-asymmetric-kpp.md}}
