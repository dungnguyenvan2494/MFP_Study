# PS-CPU Reverse-Engineering — Session Context / Handoff Notes

**Purpose of this file:** not firmware documentation (that's `00_project_scope.md`/`01_repository_map.md`/`02_architecture.md`) — this is a record of *how the work got done*, so a future session (or a different person) can resume without re-deriving everything from scratch. Written at the end of a multi-hour session on 2026-09-05.

---

## 1. What this session produced

| File | Commit | Status |
|---|---|---|
| `PS-CPU/PS-CPU-Tong-hop.html` | `c50aaa6` | Committed to `main` (earlier session, consolidated HTML digest of pre-existing docs) |
| `PS-CPU/docs/00_project_scope.md` | `10d01fe`, merged `8554904` | Committed to `main` |
| `PS-CPU/docs/01_repository_map.md` | `afe3338`, merged `36aa6ca` | Committed to `main` |
| `PS-CPU/docs/02_architecture.md` | `21ddc8f` | Pushed to `claude/ps-cpu-docs-html-export-df9rbn` only — **not yet merged to `main`** (user hasn't asked for that merge yet) |
| `PS-CPU/docs/SESSION_CONTEXT.md` (this file) | — | being written now |

**Branch:** `claude/ps-cpu-docs-html-export-df9rbn` (the designated dev branch for this repo per the session's branch requirements). `main` currently has `00_project_scope.md` and `01_repository_map.md` merged in; `02_architecture.md` is one `git merge`/push away if the user asks.

**Repo:** `dungnguyenvan2494/MFP_Study` (public GitHub repo — see confidentiality note in §5).

---

## 2. The core problem this session solved: where the real source lives

`MFP_Study` (this git repo) is a **documentation vault** — Markdown notes, PDFs, an HTML digest. It does **not** contain the actual PS-CPU firmware source tree. The real source (`pscpu_s800/`) lives in **Google Drive**, folder `PS-CPU/pscpu_s800/` (owner `dung.nv172494@gmail.com`, folder ID `12we0uZa7c5PhZmRQi5Du9icNU5aKwuMF`), accessed via the `mcp__Google_Drive__search_files` / `download_file_content` MCP tools.

**Duplicate-folder trap:** the Drive folder contains the same tree uploaded 2-3 times (sibling folders/files with identical names, some Drive-suffixed differently). All docs this session consistently cite **one self-consistent batch**: the folders/files with `createdTime` clustered around **2026-09-04T17:44:34Z–17:44:43Z** for `main/App`, `main/Inc`, `iap/App`, `iap/Inc`, and `common/BSP`; a *separate* batch at `17:44:38-39Z` turned out to be the canonical **`iap/Src`** folder (confirmed by exact byte-size match against sizes recorded in the pre-compaction summary: `mx_init.c`=8901B, `stm32f0xx_it.c`=5653B, `stm32f0xx_hal_msp.c`=5313B); a *third* batch at `17:44:42-43Z` is the canonical **`main/Src`** folder. A fourth batch at `17:45:2x-3x` exists and was deliberately **not** used (later/duplicate upload). **If you re-open this Drive folder, do not assume any file returned by a plain title search is the "right" one — always cross-check `createdTime` against this cluster, or better, reuse the exact file IDs listed in §4 below.**

---

## 3. Documentation conventions established (apply to any future doc in this series)

- **Evidence tagging:** every claim is either backed by a specific file/line actually read (stated as plain fact, no tag), or explicitly marked `[INFERRED]` (reasonable deduction, not directly verified), `[HARDWARE ASSUMPTION]` (taken from the ST datasheet/reference manual, not re-derived), or `[UNKNOWN]` (source couldn't settle it). **Never silently invent behavior.**
- **Citation format for `02_architecture.md`-style docs:** every non-trivial statement carries `file:line` (exact, not estimated — see §4's extraction method for how "exact" was achieved).
- **Confidentiality:** `Document/PS-CPU-Manual.pdf` in this repo is stamped "Konica Minolta Confidential"; `MFP_Study` is a **public** repo. All docs so far reproduce only short (≤10 line) illustrative excerpts with citations, never full files. Keep doing that.
- **Numbered doc series so far:** `00_project_scope.md` (what/why/build-system), `01_repository_map.md` (module inventory, 12-way classification, per-file table, dependency matrix), `02_architecture.md` (layer model, dependency graph, ownership tables, architectural violations). A natural `03_*` would be something like a sequence-diagram/flow doc or a security review — not started, not requested yet.

---

## 4. Critical technique: how to get byte-exact source with reliable line numbers (read this before repeating the work)

**Problem discovered this session:** `mcp__Google_Drive__download_file_content` returns file content as a base64 string inline in the tool result. For a large file (`km_extend_io.c`, 1540 lines) the harness auto-saves the tool result to a local file when it's too big to inline — but for medium files (a few KB to ~30 KB source), the base64 comes back inline in the conversation, and **hand-retyping/reproducing that base64 into a `Write` call is unreliable** — this session corrupted one file (`km_extend_io.h`) by truncating a long base64 blob mid-transcription while trying to copy it by hand into a `Write` tool call.

**The fix that worked, and should be reused every time:** the Claude Code CLI persists the *entire* session transcript (every tool call and result, including the full inline base64) as a JSONL file on disk at:
```
/root/.claude/projects/-home-user-MFP-Study/<session-id>.jsonl
```
(session id for this session: `a3200f46-fbe0-5fd1-be3d-348fb3783361` — a *different* session will have a different filename; find it via `ls /root/.claude/projects/-home-user-MFP-Study/*.jsonl` or check `total_tokens`/session metadata).

After calling `download_file_content` for a batch of files (their base64 now sitting somewhere in that JSONL), run a Python script like this to extract and decode them **programmatically** (no hand-copying, no truncation risk):

```python
import json, base64, os

path = "/root/.claude/projects/-home-user-MFP-Study/<session-id>.jsonl"
id_to_path = {
    "<drive-file-id-1>": "main/App/foo.c",
    "<drive-file-id-2>": "main/Inc/foo.h",
    # ...
}
found = {}
def scan_obj(obj):
    if isinstance(obj, dict):
        if "id" in obj and "content" in obj and obj.get("id") in id_to_path:
            found[obj["id"]] = obj["content"]
        for v in obj.values(): scan_obj(v)
    elif isinstance(obj, list):
        for v in obj: scan_obj(v)

with open(path, "r", errors="ignore") as f:
    for line in f:
        if not any(fid in line for fid in id_to_path):  # cheap pre-filter
            continue
        try: obj = json.loads(line.strip())
        except Exception: continue
        scan_obj(obj)

for fid, content in found.items():
    outpath = os.path.join("/tmp/pscpu", id_to_path[fid])
    os.makedirs(os.path.dirname(outpath), exist_ok=True)
    with open(outpath, "wb") as out:
        out.write(base64.b64decode(content))
```

Then verify byte-for-byte correctness by comparing decoded file size against the Drive metadata's `fileSize` field (returned by `search_files`) — every file this session decoded this way matched exactly (e.g. `km_i2c.c` 30,609 B, `km_it.c` 17,741 B, `mx_init.c` 19,271 B/8,901 B for Main/IAP, etc.). **Only after this size match should you trust `grep -n`/line numbers from the file for citations.**

This is dramatically more reliable than: (a) hand-copying base64 into a `Write` call, or (b) trusting `search_files`'s `contentSnippet` field, which **silently truncates** long files (confirmed truncated for `entry.c` and `km_i2c.c` when their content exceeded roughly a few KB — it cut off mid-function with a bare `...`).

---

## 5. Local decoded source tree — ephemeral, will NOT survive session end

This session built a local, byte-verified, line-numbered decoded copy of 24 firmware source files at:
```
/tmp/pscpu/
├── main/{App,Inc,Src}/*.c,*.h
├── iap/{App,Inc,Src}/*.c,*.h
└── common/BSP/BSP_STM32F03x_Nucleo.h
```
**This directory is scratch space (`/tmp`) and will be gone in a fresh session/container.** It is not committed to git (correctly — it would violate the confidentiality posture in §3, since these are full proprietary source files, not short excerpts). If a future session needs to re-derive exact line numbers for further citations, it must **re-download and re-decode from Drive** using the technique in §4. To make that fast, here is the full manifest of Drive file IDs used this session (all from the canonical batch described in §2):

| Local path | Drive file ID | Size (bytes) |
|---|---|---|
| `main/App/entry.c` | `1wvL1QVUtRfLxDDKqxbkw8d9SWbYRUvRl` | 6,149 |
| `main/App/km_i2c.c` | `1Gq0qlw7gf-iVEe7VE-rXQE_-zIlaQceb` | 30,609 |
| `main/App/km_extend_io.c` | (downloaded pre-compaction; re-search by title if needed) | 65,626 |
| `main/App/km_it.c` | `1OABakb9uv5QF81wLhJmaR6PfpWBzW8pc` | 17,741 |
| `main/App/km_alarm_wake.c` | `1TsLBg6g2Z3PLTKd92EQce6SYl3FM_LAJ` | 3,439 |
| `main/App/km_ca72_status.c` | `1jg-y6Fbjksqp6suaxireI7YJThAFeI9t` | 1,452 |
| `main/App/km_adc.c` | `1is8WFbcFOw2buIHvqx1XouBm03kxtDrP` | 2,268 |
| `main/Inc/km_extend_io.h` | `10gaKQRzghrhU9IPoiW3SGCHRaOetsUVK` | 4,729 |
| `main/Inc/km_i2c.h` | `1BfXuWY7myhFBh6E4szQCgpPMPSqkzsjH` | 9,465 |
| `main/Inc/km_it.h` | `1CLPTAMSQ_W6ZkhVsGklkBEoIENm8bVN1` | 4,163 |
| `main/Inc/mxconstants.h` | `17JFTcUP0cdZUjlFHe9dOrmSV93WbwxVV` | 4,459 |
| `main/Inc/km_ver.h` | `1evRz5WtraB4qStv-JNEFmPck1Fxguj-Z` | 1,526 |
| `main/Inc/entry.h` | `179ylBRc6T-XHH_9Mi95uN-TAaDBR193c` | 497 |
| `main/Src/mx_init.c` | `14CB7Rs7XOV1uXLKec6qBRPz-P672cm3V` | 19,271 |
| `main/Src/stm32f0xx_it.c` | `1cywPVdAfFhtR3x6P7tr0MGcJ17AThF3b` | 6,699 |
| `main/Src/stm32f0xx_hal_msp.c` | `1zM5FAdkfqtvEYlYDUUTAuwGXgIY4Y6XA` | 6,150 |
| `iap/App/entry_iap.c` | `15zdySW01uY95lH7sfXlFjK6uwhEUSsQh` | 2,523 |
| `iap/App/km_i2c_iap.c` | `1r0sia9VC4zAPtHJbaMBVjYajirFbPQyO` | 21,039 |
| `iap/App/km_extend_io_iap.c` | `1Ftbd6e207GGVXnY3ctkGS2CNMVqGH4Kv` | 2,801 |
| `iap/Inc/km_i2c_iap.h` | `1QiiNCdrZJ7z76WGVMyatU_j9Bg05xjkQ` | 6,432 |
| `iap/Inc/km_ver_iap.h` | `15VcZj5KOo59Glq_9oEvY0n9FK2M3KKvF` | 1,527 |
| `iap/Src/mx_init.c` | `1ETnWPRUAq1pH51CvvMpt-pfOF10dcyEu` | 8,901 |
| `iap/Src/stm32f0xx_it.c` | `1yl6BFCkpFwbUO3D4ZJEWnlcidaw7wd-j` | 5,653 |
| `iap/Src/stm32f0xx_hal_msp.c` | `1YqAl0y_VN0i-gL2FPqtbf6yEyUyfkQI5` | 5,313 |
| `common/BSP/BSP_STM32F03x_Nucleo.h` | `1A02fM74brg1VfHjfgGETYlmJfAj6Trid` | 4,413 |

**Not yet decoded/downloaded at all** (named/sized only, from directory listings): the 6 `BSP_*.c` implementation bodies (`BSP_GPIO_STM32F03x_Nucleo.c` 4488B, `BSP_TIM_...c` 5054B, `BSP_WDT_...c` 1390B, `BSP_PWR_...c` 1667B, `BSP_ADC_...c` 2390B — confirmed unused/dead, `BSP_Reg_...c` 1617B), the IAP-variant `mxconstants.h`/`stm32f0xx_hal_conf.h`/`stm32f0xx_it.h`/`system_stm32f0xx.c`, and everything under ST's `STM32F0xx_HAL_Driver`. These are the natural next targets if deeper citation precision is ever needed for those areas.

---

## 6. Key findings worth remembering (so they aren't re-derived)

- **No RTOS, no middleware.** Confirmed via both `.uvprojx` RTE component lists (only `ARM::CMSIS:CORE`) and absence of any `osKernelStart`/`xTaskCreate`-style call across all 24 files read. Both images (Main and IAP) are single-threaded bare-metal superloops.
- **Two firmware images share one 32KB flash on one STM32F031C6 chip:** Main at `0x08000000`/20KB, IAP at `0x08005000`/12KB (both confirmed via `.uvprojx` `OCR_RVCT4` settings).
- **`jump2iap()` is live code, not dead code** (a finding this session corrected from the prior pass): `main/App/km_i2c.c:686-693` calls `DeInit()` (`entry.c:186`) then `jump2iap()` (`entry.c:159`) when the main SoC I2C-writes magic number `0x11223344` to register `0x90` (`IAP_PG_COMMAND`). This is a **software jump**, not a hardware reset — IAP's own `entry_iap.c::main()` runs next without ever going through a chip-level reset.
- **The return path (IAP → Main) is different:** a hardware reset (`HAL_NVIC_SystemReset()`, `km_extend_io_iap.c:37`), triggered when `MSW_ON` goes low and the update-complete bit is set.
- **`IAP_ADDRESS` resolves cleanly to `0x08005000`** (`iap/Inc/km_i2c_iap.h:120`: `MAIN_APPLICATION_ADDRESS + MAIN_APPLICATION_SIZE`), despite a stale/wrong comment in `entry_iap.c:56` claiming `0x08001600` — the code is right, the comment is just outdated prose.
- **A genuine layering violation exists:** `km_extend_io.c::s800_power_off()` (lines 538-540) calls raw `NVIC_DisableIRQ()` directly on 3 EXTI lines, bypassing `km_it.c`'s own reference-counted `km_disable_irq()`/`km_enable_irq()` wrapper (`km_it.c:436-462`) that the rest of the codebase uses for the identical lines. Harmless in practice only because `s800_power_off()` ends in a full chip reset a few lines later.
- **RTC has split ownership:** `mx_init.c` configures it via HAL at boot; `km_i2c.c` reads/writes it via raw `RTC_BASE+offset` pointer casts at runtime (bypassing `HAL_RTC_GetTime`/`SetTime` entirely) because the I2C protocol *is* a register-address-to-value passthrough by design.
- **`io_extend[]` has exactly 14 entries** (`km_extend_io.c:50-65`, 8 output signals + 6 input signals) — a precise count not stated anywhere in the pre-existing secondary docs.
- **Model vs. Generation detection use unrelated mechanisms:** `get_model()` reads 2 GPIO straps once at boot and caches (immutable); `get_Generation()` reads a live, I2C-writable status bit (mutable at any time). Easy to conflate, so flagged explicitly in `02_architecture.md` Observation 4.
- **Three different I2C1-recovery code paths exist** (`i2c_sw_reset()` in `km_i2c.c`, a separate `i2c_reset()` in `km_extend_io.c` with no confirmed caller — `[UNKNOWN]`, and the boot-time `MX_I2C1_Init()`), each slightly different in scope (RCC-only force-reset vs. also calling `MX_I2C1_Init()` again).

---

## 7. Open items / natural next steps (not started, no commitment made)

- Merge `02_architecture.md` into `main` (currently only on the feature branch) — do this if/when the user asks, following the same conflict-resolution procedure used for the prior two docs (empty placeholder on `main`, resolve via `git checkout --theirs`, since `main` still carries a 1-byte placeholder for other files like `02_architecture.md` was never placeholder-scaffolded there — check `git show origin/main:PS-CPU/docs/02_architecture.md` first to see if a placeholder even exists before assuming a conflict).
- The empty placeholder skeleton on `main` (`03_*.md` through `10_error_recovery.md`, `diagrams/*`, `traceability/code_traceability.md`) is still 0 bytes for everything past `01_repository_map.md` — untouched this session.
- Reading the 6 `BSP_*.c` bodies would let `02_architecture.md`'s BSP-layer citations move from `[INFERRED]` to confirmed fact (right now, "BSP wraps HAL" is inferred from the header + call sites, not from reading the `.c` bodies).
- The `km_extend_io.c::i2c_reset()` no-caller-found question (§6, last bullet) could potentially be resolved by grep'ing files not yet read (BSP `.c` bodies, the third Keil test project under `common/Test/cubemx`).
- No diagrams have been generated yet (per earlier session instruction to hold off on diagrams) — if the user asks for visual sequence/state diagrams, `01_repository_map.md` and `02_architecture.md` between them have enough traced call chains (boot sequence, I2C dispatch, power-on/off sequences, Main↔IAP transition) to build them without re-reading source.
