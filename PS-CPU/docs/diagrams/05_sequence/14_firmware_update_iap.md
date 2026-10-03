# Sequence Diagram — Cập nhật firmware qua IAP (kịch bản bổ sung, ngoài danh sách tối thiểu)

Được đưa vào vì đây là một trong những năng lực bên ngoài (external
capability) quan trọng nhất của firmware (UC-07, `03_use_cases.md`) và
trải dài trên cả hai image — không có kịch bản đơn lẻ nào trong số 13 kịch
bản mẫu (template) nắm bắt được nó từ đầu đến cuối. Các participant:
**External Device** (I2C Master), **Application (Main)**,
**Application (IAP)**, **ISR (IAP)**, **Driver** (`Driver_Flash0`),
**Hardware**.

```mermaid
sequenceDiagram
    participant EXT as External Device (I2C Master)
    participant M as Application (Main)
    participant I as Application (IAP)
    participant ISR as ISR (IAP, EXTI4_15)
    participant DRV as Driver (Driver_Flash0)
    participant HW as Hardware

    EXT->>M: I2C write: IAP_PG_COMMAND = 0x11223344
    M->>M: DeInit()
    Note right of M: entry.c:186 — stop TIM3, uninit I2C0, disable RTC alarm
    M->>I: jump2iap()
    Note right of M: entry.c:159 — SOFTWARE JUMP, no core reset,<br/>04_call_graph.md §5
    I->>I: main() runs (NOT via Reset_Handler)
    I->>HW: copy vector table to SRAM 0x20000000 +<br/>__HAL_SYSCFG_REMAPMEMORY_SRAM()
    Note right of I: entry_iap.c:55-68
    I->>I: enter IAP super-loop

    EXT->>I: I2C write: IAP_PG_COMMAND = 0x11223344 (again)
    I->>DRV: main_project_flash_erase()
    Note right of I: km_i2c_iap.c:435 ⇢ Driver_Flash0.EraseSector (indirect)
    DRV->>HW: erase 0x08000000-0x08004FFF

    loop for each 4-byte word of new image
        EXT->>I: I2C write: IAP_PG_COMMAND_DATA
        I->>I: range check g_dest_addr
        alt in range
            I->>DRV: flash_write_32()
            Note right of I: km_i2c_iap.c:413 ⇢ Driver_Flash0.ProgramData (indirect)
            DRV->>HW: program 4 bytes
        else out of range
            I->>I: g_iap_reg[CHANGE_STATUS] |= PSCPU_ERR
            Note right of I: km_i2c_iap.c:373 — recorded, not enforced
        end
    end

    EXT->>I: I2C write: IAP_PG_COMMAND_CHANGE_INFO (COMP bit set)
    I->>I: check_sum_calc()
    Note right of I: km_i2c_iap.c:456
    alt checksum matches
        I->>I: checksum_calc() / GetVersionString() re-read new image
        Note right of I: km_i2c_iap.c:345-346
    else mismatch
        I->>I: g_iap_reg[CHANGE_STATUS] |= CHKSUM_ERR
        Note right of I: km_i2c_iap.c:339 — recorded, not enforced
    end

    EXT->>ISR: MSW_ON released (physical action)
    ISR->>ISR: msw_on_proc_iap()
    Note right of ISR: km_extend_io_iap.c:28
    ISR->>ISR: check ONLY CHANGE_INFO's COMP bit
    Note right of ISR: km_extend_io_iap.c:35 — CHKSUM_ERR/PSCPU_ERR<br/>NEVER consulted, 10_error_recovery.md §2.9-§2.10
    ISR->>HW: HAL_NVIC_SystemReset()
    Note right of ISR: TERMINAL
    HW-->>M: → 01_boot_sequence.md (Main image, now updated —<br/>regardless of verification outcome unless EXT checked first)
```

## Traceability

| Message | Function | File:Line |
|---|---|---|
| jump2iap | `jump2iap` | `main/App/entry.c:159` |
| DeInit | `DeInit` | `main/App/entry.c:186` |
| IAP main / vector relocation | `main` | `iap/App/entry_iap.c:39,55-68` |
| main_project_flash_erase | `main_project_flash_erase` | `iap/App/km_i2c_iap.c:435` |
| flash_write_32 | `flash_write_32` | `iap/App/km_i2c_iap.c:413` |
| check_sum_calc | `check_sum_calc` | `iap/App/km_i2c_iap.c:456` |
| msw_on_proc_iap | `msw_on_proc_iap` | `iap/App/km_extend_io_iap.c:28` |
