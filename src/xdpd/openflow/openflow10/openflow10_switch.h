/*
 * openflow10_switch.h
 *
 *  Created on: 06.09.2013
 *      Author: andreas
 */

#ifndef OPENFLOW10_SWITCH_H_
#define OPENFLOW10_SWITCH_H_

#include <rofl/datapath/pipeline/openflow/openflow1x/of1x_switch.h>
#include <rofl/datapath/hal/driver.h>
#include <rofl/datapath/hal/openflow/openflow1x/of1x_driver.h>
#include "../openflow_switch.h"
#include "of10_endpoint.h"

/**
* @file openflow12_switch.h
* @author Andreas Koepsel<andreas.koepsel (at) bisdn.de>
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief OF1.0 switch implementation 
*/

namespace rofl {
	class ssl_context;
}  // namespace rofl

using namespace rofl;

namespace xdpd {

/**
* @brief Implementation of a (logical) OpenFlow v1.0 switch.
* @ingroup cmm_of 
*
* @description This includes the OF endpoint and the binding with the
* forwarding plane (through HAL)
*/
class openflow10_switch : public openflow_switch {


public:
	/**
	 * Constructor
	 */
	openflow10_switch(uint64_t dpid,
				std::string const& dpname,
				unsigned int num_of_tables,
				int* ma_list,
				int reconnect_start_timeout,
				enum rofl::csocket::socket_type_t socket_type,
				cparams const& socket_params) throw (eOfSmVersionNotSupported);


	/**
	 * Destructor
	 */
	virtual ~openflow10_switch(void);

	/*
	* Public interface for the instance
	*/

	virtual rofl_result_t process_packet_in(uint8_t table_id,
					uint8_t reason,
					uint32_t in_port,
					uint32_t buffer_id,
					uint8_t* pkt_buffer,
					uint32_t buf_len,
					uint16_t total_len,
					packet_matches_t* matches);

	virtual rofl_result_t process_flow_removed(uint8_t reason, of1x_flow_entry_t* removed_flow_entry);

	/*
	* Port async notifications
	*/
	virtual rofl_result_t notify_port_attached(const switch_port_t* port);

	virtual rofl_result_t notify_port_detached(const switch_port_t* port);

	virtual rofl_result_t notify_port_status_changed(const switch_port_t* port);

	 /*
	 * Connecting and disconnecting from a controller entity
	 */
	virtual void rpc_connect_to_ctl(enum rofl::csocket::socket_type_t socket_type, caddress const& controller_addr);

	virtual void rpc_connect_to_ctl(enum rofl::csocket::socket_type_t socket_type, cparams const& socket_params);

	virtual void rpc_disconnect_from_ctl(caddress const& controller_addr);

};

}// namespace rofl


#endif /* OPENFLOW10_SWITCH_H_ */
