#include "vfs.h"
#include "../drivers/sd/block.h"
namespace pilab::vfs {
struct Ext4Super{uint32_t inodes,blocks,rblocks,free_blocks,free_inodes,first_data,log_block,log_cluster,blocks_group,clusters_group,inodes_group,mtime,wtime,mnt_count,max_mnt,magic,state,errors,minor,checktime,creator,rev,resuid,resgid;};
static bool ext4=false;
bool ext4_mount(){uint8_t s[1024];if(!block::read(2,2,s))return false;auto* sb=(Ext4Super*)(s);if(sb->magic!=0xEF53)return false;ext4=true;return true;}
bool ext4_ready(){return ext4;}
}
