#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <vector>

namespace piai::gguf { class Model; struct Tensor; }

namespace piai::inference {

struct Architecture {
    std::string name;
    size_t layers = 0;
    size_t hidden = 0;
    size_t heads = 0;
    size_t kv_heads = 0;
    size_t intermediate = 0;
    size_t vocab = 0;
    size_t context = 0;
    size_t rope_dim = 0;
    float eps = 1e-5f;
    float rope_theta = 10000.f;
};

struct Token {
    std::string text;
    float score = 0;
};

struct Sampling {
    float temperature = .8f;
    int top_k = 40;
    float top_p = .95f;
    float repeat_penalty = 1.1f;
    uint64_t seed = 0;
};

struct TensorBinding {
    const gguf::Tensor* tensor = nullptr;
    const uint8_t* data = nullptr;
};

} // namespace piai::inference
