 # PS-CPU (pscpu_s800) — 08. Phân Tích Concurrency

**Phạm vi tài liệu này:** mọi truy cập tài nguyên dùng chung giữa các
context thực thi trong firmware này, mọi primitive đồng bộ hoá thực sự
được sử dụng (và, quan trọng không kém, những cái đã được tìm kiếm và
xác nhận là **vắng mặt**), cùng một danh sách xếp hạng các rủi ro
concurrency cụ thể. Tài liệu đồng hành với `03_execution_model.md`
(context/priority), `05_data_flow.md` (owner/writer/reader của từng
đối tượng), và `07_state_machines.md` (chuyển tiếp của từng máy). Không
có gì ở đây lặp lại các suy diễn của những tài liệu đó mà không bổ sung
góc nhìn đặc thù cho concurrency (ghép cặp context, cơ chế đồng bộ hoá,
cửa sổ race cụ thể). Các nhãn bằng chứng tuân theo quy ước
`SESSION_CONTEXT.md`.

---

## 1. Khung phân tích được điều chỉnh — "concurrency" nghĩa là gì khi không có RTOS

Không có task nào, nên "ISR ↔ task" thu gọn thành **ISR ↔ main-loop**
cho mọi cặp tài nguyên dùng chung trong firmware này (đã xác nhận triệt
để — `03_execution_model.md` §8). Không có queue, semaphore, mutex, hay
event group nào ở bất cứ đâu trong source (đã xác nhận bằng grep trên
`main/` và `iap/`: không có kết quả khớp nào cho
`xQueue`/`xSemaphore`/`osMutex`/bất kỳ symbol RTOS nào). Primitive đồng
bộ hoá duy nhất tồn tại là **bật/tắt NVIC của một đường ngắt cụ thể**,
được dùng theo ba cách khác nhau (§3). Vì Cortex-M0 chỉ cho 4 mức
priority và mọi ngắt firmware này sử dụng đều được lập trình ở cùng một
mức, 0 (`03_execution_model.md` §2.6), **preemption ISR-vs-ISR không
thể xảy ra chút nào** — một ISR luôn chạy đến khi hoàn tất trước khi
ISR đang chờ kế tiếp bắt đầu. Điều này loại bỏ hoàn toàn một hạng mục
rủi ro (ISR nesting/reentrancy giữa hai ISR *khác nhau*) mà lẽ ra cần
phân tích riêng; mọi cửa sổ race thực sự trong firmware này đều nằm
giữa đúng một ISR và luồng thực thi main-loop duy nhất.

---

## 2. Ma trận tài nguyên dùng chung

