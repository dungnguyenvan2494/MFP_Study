## 1. Flow build

```
./build_uboot.sh <board> [clean|update]
```

**Bước 1 — Map tham số `$1` → tên defconfig** Một chuỗi `if/elif` map 20 giá trị board hợp lệ (`800`, `emuegl`, `emueglz`, `spam`, `hemlk`, ...) sang biến `CONFIG` = tên file `xxx_defconfig` tương ứng ([configs/](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/configs/)). Nếu `$1` không khớp cái nào → gọi `usage` và thoát (exit 1).

**Bước 2 — `setEnv`**

```bash
export CROSS_COMPILE=s800-linux-
```

Chỉ định toolchain cross-compile dùng cho toàn bộ quá trình `make` phía sau (compiler thực tế sẽ là `s800-linux-gcc`, v.v., lấy từ `$PATH`).\

**Bước 3 — rẽ nhánh theo `$2`**

|`$2`|Hành động|
|---|---|
|`clean`|`make mrproper` (xoá `.config` + mọi file build sinh ra) rồi `rm -rf output/` — dọn sạch, **không build**|
|`update`|Gọi thẳng `makeUboot` — **build tăng dần**, không chạy `mrproper` trước|
|_(để trống)_|`make mrproper` rồi mới gọi `makeUboot` — **build sạch từ đầu**|
|khác|`usage`|

**Bước 4 — bên trong `makeUboot`**

```bash
rm -rf ${OUT}; mkdir ${OUT}      # dọn lại thư mục output/
makeConfig                        # xem bên dưới
make                               # build toàn bộ theo .config hiện có
[ $? -ne 0 ] && exit 1            # build lỗi thì dừng, không copy
cp -a u-boot.bin ${OUT}/          # copy kết quả sang output/
```

**`makeConfig`**:

```bash
if [ ! -f .config ]; then
    make ${CONFIG}
fi
```

Chỉ chạy `make <defconfig>` (sinh `.config`) **nếu chưa có `.config`**.

> ⚠️ **Điểm cần lưu ý**: ở nhánh `update`, nếu `.config` từ lần build **board khác** trước đó vẫn còn tồn tại (do không chạy qua `mrproper`), lệnh `makeConfig` sẽ **bỏ qua việc tạo config mới** — script build tiếp bằng `.config` cũ, tức là **âm thầm build nhầm board** dù bạn truyền đúng tên board mới ở `$1`. Nhánh không truyền `$2` (mặc định) thì an toàn vì luôn `mrproper` trước.

## 2. Output sau khi build là gì?

Script chỉ copy đúng **1 file**: `u-boot.bin` → `output/u-boot.bin`.

Nhưng bản thân lệnh `make` (build in-tree, không dùng `O=`) sinh ra nhiều artifact hơn thế **ngay tại thư mục gốc `Src/`**, theo target mặc định `all` ([Makefile:713-773](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Src/Makefile#L713-L773)):

|File sinh ra tại `Src/`|Có được copy vào `output/` không|
|---|---|
|`u-boot` (ELF, chưa strip)|❌ Không|
|`u-boot.bin` (raw binary, đã cắt header ELF)|✅ Có|
|`u-boot.srec` (Motorola S-record)|❌ Không|
|`System.map` (bảng symbol để debug)|❌ Không|
|`.config`, các file `*.o`/`*.a` trung gian|❌ Không (ở lại `Src/`, không dọn trừ khi `clean`)|

→ Nếu bạn cần `u-boot.srec` hoặc `System.map` để debug/flash bằng công cụ khác `u-boot.bin`, phải tự lấy từ thư mục `Src/` gốc sau khi build — script không copy các file đó sang `output/`.