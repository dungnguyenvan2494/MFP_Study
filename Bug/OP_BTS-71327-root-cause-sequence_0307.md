# OP_BTS-71327: sequence diagram root cause

Bug: 2-port LAN, sau khi thoát ErP thì không ping được NW2 (eth1, LAN mở rộng). Log re-open 11/11 (F00-W4(03)), có đối chiếu với log 26/10 (F00-W4(01)).
Báo cáo đầy đủ: [`OP_BTS-71327_analysis.html`](../../OP_BTS-71327_analysis.html).

Nguồn đối chiếu:

- `Kernel/Src/drivers/usb/core/hub.c`: `hub_activate` 1064 (nhánh persist 1274–1287), `usb_disconnect` 2312, `hub_port_connect` 5527, `usb_new_device` 2651, `usb_port_resume` 3800
- `Kernel/Src/drivers/usb/core/quirks.c:743–755`: persist mặc định, ngoại lệ KM cho 8812BU
- `Kernel/Src/drivers/usb/core/driver.c`: `usb_resume` 1766, `usb_resume_interface` 1388 (gọi `reset_resume` ở 1419)
- `Kernel/Src/drivers/net/usb/lan78xx.c`: `lan78xx_disconnect` 3876 (`unregister_netdev`), `lan78xx_probe` 3967 (`phy_init` 4089, `register_netdev` 4093), `lan78xx_reset_resume` 4661
- `arch/arm64/configs/EagleZp_defconfig:3416`: `# CONFIG_USB_DEFAULT_PERSIST is not set`
- Log: `SysLogALL/messages` (11/11) dòng 728–862, `垂れ流しログ` dòng 2728–2890, ifconfig ở `202611110907_001.log` dòng 127405–127521

## Thành phần tham gia

| Tên | Vai trò |
|---|---|
| **Power** | `sys:PowerControl` trong `mfp000_hwQt`: vào và ra ErP (`setErpMode` → `req_DeepSleepShift`) |
| **mio/NIC** | Middleware mạng (mio, NicFum, ncLi). Cấu hình eth0/eth1 lúc boot, khôi phục mạng khi wake (`mioPostChangeModeToNormal`). Không có source trong repo |
| **PM core** | `pm_suspend` / resume hệ thống của kernel |
| **Hub 4-1** | Hub driver của USB core cho hub SS `4-1` (045b:0210), gồm `hub_activate`, `hub_wq` (`hub_event`) |
| **LAN7800** | Chip LAN USB `0424:7800` ở cổng `4-1.4` (phần cứng) |
| **lan78xx** | Driver netdev cho LAN7800 |
| **eth1** | `net_device` của NW2 (IP 192.168.43.118/23) |
| **WLAN** | RTL8812BU `0bda:b812` ở `3-1.2`, dùng làm đối chứng |

---

## Sơ đồ 1: Root cause tổng quan (11/11, F00-W4(03))

```mermaid
sequenceDiagram
    autonumber
    actor U as Người dùng
    participant PWR as Power (app)
    participant MIO as mio/NIC (app)
    participant PM as PM core
    participant HUB as Hub 4-1 / hub_wq
    participant HW as LAN7800 (4-1.4)
    participant DRV as lan78xx
    participant NET as eth1

    Note over MIO,NET: Boot: lan78xx probe → eth1 → mio/NIC ifup + IP + route (09:02:32–43)
    U->>PWR: Giữ phím nguồn phụ (09:04:48)
    PWR->>MIO: ncLiDeepSleepStart / mioPreChangeModeToSleep
    PWR->>PM: req_DeepSleepShift() → pm_suspend
    PM->>HUB: suspend (4-1.4, 4-1, usb4)
    Note over HUB,HW: ErP: USB mất nguồn "root hub lost power or was reset"
    U->>PWR: Bấm phím nguồn phụ (wake)
    PM->>HUB: hub_reset_resume() → hub_activate(HUB_RESET_RESUME)
    HUB->>HUB: Cổng 4: persist_enabled == 0 "power session is gone"
    HUB->>HW: usb_set_device_state(NOTATTACHED)
    PM->>DRV: usb_resume (state=0) → lan78xx resume
    DRV-->>PM: Failed to read register 0x120 ret=-19
    PM-->>PWR: suspend exit (183.86 s)
    PWR->>PWR: req_DeepSleepShift() End (09:05:35.59)
    HUB->>HUB: hub_event → hub_port_connect()
    HUB->>DRV: usb_disconnect(4-1.4) → lan78xx_disconnect()
    DRV->>NET: unregister_netdev(eth1)
    Note over DRV,NET: eth1 cũ bị huỷ: IP, route, UP đều mất
    PWR->>MIO: mioPostChangeModeToNormal (09:05:35.872 → 36.157)
    Note over MIO,PM: Khôi phục mạng sau sleep. Lúc này eth1 chưa tồn tại
    HUB->>HW: Enumerate lại: new SuperSpeed device number 4 (184.66 s)
    HUB->>DRV: usb_new_device() → lan78xx_probe()
    DRV->>HW: lan78xx_phy_init(): PHY autoneg → đèn link SÁNG
    DRV->>NET: register_netdev() → eth1 MỚI (DOWN)
    Note over MIO,NET: Không có ai ifup / gán IP / thêm route cho eth1 mới
    U->>NET: ping 192.168.43.118 → NG
```

