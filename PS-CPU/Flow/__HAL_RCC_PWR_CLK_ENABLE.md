
```
#define __HAL_RCC_PWR_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);\
                                        tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);\
                                        UNUSED(tmpreg); \
                                      } while(0)
```

1. **`SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);`** Đây là dòng quan trọng nhất: ghi 1 vào bit `PWREN` trong thanh ghi `APB1ENR` của khối **RCC** (Reset and Clock Control — bộ quản lý xung clock của chip). → Hành động này **"cấp điện xung clock"** cho khối **PWR** (Power Controller — khối quản lý nguồn/điện áp, chế độ ngủ của chip). Trên STM32, mọi khối phần cứng (GPIO, I2C, Timer, PWR...) đều **không hoạt động được nếu chưa bật clock cho nó** — giống như một thiết bị điện trong nhà, dù đã cắm dây nhưng cầu dao riêng của nó chưa gạt lên thì vẫn không có điện.
    
2. **`tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);`** Đọc lại ngay bit đó sau khi vừa ghi. Đây là một **"đọc giả" (dummy read)** — đọc xong rồi không dùng giá trị đó làm gì cả.
    
3. **`UNUSED(tmpreg);`** Macro này chỉ để nói với compiler "biến này tôi biết không dùng, đừng cảnh báo (warning) unused variable".
    
4. **Bọc trong `do { ... } while(0)`** Đây là một kỹ thuật thông thường khi viết macro nhiều dòng trong C — để macro này hoạt động an toàn như một statement bình thường (có thể đặt sau `if(...)` mà không bị lỗi cú pháp).
## Tại sao phải đọc lại ngay sau khi ghi (dòng số 2)?

Đây **không phải thừa thãi** — là một _workaround_ bắt buộc trên STM32: việc ghi vào thanh ghi bật clock **không có hiệu lực ngay lập tức** do độ trễ truyền tín hiệu trong phần cứng (clock cần vài cycle để ổn định/lan tới khối đích). Nếu code tiếp theo truy cập ngay vào thanh ghi của khối PWR mà clock chưa kịp "lan" tới, có thể gây lỗi (đọc/ghi sai).

→ Đọc lại thanh ghi đó buộc CPU phải **chờ cho đến khi việc ghi hoàn tất thật sự** (vì đọc một thanh ghi ngoại vi luôn phải đi qua bus, mất một khoảng thời gian cố định), nhờ đó đảm bảo khi dòng code tiếp theo chạy, clock của PWR **đã chắc chắn bật xong**.

## Dùng macro này ở đâu

Macro này luôn được gọi **trước khi** code muốn dùng các chức năng của khối PWR — ví dụ trước khi cấu hình điện áp lõi (`HAL_PWR_...`), vào chế độ Sleep/Stop, truy cập RTC backup domain (ghi `PWR->CR` để mở khóa `DBP` cho phép ghi RTC)... Nếu quên gọi macro này trước, các lệnh đọc/ghi vào thanh ghi của PWR sẽ không có tác dụng vì khối đó chưa có clock để hoạt động.