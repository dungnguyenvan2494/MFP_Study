# Giải thích file `sound/soc/samsung/i2s.c` cho người mới

Đây là **driver điều khiển khối phần cứng I2S** (một IP block) nằm bên trong các SoC Samsung/Exynos. Trong mô hình ASoC 4 lớp (Machine ↔ Platform/DMA ↔ **CPU DAI** ↔ Codec), file này là **CPU DAI** — tức "đầu I2S phía CPU". Nhiệm vụ tóm gọn: **tạo ra các xung clock âm thanh và đẩy/lấy mẫu số qua FIFO phần cứng**, để dữ liệu PCM chạy được ra chân của chip tới con codec (WM8994, MAX98090…).

Nó KHÔNG:

- tự chép dữ liệu từ RAM (việc đó là của lớp DMA — `dmaengine.c`),
- biết loa/mic nối vào đâu (việc đó là của machine driver — `odroid.c`, `smdk_wm8994.c`…),
- cấu hình codec (codec có driver riêng, nói chuyện qua I2C).

Tác giả gốc: Jaswinder Singh, Samsung, 2010 ([i2s.c:1-6](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1-L6)).

## 1. I2S trong 3 phút

I2S (Inter-IC Sound) là bus số nối tiếp truyền audio giữa 2 chip. Tối thiểu 3 dây + 1 dây clock chủ:

|Dây|Tên|Ý nghĩa|
|---|---|---|
|**BCLK** (bit clock)|SCLK|nhịp cho từng **bit**. Tần số = sample_rate × số_kênh × số_bit|
|**LRCLK** (left/right clock)|WS, FS, "frame clock"|mức 0 = kênh trái, mức 1 = kênh phải. Tần số **= sample_rate** (vd 48000 Hz)|
|**SDATA**|SD, DOUT/DIN|dữ liệu nối tiếp, 1 dây cho phát (TX), 1 cho thu (RX)|
|**MCLK**|CDCLK, SYSCLK|"master clock" cấp cho codec để nó chạy bộ chuyển đổi ADC/DAC. Thường = 256×fs hoặc 512×fs|

Hai khái niệm tỉ lệ xuất hiện khắp file:

- **RFS** = MCLK / LRCLK (vd 256, 384, 512…). Trong code: `get_rfs()`, `set_rfs()`.
- **BFS** = BCLK / LRCLK (vd 32, 48, 64…). Trong code: `get_bfs()`, `set_bfs()`.

**Master/slave**: bên nào phát BCLK+LRCLK thì bên đó là _master_. Ở đây thường I2S (CPU) là master, codec là slave (`SND_SOC_DAIFMT_CBS_CFS`). Nếu codec làm master thì I2S vào chế độ **slave** (`priv->slave_mode`, [i2s.c:712](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L712)) và khỏi tính prescaler.

## 2. Bản đồ thanh ghi (từ `i2s-regs.h`)

Driver truy cập khối I2S qua vùng nhớ ánh xạ `priv->addr`. Các thanh ghi chính:

| Offset                             | Tên               | Dùng để                                                                                                                                                                            |
| ---------------------------------- | ----------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `0x00` `I2SCON`                    | control/status    | bật/tắt DMA & kênh (`CON_TXDMA_ACTIVE`, `CON_ACTIVE`, `CON_*_PAUSE`), reset (`CON_RSTCLR`), xem FIFO đầy/rỗng                                                                      |
| `0x04` `I2SMOD`                    | mode              | **thanh ghi quan trọng nhất**: format (SDF), polarity (LRP), master/slave (MSS), RFS, BFS, độ rộng mẫu (BLC), hướng TX/RX/TXRX, bật kênh đa (DC1/DC2), chọn nguồn RCLK, gate CDCLK |
| `0x08` `I2SFIC` / `0x18` `I2SFICS` | FIFO control      | flush FIFO (`FIC_TXFLUSH`/`FIC_RXFLUSH`), đọc mức FIFO (`FIC_TXCOUNT`…)                                                                                                            |
| `0x0c` `I2SPSR`                    | prescaler         | chia clock nguồn để ra RCLK: `PSR_PSREN` + giá trị chia (bit 8..13)                                                                                                                |
| `0x10` `I2STXD` / `0x14` `I2SRXD`  | data              | cửa sổ FIFO — DMA ghi/đọc mẫu tại đây                                                                                                                                              |
| `0x1c` `I2STXDS`                   | secondary TX data | FIFO của DAI thứ hai                                                                                                                                                               |

