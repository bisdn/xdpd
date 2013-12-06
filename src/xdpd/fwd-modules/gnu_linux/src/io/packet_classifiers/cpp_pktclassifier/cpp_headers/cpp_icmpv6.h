/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_ICMPV6_H_
#define _CPP_ICMPV6_H_

#include <stdint.h>
#include <rofl/datapath/pipeline/common/large_types.h>

/**
* @file cpp_icmpv6.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Wrapper for the c++ classifier
*/

void icmpv6_calc_checksum(void *hdr, uint16_t length);

void *get_icmpv6_option(void *hdr, uint8_t type);

uint8_t get_icmpv6_code(void *hdr);

void set_icmpv6_code(void *hdr, uint8_t code);

uint8_t get_icmpv6_type(void *hdr);

void set_icmpv6_type(void *hdr, uint8_t type);

uint128__t get_icmpv6_neighbor_taddr(void *hdr);

void set_icmpv6_neighbor_taddr(void *hdr, uint128__t taddr);

uint8_t get_icmpv6_opt_type(void *hdr);

void set_icmpv6_opt_type(void *hdr, uint8_t type);

uint64_t get_icmpv6_ll_taddr(void *hdr);

void set_icmpv6_ll_taddr(void *hdr, uint64_t taddr);

uint64_t get_icmpv6_ll_saddr(void *hdr);

void set_icmpv6_ll_saddr(void *hdr, uint64_t saddr);

uint8_t get_icmpv6_pfx_on_link_flag(void *hdr);

void set_icmpv6_pfx_on_link_flag(void *hdr, uint8_t flag);

uint8_t get_icmpv6_pfx_aac_flag(void *hdr);

void set_icmpv6_pfx_aac_flag(void *hdr, uint8_t flag);

#endif //_CPP_ICMPV6_H_
