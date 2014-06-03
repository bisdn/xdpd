/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OPENFLOW_SWITCH_H
#define OPENFLOW_SWITCH_H 

#include <rofl/datapath/pipeline/openflow/of_switch.h>
#include <rofl/datapath/pipeline/platform/memory.h>
#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>

#include "of_endpoint.h"
#include "pirl/pirl.h"

/**
* @file openflow_switch.h
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
*
* @brief Defines the abstraction of an OpenFlow (logical) switch
*/

namespace xdpd {

/**
* @brief Version-agnostic abstraction of a complete
* (logical) OpenFlow switch.
* @ingroup cmm_of 
*/
class openflow_switch {

protected:
	//Endpoint context
	of_endpoint* endpoint;

	openflow_switch(const uint64_t dpid, const std::string &dpname, const of_version_t version, unsigned int num_of_tables);

public:
	//General elements
	uint64_t dpid;
	std::string dpname;
	of_version_t version;
	unsigned int num_of_tables;

	//Packet in rate limiter
	pirl rate_limiter;
	
	/**
	 * Destructor
	 */
	virtual ~openflow_switch(void){};


	/*
	* Pure virtual methods that must be implemented by derived classes
	*/
	inline rofl_result_t process_and_filter_packet_in(
					uint8_t table_id,
					uint8_t reason,
					uint32_t in_port,
					uint32_t buffer_id,
					uint8_t* pkt_buffer,
					uint32_t buf_len,
					uint16_t total_len,
					packet_matches_t* matches){

		if(rate_limiter.filter_pkt() == true)
			//Packet exceeds configured rate
			return ROFL_FAILURE;
		
		return process_packet_in(table_id,
					reason,
					in_port,
					buffer_id,
					pkt_buffer,
					buf_len,
					total_len,
					matches);
	};

	virtual rofl_result_t process_flow_removed(uint8_t reason, of1x_flow_entry_t* removed_flow_entry)=0;

	/**
	*
	* Dispatching of version agnostic messages comming from the driver
	*
	*/
	virtual rofl_result_t notify_port_attached(const switch_port_t* port);
	
	virtual rofl_result_t notify_port_detached(const switch_port_t* port);
	
	virtual rofl_result_t notify_port_status_changed(const switch_port_t* port);

	/**
	 * Connecting and disconnecting from a controller entity
	 */
	virtual void rpc_connect_to_ctl(enum rofl::csocket::socket_type_t socket_type, cparams const& socket_params);

	virtual void rpc_disconnect_from_ctl(enum rofl::csocket::socket_type_t socket_type, cparams const& socket_params);

protected:
	/*
	* Pure virtual method that must be implemented by derived classes
	*/
	virtual rofl_result_t process_packet_in(
					uint8_t table_id,
					uint8_t reason,
					uint32_t in_port,
					uint32_t buffer_id,
					uint8_t* pkt_buffer,
					uint32_t buf_len,
					uint16_t total_len,
					packet_matches_t* matches)=0;

};

}// namespace rofl

#endif /* OPENFLOW_SWITCH_H_ */
