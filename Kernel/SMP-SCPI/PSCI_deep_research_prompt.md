# Prompt nghiên cứu PSCI chuyên sâu từ Linux Kernel xuống Hardware

Bạn hãy đóng vai một Senior ARM64 Linux Kernel, Trusted Firmware và SoC Power Management Engineer.

Tôi là người mới tìm hiểu Linux kernel, ARM64, PSCI, Exception Level, firmware và quá trình khởi động secondary CPU. Tôi có thể đọc code C và Assembly cơ bản, nhưng chưa hiểu đầy đủ cách Linux kernel gọi firmware để bật một CPU vật lý.

Nhiệm vụ của bạn là nghiên cứu thật sâu toàn bộ PSCI, từ Linux kernel xuống firmware và phần cứng. Mục tiêu cuối cùng là giúp tôi hiểu chính xác mọi hoạt động xảy ra khi Linux muốn bật CPU1, đồng thời biết cách truy code và debug lỗi:

    CPU1 failed to come online

## YÊU CẦU QUAN TRỌNG

1. Giải thích bằng tiếng Việt đơn giản, tự nhiên, phù hợp với người mới.
2. Không được chỉ đưa định nghĩa học thuật.
3. Mỗi khái niệm khó phải có:
   - Giải thích ngắn gọn.
   - Ví dụ đời thường.
   - Vị trí của nó trong flow thực tế.
   - Source code hoặc pseudo-code minh họa.
4. Phải bám sát source code được cung cấp.
5. Không được tự đoán tên function, file, register hoặc hardware block khi chưa có bằng chứng.
6. Phải phân loại rõ:
   - CONFIRMED: Xác nhận trực tiếp từ source, log, Device Tree hoặc tài liệu.
   - INFERENCE: Suy luận hợp lý nhưng chưa có đủ bằng chứng.
   - PLATFORM-SPECIFIC: Phụ thuộc SoC, firmware hoặc board.
   - NEED TO VERIFY: Cần kiểm tra thêm.
7. Nếu không tìm thấy implementation trong Linux kernel, phải chỉ rõ rằng logic có thể nằm trong:
   - ARM Trusted Firmware-A.
   - Vendor secure firmware.
   - Bootloader.
   - Secure Monitor.
   - SCP/MCP firmware.
   - SoC power controller.
8. Không được kết luận lỗi phần cứng chỉ dựa vào `CPU1 failed to come online`.
9. Với mỗi function quan trọng, phải giải thích:
   - Function nằm ở file nào.
   - Ai gọi function.
   - Function gọi tiếp function nào.
   - Input parameter có ý nghĩa gì.
   - Return value có ý nghĩa gì.
   - Function chạy trên CPU nào.
   - Function chạy ở Exception Level nào, nếu xác định được.
   - Function có trực tiếp tác động phần cứng không.
10. Tạo nhiều sequence diagram và activity diagram bằng Mermaid.
11. Mermaid phải dùng cú pháp đơn giản, tránh ký tự dễ gây parse error.
12. Mỗi diagram phải có phần giải thích bằng văn bản phía dưới.
13. Cuối cùng phải tạo một hướng dẫn debug có thể thực hiện trực tiếp trên source và board thật.

---

## PHẦN 1. GIẢI THÍCH BỨC TRANH TỔNG THỂ

Trước tiên, hãy giải thích PSCI bằng ngôn ngữ đơn giản nhất.

Trả lời các câu hỏi:

1. PSCI là gì?
2. PSCI giải quyết vấn đề gì?
3. Vì sao Linux không tự ghi register để bật CPU1?
4. Vì sao việc bật hoặc tắt CPU thường phải đi qua firmware?
5. Secure firmware đóng vai trò gì?
6. PSCI là một firmware, driver, API, protocol hay specification?
7. Sự khác nhau giữa:
   - Linux PSCI driver.
   - PSCI specification.
   - Secure Monitor.
   - Trusted Firmware-A.
   - SoC power controller.
8. Giải thích bằng ví dụ đời thường:
   - Linux là người yêu cầu.
   - PSCI là mẫu yêu cầu chuẩn.
   - Secure Monitor là người có quyền.
   - Power controller là bộ phận trực tiếp bật điện.
   - CPU1 là thiết bị cần được bật.

Tạo sơ đồ tổng quan:

    Linux kernel
        -> PSCI kernel code
        -> SMC hoặc HVC
        -> Secure Monitor
        -> EL3 firmware
        -> platform power management
        -> power controller
        -> CPU1

Giải thích từng tầng riêng biệt.

---

