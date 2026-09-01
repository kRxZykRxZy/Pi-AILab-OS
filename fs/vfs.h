#pragma once
#include <stdint.h>
namespace pilab::vfs {
enum class Type:uint8_t{None,File,Directory};
struct Node{Type type; uint32_t size; uint64_t inode;};
bool mount(); bool exists(const char* path); bool read(const char* path,uint32_t off,void* dst,uint32_t n); bool write(const char* path,uint32_t off,const void* src,uint32_t n); Node stat(const char* path);
}
