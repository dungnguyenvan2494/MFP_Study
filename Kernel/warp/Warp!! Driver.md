Phân tích driver hibernation tuỳ biến "Warp!!" của Lineo Solutions — thay thế swsusp chuẩn của Linux bằng một cơ chế snapshot RAM riêng, dùng cho phần cứng nhúng (Konica Minolta bizhub). Tài liệu này liệt kê toàn bộ function, sơ đồ luồng gọi cấp cao, hành vi từng function, và cơ chế snapshot vào bộ nhớ / thiết bị lưu trữ.

# 1. Danh mục function & mục đích
warp.c gồm khoảng 45–50 function, chia thành 9 nhóm theo vai trò trong pipeline hibernate. Các accessor `/proc` sinh ra bởi macro `PROC_RW`/`PROC_READ` (stat, error, retry, canceled, saveno, loadno, switch, compress, shrink, separate, oneshot, halt, silent…) được gộp thành một dòng vì chúng đều là cặp đọc/ghi số nguyên gần như giống hệt nhau.

## A · Vòng đời module & giao diện /procboot & sysadmin
| Function                                        | Mục đích                                                                                                                                               |
| ----------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------ |
| warp_init                                       | Entry point `module_init`: tạo cây `/proc/warp/*`, nạp giá trị mặc định từ Kconfig (saveno, loadno, compress, shrink…), map cmdline console.           |
| warp_exit                                       | Entry point `module_exit`: gỡ toàn bộ entry `/proc`, giải phóng buffer nếu cấp phát tĩnh.                                                              |
| warp_register_machine / warp_unregister_machine | API xuất ra ngoài để driver đặc thù từng board (machine‑specific) đăng ký/huỷ bảng callback `warp_ops` — đây là điểm "cắm" board vào lõi warp.c chung. |
| warp_proc_create                                | Wrapper tạo một entry `/proc/warp/<name>`, tương thích nhiều version kernel (proc_create cũ/mới).                                                      |
| read_proc_warp / write_proc_warp                | Helper chung: đọc/ghi một số nguyên qua `/proc` kèm kiểm tra biên (min/max).                                                                           |
| read/write_proc_warp_division                   | Đọc/ghi tỉ lệ lưu snapshot theo từng CPU (chuỗi số phân tách bởi dấu phẩy) — dùng khi chia nhỏ dữ liệu snapshot cho nhiều CPU.                         |
| read_proc_warp_secure_bf_dev / _offset          | Chỉ khi `CONFIG_PM_WARP_SECURE_BOOT`: phơi ra thiết bị/offset của bootflag đã ký, để bootloader đọc ở lần khởi động kế tiếp.                           |
| 13 accessor sinh bởi PROC_RW/PROC_READ          | Đọc/ghi các tham số điều khiển: `stat, error, retry, canceled, saveno, loadno, switch, compress, shrink, separate, oneshot, halt, silent`.             |
## B · Sysfs debug kobjectobservability

| Function                   | Mục đích                                                                                                        |
| -------------------------- | --------------------------------------------------------------------------------------------------------------- |
| make_kobject               | Cấp phát + đăng ký một `kobject` tuỳ biến dưới `/sys/kernel/warp_kobj`.                                         |
| warp_show_module_parameter | Callback "show" cho sysfs: in `dramtbl_max`, `dramtbl_num`, `ngPoint` (điểm lỗi debug) ra file sysfs tương ứng. |
| warp_kobj_release          | Callback release bắt buộc của kobject (chỉ log, không giải phóng gì thêm).                                      |
## C · Vận chuyển dữ liệu cấp thấp mem / block dev / MTD

| Function                            | Mục đích                                                                                        |
| ----------------------------------- | ----------------------------------------------------------------------------------------------- |
| warp_load_mem                       | Đọc bằng `memcpy` từ một địa chỉ RAM cố định (nguồn WARP_LOAD_MEM).                             |
| warp_dev_open                       | Mở block device theo tên, tự thêm tiền tố `/dev/` hoặc `/dev/block/` (Android).                 |
| warp_load_dev                       | Đọc dữ liệu từ block device tại offset theo sector.                                             |
| warp_save_dev                       | Ghi dữ liệu ra block device (chỉ biên dịch trên x86 — thường dùng để test trên máy phát triển). |
| warp_mtd_offs                       | Dò offset tiếp theo trên MTD không phải bad‑block.                                              |
| warp_load_mtd                       | Đọc dữ liệu từ thiết bị MTD (flash), tự động nhảy qua block lỗi.                                |
| warp_load_mtd_no / warp_load_mtd_nm | Wrapper mở MTD theo số partition hoặc theo tên rồi gọi warp_load_mtd.                           |
| warp_putc / warp_printf             | Debug: in từng ký tự qua console cấp thấp của board (chỉ khi `CONFIG_PM_WARP_DEBUG`).           |
## D · Nạp driver & bootflagtrước điểm‑không‑quay‑lại

|Function|Mục đích|
|---|---|
|warp_get_drv_size|Đọc header của từng driver blob để xác thực magic ID và tính kích thước thật (kể cả kích thước chữ ký secure‑boot).|
|warp_load_drv_low|Nạp một blob driver/UserAPI đơn lẻ bằng đúng phương thức cấu hình cho nó (mem / dev / MTD).|
|warp_load_drv|Nạp toàn bộ hibernation driver + các UserAPI driver vào buffer làm việc, sắp liền kề theo trang, cập nhật bảng địa chỉ vật lý `warp_param.drv_phys[]`.|
|warp_load_bf_low|Nạp dữ liệu bootflag thô theo cấu hình của savearea tương ứng.|
|warp_load_bf|Nạp + xác thực khối bootflag (kèm giải mã header secure‑boot nếu bật).|
## E · Vòng đời vùng làm việc (workspace)setup / teardown

|Function|Mục đích|
|---|---|
|warp_work_alloc|Cấp phát 3 buffer scratch: `hd_savearea`, `warp_drv_buf`, `warp_work`; đăng ký PFN của chúng vào `warp_nosave_work[]`.|
|warp_work_free|Giải phóng 3 buffer trên.|
|warp_work_init|Điều phối toàn bộ setup workspace: remap driver cố định, cấp phát, nạp driver + bootflag, chia layout 3 bảng save‑table, tạo sysfs debug.|
|warp_work_exit|Gỡ mapping driver cố định (nếu có), gọi warp_work_free.|
## F · Xây bảng lưu (save table)what‑to‑snapshot

| Function                                 | Mục đích                                                                                                                                      |
| ---------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------- |
| warp_set_tbl / warp_set_tbl64            | Helper cấp thấp: thêm một dải (start,end) vào bảng, gộp với phần tử liền trước nếu liên tiếp.                                                 |
| warp_set_savearea                        | **API xuất ra ngoài**: đăng ký một dải bộ nhớ 64‑bit (vùng >4 GB) cần lưu vào `exttbl`, tự mở rộng bảng bằng cách mượn không gian từ dramtbl. |
| warp_set_save_zones / warp_set_save_zone | Ghi nhận một dải PFN thuộc một zone bộ nhớ hợp lệ vào `zonetbl` (bất kể có được lưu hay không).                                               |
| warp_set_save_drams / warp_set_save_dram | Ghi nhận một dải PFN **thực sự cần lưu** vào `dramtbl` — đây chính là danh sách trang sẽ đi vào snapshot.                                     |
| warp_set_nosave_dram                     | Theo dõi vùng DRAM trống/không‑lưu liên tục lớn nhất (maxarea/maxsize) để tối ưu chỗ đặt driver.                                              |
| warp_make_save_table                     | **Hàm lõi**: duyệt toàn bộ trang vật lý trong mọi zone, phân loại từng trang rồi đổ vào 3 bảng trên.                                          |
| warp_merge_savearea                      | Chỉ khi AMP: gộp save‑table do CPU phụ tính ra vào bảng của CPU chính.                                                                        |
| warp_print_savetbl(32/64)                | Debug: in nội dung 3 bảng ra log.                                                                                                             |

## G · Giải phóng bộ nhớ & drop cachegiảm dung lượng snapshot

|Function|Mục đích|
|---|---|
|invalidate_filesystems|(kernel cũ) Đồng bộ + vô hiệu hoá cache của mọi filesystem đang mount.|
|warp_drop_bdev_cache + 3 helper bdev|Flush và loại bỏ page cache của mọi block device để các trang sạch trở thành "free" trước khi chụp snapshot.|
|warp_shrink_memory|Gọi lặp `shrink_all_memory()` để thu hồi càng nhiều trang càng tốt, giảm số trang phải lưu.|
|warp_print_meminfo|Debug: in thống kê Buffers/Cached/Active/Dirty/Slab… trước và sau khi shrink.|
## H · Điều phối snapshottrái tim của driver

| Function         | Mục đích                                                                                                                                                          |
| ---------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| hibdrv_snapshot  | **API xuất ra ngoài** (được gọi từ trong driver máy‑cụ‑thể): xây save‑table rồi gọi vào driver hibernation cấp thấp để chụp/switch snapshot thật sự.              |
| warp_save_cancel | API xuất ra ngoài: cho phép huỷ một snapshot đang tiến hành.                                                                                                      |
| hibernate        | **Entry point cao nhất**, thay thế `hibernate()` chuẩn của kernel: điều phối toàn bộ pipeline freeze → shrink → suspend device → snapshot → resume device → thaw. |
## I · Shim tương thích API hibernation của kernelglue

|Function|Mục đích|
|---|---|
|pfn_is_nosave|Xác định một PFN có thuộc vùng ảnh kernel (`__nosave_begin/end`) hay không.|
|system_entering_hibernation|Trả về true nếu hệ thống đang trong quá trình hibernate (so `pm_device_down`).|
|hibernation_available / hibernation_set_ops / is_hibernate_resume_dev|Hàm rỗng/stub để thoả API chuẩn của kernel — Warp!! tự quản lý toàn bộ luồng nên không cần các platform ops mặc định.|
# 2. Sơ đồ luồng gọi cấp cao

Mỗi khối dưới đây trình bày một function "cha" cùng các function "con" mà nó gọi trực tiếp, kèm một câu tóm tắt ý nghĩa cấp cao của tổ hợp đó — đúng theo yêu cầu "function A gồm B, C, D…". Đi từ trên xuống là đi từ entry point cao nhất xuống các hàm chi tiết nhất.

## `hibernate()`

**High level:** chuẩn bị workspace & nạp driver → đóng băng toàn hệ thống và ép RAM nhỏ nhất có thể → cho thiết bị/CPU vào trạng thái nghỉ → nhảy vào chụp snapshot (điểm không quay lại) → đánh thức phần cứng → rã đông tiến trình → dọn dẹp. Đây là bản thay thế hoàn toàn cho `hibernate()` chuẩn của Linux, với rất nhiều nhãn `goto` để unwind đúng thứ tự khi một bước giữa chừng thất bại.

![[Pasted image 20260820142847.png]]

## `hibdrv_snapshot()`

**High level:** tính toán "cần lưu những trang vật lý nào" rồi giao quyền điều khiển cho driver hibernation cấp thấp (blob đã nạp sẵn) để thực sự copy dữ liệu ra thiết bị lưu trữ.
![[Pasted image 20260820142931.png]]

```
hibdrv_snapshot()
│
├─ 1. Dọn per-CPU page cache trước khi kiểm kê RAM
│      (đảm bảo không có trang nào đang "kẹt" ở bộ đệm cục bộ của CPU,
│       nếu không bảng kiểm kê ở bước sau sẽ sai lệch)
│
├─ 2. Kiểm kê toàn bộ RAM vật lý → dựng 3 bảng "cần lưu gì"
│      └─ lỗi (vd bảng tràn) → dừng ngay, không làm gì thêm phía sau
│
├─ 3. Ghi log thống kê kết quả kiểm kê (chỉ để quan sát/debug)
│
├─ 4. Báo ra ngoài: "bắt đầu lưu" (để board có thể nháy đèn/hiện UI)
│
├─ 5. [Chỉ hệ AMP nhiều CPU] chờ CPU phụ kiểm kê xong phần của nó,
│      rồi gộp chung thành một bức tranh RAM duy nhất
│
├─ 6. Kiểm tra: lượt lưu này đã bị ai đó huỷ giữa chừng chưa?
│      │
│      ├─ Đã huỷ → bỏ hẳn phần ghi thật, coi như lỗi "đã huỷ"
│      │
│      └─ Chưa huỷ → tiến hành ghi thật:
│           │
│           ├─ 6a. Đổi 3 bảng sang "địa chỉ vật lý"
│           │       (vì phần chạy tiếp theo không còn nằm trong
│           │        ngữ cảnh kernel bình thường nữa, không dùng
│           │        địa chỉ ảo được)
│           │
│           ├─ 6b. switch_mode = 0 → CHỤP một snapshot mới, ghi ra
│           │       thiết bị đích, xong thì quay lại
│           │
│           ├─ 6c. switch_mode = 1 → không chụp gì cả, CHUYỂN THẲNG
│           │       sang một snapshot đã lưu sẵn ở nơi khác (boot
│           │       tức thời vào ảnh cũ)
│           │
│           ├─ 6d. switch_mode = 2 → CHỤP trước; nếu chụp thành công
│           │       và đang ở nhánh "vừa lưu xong" (chưa phải resume)
│           │       thì lập tức CHUYỂN tiếp sang ảnh khác luôn,
│           │       không cần đợi thêm một lượt hibernate riêng
│           │
│           └─ 6e. Dịch mã lỗi trả về (I/O, hết bộ nhớ, hết chỗ chứa,
│                   timeout, bị huỷ…) thành dòng log dễ đọc — thuần
│                   debug, không đổi kết quả cuối
│
├─ 7. [Chỉ AMP] hai CPU xác nhận với nhau là đã lưu xong phần của mình
│      trước khi cùng đi tiếp
│
├─ 8. Báo ra ngoài: "đã lưu xong"
│
├─ 9. [Chỉ bizhub] đánh dấu phiên chụp là hợp lệ (cờ nội bộ dùng
│      cho một cơ chế theo dõi riêng của board)
│
└─ 10. Trả kết quả (0 = thành công, số âm = loại lỗi tương ứng)

```
**Ý nghĩa tổng thể:** hàm này chỉ làm đúng hai việc lớn — _"biết cần lưu gì"_ (bước 1‑2) rồi _"giao việc lưu/chuyển thật cho driver cấp thấp"_ (bước 6b‑6d). Mọi thứ còn lại (log, progress, AMP sync, cờ bizhub) chỉ là phụ trợ quanh hai việc cốt lõi đó. `switch_mode` chính là cái quyết định hành vi ở bước 6: **0 = lưu**, **1 = chuyển ảnh có sẵn**, **2 = lưu rồi chuyển luôn**.

