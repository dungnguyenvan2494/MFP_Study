# Phần 24: Giới hạn của source code — cần đọc thêm gì, ở đâu

Source code trả lời được **"phần mềm làm gì với chân"**, nhưng **không** trả lời được **"chân đó nối
với cái gì ở ngoài đời thực"**. Hai câu hỏi này độc lập — một chân cấu hình đúng 100% về phần mềm vẫn
có thể vô dụng hoặc nguy hiểm nếu mạch ngoài không như giả định.

## Những gì source code KHÔNG thể cho bạn biết

| Câu hỏi | Vì sao source code không đủ | Cần tìm ở đâu |
|---|---|---|
| `MC_P_ON_Pin` thực sự cấp nguồn cho IC/module nào, công suất bao nhiêu? | Code chỉ thấy "set 1 bit lên HIGH" — không biết bên ngoài là gì | **Schematic** (sơ đồ nguyên lý) |
| `SB_PG_Pin` đọc trạng thái "Power Good" từ IC nguồn nào, ngưỡng điện áp bao nhiêu? | Tên biến chỉ là suy luận (`[INFERENCE]`), không phải fact | **Schematic** + datasheet IC nguồn tương ứng |
| Ngưỡng điện áp HIGH/LOW chính xác của input (Vth) | Đây là đặc tính **analog/electrical**, không nằm trong C code | **Datasheet điện** (Electrical Characteristics) của STM32F031C6 |
| Dòng điện tối đa 1 chân GPIO chịu được (output current) | Không có trong HAL — chỉ có logic 0/1 | **Datasheet** phần "I/O current characteristics" |
| Vị trí chân vật lý trên package thật (chân số mấy trên LQFP48) | `GPIO_PIN_x` chỉ là số thứ tự logic trong Port, không phải số chân package | **Pinout diagram** trong Datasheet, hoặc Schematic |
| Có linh kiện pull-up/pull-down **ngoài** gắn thêm trên board không | Code chỉ biết pull-resistor **nội** (`PUPDR`), không biết gì về linh kiện rời trên board | **Schematic** / **Board design (PCB layout)** |
| `MODEL_BIT0/1` thực sự map ra giá trị model nào (Eagle=? Sparrow=?) | Chỉ đọc 2 bit input — ý nghĩa số nhị phân cụ thể nằm ở chỗ khác | Hàm `get_Generation()`/tương tự (cần đọc thêm file khác), hoặc Schematic (xem bit này nối cứng/strap thế nào trên từng biến thể board) |
| Reference Manual (RM0091 cho STM32F0x1) | CMSIS header chỉ có địa chỉ/bit — không giải thích timing, side-effect chi tiết của từng bit | Tài liệu **Reference Manual** chính thức của ST (PDF) |

## Nên đọc file nào để trả lời câu hỏi nào — bản đồ tra cứu

| Muốn biết... | Đọc file này trong repo |
|---|---|
| Ý nghĩa chức năng từng signal (do code xử lý ra sao) | `pscpu_s800/main/App/km_extend_io.c`, `km_it.c` (đã đọc nhiều trong bộ tài liệu này) |
| Cấu hình GPIO ban đầu | `pscpu_s800/main/Src/mx_init.c` → `MX_GPIO_Init()` |
| Định nghĩa macro Pin/Port | `pscpu_s800/main/Inc/mxconstants.h` |
| Cách HAL dịch cấu hình thành register | `pscpu_s800/common/Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c` |
| Định nghĩa register/bit thật của chip | `pscpu_s800/common/Drivers/CMSIS/Device/ST/STM32F0xx/Include/stm32f031x6.h` |
| Tên hàm ISR nào tồn tại, map vào IRQn nào | `pscpu_s800/common/Drivers/CMSIS/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f031x6.s` (chưa đọc trong bộ tài liệu này — có thể hỏi tiếp) |
| ISR thật của project override gì | `pscpu_s800/main/Src/stm32f0xx_it.c` |
| MCU chính xác đang dùng | `pscpu_s800/main/PowerSubCPU.ioc` |
| **Chân nối với linh kiện gì bên ngoài** | **Schematic PDF** — không có trong các thư mục code đã đọc, cần hỏi bạn hoặc tìm file `.pdf`/`.SchDoc`/`.brd` nếu có trong máy |
| **Ý nghĩa điện của HIGH/LOW, dòng điện tối đa** | **Datasheet STM32F031C6Tx** (file PDF chính thức của ST, tải từ st.com hoặc đã lưu sẵn trong máy) |
| **Chi tiết hành vi từng bit RCC/GPIO/EXTI (timing, side-effect)** | **Reference Manual RM0091** (PDF chính thức của ST cho dòng STM32F0x1/F0x2/F0x8) |

## Lưu ý quan trọng

Toàn bộ 12 file trong bộ tài liệu này (00-12) được xây dựng **chỉ từ source code** — mọi tag
`[INFERENCE]` xuất hiện trong các file trước đều là điểm **chưa thể xác nhận tuyệt đối** nếu không có
schematic. Nếu bạn có file schematic (PDF, hoặc file CAD như Altium `.SchDoc`, OrCAD, KiCad...), hãy
cung cấp — tôi có thể đối chiếu lại toàn bộ các suy luận `[INFERENCE]` trong bộ tài liệu này và nâng
cấp chúng thành `[SOURCE]` được xác nhận, hoặc sửa lại nếu suy luận sai.

Tiếp theo (phần cuối): [12_bai_tap.md](12_bai_tap.md).
