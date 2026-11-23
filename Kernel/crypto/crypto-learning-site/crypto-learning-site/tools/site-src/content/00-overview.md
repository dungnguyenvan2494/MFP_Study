id: ch-overview
title: Tổng quan và cách dùng tài liệu
nav: Tổng quan
group: Bắt đầu
kicker: Bắt đầu
lead: Linux Crypto API trong kernel 5.10 của repo Source_MFP — một kho thuật toán chung mà các subsystem xin theo tên. Trang này cho biết phạm vi, ví dụ xuyên suốt và cách đọc phần còn lại.
meta: Baseline: v5.10.240 + 4 file vendor | Nền tảng: arm64 (Marvell) | Khoảng 20 phút
source: 00-overview.md, README.md
related: ch-fundamentals, ch-why, ch-architecture, ch-roadmap
---
:::info Phạm vi và baseline (đã kiểm chứng)
{{include README.md#Phạm_vi}}
:::

## Mục tiêu

{{include 00-overview.md#1}}

## Kiến thức cần có

{{include 00-overview.md#2}}

## Crypto API là gì — trong 5 câu

{{include 00-overview.md#3}}

{{diagram dg-ov-layers}}

{{diagram dg-ov-journey}}

## Ba ví dụ chạy xuyên suốt

{{include 00-overview.md#4}}

## Những phát hiện đáng chú ý trong repo này

:::vendor Khác biệt so với upstream và cấu hình sản phẩm
{{include README.md#Những_phát_hiện}}
:::

## Quy ước

{{include 00-overview.md#7}}

| Nhãn | Nghĩa |
|---|---|
| [F] | Fact — đã kiểm chứng trực tiếp trong source |
| [I] | Suy luận hợp lý từ source, chưa chạy trên máy |
| [H] | Giả thuyết — cần đo hoặc kiểm trên phần cứng thật |

Mỗi sơ đồ có nhãn trạng thái: **Đã đối chiếu source**, **Có phần suy luận** hoặc **Sơ đồ khái niệm**.

## Cách dùng website

- **Tìm kiếm** (phím `/`): tìm theo tên chương, mục, hàm, struct, file, thuật ngữ và cả nội dung sơ đồ; gõ không dấu cũng được.
- **Sơ đồ**: nút *Phóng to* mở chế độ toàn màn hình (kéo để di chuyển, phím `+`/`−`), *Sao chép Mermaid* lấy mã nguồn, *Xem source* hiện mã ngay dưới sơ đồ.
- **Tiến độ** ở thanh bên chỉ đếm chương đã mở trên trình duyệt này; nó không đo mức hiểu.
- **In**: nút máy in vẽ mọi sơ đồ của chương hiện tại rồi mở hộp thoại in.

## Lỗi thường gặp khi tự học

{{include 00-overview.md#8}}

## Tóm tắt

{{include 00-overview.md#9}}

## Câu hỏi tự kiểm tra

{{quiz 00-overview.md}}
