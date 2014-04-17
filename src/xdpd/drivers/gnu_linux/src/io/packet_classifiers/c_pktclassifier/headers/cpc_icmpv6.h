/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_ICMPV6_H_
#define _CPC_ICMPV6_H_

#include <rofl/datapath/pipeline/common/large_types.h>
#include "cpc_ethernet.h"
#include "../../../../util/likely.h"
#include "../../../../util/compiler_assert.h"

/**
* @file cpc_icmpv6.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for ICMPv6
*/

enum icmpv6_ip_proto_t {
	ICMPV6_IP_PROTO = 58,
};

/**
	* ICMPv6 message types
	*/

/* ICMPv6 generic header */
typedef struct cpc_icmpv6_hdr {
	uint8_t 						type;
	uint8_t 						code;
	uint16_t 						checksum;
	uint8_t 						data[0];
} __attribute__((packed)) cpc_icmpv6_hdr_t;


/**
	* ICMPv6 error message types
	*/

/* ICMPv6 message format for Destination Unreachable */
struct cpc_icmpv6_dest_unreach_hdr {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint32_t						unused;					// a 32bit value
	uint8_t							data[0];				// the IP packet
} __attribute__((packed));

/* ICMPv6 message format for Packet Too Big */
struct cpc_icmpv6_pkt_too_big_hdr {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint32_t						unused;					// a 32bit value
	uint8_t							data[0];				// the IP packet
} __attribute__((packed));

/* ICMPv6 message format for Time Exceeded */
struct cpc_icmpv6_time_exceeded_hdr {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint32_t						unused;					// a 32bit value
	uint8_t							data[0];				// the IP packet
} __attribute__((packed));

/* ICMPv6 message format for Parameter Problem */
struct cpc_icmpv6_param_problem_hdr {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint32_t						pointer;				// a 32bit value
	uint8_t							data[0];				// the IP packet
} __attribute__((packed));

/* ICMPv6 echo request message format */
struct cpc_icmpv6_echo_request_hdr {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint16_t						id;
	uint16_t 						seqno;
	uint8_t							data[0];				// arbitrary data
} __attribute__((packed));

/* ICMPv6 echo reply message format */
struct cpc_icmpv6_echo_reply_hdr {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint16_t						id;
	uint16_t 						seqno;
	uint8_t							data[0];				// arbitrary data
} __attribute__((packed));


/**
	* ICMPv6 NDP message types
	*/

/* ICMPv6 router solicitation */
struct cpc_icmpv6_router_solicitation_hdr {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint32_t 						reserved;				// reserved for later use, for now: mbz
	union cpc_icmpv6optu			options[0];
} __attribute__((packed));

/* ICMPv6 router advertisement */
struct cpc_icmpv6_router_advertisement_hdr {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=134, code=0
	uint8_t 						cur_hop_limit;
	uint8_t							flags;
	uint16_t 						rtr_lifetime;
	uint32_t						reachable_timer;
	uint32_t 						retrans_timer;
	union cpc_icmpv6optu			options[0];
} __attribute__((packed));

/* ICMPv6 neighbor solicitation */
struct cpc_icmpv6_neighbor_solicitation_hdr {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=135, code=0
	uint32_t 						reserved;				// reserved for later use, for now: mbz
	uint8_t							taddr[IPV6_ADDR_LEN]; 	// =target address
	union cpc_icmpv6optu			options[0];
} __attribute__((packed));

/* ICMPv6 neighbor advertisement */
struct cpc_icmpv6_neighbor_advertisement_hdr {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=136, code=0
	uint32_t 						flags;
	uint8_t							taddr[IPV6_ADDR_LEN]; 	// =target address
	union cpc_icmpv6optu			options[0];
} __attribute__((packed));

/* ICMPv6 redirect message */
struct cpc_icmpv6_redirect_hdr {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=137, code=0
	uint32_t 						reserved;				// reserved for later use, for now: mbz
	uint8_t							taddr[IPV6_ADDR_LEN]; 	// =target address
	uint8_t							daddr[IPV6_ADDR_LEN];	// =destination address
	union cpc_icmpv6optu			options[0];
} __attribute__((packed));

