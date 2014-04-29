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
#include "./headers/cpc_ppp.h"
#include "./headers/cpc_pppoe.h"
#include "./headers/cpc_tcp.h"
#include "./headers/cpc_udp.h"
#include "./headers/cpc_vlan.h"

//#include "pkt_types_mockup.h"
#include "autogen_pkt_types.h"

/**
* @file c_types_pktclassifier.h
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Interface for the C classifiers
*/
typedef struct classify_state{
	bool is_classified;
	
	//Packet type
	pkt_types_t type;
	
	//Pointer + len	
	uint8_t* base;	
	size_t len;
	
	//Port in and phy port
	uint32_t port_in;
	uint32_t phy_port_in;
	
}classify_state_t;

ROFL_BEGIN_DECLS

//inline function implementations
static inline 
void* get_ether_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_ETHERNET);
	if(!tmp)
		PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_8023);
	return tmp; 
}

static inline
void* get_vlan_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_VLAN); 
	return tmp; 
}

static inline
void* get_mpls_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_MPLS); 
	return tmp; 
}

static inline
void* get_arpv4_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_ARPV4); 
	return tmp; 
}

static inline
void* get_ipv4_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_IPV4); 
	return tmp; 
}

static inline
void* get_icmpv4_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV4); 
	return tmp; 
}

static inline
void* get_ipv6_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_IPV6); 
	return tmp; 
}

static inline
void* get_icmpv6_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6); 
	return tmp; 
}

static inline
void* get_icmpv6_opt_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6_OPTS); 
	return tmp; 
}

static inline
void* get_icmpv6_opt_lladr_source_hdr(classify_state_t* clas_state, int idx){		uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6_OPTS_LLADR_SRC); 
	return tmp; 
}

static inline
void* get_icmpv6_opt_lladr_target_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6_OPTS_LLADR_TGT); 
	return tmp; 
}

static inline
void* get_icmpv6_opt_prefix_info_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_ICMPV6_OPTS_PREFIX_INFO); 
	return tmp; 
}

static inline
void* get_udp_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_UDP); 
	return tmp;
}

static inline
void* get_tcp_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_TCP); 
	return tmp;
}

static inline
void* get_pppoe_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_PPPOE); 
	return tmp;
}

static inline
void* get_ppp_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_PPP); 
	return tmp;
}

static inline
void* get_gtpu_hdr(classify_state_t* clas_state, int idx){
	uint8_t* tmp;
	PKT_TYPES_GET_HDR(tmp, clas_state, PT_PROTO_GTPU); 
	return tmp;
}

//
// Parsing code
//

static inline
void parse_ipv4(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	unsigned header_len_bytes;	

	//Set reference
	cpc_ipv4_hdr_t *ipv4 = (cpc_ipv4_hdr_t*)data; 

	//Set frame
	header_len_bytes = ipv4->ihlvers*4;
	data += header_len_bytes;
	datalen -=  header_len_bytes; 


	switch (get_ipv4_proto(ipv4)) {
		case IPV4_IP_PROTO:
			{
				//parse_ipv4(clas_state, data, datalen);
			}
			break;
		case ICMPV4_IP_PROTO:
			{
				//parse_icmpv4(clas_state, data, datalen);
			}
			break;
		case UDP_IP_PROTO:
			{
				//parse_udp(clas_state, data, datalen);
			}
			break;
		case TCP_IP_PROTO:
			{
				//parse_tcp(clas_state, data, datalen);
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

}


static inline
void parse_mpls(classify_state_t* clas_state, uint8_t* data, size_t datalen)
{
	
	//Determine the number of MPLS stacked tags
	unsigned int n_labels=0;	
	cpc_mpls_hdr_t* mpls;

	do{
		n_labels++;
		data += sizeof(cpc_mpls_hdr_t);
		datalen -= sizeof(cpc_mpls_hdr_t);
		mpls = (cpc_mpls_hdr_t*)data;
	}while(! get_mpls_bos(mpls));

	//Supported types; MPLS over ethernet, MPLS over VLAN
	switch (clas_state->type){
		case PT_ETHERNET:
				PT_ASSIGN_MPLS_TYPE(clas_state->type, PT_ETHERNET, n_labels);
				break;
		case PT_ETHERNET_VLAN:
				PT_ASSIGN_MPLS_TYPE(clas_state->type, PT_ETHERNET_VLAN, n_labels);
				break;
		default:
			assert(0);
			return;
	}
}

static inline
void parse_vlan(classify_state_t* clas_state, uint8_t *data, size_t datalen){

	//Data pointer	
	cpc_vlan_hdr_t* vlan;
	unsigned int n_vlans=0;	
	uint16_t eth_type;
	n_vlans=0;

	//Set frame
	do{
		vlan = (cpc_vlan_hdr_t *)data;
		eth_type = get_vlan_type(vlan);
		data += sizeof(cpc_vlan_hdr_t);
		datalen -= sizeof(cpc_vlan_hdr_t);
		n_vlans++;
	}while(
		eth_type == VLAN_CTAG_ETHER_TYPE || 
		eth_type == VLAN_STAG_ETHER_TYPE ||
		eth_type == VLAN_ITAG_ETHER_TYPE 
	);

	//Assign type
	//FIXME: use n_vlans 
	clas_state->type = PT_ETHERNET_VLAN;	
	
	switch (eth_type) {
		case ETH_TYPE_MPLS_UNICAST:
		case ETH_TYPE_MPLS_MULTICAST:
			{
				parse_mpls(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_PPPOE_DISCOVERY:
		case ETH_TYPE_PPPOE_SESSION:
			{
				//parse_pppoe(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_ARP:
			{
				//parse_arpv4(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_IPV4:
			{
				//parse_ipv4(clas_state, data, datalen);
			}
			break;
		default:
			break;
	}
}

static inline 
void parse_ethernet(classify_state_t* clas_state, uint8_t* data, size_t datalen){

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

	switch ( get_ether_type(ether)) {
		case VLAN_CTAG_ETHER_TYPE:
		case VLAN_STAG_ETHER_TYPE:
		case VLAN_ITAG_ETHER_TYPE:
			{
				//parse_vlan(clas_state, data, datalen);
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
				//parse_pppoe(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_ARP:
			{
				//parse_arpv4(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_IPV4:
			{
				parse_ipv4(clas_state, data, datalen);
			}
			break;
		case ETH_TYPE_IPV6:
			{
				//parse_ipv6(clas_state, data,datalen);
			}
			break;
		default:
			{
				
			}
			break;
	}


}

//
// Main functions
//

static inline
void reset_classifier(classify_state_t* clas_state){
	clas_state->is_classified = false;
}

static inline
void classify_packet(classify_state_t* clas_state, uint8_t* data, size_t len, uint32_t port_in, uint32_t phy_port_in){

	//Set basic 
	clas_state->base = data;
	clas_state->len = len;
	clas_state->port_in = port_in;
	clas_state->phy_port_in = phy_port_in;

	//XXX FIXME start to parse in ethernet ethernet	
	
	//Mark it as classified
	clas_state->is_classified = true;
}

ROFL_END_DECLS

#endif //_C_TYPES_PKTCLASSIFIER_H_
