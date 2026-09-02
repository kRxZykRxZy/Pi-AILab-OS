#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
command -v cmake >/dev/null || { echo "cmake is required"; exit 1; }
command -v c++ >/dev/null || { echo "a C++ compiler is required"; exit 1; }
JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc 2>/dev/null || echo 1)"
[ "$JOBS" -gt 0 ] 2>/dev/null || JOBS=1
cmake -S "$ROOT" -B "$ROOT/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT/build" --parallel "$JOBS"
sudo install -Dm755 "$ROOT/build/piai" /usr/local/bin/piai
sudo install -Dm644 "$ROOT/piai.service" /etc/systemd/system/piai.service
sudo install -d -m755 /etc/piai /var/lib/piai/models /usr/share/piai/web
sudo install -Dm644 "$ROOT/web/index.html" /usr/share/piai/web/index.html
sudo install -Dm644 "$ROOT/web/app.js" /usr/share/piai/web/app.js
sudo install -Dm644 "$ROOT/web/style.css" /usr/share/piai/web/style.css
sudo systemctl daemon-reload
sudo systemctl enable --now piai.service
sudo systemctl --no-pager --full status piai.service || true
echo "Pi-AI Lab installed. Open http://<PI-IP>:5453 on your phone."
