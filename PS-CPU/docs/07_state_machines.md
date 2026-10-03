 # PS-CPU (pscpu_s800) — 07. Máy Trạng Thái (State Machines)

**Phạm vi tài liệu này:** mọi máy trạng thái tường minh (một enum có tên
+ một biến được khai báo kiểu đó) và ngầm định (hành vi rõ ràng có
*hình dạng* của máy trạng thái — các chuyển tiếp có điều kiện được điều
khiển bởi sự kiện — nhưng không có enum nào hậu thuẫn) trong firmware.
Không có trạng thái nào bị bịa ra: mọi trạng thái/chuyển tiếp bên dưới
là một giá trị enumerator có thật hoặc một nhánh `if`/`switch` có thật
đã được đọc trong Phase 3-6. Ở những chỗ một nhóm điều kiện là nhãn *của
tôi* chứ không phải một cái tên có trong source, nó được đánh dấu
`[INFERRED]` và điều kiện gốc được trích dẫn nguyên văn để có thể kiểm
tra lại. Tài liệu đồng hành với `04_call_graph.md` (hàm nào gọi hàm nào)
và `05_data_flow.md` (biến này là gì). Các bản vẽ Mermaid
`stateDiagram-v2` cho các máy này là một sản phẩm bàn giao sau
(`docs/diagrams/06_state_machine/`) — tài liệu này là bản phân tích bám
sát source mà các sơ đồ đó phải được xây dựng dựa trên.

---

## 1. Các máy trạng thái tường minh

### 1.1 Trạng thái giao thức I2C slave — `i2c_info.state` (Main và IAP, hai instance riêng biệt)

- Kiểu: `enum` vô danh `{ TYPE_State_Init, TYPE_State_Wait_Write, TYPE_State_Wait_Read }` — `main/App/km_i2c.c:49-53` (IAP có enum riêng giống hệt về mặt văn bản trong `km_i2c_iap.c`, một symbol biên dịch khác, không chia sẻ trạng thái — `05_data_flow.md` §2.1).
- Biến: `i2c_info.state`, `km_i2c.c:34`.
- Ghi chú: **không có trạng thái `Error` tường minh nào.** Việc khôi
  phục lỗi (`03_execution_model.md`/`06_event_flow.md` E26/E27) không
  chuyển `state` sang một giá trị lỗi có tên — nó gọi `i2c_sw_reset()`,
  hàm này gọi `i2c_recv_first()`, hàm này set `state = TYPE_State_Wait_Write`
  trực tiếp (`km_i2c.c:322`), tức là việc khôi phục là một **reset cưỡng
  bức về trạng thái ban đầu**, không phải một chuyển tiếp qua một trạng
  thái lỗi đã được mô hình hoá.

| Trạng thái | Trigger | Điều kiện | Action | Trạng thái kế tiếp | Hàm | File:line |
|---|---|---|---|---|---|---|
| `Wait_Write` (ban đầu) | boot / `i2c_sw_reset()` | — | `Driver_I2C0.SlaveReceive()` được arm | `Wait_Write` | `i2c_recv_first()` | `km_i2c.c:317-343` |
| `Wait_Write` | hoàn tất ghi I2C, `xfer_size==1` (một *yêu cầu* đọc) | `second_addr` được nhận diện | dàn dữ liệu phản hồi, arm `SlaveTransmit` | `Wait_Read` | `i2c_recv_wait()` | `km_i2c.c:505-627` |
| `Wait_Write` | hoàn tất ghi I2C, `xfer_size>1` (một lệnh ghi thực sự) | `second_addr` được nhận diện | thay đổi thanh ghi/GPIO/timer mục tiêu | `Wait_Write` (được arm lại cho lệnh kế tiếp) | `i2c_recv_wait()` | `km_i2c.c:628-737` |
| `Wait_Write` | hoàn tất ghi I2C, `xfer_size>1`, `second_addr==IAP_PG_COMMAND` khớp magic | khớp magic number | `DeInit()`+`jump2iap()` | **kết thúc — image thoát** | `i2c_recv_wait()` | `km_i2c.c:686-693` |
| `Wait_Write` | hoàn tất đọc đến thay vì hoàn tất ghi | sai lệch giao thức | `i2c_sw_reset()` | ép về lại `Wait_Write` | `i2c_recv_wait()` | `km_i2c.c:739-742` |
| `Wait_Read` | hoàn tất đọc I2C (master clock dữ liệu phản hồi ra) | — | arm lại `SlaveReceive` | `Wait_Write` | `i2c_recv_wait()` | `km_i2c.c:750` |
| `Wait_Read` | hoàn tất ghi đến thay vì hoàn tất đọc | sai lệch giao thức | `i2c_sw_reset()` | ép về lại `Wait_Write` | `i2c_recv_wait()` | `km_i2c.c:746-748` |
| *(bất kỳ)* | `i2c_check_error()` trả về true (timeout/lỗi đã latch) | các ngưỡng busy/timeout/lỗi, `km_i2c.c:379-461` | `i2c_sw_reset()` | ép về `Wait_Write` | `i2c_recv_wait()` | `km_i2c.c:493-496` |
| `TYPE_State_Init` | không bao giờ đạt tới trong luồng bình thường | — | — | — | — | được khai báo (`:49`) nhưng **không có đường code nào từng set `state` thành giá trị này** — `[INFERRED]` một enumerator thừa (vestigial), có lẽ dự định làm sentinel trước-boot mà quá trình boot luôn bỏ qua vì `i2c_recv_first()` set `Wait_Write` trực tiếp |

