/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PORT_SNAPSHOT_H
#define PORT_SNAPSHOT_H 

#include <iostream> 
#include <string> 
#include <list> 
#include <rofl.h>
#include <rofl/common/caddress.h>
#include <rofl/datapath/pipeline/switch_port.h>

/**
* @file port_snapshot.h 
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief C++ port snapshot
*/

namespace xdpd {


/**
* @brief C++ switch port queue snapshot 
* @ingroup cmm_mgmt
*/
class port_queue_snapshot{

public:

	/**
	* Queue id
	*/
	uint32_t id; 
		
	/**
	* Queue  name
	*/
	std::string name;

	/**
	* Length of the queue (slot num.)
	*/
	uint16_t length;

	/**
	* Minimum rate (0 when unknown)
	*/
	uint16_t min_rate;

	/**
	* Maximum rate (0 when unknown)
	*/
	uint16_t max_rate;

	//
	// Stats
	//

	/**
	* Queue TX packets
	*/
	uint64_t stats_tx_pkts;

	/**
	* Queue TX bytes
	*/
	uint64_t stats_tx_bytes;

	/**
	* Overrun errors 
	*/
	uint64_t stats_overrun;


	port_queue_snapshot(port_queue_t const* s ):
		id(s->id),	
		name(s->name),	
		length(s->length),	
		min_rate(s->min_rate),
		max_rate(s->max_rate),
		stats_tx_pkts(s->stats.tx_packets),
		stats_tx_bytes(s->stats.tx_bytes),
		stats_overrun(s->stats.overrun)
	{}

};

/**
* @brief C++ switch port snapshot 
* @ingroup cmm_mgmt
*/
class port_snapshot {

public:	
	/**
	* mac address
	*/
	rofl::cmacaddr hw_address;

	//
	// General state
	//

	/**
	* Admin. state of the port
	*/
	bool up;

	/**
	* Forward packets flag
	*/
	bool forward_packets;	

	/**
	* Drop incomming packets flag
	*/
	bool drop_received;
	
	/**
	* Don't forward flood packets (OF1.0 only)
	*/
	bool no_flood;

	/**
	* Port type Is virtual/tun
	*/
	port_type_t type;

	/**
	* Port name
	*/
	std::string name;

	/**
	* Port state (port_state_t bitmap)
	*/
	bitmap32_t state;
	
	// Port statistics
	port_stats_t stats;

	//
	//Capabilities
	//

	/**
	* Port capabilities bitmap; curr
	*/
	bitmap32_t curr;
	/**
	* Port capabilities bitmap; advertised
	*/
	bitmap32_t advertised;
	/**
	* Port capabilities bitmap; supported 
	*/
	bitmap32_t supported;
	/**
	* Port capabilities bitmap; peer 
	*/
	bitmap32_t peer;

	//
	// Speeds
	//

	/**
	* Current speed
	*/
	port_features_t curr_speed;

	/**
	* Current MAX speed
	*/
	port_features_t curr_max_speed;

	//
	//Queues
	//
	/**
	* Port queues
	*/
	std::list<port_queue_snapshot> queues;	
	
	// 
	// OF related stuff
	//
	/**
	* Generate pkt in flag
	*/
	bool of_generate_packet_in;

	/**
	* To which datapath is the port attached (0x0 if none)
	*/
	uint64_t attached_sw_dpid;
 	
	/**
	* OpenFlow port number
	* @warning: OF port number != physical port num 
	*/
	unsigned int of_port_num; 
	

	port_snapshot(){};
	
	port_snapshot(switch_port_snapshot_t const* s ):
		up(s->up),
		forward_packets(s->forward_packets),
		drop_received(s->drop_received),
		no_flood(s->no_flood),
		type(s->type),
		name(s->name),
		state(s->state),
		stats(s->stats),
		curr(s->curr),
		advertised(s->advertised),
		supported(s->supported),
		peer(s->peer),
		curr_speed(s->curr_speed),
		curr_max_speed(s->curr_max_speed),
		of_generate_packet_in(s->of_generate_packet_in),
		attached_sw_dpid(s->attached_sw_dpid),
		of_port_num(s->of_port_num)
	{
		
		//HW address
		hw_address.unpack((uint8_t*)s->hwaddr, 6);
		
		for(unsigned int i=0;i<s->max_queues;++i){
			if(s->queues[i].set)
				queues.push_back(port_queue_snapshot(&s->queues[i])); 
		}	
	}	

 	//Dumping operator
	friend std::ostream& operator<<(std::ostream& os, const port_snapshot& p)
	{
		os << "[Port:"<<p.name<<"("<<p.hw_address<<") attached: 0x"<<std::hex<<p.attached_sw_dpid<<":"<<std::dec<<p.of_port_num<<"]\n";

		//TODO: Improve output 

		return os;
	}

};


}// namespace xdpd

#endif /* PORT_SNAPSHOT_H_ */
