#!/usr/bin/env python3
"""
swap_map_sim.py - mo hinh do choi (toy model) cua swap_map + per-CPU swap slots cache.

Muc dich: sinh ra cac bang trang thai dung trong docs/swap/ thay vi tinh tay.

TRUNG THANH voi source (Linux 5.10.241):
  - quy tac dem trong __swap_duplicate(), __swap_entry_free_locked(), swap_entry_free()
    (mm/swapfile.c:3473, :1238, :1362)
  - get_swap_pages() tru nr_swap_pages truoc, cong lai phan khong lay duoc (mm/swapfile.c:1056)
  - get_swap_page()/free_swap_slot()/refill/drain/check_cache_active() (mm/swap_slots.c)
  - slot nam trong slots_ret[] van mang gia tri SWAP_HAS_CACHE trong swap_map
DON GIAN HOA (khong giong source):
  - tim slot = first-free tuyen tinh tu cluster_next (source co thuat toan cluster/SSD phuc tap hon)
  - SWAP_SLOTS_CACHE_SIZE thu nho (real = 64) de bang ngan
  - khong mo phong I/O, page lock, memcg, continuation (count > 0x3e)
Chay:  python swap_map_sim.py > swap_map_sim.out.txt
"""

SWAP_HAS_CACHE = 0x40
SWAP_MAP_BAD = 0x3F
EEXIST, ENOENT = "EEXIST", "ENOENT"


class SwapArea:
    def __init__(self, nslots):
        self.swap_map = [0] * nslots
        self.swap_map[0] = SWAP_MAP_BAD          # setup_swap_map_and_extents(): header page
        self.max = nslots
        self.pages = nslots - 1
        self.inuse_pages = 0
        self.nr_swap_pages = self.pages           # _enable_swap_info(): atomic_long_add(p->pages)
        self.cluster_next = 1

    # --- mm/swapfile.c ---
    def scan_slots(self, n):
        got = []
        off = self.cluster_next
        tried = 0
        while len(got) < n and tried < self.max:
            if off >= self.max:
                off = 1
            if self.swap_map[off] == 0:
                self.swap_map[off] = SWAP_HAS_CACHE   # scan_swap_map_slots(si, SWAP_HAS_CACHE, ...)
                self.inuse_pages += 1
                got.append(off)
            off += 1
            tried += 1
        self.cluster_next = off
        return got

    def get_swap_pages(self, n_goal):
        avail = self.nr_swap_pages
        if avail <= 0:
            return []
        n_goal = min(n_goal, 64, avail)
        self.nr_swap_pages -= n_goal                  # tru truoc
        got = self.scan_slots(n_goal)
        self.nr_swap_pages += n_goal - len(got)       # check_out: tra lai phan thieu
        return got

    def swap_duplicate(self, off, usage=1):
        cnt = self.swap_map[off]
        if cnt == SWAP_MAP_BAD:
            return ENOENT
        has = cnt & SWAP_HAS_CACHE
        cnt &= ~SWAP_HAS_CACHE
        err = 0
        if usage == SWAP_HAS_CACHE:
            if not has and cnt:
                has = SWAP_HAS_CACHE
            elif has:
                err = EEXIST
            else:
                err = ENOENT
        elif cnt or has:
            cnt += usage
        else:
            err = ENOENT
        self.swap_map[off] = cnt | has
        return err

    def entry_free_locked(self, off, usage):
        cnt = self.swap_map[off]
        has = cnt & SWAP_HAS_CACHE
        cnt &= ~SWAP_HAS_CACHE
        if usage == SWAP_HAS_CACHE:
            assert has
            has = 0
        else:
            cnt -= 1
        u = cnt | has
        self.swap_map[off] = u if u else SWAP_HAS_CACHE   # 0 -> giu SWAP_HAS_CACHE cho den khi tra slot
        return u

    def swap_entry_free(self, off):                        # goi tu swapcache_free_entries()
        assert self.swap_map[off] == SWAP_HAS_CACHE
        self.swap_map[off] = 0
        self.inuse_pages -= 1
        self.nr_swap_pages += 1                            # swap_range_free()

    def swapcache_free_entries(self, offs):
        for o in offs:
            self.swap_entry_free(o)


class SlotsCache:
    def __init__(self, cpu, size):
        self.cpu, self.size = cpu, size
        self.slots, self.slots_ret = [], []


