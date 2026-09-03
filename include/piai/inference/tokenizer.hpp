#pragma once

#include "piai/inference/types.hpp"
#include <string>
#include <vector>

namespace piai::gguf { class Model; }

namespace piai::inference {

class Vocabulary {
    std::vector<Token> tokens_;
public:
    bool load(const gguf::Model& model);
    bool encode(const std::string& text, std::vector<int32_t>& out) const;
    std::string decode(int32_t id) const;
    bool encode_legacy(const std::string& text, std::vector<int32_t>& out) const;
    std::string decode_legacy(int32_t id) const;
    size_t size() const { return tokens_.size(); }
};

} // namespace piai::inference
