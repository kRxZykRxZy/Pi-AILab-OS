#include "kernel.h"
namespace pilab::vfs {
struct File { const char* name; const uint8_t* data; uint32_t size; };
static File files[64]; static uint32_t count;
void init(){count=0;}
int register_file(const char*n,const void*d,uint32_t s){if(!n||!d||count>=64)return -1;files[count]={n,(const uint8_t*)d,s};return (int)count++;}
const File* find(const char*n){for(uint32_t i=0;i<count;i++){const char*a=n,*b=files[i].name;while(*a&&*a==*b){a++;b++;}if(!*a&&!*b)return &files[i];}return 0;}
}
