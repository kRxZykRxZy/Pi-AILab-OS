#include "bcm_system_timer.h"
#include "../../arch/arm32/irq.h"
namespace pilab::timer {
static volatile uint32_t* const CLO=(volatile uint32_t*)0x20003004;
static volatile uint32_t* const C1=(volatile uint32_t*)0x20003010;
static volatile uint32_t* const CS=(volatile uint32_t*)0x20003000;
static uint32_t hz_value=100;
static uint64_t interval=10000;
static volatile uint64_t ticks_value=0;
static void tick(){uint32_t now=*CLO; *C1=now+(uint32_t)interval; *CS=(1u<<1); ++ticks_value;}
void init(uint32_t hz){if(!hz) hz=100; hz_value=hz; interval=1000000ull/hz; uint32_t now=*CLO; *C1=now+(uint32_t)interval; *CS=(1u<<1); irq::register_handler(1,tick);}
uint64_t ticks(){return ticks_value;}
uint64_t frequency(){return hz_value;}
void sleep_ticks(uint64_t n){uint64_t end=ticks_value+n; while(ticks_value<end) __asm__ volatile("wfe");}
}
