#include <inttypes.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/platform/packet.h>
#include <rofl/datapath/afa/openflow/openflow1x/of1x_cmm.h>
#include <rofl/datapath/pipeline/common/ipv6_exthdr.h>
#include <rofl/common/utils/c_logger.h>

//Frames
#include <rofl/common/protocols/fetherframe.h>
#include <rofl/common/protocols/fvlanframe.h>
#include <rofl/common/protocols/fmplsframe.h>
#include <rofl/common/protocols/farpv4frame.h>
#include <rofl/common/protocols/fipv4frame.h>
#include <rofl/common/protocols/ficmpv4frame.h>
#include <rofl/common/protocols/fipv6frame.h>
#include <rofl/common/protocols/ficmpv6frame.h>
#include <rofl/common/protocols/fudpframe.h>
#include <rofl/common/protocols/ftcpframe.h>
#include <rofl/common/protocols/fsctpframe.h>
#include <rofl/common/protocols/fpppoeframe.h>
#include <rofl/common/protocols/fpppframe.h>
#include <rofl/common/protocols/fgtpuframe.h>

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../io/datapacketx86.h"
#include "../io/bufferpool.h"
#include "../io/ports/ioport.h"

//Profiling
#include "../util/time_measurements.h"

using namespace rofl;
using namespace xdpd::gnu_linux;

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
}


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

bool
platform_packet_has_vlan(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack)
		return false;
	return (NULL != pack->headers->vlan(0));
}
uint16_t
platform_packet_get_vlan_vid(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->vlan(0))) return 0;
	
	return pack->headers->vlan(0)->get_dl_vlan_id()&0xFFF;
}

uint8_t
platform_packet_get_vlan_pcp(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->vlan(0))) return 0;
	return pack->headers->vlan(0)->get_dl_vlan_pcp()&0x07;
}

uint16_t
platform_packet_get_arp_opcode(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->arpv4(0))) return 0;
	return pack->headers->arpv4(0)->get_opcode();
}

uint64_t
platform_packet_get_arp_sha(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->arpv4(0))) return 0;
	return pack->headers->arpv4(0)->get_dl_src().get_mac();
}

uint32_t
platform_packet_get_arp_spa(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->arpv4(0))) return 0;
	return be32toh(pack->headers->arpv4(0)->get_nw_src().ca_s4addr->sin_addr.s_addr);
}

uint64_t
platform_packet_get_arp_tha(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->arpv4(0))) return 0;
	return pack->headers->arpv4(0)->get_dl_dst().get_mac();
}

uint32_t
platform_packet_get_arp_tpa(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->arpv4(0))) return 0;
	return be32toh(pack->headers->arpv4(0)->get_nw_dst().ca_s4addr->sin_addr.s_addr);
}

uint8_t
platform_packet_get_ip_ecn(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return 0;
	if (NULL != pack->headers->ipv4(0))
		return pack->headers->ipv4(0)->get_ipv4_ecn()&0xFF;
	if (NULL != pack->headers->ipv6(0))
		return pack->headers->ipv6(0)->get_ecn();
	return 0;
}

uint8_t
platform_packet_get_ip_dscp(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return 0;
	if (NULL != pack->headers->ipv4(0))
		return pack->headers->ipv4(0)->get_ipv4_dscp()&0xFF;
	if (NULL != pack->headers->ipv6(0))
		return pack->headers->ipv6(0)->get_dscp();
	return 0;
}

uint8_t
platform_packet_get_ip_proto(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return 0;
	if (NULL != pack->headers->ipv4(0))
		return pack->headers->ipv4(0)->get_ipv4_proto()&0xFF;
	if (NULL != pack->headers->ipv6(0))
		return pack->headers->ipv6(0)->get_next_header()&0xFF;
	return 0;
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

uint16_t platform_packet_get_sctp_dst(datapacket_t *const pkt){
	//TODO: add implementation when supported
	return 0x0;
}
uint16_t platform_packet_get_sctp_src(datapacket_t *const pkt){
	//TODO: add implementation when supported
	return 0x0;
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

uint128__t
platform_packet_get_ipv6_src(datapacket_t * const pkt)
{
	uint128__t ipv6_src = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}; //memset 0?
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if((NULL==pkt) || (NULL == pack->headers->ipv6(0))) return ipv6_src;
	ipv6_src = pack->headers->ipv6(0)->get_ipv6_src().get_ipv6_addr();
	return ipv6_src;
}

uint128__t
platform_packet_get_ipv6_dst(datapacket_t * const pkt)
{
	uint128__t ipv6_dst = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}; //memset 0?
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if((NULL==pkt) || (NULL == pack->headers->ipv6(0))) return ipv6_dst;
	ipv6_dst = pack->headers->ipv6(0)->get_ipv6_dst().get_ipv6_addr();
	return ipv6_dst;
}

