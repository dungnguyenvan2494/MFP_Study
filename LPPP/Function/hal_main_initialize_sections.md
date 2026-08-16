Đây là **prologue của C runtime** — phần việc mà bình thường `crt0.o` của toolchain làm, nhưng ở đây được viết tay để kiểm soát thứ tự và cài công tắc gỡ lỗi.

Trước khi hàm này return, **không một biến toàn cục nào trong firmware là hợp lệ**.

# Phần 1 · Bài toán nó giải: LMA ≠ VMA

Đây là gốc rễ. Nhìn vào linker script ([memory_sections.ld:230-243](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/linker_common/memory_sections.ld#L230-L243)):

```ld
.data.crc : ALIGN(8) { *(.crcinfo) } > rom
_data_crc_end = .;                        ← mốc trong FLASH

.data : AT(_data_crc_end) ALIGN(0x8) {    ← AT() = Load Memory Address
    __data_start__ = .;
    *(.data .data.* .gnu.linkonce.d.*)
    __data_end__ = .;
} > RAM                                    ← VMA = LCM
```

Chỉ thị `AT()` tách một section thành **hai địa chỉ khác nhau**:

|                       | Ký hiệu                           | Nằm ở             | Vai trò                                         |
| --------------------- | --------------------------------- | ----------------- | ----------------------------------------------- |
| **VMA** (Virtual/run) | `__data_start__` … `__data_end__` | LCM `0xE82xxxxx`  | Nơi code **truy cập** biến lúc chạy             |
| **LMA** (Load)        | `_data_crc_end`                   | QSPI `0xF8xxxxxx` | Nơi **giá trị khởi tạo** nằm trong ảnh firmware |
Compiler sinh `ldr r0, =myGlobal` với địa chỉ **VMA** (trong RAM). Nhưng linker đóng gói giá trị ban đầu vào ảnh tại **LMA** (trong flash). Giữa hai nơi đó **không có ai tự động chép** — trên hệ thống có OS thì loader làm, ở đây firmware phải tự làm.

Comment ngay trong linker script nói rõ ý đồ:

```ld
/* this gap between _etext and .data.version will be used to store the
   initialisation data of the ram section */
```

Vậy nên:

```c
fast_copy_section((uint32_t*)&_data_crc_end,    // nguồn = LMA trong flash
                  (uint32_t*)&__data_start__,   // đích  = VMA trong LCM
                  (uint32_t*)&__data_end__);    // biên  = VMA cuối
```

```
FLASH (QSPI CS1, 0xF8000000)          LCM (0xE8200000)
┌──────────────────────────┐
│ .reset                   │
│ .text                    │
│ .rodata      _etext ─────┤
│ .data.memtable           │
│ .data.version            │
│ .data.crc                │
│ ─── _data_crc_end ───────┼──┐
│ [ ảnh khởi tạo .data ]   │  │  fast_copy_section
│                          │  └──────────────────► ┌──────────────┐
└──────────────────────────┘                       │ .data (VMA)  │
                                                    ├──────────────┤
                            fast_fill_section(0) ──►│ .bss         │
                                                    └──────────────┘
```

---

# Phần 2 · Từng dòng

## `extern char` — vì sao không phải con trỏ

```c
extern char _data_crc_end;
extern char __data_start__;
...
fast_copy_section((uint32_t*)&_data_crc_end, ...)
```

Ký hiệu linker **không có giá trị, chỉ có địa chỉ**. Khai báo `extern char X` rồi lấy `&X` là cách duy nhất lấy đúng con số mà linker gán.

Nếu viết `extern uint32_t *_data_crc_end;` rồi dùng trực tiếp, compiler sẽ sinh code **đọc 4 byte tại địa chỉ đó** và coi đó là con trỏ — hoàn toàn sai. Đây là cái bẫy kinh điển khi làm việc với ký hiệu linker.

Chọn kiểu `char` vì nó không áp đặt yêu cầu căn chỉnh nào — compiler không được phép giả định `&X` chia hết cho 4, nên không sinh code tối ưu dựa trên giả định đó.

## Khai báo `extern` nằm **bên trong** hàm

Cả 5 ký hiệu và 2 prototype đều khai báo trong thân hàm, không phải ở đầu file. Cố ý: chúng chỉ có nghĩa ở đây, và đặt trong hàm ngăn phần còn lại của `hal_main.c` vô tình đụng vào biên section.

## Thứ tự hai thao tác

```c
if (*hook & 1)  fast_copy_section(...);   // ① chép .data
if (*hook & 4)  fast_fill_section(0, ...); // ② xoá .bss
```

Thứ tự không quan trọng về mặt đúng/sai vì `.data` và `.bss` là hai vùng rời nhau. Nhưng chép trước là hợp lý hơn: nếu chép sai biên và tràn sang `.bss`, bước xoá sau sẽ dọn sạch hậu quả.

---

# Phần 3 · Hai hàm assembly

## `fast_copy_section` ([hal_2nd_init_gnu.asm:48-67](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/lppp_s800/Hal/quartz/asic/build/hal_2nd_init_gnu.asm#L48-L67))

```asm
fast_copy_section:
    @ r0 = địa chỉ nguồn · r1 = địa chỉ đích · r2 = địa chỉ cuối đích
    @ all addresses must be 32bit aligned
    push    {r3}
fast_copy_section_loop:
    CMP     r1, r2              @ đặt cờ: đích đã tới cuối chưa?
    LDMLTIA r0!, {r3}           @ nếu LT: nạp 1 word từ [r0], r0 += 4
    STMLTIA r1!, {r3}           @ nếu LT: ghi 1 word vào [r1], r1 += 4
    BLT     fast_copy_section_loop
    pop     {r3}
    dsb                          @ đảm bảo mọi ghi đã ra khỏi write buffer
    isb                          @ xả pipeline — "applies to relocation"
    bx      lr
```

**`LDMLTIA` giải mã:** `LDM` + hậu tố điều kiện `LT` (less than) + chế độ `IA` (Increment After). Dấu `!` bật writeback tự tăng con trỏ.

Đây là **thực thi có điều kiện** của ARM32: cả ba lệnh trong vòng lặp cùng đọc cờ do `CMP` đặt, nên hoặc cả ba chạy hoặc cả ba bị bỏ qua. Không có nhánh rẽ ở giữa → không có branch misprediction, pipeline chạy trơn. Đây là lý do nó được gọi là "fast".

**Điều kiện dừng dựa trên địa chỉ đích**, không đếm số byte. Vòng lặp chạy đúng `(__data_end__ − __data_start__) / 4` lần.

**`dsb` + `isb` ở cuối:**

- `dsb` — chờ mọi lệnh ghi thực sự tới bộ nhớ, không còn nằm trong write buffer.
- `isb` — xả instruction pipeline. Với việc chép **dữ liệu** thì không cần, nhưng comment ghi _"applies to relocation"_: nếu ai đó dùng hàm này để chép **code** (như `flash_code_copy_to_ram` làm với `LPPP_Flash.o`), thiếu `isb` thì CPU có thể vẫn thực thi lệnh cũ đã nạp sẵn trong pipeline.

⚠️ `push {r3}` / `pop {r3}` là **thừa**. Theo AAPCS, r0–r3 là caller-saved — hàm được phép phá thoải mái. Hai lệnh này không sai nhưng vô ích.

## `fast_fill_section`

```asm
fast_fill_section:
    @ r0 = mẫu ghi · r1 = đầu vùng · r2 = cuối vùng
fast_fill_section_loop:
    CMP     r1, r2
    STMLTIA r1!, {r0}
    BLT     fast_fill_section_loop
    dsb
    bx      lr
```

Đơn giản hơn: không đọc, không cần scratch register, không cần `isb` (chỉ ghi dữ liệu). Tham số `r0` là **mẫu 32-bit** chứ không phải byte — gọi với `0` nên không phân biệt được, nhưng nếu gọi với `0xDEADBEEF` thì sẽ ghi lặp lại nguyên pattern đó.

---

# Phần 4 · Cơ chế hook — công tắc vá được lúc chạy

```c
const uint8_t initialize_sections[] = "5=initialize_sections";

void hal_main_initialize_sections(void) {
    const volatile uint8_t * hook = (const volatile uint8_t *)&initialize_sections[0];
    ...
    if (*hook & 1) fast_copy_section(...);
    if (*hook & 4) fast_fill_section(...);
}
```

Ký tự đầu là `'5'` = `0x35` = `0b0011_0101`:

|Bit|Giá trị|Việc|
|---|---|---|
|0|**1**|chép `.data` ✅|
|1|0|xoá `.fifosec` — đã comment|
|2|**1**|xoá `.bss` ✅|
|3|0|xoá free RAM — đã comment|
|4, 5|1|không dùng (chỉ là bit của ký tự `'5'`)|

Ba tính chất, mỗi cái đều bắt buộc:

**`const`** → biến nằm trong `.rodata`, tức **trong flash**, đọc được ngay lập tức. Nếu để trong `.data` thì đây là bài toán con gà–quả trứng: cần đọc cờ để biết có nên chép `.data` hay không, nhưng cờ lại nằm trong `.data` chưa được chép.

**`volatile`** → nếu thiếu, compiler thấy đây là hằng số biết trước, tự tính `0x35 & 1 = 1` rồi **xoá luôn câu `if`** — công tắc mất tác dụng, vá byte cũng vô ích.

**Dạng chuỗi có tên** → tìm được bằng `strings mvri.bin | grep initialize_sections`, rồi vá byte đầu bằng hex editor hoặc ghi đè qua JTAG. Không cần build lại, không cần console.

Mẫu này lặp lại khắp firmware như một **hệ thống feature-flag cho giai đoạn chưa có console**:

|Biến|Nơi|Điều khiển|
|---|---|---|
|`initialize_sections[]`|`hal_main.c`|chép `.data` / xoá `.bss`|
|`set_speed[]`|`app_init.c:104`|tốc độ chip lúc khởi động|
|`polling_functions[]`|`hal_cputimer.c:118`|từng hàm poll trong tick ISR|
|`dump_nvm[] = "0=dump_nvm"`|`sys_init.c:92`|dump NVM lúc boot|

---

# Phần 5 · Ba khối bị comment — và hai câu hỏi "why?"

## `.fifosec` — cố tình KHÔNG xoá

```c
/* zero out fifo section - why? there is some dedicated initialization happening. */
//if (*hook & 2) fast_fill_section(0, &__data_fifo_start__, &__data_fifo_end__);
```

Đây là **HCI FIFO chia sẻ với Linux** (`0xE8200800`, 0x1880 byte, khớp node `net-proxy-hci` trong DTS). Xoá nó lúc R4 boot sẽ **phá dữ liệu Linux đang giữ** trong kịch bản R4 reset mà AP806 không reset. Chữ "why?" cho thấy tác giả tự đặt câu hỏi rồi tự trả lời ngay: có khởi tạo riêng cho vùng này ở nơi khác.

## Free RAM — vô nghĩa và tốn thời gian

```c
/* zero out free ram - why?*/
//if (*hook & 8) fast_fill_section(0, &__free_ram_start__, &RAM_limit);
```

Vùng đó chính là heap. `SysMemory_Init()` sẽ dựng lại toàn bộ cấu trúc `BlockLink_t` ngay sau đó, nên xoá trước là thừa — và với ~26 KB thì tốn thêm thời gian boot không cần thiết.

## `.bss_in_dram` — ý tưởng bị bỏ

```c
//extern char __bss_in_dram_bss_start;
//extern char __bss_in_dram_bss_stop;
```

Từng có ý định đẩy các `.bss` lớn (pbuf pool của lwIP, buffer mạng) sang DRAM thay vì chiếm LCM 256 KB. Section tương ứng trong linker script cũng bị comment cả khối. Bị bỏ — có lẽ vì lúc `main()` chạy thì DDR chưa lên (`init_ddr()` mãi tận `early_board_init`).

---

# Phần 6 · Vùng được bảo vệ: `.data.nv`

```ld
/* do not zero out this section! */
.data.nv (NOLOAD) : { *(.nvramsec) } > RAM
```

Nằm **giữa** `.data` và `.bss` trong linker script, nên nó **ở ngoài** khoảng `[__bss_start__, __bss_end__]` → `fast_fill_section` không chạm tới. `NOLOAD` nghĩa là không có dữ liệu trong ảnh firmware, cũng không được `fast_copy_section` chép.

Kết quả: đây là **vùng sống sót qua reset mềm của R4** — giữ được trạng thái giữa hai lần boot, miễn là LCM không mất điện. Sự bảo vệ này hoàn toàn nhờ **thứ tự khai báo trong linker script**, không có cơ chế nào khác. Nếu ai đó chuyển `.data.nv` xuống sau `.bss`, nó sẽ bị xoá sạch mỗi lần boot mà không có cảnh báo nào.

---

# Phần 7 · Ràng buộc và rủi ro

## 🔴 Phụ thuộc ngầm: stack phải sẵn sàng

Hàm này là C thuần, dùng biến cục bộ `hook` và gọi hàm — nên **stack đã phải được thiết lập** trước khi vào `main()`. Việc đó do `hal_init_gnu.asm` trong section `.reset` làm (nằm ở offset 0 của ảnh firmware). Nếu stack pointer sai, hàm này crash trước khi làm được gì và không có cách nào chẩn đoán — chưa có UART.

## 🟠 Căn chỉnh 4 byte là hợp đồng ngầm

Comment assembly ghi _"all addresses must be 32bit aligned"_. Linker script đảm bảo bằng `ALIGN(0x8)` ở `.data` và `ALIGN(0x8)` ở `.bss`. Nhưng **không có kiểm tra runtime nào**. Nếu ai sửa alignment xuống dưới 4:

- Vòng lặp ghi lệch địa chỉ
- Trên Cortex-R4 với `SCTLR.A = 1` sẽ sinh **Data Abort** ngay lập tức

Và Data Abort ở giai đoạn này dẫn thẳng vào `DumpFaultRegisters` → `B ResetEntry2` → **treo vô hạn**, không log.

## 🟠 Không kiểm tra biên

Nếu vì lý do nào đó `__data_end__ < __data_start__` (linker script hỏng), điều kiện `CMP r1, r2` + `BLT` sẽ **không chạy lần nào** — im lặng bỏ qua. Ngược lại nếu `_data_crc_end` sai, hàm chép rác từ vùng flash bất kỳ vào `.data` mà không ai biết.

## 🟡 Toàn bộ thân hàm nằm trong `#ifdef __GNUC__`

Build bằng toolchain khác (ARM RVCT/Keil) thì hàm thành rỗng — vì những toolchain đó có scatter-loading riêng. `Makefile-cfg` có biến `TOOLTYPE` với `gnu` là một lựa chọn, nên đây là chuẩn bị cho khả năng đổi toolchain chứ không phải code chết.

---

# Tóm một câu

> Hàm này giải bài toán **LMA ≠ VMA**: giá trị khởi tạo của `.data` nằm trong flash ngay sau `.data.crc`, nhưng code lại truy cập chúng qua địa chỉ trong LCM — nên firmware phải tự chép sang bằng `fast_copy_section`, rồi xoá `.bss` bằng `fast_fill_section`. Hai bước đó được gác sau một **cờ hằng số trong flash mà người ta có thể vá bằng hex editor**, và vùng `.data.nv` được miễn trừ đơn thuần nhờ vị trí của nó trong linker script.