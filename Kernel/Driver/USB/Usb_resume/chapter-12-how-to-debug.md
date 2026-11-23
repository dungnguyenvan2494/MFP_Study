# Chương 12: Cách debug USB Resume

> Khoá học USB Resume trên Linux Kernel. Baseline: **Linux 5.10.258**. Tình trạng sẵn có trên Quartz theo `km_mvebu_v8_lsp_defconfig`: `CONFIG_DEBUG_FS=y`; **không bật** `USB_MON`, `FTRACE`, `DYNAMIC_DEBUG`, `PM_DEBUG`. Đường dẫn sysfs/debugfs đã đối chiếu với source.

## 12.0 Chuẩn bị: bản build debug cho Resume

Resume gần như **chỉ để lại log debug**. Để điều tra nghiêm túc, hãy chuẩn bị bản build bật:
- `CONFIG_DYNAMIC_DEBUG` (thấy log `dev_dbg` của hub/xHCI);
- `CONFIG_FTRACE` + `CONFIG_TRACING` (tracepoint `xhci-hcd`, sự kiện `rpm_*`, `device_pm_callback_*`);
- `CONFIG_USB_MON`;
- `CONFIG_PM_DEBUG`, `CONFIG_PM_SLEEP_DEBUG` (thông tin thời gian của từng callback suspend/resume), `CONFIG_PM_ADVANCED_DEBUG` (thuộc tính `power/runtime_usage`).

---

## 12.1 dmesg

```bash
dmesg -T | grep -iE "usb|xhci|ehci|hub|resume|suspend|PM:|phy|PMU_resume"
```
```bash
dmesg -n 8
```
Lệnh thứ hai cho phép in mọi mức log ra console, để thấy cả log info như `reset … USB device number`.

**Đọc:** tìm mốc system resume (Quartz: `in suspend_enter` → `PMU_resume:` → `root hub lost power` → `in suspend_finish`), rồi các dòng USB sau mốc đó.

## 12.2 journalctl -k

```bash
journalctl -k -b -1 | grep -iE "usb|resume"
```
Dùng khi lỗi xảy ra ở lần boot trước, nếu journald có lưu. Trên Quartz cần kiểm tra userspace có journald hay không.

## 12.3 lsusb

```bash
lsusb -t
```
```bash
lsusb -v -d 132b:20b7 | grep -iE "Remote Wakeup|bmAttributes|MaxPower"
```
- `-t`: cây cổng sau resume, xem thiết bị còn ở đúng cổng không.
- `-v`: thiết bị có khai báo **Remote Wakeup** không (bit trong `bmAttributes` của configuration).
- Số `Device NNN` tăng sau resume → thiết bị đã bị enumerate lại.

## 12.4 sysfs: `/sys/bus/usb/devices/<dev>/power/`

| File | Nghĩa | Ví dụ |
|---|---|---|
| `control` | `auto` (cho tự ngủ) / `on` (cấm ngủ) | Quartz mặc định `on` (KM tắt autosuspend) |
| `runtime_status` | `active` / `suspended` / `suspending` / `resuming` / `error` | `error` = một callback runtime trước đó lỗi |
| `autosuspend_delay_ms` | thời gian rảnh trước khi ngủ | mặc định 2000 (`CONFIG_USB_AUTOSUSPEND_DELAY=2`) |
| `wakeup` | `enabled` / `disabled`: cho phép remote wakeup | |
| `persist` | 1 = cho phép reset-resume giữ thiết bị | |
| `active_duration`, `connected_duration` | ms hoạt động / ms đã nối | `active` tăng chậm hơn `connected` → thiết bị có ngủ |
| `usb2_hardware_lpm` | LPM L1 cho USB 2.0 | |
| `runtime_usage` (của driver core, cần `CONFIG_PM_ADVANCED_DEBUG`) | usage count hiện tại | ≠ 0 mãi → ai đó giữ thiết bị |

```bash
D=/sys/bus/usb/devices/3-1.2; for f in control runtime_status wakeup persist active_duration connected_duration runtime_usage; do printf "%s=%s\n" $f "$(cat $D/power/$f 2>/dev/null)"; done
```

Thử Runtime PM trên Quartz (chỉ để thí nghiệm):
```bash
echo auto > /sys/bus/usb/devices/3-1.2/power/control
```

## 12.5 debugfs: `/sys/kernel/debug/usb/`

| Đường dẫn | Dùng cho Resume |
|---|---|
| `devices` | thiết bị, cấu hình, trạng thái hiện tại |
| `xhci/<ctrl>/ports/portNN/portsc` | **PORTSC của từng cổng**: PLS, CCS, PE, các bit change |
| `xhci/<ctrl>/reg-op` | USBCMD, USBSTS (HCH, CNR, SRE, HCE, HSE, PCD) |
| `xhci/<ctrl>/reg-runtime` | IMAN, ERDP: ngắt có bật không |
| `xhci/<ctrl>/devices/NN/slot-context`, `ep-context` | trạng thái slot/endpoint sau resume |
| `usbmon/` | chỉ khi bật `CONFIG_USB_MON` |

```bash
for p in /sys/kernel/debug/usb/xhci/8d00d0000.usb3/ports/port*; do echo "$p $(cat $p/portsc)"; done
```

**Giải mã nhanh PORTSC của xHCI:**

| Bit | Ý nghĩa |
|---|---|
| bit 0 | CCS (có kết nối) |
| bit 1 | PED (enabled) |
| bit 5–8 | **PLS** (0=U0, 2=U2/L1, 3=U3, 4=Disabled, 5=RxDetect, 6=Inactive, 7=Polling, 8=Recovery, 15=Resume) |
| bit 9 | PP (có nguồn) |
| bit 17 | CSC |
| bit 22 | **PLC** |
| bit 24 | CAS |

