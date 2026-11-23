# Chương 4: Resume ở mức phần cứng

> Khoá học USB Resume trên Linux Kernel. Thông số tín hiệu theo đặc tả USB 2.0 (chương 7, 11) và USB 3.x (chương 6, 7 về LTSSM/LFPS); hằng số kernel đối chiếu **Linux 5.10.258**. Thời lượng LFPS của USB 3.x thay đổi theo phiên bản đặc tả, nên chương này chỉ nêu các mốc được kernel dùng tới. Với các con số khác, hãy tra đặc tả bạn đang làm việc.

## 4.0 Ba thế giới khác nhau, đừng lẫn

| Thế giới | Ví dụ | Đo/thấy bằng |
|---|---|---|
| **Tín hiệu USB** (điện) | dây D+/D− ở trạng thái K trong 20 ms; LFPS | máy hiện sóng, protocol analyzer |
| **Sự kiện giao thức USB** | "cổng hết suspend", "link chuyển U3 → U0", Function Wake Notification | bit trong PORTSC, Event của xHCI |
| **Lời gọi hàm Linux** | `usb_port_resume()`, `xhci_set_link_state()` | log, ftrace |

Một lời gọi hàm có thể khiến phần cứng phát tín hiệu trong **hàng chục ms**, trong khi CPU chỉ chạy vài µs rồi **ngủ chờ** (`msleep(USB_RESUME_TIMEOUT)`).

---

## 4.1 USB 2.0: trạng thái đường dây

USB 2.0 dùng **một cặp dây vi sai** D+ và D−. Thông tin nằm ở **hiệu điện thế** giữa hai dây.

| Trạng thái | Tiếng Việt | Full-Speed (FS) | Low-Speed (LS) | Dùng để |
|---|---|---|---|---|
| **J** | trạng thái J | D+ cao, D− thấp | D− cao, D+ thấp | trạng thái **rảnh** (idle) |
| **K** | trạng thái K | ngược với J | ngược với J | **Resume**, bắt đầu gói (SOP) |
| **SE0** | Single-Ended Zero | cả hai thấp | cả hai thấp | kết thúc gói (EOP), **Reset**, ngắt kết nối |
| SE1 | cả hai cao | không hợp lệ | | |

```
         rảnh (J)           Resume (K)                    EOP               rảnh (J)
 D+  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\_____________________________/‾‾‾‾\___/‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾   (Full-Speed)
 D−  ________________/‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾\____\___/________________
                      ◄──────── ≥ 20 ms ─────────►  SE0 rồi J
```

**Vì sao cần cả J và K?** Thiết bị biết mình là FS hay LS nhờ điện trở kéo lên (pull-up) trên D+ hay D−. Trạng thái rảnh vì thế luôn là "phía có pull-up cao", tức là J. K là trạng thái **"lệch khỏi rảnh"** mà không phải SE0, rất thích hợp để báo "dậy đi" mà không bị nhầm với Reset (SE0).

**Thiết bị High-Speed (HS)** truyền dữ liệu bằng mức điện áp khác (vi sai nhỏ). Khi vào Suspend, thiết bị HS **chuyển về chế độ tín hiệu FS** (gắn lại pull-up D+), nên bus ngủ ở trạng thái J kiểu FS. Resume dùng trạng thái K kiểu FS. Khi Resume kết thúc, thiết bị **tự trở lại HS**.

---

## 4.2 USB 2.0: vào Suspend

```
 Host/hub ngừng gửi mọi gói (kể cả SOF mỗi 1 ms FS / 125 µs HS, và keep-alive của LS)
      │ bus đứng yên ở J
      ▼ ≥ 3 ms
 Thiết bị: vào Suspend (dòng tiêu thụ ≤ 2.5 mA trung bình)
 (HS: trước đó tách mạch HS, gắn pull-up FS)
```

Không có "gói Suspend". Suspend được báo bằng **sự im lặng**.

---

## 4.3 USB 2.0: Resume do host khởi động

