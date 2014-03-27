#include "cpp_tcp.h"
#include <rofl/common/protocols/ftcpframe.h>

void tcp_calc_checksum(void* hdr, uint32_t ip_src, uint32_t ip_dst, uint8_t ip_proto, uint16_t length){
	rofl::caddress ipsrc(AF_INET,"0.0.0.0");
	ipsrc.set_ipv4_addr(ip_src);
	rofl::caddress ipdst(AF_INET,"0.0.0.0");
	ipdst.set_ipv4_addr(ip_dst);
	((rofl::ftcpframe*)hdr)->tcp_calc_checksum(ipsrc, ipdst, ip_proto, length);
}

uint16_t get_tcp_sport(void *hdr){
	return ((rofl::ftcpframe*)hdr)->get_sport();
}

void set_tcp_sport(void *hdr, uint16_t port){
	((rofl::ftcpframe*)hdr)->set_sport(port);
}

uint16_t get_tcp_dport(void *hdr){
	return ((rofl::ftcpframe*)hdr)->get_dport();
}

void set_tcp_dport(void *hdr, uint16_t port){
	((rofl::ftcpframe*)hdr)->set_dport(port);
}


