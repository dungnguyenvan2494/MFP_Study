# Phân tích `km_scpi` và mailbox (MHU)

> Source không có `km_spci`. Tôi coi bạn muốn nói `km_scpi` (SCPI = System Control and Power Interface, giao thức hộp thư giữa AP và SCP). Đã đọc hết code liên quan, **chưa build hay chạy**.

## 1. Bức tranh tổng thể

Hai bộ xử lý nói chuyện với nhau qua một hộp thư:

- **AP** là Cortex-A72 chạy Linux. Nó là bên gửi lệnh.
- **LPPP** là Cortex-R4 chạy FreeRTOS, đóng vai SCP (System Control Processor). Nó quản lý nguồn, PMU, WDT cho CA72.

Mô hình này bám theo ARM Juno (DT dùng `arm,juno-scp-shmem`, `arm,juno-sram-ns`).

```
Người dùng lệnh                          Linux (AP, CA72)                                   LPPP (R4)
─────────────────   ┌────────────────────────────────────────────────────────┐
mdels800_pmu.c ───► │ km_scpi.c   (get_km_scpi_ops → km_scpi_ops)            │
quartz-ps-sleep1.c  │   cmd 0x80..0x8C (lệnh riêng của KM)                   │
                    │        │ scpi_send_message_ext()                       │
                    │ arm_scpi.c (giao thức SCPI chuẩn + sửa của KM)         │
                    │        │ mbox_send_message()                           │
                    │ mailbox core ─► arm_mhu_base.c / mv_mhu.c (MHU)        │
                    └────────┼───────────────────────────────▲───────────────┘
                             │ ghi INTR_SET (bit0)           │ IRQ 269 (LP-NS)
                             ▼                               │
                      ┌──────── SRAM 0x7fff0000 (shmem 256B) ─────────┐
                      │ [0x00..0x7F] rx_payload: command,status,data  │ ◄── LPPP ghi trả lời
                      │ [0x80..0xFF] tx_payload: command,status,data  │ ──► LPPP đọc lệnh
                      └───────────────────────────────────────────────┘
```

## 2. Mailbox phần cứng: MHU (`drivers/mailbox/`)

Chọn driver trong `km_mvebu_v8_lsp_defconfig`: `CONFIG_MV_MHU=y`, còn `CONFIG_ARM_MHU` tắt.

### File và vai trò

- `mv_mhu.c` (26 dòng) chỉ định nghĩa offset thanh ghi và `MHU_CHANS=1`, rồi `#include "arm_mhu_base.c"`.
- `arm_mhu_base.c` chứa logic thật.
- Bản ARM gốc (`arm_mhu.c`) có 3 kênh (LP, HP, Secure). Bản Marvell chỉ dùng 1 kênh LP.

### Device tree (`quartz-sb.dtsi:48`)

```
mhu@8e8009800  compatible "arm,mhu","arm,primecell"
 reg 0x8_e800_9800, size 0x1000
 interrupts: 269 (LP-NS), 268 (HP-NS), 270 (Secure)   ← driver chỉ dùng irq[0] = 269
 #mbox-cells = <1>
```

### Thanh ghi (tính từ `base`)

| Hướng | Thanh ghi | Offset |
|---|---|---|
| RX (LPPP → AP) | STAT / SET / CLR | `0x0 / 0x4 / 0x8` |
| TX (AP → LPPP) | STAT / SET / CLR | `0xc / 0x10 / 0x14` |

### Hoạt động (`arm_mhu_base.c`)

