# 05.04 — DMA, packet buffer (TX/RX FIFO), hàng đợi TX/RX (MAC 0x0100–0x02FF)

> Nhãn như 05.01. Số liệu cấp phát FIFO được tính bằng `tools/txff_sim.py` — **mô phỏng lại đúng phép toán trong source**, không phải giá trị đo trên board.

## 1. Bức tranh: từ bulk-out đến không khí

```
host (bulk-out EP n)
   │  frame + TX descriptor 48 B  (descriptor = dữ liệu, không phải thanh ghi — 05.05)
   ▼
HCI TX DMA ──(QSEL trong descriptor)──► hàng đợi phần mềm VO/VI/BE/BK/MG/HI
   │                                     │  REG_TXDMA_PQ_MAP (0x10C): mỗi hàng → 1 trong 4 "page queue"
   ▼                                     ▼
TX FIFO 256 KB = 2048 page × 128 B   HQ | NQ | LQ | EXQ | PUB (dùng chung)  ← REG_FIFOPAGE_INFO_1..5
   │                                  ... | vùng reserved (driver, H2C, FW buf) ← REG_FIFOPAGE_CTRL_2 / BCNQ_BDNY
   ▼
MAC TX (scheduler, protocol, EDCA) ──► BB/RF
```

## 2. Thanh ghi

| Địa chỉ | Tên | St | Field có bằng chứng | Ghi bởi |
|---|---|:-:|---|---|
| `0x0100` | `REG_CR_8822B` (alias `rPMAC_Reset` dùng như BB trong MP) | A | B0: `HCI_TXDMA_EN[0]`, `HCI_RXDMA_EN[1]`, `TXDMA_EN[2]`, `RXDMA_EN[3]`, `PROTOCOL_EN[4]`, `SCHEDULE_EN[5]`, `MACTXEN[6]`, `MACRXEN[7]`; B1: `ENSWBCN[8]`, `MAC_SEC_EN`; B2: `NETYPE0/1` (= `MSR`); B3: transfer mode | `init_trx_cfg_8822b`, `download_firmware_88xx`, `dl_rsvd_page_88xx`, `cfg_net_type_88xx`, `rtl8822b_init_misc`, `config_security_88xx`, `priority_queue_cfg_8822b` |
| `0x010C–0x010D` | `REG_TXDMA_PQ_MAP_8822B` | A | B0: `RXDMA_ARBBW_EN[0]` (đặt cho USB), `RXSHFT_EN[1]`, `RXDMA_AGG_EN[2]`; `VOQ_MAP[5:4]`, `VIQ_MAP[7:6]`, `BEQ_MAP[9:8]`, `BKQ_MAP[11:10]`, `MGQ_MAP[13:12]`, `HIQ_MAP[15:14]` | `txdma_queue_mapping_8822b`, `init_trx_cfg_8822b`, `rx_shift_88xx`, `cfg_usb_rx_agg_88xx`, `download_firmware_88xx` |
| `0x0115` | B1 của `REG_TRXFF_BNDY_8822B` | A | `RXFFOVFL_RSV_V2[11:8] = 0xF` — comment "For rxdesc len = 0 issue" | `cfg_drv_info_8822b` (`halmac_cfg_wmac_8822b.c:121-125`) |
| `0x011C` | `REG_RXFF_BNDY_8822B` | A | ranh giới RX FIFO | `priority_queue_cfg_8822b` |
| `0x0140/0x0144/0x0148` | `REG_PKTBUF_DBG_CTRL/DATA_L/DATA_H_8822B` | A/G | cửa sổ đọc packet buffer (debug) | `read_buf_88xx`, `dump_TX_FIFO`, `phydm_la_access_tx_pkt_buf` |
| `0x0204–0x0207` | `REG_FIFOPAGE_CTRL_2_8822B` | A | `BCN_HEAD_V1[11:0]`, `BCN_VALID_V1[15]`; `+2` = `BCN_HEAD_1` (port 1) | `priority_queue_cfg_8822b`, `dl_rsvd_page_88xx`, `hw_var_set/get_bcn_valid` |
| `0x0208–0x020B` | `REG_AUTO_LLT_V1_8822B` | A | `AUTO_INIT_LLT_V1[0]` (W1, HW tự xoá), `BLK_DESC_NUM[7:4]`; B3 = BLK_DESC_NUM | `priority_queue_cfg_8822b` |
| `0x020C–0x020D` | `REG_TXDMA_OFFSET_CHK_8822B` | A | `DROP_DATA_EN[9]` (đặt **2 lần**: `W16_SET(BIT_DROP_DATA_EN)` và `W8_SET(+1, BIT1)`), `SDIO_TXDESC_CHKSUM_EN[13]` (tên SDIO nhưng dùng chung), `RESTORE_H2C_ADDRESS[15]` | `init_usb_cfg_88xx`, `priority_queue_cfg_8822b`, `init_h2c_8822b`, `tx_desc_chksum_88xx` |
| `0x0210` | `REG_TXDMA_STATUS_8822B` | A | khác 0 = lỗi TX DMA; `PAGE_OVF[2]` | `xmit_status_check` (→ silent reset), `dlfw_end_flow_88xx`, `sreset_get_wifi_status` |
| `0x022C` | `REG_RQPN_CTRL_2_8822B` | A | `BIT31` = nạp số page | `priority_queue_cfg_8822b`, `download_firmware_88xx` |
| `0x0230–0x0240` | `REG_FIFOPAGE_INFO_1..5_8822B` | A | số page HQ, LQ, NQ, EXQ, PUB (16-bit ghi; 32-bit đọc kèm số page còn trống) | `priority_queue_cfg_8822b`, `dump_mac_txfifo`, `rtw_halmac_txfifo_wait_empty` |
| `0x0244/0x0248/0x024C/0x0254` | `REG_H2C_HEAD/TAIL/READ_ADDR/INFO_8822B` | A | ring H2C-packet trong TX FIFO (byte address, 18 bit) | `init_h2c_8822b` |
| `0x0280–0x0283` | `REG_RXDMA_AGG_PG_TH_8822B` | A | xem 05.09 §5 | `cfg_usb_rx_agg_88xx`, `rtw_set_usb_agg_by_mode_normal` |
| `0x0284–0x0287` | `REG_RXPKT_NUM_8822B` | A | `RXDMA_IDLE[17]` (RO), `RW_RELEASE_EN[18]` (dừng RX DMA) | `rtw_hal_pause_rx_dma`, `rtw_hal_release_rx_dma`, `rtw_hal_force_enable_rxdma`, `rtl8822b_sethwreg`, `rtw_hal_check_wow_ctrl` |
| `0x0288` | `REG_RXDMA_STATUS_8822B` | A | khác 0 = lỗi RX DMA | `linked_status_check` (→ silent reset) |
| `0x0290` | `REG_RXDMA_MODE_8822B` | A | xem 05.09 §2 | `init_usb_cfg_88xx` |
| `0x029C`, `0x02A0` | `REG_FWFF_CTRL/PKT_INFO_8822B` | A | con trỏ FIFO firmware (FWFF) | `init_trx_cfg_8822b`, `fwff_is_empty_88xx` |
| `0x0422` | B2 của `REG_FWHW_TXQ_CTRL` | A | `EN_WR_FREE_TAIL[20]`, `EN_BCNQ_DL[22]` | `priority_queue_cfg_8822b`, `dl_rsvd_page_88xx` |
| `0x0424`, `0x0456` | `REG_BCNQ_BDNY_V1`, `REG_BCNQ1_BDNY_V1` | A | ranh giới beacon queue port 0/1 | `priority_queue_cfg_8822b` |

