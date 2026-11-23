# Chương 11: Tình huống debug Resume thực tế

> Khoá học USB Resume trên Linux Kernel. Baseline: **Linux 5.10.258**. Nhãn log: **[đã xác minh]** = câu chữ có trong source 5.10.258 (giá trị là ví dụ); **[minh họa]** = định dạng đại diện; **★** = log thật trên Quartz. Mỗi tình huống: trạng thái ban đầu, kích hoạt, luồng, chuyển trạng thái phần cứng, log kỳ vọng, triệu chứng, nguyên nhân, cách debug, cách xác nhận.

## Danh sách

| # | Tình huống | Tầng nghi ngờ |
|---|---|---|
| 1 | Bàn phím không dùng được sau system resume | controller / reset-resume / driver |
| 2 | Chuột chạy rồi ngừng sau runtime resume | Runtime PM / driver |
| 3 | Thiết bị ngắt rồi kết nối lại sau resume | cổng / thiết bị / persist |
| 4 | Thiết bị cứ ở Suspend | Runtime PM / remote wakeup |
| 5 | Cổng xHCI kẹt U3 | link USB 3.x |
| 6 | Cổng vào Recovery nhưng không bao giờ tới U0 | link USB 3.x / tín hiệu |
| 7 | Remote wakeup USB 2.0 không hoạt động | cấu hình wakeup / ngắt |
| 8 | Thiết bị USB 3.x không ra khỏi U3 | link / PHY |
| 9 | Thiết bị resume nhưng URB thất bại | driver / ring |
| 10 | Host controller treo khi resume | controller |
| 11 | PHY không lên nguồn | PHY / clock / regulator |
| 12 | Hệ thống dậy nhưng thiết bị USB không được phục hồi | thứ tự / async resume |
| 13 | Quartz: USB dùng ngay sau deep sleep bị lỗi | async resume của KM |
| 14 | Quartz: lỗi sau SLEEP1 (OP_BTS-71095) | không phải luồng resume Linux |

---

## 1. Bàn phím không dùng được sau system resume

| Mục | Nội dung |
|---|---|
| Ban đầu | Bàn phím chạy; hệ thống vào Suspend-to-RAM |
| Kích hoạt | hệ thống thức dậy |
| Luồng | PM core → controller resume (thường khởi tạo lại) → root hub → reset-resume bàn phím → `usbhid` `reset_resume` |
| Phần cứng | xHCI: HCRST, ring mới; cổng: reset → U0; thiết bị: địa chỉ mới |
| Log kỳ vọng | `root hub lost power or was reset` [đã xác minh]; `reset low-speed USB device number N using xhci-hcd` [đã xác minh định dạng] |
| Triệu chứng | gõ phím không có gì |
| Nguyên nhân | thiếu `reset_resume` trong driver; reset-resume thất bại (`device descriptor read/64, error -71`); controller không lên (`Controller not ready at resume`) |
| Debug | `dmesg` toàn bộ sau resume; `lsusb -t`; `cat /sys/bus/usb/devices/<dev>/power/runtime_status` |
| Xác nhận | có `USB disconnect` + `new … device` → bị enumerate lại; không có gì → controller hoặc driver không resume |

## 2. Chuột chạy rồi ngừng sau runtime resume

| Mục | Nội dung |
|---|---|
| Ban đầu | Chuột autosuspend (`power/control = auto`) |
| Kích hoạt | người dùng di chuột (remote wakeup) hoặc driver cần thiết bị |
| Luồng | remote wakeup → `usb_remote_wakeup` → resume → `usbhid` gửi lại URB interrupt |
| Log kỳ vọng | `usb wakeup-resume` (debug) |
| Triệu chứng | lần đầu chạy, sau vài giây dừng hẳn |
| Nguyên nhân | driver không gửi lại URB sau resume; usage count lệch (`put` thừa) nên thiết bị bị suspend khi đang dùng; thiết bị không remote wakeup được nhưng vẫn bị cho autosuspend |
| Debug | theo dõi `power/runtime_status`, `power/active_duration`; tạm đặt `power/control = on` |
| Xác nhận | đặt `on` mà hết lỗi → lỗi Runtime PM |

## 3. Thiết bị ngắt rồi kết nối lại sau resume

| Mục | Nội dung |
|---|---|
| Luồng | `usb_port_resume` → `finish_port_resume`: GET_STATUS lỗi → (persist) reset-resume → lỗi → `hub_port_logical_disconnect` |
| Log | `USB disconnect, device number %d` → `new %s USB device number %d using %s` [đã xác minh], cả hai ở mức info |
| Nguyên nhân | thiết bị mất nguồn/cấu hình khi ngủ (VBUS tắt, hub bị reset như Quartz); `power/persist = 0`; thiết bị đổi descriptor sau reset (`descriptors_changed`) |
| Debug | `cat power/persist`; xem log debug `gone after usb resume?` (bản build debug) |
| Xác nhận | bật persist mà hết → thiết bị mất trạng thái; vẫn lỗi → thiết bị không chịu reset đúng |

