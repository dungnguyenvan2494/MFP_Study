## Các đầu mục tìm hiểu I2S driver

### 1. Vị trí trong stack ALSA/ASoC

- Mô hình 4 lớp: **Machine driver** ↔ **Platform/DMA** ↔ **CPU DAI (chính là I2S)** ↔ **Codec DAI**.
- I2S ở đây là _CPU DAI driver_ (còn gọi cpu-dai / dai-cpu), đăng ký qua `snd_soc_component_driver` + `snd_soc_dai_driver`.
- File khung: [soc-dai.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/include/sound/soc-dai.h), driver chính: [sound/soc/samsung/i2s.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c).

## 1.1. Mô hình 4 lớp của ASoC (ALSA System-on-Chip)
ASoC tách driver âm thanh nhúng thành 4 mảnh độc lập để **tái sử dụng**: cùng một codec dùng được trên nhiều SoC, cùng một khối I2S dùng được với nhiều codec, chỉ phần "ghép nối" là riêng cho từng bo mạch.

```
  ┌─────────────────────────────────────────────────────────────┐
  │  MACHINE DRIVER  (odroid.c)  — snd_soc_card + snd_soc_dai_link[] │
  │  "bo mạch này: I2S số mấy nối với codec nào, format gì"       │
  └───────┬───────────────────┬──────────────────────┬──────────┘
          │ mỗi dai_link ghép 1 phần tử từ mỗi lớp:  │
          ▼                   ▼                      ▼
   ┌────────────┐     ┌───────────────┐      ┌──────────────┐
   │ PLATFORM/  │     │   CPU DAI     │      │  CODEC DAI    │
   │ DMA (PCM)  │◄───►│  (I2S) i2s.c  │◄────►│ wm8994/max... │
   │dmaengine.c │ FIFO│ dai_ops       │ dây  │ dai_ops +     │
   │            │ addr│ hw_params/... │ I2S  │ widgets/ctls  │
   └─────┬──────┘     └───────┬───────┘      └──────┬───────┘
     DMA │ RAM↔FIFO       SFR │ I2SCON/MOD/PSR   I2C/│SPI cấu hình
         ▼                    ▼                     ▼
      bộ nhớ            chân BCLK/LRCLK/SDATA/MCLK  loa/mic

```

