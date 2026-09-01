#pragma once
#include <stdint.h>
namespace pilab::gguf {
struct Header { uint32_t magic; uint32_t version; uint64_t tensors; uint64_t metadata; };
struct Model { const uint8_t* image; uint32_t size; Header header; };
bool open(Model&,const void*,uint32_t);
}