| Tài nguyên | (Các) Writer | (Các) Reader | Ghép cặp Context | Đồng bộ hoá | Critical section? | Rủi ro |
|---|---|---|---|---|---|---|
| `i2c_info` (struct, `km_i2c.c:33-47`) | `rx_comp()` (ISR); `change_state()` (main-loop) | `i2c_recv_wait()` (main-loop) | **ISR ↔ main-loop** | không có ở mức struct | chỉ 2 trong số ~13 trường của nó từng được truy cập dưới `HAL_NVIC_DisableIRQ(I2C1_IRQn)` (`change_state`, `i2c_check_error`) | **CAO** — xem §5.1 |
| `i2c_info.rx_buf[]` (mảng byte) | HAL vendor `I2C_Slave_ISR_IT()` (ISR, `stm32f0xx_hal_i2c.c:3432`) | `i2c_recv_wait()` (main-loop) | ISR ↔ main-loop | không có | không | **CAO** — xem §5.1 |
| `g_km_timer[]` (mảng 16 phần tử, `km_it.c:18`) | `km_tim_callback()` (ISR, giảm); ~15 vị trí gọi main-loop (arm) | ~15 vị trí gọi main-loop (`km_timer_get_state()`) | ISR ↔ main-loop | không có | không | TRUNG BÌNH — §5.3 |
| `anti_chattering_info[]` (mảng 3 phần tử, `km_it.c:20-54`) | `km_exti_callback()`/`start_anti_chattering()` (ISR); `anti_chattering_proc()` (main-loop) | `anti_chattering_proc()` + macro `IS_CHECKING_CHATTERING()` dùng trong ~10 hàm main-loop | ISR ↔ main-loop | không có | không | TRUNG BÌNH — §5.3 |
| `pending_factor` (`km_it.c:17`) | `km_exti_callback()` (ISR, qua `set_pending_factor_bit`) | `intr_pending_proc()` (main-loop) | ISR ↔ main-loop | **có** — `get/put_pending_factor()` tắt/bật cả 3 đường EXTI qua wrapper `km_disable_irq`/`km_enable_irq` có đếm tham chiếu | **có**, cái duy nhất có đếm tham chiếu trong codebase | THẤP — đây là tài nguyên duy nhất được bảo vệ đúng cách, có chủ đích |
| `stop_mode` (`km_alarm_wake.c:16`) | `set_cycle_time()` (main-loop); `km_exti_callback()` (ISR, EXTI); `HAL_RTC_AlarmAEventCallback()` (ISR, RTC) | 2 ISR đó | main-loop → **2 ISR khác nhau** (EXTI và RTC), không bao giờ được main-loop đọc | không có | không | THẤP — xem §5.4 (an toàn chỉ vì cùng priority = không có ISR/ISR chồng lấp) |
| `g_io_extend_memory[]` (`km_extend_io.c:67`) | `io_extend_write()` + vài `*_proc()` (đều main-loop) | hầu hết `*_proc()` (đều main-loop) | **chỉ main-loop** | n/a | n/a | không có xét theo góc độ concurrency (main-loop đơn luồng không thể tự race với chính nó) — xem `05_data_flow.md`/`07_state_machines.md` để biết vấn đề *thứ tự* (không phải concurrency) của nó |
| `_status` (CA72, `km_ca72_status.c:11`) | `ap_power_en_proc`/`s800_power_off` (main-loop) | `sleep_status_rem_*_proc` (main-loop) | chỉ main-loop | n/a | n/a | không có xét theo góc độ concurrency (vấn đề dữ liệu cũ đã được tài liệu hoá là một vấn đề *thứ tự*, không phải *race*) |
| `g_iap_reg[4]` (IAP, `km_i2c_iap.c:49`) | `i2c_recv_wait()` (main-loop) | `i2c_recv_wait()` (main-loop) + `msw_on_proc_iap()` (**ISR**, EXTI4_15) | main-loop ↔ ISR | không có | không | THẤP — các lần đọc 32-bit căn chỉnh là atomic trên Cortex-M0 `[HARDWARE ASSUMPTION]`, nhưng xem §5.5 |
| `ui_EventCountTimer` (`km_it.c:57`) | `km_tim_callback()` (ISR, `++` mỗi 1ms) | `setEventRecord()` (main-loop, đóng dấu thời gian các mục `st_EventRecord[]`) | ISR ↔ main-loop | không có | không | THẤP — một lần đọc bị xé (torn read) của một bộ đếm 32-bit đang được tăng là có thể xảy ra về mặt lý thuyết nhưng hậu quả nhiều nhất chỉ là một dấu thời gian lệch vài ms, không phải một lỗi chức năng |
| Handle HAL `hi2c1`/`htim3`/`hrtc` | nội bộ thư viện HAL, cả ISR và main-loop đều gọi các hàm HAL thay đổi các handle này | như trên | ISR ↔ main-loop (qua HAL, không phải code ứng dụng trực tiếp) | các macro nội bộ `__HAL_LOCK`/`__HAL_UNLOCK` của chính HAL (chưa được xác nhận trong lượt này — code vendor) | nội bộ vendor | `[UNKNOWN]` — ngoài phạm vi; được đánh dấu để không bị âm thầm giả định là an toàn |

---

## 3. Các primitive đồng bộ hoá thực sự được sử dụng — kiểm kê đầy đủ

Mọi lệnh gọi bật/tắt `NVIC`/`HAL_NVIC` trong tầng ứng dụng Main, tìm
được bằng cách grep `main/App/*.c` (7 cặp, không thừa không thiếu):

