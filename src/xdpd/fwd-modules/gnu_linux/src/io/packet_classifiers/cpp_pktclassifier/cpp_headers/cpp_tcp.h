#ifndef _CPP_TCP_H_
#define _CPP_TCP_H_

#include <rofl/common/protocols/ftcpframe.h>

using namespace rofl;
using namespace xdpd::gnu_linux;

inline static
void tcp_calc_checksum(void* hdr, uint32_t ip_src, uint32_t ip_dst, uint8_t ip_proto, uint16_t length){
	((ftcpframe*)hdr)->tcp_calc_checksum(ip_src, ip_dst, ip_proto, length);
}

inline static
uint16_t get_tcp_sport(void *hdr){
	return ((ftcpframe*)hdr)->get_sport();
}

inline static
void set_tcp_sport(void *hdr, uint16_t port){
	((ftcpframe*)hdr)->set_sport(port);
}

inline static
uint16_t get_tcp_dport(void *hdr){
	return ((ftcpframe*)hdr)->get_dport();
}

inline static
void set_tcp_dport(void *hdr, uint16_t port){
	((ftcpframe*)hdr)->set_dport(port);
}


#endif //_CPP_TCP_H_