**Cách đọc:** Bước 7–8 là nơi kernel quyết định bỏ thiết bị, vì LAN7800 không có USB-persist. Bước 14–15 là hệ quả của fix OP_BTS-69103: disconnect giờ chạy được nên eth1 cũ bị huỷ. Bước 16 (`mio` khôi phục mạng) chạy xong trước khi eth1 mới được tạo ở bước 17–20, và sau đó không có trigger nào cấu hình lại eth1 mới.

---

## Sơ đồ 2: Nhánh quyết định trong `hub_activate`: WLAN sống sót, LAN7800 bị bỏ

```mermaid
sequenceDiagram
    autonumber
    participant XHCI as xHCI / root hub
    participant HUB as hub_activate()
    participant QK as quirks.c (lúc enumerate)
    participant WL as WLAN 3-1.2 (0bda:b812)
    participant LAN as LAN7800 4-1.4 (0424:7800)

    Note over HUB,QK: Lúc boot (usb_detect_quirks) CONFIG_USB_DEFAULT_PERSIST = n
    QK->>WL: persist_enabled = 1 (ngoại lệ CONFIG_KM_BIZHUB)
    QK->>LAN: persist_enabled = 0 (không có ngoại lệ)
    Note over XHCI: Resume sau ErP
    XHCI->>HUB: root hub lost power → reset-resume các hub con
    loop Mỗi cổng có thiết bị con
        alt udev->persist_enabled == 1
            HUB->>WL: reset_resume = 1 (hub.c:1276)
            WL->>WL: usb_port_resume → usb_reset_and_verify_device → driver->reset_resume()
            Note over QK,WL: "reset high-speed USB device number 4" Thiết bị và netdev giữ nguyên
        else persist_enabled == 0
            HUB->>LAN: usb_set_device_state(NOTATTACHED) (hub.c:1286)
            HUB->>HUB: set_bit(change_bits) → chờ hub_wq
            Note over WL,LAN: "usb_resume CALL state=0" → disconnect + enumerate lại
        end
    end
```

---

## Sơ đồ 3: So sánh trước và sau fix OP_BTS-69103

```mermaid
sequenceDiagram
    autonumber
    participant HUB as hub_wq
    participant DRV as lan78xx
    participant NET as eth1
    participant MIO as mio/NIC
    actor PC as PC (ping NW2)

    Note over HUB,NET: Chung: 4-1.4 = NOTATTACHED sau ErP resume
    alt 26/10 · F00-W4(01) (trước fix 69103)
        HUB--xHUB: hub_event bị chặn trong lúc resume (!usb_resume_fin)
        Note over DRV,NET: Không disconnect → eth1 CŨ vẫn UP + 192.168.43.118
        MIO->>NET: Truy cập eth1 định kỳ
        NET->>DRV: đọc register
        DRV-->>NET: ret = -19 (lặp lại ≥ 12 lần)
        Note over DRV: Thiết bị chết → đèn link TẮT
        PC->>NET: ping → NG
    else 11/11 · F00-W4(03) (sau fix 69103)
        HUB->>DRV: usb_disconnect → lan78xx_disconnect
        DRV->>NET: unregister_netdev (IP, route mất)
        HUB->>DRV: enumerate lại → lan78xx_probe
        DRV->>NET: register_netdev → eth1 MỚI, flags=4098 (DOWN)
        Note over DRV: phy_init khi probe → đèn link SÁNG
        MIO--xNET: (không có bước cấu hình lại)
        PC->>NET: ping → NG
    end
```

