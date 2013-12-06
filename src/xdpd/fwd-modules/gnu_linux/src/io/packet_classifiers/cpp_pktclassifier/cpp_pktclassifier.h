/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_PKTCLASSIFIER_H_
#define _CPP_PKTCLASSIFIER_H_

#include <stddef.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "packetclassifier.h"

#include "./cpp_headers/cpp_ethernet.h"
#include "./cpp_headers/cpp_arpv4.h"
#include "./cpp_headers/cpp_gtpu.h"
#include "./cpp_headers/cpp_icmpv4.h"
#include "./cpp_headers/cpp_icmpv6.h"
#include "./cpp_headers/cpp_ipv4.h"
#include "./cpp_headers/cpp_ipv6.h"
#include "./cpp_headers/cpp_mpls.h"
#include "./cpp_headers/cpp_ppp.h"
#include "./cpp_headers/cpp_pppoe.h"
#include "./cpp_headers/cpp_tcp.h"
#include "./cpp_headers/cpp_udp.h"
#include "./cpp_headers/cpp_vlan.h"

/**
* @file cpp_pktclassifier.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Interface for the c++ classifiers
*/

struct classify_state;

ROFL_BEGIN_DECLS

//function declarations
struct classify_state* init_classifier(void);
void destroy_classifier(struct classify_state* clas_state);
void classify_packet(struct classify_state* clas_state, uint8_t* pkt, size_t len);
void reset_classifier(struct classify_state* clas_state);

void* get_ether_hdr(struct classify_state* clas_state, int idx);
void* get_vlan_hdr(struct classify_state* clas_state, int idx);
void* get_mpls_hdr(struct classify_state* clas_state, int idx);
void* get_arpv4_hdr(struct classify_state* clas_state, int idx);
void* get_ipv4_hdr(struct classify_state* clas_state, int idx);
void* get_icmpv4_hdr(struct classify_state* clas_state, int idx);
void* get_ipv6_hdr(struct classify_state* clas_state, int idx);
void* get_icmpv6_hdr(struct classify_state* clas_state, int idx);
void* get_udp_hdr(struct classify_state* clas_state, int idx);
void* get_tcp_hdr(struct classify_state* clas_state, int idx);
void* get_pppoe_hdr(struct classify_state* clas_state, int idx);
void* get_ppp_hdr(struct classify_state* clas_state, int idx);
void* get_gtpu_hdr(struct classify_state* clas_state, int idx);

//push & pop
void pop_vlan(datapacket_t* pkt, struct classify_state* clas_state);
void pop_mpls(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void pop_pppoe(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void pop_gtp(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);

void* push_vlan(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void* push_mpls(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void* push_pppoe(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);
void* push_gtp(datapacket_t* pkt, struct classify_state* clas_state, uint16_t ether_type);

//void pkt_push();
//void pkt_pop();

void dump_pkt_classifier(struct classify_state* clas_state);
size_t get_pkt_len(datapacket_t* pkt, struct classify_state* clas_state, void *from, void *to);

ROFL_END_DECLS

#endif //_CPP_PKTCLASSIFIER_H_
