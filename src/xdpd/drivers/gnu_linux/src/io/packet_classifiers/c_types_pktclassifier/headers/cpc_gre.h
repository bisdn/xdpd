/*
 * cpc_gre.h
 *
 *  Created on: 23.09.2014
 *      Author: andreas
 */

#ifndef CPC_GRE_H_
#define CPC_GRE_H_

#include <rofl/datapath/pipeline/common/endianness.h>

#if defined(BIG_ENDIAN_DETECTED)
#define GRE_CSUM_FLAG_MASK		0x8000 // 1000 0000 0000 0000
#define GRE_KEY_FLAG_MASK		0x2000 // 0010 0000 0000 0000
#define GRE_SEQNO_FLAG_MASK		0x1000 // 0001 0000 0000 0000
#define GRE_RESERVED0_MASK 		0x7FF8 // 0111 1111 1111 1000
#define GRE_VERSION_MASK		0x0003 // 0000 0000 0000 0111
#define GRE_PROT_TYPE_TRANSPARENT_BRIDGING 0x6558 // as defined in IETF RFC 1701
#elif defined(LITTLE_ENDIAN_DETECTED)
#define GRE_CSUM_FLAG_MASK		0x0000000000000080
#define GRE_KEY_FLAG_MASK		0x0000000000000020
#define GRE_SEQNO_FLAG_MASK		0x0000000000000010
#define GRE_RESERVED0_MASK 		0x000000000000F87F
#define GRE_VERSION_MASK		0x0000000000000300
#define GRE_PROT_TYPE_TRANSPARENT_BRIDGING 0x0000000000005865
#else
	#error Unknwon endianness
#endif

/**
* @file cpc_udp.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for GRE (IETF RFC 2784 and RFC 2890)
*/


/* base GRE header according to IETF RFC 2784 without checksum and preceding fields */
typedef struct cpc_gre_base_hdr {
	uint16_t word0;
	uint16_t prot_type;
	uint8_t data[0];
} __attribute__((packed)) cpc_gre_base_hdr_t;

/* full GRE header according to IETF RFC 2784 including checksum and reserved field */
typedef struct cpc_gre_csum_hdr {
	uint16_t word0;
	uint16_t prot_type;
	uint16_t checksum;
	uint16_t reserved1;
	uint8_t data[0];
} __attribute__((packed)) cpc_gre_csum_hdr_t;

/* full GRE header according to IETF RFC 2784 including checksum and reserved field */
typedef struct cpc_gre_key_hdr {
	uint16_t word0;
	uint16_t prot_type;
	uint16_t checksum;
	uint16_t reserved1;
	uint32_t key;
	uint8_t data[0];
} __attribute__((packed)) cpc_gre_key_hdr_t;

/* full GRE header according to IETF RFC 2890 including checksum, reserved field, key and sequence number */
typedef struct cpc_gre_hdr {
	uint16_t word0;
	uint16_t prot_type;
	uint16_t checksum;
	uint16_t reserved1;
	uint32_t key;
	uint32_t seqno;
	uint8_t data[0];
} __attribute__((packed)) cpc_gre_hdr_t;

enum gre_ip_proto_t {
	GRE_IP_PROTO = 47,
};