```c
int hibdrv_snapshot(void)
{
    int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,25)
    drain_local_pages(NULL);
#else
    drain_local_pages();
#endif
    if ((ret = warp_make_save_table()) < 0)
        return ret;

    printk("dram save %d pages\n", warp_save_pages);
    printk("maxarea 0x%08x(0x%08x)  lowmem_maxarea 0x%08x(0x%08x)\n",
           warp_param.maxarea, warp_param.maxsize,
           warp_param.lowmem_maxarea, warp_param.lowmem_maxsize);
    printk("zonetbl %d  exttbl %d  dramtbl %d\n", warp_param.zonetbl_num,
           warp_param.exttbl_num, warp_param.dramtbl_num);
    printk("dramtbl_max %ld\n",dramtbl_max);
	ul_dramtbl_num = warp_param.dramtbl_num;

    if (warp_ops->progress)
        warp_ops->progress(WARP_PROGRESS_SAVE);

#ifdef WARP_AMP
#ifdef WARP_AMP_MERGE
    if (warp_amp() && warp_amp_maincpu()) {
        struct warp *warp_param1 = (struct warp *)WARP_AMP_PARAM;
        if ((ret = warp_amp_wait_subcpu_saveend()) >= 0) {
            warp_invalidate_dcache_range(WARP_AMP_PARAM,
                                         WARP_AMP_PARAM + sizeof(struct warp));
            warp_invalidate_dcache_range(WARP_AMP_WORK,
                                         WARP_AMP_WORK + WARP_AMP_WORK_SIZE);
            warp_merge_savearea(warp_param1);
            warp_param.subcpu_info = warp_param1->subcpu_info;
        } else if (ret == -ECANCELED) {
            warp_canceled = 1;
        }
    }
#else
    if (warp_amp())
        warp_param.subcpu_info = WARP_AMP_SHMEM_PHYS;
#endif
#endif

    if (warp_canceled) {
        ret = -ECANCELED;
    } else {
#ifdef WARP_PRINT_SAVETBL
        warp_print_savetbl();
#endif
        warp_param.zonetbl = __pa(zonetbl);
        warp_param.dramtbl = __pa(dramtbl);
        warp_param.exttbl = __pa(exttbl);

        if (warp_param.switch_mode == 0) {
            if ((ret = WARP_DRV_SNAPSHOT(warp_hibdrv_addr,
                                         &warp_param)) == -ECANCELED)
                warp_canceled = 1;
        } else {
            int loadf = 1;

            warp_boot_param.lowmem_maxarea = warp_param.lowmem_maxarea;
            warp_boot_param.lowmem_maxsize = warp_param.lowmem_maxsize;

            if (warp_param.switch_mode == 2) {
                loadf = 0;
                if ((ret = WARP_DRV_SNAPSHOT(warp_hibdrv_addr,
                                             &warp_param)) < 0) {
                    if (ret == -ECANCELED)
                        warp_canceled = 1;
                } else if (warp_param.stat == 0) {
                    loadf = 1;
                    if (warp_drv_info[0].mode == WARP_DRV_FIXED)
                        memcpy((void *)warp_hibdrv_addr +
                               WARP_DRV_TEXT_END(warp_hibdrv_addr),
                               warp_bootflag_buf,
                               WARP_BF_LEN);
                }
            }
            if (loadf) {
                memcpy((void *)warp_hibdrv_addr +
                       WARP_DRV_TEXT_END(warp_hibdrv_addr) + WARP_BF_LEN,
                       &warp_boot_param, sizeof(warp_boot_param));
                ret = WARP_DRV_SWITCH(warp_hibdrv_addr);
            }
        }
        if (ret < 0) {
            if (ret == -EIO)
                printk("hibdrv: I/O error\n");
            else if (ret == -ENOMEM)
                printk("hibdrv: Out of memory\n");
            else if (ret == -ENODEV)
                printk("hibdrv: No such device\n");
            else if (ret == -EINVAL)
                printk("hibdrv: Invalid argument\n");
            else if (ret == -ENOSPC)
                printk("hibdrv: No space left on snapshot device\n");
            else if (ret == -ETIMEDOUT)
                printk("hibdrv: Device timed out\n");
            else if (ret == -ECANCELED)
                printk("hibdrv: Operation Canceled\n");
            else
                printk("hibdrv: error %d\n", ret);
        }
    }

#ifdef WARP_AMP
    if (ret >= 0 && warp_amp()) {
#if defined(WARP_AMP_MERGE)
        if (warp_amp_maincpu()) {
            if (warp_param.stat == 0)
                warp_amp_send_maincpu_saveend(0);
            else
                warp_amp_boot_subcpu();
        } else {
            if (warp_param.stat == 0) {
                memcpy((void *)WARP_AMP_PARAM, &warp_param,
                       sizeof(struct warp));
                warp_clean_dcache_range(WARP_AMP_PARAM,
                                        WARP_AMP_PARAM + sizeof(struct warp));
                warp_clean_dcache_range(WARP_AMP_WORK,
                                        WARP_AMP_WORK + WARP_AMP_WORK_SIZE);
                warp_amp_send_subcpu_saveend(0);
                if ((ret = warp_amp_wait_maincpu_saveend()) == -ECANCELED)
                    warp_canceled = 1;
            }
        }
#endif
    }
#endif

    if (warp_ops->progress)
        warp_ops->progress(WARP_PROGRESS_SAVEEND);
/* for issue OP_BTS-20538 start. */
#ifdef CONFIG_KM_BIZHUB
	valid_superwarp_session = true ;
#endif
/* for issue OP_BTS-20538 end. */

    return ret;
}
```

### Ý nghĩa từng dòng code

| Dòng code                                                                                                                                                                              | Nội dung                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **[L1781-1785](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1781-L1785) — `drain_local_pages`**                        | Đẩy hết các trang đang nằm trong per-CPU pagevec (bộ đệm cấp phát/giải phóng trang nhanh của CPU hiện tại) trở về free-list thật của zone. Nếu bỏ qua bước này, những trang đó có thể bị `mark_free_pages()`/`warp_make_save_table()` đếm sai (tưởng đang dùng nhưng thực ra sắp free, hoặc ngược lại).                                                                                                                                                                                                                                                                                      |
| **[L1786-1787](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1786-L1787) — gọi `warp_make_save_table()`**               | Đây là bước kiểm kê RAM đã giải thích ở artifact trước. Nếu trả về âm (ví dụ tràn bảng `dramtbl`/`zonetbl`/`exttbl`), hàm thoát ngay lập tức — không có snapshot nào được ghi.                                                                                                                                                                                                                                                                                                                                                                                                               |
| **[L1789-1796](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1789-L1796) — 5 dòng `printk`**                            | Chỉ in số liệu debug: số trang sẽ lưu (`warp_save_pages`), vùng trống lớn nhất toàn cục và vùng thấp (`maxarea/maxsize`, `lowmem_maxarea/lowmem_maxsize`), số phần tử mỗi bảng. Dòng cuối `ul_dramtbl_num = warp_param.dramtbl_num;` cập nhật biến debug để `warp_show_module_parameter()` phơi ra sysfs (`/sys/kernel/warp_kobj/dramtbl_num`) — không ảnh hưởng logic snapshot.                                                                                                                                                                                                             |
| **[L1798-1799](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1798-L1799) — `warp_ops->progress(WARP_PROGRESS_SAVE)`**   | Gọi callback báo tiến độ do board đăng ký (qua `warp_register_machine`) — thường dùng để board bật LED/hiện màn hình "đang lưu trạng thái, đừng tắt nguồn".                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| **[L1801-1820](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1801-L1820) — khối `WARP_AMP`**                            | Chỉ tồn tại trên board nhiều CPU (asymmetric multiprocessing). Nếu CPU hiện tại là CPU chính và có bật `WARP_AMP_MERGE`: chờ CPU phụ báo đã kiểm kê xong (`warp_amp_wait_subcpu_saveend`), vô hiệu hoá dcache của vùng tham số/工作 chung để đọc dữ liệu mới nhất từ CPU phụ, rồi gộp bảng của CPU phụ vào bảng của mình (`warp_merge_savearea`). Nếu chờ bị huỷ (`-ECANCELED`), set cờ `warp_canceled`. Nhánh không‑merge chỉ đơn giản ghi lại địa chỉ vùng nhớ chia sẻ để CPU phụ tự đọc riêng.                                                                                              |
| **[L1822-1824](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1822-L1824) — kiểm tra `warp_canceled`**                   | Nếu đã bị huỷ (do `warp_save_cancel()` hoặc `/proc/warp/canceled` được set từ trước, hoặc do bước AMP ở trên phát hiện huỷ), bỏ qua toàn bộ phần ghi thật, gán thẳng `ret = -ECANCELED`.                                                                                                                                                                                                                                                                                                                                                                                                     |
| **[L1828-1830](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1828-L1830) — quy đổi bảng sang địa chỉ vật lý**           | `__pa(zonetbl)`, `__pa(dramtbl)`, `__pa(exttbl)` — driver cấp thấp sắp được nhảy vào chạy ở một môi trường không còn đầy đủ dịch vụ kernel (có thể MMU tắt hoặc mapping khác), nên nó chỉ có thể nhận địa chỉ vật lý, không dùng được con trỏ ảo của kernel.                                                                                                                                                                                                                                                                                                                                 |
| **[L1832-1835](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1832-L1835) — nhánh `switch_mode == 0`**                   | Gọi thẳng `WARP_DRV_SNAPSHOT(warp_hibdrv_addr, &warp_param)` — đây chính là **lệnh nhảy vào driver blob đã nạp sẵn trong RAM** (macro board-specific, thực chất là gọi hàm assembly/C độc lập). Nếu driver trả `-ECANCELED`, đánh dấu `warp_canceled = 1` để phần code sau (unwind trong `hibernate()`) biết mà xử lý đúng.                                                                                                                                                                                                                                                                  |
| **[L1837-1840](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1837-L1840) — chuẩn bị `warp_boot_param`**                 | `loadf = 1` mặc định là "sẽ chuyển ảnh ở cuối". Copy `lowmem_maxarea/lowmem_maxsize` (vùng trống thấp lớn nhất, tính được ở bước kiểm kê) sang `warp_boot_param` — vì đây là **struct riêng** dùng cho thao tác switch/boot, tách biệt với `warp_param` dùng cho thao tác snapshot, nên phải chuyển tay giá trị cần thiết.                                                                                                                                                                                                                                                                   |
| **[L1842-1856](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1842-L1856) — nhánh `switch_mode == 2`**                   | `loadf = 0` — tạm thời KHÔNG chuyển ảnh, ưu tiên chụp trước. Gọi `WARP_DRV_SNAPSHOT` giống mode 0. Nếu lỗi: set `warp_canceled` nếu là do huỷ, và **không bật lại `loadf`** — nghĩa là chụp thất bại thì thôi, không chuyển ảnh nữa. Nếu thành công và `warp_param.stat == 0` (đang ở nhánh vừa‑lưu‑xong, không phải vừa resume) → bật `loadf = 1` để đi tiếp bước chuyển, và nếu driver ở chế độ `FIXED`, copy `warp_bootflag_buf` (bootflag đã nạp sẵn từ `warp_work_init`) vào ngay sau vùng code của driver — dựng sẵn "gói tin" driver+bootflag liền nhau để bước switch bên dưới dùng. |
| **[L1857-1862](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1857-L1862) — thực hiện switch khi `loadf`**               | Copy cả struct `warp_boot_param` vào đúng vị trí ngay sau driver‑text + bootflag (một offset cố định mà driver cấp thấp biết trước để đọc), rồi gọi `WARP_DRV_SWITCH(warp_hibdrv_addr)` — **lệnh nhảy thực sự thực hiện việc chuyển/khởi động vào ảnh đã lưu ở slot khác**, không qua bootloader.                                                                                                                                                                                                                                                                                            |
| **[L1864-1881](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1864-L1881) — dịch mã lỗi ra log**                         | Chuỗi `if/else if` thuần diễn giải: `ret` âm ứng với loại lỗi nào (`-EIO`, `-ENOMEM`, `-ENODEV`, `-EINVAL`, `-ENOSPC`, `-ETIMEDOUT`, `-ECANCELED`, hoặc lỗi lạ khác) thì in đúng câu log tương ứng. Không thay đổi `ret`, chỉ phục vụ đọc log.                                                                                                                                                                                                                                                                                                                                               |
| **[L1884-1907](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1884-L1907) — khối `WARP_AMP` thứ hai (đồng bộ kết thúc)** | Chỉ chạy nếu `ret >= 0` (thao tác trên vừa thành công) và board là AMP. CPU chính: nếu `stat == 0` (vừa lưu xong, chưa resume) thì báo cho CPU phụ là đã lưu xong (`warp_amp_send_maincpu_saveend`); ngược lại (đang trên nhánh resume) thì khởi động luôn CPU phụ (`warp_amp_boot_subcpu`). CPU phụ: nếu `stat == 0`, copy `warp_param` của mình vào vùng nhớ chia sẻ, clean dcache để CPU chính đọc được dữ liệu mới nhất, báo hoàn tất, rồi **chờ** CPU chính xác nhận (`warp_amp_wait_maincpu_saveend`) — nếu việc chờ đó bị huỷ, set `warp_canceled`.                                   |
| **[L1909-1910](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1909-L1910) — `progress(WARP_PROGRESS_SAVEEND)`**          | Báo cho board biết giai đoạn lưu đã kết thúc (dù thành công hay lỗi) — đối lập với `WARP_PROGRESS_SAVE` ở đầu hàm.                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| **[L1911-1915](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1911-L1915) — `valid_superwarp_session = true`**           | Riêng cho `CONFIG_KM_BIZHUB`, gắn với issue OP_BTS‑20538 — một cờ toàn cục đánh dấu "vừa có một phiên chụp Warp!! hợp lệ diễn ra", được nơi khác trong hệ thống bizhub dùng để nhận biết trạng thái (không thấy logic đọc lại cờ này trong chính file warp.c, nên đây là điểm giao tiếp với code ngoài file).                                                                                                                                                                                                                                                                                |
| **[L1917](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1917) — `return ret`**                                          | Trả kết quả cuối cùng lên cho nơi gọi (`warp_ops->snapshot()` → cuối cùng là `hibernate()`), 0 = thành công, âm = lỗi (đã được log rõ ở bước trên).                                                                                                                                                                                                                                                                                                                                                                                                                                          |


## `warp_make_save_table()`

