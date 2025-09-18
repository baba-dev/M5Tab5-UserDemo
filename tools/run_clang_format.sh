#!/usr/bin/env bash
set -euo pipefail

if ! command -v clang-format >/dev/null 2>&1; then
  echo "clang-format not found; please install clang-format 13 or newer" >&2
  exit 1
fi

# Format tracked C/C++ files. Respect .clang-format rules.
mapfile -t FILES < <(git ls-files '*.c' '*.cc' '*.cpp' '*.h' '*.hpp')
if [[ ${#FILES[@]} -eq 0 ]]; then
  echo "No C/C++ files to format"
  exit 0
fi

clang-format -i "${FILES[@]}"
