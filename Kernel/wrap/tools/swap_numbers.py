#!/usr/bin/env python3
"""
swap_numbers.py - tinh moi con so vi du dung trong docs/swap/.

Tat ca hang so lay tu source local Kernel/K-S800/Src (Linux 5.10.241, arm64, 4K page):
  include/linux/swap.h        SWAP_HAS_CACHE=0x40, SWAP_MAP_MAX=0x3e, SWAP_MAP_BAD=0x3f,
                              SWAP_BATCH=64, SWAP_ADDRESS_SPACE_SHIFT=14, MAX_SWAPFILES_SHIFT=5
  include/linux/swap_slots.h  SWAP_SLOTS_CACHE_SIZE=SWAP_BATCH, ACTIVATE=5*SIZE, DEACTIVATE=2*SIZE
  include/linux/swapops.h     SWP_TYPE_SHIFT = BITS_PER_XA_VALUE - MAX_SWAPFILES_SHIFT
  arch/arm64/include/asm/pgtable.h  __SWP_TYPE_SHIFT=2, __SWP_TYPE_BITS=6, __SWP_OFFSET_BITS=50
  mm/swapfile.c               SWAPFILE_CLUSTER=256 (CONFIG_THP_SWAP=n tren arm64 5.10)
  mm/swap_cgroup.c            struct swap_cgroup { unsigned short id; } -> 2 byte/slot
  mm/swap.c swap_setup()      page_cluster = 3 neu RAM >= 16 MB

Gia dinh (INFERENCE, khong doc duoc tu source):
  sizeof(struct swap_cluster_info) = 8 (spinlock_t 4 byte khi khong bat lock debug + 4 byte bitfield)
  sizeof(swp_entry_t) = 8 (unsigned long tren arm64)
Chay:  python swap_numbers.py > swap_numbers.out.txt
"""

PAGE_SIZE = 4096
KiB, MiB, GiB = 1024, 1024 ** 2, 1024 ** 3

# ---- hang so tu source ----
SWAP_HAS_CACHE = 0x40
COUNT_CONTINUED = 0x80
SWAP_MAP_MAX = 0x3E
SWAP_MAP_BAD = 0x3F
SWAP_MAP_SHMEM = 0xBF
SWAP_CONT_MAX = 0x7F
SWAP_BATCH = 64
SWAP_SLOTS_CACHE_SIZE = SWAP_BATCH
THRESHOLD_ACTIVATE = 5 * SWAP_SLOTS_CACHE_SIZE
THRESHOLD_DEACTIVATE = 2 * SWAP_SLOTS_CACHE_SIZE
SWAP_ADDRESS_SPACE_SHIFT = 14
SWAP_ADDRESS_SPACE_PAGES = 1 << SWAP_ADDRESS_SPACE_SHIFT
SWAPFILE_CLUSTER = 256
MAX_SWAPFILES_SHIFT = 5
BITS_PER_LONG = 64
BITS_PER_XA_VALUE = BITS_PER_LONG - 1
SWP_TYPE_SHIFT = BITS_PER_XA_VALUE - MAX_SWAPFILES_SHIFT
SWP_OFFSET_MASK = (1 << SWP_TYPE_SHIFT) - 1
ARCH_SWP_TYPE_SHIFT, ARCH_SWP_TYPE_BITS, ARCH_SWP_OFFSET_BITS = 2, 6, 50
ARCH_SWP_OFFSET_SHIFT = ARCH_SWP_TYPE_BITS + ARCH_SWP_TYPE_SHIFT
SC_PER_PAGE = PAGE_SIZE // 2
SIZEOF_CLUSTER_INFO = 8   # INFERENCE
SIZEOF_SWP_ENTRY = 8      # INFERENCE
NR_CPUS = 4               # CONFIG_NR_CPUS=4 trong defconfig KM

# MAX_SWAPFILES = 32 - SWP_DEVICE_NUM - SWP_MIGRATION_NUM - SWP_HWPOISON_NUM
# defconfig KM: DEVICE_PRIVATE khong bat (0), MIGRATION=y (2), MEMORY_FAILURE=n (0)
MAX_SWAPFILES = (1 << MAX_SWAPFILES_SHIFT) - 0 - 2 - 0


def hdr(t):
    print("\n" + "=" * 72 + "\n" + t + "\n" + "=" * 72)


def swp_entry(t, off):
    return (t << SWP_TYPE_SHIFT) | (off & SWP_OFFSET_MASK)


def swp_type(v):
    return v >> SWP_TYPE_SHIFT


def swp_offset(v):
    return v & SWP_OFFSET_MASK


