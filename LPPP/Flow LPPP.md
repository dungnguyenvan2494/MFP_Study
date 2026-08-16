# 1. Luồng `main()`

`main()` là **luồng boot bare-metal của R4** — không phải task, không có scheduler bên dưới. Nó chỉ có một nhiệm vụ: đưa con chip từ trạng thái sau reset đến điểm có thể trao quyền cho FreeRTOS, và trên đường đi thì đánh thức luôn AP806.


# Flow high-level — sáu giai đoạn

```
┌─ ① DỰNG MÔI TRƯỜNG C ────────────────────────────────────────┐
│  hal_main_initialize_sections()                              │
│     chép .data từ flash → LCM · xoá .bss                     │
│  ⇒ TỪ ĐÂY biến toàn cục mới dùng được                        │
└──────────────────────────────────────────────────────────────┘
┌─ ② ĐƯA LÕI VÀO TRẠNG THÁI LÀM VIỆC ──────────────────────────┐
│  hwConfigInit()          bảng cấu hình + hwTimebaseInit()    │
│  hwSetProcSpeed(12 MHz)  ghi sổ: đang chạy ref25 / 2         │
│  cpu_init()              cycle counter · MPU 5 vùng · D-cache│
│  pad_config_init()       pad mux, slew rate                  │
└──────────────────────────────────────────────────────────────┘
┌─ ③ CÓ TIẾNG NÓI ─────────────────────────────────────────────┐
│  setup_debug_print_logic()   UART debug tối giản             │
│  isForceLogOutput()          đọc cấu hình bật/tắt log        │
│  ⇒ TỪ ĐÂY mới in được ra ngoài                               │
└──────────────────────────────────────────────────────────────┘
┌─ ④ LÊN NGUỒN VÀ CLOCK ───────────────────────────────────────┐
│  pmu_phase(pmu_baseline)                                     │
│     SYS_PLL / MC_PLL / ND_PLL lên · chia clock QSPI, PI bus  │
│     hwSetProcSpeed(200 MHz)  ← nâng từ 12 lên 200            │
│  in banner (build time, ZP/ZN)                               │
│  avs_set_from_efuse(1097)    hiệu chỉnh điện áp theo lô chip │
└──────────────────────────────────────────────────────────────┘
┌─ ⑤ BẬT NGOẠI VI CỐT LÕI ─────────────────────────────────────┐
│  intInit()               GIC — phải trước mọi intAttach      │
│  LPPP_McChangeFreq_init()  ngắt đổi clock QSPI của MC        │
│  LPPP_McWDT_init()         ngắt watchdog của MC              │
│  init_MHU()              mailbox tới AP806                   │
│  gpio_init() · app_gpio_init()                               │
│  AppPrintInit()          printf thật                         │
└──────────────────────────────────────────────────────────────┘
┌─ ⑥ ĐÁNH THỨC AP806 ★ ────────────────────────────────────────┐
│  early_board_init(false)                                     │
│     turn_on_qspi · mci_init + iris_init · init_ddr(false)    │
│     copy_run_atf(0xF4000000)  ══► cụm A72 bắt đầu chạy ATF   │
└──────────────────────────────────────────────────────────────┘
┌─ ⑦ TRAO QUYỀN CHO RTOS ──────────────────────────────────────┐
│  AppInit_Initialize()                                        │
│     SysApiInit_Initialize → SysMemory_Init → SysSystem_Init  │
│     → AppInit_StartThreads → power_up_system                 │
│     → vTaskStartScheduler()   ← KHÔNG BAO GIỜ RETURN         │
│  while (1);                   ← lưới an toàn                 │
└──────────────────────────────────────────────────────────────┘
```

# Vì sao thứ tự phải đúng như vậy

Đây mới là phần đáng đọc — mỗi bước đều bị ràng buộc bởi bước trước:

| Bước                        | Phụ thuộc vào                  | Vì sao                                                                                                                                           |
| --------------------------- | ------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------ |
| `cpu_init()`                | `hwConfigInit()`               | Nó gọi `cpu_initialize_mpu(hwGetRamSize() - hwGetUncachedRamSize())` — hai hàm này đọc `hw_config_table`, biến toàn cục do `hwConfigInit()` điền |
| mọi biến toàn cục           | `initialize_sections()`        | `.data` còn nằm trong flash, `.bss` còn là rác                                                                                                   |
| `setup_debug_print_logic()` | `pad_config_init()`            | UART cần pad được mux đúng chức năng                                                                                                             |
| mọi `ApiSysDebug_Printf`    | `setup_debug_print_logic()`    | Ba dòng lệnh đầu `main()` **im lặng tuyệt đối** — nếu chết ở đó thì không có manh mối nào                                                        |
| `LPPP_McWDT_init()`         | `intInit()`                    | Nó gọi `intAttach` — GIC phải sẵn sàng                                                                                                           |
| `early_board_init()`        | `gpio_init()` + `pmu_baseline` | Cần GPIO để dò board type, cần clock để chạy QSPI và DDR                                                                                         |
| `copy_run_atf()`            | `init_ddr()`                   | Ảnh ATF **được chép vào DDR** rồi mới chạy — không XIP như firmware R4                                                                           |
| `AppInit_Initialize()`      | tất cả                         | Scheduler chỉ nên chạy khi phần cứng đã ổn định                                                                                                  |

---

# Bốn chi tiết đáng chú ý

## ① Cơ chế "hook" — công tắc vá được lúc chạy

```c
const uint8_t initialize_sections[] = "5=initialize_sections";

void hal_main_initialize_sections(void) {
    const volatile uint8_t * hook = (const volatile uint8_t *)&initialize_sections[0];
    if (*hook & 1)  fast_copy_section(&_data_crc_end, &__data_start__, &__data_end__);
    if (*hook & 4)  fast_fill_section(0, &__bss_start__, &__bss_end__);
}
```

Ký tự đầu `'5'` = `0x35` = `0b0011_0101` → bit0 = 1 (chép `.data`), bit2 = 1 (xoá `.bss`).

Đây là **công tắc gỡ lỗi tìm được bằng `strings`**: kỹ sư có thể vá byte đó trong ảnh nhị phân hoặc qua debugger để tắt từng bước mà không cần build lại. Cùng mẫu này xuất hiện khắp firmware — `polling_functions[]`, `set_speed[]` trong `AppInit_Initialize`, `dump_nvm[]` trong `sys_init.c`.

`volatile` là bắt buộc: nếu không, compiler sẽ thấy hằng số và tối ưu bỏ luôn câu `if`.

## ② `hwSetProcSpeed` chỉ là ghi sổ, không đổi phần cứng

```c
void hwSetProcSpeed(uint32_t mhz) { hw_config_table.proc_speed_mhz = mhz; }
```

Nó **không** động vào PLL. Giá trị này chỉ để `cpu_spin_delay()` tính số vòng lặp. Nên trình tự thật là:

|Thời điểm|Giá trị ghi sổ|Clock thật|
|---|---|---|
|`hwConfigInit()`|12|chạy từ ref 25 MHz|
|`main()` gọi `hwSetProcSpeed(SYS_BUS_CLOCK_D2)`|12|(thừa — `hwConfigInit` đã đặt)|
|bên trong `pmu_phase(pmu_baseline)`|**200**|SYS_PLL đã khoá ở 400 MHz, chia 2|

Nếu ai đó đảo thứ tự và để `cpu_spin_delay` chạy với con số sai, mọi delay tính bằng µs trong firmware sẽ lệch **17 lần**.

## ③ Driver UART đầy đủ bị loại vì quá tốn RAM

```c
#if __UNUSED_BECAUSE_TOO_BIG
    // this takes about 64k of ram
    void *_uart;  uartPreInit(&_uart);  ...
#else
    setup_debug_print_logic();
#endif
```

64 KB trên tổng 256 KB LCM — một phần tư bộ nhớ chỉ để in log. Chi tiết này cho thấy áp lực bộ nhớ thực tế của con R4, và giải thích vì sao heap SYSTEM chỉ 23 KB.

## ④ `while(1)` là code chết có chủ đích

`AppInit_Initialize()` → `SysApiInit_Initialize()` → `vTaskStartScheduler()`. Hàm này **không bao giờ return** khi scheduler khởi động thành công. `while(1)` chỉ bắt trường hợp scheduler chết vì thiếu heap — treo máy tại chỗ thay vì để `main()` return vào hư không.

---

# Một câu

> `main()` đưa R4 từ reset tới trạng thái đủ điều kiện chạy RTOS, theo trình tự **runtime C → lõi + MPU → tiếng nói → clock/nguồn → ngoại vi → DDR + ATF → scheduler**. Điểm bất thường duy nhất so với một `main()` embedded thông thường là bước ⑥: nó **thả một CPU khác (AP806) chạy hệ điều hành khác (Linux)** trước khi hệ điều hành của chính nó khởi động.


