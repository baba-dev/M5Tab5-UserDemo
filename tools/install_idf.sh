#!/usr/bin/env bash
set -euo pipefail

ESP_IDF_VERSION=${ESP_IDF_VERSION:-v5.4.2}
ESP_IDF_TARGET=${ESP_IDF_TARGET:-esp32p4}
ESP_IDF_ROOT=${ESP_IDF_ROOT:-$HOME/.espressif}

IDF_PATH="${ESP_IDF_ROOT}/esp-idf-${ESP_IDF_VERSION}"
REPO_URL="https://github.com/espressif/esp-idf.git"

for cmd in git python3 bash; do
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "Error: '$cmd' is required but was not found in PATH." >&2
    echo "Please install $cmd before running this script." >&2
    exit 1
  fi
done

check_libusb() {
  python3 - <<'PY'
import ctypes
import sys

NAMES = (
    "libusb-1.0.so.0",
    "libusb-1.0.dylib",
    "libusb-1.0.dll",
)

for name in NAMES:
    try:
        ctypes.CDLL(name)
    except OSError:
        continue
    else:
        sys.exit(0)

sys.exit(1)
PY
}

if ! check_libusb; then
  case "$(uname -s)" in
    Linux)
      echo "Error: libusb-1.0 runtime not found. Install it with 'sudo apt-get install libusb-1.0-0'" \
        "or the equivalent package for your distribution." >&2
      ;;
    Darwin)
      echo "Error: libusb runtime not found. Install it with 'brew install libusb'." >&2
      ;;
    *)
      echo "Error: libusb runtime not found. Please install libusb-1.0 for your platform." >&2
      ;;
  esac
  exit 1
fi

mkdir -p "${ESP_IDF_ROOT}"

if [ -d "${IDF_PATH}" ] && [ ! -d "${IDF_PATH}/.git" ]; then
  echo "Error: ${IDF_PATH} exists but is not a git repository." >&2
  echo "Please remove or rename it before running this script." >&2
  exit 1
fi

if [ ! -d "${IDF_PATH}/.git" ]; then
  echo "Cloning ESP-IDF ${ESP_IDF_VERSION} into ${IDF_PATH}"
  git clone --depth 1 --branch "${ESP_IDF_VERSION}" --recurse-submodules "${REPO_URL}" "${IDF_PATH}"
else
  echo "Updating existing ESP-IDF checkout at ${IDF_PATH}"
  git -C "${IDF_PATH}" fetch --depth 1 origin "${ESP_IDF_VERSION}"
  git -C "${IDF_PATH}" reset --hard FETCH_HEAD
fi

git -C "${IDF_PATH}" submodule update --init --recursive --depth 1

if [ ! -x "${IDF_PATH}/install.sh" ]; then
  echo "Error: install.sh not found or not executable in ${IDF_PATH}." >&2
  exit 1
fi

"${IDF_PATH}/install.sh" "${ESP_IDF_TARGET}"

export IDF_PATH

# shellcheck source=/dev/null
source "${IDF_PATH}/export.sh"

idf.py --version

echo
cat <<INSTRUCTIONS
ESP-IDF is installed at: ${IDF_PATH}
To use ESP-IDF in new shells, run:
  export IDF_PATH="${IDF_PATH}"
  source "${IDF_PATH}/export.sh"
Then, verify with:
  idf.py --version
INSTRUCTIONS
