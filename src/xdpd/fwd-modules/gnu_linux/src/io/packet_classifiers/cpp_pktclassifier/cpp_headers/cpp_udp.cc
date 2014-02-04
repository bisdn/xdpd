#include "cpp_udp.h"
#include <rofl/common/protocols/fudpframe.h>

void udp_calc_checksum(void* hdr, uint32_t ip_src, uint32_t ip_dst, uint8_t ip_proto, uint16_t length){
	rofl::caddress ipsrc(AF_INET,"0.0.0.0");
	ipsrc.set_ipv4_addr(ip_src);
	rofl::caddress ipdst(AF_INET,"0.0.0.0");
	ipdst.set_ipv4_addr(ip_dst);
	((rofl::fudpframe*)hdr)->udp_calc_checksum(ipsrc, ipdst, ip_proto, length);;
}

uint16_t get_udp_sport(void *hdr){
	return ((rofl::fudpframe*)hdr)->get_sport();
}

void set_udp_sport(void *hdr, uint16_t port){
	((rofl::fudpframe*)hdr)->set_sport(port);
}

uint16_t get_udp_dport(void *hdr){
	return ((rofl::fudpframe*)hdr)->get_dport();
}

void set_udp_dport(void *hdr, uint16_t port){
	((rofl::fudpframe*)hdr)->set_dport(port);
}

uint16_t get_udp_length(void *hdr){
	return ((rofl::fudpframe*)hdr)->get_length();
}

void set_udp_length(void *hdr, uint16_t length){
	((rofl::fudpframe*)hdr)->set_length(length);
}


