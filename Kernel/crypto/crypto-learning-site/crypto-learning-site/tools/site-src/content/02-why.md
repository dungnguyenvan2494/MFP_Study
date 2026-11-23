id: ch-why
title: Vì sao kernel cần Crypto API?
nav: Vì sao cần Crypto API
group: Bắt đầu
kicker: Bắt đầu
lead: So sánh "mỗi subsystem tự làm" với "xin theo tên từ một registry chung", qua một ví dụ có thật: dòng cấu hình của dm-crypt.
meta: Khoảng 25 phút
source: 02-why-crypto-api.md
related: ch-architecture, ch-lookup, ch-hwaccel
---
## Mục tiêu

{{include 02-why-crypto-api.md#1}}

## Kiến thức cần có

{{include 02-why-crypto-api.md#2}}

## Tình huống

{{include 02-why-crypto-api.md#3}}

{{diagram dg-why-before}}

{{diagram dg-why-after}}

## Ví dụ: một dòng cấu hình dm-crypt đi tới đâu

{{diagram dg-why-dmcrypt}}

{{include 02-why-crypto-api.md#4}}

## File và hàm cần biết

{{include 02-why-crypto-api.md#6}}

## Lỗi thường gặp

{{include 02-why-crypto-api.md#7}}

## Tóm tắt

{{include 02-why-crypto-api.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 02-why-crypto-api.md}}
