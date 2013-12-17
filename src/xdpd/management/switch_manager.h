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

#include <stdexcept>

#include <rofl/common/crofbase.h>
#include <rofl/platform/unix/cunixenv.h>
#include <rofl/common/cerror.h>

#include <rofl/datapath/pipeline/openflow/of_switch.h>

/**
* @file switch_manager.h
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
*
* @brief Logical Switch Instance (LSI) management API file.
*/

using namespace rofl;

namespace xdpd {

class eOfSmBase				: public cerror {};	// base error class for all switch_manager related errors
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

protected:
	
	/* Static members */
	//Switch container
	static std::map<uint64_t, openflow_switch*> switchs; 

public:

	static const caddress controller_addr;
	static const caddress binding_addr;

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
					caddress const& controller_addr = switch_manager::controller_addr,
					caddress const& binding_addr = switch_manager::binding_addr) throw (eOfSmExists, eOfSmErrorOnCreation, eOfSmVersionNotSupported);


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
	 * Finds the datapath by dpid 
	 */
	static openflow_switch* find_by_dpid(uint64_t dpid);

	/**
	 * Finds the datapath by name 
	 */
	static openflow_switch* find_by_name(std::string name);


	/**
	 * Lists datapath names
	 */
	static std::list<std::string> list_sw_names(void);

	/**
	 * List available matching algorithms
	 */
	static std::list<std::string> list_matching_algorithms(of_version_t of_version);


	/**
	 * connect to controller
	 */
	static void rpc_connect_to_ctl(uint64_t dpid, caddress const& ra);

	/**
	 * disconnect from from controller
	 */
	static void rpc_disconnect_from_ctl(uint64_t dpid, caddress const& ra);
};

}// namespace xdpd

#endif /* SWITCH_MANAGER_H_ */
