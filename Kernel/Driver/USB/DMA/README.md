# Khoá học USB DMA trên Linux Kernel

Baseline: **Linux 5.10.258** (`Kernel/Src`), board Marvell Armada Quartz EMU 800 (arm64).
Đặc điểm DMA của board: xHCI mask 32-bit, non-coherent, không IOMMU, SWIOTLB 64 MiB, RAM có phần trên 4 GiB.

| Chương | Nội dung | File | Trạng thái |
|---|---|---|---|
| 1 | Hiểu DMA trong kiến trúc USB | [chapter-01-dma-in-usb-architecture.md](chapter-01-dma-in-usb-architecture.md) | xong |
| 2 | Nền tảng DMA (mapping, coherent/streaming, direction, mask, IOVA) | [chapter-02-dma-fundamentals.md](chapter-02-dma-fundamentals.md) | xong |
| 3 | Vòng đời hoàn chỉnh một lần truyền (Bulk OUT/IN, hủy URB, kiến trúc 4 làn) | [chapter-03-usb-dma-data-flow.md](chapter-03-usb-dma-data-flow.md) | xong |
| 4 | Cơ chế DMA của EHCI (QH, qTD, iTD, siTD, schedule) | [chapter-04-ehci-dma.md](chapter-04-ehci-dma.md) | xong |
| 5 | Cơ chế DMA của xHCI (ring, TRB, doorbell, context, interrupter, cycle bit) | [chapter-05-xhci-dma.md](chapter-05-xhci-dma.md) | xong |
| 6 | Cache coherency và đồng bộ bộ nhớ | [chapter-06-cache-coherency.md](chapter-06-cache-coherency.md) | xong |
| 7 | IOMMU, DMA mask và bounce buffer | [chapter-07-iommu-mask-bounce.md](chapter-07-iommu-mask-bounce.md) | xong |
| 8 | Phân tích log kernel USB DMA (theo nhóm A–F) | [chapter-08-log-analysis.md](chapter-08-log-analysis.md) | xong |
| 9 | Từ log lần ngược về luồng thực thi | [chapter-09-log-to-flow.md](chapter-09-log-to-flow.md) | xong |
| 10 | Tình huống debug thực tế | [chapter-10-debug-scenarios.md](chapter-10-debug-scenarios.md) | xong |
| 11 | Công cụ debug | [chapter-11-debug-tools.md](chapter-11-debug-tools.md) | xong |
| 12 | So sánh DMA của EHCI và xHCI | [chapter-12-ehci-vs-xhci.md](chapter-12-ehci-vs-xhci.md) | xong |

Tài liệu liên quan trong repo:
- `docs/diagrams/xhci-resume-sequence.md`: luồng resume của xHCI.
- `OP_BTS_analysis_context.md`: phân tích OP_BTS-71321 và OP_BTS-71095 (ví dụ thực tế dùng trong khoá học).

Tài liệu HTML tổng hợp (sidebar, tìm kiếm, bảng thuật ngữ): [usb-dma-handbook.html](usb-dma-handbook.html). Bảng thuật ngữ riêng: [glossary.md](glossary.md).