class System:
    def __init__(self, nslots, ncpu, size, use_thresholds=True):
        self.si = SwapArea(nslots)
        self.size = size
        self.caches = [SlotsCache(c, size) for c in range(ncpu)]
        self.active = True
        self.use_thresholds = use_thresholds
        self.ncpu = ncpu

    # --- mm/swap_slots.c ---
    def check_cache_active(self):
        if not self.use_thresholds:
            return self.active
        pages = self.si.nr_swap_pages
        if not self.active:
            if pages > self.ncpu * 5 * self.size:
                self.active = True
        elif pages < self.ncpu * 2 * self.size:
            self.active = False
            self.drain_all()
        return self.active

    def drain_all(self):
        for c in self.caches:
            self.si.swapcache_free_entries(c.slots)
            c.slots = []
            self.si.swapcache_free_entries(c.slots_ret)
            c.slots_ret = []

    def get_swap_page(self, cpu):
        c = self.caches[cpu]
        if self.check_cache_active():
            if not c.slots:
                c.slots = self.si.get_swap_pages(self.size)   # refill_swap_slots_cache()
            if c.slots:
                return c.slots.pop(0)
        got = self.si.get_swap_pages(1)                         # duong truc tiep
        return got[0] if got else 0

    def free_swap_slot(self, cpu, off):
        c = self.caches[cpu]
        if self.active:
            if len(c.slots_ret) >= self.size:
                self.si.swapcache_free_entries(c.slots_ret)
                c.slots_ret = []
            c.slots_ret.append(off)
        else:
            self.si.swapcache_free_entries([off])

    # --- cac thao tac cap cao hon (goi y nghia, ten function that ghi trong comment) ---
    def swap_free(self, cpu, off):            # swap_free() -> __swap_entry_free()
        if not self.si.entry_free_locked(off, 1):
            self.free_swap_slot(cpu, off)

    def put_swap_page(self, cpu, off):        # put_swap_page(): bo SWAP_HAS_CACHE
        if not self.si.entry_free_locked(off, SWAP_HAS_CACHE):
            self.free_swap_slot(cpu, off)

    def state(self):
        m = " ".join(f"{v:02x}" for v in self.si.swap_map)
        cs = " | ".join(f"cpu{c.cpu}: slots={c.slots} ret={c.slots_ret}" for c in self.caches)
        return (f"swap_map=[{m}]  nr_swap_pages={self.si.nr_swap_pages} "
                f"inuse={self.si.inuse_pages}\n{'':6}{cs}")


def step(sys_, i, text):
    print(f"[{i:>2}] {text}")
    print("      " + sys_.state())


def scenario_lifecycle():
    print("\n" + "=" * 78)
    print("KICH BAN 1: vong doi slot cua page A (1 process, 1 CPU, cache size=4, 8 slot)")
    print("=" * 78)
    s = System(nslots=8, ncpu=1, size=4, use_thresholds=False)
    step(s, 0, "swapon: slot 0 = header (0x3f), 7 slot trong")
    e = s.get_swap_page(0)
    step(s, 1, f"reclaim A: add_to_swap() -> get_swap_page(): refill lay 4 slot, A nhan slot {e}")
    step(s, 2, "add_to_swap_cache(A, entry): swap_map khong doi; A vao swap cache, PageDirty")
    s.si.swap_duplicate(e)
    step(s, 3, "try_to_unmap(): PTE cua A -> swap PTE, swap_duplicate(): count 0 -> 1")
    step(s, 4, "pageout() -> swap_writepage(): ghi I/O (swap_map khong doi)")
    s.put_swap_page(0, e)
    step(s, 5, "__remove_mapping(): xoa khoi swap cache + put_swap_page(): bo HAS_CACHE -> page frame duoc free")
    r = s.si.swap_duplicate(e, SWAP_HAS_CACHE)
    step(s, 6, f"fault: do_swap_page() miss -> __read_swap_cache_async(): swapcache_prepare() -> {r or 'OK'}")
    s.swap_free(0, e)
    step(s, 7, "do_swap_page(): set PTE, swap_free(): count 1 -> 0, CON HAS_CACHE -> slot chua duoc free")
    s.si.swap_duplicate(e)
    step(s, 8, "reclaim lai A (chua bi sua): da PageSwapCache -> bo qua add_to_swap; try_to_unmap(): count 1")
    s.put_swap_page(0, e)
    step(s, 9, "A sach -> khong pageout, __remove_mapping() ngay: KHONG CO I/O ghi lan 2")
    s.swap_free(0, e)
    step(s, 10, "process exit: zap_pte_range() -> free_swap_and_cache(): count -> 0 -> free_swap_slot() -> slots_ret")
    print("      (chu y: swap_map[e] van = 0x40 va nr_swap_pages CHUA tang -> slot chua cap phat lai duoc)")
    s.drain_all()
    step(s, 11, "drain (swapoff/deactivate) hoac slots_ret day -> swapcache_free_entries(): 0x40 -> 0x00")


