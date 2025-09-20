"""Rooms page UI regression tests and snapshot harness."""

# SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
#
# SPDX-License-Identifier: MIT

import os
import shutil
import struct
import subprocess
import unittest
from pathlib import Path
import zlib

PROJECT_ROOT = Path(__file__).resolve().parents[2]
BUILD_DIR = PROJECT_ROOT / "build" / "desktop"
OUTPUT_DIR = PROJECT_ROOT / "tests" / "ui" / "output"
GOLDEN_DIR = PROJECT_ROOT / "tests" / "ui" / "golden"
RAW_SNAPSHOT = OUTPUT_DIR / "rooms_three_cards.raw"
PNG_SNAPSHOT = OUTPUT_DIR / "rooms_three_cards.png"
GOLDEN_SNAPSHOT = GOLDEN_DIR / "rooms_three_cards.png"

WIDTH = 1280
HEIGHT = 720


def _run(cmd: list[str]) -> None:
    subprocess.run(cmd, check=True, cwd=PROJECT_ROOT)


def _ensure_build() -> None:
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    _run(["cmake", "-S", str(PROJECT_ROOT), "-B", str(BUILD_DIR)])
    _run(["cmake", "--build", str(BUILD_DIR), "--target", "rooms_page_test", "rooms_page_snapshot"])


def _rgb565_to_png(raw_path: Path, png_path: Path) -> None:
    data = raw_path.read_bytes()
    expected_len = WIDTH * HEIGHT * 2
    if len(data) != expected_len:
        raise ValueError(f"Unexpected raw size: {len(data)} (expected {expected_len})")

    rows: list[bytes] = []
    for y in range(HEIGHT):
        row = bytearray()
        row.append(0)  # no filter
        base = y * WIDTH * 2
        for x in range(WIDTH):
            idx = base + x * 2
            value = data[idx] | (data[idx + 1] << 8)
            r = ((value >> 11) & 0x1F) * 255 // 31
            g = ((value >> 5) & 0x3F) * 255 // 63
            b = (value & 0x1F) * 255 // 31
            row.extend((r, g, b))
        rows.append(bytes(row))

    png_data = b"".join(rows)

    with png_path.open("wb") as fh:
        fh.write(b"\x89PNG\r\n\x1a\n")
        ihdr = struct.pack(">IIBBBBB", WIDTH, HEIGHT, 8, 2, 0, 0, 0)
        fh.write(struct.pack(">I", len(ihdr)))
        fh.write(b"IHDR")
        fh.write(ihdr)
        fh.write(struct.pack(">I", zlib.crc32(b"IHDR" + ihdr) & 0xFFFFFFFF))

        compressed = zlib.compress(png_data)
        fh.write(struct.pack(">I", len(compressed)))
        fh.write(b"IDAT")
        fh.write(compressed)
        fh.write(struct.pack(">I", zlib.crc32(b"IDAT" + compressed) & 0xFFFFFFFF))

        fh.write(struct.pack(">I", 0))
        fh.write(b"IEND")
        fh.write(struct.pack(">I", zlib.crc32(b"IEND") & 0xFFFFFFFF))


class RoomsPageTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        _ensure_build()
        OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
        GOLDEN_DIR.mkdir(parents=True, exist_ok=True)

    def test_toggle_events(self) -> None:
        _run([str(BUILD_DIR / "rooms_page_test")])

    def test_snapshot_matches_golden(self) -> None:
        if RAW_SNAPSHOT.exists():
            RAW_SNAPSHOT.unlink()
        if PNG_SNAPSHOT.exists():
            PNG_SNAPSHOT.unlink()

        _run([str(BUILD_DIR / "rooms_page_snapshot")])
        self.assertTrue(RAW_SNAPSHOT.exists(), "Snapshot generator did not create raw output")

        _rgb565_to_png(RAW_SNAPSHOT, PNG_SNAPSHOT)

        if os.getenv("UPDATE_GOLDEN"):
            GOLDEN_SNAPSHOT.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(PNG_SNAPSHOT, GOLDEN_SNAPSHOT)

        if not GOLDEN_SNAPSHOT.exists():
            self.fail(
                "Golden snapshot missing. Run scripts/update_goldens.sh to refresh the reference image."
            )

        generated = PNG_SNAPSHOT.read_bytes()
        golden = GOLDEN_SNAPSHOT.read_bytes()
        self.assertEqual(golden, generated, "Snapshot PNG does not match golden reference")


if __name__ == "__main__":
    unittest.main()
