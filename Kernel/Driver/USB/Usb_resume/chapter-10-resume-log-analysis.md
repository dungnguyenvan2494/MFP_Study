# Chương 10: Phân tích log Resume

> Khoá học USB Resume trên Linux Kernel. Baseline: **Linux 5.10.258**. Câu chữ mọi log **đã kiểm tra trong source** (file:dòng). Ví dụ có tham số là **[minh họa]** về giá trị. **★** = đã thấy trong log thật của Quartz.
>
> Mức hiển thị:
> - **debug** = không in được trên bản build Quartz (`CONFIG_DYNAMIC_DEBUG` tắt);
> - **info** = có trong `dmesg`, console Quartz không hiện;
> - **notice/warn/err** = thấy trên console.

## 10.0 Chuỗi phân tích cho mọi log

```
 LOG ──► HÀM IN RA ──► SỰ KIỆN KÍCH HOẠT ──► TRẠNG THÁI PHẦN CỨNG ──► NGUYÊN NHÂN GỐC
```

Mỗi thẻ log trả lời 9 câu: **nghĩa · ai in · sự kiện · trước · sau · phần cứng · nguyên nhân · mức · kiểm tra tiếp**.

**Điều quan trọng nhất về Quartz:** gần như **toàn bộ log của một lần resume bình thường là debug**. Trên console bình thường bạn chỉ thấy `root hub lost power or was reset` và các printk của KM/PMU. **Không thấy log không có nghĩa là không có chuyện gì xảy ra.**

---

## A. Log của Resume bình thường

| Log | Nguồn | Mức | Có xuất hiện khi |
|---|---|---|---|
| `usb %sresume` → `usb resume` / `usb auto-resume` / `usb wakeup-resume` | `hub.c:3807`, `3865` | debug | resume cổng (host) / Runtime PM / remote wakeup |
| `finish resume` / `finish reset-resume` | `hub.c:3614` | debug | cuối mọi resume thiết bị |
| `resume, status %d` | `hub.c:3916` | debug | sau xử lý remote wakeup trên cổng |
| `Port change event, %d-%d, id %d, portsc: 0x%x` | `xhci-ring.c` | debug | mỗi Port Status Change Event |
| `resume root hub` | `xhci-ring.c` / `ehci-hub.c:404` | debug | root hub đang ngủ khi có sự kiện cổng |
| `port resume event for port %d` | `xhci-ring.c` | debug | PLS = Resume (thiết bị đánh thức) |
| `resume HS port %d` / `resume USB2 port %d-%d` | `xhci-ring.c` / `xhci-hub.c:864` | debug | remote wakeup / kết thúc Resume trên cổng USB 2.0 |
| `remote wake SS port %d` / `resume SS port %d finished` | `xhci-ring.c` | debug | remote wakeup / hoàn tất trên cổng USB 3.x |
| `port %d remote wakeup` | `ehci_irq` | debug | EHCI remote wakeup |
| ★ `root hub lost power or was reset` | `hub.c:4062` | **notice** | controller khởi tạo lại (system resume trên Quartz) |
| `reset %s USB device number %d using %s` | `hub.c:4994` | info | **reset-resume**: thiết bị được reset và gán lại địa chỉ |
| `USB disconnect, device number %d` + `new %s USB device number …` | `hub.c:2305`, `4994` | info | resume thất bại → ngắt kết nối logic → enumerate lại |
| `… phy_usb3_mv_resume_fin[%d]`, `xhci_resume :xhci_resume_fin [%d]`, `usb_resume CALL`, `usb_resume_work CALL`, `… first wait release`, `… usb_pm_stack.udev release`, `… usb_resume_fin[%d]` | KM: `phy-usb3-mv.c`, `xhci.c`, `core/driver.c` | info | system resume trên Quartz |

**Những gì *không nhất thiết* xuất hiện trong một lần resume tốt:**
- `reset … USB device number` (chỉ khi reset-resume);
- `USB disconnect` (chỉ khi resume thất bại);
- mọi log debug.

Không có `root hub lost power` nghĩa là controller **không** khởi tạo lại (resume thuần, hoặc không có system resume nào xảy ra).

### Thẻ ★ `usb usbN: root hub lost power or was reset`

