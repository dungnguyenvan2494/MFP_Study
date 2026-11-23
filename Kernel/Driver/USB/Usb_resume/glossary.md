# Bảng thuật ngữ USB Resume

| Thuật ngữ | Tiếng Việt | Nghĩa ngắn gọn | Chương |
|---|---|---|---|
| Suspend | tạm ngừng | không có lưu lượng, thiết bị ngủ, giữ địa chỉ và cấu hình | 1, 3 |
| Resume | tiếp tục | đưa thiết bị trở lại hoạt động, không enumerate lại | 1, 3 |
| Reset-resume | resume bằng reset | reset thiết bị rồi cấu hình lại như cũ, khi controller/thiết bị đã mất trạng thái | 1, 5, 6 |
| Remote Wakeup | đánh thức từ xa | thiết bị tự phát tín hiệu đánh thức host | 1, 4, 6 |
| Host-initiated Resume | resume do host | phần mềm host yêu cầu cổng phát Resume | 4, 6 |
| Disconnect | ngắt kết nối | thiết bị bị gỡ khỏi hệ thống, mất địa chỉ | 1 |
| Logical disconnect | ngắt kết nối logic | Linux gỡ thiết bị dù vẫn cắm (resume thất bại) rồi enumerate lại | 6, 10 |
| Persist (USB_PERSIST) | giữ thiết bị | cho phép reset-resume thay vì ngắt kết nối (`power/persist`) | 5 |
| J / K / SE0 | trạng thái dây | J = rảnh; K = Resume/bắt đầu gói; SE0 = EOP/Reset | 4 |
| EOP | kết thúc gói | SE0 ngắn rồi J | 4 |
| TDRSMDN | thời gian Resume xuôi | host/hub giữ K ≥ 20 ms (Linux 40 ms: `USB_RESUME_TIMEOUT`) | 4 |
| TDRSMUP | thời gian Resume ngược | thiết bị phát K 1–15 ms khi remote wakeup | 4 |
| TRSMRCY | thời gian phục hồi | 10 ms sau Resume trước khi giao tiếp | 4 |
| LPM (L1) | quản lý nguồn liên kết USB 2.0 | ngủ nhẹ, ra vào trong µs | 4 |
| U0 / U1 / U2 / U3 | trạng thái link USB 3.x | hoạt động / chờ nhanh / chờ chậm / suspend | 4, 8 |
| LTSSM | máy trạng thái link | quản lý huấn luyện và trạng thái link USB 3.x | 4 |
| LFPS | tín hiệu tuần hoàn tần số thấp | dùng để đánh thức, polling, warm reset | 4 |
| Recovery | phục hồi link | huấn luyện lại ngắn khi thoát U1/U2/U3 | 4, 8 |
| SS.Inactive | link bất hoạt | link hỏng, cần Warm Reset | 4, 8 |
| Warm Reset | reset mạnh | reset qua LFPS dài, dùng khi link hỏng | 4 |
| Hot Reset | reset trong băng | reset qua chuỗi huấn luyện khi link còn sống | 4 |
| Function Wake | đánh thức chức năng | thông báo của thiết bị USB 3.x cho biết interface nào muốn dậy | 4, 8 |
| Root Hub | hub gốc | hub ảo của host controller | 2, 3 |
| Port | cổng | điểm nối trên hub, có trạng thái suspend/link | 2, 3 |
| PORTSC | thanh ghi trạng thái cổng | EHCI/xHCI | 7, 8 |
| PORT_SUSPEND / PORT_RESUME | bit cổng EHCI | đang suspend / đang phát Resume (Force Port Resume) | 7 |
| STS_PCD | phát hiện thay đổi cổng | bit USBSTS của EHCI | 7 |
| PLS | trạng thái link của cổng | trường PORTSC xHCI (0 = U0, 3 = U3, 15 = Resume…) | 8 |
| LWS | strobe ghi link | phải đặt khi ghi PLS | 8 |
| PLC | thay đổi trạng thái link | bit change PORTSC xHCI | 8 |
| Port Status Change Event | sự kiện đổi cổng | Event của xHCI báo cổng có bit change | 8 |
| C_SUSPEND / C_LINK_STATE | bit thay đổi của hub | resume vừa xong / link vừa đổi | 3, 6 |
| Bus suspend / bus resume | ngủ/dậy root hub | `xhci_bus_suspend/resume`, `ehci_bus_suspend/resume` | 3, 7, 8 |
| System PM | quản lý nguồn hệ thống | Suspend-to-RAM, hibernate | 3, 5 |
| Runtime PM | quản lý nguồn khi chạy | từng thiết bị ngủ/dậy khi rảnh/cần | 3, 5 |
| Usage count | bộ đếm sử dụng | > 0 thì không ngủ | 3, 9 |
| Autosuspend / Autoresume | tự ngủ / tự dậy | sau N giây rảnh / khi có người cần | 3 |
| `dev_pm_ops` | bảng callback PM | suspend/resume/runtime_* | 5 |
| `reset_resume` | callback reset-resume | driver khởi tạo lại sau reset | 5 |
| Wakeup capability | khả năng đánh thức | hỗ trợ / được phép / driver cần | 5 |
| PHY | lớp vật lý | khối analog phải dậy trước controller | 2, 5, 9 |
| `/RST_SLP2` | tín hiệu reset của Quartz | KM giữ reset ngoại vi (gồm hub USB) trong lúc ngủ | 5, 6 |
| `usb_pm_stack` | ngăn xếp resume của KM | đảm bảo thứ tự resume thiết bị USB | 6 |
| `phy_usb3_mv_resume_fin` / `xhci_resume_fin` / `usb_resume_fin` | cờ đồng bộ của KM | PHY xong / xHCI xong / USB xong | 5, 6 |
| Deep sleep / SLEEP1 | hai mức ngủ Quartz | deep sleep = Linux Suspend-to-RAM; SLEEP1 không phải | 3, 6, 11 |
