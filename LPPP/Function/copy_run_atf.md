`copy_run_atf()` là **điểm bàn giao quyền điều khiển từ R4 sang cụm Cortex-A72** — hàm duy nhất trong toàn bộ firmware LPPP thực sự khởi động một CPU khác.

# Phần 1 · Đầu vào

```c
void copy_run_atf(void * code_header_loc)
```

**Một tham số duy nhất: địa chỉ nơi ảnh ATF nằm trong flash.** Không phải địa chỉ để chạy, mà là địa chỉ để **đọc header**.

Bốn nơi gọi, hai giá trị khác nhau:

|Nơi gọi|Giá trị truyền|Địa chỉ thật|Ý nghĩa|
|---|---|---|---|
|`board_init_palladium`|`&__rom_start__ + 0x100000`|`0xF8100000`|CS1, cách đầu firmware R4 đúng 1 MB|
|`board_init_toc`|`&__rom_start__ + 0x100000`|`0xF8100000`|như trên|
|`board_init_emu800`|`BSPI_CS0_ADDR`|**`0xF4000000`**|đầu QSPI CS0|
|`board_init_eseval`|`BSPI_CS0_ADDR`|**`0xF4000000`**|đầu QSPI CS0|

Trên **bo sản xuất** (emu800/eseval) thì ATF nằm ở **đầu CS0**, cùng chip flash với romfs và user data — khớp với comment trong [regAddrs.h:133](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/a0/include/regAddrs.h#L133):

```c
#define BSPI_CS0_ADDR 0xF4000000  /* This is where ATF/u-boot start */
#define BSPI_CS1_ADDR 0xF8000000  /* This is where R4 code start */
```

Trên **bo phát triển** (palladium/TOC) thì ATF được nhét chung vào CS1 với firmware R4, cách nhau 1 MB. Đây là lý do tham số phải truyền vào chứ không hard-code.

---

# Phần 2 · Định dạng ảnh: `code_header_t`

Đây là **header ảnh boot chuẩn của Marvell** (định dạng dùng chung cho BootROM của Armada). 68 byte, theo sau là thân ảnh.

```
offset  field                 dùng ở đây?
──────────────────────────────────────────────────────────────
+0x00   magic                 ✅ kiểm = 0xB105B002
+0x04   prolog_size           ✅ tính vị trí thân ảnh
+0x08   prolog_checksum       ❌ không kiểm
+0x0C   boot_image_size       ✅ số byte cần chép
+0x10   boot_image_checksum   ❌ không kiểm
+0x14   rsrvd0
+0x18   load_addr             ✅ chép tới đâu + đặt RVBAR
+0x1C   exec_addr             ❌ không dùng
+0x20   uart_cfg / baudrate / ext_count / aux_flags
+0x24   io_arg_0..3
+0x34   rsrvd1..3
+0x40   ble_type / ble_offset / ble_reserved
+0x44   ble_image_size        ⚠️ chỉ in ra log
+0x48   ble_image_start       — "the actual image starts here"
```

```
   code_header_loc
        │
        ▼
  ┌─────────────────┬──────────────────────────────────────┐
  │  PROLOG         │           BOOT IMAGE                 │
  │  header + ext   │           (ATF BL1)                  │
  └─────────────────┴──────────────────────────────────────┘
   ├── prolog_size ─┤├────── boot_image_size ──────────────┤
                    ▲
              nguồn của memcpy
```

**`ble_*` là phần thú vị bị bỏ qua.** Comment trong struct ghi: _"BLE executable to set up DDR"_ — trong luồng boot Armada chuẩn, BootROM của AP806 nạp BLE để tự khởi tạo DDR. Ở đây **R4 tự làm DDR** bằng `init_ddr()` trước đó, nên phần BLE hoàn toàn không được dùng — chỉ `ble_image_size` xuất hiện trong một dòng debug print.

---

# Phần 3 · Từng dòng

```c
void copy_run_atf(void * code_header_loc)
{
    volatile uint32_t *ap806addr;                                       // ① BIẾN CHẾT
    code_header_t *code_header = (code_header_t *)(code_header_loc);    // ②

    ApiSysDebug_Printf(-1, -1, "%s\n", __func__);

    if (code_header->magic != CODE_MAGIC_WORD) {                        // ③
        code_header = (code_header_t *)BSPI_CS1_ADDR;                   //   dự phòng
        if (code_header->magic != CODE_MAGIC_WORD) {
            ApiSysDebug_Printf(-1, -1, "Unable to find ATF code\n");
            return;                                                      //   ⚠ BỎ CUỘC
        }
    }

    cpu_disable_dcache();                                                // ④
    start_ap806(false);                                                  // ⑤ giữ reset

    ApiSysDebug_Printf(... code_header, magic, ble_image_size, boot_image_size);
    ApiSysDebug_Printf(... "memcpy(0x%08x, 0x%08x, %d);" ...);

    memcpy((void *)(DDR_START + code_header->load_addr),                 // ⑥ CHÉP
           (void *)((uint32_t)&code_header->magic
                    + (uint32_t)code_header->prolog_size),
           code_header->boot_image_size);

    mrvl_regwrite32(AP806_CCU_B_RVBAR_0,                                 // ⑦ vector reset
                    (DDR_START + code_header->load_addr) >> 16);

    mrvl_regwrite32(PLAT_WARMBOOT_FLAG_BASE, 0);                         // ⑧ cờ cold boot

    start_ap806(true);                                                   // ⑨ THẢ RESET
    cpu_enable_dcache();                                                 // ⑩

    ApiSysDebug_Printf(-1,-1,"ap806 is started\n");
}
```

## ① `ap806addr` — biến chết

Khai báo `volatile uint32_t *ap806addr;` rồi **không bao giờ dùng**. Tàn dư từ phiên bản trước khi chuyển sang macro `mrvl_regwrite32`.

## ② Ép kiểu con trỏ, không đọc gì

Chỉ là "diễn giải vùng flash này như một `code_header_t`". Vì R4 chạy XIP và flash được map thẳng vào không gian địa chỉ, ta đọc header **trực tiếp từ flash** — không cần chép ra RAM trước.

## ③ Kiểm magic + dự phòng

`CODE_MAGIC_WORD = 0xB105B002` — đọc theo kiểu leetspeak là **"B105 B002" ≈ "BIOS BOOT"**. Đây là magic chuẩn của ảnh boot Marvell.

Nếu địa chỉ được truyền không có magic → **thử lại ở `BSPI_CS1_ADDR` (0xF8000000)**, tức đầu chip flash kia. Vẫn không có → in một dòng rồi `return`.

⚠️ Địa chỉ dự phòng là **hard-code**, không phụ thuộc tham số. Với bo sản xuất (truyền `0xF4000000`), dự phòng trỏ vào **đầu firmware R4** — nơi bắt đầu bằng section `.reset` chứ không phải header ATF. Nên thực tế fallback này gần như chắc chắn thất bại. Nó chỉ có nghĩa nếu ai đó nạp ATF vào cả hai chip.

## ④ + ⑩ Tắt/bật D-cache

**Bắt buộc.** `memcpy` ở bước ⑥ ghi vào DDR — vùng mà **AP806 sẽ đọc bằng lõi khác qua bus khác**. Nếu D-cache của R4 còn bật, dữ liệu có thể còn nằm trong cache và chưa xuống DDR khi A72 bắt đầu fetch lệnh.

Cách xử lý ở đây là **thô nhưng chắc**: tắt hẳn cache trong toàn bộ thao tác, thay vì gọi `cpu_dcache_writeback_region()` cho đúng vùng. Đơn giản, và vì chỉ chạy một lần lúc boot nên chi phí không đáng kể.

⚠️ Có một lỗ hổng: nếu bước ③ `return` sớm thì **không sao** (chưa tắt cache). Nhưng nếu có exception giữa ④ và ⑩ thì cache bị để tắt vĩnh viễn — hiệu năng R4 sụt thảm mà không ai biết.

## ⑤ + ⑨ `start_ap806()` — mổ xẻ riêng

```c
static void start_ap806(bool start)
{
    bool isWdEnable = HalApiWatchdog_isEnable( 0 );
    if (start) {
        mrvl_regwrite32(AP806_CCU_B_PRCR_0, 0x10001);   // let it go
    } else {
        if (isWdEnable == false) {
            HalApiWatchdog_Enable(0, 500, 0);            // watchdog 500 ms
        }
        mrvl_regwrite32(AP806_CCU_B_PRCR_0, 0);          // put it in reset
        if (isWdEnable == false) {
            HalApiWatchdog_Disable(0);                   // tắt lại
        }
    }
}
```

|Thanh ghi|Địa chỉ|Giá trị|Ý nghĩa|
|---|---|---|---|
|`AP806_CCU_B_PRCR_0`|`0xCE001A50`|`0`|giữ lõi trong reset|
|||`0x10001`|thả reset — bit 0 và bit 16|

`AP806_CCU_B_PRCR_0 = SB_AP806_CONFIG_BASE (0xCE000000) + 0x001A50`.

**Vì sao bật watchdog quanh mỗi lần ghi `PRCR = 0`?** Đây là điểm rất tinh tế: thao tác đưa AP806 vào reset **có thể treo bus**. Nếu AP806 đang có giao dịch AXI dở dang qua liên kết MoChi, việc reset nó đột ngột có thể làm R4 kẹt vĩnh viễn khi chờ phản hồi bus. Watchdog 500 ms là **lưới an toàn duy nhất** cho tình huống đó — máy tự reboot thay vì đơ.

Điều kiện `isWdEnable == false` tránh giẫm chân: nếu ai đó đã bật watchdog cho mục đích khác thì để nguyên, không tắt nhầm của người ta.

## ⑥ `memcpy` — phép tính địa chỉ

```c
dest = DDR_START + load_addr           // DDR_START = 0x00000 → chính là load_addr
src  = &code_header->magic + prolog_size
len  = boot_image_size
```

`&code_header->magic` chính là `code_header` (magic là trường đầu), nên `src = địa_chỉ_header + prolog_size` — tức **bỏ qua toàn bộ prolog để tới thân ảnh**.

`DDR_START = 0x00000` với comment _"where the ap806 ddr starts in my address space"_ — từ góc nhìn R4, DDR của AP806 được map bắt đầu từ **địa chỉ 0**. Cửa sổ này do `setup_a2_primary_windows()` trong `pmu_ap_link_up` dựng lên trước đó. Khớp với `CONFIG_SYS_SDRAM_BASE 0x00000000` phía U-Boot.

⚠️ **Không kiểm checksum.** Header có sẵn `prolog_checksum` và `boot_image_checksum` nhưng cả hai đều bị bỏ qua. Chỉ magic được kiểm — 4 byte. Một ảnh ATF hỏng giữa chừng vẫn được chép và chạy.

## ⑦ Đặt vector reset

```c
mrvl_regwrite32(AP806_CCU_B_RVBAR_0, (DDR_START + code_header->load_addr) >> 16);
```

`AP806_CCU_B_RVBAR_0 = 0xCE001A40`. **RVBAR** = _Reset Vector Base Address Register_ — nơi lõi A72 bắt đầu fetch lệnh khi thoát reset.

**Phép `>> 16` rất quan trọng**: thanh ghi lưu địa chỉ theo **đơn vị 64 KB**, không phải byte. Nghĩa là điểm vào bắt buộc phải căn chỉnh 64 KB.

Điều này giải thích hằng số `WARM_BOOT_ADDR = 0x1003` trong `power_cpu()`: cùng mã hoá, tương ứng địa chỉ vật lý **`0x10030000`**. _(Suy luận — không xác nhận được `load_addr` thật vì ảnh ATF không có trong repo. Nếu `load_addr == 0x10030000` thì cold boot và warm boot vào cùng một điểm, và chỉ có cờ ở bước ⑧ phân biệt hai nhánh.)_

## ⑧ Cờ cold boot

```c
mrvl_regwrite32(PLAT_WARMBOOT_FLAG_BASE, 0);   /* Clear WarmBoot flag */
```

`PLAT_WARMBOOT_FLAG_BASE = PLAT_CSS_SCP_COM_SHARED_MEM_BASE + 0xFFF0 = 0x7FFFFFF0` — nằm trong **vùng nhớ chia sẻ SCPI**, cùng vùng mà MHU dùng.

Ghi `0` = _"đây là cold boot"_. ATF đọc cờ này để biết nên chạy khởi tạo đầy đủ hay khôi phục từ suspend. Đối chiếu:

|Đường|Ai ghi|Giá trị|
|---|---|---|
|Cold boot|`copy_run_atf()`|`0`|
|Suspend|`power_cpu()` khi core cuối tắt|`PLAT_WARMBOOT_FLAG_BASE` (khác 0)|
|Wake timer nổ|`my_callback()`|`0` — ép cold boot|

---

# Phần 4 · Flow high-level

```
                    ┌──────────────────────────────┐
                    │  copy_run_atf(header_loc)    │
                    └──────────────┬───────────────┘
                                   ▼
                     ┌─────────────────────────────┐
                     │ header->magic == 0xB105B002?│
                     └───────┬─────────────┬───────┘
                        không│             │đúng
                             ▼             │
              ┌──────────────────────┐     │
              │ thử BSPI_CS1_ADDR    │     │
              │      0xF8000000      │     │
              └──────┬───────┬───────┘     │
                không│       │đúng         │
                     ▼       └─────────────┤
        ╔════════════════════════╗         │
        ║ "Unable to find ATF"   ║         │
        ║ return — AP806 nằm im  ║         │
        ║ MÃI MÃI                ║         │
        ╚════════════════════════╝         │
                                           ▼
                         ┌─────────────────────────────────┐
                         │ cpu_disable_dcache()            │
                         │  ⇒ mọi ghi đi thẳng ra bộ nhớ   │
                         └────────────────┬────────────────┘
                                          ▼
                         ┌─────────────────────────────────┐
                         │ start_ap806(false)              │
                         │  ├ WDT 500 ms  (lưới an toàn)   │
                         │  ├ PRCR_0 = 0   → GIỮ RESET      │
                         │  └ WDT off                      │
                         └────────────────┬────────────────┘
                                          ▼
   FLASH ─────────────────────────────────────────────────────► DDR
   header_loc + prolog_size   ── memcpy(boot_image_size) ──►  load_addr
                                          │
                                          ▼
                         ┌─────────────────────────────────┐
                         │ RVBAR_0 = load_addr >> 16       │
                         │  ⇒ "thoát reset thì nhảy vào đây"│
                         └────────────────┬────────────────┘
                                          ▼
                         ┌─────────────────────────────────┐
                         │ WARMBOOT_FLAG (0x7FFFFFF0) = 0  │
                         │  ⇒ báo ATF: đây là COLD BOOT    │
                         └────────────────┬────────────────┘
                                          ▼
                         ╔═════════════════════════════════╗
                         ║ start_ap806(true)               ║
                         ║   PRCR_0 = 0x10001              ║
                         ║   ★ CỤM A72 BẮT ĐẦU CHẠY ATF   ║
                         ╚════════════════┬════════════════╝
                                          ▼
                         ┌─────────────────────────────────┐
                         │ cpu_enable_dcache()             │
                         │ return — R4 tiếp tục init_thread│
                         └─────────────────────────────────┘

   Từ đây: R4 và A72 chạy SONG SONG trên hai hệ điều hành khác nhau
```

Điểm cần nhớ: hàm này **return ngay sau khi thả reset**, không chờ ATF làm gì cả. Không có handshake, không có timeout, không có xác nhận. R4 quay về `init_thread` nạp QoS rồi tự xoá, hoàn toàn không biết ATF có boot được hay không.

---

# Phần 5 · Bảng tra biến và hằng số

|Ký hiệu|Giá trị|Nguồn|Ý nghĩa|
|---|---|---|---|
|`CODE_MAGIC_WORD`|`0xB105B002`|`ap806.c:46`|Magic ảnh boot Marvell|
|`DDR_START`|`0x00000`|`ap806.c:45`|DDR của AP806 nhìn từ R4 bắt đầu ở 0|
|`BSPI_CS0_ADDR`|`0xF4000000`|`regAddrs.h:133`|QSPI CS0 — ATF + U-Boot|
|`BSPI_CS1_ADDR`|`0xF8000000`|`regAddrs.h:134`|QSPI CS1 — firmware R4|
|`AP806_CCU_B_PRCR_0`|`0xCE001A50`|`regAddrs.h:219`|Điều khiển reset lõi|
|`AP806_CCU_B_RVBAR_0`|`0xCE001A40`|`regAddrs.h:216`|Vector reset, đơn vị 64 KB|
|`PLAT_WARMBOOT_FLAG_BASE`|`0x7FFFFFF0`|`scpi_api.h:73`|Cờ cold/warm trong vùng SCPI|
|`code_header`|biến|—|Con trỏ vào flash, có thể bị đổi ở bước ③|
|`ap806addr`|—|—|**Khai báo rồi không dùng**|

---

# Phần 6 · Bốn điểm cần dè chừng

**🔴 Không kiểm checksum.** Header mang sẵn `prolog_checksum` và `boot_image_checksum` nhưng cả hai bị bỏ qua. Chỉ 4 byte magic được kiểm. Ảnh ATF hỏng vẫn được chép và cho chạy — A72 sẽ nhảy vào rác.

**🔴 Thất bại là im lặng và vĩnh viễn.** Nhánh `return` ở bước ③ chỉ in một dòng, không đặt mã lỗi, không kích watchdog. AP806 nằm trong reset mãi mãi và `init_thread` vẫn chạy tiếp bình thường như không có gì. Từ ngoài nhìn vào: máy có điện, R4 sống, nhưng Linux không bao giờ boot.

**🟠 Địa chỉ dự phòng hard-code sai chỗ.** Fallback luôn là `BSPI_CS1_ADDR` — trên bo sản xuất đó là đầu firmware R4, không có header ATF. Fallback này thực tế vô dụng.

**🟡 `exec_addr` bị bỏ qua.** Định dạng Marvell tách `load_addr` (chép tới đâu) và `exec_addr` (chạy từ đâu) — hai giá trị có thể khác nhau. Code này giả định chúng bằng nhau và chỉ dùng `load_addr` cho cả `memcpy` lẫn `RVBAR`. Đúng với ảnh ATF hiện tại, nhưng sẽ vỡ nếu ai đó build ảnh có entry point lệch.