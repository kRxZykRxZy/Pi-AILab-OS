#include "gguf.h"
namespace pilab::gguf {
static uint32_t u32(const uint8_t*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static uint64_t u64(const uint8_t*p){uint64_t v=0;for(int i=0;i<8;i++)v|=(uint64_t)p[i]<<(i*8);return v;}
bool open(Model&m,const void*data,uint32_t size){if(!data||size<24)return false;const uint8_t*p=(const uint8_t*)data;if(u32(p)!=0x46554747u)return false;m.image=p;m.size=size;m.header.magic=u32(p);m.header.version=u32(p+4);m.header.tensors=u64(p+8);m.header.metadata=u64(p+16);return m.header.version>=1&&m.header.version<=3;}
}
