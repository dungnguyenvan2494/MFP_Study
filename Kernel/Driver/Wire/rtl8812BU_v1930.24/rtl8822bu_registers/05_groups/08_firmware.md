# 05.08 — Firmware, mailbox và giao tiếp host ↔ firmware

> RTL8822B có một CPU nhúng (8051-class, WLAN CPU) chạy firmware. Driver phải **tải firmware** vào IMEM/DMEM của CPU đó, rồi nói chuyện với nó bằng **H2C** (host-to-card) và nhận **C2H** (card-to-host). Phần này chỉ phân tích *thanh ghi* làm việc đó; nội dung các lệnh H2C (là gói dữ liệu, không phải thanh ghi) không thuộc phạm vi.
>
> Nhãn như 05.01.

## 1. Thanh ghi tham gia

| Địa chỉ | Tên | St | Field có bằng chứng | Hàm |
|---|---|:-:|---|---|
| `0x0003` bit 2 | `REG_SYS_FUNC_EN.FEN_CPUEN[10]` | A | bật/tắt CPU | `wlan_cpu_en_88xx` |
| `0x0009` bit 6 | `REG_SYS_CLK_CTRL.CPU_CLK_EN[14]` | A | clock CPU | `pltfm_reset_88xx` |
| `0x001D` bit 0 | `REG_RSV_CTRL.WLMCUIOIF[8]` | A | giao tiếp IO của MCU | `wlan_cpu_en_88xx` |
| `0x0080–0x0081` | `REG_MCUFW_CTRL_8822B` | A | `MCUFWDL_EN[0]`, `IMEM_DW_OK`, `IMEM_CHKSUM_OK[4]`, `DMEM_DW_OK`, `DMEM_CHKSUM_OK[6]`, `CCLK_CHG_MASK[11]`, `CPU_CLK_SEL[13:12]`, `FW_DW_RDY[14]`, `FW_INIT_RDY[15]`, `BOOT_FSPI_EN` | `start_dlfw_88xx`, `check_fw_chksum_88xx`, `dlfw_end_flow_88xx`, `download_firmware_88xx`, `init_system_cfg_8822b`, `mac_pwr_switch_usb_8822b` |
| `0x0084` | `REG_MCU_TST_CFG_8822B` | A | byte "trạng thái" FW ↔ host: `0x80` = EMEM ready, `0x05` = CPU vào sleep | `free_dl_fw_end_flow_88xx`, `get_cpu_mode_88xx` |
| `0x01A0`, `0x01AF` | `REG_C2HEVT_8822B` (`REG_C2HEVT_MSG_NORMAL`=0x1A0, `REG_C2HEVT_CLEAR`=0x1AF, `rtl8822b_hal.h:80-81`) | A | C2H qua thanh ghi | `hal_read_mac_hidden_rpt`, `c2h_evt_read_88xx`/`c2h_evt_clear` (`hal_com.c:1431-1465`) |
| `0x01CC–0x01CF` | `REG_HMETFR_8822B` | A | `INT_BOX0..3[3:0]` (box *n* đang chờ FW đọc), `HRCV_MSG[31:24]` | `_is_fw_read_cmd_down`, `free_dl_fw_end_flow_88xx` |
| `0x01D0–0x01DF` | `REG_HMEBOX0..3_8822B` | R | 4 mailbox × 4 byte | `rtw_halmac_send_h2c` (địa chỉ tính `REG_HMEBOX0 + n*4`) |
| `0x01F0–0x01FF` | `REG_HMEBOX_E0..E3_8822B` | R | 4 mailbox mở rộng × 4 byte | như trên |
| `0x0204–0x0205` | `REG_FIFOPAGE_CTRL_2_8822B` | A | `BCN_HEAD_V1[11:0]` (page đích), `BCN_VALID_V1[15]` (W1 = arm, HW đặt 1 = xong) | `dl_rsvd_page_88xx` |
| `0x0210` | `REG_TXDMA_STATUS_8822B` | A | `PAGE_OVF[2]` — ghi `BIT(2)` sau tải FW | `dlfw_end_flow_88xx` |
| `0x0244–0x0254` | `REG_H2C_HEAD/TAIL/READ_ADDR/INFO_8822B` | A | vùng đệm H2C-packet trong TX FIFO | `init_h2c_8822b` |
| `0x10D0`, `0x10D4` | `REG_H2C_PKT_READADDR/WRITEADDR_8822B` | A | con trỏ đọc (FW) / ghi (HW) của hàng H2C, 18 bit | `get_h2c_buf_free_space_88xx` |
| `0x10F8`, `0x10FC` | `REG_FW_DBG6/7_8822B` | A | thông tin debug FW; `FW_DBG7 & 0xFFFFFF00 == 0xFAAAAA00` = "illegal key" | `dlfw_end_flow_88xx`, `check_fw_status_88xx` |
| `0x1080–0x1082` | `REG_CPU_DMEM_CON_8822B` (`R_0x1080`) | A | `WL_PLATFORM_RST[16]` | `init_system_cfg_8822b`, `pltfm_reset_88xx` |
| `0x1200/0x1204/0x1208` | `REG_DDMA_CH0SA/DA/CTRL_8822B` | A | DMA nội bộ kênh 0: nguồn, đích, `OWN[31]`, `CHKSUM_EN[29]`, `CHKSUM_STS[27]`, `RESET_CHKSUM_STS[25]`, `CHKSUM_CONT[24]`, `DLEN[17:0]` | `iddma_en_88xx`, `iddma_dlfw_88xx`, `dlfw_to_mem_88xx`, `check_fw_chksum_88xx` |
| `0x1330` | `REG_H2CQ_CSR_8822B` | A | ghi `BIT(31)` | `download_firmware_88xx`, `init_trx_cfg_8822b` |
| `0x1700/0x1704/0x1708` | `REG_WL2LTECOEX_INDIRECT_ACCESS_CTRL/WRITE_DATA/READ_DATA_V1` | A | **cửa sổ gián tiếp** tới không gian LTE-coex (§5) | `ltecoex_reg_read/write_88xx` |

