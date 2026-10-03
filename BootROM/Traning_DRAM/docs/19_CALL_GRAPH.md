# 19 — Call Graph thật: entry → return

Đọc `00`-`18` trước. Đây thay thế hoàn toàn sơ đồ giả định ở Phần 29 (outline gốc) bằng tên hàm/file/dòng thật, ghép từ toàn bộ tài liệu 04-17.

# Call graph đầy đủ

```
BootROM
  |
  v
ble_main(bootrom_flags)                                    [ble_main.c:52]
  |  - xoá p_sram (0xE5 × 2048)
  |  - get_it(), console_init(), plat_delay_timer_init()
  v
lpddr4_config()                                             [mv_lpddr4_apn806.c:8751]
  |  - warmboot = syncronize_boot_get(BOOT_SYNC_DDR_WARMBOOT)   ← set bởi mc_configure_dram() (LPPP, tài liệu 15)
  |  - đọc GPIOa/GPIOh(Timer2)/GPIOi                              → tài liệu 13
  |  - swizzle_setting()                                         → tài liệu 13
  |  - calculate_checksum_qspi()/judge_qspi_param() [+ dự phòng]  → tài liệu 13
  |  - start Timer3, display_measure_time(TRN_A0)                → tài liệu 14
  |
  +-- [cold, warmboot==0] for i in 0..6:                         ← TẦNG NGOÀI (tài liệu 14/17)
  |         lpddr4_training_Error = 0
  |         lpddr4_dynamic_config(config_type, warmboot=0)  ─┐
  |         if error: p_sram[0]=1, p_sram[1]=i+1               │
  |     if OK: p_sram[2]=0x77                                   │
  |                                                             │
  +-- [warm, warmboot==1] lpddr4_dynamic_config(config_type,1) ─┤  (gọi 1 lần, KHÔNG có tầng ngoài)
  |                                                             │
  |  display_measure_time(TRN_N0) ; //read_ddr_osc() (tắt) ; TRN_N1
  |  syncronize_boot_set(BOOT_SYNC_DDR_READY)
  v
return MV_OK  →  U-Boot đọc pmem (0x200000000), ghi vào QSPI (qspi_winbond.c, ❓chưa đọc)


┌─────────────────────────────────────────────────────────────────────────┐
│ lpddr4_dynamic_config(staticOrDynamic, warmboot)          [~7866]        │
│                                                                           │
│  đọc chip_info_reg (FORCE_QSPI_MODE, FORCE_CA_TRAIN_IGNORE,              │
│                      DQS_GATE_PREAMBLE_EN, KM_*_SHMOO, TRAIN_RETRY_NUM)  │
│  memory_capacity_setting_qspi() / memory_capacity_setting()  → tài liệu 13│
│  reset PHY (sdram_cfg), pad_cal()                            → TRN_B0/B1 (tài liệu 04)│
│  MCConfig_ap806_dual_chan_..._dpi()                          → TRN_B2   │
│                                                                           │
│  retry:                                                                  │
│  for (train_err_cnt = 0; train_err_cnt < retry_num; train_err_cnt++) {  │  ← TẦNG TRONG (tài liệu 17)
│                                                                           │
│    lpddr4_ca_training(mc_reg)                    → TRN_C0-C5 (tài liệu 05)│
│      enter_cbt() / exit_cbt()                                            │
│                                                                           │
│    for ch,cs: lpddr4_write_leveling(mc_reg,phy_reg,ch,cs)  → TRN_D0/D1 (06)│
│                                                                           │
│    lpddr4_dqs_gate_training_preamble() | _training()  → TRN_E0 (07)     │
│                                                                           │
│    lpddr4_read_centering(FIFO, ADD_PBS)          → TRN_GX  (08)         │
│    lpddr4_read_find_deskew_start()               → TRN_H0  (09)         │
│    lpddr4_read_dm_deskew()                       → TRN_H1  (09)         │
│    lpddr4_read_deskew()                          → TRN_H2  (09) [❓ thân hàm]│
│    lpddr4_read_find_deskew_end()                 → TRN_H3  (09) [❓ thân hàm]│
│    lpddr4_read_deskew_center_align()             → TRN_H4  (09)         │
│    lpddr4_read_centering(FIFO, NO_PBS)           → TRN_H5  (08)         │
│                                                                           │
│    lpddr4_write_centering(FIFO, skip=1)          → TRN_I0/I1→IX (10)    │
│    lpddr4_write_find_deskew_start()              → TRN_J0  (11)         │
│    lpddr4_write_dm_deskew()                      → (giữa J0-J1) (11)    │
│    lpddr4_write_deskew()                         → TRN_J2  (11) [❓ thân hàm]│
│    lpddr4_write_centering(FIFO, skip=1)          → TRN_KX  (10)         │
│    backup_train_area()                                        (10/12)   │
│                                                                           │
│    [nếu !warmboot && !dma_test_skip_en]                                  │
│      common_memfill()                            → TRN_L0  (12)         │
│      lpddr4_read_centering(DMA, ADD_PBS)         → TRN_L1  (08)         │
│      lpddr4_read_centering(DMA, NO_PBS)          → TRN_L2  (08)         │
│      lpddr4_write_centering(DMA)                 → TRN_M0  (10)         │
│                                                                           │
│    if train_err_flg: [nếu chưa lần cuối] restore_train_area()+return_100mhz()│
│    else: train_err_cnt = retry_num+1   ← thoát sớm                      │
│  }                                                                        │
│  if km_cal_window_flag: calculate_window_margin()             (03)      │
│                                                                           │
│  address_decode_setting_qspi() | address_decode_setting()               │
│                                                                           │
│  [!warmboot]: restore_train_area(); lpddr4_backup_train_result() ──┐    │
│  [warmboot]:  warm_train_flag = check_warm_training()               │   │
│               if warm_train_flag: restore_train_area(); return_100mhz(); goto retry;│
│               else: enable_dbi_dm(); resetPHY(); restore_train_area()   │
└───────────────────────────────────────────────────────────────────┼────┘
                                                                       │
                                                                       v
                                                    lpddr4_backup_train_result()   [tài liệu 16]
                                                      gTrainRegReadTbl[576] → pmem (DRAM) + pmem2 (0xE8200100)
                                                      so sánh pmem/pmem2 → lpddr4_training_Error
```

