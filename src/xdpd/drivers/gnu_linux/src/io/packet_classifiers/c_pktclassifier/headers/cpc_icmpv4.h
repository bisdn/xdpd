/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_ICMPV4_H_
#define _CPC_ICMPV4_H_

#include "../cpc_utils.h"

/**
* @file cpc_icmpv4.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for ICMPv4
*/

/* ICMPv4 constants and definitions */
typedef struct cpc_icmpv4_hdr {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint8_t data[0];
} __attribute__((packed)) cpc_icmpv4_hdr_t;

enum icmpv4_ip_proto_t {
	ICMPV4_IP_PROTO = 1,
};

inline static
uint8_t get_icmpv4_type(void *hdr){
	return ((cpc_icmpv4_hdr_t*)hdr)->type;
};

inline static
void set_icmpv4_type(void *hdr, uint8_t type){
	((cpc_icmpv4_hdr_t*)hdr)->type = type;
};

inline static
uint8_t get_icmpv4_code(void *hdr){
	return ((cpc_icmpv4_hdr_t*)hdr)->code;
};

inline static
void set_icmpv4_code(void *hdr, uint8_t code){
	((cpc_icmpv4_hdr_t*)hdr)->code = code;
};

inline static
uint16_t get_icmpv4_checksum(void *hdr){
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
