#!/bin/sh
# vmstat_delta.sh - do hanh vi reclaim bang delta cua /proc/vmstat (POSIX sh + awk, chay duoc tren busybox)
#
# Cach dung tren board:
#     sh vmstat_delta.sh            # lay 2 mau cach nhau 5 giay
#     sh vmstat_delta.sh 10         # cach nhau 10 giay
# Che do offline (de test / phan tich 2 file da luu):
#     sh vmstat_delta.sh --files vmstat_A.txt vmstat_B.txt [so_giay]
#
# Doc ket qua: xem docs/vmscan/05_debugging_logs_scenarios.md  muc 15.5 va 22 (cheat sheet).
# Bien moi truong (de test): MEMINFO_FILE, KSWAPD_STAT_A, KSWAPD_STAT_B

MEMINFO_FILE=${MEMINFO_FILE:-/proc/meminfo}

if [ "$1" = "--files" ]; then
    A=$2; B=$3; SECS=${4:-1}
    LIVE=0
else
    SECS=${1:-5}
    A=/tmp/vmstat_delta_A.$$; B=/tmp/vmstat_delta_B.$$
    LIVE=1
fi

# --- tim kswapd0 (khong dung pidof/pgrep vi busybox co the thieu)
find_kswapd() {
    for d in /proc/[0-9]*; do
        if [ "$(cat "$d/comm" 2>/dev/null)" = "kswapd0" ]; then echo "${d#/proc/}"; return; fi
    done
}
# utime+stime (truong 14,15 cua /proc/PID/stat); comm co the chua khoang trang nen cat sau ')'
cpu_ticks() {
    [ -r "/proc/$1/stat" ] || { echo 0; return; }
    sed 's/^.*) //' "/proc/$1/stat" | awk '{print $12 + $13}'
}

if [ "$LIVE" = 1 ]; then
    KPID=$(find_kswapd)
    cat /proc/vmstat > "$A"
    T0=$([ -n "$KPID" ] && cpu_ticks "$KPID" || echo 0)
    sleep "$SECS"
    cat /proc/vmstat > "$B"
    T1=$([ -n "$KPID" ] && cpu_ticks "$KPID" || echo 0)
    CLK=$(getconf CLK_TCK 2>/dev/null || echo 100)
else
    KPID=""; T0=${KSWAPD_STAT_A:-0}; T1=${KSWAPD_STAT_B:-0}; CLK=100
fi