**Action lúc vào/ra:** đi vào `Wait_Write` luôn arm lại
`Driver_I2C0.SlaveReceive`; đi vào `Wait_Read` luôn arm
`Driver_I2C0.SlaveTransmit` — cả hai đều qua `change_state()`
(`km_i2c.c:274-304`), điểm nghẽn duy nhất cho mọi chuyển tiếp ở trên.
**Timeout:** ba ngưỡng của `i2c_check_error()` (2000ms busy-tx, 1000ms
rx, 1000ms tx, `km_i2c.c:381-383`).
**Retry:** không có — không có bộ đếm retry nào ở bất cứ đâu trong máy
này; một lỗi được phát hiện đi thẳng đến việc reset toàn bộ ngoại vi
I2C, mỗi lần, không có backoff hay ngân sách retry nào.
**Khôi phục:** `i2c_sw_reset()` (`km_i2c.c:356-364`).

Bản sao của image IAP (`km_i2c_iap.c`) có hình dạng 3-trạng-thái giống
hệt, thuộc tính thiếu-chuyển-tiếp-`Init` giống hệt, và thuộc tính
không-có-trạng-thái-lỗi-tường-minh giống hệt, nhưng có thêm hai đích
chuyển tiếp vì các lệnh lập trình flash (`04_call_graph.md` §2.10) —
những cái đó không thay đổi *hình dạng* của máy trạng thái, chỉ thay đổi
những gì xảy ra bên trong hành động hoàn tất ghi của `Wait_Write`.

### 1.2 Trạng thái anti-chattering (debounce) — `AntiChatteringStatus` (3 instance)

- Kiểu: `typedef enum { AC_STOP_STATE = 0, AC_START_STATE } AntiChatteringStatus` — `main/Inc/km_it.h:68-72`.
- Biến: `anti_chattering_info[i].status`, mỗi instance cho
  `TYPE_Km_AC_Idx_MSW`, `_POWER_MONI`, `_24V11_MONI` (`km_it.c:20-54`).

| Trạng thái | Trigger | Điều kiện | Action | Trạng thái kế tiếp | Hàm | File:line |
|---|---|---|---|---|---|---|
| `AC_STOP_STATE` (ban đầu) | cạnh EXTI trên chân tương ứng | — | `start_anti_chattering()`: latch `start_level`/`prev_level`, arm slot `g_km_timer[]` | `AC_START_STATE` | `km_exti_callback()` → `start_anti_chattering()` | `km_it.c:261,265,274` → `:489-501` (**ngữ cảnh ISR**) |
| `AC_START_STATE` | main-loop poll thấy `prev_level != level` (tín hiệu thay đổi nữa giữa lúc debounce) | — | `start_anti_chattering()` lại (arm lại timer, khởi động lại cửa sổ) | `AC_START_STATE` (tự lặp) | `anti_chattering_proc()` | `km_extend_io.c:1447-1451` |
| `AC_START_STATE` | slot `g_km_timer[]` tương ứng đạt `End` | `start_level == level` (tín hiệu giữ ổn định suốt cửa sổ) **và** `intr_proc != NULL` | kích hoạt `intr_proc(STATE_INT)` | `AC_STOP_STATE` | `anti_chattering_proc()` | `km_extend_io.c:1453-1462` |
| `AC_START_STATE` | timer đạt `End` | `start_level == level` **nhưng** `intr_proc == NULL` (mục 24V11) | không có callback nào được kích hoạt | `AC_STOP_STATE` | `anti_chattering_proc()` | `km_extend_io.c:1456-1462` |
| `AC_START_STATE` | timer đạt `End` | `start_level != level` (tín hiệu không ổn định ở mức *ban đầu* — tức là nó dội ngược lại) | timer bị xóa, không callback | `AC_STOP_STATE` | `anti_chattering_proc()` | `km_extend_io.c:1453-1457` (điều kiện `if(start_level==level...)` đơn giản là không được thỏa) |

