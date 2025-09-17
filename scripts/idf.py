#!/usr/bin/env bash
set -euo pipefail
. "${IDF_PATH}/export.sh" >/dev/null 2>&1 || true
exec python3 "${IDF_PATH}/tools/idf.py" "$@"
