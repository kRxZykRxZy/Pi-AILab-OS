#include "block.h"
#include <stdint.h>
namespace pilab::block {
static constexpr uintptr_t EMMC=0x20300000u;
static constexpr uintptr_t ARG1=EMMC+0x08;
static constexpr uintptr_t CMDTM=EMMC+0x0c;
static constexpr uintptr_t RESP0=EMMC+0x10;
static constexpr uintptr_t INTERRUPT=EMMC+0x30;
static constexpr uintptr_t IRPT_EN=EMMC+0x38;
static constexpr uintptr_t IRPT_MASK=EMMC+0x34;
static constexpr uintptr_t CONTROL1=EMMC+0x2c;
static Device device{0,512,false};
static inline volatile uint32_t& r(uintptr_t a){return *reinterpret_cast<volatile uint32_t*>(a);}
static void delay(){for(volatile uint32_t i=0;i<10000;i++)__asm__ volatile("nop");}
static bool wait_cmd(){for(uint32_t i=0;i<1000000;i++){uint32_t x=r(INTERRUPT);if(x&0x8000){r(INTERRUPT)=0x8000;return true;}if(x&0x8001){r(INTERRUPT)=0xffff;return false;}}return false;}
static bool cmd(uint32_t c,uint32_t a){r(ARG1)=a;r(CMDTM)=c;return wait_cmd();}
void init(){
 device={0,512,false}; r(CONTROL1)=0; delay(); r(CONTROL1)=7; delay();
 r(IRPT_EN)=0x00ffffff;r(IRPT_MASK)=0x00ffffff;
 r(ARG1)=0;r(CMDTM)=0;delay();
 if(!cmd(0x08020000,0x1aa))return;
 for(uint32_t i=0;i<100;i++){if(!cmd(0x37000000,0))return;if(!cmd(0x29020000,0x40ff8000))return;if(r(RESP0)&0x80000000){device.ready=true;break;}delay();}
}
const Device& sdcard(){return device;}
bool read(uint64_t,uint32_t,void*){return false;}
bool write(uint64_t,uint32_t,const void*){return false;}
}
