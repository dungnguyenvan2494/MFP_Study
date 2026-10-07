#!/usr/bin/env python3
"""
kswapd_toy_sim.py - mo hinh DO CHOI (toy) cua balance_pgdat() de minh hoa cau chuyen end-to-end.

!! DAY LA MO HINH DON GIAN HOA, KHONG PHAI KERNEL. !!
Nhung gi la THAT (chep tu source 5.10.241):
  - scan_target = lru_size >> priority            (get_scan_count, vmscan.c:2410)
  - priority bat dau 12, giam 1 moi vong neu chua thu du (balance_pgdat, vmscan.c:3612,3738-3739)
  - kswapd dung khi co zone >= HIGH watermark (pgdat_balanced)
  - nr_to_reclaim = max(HIGH, 32)  (kswapd_shrink_node)
  - raise_priority = false khi nr_scanned >= nr_to_reclaim   (vmscan.c:3704-3705)
  - kswapd_failures chi tang khi CA DOT khong thu duoc page nao (vmscan.c:3742-3743)
Nhung gi la GIA DINH (khong do tu board):
  - Chi 1 zone, khong swap -> chi quet file LRU (SCAN_FILE)
  - Thanh phan cua inactive-file list la ty le co dinh (clean_unmapped / referenced / dirty / writeback)
  - Page khong reclaim duoc bi chuyen khoi duoi inactive (activate) nen list nho dan
  - Khong mo hinh refault, slab, compaction, flusher
"""

MIN, LOW, HIGH = 1000, 1250, 1500       # tu tools/vmscan_numbers.py (managed=250000 page)
SWAP_CLUSTER_MAX = 32


def run(name, free, inactive_file, mix, alloc_per_round=0, max_rounds=40):
    print("\n" + "=" * 78)
    print(name)
    print("=" * 78)
    print(f"start: free={free} (LOW={LOW}, HIGH={HIGH}, MIN={MIN}); inactive_file={inactive_file}; "
          f"mix={mix}; alloc/round={alloc_per_round}")
    reclaim_frac = mix["clean_unmapped"]
    nr_to_reclaim = max(HIGH, SWAP_CLUSTER_MAX)
    priority = 12
    total_reclaimed = total_scanned = 0
    rounds = 0
    stalled = 0     # so page process muon xin nhung free = 0 (thuc te: task vao direct reclaim / cho)
    print(f"{'rnd':>3} {'prio':>4} {'scan':>6} {'reclaimed':>9} {'activated':>9} {'free':>6} {'inactive':>8} {'stalled':>7}  note")
    while priority >= 1 and rounds < max_rounds:
        rounds += 1
        if free >= HIGH:
            print(f"{rounds:>3} {priority:>4} {'-':>6} {'-':>9} {'-':>9} {free:>6} {inactive_file:>8} {stalled:>7}  pgdat_balanced -> kswapd ngu")
            break
        scan = min(inactive_file, inactive_file >> priority)
        # kswapd quet theo lo 32, bo qua phan le < 1 lo neu list nho
        reclaimed = int(scan * reclaim_frac)
        activated = scan - reclaimed
        free += reclaimed
        inactive_file -= scan
        total_reclaimed += reclaimed
        total_scanned += scan
        note = ""
        scanned_enough = scan >= nr_to_reclaim
        if scanned_enough:
            note += "scan>=nr_to_reclaim: KHONG tang priority; "
        if free < MIN:
            note += "free<MIN: task xin RAM vao DIRECT reclaim; "
        print(f"{rounds:>3} {priority:>4} {scan:>6} {reclaimed:>9} {activated:>9} {free:>6} {inactive_file:>8} {stalled:>7}  {note}")
        got = min(alloc_per_round, free)      # tien trinh van dang xin them RAM
        stalled += alloc_per_round - got      # phan khong cap duoc = task bi cho/direct reclaim (KHONG mo hinh hoa)
        free -= got
        if (not scanned_enough) or reclaimed == 0:
            priority -= 1
    eff = (100.0 * total_reclaimed / total_scanned) if total_scanned else 0.0
    print(f"--> tong quet={total_scanned}, thu={total_reclaimed}, hieu suat={eff:.1f}%, priority cuoi={priority}")
    if total_reclaimed == 0:
        print("--> thu 0 page ca dot: kswapd_failures++ (can du 16 dot lien tiep moi 'hopeless')")
    else:
        print("--> co thu duoc >0 page: kswapd_failures khong tang (se bi reset ve 0 neu dang >0)")


# A: kswapd vua bi danh thuc (free vua duoi LOW), cache sach doi dao.
run("KICH BAN A  (khoe): cache sach doi dao, process vua xin RAM lam free rot duoi LOW",
    free=1200, inactive_file=60000,
    mix=dict(clean_unmapped=0.70, referenced=0.10, dirty=0.15, writeback=0.05))

# B: working set nong + nhieu dirty, process van xin 300 page/vong.
run("KICH BAN B  (khong hieu qua): da so page nong/dirty, process xin RAM lien tuc",
    free=1100, inactive_file=60000,
    mix=dict(clean_unmapped=0.04, referenced=0.55, dirty=0.35, writeback=0.06),
    alloc_per_round=100)

# C: list inactive nho (anon/tmpfs chiem het, khong swap).
run("KICH BAN C  (khong swap, anon/tmpfs chiem RAM): inactive-file chi con 3000 page",
    free=900, inactive_file=3000,
    mix=dict(clean_unmapped=0.60, referenced=0.20, dirty=0.15, writeback=0.05),
    alloc_per_round=100)
