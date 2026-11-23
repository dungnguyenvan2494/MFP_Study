# Khoá học USB Resume trên Linux Kernel

Baseline: **Linux 5.10.258** (`Kernel/Src`), board Marvell Armada Quartz EMU 800 (arm64). Mọi hàm, thanh ghi, bit và log đều đã đối chiếu với source. Log chỉ là ví dụ minh họa thì có ghi rõ.

Đặc điểm Resume của Quartz (đã kiểm tra trong source và log thật):
- **Deep sleep = Linux Suspend-to-RAM** (`pm_suspend`). **SLEEP1 không phải**.
- xHCI có `xhci_reset_on_resume` → mỗi system resume đều **khởi tạo lại controller** (log `root hub lost power or was reset`) → thiết bị **reset-resume**.
- PHY USB3 của KM: lúc suspend giữ `/RST_SLP2` (reset hub) và tắt đầu ra USB2; lúc resume trì hoãn 300 ms + 100 ms.
- `usb_resume` của KM chạy **bất đồng bộ** (work sau 500 ms), resume **theo ngăn xếp**, và **chờ** cờ của PHY và xHCI.
- Runtime PM của thiết bị USB **bị tắt** mặc định (`usb_disable_autosuspend` trong `usb_probe_device`).
- Bản build không bật `DYNAMIC_DEBUG`, `FTRACE`, `USB_MON`: phần lớn log resume là debug, không thấy được.

| Chương | Nội dung | File |
|---|---|---|
| 1 | Hiểu USB Suspend và Resume (ví dụ bàn phím) | [chapter-01-suspend-resume-basics.md](chapter-01-suspend-resume-basics.md) |
| 2 | Kiến trúc quản lý nguồn USB | [chapter-02-pm-architecture.md](chapter-02-pm-architecture.md) |
| 3 | Các loại Suspend/Resume (thiết bị, cổng, root hub, controller, hệ thống, runtime) | [chapter-03-suspend-resume-types.md](chapter-03-suspend-resume-types.md) |
| 4 | Resume ở mức phần cứng (J/K/SE0, LFPS, U0–U3, LTSSM) | [chapter-04-hardware-signaling.md](chapter-04-hardware-signaling.md) |
| 5 | Kiến trúc Resume trong Linux | [chapter-05-linux-resume-architecture.md](chapter-05-linux-resume-architecture.md) |
| 6 | Luồng thực thi hoàn chỉnh (4 kịch bản) | [chapter-06-resume-execution-flows.md](chapter-06-resume-execution-flows.md) |
| 7 | Resume trong EHCI | [chapter-07-ehci-resume.md](chapter-07-ehci-resume.md) |
| 8 | Resume trong xHCI | [chapter-08-xhci-resume.md](chapter-08-xhci-resume.md) |
| 9 | Các hàm quan trọng | [chapter-09-key-functions.md](chapter-09-key-functions.md) |
| 10 | Phân tích log Resume (A–F) | [chapter-10-resume-log-analysis.md](chapter-10-resume-log-analysis.md) |
| 11 | Tình huống debug thực tế | [chapter-11-debug-scenarios.md](chapter-11-debug-scenarios.md) |
| 12 | Cách debug Resume | [chapter-12-how-to-debug.md](chapter-12-how-to-debug.md) |
| 13 | So sánh tổng kết | [chapter-13-final-comparison.md](chapter-13-final-comparison.md) |

Bảng thuật ngữ: [glossary.md](glossary.md). Tài liệu HTML tổng hợp: [usb-resume-handbook.html](usb-resume-handbook.html).

Tài liệu liên quan: `docs/diagrams/xhci-resume-sequence.md` (luồng `xhci_resume`), `docs/usb-dma/` (khoá USB DMA), `OP_BTS_analysis_context.md`.
