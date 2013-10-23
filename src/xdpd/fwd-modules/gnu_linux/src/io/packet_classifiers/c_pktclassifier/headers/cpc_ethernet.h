//Here: struct definitions, getters, setters (implemented) and declaration of pudh & pop

#ifndef _CPC_ETERNET_H_
#define _CPC_ETERNET_H_

#include <rofl/common/endian_conversion.h>

#define DEFAULT_ETHER_FRAME_SIZE 1518
#define CPC_ETH_ALEN 6

static inline
uint64_t mac_addr_to_u64(uint8_t *mac){
	uint32_t *p32 = (uint32_t*)mac;
	uint16_t *p16 = (uint16_t*)(&mac[4]);
	return (uint64_t)*p32 + (((uint64_t)*p16)<<32);
};

static inline
void u64_to_mac_ptr(uint8_t *mac, uint64_t val){
	uint32_t *p32 = (uint32_t*) mac;
	uint16_t *p16 = (uint16_t*) (&mac[4]);
	
	*p32 = (uint32_t)(val&0x00000000ffffffff);
	*p16 = (uint16_t)((val&0x0000ffff00000000)>>32);
};

/* Ethernet constants and definitions */

// Ethernet II header
struct cpc_eth_hdr_t {
	uint8_t dl_dst[CPC_ETH_ALEN];
	uint8_t dl_src[CPC_ETH_ALEN];
	uint16_t dl_type;
	uint8_t data[0];
} __attribute__((packed));

inline static
uint8_t* get_dl_dst(void *hdr){
	return mac_addr_to_u64(((cpc_eth_hdr_t*)hdr)->dl_dst);
};

inline static
void set_dl_dst(void* hdr, uint64_t dl_dst){
	u64_to_mac_ptr( ((cpc_eth_hdr_t*)hdr)->dl_dst, dl_dst);
	//TODO is the mac also swapped to host byte order?
};

inline static
uint64_t get_dl_src(void* hdr){
	return mac_addr_to_u64(((cpc_eth_hdr_t*)hdr)->dl_src);
};

inline static
void set_dl_src(void* hdr, uint8_t *dl_src){
	u64_to_mac_ptr( ((cpc_eth_hdr_t*)hdr)->dl_src, dl_src);
	//TODO is the mac also swapped to host byte order?
};

inline static
uint16_t get_dl_type(void* hdr){
	return be16toh(((cpc_eth_hdr_t *)hdr)->dl_type);
};

inline static
void set_dl_type(void* hdr, uint16_t dl_type){
	((cpc_eth_hdr_t *)hdr)->dl_type = htobe16(dl_type);
};

#endif //_CPC_ETERNET_H_
