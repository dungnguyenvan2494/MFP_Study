# Chương 9: Các hàm quan trọng và quan hệ giữa chúng

> Khoá học USB Resume trên Linux Kernel. Baseline: **Linux 5.10.258**. Mỗi hàm được **kiểm tra có tồn tại** (file:dòng) trong `Kernel/Src`. Đây không phải phân tích từng dòng: chỉ nêu **vì sao được gọi, ai gọi, trước/sau, và phần cứng nào bị tác động**.

## 9.0 Bảng xác minh

| Hàm | Tồn tại trong 5.10.258 | Vị trí | Ghi chú |
|---|---|---|---|
| `usb_resume_device()` | có (static) | `core/driver.c:1334` | |
| `usb_port_resume()` | có | `core/hub.c:3770` | |
| `usb_remote_wakeup()` | có | `core/hub.c:3859` | |
| `hub_event()` | có (static) | `core/hub.c` (khai báo ở dòng 70) | work item của hub driver |
| `hub_port_connect_change()` | có (static) | `core/hub.c:5653` | |
| `hub_port_status()` | có (static) | `core/hub.c:656` | |
| `pm_runtime_get_sync()` | có (inline) | `include/linux/pm_runtime.h:392` | |
| `pm_runtime_resume()` | có (inline) | `pm_runtime.h:331` | |
| `pm_runtime_resume_and_get()` | có (inline) | `pm_runtime.h:405` | thêm vào mainline ở chu kỳ v5.10. Trên nhánh 5.4.x chỉ có nếu bản stable đã backport, cần kiểm tra cây 5.4.302 |
| `pm_runtime_put()` | có (inline) | `pm_runtime.h:425` | |
| `pm_runtime_mark_last_busy()` | có (inline) | `pm_runtime.h:198` | |
| `ehci_bus_resume()` | có (static) | `host/ehci-hub.c:376` | |
| `ehci_hub_control()` | có | `host/ehci-hub.c:870` | |
| `ehci_irq()` | có (static) | `host/ehci-hcd.c:712` | |
| `xhci_bus_resume()` | có | `host/xhci-hub.c:1770` | |
| `xhci_hub_control()` | có | `host/xhci-hub.c:1091` | |
| `xhci_irq()` | có | `host/xhci-ring.c:3029` | |
| `xhci_handle_event()` | có (static) | `host/xhci-ring.c:2918` | |
| `handle_port_status()` | có (static) | `host/xhci-ring.c:1781` | |
| `phy_power_on()` / `phy_power_off()` | có | `drivers/phy/phy-core.c:292/335` | framework PHY chung |
| `phy_set_mode()` | **macro** → `phy_set_mode_ext()` | `include/linux/phy/phy.h:220`, `phy-core.c:392` | |

Các hàm liên quan khác cũng được dùng trong chương: `usb_resume_both` (`driver.c:1596`), `usb_runtime_resume` (`driver.c:2220`), `usb_autoresume_device` (`driver.c:1954`), `usb_autopm_get_interface` (`driver.c:2062`), `finish_port_resume` (`hub.c:3608`), `check_port_resume_type` (`hub.c:3283`), `hub_handle_remote_wakeup` (`hub.c:3877`), `port_event` (`hub.c:5764`), `hub_activate` (`hub.c:1064`), `hub_resume` (`hub.c:4023`), `hcd_bus_resume` (`hcd.c:2195`), `hcd_resume_work` (`hcd.c:2265`), `usb_hcd_resume_root_hub` (`hcd.c:2282`), `xhci_set_link_state` (`xhci-hub.c:681`), `xhci_resume` (`xhci.c:1115`).

---

## 9.1 Nhóm USB core / hub

