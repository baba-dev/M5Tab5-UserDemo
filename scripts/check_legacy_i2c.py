#!/usr/bin/env python3
"""Detect legacy ESP-IDF I2C driver usage in the source tree.

This script scans the repository for includes of ``driver/i2c.h`` and for calls
to helpers that belong to the deprecated "legacy" I2C driver. Mixing those
helpers with the NG driver will trigger runtime conflicts on firmware boot, so
CI blocks them from landing in ``main``.

Run the script from the repository root::

    python3 scripts/check_legacy_i2c.py

The exit status is non-zero when a forbidden pattern is found, making it easy
to wire the script into pre-commit hooks or CI jobs.
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Iterator, Sequence


REPO_ROOT = Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class LegacyPattern:
    """Representation of a legacy usage that should be flagged."""

    regex: re.Pattern[str]
    description: str


@dataclass(frozen=True)
class Finding:
    """A single match of a legacy API usage."""

    path: Path
    line_no: int
    reason: str
    line: str


@dataclass(frozen=True)
class ScanResult:
    """Aggregated result of scanning one or more paths."""

    files_scanned: int
    findings: list[Finding]


SOURCE_SUFFIXES: frozenset[str] = frozenset(
    {
        ".c",
        ".cc",
        ".cpp",
        ".cxx",
        ".h",
        ".hh",
        ".hpp",
        ".hxx",
        ".ipp",
    }
)

EXCLUDED_DIRS: frozenset[str] = frozenset(
    {
        ".git",
        "build",
        "cmake-build-debug",
        "cmake-build-release",
        "managed_components",
        "__pycache__",
        ".venv",
    }
)

KCONFIG_PREFIX: str = "sdkconfig"

LEGACY_PATTERNS: tuple[LegacyPattern, ...] = (
    LegacyPattern(
        re.compile(r"#\s*include\s*[\"<]driver/i2c\.h[\">]"),
        "legacy driver include",
    ),
    LegacyPattern(
        re.compile(r"\bi2c_param_config\b"),
        "legacy i2c_param_config helper",
    ),
    LegacyPattern(
        re.compile(r"\bi2c_driver_install\b"),
        "legacy i2c_driver_install helper",
    ),
    LegacyPattern(
        re.compile(r"\bi2c_driver_delete\b"),
        "legacy i2c_driver_delete helper",
    ),
    LegacyPattern(
        re.compile(r"\bi2c_cmd_link_create(?:_static)?\b"),
        "legacy I2C command link creation",
    ),
    LegacyPattern(
        re.compile(r"\bi2c_cmd_link_delete(?:_static)?\b"),
        "legacy I2C command link deletion",
    ),
    LegacyPattern(
        re.compile(r"\bi2c_master_cmd_begin\b"),
        "legacy i2c_master_cmd_begin helper",
    ),
    LegacyPattern(
        re.compile(r"\bi2c_master_(?:start|stop|write_byte|write|read_byte|read)\b"),
        "legacy low-level I2C master helper",
    ),
)

CONFIG_GUARDS: tuple[LegacyPattern, ...] = (
    LegacyPattern(
        re.compile(r"^CONFIG_I2C_ENABLE_LEGACY_DRIVERS=([yY])\b"),
        "legacy I2C driver enabled in configuration",
    ),
    LegacyPattern(
        re.compile(r"^CONFIG_I2C_SKIP_LEGACY_CONFLICT_CHECK=([nN])\b"),
        "legacy driver conflict check still enabled in configuration",
    ),
    LegacyPattern(
        re.compile(r"^#\s*CONFIG_I2C_SKIP_LEGACY_CONFLICT_CHECK\s+is not set"),
        "legacy driver conflict check still enabled in configuration",
    ),
)


def iter_source_files(
    paths: Sequence[Path],
    *,
    include_suffixes: Iterable[str] = SOURCE_SUFFIXES,
    excluded_dirs: Iterable[str] = EXCLUDED_DIRS,
) -> Iterator[Path]:
    """Yield source files below *paths* that match the allowed suffixes."""

    suffixes = {suffix.lower() for suffix in include_suffixes}
    exclude = set(excluded_dirs)

    for base in paths:
        if base.is_file():
            if base.suffix.lower() in suffixes:
                yield base
            continue

        for dirpath, dirnames, filenames in os.walk(base, followlinks=False):
            dirnames[:] = [d for d in dirnames if d not in exclude]
            for filename in filenames:
                path = Path(dirpath, filename)
                if path.suffix.lower() in suffixes:
                    yield path


def iter_config_files(
    paths: Sequence[Path],
    *,
    excluded_dirs: Iterable[str] = EXCLUDED_DIRS,
) -> Iterator[Path]:
    """Yield Kconfig-style files (``sdkconfig*``) below *paths*."""

    exclude = set(excluded_dirs)

    for base in paths:
        if base.is_file():
            if base.name.startswith(KCONFIG_PREFIX):
                yield base
            continue

        for dirpath, dirnames, filenames in os.walk(base, followlinks=False):
            dirnames[:] = [d for d in dirnames if d not in exclude]
            for filename in filenames:
                if filename.startswith(KCONFIG_PREFIX):
                    yield Path(dirpath, filename)


def scan_file(path: Path, patterns: Sequence[LegacyPattern] = LEGACY_PATTERNS) -> list[Finding]:
    """Return a list of legacy usage matches for *path*."""

    matches: list[Finding] = []
    with path.open("r", encoding="utf-8", errors="ignore") as handle:
        for line_no, line in enumerate(handle, start=1):
            for pattern in patterns:
                if pattern.regex.search(line):
                    matches.append(
                        Finding(
                            path=path,
                            line_no=line_no,
                            reason=pattern.description,
                            line=line.rstrip(),
                        )
                    )
    return matches


def scan_config_file(
    path: Path,
    patterns: Sequence[LegacyPattern] = CONFIG_GUARDS,
) -> list[Finding]:
    """Return a list of configuration violations for *path*."""

    matches: list[Finding] = []
    with path.open("r", encoding="utf-8", errors="ignore") as handle:
        for line_no, line in enumerate(handle, start=1):
            stripped = line.strip()
            for pattern in patterns:
                if pattern.regex.search(stripped):
                    matches.append(
                        Finding(
                            path=path,
                            line_no=line_no,
                            reason=pattern.description,
                            line=line.rstrip(),
                        )
                    )
    return matches


def scan_paths(paths: Sequence[Path]) -> ScanResult:
    """Scan the provided *paths* for legacy I2C usage."""

    findings: list[Finding] = []
    files_scanned = 0

    for file_path in iter_source_files(paths):
        files_scanned += 1
        findings.extend(scan_file(file_path))

    for config_path in iter_config_files(paths):
        files_scanned += 1
        findings.extend(scan_config_file(config_path))

    return ScanResult(files_scanned=files_scanned, findings=findings)


def _format_finding(finding: Finding, root: Path) -> str:
    resolved_path = finding.path.resolve()
    try:
        relative_path = resolved_path.relative_to(root)
    except ValueError:
        relative_path = resolved_path
    return (
        f"{relative_path}:{finding.line_no}: {finding.reason}\n"
        f"    {finding.line}"
    )


def run(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "paths",
        nargs="*",
        type=Path,
        default=[REPO_ROOT],
        help="Paths to scan (defaults to the repository root).",
    )
    parser.add_argument(
        "--root",
        type=Path,
        default=REPO_ROOT,
        help="Root used when reporting relative file paths.",
    )

    args = parser.parse_args(argv)
    target_paths = [path if path.is_absolute() else args.root / path for path in args.paths]

    result = scan_paths(target_paths)

    if result.findings:
        print(f"Found {len(result.findings)} legacy I2C usage(s) across {result.files_scanned} file(s):")
        for finding in result.findings:
            print(_format_finding(finding, args.root.resolve()))
        return 1

    print(f"No legacy I2C usage found in {result.files_scanned} file(s).")
    return 0


def main() -> None:  # pragma: no cover - thin CLI wrapper
    sys.exit(run())


if __name__ == "__main__":  # pragma: no cover - CLI entry point
    main()
