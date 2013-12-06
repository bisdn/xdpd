/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_ICMPV6_H_
#define _CPC_ICMPV6_H_

#include <rofl/datapath/pipeline/common/large_types.h>
#include "../cpc_utils.h"
#include "cpc_ethernet.h"
#include "../../../../util/likely.h"
#include "../../../../util/compiler_assert.h"

/**
* @file cpc_icmpv6.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for ICMPv6
*/

#define IPV6_ADDR_LEN		16
#define ETHER_ADDR_LEN		6

enum icmpv6_option_type_t {
	ICMPV6_OPT_LLADDR_SOURCE 		= 1,
	ICMPV6_OPT_LLADDR_TARGET 		= 2,
	ICMPV6_OPT_PREFIX_INFO			= 3,
	ICMPV6_OPT_REDIRECT				= 4,
	ICMPV6_OPT_MTU					= 5,
};

/* ICMPv6 generic option header */
struct cpc_icmpv6_option_hdr_t {
	uint8_t 							type;
	uint8_t								len;
	uint8_t 							data[0];
} __attribute__((packed));


/* ICMPv6 link layer address option */
struct cpc_icmpv6_lla_option_t {
	struct cpc_icmpv6_option_hdr_t		hdr;
	uint8_t								addr[ETHER_ADDR_LEN]; // len=1 (in 8-octets wide blocks) and we assume Ethernet here
} __attribute__((packed));

/* ICMPv6 prefix information option */
struct cpc_icmpv6_prefix_info_t {
	struct cpc_icmpv6_option_hdr_t		hdr;
	uint8_t								pfxlen;
	uint8_t								flags;
	uint32_t							valid_lifetime;
	uint32_t							preferred_lifetime;
	uint32_t							reserved;
	uint8_t								prefix[IPV6_ADDR_LEN];
} __attribute__((packed));

/* ICMPv6 redirected option header */
struct cpc_icmpv6_redirected_hdr_t {
	struct cpc_icmpv6_option_hdr_t		hdr;
	uint8_t								reserved[6];
	uint8_t								data[0];
} __attribute__((packed));

/* ICMPv6 MTU option */
struct cpc_icmpv6_mtu_t {
	struct cpc_icmpv6_option_hdr_t		hdr;
	uint8_t								reserved[2];
	uint32_t							mtu;
} __attribute__((packed));

typedef union icmpv6optu{
	struct cpc_icmpv6_option_hdr_t		optu;
	struct cpc_icmpv6_lla_option_t		optu_lla;
	struct cpc_icmpv6_prefix_info_t	optu_pfx;
	struct cpc_icmpv6_redirected_hdr_t	optu_rdr;
	struct cpc_icmpv6_mtu_t				optu_mtu;
} cpc_icmpv6optu_t;

enum icmpv6_ip_proto_t {
	ICMPV6_IP_PROTO = 58,
};

