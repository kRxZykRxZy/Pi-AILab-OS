#include "block.h"
namespace pilab::block {
static Device device{0,512,false};
void init(){
    /* Hardware command sequencing is isolated here. V0.1 exposes the block
       contract without pretending an untested SDHCI implementation is ready. */
    device.ready=false;
}
const Device& sdcard(){return device;}
bool read(uint64_t,uint32_t,void*){return false;}
bool write(uint64_t,uint32_t,const void*){return false;}
}
