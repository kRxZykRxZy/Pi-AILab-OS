from __future__ import annotations
import os
import platform
import shutil
from pathlib import Path


def _read(path: str) -> str | None:
    try:
        return Path(path).read_text().strip()
    except (OSError, UnicodeError):
        return None


def _meminfo() -> dict[str, int]:
    out = {}
    raw = _read('/proc/meminfo') or ''
    for line in raw.splitlines():
        parts = line.split()
        if len(parts) >= 2:
            try:
                out[parts[0].rstrip(':')] = int(parts[1]) * 1024
            except ValueError:
                pass
    return out


def temperature_c() -> float | None:
    candidates = list(Path('/sys/class/thermal').glob('thermal_zone*/temp'))
    for p in candidates:
        try:
            value = int(p.read_text().strip())
            return value / 1000 if value > 1000 else float(value)
        except (OSError, ValueError):
            continue
    return None


def load_average() -> list[float]:
    try:
        return list(os.getloadavg())
    except OSError:
        return [0.0, 0.0, 0.0]


def snapshot() -> dict:
    mem = _meminfo()
    total = mem.get('MemTotal', 0)
    available = mem.get('MemAvailable', mem.get('MemFree', 0))
    disk = shutil.disk_usage('/')
    return {
        'hostname': platform.node(),
        'machine': platform.machine(),
        'system': platform.system(),
        'release': platform.release(),
        'python': platform.python_version(),
        'cpu_count': os.cpu_count() or 1,
        'load_average': load_average(),
        'memory_bytes': {'total': total, 'available': available, 'used': max(0, total - available)},
        'storage_bytes': {'total': disk.total, 'free': disk.free, 'used': disk.used},
        'temperature_c': temperature_c(),
    }
