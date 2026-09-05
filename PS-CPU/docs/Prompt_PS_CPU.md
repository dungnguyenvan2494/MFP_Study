Được. Với source MCU thật, mình khuyên bạn đừng dùng một prompt duy nhất. Hãy biến Claude thành một “firmware reverse-engineering analyst”, chạy theo từng phase và mỗi phase tạo ra artifact để phase sau dùng lại.

Mục tiêu cuối:

MCU SOURCE CODE
      │
      ▼
[01] Repository Map
      │
      ▼
[02] Architecture
      │
      ├── Hardware
      ├── BSP/HAL
      ├── Driver
      ├── RTOS
      ├── Middleware
      └── Application
      │
      ▼
[03] Execution Model
      │
      ├── main()
      ├── Task
      ├── ISR
      ├── Callback
      ├── Timer
      └── DMA
      │
      ▼
[04] Call/Data/Event Analysis
      │
      ▼
[05] State Machine
      │
      ▼
[06] Use Case
      │
      ▼
[07] Activity
      │
      ▼
[08] Sequence
      │
      ▼
[09] Timing + Concurrency
      │
      ▼
[10] Validation
      │
      ▼
FINAL MCU DESIGN DOCUMENTATION
1. Chuẩn bị project

Trong repository MCU, tạo:

docs/
├── 00_project_scope.md
├── 01_repository_map.md
├── 02_architecture.md
├── 03_execution_model.md
├── 04_call_graph.md
├── 05_data_flow.md
├── 06_event_flow.md
├── 07_state_machines.md
├── 08_concurrency.md
├── 09_timing.md
├── 10_error_recovery.md
│
├── diagrams/
│   ├── 01_system_context.md
│   ├── 02_architecture.md
│   ├── 03_use_cases.md
│   ├── 04_activity/
│   ├── 05_sequence/
│   └── 06_state_machine/
│
└── traceability/
    └── code_traceability.md

Không cho Claude vẽ ngay.

Đây là nguyên tắc quan trọng nhất.

2. Phase 0 — Xác định phạm vi

Chạy prompt này trước.

You are a Senior Embedded Firmware Architect and MCU Reverse Engineer.

We are going to reverse-engineer this MCU firmware repository.

IMPORTANT RULE:

Do NOT generate diagrams yet.

First understand the repository and establish the analysis scope.

==================================================
OBJECTIVE
==================================================

The final goal is to generate source-code-grounded:

1. System Context Diagram
2. Firmware Architecture Diagram
3. Use Case Diagram
4. Activity Diagrams
5. Sequence Diagrams
6. State Machine Diagrams
7. Data Flow Diagrams
8. Concurrency Diagrams
9. Timing Diagrams
10. Code Traceability Matrix

All diagrams MUST be traceable to actual source code.

==================================================
SOURCE CODE RULE
==================================================

Use the actual repository as the primary source of truth.

DO NOT infer behavior merely from:

- function names
- filenames
- comments
- variable names
- expected MCU architecture

If behavior cannot be proven from code:

[UNKNOWN]

If behavior is inferred:

[INFERRED]

If behavior depends on hardware documentation:

[HARDWARE ASSUMPTION]

Never silently invent behavior.

==================================================
FIRST TASK
==================================================

Analyze:

- repository structure
- build system
- MCU/SoC
- compiler/toolchain
- linker script
- startup code
- BSP
- HAL
- drivers
- middleware
- RTOS
- application
- configuration
- generated code

Identify:

- major modules
- dependencies
- entry points
- important APIs
- hardware interfaces
- execution contexts

==================================================
OUTPUT
==================================================

Create:

docs/00_project_scope.md

Include:

1. MCU/platform
2. Firmware purpose
3. Build system
4. RTOS
5. Major modules
6. Hardware peripherals
7. External actors/devices
8. Main execution entry points
9. Known uncertainties
10. Recommended next analysis steps

DO NOT GENERATE DIAGRAMS.
3. Phase 1 — Repository Map

Đây là phase Sonnet + High rất hợp.

Analyze the entire firmware repository.

Create a detailed repository map.

For every major directory/module identify:

- purpose
- responsibility
- public APIs
- internal APIs
- important source files
- important header files
- dependencies
- callers
- callees
- hardware dependencies
- RTOS dependencies
- configuration dependencies

Classify each module as one of:

- Startup
- BSP
- HAL
- Driver
- RTOS
- Middleware
- Protocol
- Service
- Application
- Utility
- Configuration
- Generated code

For every important file identify:

