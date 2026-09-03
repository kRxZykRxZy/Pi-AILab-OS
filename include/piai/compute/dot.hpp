#pragma once
#include <cstddef>
#include <cstdint>
namespace piai::compute {
float dot_f16(const uint8_t* weights, const float* x, size_t n);
float dot_f32(const uint8_t* weights, const float* x, size_t n);
}
