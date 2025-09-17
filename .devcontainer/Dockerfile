FROM espressif/idf:release-v5.5

# Ensure ESP-IDF env auto-loads for all shells
RUN echo '. ${IDF_PATH}/export.sh' >> /etc/bash.bashrc
RUN printf '%s\n' '. "${IDF_PATH}/export.sh" >/dev/null 2>&1 || true' \
  | tee /etc/profile.d/esp-idf.sh >/dev/null

# Provide an idf.py shim that self-configures the environment
RUN printf '%s\n' '#!/usr/bin/env bash' \
  'set -euo pipefail' \
  '# Always bring ESP-IDF onto PATH for this process' \
  '. "${IDF_PATH}/export.sh" >/dev/null 2>&1 || true' \
  'exec python3 "${IDF_PATH}/tools/idf.py" "$@"' \
  > /usr/local/bin/idf.py \
  && chmod +x /usr/local/bin/idf.py

# Tools we need for formatting and faster builds
RUN apt-get update && apt-get install -y --no-install-recommends \
      clang-format ccache \
    && rm -rf /var/lib/apt/lists/*
