#ifndef _CPP_ARPV4_H_
#define _CPP_ARPV4_H_

#include <stdint.h>

uint16_t get_ar_hrd(void *hdr);

void set_ar_hdr(void *hdr, uint16_t ar_hdr);

uint16_t get_ar_pro(void *hdr);

void set_ar_pro(void *hdr, uint16_t ar_pro);

uint8_t get_ar_hln(void *hdr);

void set_ar_hln(void *hdr, uint8_t ar_hln);

uint8_t get_ar_pln(void *hdr);

void set_ar_pln(void *hdr, uint8_t ar_pln);

uint16_t get_ar_op(void *hdr);

void set_ar_op(void *hdr, uint16_t ar_op);

uint64_t get_dl_arpv4_dst(void *hdr);

void set_dl_arpv4_dst(void* hdr, uint64_t dl_dst);

uint64_t get_dl_arpv4_src(void* hdr);

void set_dl_arpv4_src(void* hdr, uint64_t dl_src);

uint32_t get_ip_arpv4_src(void *hdr);

void set_ip_arpv4_src(void *hdr, uint16_t ip_src);

uint16_t get_ip_arpv4_dst(void *hdr);

void set_ip_arpv4_dst(void *hdr, uint16_t ip_dst);

#endif //_CPP_ARPV4_H_
