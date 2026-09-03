#include "piai/inference/cache.hpp"
#include <cstdint>
#include <limits>

namespace piai::inference {

bool KVCache::init(size_t l, size_t h, size_t d, size_t cap) {
    if (!l || !h || !d || !cap) return false;
    if (l > SIZE_MAX / h || l * h > SIZE_MAX / d || l * h * d > SIZE_MAX / cap) return false;
    const size_t n = l * cap * h * d;
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
    // Entries are overwritten before being observed during the next generation.
    // Avoid touching the entire cache: this is especially important on Pi 2/3.
    used_ = 0;
}

float* KVCache::key(size_t l, size_t p, size_t h) {
    return k_.data() + ((l * capacity_ + p) * heads_ + h) * head_dim_;
}

float* KVCache::value(size_t l, size_t p, size_t h) {
    return v_.data() + ((l * capacity_ + p) * heads_ + h) * head_dim_;
}

} // namespace piai::inference
