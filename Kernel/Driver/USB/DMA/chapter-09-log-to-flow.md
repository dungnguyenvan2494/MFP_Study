# Chương 9: Từ log lần ngược về luồng thực thi

> Khoá học USB DMA trên Linux Kernel. Baseline: **Linux 5.10.258**. Mọi sơ đồ trong chương được dựng từ đường đi **đã đối chiếu trong source**; chỗ nào là suy luận thì ghi rõ.

## 9.0 Phương pháp năm bước

```
 LOG ──► HÀM ──► SỰ KIỆN ──► TRẠNG THÁI PHẦN CỨNG ──► NGUYÊN NHÂN GỐC
  │        │         │                │                       │
  │        │         │                │                       └ cái gì đã làm trạng thái đó sai?
  │        │         │                └ thanh ghi / context / ring đang ở trạng thái nào?
  │        │         └ Event/ngắt/timeout/lệnh nào dẫn tới hàm đó?
  │        └ grep câu chữ → file:dòng → điều kiện if bao quanh
  └ đọc nguyên văn, lấy tham số (slot, ep_index, comp_code, địa chỉ, USBSTS)
```

| Bước | Câu hỏi | Công cụ |
|---|---|---|
| 1. Log | Câu chữ chính xác là gì? Có tham số gì? Log ngay trước/sau là gì? | `dmesg`, log console |
| 2. Hàm | Dòng nào in ra? Điều kiện `if` nào phải đúng? Hàm đó được gọi từ đâu? | `grep -rn` trên đúng cây kernel |
| 3. Sự kiện | Ngắt, Event, timer, lệnh hay lời gọi từ driver đã dẫn tới hàm? | đọc call chain, ftrace (nếu có) |
| 4. Phần cứng | USBSTS, trạng thái endpoint, dequeue/enqueue của ring, PORTSC | debugfs `…/usb/xhci/…`, giá trị in trong log |
| 5. Nguyên nhân | Ai/cái gì đã đưa phần cứng vào trạng thái đó? | so với luồng bình thường, tái hiện kèm thu thập thêm |

**Ba nguyên tắc:**
1. **Nơi in log thường không phải nơi gây lỗi.** B197 do FaxP in, nhưng lỗi nằm ở xHCI.
2. **Luôn tìm log bất thường *đầu tiên* theo thời gian**, không phải log ồn nhất.
3. **Đối chiếu với đúng phiên bản kernel đang chạy.** Câu chữ và điều kiện thay đổi giữa các phiên bản.

---

## 9.1 `HC died; cleaning up` (xHCI)

```
 Có ba đường dẫn tới xhci_hc_died (đã đối chiếu xhci-ring.c, xhci.c):

 (a) USBSTS đọc ra 0xFFFFFFFF trong hàm ngắt
     └ controller mất clock/nguồn/bị reset, bus trả toàn bit 1
 (b) Hủy Command Ring thất bại (CRR không về 0 trong 5 s) ─► "Abort failed to stop command ring: %d"
     └ trước đó: một lệnh không có Command Completion trong 5 s
 (c) Stop Endpoint không phản hồi trong 5 s ─► "xHCI host not responding to stop endpoint command." + "USBSTS:…"
     └ trước đó: driver hủy URB (timeout của lớp trên)
                   │
                   ▼
   xhci_hc_died: "xHCI host controller not responding, assume dead"
                   │ trả mọi URB trên mọi endpoint (kill endpoint URBs)
                   ▼
   USB core: "HC died; cleaning up" ─► root hub và mọi thiết bị bị gỡ
```

**Lần ngược:**

| Bước | Câu hỏi |
|---|---|
| Log ngay trước là (a), (b) hay (c)? | quyết định nhánh điều tra |
| Nếu (c): USBSTS in ra là gì? | `HCHalted` → controller đã tự dừng: tìm `Host System Error` trước đó. Không bit lỗi → Event/ngắt không tới |
| Nếu (b): lệnh nào đang chờ? | Address Device/Configure Endpoint lúc cắm thiết bị? Stop Endpoint lúc hủy? |
| Trước đó có suspend/resume không? | xem `root hub lost power or was reset` |
| Nguyên nhân gốc thường gặp | HSE do DMA sai địa chỉ; controller mất clock/nguồn; ngắt không tới CPU; erratum phần cứng |

---

