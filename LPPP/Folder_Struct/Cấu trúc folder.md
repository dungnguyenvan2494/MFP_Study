Đây là **firmware SDK "Proxy Firmware on R4 LPP" của Marvell** — chạy trên chip 88PA6220 (mã Quartz), lõi ARM Cortex-R4, dùng FreeRTOS + lwIPv6. Nhiệm vụ: khi Linux host ngủ (low power), firmware này tiếp quản Ethernet, trả lời thay các gói mạng (ARP/mDNS/SNMP...) và chỉ đánh thức host khi thật cần.

Kiến trúc phân tầng từ dưới lên: **Hal → Os → Netstack → Sys → Application**

|Thư mục|Ý nghĩa|
|---|---|
|**Hal/**|Tầng phần cứng. `quartz/` (~1000 file) là chip chính: cpu, clocks, i2c, uart, timer, interrupt (GIC/SIC), MHU, cdma, linker script. `6220/` là biến thể board, `inc/` là các API trừu tượng (`hal_api_lan.h`, `hal_api_wol.h`...)|
|**Os/**|RTOS. `freertos/` gồm tasks.c, queue.c, timers.c + `portable/` cho Cortex-R4|
|**Netstack/**|Stack mạng. `lwipv6/` là lwIP hỗ trợ IPv4/IPv6; `none/` để build không cần stack|
|**Sys/**|Middleware nối HAL với ứng dụng: `sdk/` (API chuẩn `mvbase_api_*`: socket, network, pattern, offload, ipsec), `filter/` (bộ lọc gói 2 tầng), `fifo/` + `hostcmd/` (giao tiếp với driver Linux qua SRAM FIFO), `wake_PCIE`, `wake_USBD`, `memory`, `thread`, `timer`, `jtag`, `debug`|
|**Application/**|Các proxy nghiệp vụ — phần "thay mặt host trả lời": `llmnr_proxy`, `nbns_proxy`, `simple_mdns_proxy`, `snmp_proxy`, `wake_service_proxy`, `hostcmd` (xử lý IOCTL từ Linux), `thread` (task chính + cấu hình filter mặc định), `tools/` (script Perl cấu hình runtime)|
|**ApplicationMrvl/**|Code của Marvell dùng để test/bring-up: `Test/` (app_cmdproc, app_scpi, app_gpio, app_MHU), `init/`, `util/` (script Perl: sniffer, sleep_test, memory_dump_analyzer)|
|**ApplicationKM/**|Lớp tuỳ biến của khách hàng (comment tiếng Nhật, Konica Minolta). Logic riêng: `LPPP_Task` (điều khiển task), `LPPP_Matrix` (xử lý kịch bản/scenario), `LPPP_Flash`, `LPPP_Gpio`, `LPPP_I2C`, `LPPP_SCPI` (giao thức với máy chủ), `LPPP_QoS`, `LPPP_McChangeFreq` (đổi tần số CPU tiết kiệm điện)|
|**lib/**, **misc/**|Thư viện binary `libpmu_r4.a` và kiểu dữ liệu chung (`mv_types.h`, `mv_errors.h`)|

**Build:** `./build_lppp.sh <cấu hình>` với các board như `es`, `emu`, `egl`, `eglz`, `dnbmlk`, `hemlk`, `mssb`... Hệ thống Makefile phân mảnh (`Makefile-cfg` chọn ARCH/ASIC, `Makefile-rules`, `Makefile-os`, `Makefile-lib`), mỗi module có `Makefile` riêng. Kết quả là `mvri.bin` trong `output/<target>/`.