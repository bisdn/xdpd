/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _C_PKTCLASSIFIER_H_
#define _C_PKTCLASSIFIER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "../pktclassifier.h"

#include "./headers/cpc_arpv4.h"
#include "./headers/cpc_ethernet.h"
#include "./headers/cpc_gtpu.h"
#include "./headers/cpc_icmpv4.h"
#include "./headers/cpc_icmpv6_opt.h"
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
	HEADER_TYPE_ICMPV6_OPT = 8,	
	HEADER_TYPE_UDP = 9,	
	HEADER_TYPE_TCP = 10,	
	HEADER_TYPE_SCTP = 11,	
	HEADER_TYPE_PPPOE = 12,	
	HEADER_TYPE_PPP = 13,	
	HEADER_TYPE_GTP = 14,

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
#define MAX_ICMPV6_FRAMES 1
#define MAX_ICMPV6_OPT_FRAMES 3
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
						MAX_ICMPV6_OPT_FRAMES + \
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
#define FIRST_ICMPV6_OPT_FRAME_POS FIRST_ICMPV6_FRAME_POS+MAX_ICMPV6_FRAMES
#define FIRST_UDP_FRAME_POS FIRST_ICMPV6_OPT_FRAME_POS+MAX_ICMPV6_OPT_FRAMES
#define FIRST_TCP_FRAME_POS FIRST_UDP_FRAME_POS+MAX_UDP_FRAMES
#define FIRST_SCTP_FRAME_POS FIRST_TCP_FRAME_POS+MAX_TCP_FRAMES
#define FIRST_PPPOE_FRAME_POS FIRST_SCTP_FRAME_POS+MAX_SCTP_FRAMES
#define FIRST_PPP_FRAME_POS FIRST_PPPOE_FRAME_POS+MAX_PPPOE_FRAMES
#define FIRST_GTP_FRAME_POS FIRST_PPP_FRAME_POS+MAX_PPP_FRAMES

#define OFFSET_ICMPV6_OPT_LLADDR_SOURCE 0
#define OFFSET_ICMPV6_OPT_LLADDR_TARGET 1
#define OFFSET_ICMPV6_OPT_PREFIX_INFO 2

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

	//Pre-parsed packet matches
	packet_matches_t* matches; 
}classify_state_t;


//inline function implementations
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
void* get_icmpv6_opt_hdr(classify_state_t* clas_state, int idx){
	unsigned int pos;	

	if(idx > (int)MAX_ICMPV6_OPT_FRAMES)
		return NULL;

	if(idx < 0) //Inner most
		pos = FIRST_ICMPV6_OPT_FRAME_POS + clas_state->num_of_headers[HEADER_TYPE_ICMPV6_OPT] - 1;
	else
		pos = FIRST_ICMPV6_OPT_FRAME_POS + idx;	

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_icmpv6_opt_lladr_source_hdr(classify_state_t* clas_state, int idx){
	//only one option of this kind is allowed
	unsigned int pos;
	pos = FIRST_ICMPV6_OPT_FRAME_POS + OFFSET_ICMPV6_OPT_LLADDR_SOURCE;

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_icmpv6_opt_lladr_target_hdr(classify_state_t* clas_state, int idx){
	//only one option of this kind is allowed
	unsigned int pos;
	pos = FIRST_ICMPV6_OPT_FRAME_POS + OFFSET_ICMPV6_OPT_LLADDR_TARGET;

	//Return the index
	if(clas_state->headers[pos].present)
		return clas_state->headers[pos].frame;
	return NULL;
}

inline static
void* get_icmpv6_opt_prefix_info_hdr(classify_state_t* clas_state, int idx){
	//only one option of this kind is allowed
	unsigned int pos;
	pos = FIRST_ICMPV6_OPT_FRAME_POS + OFFSET_ICMPV6_OPT_PREFIX_INFO;

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

//
// Parsing code
//

static inline
void parse_tcp(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	if (unlikely(datalen < sizeof(cpc_tcp_hdr_t))) { return; }

	cpc_tcp_hdr_t* tcp = (cpc_tcp_hdr_t*)data;
	
	//Set frame
	unsigned int num_of_tcp = clas_state->num_of_headers[HEADER_TYPE_TCP];
	clas_state->headers[FIRST_TCP_FRAME_POS + num_of_tcp].frame = tcp;
	clas_state->headers[FIRST_TCP_FRAME_POS + num_of_tcp].present = true;
	clas_state->headers[FIRST_TCP_FRAME_POS + num_of_tcp].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_TCP] = num_of_tcp+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_tcp_hdr_t);
	datalen -= sizeof(cpc_tcp_hdr_t);

	if (datalen > 0){
		//TODO: something 
	}
	
	//Initialize tcp packet matches
	clas_state->matches->__tcp_src = get_tcp_sport(tcp);
	clas_state->matches->__tcp_dst = get_tcp_dport(tcp);
}

static inline
void parse_gtp(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_gtphu_t))) { return; }

	cpc_gtphu_t *gtp = (cpc_gtphu_t*)data; 
		
	//Set frame
	unsigned int num_of_gtp = clas_state->num_of_headers[HEADER_TYPE_GTP];
	clas_state->headers[FIRST_GTP_FRAME_POS + num_of_gtp].frame = gtp;
	clas_state->headers[FIRST_GTP_FRAME_POS + num_of_gtp].present = true;
	clas_state->headers[FIRST_GTP_FRAME_POS + num_of_gtp].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_GTP] = num_of_gtp+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_gtphu_t);
	datalen -= sizeof(cpc_gtphu_t);

	if (datalen > 0){
		//TODO: something
	}

	//Initialize udp packet matches
	clas_state->matches->__gtp_msg_type = get_gtpu_msg_type(gtp);
	clas_state->matches->__gtp_teid = get_gtpu_teid(gtp);
}

