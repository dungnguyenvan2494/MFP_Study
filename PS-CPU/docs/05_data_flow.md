 # PS-CPU (pscpu_s800) — 05. Data Flow

**Phạm vi của tài liệu này:** mọi mẩu state quan trọng trong firmware — nó
sống ở đâu, ai tạo/ghi/đọc nó, nó sống được bao lâu, và context nào chạm vào
nó — cộng với hai trace kinh điển (hardware→application và chiều ngược lại)
kèm trích dẫn chính xác cấp register. Đây là tài liệu đồng hành với
`03_execution_model.md` (các context) và `04_call_graph.md` (control flow —
đọc file đó trước; tài liệu này nói về *state* mà các lệnh gọi đó đi xuyên
qua). Các nhãn bằng chứng tuân theo quy ước của `SESSION_CONTEXT.md`. Mọi
trích dẫn đều là các lần đọc trực tiếp `file:line` đối chiếu với
`PS-CPU/pscpu_s800/`, bao gồm — mới trong phase này — source I2C của vendor
`STM32F0xx_HAL_Driver`, vốn chưa được mở ra trong bất kỳ phase nào trước đó.

---

## 1. Hình dạng data-flow của firmware này

Không có RTOS, nên pipeline kinh điển "Hardware → Register → ISR → Queue →
Task → Application" từ template không áp dụng theo nghĩa đen — không có
queue. Những gì thực sự tồn tại, đã được xác nhận bằng cách lần theo mọi
đường đi trong §4-§5:

```
Hardware register  →  ISR (HAL-internal, then app callback)  →  a plain
struct/array field (NOT a queue)  →  polled by the super-loop next iteration
→  Application logic  →  a plain struct/array field  →  ISR-armed transfer
→  Hardware register
```

Giai đoạn "buffer" luôn là một trong số ít các biến static kiểu
`struct`/array cấp module (§2) — không bao giờ là một ring buffer với chỉ số
head/tail ngoại trừ ring buffer đúng nghĩa duy nhất trong toàn bộ firmware
(`st_EventRecord[]`, §2.6). Có đúng một kênh DMA trong toàn bộ cấu hình của
cả hai image và nó không bao giờ được bật (`03_execution_model.md` §6), nên
danh mục "DMA buffer" không áp dụng ở đây.

---

## 2. Danh mục các đối tượng dữ liệu quan trọng

### 2.1 State giao thức I2C — bề mặt dùng chung bận rộn nhất trong firmware

**`i2c_info`** (struct `__i2c_info`)
- Type: `static struct __i2c_info { int state; uint8_t rx_buf[16]; uint8_t* rx_buf_current; uint8_t second_addr; int write_comp; int read_comp; int xfer_size; bool rx_during; uint32_t rx_tick_last; bool tx_during; uint32_t tx_tick_last; uint32_t error_code; bool dir_error; }` — `main/App/km_i2c.c:33-47` (IAP có instance riêng của chính nó với cùng cấu trúc trong `km_i2c_iap.c`, không phải cùng vùng nhớ).
- Owner: subsystem I2C bên trong mỗi image (bản của Main và bản của IAP hoàn toàn độc lập với nhau — không có linkage dùng chung).
- Creator: static storage, các field được zero hóa bởi `i2c_recv_first()` (`km_i2c.c:317-343`), được gọi lúc boot và lại được gọi bởi `i2c_sw_reset()`.
- Writers: `rx_comp()` (ISR context, I2C1 IRQ — set `read_comp`/`write_comp`/`error_code`/`rx_during`/`tx_during`/các trường tick, `km_i2c.c:171-255`); `change_state()` (main-loop context, chỉ được gọi từ `i2c_recv_wait()` — set `state`, clear `*comp`, set `rx_buf_current`, `km_i2c.c:274-304`); bản thân driver HAL của vendor ghi trực tiếp `i2c_info.rx_buf[]`, từng byte một, từ **bên trong chính code ISR của nó**, độc lập với bất kỳ hàm app nào (xem §4.1 — điều này không hề rõ ràng nếu chỉ đọc `km_i2c.c`).
- Readers: `i2c_recv_wait()` (main-loop, every iteration, `km_i2c.c:485-758`); `i2c_check_error()` (main-loop, đọc một bản copy đã latch dưới một critical section, `km_i2c.c:379-461`).
- Lifetime: suốt vòng đời tiến trình của image (được re-initialize, không phải reallocate, mỗi lần `i2c_sw_reset()`).
- Execution contexts: **cả** ISR (I2C1) lẫn main-loop đều chạm vào struct này — đây là đối tượng shared-state ISR/main-loop định danh cho firmware này.
- Synchronization: không có ở cấp struct — các field riêng lẻ được đọc/ghi dưới các cặp ngoặc `HAL_NVIC_DisableIRQ(I2C1_IRQn)` ad hoc trong `change_state()` (`:282,302`) và `i2c_check_error()` (`:392,401`), nhưng bản thân `rx_comp()` chạy trong khi I2C1 hoàn toàn được bật (nó *chính là* ISR của I2C1) và không có ngoặc nào bảo vệ một reader ở main-loop khỏi một lần ghi của `rx_comp()` rơi vào giữa lúc đọc bên ngoài hai hàm đó — ví dụ, `switch(i2c_info.state)` của chính `i2c_recv_wait()` và các lần đọc `rx_buf`/`second_addr` của nó (`:503-758`) **không** được bọc trong bất kỳ IRQ-disable nào.
- Purpose: toàn bộ state machine giao thức I2C phía slave và vùng dàn dựng (staging) nhận/gửi.

