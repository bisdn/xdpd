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

#include "../io/datapacketx86_c_wrapper.h"
#include "../io/bufferpool_c_wrapper.h"

#include "../processing/ls_internal_state.h"
#include "../io/datapacket_storage_c_wrapper.h"

/*
* ROFL-Pipeline packet mangling platform API implementation
*/

//Getters
uint32_t
platform_packet_get_size_bytes(datapacket_t * const pkt)
{
	return dpx86_get_packet_size(pkt);
}

uint32_t
platform_packet_get_port_in(datapacket_t * const pkt)
{
	return dpx86_get_packet_port_in(pkt);
}

uint32_t
platform_packet_get_phy_port_in(datapacket_t * const pkt)
{
	return dpx86_get_packet_phy_port_in(pkt);
}

uint64_t
platform_packet_get_eth_dst(datapacket_t * const pkt)
{
	return dpx86_get_packet_eth_dst(pkt);
}

uint64_t
platform_packet_get_eth_src(datapacket_t * const pkt)
{
	return dpx86_get_packet_eth_src(pkt);
}

uint16_t
platform_packet_get_eth_type(datapacket_t * const pkt)
{
	return dpx86_get_packet_eth_type(pkt);
}

uint16_t
platform_packet_get_vlan_vid(datapacket_t * const pkt)
{
	return dpx86_get_packet_vlan_vid(pkt);
}

uint8_t
platform_packet_get_vlan_pcp(datapacket_t * const pkt)
{
	return dpx86_get_packet_vlan_pcp(pkt);
}

uint8_t
platform_packet_get_ip_ecn(datapacket_t * const pkt)
{
	return dpx86_get_packet_ip_ecn(pkt);
}

uint8_t
platform_packet_get_ip_dscp(datapacket_t * const pkt)
{
	return dpx86_get_packet_ip_dscp(pkt);
}
uint8_t
platform_packet_get_ip_proto(datapacket_t * const pkt)
{
	return dpx86_get_packet_ip_proto(pkt);
}

uint32_t
platform_packet_get_ipv4_src(datapacket_t * const pkt)
{
	return dpx86_get_packet_ipv4_src(pkt);
}

uint32_t
platform_packet_get_ipv4_dst(datapacket_t * const pkt)
{
	return dpx86_get_packet_ipv4_dst(pkt);
}

uint16_t
platform_packet_get_tcp_dst(datapacket_t * const pkt)
{
	return dpx86_get_packet_tcp_dst(pkt);
}

uint16_t
platform_packet_get_tcp_src(datapacket_t * const pkt)
{
	return dpx86_get_packet_tcp_src(pkt);
}

uint16_t
platform_packet_get_udp_dst(datapacket_t * const pkt)
{
	return dpx86_get_packet_udp_dst(pkt);
}

uint16_t
platform_packet_get_udp_src(datapacket_t * const pkt)
{
	return dpx86_get_packet_udp_src(pkt);
}

uint8_t
platform_packet_get_icmpv4_type(datapacket_t * const pkt)
{
	return dpx86_get_packet_icmpv4_type(pkt);
}

uint8_t
platform_packet_get_icmpv4_code(datapacket_t * const pkt)
{
	return dpx86_get_packet_icmpv4_code(pkt);
}

uint32_t
platform_packet_get_mpls_label(datapacket_t * const pkt)
{
	return dpx86_get_packet_mpls_label(pkt);
}

uint8_t
platform_packet_get_mpls_tc(datapacket_t * const pkt)
{
	return dpx86_get_packet_mpls_tc(pkt);
}

uint8_t
platform_packet_get_pppoe_code(datapacket_t * const pkt)
{
	return dpx86_get_packet_pppoe_code(pkt);
}

uint8_t
platform_packet_get_pppoe_type(datapacket_t * const pkt)
{
	return dpx86_get_packet_pppoe_type(pkt);
}

uint16_t
platform_packet_get_pppoe_sid(datapacket_t * const pkt)
{
	return dpx86_get_packet_pppoe_sid(pkt);
}

uint16_t
platform_packet_get_ppp_proto(datapacket_t * const pkt)
{
	return dpx86_get_packet_ppp_proto(pkt);
}


//Actions
void
platform_packet_copy_ttl_in(datapacket_t* pkt)
{
	dpx86_copy_ttl_in(pkt);
}

void
platform_packet_pop_vlan(datapacket_t* pkt)
{
	dpx86_pop_vlan(pkt);
}

void
platform_packet_pop_mpls(datapacket_t* pkt, uint16_t ether_type)
{
	dpx86_pop_mpls(pkt, ether_type);
}

void
platform_packet_pop_pppoe(datapacket_t* pkt, uint16_t ether_type)
{
	dpx86_pop_pppoe(pkt, ether_type);
}

void
platform_packet_push_pppoe(datapacket_t* pkt, uint16_t ether_type)
{
	dpx86_push_pppoe(pkt, ether_type);
}

void
platform_packet_push_mpls(datapacket_t* pkt, uint16_t ether_type)
{
	dpx86_push_mpls(pkt, ether_type);
}

void
platform_packet_push_vlan(datapacket_t* pkt, uint16_t ether_type)
{
	dpx86_push_vlan(pkt, ether_type);
}

void
platform_packet_copy_ttl_out(datapacket_t* pkt)
{
	dpx86_copy_ttl_out(pkt);
}

