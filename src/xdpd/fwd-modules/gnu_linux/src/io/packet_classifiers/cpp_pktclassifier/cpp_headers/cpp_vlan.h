/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_VLAN_H_
#define _CPP_VLAN_H_

#include <stdint.h>

/**
* @file cpp_vlan.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Wrapper for the c++ classifier
*/

void set_vlan_id(void* hdr, uint16_t vid);

uint16_t get_vlan_id(void* hdr);

void set_vlan_pcp(void* hdr, uint8_t pcp);

uint16_t get_vlan_pcp(void* hdr);

void set_vlan_cfi(void* hdr, bool cfi);

bool get_vlan_cfi(void* hdr);

void set_vlan_type(void* hdr, uint16_t dl_type);

uint16_t get_vlan_type(void* hdr);


#endif //_CPP_VLAN_H_
