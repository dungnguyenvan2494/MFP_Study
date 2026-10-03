# Sơ đồ trạng thái — State Machine Ứng dụng (Application)

**Danh mục: State machine Ứng dụng.** Hai state machine được sở hữu bởi
logic ở cấp ứng dụng chứ không phải một driver/ngoại vi: trạng thái CA72,
và bitmask trạng thái cập nhật firmware IAP. Nguồn: `07_state_machines.md`
§1.4, §1.7.

## 4a. Trạng thái CA72 (bộ xử lý ứng dụng)

```mermaid
stateDiagram-v2
    [*] --> CA72_OFF : static init [km_ca72_status.c:11]

    CA72_OFF --> CA72_ON : ap_power_en_proc(STATE_INT)\n(AP_PWR_EN edge pending)\n/ set_ca72_status(CA72_ON)\n[km_extend_io.c:661-665]

    CA72_ON --> CA72_SLEEP : ap_power_en_proc(STATE_NORMAL),\nAP_PWR_EN pin reads RESET\n/ set_ca72_status(CA72_SLEEP)\n[km_extend_io.c:667-671]

    CA72_SLEEP --> CA72_ON : ap_power_en_proc(),\nAP_PWR_EN pin reads SET\n/ set_ca72_status(CA72_ON)

    CA72_ON --> CA72_OFF : s800_power_off() (any of 4 triggers)\n/ set_ca72_status(CA72_OFF)\n[km_extend_io.c:528]
    CA72_SLEEP --> CA72_OFF : s800_power_off()
    CA72_OFF --> CA72_OFF : s800_power_off() called again\n(no-op re-affirmation)
```

## Ghi chú — các transition không được thực thi cưỡng chế + state phát hiện cạnh ẩn

`set_ca72_status()` (`km_ca72_status.c:22-26`) ghi đè vô điều kiện —
**mọi mũi tên ở trên là một quy ước của bên gọi, không phải một điều kiện
bảo vệ mà setter thực thi cưỡng chế.** Ngoài ra,
`sleep_status_rem_eagle_proc()`/`_sparrow_proc()` mỗi hàm đều giữ một
**biến `static prev_status` riêng** (`km_extend_io.c:892-893,968-969`) để
phát hiện *cạnh* chuyển tiếp `ON→SLEEP`/`SLEEP→ON` — đây là một phần bộ
nhớ state machine thứ hai, độc lập, không được thể hiện trong sơ đồ ở
trên vì nó thuộc về bên tiêu thụ (consumer), không thuộc về bản thân
`km_ca72_status.c`; xem `07_state_machines.md` §1.4 để có thảo luận đầy
đủ, bao gồm cả tình trạng dữ liệu cũ đi một vòng lặp mà điều này gây ra.

Không có hành động entry/exit, không có timeout, không có state lỗi nào
tồn tại cho state machine này.

## 4b. Trạng thái cập nhật firmware IAP (tổ hợp bit, bán tường minh)

Không phải một phép phân hoạch thành các state rời rạc — mà là một
bitmask `uint32_t` (`g_iap_reg[CHANGE_STATUS]`) với 5 bit có thể đặt độc
lập. Được thể hiện dưới dạng các tổ hợp có thể đạt tới trên thực tế, theo
thứ tự chúng tích lũy:

```mermaid
stateDiagram-v2
    [*] --> Idle : boot, all bits clear [km_i2c_iap.c:49]

    Idle --> Session_Open : IAP_PG_COMMAND write,\nmagic number match\n/ bit IAP set [km_i2c_iap.c:315-322]

    Session_Open --> Data_Written : IAP_PG_COMMAND_DATA write(s)\n/ bit DATA_CMD_STS set\n(unconditionally, even on failure)\n[km_i2c_iap.c:370]

    Data_Written --> Data_Written : PSCPU_ERR : out-of-range write\n[GUARD: g_dest_addr out of bounds]\n/ bit PSCPU_ERR also set\n[km_i2c_iap.c:371-374]

    Data_Written --> Verified : IAP_PG_COMMAND_CHANGE_INFO write,\nCOMP bit set, checksum matches\n/ bit PG_CMD_STS set\n[km_i2c_iap.c:341-347]

    Data_Written --> Verify_Failed : IAP_PG_COMMAND_CHANGE_INFO write,\nCOMP bit set, checksum mismatch\n/ bits PG_CMD_STS + CHKSUM_ERR set\n[km_i2c_iap.c:336-347]

    Verified --> [*] : MSW_ON released\n/ msw_on_proc_iap():\nHAL_NVIC_SystemReset() [TERMINAL]\n[km_extend_io_iap.c:28-41]

    Verify_Failed --> [*] : MSW_ON released\n/ msw_on_proc_iap(): SAME transition —\nCHKSUM_ERR is NEVER checked\n[km_extend_io_iap.c:35]
    note right of Verify_Failed
        ENFORCEMENT GAP: no state transition
        exists that blocks this reset.
        10_error_recovery.md §2.9
    end note
```

Không có transition nào quay trở lại một tổ hợp thấp hơn được lập trình ở
bất kỳ đâu — chỉ có một reset toàn bộ (E1-E4) mới xóa các bit này thông
qua việc khởi tạo lại BSS. Không có retry nào tồn tại; các "state lỗi"
(self-loop PSCPU_ERR của `Data_Written`, `Verify_Failed`) là các ngõ cụt
gần như terminal mà master phải tự nhận biết từ bên ngoài.

## Truy vết

| Machine | Function | File:Line |
|---|---|---|
| CA72 status | `set_ca72_status`, `get_ca72_status` | `main/App/km_ca72_status.c:22,37` |
| IAP status bits | `i2c_recv_wait` (IAP) | `iap/App/km_i2c_iap.c:315-374` |
| IAP reset gate | `msw_on_proc_iap` | `iap/App/km_extend_io_iap.c:28-41` |