**Action lúc vào:** việc latch `start_level = prev_level = level` chỉ
xảy ra nếu máy đang ở `AC_STOP_STATE` lúc được arm (`km_it.c:493-496`)
— một lần arm lại trong khi đang ở `AC_START_STATE` *không* reset
`start_level`, chỉ reset `prev_level`/timer.
**Timeout:** hằng số `decision_time` theo từng tín hiệu (10ms MSW, 10ms
POWER_MONITOR, 30ms 24V11 — `km_it.h:6-10`).
**Không có trạng thái retry/lỗi** — máy này chỉ luôn có hai trạng thái.

### 1.3 Trạng thái timer phần mềm — `TYPE_Km_Timer_State` (một mẫu, 16 instance)

- Kiểu: `typedef enum { TYPE_Km_Timer_Normal = 0, TYPE_Km_Timer_Start, TYPE_Km_Timer_End, TYPE_KM_Timer_Max } TYPE_Km_Timer_State` — `km_it.h:16-22`.
- Instance: `g_km_timer[TYPE_Km_Timer_Kind_Max]`, 16 "loại" khác nhau
  (`km_it.h:24-43`) đều dùng chung hình dạng 3-trạng-thái này.

| Trạng thái | Trigger | Action | Trạng thái kế tiếp | Hàm | File:line |
|---|---|---|---|---|---|
| `Normal` (idle/đã tiêu thụ) | gọi `km_timer_set(kind, val, Start)` | `state=Start`, `ms_time=val` | `Start` | bất kỳ nào trong ~15 vị trí gọi trên `km_extend_io.c`/`km_alarm_wake.c` | ví dụ `km_extend_io.c:1287-1290` |
| `Start` | mỗi tick TIM3 1ms | `ms_time -= 1` | `Start` (tự lặp) cho đến khi `ms_time==0` | `km_tim_callback()` (**ISR**) | `km_it.c:340-348` |
| `Start` | `ms_time` đạt 0 | `state=End` | `End` | `km_tim_callback()` (**ISR**) | `km_it.c:344-346` |
| `End` | code main-loop gọi `km_timer_get_state()` và thấy `End` | action riêng của consumer (§2 chỗ khác) sau đó thường `km_timer_set(kind, 0, Normal)` để xóa | `Normal` | hàm consumer (khác nhau theo loại timer) | ví dụ `km_extend_io.c:1315,1454` |
| `End` | consumer không bao giờ xóa tường minh | — | **giữ nguyên `End` vô thời hạn** | — | — |

**Ghi chú về hàng cuối:** không phải mọi consumer đều xóa một timer đã
hết hạn về lại `Normal` — ví dụ các nhánh `PWROFF_MAXTIME2`/`PWROFF_WDGTIME`
của `poweroff_timer_proc()` (`km_extend_io.c:1319-1330`) đọc `End` và
hành động dựa trên nó nhưng đoạn code duy nhất reset hai loại cụ thể đó
về lại `Normal` là `poweroff_timer_init()` (được gọi từ
`s800_power_off()`/boot của `main()`, `km_extend_io.c:1258-1268`) — vậy
nên giữa "timer hết hạn" và "chu kỳ power-on/off kế tiếp khởi tạo lại
nó," trạng thái của loại đó thực sự nằm ở `End` và mỗi lần lặp,
`poweroff_timer_proc()` phát hiện lại cùng một sự hết hạn đó và chạy lại
`poweroff_factor_save()`+`s800_power_off()` mỗi lượt. `[INFERRED]` vô
hại vì `s800_power_off()` là kết thúc (kết thúc bằng
`HAL_NVIC_SystemReset()`), nên điều này trên thực tế chỉ xảy ra một lần
— nhưng đây là một thuộc tính thực sự "trạng thái không tự xóa" chỉ có
thể thấy được bằng cách kiểm tra mọi consumer, không thể giả định từ
riêng bản thân enum.

### 1.4 Trạng thái CA72 (application processor) — `TYPE_Km_CA72_Status`

- Kiểu: `typedef enum { CA72_OFF, CA72_ON, CA72_SLEEP } TYPE_Km_CA72_Status` — `main/Inc/km_ca72_status.h:4-8`.
- Biến: `static TYPE_Km_CA72_Status _status`, `km_ca72_status.c:11`.
- **Không phải một máy trạng thái được bảo vệ ở tầng lưu trữ**:
  `set_ca72_status()` (`:22-26`) ghi đè `_status` vô điều kiện — bất kỳ
  caller nào cũng có thể set bất kỳ giá trị nào từ bất kỳ giá trị hiện
  tại nào; "tính hợp lệ" của một chuyển tiếp chỉ tồn tại trong điều kiện
  riêng của caller, không nằm trong hàm setter.