**`i2c_info.rx_buf[16]`** — bộ đệm byte thực sự bên trong struct ở trên.
- Written by: `I2C_Slave_ISR_IT()` của STM32 HAL, `(*hi2c->pBuffPtr++) = hi2c->Instance->RXDR;` — `common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c.c:3432` — một byte cho mỗi lần interrupt I2C1, hoàn toàn bên trong code HAL của vendor, không bao giờ được `rx_comp()` chạm vào.
- Read by: `i2c_recv_wait()` (`km_i2c.c:508,633,658` etc.) một khi `write_comp` được quan sát là đã set.
- Note: `rx_buf_current` (một con trỏ thô *trỏ vào* mảng này, được `rx_comp()` tăng dần từng byte một trong phần xử lý reload của nó, `km_i2c.c:198-214`) là thứ mà `valid_second_addr()` kiểm tra để NACK một second address ngoài phạm vi *ngay khi transfer vẫn đang diễn ra* — tức là việc kiểm tra tính hợp lệ của địa chỉ diễn ra byte-theo-byte, trong ISR context, trước khi toàn bộ lệnh thậm chí được nhận xong.

**`tx_data`** (module-level `uint16_t`, `km_i2c.c:463`, IAP có bản copy riêng)
- Writers: chỉ `i2c_recv_wait()` (main-loop) — được set thành bất kỳ giá trị register nào mà I2C master yêu cầu đọc, ngay trước khi `change_state()` arm `Driver_I2C0.SlaveTransmit((uint8_t*)&tx_data, ...)`.
- Readers: driver HAL của vendor, copy nó ra `I2C1->TXDR` từng byte một bên trong chính ISR của nó (`stm32f0xx_hal_i2c.c:3457`, cùng hàm với đường RX).
- Lifetime: được tái sử dụng cho mỗi giao dịch đọc I2C — một biến scratch duy nhất, không phải được cấp phát theo từng giao dịch.
- Risk: vì đây là một scratch dùng chung duy nhất (không phải bản copy riêng cho từng giao dịch), một lệnh gọi `i2c_recv_wait()` thứ hai ghi đè `tx_data` trước khi lần đọc trước đó clock xong sẽ làm hỏng phản hồi — `[INFERRED]` thực tế không thể xảy ra vì `i2c_recv_wait()` chỉ gọi lại `change_state()` sau khi `write_comp`/`read_comp` xác nhận transfer trước đó đã hoàn tất, nhưng điều này được thực thi bởi control flow xung quanh, chứ không phải bởi bất cứ điều gì nội tại trong chính `tx_data`.

**`g_checksum_value`** (`uint16_t`, `km_i2c.c:55`) / **`g_main_version[24]`** (`uint8_t`, `km_i2c.c:58`) / **`g_checki2c_value`** (`static uint16_t`, `km_i2c.c:60`)
- Creator/writer (Main): được tính một lần lúc boot bởi `checksum_calc()`/`GetVersionString()` (`entry.c:56,60`); `g_checki2c_value` có thể được ghi bất cứ lúc nào bởi I2C master (`km_i2c.c:683`, một register test loopback thuần túy).
- Creator/writer (IAP): được tính lại sau mỗi lần cập nhật firmware hoàn tất (`km_i2c_iap.c:345-346`) — **cùng những hàm đó**, dùng chung source, xem `04_call_graph.md` §5, quan sát 3.
- Readers: `i2c_recv_wait()` khi có các lần đọc `EXTEND_CHECKSUM_COMMAND`/`EXTEND_VERSION_COMMAND` (`km_i2c.c:574,583`).
- Lifetime: `g_checksum_value`/`g_main_version` về cơ bản là write-once-mỗi-lần-boot ở Main, write-once-mỗi-lần-cập-nhật ở IAP.

### 2.2 Bảng địa chỉ I/O mở rộng — cầu nối I2C↔GPIO