# 1. `AppInit_Initialize`

`AppInit_Initialize()` là **cửa vào của tầng ứng dụng** — điểm mà `main()` của HAL trao quyền cho toàn bộ phần còn lại của firmware. Bản thân nó chỉ có hai câu lệnh có nghĩa, nhưng nó là **hàm cuối cùng `main()` gọi và không bao giờ return**.

# Nó làm gì

```c
void AppInit_Initialize(void)
{
    SYS_INIT_CFG *cfg = { SYS_INIT_USECASE_SDK };     // ① cấu hình use-case

    const volatile uint8_t * hook = &set_speed[0];    // ② công tắc đổi tốc độ chip
    if (*hook == 4) SysApiSystem_set_speed(2);        //    fast
    if (*hook == 2) SysApiSystem_set_speed(1);
    if (*hook == 1) SysApiSystem_set_speed(0);

    SysApiInit_Initialize(cfg);                        // ③ ĐI VÀ KHÔNG TRỞ LẠI
}
```

**① Chọn use-case** — SDK / ECMA / mDNS, quyết định module nào được khởi tạo.

**② Đổi tốc độ chip** — với chú thích rất rõ ràng: _"chip speed is application specific — must be done before any other hw init or scheduler start"_. Đây là chỗ duy nhất trong luồng boot mà **tầng ứng dụng** được phép nói về clock, vì nó phụ thuộc vào sản phẩm cụ thể chứ không phải vào chip.

**③ Giao quyền** — `SysApiInit_Initialize()` không bao giờ trả về.

# Chuỗi mà nó mở ra

Đây mới là giá trị thật của hàm — nó là mắt xích khiến toàn bộ hệ thống chạy:

```
AppInit_Initialize()
  └─ SysApiInit_Initialize(cfg)                            sys_init.c:76
       ├─ HalApiMemory_RamInit()
       ├─ HalApiReset_WasCold() ?
       │     cold → im lặng (SRAM còn rác, mask log chưa hợp lệ)
       │     warm → in banner bằng mask của lần chạy trước
       ├─ hook "0=dump_nvm"  → HalApiMemory_dump_nvm()
       │
       ├─ SysApi_InitApplModules(NULL)                     sys_init.c:112
       │     ├─ mem     = HalApiMemory_RamBase()   = &__free_ram_start__
       │     ├─ memsize = HalApiMemory_RamSize()   = RAM_limit − đó
       │     ├─ SysMemory_Init(&mem, &memsize)     ← dựng 2 heap: 23 KB SYS + phần dư APP
       │     ├─ SysSystem_Init(&mem, &memsize)     ← mutex, event group,
       │     │                                        tạo task SysSystem_ThreadMain (prio 1)
       │     ├─ power_up_system()                  ← tạo task init_thread (prio 3)
       │     └─ SysFilter_Init(...)
       │
       └─ vTaskStartScheduler()                    ★ SCHEDULER CHẠY, KHÔNG RETURN
```

Sau khi scheduler chạy, thứ tự do priority quyết định:

|Prio|Task|Việc|
|---|---|---|
|**3**|`init_thread`|`LPPP_KM_Init()` → tạo queue SCPI/MHU, `PWR_SW_ON` → `NORMAL`, `pmu_ap_link_up`, `pmu_system_up`, QoS → rồi `vTaskDelete(NULL)`|
|**1**|`SysSystem_ThreadMain`|gọi `AppInit_StartThreads()` → tạo `AppThread_Main`|
|**1**|`AppThread_Main`|tạo `AppMhuTest`, `PwEvtMgr`, `PwMHUEv`, `ClrWDT`|
|**0**|các task KM|phục vụ SCPI, máy trạng thái nguồn|

Chính chênh lệch priority này bảo đảm `LPPP_KM_Init()` (tạo hàng đợi) **luôn xong trước** khi `PwEvtMgr` gọi `xQueueReceive` lên hàng đợi đó.

# Ba phát hiện

## 🟠 ① Công tắc `set_speed` **chết hai lần**

```c
const uint8_t set_speed[] = "\x00=set_speed";
```