## 3. `init_trx_cfg_8822b()` — bật đường dữ liệu (`halmac_init_8822b.c:459-516`)

| # | Thao tác | Giải thích |
|---:|---|---|
| 1 | `txdma_queue_mapping_8822b(mode)` | §4 |
| 2 | nếu `REG_WMAC_FWPKT_CR & BIT_FWEN`: tạm xoá, chờ FWFF rỗng (`REG_FWFF_CTRL == REG_FWFF_PKT_INFO`, ≤5000×50 µs) | tránh mất gói FW đang trong FIFO |
| 3 | `W8(REG_CR, 0)` | **tắt toàn bộ** 8 bit DMA/MAC |
| 4 | `W16(REG_FWFF_CTRL, R16(REG_FWFF_PKT_INFO))` | đồng bộ con trỏ FWFF |
| 5 | `W8(REG_CR, MAC_TRX_ENABLE = 0xFF)` | bật lại cả 8: HCI TX/RX DMA, TX/RX DMA, protocol, scheduler, MAC TX/RX |
| 6 | `W32(REG_H2CQ_CSR, BIT31)` | |
| 7 | `priority_queue_cfg_8822b(mode)` | §5 |
| 8 | `init_h2c_8822b()` | §6 |
| 9 | USB: `W8_SET(REG_TXDMA_PQ_MAP, BIT0)` | `RXDMA_ARBBW_EN[0]` (tên header; ý nghĩa "arbiter bandwidth" = [INFERENCE]) |

Chế độ `mode`: `_choose_trx_mode()` (`hal/hal_halmac.c:2901-2922`) trả `HALMAC_TRX_MODE_WMM` nếu `registrypriv.wifi_spec` (module param `rtw_wifi_spec`), ngược lại `HALMAC_TRX_MODE_NORMAL`.

## 4. Ánh xạ hàng đợi (`txdma_queue_mapping_8822b`, `:519-561`)

