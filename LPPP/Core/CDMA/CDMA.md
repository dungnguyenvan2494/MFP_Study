# Phần 1 · CDMA là gì

**CDMA = Central DMA** — bộ điều khiển DMA đa kênh trên bus AXI của Marvell. **Không liên quan gì tới CDMA viễn thông.**

Bằng chứng từ cấu trúc thanh ghi ([CDMA_AXI_core_regstructs.h:46-83](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/a0/include/CDMA_AXI_core_regstructs.h#L46-L83)):

```c
typedef struct CDMA_AXI_CORE_REGS_s {
   volatile uint32_t CFG;          // 0x00 [R/W] DMA Configuration
   volatile uint32_t Control;      // 0x04 [R/W] DMA Control
   volatile uint32_t Status;       // 0x08 [R]   DMA Status
   volatile uint32_t QOS;          // 0x0C [R/W] AXI QOS Control
   volatile uint32_t CPR;          // 0x10 [R]   CDMA Parameter (bus width…)
   volatile uint32_t intEn;        // 0x18 [R/W] Interrupt Enable
   volatile uint32_t intPend;      // 0x1C [R]   Interrupt Pending
   volatile uint32_t intAck;       // 0x20 [W]   Interrupt Acknowledge
   volatile uint32_t FillValue;    // 0x28 [R/W] giá trị memset phần cứng
   volatile uint32_t CDSR;         // 0x3C [W]   Descriptor Register  ← kick DMA
   volatile uint32_t CLDSR;        // 0x40 [R/W] Lower Descriptor Start Addr
   volatile uint32_t CUDSR;        // 0x44 [R/W] Upper Descriptor Start Addr
   volatile uint32_t CCLDR/CCUDR;  // 0x48/4C    Current Descriptor Addr
   volatile uint32_t CLNDAR/CUNDAR;// 0x50/54    Next Descriptor Addr
   volatile uint32_t CLRBAR/CURBAR;// 0x58/5C    Read Burst Addr
   volatile uint32_t CLWBAR/CUWBAR;// 0x60/64    Write Burst Addr
   volatile uint32_t CWBRR;        // 0x70 [R]   Write Bytes Remain
   volatile uint32_t CSRR/CSRLI/CSRUI; // 0x80.. Save/Restore (qua sleep)
} CDMA_AXI_CORE_REGS_t;
```

Địa chỉ, suy ra từ [LPPP_QoS.c:76-86](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_QoS.c#L76-L86) (nơi duy nhất trong code sản xuất còn nhắc tới CDMA):

```c
#define LPPP_QOS_CDMA_BASE          (0xE8271000)
#define LPPP_QOS_CDMA_AXI_CORE0_QOS (LPPP_QOS_CDMA_BASE + 0x100*1  + 0x0C)
#define LPPP_QOS_CDMA_AXI_CORE9_QOS (LPPP_QOS_CDMA_BASE + 0x100*10 + 0x0C)
```

Vì `QOS` nằm ở offset `0x0C` trong struct → **mỗi channel chiếm một khối 0x100 byte**:

```
0xE8271000  CDMA_AXI top registers (chung)
0xE8271100  channel 0   ← core regs
0xE8271200  channel 1
   …
0xE8271A00  channel 9
```

Nhưng bản thân `LPPP_QoS.c` **cũng không dùng** các macro đó — chúng chỉ được `#define` rồi bỏ đó, giống hệt số phận của cả folder.

---

# Phần 2 · Kiến trúc: descriptor + peripheral request line

## 2.1 Descriptor 4 trường, 64-bit

```c
typedef struct CDMA2MEM_DESC2_s {
    uint64_t length;
    uint64_t source;
    uint64_t destination;
    uint64_t next;          // địa chỉ descriptor kế · 3 = kết thúc
} CDMA2MEM_DESC2_t;
```

Phần mềm **không** ghi source/dest/length vào thanh ghi. Nó dựng descriptor **trong RAM**, rồi chỉ nạp **địa chỉ descriptor** vào `CLDSR`/`CUDSR` và ghi `CDSR = 1` để khởi động ([cdma.c:117-132](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/cdma/cdma.c#L117-L132)):

```c
void setDescriptorAddr(volatile CDMA_AXI_CORE_REGS_t *cdmaRegPtr, uint32_t channel)
{
    uint64_t addr64 = g_cdma_queue.cdma_struct_addr[channel];
    cdmaRegPtr->CUDSR = (uint32_t)((addr64 >> 32) & 0xFFFFFFFF);  // 32 bit cao
    cdmaRegPtr->CLDSR = (uint32_t)( addr64        & 0xFFFFFFFF);  // 32 bit thấp
    cdmaRegPtr->CDSR  = 0x1;   // start the DMAs
}
```

Địa chỉ tách làm hai thanh ghi 32-bit vì **không gian địa chỉ của SoC là 64-bit** (nhớ `NB_MOCHIx4_DATA_ADDR = 0x800000000ULL` ở phần Iris) — engine phải với tới được RAM bên kia liên kết MoChi.

## 2.2 Chuỗi descriptor — scatter-gather

Trường `next` biến các descriptor thành **danh sách liên kết**. Engine tự động chạy tiếp descriptor kế mà không cần CPU can thiệp.

`next == 3` là **cờ kết thúc** (giá trị 3 vì hai bit thấp của địa chỉ luôn bằng 0 do căn chỉnh, nên được tái sử dụng làm cờ trạng thái). Khi xếp thêm một transfer lên cùng channel, code **vá lại `next` của descriptor cuối** ([cdma.c:257-273](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/cdma/cdma.c#L257-L273)):

```c
if (mem2mem->CDMA_channel == g_cdma_queue.cdma_chan[i]) {
    if (test_inputs[i].next == 3) {            // descriptor này đang là cuối chuỗi
        test_inputs[i].next = ((uintptr_t)&test_inputs[g_cdma_queue.next_chan] & 0xFFFFFFFC);
        g_cdma_queue.cdma_type[g_cdma_queue.next_chan] = 1;   // đánh dấu "nối chuỗi"
        cpu_dcache_clean_region(&test_inputs[i], sizeof(CDMA2MEM_DESC2_t));
    }
}
```

`& 0xFFFFFFFC` xoá 2 bit thấp — vừa căn chỉnh 4 byte, vừa đảm bảo không vô tình tạo ra giá trị `3`.

## 2.3 Peripheral request line — 35 nguồn

[cdma_peripheral_ids.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/a0/include/cdma_peripheral_ids.h) liệt kê các thiết bị có thể **yêu cầu DMA bằng phần cứng**:

|ID|Thiết bị|ID|Thiết bị|
|---|---|---|---|
|0–5|UART 0/1/2 RX/TX|19–20|Encryption 0/1|
|6–9|SPI 0/1 RX/TX|21|MVDO Edge FIFO|
|10–13|I2C 0/1 RX/TX|22–25|PI SPI 0/1 RX/TX|
|14–15|SCCP RX/TX|26–27|PI SCCP RX/TX|
|16|Audio|28–29|PI I2C 0 RX/TX|
|17|IEEE1284 Parallel|30–31|ADC DMA 0/1|
|18|MVDO Mirror Facet Times|32–34|**Stepper Motor 0/1/2**|

Danh sách này vẽ ra chân dung con chip: đây là SoC điều khiển **máy in/MFP** — có động cơ bước, cổng song song IEEE1284 (cổng máy in cổ điển), MVDO (video xuất ra đầu in), ADC đo cảm biến.

---

# Phần 3 · Ba chế độ truyền

Khác biệt nằm ở trường `FLOWCTRL` trong `CFG` — ai là bên "điều tiết nhịp".

|Hàm|`FLOWCTRL`|Nguồn|Đích|Ai giữ nhịp|
|---|---|---|---|---|
|`cdma_m2m()`|`0`|RAM|RAM|CDMA chạy hết tốc lực|
|`cdma_m2p_setup()`|`1`|RAM|ngoại vi|**Ngoại vi đích** đòi dữ liệu|
|`cdma_p2m_setup()`|`2`|ngoại vi|RAM|**Ngoại vi nguồn** báo có dữ liệu|

```c
/* m2p */ cdmaRegPtr->CFG = ..._FLOWCTRL_REPLACE_VAL(cdmaRegPtr->CFG, 1);
          cdmaRegPtr->CFG = ..._DESTPID_REPLACE_VAL(cdmaRegPtr->CFG, periph_id);

/* p2m */ cdmaRegPtr->CFG = ..._FLOWCTRL_REPLACE_VAL(cdmaRegPtr->CFG, 2);
          cdmaRegPtr->CFG = ..._SRCPID_REPLACE_VAL(cdmaRegPtr->CFG, periph_id);
```

`SRCPID`/`DESTPID` nối channel với **request line** của ngoại vi. Đây là điểm mấu chốt: với m2p/p2m, CDMA **không tự do chạy** — nó chỉ chuyển một "data unit" mỗi khi ngoại vi kéo chân request. Nhờ vậy mới truyền được sang UART/SPI/I2C mà không tràn FIFO.

Thanh ghi `Control` mô tả _hình dạng_ của mỗi lần truy cập bus:

```c
control = (OWNWRITEDISABLE_MASK | OWNPOLARITY_MASK);
control |= (CDMA_source_addrinc      << SRCADDRINC_SHIFT);   // ① tăng địa chỉ nguồn?
control |= (CDMA_destination_addrinc << DESTADDRINC_SHIFT);  //    tăng địa chỉ đích?
control = ..._SRCXFERWIDTH_REPLACE_VAL(control,  src_width);  // ② độ rộng mỗi beat
control = ..._DESTXFERWIDTH_REPLACE_VAL(control, dst_width);
control = ..._SRCBURSTSIZE_REPLACE_VAL(control,  src_burst);  // ③ số beat mỗi burst
control = ..._DESTBURSTSIZE_REPLACE_VAL(control, dst_burst);
```

**①** `addrinc = 0` là chi tiết quan trọng cho ngoại vi: FIFO của UART/SPI nằm ở **một địa chỉ cố định**, nên phải tắt tăng địa chỉ ở phía đó. RAM thì `addrinc = 1`.

**②** Độ rộng lấy từ `CPR` (thanh ghi tham số phần cứng) cho phía RAM, và từ bảng cấu hình cho phía ngoại vi:

```c
if ((cdmaRegPtr->CPR & CDMA_AXI_CORE_CPR_BUSWIDTH_MASK) == 0)
     cdmaBusWidth = 2;   // 32-bit
else cdmaBusWidth = 3;   // 64-bit
```

Mã hoá `log2(bytes)`: `2` = 4 byte, `3` = 8 byte.

**`FillValue` + `CFG.FILL`** là một tính năng ít gặp: **memset bằng phần cứng**. Bật `FILL` thì engine không đọc nguồn, chỉ ghi `FillValue` ra đích. Code luôn tắt (`CFG &= ~FILL_MASK`).

---

# Phần 4 · Hai thủ thuật đáng chú ý

## 4.1 Cross-boundary — descriptor phải nằm trong tầm nhìn của DMA engine

```c
// Create a shadow CDMA2MEM structure in MCU memory space to execute
// MCU DMAs from R4 or AP cores
#define cross_bound_test_inputs (*(CDMA2MEM_DESC2_t (*)[24])(0xEE02FE00))
```

Và ở cả ba hàm setup:

```c
if (mem2mem->CDMA_channel > 15) {
    cross_bound_test_inputs[n].source      = ...;
    cross_bound_test_inputs[n].destination = ...;
    cross_bound_test_inputs[n].length      = ...;
    cross_bound_test_inputs[n].next        = ...;
    g_cdma_queue.periph_id[n] -= 18;        // dịch không gian peripheral id
    cpu_dcache_clean_region(&cross_bound_test_inputs[n], sizeof(CDMA2MEM_DESC2_t));
}
```

Ý nghĩa: **channel 0–15 thuộc CDMA của khối này, channel > 15 thuộc CDMA của "MCU"** — một lõi khác trên SoC. Engine DMA của MCU **không nhìn thấy được RAM của R4**, nên descriptor phải được sao chép vào LCM của MCU tại `0xEE02FE00` rồi mới nạp địa chỉ đó.

"MCU" ở đây gần như chắc chắn là **Cortex-M3 của khối Machine Control** — trong `power_device_init.h` có `pmu_MC_M3` bên cạnh `pmu_MC_SPI0/SPI1/QSPI`. _(Suy luận — không có tài liệu trực tiếp trong repo.)_ Nếu đúng thì SoC này có **ba lõi CPU**: cụm A72 (AP/Linux), R4 (LPPP), M3 (MC).

Phép `periph_id -= 18` dịch id sang không gian đánh số của MCU — khớp với việc trong bảng peripheral, id 18 (`CDMA_MVDO_MIRROR_0_FACET_TIMES`) là ranh giới giữa nhóm chung và nhóm "PI" (Power Island).

## 4.2 Bảo trì cache — bắt buộc và dễ quên

DMA đọc/ghi **thẳng vào bộ nhớ**, không đi qua D-cache của R4. Nên:

```c
/* TRƯỚC khi kick DMA — đẩy descriptor từ cache xuống RAM */
cpu_dcache_clean_region(&test_inputs[n], sizeof(CDMA2MEM_DESC2_t));

/* SAU khi DMA xong — bỏ bản sao cũ trong cache */
cpu_dcache_invalidate_region(sourceAddress,      cdma_structure->length);
cpu_dcache_invalidate_region(destinationAddress, cdma_structure->length);
```

Thiếu `clean` → engine đọc descriptor rác. Thiếu `invalidate` → CPU đọc lại dữ liệu cũ và tưởng DMA hỏng.

Đây cũng chính là lý do vùng 80 KB đầu LCM được MPU đặt **NOCACHE** (phần trước) — với buffer mạng bị DMA liên tục, tắt cache hẳn đơn giản hơn là bảo trì thủ công từng lần.

## 4.3 Xử lý riêng cho từng loại ngoại vi trong `_cdma_execute()`

```c
if (g_cdma_queue.cdma_type[i] == 2) {
    *((uint32_t *)(((uintptr_t)(test_inputs[i].destination)) - 0x50)) = 0x1;  // Enable SPI slave channel
    setDescriptorAddr(cdmaRegPtr, i);
}
else if (g_cdma_queue.cdma_type[i] == 4) {
    for (j = 0; j < 4; j++) {          // Clear out SPI RX buffer with dummy reads
        while ((*((uint32_t *)(((uintptr_t)(test_inputs[i].source)) - 0x38)) & 0x8) == 0) { }
        temp = *((uint32_t *)((uintptr_t)(test_inputs[i].source)));
    }
    setDescriptorAddr(cdmaRegPtr, i);
}
```

Truy cập thanh ghi bằng **offset âm từ địa chỉ FIFO** (`-0x50`, `-0x38`) — không dùng struct, không có tên. Rất khó bảo trì, dấu hiệu điển hình của code viết vội cho bench test.

Tương tự trong `cdma_finish()`, phần abort I2C:

```c
while (((*((uint32_t *)((uintptr_t)(test_inputs[i].destination) + 0x60)) & 0xE) != 0x6)) { }
*((uint32_t *)((uintptr_t)(test_inputs[i].destination) + 0x5C)) |= 0x2;   // Set Abort bit
```

Đối chiếu bản đồ thanh ghi DesignWare I2C ở lượt trước: FIFO là `IC_DATA_CMD` (`+0x10`), nên `+0x60` = `IC_STATUS` (`0x70`) và `+0x5C` = `IC_ENABLE` (`0x6C`), bit 1 = `ABORT`. Đúng — nhưng viết theo cách không ai đọc ra được nếu không tra ngược.

---

# Phần 5 · Trạng thái trong project: **code chết, không biên dịch**

Bảy dấu hiệu độc lập, tất cả đều xác nhận từ source:

## ① Không nằm trong danh sách build

[Hal/quartz/Makefile:32](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/Makefile#L32):

```make
TL_ARFLAGS = $(ASIC) cpu src interrupt MHU register_common timer clocks i2c debug strfmt
```

**Không có `cdma`.** Trong [Makefile-rules:534](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Makefile-rules#L534), `$(ARFLAGSFILE)` chỉ phụ thuộc `$(TL_ARFLAGS)`, và các sub-module là phony target chỉ được build khi có thứ phụ thuộc vào chúng. → **make không bao giờ recurse vào `cdma/`**.

## ② `MODULE_cdma` không được định nghĩa ở đâu

Nên khối duy nhất nối CDMA với code thật — hàm `cdma_i2c()` trong [i2c.c:313-350](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/i2c/src/i2c.c#L313-L350) — bị tiền xử lý loại bỏ hoàn toàn.

## ③ Kiểu `cdma_config_t` không tồn tại

Đây là kiểu tham số của **cả ba API chính** (`cdma_m2m`, `cdma_p2m_setup`, `cdma_m2p_setup`). Grep toàn repo trong mọi `.h`: **không có định nghĩa**. `cdma64_config.h` chỉ có `cdma_instance_config_t`, `cdma_platform_config_t`, `cdma_periph_config_t` — không có cái đang dùng.

## ④ Hai biến cấu hình chỉ có `extern`, không có định nghĩa

```c
extern const cdma_platform_config_t        g_cdma_platform_config;
extern const cdma_platform_periph_config_t g_cdma_platform_periph_config;
```

Không file `.c` nào định nghĩa chúng → link sẽ báo undefined reference.

## ⑤ `#include "cpu.h"` — file không tồn tại trong đường include của Hal

Chỉ có `Netstack/lwipv6/portable/arch/cpu.h`, không nằm trong include path của Hal.

## ⑥ Các hàm `UTF_*` không tồn tại

`UTF_interruptAttach`, `UTF_interruptEnable`, `UTF_interruptDisable`, `UTF_interruptDetach`, `UTF_enable_cpu_interrupts` — không có ở bất kỳ đâu. `UTF` = **Unit Test Framework** của Marvell; code sản xuất dùng `intAttach`/`intEnable`/`intDisable`.

## ⑦ Macro log đến từ file không được include

`log_debug`, `log_error`, `log_status`, `GET_USEC_u32()`, `cpu_dcache_clean_region` đều định nghĩa trong [flex_shim.h:78-108](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/flex_shim.h#L78-L108) — nhưng `cdma.c` **không include** file đó.

> `flex_shim.h` tự mô tả: _"Macros for common source between DPI-C simulation and baremetal"_ — nó là lớp đệm để chạy code kiểm chứng RTL trên phần cứng thật.

## Kết luận

`cdma.c` **không thể biên dịch được** ở trạng thái hiện tại, và hệ thống build cũng **không cố biên dịch nó**. Đây là code kiểm thử mang từ dự án Marvell khác sang, để lại nguyên vẹn.

Dấu vết củng cố: tên biến `test_inputs[24]`, `g_CDMAMemFinishedTest`, các hàm `cdma_test()` / `cdma_test_detail()` / `check_cdma_data()` (so sánh từng long-word rồi in "Channel %d DMA Passed"), comment `//Added by Dirk`, `g_CDMAControlOverride` với chú thích _"this is for manual test manipulation to get special usages"_, và một khối `run_cdma()` bị `#if 0` mà bên trong vẫn gọi `setDescriptorAddr(channel)` — **sai số tham số** so với prototype hiện tại `setDescriptorAddr(ptr, channel)`.

Riêng `DW_I2C.h` (909 dòng) nằm trong folder CDMA là **bản sao thứ ba** của cùng bộ định nghĩa thanh ghi DesignWare I2C (hai bản kia ở `i2c/src/i2c_regmasks.h` và `ApplicationKM/LPPP_I2C.c`), chỉ khác tiền tố macro (`DW_I2C_*` thay vì `APB_DW_I2C0_*`).

---

# Phần 6 · Vậy DMA thật trong LPPP là gì

|Đường dữ liệu|Cơ chế thật|
|---|---|
|Ethernet (GMAC)|**DMA riêng của GMAC**, buffer ở `.netbufsec`/`.bufpools` trong vùng NOCACHE của LCM|
|QSPI flash|**Polling** — `wait_cmd()`, `check_busy()`|
|I2C|**Polling** — `NVM_PollReg`, vòng `KM_i2c_read_clear_intrbits`|
|MHU/SCPI|`memcpy` thường + bảo trì cache thủ công|
|CRC32 vùng DDR qua sleep|**XOR engine v2 của AP806** — `mv_xor_4dma_crc32_list()` trong [mv_lpddr4_sleep.c:662](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/mv_lpddr4_sleep.c#L662)|

Trường hợp cuối là DMA "đa dụng" duy nhất thực sự được dùng, và nó **không phải CDMA** mà là XOR engine của AP806 — nằm trong `Hal/quartz/asic/build/` nên **có** được build (vì `$(ASIC)` = `asic` nằm trong `TL_ARFLAGS`). Comment tại chỗ gọi rất đáng chú ý: _"using XOR to bypass MPU"_ — dùng DMA engine để đọc vùng nhớ mà MPU đã chặn CPU truy cập.

---

# Tóm lại

> **CDMA = Central DMA**, bộ điều khiển DMA đa kênh trên AXI tại `0xE8271000`, mỗi channel một khối `0x100` byte. Nó chạy theo **descriptor 4 trường 64-bit trong RAM** (`length/source/destination/next`) nối thành chuỗi scatter-gather, hỗ trợ ba chế độ mem→mem, mem→periph, periph→mem phân biệt bằng `CFG.FLOWCTRL`, và móc vào **35 peripheral request line** (UART, SPI, I2C, audio, ADC, động cơ bước…). Điểm kiến trúc thú vị nhất là thủ thuật **cross-boundary**: channel > 15 thuộc DMA của lõi MCU khác, nên descriptor phải được nhân bản vào LCM của lõi đó tại `0xEE02FE00`.
> 
> Nhưng trong project này, **toàn bộ folder là code chết**: không nằm trong `TL_ARFLAGS`, `MODULE_cdma` không định nghĩa, kiểu `cdma_config_t` và hai biến cấu hình toàn cục không tồn tại, và file phụ thuộc vào một bộ hàm `UTF_*` của framework kiểm thử không có trong repo. Nó là code bench test mang từ dự án Marvell khác sang và bỏ quên — hữu ích để hiểu phần cứng, vô dụng để hiểu firmware đang chạy.