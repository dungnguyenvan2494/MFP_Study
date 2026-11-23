# Chương 8: Resume trong xHCI

> Khoá học USB Resume trên Linux Kernel. Baseline: **Linux 5.10.258** (`drivers/usb/host/xhci.h`, `xhci-hub.c`, `xhci-ring.c`, `xhci.c`). Board Quartz: hai controller xHCI, mỗi controller có một root hub USB 2.0 và một root hub USB 3.x.

## 8.0 Điểm khác cơ bản so với EHCI

| | EHCI | xHCI |
|---|---|---|
| Tốc độ | chỉ HS | **mọi tốc độ**: một controller có cổng USB 2.0 **và** cổng USB 3.x |
| Biểu diễn trạng thái cổng | bit SUSPEND/RESUME | trường **PLS (Port Link State)** dùng chung cho cả USB 2.0 và USB 3.x |
| Báo thay đổi cổng | bit USBSTS.PCD + ngắt | **Port Status Change Event** trên Event Ring |
| Linux thấy | một root hub | **hai root hub** cho mỗi controller (USB 2.0 và USB 3.x), ví dụ `usb3` và `usb4` |

---

## 8.1 Thanh ghi tham gia

| Thanh ghi | Bit | Vai trò |
|---|---|---|
| **USBCMD** | Run/Stop, **EIE** (cho phép ngắt), HSEE, CSS/CRS (lưu/khôi phục trạng thái) | bus resume **tạm tắt EIE** trong lúc thao tác cổng rồi bật lại |
| **USBSTS** | HCHalted, **HSE**, **EINT**, **PCD**, SSS/RSS, **SRE**, **CNR**, **HCE** | trạng thái controller, lỗi lưu/khôi phục |
| **PORTSC[n]** | (bảng dưới) | trạng thái và điều khiển cổng |
| **PORTPMSC[n]** | U1/U2 timeout (USB 3.x), L1/BESL (USB 2.0) | tiết kiệm điện mức link |
| Interrupter (IMAN, IMOD, ERDP) | | ngắt khi có Event, kể cả Port Status Change |

## 8.2 PORTSC: các bit (đã đối chiếu `xhci.h:296-410`)

| Bit | Tên trong kernel | Nghĩa | Kiểu |
|---|---|---|---|
| 0 | `PORT_CONNECT` (CCS) | có thiết bị | chỉ đọc |
| 1 | `PORT_PE` (PED) | cổng enabled. **Ghi 1 vào bit này sẽ tắt cổng**, nên tránh | RW1CS |
| 3 | `PORT_OC` | quá dòng | chỉ đọc |
| 4 | `PORT_RESET` (PR) | reset cổng | RW1S |
| 5–8 | **`PORT_PLS_MASK`** (PLS) | **Port Link State** (bảng 8.3) | ghi được khi kèm LWS |
| 9 | `PORT_POWER` (PP) | nguồn cổng | RW |
| 10–13 | `DEV_SPEED_MASK` | tốc độ thiết bị | chỉ đọc |
| 16 | **`PORT_LINK_STROBE`** (LWS) | **phải đặt kèm** khi ghi PLS, nếu không PLS không đổi | ghi |
| 17 | `PORT_CSC` | kết nối vừa đổi | RW1C |
| 18 | `PORT_PEC` | enable vừa đổi | RW1C |
| 19 | `PORT_WRC` | warm reset xong | RW1C |
| 20 | `PORT_OCC` | quá dòng vừa đổi | RW1C |
| 21 | `PORT_RC` | reset xong | RW1C |
| **22** | **`PORT_PLC`** | **link state vừa đổi**: tín hiệu trung tâm của Resume | RW1C |
| 23 | `PORT_CEC` | lỗi cấu hình link | RW1C |
| 24 | `PORT_CAS` | cổng kẹt Compliance/Inactive, cần Warm Reset | chỉ đọc |
| 25/26/27 | `PORT_WKCONN_E` / `PORT_WKDISC_E` / `PORT_WKOC_E` | đánh thức khi có kết nối / ngắt kết nối / quá dòng | RW |
| 30 | `PORT_DEV_REMOVE` | thiết bị không tháo được | |
| 31 | `PORT_WR` (WPR) | Warm Port Reset | RW1S |

**Quy tắc ghi PORTSC:** đọc → **đưa về "trung tính"** (`xhci_port_state_to_neutral`: bỏ các bit RW1C và PED khỏi giá trị) → sửa đúng bit cần → ghi. Không làm vậy thì có thể **xóa nhầm** một thay đổi chưa xử lý, hoặc **tắt nhầm** cổng.

## 8.3 Port Link State (PLS)

