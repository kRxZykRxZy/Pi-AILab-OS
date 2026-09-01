# Pi AI Lab OS

Pi AI Lab OS is a lightweight Raspberry Pi AI appliance layer for **ARMv6, ARMv7 and ARMv8/AArch64**. V0.1 provides a boot service, local REST API, hardware telemetry, model management, an AI execution engine, CLI, and an optional web UI.

> This is intentionally a Linux userspace OS rather than a replacement kernel. It runs on a compatible Raspberry Pi Linux installation and turns it into a Pi AI Lab appliance.

## V0.1

- Architecture detection: `armv6l`, `armv7l`, `aarch64`
- CPU/RAM/load/temperature/storage telemetry
- Local model registry
- GGUF model execution through `llama-cli` when installed
- Safe subprocess execution with timeouts and memory-aware defaults
- REST API on `0.0.0.0:8787`
- systemd service
- CLI for status, models and inference
- Optional static dashboard
- Installer and uninstall scripts
- No Docker requirement
- No cloud dependency for inference

## Install

```bash
sudo ./install/install.sh
```

Then:

```bash
pilab status
pilab models
pilab run --model /var/lib/pi-ai-lab/models/model.gguf --prompt "Hello"
```

API health:

```bash
curl http://127.0.0.1:8787/api/v1/health
```

## Supported CPU families

| CPU | Linux machine | V0.1 |
|---|---|---|
| ARM11 | `armv6l` | supported |
| Cortex-A7/A53 32-bit | `armv7l` | supported |
| Cortex-A53+ 64-bit | `aarch64` | supported |

The API and engine are architecture-neutral Python code. The inference binary is treated as a pluggable runtime so a suitable `llama-cli` build can be selected for each board.

## Layout

```text
bin/              CLI
engine/           AI execution engine
api/              FastAPI service
monitor/          hardware telemetry
models/           model registry
web/              dashboard
config/           configuration
install/          installation and removal
systemd/          boot service
scripts/          maintenance helpers
```

## API

- `GET /api/v1/health`
- `GET /api/v1/system`
- `GET /api/v1/models`
- `POST /api/v1/models`
- `DELETE /api/v1/models/{id}`
- `POST /api/v1/inference`
- `GET /api/v1/jobs/{id}`
- `POST /api/v1/jobs/{id}/cancel`

Example:

```bash
curl -X POST http://127.0.0.1:8787/api/v1/inference \
  -H 'content-type: application/json' \
  -d '{"model":"/var/lib/pi-ai-lab/models/model.gguf","prompt":"Explain RAM in one sentence","max_tokens":64}'
```

## Important ARMv6 note

ARMv6 Raspberry Pis cannot run arbitrary ARMv7/ARM64 binaries. Pi AI Lab therefore never assumes one architecture-specific executable is valid everywhere. The installer detects the host and the engine validates the runtime before execution. On older boards, use a runtime compiled specifically for ARMv6 and choose small quantized models.

## License

See `LICENSE`.