## PHẦN 2. KIẾN THỨC ARM64 NỀN TẢNG

Giải thích đơn giản nhưng đầy đủ:

1. ARM64 Exception Levels: EL0, EL1, EL2, EL3.
2. Linux kernel thường chạy ở EL nào?
3. Hypervisor chạy ở EL nào?
4. Secure Monitor thường chạy ở EL nào?
5. Secure world và Non-secure world là gì?
6. TrustZone liên quan gì tới PSCI?
7. Tại sao Linux EL1 không nhất thiết có quyền điều khiển power/reset trực tiếp?
8. Khi CPU0 thực hiện lệnh SMC:
   - CPU0 có đổi sang một CPU khác không?
   - CPU0 có chuyển Exception Level không?
   - Register nào được dùng để truyền tham số?
   - Firmware trả kết quả qua register nào?
9. Khi gọi HVC thì khác SMC như thế nào?
10. Khi nào PSCI sử dụng SMC?
11. Khi nào PSCI sử dụng HVC?
12. Device Tree property `method = "smc"` hoặc `method = "hvc"` quyết định điều gì?

Tạo bảng so sánh:

| Thành phần | Exception Level | Secure/Non-secure | Vai trò |
|---|---:|---|---|
| Userspace | EL0 | Non-secure | ... |
| Linux kernel | EL1 | Non-secure | ... |
| Hypervisor | EL2 | Non-secure | ... |
| Secure Monitor | EL3 | Secure | ... |

Không giả định EL nếu chưa đủ dữ liệu từ hệ thống cụ thể.

---

## PHẦN 3. CÁCH LINUX PHÁT HIỆN VÀ KHỞI TẠO PSCI

Hãy truy code quá trình Linux phát hiện PSCI.

1. PSCI được mô tả trong Device Tree hoặc ACPI như thế nào?
2. Phân tích ví dụ:

```dts
psci {
    compatible = "arm,psci-0.2";
    method = "smc";
};
```

3. Giải thích `compatible` và `method`.
4. Function nào parse PSCI node?
5. Function nào xác định PSCI version?
6. Function nào chọn PSCI v0.1, v0.2 hoặc phiên bản mới hơn?
7. Khi nào `psci_0_2_set_functions()` được gọi?
8. Giải thích từng dòng:

```c
psci_ops.get_version = psci_get_version;
psci_ops.cpu_suspend = psci_cpu_suspend;
psci_ops.cpu_off = psci_cpu_off;
psci_ops.cpu_on = psci_cpu_on;
psci_ops.migrate = psci_migrate;
psci_ops.affinity_info = psci_affinity_info;
```

9. `psci_ops` được định nghĩa ở đâu?
10. `struct psci_operations` được khai báo ở đâu?
11. Vì sao Linux sử dụng function pointer thay vì gọi trực tiếp?
12. `psci_function_id[]` là gì?
13. Các function ID được tạo như thế nào?
14. Sự khác nhau giữa SMC32 và SMC64 function ID.
15. Giải thích macro tương tự `PSCI_FN_NATIVE(0_2, CPU_ON)`.
16. Function ID được đặt vào register nào khi SMC/HVC được thực hiện?

Tạo sequence diagram cho quá trình:

    Device Tree
      -> PSCI initialization
      -> chọn conduit SMC/HVC
      -> setup invoke_psci_fn
      -> psci_0_2_set_functions
      -> gán psci_ops.cpu_on

---

## PHẦN 4. CPU MASK VÀ TRẠNG THÁI CPU

Giải thích rõ sự khác nhau giữa:

1. `cpu_possible_mask`.
2. `cpu_present_mask`.
3. `cpu_online_mask`.
4. `cpu_active_mask`.
5. Logical CPU ID.
6. Physical CPU ID.
7. MPIDR.
8. CPU affinity.
9. `cpu_logical_map(cpu)`.

Dùng ví dụ SoC có 4 CPU: CPU0, CPU1, CPU2, CPU3.

Minh họa trạng thái ngay sau `boot_cpu_init()`:

    possible = 1111
    present  = 1111
    online   = 0001
    active   = 0001

Sau đó minh họa trạng thái khi CPU1, CPU2 và CPU3 lần lượt online.

Giải thích:

1. Tại sao `boot_cpu_init()` chỉ chạy một lần?
2. Tại sao nó chỉ online boot CPU?
3. Tại sao CPU1, CPU2, CPU3 không chạy lại `start_kernel()`?
4. CPU secondary bắt đầu từ entry point nào?
5. CPU secondary được thêm vào `cpu_online_mask` tại thời điểm nào?
6. `cpu_online(cpu)` thực chất kiểm tra gì?
7. Tại sao `cpu_online(cpu) == false` không đủ để kết luận CPU chưa từng thực thi instruction?

