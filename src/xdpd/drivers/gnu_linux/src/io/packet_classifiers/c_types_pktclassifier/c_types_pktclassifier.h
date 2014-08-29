/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _C_TYPES_PKTCLASSIFIER_H_
#define _C_TYPES_PKTCLASSIFIER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "../pktclassifier.h"

//Headers
#include "./headers/cpc_arpv4.h"
#include "./headers/cpc_ethernet.h"
#include "./headers/cpc_gtpu.h"
#include "./headers/cpc_icmpv4.h"
#include "./headers/cpc_icmpv6_opt.h"
#include "./headers/cpc_icmpv6.h"
#include "./headers/cpc_ipv4.h"
#include "./headers/cpc_ipv6.h"
#include "./headers/cpc_mpls.h"
#include "./headers/cpc_pbb.h"
#include "./headers/cpc_ppp.h"
#include "./headers/cpc_pppoe.h"
#include "./headers/cpc_tcp.h"
#include "./headers/cpc_udp.h"
#include "./headers/cpc_sctp.h"
#include "./headers/cpc_vlan.h"

#include "autogen_pkt_types.h"

/**
* @file c_types_pktclassifier.h
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Interface for the C classifiers
*/


/**
* Checksum enumeration
*/

enum calculate_checksum {
	RESET_CHECKSUM_IN_SW_FLAGS		= 0,
	RECALCULATE_IPV4_CHECKSUM_IN_SW		= 1,
	RECALCULATE_TCP_CHECKSUM_IN_SW		= 2,
	RECALCULATE_UDP_CHECKSUM_IN_SW		= 3,
	RECALCULATE_SCTP_CHECKSUM_IN_SW		= 4,
	RECALCULATE_ICMPV4_CHECKSUM_IN_SW	= 5,
	RECALCULATE_ICMPV6_CHECKSUM_IN_SW	= 6
};

/**
* Classifier state
*/
typedef struct classifier_state{
	//Packet type
	pkt_types_t type;

	//Pointer + len 
	uint8_t* base;  
	size_t len;
	
	//Port in and phy port
	uint32_t port_in;
	uint32_t phy_port_in;

	//Checksum calculation in sw bitmap 
	uint32_t calculate_checksums_in_sw;
}classifier_state_t;

ROFL_BEGIN_DECLS

//
// Checksums helpers
//

/**
* Set checksum calculation flags
*/
static inline void set_recalculate_checksum(classifier_state_t* clas_state, enum calculate_checksum type){
#ifndef DONT_CALCULATE_ANY_CHECKSUM_IN_SW
	clas_state->calculate_checksums_in_sw |= ( 1 << type );
#endif
}

/**
* Set checksum calculation flags
*/
static inline bool is_recalculate_checksum_flag_set(classifier_state_t* clas_state, enum calculate_checksum type){
#ifndef DONT_CALCULATE_ANY_CHECKSUM_IN_SW
	return ( clas_state->calculate_checksums_in_sw & ( 1 << type ) ) > 0;
#else
	return false;
#endif
}

//
// Network protocol headers
//

//inline function implementations
static inline 
void* get_ether_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_ETHERNET);
	if(!tmp)
		PT_GET_HDR(tmp, clas_state, PT_PROTO_8023);
	return tmp; 
}

static inline
void* get_vlan_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_VLAN); 
	return tmp; 
}

static inline
void* get_pbb_isid_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_ISID); 
	return tmp; 
}

static inline
void* get_mpls_hdr(classifier_state_t* clas_state, int idx){
	
	uint8_t* tmp;
	
	if(idx == 0){
		//Outer most	
		PT_GET_HDR(tmp, clas_state, PT_PROTO_MPLS); 
		return tmp;
	}else{
		int num_of_lables = mpls_num_of_labels[clas_state->type];
	
		if( idx >= 0 ){
			if( idx < num_of_lables ){
				PT_GET_HDR(tmp, clas_state, PT_PROTO_MPLS); 
				return tmp + ( (idx) * 4);
			}
		}else{
			if((num_of_lables+idx+1) >= 0){
				PT_GET_HDR(tmp, clas_state, PT_PROTO_MPLS); 
				return tmp + ( (num_of_lables+idx+1) * 4 );
			}
		}
	}

	return NULL;
}

