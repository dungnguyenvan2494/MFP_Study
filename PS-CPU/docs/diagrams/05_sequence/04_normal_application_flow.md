# Sequence Diagram — Normal Application Flow (một vòng lặp super-loop)

Các participant: **Application** (thân của super-loop), **Peripheral**
(các chân GPIO được đọc/ghi xuyên suốt), **External Device** (SoC S800, mà
một số giai đoạn đọc chân của nó). Chi tiết đầy đủ theo từng giai đoạn đã
có sẵn trong `docs/diagrams/04_activity/02_main_application_loop.md` — sơ
đồ này thể hiện cùng một vòng lặp đó dưới dạng trao đổi message thay vì
flowchart, nhằm làm rõ ra **hiểm hoạ về thứ tự đọc-trước-ghi (read-before-
write ordering hazard)** như một sự thật về thứ tự message.

```mermaid
sequenceDiagram
    participant APP as Application (main-loop)
    participant PER as Peripheral (GPIO)
    participant EXT as External Device (S800)

    loop every super-loop iteration [TIMING UNKNOWN — no fixed period, 03_execution_model.md §2.3]
        APP->>PER: sb_reset_proc()
        APP->>PER: moni_24v_proc()
        APP->>APP: BSP_WDT_Refresh()
        APP->>PER: msw_on_proc(STATE_NORMAL)
        APP->>PER: power_monitor_proc(STATE_NORMAL)
        APP->>APP: sleep_status_rem_proc(model)
        Note right of APP: reads get_ca72_status() — km_extend_io.c:896,971<br/>⚠ this read happens BEFORE ap_power_en_proc<br/>writes it below, in the SAME iteration
        APP->>PER: mc_p_on_proc(model)
        APP->>EXT: ir_p_on_proc()
        APP->>PER: mc3_3von_moni_proc()
        APP->>PER: erp_sensor_proc()
        APP->>APP: mc_oe_proc()
        Note right of APP: km_extend_io.c:1072 — no-op stub
        APP->>PER: mc_pwr_en_proc()
        APP->>APP: adc_moni_5v_off_proc()
        Note right of APP: km_adc.c:54 — [DEAD, #if 0 body]
        APP->>PER: rst_usb_proc(STATE_NORMAL)
        APP->>PER: usb2_oe_proc()
        APP->>EXT: ap_power_en_proc(STATE_NORMAL)
        Note right of APP: WRITES _status via set_ca72_status()<br/>km_extend_io.c:665,670 — this is the write<br/>sleep_status_rem_proc above just read the OLD value of
        APP->>APP: intr_pending_proc()
        opt HRESET_REQ or AP_PWR_EN pending bit set
            APP->>APP: hreset_req_proc() / ap_power_en_proc(STATE_INT)
        end
        APP->>APP: anti_chattering_proc()
        opt a debounce timer just expired
            APP->>APP: info->intr_proc(STATE_INT)
            Note right of APP: indirect call — 04_call_graph.md §3.7
        end
        APP->>APP: poweroff_timer_proc()
        APP->>APP: backupwait_timer_proc()
        APP->>EXT: i2c_recv_wait()
        Note right of APP: → 05_communication_rx.md / 06_communication_tx.md
        APP->>APP: sleep_mode()
        opt MSW on, SB_PG on, AP_PWR_EN off, no pending factor
            APP->>PER: BSP_PWR_enter_sleepmode() [WFI]
        end
    end
```

## Traceability

| Message | Function | File:Line |
|---|---|---|
| sleep_status_rem_proc | `sleep_status_rem_proc` | `main/App/km_extend_io.c:1055` |
| ap_power_en_proc | `ap_power_en_proc` | `main/App/km_extend_io.c:658` |
| intr_pending_proc | `intr_pending_proc` | `main/App/km_extend_io.c:1416` |
| anti_chattering_proc | `anti_chattering_proc` | `main/App/km_extend_io.c:1438` |
| i2c_recv_wait | `i2c_recv_wait` | `main/App/km_i2c.c:485` |
| sleep_mode | `sleep_mode` | `main/App/km_extend_io.c:1395` |

`Note` trên `sleep_status_rem_proc`/`ap_power_en_proc` ghi lại phát hiện về
độ trễ-một-vòng-lặp (one-iteration staleness) đã được xác định lần đầu
trong `04_call_graph.md` §2.2 và `05_data_flow.md` §5.1 — được đưa vào đây
vì sequence diagram chính là loại artifact có thể làm cho một hiểm hoạ về
thứ tự message trong cùng một vòng lặp trở nên hiển hiện ngay lập tức.
