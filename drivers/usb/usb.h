#pragma once
#include <stdint.h>
namespace pilab::usb {
struct Device { uint8_t address; uint16_t vid,pid; uint8_t class_code; bool connected; };
void init(); uint32_t count(); const Device* devices();
}
