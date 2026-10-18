#!/usr/bin/env python3
"""Simulate HALMAC TX FIFO page allocation + queue mapping for RTL8822BU (USB).

Re-implements, step by step, the arithmetic of:
  set_trx_fifo_info_8822b()   hal/halmac/halmac_88xx/halmac_8822b/halmac_init_8822b.c:686-748
  pg_num_parser_88xx()        hal/halmac/halmac_88xx/halmac_init_88xx.c:884-936
  rqpn_parser_88xx()          hal/halmac/halmac_88xx/halmac_init_88xx.c:939-966
  txdma_queue_mapping_8822b() halmac_init_8822b.c:519-561
  priority_queue_cfg_8822b()  halmac_init_8822b.c:563-683
and prints the register values that the driver writes. Constants are copied from
the source (file:line next to each). Nothing here is measured on hardware.

Usage: py -I txff_sim.py [bulkout_num=3] [drv_rsvd_pages=16] [mode=NORMAL|WMM]
"""
import sys

bulkout = int(sys.argv[1]) if len(sys.argv) > 1 else 3
drv_rsvd = int(sys.argv[2]) if len(sys.argv) > 2 else 16
mode = sys.argv[3] if len(sys.argv) > 3 else 'NORMAL'

TX_FIFO_SIZE_8822B = 262144          # halmac_8822b_cfg.h:66
RX_FIFO_SIZE_8822B = 24576           # halmac_8822b_cfg.h:67
TX_PAGE_SIZE_SHIFT_88XX = 7          # halmac_88xx_cfg.h:66 (128-byte pages)
C2H_PKT_BUF_88XX = 256               # halmac_88xx_cfg.h:77
RSVD = dict(H2C_EXTRAINFO=24, H2C_STATICINFO=8, H2CQ=8, CPU_INSTRUCTION=0,
            FW_TXBUF=4, CSIBUF=0)    # halmac_init_8822b.c:81-86
BLK_DESC_NUM = 3                     # halmac_init_8822b.c:94
DMA = dict(EXQ=0, LQ=1, NQ=2, HQ=3)  # enum halmac_dma_mapping, halmac_type.h:633-636

# halmac_init_8822b.c:220-286 : {mode: (vo, vi, be, bk, mg, hi)}
RQPN = {
    2: {'NORMAL': ('NQ', 'NQ', 'NQ', 'HQ', 'HQ', 'HQ'), 'WMM': ('NQ', 'NQ', 'NQ', 'HQ', 'HQ', 'HQ')},
    3: {'NORMAL': ('NQ', 'NQ', 'LQ', 'LQ', 'HQ', 'HQ'), 'WMM': ('HQ', 'NQ', 'LQ', 'NQ', 'HQ', 'HQ')},
    4: {'NORMAL': ('NQ', 'NQ', 'LQ', 'LQ', 'EXQ', 'HQ'), 'WMM': ('HQ', 'NQ', 'LQ', 'NQ', 'EXQ', 'HQ')},
}
# halmac_init_8822b.c:324-354 : {mode: (hq, nq, lq, exq, gap)}
PGNUM = {
    2: {'NORMAL': (64, 64, 0, 0, 1), 'WMM': (64, 64, 0, 0, 1)},
    3: {'NORMAL': (64, 64, 64, 0, 1), 'WMM': (256, 256, 256, 0, 1)},
    4: {'NORMAL': (64, 64, 64, 64, 1), 'WMM': (64, 64, 64, 64, 1)},
}

def bucket(n):                       # _rsvd_page_num_drv2halmac, hal_halmac.c:2840-2861
    for b in (8, 16, 24, 32, 64, 128):
        if n <= b:
            return b
    return 256

drv = bucket(drv_rsvd)
print(f'# bulkout_num={bulkout}, mode={mode}, driver asks {drv_rsvd} rsvd pages -> halmac allocates {drv}')

tx_pg = TX_FIFO_SIZE_8822B >> TX_PAGE_SIZE_SHIFT_88XX
rsvd = drv + sum(RSVD.values())
acq = tx_pg - rsvd
boundary = tx_pg - rsvd
print(f'tx_fifo_pg_num = {TX_FIFO_SIZE_8822B} >> 7 = {tx_pg} pages')
print(f'rsvd_pg_num    = drv {drv} + h2c_extra 24 + h2c_static 8 + h2cq 8 + cpu 0 + fw_txbuf 4 + csi 0 = {rsvd}')
print(f'acq_pg_num = rsvd_boundary = {tx_pg} - {rsvd} = {acq} (0x{boundary:03X})')

