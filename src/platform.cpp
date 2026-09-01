#include "platform.hpp"
#include <thread>
#if defined(__aarch64__)
#include <sys/auxv.h>
#elif defined(__arm__)
#include <sys/auxv.h>
#endif
namespace piai::platform {
const char* name(Arch a){switch(a){case Arch::ARMv6:return "armv6";case Arch::ARMv7:return "armv7";case Arch::ARMv8:return "armv8";case Arch::X86:return "x86";case Arch::X86_64:return "x86-64";default:return "unknown";}}
Info detect(){Info i{};i.cores=std::thread::hardware_concurrency();if(!i.cores)i.cores=1;
#if defined(__aarch64__)
i.arch=Arch::ARMv8;
#elif defined(__arm__)
#if defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7__)
i.arch=Arch::ARMv7;
#else
i.arch=Arch::ARMv6;
#endif
#elif defined(__x86_64__) || defined(_M_X64)
i.arch=Arch::X86_64;
#elif defined(__i386__) || defined(_M_IX86)
i.arch=Arch::X86;
#else
i.arch=Arch::Unknown;
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
i.neon=true;
#endif
#if defined(__SSE__)
i.sse=true;
#endif
#if defined(__AVX__)
i.avx=true;
#endif
#if defined(__AVX2__)
i.avx2=true;
#endif
return i;}
}
