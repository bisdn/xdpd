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

// MPLS header
typedef struct cpc_mpls_hdr {
	uint8_t label[3];
	uint8_t ttl;
} __attribute__((packed)) cpc_mpls_hdr_t;

inline static
void set_mpls_label(void *hdr, uint32_t label){
	uint32_t *ptr = (uint32_t*) &((cpc_mpls_hdr_t*)hdr)->label[0];
	*ptr = ((*ptr) & ~OF1X_20_BITS_MASK) | (label & OF1X_20_BITS_MASK);
}

inline static
uint32_t get_mpls_label(void *hdr){
	uint32_t *label_ptr = (uint32_t*) &(((cpc_mpls_hdr_t*)hdr)->label[0]) ;
	return (*label_ptr) & OF1X_20_BITS_MASK;
}

inline static
void set_mpls_tc(void *hdr, uint8_t tc){
	((cpc_mpls_hdr_t*)hdr)->label[2] = (tc & OF1X_BITS_12AND3_MASK)  | (((cpc_mpls_hdr_t*)hdr)->label[2] & ~OF1X_BITS_12AND3_MASK);
}

inline static
uint8_t get_mpls_tc(void *hdr){
	return ((cpc_mpls_hdr_t*)hdr)->label[2] & OF1X_BITS_12AND3_MASK;
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
		((cpc_mpls_hdr_t*)hdr)->label[2] |= OF1X_BIT0_MASK;
	else
		((cpc_mpls_hdr_t*)hdr)->label[2] &= ~OF1X_BIT0_MASK;
}

inline static
bool get_mpls_bos(void *hdr){
	return ((((cpc_mpls_hdr_t*)hdr)->label[2] & OF1X_BIT0_MASK) ? true : false);
}

#endif //_CPC_MPLS_H_
