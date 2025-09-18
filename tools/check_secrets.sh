#!/usr/bin/env bash
set -euo pipefail

if ! command -v rg >/dev/null 2>&1; then
  echo "ripgrep (rg) is required to scan for secrets" >&2
  exit 1
fi

PATTERNS=(
  'AKIA[0-9A-Z]{16}'
  'ASIA[0-9A-Z]{16}'
  'AIza[0-9A-Za-z\-_]{35}'
  '-----BEGIN (EC|RSA|DSA)? ?PRIVATE KEY-----'
  'xox[baprs]-[0-9A-Za-z]+'
  'ghp_[0-9A-Za-z]{36}'
)

EXIT_CODE=0

while IFS= read -r file; do
  for pattern in "${PATTERNS[@]}"; do
    if rg --hidden --no-heading --color=never -n "$pattern" "$file" >/tmp/secret_hits.$$ 2>/dev/null; then
      echo "Potential secret detected in $file:" >&2
      cat /tmp/secret_hits.$$ >&2
      EXIT_CODE=2
    fi
  done
  rm -f /tmp/secret_hits.$$
done < <(git ls-files)

exit $EXIT_CODE
