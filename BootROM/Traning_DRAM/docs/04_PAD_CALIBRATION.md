# 04 — Pad Calibration: pad_cal() thật trong S800

Đọc `00`-`03` trước, đặc biệt `02` (MC6 vs PHY, cơ chế `prfa`) và `03` (edge/window/center) — pad calibration **không dùng công thức center** ở tài liệu 03, mà dùng một cơ chế khác: **calibration tự động của hardware, rồi đọc kết quả về, rồi "đóng băng" (freeze) giá trị đó bằng tay**. Đây là lý do phải học riêng.

# Concept

Trước khi hỏi "pad_cal() làm gì", hỏi: **"pad" là gì?**

📘[GENERAL KNOWLEDGE] Mỗi chân vật lý (pin) của chip nối ra ngoài đi qua 1 mạch nhỏ gọi là **I/O pad**, gồm 2 phần:
- **Output driver**: mạch đẩy điện áp ra chân khi chip muốn gửi bit 1/0 ra ngoài (ví dụ khi Controller ghi dữ liệu). Về bản chất là 1 cặp transistor PMOS (kéo lên VDD → tạo mức cao) và NMOS (kéo xuống GND → tạo mức thấp).
- **Receiver**: mạch đọc điện áp trên chân vào để quyết định đó là bit 0 hay 1, thường bằng cách so sánh với 1 mức điện áp tham chiếu gọi là **Vref** (nếu điện áp chân > Vref → đọc là 1, ngược lại → 0).

# Why

📘[GENERAL KNOWLEDGE] Cả driver và receiver có 1 thông số vật lý gọi là **trở kháng (impedance)** — về bản chất là "độ khó" mà dòng điện gặp phải khi đi qua mạch đó. Chuẩn LPDDR4 quy định trở kháng driver/receiver phải nằm trong 1 dải nhất định (ví dụ 40Ω, 48Ω...) để khớp với trở kháng của đường dây PCB — nếu không khớp, tín hiệu bị **phản xạ** ngược lại trên đường dây (giống tiếng vang khi âm thanh gặp tường), làm méo dạng sóng (xem tài liệu 01, mục "signal integrity").

**Vấn đề thực tế**: trở kháng của transistor PMOS/NMOS trên silicon **không cố định** — nó phụ thuộc vào: nhiệt độ chip lúc đó, điện áp nguồn thực tế lúc đó, và sai lệch sản xuất (process variation) của riêng con chip đó. Vì vậy **không thể khắc một giá trị driver strength cố định vào ROM** — phải đo thực tế mỗi lần khởi động, rồi tự bù trừ (calibrate) bằng cách chỉnh số lượng transistor PMOS/NMOS được "bật" song song trong driver — đây là ý nghĩa của "calibration": không đổi thiết kế mạch, mà đổi **bao nhiêu nhánh transistor tham gia** để đạt đúng trở kháng mục tiêu.

**Dynamic calibration** = quá trình đo tự động này được 1 máy trạng thái phần cứng (hardware state machine) trong PHY thực hiện, không cần CPU tính toán — CPU chỉ cần "bật" nó, đợi, rồi đọc kết quả.

# Hardware view

[SOURCE FACT — `atf\atf_s800\ble\ddr_phy_regstructs.h:90-91, 144, 302`] 3 máy trạng thái calibration riêng biệt tồn tại trong PHY, xác nhận qua 3 register:

| Register | Trục calibrate |
|---|---|
| `dram_main_pads_cal_mach_ctrl` (+0x4cc) | Máy chính — điều khiển bật/tắt calibration + đọc trạng thái xong/chưa |
| `dram_vert_cal` (+0x4c8) | Calibration theo chiều "vertical" (🧠[INFERENCE] — có thể là nhóm pad theo hướng đặt trên die; chưa có bằng chứng chính xác ý nghĩa vật lý "vertical/horizontal" trong tài liệu Marvell nào đã đọc — ❓UNKNOWN, không đoán thêm) |
| `dram_hz_cal_mach_ctrl` (+0x7c8) | Calibration theo chiều "horizontal" |
| `dram_ctrl_cal_mach_ctrl` (+0xdc8) | Máy calibration cho khối DRAM Controller (ghi giá trị pcal/ncal cuối cùng vào đây) |

# Firmware view — trace `pad_cal()` từng bước

[SOURCE FACT — `atf\atf_s800\ble\mv_lpddr4_apn806.c:1065-1196`, hàm đầy đủ, comment gốc: "Calibrates the pads for LPDDR4 prior to running training algorithms"]

## Bước 0 — chuẩn bị trước khi gọi pad_cal() (không nằm trong hàm, nhưng luôn chạy ngay trước nó)

