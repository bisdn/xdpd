/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_ETERNET_H_
#define _CPC_ETERNET_H_

#include <string.h>
#include "../cpc_utils.h"

/**
* @file cpc_ethernet.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for Ethernet
*/

#define DEFAULT_ETHER_FRAME_SIZE 1518
#define CPC_ETH_ALEN 6

static inline
uint64_t mac_addr_to_u64(uint8_t *mac){
	
	uint64_t *p64 = (uint64_t*)mac;
	return (*p64 & 0x0000ffffffffffff);
	
};

static inline
void u64_to_mac_ptr(uint8_t *mac, uint64_t val){
	
	memcpy(mac,&val,6);
	
};

/* Ethernet constants and definitions */

// Ethernet II header
struct cpc_eth_hdr {
	uint8_t dl_dst[CPC_ETH_ALEN];
	uint8_t dl_src[CPC_ETH_ALEN];
	uint16_t dl_type;
	uint8_t data[0];
}__attribute__((packed));

typedef struct cpc_eth_hdr cpc_eth_hdr_t;

inline static
uint64_t get_ether_dl_dst(void *hdr){
	uint64_t ret = mac_addr_to_u64(((cpc_eth_hdr_t*)hdr)->dl_dst);
	CPC_SWAP_MAC(ret);
	return ret;
};

inline static
void set_ether_dl_dst(void* hdr, uint64_t dl_dst){
	CPC_SWAP_MAC(dl_dst);
	u64_to_mac_ptr( ((cpc_eth_hdr_t*)hdr)->dl_dst, dl_dst);
};

inline static
uint64_t get_ether_dl_src(void* hdr){
	uint64_t ret = mac_addr_to_u64(((cpc_eth_hdr_t*)hdr)->dl_src);
	CPC_SWAP_MAC(ret);
	return ret;
};

inline static
void set_ether_dl_src(void* hdr, uint64_t dl_src){
	CPC_SWAP_MAC(dl_src);
	u64_to_mac_ptr( ((cpc_eth_hdr_t*)hdr)->dl_src, dl_src);
};

inline static
uint16_t get_ether_type(void* hdr){
	return CPC_BE16TOH(((cpc_eth_hdr_t *)hdr)->dl_type);
};

inline static
void set_ether_type(void* hdr, uint16_t dl_type){
	((cpc_eth_hdr_t *)hdr)->dl_type = CPC_HTOBE16(dl_type);
};

#endif //_CPC_ETERNET_H_
