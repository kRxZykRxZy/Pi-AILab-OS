#!/usr/bin/env bash
set -euo pipefail

PREFIX=/opt/pi-ai-lab
DATA=/var/lib/pi-ai-lab
SCRIPT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
ARCH="$(uname -m)"
case "$ARCH" in
  armv6l|armv7l|aarch64) ;;
  *) echo "Unsupported architecture: $ARCH" >&2; exit 1;;
esac

if [[ $EUID -ne 0 ]]; then echo "Run as root: sudo $0"; exit 1; fi
apt-get update
apt-get install -y python3 python3-venv python3-pip curl ca-certificates
id -u pilab >/dev/null 2>&1 || useradd --system --home "$DATA" --shell /usr/sbin/nologin pilab
mkdir -p "$PREFIX" "$DATA/models"
cp -a "$SCRIPT_DIR/api" "$SCRIPT_DIR/engine" "$SCRIPT_DIR/monitor" "$SCRIPT_DIR/config" "$PREFIX/"
cp "$SCRIPT_DIR/requirements.txt" "$PREFIX/requirements.txt"
cp "$SCRIPT_DIR/bin/pilab" /usr/local/bin/pilab
chmod +x /usr/local/bin/pilab
python3 -m venv "$PREFIX/venv"
"$PREFIX/venv/bin/pip" install --no-cache-dir -r "$PREFIX/requirements.txt"
chown -R pilab:pilab "$PREFIX" "$DATA"
install -m 0644 "$SCRIPT_DIR/systemd/pi-ai-lab.service" /etc/systemd/system/pi-ai-lab.service
systemctl daemon-reload
systemctl enable --now pi-ai-lab.service

echo "Pi AI Lab OS v0.1 installed for $ARCH"
echo "API: http://$(hostname -I | awk '{print $1}'):8787"
echo "Add a GGUF model under $DATA/models/"
echo "Install a matching llama-cli binary and ensure it is in PATH."
