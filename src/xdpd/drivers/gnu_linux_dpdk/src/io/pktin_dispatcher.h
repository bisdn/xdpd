/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CTL_PACKETS_H
#define CTL_PACKETS_H 

/**
* @file pktin_dispatcher.h
* @author Marc Sune<marc.sune (at) bisdn.de>
*
* @brief Functions to dispatch pkt_ins from the processing subsystem
* to another thread (DPDK version). 
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <rofl/datapath/pipeline/common/datapacket.h>
#include <rofl/datapath/pipeline/openflow/of_switch.h>

#include "../config.h"
#include <rte_config.h>
#include <rte_common.h>
#include <rte_cycles.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>
#include <rte_spinlock.h>
#include <rte_ring.h>

extern sem_t pktin_sem;
extern struct rte_ring* pkt_ins;
extern bool keep_on_pktins;

//C++ extern C
ROFL_BEGIN_DECLS

/**
* Initialize packet_in dispatcher
*/
rofl_result_t pktin_dispatcher_init(void);

/**
* Stop and destroy packet_in dispatcher
*/
rofl_result_t pktin_dispatcher_destroy(void);


/**
* Waits until all PKT_INs for the switch are drained
* Note that *before* this function is called, NO more PKT_INs
* shall be enqueued for that LSI.
*/
void wait_pktin_draining(of_switch_t* sw);

/**
* Enqueue packet in the packet_ins queue
*/
inline rofl_result_t enqueue_pktin(datapacket_t* pkt){

	if( unlikely( rte_ring_mp_enqueue(pkt_ins, pkt) != 0 ) ){
		return ROFL_FAILURE;
	}	
	
	sem_post(&pktin_sem);
	return ROFL_SUCCESS;
}

//C++ extern C
ROFL_END_DECLS

#endif //CTL_PACKETS
