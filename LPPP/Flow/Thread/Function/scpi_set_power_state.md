`scpi_set_power_state` là một **adapter mỏng**: nó chỉ giải mã payload SCPI thành tham số nội bộ rồi ủy quyền toàn bộ công việc cho `power_cpu()`. Bản thân nó không đụng vào thanh ghi nào.

---

## 1. Giải mã payload

Linux/PSCI đóng gói mọi thứ vào **một `uint32_t`** duy nhất (`scpi->payload`):

```
 31        20 19     16 15     12 11      8 7       4 3       0
┌────────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
│  (không    │ suspend │ cluster │   cpu   │ cluster │   cpu   │
│   dùng)    │  (css)  │  power  │  power  │   id    │   id    │
└────────────┴─────────┴─────────┴─────────┴─────────┴─────────┘
```

```c
uint8_t clusterid    = (set_power >> 4)  & 0xf;
uint8_t cpuid        =  set_power        & 0x03;   // ← chỉ 2 bit, không phải 4
uint8_t cpupower     = (set_power >> 8)  & 0xf;    // 3 = tắt, khác = bật
uint8_t clusterpower = (set_power >> 12) & 0xf;    // ← khai báo rồi bỏ
uint8_t suspend      = (payload   >> 16) & 0xf;    // ← khai báo rồi bỏ
uint8_t cpuidb       =  payload          & 0xf;    // ← khai báo rồi bỏ
```

## 2. Ghép thành CPU id tuyến tính — đây là phần "action" duy nhất

```c
power_cpu( cpuid | (clusterid << 2), cpupower );
```

Phép `| (clusterid << 2)` chuyển từ hệ toạ độ **(cluster, cpu)** của PSCI sang **id tuyến tính** mà `power_cpu` dùng:

|cluster|cpu|→ linear id|bit trong `cpu_state`|
|---|---|---|---|
|0|0|`0`|bit 0|
|0|1|`1`|bit 1|
|1|0|`4`|bit 2|
|1|1|`5`|bit 3|

AP806 có 4 core A72, 2 core mỗi cluster — nhưng cluster 1 mang id **4 và 5** chứ không phải 2 và 3, đúng như comment ở [app_scpi.c:126](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L126): _"cluster 1 has ID of 4 & 5. (bit shifted allows for 4 cpus/cluster)"_. Dịch 2 bit chừa chỗ cho tối đa 4 core/cluster ở các biến thể chip khác.

Bên trong `power_cpu` id này lại được tách ra:

```c
int cluster_id = cpuid >> 2;
int cpu        = cpuid & 3;
cpu_state |= (1 << cpu) << (cluster_id * NUM_CORES_PER_CLUSTER /*2*/);
```

nên `cpu_state` là bitmap 4 bit gọn, và `ALL_CPU_OFF = 0xF`. Giá trị khởi tạo `cpu_state = 0xe` (`0b1110`) nghĩa là **lúc boot chỉ có core 0 đang chạy**, ba core kia đang tắt.

## 3. Ba thứ hàm này cố tình bỏ qua

**`clusterpower` và `suspend`** — trong spec ARM SCPI, lệnh `SET_CSS_POWER_STATE` mang cả trạng thái mong muốn ở mức cluster và mức hệ thống. Firmware này **không tin** hai trường đó: nó tự theo dõi `cpu_state` và **tự suy ra** khi nào hệ thống thật sự xuống nguồn (`cpu_state == ALL_CPU_OFF`). Cách này bền hơn — không phụ thuộc Linux điền đúng, nhưng đồng nghĩa Linux không thể ép cluster/system power một cách tường minh.

**Tham số `chan`** không hề được dùng trong thân hàm. Nó được truyền vào chỉ vì cùng chữ ký với các handler khác. (Đối chiếu: `pause_cpu(chan)` ở `case 0x0a` thì _có_ dùng, để tự rung chuông trả lời trước khi vào `wfi`.)

**`have_events`, `TimerConfig`, `ret`** khai báo nhưng không khởi tạo và không dùng — tàn dư copy-paste từ một phiên bản lớn hơn.

