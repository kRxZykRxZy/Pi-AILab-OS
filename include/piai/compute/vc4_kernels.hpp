#pragma once
#include <cstddef>
#include <cstdint>

namespace piai::compute::vc4_kernel {
// Minimal hand-encoded VC4 QPU program used by the runtime smoke test.
// The program loads two 16-float rows through DMA/VPM, adds them, stores one
// row, and terminates. The instruction stream is intentionally kept in the
// source tree so builds have no Python or assembler dependency.
//
// This table is populated only when a validated VC4 program is supplied.
// A zero-size program makes the backend fail closed rather than executing an
// unverified instruction stream on the GPU. C++ does not permit a zero-length
// built-in array, so keep one inert byte while reporting a logical size of 0.
inline constexpr uint8_t vector_add[1] = {0};
inline constexpr std::size_t vector_add_size = 0;
}
