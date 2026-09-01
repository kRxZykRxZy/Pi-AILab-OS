#include "fat32.h"
#include "../../drivers/sd/block.h"
namespace pilab::fat32 {
static Volume v{};
static uint8_t sector[512];
static uint16_t le16(const uint8_t* p){return (uint16_t)p[0]|((uint16_t)p[1]<<8);}
static uint32_t le32(const uint8_t* p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
bool mount(uint64_t lba){
 if(!pilab::block::read(lba,1,sector))return false;
 if(sector[510]!=0x55||sector[511]!=0xaa)return false;
 uint32_t bps=le16(sector+11),spc=sector[13],reserved=le16(sector+14),fats=sector[16],fat_size=le32(sector+36),root=le32(sector+44);
 if(bps!=512||!spc||!fats||!fat_size||!root)return false;
 v.sectors_per_cluster=spc;v.fat_start=(uint32_t)lba+reserved;v.first_data_sector=(uint32_t)lba+reserved+fats*fat_size;v.root_cluster=root;v.mounted=true;return true;
}
const Volume& volume(){return v;}
}
