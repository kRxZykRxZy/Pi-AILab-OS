#include "usb.h"
namespace pilab::usb { static Device list[16]{}; static uint32_t n=0; void init(){n=0;} uint32_t count(){return n;} const Device* devices(){return list;} }
