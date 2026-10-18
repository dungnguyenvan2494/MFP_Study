# 01 — Tổng quan kiến trúc và cấu hình build

> Nhãn: **[SOURCE]** = đã đọc đúng dòng code được dẫn; **[TOOL]** = kết quả của script trong `tools/` (tái chạy được); **[INFERENCE]** = suy luận, chưa có bằng chứng trực tiếp; **[KIỂM TRA TRÊN BOARD]** = chỉ biết được khi chạy trên máy thật.
> Đường dẫn tương đối tính từ `Kernel/K-S800/Src/drivers/net/wireless/realtek/rtl8812BU_v1930.24/` (gọi là `$D`).

## 1.1 Driver này build cho chip nào?

| Bằng chứng | Giá trị | Nguồn |
|---|---|---|
| Chọn IC | `CONFIG_RTL8822B = y`, mọi IC khác `= n` (`CONFIG_RTL8812A = n`, `CONFIG_RTL8821C = n`, …) | `Makefile:32-47` [SOURCE] |
| Giao tiếp | `CONFIG_USB_HCI = y`, `PCI/SDIO/GSPI = n` | `Makefile:49-52` [SOURCE] |
| Tên module | `MODULE_NAME = 8812bu` khi `CONFIG_BT_COEXIST = n`, ngược lại `88x2bu` | `Makefile:848-851` [SOURCE] |
| Kernel | `CONFIG_RTL8822BU=m` trong 14/18 defconfig KM (dùng `Eagle_defconfig` làm chuẩn) | `arch/arm64/configs/*_defconfig:1943` [SOURCE] |
| Thư mục kernel | `obj-$(CONFIG_RTL8822BU) += rtl8812BU_v1930.24/` (2 driver 88x2bu khác bị comment) | `drivers/net/wireless/realtek/Makefile:11-13` [SOURCE] |
| autoconf | `RTL871X_MODULE_NAME "8812BU"`, `DRV_NAME "rtl8812bu"`, `CONFIG_USB_HCI 1` | `include/autoconf.h:64-71` [SOURCE] |

**Về sự "không thống nhất" giữa tên thư mục (8812BU) và chip (8822B):** không phải lỗi. RTL8812BU là biến thể 2T2R của họ RTL8822B **không có Bluetooth**; driver Realtek dùng chung HAL `rtl8822b` và chỉ đổi tên module khi tắt BT-coex (`Makefile:848-851`). Mọi thanh ghi chip-specific do vậy là của **8822B** (`*_8822B`, `halmac_8822b/`, `phydm/rtl8822b/`, `halrf/rtl8822b/`). [SOURCE + INFERENCE về quan hệ sản phẩm]

## 1.2 Cách xác định "nhánh code đang hoạt động"

Không có `make`/`gcc` trên máy phân tích, nên dùng 2 script (tái chạy được, xem `PROGRESS.md`):

1. `tools/mk_eval.py` — đánh giá Makefile → **171 file .c** (TU) + 50 cờ `-D…` [TOOL].
2. `tools/cpp_mask.py` — tiền xử lý *chỉ các directive* (`#define/#if/#include…`) cho từng TU, đánh dấu dòng nào nằm trong nhánh được biên dịch → `work/mask.json`: 396 file được chạm tới, 32 056 macro active, 0 lỗi đánh giá [TOOL].
   - Giới hạn: header kernel (`<linux/…>`, 62 file) không được parse → macro chỉ có trong header kernel coi như *undefined* (trừ các macro seed sẵn: `LINUX_VERSION_CODE`=5.10.241, `KERNEL_VERSION`, `__KERNEL__`, các `CONFIG_*` từ defconfig).
