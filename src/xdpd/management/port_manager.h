/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PORT_MANAGER_H
#define PORT_MANAGER_H 

#include <list>
#include <string>
#include <stdbool.h>
#include <pthread.h>
#include <rofl.h>
#include <rofl/datapath/afa/fwd_module.h>
#include <rofl/common/croflexception.h>

/**
* @file port_manager.h
* @author Marc Sune<marc.sune (at) bisdn.de>
* @author Tobias Jungel<tobias.jungel (at) bisdn.de>
*
* @brief Port management API file.
*/

namespace xdpd {

class ePmBase		: public rofl::RoflException {};	// base error class for all switch_manager related errors
class ePmInvalidPort	: public ePmBase {};
class ePmUnknownError	: public ePmBase {};

/**
* @brief Port management API.
*
* The port manager API is a C++ interface that can be consumed
* by the add-on management modules for general port(interface) management operations.
* @ingroup cmm_mgmt
*/
class port_manager {

public:
	/*
	* This section contains the API to manage ports. This includes system port management and
	* port attachment to logical switchs
	*/

	//
	// Basic
	//

 	/**
	 * @brief Retrieves the port named port_name
	 */
	static bool port_exists(std::string& port_name);

	/**
	* @brief List the names of the system ports (regardless of the nature) available. 
	*/
	static std::list<std::string> list_available_port_names(void);


	//
	//Port operations
	//
	
	/**
	* Get meta-state up/down
	*/
	static bool get_admin_state(std::string& name);
	
	//TODO: Add more fine-grained "up/down states" here...
	
	/**
	* Set the port administratively up (meta up) 
	*/
	static void bring_up(std::string& name);
	
	/**
	* Set the port administratively down (meta down) 
	*/
	static void bring_down(std::string& name);


	//
	//Port attachment/detachment
	//

	/**
	 * @brief Attaches a port to a logical switch previously created. 
	 */
	static void attach_port_to_switch(uint64_t dpid, std::string& port_name, unsigned int* of_port_num);

	/**
	 * @brief Creates a virtual (xDPd) internal connection between two logical switch instances. On success
	 * the port names from both edges will contain the name of the generated port. 
	 */
	static void connect_switches(uint64_t dpid_lsi1, std::string& port_name1, uint64_t dpid_lsi2, std::string& port_name2);

	/**
	 * @brief Detaches a port from the logical, previously attached by port name. 
	 *
	 */
	static void detach_port_from_switch(uint64_t dpid, std::string& port_name);

	/**
	 * @brief Detaches a port from the logical, previously attached by Openflow port num. 
	 *
	 */
	static void detach_port_from_switch_by_num(uint64_t dpid, unsigned int port_num);

private:
	static pthread_mutex_t mutex;	
	
};

}// namespace xdpd 

#endif /* PORT_MANAGER_H_ */
