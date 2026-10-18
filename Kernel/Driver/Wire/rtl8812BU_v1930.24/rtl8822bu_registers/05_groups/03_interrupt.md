# 05.03 — Interrupt: mask, status, acknowledge (với RTL8822BU qua USB)

> Kết luận trước: **trong build này host không nhận ngắt phần cứng từ chip**. Các thanh ghi IMR/ISR của host (HIMR/HISR…) có định nghĩa nhưng không được dùng trên đường sống. Phần này trình bày bằng chứng và những gì thay thế cơ chế ngắt.

## 1. Bằng chứng

| Điểm kiểm tra | Kết quả | Nguồn |
|---|---|---|
| Interrupt-IN endpoint | `/* #define CONFIG_USB_INTERRUPT_IN_PIPE  1 */` — bị comment → `pops->_read_interrupt` không được gán; nhánh trong `usb_set_intf_ops` và `rtl8822bu_set_intf_ops` inactive | `include/autoconf.h:128`, `hal/rtl8822b/usb/rtl8822bu_io.c:88-90` [SOURCE + mask] |
| Hàm xử lý ngắt USB | `rtl8822bu_interrupt_handler` nằm trong `#ifdef CONFIG_SUPPORT_USB_INT` (không bật) | `hal/rtl8822b/usb/rtl8822bu_ops.c:66-70` |
| Ghi mask ngắt host | `rtl8822bu_update_interrupt_mask()` ghi `REG_HIMR0_8822B` (0xB0) / `REG_HIMR1_8822B` (0xB8) từ `pHalData->IntrMask[]` — **không có nơi gọi** trong build | `hal/rtl8822b/usb/rtl8822bu_halinit.c:369-392`; `work/callgraph.json` [TOOL] |
| HISR/HSISR/INT_MIG | chỉ định nghĩa (St D/Z) | `04a_catalog_mac.md` |

Danh mục: 24 thanh ghi có tên chứa `IMR`/`ISR` ở trạng thái D; `REG_HSIMR` (0x58), `REG_GPIO_INTM` (0x48, chỉ dùng trong pwr-seq như cấu hình GPIO), `REG_INT_MIG` (0x304) ở Z; `REG_HIMR0/1` ở X [TOOL].

## 2. Cái gì thay thế ngắt?

1. **C2H qua gói RX**: firmware báo sự kiện (TX report, CCX, scan done, IQK done…) bằng gói đặc biệt trong luồng bulk-in. `rtl8822bu_recv.c:178-188` phân loại `pattrib->pkt_rpt_type`; `C2H_PACKET` được chuyển cho `rtw_halmac_c2h_handle()` (`hal/hal_halmac.c:4167`) mà không cấp recvframe [SOURCE]. Loại gói được quyết định bởi field trong **RX descriptor** (không phải thanh ghi — xem 05.05).
2. **Polling theo timer/workqueue**: watchdog của driver (`rtw_dynamic_check_timer_handler` → `rtl8822b_phy_haldm_watchdog` …) định kỳ đọc thanh ghi trạng thái (false alarm counters, RSSI…) — xem 05.13.
3. **Mailbox poll**: H2C dùng poll `REG_HMETFR` (05.08 §3).

## 3. Thanh ghi "interrupt" duy nhất được đọc trên đường sống

`REG_FE1IMR_8822B` (0x120) trong `rtw_hal_check_wow_ctrl()` (`hal/hal_com.c:5152-5186`), nhánh `IS_HARDWARE_TYPE_JAGUAR2 || JAGUAR3` (đúng với 8822B) và `chk_type == 0` (nhánh `chk_type != 0` thay vào đó poll `REG_WOWLAN_WAKE_REASON` 0x1C7 về 0, tối đa 24 × 20 ms):

```c
/* Wait FW to cleare 0x120 bit16, 0x284 bit18 to 0 */   ← comment gốc, hal_com.c:5169
fe1_imr   = rtw_read32(adapter, REG_FE1IMR);      /* RxDone IMR for 3081 */
rxpkt_num = rtw_read32(adapter, REG_RXPKT_NUM);   /* Release RXDMA */
while (((fe1_imr & BIT_FS_RXDONE_INT_EN) || (rxpkt_num & BIT_RW_RELEASE_EN)) && trycnt > 1) {
	rtw_msleep_os(20);
	fe1_imr   = rtw_read32(adapter, REG_FE1IMR);
	rxpkt_num = rtw_read32(adapter, REG_RXPKT_NUM);
	…
}
res = ((fe1_imr & BIT_FS_RXDONE_INT_EN) || (rxpkt_num & BIT_RW_RELEASE_EN)) ? FALSE : TRUE;
```

Ở đây IMR không dùng để nhận ngắt mà để **kiểm tra firmware đã tắt "RX done interrupt" của CPU 3081 (CPU WLAN)** khi rời WoWLAN — tức là thanh ghi mask ngắt *của firmware*, host chỉ đọc trạng thái [SOURCE cho comment "RxDone IMR for 3081"; diễn giải = INFERENCE]. Chi tiết luồng WoWLAN ở 05.10.

## 4. Hệ quả cho debug

- Không có "interrupt storm" hay ISR latency để soi; độ trễ sự kiện = độ trễ bulk-in + RX aggregation timeout (05.09 §5).
- Nếu C2H không đến (ví dụ IQK chờ mãi), kiểm tra RX path (URB bulk-in có đang được submit) chứ không phải HIMR.