[SOURCE FACT — `:8096-8102`]:
```c
mrvl_regwrite32((addr_p)(CSS_ADDR + 0x4360), 0xFFFF0000); // ref range select
mrvl_regwrite32((addr_p)(CSS_ADDR + 0x8d38), 0x66660000); // Vref calibration values
delay_us(10);   // để Vref ổn định sau khi set
mrvl_regwrite32((addr_p)&phy_reg->sdram_cfg, 0x0b104c30);
mrvl_regwrite32((addr_p)&phy_reg->sdram_cfg, 0x7b104c30);  // reset PHY (2 lần ghi = xung reset)
display_measure_time(km_measure_time_flag, TRN_B0);  // ← mốc "S800 Pad Calibration開始"
pad_cal();
```
Comment gốc "Vref calibration values" ghi tại `CSS_ADDR` (một vùng địa chỉ khác cả MC6 và PHY — 🧠[INFERENCE] CSS = có thể là "Chip SubSystem/Core SubSystem", chưa xác nhận tên đầy đủ, ❓UNKNOWN) cho thấy: **có một bước set Vref tham chiếu ở tầng SoC, trước cả khi vào pad_cal() của riêng DDR PHY** — Vref không chỉ tồn tại trong DDR PHY.

## Bước 1 — set Vref nội bộ PHY + tắt comparator (dòng 1078-1087)

```c
regval = nova_ddrphy_read(0xA8,0,0);
regval &= 0xffffff00;
regval |= 0x4a;
nova_ddrphy_write(0xA8,0,ALL_PUPS,regval);      // set 1 phần Vref riêng của PHY (khác Vref ở bước 0)

nova_ddrphy_write(0xa2,0,ALL_PUPS,0x0);          // disable cmos receiver enable comparator
nova_ddrphy_write(0xa1,0,ALL_PUPS,0x7ff);
```
Comment gốc "disable cmos receiver enable comparator" — 🧠[INFERENCE] tắt bộ so sánh của receiver **trong lúc đang calibrate driver**, để tránh receiver "đọc nhiễu" các mức điện áp không ổn định trong quá trình calibration làm sai lệch kết quả đo.

## Bước 2 — Enable (bật máy calibration tự động)

```c
regval = DDR_PHY_DRAM_MAIN_PADS_CAL_MACH_CTRL_CALUPCTRL_REPLACE_VAL(..., 0x1);
regval = DDR_PHY_DRAM_MAIN_PADS_CAL_MACH_CTRL_DYNPADCALEN_REPLACE_VAL(regval, 0x1);  // DYNPADCALEN=1 : bật calibration ĐỘNG
mrvl_regwrite32((addr_p) &phy_reg->dram_main_pads_cal_mach_ctrl, regval);

regval = ..._CALUPCTRL_REPLACE_VAL(..., 0x2);   // CALUPCTRL=2 : "calibration update external"
mrvl_regwrite32(...);
```
`DYNPADCALEN` (**Dyn**amic **Pad** **Cal**ibration **En**able) là bit thực sự khởi động máy trạng thái tự đo trở kháng. `CALUPCTRL` (Calibration Update Control) chọn cách kết quả được áp dụng — giá trị `0x2` ("external") nghĩa là để phần cứng tự cập nhật, khác với chế độ "manual" sẽ dùng ở Bước 5.

## Bước 3 — Wait + Poll (chờ máy trạng thái báo xong)

```c
delay_us(1);
poll_cal_init_status(phy_reg);       // dòng 1042-1056
poll_cal_lock_status(phy_reg);       // dòng 1019-1033
delay_us(10000);                     // chờ thêm 10ms
```

[SOURCE FACT — `:1042-1056`] `poll_cal_init_status()`:
```c
regval = mrvl_regread32(&phy_reg->dram_main_pads_cal_mach_ctrl);
while ( (regval & CALMACHSTATUS_MASK) != CALMACHSTATUS_MASK && count < 100 ) {
    count++;
    regval = mrvl_regread32(&phy_reg->dram_main_pads_cal_mach_ctrl);
}
if (count >= 100) mrvl_msg(MSG_ERROR, "...Failed Calibration init...");
```
→ đợi bit `CALMACHSTATUS` (trạng thái máy calibration) chuyển thành "xong" (mask khớp hoàn toàn), tối đa 100 lần đọc.

