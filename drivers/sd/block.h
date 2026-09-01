#pragma once
#include <stdint.h>
namespace pilab::block {
struct Device { uint64_t sectors; uint32_t sector_size; bool ready; };
void init();
const Device& sdcard();
bool read(uint64_t lba, uint32_t count, void* buffer);
bool write(uint64_t lba, uint32_t count, const void* buffer);
}