**`g_io_extend_memory[TYPE_Io_Extend_Max]`** (`uint32_t[5]`, `km_extend_io.c:67`)
- Owner: `km_extend_io.c` (chỉ Main — IAP không có phần tương đương).
- Creator: được zero-initialize (BSS), lần đầu được điền bởi `io_extend_memory_write()` (`entry.c:71`) từ các mức GPIO thực trước cả khi giao diện I2C được arm.
- Writers: `io_extend_write()` (main-loop, I2C-driven, `km_extend_io.c:249-327`); `io_extend_memory_write()` (chỉ lúc boot); một vài hàm `*_proc()` set trực tiếp từng bit riêng lẻ (ví dụ, `mc_p_on_eagle_proc()` set các bit `MC_P_ON`/`IR_P_ON` tại `km_extend_io.c:782,784`; `sleep_status_rem_sparrow_proc()` set bit `MC_PWR_EN` tại `:1001`) — nên mảng này có **nhiều hơn một hàm ghi ngoài điểm vào "chính thức" `io_extend_write()`**, tất cả đều trong main-loop context.
- Readers: `io_extend_read()` (các lần đọc I2C-driven); gần như mọi hàm `*_proc()` trong `km_extend_io.c` đọc các bit cụ thể để quyết định trạng thái output GPIO (ví dụ, điều kiện macro `_MC_P_ON_OK()` không đọc nó, nhưng `mc_p_on_eagle_proc()`/`ir_p_on_proc()`/`sleep_status_rem_*_proc()` thì có).
- Lifetime: suốt vòng đời tiến trình, liên tục bị mutate.
- Execution context: **chỉ main-loop** — không bao giờ bị chạm tới từ một ISR, khác với `i2c_info`.
- Synchronization: không có — theo nghĩa chặt chẽ thì không cần thiết vì không có gì ở đây được ghi từ ISR context, nhưng xem §5 về một vấn đề staleness kiểu đọc-trước-khi-ghi trong cùng một lần lặp mà mảng này là một phần.
- Purpose: không gian địa chỉ "extend register" mà I2C master bên ngoài đọc/ghi, tách rời khỏi state GPIO tức thời để logic `*_proc()` nhiều bước (kiểm tra 24V/chattering, trình tự) có thể gate thời điểm một output được yêu cầu thực sự đến chân pin.

**`io_extend[14]`** (`const km_io[]`, `km_extend_io.c:50-65`)
- Không phải mutable state — một bảng hằng biên dịch ánh xạ mỗi trong số 14 tín hiệu vật lý tới `{extend_num, bit, port, pin}`. Được liệt kê ở đây vì `io_extend_read()`/`io_extend_write()`/`io_extend_memory_write()` đều duyệt qua nó như phương tiện duy nhất để dịch địa chỉ→pin; đây là nguồn sự thật duy nhất cho ánh xạ đó và không xuất hiện ở đâu khác.

### 2.3 State software timer / debounce

**`g_km_timer[TYPE_Km_Timer_Kind_Max]`** (`static km_timer_st[]`, `km_it.c:18`)
- Owner: `km_it.c`.
- Writers: `km_tim_callback()` — **ISR context**, TIM3, decrement mỗi slot đã được arm mỗi 1 ms và chuyển các slot hết hạn sang `End` (`km_it.c:340-348`); `km_timer_set()` — main-loop context, được gọi từ ~15 hàm `*_proc()`/`*_timer_*()` khác nhau trải khắp `km_extend_io.c` và `km_alarm_wake.c` để arm/clear một slot.
- Readers: `km_timer_get_state()`, chỉ được gọi từ code main-loop (không bao giờ từ một ISR) để kiểm tra một timer đã hết hạn hay chưa.
- Lifetime: suốt vòng đời tiến trình; các slot riêng lẻ liên tục được arm/tiêu thụ/rearm.
- Execution context: **ISR (writer, decrement) + main-loop (writer, arm; reader, check)** — mảng shared thứ hai được xác nhận giữa ISR/main-loop trong firmware này, cùng với `i2c_info`.
- Synchronization: không có trực tiếp trên mảng này. `mc_tim_callback()` bọc chính thân của nó trong `HAL_NVIC_DisableIRQ/EnableIRQ(TIM3_IRQn)` (`km_it.c:333,350`), điều này vô nghĩa đối với việc tái nhập (reentrancy) của chính nó nhưng không làm gì để ngăn một lần ghi `km_timer_set()` ở main-loop xen kẽ với lần decrement của ISR trên *cùng* các field struct (`state`, `ms_time`) ở một chỉ số khác — `[INFERRED]` vô hại trong thực tế chỉ vì mỗi "loại" timer được sở hữu về mặt logic bởi đúng một feature main-loop tại một thời điểm và ISR chỉ bao giờ decrement, không bao giờ set `ms_time` tăng lên.
- Purpose: mọi timeout/debounce/độ trễ trình tự trong Main image (bảng ánh xạ khái niệm RTOS ở `03_execution_model.md` §7 đã liệt kê) đều quy về một lần đọc hoặc ghi vào đúng mảng này.

**`anti_chattering_info[3]`** (`static km_anti_chattering_st[]`, `km_it.c:20-54`)
- Writers: `km_exti_callback()` — **ISR context** (EXTI), qua `start_anti_chattering()` set `status`/`start_level`/`prev_level` và arm một slot `g_km_timer[]` (`km_it.c:489-501`); `anti_chattering_proc()` — main-loop context, cập nhật `prev_level`/`status` mỗi lần lặp (`km_extend_io.c:1438-1467`).
- Readers: `anti_chattering_proc()` (main-loop); macro `IS_CHECKING_CHATTERING()`, được dùng làm điều kiện guard bên trong ~10 hàm `*_proc()` main-loop khác (`s800_power_on`, `mc_p_on_*_proc`, `ir_p_on_proc`, `sleep_mode`, v.v.) — nên field `status` của mảng nhỏ 3-entry này gate một phần lớn các quyết định trình tự nguồn điện của firmware.
- Execution context: ISR (arm) + main-loop (drain + các bên tiêu thụ chỉ-đọc khác).
- Synchronization: none.
- Purpose: state machine debounce cho 3 tín hiệu vật lý (MSW_ON, POWER_MONITOR, MONI_24V11), mỗi entry mang một callback `intr_proc` tùy chọn (`04_call_graph.md` §3.7).