static inline
void* get_arpv4_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_ARPV4); 
	return tmp; 
}

static inline
void* get_ipv4_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_IPV4); 
	return tmp; 
}

static inline
void* get_icmpv4_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV4); 
	return tmp; 
}

static inline
void* get_ipv6_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_IPV6); 
	return tmp; 
}

static inline
void* get_icmpv6_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6); 
	return tmp; 
}

static inline
void* get_icmpv6_opt_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6_OPTS); 
	return tmp; 
}

static inline
void* get_icmpv6_opt_lladr_source_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6_OPTS_LLADR_SRC);
	if (NULL == tmp)
		PT_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6_RTR_SOL_OPTS_LLADR_SRC);
	if (NULL == tmp)
		PT_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6_RTR_ADV_OPTS_LLADR_SRC);
	if (NULL == tmp)
		PT_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6_NEIGH_SOL_OPTS_LLADR_SRC);
	return tmp; 
}

static inline
void* get_icmpv6_opt_lladr_target_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6_OPTS_LLADR_TGT);
	if (NULL == tmp)
		PT_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6_NEIGH_ADV_OPTS_LLADR_TGT);
	return tmp; 
}

static inline
void* get_icmpv6_opt_prefix_info_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6_OPTS_PREFIX_INFO); 
	return tmp; 
}

static inline
void* get_sctp_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_SCTP);
	return tmp;
}

static inline
void* get_udp_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_UDP); 
	return tmp;
}

static inline
void* get_tcp_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_TCP); 
	return tmp;
}

static inline
void* get_pppoe_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_PPPOE); 
	return tmp;
}

static inline
void* get_ppp_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_PPP); 
	return tmp;
}

static inline
void* get_gtpu_hdr(classifier_state_t* clas_state, int idx){
	uint8_t* tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_GTPU4);
	if (tmp != NULL)
		return tmp;
	PT_GET_HDR(tmp, clas_state, PT_PROTO_GTPU6);
	if (tmp != NULL)
		return tmp;
	return NULL;
}

//
// Parsing code
//

static inline
void parse_tcp(classifier_state_t* clas_state, uint8_t *data, size_t datalen){
	PT_CLASS_ADD_PROTO(clas_state, TCP);	
	//No further parsing
}

static inline
void parse_gtp(classifier_state_t* clas_state, uint8_t *data, size_t datalen){
	uint8_t* tmp;
	if (unlikely(datalen < sizeof(cpc_gtphu_t))) { return; }
	PT_GET_HDR(tmp, clas_state, PT_PROTO_IPV4);
	if(tmp != NULL) {
		PT_CLASS_ADD_PROTO(clas_state, GTPU4); return;
	}
	PT_GET_HDR(tmp, clas_state, PT_PROTO_IPV6);
	if (tmp != NULL) {
		PT_CLASS_ADD_PROTO(clas_state, GTPU6); return;
	}
	//No further parsing
}

static inline
void parse_udp(classifier_state_t* clas_state, uint8_t *data, size_t datalen){

	cpc_udp_hdr_t *udp = (cpc_udp_hdr_t*)data; 
	
	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_udp_hdr_t);
	datalen -= sizeof(cpc_udp_hdr_t);

	PT_CLASS_ADD_PROTO(clas_state, UDP);	

	assert(clas_state->type != PT_INVALID);
	
	if (datalen > 0){
		switch (*get_udp_dport(udp)) {
		case UDP_DST_PORT_GTPU: {
			parse_gtp(clas_state, data, datalen);
		} break;
		default: {
			//TODO: something
		} break;
		}
	}
}

