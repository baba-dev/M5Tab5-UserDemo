FROM espressif/idf:release-v5.5

# Ensure ESP-IDF env auto-loads for all shells
RUN echo '. ${IDF_PATH}/export.sh' >> /etc/bash.bashrc

# Tools we need for formatting and faster builds
RUN apt-get update && apt-get install -y --no-install-recommends \
      clang-format ccache \
    && rm -rf /var/lib/apt/lists/*