---

## PHẦN 5. TRACE TỪ SMP INIT TỚI PSCI CPU_ON

Hãy truy từng bước call flow thật từ Linux boot đến lúc yêu cầu bật CPU1.

Ví dụ tham khảo, nhưng phải kiểm tra lại theo kernel được cung cấp:

    start_kernel()
        -> rest_init()
        -> kernel_init()
        -> kernel_init_freeable()
        -> smp_init()
        -> cpu_up()
        -> _cpu_up()
        -> architecture-specific CPU bring-up
        -> __cpu_up()
        -> boot_secondary()
        -> get_cpu_ops(cpu)
        -> ops->cpu_boot(cpu)
        -> cpu_psci_cpu_boot(cpu)
        -> psci_ops.cpu_on(...)
        -> psci_cpu_on(...)

Với mỗi function, tạo bảng:

| Function | File | CPU thực thi | Vai trò | Caller | Callee | Kết quả |
|---|---|---|---|---|---|---|

Phân tích kỹ:

```c
static int boot_secondary(unsigned int cpu,
                          struct task_struct *idle)
{
    const struct cpu_operations *ops = get_cpu_ops(cpu);

    if (ops->cpu_boot)
        return ops->cpu_boot(cpu);

    return -EOPNOTSUPP;
}
```

Trả lời:

1. `get_cpu_ops(cpu)` lấy operations từ đâu?
2. Operations được gán cho từng CPU khi nào?
3. `cpu_boot` là function hay function pointer?
4. Struct nào chứa `cpu_boot`?
5. `.cpu_boot = cpu_psci_cpu_boot` được khai báo ở đâu?
6. Khi `ops->cpu_boot(cpu)` chạy, function thực tế nào được gọi?
7. Trường hợp nào `cpu_boot == NULL`?
8. `-EOPNOTSUPP` có nghĩa gì?

---

## PHẦN 6. PHÂN TÍCH cpu_psci_cpu_boot()

Phân tích từng dòng:

```c
static int cpu_psci_cpu_boot(unsigned int cpu)
{
    int err;

    err = psci_ops.cpu_on(
        cpu_logical_map(cpu),
        __pa_symbol(secondary_entry));

    if (err)
        pr_err("failed to boot CPU%d (%d)\n", cpu, err);

    return err;
}
```

Giải thích:

1. Function này được CPU nào thực thi?
2. `cpu` là Linux logical CPU ID hay hardware ID?
3. `cpu_logical_map(cpu)` trả về gì?
4. MPIDR là gì?
5. Aff0, Aff1, Aff2, Aff3 có ý nghĩa gì?
6. Firmware dùng MPIDR để xác định target CPU ra sao?
7. `secondary_entry` là gì?
8. `__pa_symbol(secondary_entry)` làm gì?
9. Vì sao firmware cần physical address?
10. Tại sao không truyền virtual address?
11. MMU của CPU1 ở thời điểm đầu đã được bật chưa?
12. Entry point phải đáp ứng alignment hoặc address range nào?
13. Nếu entry point sai thì CPU1 có thể biểu hiện như thế nào?
14. Return `0` có bảo đảm CPU1 đã online hoàn toàn không?
15. Log `failed to boot CPU%d (%d)` khác gì với `CPU%d failed to come online`?

Tạo sequence diagram riêng cho function này.

---

## PHẦN 7. PHÂN TÍCH psci_cpu_on()

Tìm implementation thực tế của `psci_cpu_on(target_cpu, entry_point)` và phân tích từng dòng.

1. `psci_function_id[PSCI_FN_CPU_ON]` chứa gì?
2. Function ID for CPU_ON được tạo theo PSCI specification như thế nào?
3. Các argument truyền xuống firmware:
   - Function ID.
   - Target CPU.
   - Entry point.
   - Context ID.
4. Context ID dùng cho mục đích gì?
5. Linux kernel hiện tại truyền context ID bằng bao nhiêu?
6. `invoke_psci_fn()` là function pointer hay function thật?
7. Nó được gán thành SMC conduit hoặc HVC conduit ở đâu?
8. Return value thô từ firmware được xử lý như thế nào?
9. Return code PSCI được chuyển thành Linux errno thế nào?

Tạo bảng:

| PSCI return | Ý nghĩa firmware | Linux errno | Hướng điều tra |
|---|---|---|---|
| SUCCESS | ... | 0 | ... |
| NOT_SUPPORTED | ... | ... | ... |
| INVALID_PARAMS | ... | ... | ... |
| DENIED | ... | ... | ... |
| ALREADY_ON | ... | ... | ... |
| ON_PENDING | ... | ... | ... |
| INTERNAL_FAILURE | ... | ... | ... |
| INVALID_ADDRESS | ... | ... | ... |

Phải kiểm tra version PSCI cụ thể trước khi kết luận return code nào được hỗ trợ.

---

## PHẦN 8. TỪ invoke_psci_fn() ĐẾN LỆNH SMC/HVC

Truy code:

    psci_cpu_on()
        -> invoke_psci_fn()
        -> SMC hoặc HVC wrapper
        -> arm_smccc_smc() hoặc arm_smccc_hvc()
        -> assembly instruction

Giải thích:

1. SMCCC là gì?
2. PSCI và SMCCC khác nhau thế nào?
3. PSCI sử dụng SMCCC ra sao?
4. Các register ARM64 x0 đến x7 được dùng thế nào?
5. Với CPU_ON, x0, x1, x2 và x3 chứa gì?
6. Return value nằm trong register nào?
7. `smc #0` thực hiện điều gì?
8. `hvc #0` thực hiện điều gì?
9. CPU0 có tiếp tục chạy không?
10. Context CPU0 được lưu và khôi phục ra sao?
11. CPU0 chuyển từ EL1 đến EL3 theo flow nào?
12. Exception vector nào tại EL3 nhận SMC?
13. Firmware phân biệt PSCI call với các SMC service khác thế nào?

Tạo sequence diagram với các participant:

    Linux EL1
    SMCCC wrapper
    CPU0 hardware
    EL3 vector
    Secure Monitor
    PSCI service

---

## PHẦN 9. PHÍA FIRMWARE VÀ TRUSTED FIRMWARE-A

Nếu source Trusted Firmware-A có sẵn, hãy truy code chính xác. Nếu không có, mô tả flow chuẩn và đánh dấu phần cần xác minh.

1. SMC từ Linux được nhận ở đâu trong TF-A?
2. Runtime service framework là gì?
3. PSCI service được đăng ký thế nào?
4. CPU_ON request được dispatch tới function nào?
5. Kiểm tra các function tương tự `psci_smc_handler()`, `psci_cpu_on()`, `psci_cpu_on_start()` và platform power management operations.
6. Không được giả định tên function nếu source TF-A của platform khác.
7. Platform cung cấp callback nào để bật CPU?
8. `plat_psci_ops` là gì?
9. `pwr_domain_on()` làm gì?
10. `pwr_domain_on_finish()` chạy ở CPU nào?
11. CPU0 thực hiện phần nào?
12. CPU1 thực hiện phần nào sau khi được release?
13. Firmware lưu entry point `secondary_entry` ở đâu?
14. Firmware chuyển entry point cho CPU1 bằng cách nào?
15. Mailbox, per-CPU data hoặc secure memory được sử dụng thế nào?
16. Firmware quản lý trạng thái OFF, ON_PENDING và ON thế nào?
17. Race condition nào có thể xảy ra nếu hai request CPU_ON cùng đến?
18. Vì sao PSCI có trạng thái `ON_PENDING`?

Tạo sequence diagram giữa:

    Linux CPU0
    EL3 Secure Monitor
    PSCI core
    Platform PSCI ops
    Power controller
    CPU1

---

## PHẦN 10. XUỐNG POWER CONTROLLER VÀ HARDWARE

Giải thích chi tiết nhưng dễ hiểu cách firmware có thể bật một CPU vật lý. Phân biệt rõ đây là phần phụ thuộc SoC.

Nghiên cứu các bước có thể có:

1. Xác định power domain của CPU1.
2. Bật nguồn cho CPU power domain.
3. Chờ power-good hoặc power acknowledgment.
4. Bỏ isolation.
5. Bật clock.
6. Cấu hình clock source hoặc PLL nếu cần.
7. Cấu hình boot address hoặc reset vector.
8. Xóa pending reset.
9. Deassert CPU reset.
10. Gửi event hoặc wakeup signal nếu kiến trúc yêu cầu.
11. Đồng bộ cache/coherency.
12. Thêm CPU vào coherent interconnect.
13. Chờ CPU1 xác nhận trạng thái.
14. Firmware trả kết quả về CPU0.

Giải thích các hardware block:

- Power domain controller.
- Clock controller.
- Reset controller.
- Boot address register.
- Mailbox.
- System control register.
- GIC.
- CCI, CCN hoặc CMN coherent interconnect.
- Cache.
- PLL.
- Voltage regulator.
- PMIC.
- SCP hoặc management processor.

