# Chương 2: Kiến trúc quản lý nguồn USB

> Khoá học USB Resume trên Linux Kernel. Baseline: **Linux 5.10.258**, board Quartz (xHCI platform `8d00d0000.usb3`, `8d00e0000.usb2`, PHY Marvell `phy-utm-mv`, `phy-usb3-mv`, bộ quản lý nguồn PMU riêng của KM).

## 2.0 Vấn đề

Một lần "đánh thức" chạm tới rất nhiều thành phần: chip trong thiết bị, cổng hub, PHY, controller, driver, khung quản lý nguồn của Linux, thậm chí chip quản lý nguồn của SoC. Nếu không biết **mỗi thành phần chịu trách nhiệm gì**, ta sẽ không biết đọc log nào và hỏi ai khi lỗi.

---

## 2.1 Các thành phần, giải thích từng cái

| Thành phần | Tiếng Việt | Là gì | Trách nhiệm trong Suspend/Resume |
|---|---|---|---|
| **USB Device** | thiết bị USB | bàn phím, ổ đĩa, bo FAX… | tự vào Suspend khi bus im lặng (USB 2.0) hoặc khi link vào U3 (USB 3.x); phát Remote Wakeup nếu được phép |
| **USB Bus** | đường truyền USB | cặp dây D+/D− (USB 2.0) và các cặp SuperSpeed (USB 3.x) | mang tín hiệu Resume (K) hoặc LFPS |
| **USB Port** | cổng | một điểm nối trên hub | có trạng thái riêng: connected, enabled, **suspended**, link state; giữ các bit "đã thay đổi" |
| **Root Hub** | hub gốc | hub "ảo" nằm trong host controller; các cổng của nó là cổng vật lý trên board | báo thay đổi cổng cho USB core; khi tất cả cổng ngủ thì chính nó cũng ngủ |
| **External Hub** | hub ngoài | chip hub thật (ví dụ `045b:0209` ở `3-1` trên Quartz) | chuyển tiếp Resume giữa thiết bị và host; báo thay đổi cổng qua endpoint interrupt của nó |
| **USB Host Controller** | bộ điều khiển host | xHCI/EHCI | thực thi thao tác cổng (suspend/resume/reset), phát tín hiệu, báo sự kiện; tự lưu và khôi phục trạng thái |
| **USB Core** | lõi USB | `drivers/usb/core` | quyết định *khi nào* thiết bị ngủ/dậy, giữ thứ tự, gọi driver |
| **Hub driver** | driver hub | phần của USB core (`hub.c`) | xử lý sự kiện cổng, gửi lệnh suspend/resume/reset tới cổng, phát hiện remote wakeup |
| **USB Driver** | driver thiết bị | usbhid, usb-storage, usbfax… | dừng/khởi động lại công việc của mình trong callback `suspend`/`resume`/`reset_resume` |
| **USB PHY** | lớp vật lý | khối analog chuyển bit thành tín hiệu điện | phải được cấp điện/clock lại **trước** khi controller nói chuyện được với bus |
| **CPU** | bộ xử lý | | nhận ngắt, chạy code; khi cả hệ thống ngủ thì CPU cũng tắt |
| **Power Management framework** | khung quản lý nguồn | PM core của Linux: System PM, Runtime PM, wakeup | gọi callback theo đúng thứ tự cây thiết bị; đếm "ai đang dùng" thiết bị |
| **PMU của SoC** (riêng Quartz) | bộ quản lý nguồn | `mdels800_pmu` | cắt/cấp nguồn miền, ghi nhận nguồn đánh thức (log `PMU_resume:WAKE_…`) |

---

## 2.2 Sơ đồ kiến trúc

