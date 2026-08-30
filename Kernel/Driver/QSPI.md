# Lộ trình học qspi

31 prompt xếp theo thứ tự phụ thuộc, bám sát đúng code trong driver QSPI của Marvell Quartz — từ thanh ghi BOOTSPI thô cho tới đường ghi flash khi kernel đang panic.

																																				**Cách dùng:** hỏi từng prompt một, không gộp. Mỗi tầng giả định bạn đã hiểu tầng trước.

**Thứ tự quan trọng:** tầng 04 (đặc thù Quartz/KM) là nơi driver này khác hẳn mọi driver QSPI upstream — đừng nhảy cóc tới đó trước khi nắm tầng 01.

**Tầng 07** là các nghi vấn tôi đã phát hiện khi đọc code. Chúng được viết dưới dạng “kiểm chứng giúp tôi” để bạn tự xác nhận, không phải kết luận có sẵn.

## Prompt tổng — nếu chỉ hỏi được một câu

Đọc toàn bộ drivers/mtd/spi-nor/mrvl-bspi_winbond.c và BOOTSPI_regheaders.h. Giải thích cho tôi theo đúng 4 tầng: 
(1) tầng thanh ghi BOOTSPI — các magic value và execute_cmd; 
(2) tầng giao thức SPI-NOR — opcode và chế độ QPI; 
(3) tầng framework — spi_nor_controller_ops, spi_nor_scan và MTD; 
(4) tầng đặc thù KM — clock R4/LPPP, write-protect, panic notifier. Với mỗi tầng: liệt kê hàm nào thuộc tầng đó, dữ liệu đi vào/ra thế nào, và điểm nào dễ gây bug nhất.


| Prompt                                                                                                                                                                    | Nội dung                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| Định vị — driver này nằm ở đâu                                                                                                                                            | Trước khi đọc code, biết nó được build ra sao và phần cứng khai báo thế nào.<br><br>Vẽ sơ đồ kiến trúc phân tầng của mrvl-bspi_winbond.c: userspace → MTD → spi-nor core → spi_nor_controller_ops → execute_cmd → thanh ghi BOOTSPI. Chỉ rõ hàm nào nằm ở tầng nào và dữ liệu biến đổi ra sao qua mỗi bước.<br><br>→ **Mở ra:** khung xương để mọi hàm còn lại có vị trí.                                                                        |
|                                                                                                                                                                           | Tìm node device tree khớp compatible "mrvl,bspi" trong arch/arm64/boot/dts/marvell/ cho board quartz. Cho tôi xem node đó và giải thích từng property: reg, interrupts, map_memory, map_memory_size, chip_select, cùng các subnode partition.<br><br>→ **Mở ra:** cấu hình phần cứng thực tế mà `qspi_probe()` đọc vào.                                                                                                                          |
|                                                                                                                                                                           | Truy vết toàn bộ qspi_probe() theo thứ tự thực thi, đánh dấu rõ: chỗ nào trả về -EPROBE_DEFER và vì sao, chỗ nào chỉ chạy cho device đầu tiên (num_devices == 0), và tại sao cuối hàm lại đi đọc RTC bằng rtc_read_time/check_default_time.<br><br>→ **Mở ra:** vòng đời khởi tạo và các phụ thuộc ẩn (SCPI, RTC).                                                                                                                               |
| Nguyên thủy phần cứng — thanh ghi BOOTSPI                                                                                                                                 | Giải thích mọi giá trị magic ghi vào thanh ghi BSCMDR trong file: 0x200, 0x300, 0x500 + byte, và giá trị 0 dùng trong get_byte(). Đối chiếu với định nghĩa trường bit trong BOOTSPI_regheaders.h, và cho biết mỗi giá trị tương ứng với hành vi gì trên đường SPI vật lý (CS, CLK, IO).<br><br>→ **Mở ra:** ngôn ngữ giao tiếp với controller — nền tảng của mọi hàm khác.                                                                       |
|                                                                                                                                                                           | Phân tích execute_cmd() từng dòng: mỗi trường BSCR (NCSMUXSEL, DUMMYCLOCKCOUNT, ADDRESSBYTECOUNT, OPCODEBYTECOUNT, INTERFACEMODE, DATAMODE, ADDRESSMODE) được đặt giá trị gì và vì sao. Giải thích giá trị khởi tạo temp = 0x70000 nghĩa là gì, và vì sao phải ghi BSCR xong mới assert CS rồi mới gửi opcode.<br><br>→ **Mở ra:** cách một lệnh SPI được “biên dịch” thành cấu hình thanh ghi.                                                  |
|                                                                                                                                                                           | wait_cmd() là vòng busy-wait không timeout trên cờ COMMANDBUSY của BSSR. Liệt kê mọi hàm gọi trực tiếp hoặc gián tiếp tới nó, rồi đánh giá: nếu phần cứng không bao giờ hạ cờ thì hệ thống treo ở đâu, watchdog có cứu được không, và có cách nào phát hiện sớm.<br><br>→ **Mở ra:** điểm treo phần cứng nguy hiểm nhất của driver.                                                                                                              |
| Giao thức flash — opcode & chế độ QPI<br><br>Con chip Winbond ở đầu kia dây nói ngôn ngữ gì.                                                                              | Lập bảng toàn bộ opcode SPI-NOR xuất hiện trong file: 0x03, 0x04, 0x05, 0x06, 0x20, 0x32, 0x35, 0x9F, 0x5A, 0x75, 0x81, 0x85, 0xB7, cùng SPINOR_OP_ENQM và SPINOR_OP_WRCR. Với mỗi opcode: tên chuẩn JEDEC, ý nghĩa, số byte địa chỉ, và hàm nào trong file dùng nó.<br><br>→ **Mở ra:** từ điển tra cứu khi soi logic analyzer.                                                                                                                 |
|                                                                                                                                                                           | Giải thích biến toàn cục qpi_mode: được bật ở đâu, tác động tới DATAMODE/ADDRESSMODE/dummy clock ra sao trong execute_cmd(), và vì sao struct op_info_s cần hai trường riêng dummy_bytes và quad_dummy_bytes. Nêu rủi ro khi qpi_mode lệch pha với trạng thái thật của chip flash.<br><br>→ **Mở ra:** nguồn gốc nhóm lỗi “đọc ra rác sau khi đổi mode”.                                                                                         |
|                                                                                                                                                                           | Trong nor_setup(), nhánh Winbond (JEDEC 0xEF) đặt read_op.command = 0x03 (đọc thường, 1 dây) nhưng program_op.command = 0x32 (Quad Input Page Program). Giải thích sự bất đối xứng này, và đối chiếu với việc mrvl_qspi_write() còn ép DATAMODE = 2 ngay trước send_byte(). So sánh thêm với nhánh Micron (0x20) dùng read 0xEB.<br><br>→ **Mở ra:** vì sao đọc chậm mà ghi nhanh, và chỗ dễ sai khi đổi chip flash.                             |
| Tích hợp framework — spi-nor & MTD<br><br>Mối nối giữa driver này và phần còn lại của kernel.                                                                             | Giải thích struct spi_nor_controller_ops trong file: spi-nor core gọi từng callback (prepare, unprepare, read_reg, write_reg, read, write, erase) vào thời điểm nào. Sau đó truy vết đầy đủ một lệnh `flash_erase /dev/mtd0 0 1` từ userspace xuống tới mrvl_qspi_erase(), đi qua những hàm nào.<br><br>→ **Mở ra:** đường đi thật của một thao tác flash, từ shell tới chân IC.                                                                 |
|                                                                                                                                                                           | Vì sao mrvl_qspi_read() thực chất chỉ gọi memcpy_fromio() từ vùng ánh xạ plat_data->base, trong khi toàn bộ đường đọc bằng lệnh SPI lại bị bọc trong #if 0? Giải thích cơ chế memory-mapped/XIP của BSPI, ai cấu hình vùng ánh xạ đó, và hệ quả khi đọc trong lúc chip đang bận ghi hoặc xóa.<br><br>→ **Mở ra:** điểm khác biệt lớn nhất so với driver QSPI thông thường.                                                                       |
|                                                                                                                                                                           | hwcaps trong qspi_probe() chỉ khai báo SNOR_HWCAPS_READ \| READ_FAST \| PP — không có cờ quad nào. Phân tích: điều này ảnh hưởng thế nào tới spi_nor_scan() và việc framework tự chọn read/program opcode, trong khi driver vẫn tự chạy quad ở tầng dưới. Đây là chủ ý hay thiếu sót?<br><br>→ **Mở ra:** sự “nói dối có chủ đích” với framework và hệ quả bảo trì.                                                                              |
| Đặc thù Quartz / KM — phần quan trọng nhất<br><br>Những thứ không có trong bất kỳ driver QSPI upstream nào. Đây là nơi bug thực địa sinh ra.                              | Giải thích disable_clk_r4(): vì sao phải hạ lõi R4/LPPP qua SCPI dvfs_set_idx(0,1) trước mọi thao tác flash, usleep_range(1000,1000) để làm gì, và điều gì xảy ra nếu bỏ qua bước này. Vì sao trên bản KM nó được chuyển vào mrvl_prepare/mrvl_unprepare thay vì gọi trực tiếp trong từng hàm write/erase?<br><br>→ **Mở ra:** ràng buộc kiến trúc quan trọng nhất — hai lõi CPU tranh nhau một con flash.                                       |
|                                                                                                                                                                           | Lập bản đồ đầy đủ các vùng write-protect: LPDDR4PARAM (0x381800–0x382000), BOOTROM (0x000000–0x300000), SECURE_AREA (0x3FD000–0x3FE000), LPPP (0x000000–0x200000). Với mỗi vùng cho biết: chip_select nào, chặn ở đường write hay erase hay cả hai, và cờ is_write_protect_lpddr4param ảnh hưởng ra sao.<br><br>→ **Mở ra:** vì sao có lúc ghi flash trả `-EIO` tưởng như vô cớ.                                                                 |
|                                                                                                                                                                           | Giải thích khối switch(c_BootType) cuối qspi_probe(): vì sao các boot type 2, 3, 5, 9, 11, 12 lại tắt write-protect. Đối chiếu với các hằng số USB_UPDATE / DEF_IISW / BOOT_ERR / DEF_UPDATE / DEF_BACKUP / DEF_RESTORE bên phía BootROM, và cho biết c_BootType được truyền từ BootROM sang kernel bằng cách nào.<br><br>→ **Mở ra:** liên kết giữa chế độ boot và quyền ghi flash.                                                             |
|                                                                                                                                                                           | Driver quản lý nhiều chip flash (chip_select 0 và 1) như thế nào? Giải thích mảng devices[NUM_BSPI_PARTS], biến đếm num_devices, vì sao chỉ device 0 mới devm_ioremap_resource cho register_base và bspi_clk, và vì sao chỉ mtd tên "spi-flash0" mới được tạo sysfs flash_size.<br><br>→ **Mở ra:** mô hình đa thiết bị chia sẻ một controller — dễ hiểu nhầm khi debug.                                                                         |
|                                                                                                                                                                           | Giải thích mrvl_qspi_set_new_detail_code(): thuật toán trích ký tự từ current->comm theo từng độ dài chuỗi (≤4, 5, 6, 7, 8, 9, >9), cách đóng gói vào detail_code 64-bit với tiền tố 0x2202300000000000, và mục đích của việc nén tên tiến trình thành đúng 4 byte. Ai đọc lại mã này?<br><br>→ **Mở ra:** cách hệ thống ghi lại “tiến trình nào đã cố ghi vào vùng cấm”.                                                                        |
| Đường khẩn cấp — panic, oops, watchdog<br><br>Code chỉ chạy khi hệ thống đang sập. Hiếm khi chạy, nhưng chạy thì rất khó gỡ.                                              | Truy vết qspi_powerdown_write(): vì sao nó bỏ qua hoàn toàn spi-nor framework và các controller_ops để tự bit-bang qua qspi_write_then_read(). So sánh với mrvl_qspi_write() về: khóa/đồng bộ, cách chờ, kiểm tra bận, xử lý clock R4, và độ an toàn khi kernel đã chết.<br><br>→ **Mở ra:** đường ghi cuối cùng còn hoạt động khi mọi thứ khác đã hỏng.                                                                                         |
|                                                                                                                                                                           | Giải thích chi tiết qspi_panic_happened(): nó ghi những cờ gì (SUPERWARP_DISABLE, KERNEL_PANIC_OCCER, PC detail code), vào các offset nào, vì sao luôn ghi hai bản BSPTHR1 (0x377000) và BSPTHR2 (0x378000), và cờ SuperWarp disable được đọc lại ở đâu trong lần boot kế tiếp. Vì sao có điều kiện valid_superwarp_session == false?<br><br>→ **Mở ra:** cơ chế hộp đen — dữ liệu sống sót qua reset.<br><br>                                   |
|                                                                                                                                                                           | Phân tích khối xử lý Suspend trong qspi_powerdown_write(): nó xử lý tình huống chip đang Erase dở như thế nào, vì sao phải mdelay(5) rồi mới quyết định là Erase chứ không phải Write, và vòng while chờ SUS bit hoạt động ra sao. So sánh qspi_oops_happened() với qspi_panic_happened() — vì sao oops ghi ít dữ liệu hơn?<br><br>→ **Mở ra:** đường code hiếm khi chạy nhưng có thể treo máy vĩnh viễn.                                        |
|                                                                                                                                                                           | qspi_watchdog_fire(), qspi_watchdog_device_info() và qspi_set_bh_b_state() được gọi từ đâu trong kernel? Với mỗi hàm cho biết: dữ liệu ghi ra flash ở offset nào, ai đọc lại, và giải thích vì sao qspi_set_bh_b_state() phải đẩy sang workqueue (THRG_setBspThrRegion) thay vì gọi qspi_powerdown_write() trực tiếp như trước.<br><br>→ **Mở ra:** các móc nối chẩn đoán nằm ngoài luồng flash thông thường.                                    |
| Debug thực chiến — triệu chứng → nguyên nhân<br><br>Dùng khi đã có log hoặc board đang lỗi trên bàn.                                                                      | Tôi thấy log "bspi block busy, aborting. RetryLoop!!" khi ghi flash. Truy nguyên: hàm nào in ra, điều kiện nào kích hoạt, timeout bao lâu (BUSY_CHK_TIMEOUT), điều gì xảy ra khi hết timeout, và liệt kê các nguyên nhân phần cứng lẫn phần mềm có thể gây ra tình trạng này.<br><br>→ **Mở ra:** triệu chứng phổ biến nhất trong log thực địa.                                                                                                  |
|                                                                                                                                                                           | Hệ thống treo cứng khi ghi hoặc xóa SPI flash. Liệt kê theo thứ tự khả năng mọi vòng lặp vô hạn trong mrvl-bspi_winbond.c có thể là thủ phạm (wait_cmd, wait_for_write_complete, vòng chờ erase, vòng chờ Suspend), kèm cách xác nhận từng cái bằng JTAG, log, hoặc logic analyzer.<br><br>→ **Mở ra:** checklist chẩn đoán treo máy, sắp theo xác suất.                                                                                         |
|                                                                                                                                                                           | Ghi vào /dev/mtdX trả về -EIO. Truy vết mọi đường trả về -EIO và -ETIMEDOUT trong file, rồi hướng dẫn cách kiểm tra nhanh qua sysfs: đọc/ghi write_protect_lpddr4param, đọc flash_size, và xác nhận chip_select nào đang bị chặn ở dải địa chỉ nào.<br><br>v→ **Mở ra:** quy trình phân biệt lỗi phần cứng với lỗi write-protect.                                                                                                                |
|                                                                                                                                                                           | Rất nhiều printk trong file bị bọc trong #ifndef CONFIG_KM_BIZHUB nên tắt hết trên bản production. Liệt kê chúng theo hàm, đề xuất cách bật lại an toàn để debug (không làm chậm đường ghi tới mức timeout), và chỉ ra log nào hữu ích nhất khi đối chiếu với logic analyzer bắt trên chân CS/CLK/IO0-3.<br><br>→ **Mở ra:** cách lấy lại tầm nhìn khi phải soi bus thật.                                                                        |
| Săn lỗi — kiểm chứng nghi vấn<br><br>Năm điểm tôi thấy đáng ngờ khi đọc code. Viết dưới dạng “kiểm chứng giúp tôi” để bạn tự xác nhận trên cây nguồn thật, không tin sẵn. | Kiểm chứng giúp tôi: mrvl_qspi_write() chặn 4 vùng (LPDDR4PARAM, BOOTROM, SECURE_AREA, LPPP) nhưng qspi_erase() dường như chỉ chặn 3 — thiếu điều kiện SECURE_AREA (0x3FD000–0x3FE000). Đọc lại code xác nhận, và nếu đúng thì phân tích hệ quả: có thể erase vùng secure boot mà không bị chặn không, và vì sao lỗi này sinh ra (xem lịch sử comment 2024/08/21).<br><br>→ **Mở ra:** lỗ hổng bảo vệ vùng secure boot ở đường erase.            |
|                                                                                                                                                                           | Kiểm chứng: qspi_watchdog_device_info(unsigned char *uc_data) dùng sizeof(uc_data) làm kích thước ghi vào flash. Trên arm64 giá trị này thực sự là bao nhiêu, có khớp ý đồ ban đầu không, caller truyền vào mảng bao nhiêu byte, và hậu quả là ghi thiếu hay ghi thừa dữ liệu chẩn đoán?<br><br>→ **Mở ra:** lỗi `sizeof`-trên-tham-số-con-trỏ kinh điển.                                                                                        |
|                                                                                                                                                                           | Kiểm chứng: reboot_worker_func() chứa BUG_ON(1) và được hẹn giờ chạy sau 1 giây (queue_delayed_work ... HZ) mỗi khi set_bh_b_state_thread() chạy. Xác nhận: khối này có nằm trong build production không (CONFIG_THREADDRV, CONFIG_KM_BIZHUB), đường nào kích hoạt tới đó, và đây có phải code test 1071435_test_BSP còn sót lại không?<br><br>→ **Mở ra:** khả năng kernel tự panic có chủ đích trong bản phát hành.                            |
|                                                                                                                                                                           | Kiểm chứng: wait_for_write_complete() có comment "// put a timeout here" nhưng vòng do/while thực tế không hề có timeout, trong khi check_busy() lại được bọc bởi BUSY_CHK_TIMEOUT. Phân tích rủi ro cụ thể (nó được gọi trong vòng lặp ghi từng page), rồi đề xuất bản vá tối thiểu, an toàn, không đổi hành vi khi flash hoạt động bình thường.<br><br>→ **Mở ra:** điểm treo dễ sửa nhất — và cách sửa mà không phá gì.                       |
|                                                                                                                                                                           | Kiểm chứng: check_jedecid() bị #ifndef CONFIG_KM_BIZHUB loại khỏi build KM, và lời gọi duy nhất tới nó lại nằm trong khối #if 0. Vậy uc_suspend_command và uc_suspend_bit có luôn bằng 0 không? Nếu có, phân tích vòng while chờ Suspend trong qspi_powerdown_write() sẽ hành xử ra sao khi điều kiện là (rx[0] & 0) == 0, và điều đó có nghĩa gì trên đường xử lý panic.<br><br>→ **Mở ra:** vòng lặp vô hạn tiềm ẩn nằm ngay trên đường panic. |

