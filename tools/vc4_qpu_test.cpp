#include "piai/compute/vc4_compute.hpp"
#include "piai/compute/vc4_runtime.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

int main() {
    const auto r = piai::compute::vc4_runtime_info();
    std::printf("mailbox=%d memory=%d qpu=%d qpus=%u reason=%s\n", r.mailbox, r.memory, r.qpu, r.qpus, r.reason.c_str());
    if (!r.qpu) return 2;
    const size_t n = 16 * 25;
    std::vector<float> a(n), b(n), c(n);
    for (size_t i = 0; i < n; ++i) { a[i] = float(i) * .25f; b[i] = 100.f - float(i) * .125f; }
    if (!piai::compute::vc4_vector_add(a.data(), b.data(), c.data(), n)) return 3;
    float e = 0.f;
    for (size_t i = 0; i < n; ++i) e = std::max(e, std::fabs(c[i] - (a[i] + b[i])));
    std::printf("n=%zu max_error=%.9g\n", n, e);
    return e <= 1e-5f ? 0 : 4;
}
