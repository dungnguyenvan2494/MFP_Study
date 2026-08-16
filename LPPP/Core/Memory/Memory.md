Tôi đã đọc cả folder cùng linker script và cấu hình MPU. Trước khi vào `Sys/memory`, cần dựng bản đồ bộ nhớ thật của R4 — vì mọi thứ trong folder đó chỉ có nghĩa khi đặt lên bản đồ này.

---

# Phần 1 · Bản đồ bộ nhớ của Cortex-R4

## 1.1 Hai vùng vật lý

[asic/linker_common/memory_map.ld](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/linker_common/memory_map.ld):

```ld
MEMORY {
    rom (rx)  : ORIGIN = 0xF8000000, LENGTH = 128M - 0x10000   /* bspi */
    ram (rwx) : ORIGIN = 0xE8200000, LENGTH = 256K - 0x10      /* 256k lcm,
                                                     last 8 bytes for ap806 use */
}
```

Hai tên gọi này **gây hiểu nhầm nghiêm trọng** nếu đọc lướt:

|Alias|Địa chỉ|Thực chất là gì|
|---|---|---|
|`rom`|`0xF8000000`|**Cửa sổ memory-mapped của QSPI CS[1]** — chính là partition `lppp-spi-firmware`. R4 **XIP trực tiếp từ flash**.|
|`ram`|`0xE8200000`|**LCM** — SRAM on-chip 256 KB, **Linux cũng nhìn thấy được**|

Đây là mảnh ghép còn thiếu từ lượt trước về nạp firmware: **R4 chạy lệnh thẳng từ QSPI**. Đó là lý do `flash_modify_sub()` bắt buộc phải sao `LPPP_Flash.o` sang SRAM và tắt ngắt trước khi xoá flash — nếu không, CPU sẽ cố fetch lệnh từ một con flash đang bận erase.

`R0BOOT (0x00000000, 512 B)` trong bản `88pa6220` đã bị comment ở bản `asic` → không dùng trên chip sản xuất.

## 1.2 Bố cục 256 KB LCM

Từ [memory_sections.ld](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/linker_common/memory_sections.ld):

```
0xE8200000 ┌─ __start_ram__ = __nocache_start__ ──────────────────────┐
           │  .usbd2_qh_sec    USB2 device queue heads   0x0800  2 KB │ ← Linux thấy
           │  ────────────────────────────────────────────────────────│
0xE8200800 │  .fifosec         HCI FIFO (Linux ↔ R4)     0x1880 6.1KB │ ← Linux thấy
           │  ────────────────────────────────────────────────────────│
0xE8202080 │  .netbufsec       packet descriptor (GMAC DMA)           │
           │  .bufpools        buffer pool                            │
           │                                                          │
0xE8214000 └─ __nocache_end__ = start + 80 KB  ← PHẢI khớp MPU vùng 4 ┘
           ┌─ .text? KHÔNG — .text nằm ở ROM ────────────────────────┐
           │  .data          (nạp từ ROM lúc boot)                    │
           │  .data.nv       (.nvramsec, NOLOAD — sống qua reset mềm) │
           │  .bss                                                     │
           │  .stack  __stack_start__                                  │
           │     ├─ SVC  0x1C00 = 7,00 KB  ← dùng từ khi vào main()   │
           │     ├─ IRQ  0x0300 = 768 B                                │
           │     ├─ FIQ  0x0010 = 16 B                                 │
           │     ├─ UND  0x0010 = 16 B                                 │
           │     └─ ABT  0x0010 = 16 B      ─── tổng ≈ 7,8 KB          │
           │  .qspi_ram_code_start  ← bản sao LPPP_Flash.o chạy từ RAM │
           │  __free_ram_start__    ← HEAP BẮT ĐẦU TỪ ĐÂY             │
0xE823FFF0 └─ RAM_limit  (16 byte cuối để dành cho AP806) ────────────┘
```

## 1.3 Vùng non-cached là **cửa sổ chia sẻ với Linux**

Đây là điểm thiết kế quan trọng nhất. Linker script ghi:

```ld
__qhead_start__ = .;  /* start addr must match usbd_qhead_lcm in Linux DTS */
__data_fifo_start__ = .;  /* start addr must match net-proxy-hci lcm in Linux DTS */
```

Và phía Linux, [quartz-sb.dtsi:58-68](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/arch/arm64/boot/dts/marvell/quartz-sb.dtsi#L58-L68) đối ứng chính xác:

```dts
usbd_qhead_lcm: lcm@8e8200000 {   /* 32 x 64byte usbd qhead slots */
    compatible = "mmio-sram";
    reg = <0x8 0xe8200000 0 0x800>;
    /* start addr and size must match .usbd2_qh_sec in
       ...\r4-freertos\Hal\quartz\asic\linker_common\memory_sections.ld */
};
lcm@e8200800 {
    compatible = "net-proxy-hci";
    reg = <0x8 0xe8200800 0 0x1880>;
    /* start addr and size must match .fifosec in ... */
};
```

Cả hai bên **trỏ tay vào file của nhau trong comment** — một hợp đồng bằng lời, không có cơ chế nào kiểm tra tự động. Đổi `0x1880` ở một bên mà quên bên kia là hỏng câm lặng, giống hệt vấn đề với `SCPI_CHANNEL_BUFFER_SIZE` ở tầng MHU.

Đây là **kênh IPC thứ hai** giữa Linux và LPPP, song song với SCPI/MHU: SCPI dùng cho lệnh điều khiển nguồn, còn HCI FIFO ở LCM dùng cho luồng dữ liệu mạng (network proxy).

## 1.4 MPU — 5 vùng

[cortex_r4_mpu_config.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/hardware/devices/config/cortex_r4_mpu_config.c). Comment đầu file: _"all .address MUST align to boundry of .size !!"_

|#|Địa chỉ|Kích thước|Kiểu|Vai trò|
|---|---|---|---|---|
|0|`0x00000000`|4 GB|`NOCACHE`|Nền — mặc định cho mọi thứ chưa phủ|
|1|`0xC0000000`|1 GB|`DEVICE`|Toàn bộ không gian thanh ghi ngoại vi|
|2|`0xE8200000`|256 KB|`WBWACACHE`|LCM — **write-back, write-allocate**|
|3|`0xF8000000`|128 MB|`ROCACHE`|Flash XIP — **chỉ đọc, có cache**|
|4|`0xE8200000`|128 KB, mask `0xE000`|`NOCACHE`|80 KB đầu LCM — **không cache**|

**Cơ chế chồng lấn:** trong MPU của ARM, **vùng có số hiệu cao hơn thắng**. Nên:

- Vùng 1 phủ `0xC0000000–0xFFFFFFFF` là DEVICE
- Vùng 2 khoét ra 256 KB LCM thành cached
- Vùng 3 khoét ra 128 MB flash thành read-only cached
- Vùng 4 khoét tiếp 80 KB đầu LCM thành non-cached

**Giải mã `subregionMask = 0xE000`:** vùng 4 khai báo 128 KB, chia thành 8 subregion × 16 KB. `0xE000` = bits 15,14,13 → tắt 3 subregion cuối. Còn `8 − 3 = 5` subregion × 16 KB = **80 KB** — khớp chính xác `__nocache_end__ = __nocache_start__ + (80 * 1024)`.

Comment trong linker script cảnh báo đúng điều này:

```ld
/* If the amount of data in nocache region falls below the MPU's next lowest
   NOCACHE subregion boundary, the NOCACHE region should be downsized in
   cortex_r4_mpu_config.c */
__nocache_end__ = __nocache_start__ + (80 * 1024);
```

Kiểm tra căn chỉnh (yêu cầu bắt buộc của MPU R4):

- `0xE8200000 ÷ 0x40000` (256 KB) = `0x3A08` chẵn ✓
- `0xE8200000 ÷ 0x20000` (128 KB) = `0x74100` chẵn ✓
- `0xF8000000 ÷ 0x8000000` (128 MB) = `31` chẵn ✓

---

# Phần 2 · Folder `Sys/memory` — hai nhóm file không liên quan nhau

```
Sys/memory/
├── sys_memory.c / .h / sys_api_memory.h   ← ① QUẢN LÝ HEAP
├── sys_memtable.c / .h                     ← ② BẢNG MÔ TẢ RAM
└── fw_*.h  (8 file)                        ← ② định nghĩa cấu trúc cho bảng
```

Cái tên "memory" gộp hai thứ rất khác:

||① `sys_memory.c`|② `sys_memtable.c`|
|---|---|---|
|Làm gì|Cấp phát/giải phóng heap lúc chạy|Tạo một **bảng hằng số trong flash**|
|Sống ở đâu|RAM|ROM (`.data.memtable`)|
|Ai dùng|Code trong LPPP|**Tác nhân bên ngoài**: driver Linux, debugger|
|Chạy khi nào|Suốt vòng đời|Không chạy — chỉ là dữ liệu tĩnh|

---

# Phần 3 · `sys_memory.c` — hai heap tách biệt

## 3.1 Vì sao có hai heap?

```c
static void* SysMemory_SystemBytePool;   /* cho tầng System */
static void* SysMemory_AppLayerBytePool; /* cho tầng Application */
```

Kiến trúc SDK chia ba tầng **HAL → Sys → App**. Hai heap tách biệt để **lỗi tràn heap ở tầng ứng dụng không giết tầng hệ thống** — network stack, driver, IPC vẫn sống được để báo lỗi ra ngoài.

Hai API song song:

|Tầng System|Tầng App|Backend|
|---|---|---|
|`SysMemory_Alloc()`|`SysApiMemory_Alloc()`|`pvPortMalloc()` / `pvPortAppMalloc()`|
|`SysMemory_Free()`|`SysApiMemory_Free()`|`vPortFree()` / `vPortAppFree()`|
|`SysMemory_Zalloc()`|`SysApiMemory_Zalloc()`|alloc + `memset(0)`|

## 3.2 `SysMemory_Init()` — chia RAM còn lại

Được gọi từ [sys_init.c:114-118](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Sys/init/sys_init.c#L114-L118):

```c
void* mem      = HalApiMemory_RamBase();   // = &__free_ram_start__
uint32_t memsize = HalApiMemory_RamSize(); // = RAM_limit - __free_ram_start__
if (SysMemory_Init(&mem, &memsize)) { while(1); }
```

`HalApiMemory_RamBase/Size` ([hal_memory.c:113-121](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/src/hal_memory.c#L113-L121)) chỉ là hai phép trừ trên ký hiệu linker — **toàn bộ phần RAM còn sót sau .bss/.stack/.qspi_ram_code trở thành heap**.

Thân hàm:

```c
system_heap_size = SYSTEM_HEAP_SIZE;              // ① lấy phần cố định trước
if (system_heap_size > *memsize) {
    "requested sys memory not available. stopping!";
    while(1);                                      //    ⚠ treo máy vĩnh viễn
}
app_heap_size = *memsize - system_heap_size;      // ② App lấy TOÀN BỘ phần còn lại
if (app_heap_size < APP_HEAP_SIZE) {
    "requested app memory not available. stopping!";
    while(1);                                      //    ⚠ treo máy vĩnh viễn
}

SysMemory_SystemBytePool   = *mem;                 // ③ đặt hai pool liền nhau
SysMemory_AppLayerBytePool = *mem + system_heap_size;

pvPortInit(SysMemory_SystemBytePool,   system_heap_size, 0 /*MEM_SYS*/);
pvPortInit(SysMemory_AppLayerBytePool, app_heap_size,    1 /*MEM_APP*/);

*mem     += system_heap_size + app_heap_size;      // ④ trả phần thừa cho caller
*memsize -= system_heap_size + app_heap_size;
```

**Chiến lược phân bổ bất đối xứng:** `SYSTEM_HEAP_SIZE` là **giá trị cố định**, còn App heap **nuốt toàn bộ phần dư**; `APP_HEAP_SIZE` chỉ là mức sàn kiểm tra. Comment trong [Makefile-lib-cfg:54](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Makefile-lib-cfg#L54) nói rõ: _"the application heap will get assigned the rest of available SRAM and will be at last APP_HEAP_SIZE big."_

Cấu hình ([Makefile-lib-cfg:56-60](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Makefile-lib-cfg#L56-L60)):

```make
#SYSTEM_HEAP_SIZE := "(19*1024)"     ← giá trị cũ, đã bỏ
SYSTEM_HEAP_SIZE  := "(23*1024)"     ← 23 KB
APP_HEAP_SIZE     := "(3*1024)"      ←  3 KB (mức sàn)
SYSTEM_STACK_SIZE := "(1600)"
APP_STACK_SIZE    := "(2048)"
```

Truyền vào code qua `-D` ([Makefile-lib-cfg:245-246](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Makefile-lib-cfg#L245-L246)) chứ không qua header — nên grep trong `.h` sẽ không thấy.

## 3.3 Allocator: `heap_mrvl.c` — biến thể của `heap_4`

Marvell nhân đôi `heap_4` (first-fit + coalescing) thành hai thể hiện với hai bộ biến trạng thái:

```c
static BlockLink_t xStart,    *pxEnd    = NULL;   // danh sách free của MEM_SYS
static BlockLink_t xAppStart, *pxAppEnd = NULL;   // danh sách free của MEM_APP

void *pvPortMalloc(size_t n)    { return pvGenPortMalloc(n, MEM_SYS); }
void *pvPortAppMalloc(size_t n) { return pvGenPortMalloc(n, MEM_APP); }
```

`pvGenPortMalloc()` chọn cặp `(pxItStart, pxItEnd, maxSize)` theo `mem_type` rồi chạy thuật toán `heap_4` chuẩn: duyệt danh sách free theo địa chỉ tăng dần, lấy khối đầu tiên đủ lớn, tách phần thừa, và gộp khối liền kề khi free.

Điểm cần biết: **`pvPortMalloc` của FreeRTOS được chuyển hướng vào heap SYSTEM**. Nghĩa là mọi thứ kernel tự cấp phát — TCB, stack của task, queue, semaphore, event group — đều ăn vào 23 KB đó. Đây là lý do `SYSTEM_HEAP_SIZE` được nâng từ 19 KB lên 23 KB.

## 3.4 EEPROM / NVRAM

```c
MV_RC SysApiMemory_NvRamRead(...)  { return RC_ERR_NOT_AVAILABLE; }
MV_RC SysApiMemory_NvRamWrite(...) { return RC_ERR_NOT_AVAILABLE; }
uint32_t SysApiMemory_NvRamSize(void) { return 0; }
```

NVRAM là **stub** — không có trên nền tảng này.

EEPROM thì có thật, bọc trong critical section:

```c
state_critical = SysSystem_EnterCritical();
HalApiEeprom_Get_Size(&size);
if (size >= (Offs + Len)) state_read = HalApiEeprom_Read(Buf, Offs, Len);
else { state_read = 1; "requested EEPROM memory not available."; }
SysSystem_ExitCritical(state_critical);
```

Kiểm tra biên trước khi đọc/ghi — điểm sáng hiếm hoi trong codebase này.

---

# Phần 4 · `sys_memtable.c` — bảng tự mô tả cho tác nhân bên ngoài

## 4.1 Ý tưởng

```c
volatile const struct fw_memtable memtable __attribute__((section(".memtable"))) =
{
    TABLE_HEAD,                                    /* id='MeMt', version='V1.0' */
    {
        TABLE_ENTRY(fw_mt_id_printbuffer, &PrintBuf,     sizeof(PrintBuf)),
        TABLE_ENTRY(fw_mt_id_comfifo,     &ComFifo,      sizeof(ComFifo)),
        TABLE_ENTRY(fw_mt_id_version,     &Version_Info, sizeof(Version_Info)),
#ifdef ENABLE_SCA
        TABLE_ENTRY(fw_mt_id_scacfg_v1,   &DriverConfigSca, sizeof(...)),
#endif
#ifdef ENABLE_NPO
        TABLE_ENTRY(fw_mt_id_npocaps_v1,  &FwCapabilitiesNpo, sizeof(...)),
#endif
#ifdef ENABLE_WOL
        TABLE_ENTRY(fw_mt_id_wolcaps_v1,  &FwCapabilitiesWol, sizeof(...)),
#endif
        TABLE_ENTRY(fw_mt_id_linkspeedcaps_v1, &FwCapabilitiesLinkSpeed, sizeof(...)),
        TABLE_ENTRY_END,
    },
};
```

Mỗi entry là `{ id, length, start }` — **một danh bạ địa chỉ**.

Magic number ([fw_memtable.h:45-46](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Sys/memory/fw_memtable.h#L45-L46)):

```c
#define FW_MEMTABLE_ID   0x4d654d74   /* 'MeMt' */
#define FW_MEMTABLE_VER  0x56312e30   /* 'V1.0' */
```

Bảng được đặt ở vị trí cố định trong ảnh firmware ([memory_sections.ld:214-220](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/linker_common/memory_sections.ld#L214-L220)):

```ld
.data.memtable : ALIGN(8) {
    __data_memtable_start = .;
    *(.memtable)
    __data_memtable_end = .;
} > rom              ← trong FLASH, ngay sau _etext
```

## 4.2 Giải quyết vấn đề gì

Driver Linux (hoặc kỹ sư cầm debugger) muốn đọc **print buffer** của R4 để xem log, hoặc đọc **chuỗi version**, hoặc biết firmware này hỗ trợ bao nhiêu wake pattern. Không có memtable thì phải:

- biên dịch driver cùng với header của firmware, và
- rebuild driver mỗi lần layout firmware đổi.

Với memtable: **quét flash tìm magic `'MeMt'`, đọc bảng, tra theo `id`** → tìm được địa chỉ và kích thước mà không cần biết gì về nội bộ firmware. Phiên bản firmware đổi, bảng đổi theo, driver không cần rebuild.

## 4.3 Nội dung capability được sinh từ macro build-time

```c
#if defined(NUM_ARP_OFFLOAD_ENTRIES) && defined(NUM_NS_OFFLOAD_ENTRIES)
#define ENABLE_NPO
#endif
```

Ba khối `SCA` / `NPO` / `WOL` chỉ tồn tại nếu **toàn bộ** macro liên quan được định nghĩa. Rồi giá trị macro được đóng gói thành bitfield:

```c
const struct fw_wol_capabilities FwCapabilitiesWol = {
    .hdr = { .version = FW_WOL_CAPABILITIES_V1, .length = FW_WOL_CAPABILITIES_V1_SIZE },
    .config.v1.bits = {
        .num_wake_pattern_entries  = (NUM_WAKE_PATTERN_ENTRIES),
        .num_ipv4_tcp_syn_entries  = (NUM_IPV4_TCP_SYN_PATTERN_ENTRIES),
        .num_ipv6_tcp_syn_entries  = (NUM_IPV6_TCP_SYN_PATTERN_ENTRIES),
        .report_wake_reason        = (REPORT_WAKE_REASON) ? 1 : 0,
        .wake_on_magic_packet      = (ENABLE_WAKE_ON_MAGIC_PACKET) ? 1 : 0,
        ...
    }
};
```

Đây là cách firmware **tự khai báo năng lực** cho driver: driver đọc bảng này rồi mới biết được phép cấu hình bao nhiêu wake pattern. Nối thẳng vào `g_wolMode` mà bạn đã gặp ở `power_cpu()` — quyết định Sleep2 hay ErP.

⚠️ Có một lỗi copy-paste: khối `SCA` dùng nhầm hằng số của NPO:

```c
const struct fw_sca_config DriverConfigSca = {
    .hdr = {
        .version = FW_NPO_CAPABILITIES_V1,      // ← phải là FW_SCA_*
        .length  = FW_NPO_CAPABILITIES_V1_SIZE, // ← phải là FW_SCA_*
    },
```

Chỉ có hại nếu `ENABLE_SCA` được bật và hai bộ hằng số khác giá trị.

Hàm duy nhất trong file:

```c
uint32_t HalMemtable_id(void) { return memtable.head.id; }
```

— để code tự kiểm tra bảng có nguyên vẹn không.

---

# Phần 5 · Con số tổng hợp

|Hạng mục|Giá trị|
|---|---|
|Flash XIP (`rom`)|`0xF8000000`, cửa sổ 128 MB − 64 KB · ảnh thật ≤ 2 MB|
|SRAM LCM (`ram`)|`0xE8200000`, **256 KB − 16 B**|
|Vùng non-cached|80 KB đầu LCM (`0xE8200000`–`0xE8214000`)|
|├─ USB2 qhead|2 KB — chia sẻ với Linux|
|├─ HCI FIFO|6,125 KB — chia sẻ với Linux|
|└─ netbuf + bufpool|~72 KB — DMA của GMAC|
|Stack (5 chế độ ARM)|7,8 KB · SVC chiếm 7 KB|
|Heap SYSTEM|23 KB (cố định) — gồm cả heap của FreeRTOS|
|Heap APP|≥ 3 KB, thực tế = phần RAM còn dư|
|16 byte cuối LCM|để dành cho AP806|

---

# Phần 6 · Những điểm cần dè chừng

## 🔴 6.1 Hai lần `while(1)` không lối thoát

```c
if (system_heap_size > *memsize) { "...stopping!"; while(1); }
if (app_heap_size < APP_HEAP_SIZE) { "...stopping!"; while(1); }
```

Chạy trong `SysApi_InitApplModules()` — **trước `vTaskStartScheduler()`**. Máy treo cứng, watchdog chưa kịp bật, không có cách nào phục hồi ngoài rút điện. Nếu ai đó tăng `.bss` khiến `__free_ram_start__` bò lên quá cao, đây là triệu chứng sẽ gặp: **máy chết im lặng ngay lúc boot**, chỉ có một dòng log qua UART.

## 🟠 6.2 Ba hợp đồng địa chỉ bằng comment, không có kiểm tra

|Hằng số|Nơi 1|Nơi 2|Nơi 3|
|---|---|---|---|
|`0xE8200000`, `0x800`|`memory_sections.ld`|Linux DTS `usbd_qhead_lcm`|driver USB `sram_qhead`|
|`0xE8200800`, `0x1880`|`memory_sections.ld`|Linux DTS `net-proxy-hci`|driver net-proxy|
|80 KB nocache|`memory_sections.ld`|`cortex_r4_mpu_config.c` mask `0xE000`|—|

Comment ở cả hai phía đều trỏ tay vào file bên kia, nhưng **không có build-time assert nào**. Cùng dạng lỗ hổng với `SCPI_CHANNEL_BUFFER_SIZE` (firmware ↔ ATF ↔ DTS) đã gặp ở tầng MHU.

Riêng cặp linker↔MPU thì linker script có một mẹo hay để bắt lỗi:

```ld
__nocache_end__ = __nocache_start__ + (80 * 1024);
. = __nocache_end__;   /* this will catch an overflow
                          (ld error:"cannot move location counter backwards") */
```

Nếu dữ liệu vượt 80 KB, **link sẽ fail** thay vì hỏng lúc chạy. Kỹ thuật tương tự cũng dùng cho `__qhead_end__` và `__data_fifo_end__`.

## 🟠 6.3 Kích thước `uint16_t` giới hạn cấp phát ở 64 KB

```c
void* SysMemory_Alloc(uint16_t Size);
void* SysApiMemory_Alloc(uint16_t Size);
void  SysMemory_MemCpy(uint8_t *pDest, uint8_t *pSource, uint16_t numbBytes);
```

Không thể cấp phát quá 65535 byte, và `SysMemory_MemCpy` không copy được khối lớn hơn. Với heap chỉ 23 + 3 KB thì không bao giờ chạm giới hạn — nhưng API sẽ **cắt cụt im lặng** nếu ai truyền `size_t` lớn hơn, vì phép ép kiểu xảy ra ngầm.

## 🟠 6.4 `SysMemory_Free` không kiểm tra NULL, bản App thì có

```c
void SysApiMemory_Free(void* Mem) {
    if (Mem) { vPortAppFree(Mem); }     // ← có kiểm tra
}
void SysMemory_Free(void* Mem) {
    vPortFree(Mem);                      // ← KHÔNG kiểm tra
}
```

`vPortFree` của FreeRTOS tự bỏ qua NULL, nên vô hại — nhưng sự bất đối xứng giữa hai API song song dễ gây hiểu nhầm.

## 🟡 6.5 Bình luận đầu file mâu thuẫn với thực tế khoá

```c
/*
regarding SysSystem_MutexSystemLayerApiGet:
all memory allocation/etc. functions do themselves protect for concurrent access.
so it is not neccessary to handle with mutexes or similar on the level of present module.
*/
```

Đúng cho `pvPortMalloc/vPortFree` (heap_mrvl dùng `vTaskSuspendAll`). Nhưng **hai hàm EEPROM lại tự lấy critical section** — tức tác giả không hoàn toàn tin vào chính lời khẳng định đó. Khác biệt là hợp lý (EEPROM đi qua HAL, không phải heap), nhưng comment nên nói rõ phạm vi.

## 🟡 6.6 `SysMemory_MemCmp` trả **số byte khác nhau**, không phải dấu

```c
uint16_t SysMemory_MemCmp(uint8_t* pDestArea, uint8_t* pSourceArea, uint16_t numbBytes)
{
    for (i = 0; i < numbBytes; i++)
        if (*(pDestArea++) != *(pSourceArea++)) diff++;
    return diff;  // is zero in case both memory areas are equal
}
```

Ngữ nghĩa **khác hẳn `memcmp()` chuẩn** (trả âm/0/dương). Ai quen `memcmp` mà dùng nhầm hàm này để so sánh thứ tự sẽ nhận kết quả vô nghĩa. Ngoài ra nó **không dừng sớm** — luôn duyệt hết `numbBytes` kể cả đã tìm thấy khác biệt ở byte đầu.

## 🟡 6.7 Tám file `fw_*.h` mô tả năng lực có thể không được biên dịch vào

`ENABLE_SCA` / `ENABLE_NPO` / `ENABLE_WOL` chỉ bật khi **tất cả** macro con được định nghĩa trong Makefile-cfg. Nếu thiếu một macro, cả khối biến mất khỏi memtable **mà không có cảnh báo nào** — driver Linux sẽ đơn giản là không tìm thấy entry đó và mặc định về giá trị an toàn nhất. Rất khó truy khi năng lực WoL "tự nhiên biến mất" sau khi ai đó chỉnh cấu hình build.

---

# Tóm lại

> Cortex-R4 trong LPPP **chạy XIP trực tiếp từ QSPI flash tại `0xF8000000`** và dùng **256 KB SRAM (LCM) tại `0xE8200000`** làm toàn bộ bộ nhớ đọc-ghi. 80 KB đầu của LCM được MPU đánh dấu **non-cached** vì nó vừa là vùng DMA của GMAC, vừa là **cửa sổ chia sẻ với Linux** (USB queue head + HCI FIFO) — kênh IPC thứ hai bên cạnh SCPI/MHU. Phần RAM còn lại sau `.data`/`.bss`/stack (~7,8 KB, riêng SVC 7 KB) trở thành heap, chia thành **23 KB SYSTEM cố định** (gồm cả heap của FreeRTOS) và **phần dư cho APP**.
> 
> Folder `Sys/memory` gộp hai thứ khác nhau: `sys_memory.c` là **lớp bọc hai heap** trên một biến thể `heap_4` được nhân đôi; còn `sys_memtable.c` là một **bảng hằng số trong flash** đóng vai danh bạ tự mô tả, để driver Linux và debugger tìm được print buffer, version và bảng năng lực mà không cần biên dịch chung với firmware.