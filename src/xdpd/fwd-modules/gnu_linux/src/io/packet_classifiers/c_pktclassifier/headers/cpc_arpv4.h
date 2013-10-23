#ifndef _CPC_ARPV4_H_
#define _CPC_ARPV4_H_

#include <rofl/common/endian_conversion.h>
#include "cpc_ethernet.h"
/* ARPv4 constants and definitions */
enum arpv4_ether_t {
	ARPV4_ETHER = 0x0806,
};

enum arpv4_opcodes_t {
	ARPV4_OPCODE_REQUEST = 0x01,
	ARPV4_OPCODE_REPLY = 0x02,
};

struct cpc_arpv4_hdr_t {
	uint16_t ar_hrd;            	// hardware address format
	uint16_t ar_pro;            	// protocol address format
	uint8_t ar_hln;             	// hardware address length
	uint8_t ar_pln;             	// protocol address length
	uint16_t ar_op;             	// ARP opcode
	uint8_t dl_src[CPC_ETH_ALEN]; 	// source MAC address
	uint32_t ip_src;            	// source IP address
	uint8_t dl_dst[CPC_ETH_ALEN]; 	// destination MAC address
	uint32_t ip_dst;            	// destination IP address
} __attribute__((packed));
/* ARPv4 definitions */

#define DEFAULT_ARPV4_FRAME_SIZE sizeof(struct cpc_arpv4_hdr_t)

inline static
uint16_t get_ar_hrd(void *hdr){
	return be16toh(((cpc_arpv4_hdr_t *)hdr)->ar_hrd);
};

inline static
void set_ar_hdr(void *hdr, uint16_t ar_hdr){
	((cpc_arpv4_hdr_t*)hdr)->ar_hrd = htobe16(ar_hdr);
};

inline static
uint16_t get_ar_pro(void *hdr){
	return be16toh(((cpc_arpv4_hdr_t *)hdr)->ar_pro);
};

inline static
void set_ar_pro(void *hdr, uint16_t ar_pro){
	((cpc_arpv4_hdr_t*)hdr)->ar_pro = htobe16(ar_pro);
};

inline static
uint8_t get_ar_hln(void *hdr){
	return be8toh(((cpc_arpv4_hdr_t *)hdr)->ar_hln);
};

inline static
void set_ar_hln(void *hdr, uint8_t ar_hln){
	((cpc_arpv4_hdr_t*)hdr)->ar_hln = htobe8(ar_hln);
};

inline static
uint8_t get_ar_pln(void *hdr){
	return be8toh(((cpc_arpv4_hdr_t *)hdr)->ar_pln);
};

inline static
void set_ar_pln(void *hdr, uint8_t ar_pln){
	((cpc_arpv4_hdr_t*)hdr)->ar_pln = htobe8(ar_pln);
};

inline static
uint16_t get_ar_op(void *hdr){
	return be16toh(((cpc_arpv4_hdr_t *)hdr)->ar_op);
};

inline static
void set_ar_op(void *hdr, uint16_t ar_op){
	((cpc_arpv4_hdr_t*)hdr)->ar_op = htobe16(ar_op);
};

inline static
uint64_t get_dl_dst(void *hdr){
	return mac_addr_to_u64(((cpc_arpv4_hdr_t*)hdr)->dl_dst);
};

inline static
void set_dl_dst(void* hdr, uint64_t dl_dst){
	u64_to_mac_ptr(((cpc_arpv4_hdr_t*)hdr)->dl_dst, dl_dst);
	//TODO is the mac also swapped to host byte order?
};

inline static
uint64_t get_dl_src(void* hdr){
	return mac_addr_to_u64(((cpc_arpv4_hdr_t*)hdr)->dl_src);
};

inline static
void set_dl_src(void* hdr, uint8_t *dl_src){
	u64_to_mac_ptr(((cpc_arpv4_hdr_t*)hdr)->dl_src, dl_src);
	//TODO is the mac also swapped to host byte order?
};

inline static
uint32_t get_ip_src(void *hdr){
	return be32toh(((cpc_arpv4_hdr_t *)hdr)->ip_src);
};

inline static
void set_ip_src(void *hdr, uint16_t ip_src){
	((cpc_arpv4_hdr_t*)hdr)->ip_src = htobe16(ip_src);
};

inline static
uint16_t get_ip_dst(void *hdr){
	return be16toh(((cpc_arpv4_hdr_t *)hdr)->ip_dst);
};

inline static
void set_ip_dst(void *hdr, uint16_t ip_dst){
	((cpc_arpv4_hdr_t*)hdr)->ip_dst = htobe16(ip_dst);
};
#endif //_CPC_ARPV4_H_
