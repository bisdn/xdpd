#include "cpp_icmpv4.h"
#include <rofl/common/protocols/ficmpv4frame.h>

uint8_t get_icmpv4_type(void *hdr){
	return ((rofl::ficmpv4frame*)hdr)->get_icmp_type();
};

void set_icmpv4_type(void *hdr, uint8_t type){
	((rofl::ficmpv4frame*)hdr)->set_icmp_type(type);
};

uint8_t get_icmpv4_code(void *hdr){
	return ((rofl::ficmpv4frame*)hdr)->get_icmp_code();
};

void set_icmpv4_code(void *hdr, uint8_t code){
	((rofl::ficmpv4frame*)hdr)->set_icmp_code(code);
};

uint16_t get_icmpv4_checksum(void *hdr){
	//return ((rofl::ficmpv4frame*)hdr)->get_checksum();
	return 0;
};

void icmpv4_calc_checksum(void * hdr,uint16_t length){
	((rofl::ficmpv4frame*)hdr)->icmpv4_calc_checksum(length);
};