inline static
void grev4_calc_checksum(void* hdr, uint32_t ip_src, uint32_t ip_dst, uint8_t ip_proto, uint16_t length){
	int wnum;
	int i;
	uint32_t sum = 0; //sum
	uint16_t* word16;

	//Set 0 to checksum
	((cpc_gre_hdr_t*)hdr)->checksum = 0x0;

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

	sum += HTONB16(ip_proto);
	sum += HTONB16(length);
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

	sum += HTONB16((uint16_t)ip_proto);
	//fprintf(stderr, "sum: 0x%08x\n", sum);
	sum += HTONB16(length);
	//fprintf(stderr, "sum: 0x%08x\n", sum);


	/*
	* part -II- (GRE header + payload)
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

	((cpc_gre_hdr_t*)hdr)->checksum = (uint16_t)~sum; // correct: this inserts the checksum in network-byte-order

//	fprintf(stderr," %x \n", gre_hdr->checksum);
}

inline static
void grev6_calc_checksum(void* hdr, uint128__t ip_src, uint128__t ip_dst, uint8_t ip_proto, uint16_t length){
	int wnum;
	int i;
	uint32_t sum = 0; //sum
	uint16_t* word16;

	//Set 0 to checksum
	((cpc_gre_hdr_t*)hdr)->checksum = 0x0;

	/*
	* part -I- (IPv6 pseudo header)
	*/
	for (i = 0; i < 8; i++) {
		sum += (((uint16_t)ip_src.val[2*i]) << 0) + (((uint16_t)ip_src.val[2*i+1]) << 8);
	}

	for (i = 0; i < 8; i++) {
		sum += (((uint16_t)ip_dst.val[2*i]) << 0) + (((uint16_t)ip_dst.val[2*i+1]) << 8);
	}

	sum += HTONB16(ip_proto);
	sum += HTONB16(length);

	/*
	* part -II- (GRE header + payload)
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

	((cpc_gre_hdr_t*)hdr)->checksum =(uint16_t) ~sum;

//	fprintf(stderr," %x \n", gre_hdr->checksum);
}

inline static
void set_gre_csum_flag(void* hdr, uint16_t flag){
	((cpc_gre_base_hdr_t*)hdr)->word0 = ((flag & GRE_CSUM_FLAG_MASK) | (((cpc_gre_base_hdr_t*)hdr)->word0 & ~GRE_CSUM_FLAG_MASK));
}

inline static
uint16_t* get_gre_csum_flag(void* hdr){
	return &((cpc_gre_base_hdr_t*)hdr)->word0;
}

inline static
void set_gre_key_flag(void* hdr, uint16_t flag){
	((cpc_gre_base_hdr_t*)hdr)->word0 = ((flag & GRE_KEY_FLAG_MASK) | (((cpc_gre_base_hdr_t*)hdr)->word0 & ~GRE_KEY_FLAG_MASK));
}

inline static
uint16_t* get_gre_key_flag(void* hdr){
	return &((cpc_gre_base_hdr_t*)hdr)->word0;
}

inline static
void set_gre_seqno_flag(void* hdr, uint16_t flag){
	((cpc_gre_base_hdr_t*)hdr)->word0 = ((flag & GRE_SEQNO_FLAG_MASK) | (((cpc_gre_base_hdr_t*)hdr)->word0 & ~GRE_SEQNO_FLAG_MASK));
}

inline static
uint16_t* get_gre_seqno_flag(void* hdr){
	return &((cpc_gre_base_hdr_t*)hdr)->word0;
}

inline static
void set_gre_reserved0(void *hdr, uint16_t reserved0){
	// Must be 0 according to RFC 2784
	((cpc_gre_base_hdr_t*)hdr)->word0 = ((reserved0 & GRE_RESERVED0_MASK) | (((cpc_gre_base_hdr_t*)hdr)->word0 & ~GRE_RESERVED0_MASK));
}

inline static
uint16_t* get_gre_reserved0(void *hdr){
	return &((cpc_gre_base_hdr_t*)hdr)->word0;
}

inline static
void set_gre_version(void* hdr, uint16_t version){
	((cpc_gre_base_hdr_t*)hdr)->word0 = ((version & GRE_VERSION_MASK) | (((cpc_gre_base_hdr_t*)hdr)->word0 & ~GRE_VERSION_MASK));
}

inline static
uint16_t* get_gre_version(void* hdr){
	return &((cpc_gre_base_hdr_t*)hdr)->word0;
}

inline static
uint16_t* get_gre_prot_type(void *hdr){
	return &((cpc_gre_base_hdr_t*)hdr)->prot_type;
}

inline static
void set_gre_prot_type(void *hdr, uint16_t prot_type){
	((cpc_gre_hdr_t*)hdr)->prot_type = prot_type;
}

inline static
uint16_t* get_gre_reserved1(void *hdr){
	return &((cpc_gre_hdr_t*)hdr)->reserved1;
}

inline static
void set_gre_reserved1(void *hdr, uint16_t reserved1){
	((cpc_gre_hdr_t*)hdr)->reserved1 = reserved1; // Must be 0 according to RFC 2784
}

inline static
uint32_t* get_gre_key(void *hdr){
	if (((cpc_gre_base_hdr_t*)hdr)->word0 & GRE_KEY_FLAG_MASK) {
		return &((cpc_gre_hdr_t*)hdr)->key;
	}else{
		return NULL;
	}
}

inline static
void set_gre_key(void *hdr, uint32_t key){
	if (((cpc_gre_base_hdr_t*)hdr)->word0 & GRE_KEY_FLAG_MASK) {
		((cpc_gre_hdr_t*)hdr)->key = key;
	}else{
		return;
	}
}

inline static
uint32_t* get_gre_seqno(void *hdr){
	if (((cpc_gre_base_hdr_t*)hdr)->word0 & GRE_SEQNO_FLAG_MASK) {
		return &((cpc_gre_hdr_t*)hdr)->seqno;
	}else{
		return NULL;
	}
}

inline static
void set_gre_seqno(void *hdr, uint32_t seqno){
	if (((cpc_gre_base_hdr_t*)hdr)->word0 & GRE_SEQNO_FLAG_MASK) {
		((cpc_gre_hdr_t*)hdr)->seqno = seqno;
	}else{
		return;
	}
}

#endif /* CPC_GRE_H_ */
