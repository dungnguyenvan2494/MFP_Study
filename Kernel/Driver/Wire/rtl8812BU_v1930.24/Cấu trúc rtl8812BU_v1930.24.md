# Cấu trúc `rtl8812BU_v1930.24/`

Đây là driver out-of-tree của Realtek (bản v1930.24) cho chip **RTL8822B** (USB). Driver này chung một codebase cho rất nhiều chip, nên cây source chứa nhiều thứ không được build. Với cấu hình hiện tại, **module sinh ra tên là `8812bu.ko`**. Lý do là `CONFIG_BT_COEXIST = n` ([Makefile:60](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/Makefile#L60)), nên [Makefile:848](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/Makefile#L848) chọn `MODULE_NAME = 8812bu`. Nếu bật BT coex thì tên sẽ là `88x2bu`.

## Các tầng (từ trên xuống dưới)

```
 cfg80211 / wext (kernel)
        │
 os_dep/linux/   ← lớp OS: gắn driver vào Linux (USB, cfg80211, netdev, proc)
        │
 core/           ← lớp lõi 802.11, độc lập OS: MLME, AP, P2P, recv/xmit, power, cmd
        │
 hal/            ← lớp HAL: điều khiển chip cụ thể
   ├─ rtl8822b/usb/   ← phần riêng của chip 8822B + giao tiếp USB
   ├─ halmac/         ← lớp MAC (nạp firmware, tx/rx desc, power seq)
   ├─ phydm/          ← PHY dynamic mechanism (DIG, CCK PD, antenna div, RF cal…)
   ├─ btc/, led/, efuse/
        │
   USB bus
```

## Chi tiết từng thư mục

| Thư mục                                                                                     | Số file | Vai trò                                                                                                                                                                                                                                                                                                                                             |
| ------------------------------------------------------------------------------------------- | ------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [core/](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/core)         | 73      | Logic 802.11 chung: `rtw_mlme*.c` (scan/join), `rtw_ap.c` (chế độ AP), `rtw_p2p.c`, `rtw_recv.c`/`rtw_xmit.c`, `rtw_pwrctrl.c`, `rtw_cmd.c` (hàng đợi lệnh), `rtw_ieee80211.c`, `rtw_btcoex*.c`, `rtw_mp.c` (manufacturing test). Có thư mục con `mesh/`, `monitor/` (radiotap), `efuse/`, và `hostap_src/` (mã crypto AES-CCM/GCM lấy từ hostapd). |
| [os_dep/](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/os_dep)     | 22      | Lớp keo OS. `linux/usb_intf.c` là điểm vào USB (`usb_driver`, probe/disconnect), `os_intfs.c` (netdev), `ioctl_cfg80211.c` (giao diện cfg80211), `ioctl_linux.c` (wext), `usb_ops_linux.c` (URB), `recv_linux.c`/`xmit_linux.c`, `wifi_regd.c` (regdomain), `rtw_proc.c` (/proc).                                                                   |
| [hal/](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/hal)           | 286     | Điều khiển phần cứng. File gốc `hal_com.c`, `hal_intf.c`, `hal_halmac.c`, `hal_hci/hal_usb.c`.                                                                                                                                                                                                                                                      |
| ↳ `hal/rtl8822b/`                                                                           |         | Riêng chip 8822B (`rtl8822b_halinit/mac/phy/ops/cmd.c`, `hal8822b_fw.c` chứa firmware nhúng). Thư mục `usb/` chứa `rtl8822bu_*` (io, xmit, recv, led, ops).                                                                                                                                                                                         |
| ↳ `hal/halmac/`                                                                             |         | HALMAC: API MAC chung họ 88xx (`halmac_88xx/`), header bit/reg cho 8822B/8822C/8812F/8814B/8821C/8197F.                                                                                                                                                                                                                                             |
| ↳ `hal/phydm/`                                                                              |         | PHY: DIG, CCK PD, antenna diversity, dynamic TX power, beamforming, DFS, CFO tracking, `halrf/` (RF calibration).                                                                                                                                                                                                                                   |
| ↳ `hal/btc/`, `led/`, `efuse/`                                                              |         | Bluetooth coexistence (wifionly), LED, mặt nạ efuse (`HalEfuseMask8822B_USB.c`).                                                                                                                                                                                                                                                                    |
| [include/](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/include)   | 265     | Toàn bộ header. Gồm `Hal88xxPhyCfg/Reg/PwrSeq.h` cho nhiều chip, `autoconf.h`, `drv_types.h`..., thư mục con `byteorder/`, `cmn_info/`, và `linux/wireless.h` (bản wireless extension riêng, file bạn đang mở).                                                                                                                                     |
| [platform/](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/platform) | 17      | Hook theo SoC (Allwinner, Amlogic, HiSilicon, Spreadtrum, ZTE…). Phần lớn là SDIO. Với S800 thì không có file riêng ở đây, nên cấu hình S800 chỉ nằm ở Makefile.                                                                                                                                                                                    |

## File ở gốc thư mục

- [Makefile](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/Makefile) (khoảng 2500 dòng): chọn chip và HCI, đặt cờ `-D...`, gom danh sách `.o`, rồi `obj-$(CONFIG_RTL8822BU) := $(MODULE_NAME).o` ([dòng 2449](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/Makefile#L2449)). Với `CONFIG_PLATFORM_ARM_S800` ([dòng 1381](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/Makefile#L1381)) nó bật `CONFIG_IOCTL_CFG80211`, `CONFIG_CONCURRENT_MODE`, `ARCH=arm64`, `CROSS_COMPILE=s800a-linux-`.
- [Kconfig](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/Kconfig): khai báo `RTL8822BU` (tristate, depends USB).
- [rtl8822b.mk](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/rtl8822b.mk): nối danh sách file HAL của 8822B và nhánh USB vào build.
- [halmac.mk](vscode-webview://1bvn9deljhs67bs6tdc4nttro6fasb6a3o9dsbsr0n55qjrijoej/halmac.mk): nối các file trong `hal/halmac/`.
- `hal/phydm/phydm.mk`: tương tự cho PHYDM.
- `runwpa`, `wlan0dhcp`, `ifcfg-wlan0`, `clean`: script/cấu hình tiện ích của Realtek (chạy `wpa_supplicant -D wext`, xin DHCP, dọn build). Không liên quan đến build kernel.

## Lưu ý quan trọng

- Hầu hết file trong `include/` (ví dụ `Hal8188E*`, `Hal8723*`), cùng `platform/*sdio*` và các nhánh `MODULE_NAME` khác trong Makefile, là mã chip hoặc bus khác nên **không được biên dịch**. Cấu hình hiện tại chỉ dùng 8822B + USB.
- Các file cấu hình liên quan nằm đúng ở Makefile gốc của driver, vì vậy muốn đổi hành vi (BT coex, concurrent mode, chuẩn kênh...) thì sửa các biến `CONFIG_*` và `EXTRA_CFLAGS` ở đó.

Tôi chưa mở sâu nội dung từng file `.c`. Muốn đi sâu một luồng cụ thể (ví dụ `probe` USB → nạp firmware → đăng ký cfg80211, hoặc đường TX/RX), tôi có thể trace theo source thật.