```
 ╔═════════════════════════════ PHẦN MỀM (Linux) ═══════════════════════════════════╗
 ║  Ứng dụng / hệ thống (echo mem > /sys/power/state, FaxP…)                           ║
 ║        │ yêu cầu ngủ cả hệ thống                                                      ║
 ║  ┌─────▼──────────── PM core ─────────────────────────────────────────────┐        ║
 ║  │ System PM: suspend/resume theo thứ tự cây thiết bị                     │        ║
 ║  │ Runtime PM: đếm usage, autosuspend sau N giây rảnh, autoresume khi cần │        ║
 ║  │ Wakeup: thiết bị nào được phép đánh thức                               │        ║
 ║  └─────┬──────────────────────────────┬───────────────────────────────────┘        ║
 ║        │ callback dev_pm_ops           │                                             ║
 ║  ┌─────▼──────────── USB core ────────▼─────────────┐   ┌──────────────────────┐   ║
 ║  │ usb_device / usb_interface PM                     │   │ USB class driver      │   ║
 ║  │ Hub driver: suspend/resume CỔNG, remote wakeup    │◄──┤ suspend / resume /    │   ║
 ║  │ Root hub (hcd): bus_suspend / bus_resume          │   │ reset_resume          │   ║
 ║  └─────┬─────────────────────────────────────────────┘   └──────────────────────┘   ║
 ║        │ hub_control (Set/ClearPortFeature), bus_suspend/resume, suspend/resume HCD  ║
 ║  ┌─────▼──────── HCD: xHCI / EHCI driver ─────────────┐  ┌──────────────────────┐   ║
 ║  │ ghi PORTSC, USBCMD; xử lý Port Status Change Event │──┤ PHY framework         │   ║
 ║  │ lưu/khôi phục controller                            │  │ phy_power_on/off…     │   ║
 ║  └─────┬─────────────────────────────────────────────┘   └─────────┬────────────┘   ║
 ╚════════╪═══════════════════════════════════════════════════════════╪═════════════════╝
          │ MMIO, ngắt                                                │ thanh ghi PHY, clock, regulator
 ╔════════▼═══════════════════════ PHẦN CỨNG ═════════════════════════▼═════════════════╗
 ║  Host Controller (xHCI) ── Root Hub ports ── PHY ── dây USB ── Hub ngoài ── Thiết bị  ║
 ║                                     ▲                                                 ║
 ║                       PMU của SoC: nguồn, clock, nguồn đánh thức hệ thống             ║
 ╚═══════════════════════════════════════════════════════════════════════════════════════╝
```

**Nguyên tắc vàng:** *USB core quyết định, HCD thực thi, phần cứng phát tín hiệu.*
- Quyết định "cổng 3 phải resume" là của **hub driver**.
- Việc ghi bit vào PORTSC là của **HCD**.
- Việc kéo dây D+/D− thành trạng thái K, hoặc phát LFPS, là của **controller và PHY**.

---

## 2.3 Sáu tầng quản lý nguồn: đừng trộn lẫn

| Tầng | Đối tượng | Trạng thái chính | Ai điều khiển | Ví dụ |
|---|---|---|---|---|
| **Device-level** | `usb_device`, `usb_interface` | active / suspended (runtime), `USB_STATE_SUSPENDED` | USB core + Runtime PM | autosuspend bàn phím sau 2 giây rảnh |
| **Port-level** | một cổng trên hub | enabled / **suspended** (USB 2.0); link **U0/U1/U2/U3** (USB 3.x); port power | hub driver qua Set/ClearPortFeature | `PORT_SUSPEND` trên EHCI, `PLS = U3` trên xHCI |
| **Hub-level** | một hub (kể cả root hub) | hub suspended khi mọi cổng con ngủ | hub driver | hub `3-1` ngủ khi bo FAX ngủ |
| **Host controller** | xHCI/EHCI | running / halted / suspended / **lost power** | HCD + PM core | `xhci_suspend` / `xhci_resume` |
| **PHY** | khối vật lý | powered / low-power / off | PHY driver, HCD hoặc glue platform | `phy-usb3-mv` resume (Quartz có cờ `phy_usb3_mv_resume_fin`) |
| **System-level** | cả máy | running / suspend-to-RAM / hibernate | PM core | deep sleep của Quartz = `pm_suspend` |

Quan hệ giữa các tầng là quan hệ **cha–con**: con phải ngủ trước khi cha ngủ, và cha phải dậy trước khi con dậy.

```
   Ngủ:   thiết bị ──► cổng của nó ──► hub ──► … ──► root hub ──► host controller ──► PHY ──► hệ thống
   Dậy:   hệ thống ──► PHY ──► host controller ──► root hub ──► hub ──► cổng ──► thiết bị ──► driver
```