echo "=== reclaim delta trong ${SECS}s ==="
awk -v secs="$SECS" -v t0="$T0" -v t1="$T1" -v clk="$CLK" -v kpid="$KPID" '
function d(k) { return (k in b ? b[k] : 0) - (k in a ? a[k] : 0) }
function sumprefix(p,   k, s) { s = 0; for (k in b) if (index(k, p) == 1) s += b[k] - (k in a ? a[k] : 0); return s }
function pct(x, y) { return (y > 0) ? sprintf("%5.1f%%", 100.0 * x / y) : "   n/a" }
function row(name, v) { printf "  %-32s %10d  (%8.1f /s)\n", name, v, v / secs }
FNR == NR { a[$1] = $2; next }
{ b[$1] = $2 }
END {
    scan_k = d("pgscan_kswapd");  steal_k = d("pgsteal_kswapd")
    scan_d = d("pgscan_direct");  steal_d = d("pgsteal_direct")
    print "-- Ai dang reclaim?"
    row("pageoutrun (kswapd balance_pgdat)", d("pageoutrun"))
    row("pgscan_kswapd", scan_k);  row("pgsteal_kswapd", steal_k)
    row("pgscan_direct", scan_d);  row("pgsteal_direct", steal_d)
    row("allocstall_* (tong, direct reclaim)", sumprefix("allocstall_"))
    row("pgscan_direct_throttle", d("pgscan_direct_throttle"))
    print "-- Hieu suat (steal/scan)"
    printf "  kswapd : %s     direct : %s\n", pct(steal_k, scan_k), pct(steal_d, scan_d)
    print "-- Quet loai page nao?"
    row("pgscan_anon", d("pgscan_anon"));  row("pgscan_file", d("pgscan_file"))
    row("pgsteal_anon", d("pgsteal_anon")); row("pgsteal_file", d("pgsteal_file"))
    printf "  file efficiency: %s   anon efficiency: %s\n", pct(d("pgsteal_file"), d("pgscan_file")), pct(d("pgsteal_anon"), d("pgscan_anon"))
    row("pgscan_skip_* (bo qua do zone)", sumprefix("pgscan_skip_"))
    print "-- Vi sao khong reclaim duoc? (dau hieu)"
    row("pgrefill (active duoc xem)", d("pgrefill"))
    row("pgdeactivate (active -> inactive)", d("pgdeactivate"))
    row("pgactivate (inactive -> active)", d("pgactivate"))
    row("nr_vmscan_write (reclaim tu ghi)", d("nr_vmscan_write"))
    row("nr_vmscan_immediate_reclaim", d("nr_vmscan_immediate_reclaim"))
    row("workingset_refault_file", d("workingset_refault_file"))
    row("workingset_refault_anon", d("workingset_refault_anon"))
    row("workingset_activate_file", d("workingset_activate_file"))
    row("thp_swpout_fallback", d("thp_swpout_fallback"))
    row("pglazyfreed", d("pglazyfreed"))
    print "-- kswapd co kip khong?"
    row("kswapd_low_wmark_hit_quickly", d("kswapd_low_wmark_hit_quickly"))
    row("kswapd_high_wmark_hit_quickly", d("kswapd_high_wmark_hit_quickly"))
    print "-- Slab / swap / compaction / OOM"
    row("slabs_scanned", d("slabs_scanned"))
    row("pswpin", d("pswpin")); row("pswpout", d("pswpout"))
    row("pgmajfault", d("pgmajfault"))
    row("compact_stall", d("compact_stall")); row("compact_fail", d("compact_fail"))
    row("oom_kill", d("oom_kill"))
    if (kpid != "" || t1 > 0) {
        dt = t1 - t0
        printf "-- kswapd0 (pid %s) CPU: %d ticks / %ds = %.1f%% cua 1 CPU\n", kpid, dt, secs, 100.0 * dt / (clk * secs)
    }
    print ""
    print "-- Chan doan nhanh"
    tot_scan = scan_k + scan_d; tot_steal = steal_k + steal_d
    if (tot_scan == 0)                          print "  * Khong co reclaim trong cua so nay."
    else if (tot_steal * 10 < tot_scan)         print "  * CANH BAO: hieu suat < 10% (quet nhieu, thu it) -> xem Scenario 3 (file 05)."
    if (sumprefix("allocstall_") > 0)           print "  * Co DIRECT RECLAIM: task xin RAM da bi cham (allocstall_*)."
    if (d("pgscan_direct_throttle") > 0)        print "  * Co task bi THROTTLE cho kswapd (pgscan_direct_throttle) -> reserve PFMEMALLOC can."
    if (d("pgscan_anon") == 0 && tot_scan > 0)  print "  * Chi quet FILE: khong co swap? (cat /proc/swaps) -> anon khong reclaim duoc."
    if (d("nr_vmscan_immediate_reclaim") > 0)   print "  * Dirty/writeback o duoi LRU: nghi flusher/IO cham (Scenario 4)."
    if (d("kswapd_low_wmark_hit_quickly") + d("kswapd_high_wmark_hit_quickly") > 0)
                                                print "  * kswapd vua ngu da bi danh thuc lai: ap luc keo dai (alloc rate >= reclaim rate)."
    if (d("oom_kill") > 0)                      print "  * OOM KILL da xay ra trong cua so."
}' "$A" "$B"

echo
echo "=== meminfo (kB) ==="
awk '/^(MemTotal|MemFree|MemAvailable|Active\(anon\)|Inactive\(anon\)|Active\(file\)|Inactive\(file\)|Unevictable|Mlocked|Dirty|Writeback|SwapTotal|SwapFree|Shmem|Mapped|Slab|SReclaimable|SUnreclaim|KernelStack|PageTables):/ { printf "  %-16s %10s kB\n", $1, $2 }' "$MEMINFO_FILE"

if [ "$LIVE" = 1 ]; then rm -f "$A" "$B"; fi
