#pragma once
#include <stdint.h>
namespace pilab::process {
static constexpr uint32_t MAX_PROCESSES=16;
enum class State:uint8_t { Empty, Ready, Running, Blocked, Dead };
struct Process { uint32_t pid; State state; uint32_t pgdir; uint32_t entry; uint32_t user_sp; };
void init();
int create(uint32_t entry,uint32_t user_sp);
Process* get(uint32_t pid);
void exit(int code);
}