Với mỗi block, tạo bảng:

| Block | Vai trò | Ai điều khiển | Nếu lỗi thì biểu hiện |
|---|---|---|---|

Phải nhấn mạnh:

- PSCI specification không quy định cụ thể register nào phải được ghi.
- Register và sequence thật phụ thuộc SoC.
- Muốn biết register thật phải đọc SoC reference manual, TF-A platform code, vendor secure firmware, bootloader code, Device Tree và datasheet power sequence.

Tạo activity diagram cho hardware power-on sequence.

---

## PHẦN 11. CPU1 BẮT ĐẦU THỰC THI

Sau khi CPU1 được release reset, giải thích flow:

    secondary_entry
        -> low-level CPU setup
        -> MMU/cache preparation
        -> page table setup
        -> stack setup
        -> per-CPU data setup
        -> secondary_start_kernel()
        -> CPU hotplug state callbacks
        -> set_cpu_online() hoặc cơ chế tương đương
        -> cpu_online_mask bit được set
        -> scheduler có thể sử dụng CPU1
        -> idle loop

Với từng bước, giải thích:

1. CPU1 bắt đầu ở EL nào?
2. MMU ban đầu bật hay tắt?
3. Virtual address có sử dụng ngay được không?
4. Stack của CPU1 lấy từ đâu?
5. `secondary_data` hoặc cấu trúc tương đương chứa gì?
6. CPU1 tìm page table bằng cách nào?
7. CPU1 cấu hình vector table khi nào?
8. GIC CPU interface được khởi tạo lúc nào?
9. Per-CPU offset được thiết lập lúc nào?
10. Scheduler structures được gắn vào CPU1 khi nào?
11. CPU1 được đánh dấu online ở đâu?
12. CPU1 được đánh dấu active ở đâu?
13. Online và active khác nhau thế nào?
14. Khi nào CPU1 đi vào idle loop?
15. Khi nào CPU1 có thể nhận task từ scheduler?

Tạo sequence diagram có hai luồng song song: CPU0 chờ CPU1 và CPU1 thực hiện secondary startup.

---

## PHẦN 12. PHÂN TÍCH LỖI CPU1 FAILED TO COME ONLINE

Phân tích toàn bộ điểm có thể gây lỗi `CPU1 failed to come online`.

### A. Lỗi trước PSCI call

- CPU logical map sai.
- CPU không present.
- CPU operations chưa được gán.
- `cpu_boot` bằng NULL.
- Entry point sai.

### B. PSCI request bị từ chối

- NOT_SUPPORTED.
- INVALID_PARAMS.
- DENIED.
- ALREADY_ON.
- ON_PENDING.
- INTERNAL_FAILURE.
- INVALID_ADDRESS.

### C. Firmware return SUCCESS nhưng CPU1 không chạy

- Power domain không bật.
- Clock không bật.
- Reset không được release.
- Boot address sai.
- Hardware power acknowledgment timeout.
- Firmware bug.
- SCP firmware bug.
- Coherency chưa được enable.

### D. CPU1 chạy nhưng không đến `secondary_entry`

- Sai reset vector.
- Entry address không hợp lệ.
- Memory không accessible.
- Cache hoặc coherency issue.
- Exception rất sớm.
- CPU được start ở sai EL.

### E. CPU1 vào `secondary_entry` nhưng không đến C code

- MMU setup lỗi.
- Page table sai.
- Stack sai.
- CPU feature mismatch.
- Cache maintenance thiếu.
- Assembly startup loop hoặc exception.

### F. CPU1 vào `secondary_start_kernel()` nhưng không online

- GIC initialization lỗi.
- Per-CPU initialization lỗi.
- CPU hotplug callback treo.
- Timer initialization lỗi.
- Scheduler initialization lỗi.
- Deadlock hoặc spinlock.
- CPU exception hoặc panic.
- Chưa chạy tới điểm set online.

### G. CPU1 đã set online nhưng CPU0 vẫn quan sát offline

- Memory visibility.
- Barrier.
- Race condition.
- Mask bị clear lại.
- CPU chết hoặc bị offline ngay sau đó.

Với mỗi case, tạo bảng:

| Stage | Dấu hiệu | Log có thể thấy | Checkpoint nên thêm | Source cần xem |
|---|---|---|---|---|

---

## PHẦN 13. PHÂN BIỆT HAI LOG

Giải thích rõ sự khác nhau giữa:

    failed to boot CPU1 (-error)

và:

    CPU1 failed to come online