## 12.6 xHCI register inspection

```bash
cat /sys/kernel/debug/usb/xhci/8d00d0000.usb3/reg-op
```

| Đọc thấy | Kết luận |
|---|---|
| USBSTS.HCH = 1 sau resume | controller không chạy |
| USBSTS.CNR = 1 | chưa sẵn sàng (đang khởi tạo/resume dở) |
| USBSTS.SRE = 1 | khôi phục trạng thái lỗi → nên có `xHC error in resume … Reinit` |
| USBSTS.HCE = 1 | lỗi nội bộ, cần reset |
| USBSTS.HSE = 1 | lỗi bộ nhớ (khoá DMA, Chương 5) |
| giá trị 0xFFFFFFFF | không truy cập được thanh ghi: clock/nguồn |

## 12.7 EHCI register inspection

EHCI có debugfs riêng (`/sys/kernel/debug/usb/ehci/<ctrl>/registers`, chỉ có khi bật `CONFIG_DYNAMIC_DEBUG`). Hãy đọc:
- `USBSTS`: PCD, HCHalted, FATAL;
- `PORTSC`: SUSPEND (bit 7), RESUME (bit 6), PE, CCS.

Cổng kẹt resume có **RESUME = 1** kéo dài.

## 12.8 usbmon (cần build lại)

```bash
cat /sys/kernel/debug/usb/usbmon/3u > /tmp/bus3.txt
```

Trong lúc resume sẽ thấy các **control transfer tới hub**:
- `Ci`/`Co` với `SetPortFeature`, `ClearPortFeature`, `GetPortStatus`;
- sau đó `GET_STATUS` tới thiết bị (`finish_port_resume`).

Thiếu các gói này → hub driver không chạy tới bước resume cổng.

## 12.9 ftrace / trace-cmd (cần build lại)

| Sự kiện | Cho thấy |
|---|---|
| `xhci-hcd:xhci_handle_port_status` | mỗi Port Status Change kèm PORTSC |
| `xhci-hcd:xhci_get_port_status`, `xhci_hub_status_data` | hub driver đọc trạng thái |
| `rpm:rpm_resume`, `rpm:rpm_suspend`, `rpm:rpm_return_int` | Runtime PM của từng thiết bị |
| `power:device_pm_callback_start/end` | thời điểm và kết quả từng callback system PM |

```bash
trace-cmd record -e xhci-hcd:xhci_handle_port_status -e rpm -e power:device_pm_callback_start -e power:device_pm_callback_end
```

## 12.10 Dynamic debug (cần build lại)

```bash
echo 'file drivers/usb/core/hub.c +p' > /sys/kernel/debug/dynamic_debug/control
```
```bash
echo 'file drivers/usb/host/xhci-hub.c +p; file drivers/usb/host/xhci-ring.c +p' > /sys/kernel/debug/dynamic_debug/control
```

Bật hai dòng trên để thấy toàn bộ log debug ở Chương 10, mục A–C: `usb auto-resume`, `finish resume`, `port resume event …`, `resume USB2 port …`…

## 12.11 Ghép thời gian giữa các nguồn

| Nguồn | Đồng hồ | Ghi chú |
|---|---|---|
| `dmesg` | `[giây.µs]` từ lúc boot (không tính thời gian ngủ) | trên Quartz, console còn kèm giờ PC thu log |
| ftrace | timestamp của trace clock (mặc định `local`) | chuyển `trace_clock` sang `mono`/`boot` để so với dmesg |
| Ứng dụng KM | tick ~200/s, ví dụ `(30311533)` | quy đổi bằng một dòng có cả hai đồng hồ |
| usbmon | µs riêng | ghép qua sự kiện chung (submit trùng thời điểm với log driver) |

**Quy trình ghép một lần resume:**
1. Mốc **A**: `PMU_resume:` / `resume_noirq` (bắt đầu dậy).
2. Mốc **B**: `xhci_resume` (`root hub lost power`), tức là controller lên.
3. Mốc **C**: `usb_resume_work CALL` của từng thiết bị (Quartz, khoảng B + 500 ms).
4. Mốc **D**: `xhci_handle_port_status` / `portsc` từng cổng.
5. Mốc **E**: `reset … USB device number` (reset-resume xong địa chỉ).
6. Mốc **F**: URB đầu tiên hoàn tất (usbmon `C` đầu tiên).

Khoảng nào dài bất thường thì đó là chỗ cần đào.

## 12.12 Tóm tắt Chương 12

1. Bản build debug là bắt buộc nếu muốn thấy luồng resume chi tiết.
2. sysfs `power/*` cho biết trạng thái Runtime PM. debugfs `portsc` và `reg-op` cho biết trạng thái phần cứng.
3. ftrace với `xhci_handle_port_status` + `rpm_*` + `device_pm_callback_*` là bộ ba mạnh nhất.
4. Ghép thời gian theo sáu mốc A–F để tìm khoảng chậm hoặc treo.

## Câu hỏi kiểm tra

1. Bạn chỉ có `dmesg` và debugfs trên Quartz. Liệt kê các bước để biết cổng `3-1.2` có đang U0 sau deep sleep không.
2. `runtime_status = error` nghĩa là gì, và làm sao đưa thiết bị ra khỏi trạng thái đó?
3. Vì sao timestamp của dmesg không tăng trong lúc máy ngủ, và điều này ảnh hưởng gì khi ghép log?
4. Những tracepoint nào cho bạn biết hub driver đã gửi ClearPortFeature(SUSPEND) cho một cổng root hub xHCI?
5. Viết lệnh kiểm tra nhanh mọi cổng của `8d00d0000.usb3` có PLS khác 0 không.

---

**Chương tiếp theo:** Chương 13: các bảng so sánh tổng kết.
