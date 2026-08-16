Tôi đã đọc toàn bộ hai tầng I2C. Trước khi vào chi tiết, có một điều phải nói ngay vì nó quyết định cách đọc tất cả phần còn lại.

---

# Phần 0 · Trong LPPP có **hai** driver I2C song song

|              | `Hal/quartz/i2c/src/i2c.c`                  | `ApplicationKM/LPPP_I2C.c`                                      |
| ------------ | ------------------------------------------- | --------------------------------------------------------------- |
| Tác giả      | Marvell (SDK gốc)                           | Konica Minolta (port từ Linux `i2c-designware`)                 |
| Kiểu         | Bare-metal, thao tác thanh ghi trực tiếp    | Cấu trúc driver Linux: `struct i2c_msg`, `xfer`, `abort_source` |
| Truy cập reg | `APB_DW_I2C0_REGS_t*` (struct có tên field) | `dw_read_word(base + offset)` (offset thô)                      |
| Bus dùng     | **I2C1** — MCP4542 (điện áp IO)             | **I2C0** — PS-CPU                                               |
| Ai gọi       | `hal_main.c::io_update_voltage()`           | `LPPP_Task.c`, `LPPP_Matrix.c`, `power.c`, `board.c`            |

Cả hai điều khiển **cùng một loại IP: Synopsys DesignWare APB I2C**, chỉ khác cách bọc. Điều này giải thích vì sao có hai bộ macro trùng lặp (`APB_DW_I2C0_IC_CON_*` vs `DW_IC_CON_*`) mô tả cùng những bit giống hệt nhau.

---

# Phần 1 · Cơ chế giao tiếp

## 1.1 Bản chất: FIFO + máy trạng thái phần cứng, **không** bit-bang

Đây là điểm dễ hiểu nhầm nhất. Phần mềm **không** tự nhấc SCL/SDA. Nó chỉ:

```
    ┌─────────────── Phần mềm (R4) ───────────────┐
    │  IC_TAR   ← địa chỉ slave (7 bit)           │
    │  IC_DATA_CMD ← lệnh + dữ liệu (đẩy TX FIFO) │
    │  IC_DATA_CMD → dữ liệu    (lấy RX FIFO)     │
    └───────────────────┬─────────────────────────┘
                        ▼
    ┌──────── DesignWare I2C hardware ────────────┐
    │  TX FIFO ──► FSM ──► shift register         │
    │                │                             │
    │                ├─ tự phát START khi TX FIFO  │
    │                │  có phần tử & bus rảnh      │
    │                ├─ tự chèn địa chỉ từ IC_TAR  │
    │                ├─ tự tạo SCL theo HCNT/LCNT  │
    │                ├─ tự kiểm ACK, set TX_ABRT   │
    │                └─ tự phát STOP khi bit9 = 1  │
    │  RX FIFO ◄── shift register                  │
    └──────────────────────────────────────────────┘
                        ▼  SDA / SCL (open-drain)
```

Ba hệ quả quan trọng:

**① Địa chỉ slave KHÔNG nằm trong luồng dữ liệu.** Nó ở `IC_TAR`. Phần cứng tự chèn byte địa chỉ + bit R/W vào đầu mỗi transaction. Đây là lý do `IC_TAR` chỉ ghi được khi `IC_ENABLE = 0` — thay đổi giữa chừng sẽ phá transaction.

**② `IC_DATA_CMD` là một cửa sổ hai chiều.** _Ghi_ = đẩy vào TX FIFO; _đọc_ = lấy từ RX FIFO. Cùng một địa chỉ `+0x10`, hai FIFO khác nhau.

**③ Đọc dữ liệu cần "đẩy lệnh đọc" trước.** Muốn nhận N byte, phải ghi N lần `IC_DATA_CMD` với bit CMD=1 — mỗi lần là một "phiếu yêu cầu một byte". Phần cứng mới sinh clock để nhận.

## 1.2 Định dạng `IC_DATA_CMD` (offset `+0x10`)

```
 31        11    10      9       8      7                 0
┌────────────┬────────┬───────┬───────┬───────────────────┐
│  reserved  │RESTART │ STOP  │  CMD  │      DAT[7:0]     │
└────────────┴────────┴───────┴───────┴───────────────────┘
              BIT(10)  BIT(9)  0x100
```

|Bit|Ý nghĩa|
|---|---|
|`CMD` (8)|`0` = ghi byte trong DAT ra bus · `1` = yêu cầu đọc 1 byte|
|`STOP` (9)|Phát STOP sau byte này|
|`RESTART` (10)|Phát repeated START **trước** byte này|
|`DAT` (7:0)|Byte dữ liệu (chỉ có nghĩa khi CMD=0)|

## 1.3 Sinh SCL — HCNT/LCNT, tính bằng số chu kỳ `ic_clk`

Không có prescaler. Phần mềm nạp thẳng số chu kỳ vào `IC_SS_SCL_HCNT`/`IC_SS_SCL_LCNT`.

