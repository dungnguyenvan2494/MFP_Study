![[Pasted image 20260819153058.png]]\
## Nơi định nghĩa các chân GPIO này

Toàn bộ cặp macro `<TênTínHiệu>_Pin` / `<TênTínHiệu>_GPIO_Port` trong bảng của bạn được định nghĩa tại:

**[mxconstants.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Inc/mxconstants.h)** — do STM32CubeMX sinh tự động từ file project **[PowerSubCPU.ioc](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/PowerSubCPU.ioc)** (đây mới là "nguồn sự thật" gốc — nếu cần đổi chân, phải sửa trong CubeMX rồi generate lại, không nên sửa tay file này).

Đối chiếu từng dòng của bạn với file:

|Chân MCU|Macro trong [mxconstants.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Inc/mxconstants.h)|Dòng|
|---|---|---|
|PC13|`WAKEUP_Pin` / `WAKEUP_GPIO_Port` (GPIOC)|44-45|
|PA1|`_HRESET_REQ_Pin` (GPIOA)|50-51|
|PA2|`AP_PWR_EN_Pin` (GPIOA)|52-53|
|PA4|`POWER_MONITOR_Pin` (GPIOA)|54-55|
|PA5|`MSW_ON_Pin` (GPIOA)|56-57|
|PB0|`IR_P_ON_Pin` (GPIOB)|60-61|
|PB1|`ERP_SENSOR_ON_Pin` (GPIOB)|62-63|
|PB2|`_RESET_Pin` (GPIOB)|64-65|
|PB10|`_USB2_OE_Pin` (GPIOB)|66-67|
|PB13|`MC_PWR_EN_Pin` (GPIOB)|70-71|
|PA8|`MONI_24V11_Pin` (GPIOA)|76-77|
|PA9 / PA10|`PA9_IN_Pin` / `PA10_IN_Pin` (GPIOA)|78-81|
|PA11|`MC_PG_Pin` (GPIOA)|82-83|
|PA13|`TMS_Pin` (GPIOA)|84-85|
|PA15|`MC3_3VON_MONI_Pin` (GPIOA)|86-87|
|PB3|`SB_PWR_EN_Pin` (GPIOB)|88-89|
|PB4|`SB_PG_Pin` (GPIOB)|90-91|
|PB5|`MC_P_ON_Pin` (GPIOB)|92-93|
|PB8|`_DISCHG_Pin` (GPIOB)|94-95|
|PB9|`_RST_SLP2_Pin` (GPIOB)|96-97|
## Nơi cấu hình mode thực tế (input/output/EXTI/AF) cho các chân này

Macro trên chỉ là tên số hiệu chân — chiều/kiểu chân thật sự được thiết lập ở 2 nơi:

