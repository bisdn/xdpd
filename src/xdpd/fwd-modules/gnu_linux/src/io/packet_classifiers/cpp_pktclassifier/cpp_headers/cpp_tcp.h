/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_TCP_H_
#define _CPP_TCP_H_

#include <stdint.h>

/**
* @file cpp_tcp.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Wrapper for the c++ classifier
*/

void tcp_calc_checksum(void* hdr, uint32_t ip_src, uint32_t ip_dst, uint8_t ip_proto, uint16_t length);

uint16_t get_tcp_sport(void *hdr);

void set_tcp_sport(void *hdr, uint16_t port);

uint16_t get_tcp_dport(void *hdr);

void set_tcp_dport(void *hdr, uint16_t port);

#endif //_CPP_TCP_H_