**`ei_ref[32]`** (`static int[]`, `km_it.c:19`)
- Writers/readers: chỉ `km_disable_irq()`/`km_enable_irq()` (`km_it.c:436-460`), một bộ đếm tham chiếu cho mỗi giá trị `IRQn_Type` có thể có.
- Execution context: được gọi từ cả code ISR lẫn main-loop (đây là một helper nesting-IRQ mục đích chung), nhưng chỉ bao giờ được chạm tới với interrupt được ngầm serialize nhờ chính là cơ chế disable/enable.
- Purpose: làm cho `km_disable_irq`/`km_enable_irq` nest an toàn — primitive critical-section "đúng đắn" theo `03_execution_model.md` §7, trái ngược với các bypass `NVIC_DisableIRQ` thô ở nơi khác.

### 2.4 Cờ interrupt-pending và cờ mode

**`pending_factor`** (`extern uint32_t`, defined `km_it.c:17`)
- Writers: `set_pending_factor_bit()`/`clear_pending_factor_bit()` — được gọi từ `km_exti_callback()` (**ISR**, cho `_HRESET_REQ_Pin`/`AP_PWR_EN_Pin`, `km_it.c:254,257`) và từ `intr_pending_proc()` (**main-loop**, clear sau khi xử lý, `km_extend_io.c:1419,1423`) và `s800_power_off()` (**main-loop**, clear tất cả, `km_extend_io.c:535`).
- Readers: `read_pending_factor_bit()`, chỉ được gọi từ `intr_pending_proc()` (main-loop) và điều kiện gate của `sleep_mode()` (`km_extend_io.c:1400`, đọc trực tiếp biến thô, không qua accessor).
- Synchronization: **đối tượng dữ liệu duy nhất trong firmware này có một cơ chế bảo vệ tường minh, có chủ đích** — mọi accessor (`set/clear/read_pending_factor_bit`) đều đi qua `get_pending_factor()`/`put_pending_factor()` (`km_it.c:141-163`), hàm này disable/enable cả 3 đường EXTI xung quanh việc truy cập (không phải một IRQ mask chung chung — cụ thể là 3 đường có thể ghi biến này). Đây là mẩu shared state được bảo vệ cẩn thận nhất trong codebase.
- Purpose: nửa "deferred sang main loop" của việc xử lý EXTI (§2.3 trong `03_execution_model.md`).

**`stop_mode`** (`uint8_t`, `km_alarm_wake.c:16`)
- Writers: `set_cycle_time()` set nó ON khi arm RTC alarm (main-loop, `:108`); `km_exti_callback()` clear nó OFF khi wake bởi MSW_ON (**ISR**, `km_it.c:272`); `HAL_RTC_AlarmAEventCallback()` clear nó OFF khi alarm fire (**ISR**, `km_it.c:289`).
- Readers: `km_exti_callback()` (ISR, `km_it.c:269`), `HAL_RTC_AlarmAEventCallback()` (ISR, `km_it.c:287`).
- Execution context: được ghi từ main-loop, được đọc/ghi từ **hai ISR khác nhau** (EXTI và RTC) không có đồng bộ hóa — về mặt lý thuyết có thể xảy ra một race kiểu read-modify-write trên một `uint8_t` thuần giữa ISR EXTI và ISR RTC nếu cả hai fire liên tiếp nhau, nhưng vì cả hai đều cùng NVIC priority (`03_execution_model.md` §2.6, không có preemption giữa chúng) và mỗi cái hoàn tất đầy đủ trước khi cái kia có thể bắt đầu, `[INFERRED]` điều này an toàn trên phần cứng cụ thể này ngay cả khi không có lock tường minh.
- Purpose: state một-bit phân biệt "đang trong chu kỳ wake do RTC drive ở STOP-mode" với hoạt động bình thường.

**Các cờ trình tự nguồn điện (Power-sequencing flags)** — đều là `static`/biến
`int32_t`/`int` cấp module trong `km_extend_io.c`, đều được ghi và đọc **chỉ
từ main-loop context** (không có ISR nào chạm vào bất kỳ cờ nào trong số
này), nên chúng không mang rủi ro cross-context, nhưng được liệt kê ở đây vì
rất nhiều control flow phụ thuộc vào chúng:
- `g_power_on_flg` (`:44`) — `POWER_ON_FLG_START`/`NORMAL`, gate các nhánh first-boot-vs-steady-state của `mc_p_on_*_proc`/`sleep_status_rem_*_proc` cho Eagle/Sparrow.
- `g_first_power_on` (`:46`) — được clear một lần bởi `ap_power_en_proc(STATE_INT)` (`:663`), được đọc bởi logic reset của `s800_power_off()`.
- `g_24v_check_flg_off` (`:45`) — bật/tắt việc giám sát 24V.
- `g_internal_sts_boot_flg` (`:47`) — gate failsafe "boot được thông báo hai lần ⇒ reboot" trong `io_extend_write()` (§2.7.1 của `04_call_graph.md`).
- `is_usb2_oe_from_main_cpu_ctrl` (`:48`) — chỉ được set/clear từ bên trong `io_extend_write()` khi I2C master chủ động drive pin `_USB2_OE` (`:298,306`), được `usb2_oe_proc()` đọc để quyết định liệu việc điều khiển tự động của main loop cho pin đó có nên bị chặn hay không — một cờ handoff "ai sở hữu GPIO này, firmware hay I2C master" đúng nghĩa.
- `g_backupwait_pending` (`uint16_t`, `:69`) — được reset bởi `poweroff_timer_init()`/`poweroff_timer_start()` (`:1267,1291`), được `backupwait_timer_proc()` tiêu thụ (`:1350+`).

