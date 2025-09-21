#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_DIR="${APP_DIR:-${ROOT_DIR}/platforms/tab5}"
BUILD_DIR="${BUILD_DIR:-${APP_DIR}/build}"
TIDY_BUILD_DIR="${TIDY_BUILD_DIR:-${BUILD_DIR}}"
COMPILE_COMMANDS="${COMPILE_COMMANDS:-${TIDY_BUILD_DIR}/compile_commands.json}"
IDF_WRAPPER="${ROOT_DIR}/scripts/idf.py"
CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
CLANG_TIDY="${CLANG_TIDY:-clang-tidy}"

usage() {
  cat <<USAGE
Usage: $(basename "$0") <command> [options]

Commands:
  format            Run clang-format on tracked C/C++ sources and headers.
  tidy [--fix]      Run clang-tidy using compile_commands.json. Pass --fix to apply fixes.
  fix               Run both format and tidy --fix.
  all               Run format followed by tidy (without applying fixes).

Environment:
  BUILD_DIR         Default: \$APP_DIR/build. Used when locating compile_commands.json.
  TIDY_BUILD_DIR    Default: \$BUILD_DIR. Override to point clang-tidy at a different build dir.
  COMPILE_COMMANDS  Explicit path to compile_commands.json. Overrides BUILD_DIR/TIDY_BUILD_DIR.
  CLANG_FORMAT      clang-format binary to invoke (default: clang-format).
  CLANG_TIDY        clang-tidy binary to invoke (default: clang-tidy).
USAGE
}

run_idf() {
  local subcmd=("$@")
  if command -v idf.py >/dev/null 2>&1; then
    (cd "${APP_DIR}" && idf.py "${subcmd[@]}")
    return 0
  elif [[ -x "${IDF_WRAPPER}" ]]; then
    (cd "${APP_DIR}" && "${IDF_WRAPPER}" "${subcmd[@]}")
    return 0
  fi
  return 1
}

ensure_compile_commands() {
  local cc_path="${1}"
  local cc_dir
  cc_dir="$(dirname "${cc_path}")"

  if [[ -f "${cc_path}" ]]; then
    return
  fi

  mkdir -p "${cc_dir}"
  if ! run_idf -B "${cc_dir}" reconfigure; then
    cmake -S "${APP_DIR}" -B "${cc_dir}" -DIDF_TARGET=esp32p4 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  fi

  if [[ ! -f "${cc_path}" ]]; then
    echo "error: compile_commands.json not generated at ${cc_path}" >&2
    exit 1
  fi
}

run_format() {
  cd "${ROOT_DIR}"

  mapfile -t files < <(git ls-files '*.c' '*.cc' '*.cpp' '*.cxx' '*.h' '*.hh' '*.hpp' '*.hxx')
  if [[ ${#files[@]} -eq 0 ]]; then
    echo "No C/C++ files to format"
    return
  fi

  if command -v "${CLANG_FORMAT}" >/dev/null 2>&1; then
    "${CLANG_FORMAT}" -i "${files[@]}"
    return
  fi

  echo "${CLANG_FORMAT} not found on PATH; attempting to use idf.py clang-format" >&2
  if run_idf clang-format; then
    return
  fi

  echo "error: clang-format is not available and idf.py clang-format failed" >&2
  exit 1
}

run_tidy() {
  local apply_fixes="${1}"; shift || true
  local extra_args=("$@")
  local cc_path="${COMPILE_COMMANDS}"
  local build_dir

  build_dir="$(dirname "${cc_path}")"
  ensure_compile_commands "${cc_path}"

  if ! command -v "${CLANG_TIDY}" >/dev/null 2>&1; then
    echo "error: ${CLANG_TIDY} not found" >&2
    exit 1
  fi

  cd "${ROOT_DIR}"
  mapfile -t files < <(git ls-files '*.c' '*.cc' '*.cpp' '*.cxx')
  if [[ ${#files[@]} -eq 0 ]]; then
    echo "No C/C++ translation units found for clang-tidy"
    return
  fi

  local opts=("-p" "${build_dir}" "-quiet")
  if [[ "${apply_fixes}" == "fix" ]]; then
    opts+=("-fix" "--format-style=file")
  fi

  "${CLANG_TIDY}" "${opts[@]}" "${extra_args[@]}" "${files[@]}"
}

if [[ $# -lt 1 ]]; then
  usage >&2
  exit 1
fi

command="$1"
shift

case "${command}" in
  format)
    run_format
    ;;
  tidy)
    tidy_mode="no-fix"
    if [[ "${1:-}" == "--fix" ]]; then
      tidy_mode="fix"
      shift
    fi
    run_tidy "${tidy_mode}" "$@"
    ;;
  fix)
    run_format
    run_tidy fix "$@"
    ;;
  all)
    run_format
    run_tidy no-fix "$@"
    ;;
  -h|--help|help)
    usage
    ;;
  *)
    echo "error: unknown command '${command}'" >&2
    usage >&2
    exit 1
    ;;
esac
