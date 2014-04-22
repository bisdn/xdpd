/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PORT_STATE_H_
#define _PORT_STATE_H_

#include <rte_ring.h>
#include <semaphore.h>
#include <inttypes.h>

#include "../config.h"

struct dpdk_port_state{
	//Core attachement state
	bool scheduled;
	unsigned int core_id;
	unsigned int core_port_slot;

	//port id (dpdk)
	unsigned int port_id;
}__rte_cache_aligned;

typedef struct dpdk_port_state dpdk_port_state_t;

/**
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief Represent the state of a PEX port
*/
struct pex_port_state
{
	/**
	* @brief queue used to send packets to a PEX
	*/
	struct rte_ring *up_queue;
	
	/**
	* @brief queue used to receive packets from a PEX
	*/
	struct rte_ring *down_queue;
	
	/**
	* @brief POSIX named semaphore used to implement batching towards the PEX
	*/
	sem_t *semaphore;

	/**
	* @brief Time of the last sem_post done on the semaphore to wake up a PEX
	*/
	uint64_t last_flush_time;
	
	/**
	* @brief Count the number of packets inserted after the last sem_post
	*/
	uint32_t counter_from_last_flush; 
	
	/**
	* @brief Core attachement information
	*/
	bool scheduled;
	unsigned int core_id;
	unsigned int core_port_slot;

	/**
	* @brief Port identifier
	*/
	unsigned int port_id;
	
}__rte_cache_aligned;

typedef struct pex_port_state pex_port_state_t;  

#endif //_PORT_STATE_H_