Bảng chọn theo **số bulk-out endpoint** của thiết bị (`adapter->bulkout_num`, lấy từ USB descriptor) [SOURCE `:533-542`]. Giá trị `HALMAC_DMA_MAPPING_{EXTRA,LOW,NORMAL,HIGH} = 0,1,2,3` (`halmac_type.h:633-636`); field 2 bit ở vị trí VO 5:4, VI 7:6, BE 9:8, BK 11:10, MG 13:12, HI 15:14 (`halmac_bit2.h:18981-19119`).

| bulk-out | mode | VO | VI | BE | BK | MG | HI | `REG_TXDMA_PQ_MAP` (W16) [TOOL] |
|---|---|---|---|---|---|---|---|---|
| 2 | NORMAL | NQ | NQ | NQ | HQ | HQ | HQ | `0xFEA0` |
| 3 | NORMAL | NQ | NQ | LQ | LQ | HQ | HQ | `0xF5A0` |
| 4 | NORMAL | NQ | NQ | LQ | LQ | EXQ | HQ | `0xC5A0` |
| 3 | WMM | HQ | NQ | LQ | NQ | HQ | HQ | `0xF9B0` |

(tính bằng `py -I tools/txff_sim.py <bulkout> 16 <mode>`; ghi W16 **xoá luôn bit 3:0** — chúng được đặt lại sau ở bước 9 và bởi `cfg_usb_rx_agg_88xx`/`rx_shift_88xx`.) Số bulk-out thật của board: [KIỂM TRA TRÊN BOARD] (`lsusb -v`, hoặc log `RtNumOutPipes`).

## 5. Phân vùng TX FIFO (`priority_queue_cfg_8822b` + `set_trx_fifo_info_8822b`, `:563-748`)

**Mô phỏng** với 3 bulk-out, NORMAL, driver xin 16 page reserved (`py -I tools/txff_sim.py 3 16 NORMAL`):

```
tx_fifo_pg_num = 262144 >> 7 = 2048 page (128 B/page)
rsvd_pg_num    = drv 16 + h2c_extra 24 + h2c_static 8 + h2cq 8 + cpu 0 + fw_txbuf 4 + csi 0 = 60
rsvd_boundary  = acq_pg_num = 2048 - 60 = 1988 (0x7C4)

page 2048 ┬─────────────────────────
          │ fw_txbuf     2044..2047 (4)
          │ h2cq         2036..2043 (8)   ← ring H2C packet (§6)
          │ h2c_sta_info 2028..2035 (8)
          │ h2c_info     2004..2027 (24)
          │ drv          1988..2003 (16)  ← beacon, probe-rsp, null, WoW… (reserved page)
 0x7C4 ───┼───────────────────────── rsvd_boundary = BCN_HEAD = BCNQ_BDNY
          │ PUB 1795 | LQ 64 | NQ 64 | HQ 64 | gap 1     (pubq = 1988-64-64-64-0-1)
page 0    ┴─────────────────────────
```

| Ghi | Giá trị (mô phỏng) | Dòng |
|---|---|---|
| `REG_FIFOPAGE_INFO_1` (HQ) | 64 | `:603` |
| `REG_FIFOPAGE_INFO_2` (LQ) | 64 | `:604` |
| `REG_FIFOPAGE_INFO_3` (NQ) | 64 | `:605` |
| `REG_FIFOPAGE_INFO_4` (EXQ) | 0 | `:606` |
| `REG_FIFOPAGE_INFO_5` (PUB) | 1795 (`0x703`) | `:607` |
| `REG_RQPN_CTRL_2 \|= BIT31` | | `:608` — "nạp" các số trên [INFERENCE theo trình tự] |
| `REG_FIFOPAGE_CTRL_2` | 0x7C4 | `:616` |
| `REG_BCNQ_BDNY_V1`, `REG_FIFOPAGE_CTRL_2+2`, `REG_BCNQ1_BDNY_V1` | 0x7C4 | `:629-644` |
| `REG_RXFF_BNDY` | `24576 - 256 - 1 = 24319 = 0x5EFF` | `:647` (256 byte cuối RX FIFO dành cho C2H) |
| `REG_AUTO_LLT_V1[7:4]` = 3, `+3` = 3 | `BLK_DESC_NUM` (USB) | `:651-657` |
| `REG_AUTO_LLT_V1 \|= AUTO_INIT_LLT_V1` → poll về 0 | ≤1000 lần, **không delay** | `:661-667`; fail → `HALMAC_RET_INIT_LLT_FAIL` |
| `REG_CR+3 = 0` | `HALMAC_TRNSFER_NORMAL` | `:680` |

