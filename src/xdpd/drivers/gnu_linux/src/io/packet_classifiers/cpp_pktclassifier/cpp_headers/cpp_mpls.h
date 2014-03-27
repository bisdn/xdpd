/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_MPLS_H_
#define _CPP_MPLS_H_

#include <stdint.h>

/**
* @file cpp_mpls.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Wrapper for the c++ classifier
*/

void set_mpls_label(void *hdr, uint32_t label);

uint32_t get_mpls_label(void *hdr);

void set_mpls_tc(void *hdr, uint8_t tc);

uint8_t get_mpls_tc(void *hdr);

void dec_mpls_ttl(void *hdr);

void set_mpls_ttl(void *hdr, uint8_t ttl);

uint8_t get_mpls_ttl(void *hdr);

void set_mpls_bos(void *hdr, bool flag);

bool get_mpls_bos(void *hdr);

#endif //_CPP_MPLS_H_