3. `tools/guard.py` + `tools/inventory.py` — **lớp thứ 2: điều kiện lúc chạy.** Code dùng chung (phydm, halmac_88xx, hal_com) chọn chip **lúc chạy** bằng `if (dm->support_ic_type & ODM_RTL8xxx)`, `IS_HARDWARE_TYPE_xxx(adapter)`, `adapter->chip_id == HALMAC_CHIP_ID_xxx`, `adapter->intf == HALMAC_INTERFACE_xxx`. Mặt nạ `#if` không thấy được điều này. `guard.py` thay các biểu thức đó bằng giá trị thật của RTL8822B-USB (lấy từ enum trong source: `ODM_RTL8822B = BIT(7)` `phydm_pre_define.h:419`, `HALMAC_CHIP_ID_8822B = 0` `halmac_type.h:597`, …) rồi tính logic 3 trị (đúng/sai/không rõ) [TOOL].
4. Đồ thị gọi hàm: hàm mà **mọi** nơi gọi đều nằm trong nhánh chết → hàm chết (lặp tới điểm bất động) [TOOL].

Kiểm chứng tay 3 kết luận "chết" của công cụ (đều đúng):

| Vị trí | Điều kiện | Vì sao chết với 8822B |
|---|---|---|
| `hal/phydm/phydm_phystatus.c:3170-3173` | `else if (support_ic_type & ODM_IC_11AC_SERIES)` | nhánh trước `PHYSTS_2ND_TYPE_IC` (`phydm_pre_define.h:481`) **đã chứa** `ODM_RTL8822B` → 8822B luôn rẽ vào nhánh 2nd-type |
| `hal/hal_com.c:8358-8360` | `IS_HARDWARE_TYPE_8188E \|\| IS_HARDWARE_TYPE_8812` | không gồm 8822BU |
| `hal/hal_com.c:12224-12225` | `IS_HARDWARE_TYPE_8723B(adapter)` | không gồm 8822BU |

## 1.3 Các cờ cấu hình ảnh hưởng đến thanh ghi

| Cờ (Makefile → -D) | Ý nghĩa đối với thanh ghi | Nguồn |
|---|---|---|
| `CONFIG_MP_INCLUDED` | Biên dịch code MP (sản xuất/test): `core/rtw_mp.c`, `hal/hal_mp.c`, `os_dep/linux/ioctl_mp.c` — đọc/ghi MAC/BB/RF tuỳ ý qua ioctl | `Makefile:54,249` |
| `CONFIG_POWER_SAVING`, `CONFIG_USB_AUTOSUSPEND`, `CONFIG_WOWLAN`, `CONFIG_GPIO_WAKEUP` | Power-sequence, LPS, WoWLAN, GPIO wake | `Makefile:55-130` |
| `CONFIG_CONCURRENT_MODE`, `CONFIG_MCC_MODE` | 2 port (port0/port1) → bộ thanh ghi BSSID/beacon/TSF thứ 2 | `Makefile:81-82` |
| `CONFIG_EFUSE_CONFIG_FILE` | eFuse có thể nạp từ file `/system/etc/wifi/wifi_efuse_8812bu.map` thay vì đọc chip | `build.json` cflags |
| `CONFIG_LOAD_PHY_PARA_FROM_FILE` + `rtw_load_phy_file = BIT0\|BIT2\|BIT6` | MAC_REG, PHY_REG_PG, TXPWR_LMT được thử nạp từ `/lib/firmware/*.txt` **trước**, không có file mới dùng mảng trong source; BB/AGC/RF luôn dùng mảng | `os_intfs.c:837`, `hal_com_phycfg.h:380-386`, `rtl8822b_phy.c:118-129` |
| `CONFIG_BT_COEXIST = n` | Không có code BT-coex 2 ant/1 ant; chỉ `halbtc8822bwifionly.c` | `rtl8822b.mk` |
| `DBG_IO` (comment) | `rtw_read8` = `_rtw_read8`, không có lớp sniff | `autoconf.h:404`, `rtw_io.h:459` |
| `HALMAC_DBG_MONITOR_IO` = 0 | `HALMAC_REG_R8` gọi thẳng `api->halmac_reg_read_8` | `halmac_type.h:228-266` |
| `RTW_IQK_FW_OFFLOAD` (mặc định cho 8822B) | `rtw_iqk_fw_offload = 1` → IQK do **firmware** làm, chuỗi IQK bằng thanh ghi của driver chỉ chạy khi đặt module param = 0 | `hal_ic_cfg.h:296-298`, `os_intfs.c:940-945`, `halrf_iqk_8822b.c:1856-1865` |
| `CONFIG_FW_OFFLOAD_PARAM_INIT` (không bật) | `PHYDM_PHY_PARAM_OFFLOAD` không bao giờ được bật → bảng MAC/BB/RF ghi bằng USB vendor request, không qua H2C | `hal_dm.c:198-210`, `phydm.c:1520-1527` |