Byte đầu là `\x00` → cả ba câu `if` đều không thoả (chúng so `== 4`, `== 2`, `== 1`). Comment cũng thừa nhận: _"if none of bit[0:2] is set no speed change happens"_.

Nhưng kể cả khi vá byte đó, kết quả vẫn không đổi:

```c
// Sys/system/sys_system_api.c:301
void SysApiSystem_set_speed(uint32_t in) { HalApiChip_set_speed(in); }

// Hal/quartz/src/hal_chip.c:83
void HalApiChip_set_speed(uint32_t in) { }     // ← RỖNG
```

Trên nền tảng Quartz, tốc độ chip được đặt hoàn toàn ở tầng dưới — `pmu_phase(pmu_baseline)` dựng SYS_PLL rồi gọi `hwSetProcSpeed(SYS_BUS_CLOCK)` = 200 MHz. Cơ chế `set_speed` này là **di sản từ nền tảng Marvell khác**.

> Lưu ý cách so sánh: `if (*hook == 4)` chứ không phải `if (*hook & 4)` — khác với mẫu hook ở `hal_main_initialize_sections` (dùng `&`). Ở đây là **so bằng**, nên chỉ nhận đúng ba giá trị 1/2/4, không kết hợp được.

## 🟠 ② `cfg` thực chất là `NULL`

```c
typedef enum { SYS_INIT_USECASE_SDK, ... } SYS_INIT_USECASE;   // SDK = 0
typedef struct _SYS_INIT_CFG { SYS_INIT_USECASE usecase; } SYS_INIT_CFG;

SYS_INIT_CFG *cfg = { SYS_INIT_USECASE_SDK };
```

Đây khai báo một **con trỏ**, không phải struct. Trong C, một scalar được phép khởi tạo bằng danh sách ngoặc nhọn có một phần tử — nên câu này tương đương:

```c
SYS_INIT_CFG *cfg = 0;      // = NULL
```

Ý định gần như chắc chắn là:

```c
SYS_INIT_CFG cfg = { SYS_INIT_USECASE_SDK };
...
SysApiInit_Initialize(&cfg);
```

Vô hại **chỉ vì** `SysApiInit_Initialize(SYS_INIT_CFG* Cfg)` **không hề dùng tham số `Cfg`** trong thân hàm — toàn bộ khái niệm use-case (SDK/ECMA/mDNS) chưa bao giờ được triển khai. Nếu ngày nào đó ai đó thêm `switch (Cfg->usecase)` vào, sẽ dereference NULL ngay lần boot đầu tiên.

## 🟡 ③ Vì sao đổi tốc độ phải nằm ở đây

Comment giải thích: _"must be done before any other hw init or scheduler start"_.

Nếu đổi clock **sau** khi scheduler chạy, mọi task đang dùng `cpu_spin_delay()` (tính vòng lặp theo `hw_config_table.proc_speed_mhz`) sẽ đột ngột sai. Nếu đổi **sau** `early_board_init`, thì DDR đã được huấn luyện với tần số cũ. Đặt ở đây là **cửa sổ hẹp cuối cùng** còn an toàn: sau khi HAL xong, trước khi bất cứ thứ gì phụ thuộc thời gian bắt đầu chạy.

## 1.1. `SysApi_InitApplModules`
`SysApi_InitApplModules()` là **danh sách khởi tạo module của tầng System** — một chuỗi tuyến tính "dựng subsystem X, hỏng thì treo", chạy ngay trước khi scheduler bắt đầu.\

# Cấu trúc: ba nhóm việc

```c
void SysApi_InitApplModules(void* first_unused_memory)   // ← tham số KHÔNG dùng
{
    void*    mem     = HalApiMemory_RamBase();   // = &__free_ram_start__
    uint32_t memsize = HalApiMemory_RamSize();   // = RAM_limit − đó

    /* ── ① NỀN TẢNG ─────────────────────────────────── */
    SysMemory_Init(&mem, &memsize);     // 2 heap: 23 KB SYS + phần dư APP
    SysSystem_Init(&mem, &memsize);     // mutex, event group, task SysSystem_ThreadMain
    power_up_system();                  // ← tạo task init_thread (prio 3)

    /* ── ② NGĂN XẾP MẠNG & NGOẠI VI ─────────────────── */
    SysFilter_Init(...)                 // packet filter
    SysSocket_Init(...)                 // BSD socket
    SysLan_Init(...)                    // lwIPv6
    SysTimerIrq_Init();                 // lưu link state ban đầu
    SysTime_Init(...)                   // thời gian hệ thống
    lpp_usbd_init(...)                  // USB device — nguồn wake
    wake_pcie_init(...)                 // PCIe — nguồn wake

    /* ── ③ CHỐT LẠI ─────────────────────────────────── */
    if (FW_MEMTABLE_ID != HalMemtable_id()) while(1);   // kiểm bảng memtable
    HalApiIsr_Register(true, false, HAL_ISR_TYPE_TIM, SysTimerIrq_Isr);
    HalApiIsr_Unmask(false, HAL_ISR_TYPE_TIM);         // ← BẬT TICK, việc cuối cùng
    setup_debug_read();
}
```