enum icmpv6_type_t {
	ICMPV6_TYPE_DESTINATION_UNREACHABLE 							= 1,
	ICMPV6_TYPE_PACKET_TOO_BIG										= 2,
	ICMPV6_TYPE_TIME_EXCEEDED										= 3,
	ICMPV6_TYPE_PARAMETER_PROBLEM									= 4,
	ICMPV6_TYPE_ECHO_REQUEST										= 128,
	ICMPV6_TYPE_ECHO_REPLY											= 129,
	ICMPV6_TYPE_MULTICAST_LISTENER_QUERY							= 130,
	ICMPV6_TYPE_MULTICAST_LISTENER_REPORT							= 131,
	ICMPV6_TYPE_MULTICAST_LISTENER_DONE							= 132,
	ICMPV6_TYPE_ROUTER_SOLICATION									= 133,
	ICMPV6_TYPE_ROUTER_ADVERTISEMENT								= 134,
	ICMPV6_TYPE_NEIGHBOR_SOLICITATION								= 135,
	ICMPV6_TYPE_NEIGHBOR_ADVERTISEMENT								= 136,
	ICMPV6_TYPE_REDIRECT_MESSAGE									= 137,
	ICMPV6_TYPE_ROUTER_RENUMBERING									= 138,
	ICMPV6_TYPE_ICMP_NODE_INFORMATION_QUERY						= 139,
	ICMPV6_TYPE_ICMP_NODE_INFORMATION_RESPONSE						= 140,
	ICMPV6_TYPE_INVERSE_NEIGHBOR_DISCOVERY_SOLICITATION_MESSAGE 	= 141,
	ICMPV6_TYPE_INVERSE_NEIGHBOR_DISCOVERY_ADVERTISEMENT_MESSAGE 	= 142,
	ICMPV6_TYPE_MULTICAST_LISTENER_DISCOVERY_REPORT				= 143,
	ICMPV6_TYPE_HOME_AGENT_ADDRESS_DISCOVERY_REQUEST_MESSAGE		= 144,
	ICMPV6_TYPE_HOME_AGENT_ADDRESS_DISCOVERY_REPLY_MESSAGE			= 145,
	ICMPV6_TYPE_MOBILE_PREFIX_SOLICITATION							= 146,
	ICMPV6_TYPE_MOBILE_PREFIX_ADVERTISEMENT						= 147,
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
struct cpc_icmpv6_dest_unreach_hdr_t {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint32_t						unused;					// a 32bit value
	uint8_t							data[0];				// the IP packet
} __attribute__((packed));

/* ICMPv6 message format for Packet Too Big */
struct cpc_icmpv6_pkt_too_big_hdr_t {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint32_t						unused;					// a 32bit value
	uint8_t							data[0];				// the IP packet
} __attribute__((packed));

/* ICMPv6 message format for Time Exceeded */
struct cpc_icmpv6_time_exceeded_hdr_t {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint32_t						unused;					// a 32bit value
	uint8_t							data[0];				// the IP packet
} __attribute__((packed));

/* ICMPv6 message format for Parameter Problem */
struct cpc_icmpv6_param_problem_hdr_t {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint32_t						pointer;				// a 32bit value
	uint8_t							data[0];				// the IP packet
} __attribute__((packed));

/* ICMPv6 echo request message format */
struct cpc_icmpv6_echo_request_hdr_t {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint16_t						id;
	uint16_t 						seqno;
	uint8_t							data[0];				// arbitrary data
} __attribute__((packed));

/* ICMPv6 echo reply message format */
struct cpc_icmpv6_echo_reply_hdr_t {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint16_t						id;
	uint16_t 						seqno;
	uint8_t							data[0];				// arbitrary data
} __attribute__((packed));


/**
	* ICMPv6 NDP message types
	*/

/* ICMPv6 router solicitation */
struct cpc_icmpv6_router_solicitation_hdr_t {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=133, code=0
	uint32_t 						reserved;				// reserved for later use, for now: mbz
	cpc_icmpv6optu_t				options[0];
} __attribute__((packed));

/* ICMPv6 router advertisement */
struct cpc_icmpv6_router_advertisement_hdr_t {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=134, code=0
	uint8_t 						cur_hop_limit;
	uint8_t							flags;
	uint16_t 						rtr_lifetime;
	uint32_t						reachable_timer;
	uint32_t 						retrans_timer;
	cpc_icmpv6optu_t				options[0];
} __attribute__((packed));

/* ICMPv6 neighbor solicitation */
struct cpc_icmpv6_neighbor_solicitation_hdr_t {
	cpc_icmpv6_hdr_t				icmpv6_header;			// type=135, code=0
	uint32_t 						reserved;				// reserved for later use, for now: mbz
	uint8_t							taddr[IPV6_ADDR_LEN]; 	// =target address
	cpc_icmpv6optu_t				options[0];
} __attribute__((packed));

/* ICMPv6 neighbor advertisement */
struct cpc_icmpv6_neighbor_advertisement_hdr_t {
	cpc_icmpv6_hdr_t				icmpv6_header;				// type=136, code=0
	uint32_t 						flags;
	uint8_t							taddr[IPV6_ADDR_LEN]; 	// =target address
	cpc_icmpv6optu_t				options[0];
} __attribute__((packed));

/* ICMPv6 redirect message */
struct cpc_icmpv6_redirect_hdr_t {
	cpc_icmpv6_hdr_t				icmpv6_header;				// type=137, code=0
	uint32_t 						reserved;				// reserved for later use, for now: mbz
	uint8_t							taddr[IPV6_ADDR_LEN]; 	// =target address
	uint8_t							daddr[IPV6_ADDR_LEN];	// =destination address
	cpc_icmpv6optu_t				options[0];
} __attribute__((packed));

typedef union cpc_icmpv6u{
	cpc_icmpv6_hdr_t 								icmpv6u_hdr;							// ICMPv6 message header
	struct cpc_icmpv6_dest_unreach_hdr_t			icmpv6u_dst_unreach_hdr;				// ICMPv6 destination unreachable
	struct cpc_icmpv6_pkt_too_big_hdr_t			icmpv6u_pkt_too_big_hdr;				// ICMPv6 packet too big
	struct cpc_icmpv6_time_exceeded_hdr_t			icmpv6u_time_exceeded_hdr;				// ICMPv6 time exceeded
	struct cpc_icmpv6_param_problem_hdr_t			icmpv6u_param_problem_hdr;				// ICMPv6 parameter problem
	struct cpc_icmpv6_echo_request_hdr_t			icmpv6u_echo_request_hdr;				// ICMPv6 echo request
	struct cpc_icmpv6_echo_reply_hdr_t				icmpv6u_echo_reply_hdr;				// ICMPv6 echo reply
	struct cpc_icmpv6_router_solicitation_hdr_t	icmpv6u_rtr_solicitation_hdr;			// ICMPv6 rtr solicitation
	struct cpc_icmpv6_router_advertisement_hdr_t	icmpv6u_rtr_advertisement_hdr;		// ICMPv6 rtr advertisement
	struct cpc_icmpv6_neighbor_solicitation_hdr_t	icmpv6u_neighbor_solication_hdr;		// ICMPv6 NDP solication header
	struct cpc_icmpv6_neighbor_advertisement_hdr_t	icmpv6u_neighbor_advertisement_hdr;	// ICMPv6 NDP advertisement header
	struct cpc_icmpv6_redirect_hdr_t				icmpv6u_redirect_hdr;					// ICMPV6 redirect header
}cpc_icmpv6u_t;


inline static
void icmpv6_calc_checksum(void *hdr, uint16_t length){
	//TODO Implement the checksum 
};

inline static
cpc_icmpv6optu_t *get_icmpv6_option(void *hdr){

	//TODO ... return the option of the specified type
	switch(((cpc_icmpv6_hdr_t*)hdr)->type){
		case ICMPV6_TYPE_ROUTER_SOLICATION:
			return (cpc_icmpv6optu_t *)((struct cpc_icmpv6_router_solicitation_hdr_t*)hdr)->options;
			break;
		case ICMPV6_TYPE_ROUTER_ADVERTISEMENT:
			return (cpc_icmpv6optu_t *)((struct cpc_icmpv6_router_advertisement_hdr_t*)hdr)->options;
			break;
		case ICMPV6_TYPE_NEIGHBOR_SOLICITATION:
			return (cpc_icmpv6optu_t *)((struct cpc_icmpv6_neighbor_solicitation_hdr_t*)hdr)->options;
			break;
		case ICMPV6_TYPE_NEIGHBOR_ADVERTISEMENT:
			return (cpc_icmpv6optu_t *)((struct cpc_icmpv6_neighbor_advertisement_hdr_t*)hdr)->options;
			break;
		case ICMPV6_TYPE_REDIRECT_MESSAGE:
			return (cpc_icmpv6optu_t *)((struct cpc_icmpv6_redirect_hdr_t*)hdr)->options;
			break;
		default:
			return NULL;
			break;
	}

	return 0;
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
	CPC_SWAP_U128(addr); //be128toh
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
	CPC_SWAP_U128(taddr); //htobe128
	*ptr = taddr;
};


//ndp_rtr_flag
//ndp_solicited_flag
//ndp_override_flag
//neighbor_taddr


inline static
uint8_t get_icmpv6_opt_type(void *hdr){
	return ((cpc_icmpv6optu_t*)hdr)->optu.type;
};

inline static
void set_icmpv6_opt_type(void *hdr, uint8_t type){
	((cpc_icmpv6optu_t*)hdr)->optu.type = type;
};

inline static
uint64_t get_icmpv6_ll_taddr(void *hdr){
	cpc_icmpv6optu_t *icmpv6_opt_hdr = get_icmpv6_option(hdr);
	
	if(NULL == icmpv6_opt_hdr || ICMPV6_OPT_LLADDR_TARGET != icmpv6_opt_hdr->optu.type){
		return 0;
	}
	uint64_t ret =mac_addr_to_u64(icmpv6_opt_hdr->optu_lla.addr);
	CPC_SWAP_MAC(ret);
	return ret;
};

inline static
void set_icmpv6_ll_taddr(void *hdr, uint64_t taddr){
	cpc_icmpv6optu_t *icmpv6_opt_hdr = get_icmpv6_option(hdr);
	
	if(NULL == icmpv6_opt_hdr || ICMPV6_OPT_LLADDR_TARGET != icmpv6_opt_hdr->optu.type){
		return;
	}
	CPC_SWAP_MAC(taddr);
	u64_to_mac_ptr(icmpv6_opt_hdr->optu_lla.addr,taddr);
};

inline static
uint64_t get_icmpv6_ll_saddr(void *hdr){
	cpc_icmpv6optu_t *icmpv6_opt_hdr = get_icmpv6_option(hdr);
	
	if(NULL == icmpv6_opt_hdr || ICMPV6_OPT_LLADDR_SOURCE !=icmpv6_opt_hdr->optu.type){
		return 0;
	}
	
	uint64_t ret =mac_addr_to_u64(icmpv6_opt_hdr->optu_lla.addr);
	CPC_SWAP_MAC(ret);
	return ret;
};

inline static
void set_icmpv6_ll_saddr(void *hdr, uint64_t saddr){
	cpc_icmpv6optu_t *icmpv6_opt_hdr = get_icmpv6_option(hdr);
	
	if(NULL == icmpv6_opt_hdr || ICMPV6_OPT_LLADDR_SOURCE != icmpv6_opt_hdr->optu.type){
		return;
	}
	CPC_SWAP_MAC(saddr);
	u64_to_mac_ptr(icmpv6_opt_hdr->optu_lla.addr,saddr);
};

inline static
uint8_t get_icmpv6_pfx_on_link_flag(void *hdr){
	cpc_icmpv6optu_t *icmpv6_opt_hdr = get_icmpv6_option(hdr);
	
	if(NULL == icmpv6_opt_hdr || ICMPV6_OPT_PREFIX_INFO != icmpv6_opt_hdr->optu.type){
		return 0;
	}
	return ( (icmpv6_opt_hdr->optu_pfx.flags & 0x80) >> 7 );
};

inline static
void set_icmpv6_pfx_on_link_flag(void *hdr, uint8_t flag){
	cpc_icmpv6optu_t *icmpv6_opt_hdr = get_icmpv6_option(hdr);
	
	if(NULL == icmpv6_opt_hdr || ICMPV6_OPT_PREFIX_INFO != icmpv6_opt_hdr->optu.type){
		return;
	}
	icmpv6_opt_hdr->optu_pfx.flags = (icmpv6_opt_hdr->optu_pfx.flags & 0x7F) | ((flag & 0x01) << 7);
};

inline static
uint8_t get_icmpv6_pfx_aac_flag(void *hdr){
	cpc_icmpv6optu_t *icmpv6_opt_hdr = get_icmpv6_option(hdr);
	
	if(NULL == icmpv6_opt_hdr || ICMPV6_OPT_PREFIX_INFO != icmpv6_opt_hdr->optu.type){
		return 0;
	}
	return ((icmpv6_opt_hdr->optu_pfx.flags & 0x40) >> 6);
};

inline static
void set_icmpv6_pfx_aac_flag(void *hdr, uint8_t flag){
	cpc_icmpv6optu_t *icmpv6_opt_hdr = get_icmpv6_option(hdr);
	
	if(NULL == icmpv6_opt_hdr || ICMPV6_OPT_PREFIX_INFO != icmpv6_opt_hdr->optu.type){
		return;
	}
	icmpv6_opt_hdr->optu_pfx.flags = (icmpv6_opt_hdr->optu_pfx.flags & 0xBF) | ((flag & 0x01) << 6);
};

#endif //_CPC_ICMPV6_H_