uint64_t
platform_packet_get_ipv6_flabel(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if((NULL==pkt) || (NULL == pack->headers->ipv6(0))) return 0;
	return pack->headers->ipv6(0)->get_flow_label();
}

uint128__t
platform_packet_get_ipv6_nd_target(datapacket_t * const pkt)
{
	uint128__t ipv6_nd_target = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}; //memset 0?
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if((NULL==pkt) || (NULL == pack->headers->icmpv6(0))) return ipv6_nd_target;
	ipv6_nd_target = pack->headers->icmpv6(0)->get_icmpv6_neighbor_taddr().get_ipv6_addr();
	return ipv6_nd_target;
}

uint64_t
platform_packet_get_ipv6_nd_sll(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if((NULL==pkt) || (NULL == pack->headers->icmpv6(0))) return 0;
	try{
		return pack->headers->icmpv6(0)->get_option(ficmpv6opt::ICMPV6_OPT_LLADDR_SOURCE).get_ll_saddr().get_mac();
	}catch(...){
		return 0;
	}
}

uint64_t
platform_packet_get_ipv6_nd_tll(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if((NULL==pkt) || (NULL == pack->headers->icmpv6(0))) return 0;
	try{
		return pack->headers->icmpv6(0)->get_option(ficmpv6opt::ICMPV6_OPT_LLADDR_TARGET).get_ll_taddr().get_mac();
	}catch(...)	{
		return 0;
	}
}

uint16_t
platform_packet_get_ipv6_exthdr(datapacket_t * const pkt)
{
	uint64_t mask=0x0;
	/* TODO EXTENSION HEADERS NOT YET IMPLEMENTED	
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if((NULL==pkt) || (NULL == pack->headers->ipv6(0))) return 0;
	//return pack->headers->ipv6(0)->get_ipv6_ext_hdr();
	try{
		pack->headers->ipv6(0)->get_ext_hdr(fipv6frame::IPPROTO_IPV6_NONXT);
		mask |= IPV6_EH_NONEXT;
	}catch(...){}
	
	//if(pack->headers->ipv6(0)->get_ext_hdr(fipv6frame::IPPROTO_IPV6_NONXT)) //encripted
	//if(pack->headers->ipv6(0)->get_ext_hdr(fipv6frame::IPPROTO_IPV6_NONXT)) //authentication
	//if(pack->headers->ipv6(0)->get_ext_hdr(fipv6frame::IPPROTO_IPV6_NONXT)) //destination headers
	try{
		pack->headers->ipv6(0)->get_ext_hdr(fipv6frame::IPPROTO_IPV6_FRAG);
		mask |= IPV6_EH_FRAG;
	}catch(...){}
	
	try{
		pack->headers->ipv6(0)->get_ext_hdr(fipv6frame::IPPROTO_IPV6_ROUTE);
		mask |= IPV6_EH_ROUTER;
	}catch(...){}
	
	try{
		pack->headers->ipv6(0)->get_ext_hdr(fipv6frame::IPPROTO_IPV6_HOPOPT);
		mask |= IPV6_EH_HOP;
	}catch(...){}
	//if(pack->headers->ipv6(0)->get_ipv6_ext_hdr(fipv6frame::IPPROTO_IPV6_NONXT)) //unexpected repeats
	//if(pack->headers->ipv6(0)->get_ipv6_ext_hdr(fipv6frame::IPPROTO_IPV6_NONXT)) //unexpected sequencing
	*/
	return mask;
}

uint8_t
platform_packet_get_icmpv6_type(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if((NULL==pkt) || (NULL == pack->headers->icmpv6(0))) return 0;
	return pack->headers->icmpv6(0)->get_icmpv6_type();
}