| Trạng thái | Trigger | Điều kiện | Action | Trạng thái kế tiếp | Hàm | File:line |
|---|---|---|---|---|---|---|
| `CA72_OFF` (ban đầu) | khởi tạo static | — | — | `CA72_OFF` | — | `km_ca72_status.c:11` |
| bất kỳ | gọi `ap_power_en_proc(STATE_INT)` | (qua `intr_pending_proc()`, một cạnh EXTI AP_PWR_EN đang chờ) | `set_ca72_status(CA72_ON)` | `CA72_ON` | `ap_power_en_proc()` | `km_extend_io.c:661-665` |
| bất kỳ | gọi `ap_power_en_proc(STATE_NORMAL)` (mỗi lượt main-loop) | `BSP_GPIO_ReadPin(AP_PWR_EN)==RESET` | `set_ca72_status(CA72_SLEEP)` | `CA72_SLEEP` | `ap_power_en_proc()` | `km_extend_io.c:667-671` |
| bất kỳ | gọi `s800_power_off()` (4 đường trigger, `04_call_graph.md`/`06_event_flow.md` E23) | — | `set_ca72_status(CA72_OFF)` | `CA72_OFF` | `s800_power_off()` | `km_extend_io.c:528` |

**Reader/consumer:** `sleep_status_rem_eagle_proc()`/`_sparrow_proc()`
so sánh `cur_status` với một `static prev_status` mà chúng tự giữ
(`km_extend_io.c:892-893,968-969`) để phát hiện các cạnh `ON→SLEEP` và
`SLEEP→ON` — tức là **trạng thái phát hiện cạnh (`prev_status`) là một
mẩu bộ nhớ máy trạng thái thứ hai, độc lập, được xếp chồng lên
`_status`**, riêng của hai hàm đó, không được chia sẻ với
`km_ca72_status.c`. Đây đúng là tình huống dữ liệu-cũ-một-vòng-lặp đã
được nêu trong `05_data_flow.md` §5.1: cạnh mà các hàm này phát hiện
luôn cũ hơn ít nhất một vòng lặp so với lúc `ap_power_en_proc()` thực sự
thay đổi `_status`.
**Không có trạng thái lỗi/timeout.**

### 1.5 Cờ chế độ STOP — `stop_mode`

- Kiểu: `uint8_t` thuần, các giá trị `STOP_MODE_OFF`/`STOP_MODE_ON`
  (`km_alarm_wake.h`, giá trị được dùng tại `km_alarm_wake.c:16`) — một
  máy 2-trạng-thái không có kiểu enum riêng (dùng hằng số `#define`,
  không phải `typedef enum`), được liệt vào mục tường minh vì hai hằng
  số được đặt tên làm cho chủ đích không mơ hồ dù kiểu lưu trữ là
  `uint8_t` trần.

| Trạng thái | Trigger | Điều kiện | Action | Trạng thái kế tiếp | Hàm | File:line |
|---|---|---|---|---|---|---|
| `STOP_MODE_OFF` (ban đầu) | `msw_on_proc()` quyết định vào chế độ STOP | MSW off, SB_PG off, `MSWOFF_STOP_MODE` đang hoạt động (đã xác nhận là `#define`, `km_extend_io.c:1126`), timer reboot chưa chạy | `set_cycle_time(2)` set `stop_mode=ON`, sau đó `BSP_PWR_enter_stopmode()` | `STOP_MODE_ON` | `msw_on_proc()`→`set_cycle_time()` | `km_extend_io.c:1197-1205` → `km_alarm_wake.c:108` |
| `STOP_MODE_ON` | báo thức RTC kích hoạt (E25) | — | xóa cờ báo thức trực tiếp, `stop_mode=OFF` | `STOP_MODE_OFF` | `HAL_RTC_AlarmAEventCallback()` (**ISR**) | `km_it.c:284-300` |
| `STOP_MODE_ON` | cạnh EXTI MSW_ON kích hoạt (E8) trong khi `stop_mode==ON` | — | `SystemClock_Config2()` (khóa lại PLL) + `HAL_RTC_DeactivateAlarm()`, `stop_mode=OFF` | `STOP_MODE_OFF` | `km_exti_callback()` (**ISR**) | `km_it.c:267-274` |

