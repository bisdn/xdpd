#ifndef _CPP_ETERNET_H_
#define _CPP_ETERNET_H_

#include <stdint.h>

uint64_t get_dl_eth_dst(void *hdr);

void set_dl_eth_dst(void* hdr, uint64_t dl_dst);

uint64_t get_dl_eth_src(void* hdr);

void set_dl_eth_src(void* hdr, uint64_t dl_src);

uint16_t get_dl_eth_type(void* hdr);

void set_dl_eth_type(void* hdr, uint16_t dl_type);

#endif //_CPP_ETERNET_H_
