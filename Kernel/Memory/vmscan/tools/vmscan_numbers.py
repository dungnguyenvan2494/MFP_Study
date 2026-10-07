#!/usr/bin/env python3
"""
vmscan_numbers.py - tinh cac con so vi du dung trong docs/vmscan/*.md

Moi cong thuc duoc chep tu source local (Linux 5.10.241):
  mm/page_alloc.c  : init_per_zone_wmark_min / __setup_per_zone_wmarks
  mm/vmscan.c      : inactive_is_low, get_scan_count, do_shrink_slab,
                     kswapd_shrink_node, allow_direct_reclaim
  include/linux/compaction.h : compact_gap
Gia dinh (GIA DINH, khong phai do tu board): 1 GiB RAM, 1 zone Normal,
managed ~ 250000 page, 4 KiB/page, swappiness = 60.

Chay:  python vmscan_numbers.py
"""
from math import isqrt

PAGE_KB = 4
SWAP_CLUSTER_MAX = 32
DEF_PRIORITY = 12


def section(t):
    print("\n" + "=" * 70 + "\n" + t + "\n" + "=" * 70)


# ---------------------------------------------------------------- watermark
section("1. Watermark cho 1 zone, managed = 250000 page (~977 MiB)")
managed = 250000
lowmem_kb = managed * PAGE_KB                      # nr_free_buffer_pages ~ managed (xap xi)
min_free_kb = isqrt(lowmem_kb * 16)                # init_per_zone_wmark_min
min_free_kb = max(128, min(262144, min_free_kb))
pages_min = min_free_kb >> (12 - 10)               # min_free_kbytes >> (PAGE_SHIFT-10)
wm_min = pages_min                                 # 1 zone => tmp = pages_min * managed / lowmem_pages = pages_min
scale = 10                                         # watermark_scale_factor
tmp = max(wm_min >> 2, managed * scale // 10000)   # mult_frac(managed, scale, 10000)
wm_low = wm_min + tmp
wm_high = wm_min + 2 * tmp
print(f"min_free_kbytes = {min_free_kb} KiB  -> pages_min = {pages_min}")
print(f"tmp = max(min>>2={wm_min>>2}, managed*{scale}/10000={managed*scale//10000}) = {tmp}")
print(f"WMARK_MIN  = {wm_min:6d} page = {wm_min*PAGE_KB/1024:.2f} MiB")
print(f"WMARK_LOW  = {wm_low:6d} page = {wm_low*PAGE_KB/1024:.2f} MiB")
print(f"WMARK_HIGH = {wm_high:6d} page = {wm_high*PAGE_KB/1024:.2f} MiB")
print("=> free < LOW  : allocator danh thuc kswapd (wakeup_kswapd)")
print("=> free > HIGH : kswapd ngu lai (pgdat_balanced)")
print("=> free < MIN  : chi con ALLOC_HARDER/ALLOC_HIGH/PF_MEMALLOC duoc cap tiep; con lai vao direct reclaim")

nr_to_reclaim_kswapd = max(wm_high, SWAP_CLUSTER_MAX)
print(f"kswapd_shrink_node: nr_to_reclaim = max(high, 32) = {nr_to_reclaim_kswapd} page "
      f"(~{nr_to_reclaim_kswapd*PAGE_KB/1024:.1f} MiB moi lan shrink_node)")

# boost
max_boost = wm_high * 15000 // 10000               # watermark_boost_factor = 15000 (mm/page_alloc.c:367)
print(f"max watermark boost = high * 15000/10000 = {max_boost} page  (boost_watermark)")

# ---------------------------------------------------------------- priority
section("2. Priority -> so page quet (scan >>= priority), danh sach 80000 page")
lru = 80000
print("priority | scan_target | so lan goi shrink_list (batch 32)")
for p in (12, 10, 8, 6, 4, 2, 0):
    scan = lru >> p
    batches = -(-scan // SWAP_CLUSTER_MAX) if scan else 0
    print(f"   {p:2d}    | {scan:8d}    | {batches}")
print("priority 12 (DEF_PRIORITY) = quet 1/4096 danh sach; priority 0 = quet het.")
print("do_try_to_free_pages giam priority tu 12 xuong 0 (--sc->priority) neu chua du nr_to_reclaim.")

# ---------------------------------------------------------------- inactive ratio
section("3. inactive_is_low: inactive_ratio theo dung luong list (anon hoac file)")
print("tong (GiB) | inactive_ratio | inactive toi da giu (xap xi)")
for gb in (0, 1, 2, 4, 10, 100):
    ratio = isqrt(10 * gb) if gb else 1
    # inactive * ratio < active  => inactive_is_low.  Can bang: inactive = total/(ratio+1)
    frac = 1 / (ratio + 1)
    print(f"  {gb:4d}     |      {ratio:3d}       | {frac*100:5.1f}% tong")
print("(source comment: 1GB -> ratio 3 -> 25% inactive; khop voi bang trong vmscan.c:2209-2218)")

# ---------------------------------------------------------------- get_scan_count
section("4. get_scan_count / SCAN_FRACT: chia ap luc anon vs file (swappiness=60)")


def fract(swappiness, anon_cost, file_cost):
    total_cost = anon_cost + file_cost
    a = total_cost + anon_cost
    f = total_cost + file_cost
    total = a + f
    ap = swappiness * (total + 1) // (a + 1)
    fp = (200 - swappiness) * (total + 1) // (f + 1)
    return ap, fp


cases = [
    ("khong co cost (moi bat dau)", 60, 0, 0),
    ("anon refault nhieu (anon_cost=1000, file_cost=0)", 60, 1000, 0),
    ("file refault nhieu (anon_cost=0, file_cost=1000)", 60, 0, 1000),
    ("swappiness=100, cost bang nhau (1000/1000)", 100, 1000, 1000),
    ("swappiness=0 nhung SCAN_FRACT (global reclaim)", 0, 0, 0),
    ("swappiness=200 (chi anon)", 200, 0, 0),
]
print(f"{'case':58s} | ap  | fp  | anon% | file%")
for name, sw, ac, fc in cases:
    ap, fp = fract(sw, ac, fc)
    d = ap + fp
    print(f"{name:58s} | {ap:3d} | {fp:3d} | {100*ap/d:5.1f} | {100*fp/d:5.1f}")

# ---------------------------------------------------------------- shrink_list batches
section("5. Vi du 1 vong shrink_lruvec: nr[] va thu tu quet")
nr = {"anon_inact": 12, "anon_act": 10, "file_inact": 64, "file_act": 40}
print("Gia su get_scan_count tra ve:", nr)
rounds = 0
while nr["anon_inact"] or nr["file_act"] or nr["file_inact"]:
    rounds += 1
    for k in ("anon_inact", "anon_act", "file_inact", "file_act"):
        if nr[k]:
            take = min(nr[k], SWAP_CLUSTER_MAX)
            nr[k] -= take
            print(f"  round {rounds}: shrink_list({k:10s}, {take:2d})  con lai {nr[k]}")
    # vong while chi kiem tra anon_inact, file_act, file_inact (khong kiem anon_act)
print("(luu y: dieu kien while cua shrink_lruvec bo qua nr[LRU_ACTIVE_ANON])")

# ---------------------------------------------------------------- slab
section("6. do_shrink_slab: delta = (freeable >> priority) * 4 / seeks")
freeable, seeks, batch = 10000, 2, 128
print(f"freeable={freeable} object, seeks={seeks} (DEFAULT_SEEKS), batch={batch}")
print("priority | delta | total_scan | co goi scan_objects khong?")
for p in (12, 10, 8, 6, 4, 2, 0):
    delta = (freeable >> p) * 4 // seeks
    total = delta
    if delta < freeable // 4:
        total = min(total, freeable // 2)
    total = min(total, freeable * 2)
    run = total >= batch or total >= freeable
    print(f"   {p:2d}    | {delta:5d} | {total:6d}     | {'CO' if run else 'khong (cong don vao nr_deferred)'}")

# ---------------------------------------------------------------- throttle
section("7. allow_direct_reclaim (throttle_direct_reclaim)")
reserve = wm_min          # tong min watermark cac zone <= ZONE_NORMAL (o day 1 zone)
print(f"pfmemalloc_reserve = {reserve} page; direct reclaimer bi throttle neu free <= reserve/2 = {reserve//2}")

# ---------------------------------------------------------------- compaction
section("8. should_continue_reclaim: compact_gap = 2 << order")
for order in (1, 2, 3, 4, 5, 9):
    print(f"order {order}: compact_gap = {2 << order:4d} page; in_reclaim_compaction = "
          f"{'luon' if order > 3 else 'chi khi priority < 10'} (costly order > 3)")

# ---------------------------------------------------------------- vmpressure
section("9. vmpressure window")
win = SWAP_CLUSTER_MAX * 16
print(f"vmpressure_win = 32*16 = {win} page quet; pressure% = (1 - reclaimed/scanned)*100")
for scanned, reclaimed in ((512, 400), (512, 200), (512, 20), (512, 0)):
    pct = int((1 - reclaimed / scanned) * 100)
    lvl = "critical" if pct >= 95 else "medium" if pct >= 60 else "low"
    print(f"  scanned={scanned} reclaimed={reclaimed:3d} -> {pct:3d}% -> {lvl}")
