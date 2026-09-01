#include <stdint.h>
namespace pilab::uart {
static constexpr uintptr_t GPIO=0x3F200000u;
static constexpr uintptr_t UART=0x3F201000u;
static inline volatile uint32_t& R(uintptr_t a){return *reinterpret_cast<volatile uint32_t*>(a);}
void init(){
 R(UART+0x30)=0;
 R(GPIO+0x04)=(R(GPIO+0x04)&~(7u<<12))|(4u<<12);
 R(GPIO+0x08)=(R(GPIO+0x08)&~(7u<<0))|(4u<<0);
 R(UART+0x44)=0;
 R(UART+0x24)=0; R(UART+0x28)=0;
 R(UART+0x24)=26; R(UART+0x28)=3;
 R(UART+0x2c)=(3u<<5);
 R(UART+0x30)=(1u<<9)|(1u<<8)|1u;
}
void putc(char c){while(R(UART+0x18)&(1u<<5)){} R(UART)=static_cast<uint32_t>(c);}
char getc(){while(!(R(UART+0x18)&1u)){} return static_cast<char>(R(UART)&0xff);}
void write(const char*s){if(!s)return;while(*s)putc(*s++);}
}
