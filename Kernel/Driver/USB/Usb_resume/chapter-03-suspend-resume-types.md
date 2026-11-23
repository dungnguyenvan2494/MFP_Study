# Chương 3: Các loại Suspend và Resume

> Khoá học USB Resume trên Linux Kernel. Baseline: **Linux 5.10.258**. Tên hàm, cờ và thuộc tính sysfs trong chương đã đối chiếu với source.

## 3.0 Vì sao phải tách riêng

"Resume" trong log và trong code có thể là **sáu việc khác nhau**. Một ticket "USB không chạy sau khi thức dậy" có thể hỏng ở bất kỳ tầng nào. Chương này mô tả từng loại **riêng biệt**, cùng một khung:

| # | Câu hỏi |
|---|---|
| 1 | Cái gì kích hoạt? |
| 2 | Thành phần nào xử lý? |
| 3 | Trước đó có gì? |
| 4 | Trong lúc đó có gì? |
| 5 | Sau đó có gì? |
| 6 | Cái gì có thể hỏng? |

```
           ┌──────────── F. Runtime PM (cơ chế "khi nào") ────────────┐
           │      quyết định A/B/C/D xảy ra lúc nào cho từng thiết bị  │
           └──────────────────────────────────────────────────────────┘
 E. System PM: cả máy ngủ/dậy ──► kéo theo D ──► C ──► B ──► A cho mọi thiết bị
 D. Host Controller ──chứa──► C. Root Hub ──có──► B. Port ──nối──► A. Device
```

---

## A. Suspend/Resume của một thiết bị USB

| | Suspend | Resume |
|---|---|---|
| 1. Kích hoạt | Runtime PM thấy thiết bị rảnh đủ lâu (autosuspend), hoặc System PM | driver cần thiết bị (autoresume), System PM, hoặc **thiết bị đánh thức** (remote wakeup) |
| 2. Xử lý | USB core: gọi `suspend` của driver từng interface, rồi suspend cổng (B) | USB core: resume cổng (B), rồi gọi `resume` (hoặc `reset_resume`) của driver |
| 3. Trước | driver ngừng gửi URB, hủy URB đang chạy; USB core kiểm tra có cần remote wakeup không | thiết bị đang `USB_STATE_SUSPENDED` |
| 4. Trong | nếu cần, bật **DEVICE_REMOTE_WAKEUP** (USB 2.0) hoặc **Function Remote Wake** (USB 3.x) bằng lệnh SET_FEATURE; rồi suspend cổng | cổng resume → kiểm tra thiết bị còn đó (GET_STATUS) → tắt cờ remote wakeup → driver resume |
| 5. Sau | thiết bị ở trạng thái `USB_STATE_SUSPENDED`; driver không được gửi URB | thiết bị về `CONFIGURED`; driver gửi lại URB |
| 6. Hỏng | driver từ chối suspend (`-EBUSY`); thiết bị không hỗ trợ remote wakeup mà driver lại cần → không được autosuspend | thiết bị không trả lời GET_STATUS → thử **reset-resume** (nếu cho phép persist) → vẫn lỗi thì **ngắt kết nối logic** (thiết bị biến mất rồi xuất hiện lại) |

**Remote wakeup** (đánh thức từ xa):
- Thiết bị khai báo khả năng này trong configuration descriptor (bit *Remote Wakeup*).
- Linux **bật** tính năng ngay trước khi suspend, và chỉ khi có driver cần (`needs_remote_wakeup`) hoặc người dùng cho phép (`power/wakeup`).
- Linux **tắt** tính năng sau khi resume (`finish_port_resume`: `usb_disable_remote_wakeup`).

## B. Suspend/Resume của một cổng (port)

**Cổng** là nơi tín hiệu thật sự được điều khiển. Hub driver dùng các **lệnh hub chuẩn** cho cả hub ngoài lẫn root hub. Với root hub, HCD tự giả lập các lệnh này bằng thanh ghi.

| Lệnh / bit (USB 2.0) | Ý nghĩa |
|---|---|
| `SetPortFeature(PORT_SUSPEND)` | hub ngừng chuyển lưu lượng xuống cổng → thiết bị ngủ |
| `ClearPortFeature(PORT_SUSPEND)` | hub phát **Resume (K) ≥ 20 ms** xuống cổng |
| bit trạng thái `USB_PORT_STAT_SUSPEND` (0x0004) | cổng đang ngủ (hoặc đang resume) |
| bit thay đổi `USB_PORT_STAT_C_SUSPEND` (0x0004) | **resume vừa hoàn tất**, dùng để báo remote wakeup |

