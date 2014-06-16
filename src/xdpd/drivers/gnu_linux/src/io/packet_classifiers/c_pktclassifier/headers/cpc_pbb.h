/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_PBB_H_
#define _CPC_PBB_H_

#include <rofl/datapath/pipeline/common/protocol_constants.h>

/**
* @file cpc_pbb.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for Provider Backbone Bridges (IEEE 802.1ah)
*/

#define DEFAULT_ETHER_FRAME_SIZE 1518
#define PBB_ETH_ALEN 6

/* Ethernet constants and definitions */

// Provider Backbone Bridges (IEEE 802.1ah) header
// I-TAG => chapter 9.7
struct cpc_pbb_hdr {
	uint16_t b_vid;
	uint16_t i_dl_type;
	uint8_t i_tag[4];
	uint8_t dl_dst[PBB_ETH_ALEN];
	uint8_t dl_src[PBB_ETH_ALEN];
	uint16_t dl_type;
	uint8_t data[0];
}__attribute__((packed));

typedef struct cpc_pbb_hdr cpc_pbb_hdr_t;


inline static
uint16_t* get_b_vid(void* hdr){
	return &((cpc_pbb_hdr_t *)hdr)->b_vid;
};

inline static
void set_b_vid(void* hdr, uint16_t b_vid){
	((cpc_pbb_hdr_t *)hdr)->b_vid = b_vid;
};

inline static
uint16_t* get_i_dl_type(void* hdr){
	return &((cpc_pbb_hdr_t *)hdr)->i_dl_type;
};

inline static
void set_i_dl_type(void* hdr, uint16_t i_dl_type){
	((cpc_pbb_hdr_t *)hdr)->i_dl_type = i_dl_type;
};

inline static
uint32_t* get_i_tag(void* hdr){
	return (uint32_t*) &(((cpc_pbb_hdr_t*)hdr)->i_tag[1]) ;
};

inline static
void set_i_tag(void* hdr, uint32_t i_tag){
	uint32_t *ptr = (uint32_t*) &((cpc_pbb_hdr_t*)hdr)->i_tag[1];
	*ptr = ((*ptr) & ~OF1X_24_BITS_MASK) | (i_tag & OF1X_24_BITS_MASK);
};


inline static
uint64_t* get_ether_dl_dst(void *hdr){
	return (uint64_t*) &((cpc_pbb_hdr_t*)hdr)->dl_dst;
};

inline static
void set_ether_dl_dst(void* hdr, uint64_t dl_dst){
	uint64_t *ptr = (uint64_t *) &((cpc_pbb_hdr_t*)hdr)->dl_dst;
	*ptr = (p_da & OF1X_6_BYTE_MASK) | (*ptr & ~OF1X_6_BYTE_MASK);
};

inline static
uint64_t* get_ether_dl_src(void* hdr){
	return (uint64_t*) &((cpc_pbb_hdr_t*)hdr)->dl_src;
};

inline static
void set_ether_dl_src(void* hdr, uint64_t dl_src){
	uint64_t *ptr = (uint64_t *) &((cpc_pbb_hdr_t*)hdr)->dl_src;
	*ptr = (dl_src & OF1X_6_BYTE_MASK) | (*ptr & ~OF1X_6_BYTE_MASK);
};

inline static
uint16_t* get_ether_type(void* hdr){
	return &((cpc_pbb_hdr_t *)hdr)->dl_type;
};

inline static
void set_ether_type(void* hdr, uint16_t dl_type){
	((cpc_pbb_hdr_t *)hdr)->dl_type = dl_type;
};

#endif //_CPC_PBB_H_
