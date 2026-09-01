#pragma once
#include <stdint.h>
namespace pilab::tensor {
struct View { float* data; uint32_t rows, cols; };
void matmul(const View& a,const View& b,View& out);
void add(const View& a,const View& b,View& out);
void relu(View& x);
void softmax(View& x);
}