1. **Chân GPIO thường (input/output/EXTI)** → [mx_init.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Src/mx_init.c), hàm `MX_GPIO_Init()` ([mx_init.c:419-518](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Src/mx_init.c#L419-L518)):
- `_HRESET_REQ` → `GPIO_MODE_IT_FALLING` (ngắt cạnh xuống)
- `AP_PWR_EN`, `POWER_MONITOR`, `MSW_ON`, `MONI_24V11` → `GPIO_MODE_IT_RISING_FALLING` (ngắt cả 2 cạnh)
- `IR_P_ON`, `ERP_SENSOR_ON`, `_RESET`, `_USB2_OE`, `SLEEP_STATUS_REM_EG`, `MC_PWR_EN`, `SB_PWR_EN`, `MC_P_ON`, `_RST_SLP2` → `GPIO_MODE_OUTPUT_PP`, mặc định RESET (0)
- `MODEL_BIT0/1`, `SB_PG`, `_DISCHG`, `MC_PG`, `MC3_3VON_MONI` → `GPIO_MODE_INPUT` thường (không ngắt)
- `PA9_IN`, `PA10_IN` → `GPIO_MODE_INPUT` + pull-down (dù chân này hỗ trợ chức năng USART1_TX/RX theo datasheet, thiết kế này **không dùng UART**, chỉ để GPIO input dự phòng)
- `PF0_IN`, `PF1_IN` → `GPIO_MODE_INPUT` (dù có khả năng làm `RCC_OSC_IN/OUT` cho thạch anh ngoài, mạch này dùng **HSI nội** nên 2 chân này bỏ trống, cấu hình input thường)

2. **Chân chức năng đặc biệt (Alternate Function)** → [stm32f0xx_hal_msp.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Src/stm32f0xx_hal_msp.c):
- PB6/PB7 (`I2C1_SCL`/`I2C1_SDA`) → cấu hình `GPIO_AF1_I2C1` trong `HAL_I2C_MspInit()` ([stm32f0xx_hal_msp.c:76-88](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Src/stm32f0xx_hal_msp.c#L76-L88)), được gọi tự động từ `MX_I2C1_Init()`
- PC14/PC15 (`OSC32_IN/OUT`) → không phải GPIO, được cấp cho LSE 32.768kHz qua `RCC_OscInitStruct.LSEState = RCC_LSE_ON` trong `SystemClock_Config()` ([mx_init.c:120-122](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Src/mx_init.c#L120-L122)), dùng làm nguồn clock cho RTC
 - PA13/PA14 (`SWDIO`/`SWCLK`) → cổng debug SWD, do phần cứng MCU dành riêng mặc định, không qua `GPIO_Init` của app

## Hai điểm lệch giữa bảng bạn đưa và code hiện tại (đáng lưu ý)

- **PA11**: bảng bạn ghi `SB_PG`, nhưng code định nghĩa là **`MC_PG`** ([mxconstants.h:82-83](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Inc/mxconstants.h#L82-L83)). Tín hiệu `SB_PG` thật sự nằm ở **PB4** ([mxconstants.h:90-91](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Inc/mxconstants.h#L90-L91)). Có thể tài liệu spec của bạn đang dùng số hiệu khác, hoặc đây là chỗ tài liệu/code chưa đồng bộ tên gọi — nên đối chiếu lại với đặc tả điện gốc.
- **PB9**: bảng ghi `/RST_USB`, code định nghĩa là **`_RST_SLP2`**. Về mặt logic thì đúng là cùng một chân được `rst_usb_proc()` điều khiển làm reset USB (xem [km_extend_io.c:711-729](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L711-L729)), nhưng tên biến trong code vẫn giữ nguyên gốc `_RST_SLP2` (có thể là tên lịch sử chưa đổi khi đổi mục đích sử dụng chân).


## Ghi chú thêm

- `WAKEUP_Pin` (PC13) chỉ được định nghĩa trong `mxconstants.h` nhưng **không thấy dùng ở bất kỳ file .c nào** trong `main/App` — có vẻ là chân dự phòng/chưa dùng trong bản build hiện tại.
- File [km_adc.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_adc.c) bạn đang mở: cả hai hàm `adc_moni_5v_start_proc()` và `adc_moni_5v_off_proc()` hiện đang bị bọc trong `#if 0` ([km_adc.c:29-39](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_adc.c#L29-L39)) — tức chức năng đo 5V qua ADC **đã bị vô hiệu hóa** (theo ghi chú "AD変換不要になったため" — "không còn cần chuyển đổi AD nữa"), nên trong bảng pinout của bạn không thấy chân ADC nào là hợp lý.
- Có một bộ `mxconstants.h` **riêng** cho target IAP (bootloader cập nhật FW) tại [iap/Inc/mxconstants.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/iap/Inc/mxconstants.h), chỉ định nghĩa `MSW_ON_Pin` và `TMS_Pin` — vì bootloader IAP chỉ cần tối thiểu, không cần toàn bộ chân sequencing nguồn.

# Tín hiệu input  `MONI_24V11 (PA8)`

**a) Lớp phần cứng/ngắt** — cấu hình `GPIO_MODE_IT_RISING_FALLING` ([mx_init.c:442-446](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Src/mx_init.c#L442-L446)) → mỗi lần đổi mức, ngắt EXTI gọi `km_exti_callback()`:


```c
case MONI_24V11_Pin:                                          // km_it.c:263-266
    level = BSP_GPIO_ReadPin(MONI_24V11_GPIO_Port, MONI_24V11_Pin);
    start_anti_chattering(TYPE_Km_AC_Idx_24V11_MONI, level);   // chỉ khởi động bộ lọc rung, KHÔNG có callback xử lý ngay
```

Khác với MSW/POWER_MONITOR (có `intr_proc` riêng), pin này trong bảng `anti_chattering_info[]` ([km_it.c:43-53](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_it.c#L43-L53)) có `intr_proc = NULL` — nghĩa là nó chỉ chống rung (debounce), còn xử lý thật diễn ra ở vòng lặp chính bằng polling.

```c
	/* 24V11_MONI */
  , {
	    AC_STOP_STATE    					/* status   			        */
	  , GPIO_PIN_RESET  					/* start_level			        */
	  , GPIO_PIN_RESET  					/* prev_level			        */
	  , NULL                   				/* intr_proc                    */
	  , MONI_24V11_GPIO_Port                /* portno				(const) */
	  , MONI_24V11_Pin                      /* pinno				(const) */
	  , TYPE_Km_Timer_Anti_Chattering_24V   /* timer_kind			(const)	*/
	  , KM_ANTI_CHATTER_24V11_MONI          /* decision_time		(const)	*/

```

**b) Vòng lặp chính — `moni_24v_proc()`** ([km_extend_io.c:575-591](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L575-L591)): đây là state machine hiện thực đúng mô tả của bạn — khi `SB_PG=High` và `MONI_24V11=Low`, khởi động timer 500ms (`TYPE_Km_Timer_MONI_24V11_OFF`) trước khi coi là "xả xong".

```c
void moni_24v_proc(void)
{
	if(g_power_on_flg == POWER_ON_FLG_START){	/* 電源ONまたは再起動状態 */
		if((BSP_GPIO_ReadPin(SB_PG_GPIO_Port,SB_PG_Pin) && 
		   (!IS_CHECKING_CHATTERING(TYPE_Km_AC_Idx_24V11_MONI) &&  BSP_GPIO_ReadPin(MONI_24V11_GPIO_Port,MONI_24V11_Pin))) || 
		   (!IS_CHECKING_CHATTERING(TYPE_Km_AC_Idx_POWER_MONI) && !BSP_GPIO_ReadPin(POWER_MONITOR_GPIO_Port,POWER_MONITOR_Pin))){			/* 24V放電確認 */
			g_24v_check_flg_off = STATE_ACTIVE;								/* 24V放電チェック開始 */
		}
	}
	if(g_24v_check_flg_off == STATE_ACTIVE || IS_CHECKING_CHATTERING(TYPE_Km_AC_Idx_POWER_MONI) || IS_CHECKING_CHATTERING(TYPE_Km_AC_Idx_24V11_MONI)){
		if( BSP_GPIO_ReadPin(SB_PG_GPIO_Port,SB_PG_Pin) &&
		   !BSP_GPIO_ReadPin(MONI_24V11_GPIO_Port,MONI_24V11_Pin)){
			km_timer_set(TYPE_Km_Timer_MONI_24V11_OFF,500,TYPE_Km_Timer_Start);	/* 500ms wait */
			g_24v_check_flg_off = STATE_NONACTIVE;
		}
	}
}
```

**c) Chặn cấp nguồn/MC_P_ON khi chưa xả xong** — macro `_MC_P_ON_OK()` / `_MC_PWR_EN_OK()` ([km_extend_io.c:735-743](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L735-L743)) bắt buộc `MONI_24V11=Low` mới cho phép `mc_p_on_proc()` ([km_extend_io.c:766-826](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L766-L826)) và `mc_pwr_en_proc()` ([km_extend_io.c:1096-1124](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1096-L1124)) thực sự kéo chân `MC_P_ON`/`IR_P_ON`/`MC_PWR_EN` lên High — đúng chính xác yêu cầu "nếu xả chưa xong thì chặn xuất, chờ xả xong mới xuất".

```
#define _MC_P_ON_OK()                                        				            BSP_GPIO_ReadPin(AP_PWR_EN_GPIO_Port,AP_PWR_EN_Pin)  \
      &&  BSP_GPIO_ReadPin(_RESET_GPIO_Port,_RESET_Pin)	\
	  &&   BSP_GPIO_ReadPin(SB_PG_GPIO_Port,SB_PG_Pin)	\
	  &&  (!IS_CHECKING_CHATTERING(TYPE_Km_AC_Idx_24V11_MONI) && !BSP_GPIO_ReadPin(MONI_24V11_GPIO_Port,MONI_24V11_Pin))	\
	  &&  (!IS_CHECKING_CHATTERING(TYPE_Km_AC_Idx_POWER_MONI) &&  BSP_GPIO_ReadPin(POWER_MONITOR_GPIO_Port,POWER_MONITOR_Pin))  \
	&& ((!IS_CHECKING_CHATTERING(TYPE_Km_AC_Idx_MSW)        \
	&&  BSP_GPIO_ReadPin(MSW_ON_GPIO_Port,MSW_ON_Pin))               || TYPE_Km_Timer_Start == km_timer_get_state(TYPE_Km_Timer_PWROFF_WDGTIME))  \
						)
```

**d) Điều kiện bật nguồn S800 lần đầu** — `s800_power_on()` ([km_extend_io.c:607-644](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L607-L644)) yêu cầu `MONI_24V11=Low` mới bật `SB_PWR_EN`; và `sb_reset_proc()` ([km_extend_io.c:1231-1245](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1231-L1245)) cũng yêu cầu điều kiện này trước khi nhả `/RESET` — khớp với "chờ xả hoàn tất rồi mới bật nguồn cho S800".

**e) Xuất ra ngoài qua I2C**: bit `INPUT_EXTEND_24V11_MONI_BIT` trong bảng `io_extend[]` ([km_extend_io.c:59](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L59)) → Linux đọc được (read-only) qua lệnh I2C `EXTEND_INPUT_COMMAND`.

# Tín hiệu input  `SB_PG (PB4 trong code)`

Được dùng làm **điều kiện gate** ở gần như mọi state machine nguồn:

|Hàm|Vai trò của SB_PG|
|---|---|
|`moni_24v_proc()` ([:578,585](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L578))|Chỉ bắt đầu theo dõi xả 24V khi `SB_PG=High` (khối AON đã có nguồn ổn định)|
|`_MC_P_ON_OK()`/`_MC_PWR_EN_OK()` ([:738](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L738))|Bắt buộc `SB_PG=High` mới cho phép bật MC_P_ON/IR_P_ON/MC_PWR_EN|
|`mc_pwr_en_proc()` ([:1100-1101](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1100-L1101))|Cần cả `AP_PWR_EN` và `SB_PG` cùng High mới bật `MC_PWR_EN`|
|`msw_on_proc()` ([:1178,1196,1212](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1178-L1212))|Nhiều nhánh: nếu MSW bật + 24V đã xả + `SB_PG=Low` → System Reset; nếu MSW tắt + `SB_PG=Low` → vào Stop-mode tiết kiệm điện; nếu MSW bật nhưng `SB_PG=Low` → gọi lại `s800_power_on()` (retry vì nguồn SB chưa lên dù đã yêu cầu)|
|`sb_reset_proc()` ([:1235](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1235))|Điều kiện bắt buộc để **nhả `/RESET`** cho S800|
|`sleep_mode()` ([:1395-1404](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1395-L1404))|Cần `SB_PG=High` (cùng MSW ON, AP_PWR_EN Low, không ngắt chờ) để MCU vào chế độ Sleep|

→ Đây chính là tín hiệu "chốt an toàn" trung tâm: PS-CPU không dám thao tác bất kỳ tín hiệu cấp nguồn/reset nào cho tới khi khối AON thực sự báo Power Good.

# Tín hiệu input `MC3.3VON_MONI (PA15)`

Đây là tín hiệu **duy nhất trong 3 tín hiệu này hiện KHÔNG có logic xử lý thật**, đúng như mô tả của bạn:

- Trong `io_extend[]` ([km_extend_io.c:60](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L60)), nó vẫn được liệt kê để **đọc và expose ra I2C** (`EXTEND_INPUT_COMMAND`), nên Linux vẫn đọc được trạng thái High/Low bất cứ lúc nào.
- Nhưng hàm xử lý riêng của nó, `mc3_3von_moni_proc()` ([km_extend_io.c:415-429](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L415-L429)), được gọi **mỗi vòng lặp chính** trong `entry.c` nhưng thân hàm trống rỗng:

```c
void mc3_3von_moni_proc(void)
{
    return;
    /* 2017/05/01 K.hirota 3.3Vモニタで制御する信号が無くなったため処理を削除 */
    /* = "Đã xóa xử lý vì không còn tín hiệu nào cần điều khiển từ giám sát 3.3V" */
}
```

→ Khớp 100% với ghi chú của bạn: PS-CPU chỉ **đọc và giữ chỗ** tín hiệu này (dự phòng cho khối mech-con trong tương lai), chưa có bất kỳ điều khiển/gate nào dựa trên nó ở thời điểm hiện tại.

# Tín hiệu output `IR_P_ON (PB0)`

- **Boot-time**: đặt High cùng lúc với `MC_P_ON` khi timer `TYPE_Km_Timer_BOOT_MC_P_ON` hết hạn ([km_extend_io.c:777-785](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L777-L785)) — không phải "đồng thời với /RESET" theo nghĩa tức thời, mà là **sau khi** `/RESET` đã nhả (điều kiện `_MC_P_ON_OK()` bắt buộc `_RESET=High`) và sau một khoảng trễ (320ms ở Eagle, tính từ khi `MC_PWR_EN` bật).
- **POWER_MONITOR=Low → Low ngay**: [km_extend_io.c:451-452](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L451-L452) — khớp chính xác.
- **Điều khiển theo điện văn (bit0)**: `ir_p_on_proc()` ([km_extend_io.c:862-877](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L862-L877)) — chỉ đặt High khi bit0 được set VÀ đồng thời `AP_PWR_EN`, `/RESET`, `POWER_MONITOR` High, (`MSW_ON` High hoặc đang trong timer chờ tắt nguồn), và `MC_P_ON` đã High.
- **Chờ 24V xả (`MONI_24V11=Low`)**: không kiểm tra trực tiếp trong `ir_p_on_proc()`, nhưng gián tiếp đảm bảo vì hàm yêu cầu `MC_P_ON` đã High trước — mà `MC_P_ON` tự nó đã bị `_MC_P_ON_OK()` chặn cho tới khi `MONI_24V11=Low` ([km_extend_io.c:739](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L739)).

# Tín hiệu output `ERP_SENSOR_ON (PB1)`

Khớp **chính xác 100%** với tài liệu — `erp_sensor_proc()` ([km_extend_io.c:687-698](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L687-L698)):

```c
if( bit1==0 && AP_PWR_EN==Low ) → Low
else                            → High
```

- Tín hiệu ngắt nguồn cho các cảm biến liên quan đến engine (DOOR_OPEN, DEST_EDH, CLOSE).
- Ở trạng thái ErP lot.6 và ở trạng thái SLEEP2 có thiết lập tắt cảm biến, ERP_SENSOR_ON xuất mức Low, ngoài ra xuất mức High.
- Vì PS-CPU không nhận biết trạng thái tiết kiệm điện nên nó phán đoán dựa trên tín hiệu AP_PWR_EN.
	- Khi AP_PWR_EN ở mức Low và điện văn (2ndĐịa chỉ 0x70: PB1) ở mức Low, ERP_SENSOR_ON xuất mức Low.
	- Khi AP_PWR_EN ở mức High hoặc điện văn (2ndĐịa chỉ 0x70: PB1) ở mức High, ERP_SENSOR_ON xuất mức High.
# Tín hiệu `/RESET`
Ba thời điểm điều khiển đúng như tài liệu, nhưng nằm rải ở 3 hàm khác nhau:

- **Ngay sau khởi động PS-CPU**: [entry.c:74](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/entry.c#L74) — set Low tường minh trước cả `s800_power_on()`.
- **Nhả reset sau khi SB_PWR_EN High**: `sb_reset_proc()` ([km_extend_io.c:1231-1245](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1231-L1245)) — chỉ set High khi `MSW_ON`, `!MONI_24V11`, `SB_PG` đều tốt VÀ timer 100ms (`TYPE_Km_Timer_SB_RESET_RELEASE`, khởi động trong `s800_power_on()`) đã hết.
- **`/HRESET_REQ=Low` → Reset ngay**: `hreset_req_proc()` ([km_extend_io.c:489-500](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L489-L500)) gọi `s800_power_off()`, trong đó set `/RESET=Low` ([km_extend_io.c:543](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L543)).

# Tín hiệu `SB_PWR_EN (PB3)`

- **MSW_ON=High → High**: qua `s800_power_on()` ([km_extend_io.c:607-644](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L607-L644)), điều kiện gồm `MSW_ON`, `POWER_MONITOR=High`, `MONI_24V11=Low`.
- **/HRESET_REQ=Low → Low**: qua `s800_power_off()` ([km_extend_io.c:546](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L546)).
- **Chờ 0.5s sau khi 24V xả xong**: cơ chế này nằm ở `moni_24v_proc()` ([km_extend_io.c:575-591](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L575-L591)) — set timer `TYPE_Km_Timer_MONI_24V11_OFF` 500ms khi phát hiện xả xong; và `power_monitor_proc()` chỉ gọi `s800_power_on()` sau khi timer đó `End` ([km_extend_io.c:467-472](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L467-L472)). Lưu ý: đường gọi `s800_power_on()` trực tiếp từ `entry.c`/`msw_on_proc()` chỉ kiểm tra mức tức thời của `MONI_24V11` chứ không tường minh chờ đủ 500ms — độ trễ 500ms được đảm bảo chủ yếu qua nhánh `power_monitor_proc()`.

Tín hiệu điều khiển nguồn cho khối AON của S800.
Khi phát hiện `MSW_ON` ở mức High, xuất `SB_PWR_EN` High.
Khi phát hiện `/HRESET_REQ` ở mức Low, xuất `SB_PWR_EN` Low.
Tuy nhiên, là hành vi chung cho cả hai điều kiện kích hoạt trên, nếu `MONI_24V11` đang ở mức High thì chờ `MONI_24V11` xuống Low (hoàn tất xả 24V), `MONI_24V11` Low rồi 0.5 giây sau mới xuất `SB_PWR_EN` High.


# Tín hiệu `MC_P_ON (PB5)`

Cấu trúc giống hệt `IR_P_ON` nhưng tách theo dòng máy qua bảng con trỏ hàm `mc_p_on_proc()` ([km_extend_io.c:839-843](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L839-L843)):

- **Eagle** — `mc_p_on_eagle_proc()` ([km_extend_io.c:763-789](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L763-L789)): điều khiển theo điện văn bit5 khi ở trạng thái NORMAL; boot lần đầu set High khi timer `BOOT_MC_P_ON` (320ms sau `MC_PWR_EN`) hết hạn.
- **Sparrow** — `mc_p_on_sparrow_proc()` ([km_extend_io.c:801-827](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L801-L827)): dùng thêm timer `MC_P_ON_AFTER_SLEEP_STATUS_REM_SP`.
- **POWER_MONITOR=Low → Low ngay**: [km_extend_io.c:449-450](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L449-L450).
- **Chờ `MONI_24V11=Low`**: đảm bảo bởi macro `_MC_P_ON_OK()` ([km_extend_io.c:735-742](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L735-L742)) — khớp đúng tài liệu.


> [!NOTE] Giải thích
> - Tín hiệu điều khiển relay nguồn cho MC (khối MechaCon).
> - Khi khởi động, đồng thời với `/RESET`, đặt `MC_P_ON` lên High
> - Khi phát hiện `POWER_MONITOR` ở mức Low, lập tức đặt `MC_P_ON` xuống Low.
> - Ngoài hai trường hợp trên, điều khiển theo điện văn (2ndĐịa chỉ 0x70: PB5).
> - Tuy nhiên, khi đặt lên High, `24V11_MONI` chờ mức Low (trạng thái đã xả 24V) rồi mới đặt lên High.


# Tín hiệu `/DISCHG (PB8)`

Đúng như tài liệu ghi **"Không có kế hoạch sử dụng"** — grep toàn bộ source chỉ thấy **một lần duy nhất** chân này được ghi, trong `extend_output_init()`:

```c
BSP_GPIO_WritePin(_DISCHG_GPIO_Port, _DISCHG_Pin, GPIO_PIN_RESET);   // km_extend_io.c:409
```

Ngoài ra không có hàm `_proc()` riêng, không đọc/ghi qua I2C — chân này bị **giữ cố định ở mức Low vĩnh viễn**, hoàn toàn chưa được kích hoạt chức năng.


# Tín hiệu `/RST_SLP2 (PB9)` — tên gọi thực tế trong code là "RST_USB"

⚠️ **Sai lệch cần lưu ý**: tài liệu bạn đưa ghi điện văn điều khiển tín hiệu này ở **"2ndĐịa chỉ 0x70: PB5"**, nhưng theo định nghĩa bit thực tế ([km_extend_io.h:36](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Inc/km_extend_io.h#L36)): `OUTPUT_EXTEND_RST_SLP2_BIT = 9`. Bit5 đã là của `MC_P_ON` (mục 5) — nhiều khả năng đây là lỗi copy-paste trong tài liệu gốc, bit đúng phải là **PB9**.

Logic thực tế — `rst_usb_proc()` ([km_extend_io.c:711-729](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L711-L729)):

```c
if(AP_PWR_EN==High && bit9==1) → /RST_SLP2 = High
else                           → /RST_SLP2 = Low
```

Khớp đúng nội dung mô tả (chỉ khác số bit).


> [!NOTE] Giải thích
> - Tín hiệu reset cho các linh kiện ngoại vi của AP.
> - Vì các ngoại vi của AP không có reset từ bên ngoài nên PS-CPU thực hiện việc reset các linh kiện ngoại vi của AP.
> - Điện văn (2ndĐịa chỉ 0x70: PB5) ở mức Low hoặc khi phát hiện `AP_PWR_EN` Low, xuất `/RST_SLP2` ở mức Low.
> - Điện văn (2ndĐịa chỉ 0x70: PB5) ở mức High và `AP_PWR_EN` High thì xuất `/RST_USB` High.

# Tín hiệu `/USB2_OE (PB10)`

Tài liệu ghi "T.B.D" nhưng code **đã có hành vi mặc định thực tế** — `usb2_oe_proc()` ([km_extend_io.c:1373-1382](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1373-L1382)):

- Mặc định: `/USB2_OE` **tự động bám nghịch đảo** theo `/RST_SLP2` (RST_SLP2 High → USB2_OE Low; RST_SLP2 Low → USB2_OE High).
- Nếu Linux từng ghi trực tiếp bit này qua `EXTEND_OUTPUT_HIGH/LOW_COMMAND`, cờ nội bộ `is_usb2_oe_from_main_cpu_ctrl` được set ([km_extend_io.c:296-306](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L296-L306)) → từ đó `usb2_oe_proc()` **ngừng tự động điều khiển**, để quyền hoàn toàn cho S800.

Đặc tả chưa xác định (T.B.D *Sẽ được mô tả sau khi đặc tả xử lý proxy cho wireless LAN được quyết định(9/Edự kiến))

# Tín hiệu `MC_PWR_EN (PB13)`

`mc_pwr_en_proc()` ([km_extend_io.c:1096-1124](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1096-L1124)):

- Mức output thực tế phụ thuộc `AP_PWR_EN && SB_PG` (không kiểm tra `MONI_24V11` trực tiếp ở bước xuất mức).
- Điều kiện `MONI_24V11=Low` được đảm bảo **gián tiếp** qua `_MC_PWR_EN_OK()` (bí danh của `_MC_P_ON_OK()`) — chỉ dùng khi PS-CPU tự set bit này lúc boot (nhánh Eagle, dòng 1113-1121); còn khi tắt nguồn thì bit này bị xoá về 0 thông qua `extend_output_init()` (giá trị mặc định bit12=0) trong `s800_power_off()`.
- **`MC_PWR_EN` và `MC_P_ON` cùng lúc**: đúng — cả hai đều được set trong cùng khối code khi timer `BOOT_MC_P_ON` hết hạn ([km_extend_io.c:781-784](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L781-L784)).


> [!NOTE] Giải thích
> - Tín hiệu điều khiển nguồn cho các IC xung quanh MC (khối MechaCon).
> - Khi phát hiện `MONI_24V11` ở mức Low, xuất `MC_PWR_EN` Low.
> - `MC_PWR_EN` High được xuất ra đồng thời với `MC_P_ON` High.

# Tín hiệu `WAKEUP (PC13)`

Khớp **hoàn toàn** với mô tả — xác nhận qua [PowerSubCPU.ioc:191-195](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/PowerSubCPU.ioc#L191-L195):

```
PC13.Mode=AlarmA_RoutedAF
PC13.Signal=RTC_OUT_ALARM
```

Chân này được CubeMX cấu hình là **Alternate Function nối cứng phần cứng tới đầu ra Alarm A của khối RTC** (`hrtc.Init.OutPut = RTC_OUTPUT_ALARMA` tại [mx_init.c:252](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Src/mx_init.c#L252)), **không hề xuất hiện lệnh `BSP_GPIO_WritePin`/`HAL_GPIO_Init` nào** cho `WAKEUP_Pin` trong toàn bộ `main/App` hay `MX_GPIO_Init()`. Đúng như comment lịch sử để lại trong code (2016/10/13, 2017/05/01): _"Alerm割り込みがそのままWAKEUPとして出力されるためI/Oで制御しない"_ = "vì ngắt Alarm được xuất trực tiếp ra làm WAKEUP nên không điều khiển qua I/O". Việc set thời gian báo thức (`RTC_ALRMAR`) được S800 ghi trực tiếp qua các lệnh I2C `RTC_ALRMAR_CMD` (đã phân tích ở phần I2C trước) — khớp với "Phía S800 sẽ trực tiếp điều khiển RTC".


> [!NOTE] Giải thích
> - Tín hiệu khởi động do PS-CPU phát ra, gửi tới AP của S800.
> - Khi nhận được ngắt từ RTC tích hợp trong PS-CPU, WAKEUP High được xuất ra.
> - Vì tín hiệu này được nối trực tiếp tới ngắt RTC Alarm tích hợp trong PS-CPU nên phần mềm PS-CPU không can thiệp.
> - Phía S800 sẽ trực tiếp điều khiển RTC.
> - *Các ngắt Wakeup khác trong S72/73 do LPPP điều khiển.

# Tín hiệu `AP_PWR_EN`
## 1. Định nghĩa & cấu hình phần cứng

- Chân **PA2**, macro `AP_PWR_EN_Pin`/`AP_PWR_EN_GPIO_Port` ([mxconstants.h:52-53](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Inc/mxconstants.h#L52-L53))
- Cấu hình `GPIO_MODE_IT_RISING_FALLING` — ngắt cả 2 cạnh ([mx_init.c:442-446](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/Src/mx_init.c#L442-L446))
- Cũng được liệt kê trong bảng `io_extend[]` với bit `INPUT_EXTEND_AP_PWR_EN_BIT` ([km_extend_io.c:63](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L63)) → **Linux/LPPP có thể tự đọc lại trạng thái chân này qua I2C** (`EXTEND_INPUT_COMMAND`) để xác nhận.

## 2. Đường xử lý ngắt

Khác với `MSW_ON`/`POWER_MONITOR`/`MONI_24V11` (được lọc rung qua `start_anti_chattering()`), `AP_PWR_EN` đi thẳng qua cơ chế **pending-factor** ([km_it.c:256-257](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_it.c#L256-L257)):

```c
case AP_PWR_EN_Pin:
    set_pending_factor_bit(TYPE_Km_PFB_AP_PWR_EN);
```

rồi được tiêu thụ ở vòng lặp chính qua `intr_pending_proc()` ([km_extend_io.c:1416-1426](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1416-L1426)) → gọi `ap_power_en_proc(STATE_INT)`.

## 3. Hàm trung tâm — `ap_power_en_proc()` ([km_extend_io.c:658-674](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L658-L674))

Đây là nơi **thiết lập trạng thái CA72** (`set_ca72_status()`) mà rất nhiều module khác trong PS-CPU đọc lại:

```c
STATE_INT (từ ngắt, khi AP_PWR_EN vừa đổi mức):
    rst_usb_proc(STATE_INT);              // xử lý luôn /RST_SLP2
    g_first_power_on = NOMAL;             // đánh dấu đã qua pha khởi động đầu
    set_ca72_status(CA72_ON);             // AP_PWR_EN đổi mức ⇒ coi là CA72 đã "sống"

STATE_NORMAL (gọi mỗi vòng lặp, từ entry.c):
    if AP_PWR_EN == Low:
        set_ca72_status(CA72_SLEEP);      // AP đang ở DeepSleep/ErP
```

Trạng thái `CA72_ON`/`CA72_SLEEP`/`CA72_OFF` này chính là dữ liệu nền cho `get_ca72_status()` mà chúng ta đã phân tích ở [km_ca72_status.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_ca72_status.c) trước đây.

## 4. Các nơi khác dùng `AP_PWR_EN` để ra quyết định

| Hàm                                                                                                                                                                                      | Vai trò của AP_PWR_EN                                                                                                                                                                                                                                                                                                                                |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `erp_sensor_proc()` ([:690-691](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L690-L691), đúng đoạn bạn chọn)          | `ERP_SENSOR_ON=Low` chỉ khi bit điện văn=0 **và** `AP_PWR_EN=Low` — đây là phần bạn đã có trong tài liệu                                                                                                                                                                                                                                             |
| `rst_usb_proc()` ([:711-729](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L711-L729))                                 | `/RST_SLP2` chỉ lên High khi `AP_PWR_EN=High` — tức PS-CPU chỉ nhả reset cho ngoại vi AP sau khi biết AP đã bật                                                                                                                                                                                                                                      |
| `_MC_P_ON_OK()`/`_MC_PWR_EN_OK()` macro ([:735-742](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L735-L742))          | Là **điều kiện đầu tiên** trong macro — `MC_P_ON`, `IR_P_ON`, `MC_PWR_EN` đều **không thể bật nếu `AP_PWR_EN` chưa High** (comment [:759](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L759): "MC/IR_P_ON, MC_PWR_ENはAP_PWR_ENがONになってからONする" = "chỉ bật sau khi AP_PWR_EN đã ON") |
| `ir_p_on_proc()` ([:862-877](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L862-L877))                                 | Điều kiện bắt buộc để duy trì `IR_P_ON=High` ở trạng thái vận hành bình thường                                                                                                                                                                                                                                                                       |
| `sleep_status_rem_eagle/sparrow_proc()` ([:890-1043](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L890-L1043))        | So sánh `prev_status`/`cur_status` (lấy từ `get_ca72_status()`, vốn do `AP_PWR_EN` cập nhật) để phát hiện AP chuyển ON↔SLEEP, từ đó điều khiển `SLEEP_STATUS_REM` — đây là cơ chế DeepSleep/ErP                                                                                                                                                      |
| `mc_pwr_en_proc()` ([:1100-1101](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1100-L1101))                           | `MC_PWR_EN` chỉ High khi `AP_PWR_EN && SB_PG` đều High                                                                                                                                                                                                                                                                                               |
| `msw_on_proc()` ([:1182-1188](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1182-L1188))                              | Nếu tắt công tắc (`MSW_ON→Low`) mà `AP_PWR_EN` vẫn Low (AP chưa kịp bật) và không ở Sleep2/ErP → coi là **lỗi tắt máy giữa lúc đang khởi động** (`PWROFF_FACTOR_BOOTING_OFF`) → cắt nguồn ngay lập tức, không chờ tuần tự tắt mềm                                                                                                                    |
| `sleep_mode()` ([:1399](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L1399))                                          | `AP_PWR_EN=Low` là một trong các điều kiện để **chính PS-CPU** vào chế độ Sleep (STM32 STOP mode) tiết kiệm điện — chỉ ngủ khi biết chắc AP cũng đang ngủ                                                                                                                                                                                            |
| Event log (`THRD_EventRecord_ApLowDet/ApHighDet/...`, [:75-78](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/PS-CPU/pscpu_s800/main/App/km_extend_io.c#L75-L78)) | Ghi log thời điểm chuyển mức của `AP_PWR_EN` phục vụ đo thời gian boot/sleep tự động                                                                                                                                                                                                                                                                 |

## Tóm lại

`AP_PWR_EN` không chỉ đơn thuần là công tắc cho `ERP_SENSOR_ON` như tài liệu bạn trích mô tả (mục 5.2.2.2) — đó chỉ là **một trong hơn 10 nơi** sử dụng tín hiệu này. Về bản chất, đây là **tín hiệu báo hiệu chính từ LPPP/AP806 cho PS-CPU biết "khối AP đang thức hay đang ngủ"**, và toàn bộ chuỗi bật nguồn ngoại vi (`MC_P_ON`, `IR_P_ON`, `MC_PWR_EN`, `/RST_SLP2`), cơ chế DeepSleep/ErP (`SLEEP_STATUS_REM`), cơ chế bảo vệ khi tắt máy giữa chừng, và cả việc PS-CPU tự vào chế độ ngủ — đều phụ thuộc trực tiếp hoặc gián tiếp vào trạng thái của chân này. Tài liệu bạn có khả năng chỉ trích một mục nhỏ (5.2.2.2) trong một đặc tả lớn hơn mô tả đầy đủ các vai trò này.