## 9.2 `WARNING: Host System Error` (xHCI) / `fatal error` (EHCI)

```
 [SW] Ai đó ghi một DMA address vào cấu trúc mà controller sẽ đọc
      (TRB buffer ptr | Link TRB | TR Dequeue trong context | DCBAA | ERST | CRCR | ERDP)
          │
 [HW] controller DMA theo địa chỉ đó ──► interconnect/IOMMU từ chối
          │
 [HW] USBSTS.HSE = 1, controller tự Halt, phát ngắt
          │
 [SW] hàm ngắt: có EINT và HSE ──► "WARNING: Host System Error" ──► halt, thoát
          │   (EHCI: "fatal error" ──► usb_hc_died ngay)
          ▼
 [SW] không còn Event ──► URB treo ──► lớp trên timeout ──► Stop Endpoint ──► 9.1 (c)
```

**Lần ngược:**
1. Log ngay trước: resume (`root hub lost power`)? hủy URB? cắm/rút?
2. Trên SoC có SMMU: tìm `Unhandled context fault … iova=` hoặc `event … received` ngay trước. `iova` chính là địa chỉ hỏng.
3. Trên Quartz (không SMMU): không có địa chỉ trong log. Phải tái hiện kèm:
   - dump debugfs `reg-op`, `command-ring`, `event-ring`, `devices/NN/epXX/trbs` ngay sau sự cố;
   - ghi lại DMA address của các segment lúc cấp phát (trace `xhci_ring_alloc`, cần FTRACE).
4. Nguyên nhân hay gặp: dùng lại hoặc giải phóng ring/context/buffer khi controller vẫn còn dùng; bộ nhớ coherent bị ghi đè; mask sai.

---

## 9.3 `xHCI host not responding to stop endpoint command.`

```
 [SW] driver lớp trên hết kiên nhẫn ──► usb_kill_urb / usb_unlink_urb
 [SW] xHCI: đánh dấu TD hủy, gửi Stop Endpoint, hẹn giờ 5 s (XHCI_STOP_EP_CMD_TIMEOUT)
          │
          ├─ bình thường: Command Completion tới ──► gỡ TD ──► Set TR Dequeue ──► giveback -ENOENT
          │
          └─ 5 s không có Completion ──► "xHCI host not responding to stop endpoint command."
                                          "USBSTS: …"  ──► 9.1 (c)
```

**Đọc USBSTS trong log:**

| Bit | Nghĩa | Hướng điều tra |
|---|---|---|
| HCHalted | controller không chạy | có HSE trước đó? có ai xóa bit Run? |
| HSE | lỗi bộ nhớ | 9.2 |
| HCE | lỗi nội bộ controller | erratum, firmware của controller, reset |
| CNR | controller chưa sẵn sàng | đang resume/reset dở |
| Không bit lỗi | controller chạy nhưng không trả lời | Event Ring đầy? ngắt không tới (`/proc/interrupts`)? Command Ring kẹt? |

---

## 9.4 `ERROR Transfer event TRB DMA ptr not part of current TD …`

```
 [HW] controller hoàn tất một TRB tại DMA address X ──► ghi Transfer Event (X, comp_code, còn lại)
 [SW] hàm ngắt ──► xử lý Transfer Event ──► tìm ring của (slot, ep_index)
 [SW] td_list rỗng?  có ──► "WARN Event TRB … with no TDs queued?" (log khác)
                    không ──► TD đầu: X có nằm trong [dequeue … TRB cuối của TD]?
                        có ──► hoàn tất bình thường
                        không ──► các trường hợp được bỏ qua (STOPPED, isoc skip, spurious success, lỗi giữa TD)?
                                  không ──► "ERROR Transfer event TRB DMA ptr not part of current TD ep_index N comp_code C"
                                            ──► goto cleanup: Event BỊ BỎ
 [SW] TD không hoàn tất ──► URB không giveback ──► driver timeout ──► (hủy đúng: 9.3) / (không hủy: 9.5)
```

**Lần ngược (bám vào bốn khả năng ở Chương 5, mục 5.14):**