| Giá trị | Hằng số | Cổng USB 3.x | Cổng USB 2.0 | Liên quan Resume |
|---|---|---|---|---|
| 0 | `XDEV_U0` | U0 (hoạt động) | L0 (hoạt động) | đích của mọi lần resume |
| 1 | `XDEV_U1` | U1 | — | thoát nhanh, phần cứng tự lo |
| 2 | `XDEV_U2` | U2 | **L1** (LPM) | thoát nhanh/vừa |
| 3 | `XDEV_U3` | **U3 (suspend)** | **L2 (suspend)** | **trạng thái Suspend** |
| 4 | `XDEV_DISABLED` | Disabled | Disabled | |
| 5 | `XDEV_RXDETECT` | Rx.Detect | — | đang dò thiết bị |
| 6 | `XDEV_INACTIVE` | **SS.Inactive** | — | link hỏng, cần Warm Reset |
| 7 | `XDEV_POLLING` | Polling | Polling | đang huấn luyện |
| 8 | `XDEV_RECOVERY` | **Recovery** | — | **mọi lần thoát U1/U2/U3** |
| 9 | `XDEV_HOT_RESET` | Hot Reset | — | |
| 10 | `XDEV_COMP_MODE` | Compliance | — | kẹt ở đây là lỗi |
| 11 | `XDEV_TEST_MODE` | — | Test Mode | |
| 15 | `XDEV_RESUME` | **Resume** (thiết bị đánh thức) | **Resume (K)** | **đang resume** |

Loopback là trạng thái của LTSSM USB 3.x dùng cho kiểm thử. Nó không có hằng số riêng trong bảng `XDEV_*` của 5.10.258.

## 8.4 Các kiểu chuyển về U0

| Chuyển | Ai khởi động | Phần mềm làm gì | Kết thúc báo bằng |
|---|---|---|---|
| **U3 → U0** (USB 3.x, host) | phần mềm | ghi PLS = U0 + LWS, chờ tới 500 ms | PLC + PLS = U0 (`resume SS port … finished` [debug]); không thấy thì `missing U0 port change event …` [debug] |
| **U3 → Resume → U0** (USB 3.x, thiết bị) | thiết bị (LFPS) | thấy PLS = Resume → ghi U0 | PLC + U0 → `usb_wakeup_notification` |
| **U3 → Resume → U0** (USB 2.0, host) | phần mềm | ghi PLS = **Resume** → chờ 40 ms → ghi U0 → chờ thoát Resume (*RExit*) | PLC; quá hạn thì `Port resume timed out, port %d-%d: 0x%x` (warn) |
| **U3 → Resume → U0** (USB 2.0, thiết bị) | thiết bị (K) | thấy PLS = Resume → hẹn 40 ms → ghi U0 | PLC → C_SUSPEND cho hub driver |
| **U2/U1 → U0** | phần cứng (có lưu lượng) | **không làm gì** | không có Event riêng cho Linux; đây là tiết kiệm điện mức link |
| **Port Reset → U0** | phần mềm (PR hoặc WPR) | đặt PR (hot reset) hoặc WPR (warm reset) | PRC/WRC; thiết bị về địa chỉ 0 (**không phải resume**) |
| **Controller resume** | PM core | `xhci_resume`: khôi phục/khởi tạo lại controller | không phải chuyển trạng thái của cổng; cổng sẽ được resume sau, qua root hub |

## 8.5 Port Status Change Event: phần cứng báo cho phần mềm

```
 PORTSC có bit "*C" chuyển từ 0 lên 1 (CSC, PEC, WRC, OCC, PRC, PLC, CEC)
    └► controller ghi Port Status Change Event (port ID) vào Event Ring ──► ngắt
 xhci_irq ──► xhci_handle_event ──► handle_port_status (xhci-ring.c:1781):
   1. đọc PORTSC; "Port change event, %d-%d, id %d, portsc: 0x%x" [debug]
   2. root hub (hcd) đang SUSPENDED → "resume root hub" [debug] → usb_hcd_resume_root_hub
   3. USB 3.x và PLS = Inactive → đánh dấu thiết bị VDEV_PORT_ERROR
   4. PLC + PLS = Resume (thiết bị đánh thức):
        controller không Run → "xHC is not running." (warn) → bỏ
        cổng USB 3.x → "remote wake SS port %d" → ghi U0, chờ PLC tiếp theo
        cổng USB 2.0 → "resume HS port %d" → resume_timestamp = now + 40 ms, hẹn rh_timer
   5. PLC + USB 3.x + PLS ∈ {U0, U1, U2} → "resume SS port %d finished" → báo hoàn tất,
        bấm doorbell cho slot, nếu là remote wake → usb_wakeup_notification
   6. Cổng USB 2.0 đang chờ thoát Resume (rexit_active) → báo hoàn tất
   7. Còn lại: bật POLL_RH, gọi usb_hcd_poll_rh_status → hub driver đọc trạng thái cổng
```

**Lưu ý của chính driver:** Event chỉ sinh ra khi "OR" của mọi bit change chuyển từ 0 lên 1. Nếu một bit change cũ chưa được xóa, **thay đổi mới sẽ không sinh Event**. Vì vậy driver chuyển sang **poll** root hub sau mỗi Event.

## 8.6 Bus suspend và bus resume của root hub xHCI

