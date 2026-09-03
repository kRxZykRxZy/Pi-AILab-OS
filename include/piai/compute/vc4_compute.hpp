#pragma once
#include <cstddef>

namespace piai::compute {

// GPU vector primitive used to validate the QPU compute path. It uses 16-way
// SIMD and distributes 16-element chunks over the available VC4 QPUs.
bool vc4_vector_add(const float* a, const float* b, float* out, size_t n);

} // namespace piai::compute
