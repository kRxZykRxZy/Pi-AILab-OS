#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
command -v cmake >/dev/null || { echo "cmake is required"; exit 1; }
command -v c++ >/dev/null || { echo "a C++ compiler is required"; exit 1; }
# Use every online CPU for the build; fall back to one if detection fails.
JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || echo 1)"
[ "$JOBS" -gt 0 ] 2>/dev/null || JOBS=1
cmake -S "$ROOT" -B "$ROOT/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT/build" --parallel "$JOBS"
sudo install -Dm755 "$ROOT/build/piai" /usr/local/bin/piai
sudo install -Dm644 "$ROOT/piai.service" /etc/systemd/system/piai.service
sudo install -d -m755 /etc/piai /var/lib/piai/models
sudo systemctl daemon-reload
sudo systemctl enable --now piai.service
sudo systemctl --no-pager --full status piai.service || true
echo "Pi-AI Lab installed and running on port 5453 using all available CPU cores for builds."