**`_status`** (`static TYPE_Km_CA72_Status`, `km_ca72_status.c:11`)
- Writers: `set_ca72_status()`, chỉ được gọi từ `s800_power_off()` (`CA72_OFF`) và `ap_power_en_proc()` (`CA72_ON`/`CA72_SLEEP`) — đều ở main-loop.
- Readers: `get_ca72_status()`, chỉ được gọi từ `sleep_status_rem_eagle_proc()`/`sleep_status_rem_sparrow_proc()` — main-loop, nhưng **sớm hơn trong cùng một lần lặp superloop** so với writer (`04_call_graph.md` §2.2) — trường hợp staleness một-lần-lặp đã được ghi nhận ở đó; được nhắc lại ở đây vì về cơ bản đây là một vấn đề thứ tự data-flow, không phải control-flow.

### 2.5 Handle driver CMSIS / peripheral HAL

**`I2C0_Resources`** (`static I2C_Resources`, `I2C_stm32f0xx.c:73-81`) và `Handle = &hi2c1` của nó
- Fields: `Cb_event` (function pointer, §3.3 của `04_call_graph.md`), `Pending`, `Control_flags`, `DriverCapabilities`, `Handle`, `DataNum`, `Mode`.
- Writers: `I2Cx_Initialize()`/`I2Cx_Uninitialize()` (main-loop, via `Driver_I2C0.Initialize/Uninitialize`).
- Readers: mọi lệnh gọi `Driver_I2C0.*` đều gián tiếp qua `handle_to_resource()` (`:154-165`) để lấy lại struct này.
- Purpose: "instance data" theo pattern CMSIS-driver, biến struct function-pointer không trạng thái `ARM_DRIVER_I2C` (`04_call_graph.md` §3.4) thành một đối tượng có trạng thái — đây là thứ gần nhất với một object instance trong toàn bộ codebase (C, không phải C++).

**`hi2c1` / `hrtc` / `htim3` / `hiwdg`** (các struct handle HAL, đều được định nghĩa trong `mx_init.c:43-49`)
- Owner: `mx_init.c` (tạo ra), nhưng mỗi cái được tham chiếu bằng `extern` và được mutate trực tiếp bởi ISR tương ứng của nó (`stm32f0xx_it.c` đọc trực tiếp `hi2c1.Instance->ISR`, `km_it.c` disable/enable qua IRQ của `htim3`, v.v.) và bởi các thư viện nội bộ của HAL (các field `XferCount`, `pBuffPtr`, `State` bên trong `hi2c1` chính là các field mà vendor HAL code ở §4.1 mutate từ ISR context).
- Purpose: pattern sở hữu HAL tiêu chuẩn — đây là các struct phản chiếu (shadowing) phần cứng, không phải dữ liệu application, nhưng chúng là shared state theo nghĩa đen giữa "ISR" và "driver" mà narrative của Phase 3/4 trước đó chỉ mô tả bằng thuật ngữ lệnh gọi hàm.

**Các register memory-mapped của RTC** (truy cập qua ép kiểu con trỏ thô của `RTC_BASE + offset`, không qua `hrtc`)
- Writers: `i2c_recv_wait()` khi có bất kỳ lần ghi `RTC_*_CMD`/`B_REGISTER_*` nào (`km_i2c.c:657-673`, được bọc trong `RTC_WRITEPROTECT_DISABLE/ENABLE`); `save_rtc_reg()` (dead code, `km_i2c.c:74-78`, không có caller — shim tương thích save/restore RTC đã được nêu trong `SESSION_CONTEXT.md` §6).
- Readers: `i2c_recv_wait()` khi có các lần đọc tương ứng (`km_i2c.c:553`, qua `Driver_I2C0.SlaveTransmit((uint8_t*)(RTC_BASE+addr), ...)` — I2C master đọc trực tiếp *nội dung register sống*, không có staging ở cấp application).
- Purpose: quan sát "RTC có quyền sở hữu chia sẻ" của `03_execution_model.md` được diễn đạt lại như một sự thật data-flow — đây là nơi duy nhất trong firmware nơi "buffer được truyền qua I2C" *chính là* một địa chỉ register phần cứng, không phải một bản copy.

### 2.6 Ring buffer chẩn đoán