---

## 2.4 Ví dụ trên Quartz: cây thiết bị USB và các tầng

```
 Hệ thống (PMU, CPU)
 └─ xHCI 8d00d0000.usb3   (platform device, Runtime PM + System PM)
    ├─ usb3 (root hub USB 2.0 của controller này)
    │  └─ cổng 1 ─► 3-1  hub 045b:0209
    │                 ├─ cổng 2 ─► 3-1.2  FK-513 (bo FAX)   ← driver usbfax
    │                 └─ …
    └─ usb4 (root hub USB 3.x của controller này)
       └─ cổng 1 ─► 4-1 …
 └─ xHCI 8d00e0000.usb2 ─ usb1 / usb2 …
 PHY: phy-utm-mv (USB 2.0), phy-usb3-mv (USB 3.x)
```

(Số bus có thể khác giữa các lần boot. Hãy xác nhận bằng `lsusb -t`.)

**Khi Quartz vào deep sleep:**
1. FaxP gỡ driver khỏi `3-1.2` (log `echo -n 3-1.2 > /sys/bus/usb/drivers/usb/unbind`).
2. Linux `pm_suspend` → suspend thiết bị → hub → root hub → xHCI.
3. PMU cắt nguồn.

**Khi thức dậy**, đi ngược lại. xHCI bị khởi tạo lại (quirk reset-on-resume), nên mọi thiết bị dưới nó đều **reset-resume**.

---

## 2.5 Ai quyết định, ai thực hiện

| Câu hỏi | Ai quyết định | Ai thực hiện |
|---|---|---|
| Thiết bị rảnh đủ lâu, cho ngủ | Runtime PM (bộ đếm) + USB core | hub driver gửi lệnh suspend cổng → HCD ghi PORTSC → phần cứng ngừng gửi gói / đưa link vào U3 |
| Driver cần gửi URB tới thiết bị đang ngủ | driver gọi `usb_autopm_get_interface` → Runtime PM | USB core → hub driver resume cổng → HCD → phần cứng phát Resume |
| Thiết bị muốn đánh thức host | **thiết bị** (nếu được phép remote wakeup) | thiết bị phát K/LFPS → controller phát hiện → HCD báo → hub driver hoàn tất |
| Cả hệ thống ngủ / dậy | người dùng / ứng dụng / PMU | PM core gọi callback theo cây thiết bị |
| Controller có giữ trạng thái qua giấc ngủ không | phần cứng và platform (quirk reset-on-resume) | HCD: khôi phục, hoặc khởi tạo lại |

---

## 2.6 Tóm tắt Chương 2

1. Mười một thành phần tham gia. **USB core quyết định, HCD thực thi, phần cứng phát tín hiệu.**
2. Sáu tầng quản lý nguồn: **thiết bị, cổng, hub, host controller, PHY, hệ thống**. Mỗi tầng có trạng thái riêng.
3. Thứ tự luôn là: con ngủ trước cha, cha dậy trước con.
4. Trên Quartz có thêm **PMU** của SoC và các cờ đồng bộ resume của KM (Chương 6).

## Câu hỏi kiểm tra

1. Root hub là phần cứng hay phần mềm? Cổng của root hub nằm ở đâu?
2. Khi bàn phím được autosuspend, những tầng nào đổi trạng thái? Tầng nào không?
3. Vì sao PHY phải "dậy" trước host controller trong lúc resume?
4. Trong thao tác "resume cổng 3-1.2", ai quyết định, ai ghi thanh ghi, ai phát tín hiệu điện?
5. Vì sao hub `3-1` không thể ngủ khi bo FAX ở `3-1.2` còn đang hoạt động?
6. Ở Quartz, thành phần nào quyết định hệ thống có bị đánh thức hay không khi đang deep sleep?

---

**Chương tiếp theo:** Chương 3 tách riêng sáu loại Suspend/Resume: thiết bị, cổng, root hub, host controller, hệ thống, runtime. Với mỗi loại: cái gì kích hoạt, ai xử lý, trước/trong/sau, và cái gì có thể hỏng.