def scenario_fork():
    print("\n" + "=" * 78)
    print("KICH BAN 2: 1 slot dung chung boi parent + child sau fork()")
    print("=" * 78)
    s = System(nslots=8, ncpu=1, size=4, use_thresholds=False)
    e = s.get_swap_page(0)
    s.si.swap_duplicate(e)
    s.put_swap_page(0, e)
    step(s, 1, f"page B da swap out xong: slot {e} count=1 (PTE cua parent)")
    s.si.swap_duplicate(e)
    step(s, 2, "fork(): copy_nonpresent_pte() -> swap_duplicate(): count 1 -> 2")
    s.si.swap_duplicate(e, SWAP_HAS_CACHE)
    step(s, 3, "child doc B: swapcache_prepare() set HAS_CACHE, doc I/O")
    s.swap_free(0, e)
    step(s, 4, "child: swap_free(): count 2 -> 1 (parent van tro toi slot)")
    s.swap_free(0, e)
    step(s, 5, "parent exit: free_swap_and_cache(): count 1 -> 0, con HAS_CACHE; "
               "__try_to_reclaim_swap(TTRS_UNMAPPED) that bai vi child dang map page")


def scenario_two_cpu_starvation():
    print("\n" + "=" * 78)
    print("KICH BAN 3: 2 CPU, cache KHONG co nguong deactivate (gia dinh) -> slot bi 'giam' trong cache")
    print("=" * 78)
    s = System(nslots=8, ncpu=2, size=4, use_thresholds=False)
    a = s.get_swap_page(0)
    step(s, 1, f"CPU0 can 1 slot: refill 4 slot, dung {a}")
    b = s.get_swap_page(1)
    step(s, 2, f"CPU1 can 1 slot: refill chi con 3 slot, dung {b}")
    c = s.get_swap_page(1)
    d = s.get_swap_page(1)
    e = s.get_swap_page(1)
    step(s, 3, f"CPU1 can them 3 slot: nhan {c}, {d}, {e} (0 = that bai)")
    print("      -> CPU1 that bai trong khi CPU0 con 3 slot chua dung trong cache:")
    print("         day la ly do check_cache_active() deactivate + drain khi swap con it.")


def scenario_thresholds():
    print("\n" + "=" * 78)
    print("KICH BAN 4: co nguong (2 CPU, size=4 -> deactivate < 16, reactivate > 40), 48 slot")
    print("=" * 78)
    s = System(nslots=48, ncpu=2, size=4, use_thresholds=True)
    prev_active = s.active
    for i in range(36):
        cpu = i % 2
        before = (s.si.nr_swap_pages, list(s.caches[0].slots), list(s.caches[1].slots))
        off = s.get_swap_page(cpu)
        changed = s.active != prev_active
        if i < 2 or changed or i == 35:
            mark = "  <-- check_cache_active(): deactivate + drain" if changed else ""
            print(f"  lan {i + 1:>2} CPU{cpu}: truoc nr_swap_pages={before[0]:>2} cpu0={before[1]} cpu1={before[2]}")
            print(f"          -> slot {off:>2}, sau nr_swap_pages={s.si.nr_swap_pages:>2} active={s.active} "
                  f"cpu0={s.caches[0].slots} cpu1={s.caches[1].slots}{mark}")
        prev_active = s.active
    print("  -> khi nr_swap_pages < 16: deactivate, drain slot thua ve pool, cap phat truc tiep tu global pool")


def scenario_race():
    print("\n" + "=" * 78)
    print("KICH BAN 5: vi sao phai khoa khi sua swap_map (xen ke do choi, KHONG co lock)")
    print("=" * 78)
    sm = [SWAP_MAP_BAD, 0x40, 0x40, 0, 0]
    print("  ban dau swap_map=[" + " ".join(f"{v:02x}" for v in sm) + "]")
    t0 = sm[3] == 0
    print(f"  CPU0: doc swap_map[3]==0 -> {t0}")
    t1 = sm[3] == 0
    print(f"  CPU1: doc swap_map[3]==0 -> {t1}")
    sm[3] = SWAP_HAS_CACHE
    print("  CPU0: ghi swap_map[3]=0x40, giao slot 3 cho page P")
    sm[3] = SWAP_HAS_CACHE
    print("  CPU1: ghi swap_map[3]=0x40, giao slot 3 cho page Q")
    print("  -> P va Q cung ghi vao cung 1 vi tri tren dia: du lieu mot trong hai bi mat.")
    print("  Source tranh dieu nay: kiem tra + ghi swap_map nam trong si->lock (va ci->lock voi SSD),")
    print("  xem scan_swap_map_slots(): lock_cluster() ... if (si->swap_map[offset]) ... WRITE_ONCE(...)")


if __name__ == "__main__":
    scenario_lifecycle()
    scenario_fork()
    scenario_two_cpu_starvation()
    scenario_thresholds()
    scenario_race()
