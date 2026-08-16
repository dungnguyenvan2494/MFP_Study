**cụ thể hơn:** Linux không "nạp firmware vào RAM cho LPPP", mà **ghi thẳng vào chip QSPI flash** nơi firmware LPPP nằm, sau khi đã **cho LPPP ngủ**.

# Phần A · Bản đồ flash — hai chip select, một controller

Từ DTS [quartz-sb.dtsi:136-180](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/arch/arm64/boot/dts/marvell/quartz-sb.dtsi#L136-L180):

```dts
bspi0: compatible = "mrvl,bspi0";
    reg = <0x8 0xe8273000 0 0x80>;        ← CÙNG một controller
    map_memory = <0xf8000000>;  map_memory_size = <0x200000>;   /* 2 MB */
    partition@0 { label = "lppp-spi-firmware";  reg = <0x000000 0x200000>; };

bspi1: compatible = "mrvl,bspi1";
    reg = <0x8 0xe8273000 0 0x80>;        ← CÙNG một controller
    map_memory = <0xf4000000>;  map_memory_size = <0x400000>;   /* 4 MB */
    partition@0 { label = "spi-romfs";  reg = <0        0x300000>; };
    partition@1 { label = "spi-user";   reg = <0x300000 0x100000>; };
```

Ghép với hằng số trong driver ([mrvl-bspi_winbond.c:576-584](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/drivers/mtd/spi-nor/mrvl-bspi_winbond.c#L576-L584)) và trong LPPP ([LPPP_Flash.h:19](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Flash.h#L19)):

```
┌──── QSPI CS[1] @ 0xF8000000, 2 MB ─────────────────────────┐
│ 0x000000 ── 0x200000   "lppp-spi-firmware"  ← mvri.bin     │
│                         LPPP_CHIP_SELECT = 1               │
└────────────────────────────────────────────────────────────┘

┌──── QSPI CS[0] @ 0xF4000000, 4 MB ─────────────────────────┐
│ 0x000000 ── 0x300000   "spi-romfs"   (BootROM/U-Boot)      │
│                         BOOTROM_CHIP_SELECT = 0            │
│ 0x300000 ── 0x400000   "spi-user"    User Data 512 KB      │
│    ├─ 0x381800–0x382000   LPDDR4 param                     │
│    ├─ 0x377000 (+0x07)    cờ mất điện, vùng 1  ← LPPP ghi  │
│    ├─ 0x378000 (+0x07)    cờ mất điện, vùng 2  ← LPPP ghi  │
│    ├─ 0x379000 / 0x37A000 Machine Type + counter           │
│    ├─ 0x3FF01C            B014 register        ← LPPP ghi  │
│    └─ 0x3FD000–0x3FE000   Secure boot area                 │
└────────────────────────────────────────────────────────────┘
```

**Điểm mấu chốt: cả hai node DTS trỏ vào cùng thanh ghi `0xE8273000`.** Chỉ có **một** bộ điều khiển BOOTSPI trên chip, và **cả R4 lẫn CA72 đều dùng nó**. Đây là gốc rễ của toàn bộ cơ chế đồng bộ ở Phần C.

---

# Phần B · Build ra cái gì

[build_lppp.sh](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/build_lppp.sh) chỉ là lớp bọc chọn `BOARD` rồi gọi `make`:

```bash
./build_lppp.sh eglz          # BOARD=eaglez
make CROSS_COMPILE=s800-r4- --no-print-directory
cp -a mvri.bin output/eglz
```

12 cấu hình bo mạch: `es, emu, egl, eglz, dnbmlk, spa, eglb, eglbz, eglzp, eglbzp, hemlk, mssb` — khớp với danh sách `MACHINEINFO_*` mà `getMachineType()` đọc từ flash.

Cách ghép `mvri.bin` khá đặc biệt ([Makefile:117-124](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Makefile#L117-L124)):

```make
$(TARGETNAME)_gnu.bin: $(TARGETNAME).elf
	$(ARMOC) -j .reset                  -O binary $< $@        # ① chỉ section .reset
	$(ARMOC) -R .rewrite-lppp -R .reset -O binary $< $@.ROM    # ② mọi thứ TRỪ .reset
	$(CAT) $@.ROM >> $@                                        # ③ nối ②  sau  ①
```

`.reset` chứa `hal_vectors_*.o` và `hal_init_*.o` ([memory_sections.ld:40-46](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/linker_common/memory_sections.ld#L40-L46)) — vector table + code khởi động, phải nằm ở **offset 0** của ảnh.

Sau đó thêm checksum:

```make
$(ARMVLOG) ... -little-endian-checksum-negative -maximum-addr ... 4 4 -o mvri.bin
```

→ `mvri.bin` mang **negative checksum 4 byte ở cuối** để loader kiểm tra tính toàn vẹn.

> 🔎 `-R .rewrite-lppp` loại một section tên `.rewrite-lppp` ra khỏi ảnh ROM. Nhưng **section này không được định nghĩa ở bất kỳ linker script nào** trong repo — tôi đã grep cả `memory_sections.ld`. Đây là tàn dư của thiết kế `rewrite_roop()` đã bị vô hiệu hoá (_"動作しない且つ使用しないため無効にした"_). Cờ `-R` hiện là no-op. _(Đã xác nhận từ source.)_

---

# Phần C · Cơ chế then chốt — Linux phải cho LPPP ngủ trước khi ghi

Đây là phần thú vị nhất, và nó nối thẳng vào `case 0x0a` mà bạn đã phân tích ở lượt trước.

Driver MTD cài hai hook `prepare`/`unprepare` ([mrvl-bspi_winbond.c:153-168](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/drivers/mtd/spi-nor/mrvl-bspi_winbond.c#L153-L168)):

```c
static int mrvl_prepare(struct spi_nor *nor) {
    disable_clk_r4(true);       /* lppp を停止させる */
    return 0;
}
static void mrvl_unprepare(struct spi_nor *nor) {
    disable_clk_r4(false);      /* lppp を再開させる */
}
```

Và `disable_clk_r4()` ([dòng 122-151](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/drivers/mtd/spi-nor/mrvl-bspi_winbond.c#L122-L151)):

```c
static int disable_clk_r4(bool disable)
{
    struct scpi_ops *scpi = get_scpi_ops();
    if (disable) {
        scpi->dvfs_set_idx(0, 1);        // ★ shut down the r4
        usleep_range(1000, 1000);        /* LPPP停止レスポンスから1ms待つ (LPPPフリーズ対策) */
    } else {
        scpi->dvfs_set_idx(0, 0);
    }
    return 0;
}
```

`dvfs_set_idx` → **`SCPI_CMD_SET_DVFS = 0x0a`** ([arm_scpi.c:109](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/drivers/firmware/arm_scpi.c#L109)), payload `{u8 domain, u8 index}`.

Nối vào phía LPPP:

```c
case 0xa: {
    uint32_t domain   =  received_payload[0] & 0xff;         // = 0
    uint32_t op_point = (received_payload[0] & 0xff00) >> 8;  // = 1
    ...
    if (domain == 0 && op_point == 1)
        pause_cpu(chan);          // ★ R4 tự trả response rồi vào WFI
    else
        send_response = true;     /* 再開時にレスポンスを返す */
}
```

**Vòng tròn khép lại:**

```
Linux muốn ghi QSPI
   └─ mtd write → spi_nor → mrvl_prepare()
        └─ disable_clk_r4(true)
             └─ SCPI 0x0a {domain=0, idx=1}
                  └─ [LPPP] process_scpi_cmd case 0xa
                       └─ pause_cpu(chan)
                            ├─ lưu + tắt toàn bộ GIC
                            ├─ clear_MHU + write_MHU  ← TỰ trả response TRƯỚC khi ngủ
                            └─ __asm("wfi")            ← R4 ĐỨNG IM
   └─ usleep(1ms)  ← chờ chắc chắn R4 đã dừng
   └─ mrvl_qspi_erase / mrvl_qspi_write   ← Linux độc chiếm BOOTSPI controller
   └─ mrvl_unprepare()
        └─ disable_clk_r4(false)
             └─ SCPI 0x0a {domain=0, idx=0}
                  └─ MHU interrupt đánh thức R4 khỏi WFI
                       └─ khôi phục GIC, trả response bình thường
```

`pause_cpu()` chừa đúng **một** ngắt sống (`DUMMY_INT_NUM`) làm đường thức dậy — chính là ngắt MHU mang lệnh `0x0a` kế tiếp. Vì vậy `pause_cpu` **phải** gửi response trước khi `wfi`, nếu không `AppMhuTest` sẽ rung chuông lần hai rồi kẹt busy-wait trong khi R4 đang ngủ.

Comment `LPPPフリーズ対策` (biện pháp chống treo LPPP) và bug OP_BTS-28498 (_"QSPI割り込み関数を使用するとLPPPがフリーズする"_) cho thấy chuyện tranh chấp controller này **đã từng gây treo máy thật**.

---

# Phần D · Ba lớp chặn ghi vào vùng LPPP

Vùng `lppp-spi-firmware` mặc định **bị khoá ghi**. [mrvl_qspi_write, dòng 599-607](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/drivers/mtd/spi-nor/mrvl-bspi_winbond.c#L599-L607):

```c
if (is_write_protect_lpddr4param &&
    ( (cs == LPDDR4PARAM_CHIP_SELECT && ...0x381800–0x382000...) ||   // tham số LPDDR4
      (cs == BOOTROM_CHIP_SELECT     && ...0x000000–0x300000...) ||   // BootROM
      (cs == BOOTROM_CHIP_SELECT     && ...0x3FD000–0x3FE000...) ||   // Secure boot
      (cs == LPPP_CHIP_SELECT        && ...0x000000–0x200000...) ) ) { // ★ LPPP FW
    printk("%s : write error cs:%d start:%d size:%d\n", ...);
    mrvl_qspi_set_new_detail_code();
    return -EIO;
}
```

Cùng một kiểm tra được lặp lại trong `mrvl_qspi_erase` (dòng 755).

Biến `is_write_protect_lpddr4param` mở khoá theo **hai đường**:

**① Tự động theo kiểu boot** ([dòng 1210-1224](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/drivers/mtd/spi-nor/mrvl-bspi_winbond.c#L1210-L1224)):

```c
switch (c_BootType) {
case 2:  /* USB_UPDATE  */
case 3:  /* DEF_IISW    */
case 5:  /* BOOT_ERR    */
case 9:  /* DEF_UPDATE  */
case 11: /* DEF_BACKUP  */
case 12: /* DEF_RESTORE */
    is_write_protect_lpddr4param = 0;   // MỞ KHOÁ
    break;
default:
    is_write_protect_lpddr4param = 1;   // khoá
}
```

→ Máy phải **boot vào chế độ update** (cắm USB firmware, hoặc vào menu service) thì vùng LPPP mới ghi được. Boot bình thường thì không.

**② Thủ công qua sysfs** ([dòng 934-945](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/drivers/mtd/spi-nor/mrvl-bspi_winbond.c#L934-L945)):

```c
static DEVICE_ATTR(write_protect_lpddr4param, 0664, ..._show, ..._store);
```

→ `echo 0 > /sys/.../write_protect_lpddr4param`

Đây là bản vá `OP_BTS-34245` (2021/06/14) — _"ライトプロテクト領域の追加"_ — thêm BootROM và LPPP vào danh sách bảo vệ. Trước đó chỉ có vùng tham số LPDDR4.

---

# Phần E · SCPI `0x89 REWRITE_REQUEST` — vai trò thật

Đường đi từ userspace:

```
userspace: echo 1 > /sys/devices/.../LPPP_REWRITE_REQUEST
   └─ BspLpppRewriteRequestStore()              mdels800_pmu.c:836
        └─ BspLPPPRewriteRequest()              mdels800_pmu.c:801
             └─ get_km_scpi_ops()->rewrite_request()
                  └─ km_scpi_rewrite_request()  km_scpi.c:234
                       └─ scpi_send_message_ext(KM_SCPI_CMD_REWRITE_REQUEST /*0x89*/, NULL, 0, ...)
```

Phía LPPP nhận được... và **không làm gì cả**:

```c
case KM_SCPI_CMD_REWRITE_REQUEST :   /* LPPP ROM書き替え要求(0x89) */
    /* 処理は「レスポンス = ON」のみ */
    send_response = true;
    break;
```

Và ở `AppMhuTest`:

```c
if (msg.scpi_header.command_id == KM_SCPI_CMD_REWRITE_REQUEST) {
    /*rewrite_roop();*/  /* 動作しない且つ使用しないため無効にした */
}
```

**Ý định ban đầu:** khi Linux sắp ghi đè ROM của LPPP, R4 phải nhảy vào một vòng lặp chạy từ RAM (`main_rewrite_roop()` — tắt ngắt rồi `while(1)`), vì trong lúc flash bị xoá thì không thể fetch lệnh từ nó.

**Thực tế:** cơ chế đó đã bị bỏ. Việc dừng R4 được làm bằng **`pause_cpu()` qua SCPI `0x0a`** ở tầng driver MTD — sạch hơn và không cần section RAM riêng. Lệnh `0x89` giờ chỉ còn là một **thông báo mang tính nghi thức**, giữ lại cho tương thích ngược.

---

# Phần F · LPPP tự ghi flash — cơ chế RAM execution

Song song với đường Linux, **LPPP cũng tự ghi flash** (cờ mất điện, B-register, log CA72WDT). Ở đây vẫn cần chạy code từ RAM.

Linker gom `LPPP_Flash.o` vào đầu `.text` và đánh dấu hai đầu ([memory_sections.ld:101-105](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/linker_common/memory_sections.ld#L101-L105)):

```ld
.text : ALIGN(0x8) {
    __lppp_flash_start__ = .;
    *LPPP_Flash.o (.text .text.* .gnu.linkonce.t.*)
    __lppp_flash_end__ = .;
    *(.text .text.* ...)
```

Rồi `flash_modify_sub()` tự sao chép và nhảy vào bản sao ([LPPP_FlashSub.c:48-112](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_FlashSub.c#L48-L112)):

```c
static void flash_code_copy_to_ram(void) {
    if (copy_flag == 0) {
        memcpy(&__qspi_ram_code_start__, &__lppp_flash_start__,
               &__lppp_flash_end__ - &__lppp_flash_start__);
        cpu_dcache_writeback_all();       // đẩy code vừa chép xuống bộ nhớ
        cpu_icache_invalidate_all();      // ép I-cache nạp lại
        __asm volatile("isb");            // đồng bộ pipeline
        copy_flag = 1;
    }
}

int flash_modify_sub(uint32_t dst, const unsigned char *src, uint32_t size) {
    flash_code_copy_to_ram();
    /* tính địa chỉ flash_modify() TRONG BẢN SAO Ở RAM */
    flash_mod = (flash_modify_func)((uint32_t)&__qspi_ram_code_start__
              + ((uint32_t)flash_modify - (uint32_t)&__lppp_flash_start__));

    int_state = cpu_disable_interrupts();   // không cho ISR nào chạy từ flash
    ret = flash_mod(dst, src, size);
    cpu_restore_interrupts(int_state);
    return ret;
}
```

Ba chi tiết đáng học:

- **Phép tính địa chỉ**: offset của `flash_modify` trong section gốc được cộng vào base của bản sao → tìm ra entry point tương ứng trong RAM.
- **Bộ ba cache**: writeback D-cache (code vừa ghi còn nằm trong D-cache) → invalidate I-cache (chưa biết vùng RAM đó là code) → `isb` (xả pipeline). Thiếu bất kỳ bước nào là nhảy vào rác.
- **Tắt ngắt toàn cục** trong suốt thao tác flash — nếu một ISR chạy từ vùng flash đang bị xoá thì treo ngay.

Bản `#if 0` cũ của `LPPP_SpiFlashWrite_PowerFlickerFlag` dùng buffer `static unsigned char buf[4096]` và `flash_read`/`flash_write` trực tiếp; bản hiện tại (2018/05/10) chuyển sang `flash_modify_sub` chính vì lý do này.

---

# Phần G · Quy trình nạp firmware đầy đủ

```
① Build trên PC
   ./build_lppp.sh eglz
      → mvri.elf → objcopy(.reset) ++ objcopy(-R .reset) → +checksum
      → output/eglz/mvri.bin  (≤ 2 MB)

② Đóng gói vào bộ firmware của máy (cùng kernel, rootfs, BootROM…)

③ Máy boot vào chế độ update
   c_BootType ∈ {2 USB_UPDATE, 3 DEF_IISW, 5 BOOT_ERR, 9 DEF_UPDATE, 11 BACKUP, 12 RESTORE}
      → is_write_protect_lpddr4param = 0     ← MỞ KHOÁ vùng LPPP

④ Userspace (updater) ghi vào MTD partition
   flashcp mvri.bin /dev/mtdX         (X = "lppp-spi-firmware")
      │
      ▼ mtd → spi_nor → spi_nor_driver_ops
   ┌──────────────────── với MỖI erase/write ────────────────────┐
   │ mrvl_prepare()   → SCPI 0x0a{0,1} → LPPP pause_cpu() → WFI  │
   │                  → usleep(1ms)                              │
   │ mrvl_qspi_erase() / mrvl_qspi_write()                       │
   │    ├─ kiểm tra write-protect (đã mở ở ③)                    │
   │    ├─ check_busy + retry loop 10 000 × 100 µs               │
   │    ├─ Write Enable → Quad Page Program (0x32) theo page     │
   │    └─ wait_for_write_complete                               │
   │ mrvl_unprepare() → SCPI 0x0a{0,0} → MHU IRQ đánh thức R4    │
   └─────────────────────────────────────────────────────────────┘

⑤ Reboot
   BootROM đọc CS[1] @ 0xF8000000, kiểm checksum, nạp/khởi động R4
   → LPPP_KM_Init() → PWR_SW_ON → PWR_STATUS_NORMAL
```

_(Bước ⑤ là **suy luận** — tôi không có source của boot ROM cho R4 trong repo này.)_

---

# Phần H · Những điểm đáng chú ý

## 🔴 H.1 Một controller, hai chủ — không có khoá phần cứng

`BOOTSPI @ 0xE8273000` được cả R4 và CA72 truy cập. Cơ chế loại trừ duy nhất là _"bảo R4 đi ngủ rồi chờ 1 ms"_ — một **thoả thuận phần mềm**, không phải semaphore phần cứng.

Nếu R4 đang ở giữa một `flash_modify_sub()` (đã tắt ngắt, đang xoá sector) đúng lúc Linux gửi `0x0a{0,1}`, thì lệnh SCPI đó **không được xử lý** (ngắt MHU đang bị tắt). Linux chờ hết `usleep(1ms)` rồi cứ thế ghi → **hai master cùng thao tác một chip flash**.

Các bug đã ghi nhận đều xoay quanh chuyện này:

- `OP_BTS-28170` — _"Oops発生でLPPPがフリーズ"_
- `OP_BTS-28498` — _"QSPI割り込み関数を使用するとLPPPがフリーズする"_ → phải bỏ hàm ngắt QSPI, chuyển sang polling
- Comment `LPPPフリーズ対策` ngay tại `usleep_range(1000,1000)`

## 🟠 H.2 Không có rollback nếu mất điện giữa chừng

Vùng `lppp-spi-firmware` chỉ có **một bản** (0x000000–0x200000), không có A/B partition. Mất điện giữa lúc ghi → LPPP không boot được → máy chết cứng (LPPP là con quản lý nguồn, nó chết thì không ai bật được CA72).

Đối chiếu: các vùng dữ liệu nhỏ _có_ redundancy 2 mặt (`0x377000`/`0x378000` cho cờ mất điện, `0x379000`/`0x37A000` cho Machine Type với write-counter để chọn mặt mới hơn). Nhưng chính firmware thì không.

Cơ chế phục hồi duy nhất: `c_BootType == 5 /* BOOT_ERR */` cũng mở khoá ghi — tức nếu boot lỗi thì vẫn nạp lại được, **miễn là CA72 còn boot lên được**. Mà CA72 boot được lại phụ thuộc LPPP bật nguồn cho nó. Đây là một phụ thuộc vòng.

## 🟠 H.3 Kiểm tra write-protect chỉ có ở tầng driver

`is_write_protect_lpddr4param` là biến kernel, mở được bằng một dòng `echo` qua sysfs với quyền `0664`. Không có bảo vệ ở tầng phần cứng (block protect bits của chip Winbond) hay ở tầng flash chip.

Vùng `SECURE_AREA` (`0x3FD000–0x3FE000`, thêm 2024/08/21 _"セキュアブート対応"_) cũng nằm chung cơ chế này.

## 🟡 H.4 `rewrite_roop()` — thiết kế bị bỏ nhưng dấu vết còn khắp nơi

Vẫn còn: file `app_rewrite-loop.c`, cờ `-R .rewrite-lppp` trong Makefile, `case 0x89` trong LPPP, `km_scpi_rewrite_request()` trong Linux, sysfs `LPPP_REWRITE_REQUEST`, prototype `rewrite_roop()`. Toàn bộ chuỗi này **chạy được nhưng không có tác dụng gì**. Ai đọc code lần đầu rất dễ tưởng đây là đường nạp chính.

## 🟡 H.5 Watchdog phải tắt khi update

Nhớ lại `LPPP_Task.c:54-55`:

```c
int is_wdt_clear = 1;
int enable_wdt   = 1;
/* SCPI等でenable_wdtを落とすとLPPP FWアップデート時等においてWDTを停止させる事が可能 */
```

Vì trong lúc update, R4 nằm im trong `wfi` rất lâu → `ClearWatchDogCounter` không chạy → WDT phần cứng sẽ reset máy. Phải hạ `enable_wdt` trước.

Tương tự, `BspLpppCa72wdtStore` (sysfs `CA72WDT`) gửi `KM_SCPI_WDT_CA72_END` với comment: _"将来的には複数のwdを制御できるようにする可能性があるが現状は何らかの書き込みがあればwdを停止する"_ — bất kỳ ghi gì vào sysfs này cũng tắt CA72WDT, chuẩn bị cho các thao tác dài.

---

# Tóm lại

> Firmware LPPP (`mvri.bin`) nằm ở **QSPI chip-select 1, offset 0, 2 MB**, và được nạp bằng cách Linux **ghi trực tiếp vào MTD partition `lppp-spi-firmware`**. Vì R4 và CA72 dùng chung một bộ điều khiển BOOTSPI, mỗi thao tác erase/write đều được bọc bởi `mrvl_prepare/unprepare` → gửi SCPI `0x0a{domain=0, idx=1}` để **đẩy R4 vào `wfi`**, ghi xong thì `0x0a{0,0}` đánh thức lại. Vùng này mặc định write-protected và chỉ mở khi máy boot ở chế độ update. Lệnh SCPI `0x89 REWRITE_REQUEST` — thứ _trông giống_ đường nạp — thực tế là code chết.