static inline
void parse_sctp(classifier_state_t* clas_state, uint8_t *data, size_t datalen){
	PT_CLASS_ADD_PROTO(clas_state, SCTP);
	//No further parsing
}

static inline
void parse_arpv4(classifier_state_t* clas_state, uint8_t *data, size_t datalen){
	PT_CLASS_ADD_PROTO(clas_state, ARPV4);	
	//No further parsing
	assert(clas_state->type != PT_INVALID);
}
static inline
void parse_icmpv4(classifier_state_t* clas_state, uint8_t *data, size_t datalen){
	PT_CLASS_ADD_PROTO(clas_state, ICMPV4);	
	//No further parsing
	assert(clas_state->type != PT_INVALID);
}

static inline
void parse_ipv4(classifier_state_t* clas_state, uint8_t *data, size_t datalen){

	unsigned header_len_bytes, num_of_options;
	//Set reference
	cpc_ipv4_hdr_t *ipv4 = (cpc_ipv4_hdr_t*)data; 

	//Set frame
	header_len_bytes =(ipv4->ihlvers&0x0F)*4;
	num_of_options = (ipv4->ihlvers&0x0F) -(sizeof(cpc_ipv4_hdr_t)/4);
	data += header_len_bytes;
	datalen -=  header_len_bytes; 

#ifdef DEBUG
	//Prevent malformed packets to raise asserts()
	if(unlikely(num_of_options < 0) || unlikely(num_of_options > 15))
		return; 
#endif
	//Assign type
	PT_CLASS_ADD_IPV4_OPTIONS(clas_state, num_of_options);
	assert(clas_state->type != PT_INVALID);

	switch (*get_ipv4_proto(ipv4)) {
		case IPV4_IP_PROTO:
			//IPv4 on IPv4 not supported
			//parse_ipv4(clas_state, data, datalen);
			break;
		case ICMPV4_IP_PROTO:
			parse_icmpv4(clas_state, data, datalen);
			break;
		case UDP_IP_PROTO:
			parse_udp(clas_state, data, datalen);
			break;
		case TCP_IP_PROTO:
			parse_tcp(clas_state, data, datalen);
			break;
		case SCTP_IP_PROTO:
			parse_sctp(clas_state, data, datalen);
			break;
		default:
			break;
	}
}
static inline
void parse_icmpv6_opts(classifier_state_t* clas_state, uint8_t *data, size_t datalen){
	
	/*So far we only parse optionsICMPV6_OPT_LLADDR_TARGET, ICMPV6_OPT_LLADDR_SOURCE and ICMPV6_OPT_PREFIX_INFO*/
	cpc_icmpv6_option_hdr_t* icmpv6_options = (cpc_icmpv6_option_hdr_t*)data;
	
	//we asume here that there is only one option for each type
	switch(icmpv6_options->type){
		case ICMPV6_OPT_LLADDR_SOURCE:
			PT_CLASS_ADD_PROTO(clas_state, ICMPV6_OPTS_LLADR_SRC);	
			break;
		case ICMPV6_OPT_LLADDR_TARGET:
			//PT_CLASS_ADD_PROTO(clas_state, ICMPV6_OPTS_PREFIX_INFO);
			PT_CLASS_ADD_PROTO(clas_state, ICMPV6_OPTS_LLADR_TGT);
			break;
		case ICMPV6_OPT_PREFIX_INFO:
			//PT_CLASS_ADD_PROTO(clas_state, ICMPV6);
			PT_CLASS_ADD_PROTO(clas_state, ICMPV6_OPTS_PREFIX_INFO);
			break;
	}

	//We don't go beyond this
}

