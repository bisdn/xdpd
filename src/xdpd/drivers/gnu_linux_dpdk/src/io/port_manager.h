/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PORT_MGMT_H_
#define _PORT_MGMT_H_

#include <rofl.h>
#include "../config.h"
#include <rte_config.h> 
#include <rte_common.h> 
#include <rte_eal.h> 
#include <rte_log.h>
#include <rte_launch.h> 
#include <rte_mempool.h> 
#include <rte_mbuf.h> 
#include <rte_ethdev.h>
#include <rte_ring.h>
#include <rte_launch.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>

#include "port_state.h"
#include "../processing/processing.h"

//Maximum number of ports (preallocation of port_mapping)
#define PORT_MANAGER_MAX_PORTS PROCESSING_MAX_PORTS 

/**
 *  Port mappings
 */
extern switch_port_t* port_mapping[PORT_MANAGER_MAX_PORTS];

//C++ extern C
ROFL_BEGIN_DECLS

/**
* Discovers and initializes (including rofl-pipeline state) DPDK-enabled ports.
*/
rofl_result_t port_manager_discover_system_ports(void);

/**
* Creates a virtual link port pair
*/
rofl_result_t port_manager_create_virtual_port_pair(of_switch_t* lsw1, switch_port_t **vport1, of_switch_t* lsw2, switch_port_t **vport2);

/**
* Shutdown all ports in the system 
*/
rofl_result_t port_manager_destroy(void);

/**
* Setup tx and rx queues 
*/
rofl_result_t port_manager_set_queues(unsigned int core_id, unsigned int port_id);

/**
* Enable port 
*/
rofl_result_t port_manager_enable(switch_port_t* port);

/**
* Disable port 
*/
rofl_result_t port_manager_disable(switch_port_t* port);

/**
* Update link states 
*/
void port_manager_update_links(void);

/**
* Update port stats (pipeline)
*/
void port_manager_update_stats(void);


//C++ extern C
ROFL_END_DECLS

#endif //_PORT_MGMT_H_