[SOURCE FACT — `:1019-1033`] `poll_cal_lock_status()`:
```c
regval = mrvl_regread32(&phy_reg->dram_phy_lock_status);
while ( regval != 0x3ffffff && count < 100 ) { ...; count++; }
if (count >= 100) mrvl_msg(MSG_ERROR, "...Failed Calibration propogation to IO...");
```
→ đây chính là bước **"DLL lock"** trong Phần đề bài yêu cầu giải thích: `dram_phy_lock_status` phải bằng đúng `0x3ffffff` (26 bit 1) — 🧠[INFERENCE] mỗi bit trong 26 bit này rất có thể tương ứng với 1 pad/kênh calibration cần báo "đã khoá/ổn định" riêng; khi tất cả 26 bit đều lên 1, nghĩa là **giá trị calibration đã thực sự truyền tới (propagate) tận I/O pad thật**, không chỉ tính xong trong máy trạng thái nội bộ. Đây khác với `poll_cal_init_status` — 1 cái hỏi "máy tính xong chưa", 1 cái hỏi "giá trị đã tới tận chân vật lý chưa".

## Bước 4 — Disable dynamic + đọc kết quả tự động

```c
regval = ..._DYNPADCALEN_REPLACE_VAL(..., 0x0);   // tắt calibration động
mrvl_regwrite32(...);
delay_us(1);
poll_cal_init_status(phy_reg);
poll_cal_lock_status(phy_reg);

regval = mrvl_regread32(&phy_reg->dram_ctrl_cal_mach_ctrl);
ncal = AUTONGCALVAL_MASK_SHIFT(regval);   // giá trị NMOS calibration mà hardware tự đo được
pcal = AUTOPGCALVAL_MASK_SHIFT(regval);   // giá trị PMOS calibration mà hardware tự đo được
```
Đây là **"read result"**: sau khi hardware tự chạy xong, đọc lại 2 số `ncal`/`pcal` — chính là số lượng nhánh NMOS/PMOS tối ưu mà máy trạng thái vừa đo được (xem lại phần Why — đây là "bao nhiêu nhánh transistor tham gia" nói ở trên).

## Bước 5 — Manually apply (đóng băng giá trị đo được, không cho tự động chỉnh nữa)

```c
regval = ..._CALVALMANOVRD_REPLACE_VAL(..., 0x1);        // CALVALMANOVRD=1 : ép chế độ MANUAL override
mrvl_regwrite32(&phy_reg->dram_ctrl_cal_mach_ctrl, regval);

regval = ..._MANNGCALVAL_REPLACE_VAL(..., ncal);          // ghi lại đúng giá trị vừa đọc được
regval = ..._MANPGCALVAL_REPLACE_VAL(regval, pcal);
mrvl_regwrite32(&phy_reg->dram_ctrl_cal_mach_ctrl, regval);

nova_ddrphy_write(0xBF,0,ALL_PUPS,0x1);
nova_ddrphy_write(0xBF,1,ALL_PUPS,0x1);
```
🧠[INFERENCE]: đây là bước quan trọng nhất về mặt thiết kế — firmware **không để hardware tiếp tục tự động chỉnh calibration liên tục trong lúc training** (dù hardware có khả năng đó, `DYNPADCALEN`), mà **đo 1 lần, rồi ép cố định (`CALVALMANOVRD`) đúng giá trị đã đo**. Lý do hợp lý: nếu để calibration tự do chạy động trong khi đồng thời đang chạy CA training/deskew (các bước sau), driver strength có thể trôi giữa lúc đo delay/Vref, làm kết quả training không ổn định. ❓UNKNOWN: chưa có tài liệu spec xác nhận trực tiếp lý do này — đây là suy luận kỹ thuật hợp lý, không phải fact.

## Bước 6 — Enable lại + đọc thêm 2 giá trị vertical/horizontal (chỉ để log, không áp dụng lại)

```c
regval = ..._DYNPADCALEN_REPLACE_VAL(..., 0x1);
mrvl_regwrite32(...);
delay_us(1); poll_cal_init_status(phy_reg);
regval = ..._DYNPADCALEN_REPLACE_VAL(..., 0x1);   // (lặp lại, đúng như source thật — không phải lỗi chép tài liệu)
regval = ..._CALUPCTRL_REPLACE_VAL(regval, 0x1);  // CALUPCTRL=1 : "calibration update internal" lần này
mrvl_regwrite32(...);

regval = mrvl_regread32(&phy_reg->dram_vert_cal);      // đọc pcal/ncal "vertical" — chỉ log
regval = mrvl_regread32(&phy_reg->dram_hz_cal_mach_ctrl); // đọc pcal/ncal "horizontal" — chỉ log
```
⚠️[SPEC/SOURCE MISMATCH — cần theo dõi]: 2 lệnh ghi `DYNPADCALEN_REPLACE_VAL(...,0x1)` liên tiếp (dòng 1151 và 1163 trong source) là **giống nhau tuyệt đối**, không có gì khác biệt ở giữa ngoài log message — 🧠[INFERENCE] rất có khả năng là code thừa/duplicate (dead code) từ quá trình fork Marvell driver, không phải chủ ý. Không tự sửa source, chỉ ghi nhận.

