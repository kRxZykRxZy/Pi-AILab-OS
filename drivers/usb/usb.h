#pragma once
#include <stdint.h>
namespace pilab::usb { struct Device{uint16_t vid,pid;bool connected;}; void init(); uint32_t count(); const Device* devices(); }
