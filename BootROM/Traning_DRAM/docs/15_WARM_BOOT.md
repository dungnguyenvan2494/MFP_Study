# 15 — Warm Boot / Sleep Resume: mv_lpddr4_sleep.c thật

Đọc `00`-`14` trước, đặc biệt `14` (cơ chế `BOOT_SYNC_DDR_WARMBOOT` sẽ được giải thích đầu-cuối ở đây) và các phát hiện "skip khi warmboot" rải rác ở tài liệu 07/08/09/10/11/12.

File này (`lppp_s800\Hal\quartz\asic\build\mv_lpddr4_sleep.c`, 791 dòng kể cả license header, đã đọc **toàn bộ phần code thật**) chạy trên **LPPP** (không phải ATF/ble) — đây là phía "ra lệnh ngủ/thức" cho DDR, khác với phía "training" đã học ở các tài liệu trước.

# Concept

Cold Boot = full training. Warm Boot = restore tham số đã train, **không train lại**. Nhưng có 1 câu hỏi nền tảng phải trả lời trước: **trong lúc Sleep, dữ liệu trong DRAM có mất không?**

**Trả lời trực tiếp bằng bằng chứng thật**: KHÔNG — vì lúc "ngủ", DRAM không bị cắt điện, mà được đưa vào **Self-Refresh** (`mc_enter_sr()`, xem dưới) — 1 chế độ JEDEC (📘[GENERAL KNOWLEDGE]) trong đó chính con chip DRAM **tự refresh nội bộ bằng mạch riêng của nó**, không cần Memory Controller ra lệnh refresh liên tục nữa — do đó tốn ít điện hơn refresh bình thường rất nhiều, nhưng **dữ liệu vẫn còn**. Đây là lý do "Warm Boot không train lại" và "dữ liệu không mất" là **2 khái niệm độc lập, cùng đúng, vì 2 lý do khác nhau**: dữ liệu không mất vì Self-Refresh vẫn nạp điện cho tụ; tham số training không cần train lại vì (🧠[INFERENCE]) bản chất vật lý của đường dây/silicon không đổi giữa lúc ngủ và lúc thức (chỉ đổi nếu nhiệt độ trôi quá xa — liên hệ tài liệu 03 Margin).

# Why

📘[GENERAL KNOWLEDGE]: Train lại từ đầu tốn hàng trăm ms (tài liệu 14: SPEC FACT ~650ms). Mục tiêu Sleep Resume của spec là ~150ms (tài liệu 00). Không thể đạt được nếu train lại — phải **restore trực tiếp giá trị delay/Vref đã tìm được từ lần cold boot trước**, ghi thẳng vào PHY register, bỏ qua toàn bộ bước sweep.

# Hardware view + Firmware view — trace các hàm thật trong `mv_lpddr4_sleep.c`

## Đi ngủ: `mc_save_state()` (dòng 711-728, đã đọc toàn bộ)

```c
void mc_save_state()
{
    if (power_debug_flags_get_pdf_verify_ddr()) {   // cờ debug, KHÔNG chạy mặc định
        xor_corruption = mrvl_regread32(0);
        ddr_verify_crc(false);                       // lưu CRC32 snapshot 4 vùng DRAM lớn — xem dưới
    }
    mc_drain();                                       // đợi Memory Controller "xả" hết lệnh ghi đang chờ
    mc_enter_pd((MC6_REGS_t *)MC6_EXT_BASE, 2, 2);     // đưa DRAM vào Power-Down (bên trong: Self-Refresh + tắt pull-down PHY)
}
```

`mc_enter_pd()` (dòng 392-485) bên trong gọi `mc_enter_sr()` (dòng 270-357) rồi tắt thêm 1 bit "pull-down" trên PHY (`nova_ddrphy_write(0xA8,1,i, regval|0x200)`) — 🧠[INFERENCE] pull-down PHY là mạch giữ mức điện ổn định trên các chân khi không dùng; tắt nó nữa (sau khi đã vào Self-Refresh) là bước tiết kiệm điện SÂU HƠN — DRAM vẫn tự refresh, nhưng PHY (phía SoC) tắt thêm phần không cần thiết.

## Thức dậy: `mc_restore_state()` (dòng 730-757, đã đọc toàn bộ)

