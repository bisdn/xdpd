/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_ETERNET_H_
#define _CPC_ETERNET_H_

#include <rofl/datapath/pipeline/common/protocol_constants.h>

/**
* @file cpc_ethernet.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for Ethernet
*/

#define DEFAULT_ETHER_FRAME_SIZE 1518
#define CPC_ETH_ALEN 6

/* Ethernet constants and definitions */

// Ethernet II header
struct cpc_eth_hdr {
	uint8_t dl_dst[CPC_ETH_ALEN];
	uint8_t dl_src[CPC_ETH_ALEN];
	uint16_t dl_type;
	uint8_t data[0];
}__attribute__((packed));

typedef struct cpc_eth_hdr cpc_eth_hdr_t;

struct cpc_eth_llc_hdr {
	uint8_t dl_dst[CPC_ETH_ALEN];
	uint8_t dl_src[CPC_ETH_ALEN];
	uint16_t dl_len;
	uint8_t dl_dsap;
	uint8_t dl_ssap;
	uint8_t dl_control;
	uint8_t dl_vendor_code[3];
	uint16_t dl_type;
	uint8_t data[0];
}__attribute__((packed));

typedef struct cpc_eth_llc_hdr cpc_eth_llc_hdr_t;

inline static
uint64_t get_ether_dl_dst(void *hdr){
	uint64_t *ret = (uint64_t*) &((cpc_eth_hdr_t*)hdr)->dl_dst;
	return (*ret) & OF1X_6_BYTE_MASK;
};

inline static
void set_ether_dl_dst(void* hdr, uint64_t dl_dst){
	uint64_t *ptr = (uint64_t *) &((cpc_eth_hdr_t*)hdr)->dl_dst;
	*ptr = (dl_dst & OF1X_6_BYTE_MASK) | (*ptr & ~OF1X_6_BYTE_MASK);
};

inline static
uint64_t get_ether_dl_src(void* hdr){
	uint64_t *ret = (uint64_t*) &((cpc_eth_hdr_t*)hdr)->dl_src;
	return (*ret) & OF1X_6_BYTE_MASK;
};

inline static
void set_ether_dl_src(void* hdr, uint64_t dl_src){
	uint64_t *ptr = (uint64_t *) &((cpc_eth_hdr_t*)hdr)->dl_src;
	*ptr = (dl_src & OF1X_6_BYTE_MASK) | (*ptr & ~OF1X_6_BYTE_MASK);
};

inline static
bool is_llc_frame(void* hdr){
	return ( NTOHB16(((cpc_eth_hdr_t*)hdr)->dl_type) < LLC_DELIMITER_HBO );
}

inline static
uint16_t get_ether_type(void* hdr){
	if ( is_llc_frame(hdr) ){
		return ((cpc_eth_llc_hdr_t *)hdr)->dl_type;
	}else{
		return ((cpc_eth_hdr_t *)hdr)->dl_type;
	}
};

inline static
void set_ether_type(void* hdr, uint16_t dl_type){
	if ( is_llc_frame(hdr) ){
		((cpc_eth_llc_hdr_t *)hdr)->dl_type = dl_type;
	}else{
		((cpc_eth_hdr_t *)hdr)->dl_type = dl_type;
	}
};

#endif //_CPC_ETERNET_H_
