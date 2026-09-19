# U-BOOT REPOSITORY DEEP-DIVE — MASTER RESEARCH TASK

Tôi có một repository U-Boot thực tế, có thể là U-Boot upstream hoặc vendor-fork/BSP đã được tùy biến.

Mục tiêu của bạn:

Hãy reverse-engineer TOÀN BỘ U-Boot trong repository này và dạy lại cho tôi sao cho tôi có thể hiểu được cách hệ thống hoạt động mà KHÔNG CẦN tự đọc từng dòng source code.

Đây KHÔNG phải yêu cầu viết một summary chung về U-Boot.

Bạn PHẢI dựa trên SOURCE CODE THỰC TẾ trong repository này.

Nếu repository là vendor fork, phải ưu tiên behavior thực tế của repository thay vì kiến thức U-Boot upstream.

============================================================
# NGUYÊN TẮC QUAN TRỌNG
============================================================

1. KHÔNG được chỉ mô tả kiến thức U-Boot chung chung.

2. Mọi kết luận quan trọng phải trace về source code thực tế:
   - file
   - function
   - macro
   - struct
   - global variable
   - Kconfig
   - CONFIG_*
   - Makefile
   - linker script
   - device tree
   - board code
   - driver

3. Tôi KHÔNG muốn phải đọc code để hiểu.
   Vì vậy hãy đọc code thay tôi và giải thích bằng ngôn ngữ con người.

4. Không giải thích theo kiểu:
      "Hàm này dùng để..."
   rồi bỏ qua execution flow.

   Hãy giải thích:
      ai gọi nó
      nó gọi ai
      dữ liệu đi qua đâu
      state thay đổi thế nào
      hardware bị tác động ở đâu
      điều kiện nào khiến flow rẽ nhánh
      flow kết thúc ở đâu

5. Khi có function quan trọng, hãy tạo call flow:

   caller
      ↓
   function
      ↓
   sub-function
      ↓
   hardware / memory / peripheral
      ↓
   return

6. Phân biệt rõ:

   [UPSTREAM]
   behavior chuẩn của U-Boot

   [VENDOR]
   code tùy biến của vendor

   [BOARD]
   code riêng cho board

   [DRIVER]
   hardware driver

   [CONFIG]
   behavior được bật/tắt bởi configuration

7. Nếu không chắc, KHÔNG được đoán.
   Hãy tìm source/config/call-site để xác minh.

8. Nếu có nhiều implementation của cùng một function,
   phải xác định implementation nào thực sự được build/chạy dựa trên:
   - CONFIG
   - Makefile
   - Kconfig
   - architecture
   - board
   - device tree
   - linker
   - compiler flags

============================================================
# PHẦN 0 — IDENTIFY REPOSITORY
============================================================

Trước tiên hãy xác định:

- U-Boot version
- commit/version tag nếu có
- architecture
- CPU
- SoC
- board
- vendor
- boot medium
- memory map nếu xác định được
- DDR
- storage
- boot device
- serial console
- network
- secure boot nếu có
- SPL/TPL có tồn tại không
- FIT image có dùng không
- device tree có dùng không
- environment storage
- filesystem
- partition layout
- các custom subsystem

Tạo:

1. Repository profile
2. Hardware profile
3. Software architecture profile
4. Build configuration profile

============================================================
# PHẦN 1 — REPOSITORY MAP
============================================================

Hãy khảo sát toàn bộ repository.

Giải thích vai trò của:

arch/
board/
common/
drivers/
cmd/
env/
fs/
include/
lib/
net/
boot/
disk/
image/
lib/
scripts/
tools/
test/
configs/

và các thư mục vendor/custom khác.

Không chỉ liệt kê.

Phải giải thích:

"directory này nằm ở đâu trong boot process?"

Ví dụ:

Boot ROM
   ↓
TPL
   ↓
SPL
   ↓
U-Boot proper
   ↓
board_init_f()
   ↓
board_init_r()
   ↓
main_loop()
   ↓
boot command
   ↓
kernel

Nếu repository có flow khác thì phải dùng flow thực tế của repository.

============================================================
# PHẦN 2 — COMPLETE BOOT FLOW
============================================================

Trace từ thời điểm CPU reset cho tới khi Linux/OS được boot.

Phải trace bằng source code thực tế.

Tạo flow:

Reset
 ↓
Boot ROM
 ↓