| # | Vị trí | Bảo vệ | Có bỏ qua wrapper có đếm tham chiếu không? |
|---|---|---|---|
| 1 | `km_it.c:100-102` / `:124-126` (`disable/enable_external_irq`, chỉ được gọi từ `get/put_pending_factor`) | `pending_factor` | n/a — **đây chính là** cách dùng dự kiến của wrapper có đếm tham chiếu |
| 2 | `km_it.c:436-460` (bản thân `km_disable_irq`/`km_enable_irq`) | bất kỳ `IRQn` nào được truyền vào — trên thực tế, chỉ từng được gọi với 3 đường EXTI (đã xác nhận bằng grep — không có vị trí gọi nào khác tồn tại ở bất cứ đâu trong `main/`) | n/a — đây chính là bản thân primitive |
| 3 | `km_it.c:333` / `:350` (bên trong `km_tim_callback()`, bao quanh chính thân của nó) | không có gì thực sự ý nghĩa — một ISR tự tắt nguồn ngắt của chính nó từ bên trong chính nó không thể ngăn chặn reentrancy mà sơ đồ priority phẳng đã cấm sẵn | bỏ qua (raw `HAL_NVIC_DisableIRQ`, không phải `km_disable_irq`) — nhưng vô hại về mặt chức năng |
| 4 | `km_i2c.c:282` / `:302` (bên trong `change_state()`) | `i2c_info.state`, `*comp`, `rx_buf_current` trong suốt thời gian arm một transfer | bỏ qua — raw, đặc thù I2C1, được dùng nhất quán bởi đúng một hàm này |
| 5 | `km_i2c.c:392` / `:401` (bên trong `i2c_check_error()`) | một bản snapshot cục bộ của 6 trường `i2c_info` + `Driver_I2C0.GetStatus()` | bỏ qua — raw, cùng mẫu như #4, hàm khác |
| 6 | `km_extend_io.c:538-540` (bên trong `s800_power_off()`) | 3 đường EXTI, bị tắt **vĩnh viễn** (hàm kết thúc bằng một reset, nên không có bật lại tương ứng nào tồn tại hoặc cần thiết) | bỏ qua — được đánh dấu trong `02_architecture.md`/`03_execution_model.md` là một vi phạm phân lớp vì nó tắt cùng các đường mà #1/#2 tồn tại để quản lý |
| 7 | `km_extend_io.c:627` / `:637` (bên trong `s800_power_on()`) | một cập nhật 3 dòng cho `ui_EventCountTimer`/`setEventRecord()` | bỏ qua — raw, cụ thể là `TIM3_IRQn`, không có hàm nào khác từng bao quanh cặp câu lệnh này theo cách này |

