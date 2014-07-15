//Guards used only when inlining
#ifndef PACKET_IMPL_INLINE__
#define PACKET_IMPL_INLINE__

//Must be the first one
#include <inttypes.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/platform/packet.h>
#include <rofl/datapath/hal/openflow/openflow1x/of1x_cmm.h>
#include <rofl/datapath/pipeline/common/ipv6_exthdr.h>
#include <rofl/common/utils/c_logger.h>

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "../io/datapacketx86.h"
#include "../io/bufferpool.h"
#include "../io/ports/ioport.h"

#include "../io/packet_classifiers/pktclassifier.h"


//
// Configuration to include packet_inline_classifier.h 
//

#define GET_CLAS_STATE_PTR(PKT)\
	((xdpd::gnu_linux::datapacketx86*) PKT ->platform_state)

#include "packet_inline_classifier.h"

//
// Driver specific functions
//

using namespace xdpd::gnu_linux;

/* Cloning of the packet */
STATIC_PACKET_INLINE__
void clone_pkt_contents(datapacket_t* src, datapacket_t* dst){

	//Initialize buffer
	datapacketx86 *pack_src = (datapacketx86*)src->platform_state;
	datapacketx86 *pack_dst = (datapacketx86*)dst->platform_state;
	
	//Initialize the buffer and copy but do not classify
	pack_dst->init(pack_src->get_buffer(), pack_src->get_buffer_length(), pack_src->lsw, pack_src->clas_state.port_in, pack_src->clas_state.phy_port_in, false, true);

	//Copy classification state and 
	pack_dst->output_queue = pack_src->output_queue;
	pack_dst->clas_state = pack_src->clas_state;
}

STATIC_PACKET_INLINE__
void platform_packet_set_queue(datapacket_t* pkt, uint32_t queue)
{
	datapacketx86 *pack = (datapacketx86*)pkt->platform_state;
	pack->output_queue = queue;	
}

STATIC_PACKET_INLINE__
void platform_packet_drop(datapacket_t* pkt)
{
	ROFL_DEBUG(DRIVER_NAME"[pkt] Dropping packet(%p)\n",pkt);
	
	//Release buffer
	bufferpool::release_buffer(pkt);

}

