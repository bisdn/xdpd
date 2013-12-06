/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_PPP_H_
#define _CPP_PPP_H_

#include <stdint.h>

/**
* @file cpp_ppp.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Wrapper for the c++ classifier
*/

uint16_t get_ppp_prot(void *hdr);

void set_ppp_prot(void *hdr, uint16_t prot);

uint8_t get_ppp_lcp_code(void *hdr);

void set_ppp_lcp_code(void *hdr, uint8_t code);

uint8_t get_ppp_lcp_ident(void *hdr);

void set_ppp_lcp_ident(void *hdr, uint8_t ident);


uint16_t get_ppp_lcp_length(void *hdr);


void set_ppp_lcp_length(void *hdr, uint16_t len);

//TODO cpc_ppp_lcp_option_t* get_lcp_option(void *hdr, enum ppp_lcp_option_t option);

uint8_t get_ppp_ipcp_code(void *hdr);

void set_ppp_ipcp_code(void *hdr, uint8_t code);

uint8_t get_ppp_ipcp_ident(void *hdr);

void set_ppp_ipcp_ident(void *hdr, uint8_t ident);

uint16_t get_ppp_ipcp_length(void *hdr);

void set_ppp_ipcp_length(void *hdr, uint16_t len);

#if 0
TODO
fppp_ipcp_option* get_ipcp_option(void *hdr, enum ppp_ipcp_option_t option)
{	//if (ipcp_options.find(option) == ipcp_options.end()) throw ePPPIpcpOptionNotFound();

	return ipcp_options[option];}
#endif

#endif //_CPP_PPP_H_
