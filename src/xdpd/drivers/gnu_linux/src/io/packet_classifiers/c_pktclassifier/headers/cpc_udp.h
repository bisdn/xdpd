/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_UDP_H_
#define _CPC_UDP_H_

/**
* @file cpc_udp.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for UDP
*/

/* UDP constants and definitions */
typedef struct cpc_udp_hdr {
	uint16_t sport;
	uint16_t dport;
	uint16_t length;
	uint16_t checksum;
	uint8_t data[0];
} __attribute__((packed)) cpc_udp_hdr_t;

enum udp_ip_proto_t {
	UDP_IP_PROTO = 17,
};

inline static
void udpv4_calc_checksum(void* hdr, uint32_t ip_src, uint32_t ip_dst, uint8_t ip_proto, uint16_t length){
	int wnum;
	int i;
	uint32_t sum = 0; //sum
	uint16_t* word16;
	
	//Set 0 to checksum
	((cpc_udp_hdr_t*)hdr)->checksum = 0x0;

	/*
	* part -I- (IPv4 pseudo header)
	*/

#if 0
	// this seems to be wrong ...
	word16 = (uint16_t*)(void*)&ip_src;
	sum += *(word16+1);
	sum += *(word16);

	word16 = (uint16_t*)(void*)&ip_dst;
	sum += *(word16+1);
	sum += *(word16);

	sum += htobe16(ip_proto);
	sum += htobe16(length); 
#else
	word16 = (uint16_t*)(&((uint8_t*)&ip_src)[0]);
	sum += *word16;
	//fprintf(stderr, "uint16_t: 0x%04x sum: 0x%08x\n", *word16, sum);
	word16 = (uint16_t*)(&((uint8_t*)&ip_src)[2]);
	sum += *word16;
	//fprintf(stderr, "uint16_t: 0x%04x sum: 0x%08x\n", *word16, sum);

	word16 = (uint16_t*)(&((uint8_t*)&ip_dst)[0]);
	sum += *word16;
	//fprintf(stderr, "uint16_t: 0x%04x sum: 0x%08x\n", *word16, sum);
	word16 = (uint16_t*)(&((uint8_t*)&ip_dst)[2]);
	sum += *word16;
	//fprintf(stderr, "uint16_t: 0x%04x sum: 0x%08x\n", *word16, sum);
#endif

	sum += htobe16((uint16_t)ip_proto);
	//fprintf(stderr, "sum: 0x%08x\n", sum);
	sum += htobe16(length);
	//fprintf(stderr, "sum: 0x%08x\n", sum);


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

	((cpc_udp_hdr_t*)hdr)->checksum = (uint16_t)~sum; // correct: this inserts the checksum in network-byte-order

//	fprintf(stderr," %x \n", udp_hdr->checksum);
}

inline static
void udpv6_calc_checksum(void* hdr, uint128__t ip_src, uint128__t ip_dst, uint8_t ip_proto, uint16_t length){
	int wnum;
	int i;
	uint32_t sum = 0; //sum
	uint16_t* word16;

	//Set 0 to checksum
	((cpc_udp_hdr_t*)hdr)->checksum = 0x0;

	/*
	* part -I- (IPv6 pseudo header)
	*/
	for (i = 0; i < 8; i++) {
		sum += *((uint16_t*)&(ip_src.val[2*i]));
	}

	for (i = 0; i < 8; i++) {
		sum += *((uint16_t*)&(ip_dst.val[2*i]));
	}

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

	((cpc_udp_hdr_t*)hdr)->checksum =(uint16_t) ~sum;

//	fprintf(stderr," %x \n", udp_hdr->checksum);
}

inline static
uint16_t* get_udp_sport(void *hdr){
	return &((cpc_udp_hdr_t*)hdr)->sport;
}

inline static
void set_udp_sport(void *hdr, uint16_t port){
	((cpc_udp_hdr_t*)hdr)->sport = port;
}

inline static
uint16_t* get_udp_dport(void *hdr){
	return &((cpc_udp_hdr_t*)hdr)->dport;
}

inline static
void set_udp_dport(void *hdr, uint16_t port){
	((cpc_udp_hdr_t*)hdr)->dport = port;
}

inline static
uint16_t* get_udp_length(void *hdr){
	return &((cpc_udp_hdr_t*)hdr)->length;
}

inline static
void set_udp_length(void *hdr, uint16_t length){
	((cpc_udp_hdr_t*)hdr)->length = length;
}


#endif //_CPC_UDP_H_
