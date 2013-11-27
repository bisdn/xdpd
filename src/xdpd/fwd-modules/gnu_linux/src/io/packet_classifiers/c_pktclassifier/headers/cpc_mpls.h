/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_MPLS_H_
#define _CPC_MPLS_H_

/**
* @file cpc_mpls.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for MPLS
*/

// VLAN ethernet types
enum mpls_ether_t {
	MPLS_ETHER = 0x8847,
	MPLS_ETHER_UPSTREAM = 0x8848,
};

// MPLS header
typedef struct cpc_mpls_hdr {
	uint8_t label[3];
	uint8_t ttl;
} __attribute__((packed)) cpc_mpls_hdr_t;

inline static
void set_mpls_label(void *hdr, uint32_t label){
	((cpc_mpls_hdr_t*)hdr)->label[0] =  (label & 0x000ff000) >> 12;
	((cpc_mpls_hdr_t*)hdr)->label[1] =  (label & 0x00000ff0) >>  4;
	((cpc_mpls_hdr_t*)hdr)->label[2] = ((label & 0x0000000f) <<  4) | (((cpc_mpls_hdr_t*)hdr)->label[2] & 0x0f);
}

inline static
uint32_t get_mpls_label(void *hdr){
	uint32_t label =
			(((cpc_mpls_hdr_t*)hdr)->label[0] << 12) +
			(((cpc_mpls_hdr_t*)hdr)->label[1] <<  4) +
			((((cpc_mpls_hdr_t*)hdr)->label[2] & 0xf0) >>  4);
	return label;
}

inline static
void set_mpls_tc(void *hdr, uint8_t tc){
	((cpc_mpls_hdr_t*)hdr)->label[2] = ((tc & 0x07) << 1) + (((cpc_mpls_hdr_t*)hdr)->label[2] & 0xf1);
}

inline static
uint8_t get_mpls_tc(void *hdr){
	return ((((cpc_mpls_hdr_t*)hdr)->label[2] & 0x0e) >> 1);
}

inline static
void dec_mpls_ttl(void *hdr){
	((cpc_mpls_hdr_t*)hdr)->ttl--;
}

inline static
void set_mpls_ttl(void *hdr, uint8_t ttl){
	((cpc_mpls_hdr_t*)hdr)->ttl = ttl;
}

inline static
uint8_t get_mpls_ttl(void *hdr){
	return ((cpc_mpls_hdr_t*)hdr)->ttl;
}

inline static
void set_mpls_bos(void *hdr, bool flag){
	if (flag)
		((cpc_mpls_hdr_t*)hdr)->label[2] |= 0x01;
	else
		((cpc_mpls_hdr_t*)hdr)->label[2] &= ~0xfe;
}

inline static
bool get_mpls_bos(void *hdr){
	return ((((cpc_mpls_hdr_t*)hdr)->label[2] & 0x01) ? true : false);
}

#endif //_CPC_MPLS_H_