Sau khi return, `SysApiInit_Initialize()` gọi `vTaskStartScheduler()`.

**Những khối bị loại lúc biên dịch** (theo `Makefile-lib-cfg`): `ARCH_8080`, `DASH`, và `USB_DEVICE` (`#CC_DFLAGS += -DUSB_DEVICE` đã bị comment). Riêng `lpp_usbd_init` và `wake_pcie_init` nằm sau `#if 1` nên **luôn** được biên dịch — khớp với việc `lpp_usbd_isr()` là một trong các nguồn phát `SYS_EVENT_WAKE_SYSTEM`.

---

# Ba điểm đáng chú ý

## ① Mẫu `(&mem, &memsize)` là **di sản chết**

Nhìn thoáng qua thì đây là một **bump allocator**: mỗi module cắt một lát RAM từ đầu rồi đẩy con trỏ tiến lên, module sau nhận phần còn lại. Đó đúng là thiết kế gốc — thời còn dùng ThreadX, mỗi module tự tạo `tx_byte_pool` riêng.

Nhưng dưới FreeRTOS, **module đầu tiên nuốt sạch**:

```c
// SysMemory_Init
system_heap_size = SYSTEM_HEAP_SIZE;            // 23 KB
app_heap_size    = *memsize - system_heap_size; // ← TẤT CẢ phần còn lại
...
*mem     += system_heap_size + app_heap_size;   // = hết vùng RAM
*memsize -= system_heap_size + app_heap_size;   // = 0
```

Nên mọi module sau đó nhận `memsize == 0`. Và quả thật — kiểm tra thân hàm của `SysSystem_Init`, `SysFilter_Init`, `SysTime_Init`, `SysLan_Init`: **không hàm nào chạm vào `*mem` hay `*memsize`**. Chúng đều dùng `pvPortMalloc` / `SysApiThread_MutexCreate` / `xTaskCreate` — tức lấy từ hai heap vừa dựng.

Hai biến `mem`/`memsize` chỉ có tác dụng thật ở **đúng một lời gọi đầu tiên**; chín chữ ký còn lại là hình thức.

Tham số `first_unused_memory` của chính hàm này cũng vậy — được truyền `NULL` từ `sys_init.c:100` và không bao giờ dùng.

## ② Chín lần `while(1)` — không có đường lùi

```c
if (SysMemory_Init(&mem, &memsize))  { while (1); }  // error initializing memory component
if (SysSystem_Init(&mem, &memsize))  { while (1); }  // error initializing system component
if (SysFilter_Init(&mem, &memsize))  { while (1); }
...
if (FW_MEMTABLE_ID != HalMemtable_id()) { while (1); }
```

Mỗi lỗi = **treo cứng ngay tại chỗ**, không log, không mã lỗi, không watchdog. Ở giai đoạn này:

- Scheduler chưa chạy → không có `ClearWatchDogCounter`
- WDT phần cứng chưa bật (nó được bật trong `ClearWatchDogCounter` sau khi scheduler chạy)

Nên nếu ai đó tăng `.bss` khiến heap không đủ, triệu chứng là **máy chết im lặng**, chỉ có mấy dòng UART trước đó. Điều duy nhất phân biệt được là comment bên cạnh mỗi `while(1)` — muốn biết chết ở đâu thì phải cắm debugger đọc PC.

## ③ Kiểm memtable — một mẹo nhỏ khéo léo

```c
// check id of memtable, this also prevent removing the table by compiler
if (FW_MEMTABLE_ID != HalMemtable_id()) { while (1); }
```

Comment nói rõ **hai mục đích**:

1. Xác nhận bảng `memtable` (danh bạ cho driver Linux/debugger) còn nguyên magic `'MeMt'`
2. **Ngăn linker/compiler loại bỏ bảng** — vì `memtable` là biến `const` không ai đọc, rất dễ bị garbage-collect khỏi ảnh firmware

Một câu `if` giải quyết cả kiểm tra tính toàn vẹn lẫn giữ chân dữ liệu. Cùng ý đồ với `volatile const` trên khai báo bảng.

---

# Thứ tự có ý nghĩa ở ba chỗ

|Vị trí|Vì sao phải ở đó|
|---|---|
|`SysMemory_Init` **đầu tiên**|Mọi thứ sau nó đều gọi `pvPortMalloc` — không có heap thì không tạo được mutex, task, queue nào|
|`power_up_system()` ngay sau `SysSystem_Init`|Nó gọi `xTaskCreate` → cần heap đã có. Đây là điểm sớm nhất an toàn. Task `init_thread` chưa chạy — chỉ được đăng ký, đợi scheduler|
|`HalApiIsr_Unmask(TIM)` **cuối cùng**|Comment ghi _"arm interrupts are supposed to be currently disabled"_. Bật tick **sau** khi mọi module đã dựng xong, để ISR không chạy vào giữa lúc cấu trúc dữ liệu còn dở|

Riêng `SysTimerIrq_Init()` (chỉ lưu `GlPhyState = HalApiLan_LinkStateGet()`) phải nằm **sau** `SysLan_Init` vì nó đọc trạng thái link — và **trước** khi ISR timer được bật.

### Tóm gọn

Hàm này là **bản kê khai module của tầng System**: dựng hai heap, tạo hai task khởi động, lên ngăn xếp mạng và các nguồn wake, rồi chốt bằng việc bật ngắt tick — mọi lỗi đều dẫn tới `while(1)` không lối thoát. Chữ ký `(&mem, &memsize)` truyền qua chín module trông như một bump allocator, nhưng thực tế chỉ module đầu tiên dùng nó — và nó lấy sạch phần RAM còn lại.

### 1.1.1. `power_up_system` -> call `init_thread`
#### `init_thread` — pha 2 của việc dựng nền tảng

Đây là **task ưu tiên cao nhất (prio 3)**, được `power_up_system()` đăng ký trước khi scheduler chạy. Nó làm toàn bộ phần việc nặng mà `main()` không thể làm, rồi tự xoá.

```c
static void init_thread(void *input)
{
    ① cpu_power_mutex      = SysApiThread_MutexCreate("cpu_power");
       pmu_transition_mutex = SysApiThread_MutexCreate("pmu_transition");

    ② LPPP_KM_Init();

    ③ get_ddr_memory_size();
    ④ if (supportTEC() != RESULT_BASE) cpu_spin_delay(100 ms);

    ⑤ pmu_phase(pmu_ap_link_up);
    ⑥ pmu_phase(pmu_system_up);
       io_update_voltage();

    ⑦ write_ddr_memory_size();
    ⑧ board_init(false);          ★ AP806 KHỞI ĐỘNG TẠI ĐÂY
    ⑨ LPPP_QoS_Init/MoChi/UL/AP;

    ⑩ vTaskDelete(NULL);
}
```

##### Từng bước

**① Hai mutex toàn cục** — `cpu_power_mutex` bảo vệ `power_cpu()` (bật/tắt core CA72), `pmu_transition_mutex` bảo vệ chuyển pha PMU. Phải tạo **đầu tiên** vì bước ⑤ đã dùng tới chúng.

**② `LPPP_KM_Init()`** — dựng toàn bộ tầng KM: 3 hàng đợi (`scpi_msgq`, `MHU`, `PowerCtrl`), 6 IRQ nội bộ, flash, GPIO, statlib, I2C. Kết thúc bằng `event_handler(PWR_SW_ON)` → `PWR_STATUS_NORMAL`.

**③ + ⑦ Một cặp đọc–ghi tinh tế**

```c
void get_ddr_memory_size()   { gpioh_value = mrvl_regread32(0xE830A700); }
void write_ddr_memory_size() { mrvl_regwrite32(0xE8244824, gpioh_value); }
                               /* Using timer2 reg for escape the GPIOh */
```

