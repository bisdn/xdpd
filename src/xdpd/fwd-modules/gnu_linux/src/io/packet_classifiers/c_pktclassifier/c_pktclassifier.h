/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _C_PKTCLASSIFIER_H_
#define _C_PKTCLASSIFIER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <rofl/datapath/pipeline/common/datapacket.h>

#include "./headers/cpc_arpv4.h"
#include "./headers/cpc_ethernet.h"
#include "./headers/cpc_gtpu.h"
#include "./headers/cpc_icmpv4.h"
#include "./headers/cpc_icmpv6.h"
#include "./headers/cpc_ipv4.h"
#include "./headers/cpc_ipv6.h"
#include "./headers/cpc_mpls.h"
#include "./headers/cpc_ppp.h"
#include "./headers/cpc_pppoe.h"
#include "./headers/cpc_tcp.h"
#include "./headers/cpc_udp.h"
#include "./headers/cpc_vlan.h"

/**
* @file cpc_pktclassifier.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Interface for the C classifiers
*/

//Header type
enum header_type{
	HEADER_TYPE_ETHER = 0,	
	HEADER_TYPE_VLAN = 1,	
	HEADER_TYPE_MPLS = 2,	
	HEADER_TYPE_ARPV4 = 3,	
	HEADER_TYPE_IPV4 = 4,	
	HEADER_TYPE_ICMPV4 = 5,
	HEADER_TYPE_IPV6 = 6,	
	HEADER_TYPE_ICMPV6 = 7,	
	HEADER_TYPE_UDP = 8,	
	HEADER_TYPE_TCP = 9,	
	HEADER_TYPE_SCTP = 10,	
	HEADER_TYPE_PPPOE = 11,	
	HEADER_TYPE_PPP = 12,	
	HEADER_TYPE_GTP = 13,

	//Must be the last one
	HEADER_TYPE_MAX
};


// Constants
//Maximum header occurrences per type
#define MAX_ETHER_FRAMES 2
#define MAX_VLAN_FRAMES 4
#define MAX_MPLS_FRAMES 16
#define MAX_ARPV4_FRAMES 1
#define MAX_IPV4_FRAMES 2
#define MAX_ICMPV4_FRAMES 2
#define MAX_IPV6_FRAMES 2
#define MAX_ICMPV6_FRAMES 2
#define MAX_UDP_FRAMES 2
#define MAX_TCP_FRAMES 2
#define MAX_SCTP_FRAMES 2
#define MAX_PPPOE_FRAMES 1
#define MAX_PPP_FRAMES 1
#define MAX_GTP_FRAMES 1

//Total maximum header occurrences
#define MAX_HEADERS MAX_ETHER_FRAMES + \
						MAX_VLAN_FRAMES + \
						MAX_MPLS_FRAMES + \
						MAX_ARPV4_FRAMES + \
						MAX_IPV4_FRAMES + \
						MAX_ICMPV4_FRAMES + \
						MAX_IPV6_FRAMES + \
						MAX_ICMPV6_FRAMES + \
						MAX_UDP_FRAMES + \
						MAX_TCP_FRAMES + \
						MAX_SCTP_FRAMES + \
						MAX_PPPOE_FRAMES + \
						MAX_PPP_FRAMES + \
						MAX_GTP_FRAMES


//Relative positions within the array;
//Very first frame always
#define FIRST_ETHER_FRAME_POS 0
#define FIRST_VLAN_FRAME_POS FIRST_ETHER_FRAME_POS+MAX_ETHER_FRAMES
#define FIRST_MPLS_FRAME_POS FIRST_VLAN_FRAME_POS+MAX_VLAN_FRAMES
#define FIRST_ARPV4_FRAME_POS FIRST_MPLS_FRAME_POS+MAX_MPLS_FRAMES
#define FIRST_IPV4_FRAME_POS FIRST_ARPV4_FRAME_POS+MAX_ARPV4_FRAMES
#define FIRST_ICMPV4_FRAME_POS FIRST_IPV4_FRAME_POS+MAX_IPV4_FRAMES
#define FIRST_IPV6_FRAME_POS FIRST_ICMPV4_FRAME_POS+MAX_ICMPV4_FRAMES
#define FIRST_ICMPV6_FRAME_POS FIRST_IPV6_FRAME_POS+MAX_IPV6_FRAMES
#define FIRST_UDP_FRAME_POS FIRST_ICMPV6_FRAME_POS+MAX_ICMPV6_FRAMES
#define FIRST_TCP_FRAME_POS FIRST_UDP_FRAME_POS+MAX_UDP_FRAMES
#define FIRST_SCTP_FRAME_POS FIRST_TCP_FRAME_POS+MAX_TCP_FRAMES
#define FIRST_PPPOE_FRAME_POS FIRST_SCTP_FRAME_POS+MAX_SCTP_FRAMES
#define FIRST_PPP_FRAME_POS FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES
#define FIRST_GTP_FRAME_POS FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES

//Just to be on the safe side of life
//assert( (FIRST_PPP_FRAME_POS + MAX_PPP_FRAMES) == MAX_HEADERS);

ROFL_BEGIN_DECLS

