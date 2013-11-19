#ifndef _CPP_TCP_H_
#define _CPP_TCP_H_

#include <stdint.h>

void tcp_calc_checksum(void* hdr, uint32_t ip_src, uint32_t ip_dst, uint8_t ip_proto, uint16_t length);

uint16_t get_tcp_sport(void *hdr);

void set_tcp_sport(void *hdr, uint16_t port);

uint16_t get_tcp_dport(void *hdr);

void set_tcp_dport(void *hdr, uint16_t port);

#endif //_CPP_TCP_H_
