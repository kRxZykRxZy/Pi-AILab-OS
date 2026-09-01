#include "tokenizer.h"
namespace pilab::ai::tokenizer {
uint32_t encode_byte_fallback(const char*s,uint32_t n,uint32_t*out,uint32_t cap){if(!s||!out)return 0;uint32_t m=n<cap?n:cap;for(uint32_t i=0;i<m;i++)out[i]=(uint8_t)s[i];return m;}
}
