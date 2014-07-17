/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PKTCLASSIFIER_H_
#define _PKTCLASSIFIER_H_

#include <rofl.h>

#if 0
	#include "c_pktclassifier/c_pktclassifier.h"
#else
	#include "c_types_pktclassifier/c_types_pktclassifier.h"
#endif

//fwd decl
struct classifier_state;

ROFL_BEGIN_DECLS

//push & pop
void pop_pbb(datapacket_t* pkt, struct classifier_state* clas_state);
void pop_vlan(datapacket_t* pkt, struct classifier_state* clas_state);
void pop_pbb(datapacket_t* pkt, struct classifier_state* clas_state);
void pop_mpls(datapacket_t* pkt, struct classifier_state* clas_state, uint16_t ether_type);
void pop_pppoe(datapacket_t* pkt, struct classifier_state* clas_state, uint16_t ether_type);
void pop_gtp(datapacket_t* pkt, struct classifier_state* clas_state, uint16_t ether_type);

void* push_pbb(datapacket_t* pkt, struct classifier_state* clas_state, uint16_t ether_type);
void* push_vlan(datapacket_t* pkt, struct classifier_state* clas_state, uint16_t ether_type);
void* push_pbb(datapacket_t* pkt, struct classifier_state* clas_state, uint16_t ether_type);
void* push_mpls(datapacket_t* pkt, struct classifier_state* clas_state, uint16_t ether_type);
void* push_pppoe(datapacket_t* pkt, struct classifier_state* clas_state, uint16_t ether_type);
void* push_gtp(datapacket_t* pkt, struct classifier_state* clas_state, uint16_t ether_type);

void dump_pkt_classifier(struct classifier_state* clas_state);
size_t get_pkt_len(datapacket_t* pkt, struct classifier_state* clas_state, void *from, void *to);

ROFL_END_DECLS

#endif //_PKTCLASSIFIER_H_
