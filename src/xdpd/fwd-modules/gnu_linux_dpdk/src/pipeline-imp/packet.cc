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

/*
* ROFL-Pipeline packet mangling platform API implementation
*/
#define FWD_MOD_NAME "example"

//Getters
uint32_t
platform_packet_get_size_bytes(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint32_t
platform_packet_get_port_in(datapacket_t * const pkt)
{
	return 0x0;
}

uint32_t
platform_packet_get_phy_port_in(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint64_t
platform_packet_get_eth_dst(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint64_t
platform_packet_get_eth_src(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint16_t
platform_packet_get_eth_type(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint16_t
platform_packet_get_vlan_vid(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint8_t
platform_packet_get_vlan_pcp(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint16_t
platform_packet_get_arp_opcode(datapacket_t * const pkt)
{

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0; 
}

uint64_t
platform_packet_get_arp_sha(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0; 
}

uint32_t
platform_packet_get_arp_spa(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0; 
}

uint64_t
platform_packet_get_arp_tha(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0; 
}

uint32_t
platform_packet_get_arp_tpa(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0; 
}


uint8_t
platform_packet_get_ip_ecn(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint8_t
platform_packet_get_ip_dscp(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}
uint8_t
platform_packet_get_ip_proto(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint32_t
platform_packet_get_ipv4_src(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint32_t
platform_packet_get_ipv4_dst(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint16_t
platform_packet_get_tcp_dst(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint16_t
platform_packet_get_tcp_src(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint16_t
platform_packet_get_udp_dst(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint16_t
platform_packet_get_udp_src(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint8_t
platform_packet_get_icmpv4_type(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint8_t
platform_packet_get_icmpv4_code(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint32_t
platform_packet_get_mpls_label(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint8_t
platform_packet_get_mpls_tc(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint8_t
platform_packet_get_pppoe_code(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint8_t
platform_packet_get_pppoe_type(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint16_t
platform_packet_get_pppoe_sid(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint16_t
platform_packet_get_ppp_proto(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}
uint8_t
platform_packet_get_gtp_msg_type(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

uint32_t
platform_packet_get_gtp_teid(datapacket_t * const pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	return 0x0;
}

//Actions
void
platform_packet_copy_ttl_in(datapacket_t* pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_pop_vlan(datapacket_t* pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_pop_mpls(datapacket_t* pkt, uint16_t ether_type)
{
	
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

void
platform_packet_pop_pppoe(datapacket_t* pkt, uint16_t ether_type)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_push_pppoe(datapacket_t* pkt, uint16_t ether_type)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_push_mpls(datapacket_t* pkt, uint16_t ether_type)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_push_vlan(datapacket_t* pkt, uint16_t ether_type)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_copy_ttl_out(datapacket_t* pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_dec_nw_ttl(datapacket_t* pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_dec_mpls_ttl(datapacket_t* pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_mpls_ttl(datapacket_t* pkt, uint8_t new_ttl)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_nw_ttl(datapacket_t* pkt, uint8_t new_ttl)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_queue(datapacket_t* pkt, uint32_t queue)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
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
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_eth_src(datapacket_t* pkt, uint64_t eth_src)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_eth_type(datapacket_t* pkt, uint16_t eth_type)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_vlan_vid(datapacket_t* pkt, uint16_t vlan_vid)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_vlan_pcp(datapacket_t* pkt, uint8_t vlan_pcp)
{
	
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

void
platform_packet_set_arp_opcode(datapacket_t* pkt, uint16_t arp_opcode)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

void
platform_packet_set_arp_sha(datapacket_t* pkt, uint64_t arp_sha)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

void
platform_packet_set_arp_spa(datapacket_t* pkt, uint32_t arp_spa)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

void
platform_packet_set_arp_tha(datapacket_t* pkt, uint64_t arp_tha)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

void
platform_packet_set_arp_tpa(datapacket_t* pkt, uint32_t arp_tpa)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

void
platform_packet_set_ip_dscp(datapacket_t* pkt, uint8_t ip_dscp)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_ip_ecn(datapacket_t* pkt, uint8_t ip_ecn)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_ip_proto(datapacket_t* pkt, uint8_t ip_proto)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_ipv4_src(datapacket_t* pkt, uint32_t ip_src)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_ipv4_dst(datapacket_t* pkt, uint32_t ip_dst)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_tcp_src(datapacket_t* pkt, uint16_t tcp_src)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_tcp_dst(datapacket_t* pkt, uint16_t tcp_dst)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_udp_src(datapacket_t* pkt, uint16_t udp_src)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_udp_dst(datapacket_t* pkt, uint16_t udp_dst)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_icmpv4_type(datapacket_t* pkt, uint8_t type)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_icmpv4_code(datapacket_t* pkt, uint8_t code)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_mpls_label(datapacket_t* pkt, uint32_t label)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_mpls_tc(datapacket_t* pkt, uint8_t tc)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_pppoe_type(datapacket_t* pkt, uint8_t type)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
	
}

void
platform_packet_set_pppoe_code(datapacket_t* pkt, uint8_t code)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

void
platform_packet_set_pppoe_sid(datapacket_t* pkt, uint16_t sid)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

void
platform_packet_set_ppp_proto(datapacket_t* pkt, uint16_t proto)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

void
platform_packet_set_gtp_msg_type(datapacket_t* pkt, uint8_t msg_type)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}

void
platform_packet_set_gtp_teid(datapacket_t* pkt, uint32_t teid)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
}


void
platform_packet_drop(datapacket_t* pkt)
{
	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);
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

	ROFL_INFO("["FWD_MOD_NAME"] calling %s()\n",__FUNCTION__);

	return NULL; 
}


