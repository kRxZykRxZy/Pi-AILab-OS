#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
rm -rf "$ROOT/build-deb" "$ROOT/dist"
cmake -S "$ROOT" -B "$ROOT/build-deb" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOT/build-deb" -j"$(nproc)"
PKG="$ROOT/build-deb/pkg"
install -Dm755 "$ROOT/build-deb/piai" "$PKG/usr/bin/piai"
install -Dm644 "$ROOT/packaging/piai.service" "$PKG/usr/lib/systemd/system/piai.service"
install -Dm644 "$ROOT/packaging/debian/DEBIAN/control" "$PKG/DEBIAN/control"
install -d "$PKG/etc/piai" "$PKG/var/lib/piai/models"
printf 'bind 127.0.0.1\nport 8080\nmodel_dir /var/lib/piai/models\n' > "$PKG/etc/piai/piai.conf"
mkdir -p "$ROOT/dist"
dpkg-deb --build "$PKG" "$ROOT/dist/piai_0.1.0_$(dpkg --print-architecture).deb"
