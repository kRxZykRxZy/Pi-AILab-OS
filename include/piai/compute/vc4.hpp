#pragma once
#include <cstddef>
#include <cstdint>

namespace piai::inference { struct TensorBinding; }

namespace piai::compute {
// Attempts a dense matrix-vector multiply on the Raspberry Pi VideoCore IV GPU.
// Returns true when the GPU path completed successfully; false requests CPU fallback.
bool vc4_matvec(const inference::TensorBinding& weights,
                const float* x, float* y,
                size_t rows, size_t cols);

// True when a usable VideoCore IV OpenCL device was discovered.
bool vc4_available();
}
