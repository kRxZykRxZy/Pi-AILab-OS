#include "../drivers/timer/bcm_system_timer.h"
#include "../arch/arm32/irq.h"
namespace pilab::kernel_timer { void init(){ pilab::irq::init(); pilab::timer::init(100); pilab::irq::enable(); } }