static inline
void parse_udp(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_udp_hdr_t))) { return; }

	cpc_udp_hdr_t *udp = (cpc_udp_hdr_t*)data; 
	
	//Set frame
	unsigned int num_of_udp = clas_state->num_of_headers[HEADER_TYPE_UDP];
	clas_state->headers[FIRST_UDP_FRAME_POS + num_of_udp].frame = udp;
	clas_state->headers[FIRST_UDP_FRAME_POS + num_of_udp].present = true;
	clas_state->headers[FIRST_UDP_FRAME_POS + num_of_udp].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_UDP] = num_of_udp+1;

	//Set reference
	
	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_udp_hdr_t);
	datalen -= sizeof(cpc_udp_hdr_t);

	if (datalen > 0){
		switch (get_udp_dport(udp)) {
		case UDP_DST_PORT_GTPU: {
			parse_gtp(clas_state, data, datalen);
		} break;
		default: {
			//TODO: something
		} break;
		}
	}

	//Initialize udp packet matches
	clas_state->matches->__udp_src = get_udp_sport(udp);
	clas_state->matches->__udp_dst = get_udp_dport(udp);
}

static inline
void parse_arpv4(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	
	if (unlikely(datalen < sizeof(cpc_arpv4_hdr_t))) { return; }
	
	cpc_arpv4_hdr_t* arpv4 = (cpc_arpv4_hdr_t*)data;

	//Set frame
	unsigned int num_of_arpv4 = clas_state->num_of_headers[HEADER_TYPE_ARPV4];
	clas_state->headers[FIRST_ARPV4_FRAME_POS + num_of_arpv4].frame = arpv4;
	clas_state->headers[FIRST_ARPV4_FRAME_POS + num_of_arpv4].present = true;
	clas_state->headers[FIRST_ARPV4_FRAME_POS + num_of_arpv4].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_ARPV4] = num_of_arpv4+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_arpv4_hdr_t);
	datalen -= sizeof(cpc_arpv4_hdr_t);

	if (datalen > 0){
		//TODO: something?
	}

	//Initialize arpv4 packet matches
	clas_state->matches->__arp_opcode = get_arpv4_opcode(arpv4);
	clas_state->matches->__arp_sha =  get_arpv4_dl_src(arpv4);
	clas_state->matches->__arp_spa =  get_arpv4_ip_src(arpv4);
	clas_state->matches->__arp_tha =  get_arpv4_dl_dst(arpv4);
	clas_state->matches->__arp_tpa =  get_arpv4_ip_dst(arpv4);
}

