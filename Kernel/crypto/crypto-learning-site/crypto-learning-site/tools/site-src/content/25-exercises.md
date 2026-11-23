id: ch-exercises
title: Bài thực hành trên source code
nav: Bài thực hành
group: Thực hành
kicker: Thực hành
lead: Năm bài tự làm trên repo: tìm implementation, truy vết encryption, so sánh hai implementation, theo dõi request async, phân tích một lỗi giả định. Đáp án được thu gọn — hãy tự làm trước.
meta: Lệnh [S] chạy trong `Kernel/Src/` | Khoảng 2–3 giờ
source: 17-exercises.md
related: ch-debugging, ch-callflows, ch-roadmap
---
## Mục tiêu

{{include 17-exercises.md#1}}

## Kiến thức cần có

{{include 17-exercises.md#2}}

## Quy trình suy luận chung

{{diagram dg-ex-reason}}

## Bài 1 — Tìm algorithm và implementation: sha256

{{include 17-exercises.md#Bài_1}}

## Bài 2 — Truy vết encryption: encrypted-keys → cbc(aes)

{{include 17-exercises.md#Bài_2}}

## Bài 3 — So sánh aes-generic và aes-ce

{{include 17-exercises.md#Bài_3}}

## Bài 4 — Theo dõi request async (simd → cryptd)

{{diagram dg-ex-timeline}}

{{include 17-exercises.md#Bài_4}}

## Bài 5 — Phân tích lỗi giả định

{{include 17-exercises.md#Bài_5}}

## Lỗi thường gặp khi làm bài

{{include 17-exercises.md#Những_lỗi}}

## Kỹ năng đạt được

{{include 17-exercises.md#3}}

## Câu hỏi tự kiểm tra tổng hợp

{{quiz 17-exercises.md}}
