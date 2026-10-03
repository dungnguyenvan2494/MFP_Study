# Activity Diagram — Recovery (cơ chế failsafe timeout khi tuần tự tắt nguồn)

Swimlanes: **Application** duy nhất (toàn bộ cơ chế này được điều khiển
bởi main-loop/software timer — xem `06_timer_event_processing.md` để biết
cách `g_km_timer[]` bên dưới đếm tick). Tương ứng với `10_error_recovery.md`
§2.8 và phát hiện "chuỗi timeout ~120s" tại `09_timing.md` §3.

```mermaid
flowchart TD
    START(["START — a power-down sequence begins"])

    subgraph APPL["Application — km_extend_io.c"]
        ARM["poweroff_timer_start()<br/>km_extend_io.c:1279<br/>arms MAXTIME1=60000ms,\nWDGTIME=2000ms(or I2C-set),\nBACKUP_WAITTIME=50ms,\nSEQTIME=9350ms(or I2C-set)"]
        WAITLOOP["[wait] poweroff_timer_proc()\npolled every super-loop pass<br/>km_extend_io.c:1305"]
        CHECK1{"PWROFF_MAXTIME1\nstate == End?"}
        REARM2["km_timer_set(MAXTIME1, 0, Normal)\n+ km_timer_set(MAXTIME2, 60000, Start)<br/>km_extend_io.c:1315-1316<br/>[RETRY WINDOW — another full 60s]"]
        CHECK2{"PWROFF_MAXTIME2\nstate == End?"}
        SETBIT1["timeout_log |= TMOUT_LIMIT<br/>km_extend_io.c:1322"]
        CHECK3{"PWROFF_WDGTIME\nstate == End?"}
        SETBIT2["timeout_log |= TMOUT_WDG<br/>km_extend_io.c:1329"]
        CHECKANY{"timeout_log != 0?"}
        SAVEFACTOR["poweroff_factor_save(timeout_log)<br/>km_extend_io.c:1334<br/>→ RTC backup register B_REGISTER_PWROFF_LOG"]
        POWEROFF["s800_power_off()<br/>km_extend_io.c:523<br/>[TERMINAL — see 09_shutdown diagram]"]
        NOACTION["no action this pass"]
        LOOPBACK(["loop back to WAITLOOP\nnext super-loop pass"])
    end

    START --> ARM --> WAITLOOP --> CHECK1
    CHECK1 -- yes --> REARM2 --> CHECK2
    CHECK1 -- no --> CHECK2
    CHECK2 -- yes --> SETBIT1 --> CHECK3
    CHECK2 -- no --> CHECK3
    CHECK3 -- yes --> SETBIT2 --> CHECKANY
    CHECK3 -- no --> CHECKANY
    CHECKANY -- yes --> SAVEFACTOR --> POWEROFF
    CHECKANY -- no --> NOACTION --> LOOPBACK
    LOOPBACK -.-> WAITLOOP
```

## Note on the "retry" node

`REARM2` là thứ duy nhất trong sơ đồ này giống một cơ chế retry — nhưng nó
không phải là việc thử lại một thao tác thất bại; đó là một **bộ đếm
ngược 60 giây độc lập, thứ hai**, được xếp chồng sau bộ đếm đầu tiên, tạo
ra một mức trần kết hợp ~120s trước khi `TMOUT_LIMIT` thực sự được ghi
nhận (`09_timing.md` §3). Không có backoff tăng dần và không có bộ đếm số
lần retry — chuỗi này chỉ có đúng một lần gia hạn, luôn cùng một độ dài.

## Traceability

| Node | Function | File:Line |
|---|---|---|
| poweroff_timer_start | `poweroff_timer_start` | `main/App/km_extend_io.c:1279` |
| poweroff_timer_proc | `poweroff_timer_proc` | `main/App/km_extend_io.c:1305` |
| poweroff_factor_save | `poweroff_factor_save` | `main/App/km_extend_io.c:144` |
| s800_power_off | `s800_power_off` | `main/App/km_extend_io.c:523` |