File
→ Module
→ Responsibility
→ Important functions
→ Important data structures
→ Dependencies

Find:

- main()
- reset handler
- startup functions
- RTOS initialization
- task creation
- interrupt handlers
- timer callbacks
- DMA callbacks
- peripheral initialization
- application entry points

Create:

docs/01_repository_map.md

At the end provide:

MODULE DEPENDENCY MATRIX

Module | Depends On | Used By | Context
4. Phase 2 — Architecture

Đây là lúc Claude bắt đầu hiểu kiến trúc.

Using docs/01_repository_map.md and the actual source code,
reverse-engineer the firmware architecture.

Do not generate a generic MCU architecture.

Generate an architecture based ONLY on actual code.

Analyze these layers:

1. Hardware
2. MCU Peripheral
3. ISR
4. BSP
5. HAL
6. Driver
7. RTOS
8. Middleware
9. Protocol
10. Service
11. Application

Determine actual relationships.

For every dependency answer:

WHO calls WHO?
WHO owns the resource?
WHO initializes it?
WHO services it?
WHO consumes its data?

Identify:

- hardware abstraction boundaries
- driver boundaries
- application boundaries
- middleware boundaries
- RTOS boundaries

Also identify architectural violations or unusual patterns.

Create:

docs/02_architecture.md

Include:

1. Layer model
2. Module dependency graph
3. Responsibility matrix
4. Initialization ownership
5. Runtime ownership
6. Hardware ownership
7. Architectural observations

Every major statement must contain:

file
function
line number when available.
5. Phase 3 — Execution Model

Đây là phase rất quan trọng với MCU.

Reverse-engineer the firmware execution model.

Identify every execution context.

==================================================
EXECUTION CONTEXTS
==================================================

1. Reset context
2. Startup context
3. main()
4. RTOS task/thread
5. ISR
6. DMA interrupt
7. Timer callback
8. Peripheral callback
9. Deferred work
10. Event handler
11. Polling loop

==================================================
FOR EACH CONTEXT
==================================================

Identify:

- entry function
- trigger
- priority
- frequency
- stack
- blocking behavior
- called functions
- shared resources
- synchronization primitives

==================================================
INTERRUPT ANALYSIS
==================================================

For every ISR:

Peripheral
→ Interrupt source
→ Vector
→ ISR
→ Register handling
→ Driver
→ Callback/event
→ RTOS synchronization
→ Task

Determine whether ISR:

- accesses registers
- clears interrupt flags
- reads/writes buffers
- gives semaphore
- sends queue
- sets event
- wakes task
- schedules deferred work

==================================================
RTOS ANALYSIS
==================================================

Identify:

- tasks
- priorities
- periods
- queues
- semaphores
- mutexes
- event groups
- timers
- notifications
- critical sections

Create:

docs/03_execution_model.md
6. Phase 4 — Call Graph

Đừng chỉ dùng call graph của tool. Bắt Claude giải thích ý nghĩa của call graph.

Build a source-code-grounded call graph for the firmware.

Focus on important execution paths rather than every trivial utility function.

For each major path:

ENTRY
→ function
→ function
→ function
→ return

Identify:

- direct calls
- indirect calls
- callbacks
- function pointers
- interrupt entry
- RTOS callbacks
- driver callbacks

For every important function record:

Function
File
Caller
Callee
Execution Context
Purpose
Input
Output
Side Effects
Shared State

Pay special attention to:

- function pointers
- callback registration
- weak functions
- virtual dispatch if C++
- macro-generated functions
- interrupt vector tables

Create:

docs/04_call_graph.md
7. Phase 5 — Data Flow

Cái này rất quan trọng khi sau này vẽ sequence.

Reverse-engineer the firmware data flow.

Identify important:

- global variables
- static variables
- structures
- queues
- ring buffers
- DMA buffers
- protocol packets
- configuration objects
- state variables
- register-backed data
- shared memory

For every important data object determine:

NAME
TYPE
OWNER
CREATOR
WRITER
READER
LIFETIME
EXECUTION CONTEXT
SYNCHRONIZATION
PURPOSE

Trace important data:

Hardware
→ Register
→ ISR
→ Driver
→ Buffer
→ Queue
→ Task
→ Application

Also trace reverse direction:

Application
→ Driver
→ Peripheral
→ Hardware

Identify possible:

- race conditions
- stale data
- buffer ownership issues
- lifetime problems
- ISR/task sharing

Create:

docs/05_data_flow.md
8. Phase 6 — Event Flow

Đây là bước biến source code thành behavior model.

Reverse-engineer all events entering and propagating through
the firmware.

