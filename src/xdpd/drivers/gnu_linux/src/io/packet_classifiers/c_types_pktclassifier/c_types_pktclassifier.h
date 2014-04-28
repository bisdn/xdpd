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

#include "pkt_types.h"

/**
* @file c_types_pktclassifier.h
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Interface for the C classifiers
*/
typedef struct classify_state{
	bool is_classified;
	pkt_types_t type;	
}classify_state_t;

ROFL_BEGIN_DECLS

//inline function implementations
static inline 
void* get_ether_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_vlan_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_mpls_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_arpv4_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_ipv4_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_icmpv4_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_ipv6_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_icmpv6_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_icmpv6_opt_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_icmpv6_opt_lladr_source_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_icmpv6_opt_lladr_target_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_icmpv6_opt_prefix_info_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_udp_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_tcp_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_pppoe_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_ppp_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

static inline
void* get_gtpu_hdr(classify_state_t* clas_state, int idx){
	//XXX FIXME
	return NULL;
}

//shifts
static inline 
void shift_ether(classify_state_t* clas_state, int idx, ssize_t bytes){
	//XXX FIXME
}

static inline
void shift_vlan(classify_state_t* clas_state, int idx, ssize_t bytes){
	//XXX FIXME
}

//
// Parsing code
//

static inline
void reset_classifier(classify_state_t* clas_state){
	clas_state->is_classified = false;
}

static inline
void classify_packet(classify_state_t* clas_state, uint8_t* data, size_t len, uint32_t port_in, uint32_t phy_port_in){
	reset_classifier(clas_state);
}

ROFL_END_DECLS

#endif //_C_TYPES_PKTCLASSIFIER_H_
