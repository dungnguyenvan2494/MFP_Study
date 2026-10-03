# Sequence Diagram — RTOS Startup

## N/A — xác nhận vắng mặt, không chỉ đơn thuần bị lược bỏ

Kịch bản này được yêu cầu bởi mẫu (template) phân tích nhưng không áp dụng
cho firmware này. Không có RTOS nào được liên kết (link) vào bất kỳ image
nào trong hai image, cả Main lẫn IAP:

- Chỉ có `ARM::CMSIS:CORE` được sử dụng, theo danh sách component RTE
  trong file `.uvprojx` (`02_architecture.md`).
- Không có bất kỳ lần xuất hiện nào của `osKernelStart`, `xTaskCreate`,
  hay bất kỳ symbol CMSIS-RTOS/FreeRTOS nào trong toàn bộ `main/` hoặc
  `iap/` (`03_execution_model.md` §8, được xác nhận lại trong
  `08_concurrency.md` §1).
- Cả hai image đều là dạng single-stack, single-thread super-loop
  (`03_execution_model.md` §2.3-§2.4).

Không có sequence diagram nào được tạo ra cho kịch bản này vì việc tạo ra
nó sẽ đòi hỏi phải bịa ra các participant (`Task`, `Scheduler`,
`xTaskCreate`) không hề tồn tại trong source — điều mà quy tắc
source-grounding (bám sát mã nguồn) đã cấm rõ ràng, xuyên suốt mọi giai
đoạn của phân tích này (các quy ước trong
`00_project_scope.md`/`SESSION_CONTEXT.md`).

Thứ thay thế cho việc tạo task RTOS trong firmware này lại được tài liệu
hoá trong `01_boot_sequence.md` (đoạn cuối của `main()`, nơi chương trình
đi thẳng vào vòng lặp `while(1)` super-loop) và
`04_normal_application_flow.md`.
