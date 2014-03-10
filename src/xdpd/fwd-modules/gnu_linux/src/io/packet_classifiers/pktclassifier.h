/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PKTCLASSIFIER_H_
#define _PKTCLASSIFIER_H_

ROFL_BEGIN_DECLS

struct classify_state* init_classifier(datapacket_t*const pkt);
void destroy_classifier(struct classify_state* clas_state);
void classify_packet(struct classify_state* clas_state, uint8_t* pkt, size_t len, uint32_t port_in, uint32_t phy_port_in);
void reset_classifier(struct classify_state* clas_state);

//push & pop
void pop_vlan(datapacket_t* pkt, struct classify_state* clas_state);
void pop_mpls(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void pop_pppoe(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void pop_gtp(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);

void* push_vlan(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void* push_mpls(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void* push_pppoe(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void* push_gtp(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);

void dump_pkt_classifier(struct classify_state* clas_state);
size_t get_pkt_len(datapacket_t* pkt, struct classify_state* clas_state, void *from, void *to);

ROFL_END_DECLS

#endif //_PKTCLASSIFIER_H_