**Ý nghĩa:** cả hai lối thoát khỏi `STOP_MODE_ON` đều xảy ra trong
**ngữ cảnh ngắt**, một cái từ timer được arm riêng để thoát khỏi nó
(tự thức dậy) và một cái từ chính sự kiện bên ngoài mà nó tồn tại để
chờ (MSW được đảo trạng thái) — trong khi việc đi vào, ngược lại, xảy ra
trong ngữ cảnh main-loop. Đây là một máy 2-trạng-thái mà cạnh OFF→ON và
các cạnh ON→OFF sống ở các ngữ cảnh thực thi hoàn toàn khác nhau.

### 1.6 Chốt driver IWDG — `WDT_STATE` (tầng BSP)

- Kiểu: `typedef enum { WDT_STATE_INIT = 0, WDT_STATE_START } WDT_STATE` — `common/Drivers/BSP/Src/BSP_WDT_STM32F03x_Nucleo.c:17-20`.
- Biến: `static WDT_STATE WDT_State`, cùng file, khởi tạo `WDT_STATE_INIT`.

| Trạng thái | Trigger | Action | Trạng thái kế tiếp | Hàm | File:line |
|---|---|---|---|---|---|
| `WDT_STATE_INIT` (ban đầu) | gọi `BSP_WDT_Start()` (một lần, từ boot của `main()`/`entry_iap.c`) | `HAL_IWDG_Start()` | `WDT_STATE_START` | `BSP_WDT_Start()` | `:31-45` |
| `WDT_STATE_START` | gọi lại `BSP_WDT_Start()` | không có gì — trả về lỗi kiểu `HAL_BUSY`, không khởi động lại | `WDT_STATE_START` (không đổi) | `BSP_WDT_Start()` | `:34-36` |
| `WDT_STATE_INIT` | gọi `BSP_WDT_Refresh()` trước `Start()` | trả về lỗi, không gọi `HAL_IWDG_Refresh()` | `WDT_STATE_INIT` (không đổi) | `BSP_WDT_Refresh()` | `:56-58` |

**Một chốt một chiều, không phải một chu kỳ** — không có chuyển tiếp
nào quay trở lại `WDT_STATE_INIT` ở bất cứ đâu (bản thân phần cứng IWDG
không thể dừng lại một khi đã khởi động trên dòng MCU này, nên điều này
khớp với `[HARDWARE ASSUMPTION]` của phần cứng).

### 1.7 Trạng thái cập nhật firmware IAP — trạng thái tổ hợp bit (bán-tường-minh)

- Kiểu: không phải enum — một bitmask `uint32_t`,
  `g_iap_reg[IAP_REG_INDEX_CALC(IAP_PG_COMMAND_CHANGE_STATUS)]`, với 5
  bit có thể được set độc lập, định nghĩa trong
  `iap/Inc/km_i2c_iap.h:92-96`: `..._IAP` (0x1), `..._DATA_CMD_STS`
  (0x2), `..._PG_CMD_STS` (0x4), `..._CHKSUM_ERR` (0x10000),
  `..._PSCPU_ERR` (0x20000). Được phân loại "bán-tường-minh" vì các bit
  được đặt tên và có chủ đích, nhưng không có gì thực thi tính loại trừ
  lẫn nhau — đây là một tập cờ, không phải một phân hoạch thành các
  trạng thái rời rạc, nên "trạng thái" thực chất là một *tập con khả
  đạt* trong số 32 tổ hợp bit khả dĩ, được xác định hoàn toàn bởi thứ
  tự master I2C bên ngoài gửi lệnh.

| Tổ hợp khả đạt | Ý nghĩa | Trigger | Hàm | File:line |
|---|---|---|---|---|
| `0` (tất cả xóa) | idle, không có cập nhật nào đang diễn ra | boot | (khởi tạo BSS về 0) | `km_i2c_iap.c:49` |
| Bit `IAP` được set | xóa hoàn tất, phiên cập nhật được mở | ghi `IAP_PG_COMMAND` khớp magic number | `i2c_recv_wait()` | `:315-322` |
| `IAP \| DATA_CMD_STS` | ít nhất một từ dữ liệu 4-byte đã được ghi (thành công hay không — bit này được set vô điều kiện sau khi thử ghi, `:370`) | (các) lần ghi `IAP_PG_COMMAND_DATA` | `i2c_recv_wait()` | `:353-374` |
| `... \| PSCPU_ERR` | ngoài ra, kiểm tra phạm vi địa chỉ của lần ghi dữ liệu cuối thất bại (`flash_write_32()` trả về `ARM_DRIVER_ERROR`) | `g_dest_addr` ngoài phạm vi | `i2c_recv_wait()` | `:371-374` |
| `... \| PG_CMD_STS` | master báo hiệu "truyền dữ liệu hoàn tất" (bit `COMP` của `IAP_PG_COMMAND_CHANGE_INFO`) và một checksum đã được tính | ghi `IAP_PG_COMMAND_CHANGE_INFO` với bit hoàn tất | `i2c_recv_wait()` | `:330-347` |
| `... \| CHKSUM_ERR` | ngoài ra, checksum tính toán không khớp với giá trị master cung cấp | sai lệch `check_sum_calc()` | `i2c_recv_wait()` | `:336-340` |

