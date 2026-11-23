id: ch-registration
title: Đăng ký algorithm
group: Vòng đời & luồng lõi
kicker: Vòng đời & luồng lõi · Registry (1/2)
lead: Điều gì xảy ra từ lúc một implementation gọi `crypto_register_*()` đến khi nó được đánh dấu `TESTED` và có thể được chọn — và ngược lại khi nó bị gỡ.
meta: Đối chiếu: `crypto/algapi.c`, `crypto/api.c`, `crypto/algboss.c` | Khoảng 40 phút
source: 05-registry-lookup.md
related: ch-lookup, ch-templates, ch-testing, ch-concurrency
---
## Mục tiêu

- Biết `crypto_register_alg()` kiểm tra gì và trả lỗi nào (`-EINVAL`, `-EEXIST`).
- Hiểu **larval** là gì, có hai loại nào, và vì sao test larval mang cờ `TESTED`.
- Biết ai gọi `crypto_alg_tested()` và hàm này quyết định những gì.
- Biết thứ tự gỡ bỏ trong `crypto_unregister_alg()` và hai điểm `WARN`.

## Kiến thức cần có

{{include 05-registry-lookup.md#2}}

## Vấn đề cần giải quyết

{{include 05-registry-lookup.md#3.1}}

## Ý tưởng: larval, khoá đọc/ghi, refcount hai tầng

{{include 05-registry-lookup.md#3.2}}

{{card st-crypto_larval}}

## Luồng đăng ký

Luồng được tách thành hai sơ đồ: phần **chèn vào registry** (đồng bộ, dưới write lock) và phần
**self-test** (qua notifier và kthread của cryptomgr). Sau đó là các quyết định bên trong
`crypto_alg_tested()`.

{{diagram dg-reg-insert}}

{{diagram dg-reg-test}}

{{diagram dg-reg-tested-flow}}

{{card fn-crypto_register_alg}}

{{card fn-crypto_alg_tested}}

### Cây lời gọi chi tiết

{{include 05-registry-lookup.md#3.3}}

### Những gì `crypto_alg_tested()` làm, theo thứ tự

{{include 05-registry-lookup.md#3.4}}

## Trạng thái của một algorithm

{{diagram dg-reg-state}}

## Gỡ bỏ

{{include 05-registry-lookup.md#3.8}}

{{diagram dg-reg-unregister}}

{{card fn-crypto_unregister_alg}}

:::vendor Thay đổi của vendor trong luồng gỡ bỏ
`crypto_remove_alg()` phát `CRYPTO_MSG_ALG_UNREGISTER` khi **đang giữ** `crypto_alg_sem` (write).
Trong cây nguồn không có listener nào xử lý message này (`crypto/algboss.c` chỉ xử lý
`ALG_REQUEST`, `ALG_REGISTER`, `ALG_LOADED`). Nếu module ngoài cây đăng ký notifier và gọi
lookup/alloc trong callback, nó sẽ **deadlock** trên chính `crypto_alg_sem`.
:::

### "Bắn hạ" một alg

{{include 05-registry-lookup.md#3.9}}

## Lỗi thường gặp

{{include 05-registry-lookup.md#7}}

## Tóm tắt

- Mọi đăng ký đi qua `crypto_register_alg()`: kiểm tham số → chèn alg + test larval dưới write
  lock → chờ self-test.
- Self-test luôn "chạy" (tạo kthread) kể cả khi bị tắt trên sản phẩm; khi đó nó trả 0 ngay.
- `crypto_alg_tested()` là nơi alg trở thành `TESTED`, người chờ nhận kết quả, và instance dựng
  trên alg kém hơn bị gỡ.
- Gỡ bỏ an toàn nhờ module refcount + `cra_refcnt`; `WARN_ON(refcount != 1)` báo còn người giữ.

## Câu hỏi tự kiểm tra

Câu hỏi 2–4 (về lookup) nằm ở chương [Tìm kiếm & chọn implementation](#ch-lookup).

{{quiz 05-registry-lookup.md 1,5}}