| Lệnh / bit (USB 3.x) | Ý nghĩa |
|---|---|
| `SetPortFeature(PORT_LINK_STATE, U3)` | đưa link vào **U3** (suspend) |
| `SetPortFeature(PORT_LINK_STATE, U0)` | đưa link về **U0** (resume) |
| trường `USB_PORT_STAT_LINK_STATE` (0x01e0) | trạng thái link hiện tại |
| bit thay đổi `USB_PORT_STAT_C_LINK_STATE` (0x0040) | link vừa đổi trạng thái (ví dụ thiết bị tự thoát U3) |

```
 USB 2.0 port:  Enabled ──SetPortFeature(SUSPEND)──► Suspended ──ClearPortFeature(SUSPEND) hoặc thiết bị phát K──►
                Resuming (K ≥ 20 ms) ──kết thúc──► Enabled (+ C_SUSPEND nếu là remote wakeup)
 USB 3.x port:  U0 ──Set LINK_STATE=U3──► U3 ──Set LINK_STATE=U0 hoặc thiết bị LFPS──► Recovery ──► U0 (+ C_LINK_STATE)
```

| | Nội dung |
|---|---|
| 1. Kích hoạt | A (thiết bị suspend/resume), hoặc thiết bị tự phát Resume |
| 2. Xử lý | hub driver (`usb_port_suspend`, `usb_port_resume`) + hub/HCD thực thi |
| 6. Hỏng | cổng không chịu vào Suspend; Resume không kết thúc (cổng kẹt ở Resume/U3/Recovery); cổng mất **connect** hoặc **enable** sau resume (thiết bị "rớt") |

## C. Suspend/Resume của root hub

**Root hub** là hub "ảo" của host controller. Các cổng của nó là cổng vật lý của controller.

| | Nội dung |
|---|---|
| Vì sao cần | Khi **mọi** cổng của root hub đều ngủ (hoặc trống), root hub ngủ, và nhờ đó controller có thể ngủ |
| 1. Kích hoạt | Runtime PM của root hub (`usb1`…`usb4`) khi các con ngủ hết; hoặc System PM |
| 2. Xử lý | USB core gọi HCD **bus suspend/resume** (`xhci_bus_suspend` / `xhci_bus_resume`, `ehci_bus_suspend` / `ehci_bus_resume`) |
| 3. Trước (suspend) | mọi thiết bị con đã ngủ; không có cổng nào đang resume (nếu có thì bus suspend thất bại: `suspend failed because a port is resuming`, debug) |
| 4. Trong | HCD đưa các cổng đang U0 vào U3/Suspend, ghi nhớ cổng nào **do bus suspend mà ngủ**, bật **wake bit** (đánh thức khi có kết nối, ngắt kết nối, quá dòng) |
| Phát hiện hoạt động khi đang ngủ | controller vẫn theo dõi cổng: remote wakeup hoặc cắm/rút sinh ra **Port Status Change** → ngắt → HCD gọi **đánh thức root hub** (`usb_hcd_resume_root_hub`) |
| Báo cho USB core | root hub không có endpoint thật: USB core **hỏi** trạng thái qua `hub_status_data` (định kỳ hoặc khi có ngắt) rồi đánh dấu cổng có thay đổi |
| 5. Sau (resume) | cổng do bus suspend được đưa về U0/Enabled; hub driver chạy `hub_activate(HUB_RESUME)` kiểm tra lại từng cổng |
| 6. Hỏng | ngắt cổng không tới khi controller đang ngủ (remote wakeup bị mất); `port … resume PLC timeout` (xHCI) |

## D. Suspend/Resume của host controller

| | Nội dung |
|---|---|
| 1. Kích hoạt | Runtime PM của controller (mọi root hub đã ngủ); System PM |
| 2. Xử lý | driver platform/PCI → `xhci_suspend` / `xhci_resume`, `ehci_suspend` / `ehci_resume` |
| 3. Trước | root hub đã bus suspend; controller không còn lệnh đang chạy |
| 4. Trong (suspend) | dừng controller (Run = 0, chờ HCHalted); **lưu thanh ghi** (bản chụp operational và interrupter); yêu cầu controller **lưu trạng thái nội bộ** (Controller Save State); tắt ngắt; có thể tắt clock/PHY |
| 4. Trong (resume) | khôi phục thanh ghi và trạng thái nội bộ (Controller Restore State); **hoặc khởi tạo lại** nếu mất điện/quirk/lỗi; bật Run; kiểm tra sự kiện cổng đang chờ; bật lại việc poll root hub |
| 5. Sau | USB core resume root hub (C), rồi tới các thiết bị (A/B) |
| 6. Hỏng | `Controller not ready at resume`, `xHC restore state timeout`, `xHC error in resume, USBSTS … Reinit` (tự khởi tạo lại), không có ngắt sau resume |

