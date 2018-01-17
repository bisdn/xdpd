/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef OPENFLOW12_SWITCH_H
#define OPENFLOW12_SWITCH_H 

#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/hal/driver.h>
#include <rofl/datapath/hal/openflow/openflow1x/of1x_driver.h>
#include "../openflow_switch.h"
#include "../openflow10/openflow10_switch.h"
#include "of12_endpoint.h"

/**
* @file openflow12_switch.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief OF1.2 switch implementation 
*/

namespace rofl {
	class ssl_context;
}  // namespace rofl

using namespace rofl;

namespace xdpd{

/**
* @brief Implementation of a (logical) OpenFlow v1.2 switch.
* @ingroup cmm_of 
*
* @description This includes the OF endpoint and the binding with the
* forwarding plane (through HAL)
*/
class openflow12_switch : public openflow_switch {


public:
	/**
	 * Constructor
	 */
	openflow12_switch(uint64_t dpid,
				std::string const& dpname,
				unsigned int num_of_tables,
				int* ma_list,
				int reconnect_start_timeout,
				const rofl::openflow::cofhello_elem_versionbitmap& versionbitmap,
				enum xdpd::csocket::socket_type_t socket_type,
				xdpd::cparams const& socket_params,
				sw_flavor_t flavor = SW_FLAVOR_GENERIC);


	/**
	 * Destructor
	 */
	virtual ~openflow12_switch(void);

	/*
	* Public interface for the instance
	*/
	
	virtual rofl_result_t process_packet_in(uint8_t table_id,
					uint8_t reason,
					uint32_t in_port,
					uint32_t buffer_id,
					uint64_t cookie,
					uint8_t* pkt_buffer,
					uint32_t buf_len,
					uint16_t total_len,
					packet_matches_t* matches);
	
	virtual rofl_result_t process_flow_removed(uint8_t reason, of1x_flow_entry_t* removed_flow_entry);

};

}// namespace rofl

#endif /* OPENFLOW12_SWITCH_H_ */
