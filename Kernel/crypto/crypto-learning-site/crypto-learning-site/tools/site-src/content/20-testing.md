id: ch-testing
title: Testing — self-test, testmgr, tcrypt
nav: Testing
group: An toàn & vận hành
kicker: An toàn & vận hành · Kiểm thử
lead: Ai chạy self-test và khi nào, `alg_test()` tìm vector ra sao, ba loại kiểm thử khác nhau — và vì sao trên sản phẩm "selftest: passed" không chứng minh gì.
meta: Đối chiếu: `crypto/algboss.c`, `crypto/testmgr.c` | Khoảng 30 phút
source: ~16-testing-debugging.md
related: ch-registration, ch-debugging, ch-security
---
## Mục tiêu

{{include 16-testing-debugging.md#1}}

## Kiến thức cần có

{{include 16-testing-debugging.md#2}}

## Ai gọi test, khi nào

{{diagram dg-test-cryptomgr}}

{{include 16-testing-debugging.md#3.1}}

## alg_test() khi bật test

{{diagram dg-test-algtest}}

{{include 16-testing-debugging.md#3.2}}

{{card fn-alg_test}}

## Ví dụ: alg_test_skcipher

{{include 16-testing-debugging.md#3.3}}

## Ba loại kiểm thử

{{include 16-testing-debugging.md#3.4}}

## Bật test trên bản build debug

{{include 16-testing-debugging.md#3.5}}

## Câu hỏi tự kiểm tra

{{quiz 16-testing-debugging.md 1,2}}
