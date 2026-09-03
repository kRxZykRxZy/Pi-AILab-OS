#include "piai/compute/vc4_compute.hpp"
#include "piai/compute/vc4_kernels.hpp"
#include "piai/compute/vc4_memory.hpp"
#include "piai/compute/vc4_runtime.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

namespace piai::compute {

bool vc4_vector_add(const float* a, const float* b, float* out, size_t n) {
    if (!a || !b || !out || n == 0 || (n % 16) != 0) return false;
    if (vc4_kernel::vector_add_size == 0) return false;
    const VC4RuntimeInfo info = vc4_runtime_info();
    if (!info.qpu || info.qpus == 0) return false;

    // Process arbitrary multiples of 16 in batches of at most all available
    // QPUs. Never expose more work to the GPU than the allocated buffers hold.
    const size_t width = 16;
    const size_t max_vectors = std::min<size_t>(info.qpus, 12);
    size_t done = 0;
    while (done < n) {
        const size_t vectors = std::min(max_vectors, (n - done) / width);
        const size_t active = vectors * width;
        VC4Memory in_mem, out_mem;
        if (!in_mem.allocate(active * 2 * sizeof(float), 4096, true) ||
            !out_mem.allocate(active * sizeof(float), 4096, true)) return false;
        auto* in = static_cast<float*>(in_mem.cpu_ptr());
        auto* gpu_out = static_cast<float*>(out_mem.cpu_ptr());
        std::memcpy(in, a + done, active * sizeof(float));
        std::memcpy(in + active, b + done, active * sizeof(float));

        std::vector<uint32_t> uniforms(vectors * 3);
        for (size_t q = 0; q < vectors; ++q) {
            const uint32_t off = static_cast<uint32_t>(q * width * sizeof(float));
            uniforms[q * 3 + 0] = in_mem.bus_address() + off;
            uniforms[q * 3 + 1] = in_mem.bus_address() + static_cast<uint32_t>(active * sizeof(float)) + off;
            uniforms[q * 3 + 2] = out_mem.bus_address() + off;
        }
        if (!vc4_execute_program(vc4_kernel::vector_add, vc4_kernel::vector_add_size,
                                 uniforms.data(), 3, static_cast<unsigned>(vectors), 1000)) return false;
        std::memcpy(out + done, gpu_out, active * sizeof(float));
        done += active;
    }
    return true;
}

} // namespace piai::compute
