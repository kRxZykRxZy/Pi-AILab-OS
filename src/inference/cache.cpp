#include "piai/inference/cache.hpp"
#include <cstdint>
#include <limits>

namespace piai::inference {

bool KVCache::init(size_t l, size_t h, size_t d, size_t cap) {
    if (!l || !h || !d || !cap) return false;
    size_t n = l;
    if (n > SIZE_MAX / cap) return false;
    n *= cap;
    if (n > SIZE_MAX / h) return false;
    n *= h;
    if (n > SIZE_MAX / d) return false;
    n *= d;
    if (n > std::vector<float>().max_size()) return false;

    k_.assign(n, 0.f);
    v_.assign(n, 0.f);
    layers_ = l;
    heads_ = h;
    head_dim_ = d;
    capacity_ = cap;
    used_ = 0;
    return true;
}

void KVCache::clear() {
    // Every used entry is overwritten before it is read on a new generation.
    // Resetting the logical length avoids a full-cache memset on small Pis.
    used_ = 0;
}

float* KVCache::key(size_t l, size_t p, size_t h) {
    return k_.data() + ((l * capacity_ + p) * heads_ + h) * head_dim_;
}

float* KVCache::value(size_t l, size_t p, size_t h) {
    return v_.data() + ((l * capacity_ + p) * heads_ + h) * head_dim_;
}

} // namespace piai::inference
