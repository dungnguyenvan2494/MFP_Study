# Danh mục sơ đồ

Sinh tự động bởi `tools/site-src/inventory.py` từ `tools/site-src/diagrams/*.dg`. Mermaid source
của từng sơ đồ nằm trong `index.html` (thẻ `<script type="text/x-mermaid" class="diagram-source">`
bên trong `<figure class="diagram-card" id="…">`) và trong file `.dg` tương ứng.

Tổng: **91 sơ đồ** — Kiến trúc 2, Call graph 9, Class 2, Flowchart 31, Quan hệ 11, Sequence 32, State 4.
Trạng thái: Khái niệm 11, Có phần suy luận 5, Đã đối chiếu source 75.

## Các kiểm tra đã chạy cho mọi sơ đồ

| Kiểm tra | Công cụ | Kết quả |
|---|---|---|
| Cú pháp + render với cấu hình của site | mermaid-cli 11.4.2 (Mermaid 11.17.2), `checkdg.sh` + `render.sh` | 91/91 OK |
| Chữ bị ngắt giữa định danh (sequence) | `render.sh` (HYPHEN-BREAK) | 0 |
| Note một participant tràn khung | `notecheck.py` | 0 |
| Nhãn flowchart bị trình duyệt ngắt giữa định danh | `render.sh` (LONGLINE) | còn vài nhãn xuống dòng ở dấu cách giữa tham số — đã xem ảnh, đọc được |
| Vị trí `file:dòng` trong phần "File & hàm" | `refcheck.py` | 414 vị trí, 0 lỗi |
| Render trong trình duyệt (Chromium, file://) | `tools/qa/full.js` ở 1440×900, 1024×768 (tối), 390×844 | 91/91 `data-rendered="true"`, 0 lỗi console |
| Xem ảnh từng sơ đồ (nhãn chồng, cạnh cắt chéo, chữ tràn) | ảnh PNG + tờ tổng hợp 2×2 | đã xem cả 91; các lỗi tìm thấy đã sửa |

Sơ đồ đánh dấu "(rộng)" lớn hơn 1300px: trên màn hình rộng chúng giữ kích thước thật và cuộn ngang
trong khung (hoặc mở "Phóng to"); trên điện thoại mọi sơ đồ được thu vừa khung.

## Danh sách

| ID | Tiêu đề | Loại | Trạng thái | Chương | File nguồn được trích | Kích thước render (px) |
|---|---|---|---|---|---|---|
| `dg-ov-layers` | Sáu lớp từ caller tới phần cứng | Kiến trúc | Khái niệm | Tổng quan và cách dùng tài liệu | *.h, algapi.c, algboss.c, api.c, crypto/, testmgr.c | 579×938 |
| `dg-ov-journey` | Một lần dùng Crypto API — năm bước của caller | Sequence | Khái niệm | Tổng quan và cách dùng tài liệu | encrypted.c, skcipher.c, skcipher.h | 1046×859 |
| `dg-fund-choose` | Chọn loại API theo nhu cầu | Flowchart | Khái niệm | Nền tảng mật mã học cho người mới | aead.h, akcipher.h, crypto.h, hash.h, kpp.h, rng.h, skcipher.h | 1302×843 (rộng) |
| `dg-fund-cbc` | CBC — mỗi block phụ thuộc block trước | Quan hệ | Khái niệm | Nền tảng mật mã học cho người mới | cbc.c | 1072×217 |
| `dg-why-before` | Không có Crypto API — mỗi subsystem tự làm | Quan hệ | Khái niệm | Vì sao kernel cần Crypto API? | — | 473×286 |
| `dg-why-after` | Có Crypto API — xin theo tên, registry chọn implementation | Quan hệ | Đã đối chiếu source | Vì sao kernel cần Crypto API? | aes-ce-glue.c, aes-cipher-glue.c, aes_generic.c, api.c | 776×378 |
| `dg-why-dmcrypt` | Một dòng cấu hình dm-crypt đi tới implementation nào | Sequence | Có phần suy luận | Vì sao kernel cần Crypto API? | aes-glue.c, dm-crypt.c, safexcel_cipher.c | 1174×607 |
| `dg-arch-overview` | Kiến trúc Crypto API — các khối và hướng gọi | Kiến trúc | Khái niệm | Kiến trúc core — crypto_alg, crypto_type, crypto_tfm | *.h, algapi.c, algboss.c, api.c, cryptd.c, crypto/, simd.c, skcipher.c | 909×955 |
| `dg-arch-class-core` | Đối tượng lõi — alg, type, tfm, request | Class | Đã đối chiếu source | Kiến trúc core — crypto_alg, crypto_type, crypto_tfm | algapi.h, crypto.h, skcipher.h | 747×1273 |
| `dg-arch-class-tmpl` | Template, instance, spawn và larval | Class | Đã đối chiếu source | Kiến trúc core — crypto_alg, crypto_type, crypto_tfm | algapi.c, algapi.h, internal.h | 1274×446 |
| `dg-arch-tmpl` | Ví dụ lắp ghép — template cbc + aes-ce thành instance cbc(aes-ce) | Quan hệ | Đã đối chiếu source | Kiến trúc core — crypto_alg, crypto_type, crypto_tfm | aes-ce-glue.c, aes_generic.c, cbc.c, skcipher.c | 615×730 |
| `dg-src-deps` | Các nhóm file trong crypto/ và hướng phụ thuộc | Quan hệ | Khái niệm | Bản đồ thư mục crypto/ | Kconfig, Makefile, algboss.c, api.c | 1253×676 |
| `dg-reg-insert` | Đăng ký (1/2) — kiểm tra tham số và chèn vào registry | Sequence | Đã đối chiếu source | Đăng ký algorithm | algapi.c, api.c | 1055×1095 |
| `dg-reg-test` | Đăng ký (2/2) — self-test qua cryptomgr và hoàn tất | Sequence | Đã đối chiếu source | Đăng ký algorithm | algapi.c, algboss.c, api.c | 1060×993 |
| `dg-reg-tested-flow` | crypto_alg_tested() — các quyết định | Flowchart | Đã đối chiếu source | Đăng ký algorithm | algapi.c | 698×1407 |
| `dg-reg-state` | Vòng đời một crypto_alg suy ra từ cờ cra_flags | State | Đã đối chiếu source | Đăng ký algorithm | api.c, crypto.h, internal.h | 397×605 |
| `dg-reg-unregister` | crypto_unregister_alg() — gỡ và các điểm cảnh báo | Flowchart | Đã đối chiếu source | Đăng ký algorithm | algapi.c | 749×1241 |
| `dg-lk-alloc-loop` | crypto_alloc_tfm_node() — vòng lặp lookup + tạo tfm | Flowchart | Đã đối chiếu source | Tìm kiếm & chọn implementation | api.c, skcipher.c | 689×1164 |
| `dg-lk-select` | __crypto_alg_lookup() — lọc và chọn một ứng viên | Flowchart | Đã đối chiếu source | Tìm kiếm & chọn implementation | api.c, internal.h | 845×1056 |
| `dg-lk-modlookup` | crypto_alg_mod_lookup() — larval, nạp module, cryptomgr | Flowchart | Đã đối chiếu source | Tìm kiếm & chọn implementation | algboss.c, api.c | 939×996 |
| `dg-lk-wait` | crypto_larval_wait() — ánh xạ kết quả chờ thành mã trả về | Flowchart | Đã đối chiếu source | Tìm kiếm & chọn implementation | api.c | 685×775 |
| `dg-lk-seq-miss` | Lần đầu cấp phát cbc(aes) (1/3) — caller không tìm thấy, tạo larval | Sequence | Có phần suy luận | Tìm kiếm & chọn implementation | algboss.c, api.c, encrypted.c | 1297×697 |
| `dg-lk-seq-probe` | Lần đầu cấp phát cbc(aes) (2/3) — cryptomgr dựng instance từ template | Sequence | Đã đối chiếu source | Tìm kiếm & chọn implementation | algapi.c, algboss.c, cbc.c, skcipher.c | 1091×935 |
| `dg-lk-seq-wake` | Lần đầu cấp phát cbc(aes) (3/3) — caller nhận alg và tạo tfm | Sequence | Đã đối chiếu source | Tìm kiếm & chọn implementation | algapi.c, api.c, skcipher.c | 1110×881 |
| `dg-tfm-state` | Vòng đời transform theo cờ CRYPTO_TFM_NEED_KEY | State | Đã đối chiếu source | Vòng đời transform (tfm) | api.c, skcipher.c | 444×427 |
| `dg-tfm-seq-create` | Tạo tfm (crypto_create_tfm_node) cho instance cbc(aes-ce) | Sequence | Đã đối chiếu source | Vòng đời transform (tfm) | algapi.c, algapi.h, api.c, skcipher.c | 1093×1049 |
| `dg-tfm-seq-setkey` | Đặt khoá — crypto_skcipher_setkey() qua template cbc tới aes-ce | Sequence | Đã đối chiếu source | Vòng đời transform (tfm) | aes-ce-glue.c, cipher.c, skcipher.c | 1194×1071 |
| `dg-tfm-seq-free` | Giải phóng tfm — crypto_free_skcipher() | Sequence | Đã đối chiếu source | Vòng đời transform (tfm) | api.c, skcipher.c, skcipher.h | 1213×875 |
| `dg-req-state` | Vòng đời một request theo mã trả về | State | Đã đối chiếu source | Request, đồng bộ và bất đồng bộ | algapi.c, cryptd.c, dm-crypt.c, skcipher.h | 605×583 |
| `dg-req-seq-sync` | Request đồng bộ — kết quả có ngay khi hàm trả về | Sequence | Đã đối chiếu source | Request, đồng bộ và bất đồng bộ | encrypted.c, skcipher.c | 736×623 |
| `dg-req-seq-async` | Request bất đồng bộ — -EINPROGRESS rồi callback | Sequence | Đã đối chiếu source | Request, đồng bộ và bất đồng bộ | api.c, cryptd.c, simd.c, skcipher.c | 1027×675 |
| `dg-req-seq-backlog` | Hàng đợi đầy — -EBUSY, backlog, callback hai lần | Sequence | Đã đối chiếu source | Request, đồng bộ và bất đồng bộ | algapi.c, cryptd.c, dm-crypt.c | 818×699 |
| `dg-req-handle` | Caller xử lý mã trả về (mẫu dm-crypt) | Flowchart | Đã đối chiếu source | Request, đồng bộ và bất đồng bộ | dm-crypt.c | 1135×411 |
| `dg-req-enqueue` | crypto_enqueue_request() — nguồn gốc của -EINPROGRESS / -EBUSY / -ENOSPC | Flowchart | Đã đối chiếu source | Request, đồng bộ và bất đồng bộ | algapi.c, algapi.h | 712×639 |
| `dg-sk-seq-template` | Mã hoá đồng bộ qua template — encrypted-keys với cbc(aes-ce) | Sequence | Đã đối chiếu source | Symmetric cipher — skcipher, cipher, skcipher_walk | aes-ce-glue.c, cbc.c, skcipher.c | 1325×1007 (rộng) |
| `dg-sk-seq-simd` | cbc-aes-ce — vỏ simd, đường đồng bộ khi NEON dùng được | Sequence | Đã đối chiếu source | Symmetric cipher — skcipher, cipher, skcipher_walk | aes-glue.c, simd.c, simd.h | 1287×977 |
| `dg-sk-walk-next` | skcipher_walk_next() — chọn đường fast / copy / slow | Flowchart | Đã đối chiếu source | Symmetric cipher — skcipher, cipher, skcipher_walk | skcipher.c | 922×701 |
| `dg-sk-setkey` | crypto_skcipher_setkey() — kiểm tra, căn chỉnh, cờ NEED_KEY | Flowchart | Đã đối chiếu source | Symmetric cipher — skcipher, cipher, skcipher_walk | skcipher.c | 615×723 |
| `dg-hash-seq-shash` | shash — init, update nhiều lần, final | Sequence | Đã đối chiếu source | Hash và MAC — shash, ahash, HMAC | hash.h, sha256_base.h, shash.c | 747×1063 |
| `dg-hash-sha256-update` | sha256_base_do_update() — phần dư và các block đầy đủ | Flowchart | Đã đối chiếu source | Hash và MAC — shash, ahash, HMAC | sha256_base.h | 588×906 |
| `dg-hash-ahash-init` | crypto_ahash_init_tfm() — ahash thật hay shash được bọc? | Flowchart | Đã đối chiếu source | Hash và MAC — shash, ahash, HMAC | ahash.c, shash.c | 710×635 |
| `dg-hash-seq-ahash` | crypto_ahash_digest() trên một shash được bọc | Sequence | Đã đối chiếu source | Hash và MAC — shash, ahash, HMAC | ahash.c, shash.c | 1013×955 |
| `dg-hash-hmac` | hmac(sha256) — setkey lưu sẵn trạng thái ipad/opad | Sequence | Đã đối chiếu source | Hash và MAC — shash, ahash, HMAC | hmac.c | 758×903 |
| `dg-aead-layout` | Bố cục src/dst của một aead_request | Quan hệ | Đã đối chiếu source | AEAD — mã hoá có xác thực | aead.h | 638×440 |
| `dg-aead-decrypt` | crypto_aead_decrypt() — các mã trả về | Flowchart | Đã đối chiếu source | AEAD — mã hoá có xác thực | aead.c, gcm.c, ghash-ce-glue.c | 730×611 |
| `dg-aead-gcm-dec` | gcm(aes) generic — decrypt là chuỗi GHASH rồi CTR rồi so tag | Sequence | Đã đối chiếu source | AEAD — mã hoá có xác thực | gcm.c | 1267×889 |
| `dg-aead-gcm-select` | gcm(aes) trên sản phẩm — implementation nào phục vụ? | Flowchart | Có phần suy luận | AEAD — mã hoá có xác thực | gcm.c, ghash-ce-glue.c | 605×487 |
| `dg-aead-esp` | IPsec ESP xử lý mã trả về của crypto_aead_encrypt() | Flowchart | Đã đối chiếu source | AEAD — mã hoá có xác thực | esp4.c | 741×309 |
| `dg-asym-verify-setup` | Kiểm chữ ký (1/2) — chọn thuật toán, cấp tfm, nạp khoá công khai | Sequence | Đã đối chiếu source | Mật mã khoá công khai — akcipher và kpp | public_key.c, rsa-pkcs1pad.c, rsa.c | 1015×825 |
| `dg-asym-verify-run` | Kiểm chữ ký (2/2) — request verify và mẫu "async → sync" | Sequence | Đã đối chiếu source | Mật mã khoá công khai — akcipher và kpp | public_key.c, rsa-pkcs1pad.c | 1173×735 |
| `dg-asym-pkcs1-codes` | pkcs1pad_verify_complete() — mỗi mã lỗi nghĩa là gì | Flowchart | Đã đối chiếu source | Mật mã khoá công khai — akcipher và kpp | rsa-pkcs1pad.c | 745×798 |
| `dg-asym-kpp` | kpp — hai bên thoả thuận khoá chung | Sequence | Khái niệm | Mật mã khoá công khai — akcipher và kpp | dh_helper.c, ecdh_helper.c, kpp.c, kpp.h | 480×807 |
| `dg-rng-map` | Ba nguồn "ngẫu nhiên" khác nhau trong kernel | Quan hệ | Đã đối chiếu source | RNG và các nhóm bổ sung | drbg.c, ecc.c, geniv.c, rng.c | 683×476 |
| `dg-rng-default` | crypto_get_default_rng() — RNG dùng chung, cấp phát lười | Sequence | Đã đối chiếu source | RNG và các nhóm bổ sung | drbg.c, rng.c | 1047×931 |
| `dg-rng-drbg-state` | Trạng thái seed của DRBG (enum drbg_seed_state) | State | Đã đối chiếu source | RNG và các nhóm bổ sung | drbg.c, drbg.h | 610×408 |
| `dg-tp-parse` | cryptomgr_schedule_probe() — từ tên "tmpl(a,b)" tới tb[] | Flowchart | Đã đối chiếu source | Templates và thuật toán ghép | algapi.c, algboss.c | 669×942 |
| `dg-tp-create` | Dựng instance hmac(sha256) — create(), grab spawn, đăng ký | Sequence | Đã đối chiếu source | Templates và thuật toán ghép | algapi.c, hmac.c | 1065×1189 |
| `dg-tp-deps` | Cây phụ thuộc instance — gỡ dây chuyền theo cra_users | Quan hệ | Có phần suy luận | Templates và thuật toán ghép | algapi.c, authenc.c, simd.c | 466×611 |
| `dg-tp-spawn-eagain` | Tạo tfm của instance khi alg con đang chết — -EAGAIN và dựng lại | Flowchart | Đã đối chiếu source | Templates và thuật toán ghép | algapi.c, api.c | 633×737 |
| `dg-hw-select` | Một implementation phải qua những "cửa" nào để được chọn | Flowchart | Đã đối chiếu source | Tăng tốc phần cứng và lựa chọn implementation | Makefile, api.c, cpufeature.h, ghash-ce-glue.c | 1011×865 |
| `dg-hw-aes-candidates` | cbc(aes) trên sản phẩm — mask của caller quyết định implementation | Flowchart | Có phần suy luận | Tăng tốc phần cứng và lựa chọn implementation | aes-glue.c, algapi.h, dm-crypt.c, encrypted.c, simd.c | 690×510 |
| `dg-hw-cryptd-submit` | simd → cryptd (1/2) — submit khi NEON không dùng được | Sequence | Đã đối chiếu source | Tăng tốc phần cứng và lựa chọn implementation | cryptd.c, simd.c | 1250×785 |
| `dg-hw-cryptd-complete` | simd → cryptd (2/2) — kworker xử lý và gọi callback | Sequence | Đã đối chiếu source | Tăng tốc phần cứng và lựa chọn implementation | cryptd.c | 1399×811 (rộng) |
| `dg-hw-safexcel-submit` | SafeXcel (1/2) — xếp request vào ring và "bấm nút" engine | Sequence | Đã đối chiếu source | Tăng tốc phần cứng và lựa chọn implementation | safexcel.c, safexcel_cipher.c | 1070×665 |
| `dg-hw-safexcel-complete` | SafeXcel (2/2) — IRQ, threaded handler và callback | Sequence | Đã đối chiếu source | Tăng tốc phần cứng và lựa chọn implementation | safexcel.c | 1248×557 |
| `dg-kc-build` | Từ Kconfig tới /proc/crypto — đường đi của template cbc | Flowchart | Đã đối chiếu source | Kconfig, Kbuild, module và "có sẵn khi chạy" | Kconfig, Makefile, algapi.c, api.c, cbc.c | 563×1095 |
| `dg-kc-select` | Chuỗi select — bật DM_CRYPT kéo theo những gì | Quan hệ | Đã đối chiếu source | Kconfig, Kbuild, module và "có sẵn khi chạy" | Kconfig | 1187×436 |
| `dg-kc-states` | Bảy trạng thái của một thuật toán — và cách kiểm từng trạng thái | Flowchart | Khái niệm | Kconfig, Kbuild, module và "có sẵn khi chạy" | Eagle_defconfig, Makefile, proc.c | 291×837 |
| `dg-kc-initcall` | Thứ tự đăng ký lúc boot (built-in) | Flowchart | Đã đối chiếu source | Kconfig, Kbuild, module và "có sẵn khi chạy" | aes-ce-glue.c, aes-glue.c, aes_generic.c, algboss.c, cbc.c, cryptd.c | 1051×123 |
| `dg-kc-autoload` | Tự nạp module — crypto_alloc_shash("cmac(aes)") khi CMAC=m | Sequence | Đã đối chiếu source | Kconfig, Kbuild, module và "có sẵn khi chạy" | algapi.c, api.c, cmac.c, crypto.h | 1386×807 (rộng) |
| `dg-mem-nesting` | Lồng nhau về thời gian sống — buffer ⊂ request ⊂ tfm ⊂ alg | Sequence | Đã đối chiếu source | Quản lý bộ nhớ và vòng đời đối tượng | algapi.c, api.c, skcipher.h | 1248×757 |
| `dg-cc-callback-ctx` | Callback của caller chạy trong ngữ cảnh nào? | Flowchart | Đã đối chiếu source | Concurrency — khoá, ngữ cảnh callback, song song | api.c, cryptd.c, crypto_engine.c, gcm.c, safexcel.c | 855×465 |
| `dg-cc-locks` | Bản đồ khoá — primitive nào bảo vệ dữ liệu nào | Quan hệ | Đã đối chiếu source | Concurrency — khoá, ngữ cảnh callback, song song | api.c, cryptd.c, drbg.c, rng.c, safexcel_cipher.c | 557×791 |
| `dg-sec-stack-bug` | Một bug lifetime điển hình — request trên stack với tfm có thể async | Sequence | Đã đối chiếu source | Security — thuật toán yếu, khoá, side channel, vendor | cryptd.c, simd.c, skcipher.c, skcipher.h | 1110×703 |
| `dg-test-cryptomgr` | cryptomgr_schedule_test() và cryptomgr_test() — test có thật sự chạy không? | Flowchart | Đã đối chiếu source | Testing — self-test, testmgr, tcrypt | algapi.c, algboss.c, testmgr.c | 731×892 |
| `dg-test-algtest` | alg_test() khi self-test được bật | Flowchart | Đã đối chiếu source | Testing — self-test, testmgr, tcrypt | testmgr.c | 764×937 |
| `dg-dbg-cleanup` | Xử lý lỗi và dọn dẹp — alloc, setkey, request, encrypt | Flowchart | Đã đối chiếu source | Debugging — checklist theo triệu chứng | api.c, skcipher.c, skcipher.h | 935×1107 |
| `dg-dbg-enoent` | crypto_alloc_*() trả -ENOENT — thu hẹp nguyên nhân | Flowchart | Đã đối chiếu source | Debugging — checklist theo triệu chứng | algboss.c, api.c, proc.c | 1372×598 (rộng) |
| `dg-dbg-wait-req` | crypto_wait_req() bản vendor — nguồn gốc log @@@[CHK]timeout | Flowchart | Đã đối chiếu source | Debugging — checklist theo triệu chứng | Eagle_defconfig, crypto.h | 584×670 |
| `dg-cg-lookup` | Call graph — crypto_alloc_skcipher() tới lookup | Call graph | Đã đối chiếu source | Call flows — call graph đã kiểm chứng | api.c, internal.h, skcipher.c | 1123×1021 |
| `dg-cg-instantiate` | Call graph — cryptomgr dựng instance cbc(aes-ce) | Call graph | Đã đối chiếu source | Call flows — call graph đã kiểm chứng | algapi.c, algboss.c, cbc.c, skcipher.c | 1059×918 |
| `dg-cg-create` | Call graph — tạo tfm cbc(aes-ce) và đặt khoá | Call graph | Đã đối chiếu source | Call flows — call graph đã kiểm chứng | aes-ce-glue.c, algapi.c, algapi.h, api.c, cipher.c, skcipher.c | 618×793 |
| `dg-cg-encrypt` | Call graph — crypto_skcipher_encrypt() qua template cbc | Call graph | Đã đối chiếu source | Call flows — call graph đã kiểm chứng | aes-ce-glue.c, cbc.c, skcipher.c | 800×691 |
| `dg-cg-encrypt-simd` | Call graph — biến thể cbc-aes-ce (mask = 0) | Call graph | Đã đối chiếu source | Call flows — call graph đã kiểm chứng | aes-glue.c, cryptd.c, simd.c, skcipher.c | 559×771 |
| `dg-cg-cryptd` | Call graph — đường bất đồng bộ qua cryptd | Call graph | Đã đối chiếu source | Call flows — call graph đã kiểm chứng | algapi.c, algapi.h, cryptd.c | 818×963 |
| `dg-cg-free` | Call graph — crypto_free_skcipher() | Call graph | Đã đối chiếu source | Call flows — call graph đã kiểm chứng | api.c, skcipher.c, skcipher.h | 702×771 |
| `dg-cg-register` | Call graph — crypto_register_alg() (alg đơn, ví dụ aes_generic) | Call graph | Đã đối chiếu source | Call flows — call graph đã kiểm chứng | aes_generic.c, algapi.c, algboss.c, api.c | 953×918 |
| `dg-cg-unregister` | Call graph — crypto_unregister_alg() | Call graph | Đã đối chiếu source | Call flows — call graph đã kiểm chứng | algapi.c | 1008×498 |
| `dg-ex-reason` | Quy trình suy luận chung cho mọi bài tập | Flowchart | Khái niệm | Bài thực hành trên source code | algboss.c, api.c | 417×925 |
| `dg-ex-timeline` | Bài 4 — timeline một request async qua simd → cryptd | Sequence | Đã đối chiếu source | Bài thực hành trên source code | cryptd.c, simd.c | 1003×985 |
| `dg-rm-deps` | Thứ tự học — chương nào cần chương nào | Quan hệ | Khái niệm | Lộ trình học, tổng kết và viết algorithm mới | — | 915×1433 |
