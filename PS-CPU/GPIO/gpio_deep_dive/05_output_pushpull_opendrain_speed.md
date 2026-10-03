# Phần 7-9: `GPIO_MODE_OUTPUT_PP`, Open-Drain, `GPIO_SPEED_FREQ_LOW`

## Push-Pull — `GPIO_MODE_OUTPUT_PP`

`[SOURCE]` [stm32f0xx_hal_gpio.h:136](../../common/Drivers/STM32F0xx_HAL_Driver/Inc/stm32f0xx_hal_gpio.h#L136):
```c
#define GPIO_MODE_OUTPUT_PP  ((uint32_t)0x00000001)
```

`[GENERAL MCU KNOWLEDGE]` "Push-Pull" nghĩa là output driver có **2 transistor chủ động**, một để
**"push" (đẩy) chân lên VDD** khi muốn HIGH, một để **"pull" (kéo) chân xuống GND** khi muốn LOW —
luôn luôn có 1 trong 2 đang dẫn, chân luôn được **ép về một mức rõ ràng**, không bao giờ "lửng lơ".

```text
        VDD
         |
        [T1: PMOS] ── dẫn khi muốn HIGH ──┐
         |                                 ├──► Pin
        [T2: NMOS] ── dẫn khi muốn LOW  ──┘
         |
        GND
```
- Muốn **HIGH**: T1 dẫn (nối pin với VDD), T2 tắt → pin bị **đẩy mạnh** lên gần VDD.
- Muốn **LOW**: T2 dẫn (nối pin với GND), T1 tắt → pin bị **kéo mạnh** xuống gần GND.
- **Không bao giờ cả 2 cùng dẫn** (nếu cùng dẫn sẽ đoản mạch VDD→GND qua 2 transistor, gọi là "shoot-through", chip tự thiết kế logic để tránh).

### Tại sao không nên nối 2 output push-pull trực tiếp với nhau nếu mức khác nhau?

`[GENERAL MCU KNOWLEDGE]` Nếu chân A (push-pull, đang HIGH → T1 dẫn, nối cứng với VDD) nối trực tiếp
với chân B (push-pull, đang LOW → T2 dẫn, nối cứng với GND) → tạo thành **một đường dẫn điện trở thấp
từ VDD qua T1(A) → dây nối → T2(B) → GND**, bỏ qua tải thông thường. Dòng điện lớn bất thường chạy
qua, có thể làm **nóng và hỏng transistor** ở cả 2 chip (gọi là "output contention" / "bus fight").
Đây là lý do 2 thiết bị push-pull **không bao giờ được nối chung 1 dây tín hiệu** trừ khi có cơ chế
phân chia thời gian driver rõ ràng (ví dụ bus 3-state).

## Open-Drain — so sánh với Push-Pull

`[SOURCE]` [stm32f0xx_hal_gpio.h:137](../../common/Drivers/STM32F0xx_HAL_Driver/Inc/stm32f0xx_hal_gpio.h#L137):
```c
#define GPIO_MODE_OUTPUT_OD  ((uint32_t)0x00000011)
```
(Project PS-CPU **không dùng** Open-Drain cho bất kỳ pin nào trong `MX_GPIO_Init()` — toàn bộ output
đều là `OUTPUT_PP`. Phần này giải thích kiến thức tổng quát để bạn hiểu tại sao, và liên hệ với I2C1
trong cùng project — xem cuối phần.)

```text
Push-Pull:                          Open-Drain:
   VDD                                 VDD
    |                                   |  (không có trong chip — phải thêm pull-up ngoài hoặc dùng pull-up nội)
  [T1 PMOS]──┐                        (không tồn tại transistor kéo lên)
    |        ├─► Pin                                    ┌─► Pin
  [T2 NMOS]──┘                        [T2 NMOS] ─────────┘
    |                                   |
   GND                                 GND
```

| | Push-Pull | Open-Drain |
|---|---|---|
| **HIGH** | Transistor **chủ động đẩy** lên VDD | **Không có gì đẩy** — chân chỉ "thả" (floating), phải nhờ **pull-up** (nội hoặc ngoài) kéo lên |
| **LOW** | Transistor **chủ động kéo** xuống GND | Transistor NMOS kéo xuống GND — giống Push-Pull ở phần LOW |
| **Pull-up cần không?** | Không cần | **Bắt buộc cần** (nội hoặc ngoài) để có mức HIGH |
| **I2C dùng kiểu nào?** | Không | **Luôn dùng Open-Drain** |
| **Nhiều thiết bị chung 1 dây?** | Nguy hiểm (bus fight, xem trên) | **An toàn** — nhiều chip có thể cùng kéo LOW, không ai "chiến đấu" vì không ai chủ động đẩy HIGH |

### Tại sao I2C thường dùng Open-Drain?

`[GENERAL MCU KNOWLEDGE]` I2C là bus **nhiều thiết bị chia sẻ 1 dây** (SDA, SCL) — bất kỳ thiết bị nào
trên bus cũng có thể cần kéo dây xuống LOW (để báo ACK/NACK, hoặc "clock stretching"). Nếu dùng
push-pull, 2 thiết bị cùng lúc cố gắng ép 2 mức khác nhau sẽ gây đoản mạch (xem trên). Open-drain giải
quyết: **mọi thiết bị chỉ có khả năng kéo xuống LOW**, không ai chủ động đẩy HIGH — khi tất cả thiết bị
đều "thả tay" (không kéo xuống), **1 điện trở pull-up chung** (bắt buộc phải có trên bus) kéo dây lên
HIGH. → Nhiều thiết bị "rảnh tay" an toàn trên cùng 1 dây.

`[SOURCE]` Trong chính project PS-CPU, I2C1 — nơi PS-CPU đóng vai trò slave giao tiếp với host — cấu
hình 2 chân SCL/SDA ở `stm32f0xx_hal_msp.c`. Đây là ví dụ thật của Open-Drain trong cùng codebase (dù
không phải trong `MX_GPIO_Init()` mà bạn đang hỏi, mà trong `HAL_I2C_MspInit()` — đã xem khi phân tích
`HAL_Init()` trước đó, [stm32f0xx_hal_msp.c:66-80](../../pscpu_s800/main/Src/stm32f0xx_hal_msp.c#L66-L80)).

## `GPIO_SPEED_FREQ_LOW` — không phải "GPIO chỉ chạy tần số thấp"

```c
GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
```
`[SOURCE]` dùng ở nhiều chỗ trong `MX_GPIO_Init()`, ví dụ
[mx_init.c:450,461](../../pscpu_s800/main/Src/mx_init.c#L450)

**Hiểu nhầm phổ biến**: "Speed" ở đây **không giới hạn tốc độ toggle logic** (bạn vẫn có thể
`HAL_GPIO_TogglePin()` nhanh tùy ý ở phần mềm) — nó điều khiển **slew rate** của transistor output,
tức là **tốc độ chuyển tiếp vật lý giữa 2 mức điện áp** (bao lâu thì điện áp đi hết từ 0V lên 3.3V hay
ngược lại).

```text
GPIO_SPEED_FREQ_LOW (slew rate thấp):          GPIO_SPEED_FREQ_HIGH (slew rate cao):

3.3V ┤        ╱‾‾‾‾‾                    3.3V ┤       ┌──────
     │      ╱                                │       │
     │    ╱      (rise time dài hơn)         │       │  (rise time ngắn hơn, gần như thẳng đứng)
0V   ┤──╱                               0V   ┤───────┘
     └──────────────────► t                  └──────────────► t
```

`[SOURCE]` Chính header HAL của chip này ghi rõ khoảng tần số tương ứng với từng mức Speed (đây là
số liệu lấy từ datasheet ST, không phải suy diễn):
[stm32f0xx_hal_gpio.h:155,157](../../common/Drivers/STM32F0xx_HAL_Driver/Inc/stm32f0xx_hal_gpio.h#L155):
```c
#define GPIO_SPEED_FREQ_LOW   0x00000000   /* range up to 2 MHz */
#define GPIO_SPEED_FREQ_HIGH  0x00000003   /* range 10 MHz to 50 MHz */
```
→ `GPIO_SPEED_FREQ_LOW` nghĩa là driver output được tối ưu để **chuyển mức sạch sẽ (ít ringing/overshoot)
ở tần số tín hiệu tới khoảng 2MHz** — không phải "GPIO chỉ hoạt động được dưới 2MHz". Bạn vẫn có thể
toggle chân này ở tần số cao hơn bằng phần mềm, chỉ là dạng sóng ra sẽ không còn "sạch" (sườn xung bị
làm tròn, rise/fall time kéo dài tương đối so với period) — với tín hiệu điều khiển on/off tần số thấp
như trong project này thì không quan trọng.

`[GENERAL MCU KNOWLEDGE]`
- **Slew rate** = tốc độ thay đổi điện áp theo thời gian (V/ns). Speed cao → slew rate cao → rise/fall
  time ngắn → sườn xung "dựng đứng" hơn.
- **Tại sao tốc độ thấp giảm nhiễu?** Sườn xung dựng đứng (rise/fall time ngắn) chứa nhiều thành phần
  tần số cao trong miền Fourier → bức xạ EMI (nhiễu điện từ) mạnh hơn, dễ gây crosstalk sang dây dẫn
  lân cận trên board, đặc biệt nếu dây dẫn dài (như dây nối ra connector ngoài board). Dùng
  `SPEED_LOW` cho các tín hiệu không cần tần số cao giúp **giảm EMI và cải thiện signal integrity**
  (ít ringing/overshoot do phản xạ trên đường dây dài).
- Ngược lại, nếu tín hiệu thực sự cần chuyển mức rất nhanh (ví dụ clock SPI tần số cao), cần chọn
  Speed cao hơn để tín hiệu "theo kịp" — nhưng đổi lại chịu EMI cao hơn, cần thiết kế layout kỹ hơn.

### Tại sao toàn bộ output trong `MX_GPIO_Init()` đều chọn `SPEED_LOW`?

`[INFERENCE]` Nhìn vào chức năng các chân output trong project (`MC_PWR_EN`, `SB_PWR_EN`, `MC_P_ON`,
`IR_P_ON`, `_RESET`, `_RST_SLP2`, `SLEEP_STATUS_REM_SP/EG`...) — tất cả đều là **tín hiệu điều khiển
mức logic chậm** (bật/tắt nguồn, reset, báo trạng thái sleep), được code xử lý bằng các hàm polling
trong vòng lặp `while(1)` (tần số thay đổi mức tối đa cỡ vài lần/giây, không phải tín hiệu clock tốc độ
cao). Không có tín hiệu nào trong nhóm này cần sườn xung dựng đứng → chọn `SPEED_LOW` hợp lý để giảm
nhiễu, tăng độ bền tín hiệu khi đi dây dài ra các khối nguồn/relay bên ngoài MCU. Đây là suy luận dựa
trên cách các hàm xử lý các pin này trong `km_extend_io.c` (đã đọc ở các câu hỏi trước), **không có
dòng comment nào trong code xác nhận trực tiếp lý do chọn Speed Low**.

## Lỗi thường gặp

- Nhầm "Speed" với "clock của peripheral" → tưởng chọn Low sẽ làm chậm toggle bằng code, thực ra không
  liên quan.
- Chọn Speed quá cao cho tín hiệu không cần → tốn điện hơn (driver mạnh hơn tiêu thụ nhiều hơn) và
  tăng nhiễu EMI không cần thiết, có thể ảnh hưởng tới các mạch analog nhạy cảm gần đó (ví dụ ADC).
- Chọn Speed quá thấp cho tín hiệu cần nhanh (ví dụ SPI clock) → sườn xung không kịp "dựng" trước khi
  thiết bị nhận lấy mẫu → dữ liệu sai ở tần số cao.

Tiếp theo: [06_exti_nvic_interrupt_chain.md](06_exti_nvic_interrupt_chain.md) — phần ngắt, đây là phần
đào sâu nhất theo yêu cầu của bạn.