```
 Host (hub/root hub port)                      Thiết bị đang Suspend
   │ ClearPortFeature(SUSPEND) / ghi PORTSC
   │──── K ─────────────────────────────────►  phát hiện K, bắt đầu dậy
   │     giữ K ≥ 20 ms (TDRSMDN)
   │     [Linux giữ 40 ms: USB_RESUME_TIMEOUT]
   │──── kết thúc: SE0 ngắn (EOP kiểu LS) ──►  (HS: thiết bị chuyển lại HS)
   │──── J / gửi SOF ngay ─────────────────►  bus hoạt động lại
   │     chờ phục hồi 10 ms (TRSMRCY)
   │──── lệnh đầu tiên (GET_STATUS…) ───────►  phải trả lời được
```

| Mốc | Đặc tả | Linux 5.10.258 |
|---|---|---|
| Thời lượng Resume xuôi (TDRSMDN) | ≥ 20 ms | `USB_RESUME_TIMEOUT = 40` ms (`include/linux/usb.h:311`) |
| Phục hồi (TRSMRCY) | 10 ms | `msleep(10)` trong `usb_port_resume` và `hub_handle_remote_wakeup` |
| EHCI giữ Resume trong lúc resume cổng | | `reset_done = jiffies + 20 ms` trong `ehci-hub.c` |

---

## 4.4 USB 2.0: Resume do thiết bị khởi động (Remote Wakeup)

```
 Thiết bị                                      Hub/root hub port                  Host phần mềm
   │ (đã Suspend ≥ 5 ms, được phép remote wakeup)
   │ có sự kiện (nhấn phím)
   │──── K trong 1–15 ms (TDRSMUP) ─────────►  phát hiện K trên cổng đang suspend
   │                                           TỰ phát lại K xuống cổng (và lên trên
   │                                           nếu hub này cũng đang suspend)
   │                                           giữ K ≥ 20 ms (TDRSMDN)
   │                                           báo "cổng đổi trạng thái"  ─────► ngắt / Event
   │                                                                              hẹn giờ 40 ms
   │◄─── kết thúc Resume, bus hoạt động ──────  (root hub: phần mềm ra lệnh kết thúc)
   │                                           bit C_SUSPEND = 1  ──────────────► hub driver thấy
   │                                                                              remote wakeup
   │◄─── chờ 10 ms, rồi giao tiếp lại ──────────────────────────────────────────── 
```

**Hướng tín hiệu:**
- Thiết bị phát K **ngắn** (1–15 ms) chỉ để "gõ cửa".
- **Phía upstream** (hub/host) **tiếp quản** và giữ K đủ lâu (≥ 20 ms) để mọi thiết bị trên đoạn bus đó đều dậy.
- Chính phía upstream kết thúc Resume.

**Điều kiện để thiết bị được phép phát Remote Wakeup:**
1. Thiết bị khai báo khả năng trong descriptor.
2. Host đã bật bằng `SET_FEATURE(DEVICE_REMOTE_WAKEUP)` trước khi suspend.
3. Thiết bị đã ở Suspend tối thiểu 5 ms.

---

## 4.5 USB 2.0 LPM (L1): giấc ngủ ngắn

USB 2.0 có thêm **Link Power Management** với trạng thái **L1**: ngủ nhẹ, vào và ra trong khoảng µs. Host gửi gói LPM để vào L1, và resume nhanh hơn nhiều so với Suspend (L2).

| Trạng thái | Nghĩa |
|---|---|
| L0 | hoạt động |
| L1 | ngủ nhẹ (LPM) |
| L2 | **Suspend** (chương này) |
| L3 | tắt nguồn |

Kernel có `USB_PORT_STAT_L1` và sysfs `power/usb2_hardware_lpm`. L1 khác Suspend: không đi qua luồng `usb_port_suspend`/`usb_port_resume` của USB core.

---

## 4.6 USB 3.x: trạng thái link thay vì trạng thái dây

USB 3.x (SuperSpeed) không dùng J/K. Hai đầu duy trì một **liên kết (link)** được quản lý bởi máy trạng thái **LTSSM** (Link Training and Status State Machine).

### Các trạng thái công suất của link

| Trạng thái | Tên | Ý nghĩa | Ai đưa vào | Thoát mất |
|---|---|---|---|---|
| **U0** | Active | truyền dữ liệu | | |
| **U1** | Standby, thoát nhanh | ngủ ngắn khi rảnh | **tự động** bởi link (LPM theo timeout) | µs |
| **U2** | Standby, thoát chậm hơn | tiết kiệm hơn U1 | tự động, hoặc từ U1 sau timeout | µs đến ms |
| **U3** | **Suspend** | tương đương Suspend của USB 2.0 | **phần mềm host** ra lệnh | tới ms |

