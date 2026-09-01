#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ $EUID -ne 0 ]]; then echo 'Run: sudo ./install.sh'; exit 1; fi
command -v cmake >/dev/null || { apt-get update; apt-get install -y cmake g++ make; }
cmake -S "$ROOT" -B "$ROOT/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT/build" -j"$(nproc)"
install -Dm755 "$ROOT/build/piai" /usr/bin/piai
install -Dm644 "$ROOT/packaging/piai.service" /usr/lib/systemd/system/piai.service
install -d -o root -g root /etc/piai /var/lib/piai/models
id piai >/dev/null 2>&1 || useradd --system --no-create-home --shell /usr/sbin/nologin piai
chown -R piai:piai /var/lib/piai
printf 'bind 127.0.0.1\nport 8080\nmodel_dir /var/lib/piai/models\n' > /etc/piai/piai.conf
chmod 0644 /etc/piai/piai.conf
systemctl daemon-reload
systemctl enable --now piai
systemctl --no-pager --full status piai || true
echo 'Pi-AI Lab installed and enabled.'
