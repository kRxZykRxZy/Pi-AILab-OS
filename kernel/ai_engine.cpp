#include "kernel.h"

namespace pilab::ai {

/*
 * Kernel-facing AI execution ABI.
 *
 * The transformer implementation is intentionally kept out of interrupt
 * handlers and scheduler code. This engine owns model execution state and
 * consumes memory supplied by the kernel allocator. V0.1 provides the ABI,
 * tensor primitives and a deterministic execution path; GGUF/model loading
 * is layered on top of this interface.
 */
struct Tensor {
    float* data;
    uint32_t elements;
};

struct Context {
    uint32_t context_tokens;
    uint32_t threads;
    uint64_t memory_bytes;
};

static Context ctx{};
static bool ready = false;

void init(uint32_t context_tokens, uint32_t threads) {
    ctx.context_tokens = context_tokens;
    ctx.threads = threads ? threads : 1;
    ctx.memory_bytes = 0;
    ready = true;
}

bool initialized() { return ready; }

static float dot(const float* a, const float* b, uint32_t n) {
    float s = 0.0f;
    for (uint32_t i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

float execute_scalar(const float* a, const float* b, uint32_t n) {
    if (!ready || !a || !b || !n) return 0.0f;
    return dot(a, b, n);
}

}
