#!/usr/bin/env python3
import argparse
import re
import sys
import time

try:
    import serial
except ImportError:
    print("pyserial not installed. pip install pyserial", file=sys.stderr)
    sys.exit(2)

PATTERNS = [
    re.compile(rb"rst:0x", re.I),
    re.compile(rb"panic", re.I),
    re.compile(rb"abort", re.I),
    re.compile(rb"Guru Meditation", re.I),  # legacy wording still appears on some targets
    re.compile(rb"watchdog", re.I),
]

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", required=True, help="Serial port (e.g. COM5 or /dev/ttyACM0)")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--seconds", type=int, default=30)
    args = ap.parse_args()

    resets = 0
    start = time.time()

    try:
        with serial.Serial(args.port, args.baud, timeout=0.2) as ser:
            ser.reset_input_buffer()
            while time.time() - start < args.seconds:
                data = ser.read(8192)
                if not data:
                    continue
                sys.stdout.buffer.write(data)
                sys.stdout.flush()
                for pat in PATTERNS:
                    if pat.search(data):
                        if b"rst:" in data:
                            resets += 1
                        else:
                            print("\n[bootloop_guard] panic/abort/watchdog matched -> FAIL", file=sys.stderr)
                            return 1
                if resets > 3:
                    print("\n[bootloop_guard] too many resets -> FAIL", file=sys.stderr)
                    return 1
    except serial.SerialException as exc:
        print(f"[bootloop_guard] Serial error: {exc}", file=sys.stderr)
        return 2

    print("\n[bootloop_guard] NO_BOOTLOOP", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