Kiểm tra nhất quán trong source: `rsvd_boundary != rsvd_drv_addr` → `HALMAC_RET_CFG_TXFIFO_PAGE_FAIL` (`:744`); `acq < tổng page các queue` → log `"[ERR]acqnum = %d"` (`halmac_init_88xx.c:917-926`).

Số page driver xin (`rtw_hal_get_rsvd_page_num`, `hal/hal_com.c:13908-13936`) = kết quả "chạy thử" `_rtw_hal_set_fw_rsvd_page(adapter, FALSE, &num)` + 3 (MCC) + 3 (MCC_V2) + 3 (MCC) — rồi làm tròn lên 8/16/24/32/64/128/256 (`_rsvd_page_num_drv2halmac`, `hal_halmac.c:2840-2861`). Giá trị thật: log `"request %d pages, but allocate %d pages"` (`hal_halmac.c:3131`) [KIỂM TRA TRÊN BOARD]. Với 24/32 page, PUB giảm tương ứng (`txff_sim.py 3 32`).

Khi tải firmware, `download_firmware_88xx` tạm đặt `FIFOPAGE_INFO_1 = 0x200` (512 page cho HQ) rồi khôi phục (05.08 §2).

## 6. Ring H2C-packet (`init_h2c_8822b`, `:835-880`)

```
h2cq_addr = rsvd_h2cq_addr << 7      = 2036 * 128 = 0x3F200   (mô phỏng)
h2cq_size = RSVD_PG_H2CQ_NUM << 7    = 8 * 128   = 1024 B
REG_H2C_HEAD      = (old & 0xFFFC0000) | h2cq_addr
REG_H2C_READ_ADDR = (old & 0xFFFC0000) | h2cq_addr
REG_H2C_TAIL      = (old & 0xFFFC0000) | (h2cq_addr + h2cq_size)
REG_H2C_INFO      : bit1:0 = 01, rồi bit2 = 1
REG_TXDMA_OFFSET_CHK+1 : bit7 = 1   (= RESTORE_H2C_ADDRESS[15])
kiểm tra: free space (05.08 §3.2) phải = 1024, nếu không → "[ERR]get h2c free space error!"
```

Mask `0xFFFC0000` giữ bit 31:18, thay bit 17:0 → địa chỉ byte 18 bit (≤ 256 KB = đúng cỡ TX FIFO) [tính tay].

## 7. RX DMA: dừng / nhả (WoWLAN, suspend)

| Hàm | Thao tác | Nguồn |
|---|---|---|
| `rtw_hal_pause_rx_dma` | `RXPKT_NUM \|= RW_RELEASE_EN(BIT18)`; poll `RXDMA_IDLE(BIT17)` ≤ 101 lần; USB: khi idle → `rtw_intf_stop()` (dừng URB); chưa idle → `rtw_intf_start()` để xả dữ liệu còn lại. Fail → `"Stop RX DMA failed......"` | `hal/hal_com.c:5032-5080` |
| `rtw_hal_release_rx_dma` | `RXPKT_NUM &= ~RW_RELEASE_EN` | `:5020-5030` |
| `rtw_hal_force_enable_rxdma` | `REG_WOW_CTRL(0x690) &= 0xF0` rồi nhả RX DMA | `:4996-5004` |

Tên `RW_RELEASE_EN`/`RXDMA_IDLE` ở đây lấy từ `include/hal_com_reg.h:1528-1529` (đường legacy) và trùng giá trị với `BIT_RW_RELEASE_EN`/`BIT_RXDMA_IDLE` của `halmac_bit2.h:29135-29136` [TOOL].

## 8. Phát hiện treo DMA → silent reset

`rtw_dynamic_chk_wk_hdl()` (`core/rtw_cmd.c:2881-2891`, chạy theo watchdog, `DBG_CONFIG_ERROR_DETECT` bật, không chạy ở MP mode) gọi:

- `xmit_status_check()` (`hal/rtl8822b/rtl8822b_ops.c:889-…`): `R32(REG_TXDMA_STATUS) != 0` → log `"REG_TXDMA_STATUS:0x%08x"`, `self_dect_case = 4`, **`rtw_hal_sreset_reset()`**; thêm: hết xmitbuf > 2 s → kiểm tra tiến độ TX.
- `linked_status_check()` (`:975-1000`): `R32(REG_RXDMA_STATUS) != 0` → `self_dect_case = 5`, reset; `self_dect_fw` (mailbox H2C không được đọc, 05.08 §3.1) → `self_dect_case = 3`, reset.

→ Hai thanh ghi trạng thái này là "chuông báo" chính dẫn đến reset ngầm. Khi thấy log reset, xem giá trị in ra để biết bit nào [KIỂM TRA TRÊN BOARD]; ý nghĩa từng bit của `TXDMA_STATUS`/`RXDMA_STATUS` có trong `halmac_bit_8822b.h` (section tương ứng) nhưng driver không giải mã.
