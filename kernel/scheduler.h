#pragma once
#include <stdint.h>
namespace pilab::sched {
struct Context { uint32_t r[13]; uint32_t sp; uint32_t lr; uint32_t pc; uint32_t cpsr; };
using entry_t = void(*)();
void init();
int create(entry_t entry, void* stack, uint32_t stack_size);
void yield();
void tick();
uint32_t current();
}
extern "C" void sched_switch(pilab::sched::Context* old_ctx, const pilab::sched::Context* new_ctx);
