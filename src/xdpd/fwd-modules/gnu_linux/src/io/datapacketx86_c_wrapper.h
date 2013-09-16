/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DATAPACKETX86_C_WRAPPER_H_
#define DATAPACKETX86_C_WRAPPER_H_

#include <stddef.h>
#include <stdint.h>
#include <rofl.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/common/ternary_fields.h>
#include <rofl/datapath/pipeline/switch_port.h>

/**
* @file datapacketx86_c_wrapper.h
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief C wrappers for datapacketx86 operations and getters  
*
*/

//C++ extern C
ROFL_BEGIN_DECLS


/* Cloning of the packet */
void
dpx86_clone_pkt_contents(datapacket_t* src, datapacket_t* dst);

/* Copy ttl */
void
dpx86_copy_ttl_in(datapacket_t* pkt);

/* POP */
void
dpx86_pop_vlan(datapacket_t* pkt);

void
dpx86_pop_mpls(datapacket_t* pkt, uint16_t ether_type);

void
dpx86_pop_pppoe(datapacket_t* pkt, uint16_t ether_type);

void
dpx86_push_pppoe(datapacket_t* pkt, uint16_t ether_type);

void
dpx86_push_mpls(datapacket_t* pkt, uint16_t ether_type);

void
dpx86_push_vlan(datapacket_t* pkt, uint16_t ether_type);

/* Copy ttl out*/
void
dpx86_copy_ttl_out(datapacket_t* pkt);

/* Decrement ttl */
void
dpx86_dec_nw_ttl(datapacket_t* pkt);

void
dpx86_dec_mpls_ttl(datapacket_t* pkt);

/* Set field */
void
dpx86_set_mpls_ttl(datapacket_t* pkt, uint8_t new_ttl);

void
dpx86_set_nw_ttl(datapacket_t* pkt, uint8_t new_ttl);

void
dpx86_set_queue(datapacket_t* pkt, uint32_t queue);

//TODO:
//void dpx86_set_metadata(datapacket_t* pkt, uint64_t metadata);

//Ethernet
void
dpx86_set_eth_dst(datapacket_t* pkt, uint64_t eth_dst);

void
dpx86_set_eth_src(datapacket_t* pkt, uint64_t eth_src);

void
dpx86_set_eth_type(datapacket_t* pkt, uint16_t eth_type);

//802.1q
void
dpx86_set_vlan_vid(datapacket_t* pkt, uint16_t vlan_vid);

void
dpx86_set_vlan_pcp(datapacket_t* pkt, uint8_t vlan_pcp);

//ARP
void
dpx86_set_arp_opcode(datapacket_t* pkt, uint16_t arp_opcode);

void
dpx86_set_arp_sha(datapacket_t* pkt, uint64_t arp_sha);

void
dpx86_set_arp_spa(datapacket_t* pkt, uint32_t arp_spa);

void
dpx86_set_arp_tha(datapacket_t* pkt, uint64_t arp_tha);

void
dpx86_set_arp_tpa(datapacket_t* pkt, uint32_t arp_tpa);

//IP, IPv4
void
dpx86_set_ip_dscp(datapacket_t* pkt, uint8_t ip_dscp);

void
dpx86_set_ip_ecn(datapacket_t* pkt, uint8_t ip_ecn);

void
dpx86_set_ip_proto(datapacket_t* pkt, uint8_t ip_proto);

void
dpx86_set_ipv4_src(datapacket_t* pkt, uint32_t ip_src);

void
dpx86_set_ipv4_dst(datapacket_t* pkt, uint32_t ip_dst);

//IPv6
void
dpx86_set_ipv6_src(datapacket_t* pkt, uint128__t ipv6_src);

void
dpx86_set_ipv6_dst(datapacket_t* pkt, uint128__t ipv6_dst);

void
dpx86_set_ipv6_flabel(datapacket_t* pkt, uint64_t ipv6_flabel);

void
dpx86_set_ipv6_exthdr(datapacket_t* pkt, uint16_t ipv6_exthdr);

//ICMPv6
void
dpx86_set_ipv6_nd_target(datapacket_t* pkt, uint128__t ipv6_nd_target);

void
dpx86_set_ipv6_nd_sll(datapacket_t* pkt, uint64_t ipv6_nd_sll);

void
dpx86_set_ipv6_nd_tll(datapacket_t* pkt, uint64_t ipv6_nd_tll);

void
dpx86_set_icmpv6_type(datapacket_t* pkt, uint8_t icmpv6_type);

void
dpx86_set_icmpv6_code(datapacket_t* pkt, uint8_t icmpv6_code);

//TCP
void
dpx86_set_tcp_src(datapacket_t* pkt, uint16_t tcp_src);

void
dpx86_set_tcp_dst(datapacket_t* pkt, uint16_t tcp_dst);

//UDP
void
dpx86_set_udp_src(datapacket_t* pkt, uint16_t udp_src);

void
dpx86_set_udp_dst(datapacket_t* pkt, uint16_t udp_dst);

//ICMPV4
void
dpx86_set_icmpv4_type(datapacket_t* pkt, uint8_t type);

void
dpx86_set_icmpv4_code(datapacket_t* pkt, uint8_t code);

//MPLS
void
dpx86_set_mpls_label(datapacket_t* pkt, uint32_t label);

void
dpx86_set_mpls_tc(datapacket_t* pkt, uint8_t tc);

