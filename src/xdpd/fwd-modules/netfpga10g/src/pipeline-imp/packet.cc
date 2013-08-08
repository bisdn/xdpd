#include <inttypes.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/platform/packet.h>
#include <rofl/datapath/afa/openflow/openflow12/of12_cmm.h>
#include <rofl/common/utils/c_logger.h>

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../config.h"
#include "../io/datapacketx86.h"
#include "../io/bufferpool.h"

/*
* ROFL-Pipeline packet mangling platform API implementation
*/

//Getters
uint32_t
platform_packet_get_size_bytes(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	return pack->get_buffer_length();
}

uint32_t
platform_packet_get_port_in(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return 0;
	return pack->in_port;
}

uint32_t
platform_packet_get_phy_port_in(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return 0;
	return pack->in_phy_port;
}

uint64_t
platform_packet_get_eth_dst(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
        
	if ((NULL == pack) || (NULL == pack->headers->ether(0))) return 0;

	return pack->headers->ether(0)->get_dl_dst().get_mac();
}

uint64_t
platform_packet_get_eth_src(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
        
	if ((NULL == pack) || (NULL == pack->headers->ether(0))) return 0;
        
	return pack->headers->ether(0)->get_dl_src().get_mac();

}

uint16_t
platform_packet_get_eth_type(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ether(0))) return 0;
	
	if (pack->headers->vlan(-1) != NULL)
		return pack->headers->vlan(-1)->get_dl_type();
	
	return pack->headers->ether(0)->get_dl_type();
}

uint16_t
platform_packet_get_vlan_vid(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->vlan(0))) return 0;
	
	return pack->headers->vlan(0)->get_dl_vlan_id()&0x1FFF;
}

uint8_t
platform_packet_get_vlan_pcp(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->vlan(0))) return 0;
	return pack->headers->vlan(0)->get_dl_vlan_pcp()&0x07;
}

uint8_t
platform_packet_get_ip_ecn(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return 0;

	return pack->headers->ipv4(0)->get_ipv4_ecn()&0xFF;
}

uint8_t
platform_packet_get_ip_dscp(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return 0;

	return pack->headers->ipv4(0)->get_ipv4_dscp()&0xFF;
}
uint8_t
platform_packet_get_ip_proto(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return 0;

	return pack->headers->ipv4(0)->get_ipv4_proto()&0xFF;
}

uint32_t
platform_packet_get_ipv4_src(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return 0;

	return be32toh(pack->headers->ipv4(0)->get_ipv4_src().ca_s4addr->sin_addr.s_addr);
}

uint32_t
platform_packet_get_ipv4_dst(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return 0;

	return be32toh(pack->headers->ipv4(0)->get_ipv4_dst().ca_s4addr->sin_addr.s_addr);
}

uint16_t
platform_packet_get_tcp_dst(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->tcp(0))) return 0;
	return pack->headers->tcp(0)->get_dport();
}

uint16_t
platform_packet_get_tcp_src(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->tcp(0))) return 0;
	return pack->headers->tcp(0)->get_sport();
}

uint16_t
platform_packet_get_udp_dst(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->udp(0))) return 0;
	return pack->headers->udp(0)->get_dport();
}

uint16_t
platform_packet_get_udp_src(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->udp(0))) return 0;
	return pack->headers->udp(0)->get_sport();
}

uint8_t
platform_packet_get_icmpv4_type(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv4(0))) return 0;
	return pack->headers->icmpv4(0)->get_icmp_type();
}

uint8_t
platform_packet_get_icmpv4_code(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv4(0))) return 0;
	return pack->headers->icmpv4(0)->get_icmp_code();
}

uint32_t
platform_packet_get_mpls_label(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return 0;
	return pack->headers->mpls(0)->get_mpls_label()&0x000FFFFF;
}

uint8_t
platform_packet_get_mpls_tc(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return 0;
	return pack->headers->mpls(0)->get_mpls_tc()&0x07;
}

uint8_t
platform_packet_get_pppoe_code(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->pppoe(0))) return 0;
	return pack->headers->pppoe(0)->get_pppoe_code();
}

uint8_t
platform_packet_get_pppoe_type(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->pppoe(0))) return 0;
	return pack->headers->pppoe(0)->get_pppoe_type()&0x0F;
}

uint16_t
platform_packet_get_pppoe_sid(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->pppoe(0))) return 0;
	return pack->headers->pppoe(0)->get_pppoe_sessid();
}

uint16_t
platform_packet_get_ppp_proto(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ppp(0))) return 0;
	return pack->headers->ppp(0)->get_ppp_prot();
}

uint8_t
platform_packet_get_gtp_msg_type(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->gtp(0))) return 0;
	return pack->headers->gtp(0)->get_msg_type();
}

uint32_t
platform_packet_get_gtp_teid(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->gtp(0))) return 0;
	return pack->headers->gtp(0)->get_teid();
}

//Actions
void
platform_packet_copy_ttl_in(datapacket_t* pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	// TODO
}

