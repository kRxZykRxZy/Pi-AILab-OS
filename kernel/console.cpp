#include "kernel.h"

namespace pilab {
static volatile uint32_t* const UART0 = (volatile uint32_t*)0x3F201000;
static constexpr uint32_t UART_DR = 0;
static constexpr uint32_t UART_FR = 6;

void console_init() {}

void console_putc(char c) {
    while (UART0[UART_FR] & (1u << 5)) {}
    UART0[UART_DR] = (uint32_t)c;
}

void console_write(const char* s) {
    if (!s) return;
    while (*s) console_putc(*s++);
}

void console_hex(uint64_t v) {
    const char* h = "0123456789ABCDEF";
    console_write("0x");
    for (int i = 15; i >= 0; --i) console_putc(h[(v >> (i * 4)) & 0xF]);
}
}