**Vì sao có `samsung_i2s_variant_regs`** ([i2s.c:35-47](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L35-L47)): qua nhiều đời IP (v3 trên S3C6410, v5 trên S5PV210, v6 trên Exynos5420, v7 trên Exynos7), **vị trí các trường bit trong I2SMOD bị dời**. Ví dụ `bfs_off` = 1 ở v3 nhưng = 0 ở v6/v7; `rfs_mask` = 0x3 ở v3 nhưng 0x7 ở v7. Thay vì `#ifdef`, driver gói các offset đó vào struct và chọn struct đúng theo `compatible` trong Device Tree ([i2s.c:1563-1681](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1563-L1681)). Nhờ vậy cùng một hàm `set_rfs()` chạy được cho mọi đời — nó chỉ dịch bit theo `priv->variant_regs->rfs_off`.

## 3. Các cấu trúc dữ liệu

### `struct i2s_dai` — một "cổng" DAI ([i2s.c:55-85](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L55-L85))

Một khối I2S Samsung có thể trưng ra **2 DAI**:

- **Primary** (`SAMSUNG_I2S_ID_PRIMARY = 1`): phát + thu, dùng DMA ngoài.
- **Secondary** (`= 2`, chỉ khi có quirk `QUIRK_SEC_DAI`): chỉ phát, có FIFO riêng (`I2STXDS`), có thể dùng **iDMA** (SRAM on‑chip). Dùng để trộn 2 luồng nhạc mà không cần bật/tắt clock.

Các trường:

- `rfs, bfs`: ràng buộc do machine driver ép (0 = tự chọn).
- `pri_dai / sec_dai`: con trỏ chéo giữa 2 DAI của cùng khối.
- `mode`: cờ `DAI_OPENED` (đang mở) và `DAI_MANAGER` (DAI này là bên "quản lý" clock chung).
- `dma_playback / dma_capture / idma_playback`: thông tin cho lớp DMA (địa chỉ FIFO, tên kênh, độ rộng).

### `struct samsung_i2s_priv` — cả controller ([i2s.c:87-128](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L87-L128))

- `addr`: vùng thanh ghi đã ioremap.
- `clk`: clock "iis" (pclk cấp cho khối chạy).
- `op_clk`: clock nguồn sinh tín hiệu audio (opclk0/opclk1).
- `rclk_srcrate`: tần số clock nguồn RCLK (Hz) — dùng để tính prescaler.
- `variant_regs`, `quirks`: đặc tả cho đời IP hiện tại.
- `clk_table[3]`, `clk_data`: khi driver **tự làm clock provider** (mục 7).
- `suspend_i2smod/i2scon/i2spsr`: lưu để khôi phục sau suspend.
- `lock` (bảo vệ thanh ghi), `pcm_lock` (bảo vệ trạng thái mở/đóng giữa 2 DAI).
- `slave_mode`: true nếu codec làm master.

### `quirks` — cờ khác biệt phần cứng

`QUIRK_NO_MUXPSR` (không có mux+prescaler), `QUIRK_SEC_DAI` (có DAI thứ 2), `QUIRK_NEED_RSTCLR` (phải ghi `CON_RSTCLR` khi khởi động), `QUIRK_SUPPORTS_IDMA`, `QUIRK_SUPPORTS_TDM`, `QUIRK_PRI_6CHAN`. Chúng được gộp sẵn theo đời trong `i2sv3_dai_type`…`i2sv7_dai_type` ([i2s.c:1619-1650](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1619-L1650)).

## 4. Nhóm hàm helper đọc/ghi thanh ghi

- `is_secondary()`, `get_other_dai()`: điều hướng giữa primary/secondary.
- `tx_active()/rx_active()/any_active()/other_active()` ([i2s.c:137-217](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L137-L217)): đọc `I2SCON` xem có luồng nào đang chạy không. **Rất quan trọng** vì 2 DAI dùng chung 1 khối phần cứng — không được đổi format/clock khi bên kia đang phát. Chỗ nào thấy trả về `-EAGAIN` kèm log `"Other DAI busy"` là bởi kiểm tra này.
- `get_rfs/set_rfs`, `get_bfs/set_bfs` ([i2s.c:243-374](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L243-L374)): chuyển đổi qua lại giữa "số fs" (256, 384…) và mã bit trong `I2SMOD`. `set_bfs` từ chối BFS > 48 nếu IP không hỗ trợ TDM.
- `get_blc()`: đọc độ rộng mẫu hiện tại (8/16/24 bit) từ `I2SMOD[14:13]`.

