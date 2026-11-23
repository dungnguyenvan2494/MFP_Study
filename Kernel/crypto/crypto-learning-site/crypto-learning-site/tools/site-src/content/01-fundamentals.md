id: ch-fundamentals
title: Nền tảng mật mã học cho người mới
nav: Nền tảng mật mã
group: Bắt đầu
kicker: Bắt đầu
lead: Đủ kiến thức mật mã để đọc code kernel — khoá, IV, mode, hash, MAC, AEAD, khoá công khai, số ngẫu nhiên — và mỗi khái niệm tương ứng với loại API nào.
meta: Không cần biết toán | Khoảng 40 phút
source: 01-crypto-fundamentals.md
related: ch-why, ch-skcipher, ch-hash, ch-aead
---
## Mục tiêu

{{include 01-crypto-fundamentals.md#1}}

## Kiến thức cần có

{{include 01-crypto-fundamentals.md#2}}

## Các khái niệm

{{include 01-crypto-fundamentals.md#3.1 as ###}}

{{include 01-crypto-fundamentals.md#3.2 as ###}}

{{include 01-crypto-fundamentals.md#3.3 as ###}}

{{include 01-crypto-fundamentals.md#3.4 as ###}}

{{include 01-crypto-fundamentals.md#3.5 as ###}}

{{diagram dg-fund-cbc}}

{{include 01-crypto-fundamentals.md#3.6 as ###}}

{{include 01-crypto-fundamentals.md#3.7 as ###}}

{{include 01-crypto-fundamentals.md#3.8 as ###}}

{{include 01-crypto-fundamentals.md#3.9 as ###}}

{{include 01-crypto-fundamentals.md#3.10 as ###}}

{{include 01-crypto-fundamentals.md#3.11 as ###}}

## Ví dụ: một "gói tin" qua các primitive

{{include 01-crypto-fundamentals.md#4}}

:::warn Giải mã thành công ≠ dữ liệu đã được xác thực
CBC/CTR không phát hiện dữ liệu bị sửa. Khi cần cả bí mật lẫn toàn vẹn, dùng AEAD (`gcm(aes)`,
`authenc(...)`) — xem chương [AEAD](#ch-aead).
:::

## Chọn loại API

{{diagram dg-fund-choose}}

## File và hàm liên quan

{{include 01-crypto-fundamentals.md#6}}

## Lỗi thường gặp

{{include 01-crypto-fundamentals.md#7}}

## Tóm tắt

{{include 01-crypto-fundamentals.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 01-crypto-fundamentals.md}}
