/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_SCTP_H_
#define _CPC_SCTP_H_

/**
* @file cpc_sctp.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for SCTP
*/

/* SCTP constants and definitions */
typedef struct cpc_sctp_hdr {
	uint16_t sport;
	uint16_t dport;
	uint32_t verification_tag;
	uint32_t checksum;
	uint8_t data[0];
} __attribute__((packed)) cpc_sctp_hdr_t;

enum sctp_ip_proto_t {
	SCTP_IP_PROTO = 132,
};

inline static
void sctp_calc_checksum(void* hdr, uint32_t ip_src, uint32_t ip_dst, uint8_t ip_proto, uint16_t length){

	/*
	 * TODO: check whether this checksum calculation scheme also applies to SCTP
	 */

	int wnum;
	int i;
	uint32_t sum = 0; //sum
	uint16_t* word16;
	
	//Set 0 to checksum
	((cpc_sctp_hdr_t*)hdr)->checksum = 0x0;

	/*
	* part -I- (IPv4 pseudo header)
	*/
	
	word16 = (uint16_t*)(void*)&ip_src;
	sum += *(word16+1);
	sum += *(word16);

	word16 = (uint16_t*)(void*)&ip_dst;
	sum += *(word16+1);
	sum += *(word16);
	sum += htobe16(ip_proto);
	sum += htobe16(length); 

	/*
	* part -II- (TCP header + payload)
	*/
	
	// pointer on 16bit words
	// number of 16bit words
	word16 = (uint16_t*)hdr;
	wnum = (length / sizeof(uint16_t));

	for (i = 0; i < wnum; i++){
		sum += (uint32_t)word16[i];
	}
	
	if(length & 0x1)
		//Last byte
		sum += (uint32_t)( ((uint8_t*)(void*)hdr)[length-1]);

	//Fold it
	do{
		sum = (sum & 0xFFFF)+(sum >> 16);
	}while (sum >> 16); 

	((cpc_sctp_hdr_t*)hdr)->checksum =(uint16_t) ~sum;

//	fprintf(stderr," %x \n", sctp_hdr->checksum);
}

inline static
uint16_t* get_sctp_sport(void *hdr){
	return &((cpc_sctp_hdr_t*)hdr)->sport;
}

inline static
void set_sctp_sport(void *hdr, uint16_t port){
	((cpc_sctp_hdr_t*)hdr)->sport = port;
}

inline static
uint16_t* get_sctp_dport(void *hdr){
	return &((cpc_sctp_hdr_t*)hdr)->dport;
}

inline static
void set_sctp_dport(void *hdr, uint16_t port){
	((cpc_sctp_hdr_t*)hdr)->dport = port;
}

#if 0
inline static
uint16_t* get_sctp_length(void *hdr){
	return &((cpc_sctp_hdr_t*)hdr)->length;
}

inline static
void set_sctp_length(void *hdr, uint16_t length){
	((cpc_sctp_hdr_t*)hdr)->length = length;
}
#endif

#endif //_CPC_SCTP_H_