static inline
void parse_icmpv4(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_icmpv4_hdr_t))) { return; }

	//Set reference
	cpc_icmpv4_hdr_t *icmpv4 = (cpc_icmpv4_hdr_t*)data; 

	//Set frame
	unsigned int num_of_icmpv4 = clas_state->num_of_headers[HEADER_TYPE_ICMPV4];
	clas_state->headers[FIRST_ICMPV4_FRAME_POS + num_of_icmpv4].frame = icmpv4; 
	clas_state->headers[FIRST_ICMPV4_FRAME_POS + num_of_icmpv4].present = true;
	clas_state->headers[FIRST_ICMPV4_FRAME_POS + num_of_icmpv4].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_ICMPV4] = num_of_icmpv4+1;

	//Set reference

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_icmpv4_hdr_t);
	datalen -= sizeof(cpc_icmpv4_hdr_t);


	if (datalen > 0){
		//TODO: something?	
	}

	//Initialize ipv4 packet matches
	clas_state->matches->__icmpv4_code = get_icmpv4_code(icmpv4);
	clas_state->matches->__icmpv4_type = get_icmpv4_type(icmpv4);
}

static inline
void parse_ipv4(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	if (unlikely(datalen < sizeof(cpc_ipv4_hdr_t))) { return; }
	
	//Set reference
	cpc_ipv4_hdr_t *ipv4 = (cpc_ipv4_hdr_t*)data; 

	//Set frame
	unsigned int num_of_ipv4 = clas_state->num_of_headers[HEADER_TYPE_IPV4];
	clas_state->headers[FIRST_IPV4_FRAME_POS + num_of_ipv4].frame = ipv4;
	clas_state->headers[FIRST_IPV4_FRAME_POS + num_of_ipv4].present = true;
	clas_state->headers[FIRST_IPV4_FRAME_POS + num_of_ipv4].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_IPV4] = num_of_ipv4+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_ipv4_hdr_t);
	datalen -= sizeof(cpc_ipv4_hdr_t);

	if (has_ipv4_MF_bit_set(ipv4)){
		// TODO: fragment handling

		return;
	}

	// FIXME: IP header with options


	switch (get_ipv4_proto(ipv4)) {
		case IPV4_IP_PROTO:
			{
				parse_ipv4(clas_state, data, datalen);
			}
			break;
		case ICMPV4_IP_PROTO:
			{
				parse_icmpv4(clas_state, data, datalen);
			}
			break;
		case UDP_IP_PROTO:
			{
				parse_udp(clas_state, data, datalen);
			}
			break;
		case TCP_IP_PROTO:
			{
				parse_tcp(clas_state, data, datalen);
			}
			break;
#if 0
		case SCTP_IP_PROTO:
			{
				parse_sctp(clas_state, data, datalen);
			}
			break;
#endif
		default:
			{
			
			}
			break;
	}

	//Initialize ipv4 packet matches
	clas_state->matches->__ip_proto = get_ipv4_proto(ipv4);
	clas_state->matches->__ip_dscp = get_ipv4_dscp(ipv4);
	clas_state->matches->__ip_ecn = get_ipv4_ecn(ipv4);
	clas_state->matches->__ipv4_src = get_ipv4_src(ipv4);
	clas_state->matches->__ipv4_dst = get_ipv4_dst(ipv4);
}

