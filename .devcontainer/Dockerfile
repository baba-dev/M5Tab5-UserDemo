FROM espressif/idf:release-v5.5

# Export IDF env for interactive and non-interactive shells
RUN echo '. "${IDF_PATH}/export.sh" >/dev/null 2>&1 || true' >> /etc/bash.bashrc \
 && printf '%s\n' '. "${IDF_PATH}/export.sh" >/dev/null 2>&1 || true' \
    | tee /etc/profile.d/esp-idf.sh >/dev/null

# Useful tools
RUN apt-get update && apt-get install -y --no-install-recommends clang-format ccache \
 && rm -rf /var/lib/apt/lists/*
