#!/usr/bin/env bash
set -euo pipefail
. "${IDF_PATH}/export.sh" >/dev/null 2>&1 || true
idf.py build