Identify events such as:

- power-on
- reset
- GPIO interrupt
- UART RX
- UART TX complete
- SPI complete
- I2C event
- CAN RX
- timer expiration
- DMA complete
- ADC complete
- USB event
- external interrupt
- watchdog
- communication timeout
- protocol command
- error

For every event trace:

EVENT
→ hardware
→ interrupt/callback
→ driver
→ synchronization primitive
→ task
→ application
→ state change
→ response

For every event identify:

Trigger
Handler
Execution Context
Data
State
Action
Next Event

Create:

docs/06_event_flow.md
9. Phase 7 — State Machine

Bây giờ mới tạo state.

Find every explicit and implicit state machine.

Search for:

- enum state
- switch(state)
- status variables
- mode variables
- flags
- state transitions
- event-driven logic
- retry counters
- timeout states
- error states

For every state machine identify:

STATE
TRIGGER
CONDITION
ACTION
NEXT STATE
FUNCTION
FILE

Also identify:

- entry action
- exit action
- timeout
- retry
- error
- recovery

Distinguish:

EXPLICIT STATE

from

IMPLICIT STATE

Do not invent states.

Create:

docs/07_state_machines.md
10. Phase 8 — Concurrency

Đối với MCU + RTOS, phase này cực kỳ đáng tiền.

Perform a complete concurrency analysis.

Identify all shared resources between:

- ISR
- task
- timer callback
- DMA callback
- main loop
- driver
- middleware

For each shared resource:

RESOURCE
WRITER
READER
CONTEXT
SYNCHRONIZATION
CRITICAL SECTION
RISK

Analyze:

- volatile
- atomic access
- interrupt disable
- mutex
- semaphore
- queue
- event
- critical section
- memory barriers

Identify potential:

- race conditions
- priority inversion
- deadlock
- lost event
- double access
- buffer corruption
- ISR/task synchronization problems

Create:

docs/08_concurrency.md
11. Phase 9 — Timing
Reverse-engineer firmware timing behavior.

Identify actual timing sources:

- hardware timers
- SysTick
- RTOS tick
- vTaskDelay
- software timers
- timeout counters
- polling intervals
- debounce
- watchdog
- communication timeout
- retry interval
- DMA completion

For every timing behavior record:

Event
Start
Duration
Timeout
Next Action
Context

Distinguish:

HARDWARE TIMING
RTOS TIMING
SOFTWARE TIMING
UNKNOWN TIMING

Never invent timing values.

Create:

docs/09_timing.md
12. Phase 10 — Error & Recovery
Reverse-engineer all error paths.

Find:

- return codes
- error enums
- timeout
- retry
- reset
- watchdog
- fault handlers
- assertion
- recovery functions
- fallback states
- communication errors
- peripheral errors

For every error:

NORMAL OPERATION
→ ERROR
→ DETECTION
→ HANDLING
→ RETRY/RESET/RECOVERY
→ NEXT STATE

Identify exact functions and files.

Create:

docs/10_error_recovery.md
13. Bây giờ mới vẽ Use Case
Using ALL analysis documents, generate the firmware Use Case model.

Use cases represent USER/EXTERNAL behavior and firmware capabilities.

Do NOT represent individual C functions as use cases.

Identify actual actors:

- User
- Host MCU
- External MCU
- Sensor
- Actuator
- PC
- Communication master
- Hardware event
- Timer
- DMA
- Peripheral

For every use case:

ID
Name
Actor
Trigger
Precondition
Main Flow
Alternative Flow
Error Flow
Postcondition
Firmware Modules
Functions
Files

Generate:

1. High-level Use Case Diagram
2. Detailed Use Case specifications
3. Use Case → Module → Function traceability

Use Mermaid or PlantUML.

Save:

docs/diagrams/03_use_cases.md
14. Activity Diagram

Đây là prompt mình rất khuyên dùng:

Generate detailed Activity Diagrams for the firmware.

DO NOT create one giant activity diagram.

Create one diagram per major behavior.

Examples:

- Boot
- Hardware initialization
- RTOS initialization
- Main application
- Communication RX
- Communication TX
- Sensor acquisition
- DMA processing
- Error handling
- Recovery
- Shutdown

Use swimlanes:

Hardware
ISR
Driver
RTOS
Application

Show:

START
→ action
→ event
→ decision
→ condition
→ wait
→ queue
→ semaphore
→ callback
→ timeout
→ retry
→ error
→ recovery
→ END

Every important activity node must include:

[Context]
Module::Function()

Example:

[ISR]
UART_IRQHandler()

