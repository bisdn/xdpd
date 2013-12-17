/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PLATFORM_PORT_MGMT_
#define _PLATFORM_PORT_MGMT_

#include <rofl.h>
#include <rofl/datapath/afa/fwd_module.h>
#include "ports/ioport.h" 

/**
* @file iface_utils.h
* @author Victor Alvarez<victor.alvarez (at) bisdn.de>
*
* @brief Network interface (port) utils.
* 
*/


//C++ extern C
ROFL_BEGIN_DECLS

/**
 * Get the port by its name
 */
switch_port_t* get_port_by_name(const char *name);

/**
 * Discovers platform physical ports and fills up the switch_port_t sructures
 */
rofl_result_t discover_physical_ports(void);

/**
 * Creates a virtual port pair between two switches
 */
rofl_result_t create_virtual_port_pair(of_switch_t* lsw1, xdpd::gnu_linux::ioport** vport1, of_switch_t* lsw2, xdpd::gnu_linux::ioport** vport2);

/*
* Get port pair
*/
switch_port_t* get_vlink_pair(switch_port_t* port);

/*
 * @name update_port_status
 */
rofl_result_t update_port_status(char * name);
	
/**
 * Update physical port list 
 */
rofl_result_t update_physical_ports(void);

/**
 * Destroys ports previously created 
 */
rofl_result_t destroy_ports(void);


/*
 * Destroy port 
 */
rofl_result_t destroy_port(switch_port_t* port);

//Port enable/disable
/*
* Enable port (direct call to ioport instance)
*/
rofl_result_t enable_port(platform_port_state_t* ioport_instance);

/*
* Disable port (direct call to ioport instance)
*/
rofl_result_t disable_port(platform_port_state_t* ioport_instance);

//C++ extern C
ROFL_END_DECLS

#endif //PLATFORM_PORT_MGMT