| Dữ liệu cần | Lấy ở đâu | Phân biệt được |
|---|---|---|
| Giá trị X | bật lại dump Event đang `#if 0` sau dòng log | X thuộc ring này hay không |
| Danh sách segment, enqueue/dequeue của ring | debugfs `devices/NN/epXX/{trbs,enqueue,dequeue,cycle}` | X có phải TD cũ không |
| Lịch sử hủy URB, Set TR Dequeue trên endpoint | trace `xhci_urb_dequeue`, `xhci_handle_cmd_set_deq` | phần mềm đi trước phần cứng |
| Có suspend/resume ngay trước không | `root hub lost power` | trạng thái controller sau resume |
| comp_code | trong log | `1` (SUCCESS) = controller tin là đã truyền xong |

---

## 9.5 `URB … submitted while active`

```
 [SW] driver submit URB lần 1 ──► … URB không bao giờ giveback (9.4, hoặc thiết bị NAK mãi)
 [SW] driver hết thời gian chờ ──► KHÔNG gọi usb_kill_urb ──► trả lỗi lên trên
 [SW] driver submit LẠI CHÍNH URB đó ──► USB core: use_count ≠ 0
      ──► WARN_ONCE "URB %pK submitted while active" + stack ──► -EBUSY
```

**Lần ngược:** stack trace cho biết driver. Câu hỏi tiếp theo không phải "vì sao submit lại" mà là **"vì sao lần trước không hoàn tất"**: tìm log xHCI ngay trước lần timeout đầu tiên. Ví dụ thật: chuỗi OP_BTS-71095 ở Chương 3, mục 3.7.

---

## 9.6 `swiotlb buffer is full …`

```
 [SW] submit URB ──► map ──► buffer vượt mask ──► SWIOTLB tìm ô liên tiếp ──► không đủ
      ──► "swiotlb buffer is full (sz: S bytes), total T (slots), used U (slots)"
      ──► DMA_MAPPING_ERROR ──► usb_submit_urb trả -EAGAIN
```

| Quan sát | Kết luận |
|---|---|
| `S` > 256 KiB | map quá lớn: driver phải chia nhỏ hoặc tôn trọng `dma_max_mapping_size` |
| `U` gần `T`, và tăng dần theo thời gian | **rò mapping**: tìm URB/mapping không được trả |
| `U` thấp nhưng vẫn lỗi | phân mảnh |
| Chỉ lúc tải đỉnh, `U` giảm lại sau đó | kho nhỏ so với tải: tăng `swiotlb=` hoặc giảm số thiết bị cần bounce |

---

## 9.7 `device descriptor read/64, error -71` → `unable to enumerate USB device`

```
 [HW] cắm thiết bị ──► Port Status Change Event ──► hub driver: reset cổng
 [SW] Enable Slot ──► Address Device (controller DMA đọc Input Context, gửi SET_ADDRESS)
 [SW] GET_DESCRIPTOR(64) qua EP0: TRB Setup (IDT) + Data IN (DMA ghi buffer) + Status
 [HW/BUS] thiết bị không trả lời đúng / CRC hỏng ──► completion code USB Transaction Error ──► -EPROTO (-71)
      ──► "device descriptor read/64, error -71" ──► thử lại, reset cổng
      ──► … hết số lần thử ──► "unable to enumerate USB device"
```

**Lần ngược:** đây hầu như luôn là **tầng vật lý hoặc thiết bị** (cáp, nguồn VBUS, thiết bị chậm sẵn sàng sau reset, nhiễu). DMA chỉ là người đưa tin. Kiểm tra: cổng/cáp khác, nguồn, hub trung gian, và thời gian thiết bị cần sau reset.

---

## 9.8 `Ring expansion failed`

```
 [SW] submit URB ──► cần N TRB ──► ring không đủ chỗ ──► thử chèn segment (coherent, GFP_ATOMIC)
      ──► cấp phát thất bại ──► "Ring expansion failed" ──► submit lỗi
 Vì sao ring đầy?  dequeue không tiến ──► TD không hoàn tất ──► 9.4 / thiết bị NAK mãi
                   hoặc submit quá nhiều URB cùng lúc
```

**Lần ngược:** debugfs `enqueue`/`dequeue` của endpoint. Dequeue đứng yên trong khi enqueue tăng → TD không được xử lý → quay lại 9.4. Cả hai cùng tiến → tải cao thật, hoặc atomic pool cạn.

---

## 9.9 `transfer buffer is on stack`

```
 [SW] driver: u8 buf[N] cục bộ ──► usb_fill_*_urb(buf) ──► usb_submit_urb
 [SW] USB core map: buffer nằm trên stack (vmalloc do CONFIG_VMAP_STACK)
      ──► WARN_ONCE "transfer buffer is on stack" + stack trace ──► -EAGAIN
      ──► các lần sau: -EAGAIN im lặng
```