```c
void mc_restore_state()
{
    if (power_debug_flags_get_pdf_verify_ddr()) {
        mrvl_regwrite32(0, xor_corruption);
        ddr_verify_crc(true);                         // so sánh CRC32 hiện tại với snapshot lúc save
        mrvl_regwrite32(0, xor_corruption);
    }
#ifdef FLEX_quartz
    syncronize_boot_set(BOOT_SYNC_DDR_CONT_BOOT(1));   // báo cho AP (ATF): "DDR đã sẵn sàng, tiếp tục boot"
#endif
}
```
Comment gốc ngay trong hàm: *"restore training corruption locations... n/a"*, *"restore training config... n/a"* — 🧠[INFERENCE quan trọng]: **chính hàm này KHÔNG restore tham số training** — nó chỉ lo phần "thức DRAM dậy" (thoát Power-Down/Self-Refresh) và verify CRC (debug). Việc restore tham số training thật (delay/Vref) xảy ra ở **phía ATF** (`lpddr4_dynamic_config(warmboot=true)`, tài liệu 07/08/10 — nơi các hàm training đọc lại từ `train_reg`/`TRN_RSLT_REGS_t` thay vì sweep) — 2 phía (LPPP và ATF) xử lý 2 phần khác nhau của cùng 1 quá trình resume.

## Cơ chế handshake warmboot — trả lời trực tiếp "ai quyết định warmboot?"

[SOURCE FACT — `:759-789`, đã đọc toàn bộ]:
```c
void mc_configure_dram(bool warm)
{
    if (!warm) {
        syncronize_boot_get(-1, false);                       // cold boot: xoá sạch mọi cờ sync cũ
        syncronize_boot_set(BOOT_SYNC_DDR_CONT_BOOT(true));
    } else {
        syncronize_boot_get(BOOT_SYNC_DDR_FAIL(1) | BOOT_SYNC_DDR_READY(1), false);  // dọn cờ cũ còn sót
        syncronize_boot_set(BOOT_SYNC_DDR_WARMBOOT(warm));     // ← đây! set cờ mà lpddr4_config() sẽ đọc (tài liệu 14)
    }
    syncronize_boot_set(BOOT_SYNC_DDR_FREQ_VALID(true) | BOOT_SYNC_DDR_FREQ(ddr_clk));
}
```
Đây **chính xác** là nguồn của giá trị `warmboot` mà tài liệu 14 thấy `lpddr4_config()` đọc qua `syncronize_boot_get(BOOT_SYNC_DDR_WARMBOOT(1))`. ❓[UNKNOWN]: chưa xác nhận **ai gọi `mc_configure_dram(warm)` với giá trị `warm` nào** — tức là LPPP quyết định cold/warm dựa trên điều kiện gì (đọc 1 register nguồn điện? 1 flag Sleep trước đó?) — hàm này chỉ NHẬN `warm` làm tham số, không tự quyết định. Cần trace ngược lên caller của `mc_configure_dram()` (ngoài phạm vi file này) ở 1 session sau.

## `ddr_verify_crc()` — trả lời trực tiếp bug B.4 (mất dữ liệu sau khi Resume)

[SOURCE FACT — `:550-698`, đã đọc toàn bộ]: dùng **XOR DMA engine** (`mv_xor_4dma_crc32_list`) tính CRC32 của 4 vùng lớn (CH0CS0/CH0CS1/CH1CS0/CH1CS1, tự động tính địa chỉ/kích thước theo cấu hình MMAP thật đọc từ MC6). Gọi với `verify=false` **trước** khi ngủ (lưu CRC), gọi với `verify=true` **sau** khi thức (so sánh) — nếu khác, `crc_check_fails=true`, log ra `"CRC32 DDR index %d, original CRC ... != now ..."`.

⚠️ [ĐÃ XÁC NHẬN VÀ SỬA LẠI ở tài liệu 22 — Appendix B.4 đã đọc]: `ddr_verify_crc()` **KHÔNG PHẢI** công cụ/fix chính cho bug B.4. Fix thật cho B.4 là chính logic `ch0_cs1_isValid`/`mask` theo số CS thật trong `mc_enter_sr()`/`mc_enter_pd()` ở mục dưới — bug gốc là mask tính SAI khi chip chỉ có 1-CS (giả định luôn 2-CS), khiến Self-Refresh không được vào đúng, dữ liệu rò mất theo thời gian. Chi tiết đầy đủ ở tài liệu 22.

## `mc_drain()` — nghi vấn liên quan Appendix B.5 (không resume được)

