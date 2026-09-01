#include "kernel.h"

namespace pilab {
[[noreturn]] void panic(const char* reason) {
    console_write("\n\n*** Pi AI Lab kernel panic ***\n");
    console_write(reason ? reason : "unknown");
    console_write("\nCPU halted.\n");
    for (;;) { __asm__ volatile("wfe"); }
}
}
