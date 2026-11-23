# dpm_suspend

**`dpm_suspend()`** là hàm của PM core (`drivers/base/power/main.c`) dùng để ​**gọi callback `.suspend` của tất cả device trong hệ thống, theo đúng thứ tự**​, khi máy đi vào system suspend (ví dụ DeepSleep). Phần dưới là kiến thức chung về Linux kernel, không lấy từ tài liệu project.

## Nó nằm ở đâu trong luồng suspend

System suspend không gọi suspend một lần là xong. PM core chia ra nhiều pha, và mỗi pha duyệt qua ​**toàn bộ device**​:

```
echo mem > /sys/power/state

        │

        ▼

suspend_devices_and_enter()

        │

        ▼

dpm_suspend_start()

   ├── dpm_prepare()        → gọi .prepare   cho mọi device

   └── dpm_suspend()        → gọi .suspend   cho mọi device   ← đây

        │

        ▼

dpm_suspend_end()

   ├── dpm_suspend_late()   → gọi .suspend_late

   └── dpm_suspend_noirq()  → gọi .suspend_noirq (đã tắt IRQ device)

        │

        ▼

CPU/SoC vào trạng thái ngủ
```

Resume đi theo chiều ngược lại: `dpm_resume_noirq` → `dpm_resume_early` → **`dpm_resume`** (gọi `.resume`) → `dpm_complete`.

## `dpm_suspend()` làm gì cụ thể

* **Duyệt danh sách device theo chiều ngược.** Mọi device đăng ký vào `dpm_list` theo thứ tự được thêm vào, thường là cha trước con sau. `dpm_suspend()` lấy từ **cuối danh sách** (`dpm_prepared_list`) lên đầu, nên ​**con suspend trước, cha suspend sau**​.
* **Gọi callback suspend của từng device** qua `device_suspend()` / `__device_suspend()`. Callback được chọn theo thứ tự ưu tiên: PM domain → device type → class → **bus** → driver. Với thiết bị USB, callback là `usb_suspend()` (bus USB). Với xHCI là `xhci_plat_suspend()`, với PHY là `mv_usb3_phy_suspend()`.
* **Hỗ trợ async.** Device bật `async_suspend` có thể suspend song song. Trước khi suspend một device, PM core gọi `dpm_wait_for_subordinate()` để chờ **các con và các consumer (device\_link)** của nó suspend xong.
* **Xử lý lỗi.** Nếu một device trả lỗi, `dpm_suspend()` dừng lại và báo lỗi. Sau đó PM core gọi resume cho các device đã kịp suspend, và hệ thống không vào ngủ.
* ​**Chuyển device đã suspend sang`dpm_suspended_list`**​, để `dpm_resume()` sau này duyệt lại theo chiều xuôi.