STATIC_PACKET_INLINE__
void output_single_packet(datapacket_t* pkt, datapacketx86* pack, switch_port_t* port){

	//Output packet to the appropiate queue and port_num
	if(likely(port && port->platform_port_state) && port->up && port->forward_packets){
		
		ROFL_DEBUG(DRIVER_NAME"[pkt][%s] OUTPUT packet(%p)\n", port->name, pkt);
#ifdef DEBUG
		dump_packet_matches(pkt, false);
#endif

		TM_STAMP_STAGE(pkt, TM_SA5_PRE);
		
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
* Creates a copy (in heap) of the datapacket_t structure including any
* platform specific state (->platform_state). The following behaviour
* is expected from this hook:
* 
* - All data fields and pointers of datapacket_t struct must be memseted to 0, except:
* - datapacket_t flag is_replica must be set to true
* - platform_state, if used, must be replicated (copied) otherwise NULL
*
*/
STATIC_PACKET_INLINE__
datapacket_t* platform_packet_replicate(datapacket_t* pkt){

	//Get a free buffer
	datapacket_t* copy = bufferpool::get_free_buffer_nonblocking();
	
	if(!copy){
		return copy;
		return NULL;
	}
	
	//Make sure everything is memseted to 0
	memcpy(&copy->write_actions, &pkt->write_actions ,sizeof(pkt->write_actions));

	//mark as replica
	copy->is_replica = true;
	copy->sw = pkt->sw;

	//Clone contents
	clone_pkt_contents(pkt,copy);
	return copy;	
}


/**
* Output packet to the port(s)
* The action HAS to implement the destruction/release of the pkt
* (including if the pkt is a replica).
*
* If a flooding output actions needs to be done, the function
* has itself to deal with packet replication.
*/
STATIC_PACKET_INLINE__
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
	if(pack->clas_state.ipv4_recalc_checksum){
		if(get_ipv4_hdr(&pack->clas_state, 0))	
			ipv4_calc_checksum(get_ipv4_hdr(&pack->clas_state, 0));
	}

	//Outer most IPv4 frame
	void *fipv4 = get_ipv4_hdr(&pack->clas_state, 0);

	if (NULL != fipv4) {
		if ((pack->clas_state.tcp_recalc_checksum) && get_tcp_hdr(&pack->clas_state, 0) && fipv4) {

			tcpv4_calc_checksum(
					get_tcp_hdr(&pack->clas_state, 0),
					*get_ipv4_src(fipv4),
					*get_ipv4_dst(fipv4),
					*get_ipv4_proto(fipv4),
					get_pkt_len(pkt, &pack->clas_state, get_tcp_hdr(&pack->clas_state,0), NULL) ); // start at innermost IPv4 up to and including last frame

		} else if ((pack->clas_state.udp_recalc_checksum) && (get_udp_hdr(&pack->clas_state, 0)) && fipv4) {

			udpv4_calc_checksum(
					get_udp_hdr(&pack->clas_state, 0),
					*get_ipv4_src(fipv4),
					*get_ipv4_dst(fipv4),
					*get_ipv4_proto(fipv4),
					get_pkt_len(pkt, &pack->clas_state, get_udp_hdr(&pack->clas_state, 0), NULL) ); // start at innermost IPv4 up to and including last frame

		} else if ((pack->clas_state.icmpv4_recalc_checksum) && (get_icmpv4_hdr(&pack->clas_state,0))) {

			icmpv4_calc_checksum(
				get_icmpv4_hdr(&pack->clas_state, 0),
				get_pkt_len(pkt, &pack->clas_state, get_icmpv4_hdr(&pack->clas_state, 0), NULL) );
		}
	}

	//Outer most IPv6 frame
	void *fipv6 = get_ipv6_hdr(&pack->clas_state, 0);

	if (NULL != fipv6) {
		if ((pack->clas_state.tcp_recalc_checksum) && get_tcp_hdr(&pack->clas_state, 0) && fipv6) {

			tcpv6_calc_checksum(
					get_tcp_hdr(&pack->clas_state, 0),
					*get_ipv6_src(fipv6),
					*get_ipv6_dst(fipv6),
					TCP_IP_PROTO,
					get_pkt_len(pkt, &pack->clas_state, get_tcp_hdr(&pack->clas_state,0), NULL) ); // start at innermost IPv6 up to and including last frame

		} else if ((pack->clas_state.udp_recalc_checksum) && (get_udp_hdr(&pack->clas_state, 0)) && fipv6) {

			udpv6_calc_checksum(
					get_udp_hdr(&pack->clas_state, 0),
					*get_ipv6_src(fipv6),
					*get_ipv6_dst(fipv6),
					UDP_IP_PROTO,
					get_pkt_len(pkt, &pack->clas_state, get_udp_hdr(&pack->clas_state, 0), NULL) ); // start at innermost IPv6 up to and including last frame

		} else if ((pack->clas_state.icmpv6_recalc_checksum) && (get_icmpv6_hdr(&pack->clas_state,0))) {

			icmpv6_calc_checksum(
					get_icmpv6_hdr(&pack->clas_state, 0),
					*get_ipv6_src(fipv6),
					*get_ipv6_dst(fipv6),
					ICMPV6_IP_PROTO,
					get_pkt_len(pkt, &pack->clas_state, get_icmpv6_hdr(&pack->clas_state, 0), NULL) );
		}
	}

	if ((pack->clas_state.sctp_recalc_checksum) && get_sctp_hdr(&pack->clas_state, 0)) {

			sctp_calc_checksum(
					get_sctp_hdr(&pack->clas_state, 0),
					get_pkt_len(pkt, &pack->clas_state, get_sctp_hdr(&pack->clas_state,0), NULL) );
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
			if( (i == pack->clas_state.port_in) || !port_it || port_it->no_flood)
				continue;

			//replicate packet
			replica = platform_packet_replicate(pkt); 	
			replica_pack = (datapacketx86*) (replica->platform_state);

			ROFL_DEBUG(DRIVER_NAME"[pkt][%s] OUTPUT FLOOD packet(%p), origin(%p)\n", port_it->name, replica, pkt);
			
			//send the replica
			output_single_packet(replica, replica_pack, port_it);
		}

#ifdef DEBUG
		dump_packet_matches(pkt, false);
#endif
			
		//discard the original packet always (has been replicated)
		bufferpool::release_buffer(pkt);
	}else if(output_port == in_port_meta_port){
		
		//In port
		switch_port_t* port;
		sw = pkt->sw;	

		if(unlikely(pack->clas_state.port_in >= LOGICAL_SWITCH_MAX_LOG_PORTS)){
			assert(0);
			return;
		}

		port = sw->logical_ports[pack->clas_state.port_in].port;
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

#endif //Guards
