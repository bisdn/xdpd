/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/platform/packet.h>

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <rofl/datapath/hal/openflow/openflow1x/of1x_cmm.h>

uint32_t packet_size=1500;
uint32_t port=0x1;
uint128__t addr_128={{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

/*
* Getters
*/

uint32_t
platform_packet_get_size_bytes(datapacket_t * const pkt)
{
	 return packet_size;
}
uint32_t
*platform_packet_get_port_in(datapacket_t * const pkt)
{
	 return &port; //0x0;
}

uint32_t
*platform_packet_get_phy_port_in(datapacket_t * const pkt)
{
	 return &port; //0x0;
}

uint64_t
*platform_packet_get_eth_dst(datapacket_t * const pkt)
{
	 return 0x0;
}

uint64_t
*platform_packet_get_eth_src(datapacket_t * const pkt)
{
	 return 0x0;
}

uint16_t
*platform_packet_get_eth_type(datapacket_t * const pkt)
{
	 return 0x0;
}

uint16_t
*platform_packet_get_vlan_vid(datapacket_t * const pkt)
{
	 return 0x0;
}

uint8_t
*platform_packet_get_vlan_pcp(datapacket_t * const pkt)
{
	 return 0x0;
}

bool
platform_packet_has_vlan(datapacket_t * const pkt)
{
	return 0;
}

uint16_t
*platform_packet_get_arp_opcode(datapacket_t * const pkt)
{
	return 0x0;
}

uint64_t
*platform_packet_get_arp_sha(datapacket_t * const pkt)
{
	return 0x0;
}

uint32_t
*platform_packet_get_arp_spa(datapacket_t * const pkt)
{
	return 0x0;
}

uint64_t
*platform_packet_get_arp_tha(datapacket_t * const pkt)
{
	return 0x0;
}

uint32_t
*platform_packet_get_arp_tpa(datapacket_t * const pkt)
{
	return 0x0;
}

uint8_t
*platform_packet_get_ip_proto(datapacket_t * const pkt)
{
	 return 0x0;
}
uint8_t
platform_packet_get_ip_ecn(datapacket_t * const pkt)
{
	 return 0x0;
}
uint8_t
platform_packet_get_ip_dscp(datapacket_t * const pkt)
{
	 return 0x0;
}

uint32_t
*platform_packet_get_ipv4_src(datapacket_t * const pkt)
{
	 return 0x0;
}

uint32_t
*platform_packet_get_ipv4_dst(datapacket_t * const pkt)
{
	 return 0x0;
}

uint128__t
*platform_packet_get_ipv6_src(datapacket_t * const pkt)
{
	return &addr_128;
}

uint128__t
*platform_packet_get_ipv6_dst(datapacket_t * const pkt)
{
	return &addr_128;
}

uint32_t
*platform_packet_get_ipv6_flabel(datapacket_t * const pkt)
{
	return 0x0;
}

uint128__t
*platform_packet_get_ipv6_nd_target(datapacket_t * const pkt)
{
	return &addr_128;
}

uint16_t
*platform_packet_get_ipv6_exthdr(datapacket_t * const pkt)
{
	return 0x0;
}

uint64_t
*platform_packet_get_ipv6_nd_sll(datapacket_t * const pkt)
{
	return 0x0;
}

uint64_t
*platform_packet_get_ipv6_nd_tll(datapacket_t * const pkt)
{
	return 0x0;
}

uint8_t
*platform_packet_get_icmpv6_type(datapacket_t * const pkt)
{
	return 0x0;
}

uint8_t
*platform_packet_get_icmpv6_code(datapacket_t * const pkt)
{
	return 0x0;
}

uint16_t
*platform_packet_get_tcp_dst(datapacket_t * const pkt)
{
	 return 0x0;
}

uint16_t
*platform_packet_get_tcp_src(datapacket_t * const pkt)
{
	 return 0x0;
}

uint16_t
*platform_packet_get_udp_dst(datapacket_t * const pkt)
{
	 return 0x0;
}

uint16_t
*platform_packet_get_udp_src(datapacket_t * const pkt)
{
	 return 0x0;
}

uint8_t
*platform_packet_get_icmpv4_type(datapacket_t * const pkt)
{
	 return 0x0;
}

uint16_t
*platform_packet_get_sctp_dst(datapacket_t * const pkt)
{
	 return 0x0;
}

uint16_t
*platform_packet_get_sctp_src(datapacket_t * const pkt)
{
	 return 0x0;
}

uint8_t
*platform_packet_get_icmpv4_code(datapacket_t * const pkt)
{
	 return 0x0;
}

uint32_t
*platform_packet_get_mpls_label(datapacket_t * const pkt)
{
	 return 0x0;
}

uint8_t
*platform_packet_get_mpls_tc(datapacket_t * const pkt)
{
	 return 0x0;
}

bool
platform_packet_get_mpls_bos(datapacket_t * const pkt)
{
	 return 0x0;
}
uint32_t *platform_packet_get_pbb_isid(datapacket_t *const pkt){
	//TODO: add implementation when supported
	return 0x0;
}

//Tunnel id
uint64_t *platform_packet_get_tunnel_id(datapacket_t *const pkt){
	//TODO: add implementation when supported
	return 0x0ULL;
}
uint8_t
*platform_packet_get_pppoe_code(datapacket_t * const pkt)
{
	 return 0x0;
}

uint8_t
*platform_packet_get_pppoe_type(datapacket_t * const pkt)
{
	 return 0x0;
}

uint16_t
*platform_packet_get_pppoe_sid(datapacket_t * const pkt)
{
	 return 0x0;
}

uint16_t
*platform_packet_get_ppp_proto(datapacket_t * const pkt)
{
	 return 0x0;
}

uint8_t
*platform_packet_get_gtp_msg_type(datapacket_t * const pkt)
{
	 return 0x0;
}

uint32_t
*platform_packet_get_gtp_teid(datapacket_t * const pkt)
{
	 return 0x0;
}


uint8_t* platform_packet_get_capwap_wbid(datapacket_t * const pkt)
{
	return 0x0;
}

uint8_t* platform_packet_get_capwap_rid(datapacket_t * const pkt)
{
	return 0x0;
}

uint16_t* platform_packet_get_capwap_flags(datapacket_t * const pkt)
{
	return 0x0;
}

uint16_t* platform_packet_get_wlan_fc(datapacket_t * const pkt)
{
	return 0x0;
}

uint8_t* platform_packet_get_wlan_type(datapacket_t * const pkt)
{
	return 0x0;
}

uint8_t* platform_packet_get_wlan_subtype(datapacket_t * const pkt)
{
	return 0x0;
}

uint8_t* platform_packet_get_wlan_direction(datapacket_t * const pkt)
{
	return 0x0;
}

uint64_t* platform_packet_get_wlan_address_1(datapacket_t * const pkt)
{
	return 0x0;
}

uint64_t* platform_packet_get_wlan_address_2(datapacket_t * const pkt)
{
	return 0x0;
}

uint64_t* platform_packet_get_wlan_address_3(datapacket_t * const pkt)
{
	return 0x0;
}

/*
* Actions
*/

void
platform_packet_copy_ttl_in(datapacket_t* pkt)
{
	fprintf(stderr,"COPY TTL IN\n");
	//dpx86_copy_ttl_in(pkt);
}

void
platform_packet_pop_vlan(datapacket_t* pkt)
{
	fprintf(stderr,"POP VLAN\n");
}

void
platform_packet_pop_mpls(datapacket_t* pkt, uint16_t ether_type)
{
	fprintf(stderr,"POP MPLS\n");
	//dpx86_pop_mpls(pkt, ether_type);
}

void
platform_packet_pop_pppoe(datapacket_t* pkt, uint16_t ether_type)
{
	fprintf(stderr,"POP PPPOE\n");
// 	dpx86_pop_pppoe(pkt, ether_type);
}

void
platform_packet_push_pppoe(datapacket_t* pkt, uint16_t ether_type)
{
	fprintf(stderr,"PUSH PPPOE\n");
// 	dpx86_push_pppoe(pkt, ether_type);
}

void
platform_packet_push_mpls(datapacket_t* pkt, uint16_t ether_type)
{
	fprintf(stderr,"PUSH MPLS\n");
// 	dpx86_push_mpls(pkt, ether_type);
}
void platform_packet_pop_gtp(datapacket_t* pkt)
{
	//TODO: implement
}
void platform_packet_push_gtp(datapacket_t* pkt)
{
	//TODO: implement
}

void platform_packet_pop_capwap(datapacket_t* pkt)
{
	//TODO: implement
}

void platform_packet_push_capwap(datapacket_t* pkt)
{
	//TODO: implement
}

void platform_packet_pop_wlan(datapacket_t* pkt)
{
	//TODO: implement
}

void platform_packet_push_wlan(datapacket_t* pkt)
{
	//TODO: implement
}


void
platform_packet_push_vlan(datapacket_t* pkt, uint16_t ether_type)
{
	fprintf(stderr,"PUSH VLAN\n");
// 	dpx86_push_vlan(pkt, ether_type);
}
void platform_packet_pop_pbb(datapacket_t* pkt)
{
	//TODO: implement
}
void platform_packet_push_pbb(datapacket_t* pkt, uint16_t ether_type)
{
	//TODO: implement
}
void
platform_packet_copy_ttl_out(datapacket_t* pkt)
{
	fprintf(stderr,"COPY TTL OUT\n");
// 	dpx86_copy_ttl_out(pkt);
}

void
platform_packet_dec_nw_ttl(datapacket_t* pkt)
{
	fprintf(stderr,"DEC NW TTL\n");
// 	dpx86_dec_nw_ttl(pkt);
}

void
platform_packet_dec_mpls_ttl(datapacket_t* pkt)
{
	fprintf(stderr,"DEC MPLS TTL\n");
// 	dpx86_dec_mpls_ttl(pkt);
}

void
platform_packet_set_mpls_ttl(datapacket_t* pkt, uint8_t new_ttl)
{
	fprintf(stderr,"SET MPLS TTL\n");
// 	dpx86_set_mpls_ttl(pkt, new_ttl);
}

void
platform_packet_set_nw_ttl(datapacket_t* pkt, uint8_t new_ttl)
{
	fprintf(stderr,"SET NW TTL\n");
// 	dpx86_set_nw_ttl(pkt, new_ttl);
}

void
platform_packet_set_queue(datapacket_t* pkt, uint32_t queue)
{
	fprintf(stderr,"SET QUEUE\n");
// 	dpx86_set_queue(pkt, queue);
}

#if HAVE_METADATA_PROCESSING // todo this has to be implemented
void
platform_packet_set_metadata(datapacket_t* pkt, uint64_t metadata)
{
	fprintf(stderr,"SET METADATA\n");
}
#endif

void
platform_packet_set_eth_dst(datapacket_t* pkt, uint64_t eth_dst)
{
	fprintf(stderr,"SET ETH DST\n");
// 	dpx86_set_eth_dst(pkt, eth_dst);
}

void
platform_packet_set_eth_src(datapacket_t* pkt, uint64_t eth_src)
{
	fprintf(stderr,"SET ETH SRC\n");
// 	dpx86_set_eth_src(pkt, eth_src);
}

void
platform_packet_set_eth_type(datapacket_t* pkt, uint16_t eth_type)
{
	fprintf(stderr,"SET ETH TYPE\n");
// 	dpx86_set_eth_type(pkt, eth_type);
}

void
platform_packet_set_vlan_vid(datapacket_t* pkt, uint16_t vlan_vid)
{
	fprintf(stderr,"SET VLAN VID\n");
// 	dpx86_set_vlan_vid(pkt, vlan_vid);
}

void
platform_packet_set_vlan_pcp(datapacket_t* pkt, uint8_t vlan_pcp)
{
	fprintf(stderr,"SET VLAN PCP\n");
// 	dpx86_set_vlan_pcp(pkt, vlan_pcp);
}

void
platform_packet_set_arp_opcode(datapacket_t* pkt, uint16_t arp_opcode)
{
	fprintf(stderr,"SET ARP OPCODE\n");
// 	dpx86_set_arp_opcode(pkt, arp_opcode);
}

void
platform_packet_set_arp_sha(datapacket_t* pkt, uint64_t arp_sha)
{
	fprintf(stderr,"SET ARP SHA\n");
// 	dpx86_set_arp_sha(pkt, arp_sha);
}

void
platform_packet_set_arp_spa(datapacket_t* pkt, uint32_t arp_spa)
{
	fprintf(stderr,"SET ARP SPA\n");
// 	dpx86_set_arp_spa(pkt, arp_spa);
}

void
platform_packet_set_arp_tha(datapacket_t* pkt, uint64_t arp_tha)
{
	fprintf(stderr,"SET ARP THA\n");
// 	dpx86_set_arp_sha(pkt, arp_tha);
}

void
platform_packet_set_arp_tpa(datapacket_t* pkt, uint32_t arp_tpa)
{
	fprintf(stderr,"SET ARP TPA\n");
// 	dpx86_set_arp_spa(pkt, arp_tpa);
}


void
platform_packet_set_ip_dscp(datapacket_t* pkt, uint8_t ip_dscp)
{
	fprintf(stderr,"SET IP DSCP\n");
// 	dpx86_set_ip_dscp(pkt, ip_dscp);
}

void
platform_packet_set_ip_ecn(datapacket_t* pkt, uint8_t ip_ecn)
{
	fprintf(stderr,"SET IP ECN\n");
// 	dpx86_set_ip_ecn(pkt, ip_ecn);
}

void
platform_packet_set_ip_proto(datapacket_t* pkt, uint8_t ip_proto)
{
	fprintf(stderr,"SET IP PROTO\n");
// 	dpx86_set_ip_proto(pkt, ip_proto);
}

void
platform_packet_set_ipv4_src(datapacket_t* pkt, uint32_t ip_src)
{
	fprintf(stderr,"SET IPv4 SRC\n");
// 	dpx86_set_ipv4_src(pkt, ip_src);
}

void
platform_packet_set_ipv4_dst(datapacket_t* pkt, uint32_t ip_dst)
{
	fprintf(stderr,"SET IPv4 DST\n");
// 	dpx86_set_ipv4_dst(pkt, ip_dst);
}

void
platform_packet_set_ipv6_src(datapacket_t* pkt, uint128__t ip_src)
{
	fprintf(stderr,"SET IPv6 SRC\n");
// 	dpx86_set_ipv6_src(pkt, ip_src);
}

void
platform_packet_set_ipv6_dst(datapacket_t* pkt, uint128__t ip_dst)
{
	fprintf(stderr,"SET IPv6 DST\n");
// 	dpx86_set_ipv6_dst(pkt, ip_dst);
}

void
platform_packet_set_ipv6_flabel(datapacket_t* pkt, uint64_t flabel)
{
	fprintf(stderr,"SET IPv6 FLABEL\n");
// 	dpx86_set_ipv6_flabel(pkt, flabel);
}

void
platform_packet_set_ipv6_nd_target(datapacket_t* pkt, uint128__t nd_target)
{
	fprintf(stderr,"SET IPv6 ND TARGET\n");
// 	dpx86_set_ipv6_nd_target(pkt, nd_target);
}

void
platform_packet_set_ipv6_nd_sll(datapacket_t* pkt, uint64_t sll)
{
	fprintf(stderr,"SET IPv6 ND SLL\n");
// 	dpx86_set_ipv6_nd_sll(pkt, sll);
}

void
platform_packet_set_ipv6_nd_tll(datapacket_t* pkt, uint64_t tll)
{
	fprintf(stderr,"SET IPv6 ND TLL\n");
// 	dpx86_set_ipv6_nd_tll(pkt, tll);
}

void
platform_packet_set_ipv6_exthdr(datapacket_t* pkt, uint16_t exthdr)
{
	fprintf(stderr,"SET IPv6 ND TLL\n");
// 	dpx86_set_ipv6_exthdr(pkt, exthdr);
}

void
platform_packet_set_icmpv6_type(datapacket_t* pkt, uint8_t type)
{
	fprintf(stderr,"SET ICMPv6 TYPE\n");
// 	dpx86_set_icmpv6_type(pkt, type);
}

void
platform_packet_set_icmpv6_code(datapacket_t* pkt, uint8_t code)
{
	fprintf(stderr,"SET ICMPv6 CODE\n");
// 	dpx86_set_icmpv6_code(pkt, code);
}

void
platform_packet_set_tcp_src(datapacket_t* pkt, uint16_t tcp_src)
{
	fprintf(stderr,"SET TCP SRC\n");
// 	dpx86_set_tcp_src(pkt, tcp_src);
}

void
platform_packet_set_tcp_dst(datapacket_t* pkt, uint16_t tcp_dst)
{
	fprintf(stderr,"SET TCP DST\n");
// 	dpx86_set_tcp_dst(pkt, tcp_dst);
}

void
platform_packet_set_udp_src(datapacket_t* pkt, uint16_t udp_src)
{
	fprintf(stderr,"SET UDP SRC\n");
// 	dpx86_set_udp_src(pkt, udp_src);
}

void
platform_packet_set_sctp_dst(datapacket_t* pkt, uint16_t sctp_dst)
{
	fprintf(stderr,"SET UDP DST\n");
// 	dpx86_set_udp_dst(pkt, sctp_dst);
}

void
platform_packet_set_sctp_src(datapacket_t* pkt, uint16_t sctp_src)
{
	fprintf(stderr,"SET UDP SRC\n");
// 	dpx86_set_udp_src(pkt, sctp_src);
}

void
platform_packet_set_udp_dst(datapacket_t* pkt, uint16_t udp_dst)
{
	fprintf(stderr,"SET UDP DST\n");
// 	dpx86_set_udp_dst(pkt, udp_dst);
}
void
platform_packet_set_icmpv4_type(datapacket_t* pkt, uint8_t type)
{
	fprintf(stderr,"SET ICMPv4 TYPE\n");
// 	dpx86_set_icmpv4_type(pkt, type);
}

void
platform_packet_set_icmpv4_code(datapacket_t* pkt, uint8_t code)
{
	fprintf(stderr,"SET ICMPv4 CODE\n");
// 	dpx86_set_icmpv4_code(pkt, code);
}

void
platform_packet_set_mpls_label(datapacket_t* pkt, uint32_t label)
{
	fprintf(stderr,"SET MPLS LABEL\n");
// 	dpx86_set_mpls_label(pkt, label);
}

void
platform_packet_set_mpls_tc(datapacket_t* pkt, uint8_t tc)
{
	fprintf(stderr,"SET MPLS TC\n");
// 	dpx86_set_mpls_tc(pkt, tc);
}

void
platform_packet_set_mpls_bos(datapacket_t* pkt, bool bos)
{
	fprintf(stderr,"SET MPLS BOS\n");
// 	dpx86_set_mpls_bos(pkt, bos);
}

void platform_packet_set_pbb_isid(datapacket_t*pkt, uint32_t pbb_isid)
{
	//TODO: implement
}
void platform_packet_set_tunnel_id(datapacket_t*pkt, uint64_t tunnel_id)
{
	//TODO: implement
}
void
platform_packet_set_pppoe_type(datapacket_t* pkt, uint8_t type)
{
	fprintf(stderr,"SET PPPOE TYPE\n");
// 	dpx86_set_pppoe_type(pkt, type);
}

void
platform_packet_set_pppoe_code(datapacket_t* pkt, uint8_t code)
{
	fprintf(stderr,"SET PPPOE CODE\n");
// 	dpx86_set_pppoe_code(pkt, code);
}

void
platform_packet_set_pppoe_sid(datapacket_t* pkt, uint16_t sid)
{
	fprintf(stderr,"SET PPPOE SID\n");
// 	dpx86_set_pppoe_sid(pkt, sid);
}

void
platform_packet_set_ppp_proto(datapacket_t* pkt, uint16_t proto)
{
	fprintf(stderr,"SET PPP PROTO\n");
// 	dpx86_set_ppp_proto(pkt, proto);
}

void
platform_packet_set_gtp_msg_type(datapacket_t* pkt, uint8_t msg_type)
{
	fprintf(stderr,"SET GTP MSG TYPE\n");
// 	dpx86_set_gtp_msg_type(pkt, msg_type);
}

void
platform_packet_set_gtp_teid(datapacket_t* pkt, uint32_t teid)
{
	fprintf(stderr,"SET GTP TEID\n");
// 	dpx86_set_gtp_teid(pkt, teid);
}

void platform_packet_set_capwap_wbid(datapacket_t* pkt, uint8_t wbid)
{
	fprintf(stderr,"SET CAPWAP WBID\n");
}

void platform_packet_set_capwap_rid(datapacket_t* pkt, uint8_t rid)
{
	fprintf(stderr,"SET CAPWAP RID\n");
}

void platform_packet_set_capwap_flags(datapacket_t* pkt, uint16_t flags)
{
	fprintf(stderr,"SET CAPWAP FLAGS\n");
}

void platform_packet_set_wlan_fc(datapacket_t* pkt, uint16_t fc)
{
	fprintf(stderr,"SET WLAN FC\n");
}

void platform_packet_set_wlan_type(datapacket_t* pkt, uint8_t type)
{
	fprintf(stderr,"SET WLAN TYPE\n");
}

void platform_packet_set_wlan_subtype(datapacket_t* pkt, uint8_t subtype)
{
	fprintf(stderr,"SET WLAN SUBTYPE\n");
}

void platform_packet_set_wlan_direction(datapacket_t* pkt, uint8_t direction)
{
	fprintf(stderr,"SET WLAN DIRECTION\n");
}

void platform_packet_set_wlan_address_1(datapacket_t* pkt, uint64_t address_1)
{
	fprintf(stderr,"SET WLAN ADDRESS 1\n");
}

void platform_packet_set_wlan_address_2(datapacket_t* pkt, uint64_t address_2)
{
	fprintf(stderr,"SET WLAN ADDRESS 2\n");
}

void platform_packet_set_wlan_address_3(datapacket_t* pkt, uint64_t address_3)
{
	fprintf(stderr,"SET WLAN ADDRESS 3\n");
}

void
platform_packet_drop(datapacket_t* pkt)
{
	fprintf(stderr,"PAKCET DROP!\n");
	//Release buffer
// 	bufferpool_release_buffer_wrapper(pkt);

}

void
platform_packet_output(datapacket_t* pkt, switch_port_t* port)
{
	fprintf(stderr,"OUTPUT #%p\n",port);
// 	dpx86_output_packet(pkt, port);
}

datapacket_t* platform_packet_replicate(datapacket_t* pkt){

	return NULL;	
}


