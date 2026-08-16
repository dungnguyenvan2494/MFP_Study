`power_cpu()` là **tầng thấp nhất trong chuỗi điều khiển nguồn** — nơi duy nhất trong LPPP thực sự bật/tắt từng core Cortex-A72 bằng cách ghi thanh ghi PMU của AP806, và cũng là nơi duy nhất phát ra ba event `GO_TO_S2` / `GO_TO_ERP` / `WAKEUP_S1`.

---

# Phần A — Bối cảnh

## A.1 Không gian CPU id

```c
int cluster_id = cpuid >> 2;    // 0 hoặc 1
int cpu        = cpuid & 3;     // 0 hoặc 1 (AP806 có 2 core/cluster)
```

|`cpuid`|cluster|cpu|bit trong `cpu_state`|
|---|---|---|---|
|`0`|0|0|bit 0|
|`1`|0|1|bit 1|
|`4`|1|0|bit 2|
|`5`|1|1|bit 3|

```c
cpu_state |= (1 << cpu) << (cluster_id * NUM_CORES_PER_CLUSTER /*2*/);   // tắt → set bit
cpu_state &= ~((1 << cpu) << (cluster_id * 2));                          // bật → clear bit
```

Quy ước **ngược trực giác: bit = 1 nghĩa là core ĐANG TẮT**. `ALL_CPU_OFF = 0xF`. Giá trị khởi tạo `cpu_state = 0xe` (`0b1110`) tức lúc boot chỉ core 0 chạy.

## A.2 `power == 3` là "tắt", mọi giá trị khác là "bật"

Theo spec SCPI, trường power state có nhiều mức (ON / RETENTION / … / OFF). Ở đây chỉ phân biệt nhị phân: `3` → tắt, còn lại → bật. Các mức retention bị gộp thành "bật hẳn".

## A.3 Toàn hàm nằm trong một mutex

```c
SysApiThread_MutexGet(cpu_power_mutex, MUTEX_GET_WAIT_FOREVER);
… 
SysApiThread_MutexPut(cpu_power_mutex);
```

Serialize bốn lệnh `0x03` liên tiếp mà Linux gửi cho bốn core. Có đúng **một** lối thoát sớm (`pmu_resume` thất bại) và nó có nhả mutex — không rò rỉ.

---

# Phần B — Nhánh TẮT (`power == 3`)

## B.1 Chờ core vào WFI ⚠️ **không hoạt động**

```c
wfi_status = (volatile uint32_t *)CCU_B_PSR(cluster_id);
exit_loop = 20;
while (!(*(wfi_status)) & (0x1<<cpu) && exit_loop-- > 0)
    udelay(1);
```

Ý định: chờ core báo đã vào `WFI` trước khi cắt nguồn. Nhưng biểu thức sai độ ưu tiên — toán tử `!` (một ngôi) bám chặt hơn `&`, nên nó phân tích thành:

```c
( !(*wfi_status) )  &  (0x1 << cpu)
```

`!(*wfi_status)` chỉ trả về **0 hoặc 1**. Do đó:

|`cpu`|`0x1<<cpu`|`(0 hoặc 1) & mask`|Kết quả|
|---|---|---|---|
|`0`|`0x1`|`!reg & 1` = `!reg`|Chờ khi **toàn bộ thanh ghi = 0** — sai ý nhưng có chạy|
|`1`|`0x2`|`1 & 2` = 0, `0 & 2` = 0|**Luôn false → vòng lặp không chạy lần nào**|

Với AP806 (cpu ∈ {0,1}), nghĩa là **một nửa số core không hề được chờ WFI**, nửa còn lại chờ sai điều kiện.

Dòng gốc bị comment ngay phía trên đúng về mặt cú pháp:

```c
//	while( !(*((uint32_t *)CCU_B_PSR(cluster_id)) & wfi_wfe_mask(cpu)) );
```

nhưng lại là vòng lặp **vô hạn không timeout**. Bản vá đã thay một vòng chờ _đúng nhưng treo được_ bằng một vòng chờ _có timeout nhưng hỏng_.