Trả lời:

1. Log nào cho biết PSCI request return lỗi?
2. Log nào cho biết request có thể đã thành công nhưng secondary bring-up không hoàn tất?
3. Nếu chỉ thấy `CPU1 failed to come online` mà không thấy `failed to boot CPU1`, có thể suy luận được gì?
4. Suy luận nào chưa được phép kết luận?
5. Cần thêm log ở đâu để phân biệt các stage?

---

## PHẦN 14. KẾ HOẠCH THÊM DEBUG LOG

Đề xuất checkpoint cụ thể.

### Checkpoint trên CPU0

1. Trước và sau `boot_secondary()`.
2. Trước và sau `ops->cpu_boot()`.
3. Trước và sau `psci_ops.cpu_on()`.
4. In logical CPU ID, MPIDR, entry point VA/PA, PSCI function ID và return code.
5. Trước khi chờ CPU online.
6. Sau khi timeout.
7. In các CPU mask tại thời điểm lỗi.

### Checkpoint trên CPU1

1. Ngay tại `secondary_entry`.
2. Sau thiết lập stack.
3. Sau thiết lập page table.
4. Sau bật MMU.
5. Khi vào `secondary_start_kernel()`.
6. Sau mỗi nhóm initialization.
7. Ngay trước và sau điểm mark online.
8. Trước khi vào idle loop.

Cảnh báo:

- `pr_info()` hoặc `printk()` có thể chưa hoạt động ở Assembly startup rất sớm.
- Logging quá sớm có thể cần Early UART, MMIO UART, firmware trace, shared memory marker, GPIO toggle, JTAG hoặc ETM trace.
- Không được gọi function phức tạp trước khi stack/MMU/per-CPU state sẵn sàng.
- Log có thể thay đổi timing và che race condition.

Cung cấp ví dụ debug code an toàn, nhưng ghi rõ đoạn nào chỉ là pseudo-code.

---

## PHẦN 15. CÔNG CỤ DEBUG PHẦN CỨNG VÀ FIRMWARE

Giải thích cách dùng:

1. `dmesg`.
2. Dynamic debug.
3. Earlycon.
4. `initcall_debug`.
5. Ftrace hoặc function graph tracer, nếu stage cho phép.
6. Crash dump.
7. Pstore hoặc ramoops.
8. TF-A debug build.
9. TF-A `NOTICE`, `INFO`, `VERBOSE`.
10. UART firmware log.
11. JTAG debugger.
12. ARM CoreSight.
13. ETM instruction trace.
14. DAP.
15. Logic analyzer.
16. Oscilloscope để kiểm tra clock/reset/power rails nếu board expose tín hiệu.
17. Register dump của power controller, clock controller, reset controller, GIC và coherent interconnect.

Tạo bảng:

| Công cụ | Quan sát được gì | Không quan sát được gì | Khi nào dùng |
|---|---|---|---|

---

## PHẦN 16. CÁC LỆNH TÌM SOURCE

Cung cấp và giải thích thứ tự sử dụng:

```bash
git grep -n "psci_ops"
git grep -n "struct psci_operations"
git grep -n "psci_0_2_set_functions"
git grep -n "psci_cpu_on"
git grep -n "invoke_psci_fn"
git grep -n "arm_smccc_smc"
git grep -n "cpu_psci_cpu_boot"
git grep -n "\.cpu_boot"
git grep -n "get_cpu_ops"
git grep -n "boot_secondary"
git grep -n "secondary_entry"
git grep -n "secondary_start_kernel"
git grep -n "set_cpu_online"
git grep -n "failed to come online"
git grep -n "failed to boot CPU"
```

Trong Trusted Firmware-A:

```bash
git grep -n "psci_smc_handler"
git grep -n "psci_cpu_on"
git grep -n "pwr_domain_on"
git grep -n "plat_psci_ops"
git grep -n "pwr_domain_on_finish"
git grep -n "CPU_ON"
```

Không coi tên function TF-A trên là chắc chắn tồn tại trong mọi vendor tree.

---

## PHẦN 17. CALL GRAPH CUỐI CÙNG