### `i2s_txctrl()` / `i2s_rxctrl()` ([i2s.c:391-470](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L391-L470))

Bật/tắt một hướng truyền. Logic tinh tế nhất file:

- Bật TX: set `CON_ACTIVE | CON_TXDMA_ACTIVE`, bỏ pause; đồng thời chỉnh trường **TXR** trong `I2SMOD` (TX-only / RX-only / TX+RX) tuỳ hướng còn lại có đang chạy không.
- Tắt TX: nếu **DAI kia vẫn đang TX** thì chỉ pause phần của mình rồi return, **không** clear `CON_ACTIVE` (không được tắt clock chung). Chỉ khi không còn ai chạy mới clear `CON_ACTIVE`.

### `i2s_fifo()` ([i2s.c:473-495](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L473-L495))

Flush FIFO: set bit flush, chờ ~1µs bằng vòng busy-loop (`msecs_to_loops`), rồi clear. Gọi khi stop stream và trong probe để dọn rác.

## 5. `snd_soc_dai_ops` — vòng đời runtime

Bảng callback ([i2s.c:1107-1116](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1107-L1116)) là "API" mà lõi ASoC gọi khi ứng dụng phát/thu. Thứ tự điển hình khi phát nhạc:

```
open ─▶ startup ─▶ set_fmt / set_sysclk / set_clkdiv ─▶ hw_params
        ─▶ prepare ─▶ trigger(START) ─▶ ... ─▶ trigger(STOP) ─▶ shutdown ─▶ close
```

### `i2s_startup()` / `i2s_shutdown()` ([i2s.c:816-866](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L816-L866))

Đặt/xoá cờ `DAI_OPENED`. Quy tắc **manager**: nếu DAI kia chưa phải manager thì DAI này nhận `DAI_MANAGER` — tức bên chịu trách nhiệm cho các thiết lập clock dùng chung. Khi đóng, nhường lại manager cho bên kia nếu bên kia còn mở. `shutdown` cũng reset `rfs=bfs=0` (bỏ ràng buộc).

### `i2s_set_fmt()` ([i2s.c:623-717](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L623-L717))

Nhận cờ `SND_SOC_DAIFMT_*` từ machine driver, dịch thành bit `I2SMOD`:

- **Format**: `I2S` → `MOD_SDF_IIS`; `LEFT_J` → `MOD_SDF_LSB` + LR đảo; `RIGHT_J` → `MOD_SDF_MSB` + LR đảo.
- **Polarity** `NB_IF` → lật bit LRP.
- **Master/slave**: `CBM_CFM` (codec master) → set `mss` bit; `CBS_CFS` (CPU master) → nếu chưa có nguồn RCLK thì tự gọi `set_sysclk(RCLKSRC_0)`.
- Nếu khối đang chạy và format mới khác format cũ → `-EAGAIN`.

### `i2s_set_sysclk()` ([i2s.c:497-621](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L497-L621))

Xử lý 3 loại clock qua `clk_id`:

- `SAMSUNG_I2S_OPCLK`: chọn ý nghĩa chân OPCLK (CDCLK in/out, BCLK out, PCLK) — ghi `MOD_OPCLK_*`.
- `SAMSUNG_I2S_CDCLK`: bật/gate clock cấp cho codec. `CLOCK_IN` = gate (codec tự có clock), `CLOCK_OUT` = I2S phát MCLK ra. Kiểm tra xung đột với DAI kia.
- `SAMSUNG_I2S_RCLKSRC_0/1`: chọn nguồn sinh RCLK là `i2s_opclk0` hay `i2s_opclk1`; `clk_get` + `clk_prepare_enable`, lưu `rclk_srcrate`.

### `i2s_set_clkdiv()` ([i2s.c:978-1004](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L978-L1004))

Chỉ có `SAMSUNG_I2S_DIV_BCLK`: machine driver ép BFS cụ thể (lưu vào `i2s->bfs`).

### `i2s_hw_params()` ([i2s.c:719-813](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L719-L813))

Được gọi sau khi ALSA chốt rate/kênh/định dạng mẫu:

- Số kênh → bật `MOD_DC1_EN`/`MOD_DC2_EN` (multi-channel), hoặc set `dma_*.addr_width` (2 hoặc 4 byte) cho stereo/mono.
- Độ rộng mẫu 8/16/24 → `MOD_BLCP_*` (primary) / `MOD_BLCS_*` (secondary) / `MOD_BLC_*` (manager).
- Ghi `I2SMOD`.
- `snd_soc_dai_init_dma_data()` — **giao cấu trúc `dma_playback/dma_capture` cho lớp DMA** để nó biết địa chỉ FIFO.
- Lưu `frmclk = sample_rate` và cập nhật `rclk_srcrate`.