[RTOS Task]
UART_Task()

Do not replace real functions with vague descriptions.

Add source traceability:

Function
File
Line

Save diagrams under:

docs/diagrams/04_activity/
15. Sequence Diagram — quan trọng nhất

Mình sẽ bắt Claude tạo nhiều sequence, không phải một cái.

Generate source-code-grounded Sequence Diagrams.

Create separate diagrams for each important scenario.

Minimum scenarios:

1. Boot
2. Hardware initialization
3. RTOS startup
4. Normal application flow
5. Communication RX
6. Communication TX
7. Interrupt processing
8. DMA processing
9. Timer event
10. Error handling
11. Timeout/retry
12. Recovery
13. Shutdown

==================================================
PARTICIPANTS
==================================================

Show actual architectural participants:

Hardware
Peripheral
NVIC/Interrupt
ISR
Driver
HAL
RTOS
Task
Middleware
Application
External Device

==================================================
EVERY MESSAGE
==================================================

Show:

caller
callee
function
data
return
execution context

Example:

Hardware
→ UART_ISR

UART_ISR()
→ UART_Driver::Receive()

UART_Driver
→ xQueueSendFromISR()

RTOS
→ UART_Task

UART_Task
→ Protocol::Process()

Protocol
→ Application

==================================================
CONCURRENCY
==================================================

Use:

par

for concurrent execution.

Use:

alt

for conditions.

Use:

loop

for repeated behavior.

Use:

opt

for optional paths.

==================================================
TIMING
==================================================

Show timing ONLY when supported by source code.

Example:

t=0
t=+1ms
t=+10ms

Otherwise:

[TIMING UNKNOWN]

==================================================
TRACEABILITY
==================================================

Every function call must be traceable to:

file
function
line

Never invent function calls.

Save under:

docs/diagrams/05_sequence/
16. Sau đó làm State Diagram
Generate Mermaid stateDiagram-v2 diagrams.

For each state machine show:

- states
- events
- guards
- transitions
- entry actions
- exit actions
- timeout
- retry
- error
- recovery

Every transition must map to actual source code.

Format:

STATE_A --> STATE_B : EVENT [CONDITION]

Add references:

Function
File
Line

Separate:

- system state machine
- communication state machine
- peripheral state machine
- application state machine

Save:

docs/diagrams/06_state_machine/
17. Cuối cùng: Traceability

Đây là bước mình rất khuyến khích, vì nó biến diagram từ "AI vẽ cho đẹp" thành tài liệu kỹ thuật có thể review.

Perform a complete traceability audit.

For every diagram element identify the source-code evidence.

Build:

DIAGRAM ELEMENT
→ BEHAVIOR
→ FUNCTION
→ FILE
→ LINE
→ EXECUTION CONTEXT

For example:

Sequence:
UART_ISR()
    ↓
xQueueSendFromISR()
    ↓
UART_Task()

Trace:

UART_ISR()
→ uart.c:123

xQueueSendFromISR()
→ freertos_port.c:456

UART_Task()
→ uart_task.c:78

Check every:

- participant
- message
- function
- state
- transition
- activity
- event

If no source evidence exists:

[UNVERIFIED]

If inferred:

[INFERRED]

If hardware-dependent:

[HARDWARE ASSUMPTION]

Do not silently accept unsupported diagram elements.

Create:

docs/traceability/code_traceability.md
18. Và đây là prompt "Audit" cuối cùng

Cho Opus + effort cao nhất.

You are performing a final forensic audit of the firmware
documentation.

The diagrams must be treated as engineering artifacts.

Compare:

SOURCE CODE
vs
ANALYSIS DOCUMENTS
vs
DIAGRAMS

Find discrepancies.

Check:

1. Missing function calls
2. Incorrect function calls
3. Incorrect execution context
4. Incorrect ISR behavior
5. Incorrect RTOS behavior
6. Missing synchronization
7. Incorrect state transition
8. Missing error path
9. Incorrect timing
10. Incorrect hardware interaction
11. Hallucinated components
12. Hallucinated dependencies

For every discrepancy provide:

SEVERITY
DIAGRAM
ELEMENT
EXPECTED
ACTUAL
SOURCE EVIDENCE
FIX

Severity:

CRITICAL
HIGH
MEDIUM
LOW

Then update the affected documentation and diagrams.

IMPORTANT:

Prefer correctness over visual simplicity.

Never remove technical details merely to make
the diagram prettier.
19. Model/effort cho từng phase

Nếu Claude Code của bạn có lựa chọn model/effort tương ứng, mình sẽ chạy như này:

