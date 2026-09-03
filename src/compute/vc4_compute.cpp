#include "piai/compute/vc4_compute.hpp"
#include "piai/compute/vc4_memory.hpp"
#include "piai/compute/vc4_runtime.hpp"

#if __has_include("vc4_kernels.hpp")
#include "vc4_kernels.hpp"
#endif

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

namespace piai::compute {

bool vc4_vector_add(const float* a, const float* b, float* out, size_t n) {
#if __has_include("vc4_kernels.hpp")
    if (!a || !b || !out || n == 0 || (n % 16) != 0) return false;
    const VC4RuntimeInfo info = vc4_runtime_info();
    if (!info.qpu || vc4_kernel::vector_add_size == 0) return false;

    const unsigned threads = std::min<unsigned>(info.qpus, static_cast<unsigned>((n + 15) / 16));
    const size_t chunk = 16;
    const size_t active = static_cast<size_t>(threads) * chunk;

    VC4Memory in_mem, out_mem;
    if (!in_mem.allocate(active * 2 * sizeof(float), 4096, true) ||
        !out_mem.allocate(active * sizeof(float), 4096, true)) return false;

    auto* in = static_cast<float*>(in_mem.cpu_ptr());
    auto* gpu_out = static_cast<float*>(out_mem.cpu_ptr());
    std::memcpy(in, a, active * sizeof(float));
    std::memcpy(in + active, b, active * sizeof(float));

    std::vector<uint32_t> uniforms(static_cast<size_t>(threads) * 3);
    for (unsigned q = 0; q < threads; ++q) {
        const uint32_t off = static_cast<uint32_t>(q * chunk * sizeof(float));
        uniforms[q * 3 + 0] = in_mem.bus_address() + off;
        uniforms[q * 3 + 1] = in_mem.bus_address() + static_cast<uint32_t>(active * sizeof(float)) + off;
        uniforms[q * 3 + 2] = out_mem.bus_address() + off;
    }

    if (!vc4_execute_program(vc4_kernel::vector_add, vc4_kernel::vector_add_size,
                             uniforms.data(), 3, threads, 1000)) return false;

    std::memcpy(out, gpu_out, active * sizeof(float));
    return true;
#else
    (void)a; (void)b; (void)out; (void)n;
    return false;
#endif
}

} // namespace piai::compute