**U1/U2 không phải "Suspend" theo nghĩa của Linux.** Chúng là tiết kiệm điện ở mức link, phần cứng tự làm. Linux chỉ bật/tắt khả năng (LPM, LTM). U3 mới là Suspend.

### LFPS (Low Frequency Periodic Signaling)

**LFPS** (tín hiệu tuần hoàn tần số thấp) là các **chuỗi xung tần số thấp** mà hai đầu dùng khi bộ thu tốc độ cao **đang tắt**. Nhờ vậy chúng vẫn "gọi" được nhau.

| LFPS dùng cho | Nghĩa |
|---|---|
| Thoát U1/U2/U3 | "dậy đi", và đầu kia trả lời bằng LFPS (bắt tay) |
| Polling | bắt đầu huấn luyện link |
| **Warm Reset** | reset mạnh: LFPS kéo dài, dùng cả khi link hỏng |
| Ping (trong U1) | giữ đồng bộ |

### LTSSM rút gọn

```
              cắm vào
 SS.Disabled ◄──────── Rx.Detect ──phát hiện đầu kia──► Polling ──huấn luyện xong──► U0 ◄───────┐
                             ▲                              │ lỗi lặp lại                  │       │
                             │                              ▼                              │       │
                             │                         Compliance                          │       │
       Warm Reset ◄──────────┼── SS.Inactive ◄── lỗi không phục hồi được ──────────────┐     │       │
                             │                                                    │     │       │
                             │                    U1 ◄──┐  U2 ◄──┐  U3 ◄── host ra │     │       │
                             │                     │    │   │    │   │   lệnh      │     │       │
                             │                     └────┴───┴────┴───┴──► Recovery ┴─────┘       │
                             │                                          (huấn luyện lại ngắn)    │
                             │                                              │ thành công ────────┘
                             └──────────── Hot Reset (qua Recovery, lệnh reset trong TS1/TS2) ───
```

| Trạng thái LTSSM | Ý nghĩa với Resume |
|---|---|
| U3 | đang Suspend |
| **Recovery** | **mọi lần thoát U1/U2/U3 đều đi qua đây**: huấn luyện lại ngắn trước khi về U0 |
| U0 | hoạt động |
| SS.Inactive | link hỏng, cần **Warm Reset** |
| Rx.Detect, Polling | đang phát hiện và huấn luyện từ đầu (sau cắm hoặc reset) |
| Hot Reset | reset thiết bị nhưng link vẫn hoạt động |
| Compliance, Loopback | chế độ kiểm thử; kẹt ở Compliance là lỗi |

---

## 4.7 USB 3.x: Resume do host khởi động (U3 → U0)

```
 Host (root hub port)                                    Thiết bị (link U3)
   │ SetPortFeature(LINK_STATE = U0)
   │ (xHCI: ghi PLS = U0 + LWS)
   │──── LFPS thoát U3 ──────────────────────────────►  bộ thu bật lại
   │◄─── LFPS trả lời (bắt tay) ──────────────────────
   │════ Recovery: TS1/TS2 huấn luyện lại ═══════════
   │──── U0 ──────────────────────────────────────────►
   │ PORTSC: PLS = U0, PLC (link state change) = 1  ──► Port Status Change Event ──► Linux
```

Linux (5.10.258, `xhci-hub.c`): sau khi ghi U0, nếu trước đó là U3/Resume/Recovery thì **chờ tối đa 500 ms** cho sự kiện "đã về U0". Không thấy thì log debug `missing U0 port change event for port %d-%d`.

## 4.8 USB 3.x: Resume do thiết bị khởi động (Remote Wake)

Có **hai lớp** mà người mới hay nhầm:

| Lớp | Cơ chế | Linux xử lý |
|---|---|---|
| **Link** | thiết bị gửi LFPS để kéo link U3 → U0. Cổng host thấy `PLS = Resume` kèm PLC | `handle_port_status`: `remote wake SS port %d` (debug), ghi U0, chờ PLC với U0 → `resume SS port %d finished` (debug) → `usb_wakeup_notification` |
| **Function** | sau khi link về U0, thiết bị gửi **Function Wake Device Notification** để báo *interface nào* muốn dậy | xHCI tạo **Device Notification Event** → `usb_wakeup_notification` (`xhci-ring.c:1749`) |