uint8_t
platform_packet_get_icmpv6_code(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if((NULL==pkt) || (NULL == pack->headers->icmpv6(0))) return 0;
	return pack->headers->icmpv6(0)->get_icmpv6_code();
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

bool
platform_packet_get_mpls_bos(datapacket_t * const pkt)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return 0;
	return pack->headers->mpls(0)->get_mpls_bos()&0x01;
}

uint32_t platform_packet_get_pbb_isid(datapacket_t *const pkt){
	//TODO: add implementation when supported
	return 0x0;
}

//Tunnel id
uint64_t platform_packet_get_tunnel_id(datapacket_t *const pkt){
	//TODO: add implementation when supported
	return 0x0ULL;
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
	
	TM_STAMP_STAGE(pkt, TM_S5);
	
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
	if (NULL == pack)
		return;
	if(NULL != pack->headers->ipv4(0)){
		pack->headers->ipv4(0)->dec_ipv4_ttl();
		pack->ipv4_recalc_checksum = true;
	}
	if(NULL != pack->headers->ipv6(0)){
		// TODO IPv6
		//pack->headers->ipv6(0)->dec_ipv6_ttl();
	}
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
	if (NULL == pack) return;
	if (NULL != pack->headers->ipv4(0)){
		pack->headers->ipv4(0)->set_ipv4_ttl(new_ttl);
		pack->ipv4_recalc_checksum = true;
	}
	if (NULL != pack->headers->ipv6(0)){
		// TODO IPv6
		//pack->headers->ipv6(0)->set_ipv6_ttl(new_ttl);
	}
}

void
platform_packet_set_queue(datapacket_t* pkt, uint32_t queue)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;

	pack->output_queue = queue;	
}

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

void
platform_packet_set_arp_opcode(datapacket_t* pkt, uint16_t arp_opcode)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->arpv4(0))) return;
	pack->headers->arpv4(0)->set_opcode(arp_opcode);
}

void
platform_packet_set_arp_sha(datapacket_t* pkt, uint64_t arp_sha)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->arpv4(0))) return;
	pack->headers->arpv4(0)->set_dl_src(arp_sha);
}

void
platform_packet_set_arp_spa(datapacket_t* pkt, uint32_t arp_spa)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->arpv4(0))) return;
	pack->headers->arpv4(0)->set_nw_src(arp_spa);
}

void
platform_packet_set_arp_tha(datapacket_t* pkt, uint64_t arp_tha)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->arpv4(0))) return;
	pack->headers->arpv4(0)->set_dl_dst(arp_tha);
}

void
platform_packet_set_arp_tpa(datapacket_t* pkt, uint32_t arp_tpa)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->arpv4(0))) return;
	pack->headers->arpv4(0)->set_nw_dst(arp_tpa);
}

void
platform_packet_set_ip_dscp(datapacket_t* pkt, uint8_t ip_dscp)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	if (NULL != pack->headers->ipv4(0)) {
		pack->headers->ipv4(0)->set_ipv4_dscp(ip_dscp);
		pack->ipv4_recalc_checksum = true;
	}
	if (NULL != pack->headers->ipv6(0)) {
		pack->headers->ipv6(0)->set_dscp(ip_dscp);
	}
}

void
platform_packet_set_ip_ecn(datapacket_t* pkt, uint8_t ip_ecn)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	if (NULL != pack->headers->ipv4(0)){
		pack->headers->ipv4(0)->set_ipv4_ecn(ip_ecn);
		pack->ipv4_recalc_checksum = true;
	}
	if (NULL != pack->headers->ipv6(0)){
		pack->headers->ipv6(0)->set_ecn(ip_ecn);
	}
}

void
platform_packet_set_ip_proto(datapacket_t* pkt, uint8_t ip_proto)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if (NULL == pack) return;
	if (NULL != pack->headers->ipv4(0)) {
		pack->headers->ipv4(0)->set_ipv4_proto(ip_proto);
		pack->ipv4_recalc_checksum = true;
	}
	if (NULL != pack->headers->ipv6(0)) {
		// TODO IPv6
		//pack->headers->ipv6(0)->set_ipv6_proto(ip_proto);
	}
}

