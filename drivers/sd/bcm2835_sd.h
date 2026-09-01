#pragma once
#include <stdint.h>
namespace pilab::bcm_sd {
struct Card { uint32_t rca; uint32_t version; uint32_t block_size; uint64_t blocks; bool initialized; };
bool init();
const Card& card();
bool read_blocks(uint64_t lba,uint32_t count,void* dst);
bool write_blocks(uint64_t lba,uint32_t count,const void* src);
}