**`st_EventRecord[250]`** (`static THRS_EventBuff_Internal[]`, `km_i2c.c:63`) + **`nEventRecordIndex`** (`int`, `km_i2c.c:64`)
- Element type: `{uint16_t ui_Num; uint32_t ui_Time;}` (`km_i2c.h:166-169`).
- Writers: `setEventRecord()` (`km_i2c.c:838-849`) — được gọi từ code main-loop ở ~10 nơi (`s800_power_on`, `sleep_status_rem_*_proc`, và trực tiếp từ một lần ghi I2C của `EXTEND_EVENT_COMMAND`, `km_i2c.c:726`).
- Readers: `i2c_recv_wait()` khi có một lần đọc `EXTEND_EVENT_COMMAND`, hàm này đưa **toàn bộ mảng 250 entry** cho `Driver_I2C0.SlaveTransmit` trong một lần (`km_i2c.c:612-619`).
- Lifetime: suốt vòng đời tiến trình, ring buffer đúng nghĩa — `nEventRecordIndex` wrap về 0 tại `THRD_EVENT_BUFFER_MAX_NUM` (250) và ghi đè lên các entry cũ nhất (`km_i2c.c:841-843`).
- Time base: `ui_Time` được stamp từ `ui_EventCountTimer` (`km_it.c:57`), bộ đếm mili-giây chạy tự do được tăng bởi ISR TIM3 (§2.3) — nên buffer này tương quan các sự kiện main-loop với đồng hồ tường-thuật-kể-từ-boot do ISR drive, hoàn toàn bằng phần mềm.
- Synchronization: không có — một lần đọc I2C nhiều byte của toàn bộ mảng (`sizeof(st_EventRecord)` = 250×~8 byte) không atomic đối với một lần ghi `setEventRecord()` xảy ra đồng thời, mặc dù cả hai phía đều chỉ ở main-loop trong firmware này (không có ISR nào gọi `setEventRecord`), nên cửa sổ race thực tế là "hai hàm main-loop trong cùng một lần lặp," điều này không thể xảy ra với super-loop đơn luồng — `[INFERRED]` an toàn theo cấu trúc, không phải nhờ đồng bộ hóa tường minh.

### 2.7 State chỉ có ở IAP image

**`g_iap_reg[IAP_PG_COMMAND_NUM=4]`** (`uint32_t[4]`, `km_i2c_iap.c:49`)
- Writers: `i2c_recv_wait()` (IAP, main-loop) khi có các lần ghi `IAP_PG_COMMAND`/`IAP_PG_COMMAND_CHANGE_INFO`/`IAP_PG_COMMAND_CHANGE_STATUS` (`km_i2c_iap.c:315,321,332,342,351,370,373`).
- Readers: cùng hàm đó khi có các lần đọc tương ứng (`:265,271`); **và** `msw_on_proc_iap()` — **ISR context** (EXTI4_15) — đọc trực tiếp `g_iap_reg[IAP_REG_INDEX_CALC(IAP_PG_COMMAND_CHANGE_INFO)]` (`km_extend_io_iap.c:35`) để quyết định có reset về Main hay không.
- Execution context: **main-loop writer, ISR reader, không đồng bộ hóa** — lần truy cập cross-context được xác nhận duy nhất cho mảng này. Một lần đọc 32-bit aligned là atomic trên Cortex-M0 theo cấu tạo, nên `[INFERRED]` an toàn trong thực tế (đã được nêu trong bản ghi theo hàm của `04_call_graph.md` cho `msw_on_proc_iap()`), được nhắc lại ở đây như chính đối tượng dữ liệu mà nó là.
- Purpose: toàn bộ state giao thức handshake cập nhật firmware (được arm bằng magic-number, được xác minh bằng checksum, được đánh dấu hoàn tất) giữa I2C master bên ngoài và IAP.

**`g_dest_addr`** (`static uint32_t`, `km_i2c_iap.c:181`, khởi tạo bằng `MAIN_APPLICATION_ADDRESS`)
- Writer: `i2c_recv_wait()` — reset về `MAIN_APPLICATION_ADDRESS` khi magic number kích hoạt erase đến (`:318`), tăng thêm 4 byte cho mỗi lần gọi `flash_write_32()` (`:362`).
- Reader: kiểm tra phạm vi của `flash_write_32()` (`:415-416`) và chính vòng lặp ghi đó dùng nó làm địa chỉ đích (`:361`).
- Purpose: một con trỏ (cursor) ghi flash đang chạy — **không được reset nếu chuỗi cập nhật bị hủy hoặc khởi động lại mà không gửi lại lệnh erase** — `[INFERRED]` một chuỗi cập nhật cục bộ/xen kẽ từ I2C master có thể để cursor này ở một vị trí cũ, khác 0; không có guard nào ở cấp giao thức chống lại việc ghi sai thứ tự trong code được đọc ở phase này.

**`VectorTable[48]`** (`__IO uint32_t[48]`, `entry_iap.c:16-22`, được đặt tại địa chỉ tuyệt đối `0x20000000`)
- Writer: vòng lặp copy của `main()` (`entry_iap.c:55-60`), một lần lúc boot, copy từ vector table flash riêng của IAP.
- Reader: chính phần cứng NVIC của Cortex-M0, sau khi `__HAL_SYSCFG_REMAPMEMORY_SRAM()` remap địa chỉ 0 vào vùng SRAM này (`entry_iap.c:68`) — mọi exception/interrupt tiếp theo trong IAP image là một lần đọc phần cứng vào mảng này, không phải một lệnh gọi hàm theo nghĩa thông thường.
- Purpose: cơ chế relocation vector của `03_execution_model.md` §2.5, được diễn đạt như một đối tượng dữ liệu thay vì một bước control-flow.