Sau khi nghiên cứu source, tạo call graph hoàn chỉnh:

    CPU0 Linux
      start_kernel()
        -> ...
        -> smp_init()
        -> cpu_up(1)
        -> ...
        -> boot_secondary(1)
        -> ops->cpu_boot(1)
        -> cpu_psci_cpu_boot(1)
        -> psci_ops.cpu_on()
        -> psci_cpu_on()
        -> invoke_psci_fn()
        -> arm_smccc_smc()
        -> SMC instruction

    CPU0 EL3 firmware
      EL3 exception vector
        -> runtime service dispatch
        -> PSCI SMC handler
        -> PSCI CPU_ON
        -> platform pwr_domain_on()
        -> power/clock/reset controller

    CPU1
      reset release
        -> secure firmware warm boot entry
        -> restore/setup context
        -> exit EL3
        -> secondary_entry
        -> secondary_start_kernel()
        -> CPU initialization
        -> mark CPU online
        -> scheduler/idle

Call graph phải dùng đúng tên function tìm thấy trong source thực tế. Phần chưa xác nhận phải được đánh dấu.

---

## PHẦN 18. SEQUENCE DIAGRAM TỔNG HỢP

Tạo sequence diagram với các participant:

    CPU0 Linux EL1
    PSCI Linux driver
    SMCCC
    CPU0 EL3
    PSCI firmware
    Platform power driver
    Power controller
    CPU1 EL3
    CPU1 Linux EL1

Diagram cần thể hiện:

1. CPU0 muốn online CPU1.
2. Linux xác định MPIDR.
3. Linux xác định physical address của `secondary_entry`.
4. Linux thực hiện PSCI CPU_ON.
5. SMC trap vào EL3.
6. Firmware validate request.
7. Firmware lưu entry point.
8. Firmware bật power domain.
9. Firmware bật clock.
10. Firmware release reset.
11. CPU1 bắt đầu chạy.
12. CPU1 đi qua firmware warm boot path.
13. CPU1 vào `secondary_entry`.
14. CPU1 vào `secondary_start_kernel()`.
15. CPU1 được mark online.
16. CPU0 nhìn thấy CPU1 online.
17. CPU1 tham gia scheduler hoặc idle loop.

Thêm nhánh lỗi cho:

- PSCI return error.
- Firmware return success nhưng CPU1 không start.
- CPU1 start nhưng bị treo trước trạng thái online.

---

## PHẦN 19. ACTIVITY DIAGRAM DEBUG

Tạo activity diagram theo logic:

    Có log CPU1 failed to come online?
        -> Có log failed to boot CPU1 không?
            -> Có:
                đọc PSCI return code
                kiểm tra MPIDR
                kiểm tra function ID
                kiểm tra entry point
                kiểm tra firmware log
            -> Không:
                PSCI có return 0 không?
                    -> Không xác định:
                        thêm log trước/sau PSCI call
                    -> Có:
                        CPU1 có vào secondary_entry không?
                            -> Không:
                                kiểm tra firmware, power, clock,
                                reset, boot address, EL transition
                            -> Có:
                                CPU1 có vào secondary_start_kernel không?
                                    -> Không:
                                        kiểm tra assembly/MMU/page table/stack
                                    -> Có:
                                        CPU1 có chạy tới mark online không?
                                            -> Không:
                                                kiểm tra GIC, timer,
                                                per-CPU, CPUHP, deadlock
                                            -> Có:
                                                kiểm tra mask visibility,
                                                race hoặc CPU bị offline lại

---

## PHẦN 20. CHECKLIST DEBUG THỰC TẾ

Tạo checklist theo thứ tự ưu tiên:

1. Xác định log đầy đủ trước và sau lỗi.
2. Xác định kernel version.
3. Xác định ARM32 hoặc ARM64.
4. Xác định PSCI version.
5. Xác định conduit SMC hoặc HVC.
6. Xác định CPU logical ID và MPIDR.
7. Xác định `secondary_entry` physical address.
8. Kiểm tra return của `psci_ops.cpu_on()`.
9. Giải mã PSCI error code.
10. Kiểm tra firmware log.
11. Kiểm tra CPU1 có vào secondary entry không.
12. Kiểm tra MMU/stack/page table cho CPU1.
13. Kiểm tra CPU1 có vào `secondary_start_kernel()` không.
14. Kiểm tra điểm mark online.
15. Kiểm tra GIC và timer.
16. Kiểm tra power/clock/reset registers.
17. So sánh CPU1 với CPU2 hoặc CPU3 nếu các CPU khác online được.
18. So sánh cold boot với resume.
19. So sánh boot thành công với boot thất bại.
20. Xác định lỗi ổn định hay phụ thuộc timing.

Mỗi mục phải chỉ rõ:

- Cần xem file nào.
- Cần xem function nào.
- Cần thêm log nào.
- Kết quả nào là bình thường.
- Kết quả nào là bất thường.
- Bước tiếp theo tương ứng.

---

## PHẦN 21. PHƯƠNG PHÁP SO SÁNH CPU1 VỚI CPU2/CPU3

