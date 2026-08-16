# Phần A — `AppMhuTest` thực sự là gì

Bất chấp cái tên và vị trí file trong thư mục `Test/`, **đây là task quan trọng nhất về mặt giao tiếp của cả firmware LPPP**: nó là **cửa ngõ duy nhất** để CA72 (Cortex-A72 chạy ATF + Linux) nói chuyện với LPPP (Cortex-R4 "Quartz" chạy FreeRTOS). Tên `*Test` là di sản từ code mẫu của Marvell — cùng file còn chứa `TestMsgSend`/`TestMsgSendIsr` là helper inject giả lập cho CLI debug.

Được tạo tại [app_init.c:163](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/init/app_init.c#L163):

```c
xTaskCreate(AppMhuTest, "MHU", configMINIMAL_STACK_SIZE /*300 words*/, NULL, tskIDLE_PRIO
```

# Phần B — Nền tảng phần cứng: MHU là gì

**MHU = Message Handling Unit**, khối doorbell/mailbox của ARM tại `MHU_BASE = 0xE8009800` ([regAddrs.h:111](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/a0/include/regAddrs.h#L111)). Nó **không truyền dữ liệu** — chỉ truyền "tiếng chuông" (1 bit interrupt/kênh). Dữ liệu thật nằm trong **shared memory trên DDR**.

## B.1 Hai hướng, đặt tên theo góc nhìn của AP

```c
#define INPUT_PORT  1     // block 1 → thanh ghi SCPI_TX_*  = AP transmit → LPPP đọc
#define OUTPUT_PORT 0     // block 0 → thanh ghi SCPI_RX_*  = AP receive  → LPPP ghi
```

| Thao tác trong code               | Thanh ghi thật       | Ý nghĩa                                  |
| --------------------------------- | -------------------- | ---------------------------------------- |
| `get_MHU_status(INPUT_PORT, &s)`  | `SCPI_TX_INT_STATUS` | bitmap: kênh nào AP vừa gửi lệnh         |
| `clear_MHU(INPUT_PORT, 1<<bit)`   | `SCPI_TX_INT_CLEAR`  | báo "tao đã lấy lệnh kênh này"           |
| `write_MHU(OUTPUT_PORT, 1<<ch)`   | `SCPI_RX_INT_SET`    | rung chuông sang AP: "response sẵn sàng" |
| `get_MHU_status(OUTPUT_PORT, &i)` | `SCPI_RX_INT_STATUS` | đọc lại để biết AP đã lấy chưa           |

Ngắt dùng: `get_mhu_intnum()` → `INTNUM_MHU_tx0` → `INTNUM_APB_SCPI_TX` — tức **ngắt "AP đã transmit"**.

## B.2 Layout shared memory ([scpi_api.h:71-75](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/scpi_api.h#L71-L75))

```
PLAT_CSS_SCP_COM_SHARED_MEM_BASE = 0x7FFF0000     ← phải khớp "scp-shmem" trong Linux DTB
SCPI_CHANNEL_BUFFER_SIZE         = 128

  offset      chiều              macro
  ┌─────────────────────────────────────────────────────────┐
  │ +0x000  SCP → AP   (ch0)   SCPI_CMD_HEADER_SCP_TO_AP_X(0)│  LPPP ghi response
  │ +0x080  AP  → SCP  (ch0)   SCPI_CMD_HEADER_AP_TO_SCP_X(0)│  AP ghi lệnh
  │ +0x100  SCP → AP   (ch1)                                 │
  │ +0x180  AP  → SCP  (ch1)                                 │
  │  …      (5 kênh: 0..4, mỗi kênh 256 byte)                │
  │ +0xFFF0 PLAT_WARMBOOT_FLAG_BASE                          │
  └─────────────────────────────────────────────────────────┘

scpi_header_t = { u8 command_id; u8 sender_if; u16 payload_size; u32 status; u32 payload; }
                  ↑ 8 byte header cố định ────────────────────┘ ↑ payload bắt đầu
```

Comment trong header ghi rõ: _"The following 4 defines are also in atf code at plat/marvell/a8k/quartz. These lines must match"_ — nếu sửa một bên mà không sửa ATF/DTB thì giao tiếp hỏng câm lặng.

# Phần C — Khởi tạo

```c
void AppMhuTest(void *Parameters)
{
    intAttach(get_mhu_intnum(), 0, mhu_handler, 0);   // gắn ISR
    intEnable(get_mhu_intnum());                      // bật ngắt
    while (1) { ... }
}
```

Chỉ 2 dòng. Nhưng **queue `got_msg_qhandle` phải được tạo trước** — việc đó do `LPPP_KM_Init()` → `LPPP_que_create()` → `scpi_msgq_create()` làm, chạy trong `init_thread` prio 3. Vì `AppMhuTest` prio 0, thứ tự luôn đảm bảo.

```c
void scpi_msgq_create(void) {
    got_msg_qhandle = xQueueCreate(SCPI_QUEUE_MSGS /*16*/, sizeof(int_msg_format_t) /*129B*/);
    if (got_msg_qhandle == NULL) { ...; while (1); }   // treo máy nếu hết heap
}
```

16 × 129 ≈ **2KB heap** cho riêng queue này.

# Phần D — `mhu_handler` (ISR) — nửa đầu của luồng

```c
void mhu_handler(uint32_t isr_input)
{
    get_MHU_status(INPUT_PORT, &status);    // ① lấy bitmap kênh
    bit = 0;
    while (status) {                        // ② duyệt từng bit
        if (status & 1) {
            header_source = SCPI_CMD_HEADER_AP_TO_SCP_X(bit);       // ③
            cpu_dcache_invalidate_region(header_source, 32);        // ④ invalidate header
            size = 8 + header_source->payload_size;                 // ⑤ clamp
            if (size > SCPI_CHANNEL_BUFFER_SIZE) size = SCPI_CHANNEL_BUFFER_SIZE;
            if (size < 8)  size = 8;
            if (size > 32) cpu_dcache_invalidate_region(header_source, size);  // ⑥
            memcpy(&msg.scpi_header, header_source, size);          // ⑦ copy ra local
            clear_MHU(INPUT_PORT, 1 << bit);                        // ⑧ ACK cho AP
            msg.channel_number = bit;
            if (uxQueueMessagesWaitingFromISR(got_msg_qhandle) >= 8)             // ⑨
                "Warning : queue near full."
            rc = xQueueSendFromISR(got_msg_qhandle, &msg, &woken);  // ⑩
            if (rc != pdPASS) "Warning : queue full."
            if (woken) { irq/fiq ? portYIELD_FROM_ISR() : taskYIELD(); }
        }
        status >>= 1; bit++;
    }
}
```

**④⑥ Xử lý cache 2 bước — đây là phần tinh tế nhất.** Shared memory nằm trên DDR, cả CA72 lẫn R4 đều truy cập, nhưng R4 có D-cache riêng và **không** coherent với AP. Nên phải:

1. Invalidate 32 byte đầu → ép đọc header mới từ DDR (nếu không sẽ đọc trúng dòng cache cũ của lệnh trước).
2. Đọc `payload_size` từ header vừa mới → tính `size` thật.
3. Nếu `size > 32` thì invalidate tiếp cho đủ phần payload.

Cách chia 2 bước này tránh phải invalidate cả 128 byte mỗi lần cho những lệnh ngắn.

**⑤ Clamp 2 chiều** là bảo vệ chống dữ liệu hỏng từ AP: `payload_size` là `uint16_t` do AP ghi — nếu AP ghi rác thành 0xFFFF thì `size` bị kẹp về 128.

**⑦ Kiểm tra biên buffer:** `msg` là `int_msg_format_t` = `{u8 channel_number; scpi_header_t scpi_header /*12B*/; u8 payload[128-12=116];}`. Ghi từ `&msg.scpi_header` với tối đa 128 byte → vùng khả dụng là 12+116 = **đúng 128 byte**. Vừa khít, không dư một byte nào.

**⑧ `clear_MHU` ngay trong ISR** (chế độ mặc định) → AP được tự do gửi lệnh tiếp lập tức. Đây là lựa chọn "throughput cao, có rủi ro tràn queue".

**⑨ Cảnh báo sớm ở ngưỡng 8/16** — nửa queue. Thêm vào sau bug `AR.1067928` với comment _"do not loop in ISR"_: bản gốc xử lý lệnh ngay trong ISR, gây treo; bản hiện tại chỉ copy + đẩy queue.

# Phần E — Vòng lặp `AppMhuTest` — nửa sau

```c
while (1) {
    ret = xQueueReceive(got_msg_qhandle, &msg, -1);     // ① block vô hạn
    if (ret != pdFALSE) {
        switch (msg.channel_number) {
        case 0: case 1: case 2: case 3: case 4:          // ② chỉ 5 kênh hợp lệ
            tick_cnt1 = xTaskGetTickCountFromISR();      // ③ mốc thời gian
            if (process_scpi_cmd(msg.channel_number, (uint8_t*)&msg.scpi_header)) {
                write_MHU(OUTPUT_PORT, 1 << msg.channel_number);       // ④ rung chuông
                do {                                                    // ⑤ ★ busy-wait
                    get_MHU_status(OUTPUT_PORT, &i);
                } while (i & (1 << msg.channel_number));
                if (msg.scpi_header.command_id == KM_SCPI_CMD_REWRITE_REQUEST) {
                    /*rewrite_roop();*/   // ⑥ đã vô hiệu
                }
            }
            tick_cnt2 = xTaskGetTickCountFromISR();
            if (tick_cnt2 - tick_cnt1 > 50/portTICK_RATE_MS + 1)        // ⑦ = 6 tick
                "Warning!!! scpi : very slow responce. cmd : %d, %d[ms]"
            break;
        default:
            "No protocol attached to channel %d"
            break;
        }
    }
}
```

## ① `-1` = `portMAX_DELAY`

Giống `PowerControlEventManager`: `INCLUDE_vTaskSuspend=1` → task vào Suspended list, **0% CPU** khi rảnh.

## ③⑦ Cơ chế đo hiệu năng SCPI

```
portTICK_RATE_MS = 1000 / configTICK_RATE_HZ = 1000/100 = 10
ngưỡng = KM_SCPI_SLOW_THRESHOLD_TIME/portTICK_RATE_MS + 1 = 50/10 + 1 = 6 tick = 60ms
```

Nếu một lệnh SCPI mất > 60ms, in cảnh báo kèm `command_id`. Rất hữu ích để truy vết: các lệnh gọi `flash_modify_sub` hoặc `LPPP_QoS_Setup*` thường vượt ngưỡng này.

> Chi tiết nhỏ: dùng `xTaskGetTickCountFromISR()` trong **task context** — sai API (đúng phải là `xTaskGetTickCount()`), nhưng trên port này cả hai chỉ đọc `xTickCount` nên vô hại.

## ④⑤ Handshake response — **điểm rủi ro lớn nhất**

`process_scpi_cmd()` trả `send_response`. Nếu `true`, nó đã ghi response vào `SCPI_CMD_HEADER_SCP_TO_AP_X(chan)` và gọi `cpu_dcache_writeback_region(header_dest, 256)` ([app_scpi.c:1030-1031](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L1030-L1031)) để đẩy dữ liệu từ cache R4 xuống DDR cho AP thấy.

Rồi:

```c
write_MHU(OUTPUT_PORT, 1<<ch);        // SCPI_RX_INT_SET  |= 1<<ch  → ngắt sang CA72
do { get_MHU_status(OUTPUT_PORT, &i); } while (i & (1<<ch));   // chờ CA72 clear bit
```

⚠️ **Vòng chờ này không có timeout, không có `vTaskDelay`, không có số lần thử tối đa.** Nếu CA72 treo, đang reset, hoặc chưa boot xong ATF, task này spin mãi.

Hệ quả cụ thể (không phải deadlock toàn hệ thống, nhưng đủ tệ):

- `AppMhuTest` ở prio 0, spin loop không tắt ngắt → tick vẫn chạy, time-slicing vẫn hoạt động → các task prio 0 khác (`PwEvtMgr`, `PwMHUEv`, `ClrWDT`) **vẫn được chạy**, chỉ là mất ~50% băng thông CPU.
- Nhưng `AppMhuTest` **không bao giờ quay lại `xQueueReceive`** → `got_msg_qhandle` (16 slot) đầy dần → `mhu_handler` bắt đầu in _"queue full"_ và **mất lệnh im lặng**.
- Đây chính là lý do tồn tại `CA72WDT` (watchdog phần mềm 90s trong `ClearWatchDogCounter`): khi CA72 chết, nó kéo `HRESET_REQN=0` để reset cứng và thoát khỏi tình trạng này.

## ⑥ `rewrite_roop()` — code chết có chủ đích

Dành cho luồng nâng cấp ROM LPPP: khi flash đang bị ghi đè, CPU không thể fetch lệnh từ chính QSPI, nên phải nhảy vào vòng lặp chạy từ RAM. Đã bị comment với ghi chú _"動作しない且つ使用しないため無効にした"_ (không chạy được và không dùng nên đã vô hiệu).

## ② Vì sao chỉ `case 0..4`?

5 kênh SCPI khớp với 5 cặp buffer trong shared memory. `default` chỉ log — nhưng lưu ý ở nhánh này task **không** gửi response, nên nếu AP dùng nhầm kênh > 4, nó sẽ treo chờ response mãi.

# Phần F — Vị trí trong toàn hệ thống

### Đường đi của một lệnh SCPI

MHU không truyền dữ liệu — nó chỉ rung chuông. Dữ liệu nằm trên DDR dùng chung, và ranh giới thật giữa hai nhân là D-cache của R4. Trang này theo dấu một lệnh từ lúc Linux ghi nó vào shared memory cho tới khi nó chạm vào máy trạng thái nguồn.

```c
5kênh SCPI, mỗi kênh 2 buffer

128 Bmột buffer, header 8 B + payload

16slot queue giữa ISR và task

3tầng queue trước state machine

60 msngưỡng cảnh báo lệnh chậm
```



![[Pasted image 20260816032135.png]]

Một lệnh đi qua **ba tầng queue** trước khi chạm máy trạng thái. Điểm dễ bỏ sót: `send_km_queue()` được gọi **trước cả câu `switch`**, nên mọi lệnh SCPI đều được nhân bản sang KM app — việc lọc `command_id` xảy ra tận tầng `PwMHUEv`. Nhánh `power_cpu()` thì đi tắt, bỏ qua tầng đó.

#### Hình 2 · Vì sao là (2n+1)

![[Pasted image 20260816032305.png]]

Hai chiều **xen kẽ** chứ không tách khối, nên kênh `n` chiếm 256 B liên tiếp và buffer lẻ luôn là hướng AP→SCP. Bốn hằng số này phải khớp chính xác với `plat/marvell/a8k/quartz` trong ATF và `scp-shmem` trong Linux DTB — không có kiểm tra runtime nào bắt được sai lệch.

#### Hình 3 · Đường về, và chỗ nó có thể kẹt

![[Pasted image 20260816032336.png]]

Task chỉ quay lại `xQueueReceive` sau khi AP xoá bit. Vì task chạy ở `tskIDLE_PRIORITY` và vòng lặp không tắt ngắt, hệ thống **không deadlock** — time-slicing vẫn cho các task khác chạy. Nhưng `got_msg_qhandle` đầy dần và `mhu_handler` bắt đầu rơi lệnh trong im lặng. Lối thoát duy nhất là CA72WDT kéo `HRESET_REQN = 0` sau 90 s.

### Tra cứu từng chặng

| Chặng                      | Context  | Việc chính                                                                  | Nguồn           |
| -------------------------- | -------- | --------------------------------------------------------------------------- | --------------- |
| `mhu_handler`              | ISR      | Invalidate cache, đọc header, kẹp `size`, memcpy, `clear_MHU`, đẩy queue    | app_MHU.c:119   |
| `AppMhuTest`               | task · 0 | Nhận queue, gọi `process_scpi_cmd`, đo thời gian, handshake response        | app_MHU.c:206   |
| `process_scpi_cmd`         | task · 0 | Nhân bản sang KM qua `send_km_queue`, rồi `switch` ~30 case                 | app_scpi.c:676  |
| `power_cpu`                | task · 0 | Tắt/bật core CA72; khi core cuối tắt hoặc core đầu bật thì phát event nguồn | app_scpi.c:200  |
| `PowerControlMHUEvent`     | task · 0 | Lọc `0x88`, kiểm tra `payload_size`, dịch sang internal event               | LPPP_Task.c:290 |
| `PowerControlEventManager` | task · 0 | Tra `fpLPPP_ScenarioFunc[state][event]` và chạy scenario                    | LPPP_Task.c:573 |

**Điểm cần nhớ:** `process_scpi_cmd()` gọi `send_km_queue(s_header)` **ngay dòng 698, vô điều kiện, trước cả `switch`** ([app_scpi.c:698](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L698)). Nghĩa là **mọi** lệnh SCPI đều được nhân bản sang KM app; việc lọc `command_id` diễn ra ở tầng `PowerControlMHUEvent`. Đây là cách tách bạch: phần Marvell (`ApplicationMrvl`) không cần biết gì về state machine của KM, và ngược lại.

# Phần G — Hai chế độ flow control: `SCPI_BLOCK_INTR`

```c
//#define SCPI_BLOCK_INTR              ← hiện đang TẮT
#ifdef SCPI_BLOCK_INTR
  #define SCPI_QUEUE_MSGS 4
#else
  #define SCPI_QUEUE_MSGS 16   /* 2018/12/10 AR.1067928 kawamura - not enough queue? */
#endif
```

|                         | **  BẬT** (backpressure)                                  | **TẮT** (hiện tại)                             |
| ----------------------- | --------------------------------------------------------- | ---------------------------------------------- |
| Queue                   | 4 slot                                                    | 16 slot                                        |
| Trong ISR               | `intDisable(mhu)`, **không** clear status                 | `clear_MHU()` ngay                             |
| Hiệu ứng lên AP         | AP thấy bit chưa clear → **bị chặn**, không gửi tiếp được | AP tự do gửi liên tục                          |
| Sau khi task xử lý xong | `clear_MHU()` + `intEnable(mhu)`                          | (không có gì)                                  |
| Đánh đổi                | Không bao giờ mất lệnh, nhưng throughput thấp             | Nhanh, nhưng tràn queue → **mất lệnh im lặng** |

Bug `AR.1067928` cho thấy đã từng tràn thật; nhóm phát triển chọn **tăng queue 4→16** thay vì bật chế độ chặn — ưu tiên tốc độ, chấp nhận rủi ro, bù lại bằng cảnh báo sớm ở ngưỡng 8.

# Phần H — `TestMsgSend` / `TestMsgSendIsr` — công cụ debug

Hai hàm ở đầu file cho phép **inject một message giả** vào `got_msg_qhandle` từ CLI serial (lệnh `mhu` trong [app_cmdproc.c:771-791](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_cmdproc.c#L771-L791)), đồng thời ép `pwr_sd` sang trạng thái mong muốn — dùng để test state machine mà không cần CA72 thật.

⚠️ Cả hai có **lỗi đảo logic**:

```c
bool b_ret = LPPP_SetStatus( pwr_sd, stat );   // trả SYS_OK = 0 khi THÀNH CÔNG
if( false == b_ret ){                          // 0 → false → coi là LỖI
    ApiSysDebug_Printf(-1, -1, "LPPP_SetStatus Error in test\n");
    return;                                    // ← thoát sớm, KHÔNG gửi message
}
```

`LPPP_SetStatus` trả `int` (`SYS_OK = 0` / `SYS_ERROR`), gán vào `bool` → thành công thì `b_ret = false`. Kết quả: hàm **luôn in "Error" và return ngay khi thành công**, chỉ gửi được message khi SetStatus thất bại. Vì chỉ dùng trong CLI debug nên không ảnh hưởng sản phẩm, nhưng ai định dùng lệnh `mhu` để test sẽ thấy nó "không làm gì cả".

---

# Phần I — Tổng kết rủi ro

|Mức|Vấn đề|Chi tiết|
|---|---|---|
|🔴|**Busy-wait không timeout** chờ AP ACK ([app_MHU.c:233-236](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_MHU.c#L233-L236))|CA72 treo → task ngừng phục vụ queue → tràn 16 slot → mất lệnh SCPI. Chỉ thoát được nhờ CA72WDT reset cứng sau 90s.|
|🟠|**Mất lệnh im lặng khi queue đầy**|`xQueueSendFromISR` fail chỉ log warning, không có retry/đếm số lệnh mất.|
|🟠|**`default` không gửi response**|Kênh ngoài 0..4 → AP treo chờ response vô hạn.|
|🟡|**Stack 300 words (1200B)**|`process_scpi_cmd` là hàm ~360 dòng với `switch` ~30 case, gọi xuống flash/I2C/QoS. Rất sát giới hạn.|
|🟡|**Biên `memcpy` vừa khít 128 byte**|Không còn margin; nếu ai sửa `scpi_header_t` mà quên sửa `payload[]` sẽ tràn stack ngay trong ISR.|
|🟡|**Phụ thuộc ngầm vào ATF/DTB**|4 macro shared-memory phải khớp `plat/marvell/a8k/quartz` và `scp-shmem` trong Linux DTB — không có kiểm tra runtime nào.|

# Hàm `process_scpi_cmd`

## Phần A — Vai trò

`process_scpi_cmd()` là **bộ phân giải lệnh SCPI** — trái tim xử lý của toàn bộ giao tiếp CA72 ↔ LPPP. Nó chạy trong context task `AppMhuTest` (prio 0), nhận một lệnh đã được ISR sao chép ra khỏi shared memory, thực thi, ghi response vào buffer trả về, và báo cho caller biết có cần rung chuông sang AP hay không.

```c
bool process_scpi_cmd(uint32_t chan, uint8_t *scpi_cmd);
//   ↑ true = "tao đã soạn response, mày đi rung chuông đi"
//   ↑ false = "không cần response" (hoặc: tao đã tự rung rồi)
```

Giá trị trả về này chính là thứ quyết định `AppMhuTest` có bước vào [busy-wait không timeout](https://claude.ai/code/artifact/274a7285-5d3d-4bac-84c3-0a22a94c3884) hay không — nên nó quan trọng hơn vẻ ngoài của một `bool`.

## Phần B — Bốn con trỏ, và chìa khoá để đọc hiểu cả hàm

Đây là chỗ dễ đọc nhầm nhất. Hàm thao tác trên **hai buffer khác nhau** ở hai vùng nhớ khác nhau:

```c
scpi_header_t *s_header        = (scpi_header_t *)scpi_cmd;      // ① LỆNH ĐẾN
uint32_t      *received_payload = &s_header->payload;            // ② payload của lệnh đến

scpi_header_t *header_dest = SCPI_CMD_HEADER_SCP_TO_AP_X(chan);  // ③ RESPONSE ĐI
uint32_t      *payload     = &header_dest->payload;              // ④ payload của response
```

|                      | Trỏ vào đâu                                   | Ai ghi                       | Ai đọc                         |
| -------------------- | --------------------------------------------- | ---------------------------- | ------------------------------ |
| ① `s_header`         | **stack của task** — bản sao ISR đã memcpy ra | ISR `mhu_handler`            | hàm này                        |
| ② `received_payload` | như trên                                      | ISR                          | hàm này — **tham số của lệnh** |
| ③ `header_dest`      | **DDR dùng chung** `0x7FFF0000 + 2n×128`      | hàm này                      | CA72                           |
| ④ `payload`          | như trên                                      | hàm này — **kết quả trả về** | CA72                           |

Điểm mấu chốt: `s_header` **không** trỏ vào shared memory. ISR đã sao chép ra rồi, nên hàm này đọc từ RAM cục bộ (nhanh, an toàn, không lo AP ghi đè giữa chừng). Chỉ có `header_dest` mới nằm trên vùng chung và mới cần `cpu_dcache_writeback_region`.

Tên biến `payload` và `received_payload` gần giống nhau nhưng **ngược chiều** — và đó chính là nguồn gốc của một nhóm bug ở §F.

## Phần C — Khung xương

```c
bool process_scpi_cmd(uint32_t chan, uint8_t *scpi_cmd)
{
    /* ── 1. PROLOGUE: soạn sẵn header response chung ── */
    header_dest = SCPI_CMD_HEADER_SCP_TO_AP_X(chan);
    payload     = &header_dest->payload;

    header_dest->command_id   = s_header->command_id;   // echo lại
    header_dest->sender_if    = s_header->sender_if;    // echo lại
    header_dest->status       = SCPI_OK;                // lạc quan mặc định
    header_dest->payload_size = 0;                      // case nào cần thì tự set
    //  ⚠ header_dest->payload KHÔNG được xoá

    /* ── 2. FORK: nhân bản sang KM app, vô điều kiện ── */
    send_km_queue(s_header);

    /* ── 3. SWITCH: ~30 case ── */
    switch (s_header->command_id) { ... }

    /* ── 4. EPILOGUE ── */
    udelay(100);
    if (send_response)
        cpu_dcache_writeback_region(header_dest, 256);
    udelay(100);
    return send_response;
}
```

### Bước 2 là điểm kiến trúc quan trọng nhất

`send_km_queue(s_header)` nằm **trước cả câu `switch`**, không có điều kiện nào. Nghĩa là **mọi** lệnh SCPI — kể cả lệnh không liên quan gì tới nguồn, kể cả lệnh không được hỗ trợ — đều bị đẩy sang `gst_MsgQHandle_MHU` cho KM app xem.

Đây là ranh giới trách nhiệm: file `app_scpi.c` (Marvell) không cần biết gì về máy trạng thái nguồn của KM; việc lọc `command_id` xảy ra tận tầng `PowerControlMHUEvent`.

Một chi tiết dễ bỏ sót: queue được tạo với `xQueueCreate(10, sizeof(scpi_header_t))` = **10 × 12 byte**. Nên `send_km_queue` chỉ sao chép được **header 8 byte + đúng 4 byte payload đầu**. Đó là lý do `PowerControlMHUEvent` chỉ đọc được `payload[0]` — mọi tham số dài hơn 4 byte không bao giờ tới được KM app.

### Bước 4: `send_response` điều khiển cache writeback

```c
if (send_response)
    cpu_dcache_writeback_region(header_dest, 256);
```

Chỉ đẩy cache xuống DDR khi thật sự có response. Hợp lý — nhưng lưu ý `256` trong khi `SCPI_CHANNEL_BUFFER_SIZE = 128`. Vì layout xen kẽ, 128 byte thừa chính là buffer AP→SCP của cùng kênh đó. R4 không bao giờ ghi vào vùng ấy nên các dòng cache đó luôn "clean" và writeback là no-op — vô hại trên thực tế, nhưng là con số sai.

## Phần D — Bản đồ ~30 case

Chia theo tác giả và mục đích, sẽ thấy hàm này thực chất là **hai bộ lệnh ghép lại**:

### D.1 Bộ SCPI chuẩn của ARM/Marvell (`0x02` – `0x80`)

|Cmd|Việc|Response|
|---|---|---|
|`0x02`|Get capability — version 1.2, max payload 128/128, bitmap lệnh hỗ trợ|✅ 24 B|
|`0x03`|**Set power state** → `scpi_set_power_state` → `power_cpu`|❌|
|`0x04`|Get power state — trả cờ `suspended`|✅|
|`0x06`|Đặt wake timer (`TimerOpen` + `my_callback`)|❌|
|`0x07`|Huỷ wake timer|❌|
|`0x08`|Số DVFS domain (hard-code = 1)|✅|
|`0x09`|Get DVFS capability của domain|✅|
|`0x0a`|**Set DVFS** → nhánh đặc biệt gọi `pause_cpu`|tuỳ|
|`0x0b`|Get DVFS hiện tại|✅|
|`0x0d`–`0x10`|Clock: số lượng / info / set / get|✅ (trừ `0x0f`)|
|`0x1b`|**Set device power** — có nhánh riêng cho IRIS/PCIe|✅|
|`0x1c`|Get device power|✅|
|`0x80`|Đăng ký wake interrupt (tối đa 10)|❌|

### D.2 Bộ mở rộng của KM (`0x84` – `0x8C`)

|Cmd|Việc|Ảnh hưởng|
|---|---|---|
|`KM_SCPI_CMD_GET_PROTOCOL_VER`|Version protocol KM|—|
|`KM_SCPI_CMD_AP_OFF`|`SCPIAPOff()` → `AP_PWR_EN = 0`|GPIO|
|`KM_SCPI_CMD_SET_AP_STATUS`|**`ap_sd = payload`**|★ điều khiển hành vi `LPPP_TR2b` khi mất điện|
|`KM_SCPI_CMD_GET_AP_STATUS`|Đọc `ap_sd`|—|
|`KM_SCPI_CMD_SET_PWR_STATUS`|**Ghi thẳng `pwr_sd`**|★ ép state machine, bỏ qua transition|
|`KM_SCPI_CMD_GET_PWR_STATUS`|Đọc `pwr_sd`|—|
|`KM_SCPI_CMD_GET_WAKE_INFO`|Nguồn đã đánh thức máy|—|
|`KM_SCPI_CMD_AP_EVENT` (`0x88`)|Đặt biến `intr_event` — **xem §F.1**|⚠|
|`KM_SCPI_CMD_REWRITE_REQUEST`|_Chỉ_ trả response, không làm gì|—|
|`KM_SCPI_CMD_GET_PMU_EVENT`|Sự kiện PMU|—|
|`KM_SCPI_CMD_WDT_REBOOT` (`0x8B`)|CA72WDT Start/End/Refresh/StartShort|watchdog|
|`KM_SCPI_CMD_WDT_REBOOTEXT` (`0x8C`)|CA72WDT mở rộng, có kiểm tra machine type|watchdog|

Hai điều đáng chú ý ở bộ KM:

**`SET_PWR_STATUS` ghi thẳng vào `pwr_sd`** mà không đi qua ma trận transition. Nghĩa là CA72 có thể **ép** trạng thái nguồn của LPPP sang bất kỳ giá trị nào, bỏ qua toàn bộ scenario function. Đây là cửa hậu — hữu ích cho đồng bộ sau resume, nhưng cũng là chỗ hai bên dễ lệch nhau.

**`0x8B` có một luật ngầm phụ thuộc trạng thái:**

```c
else if(payload == KM_SCPI_WD_CA72_END) {
    /* Sleep状態でのWD Endコマンドは受け付けない */
    if(PWR_STATUS_SLEEP1 != LPPP_GetStatus(pwr_sd)) {
        CA72WDT_End();
    }
}
```

Lệnh "tắt watchdog" bị **từ chối im lặng khi đang ở Sleep1** — vẫn trả `SCPI_OK`. Đây là fix của OP_BTS-16528: nếu cho tắt WDT lúc ngủ, CA72 treo trong sleep sẽ không ai cứu.

## Phần E — Ba case đáng phân tích kỹ

### E.1 `case 0x03` — đường dẫn tới máy trạng thái nguồn

```c
case 0x03:
    udelay(100);
    scpi_set_power_state(chan, s_header);   // → power_cpu(cpuid|clusterid<<2, cpupower)
    udelay(100);
    break;                                   // ← send_response VẪN false
```

Không trả response. Linux gọi lệnh này cho **từng core** khi suspend; `power_cpu()` mới là nơi phát event nguồn, và chỉ phát **một lần** khi core cuối cùng tắt (`cpu_state == ALL_CPU_OFF`) hoặc core đầu tiên bật.

Giải mã payload trong `scpi_set_power_state`:

```
bit  3:0  cpuid          bit 15:12  clusterpower
bit  7:4  clusterid       bit 19:16  suspend
bit 11:8  cpupower        ← 3 = tắt, khác = bật
```

### E.2 `case 0x0a` — chỗ LPPP tự đi ngủ

```c
if (domain == 0 && op_point == 1)
    pause_cpu(chan);                 // ← không set send_response
else
    send_response = true;            /* 再開時にレスポンスを返す */
```

`pause_cpu()` ([hal_pause.c:50](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/hal_pause.c#L50)) là nơi **chính con R4 đi ngủ**:

```c
int_state = cpu_disable_interrupts();
intAttach(DUMMY_INT_NUM, ...); intEnable(DUMMY_INT_NUM);
for (i = 0; i < 32; i++) {                    // lưu rồi tắt TOÀN BỘ GIC
    ier_save[i] = GicRegs->ICD_ICER[i];
    GicRegs->ICD_ICER[i] = 0xffffffff;
}
GicRegs->ICD_ISER[DUMMY/32] = 1<<(DUMMY%32);  // chỉ chừa 1 ngắt đánh thức

clear_MHU(0, 1<<chan);    /* cần thiết để chắc chắn kích được ngắt sang CA72 */
write_MHU(0, 1<<chan);    /* trả response TRƯỚC KHI ngủ */

__asm volatile ("wfi;");   // ← R4 dừng tại đây

for (i = 0; i < 32; i++) GicRegs->ICD_ISER[i] = ier_save[i];   // thức: khôi phục
cpu_restore_interrupts(int_state);
```

Đây là lý do `send_response` giữ nguyên `false`: **`pause_cpu` đã tự rung chuông rồi**, nên `AppMhuTest` không được phép rung lần nữa và cũng không được bước vào busy-wait — vì nếu vào, nó sẽ chờ AP xoá bit trong khi chính R4 vừa mới thức dậy từ `wfi`. Thiết kế đúng, nhưng phụ thuộc vào một quy ước ngầm không được ghi ở đâu.

### E.3 `case 0x1b` — nhánh riêng cho IRIS

```c
if (domain == pmu_NB0_iris) {
    err = pmu_iris_actions((op_point & 0xff) == 0 ? POWER_IRIS_PCIE_PU
                                                  : POWER_IRIS_PCIE_PD, domain);
    send_response = true;
    if (err) header_dest->status = SCPI_E_DEVICE;
    break;                                    // ← thoát sớm, không rơi xuống power_set_device
}
err = power_set_device(domain, op_point & 0xff, op_point >> 8);
```

IRIS (khối PCIe) không đi qua `power_set_device` thông thường mà có API riêng — comment ghi _"2018/04/10 Sleep1→Normal時にPCIeのドメインをONにする"_. Bản vá OP_BTS-34391 thêm retry vào bên trong `pmu_iris_actions` vì điều khiển nguồn IRIS đôi khi thất bại.

## Phần F — Bug và điểm đáng ngờ

###  F.1 🔴 `case KM_SCPI_CMD_AP_EVENT` — hiệu ứng bị vứt bỏ

```c
case KM_SCPI_CMD_AP_EVENT :
    if (*((uint32_t*)&s_header->payload) == KM_SCPI_AP_EVENT_GO_TO_S2)
        intr_event = LPPP_INTERNALEVT_GO_TO_S2;
    else if (... == KM_SCPI_AP_EVENT_GO_TO_ERP)
        intr_event = LPPP_INTERNALEVT_GO_TO_ERP;
    send_response = true;
    break;
```

`intr_event` là biến **`static` ở phạm vi file** ([app_scpi.c:127](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L127)) — **cùng biến** mà `power_cpu()` dùng. Nhưng `power_cpu()` ghi đè nó vô điều kiện:

```c
if (g_wolMode) intr_event = LPPP_INTERNALEVT_GO_TO_ERP;
else           intr_event = LPPP_INTERNALEVT_GO_TO_S2;
xQueueSend(gst_MsgQHandle_PowerCtrl, &intr_event, ...);
```

→ Giá trị đặt ở `case 0x88` **không bao giờ được dùng**. Việc chọn Sleep2 hay ErP hoàn toàn do `g_wolMode` (Wake-on-LAN) quyết định.

Kết hợp với việc `PowerControlMHUEvent` đã `#if 0` hai nhánh S2/ERP, kết luận là: **`0x88` với payload = 2 (S2) hoặc 3 (ERP) hoàn toàn vô tác dụng.** Chỉ payload 0 (`WAKEUP_NORMAL`) và 1 (`GO_TO_S1`) mới thật sự đi tới máy trạng thái, qua đường `send_km_queue → PwMHUEv`.

###  F.2 🔴 `case 0x0e` — `return` thẳng, bỏ qua cache writeback

```c
if ((clk_payload->flags = hal_get_clk_flags(clock_id)) == -1) {
    header_dest->status = SCPI_E_RANGE;
    header_dest->payload_size = 0;
    return true;                    // ← BỎ QUA cpu_dcache_writeback_region!
}
```

Đây là lối thoát duy nhất trong hàm không đi qua epilogue. Response lỗi nằm lại trong D-cache của R4, không xuống DDR — nhưng `AppMhuTest` vẫn nhận `true`, vẫn rung chuông, vẫn busy-wait. CA72 đọc buffer trên DDR và thấy **dữ liệu cũ của lệnh trước**. Đáng lẽ phải là `send_response = true; break;`.

###  F.3 🟠 Ba case đọc tham số từ **buffer trả về** thay vì buffer nhận

```c
case 0x9:  pmu_devices domain = (payload[0] & 0xff);      // ← payload = &header_dest->payload
case 0xb:  uint32_t     domain = (payload[0] & 0xff);      // ← SAI, phải là received_payload
case 0x1c: uint16_t     domain = (payload[0] & 0xffff);    // ← SAI
```

Vì prologue **không xoá `header_dest->payload`**, ba case này đọc trúng phần đuôi của response trước đó trên cùng kênh. Đúng phải là `received_payload[0]` — như `case 0x0a` ngay bên cạnh đã làm đúng.

Đáng chú ý: nhóm bug này **chỉ nằm trong các case của Marvell** (DVFS/device). Toàn bộ case `KM_SCPI_CMD_*` đều dùng `s_header->payload` nhất quán và đúng. Nhiều khả năng đây là code mẫu chưa bao giờ được Linux gọi tới thật.

### F.4 🟡 `payload_size` không khớp lượng dữ liệu ghi

|Case|Khai báo|Thực ghi|
|---|---|---|
|`0x02`|24 B|16 B (`payload[0..3]`) — 8 byte cuối là rác|
|`0x04`|2 B|4 B|
|`GET_WAKE_INFO`|1 B|4 B (`payload[0] = ...`)|

Ghi nhiều hơn khai báo thì vô hại (AP chỉ đọc đúng `payload_size`); khai báo nhiều hơn ghi thì AP đọc trúng rác.

###  F.5 🟡 `udelay()` rải khắp nơi

Mỗi `udelay(DEF_DELAY_100)` = **100 µs busy-wait** (`HalChip_wait_us_lower_bound`). Riêng đường `0x03` đã có 4 lần trong hàm này, cộng thêm ~10 lần nữa bên trong `power_cpu()` → **hơn 1,4 ms chỉ để chờ suông** cho một lệnh set power state, chưa tính công việc thật.

Toàn bộ được thêm cùng lúc bởi OP_BTS-20790 (2020/02/25). Đây là dạng vá "thêm delay cho hết lỗi" — hiệu quả nhưng che mất nguyên nhân gốc, và là lý do chính khiến cảnh báo _"scpi : very slow responce"_ (ngưỡng 60 ms) đôi khi kích hoạt.

###  F.6 🟡 Không kiểm tra `payload_size` ở hầu hết các case

Chỉ `case 0x06` kiểm tra (`< 12 → SCPI_E_SIZE`). Các case còn lại đọc thẳng `s_header->payload` mà không xác nhận AP có gửi đủ byte hay không. ISR đã kẹp `size` về `[8…128]` nên không tràn bộ nhớ, nhưng nếu AP gửi lệnh không kèm payload thì hàm đọc trúng vùng chưa khởi tạo trong `msg` trên stack.

## Phần G — Tóm tắt một câu

`process_scpi_cmd` là một **dispatcher hai tầng trách nhiệm**: nó _fork_ mọi lệnh sang KM app trước khi làm bất cứ việc gì khác, rồi tự xử lý phần thuộc về Marvell — và giá trị `bool` nó trả về không phải "thành công hay thất bại" mà là _"caller có phải đi rung chuông và chờ AP hay không"_, một hợp đồng ngầm mà `pause_cpu()` cố tình phá vỡ theo cách đúng đắn.

# Các lệnh spi nhận từ AP

## Phần A — Khung xử lý chung

Mọi lệnh đều đi qua cùng một khung, nên nắm khung trước rồi mới đọc từng case:

```c
/* ① Soạn sẵn header response — mọi case đều thừa hưởng */
header_dest->command_id   = s_header->command_id;   // echo
header_dest->sender_if    = s_header->sender_if;    // echo
header_dest->status       = SCPI_OK;                // lạc quan mặc định
header_dest->payload_size = 0;

/* ② Nhân bản sang KM app — VÔ ĐIỀU KIỆN, trước cả switch */
send_km_queue(s_header);        // chỉ chép được 12 B: header + payload[0]

/* ③ switch (s_header->command_id) { … } */

/* ④ Chỉ writeback cache khi có response */
if (send_response) cpu_dcache_writeback_region(header_dest, 256);
return send_response;
```

Nghĩa là mỗi lệnh có **hai tác dụng song song**: tác dụng riêng trong `switch`, và tác dụng chung là bị đẩy sang `PowerControlMHUEvent` để lọc.

## Phần B — Bảng tổng hợp toàn bộ lệnh

|ID|Tên|Nhóm|Response|Tác động chính|
|---|---|---|---|---|
|`0x02`|Get capability|ARM SCPI|✅|Chỉ đọc — bắt tay version|
|`0x03`|**Set CSS power state**|ARM SCPI|❌|Tắt/bật core CA72 → sinh event nguồn|
|`0x04`|Get CSS power state|ARM SCPI|✅|Chỉ đọc|
|`0x06`|Set wake timer|ARM SCPI|❌|Mở timer phần cứng|
|`0x07`|Cancel wake timer|ARM SCPI|❌|Đóng timer|
|`0x08`|DVFS capabilities|ARM SCPI|✅|Chỉ đọc|
|`0x09`|DVFS get info|ARM SCPI|✅|Chỉ đọc ⚠️|
|`0x0a`|**DVFS set**|ARM SCPI|tuỳ|Có nhánh khiến **R4 tự ngủ**|
|`0x0b`|DVFS get|ARM SCPI|✅|Chỉ đọc ⚠️|
|`0x0d`|Get clocks capability|ARM SCPI|✅|Chỉ đọc|
|`0x0e`|Get clock info|ARM SCPI|✅|Chỉ đọc ⚠️|
|`0x0f`|Set clock rate|ARM SCPI|❌|Đổi tần số clock|
|`0x10`|Get clock rate|ARM SCPI|✅|Chỉ đọc|
|`0x1b`|**Set device power**|ARM SCPI|✅|Bật/tắt power domain, riêng IRIS|
|`0x1c`|Get device power|ARM SCPI|✅|Chỉ đọc ⚠️|
|`0x80`|Configure wake int|mở rộng|❌|Đăng ký ISR đánh thức|
|`0x81`|Get protocol version|KM|✅|Chỉ đọc|
|`0x82`|**AP OFF**|KM|✅|`AP_PWR_EN = 0`|
|`0x83`|**Set AP status**|KM|✅|Ghi `ap_sd`|
|`0x84`|Get AP status|KM|✅|Chỉ đọc|
|`0x85`|**Set power status**|KM|✅|Ghi thẳng `pwr_sd`|
|`0x86`|Get power status|KM|✅|Chỉ đọc|
|`0x87`|Get wake info|KM|✅|Đọc **và xoá**|
|`0x88`|AP event|KM|✅|Đi đường `PwMHUEv` ⚠️|
|`0x89`|Rewrite request|KM|✅|Không làm gì|
|`0x8A`|Get PMU event|KM|✅|**Gỡ ISR nút nguồn**|
|`0x8B`|**WDT reboot**|KM|✅|Điều khiển CA72WDT|
|`0x8C`|WDT reboot ext|KM|✅|CA72WDT có tham số|
|khác|—|—|✅|`SCPI_E_HANDLER`|
# Phần C — Nhóm SCPI chuẩn ARM

## `0x02` — Get capability (bắt tay)

Lệnh đầu tiên Linux gửi sau khi ATF khởi tạo xong. Nó hỏi "mày nói protocol version nào, buffer bao lớn, hỗ trợ lệnh gì".

```c
header_dest->payload_size = 24;
payload[0] = (1 << 16) | 2;                      // SCPI version 1.2
payload[1] = 128 | (128 << 16);                  // max payload AP→SCP = 128, SCP→AP = 128
payload[2] = (2 << 16) | 0;                      // firmware version 2.0
payload[3] = (STD_CMD_SET << 1) | EXT_SET_EN;    // bitmap lệnh + bật extended set
```

`STD_CMD_SET` ([scpi_api.h:64](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/scpi_api.h#L64)) là bitmap khai báo hỗ trợ các lệnh `2, 3, 4, 6, 7, 0xa, 0xb, 0xf, 0x10`. `EXT_SET_EN = 1` báo cho Linux biết có bộ lệnh mở rộng (chính là nhóm KM `0x8x`).

Con số `128` ở đây phải khớp `SCPI_CHANNEL_BUFFER_SIZE` — đây là chỗ duy nhất trong firmware "nói" cho Linux biết kích thước buffer.

## `0x03` — Set CSS power state ★

Lệnh quan trọng nhất về mặt nguồn. Linux/PSCI gửi cho **từng core** khi suspend hoặc resume.

```c
scpi_set_power_state(chan, s_header);
```

Giải mã payload (`scpi_set_power_state`, [app_scpi.c:355](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L355)):

```
bit  3:0   cpuid            bit 15:12  clusterpower
bit  7:4   clusterid        bit 19:16  suspend
bit 11:8   cpupower  ← 3 = tắt, khác = bật
```

rồi gọi `power_cpu(cpuid | (clusterid<<2), cpupower)`.

**Không trả response** — PSCI không chờ. Đây cũng là lý do đường này an toàn: không bao giờ chạm vào busy-wait.

`power_cpu()` mới là nơi phát event, và **chỉ phát một lần**:

|Điều kiện|Event phát ra|
|---|---|
|`cpu_state == ALL_CPU_OFF` sau khi tắt core cuối|`GO_TO_ERP` nếu `g_wolMode` bật, ngược lại `GO_TO_S2`|
|`cpu_state == ALL_CPU_OFF` trước khi bật core đầu, và `pmu_resume` OK|`WAKEUP_S1`|

`g_wolMode` không đến từ SCPI mà từ **host command** của network stack ([app_hostcmd.c:281-297](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Application/hostcmd/app_hostcmd.c#L281-L297)) — `MSG_CMD_SYSTEM_SET_WOL` với các bit magic packet / link act / ARP / unicast. Vậy quyết định "Sleep2 hay ErP" thực chất do **cấu hình Wake-on-LAN** chi phối, không do Linux nói trực tiếp.

Xem [[scpi_set_power_state]] function
## `0x04` — Get CSS power state

```c
payload[0] = suspended << 8 | suspended << 4;
```

`suspended` là `static bool` được đặt `true` trong `power_cpu()` khi core cuối tắt. Không bao giờ được đặt lại `false` ở đâu cả — đáng ngờ nhưng chỉ ảnh hưởng lệnh đọc này.

## `0x06` / `0x07` — Wake timer

Hẹn giờ để LPPP tự đánh thức hệ thống mà không cần sự kiện bên ngoài.

```c
if (s_header->payload_size < 12) { status = SCPI_E_SIZE; ... }   // case DUY NHẤT kiểm tra size
wake_timer = TimerOpen(-1);
TimerConfig->RepCount  = 0;                  // one-shot
TimerConfig->Count     = payload[0];
TimerConfig->eTimebase = e_TIMEBASE_10_MS;   // đơn vị 10 ms
TimerConfig->FuncPtr   = my_callback;
TimerOn(wake_timer);
```

Khi timer nổ, `my_callback` ([app_scpi.c:159](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L159)):

```c
*(volatile uint32_t *)PLAT_WARMBOOT_FLAG_BASE = 0;   // xoá cờ warm boot → boot lạnh
TimerOff(my_timer); TimerClose(my_timer);
wake_system();    // set SYS_EVENT_WAKE_SYSTEM cho system thread
```

`0x07` huỷ timer đang chạy. Cả hai đều **không trả response**.
Xem [[waker timer]]
## `0x08` / `0x09` / `0x0a` / `0x0b` — DVFS

Bốn lệnh quản lý operating point (tần số/điện áp). Bảng `op_points[]` chỉ có **đúng 1 phần tử** ([app_scpi.c:83](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L83)) — DVFS trên nền tảng này gần như là stub.

- **`0x08`**: trả `sizeof(op_points)/sizeof(struct dvfs_s)` = 1.
- **`0x09`**: `memcpy` cả struct `dvfs_s` vào response, `payload_size = 4 + num_points × 8`.
- **`0x0b`**: trả `op_points[domain].current_point`.
- **`0x0a` ★**: đây là case duy nhất có tác dụng thật:

```c
uint32_t domain   =  received_payload[0] & 0xff;
uint32_t op_point = (received_payload[0] & 0xff00) >> 8;
//power_set_device(domain, op_point, 0);      ← đã bị comment
op_points[domain].current_point = op_point;   // chỉ ghi biến

if (domain == 0 && op_point == 1)
    pause_cpu(chan);                          // ★ R4 TỰ ĐI NGỦ
else
    send_response = true;                     /* 再開時にレスポンスを返す */
```

`pause_cpu()` ([hal_pause.c:50](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/hal_pause.c#L50)) là nơi **chính con R4 vào `wfi`**:

```c
int_state = cpu_disable_interrupts();
intAttach(DUMMY_INT_NUM, ...); intEnable(DUMMY_INT_NUM);
for (i = 0; i < 32; i++) {                        // lưu rồi tắt TOÀN BỘ GIC
    ier_save[i] = GicRegs->ICD_ICER[i];
    GicRegs->ICD_ICER[i] = 0xffffffff;
}
GicRegs->ICD_ISER[DUMMY/32] = 1<<(DUMMY%32);      // chỉ chừa 1 ngắt đánh thức

clear_MHU(0, 1<<chan);   /* cần thiết để chắc chắn kích được ngắt sang CA72 */
write_MHU(0, 1<<chan);   /* TỰ trả response TRƯỚC KHI ngủ */

__asm volatile ("wfi;");                          // ← dừng tại đây

for (i = 0; i < 32; i++) GicRegs->ICD_ISER[i] = ier_save[i];
cpu_restore_interrupts(int_state);
```

Đây là lý do `send_response` **cố ý giữ `false`**: `pause_cpu` đã tự rung chuông rồi. Nếu để `true`, `AppMhuTest` sẽ rung lần hai rồi busy-wait ngay sau khi R4 vừa thức dậy. Một hợp đồng ngầm không được ghi ở đâu.

## `0x0d` – `0x10` — Clock

Bốn lệnh truy vấn/đổi clock, uỷ quyền hoàn toàn cho HAL:

|Lệnh|Action|
|---|---|
|`0x0d` Get clocks capability|`hal_get_num_clks(clock_id)` → số clock|
|`0x0e` Get clock info|Điền `get_clock_payload_t {clock_id, flags, min_rate, max_rate}` rồi `strcpy` tên clock nối ngay sau struct; `payload_size = strlen(name) + sizeof(struct)`|
|`0x0f` Set clock rate|`hal_set_current(clock_id, rate)`, rate lấy từ `((uint32_t*)&payload)[1]`; **cố ý `send_response = false`**|
|`0x10` Get clock rate|`header_dest->payload = hal_get_clk_freq(clock_id)`|

## `0x1b` / `0x1c` — Device power domain

**`0x1b`** là lệnh bật/tắt power domain tổng quát, có **nhánh riêng cho IRIS** (khối PCIe/USB/GbE trên chip bridge):

```c
uint16_t domain   = s_header->payload & 0xffff;
uint16_t op_point = s_header->payload >> 16;

if (domain == pmu_NB0_iris) {                          // = 75
    err = pmu_iris_actions((op_point & 0xff) == 0 ? POWER_IRIS_PCIE_PU
                                                  : POWER_IRIS_PCIE_PD, domain);
    send_response = true;
    if (err) header_dest->status = SCPI_E_DEVICE;
    break;                                              // KHÔNG rơi xuống dưới
}
err = power_set_device(domain, op_point & 0xff, op_point >> 8);
```

IRIS nằm ở power domain vật lý khác nên `pmu_lib.h` xếp nó ngoài dải thường (`pmu_device_end = 73`, `pmu_NB0_iris = 75`) và ghi rõ _"this device is NOT handled in same manner"_. Comment `2018/04/10 Sleep1→Normal時にPCIeのドメインをONにする` cho biết nhánh này sinh ra để sửa lỗi PCIe không lên sau khi thoát Sleep1. Bản vá OP_BTS-34391 (2021) thêm retry bên trong `pmu_iris_actions`.

**`0x1c`** đọc lại trạng thái qua `power_get_device`.

## `0x80` — Configure wake interrupt

Cho phép Linux **đăng ký một số hiệu ngắt bất kỳ** làm nguồn đánh thức:

```c
uint16_t wake_int = received_payload[0] & 0xffff;
if (num_wakes >= MAX_WAKE_INTS /*10*/) { "Too many wake interrupts"; break; }
wake_ints[num_wakes++] = wake_int;
intAttach(wake_int, 0, wake_isr, wake_int);
intEnable(wake_int);
```

`wake_isr` ([app_scpi.c:185](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L185)) rất gọn: gỡ **toàn bộ** wake interrupt rồi `wake_system()`. Việc gỡ hết là chủ ý — sau khi đã thức thì các nguồn wake không còn ý nghĩa, và `clear_wake_ints()` cũng được `power_cpu()` gọi lại ở đường resume.

Đặt `payload_size = 2` nhưng `send_response = false` — payload_size thừa, vô hại.

---

# Phần D — Nhóm mở rộng KM (`0x81` – `0x8C`)

Đây là bộ lệnh do Konica Minolta thêm vào, khai báo trong [LPPP_SCPI_Protocol.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_SCPI_Protocol.h). Khác biệt lớn so với nhóm ARM: chúng thao tác vào **trạng thái nội bộ của KM app** chứ không phải thanh ghi PMU.

## `0x81` — Get protocol version

```c
*((uint16_t *)payload) = SCPIGetProtocolVersion();   // = (1<<8)|0 = 0x0100
```

Version **của giao thức KM**, độc lập với version SCPI ở lệnh `0x02`.

## `0x82` — AP OFF

```c
SCPIAPOff();   →   LPPP_GPIO_Write(LPPP_GPIOPIN_AP_PWR_EN, 0);
```

Kéo chân `AP_PWR_EN` (GPIO 65, BANK C) xuống mức thấp — **cắt nguồn khối AP**. Linux tự yêu cầu cắt nguồn của chính mình, nên đây là lệnh cuối cùng nó gửi trước khi biến mất.

## `0x83` / `0x84` — AP status ★

```c
SCPISetAPStatus(*((ap_status_t *)&s_header->payload));   // → LPPP_SetStatus(ap_sd, status)
```

|Giá trị|Ý nghĩa|
|---|---|
|`0` `KM_SCPI_AP_NOT_BOOT`|AP chưa khởi động xong|
|`1` `KM_SCPI_AP_WARP_BOOT` / `SNAP_SHOT_FIN`|Warp!! đã boot xong, hoặc đã chụp snapshot xong|

Đây là **cờ giải giáp** cho `LPPP_TR2b_Sleep2_Or_ErPToNormal`. Khi có sự cố mất điện thoáng qua (`PWR_FLICKER`):

- `ap_sd == NO_BOOT` → LPPP ghi cờ `0x10` vào 2 vùng QSPI-Flash rồi kéo `HRESET_REQN = 0` để reset cứng CA72.
- `ap_sd == WARP_BOOT` → bỏ qua, không reset.

Nói cách khác, **Linux dùng lệnh `0x83` để nói "tao boot xong rồi, đừng reset tao nữa"**. `LPPP_TR5_TR6` đặt lại về `NO_BOOT` mỗi khi vào Sleep2/ErP.

## `0x85` / `0x86` — Power status ★ cửa hậu

```c
SCPISetPowerStatus(*((pwr_status_t *)&s_header->payload));   // → LPPP_SetStatus(pwr_sd, status)
```

Ghi **thẳng** vào `pwr_sd` — biến trạng thái mà máy trạng thái nguồn dùng để tra bảng. Không đi qua `event_handler`, không chạy scenario function nào, không đụng GPIO/flash/QoS.

|Giá trị||
|---|---|
|`0` `PWR_PWR_OFF` · `1` `NORMAL` · `2` `SLEEP1` · `3` `SLEEP2` · `4` `ERP`|khớp `pwr_status_t`|

Đây là cửa hậu đầy đủ: CA72 có thể ép trạng thái LPPP sang bất kỳ giá trị nào. Hữu ích để đồng bộ lại sau resume, nhưng cũng là chỗ hai bên dễ lệch nhau — vì phần cứng không hề thay đổi theo.

## `0x87` — Get wake info (đọc phá huỷ)

```c
payload[0] = SCPIGetWakeInfo();   →   LPPP_GetWakeInfo()
```

```c
uint8_t LPPP_GetWakeInfo(void) {
    uint8_t info = guc_wake_info;
    guc_wake_info = 0;              // ← XOÁ sau khi đọc
    return info;
}
```

**Đọc một lần là mất.** Nếu Linux gọi hai lần, lần thứ hai luôn trả `0`. Cờ được tích luỹ bằng `|=` qua `LPPP_SetWakeInfo()`. Các bit định nghĩa trong protocol header:

|Bit|Ý nghĩa|
|---|---|
|`KM_SCPI_WAKEUP_PACKET_ARRIVED`|Có gói Wake-on-LAN tới|
|`KM_SCPI_SNMPII_PACKET_ARRIVED`|Có gói SNMP tới|

## `0x88` — AP event ⚠️

```c
if (*((uint32_t*)&s_header->payload) == KM_SCPI_AP_EVENT_GO_TO_S2)
    intr_event = LPPP_INTERNALEVT_GO_TO_S2;
else if (... == KM_SCPI_AP_EVENT_GO_TO_ERP)
    intr_event = LPPP_INTERNALEVT_GO_TO_ERP;
send_response = true;
```

Đây là lệnh Linux dùng để **chủ động yêu cầu chuyển trạng thái nguồn**. Nhưng cần phân biệt rõ hai đường:

|payload|Đường đi thực tế|Kết quả|
|---|---|---|
|`0` `WAKEUP_NORMAL`|qua `send_km_queue` → `PwMHUEv` dịch thành `LPPP_INTERNALEVT_WAKEUP_NORMAL`|✅ hoạt động|
|`1` `GO_TO_S1`|qua `send_km_queue` → `PwMHUEv` dịch thành `LPPP_INTERNALEVT_GO_TO_S1`|✅ hoạt động|
|`2` `GO_TO_S2`|code trong case này đặt `intr_event`…|❌ **vô tác dụng**|
|`3` `GO_TO_ERP`|…nhưng bị `power_cpu()` ghi đè|❌ **vô tác dụng**|

Lý do: `intr_event` là biến `static` phạm vi file ([app_scpi.c:127](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L127)) **dùng chung với `power_cpu()`**, mà `power_cpu()` ghi đè vô điều kiện theo `g_wolMode` trước khi gửi queue. Đồng thời hai nhánh S2/ERP tương ứng trong `PowerControlMHUEvent` đã bị `#if 0`. Chi tiết ở §E.1.

## `0x89` — Rewrite request

```c
/* 処理は「レスポンス = ON」のみ */
send_response = true;
```

Lệnh báo trước "tao sắp ghi lại ROM của LPPP". Bản thân case không làm gì; xử lý thật nằm ở `AppMhuTest`:

```c
if (msg.scpi_header.command_id == KM_SCPI_CMD_REWRITE_REQUEST) {
    /*rewrite_roop();*/   /* 動作しない且つ使用しないため無効にした */
}
```

Ý định ban đầu là nhảy vào vòng lặp chạy từ RAM (vì khi ghi QSPI thì CPU không fetch lệnh từ đó được), nhưng đã bị vô hiệu vì "không chạy được và không dùng".

## `0x8A` — Get PMU event ⚠️ có side effect lớn

```c
payload[0] = SCPIGetPmuEvent();
```

```c
uint8_t SCPIGetPmuEvent(void) {
    LPPP_GPIO_HandleMainSW(false);            // ★ SIDE EFFECT
    uint8_t event = 0;
    if (LPPP_get_McWDT_status()) event |= KM_SCPI_PMU_MC_WDT;
    return event;
}
```

Cái tên nói là "đọc sự kiện PMU", nhưng nó còn **gỡ vĩnh viễn ISR của công tắc nguồn chính**:

```c
void LPPP_GPIO_HandleMainSW(bool on) {
    ...
    } else {                        // on == false
        gpio_isr_detach(msw_pin);
        gpio_close(msw_pin);
        msw_pin = NULL;
    }
}
```

Đây là **bàn giao quyền xử lý nút nguồn từ LPPP sang AP**: trước khi Linux boot xong, LPPP tự xử lý nút nguồn (kéo `HRESET_REQN=0` để tắt máy); sau khi Linux sẵn sàng và gọi `0x8A`, LPPP nhả ra để Linux xử lý nút nguồn theo cách của nó. Hợp lý về mặt hệ thống, nhưng đặt trong một hàm tên `Get*` thì rất dễ gây bất ngờ — và không thể gọi lại `0x8A` lần nữa để "hoàn tác".

Bit trả về duy nhất là `KM_SCPI_PMU_MC_WDT` (0x01), lấy từ `LPPP_get_McWDT_status()` — cờ do ISR `LPPP_McWDT_handler` đặt khi Machine Control watchdog nổ ([LPPP_McWDT.c:32](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationKM/LPPP_McWDT.c#L32)).

## `0x8B` — WDT reboot (watchdog mềm cho CA72)

Bốn sub-command đọc từ `payload[0]`:

|Sub|Hằng|Action|
|---|---|---|
|`0`|`WD_CA72_START`|`CA72WDT_Start()` — timeout 90 s|
|`1`|`WD_CA72_END`|`CA72WDT_End()` — **có điều kiện, xem dưới**|
|`2`|`WD_CA72_REFRESH`|`CA72WDT_Refresh()` — reset bộ đếm|
|`3`|`WD_CA72_START_SHORT`|`CA72WDT_Start_Short()` — timeout 10 s|

Luật ngầm ở sub-command `END`:

```c
/* Sleep状態でのWD Endコマンドは受け付けない */
if (PWR_STATUS_SLEEP1 != LPPP_GetStatus(pwr_sd)) {
    CA72WDT_End();
}
```

Lệnh tắt watchdog **bị từ chối khi đang ở Sleep1** — nhưng vẫn trả `SCPI_OK`, Linux không biết lệnh bị bỏ. Đây là fix OP_BTS-16528: nếu cho tắt WDT lúc ngủ, CA72 treo trong sleep sẽ không ai cứu được.

Cơ chế watchdog: `ClearWatchDogCounter` đếm mỗi 5 s; khi `ca72wdt_timer >= ca72wdt_timeout` thì `CA72WDT_Expire()` ghi log vào 2 vùng QSPI rồi kéo `HRESET_REQN = 0`. Đây cũng là **lối thoát duy nhất** khi `AppMhuTest` kẹt trong busy-wait chờ CA72.

## `0x8C` — WDT reboot ext (watchdog có tham số)

Bản mở rộng của `0x8B`, mang theo debug info để truy vết:

```c
uint16_t event        = received_payload[0];   // START / REFRESH / END
uint16_t refreshPoint = received_payload[1];   // mốc trong code Linux
uint16_t timeout      = received_payload[2];   // giây, chia 5 thành chu kỳ đếm
uint16_t debugInfo1   = received_payload[3];   // bit0 = vô hiệu reboot (debug)
char    *debugInfo2   = (char*)&received_payload[4];   // chuỗi tự do, tối đa 240 B
```

Nhưng lệnh này bị **gate theo loại máy** — logic tương đối rối:

```c
if (getMachineType(&eMachineInfo, &eMachineNo)) {
    if (!isMachineSupportWdtVersionCheck(eMachineInfo, eMachineNo)) {
        CA72WDT_Ext(received_payload);            // máy khác → luôn cho phép
    } else {
        if (isFunctionVersion_CA72WdtExt())       // Eagle/DenebMLK/Sparrow → cần IT6_2.1+
            CA72WDT_Ext(received_payload);
    }
} else {
    "Invalid Machine Type";                        // không xác định được → BỎ QUA
}
```

`getMachineType()` ([app_scpi.c:541](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/ApplicationMrvl/Test/app_scpi.c#L541)) đọc **hai mặt QSPI** (`0xF4379000` và `0xF437A000`), so write-counter để chọn mặt mới hơn, đọc 2 byte tại offset `0x54`, kiểm tra hợp lệ rồi cache lại cho các lần sau. Đây là cùng cơ chế redundant 2 mặt mà `LPPP_SpiFlashWrite_PowerFlickerFlag` dùng.

Lưu ý ba cấp gating này đều **thất bại im lặng** — `send_response = true` vô điều kiện, Linux luôn nhận `SCPI_OK` dù watchdog không hề được bật.

## `default` — lệnh không hỗ trợ

```c
"scpi cmd not handled %x";
header_dest->status = SCPI_E_HANDLER;
send_response = true;
```

Trả lỗi đàng hoàng, không im lặng. Nhưng lưu ý: lệnh vẫn đã bị `send_km_queue` nhân bản sang KM app trước đó rồi.

---

# Phần E — Phát hiện khi rà soát

## E.1 🔴 `0x88` với payload S2/ERP hoàn toàn vô tác dụng

`intr_event` là biến `static` dùng chung giữa `case 0x88` và `power_cpu()`. Nhưng `power_cpu()` ghi đè vô điều kiện:

```c
if (g_wolMode) intr_event = LPPP_INTERNALEVT_GO_TO_ERP;
else           intr_event = LPPP_INTERNALEVT_GO_TO_S2;
xQueueSend(gst_MsgQHandle_PowerCtrl, &intr_event, portMAX_DELAY);
```

Cộng với việc `PowerControlMHUEvent` đã `#if 0` hai nhánh S2/ERP → **không có đường nào để `0x88` payload 2/3 tới được máy trạng thái**. Vào Sleep2/ErP chỉ xảy ra qua `power_cpu()` với `g_wolMode` quyết định.

## E.2 🔴 `case 0x0e` — `return` sớm, bỏ qua cache writeback

```c
if ((clk_payload->flags = hal_get_clk_flags(clock_id)) == -1) {
    header_dest->status = SCPI_E_RANGE;
    header_dest->payload_size = 0;
    return true;              // ← lối thoát DUY NHẤT không qua epilogue
}
```

Response lỗi nằm lại trong D-cache R4, không xuống DDR. Nhưng trả `true` nên `AppMhuTest` vẫn rung chuông và busy-wait; CA72 đọc **dữ liệu cũ của lệnh trước**. Đúng phải là `send_response = true; break;`.

## E.3 🔴 `case 0x9` — kiểm tra biên sai mảng

```c
pmu_devices domain = (payload[0] & 0xff);
if (domain >= pmu_device_end) { … }                     // = 73
memcpy(payload, &op_points[domain], sizeof(struct dvfs_s));
```

`op_points[]` chỉ có **1 phần tử**, nhưng biên kiểm tra là `73`. `domain` từ 1 đến 72 sẽ đọc ngoài mảng ~3 KB rồi memcpy thẳng vào buffer chia sẻ và gửi sang Linux. Hai case cùng họ (`0x0a`, `0x0b`) kiểm tra đúng bằng `sizeof(op_points)/sizeof(struct dvfs_s)`.

## E.4 🟠 Ba case đọc tham số từ **buffer trả về**

```c
case 0x9:  pmu_devices domain = (payload[0] & 0xff);     // payload = &header_dest->payload
case 0xb:  uint32_t    domain = (payload[0] & 0xff);
case 0x1c: uint16_t    domain = (payload[0] & 0xffff);
```

Prologue **không xoá `header_dest->payload`**, nên ba case này đọc trúng phần đuôi của response trước đó trên cùng kênh. Đúng phải là `received_payload[0]`.

Đáng chú ý: nhóm bug E.2–E.4 **chỉ nằm trong các case của Marvell** (DVFS/clock/device). Toàn bộ case `KM_SCPI_CMD_*` dùng `s_header->payload` nhất quán và đúng — nhiều khả năng đây là code mẫu chưa bao giờ được Linux gọi tới thật.

## E.5 🟡 Chỉ 1/28 case kiểm tra `payload_size`

Duy nhất `case 0x06` kiểm tra (`< 12 → SCPI_E_SIZE`). Các case còn lại đọc thẳng `s_header->payload` mà không xác nhận AP có gửi đủ byte. ISR đã kẹp `size` về `[8…128]` nên không tràn bộ nhớ, nhưng nếu AP gửi lệnh không kèm payload thì đọc trúng vùng chưa khởi tạo trên stack.

## E.6 🟡 `payload_size` không khớp lượng dữ liệu ghi

|Case|Khai báo|Thực ghi|
|---|---|---|
|`0x02`|24 B|16 B — 8 byte cuối là rác|
|`0x04`|2 B|4 B|
|`0x09`|20 B|48 B (`sizeof(struct dvfs_s)`)|
|`0x87`|1 B|4 B|

Ghi nhiều hơn khai báo thì vô hại; khai báo nhiều hơn ghi (`0x02`) thì AP đọc trúng rác.

## E.7 🟡 Ba lệnh có side effect ẩn sau cái tên

|Lệnh|Tên gợi ý|Thực tế còn làm|
|---|---|---|
|`0x8A` Get PMU event|chỉ đọc|**Gỡ vĩnh viễn ISR nút nguồn chính**|
|`0x87` Get wake info|chỉ đọc|**Xoá cờ sau khi đọc** — không đọc lại được|
|`0x0a` DVFS set|đổi tần số|**Đưa R4 vào `wfi`** khi domain=0, op_point=1|

---

# Phần F — Ba lệnh thật sự chạm tới máy trạng thái nguồn

Trong 28 lệnh, chỉ ba lệnh có ảnh hưởng tới `fpLPPP_ScenarioFunc`:

```
0x03  →  power_cpu()          →  GO_TO_S2 / GO_TO_ERP / WAKEUP_S1   (g_wolMode quyết định)
0x88  →  send_km_queue → PwMHUEv  →  WAKEUP_NORMAL / GO_TO_S1        (chỉ payload 0 và 1)
0x85  →  LPPP_SetStatus(pwr_sd)   →  ép thẳng state, KHÔNG chạy scenario nào
```

Còn lại là truy vấn, cấu hình clock/domain, hoặc điều khiển watchdog — không đụng tới ma trận trạng thái.