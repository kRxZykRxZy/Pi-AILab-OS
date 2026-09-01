#pragma once
#include <stdint.h>
namespace pilab::fat32 {
struct Volume { uint32_t first_data_sector; uint32_t sectors_per_cluster; uint32_t fat_start; uint32_t root_cluster; bool mounted; };
bool mount(uint64_t lba);
const Volume& volume();
}