def arch_pte(t, off):
    """__swp_entry_to_pte(__swp_entry(type, offset)) tren arm64"""
    return (t << ARCH_SWP_TYPE_SHIFT) | (off << ARCH_SWP_OFFSET_SHIFT)


def ram_and_swap(ram, swap):
    hdr(f"1. Vi du RAM {ram // MiB} MiB + swap {swap // MiB} MiB (page 4 KiB)")
    ram_pages = ram // PAGE_SIZE
    swap_pages_file = swap // PAGE_SIZE
    last_page = swap_pages_file - 1          # mkswap ghi last_page = so page - 1
    maxpages = last_page + 1                 # read_swap_header(): maxpages = last_page + 1
    good = maxpages - 1                      # setup_swap_map_and_extents(): bo page header
    print(f"RAM page frame                    : {ram_pages:>10}")
    print(f"swap header last_page             : {last_page:>10}")
    print(f"si->max (maxpages)                : {maxpages:>10}")
    print(f"si->pages (slot dung duoc, tru header): {good:>6}")
    print(f"swap_map = vzalloc(maxpages)      : {maxpages:>10} byte = {maxpages / KiB:.0f} KiB (1 byte/slot)")
    ncl = -(-maxpages // SWAPFILE_CLUSTER)
    print(f"so cluster (256 slot/cluster)     : {ncl:>10}")
    print(f"cluster_info (chi SSD/non-rot)    : {ncl * SIZEOF_CLUSTER_INFO:>10} byte = {ncl * SIZEOF_CLUSTER_INFO / KiB:.0f} KiB (gia dinh 8 byte/cluster)")
    nsp = -(-maxpages // SWAP_ADDRESS_SPACE_PAGES)
    print(f"so address_space cua swap cache   : {nsp:>10} (moi cai phu {SWAP_ADDRESS_SPACE_PAGES} slot = 64 MiB)")
    sc_len = -(-maxpages // SC_PER_PAGE)
    print(f"swap_cgroup: SC_PER_PAGE          : {SC_PER_PAGE:>10} entry/page")
    print(f"swap_cgroup: ctrl->length         : {sc_len:>10} page = {sc_len * PAGE_SIZE / MiB:.2f} MiB (+ mang con tro {sc_len * 8} byte)")
    print(f"Tong metadata/so voi du lieu swap : {(maxpages + ncl * SIZEOF_CLUSTER_INFO + sc_len * PAGE_SIZE) / swap * 100:.3f} %")
    print(f"vm_swap_full() dung khi free_swap*2 < total -> free < {good / 2:.1f} slot (~50%)")
    return maxpages


def slots_cache():
    hdr("2. Per-CPU swap slots cache (swap_slots.c)")
    print(f"SWAP_SLOTS_CACHE_SIZE             : {SWAP_SLOTS_CACHE_SIZE}")
    print(f"bo nho slots[] + slots_ret[] /CPU  : {2 * SWAP_SLOTS_CACHE_SIZE * SIZEOF_SWP_ENTRY} byte")
    print(f"nr_cpus online (gia dinh)          : {NR_CPUS}")
    print(f"deactivate khi nr_swap_pages <     : {NR_CPUS * THRESHOLD_DEACTIVATE}")
    print(f"reactivate khi nr_swap_pages >     : {NR_CPUS * THRESHOLD_ACTIVATE}")
    print(f"slot 'giu' toi da trong cache      : {NR_CPUS * 2 * SWAP_SLOTS_CACHE_SIZE} "
          f"(= {NR_CPUS * 2 * SWAP_SLOTS_CACHE_SIZE * PAGE_SIZE / MiB:.0f} MiB dung luong swap)")
    print("\nHysteresis check_cache_active() voi 4 CPU (gia tri nr_swap_pages lan luot):")
    active = True
    for pages in [5000, 2000, 1300, 900, 600, 511, 400, 800, 1200, 1280, 1281, 3000]:
        before = active
        if not active:
            if pages > NR_CPUS * THRESHOLD_ACTIVATE:
                active = True
        else:
            if pages < NR_CPUS * THRESHOLD_DEACTIVATE:
                active = False
        note = ""
        if before != active:
            note = "  <-- " + ("deactivate + drain tat ca CPU" if not active else "reactivate")
        print(f"  nr_swap_pages={pages:>5}  cache_active={str(active):5}{note}")


def entries():
    hdr("3. Ma hoa swp_entry_t va swap PTE (arm64)")
    print(f"SWP_TYPE_SHIFT (arch-independent) : {SWP_TYPE_SHIFT}")
    print(f"MAX_SWAPFILES (config KM)         : {MAX_SWAPFILES}")
    print(f"Offset toi da trong PTE arm64     : 2^{ARCH_SWP_OFFSET_BITS} (nhung si->max la unsigned int)")
    for t, off in [(0, 5), (0, 1), (1, 0x1234), (0, 524287)]:
        v = swp_entry(t, off)
        p = arch_pte(t, off)
        print(f"type={t} offset={off:>7}: swp_entry.val=0x{v:016x}  "
              f"swp_type={swp_type(v)} swp_offset={swp_offset(v)}  "
              f"PTE=0x{p:016x} (bit0..1 = {p & 3} -> not present)")


def locate(maxpages):
    hdr("4. Tu offset den moi cau truc lien quan (offset = 300000, type 0)")
    off = 300000
    assert off < maxpages
    print(f"swap_map[{off}]                         -> 1 byte trong vmalloc array")
    print(f"cluster index = offset/256             -> {off // SWAPFILE_CLUSTER} (lock_cluster dung ci + idx)")
    print(f"swapper_spaces[0][offset>>14]          -> address_space #{off >> SWAP_ADDRESS_SPACE_SHIFT}, xarray index {off}")
    print(f"swap_cgroup: map[offset/2048], [offset%2048] -> page {off // SC_PER_PAGE}, entry {off % SC_PER_PAGE}")
    print(f"vi tri tren dia (1 extent, block device) -> page {off} cua device = byte {off * PAGE_SIZE} "
          f"= sector {off * PAGE_SIZE // 512}")


def swap_map_values():
    hdr("5. Giai ma mot so gia tri swap_map")
    for val in [0x00, 0x40, 0x01, 0x41, 0x03, 0x43, 0x3E, 0x3F, 0xBF, 0x80 | 0x3E]:
        has = bool(val & SWAP_HAS_CACHE) and val not in (SWAP_MAP_BAD, SWAP_MAP_SHMEM)
        cnt = val & ~SWAP_HAS_CACHE
        if val == 0:
            desc = "slot trong (free)"
        elif val == SWAP_MAP_BAD:
            desc = "SWAP_MAP_BAD: header/bad block/cluster dang discard"
        elif val == SWAP_MAP_SHMEM:
            desc = "SWAP_MAP_SHMEM: slot cua shmem/tmpfs"
        elif cnt & COUNT_CONTINUED:
            desc = f"count da dat SWAP_MAP_MAX, phan cao nam o continuation page"
        else:
            desc = f"swap count={cnt}, SWAP_HAS_CACHE={'1' if has else '0'}"
        print(f"  0x{val:02x}: {desc}")


def readahead():
    hdr("6. Swap readahead (swap_state.c)")
    page_cluster = 3
    print(f"page_cluster (swap_setup, RAM>=16MB) : {page_cluster} -> max {1 << page_cluster} page/lan")

    def nr(prev, off, hits, maxp, prev_win):
        pages = hits + 2
        if pages == 2:
            if off != prev + 1 and off != prev - 1:
                pages = 1
        else:
            r = 4
            while r < pages:
                r <<= 1
            pages = r
        pages = min(pages, maxp)
        pages = max(pages, prev_win // 2)
        return pages

    print("__swapin_nr_pages(prev, off, hits, max=8, prev_win):")
    for args in [(100, 500, 0, 8, 0), (100, 101, 0, 8, 0), (100, 500, 1, 8, 0),
                 (100, 500, 3, 8, 0), (100, 500, 0, 8, 8), (100, 500, 7, 8, 1)]:
        print(f"  prev={args[0]:>3} off={args[1]:>3} hits={args[2]} prev_win={args[4]} -> {nr(*args)} page")


def lock_counts():
    hdr("7. Dem so lan lay khoa toan cuc khi cap/tra 1 000 000 slot (vi du E)")
    n = 1_000_000
    print(f"Khong co slot cache : cap {n} lan get_swap_pages(1) -> {n} lan swap_avail_lock + {n} lan si->lock")
    print(f"                      tra {n} lan swapcache_free_entries(1) -> {n} lan si->lock")
    refills = -(-n // SWAP_SLOTS_CACHE_SIZE)
    print(f"Co slot cache (64)  : cap -> {refills} lan refill (moi lan 1 swap_avail_lock + 1 si->lock) "
          f"+ {n} lan alloc_lock (mutex per-CPU, hau nhu khong tranh chap)")
    print(f"                      tra -> {refills} lan swapcache_free_entries(64) (sort theo area, 1 si->lock/lo) "
          f"+ {n} lan free_lock (spinlock per-CPU)")
    print(f"Ti le giam khoa toan cuc: {n / refills:.0f} lan")


if __name__ == "__main__":
    mp = ram_and_swap(1 * GiB, 2 * GiB)
    slots_cache()
    entries()
    locate(mp)
    swap_map_values()
    readahead()
    lock_counts()
