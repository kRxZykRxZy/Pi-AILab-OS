#include "piai/compute/vc4_compute.hpp"
#include "piai/compute/vc4_runtime.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

int main() {
    const auto info = piai::compute::vc4_runtime_info();
    std::printf("VC4 mailbox: %s\n", info.mailbox ? "yes" : "no");
    std::printf("VC4 memory : %s\n", info.memory ? "yes" : "no");
    std::printf("VC4 QPU    : %s\n", info.qpu ? "yes" : "no");
    std::printf("QPU count  : %u\n", info.qpus);
    std::printf("Reason     : %s\n", info.reason.c_str());

    if (!info.qpu) {
        std::fprintf(stderr, "VC4 QPU runtime unavailable; no GPU code was executed.\n");
        return 2;
    }

    // Exercise more than one 12-QPU batch so the host-side dispatcher is
    // tested for correct handling of arbitrary multiples of the 16-wide SIMD
    // vector width.
    constexpr size_t n = 16 * 25;
    std::vector<float> a(n), b(n), out(n, -1.0f);
    for (size_t i = 0; i < n; ++i) {
        a[i] = static_cast<float>(i) * 0.25f;
        b[i] = 100.0f - static_cast<float>(i) * 0.125f;
    }

    if (!piai::compute::vc4_vector_add(a.data(), b.data(), out.data(), n)) {
        std::fprintf(stderr, "VC4 vector-add execution failed.\n");
        return 3;
    }

    float max_error = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        const float expected = a[i] + b[i];
        max_error = std::max(max_error, std::fabs(out[i] - expected));
    }
    std::printf("Vector elements: %zu\n", n);
    std::printf("Max error      : %.9g\n", max_error);

    if (max_error > 1e-5f) {
        std::fprintf(stderr, "FAIL: GPU result differs from CPU reference.\n");
        return 4;
    }

    std::puts("PASS: VC4 QPU vector kernel produced correct results.");
    return 0;
}
