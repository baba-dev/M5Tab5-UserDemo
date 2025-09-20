# deps: pytest, pytest-embedded, pytest-embedded-serial, pytest-embedded-idf
# Detects reboot loops using only standard boot/panic text patterns.

import re
import time

# Common boot banners from ROM/bootloader
BOOT_MARKERS = [
    re.compile(r"\bESP-ROM:", re.I),
    re.compile(r"\brst:0x[0-9a-f]+\b", re.I),          # e.g. rst:0xc (SW_CPU_RESET)
    re.compile(r"\bboot:0x[0-9a-f]+\b", re.I),         # e.g. boot:0x20c
    re.compile(r"\bboot:\s+ESP-IDF\b", re.I),          # "I (...) boot: ESP-IDF v..."
    re.compile(r"\bentry 0x[0-9a-f]+\b", re.I),
]

# Common fatal/panic signatures
PANIC_MARKERS = [
    re.compile(r"\bBacktrace:\b", re.I),
    re.compile(r"\babort\(\)\s+was\s+called\b", re.I),
    re.compile(r"\bGuru Meditation Error\b", re.I),
    re.compile(r"\bWDT\b", re.I),                      # watchdog resets
]

def _has_any(line: str, pats) -> bool:
    return any(p.search(line) for p in pats)

def test_boots_once_and_does_not_loop(dut):
    """
    Procedure:
      1) Let pytest-embedded flash and start the DUT.
      2) Consume serial until we see the first complete boot sequence.
      3) Then watch for 30s; if any boot markers or panic markers reappear, fail.
    """
    # Phase 1: consume initial boot (first marker set must appear)
    saw_first_boot = False
    t0 = time.time()
    while time.time() - t0 < 15.0:  # up to 15s to get through bootloader->app
        buf = dut.read(timeout=1.0) or b""
        line = buf.decode(errors="ignore")
        if _has_any(line, BOOT_MARKERS):
            saw_first_boot = True
            # keep consuming until boot banners quiet down for a moment
            # (heuristic) break once we hit a lull
        if saw_first_boot and not line.strip():
            break
    assert saw_first_boot, "Never observed initial ESP-IDF/ROM boot markers on UART"

    # Phase 2: verify no reboots/panics for 30s
    reboots = 0
    panics  = 0
    t1 = time.time()
    while time.time() - t1 < 30.0:
        buf = dut.read(timeout=1.0) or b""
        line = buf.decode(errors="ignore")
        if _has_any(line, PANIC_MARKERS):
            panics += 1
            raise AssertionError(f"Panic detected: {line.strip()[:200]}")
        if _has_any(line, BOOT_MARKERS):
            reboots += 1
            raise AssertionError(f"Reboot/boot-loop detected (marker seen again): {line.strip()[:200]}")

    # If we reach here, the app stayed up without printing boot/panic markers again.
