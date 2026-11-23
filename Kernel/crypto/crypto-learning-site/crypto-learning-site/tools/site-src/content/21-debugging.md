id: ch-debugging
title: Debugging — checklist theo triệu chứng
nav: Debugging
group: An toàn & vận hành
kicker: An toàn & vận hành · Debug
lead: Mười hai triệu chứng thường gặp, mỗi cái có giả thuyết, nơi đọc code, lệnh trên target và cách xác nhận. Kèm cách đọc log `@@@[CHK]timeout` của vendor.
meta: [T] = lệnh trên target · [S] = lệnh trên source tree | Khoảng 50 phút
source: 16-testing-debugging.md, BUILD_AND_DEBUG.md
related: ch-testing, ch-lookup, ch-request, ch-kconfig
---
## Mục tiêu

- Thu hẹp nguyên nhân của lỗi `crypto_alloc_*`, `setkey`, encrypt/decrypt.
- Nhận ra request async không bao giờ hoàn tất qua log của vendor.
- Biết lệnh điều tra trên target: `/proc/crypto`, dmesg, ftrace, `/proc/<pid>/stack`.

## Dọn dẹp đúng khi gặp lỗi

{{diagram dg-dbg-cleanup}}

## Checklist theo triệu chứng

{{include 16-testing-debugging.md#4.1 as ###}}

{{diagram dg-dbg-enoent}}

{{include 16-testing-debugging.md#4.2 as ###}}

{{include 16-testing-debugging.md#4.3 as ###}}

{{include 16-testing-debugging.md#4.4 as ###}}

{{include 16-testing-debugging.md#4.5 as ###}}

{{include 16-testing-debugging.md#4.6 as ###}}

{{diagram dg-dbg-wait-req}}

{{include 16-testing-debugging.md#4.7 as ###}}

{{include 16-testing-debugging.md#4.8 as ###}}

{{include 16-testing-debugging.md#4.9 as ###}}

{{include 16-testing-debugging.md#4.10 as ###}}

{{include 16-testing-debugging.md#4.11 as ###}}

{{include 16-testing-debugging.md#4.12 as ###}}

## Lệnh điều tra trên target

{{include BUILD_AND_DEBUG.md#2}}

## Bảng tra nhanh: mã lỗi → nơi xem đầu tiên

{{include BUILD_AND_DEBUG.md#3}}

## File và hàm cần biết

{{include 16-testing-debugging.md#6}}

## Lỗi thường gặp khi debug

{{include 16-testing-debugging.md#7}}

## Tóm tắt

{{include 16-testing-debugging.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 16-testing-debugging.md 3,4}}
