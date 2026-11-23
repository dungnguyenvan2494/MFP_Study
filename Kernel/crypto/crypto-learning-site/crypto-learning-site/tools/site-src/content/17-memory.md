id: ch-memory
title: Quản lý bộ nhớ và vòng đời đối tượng
nav: Memory
group: An toàn & vận hành
kicker: An toàn & vận hành · Bộ nhớ
lead: Ai cấp phát gì với cờ GFP nào, vì sao ctx được căn 128 byte, cái gì được xoá trắng khi giải phóng — và quy tắc lồng nhau buffer ⊂ request ⊂ tfm ⊂ alg.
meta: Đối chiếu: `crypto/api.c`, `crypto/skcipher.c`, `include/linux/crypto.h` | Khoảng 30 phút
source: 15-memory-concurrency-security.md
related: ch-concurrency, ch-security, ch-transform, ch-request
---
## Mục tiêu

{{include 15-memory-concurrency-security.md#1}}

## Kiến thức cần có

{{include 15-memory-concurrency-security.md#2}}

## Vòng đời lồng nhau

{{diagram dg-mem-nesting}}

## Ai cấp phát gì, với cờ nào

{{include 15-memory-concurrency-security.md#4.1}}

## Căn chỉnh

{{include 15-memory-concurrency-security.md#4.2}}

## Dữ liệu nhạy cảm: free thường và xoá trắng

{{include 15-memory-concurrency-security.md#4.3}}

## File và hàm cần biết

{{include 15-memory-concurrency-security.md#7}}

## Câu hỏi tự kiểm tra

{{quiz 15-memory-concurrency-security.md 1}}
