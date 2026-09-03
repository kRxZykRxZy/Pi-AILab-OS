#pragma once

#include "gguf.hpp"
#include "piai/inference/cache.hpp"
#include "piai/inference/rope.hpp"
#include "piai/inference/tokenizer.hpp"
#include "piai/inference/types.hpp"
#include <functional>
#include <random>
#include <string>
#include <vector>

namespace piai::inference {

bool detect_architecture(const gguf::Model& model, Architecture& out);

class Engine {
    Architecture arch_;
    Vocabulary vocab_;
    KVCache cache_;
    Sampling sampling_;
    std::mt19937_64 rng_;
    const gguf::Model* model_ = nullptr;
    TensorBinding embedding_{};
    TensorBinding output_{};
    TensorBinding norm_final_{};
    std::vector<TensorBinding> q_, k_, v_, o_, norm1_, norm2_, ffn1_, ffn2_, ffn3_;
public:
    bool load(const gguf::Model& model);
    bool generate(const std::string& prompt, size_t max_tokens,
                  std::vector<std::string>& out,
                  const std::function<bool(const std::string&)>& on_token = {});
    const Architecture& architecture() const { return arch_; }
    const TensorBinding& embedding() const { return embedding_; }
};

} // namespace piai::inference