---

## 3. Các bảng config/hằng số (bất biến — liệt kê để đầy đủ, không phải "flow")

- `io_extend[14]` (§2.2) — ánh xạ địa chỉ/pin.
- Các field `intr_proc`/`portno`/`pinno`/`decision_time` của `anti_chattering_info[3]` mang hình dạng `const` ngay từ lúc khởi tạo dù bản thân struct nói chung là mutable (`status`/`prev_level` thay đổi; phần còn lại không bao giờ thay đổi sau initializer tĩnh ở `km_it.c:20-54`).
- Các jump table `func[]` bên trong `mc_p_on_proc()`/`sleep_status_rem_proc()` (`04_call_graph.md` §3.5) — các mảng function-pointer `static const`, không phải dữ liệu theo nghĩa mutable-state, nhưng đáng nhớ là chúng tồn tại khi grep tìm "ai đọc `model`."

---

## 4. Trace kinh điển: Hardware → Application

### 4.1 Register I2C1 → `i2c_info.rx_buf` → lệnh application (chiều thuận, cấp byte)
```
I2C1 peripheral shifts in a byte, sets RXNE flag       (hardware)
  → I2C1_IRQHandler                                     main/Src/stm32f0xx_it.c:203
  → HAL_I2C_EV_IRQHandler(&hi2c1)                        (HAL, dispatches by state)
  → I2C_Slave_ISR_IT(hi2c, ...)                          stm32f0xx_hal_i2c.c:3380
      (*hi2c->pBuffPtr++) = hi2c->Instance->RXDR;        stm32f0xx_hal_i2c.c:3432  ← THE hardware-register-to-buffer copy
  → [after enough bytes / on reload/complete] weak HAL_I2C_AddrCallback / SlaveRxCpltCallback
      I2C_stm32f0xx.c:767 / :747
  → do_callback() → rx_comp(event)                       km_i2c.c:171              ← ISR, sets write_comp=1, does NOT re-copy the buffer
  ⋯ (ISR returns; buffer now sits untouched until next super-loop pass) ⋯
  → i2c_recv_wait()                                      km_i2c.c:485              ← main-loop, reads i2c_info.rx_buf[]/second_addr
  → io_extend_write(addr, data)                          km_extend_io.c:249        ← application-level decode
  → BSP_GPIO_WritePin(port, pin, ...)                     BSP_GPIO_STM32F03x_Nucleo.c:148
  → GPIOx->BSRR write                                     (hardware register, HAL_GPIO_WritePin body)
```
**Ý nghĩa:** việc điền buffer (register phần cứng → RAM) hoàn toàn là công
việc của vendor HAL, chạy trong interrupt context, và đã hoàn tất *trước
khi* `rx_comp()` — callback nhìn thấy ở tầng application — chạy. `rx_comp()`
chỉ quan sát rằng việc điền đã xảy ra; nó không phải là cơ chế khiến điều đó
xảy ra. Sự phân biệt này không thể nhìn thấy được nếu chỉ đọc `km_i2c.c`
(Phase 4) — nó chỉ có thể được chứng minh bằng cách đọc chính vendor driver.

### 4.2 Application → Register I2C1 (chiều ngược, phản hồi đọc register)
```
i2c_recv_wait() decodes a register-read request                    km_i2c.c:511-626
  → tx_data = <live application value>                              km_i2c.c: various (e.g. :574)
  → change_state(..., Driver_I2C0.SlaveTransmit, &tx_data, ...)      km_i2c.c:274
      → (*transfer)(buf, size) = Driver_I2C0.SlaveTransmit(...)      indirect call (04_call_graph.md §3.4/§3.6)
      → I2Cx_SlaveTransmit() → HAL_I2C_Slave_Transmit_IT(&hi2c1,...) (arms the transfer; does not block)
  ⋯ (function returns; main loop continues) ⋯
⚡ I2C1 TXIS flag sets (master clocks a byte out)
  → I2C1_IRQHandler → HAL_I2C_EV_IRQHandler → I2C_Slave_ISR_IT
      hi2c->Instance->TXDR = (*hi2c->pBuffPtr++);                    stm32f0xx_hal_i2c.c:3457
                                                                       → I2C1 peripheral shifts the byte out (hardware)
```
**Ý nghĩa:** cùng một hàm ISR của vendor (`I2C_Slave_ISR_IT`) là nơi duy
nhất mà ranh giới hardware/RAM cấp byte thực sự bị vượt qua ở cả hai chiều
— `km_i2c.c` chỉ bao giờ arm một transfer rồi đọc *kết quả* sau đó qua một
completion flag, không bao giờ tự mình chạm vào `I2C1->RXDR`/`TXDR`.

