# Chương 13: So sánh tổng kết

> Khoá học USB Resume trên Linux Kernel. Baseline: **Linux 5.10.258**.

## 13.1 Suspend và Resume

| | Suspend | Resume |
|---|---|---|
| Mục đích | tiết kiệm điện | trở lại hoạt động mà không enumerate lại |
| USB 2.0 | bus **im lặng** ≥ 3 ms | trạng thái **K** ≥ 20 ms (Linux 40 ms) + 10 ms phục hồi |
| USB 3.x | link → **U3** theo lệnh | LFPS → **Recovery** → **U0** |
| Thứ tự trong cây | **con trước cha** | **cha trước con** |
| Thiết bị giữ | địa chỉ, cấu hình | như cũ (resume), hoặc làm lại (reset-resume) |
| Driver | `suspend`: dừng việc, hủy URB | `resume` hoặc `reset_resume` |
| Lỗi hay gặp | `-EBUSY`, cổng đang resume | kẹt Resume/U3, thiết bị biến mất, controller không lên |

## 13.2 USB 2.0 Resume và USB 3.x Resume

| | USB 2.0 | USB 3.x |
|---|---|---|
| Tín hiệu | J/K/SE0 trên D+/D− | LFPS, huấn luyện TS1/TS2 |
| Trạng thái ngủ | Suspend (L2); L1 (LPM) | U3; U1/U2 (tự động) |
| Thời gian ai đếm | **phần mềm** (40 ms) | **phần cứng** (bắt tay + Recovery) |
| Lệnh xHCI | PLS = Resume, rồi U0 | PLS = U0 |
| Remote wakeup | K từ thiết bị, upstream giữ tiếp | LFPS (link) + Function Wake (Device Notification) |
| Lỗi đặc trưng | `Port resume timed out`, `port … resume error` | kẹt U3/Recovery, SS.Inactive, cần Warm Reset |

## 13.3 Host-initiated và device-initiated Resume

| | Host-initiated | Device-initiated (Remote Wakeup) |
|---|---|---|
| Ai muốn | phần mềm host | thiết bị (người dùng tác động) |
| Điều kiện | không | thiết bị hỗ trợ **và** host đã bật (SET_FEATURE) **và** `power/wakeup = enabled` |
| Phần mềm biết qua | chính nó gọi | **Port Status Change** (xHCI) / **PCD** (EHCI) → `C_SUSPEND` / link về U0 |
| `usb_port_resume` | có bước ClearSuspend | **bỏ** bước ClearSuspend |
| Hàm then chốt | `usb_port_resume` | `hub_handle_remote_wakeup` → `usb_remote_wakeup` |
| Có thể đánh thức cả hệ thống | không | có (nếu controller là nguồn đánh thức) |

## 13.4 Runtime Resume và System Resume

| | Runtime Resume | System Resume |
|---|---|---|
| Phạm vi | một thiết bị (và cha của nó) | **mọi** thiết bị |
| Kích hoạt | usage count > 0, remote wakeup | cả máy thức dậy |
| Callback | `runtime_resume` | `resume_noirq` → `resume_early` → `resume` → `complete` |
| Controller | thường khôi phục được | có thể mất nguồn → **khởi tạo lại** |
| Kết quả cho thiết bị | thường resume thuần | thường **reset-resume** (Quartz: luôn) |
| Quartz | **tắt** mặc định (KM `usb_disable_autosuspend`) | mỗi **deep sleep**; USB resume **bất đồng bộ** sau 500 ms |

## 13.5 USB Device Resume và Host Controller Resume

| | USB Device Resume | Host Controller Resume |
|---|---|---|
| Đối tượng | `usb_device` + cổng của nó | xHCI/EHCI + root hub |
| Thao tác chính | cổng Resume/U0, GET_STATUS, driver resume | khôi phục thanh ghi/trạng thái, hoặc HCRST + cấp lại ring/context |
| Hàm | `usb_resume_both`, `usb_port_resume`, `finish_port_resume` | `xhci_resume` / `ehci_resume`, rồi `xhci_bus_resume` / `ehci_bus_resume` |
| Dấu hiệu | `usb … resume`, `finish resume` (debug) | `root hub lost power or was reset` (nếu khởi tạo lại) |
| Phụ thuộc | cần controller và root hub đã chạy | cần clock, PHY, nguồn |

