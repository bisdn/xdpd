#ifndef _CPP_UDP_H_
#define _CPP_UDP_H_

#include <stdint.h>

void udp_calc_checksum(void* hdr, uint32_t ip_src, uint32_t ip_dst, uint8_t ip_proto, uint16_t length);

uint16_t get_udp_sport(void *hdr);

void set_udp_sport(void *hdr, uint16_t port);

uint16_t get_udp_dport(void *hdr);

void set_udp_dport(void *hdr, uint16_t port);

uint16_t get_udp_length(void *hdr);

void set_udp_length(void *hdr, uint16_t length);


#endif //_CPP_UDP_H_
