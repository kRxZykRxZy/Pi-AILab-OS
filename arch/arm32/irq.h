#pragma once
#include <stdint.h>
namespace pilab::irq {
void init();
void enable();
void disable();
void dispatch(uint32_t irq);
using handler_t = void(*)();
void register_handler(uint32_t irq, handler_t handler);
}