| Hàm | Vì sao được gọi | Ai gọi | Trước đó | Sau đó | Phần cứng |
|---|---|---|---|---|---|
| `usb_resume_device()` | resume **mức thiết bị**: gọi `resume` của *device driver* (driver generic) | `usb_resume_both` | thiết bị `SUSPENDED` hoặc cần reset-resume | resume từng interface | gián tiếp (qua generic driver → cổng) |
| `usb_port_resume()` | đưa **cổng** ra khỏi suspend và xác nhận thiết bị còn sống | generic driver (`usb_generic_driver_resume`) | cổng suspend (hoặc đã tự resume do remote wakeup) | `finish_port_resume` → interface resume | ClearPortFeature(SUSPEND) hoặc Set LINK_STATE U0 → PORTSC |
| `usb_remote_wakeup()` | phản ứng với **thiết bị tự đánh thức** | `hub_handle_remote_wakeup`; `hcd_resume_work` (cho root hub) | phát hiện C_SUSPEND / link về U0 | `usb_autoresume_device` → luồng resume bình thường | không trực tiếp |
| `hub_event()` | work xử lý **mọi sự kiện cổng** của một hub | `kick_hub_wq` (từ `hub_irq` của hub ngoài hoặc từ poll root hub) | có bit trong `event_bits` | `port_event` cho từng cổng | đọc trạng thái cổng |
| `port_event()` | phân loại thay đổi của một cổng | `hub_event` | | remote wakeup → `hub_handle_remote_wakeup`; connect change → `hub_port_connect_change`; cần warm reset… | GetPortStatus, ClearPortFeature(C_*) |
| `hub_port_connect_change()` | cổng **đổi kết nối**: thiết bị mới, rút, hoặc "hồi sinh" thiết bị cũ | `port_event` | CSC, hoặc remote wakeup thất bại | thiết bị đang SUSPENDED + persist → coi là **remote wakeup** (`usb_remote_wakeup`); không được → ngắt kết nối và enumerate lại | reset cổng nếu enumerate |
| `hub_port_status()` | đọc trạng thái + thay đổi của cổng | hầu hết hàm hub | | | GetPortStatus → HCD đọc PORTSC |
| `hub_handle_remote_wakeup()` | nhận ra một cổng vừa remote wakeup | `port_event` | USB 2.0: có C_SUSPEND; USB 3.x: link U0/U1/U2 khi thiết bị SUSPENDED | `msleep(10)` → `usb_remote_wakeup`; thất bại → đánh dấu connect change | xóa C_SUSPEND |
| `finish_port_resume()` | hoàn tất: đặt lại state, GET_STATUS, reset-resume nếu cần, tắt remote wakeup | `usb_port_resume` | cổng đã hoạt động | | control transfer EP0 |
| `hub_activate()` | khởi động lại hub sau resume/reset-resume: kiểm tra từng cổng | `hub_resume`, `hub_reset_resume`… | hub vừa dậy | đặt `event_bits`/`wakeup_bits` → `hub_event` | GetPortStatus từng cổng |

### Sơ đồ gọi (remote wakeup qua hub ngoài)

```
 [hub ngoài] endpoint interrupt báo "cổng N thay đổi"
   hub_irq ──► event_bits ──► kick_hub_wq ──► hub_event
                                               └► port_event(N)
                                                    ├► hub_handle_remote_wakeup ──► usb_remote_wakeup
                                                    │                                └► usb_autoresume_device
                                                    │                                    └► usb_runtime_resume
                                                    │                                        └► usb_resume_both
                                                    │                                            └► usb_resume_device
                                                    │                                                └► usb_port_resume
                                                    │                                                    └► finish_port_resume
                                                    └► hub_port_connect_change (nếu wakeup thất bại / kết nối đổi)
```

---

## 9.2 Nhóm Runtime PM

