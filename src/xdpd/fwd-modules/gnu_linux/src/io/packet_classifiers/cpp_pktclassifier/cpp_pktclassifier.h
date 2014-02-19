/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _CPP_PKTCLASSIFIER_H_
#define _CPP_PKTCLASSIFIER_H_

#include <stddef.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include "../pktclassifier.h"
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

ROFL_END_DECLS

#endif //_CPP_PKTCLASSIFIER_H_
