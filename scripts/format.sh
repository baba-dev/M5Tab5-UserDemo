#!/usr/bin/env bash
set -euo pipefail
if command -v idf.py >/dev/null 2>&1; then
  echo "Running: idf.py clang-format"
  idf.py clang-format
else
  echo "idf.py not found; falling back to clang-format *.c/*.h"
  find . -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' \) -print0 | xargs -0 clang-format -i
fi
echo "Formatting done."
