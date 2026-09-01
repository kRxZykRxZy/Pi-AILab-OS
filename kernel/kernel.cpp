#include "kernel.h"
namespace pilab::uart { void init(); void write(const char*); }
namespace pilab::shell { void run(); }
namespace pilab::ai { void init(uint32_t,uint32_t); }
namespace pilab {
void kernel_main(uint32_t r0,uint32_t r1,uint32_t atags){
 (void)r0;(void)r1;(void)atags;
 uart::init(); memory_init();
 uart::write("\r\n[Pi AI Lab OS] native kernel booting...\r\n");
 uart::write("[OK] ARMv7 bootstrap\r\n");
 uart::write("[OK] MMU enabled\r\n");
 uart::write("[OK] kernel heap online\r\n");
 ai::init(4096,1);
 uart::write("[OK] built-in AI execution engine online\r\n");
 shell::run();
 for(;;)__asm__ volatile("wfe");
}
}
extern "C" void kernel_main32(uint32_t a,uint32_t b,uint32_t c){pilab::kernel_main(a,b,c);}
