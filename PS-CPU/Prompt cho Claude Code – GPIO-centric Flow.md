# GPIO-Centric Hardware Flow Analysis

Bạn đang phân tích source code firmware MCU.

Mục tiêu: từ toàn bộ source code, hãy xác định **vai trò thực tế của từng GPIO** và vẽ một sơ đồ flow giúp kỹ sư hiểu:

**GPIO nào → điều khiển/nhận tín hiệu gì → được sử dụng ở đâu → điều kiện nào làm GPIO thay đổi → thay đổi GPIO gây ra tác động gì.**

## 1. Thu thập toàn bộ GPIO

Quét toàn bộ source code và tìm:

- GPIO port/pin definitions
- `GPIO_InitTypeDef`
- `HAL_GPIO_Init()`
- `HAL_GPIO_WritePin()`
- `HAL_GPIO_ReadPin()`
- `BSP_GPIO_WritePin()`
- `BSP_GPIO_ReadPin()`
- GPIO interrupt / EXTI
- `HAL_GPIO_EXTI_Callback()`
- interrupt handler
- macro liên quan đến GPIO
- các function wrapper thao tác GPIO
- timer/state-machine function có tác động đến GPIO

Không chỉ tìm tên GPIO. Hãy trace **call chain** để xác định GPIO thực sự được dùng để làm gì.

---

# 2. Tạo GPIO Inventory

Tạo bảng:

| GPIO | Direction | Initial State | Active Level | Function | Controlled By | Affects |
|---|---|---|---|---|---|---|
| SB_PWR_EN | OUT | LOW | HIGH | S800 power enable | power-on sequence | S800 power rail |
| SB_RESET | OUT | LOW/HIGH | ... | S800 reset | sb_reset_proc() | S800 boot |
| AP_PWR_EN | IN | ... | HIGH | S800 power status | CA72/Linux | MC_P_ON / IR_P_ON |
| ... | ... | ... | ... | ... | ... | ... |

Với mỗi GPIO phải xác định rõ:

### Direction
- INPUT
- OUTPUT
- EXTI
- Alternate Function

### Active level
Phân biệt:

- Active HIGH
- Active LOW
- Open drain
- Pull-up
- Pull-down

Nếu tên có `_N`, `_L`, `RESET_N`, `PWR_EN_N`... hãy kiểm tra source để xác nhận, **không được suy đoán chỉ từ tên**.

---

# 3. Trace từng GPIO

Với mỗi GPIO, trace theo dạng:

```text
GPIO
 │
 ├── Definition
 │
 ├── GPIO initialization
 │
 ├── Initial value
 │
 ├── Read/Write locations
 │
 ├── Calling function
 │
 ├── State-machine relationship
 │
 ├── Timer relationship
 │
 ├── Interrupt relationship
 │
 └── Hardware effect
```

Ví dụ:

```text
SB_PWR_EN
    │
    ▼
BSP_GPIO_WritePin()
    │
    ▼
SB_PWR_EN = HIGH
    │
    ▼
S800 power rail ON
    │
    ▼
Wait 100 ms
    │
    ▼
sb_reset_proc()
    │
    ▼
Release S800 RESET
```

---

# 4. Vẽ GPIO Architecture Flow

Không vẽ một flowchart đơn giản chỉ liệt kê GPIO.

Hãy vẽ theo kiến trúc:

```text
                         MCU
                          │
             ┌────────────┼────────────┐
             │            │            │
             ▼            ▼            ▼
        POWER GPIO     STATUS GPIO   INTERRUPT GPIO
             │            │            │
             ▼            ▼            ▼
        S800 Power     CA72 Status    Power Switch
             │            │            │
             ▼            ▼            ▼
        RESET / PWR     State Machine  EXTI
```

Sau đó trace tiếp tới hardware.

---

# 5. Phân loại GPIO

Dùng màu khác nhau cho từng nhóm:

🟢 Power control  
🔴 Reset control  
🔵 Status / feedback  
🟠 Interrupt / EXTI  
🟣 Communication  
⚫ Debug / unused / unknown

Ví dụ:

```text
                 ┌──────────────────────┐
                 │        MCU           │
                 │   State Machine      │
                 └──────────┬───────────┘
                            │
       ┌────────────────────┼─────────────────────┐
       │                    │                     │
       ▼                    ▼                     ▼
 🟢 POWER              🔴 RESET              🔵 STATUS
       │                    │                     │
       ▼                    ▼                     ▼
 SB_PWR_EN            SB_RESET_N             AP_PWR_EN
       │                    │                     │
       ▼                    ▼                     ▼
 S800 Power Rail       S800 Reset          CA72 / Linux
       │                    │                     │
       └───────────────┬────┴─────────────────────┘
                       ▼
                 S800 Boot Complete
```

---

# 6. Quan trọng: thể hiện INPUT và OUTPUT

Trên diagram phải thể hiện rõ hướng tín hiệu.

Ví dụ:

```text
MCU ──────► SB_PWR_EN ──────► Power Hardware
MCU ──────► SB_RESET_N ─────► S800
CA72 ─────► AP_PWR_EN ──────► MCU
Power HW ─► POWER_SW ───────► MCU
```

Dùng:

`──────►`

cho MCU OUTPUT.

Dùng:

`◄──────`

cho MCU INPUT.

---

# 7. Trace theo thời gian

Nếu GPIO liên quan đến power sequence, tạo thêm timeline:

```text
t=0
 │
 │ SB_PWR_EN = HIGH
 ▼
S800 Power ON
 │
 │ 100 ms
 ▼
SB_RESET_N = RELEASE
 │
 ▼
S800 boot
 │
 │
 ▼
AP_PWR_EN = HIGH
 │
 ▼
MC_P_ON = HIGH
 │
 ▼
IR_P_ON = HIGH
```

Nếu source có timer cụ thể, ghi:

```text
100 ms
TYPE_Km_Timer_SB_RESET_RELEASE
```

Không được tự tạo timing nếu source không chứng minh được.

---

# 8. Trace theo State Machine

Nếu GPIO được điều khiển bởi state machine, phải nối:

```text
State
  │
  ▼
Condition
  │
  ▼
Function
  │
  ▼
GPIO change
  │
  ▼
Hardware effect
  │
  ▼
New state
```

Ví dụ:

```text
S800_OFF
    │
    │ SB_PWR_EN == ON
    ▼
POWERING_ON
    │
    │ reset timer >= 100 ms
    ▼
RELEASE_RESET
    │
    │ AP_PWR_EN == ON
    ▼
S800_RUNNING
```

---

# 9. GPIO → Function → Hardware Matrix

Sau diagram, tạo bảng mapping:

| GPIO | Source Function | Trigger | GPIO Action | Hardware Meaning |
|---|---|---|---|---|
| SB_PWR_EN | power_on_proc() | MCU boot | HIGH | Enable S800 power |
| SB_RESET_N | sb_reset_proc() | Timer/condition | HIGH | Release S800 reset |
| AP_PWR_EN | mc_p_on_proc() | CA72 status | READ | Detect AP power |
| MC_P_ON | mc_p_on_proc() | AP_PWR_EN | HIGH | Enable MC power |
| IR_P_ON | mc_p_on_proc() | AP_PWR_EN | HIGH | Enable IR power |

Chỉ điền những gì có bằng chứng từ source.

---

# 10. Source Traceability

Mỗi GPIO trên diagram phải có source reference.

Ví dụ:

```text
SB_PWR_EN
│
├── Definition:
│   board.h:123
│
├── Init:
│   gpio.c:87
│
├── Write:
│   power.c:214
│
├── Control:
│   sb_power_proc():210-230
│
└── Trigger:
    STATE_POWER_ON
```

Nếu có thể, ghi:

```text
file.c:line
```

ngay bên cạnh node hoặc trong bảng.

---

# 11. Phát hiện GPIO bất thường

Đặc biệt tìm:

- GPIO được initialize nhưng không bao giờ đọc/ghi
- GPIO được write ở nhiều function
- GPIO có nhiều owner
- GPIO bị write HIGH rồi LOW ở nhiều state
- GPIO input nhưng không có nơi sử dụng
- GPIO EXTI nhưng callback không tồn tại
- GPIO có active level không rõ
- GPIO có comment khác với behavior thực tế
- GPIO được sử dụng gián tiếp thông qua macro/function wrapper

Đánh dấu:

⚠️ UNKNOWN  
⚠️ MULTIPLE OWNER  
⚠️ DEAD GPIO  
⚠️ CONFLICT  
⚠️ SOURCE/COMMENT MISMATCH

---

# 12. Output cuối cùng

Tạo **3 diagram**, không chỉ một:

### Diagram 1 — GPIO Overview

```text
MCU
 │
 ├── Power Control GPIO
 ├── Reset GPIO
 ├── Status GPIO
 ├── Interrupt GPIO
 └── Communication GPIO
```

### Diagram 2 — GPIO Hardware Relationship

```text
MCU GPIO
   │
   ├──► Power Hardware
   ├──► S800
   ├──► CA72
   ├──► Sensors
   └──► External Controller
```

### Diagram 3 — GPIO Runtime / State Machine

```text
Boot
 ↓
Init GPIO
 ↓
Power ON
 ↓
Wait
 ↓
Release RESET
 ↓
Check STATUS
 ↓
Enable downstream power
 ↓
Runtime monitoring
 ↓
Sleep
 ↓
Wake / EXTI
 ↓
Repeat
```

---

# 13. Diagram style

Ưu tiên **Mermaid** để có thể review và chỉnh sửa.

Dùng:

- `flowchart LR` cho architecture
- `flowchart TD` cho sequence
- `stateDiagram-v2` cho state machine
- `sequenceDiagram` khi cần thể hiện MCU ↔ S800 ↔ Host

Không nhồi tất cả vào một diagram.

Nếu diagram quá lớn, chia thành:

```text
01_GPIO_OVERVIEW.md
02_GPIO_POWER_FLOW.md
03_GPIO_RESET_FLOW.md
04_GPIO_STATUS_FLOW.md
05_GPIO_INTERRUPT_FLOW.md
06_GPIO_STATE_MACHINE.md
07_GPIO_TIMING.md
```

---

# 14. Quy tắc quan trọng

**Không được suy đoán hardware behavior.**

Phân biệt rõ:

```text
[CONFIRMED]
Có bằng chứng trực tiếp từ source.

[INFERRED]
Có thể suy ra từ call chain / state machine.

[UNKNOWN]
Source không đủ thông tin để kết luận.
```

Mục tiêu không phải chỉ tạo diagram đẹp.

Mục tiêu là để một kỹ sư mới nhìn vào diagram có thể trả lời:

> "Chân GPIO này dùng để làm gì?"

> "Ai điều khiển nó?"

> "Khi nào nó đổi trạng thái?"

> "Đổi trạng thái thì phần cứng nào bị ảnh hưởng?"

> "Điều kiện nào khiến nó đổi?"

> "Function nào thực hiện việc đó?"

> "Source code nằm ở đâu?"