## 2. Chuỗi tải firmware (`download_firmware_88xx`, `halmac_fw_88xx.c:157-273`)

Điều kiện đầu vào: MAC đã bật nguồn (`mac_pwr != OFF`, nếu không → `HALMAC_RET_POWER_STATE_INVALID`).

```mermaid
sequenceDiagram
    participant D as Driver (HALMAC)
    participant M as Thanh ghi MAC
    participant F as TX FIFO (reserved page)
    participant C as WLAN CPU
    D->>M: LTE-coex[0x38] → backup (gián tiếp 0x1700/0x1708)
    D->>M: wlan_cpu_en(0): FEN_CPUEN=0, WLMCUIOIF=0
    D->>M: backup & đặt TXDMA_PQ_MAP+1 = HIQ→HIGH(3)<<6; CR = HCI_TXDMA_EN|TXDMA_EN; H2CQ_CSR = BIT31
    D->>M: backup & đặt FIFOPAGE_INFO_1 = 0x200; RQPN_CTRL_2 |= BIT31
    D->>M: BCN_CTRL: EN_BCN_FUNCTION=0, DIS_TSF_UDT=1
    D->>M: pltfm_reset: CPU_DMEM_CON+2.b0=0, CPU_CLK_EN=0, …=1, CPU_CLK_EN=1
    D->>M: MCUFW_CTRL = (MCUFW_CTRL & 0x3800) | MCUFWDL_EN
    loop từng mảnh ≤ dlfw_pkt_size, cho DMEM rồi IMEM (rồi EMEM)
        D->>F: gửi mảnh FW như reserved page (bulk-out) — dl_rsvd_page_88xx
        D->>M: poll FIFOPAGE_CTRL_2.BCN_VALID (≤1000×10µs)
        D->>M: DDMA_CH0SA = 0x18780000 + src + 48; DA = đích; CTRL = OWN|CHKSUM_EN|len (|CHKSUM_CONT nếu không phải mảnh đầu)
        D->>M: poll DDMA_CH0CTRL.OWN == 0 (≤1000 lần)
    end
    D->>M: CHKSUM_STS? → đặt IMEM/DMEM_DW_OK, _CHKSUM_OK trong MCUFW_CTRL
    D->>M: restore 6 thanh ghi đã backup
    D->>M: TXDMA_STATUS = BIT2; kiểm tra MCUFW_CTRL & 0x50 == 0x50
    D->>M: MCUFW_CTRL = (… | FW_DW_RDY) & ~MCUFWDL_EN
    D->>M: wlan_cpu_en(1)
    C-->>M: firmware boot, đặt MCUFW_CTRL = 0xC078
    D->>M: poll R16(MCUFW_CTRL) == 0xC078 (≤5000×50µs ≈ 250 ms + USB)
    D->>M: LTE-coex[0x38] ← restore
```