# 1. Định vị mrvl-bspi
[Định vị mrvl-bspi](https://claude.ai/code/artifact/5384801a-bd5c-4fe3-86c1-a9496ca4f6f4)
Ba câu trả lời: driver nằm ở tầng nào trong stack, device tree khai báo phần cứng ra sao, và `qspi_probe()` chạy những gì theo đúng thứ tự.

## 1.1. Kiến trúc phân tầng — và ba đường đi xuyên qua nó

Sơ đồ dưới đây là câu trả lời chính. Cột giữa là đường chuẩn (ghi/xóa) đi đủ 7 tầng. Nhưng driver này có **hai lối tắt** mà driver QSPI upstream không có: đường đọc nhảy thẳng xuống cửa sổ ánh xạ bộ nhớ, và đường panic nhảy thẳng vào tầng thanh ghi.

![[Pasted image 20260823170705.png]]

Bảy tầng từ userspace xuống chân IC. Hai lối tắt là chỗ driver này khác biệt: đường đọc không hề gửi lệnh SPI (chỉ `memcpy_fromio` từ cửa sổ ánh xạ), còn đường panic bỏ qua toàn bộ khóa, write-protect và cơ chế hạ lõi R4 để kịp ghi log trước khi máy chết.

**Hệ quả khi debug tầng 4:** `mrvl_prepare()` / `mrvl_unprepare()` là cặp bọc ngoài _mọi_ thao tác — chúng gọi `disable_clk_r4()` để hạ lõi R4/LPPP. Nghĩa là khi bạn đặt breakpoint ở tầng 5 hoặc 6, lõi R4 **đang bị tắt**. Nếu dừng quá lâu, LPPP có thể mất đồng bộ.**Hệ quả khi debug đường đọc:** vì đọc không đi qua tầng 6, logic analyzer bắt trên chân SPI sẽ **không thấy gì** khi bạn `dd` từ `/dev/mtdX` — dữ liệu ra thẳng từ cửa sổ ánh xạ. Đừng kết luận là phần cứng hỏng.

## 1.2. Device tree — hai node, một controller

Có **hai** node cùng khớp `compatible = "mrvl,bspi"`, và cả hai trỏ về **cùng một khối thanh ghi** `0x8_E8273000`. Đây chính là lý do `qspi_probe()` phải phân biệt `num_devices == 0` — chỉ device đầu tiên mới được phép `ioremap` khối thanh ghi, device thứ hai dùng lại con trỏ đó.

```
/* arch/arm64/boot/dts/marvell/quartz-sb.dtsi:181 */
/* Comment gốc: "mrvl-bspi_winbond.c đăng ký theo thứ tự,
   nên KHÔNG được đổi thứ tự qspi0 / qspi1" — thứ tự trong DTS
   quyết định device nào là devices[0].                       */

qspi0: spi-flash0@8e8273000,0 {
    compatible       = "mrvl,bspi";
    reg              = <0x8 0xe8273000 0 0x80>;   // khối thanh ghi, dùng chung
    interrupts       = <0 51 4>;                  // SPI 51, level-high
    map_memory       = <0xf8000000>;              // cửa sổ XIP
    map_memory_size  = <0x200000>;                // 2 MB
    chip_select      = <1>;                       // ← CS 1, không phải 0!
    clocks           = <&bus_clk>;                // driver KHÔNG đọc (#if 0)
    spi-max-frequency = <10000000>;               // driver KHÔNG đọc
    partitions {
        partition@0 { 
             label = "lppp-spi-firmware"; reg = <0x000000 0x200000>;         
        };
    };
};

qspi1: spi-flash1@8e8273000,1 {
    compatible       = "mrvl,bspi";
    reg              = <0x8 0xe8273000 0 0x80>;   // CÙNG địa chỉ với qspi0
    map_memory       = <0xf4000000>;
    map_memory_size  = <0x400000>;                // 4 MB
    chip_select      = <0>;                       // ← CS 0
    partitions {
        partition@0      { label = "spi-romfs"; reg = <0        0x300000>; };
        partition@300000 { label = "spi-user";  reg = <0x300000 0x100000>; };
    };
};
```

**Bẫy đặt tên:** node tên `spi-flash0` lại mang `chip_select = 1`, còn `spi-flash1` mang `chip_select = 0`. Số trong tên node **không** phải chip select. Mọi hằng số write-protect trong driver dùng _chip select_, nên khi đối chiếu log phải quy đổi.Sysfs `flash_size` chỉ được tạo khi `strstr(mtd->name, "spi-flash0")` — tức nó báo kích thước của **CS 1 (LPPP, 2 MB)**, không phải flash chứa romfs.


| Property            | Giá trị                       | Ai thực sự đọc                                        | Dùng làm gì                                                                                                                      |
| ------------------- | ----------------------------- | ----------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| `reg`               | `0x8_E8273000, 0x80`          | `platform_get_resource()` → `devm_ioremap_resource()` | Ánh xạ khối thanh ghi **BOOTSPI**. Chỉ `ioremap` một lần cho device đầu.                                                         |
| `interrupts`        | `<0 51 4>`                    | `platform_get_irq()` → `devm_request_irq()`           | Đăng ký `irq_handler`, nhưng handler chỉ `printk` rồi `return 0` (= `IRQ_NONE`). Driver thực tế chạy hoàn toàn bằng **polling**. |
| `map_memory`        | `0xF800_0000` / `0xF400_0000` | `of_property_read_u32()` → `devm_ioremap()`           | Cửa sổ ánh xạ **XIP**. Đây là **nguồn dữ liệu duy nhất của đường đọc**.                                                          |
| `map_memory_size`   | `0x20_0000` / `0x40_0000`     | `of_property_read_u32()`                              | Xác định kích thước cửa sổ ánh xạ, sau đó gán vào `devices[n].size`.                                                             |
| `chip_select`       | `1` / `0`                     | `of_property_read_u32()`                              | Nạp vào trường `NCSMUXSEL` của `BSCR` trong `execute_cmd()`. Mặc định `3` nếu thiếu; từ chối nếu `> 3`.                          |
| `partitions`        | `fixed-partitions`            | `mtd_device_parse_register()`                         | Tạo `/dev/mtdN`. Bảng phân vùng chính là **bản đồ write-protect**.                                                               |
| `clocks`            | `&bus_clk`                    | **Không ai**                                          | Khối `clk_get` bị `#if 0` với ghi chú _“không dùng được”_. Property này là **di sản**.                                           |
| `spi-max-frequency` | `10 MHz`                      | **Không ai**                                          | Không có trong driver và cũng không được SPI-NOR core đọc. Tần số thực do **bootloader/SCP** đặt.                                |

Phần đắt giá nhất: **bảng phân vùng trong DT khớp chính xác với các hằng số write-protect hard-code trong driver.** Đây là mảnh ghép giúp bạn đọc được ý nghĩa của mọi địa chỉ magic trong file.

### Phân vùng DT ↔ hằng số write-protect ↔ vùng ghi log

|Vùng flash|CS|Địa chỉ|Khai báo ở DT|Hằng số trong driver|
|---|--:|---|---|---|
|LPPP firmware|1|`0x000000–0x200000`|partition `lppp-spi-firmware` (toàn bộ chip)|`LPPP_START/END_ADDR`|
|BootROM / romfs|0|`0x000000–0x300000`|partition `spi-romfs`|`BOOTROM_START/END_ADDR`|
|Vùng ghi log BSP/Thread|0|`0x377000` & `0x378000`|nằm trong `spi-user`|`BSPTHR1/BSPTHR2` (panic, watchdog)|
|Tham số LPDDR4|0|`0x381800–0x382000`|nằm trong `spi-user`|`LPDDR4PARAM_START/END_ADDR`|
|Secure boot|0|`0x3FD000–0x3FE000|||
**Cùng một vùng, hai cách đánh địa chỉ.** Đường bình thường đi qua MTD nên dùng offset _tương đối trong partition_: `THRG_getBspThrRegion(0x46, …)` đọc tại `0x77000 + 0x46` trong `spi-user` (hằng số `THRD_BspThrBackupArea1Adress = 0x00077000` ở `mtd_flash_access.c:962`). Đường panic bỏ qua MTD nên phải tự cộng base: `0x300000 + 0x77000 + …`. Cùng byte, hai công thức — rất dễ nhầm khi so địa chỉ trong log.

## 1.3.Truy vết `qspi_probe()` theo thứ tự thực thi

Chạy **hai lần** — một lần cho mỗi node DT. Nhãn cho biết bước nào chạy lần nào.

|#|Phạm vi|Thao tác / điều kiện|Kết quả / ý nghĩa|
|--:|---|---|---|
|1|Mỗi lần probe|`deferget_scpi_ops() == NULL`|`return -EPROBE_DEFER`. SCPI là mailbox tới SCP/R4; không có SCPI thì không thể `disable_clk_r4()`, nên không an toàn để giao dịch flash. Đây là lý do duy nhất trả `DEFER`.|
|2|Mỗi lần probe|`nor_num = of_get_child_count(dev->of_node)`|Tính số child nhưng **không được sử dụng**. Di sản từ driver gốc `fsl-qspi`.|
|3|**Device 0 / Device 1**|Device 0: `platform_get_resource()` → `devm_ioremap_resource()` → `register_base`.Device 1: `register_base = devices[0].register_base`|Hai node DT cùng trỏ `0x8_E8273000`, nên chỉ ánh xạ register block một lần và dùng chung.|
|4|Mỗi device|Đọc `map_memory` + `map_memory_size` → `devm_ioremap()` cửa sổ XIP|CS1: `0xF800_0000`, 2 MB. CS0: `0xF400_0000`, 4 MB. Thiếu property → `return -1`.|
|5|**Device 0**|`devm_request_mem_region(0xE824_0000, 0x100)` → `bspi_clk`|Ánh xạ thanh ghi clock. Tuy nhiên code reset R4 qua `bspi_clk` đã bị `#if 0`; hiện dùng SCPI. Biến được gán nhưng **không còn được đọc**.|
|6|**Device 0**|`platform_get_irq()` → `devm_request_irq(irq_handler)`|Đăng ký IRQ 51. `irq_handler()` chỉ log `"Got a bspi interrupt"` rồi trả `0` (`IRQ_NONE`). Driver thực tế **polling qua `wait_cmd()`**.|
|7|Mỗi device|Đọc `chip_select`, mặc định `3`; từ chối nếu `> 3`|Giá trị đi vào `NCSMUXSEL` trong `BSCR` khi `execute_cmd()`. Log `"Chip select %d"` là dấu hiệu đáng tin để xác định node đang probe.|
|8|Mỗi device|`disable_clk_r4(true)`|Hạ lõi R4/LPPP để tạo **cửa sổ độc quyền flash**. Thất bại → log `"Failed to talk to the quartz qspi part"` → `goto irq_failed`.|
|9|Mỗi device|`nor_setup()` → `readid()` → opcode `0x9F`, đọc 20 byte JEDEC|`0x20` → Micron; `0xEF` → Winbond. Sau đó điền `op_info_s` cho read/program/erase/WE/WDE. ID không khớp → bảng opcode để trắng, thao tác sau có thể gửi opcode `0`.|
|10|Mỗi device|`spi_nor_scan(nor, NULL, &hwcaps)`|`hwcaps` chỉ có `READ \| READ_FAST \| PP`, không khai báo quad flag dù tầng dưới vẫn chạy quad. Framework quét SFDP, xác định size và đặt `mtd->name`, ví dụ `8e8273000.spi-flash0`.|
|11|Mỗi device|Gán lại `nor / mtd / dev / priv / controller_ops`|5 dòng gán lại giống trước khi `spi_nor_scan()`. Mang tính phòng thủ, thực tế không cần thiết.|
|12|Mỗi device|`mtd_device_parse_register()`|Đọc DT `partitions` và tạo `/dev/mtdN`. Nếu thất bại → `return -1`; **R4 vẫn đang tắt** vì chưa chạy bước 13.|
|13|Mỗi device|`disable_clk_r4(false)`|Bật/trả lõi R4/LPPP về trạng thái ban đầu, kết thúc cửa sổ độc quyền flash.|
|14|Mỗi device|`mrvl_qspi_read(..., buffer, 0, 32)`|Đọc thử 32 byte đầu. Dữ liệu không được sử dụng; đây là **smoke test XIP** để phát hiện ánh xạ cửa sổ sai.|
|15|Mỗi device|`dev_set_drvdata()` + tạo sysfs `flash_size` nếu tên chứa `"spi-flash0"`|Ví dụ: `/sys/devices/platform/quartz-sb/8e8273000.spi-flash0/flash_size`. Tên/path khác giữa kernel 5.10.205 và 4.4.8 (`spi0` ở bản cũ), nên userspace script phụ thuộc path có thể bị vỡ.|
|16|Mỗi device|`switch(c_BootType)` → `is_write_protect_lpddr4param`|Boot ty|

**Vì sao cuối hàm probe lại đi đọc RTC?** Vì đây là một vòng phụ thuộc ngược. `check_default_time()` (ở `drivers/rtc/class.c:170`) khôi phục thời gian bằng cách gọi `THRG_getBspThrRegion(0x46, …)` và `(0x4F, …)` — hai bản backup thời gian (định kỳ và FAX) nằm **trong SPI flash**. Nhưng driver RTC khởi tạo _trước_ QSPI, nên lúc RTC probe thì flash chưa đọc được, việc khôi phục thất bại âm thầm.Vì thế QSPI, ngay khi vừa có `/dev/mtdN`, tự chạy lại phép kiểm tra: đọc giờ hiện tại từ RTC, so với bản backup trong flash, nếu RTC bị _lùi về quá khứ_ (mất pin, lỗi tinh thể) thì `rtc_set_time()` khôi phục từ flash và dựng cờ `0x4D` báo cho tầng ứng dụng. Comment gốc trong code nói thẳng: “vì RTC init chạy trước QSPI init”.

**Hai điểm mong manh đáng kiểm chứng ở bước cuối.**Một: `rtc_tmp` chỉ được gán trong `rtc_register_device()`. Nếu vì lý do nào đó RTC probe _sau_ QSPI, `rtc_tmp` vẫn là `NULL` và `rtc_read_time(NULL, &tm)` sẽ deref null ngay trong probe. Không có kiểm tra NULL nào ở đây.Hai: khối này chạy ở **cả hai** lần probe, nên `check_default_time()` thực thi hai lần liên tiếp — lần hai đọc lại flash và có thể ghi lại cờ. Vô hại nhưng lãng phí, và làm log khó đọc vì mọi thứ xuất hiện nhân đôi.


# 2. Nguyên thủy phần cứng — thanh ghi BOOTSPI
[Nguyên thủy BOOTSPI](https://claude.ai/code/artifact/d5d70610-ec75-4da2-8fa2-9e5c787431df)
Bốn giá trị magic, một thanh ghi cấu hình, và một vòng lặp không timeout — ba thứ này là toàn bộ nền tảng mà mọi hàm còn lại trong driver dựng lên trên.

## 2.1. BSCMDR — bốn giá trị, một ngôn ngữ

BSCMDR chỉ có **hai trường thật**. Mọi giá trị magic trong driver đều là tổ hợp của hai trường này — không có gì bí ẩn khi bạn nhìn đúng bit map.

![[Pasted image 20260823172643.png]]

Chỉ có 3 bit mã lệnh và 8 bit dữ liệu. Header do RegBuild sinh ra **không liệt kê** ý nghĩa từng giá trị COMMAND — bảng dưới là suy ra từ cách driver dùng và từ comment trong code.

|Giá trị `COMMANDDATAH`|Hành vi trên dây|Mô tả|Dùng ở đâu|
|--:|---|---|---|
|`0x200`|**Kéo nCS xuống thấp**|Chưa có xung CLK nào. Mở khung giao dịch.|`execute_cmd()` — đúng một lần, ngay sau khi ghi `BSCR`|
|`0x500 + byte`|**Phát 1 byte ra**|8 xung CLK ở chế độ đơn (IO0), hoặc **2 xung** ở chế độ quad (IO0–3).|`send_byte()`, `send_addr()`, và opcode trong `execute_cmd()`|
|`0x000`|**Nhận 1 byte**|Controller tự phát 8 xung CLK (2 nếu quad) và lấy mẫu đường vào. Byte nhận nằm trong `BSSR[7:0]`.|`get_byte()` — chỉ một chỗ duy nhất|
|`0x300`|**Thả nCS lên cao**|Đóng khung giao dịch, chốt lệnh vào chip flash.|Mọi caller, sau khi xong — `execute_cmd()` **không tự làm**|
Ghép lại, một giao dịch “đọc thanh ghi trạng thái” (`qspi_read_reg(nor, 0x05, buf, 1)`) trông như thế này trên dây:

![[Pasted image 20260823172722.png]]

Chú ý `execute_cmd()` chỉ lo tới hết pha opcode. Pha địa chỉ, pha dữ liệu và việc thả nCS đều do caller tự làm — đó là lý do mọi hàm gọi `execute_cmd()` đều phải tự kết thúc bằng `regs->BSCMDR = 0x300`.

### Mẹo trong `get_byte()`: một lần đọc, hai công dụng

```c
uint32_t wait_cmd(BOOTSPI_REGS_t *regs) {
    uint32_t temp;
    while (((temp = regs->BSSR) & BOOTSPI_BSSR_COMMANDBUSY_MASK) == ...);
    return temp;                // ← trả về CHÍNH lần đọc BSSR cuối cùng
}

uint8_t get_byte(BOOTSPI_REGS_t *regs) {
    unsigned char temp;
    wait_cmd(regs);              // đợi lệnh trước xong
    regs->BSCMDR = 0;            // COMMAND=0 → phát 8 xung, lấy mẫu đường vào
    temp = wait_cmd(regs);       // đợi xong — và LẤY LUÔN dữ liệu
    temp &= 0xff;                // thừa: temp đã là unsigned char, đã bị cắt
    return(temp);
}
```

BSSR có **hai trường chồng lên nhau về mục đích**: bit 8 là `COMMANDBUSY`, bit [7:0] là `DATA`. Cùng một lần đọc thanh ghi vừa cho biết “lệnh đã xong” vừa mang theo byte nhận được. Đó là lý do `wait_cmd()` trả về `uint32_t` thay vì `void` — thiết kế cố ý, không phải thừa.**Ở chế độ QPI, “8 xung” thành “2 xung”.** Bốn đường IO chạy song song nên một byte chỉ tốn 2 chu kỳ clock. Bằng chứng trong chính file: khối `#if 0` của `mrvl_qspi_read()` tính `quad_dummy_bytes / 2` (“QPI: 2clock = 1byte”) so với `dummy_bytes / 8` (“normal SPI: 8clock = 1byte”).**Các mã COMMAND 1, 4, 6, 7 không bao giờ được dùng.** Header không liệt kê chúng, nên không thể biết chúng làm gì nếu không có datasheet Quartz. Đừng thử đoán.

## 2.2. `execute_cmd()` — biên dịch một lệnh SPI thành thanh ghi

BSCR ở offset 0x0, 11 trường. Driver chạm vào 7 trong số đó.

BSCR — offset 0x0 [đọc/ghi] · chỉ vẽ bit 18…0, bit 31:19 là reserved

![[Pasted image 20260823172916.png]]

![[Pasted image 20260823173014.png]]

Bảy trường được ghi mỗi lần gọi. `READMODEBYTECOUNT` và `ERRONMISS` luôn bị đặt về 0 — kể cả khi BSCR trước đó có giá trị khác

### Giải mã `temp = 0x70000` — và tại sao nó gây hiểu nhầm

`0x70000` = `0x40000 | 0x20000 | 0x10000` = **CACHEENABLE = 1** và **NCSMUXSEL = 3**. Nhưng ngay dòng sau, `NCSMUXSEL` bị ghi đè bằng `chip_select` thật. Vậy ý nghĩa _còn sót lại_ duy nhất của hằng số này là **bật bit CACHEENABLE và xóa sạch mọi trường khác về 0**. Giá trị 3 trong NCSMUXSEL chỉ là rác vô hại — nó khiến người đọc tưởng có chủ đích trong khi không hề.

**Câu hỏi mở đáng kiểm chứng.** `execute_cmd()` ép `CACHEENABLE = 1` ở mọi lệnh, kể cả lệnh Erase và Page Program. Driver **không hề** làm động tác vô hiệu hóa cache nào sau khi ghi/xóa. Vì đường đọc lại là `memcpy_fromio()` qua cửa sổ XIP có cache, về lý thuyết có nguy cơ **đọc ra dữ liệu cũ** ngay sau khi ghi. Cần xác nhận với datasheet Quartz xem phần cứng có tự invalidate khi command-mode ghi hay không. Nếu không, đây là một lớp bug rất khó tái hiện.

|Trường|Bit|`execute_cmd()` đặt|Ý nghĩa|Tác dụng trong **command mode**?|
|---|--:|--:|---|---|
|`CACHEENABLE`|18|`1` (từ `0x70000`)|Bật cache cho đường đọc ánh xạ bộ nhớ.|**Có lẽ không** — chủ yếu ảnh hưởng XIP|
|`NCSMUXSEL`|17:16|`chip_select`|Chọn chân `nCS` sẽ được kéo xuống. 2 bit → giải thích việc driver từ chối `chip_select > 3`.|**Chắc chắn có**|
|`DUMMYCLOCKCOUNT`|15:12|`dummy_bytes` hoặc `quad_dummy_bytes`|Số **xung clock** rỗng giữa địa chỉ và dữ liệu; 4 bit → tối đa 15.|**Có lẽ không** — xem ghi chú liên quan|
|`READMODEBYTECOUNT`|11:10|`0` (mặc định)|Số byte `read mode` trong chuỗi đọc tự động, đi cùng `BSCOM.READMODE`.|**Không**|
|`ADDRESSBYTECOUNT`|9:8|`addr_bytes`|Mã hóa kiểu **N+1**: giá trị `2` → 3 byte địa chỉ, `3` → 4 byte. Comment trong `nor_setup()` xác nhận cách mã hóa này.|**Có lẽ không**|
|`OPCODEBYTECOUNT`|6:5|`op_bytes` (luôn `1`)|Số byte opcode cho chuỗi đọc tự động. Trong command mode, driver tự phát đúng **1 byte opcode**.|**Không**|
|`ADDRESSMODE`|4|`0` = đơn, `1` = quad|Chọn pha địa chỉ truyền trên 1 đường hay 4 đường.|**Chắc chắn có**|
|`DATAMODE`|3:2|`0` = đơn, `2` = quad|Chọn số đường truyền dữ liệu. Là trường quan trọng nhất; `mrvl_qspi_write()` còn ép lại thành `2` ngay trước `send_byte()` để chạy Quad Page Program.|**Chắc chắn có**|
|`ERRONMISS`|1|`0` (mặc định)|Sinh lỗi bus khi đọc trượt. Driver không bật trường này.|**Không**|
|`INTERFACEMODE`|0|`1`|`0` = memory-mapped/XIP; `1` = manual command mode. Là công tắc chuyển giữa hai chế độ.|**Chắc chắn có**|
**Bẫy đặt tên: `dummy_bytes` không phải byte, mà là xung clock.** Trường phần cứng tên `DUMMYCLOCKCOUNT`, còn trường trong `struct op_info_s` lại tên `dummy_bytes`. Trong `nor_setup()`, Micron đặt `dummy_bytes = 8` và `quad_dummy_bytes = 10` — đó là **8 và 10 xung clock**. Chính khối `#if 0` trong `mrvl_qspi_read()` xác nhận: nó chia cho 8 (SPI đơn) hoặc chia cho 2 (QPI) để ra _số byte_. Nếu bạn từng thắc mắc “sao dummy tận 10 byte” thì đó là lý do.**Vì sao nói ba trường “có lẽ không tác dụng”.** Header ghi rõ về BSCOM: _“dùng để lập trình một phần chuỗi lệnh sẽ gửi ra chip SPI khi **đọc trực tiếp từ bộ nhớ**”_. Nhóm trường đếm byte/clock nhiều khả năng phục vụ cùng bộ tuần tự tự động đó. Ở command mode, phần mềm tự bơm từng byte qua BSCMDR nên các bộ đếm này không còn ai dùng. Bằng chứng: khối `#if 0` của đường đọc phải **tự tay** gọi `get_byte()` đúng số lần dummy — nếu phần cứng tự chèn dummy theo BSCR thì đã không cần.

### Vì sao phải theo đúng thứ tự BSCR → nCS → opcode

```c
    regs->BSCR = temp;              // (1) chọn CS nào, chạy mấy đường IO
    wait_cmd(regs);
    regs->BSCMDR = 0x200;           // (2) kéo nCS xuống
    wait_cmd(regs);
    regs->BSCMDR = 0x500 + cmd->command;   // (3) phát opcode
    wait_cmd(regs);
    return qpi_mode;               // giá trị trả về KHÔNG caller nào dùng
```

Ba lý do, mỗi lý do khóa chặt một bước:

**(1) trước (2)** — `NCSMUXSEL` quyết định _chân nCS nào_ được kéo xuống. Nếu đảo thứ tự, bạn sẽ assert CS của con flash **sai** rồi mới sửa lại — tức là đã nhá một xung CS giả vào chip kia. Với chip đang bận Erase, một xung CS lạ có thể phá giao dịch của nó.**(2) trước (3)** — đây là luật của chính giao thức SPI: nCS phải xuống thấp _trước_ cạnh clock đầu tiên. Phát opcode khi nCS còn cao thì chip flash không hề nghe thấy.**`wait_cmd()` chen giữa mọi bước** — header nói thẳng: _“chỉ nên ghi BSCMDR khi bit CommandBusy đã sạch”_. Ghi đè lên một lệnh đang chạy thì lệnh mới bị nuốt mất, và bạn sẽ thấy triệu chứng “thiếu mất một byte” rất khó truy.

### **Ba thanh ghi driver không bao giờ chạm tới — và một trong số đó có thể là thủ phạm treo máy.

**`BSACTIVE` (offset 0x20): header ghi rằng chân SPI khi reset ở trạng thái **tri-state**, và bit `BSPIACTIVE` phải bằng 1 trước khi dùng COMMAND mode. Phần cứng tự set bit này khi có ai đọc vùng nhớ BootSPI — vì U-Boot đã boot từ chính con flash này nên nó vốn đã bằng 1. Nhưng driver **không kiểm tra, không set**. Nếu vì lý do nào đó bit này bị xóa (đường nguồn, pinmux, firmware SCP), command mode sẽ ra lệnh vào hư không và `COMMANDBUSY` có thể không bao giờ hạ. Đây là thứ đầu tiên nên đọc khi gặp treo ở `wait_cmd()`.`BSTR` (offset 0x24, thanh ghi timing) và `BSIEN` (offset 0x10, cho phép ngắt) cũng không bao giờ được ghi. `BSIEN` không bao giờ bật giải thích vì sao `irq_handler()` chỉ in log rồi trả `IRQ_NONE` — ngắt DATAREADY chưa từng được bật. **Nếu bạn thực sự thấy dòng “Got a bspi interrupt” trong log, nghĩa là có thứ khác ngoài driver này đã bật ngắt.**

## 2.3. `wait_cmd()` — bán kính sát thương
Một vòng `while` ba dòng, không timeout, nằm dưới đáy mọi thứ.

```c
uint32_t wait_cmd(BOOTSPI_REGS_t *regs)
{
    uint32_t temp;
    while(((temp = regs->BSSR) & BOOTSPI_BSSR_COMMANDBUSY_MASK)
                              == BOOTSPI_BSSR_COMMANDBUSY_MASK);   // ← không lối thoát
    return temp;
}
```

Không có biến đếm, không có `udelay`, không có `WARN`, không có đường thoát. Nếu phần cứng không hạ cờ, hàm này quay mãi. Vấn đề không phải bản thân vòng lặp — mà là **ai đứng phía trên nó**.

![[Pasted image 20260823173159.png]]

Ba nhóm điểm vào dồn vào cùng một phễu. Hậu quả thì khác nhau hoàn toàn — và trường hợp C là tệ nhất, vì đó chính là đường mà hệ thống trông cậy để tự cứu.

### Ai gọi tới `wait_cmd()`

|Hàm|Số lần / lượt|Gọi gián tiếp qua đó|
|---|--:|---|
|`execute_cmd()`|**3**|`check_busy` · `qspi_read_reg` · `qspi_write_reg` · `readid` · `qspi_write_volatile_config` · `qspi_write_then_read` · `mrvl_qspi_write` · `mrvl_qspi_erase`|
|`get_byte()`|**2 mỗi byte**|`qspi_read_reg` · `readid` (×20 byte) · `check_busy` · `qspi_write_then_read`|
|`send_byte()`|**1 mỗi byte**|`qspi_write_reg` · `mrvl_qspi_write` (tối đa 256 byte/page) · `qspi_write_then_read`|
|`send_addr()`|**1 mỗi byte địa chỉ**|`mrvl_qspi_write` · `mrvl_qspi_erase`|
|`mrvl_qspi_write()`|**1**|Được gọi trực tiếp sau khi thả CS của `write_enable`|
|`mrvl_qspi_erase()`|**2**|Được gọi trực tiếp|
|`qspi_write_volatile_config()`|**1**|Hiện **không có caller**|

**Một ngoại lệ quan trọng:** `mrvl_qspi_read()` trên bản KM **không** chạm tới `wait_cmd()` — nó chỉ là `memcpy_fromio()`. Nghĩa là **đường đọc miễn nhiễm** với lỗi treo này. Nếu máy vẫn đọc được flash mà ghi/xóa thì treo, đó là bằng chứng rất mạnh chỉ thẳng vào `wait_cmd()`.Ngược lại, riêng một lần ghi 256 byte đã gọi `wait_cmd()` khoảng **270 lần**. Xác suất trúng một lần cờ kẹt tỉ lệ thuận với lượng dữ liệu ghi — vì thế lỗi hay xuất hiện khi cập nhật firmware chứ không phải lúc chạy bình thường.

### Watchdog có cứu được không?
Tôi đã tra `km_mvebu_v8_lsp_defconfig`. Câu trả lời ngắn: **gần như không** — và ở trường hợp tệ nhất thì watchdog chính là nạn nhân.

|Cơ chế|Cấu hình|Bắt được không?|Nhận xét|
|---|---|---|---|
|`SOFTLOCKUP_DETECTOR`|**is not set**|❌ **Không**|Đây là cơ chế vốn có thể bắt busy-loop trong context tiến trình, thường log `BUG: soft lockup — CPU#N stuck`. Nhưng hiện **đang tắt**.|
|`DETECT_HUNG_TASK`|**is not set**|❌ **Không**|Kể cả bật cũng **không phù hợp**: hung-task phát hiện task ở trạng thái `D` (uninterruptible sleep), trong khi busy-loop là trạng thái `R`.|
|`WQ_WATCHDOG`|**is not set**|❌ **Không**|Không có cơ chế giám sát workqueue bị đình trệ.|
|`SOFT_WATCHDOG`|`=y`|⚠️ **Không bắt được trường hợp B**|Dựa trên hrtimer và cần userspace ping `/dev/watchdog`. Nếu timer vẫn chạy và process vẫn sống/ping đều thì **không fire**. Nếu fire thì lại đi vào vòng xử lý watchdog tương ứng.|
|**CA72 WDT (SCPI)**|**Có**, qua R4/SCP|⚠️ **Có khả năng**|Đây mới là watchdog độc lập với CA72. Tuy nhiên `mrvl_prepare()` gọi `disable_clk_r4(true)` và tắt R4 trong suốt giao dịch flash → cần **kiểm chứng watchdog còn được clock/chạy khi R4 bị dừng hay không**.|
|**Watchdog phần cứng ARM**|SP805 / SBSA / SMC đều **not set**|❌ **Không**|Không có driver watchdog phần cứng ARM tương ứng được bật trong kernel.|
**Đây là kết luận quan trọng nhất của cả tầng 01.** Đường phục hồi của hệ thống là `softdog_reboot()` tại `drivers/watchdog/softdog.c:207`, và nó gọi `qspi_watchdog_fire()` _trước_ `emergency_restart()`. Chuỗi đó chui thẳng vào `qspi_powerdown_write()` → `execute_cmd()` → `wait_cmd()`. Nếu nguyên nhân khiến watchdog fire lại chính là BOOTSPI hỏng, thì lệnh reboot sẽ không bao giờ chạy tới.Cùng bẫy đó cũng nằm ở `drivers/base/power/main.c:1074` — `qspi_watchdog_device_info()` được gọi trong đường suspend/resume thiết bị.

### Phát hiện sớm — bốn cách theo thứ tự thực dụng

**1 · Đọc BSSR từ một core khác.** Đây là arm64 SMP: một core kẹt trong `wait_cmd()` vẫn để các core khác chạy bình thường. BSSR ở địa chỉ vật lý `0x8E827300C` (base `0x8_E8273000` + offset `0xC`) và là thanh ghi **chỉ đọc** nên tra cứu an toàn. Bit 8 vẫn là 1 → xác nhận phần cứng thật sự không hạ cờ, chứ không phải lỗi logic phần mềm. Đọc luôn `BSACTIVE` ở `0x8E8273020` — nếu bit 0 bằng 0 thì bạn đã tìm ra nguyên nhân gốc.**2 · `cat /proc/<pid>/stack`** của tiến trình kẹt. Sẽ thấy ngay chuỗi `wait_cmd → execute_cmd → mrvl_qspi_write`. Rẻ và nhanh nhất nếu hệ thống còn shell.**3 · Bật `CONFIG_SOFTLOCKUP_DETECTOR` trên bản build phát triển.** Nó sẽ in cảnh báo kèm stack trace sau ~20 giây thay vì treo im lặng. Không nên bật trên bản phát hành nếu chưa đánh giá tác động, nhưng trên bàn debug thì đây là công tắc đáng giá nhất.**4 · JTAG** — đọc PC. Nếu nó quay trong 3 lệnh của `wait_cmd()` thì không cần đoán thêm.

### Bản vá tối thiểu

Không đổi kiểu trả về (mọi caller đang bỏ qua giá trị trả về), không làm chậm đường chạy bình thường. Chỉ biến một cú treo vĩnh viễn thành một lỗi có ghi log:

```c
uint32_t wait_cmd(BOOTSPI_REGS_t *regs)
{
    uint32_t temp;
    int spins   = 10000;      // quay nhanh trước — lệnh bình thường xong trong vài chục vòng
    int timeout = 100000;     // rồi mới đếm giờ: 100000 × 1us = 100 ms

    while (((temp = regs->BSSR) & BOOTSPI_BSSR_COMMANDBUSY_MASK)
                                 == BOOTSPI_BSSR_COMMANDBUSY_MASK) {
        if (spins) { spins--; continue; }        // pha 1: không udelay, không chậm đi
        if (--timeout < 0) {                        // pha 2: có giới hạn
            WARN_ONCE(1, "bspi: COMMANDBUSY ket, BSSR=0x%08x BSACTIVE=0x%08x\n",
                      temp, regs->BSACTIVE);
            break;
        }
        udelay(1);
    }
    return temp;
}
```

**Vì sao chia hai pha.** Một lệnh BSCMDR bình thường chỉ tốn 8 xung clock — dưới một micro giây. Nếu thêm `udelay(1)` ngay từ vòng đầu, mỗi byte ghi sẽ chậm thêm ít nhất 1 µs, và một trang 256 byte gọi `wait_cmd()` khoảng 270 lần → chậm đi hàng trăm micro giây mỗi trang. Pha quay nhanh giữ nguyên tốc độ đường thường; pha đếm giờ chỉ kích hoạt khi đã thật sự bất thường.**`WARN_ONCE` chứ không phải `printk`:** hàm này có thể chạy trong ngữ cảnh panic/hrtimer, nơi in log lặp lại sẽ làm ngập console và có thể tự gây treo tiếp.**Vẫn còn thiếu:** bản vá này để caller chạy tiếp với dữ liệu rác. Bản sửa đầy đủ cần đổi chữ ký để báo lỗi lên trên — nhưng đó là thay đổi lan rộng khắp file, nên hãy làm sau khi đã có log xác nhận đúng nguyên nhân.

# 3. Giao thức flash — opcode & chế độ QPI
[Ngôn ngữ trên dây](https://claude.ai/code/artifact/0ba64f1d-523f-416a-a09e-2c7afbe327e9)
Danh sách opcode trong file và danh sách opcode thật sự chạy trên dây là **hai danh sách khác nhau**. Chênh lệch giữa chúng giải thích gần như mọi hiểu nhầm về đường đọc của driver này.

**Kết luận trước, chứng minh sau.** Ba điều tôi xác minh được trên cây nguồn, và cả ba đều ngược với những gì đọc code driver sẽ khiến bạn tin:

① `read_op.command = 0x03` trong `nor_setup()` **không bao giờ ra tới dây**. Opcode đọc thật là `0xEB` — Fast Read Quad I/O — do **U-Boot** nạp vào `BSCOM`, kernel không hề đụng tới.
② Biến `qpi_mode` **không bao giờ thành `true`**. Không một dòng nào trong toàn bộ cây nguồn gửi opcode `0x38`.
③ Nhưng flash **vẫn chạy quad** — cả khi đọc lẫn khi ghi. Chỉ là bằng cơ chế khác, không qua `qpi_mode`.

## 3.1. Từ điển opcode — chia theo “có ra dây hay không”

Sắp xếp theo cách hữu ích khi cầm logic analyzer, không theo thứ tự số.

#### Nhóm A — thật sự chạy trên board Winbond
#### Bảy opcode bạn sẽ thật sự bắt được
Đây là toàn bộ những gì kernel driver phát ra qua BSCMDR trên cấu hình hiện tại.

|**Opcode**|**Tên JEDEC**|**Ý nghĩa**|**Byte địa chỉ**|**Nơi dùng trong file**|
|---|---|---|---|---|
|`0x9F`|**RDID**|Đọc JEDEC ID, trả 20 byte. Byte 0 = nhà sản xuất (`0xEF` Winbond), byte 1 = loại, byte 2 = dung lượng.|0|`read_id` → `readid()` → `nor_setup()`. **Opcode đầu tiên** chạy sau khi kernel lên.|
|`0x06`|**WREN**|Write Enable. Dựng bit WEL trong SR-1. Bắt buộc trước _mỗi_ lệnh ghi hoặc xóa, và tự xóa sau khi lệnh xong.|0|`write_enable` — `mrvl_qspi_write()` (mỗi trang), `mrvl_qspi_erase()`|
|`0x04`|**WRDI**|Write Disable. Xóa bit WEL. Về mặt kỹ thuật là thừa vì WEL tự xóa, nhưng driver gọi để đóng chốt an toàn.|0|`write_disable` — cuối `mrvl_qspi_write()` và `mrvl_qspi_erase()`|
|`0x05`|**RDSR1**|Đọc Status Register-1. **Bit 0 = BUSY/WIP** (đang ghi/xóa), bit 1 = WEL. Opcode chạy **nhiều nhất** trong toàn hệ thống.|0|`check_busy()`, `wait_for_write_complete()`, vòng chờ trong `mrvl_qspi_erase()`, `SPI_CMD_RDSR` trong `qspi_powerdown_write()`|
|`0x20`|**SE / BE_4K**|Sector Erase **4 KiB**. Đây là lý do `CONFIG_MTD_SPI_NOR_USE_4K_SECTORS=y` trong defconfig có ý nghĩa.|3|`erase_op` nhánh Winbond → `mrvl_qspi_erase()`|
|`0x32`|**PP_1_1_4**|**Quad Input Page Program.** Opcode và địa chỉ đi 1 đường, dữ liệu đi 4 đường. Tối đa 256 byte mỗi lệnh.|3|`program_op` — cả nhánh Winbond lẫn Micron → `mrvl_qspi_write()`|
|`0x02`|**PP**|Page Program thường, 1 đường. Chậm hơn `0x32` nhưng không cần bit QE — an toàn hơn khi hệ thống đang sập.|3|`SPI_CMD_PP` — **chỉ** trong `qspi_powerdown_write()` (đường panic/watchdog)|

**Vì sao đường panic dùng `0x02` chứ không phải `0x32`?** `0x32` chỉ hoạt động khi bit QE (Quad Enable) trong SR-2 đang bật. Ở thời điểm kernel panic, không ai dám giả định trạng thái thanh ghi cấu hình của chip còn nguyên vẹn. `0x02` chạy trên một đường IO duy nhất và không phụ thuộc bit nào — chậm hơn 4 lần nhưng gần như không thể sai. Với 5 byte cờ cần ghi thì tốc độ hoàn toàn không quan trọng.

#### Nhóm B — có trong file nhưng không bao giờ ra dây
|**Opcode**|**Tên**|**Ý nghĩa**|**Xuất hiện ở**|**Vì sao không chạy**|
|---|---|---|---|---|
|`0x03`|**READ**|Read Data, 1 đường, không dummy. Chậm nhất trong mọi lệnh đọc.|`read_op.command` nhánh Winbond|Đường đọc là `memcpy_fromio()`. Driver **không bao giờ** nạp `read_op` vào `BSCOM` — hai dòng làm việc đó đều bị comment (dòng 847 và 901).|
|`0x38`|**ENQM**|Enter QPI Mode. Chuyển toàn bộ giao thức — kể cả opcode — sang 4 đường.|Chỉ trong `if (opcode == SPINOR_OP_ENQM)`|Grep toàn cây: chỉ có **2** lần xuất hiện — định nghĩa macro và điều kiện `if` này. **Không ai gửi nó.**|
|`0x31`|**WRCR / WRSR2**|Ghi Status Register-2 (chứa bit QE).|Chỉ trong `if` ở `qspi_write_reg()`|SPI-NOR core dùng `spi_nor_write_sr(nor, sr_cr, 2)` — tức opcode **`0x01`** ghi 2 byte, không phải `0x31`.|
|`0x5A`|**RDSFDP**|Đọc bảng tham số SFDP của chip.|`discovery_param`, đặt trong `nor_setup()`|Hàm duy nhất dùng nó — `qspi_read_discovery()` — nằm trong khối `#ifndef CONFIG_KM_BIZHUB`, và lời gọi cũng đã bị comment.|
|`0x35`|**RDCR / RDSR2**|Đọc Status Register-2. Bit 1 = QE, bit 7 = SUS.|`uc_readstatus_command` trong `check_jedecid()`|`check_jedecid()` bị `#ifndef CONFIG_KM_BIZHUB` loại khỏi build, và lời gọi duy nhất nằm trong `#if 0`.|
|`0x75`|**EPS**|Erase/Program Suspend của Winbond.|`uc_suspend_command`, cùng chỗ với `0x35`|Cùng lý do. **Hệ quả nguy hiểm:** biến vẫn bằng 0, nên đường panic sẽ gửi opcode `0x00` — xem prompt 31 ở lộ trình.|
|`0xB7`|**EN4B**|Vào chế độ địa chỉ 4 byte (cho flash > 16 MB).|Xử lý đặc biệt trong `qspi_write_reg()`|Cả hai chip trên board đều ≤ 4 MB, địa chỉ 3 byte thừa sức. Core không có lý do gửi.|
|`0x81 / 0x85`|**WRVCR / RDVCR**|Ghi/đọc Volatile Configuration Register — **lệnh riêng của Micron**, không tồn tại trên Winbond.|`qspi_write_volatile_config()`|Hàm này **không có caller nào** trong toàn file.|
|`0x70`|**RDFSR**|Read Flag Status Register — cũng là lệnh riêng Micron.|Nhánh `#else` của vòng chờ trong `mrvl_qspi_erase()`|Nhánh non-KM. Bản KM dùng `0x05` bit 0 thay thế.|
|`0xD8`|**SE**|Sector Erase 64 KiB.|`erase_op` nhánh Micron (`0x20/0xBA`)|Board dùng Winbond (`0xEF`) → nhánh Micron không chạy.|
|`0xEB`|**READ_1_4_4**|Fast Read Quad I/O.|`read_op.command` nhánh **Micron** trong kernel|Nhánh Micron không chạy. **Nhưng** — xem nhóm C, opcode này lại là opcode đọc thật.|

**Vì sao có nhiều code chết đến vậy.** Makefile của `drivers/mtd/spi-nor/` chỉ biên dịch `mrvl-bspi_winbond.c` khi `CONFIG_KM_BIZHUB=y`; ngược lại nó build `mrvl-bspi.c`. Nghĩa là **mọi khối `#ifndef CONFIG_KM_BIZHUB` bên trong file này chưa từng được biên dịch một lần nào**. Chúng không phải “code cho cấu hình khác” — chúng là hóa thạch. Ít nhất một khối trong đó thậm chí **lệch dấu ngoặc** và sẽ không compile được (xem chỗ xử lý `SPINOR_OP_ENQM` — nhánh `#else` mở hai ngoặc nhọn nhưng chỉ đóng một). Đừng dùng chúng làm tài liệu tham khảo.

#### Nhóm C — ra dây nhưng không nằm trong driver kernel

Đây là nhóm hay bị bỏ sót nhất. Cửa sổ ánh xạ XIP có một **bộ tuần tự phần cứng** tự phát lệnh đọc, và nó được cấu hình bởi `BSCOM` + `BSCR` — mà kernel không hề ghi. Cấu hình đó là di sản U-Boot để lại:

```c
/* BootROM/…/drivers/spi/qspi_winbond.c:1344-1370 — nhánh Winbond 0xEF */
plat_data->read_op.command     = 0xeb;   // Fast Read Quad I/O
plat_data->read_op.addr_bytes  = 3;
plat_data->read_op.dummy_bytes = 4;      // 4 xung dummy
plat_data->read_op.read_mode_byte_count = 1;
plat_data->read_op.address_mode = 1;     // địa chỉ đi 4 đường
plat_data->read_op.data_mode    = 2;     // dữ liệu đi 4 đường

temp = BOOTSPI_BSCR_DUMMYCLOCKCOUNT_REPLACE_VAL(temp, 4);
temp = BOOTSPI_BSCR_ADDRESSMODE_REPLACE_VAL(temp, 1);
temp = BOOTSPI_BSCR_DATAMODE_REPLACE_VAL(temp, 2);
temp = BOOTSPI_BSCR_INTERFACEMODE_REPLACE_VAL(temp, 0);   // chế độ ánh xạ bộ nhớ
regs->BSCR  = temp;
regs->BSCOM = BOOTSPI_BSCOM_OPCODE_REPLACE_VAL(temp, 0xeb);   // ← opcode đọc THẬT
```

**Vậy khi bạn `dd if=/dev/mtd1`, analyzer sẽ thấy `0xEB`, không phải `0x03`.** Và nếu bạn đang debug bằng cách sửa `read_op.command` trong kernel driver thì sẽ không có gì thay đổi — chỗ cần sửa nằm ở U-Boot, hoặc bạn phải bỏ comment hai dòng `BSCOM` trong `nor_setup()`.Với nhánh Micron, U-Boot đặt `BSCOM = 0x0B` (Fast Read, 1 đường, 8 dummy) — chậm hơn hẳn. Đây là chi tiết đáng nhớ nếu board nào đó dùng flash Micron.

## 3.2. `qpi_mode` — biến không bao giờ đúng
Và vì sao flash vẫn chạy quad dù biến này luôn `false`.

### Chỗ duy nhất gán giá trị
```c
/* mrvl-bspi_winbond.c:329 — trong qspi_write_reg() */
    if (opcode == SPINOR_OP_ENQM)      // 0x38
    {
        qpi_mode = true;
        printk("enable quad mode\n");
    }
```

Đây là **chỗ duy nhất** trong toàn bộ cây nguồn gán `qpi_mode = true`. Để nó chạy, phải có ai đó gọi `write_reg(nor, 0x38, …)`. Tôi truy ba tầng và không tầng nào gửi:

|**Tầng kiểm chứng**|**Kiểm tra**|**Kết quả**|
|---|---|---|
|**Toàn cây nguồn**|`grep -rn SPINOR_OP_ENQM`|Đúng **2** kết quả: định nghĩa macro ở `spi-nor.h:52`, và điều kiện `if` ở dòng 329. Không có nơi phát.|
|**spi-nor core**|`spi_nor_quad_enable()` ở `core.c:2930`|Thoát ngay nếu `read_proto` và `write_proto` đều không rộng 4 đường. Driver khai `hwcaps` chỉ có `READ \| READ_FAST \| PP` — toàn 1-1-1 → **hàm return 0 ngay lập tức**.|
|**Nếu có chạy**|`spi_nor_sr2_bit1_quad_enable()`|Cũng không gửi `0x38` — nó gọi `spi_nor_write_sr(nor, sr_cr, 2)`, tức opcode **`0x01`**.|
|**U-Boot**|`qspi_winbond.c:1306`|Nhánh `case 0x60` (chip báo đang ở QPI mode) nằm trong `#if 0` với ghi chú tiếng Nhật: **“**_|
### Hệ quả: nhánh nào chết, nhánh nào sống

```c
if (qpi_mode) {                                     // ← luôn false, nhánh này CHẾT
        temp = ...DATAMODE_REPLACE_VAL(temp, 2);
        temp = ...ADDRESSMODE_REPLACE_VAL(temp, 1);
        temp = ...DUMMYCLOCKCOUNT_REPLACE_VAL(temp, cmd->quad_dummy_bytes);
} else {                                            // ← luôn chạy
        temp = ...DATAMODE_REPLACE_VAL(temp, 0);     // 1 đường
        temp = ...ADDRESSMODE_REPLACE_VAL(temp, 0);  // 1 đường
        temp = ...DUMMYCLOCKCOUNT_REPLACE_VAL(temp, cmd->dummy_bytes);
}
```

Vì `qpi_mode` luôn `false`, trường `quad_dummy_bytes` trong `struct op_info_s` **chưa từng được đọc tới** trên bản KM. Nó vẫn được điền cẩn thận trong `nor_setup()` (Micron: 10, Winbond: 0) nhưng không đi tới đâu.**Vậy vì sao struct cần hai trường?** Vì cùng một opcode cần số xung dummy _khác nhau_ tùy chế độ. Ví dụ `0xEB` trên Micron cần 8 xung ở SPI thường nhưng 10 xung ở QPI — không phải do chip đổi ý, mà vì ở QPI cả opcode và địa chỉ cũng đi 4 đường nên thời điểm chip bắt đầu trả dữ liệu dịch đi. Thiết kế hai trường là **đúng**; chỉ là một nửa của nó chưa bao giờ được kích hoạt.**Nhắc lại bẫy từ tầng 01:** tên trường là `dummy_bytes` nhưng đơn vị là **xung clock**. Trường phần cứng tên đúng: `DUMMYCLOCKCOUNT`.

### Phân biệt hai khái niệm hay bị nhập làm một

|**Tiêu chí**|**Chế độ QPI (`qpi_mode`)**|**Lệnh quad trong SPI thường**|
|---|---|---|
|**Opcode đi mấy đường**|**4 đường**|1 đường|
|**Bật bằng cách nào**|Gửi `0x38`, chip đổi trạng thái **toàn cục** và ở lì đó|Không cần bật gì — chỉ cần bit QE trong SR-2, rồi dùng opcode phù hợp|
|**Rủi ro**|**Cao.** Nếu CPU reset mà chip vẫn ở QPI, mọi lệnh 1 đường sau đó đều bị hiểu sai → không boot được|Thấp. Mỗi lệnh tự mang theo thông tin về lane|
|**Hệ thống này dùng**|**Không** — cố ý loại bỏ ở cả U-Boot lẫn kernel|**Có** — `0xEB` khi đọc, `0x32` khi ghi|
**Rủi ro khi `qpi_mode` lệch pha với chip — vẫn còn thật, dù biến luôn false.** Rủi ro không nằm ở chiều “biến true mà chip không QPI” (không thể xảy ra), mà ở chiều ngược lại: **chip đang ở QPI mà driver tưởng không**. Kịch bản: một lần chạy trước đó (công cụ nạp firmware, JTAG script, hay firmware SCP) đã gửi `0x38` cho chip; chip nhớ trạng thái đó qua reset mềm của CPU. Khi kernel lên, nó gửi `0x9F` trên 1 đường, chip đang nghe 4 đường → đọc ra ID rác → `nor_setup()` không khớp nhánh nào → **toàn bộ bảng `op_info_s` để trắng, mọi opcode sau đó bằng 0**. Triệu chứng là log `readid` ra giá trị vô nghĩa và không có dòng `"winbond part"`.Chính U-Boot có chỗ nhận biết tình huống này — `case 0x60` là ID mà Winbond trả về _khi đang ở QPI mode_ — nhưng nhánh đó đã bị `#if 0`. Nghĩa là hệ thống **không có đường phục hồi** nếu chip lỡ mắc kẹt ở QPI.

## 3.3. Bất đối xứng đọc/ghi — và một tiền đề sai

Đọc **không** chậm hơn ghi. Ngược lại là đằng khác.

Nhìn `nor_setup()` thì tưởng đọc dùng `0x03` (1 đường, chậm) còn ghi dùng `0x32` (4 đường, nhanh) — một sự bất đối xứng kỳ lạ. Nhưng như nhóm C ở trên đã chỉ ra, `0x03` không bao giờ ra dây. Ba giao dịch thật sự trên board trông như sau:

![[Pasted image 20260824122611.png]]

Ô đặc = đường IO đang mang dữ liệu, ô nét đứt = rảnh. Đọc dùng 4 đường ở _cả_ pha địa chỉ lẫn pha dữ liệu; ghi chỉ dùng 4 đường ở pha dữ liệu; xóa không dùng đường nào ngoài IO0.

### Kỹ thuật lật `DATAMODE` giữa giao dịch

Đây là chỗ khéo nhất của driver, và cũng là chỗ dễ phá nhất nếu ai đó “dọn dẹp” code:

```c
execute_cmd(regs, &plat_data->program_op, cs);   // BSCR ← 0x70000…, DATAMODE = 0
                                                 //   → opcode 0x32 ra trên 1 đường ✓
send_addr(regs, addr, program_op.addr_bytes + 1);// DATAMODE vẫn 0
                                                 //   → 3 byte địa chỉ ra trên 1 đường ✓
regs->BSCR = BOOTSPI_BSCR_DATAMODE_REPLACE_VAL(regs->BSCR, 2);
                                                 // ← đọc-sửa-ghi BSCR khi nCS ĐANG thấp
send_byte(regs, &buffer[off], xfer_size);        //   → dữ liệu ra trên 4 đường ✓
regs->BSCMDR = 0x300;                            // thả nCS
```

**Vì sao phải lật thủ công.** Ở command mode, phần mềm tự bơm từng byte qua `BSCMDR`. Controller không có cách nào biết byte nào là “địa chỉ”, byte nào là “dữ liệu” — nó chỉ thấy một chuỗi lệnh COMMAND=5 giống hệt nhau. Vì thế `DATAMODE` là **công tắc lane duy nhất còn hiệu lực**, và muốn giao thức 1-1-4 thì phải tự bật công tắc đúng thời điểm. Trường `ADDRESSMODE` không giúp được gì ở đây — nó phục vụ bộ tuần tự đọc tự động.

**Điều này củng cố kết luận ở tầng 01:** trong command mode, các trường đếm byte của BSCR (`ADDRESSBYTECOUNT`, `OPCODEBYTECOUNT`, `READMODEBYTECOUNT`) gần như chắc chắn vô tác dụng — nếu chúng có tác dụng thì `ADDRESSMODE` cũng phải có, và khi đó không cần trò lật `DATAMODE` này.

**Nếu ai đó gom dòng lật vào trong `execute_cmd()` cho “gọn”**, opcode 0x32 sẽ bị phát trên 4 đường. Chip đang nghe 1 đường sẽ hiểu thành một opcode hoàn toàn khác — rất có thể là một lệnh xóa. Đây là loại “refactor vô hại” có thể phá hỏng flash.

### So sánh hai nhánh chip trong `nor_setup()`

|**Tiêu chí**|**Winbond — `0xEF` đang chạy**|**Micron — `0x20` không chạy**|
|---|---|---|
|**Nhận diện**|`byte[1] = 0x40` (SPI thường) hoặc `0x60` (QPI)|`byte[1] = 0xBA`; `byte[2] = 0x19` → chuyển sang địa chỉ 4 byte|
|**`read_op`**|`0x03`, dummy 0, `quad_dummy` 0 — **toàn bộ là dữ liệu chết**|`0xEB`, dummy 8, `quad_dummy` 10 — đầy đủ và hợp lý hơn|
|**`program_op`**|`0x32`, 3 byte địa chỉ|`0x32`, 3 hoặc 4 byte địa chỉ|
|**`erase_op`**|`0x20` — **4 KiB**|`0xD8` — **64 KiB**|
|**size**|Không đặt — comment nói rõ “lấy từ FDT vì mỗi chip select một giá trị”|`0x80000000` (2 GB) — **giá trị vô lý**, không chip SPI-NOR nào lớn vậy|
|**Chờ xóa xong**|Poll `0x05` bit 0, có `usleep_range(900,1000)`|Nhánh `#else`: poll `0x70` bit 7 — **code chết**|

**Bốn thứ sẽ vỡ nếu bạn đổi chip flash.** Đây là danh sách kiểm tra thực dụng, không phải lo xa:

① **Kích thước sector.** `erase_op.command` phải khớp với `mtd->erasesize` mà spi-nor core suy ra từ bảng chip. Đặt `0x20` (4K) trong khi core nghĩ là 64K thì mỗi lần xóa sẽ chỉ xóa 1/16 vùng cần xóa — và **không hề báo lỗi**. Dữ liệu cũ còn sót lại sẽ AND với dữ liệu mới.

② **Chuỗi nhận diện.** `nor_setup()` chỉ có 2 nhánh. Chip lạ rơi vào `default: break;` → bảng `op_info_s` để trắng → mọi opcode bằng `0x00`, mọi `addr_bytes` bằng 0. Không có `printk` cảnh báo nào ở nhánh mặc định của Winbond.

③ **Cấu hình XIP nằm ở U-Boot.** Đổi chip mà chỉ sửa kernel là vô nghĩa — `BSCOM` và số xung dummy do `qspi_winbond.c` bên BootROM quyết định. Chip mới cần số dummy khác thì phải sửa cả hai nơi.

④ **Bit QE.** Cả `0xEB` lẫn `0x32` đòi bit QE trong SR-2. Không ai trong hệ thống này bật nó — nó được giả định đã bật sẵn từ nhà máy hoặc từ công cụ nạp. Chip mới xuất xưởng với QE = 0 sẽ đọc ra rác ngay từ lệnh đầu tiên, và không có đường nào tự sửa.

# 4. Tích hợp framework — spi-nor & MTD

Ba câu trả lời đều xoay quanh cùng một trục: driver này **giả vờ** nói chuyện với spi-nor core theo đúng khuôn mẫu chuẩn, nhưng ở những chỗ quan trọng nhất, nó lặng lẽ đi vòng qua framework bằng đường riêng.

## 4.1. Truy vết `flash_erase` — và một khám phá: hai lần WREN
Từ ioctl tới chân IC, đi qua đúng 6 file.

`struct spi_nor_controller_ops` có 7 callback. Bản header trong cây này đã **rút gọn** so với upstream — không còn tham số `enum spi_nor_ops` (loại thao tác) trong `prepare`/`unprepare`, chỉ còn con trỏ `nor`. Ba callback bắt buộc: `read_reg`, `write_reg`, `read`, `write`; `erase` là **tùy chọn** — nếu driver không cung cấp, core tự gửi opcode xóa qua `write_reg()`. Driver này cung cấp, và chính lựa chọn đó tạo ra khám phá bên dưới.

`struct spi_nor_controller_ops` có 7 callback. Bản header trong cây này đã **rút gọn** so với upstream — không còn tham số `enum spi_nor_ops` (loại thao tác) trong `prepare`/`unprepare`, chỉ còn con trỏ `nor`. Ba callback bắt buộc: `read_reg`, `write_reg`, `read`, `write`; `erase` là **tùy chọn** — nếu driver không cung cấp, core tự gửi opcode xóa qua `write_reg()`. Driver này cung cấp, và chính lựa chọn đó tạo ra khám phá bên dưới.

![[Pasted image 20260824123746.png]]

Đường màu đồng (accent) là lời gọi thật sự băng qua ranh giới core/driver. Đường vàng đánh dấu chỗ **trùng lặp**: WREN được gửi hai lần theo hai cơ chế khác nhau cho _cùng một_ giao dịch xóa, và việc chờ bận cũng vậy.

|**Callback**|**Core gọi khi nào**|**Bắt buộc?**|
|---|---|---|
|**`prepare`**|Đầu `spi_nor_lock_and_prep()` — trước **mọi** thao tác đọc/ghi/xóa/khóa, ngay sau `mutex_lock`. **Ngoại lệ:** đường đọc của bản KM không gọi hàm này (xem prompt 12).|Không — tùy chọn|
|**`unprepare`**|Đầu `spi_nor_unlock_and_unprep()`, trước `mutex_unlock`. Luôn đi cặp với `prepare`.|Không — tùy chọn|
|**`read_reg`**|Mọi nơi core cần đọc một thanh ghi trạng thái/cấu hình: `RDSR` (chờ bận), `RDCR`, `RDSR2`, `RDID` (nhận diện chip trong `spi_nor_scan`).|**Có** — `spi_nor_check()` từ chối nếu thiếu|
|**`write_reg`**|`WREN`, `WRDI`, `WRSR`, `WRSR2`, `CHIP_ERASE`, và — nếu driver **không** cung cấp `erase` — cả opcode xóa sector.|**Có**|
|**`read`**|`spi_nor_read_data()` — mọi lần `mtd->_read` chạy.|**Có** (khi không dùng spi-mem)|
|**`write`**|`spi_nor_write_data()` — mỗi trang trong `spi_nor_write()`.|**Có**|
|**`erase`**|`spi_nor_erase_sector()` — **thay hẳn** đường generic (WREN qua `write_reg` + gửi opcode xóa qua `write_reg` + poll qua `read_reg`) bằng một lệnh gọi duy nhất, driver tự lo từ đầu đến cuối.|Không — nhưng khi có, **toàn quyền**|

**Vì sao có WREN kép mà hệ thống vẫn chạy đúng?** WREN không cộng dồn — gửi hai lần chỉ đơn thuần là gửi thừa một lệnh vô hại (bit WEL vẫn là 1). Cái giá phải trả không phải là lỗi chức năng, mà là **thời gian**: mỗi lần xóa một sector 4 KiB tốn thêm một giao dịch WREN đầy đủ (8 xung mở CS + 8 xung opcode + đóng CS) và một lượt poll RDSR thừa — trên một flash 4 MB với sector 4K, đó là **1024 lần xóa thừa nhỏ** nếu xóa toàn chip. Không đáng kể so với thời gian erase vật lý (hàng chục ms/sector), nhưng là dấu hiệu rõ ràng rằng hai tầng code được viết **độc lập**, không tầng nào biết tầng kia cũng đang tự lo phần việc giống mình.

**Gốc rễ:** API `controller_ops.erase` theo thiết kế được kỳ vọng là “giao thức thấp” (driver chỉ phát đúng opcode xóa mà core yêu cầu), nhưng driver này biến nó thành “giao dịch trọn gói” (tự WREN, tự chờ). Core không có cách nào biết driver đã làm vậy để bỏ bớt các bước generic của chính nó.

## 4.3. Đường đọc XIP — và một race có thật giữa hai chip select

`mrvl_qspi_read()` chỉ là `memcpy_fromio()`. Câu hỏi đúng không phải “vì sao nhanh” mà là “ai đảm bảo an toàn”.

BOOTSPI có một chế độ thứ hai ngoài command-mode: khi `BSCR.INTERFACEMODE = 0`, controller tự biến thành một **bộ giải mã địa chỉ trong suốt** — bất cứ khi nào CPU đọc một địa chỉ vật lý trong cửa sổ đã cấu hình, phần cứng tự phát ra đúng chuỗi opcode+địa chỉ+dummy đã lập trình sẵn trong `BSCOM`/`BSCR` và trả dữ liệu về như thể đó là RAM thật. Đây là lý do `BSACTIVE` (tầng 01) nói rõ pin SPI phải “active” trước khi CPU có thể fetch code từ chính con flash này lúc mới cấp điện — **toàn bộ U-Boot đã chạy bằng cơ chế này** trước khi kernel tồn tại.

![[Pasted image 20260824123837.png]]

Kernel chưa từng tự tay bật quad-read hay chọn opcode 0xEB — nó thừa hưởng một cấu hình phần cứng do U-Boot để lại và không bao giờ đụng tới. Nếu ai đó thay driver flash bên U-Boot mà quên đồng bộ, đường đọc kernel sẽ vẫn “chạy” nhưng đọc sai dữ liệu — vì opcode/dummy không khớp chip mới.

#### Một chi tiết cài đặt mong manh, hiện tại vẫn an toàn

```c
of_property_read_u32(node, "map_memory", (u32 *)&devices[num_devices].base);
devices[num_devices].base = devm_ioremap(dev, (resource_size_t)devices[num_devices].base, map_size);
```

Trường `.base` là `void*` (8 byte trên arm64), nhưng dòng đầu ép kiểu thành `u32*` để nhét địa chỉ vật lý 32-bit vào — chỉ ghi **4 byte thấp**, để nguyên 4 byte cao. Dòng sau đọc lại chính field đó làm input cho `ioremap`, rồi ghi đè bằng con trỏ ảo kết quả. An toàn _hiện tại_ vì `devices[]` là mảng toàn cục (BSS, tự động = 0) và mỗi ô chỉ dùng đúng một lần trong đời driver — nhưng đây là kiểu code sẽ vỡ ngay nếu ai đó thêm một đường re-probe hoặc dùng lại slot mà không zero hóa trước.

### Hệ quả khi đọc trong lúc chip đang bận ghi hoặc xóa

**Trong cùng một chip select**: an toàn tuyệt đối, nhưng không phải nhờ phần cứng — nhờ `nor->lock`. Cả đường đọc (`mutex_lock` trực tiếp) lẫn đường ghi/xóa (`mutex_lock` qua `spi_nor_lock_and_prep`) đều tranh chấp **cùng một mutex**. Một `dd` đọc `/dev/mtd0` trong khi `flash_erase /dev/mtd0` đang chạy sẽ đơn giản là **bị chặn** ở `mutex_lock` cho tới khi lệnh xóa xong toàn bộ.

Nhưng có **hai chip select trên cùng một khối thanh ghi vật lý**, và đây là chỗ mọi thứ đổi khác:

![[Pasted image 20260824123915.png]]

Không có bằng chứng đây từng gây sự cố thực tế — có thể vì hai chip select hiếm khi bị truy cập đồng thời trong vận hành bình thường (CS1/LPPP chủ yếu chỉ được ghi lúc cập nhật firmware). Nhưng về mặt logic, đây là một race điều kiện thật, không phải suy diễn.

**Vì sao đáng lo hơn một race thông thường.** Hai độc giả tưởng driver này “giống spi-nor bình thường, chỉ khác mỗi transport” — nhưng thực tế mỗi node DT tạo ra một `struct spi_nor` riêng, hoàn toàn không biết tới sự tồn tại của node kia. spi-nor core được thiết kế với giả định **một mutex bảo vệ một chip vật lý** — giả định đó bị phá vỡ khi hai instance chia sẻ cùng một controller vật lý mà driver không tự thêm khóa cấp-controller nào để bù lại.

**Thực dụng khi debug:** nếu bạn gặp dữ liệu đọc sai/rác chỉ xảy ra khi có cập nhật firmware LPPP (CS1) chạy đồng thời với truy cập vào romfs/user (CS0) — hoặc ngược lại — đây là nghi phạm số một, đứng trước cả nghi ngờ phần cứng.

## 4.3. `hwcaps` thiếu cờ quad — im lặng có chủ đích

Không phải thiếu sót. Là một cách né framework mà không cần sửa nó.

```c
/* qspi_probe() — mrvl-bspi_winbond.c */
const struct spi_nor_hwcaps hwcaps = {
    .mask = SNOR_HWCAPS_READ | SNOR_HWCAPS_READ_FAST | SNOR_HWCAPS_PP,
};                                            // không một bit quad nào
ret = spi_nor_scan(nor, NULL, &hwcaps);
```

Cơ chế bên trong `spi_nor_default_setup()` (đã xác minh ở `core.c:2641`) rất đơn giản: `shared_mask = hwcaps->mask & params->hwcaps.mask` — phép AND giữa **“controller khai báo hỗ trợ gì”** (tham số ở trên) và **“chip thật sự hỗ trợ gì”** (đọc ra từ bảng nhận diện JEDEC/SFDP). Tôi đã kiểm chứng bảng đó — `winbond.c` khai nhiều dòng flash Winbond với cờ `SPI_NOR_QUAD_READ`, tức **chip có thể hỗ trợ quad**. Vậy vế thứ hai của phép AND không phải là 0.

![[Pasted image 20260824123957.png]]

Chip có khả năng quad — bảng JEDEC xác nhận. Chỉ vì driver không khai nó trong `hwcaps`, spi-nor core tự giới hạn mình xuống 1 đường, dù chưa từng hỏi phần cứng thật sự làm được gì.

### Hai hệ quả — một vô hại, một then chốt

|**Trường bị ảnh hưởng**|**Giá trị core chọn**|**Có ai dùng không?**|
|---|---|---|
|`nor->read_opcode`|`0x0B` (Fast Read, 1 đường)|**Không** — `grep "nor->read_opcode"` trong file driver trả về **0 kết quả**. Đường đọc thật là `memcpy_fromio()`, chưa từng hỏi trường này.|
|`nor->program_opcode`|`0x02` (Page Program, 1 đường)|**Không** — `mrvl_qspi_write()` dùng `plat_data->program_op.command` (bảng riêng của driver, = `0x32`), không đụng tới trường này.|
|`nor->read_proto` / `write_proto`|`SNOR_PROTO_1_1_1` (độ rộng 1)|**Có — đây là chỗ then chốt.** `spi_nor_quad_enable()` kiểm tra chính hai trường này.|

```c
/* core.c:2930 */
static int spi_nor_quad_enable(struct spi_nor *nor)
{
    if (!nor->params->quad_enable) return 0;

    if (!(spi_nor_get_protocol_width(nor->read_proto) == 4 ||
          spi_nor_get_protocol_width(nor->write_proto) == 4))
        return 0;                          // ← dừng ở đây, luôn luôn

    return nor->params->quad_enable(nor);   // không bao giờ tới lượt chạy
}
```

Vì `shared_mask` không có bit quad, `read_proto`/`write_proto` có độ rộng 1 — hàm trả về 0 ngay ở điều kiện thứ hai. **Core không bao giờ tự ý gửi `WRSR2` để bật bit QE, cũng không bao giờ gửi `ENQM (0x38)`.** Đây chính là mảnh ghép còn thiếu của phát hiện ở tầng 02 (`qpi_mode` luôn `false`) — giờ đã rõ nguyên nhân gốc: không phải vì thiếu sót ở một chỗ, mà là **ba lớp phòng thủ đồng thời** chặn mọi con đường có thể vô tình bật quad-toàn-cục.

**Chủ ý hay thiếu sót? Bằng chứng nghiêng hẳn về chủ ý.** Nếu là thiếu sót đơn thuần, sẽ khó giải thích vì sao cả **ba lớp độc lập** đều đồng thuận không bật quad-toàn-cục: (1) U-Boot cố tình `#if 0` nhánh nhận diện chip đang ở QPI kèm chú thích “không hỗ trợ” (tầng 02); (2) kernel driver không có đường nào gửi `0x38`; (3) `hwcaps` ở đây chặn luôn việc core tự ý gửi `WRSR2`. Ba lớp này được viết bởi có thể là những người khác nhau, ở những thời điểm khác nhau — nhưng cùng hội tụ về một chủ đích: **để trạng thái QE của chip yên, do một khâu khác (nhà máy, công cụ nạp firmware) đã thiết lập sẵn.** Khai báo `hwcaps` đúng với khả năng thật của chip sẽ mời core “giúp đỡ” quản lý bit QE — điều mà cả hệ thống rõ ràng không muốn.

**Hệ quả bảo trì — cái bẫy cho người đến sau.** Một kỹ sư mới đọc `nor_setup()` thấy `program_op.command = 0x32` (lệnh quad) sẽ tự nhiên nghĩ “vậy chỉ cần thêm cờ quad vào `hwcaps` để framework quản lý gọn hơn”. Làm vậy sẽ khiến `spi_nor_quad_enable()` lần đầu tiên thực sự chạy tới `nor->params->quad_enable()` — gửi `WRSR2` để set bit QE trên một chip mà toàn hệ thống đang **ngầm giả định** QE luôn luôn đã bật sẵn. Nếu chip thực tế đang _chưa_ bật QE (ví dụ sau khi thay chip mới từ nhà máy), thay đổi tưởng như vô hại này sẽ là lần đầu tiên phần mềm chủ động đổi trạng thái cấu hình chip — một hành vi mới, chưa từng được kiểm thử trong toàn bộ lịch sử driver này.

# 5. Đặc thù Quartz / KM — phần quan trọng nhất
[Đặc thù Quartz/KM](https://claude.ai/code/artifact/7aa820dd-50a5-41af-9272-aa8438298878?via=auto_preview)
Năm mảnh ghép không tồn tại ở bất kỳ driver QSPI upstream nào — vì chúng giải quyết những vấn đề chỉ riêng board này mới có: một lõi thứ hai cùng dùng chung con flash, một quy ước bảo vệ vùng nhớ xuyên hai file mã nguồn, và một kênh chẩn đoán sống sót qua cả reboot.

## 5.1. `disable_clk_r4()` — tạm dừng một CPU khác để mượn dây SPI

Không phải “tắt xung nhịp”. Đây là một giao thức bắt tay hai chiều với lõi R4.

Tên hàm gây hiểu nhầm — nó không tắt clock theo nghĩa phần cứng thông thường. Tôi lần theo `scpi_dvfs_set_idx()` tận trong `arm_scpi.c` và phát hiện đây là **hai cơ chế hoàn toàn khác nhau** tùy chiều bật/tắt.

```c
disable_clk_r4(true)   → scpi->dvfs_set_idx(0, 1)
disable_clk_r4(false)  → scpi->dvfs_set_idx(0, 0)
```

|**Tham số**|**Đường đi**|**Hành vi**|
|---|---|---|
|`domain=0, idx=**1**`_(tắt)_|Đi qua `scpi_send_message()` đầy đủ: khóa `mutex(&send_message)`, gửi lệnh SCPI thật `CMD_SET_DVFS` tới firmware R4, đặt cờ `is_wfi=1` trước khi gửi.|Đây là một **giao dịch round-trip** — chờ R4 xác nhận đã nhận lệnh. Đặt `is_lppp_wfi = 1` khi `is_wfi` được set.|
|`domain=0, idx=**0**`_(bật lại)_|Rẽ nhánh **đặc cách ngay đầu hàm**, không qua hàng đợi SCPI: `ioremap(0xE8008000,…)` rồi ghi thẳng bit `PAUSE_R4_INT` vào thanh ghi IPC.|Một **ngắt phần cứng trực tiếp** đánh thức R4. Rồi `is_lppp_wfi=0; wake_up(&wfi_wait_queue)`.|
**Vì sao chiều “bật lại” phải đi đường tắt.** Comment trong code ghi rõ: _“wake up the r4 using an ipc interrupt. Not scpi since other scpi cmds will wake the r4 before it should be woken”_. Lý do sâu hơn mà tôi xác minh được: `scpi_send_message()` — con đường SCPI bình thường — luôn bắt đầu bằng `wait_event_timeout(wfi_wait_queue, !is_lppp_wfi, 60000ms)` (`arm_scpi.c:532`). Nếu “đánh thức” cũng đi qua con đường đó, nó sẽ **tự khóa chính mình** — chờ cờ `is_lppp_wfi` hạ xuống trong khi chính nó là thứ duy nhất có thể hạ cờ đó. Đường tắt qua thanh ghi IPC là lối thoát duy nhất khỏi bế tắc này.

**Hệ quả toàn hệ thống, không chỉ riêng QSPI.** Trong lúc R4 đang ở WFI (`is_lppp_wfi==1`), **MỌI** lời gọi SCPI khác trong kernel — không riêng gì driver flash — sẽ bị chặn tới 60 giây tại `wait_event_timeout()`. Bất kỳ subsystem nào dùng SCPI (quản lý nhiệt, scaling tần số các domain khác, đọc cảm biến…) đều phải xếp hàng phía sau một lần ghi/xóa flash. Đây là một **vùng khóa toàn cục**, không phải chi tiết cục bộ của driver này.

### Ý nghĩa của `usleep_range(1000,1000)`

```c
scpi->dvfs_set_idx(0,1);   // gửi lệnh dừng R4
usleep_range(1000,1000);
/* LPPPの停止を確実にするため、LPPPの停止レスポンスを1ms待つ(LPPPフリーズ対策) */
/* Để đảm bảo LPPP đã dừng, chờ 1ms cho phản hồi dừng của LPPP (biện pháp chống LPPP bị treo) */
```

Chú thích gốc gọi thẳng tên vấn đề: **“LPPPフリーズ対策”** — biện pháp chống đông cứng LPPP. Việc gửi `CMD_SET_DVFS` chỉ xác nhận R4 đã _nhận_ lệnh, không đảm bảo nó đã _thực sự dừng hẳn_ ở một điểm an toàn (không giữa chừng một lệnh fetch hay một giao dịch SPI dở dang). 1ms là biên độ ổn định vật lý được đúc kết từ kinh nghiệm thực tế — tên biến chú thích cho thấy đây là bản vá cho một lỗi **đã từng xảy ra thật**, không phải phòng xa lý thuyết.

### Vì sao chuyển vào `prepare`/`unprepare` thay vì gọi trực tiếp

```c
#ifdef CONFIG_KM_BIZHUB
static int mrvl_prepare(struct spi_nor *nor)   { return disable_clk_r4(true); }
static void mrvl_unprepare(struct spi_nor *nor){ disable_clk_r4(false); }
#endif
```

![[Pasted image 20260824125209.png]]

Toàn bộ giao thức bắt tay này chỉ có ý nghĩa nếu nó bọc đúng _một_ giao dịch trọn vẹn — đó chính xác là những gì `prepare`/`unprepare` được thiết kế để đảm bảo.

**Vì sao không gọi trực tiếp trong từng hàm write/erase.** Ba lý do, xếp theo mức độ quan trọng:

① **Đối xứng bắt buộc.** Nếu gọi rải rác trong `mrvl_qspi_write()`/`mrvl_qspi_erase()`, mỗi hàm phải tự nhớ gọi cả `disable_clk_r4(true)` lẫn `(false)` đúng cặp, kể cả trên MỌI đường lỗi sớm (`return -EIO` giữa hàm). Bất kỳ đường thoát nào quên gọi `(false)` sẽ để R4 **kẹt ở WFI vĩnh viễn** — khóa cả subsystem SCPI của toàn máy tới khi timeout 60s liên tục xảy ra cho mọi caller khác.

② `spi_nor_lock_and_prep()`/`unlock_and_unprep()` (core.c) đã đảm bảo cặp gọi này **luôn khớp**, bất kể đường nào trong core dẫn tới — kể cả các đường lỗi nội bộ của chính spi-nor core mà driver không kiểm soát được.

③ **Phạm vi đúng.** `prepare`/`unprepare` bọc toàn bộ giao dịch (kể cả các lệnh `WREN`/poll trạng thái xen giữa — xem tầng 03), không chỉ phần lệnh chính. Gọi cục bộ trong `mrvl_qspi_write()` sẽ bỏ sót `write_reg(WREN)` mà core gọi _trước_ khi vào hàm này.

**Điều gì xảy ra nếu bỏ qua bước này — cả về lý thuyết lẫn theo đúng lời code tự thú nhận:** R4 tiếp tục tự fetch lệnh từ CS1 qua cửa sổ ánh xạ đúng lúc CA72 lật `BSCR.INTERFACEMODE` sang command mode để ghi/xóa. Vì đây là **một khối thanh ghi vật lý duy nhất** (đã xác nhận ở tầng 03), quá trình fetch của R4 sẽ đọc phải rác giữa chừng — hệ quả trực tiếp là **LPPP treo/đông cứng**, đúng như tên biến chú thích trong code đã gọi thẳng ra: “LPPPフリーズ対策”.

## 5.2. Bản đồ write-protect — và một khoảng trống thật ở đường xóa

Bốn vùng cấm, hai hàm kiểm tra, nhưng chỉ một hàm kiểm tra đủ.

![[Pasted image 20260824125302.png]]

Đỏ = bảo vệ cả ghi lẫn xóa. Vàng = **chỉ** bảo vệ ghi — sector chứa vùng này vẫn có thể bị lệnh `flash_erase` xóa sạch.

|**Vùng**|**Địa chỉ**|**`chip_select`**|**Chặn GHI?**|**Chặn XÓA?**|
|---|---|--:|---|---|
|**LPDDR4PARAM**|`0x381800–0x382000`|0||**✓ dòng 600 / 756**|
|**BOOTROM**|`0x000000–0x300000`|0||**✓ dòng 601 / 757**|
|**SECURE_AREA**|`0x3FD000–0x3FE000`|0||**✗ không có**|
|**LPPP**|`0x000000–0x200000`|1||**✓ dòng 603 / 758**|

**Đây không phải suy đoán — tôi đã đọc lại nguyên văn cả hai khối điều kiện.** Khối trong `mrvl_qspi_write()` (dòng 599-603) có đúng 4 nhánh `||`. Khối trong `qspi_erase()` (dòng 755-758) chỉ có 3 nhánh — thiếu hẳn điều kiện `SECURE_AREA`. Comment lịch sử ghi _“2024/08/21 Sky — Secure boot対応 保護領域追加”_ (thêm vùng bảo vệ cho secure boot) chỉ xuất hiện **một lần**, ngay trước khối `write` — gợi ý mạnh rằng khi thêm tính năng này, người viết chỉ sửa một trong hai hàm và quên hàm còn lại.**Hệ quả cụ thể:** một lệnh `flash_erase /dev/mtd0 0x3FD000 0x1000` (hoặc bất kỳ thao tác MTD nào kích hoạt xóa sector chứa `0x3FD000-0x3FE000`) sẽ **không hề bị chặn**, trong khi một lệnh ghi trực tiếp vào cùng vùng đó lại bị từ chối với `-EIO`. Với một vùng được đặt tên “SECURE_AREA” phục vụ secure boot, đây là khoảng trống đáng để vá trước, không phải chỉ để ghi chú.

### Cờ `is_write_protect_lpddr4param` — công tắc tổng cho cả bốn vùng

Mặc dù tên biến chỉ nhắc tới LPDDR4PARAM (dấu vết lịch sử — đây là vùng đầu tiên được bảo vệ, các vùng khác thêm sau nhưng dùng chung cờ), nó thực chất là **công tắc tổng** đứng đầu cả hai khối điều kiện (`if (is_write_protect_lpddr4param && (...))`). Bằng 0 → toàn bộ 4 vùng **hoàn toàn không được bảo vệ**, bất kể chip_select hay địa chỉ nào.

|**Nơi đổi giá trị**|**Cách đổi**|
|---|---|
|**Giá trị khởi tạo**|`int is_write_protect_lpddr4param = 1;` — mặc định **luôn bảo vệ**|
|**sysfs**|`/sys/…/write_protect_lpddr4param` — đọc/ghi trực tiếp qua `simple_strtoul()`, không kiểm tra quyền ngoài quyền file (`0664`)|
|**`qspi_probe()`**|Ghi đè tự động dựa theo `c_BootType` — xem prompt 16 ngay dưới đây|

## 5.3. `c_BootType` — một byte đi xuyên qua ranh giới BootROM/kernel

Không phải biến môi trường, không phải tham số dòng lệnh — mà là một property được vá thẳng vào FDT.

![[Pasted image 20260824125414.png]]

Một biến toàn cục trong RAM của BootROM trở thành một biến toàn cục trong RAM của kernel, không qua tham số dòng lệnh hay tệp tin nào — chỉ nhờ FDT blob được vá giữa hai lần khởi động.

|**Giá trị**|**Hằng số BootROM**|**Ý nghĩa**|**Vì sao cần ghi được LPDDR4PARAM**|
|--:|---|---|---|
|`2`|`USB_UPDATE`|Boot từ USB để cập nhật firmware|Cần ghi tham số LPDDR4 mới nếu bản cập nhật thay đổi cấu hình RAM|
|`3`|`DEF_IISW`|Chế độ IISW (công cụ nội bộ)|Công cụ chẩn đoán/hiệu chỉnh nhà máy cần quyền ghi rộng|
|`5`|`BOOT_ERR`|Boot vào do lỗi ở lần trước|Đường phục hồi lỗi có thể cần ghi lại tham số đã hỏng|
|`9`|`DEF_UPDATE`|Cập nhật firmware qua đường chính thức|Giống `USB_UPDATE` — bản cập nhật hợp lệ cần ghi được|
|`11`|`DEF_BACKUP`|Chế độ sao lưu|Có thể cần ghi lại metadata trước khi sao lưu|
|`12`|`DEF_RESTORE`|Chế độ khôi phục|Khôi phục thường đồng nghĩa ghi đè tham số cũ bằng tham số đã lưu|
**Suy luận về nguyên tắc thiết kế:** sáu chế độ này có điểm chung — tất cả đều là những phiên làm việc **có chủ đích thay đổi firmware/cấu hình** (cập nhật, khôi phục, công cụ chẩn đoán, phục hồi lỗi), khác với boot bình thường (`DEF_BOOT=0`, không nằm trong danh sách) nơi LPDDR4PARAM phải được giữ nguyên bất di bất dịch. Đây là một dạng “write-protect theo ngữ cảnh boot” — an toàn hơn một cờ tĩnh, nhưng cũng có nghĩa là: nếu ai đó cần ghi tham số LPDDR4 thủ công trong khi máy đang chạy boot bình thường (`c_BootType=0`), họ phải tự tay ghi `0` vào sysfs `write_protect_lpddr4param` trước — driver sẽ không tự nới lỏng.

## 5.4. Mảng `devices[]` — một controller, tối đa mười chip
Board này chỉ dùng 2 trong 10 slot. Tám slot còn lại là dung lượng dự phòng chưa từng được khai thác.

```c
#define NUM_BSPI_PARTS 10                    // dòng 77
static uint32_t num_devices = 0;             // dòng 103 — bộ đếm toàn cục, tăng dần
struct qspi_s devices[NUM_BSPI_PARTS];       // dòng 104 — mảng tĩnh, KHÔNG cấp phát động
```

`qspi_probe()` được kernel gọi **một lần cho mỗi node DT khớp** `compatible="mrvl,bspi"` — board Quartz có đúng 2 node (`spi-flash0`/CS1, `spi-flash1`/CS0, xem tầng 00), nên hàm chạy hai lần, mỗi lần chiếm một ô trong `devices[]` theo đúng thứ tự khai báo trong DTS. Bảy nguồn tài nguyên được xin trong `qspi_probe()` chia thành hai nhóm rất khác nhau về vòng đời:

### Tài nguyên nào dùng chung, tài nguyên nào riêng từng device

| **Tài nguyên**            | **Phạm vi**           | **Điều kiện trong code**                                                                                                                                                                                                  |
| ------------------------- | --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `register_base`           | **Dùng chung**        | `if (num_devices==0) devm_ioremap_resource(...)` — dòng 1010–1014. Device 1 chỉ chép lại: `devices[1].register_base = devices[0].register_base` — dòng 1018.                                                              |
| `bspi_clk` (`0xE8240000`) | **Dùng chung**        | Cũng bọc trong `if (num_devices==0)` — dòng 1054. Không có nhánh `else` vì device 1 **không cần** — trường này không lưu theo từng device.                                                                                |
| **IRQ handler**           | **Dùng chung**        | Đăng ký một lần trong nhánh `num_devices==0`, nhận diện qua `dev_id = &devices[num_devices]` — nhưng handler chỉ `printk` rồi trả `IRQ_NONE` (tầng 01), nên tính đúng/sai của việc gán `dev_id` không thực sự quan trọng. |
| `map_memory` (cửa sổ XIP) | **Riêng từng device** | Đọc từ DT của _chính node đó_ mỗi lần probe, không có điều kiện `num_devices` nào bọc quanh — dòng 1027–1049.                                                                                                             |
| `chip_select`             | **Riêng từng device** | Tương tự — đọc DT riêng, mặc định `3` nếu thiếu, từ chối nếu `>3` — dòng 1106–1111.                                                                                                                                       |
| `struct spi_nor`, `mtd`   | **Riêng từng device** | `devices[num_devices].nor` — instance độc lập hoàn toàn, kể cả `nor->lock` mutex riêng (đã phân tích race ở tầng 03).                                                                                                     |

**Nguồn gốc thiết kế:** pattern “chỉ device 0 mới ioremap tài nguyên vật lý dùng chung” là hợp lý bắt buộc — gọi `devm_ioremap_resource()` hai lần trên _cùng một_ vùng vật lý (`reg = <0x8 0xe8273000...>`, giống hệt nhau ở cả hai node DT, xem tầng 00) sẽ khiến lần gọi thứ hai thất bại vì Linux không cho ánh xạ chồng lấn vùng MMIO đã được request. `NUM_BSPI_PARTS=10` gợi ý driver gốc (trước khi được KM tùy biến) hướng tới một controller BSPI có thể phục vụ tới 10 chip select vật lý khác nhau — con số Quartz chỉ dùng 2 trên 4 chip select khả dụng của phần cứng (`chip_select` giới hạn 0-3).

### Vì sao chỉ `spi-flash0` được cấp sysfs `flash_size`

```c
if (strstr(mtd->name, "spi-flash0")) {          // dòng 1200
    ret = device_create_file(nor->dev, &dev_attr_flash_size);
}
```

Nhắc lại phát hiện từ tầng 00: `mtd->name` được spi-nor core gán bằng `dev_name(dev)` nếu chưa có tên — với node `spi-flash0@8e8273000,0` (CS1, LPPP, 2MB), tên thiết bị sẽ chứa chuỗi `"spi-flash0"`. Comment trong code tự cảnh báo đường dẫn sysfs này **đổi giữa các phiên bản kernel** (`8e8273000.spi-flash0` ở 5.10.205 nhưng `8e8273000.spi0` ở 4.4.8) — bất kỳ script giám sát nào dựa vào đường dẫn cố định sẽ vỡ khi nâng cấp kernel. Và vì tên node gắn với **CS1 (LPPP)**, sysfs này báo dung lượng của chip nhỏ hơn (2MB), không phải chip chứa romfs/user mà đa số người sẽ đoán.

## 5.5. `mrvl_qspi_set_new_detail_code()` — dấu vân tay 4 byte của một tiến trình

Khi ai đó cố ghi vào vùng cấm, hệ thống nén tên tiến trình đó thành 4 byte và ghi xuống đĩa trước khi mọi thứ có thể biến mất.

Hàm này chạy đúng hai chỗ — ngay trước `return -EIO` ở cả `mrvl_qspi_write()` (dòng 605) và `qspi_erase()` (dòng 760), tức chính là hai vi phạm write-protect vừa lập bản đồ ở prompt 15. Mục tiêu: ghi lại **tiến trình nào** đã cố vi phạm, vì bản thân tiến trình đó có thể đã chết hoặc log kernel có thể đã bị ghi đè trước khi kỹ sư hiện trường mở máy ra xem.

### Thuật toán chọn ký tự — không phải cắt chuỗi đơn giản

| **Độ dài `current->comm`** | **`uc_data[0]`**                                                       | **`uc_data[1]`** | **`uc_data[2]`** | **`uc_data[3]`** |
| -------------------------: | ---------------------------------------------------------------------- | ---------------- | ---------------- | ---------------- |
|                        ≤ 4 | Chép nguyên xi từng ký tự có, phần dư giữ giá trị cũ trong mảng cục bộ |                  |                  |                  |
|                          5 | `[0]`                                                                  | `[2]`            | `[len-2]`        | `[len-1]`        |
|                          6 | `[0]`                                                                  | `[3]`            | `[len-2]`        | `[len-1]`        |
|                          7 | `[0]`                                                                  | `[4]`            | `[len-2]`        | `[len-1]`        |
|                          8 | `[0]`                                                                  | `[5]`            | `[len-2]`        | `[len-1]`        |
|                          9 | `[0]`                                                                  | `[5]`            | `[len-3]`        | `[len-1]`        |
|                        > 9 | `[0]`                                                                  | `[5]`            | `[len-4]`        | `[len-1]`        |

**Vì sao không đơn giản lấy 4 ký tự đầu.** Tên tiến trình kernel rất hay dùng chung tiền tố dài — `kworker/0:1`, `kworker/0:2`, `kworker/u8:3`, `irq/51-bspi`… Nếu chỉ lấy `comm[0..3]`, hàng loạt tiến trình khác nhau sẽ đổ về cùng một dấu vân tay `"kwor"` — vô dụng cho việc phân biệt. Thuật toán này cố tình lấy **một ký tự đầu cố định** (neo vị trí bắt đầu) + **một ký tự giữa trôi dần theo độ dài** (chỉ số 2,3,4,5,5,5… tăng chậm rồi bão hòa ở 5) + **hai ký tự cuối** (thường là phần biến thiên nhất — số thứ tự CPU, PID rút gọn) — tối đa hóa khả năng phân biệt trong ngân sách cố định 4 byte (32 bit), thay vì một phép cắt chuỗi ngây thơ.

### Đóng gói vào khuôn 64-bit

![[Pasted image 20260824125657.png]]

Mười sáu chữ số hex, mỗi nhóm mang một ý nghĩa cố định. Tiền tố `0x22023` là “chữ ký họ mã” — cùng quy ước với `0x2201...` (panic PC, thấy trong `traps.c`) và các tiền tố khác rải rác trong cây mã KM.

### Ai đọc lại — và đọc như thế nào

```c
int set_newdetailcode( unsigned long long code )        // mdels800_pmu.c:5439
{
    if (0 == newdetailcode_first) { newdetailcode_first = 1; }
    else { return 0; }                                    // ← CHỈ ghi lần ĐẦU TIÊN mỗi phiên boot

    fd = do_sys_open(AT_FDCWD, "/ata0a/bsp_newdetailcode.bin",
                      O_WRONLY|O_CREAT, ...);
    ksys_write(fd, (char __user*)&code, sizeof(code));    // ghi thẳng 8 byte nhị phân
    emergency_sync(); emergency_sync(); emergency_sync(); // flush 3 lần liên tiếp
    ...
}
```

**Hai đặc điểm thiết kế đáng chú ý, cả hai đều là dấu hiệu “viết cho tình huống hệ thống sắp chết”.

**① **Chốt ghi-một-lần.** Cờ tĩnh `newdetailcode_first` đảm bảo chỉ sự kiện _đầu tiên_ trong một phiên boot được lưu — nếu vi phạm write-protect xảy ra nhiều lần (ví dụ một script lỗi cứ lặp lại thao tác ghi), chỉ lần đầu tiên được ghi nhận. Đúng triết lý “nguyên nhân gốc”: sự kiện đầu tiên thường là nguyên nhân, các lần sau chỉ là hệ quả lặp lại.

② **`emergency_sync()` gọi ba lần liên tiếp** — không phải một lần cho chắc, mà ba lần. Đây là mã chạy ngay sau khi phát hiện một hành vi bất thường (ghi/xóa vào vùng cấm) — hệ thống có thể sắp bị buộc reset hoặc rơi vào trạng thái không ổn định. Ưu tiên tuyệt đối là đảm bảo 8 byte này **sống sót** qua bất kỳ điều gì xảy ra tiếp theo, kể cả khi bản thân `emergency_sync()` có thể không hoàn thành trọn vẹn ở một trong ba lần gọi.**Giới hạn những gì tôi xác minh được từ mã nguồn:** file đích `/ata0a/bsp_newdetailcode.bin` là một file nhị phân 8 byte trên phân vùng lưu trữ chính. Công cụ nào đọc lại và giải mã file này (khớp lại với tiền tố `0x2201/0x2202/0x2203…` để tra ra ý nghĩa) nằm ngoài phạm vi cây nguồn kernel/BootROM — nhiều khả năng là một công cụ chẩn đoán hiện trường hoặc quy trình nội bộ không có trong hai cây mã này.

# 6. Đường khẩn cấp — panic, oops, watchdog
[Đường khẩn cấp](https://claude.ai/code/artifact/6e26c39d-e454-4b48-8c14-d74718ff259b)
Code chỉ chạy khi hệ thống đang sập — panic, oops, watchdog sắp reboot. Chính vì hiếm chạy, ba trong bốn prompt dưới đây phát hiện những đường code **chưa bao giờ được thực thi trong đời thực**, và ít nhất một trong số đó là một vòng lặp vô hạn được đảm bảo tuyệt đối về mặt toán học

### Phát hiện đầu tiên, nghiêm trọng nhất tầng này

### Cơ chế cứu dữ liệu chẩn đoán trước khi chết có thể tự treo vĩnh viễn — ngay tại thời điểm nó được sinh ra để phục vụ

Ba biến điều khiển vòng lặp chờ “chip đã Suspend chưa” trong `qspi_powerdown_write()` chỉ được khởi tạo bởi `check_jedecid()` — một hàm bị loại khỏi build KM_BIZHUB hoàn toàn, và lời gọi duy nhất của nó còn nằm trong `#if 0`. Kết quả: điều kiện dừng vòng lặp trở thành `(x & 0) == 0` — **luôn đúng, không phụ thuộc bất kỳ trạng thái phần cứng nào**. Xem chi tiết đầy đủ ở prompt 21.

## `qspi_powerdown_write()` — đường ghi cho một thế giới không còn framework

Không mutex, không R4, chip_select hard-code — mỗi thứ bị bỏ đều có lý do sinh tồn.

Hàm này không gọi `spi_nor_*`, không đụng `controller_ops`, không cần `struct spi_nor` nào tồn tại. Nó lấy thẳng `devices[0].register_base` — con trỏ MMIO thô — và tự bit-bang qua `qspi_write_then_read()`, một bản sao rút gọn của `execute_cmd()` viết lại từ đầu, độc lập hoàn toàn với mọi thứ đã phân tích ở các tầng trước.

|**Khía cạnh**|**`mrvl_qspi_write()`**|**`qspi_powerdown_write()`**|
|---|---|---|
|**Khóa/đồng bộ**|Chạy dưới `nor->lock` mutex (caller giữ qua `spi_nor_lock_and_prep`, tầng 03)|**Không có mutex nào.** Có thể chạy đè lên một giao dịch khác đang dở dang trên cùng thanh ghi.|
|**Ngữ cảnh gọi**|Process context — có thể `might_sleep()`, nằm sau `mutex_lock`|Notifier chain của `panic()`/`die()` — thường chạy với ngắt đã tắt, không được phép ngủ|
|**Kiểm tra bận trước khi bắt đầu**|`check_busy()` — vòng lặp có `BUSY_CHK_TIMEOUT` (~1s), retry đàng hoàng|Kiểm tra **một lần duy nhất**; nếu đang bận, bỏ qua bước reset CS và cứ thế tiến tới lệnh RDSR tiếp theo — không chờ, không retry|
|**WREN**|Gửi vô điều kiện mỗi trang, qua `execute_cmd(write_enable)`|Gửi **có điều kiện** — chỉ khi đọc SR1 thấy bit WEL (`0x02`) chưa bật|
|**`chip_select`**|Đọc từ `plat_data->chip_select` (`0` hoặc `1`, theo DT)|**Hard-code = 0** trong `qspi_write_then_read()` — chỉ nhắm đúng CS0. Nhất quán với giới hạn địa chỉ `addr+size > 0x400000` ngay đầu hàm (đúng 4 MB của CS0).|
|**Xử lý R4/LPPP**|Bọc trong `prepare()`/`unprepare()` → `disable_clk_r4()` đầy đủ (tầng 04)|**Không có gì cả.** Không SCPI, không WFI, không chờ R4.|
|**Opcode ghi**|`0x32` (Quad Page Program) — lật `DATAMODE` giữa||

### Vì sao thiếu R4 không phải là lỗi — mà là một đánh đổi bắt buộc

Ở tầng 04, chuỗi bắt tay hạ R4 là một giao dịch SCPI **round-trip**: khóa mutex, gửi lệnh, chờ phản hồi, rồi tự tin mới tiến hành. Trong ngữ cảnh `panic()`/`die()`, không có gì đảm bảo **ngắt phản hồi từ SCP còn được phục vụ**, hàng đợi message còn hoạt động, hay việc gọi `schedule()`/ngủ chờ còn an toàn. Nếu `qspi_powerdown_write()` cũng đòi hỏi bắt tay R4 như đường bình thường, nó có nguy cơ **treo vĩnh viễn ngay trong chính panic handler** — biến một hệ thống đang sập (nhưng còn cơ hội ghi vài byte chẩn đoán) thành một hệ thống treo cứng hoàn toàn, không còn cơ hội nào.

Đây là đánh đổi có chủ đích: **chấp nhận rủi ro race với R4** (một sự cố hiếm, thường chỉ gây LPPP treo — vẫn có thể phục hồi bằng reset) để đổi lấy **khả năng chắc chắn không tự khóa chết trong panic handler** (một sự cố nghiêm trọng hơn nhiều — mất luôn cơ hội ghi log cuối cùng).

**Đây không phải suy đoán lý thuyết — đánh đổi này đã từng gây sự cố thật.** Comment tại `qspi_set_bh_b_state()` (phân tích đầy đủ ở prompt 22) ghi rõ: _“2020/11/25 OP_BTS-28498 — sửa lỗi LPPP bị treo do dùng hàm ngắt QSPI”_, và _“qspi_powerdown_write không thực hiện loại trừ lẫn nhau với LPPP nên dùng nó sẽ gây treo”_. Tức là: có một caller **khác** (không phải panic) từng dùng `qspi_powerdown_write()` theo thói quen, và thực sự làm LPPP đông cứng ngoài thực địa. Bằng chứng này xác nhận đầy đủ phân tích ở tầng 04 — không phải suy diễn từ đọc code, mà là lịch sử sửa lỗi thật.

## `qspi_panic_happened()` — hộp đen sáu lần ghi

Ba loại dữ liệu, mỗi loại ghi hai bản, hai bank flash khác nhau.


|**#**|**Dữ liệu**|**Offset (BSPTHR1 / BSPTHR2)**|**Giá trị**|
|--:|---|---|---|
|1–2|Cờ tắt SuperWarp|`+0x24` · `0x377024` / `0x378024`|`0x10`|
|3–4|Cờ “đã panic”|`+0x42` · `0x377042` / `0x378042`|`0x01`|
|5–6|Detail code 5 byte (PC lúc crash)|`+0x44` · `0x377044` / `0x378044`|4 ký tự + độ dài|
Ghi hai bản (BSPTHR1 tại `0x377000`, BSPTHR2 tại `0x378000`) là một khuôn mẫu lặp lại xuyên suốt toàn bộ hệ thống chẩn đoán KM — hai bank dự phòng cho nhau, chống trường hợp một bank bị hỏng đúng lúc cần đọc lại. Tất cả các offset đều nằm trong vùng `spi-user` (CS0), **ngoài** phạm vi bảo vệ `LPDDR4PARAM`/`BOOTROM`/`SECURE_AREA` đã lập bản đồ ở tầng 04 — hợp lý, vì đây chính là vùng dành cho log/chẩn đoán runtime.

### Nguồn gốc chuỗi PC — tái sử dụng đúng thuật toán ở prompt 18, nhưng seed khác

```c
/* arch/arm64/kernel/process.c:299 — chạy TRƯỚC panic notifier, trong __show_regs() */
sprint_symbol(buffer_Oops, instruction_pointer(regs));
/* buffer_Oops giờ chứa: "ten_ham_bi_loi+0x123/0x456 [module]" */
```

```c
/* mrvl-bspi_winbond.c:1535 — cắt tại dấu '+' để chỉ lấy TÊN HÀM, bỏ phần offset */
for (buffer_Oops_len=0; buffer_Oops_len<strlen(buffer_Oops); buffer_Oops_len++) {
    if (buffer_Oops[buffer_Oops_len] == '+') break;
}
/* rồi áp đúng thuật toán chọn-ký-tự của prompt 18, nhưng "độ dài" ở đây
   là vị trí dấu '+' — tức độ dài TÊN HÀM, không phải độ dài buffer_Oops đầy đủ */
```

**Mẹo nén thêm 4 bit “miễn phí” bằng cách mượn bit dấu của ASCII.** Sau khi có 4 byte ký tự (`uc_data[0..3]`) và 1 byte độ dài (`uc_data[4]`), khối cuối hàm làm một việc tinh tế:

```c
extern char dev_resume_suspend_flg;  /* trạng thái toàn cục chu trình suspend/resume thiết bị */
for (i = 0; i < 4; i++) {
    uc_data[i] &= 0x7F;                              /* xóa bit cao — ký tự ASCII in được luôn <128 nên KHÔNG mất dữ liệu */
    uc_data[i] |= (((dev_resume_suspend_flg >> i) & 0x01) << 7);  /* nhét 1 bit trạng thái vào đúng chỗ vừa trống */
}
```

Vì mọi ký tự ASCII in được đều nằm trong 0x20-0x7E (bit 7 luôn = 0 sẵn), việc xóa rồi ghi lại bit 7 **không hề mất thông tin ký tự**. Bốn bit thấp của `dev_resume_suspend_flg` (0=idle, 1=đang trong `dpm_resume()`, 2=đang trong `dpm_suspend()`, 4=liên quan DeepSleep — `drivers/base/power/main.c:728,1108,1859`) được rải đều vào 4 bit dấu đó. Kết quả: chỉ với đúng 5 byte, bản ghi panic vừa mang tên hàm gây lỗi, vừa mang luôn _hệ thống có đang ở giữa một chu trình suspend/resume thiết bị hay không_ — hai loại thông tin độc lập, đóng gói chồng lên nhau không tốn thêm byte nào.

### Vì sao có điều kiện `valid_superwarp_session == false`
Tôi lần theo cả hai điểm gán giá trị cho cờ này trong `kernel/power/warp.c`:

|**Giá trị**|**Đặt ở đâu**|**Ý nghĩa**|
|---|---|---|
|`true`|Ngay sau `WARP_PROGRESS_SAVEEND` — khi thao tác lưu trạng thái SuperWarp vừa hoàn tất thành công|Đã có một checkpoint hợp lệ, hệ thống sắp/đang trong quy trình tắt máy có trật tự dựa trên checkpoint đó|
|`false`|Sau `resume_console()` trong đường resume|Phiên SuperWarp đã khép lại — không còn checkpoint “đang chờ dùng” nào nữa|
**Suy luận có căn cứ từ đúng cặp comment `“for issue OP_BTS-20538”` đánh dấu cả cờ lẫn điều kiện kiểm tra:** `true` đánh dấu một cửa sổ rất hẹp — ngay sau khi lưu checkpoint xong, trước khi phiên đó khép lại. Nếu panic xảy ra đúng trong cửa sổ này, nhiều khả năng đó là lỗi trong chính luồng dọn dẹp hậu-lưu của SuperWarp, chứ không phải một sự cố độc lập của hệ thống. Bỏ qua việc ghi “tắt SuperWarp vĩnh viễn” trong đúng trường hợp này tránh việc một panic cục bộ, tạm thời trong quy trình lưu lại vô tình vô hiệu hóa hẳn một tính năng tiết kiệm năng lượng đang hoạt động tốt.

**Giới hạn xác minh:** tôi tìm khắp `kernel/power/warp.c` lẫn phần còn lại của cây kernel nhưng **không thấy nơi nào đọc lại** cờ `SUPERWARP_DISABLE_FLAG` (offset +0x24) đã ghi ở đây. Có thể nó được đọc bởi một công cụ ngoài cây nguồn này, hoặc bởi một đường tôi chưa lần ra — tôi nêu rõ đây là ranh giới của những gì xác minh được, không suy diễn thêm.

## Vòng chờ Suspend — và một vòng lặp vô hạn được đảm bảo 100%

Không phải race condition. Không phụ thuộc timing. Chạy tới đây là treo, luôn luôn.

### Heuristic 5ms: phân biệt Write đang xong với Erase còn đang chạy

```c
tx[0] = SPI_CMD_RDSR; qspi_write_then_read(regs, tx, 1, rx, 1);
if( rx[0] & 0x01 ){                 // đang bận (WIP=1)?
    mdelay(5);                       // đợi 5ms rồi hỏi lại
    tx[0] = SPI_CMD_RDSR; qspi_write_then_read(regs, tx, 1, rx, 1);
    if( rx[0] & 0x01 ){             // VẪN bận sau 5ms?
        /* → kết luận: đây là Erase, không phải Write → phải Suspend */
        ...
    }
    // nếu KHÔNG còn bận: đó là Write vừa xong tự nhiên trong 5ms, không cần Suspend
}
```

Đây là một phép đo gián tiếp dựa trên vật lý chip flash, không phải đọc trực tiếp loại lệnh: **Page Program** trên các chip Winbond dòng này thường hoàn tất trong dưới 1ms (tối đa quanh vài ms theo datasheet), trong khi **Sector Erase** có thể mất tới hàng chục đến vài trăm ms. Bằng cách đợi 5ms rồi hỏi lại đúng một lần: nếu BUSY đã sạch, gần như chắc chắn đó là một Write đã tự xong — không cần can thiệp. Nếu BUSY vẫn còn, khả năng cao đó là một Erase còn đang chạy dở — và vì hàm này không có thời gian để chờ nốt (có thể hệ thống sắp reset), nó chủ động gửi lệnh **Suspend** để tạm dừng Erase, chen giao dịch ghi khẩn cấp của mình vào, rồi mới để Erase tiếp tục.

### Nơi vòng lặp vô hạn thực sự nằm — theo đúng ba bước trong code

![[Pasted image 20260824132917.png]]

Không một bước nào trong chuỗi này phụ thuộc dữ liệu thời gian chạy. Đây là kết luận có thể suy ra tĩnh, 100% chắc chắn, chỉ từ việc đọc mã nguồn.

```c
tx[0] = uc_suspend_command;              // = 0x00 — KHÔNG phải opcode Suspend thật (0x75)
qspi_write_then_read(regs, tx, 1, rx, 0); // gửi opcode 0x00 ra chip — hành vi không xác định

do{
    udelay(1);
    tx[0] = uc_readstatus_command;        // = 0x00 — KHÔNG phải "Read Status Register-2" (0x35)
    qspi_write_then_read(regs, tx, 1, rx, 1);
}while( (rx[0] & uc_suspend_bit) == 0 );  // uc_suspend_bit = 0x00 → (rx[0] & 0) LUÔN = 0 → LUÔN lặp tiếp
```

**Đây không phải “có thể xảy ra trong điều kiện xấu” — đây là hành vi được đảm bảo bởi chính phép toán bit.** Biểu thức `(rx[0] & 0x00) == 0` đúng với **mọi** giá trị của `rx[0]`, không phụ thuộc chip flash trả lời gì. Điều kiện duy nhất để chạm vào nhánh này là: một giao dịch MTD write/erase khác đang thực sự xóa dở một sector trên CS0 (mất vài chục tới vài trăm ms — cửa sổ hoàn toàn thực tế) **đúng vào lúc** panic/oops/watchdog-fire xảy ra và gọi tới `qspi_powerdown_write()`.**Hậu quả xâu chuỗi với tầng 01:** nhớ lại `CONFIG_SOFTLOCKUP_DETECTOR` và `CONFIG_DETECT_HUNG_TASK` đều tắt trong `km_mvebu_v8_lsp_defconfig`. Nghĩa là khi vòng lặp này kích hoạt bên trong panic handler, **không có cơ chế nào trong kernel sẽ báo cáo nó bị kẹt** — hệ thống chỉ đơn giản im lặng mãi mãi, đúng vào đúng khoảnh khắc nó được thiết kế để ghi lại thông tin cứu mạng cho lần điều tra sau.**Đúng thời điểm rủi ro nhất, nó lại tệ nhất:** panic có thể xảy ra _bất cứ lúc nào_ — kể cả ngay giữa một giao dịch `flash_erase` đang chạy trên chính con flash mà `qspi_panic_happened()` sắp cố ghi vào. Cơ chế được tạo ra để bắt lỗi hiếm gặp lại có nguy cơ tự treo đúng vào những lần panic hiếm gặp và khó tái hiện nhất — chính là những lần cần dữ liệu chẩn đoán nhất.

### Vì sao `qspi_oops_happened()` ghi ít dữ liệu hơn hẳn

```c
static int qspi_oops_happened(...) {
    unsigned char uc_flag = DEF_SUPERWARP_DISABLE_FLAG_VALUE;
    if ( in_interrupt() || panic_on_oops ) {
        /* Kernel Panicになる場合は、記録しない */
    } else {
        ...
        qspi_powerdown_write( ...SUPERWARP_DISABLE_FLAG..., &uc_flag, 1 );  // chỉ 2 lần gọi
        qspi_powerdown_write( ...SUPERWARP_DISABLE_FLAG_2..., &uc_flag, 1 );
    }
    return NOTIFY_DONE;
}
```

**Hai lý do, cả hai đều đọc được trực tiếp từ điều kiện bảo vệ.** Một oops (khác panic) là sự kiện **có thể phục hồi** — kernel tiếp tục chạy sau đó. Guard `panic_on_oops`: nếu cấu hình hệ thống quy định oops phải leo thang thành panic thật, thì `qspi_panic_happened()` sẽ tự chạy ngay sau đó với đầy đủ 6 lần ghi — ghi thêm ở đây chỉ là trùng lặp vô ích. Guard `in_interrupt()`: một oops hoàn toàn có thể xảy ra bên trong một ISR (ví dụ deref con trỏ hỏng trong ngắt phần cứng) — nơi việc chạy chuỗi RDSR/Suspend/mdelay(5) nặng nề của `qspi_powerdown_write()` là không an toàn và không phù hợp ngữ cảnh.Kết quả: oops chỉ làm đúng một việc **phòng thủ nhẹ** — tắt SuperWarp vì trạng thái hệ thống đã đáng ngờ — mà không dám khẳng định “đã panic” (vì có thể chưa) và không cố ghi PC chi tiết (việc đó dành riêng cho panic thật, nơi có nhiều khả năng đây là cơ hội ghi log cuối cùng).

## Ba móc nối watchdog — và một canary có thể không bao giờ được gỡ
Một lỗi `sizeof` kinh điển, và một cơ chế tự sát 1 giây chưa chắc đã được tắt đúng lúc.

|**Hàm**|**Gọi từ đâu**|**Offset flash**|**Ghi lại**|
|---|---|---|---|
|`qspi_watchdog_fire()`|`softdog_reboot()` — `drivers/watchdog/softdog.c:207`, ngay trước `emergency_restart()`|`+0x29` (BSPTHR1/2)|1 byte cờ `0x01` — “watchdog đã fire”|
|`qspi_watchdog_device_info()`|`watchdog_record_info()` — `softdog.c:118` · `power/main.c:1074` (đường suspend/resume)|`+0x38` (BSPTHR1/2)|Mảng byte do caller truyền — xem lỗi bên dưới|
|`qspi_set_bh_b_state()`|Đường xử lý bất thường của `end_buffer_async_write()` khi cờ `BH_Async_Write` không được set (AR1071435)|`0x0026` (qua `THRG_setBspThrRegion`, **không** qua flash trực tiếp)|4 byte trạng thái bitmap của `buffer_head`|

### Lỗi `sizeof` trên tham số con trỏ — đã xác nhận, không còn là nghi vấn

```c
void qspi_watchdog_device_info( unsigned char *uc_data )     // uc_data: THAM SỐ CON TRỎ
{
    qspi_powerdown_write( ...BSPTHR1..., uc_data, sizeof(uc_data) );  // = sizeof(unsigned char*) = 8 trên arm64
    qspi_powerdown_write( ...BSPTHR2..., uc_data, sizeof(uc_data) );  // KHÔNG phải kích thước mảng caller truyền vào
}
```

```c
/* caller thật — drivers/watchdog/softdog.c, watchdog_record_info() */
unsigned char uc_data[7] = {};     // mảng THẬT trên stack chỉ có 7 byte
...
qspi_watchdog_device_info(uc_data);   // mảng decay thành con trỏ khi truyền vào
```

**Hệ quả cụ thể, không phải lý thuyết:** bên trong `qspi_watchdog_device_info()`, `sizeof(uc_data)` luôn bằng 8 (kích thước một con trỏ trên arm64) — bất kể caller thực sự cấp phát bao nhiêu byte. Với caller ở `softdog.c`, mảng thật chỉ dài **7 byte**, nên lệnh ghi 8 byte sẽ đọc **1 byte vượt quá biên mảng trên stack** — một byte rác từ frame stack liền kề bị đọc ra và ghi thẳng vào flash. Đây là hành vi không xác định theo chuẩn C (dù trên thực tế hầu như vô hại — chỉ rò rỉ đúng 1 byte stack cũ vào log chẩn đoán), và sẽ tái diễn giống hệt ở **bất kỳ caller nào khác** có mảng khác 8 byte, không chỉ riêng caller này.

### Vì sao `qspi_set_bh_b_state()` phải đẩy sang workqueue

```c
/* 2020/11/25 コ　OP_BTS-28498 QSPI割り込み関数を使用したことでLPPPがフリーズする不具合対応 */
/* qspi_powerdown_writeはLPPPとの排他制御を行わないため使用するとフリーズするため
   タスクを立てる方式の本関数で正式に変更 */
/* 実装完了です */
void qspi_set_bh_b_state(uint32_t ul_b_state)
{
//  qspi_powerdown_write(...);   // ← cách CŨ, đã bị chú thích bỏ vì gây treo LPPP thật
//  qspi_powerdown_write(...);
    ul_b_state_tmp = ul_b_state;
    INIT_WORK(&set_bh_b_state_work, set_bh_b_state_thread);
    schedule_work_on(1, &set_bh_b_state_work);   // ← cách MỚI: đẩy vào workqueue, chạy trong process context
}
```

Dịch nguyên văn ba dòng comment: _“2020/11/25 — sửa lỗi LPPP bị đông cứng do dùng hàm ngắt QSPI. Vì `qspi_powerdown_write` không loại trừ lẫn nhau với LPPP nên dùng nó sẽ gây treo — chính thức đổi sang cơ chế dựng task này. Đã hoàn tất triển khai.”_ Đây chính là bằng chứng lịch sử tôi đã trích ở prompt 19: caller này **đã từng** gọi thẳng `qspi_powerdown_write()` (dòng bị comment vẫn còn nguyên trong code), gây treo LPPP thật ngoài thực địa (mã lỗi `OP_BTS-28498`), và được sửa bằng cách chuyển sang `THRG_setBspThrRegion()` chạy trong workqueue — con đường **có đi qua** đồng bộ hóa R4 đầy đủ vì nó không còn bị ràng buộc “phải xong ngay lập tức, atomic context” như panic nữa.

### Một canary tự hủy có vẻ chưa từng được gỡ ngòi

```c
/* 2024/11/28 1071435_test_BSP — thêm mới */
static struct delayed_work reboot_worker;

static void reboot_worker_func(struct work_struct *work)
{
    /* 2024/11/28 1071435_test_BSP：本関数が実行されたら必ずBUG_ONを入れる */
    BUG_ON(1);   // nếu hàm này chạy → cố ý crash kernel
    /* BUG_ONがリセットとなるため、destroy_workqueueは実行しない */
}

static void set_bh_b_state_thread(struct work_struct *work)
{
    printk("set_bh_b_state_thread\n");
    INIT_DELAYED_WORK(&reboot_worker, reboot_worker_func);
    qw = alloc_workqueue("reboot_qw", WQ_MEM_RECLAIM, 0);
    queue_delayed_work(qw, &reboot_worker, HZ);   // hẹn giờ nổ sau đúng 1 giây

    uc_Ret = THRG_setBspThrRegion(...);   // công việc thật — ghi flash
    if (uc_Ret) { printk(...); }
    /* KHÔNG có cancel_delayed_work(&reboot_worker) ở đây, hay bất cứ đâu trong file */
}
```

**Tôi đã `grep` toàn bộ file tìm `cancel_delayed_work` — không một kết quả nào.** Đọc đúng như code viết: mỗi lần `set_bh_b_state_thread()` chạy, nó hẹn giờ `reboot_worker_func()` nổ sau đúng 1 giây (`HZ` jiffies), rồi làm công việc ghi flash thật, rồi return — **không hề tắt bộ hẹn giờ đó dù công việc chính đã xong thành công**. Nếu tôi đọc đúng, hệ quả là: **mọi** lần gọi `qspi_set_bh_b_state()` sẽ cố ý crash kernel qua `BUG_ON(1)` đúng 1 giây sau đó, bất kể thao tác ghi flash có thành công hay không.Có hai cách đọc tình huống này: **(a)** đây thực sự là một canary có chủ đích — “nếu bạn gọi hàm này, nghĩa là đã có bất thường (buffer_head sai trạng thái) đủ nghiêm trọng để buộc crash có kiểm soát nhằm lấy coredump”, và việc luôn crash là hành vi _mong muốn_; hoặc **(b)** đây là phần còn dang dở của một lần tái cấu trúc — tên biến `1071435_test_BSP` và hàng loạt khối `#if 0` vây quanh (loại bỏ vòng chờ `write_complete` cũ) gợi ý mạnh đây là code đang trong quá trình chỉnh sửa, và người viết có thể đã dự định thêm `cancel_delayed_work()` ở cuối `set_bh_b_state_thread()` nhưng chưa kịp — hoặc chưa đẩy lên bản build này.**Đáng để xác nhận trực tiếp với tác giả hoặc bằng cách chạy thử có kiểm soát** trước khi coi đây là hành vi cuối cùng — nhưng theo đúng những gì đọc được từ mã nguồn tại thời điểm này, đây là một quả bom hẹn giờ 1 giây gắn vào một hàm được gọi từ một đường xử lý bất thường I/O bình thường (không phải panic), và không có gì trong file vô hiệu hóa nó.

# 7. Debug thực chiến — triệu chứng → nguyên nhân
[Debug thực chiến mrvl-bspi](https://claude.ai/code/artifact/9deddc47-167a-48c2-9a10-74b35f53bb54)
Bốn quy trình dùng ngay khi có log thật hoặc board đang lỗi trên bàn — không lý thuyết, chỉ triệu chứng → hàm → điều kiện → hành động.

## 7.1. “bspi block busy, aborting. RetryLoop!!”

Đây là log của một cơ chế **có timeout hẳn hoi** — khác hẳn phần lớn vòng lặp còn lại trong file này.

```c
/* check_busy() — dòng 590 · mrvl_qspi_write() dòng 619-633 · mrvl_qspi_erase() dòng 694-708 */
int timeout = BUSY_CHK_TIMEOUT;              // = 10000, dòng 112: "1sec, check_busyのタイムアウト値"

if (check_busy(plat_data) & 1) {             // đọc SR1 qua execute_cmd(opcode=5)+get_byte, test bit WIP
    printk("bspi block busy, aborting. RetryLoop!!\n");
    do {
        if (!(check_busy(plat_data) & 1)) break;
        udelay(100);
        if (--timeout < 0) {
            printk("bspi block busy, aborting\n");   // KHÔNG có "RetryLoop!!" — dấu hiệu để phân biệt 2 dòng log
            return -ETIMEDOUT;
        }
    } while (1);
    printk("RetryLoop End!!\n");             // hồi phục thành công, tiếp tục ghi/xóa bình thường
}
```

![[Pasted image 20260824133158.png]]

Hai dòng log gần giống nhau nhưng khác nghĩa: có “RetryLoop!!” = mới phát hiện bận; không có = đã hết 1 giây mà vẫn bận.

**Điểm quan trọng nhất để hiểu đúng log này:** kiểm tra này chạy **trước khi** WREN hay lệnh ghi/xóa mới được gửi đi — nó không nói “thao tác vừa bắt đầu đang chậm”, mà nói **“flash đã báo bận từ trước khi driver kịp làm gì cả”**. Nghĩa là nguyên nhân nằm ở một giao dịch _trước đó_ chưa kết thúc sạch, hoặc một tác nhân khác đang chiếm chip.

|**Loại**|**Nguyên nhân**|**Liên hệ**|
|---|---|---|
|**Phần mềm**|Một lệnh `flash_erase`/write trước đó bị panic/watchdog cắt ngang giữa chừng, khiến FSM nội bộ chip không hoàn tất sạch|Tầng 05 — `qspi_powerdown_write()` không khóa, có thể chen ngang một giao dịch đang dở|
|**Phần mềm**|R4/LPPP đang tự thao tác trên CS1 đúng lúc CA72 muốn dùng CS0, khiến trạng thái `BSCR/NCSMUXSEL` bị nhiễu|Tầng 04 — hai lõi chia sẻ một khối thanh ghi, không có exclusion khi bỏ qua `prepare()`|
|**Phần mềm**|Ghi đồng thời vào cả hai `chip_select` (hai `struct spi_nor` độc lập, hai mutex riêng) làm lệch `NCSMUXSEL` giữa hai giao dịch chồng nhau|Tầng 03 — race giữa hai instance dùng chung `register_base`|
|**Phần cứng**|Chip flash thật sự vẫn đang hoàn tất một Erase 64 KiB/sector lớn từ ngay trước đó (tới vài trăm ms là bình thường theo datasheet)|Không phải lỗi — có thể chỉ cần retry đã đủ (log “RetryLoop End!!” xuất hiện)|
|**Phần cứng**|Chip flash lão hóa/cận hết chu kỳ ghi-xóa, thời gian Erase/Program vượt xa giá trị datasheet điển hình|Nếu lặp lại thường xuyên trên cùng một vùng địa chỉ, nghi ngờ hàng đầu|
|**Phần cứng**|Nhiễu tín hiệu trên CS/CLK/MISO khiến controller đọc sai byte trạng thái (bit WIP đọc nhầm thành 1 dù chip đã rảnh)|Kiểm tra bằng cách so log phần mềm với trạng thái thật trên logic analyzer (prompt 26)|
|**Phần cứng**|`BSACTIVE.BSPIACTIVE` chưa được set lại đúng sau một sự kiện lạ (power glitch, pinmux), khiến mọi lệnh command-mode đều thất bại và bit đọc về luôn là 1|Tầng 01 — thanh ghi driver không bao giờ tự set lại|

## Checklist treo máy khi ghi/xóa — xếp theo xác suất

Năm vòng lặp thật sự có thể là thủ phạm. Chỉ một trong số đó có timeout.

![[Pasted image 20260824133232.png]]

Xếp hạng dựa trên tích số “tần suất được gọi tới” × “mức độ phơi nhiễm với các lỗ hổng đã xác nhận ở tầng 01-05”. #1 và #2 đứng đầu vì lý do khác nhau: #1 vì bị gọi khắp nơi, #2 vì chắc chắn tuyệt đối khi bị chạm tới.

### Cách xác nhận từng cái
|**Nghi phạm**|**JTAG**|**Log (dmesg)**|**Logic analyzer (CS/CLK/IO0-3)**|
|---|---|---|---|
|**#1 `wait_cmd()`**|Halt CPU, đọc PC — sẽ nằm trong 3 lệnh của `wait_cmd()` (load BSSR, test bit, branch)|Dòng cuối cùng có thể là bất kỳ `printk` nào trước lệnh gây treo — không có tín hiệu riêng|**Im lặng tuyệt đối** trên cả 6 chân — controller đang chờ, không phát xung nào cả. Đây là chữ ký phân biệt với #3/#4.|
|**#2 Suspend-wait**|PC nằm trong vòng lặp gọi `qspi_write_then_read()` lặp lại|Dòng cuối là `"qspi_panic_happened() => SuperWarp Disable"`, `"watchdog reboot 1s wait"`, hoặc dòng `"...SPI-Flush write start"` của oops — rồi im bặt|Giao dịch **opcode `0x00` lặp lại liên tục**, mỗi lần cách nhau ~1 µs (từ `udelay(1)`) — chữ ký rất đặc trưng, khác hẳn RDSR (`0x05`) bình thường|
|**#3 Write/erase completion poll**|PC trong `qspi_read_reg()`, lặp gọi từ `wait_for_write_complete()` hoặc vòng erase|Dòng cuối là `"Write called addr..."` hoặc `"got into qspi erase start..."` nếu debug bật (prompt 26)|Giao dịch **opcode `0x05` (RDSR) lặp lại** — nếu kéo dài **vượt xa** thời gian Program/Erase tối đa theo datasheet (thường vài ms/program, ~400 ms/erase 4K), gần như chắc chắn là chip lỗi hoặc đọc sai bit|
|**#4 Powerdown-write per-page poll**|Giống #2 — PC trong `qspi_write_then_read()`, nhưng opcode gửi là `SPI_CMD_RDSR` (`0x05`) thật, không phải `0x00`|Cùng nhóm dòng log khẩn cấp như #2|RDSR lặp liên tục sau một trong các chuỗi ghi cờ BSPTHR ở tầng 05 — phân biệt với #2 bằng đúng opcode quan sát được (`0x05` thay vì `0x00`)|

## 7.2.  Ghi `/dev/mtdX` trả `-EIO` — phân biệt write-protect với phần cứng

Chỉ đúng hai dòng code trong toàn file trả `-EIO` lúc chạy. Cả hai đều đã lập bản đồ ở tầng 04.

|**Dòng**|**Hàm**|**Mã lỗi**|**Khi nào chạy**|
|--:|---|---|---|
|`606`|`mrvl_qspi_write()`|**`-EIO`**|Ghi trúng LPDDR4PARAM / BOOTROM / SECURE_AREA / LPPP khi `is_write_protect_lpddr4param=1`|
|`761`|`qspi_erase()`|**`-EIO`**|Xóa trúng LPDDR4PARAM / BOOTROM / LPPP — **không** gồm SECURE_AREA (lỗ hổng đã nêu ở tầng 04)|
|`629`|`mrvl_qspi_write()` — `check_busy` retry|**`-ETIMEDOUT`**|Flash vẫn báo bận sau 1 giây chờ (prompt 23/24)|
|`704`|`mrvl_qspi_erase()` — `check_busy` retry|**`-ETIMEDOUT`**|Tương tự, nhánh erase|
|`1010–1178`|`qspi_probe()` — nhiều điểm|`-1` / `-ENOMEM` / `-EBADF`|Chỉ xảy ra lúc **probe** (boot), không liên quan lỗi runtime khi đã có `/dev/mtdX`|

**Quy tắc chẩn đoán nhanh nhất: đọc đúng errno trước khi đọc bất kỳ log nào.** Cả hai lỗi runtime đều propagate sạch từ driver lên tới `write()`/`ioctl(MEMERASE)` phía userspace (đã xác nhận qua toàn bộ chuỗi `spi_nor_erase()`/`spi_nor_write()` ở tầng 03, không có tầng nào dịch lại mã lỗi):• `errno == EIO` → **chắc chắn** là write-protect, không phải phần cứng. Không cần soi bus.• `errno == ETIMEDOUT` → chuyển sang quy trình prompt 23/24 (chip bận/treo), không phải write-protect.

### Quy trình kiểm tra qua sysfs — ba bước

- **`cat /proc/mtd`**  
    Xem tên **PARTITION thật** — sẽ hiện đúng nhãn từ DT: `spi-romfs`, `spi-user`, `lppp-spi-firmware`. Đây là cách chắc chắn nhất để biết `/dev/mtdN` nào đang thao tác — không cần đoán qua tên node cha.
    
- **`cat /sys/devices/platform/quartz-sb/8e8273000.spi-flash{0,1}/write_protect_lpddr4param`**  
    Đọc cờ tổng. Sysfs này được tạo trên **cả hai** device (dòng ~1226, ngoài điều kiện `strstr`), nhưng cả hai đường dẫn cùng trỏ về **một biến toàn cục duy nhất** `is_write_protect_lpddr4param` — đọc/ghi ở đường nào cũng ra cùng giá trị. Ghi `0` để tắt tạm thời khi cần thao tác hợp lệ vào vùng bị chặn (ví dụ cập nhật LPDDR4PARAM thủ công ngoài các `c_BootType` được phép — tầng 04, prompt 16).
    
- **`cat /sys/devices/platform/quartz-sb/8e8273000.spi-flash0/flash_size`**  
    Chỉ tồn tại ở node tên `spi-flash0` — theo DT (tầng 00), node này có `chip_select=1`, tức là chip **LPPP 2MB**, không phải chip chứa romfs/user. Đây là bẫy đặt tên đã nêu ở tầng 00: số trong tên node ngược với số `chip_select` thật.

**Bẫy cần nhớ khi tra offset bị chặn theo địa chỉ:** bản đồ write-protect ở tầng 04 dùng `chip_select` (0 hoặc 1) làm khóa chính — **không** dùng tên node. Muốn biết một địa chỉ có đang bị chặn không, phải quy đổi đúng: `chip_select=0` → node DT `spi-flash1` (romfs+user, 4MB, chứa cả BOOTROM/LPDDR4PARAM/SECURE_AREA) — `chip_select=1` → node DT `spi-flash0` (LPPP, 2MB, toàn bộ chip bị chặn). Nếu bạn tra nhầm chiều, sẽ kết luận sai hoàn toàn vùng nào được bảo vệ.

## 7.3. Bật lại tầm nhìn — printk nào an toàn, printk nào phá timeout

21 khối `printk` bị tắt. Chỉ khoảng một nửa an toàn để bật lại.

Tất cả nằm trong `#ifndef CONFIG_KM_BIZHUB` — và vì board này luôn build với `CONFIG_KM_BIZHUB=y`, tất cả đều tắt vĩnh viễn trên binary thật. Ranh giới an toàn không nằm ở “bật hết” hay “tắt hết”, mà ở việc **printk đó có nằm trong một vòng lặp không có delay hay không**.

|**Vị trí**|**Nội dung log**|**Tần suất thật**|**Đánh giá**|
|---|---|---|---|
|`execute_cmd():217`|`"cmd %x"`|1 lần / lệnh dispatch (không phải mỗi byte)|**An toàn**|
|`mrvl_qspi_write():616`|`"Write called addr %x size %d"`|1 lần / lệnh `write()` từ MTD|**An toàn**|
|`mrvl_qspi_erase():687`|`"got into qspi erase start %x"`|1 lần / lệnh `erase()` từ MTD|**An toàn**|
|`qspi_write_reg():305`|`"write reg opcode %x"` (chỉ dòng này, **không** kèm vòng dump bên dưới)|1 lần / lệnh `write_reg` (WREN mỗi trang khi ghi lớn — vẫn chấp nhận được)|**An toàn**|
|`qspi_read_reg():407`|`"got into read_reg opcode %x"` (chỉ dòng này)|1 lần / lệnh `read_reg` — **nhưng** hàm này bị gọi trong vòng poll trần của `wait_for_write_complete()`|**Rủi ro trung bình**|
|`readid():786–792`|Dump đầy đủ 20 byte JEDEC ID|Chỉ chạy trong `nor_setup()` lúc probe — đúng 2 lần/board|**An toàn**|
|`qspi_write_reg():308–312`|Dump từng byte trong vòng `for`|N lần/lệnh `write_reg` — không nằm trong poll trần nhưng cộng dồn khi ghi nhiều trang|**Rủi ro thấp–trung**|
|`qspi_read_reg():420–422`|Dump từng byte trong vòng `for`|**Nằm trực tiếp trong vòng lặp KHÔNG delay của `wait_for_write_complete()`** — có thể in hàng nghìn dòng liên tiếp trong một lần ghi|**NGUY HIỂM**|
|`disable_clk_r4():132`|Không phải `printk` — `usleep_range` thay `udelay`|—|Không liên quan log, bỏ qua|

**Vì sao dòng 420-422 nguy hiểm thật sự, không chỉ về nguyên tắc.** `wait_for_write_complete()` (dòng 505-511) gọi `qspi_read_reg(nor, 0x05, buf, 1)` trong một `do-while` **hoàn toàn không có delay** giữa các lần lặp — nếu bật full-trace ở đây, mỗi micro-giây poll sẽ sinh ra một dòng `printk` qua UART console (vốn chậm, cỡ vài chục µs tới vài ms mỗi dòng tùy tốc độ baud). Kết quả: bản thân việc log **làm chậm hẳn vòng poll**, có thể biến một Program 1ms thành hàng trăm ms chỉ vì driver bận in log — và ngập tràn dmesg tới mức mất luôn dòng log thật sự cần tìm.\

### Đề xuất vá an toàn — macro debug cục bộ, không đụng `CONFIG_KM_BIZHUB`

Không thể đơn giản đổi `#ifndef CONFIG_KM_BIZHUB` thành luôn-bật, vì macro đó chi phối hàng trăm chỗ khác trong toàn bộ cây kernel (đã thấy xuyên suốt các tầng trước) — tắt nó đi cho cả file sẽ vô tình bật lại rất nhiều nhánh `#else` chưa từng chạy (kể cả những nhánh chết đã xác định ở tầng 02/05, ví dụ `check_jedecid()`!). Thay vào đó, thêm một macro cục bộ chỉ trong file này:

```c
/* thêm gần đầu file, cạnh #define BUSY_CHK_TIMEOUT */
#define QSPI_TRACE_SAFE 0      /* đổi thành 1 khi cần soi bus, nhớ đổi lại 0 trước khi build bản phát hành */

/* rồi bọc lại đúng 6 dòng "an toàn" đã liệt kê ở trên bằng: */
#if QSPI_TRACE_SAFE
    printk("cmd %x\n", cmd->command);
#endif
    /* … và TUYỆT ĐỐI không đụng tới khối 420-422 hay bất kỳ printk nào khác
   nằm trong wait_for_write_complete() / vòng erase-completion / vòng Suspend */
```

### Log nào ghép tốt nhất với logic analyzer bắt trên CS/CLK/IO0-3

| **Log**                                                             | **Tương ứng trên bus**                                                                                            | **Cách dùng**                                                                                                                      |
| ------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| `"Write called addr %x size %d"` / `"got into qspi erase start %x"` | Điểm bắt đầu của **toàn bộ** chuỗi giao dịch (WREN → opcode → địa chỉ → dữ liệu → poll)                           | Neo thời điểm bắt đầu capture — đối chiếu timestamp dmesg với cạnh xuống đầu tiên của CS trên analyzer                             |
| `"cmd %x"` (`execute_cmd`)                                          | Đúng byte opcode sắp xuất hiện ngay sau khi CS xuống, trước 8 xung CLK đầu tiên                                   | Log hữu ích nhất để xác nhận đúng opcode nào đang chạy — so trực tiếp giá trị hex với byte đầu tiên bắt được trên MOSI/IO0         |
| `"write reg opcode %x"`                                             | Nhận diện các giao dịch ngắn không có pha địa chỉ/dữ liệu (WREN=`0x06`, WRDI=`0x04`) — chỉ opcode rồi CS lên ngay | Đếm số khung WREN trên bus, đối chiếu với số trang đã ghi — WREN kép (tầng 03) sẽ hiện ra ở đây nếu đối chiếu với log của `core.c` |

# 8. Săn lỗi — kiểm chứng nghi vấn
[Săn lỗi mrvl-bspi](https://claude.ai/code/artifact/907d1899-5cba-48a2-9bd5-200664e862df)
Năm nghi vấn nêu ra ở đầu lộ trình, giờ đối chiếu lại với bằng chứng đã thu thập xuyên suốt tầng 00-06. Cả năm đều đã có đủ căn cứ để kết luận — không còn điểm nào dừng ở mức “có thể”.

![[Pasted image 20260824133538.png]]

## SECURE_AREA được bảo vệ khi ghi, bỏ ngỏ khi xóa
Đã đối chiếu nguyên văn cả hai khối điều kiện — không có gì để tranh cãi thêm.

Xác nhận mrvl-bspi_winbond.c:599-603 (ghi) so với 755-758 (xóa)


```c
/* mrvl_qspi_write() — dòng 599-603 — BỐN điều kiện */
if( is_write_protect_lpddr4param &&
    ( (cs==LPDDR4PARAM_CHIP_SELECT && ...) ||
      (cs==BOOTROM_CHIP_SELECT     && ...) ||
      (cs==BOOTROM_CHIP_SELECT     && start_addr<SECURE_AREA_END_ADR && (start_addr+size-1)>=SECURE_AREA_START_ADR) ||
      (cs==LPPP_CHIP_SELECT        && ...) ) ) { ...; return -EIO; }
```
```c
/* qspi_erase() — dòng 755-758 — chỉ BA điều kiện */
if( is_write_protect_lpddr4param &&
    ( (cs==LPDDR4PARAM_CHIP_SELECT && ...) ||
      (cs==BOOTROM_CHIP_SELECT     && ...) ||
      /* KHÔNG có nhánh SECURE_AREA ở đây */
      (cs==LPPP_CHIP_SELECT        && ...) ) ) { ...; return -EIO; }
```

**Vì sao lỗi này sinh ra — đọc từ lịch sử comment.** Comment `/* 2024/08/21 Sky - セキュアブート対応　保護領域追加 */` (thêm vùng bảo vệ cho secure boot) chỉ xuất hiện **một lần duy nhất**, ngay phía trên khối 599-603 của `mrvl_qspi_write()`. Khối 755-758 của `qspi_erase()` hoàn toàn không có comment tương ứng, không có dấu vết chỉnh sửa nào cùng ngày. Cách đọc hợp lý nhất: khi thêm bảo vệ SECURE_AREA, người viết chỉ sửa **một trong hai** hàm write-protect song song trong cùng file — quên mất rằng erase cũng cần được vá y hệt.

**Hệ quả cụ thể, đã kiểm chứng qua toàn bộ chuỗi gọi (tầng 03):** một lệnh `ioctl(MEMERASE)` nhắm vào bất kỳ sector nào giao với `0x3FD000-0x3FE000` trên chip_select 0 sẽ đi thẳng qua `spi_nor_erase()` → `spi_nor_erase_sector()` → `controller_ops->erase()` = `qspi_erase()` mà **không hề bị chặn** — trong khi cùng vùng đó, một `write()` trực tiếp lại bị từ chối ngay với `-EIO`. Với một vùng đặt tên tường minh “SECURE_AREA” phục vụ secure boot, đây là đường vòng thực sự: kẻ tấn công hoặc script lỗi không cần ghi đè nội dung — chỉ cần **xóa sạch** sector chứa vùng đó (đặt toàn bộ về `0xFF`) là đủ vô hiệu hóa cơ chế secure boot dựa trên vùng này, mà driver không phát hiện được.

**Bản vá tối thiểu:** chép nguyên nhánh thứ ba của khối 599-603 sang khối 755-758, đổi `size-1` thành `nor->mtd.erasesize-1` cho khớp kiểu tham số của `qspi_erase(struct spi_nor *nor, loff_t offs)` — đúng cách `LPDDR4PARAM`/`BOOTROM` đã làm ở hai nhánh còn lại của cùng hàm. Không đổi hành vi gì khác.

## `sizeof(uc_data)` trên tham số con trỏ

Kinh điển tới mức có tên riêng trong giới C: “sizeof-on-pointer bug”.

Xác nhận: mrvl-bspi_winbond.c:1649-1653 · caller: drivers/watchdog/softdog.c:106-119

```c
void qspi_watchdog_device_info( unsigned char *uc_data )   // uc_data: THAM SỐ CON TRỎ, không phải mảng
{
    qspi_powerdown_write( ...BSPTHR1_WATCHDOG_DEV_INFO..., uc_data, sizeof(uc_data) );
    qspi_powerdown_write( ...BSPTHR2_WATCHDOG_DEV_INFO..., uc_data, sizeof(uc_data) );
}
```

|**Câu hỏi**|**Trả lời — đã xác minh**|
|---|---|
|`sizeof(uc_data)` thực sự là bao nhiêu trên arm64?|`sizeof(unsigned char*)` = **8 byte** — kích thước một con trỏ 64-bit, không liên quan gì tới dữ liệu trỏ tới|
|Có khớp ý đồ ban đầu không?|**Chắc chắn không** — tên hàm và comment đều mô tả “ghi thông tin thiết bị” theo nội dung caller cung cấp, không phải “luôn ghi đúng 8 byte cố định”|
|Caller thật truyền mảng bao nhiêu byte?|`watchdog_record_info()` — `softdog.c:108` — khai báo `unsigned char uc_data[**7**] = {};`, chỉ set 4 byte đầu (`[0]=0xFF`, `[1..3]` = mã CPU/điểm dừng)|
|Hậu quả: ghi thiếu hay ghi thừa?|**Ghi thừa 1 byte** — đọc `uc_data[7]`, một địa chỉ nằm **ngoài biên** mảng 7 phần tử, rồi ghi giá trị rác đó (byte kề trên stack) vào flash cùng 7 byte hợp lệ|

**Mức độ nghiêm trọng thực tế: thấp về an toàn, nhưng là lỗi thật.** Đọc tràn 1 byte trên stack trong ngữ cảnh này khó gây crash (không có bảo vệ trang ở granularity nhỏ vậy, và stack frame của `watchdog_record_info()` gần như chắc chắn còn dư chỗ) — hậu quả thực tế chỉ là **1 byte rác từ stack cũ bị ghi lẫn vào log chẩn đoán trên flash**, làm sai lệch nhẹ dữ liệu watchdog dùng để điều tra sự cố reboot sau này. Đây là hành vi không xác định (UB) theo chuẩn C dù hậu quả quan sát được nhẹ, và lỗi **tái diễn ở mọi caller khác** nếu ai đó sau này gọi hàm với mảng khác 8 byte — không chỉ riêng `softdog.c`.

**Bản vá tối thiểu:** thêm tham số `uint32_t ul_size` vào chữ ký hàm, để caller tự khai kích thước mảng thật của mình (`qspi_watchdog_device_info(uc_data, sizeof(uc_data))` — lúc đó `sizeof` đứng đúng chỗ, trong scope của caller nơi `uc_data` vẫn còn là kiểu mảng thật, chưa decay thành con trỏ), rồi dùng `ul_size` thay `sizeof(uc_data)` ở cả hai lời gọi `qspi_powerdown_write()`.

## Canary `BUG_ON(1)` hẹn giờ 1 giây — chưa thấy nơi hủy lịch

Xác nhận được phần “có tồn tại và có kích hoạt được”. Phần “có luôn nổ hay không” cần người viết code xác nhận trực tiếp.

Xác nhận có điều kiện: mrvl-bspi_winbond.c:1657-1734

|**Câu hỏi**|**Trả lời**|
|---|---|
|**Có nằm trong build production không?**|Toàn khối bọc trong `#if defined(CONFIG_KM_BIZHUB)` (dòng 1657) rồi `#ifdef CONFIG_THREADDRV` (dòng 1668) — **cả hai cấu hình đều bật** trên board Quartz thật (đã xác nhận `CONFIG_KM_BIZHUB=y` xuyên suốt cả lộ trình; `CONFIG_THREADDRV` chi phối phần lớn hệ thống chẩn đoán KM). Đây **là** code chạy trên bản phát hành, không phải nhánh chết.|
|**Đường nào kích hoạt tới đó?**|`end_buffer_async_write()` phát hiện cờ `BH_Async_Write` không được set trên một `buffer_head` (bất thường I/O bình thường, gốc từ lỗi thật `AR1071435` năm 2019 — “Oops khi nhận email”) → gọi `qspi_set_bh_b_state()` → `schedule_work_on()` → `set_bh_b_state_thread()` → hẹn `reboot_worker` nổ sau `HZ` (1 giây) → gọi `THRG_setBspThrRegion()` ghi flash thật.|
|**Có phải code test còn sót lại?**|Tên biến/comment tự gắn nhãn `1071435_test_BSP` ở **cả bốn** vị trí liên quan (khai báo `reboot_worker`, thân `reboot_worker_func`, lời gọi `queue_delayed_work`, và ba khối `#if 0` xóa code cũ) — tên gọi rất giống nhãn debug tạm, nhưng nó vẫn **đang biên dịch và chạy được**, bất kể tên gọi ngụ ý gì.|
```c
static void reboot_worker_func(struct work_struct *work)
{
    /* 本関数が実行されたら必ずBUG_ONを入れる */
    BUG_ON(1);   // cố ý crash nếu hàm này CHẠY
    /* BUG_ONがリセットとなるため、destroy_workqueueは実行しない */
}

static void set_bh_b_state_thread(struct work_struct *work)
{
    printk("set_bh_b_state_thread\n");
    INIT_DELAYED_WORK(&reboot_worker, reboot_worker_func);
    qw = alloc_workqueue("reboot_qw", WQ_MEM_RECLAIM, 0);
    queue_delayed_work(qw, &reboot_worker, HZ);   // hẹn giờ — bắt đầu đếm ngược ngay tại đây

    uc_Ret = THRG_setBspThrRegion(...);   // việc chính: ghi flash
    if (uc_Ret) { printk(...); }
    // ...hàm kết thúc. Không có cancel_delayed_work(&reboot_worker) ở đây.
}
```

**Điều tôi có thể khẳng định chắc chắn:** đã `grep` toàn bộ file cho `cancel_delayed_work` — **không một kết quả nào**, ở bất kỳ hàm nào, không riêng hai hàm này. Không có nhánh nào trong `set_bh_b_state_thread()` hủy lịch `reboot_worker`, kể cả sau khi `THRG_setBspThrRegion()` trả về thành công.**Điều tôi KHÔNG thể khẳng định chắc chắn từ cây nguồn này:** liệu `THRG_setBspThrRegion()` (định nghĩa ở file khác, `mtd_flash_access.c`) có nội bộ hủy work này bằng một cách gián tiếp nào không (không thấy khả năng đó trong chữ ký hàm `unsigned char THRG_setBspThrRegion(const uint32_t, void*, uint32_t)` — không có tham số nào cho phép nó biết về `reboot_worker`) — hoặc liệu đây có phải hành vi _được thiết kế_: “gọi hàm này tức là uỷ quyền cho hệ thống buộc crash 1 giây sau để lấy coredump, bất kể kết quả ghi flash”. Cả hai khả năng đều có thể tự nhất quán với code đang thấy.

**Việc cần làm không phải “sửa ngay” mà là “hỏi đúng người”:** xác nhận trực tiếp với tác giả đoạn `1071435_test_BSP` (hoặc lịch sử commit đầy đủ ngoài phạm vi cây nguồn có ở đây) xem `BUG_ON(1)` vô điều kiện có phải hành vi mong muốn không. Nếu KHÔNG mong muốn, bản vá là thêm `cancel_delayed_work_sync(&reboot_worker)` ngay sau khi `THRG_setBspThrRegion()` trả về (thành công lẫn thất bại có kiểm soát), trước khi hàm kết thúc.

## `wait_for_write_complete()` — comment tự thú, chưa ai vá

Hiếm khi thấy một lỗi được chính tác giả ghi chú ngay tại chỗ rồi để nguyên suốt nhiều năm.

Xác nhận: mrvl-bspi_winbond.c:501-511

```c
static int wait_for_write_complete(struct spi_nor *nor)
{
    unsigned char buf[10];
    // put a timeout here

    do
    {
        qspi_read_reg(nor, 0x05, buf, 1);          // read the flag status register
    } while (buf[0] & 0x01);
    return 0;
}
```

**Đối chiếu trực tiếp với `check_busy()` trong cùng file (dòng 574-590):** hàm đó bọc quanh y hệt một thao tác — đọc SR1, kiểm tra bit WIP — nhưng có hẳn `BUSY_CHK_TIMEOUT` (10000 × udelay(100) = 1 giây, trả `-ETIMEDOUT` khi hết giờ). Cùng file, cùng tác giả nhiều khả năng, cùng loại thao tác — một cái được bọc timeout cẩn thận, một cái thì không, kèm comment _tự ghi chú việc còn thiếu_ ngay tại chỗ. Đây không phải sơ suất không nhận ra — đây là việc **đã biết nhưng chưa làm**.

**Comment còn sai cả tên:** “read the flag status register” nhưng opcode dùng là `0x05` (RDSR — Status Register-1 chuẩn), không phải Flag Status Register (opcode `0x70`, chỉ tồn tại trên các chip kiểu Micron — chính là opcode dùng ở nhánh `#else` non-KM_BIZHUB của `mrvl_qspi_erase()`, tầng 06). Dấu hiệu cho thấy đoạn code này được chuyển thể từ một phiên bản khác (rất có thể là driver gốc hướng Micron) mà không cập nhật lại comment.

### Rủi ro cụ thể — không phải lý thuyết suông

Hàm này nằm trong vòng lặp theo trang của `mrvl_qspi_write()` (tầng 01/06): `while(current_xfer_size < size) { ...; wait_for_write_complete(&plat_data->nor); }` — gọi **một lần mỗi trang 256 byte**. Một lần ghi lớn (ví dụ cập nhật firmware nhiều trăm KB) gọi hàm này hàng trăm tới hàng nghìn lần. Chỉ cần **một** trong số đó gặp chip không tự xóa được bit WIP (chip lão hóa, lỗi tín hiệu khiến đọc sai bit, hoặc race với R4/panic-write đã nêu ở các tầng trước) — toàn bộ tiến trình ghi treo vĩnh viễn tại đúng trang đó, không có cách nào tự thoát.

### Bản vá tối thiểu — hai phần, không đổi timing đường bình thường

```c
#define WRITE_COMPLETE_TIMEOUT 5000   /* xem giải thích ngân sách bên dưới */

static int wait_for_write_complete(struct spi_nor *nor)
{
    unsigned char buf[10];
    int spins = 200;              // pha 1: quay nhanh, không delay — Page Program điển hình xong dưới 1ms
    int timeout = WRITE_COMPLETE_TIMEOUT;  // pha 2: có giới hạn, chỉ vào khi pha 1 đã hết mà vẫn bận

    do
    {
        qspi_read_reg(nor, 0x05, buf, 1);          // read the status register (đã sửa lại tên comment)
        if (!(buf[0] & 0x01)) return 0;             // ← đường thoát mới, giống hệt hành vi cũ khi chip khỏe
        if (spins) { spins--; continue; }
        if (--timeout < 0) {
            WARN_ONCE(1, "bspi: wait_for_write_complete timeout, SR1=0x%02x\n", buf[0]);
            return -ETIMEDOUT;                      // ← đường thoát mới khi thật sự treo
        }
        udelay(10);
    } while (1);
}
```

```c
/* caller — mrvl_qspi_write(), trong vòng lặp theo trang */
regs->BSCMDR = 0x300;	// drop chip select
wait_for_write_complete(&plat_data->nor);
if (wait_for_write_complete(&plat_data->nor) < 0) {
    execute_cmd(regs, &plat_data->write_disable, plat_data->chip_select);
    regs->BSCMDR = 0x300;
    regs->BSCR = bscr;
    disable_clk_r4(false);
    return -ETIMEDOUT;   // dừng hẳn thay vì âm thầm ghi tiếp trang kế, dữ liệu đã hỏng
}
```

**Vì sao chia hai pha, giống hệt triết lý bản vá `wait_cmd()` đã đề xuất ở tầng 01:** Page Program trên chip Winbond dòng này điển hình xong dưới 1ms — pha quay nhanh không delay giữ nguyên tốc độ đường bình thường tuyệt đối. Ngân sách `WRITE_COMPLETE_TIMEOUT=5000 × udelay(10)` ≈ 50ms tổng — rộng rãi so với mọi giá trị tối đa trong datasheet, không bao giờ chặn nhầm một thao tác đang chạy đúng.**Phần caller quan trọng không kém phần hàm:** hiện tại dù `wait_for_write_complete()` có trả lỗi, `mrvl_qspi_write()` cũng **không hề kiểm tra giá trị trả về** — vòng lặp cứ thế sang trang kế tiếp như không có gì xảy ra, ghi đè lên một chip đang ở trạng thái không xác định. Bản vá phải sửa cả hai phía: hàm chờ phải biết dừng, và caller phải biết dừng theo.

## Vòng chờ Suspend trong `qspi_powerdown_write()`
Phát hiện nghiêm trọng nhất toàn bộ lộ trình — đã trình bày đầy đủ ở tầng 05, đây là bản xác nhận rút gọn cuối cùng.

=> Xác nhận — đảm bảo 100% về mặt toán học

mrvl-bspi_winbond.c:1354-1381 (khởi tạo bị loại) · 1418-1424 (lời gọi bị vô hiệu) · 1449-1457 (nơi treo)

static unsigned char uc_suspend_command = 0, uc_readstatus_command = 0, uc_suspend_bit = 0;  // dòng 1354-1356

```c
#ifndef CONFIG_KM_BIZHUB                        // ← file CHỈ build khi CONFIG_KM_BIZHUB=y (Makefile, tầng 00)
static int check_jedecid( unsigned long jedec_id )    // hàm DUY NHẤT gán giá trị thật (0x75, 0x35, 0x80 cho Winbond)
{ ... uc_suspend_command = 0x75; ... }
#endif                                                 // → KHÔNG TỒN TẠI trong binary thật

#if 0	/* SPI-Flash種別対応 T.B.D */                // dòng 1418 — lời gọi duy nhất, vô hiệu hóa VĨNH VIỄN, mọi cấu hình
if( check_jedecid( ul_jedecid[1] ) < 0){ ... }
#endif
```

```c
/* → ba biến giữ nguyên giá trị 0 trong SUỐT vòng đời kernel */

tx[0] = uc_suspend_command;               // = 0x00, không phải 0x75
qspi_write_then_read(regs, tx, 1, rx, 0);

do{
    udelay(1);
    tx[0] = uc_readstatus_command;         // = 0x00, không phải 0x35
    qspi_write_then_read(regs, tx, 1, rx, 1);
}while( (rx[0] & uc_suspend_bit) == 0 );   // uc_suspend_bit=0x00 → (rx[0] & 0)==0 LUÔN ĐÚNG, mọi giá trị rx[0]
```

| **Câu hỏi**                                                | **Trả lời dứt khoát**                                                                                                                                                                                                                                                                  |
| ---------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `uc_suspend_command` và `uc_suspend_bit` có luôn bằng `0`? | **Có** — đã xác nhận bằng cách đọc trực tiếp cả nơi khởi tạo (1354–1356), nơi gán giá trị thật duy nhất (1358–1380, bị `#ifndef CONFIG_KM_BIZHUB` loại khỏi build), và lời gọi duy nhất (1420, bị `#if 0` vô hiệu hóa độc lập với mọi cấu hình).                                       |
| Vòng `while` hành xử ra sao với `(rx[0] & 0) == 0`?        | Đúng với **mọi** giá trị `rx[0]` có thể có (0–255) — không có giá trị nào của byte trả về khiến điều kiện sai. Vòng lặp không bao giờ tự thoát bằng con đường tính đúng của nó.                                                                                                        |
| **Ý nghĩa trên đường xử lý panic?**                        | Chuỗi cứu dữ liệu chẩn đoán cuối cùng (`qspi_panic_happened` / `oops_happened` / `watchdog_fire`) có thể tự **treo vĩnh viễn** đúng khi panic trùng thời điểm CS0 đang Erase dở (>5 ms) — không có `CONFIG_SOFTLOCKUP_DETECTOR` nào báo động (tầng 01), hệ thống chỉ lặng lẽ dừng lại. |
**Vì sao đây là phát hiện nặng nhất trong cả 31 prompt, không chỉ riêng tầng này.** Bốn phát hiện còn lại của tầng 07 đều có đặc điểm chung: cần một điều kiện kích hoạt cụ thể (ghi/xóa đúng vùng, gọi hàm với đúng kiểu tham số, một luồng xử lý bất thường cụ thể). Phát hiện này khác về bản chất — một khi luồng thực thi chạm đúng nhánh `if`, **không có bất kỳ giá trị runtime nào** (trạng thái chip, tốc độ bus, phiên bản firmware, hay may mắn) có thể khiến nó thoát ra. Đây là lỗi được chứng minh bằng đại số Boolean tĩnh, không phải bằng thống kê xác suất.

**Bản vá tối thiểu, ít rủi ro nhất:** gán cứng ba giá trị Winbond ngay tại chỗ khai báo, bỏ hẳn phụ thuộc vào `check_jedecid()` (vốn đã chết) — vì toàn bộ hệ thống chỉ từng thấy chạy trên chip Winbond (0xEF) theo `nor_setup()` đã xác nhận ở tầng 02:

```c
static unsigned char uc_suspend_command   = 0x75;  /* Erase/Program Suspend — Winbond */
static unsigned char uc_readstatus_command = 0x35;  /* Read Status Register-2 */
static unsigned char uc_suspend_bit        = 0x80;  /* SUS bit */
```

Đồng thời nên thêm một giới hạn vòng lặp (giống mọi bản vá khác trong lộ trình này) cho `do-while` chờ SUS bit ở dòng 1453-1457 — kể cả với ba giá trị đúng, đây vẫn là một vòng poll không timeout, chạy trong chính đường khẩn cấp không được phép tự treo.

## Khép lại lộ trình 31 prompt

Từ bit map thanh ghi BOOTSPI (tầng 01) tới quả bom hẹn giờ trong workqueue (tầng 07), toàn bộ driver `mrvl-bspi_winbond.c` giờ đã được truy vết đầy đủ: kiến trúc, giao thức trên dây, mối nối với framework, các ràng buộc đặc thù Quartz/KM, đường khẩn cấp, quy trình debug thực chiến, và cuối cùng là năm lỗi cụ thể — bốn cái có bản vá tối thiểu đã đề xuất sẵn, một cái (prompt 29) cần xác nhận chủ đích trước khi động vào.

Nếu cần đi sâu thêm vào bất kỳ điểm nào — ví dụ triển khai thật các bản vá đã đề xuất, hoặc mở rộng sang phần `drivers/mfp/mtd_flash/` (nơi `THRG_setBspThrRegion`/`THRG_getBspThrRegion` thực sự sống) — cứ tiếp tục từ đây.