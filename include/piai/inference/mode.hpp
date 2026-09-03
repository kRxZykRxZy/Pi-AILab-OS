#pragma once
#include <string>

namespace piai::inference {

enum class InferenceMode { CPU, GPU, Both };

inline const char* inference_mode_name(InferenceMode mode) {
    switch (mode) {
        case InferenceMode::CPU: return "cpu";
        case InferenceMode::GPU: return "gpu";
        case InferenceMode::Both: return "both";
    }
    return "cpu";
}

inline bool parse_inference_mode(const std::string& value, InferenceMode& out) {
    if (value == "cpu") { out = InferenceMode::CPU; return true; }
    if (value == "gpu") { out = InferenceMode::GPU; return true; }
    if (value == "both" || value == "cpu+gpu" || value == "cpu_gpu") {
        out = InferenceMode::Both; return true;
    }
    return false;
}

} // namespace piai::inference
