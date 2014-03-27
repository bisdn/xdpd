/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_ARPV4_H_
#define _CPP_ARPV4_H_

#include <stdint.h>

/**
* @file cpp_arpv4.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Wrapper for the c++ classifier
*/

uint16_t get_arpv4_htype(void *hdr);

void set_arpv4_htype(void *hdr, uint16_t htype);

uint16_t get_arpv4_ptype(void *hdr);

void set_arpv4_ptype(void *hdr, uint16_t ptype);

uint8_t get_arpv4_hlen(void *hdr);

void set_arpv4_hlen(void *hdr, uint8_t hlen);

uint8_t get_arpv4_plen(void *hdr);

void set_arpv4_plen(void *hdr, uint8_t plen);

uint16_t get_arpv4_opcode(void *hdr);

void set_arpv4_opcode(void *hdr, uint16_t opcode);

uint64_t get_arpv4_dl_dst(void *hdr);

void set_arpv4_dl_dst(void* hdr, uint64_t dl_dst);

uint64_t get_arpv4_dl_src(void* hdr);

void set_arpv4_dl_src(void* hdr, uint64_t dl_src);

uint32_t get_arpv4_ip_src(void *hdr);

void set_arpv4_ip_src(void *hdr, uint16_t ip_src);

uint16_t get_arpv4_ip_dst(void *hdr);

void set_arpv4_ip_dst(void *hdr, uint16_t ip_dst);

#endif //_CPP_ARPV4_H_