R4 đọc **strap GPIO bank H** (dung lượng LPDDR4 CH0/CH1 CS0/CS1 + hãng DRAM do phần cứng cắm sẵn), rồi cất vào **thanh ghi timer2** — một thanh ghi không dùng, làm hộp thư tạm. Lý do: các chân GPIO đó sẽ bị mux sang chức năng khác, nhưng ATF/U-Boot vẫn cần biết cấu hình DRAM. Cất trước, đọc lại sau.

**④ Chờ 100 ms cho Eagle3** — `supportTEC()` đọc Machine Type từ 2 mặt QSPI. Comment ghi _"起動時"_ (lúc khởi động), riêng Eagle3 cần khoảng nghỉ trước khi `AP_PWR_EN` lên.

**⑤ `pmu_ap_link_up`** — _"bring up AP link, turn on 806, not the CPUs"_:

```
ap806_power_on_prep · ap806_power_on(true)   ← BẬT NGUỒN AP806 (I2C → PS-CPU)
ap806_mcix4_enable · pidi_init
setup_a2_primary_windows · setup_a2_io_windows   ← cửa sổ địa chỉ A2 bus
ap806_update_voltage · icu_init · mci_init_mcix4
```

Sau bước này AP806 **có điện và có đường bus**, nhưng các lõi A72 vẫn nằm trong reset.

**⑥ `pmu_system_up`** — duyệt bảng `power_up_dev[]` bật mọi power domain còn lại: `MC_M3`, `MC_SPI0/1`, `MC_QSPI`, `UL`/VBO, `Mech_TOP`, `VBO_RX/TX`, `TSEN`… Rồi `io_update_voltage()` đi qua I2C1 tới MCP4542 chỉnh điện áp IO.

**⑧ `board_init(false)` — điểm bàn giao thật**

```
sdmmc_init · iris_detection() · ul_gpio_init
cấu hình iris_ctx cho SB0 / NB0 / NB1 (lane PCIe, reset MPP, clock ngoài)
mci_init + iris_init cho từng con Iris có mặt
set_chip_select
init_ddr(false, NULL)              ← LPDDR4 huấn luyện — bước tốn thời gian nhất
[log ATFStart qua I2C nếu tool đo bật]
copy_run_atf(0xF4000000)           ← THẢ AP806
```

**⑨ QoS** — bốn lần cấu hình băng thông AXI. Đặt **sau** `copy_run_atf` để ATF khởi động sớm nhất có thể; QoS chỉ ảnh hưởng hiệu năng, không ảnh hưởng tính đúng đắn.

**⑩ `vTaskDelete(NULL)`** — trả 1,2 KB stack (`configMINIMAL_STACK_SIZE` = 300 words) + TCB về heap SYSTEM. Với heap 23 KB thì đây là khoản đáng kể.

---

#### Ba điều đáng chú ý

##### ① Là task, nhưng hành xử như code tuần tự

`init_thread` **không bao giờ block**: mọi độ trễ đều là `cpu_spin_delay()` (busy-wait), mọi mutex đều rảnh. Ở prio 3 — cao hơn `SysSystem_ThreadMain` (1), `AppThread_Main` (1) và các task KM (0) — nó **chạy một mạch từ đầu đến `vTaskDelete`** mà không nhường CPU cho ai.

Việc gói nó thành task chỉ để có hai thứ: **stack riêng** (không ăn vào stack SVC 7 KB của `main()`), và **khả năng tự giải phóng** sau khi xong.

##### ② Vì sao phần nặng phải ở đây chứ không ở `main()`

`iris_init()`, `init_ddr()`, `pmu_phase()` đều gọi xuống các API cần **mutex và heap** — mà cả hai chỉ tồn tại sau `SysMemory_Init()` + `SysSystem_Init()`, tức sau khi scheduler khởi động. Đó là lý do việc dựng nền tảng bị **chẻ làm hai pha**:

|Pha|Ở đâu|Làm gì|
|---|---|---|
|1|`main()` → `early_board_init()`|`turn_on_qspi()` + gán clock MCI — chỉ ghi thanh ghi trần|
|2|`init_thread` → `board_init()`|Iris, DDR, ATF — cần heap/mutex/PMU|

##### ③ Cửa sổ mù vẫn tồn tại, nhưng lý do khác

Khi `copy_run_atf()` thả AP806 (bước ⑧), hàng đợi SCPI **đã có** từ bước ②. Nhưng:

