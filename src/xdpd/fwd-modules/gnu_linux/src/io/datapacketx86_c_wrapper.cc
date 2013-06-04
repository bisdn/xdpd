/*
 * datapacketx86_c_wrapper.cc
 *
 *  Created on: Jan 17, 2013
 *      Author: tobi
 */

#include "datapacketx86.h"
#include "bufferpool.h"
#include "ports/ioport.h"
#include "datapacketx86_c_wrapper.h"
#include <rofl/common/utils/c_logger.h>
#include <rofl/datapath/pipeline/platform/packet.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>

//Debug
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_packet_matches.h>
#include <rofl/datapath/pipeline/openflow/openflow12/pipeline/of12_match.h>


//Flood meta port
extern switch_port_t* flood_meta_port;

/* Cloning of the packet */
void
dpx86_clone_pkt_contents(datapacket_t* src, datapacket_t* dst){
	
	datapacketx86 *pack_src = (datapacketx86*)src->platform_state;
	datapacketx86 *pack_dst = (datapacketx86*)dst->platform_state;
	pack_dst->init(pack_src->get_buffer(), pack_src->get_buffer_length(), pack_src->lsw, pack_src->in_port, pack_src->in_phy_port, true);

	//copy checksum flags
       pack_dst->ipv4_recalc_checksum = pack_src->ipv4_recalc_checksum;
       pack_dst->icmpv4_recalc_checksum = pack_src->icmpv4_recalc_checksum;
       pack_dst->tcp_recalc_checksum = pack_src->tcp_recalc_checksum;
       pack_dst->udp_recalc_checksum = pack_src->udp_recalc_checksum;

	
};

using namespace rofl;

//FIXME: this is probably worth to be integrating it properly in the datapacketx86 class, specially
//concerning the checksum flags

/* Copy ttl */
void
dpx86_copy_ttl_in(datapacket_t* pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	// TODO
}

/* POP */
void
dpx86_pop_vlan(datapacket_t* pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	pack->headers->pop_vlan();
}

void
dpx86_pop_mpls(datapacket_t* pkt, uint16_t ether_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	pack->headers->pop_mpls(ether_type);
}

void
dpx86_pop_pppoe(datapacket_t* pkt, uint16_t ether_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	pack->headers->pop_pppoe(ether_type);
}

void
dpx86_push_pppoe(datapacket_t* pkt, uint16_t ether_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	pack->headers->push_pppoe(ether_type);
}

void
dpx86_push_mpls(datapacket_t* pkt, uint16_t ether_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	pack->headers->push_mpls(ether_type);
}

void
dpx86_push_vlan(datapacket_t* pkt, uint16_t ether_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	pack->headers->push_vlan(ether_type);
}

/* Copy ttl out*/
void
dpx86_copy_ttl_out(datapacket_t* pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	// TODO
}

/* Decrement ttl */
void
dpx86_dec_nw_ttl(datapacket_t* pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	// TODO IPv6
	pack->headers->ipv4(0)->dec_ipv4_ttl();
	pack->ipv4_recalc_checksum = true;
}

void
dpx86_dec_mpls_ttl(datapacket_t* pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return;
	pack->headers->mpls(0)->dec_mpls_ttl();
}

/* Set field */
void
dpx86_set_mpls_ttl(datapacket_t* pkt, uint8_t new_ttl)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return;
	pack->headers->mpls(0)->set_mpls_ttl(new_ttl);
}

void
dpx86_set_nw_ttl(datapacket_t* pkt, uint8_t new_ttl)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	// TODO IPv6
	pack->headers->ipv4(0)->set_ipv4_ttl(new_ttl);
	pack->ipv4_recalc_checksum = true;
}

void
dpx86_set_queue(datapacket_t* pkt, uint32_t queue)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;

	pack->output_queue = queue;	
}

//TODO:
//void dpx86_set_metadata(datapacket_t* pkt, uint64_t metadata);

//Ethernet
void
dpx86_set_eth_dst(datapacket_t* pkt, uint64_t eth_dst)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ether(0))) return;
	pack->headers->ether(0)->set_dl_dst(rofl::cmacaddr(eth_dst));
}

void
dpx86_set_eth_src(datapacket_t* pkt, uint64_t eth_src)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ether(0))) return;
	pack->headers->ether(0)->set_dl_src(rofl::cmacaddr(eth_src));
}

