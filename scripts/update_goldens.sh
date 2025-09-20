#!/usr/bin/env bash
set -euo pipefail
export UPDATE_GOLDEN=1
ctest --output-on-failure
