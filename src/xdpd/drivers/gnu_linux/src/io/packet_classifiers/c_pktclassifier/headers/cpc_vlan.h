/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_VLAN_H_
#define _CPC_VLAN_H_

/**
* @file cpc_vlan.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for VLAN
*/

/* Ethernet constants and definitions */

// VLAN header
typedef struct cpc_vlan_hdr {
	// tag control identifier (TCI)
	uint8_t byte0;		// vid (12b) + cfi (1b) + pcp (3b)
	uint8_t byte1;
	uint16_t dl_type;  		// ethernet type
} __attribute__((packed)) cpc_vlan_hdr_t;

inline static
void set_vlan_id(void* hdr, uint16_t vid){
	uint16_t *byte0 = (uint16_t*) &((cpc_vlan_hdr_t*)hdr)->byte0;
	*byte0 = ((*byte0) & ~OF1X_VLAN_ID_MASK) | (vid & OF1X_VLAN_ID_MASK);
}

inline static
uint16_t get_vlan_id(void* hdr){
	uint16_t *byte0 = (uint16_t*) &((cpc_vlan_hdr_t*)hdr)->byte0;
	return ( *byte0 & OF1X_VLAN_ID_MASK );
}

inline static
void set_vlan_pcp(void* hdr, uint8_t pcp){
	((cpc_vlan_hdr_t*)hdr)->byte0 = (pcp & OF1X_3MSBITS_MASK) | (((cpc_vlan_hdr_t*)hdr)->byte0 & ~OF1X_3MSBITS_MASK);
}

inline static
uint16_t get_vlan_pcp(void* hdr){
	return ((cpc_vlan_hdr_t*)hdr)->byte0 & OF1X_3MSBITS_MASK;
}

#if 0
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
#endif

inline static
void set_vlan_type(void* hdr, uint16_t dl_type){
	((cpc_vlan_hdr_t*)hdr)->dl_type = dl_type;
}

inline static
uint16_t get_vlan_type(void* hdr){
	return ((cpc_vlan_hdr_t*)hdr)->dl_type;
}


#endif //_CPC_VLAN_H_
