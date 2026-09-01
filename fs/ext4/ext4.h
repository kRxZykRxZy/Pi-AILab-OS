#pragma once
#include <stdint.h>
namespace pilab::ext4 {
struct Superblock { uint32_t inodes_count, blocks_count_lo, first_data_block, log_block_size, blocks_per_group, inodes_per_group; uint16_t inode_size; bool valid; };
bool mount(uint64_t lba);
const Superblock& superblock();
}
