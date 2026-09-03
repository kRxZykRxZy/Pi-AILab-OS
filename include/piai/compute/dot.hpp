#pragma once
#include <cstddef>
#include <cstdint>
namespace piai::compute {
float dot_f16(const uint8_t* weights, const float* x, size_t n);
float dot_f32(const uint8_t* weights, const float* x, size_t n);
void axpy_f32(float* dst, const float* src, float scale, size_t n);
}
