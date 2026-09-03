#pragma once

#include <cstddef>
#include <vector>

namespace piai::inference {

class KVCache {
    size_t layers_ = 0;
    size_t heads_ = 0;
    size_t head_dim_ = 0;
    size_t capacity_ = 0;
    size_t used_ = 0;
    std::vector<float> k_;
    std::vector<float> v_;
public:
    bool init(size_t layers, size_t heads, size_t head_dim, size_t capacity);
    void clear();
    size_t used() const { return used_; }
    size_t capacity() const { return capacity_; }
    float* key(size_t layer, size_t position, size_t head);
    float* value(size_t layer, size_t position, size_t head);
};

} // namespace piai::inference
