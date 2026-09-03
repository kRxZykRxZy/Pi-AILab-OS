#include "piai/inference/engine.hpp"
#include "piai/compute/vc4_runtime.hpp"

namespace piai::inference {

bool Engine::generate(const std::string& prompt, size_t max_tokens,
                      std::vector<std::string>& out, InferenceMode mode,
                      const std::function<bool(const std::string&)>& on_token) {
    if (mode == InferenceMode::CPU) {
        return generate(prompt, max_tokens, out, on_token);
    }

    const auto runtime = piai::compute::vc4_runtime_info();
    if (!runtime.qpu) {
        // GPU mode is deliberately fail-closed. A caller asking for GPU must
        // never receive CPU inference while believing the QPUs were used.
        return false;
    }

    // The full transformer QPU graph is introduced incrementally. Until all
    // transformer kernels are validated, Both uses the proven CPU graph while
    // GPU availability remains observable through the runtime API. GPU-only
    // therefore still refuses to run rather than silently falling back.
    if (mode == InferenceMode::GPU) return false;

    return generate(prompt, max_tokens, out, on_token);
}

} // namespace piai::inference
