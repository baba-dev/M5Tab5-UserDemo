#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build/lint}"
IDF_WRAPPER="${ROOT_DIR}/scripts/idf.py"
CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
CLANG_TIDY="${CLANG_TIDY:-clang-tidy}"

# shellcheck disable=SC1091
if [[ -n "${IDF_PATH:-}" && -f "${IDF_PATH}/export.sh" ]]; then
  . "${IDF_PATH}/export.sh" >/dev/null 2>&1 || true
fi

mkdir -p "${BUILD_DIR}"

pushd "${ROOT_DIR}" >/dev/null

if command -v idf.py >/dev/null 2>&1; then
  IDF_CMD=(idf.py)
elif [[ -x "${IDF_WRAPPER}" ]]; then
  IDF_CMD=("${IDF_WRAPPER}")
else
  IDF_CMD=()
fi

if [[ ${#IDF_CMD[@]} -gt 0 ]]; then
  "${IDF_CMD[@]}" -B "${BUILD_DIR}" reconfigure
else
  cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

if [[ -f "${BUILD_DIR}/compile_commands.json" ]]; then
  ln -sf "${BUILD_DIR}/compile_commands.json" "${ROOT_DIR}/compile_commands.json"
fi

mapfile -t FORMAT_FILES < <(git ls-files '*.c' '*.cc' '*.cpp' '*.cxx' '*.h' '*.hh' '*.hpp' '*.hxx')
if [[ ${#FORMAT_FILES[@]} -gt 0 ]]; then
  if ! command -v "${CLANG_FORMAT}" >/dev/null 2>&1; then
    echo "error: ${CLANG_FORMAT} not found" >&2
    exit 1
  fi
  "${CLANG_FORMAT}" -i "${FORMAT_FILES[@]}"
fi

mapfile -t TIDY_FILES < <(git ls-files '*.c' '*.cc' '*.cpp' '*.cxx')
if [[ ${#TIDY_FILES[@]} -gt 0 ]]; then
  if ! command -v "${CLANG_TIDY}" >/dev/null 2>&1; then
    echo "error: ${CLANG_TIDY} not found" >&2
    exit 1
  fi
  "${CLANG_TIDY}" --fix --format-style=file --quiet -p "${BUILD_DIR}" "${TIDY_FILES[@]}"
fi

popd >/dev/null