typedef union cpc_icmpv6u{
	cpc_icmpv6_hdr_t 								icmpv6u_hdr;							// ICMPv6 message header
	struct cpc_icmpv6_dest_unreach_hdr				icmpv6u_dst_unreach_hdr;				// ICMPv6 destination unreachable
	struct cpc_icmpv6_pkt_too_big_hdr				icmpv6u_pkt_too_big_hdr;				// ICMPv6 packet too big
	struct cpc_icmpv6_time_exceeded_hdr				icmpv6u_time_exceeded_hdr;				// ICMPv6 time exceeded
	struct cpc_icmpv6_param_problem_hdr				icmpv6u_param_problem_hdr;				// ICMPv6 parameter problem
	struct cpc_icmpv6_echo_request_hdr				icmpv6u_echo_request_hdr;				// ICMPv6 echo request
	struct cpc_icmpv6_echo_reply_hdr				icmpv6u_echo_reply_hdr;					// ICMPv6 echo reply
	struct cpc_icmpv6_router_solicitation_hdr		icmpv6u_rtr_solicitation_hdr;			// ICMPv6 rtr solicitation
	struct cpc_icmpv6_router_advertisement_hdr		icmpv6u_rtr_advertisement_hdr;			// ICMPv6 rtr advertisement
	struct cpc_icmpv6_neighbor_solicitation_hdr	icmpv6u_neighbor_solication_hdr;		// ICMPv6 NDP solication header
	struct cpc_icmpv6_neighbor_advertisement_hdr	icmpv6u_neighbor_advertisement_hdr;		// ICMPv6 NDP advertisement header
	struct cpc_icmpv6_redirect_hdr					icmpv6u_redirect_hdr;					// ICMPV6 redirect header
}cpc_icmpv6u_t;


inline static
void icmpv6_calc_checksum(void *hdr, uint16_t length){
	//TODO Implement the checksum 
};


//NOTE initialize, parse ..?¿

inline static
uint8_t get_icmpv6_code(void *hdr){
	return ((cpc_icmpv6u_t*)hdr)->icmpv6u_hdr.code;
};

inline static
void set_icmpv6_code(void *hdr, uint8_t code){
	((cpc_icmpv6u_t*)hdr)->icmpv6u_hdr.code = code;
};

inline static
uint8_t get_icmpv6_type(void *hdr){
	return ((cpc_icmpv6u_t*)hdr)->icmpv6u_hdr.type;
};

inline static
void set_icmpv6_type(void *hdr, uint8_t type){
	((cpc_icmpv6u_t*)hdr)->icmpv6u_hdr.type = type;
};

inline static
uint128__t get_icmpv6_neighbor_taddr(void *hdr){
	uint128__t addr = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
	void* tmp=NULL;
	switch (get_icmpv6_type(hdr)) {
		case ICMPV6_TYPE_NEIGHBOR_SOLICITATION:
			tmp = (void*)((cpc_icmpv6u_t*)hdr)->icmpv6u_neighbor_solication_hdr.taddr;
			addr= *(uint128__t*)tmp;
			break;
		case ICMPV6_TYPE_NEIGHBOR_ADVERTISEMENT:
			tmp = (void*)((cpc_icmpv6u_t*)hdr)->icmpv6u_neighbor_advertisement_hdr.taddr;
			addr= *(uint128__t*)tmp;
			break;
		case ICMPV6_TYPE_REDIRECT_MESSAGE:
			tmp = ((cpc_icmpv6u_t*)hdr)->icmpv6u_redirect_hdr.taddr;
			addr= *(uint128__t*)tmp;
			break;
		default:
			//TODO LOG ERROR
			break;
	}
	return addr;
};

inline static
void set_icmpv6_neighbor_taddr(void *hdr, uint128__t taddr){
	uint128__t *ptr;
	switch (get_icmpv6_type(hdr)) {
		case ICMPV6_TYPE_NEIGHBOR_SOLICITATION:
			ptr= (uint128__t*)&((cpc_icmpv6u_t*)hdr)->icmpv6u_neighbor_solication_hdr.taddr;
			break;
		case ICMPV6_TYPE_NEIGHBOR_ADVERTISEMENT:
			ptr= (uint128__t*)&((cpc_icmpv6u_t*)hdr)->icmpv6u_neighbor_advertisement_hdr.taddr;
			break;
		case ICMPV6_TYPE_REDIRECT_MESSAGE:
			ptr= (uint128__t*)&((cpc_icmpv6u_t*)hdr)->icmpv6u_redirect_hdr.taddr;
			break;
		default:
			//TODO LOG ERROR
			return;
			break;
	}
	*ptr = taddr;
};


//ndp_rtr_flag
//ndp_solicited_flag
//ndp_override_flag
//neighbor_taddr


#endif //_CPC_ICMPV6_H_