# Caller/Callee — các hàm PHY-access nền (dùng ở gần như MỌI khối trên)

```
nova_ddrphy_write(reg_num, CnD, pup_num, data)   [tài liệu 02] ← gọi bởi TẤT CẢ hàm training 04-11
nova_ddrphy_read(reg_num, CnD, pup_num)          [tài liệu 02]
resetPHY(mask)                                    ← reset FIFO/counter, gọi sau mỗi lần đổi delay
rdModWr(addr, value, mask, verify)                ← ghi MMIO trực tiếp (MC6/DDR_PHY struct thường)
send_mrx(rw, mr_num, ch, cs)                      ← gửi lệnh Mode-Register-Write/Read tới DRAM (MR13, MR3, MR1, MR14...)
change_clk_freq(freq) / wait_for_dll_lock()        ← đổi tần số + chờ khoá pha, dùng ở CA training, WL, retry
```

# Nhánh Sleep/Resume (phía LPPP, tách khỏi cây trên — tài liệu 15)

```
mc_configure_dram(warm)  → set BOOT_SYNC_DDR_WARMBOOT   [caller thật ❓UNKNOWN, ngoài phạm vi file đã đọc]
mc_save_state()    → [debug: ddr_verify_crc(false)] → mc_drain() → mc_enter_pd() → mc_enter_sr()
mc_restore_state() → [debug: ddr_verify_crc(true)]  → syncronize_boot_set(BOOT_SYNC_DDR_CONT_BOOT)
```

# Điểm chưa trace hết (❓UNKNOWN, thành thật ghi nhận)

| Hàm | Vị trí | Trạng thái |
|---|---|---|
| `get_it()` | `ble_main.c` | Chưa trace |
| `lpddr4_read_deskew()`, `lpddr4_read_find_deskew_end()` | `mv_lpddr4_apn806.c:3107-3559` | Chỉ đọc header |
| `lpddr4_write_deskew()` | `:5491+` | Chỉ đọc phần đầu |
| `lpddr4_write_com()` | `:11505+` | Chỉ có khai báo, quan hệ với `lpddr4_write_centering()` chưa xác nhận |
| `check_warm_training()` | `:13689+` | Đọc phần đầu (setup + swizzle pattern), chưa đọc điều kiện PASS/FAIL cuối |
| `qspi_winbond.c` (U-Boot) | `BootROM\...\drivers\spi\` | Chưa đọc |
| `MCConfig_ap806_dual_chan_..._dpi()` | ❓ chưa định vị dòng chính xác | Biết vai trò qua tên gọi + vị trí (giữa TRN_B1/B2) |
| Caller của `mc_configure_dram(warm)` | Ngoài `mv_lpddr4_sleep.c` | Chưa trace — ai quyết định cold/warm |

**Tiếp theo:** `20_SPEC_VS_SOURCE.md` — tổng hợp mọi ✅MATCH/⚠️MISMATCH/❓UNKNOWN đã gặp thành 1 bảng duy nhất.