static inline
void parse_icmpv6_opts(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	if (unlikely(datalen < sizeof(cpc_icmpv6_option_hdr_t))) { return; }
	/*So far we only parse optionsICMPV6_OPT_LLADDR_TARGET, ICMPV6_OPT_LLADDR_SOURCE and ICMPV6_OPT_PREFIX_INFO*/
	cpc_icmpv6_option_hdr_t* icmpv6_opt = (cpc_icmpv6_option_hdr_t*)data;
	
	//Set frame
	unsigned int num_of_icmpv6_opt = clas_state->num_of_headers[HEADER_TYPE_ICMPV6_OPT];
	
	//we asume here that there is only one option for each type
	switch(icmpv6_opt->type){
		case ICMPV6_OPT_LLADDR_SOURCE:
			clas_state->headers[FIRST_ICMPV6_OPT_FRAME_POS + OFFSET_ICMPV6_OPT_LLADDR_SOURCE].frame = icmpv6_opt;
			clas_state->headers[FIRST_ICMPV6_OPT_FRAME_POS + OFFSET_ICMPV6_OPT_LLADDR_SOURCE].present = true;
			clas_state->headers[FIRST_ICMPV6_OPT_FRAME_POS + OFFSET_ICMPV6_OPT_LLADDR_SOURCE].length = datalen;
			
			data += sizeof(struct cpc_icmpv6_lla_option);		//update data pointer
			datalen -= sizeof(struct cpc_icmpv6_lla_option);	//decrement data length
			
			clas_state->matches->__ipv6_nd_sll = get_icmpv6_ll_saddr( (struct cpc_icmpv6_lla_option *)icmpv6_opt ); //init matches

			break;
		case ICMPV6_OPT_LLADDR_TARGET:
			clas_state->headers[FIRST_ICMPV6_OPT_FRAME_POS + OFFSET_ICMPV6_OPT_LLADDR_TARGET].frame = icmpv6_opt;
			clas_state->headers[FIRST_ICMPV6_OPT_FRAME_POS + OFFSET_ICMPV6_OPT_LLADDR_TARGET].present = true;
			clas_state->headers[FIRST_ICMPV6_OPT_FRAME_POS + OFFSET_ICMPV6_OPT_LLADDR_TARGET].length = datalen;
			
			data += sizeof(struct cpc_icmpv6_lla_option);		 //update pointers
			datalen -= sizeof(struct cpc_icmpv6_lla_option);	//decrement data length
			
			clas_state->matches->__ipv6_nd_tll = get_icmpv6_ll_taddr( (struct cpc_icmpv6_lla_option *)icmpv6_opt ); //init matches

			break;
		case ICMPV6_OPT_PREFIX_INFO:
			clas_state->headers[FIRST_ICMPV6_OPT_FRAME_POS + OFFSET_ICMPV6_OPT_PREFIX_INFO].frame = icmpv6_opt;
			clas_state->headers[FIRST_ICMPV6_OPT_FRAME_POS + OFFSET_ICMPV6_OPT_PREFIX_INFO].present = true;
			clas_state->headers[FIRST_ICMPV6_OPT_FRAME_POS + OFFSET_ICMPV6_OPT_PREFIX_INFO].length = datalen;

			data += sizeof(struct cpc_icmpv6_prefix_info);		 //update pointers
			datalen -= sizeof(struct cpc_icmpv6_prefix_info);	//decrement data length

			get_icmpv6_pfx_on_link_flag( (struct cpc_icmpv6_prefix_info *)icmpv6_opt ); //init matches
			get_icmpv6_pfx_aac_flag( (struct cpc_icmpv6_prefix_info *)icmpv6_opt );

			break;
	}
	clas_state->num_of_headers[HEADER_TYPE_ICMPV6_OPT] = num_of_icmpv6_opt+1;

	if (datalen > 0){
		parse_icmpv6_opts(clas_state, data, datalen);
	}
}

static inline
void parse_icmpv6(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_icmpv6_hdr_t))) { return; }

	cpc_icmpv6_hdr_t* icmpv6 = (cpc_icmpv6_hdr_t*)data;
	
	//Set frame
	unsigned int num_of_icmpv6 = clas_state->num_of_headers[HEADER_TYPE_ICMPV6];
	clas_state->headers[FIRST_ICMPV6_FRAME_POS + num_of_icmpv6].frame = icmpv6;
	clas_state->headers[FIRST_ICMPV6_FRAME_POS + num_of_icmpv6].present = true;
	clas_state->headers[FIRST_ICMPV6_FRAME_POS + num_of_icmpv6].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_ICMPV6] = num_of_icmpv6+1;

	//Initialize icmpv6 packet matches
	clas_state->matches->__icmpv6_code = get_icmpv6_code(icmpv6);
	clas_state->matches->__icmpv6_type = get_icmpv6_type(icmpv6);
	clas_state->matches->__ipv6_nd_target = get_icmpv6_neighbor_taddr(icmpv6);
	
	//Increment pointers and decrement remaining payload size (depending on type)
	switch(clas_state->matches->__icmpv6_type){
		case ICMPV6_TYPE_ROUTER_SOLICATION:
			data += sizeof(struct cpc_icmpv6_router_solicitation_hdr);
			datalen -= sizeof(struct cpc_icmpv6_router_solicitation_hdr);
			break;
		case ICMPV6_TYPE_ROUTER_ADVERTISEMENT:
			data += sizeof(struct cpc_icmpv6_router_advertisement_hdr);
			datalen -= sizeof(struct cpc_icmpv6_router_advertisement_hdr);
			break;
		case ICMPV6_TYPE_NEIGHBOR_SOLICITATION:
			data += sizeof(struct cpc_icmpv6_neighbor_solicitation_hdr);
			datalen -= sizeof(struct cpc_icmpv6_neighbor_solicitation_hdr);
			break;
		case ICMPV6_TYPE_NEIGHBOR_ADVERTISEMENT:
			data += sizeof(struct cpc_icmpv6_neighbor_advertisement_hdr);
			datalen -= sizeof(struct cpc_icmpv6_neighbor_advertisement_hdr);
			break;
		case ICMPV6_TYPE_REDIRECT_MESSAGE:
			data += sizeof(struct cpc_icmpv6_redirect_hdr);
			datalen -= sizeof(struct cpc_icmpv6_redirect_hdr);
			break;
		default:
			//Here we have a not supported type
			// for example errors, which we are not parsing.
			data += sizeof(cpc_icmpv6_hdr_t);
			datalen -= sizeof(cpc_icmpv6_hdr_t);
			return;
			break;
	}

	if (datalen > 0){
		parse_icmpv6_opts(clas_state,data,datalen);
	}
}

