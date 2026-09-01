#include "irq.h"
#include <stdint.h>
namespace pilab::irq {
static handler_t handlers[96] = {};
static volatile uint32_t* const ENABLE1=(volatile uint32_t*)0x2000B210;
static volatile uint32_t* const ENABLE2=(volatile uint32_t*)0x2000B214;
static volatile uint32_t* const ENABLE_BASIC=(volatile uint32_t*)0x2000B218;
void init(){*ENABLE1=0;*ENABLE2=0;*ENABLE_BASIC=0;for(unsigned i=0;i<96;i++)handlers[i]=nullptr;}
void enable(){__asm__ volatile("cpsie i" ::: "memory");}
void disable(){__asm__ volatile("cpsid i" ::: "memory");}
void register_handler(uint32_t irq,handler_t h){if(irq<96)handlers[irq]=h;}
void dispatch(uint32_t irq){if(irq<96&&handlers[irq])handlers[irq]();}
}
