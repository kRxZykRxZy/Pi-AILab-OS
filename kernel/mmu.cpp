#include "kernel.h"

namespace pilab {

extern "C" void arm32_mmu_enable();

/* Called by the ARM32 bootstrap after the CPU has entered supervisor mode. */
void mmu_init() {
#if defined(__arm__)
    arm32_mmu_enable();
#endif
}

}