static inline
void parse_ipv6(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	
	if(unlikely(datalen < sizeof(cpc_ipv6_hdr_t))) { return; }
	
	//Set reference
	cpc_ipv6_hdr_t *ipv6 = (cpc_ipv6_hdr_t*)data; 

	//Set frame
	unsigned int num_of_ipv6 = clas_state->num_of_headers[HEADER_TYPE_IPV6];
	clas_state->headers[FIRST_IPV6_FRAME_POS + num_of_ipv6].frame = ipv6;
	clas_state->headers[FIRST_IPV6_FRAME_POS + num_of_ipv6].present = true;
	clas_state->headers[FIRST_IPV6_FRAME_POS + num_of_ipv6].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_IPV6] = num_of_ipv6+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_ipv6_hdr_t);
	datalen -= sizeof(cpc_ipv6_hdr_t);

	// FIXME: IP header with options

	switch (get_ipv6_next_header(ipv6)) {
		case IPV4_IP_PROTO:
			{
				parse_ipv4(clas_state, data, datalen);
			}
			break;
		case ICMPV4_IP_PROTO:
			{
				parse_icmpv4(clas_state, data, datalen);
			}
			break;
		case IPV6_IP_PROTO:
			{
				parse_ipv6(clas_state, data, datalen);
			}
			break;
		case ICMPV6_IP_PROTO:
			{
				parse_icmpv6(clas_state, data, datalen);
			}
			break;
		case UDP_IP_PROTO:
			{
				parse_udp(clas_state, data, datalen);
			}
			break;
		case TCP_IP_PROTO:
			{
				parse_tcp(clas_state, data, datalen);
			}
			break;
#if 0
		case SCTP_IP_PROTO:
			{
				parse_sctp(data, datalen);
			}
			break;
#endif
		default:
			{
			
			}
			break;
	}

	//Initialize ipv6 packet matches
	clas_state->matches->__ip_proto = get_ipv6_next_header(ipv6);
	clas_state->matches->__ip_dscp = get_ipv6_dscp(ipv6);
	clas_state->matches->__ip_ecn = get_ipv6_ecn(ipv6);
	clas_state->matches->__ipv6_src = get_ipv6_src(ipv6);
	clas_state->matches->__ipv6_dst = get_ipv6_dst(ipv6);
	clas_state->matches->__ipv6_flabel = get_ipv6_flow_label(ipv6);
}


static inline
void parse_mpls(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	
	if (unlikely(datalen < sizeof(cpc_mpls_hdr_t))) { return; }

	cpc_mpls_hdr_t* mpls = (cpc_mpls_hdr_t*)data;
	
	//Set frame
	unsigned int num_of_mpls = clas_state->num_of_headers[HEADER_TYPE_MPLS];
	clas_state->headers[FIRST_MPLS_FRAME_POS + num_of_mpls].frame = mpls;
	clas_state->headers[FIRST_MPLS_FRAME_POS + num_of_mpls].present = true;
	clas_state->headers[FIRST_MPLS_FRAME_POS + num_of_mpls].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_MPLS] = num_of_mpls+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_mpls_hdr_t);
	datalen -= sizeof(cpc_mpls_hdr_t);

	if (! get_mpls_bos(mpls)){

		parse_mpls(clas_state,data, datalen);

	}else{
		
		//TODO: We could be trying to guess if payload is IPv4/v6 and continue parsing...
	}

	//Initialize mpls packet matches
	clas_state->matches->__mpls_bos = get_mpls_bos(mpls);
	clas_state->matches->__mpls_label = get_mpls_label(mpls); 
	clas_state->matches->__mpls_tc = get_mpls_tc(mpls); 
}

