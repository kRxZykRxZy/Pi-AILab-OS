#pragma once
#include <stdint.h>
namespace pilab::ai::tokenizer {
struct Token { uint32_t id; const char* text; uint16_t length; };
uint32_t encode_byte_fallback(const char*,uint32_t,uint32_t*,uint32_t);
}
