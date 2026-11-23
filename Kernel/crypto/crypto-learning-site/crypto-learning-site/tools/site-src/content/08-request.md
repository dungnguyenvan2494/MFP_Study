id: ch-request
title: Request, đồng bộ và bất đồng bộ
nav: Request & sync/async
group: Vòng đời & luồng lõi
kicker: Vòng đời & luồng lõi · Request
lead: Request là một thao tác. Chương này giải thích mã trả về `0`, `-EINPROGRESS`, `-EBUSY`, `-ENOSPC`, khi nào callback được gọi, và ai được giải phóng bộ nhớ lúc nào.
meta: Đối chiếu: `crypto/algapi.c`, `crypto/api.c`, `crypto/cryptd.c`, `drivers/md/dm-crypt.c` | Khoảng 40 phút
related: ch-transform, ch-hwaccel, ch-concurrency, ch-debugging
---
## Mục tiêu

- Biết request chứa gì và vì sao phải cấp cho đúng tfm (`reqsize`).
- Đọc đúng bốn nhóm mã trả về của một thao tác async và quy ước callback.
- Viết được caller đúng: hoặc chờ đồng bộ bằng `crypto_req_done` + `crypto_wait_req`, hoặc xử lý callback như dm-crypt.
- Nhận ra bản `crypto_wait_req()` của vendor in log `@@@[CHK]timeout`.

## Kiến thức cần có

Chương [Vòng đời transform](#ch-transform).

## Request chứa gì

{{card st-skcipher_request}}

### Vùng nháp `__ctx[]` và `reqsize`

{{include 06-tfm-request-lifecycle.md#3.4}}

## Vòng đời request

{{diagram dg-req-state}}

## Ba kịch bản

Ba sơ đồ dưới đây dùng cùng ba vai: caller, dispatcher (`crypto_skcipher_encrypt` trong
`crypto/skcipher.c`) và implementation. Chỉ khác ở **thời điểm** có kết quả.

{{diagram dg-req-seq-sync}}

{{diagram dg-req-seq-async}}

{{diagram dg-req-seq-backlog}}

:::warn Thứ tự giữa "submit trả về" và callback
Callback có thể chạy **trước** khi caller kịp xử lý `-EINPROGRESS` (worker hoặc IRQ trên CPU
khác). Mọi trạng thái mà callback dùng phải được chuẩn bị **trước** khi submit; đừng gán gì vào
`req` sau lời gọi.
:::

## Nguồn gốc của các mã async

{{diagram dg-req-enqueue}}

{{card fn-crypto_enqueue_request}}

## Caller xử lý mã trả về

{{diagram dg-req-handle}}

### Cách đơn giản: chờ đồng bộ

```c
DECLARE_CRYPTO_WAIT(wait);

skcipher_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG | CRYPTO_TFM_REQ_MAY_SLEEP,
                              crypto_req_done, &wait);
skcipher_request_set_crypt(req, sg_src, sg_dst, len, iv);
err = crypto_wait_req(crypto_skcipher_encrypt(req), &wait);
```

{{card fn-crypto_req_done}}

{{card fn-crypto_wait_req}}

:::vendor `crypto_wait_req()` trên sản phẩm này
Bản trong `include/linux/crypto.h:591-620` chờ theo chu kỳ 2000 jiffies và in
`@@@[CHK]timeout crypto_wait_req L601 <jiffies>` mỗi chu kỳ. Hành vi không đổi (vẫn chờ tới khi
callback đến), nhưng log này là dấu hiệu trực tiếp của một request **không bao giờ hoàn tất**.
Xem chương [Debugging](#ch-debugging).
:::

## Lỗi thường gặp

{{include 06-tfm-request-lifecycle.md#7}}

## Tóm tắt

- `0` hoặc lỗi âm khác: xong ngay. `-EINPROGRESS`: callback sẽ đến. `-EBUSY` (chỉ khi
  `MAY_BACKLOG`): đã vào backlog, callback đến **hai lần**. `-ENOSPC`: bị từ chối.
- Request và mọi buffer của nó phải sống tới callback cuối cùng.
- `crypto_req_done` + `crypto_wait_req` biến một tfm có thể async thành lời gọi đồng bộ.

## Câu hỏi tự kiểm tra

{{quiz 06-tfm-request-lifecycle.md 2}}
