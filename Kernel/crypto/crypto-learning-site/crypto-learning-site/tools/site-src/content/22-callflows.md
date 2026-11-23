id: ch-callflows
title: Call flows — call graph đã kiểm chứng
nav: Call flows
group: Tra cứu
kicker: Tra cứu · Luồng thực thi
lead: Call graph của các luồng chính — cấp tfm, dựng instance, đặt khoá, mã hoá, giải phóng, đăng ký, gỡ bỏ, đường cryptd — kèm bảng trách nhiệm và bảng xử lý lỗi.
meta: Mỗi cạnh liền là lời gọi trực tiếp đã kiểm trong source | Khoảng 40 phút
source: CALL_FLOWS.md
related: ch-lookup, ch-transform, ch-skcipher, ch-debugging
---
## Cách đọc call graph

<div class="legend">
<span><span class="badge badge-fn">xanh dương</span> API công khai</span>
<span><span class="badge">xám</span> core nội bộ</span>
<span><span class="badge badge-status-verified">xanh lá</span> implementation</span>
<span><span class="badge badge-vendor">cam</span> asm / phần cứng / vendor</span>
</div>

- **Mũi tên liền**: lời gọi hàm trực tiếp (có thể qua hàm inline).
- **Mũi tên đứt** có nhãn: gọi **gián tiếp** — qua con trỏ hàm (`alg->encrypt`, `tfm->exit`…),
  notifier, `kthread_run`, workqueue hoặc IRQ. Đây là chỗ `grep` tên hàm không tìm thấy người gọi.
- Số sau tên file là dòng bắt đầu của hàm trong `Kernel/Src/`.

## Luồng 1 — Từ caller tới implementation (encrypted-keys, cbc(aes))

{{include CALL_FLOWS.md#1.1}}

{{diagram dg-cg-lookup}}

{{diagram dg-cg-instantiate}}

{{diagram dg-cg-create}}

{{diagram dg-cg-encrypt}}

{{diagram dg-cg-free}}

:::details Call graph đầy đủ dạng text (một cây, có số dòng)
{{include CALL_FLOWS.md#1.2}}
:::

### Bảng trách nhiệm

{{include CALL_FLOWS.md#1.5}}

### Biến thể: caller không đặt mask

{{diagram dg-cg-encrypt-simd}}

{{include CALL_FLOWS.md#1.6}}

## Luồng 2 — Đăng ký và gỡ bỏ

{{diagram dg-cg-register}}

{{include CALL_FLOWS.md#2.1}}

### Trường bắt buộc

{{include CALL_FLOWS.md#2.2}}

### Gỡ bỏ

{{diagram dg-cg-unregister}}

## Luồng 3 — Đồng bộ và bất đồng bộ

{{include CALL_FLOWS.md#3.1}}

### Mẫu caller đúng

{{include CALL_FLOWS.md#3.2}}

### Đường async qua cryptd

{{diagram dg-cg-cryptd}}

## Luồng 4 — Tăng tốc phần cứng

{{include CALL_FLOWS.md#4.2}}

## Luồng 5 — Xử lý lỗi và dọn dẹp

{{include CALL_FLOWS.md#Luồng_5}}
