#include "piai/inference/rope.hpp"
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace piai::inference {

void apply_rope(float* x, size_t d, size_t pos, float theta) {
    if (!x || d < 2 || theta <= 0.f) return;
    struct Cache {
        size_t d = 0;
        size_t pos = SIZE_MAX;
        float theta = 0.f;
        std::vector<float> c;
        std::vector<float> s;
    };
    static thread_local Cache cache;
    if (cache.d != d || cache.pos != pos || cache.theta != theta) {
        cache.d = d;
        cache.pos = pos;
        cache.theta = theta;
        cache.c.resize(d / 2);
        cache.s.resize(d / 2);
        for (size_t i = 0; i < d / 2; ++i) {
            const float angle = pos * std::pow(theta, -2.f * static_cast<float>(i) / static_cast<float>(d));
            cache.c[i] = std::cos(angle);
            cache.s[i] = std::sin(angle);
        }
    }
    for (size_t i = 0, j = 0; i + 1 < d; i += 2, ++j) {
        const float c = cache.c[j], s = cache.s[j];
        const float u = x[i], v = x[i + 1];
        x[i] = u * c - v * s;
        x[i + 1] = u * s + v * c;
    }
}

} // namespace piai::inference
