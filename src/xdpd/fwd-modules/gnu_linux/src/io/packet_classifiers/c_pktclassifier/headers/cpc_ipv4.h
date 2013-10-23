#ifndef _CPC_IPV4_H_
#define _CPC_IPV4_H_

#include <stddef.h>
#include <rofl/common/endian_conversion.h>

/* IPv4 constants and definitions */
// IPv4 ethernet types
enum ipv4_ether_t {
	IPV4_ETHER = 0x0800,
};

enum ipv4_ip_proto_t {
	IPV4_IP_PROTO = 4,
};

// IPv4 header
struct cpc_ipv4_hdr_t {
	uint8_t ihlvers;        // IP header length in 32bit words,
							// TODO: check for options and padding
	uint8_t tos;
	uint16_t length;
	uint16_t ident;
	uint16_t offset_flags;
	uint8_t ttl;
	uint8_t proto;
	uint16_t checksum;
	uint32_t src;
	uint32_t dst;
	uint8_t data[0];
} __attribute__((packed));

enum ipv4_flag_t {
	bit_reserved 		= (1 << 0),
	bit_dont_fragment 	= (1 << 1),
	bit_more_fragments	= (1 << 2),
};

enum ip_proto_t {
	IPPROTO_ICMP = 1,
	IPPROTO_TCP = 6,
	IPPROTO_UDP = 17,
};

inline static
void ipv4_calc_checksum(void *hdr){
	//initialize();

	size_t datalen = sizeof(struct cpc_ipv4_hdr_t);

	// force header checksum to 0x0000
	((cpc_ipv4_hdr_t*)hdr)->checksum = 0;//htobe16(0x0000);

	// pointer on 16bit words
	uint16_t *word16 = (uint16_t*)hdr;
	// number of 16bit words
	int wnum = (datalen / (sizeof(uint16_t)));
	// sum
	uint32_t sum = 0;

	for (int i = 0; i < wnum; i++)
	{
		uint32_t tmp = (uint32_t)(be16toh(word16[i]));
		sum += tmp;
		//fprintf(stderr, "word16[%d]=0x%08x sum()=0x%08x\n", i, tmp, sum);
	}
	//fprintf(stderr, "   sum(1)=0x%x\n", sum);

	uint16_t res16 = (sum & 0x0000ffff) + ((sum & 0xffff0000) >> 16);

	//fprintf(stderr, " res16(1)=0x%x\n", res16);

	((cpc_ipv4_hdr_t*)hdr)->checksum = htobe16(~res16);

	//fprintf(stderr, "~res16(1)=0x%x\n", be16toh(ipv4_hdr->checksum));
};

inline static
void set_ipv4_src(void *hdr, uint32_t src){
	((cpc_ipv4_hdr_t*)hdr)->src = htobe32(src);
};

inline static
uint32_t get_ipv4_src(void *hdr){
	return betoh32(((cpc_ipv4_hdr_t*)hdr)->src);
};

inline static
void set_ipv4_dst(void *hdr, uint32_t dst){
	((cpc_ipv4_hdr_t*)hdr)->src = htobe32(dst);
};

inline static
uint32_t get_ipv4_dst(void *hdr){
	return betoh32(((cpc_ipv4_hdr_t*)hdr)->dst);
};

inline static
void set_ipv4_dscp(void *hdr, uint8_t dscp){
	((cpc_ipv4_hdr_t*)hdr)->tos = ((dscp & 0x3f) << 2) | (((cpc_ipv4_hdr_t*)hdr)->tos & 0x03);
};

inline static
uint8_t get_ipv4_dscp(void *hdr){
	return (((cpc_ipv4_hdr_t*)hdr)->tos >> 2);
};

inline static
void set_ipv4_ecn(void *hdr, uint8_t ecn){
	((cpc_ipv4_hdr_t*)hdr)->tos = (((cpc_ipv4_hdr_t*)hdr)->tos & 0xfc) | (ecn & 0x03);
};

inline static
uint8_t get_ipv4_ecn(void *hdr){
	return (((cpc_ipv4_hdr_t*)hdr)->tos & 0x03);
};

inline static
void set_ipv4_ttl(void *hdr, uint8_t ttl){
	((cpc_ipv4_hdr_t*)hdr)->ttl = ttl;
};

inline static
uint8_t get_ipv4_ttl(void *hdr){
	return ((cpc_ipv4_hdr_t*)hdr)->ttl;
};

inline static
void dec_ipv4_ttl(void *hdr){
	((cpc_ipv4_hdr_t*)hdr)->ttl--;
};

inline static
void set_ipv4_proto(void *hdr, uint8_t proto){
	((cpc_ipv4_hdr_t*)hdr)->proto = proto;
};

inline static
uint8_t get_ipv4_proto(void *hdr){
	return ((cpc_ipv4_hdr_t*)hdr)->proto;
};

inline static
void set_ipv4_ihl(void *hdr, uint8_t ihl){
	((cpc_ipv4_hdr_t*)hdr)->ihlvers = (((cpc_ipv4_hdr_t*)hdr)->ihlvers & 0xf0) + (ihl & 0x0f);
};

inline static
uint8_t get_ipv4_ihl(void *hdr){
	
};

inline static
void set_ipv4_version(void *hdr, uint8_t version){
	((cpc_ipv4_hdr_t*)hdr)->ihlvers = (((cpc_ipv4_hdr_t*)hdr)->ihlvers & 0x0f) + ((version & 0x0f) << 4);
};

inline static
uint8_t get_ipv4_version(void *hdr){
	return ((((cpc_ipv4_hdr_t*)hdr)->ihlvers & 0xf0) >> 4);
};

inline static
void set_ipv4_length(void *hdr, uint16_t length){
	((cpc_ipv4_hdr_t*)hdr)->length = htobe16(length);
};

inline static
uint32_t get_ipv4_length(void *hdr){
	return betoh16(((cpc_ipv4_hdr_t*)hdr)->length);
};

inline static
void set_DF_bit(void *hdr){
	((cpc_ipv4_hdr_t*)hdr)->offset_flags = htobe16( be16toh(((cpc_ipv4_hdr_t*)hdr)->offset_flags) | (bit_dont_fragment << 13) );
};

inline static
bool has_DF_bit_set(void *hdr){
	return (bool)((be16toh(((cpc_ipv4_hdr_t*)hdr)->offset_flags) >> 13)  & bit_dont_fragment);
};

inline static
void clear_DF_bit(void *hdr){
	((cpc_ipv4_hdr_t*)hdr)->offset_flags = htobe16( be16toh(((cpc_ipv4_hdr_t*)hdr)->offset_flags) & ~(bit_dont_fragment << 13) );
};

inline static
void set_MF_bit(void *hdr){
	((cpc_ipv4_hdr_t*)hdr)->offset_flags = htobe16( be16toh(((cpc_ipv4_hdr_t*)hdr)->offset_flags) | (bit_more_fragments << 13) );
};

inline static
bool has_MF_bit_set(void *hdr){
	return (bool)((be16toh(((cpc_ipv4_hdr_t*)hdr)->offset_flags) >> 13)  & bit_more_fragments);
};

inline static
void clear_MF_bit(void *hdr){
	((cpc_ipv4_hdr_t*)hdr)->offset_flags = htobe16( be16toh(((cpc_ipv4_hdr_t*)hdr)->offset_flags) & ~(bit_more_fragments << 13) );
};












#endif //_CPC_IPV4_H_