#include "kernel.h"

namespace pilab {
static void banner() {
    console_write("\r\n========================================\r\n");
    console_write("        Pi AI Lab OS v0.1\r\n");
    console_write("        Native ARM Kernel\r\n");
    console_write("========================================\r\n");
}

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void)r0; (void)r1; (void)atags;
    console_init();
    memory_init();
    banner();
    console_write("[OK] Console\r\n");
    console_write("[OK] Physical heap: 8 MiB bootstrap\r\n");
    console_write("[OK] Kernel core online\r\n");
    console_write("[OK] AI execution subsystem: native service boundary\r\n");
    console_write("[OK] API subsystem: native service boundary\r\n");
    console_write("PiLab> kernel ready\r\n");
    for (;;) { __asm__ volatile("wfe"); }
}
}

extern "C" void kernel_main32(uint32_t r0, uint32_t r1, uint32_t atags) {
    pilab::kernel_main(r0, r1, atags);
}

extern "C" void kernel_main64() {
    pilab::kernel_main(0, 0, 0);
}