Nếu hệ thống có 4 CPU nhưng chỉ CPU1 lỗi, hướng dẫn so sánh:

1. MPIDR của từng CPU.
2. Power domain mapping.
3. Clock/reset bit.
4. CPU topology.
5. Device Tree CPU nodes.
6. `enable-method`.
7. CPU operations.
8. PSCI return code.
9. Secondary entry address.
10. Firmware mailbox/context.
11. GIC redistributor.
12. Per-CPU timer.
13. Cache/coherency configuration.
14. Online mask.
15. CPU hotplug state.

Nếu CPU1 lỗi nhưng CPU2 và CPU3 thành công, nêu rõ giả thuyết nào mạnh hơn và giả thuyết nào yếu hơn.

---

## PHẦN 22. OUTPUT CUỐI CÙNG

Trình bày kết quả theo cấu trúc:

1. Tóm tắt cho người mới.
2. PSCI là gì.
3. Các thành phần tham gia.
4. Kiến thức ARM Exception Level.
5. PSCI initialization.
6. SMP secondary CPU bring-up.
7. Linux call flow.
8. SMC/HVC call flow.
9. Firmware call flow.
10. Hardware power-on flow.
11. CPU1 secondary startup flow.
12. CPU masks và trạng thái CPU.
13. Những điểm có thể thất bại.
14. Phân tích `CPU1 failed to come online`.
15. Hướng dẫn thêm debug log.
16. Hướng dẫn debug firmware và hardware.
17. Sequence diagrams.
18. Activity diagrams.
19. Call graph.
20. Checklist điều tra thực tế.
21. Các thông tin còn thiếu.
22. Kết luận đã xác nhận và giả thuyết.

Mỗi phần phải có:

- Giải thích đơn giản.
- Giải thích kỹ thuật.
- Source reference.
- Flow hoặc diagram nếu phù hợp.
- Dấu hiệu nhận biết khi lỗi.
- Cách kiểm tra thực tế.

---

## SOURCE CODE VÀ DỮ LIỆU ĐẦU VÀO

Tôi sẽ cung cấp lần lượt:

1. Kernel version: `[Điền kernel version]`
2. Architecture: `[ARM64 hoặc ARM32]`
3. SoC: `[Điền tên SoC]`
4. Board: `[Điền tên board]`
5. Device Tree PSCI node: `[Dán nội dung]`
6. CPU nodes: `[Dán nội dung]`
7. Function `boot_secondary()`: `[Dán source]`
8. Function `cpu_psci_cpu_boot()`: `[Dán source]`
9. Function `psci_cpu_on()`: `[Dán source]`
10. Implementation `invoke_psci_fn()`: `[Dán source]`
11. `secondary_entry`: `[Dán Assembly]`
12. `secondary_start_kernel()`: `[Dán source]`
13. Function chứa log `CPU%d failed to come online`: `[Dán source]`
14. Log boot đầy đủ: `[Dán log]`
15. Trusted Firmware-A hoặc vendor firmware source: `[Dán source hoặc ghi không có]`
16. SoC register manual: `[Dán thông tin hoặc ghi không có]`

---

## QUY TẮC PHÂN TÍCH SOURCE

Khi tôi cung cấp source:

1. Phân tích chính xác theo source được cung cấp.
2. Không thay thế source thật bằng flow generic.
3. Trích tên file, function và line nếu có.
4. Nếu cần file khác, nói chính xác file, symbol, lệnh `git grep` và lý do.
5. Không yêu cầu toàn bộ repository nếu chỉ cần một số function.
6. Không bỏ qua Assembly.
7. Không bỏ qua Device Tree.
8. Không bỏ qua firmware.
9. Không dừng ở `psci_cpu_on()`.
10. Phải truy đến nơi thật sự thao tác power, clock, reset, boot address và coherency.
11. Nếu source firmware không có, chỉ ra chính xác ranh giới mà từ đó không còn đủ bằng chứng.
12. Luôn phân biệt sáu trạng thái:
    - Linux yêu cầu CPU_ON.
    - Firmware chấp nhận CPU_ON.
    - Hardware được bật.
    - CPU bắt đầu chạy.
    - CPU vào Linux.
    - CPU được đánh dấu online.

Đây là sáu trạng thái khác nhau, không được gộp lại.

Bắt đầu bằng cách tạo một bản đồ tổng thể dễ hiểu, sau đó mới đi sâu từng tầng. Không được nhảy ngay vào Assembly hoặc register khi người đọc chưa hiểu PSCI đứng ở đâu trong toàn hệ thống.
