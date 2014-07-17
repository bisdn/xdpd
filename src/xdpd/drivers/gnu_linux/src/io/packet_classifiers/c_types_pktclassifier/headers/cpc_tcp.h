/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_TCP_H_
#define _CPC_TCP_H_

/**
* @file cpc_tcp.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for TCP
*/

typedef struct cpc_tcp_hdr {
	uint16_t sport;
	uint16_t dport;
	uint32_t seqno;
	uint32_t ackno;
#ifdef __BIG_ENDIAN
	uint16_t offset   : 4; // =5 => 5 32bit words (= 20 bytes), >5 => options appended
	uint16_t reserved : 3;
	uint16_t ns       : 1;
	/* byte */
	uint16_t cwr      : 1;
	uint16_t ece	  : 1;
	uint16_t urg	  : 1;
	uint16_t ack	  : 1;
	uint16_t psh	  : 1;
	uint16_t rst	  : 1;
	uint16_t syn	  : 1;
	uint16_t fin	  : 1;
#elif __LITTLE_ENDIAN
	uint16_t cwr      : 1;
	uint16_t ece	  : 1;
	uint16_t urg	  : 1;
	uint16_t ack	  : 1;
	uint16_t psh	  : 1;
	uint16_t rst	  : 1;
	uint16_t syn	  : 1;
	uint16_t fin	  : 1;
	/* byte */
	uint16_t offset   : 4; // =5 => 5 32bit words (= 20 bytes), >5 => options appended
	uint16_t reserved : 3;
	uint16_t ns       : 1;
#endif
	uint16_t wnd;
	uint16_t checksum;
	uint16_t urgent;
	uint8_t data[0];
} __attribute__((packed)) cpc_tcp_hdr_t;

enum tcp_ip_proto_t {
	TCP_IP_PROTO = 6,
};

inline static
uint16_t ietf_rfc1071_checksum_hbo(uint8_t* buf, size_t buflen)
{
	/* Compute Internet Checksum for "buflen" bytes
	 *         beginning at location "buf".
	 *  C "reference" implementation defined in IETF RFC 1071
	 */
	register uint32_t sum = 0;

	//fprintf(stderr, "==> calculating checksum in hbo <==\n");

	while( buflen > 1 )  {
		/*  This is the inner loop */

		// this 16bit word contains now the value in host-byte-order
		uint16_t word16 = (buf[1] << 8) + buf[0];
		sum += word16;
		buf += 2;
		buflen -= 2;
		//fprintf(stderr, "uint16_t: 0x%04x sum: 0x%08x\n", word16, sum);
	}

	/*  Add left-over byte, if any */
	if( buflen > 0 ) {
		uint16_t word16 = (uint16_t)(buf[0]);
		sum += word16;
	}

   /*  Fold 32-bit sum to 16 bits */
   while (sum>>16)
	   sum = (sum & 0xffff) + (sum >> 16);

   //fprintf(stderr, "folded sum: 0x%08x\n", sum);

   uint16_t checksum = ~sum;

   //fprintf(stderr, "complemented sum (hbo): 0x%08x\n", checksum);

   checksum = htobe16(checksum);

   //fprintf(stderr, "complemented sum (nbo): 0x%08x\n", checksum);

   return checksum; // return value in hbo
}

inline static
uint16_t ietf_rfc1071_checksum_nbo(uint8_t* buf, size_t buflen)
{
	/* Compute Internet Checksum for "buflen" bytes
	 *         beginning at location "buf".
	 *  C "reference" implementation defined in IETF RFC 1071
	 */
	register uint32_t sum = 0;

	//fprintf(stderr, "==> calculating checksum in nbo <==\n");

	while( buflen > 1 )  {
		/*  This is the inner loop */

		// this 16bit word contains now the value in network-byte-order
		uint16_t word16 = (buf[0] << 8) + buf[1];
		sum += word16;
		buf += 2;
		buflen -= 2;
		//fprintf(stderr, "uint16_t: 0x%04x sum: 0x%08x\n", word16, sum);
	}

	/*  Add left-over byte, if any */
	if( buflen > 0 ) {
		uint16_t word16 = (buf[0] << 8);
		sum += word16;
	}

   /*  Fold 32-bit sum to 16 bits */
   while (sum>>16)
	   sum = (sum & 0xffff) + (sum >> 16);

   //fprintf(stderr, "folded sum: 0x%08x\n", sum);

   uint16_t checksum = ~sum;

   //fprintf(stderr, "complemented sum (nbo): 0x%08x\n", checksum);

   return checksum; // return value in hbo
}

