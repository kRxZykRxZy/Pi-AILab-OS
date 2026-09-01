#pragma once
#include <stdint.h>
namespace pilab::net { using socket_t=int32_t; socket_t tcp_open(uint16_t port); bool tcp_connect(socket_t,uint32_t addr,uint16_t port); int tcp_send(socket_t,const void*,uint32_t); int tcp_recv(socket_t,void*,uint32_t); void tcp_close(socket_t); }