static inline
void parse_ppp(classify_state_t* clas_state, uint8_t *data, size_t datalen){
	
	if (unlikely(datalen < sizeof(cpc_ppp_hdr_t))) { return; }

	cpc_ppp_hdr_t* ppp = (cpc_ppp_hdr_t*)data;
	
	//Set frame
	unsigned int num_of_ppp = clas_state->num_of_headers[HEADER_TYPE_PPP];
	clas_state->headers[FIRST_PPP_FRAME_POS + num_of_ppp].frame = ppp; 
	clas_state->headers[FIRST_PPP_FRAME_POS + num_of_ppp].present = true;
	clas_state->headers[FIRST_PPP_FRAME_POS + num_of_ppp].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_PPP] = num_of_ppp+1;

	//Increment pointers and decrement remaining payload size
	switch (get_ppp_prot(ppp)) {
		case PPP_PROT_IPV4:
			{
				//Increment pointers and decrement remaining payload size
				data += sizeof(cpc_ppp_hdr_t);
				datalen -= sizeof(cpc_ppp_hdr_t);

				parse_ipv4(clas_state, data, datalen);
			}
			break;
		default:
			{
				//TODO? ppp->unpack(data, datalen);
			}
			break;
	}

	//Initialize ppp packet matches
	clas_state->matches->__ppp_proto = get_ppp_prot(ppp);
}

static inline
void parse_pppoe(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_pppoe_hdr_t))) { return; }

	cpc_pppoe_hdr_t* pppoe = (cpc_pppoe_hdr_t*)data;

	//Set frame
	unsigned int num_of_pppoe = clas_state->num_of_headers[HEADER_TYPE_PPPOE];
	clas_state->headers[FIRST_PPPOE_FRAME_POS + num_of_pppoe].frame = pppoe;
	clas_state->headers[FIRST_PPPOE_FRAME_POS + num_of_pppoe].present = true;
	clas_state->headers[FIRST_PPPOE_FRAME_POS + num_of_pppoe].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_PPPOE] = num_of_pppoe+1;
	
	switch (clas_state->eth_type) {
		case ETH_TYPE_PPPOE_DISCOVERY:
			{
				datalen -= sizeof(cpc_pppoe_hdr_t);
#if 0
//TODO?
				uint16_t pppoe_len = get_pppoe_length(pppoe) > datalen ? datalen : get_pppoe_length(pppoe);

				/*
				 * parse any pppoe service tags
				 */
				pppoe->unpack(data, sizeof(cpc_pppoe_hdr_t) + pppoe_len);


				/*
				 * any remaining bytes after the pppoe tags => padding?
				 */
				if (datalen > pppoe->tags.length())
				{
					//TODO?: Continue parsing??	
				}
#endif
			}
			break;
		case ETH_TYPE_PPPOE_SESSION:
			{
				//Increment pointers and decrement remaining payload size
				data += sizeof(cpc_pppoe_hdr_t);
				datalen -= sizeof(cpc_pppoe_hdr_t);

				parse_ppp(clas_state,data, datalen);
			}
			break;
		default:
			{
				// log error?
			}
			break;
	}

	//Initialize pppoe packet matches
	clas_state->matches->__pppoe_code = get_pppoe_code(pppoe);
	clas_state->matches->__pppoe_type = get_pppoe_type(pppoe);
	clas_state->matches->__pppoe_sid = get_pppoe_sessid(pppoe);
}