| Mục | Nội dung |
|---|---|
| Nghĩa | Root hub (và mọi thứ dưới nó) phải coi như đã mất nguồn → đánh dấu reset-resume |
| Ai in | `usb_root_hub_lost_power` (`hub.c:4060`) |
| Sự kiện | `xhci_resume` (hoặc `ehci_resume`) đi nhánh **khởi tạo lại**: quirk reset-on-resume, hibernate, hoặc lỗi SRE/HCE |
| Trước | system resume (Quartz: sau `PMU_resume:…`, `mvri_resume`) |
| Sau | xHCI khởi tạo lại; sau đó (Quartz: 500 ms) reset-resume từng thiết bị: `reset … USB device number …` |
| Phần cứng | controller đã bị HCRST, ring/context mới |
| Nguyên nhân | bình thường trên Quartz (DT `xhci_reset_on_resume`) |
| Mức | **bình thường** trên Quartz. Trên hệ không có quirk mà xuất hiện thì cần xem có `xHC error in resume` trước đó không |
| Kiểm tra tiếp | đếm: trên Quartz số dòng = 4 × số lần deep sleep (log OP_BTS-71095: 4644 = 4 × 1161) |

---

## B. Lỗi Resume của USB 2.0

| Log | Nguồn | Mức |
|---|---|---|
| `Port resume timed out, port %d-%d: 0x%x` | `xhci-hub.c:893` | **warn** |
| `port %d resume error %d` | `ehci-hub.c:1068` | **err** |
| `can't resume, status %d` (cổng / thiết bị) | `hub.c:3804`, `3844` | debug |
| `can't resume usb port, status %d` | `hub.c:3781` | debug |
| `gone after usb resume? status %d` | `hub.c:3662` | debug |
| `retry with reset-resume` | `hub.c:3655` | debug |
| `won't remote wakeup, status %d` | `hub.c:3518` | debug |
| `Waited %dms for CONNECT` | `hub.c:3732` | debug |
| `Cannot enable. Maybe the USB cable is bad?` | `hub.c:3191` | **err** |
| `device descriptor read/64, error %d` | `hub.c:5074` | **err** |
| `device not accepting address %d, error %d` | `hub.c:5095` | **err** |

### Thẻ `Port resume timed out, port %d-%d: 0x%x`

| Mục | Nội dung |
|---|---|
| Nghĩa | Cổng USB 2.0 của xHCI **không thoát** trạng thái Resume về U0 trong thời gian chờ |
| Ai in | `xhci-hub.c` khi kết thúc Resume trong GetPortStatus |
| Sự kiện | đã đủ 40 ms Resume, phần mềm ghi U0, chờ báo "đã thoát Resume" (`XHCI_MAX_REXIT_TIMEOUT_MS`) |
| Trước | `resume USB2 port %d-%d` (debug) |
| Sau | cổng vẫn bị báo là **suspended** cho USB core (`rexit_active` giữ nguyên) → `usb_port_resume` thất bại → thiết bị bị **ngắt kết nối logic** |
| Phần cứng | PORTSC in kèm (`0x…`): đọc PLS (bit 5–8) xem có còn 15 (Resume) hoặc 3 (U3) không; CCS còn 1 không |
| Nguyên nhân | thiết bị không phản hồi kết thúc Resume; PHY chưa sẵn sàng; hub ngoài đang reset; thiết bị mất nguồn |
| Mức | **lỗi** |
| Kiểm tra tiếp | giải mã PORTSC; xem có system resume vừa xảy ra không; trên Quartz: PHY USB3/`/RST_SLP2` đã xong chưa |

### Thẻ `port %d resume error %d` (EHCI)

| Mục | Nội dung |
|---|---|
| Nghĩa | Bit `PORT_RESUME` không về 0 trong 2 ms sau khi phần mềm xóa |
| Sự kiện | GetPortStatus khi đủ thời gian Resume |
| Phần cứng | PORTSC.RESUME = 1, SUSPEND = 1: cổng kẹt trong Resume |
| Nguyên nhân | PHY ở chế độ tiết kiệm điện chưa dậy; lỗi controller; thiết bị kéo dây |
| Kiểm tra tiếp | `has_tdi_phy_lpm`/PHY; thử resume lại; đo dây |

