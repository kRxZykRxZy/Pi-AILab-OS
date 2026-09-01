#include "kernel.h"
namespace pilab::timer {
static volatile uint64_t ticks=0;
void init(){ticks=0;}
void tick(){++ticks;}
uint64_t uptime_ticks(){return ticks;}
}