void
dpx86_set_eth_type(datapacket_t* pkt, uint16_t eth_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ether(0))) return;
	pack->headers->ether(0)->set_dl_type(eth_type);
}

//802.1q
void
dpx86_set_vlan_vid(datapacket_t* pkt, uint16_t vlan_vid)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->vlan(0))) return;
	pack->headers->vlan(0)->set_dl_vlan_id(vlan_vid);
}

void
dpx86_set_vlan_pcp(datapacket_t* pkt, uint8_t vlan_pcp)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->vlan(0))) return;
	pack->headers->vlan(0)->set_dl_vlan_pcp(vlan_pcp);
}

//IP, IPv4
void
dpx86_set_ip_dscp(datapacket_t* pkt, uint8_t ip_dscp)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	// TODO IPv6
	pack->headers->ipv4(0)->set_ipv4_dscp(ip_dscp);
	pack->ipv4_recalc_checksum = true;
}

void
dpx86_set_ip_ecn(datapacket_t* pkt, uint8_t ip_ecn)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	// TODO IPv6
	pack->headers->ipv4(0)->set_ipv4_ecn(ip_ecn);
	pack->ipv4_recalc_checksum = true;
}

void
dpx86_set_ip_proto(datapacket_t* pkt, uint8_t ip_proto)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	// TODO IPv6
	pack->headers->ipv4(0)->set_ipv4_proto(ip_proto);
	pack->ipv4_recalc_checksum = true;
}

void
dpx86_set_ipv4_src(datapacket_t* pkt, uint32_t ip_src)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	pack->headers->ipv4(0)->set_ipv4_src(ip_src); // FIXME: is ip_src stored in network byte order
	pack->ipv4_recalc_checksum = true;
	pack->tcp_recalc_checksum = true;
	pack->udp_recalc_checksum = true;
}

void
dpx86_set_ipv4_dst(datapacket_t* pkt, uint32_t ip_dst)
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
dpx86_set_tcp_src(datapacket_t* pkt, uint16_t tcp_src)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->tcp(0))) return;
	pack->headers->tcp(0)->set_sport(tcp_src);
	pack->tcp_recalc_checksum = true;
}

void
dpx86_set_tcp_dst(datapacket_t* pkt, uint16_t tcp_dst)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->tcp(0))) return;
	pack->headers->tcp(0)->set_dport(tcp_dst);
	pack->tcp_recalc_checksum = true;
}

//UDP
void
dpx86_set_udp_src(datapacket_t* pkt, uint16_t udp_src)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->udp(0))) return;
	pack->headers->udp(0)->set_sport(udp_src);
	pack->udp_recalc_checksum = true;
}

void
dpx86_set_udp_dst(datapacket_t* pkt, uint16_t udp_dst)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->udp(0))) return;
	pack->headers->udp(0)->set_dport(udp_dst);
	pack->udp_recalc_checksum = true;
}

//ICMPV4
void
dpx86_set_icmpv4_type(datapacket_t* pkt, uint8_t type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv4(0))) return;
	pack->headers->icmpv4(0)->set_icmp_type(type);
	pack->icmpv4_recalc_checksum = true;
}

void
dpx86_set_icmpv4_code(datapacket_t* pkt, uint8_t code)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv4(0))) return;
	pack->headers->icmpv4(0)->set_icmp_code(code);
	pack->icmpv4_recalc_checksum = true;
}

//MPLS
void
dpx86_set_mpls_label(datapacket_t* pkt, uint32_t label)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return;
	pack->headers->mpls(0)->set_mpls_label(label);
}

void
dpx86_set_mpls_tc(datapacket_t* pkt, uint8_t tc)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return;
	pack->headers->mpls(0)->set_mpls_tc(tc);
}

//PPPOE
void
dpx86_set_pppoe_type(datapacket_t* pkt, uint8_t type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->pppoe(0))) return;
	pack->headers->pppoe(0)->set_pppoe_type(type);
}

void
dpx86_set_pppoe_code(datapacket_t* pkt, uint8_t code)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->pppoe(0))) return;
	pack->headers->pppoe(0)->set_pppoe_code(code);
}

void
dpx86_set_pppoe_sid(datapacket_t* pkt, uint16_t sid)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->pppoe(0))) return;
	pack->headers->pppoe(0)->set_pppoe_sessid(sid);
}

