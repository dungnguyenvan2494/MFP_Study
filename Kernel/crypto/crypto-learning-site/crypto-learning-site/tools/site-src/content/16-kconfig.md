id: ch-kconfig
title: Kconfig, Kbuild, module và "có sẵn khi chạy"
nav: Kconfig & Kbuild
group: Mở rộng & tăng tốc
kicker: Mở rộng & tăng tốc · Build
lead: Bảy trạng thái từ "có file trong repo" tới "được caller chọn", chuỗi `select`, module ghép, thứ tự initcall và cơ chế tự nạp module.
meta: Đối chiếu: `crypto/Kconfig`, `crypto/Makefile`, `arch/arm64/configs/Eagle_defconfig` | Khoảng 35 phút
source: 14-kconfig-kbuild.md
related: ch-source-tree, ch-lookup, ch-debugging
---
## Mục tiêu

{{include 14-kconfig-kbuild.md#1}}

## Kiến thức cần có

{{include 14-kconfig-kbuild.md#2}}

## Bảy trạng thái — không được đồng nhất

{{diagram dg-kc-states}}

{{include 14-kconfig-kbuild.md#3.1}}

## Symbol người dùng và symbol ẩn *2

{{include 14-kconfig-kbuild.md#3.2}}

## Chuỗi select

{{diagram dg-kc-select}}

{{include 14-kconfig-kbuild.md#3.3}}

## Makefile: object ghép và tên module

{{include 14-kconfig-kbuild.md#3.4}}

## Từ object đến đăng ký: initcall

{{diagram dg-kc-initcall}}

{{include 14-kconfig-kbuild.md#3.5}}

## Tự nạp module

{{diagram dg-kc-autoload}}

{{include 14-kconfig-kbuild.md#3.6}}

## Giới hạn của repo này

{{include 14-kconfig-kbuild.md#3.7}}

## Ví dụ: truy vết cbc(aes) trên Eagle

{{diagram dg-kc-build}}

{{include 14-kconfig-kbuild.md#4}}

## Lệnh điều tra build

:::details Lệnh [S] trên source tree (từ BUILD_AND_DEBUG)
{{include BUILD_AND_DEBUG.md#1}}
:::

## File và lệnh cần biết

{{include 14-kconfig-kbuild.md#6}}

## Lỗi thường gặp

{{include 14-kconfig-kbuild.md#7}}

## Tóm tắt

{{include 14-kconfig-kbuild.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 14-kconfig-kbuild.md}}