Chi tiết từng bước `xhci_resume`: xem `docs/diagrams/xhci-resume-sequence.md` (đã có sẵn trong repo).

## E. System Suspend/Resume (Suspend-to-RAM)

| | Nội dung |
|---|---|
| 1. Kích hoạt | người dùng/ứng dụng yêu cầu ngủ (`echo mem > /sys/power/state`); trên Quartz là **deep sleep** do PowerControl khởi động |
| 2. Xử lý | **PM core**: đóng băng tiến trình → gọi callback của mọi thiết bị theo cây (con trước cha) qua các pha `prepare` → `suspend` → `suspend_late` → `suspend_noirq` → tắt CPU phụ → nền tảng cắt nguồn |
| Đánh thức hệ thống | một **nguồn đánh thức** được bật (nút nguồn, mạng, USB, timer) báo cho PMU/SoC |
| Resume | ngược lại: `resume_noirq` → `resume_early` → `resume` → `complete`; **cha trước con** |
| Phục hồi USB | controller resume (D) → root hub (C) → hub và cổng (B) → thiết bị (A) → driver |
| 6. Hỏng | controller không khởi động được; thiết bị phải reset-resume nhưng driver không hỗ trợ (bị gỡ rồi gắn lại); thứ tự resume sai (PHY chưa sẵn sàng) |

**Log thật của Quartz** (log OP_BTS-71095, lúc 07:01:48 → 07:02:45). Các dòng `in …`/`enter …` là printk do KM thêm vào `kernel/power/suspend.c`:
```
[151668.805067] in pm_suspend
[151668.808205] enter enter_state
[151668.872312] in suspend_devices_and_enter
[151668.876396] Suspending console(s) (use no_console_suspend to debug)
[151668.981694] ==>mvri_suspend               dev_state=00000003
[151669.473759] in suspend_enter
[151669.495561] CPU1: shutdown
[151669.633225] PMU_resume:WAKE_UP not detect
[151669.690019] ==>mvri_resume                GIDontInitPhy=1 Suspended=1
[151670.188530] usb usb1: root hub lost power or was reset
[151670.188604] usb usb3: root hub lost power or was reset
[151671.107930] in suspend_finish
```
- Giữa `suspend_enter` và `PMU_resume` là lúc máy ngủ thật.
- `root hub lost power or was reset` cho thấy xHCI đi nhánh **khởi tạo lại**, nên mọi thiết bị USB sẽ reset-resume.

**Trên Quartz cần phân biệt:**

| Chế độ | Có phải Linux System Suspend không | Dấu hiệu |
|---|---|---|
| **Deep sleep** | **có** (`pm_suspend`) | `in pm_suspend`, `Suspending console(s)`, `root hub lost power or was reset` |
| **SLEEP1** (ngủ nhẹ) | **không** | chỉ có log PowerControl/BSP (`Now in SLEEP1`, `leave_sleep_1`). Controller USB không đi qua suspend/resume của Linux |

## F. Runtime Suspend/Resume

**Runtime PM** (quản lý nguồn lúc đang chạy): cho từng thiết bị ngủ riêng khi rảnh, **trong khi hệ thống vẫn chạy**.

| Khái niệm | Tiếng Việt | Nghĩa |
|---|---|---|
| usage count | bộ đếm sử dụng | số "người đang dùng" thiết bị; > 0 thì không được ngủ |
| runtime idle | rảnh | usage count về 0: PM core hỏi "ngủ được chưa?" |
| autosuspend | tự ngủ | ngủ sau khi rảnh liên tục N giây (`autosuspend_delay`) |
| runtime suspend | ngủ khi chạy | callback `runtime_suspend` của thiết bị được gọi |
| autoresume | tự dậy | có người cần (`get`) thiết bị đang ngủ → callback `runtime_resume` |
| runtime resume | dậy khi chạy | callback `runtime_resume` |

