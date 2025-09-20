#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "[preflight] Using repo installer to setup ESP-IDF..."
bash tools/install_idf.sh

echo "[preflight] Fetching external components..."
python ./fetch_repos.py

echo "[preflight] Building Tab5 app (esp32p4)..."
pushd platforms/tab5 >/dev/null
idf.py set-target esp32p4
idf.py build
popd >/dev/null

echo "[preflight] Running host/unit tests if present..."
if [ -f tests/CMakeLists.txt ]; then
  cmake -S tests -B build/tests -DCMAKE_BUILD_TYPE=Debug
  cmake --build build/tests -- -j
  ctest --test-dir build/tests --output-on-failure
fi

# Component Unity tests via IDF (if any component has test/ with Unity cases)
if grep -R --include="CMakeLists.txt" -n "idf_component_register" components >/dev/null 2>&1; then
  echo "[preflight] Scanning for Unity tests in components/*/test ..."
  # IDF standard: tests usually land in a dedicated test app; if one exists, build it.
  # If you have a test app target (e.g., 'unit_test_app'), you can uncomment:
  # pushd platforms/tab5 >/dev/null
  # idf.py build -DIDF_UNIT_TESTING=1
  # popd >/dev/null
fi

# Optional hardware boot-loop guard
if [ "${SERIAL_PORT:-}" != "" ]; then
  echo "[preflight] Serial port detected ($SERIAL_PORT); running boot-loop guard..."
  python scripts/bootloop_guard.py --port "$SERIAL_PORT" --seconds 45 || {
    echo "::error::Boot-loop or panic detected on device"
    exit 1
  }
fi

echo "[preflight] OK"
