#include "kernel.h"

namespace pilab {

static void print_unsigned(uint64_t v, unsigned base) {
    const char* d = "0123456789abcdef";
    char b[32]; unsigned n = 0;
    if (!v) { console_putc('0'); return; }
    while (v && n < sizeof(b)) { b[n++] = d[v % base]; v /= base; }
    while (n) console_putc(b[--n]);
}

void console_u64(uint64_t v) { print_unsigned(v, 10); }
void console_dec(int64_t v) {
    if (v < 0) { console_putc('-'); print_unsigned((uint64_t)(-(v + 1)) + 1, 10); }
    else print_unsigned((uint64_t)v, 10);
}

}