Thêm hai chi tiết củng cố: mặt nạ trong vòng lặp là `0x1<<cpu`, còn `wfi_wfe_mask(cpu) = (2<<cpu)|(8<<cpu)` chỉ được dùng trong `printf` — hai mặt nạ khác nhau. Và ngay cả khi chạy đúng, 20 × `udelay(1)` = **tối đa 20 µs**, quá ngắn để một core kịp vào WFI.

Đây gần như chắc chắn là lý do bản vá OP_BTS-20790 phải rải `udelay` khắp hàm: vòng chờ đồng bộ không hoạt động, nên phải bù bằng delay mù.

## B.2 Ba bước cắt nguồn

```c
/* ① Isolation enable — cách ly I/O của core khỏi phần còn lại */
reg_val = *PWRC_CPUN_CR_REG(cpuid);
reg_val |= PWRC_CPUN_CR_ISO_ENABLE_MASK;      // bit 16
*PWRC_CPUN_CR_REG(cpuid) = reg_val;
udelay(200);
exit_loop = 2000;                              // xác nhận bit đã lên
do { reg_val = *PWRC_CPUN_CR_REG(cpuid); exit_loop--; }
while (!(reg_val & ISO_ENABLE_MASK) && exit_loop > 0);

/* ② Cắt nguồn core */
reg_val &= ~PWRC_CPUN_CR_PWR_DN_RQ_MASK;      // bit 0 — CLEAR để TẮT
*PWRC_CPUN_CR_REG(cpuid) = reg_val;

/* ③ Giữ core trong reset */
mrvl_regwrite32(CCU_B_PRCR(cpuid), CCU_B_PRCR_Reset /*0x0*/);
cpu_state |= (1<<cpu) << (cluster_id*2);
```

**Isolation** phải bật trước khi cắt nguồn, nếu không các chân tín hiệu của core sẽ trôi và làm nhiễu phần logic vẫn còn điện. Đây là trình tự bắt buộc của mọi power gating.

⚠️ Vòng xác nhận `exit_loop = 2000` (`REG_WR_VALIDATE_TIMEOUT`) chạy tít không có delay bên trong, và **khi hết lượt thì không báo lỗi gì** — cứ thế đi tiếp sang bước cắt nguồn.

⚠️ Tên bit gây hiểu nhầm: `PWR_DN_RQ` nghe như "yêu cầu tắt", nhưng ở đây **xoá** nó mới là tắt, và **đặt** nó mới là bật (§C.3). Tên kế thừa từ ATF Marvell; hành vi thực tế ngược với tên.

## B.3 Khối "core cuối cùng vừa tắt"

```c
if (cpu_state == ALL_CPU_OFF) {
    *(volatile uint32_t *)PLAT_WARMBOOT_FLAG_BASE = PLAT_WARMBOOT_FLAG_BASE;
    pmu_phase(pmu_suspend);
    suspended = true;

    intr_event = g_wolMode ? LPPP_INTERNALEVT_GO_TO_ERP
                           : LPPP_INTERNALEVT_GO_TO_S2;
    if (intr_event == GO_TO_S2 || intr_event == GO_TO_ERP)      // ← luôn đúng
        xQueueSend(gst_MsgQHandle_PowerCtrl, &intr_event, portMAX_DELAY);

    CA72WDT_End();
}
```

**Cờ warm boot:** ghi chính địa chỉ `0x7FFFFFF0` làm giá trị. Comment nói rõ _"setup the warmboot flag, non zero.."_ — chỉ cần khác 0, dùng luôn địa chỉ cho tiện. ATF đọc cờ này lúc khởi động lại để biết đây là warm boot chứ không phải cold boot. `my_callback` (wake timer) xoá nó về `0` để ép cold boot.

**`pmu_phase(pmu_suspend)`** là nơi cắt các power domain còn lại — MCix4, PIDI, IRIS, UL, Mech… theo bảng tuần tự trong `power_device_init.h`.

**`g_wolMode` quyết định Sleep2 hay ErP.** Biến này không đến từ SCPI mà từ kênh host command của network stack (`MSG_CMD_SYSTEM_SET_WOL`). Nghĩa là: **cấu hình Wake-on-LAN của người dùng** mới là thứ chọn trạng thái ngủ, không phải Linux nói trực tiếp.