## 4. Thiết bị cứ ở Suspend

| Mục | Nội dung |
|---|---|
| Triệu chứng | `power/runtime_status = suspended` mãi, driver không nhận dữ liệu |
| Nguyên nhân | driver không gọi `usb_autopm_get_interface` trước khi dùng; thiết bị không phát remote wakeup (chưa bật, không hỗ trợ); `power/wakeup = disabled`; ngắt cổng không tới |
| Debug | `cat power/wakeup`; ép resume tạm bằng `echo on > power/control` |
| Xác nhận | ép `on` thì hoạt động → luồng đánh thức có vấn đề |

## 5. Cổng xHCI kẹt U3

| Mục | Nội dung |
|---|---|
| Phần cứng | PORTSC.PLS = 3 sau khi đã ra lệnh U0 |
| Log | `missing U0 port change event for port %d-%d` (debug); `port %d-%d resume PLC timeout` (warn) [đã xác minh] |
| Nguyên nhân | thiết bị không phản hồi LFPS; PHY/cổng chưa sẵn sàng; thiết bị đã bị rút trong lúc ngủ |
| Debug | `cat /sys/kernel/debug/usb/xhci/<ctrl>/ports/portNN/portsc` |
| Xác nhận | PLS không đổi sau nhiều lần thử → thử warm reset (rút/cắm lại) |

## 6. Cổng vào Recovery nhưng không bao giờ tới U0

| Mục | Nội dung |
|---|---|
| Phần cứng | PLS = 8 (Recovery) kéo dài, có thể rơi về 6 (SS.Inactive) và bật CAS |
| Nguyên nhân | tín hiệu SuperSpeed kém (cáp, connector, redriver); cấu hình PHY (equalization) |
| Log | `Cannot set port %d-%d link state %d` (warn); sau đó hub driver warm reset |
| Debug | đổi cáp/cổng; ép thiết bị chạy USB 2.0 để so sánh |
| Xác nhận | lỗi chỉ ở SuperSpeed → tầng vật lý SS |

## 7. Remote wakeup USB 2.0 không hoạt động

| Mục | Nội dung |
|---|---|
| Luồng kỳ vọng | thiết bị phát K → PORTSC Resume/RESUME + PLC/PCD → ngắt → 40 ms → C_SUSPEND → `usb_remote_wakeup` |
| Nguyên nhân | `power/wakeup = disabled`; thiết bị không khai báo/không được bật remote wakeup (`won't remote wakeup, status %d`, debug); ngắt bị tắt khi controller ngủ; PHY tắt hoàn toàn khi ngủ |
| Debug | `cat /sys/bus/usb/devices/<dev>/power/wakeup`; với controller: `cat /sys/devices/…/<ctrl>/power/wakeup`; xem `/proc/interrupts` có tăng khi di chuột |
| Xác nhận | không có ngắt → controller/PHY; có ngắt nhưng không resume → phần mềm |

## 8. Thiết bị USB 3.x không ra khỏi U3

Giống tình huống 5, cộng thêm: thiết bị có thể đánh thức (LFPS) nhưng thiếu **Function Wake**. Kiểm tra có Device Notification Event không (trace `xhci_handle_event`, cần FTRACE).

## 9. Thiết bị resume nhưng URB thất bại

| Mục | Nội dung |
|---|---|
| Triệu chứng | resume không lỗi, nhưng URB đầu tiên timeout/lỗi |
| Nguyên nhân | driver dùng lại URB/ring cũ sau **reset-resume** (ring đã được cấp mới); thiết bị chưa sẵn sàng (cần hơn 10 ms TRSMRCY); trạng thái thiết bị bị mất nhưng driver gọi `resume` thường thay vì `reset_resume` |
| Log | `xHCI … ERROR Transfer event TRB DMA ptr not part of current TD …` [đã xác minh]; `URB … submitted while active` [đã xác minh] |
| Debug | so sánh luồng `resume` và `reset_resume` của driver; chèn chờ thêm (thử nghiệm) |
| Xác nhận | lỗi chỉ sau reset-resume → driver |

## 10. Host controller treo khi resume

