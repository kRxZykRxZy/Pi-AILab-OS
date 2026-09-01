#pragma once
#include <stdint.h>
namespace pilab::timer {
void init(uint32_t hz);
uint64_t ticks();
uint64_t frequency();
void sleep_ticks(uint64_t n);
}