- **Gửi**: `mhu_send_data()` ghi giá trị vào `TX+INTR_SET`, tức là bắn interrupt sang R4. Dữ liệu gửi đi **không phải payload**. Nó là `*(u32*)msg`, mà `struct scpi_xfer` có `slot` ở **phần tử đầu tiên** (comment "has to be first element") và `slot = BIT(0)`. Vậy thực chất chỉ là "đánh chuông slot 0". Dữ liệu thật nằm trong shared memory.
- **Báo hoàn tất gửi**: `txdone_poll = true`, `txpoll_period = 1` ms. `mhu_last_tx_done()` đọc `TX+STAT`, bằng 0 nghĩa là LPPP đã xóa bit. Client đặt `tx_block = true`, `tx_tout = 20` ms.
- **Nhận**: `mhu_rx_interrupt()` đọc `RX+STAT`. Nếu khác 0, nó gọi `mbox_chan_received_data()` rồi ghi `RX+INTR_CLR` để xóa. Hàm này chạy trong hard IRQ.
- **Phần KM thêm vào**: `mhu_wdt_ca72_end()` (op `.wdt_ca72_end`) poll trạng thái RX mỗi 10 ms, tối đa khoảng 100 lần (khoảng 1 giây). Nếu LPPP không trả lời thì gọi `emergency_restart()`. Chỉ tìm thấy phần định nghĩa và gán op. Không có nơi gọi nào trong các file `.c` (header và module chưa kiểm).

## 3. Lớp giao thức: `arm_scpi.c` (bản Sudeep Holla/ARM đã bị KM sửa)

### Cấu trúc

- `scpi_chan`: một kênh mailbox, gồm `tx_payload`/`rx_payload`, danh sách `rx_pending`, pool 10 `scpi_xfer` (`MAX_SCPI_XFERS`), `spinlock rx_lock`, `mutex xfers_lock`, và `token` 8 bit.
- `scpi_shared_mem`: `{ __le32 command; __le32 status; u8 payload[]; }`.

### Định dạng word lệnh (`PACK_SCPI_CMD`)

```
bit  7:0   cmd id      ← KM mở rộng thành 0xff (upstream chỉ 7 bit)
bit 15:8   token       (chống nhầm response, tự tăng, bỏ qua 0)
bit 24:16  data size
```

Việc mở rộng `CMD_ID_MASK` lên `0xff` là bắt buộc, vì các lệnh riêng của KM nằm trong khoảng `0x80..0x8C`. Với mask 7 bit của upstream thì các lệnh này sẽ bị cắt.

### Luồng một giao dịch (`scpi_send_message_ext`, `arm_scpi.c:629`)

1. Chọn kênh: `atomic_inc_return(next_chan) % num_chans`. Hiện chỉ có 1 kênh.
2. `mutex_lock(&send_message)`. Đây là mutex toàn cục, thêm vào ở OP_BTS-18219.
3. `wait_event_timeout(wfi_wait_queue, !is_lppp_wfi, 60s)`. Nếu LPPP đang ngủ WFI thì phải chờ.
4. Vòng retry `SCPI_COMMUNICATE_RETRY_MAX = 20` lần:
   - Lấy một `scpi_xfer` từ pool, gán `cmd`, `tx_buf`, `rx_buf`.
   - Gọi `mbox_send_message()`. Trong khi đó mailbox core gọi `scpi_tx_prepare()` (`:450`), hàm này `memcpy_toio` payload vào shmem, gắn token, thêm xfer vào `rx_pending`, rồi `iowrite32(cmd, &mem->command)`. Sau đó `mhu_send_data()` bắn IRQ.
   - Chờ `wait_for_completion_timeout(done, MAX_RX_TIMEOUT = 2s)`.
   - Phía RX, IRQ → `scpi_handle_remote_msg()` (`:438`) đọc `mem->command` → `scpi_process_cmd()` (`:382`) tìm xfer có cùng `(token, cmd id)` → copy payload và `status` → `complete()`.
   - Nếu timeout thì `scpi_process_cmd()` được gọi lại để gỡ xfer khỏi `rx_pending`.
   - Thành công hoặc `-EINVAL` thì thoát vòng. Lỗi khác thì in `[RETRY_ERR]` rồi `msleep(100)`.
   - **Ở lần retry thứ 18** (`cnt == 18`): code `ioremap(0xe8008000)` rồi `base[0xc/4] |= PAUSE_R4_INT`. Đây là cách đánh thức R4 bằng khối IPC, bỏ qua SCPI (OP_BTS-30175).
