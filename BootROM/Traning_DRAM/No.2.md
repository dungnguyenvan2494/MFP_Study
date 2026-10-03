# 適用No.2,3 — Thiết lập chu kỳ Refresh (Bản dịch tiếng Việt)

## Tiêu đề

**適用No.2,3　リフレッシュサイクルの設定** → Áp dụng No.2, 3 — Thiết lập chu kỳ Refresh

Thay đổi các tham số dùng cho training được cài đặt cố định trong BOOT.

## Bảng "Danh sách setting mới" (新設定一覧)

|Register|Setting mới (1200MHz)|Setting mới (1050MHz)|
|---|---|---|
|CH0_Refresh_timing|0x01500307|0x01260307|
|CH1_Refresh_timing|0x01500307|0x01260307|
|CH0_SelfRefresh_timing_0|0x01590159|0x012e012e|
|CH1_SelfRefresh_timing_0|0x01590159|0x012e012e|
|CH0_PowerDown_timing_1|0x00000004|0x00000004|
|CH0_MRS_timing|0x0000020f|−(không đổi)|
|CH1_MRS_timing|0x0000020f|−(không đổi)|

## Bảng "Chi tiết nội dung thay đổi" (変更内容詳細)

Cột: レジスタ(Register) / Ch / アドレス(Địa chỉ) / Bit / デフォルト値(Giá trị mặc định) / 現在の設定値(Giá trị đang set hiện tại) / 1200MHz / 1050MHz / 解説(Giải thích)

**1. Power-down Timing Register 1** — Ch0: `0xF00203A4`, Ch1: `0xF00205A4`, bit `[2:0] tPDEN` Mặc định=`0x00000001`, đang set=`0x00000001` → đổi thành `0x00000004` (cả hai tần số)

Giải thích:

- **【Giá trị spec】** tCMDCKE = min MAX(1.75ns, 3nCK) = 3nCK → 1200MHz: 2.5ns, 1050MHz: 2.86ns
- **【Giá trị đang set (mặc định)】** 1200MHz: 1 →0.83ns; 1050MHz: 1 →0.95ns
- **⇒Nội dung thay đổi**: 1200MHz: 4 →3.33ns; 1050MHz: 4 →3.81ns
- Ghi chú bên phải: **tCMDCKE**: độ trễ từ lệnh hợp lệ cho đến khi CKE chuyển xuống mức L

**2. MRS Timing Register** — Ch0: `0xF00203A8`, Ch1: `0xF00205A8`, bit `[4:0] tMRD` Mặc định=`0x00001004`, đang set=`0x00000201` → đổi thành `0x0000020F` (cả hai tần số)

Giải thích:

- **【Giá trị spec】** tMRD = min MAX(14ns, 10nCK) = 14ns
- **【Giá trị đang set】** 1200MHz: ~~17(0x11)~~ 1(0x1) →0.83ns
- **⇒Nội dung thay đổi**: 1200MHz: **15(0xF)** →**12.459ns※**
- Tại thời điểm tài liệu spec tham số, setting của 1200MHz là 17(=14.17ns), nhưng **do lỗi điều khiển register nên không thể ghi được bit[4]** (chữ đỏ, in đậm).
- Vì vậy, setting được đặt ở giá trị tối đa có thể cấu hình được, đồng thời sẽ điều chỉnh bù thêm bằng Wait riêng.
- Ghi chú bên phải: **tMRD**: thời gian chu kỳ lệnh set Mode register

**3. Refresh Timing Register** — Ch0: `0xF0020394`, Ch1: `0xF0020594`, bit `[26:16] tRFC`, `[13:0] tREFI` Mặc định=`0x00d9030d`, đang set=`0x00d9030d` → đổi thành `0x01500307` (1200MHz) / `0x01260307` (1050MHz)

Giải thích:

- **tRFC**:
    - **【Giá trị spec】** Tùy loại lệnh refresh mà có 2 loại: tRFCab (all bank) và tRFCpb (per bank). Giá trị spec thay đổi theo density (mỗi channel); với 8Gb thì tRFCab(all bank)=280ns, tRFCpb(per bank)=140ns
    - **【Giá trị đang set】** 1200MHz: 217(0xd9) →180.83ns; 1050MHz: 190(0xbe) →180.95ns (giá trị đang set nằm giữa tRFCab và tRFCpb: tRFCab > setting > tRFCpb)
    - **⇒Nội dung thay đổi**: 1200MHz: 336(0x150) →280.00ns; 1050MHz: 294(0x126) →280.00ns
- **tREFI**:
    - **【Giá trị spec】** tREFI(REFab)=3,904µs (all bank); REFpb(tREFpb)=488ns (per bank)
    - **【Giá trị đang set】** 775(0x307) →3,875µs
- Ghi chú bên phải: **tRFC**: thời gian từ refresh đến active, hoặc từ refresh đến refresh tiếp theo. **tREFI**: khoảng thời gian giữa các lần tự động refresh.

**4. Self-refresh Timing Register 0** — Ch0: `0xF0020398`, Ch1: `0xF0020598`, bit `[26:16] tXSNR`, `[10:0] tXSRD` Mặc định=`0x00e100e1`, đang set=`0x00e100e1` → đổi thành `0x01590159` (1200MHz) / `0x012e012e` (1050MHz)

Giải thích:

- **tXSNR = tXSRD**:
    - **【Giá trị spec】** tXSV = tXSR = min MAX(tRFCab+7.5ns, 2tCK) = tRFCab+7.5ns
        - 1200MHz: nếu lấy tRFCab=180.83ns thì tXSR ≈188.33ns = 226
        - 1050MHz: nếu lấy tRFCab=180.95ns thì tXSR ≈188.45ns < 198
    - **【Giá trị đang set】** 1200MHz: 225(0xe1) →187.50ns; 1050MHz: 200(0xc8) →190.48ns
    - **⇒Nội dung thay đổi**: 1200MHz: 345(0x159) →287.50ns; 1050MHz: 302(0x12E) →287.62ns
- Ghi chú bên phải: **tXSV**: khoảng cách từ khi kết thúc self-refresh đến lệnh hợp lệ tiếp theo. **tXSRD**: độ trễ sau khi kết thúc self-refresh.

## Bảng tham chiếu tiêu chuẩn JEDEC (phần dưới ảnh, không dịch số liệu vì là bảng spec gốc)

**Table 117: Refresh Requirement Parameters** — bảng thông số yêu cầu refresh theo density (2Gb~16Gb per channel): số bank/channel, refresh window tREFW, số lệnh REFRESH cần trong tREFW, average refresh interval tREFI/tREFIpb, REFRESH cycle time tRFCab/tRFCpb (theo từng density).

- Ô khoanh đỏ: **Average refresh interval (REFab) tREFI = 3.904µs** — đây chính là giá trị spec được trích dẫn ở mục tREFI phía trên.
- Chú thích bên phải: **REFab**: lệnh REFRESH toàn bộ bank (all-bank); **REFpb**: lệnh REFRESH từng bank riêng (per-bank).

**Table 4: Refresh Requirement Parameters** — bảng thông số cho riêng **8Gb per Channel**: tRFCab=280ns, tRFCpb=140ns, tPBR2PBR (thời gian giữa 2 lần refresh per-bank khác bank)=90ns.

- Ô khoanh đỏ: **tRFCab = 280ns** — chính là giá trị spec 280.00ns được dùng làm căn cứ cho nội dung thay đổi tRFC ở mục 3 phía trên.