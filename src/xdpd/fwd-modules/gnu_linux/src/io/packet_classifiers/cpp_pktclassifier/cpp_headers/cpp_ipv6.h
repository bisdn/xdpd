#ifndef _CPP_IPV6_H_
#define _CPP_IPV6_H_

#include <stdint.h>
#include <rofl/datapath/pipeline/common/large_types.h>

void ipv6_calc_checksum(void *hdr, uint16_t length);

void set_ipv6_version(void *hdr, uint8_t version);

uint8_t get_ipv6_version(void *hdr);

void set_ipv6_traffic_class(void *hdr, uint8_t tc);

uint8_t get_ipv6_traffic_class(void *hdr);

void set_ipv6_dscp(void *hdr, uint8_t dscp);

uint8_t get_ipv6_dscp(void *hdr);

void set_ipv6_ecn(void *hdr, uint8_t ecn);

uint8_t get_ipv6_ecn(void *hdr);

void set_flow_label(void *hdr, uint32_t flabel);

uint32_t get_flow_label(void *hdr);

void set_payload_length(void *hdr, uint16_t len);

uint16_t get_payload_length(void *hdr);

void set_next_header(void *hdr, uint8_t nxthdr);

uint8_t get_next_header(void *hdr);

void set_hop_limit(void *hdr, uint8_t hops);

uint8_t get_hop_limit(void *hdr);

void dec_hop_limit(void *hdr);

void set_ipv6_src(void *hdr, uint128__t src);

uint128__t get_ipv6_src(void *hdr);

void set_ipv6_dst(void *hdr, uint128__t dst);

uint128__t get_ipv6_dst(void *hdr);


#endif //_CPP_IPV6_H_
