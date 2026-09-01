#!/usr/bin/env bash
set -euo pipefail
printf 'machine: '; uname -m
printf 'kernel: '; uname -r
printf 'cpu: '; grep -m1 -E '^(model name|Model|Hardware|Processor)' /proc/cpuinfo | cut -d: -f2- | sed 's/^ *//'
case "$(uname -m)" in
 armv6l) echo 'Pi AI Lab target: ARMv6 / 32-bit userspace' ;;
 armv7l) echo 'Pi AI Lab target: ARMv7 / 32-bit userspace' ;;
 aarch64) echo 'Pi AI Lab target: ARMv8+ / 64-bit userspace' ;;
 *) echo 'Unsupported ARM target' >&2; exit 1 ;;
esac