### Thẻ `gone after usb resume? status %d` (debug) và ngắt kết nối logic

| Mục | Nội dung |
|---|---|
| Nghĩa | Cổng đã resume, nhưng **GET_STATUS** tới thiết bị thất bại |
| Sau | nếu có persist thì `retry with reset-resume`; vẫn lỗi thì `can't resume` → `hub_port_logical_disconnect` → **`USB disconnect, device number N`** → enumerate lại (`new … USB device number M`) |
| Nguyên nhân | thiết bị mất cấu hình/nguồn khi ngủ; thiết bị chậm sau resume (cần hơn 10 ms) |
| Nhận ra trên console | chỉ thấy `USB disconnect` + thiết bị mới (info, cần `dmesg`) |

---

## C. Lỗi Resume của xHCI

| Log | Nguồn | Mức |
|---|---|---|
| `port %d-%d resume PLC timeout` | `xhci-hub.c:1864` | **warn** |
| `missing U0 port change event for port %d-%d` | `xhci-hub.c:1351` | debug |
| `Cannot set port %d-%d link state %d` | `xhci-hub.c:1316` | **warn** |
| `USB core suspending port %d-%d not in U0/U1/U2` | `xhci-hub.c:1224` | **warn** |
| `xHC is not running.` | `xhci-ring.c:1851` | **warn** |
| `Controller not ready at resume %d` | `xhci.c:1152` | **warn** (resume thất bại) |
| `WARN: xHC restore state timeout` | `xhci.c:1173` | **warn** (resume thất bại) |
| `xHC error in resume, USBSTS 0x%x, Reinit` | `xhci.c:1185` | **warn** (tự khởi tạo lại) |
| `WARN: xHC CMD_RUN timeout` / `WARN: xHC save state timeout` | `xhci.c:1047` / `1078` | **warn** (lúc suspend) |
| `Host halt failed, %d` / `Host not accessible, reset failed.` / `Host controller not halted, aborting reset.` | `xhci.c:122/180/185` | **warn** |
| `WARNING: Host System Error` | `xhci-ring.c:3052` | **fatal** |
| `xHCI host controller not responding, assume dead` → `HC died; cleaning up` | `xhci-ring.c:1115`, `hcd.c:2396` | **fatal** |
| `<thiết bị>: failed to resume: error %d` / `failed to resume async: error %d` | PM core `drivers/base/power/main.c:458` | **err** |

### Thẻ `port %d-%d resume PLC timeout`

| Mục | Nội dung |
|---|---|
| Nghĩa | Trong **bus resume** của root hub, cổng đã được ra lệnh về U0 nhưng không báo PLC trong 10 ms |
| Ai in | `xhci_bus_resume` |
| Trước | ghi U0 (USB 3.x) hoặc Resume + 40 ms + U0 (USB 2.0) |
| Sau | bỏ qua cổng đó (không bấm doorbell cho slot); hub driver sẽ phát hiện cổng lạ sau này |
| Phần cứng | PLS còn U3/Recovery/Resume; link chưa huấn luyện xong |
| Nguyên nhân | thiết bị/hub chậm thoát U3; tín hiệu kém; PHY chưa sẵn sàng; thiết bị bị rút trong lúc ngủ |
| Kiểm tra tiếp | debugfs `ports/portNN/portsc` ngay sau resume; thiết bị còn cắm không |

### Thẻ `xHC is not running.`

| Mục | Nội dung |
|---|---|
| Nghĩa | Có Port Status Change "Resume" (thiết bị đánh thức) nhưng controller **không ở trạng thái Run** |
| Trước | controller đang suspend/halt mà vẫn sinh Event (đánh thức hệ thống bằng USB) |
| Sau | Event bị bỏ (`goto cleanup`); remote wakeup có thể bị mất |
| Nguyên nhân | thứ tự resume: Event tới trước khi `xhci_resume` bật Run; controller bị halt do lỗi |
| Kiểm tra tiếp | thời điểm so với `xhci_resume`; USBSTS |

### Thẻ `xHC error in resume, USBSTS 0x%x, Reinit`

