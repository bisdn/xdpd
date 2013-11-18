#ifndef _CPP_UDP_H_
#define _CPP_UDP_H_

#include <rofl/common/protocols/fudpframe.h>

using namespace rofl;
using namespace xdpd::gnu_linux;

inline static
void udp_calc_checksum(void* hdr, uint32_t ip_src, uint32_t ip_dst, uint8_t ip_proto, uint16_t length){
	((fudpframe*)hdr)->udp_calc_checksum(ip_src, ip_dst, ip_proto, length);;
}

inline static
uint16_t get_udp_sport(void *hdr){
	return ((fudpframe*)hdr)->get_sport();
}

inline static
void set_udp_sport(void *hdr, uint16_t port){
	((fudpframe*)hdr)->set_sport(port);
}

inline static
uint16_t get_udp_dport(void *hdr){
	return ((fudpframe*)hdr)->get_dport();
}

inline static
void set_udp_dport(void *hdr, uint16_t port){
	((fudpframe*)hdr)->set_dport(port);
}

inline static
uint16_t get_udp_length(void *hdr){
	return ((fudpframe*)hdr)->get_length();
}

inline static
void set_udp_length(void *hdr, uint16_t length){
	((fudpframe*)hdr)->set_length(length);
}


#endif //_CPP_UDP_H_