| Mục | Nội dung |
|---|---|
| Log | `Controller not ready at resume %d`, `WARN: xHC restore state timeout`, `xHC error in resume, USBSTS 0x%x, Reinit`, `Host not accessible, reset failed.` [đã xác minh] |
| Nguyên nhân | clock/nguồn của controller chưa bật; PHY chưa sẵn sàng; erratum controller; lỗi đọc thanh ghi (toàn bit 1) |
| Debug | debugfs `reg-op` (USBSTS: CNR, HCH, SRE, HCE); thứ tự resume clock/PHY trong driver platform |
| Xác nhận | USBSTS = 0xFFFFFFFF → controller mất truy cập (clock/nguồn) |

## 11. PHY không lên nguồn

| Mục | Nội dung |
|---|---|
| Log | `phy init failed --> %d`, `phy poweron failed --> %d` [đã xác minh] (framework PHY chung); Quartz: không có log lỗi, chỉ **thiếu** `mv_usb3_phy_resume_work: phy_usb3_mv_resume_fin[1]` |
| Triệu chứng | cổng không có kết nối sau resume; trên Quartz: USB resume chờ mãi |
| Nguyên nhân | regulator, clock, I2C tới PMU (Quartz: `PMU_I2cSet_*` thất bại), reset tín hiệu |
| Debug | `dmesg | grep -i phy`; trên Quartz tìm cặp log `phy_usb3_mv_resume_fin[0]` / `[1]` |

## 12. Hệ thống dậy nhưng thiết bị USB không được phục hồi

| Mục | Nội dung |
|---|---|
| Nguyên nhân | thứ tự: controller dậy trước PHY; con được resume trước cha; một thiết bị trong chuỗi resume bị treo, chặn các thiết bị sau |
| Quartz | KM resume USB **theo ngăn xếp**: thiết bị trên đỉnh ngăn xếp treo thì **mọi thiết bị khác chờ** (chính vì vậy có mã chi tiết riêng `usb_resume_queue_status`, `usb_resume_dev_pass`) |
| Debug | `dmesg` các dòng `usb_resume_work CALL` / `first wait release` / `usb_pm_stack.udev release` |
| Xác nhận | có `CALL` mà không có `release` cho một thiết bị → thiết bị đó chặn hàng đợi |

## 13. Quartz: dùng USB ngay sau deep sleep bị lỗi

| Mục | Nội dung |
|---|---|
| Luồng | PM core xong (`in suspend_finish` ★) **trước** khi USB thật sự resume (work 500 ms + PHY 300 + 100 ms + reset-resume) |
| Triệu chứng | ứng dụng truy cập thiết bị USB vài trăm ms sau khi thức dậy thì thất bại; một lúc sau lại tốt |
| Nguyên nhân | resume bất đồng bộ của KM |
| Debug | so thời gian `in suspend_finish` với `… usb_resume_fin[1]` |
| Xác nhận | lỗi biến mất khi ứng dụng chờ tới khi `usb_resume_fin` (PMU được báo qua `pmuSetUsbResumeFin`) |

## 14. Quartz: lỗi sau SLEEP1 (OP_BTS-71095)

| Mục | Nội dung |
|---|---|
| Ban đầu | máy vào SLEEP1 (không phải deep sleep) |
| Bằng chứng | **không có** `in pm_suspend` / `root hub lost power` lúc 07:05:51 |
| Kết luận | lỗi **không** nằm trong luồng System Resume của Linux (A–D Chương 6). Controller chạy liên tục từ lần resume lúc 07:02:45; FaxP chỉ unbind/bind driver |
| Debug | xem phân tích ring xHCI ở khoá USB DMA (Chương 5, 9) |

---

## Tóm tắt Chương 11

1. Luôn xác định **loại resume** (system/runtime/remote wakeup/host) trước khi đào sâu.
2. Thiết bị "biến mất rồi xuất hiện lại" = resume thất bại → enumerate lại.
3. Kẹt U3/Recovery là chuyện của link và tín hiệu; sysfs/debugfs `portsc` là công cụ chính.
4. Trên Quartz cần nhớ ba điều: resume USB **bất đồng bộ**, **theo ngăn xếp**, **chờ PHY**. SLEEP1 không phải resume của Linux.

## Câu hỏi kiểm tra

1. Với tình huống 1, làm sao phân biệt "driver thiếu reset_resume" và "controller không lên" chỉ bằng `dmesg`?
2. Tình huống 4 và 7 khác nhau thế nào về nơi xảy ra lỗi?
3. Vì sao tình huống 9 hay xảy ra sau reset-resume mà ít khi xảy ra sau resume thuần?
4. Trong tình huống 12 trên Quartz, thiết bị nào bị ảnh hưởng nếu bo FAX treo trong resume?
5. Thiết kế một kiểm thử tự động phát hiện tình huống 13.

---

**Chương tiếp theo:** Chương 12: cách debug Resume bằng công cụ, kèm cách ghép thời gian giữa các nguồn log.
