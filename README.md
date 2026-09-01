# Pi AI Lab runtime

Lightweight native C++ GGUF runtime and quantized tensor-kernel foundation. No Python and no custom kernel.

## Phase 2
- GGUF v1-v3 header validation
- metadata and tensor table parsing
- mmap-backed model storage
- tensor bounds validation
- alignment handling
- Q4_0/Q4_1/Q5_0/Q5_1/Q8_0 tensor sizing

## Phase 3
- F32 dot product
- Q4_0/Q4_1 dot products
- Q5_0/Q5_1 dot products
- Q8_0 dot product
- scalar fallback kernels suitable for every supported CPU

Build:
```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
```

Run:
```sh
./build/piai model.gguf
```

Architecture-specific SIMD kernels and the transformer runtime are subsequent phases.