**Không có chuyển tiếp nào quay về một tổ hợp thấp hơn được lập trình
ở bất cứ đâu** trong các file được đọc trong lượt này — một khi một bit
được set, nó giữ nguyên trong suốt phần còn lại của phiên IAP (chỉ một
reset toàn bộ, E1-E4, mới xóa nó qua khởi tạo lại BSS).
**Đây là khoảng hở thực thi đã được nêu trong `06_event_flow.md` E30**:
đạt tới `CHKSUM_ERR` không ngăn `msw_on_proc_iap()` reset trở lại Main
(`km_extend_io_iap.c:28-41` chỉ kiểm tra bit `COMP` của một thanh ghi
*khác*, `IAP_PG_COMMAND_CHANGE_INFO`, không bao giờ kiểm tra từ trạng
thái này).

### 1.8 Trạng thái driver HAL I2C của vendor (không thuộc sở hữu của ứng dụng này, được nhắc đến để đầy đủ)
`HAL_I2C_STATE_RESET/READY/BUSY/BUSY_TX/BUSY_RX/TIMEOUT/ERROR`, một enum
nội bộ của `stm32f0xx_hal_i2c.h`/`.c` của ST, chỉ được phơi bày ra ứng
dụng qua switch chuyển đổi của `I2Cx_GetStatus()`
(`I2C_stm32f0xx.c:490-536`) thành bitfield `ARM_I2C_STATUS` của CMSIS
mà `i2c_check_error()` đọc (`km_i2c.c:400`). Không được phân tích từng
chuyển tiếp ở đây — nó thuộc về code của vendor, ngoài phạm vi của nỗ
lực reverse-engineering này — nhưng được ghi lại vì tính đúng đắn của
`i2c_check_error()` phụ thuộc vào việc chuyển đổi này chính xác.

---

## 2. Các máy trạng thái ngầm định (điều kiện điều khiển, không có enum hậu thuẫn)

Đây là những máy thực sự có *hình dạng* của máy trạng thái — một tập
điều kiện guard cố định chọn ra một tập action cố định và (gián tiếp,
qua các tác dụng phụ lên GPIO/thanh ghi) một "trạng thái" kế tiếp —
nhưng firmware biểu diễn trạng thái như một tổ hợp các mức chân GPIO và
vài cờ thay vì một biến enum đơn lẻ. Mỗi hàng là một bản chép lại trực
tiếp của một nhánh `if`/`else` đã được trích dẫn ở các phase trước; các
**tên** trạng thái ở cột ngoài cùng bên trái là nhãn `[INFERRED]` để dễ
đọc, không phải định danh trong source.

### 2.1 `g_power_on_flg` — trạng thái boot-vs-steady, 2 giá trị nhưng không có enum

- Kiểu: `static int32_t`, các giá trị là hằng số `#define` thô
  `POWER_ON_FLG_START`/`POWER_ON_FLG_NORMAL` (`km_extend_io.c:44`).

| Trạng thái | Trigger | Điều kiện | Action | Trạng thái kế tiếp | Hàm | File:line |
|---|---|---|---|---|---|---|
| `POWER_ON_FLG_START` (ban đầu) | boot | — | — | `POWER_ON_FLG_START` | khởi tạo static | `km_extend_io.c:44` |
| `POWER_ON_FLG_START` | gọi `s800_power_on()` | — | set `g_power_on_flg = POWER_ON_FLG_START` lại (**tái khẳng định**, `:623` — không phải lỗi đánh máy, dòng này chạy bất kể giá trị trước đó) | `POWER_ON_FLG_START` | `s800_power_on()` | `km_extend_io.c:623` |
| `POWER_ON_FLG_START` | `mc_p_on_sparrow_proc()` (chỉ model Sparrow) thấy `_MC_P_ON_OK()` true và timer trình tự đã kết thúc | — | `g_power_on_flg = POWER_ON_FLG_NORMAL` | `POWER_ON_FLG_NORMAL` | `mc_p_on_sparrow_proc()` | `km_extend_io.c:815-824` |
| `POWER_ON_FLG_NORMAL` | *(model Eagle: không tìm thấy chuyển tiếp nào được lập trình quay về `NORMAL` trong `mc_p_on_eagle_proc()` — hàm đó chỉ bao giờ đọc cờ, `:765`)* | — | — | giữ `NORMAL` cho đến lần `s800_power_on()` kế tiếp | — | — |

