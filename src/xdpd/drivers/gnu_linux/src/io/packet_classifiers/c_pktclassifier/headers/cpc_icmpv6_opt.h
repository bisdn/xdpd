/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_ICMPV6_OPT_H_
#define _CPC_ICMPV6_OPT_H_

/**
* @file cpc_icmpv6_opt.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for the options of ICMPv6
*/

#define IPV6_ADDR_LEN		16
#define ETHER_ADDR_LEN		6

struct cpc_icmpv6_router_advertisement_hdr;
enum icmpv6_type_t {
	ICMPV6_TYPE_DESTINATION_UNREACHABLE 							= 1,
	ICMPV6_TYPE_PACKET_TOO_BIG										= 2,
	ICMPV6_TYPE_TIME_EXCEEDED										= 3,
	ICMPV6_TYPE_PARAMETER_PROBLEM									= 4,
	ICMPV6_TYPE_ECHO_REQUEST										= 128,
	ICMPV6_TYPE_ECHO_REPLY											= 129,
	ICMPV6_TYPE_MULTICAST_LISTENER_QUERY							= 130,
	ICMPV6_TYPE_MULTICAST_LISTENER_REPORT							= 131,
	ICMPV6_TYPE_MULTICAST_LISTENER_DONE								= 132,
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

enum icmpv6_option_type_t {
	ICMPV6_OPT_LLADDR_SOURCE 		= 1,
	ICMPV6_OPT_LLADDR_TARGET 		= 2,
	ICMPV6_OPT_PREFIX_INFO			= 3,
	ICMPV6_OPT_REDIRECT				= 4,
	ICMPV6_OPT_MTU					= 5,
};

/* ICMPv6 generic option header */
typedef struct cpc_icmpv6_option_hdr {
	uint8_t 							type;
	uint8_t								len;
	uint8_t 							data[0];
} __attribute__((packed)) cpc_icmpv6_option_hdr_t;


/* ICMPv6 link layer address option */
typedef struct cpc_icmpv6_lla_option {
	cpc_icmpv6_option_hdr_t				hdr;
	uint8_t								addr[ETHER_ADDR_LEN]; // len=1 (in 8-octets wide blocks) and we assume Ethernet here
} __attribute__((packed)) cpc_icmpv6_lla_option_t;

/* ICMPv6 prefix information option */
typedef struct cpc_icmpv6_prefix_info {
	cpc_icmpv6_option_hdr_t				hdr;
	uint8_t								pfxlen;
	uint8_t								flags;
	uint32_t							valid_lifetime;
	uint32_t							preferred_lifetime;
	uint32_t							reserved;
	uint8_t								prefix[IPV6_ADDR_LEN];
} __attribute__((packed)) cpc_icmpv6_prefix_info_t;

/* ICMPv6 redirected option header */
struct cpc_icmpv6_redirected_hdr {
	cpc_icmpv6_option_hdr_t				hdr;
	uint8_t								reserved[6];
	uint8_t								data[0];
} __attribute__((packed));

/* ICMPv6 MTU option */
struct cpc_icmpv6_mtu {
	cpc_icmpv6_option_hdr_t				hdr;
	uint8_t								reserved[2];
	uint32_t							mtu;
} __attribute__((packed));

typedef union cpc_icmpv6optu{
	cpc_icmpv6_option_hdr_t				optu;
	struct cpc_icmpv6_lla_option		optu_lla;
	struct cpc_icmpv6_prefix_info		optu_pfx;
	struct cpc_icmpv6_redirected_hdr	optu_rdr;
	struct cpc_icmpv6_mtu				optu_mtu;
} cpc_icmpv6optu_t;

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
	uint64_t *ret = (uint64_t*) &((cpc_icmpv6_lla_option_t*)hdr)->addr;
	return (*ret) & OF1X_6_BYTE_MASK;
};

inline static
void set_icmpv6_ll_taddr(void *hdr, uint64_t taddr){
	uint64_t *ptr = (uint64_t *) &((cpc_icmpv6_lla_option_t*)hdr)->addr;
	*ptr = (taddr & OF1X_6_BYTE_MASK) | (*ptr & ~OF1X_6_BYTE_MASK);
};

inline static
uint64_t get_icmpv6_ll_saddr(void *hdr){
	uint64_t *ret = (uint64_t*) &((cpc_icmpv6_lla_option_t*)hdr)->addr;
	return (*ret) & OF1X_6_BYTE_MASK;
};

inline static
void set_icmpv6_ll_saddr(void *hdr, uint64_t saddr){
	uint64_t *ptr = (uint64_t *) &((cpc_icmpv6_lla_option_t*)hdr)->addr;
	*ptr = (saddr & OF1X_6_BYTE_MASK) | (*ptr & ~OF1X_6_BYTE_MASK);
};

inline static
uint8_t get_icmpv6_pfx_on_link_flag(void *hdr){
	// we asume that the header exists
	return (((cpc_icmpv6_prefix_info_t *)hdr)->flags & OF1X_BIT7_MASK);
};

inline static
void set_icmpv6_pfx_on_link_flag(void *hdr, uint8_t flag){
	// we asume that the header exists
	((cpc_icmpv6_prefix_info_t *)hdr)->flags = (((cpc_icmpv6_prefix_info_t *)hdr)->flags & ~OF1X_BIT7_MASK) | (flag & OF1X_BIT7_MASK);
};

inline static
uint8_t get_icmpv6_pfx_aac_flag(void *hdr){
	// we asume that the header exists
	return (((cpc_icmpv6_prefix_info_t *)hdr)->flags & OF1X_BIT6_MASK);
};

inline static
void set_icmpv6_pfx_aac_flag(void *hdr, uint8_t flag){
	// we asume that the header exists
	((cpc_icmpv6_prefix_info_t *)hdr)->flags = (((cpc_icmpv6_prefix_info_t *)hdr)->flags & ~OF1X_BIT6_MASK) | (flag & OF1X_BIT6_MASK);
};


#endif //_CPC_ICMPV6_OPT_H_