# reserved-area layout, top of FIFO downwards (halmac_init_8822b.c:725-739)
addr = tx_pg
layout = []
for name, n in (('csibuf', RSVD['CSIBUF']), ('fw_txbuf', RSVD['FW_TXBUF']),
                ('cpu_instr', RSVD['CPU_INSTRUCTION']), ('h2cq', RSVD['H2CQ']),
                ('h2c_sta_info', RSVD['H2C_STATICINFO']), ('h2c_info', RSVD['H2C_EXTRAINFO']),
                ('drv', drv)):
    addr -= n
    layout.append((name, addr, n))
print('reserved area (page index, size):')
for name, a, n in layout:
    print(f'   rsvd_{name:13s} @ page {a:4d} (0x{a:03X})  x{n}')
assert layout[-1][1] == boundary, 'rsvd_boundary != rsvd_drv_addr -> HALMAC_RET_CFG_TXFIFO_PAGE_FAIL'

hq, nq, lq, exq, gap = PGNUM[bulkout][mode]
pub = acq - hq - lq - nq - exq - gap
print(f'queue pages: HQ {hq}, NQ {nq}, LQ {lq}, EXQ {exq}, gap {gap}, PUB = {acq}-{hq}-{lq}-{nq}-{exq}-{gap} = {pub}')

vo, vi, be, bk, mg, hi = RQPN[bulkout][mode]
pq = (DMA[hi] << 14) | (DMA[mg] << 12) | (DMA[bk] << 10) | (DMA[be] << 8) | (DMA[vi] << 6) | (DMA[vo] << 4)
print(f'TXDMA_PQ_MAP: VO->{vo} VI->{vi} BE->{be} BK->{bk} MG->{mg} HI->{hi}')

rxff_bndy = RX_FIFO_SIZE_8822B - C2H_PKT_BUF_88XX - 1
print('\nregister writes (in driver order):')
rows = [
    ('0x010C', 'REG_TXDMA_PQ_MAP (W16)', pq, 'txdma_queue_mapping_8822b :558; bits 1:0 kept 0 here, later |= BIT0 (USB) / RXDMA_AGG_EN'),
    ('0x0230', 'REG_FIFOPAGE_INFO_1 (W16)', hq, 'high queue pages :603'),
    ('0x0234', 'REG_FIFOPAGE_INFO_2 (W16)', lq, 'low :604'),
    ('0x0238', 'REG_FIFOPAGE_INFO_3 (W16)', nq, 'normal :605'),
    ('0x023C', 'REG_FIFOPAGE_INFO_4 (W16)', exq, 'extra :606'),
    ('0x0240', 'REG_FIFOPAGE_INFO_5 (W16)', pub, 'public :607'),
    ('0x022C', 'REG_RQPN_CTRL_2 |= BIT31', 1 << 31, 'load page numbers :608'),
    ('0x0204', 'REG_FIFOPAGE_CTRL_2 (W16)', boundary, 'BCN_HEAD = rsvd_boundary :616'),
    ('0x0422', 'REG_FWHW_TXQ_CTRL+2 |= BIT4', 1 << 4, ':617'),
    ('0x0424', 'REG_BCNQ_BDNY_V1 (W16)', boundary, ':629'),
    ('0x0206', 'REG_FIFOPAGE_CTRL_2+2 (W16)', boundary, 'BCN_HEAD_1 :632'),
    ('0x0456', 'REG_BCNQ1_BDNY_V1 (W16)', boundary, ':644'),
    ('0x011C', 'REG_RXFF_BNDY (W32)', rxff_bndy, f'{RX_FIFO_SIZE_8822B} - {C2H_PKT_BUF_88XX} - 1 :647'),
    ('0x0208', 'REG_AUTO_LLT_V1 [7:4]', BLK_DESC_NUM << 4, 'BLK_DESC_NUM (USB) :652-655'),
    ('0x020B', 'REG_AUTO_LLT_V1+3 (W8)', BLK_DESC_NUM, ':657'),
    ('0x020D', 'REG_TXDMA_OFFSET_CHK+1 |= BIT1', 1 << 1, ':658'),
    ('0x0208', 'REG_AUTO_LLT_V1 |= AUTO_INIT_LLT', 1, 'then poll bit0 -> 0 (<=1000 reads) :661-667'),
    ('0x0103', 'REG_CR+3 (W8)', 0, 'transfer_mode NORMAL :680 (assumes HALMAC_TRNSFER_NORMAL == 0)'),
]
for a, n, v, why in rows:
    print(f'  {a}  {n:34s} = 0x{v:X} ({v})   {why}')
print(f'\nTX buffer per queue (bytes): HQ {hq*128}, NQ {nq*128}, LQ {lq*128}, EXQ {exq*128}, PUB {pub*128}')
