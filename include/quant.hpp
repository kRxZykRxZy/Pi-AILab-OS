#pragma once
#include <cstddef>
#include <cstdint>
namespace piai::quant {
void q8_0_dot(const float* x,const void* w,size_t n,float& out);
void q4_0_dot(const float* x,const void* w,size_t n,float& out);
void q5_0_dot(const float* x,const void* w,size_t n,float& out);
void q4_1_dot(const float* x,const void* w,size_t n,float& out);
void q5_1_dot(const float* x,const void* w,size_t n,float& out);
void f32_dot(const float*x,const float*w,size_t n,float&out);
}