void
dpx86_set_mpls_bos(datapacket_t* pkt, bool bos);


//PPPOE
void
dpx86_set_pppoe_type(datapacket_t* pkt, uint8_t type);

void
dpx86_set_pppoe_code(datapacket_t* pkt, uint8_t code);

void
dpx86_set_pppoe_sid(datapacket_t* pkt, uint16_t sid);

//PPP
void
dpx86_set_ppp_proto(datapacket_t* pkt, uint16_t proto);

//GTP
void
dpx86_set_gtp_msg_type(datapacket_t* pkt, uint8_t gtp_msg_type);

void
dpx86_set_gtp_teid(datapacket_t* pkt, uint32_t teid);

/* Output action */
void
dpx86_output_packet(datapacket_t* pkt, switch_port_t* port);

/**
 * retrieve a pointer to the internal data
 * @return
 */
uint8_t*
dpx86_get_raw_data(datapacket_t *pkt);

/**
 * get the size of pkt
 * todo this returns size_t while OF specification foresees only a uint16_t
 *
 * @param pkt
 * @return
 */
size_t
dpx86_get_packet_size(datapacket_t* pkt);

//Ports
uint32_t
dpx86_get_packet_port_in(datapacket_t * const pkt);

uint32_t
dpx86_get_packet_phy_port_in(datapacket_t * const pkt);

//Associated metadata TODO
//uint64_t dpx86_get_packet_metadata(datapacket_t *const pkt);

//802
uint64_t
dpx86_get_packet_eth_dst(datapacket_t * const pkt);

uint64_t
dpx86_get_packet_eth_src(datapacket_t * const pkt);

uint16_t
dpx86_get_packet_eth_type(datapacket_t * const pkt);

//802.1q VLAN outermost tag
uint16_t
dpx86_get_packet_vlan_vid(datapacket_t * const pkt);

uint8_t
dpx86_get_packet_vlan_pcp(datapacket_t * const pkt);

//ARP
uint16_t
dpx86_get_packet_arp_opcode(datapacket_t * const pkt);

uint64_t
dpx86_get_packet_arp_sha(datapacket_t * const pkt);

uint32_t
dpx86_get_packet_arp_spa(datapacket_t * const pkt);

uint64_t
dpx86_get_packet_arp_tha(datapacket_t * const pkt);

uint32_t
dpx86_get_packet_arp_tpa(datapacket_t * const pkt);

//IP
uint8_t
dpx86_get_packet_ip_proto(datapacket_t * const pkt);
uint8_t
dpx86_get_packet_ip_ecn(datapacket_t * const pkt);
uint8_t
dpx86_get_packet_ip_dscp(datapacket_t * const pkt);

//IPv4
uint32_t
dpx86_get_packet_ipv4_src(datapacket_t * const pkt);

uint32_t
dpx86_get_packet_ipv4_dst(datapacket_t * const pkt);

//TCP
uint16_t
dpx86_get_packet_tcp_dst(datapacket_t * const pkt);

uint16_t
dpx86_get_packet_tcp_src(datapacket_t * const pkt);

//UDP
uint16_t
dpx86_get_packet_udp_dst(datapacket_t * const pkt);

uint16_t
dpx86_get_packet_udp_src(datapacket_t * const pkt);

//ICMPv4
uint8_t
dpx86_get_packet_icmpv4_type(datapacket_t * const pkt);

uint8_t
dpx86_get_packet_icmpv4_code(datapacket_t * const pkt);

//IPv6
uint128__t
dpx86_get_packet_ipv6_src(datapacket_t * const pkt);

uint128__t
dpx86_get_packet_ipv6_dst(datapacket_t * const pkt);

uint64_t
dpx86_get_packet_ipv6_flabel(datapacket_t * const pkt);

uint128__t
dpx86_get_packet_ipv6_nd_target(datapacket_t * const pkt);

uint64_t
dpx86_get_packet_ipv6_nd_sll(datapacket_t * const pkt);

uint64_t
dpx86_get_packet_ipv6_nd_tll(datapacket_t * const pkt);

uint16_t
dpx86_get_packet_ipv6_exthdr(datapacket_t * const pkt);

//ICMPv6
uint8_t
dpx86_get_packet_icmpv6_type(datapacket_t * const pkt);

uint8_t
dpx86_get_packet_icmpv6_code(datapacket_t * const pkt);

//MPLS-outermost label
uint32_t
dpx86_get_packet_mpls_label(datapacket_t * const pkt);

uint8_t
dpx86_get_packet_mpls_tc(datapacket_t * const pkt);

bool
dpx86_get_packet_mpls_bos(datapacket_t * const pkt);


//PPPoE related extensions
uint8_t
dpx86_get_packet_pppoe_code(datapacket_t * const pkt);

uint8_t
dpx86_get_packet_pppoe_type(datapacket_t * const pkt);

uint16_t
dpx86_get_packet_pppoe_sid(datapacket_t * const pkt);

//PPP related extensions
uint16_t
dpx86_get_packet_ppp_proto(datapacket_t * const pkt);

//GTP related extensions
uint8_t
dpx86_get_packet_gtp_msg_type(datapacket_t * const pkt);

uint32_t
dpx86_get_packet_gtp_teid(datapacket_t * const pkt);



//C++ extern C
ROFL_END_DECLS

#endif /* DATAPACKETX86_C_WRAPPER_H_ */
