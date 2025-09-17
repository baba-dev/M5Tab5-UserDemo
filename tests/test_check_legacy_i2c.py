"""Tests for the legacy I2C detection helper."""

from __future__ import annotations

import importlib.util
from pathlib import Path
from tempfile import TemporaryDirectory

import sys
import unittest


MODULE_PATH = Path(__file__).resolve().parents[1] / "scripts" / "check_legacy_i2c.py"


def _load_module():
    spec = importlib.util.spec_from_file_location("check_legacy_i2c", MODULE_PATH)
    if spec is None or spec.loader is None:  # pragma: no cover - sanity guard
        raise RuntimeError("Unable to load check_legacy_i2c module")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


class CheckLegacyI2CTests(unittest.TestCase):
    def setUp(self) -> None:
        self.module = _load_module()

    def test_detects_legacy_include(self) -> None:
        with TemporaryDirectory() as tmpdir:
            src_path = Path(tmpdir) / "legacy.c"
            src_path.write_text('#include "driver/i2c.h"\n', encoding="utf-8")

            result = self.module.scan_paths([Path(tmpdir)])

            self.assertEqual(result.files_scanned, 1)
            self.assertEqual(len(result.findings), 1)
            finding = result.findings[0]
            self.assertEqual(finding.path, src_path)
            self.assertIn("legacy driver include", finding.reason)

    def test_allows_ng_driver_helpers(self) -> None:
        with TemporaryDirectory() as tmpdir:
            src_path = Path(tmpdir) / "ng_driver.c"
            src_path.write_text(
                (
                    '#include "driver/i2c_master.h"\n'
                    "void init_bus(void) {\n"
                    "    i2c_master_bus_add_device(0, 0, 0);\n"
                    "}\n"
                ),
                encoding="utf-8",
            )

            result = self.module.scan_paths([Path(tmpdir)])

            self.assertEqual(result.files_scanned, 1)
            self.assertFalse(result.findings)

    def test_detects_legacy_helpers(self) -> None:
        with TemporaryDirectory() as tmpdir:
            src_path = Path(tmpdir) / "legacy_helpers.c"
            src_path.write_text(
                (
                    "#include \"driver/i2c_master.h\"\n"
                    "void start(void) {\n"
                    "    i2c_master_cmd_begin(0, 0, 0);\n"
                    "}\n"
                ),
                encoding="utf-8",
            )

            result = self.module.scan_paths([Path(tmpdir)])

            self.assertEqual(result.files_scanned, 1)
            self.assertEqual(len(result.findings), 1)
            finding = result.findings[0]
            self.assertIn("legacy i2c_master_cmd_begin", finding.reason)


if __name__ == "__main__":  # pragma: no cover - manual execution
    unittest.main()
