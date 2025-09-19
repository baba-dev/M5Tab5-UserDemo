#
# SPDX-License-Identifier: MIT

import subprocess
import unittest
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]
BUILD_DIR = PROJECT_ROOT / "build" / "desktop"


def _run(cmd: list[str]) -> None:
    subprocess.run(cmd, check=True, cwd=PROJECT_ROOT)


def _ensure_build() -> None:
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    _run(["cmake", "-S", str(PROJECT_ROOT), "-B", str(BUILD_DIR)])
    _run(["cmake", "--build", str(BUILD_DIR), "--target", "media_page_test"])


class MediaPageTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        _ensure_build()

    def test_media_events(self) -> None:
        _run([str(BUILD_DIR / "media_page_test")])


if __name__ == "__main__":
    unittest.main()