**High level:** duyệt từng trang vật lý trong toàn hệ thống và phân loại nó vào một trong ba nhóm: vùng scratch của chính warp (bỏ qua), vùng free/forbidden (không lưu, nhưng theo dõi khoảng trống lớn nhất), hoặc dữ liệu thật cần lưu (đưa vào dramtbl).

![[Pasted image 20260820143003.png]]

```
warp_make_save_table()
│
└─ Với TỪNG zone bộ nhớ trong toàn hệ thống:
     │
     ├─ 1. Đánh dấu chính xác trang nào trong zone này đang "free"
     │      tại đúng thời điểm này (chụp một lát cắt tin cậy, vì
     │      free-list của bộ cấp phát trang luôn biến động)
     │
     └─ 2. Với TỪNG trang vật lý (pfn) trong zone:
            │
            ├─ 2a. Trang không tồn tại thật, hoặc thuộc vùng
            │       "cấm động vào" của kernel (ảnh kernel, vùng
            │       dự trữ...) → bỏ qua hoàn toàn, không ghi vào
            │       đâu cả
            │
            ├─ 2b. Trang thuộc vùng scratch của chính Warp!!
            │       (buffer driver / buffer bảng vừa cấp phát)
            │       → bỏ qua, không ghi vào đâu cả
            │
            ├─ 2c. Còn lại (trang hợp lệ, không bị cấm, không
            │       phải scratch của Warp) → ghi nhận "đây là
            │       một trang hợp lệ của hệ thống"
            │       │
            │       ├─ Trang đang chứa dữ liệu thật (không phải
            │       │    trang free) → đánh dấu "phải đưa vào
            │       │    snapshot", tăng bộ đếm số trang sẽ lưu
            │       │
            │       └─ Trang đang free → không lưu, nhưng cập
            │            nhật "khoảng trống free liên tục lớn
            │            nhất" tìm được tới giờ
            │
└─ 3. Sau khi quét xong toàn bộ hệ thống: nếu board không phân
       biệt "vùng nhớ thấp" (board 64-bit không cần khái niệm
       này) → dùng luôn kết quả "khoảng trống lớn nhất toàn cục"
       làm luôn kết quả cho "khoảng trống lớn nhất vùng thấp"
       (tránh để trống/không xác định)

```

**Ý nghĩa tổng thể:** đây là hàm **kiểm kê dân số RAM** — duyệt qua từng trang vật lý một, không sót trang nào, và phân đúng ba nhóm: _"đừng động vào"_ (kernel tự quản hoặc scratch của Warp), _"phải lưu"_ (dữ liệu thật) và _"không cần lưu nhưng cần biết vị trí"_ (trang free, dùng để tìm chỗ trống lớn nhất cho driver cấp thấp đặt tạm khi nó chạy). Kết quả là 3 bảng (`zonetbl`, `dramtbl`, và gián tiếp cả `maxarea/maxsize`) mà `hibdrv_snapshot()` sẽ dùng ngay sau đó.

### Ý nghĩa từng dòng code

**[L1337-1342](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1337-L1342) — khai báo biến** `zone` là con trỏ duyệt qua mọi zone bộ nhớ; `pfn`/`end` là chỉ số trang vật lý (Page Frame Number) đang xét và mốc kết thúc của zone. `pfn2` chỉ tồn tại trên kernel rất cũ (< 2.6.11), phục vụ một dạng API cũ của `swsusp_page_is_saveable` (xem giải thích ở L1363 bên dưới).

**[L1344](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1344) — `for_each_zone(zone)`** Macro lõi của kernel, lặp qua **mọi zone bộ nhớ** trên toàn hệ thống (ví dụ ZONE_DMA, ZONE_NORMAL, ZONE_HIGHMEM…, trên mọi node nếu là NUMA) — đảm bảo không sót vùng RAM nào.

**[L1345-1347](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1345-L1347) — `mark_free_pages(zone)`** Hàm lõi có sẵn của cơ chế hibernation chuẩn Linux (không định nghĩa trong file này): đánh dấu chính xác trang nào trong zone hiện đang nằm trên free-list tại **đúng thời điểm gọi**. Vì free-list của bộ cấp phát buddy allocator biến động liên tục, hàm này tạo ra một "ảnh chụp" đáng tin cậy để các hàm `swsusp_page_is_saveable`/`is_forbidden` dùng ngay sau — nếu không có bước này, việc phân loại free/không-free có thể bị race và sai lệch.

**[L1348-1349](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1348-L1349) — phạm vi duyệt** `end` = PFN bắt đầu của zone cộng tổng số trang mà zone "trải dài" (`spanned_pages` — có thể bao gồm cả các lỗ hổng không có trang thật). Vòng `for` sau đó duyệt **từng PFN một**, không nhảy cóc.

**[L1350-1355](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1350-L1355) — lọc trang không hợp lệ / bị cấm** `!warp_pfn_valid(pfn)`: PFN này rơi vào lỗ hổng trong zone (không có trang vật lý thật ở đó) → loại ngay. `swsusp_page_is_forbidden(pfn_to_page(pfn))` (kernel ≥ 2.6.22): trang này nằm trong **tập "cấm"** chuẩn của Linux hibernation — chủ yếu là ảnh nhị phân của chính kernel (`__nosave_begin/__nosave_end`, xem lại `pfn_is_nosave()` đã giải thích ở artifact đầu tiên) và các vùng dự trữ khác do kernel tự đăng ký. Một trong hai điều kiện đúng → `continue`, trang này **không được ghi vào bất kỳ bảng nào**, kể cả `zonetbl` — hoàn toàn vô hình với toàn bộ pipeline snapshot.

**[L1356-1362](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1356-L1362) — lọc trang thuộc scratch của Warp!!** Duyệt tuyến tính qua `warp_nosave_work[]` (danh sách vùng đã đăng ký ở `warp_work_alloc()` — chính là `hd_savearea`... à không, chính xác là `warp_drv_buf` và `warp_work`, xem giải thích trước) để kiểm tra `pfn` có rơi vào khoảng `[start, end)` của bất kỳ vùng nào không. Nếu có (`i < warp_nosave_work_num` sau vòng lặp) → `continue`, cũng bỏ qua hoàn toàn — vì đây là bộ nhớ tạm của chính driver Warp, không phải dữ liệu hệ thống cần snapshot.

**[L1363-1378](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1363-L1378) — nhánh kernel rất cũ (< 2.6.11)** API `swsusp_page_is_saveable` phiên bản này nhận `&pfn2` (con trỏ) và có thể **tự động đẩy `pfn2` tiến lên**, gom một dải nhiều trang liên tiếp cùng loại (đều saveable hoặc đều không) vào một lần gọi thay vì hỏi từng trang — một kiểu tối ưu hiệu năng của API cũ. Nếu dải đó saveable: xử lý y hệt nhánh hiện đại (tăng `warp_save_pages`, ghi vào cả `zonetbl` lẫn `dramtbl`) nhưng chỉ cho **một** trang `pfn`. Nếu không saveable: lùi `pfn` lại 1 rồi lặp `while` tiến `pfn` lên tới `pfn2`, ghi từng trang trong cả dải vào `zonetbl` và cập nhật `warp_set_nosave_dram` cho từng trang — xử lý bù cho cả dải mà lần gọi API đã gộp. Đây là nhánh tương thích ngược, hầu như không còn ý nghĩa với các kernel hiện đại mà file này target.

**[L1379-1389](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1379-L1389) — nhánh hiện đại (≥ 2.6.11), đường đi thực tế**

- `warp_set_save_zone(pfn)`: trang đã qua được hết các bộ lọc ở trên (hợp lệ, không cấm, không phải scratch Warp) → ghi vào `zonetbl` — **bảng ghi nhận mọi trang "đang chơi"**, bất kể có được lưu hay không.
- `swsusp_page_is_saveable(zone, pfn)`: hàm lõi chuẩn của Linux, trả lời "trang này đang chứa dữ liệu sống, không nằm trên free-list (theo đúng ảnh chụp từ `mark_free_pages` ở trên) hay không".
    - **Có** (đang có dữ liệu): `warp_save_pages++` (đếm dồn), `warp_set_save_dram(pfn)` — ghi vào `dramtbl`, **đây chính là danh sách trang sẽ được copy ra thiết bị lưu trữ**. Lỗi từ việc ghi bảng (tràn bảng) khiến hàm trả lỗi ngay, dừng toàn bộ quá trình kiểm kê.
    - **Không** (trang đang free): `warp_set_nosave_dram(zone, pfn)` — không ghi vào `dramtbl`, chỉ cập nhật bộ theo dõi "khoảng trống liên tục lớn nhất" (đã giải thích ở artifact đầu: `maxarea/maxsize` và bản riêng cho vùng thấp) — thông tin này sau đó giúp driver cấp thấp biết chỗ an toàn để tự đặt vùng làm việc của nó khi chạy, tránh đè lên dữ liệu sắp được lưu.

**[L1393-1398](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1393-L1398) — backfill kết quả cho board 64-bit không phân biệt vùng thấp** Sau khi quét hết mọi zone: nếu `warp_hibdrv_lowmem_end == 0` (board không đặt ranh giới "vùng nhớ thấp" — nhớ lại đây là giá trị tính ở `warp_work_init()`, và `warp_set_nosave_dram()` chỉ cập nhật `lowmem_maxarea/size` **khi** giá trị này khác 0), thì toàn bộ quá trình quét vừa rồi **chưa bao giờ cập nhật** cặp `lowmem_maxarea/lowmem_maxsize` (chúng vẫn giữ giá trị sentinel "chưa tìm thấy gì" từ lúc reset ở `warp_work_init`). Khối này backfill chúng bằng đúng giá trị "khoảng trống lớn nhất toàn cục" (`maxarea/maxsize`) — đảm bảo code phía sau (vốn luôn kỳ vọng `lowmem_maxarea/size` có một câu trả lời hợp lý) không nhận về dữ liệu rỗng/vô nghĩa.

**[L1400](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1400) — `return 0`** Thành công — cả 3 bảng (`zonetbl`, `dramtbl`, và gián tiếp `exttbl` nếu có gọi `warp_set_savearea` từ nơi khác) cùng số liệu thống kê (`warp_save_pages`, `maxarea/maxsize`, `lowmem_maxarea/lowmem_maxsize`) đã sẵn sàng để `hibdrv_snapshot()` đọc và giao cho driver cấp thấp.

## `warp_work_init()`

**High level:** chuẩn bị mọi thứ cần có _trước khi_ có thể tính save‑table: map vùng driver cố định nếu cần, cấp phát buffer scratch, nạp driver + bootflag vào RAM, rồi cắt buffer làm việc thành ba bảng (dramtbl/exttbl/zonetbl) mà `warp_make_save_table()` sẽ dùng.
                 ![[Pasted image 20260820143030.png]]

```
warp_work_init()
│
├─ 1. Nếu driver ở dạng "địa chỉ vật lý cố định" mà chưa từng
│      được map thành địa chỉ có thể thực thi → map nó một lần
│      (chỉ làm nếu board cấu hình kiểu FIXED và đây là lần đầu)
│
├─ 2. Chuẩn bị vùng RAM làm việc (workspace)
│      │
│      ├─ Nếu workspace đã được cấp phát sẵn từ lúc nạp module
│      │    → chỉ kiểm tra driver có vừa vùng đã cấp phát không,
│      │      không vừa thì báo lỗi hết bộ nhớ
│      │
│      └─ Nếu chưa (trường hợp bình thường)
│           → cấp phát mới workspace ngay tại đây
│
├─ 3. Nạp mã nhị phân driver (+ UserAPI) vào workspace vừa có
│      → lỗi thì dừng, trả lỗi lên trên ngay
│
├─ 4. Ghi nhận ranh giới "vùng nhớ thấp" mà driver chiếm dụng
│      (mốc này dùng về sau để biết đâu là "vùng thấp" khi tìm
│       khoảng trống lớn nhất)
│
├─ 5. Nếu đang ở chế độ có "chuyển ảnh" (switch/switch+snapshot):
│      │
│      ├─ 5a. Dành riêng một chỗ để chứa bootflag — hoặc cắt từ
│      │      đầu vùng workspace, hoặc đặt ngay sau phần code
│      │      của driver, tuỳ theo cách driver được nạp
│      │
│      └─ 5b. Nạp nội dung bootflag thật vào chỗ vừa dành riêng
│             (trên hệ nhiều CPU, còn nạp trước bootflag của CPU
│              phụ để chuyển cho nó)
│             → lỗi thì dừng, trả lỗi lên trên
│
├─ 6. Cắt phần workspace còn lại thành 3 "sổ ghi chép" rỗng:
│      bảng zone, bảng vùng mở rộng, bảng DRAM cần lưu — dành
│      phần lớn nhất cho bảng DRAM vì đây là bảng đông nhất
│
├─ 7. Xoá sạch mọi bộ đếm & mốc "khoảng trống lớn nhất" của lượt
│      hibernate trước — coi như bắt đầu một lượt kiểm kê hoàn
│      toàn mới, tinh khôi
│
├─ 8. Sinh một mã định danh ngẫu nhiên duy nhất cho lượt snapshot
│      này (để lúc resume biết chắc đây đúng là ảnh mình mong đợi)
│
├─ 9. Dựng lại giao diện debug qua sysfs
│      │
│      ├─ Có cái cũ từ lượt trước thì gỡ bỏ trước
│      └─ Tạo cái mới, đăng ký các file debug (không coi là lỗi
│         nghiêm trọng nếu bước này thất bại — vẫn tiếp tục)
│
└─ 10. Trả về thành công

```
**Ý nghĩa tổng thể:** đây là hàm "dọn bàn" trước khi kiểm kê RAM — nó biến workspace từ chỗ trống thành ba thứ sẵn sàng dùng: **driver cấp thấp đã nạp và thực thi được**, **bootflag đã có (nếu cần switch)**, và **ba bảng save-table rỗng, đã định vị đúng chỗ**. Nói cách khác: mọi thứ mà `warp_make_save_table()` và `hibdrv_snapshot()` sẽ dùng ngay sau đó đều được chuẩn bị xong trong hàm này.

### Ý nghĩa từng dòng code