//PPP
void
dpx86_set_ppp_proto(datapacket_t* pkt, uint16_t proto)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ppp(0))) return;
	pack->headers->ppp(0)->set_ppp_prot(proto);
}


static void dpx86_output_single_packet(datapacket_t* pkt, datapacketx86* pack, switch_port_t* port){

	//Output packet to the appropiate queue and port_num
	if(port && port->platform_port_state){
		
		ROFL_DEBUG("[%s] OUTPUT packet(%p)\n", port->name, pkt);
		of12_dump_packet_matches((of12_packet_matches_t*)pkt->matches);

		//Schedule in the port
		ioport* ioport_inst = (ioport*)port->platform_port_state; 
		ioport_inst->enqueue_packet(pkt, pack->output_queue);
	
		//Packet must never be retured to the buffer pool, the port will do that
		//once sent
	}else{
		//Silently drop the packet
		bufferpool::release_buffer(pkt);
		//TODO: debug trace here
	}
}

//FIXME TODO XXX this should be stripped from here and moved in a convenient place
/* Output action */
void dpx86_output_packet(datapacket_t* pkt, switch_port_t* output_port){
	of_switch_t* sw;
	datapacketx86* pack;

	if(!output_port)
		return;

	//Check whether dpx86 is NULL
	if (NULL == (pack = (datapacketx86*) (pkt->platform_state))){
		//TODO: in DEBUG do an EXIT(-1)
		assert(0);
		return;
	}

	//IP Checksum recalculation
	if(pack->ipv4_recalc_checksum){
		if(pack->headers->ipv4(0))	
			pack->headers->ipv4(0)->ipv4_calc_checksum();
	}

	fipv4frame *fipv4 = pack->headers->ipv4(-1);

	if ((pack->tcp_recalc_checksum) && pack->headers->tcp(0) && fipv4) {
		
		
	pack->headers->tcp(0)->tcp_calc_checksum(
			fipv4->get_ipv4_src(),
			fipv4->get_ipv4_dst(),
			fipv4->get_ipv4_proto(),
			pack->headers->get_pkt_len(pack->headers->tcp(0))); // start at innermost IPv4 up to and including last frame

	} else if ((pack->udp_recalc_checksum) && (pack->headers->udp(0)) && fipv4) {

		pack->headers->udp(0)->udp_calc_checksum(
				fipv4->get_ipv4_src(),
				fipv4->get_ipv4_dst(),
				fipv4->get_ipv4_proto(),
				pack->headers->get_pkt_len(pack->headers->udp(0))); // start at innermost IPv4 up to and including last frame

	} else if ((pack->icmpv4_recalc_checksum) && (pack->headers->icmpv4())) {

		pack->headers->icmpv4(0)->icmpv4_calc_checksum(pack->headers->get_pkt_len(pack->headers->icmpv4(0)));
	}


	//flood_meta_port is a static variable defined in the physical_switch
	//the meta_port
	if(output_port == flood_meta_port){
		datapacket_t* replica;
		switch_port_t* port_it;
		datapacketx86* replica_pack;

		//Get switch
		sw = pack->lsw;	

		//We need to flood
		for(unsigned i=0;i<LOGICAL_SWITCH_MAX_LOG_PORTS;++i){

			port_it = sw->logical_ports[i].port;

			//Check port is not incomming port, exists, and is up 
			if(i == pack->in_port || !port_it || !port_it->up)
				continue;

			//replicate packet
			replica = platform_packet_replicate(pkt); 	
			replica_pack = (datapacketx86*) (replica->platform_state);

			ROFL_DEBUG("[%s] OUTPUT FLOOD packet(%p), origin(%p)\n", port_it->name, replica, pkt);
			
			//send the replica
			dpx86_output_single_packet(replica, replica_pack, port_it);
		}

		of12_dump_packet_matches((of12_packet_matches_t*)pkt->matches);
			
		//discard the original packet always (has been replicated)
		bufferpool::release_buffer(pkt);
	}else{
		//Single output	
		dpx86_output_single_packet(pkt, pack, output_port);
	}

}

uint8_t*
dpx86_get_raw_data(datapacket_t* pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (ROFL_FAILURE == pack->transfer_to_user_space()) {
		return NULL;
	} else {
		return pack->get_buffer();
	}
}

size_t
dpx86_get_packet_size(datapacket_t* pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	return pack->get_buffer_length();
}

