#ifndef _CPP_IPV4_H_
#define _CPP_IPV4_H_

#include <stdint.h>

void ipv4_calc_checksum(void *hdr);

void set_ipv4_src(void *hdr, uint32_t src);

uint32_t get_ipv4_src(void *hdr);

void set_ipv4_dst(void *hdr, uint32_t dst);

uint32_t get_ipv4_dst(void *hdr);

void set_ipv4_dscp(void *hdr, uint8_t dscp);

uint8_t get_ipv4_dscp(void *hdr);

void set_ipv4_ecn(void *hdr, uint8_t ecn);

uint8_t get_ipv4_ecn(void *hdr);

void set_ipv4_ttl(void *hdr, uint8_t ttl);

uint8_t get_ipv4_ttl(void *hdr);

void dec_ipv4_ttl(void *hdr);

void set_ipv4_proto(void *hdr, uint8_t proto);

uint8_t get_ipv4_proto(void *hdr);

void set_ipv4_ihl(void *hdr, uint8_t ihl);

uint8_t get_ipv4_ihl(void *hdr);

void set_ipv4_version(void *hdr, uint8_t version);

uint8_t get_ipv4_version(void *hdr);

void set_ipv4_length(void *hdr, uint16_t length);

uint32_t get_ipv4_length(void *hdr);

void set_DF_bit(void *hdr);

bool has_DF_bit_set(void *hdr);

void clear_DF_bit(void *hdr);

void set_MF_bit(void *hdr);

bool has_MF_bit_set(void *hdr);

void clear_MF_bit(void *hdr);

#endif //_CPP_IPV4_H_
