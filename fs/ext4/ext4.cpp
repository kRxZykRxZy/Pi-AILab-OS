#include "ext4.h"
#include "../../drivers/sd/block.h"
namespace pilab::ext4 {
static Superblock sb{}; static uint8_t sector[512];
static uint32_t le32(const uint8_t*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static uint16_t le16(const uint8_t*p){return (uint16_t)p[0]|((uint16_t)p[1]<<8);}
bool mount(uint64_t lba){
 if(!pilab::block::read(lba+2,1,sector))return false;
 if(le16(sector+56)!=0xef53)return false;
 sb.inodes_count=le32(sector);sb.blocks_count_lo=le32(sector+4);sb.first_data_block=le32(sector+20);sb.log_block_size=le32(sector+24);sb.blocks_per_group=le32(sector+32);sb.inodes_per_group=le32(sector+40);sb.inode_size=le16(sector+88);if(sb.inode_size<128||sb.inode_size>4096)return false;sb.valid=true;return true;
}
const Superblock& superblock(){return sb;}
}