### `config_setup()` + `i2s_trigger()` ([i2s.c:868-976](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L868-L976))

`config_setup()` là nơi **chốt clock ngay trước khi chạy**:

1. Lấy `bfs` (từ ràng buộc, hoặc `blc*2` nếu không có).
2. Lấy `rfs` (từ ràng buộc, hoặc suy ra: BFS 16/32 → RFS 256, ngược lại 384).
3. `set_bfs()`, `set_rfs()`.
4. Nếu **không** slave mode và có prescaler: tính
    
    ```
    psr = rclk_srcrate / frmclk / rfs
    I2SPSR = ((psr - 1) << 8) | PSR_PSREN
    ```
    
    Đây là công thức trung tâm: chia clock nguồn xuống đúng `rfs × sample_rate`.

`i2s_trigger()`:

- `START/RESUME`: `pm_runtime_get_sync` → `config_setup()` → `i2s_rxctrl(1)` hoặc `i2s_txctrl(1)`.
- `STOP/SUSPEND`: `i2s_txctrl(0)` + `i2s_fifo(FLUSH)` → `pm_runtime_put`.

### `i2s_delay()` ([i2s.c:1006-1024](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1006-L1024))

Trả về số frame còn nằm trong FIFO phần cứng (đọc `FIC_TXCOUNT`/`FIC_RXCOUNT`), để ALSA báo độ trễ chính xác cho ứng dụng.

## 6. Clock provider ([i2s.c:1264-1335](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1264-L1335))

Chỉ chạy nếu Device Tree khai `#clock-cells`. Driver **đăng ký 3 clock con** để codec (và các khối khác) dùng qua DT:

- `rclk_src` = một **mux** giữa `i2s_opclk0`/`i2s_opclk1` (`clk_register_mux` ghi vào bit `rclksrc_off` của `I2SMOD`).
- `prescaler` = một **divider** (`clk_register_divider` trên `I2SPSR` bit 8, rộng 6 bit).
- `cdclk` = một **gate** (`clk_register_gate` trên bit `cdclkcon_off`) — chính là MCLK cấp ra codec.

Nhờ vậy, machine driver / codec chỉ cần `clk_set_rate()` trên `cdclk` là toàn bộ mux→prescaler→gate được lõi clock framework tính hộ. `op_clk` khi đó = cha của `rclk_src` ([i2s.c:1530](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1530)).

## 7. `samsung_i2s_probe()` — khởi tạo ([i2s.c:1379-1541](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1379-L1541))

Trình tự:

1. Lấy `i2s_dai_data` (quirks + variant_regs) theo `compatible` (DT) hoặc `platform_device_id` (legacy board file).
2. `devm_kzalloc(priv)`; `num_dais` = 2 nếu có `QUIRK_SEC_DAI`, ngược lại 1.
3. `i2s_alloc_dais()` — cấp phát mảng `i2s_dai` + `snd_soc_dai_driver`, điền tên ("samsung-i2s", "samsung-i2s-sec"), stream names, capabilities (1–2 kênh, format S8/S16/S24, dải rate), gắn `ops` và `probe/remove` cho từng DAI ([i2s.c:1151-1204](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1151-L1204)). Chỉ **primary** có capture.
4. `devm_ioremap_resource()` → `priv->addr`; `regs_base` = địa chỉ vật lý.
5. `devm_clk_get(pdev, "iis")` + `clk_prepare_enable`.
6. Điền `dma_playback.addr = regs_base + I2STXD`, `dma_capture.addr = regs_base + I2SRXD`, tên kênh "tx"/"rx".
7. `samsung_asoc_dma_platform_register()` — **đăng ký lớp Platform/DMA** cho primary.
8. Nếu có secondary: điền `I2STXDS`, iDMA addr, tạo `i2s_create_secondary_device()` (một `platform_device` phụ tên `<dev>-sec`), rồi đăng ký DMA "tx-sec".
9. `devm_snd_soc_register_component(&samsung_i2s_component, dai_drv, num_dais)` — **đăng ký CPU DAI + component** vào lõi ASoC.
10. `pm_runtime_enable()`.
11. `i2s_register_clock_provider()`.

Có nhãn `err_*` để tháo ngược đúng thứ tự khi lỗi.

### `samsung_i2s_dai_probe()` (per-DAI, [i2s.c:1041-1084](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1041-L1084))