| Dòng code                                                                                                                                                                              | Ý nghĩa                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **[L1073-1081](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1073-L1081) — map lười (lazy map) driver cố định**         | Chỉ chạy khi driver #0 khai báo ở dạng `WARP_DRV_FIXED` (có địa chỉ vật lý + kích thước cố định do board cấu hình sẵn) **nhưng** `virt == 0` (chưa từng được ánh xạ sang địa chỉ ảo). Gọi `warp_remap_exec()` để tạo mapping thực thi được (giống `ioremap` nhưng cho phép chạy code), lưu kết quả ngược lại vào `warp_drv_info[0].drv.fixed.virt`, và bật cờ `fixed_drv_iomap = 1` để `warp_work_exit()` sau này biết mà gỡ mapping đúng lúc. Vì có điều kiện `virt == 0`, việc map này **chỉ xảy ra một lần** dù `warp_work_init()` có thể chạy lại nhiều lần qua các lượt hibernate khác nhau.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| **[L1083-1092](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1083-L1092) — hai cách chuẩn bị workspace**                | Tuỳ cấu hình biên dịch `WARP_WORK_ALLOC_INIT`:<br><br>- **Có định nghĩa** (workspace đã cấp phát tĩnh một lần từ `warp_init()` lúc nạp module): ở đây chỉ _kiểm tra_ xem driver có vừa vùng cố định `WARP_DRV_LOAD_AREA_SIZE` hay không; không vừa thì log lỗi, set `uc_ngPoint = 4` (điểm lỗi debug phơi qua sysfs), trả `-ENOMEM`.<br>- **Không định nghĩa** (mặc định, cấp phát động): gọi thẳng `warp_work_alloc()` ngay tại đây — nghĩa là buffer `hd_savearea`/`warp_drv_buf`/`warp_work` được **cấp phát mới mỗi lần hibernate**, không giữ qua các lần.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| **[L1094-1095](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1094-L1095) — nạp driver**                                 | Gọi `warp_load_drv()` (đã giải thích ở artifact trước) để đưa mã nhị phân driver + UserAPI vào đúng vị trí trong workspace vừa có. Lỗi thì thoát ngay, không đi tiếp các bước dựng bảng phía dưới.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| **[L1097-1098](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1097-L1098) — ghi nhận ranh giới "vùng nhớ thấp"**         | `WARP_DRV_LOWMEM_END(warp_hibdrv_addr)` là macro board-specific đọc từ header của driver vừa nạp để biết driver chiếm bộ nhớ thấp tới đâu; chuyển sang đơn vị PFN (`>> PAGE_SHIFT`) rồi lưu vào `warp_hibdrv_lowmem_end` — biến toàn cục này sau đó được `warp_set_nosave_dram()` và `warp_make_save_table()` dùng để phân biệt "vùng thấp" khi tìm khoảng trống lớn nhất (quan trọng trên board 32-bit, nơi vùng nhớ thấp là tài nguyên khan hiếm).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| **[L1100-1101](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1100-L1101) — điểm khởi đầu của workspace bảng**           | `savetbl` trỏ vào đầu `warp_work` (buffer 512 KB mặc định), `table_size` = toàn bộ kích thước buffer đó. Đây là "vùng đất trống" trước khi bị bootflag (nếu có) cắt bớt ở bước sau.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| **[L1103-1123](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1103-L1123) — dành chỗ & nạp bootflag khi có switch_mode** | Chỉ chạy khi `warp_param.switch_mode != 0` (đang ở chế độ switch hoặc snapshot+switch — xem lại giải thích `switch_mode` ở câu trả lời trước).<br><br>- Nếu driver là `FIXED` **và** `switch_mode == 2`: bootflag được đặt ngay **đầu** `savetbl` (`warp_bootflag_buf = savetbl`), rồi dịch `savetbl` tới sau `WARP_BF_LEN` byte và trừ bớt `table_size` tương ứng — để phần workspace dành cho 3 bảng save-table không đè lên vùng bootflag này.<br>- Ngược lại (driver `FLOATING`, hoặc `switch_mode == 1`): đặt `warp_bootflag_buf` ngay **sau phần code** của driver đã nạp (`warp_hibdrv_addr + WARP_DRV_TEXT_END(...)`) — không đụng vào workspace bảng chút nào, vì vị trí này khớp với chỗ mà `hibdrv_snapshot()` sau đó sẽ ghi `warp_boot_param` tiếp liền sau.<br>- Khối `WARP_AMP`: nếu là CPU chính và board có hàm `bf_copy`, nạp trước bootflag của slot **kế tiếp** (`warp_loadno + 1`) rồi giao cho board tự copy đi đâu đó (thường là sang bộ nhớ chia sẻ cho CPU phụ dùng).<br>- Dòng cuối luôn chạy (AMP hay không): nạp bootflag của **slot hiện tại** (`warp_loadno`) vào `warp_bootflag_buf` — đây mới là bootflag thực sự được dùng cho lượt này. Lỗi ở bất kỳ bước nạp nào cũng trả lỗi ngay.<br>- _Lưu ý nhỏ:_ dòng `int ret;` ở [L1104](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1104) khai báo một biến `ret` **cục bộ trong khối `if` này**, che khuất (shadow) biến `ret` của hàm ngoài — không gây lỗi vì được dùng và `return` ngay trong cùng khối, nhưng là kiểu đặt tên dễ gây nhầm khi đọc code. |
| **[L1125-1128](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1125-L1128) — tính kích thước 3 bảng**                     | `zonetbl_max`/`exttbl_max` lấy đúng giá trị mặc định cố định (8 phần tử mỗi bảng). `dramtbl_max` = phần **còn lại** của `table_size` sau khi trừ đi dung lượng cố định của hai bảng kia, chia cho kích thước một phần tử — tức bảng DRAM luôn được ưu tiên phần lớn nhất của workspace, vì nó chứa danh sách trang cần lưu (thường đông nhất). Nếu bước 5 vừa cắt bớt `table_size` cho bootflag, `dramtbl_max` cũng tự động nhỏ lại tương ứng.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| **[L1129-1131](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1129-L1131) — log & reset điểm debug**                     | In `dramtbl_max` ra log để kiểm tra dung lượng bảng thực tế mỗi lần chạy. `ul_dramtbl_num = 0` và `uc_ngPoint = 0` reset hai biến debug phơi qua sysfs (`warp_kobj`) về trạng thái "chưa có gì / chưa lỗi" — xoá dấu vết của lượt hibernate trước.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| **[L1133-1135](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1133-L1135) — định vị 3 bảng liên tiếp nhau**              | `dramtbl = savetbl` (đặt ở đầu vùng còn lại), `exttbl` đặt ngay sau khi trừ hết chỗ tối đa của `dramtbl`, `zonetbl` đặt ngay sau khi trừ hết chỗ tối đa của `exttbl` — ba bảng nằm sát nhau thành một dải liên tục trong `warp_work`, không có khoảng hở.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| **[L1136-1147](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1136-L1147) — reset toàn bộ trạng thái kiểm kê**           | Đưa số phần tử của cả 3 bảng về 0 (bảng logic coi như rỗng dù vùng nhớ vật lý chưa bị xoá — `warp_make_save_table()` sẽ ghi đè dần). Đặt lại mốc theo dõi "khoảng trống lớn nhất" (`warp_nosave_area/size`, `warp_lowmem_nosave_area/size`) về giá trị sentinel "chưa tìm thấy gì" (`-1` ép sang `unsigned long` = giá trị lớn nhất, size = 0), đồng bộ cả bản sao trong `warp_param`. `warp_save_pages = 0` — đếm lại từ đầu số trang sẽ lưu. Toàn bộ khối này đảm bảo mỗi lượt `hibernate()` đều kiểm kê **hoàn toàn độc lập**, không bị dính số liệu của lượt trước.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| **[L1149-1159](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1149-L1159) — sinh `snapshot_id`**                         | Tạo một giá trị ngẫu nhiên/định danh duy nhất cho lượt snapshot này: `get_random_u32()` (kernel mới), `get_random_int()` (kernel cũ hơn), hoặc rơi về lấy giây hiện tại qua `do_gettimeofday()` trên kernel rất cũ (trước khi có API random tốt). Giá trị này được ghi vào `warp_param.snapshot_id`, nhiều khả năng dùng để driver cấp thấp gắn vào header khi ghi ra thiết bị — giúp lúc resume xác định "đúng ảnh mình cần" chứ không lẫn với ảnh cũ còn sót trên thiết bị.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| **[L1161-1183](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1161-L1183) — dựng lại kobject debug**                     | Nếu `warp_kobject` đã tồn tại (từ lượt `hibernate()` trước đó — hàm này chạy lại mỗi lần hibernate), gỡ nó đi trước (`kobject_del`) để tránh rò rỉ/trùng entry sysfs. Tạo `warp_kobject` mới qua `make_kobject()`; nếu thất bại chỉ gán `NULL` chứ **không** coi là lỗi của cả hàm (không `return`) — driver vẫn hoạt động bình thường, chỉ là thiếu giao diện debug qua sysfs. Nếu tạo thành công, lặp qua mảng `warp_kobj_attributes[]` (3 thuộc tính: `dramtbl_max`, `dramtbl_num`, `ngPoint`) và đăng ký từng file sysfs tương ứng bằng `sysfs_create_file`; lỗi từng file riêng lẻ chỉ log cảnh báo, không chặn các file còn lại hay dừng hàm.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| **[L1185](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1185) — `return 0`**                                            | Trả thành công — tất cả các đường lỗi có thể có trong hàm này đều đã `return` sớm ở các bước 2/3/5 phía trên; đi tới được dòng này nghĩa là workspace, driver, bootflag (nếu cần) và 3 bảng save-table đều đã sẵn sàng cho `hibdrv_snapshot()` dùng.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |


### `warp_work_alloc()`

**High level:** cấp phát ba vùng RAM scratch và đánh dấu tất cả là "không lưu vào snapshot" — vì chúng chỉ là không gian làm việc tạm thời, sẽ được tái tạo lại từ đầu ở lần hibernate kế tiếp.

![[Pasted image 20260820143115.png]]

```
warp_work_alloc()
│
├─ 1. Xoá danh sách "vùng không lưu" của lượt trước
│      (hàm này có thể chạy lại nhiều lần qua các lượt hibernate,
│       nên phải bắt đầu từ danh sách rỗng)
│
├─ 2. Kiểm tra số lượng driver có vượt quá giới hạn thiết kế không
│      → vượt thì từ chối ngay, không cấp phát gì cả
│
├─ 3. Cấp phát vùng "header scratch" nhỏ (hd_savearea)
│      → thất bại thì dừng, đánh dấu đúng bước lỗi để debug
│
├─ 4. Xác định driver hibernation cần bao nhiêu chỗ
│      (lấy số cố định nếu cấp phát tĩnh lúc boot, hoặc tính
│       chính xác bằng cách đọc header driver nếu cấp phát lúc
│       chạy thật)
│
├─ 5. Nếu driver cần được NẠP VÀO RAM (không phải đã có sẵn ở
│      địa chỉ cố định):
│      │
│      ├─ Hệ nhiều CPU chia sẻ vùng nhớ chung → lấy chỗ từ vùng
│      │   dùng chung đó, không cấp phát riêng
│      │
│      └─ Trường hợp thường → cấp phát riêng một buffer mới,
│           rồi đăng ký vùng này là "không lưu vào snapshot"
│           → thất bại thì dừng, đánh dấu bước lỗi
│
├─ 6. Cấp phát vùng làm việc lớn cho 3 bảng save-table
│      │
│      ├─ Nếu là CPU phụ trên hệ chia sẻ vùng nhớ chung → lấy
│      │   phần còn lại của vùng chung đó (không cấp phát riêng)
│      │
│      └─ Còn lại (CPU chính, hoặc hệ một CPU) → luôn cấp phát
│           riêng một buffer 512 KB cho chính mình, rồi đăng ký
│           là "không lưu vào snapshot"
│           → thất bại thì dừng, đánh dấu bước lỗi
│
└─ 7. Trả về thành công

```
**Ý nghĩa tổng thể:** hàm này chỉ lo **xin cấp bộ nhớ** cho 3 buffer scratch mà toàn bộ pipeline hibernate sẽ dùng, và đánh dấu chúng "đừng đưa vào snapshot". Điểm đáng chú ý nhất: trên board nhiều CPU dùng chung vùng nhớ đặc biệt (AMP + merge), một số buffer không cấp phát bằng `kmalloc` như bình thường mà chỉ là "cắt một miếng" từ một vùng nhớ chung đã được board dành sẵn — CPU chính vẫn luôn tự `kmalloc` bảng save-table của riêng mình, chỉ CPU phụ mới dùng chung vùng đó.

### Ý nghĩa từng dòng code

**[L960](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L960) — `int drvsize;`** Sẽ giữ tổng kích thước (byte) của driver hibernation + các UserAPI driver cần nạp — tính ở bước sau.

**[L961-964](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L961-L964) — biến chỉ tồn tại trên hệ AMP+MERGE** `work` là con trỏ "đi dần" qua một vùng nhớ chung đã được board dành sẵn cho nhiều CPU (`WARP_AMP_WORK`), `size` là dung lượng còn lại của vùng đó. Hai biến này thay thế cho việc gọi `kmalloc` ở các bước sau, chỉ khi board thuộc loại nhiều CPU chia sẻ.

**[L966](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L966) — reset danh sách "không lưu"** `warp_nosave_work_num = 0` xoá sạch danh sách vùng-không-lưu đã đăng ký ở lượt hibernate trước — vì hàm này (giống `warp_work_init`) có thể chạy lại từ đầu mỗi lần `hibernate()` được gọi, không được để sót vùng của lượt cũ.

**[L968-971](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L968-L971) — kiểm tra giới hạn số driver** Nếu tổng số driver (`WARP_DRV_NUM`, gồm driver chính + UserAPI) vượt quá `WARP_USERAPI_MAX + 1`, các mảng nội bộ có kích thước cố định (như `drv_phys[]`, `userapi_cpu[]` trong `warp_param`) sẽ không đủ chỗ chứa — nên hàm chặn ngay từ đầu bằng `-EINVAL` thay vì để tràn bộ nhớ ở đâu đó về sau.

**[L973-978](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L973-L978) — cấp phát `hd_savearea`** `kmalloc` 4 KB (`WARP_HD_SAVEAREA_SIZE`). Thất bại thì log, gán `uc_ngPoint = 1` — đây là **con số định danh bước lỗi**, phơi ra qua `/sys/kernel/warp_kobj/ngPoint`, giúp người debug trên thiết bị thật (không có console) biết chính xác cấp phát nào trong chuỗi thất bại chỉ bằng cách đọc một file sysfs.

**[L979-982](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L979-L982) — ghi lại địa chỉ vùng vừa cấp** Lưu địa chỉ bắt đầu/kết thúc của `hd_savearea` vào `warp_param.hd_savearea`/`hd_savearea_end` để driver cấp thấp biết vùng scratch này ở đâu khi nó chạy độc lập sau này. In log địa chỉ để đối chiếu khi debug.

