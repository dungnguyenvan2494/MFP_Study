id: ch-transform
title: Vòng đời transform (tfm)
nav: Vòng đời transform
group: Vòng đời & luồng lõi
kicker: Vòng đời & luồng lõi · Transform
lead: Transform là "thuật toán đã chọn + khoá đã xử lý". Chương này theo một tfm `cbc(aes)` từ lúc cấp phát, đặt khoá, tới khi bị huỷ.
meta: Đối chiếu: `crypto/api.c`, `crypto/skcipher.c`, `include/crypto/skcipher.h` | Khoảng 35 phút
source: 06-tfm-request-lifecycle.md
related: ch-request, ch-lookup, ch-skcipher, ch-memory
---
## Mục tiêu

{{include 06-tfm-request-lifecycle.md#1}}

## Kiến thức cần có

Chương [Kiến trúc core](#ch-architecture) và [Tìm kiếm & chọn implementation](#ch-lookup).

## Vấn đề và ý tưởng

{{include 06-tfm-request-lifecycle.md#3.1}}

{{include 06-tfm-request-lifecycle.md#3.2}}

## Từng thành phần là gì

{{include 06-tfm-request-lifecycle.md#3.3}}

{{card st-crypto_tfm}}

## Tạo tfm

{{diagram dg-tfm-seq-create}}

{{card fn-crypto_create_tfm_node}}

## Cờ NEED_KEY và đặt khoá

{{diagram dg-tfm-state}}

{{diagram dg-tfm-seq-setkey}}

{{include 06-tfm-request-lifecycle.md#3.5}}

## Huỷ tfm

{{diagram dg-tfm-seq-free}}

{{card fn-crypto_destroy_tfm}}

## Một tfm, nhiều request

{{include 06-tfm-request-lifecycle.md#3.6}}

## Ví dụ: encrypted-keys

{{include 06-tfm-request-lifecycle.md#4}}

## File và hàm cần biết

{{include 06-tfm-request-lifecycle.md#6}}

## Tóm tắt

{{include 06-tfm-request-lifecycle.md#8}}

## Câu hỏi tự kiểm tra

Câu 2 (request trên stack) nằm ở chương [Request](#ch-request).

{{quiz 06-tfm-request-lifecycle.md 1,3,4,5}}