[SOURCE FACT — `:490-533`]: có 1 comment lặp lại nhiều lần `/* タイムアウト対応 */` ("đối sách timeout") bọc quanh 1 bộ đếm `cnt`/`MC_STATUS_TIMEOUT=200` được **thêm vào** vòng lặp chờ "drain" (chờ Memory Controller xả hết các lệnh ghi đang chờ xử lý trước khi cho phép vào Self-Refresh):
```c
while ((status & MC_STATUS_CHX_DRAIN_IDLE_BITS) != MC_STATUS_CHX_DRAIN_IDLE_TARGET) {
    status = mrvl_regread32(&mc_reg->MC_STATUS_CH0);
    cnt++;
    if (cnt == MC_STATUS_TIMEOUT) { cnt = 0; log_status("CH0 timeout!!"); break; }   // ← thoát vòng lặp, KHÔNG treo máy vô hạn
}
```
⚠️ [ĐÃ XÁC NHẬN VÀ SỬA LẠI QUAN TRỌNG ở tài liệu 22 — Appendix B.5 đã đọc]: log `"CH0 timeout!!"`/`"CH1 timeout!!"` khớp chữ-đối-chữ với bug report — xác nhận `mc_drain()` là **nơi hiện tượng bị phát hiện**. NHƯNG root cause thật **KHÔNG nằm trong code LPDDR4 nào cả**: nguyên nhân là driver **LCDC** (điều khiển màn hình) lúc Suspend để DMA (`CFG_GRA_ENA`) bật dù đã tắt Clock, gây Read request liên tục tới DRAM → `mc_drain()` không bao giờ thấy MC idle thật → timeout → vào Self-Refresh "ép" dù chưa sạch → bị phá ngay sau đó → mất dữ liệu theo thời gian. Fix thật ở `0xc057e190` bit `CFG_GRA_ENA` (driver LCDC), ngoài phạm vi hoàn toàn của `mv_lpddr4_sleep.c`. Giả thuyết ban đầu (timeout code = fix) là **SAI** — xem tài liệu 22 để tránh lặp lại nhầm lẫn này.

# Register view

| Register/Hàm | Ý nghĩa |
|---|---|
| `mc_reg->USER_COMMAND_0` bit `0x40`/`0x80`/`0x20` | Lệnh vào/ra Self-Refresh, vào Power-Down (qua `mc_enter_sr`/`mc_exit_sr`/`mc_enter_pd`) |
| `mc_reg->DRAM_STATUS` (mask theo ch/cs) | Trạng thái xác nhận đã vào/ra SR — polling |
| `mc_reg->MC_STATUS_CH0/CH1` mask `MC_STATUS_CHX_DRAIN_IDLE_BITS` | Trạng thái "đã xả hết lệnh ghi" |
| PHY sub-reg `0xA8` bit `0x200` | Pull-down control — tắt thêm khi vào Power-Down sâu |
| `BOOT_SYNC_DDR_WARMBOOT` | Cờ đồng bộ LPPP→ATF, quyết định `warmboot` mà tài liệu 14 đọc |
| `BOOT_SYNC_DDR_CONT_BOOT` | Cờ LPPP báo cho ATF "DDR sẵn sàng, tiếp tục" |

# Source view

- `lppp_s800\Hal\quartz\asic\build\mv_lpddr4_sleep.c` — đã đọc **toàn bộ 791 dòng** (bao gồm license header)
- Đối chiếu ngược: `mv_lpddr4_apn806.c` (tài liệu 07/08/09/10/11/12) — mọi chỗ `if (warmboot==0 || skip==0)` đã trace trước đó

# Call flow — ghép 2 phía LPPP + ATF thành 1 timeline Sleep→Resume

```
[LPPP]  Quyết định đi ngủ (❓ điều kiện chưa trace)
        |
        v
mc_configure_dram(warm=true) → set BOOT_SYNC_DDR_WARMBOOT
        |
        v
mc_save_state() → [debug: ddr_verify_crc(false)] → mc_drain() → mc_enter_pd() → mc_enter_sr()
        |
        v
   [DRAM ở Self-Refresh — tự nạp điện, dữ liệu còn nguyên, SoC có thể cắt phần lớn điện khác]
        |
        v
[Sự kiện thức dậy — ❓ nguồn kích hoạt chưa trace]
        |
        v
[ATF] ble_main() → lpddr4_config() → warmboot=true (đọc lại đúng cờ đã set)
        |                                → lpddr4_dynamic_config(warmboot=true)
        |                                      → DQS Gate/Read/Write Centering/Deskew:
        |                                        SKIP sweep, đọc lại từ train_reg (tài liệu 07/08/10)
        v
[LPPP] mc_restore_state() → [debug: ddr_verify_crc(true)] → BOOT_SYNC_DDR_CONT_BOOT
        |
        v
Boot tiếp tục (U-Boot/Linux) — mục tiêu ~150ms (SPEC FACT, tài liệu 00)
```

