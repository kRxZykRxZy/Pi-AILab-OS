#pragma once
#include <stdint.h>
namespace pilab::net { struct IPv4{uint8_t b[4];}; bool arp_resolve(IPv4 ip,uint8_t mac[6]); bool ipv4_send(IPv4 dst,uint8_t proto,const void* data,uint16_t len); }