| Hàm | Vì sao | Ai gọi | Tác dụng |
|---|---|---|---|
| `pm_runtime_get_sync(dev)` | "tôi cần thiết bị, đánh thức đi" | driver, USB core (`usb_autopm_get_interface`) | usage +1; nếu đang ngủ thì **resume đồng bộ** (cha trước). **Bẫy:** khi resume lỗi, usage **vẫn bị tăng**, nên vẫn phải `put` |
| `pm_runtime_resume_and_get(dev)` | như trên nhưng an toàn hơn | `usb_port_resume` (cho *port device*), driver mới | resume lỗi thì **tự giảm usage lại** |
| `pm_runtime_resume(dev)` | resume mà **không** tăng usage | | dùng khi chỉ cần đánh thức |
| `pm_runtime_put(dev)` | "tôi xong rồi" | driver | usage −1; về 0 thì hẹn idle/autosuspend (bất đồng bộ) |
| `pm_runtime_mark_last_busy(dev)` | ghi lại lúc dùng gần nhất | driver | đồng hồ autosuspend tính lại từ thời điểm này |

```
 driver: get ──► (usage 0→1) ──► rpm_resume ──► cha đang ngủ? resume cha trước ──► callback runtime_resume
 driver: mark_last_busy + put_autosuspend ──► (usage 1→0) ──► hẹn timer autosuspend_delay ──► runtime_suspend
```

---

## 9.3 Nhóm EHCI

| Hàm | Vì sao | Ai gọi | Phần cứng |
|---|---|---|---|
| `ehci_irq()` | xử lý ngắt: hoàn tất transfer, **Port Change Detect**, lỗi nghiêm trọng | IRQ core | đọc/xóa USBSTS. PCD → nếu root hub ngủ thì `usb_hcd_resume_root_hub`; cổng đang Resume → bắt đầu đếm 40 ms |
| `ehci_hub_control()` | giả lập lệnh hub cho root hub | USB core (`usb_hcd_submit_urb` tới root hub → `hub_control`) | ClearPortFeature(SUSPEND) → đặt `PORT_RESUME`; GetPortStatus → kết thúc Resume, chờ RESUME về 0 |
| `ehci_bus_resume()` | resume **root hub** | `hcd_bus_resume` | ghi lại địa chỉ schedule, bật RUN, resume các cổng do bus suspend |

## 9.4 Nhóm xHCI

| Hàm | Vì sao | Ai gọi | Phần cứng |
|---|---|---|---|
| `xhci_irq()` | ngắt của controller | IRQ core | đọc USBSTS (HSE, EINT), xử lý Event Ring |
| `xhci_handle_event()` | phân loại từng Event | `xhci_irq` | đọc Event TRB |
| `handle_port_status()` | xử lý **Port Status Change Event** | `xhci_handle_event` | đọc PORTSC; đánh thức root hub; xử lý PLS = Resume (USB2: đếm 40 ms; USB3: ghi U0); PLC→U0; poll root hub |
| `xhci_hub_control()` | giả lập lệnh hub cho root hub | USB core | Set/ClearPortFeature, GetPortStatus → ghi/đọc PORTSC (qua `xhci_set_link_state`) |
| `xhci_bus_resume()` | resume **root hub** | `hcd_bus_resume` | tắt EIE; U3 → U0 (USB3) / U3 → Resume → U0 (USB2); chờ PLC; bật EIE |
| `xhci_resume()` | resume **controller** | `xhci_plat_resume` / `xhci_plat_runtime_resume` | khôi phục/khởi tạo lại (xem `docs/diagrams/xhci-resume-sequence.md`) |

### Sơ đồ gọi (remote wakeup trên cổng root hub xHCI, USB 2.0)

```
 IRQ ──► xhci_irq ──► xhci_handle_event ──► handle_port_status
                                              ├ root hub ngủ? usb_hcd_resume_root_hub ──► wakeup_work
                                              │                 └► hcd_resume_work ──► usb_remote_wakeup(root hub)
                                              │                       └► … ──► hcd_bus_resume ──► xhci_bus_resume
                                              └ PLS=Resume (USB2): hẹn rh_timer 40 ms
 rh_timer ──► usb_hcd_poll_rh_status ──► xhci_hub_status_data ──► kick hub driver
 hub_event ──► port_event ──► hub_port_status ──► xhci_hub_control(GetPortStatus)
                                                    └ đủ 40 ms: xhci_set_link_state(U0), chờ RExit → C_SUSPEND
            ──► hub_handle_remote_wakeup ──► usb_remote_wakeup(thiết bị) ──► … usb_port_resume
```

