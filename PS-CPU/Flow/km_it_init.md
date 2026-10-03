`km_it_init()` **không ghi thanh ghi phần cứng nào**. Nó chỉ ghi ba thứ vào RAM: bộ đếm `ei_ref[]`, con trỏ `signal_event` của driver GPIO và con trỏ `SignalHandler` của driver TIM. Phần phần cứng nằm ở lúc ngắt xảy ra sau này, nên tôi vẽ cả đường đó.

## Diagram 1: `km_it_init()` (chỉ ghi RAM)

```mermaid
sequenceDiagram
    autonumber
    participant EN as main (entry.c dòng 65)
    participant KI as km_it_init (km_it.c dòng 364)
    participant IE as init_ei_ref (km_it.c dòng 72)
    participant BG as BSP_GPIO_Initialize (BSP_GPIO dòng 87)
    participant BT as BSP_TIM_Initialize (BSP_TIM dòng 98)
    participant RAM as Biến trong RAM

    EN->>KI: km_it_init()
    KI->>IE: init_ei_ref()
    loop i = 0..31
        IE->>RAM: ei_ref[i] = 0 (static int ei_ref[32])
    end
    KI->>BG: BSP_GPIO_Initialize(km_exti_callback)
    BG->>RAM: signal_event = km_exti_callback (static, BSP_GPIO dòng 10)
    BG-->>KI: BSP_OK
    KI->>BT: BSP_TIM_Initialize(TIMER_3, km_tim_callback)
    BT->>RAM: Resource[0].SignalHandler = km_tim_callback
    BT-->>KI: BSP_OK
    KI-->>EN: return
```

## Diagram 2: đường ngắt GPIO tới thanh ghi (nơi `signal_event` được dùng)

```mermaid
sequenceDiagram
    autonumber
    participant HW as Chân PA1/2/4/5/8
    participant EX as Thanh ghi EXTI (0x40010400)
    participant NV as NVIC (0xE000E100)
    participant IH as EXTIx_IRQHandler (stm32f0xx_it.c)
    participant HG as HAL_GPIO_EXTI_IRQHandler (hal_gpio.c dòng 502)
    participant CB as HAL_GPIO_EXTI_Callback (BSP_GPIO dòng 71)
    participant KC as km_exti_callback (km_it.c dòng 247)

    HW->>EX: cạnh theo RTSR/FTSR thì EXTI_PR bit pos = 1
    EX->>NV: yêu cầu ngắt (EXTI0_1=5, EXTI2_3=6, EXTI4_15=7)
    NV->>IH: nhảy vào handler (nếu ISER bit đang bật)
    IH->>HG: HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_x)
    HG->>EX: đọc EXTI_PR & pin, nếu khác 0:
    HG->>EX: EXTI_PR = pin (ghi 1 để xóa cờ)
    HG->>CB: HAL_GPIO_EXTI_Callback(pin)
    CB->>CB: signal_event != NULL ?
    CB->>KC: signal_event(pin, GPIO_EVENT_EXT_INTR)
    KC->>KC: đặt cờ pending hoặc start_anti_chattering
```

## Diagram 3: đường ngắt TIM3 tới thanh ghi (nơi `SignalHandler` được dùng)

```mermaid
sequenceDiagram
    autonumber
    participant T3 as TIM3 (0x40000400)
    participant NV as NVIC
    participant IH as TIM3_IRQHandler (stm32f0xx_it.c)
    participant HT as HAL_TIM_IRQHandler (hal_tim.c dòng 2763)
    participant PE as HAL_TIM_PeriodElapsedCallback (BSP_TIM dòng 72)
    participant KT as km_tim_callback (km_it.c dòng 312)

    T3->>T3: CNT chạm ARR thì TIM3_SR.UIF = 1
    T3->>NV: yêu cầu ngắt (nếu DIER.UIE = 1), IRQ 16
    NV->>IH: nhảy vào TIM3_IRQHandler
    IH->>HT: HAL_TIM_IRQHandler(&htim3)
    HT->>T3: đọc SR.UIF và DIER.UIE, cùng bằng 1 thì tiếp
    HT->>T3: SR = ~UIF (xóa cờ)
    HT->>PE: HAL_TIM_PeriodElapsedCallback(&htim3)
    PE->>PE: tìm Resource[type] có Handle == htim
    PE->>KT: Resource[0].SignalHandler(TIMER_3, TIM_EVENT_PERIOD_ELAPSED)
    KT->>KT: trừ 1 ms cho mọi timer đang Start trong g_km_timer
```

## Giải thích từng bước