| Mục | Nội dung |
|---|---|
| Nghĩa | Khôi phục trạng thái thất bại (**SRE**) hoặc có lỗi nội bộ (**HCE**) → khởi tạo lại |
| Sau | `root hub lost power or was reset` → mọi thiết bị reset-resume |
| Mức | cảnh báo, hệ thống tự cứu. Lặp lại thường xuyên là dấu hiệu controller mất nguồn ngoài ý muốn |
| Quartz | không thấy dòng này vì luôn khởi tạo lại theo quirk, không qua nhánh khôi phục |

### Thẻ `<dev>: failed to resume: error %d` (PM core)

| Mục | Nội dung |
|---|---|
| Nghĩa | Callback resume của **một thiết bị** trả lỗi trong system resume |
| Ai in | `pm_dev_err` (`main.c:458`), câu chữ `failed to %s%s: error %d` (`%s` = `resume`, ` async`, ` early`, ` noirq`) |
| Hệ quả | thiết bị (và có thể các con) không hoạt động sau resume |
| **Riêng Quartz** | `usb_resume` (KM) **luôn trả 0** vì chỉ hẹn work. Lỗi resume thiết bị USB **không** hiện ở đây; chỉ lỗi của controller (`xhci_plat_resume`) hoặc PHY mới hiện |

---

## D. Lỗi Resume của EHCI

| Log | Nguồn | Mức | Gợi ý |
|---|---|---|---|
| `port %d resume error %d` | `ehci-hub.c:1068` | err | thẻ ở mục B |
| `Port status(0x%x) is wrong` | `ehci-hub.c:432` | debug | cổng enabled mà không suspend/resume lúc bus resume |
| `suspend failed because a port is resuming` | `ehci-hub.c:241` | debug | bus suspend bị hủy vì cổng đang resume |
| `resume root hub after power loss` | `ehci-hub.c:404` | debug | controller mất nguồn khi ngủ |
| `fatal error` → `HC died; cleaning up` | `ehci-hcd.c:844` | fatal | USBSTS Host System Error, thường do địa chỉ schedule sai sau resume |
| (không có ngắt PCD) | | | ngắt PCD bị tắt / USBINTR sai / controller không có nguồn khi ngủ → remote wakeup mất, **không có log** |

---

## E. Lỗi Runtime PM

| Log | Nguồn | Mức | Nghĩa |
|---|---|---|---|
| `runtime PM trying to activate child device %s but parent (%s) is not active` | `drivers/base/power/runtime.c:1276` | err | đặt con thành active khi cha còn ngủ: sai thứ tự |
| `Unbalanced pm_runtime_enable!` (`Unbalanced %s!`) | `runtime.c:1468` | warn | enable/disable lệch cặp |
| `resume error %d` (interface) | `core/driver.c:1434` | err | callback `resume`/`reset_resume` của **driver** trả lỗi |
| `suspend error %d` (interface) | `core/driver.c:1383` | err | driver từ chối suspend |
| `no reset_resume for driver %s?` | `core/driver.c:1428` | debug | driver thiếu `reset_resume` → sẽ bị unbind/bind |
| `remote wakeup needed for autosuspend` | `core/driver.c:2172` | debug | không cho autosuspend vì thiết bị không remote wakeup được |
| `HCD doesn't handle wakeup requests` | `core/driver.c:2183` | debug | controller không hỗ trợ đánh thức |

| Triệu chứng không có log riêng | Nguyên nhân |
|---|---|
| thiết bị không bao giờ ngủ | thiếu `put`; `power/control = on` (trên Quartz là mặc định); driver không `supports_autosuspend` |
| thiết bị ngủ khi đang dùng | `put` thừa |
| mọi thao tác runtime PM đều lỗi | một lần callback trước lỗi → `power.runtime_error` khác 0 (xem `power/runtime_status` = `error`) |
| `-EBUSY` khi suspend | còn URB, driver bận |

---

## F. Lỗi PHY

| Log | Nguồn | Mức |
|---|---|---|
| `phy init failed --> %d` | `drivers/phy/phy-core.c:250` | err |
| `phy poweron failed --> %d` | `phy-core.c:315` | err |
| `phy exit failed --> %d` / `phy poweroff failed --> %d` | `phy-core.c:279/346` | err |
| `can't register transceiver, err: %d` | `drivers/usb/phy/phy-usb3-mv.c` | err (lúc probe) |
| `mv_usb3_phy_suspend: phy_usb3_mv_resume_fin[0]` / `mv_usb3_phy_resume_work: phy_usb3_mv_resume_fin[1]` | `phy-usb3-mv.c` (KM) | info |