Gọi khi lõi ASoC gắn DAI vào một sound card: `snd_soc_dai_init_dma_data()`, ghi `CON_RSTCLR` nếu cần, init iDMA, tắt sạch TX/RX, flush FIFO, và **gate CDCLK mặc định** để không rò MCLK khi chưa dùng.

## 8. DAPM ([i2s.c:1118-1146](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1118-L1146))

`samsung_i2s_component` khai 3 widget + 4 route: một **"Playback Mixer"** trộn "Primary Playback" và "Secondary Playback" vào "Mixer DAI TX". Đây chính là phần cho phép phát 2 luồng nhạc đồng thời qua secondary FIFO. Các route này khớp với DAPM route trong machine driver (vd `odroid_dapm_routes`).

## 9. Power management ([i2s.c:1206-1245](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1206-L1245), [1684-1689](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1684-L1689))

- **Runtime PM**: `i2s_runtime_suspend` lưu `I2SMOD/I2SCON/I2SPSR` rồi `clk_disable_unprepare` cả `clk` lẫn `op_clk`. `i2s_runtime_resume` bật clock lại và **ghi trả 3 thanh ghi** (thứ tự CON → MOD → PSR).
- **System sleep**: `i2s_suspend/resume` gọi `pm_runtime_force_suspend/resume`.
- Mẫu hình xuyên suốt file: mọi hàm chạm thanh ghi đều bọc `pm_runtime_get_sync(dai->dev)` … `pm_runtime_put(dai->dev)` để đảm bảo clock đang bật.
## 10. Bảng thiết bị ([i2s.c:1652-1702](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1652-L1702))

- `platform_device_id` (`"samsung-i2s"` → i2sv3): đường cũ, board file không dùng DT.
- `of_device_id exynos_i2s_match[]`: DT hiện đại — `s3c6410-i2s`(v3), `s5pv210-i2s`(v5), `exynos5420-i2s`(v6), `exynos7-i2s`(v7), `exynos7-i2s1`(v5-i2s1).
- `module_platform_driver(samsung_i2s_driver)` sinh hàm `module_init/exit` chuẩn.

Lưu ý mẹo: `pdev_sec->driver_override = "samsung-i2s"` ([i2s.c:1353](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1353)) khiến `probe` chạy lại cho thiết bị phụ, và nhánh `if (!id) return 0;` ([i2s.c:1397](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1397)) bỏ qua nó — thiết bị phụ chỉ tồn tại để có một `device` riêng cho lớp DMA "tx-sec".

## 11. Đồng bộ hoá — điều dễ sai nhất

Hai spinlock:

- `priv->lock`: bảo vệ các lần đọc-sửa-ghi `I2SMOD/I2SCON/I2SPSR` (read-modify-write không nguyên tử).
- `priv->pcm_lock`: bảo vệ `i2s->mode` (OPENED/MANAGER) khi 2 DAI mở/đóng song song.

Vì primary và secondary **chia sẻ một khối phần cứng**, gần như mọi hàm `set_*` đều có nhánh: "nếu khối đang chạy và giá trị mới khác giá trị đang chạy → trả `-EAGAIN` + log `Other DAI busy`". Người mới hay tưởng đây là lỗi; thực ra là **từ chối thay đổi an toàn** — machine driver phải cấu hình 2 luồng cho tương thích (cùng rate/format).

## 12. Đọc file theo thứ tự nào

1. `i2s-regs.h` — thuộc mặt bằng thanh ghi (I2SCON/I2SMOD/I2SPSR/I2SFIC).
2. `struct i2s_dai`, `struct samsung_i2s_priv`, `quirks`, `samsung_i2s_variant_regs`.
3. Helper: `is_secondary`, `tx_active`, `get_rfs/set_rfs`, `get_bfs/set_bfs`.
4. `i2s_txctrl`, `i2s_rxctrl`, `i2s_fifo`.
5. `samsung_i2s_dai_ops` rồi lần lượt: `startup` → `set_fmt` → `set_sysclk` → `hw_params` → `config_setup` → `trigger` → `delay`.
6. `i2s_register_clock_provider`.
7. `samsung_i2s_probe` (đọc cuối vì nó "ráp" mọi thứ ở trên).
8. Đối chiếu với `odroid.c` (machine) và `dmaengine.c` (platform) để thấy 3 lớp ăn khớp.

Bản rút gọn để hiểu nhanh cùng ý tưởng nhưng chỉ có TX, không framework: [BootROM/.../sound/samsung-i2s.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/drivers/sound/samsung-i2s.c).