- `AppMhuTest` **chưa được tạo** — nó do `AppThread_Main` tạo, mà `AppThread_Main` do `SysSystem_ThreadMain` (prio 1) tạo, và task prio 1 chưa được lượt chạy vì `init_thread` prio 3 đang giữ CPU.
- `intAttach(get_mhu_intnum(), mhu_handler)` nằm **bên trong** `AppMhuTest` → **ngắt MHU chưa được bật**.
- `init_MHU()` gọi ở `main()` là hàm **rỗng** (`hal_MHU.c:66`), không làm gì.

Nên trong khoảng từ lúc AP806 chạy tới lúc `init_thread` gọi `vTaskDelete(NULL)`, **LPPP vẫn không nhận được lệnh SCPI nào**. Khoảng đó gồm: `LPPP_QoS_Init` + `SetupMoChi` + `SetupUL` + `SetupAP` — khoảng 40 lần ghi thanh ghi cộng ~25 dòng debug print. Ngắn, nhưng có thật.

##### 1.1.1.1.1. `board_init_emu800`
`board_init_emu800()` là **bước dựng phần cứng ngoại vi cuối cùng** trước khi trao quyền cho AP806. Toàn bộ hàm được tổ chức quanh một tham số: `resume`.

#### Năm việc

```
① DÒ PHẦN CỨNG THỰC TẾ         [chỉ cold boot]
     sdmmc_init()          — khởi tạo eMMC/SD
     iris_detection()      — đọc PIO xem có mấy con Iris cắm trên bo
     ul_gpio_init()        — chặn counter UL đếm nhầm lúc bật nguồn

② ĐIỀN BẢNG MÔ TẢ IRIS          [luôn chạy]
     cho SB0, và NB0/NB1 nếu ① phát hiện có:
       base_addr · lane[0..1] = PCIe · chân reset MPP · nguồn clock

③ DỰNG LIÊN KẾT + KHỞI TẠO IRIS
     mci_init(...)   dựng liên kết MoChi
     iris_init(...)  cấp nguồn, khoá PLL, cấu hình SerDes, ICU
     (riêng SB0: nếu rev < B thì lane[1] đổi PCIe → SATA)

④ set_chip_select()             — chọn lại chip select của QSPI

⑤ KHỞI ĐỘNG HỆ THỐNG LỚN        [chỉ cold boot]
     init_ddr(false, NULL)       — huấn luyện LPDDR4
     set_chip_select()           — lần nữa
     [log ATFStart qua I2C tới PS-CPU nếu tool đo bật]
     copy_run_atf(BSPI_CS0_ADDR) ← THẢ AP806 CHẠY ATF
```

---

#### Vai trò của `resume`

|                     | `resume == false` (cold boot) | `resume == true` (thức từ Sleep2/ErP)                 |
| ------------------- | ----------------------------- | ----------------------------------------------------- |
| Dò phần cứng ①      | ✅                             | ❌ — kết quả đã lưu trong `static iris_detect`         |
| Điền bảng ②         | ✅                             | ✅                                                     |
| Iris SB0/NB0 ③      | ✅                             | ❌                                                     |
| Iris NB1 ③          | ✅                             | ✅ ⚠️                                                  |
| `set_chip_select` ④ | ✅                             | ✅                                                     |
| DDR + ATF ⑤         | ✅                             | ❌ — DDR chỉ thoát self-refresh, ATF vẫn nằm trong RAM |

Đây chính là cơ chế làm resume nhanh hơn cold boot rất nhiều: **bỏ qua huấn luyện DDR và bỏ qua việc chép lại ATF**.

---

# Ba điểm đáng để ý

**⚠️ NB1 không nhất quán.** SB0 và NB0 đều có `resume == false &&` ở điều kiện, riêng NB1 thì không — nên nó **được khởi tạo lại mỗi lần resume**. Nhìn cách viết (khoảng trắng, thụt lề khác hẳn hai khối trên) thì đây là sửa chữa thêm vào sau, có thể cố ý mà cũng có thể là sót.

**`set_chip_select()` gọi hai lần** với cùng một comment: _"This should be unnecessary after emu800 calls turn_on_qspi() with an appropriate value in bscr_regval!"_ — một workaround tạm chưa ai dọn. Vì R4 chạy XIP từ QSPI, chọn nhầm chip select là mất luôn đường lấy lệnh.

**`iris_detect` là `static`** — cố ý, để giá trị dò được ở lần cold boot còn dùng được cho mọi lần resume sau. Nếu là biến cục bộ thì mỗi lần resume sẽ đọc rác.

