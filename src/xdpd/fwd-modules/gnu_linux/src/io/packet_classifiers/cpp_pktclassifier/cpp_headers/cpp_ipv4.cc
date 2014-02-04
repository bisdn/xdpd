#include "cpp_ipv4.h"
#include <rofl/common/endian_conversion.h>
#include <rofl/common/protocols/fipv4frame.h>

void ipv4_calc_checksum(void *hdr){
	((rofl::fipv4frame*)hdr)->ipv4_calc_checksum();
};

void set_ipv4_src(void *hdr, uint32_t src){
	((rofl::fipv4frame*)hdr)->set_ipv4_src(src);;
};

uint32_t get_ipv4_src(void *hdr){
	return be32toh(((rofl::fipv4frame*)hdr)->get_ipv4_src().ca_s4addr->sin_addr.s_addr);
};

void set_ipv4_dst(void *hdr, uint32_t dst){
	((rofl::fipv4frame*)hdr)->set_ipv4_dst(dst);
};

uint32_t get_ipv4_dst(void *hdr){
	return be32toh(((rofl::fipv4frame*)hdr)->get_ipv4_dst().ca_s4addr->sin_addr.s_addr);
};

void set_ipv4_dscp(void *hdr, uint8_t dscp){
	((rofl::fipv4frame*)hdr)->set_ipv4_dscp(dscp);
};

uint8_t get_ipv4_dscp(void *hdr){
	return ((rofl::fipv4frame*)hdr)->get_ipv4_dscp();
};

void set_ipv4_ecn(void *hdr, uint8_t ecn){
	((rofl::fipv4frame*)hdr)->set_ipv4_ecn(ecn);
};

uint8_t get_ipv4_ecn(void *hdr){
	return ((rofl::fipv4frame*)hdr)->get_ipv4_ecn();
};

void set_ipv4_ttl(void *hdr, uint8_t ttl){
	((rofl::fipv4frame*)hdr)->set_ipv4_ttl(ttl);
};

uint8_t get_ipv4_ttl(void *hdr){
	return ((rofl::fipv4frame*)hdr)->get_ipv4_ttl();
};

void dec_ipv4_ttl(void *hdr){
	((rofl::fipv4frame*)hdr)->dec_ipv4_ttl();
};

void set_ipv4_proto(void *hdr, uint8_t proto){
	((rofl::fipv4frame*)hdr)->set_ipv4_proto(proto);
};

uint8_t get_ipv4_proto(void *hdr){
	return ((rofl::fipv4frame*)hdr)->get_ipv4_proto();
};

void set_ipv4_ihl(void *hdr, uint8_t ihl){
	((rofl::fipv4frame*)hdr)->set_ipv4_ihl(ihl);
};

uint8_t get_ipv4_ihl(void *hdr){
	return ((rofl::fipv4frame*)hdr)->get_ipv4_ihl();
};

void set_ipv4_version(void *hdr, uint8_t version){
	((rofl::fipv4frame*)hdr)->set_ipv4_version(version);
};

uint8_t get_ipv4_version(void *hdr){
	return ((rofl::fipv4frame*)hdr)->get_ipv4_version();
};

void set_ipv4_length(void *hdr, uint16_t length){
	((rofl::fipv4frame*)hdr)->set_ipv4_length(length);
};

uint32_t get_ipv4_length(void *hdr){
	return ((rofl::fipv4frame*)hdr)->get_ipv4_length();
};

void set_ipv4_DF_bit(void *hdr){
	((rofl::fipv4frame*)hdr)->set_DF_bit();
};

bool has_ipv4_DF_bit_set(void *hdr){
	return ((rofl::fipv4frame*)hdr)->has_DF_bit_set();
};

void clear_ipv4_DF_bit(void *hdr){
	((rofl::fipv4frame*)hdr)->clear_DF_bit();
};

void set_ipv4_MF_bit(void *hdr){
	((rofl::fipv4frame*)hdr)->set_MF_bit();
};

bool has_ipv4_MF_bit_set(void *hdr){
	return ((rofl::fipv4frame*)hdr)->has_MF_bit_set();
};

void clear_ipv4_MF_bit(void *hdr){
	((rofl::fipv4frame*)hdr)->clear_MF_bit();
};