void
platform_packet_pop_vlan(datapacket_t* pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	pack->headers->pop_vlan();
}

void
platform_packet_pop_mpls(datapacket_t* pkt, uint16_t ether_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	pack->headers->pop_mpls(ether_type);
}

void
platform_packet_pop_pppoe(datapacket_t* pkt, uint16_t ether_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	pack->headers->pop_pppoe(ether_type);
	
}

void
platform_packet_push_pppoe(datapacket_t* pkt, uint16_t ether_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	pack->headers->push_pppoe(ether_type);

}

void
platform_packet_push_mpls(datapacket_t* pkt, uint16_t ether_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	pack->headers->push_mpls(ether_type);
}

void
platform_packet_push_vlan(datapacket_t* pkt, uint16_t ether_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	pack->headers->push_vlan(ether_type);
}

void
platform_packet_copy_ttl_out(datapacket_t* pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	// TODO
}

void
platform_packet_dec_nw_ttl(datapacket_t* pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	// TODO IPv6
	pack->headers->ipv4(0)->dec_ipv4_ttl();
	pack->ipv4_recalc_checksum = true;
}

void
platform_packet_dec_mpls_ttl(datapacket_t* pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return;
	pack->headers->mpls(0)->dec_mpls_ttl();

}

void
platform_packet_set_mpls_ttl(datapacket_t* pkt, uint8_t new_ttl)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return;
	pack->headers->mpls(0)->set_mpls_ttl(new_ttl);
}

void
platform_packet_set_nw_ttl(datapacket_t* pkt, uint8_t new_ttl)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	// TODO IPv6
	pack->headers->ipv4(0)->set_ipv4_ttl(new_ttl);
	pack->ipv4_recalc_checksum = true;

}

void
platform_packet_set_queue(datapacket_t* pkt, uint32_t queue)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;

	pack->output_queue = queue;	

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

//void platform_packet_set_metadata(datapacket_t* pkt, uint64_t metadata){}

//Ethernet
void
platform_packet_set_eth_dst(datapacket_t* pkt, uint64_t eth_dst)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ether(0))) return;
	pack->headers->ether(0)->set_dl_dst(rofl::cmacaddr(eth_dst));
}

void
platform_packet_set_eth_src(datapacket_t* pkt, uint64_t eth_src)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ether(0))) return;
	pack->headers->ether(0)->set_dl_src(rofl::cmacaddr(eth_src));
}

void
platform_packet_set_eth_type(datapacket_t* pkt, uint16_t eth_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ether(0))) return;
	pack->headers->ether(0)->set_dl_type(eth_type);
}

//802.1q
void
platform_packet_set_vlan_vid(datapacket_t* pkt, uint16_t vlan_vid)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->vlan(0))) return;
	pack->headers->vlan(0)->set_dl_vlan_id(vlan_vid);
}

void
platform_packet_set_vlan_pcp(datapacket_t* pkt, uint8_t vlan_pcp)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->vlan(0))) return;
	pack->headers->vlan(0)->set_dl_vlan_pcp(vlan_pcp);
}

//IP, IPv4
void
platform_packet_set_ip_dscp(datapacket_t* pkt, uint8_t ip_dscp)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	// TODO IPv6
	pack->headers->ipv4(0)->set_ipv4_dscp(ip_dscp);
	pack->ipv4_recalc_checksum = true;
}

void
platform_packet_set_ip_ecn(datapacket_t* pkt, uint8_t ip_ecn)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	// TODO IPv6
	pack->headers->ipv4(0)->set_ipv4_ecn(ip_ecn);
	pack->ipv4_recalc_checksum = true;
}

void
platform_packet_set_ip_proto(datapacket_t* pkt, uint8_t ip_proto)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	// TODO IPv6
	pack->headers->ipv4(0)->set_ipv4_proto(ip_proto);
	pack->ipv4_recalc_checksum = true;
}

void
platform_packet_set_ipv4_src(datapacket_t* pkt, uint32_t ip_src)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	pack->headers->ipv4(0)->set_ipv4_src(ip_src); // FIXME: is ip_src stored in network byte order
	pack->ipv4_recalc_checksum = true;
	pack->tcp_recalc_checksum = true;
	pack->udp_recalc_checksum = true;
}

void
platform_packet_set_ipv4_dst(datapacket_t* pkt, uint32_t ip_dst)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	pack->headers->ipv4(0)->set_ipv4_dst(ip_dst); // FIXME: is ip_dst stored in network byte order
	pack->ipv4_recalc_checksum = true;
	pack->tcp_recalc_checksum = true;
	pack->udp_recalc_checksum = true;
}

//TCP
void
platform_packet_set_tcp_src(datapacket_t* pkt, uint16_t tcp_src)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->tcp(0))) return;
	pack->headers->tcp(0)->set_sport(tcp_src);
	pack->tcp_recalc_checksum = true;
}

void
platform_packet_set_tcp_dst(datapacket_t* pkt, uint16_t tcp_dst)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->tcp(0))) return;
	pack->headers->tcp(0)->set_dport(tcp_dst);
	pack->tcp_recalc_checksum = true;
}