static inline
void parse_vlan(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_vlan_hdr_t))) { return; }

	//Data pointer	
	cpc_vlan_hdr_t* vlan = (cpc_vlan_hdr_t *)data;

	//Set frame
	unsigned int num_of_vlan = clas_state->num_of_headers[HEADER_TYPE_VLAN];
	clas_state->headers[FIRST_VLAN_FRAME_POS + num_of_vlan].frame = vlan;
	clas_state->headers[FIRST_VLAN_FRAME_POS + num_of_vlan].present = true;
	clas_state->headers[FIRST_VLAN_FRAME_POS + num_of_vlan].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_VLAN] = num_of_vlan+1;

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_vlan_hdr_t);
	datalen -= sizeof(cpc_vlan_hdr_t);

	clas_state->eth_type = get_vlan_type(vlan);

	switch (clas_state->eth_type) {
		case VLAN_CTAG_ETHER_TYPE:
		case VLAN_STAG_ETHER_TYPE:
		case VLAN_ITAG_ETHER_TYPE:
			{
				parse_vlan(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_MPLS_UNICAST:
		case ETH_TYPE_MPLS_MULTICAST:
			{
				parse_mpls(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_PPPOE_DISCOVERY:
		case ETH_TYPE_PPPOE_SESSION:
			{
				parse_pppoe(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_ARP:
			{
				parse_arpv4(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_IPV4:
			{
				parse_ipv4(clas_state, data, datalen);
			}
			break;
		default:
			{

			}
			break;
	}

	//Initialize vlan packet matches
	clas_state->matches->__has_vlan = true;
	clas_state->matches->__eth_type = get_vlan_type(vlan);
	clas_state->matches->__vlan_vid = get_vlan_id(vlan);
	clas_state->matches->__vlan_pcp = get_vlan_pcp(vlan);
}

static inline
void parse_ethernet(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	if (unlikely(datalen < sizeof(cpc_eth_hdr_t))){return;}

	//Data pointer	
	cpc_eth_hdr_t* ether = (cpc_eth_hdr_t *)data;

	//Set frame
	unsigned int num_of_ether = clas_state->num_of_headers[HEADER_TYPE_ETHER];
	clas_state->headers[FIRST_ETHER_FRAME_POS + num_of_ether].frame = ether;
	clas_state->headers[FIRST_ETHER_FRAME_POS + num_of_ether].present = true;
	clas_state->headers[FIRST_ETHER_FRAME_POS + num_of_ether].length = datalen;
	clas_state->num_of_headers[HEADER_TYPE_ETHER] = num_of_ether+1;

	//Increment pointers and decrement remaining payload size
	if( is_llc_frame(ether) ){
		data += sizeof(cpc_eth_llc_hdr_t);
		datalen -= sizeof(cpc_eth_llc_hdr_t);
	}else{
		data += sizeof(cpc_eth_hdr_t);
		datalen -= sizeof(cpc_eth_hdr_t);
	}

	clas_state->eth_type = get_ether_type(ether);

	//Initialize eth packet matches
	clas_state->matches->__eth_type = get_ether_type(ether); //This MUST be here
	clas_state->matches->__eth_src = get_ether_dl_src(ether);
	clas_state->matches->__eth_dst = get_ether_dl_dst(ether);

	switch (clas_state->eth_type) {
		case VLAN_CTAG_ETHER_TYPE:
		case VLAN_STAG_ETHER_TYPE:
		case VLAN_ITAG_ETHER_TYPE:
			{
				parse_vlan(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_MPLS_UNICAST:
		case ETH_TYPE_MPLS_MULTICAST:
			{
				parse_mpls(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_PPPOE_DISCOVERY:
		case ETH_TYPE_PPPOE_SESSION:
			{
				parse_pppoe(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_ARP:
			{
				parse_arpv4(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_IPV4:
			{
				parse_ipv4(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_IPV6:
			{
				parse_ipv6(clas_state, data,datalen);
			}
			break;
		default:
			{
				
			}
			break;
	}


}

static inline
void reset_classifier(classify_state_t* clas_state){

	packet_matches_t* matches = clas_state->matches;
	memset(clas_state,0,sizeof(classify_state_t));
	clas_state->matches = matches;

	if(likely(matches != NULL))
		memset(clas_state->matches,0,sizeof(packet_matches_t));
}

static inline
void classify_packet(classify_state_t* clas_state, uint8_t* data, size_t len, uint32_t port_in, uint32_t phy_port_in){
	if(clas_state->is_classified)
		reset_classifier(clas_state);
	parse_ethernet(clas_state, data, len);
	clas_state->is_classified = true;
	
	//Fill in the matches
	clas_state->matches->__pkt_size_bytes = len;
	clas_state->matches->__port_in = port_in;
	clas_state->matches->__phy_port_in = phy_port_in;
}

ROFL_END_DECLS

#endif //_C_PKTCLASSIFIER_H_
