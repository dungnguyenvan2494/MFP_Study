# Cách LPPP xử lý `event / point / timeout / debugInfo1`

> Source phía LPPP có sẵn trong `lppp_s800`. Cần chốt `SCPI_CHANNEL_BUFFER_SIZE`, vì nó quyết định payload 256 B có vừa không (câu hỏi để ngỏ ở lượt trước).

Phía LPPP nằm ở `lppp_s800/`. Các trường này đi vào hàm `CA72WDT_Ext()` (`ApplicationKM/LPPP_Task.c:723`), được gọi từ nhánh `KM_SCPI_CMD_WDT_REBOOTEXT` (0x8C) trong `process_scpi_cmd()` (`ApplicationMrvl/Test/app_scpi.c`).

## 1. Cách LPPP tách gói 256 byte

LPPP không dùng struct. Nó lấy con trỏ `received_payload = &s_header->payload` rồi ép kiểu từng word 32-bit về 16-bit:

| Linux gửi (u32) | LPPP đọc | Lưu vào |
|---|---|---|
| `event` | `(u16)received_payload[0]` | biến cục bộ |
| `point` | `(u16)received_payload[1]` → `refreshPoint` | `ca72wdte_refreshPoint` |
| `timeout` | `(u16)received_payload[2]` | `ca72wdt_timeout` (sau khi chia 5) |
| `debugInfo1` | `(u16)received_payload[3]` | `ca72wdte_debugInfo1` và `ca72wdte_flag` |
| `debugInfo2[240]` | `(char*)&received_payload[4]` | không lưu, chỉ in log |

## 2. Ý nghĩa của từng trường

### `event`

LPPP hiện thực một **WDT mềm** cho CA72. Nó dùng một task chạy mỗi 5 giây (`ClearWatchDogCounter`) với biến đếm `ca72wdt_timer`.

| `event` | Hành vi |
|---|---|
| `START` (0) | Nếu WDT đang tắt thì bật, đặt `timer=0` và nạp tham số. Nếu đã bật thì **vẫn nạp lại tham số và khởi động lại timer** (sửa ở OP_BTS-44676). |
| `REFRESH` (2) | Chỉ nhận khi WDT đang chạy. Đặt `timer=0`, nạp `timeout` mới, cập nhật `point` và `debugInfo1`. |
| `END` (1) | Chỉ nhận khi WDT đang chạy. Tắt WDT, `timer=0`. |
| giá trị khác | In `## unknown event`. |

`START_SHORT` (3) chỉ tồn tại ở lệnh cũ 0x8B, nên qua đường 0x8C nó bị coi là "unknown".

### `timeout`

`timeout` có đơn vị **giây**. LPPP chia cho 5 (chu kỳ task), và nếu kết quả bằng 0 thì ép thành 1:

```c
timeout = timeout/5;  if (timeout == 0) timeout = 1;
```

Log in lại là `timeout*5000` ms. Điều kiện hết hạn là `ca72wdt_timer >= ca72wdt_timeout`, và `timer++` mỗi 5 giây. Vì REFRESH có thể rơi vào bất kỳ pha nào của chu kỳ 5 giây, thời gian thực tế trước khi hết hạn nằm trong khoảng `timeout×5 s` đến `(timeout+1)×5 s`. Ví dụ `timeout=5` s cho hết hạn sau 5 đến 10 s. Giá trị không chia hết cho 5 sẽ bị làm tròn xuống (ví dụ 12 s thành 10 s).

### `point` (điểm refresh)

`point` là **mã vị trí** cho biết CA72 đã đi tới đâu. Phía Linux dùng các mã như `0x4104` (PCIe link down) và `0x4106` (PCIe link up). LPPP tự khởi động WDT bằng `{START, 0x8000, 5, 0, 0}`, nghĩa là `point=0x8000` và `timeout=5` s. LPPP không diễn giải ý nghĩa của mã này. Nó chỉ in ra và lưu lại, để phục vụ chẩn đoán sau khi WDT nổ.

### `debugInfo1`

`debugInfo1` làm **hai việc**:

1. Được lưu vào `ca72wdte_debugInfo1`, để ghi vào flash khi WDT nổ.
2. **Chỉ ở lệnh START**, nó được copy vào `ca72wdte_flag`. Bit 0 là `CA72_WDT_FLAG_DISABLE_REBOOT`. Khi bit này được bật, WDT nổ cũng **không reset** AP, mà chỉ in thông báo và đặt `ca72wdt_timeout = 99999` để không nổ liên tục. Đây là cờ debug. Vì REFRESH không cập nhật `ca72wdte_flag`, cờ chỉ được chốt ở lần START.

