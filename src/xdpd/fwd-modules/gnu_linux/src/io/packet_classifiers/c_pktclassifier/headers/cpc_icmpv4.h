#ifndef _CPC_ICMPV4_H_
#define _CPC_ICMPV4_H_

#include "../cpc_utils.h"

/* ICMPv4 constants and definitions */
struct cpc_icmpv4_hdr {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint8_t data[0];
} __attribute__((packed));

/* for UDP checksum calculation */
struct cpc_ip_pseudo_hdr_t {
	uint32_t src;
	uint32_t dst;
	uint8_t reserved;
	uint8_t proto;
	uint16_t len;
} __attribute__((packed));

typedef struct cpc_icmpv4_hdr cpc_icmpv4_hdr_t;

enum icmpv4_ip_proto_t {
	ICMPV4_IP_PROTO = 1,
};

enum icmpv4_type_t {
	ICMP_TYPE_DESTINATION_UNREACHABLE = 3,
	ICMP_TYPE_ECHO_REPLY = 0,
	ICMP_TYPE_ECHO_REQUEST = 8,
};

enum icmpv4_code_t {
	ICMP_CODE_HOST_UNREACHABLE = 1,
	ICMP_CODE_NO_CODE = 0,
	ICMP_CODE_DATAGRAM_TOO_BIG = 4,
};


inline static
uint8_t get_icmp_type(void *hdr){
	return ((cpc_icmpv4_hdr_t*)hdr)->type;
};

inline static
void set_icmp_type(void *hdr, uint8_t type){
	((cpc_icmpv4_hdr_t*)hdr)->type = type;
};

inline static
uint8_t get_icmp_code(void *hdr){
	return ((cpc_icmpv4_hdr_t*)hdr)->code;
};

inline static
void set_icmp_code(void *hdr, uint8_t code){
	((cpc_icmpv4_hdr_t*)hdr)->code = code;
};

inline static
uint16_t get_checksum(void *hdr){
	return CPC_BE16TOH(((cpc_icmpv4_hdr_t*)hdr)->checksum);
};

inline static
void icmpv4_calc_checksum(void * hdr,uint16_t length){
	int i;
	//initialize();
	((cpc_icmpv4_hdr_t *)hdr)->checksum = 0; //htobe16(0x0000);

	// ICMPv4 header

	// sum
	uint32_t sum = 0;

	// pointer on 16bit words
	uint16_t *word16 = (uint16_t*)hdr;

	// number of 16bit words
	//int wnum = (sizeof(struct icmpv4_hdr_t) / sizeof(uint16_t));
    int wnum = (length / sizeof(uint16_t));
	// header loop
	for (i = 0; i < wnum; i++)
	{
		uint32_t tmp = (uint32_t)(CPC_BE16TOH(word16[i]));
		sum += tmp;
		//fprintf(stderr, "word16[%d]=0x%08x sum()=0x%08x\n", i, tmp, sum);
	}
	// TODO: checksum must also cover data portion of ICMP message!

	uint16_t res16 = (sum & 0x0000ffff) + ((sum & 0xffff0000) >> 16);
	//fprintf(stderr, " res16(1)=0x%x\n", res16);

	((cpc_icmpv4_hdr_t *)hdr)->checksum = CPC_HTOBE16(~res16);
	//fprintf(stderr, "~res16(1)=0x%x\n", be16toh(udp_hdr->checksum));
};

#endif //_CPC_ICMPV4_H_
