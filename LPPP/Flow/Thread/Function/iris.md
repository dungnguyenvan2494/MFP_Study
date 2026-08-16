**IRIS là tên mã của một con chip/die I/O đồng hành (companion die) của Marvell** — không phải một khối logic bên trong Quartz. Nó nối vào SoC qua liên kết **MoChi/MCI** và mang theo các ngoại vi tốc độ cao: GbE, USB, PCIe, SATA, cùng một bộ gom ngắt riêng (ICU).

---

## 1 · Bằng chứng từ source

**Nó là thiết bị vật lý riêng, không nằm trong dải PMU thường** ([pmu_lib.h:205-208](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/pmu_lib.h#L205-L208)):

```c
pmu_device_end  = 73,
pmu_SB_iris     = 74,   // this device is NOT handled in same manner,
pmu_NB0_iris    = 75,   //   physically different domain;
pmu_NB1_iris    = 76,   //   includes: TOP, GBE, USB, PCIe
```

Chú ý `pmu_device_end = 73` — ba Iris nằm **ngoài** dải device thường. Đó chính là lý do `case 0x1b` phải có nhánh riêng: `power_set_device()` không xử lý được chúng.

**Có tối đa 3 con, tên theo vị trí** ([iris.h:10-16](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/iris.h#L10-L16)):

```c
typedef enum {
    IRIS_SB_0 = 0,     // phía South Bridge  (die Quartz — nơi R4/LPPP ở)
    IRIS_NB_0,         // phía North Bridge  (AP806 — nơi CA72 ở), port 0
    IRIS_NB_1,         //                                          port 1
    IRIS_NB_1_DL,      // Downlink trên Iris NB 1
    IRIS_CNT
} quartz_iris_t;
```

**Địa chỉ = cổng MoChi** ([regAddrs.h:148-151](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/a0/include/regAddrs.h#L148-L151)):

```c
#define SB_IRIS0_ADDR   0xD0000000                      // = SB_MOCHIx2_DATA_ADDR
#define NB_IRIS0_ADDR   NB_MOCHIx2_0_CONTROLLER_ADDR    // 0xCC100000
#define NB_IRIS1_ADDR   NB_MOCHIx2_1_CONTROLLER_ADDR    // 0xCC200000
// comment: "we can access Iris modules via the A2 win for controller"
```

Mỗi Iris ngồi sau một **MoChi x2 link**. Đây là kiến trúc chip-to-chip quen thuộc của Marvell Armada 8K, nơi die ứng dụng (AP) nối với die I/O (CP) qua MoChi — Iris chính là vai "CP" trong hệ này.

---

## 2 · Bên trong Iris có gì

Bốn khối con, mã hoá thành 4 byte trong một `uint32_t` ([power_api.h:142-149](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/power_api.h#L142-L149)):

```
// BYTES:  [ TOP : PCIE : USB : GBE ]
#define POWER_ACTION_GBE    (0)     // byte 0  — Gigabit Ethernet MAC
#define POWER_ACTION_USB    (1)     // byte 1  — USB controller
#define POWER_ACTION_PCIE   (2)     // byte 2  — PCIe controller
#define POWER_ACTION_TOP    (3)     // byte 3  — hạ tầng chung: PLL, clock, MoChi link
```

Mỗi byte mang các bit hành động:

```c
POWER_ACTION_PD   (1<<0)   // power down
POWER_ACTION_PU   (1<<1)   // power up
POWER_ACTION_PI   (1<<2)   // power island
POWER_ACTION_CLK  (1<<3)   // clock
POWER_ACTION_LINK (1<<4)   // MoChi link
```

Ngoài ra `iris.c` cho thấy Iris còn có:

- **COMPHY SerDes** 2 lane, cấu hình được thành `LM_SATA`, `LM_PCIE`, `LM_PCIE_RC` ([iris.h:18-23](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/iris.h#L18-L23))
- **PLL riêng** — `iris.c` có vòng chờ _"locking PLL..... PLL1 locked on iris %08x"_
- **ICU** (Interrupt Consolidation Unit) — `icu_init(base_addr)`, và `iris_icu_resume()`

---

## 3 · Vì sao `case 0x1b` phải tách nhánh cho `pmu_NB0_iris`

```c
if (domain == pmu_NB0_iris) {                              // == 75, ngoài pmu_device_end
    err = pmu_iris_actions((op_point & 0xff) == 0 ? POWER_IRIS_PCIE_PU
                                                  : POWER_IRIS_PCIE_PD, domain);
    ...
    break;                                                  // KHÔNG rơi xuống dưới
}
err = power_set_device(domain, op_point & 0xff, op_point >> 8);
```

Ba lý do, đọc được trực tiếp từ code:

1. **Khác không gian số.** `domain = 75 ≥ pmu_device_end = 73` → `power_set_device()` sẽ coi là out-of-range.
2. **Khác cơ chế.** Iris nằm bên kia liên kết MoChi, nên bật/tắt nguồn phải đi qua giao thức riêng (`pmu_iris_actions`) chứ không phải ghi thanh ghi PMU nội bộ.
3. **Chỉ điều khiển PCIe.** Nhánh này chỉ đụng `POWER_IRIS_PCIE_PU/PD` — không động tới GBE/USB/TOP. Comment ghi rõ mục đích: _`2018/04/10 Sleep1→Normal時にPCIeのドメインをONにする`_ (bật lại domain PCIe khi thoát Sleep1).

Bản vá `OP_BTS-34391` (2021/06/22) — _"IRIS電源制御失敗時にリトライ"_ — thêm retry vào bên trong `pmu_iris_actions` và kiểm tra `err` để trả `SCPI_E_DEVICE`, cho thấy điều khiển nguồn Iris **thực tế có thất bại** ngoài hiện trường.

---

## 4 · Iris trong toàn cảnh LPPP

```
┌──────────────── AP806 die (North Bridge) ─────────────────┐
│  4× Cortex-A72  ·  Linux + ATF                            │
│        │ MoChi x2 port 0 ──────► [ IRIS NB0 ] GbE/USB/PCIe│
│        │ MoChi x2 port 1 ──────► [ IRIS NB1 ] GbE/USB/PCIe│
└────────┼──────────────────────────────────────────────────┘
         │ MoChi x4
┌────────▼──── Quartz die (South Bridge) ───────────────────┐
│  Cortex-R4 (LPPP)  ·  FreeRTOS                            │
│        │ MoChi x2 ────────────► [ IRIS SB0 ] GbE/USB/PCIe │
│  print engine, PMU, APB timers, GPIO, QSPI…               │
└───────────────────────────────────────────────────────────┘
```

**Phát hiện Iris lúc boot** ([iris.h:48-53](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/iris.h#L48-L53)) — đọc thanh ghi PIO của khối UL:

```c
#define UL_yPI2_PIO_REG   (0xC0042008)
#define IRIS_DETECT_SB0   (0x01)
#define IRIS_DETECT_NB0   (0x02)
#define IRIS_DETECT_NB1   (0x04)
```

`iris_detection()` trả bitmap → `board.c` chỉ init những con thực sự gắn trên bo. Tức **số lượng Iris khác nhau tuỳ model máy in**.

**Trình tự khởi tạo** ([board.c:385-402](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/board.c#L385-L402)):

```c
if (resume == false && mci_init(IRIS_SB_0) == 0)   iris_init(IRIS_SB_0, iris_ctx);
if (init_iris_nb0   && mci_init(IRIS_NB_0) == 0)   iris_init(IRIS_NB_0, iris_ctx);
if (init_iris_nb1   && mci_init(IRIS_NB_1) == 0)   iris_init(IRIS_NB_1, iris_ctx);
```

`mci_init()` (dựng liên kết MoChi) **phải xong trước** `iris_init()` — vì mọi truy cập thanh ghi Iris đều đi qua liên kết đó. Nếu link không lên, `init_iris_nb1 = false` và con đó bị bỏ hẳn (_"Don't try and resume NB1 if it failed on initial boot"_).

**GPIO reset riêng** ([LPPP_Gpio.h:43-44](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Gpio.h#L43-L44)) — LPPP giữ chân reset của hai Iris:

```c
#define LPPP_GPIOPIN_IRIS1_RESETN   74
#define LPPP_GPIOPIN_IRIS0_RESETN   75
```

---

## 5 · Vì sao Iris quan trọng với luồng nguồn bạn đang trace

Nhớ lại đoạn resume trong `power_cpu()`:

```c
intDisable(INTNUM_MCIX2_1);   // tắt ngắt GMAC phía LPPP
clear_wake_ints();
wake_lan();                   // "re-route gmac int back to AP via iris ICU"
```

**Card mạng vật lý nằm trong Iris.** Khi máy ngủ, ngắt GbE được ICU của Iris định tuyến về **R4** để LPPP tự nghe gói Wake-on-LAN. Khi thức dậy, `wake_lan()` đổi định tuyến ở ICU để ngắt đi thẳng về **CA72** cho Linux dùng. Iris chính là điểm chuyển giao quyền sở hữu card mạng giữa hai nhân.

Tương tự, `LPPP_TR8_TR10_Sleep2_Or_ErPToSleep1` gọi `LPPP_QoS_SetupMoChi()` để nạp lại các thanh ghi QoS của MCIx4/MCIx2 — chính là băng thông trên các liên kết dẫn tới Iris, bị mất giá trị sau khi power domain tắt.

---

## Tóm một câu

> **Iris** là die I/O rời của Marvell (vai trò tương đương CP110 trong Armada 8K), gắn vào Quartz và AP806 qua liên kết MoChi x2, cung cấp GbE/USB/PCIe/SATA + ICU. Trong `case 0x1b`, `pmu_NB0_iris` là con Iris trên cổng MoChi 0 của AP806 — Linux dùng nó cho PCIe, và vì nó nằm ngoài dải PMU nội bộ (`≥ pmu_device_end`) nên phải điều khiển qua `pmu_iris_actions()` thay vì `power_set_device()`.