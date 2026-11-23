id: ch-hwaccel
title: Tăng tốc phần cứng và lựa chọn implementation
nav: Tăng tốc phần cứng
group: Mở rộng & tăng tốc
kicker: Mở rộng & tăng tốc · Implementation
lead: Ba kiểu "tăng tốc" — lệnh CPU, SIMD có vỏ async, engine phần cứng — và cơ chế kernel chọn giữa chúng. Trên sản phẩm chỉ hai kiểu đầu được build.
meta: Đối chiếu: `crypto/simd.c`, `crypto/cryptd.c`, `arch/arm64/crypto/`, `drivers/crypto/inside-secure/` | Khoảng 50 phút
source: 13-hw-acceleration.md
related: ch-skcipher, ch-request, ch-concurrency, ch-lookup
---
## Mục tiêu

{{include 13-hw-acceleration.md#1}}

## Kiến thức cần có

{{include 13-hw-acceleration.md#2}}

## Ba kiểu tăng tốc

{{include 13-hw-acceleration.md#3.1}}

## Kernel chọn implementation thế nào

{{diagram dg-hw-select}}

{{include 13-hw-acceleration.md#3.2}}

## Implementation thực tế trên sản phẩm

{{include ARCHITECTURE.md#10}}

{{diagram dg-hw-aes-candidates}}

## Generic và accelerated: aes-generic với aes-ce

{{include 13-hw-acceleration.md#3.3}}

## Đường async phần mềm: simd → cryptd

{{diagram dg-hw-cryptd-submit}}

{{diagram dg-hw-cryptd-complete}}

{{include 13-hw-acceleration.md#3.4}}

{{card fn-simd_skcipher_encrypt}}

{{card fn-cryptd_enqueue_request}}

{{card fn-cryptd_queue_worker}}

## Đường engine phần cứng: SafeXcel (tham khảo)

:::warn Không được build trên sản phẩm
`CONFIG_CRYPTO_HW` không được đặt trong defconfig sản phẩm. Phần này đọc từ source để hiểu một
thiết kế driver engine điển hình; **[H]** chưa xác định SoC sản phẩm có engine hay không.
:::

{{diagram dg-hw-safexcel-submit}}

{{diagram dg-hw-safexcel-complete}}

{{include 13-hw-acceleration.md#3.5}}

## crypto_engine — khung cho người viết driver

{{include 13-hw-acceleration.md#3.6}}

## So sánh bốn implementation cbc(aes)

{{include 13-hw-acceleration.md#4}}

## File và hàm cần biết

{{include 13-hw-acceleration.md#6}}

## Lỗi thường gặp

{{include 13-hw-acceleration.md#7}}

## Tóm tắt

{{include 13-hw-acceleration.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 13-hw-acceleration.md}}