**Lần ngược:** stack trace → hàm của driver → đổi sang `kmalloc`. Luôn kiểm tra lỗi trả về của `usb_submit_urb` và các hàm đồng bộ.

---

## 9.10 Ví dụ hoàn chỉnh: OP_BTS-71095 theo năm bước

| Bước | Kết quả |
|---|---|
| 1. Log | Log đầu tiên bất thường: `xhci-hcd 8d00d0000.usb3: ERROR Transfer event TRB DMA ptr not part of current TD ep_index 1 comp_code 1` (07:05:54.449). Không phải `B197` (07:07:37) |
| 2. Hàm | `xhci-ring.c:2788`, trong xử lý Transfer Event, nhánh không khớp TD và không thuộc trường hợp bỏ qua |
| 3. Sự kiện | Transfer Event SUCCESS cho EP1 OUT, ~1 s sau `faxAttach` đầu tiên kể từ khi thoát SLEEP1 |
| 4. Phần cứng | Controller tin đã truyền xong (SUCCESS). Phần mềm không nhận ra TRB đó. Lặp lại cả sau khi bo FAX tắt/bật nguồn và enumerate lại → trạng thái chung của controller, không phải của một ring cụ thể. SLEEP1 không đi qua resume của xHCI (không có `root hub lost power` lúc 07:05:51) |
| 5. Nguyên nhân | Chưa xác định chắc chắn. Đã loại trừ "Event cũ do cache" (Event Ring không cache, có `rmb`). Còn lại: trạng thái controller/hub sau SLEEP1, erratum. Hai lỗi phụ khiến không khôi phục được: BusReset sai cổng, `usbfax_write` không hủy URB |

---

## 9.11 Mẫu ghi chép khi phân tích một log

```
 Log nguyên văn      :
 Thời điểm / tick    :
 Log bất thường đầu  :
 File:dòng (kernel X):
 Điều kiện in        :
 Sự kiện dẫn tới     :
 Trạng thái phần cứng: USBSTS=… EP state=… deq/enq=… PORTSC=…
 Chuyện xảy ra sau   :
 Giả thuyết (xếp hạng):
 Dữ liệu còn thiếu   :
 Cách thu thập       :
```

## 9.12 Tóm tắt Chương 9

1. Năm bước: **Log → Hàm → Sự kiện → Trạng thái phần cứng → Nguyên nhân**.
2. Luôn bắt đầu từ **log bất thường đầu tiên**, không phải log ồn nhất hay mã lỗi cuối cùng.
3. `HC died; cleaning up` có ba đường đi tới. Log ngay trước nó cho biết đường nào.
4. `Host System Error` là lỗi địa chỉ DMA. Trên Quartz không có SMMU để chỉ ra địa chỉ, nên phải tái hiện kèm dump.
5. `submitted while active` và `Ring expansion failed` thường là **hậu quả**. Câu hỏi đúng là "vì sao lần trước không hoàn tất".
6. `-71`/`-EPROTO` khi enumerate gần như luôn là tầng vật lý.

## Câu hỏi kiểm tra

1. Cho chuỗi log: `WARN: TRB error for slot 3 ep 2 on endpoint` → 30 s sau `xHCI host not responding to stop endpoint command.` (USBSTS: HCHalted) → `assume dead`. Hãy dựng luồng và nêu giả thuyết.
2. Vì sao `xhci_hc_died` phải trả mọi URB về trước khi USB core in `HC died; cleaning up`? Điều gì xảy ra với buffer DMA của các URB đó?
3. Bạn chỉ có log `swiotlb buffer is full (sz: 4096 bytes), total 32768 (slots), used 32760 (slots)`. Bước tiếp theo là gì?
4. Áp năm bước cho log `device not accepting address 7, error -62`. Phần nào là DMA, phần nào không?
5. Vì sao trong OP_BTS-71095, `B197` là log *cuối cùng* của chuỗi chứ không phải nguyên nhân?

---

**Chương tiếp theo:** Chương 10 trình bày hơn 10 tình huống debug thực tế: triệu chứng, log mẫu, luồng thực thi, nguyên nhân, cách điều tra, kết quả kỳ vọng và các bước debug.