_Đáng chú ý:_ khác với `warp_drv_buf` và `warp_work` ở các bước dưới, **`hd_savearea` không được đăng ký vào `warp_nosave_work[]`** ở đây. Tức là — không như hai buffer kia (thuần scratch, nội dung vô nghĩa sau khi dùng xong nên bị loại khỏi snapshot) — vùng `hd_savearea` vẫn nằm trong diện **có thể bị `warp_make_save_table()` đưa vào snapshot** như một trang RAM bình thường. Điều này gợi ý nội dung của nó (dữ liệu header đặc thù board, được driver cấp thấp ghi vào) là thứ **cần được lưu/khôi phục cùng snapshot** chứ không phải chỉ là chỗ làm việc tạm — dù file warp.c không định nghĩa rõ nội dung thật của nó dùng để làm gì.

**[L984-987](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L984-L987) — khởi tạo con trỏ vùng chung (chỉ AMP+MERGE)** `work` trỏ vào đầu vùng nhớ chung `WARP_AMP_WORK`, `size` = toàn bộ dung lượng của nó (`WARP_AMP_WORK_SIZE`) — đây là vùng vật lý cố định, đã được board dành riêng ngoài phạm vi bộ nhớ cấp phát động bình thường, dùng để hai CPU trao đổi dữ liệu snapshot với nhau.

**[L989-994](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L989-L994) — xác định kích thước driver cần nạp** Nếu `WARP_WORK_ALLOC_INIT` (cấp phát tĩnh sớm từ lúc nạp module — thời điểm đó có thể chưa đọc chắc chắn được header driver): dùng luôn con số cấu hình tối đa `WARP_DRV_LOAD_AREA_SIZE` (dự phòng theo kích thước xấu nhất). Ngược lại (đường đi bình thường, gọi lúc `hibernate()` thật sự chạy): gọi `warp_get_drv_size()` để đọc header thật và tính kích thước chính xác; lỗi thì trả lỗi ngay.

**[L995-1019](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L995-L1019) — cấp phát buffer chứa driver** Chỉ cấp phát khi thực sự cần nạp (`drvsize > 0`) và driver #0 ở chế độ `FLOATING` (cần được nạp vào RAM, khác với `FIXED` là đã nằm sẵn ở địa chỉ vật lý cố định, không cần buffer mới):

- **AMP+MERGE:** không `kmalloc` — "cắt" `drvsize` byte từ đầu vùng nhớ chung (`warp_drv_buf = work`), rồi đẩy con trỏ `work` tới và trừ `size` tương ứng. Áp dụng cho **cả CPU chính lẫn CPU phụ** (điều kiện chỉ là `warp_amp()`, không phân biệt main/sub) — vì buffer chứa driver cần nằm ở vị trí mà cả hai CPU đều biết trước, không thể để mỗi CPU tự `kmalloc` một chỗ khác nhau.
- **Trường hợp còn lại:** `kmalloc(drvsize, GFP_KERNEL)`. Thất bại → log, `uc_ngPoint = 2`, `-ENOMEM`. Thành công → tính PFN bắt đầu (`page_to_pfn(virt_to_page(...))`) và PFN kết thúc (làm tròn lên đủ số trang: `((drvsize-1) >> PAGE_SHIFT) + 1` trang), ghi vào phần tử tiếp theo của `warp_nosave_work[]`, tăng `warp_nosave_work_num` — **đăng ký buffer này là "không lưu vào snapshot"**, vì nội dung của nó (mã driver) sẽ được nạp lại từ đầu ở lần hibernate kế tiếp. Dòng `printk` cuối chỉ log địa chỉ vùng driver để đối chiếu debug, chạy trong cả hai nhánh.

**[L1021-1042](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1021-L1042) — cấp phát buffer làm việc lớn (`warp_work`)**

- **AMP+MERGE và là CPU phụ** (`warp_amp() && !warp_amp_maincpu()`): không `kmalloc` — lấy nốt phần **còn lại** của vùng nhớ chung sau khi đã bị driver buffer ở bước trên "ăn" bớt (`warp_work = work`, `warp_work_size = size`). Không cần đăng ký vào `warp_nosave_work[]` vì vùng này vốn đã nằm ngoài bộ nhớ cấp phát động thông thường của CPU phụ.
    
- **Mọi trường hợp khác (kể cả CPU chính trên hệ AMP+MERGE):** luôn `kmalloc(WARP_WORK_SIZE, GFP_KERNEL)` (mặc định 512 KB) — **CPU chính không bao giờ lấy vùng làm việc của mình từ vùng chung**, vì đây chính là nơi CPU chính dựng bảng save-table thật sự dùng để chụp/gộp dữ liệu, cần được quản lý như bộ nhớ kernel bình thường. Thất bại → log, `uc_ngPoint = 3`, `-ENOMEM`. Thành công → đăng ký PFN range vào `warp_nosave_work[]` (cùng công thức làm tròn trang như driver buffer), tăng bộ đếm, gán `warp_work_size`, log địa chỉ.
    
    _Liên hệ với câu trả lời trước:_ đây chính là lý do vì sao trong `hibdrv_snapshot()` (khối AMP), CPU chính phải `warp_invalidate_dcache_range(WARP_AMP_WORK, ...)` trước khi `warp_merge_savearea()` — vì bảng save-table của CPU phụ nằm ngay trong vùng nhớ chung `WARP_AMP_WORK` này (được cấp phát ở nhánh CPU‑phụ vừa giải thích ở trên), chứ không phải một buffer `kmalloc` riêng như của CPU chính.
    

**[L1044](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1044) — `return 0`** Thành công — cả 2 (hoặc 3, tính cả `hd_savearea`) buffer đã sẵn sàng để `warp_work_init()` gọi tiếp `warp_load_drv()` và cắt `warp_work` thành 3 bảng save-table.

### `warp_load_drv()`→`warp_load_drv_low()`

**High level:** nạp mã nhị phân của hibernation driver (và các UserAPI driver) vào RAM bằng đúng phương thức vận chuyển cấu hình cho mỗi driver, rồi flush icache để vùng đó thực sự chạy được như code.

![[Pasted image 20260820143147.png]]

```
warp_load_drv()
│
├─ 1. Xác định "driver hibernation nằm ở đâu" (địa chỉ gốc)
│      │
│      ├─ Nếu driver chính đã có sẵn ở địa chỉ vật lý cố định
│      │    → dùng thẳng địa chỉ đó, ghi thêm thông tin lệch
│      │      virt/phys riêng cho kiểu cố định này
│      │
│      └─ Nếu driver chính cần được nạp vào RAM
│           → dùng buffer vừa cấp phát trước đó làm địa chỉ gốc,
│             dùng công thức lệch virt/phys chung của toàn kernel
│
├─ 2. Đặt "con trỏ đang rải" tại đúng địa chỉ gốc đó
│
├─ 3. Với TỪNG driver (driver chính + từng UserAPI driver):
│      │
│      ├─ Driver không tồn tại/không dùng → bỏ qua, sang cái tiếp
│      │
│      ├─ Driver cần NẠP VÀO RAM:
│      │    ├─ 3a. Không có tồn tại thật (size = 0) → bỏ qua
│      │    ├─ 3b. Nạp dữ liệu vào đúng vị trí con trỏ đang rải
│      │    │       (có xác thực chữ ký nếu bật secure boot)
│      │    │       → lỗi thì dừng cả hàm ngay
│      │    ├─ 3c. [Hệ nhiều CPU] chuyển bản sao cho CPU phụ
│      │    ├─ 3d. Ghi nhớ địa chỉ vật lý nơi vừa đặt driver này
│      │    └─ 3e. Xả cache lệnh vùng vừa ghi — để CPU chạy được
│      │           đúng code mới, không phải rác cache cũ
│      │
│      ├─ Driver đã có sẵn ở địa chỉ cố định (không phải driver
│      │    chính) → chỉ ghi nhận địa chỉ có sẵn, không đụng gì
│      │
│      ├─ 3f. Dịch con trỏ "đang rải" tới vị trí kế tiếp (làm
│      │       tròn lên theo trang) — để driver/UserAPI tiếp theo
│      │       không đè lên driver vừa đặt
│      │
│      └─ 3g. Ghi nhớ driver UserAPI này được gán chạy cho CPU nào
│
└─ 4. Ghi tổng số driver hiện có, trả về thành công

```

**Ý nghĩa tổng thể:** hàm này chỉ có một việc — biến "một danh sách cấu hình driver" (`warp_drv_info[]`, tĩnh, do board khai báo) thành "một dải bộ nhớ thật, đã có code executable nằm sát nhau theo trang, và một bảng địa chỉ vật lý tương ứng" (`warp_param.drv_phys[]`) để driver cấp thấp — vốn chạy độc lập, không còn dùng địa chỉ ảo của kernel — biết chính xác từng mảnh code của mình (và của các UserAPI driver) nằm ở đâu.

### Ý nghĩa từng dòng code

**[L766-780](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L766-L780) — xác định địa chỉ gốc `warp_hibdrv_addr`** Rẽ nhánh theo mode của driver #0 (driver hibernation chính):

- **`FIXED`** (đã có sẵn ở một địa chỉ vật lý cố định do board cấu hình, không cần buffer mới): `text_v2p_offset` được tính riêng bằng hiệu số `virt - phys` của chính vùng cố định đó (ép `u64` vì có thể chênh lệch lớn) — vì driver cấp thấp cần công thức đổi virt↔phys **đúng cho vị trí thật của nó**, không dùng công thức chung của kernel được. `text_size` ghi lại kích thước vùng cố định. `use_free_mem` bật/tắt theo cờ biên dịch `WARP_USE_FREE_MEM` — báo cho driver biết nó có được phép tự do dùng thêm bộ nhớ free hay không (chỉ có ý nghĩa khi driver không tự chiếm hết vùng nhớ nó cần). `warp_hibdrv_addr` = thẳng địa chỉ ảo cố định đó — không có bước copy nào.
- **`FLOATING`** (trường hợp thường gặp — cần nạp vào RAM): `text_v2p_offset` dùng lại công thức lệch chung `warp_param.v2p_offset` (tính một lần trong `warp_init()`), vì sau khi nạp, driver nằm trong vùng nhớ kmalloc bình thường, theo đúng ánh xạ tuyến tính chuẩn của kernel. `use_free_mem = 1` luôn (dĩ nhiên — bản thân buffer chứa nó đã được cấp phát từ bộ nhớ free). `warp_hibdrv_addr = warp_drv_buf` — buffer đã cấp phát ở `warp_work_alloc()`.

**[L781](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L781) — khởi tạo con trỏ "đang rải"** `floating_buf = warp_hibdrv_addr` — biến này sẽ **trượt dần về phía trước** mỗi khi một driver floating được đặt xong, đóng vai trò "vị trí tiếp theo còn trống" trong dải bộ nhớ chứa driver.