5. Nếu `rx_buf == NULL` thì lệnh là fire-and-forget, không chờ trả lời.

`scpi_send_message()` (`:501`) là bản "chuẩn" dùng cho clock/DVFS/sensor/power-domain. Nó giống bản `_ext` nhưng thêm: bảng lệnh qua `scpi_info->commands[idx]`, timeout 3s cho `SET_DVFS` và `SET_DEVICE_PWR_STATE` (`CONFIG_KM_BIZHUB`), và cờ `is_wfi`.

### Hai chỗ KM "lạm dụng" lệnh chuẩn

- `scpi_dvfs_set_idx(domain 0, index 1)` thực chất là lệnh **dừng LPPP vào WFI**. Nó đặt `is_lppp_wfi = 1`. Mọi lệnh SCPI sau đó phải chờ, tối đa 60s.
- `scpi_dvfs_set_idx(0, 0)` là lệnh **đánh thức LPPP**. Nó bắn IPC interrupt, đặt `is_lppp_wfi = 0` và `wake_up()`. Nó cố ý đi **ngoài mutex** `send_message` để có thể đánh thức khi có luồng khác đang giữ mutex và chờ.
- `SET_DVFS` bị loại khỏi cơ chế retry (`:380`). Lý do trong comment: LPPP dùng lệnh này làm "QSPI exclusive control" khi download firmware và không trả lời. Retry sẽ phá quá trình download. Cái này liên quan tới phần `/dev/mtd2` và `lppp-spi-firmware` ở câu hỏi trước.

### Probe (`scpi_probe`, `:1166`)

1. Đọc `mboxes` và `shmem` từ DT, `devm_ioremap` vùng 0x100 byte. `rx_payload` = nửa đầu, `tx_payload` = `rx + size/2`.
2. Cấu hình `mbox_client`: `tx_block = true`, `tx_tout = 20`, `knows_txdone = false`.
3. `mbox_request_channel()`.
4. `scpi_init_versions()` gửi `SCPI_CAPABILITIES` để lấy phiên bản.
5. Cuối cùng gọi `km_scpi_probe(pdev)`. Hàm này lấy `km_scpi_info` nằm **ngay sau** `scpi_drvinfo` (`&scpi_info[1]`, chung một lần `devm_kzalloc`) rồi gán `km_scpi_ops`.

## 4. Lớp mở rộng KM: `km_scpi.c` / `km_scpi.h`

Chỉ là wrapper mỏng. Mỗi hàm gọi `scpi_send_message_ext(cmd, tx, len, rx, len)` rồi log lỗi. Client lấy ops bằng `get_km_scpi_ops()` (`EXPORT_SYMBOL_GPL`, trả `NULL` nếu chưa probe).

| Cmd | Hàm | Dữ liệu | Ý nghĩa |
|---|---|---|---|
| 0x81 | `get_protocol_version` | → u16 | Phiên bản giao thức KM |
| 0x82 | `ap_ctl(on)` | int | Điều khiển chân AP_PWR_EN |
| 0x83/0x84 | `set/get_ap_status` | int | `NOT_BOOT=0`, `WARP_BOOT=1` (snapshot xong) |
| 0x85/0x86 | `set/get_pwr_status` | int | `PWR_OFF/NORMAL/SLEEP1/SLEEP2/ERP` = 0..4 |
| 0x87 | `get_wake_info` | → u8 | Nguyên nhân thức dậy |
| 0x88 | `ap_event(ev)` | int | `WAKEUP_NORMAL/GO_TO_S1/S2/ERP` |
| 0x89 | `rewrite_request` | không | Yêu cầu ghi lại LPPP ROM, trả 0 là xong |
| 0x8A | `get_pmu_event` | → u8 | Sự kiện PMU |
| 0x8B | `set_wdt_ca72(ev)` | int | WDT cho CA72: `START/END/REFRESH` |
| 0x8C | `set_wdt_ca72_ext` | struct 256 B | `{event,point,timeout,debugInfo1,debugInfo2[240]}` |

