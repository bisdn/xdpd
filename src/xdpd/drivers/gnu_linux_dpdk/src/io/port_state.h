/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _PORT_STATE_H_
#define _PORT_STATE_H_

#include <rte_ring.h>
#ifdef ENABLE_DPDK_SECONDARY_SEMAPHORE
	#include <semaphore.h>
#endif	
#include <inttypes.h>

#include "../config.h"

struct dpdk_port_state{
	//Core attachement state
	bool scheduled;
	bool queues_set; 
	unsigned int core_id;
	unsigned int core_port_slot;

	//port id (dpdk)
	unsigned int port_id;
}__rte_cache_aligned;

typedef struct dpdk_port_state dpdk_port_state_t;

/**
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief Represent the state of a DPDK SECONDARY NF port
*/
struct nf_port_state_dpdk
{
	/**
	* @brief queue used to send packets to a NF
	*/
	struct rte_ring *to_nf_queue;
	
	/**
	* @brief queue used to receive packets from a NF
	*/
	struct rte_ring *to_xdpd_queue;
	
#ifdef ENABLE_DPDK_SECONDARY_SEMAPHORE
	/**
	* @brief POSIX named semaphore used to implement batching towards the NF
	*/
	sem_t *semaphore;
#endif
	
	/**
	* @brief Count the number of packets inserted after the last sem_post
	*	(this counter does not consider the sem_post called for the expiration
	*	of the timeout)
	*/
	uint32_t counter_from_last_flush; 
	
	/**
	* @brief Core attachement information
	*/
	bool scheduled;
	unsigned int core_id;
	unsigned int core_port_slot;
	
	/**
	* @brief Identifier of the NF port
	*/
	unsigned int nf_id;
	
}__rte_cache_aligned;

typedef struct nf_port_state_dpdk nf_port_state_dpdk_t;  



/**
* @author Ivano Cerrato<ivano.cerrato (at) polito.it>
*
* @brief Represent the state of a DPDK KNI NF port
*/
struct nf_port_state_kni
{
	/**
	*	@brief: KNI context
	*/
	struct rte_kni *kni;
	
	/**
	*	@brief: flag to used to avoid a double
	*		notification to the CMM when the
	*		port is created
	*/
	bool just_created;
	
	/**
	* @brief Core attachement information
	*/
	bool scheduled;
	unsigned int core_id;
	unsigned int core_port_slot;
	
	/**
	* @brief Identifier of the NF port
	*/
	unsigned int nf_id;
	
}__rte_cache_aligned;

typedef struct nf_port_state_kni nf_port_state_kni_t;  

#endif //_PORT_STATE_H_