### 2.1 Phân tích từng thao tác thanh ghi

| Bước | Code | Thanh ghi / phép toán | Vì sao (theo source) |
|---|---|---|---|
| backup LTE | `:182` | gián tiếp LTE-coex offset 0x38 | khôi phục ở `:247`/`:268` — FW download có thể làm thay đổi [INFERENCE] |
| tắt CPU | `:186` | `0x03 &= ~BIT2`, `0x1D &= ~BIT0` | không cho FW cũ chạy khi đang ghi IMEM/DMEM |
| định tuyến HIQ | `:189-197` | `0x10D = HALMAC_DMA_MAPPING_HIGH(3) << 6` → `TXDMA_HIQ_MAP[15:14] = 3` | gói reserved-page đi bằng hàng đợi High |
| bật TX DMA | `:198-208` | `REG_CR(0x100) = BIT_HCI_TXDMA_EN(BIT0) \| BIT_TXDMA_EN(BIT2)` — **ghi đè cả byte** (các bit khác của CR byte 0 = 0 trong lúc tải) | chỉ cần TX DMA để đưa FW vào FIFO |
| H2C queue | `:208` | `REG_H2CQ_CSR = BIT(31)` | |
| page | `:211-220` | `FIFOPAGE_INFO_1 = 0x200` (16-bit), `RQPN_CTRL_2 \|= BIT31` | cấp page cho hàng High [INFERENCE theo tên] |
| beacon | `:223-229` | `BCN_CTRL = (v & ~EN_BCN_FUNCTION) \| DIS_TSF_UDT` | tắt beacon để reserved-page (dùng chung đường beacon queue) không bị phát ra không khí |
| reset nền tảng | `:234` → `:427-450` | `0x1082 b0` (= `WL_PLATFORM_RST[16]`) 0→1, xen kẽ `CPU_CLK_EN` 0→1 | |
| mở chế độ tải | `start_dlfw :307-309` | `MCUFW_CTRL = (MCUFW_CTRL & 0x3800) \| BIT0` | giữ `CCLK_CHG_MASK[11]`, `CPU_CLK_SEL[13:12]`, xoá mọi cờ cũ, đặt `MCUFWDL_EN` |
| mỗi mảnh | `dlfw_to_mem :623-651` | `DDMA_CH0CTRL \|= RESET_CHKSUM_STS` (1 lần đầu); với từng mảnh: reserved page + DDMA | xem 2.2 |
| checksum | `check_fw_chksum :845-877` | nếu `DDMA_CH0CTRL & CHKSUM_STS(BIT27)` → **lỗi**: đặt `*_DW_OK`, xoá `*_CHKSUM_OK`; ngược lại đặt cả hai | `mem_addr < OCPBASE_DMEM_88XX (0x00200000)` → IMEM, ngược lại DMEM |
| kết thúc | `dlfw_end_flow :690-726` | `TXDMA_STATUS = BIT2` (`PAGE_OVF` — có thể là W1C [INFERENCE]); `(MCUFW_CTRL & 0x50) == 0x50` ↔ `IMEM_CHKSUM_OK[4]` và `DMEM_CHKSUM_OK[6]`; ghi `FW_DW_RDY`, xoá `MCUFWDL_EN`; bật CPU; chờ `0xC078` | |

`0xC078` [giải bằng `tools/bitname.py`]: `1100 0000 0111 1000₂` → `FW_INIT_RDY[15]`, `FW_DW_RDY[14]`, `DMEM_CHKSUM_OK[6]`, `DMEM_DW_OK[5]`, `IMEM_CHKSUM_OK[4]`, `IMEM_DW_OK[3]` — đúng 6 cờ mà chuỗi tải FW đặt, cộng `FW_INIT_RDY`. Tức "firmware đã tải xong, checksum đúng và **đã khởi tạo xong**"; `FW_INIT_RDY` do FW tự đặt [SOURCE-SEQ: driver không ghi bit 15, chỉ chờ].

