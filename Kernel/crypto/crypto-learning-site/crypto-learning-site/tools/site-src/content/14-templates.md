id: ch-templates
title: Templates và thuật toán ghép
nav: Templates
group: Mở rộng & tăng tốc
kicker: Mở rộng & tăng tốc · Template
lead: "Hàm sinh thuật toán theo tên": từ chuỗi `authenc(hmac(sha256),cbc(aes))` tới một instance có thật, các spawn nối instance với alg con, và cơ chế gỡ dây chuyền.
meta: Đối chiếu: `crypto/algboss.c`, `crypto/algapi.c`, `crypto/hmac.c`, `crypto/cbc.c` | Khoảng 45 phút
source: 12-templates.md
related: ch-lookup, ch-registration, ch-hash, ch-aead
---
## Mục tiêu

{{include 12-templates.md#1}}

## Kiến thức cần có

{{include 12-templates.md#2}}

## Vấn đề và ý tưởng

{{include 12-templates.md#3.1}}

{{include 12-templates.md#3.2}}

## Các struct

{{include 12-templates.md#3.3}}

Sơ đồ class của template, instance, spawn và larval nằm ở chương [Kiến trúc core](#dg-arch-class-tmpl).

## Tham số template: từ chuỗi đến tb[]

{{diagram dg-tp-parse}}

{{include 12-templates.md#3.4}}

{{card fn-crypto_check_attr_type}}

## Công thức create() chuẩn

{{diagram dg-tp-create}}

{{include 12-templates.md#3.5}}

{{card fn-crypto_grab_spawn}}

{{card fn-crypto_register_instance}}

## Spawn khi tạo tfm

{{include 12-templates.md#3.6}}

{{diagram dg-tp-spawn-eagain}}

## Gỡ dây chuyền

{{include 12-templates.md#3.7}}

{{diagram dg-tp-deps}}

## Quy tắc tên và priority của các template

{{include 12-templates.md#3.8}}

## Ví dụ: authenc(hmac(sha256),cbc(aes)) lần đầu

{{include 12-templates.md#4}}

## File và hàm cần biết

{{include 12-templates.md#6}}

## Lỗi thường gặp khi viết template

{{include 12-templates.md#7}}

## Tóm tắt

{{include 12-templates.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 12-templates.md}}
