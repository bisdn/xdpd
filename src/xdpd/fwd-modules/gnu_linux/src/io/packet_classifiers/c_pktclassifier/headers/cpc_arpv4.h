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
	uint16_t ar_hrd;            	// hardware address format
	uint16_t ar_pro;            	// protocol address format
	uint8_t ar_hln;             	// hardware address length
	uint8_t ar_pln;             	// protocol address length
	uint16_t ar_op;             	// ARP opcode
	uint8_t dl_src[CPC_ETH_ALEN]; 	// source MAC address
	uint32_t ip_src;            	// source IP address
	uint8_t dl_dst[CPC_ETH_ALEN]; 	// destination MAC address
	uint32_t ip_dst;            	// destination IP address
} __attribute__((packed)) cpc_arpv4_hdr_t;

/* ARPv4 definitions */
inline static
uint16_t get_ar_hrd(void *hdr){
	return CPC_BE16TOH(((cpc_arpv4_hdr_t *)hdr)->ar_hrd);
};

inline static
void set_ar_hdr(void *hdr, uint16_t ar_hdr){
	((cpc_arpv4_hdr_t*)hdr)->ar_hrd = CPC_HTOBE16(ar_hdr);
};

inline static
uint16_t get_ar_pro(void *hdr){
	return CPC_BE16TOH(((cpc_arpv4_hdr_t *)hdr)->ar_pro);
};

inline static
void set_ar_pro(void *hdr, uint16_t ar_pro){
	((cpc_arpv4_hdr_t*)hdr)->ar_pro = CPC_HTOBE16(ar_pro);
};

inline static
uint8_t get_ar_hln(void *hdr){
	return ((cpc_arpv4_hdr_t *)hdr)->ar_hln;
};

inline static
void set_ar_hln(void *hdr, uint8_t ar_hln){
	((cpc_arpv4_hdr_t*)hdr)->ar_hln = ar_hln;
};

inline static
uint8_t get_ar_pln(void *hdr){
	return ((cpc_arpv4_hdr_t *)hdr)->ar_pln;
};

inline static
void set_ar_pln(void *hdr, uint8_t ar_pln){
	((cpc_arpv4_hdr_t*)hdr)->ar_pln = ar_pln;
};

inline static
uint16_t get_ar_op(void *hdr){
	return CPC_BE16TOH(((cpc_arpv4_hdr_t *)hdr)->ar_op);
};

inline static
void set_ar_op(void *hdr, uint16_t ar_op){
	((cpc_arpv4_hdr_t*)hdr)->ar_op = CPC_HTOBE16(ar_op);
};

inline static
uint64_t get_dl_arpv4_dst(void *hdr){
	uint64_t ret = mac_addr_to_u64(((cpc_arpv4_hdr_t*)hdr)->dl_dst);
	CPC_SWAP_MAC(ret);
	return ret;
};

inline static
void set_dl_arpv4_dst(void* hdr, uint64_t dl_dst){
	CPC_SWAP_MAC(dl_dst);
	u64_to_mac_ptr(((cpc_arpv4_hdr_t*)hdr)->dl_dst, dl_dst);
};

inline static
uint64_t get_dl_arpv4_src(void* hdr){
	uint64_t ret = mac_addr_to_u64(((cpc_arpv4_hdr_t*)hdr)->dl_src);
	CPC_SWAP_MAC(ret);
	return ret;
};

inline static
void set_dl_arpv4_src(void* hdr, uint64_t dl_src){
	CPC_SWAP_MAC(dl_src);
	u64_to_mac_ptr(((cpc_arpv4_hdr_t*)hdr)->dl_src, dl_src);
};

inline static
uint32_t get_ip_arpv4_src(void *hdr){
	return be32toh(((cpc_arpv4_hdr_t *)hdr)->ip_src);
};

inline static
void set_ip_arpv4_src(void *hdr, uint16_t ip_src){
	((cpc_arpv4_hdr_t*)hdr)->ip_src = CPC_HTOBE16(ip_src);
};

inline static
uint16_t get_ip_arpv4_dst(void *hdr){
	return CPC_BE16TOH(((cpc_arpv4_hdr_t *)hdr)->ip_dst);
};

inline static
void set_ip_arpv4_dst(void *hdr, uint16_t ip_dst){
	((cpc_arpv4_hdr_t*)hdr)->ip_dst = CPC_HTOBE16(ip_dst);
};
#endif //_CPC_ARPV4_H_
