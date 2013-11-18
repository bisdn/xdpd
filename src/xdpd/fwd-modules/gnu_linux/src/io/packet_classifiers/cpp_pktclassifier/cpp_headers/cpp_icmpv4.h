#ifndef _CPP_ICMPV4_H_
#define _CPP_ICMPV4_H_

#include <rofl/common/protocols/ficmpv4frame.h>

using namespace rofl;
using namespace xdpd::gnu_linux;


inline static
uint8_t get_icmp_type(void *hdr){
	return ((ficmpv4frame*)hdr)->get_icmp_type();
};

inline static
void set_icmp_type(void *hdr, uint8_t type){
	((ficmpv4frame*)hdr)->set_icmp_type(type);
};

inline static
uint8_t get_icmp_code(void *hdr){
	return ((ficmpv4frame*)hdr)->get_icmp_code();
};

inline static
void set_icmp_code(void *hdr, uint8_t code){
	((ficmpv4frame*)hdr)->set_icmp_code(code);
};

inline static
uint16_t get_checksum(void *hdr){
	//return ((ficmpv4frame*)hdr)->get_checksum();
	return 0;
};

inline static
void icmpv4_calc_checksum(void * hdr,uint16_t length){
	((ficmpv4frame*)hdr)->icmpv4_calc_checksum(length);
};

#endif //_CPP_ICMPV4_H_
