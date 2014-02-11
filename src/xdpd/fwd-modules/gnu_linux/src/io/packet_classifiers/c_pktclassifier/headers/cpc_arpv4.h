/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPC_ARPV4_H_
#define _CPC_ARPV4_H_

#include "../cpc_utils.h"
#include "cpc_ethernet.h"

/**
* @file cpc_arpv4.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Structure definitions and inline getters and setters for ARPv4
*/

/* ARPv4 constants and definitions */
enum arpv4_ether_t {
	ARPV4_ETHER = 0x0806,
};

typedef struct cpc_arpv4_hdr {
	uint16_t htype;            	// hardware address format
	uint16_t ptype;            	// protocol address format
	uint8_t hlen;             	// hardware address length
	uint8_t plen;             	// protocol address length
	uint16_t opcode;             	// ARP opcode
	uint8_t dl_src[CPC_ETH_ALEN]; 	// source MAC address
	uint32_t ip_src;            	// source IP address
	uint8_t dl_dst[CPC_ETH_ALEN]; 	// destination MAC address
	uint32_t ip_dst;            	// destination IP address
} __attribute__((packed)) cpc_arpv4_hdr_t;

/* ARPv4 definitions */
inline static
uint16_t get_arpv4_htype(void *hdr){
	return CPC_BE16TOH(((cpc_arpv4_hdr_t *)hdr)->htype);
};

inline static
void set_arpv4_htype(void *hdr, uint16_t htype){
	((cpc_arpv4_hdr_t*)hdr)->htype = CPC_HTOBE16(htype);
};

inline static
uint16_t get_arpv4_ptype(void *hdr){
	return CPC_BE16TOH(((cpc_arpv4_hdr_t *)hdr)->ptype);
};

inline static
void set_arpv4_ptype(void *hdr, uint16_t ptype){
	((cpc_arpv4_hdr_t*)hdr)->ptype = CPC_HTOBE16(ptype);
};

inline static
uint8_t get_arpv4_hlen(void *hdr){
	return ((cpc_arpv4_hdr_t *)hdr)->hlen;
};

inline static
void set_arpv4_hlen(void *hdr, uint8_t hlen){
	((cpc_arpv4_hdr_t*)hdr)->hlen = hlen;
};

inline static
uint8_t get_arpv4_plen(void *hdr){
	return ((cpc_arpv4_hdr_t *)hdr)->plen;
};

inline static
void set_arpv4_plen(void *hdr, uint8_t plen){
	((cpc_arpv4_hdr_t*)hdr)->plen = plen;
};

inline static
uint16_t get_arpv4_opcode(void *hdr){
	return CPC_BE16TOH(((cpc_arpv4_hdr_t *)hdr)->opcode);
};

inline static
void set_arpv4_opcode(void *hdr, uint16_t opcode){
	((cpc_arpv4_hdr_t*)hdr)->opcode = CPC_HTOBE16(opcode);
};

inline static
uint64_t get_arpv4_dl_dst(void *hdr){
	uint64_t ret = mac_addr_to_u64(((cpc_arpv4_hdr_t*)hdr)->dl_dst);
	CPC_SWAP_MAC(ret);
	return ret;
};

inline static
void set_arpv4_dl_dst(void* hdr, uint64_t dl_dst){
	CPC_SWAP_MAC(dl_dst);
	u64_to_mac_ptr(((cpc_arpv4_hdr_t*)hdr)->dl_dst, dl_dst);
};

inline static
uint64_t get_arpv4_dl_src(void* hdr){
	uint64_t ret = mac_addr_to_u64(((cpc_arpv4_hdr_t*)hdr)->dl_src);
	CPC_SWAP_MAC(ret);
	return ret;
};

inline static
void set_arpv4_dl_src(void* hdr, uint64_t dl_src){
	CPC_SWAP_MAC(dl_src);
	u64_to_mac_ptr(((cpc_arpv4_hdr_t*)hdr)->dl_src, dl_src);
};

inline static
uint32_t get_arpv4_ip_src(void *hdr){
	return CPC_BE32TOH(((cpc_arpv4_hdr_t *)hdr)->ip_src);
};

inline static
void set_arpv4_ip_src(void *hdr, uint16_t ip_src){
	((cpc_arpv4_hdr_t*)hdr)->ip_src = CPC_HTOBE32(ip_src);
};

inline static
uint32_t get_arpv4_ip_dst(void *hdr){
	return CPC_BE32TOH(((cpc_arpv4_hdr_t *)hdr)->ip_dst);
};

inline static
void set_arpv4_ip_dst(void *hdr, uint16_t ip_dst){
	((cpc_arpv4_hdr_t*)hdr)->ip_dst = CPC_HTOBE32(ip_dst);
};
#endif //_CPC_ARPV4_H_