void
platform_packet_set_ipv4_src(datapacket_t* pkt, uint32_t ip_src)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	pack->headers->ipv4(0)->set_ipv4_src(ip_src);
	pack->ipv4_recalc_checksum = true;
	pack->tcp_recalc_checksum = true;
	pack->udp_recalc_checksum = true;
}

void
platform_packet_set_ipv4_dst(datapacket_t* pkt, uint32_t ip_dst)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv4(0))) return;
	pack->headers->ipv4(0)->set_ipv4_dst(ip_dst);
	pack->ipv4_recalc_checksum = true;
	pack->tcp_recalc_checksum = true;
	pack->udp_recalc_checksum = true;
}

void
platform_packet_set_ipv6_src(datapacket_t* pkt, uint128__t ipv6_src)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv6(0))) return;
	pack->headers->ipv6(0)->set_ipv6_src((uint8_t*)&ipv6_src.val,16);
	pack->tcp_recalc_checksum = true;
	pack->udp_recalc_checksum = true;
}

void
platform_packet_set_ipv6_dst(datapacket_t* pkt, uint128__t ipv6_dst)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv6(0))) return;
	pack->headers->ipv6(0)->set_ipv6_dst((uint8_t*)&ipv6_dst.val,16);
	pack->tcp_recalc_checksum = true;
	pack->udp_recalc_checksum = true;
}

void
platform_packet_set_ipv6_flabel(datapacket_t* pkt, uint64_t ipv6_flabel)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ipv6(0))) return;
	pack->headers->ipv6(0)->set_flow_label(ipv6_flabel);
}

void
platform_packet_set_ipv6_nd_target(datapacket_t* pkt, uint128__t ipv6_nd_target)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv6(0))) return;
	caddress addr(AF_INET6,"0:0:0:0:0");
	addr.set_ipv6_addr(ipv6_nd_target);
	pack->headers->icmpv6(0)->set_icmpv6_neighbor_taddr(addr);
}

void
platform_packet_set_ipv6_nd_sll(datapacket_t* pkt, uint64_t ipv6_nd_sll)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv6(0))) return;
	cmacaddr mac(ipv6_nd_sll);
	pack->headers->icmpv6(0)->get_option(ficmpv6opt::ICMPV6_OPT_LLADDR_SOURCE).set_ll_saddr(mac);
}

void
platform_packet_set_ipv6_nd_tll(datapacket_t* pkt, uint64_t ipv6_nd_tll)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv6(0))) return;
	cmacaddr mac(ipv6_nd_tll);
	pack->headers->icmpv6(0)->get_option(ficmpv6opt::ICMPV6_OPT_LLADDR_TARGET).set_ll_taddr(mac);
}

void
platform_packet_set_ipv6_exthdr(datapacket_t* pkt, uint16_t ipv6_exthdr)
{
	/*TODO Extension headers not yet implemented*/
}

void
platform_packet_set_icmpv6_type(datapacket_t* pkt, uint8_t icmpv6_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv6(0))) return;
	pack->headers->icmpv6(0)->set_icmpv6_type(icmpv6_type);
}

void
platform_packet_set_icmpv6_code(datapacket_t* pkt, uint8_t icmpv6_code)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->icmpv6(0))) return;
	pack->headers->icmpv6(0)->set_icmpv6_code(icmpv6_code);
}

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

void
platform_packet_set_sctp_src(datapacket_t* pkt, uint16_t sctp_src)
{
	//TODO: implement
}

void
platform_packet_set_sctp_dst(datapacket_t* pkt, uint16_t sctp_dst)
{
	//TODO: implement
}

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
void
platform_packet_set_mpls_bos(datapacket_t* pkt, bool bos)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->mpls(0))) return;
	pack->headers->mpls(0)->set_mpls_bos(bos);
}
void platform_packet_set_pbb_isid(datapacket_t*pkt, uint32_t pbb_isid)
{
	//TODO: implement
}
void platform_packet_set_tunnel_id(datapacket_t*pkt, uint64_t tunnel_id)
{
	//TODO: implement
}
void platform_packet_pop_pbb(datapacket_t* pkt, uint16_t ether_type)
{
	//TODO: implement
}
void platform_packet_push_pbb(datapacket_t* pkt, uint16_t ether_type)
{
	//TODO: implement
}
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

void
platform_packet_set_ppp_proto(datapacket_t* pkt, uint16_t proto)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->ppp(0))) return;
	pack->headers->ppp(0)->set_ppp_prot(proto);
}