### 4.3 Register EXTI → hành động main-loop (một trace thuận thứ hai, không phải I2C)
```
GPIOx pin edge sets the corresponding EXTI pending bit    (hardware)
  → EXTIx_y_IRQHandler                                    main/Src/stm32f0xx_it.c:145-184
  → HAL_GPIO_EXTI_IRQHandler(pin)                          (HAL clears the EXTI pending register bit — hardware write)
  → HAL_GPIO_EXTI_Callback(pin) → signal_event(...)        BSP_GPIO_STM32F03x_Nucleo.c:70
  → km_exti_callback(pin, event)                           km_it.c:247               ← writes anti_chattering_info[] or pending_factor (§2.3/§2.4)
  ⋯ next super-loop pass ⋯
  → anti_chattering_proc() / intr_pending_proc()           km_extend_io.c:1416,1438  ← main-loop, reads the state the ISR wrote
```

---

## 5. Các vấn đề race condition, staleness và quyền sở hữu (ownership) đã tìm thấy

1. **Staleness một-lần-lặp của `_status` (CA72 status)** — `sleep_status_rem_*_proc()` đọc `get_ca72_status()` trước khi `ap_power_en_proc()` của lần lặp đó có cơ hội ghi nó (§2.4; được xác định lần đầu trong `04_call_graph.md` §2.2, được nhắc lại ở đây như sự thật ownership dữ liệu bên dưới: reader và writer là hai hàm main-loop khác nhau với thứ tự gọi cố định nhưng không được thực thi bởi bất cứ điều gì).
2. **Các field của `i2c_info` được đọc bên ngoài mọi critical section trong `i2c_recv_wait()`** — `change_state()`/`i2c_check_error()` bọc lần truy cập của chính chúng bằng `HAL_NVIC_DisableIRQ(I2C1_IRQn)`, nhưng phần lớn `switch(i2c_info.state)`/các lần đọc buffer của chính `i2c_recv_wait()` (`km_i2c.c:503-758`) chạy trong khi I2C1 hoàn toàn được bật. Trong thực tế điều này an toàn chỉ vì `rx_comp()` (writer duy nhất phía ISR) không bao giờ chạm vào nội dung `state`/`rx_buf` sau completion flag mà nó vừa set — nhưng không có gì trong hệ thống kiểu hay một comment thực thi bất biến đó; đây là một hợp đồng ngầm giữa `rx_comp()` và `i2c_recv_wait()`.
3. **`g_km_timer[]` và `anti_chattering_info[]` được ghi từ cả ISR lẫn main-loop mà không có lock dùng chung** (§2.3) — `[INFERRED]` an toàn trong thực tế vì ISR chỉ bao giờ decrement/hết hạn các timer trong khi main loop chỉ arm/đọc chúng cho các "loại" timer *khác nhau* tại bất kỳ thời điểm nào, nhưng đây là một đặc tính của cách code hiện đang được tổ chức, không phải một bảo đảm được thực thi.
4. **`g_dest_addr` (IAP) không có guard reset ở cấp giao thức** (§2.7) — một chuỗi lệnh I2C sai thứ tự từ master (ví dụ, các lần ghi `IAP_PG_COMMAND_DATA` mà không có một lần erase thành công đứng trước) sẽ dùng bất kỳ giá trị nào mà `g_dest_addr` được để lại lần cuối, mà kiểm tra phạm vi của `flash_write_32()` (`km_i2c_iap.c:415-416`) có khả năng sẽ từ chối nhưng chỉ *sau khi* cursor đã âm thầm bị tiến lên bởi một chuỗi cục bộ trước đó.
5. **Case `KM_ARM_I2C_GENERATE_NACK` của `I2Cx_Control()` không có `break` trước khi rơi (fall) vào `KM_ARM_I2C_GENERATE_ACK`** (`I2C_stm32f0xx.c:597-600`): việc phát NACK cũng vô điều kiện thực thi luôn lệnh ghi register `CR2 |= (1<<NBYTES_Pos)` của case ACK ngay sau đó. `[INFERRED]` điều này có thể là cố ý (cả hai đường đều cần `NBYTES` được re-arm cho byte tiếp theo bất kể kết quả ACK/NACK), nhưng đây là một fallthrough không có comment đánh dấu là cố ý, và nó có nghĩa là câu hỏi "ai ghi `I2C1->CR2`" có một câu trả lời không hiển nhiên cho đường NACK — đáng để xem xét lại trước khi dựa vào việc hai case này hoạt động độc lập với nhau.
6. **`tx_data` là một scratch dùng chung duy nhất được tái sử dụng cho mọi phản hồi đọc register I2C** (§2.1) — tính đúng đắn hiện tại phụ thuộc hoàn toàn vào thứ tự gọi của super-loop (không bao giờ phát hành lần đọc thứ hai trước khi lần đầu hoàn tất), không phải vào bất kỳ sự cô lập theo từng giao dịch nào.
7. **Handoff của `is_usb2_oe_from_main_cpu_ctrl` không có timeout hoặc đường giải phóng tường minh nào được tìm thấy trong phase này** — một khi I2C master set nó qua một lần ghi `_USB2_OE` (`km_extend_io.c:298,306`), việc điều khiển tự động của `usb2_oe_proc()` vẫn bị chặn (`km_extend_io.c:1375-1376`) cho đến khi master ghi mức ngược lại; `[UNKNOWN]` liệu có bất kỳ đường code nào khác clear cờ này hay không (không tìm thấy trong các file đã đọc ở phase này) — một master ngừng giao tiếp sau khi assert bit này sẽ để lại pin dưới một chế độ điều khiển mà main loop không còn tự động quản lý nữa.