| `xhci_bus_suspend` | `xhci_bus_resume` (đã đối chiếu `xhci-hub.c:1770-1880`) |
|---|---|
| Có cổng đang resume → bỏ (`suspend failed because a port is resuming` [debug]) | `HCD_HW_ACCESSIBLE` sai → `-ESHUTDOWN` |
| Cổng đang Polling → chờ; quá dòng/có connect change → bỏ (debug) | **tắt EIE** (hoãn ngắt) |
| Cổng U0 → ghi **U3**, nhớ vào `bus_suspended`; bật bit wake (CONN/DISC/OC) | với cổng trong `bus_suspended` và còn U3: USB 3.x ghi **U0**, USB 2.0 ghi **Resume** |
| | tắt bit wake |
| | USB 2.0: `msleep(USB_RESUME_TIMEOUT)` → ghi **U0** |
| | chờ **PLC** từng cổng, tối đa 10 ms. Quá hạn → `port %d-%d resume PLC timeout` (warn) |
| | bấm doorbell cho các slot, **bật lại EIE** |

## 8.7 Host-initiated resume cổng USB 3.x (SetPortFeature LINK_STATE = U0)

```
 hub driver: hub_set_port_link_state(U0)
   └► xhci_hub_control SetPortFeature(USB_PORT_FEAT_LINK_STATE, U0):
        PLS hiện tại = U0 → không làm gì
        PLS ∈ {U3, Resume, Recovery} → cần chờ U0
        PLS ≤ U3 → ghi PLS = U0 + LWS
        chờ tối đa 500 ms cho "đã về U0" (do handle_port_status báo)
        không thấy → "missing U0 port change event for port %d-%d" [debug]
```

## 8.8 Vì sao Resume của USB 3.x khác USB 2.0

| | USB 2.0 trên xHCI | USB 3.x trên xHCI |
|---|---|---|
| Lệnh resume | ghi PLS = **Resume** rồi, sau 40 ms, **U0** | ghi PLS = **U0** một lần |
| Thời gian do ai đếm | **phần mềm** (40 ms) | **phần cứng** (bắt tay LFPS + Recovery) |
| Remote wakeup | phần mềm phải hẹn giờ rồi kết thúc | phần mềm chỉ cần ghi U0 và chờ |
| Báo cho USB core | C_SUSPEND | link state change / Function Wake (Device Notification) |
| Lỗi đặc trưng | `Port resume timed out`, cổng kẹt Resume | kẹt U3/Recovery, rơi SS.Inactive (CAS) → Warm Reset |

## 8.9 Áp vào Quartz

- Bo FAX FK-513 ở `3-1.2` nằm trên **root hub USB 2.0** của `8d00d0000.usb3`. Các cổng USB 2.0 của xHCI dùng luồng **Resume → 40 ms → U0**.
- **Deep sleep:** controller khởi tạo lại, nên **không** có bus resume theo bảng 8.6. Thay vào đó: root hub mất nguồn → reset-resume.
- **SLEEP1:** không đi qua bus suspend/resume. Bo FAX được FaxP unbind/bind ở mức driver. Không có chuyển trạng thái U3/U0 do Linux điều khiển.

## 8.10 Tóm tắt Chương 8

1. xHCI dùng **PLS** cho cả USB 2.0 và USB 3.x; **U3** = Suspend; **Resume (15)** = đang resume; **PLC** báo link đổi trạng thái.
2. Ghi PLS phải kèm **LWS**, và phải đưa PORTSC về **trung tính** trước khi ghi.
3. Phần cứng báo bằng **Port Status Change Event**. Driver xử lý trong `handle_port_status` rồi chuyển sang poll root hub.
4. USB 2.0: phần mềm đếm 40 ms. USB 3.x: phần cứng tự huấn luyện, phần mềm chờ tới 500 ms.
5. U1/U2 do phần cứng tự quản. Port Reset không phải Resume. Controller resume là một tầng khác.

## Câu hỏi kiểm tra

1. Vì sao ghi PLS mà quên LWS thì cổng không đổi trạng thái?
2. Điều gì xảy ra nếu ghi lại nguyên giá trị đọc được của PORTSC mà không đưa về trung tính?
3. Cổng USB 2.0 của xHCI đang ở U3, thiết bị phát K. PLS sẽ chuyển thế nào? Phần mềm ghi gì, và khi nào?
4. Phân biệt `Port resume timed out` và `port … resume PLC timeout`: mỗi log ở luồng nào?
5. Vì sao xHCI driver phải poll root hub sau mỗi Port Status Change Event?
6. Thiết bị USB 3.x kẹt ở PLS = 6 sau resume. Bước tiếp theo của driver là gì?
7. U2 → U0 có sinh Event cho Linux không? Vì sao?
8. Trên Quartz, vì sao bảng 8.6 (bus resume) không áp dụng khi thoát deep sleep?

---

**Chương tiếp theo:** Chương 9 tổng hợp vai trò và quan hệ của các hàm quan trọng (USB core/hub, Runtime PM, EHCI, xHCI, PHY), xác minh sự tồn tại trong 5.10.258, kèm sơ đồ gọi.
