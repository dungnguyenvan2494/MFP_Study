# 04 — Danh mục tổng hợp thanh ghi: cách đọc

Danh mục được **sinh tự động** (không chép tay) để kiểm tra được độ đầy đủ:

| Trang | Nội dung |
|---|---|
| [04a_catalog_mac.md](04a_catalog_mac.md) | MAC — bus `0x0000–0x07FF`, `0x1000–0x17FF` |
| [04b_catalog_bb.md](04b_catalog_bb.md) | BB `0x0800–0x0FFF`, `0x1800–0x1FFF`; RF-direct `0x2800–0x2FFF`; BB-pg (khoá bảng công suất); other (tên `R_0x…` ngoài các dải trên — của chip JGR3, không dùng) |
| [04c_catalog_rf.md](04c_catalog_rf.md) | RF — địa chỉ 0x00–0xFF trong từng path A/B |
| [04d_catalog_other.md](04d_catalog_other.md) | USB-local `0xFE00–0xFFFF`, USB3-PHY, SDIO-local |
| `work/catalog.csv`, `work/catalog.json` | Toàn bộ dữ liệu (kể cả danh sách hàm/vị trí đầy đủ) |

## Đơn vị của một dòng

**Một dòng = một cặp (không gian, địa chỉ)**, không phải một tên macro. Mọi tên trỏ về cùng địa chỉ trong cùng không gian được gộp (cột "Tên (alias)"): `REG_SYS_FUNC_EN_8822B`, `REG_SYS_FUNC_EN`, `ODM_BB_RESET`, `R_0x2`… là **một** thanh ghi. Ngược lại MAC 0x18 và RF 0x18 là hai dòng ở hai trang khác nhau.

Địa chỉ không có tên (`*(không có macro)*`) là địa chỉ chỉ xuất hiện dưới dạng số (ví dụ `REG_SYS_FUNC_EN + 1` = 0x0003, hoặc literal `0xFE58`). Chúng vẫn được giữ thành dòng riêng vì phần cứng nhận đúng byte đó.

## Cột

| Cột | Ý nghĩa | Nguồn |
|---|---|---|
| STT | thứ tự trong trang (tăng theo địa chỉ) | — |
| Địa chỉ | hex | giá trị macro được tính bởi bộ đánh giá biểu thức, hoặc literal |
| Tên (alias) | tên chính (ưu tiên tên `_8822B`) + số alias | `work/regs.json` |
| Không gian | MAC/BB/RF A/B/… | dải địa chỉ + hàm truy cập |
| St | trạng thái — bảng dưới | `tools/report.py` |
| Nhóm | **[INFERENCE]** suy từ tên các hàm truy cập (regex trong `report.py: GROUP_RX`) — chỉ để định hướng, phân tích chi tiết ở 05 | — |
| W | độ rộng truy cập **quan sát được** (8/16/32; 20 = RF) — không phải độ rộng phần cứng của thanh ghi | access + bảng |
| Kiểu | `R` đọc, `W` ghi, `RMW-mask` ghi field qua mask, `R-mask` đọc field, `RMW-set/clr` (HALMAC W*_SET/CLR) | accessor |
| Truy cập L/M/G/X/T | số lời gọi: Live / MP / debuG / chết(X) / mục bảng(T) | — |
| Bit d/u | số macro bit/field được định nghĩa dưới section của thanh ghi / số được dùng trong code | `work/bits.json` |
| Hàm, Vị trí | tối đa 4 hàm / 3 vị trí của nhóm trạng thái quyết định; đầy đủ trong CSV | — |

## Trạng thái (St)

| Mã | Ý nghĩa | Ứng với yêu cầu |
|---|---|---|
| **A** | Được truy cập trên đường sống của RTL8822BU (code, hoặc bảng khởi tạo không điều kiện, hoặc power-seq cho USB/ALL) | "Được sử dụng trong nhánh build" |
| **T** | Chỉ xuất hiện trong bảng **có điều kiện lúc chạy**: nhánh `IF rfe==N` của bảng PHY/AGC/RF, hoặc pinmux GPIO — dùng hay không tuỳ board [KIỂM TRA TRÊN BOARD] | |
| **M** | Chỉ được truy cập từ code MP (`core/rtw_mp.c`, `hal/hal_mp.c`, `ioctl_mp.c`, hàm `mpt_*`) | "Chỉ dùng trong chế độ MP/test" |
| **G** | Chỉ từ code debug (`rtw_debug.c`, `rtw_proc.c`, `phydm_debug.c`, hàm `*dump*`, `rtw_dbg_port`…) | "debug" |
| **X** | Có lời gọi nhưng **tất cả** đều chết với RTL8822BU: nhánh chip/interface khác, early-return, cấu hình mặc định tắt, hoặc hàm không có nơi gọi | "Chỉ dành cho chipset khác…" |
| **R** | Tên được tham chiếu trong code active nhưng không qua accessor đã nhận diện (lưu vào biến/struct, so sánh, tính toán) — **cần xem tay** | "Chưa xác định" |
| **Z** | Tên chỉ xuất hiện trong code bị `#if` loại | "Code bị vô hiệu hoá" |
| **D** | Có `#define` active nhưng không được tham chiếu ở đâu | "Có trong header nhưng không thấy dùng" |

Độ tin cậy: **A/M/G/X** — "Source xác nhận" về *việc truy cập*; chức năng/bit trong 05 mới gắn nhãn riêng. Không có dòng nào được đánh "datasheet xác nhận" vì repository không kèm datasheet.

## Cách tái tạo

```sh
D=Kernel/K-S800/Src/drivers/net/wireless/realtek/rtl8812BU_v1930.24
T=docs/rtl8822bu_registers/tools; W=docs/rtl8822bu_registers/work
py -I $T/mk_eval.py $D > $W/build.json
py -I $T/cpp_mask.py $D $W/build.json Kernel/K-S800/Src/arch/arm64/configs/Eagle_defconfig $W/mask.json   # ~100 s
py -I $T/inventory.py $D $W/mask.json $W                                                                    # ~12 s
py -I $T/tables.py    $D $W/mask.json $W/tables.json
py -I $T/report.py    $D $W docs/rtl8822bu_registers
py -I $T/report_extra.py $W docs/rtl8822bu_registers
```

## Giới hạn đã biết của công cụ

1. Mask `#if` không parse header kernel (`<linux/…>`).
2. Guard lúc chạy: precedence C ≠ Python với biểu thức `a & b == c` không ngoặc; `else` không ngoặc nhọn và `switch` lồng phức tạp có thể bị bỏ sót → kết quả nghiêng về "live" (an toàn: không loại nhầm).
3. Đồ thị gọi: con trỏ hàm gán trong khởi tạo struct ở phạm vi file được coi là "sống".
4. Nhóm (cột Nhóm) chỉ là gợi ý theo tên hàm.
5. Bảng PHY/RF: điều kiện `rfe` được giải mã nhưng không biết RFE type thật của board → các mục có điều kiện để ở trạng thái T.
