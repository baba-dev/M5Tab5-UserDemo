#!/usr/bin/env bash
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y --no-install-recommends ccache nodejs npm
apt-get clean
rm -rf /var/lib/apt/lists/*

python3 -m pip install --upgrade pip
python3 -m pip install pillow
npm install -g lv_font_conv

mkdir -p "${CCACHE_DIR:-/workspaces/.ccache}"

python3 fetch_repos.py
