# Sequence Diagram — DMA Processing

## N/A — xác nhận vắng mặt, không chỉ đơn thuần bị lược bỏ

Không có kênh DMA nào được bật hoặc sử dụng ở bất kỳ đâu trong cả hai
image firmware:

- Bảng vector ngắt (vector table) có khai báo `DMA1_Channel1_IRQHandler`,
  `DMA1_Channel2_3_IRQHandler`, `DMA1_Channel4_5_IRQHandler`
  (`main/MDK-ARM/startup_stm32f031x6.s:103-105`), nhưng không có handler
  nào trong ba handler này được override trong `stm32f0xx_it.c` ở bất kỳ
  image nào, và cũng không có handler nào từng được bật (NVIC-enabled) ở
  bất cứ đâu trong `main/` hay `iap/`
  (`03_execution_model.md` §6).
- Mọi giao dịch I2C đều dùng các hàm HAL chế độ interrupt
  (`HAL_I2C_Slave_Receive_IT`/`Transmit_IT` thông qua lớp bọc driver
  CMSIS), sao chép byte bằng CPU bên trong `I2C_Slave_ISR_IT()`
  (`stm32f0xx_hal_i2c.c:3432,3457`), chứ không qua DMA.
- Đường xử lý ADC — nơi hợp lý nhất có thể dùng DMA — bản thân nó lại
  hoàn toàn là dead code (`#if 0` trong `km_adc.c`,
  `03_execution_model.md` §6).

Không có sequence diagram nào được tạo ra cho kịch bản này, vì cùng lý do
như ở `03_rtos_startup.md`: việc bịa ra một participant bộ điều khiển/kênh
DMA và một chuỗi message hoàn tất-ngắt (completion-interrupt) không hề tồn
tại trong firmware này sẽ vi phạm quy tắc source-grounding (bám sát mã
nguồn) đã được duy trì xuyên suốt mọi giai đoạn của phân tích này.