Giá trị `0xC078` còn được dùng ở 2 nơi khác như "chữ ký FW đang chạy": `mac_pwr_switch_usb_8822b` (05.01 §4) và trong debug. Power-off xoá `FW_DW_RDY`/`FW_INIT_RDY` (pwr-seq bước 16, 05.01 §5).

### 2.2 Một mảnh firmware đi thế nào — *đây không phải truy cập thanh ghi*

1. `send_fwpkt_88xx` (`:762-793`) → `dl_rsvd_page_88xx(pg_addr = src >> 7, …)` (`halmac_common_88xx.c:360-419`):
   - `FIFOPAGE_CTRL_2 = pg_addr | BIT(15)` (`BCN_HEAD_V1` = page đích, `BCN_VALID_V1` = 1 để "arm")
   - `CR+1 |= BIT0` → `ENSWBCN[8]` (cho phép beacon do SW tạo)
   - `FWHW_TXQ_CTRL+2 &= ~BIT6` → `EN_BCNQ_DL[22]` = 0
   - **gửi buffer qua bulk-out** (`PLTFM_SEND_RSVD_PAGE` = `usb_write_data_rsvd_page`, `hal/rtl8822b/usb/rtl8822bu_halmac.c:323,366`)
   - poll `FIFOPAGE_CTRL_2+1 & BIT7` (`BCN_VALID_V1`) ≤ 1000 × 10 µs; HW đặt lại 1 khi gói đã nằm trong FIFO [SOURCE-SEQ] — log lỗi `"[ERR]bcn valid!!"`
   - khôi phục `BCN_HEAD = rsvd_boundary | BIT15`, `FWHW_TXQ_CTRL+2`, `CR+1`.
   - Ngoại lệ USB: nếu `(size + 48) % 512 == 0` thêm 1 byte giả (`:768-786`) — tránh transfer đúng bội số 512 byte (USB cần zero-length packet) [SOURCE + INFERENCE lý do].
2. `iddma_dlfw_88xx` (`:796-823`) → `iddma_en_88xx` (`:826-842`): DMA nội bộ copy từ TX buffer (`OCPBASE_TXBUF_88XX = 0x18780000` + `src` + 48 byte TX descriptor) tới IMEM/DMEM.

Ví dụ tính `DDMA_CH0CTRL` cho mảnh thứ 2 dài 0x800 byte [tính tay]:

```
BIT_DDMACH0_CHKSUM_EN  = 0x2000_0000   (bit 29)
BIT_DDMACH0_OWN        = 0x8000_0000   (bit 31)  → HW xoá khi copy xong
len & 0x3FFFF          = 0x0000_0800
BIT_DDMACH0_CHKSUM_CONT= 0x0100_0000   (bit 24, vì không phải mảnh đầu → checksum cộng dồn)
ctrl                   = 0xA100_0800
```

Poll `OWN` dùng `HALMC_DDMA_POLLING_COUNT = 1000` lần **không có delay** — mỗi vòng là 1 control transfer (`0x1208 ≥ 0x1000` → không retry, không ghi 0x4E0).

### 2.3 Nhánh lỗi

`DLFW_FAIL` (`:257-272`): xoá `MCUFWDL_EN`, **bật lại `FEN_CPUEN`** (CPU chạy lại firmware cũ/ROM), khôi phục LTE-coex. Log chính: `"[ERR]Check 0x80 = 0xC078 fail"`, `"[ERR]Key!!"` (`FW_DBG7` = `0xFAAAAAxx` → `HALMAC_RET_ILLEGAL_KEY_FAIL`), `"[ERR]fw chksum!!"`, `"[ERR]iddma dlfw!!"`, `"[ERR]ch0 ready!!"`.

## 3. H2C — hai kênh song song

### 3.1 Mailbox (thanh ghi) — `rtw_halmac_send_h2c()` (`hal/hal_halmac.c:4092-4150`)