### Hành vi đáng chú ý

- `km_scpi_ap_event()` (`:201`): nếu gửi lỗi thì ghi detail code `0x2201000000000000 | (-ret & 0xff) << 36` bằng `set_newdetailcode()`, rồi `msleep(10000)` và `PMU_Reboot()`. Tức là mất liên lạc với LPPP khi báo sleep/wake sẽ **reboot cả máy**. `scpi_device_set_power_state()` cũng làm vậy (detail code `0x2201d…`), chỉ ở lần lỗi đầu tiên.
- Các hàm WDT bỏ qua `-EINVAL` (không log lỗi).
- Người gọi:
  - `drivers/pmu/mdels800_pmu.c` (và bản trùng `drivers/mfp/pmu/`) gọi `rewrite_request`, `get_wake_info`, `set_wdt_ca72[_ext]`.
  - `drivers/regulator/quartz-ps-sleep1.c:104,125` gọi `ap_event(GO_TO_S1)` và `ap_event(WAKEUP_NORMAL)`.

## 5. Những điểm rủi ro (đã đọc code, chưa chạy)

1. **Có thể ghi lố vùng shared memory.** DT khai shmem `reg = <0x0 0x100>`, tức 256 B, chia đôi nên `tx_payload` chỉ có 128 B, trừ 8 B header còn 120 B cho dữ liệu. Nhưng `set_wdt_ca72_ext` gửi **256 B** (`sizeof(sendData)`). `memcpy_toio` sẽ ghi từ offset `0x88` đến `0x188`, vượt vùng 0x100 đã `ioremap` và có thể đè lên vùng SRAM kế bên (map theo trang nên không fault). Cần đối chiếu với firmware LPPP xem nó đọc ở đâu. Có thể thực tế vẫn chạy vì vùng bên cạnh chưa dùng, nhưng đây là hợp đồng không khớp giữa DT và code.
2. **Rò mutex khi `ioremap` lỗi.** Trong `scpi_send_message` và `scpi_send_message_ext`, nhánh `if (!base) { ...; return -1; }` (khoảng `:395` và `:488`) thoát mà **không** `mutex_unlock(&send_message)`. Sau đó mọi lệnh SCPI sẽ treo vĩnh viễn. Xác suất thấp nhưng hậu quả nặng, và sửa chỉ cần một dòng.
3. **Chờ lâu khi giữ mutex.** Retry 20 lần × (timeout 2s + 100 ms) có thể giữ `send_message` tới khoảng 40 giây. Luồng thứ hai cũng có thể chờ thêm 60s ở `wfi_wait_queue`. Mọi caller đều bị block trong thời gian đó.
4. **`is_lppp_wfi` là `int` thường**, đọc ghi giữa các luồng không dùng `READ_ONCE/WRITE_ONCE`. Với `wait_event` và `wake_up` thì đúng thứ tự trong thực tế, nhưng không chặt.
5. **Truy cập IPC bằng con trỏ trần**: `base[0xc/4] |= …` là read-modify-write trên MMIO không qua `readl/writel`, và địa chỉ `0xe8008000` bị hard-code.
6. **Mã hóa file lẫn lộn.** `arm_scpi.c` là **Shift-JIS**, `km_scpi.c/.h` là **ISO-2022-JP**. Edit/Write sẽ từ chối. Phải sửa bằng shell và giữ encoding.

## 6. Đề xuất bước tiếp

- Vẽ sequence diagram chi tiết cho hai luồng: *ap_event(GO_TO_S1)* và *dừng/đánh thức LPPP qua DVFS domain 0*.
- Đối chiếu layout shmem với firmware R4 (`r4-freertos`), nếu có source, để kết luận điểm (1).
- Viết patch nhỏ sửa điểm (2).
