# Phân tích thanh ghi — driver `rtl8812BU_v1930.24` (chip RTL8822B, giao tiếp USB)

Driver: `Kernel/K-S800/Src/drivers/net/wireless/realtek/rtl8812BU_v1930.24`. Build: kernel 5.10.241, `Eagle_defconfig`, module `8812bu`. Yêu cầu gốc: [00_REQUEST.md](00_REQUEST.md). Tiến độ và ghi chú công cụ: [PROGRESS.md](PROGRESS.md).

Mọi kết luận đều có nhãn nguồn:

| Nhãn | Ý nghĩa |
|---|---|
| [SOURCE] | đọc trực tiếp từ source |
| [SOURCE-SEQ] | trình tự đọc từ source |
| [TOOL] | do script trong `tools/` tính |
| [tính tay] | tự tính |
| [INFERENCE] | suy luận |
| [KIỂM TRA TRÊN BOARD] | cần đo trên phần cứng |

## Ánh xạ 12 phần của định dạng kết quả (00_REQUEST §10)

| # | Phần | File |
|---|---|---|
| 1 | Kiến trúc và cấu hình build | [01_build_config.md](01_build_config.md) |
| 2 | Các không gian địa chỉ (MAC/BB/RF/USB-local/USB3-PHY/LTE-coex/packet buffer/CAM/eFuse…) và các đối tượng dễ nhầm với thanh ghi | [02_address_spaces.md](02_address_spaces.md) |
| 3 | Kiến trúc truy cập (`rtw_read*` → vendor request, BB RMW, RF LSSI/cửa sổ đọc, HALMAC) — kèm phần 7 của yêu cầu | [03_access_architecture.md](03_access_architecture.md) |
| 4 | Danh mục tổng hợp (2096 địa chỉ) | [04_catalog_README.md](04_catalog_README.md) → [MAC](04a_catalog_mac.md), [BB](04b_catalog_bb.md), [RF](04c_catalog_rf.md), [khác](04d_catalog_other.md) |
| 5–6 | Phân tích theo nhóm chức năng, có bit/field | [05_groups/](05_groups/) — xem bảng bên dưới |
| 7 | Sequence (probe, ifup, đổi kênh, watchdog, TX/RX, PS, lỗi) | [07_sequences.md](07_sequences.md) |
| 8 | Truy cập bằng địa chỉ số | [08_direct_numeric.md](08_direct_numeric.md) |
| 9 | Alias, định nghĩa không dùng, **25 điểm bất thường** | [09_aliases_unused.md](09_aliases_unused.md) |
| 10 | Chỉ mục theo địa chỉ | [10_index_by_address.md](10_index_by_address.md) |
| 11 | Chỉ mục theo tên | [11_index_by_name.md](11_index_by_name.md) |
| 12 | Bảng hoàn thành và vấn đề chưa xác minh | [12_completion.md](12_completion.md) |

## Tài liệu nhóm (05_groups)

| Nhóm 00_REQUEST §4.2 | File |
|---|---|
| 1 System, clock, reset, power sequence | [01_system_power_clock.md](05_groups/01_system_power_clock.md) |
| 2 MAC/WMAC, bộ lọc RX | [02_wmac_rx_filter.md](05_groups/02_wmac_rx_filter.md) |
| 3 Interrupt | [03_interrupt.md](05_groups/03_interrupt.md) |
| 4–5 DMA, FIFO, hàng đợi TX/RX | [04_dma_queue_trx.md](05_groups/04_dma_queue_trx.md) |
| 6 EDCA, beacon, TSF | [06_edca_beacon_tsf.md](05_groups/06_edca_beacon_tsf.md) |
| 7 Security/CAM | [07_security.md](05_groups/07_security.md) |
| 8 Firmware, mailbox, H2C/C2H, LTE-coex | [08_firmware.md](05_groups/08_firmware.md) |
| 9 USB | [09_usb.md](05_groups/09_usb.md) |
| 10 Power management (LPS/IPS/WoW) | [10_power_mgmt.md](05_groups/10_power_mgmt.md) |
| 11 GPIO/LED | [11_gpio_led.md](05_groups/11_gpio_led.md) |
| 12 eFuse | [12_efuse.md](05_groups/12_efuse.md) |
| 13 BB/PHY | [13_bb_phy.md](05_groups/13_bb_phy.md) |
| 14 RF | [14_rf.md](05_groups/14_rf.md) |
| 15 Calibration, TX power | [15_calibration_txpower.md](05_groups/15_calibration_txpower.md) |
| 16 MP/Debug | [16_mp_debug.md](05_groups/16_mp_debug.md) |
| 17 Khác (BF/MU, BT-coex wifi-only, LA, phydm động, MAC lẻ) | [17_misc_bf_btc.md](05_groups/17_misc_bf_btc.md) |

Phụ lục (bằng chứng từng truy cập, sinh tự động trừ A11 phần giải mã và A13):

- [A01–A10](05_groups/appendix/README.md): truy cập theo dải địa chỉ
- [A11](05_groups/appendix/A11_mac_reg_table.md): bảng MAC_REG đã giải mã
- [A12](05_groups/appendix/A12_uncovered_live.md): thanh ghi live chưa được nhắc trong các nhóm
- [A13](05_groups/appendix/A13_status_R_review.md): rà soát trạng thái R

## Công cụ (`tools/`, chạy bằng `py -I`, không cần biên dịch)

| Script | Việc làm |
|---|---|
| `mk_eval.py` | đánh giá Makefile |
| `cpp_mask.py` | xác định dòng nào active theo `#if` |
| `inventory.py` + `guard.py` | macro, truy cập, guard IC/intf/cấu hình (gồm thân `if` không có ngoặc), đồ thị gọi |
| `tables.py` | bảng MAC_REG/PHY_REG/AGC/RadioA/B/power-seq |
| `report.py` / `report_extra.py` / `appendix.py` / `mactable.py` | sinh 04, 08–11, A01–A11 |
| `regctx.py`, `bitname.py`, `r_review.py`, `completion.py` | tra cứu, giải mã bit, rà R, thống kê lượt 6 |
| `txff_sim.py`, `chbw_sim.py`, `rftab_sim.py`, `txagc_sim.py`, `pwrtrk_sim.py` | mô phỏng: FIFO, đổi kênh/BW, walker bảng theo rfe/cut, dword TXAGC, power tracking |

Lệnh chạy lại toàn bộ: xem PROGRESS.md, mục "Công cụ thêm".
