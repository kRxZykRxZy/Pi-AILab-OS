#pragma once
#include <cstdint>
namespace piai::platform {
enum class Arch { ARMv6, ARMv7, ARMv8, X86, X86_64, Unknown };
struct Info { Arch arch; unsigned cores; bool neon; bool sse; bool avx; bool avx2; };
Info detect();
const char* name(Arch);
}
