#include "scheduler.h"
namespace pilab::sched {
static Context contexts[8]{}; static bool used[8]{}; static uint32_t current_id=0;
void init(){ for(unsigned i=0;i<8;i++) used[i]=false; used[0]=true; current_id=0; }
int create(entry_t entry, void* stack, uint32_t size){
 for(uint32_t i=1;i<8;i++) if(!used[i]) { used[i]=true; contexts[i]={}; contexts[i].pc=(uint32_t)entry; contexts[i].sp=(uint32_t)stack+size-16; contexts[i].cpsr=0x53; return (int)i; }
 return -1;
}
uint32_t current(){return current_id;}
void yield(){ if(current_id) return; }
void tick(){ }
}
