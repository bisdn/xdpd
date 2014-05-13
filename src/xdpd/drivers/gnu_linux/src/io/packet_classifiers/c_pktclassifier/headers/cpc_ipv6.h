/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_IPV6_H_
#define _CPC_IPV6_H_

/**
* @file cpc_ipv6.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for IPv6
*/

//struct extension header
struct cpc_ipv6_ext_hdr_t {
	uint8_t nxthdr;
	uint8_t len;
	uint8_t data[0];
} __attribute__((packed));

/* ipv6 constants and definitions */
enum ipv6_ext_t {
	IPV6_IPPROTO_HOPOPT 		= 0,
	IPV6_IPPROTO_ICMPV4 		= 1,
	IPV6_IPPROTO_TCP 			= 6,
	IPV6_IPPROTO_UDP 			= 17,
	IPV6_IP_PROTO 				= 41,
	IPV6_IPPROTO_ROUTE			= 43,
	IPV6_IPPROTO_FRAG			= 44,
	IPV6_IPPROTO_ICMPV6			= 58,
	IPV6_IPPROTO_NONXT			= 59,
	IPV6_IPPROTO_OPTS			= 60,
	IPV6_IPPROTO_MIPV6			= 135,
};

// IPv6 header
typedef struct cpc_ipv6_hdr {
	uint8_t bytes[4];      	// version + tc + flow-label
	uint16_t payloadlen;
	uint8_t nxthdr;
	uint8_t hoplimit;
	uint8_t src[IPV6_ADDR_LEN];
	uint8_t dst[IPV6_ADDR_LEN];
	uint8_t data[0];
} __attribute__((packed)) cpc_ipv6_hdr_t;



//TODO extension headers

inline static
void ipv6_calc_checksum(void *hdr, uint16_t length){
	//TODO implement checksum calulation
};


inline static
void set_ipv6_version(void *hdr, uint8_t version){
	((cpc_ipv6_hdr_t*)hdr)->bytes[0] = (((cpc_ipv6_hdr_t*)hdr)->bytes[0] & ~OF1X_4MSBITS_MASK) + (version & OF1X_4MSBITS_MASK);
};

inline static
uint8_t get_ipv6_version(void *hdr){
	return (uint8_t)(((cpc_ipv6_hdr_t*)hdr)->bytes[0] & OF1X_4MSBITS_MASK);
};

inline static
void set_ipv6_traffic_class(void *hdr, uint16_t tc){
	uint16_t *ptr = (uint16_t*)(((cpc_ipv6_hdr_t*)hdr)->bytes);
	*ptr = (tc & OF1X_8MIDDLE_BITS_MASK) | (*ptr & ~OF1X_8MIDDLE_BITS_MASK);
}

inline static
uint16_t get_ipv6_traffic_class(void *hdr){
	uint16_t *ptr = (uint16_t*)(((cpc_ipv6_hdr_t*)hdr)->bytes);
	return (*ptr) & OF1X_8MIDDLE_BITS_MASK;
};

inline static
void set_ipv6_dscp(void *hdr, uint8_t dscp){
	//NOTE We are aligning this value to fit IPv4 alignment of DSCP
	((cpc_ipv6_hdr_t*)hdr)->bytes[0] = (((cpc_ipv6_hdr_t*)hdr)->bytes[0] & OF1X_4MSBITS_MASK) | ((dscp & OF1X_4MSBITS_MASK) >> 4);
	((cpc_ipv6_hdr_t*)hdr)->bytes[1] = (((cpc_ipv6_hdr_t*)hdr)->bytes[1] & OF1X_6LSBITS_MASK) | ((dscp & OF1X_BITS_2AND3_MASK) << 4);
}

inline static
uint8_t get_ipv6_dscp(void *hdr){
	//NOTE We are aligning this value to fit IPv4 alignment of DSCP
	return ( (((cpc_ipv6_hdr_t*)hdr)->bytes[0] & OF1X_4LSBITS_MASK) << 4 ) | ( (((cpc_ipv6_hdr_t*)hdr)->bytes[1] & OF1X_2MSBITS_MASK) >> 4 );
};

inline static
void set_ipv6_ecn(void *hdr, uint8_t ecn){
	//NOTE We are aligning this value to fit IPv4 alignment of ECN
	((cpc_ipv6_hdr_t*)hdr)->bytes[1] = (((cpc_ipv6_hdr_t*)hdr)->bytes[1] & ~OF1X_BITS_4AND5_MASK) | ((ecn<<4) & OF1X_BITS_4AND5_MASK);
}

inline static
uint8_t get_ipv6_ecn(void *hdr){
	//NOTE We are aligning this value to fit IPv4 alignment of ECN
	return (uint8_t)(((cpc_ipv6_hdr_t*)hdr)->bytes[1] & OF1X_BITS_4AND5_MASK) >> 4;
};

inline static
void set_ipv6_flow_label(void *hdr, uint32_t flabel){
	uint32_t *ptr = (uint32_t*)&((cpc_ipv6_hdr_t*)hdr)->bytes[1];
	*ptr = (flabel & OF1X_20_BITS_IPV6_FLABEL_MASK) | (*ptr & ~OF1X_20_BITS_IPV6_FLABEL_MASK);
}

inline static
uint32_t get_ipv6_flow_label(void *hdr){
	uint32_t *ptr = (uint32_t*)&((cpc_ipv6_hdr_t*)hdr)->bytes[1];
	return (*ptr) & OF1X_20_BITS_IPV6_FLABEL_MASK;
};

inline static
void set_ipv6_payload_length(void *hdr, uint16_t len){
	((cpc_ipv6_hdr_t*)hdr)->payloadlen = len;
}

inline static
uint16_t get_ipv6_payload_length(void *hdr){
	return ((cpc_ipv6_hdr_t*)hdr)->payloadlen;
};

inline static
void set_ipv6_next_header(void *hdr, uint8_t nxthdr){
	((cpc_ipv6_hdr_t*)hdr)->nxthdr = nxthdr;
}

inline static
uint8_t get_ipv6_next_header(void *hdr){
	return ((cpc_ipv6_hdr_t*)hdr)->nxthdr;
};

inline static
void set_ipv6_hop_limit(void *hdr, uint8_t hops){
	((cpc_ipv6_hdr_t*)hdr)->hoplimit = hops;
}

inline static
uint8_t get_ipv6_hop_limit(void *hdr){
	return ((cpc_ipv6_hdr_t*)hdr)->hoplimit;
};

inline static
void dec_ipv6_hop_limit(void *hdr){
	((cpc_ipv6_hdr_t*)hdr)->hoplimit--;
};

inline static
void set_ipv6_src(void *hdr, uint128__t src){
	uint128__t *ptr=(uint128__t*)&(((cpc_ipv6_hdr_t*)hdr)->src);
	*ptr = src;
};

inline static
uint128__t get_ipv6_src(void *hdr){
	void* tmp = (void*)(((cpc_ipv6_hdr_t*)hdr)->src);
	uint128__t src=*(uint128__t*)tmp;
	return src;
};

inline static
void set_ipv6_dst(void *hdr, uint128__t dst){
	uint128__t *ptr=(uint128__t*)&(((cpc_ipv6_hdr_t*)hdr)->dst);
	*ptr = dst;
};

inline static
uint128__t get_ipv6_dst(void *hdr){
	void * tmp = (((cpc_ipv6_hdr_t*)hdr)->dst);
	uint128__t dst=*(uint128__t*)tmp;
	return dst;
};


#endif //_CPC_IPV6_H_