## Bước 7 — Disable hẳn

```c
regval = ..._DYNPADCALEN_REPLACE_VAL(..., 0x0);
regval = ..._CALUPCTRL_REPLACE_VAL(regval, 0x0);
mrvl_regwrite32(&phy_reg->dram_main_pads_cal_mach_ctrl, regval);
mrvl_msg(MSG_ALGO, RAW_DATA, "Done Pad Calibration\n");
return MV_OK;
```

# Register view

| Register | Field | Ý nghĩa | Ghi bởi | Đọc bởi |
|---|---|---|---|---|
| `dram_main_pads_cal_mach_ctrl` | `DYNPADCALEN` | 1=bật máy tự calibrate động, 0=tắt | `pad_cal()` | `poll_cal_init_status()` (đọc `CALMACHSTATUS`) |
| `dram_main_pads_cal_mach_ctrl` | `CALUPCTRL` | Chọn cơ chế cập nhật kết quả: external(2)/internal(1)/off(0) | `pad_cal()` | — |
| `dram_main_pads_cal_mach_ctrl` | `CALMACHSTATUS` | Cờ báo máy calibration đã tính xong | hardware | `poll_cal_init_status()` |
| `dram_phy_lock_status` | (26-bit, toàn `0x3ffffff` = lock hoàn tất) | Báo giá trị đã truyền tới tận I/O pad | hardware | `poll_cal_lock_status()` |
| `dram_ctrl_cal_mach_ctrl` | `AUTONGCALVAL`/`AUTOPGCALVAL` | Giá trị NMOS/PMOS mà hardware tự đo | hardware | `pad_cal()` (đọc để ghi lại thủ công) |
| `dram_ctrl_cal_mach_ctrl` | `CALVALMANOVRD` | 1 = ép dùng giá trị thủ công (`MANNGCALVAL`/`MANPGCALVAL`), bỏ qua auto | `pad_cal()` | — |
| `dram_ctrl_cal_mach_ctrl` | `MANNGCALVAL`/`MANPGCALVAL` | Giá trị NMOS/PMOS cuối cùng, cố định, dùng cho toàn bộ training sau đó | `pad_cal()` | — |

# Source view

- `mv_lpddr4_apn806.c:1065-1196` — `pad_cal()` toàn bộ (đã đọc hết)
- `mv_lpddr4_apn806.c:1019-1033` — `poll_cal_lock_status()`
- `mv_lpddr4_apn806.c:1042-1056` — `poll_cal_init_status()`
- `mv_lpddr4_apn806.c:8096-8118` — call site thật, có mốc `TRN_B0`→`pad_cal()`→`TRN_B1`→`MCConfig_ap806_dual_chan_ap806_dual_chan_100_x32_dpi()`→`TRN_B2`

# Call flow

```
(trước pad_cal, ở caller)
  set Vref tầng SoC (CSS_ADDR)  →  reset PHY (sdram_cfg)  →  display TRN_B0
        |
        v
pad_cal()
   set Vref nội bộ PHY, tắt comparator
        |
   ENABLE (DYNPADCALEN=1, CALUPCTRL=external)
        |
   WAIT delay_us(1)  →  POLL init status  →  POLL lock status (DLL/IO lock)  →  WAIT thêm 10ms
        |
   DISABLE dynamic  →  POLL lại init+lock
        |
   READ ncal/pcal tự động đo được
        |
   MANUALLY APPLY: CALVALMANOVRD=1, ghi MANNGCALVAL/MANPGCALVAL = ncal/pcal vừa đọc
        |
   (enable lại + đọc thêm 2 giá trị vertical/horizontal — chỉ log, có đoạn lặp nghi là dead code)
        |
   DISABLE hẳn (DYNPADCALEN=0, CALUPCTRL=0)  →  return MV_OK
        |
        v
(ở caller) display TRN_B1  →  power_adll()  →  MCConfig_..._dpi()  →  display TRN_B2
```

# Diagram

```
   PMOS (kéo lên VDD)              NMOS (kéo xuống GND)
        |                                |
        +---------- I/O PAD -------------+
                       |
                    chân vật lý (pin)
                       |
              đường dây PCB (impedance cố định do thiết kế PCB)

Calibration = chỉnh "bao nhiêu nhánh PMOS/NMOS song song được bật"
              để trở kháng driver khớp với trở kháng đường dây PCB.
              ncal/pcal = số đo được → ghi cố định (manual override) → dùng suốt training.
```

