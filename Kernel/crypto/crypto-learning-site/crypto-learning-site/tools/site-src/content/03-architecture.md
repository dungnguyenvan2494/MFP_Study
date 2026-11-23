id: ch-architecture
title: Kiến trúc core — crypto_alg, crypto_type, crypto_tfm
nav: Kiến trúc core
group: Kiến trúc
kicker: Kiến trúc
lead: Một registry duy nhất chứa mọi loại thuật toán nhờ "kế thừa bằng struct lồng"; core tạo transform qua `crypto_type` mà không cần biết toán học bên trong.
meta: Đối chiếu: `include/linux/crypto.h`, `include/crypto/algapi.h`, `crypto/api.c` | Khoảng 45 phút
source: 04-core-architecture.md, ARCHITECTURE.md
related: ch-source-tree, ch-registration, ch-lookup, ch-transform
---
## Mục tiêu

{{include 04-core-architecture.md#1}}

## Kiến thức cần có

{{include 04-core-architecture.md#2}}

## Một câu tóm tắt

{{include ARCHITECTURE.md#1}}

{{diagram dg-arch-overview}}

## Năm khái niệm nền

{{include ARCHITECTURE.md#2}}

## Các lớp và ranh giới trách nhiệm

{{include ARCHITECTURE.md#4}}

## Một registry cho nhiều loại thuật toán

{{include 04-core-architecture.md#3.1 as ###}}

{{include 04-core-architecture.md#3.2 as ###}}

{{diagram dg-arch-class-core}}

{{include 04-core-architecture.md#3.3 as ###}}

{{include 04-core-architecture.md#3.4 as ###}}

{{include 04-core-architecture.md#3.5 as ###}}

{{card st-crypto_tfm}}

{{include 04-core-architecture.md#3.6 as ###}}

{{include 04-core-architecture.md#3.7 as ###}}

## Registry: dữ liệu toàn cục và khoá

{{include ARCHITECTURE.md#5}}

## Template, instance, spawn

{{diagram dg-arch-class-tmpl}}

{{diagram dg-arch-tmpl}}

{{include ARCHITECTURE.md#8}}

{{card st-crypto_larval}}

## Ví dụ: tfm cbc(aes-ce) của encrypted-keys

{{include 04-core-architecture.md#4}}

## Những điểm kiến trúc dễ hiểu sai

{{include ARCHITECTURE.md#13}}

## File và hàm cần biết

{{include 04-core-architecture.md#6}}

## Lỗi thường gặp

{{include 04-core-architecture.md#7}}

## Tóm tắt

{{include 04-core-architecture.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 04-core-architecture.md}}
