# Phần 16-19: Phân tích toàn bộ `MX_GPIO_Init()` theo chức năng, GPIO Map, cấu hình trùng, `#if 0`

## Phần 16 — Nhóm toàn bộ pin theo chức năng

`[SOURCE]` Toàn bộ danh sách định nghĩa tại
[mxconstants.h:44-97](../../pscpu_s800/main/Inc/mxconstants.h#L44-L97), đối chiếu với cấu hình thật
trong `MX_GPIO_Init()` ([mx_init.c:419-518](../../pscpu_s800/main/Src/mx_init.c#L419-L518)).

### Input thường (không ngắt)

| Pin | Port | Chức năng suy luận từ tên |
|---|---|---|
| `PF0_IN_Pin`, `PF1_IN_Pin` | PF0, PF1 | `[INFERENCE]` Input dự phòng/chưa rõ mục đích cụ thể — tên chỉ nói "IN", không đủ để biết chức năng chính xác. |
| `MODEL_BIT0_Pin`, `MODEL_BIT1_Pin` | PB14, PB15 | `[INFERENCE]` 2-bit đọc từ board (ví dụ cầu nối/resistor strap) để firmware biết đang chạy trên biến thể máy nào — khớp với tham số `TYPE_Model model` dùng trong `mc_p_on_proc(model)` đã phân tích ở câu hỏi trước (Eagle/Sparrow). Cần schematic để xác nhận cách đọc 2-bit này map ra giá trị model cụ thể. |
| `SB_PG_Pin` | PB4 | `[SOURCE]`+`[INFERENCE]` "SB Power Good" — dùng trong điều kiện `_MC_P_ON_OK()` đã đọc ở câu hỏi trước ([km_extend_io.c:738](../../pscpu_s800/main/App/km_extend_io.c#L738)) — báo nguồn SB (sub power) đã ổn định. |
| `_DISCHG_Pin` | PB8 | `[INFERENCE]` Tên gợi ý "discharge" — có thể là input đọc trạng thái mạch xả tụ, nhưng trong code đã đọc trước đây, `_DISCHG_Pin` lại được ghi (`BSP_GPIO_WritePin`) ở [km_extend_io.c:409](../../pscpu_s800/main/App/km_extend_io.c#L409) — **mâu thuẫn với việc cấu hình là `GPIO_MODE_INPUT` ở đây**. Đây là điểm cần làm rõ, xem mục cảnh báo cuối phần 17. |
| `MC_PG_Pin` | PA11 | `[INFERENCE]` "MC Power Good" — tương tự SB_PG nhưng cho khối Main Controller. |
| `MC3_3VON_MONI_Pin` | PA15 | `[SOURCE]` Dùng trong `mc3_3von_moni_proc()` (thấy tên hàm trong `entry.c` ở câu hỏi trước) — giám sát điện áp 3.3V của khối MC. |
| `PA9_IN_Pin`, `PA10_IN_Pin` | PA9, PA10 | `[SOURCE]`+`[INFERENCE]` Input dự phòng, pull-down; theo tài liệu đã đọc, hỗ trợ vật lý USART1_TX/RX nhưng cố ý không dùng UART. |

### Output

| Pin | Port | Chức năng (đã xác nhận ở các câu hỏi trước, có [SOURCE] rõ ràng) |
|---|---|---|
| `IR_P_ON_Pin` | PB0 | Cấp nguồn khối Image Reading — `ir_p_on_proc()` |
| `ERP_SENSOR_ON_Pin` | PB1 | Bật cảm biến ErP — `erp_sensor_proc()` |
| `_RESET_Pin` | PB2 | Reset chính của S800 (active low) — `s800_power_on()`, `sb_reset_proc()` |
| `_USB2_OE_Pin` | PB10 | Output-enable cho USB2 (cũng dùng làm LED debug trong `#ifdef DEBUG_LED`, [km_it.c:292-296](../../pscpu_s800/main/App/km_it.c#L292-L296)) |
| `SLEEP_STATUS_REM_EG_Pin` | PB12 | Báo trạng thái sleep — biến thể "Eagle" |
| `SLEEP_STATUS_REM_SP_Pin` | PA6 | Báo trạng thái sleep — biến thể "Sparrow" (chú ý: đây nằm trên **GPIOA**, cấu hình **riêng** ở dòng trước nhóm GPIOB — xem mục cảnh báo) |
| `MC_PWR_EN_Pin` | PB13 | Enable nguồn MC — `mc_pwr_en_proc()` |
| `SB_PWR_EN_Pin` | PB3 | Enable nguồn SB |
| `MC_P_ON_Pin` | PB5 | Đã phân tích chi tiết ở câu hỏi đầu tiên — cấp nguồn Main Controller |
| `_RST_SLP2_Pin` | PB9 | Reset phụ liên quan Sleep2 |

### Interrupt Input (EXTI)

| Pin | Port | Edge | Chức năng (đã trace thật ở file 06) |
|---|---|---|---|
| `_HRESET_REQ_Pin` | PA1 | Falling | Host yêu cầu reset — `set_pending_factor_bit(HRESET_REQ)` |
| `AP_PWR_EN_Pin` | PA2 | Rising+Falling | CA72/Linux báo đã boot xong — `set_pending_factor_bit(AP_PWR_EN)` |
| `POWER_MONITOR_Pin` | PA4 | Rising+Falling | Giám sát mất điện — `start_anti_chattering(POWER_MONI)` |
| `MSW_ON_Pin` | PA5 | Rising+Falling | Công tắc nguồn cơ khí — `start_anti_chattering(MSW)` |
| `MONI_24V11_Pin` | PA8 | Rising+Falling | Giám sát 24V — `start_anti_chattering(24V11_MONI)` |

### Không được cấu hình trong `MX_GPIO_Init()` dù clock Port đã bật — phát hiện thật, không suy đoán

`[SOURCE]` 2 define tồn tại trong `mxconstants.h` nhưng **không xuất hiện trong bất kỳ lời gọi
`HAL_GPIO_Init()` nào** trong hàm này:

- **`WAKEUP_Pin` (PC13)** — [mxconstants.h:44-45](../../pscpu_s800/main/Inc/mxconstants.h#L44-L45).
  `__HAL_RCC_GPIOC_CLK_ENABLE()` được gọi ([mx_init.c:422](../../pscpu_s800/main/Src/mx_init.c#L422))
  nhưng không có `HAL_GPIO_Init(GPIOC, ...)` nào theo sau trong đoạn code bạn cung cấp.
  `[INFERENCE]` PC13 là chân "User Button" kinh điển trên board phát triển **ST Nucleo** — và chính
  project này có thư mục `common/Drivers/BSP/.../BSP_STM32F03x_Nucleo.c` (tên file tôi đã trích dẫn ở
  file 06) → gợi ý mạnh rằng định nghĩa `WAKEUP_Pin` là **tàn dư từ template CubeMX ban đầu cho board
  Nucleo**, chưa bị dọn dẹp, và **không thực sự dùng trong thiết kế board PS-CPU thật**. Cần schematic
  PS-CPU thật để xác nhận chân này có nối gì không.
- **`TMS_Pin` (PA13)** — [mxconstants.h:84-85](../../pscpu_s800/main/Inc/mxconstants.h#L84-L85).
  `[GENERAL MCU KNOWLEDGE]` PA13 là chân **SWDIO** mặc định của giao diện debug SWD trên STM32. Việc
  **không cấu hình lại** chân này trong `HAL_GPIO_Init()` là **chủ động và đúng** — nếu cấu hình lại
  thành GPIO thường, bạn sẽ **mất khả năng debug/flash lại chip qua SWD** (trừ khi reset chip hoặc dùng
  BOOT0 để vào bootloader). Đây là kiến thức MCU phổ quát, áp dụng chắc chắn cho mọi thiết kế STM32
  không cố ý vô hiệu hóa debug.

## Phần 17 — Bảng GPIO Map đầy đủ

| Pin | Port | Direction | Mode | Pull | Speed | Interrupt | Initial Level | Nguồn |
|---|---|---|---|---|---|---|---|---|
| PF0_IN | F | Input | FLOATING | NOPULL | — | Không | — (mặc định input) | L429-432 |
| PF1_IN | F | Input | FLOATING | NOPULL | — | Không | — | L429-432 |
| _HRESET_REQ | A1 | Input | IT_FALLING | NOPULL | — | **Falling** | — | L436-440 |
| AP_PWR_EN | A2 | Input | IT_RISING_FALLING | NOPULL | — | **Rising+Falling** | — | L442-445 |
| POWER_MONITOR | A4 | Input | IT_RISING_FALLING | NOPULL | — | **Rising+Falling** | — | L442-445 |
| MSW_ON | A5 | Input | IT_RISING_FALLING | NOPULL | — | **Rising+Falling** | — | L442-445 |
| MONI_24V11 | A8 | Input | IT_RISING_FALLING | NOPULL | — | **Rising+Falling** | — | L442-445 |
| SLEEP_STATUS_REM_SP | A6 | Output | PP | NOPULL | LOW | Không | **RESET (LOW)** | L447-451, L504 |
| IR_P_ON | B0 | Output | PP | NOPULL | LOW | Không | **RESET (LOW)** | L455-465, L505-507 |
| ERP_SENSOR_ON | B1 | Output | PP | NOPULL | LOW | Không | **RESET (LOW)** | nt |
| _RESET | B2 | Output | PP | NOPULL | LOW | Không | **RESET (LOW)** | nt |
| SB_PWR_EN | B3 | Output | PP | NOPULL | LOW | Không | **RESET (LOW)** | nt |
| MC_P_ON | B5 | Output | PP | NOPULL | LOW | Không | **RESET (LOW)** | nt |
| _USB2_OE | B10 | Output | PP | NOPULL | LOW | Không | **RESET (LOW)** | nt |
| SLEEP_STATUS_REM_EG | B12 | Output | PP | NOPULL | LOW | Không | **RESET (LOW)** | nt |
| MC_PWR_EN | B13 | Output | PP | NOPULL | LOW | Không | **RESET (LOW)** | nt |
| _RST_SLP2 | B9 | Output | PP | NOPULL | LOW | Không | **RESET (LOW)** | nt |
| MODEL_BIT0 | B14 | Input | FLOATING | NOPULL | — | Không | — | L467-470 |
| MODEL_BIT1 | B15 | Input | FLOATING | NOPULL | — | Không | — | nt |
| SB_PG | B4 | Input | FLOATING | NOPULL | — | Không | — | L467-470 **và** L495-498 (trùng) |
| _DISCHG | B8 | Input | FLOATING | NOPULL | — | Không | — | nt (trùng) |
| MC_PG | A11 | Input | FLOATING | NOPULL | — | Không | — | L473-476 |
| MC3_3VON_MONI | A15 | Input | FLOATING | NOPULL | — | Không | — | nt |
| PA9_IN | A9 | Input | FLOATING | **PULLDOWN** | — | Không | — | L479-483 |
| PA10_IN | A10 | Input | FLOATING | **PULLDOWN** | — | Không | — | nt |
| WAKEUP (PC13) | C13 | *(không cấu hình)* | — | — | — | — | — | chỉ bật clock, L420 |
| TMS (PA13) | A13 | *(không cấu hình — giữ SWDIO)* | — | — | — | — | — | — |

**Cảnh báo phát hiện được khi lập bảng này** (không phải lỗi logic, nhưng đáng ghi chú):
- `_DISCHG_Pin` được cấu hình `GPIO_MODE_INPUT` ở đây, nhưng ở câu hỏi trước bạn đã xem code
  `BSP_GPIO_WritePin(_DISCHG_GPIO_Port, _DISCHG_Pin, GPIO_PIN_RESET)` tại
  [km_extend_io.c:409](../../pscpu_s800/main/App/km_extend_io.c#L409). **Ghi vào một chân đang ở mode
  Input hoàn toàn không có tác dụng vật lý nào** (`WritePin` chỉ ghi `BSRR`/`ODR`, không đụng `MODER`;
  chân vẫn ở mode input, output driver vẫn tắt) — dòng ghi đó trong `km_extend_io.c` **thực chất là
  dead code / vô hiệu lực** nếu `_DISCHG_Pin` không được cấu hình lại thành Output ở nơi khác trong
  project mà tôi chưa đọc tới. `[INFERENCE]` **cần tìm có file nào khác gọi lại `HAL_GPIO_Init()` cho
  `_DISCHG_Pin` với mode Output** (ví dụ runtime re-config) để xác nhận đây có đúng là bug/dead code
  không — đây là 1 điểm tốt để bạn tự điều tra tiếp hoặc hỏi tôi ở lượt sau.

## Phần 18 — Phân tích đoạn cấu hình trùng: `SB_PG_Pin`/`_DISCHG_Pin` bị cấu hình 2 lần

```c
// Lần 1
GPIO_InitStruct.Pin = MODEL_BIT0_Pin|MODEL_BIT1_Pin|SB_PG_Pin|_DISCHG_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
GPIO_InitStruct.Pull = GPIO_NOPULL;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

// ... (2 block khác cho GPIOA) ...

// Lần 2
GPIO_InitStruct.Pin = SB_PG_Pin|_DISCHG_Pin;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
GPIO_InitStruct.Pull = GPIO_NOPULL;
HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
```
`[SOURCE]` [mx_init.c:467-470](../../pscpu_s800/main/Src/mx_init.c#L467-L470) và
[mx_init.c:495-498](../../pscpu_s800/main/Src/mx_init.c#L495-L498)

**Trả lời trực tiếp các câu hỏi bạn đặt ra**:

1. **Có cấu hình trùng không?** Có — `SB_PG_Pin` và `_DISCHG_Pin` được gọi `HAL_GPIO_Init()` **2 lần**.
2. **Lần gọi thứ 2 có override lần đầu không?** Về mặt kỹ thuật **có** — mỗi lần `HAL_GPIO_Init()`
   chạy, nó **ghi đè hoàn toàn** `MODER`/`OTYPER`/`OSPEEDR`/`PUPDR` cho đúng các bit vị trí tương ứng
   (xem lại `HAL_GPIO_Init()` ở file 06/09 — nó luôn `CLEAR_BIT` rồi `SET_BIT`, không phải "chỉ set
   thêm"). Nhưng vì **Mode và Pull ở lần 2 giống hệt lần 1** (`GPIO_MODE_INPUT`, `GPIO_NOPULL`), kết
   quả cuối cùng **không đổi gì** so với chỉ chạy lần 1.
3. **Có phải bug không?** `[GENERAL MCU KNOWLEDGE]`+`[INFERENCE]` **Không gây lỗi hành vi runtime**
   (vì cấu hình giống nhau cả 2 lần) — nhưng là **code dư thừa (redundant)**, không có lý do chức năng
   để giữ lại. Không phải "bug" theo nghĩa gây sai kết quả, nhưng là điểm có thể dọn dẹp.
4. **Có thể do code generator không?** `[INFERENCE]` **Rất có khả năng cao** — đây là dấu hiệu kinh
   điển của **STM32CubeMX**: công cụ này sinh code theo cách nhóm các pin **theo Mode+Pull+Speed giống
   nhau trong 1 lần tạo cấu hình gần nhau**, nhưng nếu người dùng CubeMX **sửa Label/thuộc tính của pin
   SB_PG hoặc _DISCHG riêng lẻ sau đó** (ví dụ gán lại tên hoặc mode rồi đổi lại), CubeMX có xu hướng
   **tạo thêm 1 block cấu hình mới cho đúng pin đó** mà không luôn luôn dọn sạch block cũ, dẫn đến 2
   block trùng lặp về mặt hiệu lực cuối cùng. Đây là hiện tượng **đã được biết đến rộng rãi** với
   STM32CubeMX khi file `.ioc` được chỉnh sửa nhiều lần qua các version khác nhau — không phải do lập
   trình viên tay viết thêm nhầm.
5. **Có ảnh hưởng runtime không?** Không — chỉ tốn thêm vài chu kỳ CPU lúc khởi động (gọi
   `HAL_GPIO_Init()` thêm 1 lần, vòng lặp 16-bit chạy lại) — hoàn toàn không đáng kể về hiệu năng, và
   không gây sai kết quả vì cấu hình giống nhau.

## Phần 19 — `#if 0` là gì?

```c
#if 0
  /* デバッグ... */
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  ...
#endif
```
`[SOURCE]` [mx_init.c:485-493](../../pscpu_s800/main/Src/mx_init.c#L485-L493)

`[GENERAL MCU KNOWLEDGE]` `#if 0` / `#endif` là chỉ thị **tiền xử lý (preprocessor)** của C — chạy
**trước khi compiler thực sự dịch code**. `#if 0` nghĩa là "điều kiện này luôn sai" → toàn bộ các dòng
giữa `#if 0` và `#endif` **bị preprocessor loại bỏ hoàn toàn khỏi mã nguồn** trước khi compiler nhìn
thấy chúng — không khác gì như các dòng đó **chưa từng tồn tại** trong file khi build.

### Khác gì so với comment (`/* ... */` hoặc `//`)?

| | `#if 0` ... `#endif` | Comment `/* */` |
|---|---|---|
| Ai xử lý? | Preprocessor (bước trước compile) | Compiler lexer (bước đầu compile) |
| Code bên trong vẫn phải là **cú pháp C hợp lệ**? | **Không bắt buộc** — vì bị loại bỏ hoàn toàn trước khi compiler kiểm cú pháp | Không áp dụng — comment không chứa "code" theo nghĩa cần hợp lệ |
| Dễ bật lại không? | Rất dễ — chỉ cần đổi `#if 0` → `#if 1` | Phải xóa `/* */` thủ công, dễ quên dấu đóng nếu code có comment lồng nhau |
| Dùng để làm gì phổ biến? | Tắt tạm 1 khối code lớn, có thể **chứa comment bên trong** mà không lo xung đột `/* */` lồng nhau | Giải thích code, hoặc tắt 1-2 dòng ngắn |

### Tại sao developer dùng `#if 0` để tắt code debug?

`[INFERENCE]` Đoạn code bị tắt (set `GPIO_PIN_6|GPIO_PIN_7` trên GPIOF thành Output) có comment tiếng
Nhật "デバッグ" (debug) ngay trong code — cho thấy đây từng là code dùng để **test/debug bằng tay**
(có thể gắn LED hoặc logic analyzer vào 2 chân đó để quan sát). Dùng `#if 0` thay vì xóa hẳn giúp
developer **giữ lại đoạn code đó trong lịch sử file** (dễ bật lại khi cần debug tiếp ở tương lai) mà
**không ảnh hưởng tới binary thật được build và nạp vào chip production** — khác với chỉ comment
thông thường, `#if 0` rõ ràng hơn về chủ đích "code này bị tắt có chủ đích, không phải quên dọn".

Tiếp theo: [09_hal_to_register_trace.md](09_hal_to_register_trace.md) — phần quan trọng nhất: trace
từ code xuống tận bit register thật.
