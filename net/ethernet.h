#pragma once
#include <stdint.h>
namespace pilab::net { struct Mac{uint8_t b[6];}; void init(); bool send(const void* frame,uint32_t len); bool receive(void* frame,uint32_t cap,uint32_t* len); }