**Ý nghĩa:** chuyển tiếp sang `NORMAL` của cờ này **chỉ được lập trình
cho đường model Sparrow**; đường Eagle (`mc_p_on_eagle_proc()`) đọc cờ
nhưng `[UNKNOWN]` — không tìm thấy lệnh ghi nào vào `POWER_ON_FLG_NORMAL`
trong bất kỳ hàm nào riêng của Eagle trong các file được đọc lượt này.
Vì `s800_power_on()` vô điều kiện set lại nó về `START` ở mỗi lần bật
nguồn bất kể model, và `START` cũng là giá trị mặc định lúc boot,
`[INFERRED]` đường Eagle có thể đơn giản là không bao giờ đạt tới
`NORMAL`, hoặc chuyển tiếp xảy ra ở đâu đó không được bao phủ bởi lần
đọc của phiên này — được đánh dấu `[UNKNOWN]`, không giả định theo
hướng nào.

### 2.2 `msw_on_proc()` — máy ngầm định trình tự bật nguồn chính

Hàm đơn lẻ này (`km_extend_io.c:1149-1219`) là thứ gần nhất mà firmware
này có với một máy trạng thái nguồn ở tầng cao nhất, nhưng nó hoàn toàn
là if/else trên các lần đọc chân trực tiếp và trạng thái timer, không
phải một biến trạng thái được lưu trữ. Được trình bày dưới dạng bảng
quyết định theo thứ tự đánh giá của chính hàm (mỗi hàng chỉ được kiểm
tra nếu các hàng ở trên chưa `return`):

| Guard (theo thứ tự đánh giá) | Điều kiện (nguyên văn từ source) | Action | File:line |
|---|---|---|---|
| 1. Timer chờ backup đang chạy | `TYPE_Km_Timer_Start == km_timer_get_state(BACKUP_WAITTIME)` | nếu MSW hiện đang bật, latch một bit cờ pending; **return** (không xử lý thêm ở lần gọi này) | `:1155-1160` |
| 2. Timer trình tự tắt nguồn đang chạy | `TYPE_Km_Timer_Start == seqstate` | nếu MSW bật, `s800_power_off()` (reboot); **return** | `:1163-1168` |
| 3. Timer trình tự tắt nguồn vừa kết thúc | `TYPE_Km_Timer_End == seqstate` | **return**, không làm gì | `:1169-1170` |
| 4a. Được gọi cho một cạnh đã xác nhận debounce (`state==STATE_INT`), MSW bật + 24V11 tắt + SB_PG tắt | cả ba điều kiện chân/chattering | `HAL_NVIC_SystemReset()` | `:1176-1181` |
| 4b. `state==STATE_INT`, MSW tắt | MSW tắt (đã debounce) | nếu AP_PWR_EN tắt và không ở Sleep2/ErP → `s800_power_off()`; ngược lại `poweroff_timer_start()` | `:1182-1192` |
| 5a. Poll mỗi vòng lặp (`state!=STATE_INT`), MSW tắt, SB_PG tắt | — | `MSWOFF_STOP_MODE` đang hoạt động → arm chu kỳ STOP-mode 2s (§1.5); (thay thế bị vô hiệu hoá: sleep mode thuần) | `:1195-1209` |
| 5b. Poll mỗi vòng lặp, MSW bật, SB_PG tắt | — | `s800_power_on()` | `:1211-1214` |