## 4. Cặp `udelay` bọc ngoài

```c
udelay(DEF_DELAY_100);   // 100 µs
power_cpu(...);
udelay(DEF_DELAY_100);   // 100 µs
```

`udelay` = `HalChip_wait_us_lower_bound` — **busy-wait thật**, không nhường CPU. Hai lần này thuộc bản vá OP_BTS-20790 (2020/02/25), cùng đợt với ~10 lần `udelay` khác rải bên trong `power_cpu`. Cộng với 2 lần nữa ở epilogue của `process_scpi_cmd`, một lệnh `0x03` tiêu tối thiểu **~1,4 ms chờ suông** trước khi tính công việc thật.
Xem [[power_cpu]]
## 5. Việc thật xảy ra ở đâu

`power_cpu()` ([app_scpi.c:200](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L200)) mới là nơi động vào phần cứng, dưới `cpu_power_mutex`:

**Nếu `cpupower == 3` (tắt):**

```
poll WFI status (tối đa 20 × 1µs)
 → Isolation enable  (PWRC_CPUN_CR bit16)
 → PWR_DN_RQ         (bit0)
 → CCU_B_PRCR = Reset
 → cpu_state |= bit
 → NẾU cpu_state == ALL_CPU_OFF:
      đặt PLAT_WARMBOOT_FLAG, pmu_phase(pmu_suspend), suspended = true
      phát GO_TO_ERP (nếu g_wolMode) hoặc GO_TO_S2 vào gst_MsgQHandle_PowerCtrl
      CA72WDT_End()
```

**Ngược lại (bật):**

```
NẾU cpu_state == ALL_CPU_OFF (tức đây là core đầu tiên bật lại):
   CA72WDT_Start(), ghi B014_BEGIN_PHASE6
   pmu_phase(pmu_resume)  → thất bại thì return false, KHÔNG phát event
   phát WAKEUP_S1 vào gst_MsgQHandle_PowerCtrl
   set RVBAR_0/1 = WARM_BOOT_ADDR, clear_wake_ints(), wake_lan(), SysApiSystem_Wake()
rồi: PWR_DN_RQ off → Isolation disable → thả reset
```

Nên **event nguồn chỉ phát đúng một lần mỗi chu kỳ**, tại thời điểm core cuối tắt hoặc core đầu bật — dù Linux gửi lệnh `0x03` bốn lần (một lần mỗi core).

---

## 6. Hai điểm đáng lưu ý

**Log không khớp hành vi.** Dòng `ApiSysDebug_Printf` in `set_power & 0xf` cho "cpu id", trong khi lời gọi thật dùng `set_power & 0x03`. Với AP806 (2 core/cluster) hai giá trị luôn trùng nhau, nhưng nếu debug trên biến thể có id ≥ 4 trong cluster thì **log sẽ nói một đằng, code làm một nẻo**. Đồng thời `clusterpower`/`suspend` được in ra bằng biểu thức inline chứ không dùng biến đã khai báo — nên xoá ba biến chết đó sẽ không đổi log.

**Macro địa chỉ đúng nhờ trùng hợp.**

```c
#define PWRC_CPUN_CR_REG(cpu_id)  (0xce680000 + ((cpu_id>>2)*2 + cpu_id&0x3) * 0x10)
```

Trong C, `&` có độ ưu tiên **thấp hơn** `+`, nên biểu thức thực chất là `(((cpu_id>>2)*2 + cpu_id) & 0x3)` chứ không phải `((cpu_id>>2)*2 + (cpu_id&0x3))` như ý định. May mắn là với đúng 4 giá trị hợp lệ `{0, 1, 4, 5}` hai cách tính cho kết quả **giống hệt nhau** (offset `0x00/0x10/0x20/0x30`). Nhưng nếu id nằm ngoài tập đó — ví dụ `6` — cách sai cho offset `0x00`, tức **thao tác nhầm lên thanh ghi của core 0**. Đây là lỗi tiềm ẩn, không phải lỗi đang xảy ra; comment `// ported from atf_s800 + small fix` gợi ý macro này đã từng được chỉnh tay.