**Ý nghĩa:** trong số 7 vị trí critical-section, chỉ #1/#2 (một cơ chế
logic duy nhất) dùng primitive "đúng chuẩn," có đếm tham chiếu — và nó
chỉ bảo vệ đúng **một** tài nguyên dùng chung (`pending_factor`) trong
số ~8 tài nguyên được liệt kê ở §2. Mọi tài nguyên dùng chung khác
trong firmware đều hoặc được bảo vệ bởi một cặp
`HAL_NVIC_DisableIRQ`/`EnableIRQ` raw, tự chế tại chính vị trí gọi của
nó (#3-#7), hoặc hoàn toàn không được bảo vệ
(`i2c_info.rx_buf[]`, `g_km_timer[]`, `anti_chattering_info[]`,
`stop_mode`, `g_iap_reg[]`, `ui_EventCountTimer` — §2).

### 3.1 Những gì đã được tìm kiếm và xác nhận là vắng mặt
- `volatile` trên bất kỳ biến trạng thái dùng chung nào: **không có** —
  3 lần sử dụng `volatile` duy nhất ở bất cứ đâu trong `main/` là một
  biến đếm vòng lặp busy-wait trong `KM_RTC_Restore()`
  (`mx_init.c:312`, được dùng đúng cách để ngăn compiler loại bỏ một
  spin-wait rỗng trên một thanh ghi phần cứng — không phải một primitive
  concurrency) và hai marker phiên bản `const volatile`
  (`km_i2c.c:19-20`, chỉ dùng để ngăn linker loại bỏ một hằng số vốn
  không được đọc, không liên quan đến concurrency).
- Memory barrier (`__DMB`/`__DSB`/`__ISB`): **không có** kết quả khớp
  nào ở bất cứ đâu trong `main/`.
- Tắt ngắt toàn cục (`__disable_irq`/`PRIMASK`): **không có** kết quả
  khớp nào — mọi critical section trong firmware này chỉ tắt một đường
  ngắt ngoại vi cụ thể, không bao giờ tắt mặt nạ ngắt toàn cục.
- Bất kỳ primitive RTOS nào (queue/semaphore/mutex/event group):
  **không có** — đã được xác lập trong `03_execution_model.md` §8.

**Ý nghĩa:** sự vắng mặt của `volatile` là điều đáng lo ngại hơn trong
số này. Không có trường nào trong ~13 trường của `i2c_info`, không có
phần tử nào của `g_km_timer[]`, không có trường nào của
`anti_chattering_info[]`, và không có biến nào trong
`stop_mode`/`pending_factor`/`g_iap_reg[]` được đánh dấu `volatile`.
Trong codebase cụ thể này, rủi ro thực tế được giảm bớt vì mọi truy cập
trạng thái dùng chung đều diễn ra qua một lệnh gọi đến một hàm không tầm
thường, được biên dịch riêng (`km_timer_get_state()`,
`read_pending_factor_bit()`, `rx_comp()`, v.v.) — trình biên dịch nói
chung không thể chứng minh những lệnh gọi như vậy không có tác dụng phụ
và do đó sẽ không cache bộ nhớ bên dưới qua các lệnh gọi đó nếu không có
tối ưu hoá liên kết (link-time optimization) — nhưng sự an toàn này
mang tính **ngẫu nhiên do cách code hiện đang được cấu trúc**, không
phải một thuộc tính mà source khai báo hay đảm bảo. Một thay đổi trong
tương lai inline một trong các hàm truy cập này (một refactor thành
`static inline`, một mức tối ưu hoá quyết liệt hơn, hoặc LTO) sẽ loại bỏ
sự bảo vệ ngẫu nhiên đó mà không có bất kỳ cảnh báo nào từ trình biên
dịch, vì không có gì đánh dấu các biến này là được chạm đến từ ngữ cảnh
ngắt.

---

## 4. Deadlock và priority inversion — về cấu trúc là không áp dụng được, và lý do

- **Deadlock**: đòi hỏi ít nhất hai context, mỗi context đang chờ một
  tài nguyên mà context kia đang giữ. Firmware này không có acquire
  chặn (blocking) đối với bất cứ thứ gì — mọi "lock" ở đây là một cặp
  bật/tắt NVIC kiểu fire-and-forget không có vòng lặp chờ bên trong, và
  main loop không bao giờ block chờ một ISR (nó poll một cờ và tiếp tục
  nếu chưa set, lệnh return sớm của `i2c_recv_wait()` tại
  `km_i2c.c:499-501`). **Không thể xảy ra deadlock với hình dạng đồng bộ
  hoá này**, đã được xác nhận bằng kiểm kê primitive triệt để ở §3 thay
  vì giả định từ việc "không có RTOS."
- **Priority inversion**: đòi hỏi preemption dựa trên priority với một
  holder priority thấp hơn chặn một waiter priority cao hơn. Mọi ngắt ở
  đây đều chia sẻ một mức priority duy nhất (§1), và main loop hoàn toàn
  không có khái niệm về priority — **tiền đề cho priority inversion
  không tồn tại trên tổ hợp phần cứng/firmware này.**

---

## 5. Các rủi ro cụ thể, xếp hạng

### 5.1 CAO — mất âm thầm một giao dịch I2C dưới hoạt động dồn dập của master
`i2c_info.write_comp`/`read_comp` là **boolean, không phải bộ đếm**, và
`i2c_info.rx_buf[]` là **một buffer đơn, không phải một queue**
(`05_data_flow.md` §2.1). Nếu master I2C hoàn tất một giao dịch đầy đủ
thứ hai (address+data+STOP) trước lượt super-loop *kế tiếp* của
`i2c_recv_wait()` rút hết nội dung `write_comp`/`rx_buf` của giao dịch
đầu tiên, các lệnh ghi phía ISR của giao dịch thứ hai (`rx_comp()`, và
việc điền `rx_buf[]` từng byte của HAL vendor,
`stm32f0xx_hal_i2c.c:3432`) sẽ âm thầm ghi đè dữ liệu của giao dịch đầu
tiên và set lại cùng cờ boolean đó — `i2c_recv_wait()` không có cách nào
biết hai giao dịch đã xảy ra; nó sẽ chỉ xử lý nội dung của giao dịch thứ
hai một lần, và lệnh đầu tiên bị mất mà không có lỗi, không có NACK,
không có bản ghi log. `[INFERRED]` — điều này đòi hỏi master I2C phải
phát hành hai giao dịch đầy đủ trong khoảng thời gian ngân sách của một
lượt super-loop, mà với ~20 giai đoạn đọc GPIO đơn giản của vòng lặp
(`03_execution_model.md` §2.3) thì có khả năng nằm trong khoảng vài
chục micro giây, hoàn toàn nằm trong tầm với của ngay cả một master I2C
tốc độ vừa phải phát hành các lệnh dồn dập; điều này chưa được đo hoặc
benchmark trong lượt này, nên mức độ nghiêm trọng được nêu là một phát
hiện về cấu trúc, không phải một lỗi đã được chứng minh-quan-sát-thấy.

### 5.2 TRUNG BÌNH — lệnh đọc trạng thái hàng loạt của `i2c_recv_wait()` nằm ngoài mọi critical section
Chỉ `change_state()` và `i2c_check_error()` bao quanh truy cập
`i2c_info` của chúng bằng `HAL_NVIC_DisableIRQ(I2C1_IRQn)`. Phần thân
lớn hơn nhiều của `i2c_recv_wait()` — dispatch `switch(i2c_info.state)`
và tất cả các lần đọc `rx_buf`/`second_addr` bên trong nó
(`km_i2c.c:503-758`) — chạy với I2C1 hoàn toàn được bật. Sự an toàn hiện
tại phụ thuộc hoàn toàn vào hợp đồng ngầm định rằng `rx_comp()` không
bao giờ thay đổi nội dung `state`/`rx_buf` sau khi set cờ hoàn tất mà nó
vừa set — đúng ngày hôm nay qua việc kiểm tra code, nhưng không được
thực thi bởi bất kỳ lock, comment, hay chú thích kiểu nào.

### 5.3 TRUNG BÌNH — `g_km_timer[]`/`anti_chattering_info[]` được ghi từ cả hai context mà không có lock
`km_tim_callback()` giảm mọi slot `g_km_timer[]` đang được arm mỗi 1ms
mà không có bao quanh chống lại việc một lệnh gọi `km_timer_set()` từ
main-loop rơi vào đúng slot đó giữa lúc đang giảm
(`km_it.c:340-348` so với ~15 vị trí gọi main-loop). `[INFERRED]` an
toàn trên thực tế chỉ vì mỗi "loại" timer về mặt logic thuộc sở hữu của
một feature tại một thời điểm (ISR chỉ bao giờ giảm/hết hạn, không bao
giờ arm lên), không phải vì có bất kỳ sự loại trừ lẫn nhau được thực thi
nào. Tình huống cấu trúc tương tự đối với `anti_chattering_info[]` giữa
`start_anti_chattering()` (ISR) và `anti_chattering_proc()` (main-loop).

### 5.4 THẤP — `stop_mode` được ghi từ hai ISR khác nhau mà không có lock
Chỉ an toàn vì ngắt EXTI và RTC chia sẻ cùng một mức priority NVIC và do
đó không thể preempt lẫn nhau (§1) — nếu một thay đổi trong tương lai
gán cho một trong hai ngắt đó một priority khác, cờ dùng chung bởi 2 ISR
cụ thể này sẽ trở thành một race thực sự không có bảo vệ nào cả. Được
ghi nhận là THẤP ngày hôm nay, nhưng đáng được nêu như một trường hợp
"chỉ an toàn vì một bất biến ở nơi khác," chính xác là kiểu điều âm thầm
hỏng khi ai đó điều chỉnh priority NVIC vì một lý do không liên quan sau
này.

### 5.5 THẤP — đọc `g_iap_reg[]` xuyên context mà không có lock (image IAP)
`msw_on_proc_iap()` (ISR) đọc `g_iap_reg[CHANGE_INFO]`
(`km_extend_io_iap.c:35`) trong khi `i2c_recv_wait()` (main-loop) là
writer duy nhất. Một lệnh load/store 32-bit căn chỉnh là atomic trên
Cortex-M0 theo đảm bảo của ISA, nên một lần đọc bị xé (torn read) là
không thể — nhưng đây là một thuộc tính ISA đang được dựa vào một cách
ngầm định, không phải một bất biến đã được tài liệu hoá, và nó sẽ không
tổng quát hoá cho một lần đọc struct/nhiều từ.

### 5.6 THÔNG TIN — fallthrough NACK/ACK của `I2Cx_Control()` (`05_data_flow.md` §5, mục 5) liên quan đến concurrency
Vì `Driver_I2C0.Control()` được gọi từ `change_state()` (main-loop, bên
trong cùng critical section cũng arm transfer kế tiếp, §3 mục 4), việc
thiếu `break` giữa `KM_ARM_I2C_GENERATE_NACK` và `_ACK`
(`I2C_stm32f0xx.c:597-600`) thực thi thêm một lệnh ghi thanh ghi `CR2`
từ bên trong cùng critical section đó ở mỗi lần NACK — bản thân không
phải một race, nhưng nó có nghĩa là tác dụng thực sự ở mức thanh ghi của
critical section hơi khác so với những gì một người đọc `change_state()`
riêng lẻ sẽ mong đợi, điều này quan trọng khi suy luận về trạng thái
`CR2` được để lại khi section kết thúc.

---

## 6. Bảng tổng hợp rủi ro

| Rủi ro | Phân loại (theo brief của phase) | Mức độ nghiêm trọng | Tài nguyên | Đã xác nhận / Suy diễn |
|---|---|---|---|---|
| Mất âm thầm một giao dịch I2C đầy đủ dưới lưu lượng dồn dập của master | mất sự kiện, truy cập kép | **CAO** | `i2c_info`/`rx_buf[]` | `[INFERRED]` — cấu trúc, chưa đo lường |
| Lệnh đọc `i2c_info` hàng loạt trong `i2c_recv_wait()` nằm ngoài mọi lock | khoảng hở đồng bộ ISR/main-loop | TRUNG BÌNH | `i2c_info` | Đã xác nhận (không có bao quanh nào) |
| Ghi xuyên context không lock của `g_km_timer[]`/`anti_chattering_info[]` | race condition (vô hại hiện tại) | TRUNG BÌNH | mảng timer/debounce | Đã xác nhận cấu trúc; `[INFERRED]` vô hại |
| `stop_mode` được ghi bởi 2 ISR, an toàn chỉ nhờ non-preemption cùng priority | race condition (tiềm ẩn) | THẤP | `stop_mode` | Đã xác nhận cấu trúc; an toàn là một bất biến bên ngoài |
| Đọc `g_iap_reg[]` bởi ISR / ghi bởi main-loop | khoảng hở đồng bộ ISR/main-loop | THẤP | `g_iap_reg[]` | Đã xác nhận; an toàn nhờ tính atomic của ISA `[HARDWARE ASSUMPTION]` |
| Không có `volatile` trên bất kỳ trạng thái dùng chung xuyên context nào | rủi ro hệ thống / công cụ | TRUNG BÌNH (tiềm ẩn) | tất cả những mục trên | Đã xác nhận bằng grep triệt để |
| Fallthrough NACK/ACK của I2Cx_Control | tính đúng đắn, liên quan concurrency | THẤP | Thanh ghi `CR2` của I2C1 | Đã xác nhận (`05_data_flow.md` §5.5) |
| Deadlock | — | **không áp dụng** | — | Đã xác nhận vắng mặt (không có primitive chặn nào tồn tại) |
| Priority inversion | — | **không áp dụng** | — | Đã xác nhận vắng mặt (sơ đồ priority phẳng) |
| Race ISR-vs-ISR | — | **không áp dụng** | — | Đã xác nhận vắng mặt (không có preemption giữa các IRQ cùng priority) |