### `debugInfo2` (không được hỏi nhưng đi chung gói)

Chuỗi này chỉ được in ra console debug của LPPP, giữa hai dòng `## d2:` và `## d2 end`. Hàm `isValidDebugInfo2()` coi nó không hợp lệ nếu ký tự đầu là `\0` hoặc nếu không tìm thấy `\0` trong 240 byte.

## 3. Khi WDT hết hạn

`ClearWatchDogCounter` gọi `CA72WDT_Expire()`:

1. `CA72WDT_End()`.
2. `LPPP_SpiFlashWrite_Ca72wdt_Reboot()` (`LPPP_Matrix.c:443`) ghi vào SPI-Flash, vào **cả hai vùng** `BspThrRegion1` và `BspThrRegion2`:
   - byte cờ `0x10` tại offset `0x2e`,
   - 4 byte `(debugInfo1 << 16) | refreshPoint`,
   - 2 byte `(trạng thái nguồn << 8) | uc_pmu_phase_rec`.
3. Nếu không có cờ disable thì kéo GPIO `HRESET_REQN` xuống 0, tức **reset cứng AP**.

Sau khi reboot, Linux đọc lại vùng flash này (liên quan tới `/dev/mtd2` và `THRC_BSPif` ở câu đầu) để biết lần reset trước là do WDT, và nó đang kẹt ở `point` nào. **Chưa trace** phần đọc ngược phía Linux/app.

## 4. Điều kiện bật tính năng (gating)

Lệnh 0x8C chỉ có hiệu lực nếu:

- `getMachineType()` thành công, và
- máy không phải loại cần kiểm tra version, **hoặc** `isFunctionVersion_CA72WdtExt()` trả true (từ IT6_2.1 trở lên).

Nếu không thỏa, LPPP **bỏ qua im lặng**. Nó vẫn trả `SCPI_OK` (`send_response = true`). Phía Linux nhìn thấy `ret = 0, res = 0`, nên không có cách nào biết lệnh bị bỏ qua.

## 5. Xác nhận điểm rủi ro đã nêu lượt trước

Lượt trước nghi gói 256 B có thể vượt vùng shared memory. Đã xác nhận bằng source LPPP (`scpi_api.h:71-75`):

```
SCPI_CHANNEL_BUFFER_SIZE = 128
SCP→AP (kênh 0): 0x7fff0000 .. 0x7fff007F
AP→SCP (kênh 0): 0x7fff0080 .. 0x7fff00FF   ← chỉ 128 B, payload tối đa 120 B
```

Gói `sendData` dài 256 B, đặt tại `0x7fff0088`, kết thúc ở `0x7fff0188`. Nó **tràn sang vùng của kênh 1** (`0x100..0x17F` và hơn nữa). LPPP cũng đọc `debugInfo2` tới offset `0x188` (`memchr(di, 0, 240)`). Cả hai bên cùng tràn nên chạy được: kênh 1 không dùng, và `ioremap` phía Linux map theo trang 4 KB nên không fault. Nhưng đây là hợp đồng ngầm. Nếu sau này kích hoạt kênh thứ hai, hoặc đặt dữ liệu khác tại `0x7fff0100`, `debugInfo2` sẽ đè lên nó.

## 6. Một số điểm khác

- Trong `CA72WDT_Ext()`, nếu lấy `ca72wdt_sem` quá 1 giây thì hàm `return` sớm, tức **bỏ qua** lệnh mà vẫn trả OK. Linux không biết lệnh đã mất. Lệnh REFRESH bị mất có thể làm WDT nổ oan.
- `ca72wdte_flag` chỉ được cập nhật ở START. Gửi REFRESH với `debugInfo1` khác không đổi hành vi reset.
- `xTaskGetTickCountFromISR()` được gọi từ ngữ cảnh task, chỉ để tính log thời gian. Không gây lỗi nhưng không đúng API.

> Chưa chạy hay build LPPP, mọi kết luận ở trên đến từ việc đọc source.

## 7. Đề xuất bước tiếp

- Trace phần Linux đọc lại `CA72WDT_EXTDATA` từ flash sau khi reboot.
- Vẽ sequence diagram cho chuỗi START → REFRESH → EXPIRE → HRESET.
