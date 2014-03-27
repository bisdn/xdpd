/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_PPPOE_H_
#define _CPP_PPPOE_H_

#include <stdint.h>

/**
* @file cpp_pppoe.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Wrapper for the c++ classifier
*/

uint8_t get_pppoe_vers(void *hdr);

void set_pppoe_vers(void *hdr, uint8_t version);

uint8_t get_pppoe_type(void *hdr);

void set_pppoe_type(void *hdr, uint8_t type);

uint8_t get_pppoe_code(void *hdr);

void set_pppoe_code(void *hdr, uint8_t code);

uint16_t get_pppoe_sessid(void *hdr);

void set_pppoe_sessid(void *hdr, uint16_t sessid);

uint16_t get_pppoe_length(void *hdr);

void set_pppoe_length(void *hdr, uint16_t length);

#endif //_CPP_PPPOE_H_