inline static
void tcpv4_calc_checksum(void* hdr, /*nbo*/uint32_t ip_src, /*nbo*/uint32_t ip_dst, /*hbo ;)*/uint8_t ip_proto, /*hbo*/uint16_t length){
	int wnum;
	int i;
	uint32_t sum = 0; //sum
	uint16_t* word16 = (uint16_t*)0;

	//Set 0 to checksum
	((cpc_tcp_hdr_t*)hdr)->checksum = 0x0;

	/*
	* part -I- (IPv4 pseudo header)
	*/

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

	sum += htobe16((uint16_t)ip_proto);
	//fprintf(stderr, "uint16_t: 0x%04x sum: 0x%08x (ip-proto)\n", ip_proto, sum);
	sum += htobe16(length);
	//fprintf(stderr, "uint16_t: 0x%04x sum: 0x%08x (length)\n", length, sum);

	/*
	* part -II- (TCP header + payload)
	*/

	// pointer on 16bit words
	// number of 16bit words
	word16 = (uint16_t*)hdr;
	wnum = (length / sizeof(uint16_t));

	for (i = 0; i < wnum; i++){
		sum += (uint32_t)word16[i];
		//fprintf(stderr, "uint16_t: 0x%04x sum: 0x%08x\n", word16[i], sum);
	}

	if(length & 0x1)
		//Last byte
		sum += (uint32_t)( ((uint8_t*)hdr)[length-1]);

	//Fold it
	do{
		sum = (sum & 0xFFFF)+(sum >> 16);
	}while (sum >> 16);

	((cpc_tcp_hdr_t*)hdr)->checksum = (uint16_t)~sum; // correct: this inserts the checksum in network-byte-order

	//fprintf(stderr,"cksum (nbo):  0x04%x \n", ((cpc_tcp_hdr_t*)hdr)->checksum);
}

inline static
void tcpv6_calc_checksum(void* hdr, uint128__t ip_src, uint128__t ip_dst, uint8_t ip_proto, uint16_t length){
	int wnum;
	int i;
	uint32_t sum = 0; //sum
	uint16_t* word16;
	
	//Set 0 to checksum
	((cpc_tcp_hdr_t*)hdr)->checksum = 0x0;

	/*
	* part -I- (IPv6 pseudo header)
	*/
	for (i = 0; i < 8; i++) {
		sum += (((uint16_t)ip_src.val[2*i]) << 0) + (((uint16_t)ip_src.val[2*i+1]) << 8);
	}

	for (i = 0; i < 8; i++) {
		sum += (((uint16_t)ip_dst.val[2*i]) << 0) + (((uint16_t)ip_dst.val[2*i+1]) << 8);
	}

	sum += htobe16((uint16_t)ip_proto);
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
		sum += (uint32_t)( ((uint8_t*)hdr)[length-1]);

	//Fold it
	do{
		sum = (sum & 0xFFFF)+(sum >> 16);
	}while (sum >> 16); 

	((cpc_tcp_hdr_t*)hdr)->checksum =(uint16_t) ~sum;

//	fprintf(stderr," %x \n", tcp_hdr->checksum);
}

inline static
uint16_t* get_tcp_sport(void *hdr){
	return &((cpc_tcp_hdr_t*)hdr)->sport;
}

inline static
void set_tcp_sport(void *hdr, uint16_t port){
	((cpc_tcp_hdr_t*)hdr)->sport = port;
}

inline static
uint16_t* get_tcp_dport(void *hdr){
	return &((cpc_tcp_hdr_t*)hdr)->dport;
}

inline static
void set_tcp_dport(void *hdr, uint16_t port){
	((cpc_tcp_hdr_t*)hdr)->dport = port;
}


#endif //_CPC_TCP_H_
