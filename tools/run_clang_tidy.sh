#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR=${BUILD_DIR:-platforms/tab5/build}
COMPILE_COMMANDS=${COMPILE_COMMANDS:-$BUILD_DIR/compile_commands.json}

if [[ ! -f "$COMPILE_COMMANDS" ]]; then
  echo "compile_commands.json not found. Run 'make build' first." >&2
  exit 1
fi

if ! command -v clang-tidy >/dev/null 2>&1; then
  echo "clang-tidy not found; please install clang-tidy" >&2
  exit 1
fi

mapfile -t FILES < <(git ls-files '*.c' '*.cc' '*.cpp')
if [[ ${#FILES[@]} -eq 0 ]]; then
  echo "No C/C++ translation units found for clang-tidy"
  exit 0
fi

clang-tidy "${FILES[@]}" --quiet --p "${COMPILE_COMMANDS%/*}"
