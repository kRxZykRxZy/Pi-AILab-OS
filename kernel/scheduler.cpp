#include "kernel.h"
namespace pilab::sched {
struct Task { uint32_t id; uint32_t state; void (*entry)(); };
static Task tasks[32]; static uint32_t count=0, current=0;
void init(){ count=0; current=0; }
int create(void (*entry)()){ if(!entry || count>=32) return -1; tasks[count]={count,1,entry}; return (int)count++; }
void yield(){ if(!count) return; current=(current+1)%count; if(tasks[current].state==1 && tasks[current].entry) tasks[current].entry(); }
uint32_t current_id(){ return current<count?tasks[current].id:0; }
}