//Header container
typedef struct header_container{

	//Presence of header
	bool present;
	
	//Header pointer
	void* frame;
	size_t length;
	
	//NOTE not used:
	//enum header_type type;
	//Pseudo-linked list pointers (short-cuts)
	//struct header_container* prev;
	//struct header_container* next;
}header_container_t;

typedef struct classify_state{
	//Real container
	header_container_t headers[MAX_HEADERS];
	
	//Counters
	unsigned int num_of_headers[HEADER_TYPE_MAX];
	
	//Flag to know if it is classified
	bool is_classified;

	//Inner most (last) ethertype
	uint16_t eth_type;

}classify_state_t;


//function declarations
classify_state_t* init_classifier(void);
void destroy_classifier(classify_state_t* clas_state);
void classify_packet(classify_state_t* clas_state, uint8_t* pkt, size_t len);
void reset_classifier(classify_state_t* clas_state);

inline static 
void* get_ether_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;
	if(idx > (int)MAX_ETHER_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_ETHER_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_ETHER] - 1;
	else
		pos = FIRST_ETHER_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;	
	return NULL;
}

inline static
void* get_vlan_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;	

	if(idx > (int)MAX_VLAN_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_VLAN_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_VLAN] - 1;
	else
		pos = FIRST_VLAN_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_mpls_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;	

	if(idx > (int)MAX_MPLS_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_MPLS_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_MPLS] - 1;
	else
		pos = FIRST_MPLS_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_arpv4_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;	

	if(idx > (int)MAX_ARPV4_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_ARPV4_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_ARPV4] - 1;
	else
		pos = FIRST_ARPV4_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_ipv4_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;	

	if(idx > (int)MAX_IPV4_FRAMES)
		return NULL;

	if(idx < 0) //Inner most const
		pos = FIRST_IPV4_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_IPV4] - 1;
	else
		pos = FIRST_IPV4_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_icmpv4_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;	

	if(idx > (int)MAX_ICMPV4_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_ICMPV4_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_ICMPV4] - 1;
	else
		pos = FIRST_ICMPV4_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_ipv6_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;	

	if(idx > (int)MAX_IPV6_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_IPV6_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_IPV6] - 1;
	else
		pos = FIRST_IPV6_FRAME_POS + idx;

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_icmpv6_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;	

	if(idx > (int)MAX_ICMPV6_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_ICMPV6_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_ICMPV6] - 1;
	else
		pos = FIRST_ICMPV6_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_udp_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;	

	if(idx > (int)MAX_UDP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_UDP_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_UDP] - 1;
	else
		pos = FIRST_UDP_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_tcp_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;	

	if(idx > (int)MAX_TCP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_TCP_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_TCP] - 1;
	else
		pos = FIRST_TCP_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_pppoe_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;	

	if(idx > (int)MAX_PPPOE_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_PPPOE_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_PPPOE] - 1;
	else
		pos = FIRST_PPPOE_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_ppp_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;	

	if(idx > (int)MAX_PPP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_PPP_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_PPP] - 1;
	else
		pos = FIRST_PPP_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_gtpu_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;

	if(idx > (int)MAX_GTP_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_GTP_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_GTP] - 1;
	else
		pos = FIRST_GTP_FRAME_POS + idx;

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

//push & pop
void pop_vlan(datapacket_t* pkt, classify_state_t* clas_state);
void pop_mpls(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type);
void pop_pppoe(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type);
void pop_gtp(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type);

void* push_vlan(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type);
void* push_mpls(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type);
void* push_pppoe(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type);
void* push_gtp(datapacket_t* pkt, classify_state_t* clas_state, uint16_t ether_type);

//void pkt_push();
//void pkt_pop();

void dump_pkt_classifier(classify_state_t* clas_state);
size_t get_pkt_len(datapacket_t* pkt, classify_state_t* clas_state, void *from, void *to);

//shifts
inline static 
void shift_ether(classify_state_t* clas_state, int idx, ssize_t bytes){
	//NOTE if bytes id < 0 the header will be shifted left, if it is > 0, right
	unsigned int pos;
	if(idx > (int)MAX_ETHER_FRAMES)
		return;

	if(idx < 0) //Inner most
		pos = FIRST_ETHER_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_ETHER] - 1;
	else
		pos = FIRST_ETHER_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present){
		clas_state->headers[pos].frame = (uint8_t*)(clas_state->headers[pos].frame) + bytes;
		clas_state->headers[pos].length += bytes;
	}
}

inline static
void shift_vlan(classify_state_t* clas_state, int idx, ssize_t bytes){
	//NOTE if bytes id < 0 the header will be shifted left, if it is > 0, right
	unsigned int pos;	

	if(idx > (int)MAX_VLAN_FRAMES)
		return;

	if(idx < 0) //Inner most
		pos = FIRST_VLAN_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_VLAN] - 1;
	else
		pos = FIRST_VLAN_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present){
		clas_state->headers[pos].frame = (uint8_t*)(clas_state->headers[pos].frame) + bytes;
		clas_state->headers[pos].length += bytes;
	}
}


ROFL_END_DECLS

#endif //_C_PKTCLASSIFIER_H_