void
platform_packet_dec_nw_ttl(datapacket_t* pkt)
{
	dpx86_dec_nw_ttl(pkt);
}

void
platform_packet_dec_mpls_ttl(datapacket_t* pkt)
{
	dpx86_dec_mpls_ttl(pkt);
}

void
platform_packet_set_mpls_ttl(datapacket_t* pkt, uint8_t new_ttl)
{
	dpx86_set_mpls_ttl(pkt, new_ttl);
}

void
platform_packet_set_nw_ttl(datapacket_t* pkt, uint8_t new_ttl)
{
	dpx86_set_nw_ttl(pkt, new_ttl);
}

void
platform_packet_set_queue(datapacket_t* pkt, uint32_t queue)
{
	dpx86_set_queue(pkt, queue);
}

#if HAVE_METADATA_PROCESSING // todo this has to be implemented
void
platform_packet_set_metadata(datapacket_t* pkt, uint64_t metadata)
{

}
#endif

void
platform_packet_set_eth_dst(datapacket_t* pkt, uint64_t eth_dst)
{
	dpx86_set_eth_dst(pkt, eth_dst);
}

void
platform_packet_set_eth_src(datapacket_t* pkt, uint64_t eth_src)
{
	dpx86_set_eth_src(pkt, eth_src);
}

void
platform_packet_set_eth_type(datapacket_t* pkt, uint16_t eth_type)
{
	dpx86_set_eth_type(pkt, eth_type);
}

void
platform_packet_set_vlan_vid(datapacket_t* pkt, uint16_t vlan_vid)
{
	dpx86_set_vlan_vid(pkt, vlan_vid);
}

void
platform_packet_set_vlan_pcp(datapacket_t* pkt, uint8_t vlan_pcp)
{
	dpx86_set_vlan_pcp(pkt, vlan_pcp);
}

void
platform_packet_set_ip_dscp(datapacket_t* pkt, uint8_t ip_dscp)
{
	dpx86_set_ip_dscp(pkt, ip_dscp);
}

void
platform_packet_set_ip_ecn(datapacket_t* pkt, uint8_t ip_ecn)
{
	dpx86_set_ip_ecn(pkt, ip_ecn);
}

void
platform_packet_set_ip_proto(datapacket_t* pkt, uint8_t ip_proto)
{
	dpx86_set_ip_proto(pkt, ip_proto);
}

void
platform_packet_set_ipv4_src(datapacket_t* pkt, uint32_t ip_src)
{
	dpx86_set_ipv4_src(pkt, ip_src);
}

void
platform_packet_set_ipv4_dst(datapacket_t* pkt, uint32_t ip_dst)
{
	dpx86_set_ipv4_dst(pkt, ip_dst);
}

void
platform_packet_set_tcp_src(datapacket_t* pkt, uint16_t tcp_src)
{
	dpx86_set_tcp_src(pkt, tcp_src);
}

void
platform_packet_set_tcp_dst(datapacket_t* pkt, uint16_t tcp_dst)
{
	dpx86_set_tcp_dst(pkt, tcp_dst);
}

void
platform_packet_set_udp_src(datapacket_t* pkt, uint16_t udp_src)
{
	dpx86_set_udp_src(pkt, udp_src);
}

void
platform_packet_set_udp_dst(datapacket_t* pkt, uint16_t udp_dst)
{
	dpx86_set_udp_dst(pkt, udp_dst);
}

void
platform_packet_set_icmpv4_type(datapacket_t* pkt, uint8_t type)
{
	dpx86_set_icmpv4_type(pkt, type);
}

void
platform_packet_set_icmpv4_code(datapacket_t* pkt, uint8_t code)
{
	dpx86_set_icmpv4_code(pkt, code);
}

void
platform_packet_set_mpls_label(datapacket_t* pkt, uint32_t label)
{
	dpx86_set_mpls_label(pkt, label);
}

void
platform_packet_set_mpls_tc(datapacket_t* pkt, uint8_t tc)
{
	dpx86_set_mpls_tc(pkt, tc);
}

void
platform_packet_set_pppoe_type(datapacket_t* pkt, uint8_t type)
{
	dpx86_set_pppoe_type(pkt, type);
}

void
platform_packet_set_pppoe_code(datapacket_t* pkt, uint8_t code)
{
	dpx86_set_pppoe_code(pkt, code);
}

void
platform_packet_set_pppoe_sid(datapacket_t* pkt, uint16_t sid)
{
	dpx86_set_pppoe_sid(pkt, sid);
}

void
platform_packet_set_ppp_proto(datapacket_t* pkt, uint16_t proto)
{
	dpx86_set_ppp_proto(pkt, proto);
}

void
platform_packet_drop(datapacket_t* pkt)
{
	ROFL_DEBUG("Dropping packet(%p)\n",pkt);
	
	//Release buffer
	bufferpool_release_buffer_wrapper(pkt);

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

	//Handle correctly the output 
	dpx86_output_packet(pkt, port);
	
	
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
	datapacket_t* copy = bufferpool_get_buffer_wrapper();
	
	//Make sure everything is memseted to 0
	memcpy(&copy->matches, &pkt->matches, sizeof(pkt->matches));
	memcpy(&copy->write_actions, &pkt->write_actions ,sizeof(pkt->write_actions));

	//mark as replica
	copy->is_replica = true;

	//Clone contents
	dpx86_clone_pkt_contents(pkt,copy);
	return copy;	
}


