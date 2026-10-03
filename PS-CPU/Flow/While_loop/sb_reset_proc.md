`sb_reset_proc()` là bước **thứ hai** trong quy trình 2 bước bật nguồn cho S800 — nó quyết định **khi nào thực sự nhả chân `/RESET`** để S800 bắt đầu chạy, sau khi nguồn đã được bật ở bước trước.

## Điều kiện kiểm tra (4 điều kiện, AND tất cả)

```c
if((!IS_CHECKING_CHATTERING(TYPE_Km_AC_Idx_MSW)        &&  BSP_GPIO_ReadPin(MSW_ON_GPIO_Port,MSW_ON_Pin)) &&
   (!IS_CHECKING_CHATTERING(TYPE_Km_AC_Idx_24V11_MONI) && !BSP_GPIO_ReadPin(MONI_24V11_GPIO_Port,MONI_24V11_Pin)) &&
    BSP_GPIO_ReadPin(SB_PG_GPIO_Port,SB_PG_Pin) &&
    km_timer_get_state(TYPE_Km_Timer_SB_RESET_RELEASE) == TYPE_Km_Timer_End
   ){
```

1. **`MSW_ON` đang HIGH, không trong giai đoạn chống dội** — công tắc nguồn cơ khí vẫn đang ở trạng thái "bật", không phải tín hiệu rung/dội do vừa bấm.
2. **`MONI_24V11` đang LOW (không báo lỗi), không trong giai đoạn chống dội** — nguồn 24V không có vấn đề (nhớ lại: pin này giám sát lỗi 24V, LOW = bình thường).
3. **`SB_PG` (SB Power Good) đang HIGH** — đây là tín hiệu **phản hồi thật từ IC nguồn SB**, xác nhận nguồn SB đã _thực sự_ lên áp ổn định, không chỉ dựa vào việc PS-CPU "đã ra lệnh bật" (`SB_PWR_EN`).
4. **Timer `TYPE_Km_Timer_SB_RESET_RELEASE` đã ở trạng thái `End`** — nghĩa là đã đủ **100ms trôi qua** kể từ lúc `SB_PWR_EN` được set lên 1.

Chỉ khi **cả 4 đều đúng cùng lúc**, hàm mới:

```c
BSP_GPIO_WritePin(_RESET_GPIO_Port,_RESET_Pin,GPIO_PIN_SET);   // NHẢ reset — S800 bắt đầu chạy
km_timer_set(TYPE_Km_Timer_SB_RESET_RELEASE,0,TYPE_Km_Timer_Normal);  // reset lại timer để không lặp lại
setEventRecord(THRD_EventRecord_LPPPStart);   // ghi log sự kiện "LPPP bắt đầu khởi động" (phục vụ đo đạc tự động)
```

## Tại sao phải thực hiện hàm này — ghép với nửa đầu `s800_power_on()`

Đây là nửa sau của một **quy trình bật nguồn 2 bước có trễ**, nửa đầu nằm ở `s800_power_on()`:

```c
// s800_power_on() — km_extend_io.c:607-623
if(MSW_ON && POWER_MONITOR OK && 24V OK) {
    BSP_GPIO_WritePin(SB_PWR_EN_GPIO_Port, SB_PWR_EN_Pin, GPIO_PIN_SET);   // (A) BẬT nguồn SB
    km_timer_set(TYPE_Km_Timer_SB_RESET_RELEASE, 100, TYPE_Km_Timer_Start); // (B) khởi động đếm 100ms
}
```

[km_extend_io.c:607-623](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L607-L623)

- **(A)** chỉ ra lệnh "bật nguồn" — nhưng **nguồn điện không lên áp ổn định ngay lập tức** (tụ lọc cần thời gian nạp, IC nguồn cần thời gian khởi động/soft-start).
- **(B)** đặt hẹn giờ 100ms — một khoảng **thời gian chờ an toàn (power-up delay)** trước khi được phép nhả reset.