| | Nội dung |
|---|---|
| 1. Kích hoạt | usage count về 0 + hết thời gian autosuspend (suspend); driver gọi `usb_autopm_get_interface` / `pm_runtime_get_sync` hoặc thiết bị remote wakeup (resume) |
| 2. Xử lý | Runtime PM core → `usb_runtime_suspend` / `usb_runtime_resume` (cho `usb_device`); hub driver cho cổng; HCD cho controller |
| Mặc định | `CONFIG_USB_AUTOSUSPEND_DELAY=2` trên Quartz → **2 giây**; đổi bằng tham số `usbcore.autosuspend=` |
| Thuộc tính sysfs (đã đối chiếu `core/sysfs.c`) | `power/control` (`auto` cho phép tự ngủ, `on` cấm), `power/autosuspend_delay_ms`, `power/runtime_status`, `power/wakeup`, `power/persist`, `power/active_duration`, `power/connected_duration`, `power/usb2_hardware_lpm` |
| 3. Trước (suspend) | không còn URB; driver có `supports_autosuspend`; nếu driver cần remote wakeup mà thiết bị không hỗ trợ → **không** cho ngủ (debug: `remote wakeup needed for autosuspend`) |
| 5. Sau (resume) | thiết bị `active`; driver gửi URB |
| 6. Hỏng | quên `put` → thiết bị không bao giờ ngủ; `put` thừa → bộ đếm lệch, thiết bị có thể ngủ khi đang dùng (5.10 không có cảnh báo riêng cho trường hợp này); `pm_runtime_enable` lệch cặp → `Unbalanced pm_runtime_enable!`; resume thất bại → driver nhận lỗi; cha không active → `runtime PM trying to activate child device … but parent (…) is not active` |

---

## 3.7 Bảng tổng hợp

| Loại | Đối tượng | Kích hoạt chính | Xử lý chính | Dấu hiệu nhận biết |
|---|---|---|---|---|
| A. Device | `usb_device` | Runtime/System PM, remote wakeup | USB core | `power/runtime_status` của thiết bị |
| B. Port | cổng hub | A | hub driver + hub/HCD | bit SUSPEND, link state U3 |
| C. Root hub | `usbN` | mọi cổng ngủ / System PM | HCD bus suspend/resume | `power/runtime_status` của `usbN` |
| D. Host controller | xHCI/EHCI | mọi root hub ngủ / System PM | HCD suspend/resume | `root hub lost power…` nếu khởi tạo lại |
| E. System | cả máy | người dùng, PMU | PM core | `PM: suspend entry`/printk của KM, CPU shutdown |
| F. Runtime | từng thiết bị | usage count, timeout | Runtime PM core | sysfs `power/*` |

## 3.8 Tóm tắt Chương 3

1. Sáu loại **lồng vào nhau** nhưng **khác nhau**. Khi đọc log, hãy hỏi "đây là resume của tầng nào?".
2. **Thiết bị** resume cần **cổng** resume trước. **Cổng** cần **root hub** đang chạy. Root hub cần **controller** đang chạy.
3. Remote wakeup chỉ được bật **ngay trước** khi suspend và **tắt** ngay sau khi resume.
4. Runtime PM quyết định *khi nào*; System PM ép *tất cả* cùng lúc.
5. Trên Quartz, **deep sleep là System Suspend thật**. **SLEEP1 thì không**.

## Câu hỏi kiểm tra

1. Một ổ USB tự ngủ sau 2 giây không dùng. Đây là loại nào trong A–F? Những loại nào khác cũng xảy ra theo?
2. Vì sao Linux chỉ bật remote wakeup ngay trước khi suspend, chứ không bật sẵn từ lúc cấu hình thiết bị?
3. `USB_PORT_STAT_C_SUSPEND` báo điều gì, và vì sao nó quan trọng để nhận ra remote wakeup?
4. Root hub không có endpoint interrupt thật. Vậy USB core biết cổng của root hub có thay đổi bằng cách nào?
5. Khác biệt giữa "controller khôi phục trạng thái" và "controller khởi tạo lại" nhìn từ phía thiết bị USB là gì?
6. Trong log Quartz, dòng nào cho biết máy đã thật sự ngủ và vừa thức dậy?
7. Driver quên gọi `usb_autopm_put_interface` sau mỗi lần dùng. Hậu quả là gì?
8. Vì sao lỗi trong OP_BTS-71095 (sau SLEEP1) **không** thể là lỗi của `xhci_resume`?

---

**Chương tiếp theo:** Chương 4 xuống mức tín hiệu điện: J, K, SE0, tín hiệu Resume USB 2.0 theo cả hai chiều, thời gian; rồi USB 3.x: U1/U2/U3, LFPS, LTSSM, Recovery, Warm Reset.