static inline
void parse_icmpv6(classifier_state_t* clas_state, uint8_t *data, size_t datalen){

	cpc_icmpv6_hdr_t* icmpv6 = (cpc_icmpv6_hdr_t*)data;
	
#if 1
	PT_CLASS_ADD_PROTO(clas_state, ICMPV6);	
	assert(clas_state->type != PT_INVALID);
#endif
	
	//Increment pointers and decrement remaining payload size (depending on type)
	switch( *get_icmpv6_type(icmpv6) ){
		case ICMPV6_TYPE_ROUTER_SOLICATION:{
			PT_CLASS_ADD_PROTO(clas_state, ICMPV6_RTR_SOL);
			assert(clas_state->type != PT_INVALID);
			data += sizeof(struct cpc_icmpv6_router_solicitation_hdr);
			datalen -= sizeof(struct cpc_icmpv6_router_solicitation_hdr);

			cpc_icmpv6_option_hdr_t* icmpv6_options = (cpc_icmpv6_option_hdr_t*)data;
			//we asume here that there is only one option for each type
			switch(icmpv6_options->type){
				case ICMPV6_OPT_LLADDR_SOURCE:
					PT_CLASS_ADD_PROTO(clas_state, ICMPV6_RTR_SOL_OPTS_LLADR_SRC);
					break;
			}

			}return;
		case ICMPV6_TYPE_ROUTER_ADVERTISEMENT:{
			PT_CLASS_ADD_PROTO(clas_state, ICMPV6_RTR_ADV);
			assert(clas_state->type != PT_INVALID);
			data += sizeof(struct cpc_icmpv6_router_advertisement_hdr);
			datalen -= sizeof(struct cpc_icmpv6_router_advertisement_hdr);

			cpc_icmpv6_option_hdr_t* icmpv6_options = (cpc_icmpv6_option_hdr_t*)data;
			//we asume here that there is only one option for each type
			switch(icmpv6_options->type){
				case ICMPV6_OPT_LLADDR_SOURCE:
					PT_CLASS_ADD_PROTO(clas_state, ICMPV6_RTR_ADV_OPTS_LLADR_SRC);
					break;
			}

			}return;
		case ICMPV6_TYPE_NEIGHBOR_SOLICITATION:{
			PT_CLASS_ADD_PROTO(clas_state, ICMPV6_NEIGH_SOL);
			assert(clas_state->type != PT_INVALID);
			data += sizeof(struct cpc_icmpv6_neighbor_solicitation_hdr);
			datalen -= sizeof(struct cpc_icmpv6_neighbor_solicitation_hdr);

			cpc_icmpv6_option_hdr_t* icmpv6_options = (cpc_icmpv6_option_hdr_t*)data;
			//we asume here that there is only one option for each type
			switch(icmpv6_options->type){
				case ICMPV6_OPT_LLADDR_SOURCE:
					PT_CLASS_ADD_PROTO(clas_state, ICMPV6_NEIGH_SOL_OPTS_LLADR_SRC);
					break;
			}

			}return;
		case ICMPV6_TYPE_NEIGHBOR_ADVERTISEMENT:{
			PT_CLASS_ADD_PROTO(clas_state, ICMPV6_NEIGH_ADV);
			assert(clas_state->type != PT_INVALID);
			data += sizeof(struct cpc_icmpv6_neighbor_advertisement_hdr);
			datalen -= sizeof(struct cpc_icmpv6_neighbor_advertisement_hdr);

			cpc_icmpv6_option_hdr_t* icmpv6_options = (cpc_icmpv6_option_hdr_t*)data;
			//we asume here that there is only one option for each type
			switch(icmpv6_options->type){
				case ICMPV6_OPT_LLADDR_TARGET:
					PT_CLASS_ADD_PROTO(clas_state, ICMPV6_NEIGH_ADV_OPTS_LLADR_TGT);
					break;
			}

			}return;
		case ICMPV6_TYPE_REDIRECT_MESSAGE:
			PT_CLASS_ADD_PROTO(clas_state, ICMPV6_REDIRECT);
			assert(clas_state->type != PT_INVALID);
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
void parse_ipv6(classifier_state_t* clas_state, uint8_t *data, size_t datalen){
	
	//Set reference
	cpc_ipv6_hdr_t *ipv6 = (cpc_ipv6_hdr_t*)data; 

	PT_CLASS_ADD_PROTO(clas_state, IPV6);	
	assert(clas_state->type != PT_INVALID);

	//Increment pointers and decrement remaining payload size
	data += sizeof(cpc_ipv6_hdr_t);
	datalen -= sizeof(cpc_ipv6_hdr_t);

	// FIXME: IP header with options

	switch ( *get_ipv6_next_header(ipv6) ) {
		case IPV4_IP_PROTO:
			//Not supported
			//parse_ipv4(clas_state, data, datalen);
			break;
		case IPV6_IP_PROTO:
			//Not supported
			//parse_ipv6(clas_state, data, datalen);
			break;
		case ICMPV6_IP_PROTO:
			parse_icmpv6(clas_state, data, datalen);
			break;
		case UDP_IP_PROTO:
			parse_udp(clas_state, data, datalen);
			break;
		case TCP_IP_PROTO:
			parse_tcp(clas_state, data, datalen);
			break;
		case SCTP_IP_PROTO:
			parse_sctp(clas_state, data, datalen);
			break;
		default:
			break;
	}
}

static inline
void parse_mpls(classifier_state_t* clas_state, uint8_t* data, size_t datalen)
{
	
	//Determine the number of MPLS stacked tags
	unsigned int n_labels=0;	
	cpc_mpls_hdr_t* mpls;

	do{
		n_labels++;
		mpls = (cpc_mpls_hdr_t*)data;
		data += sizeof(cpc_mpls_hdr_t);
		datalen -= sizeof(cpc_mpls_hdr_t);

		//Add label to the stack
		PT_CLASS_ADD_PROTO(clas_state, MPLS);	
	}while(! get_mpls_bos(mpls));
		
	assert(clas_state->type != PT_INVALID);
	//MPLS does not have explicit knowledge of the headers on top of it; so classification stops here
}

static inline
void parse_ppp(classifier_state_t* clas_state, uint8_t *data, size_t datalen){

	PT_CLASS_ADD_PROTO(clas_state, PPP);	
	assert(clas_state->type != PT_INVALID);

	//We currently don't parse beyond PPPoE	
}

static inline
void parse_pppoe(classifier_state_t* clas_state, uint8_t *data, size_t datalen, uint16_t eth_type){

	PT_CLASS_ADD_PROTO(clas_state, PPPOE);
	assert(clas_state->type != PT_INVALID);

	switch (eth_type) {
		case ETH_TYPE_PPPOE_DISCOVERY:
			datalen -= sizeof(cpc_pppoe_hdr_t);
			break;
		case ETH_TYPE_PPPOE_SESSION:
			//Increment pointers and decrement remaining payload size
			data += sizeof(cpc_pppoe_hdr_t);
			datalen -= sizeof(cpc_pppoe_hdr_t);

			parse_ppp(clas_state,data, datalen);
			break;
		default:
			// log error?
			break;
	}
}

static inline
void parse_pbb_isid(classifier_state_t* clas_state, uint8_t *data, size_t datalen){

	//Special parsing schema
	clas_state->type = PT_ETHERNET_ISID_ETHERNET;

	//We don't parse beyond PBB

	return;
}


static inline
void parse_vlan(classifier_state_t* clas_state, uint8_t *data, size_t datalen){

	//Data pointer	
	cpc_vlan_hdr_t* vlan;
	uint16_t eth_type;

	//Set frame
	do{
		vlan = (cpc_vlan_hdr_t *)data;
		data += sizeof(cpc_vlan_hdr_t);
		datalen -= sizeof(cpc_vlan_hdr_t);
		eth_type = *get_vlan_type(vlan);
	
		//Add parsed vlan	
		PT_CLASS_ADD_PROTO(clas_state, VLAN);
	}while(
		eth_type == VLAN_CTAG_ETHER_TYPE || 
		eth_type == VLAN_STAG_ETHER_TYPE 
	);
	
	assert(clas_state->type != PT_INVALID);

	switch (eth_type) {
		case VLAN_ITAG_ETHER_TYPE:
			parse_pbb_isid(clas_state, data, datalen);	
			break;
		case ETH_TYPE_MPLS_UNICAST:
		case ETH_TYPE_MPLS_MULTICAST:
			parse_mpls(clas_state, data, datalen);
			break;
		case ETH_TYPE_PPPOE_DISCOVERY:
		case ETH_TYPE_PPPOE_SESSION:
			parse_pppoe(clas_state, data, datalen, eth_type);
			break;
		case ETH_TYPE_ARP:
			parse_arpv4(clas_state, data, datalen);
			break;
		case ETH_TYPE_IPV4:
			parse_ipv4(clas_state, data, datalen);
			break;
		case ETH_TYPE_IPV6:
			parse_ipv6(clas_state, data, datalen);
			break;

		default:
			break;
	}
}

static inline 
void parse_ethernet(classifier_state_t* clas_state, uint8_t* data, size_t datalen){

	//Data pointer	
	cpc_eth_hdr_t* ether = (cpc_eth_hdr_t *)data;

	//Set frame
	if( is_llc_frame(ether) ){
		data += sizeof(cpc_eth_llc_hdr_t);
		datalen -= sizeof(cpc_eth_llc_hdr_t);
		clas_state->type = PT_8023;
	}else{
		data += sizeof(cpc_eth_hdr_t);
		datalen -= sizeof(cpc_eth_hdr_t);
		clas_state->type = PT_ETHERNET;
	}

	switch ( *get_ether_type(ether) ) {
		case VLAN_ITAG_ETHER_TYPE:
			parse_pbb_isid(clas_state, data, datalen);	
			break;
		case VLAN_CTAG_ETHER_TYPE:
		case VLAN_STAG_ETHER_TYPE:
			parse_vlan(clas_state, data, datalen);
			break;
		case ETH_TYPE_MPLS_UNICAST:
		case ETH_TYPE_MPLS_MULTICAST:
			parse_mpls(clas_state, data, datalen);
			break;
		case ETH_TYPE_PPPOE_DISCOVERY:
		case ETH_TYPE_PPPOE_SESSION:
			parse_pppoe(clas_state, data, datalen, *get_ether_type(ether));
			break;
		case ETH_TYPE_ARP:
			parse_arpv4(clas_state, data, datalen);
			break;
		case ETH_TYPE_IPV4:
			parse_ipv4(clas_state, data, datalen);
			break;
		case ETH_TYPE_IPV6:
			parse_ipv6(clas_state, data,datalen);
			break;
		default:
			break;
	}
}


//
// Ethertype: get it regardless of the packet type (inner ETH_TYPE)
//
static inline
uint16_t* classifier_get_eth_type(classifier_state_t* clas_state){
	return (uint16_t*) (clas_state->base + eth_type_offsets[clas_state->type]);
}

//
// Main functions
//

static inline
void classify_packet(classifier_state_t* clas_state, uint8_t* data, size_t len, uint32_t port_in, uint32_t phy_port_in){
	
	//Set basic 
	clas_state->base = data;
	clas_state->len = len;
	clas_state->port_in = port_in;
	clas_state->phy_port_in = phy_port_in;

#ifndef DONT_CALCULATE_ANY_CHECKSUM_IN_SW
	//Reset checksums calculation in sw flags
	clas_state->calculate_checksums_in_sw = RESET_CHECKSUM_IN_SW_FLAGS;
#endif
	
	//Determine packet type	
	parse_ethernet(clas_state, data, len);
}




ROFL_END_DECLS

#endif //_C_TYPES_PKTCLASSIFIER_H_