TPL (nếu có)
 ↓
SPL (nếu có)
 ↓
DDR init
 ↓
relocation
 ↓
U-Boot proper
 ↓
early init
 ↓
board init
 ↓
driver init
 ↓
environment
 ↓
console
 ↓
main loop
 ↓
bootcmd
 ↓
load kernel
 ↓
load DTB
 ↓
load initrd
 ↓
bootm/booti/bootz
 ↓
handoff to kernel

Đối với mỗi bước:

- function entry
- caller
- important sub-functions
- important global state
- memory location
- hardware interaction
- failure condition

============================================================
# PHẦN 3 — RESET / STARTUP / LOW LEVEL
============================================================

Trace:

- reset vector
- start.S
- assembly startup
- CPU mode
- exception vector
- stack setup
- early C entry
- relocation
- BSS
- global data
- malloc area
- stack
- text location
- RAM location

Giải thích:

gd
bd
global_data
board_info
relocation_offset
relocaddr

và các structure quan trọng liên quan.

============================================================
# PHẦN 4 — TPL / SPL
============================================================

Nếu có:

phân tích toàn bộ SPL/TPL architecture.

Giải thích:

- tại sao tồn tại
- memory limitation
- stack
- SRAM
- DDR initialization
- clock
- pinmux
- PMIC
- storage
- loading U-Boot proper
- image format
- handoff

Trace code thực tế.

Tạo diagram:

Boot ROM
 ↓
TPL
 ↓
SPL
 ↓
DDR
 ↓
load U-Boot
 ↓
U-Boot proper

============================================================
# PHẦN 5 — BOARD INITIALIZATION
============================================================

Trace toàn bộ board initialization.

Tìm và giải thích:

board_init_f()
board_init_r()
dram_init()
dram_init_banksize()
board_early_init_f()
board_late_init()
misc_init_r()

và các board-specific function khác.

Cho biết:

- board nào
- SoC nào
- peripheral nào
- thứ tự init
- dependency giữa các init

============================================================
# PHẦN 6 — DRIVER MODEL
============================================================

Đây là phần cực kỳ quan trọng.

Giải thích U-Boot Driver Model:

UCLASS
udevice
driver
uclass_driver
ofnode
device tree
bind
probe
remove
sequence

Trace một driver thực tế trong repository từ:

Device Tree
 ↓
driver binding
 ↓
uclass
 ↓
device
 ↓
probe()
 ↓
hardware register
 ↓
operation

Chọn các driver quan trọng nhất của board và trace thực tế.

============================================================
# PHẦN 7 — DEVICE TREE
============================================================

Phân tích:

*.dts
*.dtsi
CONFIG_OF_*
device tree parser
ofnode
fdt
phandle
compatible
reg
clocks
resets
interrupts
status

Giải thích:

DT node
   ↓
compatible
   ↓
driver matching
   ↓
device object
   ↓
probe()
   ↓
hardware

Trace ít nhất 3 device thực tế của board.

============================================================
# PHẦN 8 — MEMORY
============================================================

Phân tích memory architecture:

ROM
SRAM
DDR
stack
heap
malloc
gd
bd
relocated U-Boot
device tree
kernel
initrd
environment
reserved memory

Tạo memory map.

Giải thích relocation thật kỹ:

link address
 ↓
runtime address
 ↓
relocation offset
 ↓
relocated code
 ↓
fixups

============================================================
# PHẦN 9 — CONSOLE / UART
============================================================

Trace UART từ:

console command
 ↓
serial subsystem
 ↓
uclass
 ↓
UART driver
 ↓
register
 ↓
TX/RX

Giải thích:

printf()
puts()
putc()
serial_putc()
driver callback
hardware register

Xác định UART thực tế của board.

============================================================
# PHẦN 10 — CLOCK / RESET / PINCTRL / GPIO
============================================================

Phân tích từng subsystem:

Clock
Reset
Pinctrl
GPIO

Trace:

consumer
 ↓
uclass
 ↓
provider
 ↓
SoC driver
 ↓
register
 ↓
hardware

Cho ví dụ thực tế từ board.

============================================================
# PHẦN 11 — DDR / DRAM
============================================================

Trace DDR initialization.

Giải thích:

- DDR controller
- PHY
- timing
- training
- size detection
- memory banks
- memory test

Nếu DDR initialization nằm ngoài U-Boot thì chỉ rõ boundary.

