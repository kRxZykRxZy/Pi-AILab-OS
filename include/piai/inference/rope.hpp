#pragma once
#include <cstddef>

namespace piai::inference {
void apply_rope(float* x, size_t dimension, size_t position, float theta);
}