Phase	Model	Effort
Repository scan	Sonnet	Medium
Module analysis	Sonnet	High
Architecture	Sonnet	High
Call graph	Sonnet	High
Data flow	Sonnet	High
Event flow	Sonnet	High
State machine	Opus	High
Concurrency	Opus	High
Timing	Opus	High
Error recovery	Opus	High
Use Case	Sonnet	High
Activity	Opus	High
Sequence	Opus	High
Traceability	Opus	High
Final audit	Opus	Max

Không cần Max mọi chỗ. Max nên để cho những phase mà sai một relationship là hỏng cả tài liệu.

20. Thứ tự chạy thực tế

Nếu là project của bạn, mình sẽ chạy đúng thứ tự:

DAY 1
│
├── 00 Scope
├── 01 Repository
├── 02 Architecture
└── 03 Execution Model

DAY 2
│
├── 04 Call Graph
├── 05 Data Flow
├── 06 Event Flow
└── 07 State Machine

DAY 3
│
├── 08 Concurrency
├── 09 Timing
└── 10 Error Recovery

DAY 4
│
├── Use Case
├── Activity
└── State Diagram

DAY 5
│
├── Boot Sequence
├── Communication Sequence
├── ISR Sequence
├── DMA Sequence
└── Error Sequence

DAY 6
│
├── Traceability
└── Final Audit

Không nhất thiết phải đúng 6 ngày; ý là đừng nhảy cóc.

21. Một nguyên tắc cực quan trọng

Đừng yêu cầu:

"Vẽ diagram thật chi tiết."

Hãy yêu cầu:

"Mỗi element của diagram phải có source evidence."

Ví dụ:

❌ Không tốt:

UART → Driver → Application

Tốt:

UART Hardware
    │
    │ RX interrupt
    ▼
[ISR]
USART1_IRQHandler()
    │
    │ xQueueSendFromISR()
    ▼
[RTOS]
UART_RX_QUEUE
    │
    │ xQueueReceive()
    ▼
[TASK]
uart_rx_task()
    │
    ▼
[Protocol]
protocol_process_frame()
    │
    ▼
[Application]
app_handle_command()

và bên cạnh phải truy được:

USART1_IRQHandler()
    uart.c:182

uart_rx_task()
    uart_task.c:91

protocol_process_frame()
    protocol.c:247

app_handle_command()
    app.c:133

Đây mới là diagram có giá trị khi bạn maintain firmware hoặc debug issue.

22. Nếu source MCU rất lớn

Ví dụ:

300+ .c files
100+ .h files
RTOS
HAL
Vendor SDK
Drivers
Middleware
Application

thì không nên cho Claude phân tích tất cả rồi một lần sinh diagram.

Chia subsystem:

MCU
│
├── System
│
├── Clock/Power
│
├── GPIO
│
├── UART
│
├── SPI
│
├── I2C
│
├── DMA
│
├── Timer
│
├── ADC
│
├── CAN
│
├── RTOS
│
├── Communication
│
└── Application

Mỗi subsystem:

Source
 ↓
Analysis
 ↓
Activity
 ↓
Sequence
 ↓
State
 ↓
Traceability

Sau đó mới ghép thành system-level diagram.

23. Và nếu bạn muốn biến thành 1 Skill dùng lâu dài

Mình thực sự khuyên làm vậy.

Cấu trúc:

.claude/
└── skills/
    └── mcu-reverse-engineering/
        ├── SKILL.md
        ├── rules/
        │   ├── source-grounding.md
        │   ├── isr-analysis.md
        │   ├── rtos-analysis.md
        │   ├── state-machine.md
        │   ├── concurrency.md
        │   └── timing.md
        │
        └── templates/
            ├── architecture.md
            ├── use-case.md
            ├── activity.md
            ├── sequence.md
            ├── state.md
            └── traceability.md

Rồi sau này chỉ cần:

Analyze this MCU repository
using the mcu-reverse-engineering skill.

Start from Phase 0.
Do not skip phases.
Do not generate diagrams until
the behavioral model is complete.

Claude sẽ có một quy trình reverse-engineering cố định, thay vì mỗi lần bạn phải viết prompt lại.

Nếu bạn muốn đi theo hướng này, bước tiếp theo hợp lý nhất là mình viết hẳn cho bạn file SKILL.md hoàn chỉnh — không chỉ prompt ở trên, mà có workflow tự động, rule phân biệt ISR/Task, rule Mermaid, source traceability, chống hallucination, cách xử lý project hàng nghìn file và checklist review.