Cấu hình trong LPPP ([LPPP_I2C.c:763-767](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_I2C.c#L763-L767)):

```c
#define I2C_CLK_FREQ      100000   /* SCL mục tiêu = 100 kHz (standard mode) */
#define IC_CLK_KHZ        200000   /* ic_clk = 200 MHz  → T = 5 ns */
#define SDA_HOLD_TIME     200      /* ns */
#define SDA_FALLING_TIME  200      /* ns */
#define SCL_FALLING_TIME  200      /* ns */
```

Công thức ([LPPP_I2C.c:193-210](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_I2C.c#L193-L210)):

$$\text{HCNT} = \frac{ic_clk_{[kHz]} \times (t_{HIGH} + t_f)_{[ns]} + 500000}{10^6} - 3$$ $$\text{LCNT} = \frac{ic_clk_{[kHz]} \times (t_{LOW} + t_f)_{[ns]} + 500000}{10^6} - 1$$

Đơn vị tự triệt tiêu: `kHz × ns / 1e6` = số chu kỳ. Số `500000` là làm tròn nửa lên.

**Thay số thật:**

```
HCNT = (200000 × (4000 + 200) + 500000) / 1e6 - 3
     = (840,000,000 + 500,000) / 1,000,000 - 3  =  840 - 3  =  837
LCNT = (200000 × (4700 + 200) + 500000) / 1e6 - 1
     = (980,000,000 + 500,000) / 1,000,000 - 1  =  980 - 1  =  979
```

Kiểm chứng ngược với T = 5 ns:

- tHIGH ≈ (837 + 3) × 5 ns = **4,20 µs** ✓ (spec I2C standard ≥ 4,0 µs)
- tLOW ≈ (979 + 1) × 5 ns = **4,90 µs** ✓ (spec ≥ 4,7 µs)
- Chu kỳ SCL ≈ 9,10 µs → **f_SCL ≈ 110 kHz**

Hơi cao hơn 100 kHz danh định vì thời gian sườn thực tế trên bo sẽ kéo dài thêm. Vẫn nằm trong dung sai standard-mode.

`SDA_HOLD`: `(200000 × 200 + 500000) / 1e6 = 40` chu kỳ = **200 ns**.

---

# Phần 2 · I2C dùng ở đâu trong LPPP — flow high-level

Đây là phần cốt lõi. **Bốn bus được khởi tạo, nhưng chỉ hai bus có thiết bị.**

```
┌──────────────── Cortex-R4 (LPPP) ────────────────┐
│                                                   │
│  I2C0 @ 0xE8006000 ─────► PS-CPU  (slave 0x3E)   │  ★ bus quan trọng nhất
│         driver: LPPP_I2C.c                        │
│                                                   │
│  I2C1 @ 0xE8006800 ─────► MCP4542 (slave 0x2D)   │  điện áp IO
│         driver: Hal/quartz/i2c/src/i2c.c          │
│                                                   │
│  I2C2 @ 0xE8007000 ─────► (khởi tạo, không dùng) │
│  I2C3 @ 0xE8007800 ─────► (khởi tạo, không dùng) │
└───────────────────────────────────────────────────┘
```

## 2.1 PS-CPU là gì và vì sao I2C0 quan trọng

**PS-CPU** (Power Supply CPU) là một **vi điều khiển phụ trên bo nguồn**, sống ngay cả khi LPPP đã ngủ. Nó nắm phần cứng mà LPPP không với tới: relay/công tắc nguồn động cơ, tín hiệu `SLEEP_STATUS_REM`, và một bộ ghi log sự kiện.

I2C0 là **kênh liên lạc duy nhất** giữa LPPP và PS-CPU. Ba loại giao dịch đi qua đây:

## 2.2 Bản đồ thanh ghi PS-CPU (slave `0x3E`)

Tổng hợp từ mọi lời gọi trong repo:

|2nd addr|Chiều|Ý nghĩa|Hàm|
|---|---|---|---|
|`0x68`|W|**Thông báo trạng thái nguồn**: `0`=Normal · `1`=Sleep2/ErP · `2`=Boot · `\|0x04` nếu là Eagle3|`send_power_state_to_pscpu()`|
|`0x6C`|W|**Yêu cầu SLEEP_STATUS_REM**: `1`=REM 0 · `2`=REM 1|`send_sleep_status_rem()`|
|`0x70`|R|Đọc lại trạng thái GPIO (debug)|`i2c_MC_PWR_EN()`|
|`0x74`|W|**Set-bit register**: `0x1000` → `MC_PWR_EN` = High|`i2c_MC_PWR_EN(true)`|
|`0x78`|W|**Clear-bit register**: `0x1000` → `MC_PWR_EN` = Low|`i2c_MC_PWR_EN(false)`|
|`0xC0`|W|**Ghi bản ghi sự kiện** (cho công cụ đo tự động)|nhiều nơi|

Mã sự kiện ghi vào `0xC0`:

|Mã|Ý nghĩa|Ghi từ đâu|
|---|---|---|
|`3`|`ATFStart` — ATF bắt đầu boot|[board.c:519](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/board.c#L519)|
|`56`|`PressPanelPowerKeyDeepSleep` — người dùng bấm nút nguồn khi đang DeepSleep|[LPPP_Matrix.c:367](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Matrix.c#L367)|
|`88`|`ResumeRequest` — LPPP gửi yêu cầu đánh thức tới PS-CPU|[power.c:329](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/power.c#L329)|

Cả ba đều bọc trong `if (isAutomaticMeasurementToolOn())` — cờ đọc từ SoftSW `0xA4` bit 8 trong flash. Nghĩa là **ghi log chỉ bật khi kỹ sư đang đo thời gian boot**, không tốn thời gian trong máy sản xuất.

## 2.3 Flow 1 — Bật nguồn AP806 (đường resume)

`ap806_power_on(true)` ([power.c:316-385](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/power.c#L316-L385)):

```
① i2c_enable(true)
     gpio_set_func_sel(pad 0..3, function=1)     ← chuyển pad về chức năng I2C
     comment: "I2Cの初期化処理を早く実行する"

② [chỉ Eagle3/Sparrow3, và không phải lần boot đầu]
     if (isAutomaticMeasurementToolOn())
        LPPP_i2c_write(0, 0x3E, 0xC0, 88)        ← log "ResumeRequest"
     send_sleep_status_rem(1)
        LPPP_i2c_write(0, 0x3E, 0x6C, 0x0002)    ← "hãy đặt SLEEP_STATUS_REM = 1"
     cpu_spin_delay(100 ms)                       ← chờ PS-CPU phản ứng
                                                     rồi mới AP_PWR_EN = 1

③ bật lần lượt các GPIO nguồn (gpio_nums[])

④ send_power_state_to_pscpu(true)
     lần đầu tiên  → LPPP_i2c_write(0, 0x3E, 0x68, 0x02 | gen)   "Boot"
     các lần sau   → LPPP_i2c_write(0, 0x3E, 0x68, 0x00 | gen)   "Normal"
     (gen = 0x04 nếu supportTEC() == RESULT_EAGLE3)

⑤ ap806_power_good(500 ms)  ← chờ chân AP_PG lên
```

Điểm tinh tế ở ②: **thứ tự bắt buộc**. `SLEEP_STATUS_REM=1` phải tới PS-CPU **trước** khi `AP_PWR_EN` lên, cách nhau 100 ms — nếu không PS-CPU sẽ hiểu nhầm là mất điện đột ngột.

## 2.4 Flow 2 — Tắt nguồn AP806

```
① send_power_state_to_pscpu(false)
     LPPP_i2c_write(0, 0x3E, 0x68, 0x01 | gen)   ← "Sleep2/ErP"

② tắt lần lượt các GPIO nguồn

③ [chỉ Eagle3] cpu_spin_delay(100 ms)            ← sau AP_PWR_EN=0 chờ 100 ms
     send_sleep_status_rem(0)
        LPPP_i2c_write(0, 0x3E, 0x6C, 0x0001)    ← "SLEEP_STATUS_REM = 0"

④ i2c_enable(false)
     gpio_set_func_sel(pad 0..3, function=0)     ← trả pad về GPIO thuần
```

④ là chi tiết tiết kiệm điện: khi ngủ sâu, các pad I2C được đưa về GPIO để tránh rò dòng qua điện trở pull-up. Cùng kỹ thuật với `LPPP_TR3_NormalToSleep1` làm với pad I2S (33/34).

## 2.5 Flow 3 — State machine gọi I2C

Đây là điểm nối vào máy trạng thái mà bạn đã phân tích ở các lượt trước. `LPPP_TR8_TR10_Sleep2_Or_ErPToSleep1` ([LPPP_Matrix.c:364-373](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_Matrix.c#L364-L373)):

```c
if (isAutomaticMeasurementToolOn() == true) {
    retval = LPPP_i2c_write(0, PSCPU_SLAVE_ADDR /*0x3E*/,
                               PSCPU_REG_EVENT  /*0xC0*/,
                               THRD_EventRecord_PressPanelPowerKeyDeepSleep /*56*/);
    if (retval < 0) ApiSysDebug_Printf(... "LPPP_i2c_write ERROR ret=[%d]" ...);
}
```

**Một phụ thuộc thứ tự tinh tế:** transition này chạy trong task `PwEvtMgr` (prio 0), sau khi nhận `WAKEUP_S1`. Còn `i2c_enable(true)` chạy trong `ap806_power_on(true)` bên trong `pmu_phase(pmu_resume)` — mà `power_cpu()` chỉ gửi `WAKEUP_S1` vào queue **sau khi** `pmu_resume` trả về thành công. Nên pad I2C chắc chắn đã được bật trước khi `TR8_TR10` chạy. _(Suy luận từ thứ tự lời gọi; đúng nhưng không có cơ chế đồng bộ tường minh nào bảo đảm.)_

## 2.6 Flow 4 — I2C1: điều chỉnh điện áp IO (driver Marvell)

Chạy một lần trong `init_thread`, ngay sau `pmu_system_up`:

```
hal_main.c:187  io_update_voltage()
    └─ io_millivolts_set(1100)                       hal_main.c:773
         ├─ nếu IC_ENABLE_STATUS bit0 == 0:
         │     MCP4542_i2c_configure()               hal_main.c:759
         │       ├─ i2c_init_new(AON_I2C1_BASE)
         │       ├─ i2c_IC_TAR(0x2D)                 ← MCP4542_ADDRESS
         │       ├─ IC_INTR_MASK = 0xFFFF
         │       ├─ IC_ENABLE = 0
         │       ├─ IC_CON = 0x63                    ← hard-code, xem §4.1
         │       └─ IC_ENABLE = 1
         └─ ghi wiper_code vào chiết áp số → đổi điện áp IO
```

MCP4542 là **chiết áp số** — LPPP dùng nó để tinh chỉnh điện áp I/O của chip theo kết quả đo AVS (Adaptive Voltage Scaling).

## 2.7 Tổng kết: I2C dùng để làm gì

```
                 ┌─────────────────────────────────────────┐
                 │           LPPP (Cortex-R4)              │
                 └──┬───────────────────────────────────┬──┘
                    │ I2C0 (0x3E)                       │ I2C1 (0x2D)
                    ▼                                   ▼
        ┌───────────────────────┐            ┌────────────────────┐
        │       PS-CPU          │            │      MCP4542       │
        │  (vi điều khiển nguồn)│            │  (chiết áp số)     │
        ├───────────────────────┤            ├────────────────────┤
        │ • nhận thông báo trạng │            │ • đặt điện áp IO   │
        │   thái nguồn (0x68)    │            │   1000–1200 mV     │
        │ • điều khiển           │            │ • chạy 1 lần lúc   │
        │   SLEEP_STATUS_REM     │            │   boot             │
        │   (0x6C)               │            └────────────────────┘
        │ • bật/tắt MC_PWR_EN    │
        │   (0x74/0x78)          │
        │ • ghi log sự kiện boot │
        │   (0xC0)               │
        └───────────────────────┘
```

Nói ngắn: **I2C0 là dây thần kinh giữa LPPP và mạch nguồn**; nó không truyền dữ liệu lớn, chỉ truyền các thông báo trạng thái vài byte tại những thời điểm chuyển nguồn quan trọng.

---

# Phần 3 · Phân tích source `Hal/quartz/i2c`

## 3.1 Cấu trúc thư mục

```
Hal/quartz/i2c/
├── INA219.h            ← header cho cảm biến dòng INA219 (I2C port 6) — KHÔNG dùng trên Quartz
├── Makefile
└── src/
    ├── i2c.c           ← driver, 1317 dòng
    ├── i2c_config.h    ← khai báo g_i2c_platform_config (KHÔNG có định nghĩa trong repo)
    ├── i2c_regstructs.h ← struct APB_DW_I2C0_REGS_t (map thanh ghi)
    └── i2c_regmasks.h  ← 883 dòng macro mask/shift
```

⚠️ **`g_i2c_platform_config` được `extern` trong `i2c_config.h` nhưng không có định nghĩa nào trong repo.** Chỉ `i2c_test_reg_access()` dùng nó — hàm này không ai gọi. Nếu linker giải quyết được thì phải có ở nơi khác; nếu không thì hàm đó không được link vào. _(Chưa đủ thông tin.)_

## 3.2 `clearAllI2CInts()` — cơ chế clear-on-read

```c
void clearAllI2CInts(void* i2cStruct)
{
    APB_DW_I2C0_REGS_t* i2cRegs = (APB_DW_I2C0_REGS_t*) i2cStruct;
    volatile int readWord;

    readWord = i2cRegs->IC_CLR_INTR;        // +0x40  xoá TẤT CẢ
    readWord = i2cRegs->IC_CLR_RX_UNDER;    // +0x44
    readWord = i2cRegs->IC_CLR_RX_OVER;     // +0x48
    readWord = i2cRegs->IC_CLR_TX_OVER;     // +0x4C
    readWord = i2cRegs->IC_CLR_RD_REQ;      // +0x50
    readWord = i2cRegs->IC_CLR_TX_ABRT;     // +0x54
    readWord = i2cRegs->IC_CLR_RX_DONE;     // +0x58
    readWord = i2cRegs->IC_CLR_ACTIVITY;    // +0x5C
    readWord = i2cRegs->IC_CLR_STOP_DET;    // +0x60
    readWord = i2cRegs->IC_CLR_START_DET;   // +0x64
    readWord = i2cRegs->IC_CLR_GEN_CALL;    // +0x68
    readWord = readWord;                     // To make compiler happy
}
```

Đây là nhóm thanh ghi **clear-on-read**: _đọc_ chúng mới là hành động xoá cờ, giá trị trả về vô nghĩa. `volatile int readWord` bắt buộc — không có nó compiler sẽ xoá sạch các phép đọc "không dùng". Dòng `readWord = readWord` chỉ để dập cảnh báo _set but not used_.

Lưu ý: đọc `IC_CLR_INTR` (+0x40) đã xoá **tất cả** rồi; 10 dòng sau là thừa. Nhưng vô hại.

## 3.3 `I2CEnable()` — bật/tắt và **xác nhận thật sự đã đổi**

```c
if (enable == true) {
    i2cRegs->IC_ENABLE |= APB_DW_I2C0_IC_ENABLE_ENABLE_MASK;   // ① set bit0
    cpu_spin_delay(1);                                          // ② chờ 1 µs
    if ((i2cRegs->IC_ENABLE_STATUS & 0x1) != 0x1)               // ③ kiểm tra nhanh
        ApiSysDebug_Printf(-1,-1,"$s FAILED TO ENABLE!\n", __func__);
    status = NVM_PollReg(&i2cRegs->IC_ENABLE_STATUS, 1, 0x1,    // ④ poll 10 × 100 µs
                         ENABLE_DELAY, ENABLE_DELAY_COUNT, EQUALS);
} else {
    i2cRegs->IC_ENABLE &= ~APB_DW_I2C0_IC_ENABLE_ENABLE_MASK;  // clear bit0
    ...
    status = NVM_PollReg(&i2cRegs->IC_ENABLE_STATUS, 0, 0x1, ...);
}
```

**Vì sao phải có `IC_ENABLE_STATUS` riêng?** Vì `IC_ENABLE` là **ghi-vào-hàng-đợi**: ghi 0 khi đang truyền thì controller hoàn tất byte hiện tại rồi mới thật sự tắt. `IC_ENABLE_STATUS.IC_EN` mới phản ánh trạng thái **thực** của FSM. Đọc lại `IC_ENABLE` sẽ cho giá trị vừa ghi, không phải trạng thái thật — đây là cái bẫy kinh điển của IP này.

⚠️ Lỗi nhỏ: `"$s FAILED TO ENABLE!\n"` — dùng `$s` thay vì `%s`, nên `__FUNCTION__` không bao giờ được in.

## 3.4 `NVM_PollReg()` — poll dùng chung

```c
static int NVM_PollReg(volatile uint32_t *pollReg, uint32_t result, uint32_t mask,
                       uint32_t poll_interval, uint32_t count, uint32_t equals)
{
   for (i = 0; i < count; i++) {
      regVal = *pollReg;
      if (equals == true) { if ((regVal & mask) == result) { found = true; break; } }
      else                { if ((regVal & mask) != result) { found = true; break; } }
      if (poll_interval != 0) cpu_spin_delay(poll_interval);
   }
   return found ? OK : FAIL;
}
```

Tên `NVM_` là di sản từ module NVM. Tham số `equals` cho phép dùng cho cả hai chiều điều kiện. `poll_interval` tính bằng µs.

## 3.5 `i2c_init()` — cấu hình master

```c
I2CEnable(i2cRegs, false);       // ① BẮT BUỘC tắt trước khi ghi IC_CON
clearAllI2CInts(i2cRegs);
cpu_spin_delay(10);

temp = i2cRegs->IC_CON;                                        // ② read-modify-write
temp = ..._IC_SLAVE_DISABLE_REPLACE_VAL(temp, 1);              //    bit6 ← 1
temp = ..._IC_RESTART_EN_REPLACE_VAL(temp, 1);                 //    bit5 ← 1
temp = ..._IC_10BITADDR_MASTER_REPLACE_VAL(temp, 0);           //    bit4 ← 0
temp = ..._IC_10BITADDR_SLAVE_REPLACE_VAL(temp, 0);            //    bit3 ← 0
temp = ..._SPEED_REPLACE_VAL(temp, CON_SPEED_STANDARD /*1*/);  //    bits2:1 ← 01
temp = ..._MASTER_MODE_REPLACE_VAL(temp, 1);                   //    bit0 ← 1
i2cRegs->IC_CON = temp;                                        //    = 0x63

i2cRegs->IC_SDA_SETUP = ..._SDA_SETUP_REPLACE_VAL(..., 300);   // ③ ⚠ xem dưới
i2cRegs->IC_RX_TL = 0;                                          // ④ ngưỡng FIFO
i2cRegs->IC_TX_TL = 0;
I2CEnable(i2cRegs, true);
```

**Macro `REPLACE_VAL` truy ngược** ([i2c_regmasks.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/i2c/src/i2c_regmasks.h)):

```c
#define APB_DW_I2C0_IC_CON_SPEED_REPLACE_VAL(reg,val) \
        (((reg) & ~0x6) | (((uint32_t)val) << 1))
```

Tức: xoá bits 2:1 rồi chèn `val << 1`. Đây là mẫu **clear-then-set** an toàn, khác với `|=` (không xoá được bit cũ).

⚠️ **③ là một lỗi thật.** `APB_DW_I2C0_IC_SDA_SETUP_SDA_SETUP_MASK = 0xff` — trường chỉ 8 bit. Nhưng code ghi `I2C_STD_SDA_SETUP_TIME = 300`. `300 & 0xFF = 44`. Ý định là "300 ns", nhưng thanh ghi tính bằng **chu kỳ ic_clk**, không phải ns. Comment ngay trên thừa nhận sự lẫn lộn: _"SDA setup time is specified in nsec, and bus speed in MHz, so divide by 1000"_ — nhưng phép chia đó không được thực hiện. Đối chiếu: driver KM tính đúng (`sda_hold_time = (clk_khz × 200 + 500000)/1e6 = 40`).

⚠️ Hai dòng `IC_SS_SCL_HCNT`/`LCNT` bị comment với `// REVISIT DAB ... xxx` → **HCNT/LCNT giữ nguyên giá trị reset**. Tốc độ SCL thật của bus I2C1 do đó là **giá trị mặc định của IP**, không phải 100 kHz đã tính toán. _(Đã xác nhận từ source — driver này chỉ dùng cho MCP4542, tốc độ không tới hạn.)_

## 3.6 `i2c_check_ack()` — dò xem có thiết bị ở địa chỉ đó không

```c
clearAllI2CInts(i2cRegs);
cpu_spin_delay(10);
while (i2cRegs->IC_TXFLR > 6);                       // ① chờ TX FIFO vơi
i2cRegs->IC_DATA_CMD = DATA_CMD_READ | DATA_CMD_STOP; // ② 0x100 | 0x200 = 0x300

if (NVM_PollReg(&i2cRegs->IC_STATUS, 0x0,
                APB_DW_I2C0_IC_STATUS_MST_ACTIVITY_MASK /*0x20*/, ...) == FAIL)
    return FAIL;                                      // ③ chờ FSM về idle

if (((IC_TX_ABRT_SOURCE & (ABRT_7B_ADDR_NOACK | ABRT_10ADDR1_NOACK
                          | ABRT_10ADDR2_NOACK)) == 0)      // ④ không bị NACK địa chỉ
 && ((IC_RAW_INTR_STAT & ACTIVITY_MASK) != 0))              //    và có hoạt động trên bus
    return OK;      // → tìm thấy thiết bị
return FAIL;
```

Đây là thuật toán quét bus I2C chuẩn: phát một transaction đọc 1 byte rồi kiểm tra xem địa chỉ có được ACK không. `IC_TX_ABRT_SOURCE` bit 0 (`ABRT_7B_ADDR_NOACK`) chính là _"không ai trả lời"_.

`i2c_search_all()` dùng nó quét `0x08 → 0xA7`, mỗi địa chỉ phải `disable → set IC_TAR → enable` vì `IC_TAR` chỉ ghi được khi disabled.

## 3.7 🔴 Bug trong `DATA_CMD_DATA()`

```c
#define DATA_CMD_DATA(val)  (((uint32_t)val) << APB_DW_I2C0_IC_DATA_CMD_CMD_SHIFT)
                                                                    ^^^ = 8
```

Dùng nhầm `CMD_SHIFT` (=8) thay vì `DAT_SHIFT` (=0). Nên `DATA_CMD_DATA(0x55)` cho `0x5500` thay vì `0x55` — dữ liệu bị đẩy ra khỏi trường `DAT[7:0]`.

Tệ hơn, cách dùng:

```c
i2cRegs->IC_DATA_CMD = DATA_CMD_WRITE & DATA_CMD_DATA(0x00);
//                   = 0xFFFFFEFF & (0x00 << 8)  =  0
```

`DATA_CMD_WRITE` được định nghĩa là `~CMD_MASK` = `0xFFFFFEFF`, rồi **AND** với dữ liệu — kết quả là xoá bit 8 của một giá trị đã bị shift sai.

File `INA219.h` cùng thư mục định nghĩa **đúng**:

```c
#define DATA_CMD_DATA(val)  (((uint32_t)val) << DW_I2C_IC_DATA_CMD_DAT_SHIFT)   // << 0
```

Lỗi này chỉ ảnh hưởng `cdma_i2c()` (`#ifdef MODULE_cdma`), `i2c_read_new()` và `send_i2c_addr()` — đều là code test không được gọi ở production. Đường MCP4542 ghi thẳng giá trị nên không dính.

---

# Phần 4 · Register mapping

## 4.1 Bản đồ đầy đủ — DesignWare APB I2C

Base: **I2C0 = `0xE8006000`** · I2C1 = `0xE8006800` · I2C2 = `0xE8007000` · I2C3 = `0xE8007800` (bước nhảy `AON_I2C_BLK_ADDR = 0x800`)

|Offset|Addr (I2C0)|Tên|R/W|Chức năng|
|---|---|---|---|---|
|`+0x00`|`0xE8006000`|`IC_CON`|R/W|Control — master/speed/restart|
|`+0x04`|`0xE8006004`|`IC_TAR`|R/W|**Target Address** (chỉ ghi khi disabled)|
|`+0x08`|`0xE8006008`|`IC_SAR`|R/W|Slave Address (không dùng)|
|`+0x10`|`0xE8006010`|`IC_DATA_CMD`|R/W|**Cửa sổ FIFO 2 chiều**|
|`+0x14`|`0xE8006014`|`IC_SS_SCL_HCNT`|R/W|Số chu kỳ SCL HIGH (standard)|
|`+0x18`|`0xE8006018`|`IC_SS_SCL_LCNT`|R/W|Số chu kỳ SCL LOW (standard)|
|`+0x1C/0x20`||`IC_FS_SCL_HCNT/LCNT`|R/W|Fast mode (không dùng)|
|`+0x2C`|`0xE800602C`|`IC_INTR_STAT`|R|Trạng thái ngắt **đã mask**|
|`+0x30`|`0xE8006030`|`IC_INTR_MASK`|R/W|Mặt nạ ngắt|
|`+0x34`|`0xE8006034`|`IC_RAW_INTR_STAT`|R|Trạng thái ngắt **thô**|
|`+0x38/0x3C`||`IC_RX_TL / IC_TX_TL`|R/W|Ngưỡng FIFO|
|`+0x40…+0x68`||`IC_CLR_*`|**R**|**Clear-on-read** (11 thanh ghi)|
|`+0x6C`|`0xE800606C`|`IC_ENABLE`|R/W|bit0 = Enable|
|`+0x70`|`0xE8006070`|`IC_STATUS`|R|Trạng thái FSM + FIFO|
|`+0x74`|`0xE8006074`|`IC_TXFLR`|R|Số phần tử trong TX FIFO|
|`+0x78`|`0xE8006078`|`IC_RXFLR`|R|Số phần tử trong RX FIFO|
|`+0x7C`|`0xE800607C`|`IC_SDA_HOLD`|R/W|Thời gian giữ SDA|
|`+0x80`|`0xE8006080`|`IC_TX_ABRT_SOURCE`|R|**Lý do abort** (xoá khi đọc `IC_CLR_TX_ABRT`)|
|`+0x94`|`0xE8006094`|`IC_SDA_SETUP`|R/W|Setup time, 8 bit|
|`+0x9C`|`0xE800609C`|`IC_ENABLE_STATUS`|R|**Trạng thái enable THẬT**|
|`+0xF4`|`0xE80060F4`|`IC_COMP_PARAM_1`|R|Độ sâu FIFO|
|`+0xF8`|`0xE80060F8`|`IC_COMP_VERSION`|R|Version IP|
|`+0xFC`|`0xE80060FC`|`IC_COMP_TYPE`|R|= `0x44570140` ("DW\x01\x40")|

## 4.2 `IC_CON` (`+0x00`) — chi tiết bit

```
 31        9   8      7        6       5       4       3      2   1    0
┌───────────┬─────┬────────┬────────┬───────┬───────┬───────┬───────┬──────┐
│ reserved  │ TX_ │STOP_DET│ SLAVE_ │RESTART│10BIT_ │10BIT_ │ SPEED │MASTER│
│           │EMPTY│IFADDR  │DISABLE │ _EN   │MASTER │SLAVE  │ [1:0] │_MODE │
└───────────┴─────┴────────┴────────┴───────┴───────┴───────┴───────┴──────┘
              0x100  0x80     0x40    0x20    0x10    0x8     0x6     0x1
```

**Giá trị thực tế được ghi = `0x63`** (cả hai driver, dù tính theo hai cách khác nhau):

|Bit|Tên|Giá trị|Vì sao|
|---|---|---|---|
|0|`MASTER_MODE`|**1**|LPPP là master|
|2:1|`SPEED`|**01**|Standard mode (100 kHz)|
|3|`10BITADDR_SLAVE`|0|không dùng slave mode|
|4|`10BITADDR_MASTER`|0|PS-CPU dùng địa chỉ 7 bit|
|5|`RESTART_EN`|**1**|cho phép repeated START|
|6|`SLAVE_DISABLE`|**1**|tắt hẳn slave logic|
|7|`STOP_DET_IFADDRESSED`|0|—|
|8|`TX_EMPTY_CTRL`|0|—|

`0x1 | 0x2 | 0x20 | 0x40 = 0x63` ✓

**Giá trị trước/sau:**

|Thời điểm|`IC_CON`|
|---|---|
|Sau reset|`0x7F` hoặc `0x67` (tuỳ tham số tổng hợp IP) — _chưa đủ thông tin_|
|Sau `KM_i2c_init`|**`0x63`**|

## 4.3 `IC_TAR` (`+0x04`)

```
 31      13   12      11       10        9                  0
┌──────────┬──────┬────────┬──────────┬────────────────────────┐
│ reserved │10BIT │SPECIAL │GC_OR_    │      IC_TAR[9:0]       │
│          │ADDR  │        │START     │                        │
└──────────┴──────┴────────┴──────────┴────────────────────────┘
            0x1000  0x800    0x400            0x3FF
```

Ghi cho PS-CPU: `IC_TAR = 0x3E` (bits 9:0), 10BITADDR = 0.

**Quan trọng:** với địa chỉ 7 bit, ghi giá trị **chưa dịch** (`0x3E`), **không phải** `0x7C`. Phần cứng tự chèn bit R/W. Đây là khác biệt với việc tự bit-bang.

Trong `i2c_IC_TAR()`, macro truy ngược:

```c
temp = ((temp) & ~0x1000) | (0 << 12);   // 10BITADDR ← 0
temp = ((temp) & ~0x3FF)  | (0x3E << 0); // IC_TAR    ← 0x3E
```

## 4.4 `IC_STATUS` (`+0x70`) — chỉ đọc

|Bit|Mask|Tên|Ý nghĩa|
|---|---|---|---|
|0|`0x01`|`ACTIVITY`|Bus có hoạt động|
|1|`0x02`|`TFNF`|TX FIFO Not Full|
|2|`0x04`|`TFE`|TX FIFO Empty|
|3|`0x08`|`RFNE`|**RX FIFO Not Empty** — dùng để biết có dữ liệu đọc|
|4|`0x10`|`RFF`|RX FIFO Full|
|5|`0x20`|`MST_ACTIVITY`|**Master FSM đang bận** — chờ về 0 = transaction xong|
|6|`0x40`|`SLV_ACTIVITY`|Slave FSM bận|

`NVM_PollReg(&IC_STATUS, 0x0, 0x20, ...)` = _"chờ tới khi `MST_ACTIVITY` = 0"_ — đây là cách duy nhất biết STOP đã phát xong.

## 4.5 `IC_TX_ABRT_SOURCE` (`+0x80`) — chẩn đoán lỗi

|Bit|Tên|Nghĩa|
|---|---|---|
|0|`ABRT_7B_ADDR_NOACK`|**Không có thiết bị ở địa chỉ đó**|
|1/2|`ABRT_10ADDR1/2_NOACK`|NACK địa chỉ 10 bit|
|3|`ABRT_TXDATA_NOACK`|Slave NACK dữ liệu|
|4/5|`ABRT_GCALL_*`|General call|
|11|`ABRT_MASTER_DIS`|Thao tác khi master mode tắt|
|12|`ARB_LOST`|**Mất quyền làm chủ bus** (multi-master)|

Bẫy: đọc `IC_CLR_TX_ABRT` **xoá luôn** `IC_TX_ABRT_SOURCE`. Driver KM xử lý đúng ([LPPP_I2C.c:710-717](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_I2C.c#L710-L717)):

```c
if (stat & DW_IC_INTR_TX_ABRT) {
    /* The IC_TX_ABRT_SOURCE register is cleared whenever
     * the IC_CLR_TX_ABRT is read. Preserve it beforehand. */
    dev->abort_source = KM_i2c_readl(dev, DW_IC_TX_ABRT_SOURCE);  // LƯU TRƯỚC
    KM_i2c_readl(dev, DW_IC_CLR_TX_ABRT);                          // rồi mới xoá
}
```

## 4.6 Trình tự ghi thanh ghi cho một `LPPP_i2c_write(0, 0x3E, 0x68, 0x00000001)`

|#|Register|Addr|Giá trị|Giải thích|
|---|---|---|---|---|
|1|`IC_ENABLE`|`0xE800606C`|`0`|Tắt để đổi `IC_TAR`|
|2|`IC_ENABLE_STATUS`|`0xE800609C`|poll → `0`|Xác nhận đã tắt thật|
|3|`IC_CON`|`0xE8006000`|`0x63`|Xoá bit 10BITADDR_MASTER|
|4|`IC_TAR`|`0xE8006004`|`0x3E`|Địa chỉ PS-CPU|
|5|`IC_INTR_MASK`|`0xE8006030`|`0`|Tắt ngắt|
|6|`IC_ENABLE`|`0xE800606C`|`1`|Bật lại|
|7|`IC_CLR_INTR`|`0xE8006040`|(đọc)|Xoá cờ tồn đọng|
|8|`IC_INTR_MASK`|`0xE8006030`|`0x254`|RX_FULL\|TX_EMPTY\|TX_ABRT\|STOP_DET|
|9|`IC_DATA_CMD`|`0xE8006010`|`0x068`|byte 0 = reg addr `0x68`, CMD=0|
|10|`IC_DATA_CMD`|`0xE8006010`|`0x001`|byte 1 = `0x01` (LSB của data)|
|11|`IC_DATA_CMD`|`0xE8006010`|`0x000`|byte 2|
|12|`IC_DATA_CMD`|`0xE8006010`|`0x000`|byte 3|
|13|`IC_DATA_CMD`|`0xE8006010`|`0x200`|byte 4 = `0x00` **+ STOP (bit9)**|
|14|`IC_ENABLE`|`0xE800606C`|`0`|Tắt adapter sau transaction|

`DW_IC_INTR_DEFAULT_MASK` = `0x004 | 0x010 | 0x040 | 0x200` = **`0x254`**.

Ghi chú: payload là **5 byte** (`writedata[0] = i2c_reg`, rồi `memcpy(&writedata[1], &data, 4)`) — LPPP luôn gửi 1 byte địa chỉ + 4 byte dữ liệu **little-endian**, kể cả khi PS-CPU chỉ cần 1 byte.

---

# Phần 5 · Những điểm cần dè chừng

## 🔴 5.1 `KM_vTaskDelay()` là **busy-wait**, không phải delay của RTOS

```c
static void KM_vTaskDelay(uint32_t count) {
    cpu_spin_delay(count*1000); /* ms */
}
```

Tên gợi ý `vTaskDelay` (nhường CPU) nhưng thực tế là **spin 1000 µs = 1 ms**. Và nó được gọi ở ba chỗ nóng:

- Vòng polling chính: mỗi vòng 1 ms × `XFER_PORLLING_TIMEOUT` (1000) = **timeout 1 giây**
- `__KM_i2c_enable()`: mỗi lần thử 1 ms × 100 lần
- **`KM_i2c_xfer_msg()` dòng 482 — giữa MỖI byte ghi**:
    
    ```c
    } else {
        KM_vTaskDelay(1); /* 1000roop */ /* 移植元 udelay(5); */
        KM_i2c_writel(dev, cmd | *buf++, DW_IC_DATA_CMD);
    }
    ```
    
    Comment thừa nhận: bản gốc Linux dùng `udelay(5)` — **5 µs**. Bản port dùng **1000 µs**, gấp **200 lần**.

Hệ quả: một `LPPP_i2c_write` 5 byte tiêu **tối thiểu ~5 ms CPU spin**, chưa tính vòng polling. Với `ap806_power_on()` gọi 2–3 lần I2C cộng thêm 2 lần `cpu_spin_delay(100 ms)`, tổng thời gian chuyển nguồn bị kéo dài đáng kể.

## 🟠 5.2 Driver dùng polling nhưng vẫn giữ nguyên cấu trúc ISR của Linux

`KM_i2c_xfer` không đăng ký ISR nào. Nó gọi `KM_i2c_read_clear_intrbits()` trong vòng lặp — tức **đọc `IC_INTR_STAT` như thể mình là ISR**. Toàn bộ cấu trúc `dev->status`, `msg_write_idx`, `rx_outstanding` được thiết kế cho mô hình ngắt (mỗi lần ISR bơm thêm một ít dữ liệu), nhưng ở đây chỉ chạy 1–2 vòng rồi `break`.

Comment gốc ở [dòng 397-401](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_I2C.c#L397-L401) còn nguyên: _"This function is only called from i2c_dw_isr"_ — nhưng `i2c_dw_isr` không tồn tại trong bản port.

## 🟠 5.3 Vòng polling `break` trước khi transaction hoàn tất

```c
if (stat & DW_IC_INTR_TX_EMPTY) {
    KM_i2c_xfer_msg(dev);
    if (!(dev->msgs->flags & I2C_M_RD)) {
        break;                    // ← thoát NGAY với thao tác ghi
    }
}
...
__KM_i2c_enable(dev, false);      // ← tắt adapter
```

`TX_EMPTY` chỉ có nghĩa _"TX FIFO đã xuống dưới ngưỡng"_, **không** phải _"đã phát STOP"_. Driver không chờ `STOP_DET` mà tắt adapter luôn. Theo databook, ghi `IC_ENABLE = 0` khi đang truyền thì controller hoàn tất byte hiện tại rồi mới tắt — nên trong thực tế vẫn chạy, nhưng đây là chỗ mong manh nhất của driver.

## 🟠 5.4 `LPPP_i2c_read` dùng STOP thay vì repeated START

```c
/* xfer 1: ghi 1 byte = địa chỉ thanh ghi  →  STOP */
msgs.flags = 0;  msgs.len = 1;  msgs.buf = &i2c_reg;
KM_i2c_xfer(&device[n], &msgs, 1);

/* xfer 2: đọc 2 byte  →  START mới */
msgs.flags = I2C_M_RD;  msgs.len = 2;
KM_i2c_xfer(&device[n], &msgs, 1);
```

Hai lời gọi `KM_i2c_xfer` riêng biệt → có **STOP ở giữa**, rồi START mới. Chuẩn I2C cho "register read" là _repeated START_ (không nhả bus). Nhiều slave sẽ reset con trỏ thanh ghi khi thấy STOP và trả sai dữ liệu.

Ở đây hoạt động được vì `RESTART_EN` đã bật và PS-CPU chấp nhận kiểu này, nhưng nếu ai đó thêm một slave khác vào bus, đây là chỗ sẽ hỏng trước tiên. Đồng thời, có STOP ở giữa nghĩa là **một master khác có thể chen vào** — không nguy hiểm vì LPPP là master duy nhất.

## 🟡 5.5 `XFER_PORLLING_TIMEOUT` — lỗi chính tả trong tên hằng số

`PORLLING` → `POLLING`. Chỉ là thẩm mỹ, nhưng làm khó khi grep.

## 🟡 5.6 Bốn bus đều được khởi tạo, hai bus không có thiết bị

`LPPP_i2c_init()` chạy `KM_i2c_probe()` cho cả I2C0–I2C3. `KM_i2c_probe` → `KM_i2c_init` → đọc `IC_COMP_TYPE` và **return `-ENODEV` nếu không khớp `0x44570140`**. Nếu I2C2/I2C3 không được cấp clock, lệnh đọc sẽ trả rác → `LPPP_i2c_init` in lỗi và **`return` sớm**, bỏ dở việc khởi tạo:

```c
ret = KM_i2c_probe(&device[i]);
if (ret) {
    ApiSysDebug_Printf(-1, -1, "failure KM_i2c_probe I2C[%d]=%d\n", i, ret);
    return;     // ← bỏ luôn các bus còn lại
}
```

May là I2C0 (bus duy nhất quan trọng) được probe **đầu tiên**, nên lỗi ở bus 2/3 không ảnh hưởng. Nhưng nếu ai đó đổi thứ tự vòng lặp thì sẽ vỡ.

## 🟡 5.7 `i2c_MC_PWR_EN()` có hai lần đọc debug bọc trong `#if 1`

```c
#if 1 /* DEBUGのため、削除可能 */  /* I2C Read */
    ret = LPPP_i2c_read(0, 0x3E, 0x70, &readData);
    ...
#endif
```

Xuất hiện **hai lần** (trước và sau lệnh ghi). Mỗi `LPPP_i2c_read` là 2 transaction → hàm này tốn **5 transaction I2C** trong khi chỉ cần 1. Comment ghi rõ "có thể xoá" nhưng vẫn còn.

## 🟡 5.8 Không có mutex bảo vệ `device[]`

`struct lppp_i2c_dev device[4]` là biến toàn cục, chứa trạng thái transaction (`tx_buf`, `msg_write_idx`, `status`…). `LPPP_i2c_write/read` ghi thẳng vào nó rồi gọi `KM_i2c_xfer` — **không có khoá nào**.

Các nơi gọi: `PwEvtMgr` (prio 0, qua `LPPP_TR8_TR10`), `init_thread` (prio 3, qua `board.c`/`power.c`). Nếu hai context cùng gọi `LPPP_i2c_write(0, ...)` thì `device[0]` bị ghi đè giữa chừng. Thực tế chưa xảy ra vì các đường gọi tách biệt về thời gian, nhưng không có gì bảo đảm điều đó.

---

# Tóm tắt

> I2C trên LPPP là **IP Synopsys DesignWare APB I2C**, bốn instance tại `0xE8006000` + `n×0x800`, điều khiển bằng FIFO chứ không bit-bang: phần mềm nạp địa chỉ vào `IC_TAR` rồi đẩy lệnh/dữ liệu qua `IC_DATA_CMD` (bit8=R/W, bit9=STOP, bit10=RESTART), phần cứng tự sinh START/địa chỉ/SCL/ACK/STOP.
> 
> Trong project, **I2C0 là dây thần kinh giữa LPPP và PS-CPU** — con vi điều khiển quản lý mạch nguồn. Mọi lần chuyển trạng thái nguồn (Boot / Normal / Sleep2 / ErP) đều kèm một lệnh ghi vào thanh ghi `0x68` của PS-CPU, cộng thêm `SLEEP_STATUS_REM` qua `0x6C` trên dòng máy Eagle3, và ghi log sự kiện qua `0xC0` khi công cụ đo tự động được bật. **I2C1** chỉ dùng một lần lúc boot để chỉnh điện áp IO qua chiết áp số MCP4542.
> 
> Có **hai driver song song** cho cùng một IP: bản Marvell (`Hal/quartz/i2c`) chỉ phục vụ MCP4542 và chứa vài lỗi ở các hàm test; bản Konica Minolta (`ApplicationKM/LPPP_I2C.c`) là đường sản xuất, port từ Linux `i2c-designware` nhưng thay ISR bằng polling — và vô tình biến `udelay(5)` thành `spin 1 ms` giữa mỗi byte.