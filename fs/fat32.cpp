#include "vfs.h"
#include "../drivers/sd/block.h"
namespace pilab::vfs {
struct BPB{uint8_t j[3];uint8_t oem[8];uint16_t bps;uint8_t spc;uint16_t rsv;uint8_t fats;uint16_t root16;uint16_t total16;uint8_t media;uint16_t fat16;uint16_t spt;uint16_t heads;uint32_t hidden;uint32_t total32;uint32_t fat32;uint16_t extflags;uint16_t fsver;uint32_t root;};
static BPB b{}; static bool mounted=false;
bool mount(){uint8_t s[512];if(!block::read(0,1,s))return false; b=*(BPB*)(s+0); if(b.bps!=512||!b.spc||!b.fat32||!b.root)return false; mounted=true;return true;}
bool exists(const char*){return mounted;} bool read(const char*,uint32_t,void*,uint32_t){return false;} bool write(const char*,uint32_t,const void*,uint32_t){return false;} Node stat(const char*){return {Type::None,0,0};}
}