## 1.4 Danh sách file theo vai trò (chỉ file thuộc build)

| Lớp | File chính | Vai trò với thanh ghi |
|---|---|---|
| OS / USB | `os_dep/linux/usb_intf.c`, `usb_ops_linux.c` | probe; `usbctrl_vendorreq()` — điểm duy nhất phát USB control transfer cho thanh ghi |
| I/O lõi | `core/rtw_io.c`, `include/rtw_io.h`, `hal/hal_hci/hal_usb.c` | `rtw_read/write8/16/32` → `io_ops` → `usb_read/write*` |
| HAL chung | `hal/hal_intf.c`, `hal_com.c`, `hal_com_phycfg.c`, `hal_dm.c` | HW_VAR → thanh ghi, BB/RF wrapper, tải bảng từ file |
| HAL 8822B | `hal/rtl8822b/*.c`, `hal/rtl8822b/usb/*.c` | init, ops, phy, xmit/recv, hwreg |
| HALMAC | `hal/hal_halmac.c`, `hal/halmac/halmac_88xx/*.c`, `halmac_88xx/halmac_8822b/*.c` | power seq, init MAC/TRX, FW download, H2C/C2H, efuse, GPIO, USB cfg |
| PHYDM | `hal/phydm/*.c`, `hal/phydm/rtl8822b/*.c` | BB: DIG, CCA, FA, CCX, rate, antenna, bảng PHY/AGC |
| HALRF | `hal/phydm/halrf/*.c`, `halrf/rtl8822b/*.c` | RF: IQK, power tracking, kfree, bảng RadioA/B |
| Header định nghĩa | `hal/halmac/halmac_reg_8822b.h`, `halmac_bit_8822b.h`, `halmac_reg2.h`, `halmac_bit2.h`, `hal/phydm/phydm_regtable.h`, `phydm_reg.h`, `phydm_regdefine11ac.h/11n.h`, `include/hal_phy_reg.h` | xem 02 |

**Không thuộc build** (dù có trong repository): `hal/rtl8812a/`, `include/rtl8812a_*.h`, `Hal8812PhyReg.h` (chỉ được include qua `rtl8812a_hal.h`), mọi `halmac_88xx/halmac_8821c|8822c|…`, `*_pcie_*`/`*_sdio_*`. Khối `REG_*` của `include/hal_com_reg.h:70-739` nằm trong `#ifndef RTW_HALMAC` nên **bị loại** với 8822B — tên MAC thật lấy từ halmac. [SOURCE: mask]

## 1.5 Sửa đổi của vendor (KM) liên quan đến thanh ghi

| Vị trí | Nội dung | Ghi chú |
|---|---|---|
| `core/rtw_io.c:92-93`, `:116-224` | `DEFINE_MUTEX(halmac_reg_mutex)`; mọi `_rtw_read8/16/32`, `_rtw_write8/16/32`, `_rtw_writeN` bọc `mutex_lock/unlock`. Comment (Shift-JIS): *"2019/04/03 AR.1072290 — link WLAN bị đứt → thêm loại trừ cho read/write"* | [SOURCE]. Hệ quả ở 03 §3.6 |
| `os_dep/linux/usb_ops_linux.c:63-64` | `extern set_WlanReset_ErrorNotify()` — *"2020/10/28 OP_BTS-28747 xử lý sự cố mất kết nối WLAN No.1"*; gọi ở `core/rtw_mlme.c:2418` | [SOURCE] |
