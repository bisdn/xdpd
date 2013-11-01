#ifndef _CPC_IPV4_H_
#define _CPC_IPV4_H_

#include "../cpc_utils.h"


struct stat;
struct cpc_ipv6_ext_hdr_t {
	uint8_t nxthdr;
	uint8_t len;
	uint8_t data[0];
} __attribute__((packed));

#define IPV6_ADDR_LEN	16
#define IPV6_VERSION 	 6

/* ipv6 constants and definitions */
// ipv6 ethernet types
enum ipv6_ether_t {
	IPV6_ETHER = 0x86dd,
};

// IPv6 header
struct cpc_ipv6_hdr_t {
	uint8_t bytes[4];      	// version + tc + flow-label
	uint16_t payloadlen;
	uint8_t nxthdr;
	uint8_t hoplimit;
	uint8_t src[IPV6_ADDR_LEN];
	uint8_t dst[IPV6_ADDR_LEN];
	uint8_t data[0];
} __attribute__((packed));

enum ipv6_ext_t {
	IPPROTO_IPV6_HOPOPT 		= 0,
	IPPROTO_ICMP 				= 1,
	IPPROTO_TCP 				= 6,
	IPPROTO_UDP 				= 17,
	IPV6_IP_PROTO 				= 41,
	IPPROTO_IPV6_ROUTE			= 43,
	IPPROTO_IPV6_FRAG			= 44,
	IPPROTO_IPV6_ICMP			= 58,
	IPPROTO_IPV6_NONXT			= 59,
	IPPROTO_IPV6_OPTS			= 60,
	IPPROTO_IPV6_MIPV6			= 135,
};
/* ipv6 definitions */

struct cpc_ipv6_hdr_t 					*ipv6_hdr;		// pointer to pppoe header
uint8_t 								*ipv6data;		// payload data
size_t 					 				 ipv6datalen;	// ppp data length
std::map<enum ipv6_ext_t, fipv6ext> 	 ipv6exts;		// IPv6 extensions headers

//TODO extension headers

inline static
void ipv6_calc_checksum(void *hdr, uint16_t length){
	//TODO implement checksum calulation
};


inline static
void set_version(void *hdr, uint8_t version){
	((cpc_ipv6_hdr_t*)hdr)->bytes[0] = (((cpc_ipv6_hdr_t*)hdr)->bytes[0] & 0x0F) + ((version & 0x0F) << 4);
};

inline static
uint8_t get_version(void *hdr){
	return (uint8_t)((((cpc_ipv6_hdr_t*)hdr)->bytes[0] & 0xF0) >> 4);
};

inline static
void set_traffic_class(void *hdr, uint8_t tc){
	((cpc_ipv6_hdr_t*)hdr)->bytes[0] = (((cpc_ipv6_hdr_t*)hdr)->bytes[0] & 0xF0) + ((tc & 0xF0) >> 4);
	((cpc_ipv6_hdr_t*)hdr)->bytes[1] = (((cpc_ipv6_hdr_t*)hdr)->bytes[1] & 0x0F) + ((tc & 0x0F) << 4);
}

inline static
uint8_t get_traffic_class(void *hdr){
	return (uint8_t)(((((cpc_ipv6_hdr_t*)hdr)->bytes[0] & 0x0F) << 4) + ((((cpc_ipv6_hdr_t*)hdr)->bytes[1] & 0xF0) >> 4));
};

inline static
void set_dscp(void *hdr, uint8_t dscp){
	((cpc_ipv6_hdr_t*)hdr)->bytes[0] = (((cpc_ipv6_hdr_t*)hdr)->bytes[0] & 0xF0) + ((dscp & 0x3C) >> 2);
	((cpc_ipv6_hdr_t*)hdr)->bytes[1] = (((cpc_ipv6_hdr_t*)hdr)->bytes[1] & 0x3F) + ((dscp & 0x03) << 6);
}

inline static
uint8_t get_dscp(void *hdr){
	return (uint8_t)(((((cpc_ipv6_hdr_t*)hdr)->bytes[0] & 0x0F) << 2) + ((((cpc_ipv6_hdr_t*)hdr)->bytes[1] & 0xC0) >> 6));
};

inline static
void set_ecn(void *hdr, uint8_t ecn){
	((cpc_ipv6_hdr_t*)hdr)->bytes[1] = (((cpc_ipv6_hdr_t*)hdr)->bytes[1] & 0xCF) + ((ecn & 0x03) << 4);
}

inline static
uint8_t get_ecn(void *hdr){
	return (uint8_t)((((cpc_ipv6_hdr_t*)hdr)->bytes[1] & 0x30) >> 4);
};

inline static
void set_flow_label(void *hdr, uint32_t flabel){
	((cpc_ipv6_hdr_t*)hdr)->bytes[1] = (((cpc_ipv6_hdr_t*)hdr)->bytes[1] & 0xF0) + ((flabel & 0x000f0000) >> 16);
	((cpc_ipv6_hdr_t*)hdr)->bytes[2] = (flabel & 0x0000ff00) >> 8;
	((cpc_ipv6_hdr_t*)hdr)->bytes[3] = (flabel & 0x000000ff) >> 0;
}

inline static
uint32_t get_flow_label(void *hdr){
	return (uint32_t)(((((cpc_ipv6_hdr_t*)hdr)->bytes[1] & 0x0F) << 16) + (((cpc_ipv6_hdr_t*)hdr)->bytes[2] << 8) + (((cpc_ipv6_hdr_t*)hdr)->bytes[3] << 0));
};

inline static
void set_payload_length(void *hdr, uint16_t len){
	((cpc_ipv6_hdr_t*)hdr)->payloadlen = CPC_HTOBE16(len);
}

inline static
uint16_t get_payload_length(void *hdr){
	return CPC_BE16TOH(((cpc_ipv6_hdr_t*)hdr)->payloadlen);
};

inline static
void set_next_header(void *hdr, uint8_t nxthdr){
	((cpc_ipv6_hdr_t*)hdr)->nxthdr = nxthdr;
}

inline static
uint8_t get_next_header(void *hdr){
	return ((cpc_ipv6_hdr_t*)hdr)->nxthdr;
};

inline static
void set_hop_limit(void *hdr, uint8_t hops){
	((cpc_ipv6_hdr_t*)hdr)->hoplimit = hops;
}

inline static
uint8_t get_hop_limit(void *hdr){
	return ((cpc_ipv6_hdr_t*)hdr)->hoplimit;
};

inline static
void dec_hop_limit(void *hdr){
	((cpc_ipv6_hdr_t*)hdr)->hoplimit--;
};

inline static
void set_ipv6_src(void *hdr, uint128__t src){
	CPC_SWAP_U128(src);//htobe128
	uint128__t *ptr=(uint128__t*)&(((cpc_ipv6_hdr_t*)hdr)->src);
	*ptr = src;
};

inline static
uint128__t get_ipv6_src(void *hdr){
	uint128__t src=(uint128__t)(((cpc_ipv6_hdr_t*)hdr)->src);
	CPC_SWAP_U128(src);//htobe128
	return src;
};

inline static
void set_ipv6_dst(void *hdr, uint128__t dst){
	CPC_SWAP_U128(dst);//htobe128
	uint128__t *ptr=(uint128__t*)&(((cpc_ipv6_hdr_t*)hdr)->dst);
	*ptr = dst;
};

inline static
uint128__t get_ipv6_dst(void *hdr){
	uint128__t dst=(uint128__t)(((cpc_ipv6_hdr_t*)hdr)->dst);
	CPC_SWAP_U128(dst);//htobe128
	return dst;
};


#endif //_CPC_IPV4_H_