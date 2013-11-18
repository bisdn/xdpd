#ifndef _CPP_IPV4_H_
#define _CPP_IPV4_H_

#include <rofl/common/endian_conversion.h>
#include <rofl/common/protocols/fipv4frame.h>

using namespace rofl;
using namespace xdpd::gnu_linux;

inline static
void ipv4_calc_checksum(void *hdr){
	((fipv4frame*)hdr)->ipv4_calc_checksum();
};

inline static
void set_ipv4_src(void *hdr, uint32_t src){
	((fipv4frame*)hdr)->set_ipv4_src(src);;
};

inline static
uint32_t get_ipv4_src(void *hdr){
	return be32toh(((fipv4frame*)hdr)->get_ipv4_src().ca_s4addr->sin_addr.s_addr);
};

inline static
void set_ipv4_dst(void *hdr, uint32_t dst){
	((fipv4frame*)hdr)->set_ipv4_dst(dst);
};

inline static
uint32_t get_ipv4_dst(void *hdr){
	return be32toh(((fipv4frame*)hdr)->get_ipv4_dst().ca_s4addr->sin_addr.s_addr);
};

inline static
void set_ipv4_dscp(void *hdr, uint8_t dscp){
	((fipv4frame*)hdr)->set_ipv4_dscp(dscp);
};

inline static
uint8_t get_ipv4_dscp(void *hdr){
	return ((fipv4frame*)hdr)->get_ipv4_dscp();
};

inline static
void set_ipv4_ecn(void *hdr, uint8_t ecn){
	((fipv4frame*)hdr)->set_ipv4_ecn(ecn);
};

inline static
uint8_t get_ipv4_ecn(void *hdr){
	return ((fipv4frame*)hdr)->get_ipv4_ecn();
};

inline static
void set_ipv4_ttl(void *hdr, uint8_t ttl){
	((fipv4frame*)hdr)->set_ipv4_ttl(ttl);
};

inline static
uint8_t get_ipv4_ttl(void *hdr){
	return ((fipv4frame*)hdr)->get_ipv4_ttl();
};

inline static
void dec_ipv4_ttl(void *hdr){
	((fipv4frame*)hdr)->dec_ipv4_ttl();
};

inline static
void set_ipv4_proto(void *hdr, uint8_t proto){
	((fipv4frame*)hdr)->set_ipv4_proto(proto);
};

inline static
uint8_t get_ipv4_proto(void *hdr){
	return ((fipv4frame*)hdr)->get_ipv4_proto();
};

inline static
void set_ipv4_ihl(void *hdr, uint8_t ihl){
	((fipv4frame*)hdr)->set_ipv4_ihl(ihl);
};

inline static
uint8_t get_ipv4_ihl(void *hdr){
	return ((fipv4frame*)hdr)->get_ipv4_ihl();
};

inline static
void set_ipv4_version(void *hdr, uint8_t version){
	((fipv4frame*)hdr)->set_ipv4_version(version);
};

inline static
uint8_t get_ipv4_version(void *hdr){
	return ((fipv4frame*)hdr)->get_ipv4_version();
};

inline static
void set_ipv4_length(void *hdr, uint16_t length){
	((fipv4frame*)hdr)->set_ipv4_length(length);
};

inline static
uint32_t get_ipv4_length(void *hdr){
	return ((fipv4frame*)hdr)->get_ipv4_length();
};

inline static
void set_DF_bit(void *hdr){
	((fipv4frame*)hdr)->set_DF_bit();
};

inline static
bool has_DF_bit_set(void *hdr){
	return ((fipv4frame*)hdr)->has_DF_bit_set();
};

inline static
void clear_DF_bit(void *hdr){
	((fipv4frame*)hdr)->clear_DF_bit();
};

inline static
void set_MF_bit(void *hdr){
	((fipv4frame*)hdr)->set_MF_bit();
};

inline static
bool has_MF_bit_set(void *hdr){
	return ((fipv4frame*)hdr)->has_MF_bit_set();
};

inline static
void clear_MF_bit(void *hdr){
	((fipv4frame*)hdr)->clear_MF_bit();
};

#endif //_CPP_IPV4_H_