void
platform_packet_set_gtp_msg_type(datapacket_t* pkt, uint8_t msg_type)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->gtp(0))) return;
	pack->headers->gtp(0)->set_msg_type(msg_type);
}

void
platform_packet_set_gtp_teid(datapacket_t* pkt, uint32_t teid)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	if ((NULL == pack) || (NULL == pack->headers->gtp(0))) return;
	pack->headers->gtp(0)->set_teid(teid);
}

void platform_packet_pop_gtp(datapacket_t* pkt)
{
	//TODO: implement
}
void platform_packet_push_gtp(datapacket_t* pkt)
{
	//TODO: implement
}


void
platform_packet_drop(datapacket_t* pkt)
{
	ROFL_DEBUG("Dropping packet(%p)\n",pkt);
	
	//Release buffer
	bufferpool::release_buffer(pkt);

}

static void output_single_packet(datapacket_t* pkt, datapacketx86* pack, switch_port_t* port){

	//Output packet to the appropiate queue and port_num
	if(likely(port && port->platform_port_state) && port->up && port->forward_packets){
		
		ROFL_DEBUG("[%s] OUTPUT packet(%p)\n", port->name, pkt);
#ifdef DEBUG
		of1x_dump_packet_matches(&pkt->matches);
#endif

		TM_STAMP_STAGE(pkt, TM_SA6_PRE);
		
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

/**
* Output packet to the port(s)
* The action HAS to implement the destruction/release of the pkt
* (including if the pkt is a replica).
*
* If a flooding output actions needs to be done, the function
* has itself to deal with packet replication.
*/
void platform_packet_output(datapacket_t* pkt, switch_port_t* output_port){

	of_switch_t const* sw;
	datapacketx86* pack;

	if(!output_port){
		assert(0);
		return;
	}

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

	//Outer most IPv4 frame
	fipv4frame *fipv4 = pack->headers->ipv4(0);

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
	if(output_port == flood_meta_port || output_port == all_meta_port){ //We don't have STP, so it is the same
		datapacket_t* replica;
		switch_port_t* port_it;
		datapacketx86* replica_pack;

		//Get switch
		sw = pkt->sw;	
		
		if(unlikely(!sw)){
			bufferpool::release_buffer(pkt);
			return;
		}
	
		//We need to flood
		for(unsigned i=0;i<LOGICAL_SWITCH_MAX_LOG_PORTS;++i){

			port_it = sw->logical_ports[i].port;

			//Check port is not incomming port, exists, and is up 
			if( (i == pack->in_port) || !port_it || port_it->no_flood)
				continue;

			//replicate packet
			replica = platform_packet_replicate(pkt); 	
			replica_pack = (datapacketx86*) (replica->platform_state);

			ROFL_DEBUG("[%s] OUTPUT FLOOD packet(%p), origin(%p)\n", port_it->name, replica, pkt);
			
			//send the replica
			output_single_packet(replica, replica_pack, port_it);
		}

#ifdef DEBUG
		of1x_dump_packet_matches(&pkt->matches);
#endif
			
		//discard the original packet always (has been replicated)
		bufferpool::release_buffer(pkt);
	}else if(output_port == in_port_meta_port){
		
		//In port
		switch_port_t* port;
		sw = pkt->sw;	

		if(unlikely(pack->in_port >= LOGICAL_SWITCH_MAX_LOG_PORTS)){
			assert(0);
			return;
		}

		port = sw->logical_ports[pack->in_port].port;
		if( unlikely(port == NULL)){
			assert(0);
			return;
		
		}
	
		//Send to the incomming port 
		output_single_packet(pkt, pack, port);
	}else{
		//Single output	
		output_single_packet(pkt, pack, output_port);
	}

}
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
	datapacket_t* copy = bufferpool::get_free_buffer(); //FIXME: this is sync!
	
	//Make sure everything is memseted to 0
	memcpy(&copy->matches, &pkt->matches, sizeof(pkt->matches));
	memcpy(&copy->write_actions, &pkt->write_actions ,sizeof(pkt->write_actions));

	//mark as replica
	copy->is_replica = true;
	copy->sw = pkt->sw;

	//Clone contents
	clone_pkt_contents(pkt,copy);
	return copy;	
}


