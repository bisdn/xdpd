/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_GTPU_H_
#define _CPP_GTPU_H_

#include <stdint.h>

/**
* @file cpp_gtpu.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Wrapper for the c++ classifier
*/

uint8_t get_gtpu_version(void *hdr);

void set_gtpu_version(void *hdr, uint8_t version);

bool get_gtpu_pt_flag(void *hdr);

void set_gtpu_pt_flag(void *hdr, bool pt);

bool get_gtpu_e_flag(void *hdr);

void set_gtpu_e_flag(void *hdr, bool e);

bool get_gtpu_s_flag(void *hdr);

void set_gtpu_s_flag(void *hdr, bool s);

bool get_gtpu_pn_flag(void *hdr);

void set_gtpu_pn_flag(void *hdr, bool pn);

uint8_t get_gtpu_msg_type(void *hdr);

void set_gtpu_msg_type(void *hdr, uint8_t msgtype);

uint16_t get_gtpu_length(void *hdr);

void set_gtpu_length(void *hdr, uint16_t length);

uint32_t get_gtpu_teid(void *hdr);

void set_gtpu_teid(void *hdr, uint32_t teid);

uint16_t get_gtpu_seq_no(void *hdr);

void set_gtpu_seq_no(void *hdr, uint16_t seqno);

uint8_t get_gtpu_npdu_no(void *hdr);

void set_gtpu_npdu_no(void *hdr, uint8_t npdu);

uint8_t get_gtpu_ext_type(void *hdr);

void set_gtpu_ext_type(void *hdr, uint8_t exthdr);

#endif //_CPP_GTPU_H_
