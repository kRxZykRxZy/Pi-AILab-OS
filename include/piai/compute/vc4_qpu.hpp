#pragma once
#include <cstddef>
#include <cstdint>

namespace piai::inference { struct TensorBinding; }

namespace piai::compute {

// Direct VideoCore IV QPU interface. This deliberately does not use OpenCL.
bool vc4_qpu_available();

// Execute the experimental QPU dense matvec kernel. Returns false when the
// platform cannot safely expose QPU execution, allowing CPU fallback.
bool vc4_qpu_matvec(const inference::TensorBinding& weights,
                    const float* x, float* y,
                    size_t rows, size_t cols);

} // namespace piai::compute
