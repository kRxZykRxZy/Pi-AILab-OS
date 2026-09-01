#include "process.h"
namespace pilab::process {
static Process table[MAX_PROCESSES]{};
static uint32_t next_pid=1;
void init(){for(auto &p:table)p={0,State::Empty,0,0,0};}
int create(uint32_t entry,uint32_t user_sp){for(auto &p:table)if(p.state==State::Empty){p={next_pid++,State::Ready,0,entry,user_sp};return (int)p.pid;}return -1;}
Process* get(uint32_t pid){for(auto &p:table)if(p.state!=State::Empty&&p.pid==pid)return &p;return nullptr;}
void exit(int code){(void)code;}
}