Câu `if` kiểm tra lại chính hai giá trị vừa gán — thừa hoàn toàn, có lẽ là tàn dư từ khi khối này còn nhánh thứ ba.

⚠️ `xQueueSend(..., portMAX_DELAY)` và `CA72WDT_End()` (lấy semaphore, timeout 1 s) đều là **lời gọi có thể block, thực hiện khi đang giữ `cpu_power_mutex`**. Nếu queue 20 slot đầy, hàm treo cùng mutex.

---

# Phần C — Nhánh BẬT (`power != 3`)

## C.1 Khối "core đầu tiên bật lại" — nơi làm gần hết việc

```c
if (cpu_state == ALL_CPU_OFF) {
    CA72WDT_Start();                                        // ① watchdog 90 s
    Bregister_modify(B014_ADDRESS, 1<<B014_BEGIN_PHASE6,
                                   1<<B014_BEGIN_PHASE6);   // ② GHI SPI-FLASH
    if (-1 == pmu_phase(pmu_resume)) {                      // ③ bật lại domain
        SysApiThread_MutexPut(cpu_power_mutex);
        return false;                                        //    ← thoát nửa chừng
    }
    intr_event = LPPP_INTERNALEVT_WAKEUP_S1;                // ④ báo state machine
    xQueueSend(gst_MsgQHandle_PowerCtrl, &intr_event, portMAX_DELAY);

    mrvl_regwrite32(AP806_CCU_B_RVBAR_0, WARM_BOOT_ADDR);   // ⑤ điểm vào warm boot
    mrvl_regwrite32(AP806_CCU_B_RVBAR_1, WARM_BOOT_ADDR);

    intDisable(INTNUM_MCIX2_1);   // ⑥ tắt ngắt GMAC phía LPPP
    clear_wake_ints();            //    gỡ toàn bộ nguồn wake đã đăng ký
    wake_lan();                   //    định tuyến ngắt GMAC trở lại AP qua IRIS ICU
    SysApiSystem_Wake();          //    chuẩn bị bàn giao GMAC cho AP
}
```

**② Ghi flash trên đường thức.** `Bregister_modify` là read-modify-write trên SPI-Flash (`0xF43FF01C`) — tốn hàng chục ms và nằm ngay trên critical path của resume, bên trong mutex. Bit `B014_BEGIN_PHASE6` là "breadcrumb" để phân tích khi máy chết giữa chừng.

**③ Lối thoát nửa chừng.** Nếu `pmu_resume` hỏng, hàm trả `false` nhưng để lại: watchdog **đã bật**, flash **đã ghi**, và **không có event nào được gửi**. State machine kẹt ở `SLEEP2`/`ErP`. Cơ chế phục hồi duy nhất là CA72WDT hết 90 s → `CA72WDT_Expire()` → `HRESET_REQN = 0` → reboot cứng. Nói cách khác, "xử lý lỗi" ở đây chính là _cố tình để watchdog nổ_.

**④ Event được gửi TRƯỚC khi core thực sự bật.** `PwEvtMgr` chạy `LPPP_TR8_TR10_Sleep2_Or_ErPToSleep1` (I2C tới PS-CPU + ghi flash B014 bit13 + ~25 thanh ghi QoS) trong khi `power_cpu` vẫn đang ghi RVBAR / PWR_DN_RQ / ISO / PRCR. Hai task cùng prio 0 nên chúng **đan xen theo tick 10 ms**, không được serialize với nhau. Chúng chạm vào các thanh ghi khác nhau nên không xung đột trực tiếp, nhưng thứ tự "QoS được nạp lại lúc nào so với lúc core thoát reset" là không xác định.

**⑤ RVBAR** (Reset Vector Base Address Register) được đặt `WARM_BOOT_ADDR = 0x1003` cho cả hai cluster, để core khi thoát reset nhảy vào điểm vào warm boot thay vì vector ROM lạnh.

**⑥ Bàn giao GMAC.** Khi ngủ, LPPP tự cầm card mạng để nghe gói Wake-on-LAN; khi thức, phải trả lại cho AP: tắt ngắt phía mình, gỡ wake interrupt, rồi `wake_lan()` định tuyến ngắt GMAC qua IRIS ICU về AP.

