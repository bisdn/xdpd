/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_GTPU_H_
#define _CPP_GTPU_H_

#include <stdint.h>

uint8_t get_gtp_version(void *hdr);

void set_gtp_version(void *hdr, uint8_t version);

bool get_pt_flag(void *hdr);

void set_pt_flag(void *hdr, bool pt);

bool get_e_flag(void *hdr);

void set_e_flag(void *hdr, bool e);

bool get_s_flag(void *hdr);

void set_s_flag(void *hdr, bool s);

bool get_pn_flag(void *hdr);

void set_pn_flag(void *hdr, bool pn);

uint8_t get_msg_type(void *hdr);

void set_msg_type(void *hdr, uint8_t msgtype);

uint16_t get_gtp_length(void *hdr);

void set_gtp_length(void *hdr, uint16_t length);

uint32_t get_teid(void *hdr);

void set_teid(void *hdr, uint32_t teid);

uint16_t get_seq_no(void *hdr);

void set_seq_no(void *hdr, uint16_t seqno);

uint8_t get_npdu_no(void *hdr);

void set_npdu_no(void *hdr, uint8_t npdu);

uint8_t get_ext_type(void *hdr);

void set_ext_type(void *hdr, uint8_t exthdr);

#endif //_CPP_GTPU_H_