============================================================
# PHẦN 12 — STORAGE
============================================================

Phân tích tất cả storage được sử dụng:

eMMC
SD
NAND
NOR
SPI-NOR
USB storage
SATA
NVMe

Chỉ tập trung vào những gì repository thực sự sử dụng.

Trace:

command
 ↓
block layer
 ↓
driver
 ↓
controller
 ↓
DMA/register
 ↓
storage

============================================================
# PHẦN 13 — FILESYSTEM
============================================================

Nếu có:

FAT
EXT4
UBIFS
JFFS2
SquashFS
...

Trace:

fs command
 ↓
filesystem layer
 ↓
block device
 ↓
driver
 ↓
storage

============================================================
# PHẦN 14 — ENVIRONMENT
============================================================

Phân tích:

env_get()
env_set()
env_save()
env_load()

Environment nằm ở đâu?

NOR?
eMMC?
NAND?
file?
redundant environment?

Trace toàn bộ lifecycle.

============================================================
# PHẦN 15 — COMMAND SYSTEM
============================================================

Giải thích U-Boot command architecture.

Trace:

U_BOOT_CMD()
 ↓
command registration
 ↓
command lookup
 ↓
do_xxx()
 ↓
subsystem
 ↓
driver

Liệt kê các command quan trọng của repository.

Đặc biệt:

boot
bootm
booti
bootz
load
fatload
ext4load
mm
md
mw
setenv
saveenv
printenv
reset
run
source

Chỉ phân tích những command thực sự được build.

============================================================
# PHẦN 16 — BOOT COMMAND
============================================================

Trace chính xác:

main_loop()
 ↓
bootdelay
 ↓
bootcmd
 ↓
run
 ↓
load
 ↓
boot command
 ↓
kernel

Giải thích:

bootargs
boot_targets
bootcmd
kernel_addr_r
fdt_addr_r
ramdisk_addr_r

nếu tồn tại.

============================================================
# PHẦN 17 — IMAGE FORMAT
============================================================

Phân tích:

uImage
FIT
ITB
Image
zImage
uImage
initrd
DTB

Trace:

load image
 ↓
parse header
 ↓
verify
 ↓
decompress
 ↓
relocate
 ↓
prepare boot params
 ↓
jump

============================================================
# PHẦN 18 — KERNEL HANDOFF
============================================================

Trace:

bootm
booti
bootz

cho đến instruction cuối cùng của U-Boot trước khi Linux nhận CPU.

Giải thích:

- registers
- kernel address
- DTB address
- initrd
- bootargs
- CPU state
- MMU/cache state nếu liên quan

============================================================
# PHẦN 19 — NETWORK
============================================================

Nếu board sử dụng network:

MAC
PHY
MDIO
Ethernet controller
DMA
ARP
DHCP
TFTP
NFS
PXE

Trace một packet flow thực tế.

============================================================
# PHẦN 20 — USB
============================================================

Nếu có USB:

USB host controller
 ↓
USB core
 ↓
hub
 ↓
device
 ↓
storage/network/etc.

Trace một USB device thực tế.

============================================================
# PHẦN 21 — SECURITY
============================================================

Phân tích nếu có:

Secure Boot
HAB
AVB
FIT signature
RSA
SHA
AES
TPM
rollback protection
image verification

Trace:

image
 ↓
hash
 ↓
signature
 ↓
key
 ↓
verification
 ↓
boot

============================================================
# PHẦN 22 — WATCHDOG / RESET / POWER
============================================================

Phân tích:

watchdog
reset
power domains
PMIC
thermal
reset cause

Trace hardware interaction.

============================================================
# PHẦN 23 — CACHE / MMU / CPU
============================================================

Giải thích:

MMU
cache
TLB
barrier
cache flush
cache invalidate
DMA coherency

Đặc biệt tìm các chỗ:

flush_dcache_all()
invalidate_dcache_all()
dma_map_*
dma_unmap_*

và giải thích tại sao chúng cần thiết.

============================================================
# PHẦN 24 — DMA / INTERRUPT
============================================================

Trace nếu có:

DMA
interrupt
IRQ controller
callback
polling

Giải thích flow thực tế.

============================================================
# PHẦN 25 — BUILD SYSTEM
============================================================

Phân tích từ:

make <board>

đến:

