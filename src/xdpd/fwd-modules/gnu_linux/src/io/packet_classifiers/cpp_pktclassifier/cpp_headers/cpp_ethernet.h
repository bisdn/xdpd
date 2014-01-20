/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_ETERNET_H_
#define _CPP_ETERNET_H_

#include <stdint.h>

/**
* @file cpp_ethernet.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Wrapper for the c++ classifier
*/

uint64_t get_ether_dl_dst(void *hdr);

void set_ether_dl_dst(void* hdr, uint64_t dl_dst);

uint64_t get_ether_dl_src(void* hdr);

void set_ether_dl_src(void* hdr, uint64_t dl_src);

uint16_t get_ether_type(void* hdr);

void set_ether_type(void* hdr, uint16_t dl_type);

#endif //_CPP_ETERNET_H_
