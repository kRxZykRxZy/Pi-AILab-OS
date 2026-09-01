#include <stdint.h>
#include "../arch/arm32/irq.h"
extern "C" void arm32_irq_dispatch(){
    // V0.1: the BCM interrupt controller is initialized, while individual
    // peripheral acknowledgement/priority logic remains driver-owned.
    pilab::irq::dispatch(0);
}