Kconfig
.config
Makefile
arch Makefile
board Makefile
driver Makefile
generated config
generated headers
linker script
compiler
linker
u-boot.bin
u-boot.img
SPL
FIT

Tạo build dependency diagram.

============================================================
# PHẦN 26 — CONFIGURATION
============================================================

Liệt kê các CONFIG_* quan trọng.

Nhưng không chỉ liệt kê.

Với mỗi CONFIG quan trọng:

CONFIG_X
 ↓
Kconfig
 ↓
Makefile
 ↓
compiled source
 ↓
runtime behavior

============================================================
# PHẦN 27 — LINKER SCRIPT
============================================================

Phân tích:

u-boot.lds
sections:

.text
.rodata
.data
.bss
.rel*
.u_boot_list
__image_copy_start
__rel_dyn
__rel_dyn_end

Giải thích linker symbols quan trọng.

============================================================
# PHẦN 28 — INTERRUPT / EXCEPTION
============================================================

Nếu có:

exception vector
IRQ
FIQ
abort
data abort
prefetch abort
undefined instruction
Synchronous exception

Trace từ CPU exception → U-Boot handler.

============================================================
# PHẦN 29 — ERROR HANDLING
============================================================

Tìm cách U-Boot xử lý:

return codes
errno
panic
hang
WATCHDOG_RESET
reset_cpu()

Phân tích những failure path quan trọng.

============================================================
# PHẦN 30 — CUSTOM / VENDOR CODE
============================================================

Đây là phần CỰC KỲ QUAN TRỌNG.

Tìm:

- vendor directories
- custom drivers
- custom commands
- patches
- custom boot flow
- custom environment
- custom board code
- custom security
- custom storage
- custom firmware update
- custom recovery
- custom reboot

Xác định:

"Vendor đã thay đổi U-Boot upstream ở đâu?"

Tạo bảng:

File
Function
Upstream behavior
Vendor modification
Reason
Runtime effect

============================================================
# PHẦN 31 — OTA / FIRMWARE UPDATE
============================================================

Nếu repository có firmware update:

Trace toàn bộ:

download
 ↓
verify
 ↓
storage
 ↓
partition
 ↓
environment
 ↓
boot selection
 ↓
rollback
 ↓
reboot

Giải thích A/B nếu có.

============================================================
# PHẦN 32 — COMPLETE CALL GRAPH
============================================================

Tạo call graph cho các flow quan trọng:

1. Reset → Linux
2. SPL → U-Boot
3. U-Boot startup
4. Driver probe
5. Storage read
6. Environment load
7. bootcmd
8. bootm/booti
9. reboot
10. recovery/update

Không cần dump toàn bộ call graph.

Chỉ giữ các function quan trọng.

============================================================
# PHẦN 33 — DATA FLOW
============================================================

Ngoài call graph, phải giải thích DATA FLOW.

Ví dụ:

Device Tree
 ↓
ofnode
 ↓
udevice
 ↓
driver
 ↓
private data
 ↓
hardware register

Hoặc:

bootcmd
 ↓
load address
 ↓
image buffer
 ↓
image parser
 ↓
kernel
 ↓
DTB
 ↓
boot params

============================================================
# PHẦN 34 — STATE MACHINE
============================================================

Tìm những subsystem có state machine.

Ví dụ:

boot state
device state
USB state
MMC state
network state
update state

Vẽ state transition.

============================================================
# PHẦN 35 — HARDWARE REGISTER
============================================================

Đối với hardware-critical code:

Không cần giải thích từng register.

Chỉ giải thích register quan trọng:

register
bit
purpose
write/read
caller
effect

Ví dụ:

driver
 ↓
REG_CTRL
 ↓
BIT_ENABLE
 ↓
hardware starts

============================================================
# PHẦN 36 — IMPORTANT STRUCTURES
============================================================

Tìm các struct quan trọng nhất.

Ví dụ:

global_data
driver
udevice
uclass
dm_driver
bootm_headers
image_header
boot_params
...

Với mỗi struct:

- ai tạo
- ai sở hữu
- ai sửa
- ai đọc
- lifecycle

============================================================
# PHẦN 37 — CONCURRENCY / ASYNC
============================================================

Nếu repository có:

interrupt
DMA
callback
timer
async operation
polling

giải thích execution context.

Đặc biệt:

CPU context
interrupt context
polling
callback

============================================================
# PHẦN 38 — PERFORMANCE
============================================================

Tìm các đoạn:

- memory copy
- DMA
- cache operation
- compression
- image verification
- storage read
- network transfer

Giải thích bottleneck có thể nằm ở đâu.

============================================================
# PHẦN 39 — DEBUGGING
============================================================

Cho tôi biết cách debug repository này.

Bao gồm:

printf
debug()
log()
CONFIG_LOG
trace
serial
commands

Và cách debug:

startup
driver probe
MMC
USB
network
boot
kernel handoff

============================================================
# PHẦN 40 — FAILURE / RECOVERY
============================================================

Tìm các failure scenario quan trọng:

DDR fail
storage fail
environment corrupt
image corrupt
signature invalid
kernel fail
DTB fail
network fail
watchdog
boot timeout
rollback

Với mỗi failure:

condition
 ↓
detection
 ↓
handler
 ↓
recovery

============================================================
# PHẦN 41 — COMPLETE BOOT STORY
============================================================

Sau khi nghiên cứu xong, hãy kể lại toàn bộ boot process bằng ngôn ngữ con người.

Ví dụ:

"CPU reset.
Boot ROM load SPL.
SPL init DDR.
SPL đọc U-Boot từ eMMC.
U-Boot relocate vào DDR.
...
Cuối cùng booti() nhảy vào Linux."

Nhưng mỗi đoạn phải gắn với function/source thực tế.

============================================================
# PHẦN 42 — "IF I ONLY REMEMBER 50 THINGS"
============================================================

Tạo danh sách:

50 điều quan trọng nhất tôi phải nhớ về U-Boot repository này.

Mỗi item:

Concept
 ↓
Why important
 ↓
Where in source
 ↓
Runtime behavior

============================================================
# PHẦN 43 — MENTAL MODEL
============================================================

Cuối cùng xây dựng cho tôi một mental model.

Tôi muốn nhìn U-Boot như:

                U-BOOT
                   │
       ┌───────────┼───────────┐
       ↓           ↓           ↓
   Startup      Drivers      Commands
       │           │           │
       ↓           ↓           ↓
    Memory      Hardware      Boot
       │           │           │
       └───────────┼───────────┘
                   ↓
                Linux

Nhưng hãy xây dựng mental model đúng với repository thực tế.

============================================================
# OUTPUT FORMAT
============================================================

Không trả lời thành một khối text khổng lồ.

Chia thành:

PART 01 — Repository Architecture
PART 02 — Hardware Architecture
PART 03 — Reset & Startup
PART 04 — TPL/SPL
PART 05 — U-Boot Relocation
PART 06 — Board Init
PART 07 — Driver Model
PART 08 — Device Tree
PART 09 — Memory
PART 10 — UART
PART 11 — Clock/Reset/GPIO
PART 12 — DDR
PART 13 — Storage
PART 14 — Filesystem
PART 15 — Environment
PART 16 — Commands
PART 17 — Boot Flow
PART 18 — Image Loading
PART 19 — Kernel Handoff
PART 20 — Network
PART 21 — USB
PART 22 — Security
PART 23 — Watchdog/Power
PART 24 — MMU/Cache/DMA
PART 25 — Build System
PART 26 — Kconfig
PART 27 — Linker
PART 28 — Exception/Interrupt
PART 29 — Error Handling
PART 30 — Vendor Customization
PART 31 — Firmware Update
PART 32 — Call Graph
PART 33 — Data Flow
PART 34 — State Machines
PART 35 — Hardware Registers
PART 36 — Important Structures
PART 37 — Async/Concurrency
PART 38 — Performance
PART 39 — Debugging
PART 40 — Failure/Recovery
PART 41 — Complete Boot Story
PART 42 — Top 50 Things
PART 43 — Mental Model

============================================================
# QUY TẮC GIẢI THÍCH
============================================================

Mỗi phần phải có:

1. WHAT
2. WHY
3. WHERE
4. HOW
5. CALL FLOW
6. DATA FLOW
7. HARDWARE INTERACTION
8. IMPORTANT CODE
9. COMMON CONFUSION
10. MENTAL MODEL

Không cần paste code dài.

Chỉ trích function/code fragment ngắn khi cần thiết để chứng minh behavior.

Mỗi function quan trọng phải có:

caller → function → callee → effect

Mỗi subsystem phải có:

User/Command
 ↓
U-Boot subsystem
 ↓
Driver
 ↓
Hardware