`sb_reset_proc()` được gọi **liên tục mỗi vòng lặp** `while(1)` ([entry.c:85](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/PS-CPU/pscpu_s800/main/App/entry.c#L85)) — nó đóng vai trò "người canh giờ", kiểm tra lại mỗi vòng: _đã đủ 100ms chưa, và quan trọng hơn, mọi điều kiện khác vẫn còn đúng không_ (không chỉ dựa vào đã hết giờ).

## Vì sao không nhả reset ngay khi hết 100ms mà vẫn phải check lại cả 3 điều kiện khác?

**Đây chính là lý do tồn tại của hàm này, không thể gộp vào `s800_power_on()`**: giữa lúc bắt đầu đếm 100ms và lúc hết giờ, **tình trạng hệ thống có thể đã thay đổi**:

- Người dùng có thể đã **tắt công tắc nguồn lại** (`MSW_ON` xuống LOW) trong 100ms đó.
- Nguồn 24V có thể **bị lỗi đột ngột** trong 100ms đó.
- Quan trọng nhất: **`SB_PG` có thể vẫn chưa lên** dù đã hết 100ms (ví dụ tụ lọc yếu, mạch nguồn có vấn đề, 100ms chỉ là thời gian _thiết kế kỳ vọng_, không phải _đảm bảo tuyệt đối_) — đây là lý do `SB_PG` được đọc **trực tiếp bằng tín hiệu phản hồi phần cứng thật**, không dựa suy luận "chắc là đã lên áp rồi".

→ Nếu nhả `/RESET` khi nguồn SB **chưa thực sự ổn định** (dù đã hết 100ms theo lý thuyết), S800 sẽ khởi động với nguồn điện không đủ/không ổn định → có thể gây **boot lỗi, treo máy, hoặc hỏng phần cứng** do dòng điện bất thường lúc CPU chính bắt đầu chạy. Việc tách thành một hàm polling riêng, chạy liên tục, cho phép hệ thống **tự động hủy/trì hoãn** việc nhả reset nếu bất kỳ điều kiện an toàn nào không còn đúng, thay vì nhả reset "mù" ngay khi timer hết hạn.

## Vì sao sau khi nhả reset phải reset lại timer về `Normal`?

```c
km_timer_set(TYPE_Km_Timer_SB_RESET_RELEASE,0,TYPE_Km_Timer_Normal);
```

Nếu không làm vậy, timer vẫn ở trạng thái `End` mãi (trạng thái này không tự quay lại `Normal`), khiến **mọi vòng lặp sau đó** điều kiện `#4` luôn đúng → hàm sẽ **ghi `_RESET_Pin=SET` lại liên tục mỗi vòng lặp** (không gây hại vì đã đang SET, nhưng cũng sẽ gọi `setEventRecord(...)` lặp lại vô nghĩa mỗi lần). Đưa về `Normal` biến hành động "nhả reset" thành **one-shot** — chỉ xảy ra đúng 1 lần cho mỗi lần chu kỳ bật nguồn, không lặp lại cho tới khi `s800_power_on()` khởi động lại một chu kỳ 100ms mới.

## `sb_reset_proc()` còn được gọi lại gián tiếp khi nào?

`s800_power_on()` (và do đó gián tiếp kích hoạt lại chu kỳ chờ cho `sb_reset_proc()`) được gọi lại ở các tình huống khôi phục:

- [km_extend_io.c:471](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L471) — sau khi timer `MONI_24V11_OFF` hết hạn (khôi phục sau sự kiện 24V).
- [km_extend_io.c:1213](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1213) — khi phát hiện `SB_PG` bị mất giữa chừng (nguồn SB bị rớt bất ngờ).
- [km_adc.c:73](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/PS-CPU/pscpu_s800/main/App/km_adc.c#L73) — sau một chu kỳ `s800_power_off()` + kiểm tra 5V, bật lại.

→ `sb_reset_proc()` không chỉ phục vụ lần bật nguồn đầu tiên lúc khởi động PS-CPU, mà là **cơ chế dùng lại được cho mọi lần S800 cần được bật/reset lại** trong suốt vòng đời hoạt động (restart sau lỗi nguồn, sau power-cycle...).