**[L783-784](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L783-L784) — vòng lặp qua mọi driver** Lặp `no` từ 0 đến `WARP_DRV_NUM - 1` (driver #0 = hibernation driver chính, các `no` còn lại = UserAPI driver), lấy cấu hình tĩnh `drv = &warp_drv_info[no]` cho mỗi slot.

**[L785-806](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L785-L806) — nhánh `FLOATING`: nạp thật vào RAM** `size = drv_size[no]` lấy kích thước đã tính sẵn từ `warp_get_drv_size()`. `size == 0` nghĩa là slot này được cấu hình nhưng **không tìm thấy driver hợp lệ** (nhớ lại: với UserAPI driver, `warp_get_drv_size()` không coi đây là lỗi chết, chỉ log cảnh báo và set size=0) → `continue`, bỏ qua hoàn toàn, không tốn chỗ trong buffer. Đặt tên hiển thị (`"Hibernation driver"` hoặc `"UserAPI driver N"`) chỉ để log dễ đọc. Nạp dữ liệu thật theo 1 trong 2 đường:

- Có `CONFIG_PM_WARP_SECURE_BOOT`: giao cho `warp_ops->sb_load(...)` — hàm board-specific xác thực/giải mã chữ ký, và **bản thân nó nhận `warp_load_drv_low` làm tham số** để dùng làm "phương tiện đọc dữ liệu thô" bên trong (đọc xong phần thô rồi mới verify) — kiểu inject-dependency đơn giản.
- Không có secure boot: gọi thẳng `warp_load_drv_low(no, floating_buf, size)` (đã giải thích ở câu trả lời trước — tự chọn mem/dev/mtd theo cấu hình). Lỗi ở cả hai đường đều `return` ngay, bỏ dở toàn bộ hàm.

**[L807-810](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L807-L810) — chuyển bản sao cho CPU phụ (chỉ AMP)** Nếu đang chạy trên CPU chính và board có cung cấp hook `drv_copy`, gọi nó để **forward driver vừa nạp sang cho CPU phụ** (thường là copy vào vùng nhớ chia sẻ) — tránh CPU phụ phải tự đọc lại từ device/MTD một lần nữa, tốn thời gian và I/O trùng lặp.

**[L811-814](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L811-L814) — ghi nhận vị trí & làm cho code chạy được** `drv_phys[no] = __pa(floating_buf)` — driver cấp thấp chỉ nhận được `warp_param`, không có ngữ cảnh kernel đầy đủ, nên mọi vị trí phải là địa chỉ vật lý. `drv_floating[no] = 1` đánh dấu slot này "vừa được nạp động". `warp_flush_icache_range(...)` — bắt buộc trên nhiều kiến trúc có cache lệnh (icache) tách biệt cache dữ liệu (dcache): dữ liệu vừa ghi vào RAM (qua memcpy/DMA) chỉ cập nhật dcache, icache của CPU có thể vẫn giữ "rác" hoặc trống ở vùng địa chỉ đó — nếu không flush, khi CPU nhảy vào chạy như code, nó có thể thực thi lệnh sai/cũ.

**[L815-817](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L815-L817) — nhánh `FIXED` (driver không phải #0)** Không nạp gì cả — chỉ ghi nhận địa chỉ vật lý đã cấu hình sẵn (`drv->drv.fixed.phys`) và đánh dấu `drv_floating[no] = 0`. Áp dụng cho một UserAPI driver được board khai báo là đã có sẵn cố định trong bộ nhớ/flash-mapped, không cần copy.

**[L818-820](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L818-L820) — slot rỗng** Mode không phải FLOATING cũng không phải FIXED nghĩa là slot này **không được board sử dụng** → `continue`, không ghi gì vào `warp_param`, không đụng tới `floating_buf`.

**[L822-824](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L822-L824) — dịch con trỏ "đang rải" tới vị trí kế tiếp** Điều kiện `drv->mode == WARP_DRV_FLOATING || no == 0` — đáng chú ý là **kể cả khi driver #0 là FIXED**, con trỏ vẫn bị dịch đi theo kích thước của nó (`drv_size[0]`, làm tròn lên bội số `WARP_PAGE_SIZE` bằng thủ thuật bit `(size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)`). Lý do: driver cấp thấp được biên dịch với giả định "các UserAPI driver nằm ngay liền sau vùng driver chính theo thứ tự", bất kể driver chính đó là cố định hay vừa nạp động — nên `floating_buf` phải luôn phản ánh đúng "chỗ trống tiếp theo sau driver chính", không riêng gì trường hợp FLOATING. Ngược lại, một UserAPI driver ở dạng FIXED (`no > 0`) thì **không** làm dịch con trỏ, vì nó nằm ở một địa chỉ vật lý hoàn toàn tách biệt, không thuộc dải bộ nhớ liên tục này.

**[L826-827](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L826-L827) — ghi nhận CPU phụ trách** Chỉ với `no > 0` (tức các UserAPI driver, không tính driver chính): lưu `drv->cpu` (CPU được board gán cho driver này, từ cấu hình `warp_drv_info[]`) vào `warp_param.userapi_cpu[no - 1]` — chỉ số bị trừ 1 vì mảng `userapi_cpu[]` đánh số riêng cho UserAPI (0..N-1), tách biệt với mảng `warp_drv_info[]` vốn dành slot 0 cho driver chính.

**[L829](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L829) — `warp_param.drv_num = WARP_DRV_NUM;`** Sau vòng lặp, ghi tổng số driver (hằng số biên dịch, bằng kích thước mảng `warp_drv_info[]`) vào `warp_param` — driver cấp thấp cần biết con số này để biết duyệt bao nhiêu phần tử hợp lệ trong `drv_phys[]`/`drv_floating[]`.

**[L831](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L831) — `return 0`** Thành công — mọi driver hợp lệ đã ở đúng vị trí, thực thi được, và `warp_param` đã có đủ thông tin để `hibdrv_snapshot()` sau này nhảy vào (`WARP_DRV_SNAPSHOT`/`WARP_DRV_SWITCH`) mà không cần quay lại kernel context nữa.

## `warp_shrink_memory()`

**High level:** giảm tối đa lượng RAM "đang dùng" trước khi chụp, để save‑table càng nhỏ càng tốt và quá trình copy ra thiết bị càng nhanh.

![[Pasted image 20260820143206.png]]

```
warp_shrink_memory()
│
├─ 1. Lưu tạm mức shrink hiện tại đang cấu hình
│      (để cuối hàm trả lại đúng như cũ, vì bước 2 có thể ghi đè nó)
│
├─ 2. Quyết định "chơi tất tay hay theo đúng cấu hình"
│      │
│      ├─ Swap-out đang ĐƯỢC PHÉP dùng
│      │    → ép mức shrink lên tối đa + ép số vòng lặp lên rất lớn
│      │      (đang còn thời gian, tận dụng swap để nén RAM ít nhất
│      │       có thể, bỏ qua mức người dùng đã cấu hình)
│      │
│      └─ Swap-out đang BỊ khoá
│           → giữ nguyên mức shrink & số vòng lặp đã cấu hình
│             (đây thường là lượt chạy gấp/cuối cùng, không có
│              nhiều thời gian để làm quá tay)
│
├─ 3. Ghi log tình trạng bộ nhớ "trước khi dọn" (chỉ để so sánh)
│
├─ 4. Xả sạch cache của mọi block device
│      (dữ liệu đã có bản durable trên đĩa thì không cần giữ bản
│       sao trong RAM nữa → các trang đó trở thành "free", giảm số
│       trang phải đưa vào snapshot)
│
├─ 5. Nếu mức shrink không phải "tắt hẳn":
│      │
│      ├─ 5a. Với các mức "giới hạn" (LIMIT1/LIMIT2), chỉ lặp một
│      │      số vòng rất ít — dọn nhẹ nhàng, không cố ép RAM
│      │
│      └─ 5b. Lặp gọi "thu hồi bộ nhớ toàn hệ thống", mỗi vòng cố
│             giải phóng càng nhiều trang càng tốt:
│               │
│               └─ nếu một vòng gần như không giải phóng thêm được
│                  gì nữa suốt nhiều lần liên tiếp → dừng sớm,
│                  không lặp hết số vòng cho đủ nếu đã hết tác dụng
│
├─ 6. Ghi log tổng số trang đã giải phóng + tình trạng bộ nhớ "sau khi dọn"
│      (so với bước 3 để thấy hiệu quả)
│
├─ 7. Trả lại mức shrink như lúc ban đầu (undo bước 2 nếu đã ép)
│
└─ 8. Luôn trả về thành công

```

**Ý nghĩa tổng thể:** hàm này chỉ có một mục đích — _làm cho tập hợp trang RAM phải đưa vào snapshot càng nhỏ càng tốt_, trước khi `warp_make_save_table()` chạy. Nó làm việc đó bằng hai đòn: xả cache block-device (loại bỏ bản sao dữ liệu đã nằm sẵn trên đĩa) và ép kernel thu hồi bộ nhớ nhiều lần. Mức độ "ép mạnh tới đâu" tự động thay đổi tuỳ theo swap có được phép dùng hay không, bất kể người dùng đã cấu hình gì qua `/proc/warp/shrink`.

### Ý nghĩa từng dòng code


| Dòng code                                                                                                                                                                              | Ý nghĩa                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **[L1734-1736](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1734-L1736) — khai báo & lưu trạng thái**                  | `shrink_sav = warp_shrink` chốt lại giá trị **hiện đang cấu hình** qua `/proc/warp/shrink` trước khi hàm có thể ghi đè nó ở bước sau — đảm bảo sau khi hàm chạy xong, cấu hình của người dùng không bị "rò rỉ" thay đổi. `repeat` khởi tạo bằng `WARP_SHRINK_REPEAT` — [10 vòng lặp](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L104-L106) trên đa số kernel, hoặc [1 vòng](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L100-L102) riêng trên dải kernel 2.6.18‑2.6.33.                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| **[L1738-1741](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1738-L1741) — override khi swap còn dùng được**            | `if (!warp_swapout_disable)`: nếu swap **không** bị khoá (nhớ lại: `warp_swapout_disable` được `hibernate()` set tuỳ theo `separate`/`oneshot` — điển hình là true ở lượt "cuối cùng, gấp gáp", false ở lượt "chuẩn bị, còn thời gian"), hàm **bỏ qua cấu hình `/proc/warp/shrink` của người dùng**, ép `warp_shrink = WARP_SHRINK_ALL` (mức mạnh nhất) và `repeat = WARP_SHRINK_REPEAT_P1` = [10000](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L115-L117) — tức "cứ lặp cho tới khi không còn tác dụng gì nữa" (nhờ cơ chế dừng sớm ở bước dưới, chứ không thật sự chạy đủ 10000 lần).                                                                                                                                                                                                                                                                                                                                                                                                  |
| **[L1743](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1743) — `warp_print_meminfo()` lần 1**                          | In số liệu Buffers/Cached/Active/Dirty/Slab… làm mốc "trước khi dọn", chỉ phục vụ debug/so sánh.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| **[L1744-1746](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1744-L1746) — `warp_drop_bdev_cache()`**                   | Ghi (flush) toàn bộ trang dirty của mọi block device xuống thiết bị thật, rồi loại các trang cache sạch ra khỏi bộ nhớ. Vì dữ liệu đó giờ đã tồn tại bền vững trên đĩa/flash, các trang RAM giữ bản sao của nó không cần thiết phải nằm trong snapshot nữa — trực tiếp giảm số trang mà `warp_make_save_table()` sẽ đánh dấu "cần lưu" sau này.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| **[L1748](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1748) — cổng kiểm tra `WARP_SHRINK_NONE`**                      | Nếu mức shrink (đã có thể bị ép ở bước 2) là "tắt hẳn", toàn bộ vòng lặp thu hồi bộ nhớ bên dưới bị bỏ qua — chỉ có bước xả cache bdev ở trên là còn tác dụng.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| **[L1749-1755](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1749-L1755) — tinh chỉnh `repeat` theo mức LIMIT1/LIMIT2** | Chỉ áp dụng ngoài dải kernel 2.6.18‑2.6.33 (dải đó không có khái niệm REPEAT2/3). Nếu người dùng chọn mức nhẹ hơn "ALL" — `WARP_SHRINK_LIMIT1` thì chỉ lặp [1 vòng](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L107-L109), `WARP_SHRINK_LIMIT2` thì lặp [2 vòng](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L110-L112) — hai mức trung gian giữa "không dọn" và "dọn tối đa 10 (hoặc 10000) vòng".                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| **[L1756-1768](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1756-L1768) — vòng lặp thu hồi bộ nhớ thật sự**            | Mỗi vòng gọi `shrink_all_memory(SHRINK_BITE)` — hàm lõi của Linux mm, yêu cầu kernel cố **thu hồi càng nhiều trang càng tốt ngay lập tức** (ghi dirty page ra swap/disk, drop clean page…), `SHRINK_BITE` = giá trị lớn nhất có thể (`ULONG_MAX`/`INT_MAX` tuỳ version) nghĩa là "không giới hạn số trang mỗi lần gọi". Kết quả mỗi vòng cộng dồn vào `pages_sum` để log tổng cuối cùng.<br><br>Cơ chế **dừng sớm**: nếu một vòng chỉ giải phóng được `≤ WARP_SHRINK_THRESHOLD` (= [1 trang](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L119-L121)), tăng bộ đếm "vòng vô ích liên tiếp"; đủ [100 vòng liên tiếp](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L123-L125) gần như không thu được gì thì `break` ngay — tránh lãng phí thời gian lặp cho đủ số vòng cấu hình khi việc thu hồi đã bão hoà. Ngược lại hễ có vòng nào giải phóng được nhiều hơn ngưỡng, bộ đếm bị reset về 0 (vẫn còn "ăn" được thì tiếp tục). |
| **[L1769](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1769) — log tổng kết**                                          | In tổng số trang đã giải phóng; ký tự `\b` (backspace) chỉ là mẹo canh dòng để đè lên dấu "..." đã in dở ở dòng "Shrinking memory... " phía trên cho log gọn hơn, không có ý nghĩa chức năng.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| **[L1771](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1771) — `warp_print_meminfo()` lần 2**                          | Mốc "sau khi dọn" để đối chiếu với lần in đầu — cho thấy hiệu quả thực sự của toàn bộ quá trình shrink trong log kernel.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| **[L1773](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1773) — khôi phục cấu hình**                                    | `warp_shrink = shrink_sav` trả `warp_shrink` về đúng giá trị người dùng đã đặt qua `/proc/warp/shrink` trước khi hàm này chạy — undo việc ép `WARP_SHRINK_ALL` ở bước 2 (nếu có), để các lần đọc `/proc/warp/shrink` sau đó phản ánh đúng cấu hình gốc, không phải giá trị tạm bị ép trong lúc chạy.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| **[L1774](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1774) — `return 0`**                                            | Hàm luôn báo thành công — không có nhánh lỗi nào trong toàn bộ thân hàm (khác với `warp_make_save_table()` hay `hibdrv_snapshot()` vốn có thể trả về âm).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |


# Từng function làm gì & làm như thế nào
Bấm vào từng dòng để xem chi tiết action + cơ chế thực hiện. Nhóm theo đúng 9 hạng mục ở phần 01; các hàm boilerplate thuần tuý (proc/sysfs get‑set, debug print) đã được giải thích đủ ở bảng trên nên không lặp lại ở đây.

## D · Nạp driver & bootflag
### Hàm `warp_get_drv_size()` - tính kích thước thật của từng driver blob
**Action:** với mỗi driver trong `warp_drv_info[]`, đọc 32 byte header đầu (từ mem/dev/MTD tuỳ mode), so magic ID với `WARP_ID_DRIVER`/`WARP_ID_USER_API` (hoặc bản "_SB" nếu secure boot).

**How:** nếu ID không khớp — driver chính (no\=\=0) thì lỗi cứng `-EIO`; UserAPI driver phụ thì chỉ log và coi như size 0 (bỏ qua, không chặn boot). Kích thước cuối được lấy từ macro `WARP_DRV_DATA_END`/`WARP_SB_ORG_SIZE` trong header, cộng dồn vào `warp_param.drv_total_size` sau khi align theo trang.

### Hàm `warp_load_drv_low`— nạp một blob driver theo đúng transport

**Action:** chọn 1 trong 4 nguồn dữ liệu: hook riêng của board (`warp_ops->drv_load`), RAM cố định (`WARP_LOAD_MEM`), block device (`WARP_LOAD_DEV`), hoặc MTD (`WARP_LOAD_MTD_NO/NAME`).

**How:** tra `drv->drv.floating.load` để biết nguồn, rồi gọi đúng hàm vận chuyển tương ứng (warp_load_mem / warp_load_dev / warp_load_mtd_no / warp_load_mtd_nm) với offset + size đã cấu hình sẵn cho board.

### Hàm `warp_load_drv`— nạp toàn bộ driver + UserAPI vào một vùng liền kề

**Action:** xác định địa chỉ gốc (`warp_hibdrv_addr`) — hoặc là địa chỉ ảo cố định (mode FIXED, dùng `text_v2p_offset`), hoặc là `warp_drv_buf` vừa cấp phát (mode FLOATING). Sau đó lần lượt nạp từng driver, đặt liền sau nhau theo bội số trang (page‑aligned).

**How:** với mỗi driver đã nạp: ghi địa chỉ vật lý vào `warp_param.drv_phys[no]`, đánh dấu `drv_floating[no]`, flush icache vùng vừa ghi (vì code này sẽ được CPU thực thi trực tiếp sau này), và với board AMP thì copy thêm sang bộ nhớ chia sẻ cho CPU phụ.

### Hàm `warp_load_bf`— nạp và xác thực bootflag

**Action:** bootflag là khối dữ liệu nhỏ đánh dấu "đã có snapshot hợp lệ ở slot nào", được ghi/đọc từ savearea. Hàm đọc header, kiểm tra magic `WARP_ID_BOOTFLAG` (hoặc bản secure‑boot).

**How:** nếu bật secure boot, đọc trước `WARP_SB_HEADER_SIZE`, xác nhận ID "_SB", rồi giao cho `warp_ops->sb_load` giải mã/xác thực chữ ký toàn khối trước khi kiểm ID gốc.

## E · Vòng đời workspace

### Hàm `warp_work_alloc`— cấp 3 buffer scratch, đăng ký chúng là "không lưu"

**Action:** `kmalloc` lần lượt `hd_savearea` (4 KB), `warp_drv_buf` (kích thước = tổng driver, từ `warp_get_drv_size`), và `warp_work` (mặc định 512 KB).

**How:** mỗi buffer vừa cấp phát được quy đổi sang PFN (`page_to_pfn(virt_to_page(...))`) và ghi vào mảng `warp_nosave_work[]` — mảng này sau đó được `warp_make_save_table()` dùng để loại các trang này ra khỏi snapshot, vì nội dung của chúng chỉ là bảng/driver tạm thời, sẽ được dựng lại từ đầu ở lần hibernate tiếp theo chứ không cần khôi phục nguyên trạng.

### Hàm `warp_work_init`— nhạc trưởng của toàn bộ giai đoạn chuẩn bị

**Action:** nếu driver ở chế độ FIXED và chưa có virt address, remap thực thi vùng vật lý đó (`warp_remap_exec`). Gọi `warp_work_alloc` rồi `warp_load_drv`. Nếu `switch_mode` khác 0, nạp thêm bootflag của cả loadno hiện tại và loadno kế tiếp.

**How:** cắt `warp_work` thành 3 bảng theo thứ tự `dramtbl` → `exttbl` (8 phần tử mặc định) → `zonetbl` (8 phần tử mặc định), phần còn lại của buffer dành hết cho `dramtbl` (thường chiếm phần lớn 512 KB). Cuối cùng tạo kobject `warp_kobj` để export `dramtbl_max/num`, `ngPoint` ra sysfs debug.

## F · Xây bảng lưu (save table)

### Hàm `warp_set_savearea`— API ngoài: đăng ký một dải nhớ 64‑bit cần lưu

**Action:** kiểm tra alignment (4 hoặc 8 byte tuỳ 32/64‑bit), rồi thêm dải vào `exttbl`.

**How:** nếu `exttbl` đầy, hàm "mượn" không gian: dịch `exttbl` lùi lại `EXTTBL_DEFAULT_NUM` phần tử vào vùng của `dramtbl` (miễn là dramtbl còn đủ chỗ), rồi `memmove` nội dung hiện có sang vị trí mới trước khi ghi thêm — cơ chế "bảng tự giãn" này lặp lại ở cả `warp_set_save_zones`.

### Hàm `warp_set_save_drams / warp_set_save_dram` — trang này CHẮC CHẮN đi vào snapshot

**Action:** thêm dải PFN vào `dramtbl` — bảng quan trọng nhất, vì nó chính là danh sách "hãy copy các trang vật lý này ra thiết bị".

**How:** gọi `warp_set_tbl`, tự gộp với phần tử liền trước nếu hai dải liên tiếp nhau (giảm số entry, tăng hiệu quả nén/di chuyển theo khối lớn).

### Hàm `warp_set_nosave_dram`— tìm khoảng trống DRAM lớn nhất

**Action:** mỗi khi gặp một trang "không lưu" (free/forbidden), hàm kiểm tra xem nó có nối tiếp dải không‑lưu trước đó không, để cập nhật độ dài dải hiện tại; đồng thời so với kỷ lục `warp_param.maxarea/maxsize` (toàn hệ thống) và `lowmem_maxarea/lowmem_maxsize` (chỉ vùng nhớ thấp, quan trọng trên board 32‑bit).

**How:** mục đích của việc tìm "khoảng trống lớn nhất" là để driver cấp thấp biết nơi an toàn nhất đặt vùng làm việc tạm của chính nó khi chạy — tránh ghi đè lên dữ liệu sắp/đang được lưu.

### Hàm `warp_make_save_table`— hàm lõi: quét toàn bộ RAM vật lý

**Action:** lặp qua mọi zone bộ nhớ (`for_each_zone`), gọi `mark_free_pages` để đánh dấu trang nào hiện đang free, rồi duyệt từng PFN trong zone đó.

**How:** với mỗi PFN hợp lệ và không bị cấm (`swsusp_page_is_forbidden` — tức không thuộc ảnh kernel `__nosave_begin/end`) và không thuộc buffer scratch của warp (`warp_nosave_work[]`): ghi vào `zonetbl` (mọi trang hợp lệ), sau đó hỏi `swsusp_page_is_saveable` — nếu có, cộng vào `warp_save_pages` và ghi vào `dramtbl`; nếu không (trang free), gọi `warp_set_nosave_dram` để cập nhật khoảng trống lớn nhất. Kết quả cuối là 3 bảng mô tả chính xác bức tranh bộ nhớ tại thời điểm chụp.

## H · Điều phối snapshot

### Hàm `hibdrv_snapshot`— cầu nối giữa "biết lưu gì" và "lưu thật"

**Action:** gọi `warp_make_save_table()` để có 3 bảng; nếu board AMP, chờ và gộp save‑table từ CPU phụ (`warp_merge_savearea`); rồi gọi báo tiến độ `WARP_PROGRESS_SAVE`.

**How:** quy đổi 3 bảng sang địa chỉ vật lý (`__pa()`) rồi ghi vào `warp_param`. Nếu `switch_mode == 0`: gọi thẳng `WARP_DRV_SNAPSHOT(warp_hibdrv_addr, &warp_param)` — đây là lệnh nhảy thực sự vào code driver blob đã nạp sẵn trong RAM, chạy độc lập với phần còn lại của kernel để copy từng dải trong dramtbl/exttbl ra thiết bị lưu trữ. Nếu `switch_mode != 0`, còn có thêm bước ghi bootflag vào cuối vùng driver và gọi `WARP_DRV_SWITCH` để chuyển sang một snapshot slot khác mà không cần khởi động lại đầy đủ. Các mã lỗi trả về (EIO, ENOMEM, ENODEV, ETIMEDOUT, ECANCELED…) được dịch thành thông báo log tương ứng.

### Hàm `hibernate`— entry point cao nhất, thay thế hibernate() chuẩn

**Action:** thực hiện đúng 6 giai đoạn ở sơ đồ phần 02 — chuẩn bị workspace, freeze + shrink, đưa thiết bị/CPU vào trạng thái nghỉ, chụp snapshot, đánh thức, rã đông + dọn dẹp.

**How:** mỗi bước đều có một nhãn lỗi `*_err` riêng và dùng `goto` để nếu bước sau thất bại, unwind chính xác các bước trước đó theo thứ tự ngược (ví dụ: nếu `freeze_kernel_threads` lỗi thì phải `dpm_complete` rồi mới `thaw_kernel_threads`, mới tới `thaw_processes`…). Biến toàn cục `pm_device_down` theo dõi trạng thái hiện tại (NORMAL / SUSPEND / RESUME) để các driver khác trong hệ thống (qua `system_entering_hibernation()`) biết mà cư xử phù hợp. Đoạn code đặc thù `CONFIG_KM_BIZHUB` bật/tắt GPIO195 ("SD_PWR_EN") quanh lời gọi `warp_ops->snapshot()` — chu kỳ nguồn cho khe SD trước/sau khi ghi, để đảm bảo ghi ổn định.

### G · Giải phóng bộ nhớ & drop cache

### Hàm `warp_shrink_memory`— nén nhu cầu RAM trước khi chụp

**Action:** in thống kê bộ nhớ, drop cache block device, rồi (nếu `warp_shrink != WARP_SHRINK_NONE`) lặp gọi `shrink_all_memory(SHRINK_BITE)`.

**How:** số lần lặp phụ thuộc cấu hình (`WARP_SHRINK_REPEAT*`) và việc `warp_swapout_disable` có bật hay không — nếu tắt swap‑out hoàn toàn thì lặp tới `WARP_SHRINK_REPEAT_P1` (10 000) lần với ngưỡng dừng sớm khi số trang giải phóng mỗi vòng liên tục dưới `WARP_SHRINK_THRESHOLD` trong `WARP_SHRINK_THRESHOLD_COUNT` vòng liền — tức "dừng khi không còn hiệu quả" thay vì chạy đủ số vòng cố định.

# Cơ chế snapshot: bộ nhớ nào, thiết bị nào, hoạt động ra sao

Trả lời trực tiếp 3 câu hỏi con, dựa trên bằng chứng trong chính file warp.c (biến, macro, cấu trúc dữ liệu).


| Ảnh kernel + reserved  <br>(pfn_is_nosave, forbidden) | Scratch của warp!!<br>had_savearea + warp_drv_buf + warp_work (warp_nosave_work[]) | Trang RAM còn lại -> dramtbl/zonetbl/axttbl = nội dung thật của snapshot |
| ----------------------------------------------------- | ---------------------------------------------------------------------------------- | ------------------------------------------------------------------------ |
| Không bao giờ chạm tới — kernel tự quản               | Bị loại khỏi snapshot — sẽ được nạp lại từ đầu ở lần sau                           | Được driver cấp thấp copy ra thiết bị                                    |

warp_hibdrv_addr chạy độc lập, đọc trực tiếp theo dramtbl/exttbl → warp_savearea[warp_saveno]


| **WARP_LOAD_MEM**                                                                   | **WARP_LOAD_DEV**                                                                             | **WARP_LOAD_MTD_NO / NAME**                                                      |
| ----------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| địa chỉ RAM cố định — dùng khi thiết bị đích cũng chỉ là một vùng bộ nhớ đã map sẵn | block device thô, mở qua warp_load_dev/warp_save_dev — ví dụ SD card / eMMC trên board bizhub | partition MTD (NAND/NOR flash thô), đọc qua warp_load_mtd, tự nhảy qua bad block |

### Memory nào lưu nội dung snapshot? Memory nào chứa driver Warp?
Đây là hai vùng RAM khác nhau và **không chồng lên nhau** — chính vì thế `warp_work_alloc()` phải đăng ký cả hai vào `warp_nosave_work[]`.

**Nội dung snapshot** không được gom vào một buffer trung gian nào cả — nó chính là các trang RAM "bình thường" đang tồn tại sẵn trong hệ thống (dữ liệu tiến trình, page cache còn dirty, v.v.). `warp_make_save_table()` chỉ tạo ra **bảng địa chỉ** (`dramtbl`/`zonetbl`/`exttbl`) mô tả các dải PFN đó; driver cấp thấp sẽ đọc trực tiếp từ đúng vị trí vật lý gốc của chúng khi copy ra thiết bị — không có bước "sao chép nháp vào RAM" nào ở giữa.

**Driver Warp!!** (mã nhị phân thực thi lúc "điểm không quay lại") nằm ở `warp_hibdrv_addr`, trỏ tới `warp_drv_buf` (nếu driver ở chế độ FLOATING, vừa `kmalloc` và nạp bằng `warp_load_drv`) hoặc một địa chỉ vật lý cố định đã `ioremap` thực thi được (nếu driver ở chế độ FIXED). Vùng này bị loại khỏi snapshot có chủ đích, vì nó sẽ được nạp lại từ thiết bị/MTD ở lần hibernate _tiếp theo_ — không cần và không nên tự lưu chính mình.

### Thiết bị nào lưu snapshot?
warp.c không tự quyết định thiết bị — nó chỉ đọc cấu hình từ mảng `struct warp_savearea warp_savearea[]` (được định nghĩa bằng macro `WARP_SAVEAREA` trong header đặc thù từng board, nằm ngoài file này). Biến `warp_saveno` chọn "slot" nào trong mảng đó sẽ được dùng để lưu (mỗi slot có field `.snapshot[cpu].dev/offs/size` cho từng CPU và `.bootflag.dev/offs/size`), và `warp_loadno` chọn slot dùng để nạp lại khi resume.

Slot đó có thể trỏ tới ba loại thiết bị (xem 3 thẻ ở sơ đồ trên): một vùng RAM cố định, một block device thô, hoặc một partition MTD/flash thô — tuỳ vào `.load` method được cấu hình. Đáng chú ý: đoạn code riêng cho `CONFIG_KM_BIZHUB` trong `hibernate()` bật GPIO195 ("`SD_PWR_EN`") ngay trước khi gọi `warp_ops->snapshot()`, rồi tắt/bật lại ngay sau đó — dấu hiệu khá rõ ràng rằng trên board bizhub cụ thể này, đích lưu snapshot là **thẻ SD**, và nguồn của nó được chu kỳ (power‑cycle) quanh thao tác ghi để đảm bảo ghi/đọc ổn định.

### Hiểu cơ chế snapshot như thế nào?

1. **Pha "trong kernel" (còn đầy đủ dịch vụ hệ thống).** `hibernate()` đóng băng tiến trình, đồng bộ filesystem, ép giải phóng RAM (`warp_shrink_memory`), rồi `warp_work_init()` nạp sẵn mã nhị phân driver cấp thấp vào RAM và dựng 3 bảng save‑table rỗng.
2. **Tính toán "cần lưu gì".** `hibdrv_snapshot()` gọi `warp_make_save_table()` quét toàn bộ RAM vật lý một lần, đổ kết quả vào dramtbl/zonetbl/exttbl — đây vẫn là code C bình thường, chạy trong ngữ cảnh kernel đầy đủ.
3. **Điểm không quay lại.** Thiết bị bị suspend, CPU phụ bị tắt, ngắt bị khoá (`local_irq_disable`), trạng thái CPU chính được lưu (`save_processor_state`). Từ đây kernel gần như "đóng băng" — không còn scheduler, không còn filesystem layer bình thường.
4. **Nhảy vào driver blob.** `WARP_DRV_SNAPSHOT()`/`WARP_DRV_SWITCH()` chuyển quyền điều khiển cho đúng đoạn mã đã nạp ở bước 1. Đoạn mã này (viết riêng, độc lập với kernel, có thể chạy cả khi MMU/cache ở trạng thái đặc biệt) tự đọc dramtbl/exttbl, copy từng dải trang vật lý ra thiết bị đích theo cấu hình `warp_savearea[warp_saveno]` — có thể kèm nén (`warp_param.compress`) và, nếu bật secure boot, kèm ký/mã hoá.
5. **Trở về (hoặc khởi động lại từ một snapshot).** Sau khi driver blob hoàn tất, quyền điều khiển quay lại `hibernate()` ngay sau lệnh gọi — `warp_stat`/`warp_retry` cho biết kết quả. Ở chiều ngược lại (resume/boot), chính driver blob này — được nạp bởi bootloader hoặc bởi `warp_load_drv` ở một lần chạy khác — sẽ đọc slot theo `warp_loadno`, ghi dữ liệu đã lưu trở lại đúng các dải vật lý đã ghi trong bảng, rồi nhảy về đúng điểm đã lưu trạng thái CPU.
6. **Unwind.** Bất kể thành công hay lỗi, `hibernate()` đưa thiết bị/CPU/tiến trình trở lại hoạt động theo đúng thứ tự ngược lại với lúc chuẩn bị, rồi gọi `warp_work_exit()` để giải phóng toàn bộ scratch RAM đã dùng ở bước 1.


Nói ngắn gọn: warp.c **không** dùng cơ chế swap‑file/swsusp‑image chuẩn của Linux. Nó tự làm ba việc mà swsusp thường làm trong kernel C thuần: (a) chọn trang RAM nào cần lưu, (b) tự mang theo driver I/O của riêng mình (không dùng lại block layer đầy đủ của kernel), và (c) tự quyết định thiết bị đích qua bảng cấu hình board — cho phép chạy trên phần cứng nhúng còn thiếu/không thể dùng driver chuẩn của Linux ở giai đoạn "điểm không quay lại".

# Chi tiết các hàm

## 1.`warp_init()`

[warp_init()](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2904-L2994) là `module_init` của driver, làm đúng 2 việc:

1. **Tạo cây `/proc/warp/*`** ([L2906-L2939](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2906-L2939)): mỗi tham số điều khiển được đăng ký thành một file trong `/proc/warp/`, tham số thứ 2 truyền cho `warp_proc_create(name, rw, fops)` là cờ **1 = đọc/ghi, 0 = chỉ đọc** — quyết định `mode = S_IRUGO | S_IWUSR` hay chỉ `S_IRUGO` ([L2889-L2893](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2889-L2893)).
2. **Gán giá trị mặc định** cho tất cả tham số từ Kconfig ([L2941-L2984](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2941-L2984)), rồi (nếu `WARP_WORK_ALLOC_INIT`) cấp phát tĩnh workspace ngay lúc boot thay vì lúc hibernate.

Đây thực chất là **bảng điều khiển runtime cho `hibernate()`** — userspace (init script, driver ứng dụng, hoặc chính người dùng bằng tay) ghi vào các file `/proc/warp/*` này _trước khi_ gọi `hibernate()`, và các biến toàn cục đó được đọc lại bên trong `hibernate()`/`hibdrv_snapshot()` ở lần gọi kế tiếp.
### Cơ chế `proc_warp_##name##_fops`

Ba tầng lồng nhau:

**Tầng 1 — helper chung, không biết tên tham số** ([L2596-L2640](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2596-L2640)):

- `read_proc_warp(buffer, count, offset, value)` — in `value` thành chuỗi thập phân vào buffer 16 byte, `copy_to_user` phần còn thiếu theo `*offset` (hỗ trợ đọc nhiều lần/từng phần như một file `/proc` bình thường).
- `write_proc_warp(buffer, count, offset, min, max, val, str)` — `copy_from_user`, `sscanf` ra số nguyên, **kiểm tra biên `[min,max]`**, in cảnh báo `printk` và trả `-EINVAL` nếu sai khoảng.

**Tầng 2 — tương thích version kernel** ([L2642-L2650](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2642-L2650)): kernel ≥ 5.6 dùng `struct proc_ops` với field `.proc_read/.proc_write`; kernel cũ hơn dùng `struct file_operations` với field `.read/.write`. Macro `warp_proc_read`/`warp_proc_write` là alias để code sinh ra dùng chung một tên field cho cả hai trường hợp.

**Tầng 3 — hai macro sinh code** ([L2652-L2691](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2652-L2691)):

- `PROC_READ(name, var)` → sinh `read_proc_warp_<name>()` (chỉ gọi lại `read_proc_warp` với biến `var`) và một `proc_warp_<name>_fops` chỉ có `.read`.
- `PROC_RW(name, var, str, min, max, func)` → sinh thêm `write_proc_warp_<name>()`: gọi `write_proc_warp` để parse+validate, rồi **chỉ gán `var = val` nếu `func(val) == 0`**. `func` là móc "validate/side-effect" tuỳ biến — đa số dùng `dummy()` (luôn trả 0, không làm gì thêm), nhưng `compress` dùng `check_compress_mode` và `separate` dùng `separate_pass_init` (xem bên dưới).

Vậy `proc_warp_stat_fops`, `proc_warp_error_fops`, … chỉ là **các struct fops sinh tự động**, mỗi cái trỏ tới một cặp hàm đọc/ghi rất mỏng bọc quanh đúng một biến toàn cục — không có logic nghiệp vụ nằm trong bản thân các fops, logic nằm ở **nơi biến đó được đọc bên trong `hibernate()`/`hibdrv_snapshot()`**.

## Ý nghĩa & tác dụng từng tham số

| /proc/warp/                         | Biến                                  | Default (Kconfig)         | Dùng ở đâu                                                                                                                                                                                                                                                                                                                                                                                    | Ý nghĩa / tác dụng                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| ----------------------------------- | ------------------------------------- | ------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `stat`<br><br>Read Only             | `warp_stat`                           | —                         | [hibernate() L2321-2324](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2321), [L2534](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2534)                                                                                                                       | Sau `warp_ops->snapshot()`, `warp_stat = warp_param.stat`. **0 = vừa lưu xong, thực thi tiếp tục theo nhánh save**; **khác 0 = CPU vừa được khôi phục từ một snapshot đã lưu trước đó** (cùng một điểm code, hai đường vào — giống swsusp cổ điển). Userspace đọc để biết boot này là "fresh save" hay "resumed".                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| `error`<br><br>Read/Write           | `warp_error`                          | —                         | [L2528](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2528)                                                                                                                                                                                                                                                                    | Ghi mã lỗi cuối của lần `hibernate()` gần nhất. Cho phép **ghi** để userspace tự reset về 0 sau khi đọc — dùng làm "đã xảy ra lỗi kể từ lần check trước chưa".                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| `retry`<br><br>Read Only            | `warp_retry`                          | —                         | [L2322](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2322)                                                                                                                                                                                                                                                                    | `warp_retry = warp_param.retry` — số lần driver cấp thấp phải retry I/O khi ghi snapshot (do board set trong `warp_param`).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| `canceled`<br><br>Read/Write        | `warp_canceled`                       | 0                         | [warp_save_cancel() L1921-1925](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1921), kiểm tra trong `hibdrv_snapshot`/`hibernate`                                                                                                                                                                                              | Cờ huỷ. Ghi trực tiếp "1" vào file này có tác dụng **giống hệt** gọi API kernel `warp_save_cancel()` — hủy lượt lưu đang/ sắp diễn ra (đặc biệt hữu ích với `separate=2`, huỷ ở giữa pass 1 và pass 2).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `saveno`<br><br>Read/Write          | `warp_saveno`<br>                     | `CONFIG_PM_WARP_SAVENO`   | khắp `hibernate()`, vd [L1949](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1949)                                                                                                                                                                                                                                             | Chọn **slot** trong `warp_savearea[]` sẽ dùng để **lưu** snapshot lần tới (board có thể có nhiều slot vật lý khác nhau).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| `loadno`<br><br>Read/Write          | `warp_loadno`                         | `CONFIG_PM_WARP_LOADNO`   | [warp_work_init L1116](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1116)                                                                                                                                                                                                                                                     | Chọn slot dùng để **nạp bootflag** hiện có (đọc trạng thái slot đó). Với `switch_mode==2`, `saveno` và `loadno` **bắt buộc khác nhau** ([L1949](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1949)) — không thể switch sang chính slot đang lưu.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| `switch`<br><br>Read/Write          | `warp_param.switch_mode`              | 0                         | [L1103](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1103), [L1832-1857](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1832-L1857), [L2128](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2128) | **0** = chụp snapshot bình thường (`WARP_DRV_SNAPSHOT`). **1** = bỏ qua cả bước `warp_shrink_memory` lẫn việc chụp mới — nhảy thẳng sang `WARP_DRV_SWITCH` để **chuyển tức thời** sang một snapshot đã lưu sẵn ở slot khác (fast user-switch, không cần reboot đầy đủ). **2** = làm cả hai: chụp snapshot mới trước (`loadf=0`), nếu quay lại với `stat==0` (tức vẫn đang ở nhánh save chứ chưa resume) thì tiếp tục `loadf=1` để switch sang slot khác ngay sau khi lưu xong.                                                                                                                                                                                                                                                                                                                                                                                   |
| `compress`<br><br>Read/Write        | `warp_param.compress`                 | `CONFIG_PM_WARP_COMPRESS` | truyền qua `warp_param` cho driver cấp thấp                                                                                                                                                                                                                                                                                                                                                   | Chọn chế độ nén dữ liệu khi ghi ra thiết bị. `check_compress_mode()` ([L2698-L2705](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2698-L2705)) chỉ chặn các giá trị "vùng chết" (`>2` nhưng `< WARP_USERAPI_COMP_MODE`) — 0/1/2 luôn hợp lệ (nén nội bộ), còn giá trị `≥ WARP_USERAPI_COMP_MODE` là **chọn một UserAPI driver cụ thể** (trong số `WARP_USERAPI_MAX` driver phụ đã nạp) để đảm nhiệm nén.                                                                                                                                                                                                                                                                                                                                                                                          |
| `shrink`<br><br>Read/Write          | `warp_shrink`                         | `CONFIG_PM_WARP_SHRINK`   | [warp_shrink_memory() L1738-1758](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1738-L1758)                                                                                                                                                                                                                                    | Mức độ giải phóng RAM trước khi chụp (0=NONE … tăng dần số vòng lặp `shrink_all_memory`). Cao hơn = snapshot nhỏ hơn nhưng hibernate chậm hơn.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| `separate`<br><br>Read/Write        | `warp_separate`                       | `CONFIG_PM_WARP_SEPARATE` | [L1972-1985](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L1972-L1985)                                                                                                                                                                                                                                                         | **0** = một lượt duy nhất, khoá swap‑out (`warp_swapout_disable=1`). **1** = một lượt, cho phép swap‑out. **2** = **hai lượt**: lần gọi `hibernate()` đầu (`warp_separate_pass` từ 0→1) chỉ chạy tới hết `warp_shrink_memory()` rồi thoát sớm ([L2130](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2130), coi như "pass chuẩn bị"); lần gọi kế tiếp (`pass=2`) mới thực sự chụp và khoá swap‑out. Dùng cho kịch bản mất điện: pha 1 chạy trước (còn thời gian), pha 2 chạy cực nhanh đúng lúc nguồn sắp mất. Ghi lại giá trị `separate` sẽ gọi `separate_pass_init()` ([L2707-2711](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2707-L2711)) để **reset `warp_separate_pass` về 0**, đảm bảo chuỗi luôn bắt đầu lại từ pass 1. |
| `division`<br><br>Read/Write        | `warp_cpu_info`<br>`[cpu].save_ratio` | mặc định 100 (CPU0)       | [hibernate() L2270-2281](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2270-L2281)                                                                                                                                                                                                                                             | Không phải 1 số mà là **danh sách phân tách dấu phẩy, một giá trị/CPU** (đọc/ghi qua `read/write_proc_warp_division`, [L2728-2803](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2728-L2803)). CPU nào có `save_ratio > 0` mới được cấp một slot thiết bị (`sa->snapshot[no]`) để tự ghi phần dữ liệu của mình; `save_ratio == 0` bị bỏ qua hoàn toàn. Đây là cách **chia việc ghi snapshot cho nhiều CPU** (AMP) song song ra các vùng/thiết bị khác nhau.                                                                                                                                                                                                                                                                                                                                       |
| `oneshot`<br><br>Read/Write         | `warp_param.oneshot`                  | `CONFIG_PM_WARP_ONESHOT`  | [L2301-2304](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2301-L2304) (trong `CONFIG_SWAP`)                                                                                                                                                                                                                                   | Nếu **không** bật oneshot và swap đã bị dùng một phần (`total_swap_pages > NR_SWAP_PAGES`) → khoá swap‑out để tránh phá swap hiện có. Bật oneshot ép `warp_swapout_disable = 0` luôn — nghĩa là "đây là một lần hibernate dùng một lần, không quan tâm hết swap vì sẽ không cần nữa".                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| `halt`<br><br>Read/Write            | `warp_param.halt`                     | `CONFIG_PM_WARP_HALT`     | truyền qua `warp_param` cho driver cấp thấp                                                                                                                                                                                                                                                                                                                                                   | Cờ báo driver cấp thấp **có tắt nguồn hệ thống ngay sau khi ghi xong snapshot hay không** (không có logic rẽ nhánh trong chính warp.c — chỉ được đọc bởi code driver board‑specific).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| `silent`<br><br>Read/Write          | `warp_param.silent`                   | `CONFIG_PM_WARP_SILENT`   | [L2287](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2287) (`warp_boot_param.silent = -1` = "giữ nguyên")                                                                                                                                                                                                                     | Mức verbosity console của driver cấp thấp lúc save/switch (0..3).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
| `secure_bf_dev`<br><br>Read Only    | —                                     | —                         | [read_proc_warp_secure_bf_dev L2807-2836](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2807-L2836)                                                                                                                                                                                                                            | Chỉ có khi `CONFIG_PM_WARP_SECURE_BOOT`. Trả về thiết bị/partition (`sa->sw_bootflag.dev.part` nếu MTD, hoặc `.dev.name` nếu block device) của slot `secure_saveno` — slot bootflag **đã ký hợp lệ gần nhất**.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| `secure_bf_offset`<br><br>Read Only | —                                     | —                         | [read_proc_warp_secure_bf_offset L2841-2864](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2841-L2864)                                                                                                                                                                                                                         | Offset tương ứng (`warp_savearea[secure_saveno].sw_bootflag.offs`).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |
`secure_saveno` (biến hậu thuẫn cho 2 file trên) chỉ được cập nhật ở cuối `hibernate()`: **`ret >= 0 && warp_stat == 0 && warp_separate_pass != 1`** ([L2534-2535](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Kernel/K-S800/Src/kernel/power/warp.c#L2534-L2535)) — tức chỉ sau một lượt **lưu mới thành công, không phải resume, và không phải đang giữa pass 1/2** thì mới coi `saveno` hiện tại là slot bootflag đáng tin để bootloader dùng ở lần khởi động kế tiếp.

Tất cả các tham số này ghép lại chính là **giao diện điều khiển runtime** cho pipeline đã mô tả ở artifact trước: userspace tinh chỉnh chúng qua `/proc/warp/*`, rồi `hibernate()`/`hibdrv_snapshot()` đọc lại để quyết định slot lưu, số pass, có nén/switch hay không, và chia việc giữa các CPU. Nếu bạn muốn, tôi có thể gộp bảng này vào artifact đã publish trước đó thành một mục con mới.