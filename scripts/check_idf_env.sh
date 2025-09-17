#!/usr/bin/env bash
set -euo pipefail
echo "IDF_PATH=${IDF_PATH:-<empty>}"
command -v idf.py || { echo "idf.py not found on PATH"; exit 1; }
idf.py --version
command -v clang-format || { echo "clang-format not found"; exit 1; }
clang-format --version
echo "OK: ESP-IDF env & clang-format present."
