#!/usr/bin/env bash
set -euo pipefail
if [[ $EUID -ne 0 ]]; then echo "Run with sudo"; exit 1; fi
systemctl disable --now pi-ai-lab.service 2>/dev/null || true
rm -f /etc/systemd/system/pi-ai-lab.service /usr/local/bin/pilab
systemctl daemon-reload
rm -rf /opt/pi-ai-lab
# Keep /var/lib/pi-ai-lab because it contains user models.
echo "Pi AI Lab software removed. Models remain in /var/lib/pi-ai-lab."