# K-S800 customization

Không tìm thấy define `KM_*` nào bọc quanh `pad_cal()` — 🧠[INFERENCE] hàm này gần như chắc chắn **giữ nguyên từ Marvell gốc**, không bị KM tùy biến (khác với các hàm training Vref/deskew ở tài liệu 03 có rất nhiều `#ifdef KM_TRAINING_REFINE`). Việc set Vref ở `CSS_ADDR` (Bước 0) và bước "reset PHY" trước khi gọi `pad_cal()` nằm ở caller, ❓UNKNOWN liệu 2 bước đó có phải do KM thêm hay cũng là code gốc Marvell — cần so sánh với `atf/atf_s800/drivers/marvell/mv_ddr/apn806/mv_ddr_apn806.c` (bản Marvell riêng, ghi chú ❓UNKNOWN từ tài liệu 00) ở một session sau.

# Debugging

Nếu boot log in ra:
- `"!!!Failed Calibration init!!!"` → máy trạng thái calibration nội bộ không báo xong sau 100 lần poll (~vài chục µs-1ms) — nghi ngờ PHY chưa được cấp clock/power đúng, hoặc lỗi ở bước reset PHY trước đó (`sdram_cfg` write ở dòng 8101-8102).
- `"!!!Failed Calibration propogation to IO!!!"` → máy trạng thái đã tính xong nhưng giá trị không "tới" được I/O pad thật (`dram_phy_lock_status` không đạt `0x3ffffff`) — nghi ngờ vấn đề phần cứng ở tầng pad/board, khác hẳn lỗi ở trên.
- Cả 2 lỗi này xảy ra **trước khi CA training/deskew bắt đầu** — nếu gặp 1 trong 2, các bước training phía sau (05-11) hầu như chắc chắn cũng FAIL, vì chúng dựa vào driver/receiver đã calibrate đúng.

# Summary

- Pad calibration chỉnh trở kháng driver/receiver (bao nhiêu nhánh PMOS/NMOS bật song song) để khớp đường dây PCB — không sửa thiết kế, chỉ bù trừ runtime.
- Quy trình thật: enable máy tự động → poll 2 tầng (tính xong nội bộ, và giá trị đã tới pad thật) → đọc kết quả tự động đo được (ncal/pcal) → ép cố định bằng tay (manual override) → tắt hẳn.
- 2 loại "lock" khác nhau: `CALMACHSTATUS` (máy tính xong) và `dram_phy_lock_status` (giá trị đã propagate ra I/O) — phải phân biệt khi debug.
- Firmware chủ động **không** để calibration tự do chạy suốt training — chốt giá trị 1 lần để các bước sau (CA training, deskew...) có 1 baseline điện ổn định.
- Có 1 đoạn code nghi là duplicate/dead-code (2 lần set DYNPADCALEN=1 liên tiếp) — ghi nhận, không tự sửa.
- `pad_cal()` không dùng công thức "center" ở tài liệu 03 — nó dùng cơ chế auto-measure-then-freeze, khác hẳn cơ chế sweep-and-center của CA training/deskew.

# Verify yourself

1. Nếu bỏ qua hoàn toàn bước "manually apply" (Bước 5), để hardware tự do chạy `DYNPADCALEN=1` suốt quá trình training phía sau, điều gì có thể xảy ra với kết quả CA training/deskew (gợi ý: liên hệ tài liệu 03, khái niệm Window có thể "trôi")?
2. `poll_cal_init_status` và `poll_cal_lock_status` khác nhau ở điều gì — 1 hỏi cái gì, 1 hỏi cái gì?
3. Vì sao phải tắt "cmos receiver enable comparator" trước khi bắt đầu calibrate driver?
4. `ncal`/`pcal` lần lượt ứng với NMOS/PMOS. Tại sao calibration cần đo *cả 2* loại transistor riêng biệt, không chỉ 1?
5. Đoạn code Bước 6 ghi 2 lần `DYNPADCALEN=1` giống nhau — theo nguyên tắc source-grounding, bạn có được phép kết luận "đây chắc chắn là bug" không, hay chỉ được ghi nhận là "nghi ngờ, cần xác minh thêm"?

**Tiếp theo:** `05_CA_TRAINING.md` — CA training/CBT thật (đã thấy một phần ở tài liệu 03, ví dụ 3) — Vref sweep, CA delay, clock alignment, và đoạn comment bug-fix `vref換算ミスの修正` sẽ được điều tra kỹ hơn ở đây.