//UDP
void
platform_packet_set_udp_src(datapacket_t* pkt, uint16_t udp_src)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->udp(0))) return;
	pack->headers->udp(0)->set_sport(udp_src);
	pack->udp_recalc_checksum = true;
}

void
platform_packet_set_udp_dst(datapacket_t* pkt, uint16_t udp_dst)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->udp(0))) return;
	pack->headers->udp(0)->set_dport(udp_dst);
	pack->udp_recalc_checksum = true;
}

//ICMPV4
void
platform_packet_set_icmpv4_type(datapacket_t* pkt, uint8_t type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv4(0))) return;
	pack->headers->icmpv4(0)->set_icmp_type(type);
	pack->icmpv4_recalc_checksum = true;
}

void
platform_packet_set_icmpv4_code(datapacket_t* pkt, uint8_t code)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv4(0))) return;
	pack->headers->icmpv4(0)->set_icmp_code(code);
	pack->icmpv4_recalc_checksum = true;
}

//MPLS
void
platform_packet_set_mpls_label(datapacket_t* pkt, uint32_t label)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return;
	pack->headers->mpls(0)->set_mpls_label(label);
}

void
platform_packet_set_mpls_tc(datapacket_t* pkt, uint8_t tc)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return;
	pack->headers->mpls(0)->set_mpls_tc(tc);
}

//PPPOE
void
platform_packet_set_pppoe_type(datapacket_t* pkt, uint8_t type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->pppoe(0))) return;
	pack->headers->pppoe(0)->set_pppoe_type(type);
}

void
platform_packet_set_pppoe_code(datapacket_t* pkt, uint8_t code)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->pppoe(0))) return;
	pack->headers->pppoe(0)->set_pppoe_code(code);
}

void
platform_packet_set_pppoe_sid(datapacket_t* pkt, uint16_t sid)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->pppoe(0))) return;
	pack->headers->pppoe(0)->set_pppoe_sessid(sid);
}

//PPP
void
platform_packet_set_ppp_proto(datapacket_t* pkt, uint16_t proto)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ppp(0))) return;
	pack->headers->ppp(0)->set_ppp_prot(proto);
}

//GTP
void
platform_packet_set_gtp_msg_type(datapacket_t* pkt, uint8_t gtp_msg_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->gtp(0))) return;
	pack->headers->gtp(0)->set_msg_type(gtp_msg_type);
}

void
platform_packet_set_gtp_teid(datapacket_t* pkt, uint32_t teid)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->gtp(0))) return;
	pack->headers->gtp(0)->set_teid(teid);
}

void
platform_packet_drop(datapacket_t* pkt)
{
	ROFL_DEBUG("Dropping packet(%p)\n",pkt);
	
	//Release buffer
	bufferpool::release_buffer(pkt);
}

/**
* Output packet to the port(s)
* The action HAS to implement the destruction/release of the pkt
* (including if the pkt is a replica).
*
* If a flooding output actions needs to be done, the function
* has itself to deal with packet replication.
*/
void platform_packet_output(datapacket_t* pkt, switch_port_t* port){

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	//XXX
	//XXX implement!
	//XXX
}

/* Cloning of the packet */
static void clone_pkt_contents(datapacket_t* src, datapacket_t* dst){
	
	datapacketx86 *pack_src = (datapacketx86*)src->platform_state;
	datapacketx86 *pack_dst = (datapacketx86*)dst->platform_state;
	pack_dst->init(pack_src->get_buffer(), pack_src->get_buffer_length(), pack_src->lsw, pack_src->in_port, pack_src->in_phy_port, true, true);

	//copy checksum flags
	pack_dst->ipv4_recalc_checksum = pack_src->ipv4_recalc_checksum;
	pack_dst->icmpv4_recalc_checksum = pack_src->icmpv4_recalc_checksum;
	pack_dst->tcp_recalc_checksum = pack_src->tcp_recalc_checksum;
	pack_dst->udp_recalc_checksum = pack_src->udp_recalc_checksum;

	
};

/**
* Creates a copy (in heap) of the datapacket_t structure including any
* platform specific state (->platform_state). The following behaviour
* is expected from this hook:
* 
* - All data fields and pointers of datapacket_t struct must be memseted to 0, except:
* - datapacket_t flag is_replica must be set to true
* - platform_state, if used, must be replicated (copied) otherwise NULL
*
*/
datapacket_t* platform_packet_replicate(datapacket_t* pkt){

	//Get a free buffer
	datapacket_t* copy = bufferpool::get_free_buffer();
	
	//Make sure everything is memseted to 0
	memcpy(&copy->matches, &pkt->matches, sizeof(pkt->matches));
	memcpy(&copy->write_actions, &pkt->write_actions ,sizeof(pkt->write_actions));

	//mark as replica
	copy->is_replica = true;

	//Clone contents
	clone_pkt_contents(pkt,copy);
	return copy;	
}

