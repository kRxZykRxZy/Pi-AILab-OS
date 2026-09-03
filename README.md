# Pi AI Lab OS

Native C++ GGUF inference runtime optimized across the Raspberry Pi family. The inference stack is split into platform detection, compute kernels, model loading, transformer execution and API/runtime layers.

## Raspberry Pi support

| Board | SoC / CPU | ISA | Cores | Kernel path |
|---|---|---:|---:|---|
| Pi 1 Model A/B/B+ | ARM11 | ARMv6 | 1 | scalar/VFP |
| Pi 2 Model B v1.1 | BCM2836 Cortex-A7 | ARMv7 | 4 | NEON |
| Pi 2 Model B v1.2 | BCM2837 Cortex-A53 | ARMv8 | 4 | NEON |
| Pi 3 Model A+ | BCM2837B0 Cortex-A53 | ARMv8 | 4 | NEON |
| Pi 3 Model B | BCM2837 Cortex-A53 | ARMv8 | 4 | NEON |
| Pi 3 Model B+ | BCM2837B0 Cortex-A53 | ARMv8 | 4 | NEON |
| Pi 4 Model B | BCM2711 Cortex-A72 | ARMv8-A | 4 | NEON |
| Pi 400 | BCM2711 Cortex-A72 | ARMv8-A | 4 | NEON |
| Pi 4 Compute Module | BCM2711 Cortex-A72 | ARMv8-A | 4 | NEON |
| Pi 5 | BCM2712 Cortex-A76 | ARMv8-A | 4 | NEON |
| Pi 500 | BCM2712 Cortex-A76 | ARMv8-A | 4 | NEON |
| Pi 5 Compute Module | BCM2712 Cortex-A76 | ARMv8-A | 4 | NEON |

The build tunes natively when compiling on the target Pi. Cross-builds use safe ISA-specific flags. The runtime detects the Pi model and uses all available CPU cores for inference.

## Performance design

- Persistent inference worker pool; no thread creation per token.
- Dedicated ARM NEON F16/F32 dot kernels.
- Four independent F16 accumulators to improve instruction-level parallelism.
- Reused generation, attention and FFN scratch buffers.
- KV cache reset only changes the logical length instead of clearing the whole allocation.
- RoPE values are cached per position.
- Attention score and value paths are cache-friendly.
- Memory-mapped GGUF weights avoid copying the model into another large buffer.

## Modular layout

```text
include/piai/
  compute/        # CPU kernels
  platform/       # Raspberry Pi profiles and detection
src/
  compute/        # kernel implementations
  platform/       # platform implementations
  inference_v4.cpp# model execution core
  inference_parallel.cpp # integration boundary
  ...
tools/
  fix_inference_source.py
  bind_compute_backend.py
```

## Build

On the Pi itself:
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DPIAI_NATIVE=ON
cmake --build build -j$(nproc)
```

For a portable/cross build:
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DPIAI_NATIVE=OFF
cmake --build build -j2
```

## Run

```sh
./build/piai model.gguf
```

Performance still depends heavily on model format and memory bandwidth. Quantized models are expected to be substantially faster than FP16 on older Pis; no single tokens/sec target can be guaranteed across every board.
