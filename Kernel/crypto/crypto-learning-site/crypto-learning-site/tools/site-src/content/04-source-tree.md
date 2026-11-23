id: ch-source-tree
title: Bản đồ thư mục crypto/
nav: Bản đồ crypto/
group: Kiến trúc
kicker: Kiến trúc
lead: Mọi file trong `crypto/` của repo, phân nhóm theo chức năng, kèm Kconfig, giá trị trên Eagle_defconfig, mức ưu tiên đọc và mức độ đã phân tích. Lọc theo tên, hàm hoặc ưu tiên.
meta: Danh sách lấy từ `ls crypto/` thật | Mô tả từ bảng khảo sát | Khoảng 30 phút
source: 03-directory-map.md, SOURCE_TREE_MAP.md
related: ch-architecture, ch-kconfig, ch-roadmap
---
## Mục tiêu

{{include 03-directory-map.md#1}}

## Kiến thức cần có

{{include 03-directory-map.md#2}}

## Cây source tương tác

Mỗi dòng là một file (hoặc một nhóm file cùng vai trò) **có thật** trong `Kernel/Src/crypto/`.
Nhãn **Đọc chi tiết** nghĩa là tài liệu có trích dẫn số dòng trong file đó; **Khảo sát** nghĩa
là chỉ có mô tả trong bảng; **Chỉ liệt kê** là file có trong thư mục nhưng chưa được mô tả riêng.

{{tree}}

## Năm "dấu vân tay" để phân loại file

{{include 03-directory-map.md#3}}

## Ví dụ: tự phân loại crypto/essiv.c

{{include 03-directory-map.md#4}}

## Các nhóm và hướng phụ thuộc

{{diagram dg-src-deps}}

:::details Bảng mô tả đầy đủ 16 nhóm
{{include SOURCE_TREE_MAP.md#3.1}}
:::

## Baseline của repo

{{include SOURCE_TREE_MAP.md#1.1 as ###}}

{{include SOURCE_TREE_MAP.md#1.2 as ###}}

{{include SOURCE_TREE_MAP.md#1.3 as ###}}

{{include SOURCE_TREE_MAP.md#1.4 as ###}}

## Thống kê thư mục

{{include SOURCE_TREE_MAP.md#2}}

## File ngoài crypto/ cần biết

{{include SOURCE_TREE_MAP.md#5}}

## Thứ tự đọc source đề xuất

{{include SOURCE_TREE_MAP.md#6}}

## Lỗi thường gặp

{{include 03-directory-map.md#7}}

## Tóm tắt

{{include 03-directory-map.md#8}}

## Câu hỏi tự kiểm tra

{{quiz 03-directory-map.md}}
