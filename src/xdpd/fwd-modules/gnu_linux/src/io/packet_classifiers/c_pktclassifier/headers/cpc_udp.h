#ifndef _CPC_UDP_H_
#define _CPC_UDP_H_

#include <rofl/common/endian_conversion.h>

/* UDP constants and definitions */
struct cpc_udp_hdr_t {
	uint16_t sport;
	uint16_t dport;
	uint16_t length;
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

enum udp_ip_proto_t {
	UDP_IP_PROTO = 17,
};

inline static
void udp_calc_checksum(void* hdr, uint32_t ip_src, uint32_t ip_dst, uint8_t ip_proto, uint16_t length){
	int wnum;
	uint32_t sum = 0; //sum
	uint16_t* word16;
	
	//Set 0 to checksum
	((cpc_udp_hdr_t*)hdr)->checksum = 0x0;

	/*
	* part -I- (IPv4 pseudo header)
	*/
	
	word16 = (uint16_t*)(void*)&ip_src; //NOTE endianess
	sum += *(word16+1);
	sum += *(word16);

	word16 = (uint16_t*)(void*)&ip_dst; //NOTE endianess
	sum += *(word16+1);
	sum += *(word16);
	sum += htons(ip_proto);
	sum += htons(length); 

	/*
	* part -II- (TCP header + payload)
	*/
	
	// pointer on 16bit words
	// number of 16bit words
	word16 = (uint16_t*)hdr;
	wnum = (length / sizeof(uint16_t));

	for (int i = 0; i < wnum; i++){
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
uint16_t get_sport(viod *hdr){
	return be16toh(((cpc_udp_hdr_t*)hdr)->sport);
}

inline static
void set_sport(viod *hdr, uint16_t port){
	((cpc_udp_hdr_t*)hdr)->sport = htobe16(port);
}

inline static
uint16_t get_dport(viod *hdr){
	return be16toh(((cpc_udp_hdr_t*)hdr)->dport);
}

inline static
void set_dport(viod *hdr, uint16_t port){
	((cpc_udp_hdr_t*)hdr)->dport = htobe16(port);
}

inline static
uint16_t get_length(viod *hdr){
	return be16toh(((cpc_udp_hdr_t*)hdr)->length);
}

inline static
void set_length(viod *hdr, uint16_t length){
	((cpc_udp_hdr_t*)hdr)->length = htobe16(length);
}


#endif //_CPC_UDP_H_
