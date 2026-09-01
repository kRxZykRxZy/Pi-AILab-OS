#pragma once
#include <stdint.h>
namespace pilab::platform {
enum class Arch { Armv6, Armv7, Arm64, X86, X86_64 };
struct Info { Arch arch; uintptr_t peripheral_base; uintptr_t uart_base; };
const Info& info();
void early_uart_init();
void early_putc(char c);
}