# K-S800 customization

- `mc_drain()` với timeout `MC_STATUS_TIMEOUT=200` — ứng viên mạnh nhất tìm được cho fix bug B.5.
- `ddr_verify_crc()` — công cụ debug (không chạy mặc định, cần `power_debug_flags_get_pdf_verify_ddr()`) — ứng viên cho công cụ điều tra bug B.4.
- Cả 2 đều nằm trong file LPPP, KHÁC vị trí với phần lớn KM customization đã thấy (chủ yếu ở `atf/ble`) — cho thấy fix cho bug Sleep/Resume nằm ở đúng lớp chịu trách nhiệm (LPPP quản lý power, không phải ATF quản lý training).

# Debugging

- Nghi ngờ "Sleep xong không resume được / treo máy" → kiểm tra log `"CH0 timeout!!"`/`"CH1 timeout!!"` từ `mc_drain()` trước tiên — nếu xuất hiện thường xuyên (không phải hiếm), bug B.5 có thể chưa được giải quyết triệt để, chỉ đang "che" bằng timeout.
- Nghi ngờ "dữ liệu sai sau khi resume" (không phải crash, mà giá trị RAM khác trước) → bật cờ `power_debug_flags_get_pdf_verify_ddr()`, đọc log `ddr_verify_crc` — nếu báo `CRC32 ... != now`, xác nhận đúng bug B.4, không phải lỗi ứng dụng.
- Nếu nghi ngờ resume chạy training lại toàn bộ (chậm, không đạt ~150ms) → kiểm tra `BOOT_SYNC_DDR_WARMBOOT` có được set đúng trước khi ATF chạy không (tài liệu 14) — nếu cờ sai, `lpddr4_config()` sẽ chạy full cold-boot training dù ý định là resume.

# Summary

- Sleep = Self-Refresh (`mc_enter_sr`, JEDEC) + tắt pull-down PHY — dữ liệu KHÔNG mất, chỉ tiết kiệm điện.
- Warm Boot không train lại vì lý do KHÁC với việc dữ liệu còn nguyên — 2 khái niệm độc lập.
- Cơ chế `BOOT_SYNC_DDR_WARMBOOT` chính là cầu nối LPPP↔ATF quyết định `warmboot` — khép lại vòng tròn với tài liệu 14.
- `mc_restore_state()` (LPPP) KHÔNG restore tham số training — việc đó do ATF làm riêng (đã trace ở tài liệu 07/08/10).
- Tìm được 2 ứng viên mạnh, có tên/comment khớp cao, cho 2 bug thật trong spec Appendix B.4 và B.5 — cần xác nhận thêm bằng cách đọc chính 2 file PDF đó ở tài liệu 22.

# Verify yourself

1. Vì sao "dữ liệu không mất khi Sleep" và "không cần train lại" là 2 lý do độc lập, không phải cùng 1 lý do?
2. `mc_restore_state()` không restore tham số training — vậy nó restore/kiểm tra cái gì? Trả lời đúng bằng những gì đã đọc, không suy đoán thêm.
3. Nếu bỏ hẳn timeout trong `mc_drain()` (quay về code Marvell gốc, giả định), điều gì sẽ xảy ra nếu đúng lúc đó Memory Controller không bao giờ báo "idle"?
4. `ddr_verify_crc()` không chạy mặc định — bạn nghĩ tại sao KM không để nó chạy luôn trên mọi máy thật (gợi ý: chi phí thời gian của việc tính CRC32 trên vài GB RAM)?
5. Câu hỏi mở: ai/điều gì gọi `mc_configure_dram(warm)` và quyết định `warm=true/false`? Bạn sẽ tìm ở đâu tiếp theo (gợi ý: tên file/thư mục nào trong `lppp_s800` có khả năng chứa logic quản lý power/sleep cấp cao hơn)?

**Tiếp theo:** `16_TRAINING_RESULT.md` — `DDR_trn_result_regstructs.c/h`, `lpddr4_backup_train_result()` — chính xác THỨ GÌ được lưu, lưu ở đâu, và ai đọc lại khi warmboot=true.