| Triệu chứng | Liên hệ |
|---|---|
| Clock chưa bật | `xhci_plat_resume` bật clock trước. Lỗi thì trả lỗi → PM core `failed to resume` |
| Regulator / nguồn | thường hiện thành cổng không có kết nối sau resume, hoặc PORTSC toàn 0 |
| PHY chưa xong (Quartz) | USB core **chờ** cờ `phy_usb3_mv_resume_fin`, không có log lỗi; triệu chứng: resume USB chậm/treo, có `… first wait release` muộn |
| Link không lên (USB 3.x) | `port … resume PLC timeout`, `Cannot set port … link state`, PLS = Inactive/Compliance |

---

## 10.7 Log-to-flow mẫu

### `Port resume timed out, port 3-2: 0x…3e1` [minh họa giá trị]

```
 [Hàm] xhci-hub.c GetPortStatus, nhánh kết thúc Resume USB 2.0
 [Sự kiện] hub driver đọc trạng thái cổng sau 40 ms (host resume hoặc remote wakeup)
 [Phần cứng] giải mã PORTSC: PLS = (0x3e1 >> 5) & 0xF = 15 → VẪN Resume; CCS = 1; PE = 0
 [Suy luận] controller đã được ra lệnh U0 nhưng không thoát Resume → PHY/thiết bị không hoàn tất
 [Nguyên nhân gốc] PHY chưa sẵn sàng sau system resume, hoặc hub/thiết bị đang bị reset
```

### Thiết bị biến mất rồi xuất hiện lại sau resume

```
 console: (không có gì)
 dmesg:   usb 3-1.2: USB disconnect, device number 92
          usb 3-1.2: new high-speed USB device number 93 using xhci-hcd
 [Hàm] hub_port_logical_disconnect ← usb_port_resume (can't resume) ← finish_port_resume (gone after usb resume?)
 [Sự kiện] GET_STATUS thất bại sau resume, reset-resume cũng thất bại (hoặc driver không có reset_resume)
 [Nguyên nhân] thiết bị mất cấu hình/nguồn; driver thiếu reset_resume; thiết bị chậm
```

## 10.8 Tóm tắt Chương 10

1. Trên Quartz, resume bình thường **gần như không để lại log trên console**. Dùng `dmesg`, và cần bản build debug để thấy luồng chi tiết.
2. `root hub lost power or was reset` là **bình thường** trên Quartz (mỗi deep sleep 4 dòng).
3. Lỗi cổng: `Port resume timed out` (xHCI USB 2.0), `port … resume PLC timeout` (bus resume), `port … resume error` (EHCI).
4. Lỗi controller: `Controller not ready at resume`, `restore state timeout`, `xHC error in resume … Reinit`, `Host System Error`, `assume dead`.
5. Lỗi Runtime PM thường **không có log**: phải xem sysfs.
6. Trên Quartz, PM core **không báo** lỗi resume của thiết bị USB, vì `usb_resume` của KM luôn trả 0.

## Câu hỏi kiểm tra

1. Bạn chỉ thấy `root hub lost power or was reset` trên console sau mỗi lần thoát deep sleep. Hệ thống có lỗi không? Vì sao?
2. Giải mã PORTSC `0x00000e63` của một cổng USB 2.0 ngay sau `Port resume timed out`: CCS, PE, PLS, PP và tốc độ là gì? Cổng đang kẹt ở đâu?
3. Vì sao sau resume, thiết bị có thể "biến mất rồi xuất hiện lại" với số thiết bị mới? Những log nào cho thấy điều này?
4. Phân biệt `Controller not ready at resume` và `xHC error in resume … Reinit` về hậu quả.
5. Trên Quartz, vì sao lỗi resume của bo FAX không bao giờ hiện thành `failed to resume: error …`?
6. Lỗi Runtime PM nào có log, lỗi nào không? Với lỗi không có log, xem ở đâu?

---

**Chương tiếp theo:** Chương 11 trình bày 12+ tình huống debug Resume thực tế.
