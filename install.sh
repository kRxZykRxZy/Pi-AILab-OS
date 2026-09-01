#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
command -v cmake >/dev/null || { echo "cmake is required"; exit 1; }
command -v c++ >/dev/null || { echo "a C++ compiler is required"; exit 1; }
cmake -S "$ROOT" -B "$ROOT/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT/build" -j1
install -Dm755 "$ROOT/build/piai" /usr/local/bin/piai
install -Dm644 "$ROOT/piai.service" /etc/systemd/system/piai.service
install -d -m755 /etc/piai /var/lib/piai/models
systemctl daemon-reload
systemctl enable --now piai.service
systemctl --no-pager --full status piai.service || true
echo "Pi-AI Lab installed and running on port 5453."
