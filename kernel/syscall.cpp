#include "kernel.h"
namespace pilab::sys {
enum : uint32_t { SYS_WRITE=1, SYS_ALLOC=2, SYS_FREE=3, SYS_YIELD=4 };
uint64_t dispatch(uint32_t n,uint64_t a,uint64_t b){
 switch(n){
 case SYS_WRITE: console_write((const char*)a); return 0;
 case SYS_ALLOC: return (uint64_t)kmalloc((uint32_t)b);
 case SYS_FREE: kfree((void*)a); return 0;
 case SYS_YIELD: return 0;
 default: return (uint64_t)-1;
 }
}
}
