/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _NF_IFACE_MANAGER_H_
#define _NF_IFACE_MANAGER_H_

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
#include <rte_kni.h>
#include <rofl/datapath/pipeline/physical_switch.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>

#include "port_state.h"
#include "../processing/processing.h"


/**
* @file port_manager.h
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief Functions related to NF ports.
*/

/**
 * Identifier of NF ports
 */
 extern unsigned int nf_id;

/**
* TX is served only by the port lcore. The other lcores shall enqueue packets in this queue when they need to be flushed.
* This array is similar to the previous one but, for each port, it defines a single queue. In fact, NF ports (DPDK and
* KNI) does not have many queues
*/
extern struct rte_ring* port_tx_nf_lcore_queue[]; //PORT_MANAGER_MAX_PORTS

//C++ extern C
ROFL_BEGIN_DECLS

/**
* Handle commands for KNI ports
*/
void iface_manager_handle_kni_commands(void);

/**
 * @brief: used to prevent seg fault when a NF DPDK KNI port is descheduled
 */
extern pthread_rwlock_t rwlock;

/**
 * @name iface_manager_create_nf_port
 * @brief Create and initializes (including rofl-pipeline state) a NF port
 * 
 * @param nf_name		Name of the NF associated with the port to be created
 * @param port_name		Name of the port to be created
 * @param nf_type		Type of the NF to be created
 */
rofl_result_t iface_manager_create_nf_port(const char *nf_name, const char *port_name, port_type_t nf_port_type);

/**
 * @name iface_manager_destroy_nf_port
 * @brief Destroy a NF port, and its rofl-pipeline state
 * 
 * @param port_name		Name of the port to be destroyed
 */
rofl_result_t iface_manager_destroy_nf_port(const char *port_name);

/**
 * @brief bring up NF port
 * 
 * @param port NF port 
 */
rofl_result_t nf_iface_manager_bring_up_port(switch_port_t* port);

/**
 * @brief bring down NF port
 * 
 * @param port NF port 
 */
rofl_result_t nf_iface_manager_bring_down_port(switch_port_t* port);

//C++ extern C
ROFL_END_DECLS

#endif //_NF_IFACE_MANAGER_H_
