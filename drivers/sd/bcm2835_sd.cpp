#include "block.h"
#include <stdint.h>
namespace pilab::block {
static constexpr uintptr_t EMMC=0x20300000u;
static constexpr uintptr_t ARG1=EMMC+0x08,CMDTM=EMMC+0x0c,RESP0=EMMC+0x10;
static constexpr uintptr_t DATA=EMMC+0x20,STATUS=EMMC+0x24,CONTROL1=EMMC+0x2c;
static constexpr uintptr_t INTERRUPT=EMMC+0x30,IRPT_EN=EMMC+0x38,IRPT_MASK=EMMC+0x34,BLKSIZECNT=EMMC+0x04;
static Device device{0,512,false};
static inline volatile uint32_t& r(uintptr_t a){return *reinterpret_cast<volatile uint32_t*>(a);}
static void delay(){for(volatile uint32_t i=0;i<10000;i++)__asm__ volatile("nop");}
static bool wait_cmd(){for(uint32_t i=0;i<1000000;i++){uint32_t x=r(INTERRUPT);if(x&0x8000){r(INTERRUPT)=0x8000;return true;}if(x&0x8001){r(INTERRUPT)=0xffff;return false;}}return false;}
static bool wait_data(uint32_t mask){for(uint32_t i=0;i<2000000;i++){uint32_t x=r(INTERRUPT);if(x&mask)return true;if(x&0x8000)return false;}return false;}
static bool cmd(uint32_t c,uint32_t a){r(ARG1)=a;r(CMDTM)=c;return wait_cmd();}
void init(){device={0,512,false};r(CONTROL1)=0;delay();r(CONTROL1)=7;delay();r(IRPT_EN)=0x00ffffff;r(IRPT_MASK)=0x00ffffff;r(ARG1)=0;r(CMDTM)=0;delay();if(!cmd(0x08020000,0x1aa))return;for(uint32_t i=0;i<100;i++){if(!cmd(0x37000000,0))return;if(!cmd(0x29020000,0x40ff8000))return;if(r(RESP0)&0x80000000){device.ready=true;break;}delay();}}
const Device& sdcard(){return device;}
bool read(uint64_t lba,uint32_t count,void* buffer){
 if(!device.ready||!buffer||!count||lba>0xffffffffull)return false; uint8_t* out=(uint8_t*)buffer;
 r(BLKSIZECNT)=((count&0xffff)<<16)|512; uint32_t arg=(uint32_t)lba;
 if(count==1){if(!cmd(0x11000000,arg))return false;}else{if(!cmd(0x12000000,arg))return false;}
 for(uint32_t b=0;b<count;b++)for(uint32_t i=0;i<128;i++){if(!wait_data(0x20))return false;uint32_t v=r(DATA);out[0]=v;out[1]=v>>8;out[2]=v>>16;out[3]=v>>24;out+=4;}
 if(count>1 && !cmd(0x0c010000,0))return false; r(INTERRUPT)=0xffff; return true;
}
bool write(uint64_t lba,uint32_t count,const void* buffer){
 if(!device.ready||!buffer||!count||lba>0xffffffffull)return false; const uint8_t* in=(const uint8_t*)buffer;
 r(BLKSIZECNT)=((count&0xffff)<<16)|512; uint32_t arg=(uint32_t)lba;
 if(count==1){if(!cmd(0x18020000,arg))return false;}else{if(!cmd(0x19020000,arg))return false;}
 for(uint32_t b=0;b<count;b++)for(uint32_t i=0;i<128;i++){if(!wait_data(0x10))return false;uint32_t v=(uint32_t)in[0]|((uint32_t)in[1]<<8)|((uint32_t)in[2]<<16)|((uint32_t)in[3]<<24);r(DATA)=v;in+=4;}
 if(count>1 && !cmd(0x0c010000,0))return false; r(INTERRUPT)=0xffff; return true;
}
}
