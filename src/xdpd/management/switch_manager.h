/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SWITCH_MANAGER_H
#define SWITCH_MANAGER_H 

#include <map>
#include <list>
#include <string>
#include <iostream>

#include <pthread.h>
#include <stdio.h>
#include <limits.h>
#include <endian.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdexcept>

#include <rofl.h>
#include <rofl/datapath/afa/afa.h>
#include <rofl/datapath/afa/cmm.h>
#include <rofl/common/caddress.h>
#include <rofl/common/croflexception.h>

#include <rofl/datapath/pipeline/openflow/of_switch.h>

/**
* @file switch_manager.h
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
*
* @brief Logical Switch Instance (LSI) management API file.
*/

namespace xdpd {

class eOfSmBase				: public rofl::RoflException {};	// base error class for all switch_manager related errors
class eOfSmGeneralError			: public eOfSmBase {};
class eOfSmErrorOnCreation		: public eOfSmBase {};
class eOfSmExists			: public eOfSmBase {};
class eOfSmDoesNotExist			: public eOfSmBase {};
class eOfSmNotFound			: public eOfSmBase {};
class eOfSmFlowModBadCommand		: public eOfSmBase {};
class eOfSmPipelineBadTableId		: public eOfSmBase {};
class eOfSmPipelineTableFull		: public eOfSmBase {};
class eOfSmPortModBadPort		: public eOfSmBase {};
class eOfSmVersionNotSupported		: public eOfSmBase {};

//Fwd declaration
class openflow_switch;

/**
* @brief Logical Switch (LS) management API.
* 
* The switch manager API is a C++ interface that can be consumed
* by the add-on management modules for general logical switch management
* (e.g. create/destroy logical switches)
* @ingroup cmm_mgmt
*/
class switch_manager {

public:

	//
	// Switch management
	//

	/**
	 * @brief	static factory method for creating a logical switch (LS) 
	 *
	 * This method creates a new Openflow Logical Switch instance with dpid and dpname.
	 *
	 * @param dpid data path element id
	 * @param dpname name of this data path element (local significance only)
	 */
	static openflow_switch* create_switch(of_version_t version,
					uint64_t dpid,
					std::string const& dpname,
					unsigned int num_of_tables,
					int* ma_list,
					int reconnect_start_timeout = 2,
					rofl::caddress const& controller_addr = switch_manager::controller_addr,
					rofl::caddress const& binding_addr = switch_manager::binding_addr) throw (eOfSmExists, eOfSmErrorOnCreation, eOfSmVersionNotSupported);


	/**
	 * @brief	static method that deletes 
	 *
	 * this method destroy the logical switch referenced by dpid. it also
	 * closes all the active connections and end-point listening sockets
	 *
	 * @param dpid data path element id
	 */
	static void destroy_switch(uint64_t dpid) throw (eOfSmDoesNotExist);

	/**
	 * @brief	static method that deletes all switches 
	 *
	 */
	static void destroy_all_switches(void);



	//Add missing attach port, detach and bring up down port


	/**
	 * Lists datapath names
	 */
	static std::list<std::string> list_sw_names(void);

	/**
	 * Return true if switch exists
	 */
	static bool exists(uint64_t dpid);

	/**
	 * Return true if switch exists with the name 'name'
	 */
	static bool exists_by_name(std::string& name);
		
	
	/**
	 * Return the dpid of the switch 
	 */
	static uint64_t get_switch_dpid(std::string& name);

	/**
	 * List available matching algorithms
	 */
	static std::list<std::string> list_matching_algorithms(of_version_t of_version);

	//
	// Switch controller setup
	//

	/**
	 * connect to controller
	 */
	static void rpc_connect_to_ctl(uint64_t dpid, rofl::caddress const& ra);

	/**
	 * disconnect from from controller
	 */
	static void rpc_disconnect_from_ctl(uint64_t dpid, rofl::caddress const& ra);

	//
	//CMM demux
	//
	
	static afa_result_t __notify_port_add(switch_port_snapshot_t* port_snapshot);	
	static afa_result_t __notify_port_status_changed(switch_port_snapshot_t* port_snapshot);	
	static afa_result_t __notify_port_delete(switch_port_snapshot_t* port_snapshot);	
	static afa_result_t __process_of1x_packet_in(uint64_t dpid,
					uint8_t table_id,
					uint8_t reason,
					uint32_t in_port,
					uint32_t buffer_id,
					uint8_t* pkt_buffer,
					uint32_t buf_len,
					uint16_t total_len,
					packet_matches_t* matches);
	static afa_result_t __process_of1x_flow_removed(uint64_t dpid, 
					uint8_t reason, 	
					of1x_flow_entry_t* removed_flow_entry);

private:
	
	/* Static members */
	//Switch container
	static std::map<uint64_t, openflow_switch*> switchs; 

	//Shall never be used except for the cmm
	static openflow_switch* __get_switch_by_dpid(uint64_t dpid);	

	//Default addresses
	static const rofl::caddress controller_addr;
	static const rofl::caddress binding_addr;

	static pthread_rwlock_t rwlock;

};




}// namespace xdpd

#endif /* SWITCH_MANAGER_H_ */