Dùng bởi `rtl8822b_fillh2ccmd` (`hal/rtl8822b/rtl8822b_cmd.c:112,132,135`) cho lệnh H2C "cũ" 8 byte.

```c
box = hal->LastHMEBoxNum;                                 /* 0..3, xoay vòng */
_is_fw_read_cmd_down(adapter, box);                       /* chờ REG_HMETFR(0x1CC) bit box == 0, ≤100 × 1 ms */
rtw_write32(adapter, REG_HMEBOX_E0 + box*4, h2c[4..7]);  /* phần mở rộng TRƯỚC */
rtw_write32(adapter, REG_HMEBOX0   + box*4, h2c[0..3]);  /* phần chính SAU → kích FW */
hal->LastHMEBoxNum = (box + 1) % 4;
```

| Bit | Ý nghĩa theo cách dùng | Nguồn |
|---|---|---|
| `REG_HMETFR.INT_BOX<n>[n]` | 1 = box *n* còn lệnh FW chưa đọc → host phải chờ; 0 = trống | `hal_halmac.c:4066-4071` [SOURCE-SEQ] |

Thứ tự ghi EX trước, chính sau là bắt buộc [INFERENCE]: ghi box chính làm HW đặt cờ `INT_BOX<n>` → FW đọc cả hai; nếu ngược thứ tự FW có thể đọc EX cũ. Toàn bộ chạy dưới `h2c_fwcmd_mutex`. Lỗi: `"fw read cmd failed..."`, `"reg_1cc(%x), msg_box(%d)..."` và (vì `DBG_CONFIG_ERROR_DETECT` bật) `srestpriv.self_dect_fw = TRUE` → có thể kích silent-reset (`core/rtw_sreset.c`).

Vì `REG_HMEBOX0` chỉ xuất hiện trong phép cộng `REG_HMEBOX0 + (box * MESSAGE_BOX_SIZE)` nên danh mục xếp nó là **R** (dùng qua biến); địa chỉ thực tế: `0x1D0/0x1D4/0x1D8/0x1DC` và `0x1F0/0x1F4/0x1F8/0x1FC` [tính tay từ `MESSAGE_BOX_SIZE = EX_MESSAGE_BOX_SIZE = 4`, `MAX_H2C_BOX_NUMS = 4`, `hal/hal_halmac.h:73-75`].

### 3.2 H2C packet (FIFO) — `send_h2c_pkt_88xx()` (`halmac_common_88xx.c:714-744`)

Dùng bởi HALMAC cho các lệnh offload (gói 32 byte, `H2C_PKT_SIZE_88XX`). Gói đi **bulk-out** (`usb_write_data_h2c`, `rtl8822bu_halmac.c:340,367`); thanh ghi chỉ dùng để tính chỗ trống:

```
hw_wptr = R32(REG_H2C_PKT_WRITEADDR 0x10D4) & 0x3FFFF
fw_rptr = R32(REG_H2C_PKT_READADDR  0x10D0) & 0x3FFFF
free = (hw_wptr >= fw_rptr) ? buf_size - (hw_wptr - fw_rptr) : fw_rptr - hw_wptr
```

Đây là ring buffer chuẩn: HW tăng `wptr` khi nhận gói, FW tăng `rptr` khi xử lý. Hết chỗ sau 100 lần thử → `"[ERR]h2c free space!!"`. Vùng ring được cấu hình ở `init_h2c_8822b` qua `REG_H2C_HEAD/TAIL/READ_ADDR/INFO` (xem 05.04).

## 4. C2H qua thanh ghi (`c2h_evt_clear`/`c2h_evt_read_88xx` `hal_com.c:1429-1470`, `hal_read_mac_hidden_rpt` `:1763-1810`)

- Đọc: `REG_C2HEVT_CLEAR` (0x1AF) = trigger; `REG_C2HEVT_MSG_NORMAL` (0x1A0) = ID; payload ở `0x1A2 + i`; xong ghi `REG_C2HEVT_CLEAR = C2H_EVT_HOST_CLOSE`.
- `hal_read_mac_hidden_rpt()` (`:1763`): ghi `REG_C2HEVT_MSG_NORMAL = C2H_DEFEATURE_RSVD` rồi gửi H2C yêu cầu FW báo cáo "hidden feature", poll ID, đọc `0x1A2+i` — dùng để biết chip bị khoá tính năng (ví dụ băng thông) [SOURCE cho thao tác; ý nghĩa "hidden rpt" theo tên hàm].
- Phần lớn C2H thực tế với USB đi theo **gói RX** (C2H packet trong RX aggregation), không qua thanh ghi — xem 05.05.