**Cách đọc:** Cả hai lần đều có chung gốc là bước "NOTATTACHED". Trước fix 69103 thì thiết bị chết mà netdev cũ vẫn còn. Sau fix thì thiết bị chạy lại, nhưng netdev mới không được cấu hình. Triệu chứng đèn link vì thế đảo ngược (tắt thành sáng), còn ping thì vẫn NG.

---

## Sơ đồ 4: Race giữa khôi phục mạng của app và re-enumerate của kernel (11/11)

Quy đổi thời gian: kernel `184.631414` ↔ wall `09:05:36.332` (垂れ流しログ dòng 2888).

```mermaid
sequenceDiagram
    autonumber
    participant PWR as Power
    participant MIO as mio/NIC
    participant K as Kernel USB / lan78xx
    participant NET as eth1

    K->>K: 183.26 s · 4-1.4 state=0, đọc reg -19
    K->>NET: 183.86 s (≈09:05:35.56) · USB disconnect → unregister eth1
    PWR->>PWR: 09:05:35.593 · req_DeepSleepShift() End
    PWR->>MIO: 09:05:35.872 · mioPostChangeModeToNormal ←
    MIO-->>PWR: 09:05:36.157 · mioPostChangeModeToNormal → (code=0)
    Note over MIO,NET: mio kết thúc khi eth1 không tồn tại
    K->>K: 184.63 s (09:05:36.33) · port4 U0
    K->>NET: 184.67 s (≈09:05:36.37) · enumerated 0424:7800 → register eth1 mới
    Note over PWR,NET: 09:05:36 → 09:07 (hết log): không có "8021q … eth1", "Link is Up" hay dòng lan78xx nào nữa
    Note over K,NET: SysLog 09:07: eth1 DOWN, không có IP, không có route NW2, UDP vẫn bind 192.168.43.118:137/138
```

---

## Sơ đồ 5: Phương án A (BSP): bật persist cho LAN7800, giữ nguyên eth1

```mermaid
sequenceDiagram
    autonumber
    participant QK as quirks.c
    participant HUB as hub_activate()
    participant CORE as usb_port_resume / usb_resume_interface
    participant DRV as lan78xx
    participant NET as eth1

    QK->>QK: 0424:7800 → persist_enabled = 1 (thêm như 8812BU)
    Note over HUB: ErP resume, root hub lost power
    HUB->>HUB: Cổng 4: persist_enabled → reset_resume = 1
    CORE->>CORE: usb_reset_and_verify_device(4-1.4)
    alt Reset và verify descriptor OK
        CORE->>DRV: driver->reset_resume() = lan78xx_reset_resume()
        DRV->>DRV: lan78xx_reset(): nạp lại MAC, features, multicast
        DRV->>DRV: phy_start() → lan78xx_resume()
        Note over NET: eth1 giữ nguyên ifindex, UP, IP, route, socket
        Note over NET: → Ping NW2 OK, không cần sửa app
    else Reset thất bại (LAN7800 lên chậm)
        CORE->>HUB: logical disconnect → quay lại đường hiện tại
        Note over NET: eth1 vẫn được tạo lại → cần phương án B
    end
```

Có thể kiểm chứng mà không cần build: `echo 1 > /sys/bus/usb/devices/4-1.4/power/persist` rồi vào và ra ErP. Kỳ vọng log có `usb 4-1.4: reset SuperSpeed …` thay vì `USB disconnect`.

---

## Sơ đồ 6: Phương án B (2-port LAN): phát hiện eth1 được tạo lại và cấu hình lại

```mermaid
sequenceDiagram
    autonumber
    participant K as Kernel (lan78xx / rtnetlink)
    participant MON as 2-port LAN monitor (mới)
    participant MIO as mio/NIC
    participant NET as eth1

    MON->>K: Đăng ký netlink RTNLGRP_LINK (hoặc uevent net)
    K-->>MON: RTM_DELLINK eth1 (USB disconnect)
    MON->>MIO: Đánh dấu NW2 không khả dụng (dừng service bind IP NW2)
    K-->>MON: RTM_NEWLINK eth1 (ifindex mới)
    MON->>MON: Kiểm tra /sys/class/net/eth1/device → 4-1.4:1.0
    MON->>MIO: Yêu cầu khởi tạo lại NW2 (giống lúc boot)
    MIO->>NET: ifup, IP/mask, default GW metric 1, route 192.168.42.0/23, 224.0.0.0/4
    MIO->>NET: VLAN / IPv6 (nếu có) · bind lại SMB/IPP/RAW…
    K-->>MON: Link is Up
    Note over MIO,NET: Ping NW2 OK. Xử lý được mọi đường disconnect/connect, không chỉ riêng ErP
```
