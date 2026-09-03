#include "gguf.hpp"
#include "quant.hpp"
#include "transformer.hpp"
#include "api.hpp"
#include "piai/compute/vc4_compute.hpp"
#include "piai/compute/vc4_runtime.hpp"
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <chrono>

namespace {
int run_vc4_test() {
    const auto info = piai::compute::vc4_runtime_info();
    std::printf("VC4 mailbox: %s\n", info.mailbox ? "yes" : "no");
    std::printf("VC4 memory:  %s\n", info.memory ? "yes" : "no");
    std::printf("VC4 QPU:     %s (%s)\n", info.qpu ? "yes" : "no", info.reason.c_str());
    if (!info.qpu) return 3;

    alignas(16) float a[32];
    alignas(16) float b[32];
    alignas(16) float out[32]{};
    for (size_t i = 0; i < 32; ++i) {
        a[i] = static_cast<float>(i + 1);
        b[i] = static_cast<float>(32 - i);
    }

    if (!piai::compute::vc4_vector_add(a, b, out, 32)) {
        std::fprintf(stderr, "VC4 vector-add execution failed\n");
        return 4;
    }

    for (size_t i = 0; i < 32; ++i) {
        if (out[i] != 33.0f) {
            std::fprintf(stderr, "VC4 vector-add mismatch at %zu: got %.8g expected 33\n",
                         i, out[i]);
            return 5;
        }
    }
    std::printf("VC4 vector-add: PASS (32 elements, 16-wide SIMD)\n");
    return 0;
}
}

int main(int argc, char** argv) {
    if (argc >= 2 && std::strcmp(argv[1], "--vc4-test") == 0) {
        return run_vc4_test();
    }

    piai::api::Server server;
    if (!server.listen(5453)) {
        std::fprintf(stderr, "piai: unable to bind API port 5453\n");
        return 10;
    }
    std::printf("Pi-AI Lab runtime v0.1\nAPI listening on port 5453\n");
    if (argc >= 2) {
        piai::gguf::Model m;
        if (!m.open(argv[1])) {
            std::fprintf(stderr, "piai: invalid or unsupported GGUF: %s\n", argv[1]);
            return 2;
        }
        std::printf("GGUF v%u, tensors=%zu, data_offset=%llu\n",
                    m.version(), m.tensors().size(),
                    (unsigned long long)m.data_offset());
    }
    server.serve_forever();
    return 0;
}
