/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_VLAN_H_
#define _CPC_VLAN_H_

#include "../cpc_utils.h"

/**
* @file cpc_vlan.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for VLAN
*/

/* Ethernet constants and definitions */
// VLAN ethernet types
enum vlan_ether_t {
	VLAN_CTAG_ETHER = 0x8100,
	VLAN_STAG_ETHER = 0x88a8,
	VLAN_ITAG_ETHER = 0x88e7,
};

// VLAN header
typedef struct cpc_vlan_hdr {
	// tag control identifier (TCI)
	uint8_t byte0;
	uint8_t byte1;
	//uint16_t hdr;			// vid + cfi + pcp
	uint16_t dl_type;  		// ethernet type
} __attribute__((packed)) cpc_vlan_hdr_t;

inline static
void set_vlan_id(void* hdr, uint16_t vid){
	((cpc_vlan_hdr_t*)hdr)->byte1 = vid & 0x00ff;
	((cpc_vlan_hdr_t*)hdr)->byte0 = (((cpc_vlan_hdr_t*)hdr)->byte0 & 0xf0) + ((vid & 0x0f00) >> 8);
}

inline static
uint16_t get_vlan_id(void* hdr){
	return (((((cpc_vlan_hdr_t*)hdr)->byte0 & 0x0f) << 8) + ((cpc_vlan_hdr_t*)hdr)->byte1);
}

inline static
void set_vlan_pcp(void* hdr, uint8_t pcp){
	((cpc_vlan_hdr_t*)hdr)->byte0 = ((pcp & 0x07) << 5) + (((cpc_vlan_hdr_t*)hdr)->byte0 & 0x1f);
}

inline static
uint16_t get_vlan_pcp(void* hdr){
	return (((cpc_vlan_hdr_t*)hdr)->byte0 & 0xe0) >> 5;
}

inline static
void set_vlan_cfi(void* hdr, bool cfi){
	((cpc_vlan_hdr_t*)hdr)->byte0 &= 0xef;
	if (cfi)
	{
	    ((cpc_vlan_hdr_t*)hdr)->byte0 |= (1 << 4);
	}
	else
	{
	    ((cpc_vlan_hdr_t*)hdr)->byte0 &= ~(1 << 4);
	}
}

inline static
bool get_vlan_cfi(void* hdr){
	return (((cpc_vlan_hdr_t*)hdr)->byte0 & 0x10) >> 4;
}

inline static
void set_vlan_type(void* hdr, uint16_t dl_type){
	((cpc_vlan_hdr_t*)hdr)->dl_type = CPC_HTOBE16(dl_type);
}

inline static
uint16_t get_vlan_type(void* hdr){
	return CPC_BE16TOH(((cpc_vlan_hdr_t*)hdr)->dl_type);
}


#endif //_CPC_VLAN_H_