---

## 9.5 Nhóm PHY

| Hàm | Framework | Vai trò | Trên Quartz |
|---|---|---|---|
| `phy_init()` / `phy_exit()` | PHY chung (`drivers/phy`) | khởi tạo/giải phóng | xHCI Quartz không dùng cho root hub (dùng `usb_phy` kiểu cũ) |
| `phy_power_on()` / `phy_power_off()` | PHY chung | cấp/cắt nguồn PHY; HCD core gọi qua `usb_phy_roothub_*` khi suspend/resume root hub (`hcd.c:2168/2209`) | chỉ khi controller có PHY khai báo theo framework chung |
| `phy_set_mode()` | PHY chung | đặt chế độ (host/device, USB2/USB3) | |
| `usb_phy_init()` / `usb_phy_set_suspend()` | **USB PHY kiểu cũ** (`drivers/usb/phy`) | khởi tạo/ngủ PHY | **Quartz dùng**: `xhci-plat.c` lấy PHY theo `usb-phy`/`usb2-phy` và gọi `usb_phy_init` |
| `mv_usb3_phy_suspend/resume` | PM ops của driver PHY (KM) | system sleep của PHY | resume trì hoãn 300 ms + 100 ms, nhả `/RST_SLP2`, đặt `phy_usb3_mv_resume_fin` |

**Lỗi PHY sẽ hiện ra thế nào:**
- framework chung: `phy init failed --> %d`, `phy poweron failed --> %d` (`phy-core.c:250/315`);
- trên Quartz: PHY chưa xong thì USB core **chờ** (cờ `phy_usb3_mv_resume_fin`), nên triệu chứng là **resume chậm hoặc treo**, không phải log lỗi.

---

## 9.6 Tóm tắt Chương 9

1. Mọi hàm trong danh sách đều có trong 5.10.258. Riêng `pm_runtime_resume_and_get` được thêm vào mainline ở v5.10 (trên 5.4.x phải kiểm tra xem bản stable có backport không), còn `phy_set_mode` là macro.
2. Hai trục chính:
   - **USB core/hub**: `usb_resume_both` → `usb_resume_device` → `usb_port_resume` → `finish_port_resume`;
   - **sự kiện**: `xhci_irq` → `handle_port_status` → poll root hub → `hub_event` → `port_event` → `hub_handle_remote_wakeup` → `usb_remote_wakeup`.
3. HCD chỉ "giả lập hub" (`hub_control`) và thực thi bus suspend/resume. Quyết định nằm ở USB core.
4. `pm_runtime_get_sync` vẫn tăng usage khi resume lỗi. `pm_runtime_resume_and_get` thì không.
5. Quartz dùng **USB PHY kiểu cũ** và có đồng bộ PHY riêng của KM.

## Câu hỏi kiểm tra

1. Hàm nào nhận ra một remote wakeup trên cổng của hub ngoài? Nó được gọi từ đâu?
2. `hub_port_connect_change` có thể dẫn tới `usb_remote_wakeup` trong trường hợp nào?
3. Vì sao viết `if (pm_runtime_get_sync(dev) < 0) return err;` là sai? Cách viết đúng là gì?
4. Vẽ chuỗi gọi từ một ngắt xHCI tới `usb_port_resume` cho remote wakeup trên cổng USB 2.0 của root hub.
5. `xhci_hub_control` và `xhci_bus_resume` khác nhau thế nào về phạm vi?
6. Trên Quartz, nếu PHY USB3 không bao giờ đặt `phy_usb3_mv_resume_fin = true`, triệu chứng sẽ là gì?

---

**Chương tiếp theo:** Chương 10 phân tích log Resume theo sáu nhóm A–F, mỗi log theo chuỗi *Log → Hàm in ra → Sự kiện kích hoạt → Trạng thái phần cứng → Nguyên nhân gốc*.
