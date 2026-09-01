#pragma once
#include <stdint.h>

namespace pilab {
void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags);
void console_init();
void console_putc(char c);
void console_write(const char* s);
void console_hex(uint64_t v);
void memory_init();
void* kmalloc(uint32_t size);
void kfree(void* ptr);
}

extern "C" void kernel_main32(uint32_t r0, uint32_t r1, uint32_t atags);
extern "C" void kernel_main64();