**Ý nghĩa:** các hàng 1-3 thực chất là ba *trạng thái ngầm định* bổ
sung ("đang backup," "đang tắt nguồn," "vừa tắt nguồn xong, bỏ qua MSW
lượt này") được xếp phía trước logic MSW "thực sự" (hàng 4-5) đơn thuần
bằng thứ tự early-return — không có gì đánh dấu chúng là trạng thái ở
bất cứ đâu; chúng chỉ tồn tại như hình dạng luồng điều khiển của một
hàm duy nhất này.

### 2.3 `pending_factor` — 2-trạng-thái armed/consumed theo từng bit (4 instance độc lập)

- Mỗi bit trong 4 bit (`TYPE_Km_Pending_Factor_Bit`: HRESET_REQ,
  AP_PWR_EN, POWER_MONITOR, MSW_ON — `km_it.h:50-58`) hoạt động như một
  cờ 2-trạng-thái độc lập của riêng nó (armed/clear), dù chỉ 2 trong 4
  (`HRESET_REQ`, `AP_PWR_EN`) từng thực sự được set bởi
  `km_exti_callback()` (`km_it.c:254,257`) — các bit
  `POWER_MONITOR`/`MSW_ON` trong enum này được khai báo nhưng
  `[INFERRED]` không được sử dụng bởi bất kỳ lệnh gọi
  `set_pending_factor_bit()` nào tìm thấy trong lượt này (hai tín hiệu
  đó thay vào đó đi thẳng đến máy anti-chattering, §1.2, hoàn toàn
  không qua `pending_factor`).

| Trạng thái | Trigger | Action | Trạng thái kế tiếp | Hàm | File:line |
|---|---|---|---|---|---|
| clear (ban đầu) | cạnh EXTI khớp (chỉ HRESET_REQ hoặc AP_PWR_EN) | `set_pending_factor_bit()` | armed | `km_exti_callback()` (ISR) | `km_it.c:254,257` |
| armed | `intr_pending_proc()` poll và thấy bit được set | `clear_pending_factor_bit()` rồi dispatch (`hreset_req_proc()`/`ap_power_en_proc(STATE_INT)`) | clear | `intr_pending_proc()` | `km_extend_io.c:1416-1426` |
| armed hoặc clear | `s800_power_off()` chạy | `clear_pending_factor_bit(TYPE_Km_PFB_All)` — cưỡng bức xóa mọi thứ | clear (tất cả bit) | `s800_power_off()` | `km_extend_io.c:535` |

---

## 3. Tổng hợp — entry/exit/timeout/retry/error/recovery trên tất cả các máy

| Máy | Action lúc vào | Action lúc ra | Timeout | Retry | Trạng thái lỗi | Khôi phục |
|---|---|---|---|---|---|---|
| Giao thức I2C (§1.1) | arm Rx hoặc Tx qua `change_state()` | không khác gì so với lần vào kế tiếp | 1-2s (`i2c_check_error`) | **không có** | **không được mô hình hoá** — quay về trạng thái ban đầu | `i2c_sw_reset()` |
| Anti-chattering (§1.2) | latch mức start/prev | không có | 10-30ms tùy tín hiệu | ngầm định (arm lại khi bị dội) | không có | tự xóa về `AC_STOP_STATE` |
| Timer phần mềm (§1.3) | set `ms_time` | không có | chính giá trị `ms_time` (caller tự chọn) | n/a | không có | tùy consumer, không phải lúc nào cũng được lập trình (xem ghi chú) |
| Trạng thái CA72 (§1.4) | không có | không có | không có | n/a | không có | không cần (ghi đè vô điều kiện) |
| Chế độ STOP (§1.5) | `set_cycle_time`/`BSP_PWR_enter_stopmode` | khóa lại PLL / xóa alarm | tự thức dậy ~2s | n/a (lặp lại kiểu E25) | không có | thoát do ISR điều khiển, cả hai đường |
| Chốt IWDG (§1.6) | `HAL_IWDG_Start` | không có (một chiều) | ~26.2s (phần cứng) | n/a | n/a | reset toàn bộ MCU (E4) |
| Các bit trạng thái IAP (§1.7) | không có (bit được set cộng dồn) | không có | không có | **không có** | các bit `CHKSUM_ERR`/`PSCPU_ERR` — **không được thực thi** | không có gì được lập trình — master bên ngoài phải tự hành động |
| `g_power_on_flg` (§2.1) | — | — | không có | n/a | `[UNKNOWN]` chuyển tiếp của Eagle | được tái khẳng định ở mỗi `s800_power_on()` |
| `msw_on_proc` (§2.2) | — | — | được điều khiển bởi các timer ở §1.3 | n/a | không có | `HAL_NVIC_SystemReset()` bản thân nó chính là "khôi phục" cho trường hợp hàng 4a |
| `pending_factor` (§2.3) | bit set trong ISR | bit xóa trong main-loop | không có | n/a | không có | cưỡng bức xóa khi tắt nguồn |

**Quan sát tổng thể:** trên toàn bộ các máy trạng thái trong firmware
này, chỉ một máy duy nhất (§1.1, giao thức I2C) có một hành động khôi
phục chuyên dụng, có thể lặp lại (`i2c_sw_reset()`); mọi máy khác hoặc
là không có đường xử lý lỗi nào cả, hoặc "khôi phục" của nó thực chất là
một reset toàn hệ thống (`HAL_NVIC_SystemReset()`) thay vì quay về một
trạng thái đã biết là tốt bên trong chính máy đó. Không có mẫu
retry-with-backoff nào ở bất cứ đâu trong firmware — mọi lỗi được phát
hiện đều được xử lý bằng cách reset một ngoại vi (I2C) hoặc reset toàn
bộ MCU.