## C.2 Trình tự bật core — đảo ngược trình tự tắt

```c
reg_val |=  PWRC_CPUN_CR_PWR_DN_RQ_MASK;   // ① cấp nguồn
udelay(100);                                //    "Wait for CPU on, up to 100 uSec"
reg_val &= ~PWRC_CPUN_CR_ISO_ENABLE_MASK;   // ② gỡ isolation
do { … } while ((reg_val & ISO_ENABLE_MASK) && exit_loop > 0);   // xác nhận bit đã XUỐNG
udelay(100);                                //    OP_BTS-19448 [HeliosMLK]
mrvl_regwrite32(CCU_B_PRCR(cpuid), CCU_B_PRCR_nReset /*0x10001*/);   // ③ thả reset
cpu_state &= ~((1<<cpu) << (cluster_id*2));
suspended = false;
```

## C.3 Đối chiếu hai chiều

|Bước|TẮT|BẬT|
|---|---|---|
|1|(chờ WFI — hỏng)|`pmu_resume`, RVBAR, bàn giao GMAC|
|2|Isolation **enable** (bit 16 ← 1)|Nguồn **on** (bit 0 ← 1)|
|3|Nguồn **off** (bit 0 ← 0)|Isolation **disable** (bit 16 ← 0)|
|4|PRCR ← `Reset` (0x0)|PRCR ← `nReset` (0x10001)|
|5|`cpu_state|= bit`|

Hai trình tự đối xứng gương — đúng như yêu cầu của power gating: khi tắt thì cách ly trước rồi mới cắt điện; khi bật thì cấp điện trước rồi mới bỏ cách ly.

---

# Phần D — Tổng hợp phát hiện

## 🔴 D.1 Vòng chờ WFI sai độ ưu tiên toán tử

```c
while (!(*(wfi_status)) & (0x1<<cpu) && exit_loop-- > 0)
```

Phân tích thành `(!(*wfi_status)) & (0x1<<cpu)` → với `cpu == 1` luôn bằng 0, vòng lặp không chạy lần nào. Nghĩa là **core được cắt nguồn mà không hề xác nhận nó đã vào WFI**. Đúng phải là:

```c
while (!((*wfi_status) & wfi_wfe_mask(cpu)) && exit_loop-- > 0)
```

Đây nhiều khả năng là nguyên nhân gốc khiến phải rải ~14 lần `udelay` khắp hàm.

## 🟠 D.2 Hai vòng xác nhận thanh ghi bỏ qua timeout

Cả hai vòng `exit_loop = REG_WR_VALIDATE_TIMEOUT` đều **không kiểm tra kết quả** sau khi thoát. Nếu isolation không lên/không xuống được, hàm vẫn đi tiếp và vẫn trả `true`.

## 🟠 D.3 Gọi hàm có thể block khi đang giữ mutex

`xQueueSend(portMAX_DELAY)` (2 chỗ), `CA72WDT_End()` / `CA72WDT_Start()` (semaphore 1 s), và `Bregister_modify()` (ghi flash hàng chục ms) — tất cả nằm trong `cpu_power_mutex`. Chưa gây sự cố vì `PwEvtMgr` luôn kịp rút queue, nhưng đây là mẫu thiết kế rủi ro.

## 🟡 D.4 Xử lý lỗi `pmu_resume` để lại trạng thái nửa vời

Watchdog đã bật, flash đã ghi, event chưa gửi. Chỉ CA72WDT (90 s) mới kéo hệ thống ra được.

## 🟡 D.5 Tên bit trái nghĩa với hành vi

`PWRC_CPUN_CR_PWR_DN_RQ` — **xoá** để tắt, **đặt** để bật. Bất cứ ai đọc code theo tên biến sẽ hiểu ngược. Cần đối chiếu datasheet AP806 để xác nhận cực tính thật.

## 🟡 D.6 Biến chết và `volatile` thừa

`int i` chỉ dùng trong khối `#if 0`; `volatile uint32_t reg_val` là biến cục bộ nên `volatile` không cần thiết (có lẽ thêm vào cùng OP_BTS-20790 để chặn compiler tối ưu); câu `if (intr_event == GO_TO_S2 || … GO_TO_ERP)` luôn đúng.