//Ports
uint32_t
dpx86_get_packet_port_in(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return 0;
	return pack->in_port;
}

uint32_t
dpx86_get_packet_phy_port_in(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return 0;
	return pack->in_phy_port;
}

//Associated metadata TODO
//uint64_t dpx86_get_packet_metadata(datapacket_t *const pkt);

//802
uint64_t
dpx86_get_packet_eth_dst(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
        
	if ((NULL == pack) || (NULL == pack->headers->ether(0))) return 0;

	return pack->headers->ether(0)->get_dl_dst().get_mac();
}

uint64_t
dpx86_get_packet_eth_src(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
        
	if ((NULL == pack) || (NULL == pack->headers->ether(0))) return 0;
        
	return pack->headers->ether(0)->get_dl_src().get_mac();
}

uint16_t
dpx86_get_packet_eth_type(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ether(0))) return 0;
	
	if (pack->headers->vlan(-1) != NULL)
		return pack->headers->vlan(-1)->get_dl_type();
	
	return pack->headers->ether(0)->get_dl_type();
}

//802.1q VLAN outermost tag
uint16_t
dpx86_get_packet_vlan_vid(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->vlan(0))) return 0;
	
	return pack->headers->vlan(0)->get_dl_vlan_id()&0x1FFF;
}

uint8_t
dpx86_get_packet_vlan_pcp(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->vlan(0))) return 0;
	return pack->headers->vlan(0)->get_dl_vlan_pcp()&0x07;
}

//IP
uint8_t
dpx86_get_packet_ip_ecn(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return 0;

	return pack->headers->ipv4(0)->get_ipv4_ecn()&0xFF;
}

uint8_t
dpx86_get_packet_ip_dscp(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return 0;

	return pack->headers->ipv4(0)->get_ipv4_dscp()&0xFF;
}

uint8_t
dpx86_get_packet_ip_proto(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return 0;

	return pack->headers->ipv4(0)->get_ipv4_proto()&0xFF;
}

//IPv4
uint32_t
dpx86_get_packet_ipv4_src(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return 0;

	return be32toh(pack->headers->ipv4(0)->get_ipv4_src().ca_s4addr->sin_addr.s_addr);
}

uint32_t
dpx86_get_packet_ipv4_dst(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return 0;

	return be32toh(pack->headers->ipv4(0)->get_ipv4_dst().ca_s4addr->sin_addr.s_addr);
}

//TCP
uint16_t
dpx86_get_packet_tcp_dst(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->tcp(0))) return 0;
	return pack->headers->tcp(0)->get_dport();
}

uint16_t
dpx86_get_packet_tcp_src(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->tcp(0))) return 0;
	return pack->headers->tcp(0)->get_sport();
}

//UDP
uint16_t
dpx86_get_packet_udp_dst(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->udp(0))) return 0;
	return pack->headers->udp(0)->get_dport();
}

uint16_t
dpx86_get_packet_udp_src(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->udp(0))) return 0;
	return pack->headers->udp(0)->get_sport();
}

//ICMPv4
uint8_t
dpx86_get_packet_icmpv4_type(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv4(0))) return 0;
	return pack->headers->icmpv4(0)->get_icmp_type();
}

uint8_t
dpx86_get_packet_icmpv4_code(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv4(0))) return 0;
	return pack->headers->icmpv4(0)->get_icmp_code();
}

//MPLS-outermost label
uint32_t
dpx86_get_packet_mpls_label(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return 0;
	return pack->headers->mpls(0)->get_mpls_label()&0x000FFFFF;
}

uint8_t
dpx86_get_packet_mpls_tc(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return 0;
	return pack->headers->mpls(0)->get_mpls_tc()&0x07;
}

//PPPoE related extensions
uint8_t
dpx86_get_packet_pppoe_code(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->pppoe(0))) return 0;
	return pack->headers->pppoe(0)->get_pppoe_code();
}

uint8_t
dpx86_get_packet_pppoe_type(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->pppoe(0))) return 0;
	return pack->headers->pppoe(0)->get_pppoe_type()&0x0F;
}

uint16_t
dpx86_get_packet_pppoe_sid(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->pppoe(0))) return 0;
	return pack->headers->pppoe(0)->get_pppoe_sessid();
}

//PPP related extensions
uint16_t
dpx86_get_packet_ppp_proto(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ppp(0))) return 0;
	return pack->headers->ppp(0)->get_ppp_prot();
}