### Lớp 1 — Codec DAI (+ codec component)
- Driver của chip codec (WM8994, MAX98090, UDA134x…). Đăng ký `snd_soc_dai_driver` (khai báo rate/format hỗ trợ, `snd_soc_dai_ops`) **và** `snd_soc_component_driver` chứa **DAPM widgets + kcontrols** (đường tín hiệu, mixer, mute, gain).
- Được cấu hình qua bus điều khiển **I2C/SPI** (khác với đường dữ liệu I2S).
- Trong `odroid.c` codec lấy từ Device Tree: `snd_soc_of_get_dai_link_codecs()` ([odroid.c:284](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/odroid.c#L284)); khi không có codec thật thì dùng `COMP_DUMMY()`.

### Lớp 2 — CPU DAI = khối I2S trong SoC ([i2s.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c))

- Là một `platform_driver` ([i2s.c:1691](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1691)), probe map vùng thanh ghi SFR, xin clock, rồi `devm_snd_soc_register_component()` với `snd_soc_dai_driver` + `samsung_i2s_dai_ops` ([i2s.c:1107](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1107), :1517).
- Trách nhiệm: sinh **BCLK/LRCLK/MCLK**, chọn format (`i2s_set_fmt` [:623]), tỉ lệ clock (`i2s_set_sysclk` [:497], `config_setup` [:868]), bật/tắt luồng (`i2s_trigger` [:929] → `i2s_txctrl`/`i2s_rxctrl`).
- **Không tự đụng vào bộ nhớ** — nó chỉ báo cho lớp Platform biết địa chỉ FIFO phần cứng qua `snd_soc_dai_init_dma_data()` ([i2s.c:804](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L804), :1052): `dma_playback.addr = regs_base + I2STXD`, `dma_capture.addr = regs_base + I2SRXD` [:1464-1465]).

### Lớp 3 — Platform / DMA (thành phần PCM) ([dmaengine.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/dmaengine.c))

- `samsung_asoc_dma_platform_register()` → `devm_snd_dmaengine_pcm_register()` ([dmaengine.c:17-37](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/dmaengine.c#L17-L37)). Chính khối I2S gọi hàm này lúc probe ([i2s.c:1475](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1475), :1501 cho secondary).
- Trách nhiệm: cấp phát buffer, khai báo `snd_pcm_hardware` (kích thước period, số kênh…), và dùng **dmaengine** để chuyển dữ liệu giữa **RAM ↔ FIFO I2S** ở địa chỉ mà lớp CPU DAI đã đưa. Trên dòng cũ có thể là DMA riêng của Samsung; ngoài ra I2S còn có **iDMA** dùng SRAM on-chip (`idma_playback.addr`, `idma.c`).

### Lớp 4 — Machine driver (keo dán) ([odroid.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/odroid.c))

Chi tiết [[MACHINE DRIVER  (odroid.c)]]

- Tạo `struct snd_soc_card` chứa mảng `snd_soc_dai_link[]`, rồi `devm_snd_soc_register_card()` ([odroid.c:305](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/odroid.c#L305)).
- **Mỗi `dai_link` ghép đúng 1 phần tử từ mỗi lớp**: khai báo bằng `SND_SOC_DAILINK_DEFS(name, <cpu>, <codec>, <platform>)` và gắn vào link bằng `SND_SOC_DAILINK_REG(name)` ([odroid.c:153-165](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/odroid.c#L153-L165)):
    
    ```c
    SND_SOC_DAILINK_DEFS(primary,
        DAILINK_COMP_ARRAY(COMP_EMPTY()),                 // CPU DAI  (điền từ DT)
        DAILINK_COMP_ARRAY(COMP_DUMMY()),                 // CODEC DAI
        DAILINK_COMP_ARRAY(COMP_PLATFORM("3830000.i2s"))); // PLATFORM/DMA
    ```
    
- Machine driver cũng đặt **quan hệ master/slave và format chung** cho link: `.dai_fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS` ([odroid.c:183](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/odroid.c#L183)) — `CBS_CFS` = codec là slave, I2S (CPU) sinh BCLK/LRCLK.
- Và đặt clock theo sample-rate trong `snd_soc_ops` của nó: `odroid_card_be_hw_params()` tính `pll_freq`/`rfs`, gọi `clk_set_rate()` cho `sclk_i2s` và `snd_soc_dai_set_sysclk(codec_dai, …)` ([odroid.c:56-118](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/odroid.c#L56-L118)).

### Luồng runtime khi phát nhạc (đi qua cả 4 lớp)

ASoC core (`sound/soc/soc-core.c`, `soc-pcm.c`, `soc-dapm.c`) tạo một thiết bị PCM cho mỗi `dai_link`. Khi ứng dụng mở và ghi dữ liệu:

1. **open/startup** → gọi `.startup` của machine ops, rồi `i2s_startup()` (CPU) và `.startup` của codec.
2. **hw_params** (chốt rate/format/kênh) → lan xuống **codec `hw_params`** (cấu hình ADC/DAC qua I2C) → **CPU `i2s_hw_params()`** [i2s.c:719] (ghi I2SMOD/I2SPSR, chọn `dma_playback`) → **platform** cấp buffer + cấu hình kênh DMA tới `regs_base + I2STXD`.
3. **prepare** → `config_setup()` chốt RFS/BFS/prescaler.
4. **trigger(START)** → platform khởi động DMA (RAM→FIFO) → `i2s_trigger()` [i2s.c:929] bật `CON_TXDMA_ACTIVE` + FIFO → I2S phát BCLK/LRCLK/SDATA ra dây → codec (đã bật đường DAPM) nhận và đưa ra loa.
5. **DAPM** đóng/mở các widget dọc đường "playback stream → codec DAC → HP amp" theo route khai trong DT (`samsung,audio-routing`).

### Vì sao tách 4 lớp

|Lớp|Thay đổi khi…|Không phải sửa|
|---|---|---|
|Codec DAI|đổi chip codec|I2S, DMA, machine|
|CPU DAI (I2S)|đổi SoC/IP I2S|codec, machine|
|Platform/DMA|đổi bộ điều khiển DMA|codec, I2S|
|Machine|đổi bo mạch (nối chân, chọn format, clock)|3 lớp kia|

Trong repo này: khối I2S = `i2s.c` (dùng lại cho mọi Exynos), platform = `dmaengine.c`, còn các bo mạch khác nhau chỉ khác ở machine driver (`odroid.c`, `arndale.c`, `smdk_wm8994.c`, `tm2_wm5110.c`, `snow.c`…), mỗi file chỉ khai `snd_soc_card` + `dai_link` + clock/format riêng.

### 2. Lý thuyết giao thức I2S

- 4 tín hiệu: **BCLK** (bit clock), **LRCLK/WS** (word/frame select), **SDATA**, **MCLK/SYSCLK/CDCLK** (master clock cấp cho codec).
- Quan hệ tần số: `RFS` = MCLK/LRCLK, `BFS` = BCLK/LRCLK, `PSR` prescaler — xem `rfs, bfs, frmclk` trong [i2s.c:60-65](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L60-L65).
- Các format: `SND_SOC_DAIFMT_I2S / LEFT_J / RIGHT_J`, đảo clock `NB_NF/NB_IF/...`, master/slave `CBM_CFM / CBS_CFS` — xử lý ở [i2s_set_fmt():623](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L623).

### 3. Bản đồ thanh ghi phần cứng

- [i2s-regs.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s-regs.h): `I2SCON` (0x0, điều khiển/trạng thái DMA & FIFO), `I2SMOD` (0x4, mode/format/clock source), `I2SFIC/I2SFICS` (FIFO control), `I2SPSR` (0xc, prescaler), `I2STXD/I2STXDS/I2SRXD` (data), `I2SAHB` + `I2SSTR0/I2SLVLnADDR` (internal DMA), `I2STDM` (TDM).
- Các bit đáng chú ý: `CON_TXDMA_ACTIVE/RXDMA_ACTIVE`, `CON_*FIFO_EMPTY/FULL`, `CON_TXSDMA_*` (secondary), `MOD_OPCLK_*`.

### 4. Cấu trúc dữ liệu của driver

- [`struct i2s_dai`](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L55): một interface DAI — `pri_dai/sec_dai` (primary vs secondary FIFO), `dma_playback/dma_capture/idma_playback`, `mode` (DAI_OPENED/DAI_MANAGER).
- [`struct samsung_i2s_priv`](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L87): tài nguyên controller — `addr` (SFR ioremap), `clk/op_clk`, `variant_regs`, `quirks`, `clk_table/clk_data` (clock provider), `slave_mode`, cache `suspend_i2smod/i2scon/i2spsr`.
- Helper trạng thái: `is_secondary()`, `tx_active()/rx_active()`, `any_active()`, `to_info()` [i2s.c:131-224](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L131-L224).

### 5. Đăng ký & probe

- `platform_driver` + `module_platform_driver()` [i2s.c:1691-1702](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1691-L1702), bảng OF `exynos_i2s_match` / `samsung_i2s_driver_ids`.
- [`samsung_i2s_probe():1379`](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1379): parse DT, `ioremap` SFR, `clk_get`, `i2s_alloc_dais()` [:1151](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1151), set DMA data, `devm_snd_soc_register_component` + DAIs, tạo secondary device, bật runtime PM.
- Per-DAI: [`samsung_i2s_dai_probe():1041`](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1041) / `samsung_i2s_dai_remove():1086`.

### 6. `snd_soc_dai_ops` — luồng runtime khi phát/thu

Thứ tự gọi từ ASoC ([ops table :1107](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1107)):

- `startup()` [:816] → `set_sysclk()` [:497] / `set_fmt()` [:623] / `set_clkdiv()` [:978]
- `hw_params()` [:719] (chốt rate/format/channels, chọn DMA data)
- `config_setup()` [:868] (tính RFS/BFS/PSR, ghi I2SMOD/I2SPSR)
- `trigger()` [:929] (START/STOP/RESUME → `i2s_txctrl()`/`i2s_rxctrl()` bật DMA + FIFO)
- `i2s_delay()` [:1007] (báo độ trễ FIFO cho ALSA), `shutdown()` [:843].

### 7. Quản lý clock

- Nguồn: core `clk`, `op_clk` (CDCLK), `rclk_srcrate`; chọn nguồn RCLK trong `i2s_set_sysclk()`.
- Prescaler `I2SPSR`, tỉ lệ RFS/BFS trong `config_setup()`.
- **Clock provider**: driver tự expose CDCLK/RCLK ra ngoài cho codec — `i2s_register_clock_provider()` [:1264](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1264), `i2s_unregister_clock_provider()` [:1257].

### 8. Đường dữ liệu (data path)

- **DMA ngoài** qua dmaengine: `snd_dmaengine_dai_dma_data` playback/capture, kết hợp platform driver [sound/soc/samsung/dma.c].
- **iDMA / internal DMA** (SRAM on-chip): `idma_playback`, `I2SAHB`/`I2SLVLnADDR` — xem [idma.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/idma.c) nếu có.
- FIFO: `i2s_txctrl()` [:391] / `i2s_rxctrl()` [:442] flush & bật kênh; TDM qua `I2STDM`.

### 9. Secondary / overlay DAI

- FIFO thứ hai cho phép trộn 2 luồng playback: `is_secondary()`, `i2s_create_secondary_device()` [:1338](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1338) / `i2s_delete_secondary_device()` [:1373], DAPM "Playback Mixer" [:1120-1124](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1120-L1124).

### 10. Power management

- `i2s_suspend()/i2s_resume()` [:1027-1032] lưu/khôi phục `I2SMOD/I2SCON/I2SPSR`.
- Runtime PM: `i2s_runtime_suspend()/resume()` [:1207-1222](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/samsung/i2s.c#L1207-L1222), `pm_runtime_get_sync/put` bao quanh truy cập thanh ghi.

### 11. Machine driver & Device Tree

- Binding: [samsung-i2s.yaml](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/Documentation/devicetree/bindings/sound/samsung-i2s.yaml), macro DT: [dt-bindings/sound/samsung-i2s.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/include/dt-bindings/sound/samsung-i2s.h).
- Glue machine phổ biến: [simple-card-utils.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/sound/soc/generic/simple-card-utils.c) (`asoc_simple_init_dai_link_params`) — cách `dai_link` nối CPU I2S với codec.

### 12. Đối chiếu với bản BootROM (bare-metal, tối giản)

Đọc để hiểu lõi phần cứng không vướng framework:

- [BootROM/.../sound/samsung-i2s.c](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/drivers/sound/samsung-i2s.c): chỉ TX — `i2s_tx_init()` [:297], `i2s_transfer_tx_data()` [:259], `i2s_set_fmt()` [:152], `i2s_set_lr_framesize()` [:30], `i2s_set_bitclk_framesize()` [:83], `i2s_fifo()` [:115].
- Header thanh ghi tương ứng: [Emu800/.../include/i2s.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/include/i2s.h), [i2s-regs.h](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/BootROM/BootROM/Emu800/Src/arch/arm/include/asm/arch-exynos/i2s-regs.h).

### 13. Hướng thực hành

- Đọc theo thứ tự: `i2s-regs.h` → `struct` → `probe` → `dai_ops` (`hw_params` → `config_setup` → `trigger`) → clock provider → PM.
- Dò runtime: bật `CONFIG_SND_DEBUG`, `dmesg`, `ftrace` trên `i2s_*`, xem `/sys/kernel/debug/asoc`.
- Bản BootROM là "phiên bản rút gọn" để nắm nhanh trước khi vào bản ASoC đầy đủ.