Nếu host không phản hồi LFPS đánh thức, thiết bị USB 3 **gửi lại sau khoảng 100 ms**. Đây là lý do `xhci_resume` chờ thêm **120 ms** khi chưa thấy sự kiện cổng (comment trong `xhci.c`).

## 4.9 Warm Reset, Hot Reset và Resume

| | Resume (U3 → U0) | Hot Reset | Warm Reset |
|---|---|---|---|
| Tín hiệu | LFPS thoát U3 + Recovery | lệnh reset trong chuỗi huấn luyện (TS1/TS2) | **LFPS dài** |
| Cần link còn hoạt động? | có (đang U3) | có | **không**, dùng được khi link hỏng (SS.Inactive) |
| Thiết bị giữ địa chỉ/cấu hình? | **có** | không | không |
| Linux dùng khi | resume bình thường | reset thiết bị (`usb_reset_device`) | link kẹt SS.Inactive/Compliance, reset sau lỗi nặng |

## 4.10 So sánh USB 2.0 Resume và USB 3.x Recovery

| | USB 2.0 | USB 3.x |
|---|---|---|
| Vào Suspend | im lặng ≥ 3 ms | lệnh U3 rõ ràng |
| Báo dậy | trạng thái **K** trên D+/D− | **LFPS** |
| Thời lượng | K ≥ 20 ms (Linux 40 ms) | bắt tay LFPS + Recovery, thường ngắn hơn |
| Ai kết thúc | phía upstream (hub/host) | hai đầu cùng huấn luyện xong |
| Sau Resume | chờ 10 ms (TRSMRCY) | link đã U0 |
| Remote wakeup | một lớp (K) | hai lớp (link LFPS + Function Wake) |
| Ngủ nhẹ | L1 (LPM) | U1, U2 (tự động) |
| Lỗi đặc trưng | cổng không thoát Resume, thiết bị không dậy kịp | kẹt U3, kẹt Recovery, rơi về SS.Inactive → cần Warm Reset |

## 4.11 Tóm tắt Chương 4

1. USB 2.0: **J** = rảnh, **K** = Resume, **SE0** = EOP/Reset. Suspend được báo bằng **im lặng ≥ 3 ms**.
2. Resume do host: K ≥ 20 ms (Linux 40 ms), rồi phục hồi 10 ms. Remote wakeup: thiết bị phát K 1–15 ms, **upstream tiếp quản** và giữ ≥ 20 ms.
3. HS khi ngủ dùng tín hiệu FS, dậy thì tự về HS.
4. USB 3.x: **U0/U1/U2/U3**. Chỉ **U3** là Suspend. Mọi lần thoát đều qua **Recovery**. Báo dậy bằng **LFPS**.
5. Remote wake USB 3.x có **hai lớp**: link (LFPS) và function (Device Notification).
6. Warm Reset (LFPS dài) cứu được link hỏng. Hot Reset và Resume thì cần link còn sống.

## Câu hỏi kiểm tra

1. Vì sao Resume dùng trạng thái K mà không dùng SE0?
2. Trong remote wakeup USB 2.0, ai giữ tín hiệu K lâu hơn: thiết bị hay hub? Vì sao phải như vậy?
3. Một thiết bị High-Speed khi ngủ thì bus ở trạng thái gì? Khi dậy nó trở về HS lúc nào?
4. U1 và U3 khác nhau thế nào về "ai quyết định" và "Linux có biết không"?
5. Vì sao mọi lần thoát U1/U2/U3 đều phải qua Recovery?
6. Khi nào cần Warm Reset thay vì Hot Reset?
7. Vì sao `xhci_resume` chờ thêm 120 ms nếu chưa thấy sự kiện cổng nào?
8. Phân biệt "tín hiệu Resume", "bit C_SUSPEND" và "hàm `usb_port_resume`" bằng một câu cho mỗi cái.

---

**Chương tiếp theo:** Chương 5 đi vào kiến trúc phần mềm: USB core, hub driver, HCD, PM core, Runtime PM, PHY framework, và các cấu trúc dữ liệu chính tham gia resume.