## 13.6 EHCI Resume và xHCI Resume

| | EHCI | xHCI |
|---|---|---|
| Phạm vi | chỉ USB 2.0 HS | mọi tốc độ, hai root hub |
| Bit trạng thái cổng | `PORT_SUSPEND`, `PORT_RESUME` (Force Port Resume) | trường **PLS** (U0/U3/Resume…) + **LWS** |
| Bit "đã đổi" | `STS_PCD` toàn cục + CSC/PEC | **PLC**, CSC, PEC… mỗi cổng |
| Báo phần mềm | ngắt PCD, phần mềm quét cổng | **Port Status Change Event** trên Event Ring |
| Kết thúc Resume | xóa RESUME, chờ về 0 trong 2 ms | ghi U0, chờ thoát Resume / PLC |
| Bus resume | ghi lại địa chỉ schedule, RUN, RESUME các cổng | tắt EIE, U0/Resume các cổng, chờ PLC, bật EIE |
| Controller resume | khôi phục thanh ghi, có nhánh "after power loss" | CRS/khôi phục **hoặc** khởi tạo lại (SRE/HCE/quirk) |
| Lỗi tiêu biểu | `port %d resume error %d` | `Port resume timed out`, `port … resume PLC timeout`, `xHC error in resume` |

## 13.7 Bản đồ một trang: đọc log Resume trên Quartz

```
 Thấy "in pm_suspend" … "root hub lost power or was reset" … "in suspend_finish"
     → System Resume (deep sleep). Controller KHỞI TẠO LẠI. Mọi thiết bị REset-resume, BẤT ĐỒNG BỘ sau ~500 ms.
       Lỗi USB ngay sau đó? → kiểm tra mốc usb_resume_fin, thứ tự ngăn xếp, PHY fin.
 Không thấy các dòng trên nhưng có "Now in SLEEP1"/"leave_sleep_1"
     → KHÔNG phải resume của Linux. Controller chạy liên tục. Tìm lỗi ở tầng khác (driver, ring, thiết bị).
 Thấy "Port resume timed out" / "resume PLC timeout"
     → cổng không ra khỏi Resume/U3: PHY, hub, thiết bị.
 Thấy "USB disconnect" + "new … USB device number" sau resume
     → resume/reset-resume thất bại, thiết bị được enumerate lại.
```

## 13.8 Tóm tắt toàn khoá

1. Có **sáu tầng** (thiết bị, cổng, hub, controller, PHY, hệ thống) và **bốn luồng** (host-initiated, remote wakeup, runtime, system). Đừng trộn chúng.
2. **USB core quyết định, HCD thực thi, phần cứng phát tín hiệu.**
3. USB 2.0: K ≥ 20 ms (Linux 40 ms), phục hồi 10 ms. USB 3.x: U3 → LFPS → Recovery → U0.
4. xHCI: **PLS + LWS + PLC + Port Status Change Event**. EHCI: **SUSPEND + RESUME + PCD**.
5. Trên Quartz: deep sleep = System Suspend thật, xHCI khởi tạo lại, reset-resume, resume USB **bất đồng bộ theo ngăn xếp**, **chờ PHY** (300 + 100 ms, `/RST_SLP2`), Runtime PM USB **tắt**. SLEEP1 không phải resume của Linux.

## Câu hỏi kiểm tra cuối khoá

1. Vẽ lại từ trí nhớ luồng remote wakeup trên cổng USB 2.0 của root hub xHCI, ghi rõ ai giữ tín hiệu K và ai đếm 40 ms.
2. Liệt kê năm khác biệt giữa resume trên Quartz và resume upstream, và hệ quả của từng cái.
3. Bạn được giao ticket "chuột USB không chạy sau khi máy thức dậy" trên một SoC có EHCI. Viết kế hoạch debug năm bước, kèm log/sysfs/debugfs cần xem.
4. Vì sao `xhci_bus_resume` tắt EIE trong lúc thao tác cổng?
5. Giải thích vì sao `finish_port_resume` tắt remote wakeup ngay sau khi resume.

---

**Kết thúc khoá học.** Toàn bộ 13 chương được tổng hợp trong tài liệu HTML tra cứu (sidebar, mục lục, tìm kiếm, thuật ngữ).