1. **`init_ei_ref` (RAM, không phải thanh ghi).** `[SOURCE]` Xóa mảng `ei_ref[32]` về 0. Mảng này là bộ đếm tham chiếu cho cặp `km_disable_irq` / `km_enable_irq` ([km_it.c:436-460](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/PS-CPU/pscpu_s800/main/App/km_it.c#L436-L460)). Cặp này chỉ chạm thanh ghi NVIC khi bộ đếm chuyển giữa 0 và 1:
    - `km_disable_irq`: nếu `ei_ref[IRQn]++ == 0` thì mới gọi `NVIC_DisableIRQ`, tức ghi `NVIC_ICER` bit tương ứng.
    - `km_enable_irq`: nếu `--ei_ref[IRQn] == 0` thì mới gọi `NVIC_EnableIRQ`, tức ghi `NVIC_ISER`.
    - Giá trị khởi đầu 0 khớp với trạng thái "ngắt đang bật" (do `MX_GPIO_Init` đã bật EXTI trước đó mà không qua `km_enable_irq`).
2. **`BSP_GPIO_Initialize`.** `[SOURCE]` Gán `signal_event = km_exti_callback`. Biến này là `static` trong `BSP_GPIO_...c`, khởi đầu `NULL`. Sau lệnh gán, đường Diagram 2 mới có nơi đến.
3. **`BSP_TIM_Initialize`.** `[SOURCE]` Gán `Resource[TIMER_3].SignalHandler = km_tim_callback`. Bảng `Resource[]` chỉ có một phần tử (`htim3`) vì các timer khác nằm trong `#ifdef STM32F030x8`, mà chip này là F031.
4. **Đường ngắt GPIO (Diagram 2).** `[SOURCE]` Điểm đáng chú ý là hàm `HAL_GPIO_EXTI_Callback` ở đây do dự án định nghĩa lại, thay cho bản `__weak` của HAL. Nó chuyển sự kiện cho `signal_event` nếu đã được đăng ký. Tệp [stm32f0xx_it.c](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/PS-CPU/pscpu_s800/main/Src/stm32f0xx_it.c) gọi `HAL_GPIO_EXTI_IRQHandler` cho từng chân: `EXTI0_1` chỉ chân 1, `EXTI2_3` chỉ chân 2, `EXTI4_15` chân 4, 5 và 8 (đủ cho `POWER_MONITOR`, `MSW_ON`, `MONI_24V11`).
5. **Xóa cờ ngắt.** `[SOURCE]` `EXTI->PR = pin` (ghi 1 để xóa). Cờ phải được xóa trước khi gọi callback, nếu không ngắt sẽ vào lại liên tục.
6. **Đường ngắt TIM3 (Diagram 3).** `[SOURCE]` `HAL_TIM_IRQHandler` kiểm tra cả cờ `UIF` lẫn bit `UIE` rồi mới xóa cờ (`SR = ~UIF`) và gọi `HAL_TIM_PeriodElapsedCallback`. Bản callback này cũng do dự án định nghĩa lại, dò bảng `Resource[]` theo địa chỉ `htim` để tìm đúng `SignalHandler`.

## Điều quan trọng cần nhớ

- **Trước `km_it_init()`: ngắt đã bật nhưng chưa có người nhận.** `[SOURCE]` `MX_GPIO_Init` đã bật NVIC cho EXTI từ trước (không qua `km_enable_irq`). Nếu có cạnh nào đến trước dòng `signal_event = ...`, `HAL_GPIO_EXTI_Callback` thấy `NULL` và **bỏ qua im lặng** (xem `if(signal_event != NULL)`). Cờ `EXTI_PR` vẫn được xóa, nên sự kiện đó mất hẳn.
- **Vì sao timer cần đăng ký trước khi chạy.** `[SOURCE]` `BSP_TIM_Start(TIMER_3,1)` nằm ngay sau `km_it_init()` ở [entry.c:66](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/PS-CPU/pscpu_s800/main/App/entry.c#L66). Thứ tự này đảm bảo `SignalHandler` đã có trước khi tick đầu tiên đến.
- **Hạn chế của `ei_ref`.** `[INFERENCE]` `km_enable_irq` giảm bộ đếm không kiểm tra chặn dưới. Nếu gọi enable nhiều hơn disable một lần, `ei_ref` thành âm và điều kiện `== 0` không bao giờ đúng trở lại, khiến ngắt không được bật lại. Tôi thấy các lời gọi trong code đều đi theo cặp (`get_pending_factor`/`put_pending_factor`), nên điều này chưa xảy ra, nhưng đó là điểm dễ vỡ nếu ai thêm lời gọi.
- **Chưa kiểm tra.** Tôi chưa mở thân hàm `HAL_GPIO_EXTI_Callback` bản `__weak` của HAL để xác nhận nó bị ghi đè bởi bản BSP (thực tế suy ra từ việc cả hai cùng tên và cùng chữ ký). Vị trí bit `UIF` (`SR` bit0) và `UIE` (`DIER` bit0) là kiến thức reference manual, tôi không đọc từ repo.