## 5. Không gian gián tiếp LTE-coex (`halmac_common_88xx.c:3473-3523`)

```
chờ R8(0x1703) & BIT5 == 1          ≤ 10000 × 50 µs   (cờ "ready" của cửa sổ)
đọc : W32(0x1700, 0x800F0000 | off);  v = R32(0x1708)
ghi : W32(0x1704, v);  W32(0x1700, 0xC00F0000 | off)
```

Giải mã word điều khiển [`tools/bitname.py`]: `0x800F0000` = `LTECOEX_ACCESS_START_V1[31]` + `WRITE_BYTE_EN_V1[19:16] = 0xF` (cả 4 byte), `off` ở bit 15:0; lệnh ghi thêm `LTECOEX_WRITE_MODE_V1[30]` (`0xC00F0000`). Cờ chờ `0x1703 & BIT5` = `LTECOEX_READY_BIT_V1[29]`. Đây là **không gian địa chỉ thứ 6** (ngoài MAC/BB/RF/USB-local/USB3-PHY): offset `0x38` trong không gian LTE-coex **không phải** MAC `0x38` (`REG_PWR_OPTION_CTRL`). Trong build này chỉ có đúng một offset được dùng: `0x38`, đọc-backup/ghi-restore quanh việc tải FW (`halmac_fw_88xx.c:182,247,268,525,534`). Ý nghĩa: `ex_hal8822b_wifi_only_hw_config` ghi offset này = `0x7700` với comment **"gnt_wl=1 , gnt_bt=0"** (`hal/btc/halbtc8822bwifionly.c`, 05.17 §2) → offset `0x38` chứa điều khiển tín hiệu GNT_WL/GNT_BT [SOURCE-comment]; ý nghĩa từng bit vẫn chưa có nguồn.

## 6. Đọc trạng thái CPU

| Hàm | Thanh ghi | Logic | Nguồn |
|---|---|---|---|
| `get_cpu_mode_88xx` | `0x84` | `== ID_CHECK_ENETR_CPU_SLEEP (0x05)` → CPU sleep, ghi lại 0 | `halmac_fw_88xx.c:1069-1072` |
| `free_dl_fw_end_flow_88xx` (tải EMEM khi FW đang chạy) | `0x1CF`, `0x84` | chờ `HRCV_MSG` (0x1CF) == 0 ≤100×50µs; ghi `0x1CF = ID_INFORM_DLEMEM_RDY (0x80)`; chờ `0x84 == 0x80` ≤ 10000×50µs; xoá `0x84` | `:729-759` |
| `pwr_state_88xx` | `0x03` | `SYS_FUNC_EN+1 & BIT3` (`FEN_DCORE[11]`) == 0 → MAC off | `halmac_common_88xx.c:3526-3534` |
| `check_fw_status_88xx` | `0x10F8`, `0x10FC` | đọc `FW_DBG6/7`, PC của FW | `:887-…` |

## 7. Dấu hiệu lỗi và cách kiểm tra

| Log | Ý nghĩa | Thanh ghi nên đọc |
|---|---|---|
| `[ERR]Check 0x80 = 0xC078 fail` | FW không khởi tạo xong | `0x80` (16-bit), `0x10FC` |
| `[ERR]Key!!` | FW bị từ chối (key group) | `0x10FC & 0xFFFFFF00` |
| `[ERR]fw chksum!!` | checksum IMEM/DMEM sai (file FW hỏng/USB lỗi) | `0x1208` bit 27 |
| `[ERR]bcn valid!!` | reserved page không vào FIFO | `0x204–0x205`, `0x100`, `0x420–0x422` |
| `fw read cmd failed...` / `reg_1cc(..)` | FW không đọc mailbox (treo / đang ngủ) | `0x1CC`, `0x84` |
| `[ERR]h2c free space!!` | FW không tiêu thụ H2C packet | `0x10D0`, `0x10D4` |
